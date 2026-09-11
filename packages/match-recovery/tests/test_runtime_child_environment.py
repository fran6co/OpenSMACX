"""Real nested interpreters must not mutate a selected installed environment."""

import os
import sys

import pytest

from match_recovery import framework_runtime as runtime, processes
from test_framework_runtime import installed, write_binding


@pytest.mark.parametrize("parallel", [False, True])
def test_nested_interpreter_inherits_cache_control_without_changing_parent(tmp_path, installed, monkeypatch, parallel):
    path = write_binding(tmp_path, installed)
    python, package, _ = installed
    monkeypatch.delenv("PYTHONDONTWRITEBYTECODE", raising=False)
    before = dict(os.environ)
    # The intermediary is analogous to a build tool: the selected Python child
    # has no -B argument. Its startup imports _virtualenv and its body imports
    # the package, so both would write caches without inherited suppression.
    script = ("import subprocess, sys; "
              f"subprocess.run([{str(python)!r}, '-c', "
              "'import sys, match_recovery; assert sys.dont_write_bytecode; print(42)'], check=True)")
    argv = [sys.executable, "-c", script]
    with runtime.binding(path):
        identity = runtime.fingerprint()
        if parallel:
            logs = tmp_path / "logs"
            logs.mkdir()
            reports = processes.run_parallel([dict(argv=argv, cwd=tmp_path, timeout=30)] * 2,
                                            limit=2, directory=logs)
            assert all(row["returncode"] == 0 for row in reports)
        else:
            result = processes.run_bounded(argv, cwd=tmp_path, timeout=30)
            assert result.returncode == 0, result.stderr.decode()
            assert result.stdout.strip() == b"42"
        assert runtime.fingerprint() == identity
        assert not list(package.parent.rglob("*.pyc"))
        assert dict(os.environ) == before
    assert dict(os.environ) == before


def test_inactive_runtime_leaves_child_environment_inherited(monkeypatch):
    monkeypatch.setenv("PYTHONDONTWRITEBYTECODE", "custom-parent-value")
    assert runtime.child_environment() is None
