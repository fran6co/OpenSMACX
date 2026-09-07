"""Source-derived task authorization, independent of a target or compiler.

These operations authorize work, not regressions or acceptance. Historical
restoration evidence is supplied separately until its source provider is generic.
"""

import hashlib
from pathlib import Path
import re
import subprocess

from .model import Origin, require_text
from .ownership_source import project_view, structural_impact
from .partition import select_group
from .records import read_json
from .source_config import SourceConfiguration
from .source_inventory import assert_snapshot_current
from .source_audits import context_audits
from .tasks import Task, TaskStore


def _git(root, *arguments, check=True):
    return subprocess.run(["git", "--literal-pathspecs", *arguments], cwd=root,
                          capture_output=True, check=check)


def create_task(root: Path, *, name, group, author, intent, evidence,
                editable_files=(), target=None, base=None):
    root = root.resolve()
    driver = root / "recovery/driver.json"
    driver_bytes = None
    if target is None:
        if not driver.resolve().is_relative_to(root):
            raise ValueError("driver configuration resolves outside the worktree")
        driver_bytes = driver.read_bytes()
        target = read_json(driver).get("target")
    require_text(target, "task target (supply --target or recovery/driver.json)")
    view, snapshot = project_view(root)
    configuration = SourceConfiguration.from_dict(snapshot["source_configuration"])
    selected = select_group(view, group)
    if selected["next_action"] != "review_ownership_and_structure":
        raise ValueError("structural recovery tasks require an unambiguous source/class group")
    files = sorted(set(structural_impact(snapshot, selected)["root_files"]) | set(editable_files))
    for name_in_source in files:
        configuration.path(root, name_in_source)
    paths = sorted(set(configuration.roots) | set(files) |
                   {configuration.image_pin, "recovery/project.json"})
    required = set(snapshot["files"]) | {configuration.image_pin, "recovery/project.json"}
    required.update(name_in_source for name_in_source in files if (root / name_in_source).exists())

    def clean_source():
        changes = _git(root, "status", "--porcelain", "--untracked-files=all", "--", *paths)
        tracked = {}
        for item in _git(root, "ls-tree", "-r", "-z", snapshot["revision"], "--", *paths).stdout.split(b"\0"):
            if item:
                metadata, path = item.split(b"\t", 1)
                mode, kind, oid = metadata.decode().split()
                tracked[path.decode()] = (mode, kind, oid)
        if changes.stdout.strip() or not required <= tracked.keys():
            raise ValueError("checkpoint source and configuration before fixing a task's committed baseline")
        # Git status can hide assume-unchanged/skip-worktree files. Compare the
        # actual bytes to their committed blobs, without one process per file.
        for path in required:
            mode, kind, oid = tracked[path]
            raw = configuration.path(root, path).read_bytes()
            payload = b"blob " + str(len(raw)).encode() + b"\0" + raw
            actual = hashlib.new("sha1" if len(oid) == 40 else "sha256", payload).hexdigest()
            if mode not in ("100644", "100755") or kind != "blob" or actual != oid:
                raise ValueError(f"checkpoint source bytes before fixing a task baseline: {path}")

    clean_source()
    baseline = snapshot["revision"] if base is None else base
    if not isinstance(baseline, str) or not re.fullmatch(r"[0-9a-f]{40}|[0-9a-f]{64}", baseline):
        raise ValueError("--base must be a full committed revision")
    ancestor = _git(root, "merge-base", "--is-ancestor", baseline, snapshot["revision"], check=False)
    if ancestor.returncode != 0:
        raise ValueError("task baseline must be an ancestor of the current source revision")
    equivalent = _git(root, "diff", "--quiet", baseline, snapshot["revision"], "--", *paths, check=False)
    if equivalent.returncode != 0:
        raise ValueError("task baseline source/configuration differs from the current source")
    task = Task(name, target, author, intent, baseline,
                tuple(Origin(**entity["origin"]) for entity in selected["entities"]),
                tuple(files), tuple(evidence))
    assert_snapshot_current(root, snapshot)
    clean_source()
    if driver_bytes is not None and driver.read_bytes() != driver_bytes:
        raise ValueError("task target configuration changed during creation")
    return {"created": TaskStore(root).create(task), "task": task.to_dict(), "verification": "not_measured"}


def expand_task(root: Path, *, name, subjects, editable_files, **decision):
    root = root.resolve()
    view, snapshot = project_view(root)
    entities = {entity["entity_id"]: entity for group in view["groups"] for entity in group["entities"]}
    if not set(subjects) <= entities.keys():
        raise ValueError("scope expansion names an entity missing from current source")
    configuration = SourceConfiguration.from_dict(snapshot["source_configuration"])
    for path in editable_files:
        configuration.path(root, path)
    assert_snapshot_current(root, snapshot)
    return TaskStore(root).expand(name, subjects=(Origin(**entities[subject]["origin"]) for subject in subjects),
                                  editable_files=editable_files, **decision)


def task_context(root: Path, name: str, *, restoration_for):
    """Rebuild context; never replace missing restoration coverage with an empty list."""
    root = root.resolve()
    store = TaskStore(root)
    task, decisions = store.read(name), store.decisions(name)
    scope, files = store._scope(task, decisions)
    view, snapshot = project_view(root)
    address_space = snapshot["source_configuration"]["address_space"]
    if any(subject.image_sha256 != snapshot["image_sha256"] or subject.address_space != address_space
           for subject in task.subjects):
        raise ValueError("task original does not match the configured source image/address space")
    selected = [entity for group in view["groups"] for entity in group["entities"] if entity["entity_id"] in scope]
    group = {"entities": selected, "source_homes": sorted(files)}
    output = {"task": task.to_dict(), "subjects": selected,
              "decision_state": store.status(name, (entity["entity_id"] for entity in selected)),
              "impact": structural_impact(snapshot, group), "restoration": restoration_for(root, name),
              "source_hints": context_audits(root, files),
              "binding_hints": [{"entity_id": row["entity_id"], "path": row["path"], **row["binding_hint"]}
                                for row in snapshot["records"] if row["entity_id"] in scope and row["binding_hint"]],
              "instructions": ["Read source declarations, definitions and evidence before editing.",
                               "Treat archived bodies and class/name labels as seeds, not mandatory source form.",
                               "Review ownership alternatives against declarations and callers; a member-symbol group does not prove hierarchy or erase an older interpretation.",
                               "Report structural discoveries even when they affect other functions or units.",
                               "Do not edit beyond authorized scope or discard matching obligations.",
                               "A scope decision does not approve a regression; migration review and fresh checks are separate."],
              "persisted": False}
    assert_snapshot_current(root, snapshot)
    if task != store.read(name) or decisions != store.decisions(name):
        raise ValueError("task decisions changed during context generation")
    return output
