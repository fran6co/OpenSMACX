"""Parallel launch boundaries must not execute changed runtime content."""

import json
import subprocess
import sys

import pytest

from match_recovery import framework_runtime as runtime, processes
from test_framework_runtime import installed, write_binding


@pytest.mark.parametrize("damage", ["binding", "package", "interpreter"])
def test_parallel_rechecks_before_second_launch_and_cleans_first(tmp_path, installed, monkeypatch, damage):
    path = write_binding(tmp_path, installed)
    python, package, _ = installed
    logs = tmp_path / "logs"
    logs.mkdir()
    children = []
    real_popen = subprocess.Popen

    def capture(*args, **kwargs):
        child = real_popen(*args, **kwargs)
        children.append(child)
        return child

    def progress(event):
        if event["event"] != "started":
            return
        if damage == "binding":
            value = json.loads(path.read_text())
            value["launchers"] = [["recover"]]
            path.write_text(json.dumps(value))
        elif damage == "package":
            (package / "unexpected.py").write_text("# changed\n")
        else:
            python.unlink()  # Test-owned venv symlink, not the shared interpreter.
            python.write_text("#!/bin/sh\nexit 77\n")
            python.chmod(0o755)

    with pytest.raises(ValueError):
        with runtime.binding(path):
            monkeypatch.setattr(subprocess, "Popen", capture)
            processes.run_parallel([
                dict(argv=[sys.executable, "-c", "import time; time.sleep(60)"], cwd=tmp_path, timeout=65),
                dict(argv=runtime.native_argv(["--help"]), cwd=tmp_path, timeout=30),
            ], limit=2, directory=logs, progress=progress)
    assert len(children) == 1
    assert children[0].poll() is not None


def test_parallel_rechecks_after_final_progress_callback(tmp_path, installed):
    path = write_binding(tmp_path, installed)
    _, package, _ = installed
    logs = tmp_path / "logs"
    logs.mkdir()

    def progress(event):
        if event["event"] == "finished":
            (package / "changed.py").write_text("# changed\n")

    with pytest.raises(ValueError):
        with runtime.binding(path):
            processes.run_parallel([
                dict(argv=[sys.executable, "-c", "pass"], cwd=tmp_path, timeout=30),
            ], limit=1, directory=logs, progress=progress)
