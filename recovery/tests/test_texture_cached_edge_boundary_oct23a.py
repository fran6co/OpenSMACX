"""Actual-source controls for the mapped cached Gouraud edge setup.

The host fixture splices the complete sub_625080 body, the real Texture/Vert
storage declarations, its actual local arithmetic helpers, and the current
TextureStore lifetime. fixed_div is an observable host service double. These
checks are source/dataflow controls, not VC6 ABI, native-link, or pixel-runtime
acceptance.
"""
from pathlib import Path
import re
import shutil
import subprocess

from match_recovery.source import read_file, read_text
from texture_direct_storage_host import PRELUDE_MARKER, direct_storage_prelude
from test_texture_edge_cache_actual_source import _declaration, _definition


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/texture.cpp"
HEADER = ROOT / "src/texture.h"
VERT_HEADER = ROOT / "src/vert.h"
BASE = "e9244fc4985eab49ab6ce6ad7209c7429f259470"
ADDRESS = 0x00625080


def _baseline_source():
    return subprocess.check_output(
        ["git", "show", f"{BASE}:src/texture.cpp"],
        cwd=ROOT, timeout=30,
    ).decode("utf-8")


def _origin_history(row):
    if row.origin is None:
        return None
    return tuple((item.kind, item.key, item.prose,
                  tuple(line for _, line in item.lines)) for item in row.origin)


def test_original_mapping_and_historical_obligations_are_monotonic():
    before_source = _baseline_source()
    current_source = SOURCE.read_text(encoding="utf-8")
    before = {row.address: row for row in read_text(before_source, SOURCE)}
    current = {row.address: row for row in read_file(SOURCE)}
    assert ADDRESS in before and ADDRESS in current
    old, new = before[ADDRESS], current[ADDRESS]

    # Keep identity, original span, symbol and every historical note/attempt.
    # Old negative claim values are deliberately not frozen: only an existing
    # positive claim is an obligation that must remain positive.
    for name in ("address", "mode", "name", "image_spans", "symbol", "kind",
                 "exclusion", "recipe", "levers", "ruled_out", "unrecoverable",
                 "deferred"):
        assert getattr(new, name) == getattr(old, name), (name, getattr(old, name))
    assert _origin_history(new) == _origin_history(old)
    for name in ("byte_exact", "semantic"):
        if getattr(old, name):
            assert getattr(new, name), f"historical positive {name} claim was lost"

    old_body = _definition(before_source, "int __cdecl sub_625080(")
    new_body = _definition(current_source, "int __cdecl sub_625080(")
    assert old_body != new_body
    assert "return texture_cached_edge(*edge, index," in old_body
    assert "return texture_cached_edge(*edge, index," not in new_body


def _translation_unit(source=None, old_wrapper=False, mutation=None):
    source = SOURCE.read_text(encoding="utf-8") if source is None else source
    header = HEADER.read_text(encoding="utf-8")
    vert_header = VERT_HEADER.read_text(encoding="utf-8")
    fixture = r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#ifndef __cdecl
#define __cdecl
#endif

// ACTUAL_EDGESCAN_DECLARATION
// ACTUAL_VERT_DECLARATION
// ACTUAL_TEXTURESTORE_DECLARATION
// ACTUAL_TEXTURESTORE_LIFETIME
// TEXTURE_DIRECT_STORAGE_PRELUDE

struct DivCall { int numerator, denominator; };
static DivCall div_calls[8];
static int div_call_count;
static EdgeScan *observed_edge;
static TextureStore *observed_cache;
static int observed_slot;
static int initial_record_delta_a;
static int initial_edge_delta_a;
static int first_division_is_uv;
static int callback_mode;
static Vert alternate_uv[3];
static Vert alternate_geometry[3];
static int alternate_light[3];

