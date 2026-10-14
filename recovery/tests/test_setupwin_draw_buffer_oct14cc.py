"""Actual-source ownership controls for SetupWin::draw_item."""
from pathlib import Path
import shutil
import subprocess

import pytest
from match_recovery.source import read_file


ROOT = Path(__file__).resolve().parents[2]
PRODUCT = ROOT / "src" / "setupwin.cpp"
SIGNATURE = "int SetupWin::draw_item(int position, int selected)"


def extract_body(source):
    start = source.find(SIGNATURE)
    assert start >= 0, "mapped SetupWin::draw_item definition missing"
    opening = source.find("{", start)
    depth = 0
    state = "code"
    i = opening
    while i < len(source):
        ch = source[i]
        nxt = source[i + 1] if i + 1 < len(source) else ""
        if state == "code":
            if ch == "/" and nxt == "/":
                state = "line"
                i += 2
                continue
            if ch == "/" and nxt == "*":
                state = "block"
                i += 2
                continue
            if ch == '"':
                state = "string"
            elif ch == "'":
                state = "char"
            elif ch == "{":
                depth += 1
            elif ch == "}":
                depth -= 1
                if depth == 0:
                    return source[start:i + 1]
        elif state == "line":
            if ch == "\n":
                state = "code"
        elif state == "block":
            if ch == "*" and nxt == "/":
                state = "code"
                i += 2
                continue
        else:
            if ch == "\\":
                i += 2
                continue
            if (state == "string" and ch == '"') or (state == "char" and ch == "'"):
                state = "code"
        i += 1
    raise AssertionError("unterminated draw_item body")


def actual_body():
    records = [record for record in read_file(PRODUCT) if record.address == 0x004AE4D0]
    assert len(records) == 1, "draw_item native identity must remain unique"
    assert records[0].state.value == "implemented"
    return extract_body(PRODUCT.read_text(encoding="utf-8"))


