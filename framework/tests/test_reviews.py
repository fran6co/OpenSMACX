import subprocess
import sys

import pytest

from match_recovery.driver import drive, working_state
from match_recovery.cli import main
from match_recovery.reviews import ReviewStore, acceptance_result, validate_review
from test_driver import CONFIG, WORKER, project


REVIEWER = {"author": "independent-reviewer", "command": {"argv": ["review"], "timeout": 10}}
GATED = {**CONFIG, "acceptance": {"argv": ["acceptance"], "timeout": 10}}
PASS = {"scope": "full_project", "result": "pass", "build": "pass", "structure": "pass",
        "references": "pass", "obligations": "pass", "runtime": "pass"}


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
            return gate_code, gate
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
        acceptance_result(0, {**PASS, dimension: "unknown"})
    assert acceptance_result(3, {**PASS, dimension: "unknown", "result": "unknown"}) == "unknown"
    assert acceptance_result(1, {**PASS, dimension: "fail", "result": "fail"}) == "fail"


def test_review_records_cannot_smuggle_in_current_measurements():
    value = {"kind": "review", "author": "worker", "reviewer": "reviewer", "snapshot": "a" * 64,
             "decision": "approve", "evidence": ["source:f.cpp"], "feedback": "Sound ownership."}
    validate_review(value)
    with pytest.raises(ValueError, match="fields"):
        validate_review({**value, "measurements": {"bytes": "pass"}})


@pytest.mark.parametrize("ready, expected", [(True, 0), (False, 3)])
def test_cli_only_reports_success_after_full_acceptance(project, monkeypatch, ready, expected):
    import json
    root, task = project
    (root / "config.json").write_text(json.dumps(CONFIG))
    (root / "worker.json").write_text(json.dumps(WORKER))
    monkeypatch.setattr("match_recovery.driver.drive", lambda *a, **kw: {"ready_for_collection": ready})
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
              "acceptance": command("from pathlib import Path; import json,sys; good=Path('src/f.cpp').read_text()=='finished'; r='pass' if good else 'fail'; print(json.dumps(dict(scope='full_project',result=r,build=r,structure=r,references=r,obligations=r,runtime=r))); sys.exit(0 if good else 1)")}
    reviewer = {"author": "reviewer", "command": command("from pathlib import Path; import json,sys; p=json.load(sys.stdin); good=Path('src/f.cpp').read_text()=='finished'; print(json.dumps(dict(snapshot=p['snapshot'],decision='approve' if good else 'request_changes',evidence=['source:src/f.cpp'],feedback='Finish the fixture.')))")}
    worker = {"author": "worker", "command": command("from pathlib import Path; import json,sys; p=json.load(sys.stdin); assert p['review_feedback']['decision']=='request_changes'; Path('src/f.cpp').write_text('finished'); print('{\"discoveries\":[]}')")}
    result = drive(root, task.task_id, config, worker, iterations=2, reviewer_configuration=reviewer)
    assert result["ready_for_collection"] and result["worker_rounds"] == 1
    assert len(ReviewStore(root, task.task_id).decisions()) == 2
