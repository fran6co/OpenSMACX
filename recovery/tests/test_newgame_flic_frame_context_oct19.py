"""Pinned Newgame frame facts and source-owner/lifetime continuity controls."""

from __future__ import annotations

import os
from pathlib import Path
import re

import pytest

from match_recovery.model import Span
from match_recovery.pe_coff import PeImage
from match_recovery.source.asm import Listing
from match_recovery.source import read_file


ROOT = Path(__file__).resolve().parents[2]
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
NEWGAME_MAPS = {
    0x0058CED0: ("?size_of_planet@@YAHH@Z", ((0x0058CED0, 0x0058D6A4),
                                               (0x006607D7, 0x00660989))),
    0x0058C2A0: ("?custom_planet@@YAHHH@Z", ((0x0058C2A0, 0x0058CE9C),
                                               (0x006605A5, 0x006607D7))),
    0x0058D6B0: ("?map_menu@@YAHH@Z", ((0x0058D6B0, 0x0058DB17),
                                        (0x00660989, 0x00660A96))),
    0x0058E360: ("?top_menu@@YAHH@Z", ((0x0058E360, 0x0058EE31),
                                        (0x00660A96, 0x00660BD7))),
    0x0058DB30: ("?multiplayer_init@@YAHH@Z", ((0x0058DB30, 0x0058E35E),)),
    0x00589870: ("sub_589870", ((0x00589870, 0x00589886),)),
    0x00589890: ("sub_589890", ((0x00589890, 0x0058989A),)),
    0x005898A0: ("?thumb_routine@@YAXH@Z", ((0x005898A0, 0x00589A24),)),
    0x00589D30: ("?config_game@@YAHH@Z", ((0x00589D30, 0x0058C29D),
                                            (0x0066028A, 0x006605A5))),
}
CRASH_LANDING_MAP = ("?crash_landing@@YAXH@Z", ((0x005AE120, 0x005AED43),
                                                   (0x00662450, 0x00662705)))
ORDINARY_ENTRIES = (
    0x0058CED0, 0x0058C2A0, 0x0058D6B0, 0x0058E360,
    0x0058DB30, 0x005898A0, 0x00589D30, 0x005AE120,
)


def _pinned_image() -> PeImage:
    configured = os.environ.get("OPENSMACX_IMAGE")
    image_path = Path(configured) if configured else ROOT / ".opensmacx/game/terranx_original.exe"
    if not image_path.is_file():
        pytest.skip("pinned original image is not available")
    return PeImage(image_path.read_bytes(), PIN)


def _state_by_address(path: Path):
    return {state.address: state for state in read_file(path)}


def _class_body(source: str, class_name: str) -> str:
    start = source.index(f"class {class_name}")
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        if source[end] == "{":
            depth += 1
        elif source[end] == "}":
            depth -= 1
            if depth == 0:
                return source[opening + 1:end]
    raise AssertionError(f"unterminated class {class_name}")


def test_all_eight_pinned_newgame_entries_have_ebp_prologues():
    image = _pinned_image()
    for address in ORDINARY_ENTRIES:
        listing = Listing(image.read(Span(address, address + 8)), address)
        instructions = listing.instructions
        assert [(item.mnemonic, item.op_str) for item in instructions[:2]] == [
            ("push", "ebp"), ("mov", "ebp, esp")
        ], f"unexpected original entry prologue at 0x{address:08X}"


def test_newgame_and_crash_landing_mappings_keep_identity_and_spans():
    states = _state_by_address(ROOT / "src/newgame.cpp")
    assert set(NEWGAME_MAPS) <= set(states)
    for address, (name, spans) in NEWGAME_MAPS.items():
        state = states[address]
        assert state.name == name
        assert state.image_spans == spans

    crash = _state_by_address(ROOT / "src/game.cpp")[0x005AE120]
    assert crash.name == CRASH_LANDING_MAP[0]
    assert crash.image_spans == CRASH_LANDING_MAP[1]


def test_newgame_uses_the_existing_framed_source_group():
    cmake = (ROOT / "CMakeLists.txt").read_text(encoding="utf-8")
    groups = re.findall(r"set_source_files_properties\((.*?)\)", cmake, re.DOTALL)
    owners = [group for group in groups if re.search(r"\bsrc/newgame\.cpp\b", group)]
    assert len(owners) == 1
    options = re.search(r'PROPERTIES\s+COMPILE_OPTIONS\s+"([^"]+)"', owners[0])
    assert options is not None
    assert options.group(1).split() == ["/Oy-"]


def test_in_class_flic_destructor_and_mapping_remain_on_real_owner():
    header = (ROOT / "src/flic.h").read_text(encoding="utf-8")
    destructor = _class_body(header, "Flic")
    assert re.search(r"~Flic\s*\(\s*\)\s*\{\s*close\(\)\s*;\s*\}", destructor)

    states = _state_by_address(ROOT / "src/flic.cpp")
    state = states[0x004327A0]
    assert state.name == "??1Flic@@QAE@XZ"
    assert state.image_spans == ((0x004327A0, 0x004327F8),
                                 (0x00653420, 0x00653443))

