"""Actual vertex/edge declarations and setup body, including live input reloads."""
from pathlib import Path
import re
import shutil
import subprocess

import pytest

from test_texture_edge_cache_actual_source import _declaration, _definition


ROOT = Path(__file__).resolve().parents[2]


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
@pytest.mark.parametrize("mutation", (None, "stale_uv", "stale_geometry", "wrong_axis"))
def test_typed_context_retains_live_reload_order(tmp_path, compiler, mutation):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(compiler + " unavailable")
    source = (ROOT / "src/texture.cpp").read_text()
    body = _definition(source, "int __cdecl sub_620100(")
    if mutation == "stale_uv":
        assert body.count("            tbl490 = *g2_009bb490;") == 2
        body = body.replace("            tbl490 = *g2_009bb490;", "")
    elif mutation == "stale_geometry":
        assert body.count("            tbl4f4 = *g2_009bb4f4;") == 2
        body = body.replace("            tbl4f4 = *g2_009bb4f4;", "")
    elif mutation == "wrong_axis":
        assert "tbl490[idx].y" in body
        body = body.replace("tbl490[idx].y", "tbl490[idx].x", 1)
    declarations = "\n".join((
        _declaration((ROOT / "src/vert.h").read_text(), "struct", "Vert"),
        _declaration((ROOT / "src/texture.h").read_text(), "struct", "EdgeScan"),
    ))
    harness = r'''
#include <cassert>
#include <cstddef>
#include <stdint.h>
#define __cdecl
__DECLARATIONS__
static_assert(sizeof(Vert) == 8, "original coordinate stride");
static_assert(offsetof(Vert, x) == 0 && offsetof(Vert, y) == 4,
              "original signed coordinate pair");
static Vert uv_before[3] = {{2, 3}, {10, 11}, {18, 19}};
static Vert uv_after[3] = {{100, 200}, {20, 30}, {40, 50}};
static Vert geometry_before[3] = {{8, 0}, {1, 4}, {6, 8}};
static Vert geometry_after[3] = {{50, 0}, {70, 4}, {90, 8}};
static Vert *uv = uv_before, *geometry = geometry_before;
static int light[3] = {100, 108, 116};
static int *light_values = light;
static Vert **const g2_009bb490 = &uv;
static Vert **const g2_009bb4f4 = &geometry;
static int **const g2_009bb4fc = &light_values;
static int count = 3, max_y = 2;
static int *const blit_vertex_count = &count;
static int *const blit_max_y_index = &max_y;
static int divisions;
static int fixed_div(int numerator, int denominator) {
    ++divisions;
    if (divisions == 1) uv = uv_after;
    if (divisions == 2) geometry = geometry_after;
    return static_cast<int>((static_cast<int64_t>(numerator) * 65536) /
                            denominator);
}
__ACTUAL_BODY__
int main() {
    EdgeScan edge = {};
    edge.f0 = 1;
    assert(sub_620100(&edge, 0) == 1);
    assert(divisions == 3 && edge.counter == 4 && edge.id == 1);
    assert(edge.accumA == 2 * 65536 && edge.accumB == 3 * 65536);
    assert(edge.deltaA == 2 * 65536);
    assert(edge.deltaB == 27 * 65536 / 4);
    assert(edge.accumC == 50 && edge.deltaC == 5);
    assert(edge.accumD == 100 * 65536 && edge.deltaD == 2 * 65536);
    assert(uv == uv_after && geometry == geometry_after);
}
'''
    harness = harness.replace("__DECLARATIONS__", declarations).replace(
        "__ACTUAL_BODY__", body)
    cpp, binary = tmp_path / "live_vertices.cpp", tmp_path / "live_vertices"
    cpp.write_text(harness)
    compiled = subprocess.run(
        [executable, "-std=c++11", "-Wall", "-Wextra", "-Werror", str(cpp),
         "-o", str(binary)], capture_output=True, text=True, timeout=30)
    assert compiled.returncode == 0, compiled.stdout + compiled.stderr
    result = subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)
    assert (result.returncode == 0) == (mutation is None), result.stderr


def test_unresolved_producer_boundaries_keep_original_signatures():
    source = (ROOT / "src/texture.cpp").read_text()
    expected = {
        "draw_trans2": ("a2",),
        "sub_61c630": ("verts", "unusedIn"),
        "sub_61d990": ("vertices",),
        "sub_61b780": ("vertices",),
        "sub_620a10": ("vertices_arg", "edge_vertices_arg"),
        "sub_61d090": ("vertices",),
        "sub_61f0d0": ("param_3",),
    }
    for name, arguments in expected.items():
        body = _definition(source, name + "(")
        for argument in arguments:
            assert "reinterpret_cast<Vert *>(" + argument + ")" in body
    assert re.search(r"int Texture::draw_trans2\(Buffer \*a1, int \*a2, int a3\)", source)