static int wrap_sub(int a, int b) {
    return static_cast<int>(static_cast<unsigned int>(a) -
                            static_cast<unsigned int>(b));
}
int __cdecl fixed_div(int numerator, int denominator) {
    assert(div_call_count < 8);
    const int call = div_call_count++;
    div_calls[call].numerator = numerator;
    div_calls[call].denominator = denominator;
    assert(denominator != 0);
    // Pinned order: A is committed before B begins; B is committed before D
    // begins. This spy observes source ordering only; it does not claim the
    // original fixed_div mutates any edge, cache, or shared input.
    if (call == 0 && first_division_is_uv) {
        assert(observed_cache->edges_[observed_slot].deltaA == initial_record_delta_a);
        assert(observed_edge->deltaA == initial_edge_delta_a);
    }
    if (call == 1) {
        assert(observed_cache->edges_[observed_slot].deltaA ==
               observed_edge->deltaA);
    }
    if (call == 2) {
        assert(observed_cache->edges_[observed_slot].deltaB ==
               observed_edge->deltaB);
        assert(observed_cache->edges_[observed_slot].deltaC ==
               observed_edge->deltaC);
        assert(observed_cache->edges_[observed_slot].fracDelta ==
               observed_edge->fracDelta);
        assert(observed_cache->edges_[observed_slot].accumD ==
               observed_edge->accumD);
    }
    const int result = static_cast<int>(
        (static_cast<int64_t>(numerator) * 65536) / denominator);
    // Diagnostic-only callback proves each shared table cell is re-read after
    // earlier fixed_div boundaries, rather than cached at function entry.
    if (callback_mode && call == 0) TextureRasterEdgeVerticesA = alternate_uv;
    if (callback_mode && call == 1) TextureRasterEdgeVerticesB = alternate_geometry;
    if (callback_mode && call == 1) TextureRasterZValues = alternate_light;
    return result;
}

// ACTUAL_TEXTURE_ARITHMETIC_HELPERS
// ACTUAL_CACHED_EDGE_HELPER
// ACTUAL_SUB_625080

static_assert(sizeof(EdgeScan) == 0x3c, "EdgeScan storage");
static_assert(sizeof(TextureStore) == 0x260, "TextureStore storage");
static_assert(offsetof(TextureStore, flags_) == 0, "flags offset");
static_assert(offsetof(TextureStore, next_record_) == 4, "cursor offset");
static_assert(offsetof(TextureStore, edges_) == 8, "record array offset");

static void poison(EdgeScan &edge, unsigned char byte) {
    memset(&edge, byte, sizeof(edge));
}
static void install_scene() {
    TextureRasterVertexCount = 3;
    TextureRasterMaxYIndex = 2;
    TextureRasterEdgeVerticesB = 0;
    TextureRasterEdgeVerticesA = 0;
    TextureRasterZValues = 0;
}
static void reset_divs() {
    memset(div_calls, 0, sizeof(div_calls));
    div_call_count = 0;
    callback_mode = 0;
    first_division_is_uv = 0;
}
static int fixed(int value) {
    return static_cast<int>(static_cast<unsigned int>(value) << 16);
}

