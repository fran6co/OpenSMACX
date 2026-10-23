"""Exercise Buffer's four parameter rows and all four text-slot columns.

The complete current write_raw_l and four current column setters are compiled
on the host. GDI, HDC acquisition, and Font width are controlled services;
this does not model native layout, GDI behavior, or VC6 output.
"""
from __future__ import annotations

from pathlib import Path
import shutil
import subprocess

import pytest

from test_buffer_line_layout_fidelity_oct12 import body


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/buffer.cpp"
HEADER = ROOT / "src/buffer.h"


FIXTURE = r"""
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <string>
#include <vector>

#define __cdecl
typedef char *LPSTR;
typedef uint32_t COLORREF;
typedef int HDC;
typedef int HGDIOBJ;
static const int TRANSPARENT = 1;
static const int SYSTEM_FONT = 2;

struct RECT { int left, top, right, bottom; };
struct Draw { COLORREF color; int x, y, length; std::string text; };
static COLORREF active_color;
static std::vector<COLORREF> selected_colors;
static std::vector<HGDIOBJ> selected_objects;
static std::vector<Draw> draw_calls;
static std::vector<uint32_t> converted_colors;
static std::function<void(class Buffer &)> hdc_action;

static int SetBkMode(HDC, int) { return 0; }
static HGDIOBJ SelectObject(HDC, HGDIOBJ object) {
    selected_objects.push_back(object);
    return 0;
}
static COLORREF SetTextColor(HDC, COLORREF color) {
    selected_colors.push_back(color);
    active_color = color;
    return color;
}
static int TextOutA(HDC, int x, int y, const char *text, int length) {
    draw_calls.push_back({active_color, x, y, length,
                          std::string(text, static_cast<size_t>(length))});
    return 1;
}
static HGDIOBJ GetStockObject(int) { return 0x7f; }

struct Font {
    HGDIOBJ font_obj_;
    int height_;
    int width_calls;
    int width(char *, int length) { ++width_calls; return length; }
};
struct SpecialText { uint32_t color_; RECT bounds[2]; };
struct SpecialTextOwner {
    SpecialText value = {};
    SpecialText &at(int) { return value; }
};
static SpecialTextOwner BufferSpecialTextsOwner;

typedef int (__cdecl *BufferWriteRawCallback)(LPSTR, int, int, int);
class Buffer {
public:
    Font *fonts_[4];
    uint32_t text_parameters_[4][4];
    BufferWriteRawCallback field_8_;
    uint32_t markup_enabled_;
    uint32_t write_font_slot_;
    uint32_t field_524_;
    uint32_t field_528_;
    HDC hdc2_;
    int release_count_;

    Buffer() : fonts_{}, text_parameters_{}, field_8_(nullptr),
        markup_enabled_(1), write_font_slot_(0), field_524_(0), field_528_(0),
        hdc2_(1), release_count_(0) {}

    HDC get_hdc() {
        if (hdc_action) hdc_action(*this);
        return hdc2_;
    }
    void release_hdc(int count) { release_count_ += count; }
    COLORREF text_color_to_gdi(uint32_t color) const {
        converted_colors.push_back(color);
        return color;
    }
    int write_raw_l(char *, int, int, int);
    void set_text_color(int, int, int, int);
    void set_text_color2(int, int, int, int);
    void set_text_color3(int, int, int, int);
    void set_text_color_hyper(int, int, int, int);
};

/* ACTUAL_WRITE_RAW_L */
/* ACTUAL_SET_COLOR_0 */
/* ACTUAL_SET_COLOR_1 */
/* ACTUAL_SET_COLOR_2 */
/* ACTUAL_SET_COLOR_3 */

static void reset_observations() {
    selected_colors.clear();
    selected_objects.clear();
    draw_calls.clear();
    converted_colors.clear();
    active_color = 0;
}

int main() {
    Buffer buffer;
    Font fonts[4] = {{10, 5, 0}, {11, 11, 0}, {12, 8, 0}, {13, 30, 0}};
    for (int slot = 0; slot != 4; ++slot) buffer.fonts_[slot] = &fonts[slot];

    const int foreground[4] = {0x101, 0x202, 0x303, 0x404};
    const int shadow[4] = {0x111, 0x222, 0x333, 0x444};
    const int shadow_x[4] = {-7, -17, -27, -37};
    const int shadow_y[4] = {9, -19, 29, -39};
    buffer.set_text_color(foreground[0], shadow[0], shadow_x[0], shadow_y[0]);
    buffer.set_text_color2(foreground[1], shadow[1], shadow_x[1], shadow_y[1]);
    buffer.set_text_color3(foreground[2], shadow[2], shadow_x[2], shadow_y[2]);
    buffer.set_text_color_hyper(foreground[3], shadow[3], shadow_x[3], shadow_y[3]);

    for (int slot = 0; slot != 4; ++slot) {
        assert(buffer.text_parameters_[0][slot] ==
               static_cast<uint32_t>(foreground[slot]));
        assert(buffer.text_parameters_[1][slot] ==
               static_cast<uint32_t>(shadow[slot]));
        assert(buffer.text_parameters_[2][slot] ==
               static_cast<uint32_t>(shadow_x[slot]));
        assert(buffer.text_parameters_[3][slot] ==
               static_cast<uint32_t>(shadow_y[slot]));
    }

    char text[] = "abc";
    for (int slot = 0; slot != 4; ++slot) {
        reset_observations();
        buffer.release_count_ = 0;
        buffer.write_font_slot_ = static_cast<uint32_t>(slot);
        assert(buffer.write_raw_l(text, 50, 20, 3) == 53);
        assert(selected_objects.size() == 2);
        assert(selected_objects[0] == fonts[slot].font_obj_);
        assert(selected_objects[1] == 0x7f);
        assert(selected_colors == std::vector<COLORREF>({
            static_cast<COLORREF>(shadow[slot]),
            static_cast<COLORREF>(foreground[slot])}));
        assert(draw_calls.size() == 2);
        assert(draw_calls[0].color == static_cast<COLORREF>(shadow[slot]));
        assert(draw_calls[0].x == 50 + shadow_x[slot]);
        assert(draw_calls[0].y == 20 + 11 + shadow_y[slot]);
        assert(draw_calls[0].text == "abc" && draw_calls[0].length == 3);
        assert(draw_calls[1].color == static_cast<COLORREF>(foreground[slot]));
        assert(draw_calls[1].x == 50 && draw_calls[1].y == 31);
        assert(draw_calls[1].text == "abc" && draw_calls[1].length == 3);
        assert(converted_colors == std::vector<uint32_t>({
            static_cast<uint32_t>(shadow[slot]),
            static_cast<uint32_t>(foreground[slot])}));
        assert(buffer.release_count_ == 1 && fonts[slot].width_calls == 1);
    }

    // A foreground sentinel keeps the requested font slot but falls back to
    // color column 0, including that column's shadow color and signed offsets.
    buffer.set_text_color_hyper(-1, shadow[3], shadow_x[3], shadow_y[3]);
    reset_observations();
    buffer.write_font_slot_ = 3;
    assert(buffer.text_parameters_[0][3] == UINT32_MAX);
    assert(buffer.write_raw_l(text, 50, 20, 3) == 53);
    assert(selected_objects[0] == fonts[3].font_obj_);
    assert(selected_colors == std::vector<COLORREF>({
        static_cast<COLORREF>(shadow[0]),
        static_cast<COLORREF>(foreground[0])}));
    assert(draw_calls[0].x == 50 + shadow_x[0]);
    assert(draw_calls[0].y == 20 + 11 + shadow_y[0]);
    assert(draw_calls[1].x == 50 && draw_calls[1].y == 31);

    // The shadow sentinel suppresses only the shadow draw; foreground remains.
    buffer.set_text_color_hyper(foreground[3], -1, shadow_x[3], shadow_y[3]);
    reset_observations();
    assert(buffer.write_raw_l(text, 50, 20, 3) == 53);
    assert(draw_calls.size() == 1 && selected_colors.size() == 1);
    assert(draw_calls[0].color == static_cast<COLORREF>(foreground[3]));
    assert(draw_calls[0].x == 50 && draw_calls[0].y == 31);

    // The HDC service changes the selected font and all four parameters after
    // entry. write_raw_l must observe the live values when it selects/draws.
    Font replacement = {99, 13, 0};
    buffer.set_text_color3(foreground[2], shadow[2], shadow_x[2], shadow_y[2]);
    buffer.write_font_slot_ = 2;
    hdc_action = [&](Buffer &live) {
        live.fonts_[2] = &replacement;
        live.text_parameters_[0][2] = 0x7a01;
        live.text_parameters_[1][2] = 0x7a02;
        live.text_parameters_[2][2] = static_cast<uint32_t>(-23);
        live.text_parameters_[3][2] = static_cast<uint32_t>(-29);
    };
    reset_observations();
    assert(buffer.write_raw_l(text, 50, 20, 3) == 53);
    hdc_action = {};
    assert(selected_objects[0] == replacement.font_obj_);
    assert(selected_colors == std::vector<COLORREF>({0x7a02, 0x7a01}));
    assert(draw_calls[0].x == 27 && draw_calls[0].y == 4);
    assert(draw_calls[1].x == 50 && draw_calls[1].y == 33);
    assert(replacement.width_calls == 1);
    return 0;
}
"""


