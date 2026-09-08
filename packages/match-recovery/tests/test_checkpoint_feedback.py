"""Per-change repair controls; these are not application runtime proofs."""

import pytest

from match_recovery.driver import drive
from match_recovery.reviews import ReviewStore
from test_migration_driver import CONFIGURED, WORKER, REVIEWER, project, setup


def control(project, outcome, *, action="repair", damage=None, diagnostics=True):
    permission, original, calls = setup(project, restored=True)
    root, task = project
    snapshots = []
    def execute(command, root, name, payload):
        stage = command.argv[0]
        if stage == "worker":
            calls.append(stage)
            feedback = payload["checkpoint_feedback"]
            assert feedback["result"] == outcome
            assert feedback["report"]["outside_scope"] == outcome
            assert feedback["snapshot"] == payload["review_feedback"]["snapshot"]
            if diagnostics:
                assert "unscoped caller" in str(feedback["report"]["diagnostics"])
            if action in ("repair", "budget", "outside_scope"):
                path = root / ("src/outside.cpp" if action == "outside_scope" else "src/f.cpp")
                path.write_text("fixed\n" if action == "repair" else "more evidence\n")
            return 0, {"discoveries": ([{"hypothesis": "Caller ownership needs expansion",
                                         "affects": [task.subjects[0].entity_id], "evidence": ["source:src/f.cpp"]}]
                                       if action == "discovery" else [])}
        code, report = original(command, root, name, payload)
        if stage == "review":
            snapshots.append(payload["snapshot"])
        if stage == "migration" and (root / "src/f.cpp").read_text() != "fixed\n":
            report["outside_scope"] = outcome
            if diagnostics:
                report["diagnostics"] = [{"check": "preservation", "result": outcome,
                                           "evidence": ["unscoped caller differs; source:src/f.cpp"]}]
            if damage:
                damage(report)
        return code, report
    def run(iterations=2):
        return drive(root, task.task_id, {**CONFIGURED, "acceptance_scope": "change"}, WORKER,
                     iterations=iterations, reviewer_configuration=REVIEWER,
                     migration_configuration=permission, execute=execute)
    return run, calls, snapshots


@pytest.mark.parametrize("outcome", ["fail", "unknown"])
def test_refusal_repairs_then_rechecks_and_rereviews(project, outcome):
    run, calls, snapshots = control(project, outcome)
    report = run()
    assert report["stop_reason"] == "change_accepted" and report["worker_rounds"] == 1
    assert report["ready_for_checkpoint"] and not report["ready_for_collection"]
    assert "checkpoint_feedback" not in report and "acceptance" not in calls
    assert calls.count("check") == calls.count("migration") == calls.count("review") == 2
    assert snapshots[0] != snapshots[1]
    assert len(ReviewStore(project[0], project[1].task_id).decisions()) == 2


@pytest.mark.parametrize("outcome", ["fail", "unknown"])
@pytest.mark.parametrize("action,reason", [("none", "no_source_progress"), ("budget", None),
                                           ("discovery", "evaluate_structural_discoveries")])
def test_refusal_feedback_keeps_progress_budget_and_discovery_stops(project, outcome, action, reason):
    run, calls, _ = control(project, outcome, action=action)
    report = run(iterations=1 if action == "budget" else 2)
    assert report["stop_reason"] == (reason or "change_" + outcome)
    assert calls.count("worker") == 1 and not report.get("ready_for_checkpoint")
    assert not report["ready_for_collection"] and report["checkpoint_feedback"]["result"] == outcome


def test_unknown_without_actionable_evidence_stops_before_worker(project):
    run, calls, _ = control(project, "unknown", diagnostics=False)
    assert run()["stop_reason"] == "change_unknown"
    assert "worker" not in calls


def test_feedback_does_not_authorize_outside_source_edits(project):
    run, _, _ = control(project, "fail", action="outside_scope")
    with pytest.raises(ValueError, match="exceeded editing scope"):
        run()
    assert (project[0] / "src/outside.cpp").exists()


@pytest.mark.parametrize("damage", [lambda r: r.update(snapshot="b" * 64),
                                    lambda r: r.pop("references"),
                                    lambda r: r.update(diagnostics={}),
                                    lambda r: r["diagnostics"][0].update(evidence=[]),
                                    lambda r: r["measurements"].append(dict(r["measurements"][0]))])
def test_bad_receipts_remain_protocol_errors_even_with_real_failures(project, damage):
    run, calls, _ = control(project, "fail", damage=damage)
    with pytest.raises(ValueError):
        run()
    assert "worker" not in calls
