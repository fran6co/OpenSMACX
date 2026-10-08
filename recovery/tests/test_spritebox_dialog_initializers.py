"""Execute actual initializer wrappers with a real virtual Dialog base.

Host boundary doubles establish forwarding and sequencing, not VC6 layout,
original byte equality, or application runtime acceptance.
"""

from pathlib import Path
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]


def _body(source, signature):
    start = source.index(signature + " {")
    end = source.index("\n}", start) + 2
    return source[start:end]


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_initializers_forward_after_close(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    source = (ROOT / "src/spritebox.cpp").read_text()
    signatures = (
        "int SpriteBox::init(RECT* group_id, int flags)",
        "int SpriteBox::init(int file_name)",
        "int SpriteBox::init(Heap *file_name)",
        "int SpriteBox::init(int a1, int a2, int a3, int a4, Heap * a5)",
    )
    bodies = "\n".join(_body(source, signature) for signature in signatures)
    harness = r'''
#include <cassert>
struct RECT { int left, top, right, bottom; };
struct Heap {};
static int stage, expected_kind, expected_integer;
static RECT *expected_rect;
static Heap *expected_heap;
struct Dialog {
    int token;
    int check(int kind) {
        assert(stage == 1 && kind == expected_kind);
        assert(token == 97); // close modified this virtual base before init
        stage = 2;
        return -100 - kind;
    }
    int init(RECT *rect, int flags) {
        assert(rect == expected_rect && flags == expected_integer);
        return check(1);
    }
    int init(int value) {
        assert(value == expected_integer);
        return check(2);
    }
    int init(Heap *heap) {
        assert(heap == expected_heap);
        return check(3);
    }
    int init(int a, int b, int c, int d, Heap *heap) {
        assert(a == -7 && b == 0 && c == 23 && d == -91);
        assert(heap == expected_heap);
        return check(4);
    }
};
struct GraphicWin { int unrelated[17]; virtual ~GraphicWin() {} };
struct SpriteBox : virtual GraphicWin, virtual Dialog {
    void close() { assert(stage == 0); stage = 1; token = 97; }
    int init(RECT*, int);
    int init(int);
    int init(Heap*);
    int init(int,int,int,int,Heap*);
};
''' + bodies + r'''
int main() {
    SpriteBox box;
    RECT rect = {1,2,3,4}; Heap heap;
    expected_rect = &rect; expected_heap = &heap;
    for (int repeat = 0; repeat < 2; ++repeat) {
        stage = 0; expected_kind = 1; expected_integer = -123;
        assert(box.init(expected_rect, expected_integer) == -101 && stage == 2);
        stage = 0; expected_kind = 2; expected_integer = -456;
        assert(box.init(expected_integer) == -102 && stage == 2);
        stage = 0; expected_kind = 3;
        assert(box.init(expected_heap) == -103 && stage == 2);
        stage = 0; expected_kind = 4;
        assert(box.init(-7,0,23,-91,expected_heap) == -104 && stage == 2);
        expected_rect = 0; expected_heap = 0;
    }
}
'''
    path = tmp_path / "initializers.cpp"
    binary = tmp_path / "initializers"
    path.write_text(harness)
    subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
                    "-fsanitize=address,undefined", "-fno-omit-frame-pointer",
                    str(path), "-o", str(binary)], check=True)
    subprocess.run([str(binary)], check=True)
