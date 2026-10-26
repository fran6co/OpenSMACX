"""Host controls for the complete current AlphaMenu::show body.

The spies make ordering and arguments observable; this is not a VC6 ABI or
game-runtime proof. The one damaged countdown is required to compile and fail
the same unchanged animation witness that the actual extracted body passes.
"""

from pathlib import Path
import os
import resource
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/alphamenu.cpp"
SIGNATURE = "void AlphaMenu::show()"


def extract_function(source: str, signature: str) -> str:
    assert source.count(signature) == 1
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    state = "normal"
    quote = ""
    escape = False
    index = opening
    while index < len(source):
        char = source[index]
        following = source[index + 1] if index + 1 < len(source) else ""
        if state == "line":
            if char == "\n":
                state = "normal"
        elif state == "block":
            if char == "*" and following == "/":
                state = "normal"
                index += 1
        elif state == "quoted":
            if escape:
                escape = False
            elif char == "\\":
                escape = True
            elif char == quote:
                state = "normal"
        else:
            if char == "/" and following == "/":
                state = "line"
                index += 1
            elif char == "/" and following == "*":
                state = "block"
                index += 1
            elif char in ("'", '"'):
                state, quote = "quoted", char
            elif char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return source[start:index + 1]
        index += 1
    raise AssertionError("unterminated AlphaMenu::show definition")


FIXTURE = r'''
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
using std::string;
using std::vector;
typedef unsigned int DWORD;
struct RECT { int left, top, right, bottom; };
static vector<string> events;
static void record(const char *s) { events.push_back(s); }
static string number(const char *prefix, int a, int b = -999, int c = -999) {
  char out[128];
  if (b == -999) std::sprintf(out, "%s%d", prefix, a);
  else if (c == -999) std::sprintf(out, "%s%d:%d", prefix, a, b);
  else std::sprintf(out, "%s%d:%d:%d", prefix, a, b, c);
  return string(out);
}
struct Win {
  void show(int mode) { events.push_back(number("win.show:", mode)); }
};
struct Buffer {
  RECT last_clip;
  void set_clip(RECT *r) {
    last_clip = *r;
    events.push_back(number("clip.set:", r->left, r->top, r->bottom));
  }
  void restore_clip() { record("clip.restore"); }
  void draw(Buffer *target, int id, int x, int y, int a, int b) {
    if (!target || id != 9 || a != 1 || b != 1) record("bad.draw.args");
    char out[128]; std::sprintf(out, "draw:%d:%d:%d:%d:%d", id, x, y, a, b);
    events.push_back(out);
  }
};
struct Sprite { int height() const { return 10; } };
struct Fields { unsigned int game_preferences_; };
struct GameData { Fields fields_; };
static GameData GameDataStateStorage;
static int WinScreenWidth;
static bool mutate_screen_width;
static int IsMultiplayerNet;
static int WinFlipClipped;
struct Wave { void play() { record("wave.play"); } };
static Wave g_ALPHAMENU_WAVE;
struct ScreenOwner { Buffer screen; Buffer &buffer() { return screen; } };
static ScreenOwner ScreenBufferOwner;
struct Button {
  int pressed;
  void set_pressed_visual(int v) { pressed = v; events.push_back(number("button:", v)); }
  void redraw() { record("button.redraw"); }
};
struct MainInterface { Button flat_button_[4]; };
static MainInterface MainInterfaceGlobal;
static int clock_value;
static DWORD timeGetTime() { events.push_back(number("time:", 100 + clock_value)); return 100 + clock_value++; }
static void flip(RECT *r) {
  events.push_back(number("flip:", r->left, r->top, r->bottom));
}
static void wait_for_animation_frame(DWORD started) {
  events.push_back(number("wait:", static_cast<int>(started)));
}
struct AlphaMenu : Win, Buffer {
  bool visible;
  RECT work;
  Sprite sprite1_;
  AlphaMenu() : visible(false), work() {}
  bool is_visible() { record("visible?"); return visible; }
  void on_redraw() { record("on_redraw"); }
  RECT *buffer_working_rect() { record("working_rect"); return &work; }
  void client_to_screen(RECT *r) {
    record("client_to_screen"); r->left += 10; r->top += 10;
    r->right += 10; r->bottom += 10;
    if (mutate_screen_width) WinScreenWidth = 1060;
  }
  void do_sound() { record("sound"); }
  void show();
};
int animation_frame_x();
static void assert_equal(const vector<string> &got, const vector<string> &want) {
  if (got != want) {
    std::fprintf(stderr, "event mismatch: got %lu expected %lu\n",
                 static_cast<unsigned long>(got.size()),
                 static_cast<unsigned long>(want.size()));
    for (size_t i = 0; i < got.size(); ++i) std::fprintf(stderr, "G %s\n", got[i].c_str());
    for (size_t i = 0; i < want.size(); ++i) std::fprintf(stderr, "W %s\n", want[i].c_str());
    ::exit(17);
  }
}
'''


