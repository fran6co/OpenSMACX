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
from texture_direct_storage_host import direct_storage_prelude

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


def extract_edgescan_declaration(header, name="EdgeScan"):
    view = masked(header)
    match = re.search(r"\bstruct\s+" + re.escape(name) + r"\s*\{", view)
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
#include <limits.h>
#include <stdint.h>
#include <string.h>

#ifndef __cdecl
#define __cdecl
#endif

// ACTUAL_EDGESCAN_DECLARATION
// ACTUAL_VERT_DECLARATION

static_assert(sizeof(EdgeScan) == 0x3c, "actual EdgeScan stride");
class Texture {
public:
    static int setup_edge_gouraud(EdgeScan *, int);
    static int setup_edge(EdgeScan *, int);
};

// TEXTURE_DIRECT_STORAGE_PRELUDE

// Host-only spelling of fixed_div's signed 16.16 quotient. Multiplication
// avoids UB from shifting a negative signed numerator; product source remains
// untouched. Test inputs stay within the original helper's valid int domain.
int fixed_div(int numerator, int denominator) {
    return static_cast<int>((static_cast<int64_t>(numerator) * 65536) /
                            denominator);
}

// ACTUAL_EDGE_SCAN_METHODS

// ACTUAL_EDGE_STEP_HELPERS

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
    Vert uv_a[3] = {{1, 2}, {5, 6}, {9, 10}};
    // [z0,y0,z1,y1,z2,y2]: 4-row edge with positive depth delta 21.
    Vert uv_b[3] = {{10, 0}, {31, 4}, {50, 8}};
    int z[3] = {100, 108, 116};
    TextureRasterEdgeVerticesA = uv_a;
    TextureRasterEdgeVerticesB = uv_b;
    TextureRasterZValues = z;
    TextureRasterVertexCount = 3;
    TextureRasterMaxYIndex = 2;

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
    uv_b[0].x = 31;
    uv_b[1].x = 10;
    set_sentinels(edge);
    assert(Texture::setup_edge_gouraud(&edge, 0) == 1);
    assert(edge.accumC == 31 && edge.deltaC == -5);
    assert(edge.extraDeltaC == -1 && edge.fracAccum == -3);
    assert(edge.fracSub == 4 && edge.fracDelta == 1);

    // Plain edge uses no half-pixel bias and leaves the D-channel words
    // untouched even on a successful setup.
    uv_b[0].x = 10;
    uv_b[1].x = 31;
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

    uv_b[0].x = 31;
    uv_b[1].x = 10;
    set_sentinels(edge);
    assert(Texture::setup_edge(&edge, 0) == 1);
    assert(edge.accumC == 31 && edge.deltaC == -5);
    assert(edge.extraDeltaC == -1 && edge.fracAccum == -3);
    assert(edge.fracSub == 4 && edge.fracDelta == 1);
    assert(edge.accumD == 113 && edge.deltaD == 114);

    // Starting at the max-y vertex returns before touching any field.
    TextureRasterMaxYIndex = 0;
    set_sentinels(edge);
    assert(Texture::setup_edge_gouraud(&edge, 0) == 0);
    assert_non_counter_fields_unchanged(edge);
    assert(edge.counter == 101);
    assert(Texture::setup_edge(&edge, 0) == 0);
    assert_non_counter_fields_unchanged(edge);
    assert(edge.counter == 101);

    // A zero-height walk visits the next vertices until max-y, writes only
    // counter, and leaves every other field untouched in both helpers.
    TextureRasterMaxYIndex = 2;
    uv_b[0].y = uv_b[1].y = uv_b[2].y = 0;
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
    uv_b[0].y = 0; uv_b[1].y = -4; uv_b[2].y = -4;
    set_sentinels(edge);
    assert(Texture::setup_edge(&edge, 0) == 0);
    assert(edge.counter == -4);
    assert_non_counter_fields_unchanged(edge);
    set_sentinels(edge);
    assert(Texture::setup_edge_gouraud(&edge, 0) == 0);
    assert(edge.counter == -4);
    assert_non_counter_fields_unchanged(edge);

    // Exercise nonzero post-decrement behavior for ordinary zero/negative
    // counters and native signed wrap at INT_MIN. Conversion of out-of-range
    // unsigned bits to int is implementation-defined; these assertions state
    // the pinned x86 two's-complement target behavior explicitly.
    int (*steppers[2])(EdgeScan *) = {sub_61f790, sub_61fdf0};
    const int input_counters[3] = {0, -1, INT_MIN};
    const int expected_counters[3] = {-1, -2, INT_MAX};
    for (int stepper = 0; stepper < 2; ++stepper) {
        for (int index = 0; index < 3; ++index) {
            edge.f0 = 1;
            edge.counter = input_counters[index];
            edge.id = 0;
            edge.accumA = 1; edge.deltaA = 2;
            edge.accumB = 3; edge.deltaB = 4;
            edge.accumC = 5; edge.deltaC = 6;
            edge.extraDeltaC = 7;
            edge.fracAccum = -10; edge.fracDelta = 0; edge.fracSub = 2;
            edge.accumD = 20; edge.deltaD = 4;
            assert(steppers[stepper](&edge) == 1);
            assert(edge.counter == expected_counters[index]);
            assert(edge.accumA == 3 && edge.accumB == 7);
            assert(edge.accumC == 11 && edge.fracAccum == -10);
            assert(edge.accumD == (stepper == 0 ? 20 : 24));
        }
    }

    // Positive carry, exactly-zero sum, negative sum, and both signed
    // comparison outcomes after 32-bit addition overflow. Run all cases
    // through both extracted step bodies.
    for (int stepper = 0; stepper < 2; ++stepper) {
        edge.counter = 2;
        edge.accumA = 1; edge.deltaA = 2;
        edge.accumB = 3; edge.deltaB = 4;
        edge.accumC = 100; edge.deltaC = -2; edge.extraDeltaC = 7;
        edge.fracAccum = 3; edge.fracDelta = 2; edge.fracSub = 4;
        edge.accumD = INT_MAX; edge.deltaD = 1;
        assert(steppers[stepper](&edge) == 1);
        assert(edge.accumC == 105 && edge.fracAccum == 1);
        assert(edge.accumD == (stepper == 0 ? INT_MAX : INT_MIN));

        // 0 is not positive; leave the incremented C value and fraction alone.
        edge.counter = 2; edge.accumC = 10; edge.deltaC = 2;
        edge.extraDeltaC = 99; edge.fracAccum = -2; edge.fracDelta = 2;
        edge.fracSub = 4;
        assert(steppers[stepper](&edge) == 1);
        assert(edge.accumC == 12 && edge.fracAccum == 0);

        // Negative fractional sum also bypasses correction.
        edge.counter = 2; edge.accumC = 10; edge.deltaC = 2;
        edge.fracAccum = -3; edge.fracDelta = -2; edge.fracSub = 4;
        assert(steppers[stepper](&edge) == 1);
        assert(edge.accumC == 12 && edge.fracAccum == -5);

        // INT_MAX + 1 wraps to INT_MIN and fails the signed `> 0` test.
        edge.counter = 2; edge.accumC = 10; edge.deltaC = 2;
        edge.extraDeltaC = 99;
        edge.fracAccum = INT_MAX; edge.fracDelta = 1; edge.fracSub = 4;
        assert(steppers[stepper](&edge) == 1);
        assert(edge.accumC == 12 && edge.fracAccum == INT_MIN);

        // INT_MIN - 1 wraps to INT_MAX, takes the correction, then subtracts
        // fracSub from that wrapped positive value.
        edge.counter = 2; edge.accumC = 10; edge.deltaC = 2;
        edge.extraDeltaC = 99;
        edge.fracAccum = INT_MIN; edge.fracDelta = -1; edge.fracSub = 1;
        assert(steppers[stepper](&edge) == 1);
        assert(edge.accumC == 111 && edge.fracAccum == INT_MAX - 1);
    }

    // Counter refresh: a zero after decrement calls the corresponding real
    // static setup on this same record/id and returns its normalized result.
    uv_b[0].x = 10; uv_b[0].y = 0;
    uv_b[1].x = 31; uv_b[1].y = 4;
    uv_b[2].x = 50; uv_b[2].y = 8;
    TextureRasterVertexCount = 3;
    TextureRasterMaxYIndex = 2;
    for (int stepper = 0; stepper < 2; ++stepper) {
        edge.f0 = 1; edge.counter = 1; edge.id = 0;
        assert(steppers[stepper](&edge) == 1);
        assert(edge.counter == 4 && edge.id == 1);
    }

    // The same callback paths return false when the endpoint already equals
    // max-y; only the stepper's initial counter decrement has taken effect.
    TextureRasterMaxYIndex = 0;
    for (int stepper = 0; stepper < 2; ++stepper) {
        edge.f0 = 1; edge.counter = 1; edge.id = 0;
        edge.accumA = 303; edge.accumB = 304; edge.accumC = 307;
        edge.accumD = 313; edge.fracAccum = 310;
        assert(steppers[stepper](&edge) == 0);
        assert(edge.counter == 0 && edge.accumA == 303 && edge.accumB == 304);
        assert(edge.accumC == 307 && edge.accumD == 313);
        assert(edge.fracAccum == 310);
    }
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
        extract_braced(cpp, "int __cdecl sub_61f790(EdgeScan *edge)"),
        extract_braced(cpp, "int __cdecl sub_61fdf0(EdgeScan *edge)"),
    )
    assert FIXTURE.count("// ACTUAL_EDGESCAN_DECLARATION") == 1
    assert FIXTURE.count("// ACTUAL_EDGE_SCAN_METHODS") == 1
    assert FIXTURE.count("// ACTUAL_EDGE_STEP_HELPERS") == 1
    return (FIXTURE.replace("// ACTUAL_EDGESCAN_DECLARATION", edge_scan)
            .replace("// ACTUAL_VERT_DECLARATION", extract_edgescan_declaration(
                (HEADER_SOURCE.parent / "vert.h").read_text(), "Vert"))
            .replace("// TEXTURE_DIRECT_STORAGE_PRELUDE",
                     direct_storage_prelude(cpp), 1)
            .replace("// ACTUAL_EDGE_SCAN_METHODS", "\n\n".join(bodies[:2]))
            .replace("// ACTUAL_EDGE_STEP_HELPERS", "\n\n".join(bodies[2:])))


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
         "-Wno-unused-function", "-Wno-unused-variable",
         "-fsanitize=address,undefined", "-fno-omit-frame-pointer",
         str(cpp), "-o", str(exe)],
        check=True,
    )
    subprocess.run([str(exe)], check=True)


def test_actual_step_bodies_keep_native_field_store_order():
    cpp = CPP_SOURCE.read_text(encoding="utf-8")
    plain = extract_braced(cpp, "int __cdecl sub_61f790(EdgeScan *edge)")
    gouraud = extract_braced(cpp, "int __cdecl sub_61fdf0(EdgeScan *edge)")

    def stores(body):
        return re.findall(
            r"edge->(counter|accumA|accumB|accumC|accumD|fracAccum)\s*=(?!=)",
            masked(body),
        )

    assert stores(plain) == [
        "counter", "accumB", "accumA", "accumC", "fracAccum", "accumC", "fracAccum"
    ]
    assert stores(gouraud) == [
        "counter", "accumB", "accumA", "accumC", "accumD", "fracAccum",
        "accumC", "fracAccum"
    ]
    assert "Texture::setup_edge(edge, edge->id)" in masked(plain)
    assert "Texture::setup_edge_gouraud(edge, edge->id)" in masked(gouraud)
