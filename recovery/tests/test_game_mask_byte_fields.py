"""Actual lock-mask byte operation control using the source-owned union view."""
from __future__ import annotations

from pathlib import Path
import re
import shutil
import subprocess

import pytest
from match_recovery.includes import COMMENTS_OR_STRINGS

ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/game.cpp").is_file())


def definition(text: str, signature: str) -> str:
    masked = COMMENTS_OR_STRINGS.sub(
        lambda match: "".join("\n" if char == "\n" else " " for char in match[0]),
        text,
    )
    assert masked.count(signature) == 1, signature
    start = masked.index(signature)
    opening = masked.find("{", start)
    depth = 0
    for position in range(opening, len(masked)):
        if masked[position] == "{":
            depth += 1
        elif masked[position] == "}":
            depth -= 1
            if depth == 0:
                return text[start:position + 1]
    raise AssertionError("unterminated definition: " + signature)


def projected(update_mutation: tuple[str, str] | None = None) -> str:
    header = (ROOT / "src/game_data_state_window.h").read_text()
    game = (ROOT / "src/game.cpp").read_text()
    bytes_decl = definition(header, "struct GameDataLockMaskBytes") + ";"
    view_decl = definition(header, "union GameDataLockMaskView") + ";"
    turn = definition(game, "void __cdecl turn_upkeep()")
    update = re.findall(
        r"GameDataStateStorage\.fields_\.lock_masks_\.bytes_\.alive_mask_\s*\|=\s*"
        r"static_cast<uint8_t>\(bit\);", turn)
    assert len(update) == 1
    operation = update[0]
    if update_mutation is not None:
        old, new = update_mutation
        assert operation.count(old) == 1
        operation = operation.replace(old, new)
    return f'''#include <cassert>
#include <cstdint>
{bytes_decl}
{view_decl}
struct Fields {{ GameDataLockMaskView lock_masks_; }};
struct GameState {{ Fields fields_; }};
GameState GameDataStateStorage;
static void promote(unsigned int bit) {{ {operation} }}
int main() {{
  GameDataStateStorage.fields_.lock_masks_.enable_mask_ = 0xA5C30005u;
  promote(2u);
  assert(GameDataStateStorage.fields_.lock_masks_.enable_mask_ == 0xA5C30205u);
}}
'''


def run(tmp_path: Path, source: str) -> subprocess.CompletedProcess:
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("g++ unavailable")
    path = tmp_path / "mask_bytes.cpp"
    binary = tmp_path / "mask_bytes"
    path.write_text(source)
    subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
                    str(path), "-o", str(binary)], check=True,
                   capture_output=True, timeout=45)
    return subprocess.run([str(binary)], capture_output=True, timeout=15)


def test_turn_upkeep_actual_alive_byte_update_preserves_other_bytes(tmp_path: Path):
    result = run(tmp_path, projected())
    assert result.returncode == 0, result.stderr.decode(errors="replace")


@pytest.mark.parametrize("replacement", [
    "GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ |=",
    "GameDataStateStorage.fields_.lock_masks_.enable_mask_ |=",
])
def test_turn_upkeep_swapped_or_wide_store_mutants_fail(tmp_path: Path, replacement: str):
    result = run(tmp_path, projected((
        "GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_ |=", replacement)))
    assert result.returncode != 0, "wrong-byte or wide-store mask mutant survived"


def test_game_and_newgame_remove_byte_pointer_punning_only():
    game = (ROOT / "src/game.cpp").read_text()
    newgame = (ROOT / "src/newgame.cpp").read_text()
    assert "reinterpret_cast<unsigned char *>(&GameDataStateStorage.fields_.lock_masks_" not in game
    assert "reinterpret_cast<const unsigned char *>(&GameDataStateStorage.fields_.lock_masks_" not in game
    assert "reinterpret_cast<uint8_t *>(&GameDataStateStorage.fields_.lock_masks_" not in game
    assert "reinterpret_cast<unsigned char *>(&GameDataStateStorage.fields_.lock_masks_" not in newgame
    assert "GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 0xFFFFFF00u" in newgame
    assert "GameDataStateStorage.fields_.lock_masks_.enable_mask_ = (GameDataStateStorage.fields_.lock_masks_.enable_mask_ & ~0x0000FF00u)" in game
