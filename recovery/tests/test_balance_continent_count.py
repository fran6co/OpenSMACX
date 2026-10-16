"""Pinned member-address and signed score-arithmetic evidence for balance."""

from pathlib import Path
import os

import pytest


ROOT = Path(__file__).resolve().parents[2]


def test_complete_pinned_balance_span_uses_region_tile_count_twice():
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
               if item.address == 0x005B0420]
    assert record.image_spans == ((0x005B0420, 0x005B0A2C),)
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
    assert cursor == high, "the complete mapped balance span must decode"

    def instruction(address, mnemonic, operands):
        item = by_address[address]
        assert (item.mnemonic, item.op_str) == (mnemonic, operands)

    # The selected region is the tile's byte at +3, retained in [ebp-0x1c].
    instruction(0x005B0505, "mov", "cl, byte ptr [edx + eax*4 + 3]")
    instruction(0x005B050C, "mov", "dword ptr [ebp - 0x1c], ecx")

    # Both score inputs reuse that region, form 7*region, and scale by four:
    # array base + 28*region + offset zero, i.e. signed Continent::tile_count.
    for region_load, lea, sub, member_load in (
        (0x005B0613, 0x005B0616, 0x005B061D, 0x005B061F),
        (0x005B0764, 0x005B0767, 0x005B076E, 0x005B0770),
    ):
        instruction(region_load, "mov", "eax, dword ptr [ebp - 0x1c]")
        instruction(lea, "lea", "edx, [eax*8]")
        instruction(sub, "sub", "edx, eax")
        instruction(member_load, "mov", "eax, dword ptr [edx*4 + 0x9aa730]")

    # Signed rounded division by 32: (tile_count + 31) / 32, then clamp 1..8.
    instruction(0x005B0626, "add", "eax, 0x1f")
    instruction(0x005B0629, "cdq", "")
    instruction(0x005B062A, "and", "edx, 0x1f")
    instruction(0x005B062D, "add", "eax, edx")
    instruction(0x005B062F, "sar", "eax, 5")
    instruction(0x005B0632, "cmp", "eax, 1")
    instruction(0x005B0635, "jl", "0x5b0646")
    instruction(0x005B0637, "cmp", "eax, 8")

    # Signed rounded division by 64: (tile_count + 63) / 64, then clamp 0..3.
    instruction(0x005B0777, "cdq", "")
    instruction(0x005B0778, "and", "edx, 0x3f")
    instruction(0x005B077B, "add", "eax, edx")
    instruction(0x005B077D, "sar", "eax, 6")
    instruction(0x005B0780, "js", "0x5b078e")
    instruction(0x005B0782, "cmp", "eax, 3")

    header = (ROOT / "src/map.h").read_text()
    declaration_start = header.index("struct Continent {")
    declaration_end = header.index("};", declaration_start)
    declaration = header[declaration_start:declaration_end]
    assert "int tile_count;" in declaration
    assert declaration.index("int tile_count;") < declaration.index("open_terrain;")
