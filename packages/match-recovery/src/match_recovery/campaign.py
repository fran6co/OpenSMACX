"""Bounded multi-task coordination through native recovery and fresh publication.

Source/tasks/Git remain authoritative. Scheduling exclusions are in-memory and
expire when source changes; reports are not acceptance inputs. Interrupted or
conflicting work is preserved, never reset to make a campaign continue.
"""

import os
from contextlib import ExitStack
from pathlib import Path
import tempfile

from .checkpoints import authorization as checkpoint_authorization, _publication_changes, preflight
from .campaign_choices import ancestor, batches as saved_batches, coordinator_authority, integration_choice, latest_batch, load_run, manifest, retire, retirement, worker_choice
from .dispatch import _checkout, _configuration, dispatch
from .driver import Command, _resumable_history, changes, drive, invoke, working_state
from .git_worktree import assert_raw_checkout, destination_state, git_options, run_git
from .integration_prepare import prepare_integration
from .integration_publish import publish_integration
from .integration import decision as integration_decision
from .locking import file_lock
from .model import digest, require_text
from .planning import policy as planner_policy, select_task
from .campaign_peers import peer_epoch
from .records import publish_json
from .preparation import prepare_project
from .resources import INHERITED, _slots, resource_environment
from .reviews import ReviewStore
from .scheduling import _state as reservation_state, reservations
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


def _clean_checkout(root, revision):
    state = assert_raw_checkout(root, revision)
    if reservation_state(root) != state:
        raise ValueError("worktree has ignored recovery work; preserve it before retirement or cleanup")
    return state


def _remove_integrated(root, child, branch, run_directory):
    if not child.is_relative_to(run_directory) or child.resolve() != child:
        raise ValueError("campaign cleanup covers only its own canonical worktrees")
    with file_lock(child / "build/recovery/driver.lock", timeout=0), git_options(root) as options:
        if preflight(child) != "refs/heads/" + branch:
            raise ValueError("campaign worker branch changed; preserve it")
        revision = git(child, "rev-parse", "HEAD").decode().strip()
        _clean_checkout(child, revision)
        git(root, "merge-base", "--is-ancestor", revision, "HEAD")
        # Git's ordinary removal refuses dirt; never force a worktree away.
        run_git(root, options, "worktree", "remove", str(child))
        run_git(root, options, "branch", "-d", branch)


def _worker_state(root, name, path, branch, revision, profiles):
    """Inspect a preserved assignment while its caller holds the worker lease."""
    common, current_branch = _checkout(path)
    if common != _checkout(root)[0] or current_branch != "refs/heads/" + branch:
        raise ValueError("resumed worker repository or branch changed")
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
    # Compare raw bytes to the assignment, including committed edits. Reauthorization
    # never upgrades a worker's scripts or permits edits outside its existing task.
    _publication_changes(path, {**state, "head": (revision + "\n").encode()}, allowed)
    return state


def _existing_worker(root, name, path, branch, revision, profiles):
    _checkout(path)
    with file_lock(path / "build/recovery/driver.lock", timeout=0):
        state = _worker_state(root, name, path, branch, revision, profiles)
        task = TaskStore(path).read(name)
        with resource_environment(profiles["resources"], [path]):
            preparation = prepare_project(path)
        if changes(state, working_state(path)):
            raise ValueError("preparing a resumed worker changed its source; preserve it")
        prepared = preparation["result"] == "pass" and preparation["published"]
        return {"schema_version": 1, "task_id": name, "worker_created": False, "resumed": True,
                "worktree": str(path), "branch": branch, "revision": revision, "task_base_revision": task.base_revision,
                "compiler_prepared": prepared, "preparation": preparation, "ready_for_collection": False}


