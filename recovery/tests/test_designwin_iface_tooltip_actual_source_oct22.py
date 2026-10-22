"""Actual-source controls for the original DesignWin tooltip call order."""

from __future__ import annotations

import shutil
import subprocess
from pathlib import Path

import pytest
from match_recovery.source import read_file
from test_net_hotseat_owner_actual_source_oct6 import extract_definition


SIGNATURE = "void DesignWin::on_iface_right_click(int xCoord, int yCoord)"
ADDRESS = 0x0043D290
CATALOGUE_NAME = "?on_iface_right_click@DesignWin@@QAEXHH@Z"
EMITTED_SYMBOL = "?on_iface_right_click@DesignWin@@UAEXHH@Z"
IMAGE_SPANS = ((0x0043D290, 0x0043D327),)


def _root() -> Path:
    for parent in Path(__file__).resolve().parents:
        if (parent / "src/designwin.cpp").is_file():
            return parent
    raise AssertionError("cannot locate repository root")


SUPPORT = r'''#include <assert.h>
#include <stdint.h>

typedef char *LPSTR;
typedef const char *LPCSTR;
struct RECT { int left, top, right, bottom; };

enum { EV_RESET = 1, EV_CHECK, EV_LOOKUP, EV_CONVERT, EV_WIN_BUBBLE,
       EV_MAIN_BUBBLE };
struct Trace {
  int events[8];
  int count;
  int derived_bubbles;
  int lookups;
  int conversions;
  int base_bubbles;
  int checked_x, checked_y;
  LPSTR lookup_argument;
  LPSTR bubble_argument;
  RECT bubble_rect;
} trace;
static void event(int value) { trace.events[trace.count++] = value; }
static void reset_trace() {
  trace.count = trace.derived_bubbles = trace.lookups = 0;
  trace.conversions = trace.base_bubbles = 0;
  trace.checked_x = trace.checked_y = -999;
  trace.lookup_argument = trace.bubble_argument = 0;
  trace.bubble_rect.left = trace.bubble_rect.top =
      trace.bubble_rect.right = trace.bubble_rect.bottom = -999;
}

static LPSTR source_label = const_cast<LPSTR>("prototype label");
static LPSTR changed_label = const_cast<LPSTR>("mutated slot");
static LPSTR translated_label = const_cast<LPSTR>("translated label");
static LPSTR changed_translation = const_cast<LPSTR>("mutated translation");
static LPSTR current_translation = translated_label;
static LPSTR label_slots[0x33B];
struct LabelsState { LPSTR *strings_ptr; } Labels = { label_slots };

struct Strings {
  LPSTR get(LPSTR address) {
    event(EV_LOOKUP);
    ++trace.lookups;
    trace.lookup_argument = address;
    return current_translation;
  }
} StringTable;

struct Tutorial { void reset() { event(EV_RESET); } } TutorialWindow;
static int spot_result = 0;
static int spot_position = 0;
static int spot_type = 0;
struct Spot {
  int check(int x, int y, int *position, int *type) {
    event(EV_CHECK);
    trace.checked_x = x;
    trace.checked_y = y;
    *position = spot_position;
    *type = spot_type;
    return spot_result;
  }
};

struct Win {
  void set_bubble_text(char *text, RECT *rect);
};
struct GraphicWin : Win {};
struct MainInterface : GraphicWin {
  void client_to_screen(int *x, int *y) {
    event(EV_CONVERT);
    ++trace.conversions;
    assert(*x == 10 && *y == 20);
    *x += 100;
    *y += 200;
    // Sensitivity probe only: exercise retention if a converter changed label
    // state. This does not assert that the real conversion mutates label state.
    Labels.strings_ptr[0x33A] = changed_label;
    current_translation = changed_translation;
  }
  int set_bubble_text(uint32_t button_id, LPCSTR text) {
    event(EV_MAIN_BUBBLE);
    ++trace.derived_bubbles;
    assert(button_id == 0);
    (void)text;
    return 0;
  }
};
static MainInterface MainInterfaceGlobal;
void Win::set_bubble_text(char *text, RECT *rect) {
  event(EV_WIN_BUBBLE);
  ++trace.base_bubbles;
  trace.bubble_argument = text;
  trace.bubble_rect = *rect;
  assert(this == static_cast<Win *>(&MainInterfaceGlobal));
}
struct DesignWin {
  Spot spot1_;
  void on_iface_right_click(int xCoord, int yCoord);
};

''' 


