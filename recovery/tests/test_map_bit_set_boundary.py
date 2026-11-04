"""Cross-TU bit_set callers target its existing mapped owner directly."""

from pathlib import Path
import os
import re

import pytest

from match_recovery.source import read_file


ROOT = Path(__file__).resolve().parents[2]


def _code(text):
    return re.sub(r"/\*.*?\*/|//[^\n]*", "", text, flags=re.S)


def _body(text, signature):
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for end in range(opening, len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            return text[start:end + 1]
    raise AssertionError(f"unterminated body: {signature}")


def _block(text, start):
    opening = text.index("{", start)
    depth = 0
    for end in range(opening, len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            return text[start:end + 1]
    raise AssertionError(f"unterminated block at {start}")


def _mapped_spans(address):
    rows = [row for source in (ROOT / "src/base.cpp", ROOT / "src/game.cpp",
                               ROOT / "src/map_grid.cpp")
            for row in read_file(source) if row.address == address]
    row, = rows
    from match_recovery.model import Span
    return [Span(*span) for span in row.image_spans]


def test_all_verified_cross_tu_calls_name_the_mapped_bit_set_owner():
    header = (ROOT / "src/map.h").read_text(encoding="utf-8")
    map_cpp = (ROOT / "src/map_grid.cpp").read_text(encoding="utf-8")
    base_cpp = (ROOT / "src/base.cpp").read_text(encoding="utf-8")
    game_cpp = (ROOT / "src/game.cpp").read_text(encoding="utf-8")

    assert "bit_set_call" not in _code(header + map_cpp + base_cpp + game_cpp)
    assert header.count("void __cdecl bit_set(int x, int y, int bit, BOOL set);") == 1
    assert map_cpp.count("void __cdecl bit_set(int x, int y, int bit, BOOL set)") == 1

    production_spread = _code(base_cpp)
    helper = production_spread[production_spread.index(
        "void production_voice_of_planet_spread("):production_spread.index(
        "int production_vehicle_morale(")]
    assert helper.count("bit_set(") == 4
    assert "bit_set(x_coord, y_coord, BIT_FUNGUS, TRUE);" in helper
    assert "bit_set(x_coord, y_coord, MapTileFlagClearMask, FALSE);" in helper
    assert "bit_set(x, y, BIT_FUNGUS, TRUE);" in helper
    assert "bit_set(x, y, MapTileFlagClearMask, FALSE);" in helper
    assert "production_voice_of_planet_spread(production_bitmask_offset);" in base_cpp

    base_mark = _body(production_spread, "void __cdecl base_mark(int base_id)")
    assert base_mark.count("bit_set(") == 1
    assert "bit_set(x_radius, y_radius, BIT_BASE_RADIUS, true);" in base_mark

    random_events = _code(game_cpp)
    event_helper = _body(
        random_events, "static void random_events_cases_19_21(")
    case20_start = event_helper.index("case 20:")
    case20 = _block(event_helper, case20_start)
    case21_start = event_helper.index("case 21:", case20_start)
    case21 = _block(event_helper, case21_start)
    assert case20.count("bit_set(") == 1
    assert "bit_set(x, y, BIT_FARM, 0);" in case20
    assert case21.count("bit_set(") == 1
    assert "bit_set(x, y, BIT_MINE, 0);" in case21


def test_pinned_full_original_spans_have_the_seven_direct_targets():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from capstone.x86 import X86_OP_IMM
    from match_recovery.pe_coff import PeImage
    from match_recovery.source_inventory import image_pin

    image_path = Path(os.environ.get(
        "OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not image_path.is_file():
        pytest.skip("pinned original required for call-boundary verification")
    image = PeImage(image_path.read_bytes(), image_pin(ROOT))
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True

    def calls_from(address):
        calls = {}
        for span in _mapped_spans(address):
            decoded = list(decoder.disasm(image.read(span), span.start))
            cursor = span.start
            for insn in decoded:
                assert insn.address == cursor, f"undecoded byte at {cursor:#x}"
                cursor += insn.size
            assert cursor == span.end
            calls.update({insn.address: insn.operands[0].imm for insn in decoded
                          if insn.mnemonic == "call"
                          and insn.operands[0].type == X86_OP_IMM})
        return calls

    expected = {
        0x004E4350: {0x004E43C8},
        0x004F07E0: {0x004F2D83, 0x004F2D92, 0x004F2EBD, 0x004F2ECC},
        0x0051F2E0: {0x00521D8A, 0x00522050},
    }
    for owner, pcs in expected.items():
        calls = calls_from(owner)
        assert {pc: calls.get(pc) for pc in pcs} == {
            pc: 0x00591D60 for pc in pcs
        }
