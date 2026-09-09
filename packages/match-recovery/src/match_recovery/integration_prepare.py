"""Create unverified merge candidates without moving the destination branch.

Only committed source is materialized. Hooks, checkout filters and external
merge drivers cannot participate. Compiler resources and verification are
separate; a clean text merge is never an acceptance result.
"""

from pathlib import Path
import os
import re
import subprocess

from .checkpoints import preflight
from .driver import working_state
from .git_worktree import (destination_state, git_options, materialize, new_destination,
                           raw_inputs, run_git)
from .integration import decision, validate
from .locking import file_lock
from .model import canonical, require_text
from .records import parse_json
from .source_history import _tree, git
from .tasks import Task, task_name


def prepare_integration(root, name, context, *, worktree, branch, _held_lease=None):
    root = Path(root).resolve()
    task_name(name)
    require_text(branch, "candidate branch")
    if not isinstance(context, dict) or not isinstance(context.get("source_revision"), str) or not re.fullmatch(
            r"[0-9a-f]{40}|[0-9a-f]{64}", context["source_revision"]):
        raise ValueError("integration preparation requires an immutable incoming commit")
    task_path = "recovery/tasks/" + name + ".json"
    entry = _tree(root, context["source_revision"], (task_path,)).get(task_path)
    if entry is None or entry[:2] != ("100644", "blob"):
        raise ValueError("incoming source must contain a regular task authorization")
    task = Task.from_dict(parse_json(git(root, "cat-file", "blob", entry[2])))
    if task.task_id != name:
        raise ValueError("incoming task filename and identity disagree")
    context = decision(task, context)
    with file_lock(root / "build/recovery/driver.lock", timeout=0, held=_held_lease), git_options(root) as options:
        target_branch = preflight(root)
        target, source = context["target_revision"], context["source_revision"]
        if git(root, "rev-parse", "HEAD").decode().strip() != target:
            raise ValueError("destination HEAD has moved from the explicitly authorized target")
        before = destination_state(root, target, options)
        for revision in (target, source):
            if git(root, "rev-parse", revision + "^{commit}").decode().strip() != revision:
                raise ValueError("integration inputs must be commits")
            git(root, "merge-base", "--is-ancestor", task.base_revision, revision)
        if git(root, "merge-base", source, target).decode().strip() == source:
            raise ValueError("incoming source is already incorporated in the destination")
        path = new_destination(root, options, worktree, branch)

        def fresh():
            if working_state(root) != before or preflight(root) != target_branch:
                raise ValueError("destination changed during candidate preparation; preserve work and re-evaluate")

        merged = run_git(root, options, "merge-tree", "--write-tree", "--name-only", "--no-messages", "-z", target, source, codes=(0, 1))
        parts = [part for part in merged.stdout.split(b"\0") if part]
        if not parts or not re.fullmatch(rb"[0-9a-f]{40}|[0-9a-f]{64}", parts[0]):
            raise ValueError("unsupported Git merge-tree result")
        fresh()
        report = {"task_id": name, "integration_context": context, "destination_branch": target_branch,
                  "candidate_created": False, "verification": "not_measured", "ready_for_collection": False}
        if merged.returncode == 1:
            return {**report, "stop_reason": "merge_conflicts", "conflicts": [os.fsdecode(item) for item in parts[1:]]}
        if len(parts) != 1:
            raise ValueError("clean merge unexpectedly returned unresolved paths")
        tree = parts[0].decode()
        entries, blobs = raw_inputs(root, options, tree)
        message = b"Prepare unverified recovery integration: " + name.encode() + b"\n\nIntegration authorization:\n" + canonical(context)
        commit = run_git(root, options, "commit-tree", tree, "-p", target, "-p", source, input=message).stdout.decode().strip()
        fresh()
        # No existing checkout, index or branch is repurposed. Failures retain
        # any newly created candidate for inspection, never a reset/reap.
        try:
            materialize(root, options, path, branch, commit, entries, blobs)
            validate(path, task, context)
            fresh()
        except (OSError, ValueError, subprocess.SubprocessError) as error:
            raise ValueError(f"candidate preparation incomplete; preserve and inspect {path} and branch {branch}: {error}") from error
        return {**report, "stop_reason": "candidate_prepared", "candidate_created": True,
                "worktree": str(path), "branch": branch, "commit": commit,
                "next_action": "prepare isolated compiler recipes/resources, then run fresh integration-context verification"}
