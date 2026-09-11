"""Invalid scope proposals stop one task, not the preserved campaign."""

import json
from pathlib import Path
import subprocess

import pytest

from match_recovery.coordination import SerialCoordinationRequired, coordinate
from match_recovery.driver import drive, working_state
from match_recovery.tasks import Task, TaskStore
from test_coordination import answer, discover, setup
from test_source_configuration import configured
from test_task_source import git, project


@pytest.mark.parametrize("case,action,reason", [
    ("outside", "reject", "outside_delegated_scope"),
    ("investigate", "investigate", "investigation_requires_expansion"),
    ("scope", "expand", "expansion_exceeds_delegation"),
    ("no_progress", "expand", "expansion_no_progress"),
    ("resolve", "incorporate", "resolution_requires_expansion"),
])
def test_scope_proposal_refusal_preserves_full_pending_finding_and_all_authority(setup, case, action, reason):
    root, store, task, caller, delegation = setup
    discover(setup)
    if case == "outside": delegation["subjects"] = []
    before = working_state(root), store.decisions(task.task_id), store.scope(task.task_id)
    seen = []
    def execute(command, current, name, payload):
        seen.append(payload)
        proposed = answer(payload, action)
        if case == "scope": proposed["editable_files"] = ["code/owner.cc"]
        if case == "no_progress": proposed["subjects"] = []
        return 0, proposed
    result = coordinate(root, task.task_id, delegation, execute=execute)
    assert set(result) == {"action", "published", "discovery_id", "snapshot", "proposed_action",
                           "reason_code", "feedback", "ready_for_collection", "verification"}
    assert result["action"] == "proposal_rejected" and result["reason_code"] == reason
    assert result["proposed_action"] == action and result["discovery_id"] == "layout"
    assert result["snapshot"] == seen[0]["snapshot"] and result["feedback"]
    assert not result["published"] and not result["ready_for_collection"]
    assert result["verification"] == "not_measured" and len(seen) == 1
    assert (working_state(root), store.decisions(task.task_id), store.scope(task.task_id)) == before
    assert store.read(task.task_id) == task
    pending, = store.status(task.task_id, store.scope(task.task_id)[0])["pending_discoveries"]
    assert pending["discovery_id"] == "layout" and pending["affects"] == [caller.entity_id]


@pytest.mark.parametrize("budget", [1, 5])
def test_driver_charges_one_decision_without_retry_worker_review_or_acceptance(setup, budget):
    root, store, task, caller, delegation = setup
    discover(setup)
    delegation.update(subjects=[], max_decisions=budget)
    configuration = {"schema_version": 1, "target": task.target,
                     **{role: {"argv": [role], "timeout": 10} for role in ("context", "check", "build", "acceptance")}}
    roles = {role: {"author": role, "command": {"argv": [role], "timeout": 10}} for role in ("worker", "reviewer")}
    before, calls = working_state(root), []
    def execute(command, current, name, payload):
        role = command.argv[0]
        calls.append(role)
        if role == "context": return 0, {}
        if role == "check": return 1, {"result": "fail"}
        assert role == "coordinate", "Refusal must not reach worker, review, build or acceptance"
        return 0, answer(payload, "reject")
    report = drive(root, task.task_id, configuration, roles["worker"], reviewer_configuration=roles["reviewer"],
                   coordinator_configuration=delegation, execute=execute, iterations=3)
    assert report["stop_reason"] == "coordination_proposal_rejected"
    assert report["coordinator_decisions"] == 1 and report["worker_rounds"] == 0
    assert report["coordination"]["reason_code"] == "outside_delegated_scope"
    assert not report["ready_for_collection"] and calls == ["context", "check", "coordinate"]
    assert working_state(root) == before and len(store.decisions(task.task_id)) == 1


def test_restricted_legal_publishing_proposal_still_requests_serial_coordination(setup):
    root, store, task, _, delegation = setup
    discover(setup)
    before = working_state(root)
    with pytest.raises(SerialCoordinationRequired):
        coordinate(root, task.task_id, delegation, allowed_actions=("investigate", "investigate_local", "defer"),
                   execute=lambda *args: (0, answer(args[-1], "expand")))
    assert working_state(root) == before and len(store.decisions(task.task_id)) == 1


@pytest.mark.parametrize("damage", ["source", "stale", "journal", "timeout"])
def test_unsafe_faults_still_propagate_instead_of_becoming_proposal_feedback(setup, damage):
    root, store, task, _, delegation = setup
    discover(setup)
    delegation["subjects"] = []
    def execute(command, current, name, payload):
        response = answer(payload, "reject")
        if damage == "source":
            (root / "code/owner.cc").write_text("Preserve unauthorized write")
        elif damage == "stale": response["snapshot"] = "f" * 64
        elif damage == "journal":
            store.discover(task.task_id, request_id="concurrent", author="worker", hypothesis="Concurrent finding",
                           affects=[task.subjects[0].entity_id], evidence=["source"], expected_revision=1)
        else: raise subprocess.TimeoutExpired("coordinator", 1)
        return 0, response
    with pytest.raises(subprocess.TimeoutExpired if damage == "timeout" else ValueError):
        coordinate(root, task.task_id, delegation, execute=execute)
    assert store.status(task.task_id, store.scope(task.task_id)[0])["pending_discoveries"]
    if damage == "source": assert (root / "code/owner.cc").read_text() == "Preserve unauthorized write"
    if damage == "journal": assert len(store.decisions(task.task_id)) == 2


