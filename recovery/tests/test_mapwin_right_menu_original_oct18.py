"""Pinned original right-menu instruction, table and source-owner witnesses.

These are bounded original facts, not native runtime, whole-body equivalence,
linked-retention or game-acceptance proofs. Proprietary bytes remain local.
"""
from __future__ import annotations

import hashlib
import os
from pathlib import Path
import struct

from capstone import Cs, CS_ARCH_X86, CS_MODE_32
import pytest

from match_recovery.model import Span
from match_recovery.pe_coff import PeImage
from match_recovery.source import read_file


ROOT = Path(__file__).resolve().parents[2]
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
BODY = Span(0x46C0D0, 0x46D552)


@pytest.fixture(scope="module")
def original():
    path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not path.is_file():
        pytest.skip("pinned image unavailable; no original-instruction evidence")
    data = path.read_bytes()
    assert hashlib.sha256(data).hexdigest() == PIN
    image = PeImage(data, PIN)
    instructions = list(Cs(CS_ARCH_X86, CS_MODE_32).disasm(image.read(BODY), BODY.start))
    assert sum(i.size for i in instructions) == BODY.end - BODY.start
    assert all(a.address + a.size == b.address for a, b in zip(instructions, instructions[1:]))
    return image, {i.address: i for i in instructions}


def expect(original, facts):
    _, instructions = original
    for address, pair in facts.items():
        instruction = instructions[address]
        assert (instruction.mnemonic, instruction.op_str) == pair, hex(address)


def calls(original, start, end):
    return [i.op_str for a, i in original[1].items()
            if start <= a < end and i.mnemonic == "call"]


def test_lifetime_and_selected_snapshot_order(original):
    assert calls(original, 0x46C0F3, 0x46C151) == [
        "0x5f8800", "0x5ed240", "0x4e3a50", "0x5bfe90", "0x5f8920"]
    expect(original, {
        0x46C133: ("mov", "ecx, dword ptr [0x93928c]"),
        0x46C13E: ("mov", "dword ptr [ebp - 0x10], ecx"),
        0x46D538: ("call", "0x5f88a0"),
        0x46D54F: ("ret", "0x10"),
    })
    record, = [r for r in read_file(ROOT / "src/mapwin.cpp") if r.address == BODY.start]
    assert (record.symbol or record.name) == "?right_menu@MapWin@@QAEXHHHH@Z"
    assert record.image_spans == ((BODY.start, BODY.end), (0x655944, 0x655959))


def test_menu_uses_space_shared_scratch_and_production_label(original):
    image, _ = original
    assert image.read(Span(0x682820, 0x682822)) == b" \0"
    expect(original, {
        0x46C642: ("push", "0x682820"),
        0x46C647: ("push", "0x9b86a0"),
        0x46C64C: ("mov", "byte ptr [0x9b86a0], 0"),
        0x46C666: ("mov", "eax, dword ptr [edx + 0x898]"),
        0x46C686: ("push", "0x9b86a0"),
        0x46C68B: ("push", "0x33"),
        0x46C6B1: ("mov", "eax, dword ptr [ecx + 0x89c]"),
        0x46C6CD: ("push", "0x682820"),
        0x46C6DC: ("mov", "eax, dword ptr [esi + 0x97d090]"),
        0x46C6E7: ("jge", "0x46c704"),
        0x46C6F6: ("mov", "eax, dword ptr [edx + 0x9a4b68]"),
        0x46C70A: ("lea", "eax, [edx*4 + 0x9ab868]"),
        0x46C72A: ("push", "0x34"),
    })
    # No concatenation of a closing delimiter, nor a null-label skip.
    assert calls(original, 0x46C642, 0x46C692) == [
        "0x645470", "0x6169a0", "0x645470", "0x5f89d0"]


def test_eligibility_uses_selected_aircraft_plan_and_named_preference_bit(original):
    expect(original, {
        0x46C42C: ("je", "0x46c482"),
        0x46C42E: ("mov", "eax, dword ptr [ebp - 0x10]"),
        0x46C437: ("movsx", "eax, word ptr [edx*4 + 0x952832]"),
        0x46C447: ("mov", "al, byte ptr [edx*4 + 0x9ab88c]"),
        0x46C454: ("cmp", "byte ptr [eax + 0x94a379], 2"),
        0x46C475: ("push", "0x6e"),
        0x46C875: ("test", "ah, 4"),
        0x46C938: ("test", "ch, 4"),
        0x46C93B: ("je", "0x46c9d3"),
        0x46CCE1: ("test", "ah, 4"),
        0x46CB48: ("movsx", "eax, word ptr [eax*4 + 0x952832]"),
        0x46CB56: ("cmp", "byte ptr [edx*4 + 0x9ab892], 9"),
        0x46CB77: ("push", "0x66"),
    })
    assert 0x9AB892 - 0x9AB868 == 0x2A  # VehPrototype::plan, not stack count.
    # Clearing advanced-menu preference bypasses BOTH patrol and air bombing.
    assert calls(original, 0x46C93B, 0x46C9D3) == [
        "0x6169a0", "0x5f89d0", "0x6169a0", "0x5f89d0"]


