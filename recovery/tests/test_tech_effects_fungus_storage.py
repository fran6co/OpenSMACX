"""Bounded actual-source tech_effects four-slot host control.

The synthetic PlayerData keeps the supported four-int storage model. Its
unknown fourth slot stays unnamed; this test does not assign it a resource
meaning or claim game-runtime behavior.
"""
from pathlib import Path
import re
import resource
import shutil
import subprocess

import pytest
from match_recovery.source import read_file


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/technology_queries.cpp"
ADDRESS = 0x005BAE60
SIGNATURE = "void __cdecl tech_effects(int faction_id) {"


def _definition(region):
    start = region.find(SIGNATURE)
    assert start >= 0, "mapped tech_effects definition missing"
    opening = region.find("{", start, start + len(SIGNATURE) + 1)
    assert opening >= 0
    depth = 0
    state = "code"
    i = opening
    while i < len(region):
        char = region[i]
        following = region[i + 1] if i + 1 < len(region) else ""
        if state == "code":
            if char == "/" and following == "/":
                state = "line"
                i += 2
                continue
            if char == "/" and following == "*":
                state = "block"
                i += 2
                continue
            if char == '"':
                state = "string"
            elif char == "'":
                state = "char"
            elif char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return region[start:i + 1]
        elif state == "line":
            if char == "\n":
                state = "code"
        elif state == "block":
            if char == "*" and following == "/":
                state = "code"
                i += 2
                continue
        else:
            if char == "\\":
                i += 2
                continue
            if ((state == "string" and char == '"')
                    or (state == "char" and char == "'")):
                state = "code"
        i += 1
    raise AssertionError("unterminated tech_effects definition")


def _enum(path, name):
    source = (ROOT / path).read_text(encoding="utf-8")
    match = re.search(r"enum\s+" + re.escape(name) + r"\s*\{.*?\};",
                      source, re.DOTALL)
    assert match, f"production enum {name} missing"
    return match.group(0)


def _production_constants():
    technology = (ROOT / "src/technology.h").read_text(encoding="utf-8")
    count = re.search(r"static\s+const\s+int\s+MaxTechnologyNum\s*=\s*(\d+)\s*;",
                      technology)
    assert count, "production MaxTechnologyNum declaration missing"
    faction_header = (ROOT / "src/faction.h").read_text(encoding="utf-8")
    effect = re.search(r"struct\s+SocialEffect\s*\{.*?\n\};",
                       faction_header, re.DOTALL)
    assert effect, "production SocialEffect declaration missing"
    return "\n".join((
        _enum("src/map.h", "ResourceType"),
        _enum("src/technology.h", "TechnologyFlagBitfield"),
        _enum("src/faction.h", "FactionSpecialRules"),
        "static const int MaxTechnologyNum = " + count.group(1) + ";",
        effect.group(0),
    ))


def actual_body():
    records = [row for row in read_file(SOURCE) if row.address == ADDRESS]
    assert len(records) == 1 and not records[0].extract_error
    body = _definition(records[0].region)
    assert "tech_fungus_[4]" in (ROOT / "src/faction.h").read_text(encoding="utf-8")
    assert "tech_fungus_[RSC_NUTRIENTS]" in body
    assert "tech_fungus_[RSC_MINERALS]" in body
    assert "tech_fungus_[RSC_ENERGY]" in body
    return body


PREAMBLE = r"""
#include <cassert>
#include <cstdint>
#include <cstring>
#define __cdecl

struct Player {
    int rule_commerce;
    int faction_bonus_count;
    int faction_bonus_id[8];
    int faction_bonus_val1[8];
};
struct PlayerData {
    int before_slots;
    int tech_commerce_bonus;
    int tech_fungus_[4];
    int after_slots;
    SocialEffect soc_effect_pending;
};
struct RulesTechnology { uint32_t flags; };

static Player Players[8];
static PlayerData PlayersData[8];
static RulesTechnology Technology[MaxTechnologyNum];
static uint8_t OwnedTech[8][MaxTechnologyNum];

int __cdecl has_tech(int tech_id, int faction_id) {
    assert(tech_id >= 0 && tech_id < MaxTechnologyNum);
    assert(faction_id >= 0 && faction_id < 8);
    return OwnedTech[faction_id][tech_id] != 0;
}

static void clear_inputs() {
    std::memset(Players, 0, sizeof Players);
    std::memset(PlayersData, 0, sizeof PlayersData);
    std::memset(Technology, 0, sizeof Technology);
    std::memset(OwnedTech, 0, sizeof OwnedTech);
}
"""


