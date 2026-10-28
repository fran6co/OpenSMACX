"""Actual-source controls for has_abil's original raw alien-flag test.

The harness extracts the complete production body from src/veh.cpp. It checks
the direct ability path, the basic-prototype shortcut, faction free abilities,
and the probe-team project fallback. The expansion-off case distinguishes the
pinned direct RFLAG_ALIEN check from the later is_alien_faction helper.
"""

from __future__ import annotations

import re
import shutil
import subprocess
from pathlib import Path

import pytest
from match_recovery.includes import COMMENTS_OR_STRINGS


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/veh.cpp"
SIGNATURE = "BOOL __cdecl has_abil(int proto_id, int ability_id)"


def _mask(text: str) -> str:
    return COMMENTS_OR_STRINGS.sub(lambda match: re.sub(r"[^\n]", " ", match[0]), text)


def _function(text: str) -> str:
    clean = _mask(text)
    starts = [match.start() for match in re.finditer(re.escape(SIGNATURE), clean)]
    assert len(starts) == 1, f"expected exactly one production definition, found {len(starts)}"
    start = starts[0]
    opening = clean.index("{", start)
    depth = 0
    for pos in range(opening, len(clean)):
        if clean[pos] == "{":
            depth += 1
        elif clean[pos] == "}":
            depth -= 1
            if depth == 0:
                return text[start : pos + 1]
    raise AssertionError("unterminated has_abil definition")


SUPPORT = r'''#include <cstdint>
#include <cstring>
#define __cdecl
typedef int BOOL;
enum { ABL_DEEP_RADAR = 0x2, ABL_ALGO_ENHANCEMENT = 0x10000000,
       WPN_PROBE_TEAM = 21, RULE_FREEABIL = 15, SP_NETHACK_TERMINUS = 34,
       RFLAG_ALIEN = 0x80 };
struct VehPrototype { uint32_t ability_flags; int weapon_id; };
struct AbilityInfo { int preq_tech; };
struct PlayerInfo {
    uint32_t rule_flags;
    int faction_bonus_count;
    int faction_bonus_id[8];
    int faction_bonus_val1[8];
};
static VehPrototype VehPrototypes[512];
static AbilityInfo Ability[64];
static PlayerInfo Players[8];
static int ExpansionEnabled;
static int TechResult;
static int ProjectResult;
static int ChangeBonusOnTech;
static int BonusAfterTech;
static const int MaxVehProtoFactionNum = 64;
static BOOL is_alien_faction(uint32_t faction_id) {
    return ExpansionEnabled && (Players[faction_id].rule_flags & RFLAG_ALIEN);
}
static int has_tech(int, int faction_id) {
    if (ChangeBonusOnTech) {
        Players[faction_id].faction_bonus_val1[0] = BonusAfterTech;
        ChangeBonusOnTech = 0;
    }
    return TechResult;
}
static int has_project(int project, int faction_id) {
    return project == SP_NETHACK_TERMINUS && faction_id == 1 && ProjectResult;
}

FUNCTION_BODY

int main() {
    std::memset(VehPrototypes, 0, sizeof(VehPrototypes));
    std::memset(Ability, 0, sizeof(Ability));
    std::memset(Players, 0, sizeof(Players));
    ExpansionEnabled = 0;
    TechResult = 0;
    ProjectResult = 0;

    // Faction 1 is alien with expansion disabled. The pin still grants Deep Radar.
    Players[1].rule_flags = RFLAG_ALIEN;
    if (!has_abil(64, ABL_DEEP_RADAR)) return 1;

    // Ordinary prototype bits take precedence and absent bits stay absent.
    VehPrototypes[64].ability_flags = 0x40;
    if (!has_abil(64, 0x40) || has_abil(64, 0x20)) return 2;

    // Basic prototypes return after their direct flag test; faction bonuses do not apply.
    Players[0].rule_flags = RFLAG_ALIEN;
    if (has_abil(7, ABL_DEEP_RADAR)) return 3;
    VehPrototypes[7].ability_flags = 0x20;
    if (!has_abil(7, 0x20)) return 4;

    // RULE_FREEABIL keeps its original prerequisite and (1 << ability-index) test.
    Players[2].faction_bonus_count = 1;
    Players[2].faction_bonus_id[0] = RULE_FREEABIL;
    Players[2].faction_bonus_val1[0] = 3;
    Ability[3].preq_tech = 9;
    TechResult = 1;
    if (!has_abil(128, 1 << 3)) return 5;
    TechResult = 0;
    if (has_abil(128, 1 << 3)) return 6;

    // The pin reloads faction_bonus_val1 after has_tech; model a callback mutation.
    Players[2].faction_bonus_val1[0] = 3;
    Ability[3].preq_tech = 9;
    Ability[4].preq_tech = 10;
    TechResult = 1;
    BonusAfterTech = 4;
    ChangeBonusOnTech = 1;
    if (!has_abil(128, 1 << 4)) return 7;

    // Probe-team algorithm enhancement remains available through the project fallback.
    VehPrototypes[64].weapon_id = WPN_PROBE_TEAM;
    ProjectResult = 1;
    if (!has_abil(64, ABL_ALGO_ENHANCEMENT)) return 8;
    return 0;
}
'''


def _program(definition: str) -> str:
    return SUPPORT.replace("FUNCTION_BODY", definition)


def _compile_and_run(source: str, tmp_path: Path) -> subprocess.CompletedProcess[str]:
    compiler = shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        pytest.skip("a host C++ compiler is unavailable")
    cpp = tmp_path / "has_abil_actual_source.cpp"
    binary = tmp_path / "has_abil_actual_source"
    cpp.write_text(source, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror", "-Wno-unused-function",
         str(cpp), "-o", str(binary)],
        capture_output=True, text=True, timeout=45,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True, timeout=45)


def test_actual_has_abil_preserves_alien_flag_and_fallback_paths(tmp_path: Path) -> None:
    definition = _function(SOURCE.read_text(encoding="utf-8"))
    result = _compile_and_run(_program(definition), tmp_path)
    assert result.returncode == 0, result.stdout + result.stderr


def test_old_expansion_gated_helper_mutant_is_rejected(tmp_path: Path) -> None:
    definition = _function(SOURCE.read_text(encoding="utf-8"))
    direct_test = "(Players[faction_id].rule_flags & RFLAG_ALIEN)"
    assert definition.count(direct_test) == 1
    mutant = definition.replace(direct_test, "is_alien_faction(faction_id)", 1)
    result = _compile_and_run(_program(mutant), tmp_path)
    assert result.returncode != 0, "the old ExpansionEnabled-gated helper mutant passed"


def test_cached_bonus_index_mutant_is_rejected(tmp_path: Path) -> None:
    definition = _function(SOURCE.read_text(encoding="utf-8"))
    live_read = """if (has_tech(Ability[Players[faction_id].faction_bonus_val1[i]].preq_tech, faction_id) &&
                (ability_id & (1 << Players[faction_id].faction_bonus_val1[i]))) {"""
    cached = """int abil_bonus_id = Players[faction_id].faction_bonus_val1[i];
            if (has_tech(Ability[abil_bonus_id].preq_tech, faction_id) &&
                (ability_id & (1 << abil_bonus_id))) {"""
    assert definition.count(live_read) == 1
    mutant = definition.replace(live_read, cached, 1)
    result = _compile_and_run(_program(mutant), tmp_path)
    assert result.returncode != 0, "the cached pre-call bonus index mutant passed"
