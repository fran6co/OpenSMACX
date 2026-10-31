"""Actual-source controls for Dialogs click and ListBox-scroll dispatch."""
from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/IMAGE_PIN").is_file())
SOURCE = (ROOT / "src/dialogs.cpp").read_text(encoding="utf-8")


def extract_body(signature: str) -> str:
    start = SOURCE.index(signature)
    opening = SOURCE.index("{", start + len(signature))
    depth = 0
    for end in range(opening, len(SOURCE)):
        depth += (SOURCE[end] == "{") - (SOURCE[end] == "}")
        if depth == 0:
            return SOURCE[start:end + 1]
    raise AssertionError(f"unterminated body: {signature}")


CLICK = extract_body("void Dialogs::on_left_click(int a, int b)")
SCROLL = extract_body("Scroll *Dialogs::on_scroll_create()")

HARNESS = r'''#include <cassert>
#include <climits>
#include <cstdint>

struct Scroll {};
struct Win {
    Scroll *result;
    int calls;
    Win() : result(0), calls(0) {}
    Scroll *on_listbox_scroll_create() { ++calls; return result; }
};
struct SpriteBox {
    int calls, x, y;
    SpriteBox() : calls(0), x(0), y(0) {}
    void on_left_click(int a, int b) { ++calls; x = a; y = b; }
};
struct DialogsMock : SpriteBox {
    int kind_;
    uint32_t iFlags_;
    Win *win_parent_;
    DialogsMock() : kind_(0), iFlags_(0), win_parent_(0) {}
    void on_left_click(int a, int b);
    Scroll *on_scroll_create();
};
CLICK_BODY
SCROLL_BODY

int main() {
    DialogsMock dialog;
    for (int kind = 0; kind <= 16; ++kind) {
        dialog.kind_ = kind;
        dialog.calls = dialog.x = dialog.y = 0;
        dialog.on_left_click(-12345, 67890);
        assert(dialog.calls == (kind == 8 ? 1 : 0));
        if (kind == 8) assert(dialog.x == -12345 && dialog.y == 67890);
    }
    const int outside[] = {INT_MIN, INT_MIN + 1, -1, 17, INT_MAX - 1, INT_MAX};
    for (unsigned i = 0; i < sizeof(outside) / sizeof(outside[0]); ++i) {
        dialog.kind_ = outside[i];
        dialog.calls = 0;
        dialog.on_left_click(INT_MIN, INT_MAX);
        assert(dialog.calls == 0);
    }

    Win parent;
    Scroll expected;
    parent.result = &expected;
    dialog.win_parent_ = &parent;
    for (int kind = -2; kind <= 5; ++kind) {
        dialog.kind_ = kind;
        dialog.iFlags_ = 0x40;
        parent.calls = 0;
        Scroll *actual = dialog.on_scroll_create();
        assert(actual == (kind == 2 ? &expected : 0));
        assert(parent.calls == (kind == 2 ? 1 : 0));
    }
    dialog.kind_ = 2;
    dialog.iFlags_ = 0;
    parent.calls = 0;
    assert(dialog.on_scroll_create() == 0 && parent.calls == 0);
    dialog.iFlags_ = 0x40;
    dialog.win_parent_ = 0;
    assert(dialog.on_scroll_create() == 0);
}
'''


def compile_source(compiler: str, tmp_path: Path, click: str, scroll: str,
                   name: str) -> subprocess.CompletedProcess:
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(f"{compiler} is not installed")
    body = (click.replace("Dialogs::", "DialogsMock::")
            + "\n" + scroll.replace("Dialogs::", "DialogsMock::"))
    source = HARNESS.replace("CLICK_BODY\nSCROLL_BODY", body)
    cpp, binary = tmp_path / f"{name}.cpp", tmp_path / name
    cpp.write_text(source)
    subprocess.run([executable, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                    str(cpp), "-o", str(binary)], check=True, timeout=30)
    return subprocess.run([str(binary)], timeout=30)


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_dialogs_actual_source_dispatch_and_damage_controls(compiler, tmp_path):
    assert compile_source(compiler, tmp_path, CLICK, SCROLL, "actual").returncode == 0

    wrong_click = CLICK.replace("if (kind_ == 8)", "if (kind_ == 7)", 1)
    assert wrong_click != CLICK
    assert compile_source(compiler, tmp_path, wrong_click, SCROLL,
                          "wrong_click").returncode != 0

    wrong_scroll = SCROLL.replace("kind - 2u != 0", "kind - 3u != 0", 1)
    assert wrong_scroll != SCROLL
    assert compile_source(compiler, tmp_path, CLICK, wrong_scroll,
                          "wrong_scroll").returncode != 0
