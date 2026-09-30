"""Pinned callback call-site controls; not linked/runtime acceptance.

These instructions establish argument cleanup and discarded results at the two
draw dispatch sites. They do not prove all callback lifetimes or game behavior.
"""

import os
from pathlib import Path

import pytest
from capstone import Cs, CS_ARCH_X86, CS_MODE_32

from match_recovery.model import Span
from match_recovery.pe_coff import PeImage
from match_recovery.source_inventory import image_pin


ROOT = Path(__file__).resolve().parents[2]


def test_original_draw_dispatch_is_cdecl_and_discards_result():
    path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not path.is_file():
        pytest.skip("pinned original image required")
    image = PeImage(path.read_bytes(), image_pin(ROOT))
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)

    def instructions(start, end):
        return [(i.mnemonic, i.op_str) for i in
                decoder.disasm(image.read(Span(start, end)), start)]

    # Callback is followed by caller cleanup and overwrites of the scratch
    # registers, not a test or use of its result.
    assert instructions(0x610ECA, 0x610ED2) == [
        ("call", "dword ptr [esi + 0x64]"),
        ("mov", "ecx, dword ptr [esi]"), ("add", "esp, 0x14")]
    assert instructions(0x604C37, 0x604C40)[:2] == [
        ("call", "dword ptr [esi + 0x3168]"), ("add", "esp, 0x14")]
    assert instructions(0x4BDFDB, 0x4BDFDD) == [("pop", "ebp"), ("ret", "")]
    assert "typedef void (__cdecl *DrawCallback)(Sprite *, GraphicWin *, int, int, int);" in (
        ROOT / "src/spritebox.h").read_text()
    assert "void __cdecl veh_sprite_draw(" in (ROOT / "src/popup.h").read_text()
