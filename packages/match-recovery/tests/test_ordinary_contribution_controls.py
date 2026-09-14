"""Independent ordinary-storage/PDB correspondence and live-epoch controls."""

from dataclasses import asdict
from hashlib import sha256
import struct

import pytest

from match_recovery import linked_context, linked_contributions
from match_recovery.msf import SmallMsf
from match_recovery.pdb_contributions import bound_contributions
from match_recovery.pe_coff import PeImage
from test_linked_initializers import update_artifact
from test_msf import small_msf
from test_pdb_contributions import contribution, module
from test_ordinary_contributions import fixture


def observe(context):
    with linked_context.linked_build(context) as live:
        return live.ordinary_contributions()


def inventory(context):
    raw = (context.directory / "app.exe").read_bytes()
    return bound_contributions(PeImage(raw, sha256(raw).hexdigest()),
                               (context.directory / "app.pdb").read_bytes())


def replace_records(context, records, *, append_module=None):
    msf = SmallMsf((context.directory / "app.pdb").read_bytes())
    streams = [msf.stream(index) for index in range(msf.stream_count)]
    dbi = streams[3]
    module_size, record_size = struct.unpack_from("<II", dbi, 24)
    modules = dbi[64:64 + module_size] + (append_module or b"")
    rows = b"".join(contribution(**row) for row in records)
    header = bytearray(dbi[:64])
    struct.pack_into("<II", header, 24, len(modules), 4 + len(rows))
    streams[3] = bytes(header) + modules + struct.pack("<I", 0xF12EBA2D) + rows + dbi[64 + module_size + record_size:]
    update_artifact(context, "app.pdb", small_msf(streams))


def target(context):
    report = observe(context)
    matched = [row for row in report["sections"] if row["correspondence"] == "pass"]
    assert len(matched) == 1
    return matched[0]


def target_index(context):
    row = target(context)
    return row["intersections"][0]


def test_automatic_inventory_retains_unmatched_sc_without_claiming_completeness(tmp_path):
    context = fixture(tmp_path / "project")
    old = inventory(context)
    rows = [asdict(row) for row in old.contributions]
    extra = dict(rows[target_index(context)])
    extra["offset"] += 64
    rows.append(extra)
    replace_records(context, rows)
    report = observe(context)
    assert len(report["contributions"]) == len(rows)
    assert len(rows) - 1 in report["unmatched_contributions"]
    assert any(row["correspondence"] == "pass" for row in report["sections"])
    assert report["census"]["enumeration"] == "pass"
    assert report["result"] == report["retained_storage_coverage"] == report["allocation_isolation"] == "unknown"
    assert not report["ready_for_collection"]


@pytest.mark.parametrize("damage", ["missing", "shifted", "larger", "smaller", "duplicate", "partial_overlap"])
def test_size_crc_similarity_cannot_replace_unique_exact_independent_extent(tmp_path, damage):
    context = fixture(tmp_path / "project")
    index = target_index(context)
    rows = [asdict(row) for row in inventory(context).contributions]
    row = rows[index]
    if damage == "missing": rows.pop(index)
    elif damage == "shifted": row["offset"] += 4
    elif damage == "larger": row["size"] += 4
    elif damage == "smaller": row["size"] -= 1
    elif damage == "duplicate": rows.append(dict(row))
    else: rows.append({**row, "offset": row["offset"] + row["size"] - 1, "size": 2})
    replace_records(context, rows)
    report = observe(context)
    assert all(row["correspondence"] == "unknown" for row in report["sections"])
    assert report["allocation_isolation"] == "unknown"
    assert len(report["contributions"]) == len(rows)


def test_exact_coordinates_under_unbound_module_do_not_borrow_physical_owner(tmp_path):
    context = fixture(tmp_path / "project")
    index = target_index(context)
    old = inventory(context)
    rows = [asdict(row) for row in old.contributions]
    rows[index]["module"] = len(old.modules)
    replace_records(context, rows, append_module=module(b"* Linker *", b""))
    report = observe(context)
    assert all(row["correspondence"] == "unknown" for row in report["sections"])
    assert index in report["unmatched_contributions"]
    assert report["modules"]["modules"][-1]["result"] == "unknown"


def test_duplicate_module_physical_mapping_cannot_be_repaired_by_exact_sc(tmp_path):
    context = fixture(tmp_path / "project")
    index = target_index(context)
    old = inventory(context)
    record = old.contributions[index]
    owner = old.modules[record.module]
    replace_records(context, [asdict(row) for row in old.contributions],
                    append_module=module(owner.name.encode(), owner.object_name.encode()))
    report = observe(context)
    assert all(row["correspondence"] == "unknown" for row in report["sections"])
    assert index in report["unmatched_contributions"]


