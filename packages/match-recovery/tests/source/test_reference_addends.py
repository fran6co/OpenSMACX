"""A masked relocation field still carries its addend, and the addend is checked.

`vehdraw_unk1` read `VehDrawFacetNormals1[index]` where the image reads
`[index - 400]`; the four field bytes were masked, so the body measured
BYTE_EXACT. Only targets with an independent source binding are asked.
"""

import struct
from pathlib import Path

from match_recovery.source import asm, read_file
from match_recovery.source.asm import (CodeReference, Listing, ReferenceTargets,
                                       build_reference_targets, reference_mismatches)

BASE = 0x00401000
TABLE = 0x008D4D9C
CALLEE = 0x00402000

MARKED = """// ORIGINAL: 0x00401000 ?f@@YAXXZ 0x00401000-0x0040100B
void f() {
}
"""


def obj_with(addend: int) -> bytes:
    """`mov eax, [?table+addend]; call ?g; ret`, relocations on both fields."""
    code = (b"\xA1" + struct.pack("<i", addend)      # DIR32 at 1
            + b"\xE8" + bytes(4)                     # REL32 at 6
            + b"\xC3")
    names = ["?f@@YAXXZ", "?table@@3PAHA", "?g@@YAXXZ"]
    raw_ptr = 20 + 40
    reloc_ptr = raw_ptr + len(code)
    relocs = struct.pack("<IIH", 1, 1, 0x0006) + struct.pack("<IIH", 6, 2, 0x0014)
    sym_ptr = reloc_ptr + len(relocs)
    header = struct.pack("<HHIIIHH", 0x14C, 1, 0, sym_ptr, len(names), 0, 0)
    section = struct.pack("<8sIIIIIIHHI", b".text\x00\x00\x00", 0, 0, len(code),
                          raw_ptr, reloc_ptr, 0, 2, 0, 0x60000020)
    entries, strtab, offset = b"", b"", 4
    for index, name in enumerate(names):
        entries += (bytes(4) + struct.pack("<I", offset)
                    + struct.pack("<IhHBB", 0, 1 if index == 0 else 0,
                                  0x20 if index != 1 else 0, 2, 0))
        strtab += name.encode() + b"\0"
        offset += len(name) + 1
    return header + section + code + relocs + entries + struct.pack("<I", 4 + len(strtab)) + strtab


def image_listing(read: int) -> Listing:
    call = CALLEE - (BASE + 10)
    code = b"\xA1" + struct.pack("<I", read) + b"\xE8" + struct.pack("<i", call) + b"\xC3"
    return Listing(code=code, base=BASE, mask=frozenset(range(1, 5)))


def measure(tmp_path, monkeypatch, addend, targets, read=TABLE - 0x12C0):
    path = tmp_path / "x.cpp"
    path.write_text(MARKED)
    record = read_file(path)[0]
    monkeypatch.setattr(asm, "original_asm", lambda *_args, **_kw: image_listing(read))
    compiled = asm.subject_asm(obj_with(addend), record)
    return asm.compare_subject(record, "unused.exe", compiled, targets=targets)


TARGETS = build_reference_targets(
    [(Path("elsewhere.cpp"), CALLEE, ("?g@@YAXXZ",), False)], [("?table@@3PAHA", TABLE)])


def test_the_addend_is_read_from_the_object(tmp_path):
    path = tmp_path / "x.cpp"
    path.write_text(MARKED)
    compiled = asm.subject_asm(obj_with(-0x12C0), read_file(path)[0])
    assert compiled.references == (
        CodeReference(1, False, "?table@@3PAHA", 2, -0x12C0),
        CodeReference(6, True, "?g@@YAXXZ", 2, 0))


def test_a_wrong_offset_into_a_bound_object_is_not_byte_exact(tmp_path, monkeypatch):
    result = measure(tmp_path, monkeypatch, 0, TARGETS)
    assert result.verdict == "SHAPE_EXACT"
    assert result.allocation_only                # a semantic claim cannot hold either
    assert "?table@@3PAHA+0x0: source 0x008D4D9C, image 0x008D3ADC" in result.diagnostic


def test_the_image_offset_and_call_target_are_byte_exact(tmp_path, monkeypatch):
    assert measure(tmp_path, monkeypatch, -0x12C0, TARGETS).verdict == "BYTE_EXACT"


def test_a_wrong_call_target_is_not_byte_exact(tmp_path, monkeypatch):
    targets = build_reference_targets(
        [(Path("elsewhere.cpp"), CALLEE + 0x10, ("?g@@YAXXZ",), False)], [("?table@@3PAHA", TABLE)])
    result = measure(tmp_path, monkeypatch, -0x12C0, targets)
    assert result.verdict == "SHAPE_EXACT"
    assert "?g@@YAXXZ" in result.diagnostic


