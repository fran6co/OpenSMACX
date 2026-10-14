"""Seven native claim promotions; not linked/reference/game acceptance."""
from dataclasses import fields, replace
from pathlib import Path
import subprocess

import pytest

from match_recovery.source import read_text
from match_recovery.source.writer import write_bytes

ROOT = Path(__file__).resolve().parents[2]
BASE = "0e6b4b490aea7df0e4dc5ce7961eae3de4f129db"
SELECTED = {0x59E950, 0x5D4BB0, 0x5CBBC0, 0x631B30,
            0x633A50, 0x634330, 0x44C870}


def historical_core_claim_view(text, relative):
    """Test-only projection of the seven flags, never a source-file write.

    Historical guards keep their own original baselines. This bridge reverses
    only this separately verified publication, preserving every other byte.
    The full-file test below requires the live stronger claims, so a removed
    promotion cannot hide behind this historical projection.
    """
    path = ROOT / relative
    selected = [row for row in read_text(text, path) if row.address in SELECTED]
    if not selected:
        return text
    baseline = subprocess.check_output(["git", "show", BASE + ":" + relative], cwd=ROOT)
    old = {row.address: row for row in read_text(baseline.decode(), path)}
    restored = []
    for row in selected:
        original = old[row.address]
        assert (row.byte_exact, row.semantic) in {
            (original.byte_exact, original.semantic), (True, False)}
        restored.append(replace(row, byte_exact=original.byte_exact,
                                semantic=original.semantic))
    return write_bytes(text.encode(), restored).decode()


def test_historical_projection_preserves_unrelated_text_and_claims():
    relative = "src/net_fifo.cpp"
    text = (ROOT / relative).read_text()
    baseline = subprocess.check_output(["git", "show", BASE + ":" + relative], cwd=ROOT).decode()
    assert historical_core_claim_view(text, relative) == baseline
    mutant = text.replace("this->count_ += 1;", "this->count_ += 2;", 1)
    assert mutant != text
    assert historical_core_claim_view(mutant, relative) != baseline
    rows = read_text(text, ROOT / relative)
    unrelated = next(row for row in rows if row.address == 0x6339C0)
    assert unrelated.byte_exact
    mutant = write_bytes(text.encode(), [replace(unrelated, byte_exact=False)]).decode()
    assert historical_core_claim_view(mutant, relative) != baseline


@pytest.mark.parametrize("relative", [
    "src/alpha.cpp", "src/alphanet.cpp", "src/movie_media.cpp",
    "src/net_class.cpp", "src/net_fifo.cpp", "src/netdaemon.cpp",
    "src/shared_word.cpp",
])
def test_only_native_promotions_preserve_complete_source(relative):
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
                assert getattr(recorded, field.name) == getattr(expected, field.name)
