#!/usr/bin/env python3

import argparse
import json
import os
from pathlib import Path, PureWindowsPath
import re
import secrets
import subprocess
import sys
import time

from owned_wine_prefix import prepare_owned_wine_prefix, stop_owned_wine_prefix
from local_artifact import require_local_artifact_path
from movie_skip import configure_intro_movie_skip, restore_intro_movie_config
from runtime_process import (
    matching_scenario_process_ids,
    stage_scenario_executable,
    stop_executable_processes,
)
from wine_runtime import find_wine


FATAL_PATTERNS = (
    re.compile(r"Unhandled exception(?: code)?", re.IGNORECASE),
    re.compile(r"wine: Unhandled", re.IGNORECASE),
    re.compile(r"err:seh:NtRaiseException Unhandled", re.IGNORECASE),
)
LOADER_RECORD = re.compile(
    r'^(?P<context>[0-9A-F]+)(?::[0-9A-F]+)?:.*'
    r'\bLoaded\s+L?"(?P<path>[^"]+)"\s+at\s+'
    r'(?:0x)?[0-9A-F]+:\s*(?P<kind>native|builtin)\s*$',
    re.IGNORECASE,
)
CONTEXT_RECORD = re.compile(
    r"^(?P<context>[0-9A-F]+)(?::[0-9A-F]+)?:", re.IGNORECASE)
LOADER_FAILURE_PATTERNS = (
    re.compile(
        r"\b(?:failed|unable|could not|cannot)\s+to\s+load\b",
        re.IGNORECASE),
    re.compile(r"\bload(?:ing)?\b.*\bfailed\b", re.IGNORECASE),
    re.compile(r"\bmodule\b.*\bnot found\b", re.IGNORECASE),
    re.compile(r"\bimport\b.*\b(?:failed|unresolved)\b", re.IGNORECASE),
)
RUNTIME_ORACLE_ENVIRONMENT = "OPENSMACX_RUNTIME_ORACLE_RESULT"


def diagnostic_context(line):
    match = CONTEXT_RECORD.search(line)
    return match.group("context").casefold() if match else None


def analyze_diagnostics(text, executable_name):
    lines = text.splitlines()
    loaded = {}
    for line in lines:
        match = LOADER_RECORD.search(line)
        if match:
            context = match.group("context").casefold()
            name = PureWindowsPath(match.group("path")).name.casefold()
            loaded.setdefault(context, {}).setdefault(name, set()).add(
                match.group("kind").casefold())
    required = {
        "executable": (executable_name.casefold(), "native"),
        "opensmacx": ("opensmacx.dll", "native"),
        "pracx": ("prax.dll", "native"),
        "builtin_ddraw": ("ddraw.dll", "builtin"),
    }
    executable, executable_kind = required["executable"]
    game_contexts = [
        context for context, modules in loaded.items()
        if executable_kind in modules.get(executable, set())
    ]
    best_context = None
    if game_contexts:
        best_context = max(sorted(game_contexts), key=lambda context: sum(
            kind in loaded[context].get(name, set())
            for name, kind in required.values()))
    missing = list(required) if best_context is None else [
        label for label, (name, kind) in required.items()
        if kind not in loaded[best_context].get(name, set())
    ]
    fatal_lines = [
        line for line in lines
        if any(pattern.search(line) for pattern in FATAL_PATTERNS)
    ]
    required_names = {name for name, _ in required.values()}
    loader_failure_lines = []
    for line in lines:
        context = diagnostic_context(line)
        if game_contexts and context not in game_contexts:
            continue
        folded = line.casefold()
        generic_required_failure = (
            any(name in folded for name in required_names)
            and any(pattern.search(line) for pattern in LOADER_FAILURE_PATTERNS))
        if generic_required_failure:
            loader_failure_lines.append(line)
    rendering_started = any(
        (not game_contexts or diagnostic_context(line) in game_contexts)
        and "ddraw_surface" in line.casefold() and "flip" in line.casefold()
        for line in lines)
    return {
        "fatal_lines": fatal_lines,
        "loader_failure_lines": loader_failure_lines,
        "missing_markers": missing,
        "rendering_started": rendering_started,
    }


def validate_smoke(analysis, new_processes):
    if analysis["fatal_lines"]:
        raise RuntimeError("Wine diagnostics contain an unhandled exception")
    if analysis["loader_failure_lines"]:
        raise RuntimeError("Wine diagnostics contain a module or import load failure")
    if analysis["missing_markers"]:
        raise RuntimeError(
            "missing loader markers: " + ", ".join(analysis["missing_markers"]))
    if not new_processes:
        raise RuntimeError("the launched game process did not survive the smoke window")
    return "surface_flip" if analysis["rendering_started"] else "process_survival"