def _reauthorize_coordinator(root, directory, saved, configuration, profiles, authorization, reserved):
    """Publish only a delegation decision, with every existing assignment frozen."""
    if saved is None:
        raise ValueError("coordinator reauthorization requires a preserved batch")
    from .coordination import policy
    before = working_state(root)
    states, scopes, absent = {}, {}, []
    with ExitStack() as leases:
        for job, branch in zip(saved["plan"]["jobs"], saved["branches"], strict=True):
            path, name = Path(job["root"]), job["task"]
            retired = retirement(root, directory, saved["index"], name)
            if retired is not None:
                retained = [(path, branch, retired["worker_revision"])]
                choice_path = directory / f"batch-{saved['index']}-integration-{name}.json"
                if choice_path.exists():
                    choice = _configuration(directory, choice_path.name)
                    context = integration_decision(TaskStore(root).read(name), choice.get("context"))
                    if choice != integration_choice(directory, saved["index"], name, context):
                        raise ValueError("retired candidate destination changed")
                    retained.append((Path(choice["worktree"]), choice["branch"], None))
                for child, ref, revision in retained:
                    state = _retained_attempt(root, child, ref, directory, expected_revision=revision)
                    if state is None:
                        absent.append((child, ref, revision))
                    else:
                        leases.enter_context(file_lock(child / "build/recovery/driver.lock", timeout=0))
                        if reservation_state(child) != state:
                            raise ValueError("retired worker changed during reauthorization")
                        states[child] = state
                continue
            if path.resolve() != path or not path.is_dir():
                raise ValueError("coordinator reauthorization requires existing canonical workers")
            leases.enter_context(file_lock(path / "build/recovery/driver.lock", timeout=0))
            _worker_state(root, name, path, branch, saved["revision"], profiles)
            states[path] = reservation_state(path)
            store = TaskStore(path)
            policy(profiles["coordinator"], store.read(name))
            scopes[name] = store.scope(name)
        grants = (set(profiles["coordinator"]["subjects"]), set(profiles["coordinator"]["editable_files"]))
        # Existing tasks already authorize their own scope. A shared expansion
        # grant must not give another worker access to an already-owned group.
        for path in sorted((root / "recovery/tasks").glob("*.json")):
            name = path.stem
            scope = scopes.get(name)
            if scope is None:
                scope = TaskStore(root).scope(name)
            if any(grant & occupied for grant, occupied in zip(grants, scope, strict=True)):
                raise ValueError("coordinator expansion overlaps existing task ownership: " + name)
        reserved.check_scope("", *grants)

        def fresh():
            reserved.assert_current()
            if (working_state(root) != before or _policy(root, configuration) != profiles
                    or any(reservation_state(path) != state for path, state in states.items())):
                raise ValueError("reauthorization inputs changed; preserve assignments and retry fresh")
            for child, ref, revision in absent:
                if child.exists() or child.is_symlink():
                    raise ValueError("retired worker reappeared during reauthorization")
                _retained_attempt(root, child, ref, directory, expected_revision=revision)

        fresh()
        coordinator_authority(root, directory, configuration, profiles, authorization, publish=True)
        fresh()


def _scoped_coordinator(root, profile, name, jobs):
    """Shared grants exclude current peer ownership, including collected workers."""
    if profile is None:
        return None
    subjects, files = set(profile["subjects"]), set(profile["editable_files"])
    scopes = [TaskStore(root).scope(path.stem) for path in (root / "recovery/tasks").glob("*.json")
              if path.stem != name]
    for job in jobs:
        if job["task"] != name and Path(job["root"]).is_dir():
            scopes.append(TaskStore(Path(job["root"])).scope(job["task"]))
    for occupied, editable in scopes:
        subjects.difference_update(occupied)
        files.difference_update(editable)
    return {**profile, "subjects": sorted(subjects), "editable_files": sorted(files)}


