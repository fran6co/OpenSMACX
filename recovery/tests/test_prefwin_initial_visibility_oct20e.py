"""Focused host control for PrefWin's initial child-visibility sequence.

The harness extracts the real member arrays and hide loops from init(); it
checks those effects and order only, not the full initializer or native ABI.
"""

from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/prefwin.cpp"


def extract_function(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        if source[end] == "{":
            depth += 1
        elif source[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start:end + 1]
    raise AssertionError(f"unterminated production function: {signature}")


def extract_statement(source: str, anchor: str) -> str:
    start = source.index(anchor)
    end = source.index(";", start) + 1
    return source[start:end]


def extract_single_line_loop(source: str, anchor: str) -> str:
    start = source.index(anchor)
    closing_paren = source.index(")", start)
    end = source.index(";", closing_paren) + 1
    return source[start:end]


def visibility_harness(*, omit_map=False, reverse_hide_order=False) -> str:
    init = extract_function(SOURCE.read_text(), "void PrefWin::init()")
    scroll_array = extract_statement(init, "Scroll *scrolls[]")
    box_array = extract_statement(init, "CheckBox *boxes[]")
    scroll_hide = extract_single_line_loop(init, "for (int scroll_hide_index = 0;")
    box_hide = extract_single_line_loop(init, "for (int box_hide_index = 0;")
    if omit_map:
        expected = "box_hide_index < 7"
        assert expected in box_hide
        box_hide = box_hide.replace(expected, "box_hide_index < 6", 1)
    hide_loops = [
        (init.index("for (int scroll_hide_index = 0;"), scroll_hide),
        (init.index("for (int box_hide_index = 0;"), box_hide),
    ]
    hide_loops.sort(key=lambda item: item[0], reverse=reverse_hide_order)

    return (r'''#include <cassert>
#include <vector>
std::vector<int> events;
struct Scroll {
  int id;
  explicit Scroll(int value) : id(value) {}
  void hide() { events.push_back(id); }
};
struct CheckBox {
  int id;
  explicit CheckBox(int value) : id(value) {}
  void hide() { events.push_back(id); }
};
struct PrefWin {
  Scroll volumeMainScroll_, volumeVoiceScroll_, volumeSFXScroll_, volumeMusicScroll_;
  CheckBox bscPrefCheckBox_, advPrefCheckBox_, autoPrefCheckBox_;
  CheckBox audioVisPrefCheckBox_, warnPrefCheckBox_, warnContPrefCheckBox_;
  CheckBox mapPrefCheckBox_;
  PrefWin()
      : volumeMainScroll_(100), volumeVoiceScroll_(101), volumeSFXScroll_(102),
        volumeMusicScroll_(103), bscPrefCheckBox_(200), advPrefCheckBox_(201),
        autoPrefCheckBox_(202), audioVisPrefCheckBox_(203), warnPrefCheckBox_(204),
        warnContPrefCheckBox_(205), mapPrefCheckBox_(206) {}
  void run_visibility_sequence() {
    ARRAY_SCROLLS
    ARRAY_BOXES
    FIRST_HIDE_LOOP
    SECOND_HIDE_LOOP
  }
};
int main() {
  PrefWin window;
  window.run_visibility_sequence();
  const int expected_values[] = {100, 101, 102, 103, 200, 201, 202, 203, 204, 205, 206};
  assert(events.size() == sizeof(expected_values) / sizeof(expected_values[0]));
  for (unsigned int i = 0; i < events.size(); ++i)
    assert(events[i] == expected_values[i]);
}
'''.replace("ARRAY_SCROLLS", scroll_array).replace("ARRAY_BOXES", box_array)
            .replace("FIRST_HIDE_LOOP", hide_loops[0][1])
            .replace("SECOND_HIDE_LOOP", hide_loops[1][1]))


def compile_and_run(tmp_path, compiler: str, source: str, stem: str):
    cpp = tmp_path / f"{stem}.cpp"
    binary = tmp_path / stem
    cpp.write_text(source)
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(binary)],
        capture_output=True, text=True, timeout=60,
    )
    assert built.returncode == 0, built.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True, timeout=30)


COMPILERS = [pytest.param(path, id=Path(path).name)
             for name in ("g++", "clang++") if (path := shutil.which(name))]


@pytest.mark.parametrize("compiler", COMPILERS)
def test_actual_prefwin_init_hides_four_scrolls_then_all_seven_checkboxes(
    tmp_path, compiler
):
    result = compile_and_run(
        tmp_path, compiler, visibility_harness(), "prefwin-initial-visibility"
    )
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("compiler", COMPILERS)
def test_six_checkbox_hide_mutant_is_rejected(tmp_path, compiler):
    result = compile_and_run(
        tmp_path, compiler, visibility_harness(omit_map=True),
        "prefwin-initial-visibility-six-only",
    )
    assert result.returncode != 0


@pytest.mark.parametrize("compiler", COMPILERS)
def test_reversed_scroll_and_checkbox_hide_order_mutant_is_rejected(tmp_path, compiler):
    result = compile_and_run(
        tmp_path, compiler, visibility_harness(reverse_hide_order=True),
        "prefwin-initial-visibility-reversed-order",
    )
    assert result.returncode != 0
