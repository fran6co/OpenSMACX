"""Actual-source MainInterface controls; not target layout or runtime proof."""
from pathlib import Path
import shutil
import subprocess
import tempfile

import pytest

ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/maininterface.cpp"


def extract(signature):
    source = SOURCE.read_text()
    start = source.index(signature)
    brace = source.index("{", start)
    depth = 0
    for pos in range(brace, len(source)):
        if source[pos] == "{":
            depth += 1
        elif source[pos] == "}":
            depth -= 1
            if depth == 0:
                return source[start:pos + 1]
    raise AssertionError("unterminated method: " + signature)


RESTORE = extract("void MainInterface::restore_back(int a1)")
ALERT = extract("void MainInterface::set_alert(int a1)")

HARNESS = r'''#include <cassert>
#include <climits>
#include <stdint.h>
#include <vector>
struct RECT { int left, top, right, bottom; };
struct Tile { const void *receiver; int style, x1, y1, x2, y2, w, h; };
static std::vector<int> events;
static std::vector<Tile> tiles;
static int clips;
static RECT *last_clip;
static struct Time *expected_time;
static int MainInterfaceTileFlag;
struct Time {
  int starts;
  uint32_t *observed_alert;
  Time() : starts(0), observed_alert(0) {}
  void start() { ++starts; assert(this == expected_time); assert(observed_alert && *observed_alert == 0u); events.push_back(20); }
};
struct Buffer {
  RECT rect2_;
  int index;
  Buffer() : rect2_(), index(-1) {}
  RECT &current_clip() { return current_; }
  RECT current_;
  void set_clip(RECT *r) { assert(r == &rect2_); last_clip = r; ++clips; events.push_back(10); }
  void restore_clip() { set_clip(&rect2_); }
  void tile(void *style, int x1, int y1, int x2, int y2, int w, int h);
};
struct BoxStyle { void *mask_; };
static BoxStyle InterfaceBoxStyles[10];
struct MainInterface : Buffer {
  RECT rect4_, right_frame_rect_, info_panel_frame_, lower_frame_rect_, rect1_, lower_animation_bounds_;
  uint32_t alert_;
  Time time1_, time2_;
  void restore_back(int);
  void set_alert(int);
  MainInterface() : alert_(77) { time2_.observed_alert = &alert_; }
};
static MainInterface MainInterfaceGlobal;
void Buffer::tile(void *style, int x1, int y1, int x2, int y2, int w, int h) {
  assert(this == static_cast<Buffer *>(&MainInterfaceGlobal));
  Tile t; t.receiver = this; t.style = style ? 1 : 0;
  t.x1 = x1; t.y1 = y1; t.x2 = x2; t.y2 = y2; t.w = w; t.h = h;
  tiles.push_back(t);
  events.push_back(11 + static_cast<int>(tiles.size()));
  // Simulate callback/global mutation after tile 2; tile 3 must reread it.
  if (tiles.size() == 2) MainInterfaceTileFlag = 0;
}
__RESTORE__
__ALERT__

static void eq(const Tile &t, int x, int y, int w, int h) {
  assert(t.style == 1 && t.x1 == x && t.y1 == y && t.x2 == x && t.y2 == y);
  assert(t.w == w && t.h == h);
}
static RECT rect(int l, int t, int r, int b) {
  RECT value; value.left = l; value.top = t; value.right = r; value.bottom = b;
  return value;
}
int main() {
  InterfaceBoxStyles[9].mask_ = reinterpret_cast<void *>(1);
  MainInterface receiver;
  receiver.rect4_ = rect(10, 20, 90, 100);
  receiver.lower_animation_bounds_ = rect(30, 40, 130, 140);
  receiver.lower_frame_rect_ = rect(50, 60, 150, 160);
  receiver.rect1_ = rect(70, 80, 170, 180);
  receiver.info_panel_frame_ = rect(90, 100, 190, 200);
  receiver.right_frame_rect_ = rect(110, 120, 210, 220);
  MainInterfaceGlobal.rect2_ = rect(1, 2, 3, 4);
  MainInterfaceTileFlag = 1;
  receiver.restore_back(1);
  assert(clips == 1 && last_clip == &MainInterfaceGlobal.rect2_ && tiles.size() == 4);
  assert(events[0] == 10 && events[1] == 12);
  // Tile 2 sees flag=1; the tile callback changes it; tile 3 sees flag=0.
  eq(tiles[0], 11, 21, 78, 78);
  eq(tiles[1], 30, 41, 99, 98);
  eq(tiles[2], 91, 101, 98, 98);
  eq(tiles[3], 111, 121, 98, 98);
  assert(&receiver.rect2_ != &MainInterfaceGlobal.rect2_);
  assert(receiver.alert_ == 77 && receiver.time2_.starts == 0);

  tiles.clear(); events.clear(); clips = 0;
  MainInterfaceTileFlag = 0;
  receiver.restore_back(0);
  assert(clips == 0 && tiles.size() == 4 && events[0] == 12);
  eq(tiles[1], 50, 61, 99, 98);
  eq(tiles[2], 91, 101, 98, 98);

  events.clear();
  receiver.alert_ = 9;
  expected_time = &receiver.time2_;
  receiver.set_alert(1);
  assert(receiver.alert_ == 0 && receiver.time2_.starts == 1);
  assert(events.size() == 1 && events[0] == 20);
  assert(MainInterfaceGlobal.alert_ == 77 && MainInterfaceGlobal.time2_.starts == 0);
  receiver.alert_ = 9;
  receiver.set_alert(0);
  assert(receiver.alert_ == 0xffffffffu && receiver.time2_.starts == 1);

  // Exercise pinned x86 dword wrap at each arithmetic stage. Expected values
  // are signed interpretations of modulo-2^32 results, not saturation.
  receiver.rect4_ = rect(INT_MAX, INT_MIN, INT_MIN, INT_MAX);
  receiver.lower_animation_bounds_ = rect(INT_MIN, INT_MAX, INT_MAX, INT_MIN);
  receiver.lower_frame_rect_ = rect(1, 2, 3, 4);
  receiver.rect1_ = rect(5, 6, 7, 8);
  receiver.info_panel_frame_ = rect(INT_MAX, INT_MAX, INT_MIN, INT_MIN);
  receiver.right_frame_rect_ = rect(INT_MIN, INT_MIN, INT_MAX, INT_MAX);
  tiles.clear(); events.clear(); clips = 0;
  MainInterfaceTileFlag = 1;
  receiver.restore_back(0);
  assert(tiles.size() == 4 && clips == 0);
  eq(tiles[0], INT_MIN, INT_MIN + 1, -1, -3);
  eq(tiles[1], INT_MIN, INT_MIN, -2, -1);
  // Tile callback changed the flag after tile 2, so tile 3 uses info_panel_frame_.
  eq(tiles[2], INT_MIN, INT_MIN, -1, -1);
  eq(tiles[3], INT_MIN + 1, INT_MIN + 1, -3, -3);
}
'''


