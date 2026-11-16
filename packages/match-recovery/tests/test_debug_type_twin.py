"""Debug-typed twins supply data extents only for a provably identical object."""

import struct

import pytest

from match_recovery.codeview_data import data_layout, member_data_layout
from match_recovery.debug_type_twin import attached_twin, prove_debug_type_twin
from match_recovery.pe_coff import Coff
from test_codeview import record
from test_member_object_binding import udt_types

TEXT, DATA, DEBUG, DEBUG_CHILD = 0x60301020 | 0x1000, 0xC0300040, 0x42100048, 0x42101048


def build(parts, symbols):
    return Coff(build_bytes(parts, symbols))


def build_bytes(parts, symbols):
    """parts: (name, raw, relocations, flags); symbols: (name, value, section, type, storage, aux)."""
    strings = b""
    table = b""
    for name, value, section, kind, storage, aux in symbols:
        encoded = name.encode()
        if len(encoded) > 8:
            raw_name = struct.pack("<II", 0, 4 + len(strings))
            strings += encoded + b"\0"
        else:
            raw_name = encoded.ljust(8, b"\0")
        assert len(aux) % 18 == 0
        table += raw_name + struct.pack("<IhHBB", value, section, kind, storage, len(aux) // 18) + aux
    headers, chunks, offset = [], [], 20 + 40 * len(parts)
    for name, raw, rels, flags in parts:
        relbytes = b"".join(struct.pack("<IIH", *rel) for rel in rels)
        headers.append(struct.pack("<8sIIIIIIHHI", name.encode(), 0, 0, len(raw), offset if raw else 0,
                                   offset + len(raw) if rels else 0, 0, len(rels), 0, flags))
        chunks.append(raw + relbytes)
        offset += len(raw) + len(relbytes)
    count = sum(1 + len(aux) // 18 for *_, aux in symbols)
    return (struct.pack("<HHIIIHH", 0x14C, len(parts), 0, offset, count, 0, 0)
            + b"".join(headers + chunks) + table + struct.pack("<I", 4 + len(strings)) + strings)


def section_aux(length, relocs=0, *, lines=0, checksum=0, parent=0, selection=0):
    return struct.pack("<IHHIHB", length, relocs, lines, checksum, parent, selection) + bytes(3)


TYPES, TYPE_INDEX, PATH = udt_types()
VALUE = bytes(16)


def plain_object(*, value=VALUE, label="$L3", reference="_h", extra=()):
    # Sections: 1 .text COMDAT, 2 .data
    parts = [(".text", b"\xc3", (), TEXT), (".data", value, ((0, 3, 6),), DATA)]
    symbols = [(".text", 0, 1, 0, 3, section_aux(1, selection=2)),       # 0, 1
               ("_f", 0, 1, 0x20, 2, b""),                               # 2
               (reference, 0, 0, 0, 2, b""),                             # 3
               ("_parent", 0, 2, 0, 2, b""),                             # 4
               (label, 0, 1, 0, 6, b""),                                 # 5
               *extra]
    return build(parts, symbols)


def twin_object(*, value=VALUE, label="$L12", types=TYPES, extra_types=False, reference="_h",
                data_name=".data", extra=()):
    # Debug records interleave: 1 .debug$S root, 2 .text, 3 .debug$S child,
    # 4 .debug$T, 5 .data (and optionally 6 a second .debug$T).
    debug = struct.pack("<I", 2) + record(0x1008, struct.pack("<IIH", TYPE_INDEX, 0, 0) + b"\x06parent")
    parts = [(".debug$S", debug, ((12, 8, 11), (16, 8, 10)), DEBUG),
             (".text", b"\xc3", (), TEXT),
             (".debug$S", b"", (), DEBUG_CHILD),
             (".debug$T", struct.pack("<I", 2) + types, (), DEBUG),
             (data_name, value, ((0, 7, 6),), DATA)]
    if extra_types:
        parts.append((".debug$T", struct.pack("<I", 2) + types, (), DEBUG))
    symbols = [(".text", 0, 2, 0, 3, section_aux(1, lines=3, selection=2)),       # 0, 1
               (".debug$S", 0, 3, 0, 3, section_aux(0, parent=2, selection=5)),  # 2, 3
               ("_f", 0, 2, 0x20, 2, struct.pack("<IIIIH", 6, 1, 0, 0, 0)),      # 4, 5
               (".bf", 0, 2, 0, 101, b""),                                       # 6
               (reference, 0, 0, 0, 2, b""),                                     # 7
               ("_parent", 0, 5, 0, 2, b""),                                     # 8
               (label, 0, 2, 0, 6, b""),                                         # 9
               *extra]
    return build(parts, symbols)


def test_twin_supplies_extents_in_the_plain_object_numbering():
    plain, twin = plain_object(), twin_object()
    with pytest.raises(ValueError, match="standalone CodeView type section"):
        data_layout(plain, "_parent")
    witness = prove_debug_type_twin(plain, twin)
    plain.debug_type_twin = witness
    assert attached_twin(plain) is witness
    layout = data_layout(plain, "_parent")
    assert (layout["section"], layout["offset"], layout["size"]) == (2, 0, 16)
    member = member_data_layout(plain, "_parent", PATH)
    assert (member["section"], member["offset"], member["size"]) == (2, 4, 4)


def test_object_with_its_own_debug_records_ignores_an_attached_twin():
    twin = twin_object()
    twin.debug_type_twin = prove_debug_type_twin(plain_object(), twin_object())
    assert attached_twin(twin) is None


@pytest.mark.parametrize("change,reason", [
    (dict(value=b"\x01" + bytes(15)), "changes section .data"),
    (dict(reference="_other"), "symbol table"),
    (dict(label="$Lx"), "symbol table"),
    (dict(data_name=".rdata"), "section"),
    (dict(extra=(("_extra", 0, 0, 0, 2, b""),)), "symbol table"),
    (dict(extra_types=True), "one standalone CodeView type section"),
])
def test_any_non_debug_difference_refuses_the_twin(change, reason):
    with pytest.raises(ValueError, match=reason):
        prove_debug_type_twin(plain_object(), twin_object(**change))


def test_renumbered_compiler_locals_are_identified_by_section_and_offset():
    prove_debug_type_twin(plain_object(label="$L99"), twin_object(label="$L1"))
    with pytest.raises(ValueError, match="symbol table"):
        prove_debug_type_twin(plain_object(label="$L99"), twin_object(label="_named"))


def test_relocation_identity_is_compared_through_symbols():
    plain = plain_object(extra=(("_i", 0, 0, 0, 2, b""),))
    twin = twin_object(extra=(("_i", 0, 0, 0, 2, b""),))
    prove_debug_type_twin(plain, twin)
    data = plain.sections[1]
    plain.sections = (plain.sections[0], type(data)(data.data, ((0, 6, 6),), data.flags, data.name))
    with pytest.raises(ValueError, match="relocations of .data"):
        prove_debug_type_twin(plain, twin)


def test_precompiled_types_and_plain_debug_records_are_refused():
    twin = twin_object()
    pch = type(twin.sections[3])(b"", (), DEBUG, ".debug$P")
    twin.sections = (*twin.sections, pch)
    with pytest.raises(ValueError, match="precompiled"):
        prove_debug_type_twin(plain_object(), twin)
    with pytest.raises(ValueError, match="already carries"):
        prove_debug_type_twin(twin_object(), twin_object())


GOTO_LABEL = ("$not_in_turn$58438", 0, 2, 0, 6, b"")  # twin numbering: .text is section 2


def test_twin_only_goto_label_names_are_tolerated():
    # /Z7 records a named C++ goto label as a storage-class-6 `$name$N`
    # symbol in its code section; the plain object has none.
    prove_debug_type_twin(plain_object(), twin_object(extra=(GOTO_LABEL,)))
    # The debug records may refer to it; they are not compared.
    twin = twin_object(extra=(GOTO_LABEL,))
    debug = twin.sections[0]
    twin.sections = (type(debug)(debug.data, (*debug.relocations, (4, 10, 6)), debug.flags, debug.name),
                     *twin.sections[1:])
    prove_debug_type_twin(plain_object(), twin)


def test_goto_label_tolerance_does_not_cover_linkable_or_other_symbols():
    # Referenced from a non-debug section.
    twin = twin_object(extra=(GOTO_LABEL,))
    data = twin.sections[4]
    twin.sections = (*twin.sections[:4], type(data)(data.data, ((0, 10, 6),), data.flags, data.name))
    with pytest.raises(ValueError, match="symbol table|relocations"):
        prove_debug_type_twin(plain_object(), twin)
    for symbol in (("$not_in_turn$58438", 0, 5, 0, 6, b""),          # in a data section
                   ("$not_in_turn$58438", 0, 2, 0, 3, b""),          # another storage class
                   ("$not_in_turn$58438", 0, 2, 0, 6, bytes(18)),    # with an auxiliary record
                   ("_not_in_turn", 0, 2, 0, 6, b""),                # not a `$name$N` label
                   ("$not_in_turn$x", 0, 2, 0, 6, b"")):
        with pytest.raises(ValueError, match="symbol table"):
            prove_debug_type_twin(plain_object(), twin_object(extra=(symbol,)))
    # Only the twin's extra labels are tolerated; the plain object never
    # carries them.
    with pytest.raises(ValueError, match="symbol table"):
        prove_debug_type_twin(plain_object(extra=(("$not_in_turn$58438", 0, 1, 0, 6, b""),)),
                              twin_object(extra=(GOTO_LABEL,)))


SCOPE_LABEL = ("$L59741", 0, 2, 0, 6, b"")  # twin numbering: .text is section 2


def test_unreferenced_numbered_scope_labels_added_by_z7_are_tolerated():
    # Measured on VC6 (stringstruct.cpp): /Z7 adds unreferenced `$L<n>` labels
    # in code sections for its CodeView scope records.
    prove_debug_type_twin(plain_object(), twin_object(extra=(SCOPE_LABEL, ("$L59740", 0, 2, 0, 6, b""))))


def test_numbered_label_tolerance_does_not_cover_referenced_or_data_labels():
    twin = twin_object(extra=(SCOPE_LABEL,))
    data = twin.sections[4]
    twin.sections = (*twin.sections[:4], type(data)(data.data, ((0, 10, 6),), data.flags, data.name))
    with pytest.raises(ValueError, match="symbol table|relocations"):
        prove_debug_type_twin(plain_object(), twin)
    for symbol in (("$L59741", 0, 5, 0, 6, b""),          # in a data section
                   ("$L59741", 0, 2, 0, 6, bytes(18))):   # with an auxiliary record
        with pytest.raises(ValueError, match="symbol table"):
            prove_debug_type_twin(plain_object(), twin_object(extra=(symbol,)))
