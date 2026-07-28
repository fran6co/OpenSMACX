#!/usr/bin/env python3

import os
from pathlib import Path
import shutil
import signal
import subprocess
import time


def scenario_executable_path(executable, token):
    return executable.with_name(f".{executable.stem}-scenario-{token}.exe")


def stage_scenario_executable(executable, token):
    staged = scenario_executable_path(executable, token)
    try:
        os.link(executable, staged)
    except OSError:
        shutil.copy2(executable, staged)
    return staged


def command_runs_scenario_executable(command, executable):
    # Matching is case-insensitive because the Windows-side path comes back
    # from Wine with arbitrary case. But the LAUNCHER prefix is then stat'ed on
    # the host, so it has to be sliced out of the original string, not the
    # casefolded one: on a case-sensitive filesystem `/opt/Wine/bin/wine`
    # casefolds to a path that does not exist, is_file() says no, and the
    # process goes unrecognised. That difference is invisible on macOS, whose
    # filesystem is case-insensitive by default, and it surfaced the moment
    # this ran on Linux.
    command = command.strip()
    folded = command.casefold()
    native_path = str(executable).casefold()
    windows_path = ("z:" + str(executable).replace("/", "\\")).casefold()
    for marker in (native_path, windows_path):
        if folded == marker or folded.startswith(marker + " "):
            return True
        index = folded.find(marker)
        if index < 0 or (index + len(marker) < len(folded)
                         and not folded[index + len(marker)].isspace()):
            continue
        launcher = Path(command[:index].strip())
        if (launcher.is_absolute() and launcher.is_file()
                and launcher.name.casefold() in ("wine", "wine64",
                                                 "wine-preloader")):
            return True
    return False


def matching_scenario_process_ids(executable):
    if os.name == "nt":
        return set()
    result = subprocess.run(
        ["ps", "-ax", "-o", "pid=,command="], check=True,
        text=True, capture_output=True)
    matches = set()
    for line in result.stdout.splitlines():
        fields = line.strip().split(maxsplit=1)
        if len(fields) != 2:
            continue
        pid = int(fields[0])
        if (pid != os.getpid()
                and command_runs_scenario_executable(fields[1], executable)):
            matches.add(pid)
    return matches


def stop_executable_processes(executable):
    owned = matching_scenario_process_ids(executable)
    for pid in owned:
        try:
            os.kill(pid, signal.SIGTERM)
        except ProcessLookupError:
            pass
    deadline = time.monotonic() + 5.0
    while owned and time.monotonic() < deadline:
        time.sleep(0.25)
        owned = matching_scenario_process_ids(executable)
    for pid in owned:
        try:
            os.kill(pid, signal.SIGKILL)
        except ProcessLookupError:
            pass
    deadline = time.monotonic() + 2.0
    while owned and time.monotonic() < deadline:
        time.sleep(0.25)
        owned = matching_scenario_process_ids(executable)
    return not owned
