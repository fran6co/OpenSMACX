"""Pinned caller/callee stack proof for the Caviar shadow visibility value.

This checks original PE instructions from independently rooted complete spans;
it does not establish native candidate code generation or rendering behavior.
"""
from __future__ import annotations

import hashlib
import os
from pathlib import Path

from capstone import Cs, CS_ARCH_X86, CS_MODE_32

from match_recovery.model import Span
from match_recovery.pe_coff import PeImage
from match_recovery.source_inventory import image_pin


ROOT = Path(__file__).resolve().parents[2]
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
DRAW_SCENE = Span(0x00639E60, 0x0063A279)
DRAW_SHADOW = Span(0x0063F000, 0x0063F5D7)


def _original() -> PeImage:
    assert image_pin(ROOT) == PIN
    configured = os.environ.get("OPENSMACX_IMAGE")
    path = (Path(configured).expanduser().resolve() if configured else
            ROOT / ".opensmacx/game/terranx_original.exe")
    assert path.is_file(), f"pinned original required: set OPENSMACX_IMAGE ({path})"
    raw = path.read_bytes()
    assert hashlib.sha256(raw).hexdigest() == PIN
    return PeImage(raw, PIN)


def _complete_entry_decode(image: PeImage, span: Span):
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    instructions = tuple(decoder.disasm(image.read(span), span.start))
    cursor = span.start
    for instruction in instructions:
        assert instruction.address == cursor, hex(cursor)
        cursor += instruction.size
    assert cursor == span.end
    assert instructions and instructions[0].address == span.start
    return {instruction.address: instruction for instruction in instructions}


def _pair(instructions, address: int) -> tuple[str, str]:
    instruction = instructions[address]
    return instruction.mnemonic, instruction.op_str


def test_scene_passes_the_fifth_cdecl_argument_as_shadow_visibility():
    image = _original()
    scene = _complete_entry_decode(image, DRAW_SCENE)
    shadow = _complete_entry_decode(image, DRAW_SHADOW)

    assert _pair(scene, 0x00639E65) == ("sub", "esp, 0x44")
    assert [_pair(scene, address) for address in (
        0x00639E6A, 0x00639E6B, 0x00639E6C, 0x00639E6D
    )] == [("push", register) for register in ("ebx", "ebp", "esi", "edi")]
    assert _pair(scene, 0x00639FA9) == ("mov", "dword ptr [esp + 0x10], 1")
    assert _pair(scene, 0x00639FC7) == ("jne", "0x639fd1")
    assert _pair(scene, 0x00639FC9) == ("mov", "dword ptr [esp + 0x10], 0")

    # At 0x63A1E0 the three matrix-multiply arguments are still on the stack.
    # The load at +0x1C is therefore the ordinary-frame +0x10 local; after
    # their 12-byte cleanup that value is preserved in ECX and pushed first,
    # i.e. as cdecl argument five to vox_draw_shadow.
    assert _pair(scene, 0x0063A1CF) == ("push", "edx")
    assert _pair(scene, 0x0063A1DA) == ("push", "eax")
    assert _pair(scene, 0x0063A1DB) == ("push", "0x9c0b2c")
    assert _pair(scene, 0x0063A1E0) == ("call", "0x627e20")
    visibility_load = scene[0x0063A1EC]
    assert _pair(scene, visibility_load.address) == ("mov", "ecx, dword ptr [esp + 0x1c]")
    assert _pair(scene, 0x0063A1F0) == ("add", "esp, 0xc")
    assert 0x1C - 3 * 4 == 0x10

    # Push order is reverse parameter order: visibility, matrix scratch,
    # projected point, shape, then record. No pointer-shaped sentinel is
    # involved in the fifth stack argument.
    assert [scene[address].op_str for address in (
        0x0063A1FB, 0x0063A1FC, 0x0063A208, 0x0063A209, 0x0063A20A
    )] == ["ecx", "edx", "eax", "esi", "edi"]
    assert _pair(scene, 0x0063A23B) == ("call", "0x63f000")
    assert _pair(scene, 0x0063A240) == ("add", "esp, 0x14")

    # Callee entry is independently decoded at 0x63F000. Its local frame plus
    # four saved registers is 0x68 bytes. After its one helper argument is
    # removed, [esp+0x7C] is entry ESP+0x14, the fifth cdecl argument.
    assert _pair(shadow, 0x0063F000) == ("sub", "esp, 0x58")
    assert [shadow[address].op_str for address in (
        0x0063F003, 0x0063F004, 0x0063F009, 0x0063F00A
    )] == ["ebx", "ebp", "esi", "edi"]
    assert _pair(shadow, 0x0063F026) == ("add", "esp, 4")
    visibility = shadow[0x0063F0C1]
    assert _pair(shadow, visibility.address) == ("mov", "esi, dword ptr [esp + 0x7c]")
    assert 0x7C - (0x58 + 4 * 4) == 0x14
    assert _pair(shadow, 0x0063F02B) == ("xor", "edi, edi")
    assert _pair(shadow, 0x0063F0CB) == ("cmp", "esi, edi")
    assert _pair(shadow, 0x0063F0D5) == ("setne", "bl")
    assert _pair(shadow, 0x0063F0DA) == ("mov", "dword ptr [esp + 0x7c], ebx")