static void test_flags_slots_and_untouched_fields() {
    const Vert geometry[3] = {{10, 0}, {17, 5}, {4, 10}};
    const Vert uv[3] = {{1, 2}, {7, 12}, {15, 20}};
    const int light[3] = {3, 8, 23};
    const int slots[3] = {0, 3, 9};
    for (int flags = 0; flags < 4; ++flags) {
        for (int si = 0; si < 3; ++si) {
            install_scene();
            TextureRasterEdgeVerticesB = const_cast<Vert *>(geometry);
            TextureRasterEdgeVerticesA = const_cast<Vert *>(uv);
            TextureRasterZValues = const_cast<int *>(light);
            TextureStore cache;
            memset(cache.edges_, 0x5a, sizeof(cache.edges_));
            cache.flags_ = static_cast<unsigned int>(flags) | 0x80000000u;
            cache.next_record_ = slots[si];
            EdgeScan records_before[10];
            memcpy(records_before, cache.edges_, sizeof(records_before));
            EdgeScan edge;
            poison(edge, 0x36);
            edge.f0 = 1;
            const EdgeScan edge_before = edge;
            reset_divs();
            observed_edge = &edge;
            observed_cache = &cache;
            observed_slot = slots[si];
            initial_record_delta_a = records_before[slots[si]].deltaA;
            initial_edge_delta_a = edge_before.deltaA;
            first_division_is_uv = (flags & 2) != 0;

            assert(sub_625080(&edge, 0, &cache) == 1);
            assert(cache.flags_ == (static_cast<unsigned int>(flags) | 0x80000000u));
            const int next_cursor = slots[si] == 9 ? 0 : slots[si] + 1;
            assert(cache.next_record_ == next_cursor);
            assert(edge.f0 == edge_before.f0 && edge.counter == 5 && edge.id == 1);
            assert(edge.accumA == fixed(1) + 0x8000);
            assert(edge.accumB == fixed(2) + 0x8000);
            assert(edge.accumC == 10 && edge.deltaC == (flags & 2 ? 1 : records_before[slots[si]].deltaC));
            assert(edge.extraDeltaC == 1 && edge.fracAccum == 0 && edge.fracSub == 5);
            assert(edge.accumD == (flags & 1 ? fixed(3) : records_before[slots[si]].accumD));
            assert(edge.deltaD == (flags & 1 ? cache.edges_[slots[si]].deltaD :
                                   records_before[slots[si]].deltaD));

            const int expected_calls = ((flags & 2) ? 2 : 0) + ((flags & 1) ? 1 : 0);
            assert(div_call_count == expected_calls);
            int call = 0;
            const int scale = fixed(5);
            if (flags & 2) {
                assert(div_calls[call].numerator == wrap_sub(fixed(7), fixed(1) + 0x8000));
                assert(div_calls[call++].denominator == scale);
                assert(div_calls[call].numerator == wrap_sub(fixed(12), fixed(2) + 0x8000));
                assert(div_calls[call++].denominator == scale);
                assert(cache.edges_[slots[si]].deltaA == edge.deltaA);
                assert(cache.edges_[slots[si]].deltaB == edge.deltaB);
                assert(cache.edges_[slots[si]].deltaC == edge.deltaC);
                assert(cache.edges_[slots[si]].fracDelta == edge.fracDelta);
                assert(edge.deltaA == div_calls[0].numerator * 65536LL / scale);
                assert(edge.deltaB == div_calls[1].numerator * 65536LL / scale);
                assert(cache.edges_[slots[si]].fracDelta == 2);
            } else {
                assert(edge.deltaA == records_before[slots[si]].deltaA);
                assert(edge.deltaB == records_before[slots[si]].deltaB);
                assert(edge.fracDelta == records_before[slots[si]].fracDelta);
                assert(cache.edges_[slots[si]].deltaA == records_before[slots[si]].deltaA);
                assert(cache.edges_[slots[si]].deltaB == records_before[slots[si]].deltaB);
                assert(cache.edges_[slots[si]].deltaC == records_before[slots[si]].deltaC);
                assert(cache.edges_[slots[si]].fracDelta == records_before[slots[si]].fracDelta);
            }
            if (flags & 1) {
                assert(div_calls[call].numerator == wrap_sub(fixed(8), fixed(3)));
                assert(div_calls[call].denominator == scale);
                assert(cache.edges_[slots[si]].accumD == fixed(3));
                assert(cache.edges_[slots[si]].deltaD == edge.deltaD);
            } else {
                assert(cache.edges_[slots[si]].accumD == records_before[slots[si]].accumD);
                assert(cache.edges_[slots[si]].deltaD == records_before[slots[si]].deltaD);
            }
            EdgeScan expected_record = records_before[slots[si]];
            if (flags & 2) {
                expected_record.deltaA = edge.deltaA;
                expected_record.deltaB = edge.deltaB;
                expected_record.deltaC = edge.deltaC;
                expected_record.fracDelta = edge.fracDelta;
            }
            if (flags & 1) {
                expected_record.accumD = edge.accumD;
                expected_record.deltaD = edge.deltaD;
            }
            assert(memcmp(&cache.edges_[slots[si]], &expected_record,
                          sizeof(expected_record)) == 0);
            EdgeScan expected_edge = edge_before;
            expected_edge.counter = 5;
            expected_edge.id = 1;
            expected_edge.accumA = fixed(1) + 0x8000;
            expected_edge.accumB = fixed(2) + 0x8000;
            expected_edge.deltaA = expected_record.deltaA;
            expected_edge.deltaB = expected_record.deltaB;
            expected_edge.accumC = 10;
            expected_edge.deltaC = expected_record.deltaC;
            expected_edge.extraDeltaC = 1;
            expected_edge.fracAccum = 0;
            expected_edge.fracDelta = expected_record.fracDelta;
            expected_edge.fracSub = 5;
            expected_edge.accumD = expected_record.accumD;
            expected_edge.deltaD = expected_record.deltaD;
            assert(memcmp(&edge, &expected_edge, sizeof(edge)) == 0);
            for (int record = 0; record < 10; ++record) {
                if (record != slots[si]) {
                    assert(memcmp(&cache.edges_[record], &records_before[record],
                                  sizeof(EdgeScan)) == 0);
                }
            }
            assert(edge.deltaD == (flags & 1 ? cache.edges_[slots[si]].deltaD :
                                   records_before[slots[si]].deltaD));
        }
    }
}

