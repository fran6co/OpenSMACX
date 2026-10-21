"""Host controls for the extracted not_my_turn predicate body.

This checks the actual source body under GCC/Clang, not VC6 code generation,
the export ABI, or the historical translation unit.
"""
from pathlib import Path
import re
import shutil
import subprocess
import tempfile

import pytest


ROOT = Path(__file__).resolve().parents[2]


def _actual_body() -> str:
    source = (ROOT / "src/game.cpp").read_text()
    header = (ROOT / "src/game.h").read_text()
    assert "MEASURED BOOL __cdecl not_my_turn();" in header
    assert "MEASURED inline BOOL __cdecl not_my_turn()" not in header
    match = re.search(r"(?m)^BOOL __cdecl not_my_turn\(\)\s*\{", source)
    assert match, "missing ordinary not_my_turn definition in game.cpp"
    opening = source.index("{", match.start())
    depth = 0
    state = "code"
    index = opening
    while index < len(source):
        char = source[index]
        next_char = source[index + 1] if index + 1 < len(source) else ""
        if state == "code":
            if char == "/" and next_char == "/":
                state = "line"
                index += 1
            elif char == "/" and next_char == "*":
                state = "block"
                index += 1
            elif char == '"':
                state = "string"
            elif char == "'":
                state = "char"
            elif char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return source[match.start():index + 1]
        elif state == "line" and char == "\n":
            state = "code"
        elif state == "block" and char == "*" and next_char == "/":
            state = "code"
            index += 1
        elif state in ("string", "char"):
            if char == "\\":
                index += 1
            elif (state == "string" and char == '"') or (
                    state == "char" and char == "'"):
                state = "code"
        index += 1
    raise AssertionError("unterminated not_my_turn body")


HARNESS = r"""
#include <stdio.h>
#include <stdlib.h>

#define __cdecl
typedef int BOOL;
struct Console { int faction_id_; } ConsoleGlobal;
struct GameStateFields { unsigned int net_turn_flags_; int net_turn_faction_; };
struct GameState { GameStateFields fields_; } GameDataStateStorage;
int IsMultiplayerNet;

void require_at(int condition, int line) {
    if (!condition) {
        fprintf(stderr, "assertion failed at line %d\n", line);
        exit(93);
    }
}
#define require(condition) require_at((condition), __LINE__)

BODY

int main() {
    IsMultiplayerNet = 0;
    GameDataStateStorage.fields_.net_turn_flags_ = 0x10;
    GameDataStateStorage.fields_.net_turn_faction_ = 2;
    ConsoleGlobal.faction_id_ = 7;
    require(not_my_turn() == 0);

    IsMultiplayerNet = 1;
    GameDataStateStorage.fields_.net_turn_flags_ = 0;
    require(not_my_turn() == 0);
    GameDataStateStorage.fields_.net_turn_flags_ = 1;
    require(not_my_turn() == 0);

    GameDataStateStorage.fields_.net_turn_flags_ = 0x11;
    GameDataStateStorage.fields_.net_turn_faction_ = 7;
    require(not_my_turn() == 0);
    GameDataStateStorage.fields_.net_turn_flags_ = 0x10;
    GameDataStateStorage.fields_.net_turn_faction_ = 2;
    require(not_my_turn() == 1);
    GameDataStateStorage.fields_.net_turn_faction_ = 9;
    require(not_my_turn() == 1);
    require(IsMultiplayerNet == 1);
    require(GameDataStateStorage.fields_.net_turn_flags_ == 0x10);
    require(GameDataStateStorage.fields_.net_turn_faction_ == 9);
    require(ConsoleGlobal.faction_id_ == 7);
    return 0;
}
"""


def _compile_and_run(compiler: str, body: str, directory: Path, name: str) -> int:
    source = directory / (name + ".cpp")
    executable = directory / name
    source.write_text(HARNESS.replace("BODY", body))
    built = subprocess.run(
        [compiler, "-std=c++98", "-O2", "-Wall", "-Wextra", "-Werror",
         str(source), "-o", str(executable)],
        text=True, capture_output=True, check=False)
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(executable)], text=True, capture_output=True,
                         check=False)
    return ran.returncode


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_turn_predicate_body_and_compile_success_mutants(compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " is unavailable")
    body = _actual_body()
    with tempfile.TemporaryDirectory(prefix="turn-predicate-oct21v-") as name:
        directory = Path(name)
        assert _compile_and_run(compiler, body, directory, "actual") == 0

        wrong_flag = body.replace("& 0x10", "& 0x1")
        assert wrong_flag != body
        assert _compile_and_run(compiler, wrong_flag, directory,
                                "wrong_flag") == 93

        wrong_relation = body.replace(
            "net_turn_faction_ != ConsoleGlobal.faction_id_",
            "net_turn_faction_ == ConsoleGlobal.faction_id_")
        assert wrong_relation != body
        assert _compile_and_run(compiler, wrong_relation, directory,
                                "wrong_relation") == 93
