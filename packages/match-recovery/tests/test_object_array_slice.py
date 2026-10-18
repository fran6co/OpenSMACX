"""Bounded v5 root-array slices: native metadata, identity and refusal controls."""
from dataclasses import replace
import hashlib
import struct

import pytest

from match_recovery.codeview_data import member_data_layout
from match_recovery.model import Origin, Span
from match_recovery.object_data import check_initial_data
from match_recovery.object_source import ObjectBinding, object_targets, parse_objects
from match_recovery.object_writer import replace_object_records, write_object_bytes
from match_recovery.pe_coff import Coff, PeImage
from test_codeview import record
from test_object_data import data_fixture, data_image

PIN, UNIT, PARENT = "a" * 64, "code/colors.cc", "_colors"


def array_object(value=None, *, size=64, element=0x74, subscript=0x75,
                 root_pointer=False, relocations=()):
    array = record(0x1003, struct.pack("<IIH", element, subscript, size) + b"\x06colors")
    if root_pointer:
        array = record(0x1002, struct.pack("<II", 0x74, 0xA))
    return Coff(data_fixture(bytes(size) if value is None else value,
                             type_index=0x1000, types=array, definition=PARENT.encode(),
                             relocations=relocations))


def child(*, pin=PIN, start=0x401008, size=8, path="[2:4]"):
    origin = Origin(pin, "pe:va", start)
    return ObjectBinding(origin, "_old", UNIT, 1, Span(start, start + size), origin,
                         parent_symbol=PARENT, member_path=(path,))


def parse(text):
    return parse_objects(text, UNIT, PIN, address_space="pe:va")


def annotation(path="[2:4]", version=5):
    return f"// RECOVERY_OBJECT: v{version} 0x00401008 0x00401010 _old 0x00401008 member _colors {path}\n"


def test_v5_writer_roundtrip_and_guarded_replacement_preserve_original_obligation():
    item, = parse(annotation())
    assert item == child()
    source = b"int colors[16];\n"
    written = write_object_bytes(source, expected=(), additions=(item,), path=UNIT,
                                 pin=PIN, address_space="pe:va")
    assert written == annotation().encode() + source
    old_source = b"// RECOVERY_OBJECT: v2 0x00401008 0x00401010 _old 0x00401008\nint old[2];\n"
    old, = parse(old_source.decode())
    output = replace_object_records({UNIT: old_source}, expected=(old,), replacements=(item,),
                                    destination=UNIT, pin=PIN, address_space="pe:va")
    migrated, = parse(output[UNIT].decode())
    assert migrated.interpretation_id == old.interpretation_id
    assert (migrated.symbol, migrated.origin, migrated.span, migrated.evidence) == (
        old.symbol, old.origin, old.span, old.evidence)
    assert output[UNIT].endswith(b"int old[2];\n")
    with pytest.raises(ValueError, match="historical identity"):
        replace_object_records({UNIT: old_source}, expected=(old,),
                               replacements=(replace(item, span=Span(0x401008, 0x401014)),),
                               destination=UNIT, pin=PIN, address_space="pe:va")


@pytest.mark.parametrize("path", ["[02:4]", "[2:04]", "[-1:4]", "[+2:4]", "[2:2]",
                                  "[4:2]", "[:4]", "[2:]", "[2:4].x", "x[2:4]",
                                  "[0].[2:4]", "[2:4][0]", "[2:4:1]", "[2: 4]", "[2]"])
def test_v5_refuses_noncanonical_empty_or_nonsole_slice(path):
    with pytest.raises(ValueError):
        parse(annotation(path))


def test_v3_grammar_and_unbounded_slice_refusal_remain_explicit():
    for version in (3,):
        with pytest.raises(ValueError, match="version 5"):
            parse(annotation(version=version))
    ordinary, = parse(annotation("[2]", version=3))
    assert ordinary.member_path == ("[2]",)
    with pytest.raises(ValueError):
        parse("// RECOVERY_OBJECT: v5 0x00401008 _old member _colors [2:4]\n")
    with pytest.raises(ValueError, match="bounds and evidence"):
        ObjectBinding(Origin(PIN, "pe:va", 0x401008), "_old", UNIT, 1,
                      parent_symbol=PARENT, member_path=("[2:4]",))


def test_native_array_metadata_defines_offset_extent_and_explicit_range_not_scalar_type():
    observed = member_data_layout(array_object(), PARENT, ("[2:4]",))
    assert (observed["member_offset"], observed["size"], observed["parent_size"]) == (8, 8, 64)
    assert observed["selection_kind"] == "array_slice" and observed["type_index"] is None
    assert observed["member_path"] == [{"begin": 2, "end": 4, "offset": 8, "size": 8,
        "array_size": 64, "element_size": 4, "element_count": 16,
        "element_type_index": 0x74, "array_type_index": 0x1000}]
    narrow = member_data_layout(array_object(element=0x70), PARENT, ("[2:4]",))
    assert (narrow["member_offset"], narrow["size"]) == (2, 2)
    tail = member_data_layout(array_object(), PARENT, ("[15:16]",))
    assert (tail["member_offset"], tail["size"]) == (60, 4)


