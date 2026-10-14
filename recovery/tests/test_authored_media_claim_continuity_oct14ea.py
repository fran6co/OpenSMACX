"""Four native promotions; implementations and recovery history unchanged."""
from dataclasses import fields, replace
from pathlib import Path
import subprocess

import pytest
from match_recovery.source import read_text
from match_recovery.source.writer import write_bytes

ROOT = Path(__file__).resolve().parents[2]
BASE = "7a20029edfe4fb109e048bdcdce23f0e44cc0117"
SELECTED = {0x629E50, 0x637310, 0x63E7F0, 0x63B2D0}


@pytest.mark.parametrize("relative", ["src/flic.cpp", "src/video.cpp", "src/cursor.cpp"])
def test_native_publication_preserves_complete_source(relative):
    path = ROOT / relative
    old = subprocess.check_output(["git", "show", BASE + ":" + relative], cwd=ROOT)
    before = read_text(old.decode(), path)
    after = read_text(path.read_text(), path)
    promoted = [replace(row, byte_exact=True, semantic=False)
                for row in before if row.address in SELECTED]
    assert promoted
    assert all(not row.byte_exact for row in before if row.address in SELECTED)
    assert write_bytes(old, promoted) == path.read_bytes()
    assert len(before) == len(after)
    for original, recorded in zip(before, after):
        expected = (replace(original, byte_exact=True, semantic=False)
                    if original.address in SELECTED else original)
        for field in fields(original):
            if field.name != "region":
                assert getattr(expected, field.name) == getattr(recorded, field.name)
