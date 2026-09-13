"""Physical associated COFF and image-bound PDB FPO observations."""

import hashlib
import struct

import pytest

from match_recovery.linked_fpo import validate_fpo_source, verify_fpo
from match_recovery.pdb_fixups import bound_fixups
from match_recovery.pe_coff import Coff, PeImage
from test_msf import small_msf
from test_pdb_fixups import parts


def object_bytes(payload=None, *, code=None):
    payload = payload if payload is not None else struct.pack("<IIIHH", 0, 16, 3, 2, 0x0201)
    code = code if code is not None else b"\x90" * 15 + b"\xc3"
    code_at, fpo_at = 100, 100 + len(code)
    reloc_at = fpo_at + len(payload)
    symbol_at = reloc_at + 10
    root_def = b".text\0\0\0" + struct.pack("<IhHBB", 0, 1, 0, 3, 1)
    root_aux = struct.pack("<IHHIHB3x", len(code), 0, 0, 0, 0, 1)
    root = b"_f".ljust(8, b"\0") + struct.pack("<IhHBB", 0, 1, 32, 2, 0)
    child = b".debug$F" + struct.pack("<IhHBB", 0, 2, 0, 3, 1)
    child_aux = struct.pack("<IHHIHB3x", len(payload), 1, 0, 0, 1, 5)
    header = struct.pack("<HHIIIHH", 0x14C, 2, 0, symbol_at, 5, 0, 0)
    section = lambda name, size, at, reloc, count, flags: name.ljust(8, b"\0") + struct.pack(
        "<IIIIIIHHI", 0, 0, size, at, reloc, 0, count, 0, flags)
    return (header + section(b".text", len(code), code_at, 0, 0, 0x60501020)
            + section(b".debug$F", len(payload), fpo_at, reloc_at, 1, 0x42101048)
            + code + payload + struct.pack("<IIH", 0, 2, 7)
            + root_def + root_aux + root + child + child_aux + struct.pack("<I", 4))


def evidence(records=None):
    raw, streams = parts(())
    raw = bytearray(raw)
    struct.pack_into("<I", raw, 0x178 + 36, 0x60000020)
    streams[5] = bytes(raw[0x178:0x1A0])
    dbi = bytearray(streams[3])
    struct.pack_into("<H", dbi, 64, 6)
    streams[3] = bytes(dbi)
    default = struct.pack("<IIIHH", 0x1000, 16, 3, 2, 0x0201)
    streams.append(default if records is None else records)
    image = PeImage(bytes(raw), hashlib.sha256(raw).hexdigest())
    return image, bound_fixups(image, small_msf(streams))


def check(raw=None, records=None):
    obj = Coff(object_bytes() if raw is None else raw)
    image, fixups = evidence(records)
    return verify_fpo(obj, 2, obj.function("_f"), image, 0x401000, fixups)


def test_nonzero_frame_data_matches_complete_image_bound_metadata():
    proof = check()
    assert proof == {"format": "fpo_data", "data": struct.pack("<IIHH", 16, 3, 2, 0x0201).hex(), "code_size": 16}
    assert not any(key in proof for key in ("discarded", "runtime", "placement", "owner"))


def test_alignment_padding_is_not_misrepresented_as_function_code():
    payload = struct.pack("<IIIHH", 0, 14, 3, 2, 0x0201)
    raw = object_bytes(payload, code=b"\x90" * 13 + b"\xc3\xcc\xcc")
    assert check(raw, struct.pack("<I", 0x1000) + payload[4:])["code_size"] == 14


@pytest.mark.parametrize("records", [b"", bytes(16),
    struct.pack("<IIIHH", 0x1000, 16, 4, 2, 0x0201),
    struct.pack("<IIIHH", 0x1000, 16, 3, 3, 0x0201),
    struct.pack("<IIIHH", 0x1000, 16, 3, 2, 0x0202)])
def test_missing_wrong_root_or_any_changed_frame_field_refuses(records):
    with pytest.raises(ValueError, match="metadata"):
        check(records=records)


