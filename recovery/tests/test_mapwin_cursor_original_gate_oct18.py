"""Pinned original cursor-family gate, action table and cleanup witnesses.

This is bounded instruction/source-binding evidence, not proof of the complete
MapWin body, generated lifetime equivalence, linked retention or game behavior.
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
BODY = Span(0x0046D5D0, 0x0046EB68)
TABLE = Span(0x0046EB68, 0x0046EB90)


@pytest.fixture(scope="module")
def original():
    path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not path.is_file():
        pytest.skip("pinned original unavailable; no original-instruction evidence")
    data = path.read_bytes()
    assert hashlib.sha256(data).hexdigest() == PIN
    image = PeImage(data, PIN)
    raw = image.read(BODY)
    instructions = list(Cs(CS_ARCH_X86, CS_MODE_32).disasm(raw, BODY.start))
    assert sum(i.size for i in instructions) == BODY.end - BODY.start
    assert all(a.address + a.size == b.address for a, b in zip(instructions, instructions[1:]))
    return image, {i.address: i for i in instructions}


def test_original_gate_distinguishes_zero_negative_vehicle_and_table_default(original):
    _, instructions = original
    expected = {
        0x46E11F: ("mov", "edi, dword ptr [ebp - 0x24]"),
        0x46E122: ("test", "edi, edi"),
        0x46E124: ("je", "0x46e299"),
        0x46E12A: ("mov", "eax, dword ptr [0x93928c]"),
        0x46E12F: ("test", "eax, eax"),
        0x46E131: ("mov", "dword ptr [ebp - 0x18], eax"),
        0x46E134: ("jge", "0x46e1af"),
        0x46E1AF: ("lea", "esi, [edi - 2]"),
        0x46E1B2: ("cmp", "esi, 9"),
        0x46E1B5: ("ja", "0x46e216"),
        0x46E1B7: ("jmp", "dword ptr [esi*4 + 0x46eb68]"),
        0x46E216: ("mov", "dword ptr [0x939294], 0"),
    }
    for address, pair in expected.items():
        instruction = instructions[address]
        assert (instruction.mnemonic, instruction.op_str) == pair, hex(address)
    record, = [r for r in read_file(ROOT / "src/mapwin.cpp") if r.address == BODY.start]
    assert (BODY.start, BODY.end) in record.image_spans
    assert (TABLE.start, TABLE.end) not in record.image_spans  # No invented function extent for associated data.


def test_original_table_is_not_the_linear_order_of_action_blocks(original):
    image, instructions = original
    entries = struct.unpack("<10I", image.read(TABLE))
    assert entries == (0x46E1CD, 0x46E1DE, 0x46E1BE, 0x46E216, 0x46E216,
                       0x46E216, 0x46E1EF, 0x46E216, 0x46E1FC, 0x46E209)
    expected = {
        0x46E1BE: (0x532140, ["edx", "ecx", "eax"], "0x93cd90"),
        0x46E1CD: (0x4D8DC0, ["0x1b", "edx", "ecx", "eax"], "0x9156b0"),
        0x46E1DE: (0x4D8DC0, ["0x1c", "edx", "ecx", "eax"], "0x9156b0"),
        0x46E1EF: (0x4CDA30, ["edx", "ecx", "eax"], None),
        0x46E1FC: (0x4CD090, ["edx", "ecx", "eax"], None),
        0x46E209: (0x4D8CD0, ["edx", "ecx", "eax"], "0x9156b0"),
    }
    for start, (callee, pushes, receiver) in expected.items():
        block = []
        at = start
        while at < 0x46E216:
            instruction = instructions[at]
            block.append(instruction)
            if instruction.mnemonic == "call":
                break
            at += instruction.size
        assert [i.op_str for i in block if i.mnemonic == "push"] == pushes
        assert block[-1].mnemonic == "call" and block[-1].op_str == hex(callee)
        receivers = [i.op_str for i in block if i.mnemonic == "mov"]
        assert receivers == ([] if receiver is None else ["ecx, " + receiver])


def test_original_negative_vehicle_and_order_paths_share_cleanup_not_reset(original):
    _, instructions = original
    cleanup_calls = ["0x404900", "0x605370", "0x406880", "0x406880"]
    for start, end, exit_address in ((0x46E136, 0x46E1AF, 0x46E1AA),
                                    (0x46E220, 0x46E299, 0x46E294)):
        block = [i for address, i in instructions.items() if start <= address < end]
        assert [i.op_str for i in block if i.mnemonic == "call"] == cleanup_calls
        assert not any("0x939294" in i.op_str for i in block)
        assert (instructions[exit_address].mnemonic, instructions[exit_address].op_str) == ("jmp", "0x46eb38")
    assert [(instructions[a].mnemonic, instructions[a].op_str)
            for a in (0x46EB3E, 0x46EB50, 0x46EB65)] == [
                ("call", "0x5d4dd0"), ("call", "0x4064d0"), ("ret", "0xc")]
    for path, address, symbol in (("popup.cpp", 0x404900, "?close@Popup@@QAEXXZ"),
                                  ("scroll.cpp", 0x605370, "?close@Scroll@@QAEXXZ"),
                                  ("flatbutton.cpp", 0x406880, "??1FlatButton@@UAE@XZ")):
        record, = [r for r in read_file(ROOT / "src" / path) if r.address == address]
        assert (record.symbol or record.name) == symbol
