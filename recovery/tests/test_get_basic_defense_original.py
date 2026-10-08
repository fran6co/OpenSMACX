"""Complete actual defense body on valid IDs, not native/runtime proof.

The original invalid-ID read has no portable expected outcome; check its
unguarded source shape without executing that path.
"""
from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src" / "veh.cpp"
SIGNATURE = "int __cdecl get_basic_defense(int veh_id_def, int veh_id_atk"
HQ_MUTANT = '''        if (has_fac_built(FAC_HEADQUARTERS, base_id_def)) {
            morale++;
        }
'''


def extract_function(text: str) -> str:
    start = text.index(SIGNATURE)
    opening = text.index("{", start)
    depth = 0
    state = "code"
    i = opening
    while i < len(text):
        c = text[i]
        n = text[i + 1] if i + 1 < len(text) else ""
        if state == "code":
            if c == "/" and n == "/":
                state, i = "line", i + 2
                continue
            if c == "/" and n == "*":
                state, i = "block", i + 2
                continue
            if c == '"':
                state = "string"
            elif c == "'":
                state = "character"
            elif c == "{":
                depth += 1
            elif c == "}":
                depth -= 1
                if depth == 0:
                    return text[start:i + 1]
        elif state == "line":
            if c == "\n":
                state = "code"
        elif state == "block":
            if c == "*" and n == "/":
                state, i = "code", i + 2
                continue
        elif state in ("string", "character"):
            if c == "\\":
                i += 2
                continue
            if (state == "string" and c == '"') or (state == "character" and c == "'"):
                state = "code"
        i += 1
    raise AssertionError("unclosed source function")


def require_original_plan_read(body):
    assert "veh_id_atk < 0 ||" not in body
    assert "&& VehPrototypes[Vehs[veh_id_atk].proto_id].plan != PLAN_INFO_WARFARE" in body


def test_extra_attacker_guard_damage_is_rejected_without_invalid_id_execution():
    body = extract_function(SOURCE.read_text())
    require_original_plan_read(body)
    anchor = "&& VehPrototypes[Vehs[veh_id_atk].proto_id].plan != PLAN_INFO_WARFARE"
    assert body.count(anchor) == 1
    damaged = body.replace(anchor,
        "&& (veh_id_atk < 0 || VehPrototypes[Vehs[veh_id_atk].proto_id].plan != PLAN_INFO_WARFARE)", 1)
    with pytest.raises(AssertionError):
        require_original_plan_read(damaged)


HOST = r'''#include <assert.h>
#include <stdint.h>
#define __cdecl
typedef int BOOL;
enum {
    MaxVehProtoFactionNum = 100,
    BSC_SPORE_LAUNCHER = 63,
    BSC_FUNGAL_TOWER = 62,
    PLAN_ALIEN_ARTIFACT = 12,
    PLAN_INFO_WARFARE = 11,
    FAC_CHILDREN_CRECHE = 1,
    FAC_BROOD_PIT = 2,
    FAC_HEADQUARTERS = 3,
    ABL_DISSOCIATIVE_WAVE = 4,
    ABL_SOPORIFIC_GAS = 8
};
struct SocialEffect { enum { MORALE = 4 }; int values_[11]; };
struct PlayerData { SocialEffect soc_effect_active, soc_effect_pending; };
struct Veh { int faction_id, proto_id, x, y; };
struct VehPrototype { int plan, offense_rating, defense_rating; };
static Veh Vehs[8] = {};
static VehPrototype VehPrototypes[8] = {};
static PlayerData PlayersData[8] = {};
static int VehBasicBattleMorale[2] = {};
static int test_base_id = -1;
static int test_headquarters_built = 0;
static int armor_calls, psi_calls;
static int morale_veh(int, bool, int) { return 4; }
static int morale_alien(int, int) { return 4; }
static int base_at(int, int) { return test_base_id; }
static int has_fac_built(int facility, int) {
    return facility == FAC_HEADQUARTERS && test_headquarters_built;
}
static int has_fac_built_call(int, int) { return 0; }
static int range(int v, int lo, int hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}
static int get_proto_offense_rating(int p) { return VehPrototypes[p].offense_rating; }
static int get_proto_defense_rating(int p) { return VehPrototypes[p].defense_rating; }
static int has_abil(int, int) { return 0; }
static int armor_proto(int, int, int) { ++armor_calls; return 2; }
static int psi_factor(uint32_t v, uint32_t, bool, bool) {
    ++psi_calls; return static_cast<int>(v);
}
'''

