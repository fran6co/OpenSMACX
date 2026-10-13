"""Actual vertex/edge declarations and setup body, including live input reloads."""
from pathlib import Path
import re
import shutil
import subprocess

import pytest

from test_texture_edge_cache_actual_source import _declaration, _definition
from texture_direct_storage_host import PRELUDE_MARKER, direct_storage_prelude


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
        old = "            tbl490 = TextureRasterEdgeVerticesA;\n            edge->deltaB"
        assert body.count(old) == 1
        body = body.replace(old, "            edge->deltaB", 1)
    elif mutation == "stale_geometry":
        old = "            tbl4f4 = TextureRasterEdgeVerticesB;\n            edge->accumC"
        assert body.count(old) == 1
        body = body.replace(old, "            edge->accumC", 1)
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
#include <string.h>
#define __cdecl
__DECLARATIONS__
static_assert(sizeof(Vert) == 8, "original coordinate stride");
static_assert(offsetof(Vert, x) == 0 && offsetof(Vert, y) == 4,
              "original signed coordinate pair");
static Vert uv_before[3] = {{2, 3}, {10, 11}, {18, 19}};
static Vert uv_after[3] = {{100, 200}, {20, 30}, {40, 50}};
static Vert geometry_before[3] = {{8, 0}, {1, 4}, {6, 8}};
static Vert geometry_after[3] = {{50, 0}, {70, 4}, {90, 8}};
static int light[3] = {100, 108, 116};
// TEXTURE_DIRECT_STORAGE_PRELUDE
static int divisions;
static int fixed_div(int numerator, int denominator) {
    ++divisions;
    if (divisions == 1) TextureRasterEdgeVerticesA = uv_after;
    if (divisions == 2) TextureRasterEdgeVerticesB = geometry_after;
    return static_cast<int>((static_cast<int64_t>(numerator) * 65536) /
                            denominator);
}
__ACTUAL_BODY__
int main() {
    TextureRasterEdgeVerticesA = uv_before;
    TextureRasterEdgeVerticesB = geometry_before;
    TextureRasterZValues = light;
    TextureRasterVertexCount = 3;
    TextureRasterMaxYIndex = 2;
    EdgeScan edge = {};
    edge.f0 = 1;
    assert(sub_620100(&edge, 0) == 1);
    assert(divisions == 3 && edge.counter == 4 && edge.id == 1);
    assert(edge.accumA == 2 * 65536 && edge.accumB == 3 * 65536);
    assert(edge.deltaA == 2 * 65536);
    assert(edge.deltaB == 27 * 65536 / 4);
    assert(edge.accumC == 50 && edge.deltaC == 5);
    assert(edge.accumD == 100 * 65536 && edge.deltaD == 2 * 65536);
    assert(TextureRasterEdgeVerticesA == uv_after &&
           TextureRasterEdgeVerticesB == geometry_after);
}
'''
    assert harness.count(PRELUDE_MARKER) == 1
    harness = harness.replace("__DECLARATIONS__", declarations).replace(
        PRELUDE_MARKER, direct_storage_prelude(source), 1).replace(
        "__ACTUAL_BODY__", body)
    cpp, binary = tmp_path / "live_vertices.cpp", tmp_path / "live_vertices"
    cpp.write_text(harness)
    compiled = subprocess.run(
        [executable, "-std=c++11", "-Wall", "-Wextra", "-Werror",
         "-Wno-unused-variable", "-Wno-unused-function", str(cpp),
         "-o", str(binary)], capture_output=True, text=True, timeout=30)
    assert compiled.returncode == 0, compiled.stdout + compiled.stderr
    result = subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)
    assert (result.returncode == 0) == (mutation is None), result.stderr


def test_remaining_integer_shape_is_not_vertex_pointer_punning():
    source = (ROOT / "src/texture.cpp").read_text()
    for name in ("draw_trans2", "sub_61c630", "sub_61d990", "sub_61b780",
                 "sub_61d090", "sub_61f0d0"):
        body = _definition(source, name + "(")
        assert "reinterpret_cast<Vert *>" not in body
        assert "reinterpret_cast<const Vert *>" not in body
    assert re.search(r"int Texture::draw_trans2\(Buffer \*a1, Vert \*a2, int a3\)", source)
