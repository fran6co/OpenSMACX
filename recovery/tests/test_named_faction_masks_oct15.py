"""Actual-source controls for the named human/alive mask bytes."""

from pathlib import Path
import re
import shutil
import subprocess
import tempfile


ROOT = Path(__file__).resolve().parents[2]


def function_body(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for position in range(opening, len(source)):
        if source[position] == "{":
            depth += 1
        elif source[position] == "}":
            depth -= 1
            if depth == 0:
                return source[start:position + 1]
    raise AssertionError("unterminated function: " + signature)


def uncomment(source: str) -> str:
    return re.sub(r'//[^\n]*|/\*.*?\*/|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'',
                  "", source, flags=re.S)


def compile_and_run(program: str, *, succeeds: bool) -> None:
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    compilers = [compiler for compiler in compilers if compiler]
    assert compilers, "GCC or Clang is required for the named-mask control"
    with tempfile.TemporaryDirectory(prefix="named-faction-mask-") as temp:
        source = Path(temp) / "control.cpp"
        source.write_text(program)
        for compiler in compilers:
            binary = Path(temp) / Path(compiler).name.replace("+", "p")
            build = subprocess.run(
                [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
                 str(source), "-o", str(binary)],
                capture_output=True, text=True, timeout=45)
            assert build.returncode == 0, build.stdout + build.stderr
            result = subprocess.run([str(binary)], capture_output=True,
                                    text=True, timeout=10)
            assert (result.returncode == 0) == succeeds, result.stderr


def actual_wrapper_control() -> str:
    header = (ROOT / "src/faction.h").read_text()
    state_header = (ROOT / "src/game_data_state_window.h").read_text()
    mask_bytes = function_body(state_header, "struct GameDataLockMaskBytes {")
    mask_view = function_body(state_header, "union GameDataLockMaskView {")
    human = function_body(header, "inline BOOL __cdecl is_human(")
    alive = function_body(header, "inline BOOL __cdecl is_alive(")
    declarations = """#include <stdint.h>
#define __cdecl
typedef int BOOL;
""" + mask_bytes + ";\n" + mask_view + ";\n" + """
struct FixtureWindow { struct Fields { GameDataLockMaskView lock_masks_; } fields_; };
FixtureWindow GameDataStateStorage;
""" + human + "\n" + alive + """
int main() {
    GameDataStateStorage.fields_.lock_masks_.enable_mask_ = 0x00000102u;
    if (!is_human(1) || is_human(0)) return 1;
    if (!is_alive(0) || is_alive(1)) return 2;
    GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_ = 0x02u;
    if (is_alive(0) || !is_alive(1)) return 3;
    return 0;
}
"""
    return declarations


def test_status_wrappers_use_the_named_byte_members_and_preserve_unmasked_shift():
    header = (ROOT / "src/faction.h").read_text()
    assert "inline uint8_t __cdecl factions_status_byte" not in header
    human = function_body(header, "inline BOOL __cdecl is_human(")
    alive = function_body(header, "inline BOOL __cdecl is_alive(")
    assert "bytes_.human_mask_ & (1u << faction_id)" in human
    assert "bytes_.alive_mask_ & (1u << faction_id)" in alive
    assert "faction_id & 31" not in human + alive


def test_direct_mask_reads_use_named_members_and_keep_explicit_snapshots():
    source_files = (
        "src/faction.cpp", "src/probe.cpp", "src/game.cpp", "src/pickwin.cpp",
        "src/reportwin.cpp", "src/base.cpp", "src/newgame.cpp", "src/veh.cpp",
    )
    source = "\n".join((ROOT / name).read_text() for name in source_files)
    code = uncomment(source)
    assert "factions_status_byte(" not in code
    assert "lock_masks_.bytes_.human_mask_" in code
    assert "lock_masks_.bytes_.alive_mask_" in code

    tamper = function_body((ROOT / "src/game.cpp").read_text(),
                           "void __cdecl check_tamper()")
    assert "const uint8_t human_mask = GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_;" in tamper
    assert "const uint8_t alive_mask = GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_;" in tamper

    production_tail = function_body((ROOT / "src/base.cpp").read_text(),
                                    "bool production_project_events_tail(")
    assert "const uint8_t human = GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_;" in production_tail


def test_all_changed_direct_reads_use_byte_zero_or_one_without_changing_shift_forms():
    game = (ROOT / "src/game.cpp").read_text()
    faction = (ROOT / "src/faction.cpp").read_text()
    veh = (ROOT / "src/veh.cpp").read_text()
    assert "static_cast<uint8_t>(1 << turncoat_faction) & GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_" in faction
    assert "GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_ & (1 << other)" in (ROOT / "src/reportwin.cpp").read_text()
    assert "GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ &\n         static_cast<uint8_t>(1u << (static_cast<unsigned>(faction) & 31u))" in veh
    assert "const uint8_t human_mask = GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_;" in game


def test_compiled_production_wrappers_read_the_named_bytes_live():
    program = actual_wrapper_control()
    compile_and_run(program, succeeds=True)
    alive = function_body((ROOT / "src/faction.h").read_text(),
                          "inline BOOL __cdecl is_alive(")
    old = "bytes_.alive_mask_"
    assert alive.count(old) == 1
    mutant = program.replace(alive, alive.replace(old, "bytes_.human_mask_", 1), 1)
    compile_and_run(mutant, succeeds=False)
