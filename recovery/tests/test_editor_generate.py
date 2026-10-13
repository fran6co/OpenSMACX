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


def state_storage_declarations():
    """Project the actual shared-state declaration and its real nested type."""
    fields = (ROOT / "src/game_data_state_window.h").read_text()
    base = (ROOT / "src/base.h").read_text()
    declarations = []
    for source, marker in (
        (base, "struct BaseSecretProject {"),
        (fields, "struct GameDataLockMaskBytes {"),
        (fields, "union GameDataLockMaskView {"),
        (fields, "struct GameDataStateWindowFields {"),
        (fields, "struct GameDataStateWindow {"),
    ):
        declarations.append(braced(source, source.index(marker)) + ";")
    storage_decl = re.search(
        r"extern GameDataStateWindow GameDataStateStorage;", fields).group()
    return "\n".join(declarations + [storage_decl])


def map_storage_declarations():
    """Use MapArea/MapTiles aliases over the actual MapStorage owner."""
    header = (ROOT / "src/map.h").read_text()
    declarations = [
        "typedef int BOOL;",
        re.search(r"static const int MaxLandmarkNum = \d+;", header).group(),
        braced(header, header.index("struct Landmark {")) + ";",
        braced(header, header.index("struct MapMetadataFields {")) + ";",
        braced(header, header.index("struct MapStorageWindow {")) + ";",
        re.search(r"extern MapStorageWindow MapStorage;", header).group(),
    ]
    aliases = [re.search(r"^#define " + name + r" .+$", header, re.MULTILINE).group()
               for name in ("MapArea", "MapTiles")]
    return "\n".join(declarations + ["MapStorageWindow MapStorage;"] + aliases)


