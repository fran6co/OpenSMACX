"""Bounded multi-task coordination through native recovery and fresh publication.

Source/tasks/Git remain authoritative. Scheduling exclusions are in-memory and
expire when source changes; reports are not acceptance inputs. Interrupted or
conflicting work is preserved, never reset to make a campaign continue.
"""

import os
from pathlib import Path
import tempfile

from .checkpoints import authorization as checkpoint_authorization, _publication_changes, preflight
from .campaign_choices import ancestor, batches as saved_batches, integration_choice, latest_batch, load_run, manifest, retire, retirement, worker_choice
from .dispatch import _checkout, _configuration, dispatch
from .driver import Command, _resumable_history, changes, drive, invoke, working_state
from .git_worktree import assert_raw_checkout, destination_state, git_options, run_git
from .integration_prepare import prepare_integration
from .integration_publish import publish_integration
from .integration import decision as integration_decision
from .locking import file_lock
from .model import digest, require_text
from .planning import policy as planner_policy, select_task
from .records import publish_json
from .preparation import prepare_project
from .resources import INHERITED, _slots, resource_environment
from .reviews import ReviewStore
from .source_config import SourceConfiguration
from .source_history import git
from .source_inventory import source_snapshot
from .source_replica import artifact_parent
from .tasks import TaskStore
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


def _existing_worker(root, name, path, branch, revision, profiles):
    common, current_branch = _checkout(path)
    if common != _checkout(root)[0] or current_branch != "refs/heads/" + branch:
        raise ValueError("resumed worker repository or branch changed")
    with file_lock(path / "build/recovery/driver.lock", timeout=0):
        preflight(path)
        if not ancestor(path, revision):
            raise ValueError("resumed worker lost its authorized starting revision")
        store = TaskStore(path)
        task = store.read(name)
        if (task != TaskStore(root).read(name) or task.author != profiles["planner"]["author"]
                or task.target != profiles["driver"]["target"]):
            raise ValueError("resumed worker changed its task identity or baseline")
        allowed = set(store.scope(name)[1]) | _resumable_history(path, store, ReviewStore(path, name), name)
        state = working_state(path)
        # Compare raw current bytes to the assigned starting commit, including
        # committed edits. Policy changes cannot run a modified compiler script.
        _publication_changes(path, {**state, "head": (revision + "\n").encode()}, allowed)
        with resource_environment(profiles["resources"], [path]):
            preparation = prepare_project(path)
        if changes(state, working_state(path)):
            raise ValueError("preparing a resumed worker changed its source; preserve it")
        prepared = preparation["result"] == "pass" and preparation["published"]
        return {"schema_version": 1, "task_id": name, "worker_created": False, "resumed": True,
                "worktree": str(path), "branch": branch, "revision": revision, "task_base_revision": task.base_revision,
                "compiler_prepared": prepared, "preparation": preparation, "ready_for_collection": False}


def _retained_attempt(root, path, branch, directory, *, expected_revision=None, cleanup=False):
    """Inspect actual retained content even when cleanup is disabled."""
    if path.exists() or path.is_symlink():
        if _checkout(path) != (_checkout(root)[0], "refs/heads/" + branch):
            raise ValueError("retired worktree repository or branch changed")
        with file_lock(path / "build/recovery/driver.lock", timeout=0):
            preflight(path)
            current = git(path, "rev-parse", "HEAD").decode().strip()
            if expected_revision is not None and current != expected_revision:
                raise ValueError("retired worker has additional work; preserve and reauthorize")
            assert_raw_checkout(path, current)
            if not ancestor(root, current):
                raise ValueError("retired worktree contains unincorporated work; preserve it")
        if cleanup:
            _remove_integrated(root, path, branch, directory)
        return
    # Removal may have completed just before interruption, leaving only its
    # branch. Never delete a moved branch or prune unrelated worktree metadata.
    with git_options(root) as options:
        ref = run_git(root, options, "show-ref", "--verify", "--hash", "refs/heads/" + branch, codes=(0, 1))
        if ref.returncode:
            return
        current = ref.stdout.decode().strip()
        if (expected_revision is not None and current != expected_revision) or not ancestor(root, current):
            raise ValueError("retired branch contains additional work; preserve it")
        if cleanup:
            run_git(root, options, "branch", "-d", branch)


