"""Real Git collection with deterministic protocol doubles, not game acceptance."""

from pathlib import Path
from dataclasses import asdict

import pytest

from match_recovery.driver import working_state
from match_recovery.integration_publish import publish_integration
from match_recovery.locking import FileLease, file_lock
from match_recovery.model import Dimension, Obligation
from match_recovery.tasks import TaskStore
from test_integration_prepare import inputs, prepare, project
from test_task_source import git


CONFIG = {"schema_version": 1, "target": "portable", "acceptance_scope": "change",
          **{name: {"argv": [command], "timeout": 10} for name, command in
             (("context", "context"), ("check", "check"), ("obligations", "inventory"), ("migration_check", "migration"))}}
WORKER = {"author": "worker", "command": {"argv": ["worker"], "timeout": 10}}
REVIEWER = {"author": "reviewer", "command": {"argv": ["review"], "timeout": 10}}


@pytest.fixture
def collection(inputs):
    root, context = inputs
    result = prepare(inputs)
    candidate = Path(result["worktree"])
    (candidate / "build").mkdir(exist_ok=True)
    permission = {"author": "coordinator", "candidate_revision": result["commit"], "destination_branch": "refs/heads/destination",
                  "integration": context, "message": "Collect test-owned ownership", "allow_migration": False}
    return root, candidate, permission


def collect(collection, *, effect=None, permission=None, regression=False, matching_permission=False):
    root, candidate, original = collection
    task = TaskStore(candidate).read("owner")
    obligation = Obligation(task.subjects[0].entity_id, Dimension.BYTES, "git-history:" + task.base_revision)
    protected = {**asdict(obligation), "obligation_id": obligation.obligation_id}
    migration = ({"migration_id": "collected-layout", "author": task.author, "evidence": ["test-owned scoped regression"],
                  "allowed": [{"entity_id": obligation.entity_id, "dimension": "bytes"}]} if matching_permission else None)
    calls = []
    def execute(command, checkout, name, payload):
        stage = command.argv[0]
        calls.append((checkout, stage))
        if effect:
            effect(checkout, stage, payload)
        if stage == "context":
            return 0, {"source": "code/owner.cc"}
        if stage == "inventory":
            return 0, {"obligations": [protected] if regression else []}
        if stage == "check":
            assert payload["context"]["collection_context"]["destination"] == str(root)
            return (1 if regression else 0), {"result": "fail" if regression else "pass"}
        if stage == "review":
            return 0, {"snapshot": payload["snapshot"], "decision": "approve",
                       "evidence": ["test-owned protocol evidence"], "feedback": "Independent fixture review"}
        assert stage == "migration", "collection must not invoke a worker"
        result = "unknown" if (checkout / "build/refuse").exists() else "pass"
        return 0, {"snapshot": payload["snapshot"], "changed": [obligation.entity_id] if regression else [], "build": "pass", "structure": "pass",
                   "references": "pass", "outside_scope": result,
                   "measurements": [{"obligation_id": obligation.obligation_id, "fingerprint": "a" * 64,
                                      "result": "fail", "evidence": "test-owned measured regression"}] if regression else [],
                   "current": {obligation.obligation_id: "a" * 64} if regression else {}}
    return publish_integration(root, "owner", candidate, permission or original, configuration=CONFIG,
                               worker_configuration=WORKER, reviewer_configuration=REVIEWER,
                               migration_configuration=migration, execute=execute), calls


def test_collect_verifies_both_worktrees_and_fast_forwards_only_authorized_destination(collection):
    root, candidate, permission = collection
    task = TaskStore(candidate).read("owner")
    result, calls = collect(collection)
    assert result["stop_reason"] == "destination_accepted" and result["destination_updated"] and result["destination_verified"], result
    assert not result["ready_for_collection"]
    for path in (candidate, root):
        assert calls.count((path, "check")) == calls.count((path, "review")) == calls.count((path, "migration")) == 1
        assert TaskStore(path).read("owner") == task
        assert not git(path, "status", "--porcelain")
        assert "return 2" in (path / "code/owner.cc").read_text()
    assert git(root, "rev-parse", "HEAD") == result["final_commit"]
    assert git(candidate, "rev-parse", "HEAD") == result["verified_candidate_commit"]
    assert git(root, "rev-parse", "incoming") == permission["integration"]["source_revision"]
    git(root, "merge-base", "--is-ancestor", permission["integration"]["target_revision"], result["final_commit"])
    assert "Collection authorization:" in git(root, "log", "-1", "--format=%B")


