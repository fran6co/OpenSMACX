"""A separate NODUP placement distinguishes bytes without deleting its owner."""

import pytest

from match_recovery.linked_context import linked_build
from match_recovery.linked_section_domain import observe_sections
from test_application_icf_sections import physical
from test_linked_comdat_unique_placements import fixture


@pytest.mark.parametrize("fpo", [False, True])
def test_distinct_peer_stays_in_physical_inventory_and_root_domain(tmp_path, fpo):
    context = fixture(tmp_path / "project", fpo=fpo)
    with linked_build(context) as native:
        plain = observe_sections(context, native.inputs.objects)
        report = observe_sections(context, native.inputs.objects, native=native)
    assert physical(report) == physical(plain)
    assert {(row["owner"], row["symbol"]) for row in report["code_roots"]} == {
        ("unit0.obj", "_K"), ("dependency.obj", "_P")}
    assert {"owner": "dependency.obj", "symbol": "_K"} in plain["code_roots"]
    peer, = report["comdat_unique_placements"]
    assert peer["result"] == "pass"
    cert, = report["comdat_selection_certificates"]
    assert peer["placement_id"] in cert["no_other_fold"]["unique_placements"]
    dependency, = [row for row in report["inputs"] if row["owner"] == "dependency.obj"]
    for section in dependency["sections"]:
        if section["object_section"] >= (3 if fpo else 2):
            assert section["retention"] == "unknown" and "disposition" not in section
    assert report["result"] == report["retained_section_selection"] == "unknown"
    assert not report["ready_for_collection"]


@pytest.mark.parametrize("damage", ["overlap", "wrong_module", "ref_event", "hidden_peer",
    "duplicate_key", "any_peer", "winner_ref"])
def test_unqualified_distinction_keeps_every_original_root(tmp_path, damage):
    context = fixture(tmp_path / "project", damage=damage)
    with linked_build(context) as native:
        plain = observe_sections(context, native.inputs.objects)
        report = observe_sections(context, native.inputs.objects, native=native)
    assert physical(report) == physical(plain)
    assert report["code_roots"] == plain["code_roots"]
    assert not report["comdat_selection_certificates"]
    assert any("COMDAT selection remains unqualified" in row["reason"] for row in report["gaps"])
    assert report["result"] == "unknown" and not report["ready_for_collection"]