def test_exact_sc_cannot_borrow_a_different_independently_bound_module(tmp_path):
    context = fixture(tmp_path / "project")
    index = target_index(context)
    old = inventory(context)
    rows = [asdict(row) for row in old.contributions]
    rows[index]["module"] = len(old.modules)
    other = str(context.objects[0]).encode()
    replace_records(context, rows, append_module=module(other, other))
    report = observe(context)
    assert report["modules"]["modules"][-1]["result"] == "pass"
    assert all(row["correspondence"] == "unknown" for row in report["sections"])
    assert index in report["unmatched_contributions"]


def two_ordinary_sections():
    # Independently addressable ordinary sections, no guessed function extents
    # or parser mutation: both have raw bytes and distinct private anchors.
    sections = b"".join(struct.pack("<8sIIIIIIHHI", name, 0, 0, 8, 100 + index * 8,
        0, 0, 0, 0, 0xC0300040) for index, name in enumerate((b".data$a", b".data$b")))
    symbols = b"".join(struct.pack("<8sIhHBB", name, 4, index + 1, 0, 3, 0)
                        for index, name in enumerate((b"_a", b"_b")))
    return struct.pack("<HHIIIHH", 0x14C, 2, 0, 116, 2, 0, 0) + sections + bytes(16) + symbols + struct.pack("<I", 4)


@pytest.mark.parametrize("reversed_anchors", [False, True])
def test_two_independently_placed_sections_cannot_borrow_one_exact_sc(tmp_path, reversed_anchors):
    context = fixture(tmp_path / "project", raw=two_ordinary_sections(), anchor=False)
    anchors = [b" 0002:00000024 _a 00402024 RUNTIME:part.obj\n",
               b" 0002:00000024 _b 00402024 RUNTIME:part.obj\n"]
    mapping = (context.directory / "app.map").read_bytes()
    update_artifact(context, "app.map", mapping + b"".join(reversed(anchors) if reversed_anchors else anchors))
    report = observe(context)
    rows = [row for row in report["sections"] if row["input"].get("member") == "dir/part.obj"]
    assert {row["object_section"] for row in rows} == {1, 2}
    assert all(row["envelope"]["address"] == 0x402020 for row in rows)
    assert all(row["intersections"] == [0] and row["correspondence"] == "unknown" for row in rows)
    assert report["unmatched_contributions"] == [0]


@pytest.mark.parametrize("extra_flag", [0x2000, 0x100, 0x80000])
def test_unexplained_flags_are_retained_and_never_certified_as_equivalent(tmp_path, extra_flag):
    raw_flags = 0xC0300040
    rows = contribution(section=2, offset=32, size=8, flags=raw_flags | extra_flag)
    context = fixture(tmp_path / "project", rows=rows)
    report = observe(context)
    row = next(row for row in report["sections"] if row["input"].get("member") == "dir/part.obj")
    assert row["correspondence"] == "pass"
    assert row["characteristics_agreement"] == "unknown"
    assert row["flags"] == raw_flags and report["contributions"][0]["flags"] == raw_flags | extra_flag
    assert report["result"] == "unknown"


def test_same_module_and_sc_size_crc_do_not_supply_a_missing_native_anchor(tmp_path):
    context = fixture(tmp_path / "project", anchor=False,
                      rows=contribution(section=2, offset=32, size=8, flags=0xC0302040,
                                        data_crc=0x12345678, relocation_crc=0x87654321))
    report = observe(context)
    assert report["modules"]["modules"][0]["result"] == "pass"
    row = next(row for row in report["sections"] if row["input"].get("member") == "dir/part.obj")
    assert row["correspondence"] == "unknown" and "envelope" not in row
    assert report["contributions"][0]["data_crc"] == 0x12345678
    assert report["unmatched_contributions"] == [0]


@pytest.mark.parametrize("delta", [-1, 1])
def test_adjacent_nonintersecting_sc_stays_unmatched_without_destroying_exact_match(tmp_path, delta):
    context = fixture(tmp_path / "project")
    index = target_index(context)
    rows = [asdict(row) for row in inventory(context).contributions]
    selected = rows[index]
    extra = {**selected, "size": 1,
             "offset": selected["offset"] - 1 if delta < 0 else selected["offset"] + selected["size"]}
    assert extra["offset"] >= 0
    rows.append(extra)
    replace_records(context, rows)
    report = observe(context)
    assert any(row["correspondence"] == "pass" for row in report["sections"])
    assert len(rows) - 1 in report["unmatched_contributions"]


def test_native_outputs_are_defensive_and_handle_expiry_prevents_reobservation(tmp_path):
    context = fixture(tmp_path / "project")
    with linked_context.linked_build(context) as live:
        expected = live.ordinary_contributions()
        changed = live.ordinary_contributions()
        changed["sections"].clear()
        changed["contributions"].clear()
        changed["modules"]["modules"].clear()
        changed["census"]["inputs"].clear()
        assert live.ordinary_contributions() == expected
    assert context.alive
    with pytest.raises(ValueError, match="expired"):
        live.ordinary_contributions()