def test_unbound_and_ambiguous_targets_stay_masked(tmp_path, monkeypatch):
    assert measure(tmp_path, monkeypatch, 0, None).verdict == "BYTE_EXACT"
    assert measure(tmp_path, monkeypatch, 0, ReferenceTargets()).verdict == "BYTE_EXACT"
    ambiguous = build_reference_targets([], [("?table@@3PAHA", TABLE), ("?table@@3PAHA", 0x500000)])
    assert ambiguous.external == {}
    assert measure(tmp_path, monkeypatch, 0, ambiguous).verdict == "BYTE_EXACT"


def test_local_targets_are_scoped_to_their_host(tmp_path):
    host = tmp_path / "a.cpp"
    targets = build_reference_targets([(host, 0x401000, ("?h@@YAXXZ",), True)])
    local = CodeReference(0, False, "?h@@YAXXZ", 3, 0)
    assert targets.address(local, host) == 0x401000
    assert targets.address(local, tmp_path / "b.cpp") is None
    assert targets.address(CodeReference(0, False, "?h@@YAXXZ", 2, 0), host) is None
    original = Listing(code=bytes(4), base=BASE)
    compiled = Listing(code=bytes(4), base=BASE, references=(local,))
    assert reference_mismatches(original, compiled, targets, host)


RUNTIME_COMPONENT = """// RECOVERY_COMPONENT_BEGIN v1
// {
//   "archive_sha256": "%s",
//   "component": {"component_id": "crt", "delivery": "toolchain", "evidence": ["test"],
//                 "ownership": "runtime", "version": "test"},
//   "kind": "toolchain_component", "absolute_symbols": [],
//   "license_evidence": "test",
//   "schema_version": 1,
//   "image": {"sha256": "%s", "address_space": "pe:va"},
//   "bindings": [
//     {"evidence": "test", "origin": "0x00402000", "spans": ["0x00402000-0x00402010"],
//      "symbol": "?g@@YAXXZ", "unit": "build/intel/mt_obj/g.obj"}
//   ]
// }
// RECOVERY_COMPONENT_END
""" % ("1" * 64, "2" * 64)


def test_a_library_call_target_is_checked_against_its_component_origin(tmp_path, monkeypatch):
    """`_stricmp` called where the image calls `_strcmp` once measured BYTE_EXACT."""
    from match_recovery.component_source import parse_component
    from match_recovery.source_selection import component_reference_objects
    manifest = parse_component(RUNTIME_COMPONENT)
    objects = component_reference_objects([manifest]) + [("?table@@3PAHA", TABLE)]
    assert ("?g@@YAXXZ", CALLEE) in objects
    assert measure(tmp_path, monkeypatch, -0x12C0, build_reference_targets([], objects)).verdict == "BYTE_EXACT"
    moved = parse_component(RUNTIME_COMPONENT.replace("0x00402000", "0x00403000").replace("0x00402010", "0x00403010"))
    result = measure(tmp_path, monkeypatch, -0x12C0, build_reference_targets(
        [], component_reference_objects([moved]) + [("?table@@3PAHA", TABLE)]))
    assert result.verdict == "SHAPE_EXACT"
    assert "?g@@YAXXZ+0x0: source 0x00403000, image 0x00402000" in result.diagnostic


# ------------------------------------------------------------ string literals

LITERAL_AT = 0x00688CA4


def literal_obj(name: str, literal: bytes, addend: int = 0) -> bytes:
    """`push offset <name>+addend; ret`, the literal in a second `.data` section."""
    code = b"\x68" + struct.pack("<i", addend) + b"\xC3"    # DIR32 at 1
    names = ["?f@@YAXXZ", name]
    raw_ptr = 20 + 2 * 40
    data_ptr = raw_ptr + len(code)
    reloc_ptr = data_ptr + len(literal)
    relocs = struct.pack("<IIH", 1, 1, 0x0006)
    sym_ptr = reloc_ptr + len(relocs)
    header = struct.pack("<HHIIIHH", 0x14C, 2, 0, sym_ptr, len(names), 0, 0)
    text = struct.pack("<8sIIIIIIHHI", b".text\x00\x00\x00", 0, 0, len(code),
                       raw_ptr, reloc_ptr, 0, 1, 0, 0x60000020)
    data = struct.pack("<8sIIIIIIHHI", b".data\x00\x00\x00", 0, 0, len(literal),
                       data_ptr, 0, 0, 0, 0, 0xC0300040)
    entries, strtab, offset = b"", b"", 4
    for index, symbol in enumerate(names):
        storage = 2 if index == 0 else (3 if symbol.startswith("$SG") else 2)
        entries += (bytes(4) + struct.pack("<I", offset)
                    + struct.pack("<IhHBB", 0, index + 1, 0x20 if index == 0 else 0, storage, 0))
        strtab += symbol.encode() + b"\0"
        offset += len(symbol) + 1
    return (header + text + data + code + literal + relocs + entries
            + struct.pack("<I", 4 + len(strtab)) + strtab)


LITERAL_MARKED = """// ORIGINAL: 0x00401000 ?f@@YAXXZ 0x00401000-0x00401006
void f() {
}
"""


