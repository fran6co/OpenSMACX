"""Coordinator-authorized parallel driver runs in existing isolated worktrees.

A batch is a scheduling decision, never an acceptance ledger. Child reports
remain operational hints; integration still requires fresh native verification.
No worker may independently expand a parallel batch's authorized editing scope.
Commands are trusted project inputs, not an adversarial-agent sandbox.
"""

import os
from pathlib import Path, PurePosixPath
import sys
import tempfile

from .components import VendorManifest, relative_path
from .driver import Command, changes, working_state
from .locking import file_lock
from .model import require_text
from .ownership_source import component_manifests
from .processes import run_parallel
from .records import publish_json, read_json
from .resources import INHERITED, _slots
from .source_config import SourceConfiguration
from .source_history import git
from .source_inventory import source_snapshot
from .source_replica import artifact_parent
from .tasks import TaskStore, task_name


def _configuration(root, name):
    relative_path(name)
    path = root / name
    if path.resolve() != path or not path.is_file() or path.stat().st_nlink != 1:
        raise ValueError("dispatch configuration must be a canonical unaliased project file")
    return read_json(path)


def _checkout(path):
    if any(name in os.environ for name in ("GIT_INDEX_FILE", "GIT_DIR", "GIT_WORK_TREE", "GIT_COMMON_DIR")):
        raise ValueError("dispatch refuses Git path overrides that could defeat worktree isolation")
    if not path.is_absolute() or path.resolve() != path or not path.is_dir():
        raise ValueError("dispatch requires canonical existing worktrees")
    if Path(os.fsdecode(git(path, "rev-parse", "--show-toplevel")).strip()) != path:
        raise ValueError("dispatch requires a worktree root, not a subdirectory")
    branch = git(path, "symbolic-ref", "HEAD").decode().strip()
    if not branch.startswith("refs/heads/"):
        raise ValueError("dispatch requires local worktree branches")
    for name in ("MERGE_HEAD", "CHERRY_PICK_HEAD", "REVERT_HEAD", "rebase-merge", "rebase-apply", "sequencer"):
        if Path(os.fsdecode(git(path, "rev-parse", "--path-format=absolute", "--git-path", name)).strip()).exists():
            raise ValueError("finish the existing Git operation before parallel dispatch")
    common = Path(os.fsdecode(git(path, "rev-parse", "--path-format=absolute", "--git-common-dir")).strip())
    return common.resolve(), branch


def _application_files(root):
    source = source_snapshot(root)
    config = SourceConfiguration.from_dict(source["source_configuration"])
    vendor_roots = [manifest.source_root for manifest in component_manifests(source)
                    if isinstance(manifest, VendorManifest)]
    return {name for name in source["files"]
            if not config.is_seed(name) and name != config.image_pin and not name.endswith(".recovery.h")
            and not any(PurePosixPath(name).is_relative_to(base) for base in vendor_roots)}


