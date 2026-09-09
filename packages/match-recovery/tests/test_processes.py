import os
from pathlib import Path
import signal
import subprocess
import sys
import time

import pytest

from match_recovery.processes import run_bounded, run_parallel


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


def _job(root, code, timeout=5):
    return {"argv": [sys.executable, "-c", code], "cwd": root, "timeout": timeout}


def _logs(root):
    directory = root / "logs"
    directory.mkdir()
    return directory


@pytest.mark.skipif(os.name != "posix", reason="POSIX supervision")
def test_parallel_jobs_reach_a_shared_barrier_and_preserve_binary_logs(tmp_path):
    # Neither can finish unless both have actually started. Not a timing benchmark.
    def code(index):
        return ("from pathlib import Path; import time,sys; "
                f"Path('{index}.ready').touch();\n"
                f"while not Path('{1-index}.ready').exists(): time.sleep(.01)\n"
                f"sys.stdout.buffer.write(bytes([{index},255])); "
                f"sys.stderr.buffer.write(b'\\xfe'); sys.exit({index})")
    events = []
    result = run_parallel([_job(tmp_path, code(i)) for i in range(2)],
                          limit=2, directory=_logs(tmp_path), progress=events.append)
    assert [row["returncode"] for row in result] == [0, 1]
    assert not any(row["timed_out"] for row in result)
    assert [Path(row["stdout"]).read_bytes() for row in result] == [b"\0\xff", b"\1\xff"]
    assert all(Path(row["stderr"]).read_bytes() == b"\xfe" for row in result)
    assert [row["event"] for row in events[:2]] == ["started", "started"]


@pytest.mark.skipif(os.name != "posix", reason="POSIX supervision")
def test_parallel_limit_queues_jobs_and_continues_after_failure(tmp_path):
    events = []
    result = run_parallel([_job(tmp_path, "import sys; sys.exit(7)"),
                           _job(tmp_path, "print('next')")],
                          limit=1, directory=_logs(tmp_path), progress=events.append)
    assert [(row["job"], row["event"]) for row in events] == [
        (0, "started"), (0, "finished"), (1, "started"), (1, "finished")]
    assert [row["returncode"] for row in result] == [7, 0]


@pytest.mark.skipif(sys.platform != "linux", reason="Linux descendant liveness assertions")
def test_parallel_timeout_unwinds_nested_compiler_and_runs_next_job(tmp_path):
    child = ("import os,signal,time; from pathlib import Path; "
             "signal.signal(signal.SIGTERM, signal.SIG_IGN); "
             "Path('child.pid').write_text(str(os.getpid())); time.sleep(60)")
    nested = ("from pathlib import Path; import sys; from match_recovery.processes import run_bounded; "
              f"run_bounded([sys.executable,'-c',{child!r}], cwd=Path.cwd(),timeout=30)")
    unrelated = subprocess.Popen([sys.executable, "-c", "import time; time.sleep(60)"],
                                 start_new_session=True)
    before = signal.getsignal(signal.SIGTERM)
    try:
        result = run_parallel([_job(tmp_path, nested, timeout=1), _job(tmp_path, "pass")],
                              limit=1, directory=_logs(tmp_path))
        assert result[0]["timed_out"] and result[0]["returncode"] != 0
        assert result[1]["returncode"] == 0 and not result[1]["timed_out"]
        _assert_stopped(int((tmp_path / "child.pid").read_text()))
        assert unrelated.poll() is None
        assert signal.getsignal(signal.SIGTERM) == before
    finally:
        unrelated.kill()
        unrelated.wait(timeout=5)


@pytest.mark.skipif(sys.platform != "linux", reason="Linux descendant liveness assertions")
@pytest.mark.parametrize("failure", ["startup", "callback"])
def test_parallel_supervisor_failure_stops_running_jobs(tmp_path, failure):
    events = []
    def progress(event):
        events.append(event)
        if failure == "callback":
            raise RuntimeError("observer failed")
    jobs = [_job(tmp_path, "import time; time.sleep(60)"),
            {"argv": [str(tmp_path / "missing-program")], "cwd": tmp_path, "timeout": 5}]
    with pytest.raises(RuntimeError if failure == "callback" else FileNotFoundError):
        run_parallel(jobs, limit=2, directory=_logs(tmp_path), progress=progress)
    _assert_stopped(events[0]["pid"])


@pytest.mark.parametrize("damage", ["limit", "boolean", "empty", "timeout", "nan", "argv", "cwd", "extra"])
def test_parallel_refuses_invalid_requests_before_launch(tmp_path, damage):
    jobs, limit = [_job(tmp_path, "raise AssertionError('must not run')")], 1
    if damage == "limit":
        limit = 33
    elif damage == "boolean":
        limit = True
    elif damage == "empty":
        jobs = []
    else:
        jobs[0].update({"timeout": {"timeout": 0, "nan": float("nan")}.get(damage, 5)})
        if damage == "argv":
            jobs[0]["argv"] = ["bad\0command"]
        elif damage == "cwd":
            jobs[0]["cwd"] = str(tmp_path)
        elif damage == "extra":
            jobs[0]["shell"] = True
    logs = _logs(tmp_path)
    with pytest.raises(ValueError):
        run_parallel(jobs, limit=limit, directory=logs)
    assert not list(logs.iterdir())


def test_parallel_never_overwrites_previous_logs(tmp_path):
    logs = _logs(tmp_path)
    (logs / "0.stdout").write_bytes(b"previous")
    with pytest.raises(ValueError):
        run_parallel([_job(tmp_path, "pass")], limit=1, directory=logs)
    assert (logs / "0.stdout").read_bytes() == b"previous"


@pytest.mark.skipif(sys.platform != "linux", reason="Linux descendant liveness assertions")
@pytest.mark.parametrize("nested", [False, True])
def test_terminated_parallel_supervisor_finishes_cleanup_before_outer_kill(tmp_path, nested):
    child = ("import os,signal,time; from pathlib import Path; "
             "signal.signal(signal.SIGTERM, signal.SIG_IGN); "
             "Path('compiler.pid').write_text(str(os.getpid())); time.sleep(60)")
    if nested:
        child = ("import sys; from pathlib import Path; from match_recovery.processes import run_bounded; "
                 f"run_bounded([sys.executable,'-c',{child!r}],cwd=Path.cwd(),timeout=30)")
    supervisor = ("import sys; from pathlib import Path; from match_recovery.processes import run_parallel; "
                  "logs=Path.cwd()/'logs'; logs.mkdir(); "
                  f"run_parallel([dict(argv=[sys.executable,'-c',{child!r}],cwd=Path.cwd(),timeout=30)],"
                  "limit=1,directory=logs)")
    before = signal.getsignal(signal.SIGTERM)
    try:
        with pytest.raises(subprocess.TimeoutExpired):
            run_bounded([sys.executable, "-c", supervisor], cwd=tmp_path, timeout=1)
        _assert_stopped(int((tmp_path / "compiler.pid").read_text()))
        assert signal.getsignal(signal.SIGTERM) == before
    finally:
        path = tmp_path / "compiler.pid"
        if path.exists():
            try:
                os.kill(int(path.read_text()), signal.SIGKILL)
            except ProcessLookupError:
                pass
