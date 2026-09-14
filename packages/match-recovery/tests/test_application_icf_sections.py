"""Physical-domain consumers must retain original folded producers, not MAP aliases."""

import pytest

from match_recovery.linked_context import linked_build
from match_recovery.linked_section_domain import observe_sections
from test_linked_comdat_icf import fixture


def physical(report):
    """Selection may annotate originals, never rewrite or remove their evidence."""
    return [(row["input"], row["object_sha256"], [
        {key: value for key, value in section.items()
         if key not in ("retention", "disposition", "selection_certificate")}
        for section in row["sections"]]) for row in report["inputs"]]


@pytest.mark.parametrize("fpo", [False, True])
def test_only_same_key_loser_stops_seeding_roots(tmp_path, fpo):
    context = fixture(tmp_path / "project", fpo=fpo)
    with linked_build(context) as native:
        plain = observe_sections(context, native.inputs.objects)
        report = observe_sections(context, native.inputs.objects, native=native)
    assert physical(report) == physical(plain)
    assert report["section_count"] == plain["section_count"] == (6 if fpo else 3)
    assert {(row["owner"], row["symbol"]) for row in report["code_roots"]} == {
        ("unit0.obj", "_K"), ("dependency.obj", "_J")}
    dependency, = [row for row in report["inputs"] if row["owner"] == "dependency.obj"]
    for section in dependency["sections"]:
        removed = section["object_section"] >= (3 if fpo else 2)
        assert section["retention"] == ("pass" if removed else "unknown")
        assert (section.get("disposition") == "unselected_comdat_duplicate") is removed
    group, = report["comdat_folded_groups"]
    cert, = report["comdat_selection_certificates"]
    assert cert["fold_group_id"] == group["group_id"]
    assert cert["winner"]["owner"] == "unit0.obj"
    assert group["final_storage"]["owner"] == "dependency.obj"
    assert report["result"] == report["retained_section_selection"] == "unknown"
    assert not report["ready_for_collection"]


@pytest.mark.parametrize("damage", ["original_bytes", "survivor_bytes", "survivor_alias",
    "wrong_module", "duplicate_sc", "qualified", "ref_conflict"])
def test_refused_composition_preserves_every_original_root(tmp_path, damage):
    context = fixture(tmp_path / "project", damage=damage)
    with linked_build(context) as native:
        plain = observe_sections(context, native.inputs.objects)
        report = observe_sections(context, native.inputs.objects, native=native)
    assert physical(report) == physical(plain)
    assert report["code_roots"] == plain["code_roots"]
    assert len(report["code_roots"]) == 3
    assert not report["comdat_selection_certificates"]
    assert not report["comdat_folded_groups"]
    assert any("COMDAT selection remains unqualified" in row["reason"] for row in report["gaps"])
    assert report["result"] == "unknown" and not report["ready_for_collection"]