def test_distance_reloads_and_cursor_edit_split(original):
    expect(original, {
        0x46CA29: ("call", "0x5c0db0"),
        0x46CA47: ("movsx", "ebx, word ptr [eax + 0x95282a]"),
        0x46CA4E: ("movsx", "eax, word ptr [eax + 0x952828]"),
        0x46CA66: ("test", "cl, 1"),
        0x46CA69: ("jne", "0x46ca7d"),
        0x46CA92: ("sar", "eax, 1"),
        0x46CACC: ("movsx", "ebx, word ptr [eax + 0x95282a]"),
        0x46CAEB: ("test", "cl, 1"),
        0x46CAEE: ("jne", "0x46cb02"),
        0x46CB14: ("and", "al, 0xfe"),
        0x46CBE1: ("mov", "eax, dword ptr [0x939288]"),
        0x46CBE8: ("je", "0x46cc05"),
        0x46CC01: ("push", "4"),
        0x46CC03: ("jmp", "0x46cc2d"),
        0x46CC0A: ("call", "0x4e1f40"),
        0x46CC11: ("jne", "0x46cc38"),
        0x46CC2B: ("push", "3"),
    })


def test_actual_indexed_dispatch_tables(original):
    image, _ = original
    tables = {
        0x46D554: (0x46C36D, 0x46C3A2, 0x46C3BF, 0x46C3D8, 0x46C3E7, 0x46C3F7),
        0x46D56C: (0x46CFA6, 0x46D025, 0x46D054, 0x46D091, 0x46D0BE,
                   0x46D0D3, 0x46D115, 0x46D144, 0x46D1B8, 0x46D1AE, 0x46D1DF),
        0x46D598: (0x46D257, 0x46D284, 0x46D2B6, 0x46D2E8, 0x46D31A, 0x46D34C),
        0x46D5B0: (0x46D3BE, 0x46D3E8, 0x46D413, 0x46D43C, 0x46D465),
    }
    for start, targets in tables.items():
        assert struct.unpack(f"<{len(targets)}I", image.read(Span(start, start + 4*len(targets)))) == targets
    assert calls(original, 0x46D206, 0x46D235) == ["0x41ae40", "0x417040"]
    expect(original, {
        0x46D206: ("test", "edi, edi"),
        0x46D20E: ("test", "esi, esi"),
        0x46D224: ("push", "esi"),
        0x46D225: ("push", "esi"),
        0x46D404: ("mov", "ecx, 0x93cd90"),
        0x46D409: ("call", "0x532140"),
        0x46D1DF: ("test", "edi, edi"),
        0x46D1E7: ("test", "esi, esi"),
        0x46D172: ("mov", "eax, dword ptr [0x93928c]"),
    })
    for path, address, symbol in (
        ("basewin.cpp", 0x417040, "?production@BaseWin@@QAEXHH@Z"),
        ("netdaemon.cpp", 0x532140, "?veh_drop@NetDaemon@@QAEHHHH@Z"),
    ):
        record, = [r for r in read_file(ROOT / "src" / path) if r.address == address]
        assert (record.symbol or record.name) == symbol


def test_original_action_lookups_have_no_added_bounds_fallback(original):
    # Observe unchecked native blocks; do NOT execute out-of-bounds host C++
    # or claim malformed-state reachability/fault equivalence from this.
    for start, end in ((0x46D091, 0x46D0BE), (0x46D0BE, 0x46D0D3),
                       (0x46D372, 0x46D39C), (0x46D3BE, 0x46D3E8),
                       (0x46CFB6, 0x46CFC5)):
        block = [i for a, i in original[1].items() if start <= a < end]
        assert not any(i.mnemonic.startswith("j") and i.mnemonic != "jmp" for i in block)
    assert calls(original, 0x46D372, 0x46D39C) == ["0x5fb990", "0x5f9170"]
    assert calls(original, 0x46D3BE, 0x46D3E8) == ["0x5fb990", "0x5f9170"]
    expect(original, {
        0x46C292: ("call", "0x5f9f40"),
        0x46C29A: ("cmp", "eax, 0x40"),
        0x46C29D: ("jge", "0x46c2ab"),
        0x46CE8A: ("call", "0x5f9f40"),
        0x46CE8F: ("cmp", "eax, 0x40"),
        0x46CE92: ("jge", "0x46cea0"),
    })
