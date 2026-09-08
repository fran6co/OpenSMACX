"""Final verification must not turn a failed collection check into new repairs."""

import pytest

from match_recovery.driver import drive, working_state
from match_recovery.reviews import ReviewStore
from match_recovery.tasks import TaskStore
from test_migration_driver import CONFIGURED, WORKER, REVIEWER, project, setup


def verify(project, *, restored=True, decision="approve", damage=None, checkpoint=None):
    permission, original, calls = setup(project, restored=restored, damage=damage)
    permission["allowed"] = []

    def execute(command, root, name, payload):
        assert command.argv[0] != "worker", "verification-only must never dispatch a repair"
        code, response = original(command, root, name, payload)
        if command.argv[0] == "review":
            response["decision"] = decision
        return code, response

    report = drive(project[0], project[1].task_id, {**CONFIGURED, "acceptance_scope": "change"}, WORKER,
                   iterations=10, reviewer_configuration=REVIEWER, migration_configuration=permission,
                   checkpoint_configuration=checkpoint, verify_only=True, execute=execute)
    return report, calls


def test_verification_only_accepts_fresh_checks_without_worker_edits(project):
    before = working_state(project[0])
    result, calls = verify(project)
    assert result["stop_reason"] == "change_accepted" and result["verification_only"]
    assert result["worker_rounds"] == 0 and not result["ready_for_collection"]
    assert calls.count("check") == calls.count("migration") == calls.count("review") == 1
    assert "worker" not in calls and "acceptance" not in calls
    assert working_state(project[0])["head"] == before["head"]
    assert (project[0] / "src/f.cpp").read_text() == "unfinished\n"
    assert TaskStore(project[0]).read(project[1].task_id) == project[1]


def test_identical_review_is_reasked_but_its_decision_is_idempotent(project):
    first, _ = verify(project)
    second, calls = verify(project)
    assert first["review"] == second["review"]
    assert calls.count("review") == calls.count("migration") == 1
    assert len(ReviewStore(project[0], project[1].task_id).decisions()) == 1


def test_new_review_can_reject_previously_approved_identical_source(project):
    verify(project)
    result, calls = verify(project, decision="request_changes")
    assert result["stop_reason"] == "review_changes_requested"
    assert result["review"]["decision"] == "request_changes"
    assert not result.get("ready_for_checkpoint") and "migration" not in calls
    assert len(ReviewStore(project[0], project[1].task_id).decisions()) == 2


@pytest.mark.parametrize("outcome", ["fail", "unknown"])
def test_failed_preservation_stops_with_feedback_and_no_repair(project, outcome):
    before = (project[0] / "src/f.cpp").read_bytes()
    result, calls = verify(project, damage=lambda report: report.update(outside_scope=outcome))
    assert result["stop_reason"] == "change_" + outcome
    assert result["checkpoint_feedback"]["result"] == outcome
    assert result["worker_rounds"] == 0 and not result.get("ready_for_checkpoint")
    assert "worker" not in calls and (project[0] / "src/f.cpp").read_bytes() == before


def test_unrestored_matching_is_not_implicitly_permitted(project):
    result, calls = verify(project, restored=False)
    assert result["stop_reason"] == "change_fail" and result["worker_rounds"] == 0
    assert "worker" not in calls


def test_publication_still_requires_fresh_acceptance_and_explicit_permission(project):
    permission = {"author": "coordinator", "message": "Verified only", "allow_migration": False}
    head = working_state(project[0])["head"]
    failed, _ = verify(project, decision="request_changes", checkpoint=permission)
    assert "checkpoint" not in failed and working_state(project[0])["head"] == head
    passed, calls = verify(project, checkpoint=permission)
    assert passed["checkpoint"]["published"] and passed["stop_reason"] == "change_accepted"
    assert calls.count("review") == calls.count("migration") == 1 and "worker" not in calls


@pytest.mark.parametrize("options", [{"verify_only": 1}, {"verify_only": True, "coordinator_configuration": {}}])
def test_invalid_mode_or_scope_delegation_refuses_before_any_command(project, options):
    before = working_state(project[0])
    with pytest.raises(ValueError, match="boolean|cannot delegate"):
        drive(project[0], project[1].task_id, CONFIGURED, WORKER, iterations=1,
              execute=lambda *_: pytest.fail("invalid verification must not launch a command"), **options)
    assert working_state(project[0]) == before


def test_pending_discovery_stops_before_checks_without_scope_mutation(project):
    root, task = project
    store = TaskStore(root)
    store.discover(task.task_id, request_id="pending", author="worker", expected_revision=0,
                   hypothesis="Class ownership needs review", affects=[task.subjects[0].entity_id], evidence=["source note"])
    before = working_state(root)
    result, calls = verify(project)
    assert result["worker_rounds"] == 0 and not calls
    assert result["stop_reason"] == store.status(task.task_id, store.scope(task.task_id)[0])["next_action"]
    assert working_state(root) == before
