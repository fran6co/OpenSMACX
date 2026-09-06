"""Bounded command execution and cooperative cleanup of nested commands.

Not a sandbox: descendants that deliberately detach require external job
containment. Never stop a shared Wine server as part of command cleanup.
"""

from contextlib import contextmanager
import os
from pathlib import Path
import signal
import subprocess
import threading


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
