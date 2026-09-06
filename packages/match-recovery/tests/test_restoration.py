"""Historical obligations are measured, not erased by a successful new body."""

from dataclasses import replace
import json

import pytest

from match_recovery.cli import main
from match_recovery.restoration import _binary_outcome, measure_restoration
from match_recovery.tasks import TaskStore
from test_application_commands import project, combined_object
from test_task_source import git


@pytest.fixture
def historical(project):
    store = TaskStore(project)
    task = store.read("owner")
    (project / "recovery/tasks/owner.json").unlink()
    path = project / "code/unit.cc"
    path.write_text(path.read_text().replace("0x00401001\n", "0x00401001 BYTE_EXACT\n"))
    # A layout observation outside the task's original evidence anchor supplies
    # scoped diagnostics, but does not invent a structural obligation for _f.
    path = project / "code/unit.hpp"
    path.write_text(path.read_text().replace("0x00401000", "0x00401010"))
    git(project, "init", "--initial-branch=work")
    git(project, "add", ".")
    git(project, "commit", "-m", "Synthetic original claim")
    store.create(replace(task, base_revision=git(project, "rev-parse", "HEAD")))
    git(project, "add", ".")
    git(project, "commit", "-m", "Authorize synthetic owner")
    return project


def measure(root, **kwargs):
    return measure_restoration(root, "owner", root / "original.exe", **kwargs)


def test_native_restoration_measures_historical_identity(historical, capsys):
    assert main(["tasks", "check-restoration", "owner", "--root", str(historical),
                 "--image", "original.exe"]) == 0
    report = json.loads(capsys.readouterr().out)
    assert report["result"] == report["restoration"] == report["references"] == "pass"
    obligation, = report["protected"]
    measurement, = report["measurements"]
    assert measurement["obligation_id"] == obligation["obligation_id"]
    assert measurement["fingerprint"] == report["current"][measurement["obligation_id"]]
    assert not report["scope_complete"] and not report["ready_for_collection"] and not report["persisted"]
    assert report["candidate"]["compilation"]["attempts"] == 1
    assert git(historical, "status", "--porcelain") == ""


def test_deleted_claim_is_still_measured_and_can_be_restored(historical):
    before = measure(historical)
    source = historical / "code/unit.cc"
    source.write_text(source.read_text().replace(" BYTE_EXACT", ""))
    git(historical, "add", ".")
    git(historical, "commit", "-m", "Synthetic claim deletion damage")
    damaged = bytearray(combined_object())
    damaged[100] = 0xCC  # same bounded extent, different instruction
    report = measure(historical, compiler=lambda *args: bytes(damaged))
    assert report["protected"] == before["protected"]
    assert report["restoration"] == report["result"] == "fail"
    assert report["references"] == "pass"
    assert report["unrestored"]["failed"] == (before["protected"][0]["obligation_id"],)
    restored = measure(historical)
    assert restored["restoration"] == "pass"
    assert restored["protected"] == before["protected"]
    assert restored["input_fingerprint"] != before["input_fingerprint"]
    assert "BYTE_EXACT" not in source.read_text()  # no stamping


def test_missing_product_binding_is_unknown_not_a_permitted_failure(historical):
    (historical / "code/unit.cc").write_text("// ORIGINAL: 0x00401010 _g 0x00401010-0x00401011\nvoid g() {}\n")
    report = measure(historical)
    assert report["restoration"] == report["references"] == "unknown"
    assert len(report["unrestored"]["unknown"]) == 1
    assert report["unrestored"]["failed"] == ()


def test_passing_offset_does_not_discharge_hierarchy_or_lifetime(historical):
    header = historical / "code/unit.hpp"
    header.write_text(header.read_text().replace("0x00401010", "0x00401000"))
    report = measure(historical)
    assert report["candidate"]["checks"]["layout"]["result"] == "pass"
    assert report["restoration"] == report["result"] == "unknown"
    structural, = [row for row in report["details"] if row["dimension"] == "structure"]
    assert structural["result"] == "unknown" and "lifetime" in structural["reason"]


@pytest.mark.parametrize("damage", ["source", "history", "image"])
def test_concurrent_changes_refuse_fresh_measurements(historical, damage):
    def compiler(*args):
        if damage == "source":
            path = historical / "code/unit.hpp"
            path.write_text(path.read_text() + "// concurrent change\n")
        elif damage == "history":
            git(historical, "commit", "--allow-empty", "-m", "Concurrent checkpoint")
        else:
            path = historical / "original.exe"
            path.write_bytes(path.read_bytes() + b"changed")
        return combined_object()
    with pytest.raises(ValueError, match="changed"):
        measure(historical, compiler=compiler)


def trial(byte="pass", refs="pass", boundary="pass", **extra):
    return {"bytes": byte, "references": refs, "boundary": boundary, "flags": [byte, refs], **extra}


@pytest.mark.parametrize("trials,expected", [
    ([trial()], ("pass", "pass")),
    ([trial("fail")], ("fail", "pass")),
    ([trial("fail", "unknown")], ("unknown", "unknown")),
    ([trial("pass", "fail")], ("fail", "fail")),
    ([trial("unknown", "unknown", "fail")], ("fail", "unknown")),
    ([trial("fail"), trial("unknown", "unknown")], ("unknown", "unknown")),
    ([trial("pass", "fail"), trial("fail", "pass")], ("fail", "pass")),
    ([trial("unknown", "unknown"), trial()], ("pass", "pass")),
    ([trial("fail", "fail", relocations=[{"result": "unknown"}])], ("unknown", "fail")),
    ([trial("fail", "fail", spans=[trial("fail", "fail"), trial("unknown", "unknown")])], ("unknown", "fail")),
])
def test_matching_permission_cannot_stitch_trials_or_hide_unknowns(trials, expected):
    assert _binary_outcome({"variants": trials})[:2] == expected


def test_summary_keeps_obligation_coverage_and_refusals(historical, capsys):
    assert main(["tasks", "check-restoration", "owner", "--root", str(historical),
                 "--image", "original.exe", "--summary"]) == 0
    report = json.loads(capsys.readouterr().out)
    assert "candidate" not in report and "measurements" not in report and "current" not in report
    assert len(report["protected"]) == len(report["details"]) == 1
    assert report["details"][0]["symbol"] == "_f"


def test_unresolved_reference_reason_identifies_the_dependency():
    result, refs, reason = _binary_outcome({"variants": [trial("fail", "unknown", relocations=[
        {"symbol": "_missing_global", "result": "unknown"}])]})
    assert result == refs == "unknown"
    assert "unresolved targets: _missing_global" in reason


def test_nonexact_code_cannot_fabricate_an_instruction_shape_failure(historical):
    source = historical / "code/unit.cc"
    source.write_text(source.read_text().replace("BYTE_EXACT", "BYTE_EXACT SEMANTIC"))
    exact = measure(historical)
    assert all(row["result"] == "pass" for row in exact["measurements"])
    assert {row["dimension"] for row in exact["protected"]} == {"bytes", "instruction_shape"}
    damaged = bytearray(combined_object())
    damaged[100] = 0xCC
    report = measure(historical, compiler=lambda *args: bytes(damaged))
    by_dimension = {row["dimension"]: row["result"] for row in report["details"]}
    assert by_dimension == {"bytes": "fail", "instruction_shape": "unknown"}
