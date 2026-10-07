"""Actual-source and pinned-callsite controls for veh.cpp tutorial ownership."""

from __future__ import annotations

import re
from pathlib import Path

from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from capstone.x86_const import X86_OP_IMM, X86_OP_REG, X86_REG_ECX

from match_recovery.model import Span
from match_recovery.pe_coff import PeImage
from match_recovery.source_inventory import image_pin


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/veh.cpp").is_file())
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"


def _definition(text: str, signature: str) -> str:
    masked = list(text)
    index = 0
    state = "code"
    quote = ""
    while index < len(text):
        if state == "code":
            if text.startswith("//", index):
                masked[index:index + 2] = "  "
                index += 2
                state = "line"
            elif text.startswith("/*", index):
                masked[index:index + 2] = "  "
                index += 2
                state = "block"
            elif text[index] in "\"'":
                quote = text[index]
                masked[index] = " "
                index += 1
                state = "literal"
            else:
                index += 1
        elif state == "line":
            if text[index] == "\n":
                state = "code"
            else:
                masked[index] = " "
            index += 1
        elif state == "block":
            if text.startswith("*/", index):
                masked[index:index + 2] = "  "
                index += 2
                state = "code"
            else:
                if text[index] != "\n":
                    masked[index] = " "
                index += 1
        else:
            if text[index] == "\\":
                masked[index] = " "
                if index + 1 < len(text) and text[index + 1] != "\n":
                    masked[index + 1] = " "
                index += 2
            elif text[index] == quote:
                masked[index] = " "
                index += 1
                state = "code"
            else:
                if text[index] != "\n":
                    masked[index] = " "
                index += 1
    code = "".join(masked)
    hits = list(re.finditer(re.escape(signature), code))
    assert len(hits) == 1, (signature, len(hits))
    start = hits[0].start()
    opening = code.find("{", hits[0].end())
    depth = 0
    for position in range(opening, len(code)):
        if code[position] == "{":
            depth += 1
        elif code[position] == "}":
            depth -= 1
            if depth == 0:
                return text[start:position + 1]
    raise AssertionError(f"unterminated source definition: {signature}")


def _instruction(image: PeImage, address: int, size: int):
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    decoded = list(decoder.disasm(image.read(Span(address, address + size)), address))
    assert len(decoded) == 1 and decoded[0].size == size
    return decoded[0]


def test_actual_terraform_owner_calls_the_recovered_window_members():
    veh = (ROOT / "src/veh.cpp").read_text()
    body = _definition(veh, "int __cdecl action_terraform(")

    assert body.count("TutWinBaseWindow.check_veh(veh_id);") == 2
    assert body.count("TutorialWindow.tut_map(") == 1
    assert body.count(
        "TutorialWindow.tut_map(msg, x, y, noun, sprite, 1, -1, -1);"
    ) == 1
    assert "Sprite *sprite = 0;" in body
    assert "const_cast<void *>(sprite)" not in body
    for stale in ("BaseWinOriginalCheckVeh", "TutWinOriginalTutMap",
                  "ORIGINAL(", "original_method<", "original_seam.h"):
        assert stale not in veh, stale

    basewin_h = (ROOT / "src/basewin.h").read_text()
    basewin_cpp = (ROOT / "src/basewin.cpp").read_text()
    tutwin_h = (ROOT / "src/tutwin.h").read_text()
    tutwin_cpp = (ROOT / "src/tutwin.cpp").read_text()
    assert re.search(r"\bvoid\s+check_veh\s*\(\s*int\s+a1\s*\)\s*;", basewin_h)
    assert "void BaseWin::check_veh(int veh_id)" in basewin_cpp
    assert re.search(
        r"\bint\s+tut_map\s*\(\s*const char \*text,.*?Sprite \*sprite,",
        tutwin_h, re.S,
    )
    assert "int TutWin::tut_map(const char *text" in tutwin_cpp
    assert "BaseWin TutWinBaseWindow;  // 0x006A7628" in basewin_cpp
    assert "RECOVERY_OBJECT: v1 0x006A7628 ?TutWinBaseWindow@@3VBaseWin@@A" in basewin_cpp
    assert "TutWin TutorialWindow;" in tutwin_cpp
    assert "RECOVERY_OBJECT: v2 0x008C6E68 0x008CC240 ?TutorialWindow@@3VTutWin@@A" in tutwin_cpp


def test_pinned_callers_use_the_same_typed_owner_receivers():
    assert image_pin(ROOT) == PIN
    image = PeImage((ROOT / ".opensmacx/game/terranx_original.exe").read_bytes(),
                    image_pin(ROOT))

    # Both BaseWin calls load the real singleton receiver before the direct
    # target call; the map tutorial call loads TutorialWindow's object address.
    sites = (
        (0x004CA02C, 0x0041A8B0, 0x004CA026, 0x006A7628),
        (0x004CA521, 0x0041A8B0, 0x004CA51B, 0x006A7628),
        (0x004CA70E, 0x004BE650, 0x004CA709, 0x008C6E68),
    )
    for call_at, target, receiver_at, receiver in sites:
        call = _instruction(image, call_at, 5)
        assert call.mnemonic == "call"
        assert call.operands[0].type == X86_OP_IMM
        assert call.operands[0].imm == target
        load = _instruction(image, receiver_at, 5)
        assert load.mnemonic == "mov"
        assert load.operands[0].type == X86_OP_REG
        assert load.operands[0].reg == X86_REG_ECX
        assert load.operands[1].type == X86_OP_IMM
        assert load.operands[1].imm == receiver