def _resume_round(root, plan, profiles, directory, number, revision, *, execute, fresh, announce, lease):
    """Plan stopped-worker investigations centrally, then dispatch disjoint work.

    Original assignments remain immutable. Grants expire on any source/decision
    change and permit one local round; expanded scopes and pending integration
    stay on the existing serial coordination/publication path.
    """
    from .investigation_round import authorize
    from .coordination import SerialCoordinationRequired
    jobs, decisions, deferred = [], [], set()
    for job in plan["jobs"]:
        name, child = job["task"], Path(job["root"])
        incoming = git(child, "rev-parse", "HEAD").decode().strip()
        if ((incoming != revision and ancestor(root, incoming))
                or (directory / f"batch-{number}-integration-{name}.json").exists()
                or TaskStore(child).scope(name) != TaskStore(root).scope(name)):
            continue
        pending = TaskStore(child).status(name, TaskStore(child).scope(name)[0])["pending_discoveries"]
        if pending:
            if "coordinator" not in profiles:
                continue
            announce("authorize_investigation", batch=number, task=name)
            with peer_epoch(root, name, plan["jobs"]) as peers_fresh, resource_environment(profiles["resources"], [child]):
                def decide(*args, **kwargs):
                    fresh()
                    peers_fresh()
                    try:
                        return execute(*args, **kwargs)
                    finally:
                        peers_fresh()
                        fresh()
                try:
                    permission = authorize(child, name, _scoped_coordinator(root, profiles["coordinator"], name, plan["jobs"]),
                                           execute=decide)
                except SerialCoordinationRequired as error:
                    decisions.append({"task": name, "status": "serial_coordination_required", "reason": str(error)})
                    continue
            decisions.append({"task": name, **permission})
            if permission["status"] != "authorized":
                deferred.add(name)
                continue
            job = {**job, "iterations": 1, "investigation": permission["authorization"]}
        jobs.append(job)
    if not jobs:
        return None, decisions, deferred
    selected = {job["task"] for job in jobs}
    inactive = [job for job in plan["jobs"] if job["task"] not in selected]
    # Protect nonparticipating peers while active drivers hold their own leases.
    with peer_epoch(root, jobs[0]["task"], inactive):
        fresh()
        announce("dispatch", batch=number, tasks=[job["task"] for job in jobs], resumed=True)
        report = dispatch(root, {**plan, "jobs": jobs}, _held_lease=lease,
                          progress=lambda event: announce("worker", batch=number, **event))
        fresh()
    return report, decisions, deferred


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
            state = _clean_checkout(path, current)
            if not ancestor(root, current):
                raise ValueError("retired worktree contains unincorporated work; preserve it")
        if cleanup:
            _remove_integrated(root, path, branch, directory)
        return state if not cleanup else None
    # Removal may have completed just before interruption, leaving only its
    # branch. Never delete a moved branch or prune unrelated worktree metadata.
    with git_options(root) as options:
        # Without --quiet Git reports an absent ref as fatal (128), not the
        # documented presence-test result (1). Do not treat other fatal errors
        # as permission to forget a retained branch.
        exists = run_git(root, options, "show-ref", "--verify", "--quiet", "refs/heads/" + branch, codes=(0, 1))
        if exists.returncode:
            return
        ref = run_git(root, options, "show-ref", "--verify", "--hash", "refs/heads/" + branch)
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


