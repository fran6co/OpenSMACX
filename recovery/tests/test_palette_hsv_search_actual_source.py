"""Actual-source controls for the pinned HSV search evaluation contract.

Callbacks observe the original eight-call order and stride-one search domains.
These host controls do not establish x87, uninitialized-return or game acceptance.
"""
from pathlib import Path
import re
import subprocess

import pytest

from test_palette_hsv_owner import actual_hsv_declaration, braced_region, host_compiler

ROOT = Path(__file__).resolve().parents[2]


def product_parts():
    source = (ROOT / "src/jackal/palette.cpp").read_text()
    helper = braced_region(source, source.index("static __forceinline double hsv_sq_distance("))
    body = braced_region(source, source.index("int Palette::get_nearest_palette_index(const HSV *query,"))
    scale = re.search(r"(?m)^const double hsv_deg_to_rad\s*=\s*([^;]+);", source)
    assert scale
    return helper, body, scale.group(1)


def harness(helper, body, scale):
    return r'''
#include <cassert>
#include <cstddef>
#include <vector>
#define __cdecl
#define __forceinline inline
''' + actual_hsv_declaration() + r'''
struct Event { char kind; double argument; };
static std::vector<Event> events;
static bool rounding_case;
static const double hsv_deg_to_rad = ''' + scale + r''';
static int initialized;
static void *PaletteInitialized = &initialized;
static double trace(char kind, double value) {
    Event e = {kind, value}; events.push_back(e);
    if (rounding_case) return value == 0.0 ? 0.0 : 1.0;
    return value + (kind == 'S' ? 0.125 : -0.0625) * events.size();
}
static double observed_sin(double x) { return trace('S', x); }
static double observed_cos(double x) { return trace('C', x); }
#define sin observed_sin
#define cos observed_cos
class Palette {
public:
    struct Range { unsigned int key, first, count; } internal_[5];
    int get_nearest_palette_index(const HSV *, const HSV *, int);
};
''' + helper + "\n" + body + r'''

// Oracle follows the original call ordinal and accumulator, independently of
// the source helper's temporaries. It intentionally models observable calls.
static double expected_distance(const HSV &a, const HSV &b, int first) {
    double qa = a.h * hsv_deg_to_rad, ca = b.h * hsv_deg_to_rad;
    double s1 = (qa + 0.125 * (first + 1)) * a.s
              - (ca + 0.125 * (first + 2)) * b.s;
    double s2 = (qa + 0.125 * (first + 3)) * a.s
              - (ca + 0.125 * (first + 4)) * b.s;
    double c1 = (qa - 0.0625 * (first + 5)) * a.s
              - (ca - 0.0625 * (first + 6)) * b.s;
    double c2 = (qa - 0.0625 * (first + 7)) * a.s
              - (ca - 0.0625 * (first + 8)) * b.s;
    double dv = a.v - b.v;
    return s2 * s1 + c2 * c1 + dv * dv;
}
static void run_search(int mode) {
    Palette palette;
    for (int i = 0; i < 5; ++i) palette.internal_[i].key = ~0U;
    palette.internal_[0].key = 1;
    palette.internal_[0].first = 12;
    palette.internal_[0].count = 3;
    HSV query = {5.0, 0.8, 2.0};
    HSV block[256];
    for (int i = 0; i < 256; ++i) {
        block[i].h = 0.75 + i * 0.002;
        block[i].s = 0.5;
        block[i].v = 2.0 + (i % 17) * 0.01;
    }
    int first = mode ? 10 : 0, end = mode ? 246 : 256;
    int ordinal = 0, expected = -1;
    double best = 200000.0;
    for (int i = first; i < end; ++i) {
        if (mode && i >= 12 && i < 15) continue;
        double distance = expected_distance(query, block[i], ordinal);
        ordinal += 8;
        if (distance < best) { best = distance; expected = i; }
    }
    assert(expected >= 0); // no undefined no-winner result is inspected
    events.clear();
    assert(palette.get_nearest_palette_index(&query, block, mode) == expected);
    assert(events.size() == static_cast<std::size_t>(ordinal));
    int at = 0;
    for (int i = first; i < end; ++i) {
        if (mode && i >= 12 && i < 15) continue;
        for (int call = 0; call < 8; ++call, ++at) {
            assert(events[at].kind == (call < 4 ? 'S' : 'C'));
            double angle = (call % 2 ? block[i].h : query.h) * hsv_deg_to_rad;
            assert(events[at].argument == angle);
        }
    }
    PaletteInitialized = 0; events.clear();
    assert(palette.get_nearest_palette_index(&query, block, mode) == 7);
    assert(events.empty()); PaletteInitialized = &initialized;
}
int main() {
    run_search(0); run_search(1);
    // A host-double rounding control for the original final value-square add.
    rounding_case = true; events.clear();
    HSV a = {5.0, 1.0, 100000000.0}, b = {0.0, 1.0, 0.0};
    assert(hsv_sq_distance(&a, &b) == 10000000000000002.0);
    assert(events.size() == 8);
    return 0;
}
'''


@pytest.mark.parametrize("damage", [None, "cached_trig", "stride_two", "value_square_first"])
def test_original_hsv_evaluation(host_compiler, tmp_path, damage):
    helper, body, scale = product_parts()
    if damage == "cached_trig":
        helper = '''static __forceinline double hsv_sq_distance(const HSV *a, const HSV *b) {
    double dv = a->v - b->v;
    double ds = sin(a->h * hsv_deg_to_rad) * a->s - sin(b->h * hsv_deg_to_rad) * b->s;
    double dc = cos(a->h * hsv_deg_to_rad) * a->s - cos(b->h * hsv_deg_to_rad) * b->s;
    return ds * ds + dc * dc + dv * dv;
}'''
    elif damage == "stride_two":
        before = "for (i = 0; i < 0x100; i++)"
        assert body.count(before) == 2
        body = body.replace(before, "for (i = 0; i < 0x100; i += 2)", 1)
        body = body.replace("arr++;", "arr += 2;", 1)
    elif damage == "value_square_first":
        before = "double chroma_square = "
        last = "    return chroma_square + dv * dv;"
        assert helper.count(before) == helper.count(last) == 1
        helper = helper.replace(before, "return dv * dv + ").replace(last, "")
    source = tmp_path / "hsv_search.cpp"
    binary = tmp_path / "hsv_search"
    source.write_text(harness(helper, body, scale))
    compiled = subprocess.run([host_compiler, "-std=c++11", "-O0", str(source), "-o", str(binary)],
                              capture_output=True, text=True, timeout=60)
    assert compiled.returncode == 0, compiled.stdout + compiled.stderr
    ran = subprocess.run([str(binary)], capture_output=True, text=True, timeout=30)
    if damage:
        assert ran.returncode != 0, f"{damage} escaped the original evaluation control"
    else:
        assert ran.returncode == 0, ran.stdout + ran.stderr
