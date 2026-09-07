"""Passing scoped evidence can request review without authorizing collection."""

import pytest

from match_recovery.driver import drive
from test_driver import CONFIG, WORKER, project
from test_reviews import REVIEWER


@pytest.mark.parametrize("reviewer", [None, REVIEWER])
def test_structural_review_request_does_not_burn_worker_rounds_or_claim_acceptance(project, reviewer):
    root, task = project
    calls = []
    def execute(command, root, task_name, payload):
        name = command.argv[0]
        calls.append(name)
        if name == "context":
            return 0, {}
        if name == "check":
            return 3, {"result": "unknown", "next_action": "review_structure"}
        assert name == "review"  # No worker or full-acceptance invocation.
        assert payload["check"]["result"] == "unknown"
        return 0, {"snapshot": payload["snapshot"], "decision": "approve", "evidence": ["synthetic structural review"],
                   "feedback": "Declared requirements agree; broader structural proof remains."}
    config = {**CONFIG, "acceptance": {"argv": ["acceptance"], "timeout": 10}}
    report = drive(root, task.task_id, config, WORKER, iterations=5, reviewer_configuration=reviewer, execute=execute)
    assert report["worker_rounds"] == 0 and not report["ready_for_collection"]
    assert report["stop_reason"] == ("structural_checkpoint_reviewed" if reviewer else "independent_review_required")
    assert calls == ["context", "check"] + (["review"] if reviewer else [])


def test_requested_review_corrections_drive_the_next_worker_round(project):
    root, task = project
    calls = []
    def execute(command, root, task_name, payload):
        name = command.argv[0]
        calls.append(name)
        if name == "context":
            return 0, {}
        if name == "check":
            return 3, {"result": "unknown", "next_action": "review_structure"}
        if name == "review":
            corrected = (root / "src/f.cpp").read_text() == "real member\n"
            return 0, {"snapshot": payload["snapshot"], "decision": "approve" if corrected else "request_changes",
                       "evidence": ["synthetic constructor evidence"], "feedback": "Use the real member."}
        assert name == "worker"
        assert payload["review_feedback"]["decision"] == "request_changes"
        (root / "src/f.cpp").write_text("real member\n")
        return 0, {"discoveries": []}
    report = drive(root, task.task_id, CONFIG, WORKER, iterations=5, reviewer_configuration=REVIEWER, execute=execute)
    assert report["worker_rounds"] == 1 and report["stop_reason"] == "structural_checkpoint_reviewed"
    assert calls == ["context", "check", "review", "worker", "context", "check", "review"]
    assert not report["ready_for_collection"]


@pytest.mark.parametrize("result, build", [("fail", None), ("unknown", "fail"), ("unknown", "unknown")])
def test_review_hint_cannot_skip_recovery_or_build_failures(project, result, build):
    root, task = project
    calls = []
    def execute(command, root, task_name, payload):
        name = command.argv[0]
        calls.append(name)
        if name == "context":
            return 0, {}
        if name == "check":
            return {"fail": 1, "unknown": 3}[result], {"result": result, "next_action": "review_structure"}
        if name == "build":
            return {"fail": 1, "unknown": 3}[build], {"result": build}
        assert name == "worker"  # A hint cannot excuse a failed/unknown build.
        return 3, {"error": "synthetic worker stop"}
    config = {**CONFIG, **({"build": {"argv": ["build"], "timeout": 10}} if build else {})}
    report = drive(root, task.task_id, config, WORKER, iterations=5, reviewer_configuration=REVIEWER, execute=execute)
    assert calls == ["context", "check"] + (["build"] if build else []) + ["worker"]
    assert report["stop_reason"] == "worker_interrupted" and not report["ready_for_collection"]
