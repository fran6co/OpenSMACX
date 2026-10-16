"""Behavior controls for the distinct point and RECT nonclient transforms."""

from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/win.cpp"


def extract_body(text: str, signature: str) -> str:
    assert text.count(signature) == 1
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for index in range(opening, len(text)):
        if text[index] == "{":
            depth += 1
        elif text[index] == "}":
            depth -= 1
            if depth == 0:
                return text[start:index + 1]
    raise AssertionError("unterminated method body")


HARNESS = r'''
#include <cassert>

struct RECT { int left, top, right, bottom; };
struct Menu {
    int height;
    int calls;
    Menu(int h) : height(h), calls(0) {}
    int requested_height() { ++calls; return height; }
};
int ScrollDefaultThickness = 5;
struct Win {
    int iFlags_;
    int caption_height_;
    int border_thickness_;
    int bottom_border_thickness_;
    Menu *menu_;
    void nonclient_to_client(int *x, int *y);
    void nonclient_to_client(RECT *rect);
};
POINT_BODY
RECT_BODY

static void point_border_caption_and_menu_order() {
    Menu menu(4); Win win={0x41c, 3, 7, 2, &menu};
    int x=100, y=200;
    win.nonclient_to_client(&x, &y);
    assert(x==81 && y==186 && menu.calls==1);
}

static void rect_uses_opposite_caption_direction() {
    Menu menu(4); Win win={0x41c, 3, 7, 2, &menu};
    RECT rect={100,200,300,400};
    win.nonclient_to_client(&rect);
    assert(rect.left==107 && rect.top==207 && rect.right==288
           && rect.bottom==393 && menu.calls==1);
}

static void rect_bottom_sentinel_and_point_else_arm() {
    Win win={0x11, 2, 6, -1, 0};
    int x=20, y=30;
    win.nonclient_to_client(&x, &y);
    assert(x==8 && y==22);
    RECT rect={20,30,60,80};
    win.nonclient_to_client(&rect);
    assert(rect.left==26 && rect.top==32 && rect.right==54 && rect.bottom==74);
}

static void null_and_alias_contract() {
    Menu menu(13); Win win={0x1c, 4, 9, 3, &menu};
    int x=17, y=29;
    win.nonclient_to_client(0, &y);
    assert(x==17 && y==29 && menu.calls==0);
    win.nonclient_to_client(&x, 0);
    assert(x==17 && y==29 && menu.calls==0);
    win.iFlags_=0x0c; win.menu_=0; int aliased=50;
    win.nonclient_to_client(&aliased, &aliased);
    assert(aliased==40 && menu.calls==0);
    win.nonclient_to_client(static_cast<RECT *>(0));
    assert(menu.calls==0);
}

int main() {
    point_border_caption_and_menu_order();
    rect_uses_opposite_caption_direction();
    rect_bottom_sentinel_and_point_else_arm();
    null_and_alias_contract();
    return 0;
}
'''


def compile_run(tmp_path: Path, compiler: str, point: str, rect: str,
                suffix: str, succeeds: bool) -> None:
    source = tmp_path / f"{suffix}.cpp"
    executable = tmp_path / suffix
    source.write_text(HARNESS.replace("POINT_BODY", point).replace("RECT_BODY", rect),
                      encoding="utf-8")
    built = subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                            str(source), "-o", str(executable)],
                           text=True, capture_output=True, timeout=30)
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(executable)], text=True, capture_output=True, timeout=10)
    assert (ran.returncode == 0) == succeeds, ran.stdout + ran.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_overloads_preserve_distinct_caption_directions_and_quirks(
        tmp_path: Path, compiler_name: str) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    source = SOURCE.read_text(encoding="utf-8")
    point_signature = "void Win::nonclient_to_client(int * x, int * y)"
    rect_signature = "void Win::nonclient_to_client(RECT * rect)"
    point = extract_body(source, point_signature)
    rect = extract_body(source, rect_signature)
    compile_run(tmp_path, compiler, point, rect, f"actual-{compiler_name}", True)

    point_old = "*y += border_thickness_ - caption_height_;"
    point_new = "*y += caption_height_ - border_thickness_;"
    assert point.count(point_old) == 1
    compile_run(tmp_path, compiler, point.replace(point_old, point_new, 1), rect,
                f"point-reversed-{compiler_name}", False)

    rect_old = "rect->top = rect->top + (caption_height_ - border_thickness_);"
    rect_new = "rect->top = rect->top + (border_thickness_ - caption_height_);"
    assert rect.count(rect_old) == 1
    compile_run(tmp_path, compiler, point, rect.replace(rect_old, rect_new, 1),
                f"rect-reversed-{compiler_name}", False)
