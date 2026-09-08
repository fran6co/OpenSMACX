"""Admission protocol damage controls, not a substitute for a real verifier."""

from copy import deepcopy
from dataclasses import asdict
import subprocess

import pytest

from match_recovery.driver import drive
from match_recovery.model import Dimension, Obligation
from match_recovery.reviews import acceptance_result
from test_driver import WORKER, project
from test_reviews import GATED, REVIEWER, PASS, execution, receipt


@pytest.fixture
def request_data(project):
    _, task = project
    obligations = [Obligation(task.subjects[0].entity_id, dimension, "source-history:fixture")
                   for dimension in (Dimension.BYTES, Dimension.STRUCTURE)]
    return {"task": task.to_dict(), "snapshot": "a" * 64,
            "protected": [{**asdict(row), "obligation_id": row.obligation_id} for row in obligations],
            "interpretations": [], "verification_id": "one invocation",
            "review": {"kind": "review", "author": "worker", "reviewer": "independent-reviewer",
                       "snapshot": "a" * 64, "decision": "approve", "evidence": ["fixture evidence"],
                       "feedback": "Controlled approval; actual acceptance still required."}}


def test_complete_fresh_receipt_passes_without_changing_source_or_decisions(project, request_data):
    root, _ = project
    source = (root / "src/f.cpp").read_bytes()
    assert acceptance_result(0, receipt(request_data), request=request_data) == "pass"
    assert (root / "src/f.cpp").read_bytes() == source


@pytest.mark.parametrize("dimension", ["task", "snapshot", "protected", "review", "interpretations", "verification_id"])
def test_receipt_binds_every_request_input_not_just_source_snapshot(request_data, dimension):
    stale = receipt(request_data)
    changed = deepcopy(request_data)
    if dimension == "task":
        changed[dimension]["intent"] = "Another recovery task"
    elif dimension == "snapshot":
        changed[dimension] = changed["review"]["snapshot"] = "b" * 64
    elif dimension == "protected":
        changed[dimension] = changed[dimension][1:]  # Same source, different protected set.
    elif dimension == "review":
        changed[dimension]["feedback"] = "Changed review decision evidence"
    elif dimension == "interpretations":
        changed[dimension] = [{"source": "changed interpretation"}]
    else:
        changed[dimension] = "a later invocation of exactly the same verifier"
    with pytest.raises(ValueError, match="stale or different request"):
        acceptance_result(0, stale, request=changed)


@pytest.mark.parametrize("damage", ["omitted", "current_omitted", "stale", "failed", "unknown",
                                    "duplicate", "unexpected", "no_evidence", "bad_fingerprint"])
def test_all_pass_booleans_cannot_hide_missing_stale_or_failed_obligations(request_data, damage):
    report = receipt(request_data)
    row = report["measurements"][0]
    key = row["obligation_id"]
    if damage == "omitted":
        report["measurements"].pop(0)
    elif damage == "current_omitted":
        report["current"].pop(key)
    elif damage == "stale":
        report["current"][key] = "b" * 64
    elif damage in ("failed", "unknown"):
        row["result"] = "fail" if damage == "failed" else "unknown"
    elif damage == "duplicate":
        report["measurements"].append(dict(row))
    elif damage == "unexpected":
        replacement = Obligation(request_data["protected"][0]["entity_id"], Dimension.RUNTIME, "other requirement")
        row["obligation_id"] = replacement.obligation_id  # Same count, wrong identity.
    elif damage == "no_evidence":
        row["evidence"] = ""
    else:
        row["fingerprint"] = "not a SHA-256"
    with pytest.raises(ValueError):
        acceptance_result(0, report, request=request_data)


def test_absent_measurements_are_reportable_as_unknown_never_success(request_data):
    report = receipt(request_data, {**PASS, "result": "unknown", "obligations": "unknown"})
    report.update(measurements=[], current={})
    assert acceptance_result(3, report, request=request_data) == "unknown"


def test_measured_protected_failure_is_reportable_for_worker_repair(request_data):
    report = receipt(request_data, {**PASS, "result": "fail", "obligations": "fail"})
    report["measurements"][0]["result"] = "fail"
    assert acceptance_result(1, report, request=request_data) == "fail"


def test_known_failure_remains_actionable_with_other_unknown_proofs(request_data):
    report = receipt(request_data, {**PASS, "result": "fail", "build": "fail",
                                   "runtime": "unknown", "obligations": "unknown"})
    report.update(measurements=[], current={})
    assert acceptance_result(1, report, request=request_data) == "fail"


def test_request_itself_cannot_erase_the_need_for_independent_approval(request_data):
    request_data["review"]["decision"] = "request_changes"
    with pytest.raises(ValueError, match="reviewed snapshot"):
        acceptance_result(0, receipt(request_data), request=request_data)


def test_driver_rejects_replaying_a_pass_even_when_source_and_review_are_identical(project):
    root, task = project
    run, calls = execution(project)
    receipts, requests = [], []

    def execute(command, root, name, payload):
        if command.argv[0] == "acceptance":
            requests.append(deepcopy(payload))
            if not receipts:
                receipts.append(receipt(payload))
            return 0, receipts[0]
        return run(command, root, name, payload)

    assert drive(root, task.task_id, GATED, WORKER, iterations=1,
                 reviewer_configuration=REVIEWER, execute=execute)["ready_for_collection"]
    subprocess.run(["git", "add", "recovery"], cwd=root, check=True, capture_output=True)
    subprocess.run(["git", "commit", "-m", "checkpoint independent review"], cwd=root,
                   check=True, capture_output=True)
    with pytest.raises(ValueError, match="stale or different request"):
        drive(root, task.task_id, GATED, WORKER, iterations=1,
              reviewer_configuration=REVIEWER, execute=execute)
    assert requests[0]["snapshot"] == requests[1]["snapshot"]
    assert requests[0]["verification_id"] != requests[1]["verification_id"]
    assert calls.count("review") == 1  # Review reuse does not authorize receipt reuse.


def test_legacy_boolean_only_report_is_refused(request_data):
    report = {key: value for key, value in PASS.items()
              if key not in {"request_fingerprint", "measurements", "current"}}
    with pytest.raises(ValueError, match="full-project gate report"):
        acceptance_result(0, report, request=request_data)


@pytest.mark.parametrize("damage", ["omit", "stale", "fail"])
def test_driver_cannot_collect_without_fresh_protected_measurements(project, damage):
    root, task = project
    obligation = Obligation(task.subjects[0].entity_id, Dimension.BYTES, "fixture baseline")
    run, _ = execution(project)

    def execute(command, root, name, payload):
        if command.argv[0] == "obligations":
            return 0, {"obligations": [{**asdict(obligation), "obligation_id": obligation.obligation_id}]}
        if command.argv[0] == "acceptance":
            assert len(payload["protected"]) == 1
            report = receipt(payload)
            if damage == "omit":
                report["measurements"] = []
            elif damage == "stale":
                report["current"][obligation.obligation_id] = "b" * 64
            else:
                report["measurements"][0]["result"] = "fail"
            return 0, report
        return run(command, root, name, payload)

    config = {**GATED, "obligations": {"argv": ["obligations"], "timeout": 10}}
    with pytest.raises(ValueError, match="acceptance obligations"):
        drive(root, task.task_id, config, WORKER, iterations=1,
              reviewer_configuration=REVIEWER, execute=execute)
