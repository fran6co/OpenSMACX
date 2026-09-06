"""Synthetic section-ownership fixtures; no original executable bytes."""

from dataclasses import replace
import hashlib
import struct

import pytest

from match_recovery.bound_objects import FunctionBinding, check_objects
from match_recovery.model import Origin, Span
from match_recovery.pe_coff import Coff, PeImage
from test_binary import pe_fixture


def object_fixture(*, tail=b"\xc3", parent=1, selection=5, extra=None):
    # Each section has a section symbol, its auxiliary definition and a code
    # symbol. Entry relocations refer to the tail symbol (index 5), not bytes.
    specs = [(b".text", b"\xe8\0\0\0\0\xc3", ((1, 5, 20),), 0, 2, 0x60501020),
             (b".text$x", tail, (), parent, selection, 0x60501020)]
    if extra is not None:
        specs.append(extra)
    header_size = 20 + 40 * len(specs)
    headers, payload, symbols = [], bytearray(), bytearray()
    for number, (name, code, relocations, owner, choice, flags) in enumerate(specs, 1):
        raw = code + b"\xcc" * (-len(code) % 16)
        offset = header_size + len(payload)
        payload.extend(raw)
        reloc_offset = header_size + len(payload) if relocations else 0
        for relocation in relocations:
            payload.extend(struct.pack("<IIH", *relocation))
        headers.append(name.ljust(8, b"\0") + struct.pack("<IIIIIIHHI", 0, 0, len(raw), offset,
                                                        reloc_offset, 0, len(relocations), 0, flags))
        symbols.extend(name.ljust(8, b"\0") + struct.pack("<IhHBB", 0, number, 0, 3, 1))
        symbols.extend(struct.pack("<IHHIHB3x", len(raw), len(relocations), 0, 0, owner, choice))
        label = b"_f" if number == 1 else f"$tail{number}".encode()
        symbols.extend(label.ljust(8, b"\0") + struct.pack("<IhHBB", 0, number,
                                                           0x20 if number == 1 else 0,
                                                           2 if number == 1 else 6, 0))
    return (struct.pack("<HHIIIHH", 0x14C, len(specs), 0, header_size + len(payload),
                        len(specs) * 3, 0, 0) + b"".join(headers) + payload + symbols + struct.pack("<I", 4))


def fixture(**kwargs):
    # The source declares entry at 0x401000 and tail at 0x401020 independently.
    raw = pe_fixture(b"\xe8\x1b\0\0\0\xc3" + bytes(26) + b"\xc3")
    pin = hashlib.sha256(raw).hexdigest()
    binding = FunctionBinding(Origin(pin, "pe:va", 0x401000),
                              (Span(0x401000, 0x401006), Span(0x401020, 0x401021)), "_f", "src/f.cpp")
    return binding, {binding.unit: Coff(object_fixture(**kwargs))}, PeImage(raw, pin)


def test_unique_associated_tail_checks_all_spans_and_references():
    binding, objects, image = fixture()
    row, = check_objects([binding], objects, image)
    assert row["result"] == row["bytes"] == row["references"] == "pass"
    assert len(row["spans"]) == 2
    assert row["relocations"][0]["symbol"] == "$tail2"
    assert row["relocations"][0]["span_start"] == 0x401000
    assert [proof["alignment_fill"] for proof in row["spans"]] == [10, 15]


@pytest.mark.parametrize("tail", [b"\x90", b"\xc3\xc3"])
def test_matching_entry_cannot_hide_changed_or_extended_cleanup(tail):
    binding, objects, image = fixture(tail=tail)
    row, = check_objects([binding], objects, image)
    assert row["result"] == "fail"
    assert row["spans"][0]["result"] == "pass" and row["spans"][1]["result"] == "fail"


def test_source_extent_is_not_moved_to_fit_the_original_call_operand():
    binding, objects, image = fixture()
    binding = replace(binding, spans=(binding.spans[0], Span(0x401030, 0x401031)))
    row, = check_objects([binding], objects, image)
    assert row["result"] == "fail" and row["spans"][0]["references"] == "fail"


def test_unrecorded_emitted_cleanup_is_not_silently_ignored():
    binding, objects, image = fixture()
    binding = replace(binding, spans=binding.spans[:1])
    row, = check_objects([binding], objects, image)
    assert row["result"] == "unknown" and "associated recorded tail" in row["reason"]


def test_matching_name_order_and_bytes_do_not_replace_compiler_ownership():
    binding, objects, image = fixture(selection=2)
    row, = check_objects([binding], objects, image)
    assert row["result"] == "unknown"


def test_multiple_associated_code_sections_are_not_paired_by_order():
    binding, objects, image = fixture(extra=(b".text$y", b"\xc3", (), 1, 5, 0x60501020))
    binding = replace(binding, spans=(*binding.spans, Span(0x401040, 0x401041)))
    assert check_objects([binding], objects, image)[0]["result"] == "unknown"


def test_associated_data_does_not_take_the_place_of_a_code_tail():
    binding, objects, image = fixture(extra=(b".xdata$x", bytes(16), (), 1, 5, 0x40401040))
    obj = objects[binding.unit]
    assert obj.code_sections(obj.function("_f")) == (1, 2)
    assert check_objects([binding], objects, image)[0]["result"] == "pass"
    # A real relocation to that unmapped exception data still stays unknown.
    obj.sections = (replace(obj.sections[0], relocations=((1, 8, 20),)), *obj.sections[1:])
    row, = check_objects([binding], objects, image)
    assert row["result"] == row["references"] == "unknown"


@pytest.mark.parametrize("parent", [0, 2, 3, 65535])
def test_invalid_or_cyclic_association_cannot_supply_placement(parent):
    with pytest.raises(ValueError, match="COMDAT"):
        Coff(object_fixture(parent=parent))


def test_transitive_ownership_is_followed_without_using_section_order():
    obj = Coff(object_fixture(parent=3, extra=(b".xdata$x", bytes(16), (), 1, 5, 0x40401040)))
    assert obj.code_sections(obj.function("_f")) == (1, 2)


def test_overlapping_original_spans_are_refused():
    binding, _, _ = fixture()
    with pytest.raises(ValueError, match="overlap"):
        replace(binding, spans=(binding.spans[0], Span(0x401004, 0x401010)))


@pytest.mark.parametrize("offset, value", [(0, 15), (4, 1), (14, 7), (15, 1)])
def test_damaged_section_auxiliary_metadata_is_not_accepted(offset, value):
    data = bytearray(object_fixture())
    symbol_offset, = struct.unpack_from("<I", data, 8)
    # Tail section definition: skip entry's three records plus section symbol.
    data[symbol_offset + 4 * 18 + offset] = value
    with pytest.raises(ValueError, match="COFF|COMDAT"):
        Coff(bytes(data))
