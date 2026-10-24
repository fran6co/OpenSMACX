"""Controls over the exact source faction-value branch in tech_val.

This tests only the two predicate calls, factor updates, and signed quotient
block. It does not assert invalid ai_fight values are valid game states.
"""

from pathlib import Path
import shutil
import subprocess

import pytest
from match_recovery.source import read_file


ROOT = Path(__file__).resolve().parents[2]


def _actual_branch():
    records = [r for r in read_file(ROOT / "src/technology_queries.cpp")
               if r.address_hex == "0x005BCBE0"]
    assert len(records) == 1
    source = records[0].region
    assert "int factor = 1;" in source
    assert "uint32_t factor = 1;" not in source
    start = source.index("int factor = 1;")
    end = source.index("    } else {  // prototypes", start)
    branch = source[start:end]
    assert "wants_to_attack(faction_id, faction_id_2, 0)" in branch
    assert "wants_to_attack(faction_id_2, faction_id, 0)" in branch
    assert "factor / (PlayersData[faction_id].ai_fight + 2)" in branch
    return branch


def _fixture(branch, *, unsigned_factor=False):
    if unsigned_factor:
        branch = branch.replace("int factor = 1;", "uint32_t factor = 1;")
        assert "uint32_t factor = 1;" in branch
    return r"""
#include <cassert>
#include <cstdint>
const int MaxTechnologyNum = 89;
struct PlayerData { int ai_fight; };
PlayerData PlayersData[8];
int response[2];
int call_count;
int call_actor[2], call_target[2], call_mode[2];
int wants_to_attack(int actor, int target, int mode) {
    assert(call_count < 2);
    call_actor[call_count] = actor;
    call_target[call_count] = target;
    call_mode[call_count] = mode;
    return response[call_count++];
}
int calculate(int first, int second, int ai_fight) {
    const int faction_id = first;
    const int tech_id = MaxTechnologyNum + second;
    PlayersData[faction_id].ai_fight = ai_fight;
    call_count = 0;
    int value_ret = 0;
    ACTUAL_BRANCH
    assert(call_count == 2);
    assert(call_actor[0] == first && call_target[0] == second && call_mode[0] == 0);
    assert(call_actor[1] == second && call_target[1] == first && call_mode[1] == 0);
    return value_ret;
}
int run(int a, int b, int ai, int expected) {
    response[0] = a;
    response[1] = b;
    const int value = calculate(4, 3, ai);
    assert(value == expected);
    return value;
}
int main() {
    // Expected ordinary faction state range from PlayerData::ai_fight.
    // For factors 1, 2, 3, respectively:
    //   ai_fight -1 (denominator 1): 1, 4, 9
    //   ai_fight  0 (denominator 2): 0, 2, 3
    //   ai_fight  1 (denominator 3): 0, 0, 3
    run(1, 1, -1, 1); run(1, 1, 0, 0); run(1, 1, 1, 0);
    run(0, 1, -1, 4); run(0, 1, 0, 2); run(0, 1, 1, 0);
    run(1, 0, -1, 4); run(1, 0, 0, 2); run(1, 0, 1, 0);
    run(0, 0, -1, 9); run(0, 0, 0, 3); run(0, 0, 1, 3);

    // ai_fight=-3 gives denominator -1. This is outside the documented
    // normal range, but scenario loading copies the integer without a clamp.
    // It is a bounded operation diagnostic, not a claim that this is a valid
    // game state. Signed IDIV yields -1/-4/-9; the unsigned-factor mutant
    // yields zero. All arithmetic here remains within signed 32-bit range.
    run(1, 1, -3, -1);
    run(0, 1, -3, -4);
    run(0, 0, -3, -9);
}
""".replace("ACTUAL_BRANCH", branch)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_tech_val_faction_factor_signed_division(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    branch = _actual_branch()

    def compile_and_run(name, fixture):
        cpp, exe = tmp_path / (name + ".cpp"), tmp_path / name
        cpp.write_text(fixture)
        built = subprocess.run(
            [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(exe)],
            capture_output=True, text=True, timeout=30)
        assert built.returncode == 0, built.stderr
        return subprocess.run([str(exe)], capture_output=True, text=True, timeout=10)

    assert compile_and_run("tech_val_signed_factor", _fixture(branch)).returncode == 0
    # The sole changed local type recreates unsigned DIV and must fail the
    # malformed-negative diagnostic while ordinary states remain unchanged.
    mutant = compile_and_run("tech_val_unsigned_factor",
                             _fixture(branch, unsigned_factor=True))
    assert mutant.returncode != 0
