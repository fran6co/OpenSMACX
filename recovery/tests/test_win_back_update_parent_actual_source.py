"""Actual-source controls for Win::update_back_to_window's live state."""

from __future__ import annotations

import shutil
import subprocess
from pathlib import Path

import pytest
from match_recovery.source import read_file


UPDATE_ADDRESS = 0x005F7EC0
POINT_ADDRESS = 0x005ED240
UPDATE_SIGNATURE = "void Win::update_back_to_window(Buffer * buffer)"
POINT_SIGNATURE = "void Win::client_to_screen(int *x, int *y)"


def _root() -> Path:
    for parent in Path(__file__).resolve().parents:
        if (parent / "src/win.cpp").is_file():
            return parent
    raise AssertionError("could not locate repository root")


def _definition(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for offset in range(opening, len(source)):
        if source[offset] == "{":
            depth += 1
        elif source[offset] == "}":
            depth -= 1
            if depth == 0:
                return source[start : offset + 1]
    raise AssertionError("unterminated definition: " + signature)


def _actual_definitions() -> tuple[str, str]:
    root = _root()
    update_path = root / "src/win.cpp"
    point_path = root / "src/win_point_coordinates.cpp"
    update_source = update_path.read_text(encoding="utf-8")
    point_source = point_path.read_text(encoding="utf-8")
    update_rows = {row.address: row for row in read_file(update_path)}
    point_rows = {row.address: row for row in read_file(point_path)}

    assert UPDATE_ADDRESS in update_rows
    update_row = update_rows[UPDATE_ADDRESS]
    assert update_row.name == "?update_back_to_window@Win@@QAEXPAUBuffer@@@Z"
    assert update_row.image_spans == ((UPDATE_ADDRESS, 0x005F813F),)
    assert POINT_ADDRESS in point_rows
    point_row = point_rows[POINT_ADDRESS]
    assert point_row.name == "?client_to_screen@Win@@QAEXPAH0@Z"
    assert point_row.image_spans == ((POINT_ADDRESS, 0x005ED2C5),)

    update = _definition(update_source, UPDATE_SIGNATURE)
    point = _definition(point_source, POINT_SIGNATURE)
    assert update in update_row.region
    assert point in point_row.region
    assert update.count("win_parent_->client_to_screen(&WinViewOriginX, &WinViewOriginY);") == 1
    assert update.count("this->client_to_screen(&WinViewOriginX, &WinViewOriginY);") == 1
    assert update.count("if (!(iSomeFlag_ & 2))") == 2
    assert "savedFlags & 2" not in update
    return update, point


HARNESS = r'''#include <assert.h>
#include <stdint.h>

struct RECT { int left, top, right, bottom; };
class Win;
static int events[16];
static int event_count;
static void event(int value) { events[event_count++] = value; }
static int init_count, init_width, init_height, init_a, init_b;
static int screen_count, screen_window_was_null;
static RECT screen_rect;
static int zorder_count;
static Win *hook_target;
static int hook_bit2_mode;

class Buffer {
public:
  void init(int width, int height, int a, int b) {
    event(2); ++init_count;
    init_width = width; init_height = height; init_a = a; init_b = b;
  }
};

class Win {
public:
  RECT client_rect_;
  RECT outer_rect_;
  unsigned int iFlags_;
  unsigned int iSomeFlag_;
  Win *win_parent_;
  Win() : iFlags_(0), iSomeFlag_(0), win_parent_(0) {
    client_rect_.left = client_rect_.top = 0;
    client_rect_.right = client_rect_.bottom = 0;
    outer_rect_.left = outer_rect_.top = 0;
    outer_rect_.right = outer_rect_.bottom = 0;
  }
  void client_to_screen(int *x, int *y);
  void update_back_to_window(Buffer *buffer);
  static int update_screen(RECT *area, Win *window) {
    event(3); ++screen_count;
    screen_window_was_null = window == 0;
    if (area != 0) screen_rect = *area;
    return 71;
  }
};

static Buffer *WinBackBuffer;
static Win *WinZOrderWindow;
static int WinViewOriginX, WinViewOriginY;
static int WinZOrderCount, WinZOrderFlag;
static int WinRootCount;
static Win *WinRootWindows[40];
static void recurse_zorder(Win *window);

static void reset_observations() {
  event_count = init_count = screen_count = zorder_count = 0;
  init_width = init_height = init_a = init_b = -777;
  screen_window_was_null = 0;
  screen_rect.left = screen_rect.top = screen_rect.right = screen_rect.bottom = -777;
  hook_target = 0; hook_bit2_mode = 0;
}

void recurse_zorder(Win *window) {
  (void)window;
  event(1); ++zorder_count;
  // This deliberate harness hook distinguishes a live member read from an
  // entry snapshot. It is not a claim that the pinned recurse_zorder closure
  // synchronously writes this bit in normal gameplay.
  if (hook_target != 0) {
    if (hook_bit2_mode > 0) hook_target->iSomeFlag_ |= 2U;
    if (hook_bit2_mode < 0) hook_target->iSomeFlag_ &= ~2U;
    hook_target = 0;
  }
}

'''


MAIN = r'''static void set_rect(RECT *r, int l, int t, int rr, int b) {
  r->left = l; r->top = t; r->right = rr; r->bottom = b;
}
static void set_window(Win *w, unsigned int flags, unsigned int some,
                       Win *parent) {
  set_rect(&w->client_rect_, 10, 20, 130, 140);
  set_rect(&w->outer_rect_, 1, 2, 101, 102);
  w->iFlags_ = flags; w->iSomeFlag_ = some; w->win_parent_ = parent;
}
static void set_chain(Win *self, Win *parent, Win *grand) {
  set_window(self, 0x8020U, 1U, parent);
  set_window(parent, 0x8020U, 0U, grand);
  set_window(grand, 0U, 0U, 0);
  set_rect(&parent->client_rect_, 100, 200, 200, 300);
  set_rect(&parent->outer_rect_, 3, 4, 103, 104);
  set_rect(&grand->client_rect_, 1000, 2000, 1100, 2100);
  set_rect(&grand->outer_rect_, 5, 6, 105, 106);
}
static void setup_zorder(Win *z) {
  WinRootCount = 1; WinRootWindows[0] = z;
  z->iSomeFlag_ = 1U;
  WinZOrderWindow = 0; WinZOrderCount = 17; WinZOrderFlag = 23;
  WinBackBuffer = 0; WinViewOriginX = 33; WinViewOriginY = 44;
}
static void expect_events() {
  assert(event_count == 4);
  assert(events[0] == 1 && events[1] == 2 && events[2] == 3 && events[3] == 1);
  assert(zorder_count == 2 && init_count == 1 && screen_count == 1);
  assert(init_a == 0 && init_b == 0 && screen_window_was_null);
  assert(WinBackBuffer == 0 && WinZOrderWindow == 0);
  assert(WinZOrderCount == 0);
}
static void expect_screen(int l, int t, int r, int b, int width, int height) {
  assert(screen_rect.left == l && screen_rect.top == t);
  assert(screen_rect.right == r && screen_rect.bottom == b);
  assert(init_width == width && init_height == height);
}

int main() {
  Win self, parent, grand, z;
  Buffer buffer;
  set_chain(&self, &parent, &grand);
  setup_zorder(&z);

  // Parent branch: independent parent/grandparent origins and both 0x8000
  // subtractions are visible. The receiver must not add self a second time.
  reset_observations(); setup_zorder(&z);
  self.iSomeFlag_ = 1U;
  self.update_back_to_window(&buffer);
  expect_events(); expect_screen(1111, 2222, 1211, 2322, 100, 100);
  assert(self.iSomeFlag_ == 1U);

  // Entry snapshot says bit 2 set, but the z-order service boundary hook
  // clears the live bit before either branch selection. Both selections must
  // use the live field: parent-coordinate path and outer RECT.
  reset_observations(); setup_zorder(&z);
  self.iSomeFlag_ = 3U; hook_target = &self; hook_bit2_mode = -1;
  self.update_back_to_window(&buffer);
  expect_events(); expect_screen(1111, 2222, 1211, 2322, 100, 100);
  assert(self.iSomeFlag_ == 3U);

  // Conversely, the snapshot is clear and the live bit becomes set. The
  // this-helper/client RECT branch is selected, and entry flags are restored.
  reset_observations(); setup_zorder(&z);
  self.iSomeFlag_ = 1U; hook_target = &self; hook_bit2_mode = 1;
  self.update_back_to_window(&buffer);
  expect_events(); expect_screen(1110, 2220, 1230, 2340, 120, 120);
  assert(self.iSomeFlag_ == 1U);

  // 0x20 without a parent and 0x8000 without 0x20 both avoid parent access
  // and the subtraction. This also preserves zero-origin local drawing.
  reset_observations(); setup_zorder(&z);
  self.iFlags_ = 0x8020U; self.win_parent_ = 0; self.iSomeFlag_ = 1U;
  self.update_back_to_window(&buffer);
  expect_events(); expect_screen(11, 22, 111, 122, 100, 100);

  reset_observations(); setup_zorder(&z);
  self.iFlags_ = 0x8000U; self.win_parent_ = &parent; self.iSomeFlag_ = 1U;
  self.update_back_to_window(&buffer);
  expect_events(); expect_screen(11, 22, 111, 122, 100, 100);

  // The initial null-buffer return has no observable writes, calls, or flag
  // restoration work; pre-existing global values remain untouched.
  reset_observations(); setup_zorder(&z);
  Buffer *sentinel = &buffer;
  WinBackBuffer = sentinel; WinZOrderWindow = &z;
  WinViewOriginX = 91; WinViewOriginY = 92;
  WinZOrderCount = 93; WinZOrderFlag = 94;
  self.iSomeFlag_ = 3U;
  self.update_back_to_window(0);
  assert(event_count == 0 && init_count == 0 && screen_count == 0 && zorder_count == 0);
  assert(WinBackBuffer == sentinel && WinZOrderWindow == &z);
  assert(WinViewOriginX == 91 && WinViewOriginY == 92);
  assert(WinZOrderCount == 93 && WinZOrderFlag == 94 && self.iSomeFlag_ == 3U);
  return 0;
}
'''


def _compile_run(tmp_path: Path, update: str, point: str, compiler: str) -> int:
    unit = tmp_path / "win_back_update.cpp"
    exe = tmp_path / "win_back_update"
    unit.write_text(HARNESS + point + "\n" + update + "\n" + MAIN,
                    encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++98", "-O0", str(unit), "-o", str(exe)],
        text=True, capture_output=True, timeout=30,
    )
    assert built.returncode == 0, built.stderr
    return subprocess.run([str(exe)], text=True, capture_output=True,
                          timeout=10).returncode


def _compilers() -> list[str]:
    values = [shutil.which("g++"), shutil.which("clang++")]
    values = [value for value in values if value]
    if not values:
        pytest.skip("host GCC or Clang unavailable")
    return values


@pytest.mark.parametrize("compiler", _compilers())
def test_back_update_uses_mapped_receivers_and_live_flags(
    tmp_path: Path, compiler: str,
) -> None:
    update, point = _actual_definitions()
    assert _compile_run(tmp_path, update, point, compiler) == 0


@pytest.mark.parametrize("compiler", _compilers())
def test_old_parent_receiver_damage_fails_parent_origin_witness(
    tmp_path: Path, compiler: str,
) -> None:
    update, point = _actual_definitions()
    old = "win_parent_->client_to_screen(&WinViewOriginX, &WinViewOriginY);"
    assert update.count(old) == 1
    damaged = update.replace(
        old, "this->client_to_screen(&WinViewOriginX, &WinViewOriginY);", 1
    )
    assert _compile_run(tmp_path, damaged, point, compiler) != 0


@pytest.mark.parametrize("compiler", _compilers())
@pytest.mark.parametrize("branch", (0, 1))
def test_saved_snapshot_branch_damage_fails_live_flag_witness(
    tmp_path: Path, compiler: str, branch: int,
) -> None:
    update, point = _actual_definitions()
    live = "if (!(iSomeFlag_ & 2))"
    pieces = update.split(live)
    assert len(pieces) == 3
    stale = "if (!(savedFlags & 2))"
    damaged = (
        pieces[0] + stale + pieces[1] + live + pieces[2]
        if branch == 0
        else pieces[0] + live + pieces[1] + stale + pieces[2]
    )
    assert _compile_run(tmp_path, damaged, point, compiler) != 0
