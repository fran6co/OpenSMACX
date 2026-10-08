"""Actual-source controls for morale_veh's pinned drone-riot clamp path.

The complete current definition is extracted without body rewriting. The
correction mutant removes only the evidenced clamp once integrated. Host tests
are not native compiler or game-runtime evidence.
"""

from pathlib import Path
import re
import shutil
import subprocess

import pytest
from match_recovery.includes import COMMENTS_OR_STRINGS


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/veh.cpp"

HARNESS = r"""
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define __cdecl
typedef int BOOL;
enum { PLAN_INFO_WARFARE = 1, MaxVehProtoFactionNum = 64,
       MaxTechnologyNum = 0, TFLAG_IMPROVED_PROBES = 1,
       SP_TELEPATHIC_MATRIX = 2, FAC_CHILDREN_CRECHE = 3, FAC_BROOD_PIT = 4,
       BSC_SPORE_LAUNCHER = 7, BSTATE_DRONE_RIOTS_ACTIVE = 2,
       RFLAG_MORALE = 0x8000 };
enum SocialEffect { PROBE = 0, MORALE = 1 };
struct SocEffects { int values_[2]; };
struct PlayerData { SocEffects soc_effect_active; };
struct Player { int rule_morale; unsigned int rule_flags; };
struct Vehicle { unsigned char faction_id; int16_t proto_id; unsigned char morale; int16_t home_base_id; };
struct Prototype { int plan; };
struct TechnologyRec { unsigned int flags; };
struct Base { unsigned int state; };
PlayerData PlayersData[8];
Player Players[8];
Vehicle Vehs[16];
Prototype VehPrototypes[128];
TechnologyRec Technology[1];
Base Bases[16];

int range(int value, int low, int high) {
    if (value < low) return low;
    if (value > high) return high;
    return value;
}
int morale_alien(int, int) { return 0; }
int has_project(int, int) { return 0; }
int has_tech(int, int) { return 0; }
int8_t get_proto_offense_rating(int) { return 1; }
int has_fac_built_call(int, int) { return 0; }
int has_fac_built(int, int) { return 0; }

ACTUAL_FUNCTION

void reset_state() {
    memset(PlayersData, 0, sizeof(PlayersData));
    memset(Players, 0, sizeof(Players));
    memset(Vehs, 0, sizeof(Vehs));
    memset(VehPrototypes, 0, sizeof(VehPrototypes));
    memset(Bases, 0, sizeof(Bases));
    Vehs[0].faction_id = 1;
    Vehs[0].proto_id = 10;
    Vehs[0].morale = 6;
    Vehs[0].home_base_id = 0;
    PlayersData[1].soc_effect_active.values_[MORALE] = -4;
    Players[1].rule_morale = -1;
}
int main(int argc, char **argv) {
    if (argc != 2) return 90;
    reset_state();
    if (strcmp(argv[1], "eligible") == 0) {
        Bases[0].state = BSTATE_DRONE_RIOTS_ACTIVE;
        return morale_veh(0, 1, 0) == 6 ? 0 : 1;
    }
    if (strcmp(argv[1], "no_check") == 0)
        return morale_veh(0, 0, 0) == 2 ? 0 : 2;
    if (strcmp(argv[1], "no_riot") == 0)
        return morale_veh(0, 1, 0) == 2 ? 0 : 3;
    if (strcmp(argv[1], "morale_flag") == 0) {
        Bases[0].state = BSTATE_DRONE_RIOTS_ACTIVE;
        Players[1].rule_flags = RFLAG_MORALE;
        return morale_veh(0, 1, 0) == 6 ? 0 : 4;
    }
    return 91;
}
"""


def extract_function():
    text = SOURCE.read_text()
    masked = COMMENTS_OR_STRINGS.sub(lambda m: re.sub(r"[^\n]", " ", m[0]), text)
    signature = "int __cdecl morale_veh(int veh_id, BOOL check_drone_riot, int faction_id_vs_native)"
    start = masked.index(signature)
    opening = masked.index("{", start)
    depth = 0
    for pos in range(opening, len(masked)):
        if masked[pos] == "{":
            depth += 1
        elif masked[pos] == "}":
            depth -= 1
            if depth == 0:
                return text[start:pos + 1]
    raise AssertionError("morale_veh body is unterminated")


def source_body(remove_drone_clamp=False):
    body = extract_function()
    if remove_drone_clamp:
        anchor = "        morale_modifier = range(morale_modifier, 0, 6);\n"
        assert body.count(anchor) == 1, "expected the pinned-path clamp in current source"
        body = body.replace(anchor, "", 1)
    return body


def compile_body(tmp_path, remove_drone_clamp=False, compiler_name="g++"):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"host {compiler_name} required")
    unit = tmp_path / "morale_veh_actual.cpp"
    executable = tmp_path / "morale_veh_actual"
    unit.write_text(HARNESS.replace("ACTUAL_FUNCTION", source_body(remove_drone_clamp)))
    result = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", "-pedantic",
         str(unit), "-o", str(executable)], capture_output=True, text=True, timeout=30)
    assert result.returncode == 0, result.stderr
    return executable


@pytest.fixture(scope="module", params=("g++", "clang++"))
def actual_binary(request, tmp_path_factory):
    return compile_body(tmp_path_factory.mktemp("morale-veh-actual"),
                        compiler_name=request.param)


@pytest.mark.parametrize("scenario", ("eligible", "no_check", "no_riot", "morale_flag"))
def test_actual_full_body_drone_clamp(actual_binary, scenario):
    result = subprocess.run([str(actual_binary), scenario], capture_output=True,
                            text=True, timeout=10)
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_missing_drone_clamp_mutant_is_rejected(tmp_path, compiler_name):
    executable = compile_body(tmp_path, remove_drone_clamp=True,
                              compiler_name=compiler_name)
    result = subprocess.run([str(executable), "eligible"], capture_output=True,
                            text=True, timeout=10)
    assert result.returncode != 0, "removing the original modifier clamp escaped control"
