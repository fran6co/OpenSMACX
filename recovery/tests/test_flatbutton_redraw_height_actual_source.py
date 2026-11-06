"""Host regression control for FlatButton's original signed DIB-height reads.

The actual complete on_redraw body is compiled with narrow service doubles.
The control checks pinned draw-coordinate formulas; it does not claim native
layout, VC6 code generation, or application acceptance.
"""
from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/flatbutton.cpp").is_file())
SOURCE = ROOT / "src/flatbutton.cpp"
SIGNATURE = "void FlatButton::on_redraw()"
RAW_HEIGHT = "buffer->dib_.bmiHeader.biHeight"


def _actual_body():
    text = SOURCE.read_text(encoding="utf-8")
    start = text.index(SIGNATURE)
    opening = text.index("{", start + len(SIGNATURE))
    depth = 0
    for end in range(opening, len(text)):
        if text[end] == "{":
            depth += 1
        elif text[end] == "}":
            depth -= 1
            if depth == 0:
                return text[start:end + 1]
    raise AssertionError("unterminated FlatButton::on_redraw body")


HARNESS = r'''
#include <cassert>
#include <cstdint>
#include <cstring>
#include <vector>
using std::uint32_t;

struct RECT { int left, top, right, bottom; };
struct DibHeader { int biWidth, biHeight; };
struct Dib { DibHeader bmiHeader; };
class Sprite;
class FlatButton;
struct DrawCall { Sprite *sprite; int x, y; };
struct TileCall { Sprite *sprite; int x, y, width, height; };
static std::vector<DrawCall> draws;
static std::vector<TileCall> tiles;

class Sprite {
public:
    int width_value;
    int height_value;
    int key;
    Sprite(int width, int height, int transparent)
        : width_value(width), height_value(height), key(transparent) {}
    int width() const { return width_value; }
    int height() const { return height_value; }
    int transparent_index() const { return key; }
    int draw(class Buffer *, int, int x, int y, int, int) {
        draws.push_back(DrawCall{this, x, y});
        return 0;
    }
};

class Buffer {
public:
    Dib dib_;
    RECT clip_;
    Buffer() { dib_.bmiHeader.biWidth = 100; dib_.bmiHeader.biHeight = -60;
               clip_ = RECT{0, 0, 100, 80}; }
    void sync_to_palette(int) {}
    RECT current_clip() const { return clip_; }
    int pixel_width() const { return dib_.bmiHeader.biWidth; }
    int pixel_height() const { return -dib_.bmiHeader.biHeight; }
    int fill(int) { return 0; }
    int box_sprite(RECT *, Sprite *) { return 0; }
    int box(RECT *, int, int) { return 0; }
    int tile(Sprite *sprite, int, int, int x, int y, int width, int height) {
        tiles.push_back(TileCall{sprite, x, y, width, height});
        return 0;
    }
    int set_text_color(int, int, int, int) { return 0; }
    int wrap_height(const char *, int) const { return 0; }
    int text_line_height() const { return 0; }
    int text_height() const { return 0; }
    int text_width(const char *) const { return 0; }
    void wrap_cent(const char *, int, int, int) {}
    void set_clip(RECT *) {}
    int write_l(const char *, RECT *, int) { return 0; }
    void restore_clip() {}
};

struct Win {
    bool has_dialog_focus_flag() const { return false; }
    int on_button_text_draw(FlatButton *, char *, int) { return 0; }
};

int HandleMain = 1;
int PaletteActive = 0;
int WinFillColour = 0;
Win *WinCallbackWindow = 0;
static Win parent;

class FlatButton : public Buffer {
public:
    uint32_t field_AB8_ = 0;
    uint32_t field_A9C_ = 0;
    uint32_t field_A18_ = 0;
    uint32_t field_A74_ = 0;
    uint32_t field_A94_ = static_cast<uint32_t>(-1);
    uint32_t field_A78_ = 0;
    uint32_t field_ABC_[3] = {1, 1, 1};
    Sprite *field_AD4_[3] = {0, 0, 0};
    Sprite *border_styles_[3] = {0, 0, 0};
    Sprite *config[27] = {};
    uint32_t bevel_upper_ = 0;
    uint32_t bevel_lower_ = 0;
    uint32_t bevel_thickness_ = 0;
    char *name_ = 0;
    int field_18_ = 0;
    Win *parent_window() { return &parent; }
    int is_dialog_focus() const { return 0; }
    void on_redraw();
};

// ACTUAL_ON_REDRAW

int main() {
    Sprite fill_sprite(13, 11, 7);
    FlatButton centered;
    centered.field_AD4_[0] = &fill_sprite;
    centered.on_redraw();
    assert(draws.size() == 1);
    assert(draws[0].sprite == &fill_sprite);
    assert(draws[0].x == 43);
    // Pinned form: -((11 + raw -60) / 2) == 24 (C++ truncates toward zero).
    assert(draws[0].y == 24);

    draws.clear();
    tiles.clear();
    Sprite center(31, 17, 1), right(11, 13, 2), bottom(15, 9, 3);
    Sprite bottom_right(8, 5, 4), top_left_tile(4, 3, 5);
    Sprite bottom_left_tile(6, 8, 6), top_right_tile(5, 4, 7);
    Sprite bottom_right_mid_tile(2, 6, 8), full_center_tile(40, 20, 9);
    FlatButton frame;
    frame.config[0] = &top_left_tile;
    frame.config[3] = &bottom_left_tile;
    frame.config[6] = &top_right_tile;
    frame.config[9] = &bottom_right_mid_tile;
    frame.config[12] = &center;
    frame.config[15] = &right;
    frame.config[18] = &bottom;
    frame.config[21] = &bottom_right;
    frame.config[24] = &full_center_tile;
    frame.on_redraw();

    assert(draws.size() == 4);
    assert(draws[0].sprite == &center && draws[0].x == 0 && draws[0].y == 0);
    assert(draws[1].sprite == &right && draws[1].x == 89 && draws[1].y == 0);
    assert(draws[2].sprite == &bottom && draws[2].x == 0 && draws[2].y == 51);
    assert(draws[3].sprite == &bottom_right && draws[3].x == 92 && draws[3].y == 55);

    assert(tiles.size() == 5);
    assert(tiles[0].sprite == &top_left_tile && tiles[0].x == 0 && tiles[0].y == 17
           && tiles[0].width == 4 && tiles[0].height == 34);
    assert(tiles[1].sprite == &bottom_left_tile && tiles[1].x == 94 && tiles[1].y == 13
           && tiles[1].width == 6 && tiles[1].height == 42);
    assert(tiles[2].sprite == &top_right_tile && tiles[2].x == 31 && tiles[2].y == 0
           && tiles[2].width == 58 && tiles[2].height == 4);
    assert(tiles[3].sprite == &bottom_right_mid_tile && tiles[3].x == 15 && tiles[3].y == 54
           && tiles[3].width == 77 && tiles[3].height == 6);
    assert(tiles[4].sprite == &full_center_tile && tiles[4].x == 6 && tiles[4].y == 4
           && tiles[4].width == 90 && tiles[4].height == 50);
    return 0;
}
'''


def _run(tmp_path, body):
    compiler = shutil.which("clang++") or shutil.which("g++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    assert HARNESS.count("// ACTUAL_ON_REDRAW") == 1
    source = tmp_path / "flatbutton_redraw_height.cpp"
    binary = tmp_path / "flatbutton_redraw_height"
    source.write_text(HARNESS.replace("// ACTUAL_ON_REDRAW", body), encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", str(source), "-o", str(binary)],
        text=True, capture_output=True, timeout=30)
    assert built.returncode == 0, built.stderr
    return subprocess.run([str(binary)], text=True, capture_output=True, timeout=10)


def test_actual_redraw_uses_pinned_signed_raw_dib_height(tmp_path):
    body = _actual_body()
    assert body.count(RAW_HEIGHT) == 7
    result = _run(tmp_path, body)
    assert result.returncode == 0, result.stderr


def test_positive_pixel_height_substitution_breaks_pinned_draw_coordinates(tmp_path):
    body = _actual_body()
    assert body.count(RAW_HEIGHT) == 7
    damaged = body.replace(RAW_HEIGHT, "buffer->pixel_height()")
    result = _run(tmp_path, damaged)
    assert result.returncode != 0, "raw-height regression escaped the coordinate control"
