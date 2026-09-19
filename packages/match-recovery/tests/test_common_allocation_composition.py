"""Native common/ordinary composition, never saved verdicts or byte waivers."""

import struct

import pytest

from match_recovery.linked_context import linked_build
from match_recovery.generated_code_residual import _compare_contexts
import test_ordinary_allocation_coverage as ordinary
from match_recovery.common_allocation_coverage import observe_common_allocations
from match_recovery.msf import SmallMsf
from test_linked_initializers import update_artifact
from test_msf import small_msf


COMMON = ("_common", 4, 0x80, 0xC0300080)


def fixture(root, **options):
    return ordinary.fixture(root, commons=options.pop("commons", (COMMON,)), **options)


def test_common_retention_composes_with_closed_native_ordinary_domain(tmp_path):
    with linked_build(fixture(tmp_path / "project")) as live:
        report = live.ordinary_allocations()
        common, = report["common_storage"]["allocations"]
        assert common["retention"] == common["characteristics_agreement"] == "pass", report
        assert common["allocation_isolation"] == report["allocation_domain"] == "pass", report
        assert report["result"] == "pass" and report["unmatched_contributions"] == []
        assert report["common_storage"]["retained_contributions"] == [3]
        assert report["remaining_linked_effects"] == "unknown"
        assert not report["ready_for_collection"]


def test_common_reserved_flag_keeps_mandatory_gap_after_retention(tmp_path):
    with linked_build(fixture(tmp_path / "project", commons=(("_common", 4, 0x80, 0xC0302080),))) as live:
        report = live.ordinary_allocations()
        common, = report["common_storage"]["allocations"]
        assert common["retention"] == "pass"
        assert common["pdb_flags"] == 0xC0302080
        assert common["characteristics_agreement"] == common["allocation_isolation"] == "unknown"
        assert report["unmatched_contributions"] == []
        assert report["result"] == report["allocation_domain"] == report["allocation_isolation"] == "unknown"
        assert any("common contribution characteristics remain unexplained" == row["reason"] for row in report["gaps"])


@pytest.mark.parametrize("flags", [0xC0300040, 0xC0301080, 0xC0304080, 0x40300080,
                                   0xE0300080, 0xC0000080, 0xC0F00080, None])
def test_unsupported_or_missing_common_contribution_does_not_gain_retention(tmp_path, flags):
    with linked_build(fixture(tmp_path / "project", commons=(("_common", 4, 0x80, flags),))) as live:
        report = live.ordinary_allocations()
        assert report["common_storage"]["allocations"][0]["retention"] == "unknown"
        assert report["result"] == report["allocation_isolation"] == "unknown"
        assert report["common_storage"]["retained_contributions"] == []


def test_declared_alignment_must_hold_at_actual_common_address(tmp_path):
    with linked_build(fixture(tmp_path / "project", commons=(("_common", 4, 0x81, 0xC0300080),))) as live:
        report = live.ordinary_allocations()
        assert report["common_storage"]["allocations"][0]["retention"] == "unknown"
        assert report["result"] == "unknown"


@pytest.mark.parametrize("ordinary_unknown", [False, True])
def test_common_zero_storage_damage_survives_other_unknowns(tmp_path, ordinary_unknown):
    with linked_build(fixture(tmp_path / "project", common_damage=True,
                             missing_sc=ordinary_unknown)) as live:
        report = live.ordinary_allocations()
        assert report["common_storage"]["input_consistency"] == "fail"
        assert report["input_consistency"] == report["result"] == "fail"


def test_common_damage_reaches_application_residual(tmp_path):
    old, new = fixture(tmp_path / "old"), fixture(tmp_path / "new", common_damage=True)
    report = _compare_contexts(old, new, [], application_scope={"baseline": {}, "current": {}})
    assert report["ordinary_allocations"]["baseline"]["result"] == "pass"
    assert report["ordinary_allocations"]["current"]["common_storage"]["input_consistency"] == "fail"
    assert report["result"] == "fail"


