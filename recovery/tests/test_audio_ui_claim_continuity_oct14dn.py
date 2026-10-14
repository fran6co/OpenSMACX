"""Native claim publication only; no body, identity or history rewrite."""
from dataclasses import fields, replace
from pathlib import Path
import subprocess

import pytest

from match_recovery.source import read_text
from match_recovery.source.writer import write_bytes

ROOT = Path(__file__).resolve().parents[2]
BASE = "fc450f64daa008672e76de4070069b1e7faf9d80"
SELECTED = {0x4C6440, 0x4C6920, 0x4C6EA0, 0x5D5470, 0x613900}


@pytest.mark.parametrize("relative", [
    "src/sound.cpp", "src/wave.cpp", "src/graphicwin.cpp",
    "src/jackal/filewin.cpp",
])
def test_only_native_claim_promotions_preserve_complete_source(relative):
    path = ROOT / relative
    old = subprocess.check_output(["git", "show", BASE + ":" + relative], cwd=ROOT)
    before = read_text(old.decode(), path)
    after = read_text(path.read_text(), path)
    promoted = [replace(row, byte_exact=True, semantic=False)
                for row in before if row.address in SELECTED]
    assert promoted
    assert all(not row.byte_exact for row in before if row.address in SELECTED)
    # Lossless writer equality checks every source byte, including TRIED/LEVER,
    # original symbols and body text; only the five native flags may change.
    assert write_bytes(old, promoted) == path.read_bytes()
    assert len(before) == len(after)
    for original, recorded in zip(before, after):
        expected = (replace(original, byte_exact=True, semantic=False)
                    if original.address in SELECTED else original)
        for field in fields(original):
            if field.name != "region":
                assert getattr(recorded, field.name) == getattr(expected, field.name)
