"""Private placement distinguishes code without deleting private owners."""

import pytest

from match_recovery.linked_context import linked_build
from match_recovery.linked_section_domain import observe_sections
from test_application_icf_sections import physical
from test_linked_comdat_private_placements import fixture


@pytest.mark.parametrize(("two", "fpo"), [(True, False), (False, False), (False, True)])
def test_private_placement_retains_each_physical_root_and_its_descendants(tmp_path, two, fpo):
    context = fixture(tmp_path / "project", two=two, fpo=fpo)
    with linked_build(context) as native:
        plain = observe_sections(context, native.inputs.objects)
        report = observe_sections(context, native.inputs.objects, native=native)
    assert physical(report) == physical(plain)
    assert report["code_roots"] == [row for row in plain["code_roots"]
                                    if row != {"owner": "dependency.obj", "symbol": "_K"}]
    assert {"owner": "unit0.obj", "symbol": "_K"} in report["code_roots"]
    owners = ("dependency.obj", "unit0.obj") if two else ("dependency.obj",)
    placements = report["comdat_unique_placements"]
    assert len(placements) == len(owners)
    assert all(row["root_identity_kind"] == "private" for row in placements)
    for owner in owners:
        assert {"owner": owner, "symbol": "_P"} in report["code_roots"]
        placement, = [row for row in placements if row["owner"] == owner]
        original, = [row for row in report["inputs"] if row["owner"] == owner]
        for section in original["sections"]:
            if section["object_section"] >= (3 if fpo else 2):
                # The private peer earns positive retention while remaining a
                # code root; annotation never excludes it.
                assert section["retention"] == "pass"
                assert section["disposition"] == "independently_placed_nodup_root"
                assert section["selection_placement"] == placement["placement_id"]
                assert section["root_identity_kind"] == "private"
    certificate, = report["comdat_selection_certificates"]
    assert set(certificate["no_other_fold"]["unique_placements"]) == {
        row["placement_id"] for row in placements}
    assert certificate["no_other_fold"]["ref_discard_certificates"] == []
    assert report["result"] == "unknown"
    assert report["retained_section_selection"] == "pass"
    assert not report["ready_for_collection"]


@pytest.mark.parametrize("damage", ["missing_map", "wrong_owner", "public_map",
    "own_ref", "ownerless", "own_section", "overlap", "wrong_module", "peer_bytes", "missing_fpo"])
def test_unqualified_private_peer_does_not_remove_any_original_root(tmp_path, damage):
    context = fixture(tmp_path / "project", two=False, fpo=damage == "missing_fpo", damage=damage)
    with linked_build(context) as native:
        plain = observe_sections(context, native.inputs.objects)
        report = observe_sections(context, native.inputs.objects, native=native)
    assert physical(report) == physical(plain)
    assert report["code_roots"] == plain["code_roots"]
    assert not report["comdat_selection_certificates"]
    assert report["gaps"] and report["result"] == "unknown"
    assert not report["ready_for_collection"]


def test_duplicate_private_map_refuses_before_a_native_domain_can_be_issued(tmp_path):
    context = fixture(tmp_path / "project", two=False, damage="duplicate_map")
    with pytest.raises(ValueError, match="duplicate|ambiguous"):
        with linked_build(context):
            pytest.fail("a duplicate private MAP entry cannot issue a native capability")