def validate_runtime_oracles(result_path, expected_suites=()):
    """The suites that ran, or a refusal naming what did not.

    `expected_suites` is what the DLL registers. Checking the terminator alone
    was not enough and the tree proves it: the last green gate's result file
    lists twelve suites and `all passed` while `generated-signatures` - 108
    oracles - is absent, and the smoke log holds zero verdict lines. The suite
    needs far longer than the smoke window, the process was killed mid-run, and
    the gate reported success. A named-set check is what turns that into a
    failure; the terminator cannot, because a truncated run's terminator is
    indistinguishable from a complete one's.
    """
    if not result_path.is_file():
        raise RuntimeError("runtime recovery oracles did not produce a result")
    lines = result_path.read_text(encoding="ascii").splitlines()
    if lines and lines[-1] == "report-overflow":
        raise RuntimeError(
            "runtime oracle report overflowed its buffer; the run is truncated "
            "and its verdicts are incomplete")
    if lines and lines[-1] == "deferred pending":
        raise RuntimeError(
            "runtime recovery oracles stopped after the first phase: the "
            "deferred suites never ran")
    if not lines or lines[-1] != "all passed":
        failed = [line.rsplit(" ", 1)[0] for line in lines
                  if line.endswith(" failed")]
        if failed:
            raise RuntimeError(
                "runtime recovery oracle suites failed: " + ", ".join(failed))
        raise RuntimeError("runtime recovery oracles did not report all passed")
    suites = {}
    for line in lines[:-1]:
        name, _, status = line.rpartition(" ")
        if not name or status not in ("passed", "failed"):
            raise RuntimeError(
                f"malformed runtime oracle result line: {line!r}")
        suites[name] = status
    if not suites:
        raise RuntimeError("runtime recovery oracles reported no suites")
    missing = [name for name in expected_suites if name not in suites]
    if missing:
        raise RuntimeError(
            "runtime recovery oracle suites never reported: "
            + ", ".join(missing))
    return suites


def parse_generated_verdicts(text):
    """{address: state} from the generated suite's machine-readable lines.

    The verdict lines are the only record that the 108 generated oracles ran at
    all: the suite writes one `GENERATED-ORACLE-VERDICT: 0xADDR STATE name` per
    function through printf+fflush, so they survive a process that dies later.
    """
    verdicts = {}
    for match in re.finditer(
            r"^GENERATED-ORACLE-VERDICT: (0x[0-9A-Fa-f]{8}) (\S+)",
            text, re.M):
        verdicts[int(match.group(1), 16)] = match.group(2)
    return verdicts


def prepare_runtime_oracle_result_path(value):
    path = require_local_artifact_path(
        Path(value).expanduser().absolute(), "runtime oracle result")
    if not path.parent.is_dir():
        raise RuntimeError("runtime oracle result parent directory must exist")
    path.unlink(missing_ok=True)
    return path


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


