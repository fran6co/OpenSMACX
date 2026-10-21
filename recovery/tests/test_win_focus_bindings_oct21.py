"""Source and pinned-image evidence for Win focus objects.

The five annotations are start-only identities. This test does not infer
extent, initialization, lifetime, compiler selection, or full reference proof.
"""

from hashlib import sha256
import os
from pathlib import Path
import re

import pytest

from match_recovery.model import Span
from match_recovery.object_source import parse_objects
from match_recovery.pe_coff import PeImage


ROOT = Path(__file__).resolve().parents[2]
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
EXPECTED = {
    0x009B8D7C: ("?WinModalFocus@@3PAVWin@@A", "Win *", "WinModalFocus"),
    0x009B7AC4: ("?WinInputFocus@@3PAVWin@@A", "Win *", "WinInputFocus"),
    0x009B7AC8: ("?WinActiveWindow@@3PAVWin@@A", "Win *", "WinActiveWindow"),
    0x009B7AEC: ("?WinDefaultFocus@@3PAVWin@@A", "Win *", "WinDefaultFocus"),
    0x009B7A8C: ("?WinKeyHook@@3P6AXI@ZA", "callback", "WinKeyHook"),
}


def _sources():
    source_path = ROOT / "src/win.cpp"
    header_path = ROOT / "src/win.h"
    source = source_path.read_text(encoding="utf-8")
    header = header_path.read_text(encoding="utf-8")
    bindings = parse_objects(source, "src/win.cpp", PIN, address_space="pe:va")
    return source, header, bindings


def test_focus_object_bindings_are_unique_unbounded_v1_beside_typed_definitions():
    source, header, bindings = _sources()
    selected = [row for row in bindings if row.origin.address in EXPECTED]
    assert len(selected) == len(EXPECTED)
    assert {row.origin.address for row in selected} == set(EXPECTED)
    assert len({row.symbol for row in selected}) == len(EXPECTED)

    for row in selected:
        symbol, declared_type, name = EXPECTED[row.origin.address]
        assert row.symbol == symbol
        assert row.unit == "src/win.cpp"
        assert row.origin.image_sha256 == PIN
        assert row.origin.address_space == "pe:va"
        assert row.span is None and row.evidence is None
        assert row.parent_symbol is None and row.target_symbol is None

        lines = source.splitlines()
        annotation = lines[row.line - 1]
        assert annotation == f"// RECOVERY_OBJECT: v1 0x{row.origin.address:08X} {symbol}"
        assert re.fullmatch(r"// RECOVERY_OBJECT: v1 0x[0-9A-Fa-f]+ \S+", annotation)
        definition = lines[row.line]
        if declared_type == "callback":
            assert re.match(r"void\s*\(\s*__cdecl\s*\*\s*WinKeyHook\s*\)\s*\(WPARAM key\);", definition)
            header_decl = re.search(r"(?m)^extern\s+void\s*\(\s*__cdecl\s*\*\s*WinKeyHook\s*\)\s*\(WPARAM key\);", header)
        else:
            assert re.match(r"Win\s*\*\s*" + re.escape(name) + r"\s*;", definition)
            header_decl = re.search(r"(?m)^extern\s+Win\s*\*\s*" + re.escape(name) + r"\s*;", header)
        assert header_decl

    # No second source annotation may claim one of these identities, and no
    # different annotation may alias one of their original starts.
    for address, (symbol, _, _) in EXPECTED.items():
        assert sum(row.origin.address == address for row in bindings) == 1
        assert sum(row.symbol == symbol for row in bindings) == 1


def _pinned_image():
    configured = os.environ.get("OPENSMACX_IMAGE")
    candidates = ([Path(configured)] if configured else []) + [
        ROOT / ".opensmacx/game/terranx_original.exe",
        Path("/home/fran6co/code/OpenSMACX/.opensmacx/game/terranx_original.exe"),
    ]
    path = next((candidate for candidate in candidates if candidate.is_file()), None)
    if path is None:
        pytest.skip("pinned original image is absent; source binding checks still run")
    raw = path.read_bytes()
    assert sha256(raw).hexdigest() == PIN, "unexpected image supplied for pinned evidence test"
    return PeImage(raw, PIN)


def _instructions(image, start, end):
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32

    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    return {
        instruction.address: instruction
        for instruction in decoder.disasm(image.read(Span(start, end)), start)
    }


