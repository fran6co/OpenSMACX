"""Bounded shared display is diagnostic only, never admission authority."""

from copy import deepcopy
import json

import pytest

from match_recovery import acceptance_check, migration_check
from match_recovery.native_diagnostics import diagnostic_evidence
from test_native_acceptance import acceptance, migration, project, measure, proofs


def large_report():
    return {"result": "unknown", "aaa_artifacts": [
        {"path": "object-" + str(i), "sha256": "a" * 64, "result": "pass"}
        for i in range(10000)], "application": {"result": "unknown",
        "application_residual": {"scoped_code": {"graph": {"result": "unknown", "nodes": [
            {"result": "unknown", "reason": "unsupported relocation at native entry",
             "symbol": "?entry@Owner@@QAEXXZ", "owner": "owner.obj"}]}}}},
        "zzz_receipts": "x" * 14000}


def summary(evidence):
    return json.loads(evidence[0].split(": ", 1)[1])


def test_both_admission_paths_share_nested_bounded_formatter_without_mutation():
    report = large_report()
    before = deepcopy(report)
    value = diagnostic_evidence(report)
    assert migration_check._diagnostic_evidence(report) == value
    assert acceptance_check._diagnostic("linked-preservation", report) == "linked-preservation: " + "\n".join(value)
    assert sum(map(len, value)) < 18200
    row = summary(value)["reasons"][0]
    assert row["reason"] == "unsupported relocation at native entry"
    assert row["path"] == "$.application.application_residual.scoped_code.graph.nodes[0]"
    assert row["symbol"] == "?entry@Owner@@QAEXXZ" and row["owner"] == "owner.obj"
    assert summary(value)["diagnostic_only"] is True and report == before


def test_refusal_replaces_passing_reason_slots_instead_of_disappearing():
    report = {"result": "unknown", "nodes": [
        {"result": "pass", "reason": "file contents verified " + str(i)} for i in range(1000)] + [
        {"result": "unknown", "reason": "private target has no bound address", "symbol": "_private"}]}
    observed = summary(diagnostic_evidence(report))
    assert observed["reasons"][0]["reason"] == "private target has no bound address"
    assert observed["reasons"][0]["symbol"] == "_private"
    assert sum(row["occurrences"] for row in observed["reasons"]) + observed["omitted_reason_occurrences"] == 1001


@pytest.mark.parametrize("report", [None, [], {"reason": "missing result"},
    {"result": {"malformed": "x" * 20000}}, {"result": "x" * 20000},
    [{"reason": "unwrapped refusal", "symbol": "_item"}] * 1000,
    {1: {"reason": "numeric diagnostic key"}, 2: "x" * 20000}])
def test_malformed_json_report_shape_stays_bounded_without_becoming_authority(report):
    before = deepcopy(report)
    evidence = diagnostic_evidence(report)
    assert evidence and sum(map(len, evidence)) < 18200
    assert report == before


@pytest.mark.parametrize("kind", ["object", "cycle", "nan", "mixed_keys"])
def test_nonserializable_diagnostics_never_mask_original_result_with_an_exception(kind):
    report = {"result": "unknown"}
    if kind == "object":
        report["bad"] = object()
    elif kind == "cycle":
        report["cycle"] = report
    elif kind == "nan":
        report["bad"] = float("nan")
    else:
        report["bad"] = {1: "integer", "one": "string"}
    evidence = diagnostic_evidence(report)
    assert len(evidence) == 1 and len(evidence[0]) < 500
    assert "Diagnostic unavailable (not verification evidence)" in evidence[0]
    assert "report is not serializable JSON" in evidence[0]
    assert report["result"] == "unknown"


def test_full_acceptance_gate_refusal_exposes_native_path_and_keeps_unknown(acceptance, proofs, monkeypatch):
    report = large_report()
    original = deepcopy(report)
    monkeypatch.setattr(acceptance_check, "check_project", lambda *args, **kwargs: {
        "result": "unknown", "checks": [{"name": "build", "result": "unknown", "report": report}]})
    outcome = measure(acceptance)
    assert outcome["result"] == outcome["obligations"] == "unknown"
    assert not proofs
    evidence = outcome["findings"][0]["evidence"][0]
    assert evidence.startswith("build: Nested diagnostic summary")
    assert "unsupported relocation at native entry" in evidence and "?entry@Owner@@QAEXXZ" in evidence
    assert len(evidence) < 18400 and report == original
