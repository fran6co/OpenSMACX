"""Actual-source host checks for Popup's two Dialogs scroll forwarders."""

import shutil
import subprocess
import tempfile
from pathlib import Path

import pytest

from recovery.tests.test_pact_ascent import ROOT, definition


HARNESS = r'''class GraphicWin {
public:
    int scrolled_calls;
    int scrolled_code;
    int scrolled_position;
    int scrolling_calls;
    int scrolling_code;
    int scrolling_position;

    GraphicWin()
        : scrolled_calls(0), scrolled_code(0), scrolled_position(0),
          scrolling_calls(0), scrolling_code(0), scrolling_position(0) {}

    void on_scrolled(int code, int position) {
        ++scrolled_calls;
        scrolled_code = code;
        scrolled_position = position;
    }

    void on_scrolling(int code, int position) {
        ++scrolling_calls;
        scrolling_code = code;
        scrolling_position = position;
    }
};

struct Dialogs : public GraphicWin {};

class Popup {
public:
    Dialogs dialogs_;
    void on_scrolled(int first, int second);
    void on_scrolling(int first, int second);
};

SCROLLED

SCROLLING

int main() {
    Popup popup;
    popup.on_scrolled(7, 11);
    popup.on_scrolling(13, 17);
    if (popup.dialogs_.scrolled_calls != 1 ||
        popup.dialogs_.scrolled_code != 7 ||
        popup.dialogs_.scrolled_position != 11)
        return 1;
    if (popup.dialogs_.scrolling_calls != 1 ||
        popup.dialogs_.scrolling_code != 13 ||
        popup.dialogs_.scrolling_position != 17)
        return 2;
    return 0;
}
'''


def _build_and_run(tmp_path, compiler, scrolled, scrolling, *, succeeds):
    source = (HARNESS.replace("SCROLLED", scrolled, 1)
              .replace("SCROLLING", scrolling, 1))
    cpp = tmp_path / "popup_cohort.cpp"
    executable = tmp_path / "popup_cohort"
    cpp.write_text(source)
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", "-pedantic",
         str(cpp), "-o", str(executable)],
        capture_output=True, text=True, timeout=45)
    assert built.returncode == 0, built.stdout + built.stderr
    result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=15)
    if succeeds:
        assert result.returncode == 0, result.stdout + result.stderr
    else:
        assert result.returncode != 0, "actual-source forwarding mutation escaped the host control"


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_popup_scroll_forwarders_and_damage_controls(compiler_name):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(compiler_name + " is unavailable")
    source = (ROOT / "src/popup.cpp").read_text(encoding="utf-8")
    scrolled = definition(source, r"void Popup::on_scrolled\(int first, int second\)\s*")
    scrolling = definition(source, r"void Popup::on_scrolling\(int first, int second\)\s*")

    with tempfile.TemporaryDirectory(prefix="popup-scroll-cohort-") as directory:
        tmp_path = Path(directory)
        _build_and_run(tmp_path, compiler, scrolled, scrolling, succeeds=True)

        damaged_scrolled = scrolled.replace(
            "dialogs_.on_scrolled(first, second);",
            "dialogs_.on_scrolled(first + 1, second);", 1)
        assert damaged_scrolled != scrolled
        _build_and_run(tmp_path, compiler, damaged_scrolled, scrolling, succeeds=False)

        damaged_scrolling = scrolling.replace(
            "dialogs_.on_scrolling(first, second);",
            "dialogs_.on_scrolling(first, second + 1);", 1)
        assert damaged_scrolling != scrolling
        _build_and_run(tmp_path, compiler, scrolled, damaged_scrolling, succeeds=False)
