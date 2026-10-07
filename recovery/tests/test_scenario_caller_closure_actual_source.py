"""Host controls for the Scenario owner closure, extracted from checkout source.

Refresh and opening-movie bodies are extracted in full; menu and movement
controls extract only their changed call statements, not surrounding policy.
Minimal host doubles do not establish VC6 ABI or game runtime acceptance.
"""

from __future__ import annotations

import shutil
import subprocess
from pathlib import Path

import pytest


def repo_root() -> Path:
    for parent in Path(__file__).resolve().parents:
        if (parent / "src" / "scenario.cpp").is_file():
            return parent
    raise AssertionError("could not locate checkout containing src/scenario.cpp")


def body_after(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        if source[end] == "{":
            depth += 1
        elif source[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start : end + 1]
    raise AssertionError(f"unterminated function body: {signature}")


def statement_in(body: str, statement: str) -> str:
    matches = [line.strip() for line in body.splitlines() if line.strip() == statement]
    assert len(matches) == 1, (statement, matches)
    return matches[0]


def translation_unit(source: str, mutant: str | None = None) -> str:
    refresh = body_after(source, "void refresh_loaded_game()")
    opening = body_after(source, "void __cdecl scenario_opening_movie(char *movie_name)")
    execute = body_after(source, "void execute_commands(Console *self)")
    top_menu = body_after(source, "int __cdecl scenario_top_menu(int mode)")

    menu_call = statement_in(top_menu, "return top_menu(mode);")
    action_call = statement_in(execute, "action_go_to(State.vehicle_id);")
    if mutant == "menu_argument":
        menu_call = menu_call.replace("mode)", "mode + 1)")
    elif mutant == "action_argument":
        action_call = action_call.replace("State.vehicle_id", "State.target_x")
    elif mutant == "movie_gate":
        opening = opening.replace("\n        || GetEnvironmentVariableA", "\n        && GetEnvironmentVariableA")

    return r"""
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#define __cdecl
using DWORD = unsigned int;
using LPCSTR = const char *;
std::vector<std::string> events;
unsigned save_length = 1;
unsigned result_length = 0;

struct GameFields { unsigned game_state_; };
struct GameStorage { GameFields fields_; };
GameStorage GameDataStateStorage = {{0xFFFFFFFFu}};
struct Button { void hide() { events.push_back("hide"); } };
struct Interface { Button flat_button_[3]; };
Interface MainInterfaceGlobal;
void load_faction_art(int faction) { events.push_back("f" + std::to_string(faction)); }
void load_flags() { events.push_back("flags"); }
void prefs_use() { events.push_back("prefs"); }
DWORD GetEnvironmentVariableA(LPCSTR name, char *, DWORD) {
    events.push_back(name);
    return std::strcmp(name, "OPENSMACX_SCENARIO_SAVE") == 0 ? save_length : result_length;
}
void amovie_project(char *movie) { events.push_back(std::string("movie:") + movie); }
int top_menu(int mode) { events.push_back("menu:" + std::to_string(mode)); return 700 + mode; }
struct ScenarioState { int vehicle_id; int target_x; } State = {42, 77};
void action_go_to(int id) { events.push_back("action:" + std::to_string(id)); }

@@REFRESH@@
@@OPENING@@

int invoke_menu(int mode) { @@MENU_CALL@@ }
void invoke_action() { @@ACTION_CALL@@ }

int main() {
    refresh_loaded_game();
    const char *expected_refresh[] = {"f1", "f2", "f3", "f4", "f5", "f6", "f7", "flags", "hide", "prefs"};
    if (events.size() != 10) return 1;
    for (unsigned i = 0; i < events.size(); ++i) if (events[i] != expected_refresh[i]) return 2;
    if (GameDataStateStorage.fields_.game_state_ != 0xF79FCF16u) return 3;

    events.clear();
    save_length = 1; result_length = 0;
    char movie[] = "opening";
    scenario_opening_movie(movie);
    if (events.size() != 3 || events[0] != "OPENSMACX_SCENARIO_SAVE"
        || events[1] != "OPENSMACX_SCENARIO_RESULT" || events[2] != "movie:opening") return 4;

    events.clear();
    save_length = 1; result_length = 1;
    scenario_opening_movie(movie);
    if (events.size() != 2 || events[0] != "OPENSMACX_SCENARIO_SAVE"
        || events[1] != "OPENSMACX_SCENARIO_RESULT") return 5;

    events.clear();
    if (invoke_menu(13) != 713 || events.size() != 1 || events[0] != "menu:13") return 6;
    events.clear();
    invoke_action();
    if (events.size() != 1 || events[0] != "action:42") return 7;
    return 0;
}
""".replace("@@REFRESH@@", refresh).replace("@@OPENING@@", opening).replace(
        "@@MENU_CALL@@", menu_call
    ).replace("@@ACTION_CALL@@", action_call)


def test_scenario_direct_call_closure_and_retired_trampolines():
    root = repo_root()
    source = (root / "src" / "scenario.cpp").read_text()
    header = (root / "src" / "scenario.h").read_text()

    for spelling in (
        "top_menu(mode)",
        "load_flags();",
        "amovie_project(movie_name)",
        "action_go_to(State.vehicle_id)",
    ):
        assert spelling in source
    for retired in (
        "TopMenuAddress",
        "OpeningMovieAddress",
        "LoadFlagsAddress",
        "ActionGoToAddress",
        "OriginalTopMenu",
        "OriginalOpeningMovie",
        "LoadFlagsFunction",
        "ActionGoToFunction",
        "scenario_human_turn_trampoline",
        "scenario_turn_advance_trampoline",
        "ScenarioTrampolineAction",
        "ScenarioTurnAdvanceAction",
    ):
        assert retired not in source
        assert retired not in header
    # Keep the source-only fixture logic; only its retired machine-code seams go.
    assert "scenario_human_turn_ready(&ConsoleGlobal);" in source
    assert "scenario_turn_advanced(uintptr_t caller_return)" in source


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_extracted_scenario_calls_and_mutation_controls(tmp_path: Path, compiler_name: str):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is not installed")
    source = (repo_root() / "src" / "scenario.cpp").read_text()

    def compile_and_run(label: str, mutant: str | None, expected_success: bool):
        cpp = tmp_path / f"scenario_{compiler_name}_{label}.cpp"
        exe = tmp_path / f"scenario_{compiler_name}_{label}"
        cpp.write_text(translation_unit(source, mutant))
        built = subprocess.run(
            [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(exe)],
            text=True,
            capture_output=True,
        )
        assert built.returncode == 0, built.stdout + built.stderr
        result = subprocess.run([str(exe)], text=True, capture_output=True)
        assert (result.returncode == 0) is expected_success, result.stdout + result.stderr

    compile_and_run("actual", None, True)
    compile_and_run("wrong_menu_argument", "menu_argument", False)
    compile_and_run("wrong_action_argument", "action_argument", False)
    compile_and_run("wrong_movie_gate", "movie_gate", False)

