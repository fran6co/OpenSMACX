"""Host-test the actual psi_factor definition; not native ABI/game proof."""
import re
import shutil
import subprocess
import tempfile
from pathlib import Path


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/veh.cpp").is_file())
SOURCE = (ROOT / "src/veh.cpp").read_text()
CONTROL = r'''#include <stdlib.h>
#define __cdecl
#define FALSE 0
#define TRUE 1
typedef int BOOL;
enum { SP_NEURAL_AMPLIFIER = 9, SP_DREAM_TWISTER = 19 };
struct Player { int rule_psi; };
Player Players[4];
static int neural_project;
static int dream_project;
static int calls[8];
static int factions[8];
static int call_count;
static int mutate_rule_on_project;
BOOL has_project(int project, int faction) {
    if (call_count >= 8) return 0;
    calls[call_count] = project;
    factions[call_count] = faction;
    ++call_count;
    if (mutate_rule_on_project) Players[faction].rule_psi = 900;
    if (project == SP_DREAM_TWISTER) return dream_project;
    if (project == SP_NEURAL_AMPLIFIER) return neural_project;
    return 0;
}
// @@ACTUAL_DEFINITION@@
static int reset(int rule_psi, int faction) {
    Players[faction].rule_psi = rule_psi;
    neural_project = dream_project = 0;
    call_count = mutate_rule_on_project = 0;
    return faction;
}
static int expect(int actual, int expected, int code) {
    return actual == expected ? 0 : code;
}
int main() {
    int faction = reset(0, 1);
    // Attack asks only for Dream Twister; fungal status is not an attack bonus.
    if (expect(psi_factor(5, faction, 1, 1), 5, 1)) return 1;
    if (call_count != 1 || calls[0] != SP_DREAM_TWISTER || factions[0] != faction) return 2;

    faction = reset(50, 2);
    dream_project = 1;
    // 150*9/100 truncates to 13, then attack bonus adds 13/2.
    if (expect(psi_factor(9, faction, 1, 0), 19, 3)) return 3;
    if (call_count != 1 || calls[0] != SP_DREAM_TWISTER) return 4;

    faction = reset(50, 2);
    // C++ signed division truncates -450/100 toward zero to -4; each half
    // likewise truncates toward zero: -4 -> -6 -> -9.
    neural_project = 1;
    if (expect(psi_factor(-3, faction, 0, 1), -9, 5)) return 5;
    if (call_count != 1 || calls[0] != SP_NEURAL_AMPLIFIER) return 6;

    faction = reset(-50, 3);
    // A negative rule_psi is preserved: 50*9/100 truncates toward zero to 4.
    if (expect(psi_factor(9, faction, 1, 0), 4, 13)) return 13;
    if (call_count != 1 || calls[0] != SP_DREAM_TWISTER) return 14;

    faction = reset(0, 3);
    // Defense project adds one half; no fungal bonus when false.
    neural_project = 1;
    if (expect(psi_factor(7, faction, 0, 0), 10, 7)) return 7;
    if (call_count != 1 || calls[0] != SP_NEURAL_AMPLIFIER) return 8;

    faction = reset(0, 3);
    // Negative odd half uses truncation toward zero: -5 + (-5/2) == -7.
    if (expect(psi_factor(-5, faction, 0, 1), -7, 9)) return 9;
    if (call_count != 1 || calls[0] != SP_NEURAL_AMPLIFIER) return 10;

    faction = reset(0, 1);
    mutate_rule_on_project = 1;
    // rule_psi is read before the project call; mutation inside that call must
    // not retroactively change this factor.
    if (expect(psi_factor(5, faction, 1, 0), 5, 11)) return 11;
    if (Players[faction].rule_psi != 900 || call_count != 1) return 12;
    return 0;
}
'''


def _closing_brace(text, opening):
    depth = 0
    state = "code"
    index = opening
    while index < len(text):
        ch = text[index]
        nxt = text[index + 1] if index + 1 < len(text) else ""
        if state == "line":
            if ch == "\n":
                state = "code"
        elif state == "block":
            if ch == "*" and nxt == "/":
                state = "code"
                index += 1
        elif state in ("string", "char"):
            if ch == "\\":
                index += 1
            elif (state == "string" and ch == '"') or (state == "char" and ch == "'"):
                state = "code"
        elif ch == "/" and nxt == "/":
            state = "line"
            index += 1
        elif ch == "/" and nxt == "*":
            state = "block"
            index += 1
        elif ch == '"':
            state = "string"
        elif ch == "'":
            state = "char"
        elif ch == "{":
            depth += 1
        elif ch == "}":
            depth -= 1
            if depth == 0:
                return index
        index += 1
    raise AssertionError("unterminated actual psi_factor definition")


def actual_definition(source=SOURCE):
    match = re.search(r"int\s+__cdecl\s+psi_factor\s*\(", source)
    assert match, "actual veh.cpp definition missing"
    opening = source.find("{", match.end())
    return source[match.start():_closing_brace(source, opening) + 1]


def build(directory, definition, compiler):
    source = CONTROL.replace("// @@ACTUAL_DEFINITION@@", definition, 1)
    assert "@@ACTUAL_DEFINITION@@" not in source
    cpp, exe = directory / "actual_psi.cpp", directory / "actual_psi"
    cpp.write_text(source)
    result = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", "-pedantic",
         str(cpp), "-o", str(exe)], capture_output=True, text=True, timeout=45
    )
    assert result.returncode == 0, result.stdout + result.stderr
    return exe


def run(exe, expected_success):
    result = subprocess.run([str(exe)], capture_output=True, text=True, timeout=15)
    if expected_success:
        assert result.returncode == 0, result.stdout + result.stderr
    else:
        assert result.returncode != 0, "psi_factor control failed to detect mutation"


def test_actual_psi_factor_and_compile_success_mutants():
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    compilers = [compiler for compiler in compilers if compiler]
    assert compilers, "GCC or Clang is required for these host controls"
    actual = actual_definition()
    mutations = (
        ("(rule_psi + 100) * combat_ratio", "(rule_psi + 90) * combat_ratio"),
        ("SP_DREAM_TWISTER, faction_id", "SP_NEURAL_AMPLIFIER, faction_id"),
        ("add_half = is_fungal_twr;", "add_half = !is_fungal_twr;"),
        ("if (add_half) {\n        factor += factor / 2;\n    }",
         "if (!add_half) {\n        factor += factor / 2;\n    }"),
    )
    for compiler in compilers:
        with tempfile.TemporaryDirectory(prefix="vehicle-psi-source-control-") as temp:
            directory = Path(temp)
            run(build(directory, actual, compiler), True)
            for before, after in mutations:
                assert actual.count(before) == 1, before
                damaged = actual.replace(before, after, 1)
                run(build(directory, damaged, compiler), False)
