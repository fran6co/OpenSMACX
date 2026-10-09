"""Behavior controls that compile the current five Scroll mutator bodies."""
from __future__ import annotations

from pathlib import Path
import shutil
import subprocess
import tempfile

import pytest

from match_recovery.source import read_file


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/scroll.cpp").is_file())
CPP = ROOT / "src/scroll.cpp"
HEADER = ROOT / "src/scroll.h"
GENERAL = ROOT / "src/general.h"
SWAP_SIGNATURE = "MEASURED inline void __cdecl swap(int *var1, int *var2)"
RANGE_SWAP_SIGNATURE = "static void __cdecl range_swap_call(int *var1, int *var2)"
FUNCTIONS = {
    "set_range": (0x006059B0, "void Scroll::set_range(int minimum, int maximum)"),
    "set_button_color": (0x00605A10, "void Scroll::set_button_color(int color)"),
    "set_bevel_thickness": (0x00605A50, "void Scroll::set_bevel_thickness(int thickness)"),
    "set_bevel_upper": (0x00605A90, "void Scroll::set_bevel_upper(int color)"),
    "set_bevel_lower": (0x00605AD0, "void Scroll::set_bevel_lower(int color)"),
}

FIXTURE = r'''#include <assert.h>
#include <stdint.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#define MEASURED
#define __cdecl

static int event_count;
static int event_ids[8];
static int event_min[8], event_pos[8], event_max[8], event_reversed[8];
static uint32_t event_values[8][8];
struct Scroll;
struct FlatButton {
    uint32_t color_, bevel_thickness_, bevel_upper_, bevel_lower_;
    int id;
    Scroll *owner;
    void (*hook)(FlatButton *);
    void redraw();
};
struct Scroll {
    int range_reversed_, range_minimum_, position_, range_maximum_;
    uint32_t color_, bevel_thickness_, bevel_upper_, bevel_lower_;
    FlatButton flat_button_left_, flat_button_right_;
    void set_range(int minimum, int maximum);
    void set_button_color(int color);
    void set_bevel_thickness(int thickness);
    void set_bevel_upper(int color);
    void set_bevel_lower(int color);
    virtual ~Scroll() {}
    virtual void redraw() {
        const int i = event_count++;
        event_ids[i] = 0;
        event_min[i] = range_minimum_; event_pos[i] = position_;
        event_max[i] = range_maximum_; event_reversed[i] = range_reversed_;
    }
};
struct RedrawSpy : Scroll {
    int overrides;
    RedrawSpy() : overrides(0) {}
    void redraw() override {
        ++overrides;
        const int i = event_count++;
        event_ids[i] = 0;
        event_min[i] = range_minimum_; event_pos[i] = position_;
        event_max[i] = range_maximum_; event_reversed[i] = range_reversed_;
    }
};
void FlatButton::redraw() {
    const int i = event_count++;
    event_ids[i] = id;
    event_values[i][0] = owner->color_;
    event_values[i][1] = owner->bevel_thickness_;
    event_values[i][2] = owner->bevel_upper_;
    event_values[i][3] = owner->bevel_lower_;
    event_values[i][4] = color_;
    event_values[i][5] = bevel_thickness_;
    event_values[i][6] = bevel_upper_;
    event_values[i][7] = bevel_lower_;
    if (hook) hook(this);
}
'''