FIXTURE = r"""
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>

static void semantic_failure(const char *message) {
    std::fprintf(stderr, "SEMANTIC97: %s\n", message);
    std::exit(97);
}
struct RECT { int left, top, right, bottom; };
enum Role { INHERITED, AUXILIARY, FLIC_SOURCE, OVERLAY, OPENING };
struct Buffer {
    struct { struct { int biWidth, biHeight; } bmiHeader; } dib_;
    Role role;
    bool open;
    bool has_font;
    int clip_calls, color_calls, text_calls, restore_calls, copy_calls, draw_calls;
    int text_color, text_len, last_width, last_height, last_x, last_y;
    int pixel;
    char last_text[64];
    Buffer(Role r = AUXILIARY)
        : role(r), open(true), has_font(false), clip_calls(0), color_calls(0),
          text_calls(0), restore_calls(0), copy_calls(0), draw_calls(0),
          text_color(0), text_len(0), last_width(0), last_height(0),
          last_x(0), last_y(0), pixel(0) {
        dib_.bmiHeader.biWidth = 20;
        dib_.bmiHeader.biHeight = -8;
    }
    void set_clip(RECT *rect) {
        if (role != INHERITED) semantic_failure("clip used auxiliary buffer");
        if (!open) semantic_failure("clip used closed inherited buffer");
        if (rect->top != 51 || rect->bottom != 77)
            semantic_failure("clip rectangle changed");
        ++clip_calls;
    }
    void set_text_color(int color, int, int, int) {
        if (role != INHERITED) semantic_failure("color used auxiliary buffer");
        if (!open || !has_font) semantic_failure("inherited text buffer not initialized");
        ++color_calls;
        text_color = color;
    }
    void copy(Buffer *destination, int, int, int x, int y, int width, int height) {
        if (role == FLIC_SOURCE) {
            if (destination == 0 || destination->role != INHERITED)
                semantic_failure("Flic copy target is not inherited buffer");
            if (!destination->open || !destination->has_font)
                semantic_failure("Flic copy target is not initialized");
        } else if (role == OVERLAY) {
            if (destination == 0 || destination->role != OPENING)
                semantic_failure("overlay copy target changed");
        } else {
            semantic_failure("copy source ownership changed");
        }
        ++copy_calls;
        last_width = width;
        last_height = height;
        last_x = x;
        last_y = y;
    }
    void write_cent_l(char *text, RECT *rect, int length) {
        if (role != INHERITED) semantic_failure("text used auxiliary buffer");
        if (!open || !has_font) semantic_failure("text target is not initialized");
        if (rect->top != 53 || rect->bottom != 75 ||
            rect->left != 112 || rect->right != 304)
            semantic_failure("text rectangle changed");
        ++text_calls;
        text_len = length;
        std::strncpy(last_text, text, sizeof(last_text) - 1);
        last_text[sizeof(last_text) - 1] = 0;
    }
    void restore_clip() {
        if (role != INHERITED) semantic_failure("clip restored on auxiliary buffer");
        if (!open) semantic_failure("clip restored on closed inherited buffer");
        ++restore_calls;
    }
    int get_pixel(int, int) { return pixel; }
    void draw(Buffer *destination, int, int x, int y, int width, int height) {
        if (role != OVERLAY || destination == 0 || destination->role != OPENING)
            semantic_failure("overlay draw ownership changed");
        ++draw_calls; last_x = x; last_y = y;
        last_width = width; last_height = height;
    }
};
struct GraphicWin : Buffer {
    int soft_calls, soft_x, soft_y, soft_width, soft_height;
    explicit GraphicWin(Role role = INHERITED)
        : Buffer(role), soft_calls(0), soft_x(0), soft_y(0),
          soft_width(0), soft_height(0) {}
    void soft_update(int x, int y, int width, int height) {
        ++soft_calls; soft_x = x; soft_y = y;
        soft_width = width; soft_height = height;
    }
};
struct StringPayload { char *text; };
struct StringStructEntry {
    StringStructEntry *next;
    StringPayload *payload;
    StringPayload *string_payload() { return payload; }
};
struct StringList {
    int entry_count_;
    StringStructEntry *head_;
    StringStructEntry *current_;
    int current_position_;
};
struct Spot {
    void get_rect(int position, int, RECT *rect) {
        if (position != 0) semantic_failure("unexpected test row position");
        rect->left = 100; rect->top = 20; rect->right = 300; rect->bottom = 45;
    }
};
static char *CharUpperA(char *text) {
    for (char *p = text; *p; ++p)
        *p = static_cast<char>(std::toupper(static_cast<unsigned char>(*p)));
    return text;
}
static GraphicWin opening_surface(OPENING);
static GraphicWin *opening_window() { return &opening_surface; }
static int WinScreenWidth = 1024;
struct SetupWin : GraphicWin {
    Buffer buffer_;
    Buffer buffer2_;
    Spot spot_;
    unsigned int row_count_;
    StringList *item_texts_;
    Buffer *overlay_buffers_[3];
    int overlay_x_[3], overlay_y_[3];
    SetupWin() : buffer_(AUXILIARY), buffer2_(FLIC_SOURCE), row_count_(1), item_texts_(0) {
        buffer_.open = false;
        has_font = true;
        overlay_buffers_[0] = overlay_buffers_[1] = overlay_buffers_[2] = 0;
        overlay_x_[0] = overlay_y_[0] = 7;
    }
    int draw_item(int position, int selected);
};

// ACTUAL_BODY_INSERTION_POINT

static int run_case(int selected, int transparent_overlay) {
    char first[] = "alpha|beta (tail)";
    char second[] = "unused";
    StringPayload payload0 = { first }, payload1 = { second };
    StringStructEntry entry1 = { 0, &payload1 }, entry0 = { &entry1, &payload0 };
    StringList list = { 2, &entry0, 0, 0 };
    SetupWin setup;
    setup.item_texts_ = &list;
    setup.overlay_buffers_[0] = new Buffer(OVERLAY);
    setup.overlay_buffers_[0]->dib_.bmiHeader.biWidth = 13;
    setup.overlay_buffers_[0]->dib_.bmiHeader.biHeight = -9;
    setup.overlay_buffers_[0]->pixel = transparent_overlay ? 9 : 0;
    setup.overlay_x_[0] = 41; setup.overlay_y_[0] = 52;
    opening_surface.soft_calls = 0;
    if (setup.draw_item(0, selected) != 0) return 1;
    if (setup.clip_calls != 1 || setup.color_calls != 1 ||
        setup.copy_calls != 0 || setup.text_calls != 1 || setup.restore_calls != 1 ||
        setup.buffer2_.copy_calls != 1)
        return 2;
    if (setup.buffer_.clip_calls || setup.buffer_.color_calls ||
        setup.buffer_.copy_calls || setup.buffer_.text_calls || setup.buffer_.restore_calls)
        return 3;
    if (setup.buffer2_.last_width != 20 || setup.buffer2_.last_height != 8 ||
        setup.buffer2_.last_x != 85 || setup.buffer2_.last_y != 20)
        return 4;
    if (setup.text_color != (selected ? 0xCA : 0xCD) ||
        std::strcmp(setup.last_text, "ALPHA BETA") != 0 || setup.text_len != 10)
        return 5;
    if (selected) {
        if (transparent_overlay &&
            (setup.overlay_buffers_[0]->draw_calls != 1 ||
             setup.overlay_buffers_[0]->copy_calls != 0 ||
             setup.overlay_buffers_[0]->last_width != 1 ||
             setup.overlay_buffers_[0]->last_height != 1 ||
             setup.overlay_buffers_[0]->last_x != 41 ||
             setup.overlay_buffers_[0]->last_y != 52))
            return 8;
        if (!transparent_overlay &&
            (setup.overlay_buffers_[0]->copy_calls != 1 ||
             setup.overlay_buffers_[0]->draw_calls != 0 ||
             setup.overlay_buffers_[0]->last_width != 13 ||
             setup.overlay_buffers_[0]->last_height != 9 ||
            setup.overlay_buffers_[0]->last_x != 41 ||
             setup.overlay_buffers_[0]->last_y != 52))
            return 8;
        if (opening_surface.soft_calls != 1 || opening_surface.soft_x != 41 ||
            opening_surface.soft_y != 52 || opening_surface.soft_width != 13 ||
            opening_surface.soft_height != 9)
            return 6;
    } else if (opening_surface.soft_calls != 0 ||
               setup.overlay_buffers_[0]->copy_calls != 0 ||
               setup.overlay_buffers_[0]->draw_calls != 0) {
        return 7;
    }
    delete setup.overlay_buffers_[0];
    return 0;
}

int main() {
    const int selected = run_case(1, 0);
    if (selected != 0) { std::fprintf(stderr, "selected row fixture %d\n", selected); return 1; }
    const int selected_draw = run_case(1, 1);
    if (selected_draw != 0) { std::fprintf(stderr, "selected draw fixture %d\n", selected_draw); return 1; }
    const int unselected = run_case(0, 0);
    if (unselected != 0) { std::fprintf(stderr, "unselected row fixture %d\n", unselected); return 1; }
    std::puts("SetupWin draw_item inherited-buffer control: PASS");
    return 0;
}
"""