def _pending_runs(root, parent):
    pending = []
    with git_options(root) as options:
        def exists(path, branch):
            return (path.exists() or path.is_symlink() or run_git(root, options, "show-ref", "--verify", "--quiet",
                    "refs/heads/" + branch, codes=(0, 1)).returncode == 0)
        for directory in sorted(parent.glob("run-*")):
            if directory.resolve() != directory or not directory.is_dir():
                raise ValueError("campaign storage contains a noncanonical run; preserve and inspect it")
            if not (directory / "run.json").exists():
                continue  # No recorded assignments from this protocol.
            record = _configuration(directory, "run.json")
            if record.get("root") != str(root) or not isinstance(record.get("configuration"), dict):
                raise ValueError("campaign storage belongs to a different coordinator")
            for batch in saved_batches(root, directory, record["configuration"]):
                number = batch["index"]
                for job, branch in zip(batch["plan"]["jobs"], batch["branches"], strict=True):
                    name, path = job["task"], Path(job["root"])
                    candidate = integration_choice(directory, number, name, {})
                    candidate_path = Path(candidate["worktree"])
                    if not exists(path, branch) and not exists(candidate_path, candidate["branch"]):
                        continue
                    retired = retirement(root, directory, number, name)
                    if retired is None:
                        if directory not in pending:
                            pending.append(directory)
                    else:
                        _retained_attempt(root, path, branch, directory, expected_revision=retired["worker_revision"])
                        _retained_attempt(root, candidate_path, candidate["branch"], directory)
    return pending


