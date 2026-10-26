"""Host controls for the actual ProdPicker::on_scrolled source body.

The extracted production method runs against callback stubs that mutate the
same members between calls. This checks reload/caching behavior and ordering;
it is not ABI or game-runtime evidence.
"""

from pathlib import Path
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
UNIT = ROOT / "src/prodpicker.cpp"
SIGNATURE = "void ProdPicker::on_scrolled(int, int position)"


def extract_function(source: str) -> str:
    start = source.index(SIGNATURE)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        if source[end] == "{":
            depth += 1
        elif source[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start : end + 1]
    raise AssertionError("unterminated ProdPicker::on_scrolled")


HARNESS = r'''
#include <array>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

struct RECT { int unused; };
std::vector<std::string> events;
int scenario = 0;

#define CHECK(test, label) do { \
    if (!(test)) { std::cerr << "CHECK FAILED: " << label << "\n"; std::exit(7); } \
} while (0)

struct ProdPicker {
    int32_t field_A77C_ = 0;
    int32_t column_count_ = 2;
    uint32_t field_A814_ = 2;
    int32_t cell_width_ = 5;
    int32_t row_height_ = 8;
    RECT rect2_ = {0};
    std::vector<int> drawn;
    std::vector<std::array<int, 6> > blits;
    int draw_calls = 0;

    int set_clip(RECT *rect) {
        (void)rect;
        events.push_back("clip");
        if (scenario == 1) {
            column_count_ = 3; cell_width_ = 7; row_height_ = 12; field_A814_ = 4;
        } else if (scenario == 2) {
            column_count_ = 3; cell_width_ = 7; row_height_ = 10; field_A814_ = 4;
        } else if (scenario == 3) {
            column_count_ = 2; cell_width_ = 5; row_height_ = 8; field_A814_ = 2;
        }
        return 0;
    }

    int copy(ProdPicker *source, int sx, int sy, int dx, int dy, int width, int height) {
        CHECK(source == this, "copy source");
        events.push_back("copy");
        blits.push_back({sx, sy, dx, dy, width, height});
        if (scenario == 1) {
            field_A77C_ = 10; column_count_ = 2;
        } else if (scenario == 2) {
            field_A77C_ = 10; column_count_ = 2; field_A814_ = 3;
        } else if (scenario == 3) {
            field_A77C_ = -3; column_count_ = 2;
        }
        return 0;
    }

    void draw_item(int item, int update) {
        CHECK(update == 0, "draw update argument");
        events.push_back("draw");
        drawn.push_back(item);
        if (scenario == 1) {
            if (draw_calls == 0) { field_A77C_ = 12; column_count_ = 4; }
            else if (draw_calls == 2) { field_A77C_ = 0; column_count_ = 0; }
        } else if (scenario == 2) {
            if (draw_calls == 0) column_count_ = 4;
            else if (draw_calls == 2) column_count_ = 1;
        }
        ++draw_calls;
    }

    void redraw() { events.push_back("redraw"); }
    void soft_update() { events.push_back("soft"); }
    void on_scrolled(int, int position);
};

BODY

int main() {
    {
        scenario = 0; events.clear();
        ProdPicker p; p.column_count_ = 2; p.field_A77C_ = 0;
        p.on_scrolled(0, 5);
        CHECK(p.field_A77C_ == 10, "entry snapshot new position");
        CHECK(events == std::vector<std::string>({"redraw", "soft"}), "large distance path");
        CHECK(p.blits.empty() && p.drawn.empty(), "large distance skips scroll callbacks");
    }
    {
        scenario = 0; events.clear();
        ProdPicker p; p.column_count_ = 2; p.field_A77C_ = 4;
        p.on_scrolled(0, 2);
        CHECK(events == std::vector<std::string>({"soft"}), "equal position path");
    }
    {
        scenario = 1; events.clear();
        ProdPicker p; p.column_count_ = 2; p.field_A77C_ = 6;
        p.on_scrolled(0, 2); // entry page size 2 gives new position 4 and down-scroll
        CHECK(p.blits.size() == 1, "down copy count");
        CHECK(p.blits[0][0] == 0x15 && p.blits[0][1] == 4 && p.blits[0][2] == 0x15 && p.blits[0][3] == 16 && p.blits[0][4] == 25 && p.blits[0][5] == 36,
              "down post-clip width and geometry");
        CHECK(p.drawn == std::vector<int>({10, 11, 12}), "down loop refresh");
        CHECK(p.field_A77C_ == 0, "down callback mutations remain visible");
        CHECK(events == std::vector<std::string>({"clip", "copy", "draw", "draw", "draw", "soft"}),
              "down callback order and final soft update");
    }
    {
        scenario = 2; events.clear();
        ProdPicker p; p.column_count_ = 2; p.field_A77C_ = 0;
        p.on_scrolled(0, 1); // up-scroll; copy callback changes first-item inputs
        CHECK(p.blits.size() == 1, "up copy count");
        CHECK(p.blits[0][0] == 0x15 && p.blits[0][1] == 14 && p.blits[0][2] == 0x15 && p.blits[0][3] == 4 && p.blits[0][4] == 25 && p.blits[0][5] == 30,
              "up post-clip width and geometry");
        CHECK(p.drawn == std::vector<int>({14, 15, 16}), "up first item and refreshed bound");
        CHECK(events == std::vector<std::string>({"clip", "copy", "draw", "draw", "draw", "soft"}),
              "up callback order and final soft update");
    }
    {
        scenario = 3; events.clear();
        ProdPicker p; p.column_count_ = 2; p.field_A77C_ = 0;
        p.on_scrolled(0, -1); // bounded negative values: signed loop bound, no overflow
        CHECK(p.drawn == std::vector<int>({-3, -2}), "negative signed loop bound");
        CHECK(events.back() == "soft", "negative loop final soft update");
    }
    return 0;
}
'''


def compile_and_run(tmp_path, *, cached_bound_mutant=False):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host g++ compiler unavailable")
    body = extract_function(UNIT.read_text())
    if cached_bound_mutant:
        pattern = re.compile(
            r"while \(item < static_cast<int>\(field_A77C_\s*\+\s*"
            r"static_cast<uint32_t>\(column_count_\)\)\) \{"
        )
        assert len(pattern.findall(body)) == 1, "expected actual down-loop live bound"
        body = pattern.sub(
            "const int stale_end = static_cast<int>(field_A77C_ + "
            "static_cast<uint32_t>(column_count_));\n        while (item < stale_end) {",
            body,
            count=1,
        )
    source = tmp_path / ("scroll_cached_mutant.cpp" if cached_bound_mutant else "scroll_actual.cpp")
    executable = source.with_suffix("")
    source.write_text(HARNESS.replace("BODY", body))
    subprocess.run(
        [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror", str(source), "-o", str(executable)],
        check=True,
        capture_output=True,
        text=True,
    )
    return subprocess.run([str(executable)], capture_output=True, text=True)


def test_actual_scroll_method_observes_mutations_at_pinned_callback_boundaries(tmp_path):
    result = compile_and_run(tmp_path)
    assert result.returncode == 0, result.stderr


def test_compiled_old_cached_bound_mutant_fails_at_down_loop_control(tmp_path):
    result = compile_and_run(tmp_path, cached_bound_mutant=True)
    assert result.returncode != 0
    assert "CHECK FAILED: down loop refresh" in result.stderr