def test_another_passing_common_cannot_discharge_reserved_flag_obligation(tmp_path):
    with linked_build(fixture(tmp_path / "project", commons=(COMMON, ("_other", 4, 0x90, 0xC0302080)))) as live:
        report = live.ordinary_allocations()
        assert report["common_storage"]["retained_contributions"] == [3, 4]
        assert report["unmatched_contributions"] == []
        assert report["result"] == "unknown"
        assert any(row.get("symbol") == "_other" for row in report["gaps"])


def test_earlier_ordinary_extent_is_a_competitor_even_when_bytes_match(tmp_path, monkeypatch):
    monkeypatch.setattr(ordinary, "FIRST", bytes(16))
    monkeypatch.setattr(ordinary, "SECOND", bytes(8))
    with linked_build(fixture(tmp_path / "project", commons=(("_common", 4, 0x2C, 0xC0300080),))) as live:
        report = live.ordinary_allocations()
        common, = report["common_storage"]["allocations"]
        assert common["input_consistency"] == "pass"
        assert common["ordinary_overlaps"] == [0]
        assert common["retention"] == "unknown"
        assert report["allocation_isolation"] == report["result"] == "fail"


def test_saved_common_diagnostics_do_not_mutate_or_outlive_native_epoch(tmp_path):
    with linked_build(fixture(tmp_path / "project")) as live:
        report = live.ordinary_allocations()
        report["common_storage"]["allocations"].clear()
        assert live.ordinary_allocations()["common_storage"]["allocations"]
    with pytest.raises(ValueError, match="expired"):
        live.ordinary_allocations()


def test_reserved_common_characteristics_still_block_without_ordinary_sections(tmp_path):
    with linked_build(fixture(tmp_path / "project", commons=(("_common", 4, 0x80, 0xC0302080),))) as live:
        report = observe_common_allocations(live.inputs, [])
        assert report["retained_contributions"] == [3]
        assert report["result"] == report["allocation_isolation"] == "unknown"
        assert report["gaps"]


def test_an_earlier_common_refused_for_public_overlap_remains_a_competitor(tmp_path):
    with linked_build(fixture(tmp_path / "project", commons=(
            ("_first", 8, 0x80, 0xC0300080), ("_second", 4, 0x84, 0xC0300080)))) as live:
        report = live.ordinary_allocations()
        rows = report["common_storage"]["allocations"]
        assert rows[0]["proof"]["result"] == "unknown"
        assert all(row["common_overlaps"] for row in rows)
        assert all(row["retention"] == "unknown" for row in rows)
        assert report["result"] == report["allocation_isolation"] == "fail"


@pytest.mark.parametrize("site", [0x207D, 0x2080, 0x2083])
def test_pdb_fixup_write_inside_or_across_common_boundary_refuses_retention(tmp_path, site):
    def adjust(context):
        msf = SmallMsf((context.directory / "app.pdb").read_bytes())
        streams = [msf.stream(index) for index in range(msf.stream_count)]
        streams[4] = struct.pack("<HHII", 6, 0, site, 0x1000)
        update_artifact(context, "app.pdb", small_msf(streams))
    with linked_build(fixture(tmp_path / "project", adjust=adjust)) as live:
        report = live.ordinary_allocations()
        assert report["common_storage"]["allocations"][0]["retention"] == "unknown"
        assert report["common_storage"]["retained_contributions"] == []
        assert any("PDB fixup" in row["reason"] for row in report["gaps"])
        assert report["result"] != "pass"


@pytest.mark.parametrize("artifact", ["app.exe", "app.map", "app.pdb", "unit0.obj", "link.log", "link.json"])
def test_physical_common_evidence_cannot_change_within_native_epoch(tmp_path, artifact):
    context = fixture(tmp_path / "project")
    with pytest.raises(ValueError, match="changed"):
        with linked_build(context) as live:
            assert live.ordinary_allocations()["result"] == "pass"
            path = context.directory / artifact
            path.write_bytes(path.read_bytes() + b"changed")
            with pytest.raises(ValueError, match="changed"):
                live.ordinary_allocations()
