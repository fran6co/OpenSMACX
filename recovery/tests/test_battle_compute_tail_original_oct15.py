"""Bounded host controls for battle_compute's original attacker-sign gates.

The actual Trance statement, complete guarded armor-through-Tracking tail, and
common output stores are extracted from src/veh.cpp. This exercises that
bounded production span, not the full battle-computation function.
"""

from pathlib import Path
import shutil
import subprocess
import tempfile

import pytest


ROOT = Path(__file__).resolve().parents[2]
OWNER = ROOT / "src" / "veh.cpp"


def _balanced(source: str, opening: int, left: str, right: str) -> int:
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == left:
            depth += 1
        elif source[index] == right:
            depth -= 1
            if depth == 0:
                return index + 1
    raise AssertionError("unterminated production delimiter")


def _function_body(source: str) -> str:
    signature = source.index("void __cdecl battle_compute(")
    opening = source.index("{", signature)
    end = _balanced(source, opening, "{", "}")
    return source[opening + 1:end - 1]


def _if_statement(source: str, start: int) -> tuple[str, str, int]:
    opening = source.index("(", start)
    condition_end = _balanced(source, opening, "(", ")")
    condition = source[opening + 1:condition_end - 1]
    brace = source.index("{", condition_end)
    end = _balanced(source, brace, "{", "}")
    return condition, source[brace + 1:end - 1], end


def _production_parts():
    body = _function_body(OWNER.read_text(encoding="utf-8"))
    trance_start = body.index("if (!has_abil(Vehs[veh_id_atk].proto_id, ABL_DISSOCIATIVE_WAVE)")
    trance_condition, _, trance_end = _if_statement(body, trance_start)
    trance_statement = body[trance_start:trance_end]
    assert "veh_id_atk >= 0" in trance_condition
    assert trance_condition.index("Vehs[veh_id_atk].proto_id") < trance_condition.index("veh_id_atk >= 0")

    armor_start = body.index("uint32_t armor_id_def =", trance_end)
    guard_start = body.rindex("if (veh_id_atk >= 0) {", 0, armor_start)
    guard_condition, guarded_tail, guard_end = _if_statement(body, guard_start)
    assert guard_condition.strip() == "veh_id_atk >= 0"
    assert "uint32_t armor_id_def =" in guarded_tail
    assert 'label_get(337)); // "Tracking"' in guarded_tail
    assert "veh_id_atk >= 0" not in guarded_tail
    assert guard_end < body.index("} else if (veh_id_def >= 0)", guard_end)

    output_start = body.index("if (offense_out)")
    output_stores = body[output_start:]
    assert "*offense_out = offense;" in output_stores
    assert "*defense_out = defense;" in output_stores
    return trance_statement, guard_condition, guarded_tail, output_stores


def _compilers():
    found = [shutil.which(name) for name in ("g++", "clang++")]
    found = [compiler for compiler in found if compiler]
    if not found:
        pytest.skip("neither GCC nor Clang is installed")
    return found


