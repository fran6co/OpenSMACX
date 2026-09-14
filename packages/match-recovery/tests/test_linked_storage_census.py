"""Complete native storage enumeration and automatic common overlap checks."""

from copy import deepcopy

import pytest

from match_recovery import linked_storage, linked_storage_census
from match_recovery.linked_context import linked_build
from test_common_storage_intersections import fixture as common_fixture
from test_linked_storage import OWNER
from test_linked_storage_controls import native_context
from test_linked_storage_census_controls import configured, rows
from test_linked_storage import data_object


def test_zero_extent_cannot_hide_a_defined_storage_symbol(tmp_path):
    report = linked_storage_census.observe_storage(configured(tmp_path, raw=data_object(size=0)))
    item, = rows(report, "private.obj")
    section, = item["sections"]
    assert section["classification"] == "zero_extent_with_symbols"
    assert section["symbols"][0]["name"] == "_private"
    assert section["symbols"][0]["value"] == 4
    assert report["gaps"] and report["retained_data_coverage"] == "unknown"


def test_census_derives_physical_sections_without_an_owner_list(tmp_path):
    context, _ = native_context(tmp_path / "project")
    with linked_build(context) as live:
        report = live.storage_census()
        assert report["enumeration"] == "pass"
        assert report["input_count"] == len(live.inputs.records) == 2
        assert report["section_count"] == report["positive_section_count"] == 2
        assert {owner for row in report["inputs"] for owner in row["owners"]} == {OWNER, "app.obj"}
        assert all(row["participation"] == "final_pass" for row in report["inputs"])
        assert report["section_envelopes"] == [live.data_section(OWNER, 1)]
        assert report["gaps"] == []
        assert report["result"] == report["allocation_isolation"] == "unknown"
        assert report["retained_data_coverage"] == report["remaining_linked_effects"] == "unknown"
        assert not report["ready_for_collection"] and not report["persisted"]


@pytest.mark.parametrize("limit", [True, False, 0, -1, 1.0, "2", None, 1000001])
def test_census_requires_an_exact_bounded_section_limit(tmp_path, limit):
    context, _ = native_context(tmp_path / "project")
    with linked_build(context) as live:
        with pytest.raises(ValueError, match="bounded section limit"):
            live.storage_census(max_sections=limit)


def test_exhaustion_refuses_instead_of_passing_a_truncated_census(tmp_path):
    context, _ = native_context(tmp_path / "project")
    with linked_build(context) as live:
        with pytest.raises(ValueError, match="section resource bound"):
            live.storage_census(max_sections=1)
        assert live.storage_census(max_sections=2)["section_count"] == 2


@pytest.mark.parametrize("common_offset", [4, 8])
def test_default_common_check_finds_all_ordinary_envelopes_without_a_subset(tmp_path, common_offset):
    with linked_build(common_fixture(tmp_path / "project", common_offset=common_offset)) as live:
        report = live.common_intersections(("_g",))
        assert report["coverage"] == "complete_physical_candidate_population"
        assert report["storage_census"]["enumeration"] == "pass"
        assert report["storage_census"]["section_envelopes"] == report["section_envelopes"]
        assert report["gaps"] == report["storage_census"]["gaps"]
        assert any(row["common_requests"] for row in report["storage_census"]["inputs"])
        assert report["allocation_isolation"] == report["result"] == ("fail" if common_offset == 4 else "unknown")
        assert bool(report["intersections"]) == (common_offset == 4)
        assert report["retained_data_coverage"] == "unknown"
        assert not report["ready_for_collection"]


def test_census_keeps_unplaced_sections_instead_of_failing_to_enumerate_them(tmp_path, monkeypatch):
    context, _ = native_context(tmp_path / "project")
    def refuse(*args):
        raise ValueError("test-owned missing independent placement")
    monkeypatch.setattr(linked_storage, "_observe_data_section", refuse)
    with linked_build(context) as live:
        report = live.storage_census()
        assert report["enumeration"] == "pass" and report["section_count"] == 2
        assert report["section_envelopes"] == []
        assert any("missing independent placement" in row["reason"] for row in report["gaps"])
        assert report["result"] == "unknown"


def test_batch_parses_population_and_linked_debug_index_once(tmp_path, monkeypatch):
    context, _ = native_context(tmp_path / "project")
    calls = []
    original = linked_storage.linked_coff_symbols
    def observe(image):
        calls.append(image)
        return original(image)
    monkeypatch.setattr(linked_storage, "linked_coff_symbols", observe)
    with linked_build(context) as live:
        with linked_storage._data_sections(live.inputs) as batch:
            first = batch(OWNER, 1)
            saved = deepcopy(first)
            first["input"].clear()
            first["symbols"].clear()
            first["anchors"].clear()
            for _ in range(4):
                assert batch(OWNER, 1) == saved
        assert calls == [live.inputs.image]


def test_section_observer_batch_guard_runs_even_on_refusal(tmp_path):
    context, _ = native_context(tmp_path / "project")
    with pytest.raises(ValueError):
        with linked_build(context) as live:
            with pytest.raises(ValueError, match="changed"):
                with linked_storage._data_sections(live.inputs):
                    path = context.root / "runtime.lib"
                    path.write_bytes(path.read_bytes() + b"changed")
                    raise ValueError("initial failure must not skip final freshness")


def test_default_intersection_report_does_not_grant_a_reusable_census(tmp_path):
    with linked_build(common_fixture(tmp_path / "project")) as live:
        report = live.common_intersections(("_g",))
        report["storage_census"]["inputs"].clear()
        report["section_envelopes"].clear()
        assert live.common_intersections(("_g",))["result"] == "fail"
        with pytest.raises(ValueError):
            live.common_intersections(("_g",), report)
    with pytest.raises(ValueError, match="expired"):
        live.common_intersections(("_g",))
