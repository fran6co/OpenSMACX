"""Bounded host controls for Console cursor storage, not native ABI evidence.

Cursor/survey/draw bodies and scalar/array declarations are extracted from
product source. Dependencies are explicit doubles. Constructor and close tests
execute only their contiguous scalar/history initialization regions; they do
not exercise base/member lifetimes, close callbacks, or full-object layout.
Original behavior is grounded in task history 39: receiver writes coexist with
absolute ConsoleGlobal reads, and resets repeatedly clear slot zero only.
"""

from pathlib import Path
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def braced(source, signature):
    start = source.index(signature)
    brace = source.index("{", start)
    depth = 0
    for end in range(brace, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError("unterminated product body")


def bodies():
    console = (ROOT / "src/console.cpp").read_text()
    result = {name: braced(console, "void Console::" + signature)
              for name, signature in (
                  ("next", "cursor_next(int x_coord, int y_coord) {"),
                  ("prev", "prev_cursor() {"), ("survey", "survey() {"))}
    result["draw"] = braced((ROOT / "src/mapwin.cpp").read_text(),
                            "void __cdecl draw_cursor() {")
    for key, signature in (("construct_region", "Console::Console()"),
                           ("close_region", "void Console::close() {")):
        method = braced(console, signature)
        start = method.index("field_23BC8_ =")
        end = method.index("field_23D10_ =", start)
        result[key] = "void Console::" + key + "() {\n" + method[start:end] + "}"
    return result


def declaration(header, name):
    matches = re.findall(r"^\s*((?:u?int32_t|int)\s+" + re.escape(name)
                         + r"(?:\[[^\]]+\])?\s*;)", header, re.M)
    assert len(matches) == 1, (name, matches)
    return matches[0]


def harness(methods):
    header = (ROOT / "src/console.h").read_text()
    names = ["field_23BC8_", "cursor_active_", "selected_vehicle_",
             "field_23BE4_", "field_23BE8_", "field_23BF0_", "field_23BF4_",
             "field_23BF8_", "field_23BFC_", "survey_mode_", "field_23C04_",
             "cursor_history_index_", "cursor_history_head_",
             "cursor_history_x_", "cursor_history_y_"]
    declarations = "\n".join(declaration(header, name) for name in names)
    # Deliberately separate the arrays in this HOST double. Reintroducing an
    # X-plus-32 store must fail even if it happens to hit Y in native layout.
    declarations = declarations.replace(declaration(header, "cursor_history_y_"),
                                        "int32_t between_arrays[32];\n"
                                        + declaration(header, "cursor_history_y_"))
    map_header = (ROOT / "src/mapwin.h").read_text()
    map_fields = "\n".join(declaration(map_header, name) for name in (
        "field_1DD70_", "field_1DD74_", "field_1DDA4_", "field_1DDA8_",
        "field_1DDCC_", "field_1DDD0_", "field_1DDD4_", "field_1DDD8_"))
    return r'''
        #include <cstdint>
        #include <cstdlib>
        #include <cstdio>
        #include <cstring>
        #include <type_traits>
        #include <initializer_list>
        #define __cdecl
        #define require(condition) do { if (!(condition)) { \
            std::fprintf(stderr, "line %d: %s\n", __LINE__, #condition); \
            std::exit(42); } } while (0)
        static_assert(sizeof(int) == 4, "host integer width");
        struct Menu { void check(int) { require(false); } };
        struct Console {
            CONSOLE_FIELDS
            Menu alpha_menu_;
            void cursor_next(int, int);
            void prev_cursor();
            void survey();
            void construct_region();
            void close_region();
        };
        Console ConsoleGlobal, receiver;
        struct MapWin {
            MAP_FIELDS
            void draw_cursor();
            void draw_map(int);
            void focus(int, int);
            void main_caption() { require(false); }
            void draw_radius(int, int, int, int);
        } window;
        typedef MapWin *MapWinSlots[8];
        MapWinSlots MapWinTable;
        const unsigned MapWinTableSlots = 8;
        uint32_t MapIsFlat = 1;
        int MapLongitude = 256;
        MapWin *console_map_win() { return &window; }
        struct Status { void set_loc(int, int); void redraw() { require(false); } } ConsoleStatusWin;
        struct Info { void change(int) { require(false); } } ConsoleInfoWin;
        struct Tutorial { void draw_arrow(); } tutorial;
        Tutorial *TutWinGlobal = &tutorial;
        uint32_t GameState = 0;
        const uint32_t STATE_DEBUG_MODE = 0x100;
        const char *CursorNextDangerMsg1 = "first", *CursorNextDangerMsg2 = "second";
        void danger(const char *, const char *, int, int, int) { require(false); }
        void ambience(int) { require(false); }
        void draw_cursor();
        int scenario, status_calls, map_calls, cursor_calls, focus_calls, radius_calls, arrow_calls;
        int expected_x, expected_y, replacement_index, replacement_x, replacement_y;
        bool change_status;
        uint32_t expected_mode, expected_mask;
        void Status::set_loc(int x, int y) {
            require(this == &ConsoleStatusWin && ++status_calls == 1);
            require(x == expected_x && y == expected_y);
            if (change_status) {
                ConsoleGlobal.cursor_history_index_ = replacement_index;
                ConsoleGlobal.cursor_history_x_[replacement_index] = replacement_x;
                ConsoleGlobal.cursor_history_y_[replacement_index] = replacement_y;
            }
        }
        void MapWin::draw_cursor() {
            require(this == &window && scenario == 2 && ++cursor_calls == 1);
        }
        void MapWin::draw_map(int repaint) {
            require(this == &window && repaint == 1 && ++map_calls == 1);
            require(receiver.survey_mode_ == expected_mode);
            if (scenario == 3) {
                require(field_1DD70_ == expected_mask);
                // A normal overlay step must restore its saved mask even
                // when drawing changes it; the zero-state step has no restore.
                field_1DD70_ = 0xdeadbeef;
            } else require(scenario == 2);
        }
        void MapWin::focus(int x, int y) {
            require(this == &window && scenario == 2 && status_calls == 1);
            require(++focus_calls == 1 && x == replacement_x && y == replacement_y);
        }
        void MapWin::draw_radius(int x, int y, int a, int b) {
            require(this == &window && scenario == 4);
            require(++radius_calls == 1 && x == 4 && y == 6 && a == 0 && b == 2);
        }
        void Tutorial::draw_arrow() { require(++arrow_calls == 1); }

        ACTUAL_BODIES

        void seed(Console &c, int bias) {
            std::memset(&c, 0, sizeof(c));
            c.field_23BF8_ = 81;
            c.survey_mode_ = 3;
            c.cursor_history_index_ = 7;
            c.cursor_history_head_ = 19;
            for (int i = 0; i < 32; ++i) {
                c.cursor_history_x_[i] = bias + i;
                c.cursor_history_y_[i] = -bias - i;
                c.between_arrays[i] = 0x13579;
            }
        }
        void reset_calls() {
            status_calls = map_calls = cursor_calls = focus_calls = radius_calls = arrow_calls = 0;
            change_status = false;
        }
        void check_arrays(const Console &actual, const Console &expected) {
            require(std::memcmp(actual.cursor_history_x_, expected.cursor_history_x_,
                                sizeof(actual.cursor_history_x_)) == 0);
            require(std::memcmp(actual.cursor_history_y_, expected.cursor_history_y_,
                                sizeof(actual.cursor_history_y_)) == 0);
            require(std::memcmp(actual.between_arrays, expected.between_arrays,
                                sizeof(actual.between_arrays)) == 0);
        }
        int main() {
            // These are actual member declarations, but this compact host
            // class does not establish original offsets, extent or ABI.
            require((std::is_same<decltype(receiver.survey_mode_), uint32_t>::value));
            require((std::is_same<decltype(receiver.field_23BF8_), uint32_t>::value));
            require((std::is_same<decltype(receiver.cursor_history_index_), int32_t>::value));
            require((std::is_same<decltype(receiver.cursor_history_head_), int32_t>::value));
            require((std::is_same<decltype(receiver.cursor_history_x_), int32_t[32]>::value));
            require((std::is_same<decltype(receiver.cursor_history_y_), int32_t[32]>::value));
            for (bool close : {false, true}) {
                seed(receiver, 200); seed(ConsoleGlobal, 700);
                Console expected = receiver, global_before = ConsoleGlobal;
                expected.cursor_history_x_[0] = expected.cursor_history_y_[0] = 0;
                if (close) receiver.close_region(); else receiver.construct_region();
                check_arrays(receiver, expected);
                require(receiver.cursor_history_index_ == 0 && receiver.cursor_history_head_ == 19);
                require(receiver.survey_mode_ == 0 && receiver.field_23BF8_ == 0);
                require(std::memcmp(&ConsoleGlobal, &global_before, sizeof(ConsoleGlobal)) == 0);
            }

            scenario = 1;
            for (int head : {0, 30, 31, -1})
              for (int match_after_callback : {0, 1, 2}) {
                seed(receiver, 200); seed(ConsoleGlobal, 700); reset_calls();
                receiver.cursor_history_head_ = head;
                receiver.survey_mode_ = 2;
                Console expected = receiver;
                expected_x = -12; expected_y = -8;
                change_status = true; replacement_index = 11;
                replacement_x = match_after_callback ? expected_x : 82;
                replacement_y = match_after_callback == 1 ? expected_y : 84;
                Console expected_global = ConsoleGlobal;
                expected_global.cursor_history_index_ = replacement_index;
                expected_global.cursor_history_x_[replacement_index] = replacement_x;
                expected_global.cursor_history_y_[replacement_index] = replacement_y;
                if (match_after_callback != 1) {
                    const int slot = head == 31 ? 0 : head + 1;
                    expected.cursor_history_head_ = expected.cursor_history_index_ = slot;
                    expected.cursor_history_x_[slot] = expected_x;
                    expected.cursor_history_y_[slot] = expected_y;
                }
                receiver.cursor_next(expected_x, expected_y);
                require(status_calls == 1);
                require(receiver.cursor_history_head_ == expected.cursor_history_head_);
                require(receiver.cursor_history_index_ == expected.cursor_history_index_);
                require(receiver.survey_mode_ == 2 && receiver.field_23BF8_ == 81);
                check_arrays(receiver, expected);
                require(std::memcmp(&ConsoleGlobal, &expected_global, sizeof(ConsoleGlobal)) == 0);
              }
            // Same process-global receiver must work too; append then duplicate.
            seed(ConsoleGlobal, 700); reset_calls();
            ConsoleGlobal.cursor_history_head_ = 31;
            expected_x = 40; expected_y = 42;
            ConsoleGlobal.cursor_next(40, 42);
            require(ConsoleGlobal.cursor_history_head_ == 0 && ConsoleGlobal.cursor_history_index_ == 0);
            require(ConsoleGlobal.cursor_history_x_[0] == 40 && ConsoleGlobal.cursor_history_y_[0] == 42);
            reset_calls(); ConsoleGlobal.cursor_next(40, 42);
            require(ConsoleGlobal.cursor_history_head_ == 0 && status_calls == 1);

            scenario = 2;
            for (int index : {0, 1, 31})
              for (uint32_t mode : {0u, 3u}) {
                seed(receiver, 200); seed(ConsoleGlobal, 700); reset_calls();
                receiver.cursor_history_index_ = index; receiver.survey_mode_ = mode;
                Console before = receiver;
                expected_mode = 0;
                expected_x = ConsoleGlobal.cursor_history_x_[7];
                expected_y = ConsoleGlobal.cursor_history_y_[7];
                change_status = true; replacement_index = 17;
                replacement_x = -44; replacement_y = -46;
                receiver.prev_cursor();
                require(receiver.cursor_history_index_ == (index == 0 ? 31 : index - 1));
                require(receiver.cursor_history_head_ == 19 && receiver.field_23BF8_ == 0);
                require(receiver.survey_mode_ == 0);
                require(cursor_calls == 1 && focus_calls == 1 && map_calls == (mode ? 1 : 0));
                check_arrays(receiver, before);
              }

            scenario = 3;
            seed(receiver, 200); seed(ConsoleGlobal, 700);
            receiver.survey_mode_ = 0;
            const uint32_t masks[] = {0x623, 0x603, 0x402, 0x12345678};
            for (unsigned step = 0; step < 4; ++step) {
                reset_calls(); window.field_1DD70_ = 0x12345678;
                expected_mode = (step + 1) % 4; expected_mask = masks[step];
                receiver.survey();
                require(receiver.survey_mode_ == expected_mode && map_calls == 1);
                require(window.field_1DD70_ == (step == 3 ? 0xdeadbeef : 0x12345678));
                require(ConsoleGlobal.survey_mode_ == 3 && receiver.field_23BF8_ == 81);
            }

            scenario = 4;
            seed(ConsoleGlobal, 700);
            ConsoleGlobal.cursor_history_index_ = 5;
            ConsoleGlobal.cursor_history_x_[5] = 4; ConsoleGlobal.cursor_history_y_[5] = 6;
            std::memset(&window, 0, sizeof(window));
            window.field_1DDCC_ = window.field_1DDD0_ = 20;
            MapWinTable[0] = &window;
            for (uint32_t mode : {0u, 1u, 2u, 3u}) {
                reset_calls(); ConsoleGlobal.survey_mode_ = mode;
                // The independent flag is deliberately opposite the gate.
                ConsoleGlobal.field_23BF8_ = mode ? 0 : 1;
                draw_cursor();
                require(radius_calls == (mode == 0 ? 1 : 0) && arrow_calls == 1);
            }
        }
    '''.replace("CONSOLE_FIELDS", declarations).replace("MAP_FIELDS", map_fields).replace(
        "ACTUAL_BODIES", "\n".join(methods.values()))


def run_host(tmp_path, methods):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("host g++ required")
    source = tmp_path / "cursor.cpp"
    executable = tmp_path / "cursor"
    source.write_text(harness(methods))
    compiled = subprocess.run(
        [compiler, "-std=c++11", "-O1", "-fsanitize=undefined",
         "-fno-sanitize-recover=undefined", str(source), "-o", str(executable)],
        capture_output=True, text=True, timeout=30)
    assert compiled.returncode == 0, compiled.stderr
    return subprocess.run([str(executable)], capture_output=True, text=True, timeout=10)


def test_actual_console_cursor_storage_bodies(tmp_path):
    result = run_host(tmp_path, bodies())
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("method,before,after", [
    ("next", "ConsoleGlobal.cursor_history_index_", "cursor_history_index_"),
    ("next", "ConsoleGlobal.cursor_history_x_[idx]", "cursor_history_x_[idx]"),
    ("next", "ConsoleGlobal.cursor_history_y_[idx]", "cursor_history_y_[idx]"),
    ("next", "ConsoleStatusWin.set_loc(x_coord, y_coord);\n    int idx = ConsoleGlobal.cursor_history_index_;",
     "int idx = ConsoleGlobal.cursor_history_index_;\n    ConsoleStatusWin.set_loc(x_coord, y_coord);"),
    ("next", "if (y_coord == ConsoleGlobal.cursor_history_y_[idx])", "if (true)"),
    ("next", "% 32", "% 31"),
    ("next", "cursor_history_y_[reIdx] = y_coord;",
     "reinterpret_cast<int32_t *>(cursor_history_x_)[32 + reIdx] = y_coord;"),
    ("prev", "cursor_history_index_ = 0x1F;", "cursor_history_index_ = 0;"),
    ("prev", "const int slot_now = ConsoleGlobal.cursor_history_index_;", "const int slot_now = slot;"),
    ("prev", "const int slot = ConsoleGlobal.cursor_history_index_;", "const int slot = cursor_history_index_;"),
    ("construct_region", "cursor_history_index_ = 0;", "cursor_history_index_ = 0; cursor_history_head_ = 0;"),
    ("construct_region", "cursor_history_x_[cursor_history_index_] = 0;", "cursor_history_x_[i - 1] = 0;"),
    ("close_region", "cursor_history_index_ = 0;", "cursor_history_index_ = 0; cursor_history_head_ = 0;"),
    ("close_region", "cursor_history_x_[idx1] = 0;", "cursor_history_x_[count] = 0;"),
    ("close_region", "cursor_history_y_[idx2] = 0;", "cursor_history_y_[count] = 0;"),
    ("survey", "survey_mode_ = 2;", "survey_mode_ = 1;"),
    ("survey", "0x603", "0x623"),
    ("draw", "ConsoleGlobal.survey_mode_ == 0", "ConsoleGlobal.field_23BF8_ == 0"),
])
def test_compiling_storage_mutations_are_rejected(tmp_path, method, before, after):
    actual = bodies()
    assert actual[method].count(before) == 1
    actual[method] = actual[method].replace(before, after)
    result = run_host(tmp_path, actual)
    assert result.returncode == 42, result.stderr
