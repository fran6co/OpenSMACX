"""Physical PE/NB10 + Small MSF + DBI/XFIXUP binding damage controls."""

from dataclasses import FrozenInstanceError
import hashlib
import struct

import pytest

from match_recovery.pe_coff import PeImage
from match_recovery.pdb_fixups import BoundFixups, Fixup, bound_fixups
from test_binary import pe_fixture
from test_msf import small_msf


DIRECTORY = 0x98 + 96 + 6 * 8
DEBUG, PAYLOAD = 0x300, 0x400
SIGNATURE = 0x12345678


def parts(fixups=((6, 0, 0x1000, 0x1010),), *, mapped=False):
    """Return physical PE bytes and indexed streams for targeted corruption."""
    raw = bytearray(pe_fixture())
    payload = struct.pack("<4sIII", b"NB10", 0, SIGNATURE, 1) + b"Z:\\build\\app.pdb\0"
    raw.extend(payload)
    struct.pack_into("<I", raw, 0x98 + 60, 0x200)
    struct.pack_into("<II", raw, DIRECTORY, 0x1100, 28)
    struct.pack_into("<IIHHIIII", raw, DEBUG, 0, 0, 0, 0, 2, len(payload), 0x1200 if mapped else 0, PAYLOAD)
    if mapped:
        struct.pack_into("<I", raw, 0x178 + 8, len(raw) - 0x200)
        struct.pack_into("<I", raw, 0x178 + 16, len(raw) - 0x200)
    dbi = bytearray(64)
    struct.pack_into("<III", dbi, 0, 0xFFFFFFFF, 19970606, 1)
    for offset in (12, 16, 20):
        struct.pack_into("<H", dbi, offset, 0xFFFF)
    struct.pack_into("<I", dbi, 48, 12)
    dbi.extend(struct.pack("<6H", 0xFFFF, 0xFFFF, 4, 0xFFFF, 0xFFFF, 5))
    streams = [None, struct.pack("<III", 19970604, SIGNATURE, 1), None, bytes(dbi),
               b"".join(struct.pack("<HHII", *row) for row in fixups), bytes(raw[0x178:0x1A0])]
    return bytes(raw), streams


def bind(raw, streams):
    image = PeImage(raw, hashlib.sha256(raw).hexdigest())
    return bound_fixups(image, small_msf(streams))


def pdb_pair(fixups=((6, 0, 0x1000, 0x1010),), *, mapped=False):
    raw, streams = parts(fixups, mapped=mapped)
    return PeImage(raw, hashlib.sha256(raw).hexdigest()), small_msf(streams)


@pytest.mark.parametrize("mapped", [False, True])
@pytest.mark.parametrize("kind", [6, 7, 20])
def test_physical_pair_preserves_fixup_type_and_target_without_operand_guessing(mapped, kind):
    image, pdb = pdb_pair(((kind, 0, 0x1000, 0x1010),), mapped=mapped)
    result = bound_fixups(image, pdb)
    assert isinstance(result, BoundFixups)
    assert dict(result) == {0x1000: Fixup(kind, 0x1010)}
    assert result.image_sha256 == hashlib.sha256(image.data).hexdigest()
    assert result.pdb_sha256 == hashlib.sha256(pdb).hexdigest()
    with pytest.raises(TypeError):
        result[0x1000] = Fixup(6, 0x1020)
    with pytest.raises(FrozenInstanceError):
        result[0x1000].target_rva = 0x1020
    with pytest.raises(FrozenInstanceError):
        result.image_sha256 = "0" * 64


def test_present_empty_fixup_stream_is_distinct_from_missing():
    raw, streams = parts(())
    assert not bind(raw, streams)
    streams[4] = None
    with pytest.raises(ValueError):
        bind(raw, streams)


@pytest.mark.parametrize("index", [1, 3, 4, 5])
def test_required_streams_cannot_be_absent(index):
    raw, streams = parts()
    streams[index] = None
    with pytest.raises(ValueError):
        bind(raw, streams)


