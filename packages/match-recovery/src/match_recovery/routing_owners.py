"""Fresh routing context from explicitly supplied ownership stores.

Availability describes a controller's managed assignments, not an executable
permission. This module never discovers other checkouts, takes ownership, writes
journals, or activates reservations. Callers retain their existing leases and
compare snapshots around external hooks.
"""

from pathlib import Path

from .dispatch import _checkout
from .locking import file_lock
from .task_routes import incoming_routes, target_reference
from .tasks import TaskStore, task_name


def _path(value):
    path = Path(value)
    if not path.is_absolute() or path.resolve() != path or path.is_symlink():
        raise ValueError("routing owners require canonical explicit checkout paths")
    return path


def _observe(paths):
    """Capture every task, including unassigned replicas, in explicit stores."""
    catalogs, replicas = [], {}
    common = None
    for path in paths:
        repository, branch = _checkout(path)
        if common is not None and common != repository:
            raise ValueError("routing owner belongs to another repository")
        common = repository
        store = TaskStore(path)
        entries = []
        for filename in sorted((path / "recovery/tasks").glob("*.json")):
            name = task_name(filename.stem)
            reference = target_reference(store, name)
            events = store.journal.read("task:" + name)
            if reference != target_reference(store, name) or reference["history_head"] != (
                    events[-1].event_id if events else None):
                raise ValueError("routing owner changed during observation")
            prior = replicas.setdefault(name, [])
            for _, previous, history in prior:
                shared = min(len(events), len(history))
                if (reference["task_sha256"] != previous["task_sha256"]
                        or events[:shared] != history[:shared]):
                    raise ValueError("routing owner task/history replicas conflict")
            prior.append((path, reference, events))
            entries.append(reference)
        catalogs.append({"root": str(path), "branch": branch, "tasks": entries})
    return catalogs, replicas


def _owners(replicas, preferred):
    result = []
    for name in sorted(replicas):
        copies = replicas[name]
        latest = max(len(row[2]) for row in copies)
        if name in preferred:
            path, availability = preferred[name]
            chosen = next((row for row in copies if row[0] == path), None)
            if chosen is None or len(chosen[2]) != latest:
                raise ValueError("selected routing owner is missing or has stale task history")
        else:
            chosen = next(row for row in copies if len(row[2]) == latest)
            availability = "unassigned"
        path, reference, _ = chosen
        subjects, files = TaskStore(path).scope(name)
        result.append({"root": str(path), "task": reference, "availability": availability,
                       "subjects": sorted(subjects), "editable_files": sorted(files)})
    if set(preferred) - set(replicas):
        raise ValueError("declared routing owner task is missing")
    return result


def _conflicts(owners):
    """Expose unresolved ambiguity without disabling unrelated owner context."""
    groups = {}
    for field in ("subjects", "editable_files"):
        occupancy = {}
        for owner in owners:
            for value in owner[field]:
                occupancy.setdefault(value, set()).add(owner["task"]["task_id"])
        for value, names in occupancy.items():
            if len(names) > 1:
                key = tuple(sorted(names))
                groups.setdefault(key, {"tasks": list(key), "subjects": [], "editable_files": []})[field].append(value)
    return [{"tasks": row["tasks"], "subjects": sorted(row["subjects"]),
             "editable_files": sorted(row["editable_files"])} for _, row in sorted(groups.items())]


def _snapshot(paths, preferred, current_root, current_task):
    if current_root not in paths:
        raise ValueError("current routing task is outside explicit ownership stores")
    catalogs, replicas = _observe(paths)
    if current_task not in {item["task_id"] for item in next(
            row for row in catalogs if row["root"] == str(current_root))["tasks"]}:
        raise ValueError("current routing task is missing from its checkout")
    owners = _owners(replicas, preferred)
    incoming = incoming_routes(TaskStore(current_root), current_task,
                               sources=[TaskStore(path) for path in paths])
    after, _ = _observe(paths)
    if after != catalogs:
        raise ValueError("routing owner catalogs changed during observation")
    return {"owners": owners, "incoming": incoming, "catalogs": catalogs, "conflicts": _conflicts(owners)}


