"""Control-flow proofs, not evidence of an autonomous application recovery."""

from dataclasses import asdict

import pytest

from match_recovery.driver import drive
from match_recovery.model import Dimension, Obligation
from match_recovery.reviews import ReviewStore, acceptance_outcome
from test_driver import CONFIG, WORKER, project
from test_reviews import GATED, PASS, REVIEWER, interpretation, supersession, receipt


def approve(payload):
    return 0, {"snapshot": payload["snapshot"], "decision": "approve",
               "evidence": ["source:src/f.cpp"], "feedback": "Review is not measured acceptance."}


@pytest.mark.parametrize("outcome", ["pass", "fail", "unknown"])
def test_structural_review_reaches_fresh_full_verification_not_an_automatic_pass(project, outcome):
    root, task = project
    calls = []
    report = {**PASS, "structure": outcome, "result": outcome}

    def execute(command, root, task_name, payload):
        stage = command.argv[0]
        calls.append(stage)
        if stage == "context":
            return 0, {}
        if stage == "check":
            return 3, {"result": "unknown", "next_action": "review_structure"}
        if stage == "review":
            return approve(payload)
        if stage == "acceptance":
            assert payload["review"]["snapshot"] == payload["snapshot"]
            assert payload["review"]["decision"] == "approve"
            assert set(payload) == {"task", "snapshot", "protected", "review", "interpretations", "verification_id"}
            report.update(receipt(payload, report))
            return {"pass": 0, "fail": 1, "unknown": 3}[outcome], report
        assert stage == "worker" and outcome == "fail"
        assert payload["acceptance_feedback"]["report"] == report
        return 3, {"error": "controlled pause after a measured counterexample"}

    result = drive(root, task.task_id, GATED, WORKER, iterations=2,
                   reviewer_configuration=REVIEWER, execute=execute)
    assert calls == ["context", "check", "review", "acceptance"] + (["worker"] if outcome == "fail" else [])
    assert result["ready_for_collection"] == (outcome == "pass")
    assert result["check"]["result"] == "unknown"  # No rewritten scoped verdict.


def test_acceptance_counterexample_repairs_then_rechecks_and_rereviews(project):
    root, task = project
    calls, snapshots = [], []
    failure = {**PASS, "runtime": "fail", "result": "fail", "findings": [
        {"dimension": "runtime", "message": "Owned member is destroyed twice.",
         "evidence": ["controlled lifetime observation", "source:src/f.cpp"]}]}

    def execute(command, root, task_name, payload):
        stage = command.argv[0]
        calls.append(stage)
        if stage == "context":
            return 0, {}
        if stage == "check":
            return 0, {"result": "pass"}
        if stage == "build":
            return 0, {"result": "pass"}
        if stage == "review":
            snapshots.append(payload["snapshot"])
            return approve(payload)
        if stage == "acceptance":
            assert payload["snapshot"] == snapshots[-1]
            failure.update(receipt(payload, failure))
            return (0, receipt(payload)) if (root / "src/f.cpp").read_text() == "owned member\n" else (1, failure)
        assert stage == "worker"
        assert payload["check"]["result"] == "pass"
        assert payload["review_feedback"]["decision"] == "approve"
        assert payload["acceptance_feedback"] == {"snapshot": snapshots[0], "report": failure}
        (root / "src/f.cpp").write_text("owned member\n")
        return 0, {"discoveries": []}

    config = {**GATED, "build": {"argv": ["build"], "timeout": 10}}
    result = drive(root, task.task_id, config, WORKER, iterations=2,
                   reviewer_configuration=REVIEWER, execute=execute)
    assert result["ready_for_collection"] and result["worker_rounds"] == 1
    assert calls == ["context", "check", "build", "review", "acceptance", "worker",
                     "context", "check", "build", "review", "acceptance"]
    assert snapshots[0] != snapshots[1]
    assert len(ReviewStore(root, task.task_id).decisions()) == 2
    assert "acceptance_feedback" not in result
    assert all("runtime" not in row for row in ReviewStore(root, task.task_id).decisions())


