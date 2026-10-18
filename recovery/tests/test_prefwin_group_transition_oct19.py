from __future__ import annotations

from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/prefwin.cpp").is_file())
SIGNATURE = "void PrefWin::on_group_clicked(int group, int previous, int selected)"


def actual_body() -> str:
    source = (ROOT / "src/prefwin.cpp").read_text(encoding="utf-8")
    assert source.count(SIGNATURE) == 1
    start = source.index(SIGNATURE)
    opening = source.index("{", start)
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
    raise AssertionError("on_group_clicked body is not brace-balanced")


HARNESS_PREFIX = r"""
#include <cassert>

struct CountedField {
    int value;
    int writes;
    explicit CountedField(int initial = 0) : value(initial), writes(0) {}
    operator int() const { return value; }
    CountedField &operator=(int next) { value = next; ++writes; return *this; }
};

class PrefWin {
public:
    CountedField field_A1C_;
    CountedField field_A18_;
    CountedField field_A14_;
    int callback_count;
    int callback_selected[3];
    int callback_flag[3];
    int callback_previous[3];
    int callback_selected_field[3];
    bool reenter;

    PrefWin() : field_A1C_(0), field_A18_(77), field_A14_(88),
        callback_count(0), reenter(false) {}
    void on_button_clicked(int selected);
    void on_group_clicked(int group, int previous, int selected);
};

void PrefWin::on_button_clicked(int selected) {
    const int event = callback_count++;
    callback_selected[event] = selected;
    callback_flag[event] = field_A1C_.value;
    callback_previous[event] = field_A18_.value;
    callback_selected_field[event] = field_A14_.value;
    if (reenter) {
        reenter = false;
        on_group_clicked(9, 5, 77);
    }
}
"""

HARNESS_MAIN = r"""
static void check_case(int initial_flag, int previous, int expected_a18_writes,
                       int expected_flag_writes) {
    PrefWin win;
    win.field_A1C_.value = initial_flag;
    win.field_A18_.value = 31;
    win.field_A14_.value = -8;
    win.on_group_clicked(123, previous, -19);
    assert(win.field_A1C_.writes == expected_flag_writes);
    assert(win.field_A18_.writes == expected_a18_writes);
    assert(win.field_A18_.value ==
           (expected_a18_writes ? previous : 31));
    assert(win.field_A14_.writes == 1 && win.field_A14_.value == -19);
    assert(win.callback_count == 1);
    assert(win.callback_selected[0] == -19);
    assert(win.callback_flag[0] == 0);
    assert(win.callback_previous[0] == win.field_A18_.value);
    assert(win.callback_selected_field[0] == -19);
}

int main() {
    // With a clear flag only the inclusive 0..5 range writes the previous
    // group. Invalid values, including arbitrary nonzero values, write neither
    // navigation field.
    check_case(0, -1, 0, 0);
    check_case(0, 0, 1, 0);
    check_case(0, 5, 1, 0);
    check_case(0, 6, 0, 0);
    check_case(0, 417, 0, 0);

    // Any nonzero pending flag is cleared; previous is ignored.
    check_case(1, 5, 0, 1);
    check_case(23, -77, 0, 1);

    // Callback observes the outer publication, then re-enters the real method.
    PrefWin win;
    win.field_A18_.value = 44;
    win.field_A14_.value = 55;
    win.reenter = true;
    win.on_group_clicked(0, 8, 12);
    assert(win.callback_count == 2);
    assert(win.callback_selected[0] == 12);
    assert(win.callback_flag[0] == 0);
    assert(win.callback_previous[0] == 44);
    assert(win.callback_selected_field[0] == 12);
    assert(win.callback_selected[1] == 77);
    assert(win.callback_flag[1] == 0);
    assert(win.callback_previous[1] == 5);
    assert(win.callback_selected_field[1] == 77);
    assert(win.field_A1C_.writes == 0);
    assert(win.field_A18_.writes == 1 && win.field_A18_.value == 5);
    assert(win.field_A14_.writes == 2 && win.field_A14_.value == 77);
}
"""


def compile_and_run(tmp_path: Path) -> subprocess.CompletedProcess[str]:
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("g++ unavailable")
    source = tmp_path / "prefwin_group_transition.cpp"
    binary = tmp_path / "prefwin_group_transition"
    source.write_text(HARNESS_PREFIX + actual_body() + HARNESS_MAIN,
                      encoding="utf-8")
    subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", str(source), "-o",
         str(binary)],
        check=True, capture_output=True, text=True, timeout=60,
    )
    return subprocess.run([str(binary)], capture_output=True, text=True,
                          timeout=30)


def test_actual_prefwin_transition_observes_original_field_writes_and_callback(
        tmp_path: Path) -> None:
    result = compile_and_run(tmp_path)
    assert result.returncode == 0, result.stderr