MAIN = r'''int main() {
  reset_trace();
  label_slots[0x33A] = source_label;
  current_translation = translated_label;
  spot_result = 3;
  spot_position = 6;
  spot_type = 9;
  DesignWin win;
  win.on_iface_right_click(10, 20);
  assert(trace.count == 5);
  assert(trace.events[0] == EV_RESET && trace.events[1] == EV_CHECK &&
         trace.events[2] == EV_LOOKUP && trace.events[3] == EV_CONVERT &&
         trace.events[4] == EV_WIN_BUBBLE);
  assert(trace.checked_x == 10 && trace.checked_y == 20);
  assert(trace.lookups == 1 && trace.lookup_argument == source_label);
  assert(trace.conversions == 1 && trace.base_bubbles == 1 &&
         trace.derived_bubbles == 0);
  assert(trace.bubble_argument == translated_label);
  assert(trace.bubble_rect.left == 110 && trace.bubble_rect.right == 110 &&
         trace.bubble_rect.top == 220 && trace.bubble_rect.bottom == 220);

  reset_trace();
  spot_result = -1;
  spot_position = 4;
  label_slots[0x33A] = source_label;
  current_translation = translated_label;
  win.on_iface_right_click(10, 20);
  assert(trace.count == 2 && trace.events[0] == EV_RESET &&
         trace.events[1] == EV_CHECK);
  assert(trace.lookups == 0 && trace.conversions == 0 && trace.base_bubbles == 0);

  reset_trace();
  spot_result = 0;
  spot_position = -1;
  label_slots[0x33A] = source_label;
  current_translation = translated_label;
  win.on_iface_right_click(10, 20);
  assert(trace.count == 2 && trace.events[0] == EV_RESET &&
         trace.events[1] == EV_CHECK);
  assert(trace.lookups == 0 && trace.conversions == 0 && trace.base_bubbles == 0);
  return 0;
}
'''


def _compile_run(tmp_path: Path, body: str, compiler: str, *, pass_expected: bool) -> None:
    unit = tmp_path / "designwin_tooltip.cpp"
    exe = tmp_path / "designwin_tooltip"
    unit.write_text(SUPPORT + body + "\n" + MAIN, encoding="utf-8")
    built = subprocess.run([compiler, "-std=c++98", "-O0", str(unit), "-o", str(exe)],
                           text=True, capture_output=True, timeout=45)
    assert built.returncode == 0, built.stderr
    ran = subprocess.run([str(exe)], text=True, capture_output=True, timeout=15)
    if pass_expected:
        assert ran.returncode == 0, ran.stderr
    else:
        assert ran.returncode != 0, "behavioral damage mutant unexpectedly passed"


def _compilers() -> list[str]:
    values = [shutil.which("g++"), shutil.which("clang++")]
    values = [value for value in values if value]
    if not values:
        pytest.skip("host GCC or Clang unavailable")
    return values


def _actual_body() -> str:
    root = _root()
    path = root / "src/designwin.cpp"
    source = path.read_text(encoding="utf-8")
    rows = [row for row in read_file(path) if row.address == ADDRESS]
    assert len(rows) == 1, f"expected one native source identity at {ADDRESS:08X}"
    row = rows[0]
    assert row.name == CATALOGUE_NAME
    assert row.symbol == EMITTED_SYMBOL
    assert row.image_spans == IMAGE_SPANS
    assert Path(row.path).resolve() == path.resolve()
    body = extract_definition(source, SIGNATURE)
    assert body in row.region, "extracted actual definition must be inside mapped source region"
    return body


@pytest.mark.parametrize("compiler", _compilers())
def test_actual_tooltip_uses_original_order_guards_and_typed_win_receiver(
    tmp_path: Path, compiler: str,
) -> None:
    _compile_run(tmp_path, _actual_body(), compiler, pass_expected=True)


def _replace_once(body: str, old: str, new: str) -> str:
    assert body.count(old) == 1, (old, body.count(old))
    return body.replace(old, new, 1)


def _mutant(body: str, name: str) -> str:
    if name == "late-lookup":
        old = (
            "        LPSTR resolved_label = StringTable.get(label);\n"
            "        // The image swaps its two argument slots so the client_to_screen call\n"
            "        // receives (&xCoord, &yCoord); the anchor rect is that screen square.\n"
            "        MainInterfaceGlobal.client_to_screen(&xCoord, &yCoord);"
        )
        new = (
            "        // The image swaps its two argument slots so the client_to_screen call\n"
            "        // receives (&xCoord, &yCoord); the anchor rect is that screen square.\n"
            "        MainInterfaceGlobal.client_to_screen(&xCoord, &yCoord);\n"
            "        LPSTR resolved_label = StringTable.get(label);"
        )
        return _replace_once(body, old, new)
    if name == "raw-label":
        return _replace_once(body,
                             "MainInterfaceGlobal.Win::set_bubble_text(resolved_label, &rect);",
                             "MainInterfaceGlobal.Win::set_bubble_text(label, &rect);")
    if name == "swapped-coordinates":
        return _replace_once(body,
                             "MainInterfaceGlobal.client_to_screen(&xCoord, &yCoord);",
                             "MainInterfaceGlobal.client_to_screen(&yCoord, &xCoord);")
    if name == "negative-position-guard":
        return _replace_once(body, "position >= 0", "position > -2")
    if name == "failed-hit-guard":
        return _replace_once(body,
                             "spot1_.check(xCoord, yCoord, &position, &type) != -1",
                             "spot1_.check(xCoord, yCoord, &position, &type) != -2")
    if name == "hidden-derived-overload":
        return _replace_once(body,
                             "MainInterfaceGlobal.Win::set_bubble_text(resolved_label, &rect);",
                             "MainInterfaceGlobal.set_bubble_text(0, resolved_label);")
    raise AssertionError(name)


@pytest.mark.parametrize("compiler", _compilers())
@pytest.mark.parametrize("name", (
    "late-lookup", "raw-label", "swapped-coordinates", "negative-position-guard",
    "failed-hit-guard", "hidden-derived-overload",
))
def test_compiling_tooltip_damage_mutants_fail_actual_source_controls(
    tmp_path: Path, compiler: str, name: str,
) -> None:
    _compile_run(tmp_path, _mutant(_actual_body(), name), compiler, pass_expected=False)