def test_195_word_parent_slice_retains_32_byte_extent_and_parent_address():
    obj = array_object(size=195 * 4)
    item = child(start=0x401058, size=32, path="[22:30]")
    observed = member_data_layout(obj, PARENT, item.member_path)
    assert (observed["member_offset"], observed["size"], observed["parent_size"]) == (88, 32, 780)
    targets, reports = object_targets((item,), {UNIT: obj})
    assert targets == {PARENT: 0x401000} and reports[0]["definition"] == "pass"


@pytest.mark.parametrize("kwargs,path", [({}, "[15:17]"), ({"size": 63}, "[2:4]"),
    ({"root_pointer": True}, "[2:4]"), ({"subscript": 0x403}, "[2:4]"),
    ({"subscript": 0x40}, "[2:4]"), ({"element": 0x1009}, "[2:4]")])
def test_invalid_or_unsupported_fresh_array_evidence_refuses(kwargs, path):
    with pytest.raises(ValueError):
        member_data_layout(array_object(**kwargs), PARENT, (path,))


def test_pointer_elements_are_storage_only_and_slice_cannot_be_followed():
    obj = array_object(element=0x403)
    assert member_data_layout(obj, PARENT, ("[2:4]",))["size"] == 8
    with pytest.raises(ValueError):
        member_data_layout(obj, PARENT, ("[2:4]", "member"))


def test_targets_resolve_parent_base_and_refuse_inconsistent_or_missing_definitions():
    obj, item = array_object(), child()
    parent = ObjectBinding(Origin(PIN, "pe:va", 0x401000), PARENT, UNIT, 2)
    targets, reports = object_targets((parent, item), {UNIT: obj})
    assert targets == {PARENT: 0x401000} and all(r["definition"] == "pass" for r in reports)
    targets, reports = object_targets((replace(parent, origin=Origin(PIN, "pe:va", 0x402000)), item), {UNIT: obj})
    assert not targets and all(r["definition"] == "unknown" for r in reports)
    obj.sections = tuple(section for section in obj.sections if section.name != ".debug$T")
    targets, reports = object_targets((parent, item), {UNIT: obj})
    assert not targets and all(r["definition"] == "unknown" for r in reports)


def test_initial_data_uses_exact_slice_and_preserves_boundary_and_byte_failures():
    original = bytes(range(64))
    raw = data_image(original)
    pin = hashlib.sha256(raw).hexdigest()
    image, item = PeImage(raw, pin), child(pin=pin)
    value = bytes(8) + original[8:16] + bytes(48)
    obj = array_object(value)
    row, = check_initial_data((item,), {UNIT: obj}, image, {PARENT: 0x401000})
    assert row["content"] == "pass"
    damaged = array_object(bytes(64))
    row, = check_initial_data((item,), {UNIT: damaged}, image, {PARENT: 0x401000})
    assert row["content"] == "fail"
    row, = check_initial_data((child(pin=pin, size=4),), {UNIT: obj}, image, {PARENT: 0x401000})
    assert row["proof"]["boundary"] == "fail" and row["proof"]["candidate_size"] == 8


def test_slice_relocations_rebase_and_refuse_both_edges_or_wrong_targets():
    original = bytes(8) + struct.pack("<I", 0x402004) + bytes(52)
    raw = data_image(original, absolute=8)
    pin = hashlib.sha256(raw).hexdigest()
    image, item = PeImage(raw, pin), child(pin=pin)
    obj = array_object(bytes(8) + struct.pack("<I", 4) + bytes(52), relocations=((8, 1, 6),))
    targets = {PARENT: 0x401000, "_h": 0x402000}
    row, = check_initial_data((item,), {UNIT: obj}, image, targets)
    assert row["content"] == "pass" and row["proof"]["relocations"][0]["offset"] == 0
    row, = check_initial_data((item,), {UNIT: obj}, image, dict(targets, _h=0x403000))
    assert row["proof"]["references"] == "fail"
    for offset, kind in ((6, 6), (14, 6), (8, 20)):
        damaged = array_object(bytes(64), relocations=((offset, 1, kind),))
        row, = check_initial_data((item,), {UNIT: damaged}, image, targets)
        assert row["content"] == "unknown" and ("cuts" in row["reason"] or "unsupported" in row["reason"])
