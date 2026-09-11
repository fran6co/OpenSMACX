"""One-use admission for already authorized, disjoint retained worker scopes.

This never expands a task, copies its journal, resets its baseline or accepts its
source. The coordinator holds scheduling/worker leases while deriving admission.
Active peers may edit source and append findings without invalidating ownership.
"""

from pathlib import Path
import re
from uuid import uuid4

from .checkpoints import _publication_changes, preflight
from .dispatch import _checkout
from .driver import _resumable_history
from .investigation_round import epoch
from .model import digest, require_digest
from .records import publish_json
from .reviews import ReviewStore
from .scheduling import _state, _token
from .source_config import SourceConfiguration
from .source_replica import artifact_parent
from .tasks import TaskStore, task_name as require_task


def _path(value):
    if not isinstance(value, (str, Path)):
        raise ValueError("retained scope needs a canonical worktree path")
    path = Path(value)
    if not path.is_absolute() or path.resolve() != path or not path.is_dir():
        raise ValueError("retained scope needs a canonical existing worktree")
    return path


def _owner(path, name):
    store = TaskStore(path)
    task = store.read(name)
    subjects, files = store.scope(name)
    return {"task": task.to_dict(), "subjects": sorted(subjects), "editable_files": sorted(files)}


def _ownership(root, child, name, peers, reservations):
    common, root_branch = _checkout(root)
    child_common, child_branch = _checkout(child)
    if child == root or child_common != common or child_branch == root_branch:
        raise ValueError("retained scope requires an isolated branch in the coordinator repository")
    own = _owner(child, name)
    if own["task"] != _owner(root, name)["task"]:
        raise ValueError("retained task identity or original baseline differs from coordinator")
    subjects, files = set(own["subjects"]), set(own["editable_files"])
    owners = {}

    def include(path, other):
        owner = _owner(path, other)
        if subjects.intersection(owner["subjects"]) or files.intersection(owner["editable_files"]):
            raise ValueError("retained scope overlaps another task or reserved owner")
        owners[str(path) + ":" + other] = owner

    for path in sorted((root / "recovery/tasks").glob("*.json")):
        if path.stem != name:
            include(root, path.stem)
    seen, branches = set(), {root_branch, child_branch}
    peer_rows = []
    for row in peers:
        if not isinstance(row, dict) or set(row) != {"task", "root", "branch"}:
            raise ValueError("retained peers require explicit task, worktree and assigned branch")
        other, path = require_task(row["task"]), _path(row["root"])
        if path in seen:
            raise ValueError("duplicate retained peer worktree")
        seen.add(path)
        peer_common, branch = _checkout(path)
        if peer_common != common or branch != row["branch"]:
            raise ValueError("retained peer repository or assigned branch changed")
        if path == child:
            if other != name:
                raise ValueError("retained assignment changed its task")
            continue
        if other == name or branch in branches:
            raise ValueError("retained peer repeats a task or branch")
        branches.add(branch)
        include(path, other)
        if (root / "recovery/tasks" / (other + ".json")).exists():
            if _owner(path, other)["task"] != _owner(root, other)["task"]:
                raise ValueError("retained peer task identity or baseline changed")
        peer_rows.append({"root": str(path), "task": other, "branch": branch})
    external = []
    for row in reservations:
        if not isinstance(row, dict) or set(row) != {"root", "tasks"}:
            raise ValueError("retained reservations require explicit worktree and tasks")
        path = _path(row["root"])
        names = row["tasks"]
        if (path in seen or path in (root, child) or not isinstance(names, list) or not names
                or any(not isinstance(item, str) for item in names) or len(set(names)) != len(names)):
            raise ValueError("retained reservations must be distinct named external owners")
        seen.add(path)
        external_common, branch = _checkout(path)
        if external_common != common or branch in branches:
            raise ValueError("retained external reservation repository or branch conflicts")
        branches.add(branch)
        for other in names:
            include(path, require_task(other))
        external.append({"root": str(path), "tasks": sorted(names), "branch": branch})
    return {"common": str(common), "root_branch": root_branch, "branch": child_branch,
            "own": own, "owners": owners, "peers": peer_rows, "reservations": external}


