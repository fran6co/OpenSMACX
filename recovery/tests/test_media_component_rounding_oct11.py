"""Actual-source rounding and pinned instruction controls, not full codec proof."""
from pathlib import Path
import subprocess

import pytest

from test_movie_media_behavior import build_and_run, definition

ROOT = Path(__file__).resolve().parents[2]
BASE = "03ad13280"
SIGNATURE = "static int media_scale_signed_component("


def test_only_private_scaling_body_changes_and_mappings_survive():
    from match_recovery.source import read_text
    from test_texture_raster_actual_source import _continuity_fields
    path = ROOT / "src/movie_media.cpp"
    old = subprocess.check_output(["git", "show", BASE + ":src/movie_media.cpp"],
                                  cwd=ROOT, text=True)
    new = path.read_text()
    assert old.replace(definition(old, SIGNATURE), "REVIEWED_SCALING") == new.replace(
        definition(new, SIGNATURE), "REVIEWED_SCALING")
    assert [(_continuity_fields(r), r.symbol, r.function_fragment) for r in read_text(old, path)] == [
        (_continuity_fields(r), r.symbol, r.function_fragment) for r in read_text(new, path)]


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
