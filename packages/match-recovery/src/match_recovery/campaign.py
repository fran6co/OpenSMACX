"""Bounded multi-task coordination through native recovery and fresh publication.

Source/tasks/Git remain authoritative. Scheduling exclusions are in-memory and
expire when source changes; reports are not acceptance inputs. Interrupted or
conflicting work is preserved, never reset to make a campaign continue.
"""

import os
from pathlib import Path
import tempfile

from .checkpoints import authorization as checkpoint_authorization, preflight
from .dispatch import _configuration, dispatch
from .driver import Command, changes, drive, invoke, working_state
from .git_worktree import assert_raw_checkout, destination_state, git_options, run_git
from .integration_prepare import prepare_integration
from .integration_publish import publish_integration
from .locking import file_lock
from .model import digest, require_text
from .planning import policy as planner_policy, select_task
from .records import publish_json
from .resources import INHERITED, _slots, resource_environment
from .source_config import SourceConfiguration
from .source_history import git
from .source_inventory import source_snapshot
from .source_replica import artifact_parent
from .worker_worktree import prepare_worker


def _policy(root, value):
    required = {"schema_version", "author", "destination_branch", "planner", "driver", "worker", "reviewer",
                "checkpoint", "resources", "max_batches", "max_parallel", "iterations", "timeout", "publication", "evidence"}
    if (not isinstance(value, dict) or not required <= value.keys() or value.keys() - required - {"coordinator", "migration"}
            or type(value["schema_version"]) is not int or value["schema_version"] != 1):
        raise ValueError("invalid campaign configuration")
    for key in ("author", "destination_branch"):
        require_text(value[key], "campaign " + key)
    if not value["destination_branch"].startswith("refs/heads/"):
        raise ValueError("campaign requires an explicit destination branch ref")
    for key, maximum in (("max_batches", 100), ("max_parallel", 32), ("iterations", 100), ("timeout", 86400)):
        if type(value[key]) is not int or not 1 <= value[key] <= maximum:
            raise ValueError("campaign " + key + " is outside its bounded range")
    if not isinstance(value["evidence"], list) or not value["evidence"]:
        raise ValueError("campaign requires standing authorization evidence")
    for item in value["evidence"]:
        require_text(item, "campaign evidence")
    publication = value["publication"]
    if (not isinstance(publication, dict) or set(publication) != {"enabled", "allow_migration", "message", "cleanup"}
            or publication["enabled"] is not True or type(publication["allow_migration"]) is not bool
            or type(publication["cleanup"]) is not bool):
        raise ValueError("campaign requires explicit integration, migration and cleanup permissions")
    require_text(publication["message"], "campaign publication message")
    profiles = {key: _configuration(root, value[key]) for key in
                ("planner", "driver", "worker", "reviewer", "checkpoint", "resources", "coordinator", "migration") if key in value}
    planner_policy(profiles["planner"])
    if profiles["planner"]["author"] != value["author"]:
        raise ValueError("campaign and planner must have the same coordinator")
    for key in ("worker", "reviewer"):
        if set(profiles[key]) != {"author", "command"}:
            raise ValueError("campaign roles require author and command")
        require_text(profiles[key]["author"], "campaign role author")
        Command.read(profiles[key]["command"])
    if len({value["author"], profiles["worker"]["author"], profiles["reviewer"]["author"]}) != 3:
        raise ValueError("campaign requires independent coordinator, worker and reviewer identities")
    require_text(profiles["driver"].get("target"), "campaign target")
    if profiles["driver"].get("acceptance_scope") != "change":
        raise ValueError("campaign publication requires native change acceptance")
    checkpoint_authorization(profiles["checkpoint"], value["author"], profiles["worker"]["author"], profiles["reviewer"]["author"])
    if value["max_parallel"] > len(_slots(profiles["resources"], [root])):
        raise ValueError("campaign concurrency exceeds provisioned compiler capacity")
    return profiles


def _source_key(root):
    source = source_snapshot(root)
    return digest({"configuration": source["source_configuration"], "image": source["image_sha256"],
                   "files": {name: row["sha256"] for name, row in source["files"].items()}})


