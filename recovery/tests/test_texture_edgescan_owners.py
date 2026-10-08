"""Actual-source host controls for the typed EdgeScan setup helpers.

The helper definitions and EdgeScan declaration are extracted from product
source. The only behavioral bridge is the test fixture's fixed_div multiply,
which avoids C++ UB from left-shifting a negative signed numerator while
preserving the pinned helper's signed scaled-divide result on these inputs.
This does not establish VC6 codegen or full rasterizer behavior.
"""
from pathlib import Path
import re
import shutil
import subprocess

import pytest
from match_recovery.includes import COMMENTS_OR_STRINGS

ROOT = Path(__file__).resolve().parents[2]
CPP_SOURCE = ROOT / "src/texture.cpp"
HEADER_SOURCE = ROOT / "src/texture.h"


def masked(source):
    return COMMENTS_OR_STRINGS.sub(
        lambda match: re.sub(r"[^\n]", " ", match[0]), source
    )


def extract_braced(source, signature):
    view = masked(source)
    starts = [match.start() for match in re.finditer(re.escape(signature), view)]
    results = []
    for start in starts:
        opening = view.find("{", start)
        if opening < 0:
            continue
        depth = 0
        for index in range(opening, len(view)):
            if view[index] == "{":
                depth += 1
            elif view[index] == "}":
                depth -= 1
                if depth == 0:
                    results.append(source[start:index + 1])
                    break
    assert len(results) == 1, signature
    return results[0]


def extract_edgescan_declaration(header):
    view = masked(header)
    match = re.search(r"\bstruct\s+EdgeScan\s*\{", view)
    assert match, "actual EdgeScan declaration absent"
    opening = view.find("{", match.start())
    depth = 0
    for index in range(opening, len(view)):
        if view[index] == "{":
            depth += 1
        elif view[index] == "}":
            depth -= 1
            if depth == 0:
                end = view.find(";", index)
                assert end >= 0
                return header[match.start():end + 1]
    raise AssertionError("unterminated EdgeScan declaration")


