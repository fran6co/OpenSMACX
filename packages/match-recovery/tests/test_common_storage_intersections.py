"""Proved ordinary extents expose earlier overlaps; subsets never prove absence."""

import json

import pytest

from match_recovery.linked_context import linked_build
from test_common_preservation import build
from test_link_file_evidence import stamp
from test_linked_initializers import update_artifact
from test_linked_storage import data_object


def fixture(root, *, common_offset=4, section_offset=0):
    context = build(root, offset=common_offset)
    path = root / "ordinary.obj"
    path.write_bytes(data_object((("_prior", 0, 2),)))
    evidence = json.loads((context.directory / "link.json").read_bytes())
    evidence["explicit_inputs"].append(str(path))
    evidence["arguments"].append(str(path))
    evidence["files"].append({"path": str(path), "before": stamp(path.read_bytes()), "after": stamp(path.read_bytes())})
    log = (context.directory / "link.log").read_text().replace("End Pass2", "ordinary.obj\nEnd Pass2")
    evidence["link_log_sha256"] = stamp(log.encode())["sha256"]
    mapping = (context.directory / "app.map").read_text().replace(" entry point",
        f" 0002:{section_offset:08x} _prior {0x402000 + section_offset:08x} ordinary.obj\n entry point")
    update_artifact(context, "link.log", log.encode())
    update_artifact(context, "link.json", json.dumps(evidence).encode())
    update_artifact(context, "app.map", mapping.encode())
    return context


def test_a_section_start_before_all_queried_publics_still_exposes_overlap(tmp_path):
    context = fixture(tmp_path / "project")
    with linked_build(context) as live:
        # The old requested-span/public-start check misses the earlier extent.
        assert live.common(("_g",))["result"] == "pass"
        report = live.common_intersections(("_g",), [{"owner": "ordinary.obj", "section": 1}])
        assert report["result"] == report["allocation_isolation"] == "fail"
        assert report["intersections"] == [{"symbol": "_g", "owner": "ordinary.obj", "object_section": 1,
            "input": {"path": str(context.root / "ordinary.obj")}, "address": 0x402004, "size": 4}]
        assert report["section_envelopes"][0]["size"] == 8
        assert report["retained_data_coverage"] == report["remaining_linked_effects"] == "unknown"
        assert not report["ready_for_collection"]


@pytest.mark.parametrize("common_offset,section_offset", [(8, 0), (0, 4), (16, 0)])
def test_disjoint_or_adjacent_requested_sections_do_not_prove_complete_isolation(tmp_path, common_offset, section_offset):
    context = fixture(tmp_path / "project", common_offset=common_offset, section_offset=section_offset)
    with linked_build(context) as live:
        report = live.common_intersections(("_g",), [{"owner": "ordinary.obj", "section": 1}])
        assert report["common_allocations"]["result"] == "pass"
        assert report["intersections"] == report["gaps"] == []
        assert report["result"] == report["allocation_isolation"] == "unknown"
        assert report["coverage"] == "requested_data_sections_only"
        assert not report["ready_for_collection"]


def test_unplaceable_subset_retains_gap_without_dropping_a_proved_overlap(tmp_path):
    context = fixture(tmp_path / "project")
    with linked_build(context) as live:
        report = live.common_intersections(("_g",), [{"owner": "ordinary.obj", "section": 1},
                                                   {"owner": "missing.obj", "section": 1}])
        assert report["result"] == "fail"
        assert len(report["intersections"]) == len(report["gaps"]) == 1
        assert report["retained_data_coverage"] == "unknown"


def test_missing_common_evidence_cannot_be_replaced_by_selected_data(tmp_path):
    context = fixture(tmp_path / "project")
    with linked_build(context) as live:
        report = live.common_intersections(("_missing",), [{"owner": "ordinary.obj", "section": 1}])
        assert report["common_allocations"]["result"] == report["result"] == "unknown"
        assert report["intersections"] == []


@pytest.mark.parametrize("sections", [[], [{"owner": "ordinary.obj", "section": True}],
    [{"owner": "ordinary.obj", "section": 1, "size": 8}],
    [{"owner": "ordinary.obj", "section": 1}, {"owner": "ORDINARY.OBJ", "section": 1}],
    [{"owner": "ordinary.obj", "section": 1}] * 4097])
def test_query_cannot_supply_saved_extents_or_duplicate_unbounded_identities(tmp_path, sections):
    with linked_build(fixture(tmp_path / "project")) as live:
        with pytest.raises(ValueError, match="unique owner/section"):
            live.common_intersections(("_g",), sections)


def test_intersection_reports_do_not_hold_mutable_native_state_or_outlive_epoch(tmp_path):
    with linked_build(fixture(tmp_path / "project")) as live:
        selections = [{"owner": "ordinary.obj", "section": 1}]
        report = live.common_intersections(("_g",), selections)
        report["common_allocations"]["input_coverage"]["inputs"].clear()
        report["section_envelopes"][0]["input"]["path"] = "forged"
        assert live.common_intersections(("_g",), selections)["result"] == "fail"
    with pytest.raises(ValueError, match="expired"):
        live.common_intersections(("_g",), selections)
