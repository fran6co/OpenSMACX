"""Physical ordinary section/module/SC joins, without inferred COFF indices."""

import struct

import pytest

from match_recovery.linked_context import linked_build
from match_recovery.msf import SmallMsf
from test_linked_initializers import update_artifact
from test_linked_modules import fixture as module_context, add_archive, set_modules
from test_linked_storage import data_object
from test_msf import small_msf
from test_pdb_contributions import module, contribution


def set_contributions(context, rows):
    original = SmallMsf((context.directory / "app.pdb").read_bytes())
    streams = [original.stream(index) for index in range(original.stream_count)]
    dbi = streams[3]
    modules, count = struct.unpack_from("<II", dbi, 24)
    header = bytearray(dbi[:64])
    struct.pack_into("<I", header, 28, 4 + len(rows))
    streams[3] = bytes(header) + dbi[64:64 + modules] + struct.pack("<I", 0xF12EBA2D) + rows + dbi[64 + modules + count:]
    update_artifact(context, "app.pdb", small_msf(streams))


def fixture(root, *, raw=None, rows=None, anchor=True):
    value = module_context(root)
    path, spelling = add_archive(value, [("dir/part.obj/", data_object() if raw is None else raw)], ["part.obj"])
    set_modules(value, module(b"dir/part.obj", spelling.encode()))
    set_contributions(value, contribution(section=2, offset=32, size=8, flags=0xC0302040) if rows is None else rows)
    if anchor:
        mapping = (value.directory / "app.map").read_bytes()
        update_artifact(value, "app.map", mapping + b" 0002:00000024 _private 00402024 RUNTIME:part.obj\n")
    value.archive = path
    return value


def target(report):
    return next(row for row in report["sections"] if row["input"].get("member") == "dir/part.obj")


def test_native_full_census_joins_independent_whole_section_and_bound_module(tmp_path):
    context = fixture(tmp_path / "project")
    with linked_build(context) as live:
        report = live.ordinary_contributions()
        row = target(report)
        assert row["correspondence"] == "pass" and row["module"] == row["contribution"] == 0
        assert row["input"] == {"path": str(context.archive), "member": "dir/part.obj", "occurrence": 0}
        assert row["envelope"]["address"] == 0x402020 and row["size"] == 8
        assert row["envelope"]["anchors"][0]["object_offset"] == 4
        assert row["intersections"] == [0] and report["unmatched_contributions"] == []
        assert row["flags"] == 0xC0300040 and report["contributions"][0]["flags"] == 0xC0302040
        assert row["characteristics_agreement"] == "unknown"
        assert report["census"]["enumeration"] == "pass"
        assert any(item["correspondence"] == "unknown" for item in report["sections"])
        for key in ("result", "allocation_isolation", "retained_storage_coverage", "contents", "remaining_linked_effects"):
            assert report[key] == "unknown"
        assert not report["ready_for_collection"] and not report["persisted"]


def test_size_crc_and_module_without_independent_anchor_cannot_place_a_section(tmp_path):
    with linked_build(fixture(tmp_path / "project", anchor=False)) as live:
        report = live.ordinary_contributions()
        row = target(report)
        assert row["correspondence"] == "unknown" and "envelope" not in row
        assert report["unmatched_contributions"] == [0]


@pytest.mark.parametrize("flags", [0xC0300040, 0xC0302040])
def test_raw_characteristics_agreement_is_separate_from_coordinate_correspondence(tmp_path, flags):
    rows = contribution(section=2, offset=32, size=8, flags=flags)
    with linked_build(fixture(tmp_path / "project", rows=rows)) as live:
        report = live.ordinary_contributions()
        row = target(report)
        assert row["correspondence"] == "pass"
        assert row["characteristics_agreement"] == ("pass" if flags == row["flags"] else "unknown")
        assert report["result"] == "unknown"


@pytest.mark.parametrize("options", [{"offset": 28}, {"offset": 36}, {"size": 4}, {"size": 12},
                                     {"section": 1}, {"flags": 0x60302020}, {"flags": 0x40302040}, {"flags": 0xC0402040}])
