"""Bounded actual-body editor controls, not game runtime or VC6 ABI proof.

Map storage and the method body come from product source. Window, button and
callback implementations below are explicit dependency doubles. They check
receiver identity, call order and state visible at each call; they do not claim
the original global storage, virtual layout or callback bodies are recovered.
"""

from pathlib import Path
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def braced(source, start):
    brace = source.index("{", start)
    depth = 0
    for end in range(brace, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError("unterminated source body")


def body():
    source = (ROOT / "src/console.cpp").read_text()
    return braced(source, source.index("void Console::editor_generate() {"))


def harness(method):
    header = (ROOT / "src/map.h").read_text()
    record = braced(header, header.index("struct Map {")) + ";"
    constants = [re.search(r"\b" + name + r"\s*=\s*[^,]+,", header).group()
                 for name in ("BIT_BASE_IN_TILE", "BIT_VEH_IN_TILE")]
    # Keep the production signedness of state/count globals explicit. Host
    # int and uint32_t are required to have the original 32-bit widths below.
    declarations = []
    for unit, name in (("map.h", "MapArea"), ("game.h", "GameState"),
                       ("veh.h", "VehCurrentCount"), ("base.h", "BaseCurrentCount")):
        text = (ROOT / "src" / unit).read_text()
        declarations.append(re.search(r"extern (?:uint32_t|int) " + name + r";", text).group()[7:])
    return r'''
        #include <cstdint>
        #include <cstddef>
        #include <cstdlib>
        #include <cstring>
        #include <initializer_list>
        MAP_RECORD
        enum { TERRAIN_CONSTANTS };
        GLOBAL_DECLARATIONS
        static_assert(sizeof(int) == 4 && sizeof(Map) == 44, "host widths");
        static_assert(offsetof(Map, bit) == 8, "actual tile member");
        uint8_t *LockMapTable;
        int EditorGenerateUndoDepth;
        void require(bool ok) { if (!ok) std::exit(42); }
        struct MapWin {
            uint32_t field_1DD98_;
            void draw_map(int);
        };
        struct Console : MapWin {
            uint32_t cursor_active_;
            int32_t selected_vehicle_;
            void editor_generate();
        } ConsoleGlobal;
        struct Button { void show(int); void hide(); };
        struct Interface { Button flat_button_[42]; } MainInterfaceGlobal;
        MapWin first_window, other_window;
        MapWin *MapWinTable[8];
        Map records[8], expected[8], decoy[8];
        uint32_t requested_count, undo_state, member_state, world_state;
        uint32_t first_draw_state, second_draw_state, final_state;
        int stage, free_calls, button_calls;
        bool initially_set;

        void auto_undo() {
            require(stage == 0 && GameState == 0x11223344u);
            require(VehCurrentCount == 17 && BaseCurrentCount == 18);
            require(ConsoleGlobal.cursor_active_ == 19 && ConsoleGlobal.selected_vehicle_ == 20);
            require(EditorGenerateUndoDepth == 21);
            stage = 1;
            // The body must acquire both the pointer and count AFTER undo.
            LockMapTable = reinterpret_cast<uint8_t *>(&records[1]);
            MapArea = requested_count;
            GameState = undo_state;
        }
        void MapWin::draw_map(int repaint) {
            require(stage == 1 && this == &first_window && repaint == 1);
            require(field_1DD98_ == static_cast<uint32_t>(-14));
            require(other_window.field_1DD98_ == 0x55667788u);
            require(GameState == (undo_state | 4u));
            require(VehCurrentCount == 0 && BaseCurrentCount == 0);
            require(ConsoleGlobal.cursor_active_ == 0 && ConsoleGlobal.selected_vehicle_ == -1);
            require(EditorGenerateUndoDepth == 0);
            // Includes every non-bit member and neighboring records before
            // and after the requested span, not just the touched flag words.
            require(std::memcmp(records, expected, sizeof(records)) == 0);
            stage = 2;
            GameState = member_state;
        }
        void world_build() {
            require(stage == 2 && GameState == (member_state | 0x80u));
            stage = 3;
            GameState = world_state;
        }
        void draw_map(int repaint) {
            require(repaint == 1);
            if (free_calls++ == 0) {
                require(stage == 3 && GameState == world_state);
                stage = 4;
                GameState = first_draw_state;
            } else {
                require(free_calls == 2 && !initially_set && stage == 4);
                require(GameState == (first_draw_state & ~0x80u));
                stage = 5;
                GameState = second_draw_state;
            }
        }
        void button_event(Button *receiver, bool shown, int argument) {
            require(stage == (initially_set ? 4 : 5));
            require(receiver == &MainInterfaceGlobal.flat_button_[2]);
            require(GameState == final_state && argument == 0);
            require(shown == ((final_state & 0xc0u) == 0xc0u));
            require(++button_calls == 1);
            stage = 6;
        }
        void Button::show(int n) { button_event(this, true, n); }
        void Button::hide() { button_event(this, false, 0); }

        ACTUAL_METHOD

        int main() {
            for (uint32_t count : {0u, 1u, 3u, 0x80000000u, 0xffffffffu})
              for (unsigned bits = 0; bits < 4; ++bits)
                for (unsigned callbacks_change_state = 0; callbacks_change_state < 2;
                     ++callbacks_change_state) {
                    // Distinct bytes make accidental changes to any tile
                    // field or neighbor visible; all reads are integer data.
                    auto bytes = reinterpret_cast<unsigned char *>(records);
                    for (unsigned i=0; i<sizeof(records); ++i)
                        bytes[i] = static_cast<unsigned char>(i * 37u + bits);
                    std::memcpy(expected, records, sizeof(records));
                    for (uint32_t i=0; i<(count <= 3 ? count : 0); ++i)
                        expected[i+1].bit &= ~3u;
                    std::memset(decoy, 0x55, sizeof(decoy));
                    requested_count = count;
                    undo_state = 0x81003400u | ((bits & 1) ? 0x80u : 0)
                                              | ((bits & 2) ? 0x40u : 0);
                    member_state = undo_state | 4u;
                    if (callbacks_change_state) member_state ^= 0x80u;
                    initially_set = (member_state & 0x80u) != 0;
                    world_state = member_state | 0x80u;
                    if (callbacks_change_state) world_state ^= 0x1040u;
                    first_draw_state = world_state;
                    if (callbacks_change_state) first_draw_state ^= 0x400000c0u;
                    second_draw_state = first_draw_state & ~0x80u;
                    if (callbacks_change_state) second_draw_state ^= 0x20000080u;
                    final_state = initially_set ? first_draw_state : second_draw_state;
                    first_window.field_1DD98_ = 0x12345678u;
                    other_window.field_1DD98_ = 0x55667788u;
                    for (auto &slot : MapWinTable) slot = &other_window;
                    MapWinTable[0] = &first_window;
                    GameState = 0x11223344u;
                    MapArea = 0;
                    LockMapTable = reinterpret_cast<uint8_t *>(decoy);
                    VehCurrentCount = 17; BaseCurrentCount = 18;
                    ConsoleGlobal.cursor_active_ = 19;
                    ConsoleGlobal.selected_vehicle_ = 20;
                    EditorGenerateUndoDepth = 21;
                    stage = free_calls = button_calls = 0;
                    Console console;
                    console.field_1DD98_ = 0xaabbccddu;
                    console.cursor_active_ = 0x13579u;
                    console.selected_vehicle_ = 73;
                    console.editor_generate();
                    require(stage == 6 && button_calls == 1);
                    require(free_calls == (initially_set ? 1 : 2));
                    require(GameState == final_state);
                    require(console.field_1DD98_ == 0xaabbccddu);
                    require(console.cursor_active_ == 0x13579u);
                    require(console.selected_vehicle_ == 73);
                    auto untouched = reinterpret_cast<unsigned char *>(decoy);
                    for (unsigned i=0; i<sizeof(decoy); ++i) require(untouched[i] == 0x55);
                }
        }
    '''.replace("MAP_RECORD", record).replace("TERRAIN_CONSTANTS", "\n".join(constants)).replace(
        "GLOBAL_DECLARATIONS", "\n".join(declarations)).replace("ACTUAL_METHOD", method)


def run_host(tmp_path, method):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("host g++ required")
    cpp = tmp_path / "editor.cpp"
    executable = tmp_path / "editor"
    cpp.write_text(harness(method))
    result = subprocess.run([compiler, "-std=c++11", "-O1", "-fsanitize=undefined",
                             "-fno-sanitize-recover=undefined", str(cpp), "-o", str(executable)],
                            capture_output=True, text=True, timeout=30)
    assert result.returncode == 0, result.stderr
    return subprocess.run([str(executable)], capture_output=True, text=True, timeout=10)


def test_actual_editor_body_preserves_bounded_state_and_call_contract(tmp_path):
    result = run_host(tmp_path, body())
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("before,after,count", [
    ("int count = MapArea;", "unsigned count = MapArea;", 1),
    ("tile->bit &= ~(BIT_BASE_IN_TILE | BIT_VEH_IN_TILE);", "tile->bit = 0;", 1),
    ("++tile;", "tile += 2;", 1),
    ("MapWinTable[0]", "MapWinTable[1]", 2),
    ("::draw_map(1);", "draw_map(1);", 2),
    ("f2 &= ~0x80;", "f2 &= 0x7f;", 1),
    ("int flags = GameState;", "int flags = undo_state;", 1),
    ("int f2 = GameState;", "int f2 = flags;", 1),
    ("MainInterfaceGlobal.flat_button_[2]", "MainInterfaceGlobal.flat_button_[3]", 2),
    ("static_cast<unsigned char>(GameState)", "static_cast<unsigned char>(flags)", 1),
    (".show(0);", ".show(1);", 1),
    ("auto_undo();", "", 1),
    ("ConsoleGlobal.cursor_active_", "this->cursor_active_", 1),
    ("ConsoleGlobal.selected_vehicle_", "this->selected_vehicle_", 1),
])
def test_editor_regression_mutations_are_rejected(tmp_path, before, after, count):
    method = body()
    assert method.count(before) == count
    result = run_host(tmp_path, method.replace(before, after))
    assert result.returncode == 42, result.stderr
