"""MASM-dialect sources: one annotation grammar, `;` comments, assembled bodies."""

import struct
from dataclasses import replace
from pathlib import Path

from match_recovery.source import asm
from match_recovery.source.grammar import (assembler_annotation_line,
                                           assembler_comment_line)
from match_recovery.source.model import Mode, Tier
from match_recovery.source.reader import read_text
from match_recovery.source.writer import write_bytes

MODULE = """; header prose
        .386
; ORIGINAL: 0x00401000 sub_401000 0x00401000-0x00401004;0x00402000-0x00402001 FILE
; symbol    ?f@@YAXXZ
; TRIED: kept as a note
?f@@YAXXZ PROC PUBLIC
        mov al, ';'          ; a quoted semicolon is data
        ret
?f@@YAXXZ ENDP
; ORIGINAL: 0x00401010 sub_401010 0x00401010-0x00401011 FILE
?g@@YAXXZ PROC PUBLIC
        ret
?g@@YAXXZ ENDP
        END
"""


def test_assembler_comments_carry_the_one_grammar():
    records = read_text(MODULE, Path("/tree/src/module.asm"))
    assert [(row.address, row.mode, row.line) for row in records] == [
        (0x00401000, Mode.FILE, 3), (0x00401010, Mode.FILE, 10)]
    first = records[0]
    assert first.symbol == "?f@@YAXXZ"
    assert first.image_spans == ((0x00401000, 0x00401004), (0x00402000, 0x00402001))
    assert first.ruled_out == ("kept as a note",)


def test_semicolon_comments_are_not_annotations_in_cpp():
    assert read_text(MODULE, Path("/tree/src/module.cpp")) == []


def test_quoted_semicolons_are_operands_not_comment_openers():
    assert assembler_annotation_line("  mov al, ';' ; x") == "  mov al, ';' // x"
    assert assembler_comment_line("// ORIGINAL: 0x1 a 0x1-0x2;0x3-0x4") == \
        "; ORIGINAL: 0x1 a 0x1-0x2;0x3-0x4"
    assert assembler_comment_line("  mov eax, 1") == "  mov eax, 1"


def test_claim_publication_respells_only_the_changed_marker():
    path = Path("/tree/src/module.asm")
    raw = MODULE.replace("\n", "\r\n").encode()
    records = read_text(MODULE, path)
    assert write_bytes(raw, records) == raw
    updated = write_bytes(raw, [replace(records[0], byte_exact=True)])
    expected = MODULE.replace(
        "0x00402000-0x00402001 FILE\n", "0x00402000-0x00402001 FILE BYTE_EXACT\n")
    assert updated == expected.replace("\n", "\r\n").encode()
    assert read_text(updated.decode(), path)[0].byte_exact


def segment_coff(functions, *, name=b"_SELFMOD", characteristics=0xE0000020,
                 kind=0x20):
    """One named segment holding several procedures, as an assembler emits."""
    blob, placed = b"", []
    for symbol, code in functions:
        placed.append((symbol, len(blob)))
        blob += code
    raw_ptr = 20 + 40
    sym_ptr = raw_ptr + len(blob)
    header = struct.pack("<HHIIIHH", 0x14C, 1, 0, sym_ptr, len(placed), 0, 0)
    section = struct.pack("<8sIIIIIIHHI", name.ljust(8, b"\x00"), 0, 0,
                          len(blob), raw_ptr, 0, 0, 0, 0, characteristics)
    entries, strtab, offset = b"", b"", 4
    for symbol, value in placed:
        entries += (b"\x00\x00\x00\x00" + struct.pack("<I", offset)
                    + struct.pack("<IhHBB", value, 1, kind, 2, 0))
        strtab += symbol.encode() + b"\x00"
        offset += len(symbol) + 1
    return header + section + blob + entries + struct.pack("<I", 4 + len(strtab)) + strtab


def test_a_procedure_ends_at_the_next_procedure_in_its_segment():
    obj = segment_coff([("?f@@YAXXZ", b"\x33\xC0\xC3"), ("?g@@YAXXZ", b"\x40\xC3")])
    assert asm._coff_function_masked(obj, "?f@@YAXXZ")[0] == b"\x33\xC0\xC3"
    assert asm._coff_function_masked(obj, "?g@@YAXXZ")[0] == b"\x40\xC3"


def test_non_function_symbols_never_cut_a_body_short():
    obj = segment_coff([("?f@@YAXXZ", b"\x33\xC0"), ("label", b"\xC3")], kind=0)
    assert asm._coff_function_masked(obj, "?f@@YAXXZ")[0] == b"\x33\xC0\xC3"


def test_data_segments_hold_no_function_subjects():
    obj = segment_coff([("?f@@YAXXZ", b"\xC3")], name=b"UVA_DATA",
                       characteristics=0xC0000040)
    assert asm.text_symbols(obj) == []


def test_writable_code_is_refused_for_compilers_and_measured_for_assemblers(monkeypatch):
    monkeypatch.setattr(asm, "_sections", lambda _exe: (
        ("_SELFMOD", 0x00664000, 0x00669000, 0xE0000020),))
    text = MODULE.replace("0x00401000", "0x00664000").replace(
        "0x00401004;0x00402000-0x00402001", "0x00664004")
    assembled = read_text(text, Path("/tree/src/module.asm"))[0]
    compiled = replace(assembled, path=Path("/tree/src/module.cpp"))
    assert asm.span_refusal(compiled, "image.exe") is Tier.REFUSED
    assert asm.span_refusal(assembled, "image.exe") is None
    hosted = replace(assembled, context="src/module.cpp")
    assert asm.span_refusal(hosted, "image.exe") is Tier.REFUSED


BODY_MODULE = """; ORIGINAL: 0x00401000 sub_401000 0x00401000-0x00401004
; symbol    ?f@@YAXXZ
?f@@YAXXZ PROC PUBLIC
        ret
?f@@YAXXZ ENDP
; ORIGINAL: 0x00401010 sub_401010 0x00401010-0x00401011
?g@@YAXXZ PROC PUBLIC
        ret
?g@@YAXXZ ENDP
"""


def test_a_body_marker_owns_its_procedure_up_to_the_next_marker():
    first, second = read_text(BODY_MODULE, Path("/tree/src/module.asm"))
    assert first.mode is Mode.BODY and not first.extract_error
    assert "?f@@YAXXZ ENDP" in first.region and "?g@@YAXXZ" not in first.region
    assert "?g@@YAXXZ ENDP" in second.region
