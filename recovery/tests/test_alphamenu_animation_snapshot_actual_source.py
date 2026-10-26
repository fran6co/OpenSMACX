"""Check pinned AlphaMenu animation callback and copy instruction order."""

from hashlib import sha256
import os
from pathlib import Path

import pytest


PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
PIN_IMAGE = Path(os.environ.get(
    "OPENSMACX_IMAGE",
    "/home/fran6co/code/OpenSMACX/.opensmacx/game/terranx_original.exe",
))


def pinned_listing(start: int, end: int) -> dict[int, str]:
    if not PIN_IMAGE.is_file():
        pytest.skip(f"pinned original unavailable at absolute image root: {PIN_IMAGE}")
    data = PIN_IMAGE.read_bytes()
    assert sha256(data).hexdigest() == PIN, f"wrong pinned original at {PIN_IMAGE}"
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    from match_recovery.source.asm import Listing

    lines = Listing(PeImage(data, PIN).read(Span(start, end)), start).lines
    return {int(line.split()[0].rstrip(":"), 16): line.lower() for line in lines}


def test_pinned_show_and_hide_callback_and_copy_instruction_order():
    show = pinned_listing(0x00403680, 0x0040382F)
    assert "call 0x5ecfe0" in show[0x004036DB]
    assert "mov eax, dword ptr [0x9b7b1c]" in show[0x004036E0]
    assert "call 0x5d8000" in show[0x00403733]
    assert "test dword ptr [0x9a6490], 0x8000000" in show[0x00403738]
    hide = pinned_listing(0x00403830, 0x00403AE5)
    assert "call 0x5d7670" in hide[0x004038EE]  # initial Buffer::init
    assert "[esi + 0x4c8]" in hide[0x004038F3]  # reloaded pixel height
    assert "[esi + 0x4c4]" in hide[0x004038F9]  # reloaded pixel width
    assert "call 0x5dff00" in hide[0x0040391D]  # initial Console copy
    assert "mov eax, dword ptr [0x9b7b1c]" in hide[0x00403922]
    assert "call 0x5d8000" in hide[0x00403964]  # clipping callback
    assert "[esi + 0x1108]" in hide[0x0040396C]  # sprite1 height, after clip
    assert "test eax, 0x8000000" in hide[0x00403979]
    # The animation and final-tail copy sites reload both dimensions after
    # intervening callbacks; they are distinct from the initial Console copy.
    assert "[esi + 0x4c8]" in hide[0x004039A8]
    assert "[esi + 0x4c4]" in hide[0x004039B4]
    assert "call 0x5dff00" in hide[0x004039CF]
    assert "[esi + 0x4c8]" in hide[0x00403A4B]
    assert "[esi + 0x4c4]" in hide[0x00403A51]
    assert "call 0x5dff00" in hide[0x00403A6F]