static void test_live_global_rereads_between_fixed_divisions() {
    const Vert geometry[3] = {{10, 0}, {17, 5}, {4, 10}};
    const Vert uv[3] = {{1, 2}, {7, 12}, {15, 20}};
    const int light[3] = {3, 8, 23};
    alternate_uv[0] = Vert{101, 102};
    alternate_uv[1] = Vert{111, 112};
    alternate_uv[2] = Vert{121, 122};
    alternate_geometry[0] = Vert{30, 0};
    alternate_geometry[1] = Vert{41, 5};
    alternate_geometry[2] = Vert{50, 12};
    alternate_light[0] = 13;
    alternate_light[1] = 18;
    alternate_light[2] = 28;
    install_scene();
    TextureRasterEdgeVerticesB = const_cast<Vert *>(geometry);
    TextureRasterEdgeVerticesA = const_cast<Vert *>(uv);
    TextureRasterZValues = const_cast<int *>(light);
    TextureStore cache;
    memset(cache.edges_, 0x31, sizeof(cache.edges_));
    cache.flags_ = 3;
    cache.next_record_ = 3;
    EdgeScan edge;
    poison(edge, 0x45);
    edge.f0 = 1;
    reset_divs();
    callback_mode = 1;
    observed_edge = &edge;
    observed_cache = &cache;
    observed_slot = 3;
    initial_record_delta_a = cache.edges_[3].deltaA;
    initial_edge_delta_a = edge.deltaA;
    first_division_is_uv = 1;
    assert(sub_625080(&edge, 0, &cache) == 1);
    assert(div_call_count == 3);
    // A sees the entry UV base; B sees the callback-updated base. D rereads
    // the light base after the callback at the preceding division boundary.
    assert(div_calls[0].numerator == wrap_sub(fixed(7), fixed(1) + 0x8000));
    assert(div_calls[1].numerator == wrap_sub(fixed(112), fixed(2) + 0x8000));
    assert(edge.accumC == 30 && edge.id == 1);
    assert(edge.accumD == fixed(13));
    assert(div_calls[2].numerator == wrap_sub(fixed(18), fixed(13)));
}