def _prepare(root, plan):
    required = {"schema_version", "author", "evidence", "max_parallel", "resources", "timeout", "jobs"}
    if (not isinstance(plan, dict) or set(plan) != required
            or type(plan["schema_version"]) is not int or plan["schema_version"] != 1):
        raise ValueError("invalid dispatch plan")
    require_text(plan["author"], "dispatch coordinator")
    if not isinstance(plan["evidence"], list) or not plan["evidence"]:
        raise ValueError("dispatch requires authorization evidence")
    for item in plan["evidence"]:
        require_text(item, "dispatch evidence")
    if type(plan["max_parallel"]) is not int or not 1 <= plan["max_parallel"] <= 32:
        raise ValueError("dispatch max_parallel must be in 1..32")
    if type(plan["timeout"]) is not int or not 1 <= plan["timeout"] <= 86400:
        raise ValueError("dispatch per-job timeout must be in 1..86400 seconds")
    if not isinstance(plan["jobs"], list) or not 1 <= len(plan["jobs"]) <= 256:
        raise ValueError("dispatch requires 1..256 jobs")
    if INHERITED in os.environ:
        raise ValueError("dispatch cannot share an inherited compiler lease across workers")
    common, branch = _checkout(root)
    store = TaskStore(root)
    application = _application_files(root)
    seen_roots, seen_branches, seen_tasks, subjects, files = [], {branch}, set(), set(), set()
    jobs = []
    for row in plan["jobs"]:
        required_job = {"task", "root", "config", "worker", "reviewer", "iterations"}
        if (not isinstance(row, dict) or not required_job <= row.keys()
                or row.keys() - required_job - {"migration", "checkpoint"}
                or type(row["iterations"]) is not int or not 1 <= row["iterations"] <= 100):
            raise ValueError("invalid dispatch job; scope delegation is not allowed within a batch")
        name = task_name(row["task"])
        require_text(row["root"], "worker worktree")
        child = Path(row["root"])
        if not child.is_absolute():
            relative_path(row["root"])
            child = root / child
        child_common, child_branch = _checkout(child)
        if (child == root or root.is_relative_to(child) or child_common != common
                or child_branch in seen_branches or name in seen_tasks
                or any(child.is_relative_to(other) or other.is_relative_to(child) for other in seen_roots)):
            raise ValueError("dispatch jobs require distinct tasks and disjoint worktrees/branches in the same repository")
        if child.is_relative_to(root):
            artifact_parent(root, child.relative_to(root).as_posix(),
                            excluded_roots=SourceConfiguration.read(root).roots)
        child_store = TaskStore(child)
        task = store.read(name)
        scope = store.scope(name)
        if task.author != plan["author"] or child_store.read(name) != task or child_store.scope(name) != scope:
            raise ValueError("dispatch requires identical coordinator-authorized task and scope in each worker")
        child_subjects, child_files = scope
        if subjects & child_subjects or files & child_files:
            raise ValueError("parallel dispatch scopes overlap in original identities or editable files")
        if not child_files <= application or not child_files <= _application_files(child):
            raise ValueError("parallel workers may edit only existing application source, not seeds, dependencies or recovery policy")
        for base in (root, child):
            for name_in_source in child_files:
                path = base / name_in_source
                if path.resolve() != path or not path.is_file() or path.stat().st_nlink != 1:
                    raise ValueError("parallel source scope requires canonical unaliased regular files")
        configs = {key: _configuration(child, row[key]) for key in ("config", "worker", "reviewer")}
        identities = [plan["author"]]
        for role in ("worker", "reviewer"):
            value = configs[role]
            if set(value) != {"author", "command"}:
                raise ValueError("dispatch roles require author and command")
            require_text(value["author"], "dispatch role author")
            Command.read(value["command"])
            identities.append(value["author"])
        if len(set(identities)) != 3 or configs["config"].get("target") != task.target:
            raise ValueError("dispatch requires independent coordinator/worker/reviewer and matching target")
        for key in ("migration", "checkpoint"):
            if key in row:
                _configuration(child, row[key])
        seen_roots.append(child)
        seen_branches.add(child_branch)
        seen_tasks.add(name)
        subjects.update(child_subjects)
        files.update(child_files)
        jobs.append((row, child, scope))
    resources = _configuration(root, plan["resources"])
    if plan["max_parallel"] > len(_slots(resources, [root, *seen_roots])):
        raise ValueError("dispatch concurrency exceeds the configured compiler resource slots")
    return jobs, resources


def dispatch(root: Path, plan: dict, *, progress=None, _held_lease=None):
    """Run one bounded batch; return process outcomes, never accepted source."""
    root = Path(root).absolute()
    _checkout(root)
    with file_lock(root / "build/recovery/driver.lock", timeout=0, held=_held_lease):
        before = working_state(root)
        jobs, resources = _prepare(root, plan)
        parent = artifact_parent(root, "build/recovery/dispatch", excluded_roots=SourceConfiguration.read(root).roots)
        directory = Path(tempfile.mkdtemp(prefix="batch-", dir=parent))
        # Freeze the shared pool declaration, not measured state or acceptance.
        resource_path = directory / "resources.json"
        publish_json(resource_path, resources)
        logs = directory / "logs"
        logs.mkdir()
        commands = []
        for row, child, _ in jobs:
            argv = [sys.executable, "-I", "-m", "match_recovery", "run", row["task"], "--root", str(child),
                    "--iterations", str(row["iterations"]), "--resources", str(resource_path)]
            for key in ("config", "worker", "reviewer", "migration", "checkpoint"):
                if key in row:
                    argv.extend(["--" + key, row[key]])
            commands.append({"argv": argv, "cwd": child, "timeout": plan["timeout"]})
        if changes(before, working_state(root)):
            raise ValueError("coordinator inputs changed during dispatch preparation")
        outcomes = run_parallel(commands, limit=plan["max_parallel"], directory=logs, progress=progress)
        coordinator_changed = bool(changes(before, working_state(root)))
        results = []
        for (row, child, scope), outcome in zip(jobs, outcomes, strict=True):
            try:
                intact = (TaskStore(child).read(row["task"]) == TaskStore(root).read(row["task"])
                          and TaskStore(child).scope(row["task"]) == scope == TaskStore(root).scope(row["task"]))
            except (OSError, ValueError):
                intact = False
            results.append({"task": row["task"], "root": str(child), "scope_intact": intact, **outcome})
        return {"schema_version": 1, "jobs": results, "artifacts": str(directory),
                "coordinator_changed": coordinator_changed,
                "batch_completed": not coordinator_changed and all(
                    row["scope_intact"] and not row["timed_out"] and row["returncode"] in (0, 2) for row in results),
                "verification": "not_measured_by_dispatch", "ready_for_collection": False,
                "next_action": "inspect_worktrees_then_fresh_serial_integration_or_replan"}
