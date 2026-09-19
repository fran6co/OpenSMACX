"""Physical PE/PDB generated-directory controls, never saved proof inputs."""

from dataclasses import asdict
from hashlib import sha256
import struct

import pytest

from match_recovery.linker_generated_storage import assign_linker_contributions
from match_recovery.pdb_contributions import bound_contributions
from match_recovery.pe_coff import PeImage
from test_msf import small_msf
from test_pdb_contributions import contribution, module, parts
from test_pdb_fixups import DIRECTORY, DEBUG, PAYLOAD


def fixture(*, size=56, offset=0x100, flags=0x40000040, name=b"* Linker *",
            object_name=b"", extra=b"", fixups=b""):
    record = contribution(offset=offset, size=size, flags=flags)
    raw, streams = parts(module(name, object_name), record + extra)
    raw = bytearray(raw)
    struct.pack_into("<II", raw, 0x98 + 32, 0x1000, 0x200)
    struct.pack_into("<I", raw, 0x98 + 56, 0x2000)
    struct.pack_into("<I", raw, 0x178 + 36, flags)
    streams[4] = fixups
    streams[5] = bytes(raw[0x178:0x1A0])
    return raw, streams


def observe(raw, streams, *, rows=None, modules=None, pdb=True):
    image = PeImage(bytes(raw), sha256(raw).hexdigest())
    raw_pdb = small_msf(streams)
    # Supplied observations deliberately do not come from the damaged parser:
    # the implementation must re-establish physical correspondence itself.
    rows = rows if rows is not None else [{"index": 0, "section": 1, "offset": 0x100,
        "size": 56, "flags": 0x40000040, "module": 0, "data_crc": 0, "relocation_crc": 0,
        "address": 0x401100}]
    modules = modules if modules is not None else [{"index": 0, "name": "* Linker *",
        "kind": "linker_generated", "object_name": ""}]
    return assign_linker_contributions(rows, modules, image, pdb=raw_pdb if pdb else None)


def test_physical_directory_and_retained_suffix_are_assigned_without_projection():
    raw, streams = fixture()
    original = bytes(raw)
    report = observe(raw, streams)
    assert report["assigned"] == 1 and report["result"] == "pass", report
    record, = report["records"]
    assert record["assignment"]["kind"] == "nb10_directory_with_retained_zero_suffix"
    proof = record["proof"]
    assert proof["size"] == 56 and proof["directory_size"] == 28
    assert proof["retained_zero_suffix_size"] == 28
    assert proof["sha256"] == sha256(raw[DEBUG:DEBUG + 56]).hexdigest()
    assert proof["image_sha256"] == sha256(raw).hexdigest()
    assert proof["pdb_sha256"] == sha256(small_msf(streams)).hexdigest()
    assert not proof["byte_projection"] and not report["ready_for_collection"]
    assert bytes(raw) == original


def test_exact_single_directory_without_suffix_has_its_own_supported_shape():
    raw, streams = fixture(size=28)
    image = PeImage(bytes(raw), sha256(raw).hexdigest())
    inventory = bound_contributions(image, small_msf(streams))
    row = {"index": 0, **asdict(inventory.contributions[0]), "address": 0x401100}
    report = observe(raw, streams, rows=[row])
    assert report["assigned"] == 1, report
    assert report["records"][0]["assignment"]["kind"] == "nb10_debug_directory"
    assert report["records"][0]["proof"]["retained_zero_suffix_size"] == 0


@pytest.mark.parametrize("size", [28, 56])
def test_reserved_native_pdb_flag_is_not_masked_to_close_assignment(size):
    raw, streams = fixture(size=size, flags=0x40002040)
    # Match the observed game asymmetry: PDB retains the unexplained flag,
    # while its independently bound PE section is ordinary readonly data.
    struct.pack_into("<I", raw, 0x178 + 36, 0x40000040)
    streams[5] = bytes(raw[0x178:0x1A0])
    inventory = bound_contributions(PeImage(bytes(raw), sha256(raw).hexdigest()), small_msf(streams))
    row = {"index": 0, **asdict(inventory.contributions[0]), "address": 0x401100}
    report = observe(raw, streams, rows=[row])
    assert report["assigned"] == 0 and report["result"] == "unknown", report


@pytest.mark.parametrize("options", [{"size": 27}, {"size": 29}, {"size": 55}, {"size": 57}, {"size": 84},
    {"offset": 0x101}, {"flags": 0xC0000040}, {"flags": 0x60000020},
    {"flags": 0x40000080}, {"name": b"app.obj"}, {"object_name": b"app.obj"}])
def test_altered_physical_shape_or_module_is_not_inferred_from_the_supplied_row(options):
    raw, streams = fixture(**options)
    assert observe(raw, streams)["assigned"] == 0


