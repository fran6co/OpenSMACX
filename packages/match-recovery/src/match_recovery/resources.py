"""Project-declared resource leases, not compiler installation or recovery state.

The top-level coordinator holds the lease. Children inherit its environment, not
permission to allocate again. An abruptly terminated owner leaves a quarantine
marker: a released kernel lock alone cannot prove its subprocesses have stopped.
"""

from contextlib import contextmanager, ExitStack
import json
import math
import os
from pathlib import Path
import re
import threading
import time
import uuid

from .locking import file_lock
from .records import read_json


LOCK = ".match-recovery-resource.lock"
ACTIVE = ".match-recovery-resource-active.json"
INHERITED = "MATCH_RECOVERY_RESOURCE_LEASE"
_environment_lock = threading.Lock()
_variable = re.compile(r"[A-Za-z_][A-Za-z_0-9]*\Z")


def _expand(value, *, resource=None):
    if not isinstance(value, str) or not value or "\0" in value:
        raise ValueError("resource configuration requires nonempty strings")
    def replacement(match):
        key = match[1]
        if key == "resource" and resource is not None:
            return str(resource)
        if key.startswith("env:") and _variable.fullmatch(key[4:]):
            result = os.environ.get(key[4:])
            if result:
                return result
        raise ValueError("resource configuration has an unavailable placeholder")
    result = re.sub(r"\{([^{}]*)\}", replacement, value)
    if "{" in result or "}" in result:
        raise ValueError("resource configuration has an unresolved placeholder")
    return result


def _slots(configuration, roots):
    if (not isinstance(configuration, dict)
            or set(configuration) != {"schema_version", "slots"}
            or type(configuration["schema_version"]) is not int
            or configuration["schema_version"] != 1
            or not isinstance(configuration["slots"], list) or not configuration["slots"]):
        raise ValueError("resource pool requires schema_version 1 and nonempty slots")
    slots, names, directories, variables = [], set(), set(), None
    for row in configuration["slots"]:
        if not isinstance(row, dict) or set(row) != {"name", "directory", "environment"}:
            raise ValueError("resource slot requires name, directory and environment")
        name = row["name"]
        if not isinstance(name, str) or not re.fullmatch(r"[A-Za-z0-9_-]+", name) or name in names:
            raise ValueError("resource slot names must be distinct identifiers")
        directory = Path(_expand(row["directory"]))
        if (not directory.is_absolute() or directory.resolve() != directory
                or not directory.is_dir() or directory in directories
                or any(directory in other.parents or other in directory.parents for other in directories)):
            raise ValueError("resource directories must exist, be canonical, absolute, distinct and disjoint")
        environment = row["environment"]
        if (not isinstance(environment, dict) or not environment
                or any(not isinstance(key, str) or not _variable.fullmatch(key) for key in environment)):
            raise ValueError("resource environment requires named variables")
        if variables is not None and variables != set(environment):
            raise ValueError("all resource slots must configure the same environment variables")
        variables = set(environment)
        if INHERITED in variables:
            raise ValueError("resource environment uses a reserved variable")
        expanded = {key: _expand(value, resource=directory) for key, value in environment.items()}
        slots.append((name, directory, expanded))
        names.add(name)
        directories.add(directory)
    for root in roots:
        compiler = read_json(root / "recovery/project.json").get("compiler", {})
        declared = compiler.get("environment") if isinstance(compiler, dict) else None
        if (not isinstance(declared, list) or any(not isinstance(key, str) for key in declared)
                or not variables <= {"PATH", *declared}):
            raise ValueError("allocated variables must be declared compiler environment inputs in each checkout")
    return slots


@contextmanager
def resource_environment(configuration, roots, *, timeout=0):
    """Lease one provisioned resource for a whole run; busy pools refuse boundedly.

    Configuration is coordinator-owned. It must assign different actual compiler
    resources to different directories; a generic allocator cannot infer that.
    """
    if type(timeout) not in (int, float) or not math.isfinite(timeout) or not 0 <= timeout <= 3600:
        raise ValueError("resource timeout must be finite and between 0 and 3600 seconds")
    if not roots:
        raise ValueError("resource allocation requires a project checkout")
    if INHERITED in os.environ:
        raise ValueError("nested commands must inherit the allocated resource, not allocate again")
    if not _environment_lock.acquire(blocking=False):
        raise ValueError("resource environment is already allocated in this process")
    try:
        slots = _slots(configuration, roots)
        deadline = time.monotonic() + timeout
        while True:
            for name, directory, environment in slots:
                with ExitStack() as stack:
                    try:
                        stack.enter_context(file_lock(directory / LOCK, timeout=0, guarded=True))
                    except TimeoutError:
                        continue
                    marker = directory / ACTIVE
                    try:
                        descriptor = os.open(marker, os.O_WRONLY | os.O_CREAT | os.O_EXCL, 0o600)
                    except FileExistsError:
                        # Do not infer descendant termination from a stale PID.
                        continue
                    ownership = json.dumps({"schema_version": 1, "pid": os.getpid(), "slot": name,
                                            "nonce": uuid.uuid4().hex}).encode()
                    with os.fdopen(descriptor, "wb") as stream:
                        stream.write(ownership)
                        stream.flush()
                        os.fsync(stream.fileno())
                        identity = os.fstat(stream.fileno())
                    allocated = {**environment, INHERITED: "1"}
                    before = {key: os.environ.get(key) for key in allocated}
                    releasable = True
                    try:
                        os.environ.update(allocated)
                        yield name
                    except BaseException as error:
                        releasable = isinstance(error, Exception)
                        raise
                    finally:
                        for key, value in before.items():
                            if value is None:
                                os.environ.pop(key, None)
                            else:
                                os.environ[key] = value
                        if releasable:
                            current = marker.lstat()
                            if (marker.is_symlink() or current.st_nlink != 1
                                    or (current.st_dev, current.st_ino) != (identity.st_dev, identity.st_ino)
                                    or marker.read_bytes() != ownership):
                                raise ValueError("resource ownership marker changed; preserve it for inspection")
                            marker.unlink()
                    return
            if time.monotonic() >= deadline:
                raise TimeoutError("no resource slot available (busy or interrupted; inspect provisioned slots)")
            time.sleep(min(0.05, max(0, deadline - time.monotonic())))
    finally:
        _environment_lock.release()
