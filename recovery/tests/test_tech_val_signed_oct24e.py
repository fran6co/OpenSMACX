"""Bounded signed-operation controls over exact source blocks in tech_val.

This does not emulate the whole tech_val calculation or define its overflow
behavior. It compiles the actual mapped gate and regional-weight blocks.
"""

from pathlib import Path
import shutil
import subprocess

import pytest
from match_recovery.source import read_file


ROOT = Path(__file__).resolve().parents[2]


def _braced(text, start):
    opening = text.index("{", start)
    depth = 0
    for end in range(opening, len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            return text[start:end + 1]
    raise AssertionError("unterminated actual-source block")


def _actual_blocks():
    records = [r for r in read_file(ROOT / "src/technology_queries.cpp")
               if r.address_hex == "0x005BCBE0"]
    assert len(records) == 1
    source = records[0].region
    assert "int base_count = PlayersData[faction_id].current_num_bases;" in source
    assert "int pwr_base = PlayersData[faction_id].region_total_bases[region] * power;" in source
    gate_start = source.index("if ((!power || !ai_power && !vendetta_count)")
    gate = _braced(source, gate_start)
    region_start = source.index("if (base_count) {")
    regional = _braced(source, region_start)
    return gate, regional


def _fixture(gate, regional, *, unsigned_gate=False, unsigned_product=False):
    if unsigned_gate:
        # A sensitivity control for the pre-correction local type. The
        # conversion from negative int to uint32_t is defined modulo 2^32.
        gate_decl = "uint32_t base_count = static_cast<uint32_t>(count);"
    else:
        gate_decl = "int base_count = count;"
    if unsigned_product:
        regional = regional.replace(
            "int pwr_base = PlayersData[faction_id].region_total_bases[region] * power;",
            "uint32_t pwr_base = PlayersData[faction_id].region_total_bases[region] * power;")
        assert "uint32_t pwr_base =" in regional

    return r"""
#include <cassert>
#include <cstdint>
typedef int BOOL;
const int MaxRegionLandNum = 2;
const int MaxPlayerNum = 2;
const int PLAN_NAVAL_TRANSPORT = 1;
const int PLAN_DEFENSIVE = 2;
const int PLAN_OFFENSIVE = 3;
const int DTREATY_COMMLINK = 1;
const int DTREATY_PACT = 2;
const int DTREATY_TREATY = 4;
const int DTREATY_WANT_REVENGE = 8;
struct PlayerData {
    uint32_t current_num_bases;
    uint8_t region_total_bases[128];
    uint8_t region_base_plan[128];
    int best_weapon_value;
    int enemy_best_weapon_value;
};
PlayerData PlayersData[2];
int bad_reg_calls;
int bad_reg(int) { ++bad_reg_calls; return 0; }
int has_treaty(int faction, int other, int treaty) {
    assert(faction == 0);
    return other == 1 && treaty == DTREATY_COMMLINK;
}

int gate_case(int count) {
    GATE_DECL
    int value_ret = 9;
    int power = 0, ai_power = 0, vendetta_count = 0;
    int tech = 0, ai_tech = 0, wealth = 0, ai_wealth = 0;
    int growth = 1, ai_growth = 0;
    ACTUAL_GATE
    return value_ret;
}

int regional_case(int plan, int power, int count, int is_human_player) {
    PlayersData[0] = PlayerData();
    PlayersData[1] = PlayerData();
    bad_reg_calls = 0;
    PlayersData[0].region_total_bases[1] = 2;
    PlayersData[1].region_total_bases[1] = 1;
    PlayersData[0].region_base_plan[1] = static_cast<uint8_t>(plan);
    PlayersData[0].best_weapon_value = 5;
    PlayersData[0].enemy_best_weapon_value = 5;
    const int faction_id = 0;
    const int base_count = count;
    const uint32_t vendetta_count = 1;
    int value_ret = 0;
    ACTUAL_REGION
    return value_ret;
}

int main() {
    // The actual gate uses signed >=: -1 and 3 do not halve; 4 does.
    assert(gate_case(-1) == 9);
    assert(gate_case(3) == 9);
    assert(gate_case(4) == 5);

    // Normal positive quotients are retained on all four plan paths.
    assert(regional_case(PLAN_NAVAL_TRANSPORT, 5, 4, 0) == 2);
    assert(regional_case(PLAN_DEFENSIVE, 5, 4, 0) == 10);
    assert(regional_case(PLAN_OFFENSIVE, 5, 4, 0) == 5);
    assert(regional_case(0, 5, 4, 0) == 2);

    // Negative products are in range (no signed overflow). IDIV truncates
    // toward zero; an unsigned-product mutant yields large positive values.
    assert(regional_case(PLAN_NAVAL_TRANSPORT, -3, 4, 0) == -1);
    assert(regional_case(PLAN_DEFENSIVE, -3, 4, 0) == -6);
    assert(regional_case(PLAN_OFFENSIVE, -3, 4, 0) == -3);
    assert(regional_case(0, -3, 4, 0) == -1);

    // Preserve the outer nonzero-base guard: it does not enter bad_reg or
    // consult a region when the count is zero.
    assert(regional_case(PLAN_DEFENSIVE, -3, 0, 0) == 0);
    assert(bad_reg_calls == 0);
}
""".replace("GATE_DECL", gate_decl).replace("ACTUAL_GATE", gate).replace(
    "ACTUAL_REGION", regional)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_tech_val_signed_gate_and_region_blocks(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    gate, regional = _actual_blocks()
    fixture = _fixture(gate, regional)
    cpp = tmp_path / "tech_val_signed.cpp"
    exe = tmp_path / "tech_val_signed"
    cpp.write_text(fixture)
    built = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", "-Wno-parentheses", str(cpp), "-o", str(exe)],
        capture_output=True, text=True, timeout=30,
    )
    assert built.returncode == 0, built.stderr
    ran = subprocess.run([str(exe)], capture_output=True, text=True, timeout=10)
    assert ran.returncode == 0, ran.stderr

    # Sensitivity controls: restore each old unsigned local independently.
    # Both mutated fixtures must fail the defined small-value assertions.
    for name, mutant in (
        ("unsigned_gate", _fixture(gate, regional, unsigned_gate=True)),
        ("unsigned_product", _fixture(gate, regional, unsigned_product=True)),
    ):
        mutant_cpp = tmp_path / (name + ".cpp")
        mutant_exe = tmp_path / name
        mutant_cpp.write_text(mutant)
        mutant_built = subprocess.run(
            [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", "-Wno-parentheses", str(mutant_cpp), "-o", str(mutant_exe)],
            capture_output=True, text=True, timeout=30,
        )
        assert mutant_built.returncode == 0, mutant_built.stderr
        mutant_run = subprocess.run([str(mutant_exe)], capture_output=True, text=True, timeout=10)
        assert mutant_run.returncode != 0, name
