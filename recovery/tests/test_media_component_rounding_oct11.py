"""Actual-source rounding and pinned instruction controls, not full codec proof."""
from pathlib import Path

import pytest

from test_movie_media_behavior import build_and_run, definition

ROOT = Path(__file__).resolve().parents[2]
SIGNATURE = "static int media_scale_signed_component("


@pytest.mark.parametrize("replacement", [
    "return (int)(product >> 16);",
    "return (int)(product >> 16) + (int)((product >> 16) & 1);",
    "return (int)(product >> 16) - (int)((product >> 15) & 1);",
])
def test_actual_rounding_controls_reject_missing_or_wrong_carry(replacement):
    result = build_and_run([(
        "return (int)(product >> 16) + (int)((product >> 15) & 1);", replacement)])
    assert result.returncode < 0, result.stdout + result.stderr


def test_pinned_rounding_precedes_half_shift_and_mask():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    from match_recovery.source_inventory import image_pin
    path = ROOT / ".opensmacx/game/terranx_original.exe"
    if not path.exists():
        pytest.skip("pinned executable unavailable")
    image = PeImage(path.read_bytes(), image_pin(ROOT))
    rows = [(i.mnemonic, i.op_str) for i in Cs(CS_ARCH_X86, CS_MODE_32).disasm(
        image.read(Span(0x5CD571, 0x5CD74D)), 0x5CD571)]
    sequence = [("shl", "edx, 0x10"), ("shr", "eax, 0x10"), ("adc", "eax, edx")]
    assert sum(rows[i:i+3] == sequence for i in range(len(rows)-2)) == 6
    assert ("sar", "eax, 1") in rows and ("and", "eax, 0x7f") in rows
    assert ("sar", "ecx, 1") in rows and ("and", "ecx, 0x7f") in rows
