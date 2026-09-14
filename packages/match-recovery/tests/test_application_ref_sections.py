"""Positive native REF events narrow roots without deleting physical inputs."""

from copy import deepcopy

from match_recovery.linked_context import linked_build
from match_recovery.linked_section_domain import observe_sections
from test_linked_comdat_ref import fixture


def test_native_ref_keeps_original_section_inventory_but_not_removed_code_roots(tmp_path):
    context = fixture(tmp_path)
    with linked_build(context) as native:
        plain = observe_sections(context, native.inputs.objects)
        report = observe_sections(context, native.inputs.objects, native=native)
    assert {"owner": "dependency.obj", "symbol": "_unused"} in plain["code_roots"]
    assert {"owner": "dependency.obj", "symbol": "_unused"} not in report["code_roots"]
    assert {"owner": "unit0.obj", "symbol": "_f"} in report["code_roots"]
    assert report["section_count"] == plain["section_count"]
    original, = [row for row in plain["inputs"] if row["owner"] == "dependency.obj"]
    discarded, = [row for row in report["inputs"] if row["owner"] == "dependency.obj"]
    ids = set()
    for selected, unqualified in zip(discarded["sections"], original["sections"], strict=True):
        selected = deepcopy(selected)
        ids.add(selected.pop("selection_certificate"))
        assert selected.pop("disposition") == "discarded_unreferenced_comdat"
        assert selected["retention"] == "pass"
        selected["retention"] = "unknown"
        assert selected == unqualified
    assert len(ids) == 1
    assert ids <= {row["certificate_id"] for row in report["comdat_selection_certificates"]}
    observations = report["ref_discard_observations"]
    assert observations["row_count"] == 1
    assert observations["selection_verification"] == "unknown"
    assert report["result"] == report["retained_section_selection"] == "unknown"
    assert not report["ready_for_collection"]


def test_no_discard_event_leaves_the_unmapped_original_root_required(tmp_path):
    context = fixture(tmp_path, events="")
    with linked_build(context) as native:
        report = observe_sections(context, native.inputs.objects, native=native)
    assert {"owner": "dependency.obj", "symbol": "_unused"} in report["code_roots"]
    row, = [row for row in report["inputs"] if row["owner"] == "dependency.obj"]
    assert all(section["retention"] == "unknown" for section in row["sections"])
    assert report["ref_discard_observations"]["rows"] == []


def test_repeated_discard_event_is_not_deduplicated_into_permission(tmp_path):
    context = fixture(tmp_path, events="Discarded _unused from dependency.obj\n" * 2)
    with linked_build(context) as native:
        report = observe_sections(context, native.inputs.objects, native=native)
    assert {"owner": "dependency.obj", "symbol": "_unused"} in report["code_roots"]
    assert report["ref_discard_observations"]["row_count"] == 2
    assert any("COMDAT selection remains unqualified" in row["reason"] for row in report["gaps"])


def test_contradictory_qualified_event_never_borrows_a_bare_owner(tmp_path):
    context = fixture(tmp_path, events="Discarded _unused from wrong/dependency.obj\n")
    with linked_build(context) as native:
        report = observe_sections(context, native.inputs.objects, native=native)
    assert {"owner": "dependency.obj", "symbol": "_unused"} in report["code_roots"]
    row, = [row for row in report["inputs"] if row["owner"] == "dependency.obj"]
    assert all(section["retention"] == "unknown" for section in row["sections"])
