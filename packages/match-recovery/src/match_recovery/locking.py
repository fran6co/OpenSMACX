"""Kernel-backed local leases. Lock files contain no recovery truth."""

from contextlib import contextmanager
import errno
import os
from pathlib import Path
import stat
import time


class FileLease:
    """In-process nesting token, not authorization or a transferable lock file."""

    def __init__(self, path):
        self._path = path.resolve()
        self._pid = os.getpid()
        self._active = False

    path = property(lambda self: self._path)
    pid = property(lambda self: self._pid)
    active = property(lambda self: self._active)


@contextmanager
def _lock_stream(path, guarded):
    if not guarded:
        with path.open("a+b") as stream:
            yield stream
        return
    # Resource locks must not silently acquire an alias of another lock inode.
    if path.parent.resolve() != path.parent or path.is_symlink():
        raise ValueError("resource lock requires a canonical path")
    descriptor = os.open(path, os.O_RDWR | os.O_CREAT | getattr(os, "O_NOFOLLOW", 0), 0o600)
    with os.fdopen(descriptor, "r+b") as stream:
        info = os.fstat(stream.fileno())
        current = path.lstat()
        if (not stat.S_ISREG(info.st_mode) or info.st_nlink != 1
                or not stat.S_ISREG(current.st_mode)
                or (info.st_dev, info.st_ino) != (current.st_dev, current.st_ino)):
            raise ValueError("resource lock requires an unaliased regular file")
        yield stream


@contextmanager
def file_lock(path: Path, *, timeout: float = 10, held: FileLease | None = None,
              guarded: bool = False):
    if held is not None:
        if (not isinstance(held, FileLease) or not held.active or held.pid != os.getpid()
                or held.path != path.resolve()):
            raise ValueError("nested operation requires its active local file lease")
        yield held
        return
    if not guarded:
        path.parent.mkdir(parents=True, exist_ok=True)
    with _lock_stream(path, guarded) as stream:
        if os.name == "nt":
            import msvcrt
            stream.write(b"\0")
            stream.flush()

            def acquire():
                stream.seek(0)
                msvcrt.locking(stream.fileno(), msvcrt.LK_NBLCK, 1)

            def release():
                stream.seek(0)
                msvcrt.locking(stream.fileno(), msvcrt.LK_UNLCK, 1)
        else:
            import fcntl

            def acquire():
                fcntl.flock(stream, fcntl.LOCK_EX | fcntl.LOCK_NB)

            def release():
                fcntl.flock(stream, fcntl.LOCK_UN)
        deadline = time.monotonic() + timeout
        while True:
            try:
                acquire()
                break
            except OSError as error:
                if error.errno not in (errno.EACCES, errno.EAGAIN):
                    raise
                if time.monotonic() >= deadline:
                    raise TimeoutError(f"resource is leased: {path.name}") from error
                time.sleep(0.01)
        lease = FileLease(path)
        lease._active = True
        try:
            yield lease
        finally:
            lease._active = False
            release()