def main_for(source: str) -> str:
    return source + r'''
int main() {
  AlphaMenu menu;
  menu.work.left = 1; menu.work.top = 2; menu.work.right = 101; menu.work.bottom = 62;
  WinScreenWidth = 800; mutate_screen_width = true;
  GameDataStateStorage.fields_.game_preferences_ = 0x8000000u | 0x400u;
  IsMultiplayerNet = 0; WinFlipClipped = 0; clock_value = 0;
  menu.show();
  vector<string> expected;
  expected.push_back("visible?"); expected.push_back("wave.play");
  expected.push_back("on_redraw"); expected.push_back("working_rect");
  expected.push_back("client_to_screen"); expected.push_back("frame_x");
  expected.push_back("clip.set:11:12:72");
  const int ys[] = {62, 50, 38, 26, 14, 2};
  for (int i = 0; i != 6; ++i) {
    expected.push_back(number("time:", 100 + i));
    char draw[128]; std::sprintf(draw, "draw:9:18:%d:1:1", ys[i]); expected.push_back(draw);
    expected.push_back("flip:11:12:72"); expected.push_back("sound");
    expected.push_back(number("wait:", 100 + i));
  }
  expected.push_back("clip.restore"); expected.push_back("clip.restore");
  expected.push_back("win.show:3"); expected.push_back("button:1");
  expected.push_back("button.redraw");
  assert_equal(events, expected);
  if (WinFlipClipped != 0 || MainInterfaceGlobal.flat_button_[3].pressed != 1) return 18;
  if (ScreenBufferOwner.screen.last_clip.left != 11 ||
      ScreenBufferOwner.screen.last_clip.top != 12 ||
      ScreenBufferOwner.screen.last_clip.right != 111 ||
      ScreenBufferOwner.screen.last_clip.bottom != 72) return 19;

  // The visible guard must return before preferences, redraw, clip or UI writes.
  events.clear(); menu.visible = true; menu.show();
  vector<string> visible_expected; visible_expected.push_back("visible?");
  assert_equal(events, visible_expected);

  // Animation gate disabled: wave still follows its option, but there are no
  // frame callbacks and the original unconditional clip restoration remains.
  events.clear(); menu.visible = false; menu.work.left = 1; menu.work.top = 2;
  menu.work.right = 101; menu.work.bottom = 62;
  WinScreenWidth = 800; mutate_screen_width = true;
  GameDataStateStorage.fields_.game_preferences_ = 0x400u; IsMultiplayerNet = 0;
  menu.show();
  vector<string> disabled;
  disabled.push_back("visible?"); disabled.push_back("wave.play");
  disabled.push_back("on_redraw"); disabled.push_back("working_rect");
  disabled.push_back("client_to_screen"); disabled.push_back("frame_x");
  disabled.push_back("clip.set:11:12:72");
  disabled.push_back("clip.restore"); disabled.push_back("win.show:3");
  disabled.push_back("button:1"); disabled.push_back("button.redraw");
  assert_equal(events, disabled);
  if (WinFlipClipped != 0) return 20;

  // Multiplayer suppresses the animation even when its preference is set;
  // without the wave bit there is no optional wave playback.
  events.clear(); menu.work.left = 1; menu.work.top = 2;
  menu.work.right = 101; menu.work.bottom = 62;
  WinScreenWidth = 800; mutate_screen_width = true;
  GameDataStateStorage.fields_.game_preferences_ = 0x8000000u;
  IsMultiplayerNet = 1;
  menu.show();
  vector<string> multiplayer;
  multiplayer.push_back("visible?"); multiplayer.push_back("on_redraw");
  multiplayer.push_back("working_rect"); multiplayer.push_back("client_to_screen");
  multiplayer.push_back("frame_x"); multiplayer.push_back("clip.set:11:12:72");
  multiplayer.push_back("clip.restore");
  multiplayer.push_back("win.show:3"); multiplayer.push_back("button:1");
  multiplayer.push_back("button.redraw");
  assert_equal(events, multiplayer);
  if (WinFlipClipped != 0) return 21;
  return 0;
}
'''


def run_cpp(tmp_path: Path, compiler: str, source: str, name: str) -> subprocess.CompletedProcess:
    cpp = tmp_path / (name + ".cpp")
    exe = tmp_path / name
    cpp.write_text(source)
    subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
                    "-Wno-unused-parameter", "-Wno-unused-variable", str(cpp), "-o", str(exe)],
                   check=True, capture_output=True, text=True, timeout=60, cwd=tmp_path)
    def no_core():
        resource.setrlimit(resource.RLIMIT_CORE, (0, 0))
    return subprocess.run([str(exe)], capture_output=True, text=True, timeout=30,
                          cwd=tmp_path, preexec_fn=no_core,
                          env=dict(os.environ, OPENSMACX_DISABLE_CORES="1"))


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_show_animation_and_five_frame_damage(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(compiler_name + " unavailable")
    source = SOURCE.read_text()
    actual = extract_function(source, SIGNATURE)
    helper = extract_function(source, "int animation_frame_x()")
    assert actual.index("client_to_screen(&screen_rect)") < actual.index(
        "const int frame_x = animation_frame_x()")
    assert actual.index("const int frame_x = animation_frame_x()") < actual.index(
        "int y = screen_rect.bottom - sprite1_.height()")
    assert actual.index("const int step =") < actual.index(".set_clip(&screen_rect)")
    assert actual.index(".set_clip(&screen_rect)") < actual.index(
        "if ((GameDataStateStorage.fields_.game_preferences_ & 0x8000000)")
    helper_impl = helper.replace("int animation_frame_x()", "int animation_frame_x_impl()", 1)
    assert helper_impl != helper
    instrumented_helper = helper_impl + r'''
int animation_frame_x() {
  record("frame_x");
  return animation_frame_x_impl();
}
'''
    assert actual.count("for (int frame = 0; frame != 6; ++frame)") == 1
    passing = run_cpp(tmp_path, compiler,
                      FIXTURE + instrumented_helper + actual + main_for(""), "actual")
    assert passing.returncode == 0, passing.stdout + passing.stderr

    damaged = actual.replace("frame != 6", "frame != 5")
    assert damaged != actual
    failing = run_cpp(tmp_path, compiler,
                      FIXTURE + instrumented_helper + damaged + main_for(""), "five_frame_damage")
    assert failing.returncode != 0, "five-frame mutation unexpectedly passed the unchanged six-frame witness"