SUFFIX = r'''
static void reset(Scroll &s) {
    memset(event_ids, 0, sizeof(event_ids)); memset(event_min, 0, sizeof(event_min));
    memset(event_pos, 0, sizeof(event_pos)); memset(event_max, 0, sizeof(event_max));
    memset(event_reversed, 0, sizeof(event_reversed));
    memset(event_values, 0, sizeof(event_values)); event_count = 0;
    s.range_reversed_ = s.range_minimum_ = s.position_ = s.range_maximum_ = 0;
    s.color_ = s.bevel_thickness_ = s.bevel_upper_ = s.bevel_lower_ = 0;
    s.flat_button_left_ = FlatButton(); s.flat_button_right_ = FlatButton();
    s.flat_button_left_.id = 1; s.flat_button_right_.id = 2;
    s.flat_button_left_.owner = s.flat_button_right_.owner = &s;
}
static Scroll *callback_owner;
static void mutate_from_left(FlatButton *) {
    callback_owner->position_ = 77;
    callback_owner->color_ = 0xDEADBEEFu;
}
static void test_ranges() {
    RedrawSpy s; reset(s);
    s.set_range(-4, 9);
    assert(s.range_reversed_ == 0 && s.range_minimum_ == -4 &&
           s.position_ == -4 && s.range_maximum_ == 9);
    assert(s.overrides == 1 && event_count == 1 && event_ids[0] == 0 && event_min[0] == -4 &&
           event_pos[0] == -4 && event_max[0] == 9 && event_reversed[0] == 0);
    reset(s); s.set_range(9, -4);
    assert(s.range_reversed_ == 1 && s.range_minimum_ == -4 &&
           s.position_ == -4 && s.range_maximum_ == 9);
    assert(s.overrides == 2 && event_count == 1 && event_reversed[0] == 1);
    reset(s); s.set_range(6, 6);
    assert(s.range_reversed_ == 1 && s.range_minimum_ == 6 &&
           s.position_ == 6 && s.range_maximum_ == 6);
    reset(s); s.set_range(INT_MIN, INT_MAX);
    assert(s.range_reversed_ == 0 && s.range_minimum_ == INT_MIN && s.range_maximum_ == INT_MAX);
    reset(s); s.set_range(INT_MAX, INT_MIN);
    assert(s.range_reversed_ == 1 && s.range_minimum_ == INT_MIN && s.range_maximum_ == INT_MAX);
}
static void test_style_setters() {
    Scroll s; reset(s); callback_owner = &s;
    s.flat_button_left_.hook = mutate_from_left;
    s.set_button_color(-1);
    assert(s.color_ == 0xDEADBEEFu && s.flat_button_left_.color_ == 0xFFFFFFFFu &&
           s.flat_button_right_.color_ == 0xFFFFFFFFu);
    assert(event_count == 2 && event_ids[0] == 1 && event_ids[1] == 2);
    assert(event_values[0][0] == 0xFFFFFFFFu && event_values[1][0] == 0xDEADBEEFu);
    assert(event_values[0][4] == 0xFFFFFFFFu && event_values[0][7] == 0);
    assert(event_values[1][4] == 0xFFFFFFFFu && event_values[1][7] == 0);

    reset(s); s.set_bevel_thickness(INT_MIN);
    assert(s.bevel_thickness_ == 0x80000000u &&
           s.flat_button_left_.bevel_thickness_ == 0x80000000u &&
           s.flat_button_right_.bevel_thickness_ == 0x80000000u);
    assert(event_count == 2 && event_ids[0] == 1 && event_ids[1] == 2);
    assert(event_values[0][1] == 0x80000000u && event_values[1][1] == 0x80000000u);
    assert(event_values[0][5] == 0x80000000u && event_values[1][5] == 0x80000000u);

    reset(s); s.set_bevel_upper(-2147483647 - 1);
    assert(s.bevel_upper_ == 0x80000000u && s.flat_button_left_.bevel_upper_ == 0x80000000u &&
           s.flat_button_right_.bevel_upper_ == 0x80000000u);
    assert(event_count == 2 && event_ids[0] == 1 && event_ids[1] == 2);
    assert(event_values[0][2] == 0x80000000u && event_values[1][2] == 0x80000000u);
    assert(event_values[0][6] == 0x80000000u && event_values[1][6] == 0x80000000u);

    reset(s); s.set_bevel_lower(-1);
    assert(s.bevel_lower_ == 0xFFFFFFFFu && s.flat_button_left_.bevel_lower_ == 0xFFFFFFFFu &&
           s.flat_button_right_.bevel_lower_ == 0xFFFFFFFFu);
    assert(event_count == 2 && event_ids[0] == 1 && event_ids[1] == 2);
    assert(event_values[0][3] == 0xFFFFFFFFu && event_values[1][3] == 0xFFFFFFFFu);
    assert(event_values[0][7] == 0xFFFFFFFFu && event_values[1][7] == 0xFFFFFFFFu);
}
int main() { test_ranges(); test_style_setters(); puts("scroll actual-source controls PASS"); }
'''


def extract_body(source: str, signature: str) -> str:
    assert source.count(signature) == 1, f"expected one actual definition: {signature}"
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
    raise AssertionError(f"unbalanced body for {signature}")


def test_source_apis_and_original_identities():
    header = HEADER.read_text()
    source = CPP.read_text()
    for name, (address, signature) in FUNCTIONS.items():
        rows = [row for row in read_file(CPP) if row.address == address]
        assert len(rows) == 1 and rows[0].mode.value == "body"
        assert signature in source
        assert f"void {name}(" in header
        body = extract_body(source, signature)
        assert "return" not in body
    assert "#pragma warning(disable : 4716)" not in source


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_actual_scroll_bodies_with_return_warnings_as_errors(compiler: str) -> None:
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(f"{compiler} is unavailable")
    source = CPP.read_text()
    bodies = "\n\n".join(extract_body(source, signature)
                            for _, signature in FUNCTIONS.values())
    general = GENERAL.read_text()
    swap_body = extract_body(general, SWAP_SIGNATURE)
    range_swap_body = extract_body(source, RANGE_SWAP_SIGNATURE)
    with tempfile.TemporaryDirectory(prefix="opensmacx-scroll-mutators-") as directory:
        root = Path(directory)
        cpp = root / "scroll.cpp"
        binary = root / "scroll"
        cpp.write_text(FIXTURE + swap_body + "\n" + range_swap_body +
                       "\n" + bodies + SUFFIX)
        result = subprocess.run(
            [executable, "-std=c++17", "-Wall", "-Wextra", "-Werror=return-type",
             str(cpp), "-o", str(binary)],
            text=True, capture_output=True, check=False)
        assert result.returncode == 0, result.stdout + result.stderr
        run = subprocess.run([str(binary)], text=True, capture_output=True, check=False)
        assert run.returncode == 0, run.stdout + run.stderr
        assert "scroll actual-source controls PASS" in run.stdout
