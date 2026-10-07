"""Extract product-owned game storage for actual-source host controls."""
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[2]


def _definition(source, marker):
    match = re.search(re.escape(marker) + r"\s*\{", source)
    assert match, "missing current product declaration: " + marker
    start = match.start()
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        if source[end] == "{":
            depth += 1
        elif source[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start:end + 1] + ";"
    raise AssertionError("unterminated current product declaration: " + marker)


def current_game_storage_fixture(*, scenario=False):
    """Return exact current storage types and definitions for a host harness."""
    base = (ROOT / "src/base.h").read_text()
    state_header = (ROOT / "src/game_data_state_window.h").read_text()
    game_header = (ROOT / "src/game.h").read_text()
    game_source = (ROOT / "src/game.cpp").read_text()
    pieces = ["#include <stdint.h>\n#include <stddef.h>",
              _definition(base, "struct BaseSecretProject")]
    for marker in ("struct GameDataLockMaskBytes", "union GameDataLockMaskView",
                   "struct GameDataStateWindowFields", "struct GameDataStateWindow"):
        pieces.append(_definition(state_header, marker))
    match = re.search(r"^GameDataStateWindow\s+GameDataStateStorage\s*;", game_source, re.M)
    assert match, "missing current GameDataStateStorage definition in game.cpp"
    pieces.append(match.group(0))
    if scenario:
        pieces.append(_definition(game_header, "struct ScenarioSaveState"))
        match = re.search(r"^ScenarioSaveState\s+ScenarioSave\s*;", game_source, re.M)
        assert match, "missing current ScenarioSave definition in game.cpp"
        pieces.append(match.group(0))
    return "\n".join(pieces)