class RoutingOwners:
    """Process-local explicit ownership inventory, never scheduling authority."""

    def __init__(self, root, assignments, reservations=(), leases=None, *, retired=()):
        self.root = _path(root)
        self.assignments, self.reservations = [], []
        names, paths = set(), set()
        for row in assignments:
            if not isinstance(row, dict) or set(row) != {"root", "task", "branch"}:
                raise ValueError("invalid managed routing assignment")
            path, name = _path(row["root"]), task_name(row["task"])
            if (path == self.root or path in paths or name in names
                    or not isinstance(row["branch"], str) or not row["branch"].startswith("refs/heads/")):
                raise ValueError("routing assignments require distinct tasks, checkouts and local branches")
            paths.add(path)
            names.add(name)
            self.assignments.append({"root": path, "task": name, "branch": row["branch"]})
        for row in reservations:
            if not isinstance(row, dict) or set(row) != {"root", "tasks"}:
                raise ValueError("invalid routing reservation")
            path = _path(row["root"])
            tasks = row["tasks"]
            if (path == self.root or path in paths or not isinstance(tasks, list) or not tasks
                    or any(not isinstance(name, str) for name in tasks) or len(set(tasks)) != len(tasks)):
                raise ValueError("routing reservations require distinct explicit owner tasks and checkouts")
            for name in tasks:
                task_name(name)
                if name in names:
                    raise ValueError("routing owner cannot be both managed and reserved")
                names.add(name)
            paths.add(path)
            self.reservations.append({"root": path, "tasks": tuple(tasks)})
        self.retired = frozenset(task_name(name) for name in retired)
        if not self.retired <= {row["task"] for row in self.assignments}:
            raise ValueError("retired routing owners must belong to the explicit assignments")
        self.leases = {} if leases is None else dict(leases)
        for path in self.leases:
            if not isinstance(path, Path) or _path(path) not in paths | {self.root}:
                raise ValueError("routing lease is outside explicit ownership stores")

    def snapshot(self, current_root, current_task):
        current_root, current_task = _path(current_root), task_name(current_task)
        preferred, paths = {}, {self.root}
        branches = {}
        for row in self.assignments:
            path = _path(row["root"])
            if not path.exists():
                continue  # Absent retired assignments retain only visible root ownership.
            paths.add(path)
            branches[path] = row["branch"]
            preferred[row["task"]] = (path, "retained" if row["task"] in self.retired else "managed")
        for row in self.reservations:
            path = _path(row["root"])
            paths.add(path)
            preferred.update({name: (path, "reserved") for name in row["tasks"]})
        for path, lease in self.leases.items():
            with file_lock(path / "build/recovery/driver.lock", timeout=0, held=lease):
                pass  # Validate real local tokens without acquiring a second lease.
        ordered = [self.root, *sorted(paths - {self.root})]
        result = _snapshot(ordered, preferred, current_root, current_task)
        if any(row["root"] in {str(path) for path in branches}
               and row["branch"] != branches[Path(row["root"])] for row in result["catalogs"]):
            raise ValueError("managed routing owner branch changed from its assignment")
        return result

    def external_epoch(self, current_root):
        """Capture external source/Git state, separately from provider evidence.

        The native Path-keyed result includes absent managed checkouts as None.
        It is ephemeral, not serializable routing authority. Compare around
        hooks while holding existing leases; never discard a changed peer.
        """
        from .scheduling import _state
        current_root = _path(current_root)
        assignments = {row["root"]: row for row in self.assignments}
        reserved = {row["root"] for row in self.reservations}
        paths = {self.root, *assignments, *reserved}
        if current_root not in paths:
            raise ValueError("current routing worktree is outside explicit ownership stores")
        common, _ = _checkout(self.root)
        current_checkout = _checkout(current_root)
        if current_checkout[0] != common:
            raise ValueError("routing owner belongs to another repository")
        if current_root in assignments and current_checkout[1] != assignments[current_root]["branch"]:
            raise ValueError("managed routing owner branch changed from its assignment")
        result = {}
        for path in sorted(paths - {current_root}):
            _path(path)
            if path in assignments and not path.exists():
                result[path] = None
                continue
            checkout = _checkout(path)
            if checkout[0] != common:
                raise ValueError("routing owner belongs to another repository")
            if path in assignments and checkout[1] != assignments[path]["branch"]:
                raise ValueError("managed routing owner branch changed from its assignment")
            state = _state(path)
            if _checkout(path) != checkout:
                raise ValueError("external routing checkout changed during observation")
            result[path] = {"checkout": checkout, "state": state}
        return result


def validate_snapshot(root, task, snapshot):
    """Recompute explicit owner evidence; availability remains controller input.

    This validates request freshness, not permission to route or run a worker.
    Native publication must still use the caller's live RoutingOwners object.
    """
    root, task = _path(root), task_name(task)
    if (not isinstance(snapshot, dict) or set(snapshot) != {"owners", "incoming", "catalogs", "conflicts"}
            or any(not isinstance(snapshot[key], list) for key in snapshot) or not snapshot["catalogs"]):
        raise ValueError("invalid routing owner snapshot")
    paths = []
    for row in snapshot["catalogs"]:
        if not isinstance(row, dict) or set(row) != {"root", "branch", "tasks"}:
            raise ValueError("invalid routing owner catalog")
        path = _path(row["root"])
        if path in paths:
            raise ValueError("duplicate routing owner catalog")
        paths.append(path)
    preferred = {}
    for row in snapshot["owners"]:
        if (not isinstance(row, dict) or set(row) != {"root", "task", "availability", "subjects", "editable_files"}
                or row["availability"] not in ("managed", "reserved", "retained", "unassigned")):
            raise ValueError("invalid routing owner row")
        reference = row["task"]
        if not isinstance(reference, dict) or "task_id" not in reference:
            raise ValueError("invalid routing owner task reference")
        name = task_name(reference["task_id"])
        path = _path(row["root"])
        if name in preferred or path not in paths:
            raise ValueError("duplicate or undisclosed routing owner")
        preferred[name] = (path, row["availability"])
    if _snapshot(paths, preferred, root, task) != snapshot:
        raise ValueError("routing owner snapshot differs from current explicit ownership")