def run_host(tmp_path, compiler, body):
    if compiler is None:
        pytest.skip("a host C++ compiler is required")
    source = tmp_path / "setupwin-draw-item.cpp"
    binary = tmp_path / "setupwin-draw-item"
    source.write_text(FIXTURE.replace("// ACTUAL_BODY_INSERTION_POINT", body), encoding="utf-8")
    subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
                    str(source), "-o", str(binary)], check=True,
                   capture_output=True, text=True, timeout=30)
    return subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_draw_item_uses_inherited_buffer_for_window_operations(tmp_path, compiler_name):
    result = run_host(tmp_path, shutil.which(compiler_name), actual_body())
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("before,after,diagnostic", [
    ("window_buffer->set_clip(&row);", "buffer_.set_clip(&row);",
     "clip used auxiliary buffer"),
    ("window_buffer->set_text_color(selected ? 0xCA : 0xCD, -1, 1, 1);",
     "buffer_.set_text_color(selected ? 0xCA : 0xCD, -1, 1, 1);",
     "color used auxiliary buffer"),
    ("buffer2_.copy(window_buffer,", "buffer2_.copy(&buffer_,",
     "Flic copy target is not inherited buffer"),
    ("window_buffer->write_cent_l(name, &row,",
     "buffer_.write_cent_l(name, &row,", "text used auxiliary buffer"),
    ("window_buffer->restore_clip();", "buffer_.restore_clip();",
     "clip restored on auxiliary buffer"),
])
@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_wrong_auxiliary_buffer_mutants_fail_semantically(
        tmp_path, compiler_name, before, after, diagnostic):
    body = actual_body()
    assert before in body, (before, body)
    result = run_host(tmp_path, shutil.which(compiler_name),
                      body.replace(before, after, 1))
    assert result.returncode == 97, (result.returncode, result.stdout, result.stderr)
    assert ("SEMANTIC97: " + diagnostic) in result.stderr