def test_collection_can_introduce_the_original_incoming_task_without_reauthoring_it(inputs):
    root, context = inputs
    task = TaskStore(root).read("owner")
    git(root, "switch", "-c", "first-destination", task.base_revision)
    context = {**context, "target_revision": task.base_revision}
    assert not (root / "recovery/tasks/owner.json").exists()
    prepared = prepare((root, context))
    candidate = Path(prepared["worktree"])
    permission = {"author": task.author, "candidate_revision": prepared["commit"], "destination_branch": "refs/heads/first-destination",
                  "integration": context, "message": "Collect first authorized task", "allow_migration": False}
    result, _ = collect((root, candidate, permission))
    assert result["stop_reason"] == "destination_accepted" and TaskStore(root).read("owner") == task


def test_candidate_refusal_does_not_move_destination(collection):
    root, candidate, _ = collection
    before = working_state(root)
    (candidate / "build/refuse").write_text("missing evidence")
    result, calls = collect(collection)
    assert result["stop_reason"] == "candidate_verification_required" and not result["destination_updated"]
    assert not result["destination_verified"] and working_state(root) == before
    assert all(checkout == candidate for checkout, _ in calls)


@pytest.mark.parametrize("publish_permission,matching_permission", [(False, True), (True, False), (True, True)])
def test_collected_migration_needs_both_permissions_and_never_reports_full_acceptance(collection, publish_permission, matching_permission):
    root, _, permission = collection
    before = working_state(root)
    result, _ = collect(collection, permission={**permission, "allow_migration": publish_permission},
                        regression=True, matching_permission=matching_permission)
    if publish_permission and matching_permission:
        assert result["stop_reason"] == "destination_migration_admitted" and result["destination_verified"]
        assert result["candidate_verification"]["migration"]["exit_code"] == 2
        assert result["destination_verification"]["migration"]["exit_code"] == 2
    else:
        assert result["stop_reason"] == "candidate_verification_required" and not result["destination_updated"]
        assert working_state(root) == before
    assert not result["ready_for_collection"]


def test_post_fast_forward_failure_is_not_accepted_or_reset(collection):
    root, _, permission = collection
    (root / "build/refuse").write_text("destination evidence unavailable")
    result, calls = collect(collection)
    assert result["stop_reason"] == "destination_verification_required" and result["destination_updated"]
    assert not result["destination_verified"]
    assert git(root, "rev-parse", "HEAD") == result["verified_candidate_commit"]
    assert "return 2" in (root / "code/owner.cc").read_text()
    assert calls.count((root, "migration")) == 1
    git(root, "merge-base", "--is-ancestor", permission["integration"]["target_revision"], result["final_commit"])


@pytest.mark.parametrize("damage", ["destination_head", "candidate_head", "destination_branch", "author", "staged", "dirty", "mode", "candidate_dirty"])
def test_stale_authority_and_dirty_work_are_preserved_before_dispatch(collection, damage):
    root, candidate, permission = collection
    if damage == "destination_head":
        git(root, "commit", "--allow-empty", "-m", "Concurrent coordinator")
    elif damage == "candidate_head":
        git(candidate, "-c", "user.name=Fixture", "-c", "user.email=fixture@example.invalid", "commit", "--allow-empty", "-m", "Different candidate")
    elif damage in ("author", "destination_branch"):
        permission = {**permission, damage: "worker" if damage == "author" else "refs/heads/other"}
    elif damage == "mode":
        git(root, "config", "core.fileMode", "false")
        (root / "code/owner.cc").chmod(0o755)
    else:
        path = (candidate if damage == "candidate_dirty" else root) / "code/owner.cc"
        path.write_text("user edit\n")
        if damage == "staged":
            git(root, "add", "code/owner.cc")
    before = working_state(root), working_state(candidate)
    with pytest.raises(ValueError):
        collect(collection, permission=permission, effect=lambda *_: pytest.fail("no command may run"))
    assert (working_state(root), working_state(candidate)) == before


