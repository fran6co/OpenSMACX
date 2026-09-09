"""Prepare a committed, authorized task for an isolated native worker run.

No task baseline reset, implicit compiler installation, copied build cache or
acceptance. The coordinator retains failed preparations for inspection.
"""

from contextlib import nullcontext
from pathlib import Path
import re
import subprocess

from .build_check import Build
from .checkpoints import preflight
from .driver import working_state
from .git_worktree import (assert_raw_checkout, destination_state, git_options, materialize,
                           new_destination, raw_inputs)
from .locking import file_lock
from .model import require_text
from .preparation import prepare_project
from .resources import resource_environment
from .source_config import SourceConfiguration
from .source_history import git
from .source_inventory import image_pin
from .tasks import TaskStore, task_name


def prepare_worker(root, name, authorization, *, worktree, branch, resources=None, resource_timeout=0):
    root = Path(root).absolute()
    if root.resolve() != root:
        raise ValueError("worker preparation needs a canonical coordinator root")
    task_name(name)
    if (not isinstance(authorization, dict) or set(authorization) != {"author", "revision", "evidence", "prepare"}
            or not isinstance(authorization["revision"], str)
            or not re.fullmatch(r"[0-9a-f]{40}|[0-9a-f]{64}", authorization["revision"])
            or type(authorization["prepare"]) is not bool):
        raise ValueError("worker authorization needs author, immutable revision, evidence and boolean prepare")
    require_text(authorization["author"], "worker setup coordinator")
    if not isinstance(authorization["evidence"], list) or not authorization["evidence"]:
        raise ValueError("worker preparation requires authorization evidence")
    for item in authorization["evidence"]:
        require_text(item, "worker preparation evidence")
    if authorization["prepare"] != (resources is not None):
        raise ValueError("automatic compiler preparation requires explicit resources; checkout-only setup does not lease them")
    if resources is None and resource_timeout != 0:
        raise ValueError("resource timeout requires compiler preparation resources")
    with file_lock(root / "build/recovery/driver.lock", timeout=0), git_options(root) as options:
        coordinator_branch = preflight(root)
        revision = authorization["revision"]
        if git(root, "rev-parse", "HEAD").decode().strip() != revision:
            raise ValueError("coordinator HEAD moved from the authorized worker revision")
        before = destination_state(root, revision, options)
        store = TaskStore(root)
        task, scope = store.read(name), store.scope(name)
        if task.author != authorization["author"]:
            raise ValueError("only the task coordinator may authorize worker setup")
        git(root, "merge-base", "--is-ancestor", task.base_revision, revision)
        source = SourceConfiguration.read(root)
        pin = image_pin(root, source)
        if any(subject.image_sha256 != pin or subject.address_space != source.address_space for subject in task.subjects):
            raise ValueError("worker task refers to a different source-pinned original")
        path = new_destination(root, options, worktree, branch, excluded_roots=source.roots)
        entries, blobs = raw_inputs(root, options, revision)
        if authorization["prepare"]:
            Build.read(root, key="preparation")  # Refuse missing configuration before creating a branch.

        def fresh():
            if working_state(root) != before or preflight(root) != coordinator_branch:
                raise ValueError("coordinator changed during worker preparation; preserve work and re-evaluate")

        environment = (resource_environment(resources, [root], timeout=resource_timeout)
                       if authorization["prepare"] else nullcontext())
        with environment:
            fresh()
            try:
                materialize(root, options, path, branch, revision, entries, blobs)
                child = TaskStore(path)
                if child.read(name) != task or child.scope(name) != scope:
                    raise ValueError("materialized worker task or scope disagrees with the coordinator")
                with file_lock(path / "build/recovery/driver.lock", timeout=0):
                    preparation = prepare_project(path) if authorization["prepare"] else None
                    assert_raw_checkout(path, revision)
                fresh()
            except (OSError, ValueError, subprocess.SubprocessError) as error:
                raise ValueError(f"worker preparation incomplete; preserve and inspect {path} and branch {branch}: {error}") from error
        prepared = preparation is not None and preparation["result"] == "pass"
        return {"schema_version": 1, "task_id": name, "worker_created": True,
                "worktree": str(path), "branch": branch, "revision": revision,
                "task_base_revision": task.base_revision, "compiler_prepared": prepared,
                "preparation": preparation, "verification": "not_measured", "ready_for_collection": False,
                "stop_reason": ("worker_prepared" if prepared else
                                "checkout_prepared" if preparation is None else "compiler_preparation_failed"),
                "next_action": "inspect preparation, then dispatch with explicit worker/reviewer and compiler resource configuration"}