def _remove_integrated(root, child, branch, run_directory):
    if not child.is_relative_to(run_directory) or child.resolve() != child:
        raise ValueError("campaign cleanup covers only its own canonical worktrees")
    with file_lock(child / "build/recovery/driver.lock", timeout=0), git_options(root) as options:
        if preflight(child) != "refs/heads/" + branch:
            raise ValueError("campaign worker branch changed; preserve it")
        revision = git(child, "rev-parse", "HEAD").decode().strip()
        assert_raw_checkout(child, revision)
        git(root, "merge-base", "--is-ancestor", revision, "HEAD")
        # Git's ordinary removal refuses dirt; never force a worktree away.
        run_git(root, options, "worktree", "remove", str(child))
        run_git(root, options, "branch", "-d", branch)


def campaign(root: Path, configuration: dict, *, execute=invoke, progress=None):
    root = Path(root).absolute()
    if root.resolve() != root or INHERITED in os.environ:
        raise ValueError("campaign needs a canonical coordinator and separately leased worker resources")
    profiles = _policy(root, configuration)
    with file_lock(root / "build/recovery/driver.lock", timeout=0) as lease:
        if preflight(root) != configuration["destination_branch"]:
            raise ValueError("campaign is not on the authorized destination branch")
        initial = git(root, "rev-parse", "HEAD").decode().strip()
        with git_options(root) as options:
            expected = destination_state(root, initial, options)
        source_config = SourceConfiguration.read(root)
        parent = artifact_parent(root, "build/recovery/campaigns", excluded_roots=source_config.roots)
        directory = Path(tempfile.mkdtemp(prefix="run-", dir=parent))
        publish_json(directory / "authorization.json", configuration)
        attempted, batches = {}, []

        def announce(stage, **details):
            if progress is not None:
                progress({"stage": stage, **details})

        def fresh_policy():
            if (changes(expected, working_state(root)) or _policy(root, configuration) != profiles
                    or preflight(root) != configuration["destination_branch"]):
                raise ValueError("campaign policy or destination changed; preserve workers and reauthorize")

        def finish(reason):
            return {"schema_version": 1, "stop_reason": reason, "batches": batches, "artifacts": str(directory),
                    "initial_commit": initial, "final_commit": git(root, "rev-parse", "HEAD").decode().strip(),
                    "ready_for_collection": False, "full_project_accepted": False,
                    "unresolved_work": [item for batch in batches for item in (*batch["pending"], *batch["deferred"])],
                    "next_action": "inspect current source and pending work; reports are not acceptance inputs"}

        for number in range(configuration["max_batches"]):
            fresh_policy()
            source_key = _source_key(root)
            skip = {name for name, key in attempted.items() if key == source_key}
            selected, selections = [], []
            for _ in range(configuration["max_parallel"]):
                announce("select", batch=number)
                fresh_policy()
                choice = select_task(root, profiles["driver"]["target"], profiles["planner"], execute=execute,
                                     skip_tasks=skip | set(selected), parallel_tasks=selected,
                                     application_only=True, _held_lease=lease)
                expected = working_state(root)  # Only native task publication may change the coordinator here.
                selections.append(choice)
                if choice["action"] not in ("create", "resume"):
                    break
                selected.append(choice["task_id"])
            batch = {"index": number, "selections": selections, "workers": [], "integrations": [], "pending": [], "deferred": []}
            batches.append(batch)
            if not selected:
                return finish("planning_" + selections[-1]["action"])
            revision = git(root, "rev-parse", "HEAD").decode().strip()
            jobs = []
            for index, name in enumerate(selected):
                announce("prepare", batch=number, task=name)
                fresh_policy()
                path = directory / f"batch-{number}-worker-{index}"
                branch = f"recovery/campaign-{directory.name}-{number}-{index}"
                setup = prepare_worker(root, name, {"author": configuration["author"], "revision": revision,
                                      "prepare": True, "evidence": configuration["evidence"]},
                                       worktree=path, branch=branch, resources=profiles["resources"], _held_lease=lease)
                batch["workers"].append(setup)
                if not setup["compiler_prepared"]:
                    batch["pending"].append({"task": name, "reason": "compiler_preparation_failed", "worktree": str(path)})
                    return finish("preparation_required")
                jobs.append({"task": name, "root": str(path), "config": configuration["driver"],
                             "worker": configuration["worker"], "reviewer": configuration["reviewer"],
                             "checkpoint": configuration["checkpoint"], "iterations": configuration["iterations"],
                             **({"migration": configuration["migration"]} if "migration" in configuration else {})})
            plan = {"schema_version": 1, "author": configuration["author"], "evidence": configuration["evidence"],
                    "max_parallel": configuration["max_parallel"], "resources": configuration["resources"],
                    "timeout": configuration["timeout"], "jobs": jobs}
            publish_json(directory / f"batch-{number}-plan.json", plan)
            announce("dispatch", batch=number, tasks=selected)
            fresh_policy()
            batch["dispatch"] = dispatch(root, plan, _held_lease=lease,
                                         progress=lambda event: announce("worker", batch=number, **event))
            if batch["dispatch"]["coordinator_changed"]:
                return finish("coordinator_changed")
            for setup, outcome in zip(batch["workers"], batch["dispatch"]["jobs"], strict=True):
                name, child = setup["task_id"], Path(setup["worktree"])
                fresh_policy()
                if outcome["timed_out"] or not outcome["scope_intact"]:
                    batch["pending"].append({"task": name, "worktree": str(child), "reason": "interrupted_or_changed_scope"})
                    continue
                # After the parallel phase, a single central coordinator may
                # handle discoveries under separately supplied delegated scope.
                if "coordinator" in profiles:
                    announce("coordinate", batch=number, task=name)
                    with resource_environment(profiles["resources"], [child]):
                        repaired = drive(child, name, profiles["driver"], profiles["worker"], iterations=configuration["iterations"],
                                         reviewer_configuration=profiles["reviewer"], coordinator_configuration=profiles["coordinator"],
                                         migration_configuration=profiles.get("migration"), checkpoint_configuration=profiles["checkpoint"], execute=execute)
                    batch.setdefault("coordination", []).append(repaired)
                incoming = git(child, "rev-parse", "HEAD").decode().strip()
                try:
                    preflight(child)
                    assert_raw_checkout(child, incoming)
                except ValueError:
                    batch["pending"].append({"task": name, "worktree": str(child), "reason": "uncommitted_worker_progress"})
                    continue
                if incoming == revision:
                    attempted[name] = source_key
                    # No committed or uncommitted progress: defer this source
                    # snapshot, without losing source work or declaring it done.
                    batch["deferred"].append({"task": name, "worktree": str(child), "reason": "no_worker_checkpoint"})
                    if configuration["publication"]["cleanup"]:
                        _remove_integrated(root, child, setup["branch"], directory)
                        announce("cleaned", batch=number, task=name)
                    continue
                context = {"author": configuration["author"], "source_revision": incoming,
                           "target_revision": git(root, "rev-parse", "HEAD").decode().strip(), "evidence": configuration["evidence"]}
                announce("integrate", batch=number, task=name)
                fresh_policy()
                candidate = prepare_integration(root, name, context, worktree=directory / f"batch-{number}-candidate-{name}",
                                                branch=f"recovery/campaign-{directory.name}-{number}-candidate-{name}", _held_lease=lease)
                if not candidate["candidate_created"]:
                    batch["pending"].append({"task": name, "reason": "merge_conflicts", "candidate": candidate})
                    continue
                candidate_path = Path(candidate["worktree"])
                publication = configuration["publication"]
                with resource_environment(profiles["resources"], [root, candidate_path]):
                    integrated = publish_integration(root, name, candidate_path,
                        {"author": configuration["author"], "candidate_revision": candidate["commit"],
                         "destination_branch": configuration["destination_branch"], "integration": context,
                         "message": publication["message"], "allow_migration": publication["allow_migration"]},
                        configuration=profiles["driver"], worker_configuration=profiles["worker"], reviewer_configuration=profiles["reviewer"],
                        migration_configuration=profiles.get("migration"), execute=execute, _held_lease=lease)
                batch["integrations"].append(integrated)
                if not integrated["destination_verified"]:
                    batch["pending"].append({"task": name, "reason": integrated["stop_reason"], "candidate": str(candidate_path)})
                    if integrated.get("destination_updated") is not False:
                        return finish("destination_verification_required")
                    continue
                expected = working_state(root)  # Fresh native destination verification, never a worker receipt.
                attempted[name] = _source_key(root)
                if publication["cleanup"]:
                    _remove_integrated(root, candidate_path, candidate["branch"], directory)
                    _remove_integrated(root, child, setup["branch"], directory)
                    announce("cleaned", batch=number, task=name)
            if batch["pending"]:
                return finish("replanning_required")
            fresh_policy()
        return finish("batch_budget_exhausted")
