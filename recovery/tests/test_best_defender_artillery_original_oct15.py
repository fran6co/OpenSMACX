"""Bounded host controls for best_defender's original predicates.

The predicate expressions below are extracted from src/veh.cpp. These tests
cover their order and short-circuit behavior, not native layout or invalid
vehicle IDs. A negative-ID case uses an interior pointer into a real host array
only to make the original predecessor-row read defined in this fixture.
"""

from pathlib import Path
import shutil
import subprocess
import tempfile

import pytest


ROOT = Path(__file__).resolve().parents[2]
OWNER = ROOT / "src" / "veh.cpp"


def _balanced_expression(source: str, start: int) -> tuple[str, int]:
    opening = source.find("(", start)
    assert opening >= 0
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "(":
            depth += 1
        elif source[index] == ")":
            depth -= 1
            if depth == 0:
                return source[opening + 1:index], index + 1
    raise AssertionError("unterminated production predicate")


def _best_defender_body(source: str) -> str:
    signature = source.index("int __cdecl best_defender(")
    opening = source.index("{", signature)
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[opening + 1:index]
    raise AssertionError("unterminated best_defender definition")


def _actual_predicates():
    source = _best_defender_body(OWNER.read_text())
    candidate_start = source.index("if ((get_proto_triad(proto_id_def)")
    candidate, candidate_end = _balanced_expression(source, candidate_start)
    assert "VehPrototypes[Vehs[veh_id_atk].proto_id].plan != PLAN_INFO_WARFARE" in candidate
    assert source[candidate_end:source.find("{", candidate_end)].strip() == ""

    artillery_start = source.index("uint32_t combat_type = (", candidate_start)
    artillery, artillery_end = _balanced_expression(source, artillery_start)
    assert source[artillery_end:artillery_end + 12].lstrip().startswith("? 2 : 0;")
    assert "can_arty(proto_id_def, true)" in artillery
    assert "can_arty(Vehs[veh_id_atk].proto_id, true)" in artillery
    return candidate, artillery


def _compilers():
    found = [shutil.which(name) for name in ("g++", "clang++")]
    found = [compiler for compiler in found if compiler]
    if not found:
        pytest.skip("neither GCC nor Clang is installed")
    return found


def _compile_run(compiler: str, fixture: str, expected_run=0):
    with tempfile.TemporaryDirectory(prefix="osmx-best-defender-original-") as temp:
        cpp = Path(temp) / "actual_predicate.cpp"
        exe = Path(temp) / "actual_predicate"
        cpp.write_text(fixture)
        built = subprocess.run(
            [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", "-Wno-parentheses",
             str(cpp), "-o", str(exe)],
            text=True, capture_output=True, check=False, timeout=30,
        )
        assert built.returncode == 0, compiler + " compile failed:\n" + built.stdout + built.stderr
        ran = subprocess.run([str(exe)], check=False, timeout=5)
        if expected_run is None:
            assert ran.returncode != 0, compiler + " mutant unexpectedly passed"
        else:
            assert ran.returncode == expected_run, compiler + " runtime assertion failed"


def _candidate_fixture(expression: str) -> str:
    return r'''#include <assert.h>
struct Vehicle { int proto_id; };
struct Prototype { int plan; int triad; };
static const int TRIAD_LAND = 0;
static const int PLAN_INFO_WARFARE = 7;
static Vehicle backing[3];
static Vehicle *Vehs = &backing[1];
static Prototype VehPrototypes[16];
static int get_proto_triad(int proto_id) { return VehPrototypes[proto_id].triad; }
static bool candidate_admitted(int veh_id_atk, int proto_id_def,
                               bool is_ocean_def, int base_id_def) {
    return ''' + expression + r''';
}
int main() {
    VehPrototypes[3].triad = 1;
    VehPrototypes[3].plan = 0;
    /* Native reads the attacker's plan even when its ID is negative. */
    backing[0].proto_id = 4;
    VehPrototypes[4].plan = PLAN_INFO_WARFARE;
    assert(!candidate_admitted(-1, 3, false, -1));
    /* A non-war attacker admits a non-war candidate. */
    backing[2].proto_id = 5;
    VehPrototypes[5].plan = 0;
    assert(candidate_admitted(1, 3, false, -1));
    /* The candidate's own warfare plan admits it against a warfare attacker. */
    VehPrototypes[3].plan = PLAN_INFO_WARFARE;
    assert(candidate_admitted(-1, 3, false, -1));
    return 0;
}
'''


def test_candidate_plan_predicate_reads_the_original_attacker_row():
    candidate, _ = _actual_predicates()
    for compiler in _compilers():
        _compile_run(compiler, _candidate_fixture(candidate))


def test_negative_id_candidate_plan_bypass_mutant_is_detected():
    candidate, _ = _actual_predicates()
    needle = "VehPrototypes[Vehs[veh_id_atk].proto_id].plan != PLAN_INFO_WARFARE"
    assert candidate.count(needle) == 1
    mutant = candidate.replace(needle, "veh_id_atk < 0 || " + needle, 1)
    for compiler in _compilers():
        _compile_run(compiler, _candidate_fixture(mutant), expected_run=None)


def _artillery_fixture(expression: str) -> str:
    return r'''#include <assert.h>
struct Vehicle { int proto_id; };
static Vehicle backing[3];
static Vehicle *Vehs = &backing[1];
static bool artillery[16];
static int calls[4];
static int call_count;
static bool can_arty(int proto_id, bool use_artillery) {
    assert(use_artillery);
    calls[call_count++] = proto_id;
    return artillery[proto_id];
}
static unsigned int artillery_type(int proto_id_def, int veh_id_atk) {
    return (''' + expression + r''') ? 2 : 0;
}
int main() {
    backing[2].proto_id = 4;
    backing[0].proto_id = 5;
    artillery[3] = true;
    artillery[4] = false;
    call_count = 0;
    assert(artillery_type(3, 1) == 0);
    assert(call_count == 2 && calls[0] == 3 && calls[1] == 4);

    artillery[3] = false;
    call_count = 0;
    assert(artillery_type(3, 1) == 0);
    assert(call_count == 1 && calls[0] == 3);

    artillery[3] = true;
    artillery[4] = true;
    call_count = 0;
    assert(artillery_type(3, 1) == 2);
    assert(call_count == 2 && calls[0] == 3 && calls[1] == 4);

    /* Negative ID uses the bounded predecessor row, not an out-of-array read. */
    artillery[5] = true;
    call_count = 0;
    assert(artillery_type(3, -1) == 2);
    assert(call_count == 2 && calls[0] == 3 && calls[1] == 5);
    return 0;
}
'''


def test_artillery_admission_requires_both_capabilities_in_order():
    _, artillery = _actual_predicates()
    for compiler in _compilers():
        _compile_run(compiler, _artillery_fixture(artillery))


def test_artillery_nonzero_id_shortcut_mutant_is_detected():
    _, artillery = _actual_predicates()
    needle = "can_arty(proto_id_def, true)\n                && can_arty(Vehs[veh_id_atk].proto_id, true)"
    assert artillery.count(needle) == 1
    mutant = artillery.replace(
        needle,
        "can_arty(proto_id_def, true)\n                && (veh_id_atk || can_arty(Vehs[veh_id_atk].proto_id, true))",
        1,
    )
    for compiler in _compilers():
        _compile_run(compiler, _artillery_fixture(mutant), expected_run=None)