static void test_wrap_horizontal_skip_descending_and_bottom() {
    Vert geometry[3] = {{0, 4}, {3, 4}, {8, 9}};
    Vert uv[3] = {{1, 2}, {3, 4}, {5, 6}};
    int light[3] = {4, 5, 6};
    install_scene();
    TextureRasterVertexCount = 3;
    TextureRasterMaxYIndex = 2;
    TextureRasterEdgeVerticesB = geometry;
    TextureRasterEdgeVerticesA = uv;
    TextureRasterZValues = light;
    TextureStore cache;
    memset(cache.edges_, 0x7c, sizeof(cache.edges_));
    cache.flags_ = 0x80000000u;
    cache.next_record_ = 9;
    EdgeScan edge;
    poison(edge, 0x62);
    edge.f0 = 1;
    reset_divs();
    observed_edge = &edge; observed_cache = &cache; observed_slot = 9;
    initial_record_delta_a = cache.edges_[9].deltaA;
    initial_edge_delta_a = edge.deltaA;
    // The zero-height 0->1 edge is skipped; the next edge is initialized.
    assert(sub_625080(&edge, 0, &cache) == 1);
    assert(edge.id == 2 && edge.counter == 5 && cache.next_record_ == 0);
    assert(div_call_count == 0);

    // Unsigned 32-bit endpoint arithmetic wraps 0 + (-1) to the last vertex.
    geometry[0] = Vert{0, 4}; geometry[2] = Vert{7, 9};
    cache.next_record_ = 0;
    cache.flags_ = 2;
    edge.f0 = -1;
    reset_divs(); observed_slot = 0;
    initial_record_delta_a = cache.edges_[0].deltaA;
    initial_edge_delta_a = edge.deltaA;
    first_division_is_uv = 1;
    assert(sub_625080(&edge, 0, &cache) == 1);
    assert(edge.id == 2 && edge.counter == 5 && div_call_count == 2);
    assert(edge.accumC == 0 && edge.deltaC == 1 && edge.fracDelta == 2);

    // Descending and all-horizontal paths return without allocating a slot or
    // changing cache records; reaching the nominated bottom also returns early.
    EdgeScan saved_records[10];
    memcpy(saved_records, cache.edges_, sizeof(saved_records));
    const int cursor = cache.next_record_;
    edge.f0 = -1;
    geometry[0].y = 2; geometry[1].y = 4; geometry[2].y = 9;
    EdgeScan before = edge;
    reset_divs();
    assert(sub_625080(&edge, 1, &cache) == 0);
    assert(edge.counter == -2 && edge.id == before.id);
    assert(cache.next_record_ == cursor && div_call_count == 0);
    assert(memcmp(cache.edges_, saved_records, sizeof(saved_records)) == 0);

    edge = before;
    const EdgeScan before_bottom = edge;
    reset_divs();
    assert(sub_625080(&edge, 2, &cache) == 0);
    assert(memcmp(&edge, &before_bottom, sizeof(edge)) == 0);
    assert(cache.next_record_ == cursor && div_call_count == 0);

    geometry[0].y = geometry[1].y = geometry[2].y = 0;
    const EdgeScan before_horizontal = edge;
    reset_divs();
    assert(sub_625080(&edge, 0, &cache) == 0);
    EdgeScan expected = before_horizontal;
    expected.counter = 0;
    assert(memcmp(&edge, &expected, sizeof(edge)) == 0);
    assert(cache.next_record_ == cursor && div_call_count == 0);
    assert(memcmp(cache.edges_, saved_records, sizeof(saved_records)) == 0);
}

static void test_signed_32_bit_coordinate_differences() {
    Vert geometry[3] = {{0x7fffffff, 0}, {static_cast<int>(0x80000000u), 5}, {4, 10}};
    Vert uv[3] = {{0, 0}, {1, 1}, {2, 2}};
    int light[3] = {0, 1, 2};
    install_scene();
    TextureRasterEdgeVerticesB = geometry;
    TextureRasterEdgeVerticesA = uv;
    TextureRasterZValues = light;
    TextureStore cache;
    memset(cache.edges_, 0x28, sizeof(cache.edges_));
    cache.flags_ = 2;
    cache.next_record_ = 0;
    EdgeScan edge;
    poison(edge, 0x19);
    edge.f0 = 1;
    reset_divs(); observed_edge = &edge; observed_cache = &cache; observed_slot = 0;
    initial_record_delta_a = cache.edges_[0].deltaA;
    initial_edge_delta_a = edge.deltaA;
    first_division_is_uv = 1;
    assert(sub_625080(&edge, 0, &cache) == 1);
    assert(edge.accumC == 0x7fffffff);
    assert(edge.deltaC == 0 && edge.fracDelta == 1);
}

