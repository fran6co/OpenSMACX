import subprocess
import sys

import pytest

from match_recovery.driver import drive, working_state
from match_recovery.cli import main
from match_recovery.reviews import ReviewStore, acceptance_outcome, merge_interpretations, pending_interpretations, validate_review, validate_supersessions
from match_recovery.model import digest
from test_driver import CONFIG, WORKER, project


REVIEWER = {"author": "independent-reviewer", "command": {"argv": ["review"], "timeout": 10}}
GATED = {**CONFIG, "acceptance": {"argv": ["acceptance"], "timeout": 10}}
PASS = {"scope": "full_project", "result": "pass", "build": "pass", "structure": "pass",
        "references": "pass", "obligations": "pass", "runtime": "pass",
        "request_fingerprint": "a" * 64, "measurements": [], "current": {}}


def receipt(request, report=PASS):
    """Synthetic protocol fixture, never a real verifier or measurement cache."""
    current = {row["obligation_id"]: digest([request, row["obligation_id"]]) for row in request["protected"]}
    return {**report, "request_fingerprint": digest(request), "current": current,
            "measurements": [{"obligation_id": key, "fingerprint": value, "result": "pass",
                              "evidence": "synthetic protocol control"} for key, value in current.items()]}


def execution(project, *, reject_first=False, malformed=False, mutate=False, gate=PASS, gate_code=0):
    root, task = project
    calls = []
    def execute(command, root, task_name, payload):
        name = command.argv[0]
        calls.append(name)
        if name == "context":
            return 0, {"source": "src/f.cpp"}
        if name == "check":
            return 0, {"result": "pass"}
        if name == "acceptance":
            return gate_code, receipt(payload, gate)
        if name == "review":
            if mutate:
                (root / "src/f.cpp").write_text("reviewer edited the code")
            return 0, {"snapshot": "b" * 64 if malformed else payload["snapshot"],
                       "decision": "request_changes" if reject_first and calls.count("review") == 1 else "approve",
                       "evidence": ["source:src/f.cpp"], "feedback": "Use the real owning member."}
        assert name == "worker"
        assert payload["review_feedback"]["decision"] == "request_changes"
        (root / "src/f.cpp").write_text("structurally improved\n")
        return 0, {"discoveries": []}
    return execute, calls


def test_independent_review_and_full_gate_are_both_required(project):
    root, task = project
    execute, calls = execution(project)
    result = drive(root, task.task_id, GATED, WORKER, iterations=2,
                   reviewer_configuration=REVIEWER, execute=execute)
    assert result["ready_for_collection"]
    assert calls == ["context", "check", "review", "acceptance"]
    recorded, = ReviewStore(root, task.task_id).decisions()
    assert recorded["decision"] == "approve"
    assert "result" not in recorded and "acceptance" not in recorded


def test_passing_scoped_check_and_review_cannot_replace_missing_acceptance(project):
    root, task = project
    execute, _ = execution(project)
    result = drive(root, task.task_id, CONFIG, WORKER, iterations=1,
                   reviewer_configuration=REVIEWER, execute=execute)
    assert result["stop_reason"] == "full_acceptance_gate_required"
    assert not result["ready_for_collection"]


def test_review_feedback_drives_another_worker_round_then_requires_fresh_review(project):
    root, task = project
    execute, calls = execution(project, reject_first=True)
    result = drive(root, task.task_id, GATED, WORKER, iterations=2,
                   reviewer_configuration=REVIEWER, execute=execute)
    assert result["ready_for_collection"] and result["worker_rounds"] == 1
    assert calls == ["context", "check", "review", "worker", "context", "check", "review", "acceptance"]
    first, second = ReviewStore(root, task.task_id).decisions()
    assert first["snapshot"] != second["snapshot"]


@pytest.mark.parametrize("options, message", [({"malformed": True}, "stale"), ({"mutate": True}, "read-only")])
def test_review_must_match_the_snapshot_and_be_read_only(project, options, message):
    root, task = project
    execute, _ = execution(project, **options)
    with pytest.raises(ValueError, match=message):
        drive(root, task.task_id, GATED, WORKER, iterations=1,
              reviewer_configuration=REVIEWER, execute=execute)
    assert not ReviewStore(root, task.task_id).decisions()


def test_worker_cannot_review_its_own_source(project):
    root, task = project
    with pytest.raises(ValueError, match="independent"):
        drive(root, task.task_id, GATED, WORKER, iterations=1, reviewer_configuration=WORKER)


def interpretation(entity, offset, current):
    value = {"entity_id": entity, "value": {"member": "slot_", "offset": offset}}
    return {"interpretation_id": digest(value), **value, "current": current}


