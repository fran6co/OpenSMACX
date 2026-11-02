"""Actual TechAI query phases and pinned-x86 divide fault, using service doubles.

Mutating collaborators expose stale query damage; these controls do not claim
that real tech_val/tech_recurse currently mutate the human mask. Host SIGFPE is
an x86 machine control, not a portable C++ zero-division result or game proof.
"""
from pathlib import Path
import re
import resource
import shutil
import signal
import subprocess

import pytest
from match_recovery.source import function_line

ROOT = Path(__file__).resolve().parents[2]


def definition(text, name):
    line = function_line(text, name)
    start = sum(len(x) for x in text.splitlines(True)[:line - 1])
    opened = text.index("{", start)
    depth = 0
    for end in range(opened, len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            return text[start:end + 1]
    raise AssertionError(name)


def actual_program(damage):
    body = definition((ROOT / "src/technology.cpp").read_text(), "tech_ai")
    primitive = definition((ROOT / "src/crt_random.h").read_text(), "bounded_crt_random")
    human = definition((ROOT / "src/faction.h").read_text(), "is_human")
    maximum, = re.findall(r"static const int MaxTechnologyNum = \d+;",
                           (ROOT / "src/technology.h").read_text())
    formers, = re.findall(r"BSC_FORMERS\s*=\s*\d+", (ROOT / "src/veh.h").read_text())
    blind, = re.findall(r"RULES_BLIND_RESEARCH\s*=\s*0x[0-9a-fA-F]+",
                        (ROOT / "src/game.h").read_text())
    if damage in ("cached_entry", "cached_after_value"):
        assert body.count("is_human(faction_id)") == 2
        body = body.replace("is_human(faction_id)", "cached_human")
        if damage == "cached_entry":
            anchor = "    int tech_id = -1;"
        else:
            anchor = "            int tech_value = tech_val(i, faction_id, false);"
        assert body.count(anchor) == 1
        body = body.replace(anchor, anchor + "\n    BOOL cached_human = is_human(faction_id);", 1)
    elif damage == "zero_fallback":
        old = "tech_value = (tech_value << 8) / preq;"
        assert body.count(old) == 1
        body = body.replace(old, "tech_value = preq ? (tech_value << 8) / preq : 0;", 1)
    else:
        assert damage is None
    prelude = r'''
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <string>
#define __cdecl
using uint32_t = unsigned int;
using BOOL = int;
MAXIMUM
enum { FORMERS, BLIND };
struct LockBytes { unsigned char human_mask_; };
struct LockMasks { LockBytes bytes_; };
struct StateFields { unsigned int game_rules_; LockMasks lock_masks_; };
struct State { StateFields fields_; } GameDataStateStorage;
struct Player { int ai_growth, ai_wealth; } PlayersData[8];
struct Prototype { int preq_tech; } VehPrototypes[16];
std::string events;
int after_value, after_recurse, divisor = 2;
int tech_avail(int technology, int) { return technology == 0; }
int tech_val(int, int, bool) {
    events += 'V'; GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ = after_value;
    return 10;
}
int tech_recurse(int, int) {
    events += 'R'; GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ = after_recurse;
    return divisor;
}
uint32_t random(int lower, int upper) {
    assert(lower == 0 && upper > 0); events += 'H'; return 5;
}
int fixture_rand() { events += 'C'; return 7; }
#define rand fixture_rand
PRIMITIVE
#undef rand
HUMAN
ACTUAL
void reset(int before, int value, int recurse) {
    events.clear(); divisor = 2;
    GameDataStateStorage.fields_.game_rules_ = RULES_BLIND_RESEARCH;
    GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ = before;
    after_value = value; after_recurse = recurse;
    PlayersData[1].ai_growth = PlayersData[1].ai_wealth = 0;
    VehPrototypes[BSC_FORMERS].preq_tech = 0;
}
int main(int argc, char **) {
    if (argc > 1) {
        reset(0, 0, 0); divisor = 0;
        tech_ai(1);
        return 0; // Only reached by the unoriginal guarded-divide mutant.
    }
    reset(0, 2, 0); PlayersData[1].ai_growth = 1;
    assert(tech_ai(1) == 0 && events == "V");
    reset(0, 0, 2); assert(tech_ai(1) == 0 && events == "VRH");
    reset(2, 2, 0); assert(tech_ai(1) == 0 && events == "VRC");
    reset(2, 0, 0); assert(tech_ai(1) == 0 && events == "VRC");
    return 0;
}
'''
    return (prelude.replace("MAXIMUM", maximum).replace("FORMERS", formers, 1)
            .replace("BLIND", blind, 1).replace("PRIMITIVE", primitive)
            .replace("HUMAN", human).replace("ACTUAL", body))


def no_core_dump():
    resource.setrlimit(resource.RLIMIT_CORE, (0, 0))


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
@pytest.mark.parametrize("damage", [None, "cached_entry", "cached_after_value", "zero_fallback"])
def test_actual_tech_ai_live_queries_and_x86_divide_fault(tmp_path, compiler_name, damage):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(compiler_name + " unavailable")
    source = tmp_path / "tech_ai.cpp"
    executable = tmp_path / "tech_ai"
    source.write_text(actual_program(damage))
    built = subprocess.run([compiler, "-std=c++17", "-O0", "-fwrapv", "-Wall", "-Wextra",
                            "-Werror", str(source), "-o", str(executable)],
                           capture_output=True, text=True)
    assert built.returncode == 0, built.stderr
    normal = subprocess.run([str(executable)], cwd=tmp_path, capture_output=True,
                            preexec_fn=no_core_dump)
    fault = subprocess.run([str(executable), "zero"], cwd=tmp_path, capture_output=True,
                           preexec_fn=no_core_dump)
    correct = normal.returncode == 0 and fault.returncode == -signal.SIGFPE
    assert correct == (damage is None), (compiler_name, damage, normal.returncode, fault.returncode)
