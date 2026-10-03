"""Controls for the source-extracted Console::selected body and return value."""

from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def selected_body():
    source = (ROOT / "src/console.cpp").read_text()
    start = source.index("int Console::selected()")
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            body = source[start:end + 1]
            return body.replace("int Console::selected()", "int ConsoleMock::selected()", 1)
    raise AssertionError("unterminated Console::selected source")


HARNESS = r'''#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <vector>

static void require(bool ok, int line) {
    if (!ok) { std::fprintf(stderr, "assertion at %d\n", line); std::exit(98); }
}
#define REQUIRE(x) require((x), __LINE__)

static std::vector<int> events;
static const int MapWinTableSlots = 8;
struct MapWin;
static MapWin *MapWinTable[MapWinTableSlots];
static int mutate_slot = -1;
static MapWin *replacement_slot = nullptr;
static void do_non_input() { events.push_back(90); }

struct GraphicWin {
    int slot;
    explicit GraphicWin(int value = -1) : slot(value) {}
    void set_cursor(int resource_id);
};
struct MapWin : GraphicWin {
    int field_1DD74_;
    uint32_t field_1DD70_;
    void *cursor_sprite_;
    uint32_t cursor_resource_id_;
    int draws;
    explicit MapWin(int slot_id = -1) : GraphicWin(slot_id), field_1DD74_(0),
        field_1DD70_(0), cursor_sprite_(nullptr), cursor_resource_id_(0x7f00),
        draws(0) {}
    void draw_map(int draw_type) {
        REQUIRE(draw_type == 1);
        ++draws;
        events.push_back(100 + slot);
    }
};
void GraphicWin::set_cursor(int resource_id) {
    REQUIRE(resource_id == 0x7f00);
    events.push_back(10 + slot);
    if (slot == mutate_slot) MapWinTable[slot] = replacement_slot;
}
struct ConsoleMock {
    uint32_t field_23D10_;
    int selected();
} ConsoleGlobal;

CONSOLE_SELECTED_BODY

static void reset() {
    events.clear();
    for (int slot = 0; slot < MapWinTableSlots; ++slot) MapWinTable[slot] = nullptr;
    mutate_slot = -1; replacement_slot = nullptr;
    ConsoleGlobal.field_23D10_ = 0;
}

static void zero_mode_is_neutral() {
    reset();
    MapWin primary(0);
    primary.cursor_resource_id_ = 1;
    primary.field_1DD70_ = 0xc40000;
    MapWinTable[0] = &primary;
    REQUIRE(ConsoleGlobal.selected() == 0);
    REQUIRE(ConsoleGlobal.field_23D10_ == 0);
    REQUIRE(events.empty());
}

static void return_and_live_slot_reload() {
    reset();
    MapWin primary(0), original_secondary(1), replacement(1), inactive(2);
    primary.cursor_sprite_ = reinterpret_cast<void *>(1);
    primary.cursor_resource_id_ = 1;
    primary.field_1DD70_ = 0xc40000;
    original_secondary.field_1DD74_ = 1;
    original_secondary.cursor_sprite_ = reinterpret_cast<void *>(2);
    original_secondary.cursor_resource_id_ = 1;
    replacement.field_1DD74_ = 1;
    replacement.field_1DD70_ = 0xc40000;
    replacement.cursor_sprite_ = reinterpret_cast<void *>(3);
    replacement.cursor_resource_id_ = 1;
    inactive.field_1DD74_ = 0;
    inactive.field_1DD70_ = 0xc40000;
    MapWinTable[0] = &primary;
    MapWinTable[1] = &original_secondary;
    MapWinTable[2] = &inactive;
    mutate_slot = 1;
    replacement_slot = &replacement;
    ConsoleGlobal.field_23D10_ = 4;

    REQUIRE(ConsoleGlobal.selected() == 4);
    REQUIRE(ConsoleGlobal.field_23D10_ == 0);
    REQUIRE(primary.cursor_sprite_ == nullptr && primary.cursor_resource_id_ == 0x7f00);
    REQUIRE(original_secondary.cursor_sprite_ == nullptr &&
            original_secondary.cursor_resource_id_ == 0x7f00);
    REQUIRE(primary.draws == 1);
    REQUIRE(replacement.draws == 1 && replacement.field_1DD70_ == 0);
    REQUIRE(inactive.draws == 0);
    REQUIRE((events == std::vector<int>{10, 100, 90, 11, 101, 90}));
}

int main() { zero_mode_is_neutral(); return_and_live_slot_reload(); }
'''


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_source_selected_return(compiler, tmp_path):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(f"{compiler} is not installed")
    source = HARNESS.replace("CONSOLE_SELECTED_BODY", selected_body())
    cpp = tmp_path / "console_selected.cpp"
    binary = tmp_path / "console_selected"
    cpp.write_text(source)
    subprocess.run([executable, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                    str(cpp), "-o", str(binary)], check=True, timeout=30)
    subprocess.run([str(binary)], check=True, timeout=30)