def measure_literal(tmp_path, monkeypatch, obj, image_strings, pushed=LITERAL_AT):
    """Compare `obj` against an image pushing `pushed`, whose data holds `image_strings`."""
    path = tmp_path / "x.cpp"
    path.write_text(LITERAL_MARKED)
    record = read_file(path)[0]
    code = b"\x68" + struct.pack("<I", pushed) + b"\xC3"
    monkeypatch.setattr(asm, "original_asm", lambda *_args, **_kw: Listing(
        code=code, base=BASE, mask=frozenset(range(1, 5))))

    def pe_bytes(_exe, address, size):
        for start, raw in image_strings.items():
            if start <= address and address + size <= start + len(raw):
                return raw[address - start:address - start + size]
        raise ValueError(f"0x{address:08X} is in no section")
    monkeypatch.setattr(asm, "_pe_bytes", pe_bytes)
    compiled = asm.subject_asm(obj, record)
    return asm.compare_subject(record, "unused.exe", compiled, targets=ReferenceTargets())


def test_a_literal_reference_carries_the_object_bytes(tmp_path):
    path = tmp_path / "x.cpp"
    path.write_text(LITERAL_MARKED)
    compiled = asm.subject_asm(literal_obj("$SG1234", b"NOTINTRIAD\0\0"), read_file(path)[0])
    assert compiled.references == (CodeReference(1, False, "$SG1234", 3, 0, b"NOTINTRIAD\0"),)


def test_a_wrong_string_literal_is_not_byte_exact(tmp_path, monkeypatch):
    """Console::terraform pushed "NOTINTRIAD" where the image pushes "TERRASEA"."""
    result = measure_literal(tmp_path, monkeypatch, literal_obj("$SG1234", b"NOTINTRIAD\0\0"),
                             {LITERAL_AT: b"TERRASEA\0\0\0\0NOTINTRIAD\0\0"})
    assert result.verdict == "SHAPE_EXACT"
    assert "literal 'NOTINTRIAD' vs image 'TERRASEA'" in result.diagnostic


def test_the_same_text_at_another_address_is_byte_exact(tmp_path, monkeypatch):
    """Pooled and unpooled copies share text, not addresses; only text is asked."""
    image = {LITERAL_AT: b"TERRASEA\0\0\0\0"}
    assert measure_literal(tmp_path, monkeypatch, literal_obj("$SG1234", b"TERRASEA\0\0\0\0"),
                           image).verdict == "BYTE_EXACT"
    pooled = "??_C@_08ABCDEFGH@TERRASEA?$AA@"
    assert measure_literal(tmp_path, monkeypatch, literal_obj(pooled, b"TERRASEA\0"),
                           image).verdict == "BYTE_EXACT"


def test_a_literal_is_compared_through_its_terminator(tmp_path, monkeypatch):
    """A prefix of the image's text is a different string."""
    result = measure_literal(tmp_path, monkeypatch, literal_obj("$SG1", b"TERRA\0\0\0"),
                             {LITERAL_AT: b"TERRASEA\0\0\0\0"})
    assert result.verdict == "SHAPE_EXACT"
    assert "literal 'TERRA' vs image 'TERRASEA'" in result.diagnostic


def test_a_literal_addend_reads_both_sides_at_the_same_offset(tmp_path, monkeypatch):
    obj = literal_obj("$SG1", b"XTERRASEA\0\0\0", addend=1)
    image = {LITERAL_AT - 1: b"YTERRASEA\0\0\0"}
    assert measure_literal(tmp_path, monkeypatch, obj, image).verdict == "BYTE_EXACT"
    result = measure_literal(tmp_path, monkeypatch, obj, {LITERAL_AT - 1: b"XTERRALAND\0\0"})
    assert result.verdict == "SHAPE_EXACT"
    assert "$SG1+0x1: literal 'TERRASEA' vs image 'TERRALAND'" in result.diagnostic


def test_a_wide_literal_ends_at_an_aligned_nul_pair(tmp_path, monkeypatch):
    wide = "AB".encode("utf-16-le") + b"\0\0"
    name = "??_C@_1G@ABCDEFGH@?$AAA?$AAB?$AA?$AA@"
    image = {LITERAL_AT: wide + "C".encode("utf-16-le") + b"\0\0"}
    assert measure_literal(tmp_path, monkeypatch, literal_obj(name, wide), image).verdict == "BYTE_EXACT"
    other = {LITERAL_AT: "AC".encode("utf-16-le") + b"\0\0"}
    assert measure_literal(tmp_path, monkeypatch, literal_obj(name, wide), other).verdict == "SHAPE_EXACT"


def test_an_unreadable_image_literal_is_not_byte_exact(tmp_path, monkeypatch):
    result = measure_literal(tmp_path, monkeypatch, literal_obj("$SG1", b"TERRASEA\0"), {})
    assert result.verdict == "SHAPE_EXACT"
    assert "unreadable" in result.diagnostic


def test_other_static_data_stays_masked(tmp_path, monkeypatch):
    """Only literal symbols are read; other unbound data is unchanged."""
    result = measure_literal(tmp_path, monkeypatch, literal_obj("_table", b"TERRA\0"),
                             {LITERAL_AT: b"OTHER\0"})
    assert result.verdict == "BYTE_EXACT"
