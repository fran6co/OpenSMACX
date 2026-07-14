#!/usr/bin/env python3

import argparse
import json
import os
from pathlib import Path
import secrets
import subprocess
import sys
import time

from owned_wine_prefix import prepare_owned_wine_prefix, stop_owned_wine_prefix
from movie_skip import configure_intro_movie_skip, restore_intro_movie_config
from runtime_process import (
    command_runs_scenario_executable,
    matching_scenario_process_ids,
    scenario_executable_path,
    stage_scenario_executable,
    stop_executable_processes,
)
from setup_game import find_wine
from smoke_hybrid_game import FATAL_PATTERNS, matching_process_ids, validate_executable


def resolve_fixture(game_dir, value):
    game_dir = game_dir.resolve()
    candidate = game_dir / value
    resolved = candidate.resolve()
    try:
        resolved.relative_to(game_dir)
    except ValueError as error:
        raise RuntimeError(f"scenario fixture is outside the game directory: {candidate}") from error
    if candidate.is_symlink() or not resolved.is_file():
        raise RuntimeError(f"scenario fixture not found: {candidate}")
    return resolved


def wine_path(path):
    return "Z:" + str(path).replace("/", "\\")


def validate_report(
        report, inspect, vehicle=None, x=None, y=None, resolve=False, advance=False):
    status = report.get("status")
    if status == "failed":
        raise RuntimeError(f"scenario failed: {report.get('error', 'unknown_error')}")
    if inspect:
        candidate = report.get("candidate")
        adjacent = report.get("adjacent")
        valid = (status == "inspected"
                 and type(report.get("turn")) is int
                 and type(report.get("current_faction")) is int
                 and type(report.get("vehicle_count")) is int
                 and isinstance(candidate, dict)
                 and all(type(candidate.get(name)) is int
                         for name in ("id", "x", "y", "prototype", "triad", "moves"))
                 and isinstance(adjacent, list) and bool(adjacent)
                 and all(isinstance(tile, dict)
                         and all(type(tile.get(name)) is int
                                 for name in ("x", "y", "occupied", "cost"))
                         and type(tile.get("ocean")) is bool
                         and type(tile.get("base")) is bool
                         for tile in adjacent))
        if not valid:
            raise RuntimeError(f"expected complete inspection result, got {status!r}")
        return
    target = report.get("target")
    start = report.get("start")
    if (status != "passed"
            or report.get("movement_ordered") is not True
            or report.get("end_turn_requested") is not True
            or type(report.get("vehicle")) is not int
            or report["vehicle"] != vehicle
            or not isinstance(target, list) or len(target) != 2
            or any(type(value) is not int for value in target)
            or target != [x, y]
            or type(report.get("turn")) is not int
            or not isinstance(start, list) or len(start) != 2
            or any(type(value) is not int for value in start)):
        raise RuntimeError(f"expected passing scenario result, got {status!r}")
    if resolve and (report.get("movement_resolved") is not True
                    or report.get("order_cleared") is not True
                    or type(report.get("movement_cost")) is not int
                    or report["movement_cost"] <= 0):
        raise RuntimeError("expected resolved movement result")
    if advance and (report.get("turn_advanced") is not True
                    or type(report.get("initial_turn")) is not int
                    or type(report.get("advanced_turn")) is not int
                    or report["advanced_turn"] != report["initial_turn"] + 1
                    or type(report.get("mission_year")) is not int):
        raise RuntimeError("expected turn advancement result")


def fatal_diagnostics(text):
    return [
        line for line in text.splitlines()
        if any(pattern.search(line) for pattern in FATAL_PATTERNS)
    ]


def cleanup_owned_processes(
        executable, process, wine=None, wine_prefix=None):
    if os.name == "nt":
        if process is not None and process.poll() is None:
            process.terminate()
            try:
                process.wait(timeout=5.0)
            except subprocess.TimeoutExpired:
                return False
        return True

    stop_owned_wine_prefix(wine_prefix, wine)
    return stop_executable_processes(executable)