def campaign(root: Path, configuration: dict, *, execute=invoke, progress=None, resume=None,
             coordinator_authorization=None):
    from .framework_runtime import fingerprint
    if fingerprint() is not None:
        raise ValueError("campaign runtime changes require separate run-bound authorization; not yet supported")
    root = Path(root).absolute()
    if root.resolve() != root or INHERITED in os.environ:
        raise ValueError("campaign needs a canonical coordinator and separately leased worker resources")
    if coordinator_authorization is not None and resume is None:
        raise ValueError("coordinator reauthorization requires --resume")
    profiles = _policy(root, configuration)
    with file_lock(root / "build/recovery/driver.lock", timeout=0) as lease, ExitStack() as stack:
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
            directory = load_run(root, parent, resume, configuration, profiles,
                                 coordinator_authorization=coordinator_authorization)
            saved = latest_batch(root, directory, configuration)
            expected = working_state(root)
            history = set()
            for job in saved["plan"]["jobs"] if saved else ():
                history.update(_resumable_history(root, TaskStore(root), ReviewStore(root, job["task"]), job["task"]))
            _publication_changes(root, expected, history)
        managed = set()
        for batch_choice in ([saved] if saved else []):
            for job in batch_choice["plan"]["jobs"]:
                managed.add(Path(job["root"]))
                candidate = integration_choice(directory, batch_choice["index"], job["task"], {})
                managed.add(Path(candidate["worktree"]))
        reserved = stack.enter_context(reservations(root, profiles["planner"].get("reserved_worktrees", []), managed=managed))
        retained = {}
        if "coordinator" in profiles:
            reserved.check_scope("", profiles["coordinator"].get("subjects", []), profiles["coordinator"].get("editable_files", []))
        if (coordinator_authorization is not None
                and coordinator_authority(root, directory, configuration, profiles) != digest(profiles)):
            _reauthorize_coordinator(root, directory, saved, configuration, profiles, coordinator_authorization, reserved)
        def external_freshness():
            reserved.assert_current()
            if any(reservation_state(path) != state for path, state in retained.items()):
                raise ValueError("retired campaign worktree changed; preserve work and replan")
        original_execute = execute
        def guarded_execute(*args, **kwargs):
            external_freshness()
            result = original_execute(*args, **kwargs)
            external_freshness()
            return result
        execute = guarded_execute
        attempted, batches = {}, []

        def announce(stage, **details):
            if progress is not None:
                progress({"stage": stage, **details})

        def fresh_policy():
            external_freshness()
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
                                         application_only=True, _held_lease=lease, _reservations=reserved)
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
                reserved.check_scope(name, *TaskStore(root).scope(name))
                managed.add(path)
                if resuming and path.exists():
                    reserved.check_scope(name, *TaskStore(path).scope(name))
                retired = retirement(root, directory, number, name) if resuming else None
                if retired is not None:
                    state = _retained_attempt(root, path, branch, directory, expected_revision=retired["worker_revision"],
                                              cleanup=configuration["publication"]["cleanup"])
                    if state is not None:
                        retained[path] = state
                    choice_path = directory / f"batch-{number}-integration-{name}.json"
                    if choice_path.exists():
                        previous = _configuration(directory, choice_path.name)
                        context = integration_decision(TaskStore(root).read(name), previous.get("context"))
                        if previous != integration_choice(directory, number, name, context):
                            raise ValueError("retired candidate destination changed")
                        state = _retained_attempt(root, Path(previous["worktree"]), previous["branch"], directory,
                                                  cleanup=configuration["publication"]["cleanup"])
                        if state is not None:
                            retained[Path(previous["worktree"])] = state
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
                fresh_policy()
                deferred_investigations = set()
            else:
                batch["dispatch"], decisions, deferred_investigations = _resume_round(
                    root, {**plan, "jobs": active_jobs}, profiles, directory, number, revision,
                    execute=execute, fresh=fresh_policy, announce=announce, lease=lease)
                batch["investigation_planning"] = decisions
                if batch["dispatch"] is not None and batch["dispatch"]["coordinator_changed"]:
                    return finish("coordinator_changed")
                current = {row["task"]: row for row in batch["dispatch"]["jobs"]} if batch["dispatch"] else {}
                outcomes = [current.get(job["task"]) for job in active_jobs]
            for setup, outcome in zip(batch["workers"], outcomes, strict=True):
                name, child = setup["task_id"], Path(setup["worktree"])
                fresh_policy()
                if name in deferred_investigations:
                    batch["pending"].append({"task": name, "worktree": str(child), "reason": "investigation_deferred"})
                    continue
                if outcome is not None and (outcome["timed_out"] or not outcome["scope_intact"]):
                    batch["pending"].append({"task": name, "worktree": str(child), "reason": "interrupted_or_changed_scope"})
                    continue
                # After the parallel phase, a single central coordinator may
                # handle discoveries under separately supplied delegated scope.
                current_incoming = git(child, "rev-parse", "HEAD").decode().strip()
                reserved.check_scope(name, *TaskStore(child).scope(name))
                incorporated = current_incoming != revision and ancestor(root, current_incoming)
                choice_file = directory / f"batch-{number}-integration-{name}.json"
                if ("coordinator" in profiles or resuming and outcome is None) and not incorporated and not choice_file.exists():
                    announce("coordinate", batch=number, task=name)
                    with peer_epoch(root, name, plan["jobs"]) as peers_fresh, resource_environment(profiles["resources"], [child]):
                        def coordinated_execute(*args, **kwargs):
                            fresh_policy()
                            peers_fresh()
                            try:
                                return execute(*args, **kwargs)
                            finally:
                                peers_fresh()
                                fresh_policy()
                        repaired = drive(child, name, profiles["driver"], profiles["worker"], iterations=configuration["iterations"],
                                         reviewer_configuration=profiles["reviewer"], coordinator_configuration=_scoped_coordinator(
                                             root, profiles.get("coordinator"), name, plan["jobs"]),
                                         migration_configuration=profiles.get("migration"), checkpoint_configuration=profiles["checkpoint"], execute=coordinated_execute)
                    batch.setdefault("coordination", []).append(repaired)
                incoming = git(child, "rev-parse", "HEAD").decode().strip()
                try:
                    preflight(child)
                    worker_state = _clean_checkout(child, incoming)
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
                    else:
                        retained[child] = worker_state
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
                managed.add(candidate_path)
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
                else:
                    retained[child] = worker_state
                    retained[candidate_path] = _clean_checkout(candidate_path, git(candidate_path, "rev-parse", "HEAD").decode().strip())
            if batch["pending"]:
                return finish("replanning_required")
            fresh_policy()
        return finish("batch_budget_exhausted")
