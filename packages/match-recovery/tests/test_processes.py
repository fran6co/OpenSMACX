import os
from pathlib import Path
import signal
import subprocess
import sys
import time

import pytest

from match_recovery.processes import run_bounded


def test_capture_is_binary_and_preserves_exit_status(tmp_path):
    result = run_bounded([sys.executable, "-c",
                          "import sys; sys.stdout.buffer.write(sys.stdin.buffer.read()); "
                          "sys.stderr.buffer.write(b'\\xff'); sys.exit(7)"],
                         cwd=tmp_path, timeout=5, input=b"not a shell $(command)")
    assert result.returncode == 7
    assert result.stdout == b"not a shell $(command)" and result.stderr == b"\xff"


def _assert_stopped(pid):
    # Linux can retain an orphan as a zombie until its init reaps it. That is
    # stopped work, not a live compiler. Use a bounded wait for signal delivery.
    deadline = time.monotonic() + 2
    while time.monotonic() < deadline:
        try:
            state = Path(f"/proc/{pid}/stat").read_text().rsplit(")", 1)[1].split()[0]
        except (FileNotFoundError, ProcessLookupError):
            # procfs can return ESRCH if the process disappears after open but
            # before read, rather than ENOENT when the path is already absent.
            return
        if state == "Z":
            return
        time.sleep(0.01)
    pytest.fail(f"command descendant {pid} is still running")


@pytest.mark.parametrize("error", [FileNotFoundError, ProcessLookupError])
def test_liveness_probe_accepts_both_disappearance_races(monkeypatch, error):
    def disappeared(*_):
        raise error("process disappeared")
    monkeypatch.setattr(Path, "read_text", disappeared)
    _assert_stopped(123)


def test_liveness_probe_does_not_treat_permission_failure_as_stopped(monkeypatch):
    def denied(*_):
        raise PermissionError("cannot inspect process")
    monkeypatch.setattr(Path, "read_text", denied)
    with pytest.raises(PermissionError):
        _assert_stopped(123)


@pytest.mark.skipif(sys.platform != "linux", reason="Linux /proc liveness assertions")
@pytest.mark.parametrize("nested", [False, True])
def test_timeout_cleans_descendants_including_nested_compiler_group(tmp_path, nested):
    pidfile = tmp_path / "child.pid"
    child = ("import os,signal,time; from pathlib import Path; "
             "signal.signal(signal.SIGTERM, signal.SIG_IGN); "
             f"Path({str(pidfile)!r}).write_text(str(os.getpid())); time.sleep(60)")
    parent = ("import subprocess,sys,time; "
              f"subprocess.Popen([sys.executable, '-c', {child!r}]); time.sleep(60)")
    if nested:
        parent = ("import sys; from pathlib import Path; "
                  "from match_recovery.processes import run_bounded; "
                  f"run_bounded([sys.executable, '-c', {parent!r}], "
                  "cwd=Path.cwd(), timeout=30)")
    unrelated = subprocess.Popen([sys.executable, "-c", "import time; time.sleep(60)"],
                                 start_new_session=True)
    try:
        before = signal.getsignal(signal.SIGTERM)
        start = time.monotonic()
        with pytest.raises(subprocess.TimeoutExpired):
            run_bounded([sys.executable, "-c", parent], cwd=tmp_path, timeout=1)
        assert time.monotonic() - start < 8
        assert signal.getsignal(signal.SIGTERM) == before
        _assert_stopped(int(pidfile.read_text()))
        assert unrelated.poll() is None
    finally:
        unrelated.kill()
        unrelated.wait(timeout=5)
        if pidfile.exists():
            try:
                os.kill(int(pidfile.read_text()), signal.SIGKILL)
            except ProcessLookupError:
                pass