def _admissible(root, child, name, revision):
    from .campaign_choices import ancestor
    if not isinstance(revision, str) or not re.fullmatch(r"[0-9a-f]{40}|[0-9a-f]{64}", revision):
        raise ValueError("retained assignment requires a full original revision")
    preflight(child)
    if not ancestor(child, revision):
        raise ValueError("retained worker lost assignment ancestry")
    parent, store = TaskStore(root), TaskStore(child)
    if parent.read(name) != store.read(name):
        raise ValueError("retained task identity or baseline changed")
    stream = "task:" + name
    prior, current = parent.journal.read(stream), store.journal.read(stream)
    if current[:len(prior)] != prior:
        raise ValueError("retained task journal does not extend coordinator history")
    for event in prior:
        filename = f"{event.sequence:08d}.json"
        if ((parent.journal.stream_directory(stream) / filename).read_bytes()
                != (store.journal.stream_directory(stream) / filename).read_bytes()):
            raise ValueError("retained task journal changed original event bytes")
    allowed = set(store.scope(name)[1]) | _resumable_history(child, store, ReviewStore(child, name), name)
    # Include ignored configured source: ignore rules cannot hide collateral WIP.
    state = _state(child)
    _publication_changes(child, {**state, "head": (revision + "\n").encode()}, allowed)


def authorize(root, child, task_name, *, revision, branch, peers, reservations=()):
    """Derive admission under coordinator-owned source and ownership leases."""
    root, child = _path(root), _path(child)
    require_task(task_name)
    root_epoch, own_epoch = _token(_state(root)), epoch(child, task_name)
    ownership = _ownership(root, child, task_name, peers, reservations)
    if ownership["branch"] != branch:
        raise ValueError("retained worker is not on its assigned branch")
    _admissible(root, child, task_name, revision)
    permission = {"schema_version": 1, "kind": "retained_parallel_scope", "request_id": uuid4().hex,
                  "root": str(child), "coordinator_root": str(root), "task": task_name,
                  "author": TaskStore(child).read(task_name).author, "revision": revision,
                  "branch": branch, "root_epoch": root_epoch, "epoch": own_epoch,
                  "peers": peers, "reservations": list(reservations), "ownership": ownership}
    validate(child, task_name, permission)
    return permission


def check_ownership(child, task_name, permission):
    """Recheck ownership, not changing peer source or legitimate new findings."""
    fields = {"schema_version", "kind", "request_id", "root", "coordinator_root", "task", "author",
              "revision", "branch", "root_epoch", "epoch", "peers", "reservations", "ownership"}
    if (not isinstance(permission, dict) or set(permission) != fields
            or type(permission["schema_version"]) is not int or permission["schema_version"] != 1
            or permission["kind"] != "retained_parallel_scope"
            or not isinstance(permission["request_id"], str)
            or not re.fullmatch(r"[0-9a-f]{32}", permission["request_id"])
            or not isinstance(permission["peers"], list) or not isinstance(permission["reservations"], list)):
        raise ValueError("invalid retained parallel scope permission")
    child, root = _path(child), _path(permission["coordinator_root"])
    if permission["root"] != str(child) or permission["task"] != task_name:
        raise ValueError("retained permission names another worker or task")
    require_task(task_name)
    for name in ("root_epoch", "epoch"):
        require_digest(permission[name])
    if permission["author"] != TaskStore(child).read(task_name).author:
        raise ValueError("retained permission names another coordinator")
    if permission["root_epoch"] != _token(_state(root)):
        raise ValueError("retained coordinator state changed; replan")
    actual = _ownership(root, child, task_name, permission["peers"], permission["reservations"])
    if digest(actual) != digest(permission["ownership"]) or actual["branch"] != permission["branch"]:
        raise ValueError("retained ownership, task or branch changed; replan")
    return TaskStore(child).scope(task_name)


def validate(child, task_name, permission):
    scope = check_ownership(child, task_name, permission)
    child, root = _path(child), _path(permission["coordinator_root"])
    if permission["epoch"] != epoch(child, task_name):
        raise ValueError("stale retained worker admission; source or decisions changed")
    _admissible(root, child, task_name, permission["revision"])
    if permission["epoch"] != epoch(child, task_name):
        raise ValueError("retained worker changed during admission validation")
    check_ownership(child, task_name, permission)
    return scope


def consume(child, task_name, permission):
    scope = validate(child, task_name, permission)
    child = _path(child)
    parent = artifact_parent(child, "build/recovery/retained-scope-uses",
                             excluded_roots=SourceConfiguration.read(child).roots)
    try:
        publish_json(parent / (permission["request_id"] + ".json"),
                     {"permission_sha256": digest(permission), "task": task_name})
    except FileExistsError as error:
        raise ValueError("retained scope permission already consumed; obtain fresh admission") from error
    validate(child, task_name, permission)
    return scope
