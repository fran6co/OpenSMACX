"""Permanent integration control for the complete end_of_game game.cpp owner."""
from pathlib import Path

from match_recovery.source import read_file

ROOT = Path(__file__).resolve().parents[2]
ADDRESS = 0x00524870
SIGNATURE = "int __cdecl end_of_game(int check_victory)"


def record(path: str):
    rows = [row for row in read_file(ROOT / path) if row.address == ADDRESS]
    assert len(rows) == 1, (path, hex(ADDRESS), len(rows))
    return rows[0]


def definition(region: str):
    start = region.find(SIGNATURE)
    assert start >= 0
    brace = region.find("{", start)
    assert brace >= 0
    depth = 0
    for index in range(brace, len(region)):
        if region[index] == "{":
            depth += 1
        elif region[index] == "}":
            depth -= 1
            if depth == 0:
                return region[start:index + 1]
    raise AssertionError("unterminated end_of_game definition")


def test_endgame_source_owner_retains_original_identity_and_complete_body():
    current = record("src/game.cpp")
    assert current.address == ADDRESS
    assert (current.symbol or current.name) == "?end_of_game@@YAHH@Z"
    assert current.image_spans == ((0x00524870, 0x0052581F),)
    assert current.byte_exact is False
    assert current.semantic is False
    body = definition(current.region)
    assert "BODY GOES HERE." not in body
    assert "PENDING_BODY" not in body
    assert "ORIGINAL: " not in body
    assert "mechanical transcription of the Ghidra decompilation" in current.region
    assert "both nested nested switches" in current.region


def test_endgame_uses_typed_game_and_netdaemon_owners():
    region = (ROOT / "src/game.cpp").read_text()
    body = definition(region)
    assert "NetDaemonNet.hang_up()" in region
    assert "NetDaemonNet.locks_.clear()" in region
    assert "data.diplo_treaties[" in body
