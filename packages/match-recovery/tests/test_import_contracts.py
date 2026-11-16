"""Synthetic controls for header-derived import contracts in callee preservation."""

import hashlib
import struct

import pytest

from match_recovery.callee_preservation import callee_preserves
from match_recovery.import_contracts import (
    declare_import_contracts, image_import_contracts, library_contracts)
from match_recovery.pe_coff import PeImage


BASE = 0x401000
SECTION = 0x800
SLOT = BASE + 0x650  # the one IAT slot: KERNEL32.dll!<export>


def _image(placed: dict[int, bytes], export=b"Sleep") -> PeImage:
    """One executable section at BASE whose import directory names one export."""
    text = bytearray(b"\xcc" * SECTION)
    for offset, code in placed.items():
        text[offset:offset + len(code)] = code
    hint_name = 0x1660
    struct.pack_into("<IIIII", text, 0x600, 0x1640, 0, 0, 0x1680, 0x1650)
    struct.pack_into("<IIIII", text, 0x614, 0, 0, 0, 0, 0)
    struct.pack_into("<II", text, 0x640, hint_name, 0)
    struct.pack_into("<II", text, 0x650, hint_name, 0)
    text[0x660:0x663 + len(export)] = b"\0\0" + export + b"\0"
    text[0x680:0x68D] = b"KERNEL32.dll\0"
    raw = bytearray(0x400)
    raw[:2] = b"MZ"
    struct.pack_into("<I", raw, 0x3C, 0x80)
    raw[0x80:0x84] = b"PE\0\0"
    struct.pack_into("<HHIIIHH", raw, 0x84, 0x14C, 1, 0, 0, 0, 224, 0)
    struct.pack_into("<H", raw, 0x98, 0x10B)
    struct.pack_into("<I", raw, 0x98 + 28, 0x400000)
    struct.pack_into("<I", raw, 0x98 + 92, 16)
    struct.pack_into("<II", raw, 0x98 + 96 + 8, 0x1600, 40)
    raw[0x178:0x180] = b".text\0\0\0"
    struct.pack_into("<IIII", raw, 0x180, SECTION, 0x1000, SECTION, 0x400)
    struct.pack_into("<I", raw, 0x178 + 36, 0x60000020)
    data = bytes(raw + text)
    return PeImage(data, hashlib.sha256(data).hexdigest())


def _short_import(symbol: bytes, name_type: int = 3, kind: int = 0, ordinal: int = 0) -> bytes:
    strings = symbol + b"\0KERNEL32.dll\0"
    return (struct.pack("<HHHHIIHH", 0, 0xFFFF, 0, 0x14C, 0, len(strings), ordinal,
                        kind | name_type << 2) + strings)


def _library(*members: bytes) -> bytes:
    out = bytearray(b"!<arch>\n")
    for body in members:
        out += b"KERNEL32.dll/".ljust(16) + b" " * 32 + str(len(body)).encode().ljust(10) + b"`\n"
        out += body
        if len(out) % 2:
            out += b"\n"
    return bytes(out)


def _declared(placed, *members, export=b"Sleep") -> PeImage:
    image = _image(placed, export)
    declare_import_contracts(image, image_import_contracts(image, [("KERNEL32.LIB", _library(*members))]))
    return image


CALL_SLOT = b"\xff\x15" + struct.pack("<I", SLOT)
# push ebx; xor ebx,ebx; push 1; call [Sleep]; pop ebx; ret
SAVED_AROUND_IMPORT = bytes.fromhex("53 31 db 6a 01") + CALL_SLOT + bytes.fromhex("5b c3")


def test_library_contracts_follow_the_decorated_import_name():
    rows = library_contracts(_library(
        _short_import(b"_Sleep@4"),
        _short_import(b"_wsprintfA", name_type=2),
        _short_import(b"@Fast@8"),                  # fastcall: no contract
        _short_import(b"_Data", kind=1),            # data import: no contract
        _short_import(b"_ByOrdinal@12", name_type=0, ordinal=7),
    ), "KERNEL32.LIB")
    assert {(row.name, row.ordinal, row.convention, row.popped) for row in rows} == {
        ("Sleep", None, "stdcall", 4), ("wsprintfA", None, "cdecl", 0), (None, 7, "stdcall", 12)}


def test_stdcall_import_stack_effect_balances_the_restore():
    callee_preserves(_declared({0x100: SAVED_AROUND_IMPORT}, _short_import(b"_Sleep@4")),
                     BASE + 0x100, "ebx")
    # A different header-derived effect misplaces the restoring POP.
    with pytest.raises(ValueError, match="releases|without restoring"):
        callee_preserves(_declared({0x100: SAVED_AROUND_IMPORT}, _short_import(b"_Sleep@8")),
                         BASE + 0x100, "ebx")