def launch(executable, wine, wine_prefix, variables, log_path):
    environment = os.environ.copy()
    environment["WINEDEBUG"] = "+seh,+tid"
    for name in (
            "OPENSMACX_SCENARIO_SAVE", "OPENSMACX_SCENARIO_RESULT",
            "OPENSMACX_SCENARIO_INSPECT", "OPENSMACX_SCENARIO_VEHICLE",
            "OPENSMACX_SCENARIO_X", "OPENSMACX_SCENARIO_Y",
            "OPENSMACX_SCENARIO_RESOLVE", "OPENSMACX_SCENARIO_ADVANCE"):
        environment.pop(name, None)
    environment.update(variables)
    if wine_prefix:
        environment["WINEPREFIX"] = str(wine_prefix)

    if os.name == "nt":
        log_file = log_path.open("w", encoding="utf-8")
        process = subprocess.Popen(
            [str(executable)], cwd=str(executable.parent), env=environment,
            stdout=log_file, stderr=subprocess.STDOUT)
        return process, log_file

    wine_path_value = Path(find_wine(wine))
    if sys.platform == "darwin" and len(wine_path_value.parents) >= 5:
        application = wine_path_value.parents[4]
        if application.suffix == ".app":
            command = ["open", "-n", "-a", application.stem]
            if wine_prefix:
                command.extend(["--env", f"WINEPREFIX={wine_prefix}"])
            command.extend(["--env", "WINEDEBUG=+seh,+tid"])
            for name, value in variables.items():
                command.extend(["--env", f"{name}={value}"])
            command.extend(["--stderr", str(log_path), "--args", str(executable)])
            subprocess.run(command, cwd=str(executable.parent), check=True)
            return None, None

    log_file = log_path.open("w", encoding="utf-8")
    process = subprocess.Popen(
        [str(wine_path_value), str(executable)], cwd=str(executable.parent),
        env=environment, stdout=log_file, stderr=subprocess.STDOUT)
    return process, log_file


