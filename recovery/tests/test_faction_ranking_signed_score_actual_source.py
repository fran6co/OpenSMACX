"""Signed ranking score checks using the complete current great_satan body.

These host controls establish the signed sentinel/compare and 32-bit product
cases. They do not establish all game score ranges or game acceptance.
"""

from pathlib import Path
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def extract_function(path: Path, signature: str) -> str:
    text = re.sub(r"/\*.*?\*/|//[^\n]*", "", path.read_text(), flags=re.S)
    assert text.count(signature) == 1
    start = text.index(signature)
    brace = text.index("{", start)
    depth = 0
    for end in range(brace, len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            return text[start:end + 1]
    raise AssertionError("unterminated actual source function")


def actual_great_satan():
    return extract_function(
        ROOT / "src/faction.cpp",
        "BOOL __cdecl great_satan(int faction_id, BOOL is_aggressive)",
    )


def game_constant(name: str) -> int:
    text = (ROOT / "src/game.h").read_text()
    match = re.search(rf"\b{name}\s*=\s*(0[xX][0-9A-Fa-f]+|[0-9]+)\b", text)
    assert match is not None, f"{name} must have a literal declaration in game.h"
    return int(match.group(1), 0)


def actual_harness() -> str:
    names = (
        "DLVL_SPECIALIST", "DLVL_TALENT", "DLVL_LIBRARIAN", "DLVL_TRANSCEND",
        "RULES_INTENSE_RIVALRY", "RULES_VICTORY_CONQUEST",
    )
    text = HARNESS
    for name in names:
        text = text.replace(f"@{name}@", str(game_constant(name)))
    assert "@" not in text
    return text


HARNESS = r"""
#include <cassert>
#include <cstdint>
#define __cdecl
typedef int BOOL;
const int DLVL_SPECIALIST=@DLVL_SPECIALIST@, DLVL_TALENT=@DLVL_TALENT@,
          DLVL_LIBRARIAN=@DLVL_LIBRARIAN@, DLVL_TRANSCEND=@DLVL_TRANSCEND@;
const uint32_t RULES_INTENSE_RIVALRY=@RULES_INTENSE_RIVALRY@,
               RULES_VICTORY_CONQUEST=@RULES_VICTORY_CONQUEST@;
struct GameFields { int faction_rankings_[8]; int turn_current_num_;
                    uint32_t game_rules_; };
struct GameState { GameFields fields_; } GameDataStateStorage;
struct Player { int current_num_bases; int diff_level; } PlayersData[8];
struct Scenario { int objective_req_victory; } ScenarioSave;
int FactionRankingsUnk[8];
int human_calls, climactic_calls, aah_calls;
BOOL is_human(int faction) { ++human_calls; return faction==1; }
BOOL climactic_battle() { ++climactic_calls; return 0; }
int aah_ooga(int, int) { ++aah_calls; return 1; }
// ACTUAL GREAT SATAN

void set_case(int level, int rules, int target, int opposition) {
    human_calls=climactic_calls=aah_calls=0;
    GameDataStateStorage.fields_.faction_rankings_[7]=1;
    GameDataStateStorage.fields_.faction_rankings_[6]=2;
    GameDataStateStorage.fields_.turn_current_num_=500;
    GameDataStateStorage.fields_.game_rules_=rules;
    PlayersData[1].current_num_bases=20;
    PlayersData[1].diff_level=level;
    ScenarioSave.objective_req_victory=2000;
    FactionRankingsUnk[1]=target;
    FactionRankingsUnk[2]=opposition;
}

int main() {
    // Ordinary positive scores and the signed -1 ranking sentinel.
    set_case(2, 0, 10, 5);
    assert(great_satan(1, 0));
    assert(human_calls==1 && climactic_calls==1 && aah_calls==0);
    set_case(2, 0, -1, 1);
    assert(!great_satan(1, 0));
    assert(human_calls==1 && climactic_calls==1 && aah_calls==0);

    // Librarian factors 2 and 3 overflow the low 32-bit products differently:
    // 2*0x40000000 is INT_MIN, while 3*0x2aaaaaaa is 0x7ffffffe.
    set_case(3, 0, 0x40000000, 0x2aaaaaaa);
    assert(!great_satan(1, 0));

    // Intense-rivalry factors retain the same signed wrapped comparison.
    set_case(2, RULES_INTENSE_RIVALRY, 0x70000000, 0x10000000);
    assert(!great_satan(1, 0));

    // Existing early guards still avoid the later climactic check.
    set_case(2, 0, 10, 5);
    PlayersData[1].current_num_bases=1;
    assert(!great_satan(1, 0));
    assert(climactic_calls==0);
}
"""


def compile_run(tmp_path: Path, compiler: str, body: str, label: str):
    source = tmp_path / f"ranking-{label}.cpp"
    binary = tmp_path / f"ranking-{label}"
    source.write_text(actual_harness().replace("// ACTUAL GREAT SATAN", body))
    subprocess.run([compiler, "-std=c++11", "-O2", "-fwrapv", str(source), "-o", str(binary)],
                   check=True, capture_output=True, text=True, timeout=45)
    return subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)


@pytest.mark.parametrize("compiler", tuple(filter(None, (shutil.which("g++"), shutil.which("clang++")))) or (None,))
def test_actual_signed_score_storage_and_great_satan_mutant(tmp_path, compiler):
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")

    faction_h = (ROOT / "src/faction.h").read_text()
    faction = (ROOT / "src/faction.cpp").read_text()
    game_h = (ROOT / "src/game.h").read_text()
    game = (ROOT / "src/game.cpp").read_text()
    netdaemon = (ROOT / "src/netdaemon.cpp").read_text()
    assert re.search(r"extern int FactionRankingsUnk\[MaxPlayerNum\]", faction_h)
    assert re.search(r"\nint FactionRankingsUnk\[MaxPlayerNum\];", faction)
    assert game_constant("RULES_INTENSE_RIVALRY") == 0x40
    assert game_constant("RULES_VICTORY_CONQUEST") == 0x02
    assert game_constant("DLVL_SPECIALIST") == 1
    assert game_constant("DLVL_TALENT") == 2
    assert game_constant("DLVL_LIBRARIAN") == 3
    assert game_constant("DLVL_TRANSCEND") == 5
    assert "FactionRankingsUnk[faction_id] = score;" in faction
    assert "FactionRankingsUnk[best_faction] = -1;" in faction
    assert "game_data_candidate_transfer(file, FactionRankingsUnk, 0x20)" in game
    assert "case 0x19: NATIVE_COPY(&FactionRankingsUnk, 0); return true;" in netdaemon

    current = actual_great_satan()
    good = compile_run(tmp_path, compiler, current, "signed")
    assert good.returncode == 0, good.stderr

    signed_left = "factor * FactionRankingsUnk[GameDataStateStorage.fields_.faction_rankings_[7]]"
    signed_right = "diff_factor * FactionRankingsUnk[GameDataStateStorage.fields_.faction_rankings_[6]]"
    old_unsigned = current.replace(
        f"{signed_left}\n        >= {signed_right}",
        f"static_cast<uint32_t>({signed_left})\n        >= static_cast<uint32_t>({signed_right})",
    )
    assert old_unsigned != current
    damaged = compile_run(tmp_path, compiler, old_unsigned, "old-unsigned")
    assert damaged.returncode != 0, "unsigned-comparison control unexpectedly passed"
