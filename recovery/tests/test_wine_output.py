"""Compiler diagnostics must not wait for unrelated Wine service pipe owners."""

import os
import signal
import subprocess
import sys

import pytest

from recovery.environment import wine


@pytest.mark.parametrize("code", [0, 1, 3])
def test_tool_preserves_status_and_both_diagnostic_streams(code):
    result = wine.run_tool([sys.executable, "-c",
                           "import os,sys; os.write(1,b'output\\r\\n'); "
                           "os.write(2,b'error\\xff\\r\\n'); sys.exit(int(sys.argv[1]))", str(code)])
    assert result.returncode == code
    assert result.stdout == "output\n"
    assert result.stderr == "error\ufffd\n"


@pytest.mark.skipif(os.name != "posix", reason="models inherited Wine descriptors using fork")
def test_tool_returns_while_background_output_owner_is_alive(tmp_path, monkeypatch):
    pid_file = tmp_path / "background.pid"
    original_run = subprocess.run

    def bounded(*args, **kwargs):
        # A PIPE-based implementation times out: the background process owns
        # both output descriptors until our finally clause terminates it.
        return original_run(*args, timeout=5, **kwargs)

    monkeypatch.setattr(wine.subprocess, "run", bounded)
    script = ("import os,signal,sys\n"
              "pid=os.fork()\n"
              "if pid == 0:\n"
              "    signal.pause()\n"
              "    os._exit(0)\n"
              "with open(sys.argv[1], 'w') as stream: stream.write(str(pid))\n"
              "os.write(1,b'compiler output\\n')\n"
              "os.write(2,b'compiler warning\\n')\n"
              "os._exit(7)\n")
    try:
        result = wine.run_tool([sys.executable, "-c", script, str(pid_file)])
        assert result.returncode == 7
        assert result.stdout == "compiler output\n"
        assert result.stderr == "compiler warning\n"
        os.kill(int(pid_file.read_text()), 0)
    finally:
        if pid_file.exists():
            os.kill(int(pid_file.read_text()), signal.SIGTERM)


def test_missing_tool_is_not_reported_as_success():
    with pytest.raises(FileNotFoundError):
        wine.run_tool(["/nonexistent/recovery-test-compiler"])
