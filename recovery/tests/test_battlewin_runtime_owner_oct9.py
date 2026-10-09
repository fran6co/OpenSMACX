import shutil
import subprocess
from pathlib import Path

import pytest


ROOT = Path(__file__).resolve().parent
PROJECT_ROOT = ROOT.parents[1]


def extract_method(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start : index + 1]
    raise AssertionError("unbalanced proposed BattleWin body")


HOST_TYPES = r"""
#include <stdio.h>

struct RECT { int left, top, right, bottom; };
struct BitmapInfoHeader { int biHeight; };
struct Dib { BitmapInfoHeader bmiHeader; };
struct Calls {
  int events[64];
  int count;
  const void *released;
  const void *status_buffer;
  const void *group_receiver;
  int group, x, y;
  int clip[4];
} calls;
static void event(int value) { calls.events[calls.count++] = value; }

struct Buffer {
  Dib dib_;
  void set_clip(int left, int top, int right, int bottom) {
    calls.status_buffer = this;
    calls.clip[0] = left; calls.clip[1] = top;
    calls.clip[2] = right; calls.clip[3] = bottom;
    event(1);
  }
};
struct GraphicWin : Buffer {};
struct SubInterface {
  void release_iface_mode() { calls.released = this; event(20); }
};
struct Button {
  int id;
  void show(int) { event(30 + id); }
  void hide() { event(40 + id); }
};
struct MainInterface : GraphicWin, SubInterface {
  Button flat_button_[8];
  void restore_back(int) { event(2); }
  void set_alert(int value) { event(value ? 8 : 50); }
  void set_date(int) { event(9); }
  void soft_update() { event(10); }
  void on_iface_group_clicked(int group, int x, int y) {
    calls.group_receiver = this;
    calls.group = group; calls.x = x; calls.y = y;
    event(21);
  }
};
struct BattleWin : SubInterface {
  void on_status(int);
  void on_iface_group_clicked(int, int, int);
};
extern MainInterface MainInterfaceGlobal;
extern RECT interface_screen_rect;
void slide_show(GraphicWin *, int);
void slide_hide(GraphicWin *, int);
"""


HOST_CONTROL = r"""
MainInterface MainInterfaceGlobal;
RECT interface_screen_rect;
void slide_show(GraphicWin *, int) { event(7); }
void slide_hide(GraphicWin *, int) { event(51); }

static int expect(int condition, const char *message) {
  if (!condition) fprintf(stderr, "FAIL: %s\n", message);
  return condition ? 0 : 1;
}
static void reset_calls() {
  calls = Calls();
  for (int i = 0; i != 8; ++i) MainInterfaceGlobal.flat_button_[i].id = i;
}
static int has_events(const int *expected, int count) {
  if (calls.count != count) return 0;
  for (int i = 0; i != count; ++i)
    if (calls.events[i] != expected[i]) return 0;
  return 1;
}

int main() {
  int failures = 0;
  BattleWin battle;

  // The shared canvas is MainInterface's real Buffer base at +0x444.
  reset_calls();
  interface_screen_rect.left = 321;
  MainInterfaceGlobal.dib_.bmiHeader.biHeight = -480;
  battle.on_status(1);
  const int show_events[] = {1, 2, 33, 34, 36, 37, 7, 8, 9, 10};
  failures += expect(calls.status_buffer == static_cast<Buffer *>(&MainInterfaceGlobal),
                     "status clip uses MainInterfaceGlobal Buffer base");
  failures += expect(calls.clip[0] == 0 && calls.clip[1] == 0
                     && calls.clip[2] == 321 && calls.clip[3] == 480,
                     "status clip keeps original extent and inverted DIB height");
  failures += expect(has_events(show_events, 10),
                     "status entry call order is preserved");

  reset_calls();
  battle.on_status(0);
  const int hide_events[] = {46, 47, 43, 44, 50, 51};
  failures += expect(has_events(hide_events, 6),
                     "status exit button and slide order is preserved");

  // The callback arrives through the BattleWin SubInterface, then the
  // MainInterface front receives its own group callback after release.
  reset_calls();
  battle.on_iface_group_clicked(0, 17, 29);
  failures += expect(calls.count == 0, "non-group-1 callback is ignored");
  reset_calls();
  battle.on_iface_group_clicked(1, 17, 29);
  const int group_events[] = {20, 21};
  failures += expect(calls.released == static_cast<SubInterface *>(&battle),
                     "interface mode releases the BattleWin SubInterface");
  failures += expect(calls.group_receiver == &MainInterfaceGlobal,
                     "group callback receives MainInterface front");
  failures += expect(calls.group == 1 && calls.x == 17 && calls.y == 29,
                     "group callback forwards group and coordinates");
  failures += expect(has_events(group_events, 2),
                     "release precedes MainInterface group callback");
  if (failures) return 1;
  puts("BattleWin runtime owner actual-body controls passed");
  return 0;
}
"""


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_actual_battlewin_runtime_owner_bodies(compiler, tmp_path):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(f"{compiler} is not installed")

    source_text = (PROJECT_ROOT / "src/battlewin.cpp").read_text()
    status = extract_method(source_text, "void BattleWin::on_status(")
    group = extract_method(source_text, "void BattleWin::on_iface_group_clicked(")
    translation_unit = (
        HOST_TYPES
        + "\n#define InterfaceScreenRect (&interface_screen_rect)\n"
        + status
        + "\n"
        + group
        + "\n"
        + HOST_CONTROL
    )
    source = tmp_path / "battlewin_runtime_owner_extracted.cpp"
    binary = tmp_path / "battlewin_runtime_owner_host"
    source.write_text(translation_unit)
    built = subprocess.run(
        [executable, "-std=c++98", "-Wall", "-Wextra", str(source), "-o", str(binary)],
        capture_output=True,
        text=True,
        timeout=30,
    )
    assert built.returncode == 0, built.stdout + built.stderr

    completed = subprocess.run(
        [str(binary)], capture_output=True, text=True, timeout=30
    )
    assert completed.returncode == 0, (
        f"host control returned {completed.returncode}\n"
        + completed.stdout
        + completed.stderr
    )
