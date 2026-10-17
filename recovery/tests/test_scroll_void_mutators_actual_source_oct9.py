"""Behavior controls that compile the current eight Scroll mutator bodies."""
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
GENERAL_CPP = ROOT / "src/general.cpp"
SWAP_SIGNATURE = "void __cdecl swap(int *var1, int *var2)"
FUNCTIONS = {
    "set_range": (0x006059B0, "void Scroll::set_range(int minimum, int maximum)"),
    "set_button_color": (0x00605A10, "void Scroll::set_button_color(int color)"),
    "set_bevel_thickness": (0x00605A50, "void Scroll::set_bevel_thickness(int thickness)"),
    "set_bevel_upper": (0x00605A90, "void Scroll::set_bevel_upper(int color)"),
    "set_bevel_lower": (0x00605AD0, "void Scroll::set_bevel_lower(int color)"),
    "set_bar_thickness": (0x00605B80, "void Scroll::set_bar_thickness(int thickness)"),
    "set_border_color": (0x00605B10, "void Scroll::set_border_color(int color)"),
    "set_thumb_rect": (0x00606EA0, "void Scroll::set_thumb_rect()"),
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
typedef int32_t LONG;
struct RECT { LONG left, top, right, bottom; };
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
    int bar_thickness_, border_color_;
    RECT thumb_rect_;
    FlatButton flat_button_left_, flat_button_right_;
    void set_range(int minimum, int maximum);
    void set_button_color(int color);
    void set_bevel_thickness(int thickness);
    void set_bevel_upper(int color);
    void set_bevel_lower(int color);
    void set_bar_thickness(int thickness);
    void set_border_color(int color);
    void set_thumb_rect();
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
    s.bar_thickness_ = s.border_color_ = 0;
    s.thumb_rect_.left = s.thumb_rect_.top = s.thumb_rect_.right = s.thumb_rect_.bottom = 0;
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
static int32_t signed_bits(uint32_t value) {
    int32_t result; memcpy(&result, &value, sizeof(result)); return result;
}
static void expect_rect(const Scroll &s, int thickness, int border) {
    const uint32_t t = static_cast<uint32_t>(thickness);
    const int32_t edge = signed_bits(border == -1 ? t : t - 1U);
    const int32_t inset = border == -1 ? 0 : 1;
    assert(s.thumb_rect_.left == inset && s.thumb_rect_.top == inset);
    assert(s.thumb_rect_.right == edge && s.thumb_rect_.bottom == edge);
}
static void test_geometry_setters() {
    const int thicknesses[] = {0, 1, -1, INT_MIN, INT_MAX};
    const int borders[] = {-1, 0, 7};
    RedrawSpy s;
    for (unsigned ti = 0; ti < sizeof(thicknesses) / sizeof(thicknesses[0]); ++ti) {
        const int thickness = thicknesses[ti];
        for (unsigned bi = 0; bi < sizeof(borders) / sizeof(borders[0]); ++bi) {
            const int border = borders[bi];
            reset(s);
            s.range_reversed_ = 9; s.range_minimum_ = 10; s.position_ = 11; s.range_maximum_ = 12;
            s.color_ = 13; s.bevel_thickness_ = 14; s.bevel_upper_ = 15; s.bevel_lower_ = 16;
            s.border_color_ = border;
            s.set_bar_thickness(thickness);
            assert(s.bar_thickness_ == thickness);
            expect_rect(s, thickness, border);
            assert(s.range_reversed_ == 9 && s.range_minimum_ == 10 && s.position_ == 11 &&
                   s.range_maximum_ == 12 && s.color_ == 13 && s.bevel_thickness_ == 14 &&
                   s.bevel_upper_ == 15 && s.bevel_lower_ == 16);
            assert(event_count == 0 && s.overrides == 0);

            reset(s);
            s.range_reversed_ = 9; s.range_minimum_ = 10; s.position_ = 11; s.range_maximum_ = 12;
            s.color_ = 13; s.bevel_thickness_ = 14; s.bevel_upper_ = 15; s.bevel_lower_ = 16;
            s.bar_thickness_ = thickness; s.border_color_ = -1;
            s.set_border_color(border);
            assert(s.border_color_ == border);
            expect_rect(s, thickness, border);
            assert(s.range_reversed_ == 9 && s.range_minimum_ == 10 && s.position_ == 11 &&
                   s.range_maximum_ == 12 && s.color_ == 13 && s.bevel_thickness_ == 14 &&
                   s.bevel_upper_ == 15 && s.bevel_lower_ == 16);
            assert(event_count == 0 && s.overrides == 0);

            reset(s);
            s.range_reversed_ = 9; s.range_minimum_ = 10; s.position_ = 11; s.range_maximum_ = 12;
            s.color_ = 13; s.bevel_thickness_ = 14; s.bevel_upper_ = 15; s.bevel_lower_ = 16;
            s.bar_thickness_ = thickness; s.border_color_ = border;
            s.set_thumb_rect();
            expect_rect(s, thickness, border);
            assert(s.range_reversed_ == 9 && s.range_minimum_ == 10 && s.position_ == 11 &&
                   s.range_maximum_ == 12 && s.color_ == 13 && s.bevel_thickness_ == 14 &&
                   s.bevel_upper_ == 15 && s.bevel_lower_ == 16 && s.border_color_ == border);
            assert(event_count == 0 && s.overrides == 0);
        }
    }
}
int main() { test_ranges(); test_style_setters(); test_geometry_setters(); puts("scroll actual-source controls PASS"); }
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
    bits_helper = extract_body(source, "inline LONG long_from_bits(uint32_t bits)")
    swap_body = extract_body(GENERAL_CPP.read_text(), SWAP_SIGNATURE)
    with tempfile.TemporaryDirectory(prefix="opensmacx-scroll-mutators-") as directory:
        root = Path(directory)
        cpp = root / "scroll.cpp"
        binary = root / "scroll"
        cpp.write_text(FIXTURE + swap_body + "\n" + bits_helper + "\n" + bodies + SUFFIX)
        result = subprocess.run(
            [executable, "-std=c++17", "-Wall", "-Wextra", "-Werror=return-type",
             str(cpp), "-o", str(binary)],
            text=True, capture_output=True, check=False)
        assert result.returncode == 0, result.stdout + result.stderr
        run = subprocess.run([str(binary)], text=True, capture_output=True, check=False)
        assert run.returncode == 0, run.stdout + run.stderr
        assert "scroll actual-source controls PASS" in run.stdout


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
@pytest.mark.parametrize("mutation", ["wrong_guard", "wrong_slot", "missing_update", "wrong_arithmetic"])
def test_geometry_mutants_compile_but_fail_runtime(compiler: str, mutation: str) -> None:
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(f"{compiler} is unavailable")
    source = CPP.read_text()
    bodies = {name: extract_body(source, signature) for name, (_, signature) in FUNCTIONS.items()}
    if mutation == "wrong_guard":
        target = "if (border_color_ != -1)"
        bodies["set_border_color"] = bodies["set_border_color"].replace(
            target, "if (border_color_ == -1)", 1)
        assert target in extract_body(source, FUNCTIONS["set_border_color"][1])
    elif mutation == "wrong_slot":
        target = "thumb_rect_.right = long_from_bits(thickness);"
        # A duplicate bottom store where the right store belongs leaves right
        # at its zero-init value and is a compiled, behaviorally distinct mutant.
        assert target in bodies["set_thumb_rect"]
        bodies["set_thumb_rect"] = bodies["set_thumb_rect"].replace(
            target, "thumb_rect_.bottom = long_from_bits(thickness);", 1)
    elif mutation == "missing_update":
        target = "    bar_thickness_ = thickness;\n"
        assert target in bodies["set_bar_thickness"]
        bodies["set_bar_thickness"] = bodies["set_bar_thickness"].replace(target, "", 1)
    else:
        target = "static_cast<uint32_t>(thumb_rect_.right) - 1U"
        assert target in bodies["set_thumb_rect"]
        bodies["set_thumb_rect"] = bodies["set_thumb_rect"].replace(
            target, "static_cast<uint32_t>(thumb_rect_.right) + 1U", 1)

    ordered_bodies = "\n\n".join(bodies[name] for name in FUNCTIONS)
    bits_helper = extract_body(source, "inline LONG long_from_bits(uint32_t bits)")
    swap_body = extract_body(GENERAL_CPP.read_text(), SWAP_SIGNATURE)
    with tempfile.TemporaryDirectory(prefix="opensmacx-scroll-mutant-") as directory:
        root = Path(directory)
        cpp, binary = root / "scroll.cpp", root / "scroll"
        cpp.write_text(FIXTURE + swap_body + "\n" + bits_helper + "\n" +
                       ordered_bodies + SUFFIX)
        build = subprocess.run(
            [executable, "-std=c++17", "-Wall", "-Wextra", "-Werror=return-type",
             str(cpp), "-o", str(binary)], text=True, capture_output=True, check=False)
        assert build.returncode == 0, build.stdout + build.stderr
        run = subprocess.run([str(binary)], text=True, capture_output=True, check=False)
        assert run.returncode != 0, f"{mutation} mutant unexpectedly passed all runtime controls"
