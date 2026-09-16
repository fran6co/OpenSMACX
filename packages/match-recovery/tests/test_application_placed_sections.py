"""Placement dispositions annotate retained roots without discharging them."""

import pytest

from match_recovery.linked_context import linked_build
from match_recovery.linked_section_domain import observe_sections
from test_application_icf_sections import physical
from test_linked_comdat_private_placements import fixture as private_fixture
from test_linked_comdat_unique_placements import fixture


@pytest.mark.parametrize("fpo", [False, True])
def test_placed_sections_earn_retention_and_stay_roots(tmp_path, fpo):
    context = fixture(tmp_path / "project", fpo=fpo)
    with linked_build(context) as native:
        plain = observe_sections(context, native.inputs.objects)
        report = observe_sections(context, native.inputs.objects, native=native)
    assert physical(report) == physical(plain)
    # Only the ANY loser leaves the roots; the placed peer stays one.
    assert report["code_roots"] == [row for row in plain["code_roots"]
                                    if row != {"owner": "dependency.obj", "symbol": "_K"}]
    placement, = report["comdat_unique_placements"]
    dependency, = [row for row in report["inputs"] if row["owner"] == "dependency.obj"]
    for section in dependency["sections"]:
        if section["object_section"] >= (3 if fpo else 2):
            assert section["retention"] == "pass"
            assert section["disposition"] == "independently_placed_nodup_root"
            assert section["selection_placement"] == placement["placement_id"]
            assert section["root_identity_kind"] == "external"
    # A qualified selection leaves no unqualified gap behind.
    assert not [row for row in report["gaps"] if "COMDAT selection remains unqualified" in row["reason"]]
    assert report["result"] == report["retained_section_selection"] == "unknown"
    assert not report["ready_for_collection"]


@pytest.mark.parametrize("damage", ["overlap", "wrong_module", "ref_event", "hidden_peer",
    "duplicate_key", "any_peer", "winner_ref"])
def test_unqualified_placement_keeps_unknown_retention_and_gaps(tmp_path, damage):
    context = fixture(tmp_path / "project", damage=damage)
    with linked_build(context) as native:
        plain = observe_sections(context, native.inputs.objects)
        report = observe_sections(context, native.inputs.objects, native=native)
    assert physical(report) == physical(plain)
    assert report["code_roots"] == plain["code_roots"]
    assert not report["comdat_selection_certificates"]
    # The damaged leg keeps its section unqualified; a placement that still
    # verifies on its own does not waive that obligation.
    dependency, = [row for row in report["inputs"] if row["owner"] == "dependency.obj"]
    assert any(section["retention"] == "unknown" for section in dependency["sections"])
    assert any("COMDAT selection remains unqualified" in row["reason"] for row in report["gaps"])
    assert report["result"] == "unknown" and not report["ready_for_collection"]


def test_private_peers_each_earn_their_own_placement_annotation(tmp_path):
    context = private_fixture(tmp_path / "project", two=True)
    with linked_build(context) as native:
        plain = observe_sections(context, native.inputs.objects)
        report = observe_sections(context, native.inputs.objects, native=native)
    assert physical(report) == physical(plain)
    placements = {row["owner"]: row for row in report["comdat_unique_placements"]}
    assert set(placements) == {"dependency.obj", "unit0.obj"}
    for owner, placement in placements.items():
        assert {"owner": owner, "symbol": "_P"} in report["code_roots"]
        original, = [row for row in report["inputs"] if row["owner"] == owner]
        placed, = [section for section in original["sections"] if section["object_section"] == 2]
        assert placed["retention"] == "pass"
        assert placed["disposition"] == "independently_placed_nodup_root"
        assert placed["selection_placement"] == placement["placement_id"]
        assert placed["root_identity_kind"] == "private"
    assert not [row for row in report["gaps"] if "COMDAT selection remains unqualified" in row["reason"]]
    assert report["retained_section_selection"] == "unknown"
