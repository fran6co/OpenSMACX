"""Actual-source controls for the proposed typed MapWin nonclient click path.

The host fixture tests behavior, not native VC6 layout or ABI.
The complete installed handler is extracted without rewriting its body.
"""

from pathlib import Path
import os
import shutil
import subprocess
import pytest


ROOT = Path(__file__).resolve().parents[2]


def extract_body(source: str, signature: str) -> str:
    start = source.index(signature)
    brace = source.index("{", start)
    depth = 0
    for end in range(brace, len(source)):
        if source[end] == "{":
            depth += 1
        elif source[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start : end + 1]
    raise AssertionError("unterminated actual source function")


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_nc_left_down_uses_typed_geometry_and_virtual_click(tmp_path, compiler_name):
    source = (ROOT / "src/mapwin.cpp").read_text()
    body = extract_body(source, "void MapWin::on_nc_left_down(")

    assert "client_rect()" in body
    assert "on_button_clicked(" in body
    for raw in ("entered_receiver", "VCallI", "slot044", "reinterpret_cast"):
        assert raw not in body

    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")

    fixture = r"""
#include <cassert>
#include <vector>
struct RECT { int left, top, right, bottom; };
static std::vector<RECT> tested;
int in_box(int x, int y, const RECT *r) {
    tested.push_back(*r);
    return x >= r->left && x < r->right && y >= r->top && y < r->bottom;
}
class Win {
    RECT client_;
public:
    Win() { client_.left = 37; client_.top = 0; client_.right = 217; client_.bottom = 200; }
    const RECT &client_rect() const { return client_; }
    virtual void on_button_clicked(int) {}
    virtual ~Win() {}
};
class GraphicWin : public Win {};
class MapWin : public virtual GraphicWin {
public:
    void on_nc_left_down(int, int);
    virtual void on_button_clicked(int);
};
void MapWin::on_button_clicked(int) {}
"""
    observer = r"""
class Observed : public MapWin {
public:
    std::vector<int> clicks;
    virtual void on_button_clicked(int id) { clicks.push_back(id); }
};
"""
    main = r"""
static void expect_hit(int x, int expected, int expected_checks) {
    Observed map;
    tested.clear();
    map.on_nc_left_down(x, 15);
    assert(map.clicks.size() == 1);
    assert(map.clicks[0] == expected);
    assert((int)tested.size() == expected_checks);
}
int main() {
    // width=180: origin=180-0x5b=89; each hit strip is [14,31).
    expect_hit(90, 3, 1);
    expect_hit(111, 2, 2);
    expect_hit(132, 1, 3);
    expect_hit(153, 0, 4);

    Observed outside;
    tested.clear();
    outside.on_nc_left_down(170, 15);
    assert(outside.clicks.empty() && tested.size() == 4);

    // In-box right and bottom edges are exclusive, as in Win::in_box.
    Observed right_edge;
    tested.clear();
    right_edge.on_nc_left_down(106, 15);
    assert(right_edge.clicks.empty() && tested.size() == 4);
    Observed bottom_edge;
    tested.clear();
    bottom_edge.on_nc_left_down(90, 31);
    assert(bottom_edge.clicks.empty() && tested.size() == 4);

    // The recorded rectangles preserve all source increments and order.
    assert(tested[0].left == 89 && tested[0].right == 106);
    assert(tested[0].top == 14 && tested[0].bottom == 31);
    assert(tested[1].left == 110 && tested[1].right == 127);
    assert(tested[2].left == 131 && tested[2].right == 148);
    assert(tested[3].left == 152 && tested[3].right == 169);
}
"""
    translation_unit = fixture + body + observer + main
    cpp = tmp_path / "mapwin-nc-left-down.cpp"
    exe = tmp_path / "mapwin-nc-left-down"
    cpp.write_text(translation_unit)
    subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
         "-fsanitize=address,undefined", "-fno-sanitize-recover=all", str(cpp), "-o", str(exe)],
        check=True,
        cwd=ROOT,
    )
    subprocess.run([str(exe)], check=True, cwd=ROOT,
                   env=dict(os.environ, ASAN_OPTIONS="detect_leaks=0"))
