"""Datalink renderers retain only their original explicit list clears."""
from __future__ import annotations

import hashlib
import os
from pathlib import Path

from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from match_recovery.pe_coff import PeImage, Span
import pytest


ROOT = Path(__file__).resolve().parents[2]
IMAGE = Path(os.environ.get(
    "OPENSMACX_IMAGE",
    str(ROOT / ".opensmacx/game/terranx_original.exe"),
))
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"


def definition(text: str, signature: str) -> str:
    assert text.count(signature) == 1
    start = text.index(signature)
    opening = text.index("{", start + len(signature))
    depth = 0
    quote = None
    escaped = False
    for pos in range(opening, len(text)):
        ch = text[pos]
        if quote:
            if escaped:
                escaped = False
            elif ch == "\\":
                escaped = True
            elif ch == quote:
                quote = None
        elif ch in "\"'":
            quote = ch
        elif ch == "{":
            depth += 1
        elif ch == "}":
            depth -= 1
            if depth == 0:
                return text[start:pos + 1]
    raise AssertionError(f"unterminated definition: {signature}")


def test_renderer_explicit_clear_counts_and_source_order():
    source = (ROOT / "src/datalink.cpp").read_text(encoding="utf-8")
    bodies = {
        name: definition(source, f"void Datalink::{name}()")
        for name in (
            "draw_unit", "draw_chassis", "draw_reactor", "draw_weapon",
            "draw_defense", "draw_ability",
        )
    }
    # These are the caller-owned clears. StringList's derived/base destructor
    # stages remain implicit and are separately witnessed in the pinned spans.
    assert {name: body.count(".remove_all();")
            for name, body in bodies.items()} == {
        "draw_unit": 1,
        "draw_chassis": 1,
        "draw_reactor": 2,
        "draw_weapon": 1,
        "draw_defense": 1,
        "draw_ability": 1,
    }

    chassis = bodies["draw_chassis"]
    assert chassis.index("description.remove_all();") < chassis.index(
        "datalink_vehicle_line_start")
    assert chassis.index("soft_update(&description_rect_);") < chassis.rindex("}")

    weapon = bodies["draw_weapon"]
    assert weapon.index("description.remove_all();") < weapon.index(
        "description.load(")
    assert weapon.index("description.load(") < weapon.index("write_strings(")
    assert weapon.index("write_strings(") < weapon.index('"WEAPONDESC"')
    assert weapon.index('"WEAPONDESC"') < weapon.index(
        "soft_update(&description_rect_);")
    # No explicit reset is inserted after rendering; the real object teardown
    # remains the original two-stage lifetime boundary.
    assert "description.remove_all();" not in weapon[
        weapon.index("write_strings("):]


def test_pinned_chassis_and_weapon_cleanup_stages():
    if not IMAGE.is_file():
        pytest.skip(f"pinned image unavailable: {IMAGE}")
    raw = IMAGE.read_bytes()
    assert hashlib.sha256(raw).hexdigest() == PIN
    image = PeImage(raw, PIN)
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)

    def function_instructions(lo: int, hi: int):
        code = image.read(Span(lo, hi))
        decoded = list(decoder.disasm(code, lo))
        assert sum(item.size for item in decoded) == hi - lo
        return {item.address: (item.mnemonic, item.op_str) for item in decoded}

    # Decode complete mapped code spans from their original function entries,
    # so both presence and absence of direct calls are bounded by the owner.
    chassis = function_instructions(0x42D610, 0x42DEEF)
    assert [address for address, (mnemonic, operand) in chassis.items()
            if mnemonic == "call" and operand == "0x402970"] == [0x42D7CA]
    assert {
        address: chassis[address]
        for address in (0x42D7CA, 0x42DD96, 0x42DD9E, 0x42DE27)
    } == {
        0x42D7CA: ("call", "0x402970"),  # one explicit pre-render clear
        0x42DD96: ("call", "0x5d5930"),  # soft_update
        0x42DD9E: ("mov", "dword ptr [ebp - 0x40], 0x6698c4"),
        0x42DE27: ("mov", "dword ptr [ebp - 0x40], 0x6693a4"),
    }

    weapon = function_instructions(0x42E2E0, 0x42E8D3)
    assert [address for address, (mnemonic, operand) in weapon.items()
            if mnemonic == "call" and operand == "0x402970"] == []
    assert {
        address: weapon[address]
        for address in (
            0x42E60C, 0x42E668, 0x42E6F6, 0x42E77A, 0x42E782, 0x42E80B,
        )
    } == {
        0x42E60C: ("jle", "0x42e668"),  # bounded pre-load inline walk
        0x42E668: ("mov", "dword ptr [ebp - 0x38], ebx"),
        0x42E6F6: ("call", "0x5db040"),  # write_strings
        0x42E77A: ("call", "0x5d5930"),  # soft_update
        0x42E782: ("mov", "dword ptr [ebp - 0x40], 0x6698c4"),
        0x42E80B: ("mov", "dword ptr [ebp - 0x40], 0x6693a4"),
    }
