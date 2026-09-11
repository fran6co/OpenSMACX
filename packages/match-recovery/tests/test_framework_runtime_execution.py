"""Pinned runtime identity survives native command and review boundaries."""

import json
import os
from pathlib import Path
import subprocess
import sys

import pytest

from match_recovery import framework_runtime as runtime, processes
from match_recovery.driver import Command, changes, invoke, working_state
from match_recovery.fingerprint import input_snapshot
from match_recovery.gate import _command
from match_recovery.resources import ACTIVE, resource_environment
from match_recovery.reviews import ReviewStore
from test_framework_runtime import installed, write_binding
from test_resources import pool
from test_source_configuration import configured
from test_task_source import project


def old_launcher(root, monkeypatch):
    directory = root / "old-bin"
    directory.mkdir()
    marker = directory / "old-runtime-executed"
    executable = directory / "uv"
    executable.write_text(f"#!/bin/sh\ntouch '{marker}'\nexit 77\n")
    executable.chmod(0o755)
    monkeypatch.setenv("PATH", str(directory) + os.pathsep + os.environ["PATH"])
    return marker


@pytest.mark.parametrize("boundary", ["driver", "gate"])
def test_native_driver_and_gate_commands_use_selected_install(configured, installed, monkeypatch, boundary):
    path = write_binding(configured, installed)
    marker = old_launcher(configured, monkeypatch)
    command = Command(("uv", "run", "recover", "inventory", "--root", "{root}"), 30)
    with runtime.binding(path):
        if boundary == "driver":
            code, report = invoke(command, configured, "unused-task", {"ignored_stdin": True})
            assert code == 0
        else:
            result = _command(configured, configured / "unused-image", command)
            assert result["result"] == "pass", result
            report = json.loads(result["stdout"])
        assert report["entities"] == 1 and report["annotations"] == 2
        assert report["verification"] == "not_measured"
    assert not marker.exists()


def test_driver_expansion_input_and_timeout_are_unchanged(tmp_path, installed, monkeypatch):
    path = write_binding(tmp_path, installed)
    calls = []

    def execute(argv, **kwargs):
        calls.append((argv, kwargs))
        return subprocess.CompletedProcess(argv, 3, b'{"result":"unknown"}', b"diagnostic")

    monkeypatch.setattr(processes, "_run_bounded", execute)
    with runtime.binding(path):
        command = Command(("uv", "run", "recover", "tasks", "context", "{task}", "--root", "{root}"), 37)
        assert invoke(command, tmp_path, "selected-task", {"evidence": ["source"]}) == (3, {"result": "unknown"})
        expected = runtime.native_argv(["tasks", "context", "selected-task", "--root", str(tmp_path)])
    assert len(calls) == 1
    assert calls[0][0] == expected
    assert calls[0][1] == {"cwd": tmp_path, "timeout": 37, "input": b'{"evidence":["source"]}\n'}


@pytest.mark.parametrize("boundary", ["process", "driver", "gate"])
@pytest.mark.parametrize("damage", ["binding", "package"])
def test_command_cannot_return_success_after_mutating_runtime(tmp_path, installed, boundary, damage):
    path = write_binding(tmp_path, installed)
    if damage == "binding":
        script = (
            "import json,sys; from pathlib import Path; p=Path(sys.argv[1]); "
            "value=json.loads(p.read_text()); value['unexpected']='mutation'; "
            "p.write_text(json.dumps(value)); print('{}')"
        )
        target = path
    else:
        script = (
            "import sys; from pathlib import Path; p=Path(sys.argv[1]); "
            "p.write_text(p.read_text()+'\\n# child changed runtime\\n'); print('{}')"
        )
        target = installed[1] / "inspection.py"
    argv = [sys.executable, "-I", "-B", "-c", script, str(target)]
    with pytest.raises(ValueError, match="changed"):
        with runtime.binding(path):
            if boundary == "process":
                processes.run_bounded(argv, cwd=tmp_path, timeout=30)
            elif boundary == "driver":
                invoke(Command(tuple(argv), 30), tmp_path, "unused", {})
            else:
                _command(tmp_path, tmp_path / "unused-image", Command(tuple(argv), 30))
    if damage == "binding":
        assert json.loads(path.read_text())["unexpected"] == "mutation"
    else:
        assert "child changed runtime" in target.read_text()


@pytest.mark.parametrize("kind", ["timeout", "oserror", "interrupt", "termination"])
def test_failed_command_keeps_original_exception_and_resource_quarantine(pool, installed, monkeypatch, kind):
    root, configuration = pool
    path = write_binding(root, installed)
    errors = {"timeout": subprocess.TimeoutExpired(["provider"], 17),
              "oserror": OSError("command unavailable"),
              "interrupt": KeyboardInterrupt("operator interruption"),
              "termination": processes._TerminationRequested("supervisor termination")}
    original = errors[kind]

    def execute(*args, **kwargs):
        value = json.loads(path.read_text())
        value["unexpected"] = "concurrent change"
        path.write_text(json.dumps(value))
        raise original

    monkeypatch.setattr(processes, "_run_bounded", execute)
    with pytest.raises(type(original)) as failure:
        with runtime.binding(path):
            with resource_environment(configuration, [root]):
                processes.run_bounded(["provider"], cwd=root, timeout=17)
    assert failure.value is original
    marker = Path(configuration["slots"][0]["directory"]) / ACTIVE
    assert marker.exists() == (kind in {"interrupt", "termination"})


def test_swallowed_runtime_failure_cannot_leave_binding_normally(tmp_path, installed):
    path = write_binding(tmp_path, installed)
    with pytest.raises(ValueError, match="binding changed"):
        with runtime.binding(path):
            value = json.loads(path.read_text())
            value["unexpected"] = "mutation"
            path.write_text(json.dumps(value))
            with pytest.raises(ValueError, match="binding changed"):
                runtime.resolve(["uv", "run", "recover", "inventory"])
    assert runtime.fingerprint() is None


def test_real_subprocess_timeout_remains_timeout(tmp_path, installed):
    path = write_binding(tmp_path, installed)
    with runtime.binding(path):
        with pytest.raises(subprocess.TimeoutExpired):
            processes.run_bounded([sys.executable, "-I", "-B", "-c", "import time; time.sleep(30)"],
                                  cwd=tmp_path, timeout=0.05)
        assert runtime.fingerprint() is not None


def test_input_and_review_snapshots_bind_the_runtime_even_with_identical_source(project, installed):
    path = write_binding(project, installed)
    files = ["code/owner.cc", "code/owner.hpp", "recovery/project.json"]
    configuration = {"measurement": "synthetic configuration"}
    before_input = input_snapshot(project, files, configuration)
    before_working = working_state(project)
    reviews = ReviewStore(project, "owner")
    before_review = reviews.snapshot(before_working, configuration)
    with runtime.binding(path):
        active_input = input_snapshot(project, files, configuration)
        active_working = working_state(project)
        active_review = reviews.snapshot(active_working, configuration)
        assert active_input["framework_runtime"] == runtime.fingerprint()
        assert active_working["framework_runtime"] == runtime.fingerprint()
        assert active_input["inputs"] == before_input["inputs"]
        assert active_working["files"] == before_working["files"]
        assert active_input != before_input and active_review != before_review
        with pytest.raises(ValueError, match="runtime changed"):
            changes(before_working, active_working)
    assert input_snapshot(project, files, configuration) == before_input
    assert reviews.snapshot(working_state(project), configuration) == before_review
