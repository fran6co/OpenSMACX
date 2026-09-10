"""Derived reservations for preserved work outside a campaign.

Policy names parked checkouts/tasks, never copies their scope or measurements.
Unaccounted recovery work refuses scheduling. No checkout is adopted or repaired.
"""

from contextlib import ExitStack, contextmanager
import hashlib
import os
from pathlib import Path, PurePosixPath

from .components import relative_path
from .dispatch import _checkout
from .driver import _resumable_history, working_state
from .git_worktree import committed_files
from .locking import file_lock
from .model import digest, require_text
from .reviews import ReviewStore
from .source_config import SourceConfiguration
from .source_history import git
from .tasks import TaskStore, task_name


def validate_reservations(rows):
    if not isinstance(rows, list):
        raise ValueError("reserved_worktrees must be an array")
    paths = set()
    for row in rows:
        if not isinstance(row, dict) or set(row) != {"path", "tasks", "evidence"}:
            raise ValueError("reserved worktree needs path, tasks and evidence")
        require_text(row["path"], "reserved worktree path")
        if not Path(row["path"]).is_absolute():
            relative_path(row["path"])
        if row["path"] in paths:
            raise ValueError("duplicate reserved worktree")
        paths.add(row["path"])
        for key in ("tasks", "evidence"):
            if (not isinstance(row[key], list) or not row[key]
                    or any(not isinstance(item, str) for item in row[key])
                    or len(set(row[key])) != len(row[key])):
                raise ValueError("reserved worktree needs nonempty unique task/evidence arrays")
            for item in row[key]:
                task_name(item) if key == "tasks" else require_text(item, "reservation evidence")


def _worktrees(root):
    blocks = git(root, "worktree", "list", "--porcelain", "-z").split(b"\0\0")
    result = {}
    for block in filter(None, blocks):
        fields = block.rstrip(b"\0").split(b"\0")
        if not fields[0].startswith(b"worktree "):
            raise ValueError("unsupported worktree inventory")
        path = Path(os.fsdecode(fields[0][9:]))
        if path in result or path.resolve() != path or not path.is_dir():
            raise ValueError("worktree inventory needs canonical available checkouts")
        result[path] = tuple(fields[1:])
    return result


def _token(state):
    return digest({"head": state["head"].hex(), "index": state["index"].hex(), "files": state["files"]})


def _state(root):
    state = working_state(root)
    config = SourceConfiguration.read(root)
    # Ignore rules are for caches, not a way to hide original bindings or an
    # appended scope decision from scheduling/freshness checks.
    paths = set(config.files(root)) | {root / config.image_pin}
    for name in ("recovery/tasks", "recovery/history"):
        directory = root / name
        if directory.resolve() != directory:
            raise ValueError("external recovery decisions require canonical directories")
        paths.update(directory.rglob("*"))
    for path in paths:
        if path.resolve() != path or path.is_symlink():
            raise ValueError("external recovery inputs require canonical files")
        if path.is_file():
            state["files"][path.relative_to(root).as_posix()] = (path.stat().st_mode, hashlib.sha256(path.read_bytes()).hexdigest())
    return state


def _delta(root, child, state):
    # Compare the worker side of divergence, not changes made only on master.
    base = git(root, "merge-base", "HEAD", state["head"].decode().strip()).decode().strip()
    entries, blobs = committed_files(root, base)
    original = {name: (("symlink", os.fsdecode(blobs[blob])) if mode == "120000" else
                       (mode, hashlib.sha256(blobs[blob]).hexdigest()))
                for name, (mode, _, blob) in entries.items()}
    current = {name: (value if value is None or value[0] == "symlink" else
                      ("100755" if value[0] & 0o111 else "100644", value[1]))
               for name, value in state["files"].items()}
    changed = {name for name in original.keys() | current.keys() if original.get(name) != current.get(name)}
    # A dirty reversal must not cancel unmerged committed work, nor may working
    # bytes restored after staging hide an outstanding index change.
    for revisions in ((base, state["head"].decode().strip()), ("--cached", "HEAD")):
        changed.update(os.fsdecode(name) for name in git(child, "diff", "--name-only", "--no-renames",
                        "--no-ext-diff", "--no-textconv", "-z", *revisions, "--").split(b"\0") if name)
    return changed