def harness(method):
    header = (ROOT / "src/map.h").read_text()
    record = braced(header, header.index("struct Map {")) + ";"
    constants = [re.search(r"\b" + name + r"\s*=\s*[^,]+,", header).group()
                 for name in ("BIT_BASE_IN_TILE", "BIT_VEH_IN_TILE")]
    return r'''
        #include <cstdint>
        #include <cstddef>
        #include <cstdlib>
        #include <cstring>
        #include <initializer_list>
        MAP_RECORD
        enum { TERRAIN_CONSTANTS };
        MAP_STORAGE_DECLARATIONS
        STATE_STORAGE_DECLARATIONS
        static_assert(sizeof(int) == 4 && sizeof(Map) == 44, "host widths");
        static_assert(offsetof(GameDataStateWindowFields, game_state_) == 0x30,
                      "actual shared state member");
        static_assert(offsetof(GameDataStateWindowFields, vehicle_current_count_) == 0x38,
                      "actual vehicle count member");
        static_assert(offsetof(GameDataStateWindowFields, base_current_count_) == 0x3c,
                      "actual base count member");
        GameDataStateWindow GameDataStateStorage;
        static_assert(offsetof(Map, bit) == 8, "actual tile member");
        MAP_ACCESSOR
        void require(bool ok) { if (!ok) std::exit(42); }
        struct MapWin {
            uint32_t field_1DD98_;
            void draw_map(int);
        };
        struct Console : MapWin {
            uint32_t cursor_active_;
            int32_t selected_vehicle_;
            uint32_t field_23BF8_;
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
            require(stage == 0 && GameDataStateStorage.fields_.game_state_ == 0x11223344u);
            require(GameDataStateStorage.fields_.vehicle_current_count_ == 17 &&
                    GameDataStateStorage.fields_.base_current_count_ == 18);
            require(ConsoleGlobal.cursor_active_ == 19 && ConsoleGlobal.selected_vehicle_ == 20);
            require(ConsoleGlobal.field_23BF8_ == 21);
            stage = 1;
            // The body must acquire both the pointer and count AFTER undo.
            MapTiles = &records[1];
            MapArea = requested_count;
            GameDataStateStorage.fields_.game_state_ = undo_state;
        }
        void MapWin::draw_map(int repaint) {
            require(stage == 1 && this == &first_window && repaint == 1);
            require(field_1DD98_ == static_cast<uint32_t>(-14));
            require(other_window.field_1DD98_ == 0x55667788u);
            require(GameDataStateStorage.fields_.game_state_ == (undo_state | 4u));
            require(GameDataStateStorage.fields_.vehicle_current_count_ == 0 &&
                    GameDataStateStorage.fields_.base_current_count_ == 0);
            require(ConsoleGlobal.cursor_active_ == 0 && ConsoleGlobal.selected_vehicle_ == -1);
            require(ConsoleGlobal.field_23BF8_ == 0);
            // Includes every non-bit member and neighboring records before
            // and after the requested span, not just the touched flag words.
            require(std::memcmp(records, expected, sizeof(records)) == 0);
            stage = 2;
            GameDataStateStorage.fields_.game_state_ = member_state;
        }
        void world_build() {
            require(stage == 2 &&
                    GameDataStateStorage.fields_.game_state_ == (member_state | 0x80u));
            stage = 3;
            GameDataStateStorage.fields_.game_state_ = world_state;
        }
        void draw_map(int repaint) {
            require(repaint == 1);
            if (free_calls++ == 0) {
                require(stage == 3 &&
                        GameDataStateStorage.fields_.game_state_ == world_state);
                stage = 4;
                GameDataStateStorage.fields_.game_state_ = first_draw_state;
            } else {
                require(free_calls == 2 && !initially_set && stage == 4);
                require(GameDataStateStorage.fields_.game_state_ ==
                        (first_draw_state & ~0x80u));
                stage = 5;
                GameDataStateStorage.fields_.game_state_ = second_draw_state;
            }
        }
        void button_event(Button *receiver, bool shown, int argument) {
            require(stage == (initially_set ? 4 : 5));
            require(receiver == &MainInterfaceGlobal.flat_button_[2]);
            require(GameDataStateStorage.fields_.game_state_ == final_state && argument == 0);
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
                    GameDataStateStorage.fields_.game_state_ = 0x11223344u;
                    MapArea = 0;
                    MapTiles = decoy;
                    GameDataStateStorage.fields_.vehicle_current_count_ = 17;
                    GameDataStateStorage.fields_.base_current_count_ = 18;
                    ConsoleGlobal.cursor_active_ = 19;
                    ConsoleGlobal.selected_vehicle_ = 20;
                    ConsoleGlobal.field_23BF8_ = 21;
                    stage = free_calls = button_calls = 0;
                    Console console;
                    console.field_1DD98_ = 0xaabbccddu;
                    console.cursor_active_ = 0x13579u;
                    console.selected_vehicle_ = 73;
                    console.field_23BF8_ = 0x2468u;
                    console.editor_generate();
                    require(stage == 6 && button_calls == 1);
                    require(free_calls == (initially_set ? 1 : 2));
                    require(GameDataStateStorage.fields_.game_state_ == final_state);
                    require(console.field_1DD98_ == 0xaabbccddu);
                    require(console.cursor_active_ == 0x13579u);
                    require(console.selected_vehicle_ == 73);
                    require(console.field_23BF8_ == 0x2468u);
                    auto untouched = reinterpret_cast<unsigned char *>(decoy);
                    for (unsigned i=0; i<sizeof(decoy); ++i) require(untouched[i] == 0x55);
                }
        }
    '''.replace("MAP_RECORD", record).replace("MAP_STORAGE_DECLARATIONS",
        map_storage_declarations()).replace("STATE_STORAGE_DECLARATIONS",
        state_storage_declarations()).replace("TERRAIN_CONSTANTS", "\n".join(constants)).replace(
        "MAP_ACCESSOR", braced(header, header.index("inline Map *&map_tiles()"))).replace(
        "ACTUAL_METHOD", method)


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
    ("ConsoleGlobal.field_23BF8_ = 0;", "field_23BF8_ = 0;", 1),
    ("int count = MapArea;", "unsigned count = MapArea;", 1),
    ("tile->bit &= ~(BIT_BASE_IN_TILE | BIT_VEH_IN_TILE);", "tile->bit = 0;", 1),
    ("++tile;", "tile += 2;", 1),
    ("MapWinTable[0]", "MapWinTable[1]", 2),
    ("::draw_map(1);", "draw_map(1);", 2),
    ("f2 &= ~0x80;", "f2 &= 0x7f;", 1),
    ("int flags = GameDataStateStorage.fields_.game_state_;",
     "int flags = undo_state;", 1),
    ("int f2 = GameDataStateStorage.fields_.game_state_;",
     "int f2 = flags;", 1),
    ("MainInterfaceGlobal.flat_button_[2]", "MainInterfaceGlobal.flat_button_[3]", 2),
    ("static_cast<unsigned char>(GameDataStateStorage.fields_.game_state_)",
     "static_cast<unsigned char>(flags)", 1),
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


