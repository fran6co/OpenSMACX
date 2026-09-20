"""Local generated bodies require physical ownership, not convenient names."""

from dataclasses import replace
import struct

import pytest

from match_recovery.source import asm
from match_recovery.source import read_text


def local_coff(code=b"\xc3\x90\xcc", *, name="_$E19", section_name=".text",
               flags=0x60501020, value=0, kind=0x20, definition=True,
               peers=(), relocations=(), extra=None):
    """An explicit section definition, local function and optional damage."""
    def symbol(label, offset, type_, storage, aux=0, section=1):
        assert len(label.encode()) <= 8
        return label.encode().ljust(8, b"\0") + struct.pack("<IhHBB", offset, section, type_, storage, aux)

    symbols = []
    if definition:
        symbols += [symbol(section_name, 0, 0, 3, 1),
                    struct.pack("<IHHIHB3x", len(code), len(relocations), 0, 0, 0,
                                2 if flags & 0x1000 else 0)]
    target = len(symbols)
    symbols.append(symbol(name, value, kind, 3))
    symbols.extend(symbol(*peer) for peer in peers)
    if extra:
        symbols += [symbol(extra["name"], 0, 0, 3, 1, 2),
                    struct.pack("<IHHIHB3x", 1, 0, 0, 0, extra.get("parent", 0), extra["selection"])]
        if extra.get("alias"):
            symbols.append(symbol(name, 0, 0, 2, section=2))
    body_at = 60 + (40 if extra else 0)
    reloc_at = body_at + len(code)
    symbols_at = reloc_at + len(relocations) * 10 + (1 if extra else 0)
    header = struct.pack("<HHIIIHH", 0x14C, 2 if extra else 1, 0, symbols_at, len(symbols), 0, 0)
    section = struct.pack("<8sIIIIIIHHI", section_name.encode().ljust(8, b"\0"), 0, 0,
                          len(code), body_at, reloc_at if relocations else 0, 0,
                          len(relocations), 0, flags)
    fixups = b"".join(struct.pack("<IIH", offset, target, type_) for offset, type_ in relocations)
    if extra:
        section += struct.pack("<8sIIIIIIHHI", extra["name"].encode().ljust(8, b"\0"), 0, 0,
                               1, symbols_at - 1, 0, 0, 0, 0, extra["flags"])
    return header + section + code + fixups + (b"\0" if extra else b"") + b"".join(symbols) + struct.pack("<I", 4)


@pytest.mark.parametrize("section_name", [".text", ".text$q"])
def test_explicit_local_function_uses_its_exclusive_code_comdat(section_name):
    obj = local_coff(section_name=section_name)
    assert asm._coff_function_masked(obj, "historical_name", "_$E19") == (b"\xc3", frozenset())
    assert asm.text_symbols(obj) == []  # Existing external-symbol diagnostics stay external.


def test_local_original_extent_bounds_body_not_alignment_padding():
    assert asm._coff_function_masked(
        local_coff(), "historical", "_$E19", spans=((0x401000, 0x401001),)
    ) == (b"\xc3", frozenset())
    with pytest.raises(ValueError, match="exceeds its explicit original image span"):
        asm._coff_function_masked(
            local_coff(b"\xc3\x33\xc0\xc3"), "historical", "_$E19",
            spans=((0x401000, 0x401001),))


def test_local_relocations_mask_only_complete_fields():
    code = bytes.fromhex("b900000000e9000000009090")
    obj = local_coff(code, relocations=((1, 6), (6, 20)))
    body, mask = asm._coff_function_masked(obj, "historical", "_$E19")
    assert body == code[:10]
    assert mask == frozenset([1, 2, 3, 4, 6, 7, 8, 9])


@pytest.mark.parametrize("options,diagnostic", [
    ({"flags": 0x60500020}, "code COMDAT"),
    ({"flags": 0xC0501040}, "code COMDAT"),
    ({"value": 1}, "exclusively own"),
    ({"kind": 0}, "one defined COFF function"),
    ({"definition": False}, "independently defined"),
    ({"code": b"\x90\xcc"}, "no code"),
    ({"peers": (("other", 1, 0x20, 3),)}, "exclusively own"),
    ({"peers": (("other", 0, 0x20, 2),)}, "exclusively own"),
    ({"peers": (("_$E19", 0, 0x20, 2),)}, "ambiguous"),
    ({"peers": (("_$E19", 0, 0, 3),)}, "ambiguous"),
    ({"peers": (("_$E19", 0, 0x20, 3),)}, "ambiguous"),
])
def test_unbounded_or_ambiguous_local_definitions_refuse(options, diagnostic):
    with pytest.raises(ValueError, match=diagnostic):
        asm._coff_function_masked(local_coff(**options), "historical", "_$E19")