@pytest.mark.parametrize("offset", [0, 8, 27])
def test_nonzero_suffix_stays_unmatched(offset):
    raw, streams = fixture()
    raw[DEBUG + 28 + offset] = 1
    assert observe(raw, streams)["assigned"] == 0


@pytest.mark.parametrize("damage", ["no_pdb", "pdb_signature", "pdb_age", "nb10_signature", "nb10_magic",
    "directory_extent", "directory_start", "payload_kind", "payload_major", "payload_minor",
    "payload_pointer", "payload_rva", "other_directory", "certificate_alias", "header_geometry",
    "loader_fixup", "loader_crossing_suffix", "parsed_image_geometry"])
def test_debug_identification_framing_and_full_storage_are_required(damage):
    raw, streams = fixture()
    if damage in ("pdb_signature", "pdb_age"):
        info = bytearray(streams[1]); info[4 if damage == "pdb_signature" else 8] ^= 1
        streams[1] = bytes(info)
    elif damage in ("nb10_signature", "nb10_magic"):
        raw[PAYLOAD + (8 if damage == "nb10_signature" else 0)] ^= 1
    elif damage == "directory_extent": struct.pack_into("<I", raw, DIRECTORY + 4, 56)
    elif damage == "directory_start": struct.pack_into("<I", raw, DIRECTORY, 0x1101)
    elif damage == "payload_kind": struct.pack_into("<I", raw, DEBUG + 12, 1)
    elif damage == "payload_major": struct.pack_into("<H", raw, DEBUG + 8, 1)
    elif damage == "payload_minor": struct.pack_into("<H", raw, DEBUG + 10, 1)
    elif damage == "payload_pointer": struct.pack_into("<I", raw, DEBUG + 24, 0x320)
    elif damage == "payload_rva": struct.pack_into("<I", raw, DEBUG + 20, 0x1120)
    elif damage == "other_directory": struct.pack_into("<II", raw, 0x98 + 96, 0x112C, 4)
    elif damage == "certificate_alias": struct.pack_into("<II", raw, 0x98 + 96 + 4 * 8, DEBUG + 28, 4)
    elif damage == "header_geometry": struct.pack_into("<I", raw, 0x98 + 36, 3)
    elif damage.startswith("loader_"):
        struct.pack_into("<II", raw, 0x98 + 96 + 5 * 8, 0x1180, 12)
        site = 0x12C if damage == "loader_fixup" else 0x12A
        struct.pack_into("<IIHH", raw, 0x380, 0x1000, 12, 0x3000 + site, 0)
    elif damage == "parsed_image_geometry":
        image = PeImage(bytes(raw), sha256(raw).hexdigest())
        image.section_flags = (0x60000020,)
        with pytest.raises(ValueError, match="parsed PE"):
            from match_recovery.linker_debug_storage import verify_debug_storage
            verify_debug_storage(image, small_msf(streams), {"size": 56, "flags": 0x40000040})
        return
    report = observe(raw, streams, pdb=damage != "no_pdb")
    assert report["assigned"] == 0 and report["records"][0]["result"] == "unknown", report


@pytest.mark.parametrize("site", [0x1100, 0x111B, 0x111C, 0x1120, 0x1137])
def test_pdb_write_including_crossing_the_suffix_cannot_be_assigned(site):
    raw, streams = fixture(fixups=struct.pack("<HHII", 6, 0, site, 0x1010))
    assert observe(raw, streams)["assigned"] == 0


@pytest.mark.parametrize("offset", [0x100, 0x110, 0x137])
def test_omitting_an_overlapping_native_contribution_cannot_close_the_domain(offset):
    raw, streams = fixture(extra=contribution(offset=offset, size=1, flags=0x40000040))
    assert observe(raw, streams)["assigned"] == 0


def test_supplied_fields_cannot_replace_native_contribution_coordinates():
    raw, streams = fixture()
    inventory = bound_contributions(PeImage(bytes(raw), sha256(raw).hexdigest()), small_msf(streams))
    row = {"index": 0, **asdict(inventory.contributions[0]), "address": 0x401100}
    for field in ("index", "address", "section", "offset", "size", "module", "data_crc", "relocation_crc"):
        assert observe(raw, streams, rows=[{**row, field: row[field] + 1}])["assigned"] == 0


def test_suffix_remains_part_of_whole_image_residual_comparison():
    from match_recovery.pe_output_residual import compare_same_layout_pe
    raw, streams = fixture()
    assert observe(raw, streams)["assigned"] == 1
    baseline = PeImage(bytes(raw), sha256(raw).hexdigest())
    raw[DEBUG + 28] = 1
    changed = PeImage(bytes(raw), sha256(raw).hexdigest())
    report = compare_same_layout_pe(baseline, changed, [], [])
    assert report["outside_bytes"] == "fail" and report["different_file_bytes"] == 1, report
    assert report["result"] == "unknown" and not report["ready_for_collection"]
