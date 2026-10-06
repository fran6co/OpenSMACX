"""Owner transfers preserve original identities, spans, claims and seed lessons.

This is continuity evidence, not executable equality or game acceptance.
"""
from functools import lru_cache
from pathlib import Path
import subprocess

import pytest
from match_recovery.source import Mode, SourceRole, read_file, read_text

ROOT = Path(__file__).resolve().parents[2]
BASE = "bb4ef7eb469da4b456949628f932845ea50e5c68"
SPECS = (
    (0x548DD0, "faction.cpp"),
    (0x539820, "faction.cpp"),
    (0x55F770, "faction.cpp"),
    (0x53A780, "faction.cpp"),
    (0x52C670, "council.cpp"),
    (0x52BE60, "council.cpp"),
    (0x52AEB0, "council.cpp"),
    (0x528EA0, "game.cpp"),
    (0x455E50, "game.cpp"),
    (0x609150, "dialog.cpp"),
    (0x627830, "popup.cpp"),
    (0x5BECA0, "text.cpp"),
    (0x405140, "popup.cpp"),
)


@lru_cache(None)
def prior_rows():
    text = subprocess.check_output(
        ["git", "show", f"{BASE}:src/leaf_recoveries.cpp"], cwd=ROOT
    ).decode()
    return {r.address: r for r in read_text(text, ROOT / "src/leaf_recoveries.cpp")}


@pytest.mark.parametrize("address,owner", SPECS)
def test_transfer_preserves_original_obligations_and_lessons(address, owner):
    rows = [r for r in read_file(ROOT / "src" / owner) if r.address == address]
    assert len(rows) == 1
    current, = rows
    assert current.mode is Mode.BODY
    assert current.source_role is SourceRole.UNCLASSIFIED
    assert not current.extract_error
    for field in ("name", "image_spans", "kind", "byte_exact", "semantic",
                  "levers", "ruled_out", "unrecoverable", "deferred"):
        assert getattr(current, field) == getattr(prior_rows()[address], field), (hex(address), field)
    assert address not in {r.address for r in read_file(ROOT / "src/leaf_recoveries.cpp")}