def test_native_writer_failure_is_not_caught_as_a_recoverable_proposal(setup, monkeypatch):
    root, store, task, _, delegation = setup
    discover(setup)
    before = working_state(root)
    failure = ValueError("native journal publication failed")
    def failed(*args, **kwargs): raise failure
    monkeypatch.setattr(TaskStore, "expand", failed)
    with pytest.raises(ValueError) as raised:
        coordinate(root, task.task_id, delegation, execute=lambda *args: (0, answer(args[-1], "expand")))
    assert raised.value is failure and working_state(root) == before


def test_campaign_serial_refusal_preserves_clean_worker_and_continues_next_peer(setup, monkeypatch):
    import match_recovery.campaign as campaign

    root, store, owner, caller, delegation = setup
    discover(setup)
    store.create(Task("zpeer", owner.target, owner.author, "Recover separate caller", owner.base_revision,
                      (caller,), ("code/caller.cc",), ("Separate source ownership",)))
    delegation.update(subjects=[], editable_files=[])
    source_config = json.loads((root / "recovery/project.json").read_text())
    source_config["compiler"] = {"environment": ["TEST_COMPILER_SLOT"]}
    (root / "recovery/project.json").write_text(json.dumps(source_config))
    slots = []
    for number in range(2):
        slot = root / f"build/refusal-slot-{number}"
        slot.mkdir(parents=True)
        slots.append({"name": f"slot-{number}", "directory": str(slot),
                      "environment": {"TEST_COMPILER_SLOT": "{resource}"}})
    command = lambda name: {"argv": [name], "timeout": 10}
    profiles = {
        "planner": {"schema_version": 1, "author": "coordinator", "command": command("planner"),
                    "max_entities": 10, "max_editable_files": 4, "commit_decision": True, "evidence": ["Test planning"]},
        "driver": {"schema_version": 1, "target": owner.target, "acceptance_scope": "change",
                   **{name: command(name) for name in ("context", "check", "obligations", "migration_check")}},
        "worker": {"author": "worker", "command": command("worker")},
        "reviewer": {"author": "reviewer", "command": command("reviewer")},
        "checkpoint": {"author": "coordinator", "message": "Synthetic checkpoint", "allow_migration": False},
        "coordinator": delegation, "resources": {"schema_version": 1, "slots": slots},
    }
    for name, value in profiles.items():
        (root / f"recovery/{name}.json").write_text(json.dumps(value))
    configuration = {"schema_version": 1, "author": "coordinator", "destination_branch": "refs/heads/work",
                     **{name: f"recovery/{name}.json" for name in profiles}, "max_batches": 1, "max_parallel": 2,
                     "iterations": 1, "timeout": 30, "evidence": ["Test isolated serial refusal handling"],
                     "publication": {"enabled": True, "allow_migration": False, "message": "Test publication", "cleanup": True}}
    git(root, "add", ".")
    git(root, "commit", "-m", "Synthetic bounded two-task campaign")
    before = working_state(root)
    selected = iter(("owner", "zpeer"))
    monkeypatch.setattr(campaign, "select_task", lambda *args, **kwargs:
                        {"action": "resume", "task_id": next(selected)})
    native_prepare = campaign.prepare_worker
    def prepare(current, name, authorization, **kwargs):
        # Native raw checkout/task preservation, with compiler readiness doubled
        # only to reach the orchestration branch; no compiler proof is claimed.
        result = native_prepare(current, name, {**authorization, "prepare": False},
                                **{**kwargs, "resources": None})
        return {**result, "compiler_prepared": True}
    monkeypatch.setattr(campaign, "prepare_worker", prepare)
    monkeypatch.setattr(campaign, "dispatch", lambda current, plan, **kwargs:
                        {"coordinator_changed": False, "jobs": [{"task": job["task"], "scope_intact": True,
                           "timed_out": False, "returncode": 3} for job in plan["jobs"]]})
    monkeypatch.setattr(campaign, "prepare_integration", lambda *args, **kwargs: pytest.fail("Refused task must not integrate"))
    monkeypatch.setattr(campaign, "publish_integration", lambda *args, **kwargs: pytest.fail("Refused task must not publish"))
    driven, cleaned, seen = [], [], []
    native_cleanup = campaign._remove_integrated
    def cleanup(current, child, branch, directory):
        assert child != seen[0], "A clean worker with rejected pending discovery must not be retired"
        cleaned.append(child)
        return native_cleanup(current, child, branch, directory)
    monkeypatch.setattr(campaign, "_remove_integrated", cleanup)
    def handle(child, name, *args, **kwargs):
        driven.append(name)
        seen.append(child)
        if name == "owner":
            assert not git(child, "status", "--porcelain")
            return drive(child, name, *args, **kwargs)
        return {"stop_reason": "no_source_progress", "ready_for_collection": False}
    monkeypatch.setattr(campaign, "drive", handle)
    def execute(command, child, name, payload):
        role = command.argv[0]
        if role == "obligations": return 0, {"obligations": []}
        if role == "context": return 0, {}
        if role == "check": return 1, {"result": "fail"}
        assert role == "coordinate" and name == "owner"
        return 0, answer(payload, "reject")
    report = campaign.campaign(root, configuration, execute=execute)
    assert driven == ["owner", "zpeer"] and cleaned == [seen[1]]
    assert seen[0].is_dir() and not seen[1].exists()
    batch, = report["batches"]
    assert batch["pending"][0]["task"] == "owner"
    assert batch["pending"][0]["reason"] == "coordination_proposal_rejected"
    assert not batch["integrations"] and not report["ready_for_collection"] and not report["full_project_accepted"]
    assert not list(Path(report["artifacts"]).glob("*retired-owner.json"))
    assert TaskStore(seen[0]).read("owner") == owner
    assert TaskStore(seen[0]).decisions("owner") == store.decisions("owner")
    assert working_state(root) == before