MAIN = r'''
int main() {
    Vehs[1] = {1, 1, 10, 20};
    Vehs[2] = {2, 2, 10, 20};
    VehPrototypes[1] = {PLAN_INFO_WARFARE, 0, 1};
    VehPrototypes[2] = {2, 0, 0};

    armor_calls = psi_calls = 0;
    assert(get_basic_defense(1, 2, 0, 0) == 1);
    assert(VehBasicBattleMorale[1] == 4);
    assert(armor_calls == 0 && psi_calls == 0);

    VehPrototypes[2].plan = PLAN_INFO_WARFARE;
    armor_calls = psi_calls = 0;
    assert(get_basic_defense(1, 2, 0, 0) == 20);
    assert(VehBasicBattleMorale[1] == 4);
    assert(armor_calls == 1 && psi_calls == 0);

    VehPrototypes[1].plan = 2;
    armor_calls = psi_calls = 0;
    assert(get_basic_defense(1, 2, 1, 0) == 20);
    assert(armor_calls == 1 && psi_calls == 1);

    // Pinned pending morale predicate is inclusive at 2 and 3 only.
    test_base_id = 3;
    for (int pending = 1; pending <= 4; ++pending) {
        PlayersData[1].soc_effect_pending.values_[SocialEffect::MORALE] = pending;
        armor_calls = psi_calls = 0;
        const int bonus = pending >= 2 && pending <= 3 ? 1 : 0;
        assert(get_basic_defense(1, 2, 0, 0) == (4 + bonus + 6) * 2);
        assert(VehBasicBattleMorale[1] == 4 + bonus);
        assert(armor_calls == 1 && psi_calls == 0);
    }

    // The injected HQ mutant changes valid behavior; the source body does not.
    test_headquarters_built = 1;
    PlayersData[1].soc_effect_pending.values_[SocialEffect::MORALE] = 0;
    armor_calls = psi_calls = 0;
    const int faithful = get_basic_defense(1, 2, 0, 0);
    const int hq_mutant = get_basic_defense_with_hq(1, 2, 0, 0);
    assert(faithful == 20 && hq_mutant == 22);
    assert(armor_calls == 2 && psi_calls == 0);
}
'''


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_tracked_ready_complete_actual_body(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")
    body = extract_function(SOURCE.read_text())
    assert "has_fac_built(FAC_HEADQUARTERS, base_id_def)" not in body
    assert "veh_id_atk < 0 ||" not in body
    assert "VehPrototypes[Vehs[veh_id_atk].proto_id].plan != PLAN_INFO_WARFARE" in body
    assert "morale_pending >= 2 && morale_pending <= 3" in body
    assert body.count("int __cdecl get_basic_defense(") == 1
    assert body.count("int __cdecl get_basic_defense_with_hq(") == 0

    anchor = "        int morale_pending = PlayersData[faction_id_def].soc_effect_pending.values_[SocialEffect::MORALE];\n"
    assert body.count(anchor) == 1
    mutant = body.replace(anchor, HQ_MUTANT + anchor, 1)
    mutant = mutant.replace(
        "int __cdecl get_basic_defense(",
        "int __cdecl get_basic_defense_with_hq(", 1,
    )
    cpp_path = tmp_path / "get_basic_defense_tracked.cpp"
    binary_path = tmp_path / f"get-basic-defense-tracked-{compiler_name}"
    cpp_path.write_text(HOST + body + mutant + MAIN)
    build = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
         str(cpp_path), "-o", str(binary_path)],
        capture_output=True, text=True, timeout=60, check=False,
    )
    assert build.returncode == 0, build.stdout + build.stderr
    run = subprocess.run([str(binary_path)], capture_output=True, text=True,
                         timeout=30, check=False)
    assert run.returncode == 0, run.stdout + run.stderr