def random_editor_harness(methods):
    """Exercise actual typed tile access; dependencies are explicit doubles."""
    header = (ROOT / "src/map.h").read_text()
    record = braced(header, header.index("struct Map {")) + ";"
    accessor = braced(header, header.index("inline Map *&map_tiles()"))
    return r'''
        #include <cstdint>
        #include <cstdlib>
        #include <cstring>
        MAP_RECORD
        Map *MapTiles;
        MAP_ACCESSOR
        int MapArea;
        uint32_t MapRandSeed;
        Map tiles[5], expected[5], decoy[5];
        int stage, random_calls;
        struct Console {
            void editor_randomize();
            void editor_clear_random();
            void draw_map(int);
        };
        void require(bool ok) { if (!ok) std::exit(42); }
        void auto_undo() {
            require(stage++ == 0);
            MapTiles = &tiles[1];
        }
        int random(int lo, int hi) {
            require(stage == 1 && lo == 0 && hi == 0x7fff);
            ++random_calls;
            return 123;
        }
        void Console::draw_map(int repaint) {
            require(stage++ == 1 && repaint == 1);
            require(std::memcmp(tiles, expected, sizeof(tiles)) == 0);
            auto bytes = reinterpret_cast<unsigned char *>(decoy);
            for (unsigned i = 0; i < sizeof(decoy); ++i)
                require(bytes[i] == 0x55);
        }
        ACTUAL_METHODS
        int main() {
            for (int count = -1; count <= 3; ++count)
              for (int mode = 0; mode < 3; ++mode) {
                auto bytes = reinterpret_cast<unsigned char *>(tiles);
                for (unsigned i = 0; i < sizeof(tiles); ++i)
                    bytes[i] = static_cast<unsigned char>(i * 37u);
                tiles[1].bit = 0xffffffffu; // randomization flags; clear guard excludes it
                tiles[2].bit = 0x40000400u; // clear guard includes it
                tiles[3].bit = 0x40000000u; // absent bit: unchanged
                std::memcpy(expected, tiles, sizeof(tiles));
                for (int i = 0; i < count; ++i) {
                    Map &tile = expected[i + 1];
                    if (mode == 0) tile.bit &= 0xcfeffbffu;
                    else if (mode == 1 && (tile.bit & 0x400u) &&
                             !(tile.bit & 0x20000000u)) tile.bit &= 0xdffffbffu;
                    else continue;
                    for (int j = 0; j < 7; ++j) tile.bit_visible[j] = tile.bit;
                }
                std::memset(decoy, 0x55, sizeof(decoy));
                MapTiles = decoy;
                MapArea = count;
                MapRandSeed = mode == 2 ? 0u : 999u;
                stage = random_calls = 0;
                Console console;
                if (mode == 0) console.editor_randomize();
                else console.editor_clear_random();
                require(stage == 2 && random_calls == (mode == 1 ? 0 : 1));
                require(MapRandSeed == (mode == 1 ? 0u : 124u));
              }
        }
    '''.replace("MAP_RECORD", record).replace("MAP_ACCESSOR", accessor).replace(
        "ACTUAL_METHODS", methods)


def random_editor_methods():
    source = (ROOT / "src/console.cpp").read_text()
    return "\n".join(braced(source, source.index("void Console::" + name + "() {"))
                     for name in ("editor_randomize", "editor_clear_random"))


@pytest.mark.parametrize("mutation", [None, "stride", "visible", "guard", "early_pointer"])
def test_random_editors_use_real_map_members(tmp_path, mutation):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("host g++ required")
    methods = random_editor_methods()
    changes = {
        "stride": ("++tile;", "tile += 2;"),
        "visible": ("*dst = v;", "*dst = 0;"),
        "guard": ("(v & 0x20000000) == 0", "(v & 0x20000000) != 0"),
        "early_pointer": ("auto_undo();", "Map *old = map_tiles(); auto_undo(); map_tiles() = old;"),
    }
    if mutation:
        before, after = changes[mutation]
        assert before in methods
        methods = methods.replace(before, after)
    cpp = tmp_path / "random_editors.cpp"
    executable = tmp_path / "random_editors"
    cpp.write_text(random_editor_harness(methods))
    compiled = subprocess.run([compiler, "-std=c++11", "-O1", str(cpp), "-o", str(executable)],
                              capture_output=True, text=True, timeout=30)
    assert compiled.returncode == 0, compiled.stderr
    result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=10)
    assert result.returncode == (42 if mutation else 0), result.stderr