def test_cdecl_import_pops_nothing():
    # push ebx; xor ebx,ebx; push 1; call [wsprintfA]; add esp,4; pop ebx; ret
    code = bytes.fromhex("53 31 db 6a 01") + CALL_SLOT + bytes.fromhex("83 c4 04 5b c3")
    callee_preserves(_declared({0x100: code}, _short_import(b"_wsprintfA", name_type=2),
                               export=b"wsprintfA"), BASE + 0x100, "ebx")


def test_documented_import_preserves_only_the_callee_saved_registers():
    code = bytes.fromhex("6a 01") + CALL_SLOT + b"\xc3"  # push 1; call [Sleep]; ret
    image = _declared({0x100: code}, _short_import(b"_Sleep@4"))
    for family in ("ebx", "esi", "edi", "ebp"):
        callee_preserves(image, BASE + 0x100, family)
    with pytest.raises(ValueError, match="does not preserve ecx"):
        callee_preserves(image, BASE + 0x100, "ecx")


def test_import_pointer_held_in_a_register_and_jump_thunks():
    # mov esi,[Sleep]; push 1; call esi; ret  (esi itself is overwritten)
    code = bytes.fromhex("8b 35") + struct.pack("<I", SLOT) + bytes.fromhex("6a 01 ff d6 c3")
    image = _declared({0x100: code}, _short_import(b"_Sleep@4"))
    callee_preserves(image, BASE + 0x100, "ebx")
    with pytest.raises(ValueError, match="overwrites esi"):
        callee_preserves(image, BASE + 0x100, "esi")
    # push ebx; xor ebx,ebx; push 1; call thunk; pop ebx; ret / thunk: jmp [Sleep]
    rel = struct.pack("<i", (BASE + 0x180) - (BASE + 0x105 + 5))
    caller = bytes.fromhex("53 31 db 6a 01 e8") + rel + bytes.fromhex("5b c3")
    thunk = b"\xff\x25" + struct.pack("<I", SLOT)
    callee_preserves(_declared({0x100: caller, 0x180: thunk}, _short_import(b"_Sleep@4")),
                     BASE + 0x100, "ebx")


def test_imports_without_a_header_derived_contract_still_refuse():
    # Undeclared image: no contract at all.
    with pytest.raises(ValueError, match="unknown stack depth"):
        callee_preserves(_image({0x100: SAVED_AROUND_IMPORT}), BASE + 0x100, "ebx")
    with pytest.raises(ValueError, match="unprovable call"):
        callee_preserves(_image({0x100: bytes.fromhex("6a 01") + CALL_SLOT + b"\xc3"}),
                         BASE + 0x100, "ebx")
    # The library names a different export, or only a fastcall decoration.
    for member in (_short_import(b"_Beep@8"), _short_import(b"@Sleep@4")):
        image = _declared({0x100: SAVED_AROUND_IMPORT}, member)
        with pytest.raises(ValueError, match="unknown stack depth"):
            callee_preserves(image, BASE + 0x100, "ebx")
    # Conflicting decorations for one export give no contract.
    image = _image({0x100: SAVED_AROUND_IMPORT})
    assert image_import_contracts(image, [("A.LIB", _library(_short_import(b"_Sleep@4"))),
                                          ("B.LIB", _library(_short_import(b"_Sleep@8")))]) == {}


def test_virtual_calls_still_refuse_with_contracts_declared():
    # mov eax,[ecx]; push 1; call [eax+4]; ret
    virtual = bytes.fromhex("8b 01 6a 01 ff 50 04 c3")
    image = _declared({0x100: virtual}, _short_import(b"_Sleep@4"))
    with pytest.raises(ValueError, match="unprovable call"):
        callee_preserves(image, BASE + 0x100, "ebx")
    # Under a live save the virtual call's stack effect stays unknown.
    saved = bytes.fromhex("53 31 db 8b 01 6a 01 ff 50 04 5b c3")
    image = _declared({0x100: saved}, _short_import(b"_Sleep@4"))
    with pytest.raises(ValueError, match="unknown stack depth"):
        callee_preserves(image, BASE + 0x100, "ebx")


def test_contracts_must_name_real_import_slots():
    image = _image({0x100: SAVED_AROUND_IMPORT})
    contract = image_import_contracts(image, [("K.LIB", _library(_short_import(b"_Sleep@4")))])[SLOT]
    with pytest.raises(ValueError, match="invalid import contract"):
        declare_import_contracts(image, {SLOT + 4: contract})