@pytest.mark.parametrize("different", [False, True])
def test_multiple_records_for_same_root_are_not_deduplicated(different):
    first = struct.pack("<IIIHH", 0x1000, 16, 3, 2, 0x0201)
    second = struct.pack("<IIIHH", 0x1000, 16, 4 if different else 3, 2, 0x0201)
    image, fixups = evidence(first + second)
    assert len(fixups.fpo_records) == 2
    obj = Coff(object_bytes())
    with pytest.raises(ValueError, match="ambiguous"):
        verify_fpo(obj, 2, obj.function("_f"), image, 0x401000, fixups)


@pytest.mark.parametrize("size", [1, 15, 17, 31])
def test_partial_pdb_fpo_records_refuse(size):
    with pytest.raises(ValueError, match="FPO stream extent"):
        evidence(bytes(size))


@pytest.mark.parametrize("field,value", [(0, 1), (1, 0), (1, 17), (4, 0x2001), (4, 0x0211)])
def test_root_addend_reserved_frame_and_extent_are_not_accepted_even_when_pdb_matches(field, value):
    fields = [0, 16, 3, 2, 0x0201]
    fields[field] = value
    payload = struct.pack("<IIIHH", *fields)
    with pytest.raises(ValueError, match="frame extent"):
        check(object_bytes(payload), struct.pack("<I", 0x1000) + payload[4:])


@pytest.mark.parametrize("tail", [b"\0\0", b"\xc3\x90", b"\x90" * 18])
def test_nonpadding_or_overlong_root_tail_is_not_hidden_by_fpo_size(tail):
    payload = struct.pack("<IIIHH", 0, 14, 3, 2, 0x0201)
    with pytest.raises(ValueError, match="alignment"):
        check(object_bytes(payload, code=b"\x90" * 14 + tail), struct.pack("<I", 0x1000) + payload[4:])


@pytest.mark.parametrize("offset,fmt,value", [(60, "8s", b".debug$S"), (96, "I", 0x42101040),
    (132, "I", 4), (136, "I", 0), (140, "H", 6), (142 + 3 * 18 + 12, "h", 1),
    (142 + 3 * 18 + 16, "B", 2), (142 + 4 * 18 + 6, "H", 1)])
def test_physical_coff_child_identity_reference_and_metadata_damage_refuse(offset, fmt, value):
    raw = bytearray(object_bytes())
    struct.pack_into("<" + fmt, raw, offset, value)
    with pytest.raises(ValueError):
        check(bytes(raw))


def test_another_image_cannot_reuse_bound_fpo_records():
    image, fixups = evidence()
    changed = bytearray(image.data)
    changed[0x200] ^= 1
    other = PeImage(bytes(changed), hashlib.sha256(changed).hexdigest())
    obj = Coff(object_bytes())
    with pytest.raises(ValueError, match="image binding"):
        verify_fpo(obj, 2, obj.function("_f"), other, 0x401000, fixups)


@pytest.mark.parametrize("weak", [False, True])
def test_public_and_weak_aliases_cannot_hide_debug_metadata_identity(weak):
    raw = bytearray(object_bytes())
    auxiliary = struct.pack("<II10x", 3, 3) if weak else b""
    alias = b"_alias\0\0" + struct.pack("<IhHBB", 0, 0 if weak else 2, 0, 105 if weak else 2, int(weak))
    raw[-4:] = alias + auxiliary + raw[-4:]
    struct.pack_into("<I", raw, 12, 7 if weak else 6)
    with pytest.raises(ValueError, match="alias"):
        check(bytes(raw))


def test_loaded_root_reference_to_child_metadata_is_not_ignored():
    raw = bytearray(object_bytes())
    struct.pack_into("<I", raw, 20 + 24, len(raw))
    struct.pack_into("<H", raw, 20 + 32, 1)
    struct.pack_into("<H", raw, 142 + 18 + 4, 1)
    raw.extend(struct.pack("<IIH", 0, 3, 6))
    with pytest.raises(ValueError, match="incoming"):
        check(bytes(raw))