def launch(executable, wine, wine_prefix, log_path, oracle_result):
    environment = os.environ.copy()
    environment["WINEDEBUG"] = "+loaddll"
    environment.pop(RUNTIME_ORACLE_ENVIRONMENT, None)
    oracle_value = (str(oracle_result) if os.name == "nt"
                    else "Z:" + str(oracle_result).replace("/", "\\"))
    environment[RUNTIME_ORACLE_ENVIRONMENT] = oracle_value
    if wine_prefix:
        environment["WINEPREFIX"] = str(wine_prefix)

    if os.name == "nt":
        log_file = log_path.open("w", encoding="utf-8")
        process = subprocess.Popen(
            [str(executable)], cwd=str(executable.parent),
            env=environment,
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
                "--env", f"{RUNTIME_ORACLE_ENVIRONMENT}={oracle_value}",
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
    parser.add_argument("--oracle-result", required=True)
    parser.add_argument("--expect-oracle-suite", action="append", default=[],
                        metavar="NAME",
                        help="a suite the DLL registers and must report; "
                             "repeatable. Absent names fail the smoke.")
    parser.add_argument("--expect-verdicts", type=int, default=0,
                        metavar="N",
                        help="minimum GENERATED-ORACLE-VERDICT lines the run "
                             "must emit")
    parser.add_argument("--play-intro-movie", action="store_true",
                        help="Leave PRACX's configured movie player enabled")
    args = parser.parse_args()

    game_dir = Path(args.game_dir).expanduser().resolve()
    log_path = Path(args.log).expanduser().resolve()
    result_path = Path(args.result).expanduser().resolve()
    oracle_result_path = Path(args.oracle_result).expanduser().absolute()
    wine_prefix = (Path(args.wine_prefix).expanduser().absolute()
                   if args.wine_prefix else None)
    report = {
        "duration_seconds": args.duration,
        "status": "failed",
    }

    prefix_prepared = False
    process = None
    scenario_executable = None
    movie_config = None
    try:
        if args.duration <= 0:
            raise RuntimeError("duration must be positive")
        if os.name != "nt" and wine_prefix is None:
            raise RuntimeError("a dedicated --wine-prefix is required")
        if not log_path.parent.is_dir() or not result_path.parent.is_dir():
            raise RuntimeError("log and result parent directories must exist")
        oracle_result_path = prepare_runtime_oracle_result_path(oracle_result_path)
        if os.name != "nt":
            prepare_owned_wine_prefix(wine_prefix, args.wine)
            prefix_prepared = True
        executable = validate_executable(game_dir, args.executable)
        if not args.play_intro_movie:
            movie_config = configure_intro_movie_skip(game_dir, secrets.token_hex(16))
            report["intro_movie_skipped"] = True
        scenario_executable = stage_scenario_executable(
            executable, secrets.token_hex(16))
        before = matching_scenario_process_ids(scenario_executable)
        log_path.write_text("", encoding="utf-8")
        process, log_file = launch(
            scenario_executable, args.wine, wine_prefix, log_path,
            oracle_result_path)
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
        analysis = analyze_diagnostics(diagnostics, scenario_executable.name)
        after = matching_scenario_process_ids(scenario_executable)
        new_processes = sorted(after - before)
        # Recorded BEFORE validation, deliberately. How far the generated suite
        # got is at its most useful on the run that failed - a suite that dies
        # on function 18 of 108 has still banked 17 verdicts, and they are the
        # whole reason the run was worth doing. Reporting them only on success
        # threw away the evidence in exactly the case that produced it.
        verdicts = parse_generated_verdicts(diagnostics)
        states = {}
        for state in verdicts.values():
            states[state] = states.get(state, 0) + 1
        report["generated_oracle_verdicts"] = len(verdicts)
        report["generated_oracle_verdict_states"] = dict(sorted(states.items()))
        oracle_suites = validate_runtime_oracles(
            oracle_result_path, args.expect_oracle_suite)
        if len(verdicts) < args.expect_verdicts:
            raise RuntimeError(
                f"the generated oracle suite produced {len(verdicts)} verdicts, "
                f"expected at least {args.expect_verdicts}")
        report.update({
            **analysis,
            "executable": str(executable),
            "launch_executable": str(scenario_executable),
            "log": str(log_path),
            "new_processes": new_processes,
            "preexisting_processes": sorted(before),
            "runtime_oracles": oracle_suites,
        })
        report["runtime_evidence"] = validate_smoke(analysis, new_processes)
        report["status"] = "passed"
    except (OSError, subprocess.CalledProcessError, RuntimeError) as error:
        report["error"] = str(error)
    finally:
        try:
            if os.name == "nt" and process is not None and process.poll() is None:
                process.terminate()
                process.wait(timeout=5.0)
            elif prefix_prepared:
                stop_owned_wine_prefix(wine_prefix, args.wine)
            if (scenario_executable is not None
                    and not stop_executable_processes(scenario_executable)):
                raise RuntimeError("smoke processes did not terminate")
            if scenario_executable is not None:
                scenario_executable.unlink(missing_ok=True)
            report["runtime_stopped"] = True
        except (OSError, subprocess.CalledProcessError, subprocess.TimeoutExpired,
                RuntimeError) as error:
            report["status"] = "failed"
            report["runtime_stopped"] = False
            report["error"] = f"runtime cleanup failed: {error}"
        finally:
            if movie_config is not None:
                try:
                    restore_intro_movie_config(*movie_config)
                except OSError as error:
                    report["status"] = "failed"
                    report["error"] = f"movie configuration cleanup failed: {error}"
    result_path.write_text(
        json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(f"Hybrid smoke result: {report['status']}")
    print(f"Diagnostics: {log_path}")
    print(f"Result: {result_path}")
    if report["status"] != "passed":
        raise SystemExit(1)


if __name__ == "__main__":
    main()
