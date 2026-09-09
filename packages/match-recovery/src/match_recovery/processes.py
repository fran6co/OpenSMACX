"""Bounded command execution and cooperative cleanup of nested commands.

Not a sandbox: descendants that deliberately detach require external job
containment. Never stop a shared Wine server as part of command cleanup.
"""

from contextlib import contextmanager
import math
import os
from pathlib import Path
import signal
import subprocess
import threading
import time


class _TerminationRequested(BaseException):
    """Not InterruptedError: selectors intentionally swallow/retry that type."""


@contextmanager
def _termination_cleanup():
    # A containing runner sends TERM before KILL. Turn that TERM into unwinding
    # so a Python checker can clean up its separately grouped compiler too.
    # Do not replace an application's explicit handler or install from a thread.
    active = (os.name != "nt" and threading.current_thread() is threading.main_thread()
              and signal.getsignal(signal.SIGTERM) == signal.SIG_DFL)

    def interrupted(signum, frame):
        raise _TerminationRequested("command interrupted by SIGTERM")

    if active:
        signal.signal(signal.SIGTERM, interrupted)
    try:
        yield
    finally:
        if active:
            signal.signal(signal.SIGTERM, signal.SIG_DFL)


def _signal(process, signum):
    try:
        if os.name != "nt":
            os.killpg(process.pid, signum)
        elif signum == signal.SIGTERM:
            process.terminate()
        else:
            process.kill()
    except ProcessLookupError:
        pass


def run_bounded(argv: list[str], *, cwd: Path, timeout: float,
                input: bytes | None = None) -> subprocess.CompletedProcess:
    """Capture bytes; preserve failure/timeout, with bounded process cleanup.

    POSIX commands own a process group. Timeout/interruption first requests
    cleanup, then kills the remaining group. Native Windows descendant cleanup
    must be supplied by an external job/sandbox; only its direct child is killed.
    """
    if timeout <= 0:
        raise ValueError("command timeout must be positive")
    with _termination_cleanup():
        process = subprocess.Popen(argv, cwd=cwd, stdin=subprocess.PIPE,
                                   stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                                   start_new_session=os.name != "nt")
        try:
            output, errors = process.communicate(input, timeout=timeout)
        except BaseException as error:
            try:
                if not isinstance(error, _TerminationRequested):
                    _signal(process, signal.SIGTERM)
                    process.communicate(timeout=2)
            except subprocess.TimeoutExpired:
                pass
            finally:
                # A containing runner's grace period is already running on TERM;
                # do not spend another grace period before killing our compiler.
                # A terminated leader can still leave children in its group.
                _signal(process, signal.SIGKILL if os.name != "nt" else signal.SIGTERM)
            try:
                process.communicate(timeout=2)
            except subprocess.TimeoutExpired:
                # Detached descendants may retain pipes; never wait indefinitely.
                for stream in (process.stdin, process.stdout, process.stderr):
                    if stream is not None:
                        stream.close()
                process.wait(timeout=2)
            raise
        return subprocess.CompletedProcess(argv, process.returncode, output, errors)


def run_parallel(jobs: list[dict], *, limit: int, directory: Path, progress=None) -> list[dict]:
    """Supervise bounded independent POSIX jobs, preserving file-backed logs.

    Each job contains argv, cwd and timeout. This is process supervision, not
    recovery acceptance, compiler resource allocation or a sandbox. Callers own
    scope/worktree validation and supply a new ignored artifact directory. A
    timeout or interruption unwinds cooperative nested command groups; detached
    descendants still require external containment and quarantined resources.
    """
    if os.name != "posix":
        raise ValueError("parallel command supervision currently requires POSIX process groups")
    if type(limit) is not int or not 1 <= limit <= 32 or not isinstance(jobs, list) or not jobs:
        raise ValueError("parallel commands require nonempty jobs and a limit in 1..32")
    for job in jobs:
        if (not isinstance(job, dict) or set(job) != {"argv", "cwd", "timeout"}
                or not isinstance(job["argv"], list) or not job["argv"]
                or any(not isinstance(word, str) or not word or "\0" in word for word in job["argv"])
                or type(job["timeout"]) not in (int, float) or not math.isfinite(job["timeout"])
                or not 0 < job["timeout"] <= 86400):
            raise ValueError("invalid parallel command")
        if not isinstance(job["cwd"], Path) or not job["cwd"].is_dir() or job["cwd"].resolve() != job["cwd"]:
            raise ValueError("parallel command cwd must be a canonical existing directory")
    directory = Path(directory)
    if not directory.is_dir() or directory.resolve() != directory or any(directory.iterdir()):
        raise ValueError("parallel command logs require a new empty canonical directory")
    active, results, next_job = {}, [None] * len(jobs), 0

    def stop(entries, *, grace=2):
        for process, _, _ in entries:
            _signal(process, signal.SIGTERM)
        deadline = time.monotonic() + grace
        while any(process.poll() is None for process, _, _ in entries) and time.monotonic() < deadline:
            time.sleep(0.02)
        for process, _, _ in entries:
            # A leader that already exited can still own live group children.
            _signal(process, signal.SIGKILL)
        for process, _, _ in entries:
            process.wait(timeout=2)

    def announce(index, event, **details):
        if progress is not None:
            progress({"job": index, "event": event, **details})

    with _termination_cleanup():
        try:
            while next_job < len(jobs) or active:
                while next_job < len(jobs) and len(active) < limit:
                    index = next_job
                    paths = [directory / (str(index) + suffix) for suffix in (".stdout", ".stderr")]
                    with paths[0].open("xb") as output, paths[1].open("xb") as errors:
                        process = subprocess.Popen(jobs[index]["argv"], cwd=jobs[index]["cwd"],
                                                   stdin=subprocess.DEVNULL, stdout=output, stderr=errors,
                                                   start_new_session=True)
                    active[index] = (process, time.monotonic(), paths)
                    next_job += 1
                    announce(index, "started", pid=process.pid)
                for index, entry in list(active.items()):
                    process, started, paths = entry
                    timed_out = process.poll() is None and time.monotonic() - started >= jobs[index]["timeout"]
                    if timed_out:
                        stop([entry])
                    if process.poll() is not None:
                        results[index] = {"returncode": process.returncode, "timed_out": timed_out,
                                          "stdout": str(paths[0]), "stderr": str(paths[1])}
                        del active[index]
                        announce(index, "finished", returncode=process.returncode, timed_out=timed_out)
                if active:
                    time.sleep(0.02)
        except BaseException as error:
            # A containing runner has already started its own two-second grace
            # period. Leave time for our cleanup before it kills this supervisor.
            stop(list(active.values()), grace=0.5 if isinstance(error, _TerminationRequested) else 2)
            raise
    return results
