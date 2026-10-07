"""Eight milestone mappings move with their unchanged bodies and seed lessons.

This controls source continuity, not fresh executable equality or game acceptance.
"""
from functools import lru_cache
from pathlib import Path
import subprocess

import pytest
from match_recovery.source import Mode, SourceRole, read_file, read_text

ROOT = Path(__file__).resolve().parents[2]
BASE = "b1c6121c542e8dca18e0433b552c76254e0a967e"
SPECS = (
    (0x00476EE0, "mon_prototype_built", "int faction_id"),
    (0x00477440, "mon_naval_unit_built", "int faction_id"),
    (0x00477540, "mon_air_unit_built", "int faction_id"),
    (0x00477640, "mon_native_life_bred", "int faction_id"),
    (0x00477740, "mon_first_in_space", "int faction_id"),
    (0x00477940, "mon_winning_unify", "int faction_id, int counterpart"),
    (0x004779D0, "mon_winning_trans", "int faction_id, int counterpart"),
    (0x00477A60, "mon_killed_faction", "int faction_id, int counterpart"),
)


@lru_cache(None)
def prior_source():
    return subprocess.check_output(
        ["git", "show", f"{BASE}:src/leaf_recoveries.cpp"], cwd=ROOT
    ).decode()


@pytest.mark.parametrize("address,name,parameters", SPECS)
def test_monument_transfer_retains_body_identity_and_seed_history(address, name, parameters):
    before = {r.address: r for r in read_text(prior_source(), ROOT / "src/leaf_recoveries.cpp")}
    rows = [r for r in read_file(ROOT / "src/game.cpp") if r.address == address]
    assert len(rows) == 1
    current, = rows
    assert current.mode is Mode.BODY
    assert current.source_role is SourceRole.UNCLASSIFIED
    assert not current.extract_error
    for field in ("name", "image_spans", "kind", "byte_exact", "semantic",
                  "levers", "ruled_out", "unrecoverable", "deferred"):
        assert getattr(current, field) == getattr(before[address], field), (hex(address), field)
    assert not (ROOT / "src/leaf_recoveries.cpp").exists(), "retired leaf owner source reappeared"
    signature = f"void __cdecl {name}({parameters}) {{"
    raw = prior_source()
    start = raw.index(signature)
    opening = start + len(signature) - 1
    depth = 0
    for end in range(opening, len(raw)):
        if raw[end] == "{":
            depth += 1
        elif raw[end] == "}":
            depth -= 1
            if depth == 0:
                break
    else:
        pytest.fail(f"Unterminated historical {name} body")
    assert current.region[current.region.index(signature):].strip() == raw[start:end + 1].strip()
    assert f"void __cdecl {name}({parameters});" in (ROOT / "src/game.h").read_text()