@pytest.mark.parametrize("offset,value", [(0, 0), (4, 0), (8, 2)])
def test_info_version_signature_and_age_are_bound(offset, value):
    raw, streams = parts()
    info = bytearray(streams[1])
    struct.pack_into("<I", info, offset, value)
    streams[1] = bytes(info)
    with pytest.raises(ValueError, match="signature/age"):
        bind(raw, streams)


@pytest.mark.parametrize("offset,value", [(0, 0), (4, 19990903), (8, 2), (24, 0xFFFFFFFF),
    (28, 1), (32, 1), (36, 1), (40, 1), (48, 0), (48, 11), (48, 13), (48, 24),
    (52, 1), (56, 1), (58, 0x8664), (60, 1)])
def test_dbi_version_lengths_flags_and_machine_damage_refuse(offset, value):
    raw, streams = parts()
    dbi = bytearray(streams[3])
    struct.pack_into("<H" if offset in (56, 58) else "<I", dbi, offset, value)
    streams[3] = bytes(dbi)
    with pytest.raises(ValueError):
        bind(raw, streams)


def test_dbi_explicit_i386_machine_and_nonempty_substreams_are_supported():
    raw, streams = parts()
    dbi = bytearray(streams[3][:64])
    struct.pack_into("<H", dbi, 58, 0x14C)
    # Only substream extents participate here; no module ownership is inferred.
    struct.pack_into("<I", dbi, 24, 4)
    struct.pack_into("<I", dbi, 52, 3)
    streams[3] = bytes(dbi) + bytes(7) + streams[3][64:]
    assert bind(raw, streams)[0x1000] == Fixup(6, 0x1010)


@pytest.mark.parametrize("slot,index", [(2, 0xFFFF), (5, 0xFFFF), (2, 3), (2, 7), (0, 4), (3, 6), (4, 6)])
def test_optional_stream_missing_alias_out_of_range_or_remapping_refuses(slot, index):
    raw, streams = parts()
    dbi = bytearray(streams[3])
    struct.pack_into("<H", dbi, 64 + slot * 2, index)
    streams[3] = bytes(dbi)
    streams.append(b"")
    with pytest.raises(ValueError):
        bind(raw, streams)


@pytest.mark.parametrize("index", [3, 4, 7])
def test_symbol_stream_indices_cannot_alias_bound_metadata_or_be_missing(index):
    raw, streams = parts()
    dbi = bytearray(streams[3])
    struct.pack_into("<H", dbi, 12, index)
    streams[3] = bytes(dbi)
    with pytest.raises(ValueError):
        bind(raw, streams)


@pytest.mark.parametrize("offset", [0, 8, 12, 16, 20, 36])
def test_every_pe_section_header_field_remains_bound(offset):
    raw, streams = parts()
    sections = bytearray(streams[5])
    sections[offset] ^= 1
    streams[5] = bytes(sections)
    with pytest.raises(ValueError, match="section headers"):
        bind(raw, streams)


@pytest.mark.parametrize("row", [(1, 0, 0x1000, 0x1010), (6, 1, 0x1000, 0x1010),
    (6, 0, 0, 0x1010), (6, 0, 0x11FD, 0x1010), (6, 0, 0xFFFFFFFF, 0x1010),
    (6, 0, 0x1000, 0), (6, 0, 0x1000, 0x1200), (6, 0, 0x1000, 0xFFFFFFFF)])
def test_fixup_types_flags_and_site_target_bounds_refuse(row):
    with pytest.raises(ValueError):
        bind(*parts((row,)))


@pytest.mark.parametrize("delta", [0, 1, 2, 3, -1])
def test_duplicate_and_overlapping_sites_cannot_supply_multiple_evidence(delta):
    with pytest.raises(ValueError, match="duplicate|overlapping"):
        bind(*parts(((6, 0, 0x1010, 0x1080), (20, 0, 0x1010 + delta, 0x1090))))


def test_partial_fixup_record_and_dbi_trailing_bytes_refuse():
    for index in (3, 4):
        raw, streams = parts()
        streams[index] += b"\0"
        with pytest.raises(ValueError):
            bind(raw, streams)