def main():
    parser = argparse.ArgumentParser(
        description="Run a deterministic in-process OpenSMACX gameplay scenario")
    parser.add_argument("--game-dir", required=True)
    parser.add_argument("--executable", default="terranx_hybrid.exe")
    parser.add_argument("--save", required=True,
                        help="Fixture path relative to the staged game directory")
    parser.add_argument("--result", required=True)
    parser.add_argument("--log", required=True)
    parser.add_argument("--wine")
    parser.add_argument("--wine-prefix")
    parser.add_argument("--timeout", type=float, default=120.0)
    parser.add_argument("--inspect", action="store_true")
    parser.add_argument("--resolve", action="store_true",
                        help="Resolve the issued movement order before asserting state")
    parser.add_argument("--advance", action="store_true",
                        help="Continue through the next turn/year increment")
    parser.add_argument("--vehicle", type=int)
    parser.add_argument("--x", type=int)
    parser.add_argument("--y", type=int)
    parser.add_argument("--leave-running", action="store_true",
                        help="Do not terminate processes created by this scenario run")
    parser.add_argument("--play-intro-movie", action="store_true",
                        help="Leave PRACX's configured movie player enabled")
    args = parser.parse_args()
    if args.advance:
        args.resolve = True

    game_dir = Path(args.game_dir).expanduser().resolve()
    result_path = Path(args.result).expanduser().resolve()
    log_path = Path(args.log).expanduser().resolve()
    wine_prefix = (Path(args.wine_prefix).expanduser().absolute()
                   if args.wine_prefix else None)

    if args.timeout <= 0:
        parser.error("timeout must be positive")
    if os.name != "nt" and wine_prefix is None:
        parser.error("a dedicated --wine-prefix is required")
    if not result_path.parent.is_dir() or not log_path.parent.is_dir():
        parser.error("result and log parent directories must exist")
    if not args.inspect and (args.vehicle is None or args.x is None or args.y is None):
        parser.error("execution requires --vehicle, --x, and --y")
    if not args.inspect and any(
            value < -2147483648 or value > 2147483647
            for value in (args.vehicle, args.x, args.y)):
        parser.error("vehicle and coordinates must fit a signed 32-bit integer")

    executable = None
    scenario_executable = None
    process = None
    before = set()
    launched = False
    owned_processes = set()
    movie_config = None
    token = secrets.token_hex(16)
    try:
        executable = validate_executable(game_dir, args.executable)
        before = matching_process_ids(executable)
        fixture = resolve_fixture(game_dir, args.save)
        result_path.unlink(missing_ok=True)
        log_path.write_text("", encoding="utf-8")
        if os.name != "nt":
            prepare_owned_wine_prefix(wine_prefix, args.wine)
        scenario_executable = stage_scenario_executable(executable, token)
        if not args.play_intro_movie:
            movie_config = configure_intro_movie_skip(game_dir, token)

        variables = {
            "OPENSMACX_SCENARIO_SAVE": (str(fixture) if os.name == "nt" else wine_path(fixture)),
            "OPENSMACX_SCENARIO_RESULT": (
                str(result_path) if os.name == "nt" else wine_path(result_path)),
            "OPENSMACX_SCENARIO_INSPECT": "",
            "OPENSMACX_SCENARIO_VEHICLE": "",
            "OPENSMACX_SCENARIO_X": "",
            "OPENSMACX_SCENARIO_Y": "",
            "OPENSMACX_SCENARIO_RESOLVE": "",
            "OPENSMACX_SCENARIO_ADVANCE": "",
        }
        if args.inspect:
            variables["OPENSMACX_SCENARIO_INSPECT"] = "1"
        else:
            variables.update({
                "OPENSMACX_SCENARIO_VEHICLE": str(args.vehicle),
                "OPENSMACX_SCENARIO_X": str(args.x),
                "OPENSMACX_SCENARIO_Y": str(args.y),
            })
            if args.resolve:
                variables["OPENSMACX_SCENARIO_RESOLVE"] = "1"
            if args.advance:
                variables["OPENSMACX_SCENARIO_ADVANCE"] = "1"

        process, log_file = launch(
            scenario_executable, args.wine, wine_prefix, variables, log_path)
        launched = True
        try:
            deadline = time.monotonic() + args.timeout
            while time.monotonic() < deadline:
                if os.name != "nt":
                    owned_processes.update(
                        matching_scenario_process_ids(scenario_executable))
                if result_path.is_file():
                    try:
                        current = json.loads(result_path.read_text(encoding="utf-8"))
                        if current.get("status") != "running":
                            break
                    except (OSError, json.JSONDecodeError):
                        pass
                if process is not None and process.poll() is not None:
                    break
                time.sleep(0.25)
        finally:
            if log_file is not None:
                log_file.close()

        if not result_path.is_file():
            raise RuntimeError("scenario result was not produced before timeout or process exit")
        report = json.loads(result_path.read_text(encoding="utf-8"))
        if report.get("status") == "running":
            raise RuntimeError(
                f"scenario stalled in phase {report.get('phase', 'unknown')!r}")
        validate_report(
            report, args.inspect, args.vehicle, args.x, args.y,
            args.resolve, args.advance)
        if os.name != "nt":
            owned_processes.update(matching_scenario_process_ids(scenario_executable))
        new_processes = set(owned_processes)
        time.sleep(0.5)
        if os.name == "nt":
            running = process is not None and process.poll() is None
        else:
            running = bool(
                owned_processes & matching_scenario_process_ids(scenario_executable))
        if not running:
            raise RuntimeError("scenario process exited before host cleanup")
        if not args.leave_running:
            if not cleanup_owned_processes(
                    scenario_executable, process, args.wine, wine_prefix):
                raise RuntimeError("scenario processes did not terminate")
            launched = False
        fatal_lines = fatal_diagnostics(log_path.read_text(encoding="utf-8", errors="replace"))
        if fatal_lines:
            raise RuntimeError(f"fatal Wine diagnostic: {fatal_lines[0]}")
        report["preexisting_processes"] = sorted(before)
        report["new_processes"] = sorted(new_processes)
        report["processes_terminated"] = not args.leave_running
        print(json.dumps(report, indent=2, sort_keys=True))
    except (OSError, RuntimeError, subprocess.SubprocessError, json.JSONDecodeError) as error:
        parser.exit(1, f"Gameplay scenario failed: {error}\n")
    finally:
        if scenario_executable is not None and not args.leave_running:
            if os.name != "nt":
                owned_processes.update(matching_scenario_process_ids(scenario_executable))
            if launched or owned_processes:
                cleanup_owned_processes(
                    scenario_executable, process, args.wine, wine_prefix)
            if not matching_scenario_process_ids(scenario_executable):
                scenario_executable.unlink(missing_ok=True)
        if movie_config is not None:
            restore_intro_movie_config(*movie_config)


if __name__ == "__main__":
    main()
