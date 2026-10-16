"""Pinned operand evidence for the alien-fauna continent threshold."""

from pathlib import Path
import os

import pytest

ROOT = Path(__file__).resolve().parents[2]


def test_full_pinned_fauna_span_uses_continent_tile_count_and_signed_gate():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    from match_recovery.source import read_file
    from match_recovery.source_inventory import image_pin

    image_path = Path(os.environ.get(
        "OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not image_path.is_file():
        pytest.skip("pinned original required for instruction verification")
    record, = [item for item in read_file(ROOT / "src/game.cpp")
               if item.address == 0x00522180]
    assert record.image_spans == ((0x00522180, 0x00522EFC),)
    image = PeImage(image_path.read_bytes(), image_pin(ROOT))
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    low, high = record.image_spans[0]
    instructions = list(decoder.disasm(image.read(Span(low, high)), low))
    by_address = {}
    cursor = low
    for instruction in instructions:
        assert instruction.address == cursor, f"undecoded byte at {cursor:#x}"
        by_address[cursor] = instruction
        cursor += instruction.size
    assert cursor == high, "the complete mapped alien_fauna span must decode"

    # EDI is masked to a continent selector. 7*region is then scaled by four,
    # yielding the 28-byte Continent stride from array base 0x9AA730.
    assert (by_address[0x005226F0].mnemonic,
            by_address[0x005226F0].op_str) == ("and", "eax, 0x3f")
    assert (by_address[0x00522704].mnemonic,
            by_address[0x00522704].op_str) == ("lea", "eax, [edi*8]")
    assert (by_address[0x0052270B].mnemonic,
            by_address[0x0052270B].op_str) == ("sub", "eax, edi")
    assert (by_address[0x0052270D].mnemonic,
            by_address[0x0052270D].op_str) == (
                "cmp", "dword ptr [eax*4 + 0x9aa730], 0x32")
    assert by_address[0x00522715].mnemonic == "jl"
    assert by_address[0x00522715].operands[0].imm == 0x00522633

    header = (ROOT / "src/map.h").read_text()
    start = header.index("struct Continent {")
    end = header.index("};", start)
    declaration = header[start:end]
    assert "int tile_count;" in declaration
    assert declaration.index("int tile_count;") < declaration.index("open_terrain;")