@pytest.mark.parametrize("offset,value", [(DIRECTORY, 0), (DIRECTORY + 4, 27), (DEBUG, 1),
    (DEBUG + 12, 1), (DEBUG + 16, 15), (DEBUG + 16, 0xFFFFFFFF), (DEBUG + 20, 0x1000),
    (DEBUG + 24, 0), (DEBUG + 24, 0xFFFFFFFF), (PAYLOAD, 0), (PAYLOAD + 4, 1),
    (PAYLOAD + 8, 0), (PAYLOAD + 12, 2), (0x98 + 60, 0)])
def test_nb10_directory_identity_and_bounds_damage_refuse(offset, value):
    raw, streams = parts()
    raw = bytearray(raw)
    struct.pack_into("<I", raw, offset, value)
    with pytest.raises(ValueError):
        bind(bytes(raw), streams)


@pytest.mark.parametrize("damage", ["unterminated", "interior_nul", "control", "duplicate", "header", "mapped_overlay"])
def test_nb10_paths_and_physical_aliases_are_not_accepted(damage):
    raw, streams = parts(mapped=damage == "mapped_overlay")
    raw = bytearray(raw)
    if damage == "unterminated":
        raw[-1] = 65
    elif damage == "interior_nul":
        raw[PAYLOAD + 18] = 0
    elif damage == "control":
        raw[PAYLOAD + 18] = 1
    elif damage == "duplicate":
        payload = bytes(raw[PAYLOAD:])
        offset = len(raw)
        raw.extend(payload)
        struct.pack_into("<I", raw, DIRECTORY + 4, 56)
        struct.pack_into("<IIHHIIII", raw, DEBUG + 28, 0, 0, 0, 0, 2, len(payload), 0, offset)
    elif damage == "header":
        payload = bytes(raw[PAYLOAD:])
        raw[0x40:0x40 + len(payload)] = payload
        struct.pack_into("<I", raw, DEBUG + 24, 0x40)
    else:
        struct.pack_into("<I", raw, DEBUG + 20, 0)
    with pytest.raises(ValueError):
        bind(bytes(raw), streams)


@pytest.mark.parametrize("rva", [0x1100, 0x1200])
def test_loader_fixups_cannot_rewrite_nb10_binding_metadata(rva):
    raw, streams = parts(mapped=True)
    raw = bytearray(raw)
    struct.pack_into("<II", raw, 0x98 + 96 + 5 * 8, 0x1120, 12)
    struct.pack_into("<IIHH", raw, 0x320, 0x1000, 12, 0x3000 + rva - 0x1000, 0)
    with pytest.raises(ValueError, match="loader fixups"):
        bind(bytes(raw), streams)


def test_original_section_address_remapping_stream_is_not_silently_ignored():
    raw, streams = parts()
    dbi = bytearray(streams[3])
    struct.pack_into("<I", dbi, 48, 22)
    dbi.extend(struct.pack("<5H", 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 6))
    streams[3] = bytes(dbi)
    streams.append(b"")
    with pytest.raises(ValueError, match="remapping"):
        bind(raw, streams)


@pytest.mark.parametrize("index,size", [(1, 11), (3, 63), (5, 39)])
def test_truncated_fixed_identity_or_section_header_streams_refuse(index, size):
    raw, streams = parts()
    streams[index] = streams[index][:size]
    with pytest.raises(ValueError):
        bind(raw, streams)


def test_duplicate_symbol_stream_indices_refuse():
    raw, streams = parts()
    dbi = bytearray(streams[3])
    struct.pack_into("<H", dbi, 12, 6)
    struct.pack_into("<H", dbi, 16, 6)
    streams[3] = bytes(dbi)
    streams.append(b"")
    with pytest.raises(ValueError, match="aliased DBI symbol"):
        bind(raw, streams)


def test_mutable_image_input_is_not_digest_bound_once_and_then_reused():
    raw, streams = parts()
    image = PeImage(bytearray(raw), hashlib.sha256(raw).hexdigest())
    with pytest.raises(ValueError, match="immutable image"):
        bound_fixups(image, small_msf(streams))