def supersession(previous, replacement):
    return {"previous": previous["interpretation_id"], "replacement": replacement["interpretation_id"],
            "evidence": ["source:src/f.cpp"], "reason": "Constructor evidence identifies the actual member offset."}


@pytest.mark.parametrize("retract", [False, True])
def test_reviewed_replacement_reaches_full_gate_without_dropping_obligations(project, retract):
    from dataclasses import asdict
    from match_recovery.model import Dimension, Obligation

    root, task = project
    entity = task.subjects[0].entity_id
    previous, replacement = interpretation(entity, 4, False), interpretation(entity, 8, True)
    obligation = Obligation(entity, Dimension.STRUCTURE, "source interpretation history")
    config = {**GATED, "obligations": {"argv": ["obligations"], "timeout": 10}}
    observed = []
    disposition = ({"retractions": [{"interpretation": previous["interpretation_id"],
                                     "evidence": ["source:src/f.cpp"], "reason": "The earlier layout assumption was unsupported."}]}
                   if retract else {"supersessions": [supersession(previous, replacement)]})

    def execute(command, root, task_name, payload):
        if command.argv[0] == "obligations":
            return 0, {"obligations": [{**asdict(obligation), "obligation_id": obligation.obligation_id}],
                       "interpretations": [previous, replacement]}
        if command.argv[0] == "context":
            return 0, {}
        if command.argv[0] == "check":
            return 0, {"result": "pass"}
        if command.argv[0] == "review":
            assert len(payload["context"]["review_interpretations"]) == 2
            return 0, {"snapshot": payload["snapshot"], "decision": "approve", "evidence": ["source:src/f.cpp"],
                       "feedback": "The new offset is justified.", **disposition}
        assert command.argv[0] == "acceptance"
        observed.append(payload)
        # Reviewed replacement is not current build/runtime/layout proof.
        return 3, receipt(payload, {**PASS, "result": "unknown", "structure": "unknown"})

    report = drive(root, task.task_id, config, WORKER, iterations=1,
                   reviewer_configuration=REVIEWER, execute=execute)
    assert report["stop_reason"] == "acceptance_unknown" and not report["ready_for_collection"]
    assert observed[0]["protected"][0]["obligation_id"] == obligation.obligation_id
    field = "retractions" if retract else "supersessions"
    assert observed[0]["review"][field] == disposition[field]
    assert ReviewStore(root, task.task_id).decisions()[0][field]


def test_generic_approval_cannot_skip_removed_interpretation_and_review_can_resume(project):
    root, task = project
    previous = interpretation(task.subjects[0].entity_id, 4, False)
    config = {**GATED, "obligations": {"argv": ["obligations"], "timeout": 10}}
    calls = []
    retract = False

    def execute(command, root, task_name, payload):
        name = command.argv[0]
        calls.append(name)
        if name == "obligations":
            return 0, {"obligations": [], "interpretations": [previous]}
        if name == "context":
            return 0, {}
        if name == "check":
            return 0, {"result": "pass"}
        if name == "review":
            response = {"snapshot": payload["snapshot"], "decision": "approve", "evidence": ["source:src/f.cpp"],
                        "feedback": "Reviewed current code."}
            if retract:
                response["retractions"] = [{"interpretation": previous["interpretation_id"],
                                             "reason": "Original evidence disproves this field assumption.",
                                             "evidence": ["source:src/f.cpp"]}]
            return 0, response
        assert name == "acceptance"
        return 3, receipt(payload, {**PASS, "result": "unknown", "structure": "unknown"})

    first = drive(root, task.task_id, config, WORKER, iterations=1,
                  reviewer_configuration=REVIEWER, execute=execute)
    assert first["stop_reason"] == "interpretation_review_incomplete"
    assert first["pending_interpretations"] == [previous["interpretation_id"]]
    assert "acceptance" not in calls and not first["ready_for_collection"]
    subprocess.run(["git", "add", "recovery"], cwd=root, check=True)
    subprocess.run(["git", "commit", "-m", "checkpoint incomplete review"], cwd=root, check=True, capture_output=True)
    retract = True
    second = drive(root, task.task_id, config, WORKER, iterations=1,
                   reviewer_configuration=REVIEWER, execute=execute)
    assert calls.count("review") == 2  # Incomplete approval must not be reused forever.
    assert second["stop_reason"] == "acceptance_unknown" and not second["ready_for_collection"]


