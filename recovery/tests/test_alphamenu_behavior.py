"""Host checks that compile and execute selected AlphaMenu production bodies."""

from pathlib import Path
import os
import shutil
import subprocess

import pytest


ROOT = (Path(os.environ["OPENSMACX_ROOT"]).resolve() if "OPENSMACX_ROOT" in os.environ
       else Path(__file__).resolve().parents[2])
SOURCE = ROOT / "src" / "alphamenu.cpp"


def extract_function(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for pos in range(opening, len(source)):
        if source[pos] == "{":
            depth += 1
        elif source[pos] == "}":
            depth -= 1
            if depth == 0:
                return source[start : pos + 1]
    raise AssertionError(f"unterminated function {signature}")


def extract_if_statement(source: str, marker: str, include_else: bool = False) -> str:
    start = source.index(marker)
    opening = source.index("{", start)
    depth = 0
    for pos in range(opening, len(source)):
        if source[pos] == "{":
            depth += 1
        elif source[pos] == "}":
            depth -= 1
            if depth == 0:
                end = pos + 1
                if include_else:
                    else_at = source.index("else", end)
                    semicolon = source.index(";", else_at)
                    end = semicolon + 1
                return source[start:end]
    raise AssertionError(f"unterminated conditional {marker}")


def compile_and_run(tmp_path: Path, source: str) -> None:
    compiler = shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    cpp = tmp_path / "alphamenu-production-bodies.cpp"
    binary = tmp_path / "alphamenu-production-bodies"
    cpp.write_text(source)
    subprocess.run(
        [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(binary)],
        check=True,
        timeout=30,
        capture_output=True,
        text=True,
    )
    subprocess.run([str(binary)], check=True, timeout=10)


def test_adjust_uses_real_sprite_dimensions_and_owned_maininterface_bounds(tmp_path):
    source = SOURCE.read_text()
    adjust = extract_function(source, "void AlphaMenu::adjust()")
    compute = extract_function(source, "void AlphaMenu::compute()")
    harness = r"""
#include <cassert>
struct RECT { int left, top, right, bottom; };
struct MainInterface { RECT window_bounds_; };
MainInterface MainInterfaceGlobal = {{0, 11, 0, 19}};
int WinScreenWidth = 1280;
int WinScreenHeight = 768;
struct Sprite { int w, h; int width() const { return w; } int height() const { return h; } };
struct MenuEntry { unsigned char flags; };
struct Spot {
  struct Row { int index, left, top, right, bottom; } rows[8];
  int used = 0;
  void init(int) { used = 0; }
  int add(int index, int, int left, int top, int length, int height) {
    rows[used++] = {index, left, top, left + length, top + height};
    return 0;
  }
};
class Win {
 public:
  void resize(int w, int h, int mode) { resized_w = w; resized_h = h; resize_mode = mode; }
  void move(int x, int y) { moved_x = x; moved_y = y; }
  void redraw() { ++redraws; }
  int resized_w = -1, resized_h = -1, resize_mode = -1;
  int moved_x = -1, moved_y = -1, redraws = 0;
};
class GraphicWin : public Win {};
class AlphaMenu : public GraphicWin {
 public:
  void adjust();
  void compute();
  int count_ = 3;
  Sprite sprites_[3] = {{7, 12}, {8, 13}, {9, 14}};
  Sprite sprite1_ = {31, 5};
  Spot spot_;
  MenuEntry entries_[3] = {{1}, {5}, {1}};
};
""" + adjust + "\n" + compute + r"""
int main() {
  AlphaMenu menu;
  menu.adjust();
  assert(menu.resized_w == 31 && menu.resized_h == 41 && menu.resize_mode == 0);
  assert(menu.moved_x == 128 && menu.moved_y == 719);
  assert(menu.redraws == 1 && menu.spot_.used == 3);
  assert(menu.spot_.rows[0].index == 0 && menu.spot_.rows[0].top == 5 && menu.spot_.rows[0].bottom == 17);
  assert(menu.spot_.rows[1].index == 1 && menu.spot_.rows[1].top == 29 && menu.spot_.rows[1].bottom == 41);
  assert(menu.spot_.rows[2].index == 2 && menu.spot_.rows[2].top == 17 && menu.spot_.rows[2].bottom == 29);

  WinScreenWidth = 800;
  menu.adjust();
  assert(menu.moved_x == 0);
}
"""
    compile_and_run(tmp_path, harness)


def test_fill_callback_clips_and_selects_original_box_style(tmp_path):
    source = SOURCE.read_text()
    fill = extract_function(
        source,
        "int __cdecl AlphaMenu::fill_func(GraphicWin *target, int mode, RECT *clip)",
    )
    harness = r"""
#include <cassert>
#define __cdecl
struct RECT { int left, top, right, bottom; };
struct BoxSpriteParams { int tag; };
BoxSpriteParams InterfaceBoxStyles[30];
class Buffer {
 public:
  void set_clip(const RECT *r) { if (clip_calls == 0) first_clip = *r; clip = *r; ++clip_calls; }
  void box_sprite(const RECT *r, const BoxSpriteParams *p) { box = *r; style = p->tag; }
  RECT clip = {-1, -1, -1, -1}, first_clip = {-1, -1, -1, -1};
  RECT box = {-1, -1, -1, -1};
  int style = -1, clip_calls = 0;
};
class GraphicWin : public Buffer {
 public:
  RECT *buffer_working_rect() { return &working; }
  RECT working = {0, 0, 100, 80};
};
class AlphaMenu { public: static int __cdecl fill_func(GraphicWin *, int, RECT *); };
""" + fill + r"""
int main() {
  for (int i = 0; i != 30; ++i) InterfaceBoxStyles[i].tag = i;
  GraphicWin win;
  RECT original = {4, 5, 88, 90};
  assert(AlphaMenu::fill_func(&win, 0, &original) == 1);
  assert(win.clip_calls == 2);
  assert(win.first_clip.left == 1 && win.first_clip.top == 5 && win.first_clip.right == 99 && win.first_clip.bottom == 90);
  assert(win.box.left == 0 && win.box.top == 0 && win.box.right == 100 && win.box.bottom == 80);
  assert(win.style == 27);
  assert(win.clip.left == 0 && win.clip.top == 0 && win.clip.right == 100 && win.clip.bottom == 80);

  AlphaMenu::fill_func(&win, 1, &original);
  assert(win.style == 29);
  assert(win.clip.left == 0 && win.clip.right == 100);
}
"""
    compile_and_run(tmp_path, harness)


def test_redraw_uses_menu_selection_colors_and_spot_rectangles(tmp_path):
    source = SOURCE.read_text()
    redraw = extract_function(source, "void AlphaMenu::on_redraw()")
    assert "string_upper_slot" not in redraw
    assert "0x0066931C" not in redraw
    harness = r"""
#include <cassert>
#include <cctype>
#include <cstdint>
#include <cstring>
#define __stdcall
struct RECT { int left, top, right, bottom; };
struct Font {};
Font popup_font;
Font *PopupMenuBoldFont = &popup_font;
int TextColourTable[195];
int MenuSelection = 4;
char *__stdcall CharUpperA(char *text) {
  char *result = text;
  for (; *text; ++text) *text = static_cast<char>(std::toupper(static_cast<unsigned char>(*text)));
  return result;
}
struct MenuEntry { const char *text; unsigned char flags; };
class Buffer {
 public:
  void set_font(Font *, Font *, Font *, Font *) { ++font_sets; }
  void set_text_color(int fg, int shadow, int, int) {
    colors[color_count][0] = fg;
    colors[color_count++][1] = shadow;
  }
  void write_l(const char *text, const RECT *rect, unsigned int) {
    std::strcpy(written[text_count], text);
    rects[text_count++] = *rect;
  }
  int font_sets = 0, color_count = 0, text_count = 0;
  int colors[8][2] = {};
  char written[8][256] = {};
  RECT rects[8] = {};
};
class GraphicWin : public Buffer {
 public:
  void fill(int value) { fill_arg = value; ++fills; }
  int fill_arg = -1, fills = 0;
};
class Sprite {
 public:
  int height() const { return h; }
  unsigned char transparent_index() const { return transparent; }
  void draw(Buffer *target, unsigned char, int x, int y, int, int) {
    target->drawn_ids[target->draw_count] = id;
    target->drawn_y[target->draw_count++] = y;
    (void)x;
  }
  int id = 0, h = 10;
  unsigned char transparent = 7;
};
// Keep the observation arrays beside Buffer to record the production draw calls.
struct Spot {
  RECT rows[5];
  void get_rect_at(int index, RECT *out) { *out = rows[index]; }
};
"""
    # Inject the draw observations into Buffer itself while keeping the
    # production call and receiver expression untouched.
    harness = harness.replace(
        "class Buffer {\n public:",
        "class Buffer {\n public:\n  int drawn_ids[16] = {}, drawn_y[16] = {}, draw_count = 0;",
    )
    harness += r"""
class AlphaMenu : public GraphicWin {
 public:
  void on_redraw();
  int count_ = 5;
  int field_A24_ = 2;
  MenuEntry entries_[5] = {{"alpha", 1}, {"beta", 3}, {"gamma", 3}, {"delta", 1}, {"omega", 1}};
  Spot spot_ = {{{10, 20, 90, 30}, {11, 31, 91, 41}, {12, 42, 92, 52},
                  {13, 53, 93, 63}, {14, 64, 94, 74}}};
  Sprite sprite1_ = {10, 6, 7};
  Sprite sprites_[3] = {{0, 10, 1}, {1, 10, 2}, {2, 10, 3}};
};
""" + redraw + r"""
int main() {
  for (int i = 0; i != 195; ++i) TextColourTable[i] = i * 10;
  AlphaMenu menu;
  menu.on_redraw();
  assert(menu.fills == 1 && menu.fill_arg == 9 && menu.font_sets == 1);
  assert(menu.draw_count == 6);
  assert(menu.drawn_ids[0] == 10 && menu.drawn_y[0] == 0);
  assert(menu.drawn_ids[1] == 0 && menu.drawn_y[1] == 6);
  assert(menu.drawn_ids[2] == 0 && menu.drawn_y[2] == 16);
  assert(menu.drawn_ids[3] == 2 && menu.drawn_y[3] == 26);
  assert(menu.drawn_ids[4] == 0 && menu.drawn_y[4] == 36);
  assert(menu.drawn_ids[5] == 1 && menu.drawn_y[5] == 46);
  assert(menu.text_count == 5 && menu.color_count == 5);
  assert(std::strcmp(menu.written[0], "ALPHA") == 0);
  assert(std::strcmp(menu.written[1], "BETA") == 0);
  assert(menu.colors[0][0] == 320 && menu.colors[1][0] == 360);
  assert(menu.colors[2][0] == 340 && menu.colors[3][0] == 320);
  assert(menu.colors[4][0] == 320);
  assert(menu.rects[0].left == 18 && menu.rects[0].right == 82);
  assert(menu.rects[4].top == 64 && menu.rects[4].bottom == 74);
}
"""
    compile_and_run(tmp_path, harness)


def test_hide_animation_draws_the_owned_graphic_buffer_to_screen(tmp_path):
    source = SOURCE.read_text()
    hide = extract_function(source, "void AlphaMenu::hide()")
    frame_x = extract_function(source, "int animation_frame_x()")
    wait = extract_function(source, "void wait_for_animation_frame(DWORD started)")
    harness = r"""
#include <cassert>
#include <vector>
typedef unsigned int DWORD;
struct RECT { int left, top, right, bottom; };
int WinScreenWidth = 800;
int WinScreenHeight = 600;
struct GameDataStateFields { int game_preferences_; };
struct GameDataStateWindow { GameDataStateFields fields_; };
GameDataStateWindow GameDataStateStorage = {{0x8000400}};
int IsMultiplayerNet = 0;
int MenuSelection = 8;
int WinFlipClipped = 0;
int WindowAnimationFrameDelay = 0;
DWORD ticks = 0;
DWORD timeGetTime() { return ++ticks; }
int sounds = 0;
void do_sound() { ++sounds; }
struct DrawRecord;
class Buffer;
struct DrawRecord { Buffer *source; Buffer *destination; int index, x, y; };
std::vector<DrawRecord> draws;
class Buffer {
 public:
  void init(int w, int h, int, int) { width = w; height = h; initialized = true; }
  void close() { closed = true; }
  int pixel_width() const { return width; }
  int pixel_height() const { return height; }
  int copy(Buffer *destination, int sx, int sy, int dx, int dy, int w, int h) {
    copy_source = this; copy_destination = destination;
    copy_args[0] = sx; copy_args[1] = sy; copy_args[2] = dx; copy_args[3] = dy;
    copy_args[4] = w; copy_args[5] = h; ++copy_count; return 0;
  }
  int draw(Buffer *destination, int index, int x, int y, int, int) {
    draws.push_back({this, destination, index, x, y}); return 0;
  }
  void set_clip(const RECT *r) { clip = *r; ++clip_count; }
  int width = 100, height = 100;
  bool initialized = false, closed = false;
  int copy_count = 0, copy_args[6] = {};
  Buffer *copy_source = nullptr, *copy_destination = nullptr;
  int clip_count = 0;
  RECT clip = {};
};
Buffer ScreenBuffer;
RECT WinScreenClipRect = {0, 0, 800, 600};
class Win {
 public:
  int is_visible() const { return visible; }
  void client_to_screen(RECT *r) { r->left += 10; r->right += 10; r->top += 20; r->bottom += 20; }
  void flip(RECT *) { ++flips; }
  void hide() { visible = false; ++hides; }
  bool visible = true;
  int flips = 0, hides = 0;
};
class GraphicWin : public Buffer, public Win {
 public:
  RECT *buffer_working_rect() { return &working; }
  RECT working = {0, 0, 100, 100};
};
class Console : public Buffer, public Win {
 public:
  void screen_to_client(RECT *r) { r->left -= 50; r->right -= 50; r->top -= 60; r->bottom -= 60; }
};
Console ConsoleGlobal;
struct Sprite { int height() const { return h; } int h = 10; };
struct Button { int pressed = -1, redraws = 0; void set_pressed_visual(int v) { pressed = v; } void redraw() { ++redraws; } };
struct MainInterface { int field_216E8_ = 1; Button flat_button_[4]; };
MainInterface MainInterfaceGlobal;
struct Wave { int plays = 0; int play() { ++plays; return 0; } };
Wave g_ALPHAMENU_WAVE;
class AlphaMenu : public GraphicWin {
 public:
  void hide();
  void on_redraw() { ++redraws; }
  Buffer buffer_;
  Sprite sprite1_;
  int redraws = 0;
};
""" + frame_x + wait + hide + r"""
int main() {
  AlphaMenu menu;
  menu.hide();
  assert(menu.redraws == 1 && MenuSelection == -1);
  assert(MainInterfaceGlobal.field_216E8_ == 0);
  assert(g_ALPHAMENU_WAVE.plays == 1);
  assert(menu.buffer_.initialized && menu.buffer_.closed);
  assert(menu.buffer_.width == 100 && menu.buffer_.height == 100);
  assert(menu.buffer_.copy_count == 6);
  assert(menu.buffer_.copy_source == &menu.buffer_);
  assert(menu.buffer_.copy_destination == &ScreenBuffer);
  assert(menu.buffer_.copy_args[0] == 0 && menu.buffer_.copy_args[1] == 0);
  assert(menu.buffer_.copy_args[2] == 10 && menu.buffer_.copy_args[3] == 20);
  assert(menu.buffer_.copy_args[5] == 100);
  assert(ConsoleGlobal.copy_count == 1);
  assert(ConsoleGlobal.copy_source == &ConsoleGlobal);
  assert(ConsoleGlobal.copy_destination == &menu.buffer_);
  assert(ConsoleGlobal.copy_args[0] == -40 && ConsoleGlobal.copy_args[1] == -40);
  assert(ConsoleGlobal.copy_args[5] == 100);
  assert(draws.size() == 6);
  for (int i = 0; i != 6; ++i) {
    assert(draws[i].source == static_cast<Buffer *>(&menu));
    assert(draws[i].destination == &ScreenBuffer);
    assert(draws[i].index == 9 && draws[i].x == 0);
  }
  assert(draws[0].y == 20 && draws[1].y == 40 && draws[2].y == 60);
  assert(draws[3].y == 80 && draws[4].y == 100 && draws[5].y == 110);
  assert(menu.flips == 6 && WinFlipClipped == 0 && ScreenBuffer.clip_count == 2);
  assert(!menu.visible && menu.hides == 1 && sounds == 6);
  assert(MainInterfaceGlobal.flat_button_[3].pressed == 0);
  assert(MainInterfaceGlobal.flat_button_[3].redraws == 1);

  // The original disabled/networked path branches around clip restoration and
  // WinFlipClipped clearing; hide still closes buffers and the owner window.
  AlphaMenu networked;
  GameDataStateStorage.fields_.game_preferences_ = 0;
  IsMultiplayerNet = 1;
  WinFlipClipped = 7;
  const int clips_before = ScreenBuffer.clip_count;
  const size_t draws_before = draws.size();
  networked.hide();
  assert(draws.size() == draws_before && networked.flips == 0);
  assert(ScreenBuffer.clip_count == clips_before + 1);
  assert(ScreenBuffer.clip.left == 10 && ScreenBuffer.clip.top == 20);
  assert(WinFlipClipped == 7);
  assert(!networked.visible && networked.hides == 1 && networked.buffer_.closed);
}
"""
    compile_and_run(tmp_path, harness)


def test_alpha_menu_show_callers_use_the_zero_argument_override(tmp_path):
    maininterface = (ROOT / "src" / "maininterface.cpp").read_text()
    tutwin = (ROOT / "src" / "tutwin.cpp").read_text()
    main_site = extract_if_statement(
        maininterface,
        "if (ConsoleGlobal.alpha_menu_.is_visible()) {",
        include_else=True,
    )
    tutorial_site = extract_if_statement(
        tutwin,
        "if (!ConsoleGlobal.alpha_menu_.is_visible()) {",
    )
    harness = r"""
#include <cassert>
class AlphaMenu {
 public:
  bool is_visible() const { return visible; }
  void hide_menus() { ++hide_menus_calls; }
  void hide() { ++hide_calls; visible = false; }
  void show() { ++show_calls; visible = true; }
  bool visible = false;
  int hide_menus_calls = 0, hide_calls = 0, show_calls = 0;
};
struct Console { AlphaMenu alpha_menu_; } ConsoleGlobal;
""" + "void maininterface_site() {\n" + main_site + "\n}\n" + \
        "void tutorial_site() {\n" + tutorial_site + "\n}\n" + r"""
int main() {
  maininterface_site();
  assert(ConsoleGlobal.alpha_menu_.show_calls == 1);
  ConsoleGlobal.alpha_menu_.visible = true;
  maininterface_site();
  assert(ConsoleGlobal.alpha_menu_.hide_menus_calls == 1);
  assert(ConsoleGlobal.alpha_menu_.hide_calls == 1);

  ConsoleGlobal.alpha_menu_.visible = false;
  tutorial_site();
  assert(ConsoleGlobal.alpha_menu_.show_calls == 2);
  ConsoleGlobal.alpha_menu_.visible = true;
  tutorial_site();
  assert(ConsoleGlobal.alpha_menu_.show_calls == 2);
}
"""
    compile_and_run(tmp_path, harness)