def mutate(context, live, damage):
    if damage in {"pdb", "image", "map", "receipt", "log"}:
        name = {"pdb": "app.pdb", "image": "app.exe", "map": "app.map", "receipt": "link.json", "log": "link.log"}[damage]
        path = context.directory / name
        path.write_bytes(path.read_bytes() + b"changed")
    elif damage == "input":
        paths = [row["path"] for row in live.inputs.records if "member" in row]
        assert paths
        from pathlib import Path
        path = Path(paths[0])
        path.write_bytes(path.read_bytes() + b"changed")
    elif damage == "parsed_image": live.inputs.image.base += 4096
    elif damage == "parsed_map": object.__setattr__(live.inputs.mapping, "base", 0)
    elif damage == "parsed_pdb": live.inputs.artifacts["msvc-pdb"] += b"changed"
    elif damage == "records": live.inputs.records.pop()
    elif damage == "candidates": live.inputs.objects._constant_records.pop()
    elif damage == "command_report":
        context.report["compile_link"]["compilations"][0]["compilation"]["exit_code"] = False
    elif damage == "expired": context.alive = False
    else: raise AssertionError(damage)


@pytest.mark.parametrize("damage", ["pdb", "image", "map", "receipt", "log", "input", "parsed_image",
                                    "parsed_map", "parsed_pdb", "records", "candidates", "command_report", "expired"])
@pytest.mark.parametrize("at_close", [False, True])
def test_live_correspondence_refuses_changed_physical_and_parsed_inputs(tmp_path, damage, at_close):
    context = fixture(tmp_path / "project")
    with pytest.raises(ValueError):
        with linked_context.linked_build(context) as live:
            assert any(row["correspondence"] == "pass" for row in live.ordinary_contributions()["sections"])
            mutate(context, live, damage)
            if not at_close:
                with pytest.raises(ValueError):
                    live.ordinary_contributions()


@pytest.mark.parametrize("damage", ["pdb", "input", "parsed_image", "records", "command_report", "expired"])
def test_native_final_guard_refuses_damage_inside_successful_composition(tmp_path, monkeypatch, damage):
    context = fixture(tmp_path / "project")
    original = linked_contributions.ordinary_contributions
    with pytest.raises(ValueError):
        with linked_context.linked_build(context) as live:
            def changed(*args, **kwargs):
                result = original(*args, **kwargs)
                mutate(context, live, damage)
                return result
            monkeypatch.setattr(linked_contributions, "ordinary_contributions", changed)
            with pytest.raises(ValueError):
                live.ordinary_contributions()


def test_native_final_guard_also_runs_after_composition_refusal(tmp_path, monkeypatch):
    context = fixture(tmp_path / "project")
    with pytest.raises(ValueError, match="expired"):
        with linked_context.linked_build(context) as live:
            def refused(*args, **kwargs):
                context.alive = False
                raise ValueError("unsupported correspondence")
            monkeypatch.setattr(linked_contributions, "ordinary_contributions", refused)
            with pytest.raises(ValueError, match="expired"):
                live.ordinary_contributions()


def test_missing_command_proof_fallback_still_detects_a_positive_storage_overlap(tmp_path):
    from test_common_storage_intersections import fixture as overlap_context
    from test_linked_modules import set_modules
    from test_ordinary_contributions import set_contributions
    from test_pdb_build_binding import add_bound_pdb
    context = overlap_context(tmp_path / "project")
    add_bound_pdb(context)
    raw = bytearray((context.directory / "app.exe").read_bytes())
    struct.pack_into("<I", raw, 0x98 + 92, 16)
    update_artifact(context, "app.exe", bytes(raw))
    set_modules(context, module(b"* Linker *", b""))
    set_contributions(context, contribution(section=2, offset=4, size=4, flags=0xC0102080))
    with linked_context.linked_build(context) as live:
        assert live.common(("_g",))["result"] == "pass"
        report = live.common_intersections(("_g",))
        assert report["ordinary_contributions"]["result"] == "unknown"
        assert "reason" in report["ordinary_contributions"]
        assert report["storage_census"]["enumeration"] == "pass"
        assert any(row["reason"].startswith("PDB ordinary contribution evidence refused") for row in report["gaps"])
        assert report["result"] == report["allocation_isolation"] == "fail"
        assert any(row["owner"] == "ordinary.obj" and row["symbol"] == "_g"
                   and row["address"] == 0x402004 and row["size"] == 4 for row in report["intersections"])
        assert report["retained_data_coverage"] == "unknown"
        assert not report["ready_for_collection"]
