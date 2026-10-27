"""Exercise draw_flash's actual flash-rectangle operation on host RECTs."""

from pathlib import Path
import re
import shutil
import subprocess


ROOT = Path(__file__).resolve().parents[2]


def _function_body(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[opening + 1:end]
    raise AssertionError(f"unterminated function: {signature}")


def test_draw_flash_unions_the_two_flash_rects_in_its_destination(tmp_path):
    source = (ROOT / "src/basewin.cpp").read_text(encoding="utf-8")
    body = _function_body(source, "void BaseWin::draw_flash(int redraw_mode)")
    match = re.search(
        r"^\s*((?:UnionRect|IntersectRect)\(&area, &flash_rect_a_, &flash_rect_b_\);)\s*$",
        body,
        re.MULTILINE,
    )
    assert match, "draw_flash must contain its actual flash rectangle operation"
    actual_statement = match.group(1)

    compiler = shutil.which("g++") or shutil.which("clang++")
    assert compiler, "host C++ compiler is required for the source control"
    unit = tmp_path / "draw_flash_rectangle.cpp"
    executable = tmp_path / "draw_flash_rectangle"
    unit.write_text(
        """#include <cassert>
struct RECT { int left, top, right, bottom; };
static int UnionRect(RECT *out, const RECT *a, const RECT *b) {
  out->left = a->left < b->left ? a->left : b->left;
  out->top = a->top < b->top ? a->top : b->top;
  out->right = a->right > b->right ? a->right : b->right;
  out->bottom = a->bottom > b->bottom ? a->bottom : b->bottom;
  return 1;
}
int IntersectRect(RECT *out, const RECT *a, const RECT *b) {
  out->left = a->left > b->left ? a->left : b->left;
  out->top = a->top > b->top ? a->top : b->top;
  out->right = a->right < b->right ? a->right : b->right;
  out->bottom = a->bottom < b->bottom ? a->bottom : b->bottom;
  if (out->left >= out->right || out->top >= out->bottom) {
    *out = RECT{0, 0, 0, 0};
    return 0;
  }
  return 1;
}
static RECT run() {
  RECT area{0, 0, 0, 0};
  RECT flash_rect_a_{0, 2, 4, 6};
  RECT flash_rect_b_{8, 10, 12, 14};
  """
        + actual_statement
        + """
  return area;
}
int main() {
  const RECT area = run();
  assert(area.left == 0 && area.top == 2);
  assert(area.right == 12 && area.bottom == 14);
}
""",
        encoding="utf-8",
    )
    subprocess.run(
        [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror", str(unit), "-o", str(executable)],
        check=True,
        capture_output=True,
        text=True,
    )
    subprocess.run([str(executable)], check=True, capture_output=True, text=True)
