"""The recovery loop must act on build failures even when scoped bytes pass."""

import pytest

from match_recovery.driver import drive
from test_driver import CONFIG, WORKER, project
from test_reviews import REVIEWER
from test_migration_driver import CONFIGURED, setup


BUILD = {"argv": ["build"], "timeout": 10}


def test_build_failure_drives_edit_and_fresh_recheck_before_review(project):
    root, task = project
    calls = []
    def execute(command, root, task_name, payload):
        stage = command.argv[0]
        calls.append(stage)
        if stage == "context":
            return 0, {}
        if stage == "check":
            return 0, {"result": "pass", "native_detail": "byte proof"}
        if stage == "build":
            restored = (root / "src/f.cpp").read_text() == "out-of-line definition\n"
            return (0 if restored else 1), {"result": "pass" if restored else "fail", "reason": "link check"}
        if stage == "worker":
            assert payload["check"]["result"] == "fail"
            assert payload["check"]["recovery_result"] == "pass"
            assert payload["check"]["native_detail"] == "byte proof"
            assert payload["check"]["build_check"]["result"] == "fail"
            (root / "src/f.cpp").write_text("out-of-line definition\n")
            return 0, {"discoveries": []}
        assert stage == "review"
        assert payload["check"]["build_check"]["result"] == "pass"
        return 0, {"snapshot": payload["snapshot"], "decision": "approve", "evidence": ["synthetic source review"],
                   "feedback": "Real cross-unit definition"}
    report = drive(root, task.task_id, {**CONFIG, "build": BUILD}, WORKER, iterations=1,
                   reviewer_configuration=REVIEWER, execute=execute)
    assert calls == ["context", "check", "build", "worker", "context", "check", "build", "review"]
    assert report["stop_reason"] == "full_acceptance_gate_required"
    assert not report["ready_for_collection"]


@pytest.mark.parametrize("build_result", ["fail", "unknown"])
def test_migration_permission_and_approval_cannot_waive_bad_build(project, build_result):
    permission, original, calls = setup(project)
    def execute(command, root, task_name, payload):
        if command.argv[0] == "build":
            return (1 if build_result == "fail" else 3), {"result": build_result}
        return original(command, root, task_name, payload)
    report = drive(project[0], project[1].task_id, {**CONFIGURED, "build": BUILD}, WORKER, iterations=1,
                   reviewer_configuration=REVIEWER, migration_configuration=permission, execute=execute)
    assert report["stop_reason"] == "build_verification_required"
    assert "migration" not in calls and "acceptance" not in calls
    assert not report["ready_for_collection"]


@pytest.mark.parametrize("damage", ["mutation", "false_exit"])
def test_build_command_uses_readonly_and_exit_consistency_guards(project, damage):
    def execute(command, root, task_name, payload):
        stage = command.argv[0]
        if stage == "context":
            return 0, {}
        if stage == "check":
            return 0, {"result": "pass"}
        assert stage == "build"
        if damage == "mutation":
            (root / "src/f.cpp").write_text("unauthorized builder edit\n")
        return 0, {"result": "unknown"}
    with pytest.raises(ValueError, match="read-only" if damage == "mutation" else "disagree"):
        drive(project[0], project[1].task_id, {**CONFIG, "build": BUILD}, WORKER, iterations=1, execute=execute)
