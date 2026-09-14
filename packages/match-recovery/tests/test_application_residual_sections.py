"""Physical application roots include code/data outside initializer reachability."""

from copy import deepcopy
import struct

import pytest

from match_recovery.generated_code_residual import _compare_contexts
from match_recovery.linked_context import linked_build
from match_recovery.linked_input_epoch import input_epoch
from match_recovery.linked_section_domain import observe_sections
from test_binary import coff_fixture
from test_coff_inventory import imported
from test_generated_selection_residual import selected_context
from test_linked_data_ownership import fixture as data_context
from test_linked_initializers import update_artifact
from test_slot_local_initializer_order import physical_context, replace_object


def test_roots_come_from_all_selected_code_sections_not_initializer_calls(tmp_path):
    context = selected_context(tmp_path / "physical", entry_call=True)
    with linked_build(context) as live:
        report = observe_sections(context, live.inputs.objects)
        assert report["enumeration"] == "pass"
        assert {row["owner"] for row in report["code_roots"]} >= {"dependency.obj", "RUNTIME:sibling.obj"}
        assert live.code(report["code_roots"])["result"] == "pass"
        assert report["retained_section_selection"] == "unknown"


def test_unrelated_sibling_is_now_in_native_code_residual(tmp_path):
    old = selected_context(tmp_path / "old", entry_call=True)
    new = selected_context(tmp_path / "new", entry_call=True, changed=True)
    report = _compare_contexts(old, new, [], application_scope={"baseline": {}, "current": {}})
    assert "physical_sections" in report
    assert report["result"] != "pass", report
    assert report["input_selection"]["dependency_families"] == "fail"


def test_missing_code_map_row_keeps_physical_section_obligation(tmp_path):
    def remove(context):
        mapping = (context.directory / "app.map").read_text()
        update_artifact(context, "app.map", "\n".join(line for line in mapping.splitlines()
                                                      if " _dep " not in line).encode())
    from test_common_request_coverage import common_context
    context = common_context(tmp_path / "physical", witness=False, damage=remove)[0]
    with linked_build(context) as live:
        report = observe_sections(context, live.inputs.objects)
        assert {"owner": "dependency.obj", "symbol": "_dep"} in report["code_roots"]
        code = live.code(report["code_roots"])
        assert code["result"] != "pass"
        assert any(row["owner"] == "dependency.obj" and row["sections"] for row in report["inputs"])


def test_unreferenced_anonymous_data_remains_a_seed_without_any_map_anchor(tmp_path):
    context = data_context(tmp_path)
    with linked_build(context) as live:
        report = observe_sections(context, live.inputs.objects)
        assert {"owner": "unit0.obj", "section": 1} in report["data_sections"]
        row, = [row for row in report["inputs"] if row["owner"] == "unit0.obj"]
        assert row["sections"][0]["size"] == 8
        assert row["sections"][0]["retention"] == "unknown"


@pytest.mark.parametrize("same", [False, True])
def test_ambiguous_archive_candidates_are_never_seeded_or_dropped(tmp_path, same):
    context = selected_context(tmp_path / "physical", imports=(imported(), imported() if same else imported("_Other@8")))
    with linked_build(context) as live:
        report = observe_sections(context, live.inputs.objects)
        rows = [row for row in report["inputs"] if row["format"] == "short_import"]
        assert len(rows) == 2 and all(row["selection"] == "unknown" for row in rows)
        assert all(len(row["families"]) == 2 for row in rows)
        assert not any("DEMO" in row["owner"] for row in report["code_roots"])


def test_archive_candidate_only_sections_remain_enumerated_but_not_roots(tmp_path):
    context = selected_context(tmp_path / "physical", present=False)
    with linked_build(context) as live:
        report = observe_sections(context, live.inputs.objects)
        rows = [row for row in report["inputs"] if row["participation"] == "archive_candidate_only"]
        assert len(rows) == 1 and rows[0]["sections"]
        assert rows[0]["owner"] is None
        assert not any("sibling" in root["owner"] for root in report["code_roots"])


