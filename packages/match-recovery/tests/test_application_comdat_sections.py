"""Native duplicate selection narrows roots, never the physical inventory."""

from copy import deepcopy

import pytest

from match_recovery.linked_context import linked_build
from match_recovery.linked_input_epoch import input_epoch
from match_recovery.linked_section_domain import observe_sections
from test_linked_comdat_selection import fixture, object_bytes


def test_only_certified_original_loser_sections_leave_the_root_domain(tmp_path):
    context = fixture(tmp_path, loser=object_bytes(b"\x90\xc3", child=True))
    with linked_build(context) as native:
        plain = observe_sections(context, native.inputs.objects)
        report = observe_sections(context, native.inputs.objects, native=native)
    assert {"owner": "dependency.obj", "symbol": "_f"} in plain["code_roots"]
    assert report["code_roots"] == [{"owner": "unit0.obj", "symbol": "_f"}]
    assert report["section_count"] == plain["section_count"]
    certificate, = report["comdat_selection_certificates"]
    loser, = [row for row in report["inputs"] if row["owner"] == "dependency.obj"]
    original, = [row for row in plain["inputs"] if row["owner"] == "dependency.obj"]
    assert len(loser["sections"]) == 2
    for selected, unqualified in zip(loser["sections"], original["sections"]):
        selected = deepcopy(selected)
        assert selected.pop("selection_certificate") == certificate["certificate_id"]
        assert selected.pop("disposition") == "unselected_comdat_duplicate"
        assert selected["retention"] == "pass"
        selected["retention"] = "unknown"
        assert selected == unqualified
    assert report["result"] == report["retained_section_selection"] == "unknown"
    assert not report["ready_for_collection"]


@pytest.mark.parametrize("damage", [dict(corrupt=True), dict(sc=False),
    dict(wrong_module=True), dict(loser=object_bytes(alias=True))])
def test_refused_duplicate_evidence_keeps_the_original_physical_roots(tmp_path, damage):
    context = fixture(tmp_path, **damage)
    with linked_build(context) as native:
        report = observe_sections(context, native.inputs.objects, native=native)
    assert {"owner": "dependency.obj", "symbol": "_f"} in report["code_roots"]
    assert {"owner": "unit0.obj", "symbol": "_f"} in report["code_roots"]
    assert any("COMDAT selection remains unqualified" in row["reason"] for row in report["gaps"])
    assert not report["comdat_selection_certificates"]
    assert all(section["retention"] == "unknown" for row in report["inputs"] for section in row["sections"])


def test_duplicate_public_placement_refuses_before_observation(tmp_path):
    context = fixture(tmp_path, duplicate_map=True)
    with pytest.raises(ValueError, match="duplicate MSVC link map"):
        with linked_build(context) as native:
            observe_sections(context, native.inputs.objects, native=native)


def test_selection_requires_exact_native_context_and_input_epoch(tmp_path):
    first, second = fixture(tmp_path / "first"), fixture(tmp_path / "second")
    with linked_build(first) as a, linked_build(second) as b:
        with pytest.raises(ValueError, match="exact native"):
            observe_sections(first, a.inputs.objects, native=b)
        with input_epoch(second, b.inputs.objects) as epoch:
            with pytest.raises(ValueError):
                observe_sections(first, a.inputs.objects, native=a, epoch=epoch)


def test_serialized_domain_cannot_supply_a_selection_handle(tmp_path):
    context = fixture(tmp_path)
    with linked_build(context) as native:
        report = observe_sections(context, native.inputs.objects, native=native)
        with pytest.raises(ValueError, match="exact native"):
            observe_sections(context, native.inputs.objects, native=report)
        report["comdat_selection_certificates"].clear()
        assert observe_sections(context, native.inputs.objects, native=native)["comdat_selection_certificates"]
