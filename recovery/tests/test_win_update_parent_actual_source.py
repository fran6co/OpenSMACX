"""Actual-source regression for Win::update_window's parent receiver."""

from __future__ import annotations

import shutil
import subprocess
from pathlib import Path

import pytest
from match_recovery.source import read_file


ADDRESS = 0x005F74A0
HELPER_ADDRESS = 0x005ED240
UPDATE_SIGNATURE = "int Win::update_window(RECT * area)"
HELPER_SIGNATURE = "void Win::client_to_screen(int *x, int *y)"


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
    path = root / "src/win.cpp"
    source = path.read_text(encoding="utf-8")
    rows = {row.address: row for row in read_file(path)}
    assert ADDRESS in rows and HELPER_ADDRESS in rows
    assert rows[ADDRESS].name == "?update_window@Win@@QAEHPAURECT@@@Z"
    assert rows[ADDRESS].symbol == "?update_window@Win@@QAEHPAUtagRECT@@@Z"
    assert rows[ADDRESS].image_spans == ((ADDRESS, 0x005F757F),)
    assert rows[HELPER_ADDRESS].name == "?client_to_screen@Win@@QAEXPAH0@Z"
    assert rows[HELPER_ADDRESS].image_spans == ((HELPER_ADDRESS, 0x005ED2C5),)
    update = _definition(source, UPDATE_SIGNATURE)
    helper = _definition(source, HELPER_SIGNATURE)
    assert update in rows[ADDRESS].region
    assert helper in rows[HELPER_ADDRESS].region
    assert "win_parent_->client_to_screen(&x, &y);" in update
    return update, helper


SUPPORT = r'''#include <assert.h>

struct RECT { int left, top, right, bottom; };
struct Win {
  RECT client_rect_;
  RECT outer_rect_;
  unsigned int iFlags_;
  Win *win_parent_;
  Win() : iFlags_(0), win_parent_(0) {
    client_rect_.left = client_rect_.top = 0;
    client_rect_.right = client_rect_.bottom = 0;
    outer_rect_.left = outer_rect_.top = 0;
    outer_rect_.right = outer_rect_.bottom = 0;
  }
  void client_to_screen(int *x, int *y);
  int update_window(RECT *area);
};

static int screen_calls;
static int screen_was_null;
static RECT screen_rect;
static Win *screen_window;
static int screen_result;
static void reset_screen() {
  screen_calls = 0;
  screen_was_null = 0;
  screen_rect.left = screen_rect.top = screen_rect.right = screen_rect.bottom = -9999;
  screen_window = 0;
  screen_result = 73;
}

extern "C" int update_screen(RECT *area, Win *window) {
  ++screen_calls;
  screen_was_null = area == 0;
  screen_window = window;
  if (area != 0) screen_rect = *area;
  return screen_result;
}

'''


MAIN = r'''static void set_rect(RECT *rect, int left, int top, int right, int bottom) {
  rect->left = left; rect->top = top; rect->right = right; rect->bottom = bottom;
}

static void set_chain(Win *self, Win *parent, Win *grandparent) {
  set_rect(&self->client_rect_, 10, 20, 0, 0);
  set_rect(&self->outer_rect_, 1, 2, 0, 0);
  self->win_parent_ = parent;
  parent->iFlags_ = 0x20U;
  set_rect(&parent->client_rect_, 100, 200, 0, 0);
  set_rect(&parent->outer_rect_, 3, 4, 0, 0);
  parent->win_parent_ = grandparent;
  grandparent->iFlags_ = 0;
  set_rect(&grandparent->client_rect_, 1000, 2000, 0, 0);
  set_rect(&grandparent->outer_rect_, 5, 6, 0, 0);
  grandparent->win_parent_ = 0;
}

static void check_rect(int left, int top, int right, int bottom) {
  assert(screen_rect.left == left && screen_rect.top == top);
  assert(screen_rect.right == right && screen_rect.bottom == bottom);
}

int main() {
  Win self, parent, grandparent;
  set_chain(&self, &parent, &grandparent);
  RECT input = {7, 8, 9, 10};
  const RECT original = input;

  // Parent-relative path walks parent then grandparent, adding self only once.
  reset_screen();
  parent.iFlags_ = 0x8020U;
  self.iFlags_ = 0x20U;
  assert(self.update_window(&input) == 73);
  assert(screen_calls == 1 && !screen_was_null && screen_window == &self);
  // Parent's helper subtracts the grandparent's outer origin exactly once.
  check_rect(1121, 2234, 1123, 2236);
  assert(input.left == original.left && input.top == original.top &&
         input.right == original.right && input.bottom == original.bottom);

  // Self's 0x8000 gate additionally subtracts only the immediate parent's outer origin.
  reset_screen();
  self.iFlags_ = 0x8020U;
  assert(self.update_window(&input) == 73);
  assert(screen_calls == 1 && !screen_was_null && screen_window == &self);
  check_rect(1118, 2230, 1120, 2232);

  // Without 0x20 no parent walk or 0x8000 subtraction occurs.
  reset_screen();
  parent.iFlags_ = 0x20U;
  self.iFlags_ = 0x8000U;
  assert(self.update_window(&input) == 73);
  assert(screen_calls == 1 && screen_window == &self);
  check_rect(18, 30, 20, 32);

  // A null parent suppresses both parent traversal and the 0x8000 subtraction.
  reset_screen();
  self.iFlags_ = 0x8020U;
  self.win_parent_ = 0;
  assert(self.update_window(&input) == 73);
  assert(screen_calls == 1 && screen_window == &self);
  check_rect(18, 30, 20, 32);

  // Null-area fast path forwards null and the original receiver unchanged.
  reset_screen();
  assert(self.update_window(0) == 73);
  assert(screen_calls == 1 && screen_was_null && screen_window == &self);
  return 0;
}
'''


def _compile_run(tmp_path: Path, update: str, helper: str, compiler: str) -> int:
    unit = tmp_path / "win_update_parent.cpp"
    exe = tmp_path / "win_update_parent"
    unit.write_text(SUPPORT + helper + "\n" + update + "\n" + MAIN,
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
def test_update_window_uses_parent_receiver_and_preserves_other_paths(
    tmp_path: Path, compiler: str,
) -> None:
    update, helper = _actual_definitions()
    assert _compile_run(tmp_path, update, helper, compiler) == 0


@pytest.mark.parametrize("compiler", _compilers())
def test_old_this_receiver_damage_fails_nonzero_offset_witness(
    tmp_path: Path, compiler: str,
) -> None:
    update, helper = _actual_definitions()
    old = "win_parent_->client_to_screen(&x, &y);"
    assert update.count(old) == 1
    damaged = update.replace(old, "client_to_screen(&x, &y);", 1)
    assert _compile_run(tmp_path, damaged, helper, compiler) != 0
