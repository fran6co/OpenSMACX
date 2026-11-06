"""Host geometry control for PushButton's raw signed DIB-height operands.

The fixture compiles the complete mapped PushButton::on_redraw body and checks
the pinned nine-sprite draw/tile relationships for both sprite groups. It is a
behavior control, not VC6 or linked-application evidence.
"""
from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/pushbutton.cpp").is_file())
SOURCE = ROOT / "src/pushbutton.cpp"
SIGNATURE = "void PushButton::on_redraw()"
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
    raise AssertionError("unterminated PushButton::on_redraw body")


HARNESS = r'''
#include <cassert>
#include <cstring>
#include <vector>
typedef unsigned int uint32_t;
typedef int LONG;

struct RECT { int left, top, right, bottom; };
struct DibHeader { int biWidth; int biHeight; };
struct Dib { DibHeader bmiHeader; };
class Buffer;

struct Event {
    bool tile;
    class Sprite *sprite;
    int x, y, width, height;
};
static std::vector<Event> events;

class Sprite {
public:
    int w, h, transparent;
    Sprite() : w(1), h(1), transparent(0) {}
    int width() const { return w; }
    int height() const { return h; }
    int transparent_index() const { return transparent; }
    int draw(Buffer *, int, int x, int y, int, int) {
        events.push_back(Event{false, this, x, y, 0, 0});
        return 0;
    }
};

class Buffer {
public:
    Dib dib_;
    RECT rect_;
    Buffer() {
        dib_.bmiHeader.biWidth = 100;
        dib_.bmiHeader.biHeight = -60;
        rect_ = RECT{0, 0, 100, 60};
    }
    void sync_to_palette(int) {}
    int pixel_width() const { return dib_.bmiHeader.biWidth; }
    int pixel_height() const { return -dib_.bmiHeader.biHeight; }
    int box(RECT *, int, int) { return 0; }
    int tile(Sprite *sprite, int, int, int x, int y, int width, int height) {
        events.push_back(Event{true, sprite, x, y, width, height});
        return 0;
    }
    int write_cent_l(char *, RECT *, unsigned int) { return 0; }
};

class Win {
public:
    bool has_dialog_focus_flag() const { return false; }
};
static Win parent;
static Win *WinCallbackWindow = 0;
static int HandleMain = 1;
static int PaletteActive = 0;

class PushButton : public Buffer {
public:
    Sprite *field_AB8_, *field_ABC_, *field_AC0_, *field_AC4_, *field_AC8_, *field_ACC_;
    Sprite *field_AD0_, *field_AD4_, *field_AD8_, *field_ADC_, *field_AE0_, *field_AE4_;
    Sprite *field_AE8_, *field_AEC_, *field_AF0_, *field_AF4_, *field_AF8_, *field_AFC_;
    uint32_t field_A94_, field_A9C_, color_, bevel_upper_, bevel_lower_;
    uint32_t bevel_thickness_;
    char *name_;
    PushButton() : field_AB8_(0), field_ABC_(0), field_AC0_(0), field_AC4_(0),
        field_AC8_(0), field_ACC_(0), field_AD0_(0), field_AD4_(0), field_AD8_(0),
        field_ADC_(0), field_AE0_(0), field_AE4_(0), field_AE8_(0), field_AEC_(0),
        field_AF0_(0), field_AF4_(0), field_AF8_(0), field_AFC_(0),
        field_A94_(0xffffffffU), field_A9C_(0), color_(0), bevel_upper_(0),
        bevel_lower_(0), bevel_thickness_(0), name_(0) {}
    Win *parent_window() { return &parent; }
    int is_dialog_focus() const { return 0; }
    RECT *buffer_working_rect() { return &rect_; }
    int fill(int) { return 0; }
    void on_redraw();
};

// ACTUAL_ON_REDRAW

static void expect_event(size_t at, bool tile, Sprite *sprite,
                         int x, int y, int width, int height) {
    assert(at < events.size());
    const Event &event = events[at];
    assert(event.tile == tile && event.sprite == sprite);
    assert(event.x == x && event.y == y);
    assert(event.width == width && event.height == height);
}

static void set_sprite(Sprite &sprite, int width, int height, int key) {
    sprite.w = width;
    sprite.h = height;
    sprite.transparent = key;
}

static void attach_group(PushButton &button, Sprite *s, bool pressed) {
    if (!pressed) {
        button.field_AB8_ = &s[0];
        button.field_ABC_ = &s[1];
        button.field_AC0_ = &s[2];
        button.field_AC4_ = &s[3];
        button.field_AC8_ = &s[4];
        button.field_ACC_ = &s[5];
        button.field_AD0_ = &s[6];
        button.field_AD4_ = &s[7];
        button.field_AD8_ = &s[8];
    } else {
        button.field_ADC_ = &s[0];
        button.field_AE0_ = &s[1];
        button.field_AE4_ = &s[2];
        button.field_AE8_ = &s[3];
        button.field_AEC_ = &s[4];
        button.field_AF0_ = &s[5];
        button.field_AF4_ = &s[6];
        button.field_AF8_ = &s[7];
        button.field_AFC_ = &s[8];
        // AC8 is also the frame-enabled sentinel in both states.
        button.field_AC8_ = &s[4];
    }
    button.field_A9C_ = pressed ? 1U : 0U;
    button.bevel_thickness_ = 0;
}

static void check_default_100_60() {
    Sprite s[9];
    set_sprite(s[0], 4, 3, 1);   // top edge
    set_sprite(s[1], 5, 4, 2);   // bottom edge
    set_sprite(s[2], 6, 8, 3);   // left edge
    set_sprite(s[3], 2, 6, 4);   // right edge
    set_sprite(s[4], 13, 11, 5); // top-left
    set_sprite(s[5], 11, 13, 6); // top-right
    set_sprite(s[6], 15, 9, 7);  // bottom-left
    set_sprite(s[7], 8, 5, 8);   // bottom-right
    set_sprite(s[8], 40, 20, 9); // center
    PushButton button;
    button.dib_.bmiHeader.biWidth = 100;
    button.dib_.bmiHeader.biHeight = -60;
    attach_group(button, s, false);
    events.clear();
    button.on_redraw();
    assert(events.size() == 9);
    expect_event(0, false, &s[4], 0, 0, 0, 0);
    expect_event(1, false, &s[5], 89, 0, 0, 0);
    expect_event(2, false, &s[6], 0, 51, 0, 0);
    expect_event(3, false, &s[7], 92, 55, 0, 0);
    expect_event(4, true, &s[0], 0, 11, 4, 40);
    expect_event(5, true, &s[1], 95, 13, 5, 42);
    expect_event(6, true, &s[2], 13, 0, 76, 8);
    expect_event(7, true, &s[3], 15, 54, 77, 6);
    expect_event(8, true, &s[8], 5, 8, 91, 46);
}

static void check_pressed_100_60() {
    Sprite s[9];
    set_sprite(s[0], 3, 5, 11);  // top edge
    set_sprite(s[1], 6, 4, 12);  // bottom edge
    set_sprite(s[2], 5, 9, 13);  // left edge
    set_sprite(s[3], 8, 11, 14); // right edge
    set_sprite(s[4], 9, 7, 15);  // top-left
    set_sprite(s[5], 12, 10, 16);// top-right
    set_sprite(s[6], 14, 12, 17);// bottom-left
    set_sprite(s[7], 7, 9, 18);  // bottom-right
    set_sprite(s[8], 2, 2, 19);  // center
    PushButton button;
    button.dib_.bmiHeader.biWidth = 100;
    button.dib_.bmiHeader.biHeight = -60;
    attach_group(button, s, true);
    events.clear();
    button.on_redraw();
    assert(events.size() == 9);
    expect_event(0, false, &s[4], 0, 0, 0, 0);
    expect_event(1, false, &s[5], 88, 0, 0, 0);
    expect_event(2, false, &s[6], 0, 48, 0, 0);
    expect_event(3, false, &s[7], 93, 51, 0, 0);
    expect_event(4, true, &s[0], 0, 7, 3, 41);
    expect_event(5, true, &s[1], 94, 10, 6, 41);
    expect_event(6, true, &s[2], 9, 0, 79, 9);
    expect_event(7, true, &s[3], 14, 49, 79, 11);
    expect_event(8, true, &s[8], 6, 9, 91, 40);
}

static void check_default_137_83() {
    Sprite s[9];
    set_sprite(s[0], 3, 5, 1);
    set_sprite(s[1], 6, 4, 2);
    set_sprite(s[2], 5, 9, 3);
    set_sprite(s[3], 8, 11, 4);
    set_sprite(s[4], 9, 7, 5);
    set_sprite(s[5], 12, 10, 6);
    set_sprite(s[6], 14, 12, 7);
    set_sprite(s[7], 7, 9, 8);
    set_sprite(s[8], 2, 2, 9);
    PushButton button;
    button.dib_.bmiHeader.biWidth = 137;
    button.dib_.bmiHeader.biHeight = -83;
    attach_group(button, s, false);
    events.clear();
    button.on_redraw();
    assert(events.size() == 9);
    expect_event(0, false, &s[4], 0, 0, 0, 0);
    expect_event(1, false, &s[5], 125, 0, 0, 0);
    expect_event(2, false, &s[6], 0, 71, 0, 0);
    expect_event(3, false, &s[7], 130, 74, 0, 0);
    expect_event(4, true, &s[0], 0, 7, 3, 64);
    expect_event(5, true, &s[1], 131, 10, 6, 64);
    expect_event(6, true, &s[2], 9, 0, 116, 9);
    expect_event(7, true, &s[3], 14, 72, 116, 11);
    expect_event(8, true, &s[8], 6, 9, 128, 63);
}

static void check_pressed_137_83() {
    Sprite s[9];
    set_sprite(s[0], 4, 6, 11);
    set_sprite(s[1], 8, 5, 12);
    set_sprite(s[2], 6, 10, 13);
    set_sprite(s[3], 9, 12, 14);
    set_sprite(s[4], 10, 8, 15);
    set_sprite(s[5], 13, 11, 16);
    set_sprite(s[6], 16, 13, 17);
    set_sprite(s[7], 9, 10, 18);
    set_sprite(s[8], 2, 2, 19);
    PushButton button;
    button.dib_.bmiHeader.biWidth = 137;
    button.dib_.bmiHeader.biHeight = -83;
    attach_group(button, s, true);
    events.clear();
    button.on_redraw();
    assert(events.size() == 9);
    expect_event(0, false, &s[4], 0, 0, 0, 0);
    expect_event(1, false, &s[5], 124, 0, 0, 0);
    expect_event(2, false, &s[6], 0, 70, 0, 0);
    expect_event(3, false, &s[7], 128, 73, 0, 0);
    expect_event(4, true, &s[0], 0, 8, 4, 62);
    expect_event(5, true, &s[1], 129, 11, 8, 62);
    expect_event(6, true, &s[2], 10, 0, 114, 10);
    expect_event(7, true, &s[3], 16, 71, 112, 12);
    expect_event(8, true, &s[8], 8, 10, 125, 61);
}

int main() {
    check_default_100_60();
    check_pressed_100_60();
    check_default_137_83();
    check_pressed_137_83();
    return 0;
}
'''


def _run(tmp_path, body):
    compiler = shutil.which("clang++") or shutil.which("g++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    assert HARNESS.count("// ACTUAL_ON_REDRAW") == 1
    source = tmp_path / "pushbutton_redraw_height.cpp"
    binary = tmp_path / "pushbutton_redraw_height"
    source.write_text(HARNESS.replace("// ACTUAL_ON_REDRAW", body), encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
         str(source), "-o", str(binary)], text=True, capture_output=True, timeout=30)
    assert built.returncode == 0, built.stderr
    return subprocess.run([str(binary)], text=True, capture_output=True, timeout=10)


def test_actual_pushbutton_redraw_uses_raw_height_for_both_sprite_groups(tmp_path):
    body = _actual_body()
    assert body.count(RAW_HEIGHT) == 6
    result = _run(tmp_path, body)
    assert result.returncode == 0, result.stderr


def test_positive_pixel_height_mutant_breaks_original_raw_coordinate_control(tmp_path):
    body = _actual_body()
    assert body.count(RAW_HEIGHT) == 6
    damaged = body.replace(RAW_HEIGHT, "buffer->pixel_height()")
    result = _run(tmp_path, damaged)
    assert result.returncode != 0, "positive pixel_height substitution escaped geometry control"