def campaign(root: Path, configuration: dict, *, execute=invoke, progress=None, resume=None):
    root = Path(root).absolute()
    if root.resolve() != root or INHERITED in os.environ:
        raise ValueError("campaign needs a canonical coordinator and separately leased worker resources")
    profiles = _policy(root, configuration)
    with file_lock(root / "build/recovery/driver.lock", timeout=0) as lease:
        if preflight(root) != configuration["destination_branch"]:
            raise ValueError("campaign is not on the authorized destination branch")
        initial = git(root, "rev-parse", "HEAD").decode().strip()
        source_config = SourceConfiguration.read(root)
        parent = artifact_parent(root, "build/recovery/campaigns", excluded_roots=source_config.roots)
        if resume is None:
            pending = _pending_runs(root, parent)
            if pending:
                raise ValueError("preserved campaign work must be resumed before starting another run: --resume " + str(pending[0]))
            with git_options(root) as options:
                expected = destination_state(root, initial, options)
            directory = Path(tempfile.mkdtemp(prefix="run-", dir=parent))
            publish_json(directory / "run.json", manifest(root, configuration, profiles, initial))
            saved = None
        else:
            directory = load_run(root, parent, resume, configuration, profiles)
            saved = latest_batch(root, directory, configuration)
            expected = working_state(root)
            history = set()
            for job in saved["plan"]["jobs"] if saved else ():
                history.update(_resumable_history(root, TaskStore(root), ReviewStore(root, job["task"]), job["task"]))
            _publication_changes(root, expected, history)
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

        start_number = saved["index"] if saved else 0
        for number in range(start_number, start_number + configuration["max_batches"]):
            fresh_policy()
            source_key = _source_key(root)
            skip = {name for name, key in attempted.items() if key == source_key}
            selected, selections = [], []
            resuming = saved is not None
            if resuming:
                selected = [row["task"] for row in saved["plan"]["jobs"]]
                selections = [{"action": "resume_worker", "task_id": name} for name in selected]
            else:
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
            revision = saved["revision"] if resuming else git(root, "rev-parse", "HEAD").decode().strip()
            choice = saved or worker_choice(directory, number, revision, selected, configuration)
            publish_json(directory / f"batch-{number}-workers.json", choice, allow_identical=True)
            saved = None
            plan = choice["plan"]
            active_jobs = []
            for index, name in enumerate(selected):
                announce("prepare", batch=number, task=name)
                fresh_policy()
                job = plan["jobs"][index]
                path, branch = Path(job["root"]), choice["branches"][index]
                retired = retirement(root, directory, number, name) if resuming else None
                if retired is not None:
                    _retained_attempt(root, path, branch, directory, expected_revision=retired["worker_revision"],
                                      cleanup=configuration["publication"]["cleanup"])
                    choice_path = directory / f"batch-{number}-integration-{name}.json"
                    if choice_path.exists():
                        previous = _configuration(directory, choice_path.name)
                        context = integration_decision(TaskStore(root).read(name), previous.get("context"))
                        if previous != integration_choice(directory, number, name, context):
                            raise ValueError("retired candidate destination changed")
                        _retained_attempt(root, Path(previous["worktree"]), previous["branch"], directory,
                                          cleanup=configuration["publication"]["cleanup"])
                    batch.setdefault("retained_in_history", []).append({"task": name, "commit": retired["worker_revision"],
                                                                       "verification": "not_replayed"})
                    if retired["destination_revision"] == git(root, "rev-parse", "HEAD").decode().strip():
                        attempted[name] = source_key
                    continue
                if resuming and path.exists():
                    setup = _existing_worker(root, name, path, branch, revision, profiles)
                else:
                    setup = prepare_worker(root, name, {"author": configuration["author"], "revision": revision,
                                          "prepare": True, "evidence": configuration["evidence"]},
                                           worktree=path, branch=branch, resources=profiles["resources"], _held_lease=lease)
                batch["workers"].append(setup)
                if not setup["compiler_prepared"]:
                    batch["pending"].append({"task": name, "reason": "compiler_preparation_failed", "worktree": str(path)})
                    return finish("preparation_required")
                active_jobs.append(job)
            if not resuming:
                announce("dispatch", batch=number, tasks=selected)
                fresh_policy()
                batch["dispatch"] = dispatch(root, plan, _held_lease=lease,
                                             progress=lambda event: announce("worker", batch=number, **event))
                if batch["dispatch"]["coordinator_changed"]:
                    return finish("coordinator_changed")
                outcomes = batch["dispatch"]["jobs"]
            else:
                batch["dispatch"] = None  # No old process outcome is an input to resumption.
                outcomes = [None] * len(active_jobs)
            for setup, outcome in zip(batch["workers"], outcomes, strict=True):
                name, child = setup["task_id"], Path(setup["worktree"])
                fresh_policy()
                if outcome is not None and (outcome["timed_out"] or not outcome["scope_intact"]):
                    batch["pending"].append({"task": name, "worktree": str(child), "reason": "interrupted_or_changed_scope"})
                    continue
                # After the parallel phase, a single central coordinator may
                # handle discoveries under separately supplied delegated scope.
                current_incoming = git(child, "rev-parse", "HEAD").decode().strip()
                incorporated = current_incoming != revision and ancestor(root, current_incoming)
                choice_file = directory / f"batch-{number}-integration-{name}.json"
                if ("coordinator" in profiles or resuming) and not incorporated and not choice_file.exists():
                    announce("coordinate", batch=number, task=name)
                    with resource_environment(profiles["resources"], [child]):
                        repaired = drive(child, name, profiles["driver"], profiles["worker"], iterations=configuration["iterations"],
                                         reviewer_configuration=profiles["reviewer"], coordinator_configuration=profiles.get("coordinator"),
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
                    retire(directory, number, name, incoming, git(root, "rev-parse", "HEAD").decode().strip(), reason="no_checkpoint")
                    if configuration["publication"]["cleanup"]:
                        _remove_integrated(root, child, setup["branch"], directory)
                        announce("cleaned", batch=number, task=name)
                    continue
                context = {"author": configuration["author"], "source_revision": incoming,
                           "target_revision": git(root, "rev-parse", "HEAD").decode().strip(), "evidence": configuration["evidence"]}
                announce("integrate", batch=number, task=name)
                fresh_policy()
                if resuming and choice_file.exists():
                    intention = _configuration(directory, choice_file.name)
                    context = integration_decision(TaskStore(root).read(name), intention.get("context"))
                    if intention != integration_choice(directory, number, name, context):
                        raise ValueError("saved integration destinations changed")
                    if context["source_revision"] != incoming:
                        raise ValueError("worker advanced beyond its integration choice; preserve candidates and reauthorize")
                else:
                    intention = integration_choice(directory, number, name, context)
                    publish_json(choice_file, intention)
                candidate_path = Path(intention["worktree"])
                if resuming and candidate_path.exists():
                    if _checkout(candidate_path) != (_checkout(root)[0], "refs/heads/" + intention["branch"]):
                        raise ValueError("resumed candidate repository or branch changed")
                    candidate = {"candidate_created": True, "worktree": str(candidate_path), "branch": intention["branch"],
                                 "commit": git(candidate_path, "rev-parse", "HEAD").decode().strip()}
                else:
                    candidate = prepare_integration(root, name, context, worktree=candidate_path,
                                                    branch=intention["branch"], _held_lease=lease)
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
                        migration_configuration=profiles.get("migration"), execute=execute, _held_lease=lease, resume=incorporated)
                if incorporated:
                    integrated["resumed_after_transfer"] = True
                batch["integrations"].append(integrated)
                if not integrated["destination_verified"]:
                    batch["pending"].append({"task": name, "reason": integrated["stop_reason"], "candidate": str(candidate_path)})
                    if incorporated or integrated.get("destination_updated") is not False:
                        return finish("destination_verification_required")
                    continue
                expected = working_state(root)  # Fresh native destination verification, never a worker receipt.
                attempted[name] = _source_key(root)
                retire(directory, number, name, incoming, git(root, "rev-parse", "HEAD").decode().strip(), reason="integrated")
                if publication["cleanup"]:
                    _remove_integrated(root, candidate_path, candidate["branch"], directory)
                    _remove_integrated(root, child, setup["branch"], directory)
                    announce("cleaned", batch=number, task=name)
            if batch["pending"]:
                return finish("replanning_required")
            fresh_policy()
        return finish("batch_budget_exhausted")