@pytest.mark.parametrize("damage", ["none", "budget", "discovery", "outside_scope"])
def test_failed_acceptance_retains_bounds_scope_and_discoveries(project, damage):
    root, task = project
    calls = []

    def execute(command, root, task_name, payload):
        stage = command.argv[0]
        calls.append(stage)
        if stage == "context":
            return 0, {}
        if stage == "check":
            return 0, {"result": "pass"}
        if stage == "review":
            return approve(payload)
        if stage == "acceptance":
            return 1, receipt(payload, {**PASS, "references": "fail", "result": "fail"})
        assert stage == "worker"
        if damage in ("budget", "outside_scope"):
            name = "src/f.cpp" if damage == "budget" else "src/outside.cpp"
            (root / name).write_text("structural progress\n")
        return 0, {"discoveries": ([{"hypothesis": "Callee ownership needs expansion.",
                                      "affects": [task.subjects[0].entity_id], "evidence": ["source:src/f.cpp"]}]
                                    if damage == "discovery" else [])}

    if damage == "outside_scope":
        with pytest.raises(ValueError, match="exceeded editing scope"):
            drive(root, task.task_id, GATED, WORKER, iterations=1,
                  reviewer_configuration=REVIEWER, execute=execute)
        assert (root / "src/outside.cpp").exists()  # No destructive cleanup.
        return
    result = drive(root, task.task_id, GATED, WORKER, iterations=1,
                   reviewer_configuration=REVIEWER, execute=execute)
    assert not result["ready_for_collection"] and calls.count("worker") == 1
    assert result["acceptance_feedback"]["report"]["references"] == "fail"
    if damage == "none":
        assert result["stop_reason"] == "no_source_progress"
    elif damage == "budget":
        assert result["stop_reason"] == "acceptance_fail"
        assert calls.count("acceptance") == 2
        assert (root / "src/f.cpp").read_text() == "structural progress\n"
    else:
        assert result["stop_reason"] == "evaluate_structural_discoveries"
        assert calls.count("acceptance") == 1


def test_historical_interpretations_and_obligations_reach_post_review_gate(project):
    root, task = project
    entity = task.subjects[0].entity_id
    old, new = interpretation(entity, 4, False), interpretation(entity, 8, True)
    obligation = Obligation(entity, Dimension.STRUCTURE, "original class ownership")
    protected = [{**asdict(obligation), "obligation_id": obligation.obligation_id}]
    config = {**GATED, "obligations": {"argv": ["obligations"], "timeout": 10}}

    def execute(command, root, task_name, payload):
        stage = command.argv[0]
        if stage == "context":
            return 0, {}
        if stage == "obligations":
            return 0, {"obligations": protected, "interpretations": [old, new]}
        if stage == "check":
            return 3, {"result": "unknown", "next_action": "review_structure"}
        if stage == "review":
            code, response = approve(payload)
            return code, {**response, "supersessions": [supersession(old, new)]}
        assert stage == "acceptance"
        assert payload["protected"] == protected
        assert len(payload["interpretations"]) == 2
        assert payload["review"]["supersessions"] == [supersession(old, new)]
        return 3, receipt(payload, {**PASS, "structure": "unknown", "result": "unknown"})

    result = drive(root, task.task_id, config, WORKER, iterations=1,
                   reviewer_configuration=REVIEWER, execute=execute)
    assert result["stop_reason"] == "acceptance_unknown" and not result["ready_for_collection"]


@pytest.mark.parametrize("finding", [None, {}, [{"dimension": "made_up", "message": "bad", "evidence": ["x"]}],
                                    [{"dimension": "runtime", "message": "", "evidence": ["x"]}],
                                    [{"dimension": "runtime", "message": "bad", "evidence": []}],
                                    [{"dimension": "build", "message": "bad", "evidence": ["x"]}]])
def test_acceptance_findings_are_actionable_and_cannot_contradict_passes(finding):
    with pytest.raises(ValueError):
        acceptance_outcome(1, {**PASS, "result": "fail", "runtime": "fail", "findings": finding})


@pytest.mark.parametrize("damage", ["mutation", "false_exit"])
def test_post_structural_review_gate_keeps_readonly_and_verdict_guards(project, damage):
    def execute(command, root, task_name, payload):
        stage = command.argv[0]
        if stage == "context":
            return 0, {}
        if stage == "check":
            return 3, {"result": "unknown", "next_action": "review_structure"}
        if stage == "review":
            return approve(payload)
        assert stage == "acceptance"
        if damage == "mutation":
            (root / "src/f.cpp").write_text("unauthorized verifier edit\n")
        return 0, receipt(payload, {**PASS, "result": "unknown", "runtime": "unknown"})

    with pytest.raises(ValueError, match="read-only" if damage == "mutation" else "disagree"):
        drive(project[0], project[1].task_id, GATED, WORKER, iterations=1,
              reviewer_configuration=REVIEWER, execute=execute)
