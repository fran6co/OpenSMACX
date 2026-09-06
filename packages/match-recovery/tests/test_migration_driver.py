from dataclasses import asdict
import json
import subprocess
import sys

import pytest

from match_recovery.driver import drive
from match_recovery.model import Dimension, Obligation
from test_driver import CONFIG, WORKER, project
from test_reviews import REVIEWER, PASS


CONFIGURED = {**CONFIG, "obligations": {"argv": ["inventory"], "timeout": 10},
              "migration_check": {"argv": ["migration"], "timeout": 10},
              "acceptance": {"argv": ["acceptance"], "timeout": 10}}


def setup(project, *, damage=None, disappear=False, restored=False):
    root, task = project
    obligation = Obligation(task.subjects[0].entity_id, Dimension.BYTES, "git-history:" + task.base_revision)
    row = {**asdict(obligation), "obligation_id": obligation.obligation_id}
    permission = {"migration_id": "member-layout", "author": task.author,
                  "evidence": ["synthetic member layout evidence"],
                  "allowed": [{"entity_id": obligation.entity_id, "dimension": "bytes"}]}
    calls = []
    def execute(command, root, name, payload):
        stage = command.argv[0]
        calls.append(stage)
        if stage == "inventory":
            return 0, {"obligations": [] if disappear and "worker" in calls else [row]}
        if stage == "context":
            return 0, {"source": "src/f.cpp"}
        if stage == "check":
            assert payload["context"]["protected_obligations"] == [row]
            return (0 if restored else 1), {"result": "pass" if restored else "fail"}
        if stage == "worker":
            assert payload["context"]["protected_obligations"] == [row]
            (root / "src/f.cpp").write_text("correct member layout, matching work remains\n")
            return 0, {"discoveries": []}
        if stage == "review":
            assert payload["context"]["migration_permission"] == permission
            return 0, {"snapshot": payload["snapshot"], "decision": "approve",
                       "evidence": ["source:src/f.cpp"], "feedback": "The member ownership is correct."}
        if stage == "migration":
            assert payload["protected"] == [row]
            report = {"snapshot": payload["snapshot"], "changed": [obligation.entity_id],
                      "build": "pass", "structure": "pass", "references": "pass", "outside_scope": "pass",
                      "measurements": [{"obligation_id": obligation.obligation_id, "fingerprint": "a" * 64,
                                        "result": "pass" if restored else "fail", "evidence": "synthetic comparison"}],
                      "current": {obligation.obligation_id: "a" * 64}}
            if damage:
                damage(report)
            return 0, report
        assert stage == "acceptance"
        assert payload["protected"] == [row]
        return 0, PASS
    return permission, execute, calls


def run(project, **options):
    permission, execute, calls = setup(project, **options)
    root, task = project
    result = drive(root, task.task_id, CONFIGURED, WORKER, iterations=1,
                   reviewer_configuration=REVIEWER, migration_configuration=permission, execute=execute)
    return result, calls


def test_admitted_structural_regression_is_not_collection_ready(project):
    result, calls = run(project)
    assert result["stop_reason"] == "migration_admitted"
    assert result["migration"]["exit_code"] == 2
    assert len(result["migration"]["tracked"]) == 1
    assert not result["ready_for_collection"]
    assert "acceptance" not in calls
    assert "correct member layout" in (project[0] / "src/f.cpp").read_text()


def test_worker_cannot_drop_protected_requirement_by_erasing_its_annotation(project):
    result, _ = run(project, disappear=True)
    assert result["stop_reason"] == "migration_admitted"
    assert len(result["migration"]["tracked"]) == 1


def test_already_correct_checkpoint_can_be_reviewed_without_forcing_an_edit(project):
    root, _ = project
    (root / "src/f.cpp").write_text("correct member layout, matching work remains\n")
    result, calls = run(project)
    assert result["stop_reason"] == "migration_admitted"
    assert calls.count("worker") == 1 and calls.count("review") == 1