def test_fpo_collection_and_records_are_immutable():
    image, fixups = evidence()
    assert isinstance(fixups.fpo_records, tuple)
    assert isinstance(fixups.fpo_records[0], bytes)
    with pytest.raises(TypeError):
        fixups.fpo_records[0][0] = 1


def test_two_coff_fpo_children_cannot_consume_the_same_linked_record():
    original = bytearray(object_bytes())
    sections = bytearray(original[20:100])
    for start in (0, 40):
        for field in (20, 24):
            pointer, = struct.unpack_from("<I", sections, start + field)
            if pointer:
                struct.pack_into("<I", sections, start + field, pointer + 40)
    third = sections[40:80]
    header = bytearray(original[:20])
    struct.pack_into("<H", header, 2, 3)
    struct.pack_into("<II", header, 8, 182, 7)
    extra = b".debug$F" + struct.pack("<IhHBB", 0, 3, 0, 3, 1) + struct.pack("<IHHIHB3x", 16, 1, 0, 0, 1, 5)
    raw = bytes(header + sections + third + original[100:-4] + extra + original[-4:])
    with pytest.raises(ValueError, match="unsupported associated"):
        check(raw)


@pytest.mark.parametrize("records", [b"", struct.pack("<IIIHH", 0x1000, 16, 4, 2, 0x0201),
    struct.pack("<IIIHH", 0x1000, 16, 3, 2, 0x0201) * 2])
def test_source_validation_does_not_claim_missing_different_or_duplicate_pdb_correspondence(records):
    obj = Coff(object_bytes())
    image, fixups = evidence(records)
    root = obj.function("_f")
    source = validate_fpo_source(obj, 2, root, image, 0x401000, fixups)
    assert source == {"format": "fpo_data", "data": struct.pack("<IIHH", 16, 3, 2, 0x0201).hex(), "code_size": 16}
    assert set(source) == {"format", "data", "code_size"}
    assert fixups.fpo_records == tuple(records[at:at + 16] for at in range(0, len(records), 16))
    with pytest.raises(ValueError, match="missing, ambiguous or different"):
        verify_fpo(obj, 2, root, image, 0x401000, fixups)


def test_source_and_unique_pdb_validation_return_the_same_canonical_frame():
    obj = Coff(object_bytes())
    image, fixups = evidence()
    root = obj.function("_f")
    assert validate_fpo_source(obj, 2, root, image, 0x401000, fixups) == verify_fpo(obj, 2, root, image, 0x401000, fixups)


@pytest.mark.parametrize("address", [True, 0x3FFFFF, 0x4011F1, 0xFFFFFFFF])
def test_extracted_source_validator_keeps_linked_extent_bounds(address):
    obj = Coff(object_bytes())
    image, fixups = evidence()
    with pytest.raises(ValueError):
        validate_fpo_source(obj, 2, obj.function("_f"), image, address, fixups)


@pytest.mark.parametrize("damage", ["name", "relocation", "reserved_frame", "alignment", "image"])
def test_extracted_source_validator_keeps_structural_and_image_obligations(damage):
    raw = bytearray(object_bytes())
    image, fixups = evidence()
    if damage == "name":
        raw[60:68] = b".debug$S"
        raw[142 + 3 * 18:142 + 3 * 18 + 8] = b".debug$S"
    elif damage == "relocation":
        struct.pack_into("<H", raw, 140, 6)
    elif damage == "reserved_frame":
        struct.pack_into("<H", raw, 116 + 14, 0x2201)
    elif damage == "alignment":
        struct.pack_into("<I", raw, 116 + 4, 14)
        raw[115] = 0
    else:
        changed = bytearray(image.data)
        changed[0x200] ^= 1
        image = PeImage(bytes(changed), hashlib.sha256(changed).hexdigest())
    obj = Coff(bytes(raw))
    with pytest.raises(ValueError):
        validate_fpo_source(obj, 2, obj.function("_f"), image, 0x401000, fixups)
