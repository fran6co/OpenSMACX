"""Positive REF competitor composition keeps the complete physical domain."""

import json

import pytest

from match_recovery.linked_context import linked_build
from match_recovery.linked_section_domain import observe_sections
from test_application_icf_sections import physical
from test_linked_comdat_unique_placements import fixture as placed_fixture
from test_linked_modules import set_link
from test_ordinary_contributions import set_contributions
from test_pdb_contributions import contribution


def fixture(root, *, damage=None):
    context = placed_fixture(root, damage="missing_map")
    # P is an original identical NODUP root, not retained code. Only K has
    # final storage; no missing placement is itself used as discard evidence.
    set_contributions(context, contribution(section=1, offset=0, size=16,
                                           flags=0x60501020, module=0))
    evidence = json.loads((context.directory / "link.json").read_bytes())
    if damage != "missing_option":
        evidence["arguments"].append("/OPT:REF")
    if damage == "forced":
        evidence["arguments"].append("/INCLUDE:_P")
    event = "Discarded _P from dependency.obj\n"
    if damage == "missing_event": event = ""
    if damage == "repeated": event *= 2
    if damage == "ownerless": event = "Discarded _P\n"
    if damage == "qualified": event = "Discarded _P from elsewhere/dependency.obj\n"
    log = (context.directory / "link.log").read_text().replace("Start Pass2", event + "Start Pass2")
    set_link(context, evidence, log)
    return context


def test_positive_ref_certificate_can_discharge_competitor_without_erasing_inventory(tmp_path):
    context = fixture(tmp_path / "project")
    with linked_build(context) as native:
        plain = observe_sections(context, native.inputs.objects)
        report = observe_sections(context, native.inputs.objects, native=native)
    assert physical(report) == physical(plain)
    assert {(row["owner"], row["symbol"]) for row in plain["code_roots"]} == {
        ("unit0.obj", "_K"), ("dependency.obj", "_K"), ("dependency.obj", "_P")}
    assert report["code_roots"] == [{"owner": "unit0.obj", "symbol": "_K"}]
    by_kind = {row["check"]: row for row in report["comdat_selection_certificates"]}
    assert len(by_kind) == len(report["comdat_selection_certificates"]) == 2
    any_copy = by_kind["native_comdat_any_selection"]
    removed = by_kind["native_comdat_ref_selection"]
    assert any_copy["no_other_fold"]["ref_discard_certificates"] == [removed["certificate_id"]]
    assert removed["original"]["root"]["name"] == "_P"
    assert removed["incoming_references"] == "unknown"
    dependency, = [row for row in report["inputs"] if row["owner"] == "dependency.obj"]
    assert {row["object_section"]: row["disposition"] for row in dependency["sections"]} == {
        1: "unselected_comdat_duplicate", 2: "discarded_unreferenced_comdat"}
    assert not report["comdat_unique_placements"]
    assert report["result"] == report["retained_section_selection"] == "unknown"
    assert not report["ready_for_collection"]


@pytest.mark.parametrize("damage", ["missing_event", "missing_option", "repeated", "ownerless", "qualified", "forced"])
def test_unqualified_ref_competitor_keeps_all_original_roots(tmp_path, damage):
    context = fixture(tmp_path / "project", damage=damage)
    with linked_build(context) as native:
        plain = observe_sections(context, native.inputs.objects)
        report = observe_sections(context, native.inputs.objects, native=native)
    assert physical(report) == physical(plain)
    assert report["code_roots"] == plain["code_roots"]
    assert not report["comdat_selection_certificates"]
    assert not report["comdat_unique_placements"]
    assert report["gaps"] and report["result"] == "unknown"
    assert not report["ready_for_collection"]
