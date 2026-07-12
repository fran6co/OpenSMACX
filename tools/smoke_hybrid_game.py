#!/usr/bin/env python3

import argparse
import json
import os
from pathlib import Path, PureWindowsPath
import re
import subprocess
import sys
import time

from setup_game import find_wine


FATAL_PATTERNS = (
    re.compile(r"Unhandled exception(?: code)?", re.IGNORECASE),
    re.compile(r"wine: Unhandled", re.IGNORECASE),
    re.compile(r"err:seh:NtRaiseException Unhandled", re.IGNORECASE),
)


def analyze_diagnostics(text, executable_name):
    lines = text.splitlines()
    lowered = text.casefold()
    required = {
        "executable": executable_name.casefold(),
        "opensmacx": "opensmacx.dll",
        "pracx": "prax.dll",
    }
    missing = [name for name, marker in required.items() if marker not in lowered]
    builtin_ddraw = any(
        "ddraw.dll" in line.casefold() and "builtin" in line.casefold()
        for line in lines)
    if not builtin_ddraw:
        missing.append("builtin_ddraw")
    fatal_lines = [
        line for line in lines
        if any(pattern.search(line) for pattern in FATAL_PATTERNS)
    ]
    rendering_started = any(
        "ddraw_surface" in line.casefold() and "flip" in line.casefold()
        for line in lines)
    return {
        "fatal_lines": fatal_lines,
        "missing_markers": missing,
        "rendering_started": rendering_started,
    }


def matching_process_ids(executable):
    if os.name == "nt":
        return set()
    result = subprocess.run(
        ["ps", "-ax", "-o", "pid=,command="], check=True,
        text=True, capture_output=True)
    native_path = str(executable).casefold()
    windows_path = ("z:" + str(executable).replace("/", "\\")).casefold()
    matches = set()
    for line in result.stdout.splitlines():
        fields = line.strip().split(maxsplit=1)
        if len(fields) != 2:
            continue
        pid_text, command = fields
        if native_path not in command.casefold() and windows_path not in command.casefold():
            continue
        pid = int(pid_text)
        if pid != os.getpid():
            matches.add(pid)
    return matches


def validate_executable(game_dir, executable_name):
    windows_name = PureWindowsPath(executable_name)
    if (not executable_name or windows_name.drive or windows_name.root or
            len(windows_name.parts) != 1 or ":" in executable_name or
            not executable_name.casefold().endswith(".exe")):
        raise RuntimeError(f"invalid executable filename: {executable_name!r}")
    executable = game_dir / executable_name
    if executable.is_symlink() or executable.resolve().parent != game_dir:
        raise RuntimeError(f"executable is outside the game directory: {executable}")
    if not executable.is_file():
        raise RuntimeError(f"staged executable not found: {executable}")
    return executable


def launch(executable, wine, wine_prefix, log_path):
    environment = os.environ.copy()
    environment["WINEDEBUG"] = "+loaddll"
    if wine_prefix:
        environment["WINEPREFIX"] = str(wine_prefix)

    if os.name == "nt":
        log_file = log_path.open("w", encoding="utf-8")
        process = subprocess.Popen(
            [str(executable)], cwd=str(executable.parent),
            stdout=log_file, stderr=subprocess.STDOUT)
        return process, log_file

    wine_path = Path(find_wine(wine))
    if sys.platform == "darwin" and len(wine_path.parents) >= 5:
        application = wine_path.parents[4]
        if application.suffix == ".app":
            command = ["open", "-n", "-a", application.stem]
            if wine_prefix:
                command.extend(["--env", f"WINEPREFIX={wine_prefix}"])
            command.extend([
                "--env", "WINEDEBUG=+loaddll",
                "--stderr", str(log_path),
                "--args", str(executable),
            ])
            subprocess.run(command, cwd=str(executable.parent), check=True)
            return None, None

    log_file = log_path.open("w", encoding="utf-8")
    process = subprocess.Popen(
        [str(wine_path), str(executable)], cwd=str(executable.parent),
        env=environment, stdout=log_file, stderr=subprocess.STDOUT)
    return process, log_file


def main():
    parser = argparse.ArgumentParser(
        description="Smoke-test a staged OpenSMACX hybrid game")
    parser.add_argument("--game-dir", required=True)
    parser.add_argument("--executable", default="terranx_hybrid.exe")
    parser.add_argument("--wine")
    parser.add_argument("--wine-prefix")
    parser.add_argument("--duration", type=float, default=20.0)
    parser.add_argument("--log", required=True)
    parser.add_argument("--result", required=True)
    args = parser.parse_args()

    game_dir = Path(args.game_dir).expanduser().resolve()
    log_path = Path(args.log).expanduser().resolve()
    result_path = Path(args.result).expanduser().resolve()
    wine_prefix = (Path(args.wine_prefix).expanduser().resolve()
                   if args.wine_prefix else None)
    report = {
        "duration_seconds": args.duration,
        "status": "failed",
    }

    try:
        if args.duration <= 0:
            raise RuntimeError("duration must be positive")
        if not log_path.parent.is_dir() or not result_path.parent.is_dir():
            raise RuntimeError("log and result parent directories must exist")
        executable = validate_executable(game_dir, args.executable)
        before = matching_process_ids(executable)
        log_path.write_text("", encoding="utf-8")
        process, log_file = launch(
            executable, args.wine, wine_prefix, log_path)
        try:
            deadline = time.monotonic() + args.duration
            while time.monotonic() < deadline:
                time.sleep(min(1.0, max(0.0, deadline - time.monotonic())))
                if process is not None and process.poll() is not None:
                    break
        finally:
            if log_file is not None:
                log_file.flush()
                log_file.close()

        diagnostics = log_path.read_text(encoding="utf-8", errors="replace")
        analysis = analyze_diagnostics(diagnostics, executable.name)
        after = matching_process_ids(executable)
        new_processes = sorted(after - before)
        report.update({
            **analysis,
            "executable": str(executable),
            "log": str(log_path),
            "new_processes": new_processes,
            "preexisting_processes": sorted(before),
        })
        if analysis["fatal_lines"]:
            raise RuntimeError("Wine diagnostics contain an unhandled exception")
        if analysis["missing_markers"]:
            raise RuntimeError(
                "missing loader markers: " + ", ".join(analysis["missing_markers"]))
        if not analysis["rendering_started"]:
            raise RuntimeError("DirectDraw rendering did not reach a surface flip")
        if not new_processes and not before:
            raise RuntimeError("the launched game process did not survive the smoke window")
        report["status"] = "passed"
    except (OSError, subprocess.CalledProcessError, RuntimeError) as error:
        report["error"] = str(error)
    result_path.write_text(
        json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(f"Hybrid smoke result: {report['status']}")
    print(f"Diagnostics: {log_path}")
    print(f"Result: {result_path}")
    if report["status"] != "passed":
        raise SystemExit(1)


if __name__ == "__main__":
    main()