def test_whole_extent_section_and_supported_characteristics_must_agree(tmp_path, options):
    rows = contribution(**({"section": 2, "offset": 32, "size": 8, "flags": 0xC0302040} | options))
    with linked_build(fixture(tmp_path / "project", rows=rows)) as live:
        report = live.ordinary_contributions()
        assert target(report)["correspondence"] == "unknown"
        assert report["unmatched_contributions"] == [0]


def test_default_common_path_consumes_fresh_section_join_and_one_shared_census(tmp_path, monkeypatch):
    from match_recovery import linked_storage_census
    original = linked_storage_census.observe_storage
    calls = []
    def observed(*args, **kwargs):
        calls.append(1)
        return original(*args, **kwargs)
    monkeypatch.setattr(linked_storage_census, "observe_storage", observed)
    with linked_build(fixture(tmp_path / "project")) as live:
        report = live.common_intersections(("_missing",))
        assert target(report["ordinary_contributions"])["correspondence"] == "pass"
        assert report["storage_census"] == report["ordinary_contributions"]["census"]
        assert calls == [1]
        assert report["result"] == report["allocation_isolation"] == "unknown"


def test_missing_command_membership_retains_census_and_explicit_ordinary_gap(tmp_path):
    context = fixture(tmp_path / "project")
    context.report["compile_link"].pop("command_bindings")
    with linked_build(context) as live:
        report = live.common_intersections(("_missing",))
        assert report["ordinary_contributions"]["result"] == "unknown"
        assert "reason" in report["ordinary_contributions"]
        assert report["storage_census"]["enumeration"] == "pass"
        assert any(row["reason"].startswith("PDB ordinary contribution evidence refused") for row in report["gaps"])
        assert report["result"] == report["allocation_isolation"] == "unknown"


def test_explicit_section_subset_does_not_invent_full_module_or_census_coverage(tmp_path):
    with linked_build(fixture(tmp_path / "project")) as live:
        report = live.common_intersections(("_missing",), [{"owner": "RUNTIME:part.obj", "section": 1}])
        assert report["ordinary_contributions"] is None and report["storage_census"] is None
        assert report["coverage"] == "requested_data_sections_only"


@pytest.mark.parametrize("flag", [0x200, 0x800, 0x1000, 0x8000, 0x1000000, 0x2000000,
                                 0x4000000, 0x8000000, 0x10000000])
def test_known_special_contribution_forms_cannot_become_ordinary_sections(tmp_path, flag):
    rows = contribution(section=2, offset=32, size=8, flags=0xC0302040 | flag)
    with linked_build(fixture(tmp_path / "project", rows=rows)) as live:
        report = live.ordinary_contributions()
        assert target(report)["correspondence"] == "unknown"
        assert report["unmatched_contributions"] == [0]


@pytest.mark.parametrize("delta", [-4, 4])
def test_partial_competitor_cannot_leave_one_apparently_unique_exact_join(tmp_path, delta):
    from test_ordinary_contribution_controls import two_ordinary_sections
    context = fixture(tmp_path / "project", raw=two_ordinary_sections(), anchor=False)
    mapping = (context.directory / "app.map").read_bytes()
    anchors = (" 0002:00000024 _a 00402024 RUNTIME:part.obj\n"
               f" 0002:{0x24 + delta:08x} _b {0x402024 + delta:08x} RUNTIME:part.obj\n")
    update_artifact(context, "app.map", mapping + anchors.encode())
    with linked_build(context) as live:
        report = live.ordinary_contributions()
        rows = [row for row in report["sections"] if row["input"].get("member") == "dir/part.obj"]
        assert len(rows) == 2 and all(row["envelope"]["placement"] == "pass" for row in rows)
        assert all(row["intersections"] == [0] and row["correspondence"] == "unknown" for row in rows)
        assert report["unmatched_contributions"] == [0]
