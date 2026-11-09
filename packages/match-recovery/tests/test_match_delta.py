"""Baseline deltas only describe movement; they never change a verdict."""

from match_recovery.match_delta import compare_match_reports


def _row(address, path, verdict, result=None, claimed=False, matching=10):
    return {"address": address, "path": path, "name": address, "verdict": verdict, "result": result,
            "byte_obligation": claimed, "shape_obligation": False,
            "matching_instructions": matching, "original_instructions": 10}


def test_regressions_restorations_and_free_movement_are_reported():
    baseline = {"input_fingerprint": "old", "measurements": [
        _row("0x1", "src/a.cpp", "BYTE_EXACT", "pass", True),
        _row("0x2", "src/a.cpp", "MISMATCH", "fail", True, 8),
        _row("0x3", "src/a.cpp", "MISMATCH", matching=7),
        _row("0x4", "src/a.cpp", "BYTE_EXACT"),
        _row("0x5", "src/b.cpp", "MISMATCH", matching=5)]}
    current = {"result": "fail", "measurements": [
        _row("0x1", "src/a.cpp", "MISMATCH", "fail", True, 9),
        _row("0x2", "src/a.cpp", "BYTE_EXACT", "pass", True),
        _row("0x3", "src/a.cpp", "BYTE_EXACT"),
        _row("0x4", "src/a.cpp", "MISMATCH", matching=9),
        _row("0x6", "src/a.cpp", "MISMATCH")]}
    delta = compare_match_reports(baseline, current)
    assert [row["address"] for row in delta["newly_regressed"]] == ["0x1"]
    assert [row["address"] for row in delta["restored"]] == ["0x2"]
    assert [row["address"] for row in delta["newly_free"]] == ["0x3"]
    assert [row["address"] for row in delta["lost_free"]] == ["0x4"]
    assert [row["address"] for row in delta["appeared"]] == ["0x6"]
    assert [row["address"] for row in delta["disappeared"]] == ["0x5"]
    assert delta["compared_rows"] == 4 and current["result"] == "fail"


def test_scoped_reports_do_not_call_omitted_rows_disappeared():
    baseline = {"measurements": [_row("0x1", "src/a.cpp", "BYTE_EXACT"), _row("0x2", "src/b.cpp", "BYTE_EXACT")]}
    current = {"scope": {"complete": False}, "measurements": [_row("0x1", "src/a.cpp", "BYTE_EXACT")]}
    delta = compare_match_reports(baseline, current)
    assert delta["disappeared"] == [] and delta["changed"] == [] and delta["compared_rows"] == 1