static void test_negative_dx_fill_replay_and_light_only() {
    const Vert geometry[3] = {{10, 0}, {3, 5}, {20, 10}};
    const Vert uv[3] = {{1, 2}, {7, 12}, {15, 20}};
    const int light[3] = {3, 8, 23};
    const int flags[3] = {2, 0, 1};
    const int slots[3] = {0, 3, 9};
    for (int which = 0; which < 3; ++which) {
        install_scene();
        TextureRasterEdgeVerticesB = const_cast<Vert *>(geometry);
        TextureRasterEdgeVerticesA = const_cast<Vert *>(uv);
        TextureRasterZValues = const_cast<int *>(light);
        TextureStore cache;
        memset(cache.edges_, 0x4b, sizeof(cache.edges_));
        cache.flags_ = static_cast<unsigned int>(flags[which]);
        cache.next_record_ = slots[which];
        EdgeScan &record = cache.edges_[slots[which]];
        record.deltaA = 0x12345;
        record.deltaB = -0x23456;
        record.deltaC = -1;
        record.fracDelta = 2;
        record.accumD = 0x34567;
        record.deltaD = -0x45678;
        EdgeScan edge;
        poison(edge, 0x53);
        edge.f0 = 1;
        reset_divs();
        observed_edge = &edge;
        observed_cache = &cache;
        observed_slot = slots[which];
        initial_record_delta_a = record.deltaA;
        initial_edge_delta_a = edge.deltaA;
        first_division_is_uv = (flags[which] & 2) != 0;

        assert(sub_625080(&edge, 0, &cache) == 1);
        assert(edge.id == 1 && edge.counter == 5 && edge.accumC == 10);
        assert(edge.deltaC == -1 && edge.fracDelta == 2);
        assert(edge.extraDeltaC == -1 && edge.fracAccum == -4 && edge.fracSub == 5);
        assert(edge.deltaA == ((flags[which] & 2) ?
                               div_calls[0].numerator * 65536LL / fixed(5) : 0x12345));
        assert(edge.deltaB == ((flags[which] & 2) ?
                               div_calls[1].numerator * 65536LL / fixed(5) : -0x23456));
        const int expected_calls = ((flags[which] & 2) ? 2 : 0) +
                                   ((flags[which] & 1) ? 1 : 0);
        assert(div_call_count == expected_calls);
        if (flags[which] & 2) {
            assert(div_calls[0].numerator == wrap_sub(fixed(7), fixed(1) + 0x8000));
            assert(div_calls[1].numerator == wrap_sub(fixed(12), fixed(2) + 0x8000));
            assert(record.deltaC == -1 && record.fracDelta == 2);
        } else {
            assert(record.deltaA == 0x12345 && record.deltaB == -0x23456);
            assert(record.deltaC == -1 && record.fracDelta == 2);
        }
        assert(cache.next_record_ == (slots[which] == 9 ? 0 : slots[which] + 1));
        if (flags[which] & 1) {
            assert(edge.accumD == fixed(3));
            assert(edge.deltaD == div_calls[div_call_count - 1].numerator *
                                  65536LL / fixed(5));
            assert(record.accumD == edge.accumD && record.deltaD == edge.deltaD);
        } else {
            assert(edge.accumD == 0x34567 && edge.deltaD == -0x45678);
            assert(record.accumD == 0x34567 && record.deltaD == -0x45678);
        }
    }
}