def _assert_abs_dword(ins, *, destination_reg, address):
    from capstone.x86 import X86_OP_MEM, X86_OP_REG, X86_REG_INVALID

    assert ins.mnemonic == "mov"
    assert len(ins.operands) == 2
    destination, source = ins.operands
    assert destination.type == X86_OP_REG
    assert ins.reg_name(destination.reg) == destination_reg
    assert source.type == X86_OP_MEM and source.size == 4
    assert source.mem.base == X86_REG_INVALID
    assert source.mem.index == X86_REG_INVALID
    assert source.mem.disp == address


def _assert_abs_store(ins, *, source_reg, address):
    from capstone.x86 import X86_OP_MEM, X86_OP_REG, X86_REG_INVALID

    assert ins.mnemonic == "mov" and len(ins.operands) == 2
    destination, source = ins.operands
    assert destination.type == X86_OP_MEM and destination.size == 4
    assert destination.mem.base == X86_REG_INVALID
    assert destination.mem.index == X86_REG_INVALID
    assert destination.mem.disp == address
    assert source.type == X86_OP_REG and ins.reg_name(source.reg) == source_reg


def _call_target(image, address):
    raw = image.read(Span(address, address + 5))
    assert raw[0] == 0xE8
    return address + 5 + int.from_bytes(raw[1:], "little", signed=True)