MAIN = r"""
int main() {
    clear_inputs();
    const int faction = 2;
    Players[faction].rule_commerce = 7;
    PlayersData[faction].before_slots = 0x13579;
    PlayersData[faction].after_slots = 0x2468A;
    PlayersData[faction].tech_commerce_bonus = 999;
    PlayersData[faction].tech_fungus_[0] = 101;
    PlayersData[faction].tech_fungus_[1] = -202;
    PlayersData[faction].tech_fungus_[2] = 303;
    PlayersData[faction].tech_fungus_[3] = 404; // Positive: clamping cannot hide a missed clear.
    PlayersData[faction].soc_effect_pending.values_[SocialEffect::ECONOMY] = 5;

    Technology[4].flags = TFLAG_INC_ENERGY_FUNGUS | TFLAG_INC_COMMERCE;
    Technology[9].flags = TFLAG_INC_NUTRIENT_FUNGUS;
    Technology[17].flags = TFLAG_INC_NUTRIENT_FUNGUS | TFLAG_INC_MINERALS_FUNGUS;
    OwnedTech[faction][4] = OwnedTech[faction][9] = OwnedTech[faction][17] = 1;
    Players[faction].faction_bonus_count = 5;
    Players[faction].faction_bonus_id[0] = RULE_FUNGNUTRIENT;
    Players[faction].faction_bonus_val1[0] = 4;
    Players[faction].faction_bonus_id[1] = RULE_FUNGNUTRIENT;
    Players[faction].faction_bonus_val1[1] = 1;
    Players[faction].faction_bonus_id[2] = RULE_FUNGMINERALS;
    Players[faction].faction_bonus_val1[2] = -3;
    Players[faction].faction_bonus_id[3] = RULE_FUNGENERGY;
    Players[faction].faction_bonus_val1[3] = 2;
    Players[faction].faction_bonus_id[4] = 99; // Unrecognized bonus remains ignored.
    Players[faction].faction_bonus_val1[4] = 1000;
    PlayersData[0].tech_fungus_[0] = 71; // Other factions remain untouched.

    tech_effects(faction);
    assert(PlayersData[faction].tech_fungus_[RSC_NUTRIENTS] == 7);
    assert(PlayersData[faction].tech_fungus_[RSC_MINERALS] == 0); // signed negative clamp
    assert(PlayersData[faction].tech_fungus_[RSC_ENERGY] == 3);
    assert(PlayersData[faction].tech_fungus_[3] == 0); // unknown slot, clear/clamp only
    assert(PlayersData[faction].tech_commerce_bonus == 11); // base 7 + tech 1 + economy 3
    assert(PlayersData[faction].soc_effect_pending.values_[SocialEffect::ECONOMY] == 5);
    assert(PlayersData[faction].before_slots == 0x13579);
    assert(PlayersData[faction].after_slots == 0x2468A);
    assert(PlayersData[0].tech_fungus_[0] == 71);

    clear_inputs();
    Players[1].rule_commerce = -2;
    PlayersData[1].before_slots = 0x13579;
    PlayersData[1].after_slots = 0x2468A;
    PlayersData[1].tech_fungus_[0] = 31;
    PlayersData[1].tech_fungus_[1] = 32;
    PlayersData[1].tech_fungus_[2] = 33;
    PlayersData[1].tech_fungus_[3] = -34;
    Players[1].faction_bonus_count = 3;
    Players[1].faction_bonus_id[0] = RULE_FUNGNUTRIENT;
    Players[1].faction_bonus_val1[0] = -7;
    Players[1].faction_bonus_id[1] = RULE_FUNGMINERALS;
    Players[1].faction_bonus_val1[1] = -5;
    Players[1].faction_bonus_id[2] = RULE_FUNGENERGY;
    Players[1].faction_bonus_val1[2] = -2;
    tech_effects(1);
    assert(PlayersData[1].tech_fungus_[RSC_NUTRIENTS] == 0);
    assert(PlayersData[1].tech_fungus_[RSC_MINERALS] == 0);
    assert(PlayersData[1].tech_fungus_[RSC_ENERGY] == 0);
    assert(PlayersData[1].tech_fungus_[3] == 0);
    assert(PlayersData[1].tech_commerce_bonus == -2);
    assert(PlayersData[1].before_slots == 0x13579);
    assert(PlayersData[1].after_slots == 0x2468A);
}
"""


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
@pytest.mark.parametrize("damage", [None, "short_clear", "wrong_clamp_threshold"])
def test_actual_tech_effects_four_slot_array(compiler, damage, tmp_path):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(compiler + " unavailable")
    body = actual_body()
    changes = {
        "short_clear": ("sizeof PlayersData[faction_id].tech_fungus_", "3 * sizeof(int)"),
        "wrong_clamp_threshold": ("value > 0 ? value : 0", "value > -100 ? value : 0"),
    }
    if damage:
        old, new = changes[damage]
        assert body.count(old) == 1
        body = body.replace(old, new, 1)
    source = tmp_path / "tech_effects_array.cpp"
    binary = tmp_path / "tech_effects_array"
    source.write_text(_production_constants() + PREAMBLE + body + "\n" + MAIN)
    compiled = subprocess.run(
        [executable, "-std=c++11", "-O2", "-Wall", "-Wextra", "-Werror",
         str(source), "-o", str(binary)], capture_output=True, text=True, timeout=60)
    assert compiled.returncode == 0, compiled.stderr
    def no_core():
        resource.setrlimit(resource.RLIMIT_CORE, (0, 0))
    result = subprocess.run([str(binary)], capture_output=True, text=True,
                            timeout=10, cwd=tmp_path, preexec_fn=no_core)
    assert (result.returncode == 0) == (damage is None), result.stderr