def test_resume_must_restore_before_full_acceptance(project):
    first, _ = run(project)
    assert first["migration"]["tracked"]
    root, _ = project
    subprocess.run(["git", "add", "."], cwd=root, check=True)
    subprocess.run(["git", "commit", "-m", "reviewed structural checkpoint"], cwd=root, check=True, capture_output=True)
    (root / "src/f.cpp").write_text("structure and matching restored\n")
    second, calls = run(project, restored=True)
    assert second["migration"]["exit_code"] == 0
    assert not second["migration"]["tracked"]
    assert second["ready_for_collection"] and "acceptance" in calls


@pytest.mark.parametrize("damage", [
    lambda r: r.update(measurements=[]),
    lambda r: r.update(current={}),
    lambda r: r["measurements"][0].update(result="unknown"),
    lambda r: r["measurements"][0].update(fingerprint="b" * 64),
    lambda r: r.update(build="fail"),
    lambda r: r.update(structure="unknown"),
    lambda r: r.update(references="fail"),
    lambda r: r.update(references="unknown"),
    lambda r: r.pop("references"),
    lambda r: r.update(outside_scope="unknown"),
    lambda r: r.update(snapshot="b" * 64),
])
def test_incomplete_or_stale_evidence_is_never_an_allowed_regression(project, damage):
    with pytest.raises(ValueError):
        run(project, damage=damage)


def test_unpermitted_matching_regression_is_refused(project):
    permission, execute, calls = setup(project)
    permission["allowed"] = []
    root, task = project
    with pytest.raises(ValueError, match="not admissible"):
        drive(root, task.task_id, CONFIGURED, WORKER, iterations=1, reviewer_configuration=REVIEWER,
              migration_configuration=permission, execute=execute)
    assert "acceptance" not in calls


def test_only_coordinator_can_supply_migration_permission(project):
    permission, execute, calls = setup(project)
    permission["author"] = WORKER["author"]
    with pytest.raises(ValueError, match="coordinator"):
        drive(project[0], project[1].task_id, CONFIGURED, WORKER, iterations=1,
              migration_configuration=permission, execute=execute)
    assert calls == []


def test_real_command_adapters_admit_a_checkpoint_without_banking_a_pass(project):
    root, task = project
    permission, _, _ = setup(project)
    row = Obligation(task.subjects[0].entity_id, Dimension.BYTES, "git-history:" + task.base_revision)
    inventory = {"obligations": [{**asdict(row), "obligation_id": row.obligation_id}]}
    def command(program):
        return {"argv": [sys.executable, "-c", program], "timeout": 10}
    config = {**CONFIG,
              "context": command("print('{}')"),
              "check": command("print('{\"result\":\"fail\"}'); raise SystemExit(1)"),
              "obligations": command("print(" + repr(json.dumps(inventory)) + ")"),
              "migration_check": command("import json,sys; p=json.load(sys.stdin); "
                  "o=p['protected'][0]['obligation_id']; print(json.dumps({'snapshot':p['snapshot'],"
                  "'changed':[],'build':'pass','structure':'pass','references':'pass','outside_scope':'pass',"
                  "'measurements':[{'obligation_id':o,'fingerprint':'a'*64,'result':'fail','evidence':'synthetic'}],"
                  "'current':{o:'a'*64}}))")}
    worker = {"author": WORKER["author"], "command": command(
        "from pathlib import Path; Path('src/f.cpp').write_text('structural checkpoint'); print('{\"discoveries\":[]}')")}
    reviewer = {"author": REVIEWER["author"], "command": command(
        "import json,sys; p=json.load(sys.stdin); print(json.dumps({'snapshot':p['snapshot'],"
        "'decision':'approve','evidence':['source:src/f.cpp'],'feedback':'synthetic structural review'}))")}
    result = drive(root, task.task_id, config, worker, iterations=1,
                   reviewer_configuration=reviewer, migration_configuration=permission)
    assert result["stop_reason"] == "migration_admitted"
    assert not result["ready_for_collection"]
