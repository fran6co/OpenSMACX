"""Compile the actual Win::left_down_event body against a narrow host fixture.

This exercises the slot-33 virtual call as well as the neighboring, nullary
Sound calls. It is a source control only, not evidence of native ABI/game
acceptance.
"""

from __future__ import annotations

import subprocess
import tempfile
from pathlib import Path


ROOT = next(p for p in Path(__file__).resolve().parents if (p / 'src/IMAGE_PIN').is_file())
SOURCE = ROOT / "src/win.cpp"
START = "void Win::left_down_event(int x, int y, int from_parent) {"


def _function_body(source: str) -> str:
    start = source.index(START)
    brace = source.index("{", start)
    depth = 0
    for pos in range(brace, len(source)):
        if source[pos] == "{":
            depth += 1
        elif source[pos] == "}":
            depth -= 1
            if depth == 0:
                return source[start : pos + 1]
    raise AssertionError("unterminated Win::left_down_event")


FIXTURE = r"""
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

using std::string;
using std::vector;
static vector<string> events;
using Callback = void (*)(int, int);
using FnCallback = void (*)(int, int);
using WinMouseEventCallback = void (*)(int, int);
static Callback WinLeftDownHook = nullptr;
static void *WinCallbackWindow = nullptr;
static void callback(int x, int y) {
  events.push_back("callback:" + std::to_string(x) + ":" + std::to_string(y));
}

struct Sound {
  int playing;
  explicit Sound(int state) : playing(state) {}
  virtual int is_playing() { events.push_back("is_playing"); return playing; }
  virtual int play() { events.push_back("play"); return 1; }
};

struct Win {
  uint32_t iFlags_ = 0;
  uint8_t iSomeFlag_ = 0;
  FnCallback field_418_ = nullptr;
  Sound *val_9_ = nullptr;
  Sound *val_21_ = nullptr;
  virtual void on_left_down(int, int) { events.push_back("left"); }
  virtual void on_nc_left_down(int, int) { events.push_back("nc"); }
  void left_down_event(int, int, int);
};
// BODY_MARKER

struct ProbeWin : Win {
  void on_left_down(int x, int y) override {
    events.push_back("left:" + std::to_string(x) + ":" + std::to_string(y));
  }
  void on_nc_left_down(int x, int y) override {
    events.push_back("nc:" + std::to_string(x) + ":" + std::to_string(y));
  }
};
static void hook(int x, int y) {
  events.push_back("hook:" + std::to_string(x) + ":" + std::to_string(y));
}
static bool check(const vector<string>& want) {
  if (events == want) return true;
  for (const auto& e : events) std::fprintf(stderr, "%s\n", e.c_str());
  return false;
}
int main() {
  ProbeWin win;
  Sound sound{1};
  win.field_418_ = callback;
  win.val_9_ = &sound;
  win.val_21_ = &sound;
  WinLeftDownHook = hook;

  win.left_down_event(7, 9, 1);
  if (!check({"hook:7:9", "nc:7:9", "is_playing"})) return 1;
  if (WinCallbackWindow != nullptr) return 2;

  events.clear();
  sound.playing = 0;
  win.left_down_event(3, 4, 0);
  if (!check({"callback:3:4", "hook:3:4", "left:3:4", "is_playing", "play"})) return 3;
  if (WinCallbackWindow != &win) return 4;

  events.clear();
  win.iFlags_ = 0x200000;
  win.left_down_event(1, 2, 1);
  if (!events.empty()) return 5;
  win.iFlags_ = 0;
  win.iSomeFlag_ = 8;
  win.left_down_event(1, 2, 1);
  if (!events.empty()) return 6;
  return 0;
}
"""


def _compile_and_run(body: str) -> subprocess.CompletedProcess[str]:
    # Replace the fixture body with the exact extracted production function.
    fixture = FIXTURE.replace("// BODY_MARKER", body)
    with tempfile.TemporaryDirectory(prefix="opensmacx-leftdown-") as tmp:
        cpp = Path(tmp) / "left_down.cpp"
        exe = Path(tmp) / "left_down"
        cpp.write_text(fixture)
        subprocess.run(["g++", "-std=c++17", str(cpp), "-o", str(exe)], check=True)
        return subprocess.run([str(exe)], text=True, capture_output=True)


def test_actual_left_down_body_routes_parent_callback_and_sound_in_order() -> None:
    body = _function_body(SOURCE.read_text())
    assert "this->on_nc_left_down(x, y);" in body
    assert "this->vslot_33(x, y);" not in body
    result = _compile_and_run(body)
    assert result.returncode == 0, result.stderr


def test_control_rejects_dropped_slot33_dispatch() -> None:
    body = _function_body(SOURCE.read_text())
    mutant = body.replace("this->on_nc_left_down(x, y);", "(void)x; (void)y;")
    assert mutant != body
    result = _compile_and_run(mutant)
    assert result.returncode != 0