def test_pinned_focus_operands_and_parent_receiver():
    image = _pinned_image()
    from capstone.x86 import X86_OP_IMM, X86_OP_MEM, X86_OP_REG

    load_witnesses = {
        0x005F15CA: ("eax", 0x009B8D7C),  # OnKey modal
        0x005F15D7: ("eax", 0x009B7AC4),  # OnKey input
        0x005F15E0: ("eax", 0x009B7AEC),  # OnKey default
        0x005F15F0: (None, 0x009B7AC8),    # OnKey active store
        0x005F166A: ("eax", 0x009B8D7C),  # OnChar modal
        0x005F1677: ("eax", 0x009B7AC4),
        0x005F1680: ("eax", 0x009B7AEC),
        0x005F1688: (None, 0x009B7AC8),
        0x005F16DA: ("eax", 0x009B8D7C),  # OnSysKey independently decoded
        0x005F16E7: ("eax", 0x009B7AC4),
        0x005F16F0: ("eax", 0x009B7AEC),
        0x005F16F8: (None, 0x009B7AC8),
    }
    for address, (destination, target) in load_witnesses.items():
        ins = _instructions(image, address, address + 8)[address]
        if destination is not None:
            _assert_abs_dword(ins, destination_reg=destination, address=target)
        else:
            assert ins.mnemonic == "mov" and len(ins.operands) == 2
            assert ins.operands[0].type == X86_OP_MEM and ins.operands[0].size == 4
            assert ins.operands[0].mem.base == 0 and ins.operands[0].mem.disp == target
            assert ins.operands[1].type == X86_OP_REG and ins.reg_name(ins.operands[1].reg) == "eax"
    _assert_abs_store(_instructions(image, 0x005F2CE4, 0x005F2CE9)[0x005F2CE4],
                      source_reg="eax", address=0x009B7AEC)

    # OnKey reads the opaque callback pointer, gates it on the native flag
    # argument, then invokes that pointer with the original key argument.
    key = _instructions(image, 0x005F1635, 0x005F1648)
    _assert_abs_dword(key[0x005F1635], destination_reg="eax", address=0x009B7A8C)
    assert key[0x005F163A].mnemonic == "test"
    assert [key[0x005F163A].reg_name(op.reg) for op in key[0x005F163A].operands] == ["eax", "eax"]
    assert key[0x005F163E].mnemonic == "test"
    assert [key[0x005F163E].reg_name(op.reg) for op in key[0x005F163E].operands] == ["edi", "edi"]
    assert key[0x005F1643].mnemonic == "call"
    assert key[0x005F1643].operands[0].type == X86_OP_REG
    assert key[0x005F1643].reg_name(key[0x005F1643].operands[0].reg) == "eax"
    assert key[0x005F1642].mnemonic == "push" and key[0x005F1642].op_str == "ebx"

    # DiploPop loads and null-tests the same modal-focus cell, then dispatches
    # through the selected Win object's vtable slot +0xB0 with its argument.
    diplo = _instructions(image, 0x0043F293, 0x0043F2AA)
    _assert_abs_dword(diplo[0x0043F293], destination_reg="ecx", address=0x009B8D7C)
    assert diplo[0x0043F299].mnemonic == "test"
    assert [diplo[0x0043F299].reg_name(op.reg) for op in diplo[0x0043F299].operands] == ["ecx", "ecx"]
    assert diplo[0x0043F2A0].mnemonic == "mov"
    assert diplo[0x0043F2A0].reg_name(diplo[0x0043F2A0].operands[0].reg) == "eax"
    assert diplo[0x0043F2A0].operands[1].type == X86_OP_MEM
    assert diplo[0x0043F2A0].operands[1].size == 4
    assert diplo[0x0043F2A0].reg_name(diplo[0x0043F2A0].operands[1].mem.base) == "ecx"
    assert diplo[0x0043F2A0].operands[1].mem.disp == 0
    assert diplo[0x0043F2A3].mnemonic == "call"
    assert diplo[0x0043F2A3].operands[0].type == X86_OP_MEM
    assert diplo[0x0043F2A3].operands[0].size == 4
    assert diplo[0x0043F2A3].reg_name(diplo[0x0043F2A3].operands[0].mem.base) == "eax"
    assert diplo[0x0043F2A3].operands[0].mem.disp == 0xB0
    assert diplo[0x0043F2A2].mnemonic == "push" and diplo[0x0043F2A2].op_str == "edx"

    # The branch in get_key_window tests the base object's flag, loads its
    # parent as ECX, and directly calls Win::is_visible on that parent.
    instructions = _instructions(image, 0x005F6A50, 0x005F6AB0)
    assert instructions[0x005F6A82].mnemonic == "mov"
    assert instructions[0x005F6A82].reg_name(instructions[0x005F6A82].operands[0].reg) == "esi"
    assert instructions[0x005F6A82].operands[1].type == X86_OP_MEM
    assert instructions[0x005F6A82].operands[1].size == 4
    assert instructions[0x005F6A82].reg_name(instructions[0x005F6A82].operands[1].mem.base) == "esi"
    assert instructions[0x005F6A82].operands[1].mem.disp == 0xA8
    assert instructions[0x005F6A8C].mnemonic == "test"
    assert instructions[0x005F6A8C].operands[0].type == X86_OP_MEM
    assert instructions[0x005F6A8C].operands[0].size == 1
    assert instructions[0x005F6A8C].reg_name(instructions[0x005F6A8C].operands[0].mem.base) == "esi"
    assert instructions[0x005F6A8C].operands[0].mem.disp == 0x9C
    assert instructions[0x005F6A8C].operands[1].type == X86_OP_IMM
    assert instructions[0x005F6A8C].operands[1].imm == 1
    assert instructions[0x005F6A95].mnemonic == "mov"
    assert instructions[0x005F6A95].reg_name(instructions[0x005F6A95].operands[0].reg) == "ecx"
    assert instructions[0x005F6A95].operands[1].type == X86_OP_MEM
    assert instructions[0x005F6A95].operands[1].size == 4
    assert instructions[0x005F6A95].reg_name(instructions[0x005F6A95].operands[1].mem.base) == "esi"
    assert instructions[0x005F6A95].operands[1].mem.disp == 0xC4
    assert _call_target(image, 0x005F6A9F) == 0x005F7E90
    assert instructions[0x005F6AAC].mnemonic == "mov"
    assert instructions[0x005F6AAC].reg_name(instructions[0x005F6AAC].operands[0].reg) == "eax"
    assert instructions[0x005F6AAC].reg_name(instructions[0x005F6AAC].operands[1].reg) == "esi"
    callee = _instructions(image, 0x005F7E90, 0x005F7F10)
    recursive_parent = callee[0x005F7E9C]
    assert recursive_parent.mnemonic == "mov"
    assert recursive_parent.reg_name(recursive_parent.operands[0].reg) == "ecx"
    assert recursive_parent.operands[1].type == X86_OP_MEM
    assert recursive_parent.reg_name(recursive_parent.operands[1].mem.base) == "ecx"
    assert recursive_parent.operands[1].mem.disp == 0xC4


def test_pinned_window_proc_has_five_focus_helper_call_paths():
    image = _pinned_image()
    expected_calls = (0x005F07B0, 0x005F07F0, 0x005F0821, 0x005F086D, 0x005F08A3)
    assert tuple(_call_target(image, address) for address in expected_calls) == (0x005F6A50,) * 5

    source, _, _ = _sources()
    start = source.index("LRESULT __stdcall Win::window_proc(")
    body = source.index("{", start)
    depth = 0
    end = body
    for end in range(body, len(source)):
        if source[end] == "{":
            depth += 1
        elif source[end] == "}":
            depth -= 1
            if depth == 0:
                end += 1
                break
    window_proc = source[start:end]
    assert window_proc.count("get_key_window()") == 5
    for route in ("case WM_KEYDOWN", "case WM_KEYUP", "WM_CHAR",
                  "case WM_SYSKEYDOWN", "case WM_SYSKEYUP"):
        assert route in window_proc