@pytest.mark.parametrize("limit", [True, 0, 1000001, 1])
def test_domain_bound_refuses_without_truncating_to_complete_subset(tmp_path, limit):
    context = selected_context(tmp_path / "physical")
    with linked_build(context) as live:
        with pytest.raises(ValueError, match="bound"):
            observe_sections(context, live.inputs.objects, max_sections=limit)


def test_domain_reuses_authentic_epoch_and_defends_its_return_values(tmp_path):
    context = selected_context(tmp_path / "physical")
    with linked_build(context) as live:
        with input_epoch(context, live.inputs.objects) as epoch:
            report = observe_sections(context, live.inputs.objects, epoch=epoch)
            expected = deepcopy(report)
            report["inputs"][0]["sections"].clear()
            report["code_roots"].clear()
            assert observe_sections(context, live.inputs.objects, epoch=epoch) == expected
        with pytest.raises(ValueError, match="expired"):
            observe_sections(context, live.inputs.objects, epoch=epoch)


def ordinary_pair(monkeypatch, tmp_path, *, scoped):
    def change(context):
        raw = bytearray(coff_fixture(b"\x90\xc3"))
        at, = struct.unpack_from("<I", raw, 8)
        raw[at:at + 8] = b"_dep\0\0\0\0"
        replace_object(context, bytes(raw), index=1)
        image = bytearray((context.directory / "app.exe").read_bytes())
        image[0x3E0:0x3F0] = b"\x90\xc3".ljust(16, b"\xcc")
        update_artifact(context, "app.exe", bytes(image))
    old = physical_context(monkeypatch, tmp_path / "old")
    new = physical_context(monkeypatch, tmp_path / "new", damage=change)
    scope = {"src/dependency.cpp": {"_dep": "source-owned-task-function"}} if scoped else {}
    return _compare_contexts(old, new, [], application_scope={"baseline": scope, "current": scope})


def test_authorized_single_function_body_changes_are_projected_not_whole_tus(monkeypatch, tmp_path):
    report = ordinary_pair(monkeypatch, tmp_path, scoped=True)
    assert report["result"] == "pass", report
    assert report["omitted_bodies"] == {"baseline": 1, "current": 1}
    assert report["remaining_linked_effects"] == "unknown"


def test_same_function_change_without_source_scope_fails_code_comparison(monkeypatch, tmp_path):
    report = ordinary_pair(monkeypatch, tmp_path, scoped=False)
    assert report["result"] == "fail", report
    assert report["omitted_bodies"] == {"baseline": 0, "current": 0}


def test_source_function_scope_cannot_waive_an_initializer_permission(monkeypatch, tmp_path):
    old = physical_context(monkeypatch, tmp_path / "old")
    new = physical_context(monkeypatch, tmp_path / "new")
    scope = {"src/unit0.cpp": {"_$E1": "task-function"}}
    report = _compare_contexts(old, new, [], application_scope={"baseline": scope, "current": scope})
    assert report["result"] == "unknown", report
    assert "generated-root permission" in report["reason"]


def test_dependency_name_cannot_borrow_compiled_source_function_scope(tmp_path):
    old = selected_context(tmp_path / "old", entry_call=True)
    new = selected_context(tmp_path / "new", entry_call=True)
    # Dependency code cannot gain source task scope, even by matching its name
    # to a source TU. The source qualifier must match an actual compile receipt.
    scope = {"src/runtime.cpp": {"_sibling": "task-function"}}
    report = _compare_contexts(old, new, [], application_scope={"baseline": scope, "current": scope})
    assert report["omitted_bodies"] == {"baseline": 0, "current": 0}, report


def test_full_physical_root_expansion_retains_folded_native_members(monkeypatch, tmp_path):
    from test_generated_residual_folded_roots import context
    old = context(monkeypatch, tmp_path / "old")
    new = context(monkeypatch, tmp_path / "new")
    report = _compare_contexts(old, new, [], application_scope={"baseline": {}, "current": {}})
    assert report["result"] == "pass", report
    assert report["preserved_producers"] == 3
