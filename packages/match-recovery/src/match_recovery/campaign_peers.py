"""Freeze sibling ownership while a campaign coordinates one retained worker.

The caller validates assignments/retirements and protects coordinator ownership.
This epoch preserves sibling WIP; leases and snapshots are not acceptance proof.
"""

from contextlib import ExitStack, contextmanager
from pathlib import Path

from .checkpoints import preflight
from .dispatch import _checkout
from .locking import file_lock
from .scheduling import _state as reservation_state
from .tasks import TaskStore, task_name


@contextmanager
def peer_epoch(root, current_task, jobs):
    """Yield a freshness check with existing sibling driver leases held.

    Already collected absent peers may be skipped, but cannot reappear unnoticed.
    Final validation also runs on exceptional exit, before releasing any lease.
    """
    root = Path(root)
    task_name(current_task)
    common, _ = _checkout(root)
    peers, names, paths = {}, set(), set()
    for job in jobs:
        name = task_name(job["task"])
        path = Path(job["root"])
        if name in names or path in paths:
            raise ValueError("campaign peer assignments must be unique")
        names.add(name)
        paths.add(path)
        if name == current_task:
            continue
        if not path.is_absolute() or path.resolve() != path or path == root:
            raise ValueError("campaign peers require distinct canonical worktrees")
        peers[path] = name
    with ExitStack() as leases:
        states, checkouts, absent = {}, {}, set()
        for path, name in sorted(peers.items()):
            if not path.exists():
                if path.is_symlink():
                    raise ValueError("campaign peer cannot be a dangling worktree alias")
                absent.add(path)
                continue
            checkout = _checkout(path)
            if checkout[0] != common:
                raise ValueError("campaign peer belongs to another repository")
            leases.enter_context(file_lock(path / "build/recovery/driver.lock", timeout=0))
            if preflight(path) != checkout[1]:
                raise ValueError("campaign peer branch changed while acquiring its lease")
            TaskStore(path).scope(name)
            states[path] = reservation_state(path)
            checkouts[path] = checkout

        def fresh():
            if any(path.exists() or path.is_symlink() for path in absent):
                raise ValueError("collected campaign peer reappeared; preserve work and replan")
            for path, state in states.items():
                if (_checkout(path) != checkouts[path] or preflight(path) != checkouts[path][1]
                        or reservation_state(path) != state):
                    raise ValueError("campaign peer state or scope changed; preserve work and replan")

        fresh()
        try:
            yield fresh
        finally:
            fresh()