def source_fixture(mutation: str | None = None) -> str:
    source = SOURCE.read_text(encoding="utf-8")
    raw = body(source, "int Buffer::write_raw_l(char *text, int x, int y, int length)")
    setters = [
        body(source, "void Buffer::set_text_color(int color1, int color2, int color3, int color4)"),
        body(source, "void Buffer::set_text_color2(int color1, int color2, int color3, int color4)"),
        body(source, "void Buffer::set_text_color3(int color1, int color2, int color3, int color4)"),
        body(source, "void Buffer::set_text_color_hyper(int color1, int color2, int color3, int color4)"),
    ]
    if mutation == "cached_callback_read":
        acquire = "    if (get_hdc() == 0) return x;"
        assert raw.count(acquire) == 1
        cache = (
            "    uint32_t cached_parameters[4][4];\n"
            "    std::memcpy(cached_parameters, text_parameters_, sizeof(cached_parameters));\n"
        )
        raw = raw.replace(acquire, cache + acquire, 1)
        for row in range(4):
            old = f"text_parameters_[{row}]"
            assert raw.count(old) == 1
            raw = raw.replace(old, f"cached_parameters[{row}]", 1)
    elif mutation == "wrong_row":
        old = "text_parameters_[1][0] = color2;"
        assert old in setters[0]
        setters[0] = setters[0].replace(old, "text_parameters_[2][0] = color2;", 1)
    elif mutation == "wrong_column":
        old = "text_parameters_[3][2] = color4;"
        assert old in setters[2]
        setters[2] = setters[2].replace(old, "text_parameters_[3][1] = color4;", 1)
    elif mutation is not None:
        raise AssertionError(f"unknown mutation: {mutation}")

    pieces = [raw, *setters]
    placeholders = [
        "/* ACTUAL_WRITE_RAW_L */",
        "/* ACTUAL_SET_COLOR_0 */",
        "/* ACTUAL_SET_COLOR_1 */",
        "/* ACTUAL_SET_COLOR_2 */",
        "/* ACTUAL_SET_COLOR_3 */",
    ]
    fixture = FIXTURE
    for placeholder, piece in zip(placeholders, pieces, strict=True):
        assert fixture.count(placeholder) == 1
        fixture = fixture.replace(placeholder, piece, 1)
    return fixture