def _fixture(trance_statement: str, guard_condition: str, guarded_tail: str,
             output_stores: str, *, cached_proto_mutant: bool = False,
             remove_trance_sign_gate: bool = False):
    if cached_proto_mutant:
        old = "Vehs[veh_id_atk].proto_id"
        assert trance_statement.count(old) == 1
        trance_statement = trance_statement.replace(old, "proto_id_atk", 1)
    if remove_trance_sign_gate:
        old = "&& Rules.combat_pct_trance_def_vs_psi && veh_id_atk >= 0"
        assert old in trance_statement
        trance_statement = trance_statement.replace(
            old, "&& Rules.combat_pct_trance_def_vs_psi", 1)

    return r'''#include <assert.h>
#include <string.h>
typedef unsigned int uint32_t;
struct Vehicle { int proto_id; };
struct Prototype {
    int armor_id, chassis_id, triad, offense_rating, defense_rating;
};
struct ChassisType { int speed; };
struct RulesType {
    int combat_pct_trance_def_vs_psi;
    int combat_pen_pct_air_supr_vs_grnd;
    int combat_pct_air_supr_vs_air;
    int combat_pen_pct_non_cbt_def_vs_cbt;
    int combat_pct_bonus_vs_ship_port;
    int combat_pct_com_jam_def_vs_mobl;
    int combat_pct_aaa_bonus_vs_air;
};
static Vehicle vehicle_rows[3];
static Vehicle *Vehs = &vehicle_rows[1];
static Prototype prototype_rows[32];
static Prototype *VehPrototypes = &prototype_rows[1];
static ChassisType Chassis[8];
static RulesType Rules;
static bool abilities[32][8];
static int label_counts[1200];
static int ability_protos[16];
static int ability_count;
static int current_label;
enum {
    ABL_DISSOCIATIVE_WAVE = 0, ABL_TRANCE = 1, ABL_AIR_SUPERIORITY = 2,
    ABL_COMM_JAMMER = 3, ABL_AAA = 4,
    ARM_RESONANCE_3_ARMOR = 10, ARM_RESONANCE_8_ARMOR = 11,
    ARM_PULSE_3_ARMOR = 12, ARM_PULSE_8_ARMOR = 13,
    TRIAD_LAND = 1, TRIAD_SEA = 2, TRIAD_AIR = 3
};
static bool has_abil(int proto, int ability) {
    (void)ability;
    ability_protos[ability_count++] = proto;
    return abilities[proto][ability];
}
static uint32_t get_proto_triad(int proto) { return (uint32_t)VehPrototypes[proto].triad; }
static int get_proto_offense_rating(int proto) { return VehPrototypes[proto].offense_rating; }
static int get_proto_defense_rating(int proto) { return VehPrototypes[proto].defense_rating; }
static bool is_proto_missile(int) { return false; }
static const char *label_get(int label) { current_label = label; return "label"; }
static void add_bat(int, int, const char *) { ++label_counts[current_label]; }

static void reset_state() {
    memset(vehicle_rows, 0, sizeof(vehicle_rows));
    memset(prototype_rows, 0, sizeof(prototype_rows));
    memset(Chassis, 0, sizeof(Chassis));
    memset(&Rules, 0, sizeof(Rules));
    memset(abilities, 0, sizeof(abilities));
    memset(label_counts, 0, sizeof(label_counts));
    memset(ability_protos, 0, sizeof(ability_protos));
    ability_count = 0;
    vehicle_rows[0].proto_id = 3;
    vehicle_rows[2].proto_id = 3;
    VehPrototypes[3].chassis_id = 1;
    VehPrototypes[3].triad = TRIAD_LAND;
    VehPrototypes[3].offense_rating = 1;
    VehPrototypes[5].chassis_id = 1;
    VehPrototypes[5].triad = TRIAD_LAND;
    VehPrototypes[5].offense_rating = 1;
    VehPrototypes[5].defense_rating = 1;
    Chassis[1].speed = 2;
}

static void run_case(int veh_id_atk, int proto_id_atk, int proto_id_def,
                     int faction_id_atk, int base_id_def, int psi_combat_type,
                     int &offense, int &defense) {
    (void)Vehs;
    ''' + trance_statement + r'''
    if (''' + guard_condition + r''') {
    ''' + guarded_tail + r'''
    }
}

static void store_outputs(int offense, int defense, int *offense_out,
                          int *defense_out) {
    ''' + output_stores + r'''
}

int main() {
    int offense = 100, defense = 100;
    int offense_out = -1, defense_out = -1;
    reset_state();
    VehPrototypes[5].armor_id = ARM_RESONANCE_3_ARMOR;
    Rules.combat_pct_trance_def_vs_psi = 20;
    abilities[5][ABL_TRANCE] = true;
    abilities[4][ABL_DISSOCIATIVE_WAVE] = true; // stale cache deliberately disagrees with Vehs[-1].
    run_case(-1, 4, 5, 1, -1, 1, offense, defense);
    assert(ability_count == 1 && ability_protos[0] == 3);
    assert(defense == 100 && offense == 100);
    assert(label_counts[329] == 0 && label_counts[1111] == 0);
    store_outputs(offense, defense, &offense_out, &defense_out);
    assert(offense_out == 100 && defense_out == 100);

    reset_state();
    VehPrototypes[5].armor_id = ARM_RESONANCE_3_ARMOR;
    Rules.combat_pct_trance_def_vs_psi = 20;
    abilities[5][ABL_TRANCE] = true;
    offense = 100; defense = 100;
    run_case(1, 3, 5, 1, -1, 1, offense, defense);
    assert(ability_count >= 2 && ability_protos[0] == 3 && ability_protos[1] == 5);
    assert(defense == 150 && label_counts[329] == 1 && label_counts[1111] == 1);

    reset_state();
    VehPrototypes[3].triad = TRIAD_AIR;
    VehPrototypes[5].triad = TRIAD_LAND;
    abilities[3][ABL_AIR_SUPERIORITY] = true;
    Rules.combat_pen_pct_air_supr_vs_grnd = 10;
    offense = 100; defense = 100;
    run_case(1, 3, 5, 1, -1, 0, offense, defense);
    assert(offense == 90 && label_counts[448] == 1);

    reset_state();
    VehPrototypes[5].offense_rating = 0;
    VehPrototypes[5].defense_rating = 1;
    Rules.combat_pen_pct_non_cbt_def_vs_cbt = 10;
    offense = 100; defense = 100;
    run_case(-1, 4, 5, 1, -1, 0, offense, defense);
    assert(defense == 100 && label_counts[439] == 0);
    offense = 100; defense = 100;
    run_case(1, 3, 5, 1, -1, 0, offense, defense);
    assert(defense == 90 && label_counts[439] == 1);
    return 0;
}
'''


def _compile_run(compiler: str, fixture: str, *, should_pass: bool):
    with tempfile.TemporaryDirectory(prefix="osmx-battle-compute-tail-") as temp:
        cpp = Path(temp) / "actual_source.cpp"
        exe = Path(temp) / "actual_source"
        cpp.write_text(fixture, encoding="utf-8")
        built = subprocess.run(
            [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(exe)],
            capture_output=True, text=True, timeout=30,
        )
        assert built.returncode == 0, compiler + " compile failed:\n" + built.stdout + built.stderr
        ran = subprocess.run([str(exe)], capture_output=True, text=True, timeout=5)
        if should_pass:
            assert ran.returncode == 0, compiler + " runtime assertion failed"
        else:
            assert ran.returncode != 0, compiler + " mutant unexpectedly passed"


def test_actual_trance_tail_and_output_stores_preserve_original_gates():
    trance, guard, tail, outputs = _production_parts()
    assert tail.index("uint32_t armor_id_def") < tail.index('label_get(337)); // "Tracking"')
    for compiler in _compilers():
        _compile_run(compiler, _fixture(trance, guard, tail, outputs), should_pass=True)


@pytest.mark.parametrize("mutant", ["cached_proto", "ungated_tail", "trance_without_sign"])
def test_compiling_old_gate_mutants_fail_runtime(mutant):
    trance, guard, tail, outputs = _production_parts()
    fixture = _fixture(
        trance, "true" if mutant == "ungated_tail" else guard, tail, outputs,
        cached_proto_mutant=mutant == "cached_proto",
        remove_trance_sign_gate=mutant == "trance_without_sign",
    )
    for compiler in _compilers():
        _compile_run(compiler, fixture, should_pass=False)