int main() {
    test_flags_slots_and_untouched_fields();
    test_live_global_rereads_between_fixed_divisions();
    test_wrap_horizontal_skip_descending_and_bottom();
    test_signed_32_bit_coordinate_differences();
    test_negative_dx_fill_replay_and_light_only();
    return 0;
}
'''
    actual = {
        "// ACTUAL_EDGESCAN_DECLARATION": _declaration(header, "struct", "EdgeScan"),
        "// ACTUAL_VERT_DECLARATION": _declaration(vert_header, "struct", "Vert"),
        "// ACTUAL_TEXTURESTORE_DECLARATION": _declaration(header, "class", "TextureStore"),
        "// ACTUAL_TEXTURESTORE_LIFETIME": "\n\n".join((
            _definition(source, "TextureStore::TextureStore("),
            _definition(source, "TextureStore::~TextureStore("),
            _definition(source, "EdgeScan *TextureStore::sub_6252c0("),
        )),
        "// ACTUAL_TEXTURE_ARITHMETIC_HELPERS": "\n\n".join((
            _definition(source, "static int texture_fixed("),
            _definition(source, "static int texture_difference("),
        )),
        "// ACTUAL_CACHED_EDGE_HELPER": _definition(source, "static int texture_cached_edge("),
        "// ACTUAL_SUB_625080": _definition(source, "int __cdecl sub_625080("),
    }
    for marker, actual_source in actual.items():
        assert fixture.count(marker) == 1, marker
        fixture = fixture.replace(marker, actual_source, 1)
    assert fixture.count(PRELUDE_MARKER) == 1
    fixture = fixture.replace(PRELUDE_MARKER, direct_storage_prelude(source), 1)
    if old_wrapper:
        old_body = _definition(_baseline_source(), "int __cdecl sub_625080(")
        current_body = actual["// ACTUAL_SUB_625080"]
        assert "texture_cached_edge" in old_body
        fixture = fixture.replace(current_body, old_body, 1)
    if mutation is not None:
        fixture = mutation(fixture)
    return fixture


def _compiler():
    compiler = shutil.which("clang++") or shutil.which("g++")
    assert compiler, "host C++ compiler is required for these controls"
    return compiler


def _compile_run(tmp_path, *, old_wrapper=False, mutation=None):
    tmp_path.mkdir(parents=True, exist_ok=True)
    compiler = _compiler()
    cpp = tmp_path / "texture_cached_edge_actual.cpp"
    binary = tmp_path / "texture_cached_edge_actual"
    cpp.write_text(_translation_unit(old_wrapper=old_wrapper, mutation=mutation),
                   encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Wno-unused-parameter",
         str(cpp), "-o", str(binary)],
        check=False, capture_output=True, text=True, timeout=45,
        preexec_fn=lambda: __import__("resource").setrlimit(
            __import__("resource").RLIMIT_CORE, (0, 0)),
    )
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run(
        [str(binary)], check=False, capture_output=True, text=True, timeout=30,
        preexec_fn=lambda: __import__("resource").setrlimit(
            __import__("resource").RLIMIT_CORE, (0, 0)),
    )
    return ran


def test_actual_cached_edge_body_flags_slots_and_order(tmp_path):
    result = _compile_run(tmp_path)
    assert result.returncode == 0, result.stdout + result.stderr


def _wrong_channel(unit):
    start = unit.index("int __cdecl sub_625080(")
    end = unit.index("\n}\n", start) + 2
    body = unit[start:end]
    assert body.count("cache->flags_ & 2u") == 2
    body = body.replace("cache->flags_ & 2u", "cache->flags_ & 1u")
    return unit[:start] + body + unit[end:]


def _stale_uv_snapshot(unit):
    body_start = unit.index("int __cdecl sub_625080(")
    body_end = unit.index("\n}\n", body_start) + 2
    body = unit[body_start:body_end]
    body = body.replace("{\n    if (index", "{\n    const Vert *const old_uv = TextureRasterEdgeVerticesA;\n    if (index", 1)
    body = body.replace("TextureRasterEdgeVerticesA[", "old_uv[")
    return unit[:body_start] + body + unit[body_end:]


def _premature_store(unit):
    needle = """                const int delta_a = fixed_div(
                    texture_difference(
                        texture_fixed(TextureRasterEdgeVerticesA[endpoint].x),
                        edge->accumA), scale);"""
    assert needle in unit
    return unit.replace(needle, """                record->deltaA = 0x1234567;
                edge->deltaA = 0x1234567;
                const int delta_a = fixed_div(
                    texture_difference(
                        texture_fixed(TextureRasterEdgeVerticesA[endpoint].x),
                        edge->accumA), scale);""", 1)


def _wrong_negative_slope(unit):
    start = unit.index("int __cdecl sub_625080(")
    end = unit.index("\n}\n", start) + 2
    body = unit[start:end]
    assert body.count("if (dx < 0)") == 2
    body = body.replace("if (dx < 0)", "if (dx > 0)")
    return unit[:start] + body + unit[end:]


def test_compiled_damage_mutants_are_sensitive_and_compile(tmp_path):
    result = _compile_run(tmp_path, old_wrapper=True)
    assert result.returncode != 0, "old delegated wrapper escaped the real-body controls"
    for name, mutation in (("wrong_cache_channel", _wrong_channel),
                           ("stale_uv_snapshot", _stale_uv_snapshot),
                           ("premature_cache_store", _premature_store),
                           ("wrong_negative_slope", _wrong_negative_slope)):
        result = _compile_run(tmp_path / name, mutation=mutation)
        assert result.returncode != 0, f"{name} mutant escaped the controls"
