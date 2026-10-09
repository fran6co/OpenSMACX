import os
from pathlib import Path
import shutil
import subprocess
import tempfile


HERE = Path(__file__).resolve().parent
ROOT = next(parent for parent in HERE.parents if (parent / "src/caviar.cpp").is_file())
PRODUCT = Path(os.environ.get("OPENSMACX_CAVIAR_SOURCE", ROOT / "src/caviar.cpp"))


def source_statement(source, statement):
    start = source.index(statement)
    end = source.index(";", start) + 1
    return source[start:end]


def test_rect_accumulators_use_real_rect_storage_and_aliasing_calls():
    source = PRODUCT.read_text()
    assert "RECT CaviarViewportState;  // 0x009B9690" in source
    assert "RECT scene_rect_;        // 0x9BB438" in source
    assert "CaviarViewportState[" not in source
    assert "CaviarScene.field_00_" not in source
    assert source.count("UnionRect(&CaviarViewportState,") == 2
    assert "reinterpret_cast<RECT *>(&CaviarViewportState)" not in source
    assert "reinterpret_cast<RECT *>(&CaviarScene.field_00_)" not in source

    expected_clear = (
        "CaviarViewportState.left = 0;",
        "CaviarViewportState.right = 0;",
        "CaviarViewportState.bottom = 0;",
        "CaviarViewportState.top = 0;",
        "CaviarScene.scene_rect_.left = 0;",
        "CaviarScene.scene_rect_.right = 0;",
        "CaviarScene.scene_rect_.bottom = 0;",
        "CaviarScene.scene_rect_.top = 0;",
    )
    for function_start, function_end in (
        (source.index("int Caviar::init_class()"), source.index("// The six vx_* IO shims")),
        (source.index("void Caviar::clear_buffers("), source.index("Purpose: Render a scene and blit")),
    ):
        body = source[function_start:function_end]
        positions = [body.index(statement) for statement in expected_clear]
        assert positions == sorted(positions)

    viewport_call = source_statement(
        source, "UnionRect(&CaviarViewportState, &drawn, &CaviarViewportState)"
    )
    scene_call = source_statement(
        source, "UnionRect(&CaviarScene.scene_rect_, &scene,"
    )
    reset_start = source.index("            CaviarViewportState.left = 0;")
    reset_end = source.index("            CaviarScene.scene_rect_.top = 0;", reset_start)
    reset_end = source.index(";", reset_end) + 1
    reset = source[reset_start:reset_end]

    program = r'''#include <cassert>
#include <stdint.h>
typedef int32_t LONG;
struct RECT { LONG left, top, right, bottom; };
typedef char rect_is_four_signed_32_bit_words[sizeof(RECT) == 16 ? 1 : -1];
static RECT CaviarViewportState;
struct CaviarSceneRecord { RECT scene_rect_; };
static CaviarSceneRecord CaviarScene;
static const RECT *first_arg;
static const RECT *second_arg;
static RECT *dest_arg;
static int calls;
static int UnionRect(RECT *dest, const RECT *first, const RECT *second) {
    dest_arg = dest; first_arg = first; second_arg = second; ++calls;
    RECT a = *first;
    RECT b = *second;
    if (a.left >= a.right || a.top >= a.bottom) {
        if (b.left >= b.right || b.top >= b.bottom)
            return 0;
        *dest = b;
        return 1;
    }
    if (b.left >= b.right || b.top >= b.bottom) {
        *dest = a;
        return 1;
    }
    dest->left = a.left < b.left ? a.left : b.left;
    dest->top = a.top < b.top ? a.top : b.top;
    dest->right = a.right > b.right ? a.right : b.right;
    dest->bottom = a.bottom > b.bottom ? a.bottom : b.bottom;
    return 1;
}
static void viewport_union(RECT drawn) {
  /* viewport actual source call */
  VIEWPORT_CALL
}
static void scene_union(RECT scene) {
  /* scene actual source call */
  SCENE_CALL
}
static void clear_state() {
  RESET_BLOCK
}
int main() {
    clear_state();
    assert(CaviarViewportState.left == 0 && CaviarViewportState.top == 0 &&
           CaviarViewportState.right == 0 && CaviarViewportState.bottom == 0);
    assert(CaviarScene.scene_rect_.left == 0 && CaviarScene.scene_rect_.top == 0 &&
           CaviarScene.scene_rect_.right == 0 && CaviarScene.scene_rect_.bottom == 0);
    RECT negative = {-12, -7, -2, 5};
    viewport_union(negative);
        assert(dest_arg == &CaviarViewportState && second_arg == &CaviarViewportState);
        assert(first_arg != &CaviarViewportState);
    assert(CaviarViewportState.left == -12 && CaviarViewportState.top == -7 &&
           CaviarViewportState.right == -2 && CaviarViewportState.bottom == 5);
    RECT next = {-4, -20, 13, 9};
    viewport_union(next);
        assert(dest_arg == &CaviarViewportState && second_arg == &CaviarViewportState);
    assert(CaviarViewportState.left == -12 && CaviarViewportState.top == -20 &&
           CaviarViewportState.right == 13 && CaviarViewportState.bottom == 9);
    RECT scene_a = {-30, 4, -10, 20};
    RECT scene_b = {-8, -3, 5, 23};
    scene_union(scene_a);
        assert(dest_arg == &CaviarScene.scene_rect_ && second_arg == &CaviarScene.scene_rect_);
        assert(first_arg != &CaviarScene.scene_rect_);
    scene_union(scene_b);
    assert(CaviarScene.scene_rect_.left == -30 && CaviarScene.scene_rect_.top == -3 &&
           CaviarScene.scene_rect_.right == 5 && CaviarScene.scene_rect_.bottom == 23);
    RECT empty = {0, 0, 0, 0};
    viewport_union(empty);
    assert(CaviarViewportState.left == -12 && CaviarViewportState.top == -20 &&
           CaviarViewportState.right == 13 && CaviarViewportState.bottom == 9);
}
'''
    program = program.replace("VIEWPORT_CALL", viewport_call)
    program = program.replace("SCENE_CALL", scene_call)
    program = program.replace("RESET_BLOCK", reset)
    available = [shutil.which(name) for name in ("g++", "clang++")]
    available = [compiler for compiler in available if compiler]
    assert available, "GCC or Clang is required for extracted source controls"
    with tempfile.TemporaryDirectory(prefix="caviar-rect-source-") as directory:
        source_file = Path(directory) / "rect.cpp"
        source_file.write_text(program)
        for compiler in available:
            executable = Path(directory) / Path(compiler).name.replace("+", "p")
            subprocess.run(
                [compiler, "-std=c++98", "-Wall", "-Wextra", str(source_file), "-o", str(executable)],
                check=True, timeout=60,
            )
            subprocess.run([str(executable)], check=True, timeout=30)
