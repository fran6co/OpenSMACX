"""Source-linked controls for fullword owners consumed as low-byte flags."""
from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def function_body(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for position in range(opening, len(source)):
        depth += (source[position] == "{") - (source[position] == "}")
        if depth == 0:
            return source[start:position + 1]
    raise AssertionError("unterminated source function: " + signature)


def declaration(body: str, prefix: str) -> str:
    matches = [line.split("//", 1)[0].strip() for line in body.splitlines()
               if line.strip().startswith(prefix)]
    assert len(matches) == 1, (prefix, matches)
    return matches[0]


def condition(body: str, token: str, after: int = 0) -> str:
    lines = [line.strip() for line in body.splitlines()
             if line.strip().startswith("if (") and token in line]
    assert len(lines) > after, (token, lines)
    return lines[after]


def projections():
    console = (ROOT / "src/console.cpp").read_text()
    mapwin = (ROOT / "src/mapwin.cpp").read_text()
    state_header = (ROOT / "src/game_data_state_window.h").read_text()
    map_header = (ROOT / "src/map.h").read_text()
    editor = function_body(console, "void Console::editor_fast()")
    autoplay = function_body(console, "void __cdecl auto_play_callback(int)")
    dimensions = function_body(mapwin, "void MapWin::calculate_dimensions()")
    state_field = declaration(state_header, "uint32_t game_state_")
    flat_field = declaration(map_header, "BOOL is_flat_")
    return {
        "game_state_field": state_field,
        "flat_field": flat_field,
        "editor_decl": declaration(editor, "unsigned char al ="),
        "editor_if": condition(editor, "al & 0x80"),
        "autoplay_decl": declaration(autoplay, "unsigned char state ="),
        "autoplay_if": condition(autoplay, "state & 2"),
        "flat_decl": declaration(dimensions, "uint8_t wrapFlag ="),
        "flat_nonwrap_if": condition(dimensions, "wrapFlag & 1"),
        "flat_wrap_if": condition(dimensions, "wrapFlag & 1", 1),
    }


HARNESS = r'''#include <cassert>
#include <cstdint>
#include <cstring>
typedef int BOOL;
struct GameDataStateFields { GAME_STATE_FIELD };
struct GameDataStateWindow { GameDataStateFields fields_; };
GameDataStateWindow GameDataStateStorage;
struct MapMetadataFields { FLAT_FIELD };
struct MapStorageWindow { MapMetadataFields metadata_; };
MapStorageWindow MapStorage;
#define MapIsFlat (MapStorage.metadata_.is_flat_)

int editor_flags(uint32_t value) {
    GameDataStateStorage.fields_.game_state_ = value;
    EDITOR_DECL
    EDITOR_IF return 1; }
    return 0;
}
int autoplay_flag(uint32_t value) {
    GameDataStateStorage.fields_.game_state_ = value;
    AUTOPLAY_DECL
    AUTOPLAY_IF return 1; }
    return 0;
}
int dimensions_flags(int value) {
    MapIsFlat = value;
    FLAT_DECL
    int result = 0;
    FLAT_NONWRAP_IF result |= 1; }
    FLAT_WRAP_IF result |= 2; }
    return result;
}
int main() {
    const uint32_t high_words[] = {
        0x00000000u, 0x12345600u, 0x7fffff00u, 0x80000000u, 0xffffff00u
    };
    for (unsigned int high = 0; high < sizeof(high_words) / sizeof(high_words[0]); ++high) {
        for (unsigned int low = 0; low < 256; ++low) {
            const uint32_t bits = high_words[high] | low;
            assert(editor_flags(bits) == ((low & 0xc0u) == 0xc0u));
            assert(autoplay_flag(bits) == ((low & 2u) != 0));
            int map_flag;
            std::memcpy(&map_flag, &bits, sizeof(map_flag));
            assert(dimensions_flags(map_flag) == ((low & 1u) != 0 ? 2 : 1));
        }
    }
    return 0;
}
'''


def compile_run(tmp_path, parts):
    compiler = shutil.which("clang++") or shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler required")
    source_text = HARNESS
    for key, value in parts.items():
        source_text = source_text.replace(key.upper(), value)
    source = tmp_path / "scalar-flags.cpp"
    source.write_text(source_text)
    binary = tmp_path / "scalar-flags"
    built = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", "-pedantic",
         str(source), "-o", str(binary)], capture_output=True, text=True,
        timeout=30,
    )
    assert built.returncode == 0, built.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True,
                          timeout=10)


def test_actual_scalar_conversions_preserve_low_byte_flag_behavior(tmp_path):
    parts = projections()
    assert "reinterpret_cast" not in parts["editor_decl"]
    assert "reinterpret_cast" not in parts["autoplay_decl"]
    assert "reinterpret_cast" not in parts["flat_decl"]
    result = compile_run(tmp_path, parts)
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize(("part", "old", "new"), [
    ("editor_if", "0x40", "0x20"),
    ("autoplay_if", "& 2", "& 4"),
    ("flat_wrap_if", "& 1", "& 2"),
])
def test_behavior_controls_reject_flag_mask_mutations(tmp_path, part, old, new):
    parts = projections()
    assert old in parts[part], (part, parts[part])
    parts[part] = parts[part].replace(old, new, 1)
    result = compile_run(tmp_path, parts)
    assert result.returncode != 0, part + " mask mutation escaped controls"


def test_mapwin_value_conversion_remains_at_the_current_read_site():
    parts = projections()
    assert "static_cast<uint8_t>(MapIsFlat)" in parts["flat_decl"]
    dimensions = function_body(
        (ROOT / "src/mapwin.cpp").read_text(),
        "void MapWin::calculate_dimensions()",
    )
    assert dimensions.index("int32_t panXinit =") < dimensions.index(
        parts["flat_decl"]
    ) < dimensions.index("int32_t panX;")