def test_destination_race_during_candidate_verification_preserves_user_edit(collection):
    root, candidate, _ = collection
    head = git(root, "rev-parse", "HEAD")
    def race(checkout, stage, _):
        if checkout == candidate and stage == "check":
            (root / "code/caller.cc").write_text("concurrent destination work\n")
    with pytest.raises(ValueError, match="destination changed"):
        collect(collection, effect=race)
    assert git(root, "rev-parse", "HEAD") == head
    assert (root / "code/caller.cc").read_text() == "concurrent destination work\n"


def test_post_update_exception_reports_unverified_and_keeps_source(collection):
    root, _, _ = collection
    def interrupted(checkout, stage, _):
        if checkout == root and stage == "check":
            raise ValueError("destination verifier unavailable")
    result, _ = collect(collection, effect=interrupted)
    assert result["stop_reason"] == "destination_verification_required" and result["destination_updated"]
    assert result["observed_head"] == result["verified_candidate_commit"]
    assert not result["destination_verified"] and "verifier unavailable" in result["error"]
    assert "return 2" in (root / "code/owner.cc").read_text()


def test_hooks_and_autostash_do_not_participate(collection):
    root, _, _ = collection
    git(root, "config", "merge.autostash", "true")
    hook = root / ".git/hooks/post-merge"
    hook.write_text("#!/bin/sh\nprintf bad > code/owner.cc\n")
    hook.chmod(0o755)
    result, _ = collect(collection)
    assert result["destination_verified"] and "return 2" in (root / "code/owner.cc").read_text()
    assert git(root, "config", "merge.autostash") == "true"
    assert not git(root, "stash", "list")


def test_active_filter_refuses_without_executing_it(collection):
    root, _, _ = collection
    (root / ".git/info/attributes").write_text("*.cc filter=damage\n")
    git(root, "config", "filter.damage.clean", "touch filter-ran; cat")
    git(root, "config", "filter.damage.smudge", "touch filter-ran; cat")
    before = working_state(root)
    with pytest.raises(ValueError, match="checkout filters"):
        collect(collection)
    assert not (root / "filter-ran").exists() and working_state(root) == before


def test_nested_lease_is_bound_to_path_and_lifetime(tmp_path):
    path = tmp_path / "coordinator.lock"
    with pytest.raises(ValueError, match="active local"):
        with file_lock(path, held=FileLease(path)):
            pytest.fail("an unacquired token is not a lease")
    with file_lock(path) as lease:
        with pytest.raises(AttributeError):
            lease.path = tmp_path / "other.lock"
        with file_lock(path, held=lease):
            assert lease.active
        assert lease.active
        with pytest.raises(ValueError, match="active local"):
            with file_lock(tmp_path / "other.lock", held=lease):
                pytest.fail("wrong lease")
        with pytest.raises(TimeoutError):
            with file_lock(path, timeout=0):
                pytest.fail("lease was dropped")
    with pytest.raises(ValueError, match="active local"):
        with file_lock(path, held=lease):
            pytest.fail("expired lease")


def test_ignored_destination_history_collision_is_not_overwritten(collection, monkeypatch):
    from match_recovery import integration_publish
    root, candidate, permission = collection
    head = git(root, "rev-parse", "HEAD")
    occupied = []
    original = integration_publish._checkout_policy
    def collision(checkout, revision, options):
        entries = original(checkout, revision, options)
        if checkout == root and revision not in (head, permission["candidate_revision"]):
            # This review is already committed in the candidate. A concurrent
            # ignored file at its destination must not be overwritten by Git.
            history = next((candidate / "recovery/history").rglob("*.json"))
            path = root / history.relative_to(candidate)
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text("ignored user evidence\n")
            (root / ".git/info/exclude").write_text("/" + path.relative_to(root).as_posix() + "\n")
            occupied.append(path)
        return entries
    monkeypatch.setattr(integration_publish, "_checkout_policy", collision)
    with pytest.raises(ValueError, match="ignored destination path"):
        collect(collection)
    assert len(occupied) == 1 and occupied[0].read_text() == "ignored user evidence\n"
    assert git(root, "rev-parse", "HEAD") == head


def test_hardlinked_destination_output_is_not_overwritten(collection):
    root, _, _ = collection
    source = root / "code/owner.cc"
    linked = root / "build/user-copy"
    linked.hardlink_to(source)
    before = source.read_bytes(), working_state(root)
    with pytest.raises(ValueError, match="hardlinked destination"):
        collect(collection)
    assert source.read_bytes() == linked.read_bytes() == before[0]
    assert working_state(root) == before[1]