def test_unrelated_local_label_does_not_supply_or_destroy_the_function_boundary():
    obj = local_coff(peers=(("$label", 1, 0, 3),))
    assert asm._coff_function_masked(obj, "historical", "_$E19")[0] == b"\xc3"
    with pytest.raises(ValueError, match="one defined COFF function"):
        asm._coff_function_masked(obj, "historical", "$label")


def test_local_names_have_no_underscore_guess_or_neighbor_fallback():
    obj = local_coff(name="_local")
    with pytest.raises(ValueError, match="not found"):
        asm._coff_function_masked(obj, "local")
    with pytest.raises(ValueError, match="fact is stale"):
        asm._coff_function_masked(obj, "historical", "absent")


def test_same_name_external_data_in_another_section_is_ambiguous():
    obj = local_coff(extra={"name": ".data", "flags": 0xC0100040, "selection": 0, "alias": True})
    with pytest.raises(ValueError, match="ambiguous"):
        asm._coff_function_masked(obj, "historical", "_$E19")


def test_associated_data_is_not_a_second_function_but_associated_code_cannot_be_omitted():
    extra = {"name": ".child", "flags": 0x40101040, "selection": 5, "parent": 1}
    assert asm._coff_function_masked(local_coff(extra=extra), "historical", "_$E19")[0] == b"\xc3"
    with pytest.raises(ValueError, match="without associated code"):
        asm._coff_function_masked(local_coff(extra={**extra, "flags": 0x60101020}), "historical", "_$E19")


def test_local_function_in_an_associated_section_is_not_independent():
    raw = bytearray(local_coff(extra={"name": ".root", "flags": 0x60101020, "selection": 2}))
    symbols_at, = struct.unpack_from("<I", raw, 8)
    struct.pack_into("<HB", raw, symbols_at + 18 + 12, 2, 5)
    with pytest.raises(ValueError, match="independently defined"):
        asm._coff_function_masked(bytes(raw), "historical", "_$E19")


@pytest.mark.parametrize("offset,value", [(0, 99), (12, 0xFFFF), (14, 0), (15, 1)])
def test_malformed_comdat_auxiliary_cannot_supply_a_boundary(offset, value):
    raw = bytearray(local_coff())
    symbols_at, = struct.unpack_from("<I", raw, 8)
    struct.pack_into("<H" if offset == 12 else "<B", raw, symbols_at + 18 + offset, value)
    with pytest.raises(ValueError):
        asm._coff_function_masked(bytes(raw), "historical", "_$E19")


@pytest.mark.parametrize("relocations,diagnostic", [
    (((1, 1),), "unsupported i386 relocation"),
    (((4, 6),), "crosses its code boundary"),
    (((100, 6),), "crosses its code boundary"),
    (((1, 6), (2, 20)), "overlapping relocations"),
])
def test_invalid_local_relocations_are_not_silently_clipped_or_ignored(relocations, diagnostic):
    obj = local_coff(b"\xb8\0\0\0\0\xc3\x90\x90", relocations=relocations)
    with pytest.raises(ValueError, match=diagnostic):
        asm._coff_function_masked(obj, "historical", "_$E19")


def test_multispan_annotation_cannot_bank_only_its_local_primary_body(tmp_path):
    record, = read_text("// ORIGINAL: 0x00401000 historical 0x00401000-0x00401001 FILE\n"
                       "// symbol _$E19\nint owner;\n", tmp_path / "owner.cc")
    obj = local_coff()
    assert asm.subject_asm(obj, record).code == b"\xc3"
    damaged = replace(record, image_spans=(*record.image_spans, (0x401010, 0x401011)))
    with pytest.raises(ValueError, match="one original image span"):
        asm.subject_asm(obj, damaged)