def test_retraction_cannot_hide_current_interpretation_or_duplicate_disposition():
    previous = interpretation("entity-one", 4, False)
    decision = {"kind": "review", "author": "worker", "reviewer": "reviewer", "snapshot": "a" * 64,
                "decision": "approve", "evidence": ["source:f.cpp"], "feedback": "Withdraw old interpretation.",
                "retractions": [{"interpretation": previous["interpretation_id"], "reason": "Disproved assumption.",
                                  "evidence": ["source:f.cpp"]}]}
    known = {previous["interpretation_id"]: previous}
    assert pending_interpretations(decision, known) == []
    with pytest.raises(ValueError, match="duplicate"):
        validate_review({**decision, "retractions": decision["retractions"] * 2})
    previous["current"] = True
    with pytest.raises(ValueError, match="historical"):
        validate_supersessions(decision, known)


def test_new_historical_interpretation_after_review_stops_before_acceptance(project):
    root, task = project
    base_execute, calls = execution(project)
    reads = 0
    old = interpretation(task.subjects[0].entity_id, 4, False)
    config = {**GATED, "obligations": {"argv": ["obligations"], "timeout": 10}}

    def execute(command, root, task_name, payload):
        nonlocal reads
        if command.argv[0] == "obligations":
            reads += 1
            return 0, {"obligations": [], "interpretations": [] if reads == 1 else [old]}
        return base_execute(command, root, task_name, payload)

    with pytest.raises(ValueError, match="inventory changed after review"):
        drive(root, task.task_id, config, WORKER, iterations=1,
              reviewer_configuration=REVIEWER, execute=execute)
    assert "review" in calls and "acceptance" not in calls


@pytest.mark.parametrize("bad", ["unknown", "still_current", "not_current", "other_anchor"])
def test_supersession_requires_historical_to_current_same_anchor(bad):
    previous, replacement = interpretation("entity-one", 4, False), interpretation("entity-one", 8, True)
    if bad == "still_current":
        previous["current"] = True
    if bad == "not_current":
        replacement["current"] = False
    if bad == "other_anchor":
        replacement = interpretation("entity-two", 8, True)
    decision = {"kind": "review", "author": "worker", "reviewer": "reviewer", "snapshot": "a" * 64,
                "decision": "approve", "evidence": ["source:f.cpp"], "feedback": "Replace interpretation.",
                "supersessions": [supersession(previous, replacement)]}
    known = {item["interpretation_id"]: item for item in (previous, replacement)}
    if bad == "unknown":
        known.pop(previous["interpretation_id"])
    with pytest.raises(ValueError, match="supersession"):
        validate_supersessions(decision, known)


def test_interpretation_inventory_retains_seen_values_but_refreshes_current_status():
    first, second = interpretation("entity-one", 4, True), interpretation("entity-one", 8, True)
    known = {}
    merge_interpretations(known, [first], frozenset({"entity-one"}))
    merge_interpretations(known, [second], frozenset({"entity-one"}))
    assert not known[first["interpretation_id"]]["current"]
    assert known[second["interpretation_id"]]["current"]
    with pytest.raises(ValueError, match="identity"):
        merge_interpretations(known, [{**second, "interpretation_id": "b" * 64}], frozenset({"entity-one"}))


def test_commit_of_review_does_not_stale_source_but_changed_header_does(project):
    root, task = project
    reviews = ReviewStore(root, task.task_id)
    before = reviews.snapshot(working_state(root), GATED)
    reviews.append({"kind": "review", "author": "worker", "reviewer": "reviewer", "snapshot": before,
                    "decision": "approve", "evidence": ["source:src/f.cpp"], "feedback": "Sound structure."},
                   expected_revision=0)
    assert reviews.snapshot(working_state(root), GATED) == before
    subprocess.run(["git", "add", "recovery"], cwd=root, check=True)
    subprocess.run(["git", "commit", "-m", "review decision"], cwd=root, check=True, capture_output=True)
    assert reviews.snapshot(working_state(root), GATED) == before
    (root / "src/new.h").write_text("class NewHeader {};\n")
    assert reviews.snapshot(working_state(root), GATED) != before


def test_current_approval_can_be_reused_but_acceptance_always_reruns(project):
    root, task = project
    execute, calls = execution(project)
    drive(root, task.task_id, GATED, WORKER, iterations=1, reviewer_configuration=REVIEWER, execute=execute)
    subprocess.run(["git", "add", "recovery"], cwd=root, check=True)
    subprocess.run(["git", "commit", "-m", "review decision"], cwd=root, check=True, capture_output=True)
    calls.clear()
    result = drive(root, task.task_id, GATED, WORKER, iterations=1, reviewer_configuration=REVIEWER, execute=execute)
    assert result["ready_for_collection"] and calls == ["context", "check", "acceptance"]


