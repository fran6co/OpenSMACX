"""Source-extracted host controls for Console's now-owned call targets.

Install under recovery/tests after applying owner-migration.patch. This reads
only the checkout's real sources and intentionally has no ignored-source overlay.
"""

from __future__ import annotations

import shutil
import subprocess
from pathlib import Path

import pytest


def repo_root() -> Path:
    for parent in Path(__file__).resolve().parents:
        if (parent / "src" / "console.cpp").is_file():
            return parent
    raise AssertionError("could not locate checkout containing src/console.cpp")


def function_body(source: str, signature: str) -> str:
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
    raise AssertionError(f"unterminated function: {signature}")


def block_after(body: str, marker: str) -> str:
    start = body.index(marker)
    opening = body.index("{", start)
    depth = 0
    for end in range(opening, len(body)):
        if body[end] == "{":
            depth += 1
        elif body[end] == "}":
            depth -= 1
            if depth == 0:
                return body[start : end + 1]
    raise AssertionError(f"unterminated block: {marker}")


def exact_statement(body: str, expected: str) -> str:
    lines = [line.strip() for line in body.splitlines() if line.strip() == expected]
    assert len(lines) == 1, (expected, lines)
    return lines[0]


def harness(source: str, mutant: str | None = None) -> str:
    energy = function_body(source, "void Console::editor_energy()")
    load = function_body(source, "void Console::load_proc(int save_slot, int faction_id)")
    destroy = function_body(source, "void Console::destroy(int veh_id)")

    energy_block = block_after(energy, "if (answer == 0)")
    exit_call = exact_statement(load, "TutWinBaseWindow.exit();")
    destroy_call = exact_statement(destroy, "action_destroy(veh_id, result, -1, -1);")
    if mutant == "destroy_arguments":
        destroy_call = destroy_call.replace("result, -1, -1", "result, -1, 1")
    elif mutant == "energy_order":
        energy_block = energy_block.replace(
            "TutWinBaseWindow.on_redraw();\n            ConsoleGlobal.update_data(0);",
            "ConsoleGlobal.update_data(0);\n            TutWinBaseWindow.on_redraw();",
        )
    elif mutant == "load_order":
        exit_call = exit_call.replace("TutWinBaseWindow.exit()", "wrong_exit()")

    return r"""
#include <string>
#include <vector>
std::vector<std::string> events;
struct BaseWin { void on_redraw() { events.push_back("redraw"); }
                 void exit() { events.push_back("exit"); } } TutWinBaseWindow;
struct PlayerData { int energy_reserves; } PlayersData[8];
int ParseNumTable[1] = {93};
struct ConsoleState { void update_data(int mode) { events.push_back("update:" + std::to_string(mode)); } } ConsoleGlobal;
int faction = 2;
int captured[4] = {-99, -99, -99, -99};
void action_destroy(int veh, int bits, int x, int y) {
    captured[0] = veh; captured[1] = bits; captured[2] = x; captured[3] = y;
    events.push_back("destroy");
}
void wrong_exit() { events.push_back("wrong-exit"); }

void run_energy(int answer) { @@ENERGY@@ }
void run_load_proc_prefix() { @@EXIT_CALL@@ }
void run_destroy_branch(int veh_id, int result) { @@DESTROY_CALL@@ }

int main() {
    PlayersData[faction].energy_reserves = 14;
    run_energy(0);
    if (PlayersData[faction].energy_reserves != 93 || events.size() != 2
        || events[0] != "redraw" || events[1] != "update:0") return 1;
    events.clear();
    run_energy(1);
    if (PlayersData[faction].energy_reserves != 93 || !events.empty()) return 2;

    run_load_proc_prefix();
    if (events.size() != 1 || events[0] != "exit") return 3;
    events.clear();
    run_destroy_branch(23, 6);
    if (captured[0] != 23 || captured[1] != 6 || captured[2] != -1 || captured[3] != -1
        || events.size() != 1 || events[0] != "destroy") return 4;
    return 0;
}
""".replace("@@ENERGY@@", energy_block).replace("@@EXIT_CALL@@", exit_call).replace(
        "@@DESTROY_CALL@@", destroy_call
    )


def test_console_callees_are_typed_source_owned_and_no_raw_seams_remain():
    root = repo_root()
    console = (root / "src" / "console.cpp").read_text()
    veh_h = (root / "src" / "veh.h").read_text()
    veh_cpp = (root / "src" / "veh.cpp").read_text()
    basewin_h = (root / "src" / "basewin.h").read_text()
    basewin_cpp = (root / "src" / "basewin.cpp").read_text()
    tutwin_h = (root / "src" / "tutwin.h").read_text()

    assert "void __cdecl action_destroy(int veh_id, int bits, int x, int y);" in veh_h
    assert "void __cdecl action_destroy(int veh_id, int bits, int x, int y) {" in veh_cpp
    assert "void on_redraw();" in basewin_h and "void BaseWin::on_redraw() {" in basewin_cpp
    assert "void exit();" in basewin_h and "void BaseWin::exit() {" in basewin_cpp
    assert "extern BaseWin TutWinBaseWindow;" in tutwin_h
    assert "RECOVERY_OBJECT: v1 0x006A7628 ?TutWinBaseWindow@@3VBaseWin@@A" in basewin_cpp

    assert "action_destroy(veh_id, result, -1, -1);" in console
    assert "TutWinBaseWindow.on_redraw();" in console
    assert "TutWinBaseWindow.exit();" in console
    for obsolete in (
        "ActionDestroy",
        "func_action_destroy",
        "BaseWinOnRedraw",
        "BaseWinExit",
        "func_base_win_on_redraw",
        "func_base_win_exit",
        "original_method<func_base_win_",
    ):
        assert obsolete not in console

    load = function_body(console, "void Console::load_proc(int save_slot, int faction_id)")
    assert load.index("TutWinBaseWindow.exit();") < load.index("if (IsMultiplayerNet")
    assert load.index("TutWinBaseWindow.exit();") < load.index("load_game(save_slot, faction_id)")
    energy = function_body(console, "void Console::editor_energy()")
    assert energy.index("TutWinBaseWindow.on_redraw();") < energy.index("ConsoleGlobal.update_data(0);")
    destroy = function_body(console, "void Console::destroy(int veh_id)")
    clear = destroy.index("field_23BE4_ = 0;")
    dispatch_guard = destroy.index("if (IsMultiplayerNet != 0)", clear)
    assert clear < dispatch_guard
    assert dispatch_guard < destroy.index(
        "action_destroy(veh_id, result, -1, -1);"
    )


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_source_extracted_call_order_and_damage_controls(tmp_path: Path, compiler_name: str):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is not installed")
    source = (repo_root() / "src" / "console.cpp").read_text()

    def compile_and_run(label: str, mutant: str | None, should_pass: bool):
        cpp = tmp_path / f"console_{compiler_name}_{label}.cpp"
        exe = tmp_path / f"console_{compiler_name}_{label}"
        cpp.write_text(harness(source, mutant))
        result = subprocess.run(
            [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(exe)],
            text=True,
            capture_output=True,
        )
        assert result.returncode == 0, result.stdout + result.stderr
        ran = subprocess.run([str(exe)], text=True, capture_output=True)
        assert (ran.returncode == 0) is should_pass, ran.stdout + ran.stderr

    compile_and_run("actual", None, True)
    compile_and_run("wrong_destroy_arguments", "destroy_arguments", False)
    compile_and_run("reordered_energy_callbacks", "energy_order", False)
    compile_and_run("wrong_exit_target", "load_order", False)