class Reservations:
    def __init__(self, root, rows, managed, stack):
        self.root, self.managed, self.stack = root, managed, stack
        self.rows = {}
        for row in rows:
            path = Path(row["path"])
            path = path if path.is_absolute() else root / path
            if path == root or path.resolve() != path or path in self.rows:
                raise ValueError("reserved worktree must be a distinct canonical checkout")
            self.rows[path] = row
        self.inventory = _worktrees(root)
        if not self.rows.keys() <= self.inventory.keys() or self.rows.keys() & managed:
            raise ValueError("reserved worktree must be registered and outside campaign assignments")
        common, _ = _checkout(root)
        self.states, self.records = {}, []
        for child in sorted(self.inventory):
            if child == root or child in managed:
                continue
            if _checkout(child)[0] != common:
                raise ValueError("reserved worktree belongs to another repository")
            stack.enter_context(file_lock(child / "build/recovery/driver.lock", timeout=0))
            before = _state(child)
            changed = _delta(root, child, before)
            configs = (SourceConfiguration.read(root), SourceConfiguration.read(child))
            source = {name for name in changed if any(
                name == config.image_pin or any(PurePosixPath(name).is_relative_to(base) for base in config.roots)
                for config in configs)}
            decisions = {name for name in changed if name == "recovery/project.json" or any(
                PurePosixPath(name).is_relative_to(base) for base in ("recovery/tasks", "recovery/history"))}
            row = self.rows.get(child)
            if row is None:
                if source or decisions:
                    raise ValueError("unreserved recovery worktree; preserve it and authorize reserved_worktrees: " + str(child))
            else:
                subjects, files, allowed = set(), set(), set()
                for name in row["tasks"]:
                    store = TaskStore(child)
                    task = store.read(name)
                    allowed.update(_resumable_history(child, store, ReviewStore(child, name), name))
                    selected, editable = store.scope(name)
                    subjects.update(selected)
                    files.update(editable)
                    allowed.add("recovery/tasks/" + name + ".json")
                    if (root / "recovery/tasks" / (name + ".json")).exists():
                        owner = TaskStore(root)
                        if owner.read(name) != task:
                            raise ValueError("reserved task identity or baseline conflicts with destination")
                        selected, editable = owner.scope(name)
                        subjects.update(selected)
                        files.update(editable)
                # Unknown source changes could move another original identity;
                # do not infer ownership from filenames or silently expand scope.
                if source - files or decisions - allowed:
                    raise ValueError("reserved worktree has changes outside its named task scopes/history")
                self.records.append({"path": str(child), "tasks": row["tasks"],
                                     "subjects": sorted(subjects), "editable_files": sorted(files),
                                     "state": _token(before), "evidence": row["evidence"]})
            if _state(child) != before:
                raise ValueError("external worktree changed while deriving reservations")
            self.states[child] = before
        self.assert_current()

    @property
    def subjects(self):
        return {item for row in self.records for item in row["subjects"]}

    @property
    def files(self):
        return {item for row in self.records for item in row["editable_files"]}

    @property
    def tasks(self):
        return {item for row in self.records for item in row["tasks"]}

    def assert_current(self):
        inventory = _worktrees(self.root)
        expected = {path: data for path, data in self.inventory.items() if path != self.root and path not in self.managed}
        actual = {path: data for path, data in inventory.items() if path != self.root and path not in self.managed}
        if actual != expected or any(_state(path) != state for path, state in self.states.items()):
            raise ValueError("external worktree reservations changed; preserve work and replan")

    def check_scope(self, name, subjects, files):
        if name in self.tasks or self.subjects.intersection(subjects) or self.files.intersection(files):
            raise ValueError("task or delegated scope overlaps a reserved worktree")


@contextmanager
def reservations(root, rows, *, managed=None):
    validate_reservations(rows)
    with ExitStack() as stack:
        yield Reservations(root, rows, managed if managed is not None else set(), stack)