@pytest.mark.parametrize("dimension", ["build", "structure", "references", "obligations", "runtime"])
def test_no_missing_dimension_can_be_green(dimension):
    with pytest.raises(ValueError, match="disagree"):
        acceptance_outcome(0, {**PASS, dimension: "unknown"})
    assert acceptance_outcome(3, {**PASS, dimension: "unknown", "result": "unknown"}) == "unknown"
    assert acceptance_outcome(1, {**PASS, dimension: "fail", "result": "fail"}) == "fail"


def test_review_records_cannot_smuggle_in_current_measurements():
    value = {"kind": "review", "author": "worker", "reviewer": "reviewer", "snapshot": "a" * 64,
             "decision": "approve", "evidence": ["source:f.cpp"], "feedback": "Sound ownership."}
    validate_review(value)
    with pytest.raises(ValueError, match="fields"):
        validate_review({**value, "measurements": {"bytes": "pass"}})


@pytest.mark.parametrize("ready, reason, expected", [(True, "ready_for_collection", 0),
                                                    (False, "full_acceptance_gate_required", 3),
                                                    (False, "migration_admitted", 2)])
def test_cli_only_reports_success_after_full_acceptance(project, monkeypatch, ready, reason, expected):
    import json
    root, task = project
    (root / "config.json").write_text(json.dumps(CONFIG))
    (root / "worker.json").write_text(json.dumps(WORKER))
    monkeypatch.setattr("match_recovery.driver.drive", lambda *a, **kw: {"ready_for_collection": ready, "stop_reason": reason})
    assert main(["run", task.task_id, "--root", str(root), "--config", str(root / "config.json"),
                 "--worker", str(root / "worker.json")]) == expected


def test_scoped_check_cannot_impersonate_full_acceptance(project):
    root, task = project
    execute, _ = execution(project, gate={"result": "pass"})
    with pytest.raises(ValueError, match="full-project"):
        drive(root, task.task_id, GATED, WORKER, iterations=1, reviewer_configuration=REVIEWER, execute=execute)


def test_approved_structural_checkpoint_keeps_regressions_and_is_not_admitted(project):
    root, task = project
    def execute(command, root, task_name, payload):
        name = command.argv[0]
        if name == "context":
            return 0, {}
        if name == "check":
            return 1, {"result": "fail"}
        if name == "worker":
            (root / "src/f.cpp").write_text("correct structure; matching unfinished\n")
            return 0, {"discoveries": []}
        if name == "review":
            return 0, {"snapshot": payload["snapshot"], "decision": "approve",
                       "evidence": ["constructor evidence"], "feedback": "Keep this ownership; restore remaining matches."}
        pytest.fail("a regressed checkpoint must not reach final acceptance")
    result = drive(root, task.task_id, GATED, WORKER, iterations=1,
                   reviewer_configuration=REVIEWER, execute=execute)
    assert result["stop_reason"] == "structural_checkpoint_reviewed"
    assert not result["ready_for_collection"] and result["check"]["result"] == "fail"
    assert "correct structure" in (root / "src/f.cpp").read_text()


def test_real_subprocess_review_feedback_and_acceptance_loop(project):
    root, task = project
    def command(code):
        return {"argv": [sys.executable, "-c", code], "timeout": 10}
    config = {"schema_version": 1, "target": "synthetic",
              "context": command("print('{}')"),
              "check": command("print('{\"result\":\"pass\"}')"),
              "acceptance": command("from pathlib import Path; import json,sys; from match_recovery.model import digest; p=json.load(sys.stdin); good=Path('src/f.cpp').read_text()=='finished'; r='pass' if good else 'fail'; print(json.dumps(dict(scope='full_project',result=r,build=r,structure=r,references=r,obligations=r,runtime=r,request_fingerprint=digest(p),measurements=[],current={}))); sys.exit(0 if good else 1)")}
    reviewer = {"author": "reviewer", "command": command("from pathlib import Path; import json,sys; p=json.load(sys.stdin); good=Path('src/f.cpp').read_text()=='finished'; print(json.dumps(dict(snapshot=p['snapshot'],decision='approve' if good else 'request_changes',evidence=['source:src/f.cpp'],feedback='Finish the fixture.')))")}
    worker = {"author": "worker", "command": command("from pathlib import Path; import json,sys; p=json.load(sys.stdin); assert p['review_feedback']['decision']=='request_changes'; Path('src/f.cpp').write_text('finished'); print('{\"discoveries\":[]}')")}
    result = drive(root, task.task_id, config, worker, iterations=2, reviewer_configuration=reviewer)
    assert result["ready_for_collection"] and result["worker_rounds"] == 1
    assert len(ReviewStore(root, task.task_id).decisions()) == 2
