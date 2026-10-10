"""Compile bounded actual-source slices with named host test doubles.

The fixtures exercise source member/call semantics; they are not the actual
owner declarations, layout, Windows ABI, or runtime.
"""
from __future__ import annotations

import shutil
import subprocess
from pathlib import Path

import pytest


ROOT = Path(__file__).resolve().parents[2]


def extract_definition(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
    raise AssertionError(f"unterminated definition: {signature}")


def extract_map_flag_block(source: str) -> str:
    start = source.index("            MapWin *const primary_window = MapWinTable[0];")
    end = source.index("        }\n        ConsoleGlobal.focus_vehicle_", start)
    return source[start:end]


def control_harness(block: str) -> str:
    return r'''#include <cstdint>
#include <cstdio>
struct MapWin { uint32_t field_1DD70_; uint32_t decoy_; };
typedef MapWin *MapWinSlots[8];
MapWinSlots MapWinTable = {};
uint32_t WorldClimateSkipTerrainClear = 1;
int main() {
    MapWin primary = {0x81234021u, 0};
    MapWin decoy = {0x13572468u, 0};
    MapWinTable[0] = &primary;
    MapWinTable[1] = &decoy;
    {
''' + block + r'''
    }
    if (WorldClimateSkipTerrainClear != 0 ||
        primary.field_1DD70_ != 0x812348E1u ||
        primary.decoy_ != 0 || decoy.field_1DD70_ != 0x13572468u) {
        std::fprintf(stderr, "FAIL: live slot-zero dword was not ORed exactly\n");
        return 1;
    }
    WorldClimateSkipTerrainClear = 1;
    {
''' + block + r'''
    }
    if (primary.field_1DD70_ != 0x812348E1u ||
        WorldClimateSkipTerrainClear != 0) {
        std::fprintf(stderr, "FAIL: repeated OR changed stored flag state\n");
        return 2;
    }
    return 0;
}
'''


def lifecycle_harness(desktop_close: str, game_reload: str) -> str:
    return r'''#include <cstdint>
#include <cstdio>
#include <cstring>
#define __cdecl
static char events[128];
static unsigned event_count;
static void note(char value) { events[event_count++] = value; events[event_count] = 0; }
static void reset() { event_count = 0; events[0] = 0; }
struct Window {
    char id;
    explicit Window(char value = '?') : id(value) {}
    void close() { note(id); }
};
struct GraphicWin { void close() { note('G'); } };
struct MultiWin : GraphicWin {
    void close() { note('M'); }
} MultiWindow;
struct ClimateWindow : GraphicWin {} WorldClimateWorldWin;
struct Win { void hide() { note('H'); } };
struct MainInterface : Win {} MainInterfaceGlobal;
Window TutWinDesWindow('A'), TutWinBaseWindow('B'), ConsoleStatusWin('C');
void mapwin_system_shutdown() { note('S'); }
void stop_timers_call() { note('T'); }
void labels_shutdown_call() { note('L'); }
struct Strings { void shutdown() { note('X'); } } StringTable;
static int init_result;
int game_init(int, int) { note('I'); return init_result; }
void desktop_init(int) { note('U'); }
uint32_t WorldClimateSkipTerrainClear = 1;
''' + desktop_close + "\n" + game_reload + r'''
int main() {
    desktop_close();
    if (std::strcmp(events, "ABSCGGH") != 0) {
        std::fprintf(stderr, "FAIL: desktop close order: %s\n", events);
        return 1;
    }

    reset(); init_result = 0; WorldClimateSkipTerrainClear = 1;
    if (game_reload(0, 7) != 0 || std::strcmp(events, "TLXI") != 0 ||
        WorldClimateSkipTerrainClear != 0) {
        std::fprintf(stderr, "FAIL: mode-zero reload order/result: %s\n", events);
        return 2;
    }

    reset(); init_result = 0; WorldClimateSkipTerrainClear = 1;
    if (game_reload(1, 9) != 0 || std::strcmp(events, "ABSCGGHTLXIU") != 0 ||
        WorldClimateSkipTerrainClear != 0) {
        std::fprintf(stderr, "FAIL: mode-one reload order/result: %s\n", events);
        return 3;
    }

    reset(); init_result = 1; WorldClimateSkipTerrainClear = 0x1234;
    if (game_reload(1, 3) != 1 || std::strcmp(events, "ABSCGGHTLXI") != 0 ||
        WorldClimateSkipTerrainClear != 0x1234) {
        std::fprintf(stderr, "FAIL: failed reload path: %s\n", events);
        return 4;
    }
    return 0;
}
'''


def compile_and_run(compiler: str, source: str, tmp_path: Path, name: str):
    unit = tmp_path / f"{name}.cpp"
    executable = tmp_path / name
    unit.write_text(source)
    built = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", "-pedantic",
         str(unit), "-o", str(executable)],
        capture_output=True, text=True, timeout=30)
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(executable)], capture_output=True, text=True,
                          timeout=10)


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_control_game_live_mapwin_field_block(compiler_name: str,
                                                     tmp_path: Path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is not installed")
    source = (ROOT / "src/game.cpp").read_text()
    block = extract_map_flag_block(source)
    table_load = block.index("MapWin *const primary_window = MapWinTable[0];")
    assert "volatile MapWin *const live_window = primary_window;" in block
    climate_store = block.index("WorldClimateSkipTerrainClear = 0;")
    field_read = block.index("uint32_t flags = live_window->field_1DD70_;")
    flag_or = block.index("flags |= 0x8C0;")
    field_write = block.index("live_window->field_1DD70_ = flags;")
    assert table_load < climate_store < field_read < flag_or < field_write
    assert block.count("live_window->field_1DD70_") == 2
    assert "reinterpret_cast" not in block
    ran = compile_and_run(compiler, control_harness(block), tmp_path,
                          f"control_mapwin_{compiler_name}")
    assert ran.returncode == 0, ran.stdout + ran.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_compiling_wrong_mapwin_member_mutant_is_caught(compiler_name: str,
                                                         tmp_path: Path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is not installed")
    source = (ROOT / "src/game.cpp").read_text()
    block = extract_map_flag_block(source)
    mutant = block.replace("field_1DD70_", "decoy_")
    assert mutant != block
    ran = compile_and_run(compiler, control_harness(mutant), tmp_path,
                          f"control_mapwin_mutant_{compiler_name}")
    assert ran.returncode == 1
    assert "FAIL: live slot-zero dword was not ORed exactly" in ran.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_lifecycle_close_calls_and_reload_paths(compiler_name: str,
                                                        tmp_path: Path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is not installed")
    source = (ROOT / "src/game.cpp").read_text()
    close_body = extract_definition(source, "void __cdecl desktop_close()")
    reload_body = extract_definition(source, "int __cdecl game_reload(int mode, int reload)")
    assert close_body.count("MultiWindow.GraphicWin::close();") == 1
    assert reload_body.count("MultiWindow.GraphicWin::close();") == 1
    assert "reinterpret_cast<GraphicWin *>(&MultiWindow)" not in close_body + reload_body
    ran = compile_and_run(compiler, lifecycle_harness(close_body, reload_body),
                          tmp_path, f"lifecycle_{compiler_name}")
    assert ran.returncode == 0, ran.stdout + ran.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_compiling_unqualified_multiwin_close_mutant_is_caught(
        compiler_name: str, tmp_path: Path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is not installed")
    source = (ROOT / "src/game.cpp").read_text()
    close_body = extract_definition(source, "void __cdecl desktop_close()")
    reload_body = extract_definition(source, "int __cdecl game_reload(int mode, int reload)")
    mutant = close_body.replace(
        "MultiWindow.GraphicWin::close();", "MultiWindow.close();", 1)
    assert mutant != close_body
    ran = compile_and_run(compiler, lifecycle_harness(mutant, reload_body),
                          tmp_path, f"lifecycle_mutant_{compiler_name}")
    assert ran.returncode == 1
    assert "FAIL: desktop close order" in ran.stderr