def compile_and_run(compiler, restore=RESTORE, alert=ALERT, expected_success=True):
    with tempfile.TemporaryDirectory(prefix="maininterface-host-") as temp:
        path = Path(temp) / "control.cpp"
        path.write_text(HARNESS.replace("__RESTORE__", restore).replace("__ALERT__", alert))
        exe = Path(temp) / "control"
        subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", "-fsanitize=undefined", "-fno-sanitize-recover=all", str(path), "-o", str(exe)], check=True)
        result = subprocess.run([str(exe)], timeout=5, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        if expected_success and result.returncode != 0:
            raise AssertionError(compiler + " actual-source control failed: " + result.stderr.decode(errors="replace"))
        if not expected_success and result.returncode == 0:
            raise AssertionError(compiler + " failed to reject a damage mutant")


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_source_and_binding_damage(compiler_name):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip("host compiler unavailable")
    compile_and_run(compiler)
    mutants = [
        (RESTORE.replace("&MainInterfaceGlobal", "this"), ALERT),
        (RESTORE.replace("lower_animation_bounds_", "lower_frame_rect_", 1), ALERT),
        (RESTORE.replace("restore_clip()", "set_clip(&current_clip())"), ALERT),
        (RESTORE, ALERT.replace("alert_ = 0;", "alert_ = 1;")),
        (RESTORE, ALERT.replace("time2_.start()", "time1_.start()")),
    ]
    for restore, alert in mutants:
        assert (restore, alert) != (RESTORE, ALERT)
        compile_and_run(compiler, restore, alert, expected_success=False)