FIXTURE = r'''#include <assert.h>
#include <stdint.h>

#ifndef __cdecl
#define __cdecl
#endif

// ACTUAL_EDGESCAN_DECLARATION

static_assert(sizeof(EdgeScan) == 0x3c, "actual EdgeScan stride");
class Texture {
public:
    static int setup_edge_gouraud(EdgeScan *, int);
    static int setup_edge(EdgeScan *, int);
};

static int max_y_index;
static int vertex_count;
static int *uv_a_data;
static int *uv_b_data;
static int *z_data;
int *blit_max_y_index = &max_y_index;
int *blit_vertex_count = &vertex_count;
int **blit_edge_uvs_a = &uv_a_data;
int **blit_edge_uvs_b = &uv_b_data;
int **blit_z_values = &z_data;

// Host-only spelling of fixed_div's signed 16.16 quotient. Multiplication
// avoids UB from shifting a negative signed numerator; product source remains
// untouched. Test inputs stay within the original helper's valid int domain.
int fixed_div(int numerator, int denominator) {
    return static_cast<int>((static_cast<int64_t>(numerator) * 65536) /
                            denominator);
}

// ACTUAL_EDGE_SCAN_METHODS

static void set_sentinels(EdgeScan &edge) {
    edge.f0 = 1;
    edge.counter = 101;
    edge.id = 102;
    edge.accumA = 103;
    edge.accumB = 104;
    edge.deltaA = 105;
    edge.deltaB = 106;
    edge.accumC = 107;
    edge.deltaC = 108;
    edge.extraDeltaC = 109;
    edge.fracAccum = 110;
    edge.fracDelta = 111;
    edge.fracSub = 112;
    edge.accumD = 113;
    edge.deltaD = 114;
}
static void assert_non_counter_fields_unchanged(const EdgeScan &edge) {
    assert(edge.f0 == 1 && edge.id == 102);
    assert(edge.accumA == 103 && edge.accumB == 104);
    assert(edge.deltaA == 105 && edge.deltaB == 106);
    assert(edge.accumC == 107 && edge.deltaC == 108);
    assert(edge.extraDeltaC == 109 && edge.fracAccum == 110);
    assert(edge.fracDelta == 111 && edge.fracSub == 112);
    assert(edge.accumD == 113 && edge.deltaD == 114);
}

int main() {
    int uv_a[6] = {1, 2, 5, 6, 9, 10};
    // [z0,y0,z1,y1,z2,y2]: 4-row edge with positive depth delta 21.
    int uv_b[6] = {10, 0, 31, 4, 50, 8};
    int z[3] = {100, 108, 116};
    uv_a_data = uv_a;
    uv_b_data = uv_b;
    z_data = z;
    vertex_count = 3;
    max_y_index = 2;

    EdgeScan edge;
    set_sentinels(edge);
    assert(Texture::setup_edge_gouraud(&edge, 0) == 1);
    assert(edge.counter == 4 && edge.id == 1);
    assert(edge.accumA == 1 * 0x10000 + 0x8000);
    assert(edge.accumB == 2 * 0x10000 + 0x8000);
    assert(edge.deltaA == fixed_div(5 * 0x10000 - edge.accumA,
                                    4 * 0x10000));
    assert(edge.deltaB == fixed_div(6 * 0x10000 - edge.accumB,
                                    4 * 0x10000));
    assert(edge.accumC == 10 && edge.deltaC == 5);
    assert(edge.extraDeltaC == 1 && edge.fracAccum == 0);
    assert(edge.fracSub == 4 && edge.fracDelta == 1);
    assert(edge.accumD == 100 * 0x10000);
    assert(edge.deltaD == fixed_div(8 * 0x10000, 4 * 0x10000));

    // Negative depth keeps signed quotient/correction and positive magnitude
    // remainder; the fraction fields must not be normalized to signed mod.
    uv_b[0] = 31;
    uv_b[2] = 10;
    set_sentinels(edge);
    assert(Texture::setup_edge_gouraud(&edge, 0) == 1);
    assert(edge.accumC == 31 && edge.deltaC == -5);
    assert(edge.extraDeltaC == -1 && edge.fracAccum == -3);
    assert(edge.fracSub == 4 && edge.fracDelta == 1);

    // Plain edge uses no half-pixel bias and leaves the D-channel words
    // untouched even on a successful setup.
    uv_b[0] = 10;
    uv_b[2] = 31;
    set_sentinels(edge);
    assert(Texture::setup_edge(&edge, 0) == 1);
    assert(edge.counter == 4 && edge.id == 1);
    assert(edge.accumA == 1 * 0x10000 && edge.accumB == 2 * 0x10000);
    assert(edge.deltaA == fixed_div((5 - 1) * 0x10000, 4 * 0x10000));
    assert(edge.deltaB == fixed_div((6 - 2) * 0x10000, 4 * 0x10000));
    assert(edge.accumC == 10 && edge.deltaC == 5);
    assert(edge.extraDeltaC == 1 && edge.fracAccum == 0);
    assert(edge.fracSub == 4 && edge.fracDelta == 1);
    assert(edge.accumD == 113 && edge.deltaD == 114);

    uv_b[0] = 31;
    uv_b[2] = 10;
    set_sentinels(edge);
    assert(Texture::setup_edge(&edge, 0) == 1);
    assert(edge.accumC == 31 && edge.deltaC == -5);
    assert(edge.extraDeltaC == -1 && edge.fracAccum == -3);
    assert(edge.fracSub == 4 && edge.fracDelta == 1);
    assert(edge.accumD == 113 && edge.deltaD == 114);

    // Starting at the max-y vertex returns before touching any field.
    max_y_index = 0;
    set_sentinels(edge);
    assert(Texture::setup_edge_gouraud(&edge, 0) == 0);
    assert_non_counter_fields_unchanged(edge);
    assert(edge.counter == 101);
    assert(Texture::setup_edge(&edge, 0) == 0);
    assert_non_counter_fields_unchanged(edge);
    assert(edge.counter == 101);

    // A zero-height walk visits the next vertices until max-y, writes only
    // counter, and leaves every other field untouched in both helpers.
    max_y_index = 2;
    uv_b[1] = uv_b[3] = uv_b[5] = 0;
    set_sentinels(edge);
    assert(Texture::setup_edge(&edge, 0) == 0);
    assert(edge.counter == 0);
    assert_non_counter_fields_unchanged(edge);
    set_sentinels(edge);
    assert(Texture::setup_edge_gouraud(&edge, 0) == 0);
    assert(edge.counter == 0);
    assert_non_counter_fields_unchanged(edge);

    // A negative first height is stored, then returned early without touching
    // the remaining fields; the original helper intentionally has no guard.
    uv_b[1] = 0; uv_b[3] = -4; uv_b[5] = -4;
    set_sentinels(edge);
    assert(Texture::setup_edge(&edge, 0) == 0);
    assert(edge.counter == -4);
    assert_non_counter_fields_unchanged(edge);
    set_sentinels(edge);
    assert(Texture::setup_edge_gouraud(&edge, 0) == 0);
    assert(edge.counter == -4);
    assert_non_counter_fields_unchanged(edge);
}
'''


def host_source():
    cpp = CPP_SOURCE.read_text(encoding="utf-8")
    header = HEADER_SOURCE.read_text(encoding="utf-8")
    edge_scan = extract_edgescan_declaration(header)
    assert "static_assert(sizeof(EdgeScan) == 0x3C" in header
    bodies = (
        extract_braced(cpp, "int __cdecl Texture::setup_edge_gouraud("),
        extract_braced(cpp, "int __cdecl Texture::setup_edge("),
    )
    assert FIXTURE.count("// ACTUAL_EDGESCAN_DECLARATION") == 1
    assert FIXTURE.count("// ACTUAL_EDGE_SCAN_METHODS") == 1
    return (FIXTURE.replace("// ACTUAL_EDGESCAN_DECLARATION", edge_scan)
            .replace("// ACTUAL_EDGE_SCAN_METHODS", "\n\n".join(bodies)))


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_edgescan_owners_on_bounded_walk_and_sign_cases(
        compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    cpp = tmp_path / "edgescan_actual.cpp"
    exe = tmp_path / "edgescan_actual"
    cpp.write_text(host_source(), encoding="utf-8")
    subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
         "-fsanitize=address,undefined", "-fno-omit-frame-pointer",
         str(cpp), "-o", str(exe)],
        check=True,
    )
    subprocess.run([str(exe)], check=True)