def compile_run(tmp_path: Path, compiler_name: str, source: str):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    cpp = tmp_path / f"buffer_text_parameters_{compiler_name}.cpp"
    exe = tmp_path / f"buffer_text_parameters_{compiler_name}"
    cpp.write_text(source, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
         "-Wno-unused-parameter", str(cpp), "-o", str(exe)],
        capture_output=True, text=True, timeout=30)
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(exe)], capture_output=True, text=True, timeout=10)


def test_storage_accessor_and_direct_editbox_consumer_follow_foreground_slot():
    header = HEADER.read_text(encoding="utf-8")
    editbox = (ROOT / "src/editbox.cpp").read_text(encoding="utf-8")
    assert "uint32_t text_parameters_[4][4];" in header
    assert "return static_cast<int>(text_parameters_[0][0]);" in header
    assert "text_parameters_[0][0]" in editbox
    assert "color_val_1_" not in header and "color_val_1_" not in editbox


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_write_raw_and_all_four_setters(compiler_name, tmp_path):
    result = compile_run(tmp_path, compiler_name, source_fixture())
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize("mutation", ("wrong_row", "wrong_column", "cached_callback_read"))
def test_compiling_row_column_and_callback_mutants_fail(compiler_name, mutation, tmp_path):
    result = compile_run(tmp_path, compiler_name, source_fixture(mutation))
    assert result.returncode != 0, f"behavior control missed {mutation}"
