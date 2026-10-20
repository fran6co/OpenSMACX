"""Exercise AlphaMenu's mapped rectangle snapshot and output-store order."""
from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
OWNER = ROOT / "src" / "alphamenu.cpp"


def _definition(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
    raise AssertionError("unterminated mapped method")


FIXTURE = r"""
#include <cassert>
#include <cstring>

struct RECT { int left, top, right, bottom; };

struct Spot {
    RECT rows[8];
    mutable int calls;
    mutable int last_index;

    const RECT &get_rect_at(int index) const {
        assert(index >= 0 && index < 8);
        ++calls;
        last_index = index;
        return rows[index];
    }
};

struct AlphaMenu {
    int field_A24_;
    RECT outer_rect_;
    int caption_height_;
    int border_thickness_;
    Spot spot_;

    void on_adjust_pulldown_pos(int *x, int *y);
};

// @@BODY@@

static void initialize(AlphaMenu *menu) {
    menu->field_A24_ = 1;
    menu->outer_rect_.left = 4;
    menu->outer_rect_.top = 17;
    menu->outer_rect_.right = 2;
    menu->outer_rect_.bottom = 31;
    menu->caption_height_ = 99;
    menu->border_thickness_ = 3;
    menu->spot_.calls = 0;
    menu->spot_.last_index = -1;
    for (int i = 0; i < 8; ++i) {
        menu->spot_.rows[i].left = 100 + i;
        menu->spot_.rows[i].top = 5 + i * 2;
        menu->spot_.rows[i].right = 200 + i;
        menu->spot_.rows[i].bottom = 300 + i;
    }
}

int main(int argc, char **argv) {
    assert(argc == 2);
    AlphaMenu menu;
    initialize(&menu);

    if (std::strcmp(argv[1], "normal") == 0) {
        int x = -10, y = -20;
        menu.on_adjust_pulldown_pos(&x, &y);
        assert(x == 2 && y == 27);
        assert(menu.outer_rect_.top == 17);
        assert(menu.spot_.calls == 1 && menu.spot_.last_index == 1);
    } else if (std::strcmp(argv[1], "top_alias") == 0) {
        int y = -20;
        menu.on_adjust_pulldown_pos(&menu.outer_rect_.top, &y);
        assert(menu.outer_rect_.top == 2);
        assert(y == 27); // uses the pre-store top value 17
        assert(menu.spot_.calls == 1 && menu.spot_.last_index == 1);
    } else if (std::strcmp(argv[1], "field_alias") == 0) {
        menu.outer_rect_.right = 3;
        int y = -20;
        menu.on_adjust_pulldown_pos(&menu.field_A24_, &y);
        assert(menu.field_A24_ == 3);
        assert(y == 31); // live index 3 is read after the x store
        assert(menu.spot_.calls == 1 && menu.spot_.last_index == 3);
    } else if (std::strcmp(argv[1], "same_output") == 0) {
        int output = -1;
        menu.on_adjust_pulldown_pos(&output, &output);
        assert(output == 27); // the later y store wins
        assert(menu.spot_.calls == 1 && menu.spot_.last_index == 1);
    } else {
        assert(std::strcmp(argv[1], "sentinel") == 0);
        menu.field_A24_ = -1;
        int y = -20;
        menu.on_adjust_pulldown_pos(&menu.outer_rect_.top, &y);
        assert(menu.outer_rect_.top == 17 && y == -20);
        assert(menu.spot_.calls == 0 && menu.spot_.last_index == -1);
    }
}
"""


def _compile(compiler: str, tmp_path: Path, name: str, body: str) -> Path:
    program = FIXTURE.replace("// @@BODY@@", body)
    source = tmp_path / (name + ".cpp")
    binary = tmp_path / name
    source.write_text(program)
    built = subprocess.run(
        [compiler, "-std=c++98", str(source), "-o", str(binary)],
        capture_output=True,
        text=True,
        timeout=45,
    )
    assert built.returncode == 0, name + built.stdout + built.stderr
    return binary


def _run(binary: Path, case: str) -> subprocess.CompletedProcess:
    return subprocess.run([str(binary), case], capture_output=True, text=True,
                          timeout=15)


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_alphamenu_snapshot_alias_contract_and_mutants(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")

    source = OWNER.read_text()
    body = _definition(source, "void AlphaMenu::on_adjust_pulldown_pos(int *x, int *y)")
    snapshot = "const RECT outer_rect_snapshot = outer_rect_;"
    x_store = "*x = outer_rect_snapshot.right;"
    y_store = "*y = spot_.get_rect_at(field_A24_).top + border_thickness_"
    assert body.count(snapshot) == body.count(x_store) == body.count(y_store) == 1
    assert body.index(snapshot) < body.index(x_store) < body.index(y_store)

    actual = _compile(compiler, tmp_path, "actual", body)
    for case in ("normal", "top_alias", "field_alias", "same_output", "sentinel"):
        result = _run(actual, case)
        assert result.returncode == 0, case + result.stdout + result.stderr

    live_top = body.replace("outer_rect_snapshot.top", "outer_rect_.top", 1)
    early_spot = body.replace(
        "        *x = outer_rect_snapshot.right;",
        "        const int saved_index = field_A24_;\n"
        "        *x = outer_rect_snapshot.right;",
        1,
    ).replace("get_rect_at(field_A24_).top", "get_rect_at(saved_index).top", 1)
    reversed_stores = body.replace(
        "        *x = outer_rect_snapshot.right;\n"
        "        *y = spot_.get_rect_at(field_A24_).top + border_thickness_\n"
        "            + outer_rect_snapshot.top;",
        "        *y = spot_.get_rect_at(field_A24_).top + border_thickness_\n"
        "            + outer_rect_snapshot.top;\n"
        "        *x = outer_rect_snapshot.right;",
        1,
    )
    assert live_top != body and early_spot != body and reversed_stores != body

    wrong_field = body.replace("border_thickness_", "caption_height_", 1)
    assert wrong_field != body

    mutants = (
        ("wrong_field", wrong_field, "normal"),
        ("live_top", live_top, "top_alias"),
        ("early_spot", early_spot, "field_alias"),
        ("reversed_stores", reversed_stores, "same_output"),
    )
    for name, mutant, case in mutants:
        binary = _compile(compiler, tmp_path, name, mutant)
        result = _run(binary, case)
        assert result.returncode != 0, name + " mutant survived " + case
