"""Bounded actual-source controls for four EdgeScan-shaped raster inputs.

The complete mapped bodies and EdgeScan declaration are extracted from product
source. The fixture uses low-address memory because two recovered entries carry
record addresses through int parameters. The shade cursor now uses product
address-domain storage rather than accessing an int cell as a pointer. This
is not a target-layout or ABI proof, nor
does it establish the helpers' incoming producer identity.
"""
from pathlib import Path
import platform
import re
import shutil
import subprocess
import sys

import pytest

from match_recovery.includes import COMMENTS_OR_STRINGS
from match_recovery.source import read_text
from test_texture_raster_actual_source import _continuity_fields


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/texture.cpp").is_file()
            and (parent / "src/texture.h").is_file())
TEXTURE = ROOT / "src/texture.cpp"
HEADER = ROOT / "src/texture.h"
FUNCTIONS = (
    ("void __cdecl sub_61f800(", "sub_61f800"),
    ("void __cdecl sub_61fe70(", "sub_61fe70"),
    ("void __cdecl sub_6202e0(", "sub_6202e0"),
    ("void __cdecl sub_620640(", "sub_620640"),
)
POINTER_HELPERS = (
    "static uintptr_t texture_pointer_address(",
    "static uintptr_t texture_pointer_offset(",
)
DECLARATION_SLOT = "// ACTUAL_EDGESCAN_DECLARATION"
DEFINITIONS_SLOT = "// ACTUAL_RASTER_DEFINITIONS"

FIXTURE = r'''#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <sys/mman.h>

#ifndef __cdecl
#define __cdecl
#endif
#ifndef __int64
#define __int64 long long
#endif

// ACTUAL_EDGESCAN_DECLARATION
static_assert(sizeof(EdgeScan) == 0x3c, "actual EdgeScan layout");

static int clip_left_value, clip_right_value;
static int *const blit_clip_left = &clip_left_value;
static int *const blit_clip_right = &clip_right_value;
static int light_accum_value, light_step_value;
static int *const blit_light_accum_b = &light_accum_value;
static int *const blit_light_step_b = &light_step_value;
static unsigned short light_pack_value;
static unsigned short *const blit_light_pack_a = &light_pack_value;
static int signed_pitch_value, colour_step_value;
static int *const blit_slot_4e8 = &signed_pitch_value;
static int *const blit_colour_step = &colour_step_value;
static void *pixel_base_value;
static void **const blit_pixel_base = &pixel_base_value;
static int row_offset_value, buffer_stride_value;
static int *const blit_row_offset = &row_offset_value;
static int *const blit_buffer_stride = &buffer_stride_value;
static void *texture_pixels_value;
static void **const blit_texture_pixels = &texture_pixels_value;
static int TextureRowPitchScratch;
static const void *remap_address_value;
static const void **const blit_remap_table = &remap_address_value;
static void *shade_table_value;
static void **const blit_shade_table = &shade_table_value;
static uintptr_t shade_cursor_storage;
static uintptr_t *const blit_slot_558 = &shade_cursor_storage;
static int shade_value;
static int *const blit_slot_4d8 = &shade_value;
static int dither_address_value;
static int *const blit_slot_55c = &dither_address_value;

// ACTUAL_RASTER_DEFINITIONS

static unsigned char *low_arena(size_t size) {
#if defined(MAP_32BIT)
    void *memory = mmap(0, size, PROT_READ | PROT_WRITE,
                        MAP_PRIVATE | MAP_ANONYMOUS | MAP_32BIT, -1, 0);
    assert(memory != MAP_FAILED);
    assert(reinterpret_cast<uintptr_t>(memory) <= 0x7fffffffu);
    return static_cast<unsigned char *>(memory);
#else
    (void)size;
    assert(!"MAP_32BIT is required for the int-address entries");
    return 0;
#endif
}

static void seed_records(EdgeScan &left, EdgeScan &right) {
    memset(&left, 0, sizeof(left));
    memset(&right, 0, sizeof(right));
    left.accumA = right.accumA = 1 << 16;
    left.accumB = right.accumB = 2 << 16;
    left.accumC = 0;
    right.accumC = 2;
    left.accumD = 1 << 16;
    right.accumD = 3 << 16;
    left.deltaD = 0x5000;
    right.deltaD = 0x7000;
}

static void setup_case(unsigned char *arena, EdgeScan **left_out,
                       EdgeScan **right_out, unsigned char **source_out,
                       unsigned char **destination_out) {
    EdgeScan *left = reinterpret_cast<EdgeScan *>(arena + 0x2000);
    EdgeScan *right = reinterpret_cast<EdgeScan *>(arena + 0x2040);
    unsigned char *source = arena + 0x400;
    unsigned char *destination = arena + 0x100;
    memset(source, 0x31, 256);
    source[9] = 50;
    source[10] = 31;
    source[6] = 41;
    source[5] = 44;
    memset(destination, 0x77, 16);
    seed_records(*left, *right);

    clip_left_value = 0;
    clip_right_value = 2;
    light_accum_value = 0;
    light_step_value = 0;
    light_pack_value = 0;
    signed_pitch_value = 0;
    colour_step_value = 0;
    pixel_base_value = destination;
    row_offset_value = 0;
    buffer_stride_value = 8;
    texture_pixels_value = source;
    TextureRowPitchScratch = 4;
    remap_address_value = 0;
    shade_table_value = 0;
    shade_cursor_storage = 0;
    shade_value = 0;
    dither_address_value = 0;

    *left_out = left;
    *right_out = right;
    *source_out = source;
    *destination_out = destination;
}

static void assert_destination(const unsigned char *destination,
                               unsigned char first, unsigned char second) {
    assert(destination[0] == first);
    assert(destination[1] == second);
    for (int index = 2; index < 8; ++index) assert(destination[index] == 0x77);
}

static void snapshot_records(const EdgeScan &left, const EdgeScan &right,
                            unsigned char *left_bytes,
                            unsigned char *right_bytes) {
    memcpy(left_bytes, &left, sizeof(left));
    memcpy(right_bytes, &right, sizeof(right));
}

static void assert_records_unchanged(const EdgeScan &left,
                                     const EdgeScan &right,
                                     const unsigned char *left_bytes,
                                     const unsigned char *right_bytes) {
    assert(memcmp(left_bytes, &left, sizeof(left)) == 0);
    assert(memcmp(right_bytes, &right, sizeof(right)) == 0);
}

int main() {
#if defined(MAP_32BIT)
    unsigned char *arena = low_arena(0x10000);
    EdgeScan *left, *right;
    unsigned char *source, *destination;
    unsigned char left_bytes[sizeof(EdgeScan)];
    unsigned char right_bytes[sizeof(EdgeScan)];

    setup_case(arena, &left, &right, &source, &destination);
    snapshot_records(*left, *right, left_bytes, right_bytes);
    sub_61f800(left, right);
    assert_destination(destination, 50, 50);
    assert_records_unchanged(*left, *right, left_bytes, right_bytes);

    setup_case(arena, &left, &right, &source, &destination);
    snapshot_records(*left, *right, left_bytes, right_bytes);
    row_offset_value = static_cast<int>(reinterpret_cast<uintptr_t>(destination));
    sub_61fe70(static_cast<int>(reinterpret_cast<uintptr_t>(left)),
               static_cast<int>(reinterpret_cast<uintptr_t>(right)));
    assert_destination(destination, 50, 51);
    assert_records_unchanged(*left, *right, left_bytes, right_bytes);

    setup_case(arena, &left, &right, &source, &destination);
    snapshot_records(*left, *right, left_bytes, right_bytes);
    unsigned char *shade_table = arena + 0x800;
    int *remap = reinterpret_cast<int *>(arena + 0xc00);
    shade_table[9] = 3;
    remap[3] = 7;
    shade_table_value = shade_table;
    remap_address_value = remap;
    sub_6202e0(left, right);
    assert_destination(destination, 57, 58);
    assert_records_unchanged(*left, *right, left_bytes, right_bytes);

    setup_case(arena, &left, &right, &source, &destination);
    snapshot_records(*left, *right, left_bytes, right_bytes);
    shade_table = arena + 0x800;
    remap = reinterpret_cast<int *>(arena + 0xc00);
    unsigned char *dither = arena + 0x1000;
    memset(dither, 0, 512);
    shade_table[9] = 3;
    remap[3] = 7;
    dither[source[9]] = 1;
    shade_table_value = shade_table;
    remap_address_value = remap;
    dither_address_value = static_cast<int>(reinterpret_cast<uintptr_t>(dither));
    sub_620640(static_cast<int>(reinterpret_cast<uintptr_t>(left)),
               static_cast<int>(reinterpret_cast<uintptr_t>(right)));
    assert_destination(destination, 57, 58);
    assert_records_unchanged(*left, *right, left_bytes, right_bytes);

    // The first clip check returns before pixel writes; Gouraud entries still
    // publish both endpoint accumulators before reaching their clip gates.
    setup_case(arena, &left, &right, &source, &destination);
    clip_left_value = 2;
    clip_right_value = 4;
    snapshot_records(*left, *right, left_bytes, right_bytes);
    sub_61f800(left, right);
    assert_destination(destination, 0x77, 0x77);
    assert_records_unchanged(*left, *right, left_bytes, right_bytes);

    setup_case(arena, &left, &right, &source, &destination);
    clip_left_value = 2;
    clip_right_value = 4;
    row_offset_value = static_cast<int>(reinterpret_cast<uintptr_t>(destination));
    snapshot_records(*left, *right, left_bytes, right_bytes);
    sub_61fe70(static_cast<int>(reinterpret_cast<uintptr_t>(left)),
               static_cast<int>(reinterpret_cast<uintptr_t>(right)));
    assert_destination(destination, 0x77, 0x77);
    assert(light_accum_value == left->accumD);
    assert(light_step_value == right->accumD);
    assert_records_unchanged(*left, *right, left_bytes, right_bytes);

    setup_case(arena, &left, &right, &source, &destination);
    clip_left_value = 2;
    clip_right_value = 4;
    snapshot_records(*left, *right, left_bytes, right_bytes);
    sub_6202e0(left, right);
    assert_destination(destination, 0x77, 0x77);
    assert(light_accum_value == left->accumD);
    assert(light_step_value == right->accumD);
    assert_records_unchanged(*left, *right, left_bytes, right_bytes);

    setup_case(arena, &left, &right, &source, &destination);
    clip_left_value = 2;
    clip_right_value = 4;
    snapshot_records(*left, *right, left_bytes, right_bytes);
    sub_620640(static_cast<int>(reinterpret_cast<uintptr_t>(left)),
               static_cast<int>(reinterpret_cast<uintptr_t>(right)));
    assert_destination(destination, 0x77, 0x77);
    assert(light_accum_value == left->accumD);
    assert(light_step_value == right->accumD);
    assert_records_unchanged(*left, *right, left_bytes, right_bytes);

    // Cover left skip and right clamp together without negative fixed-point
    // deltas: C drives the scan interval while A/B keep the source texel fixed.
    setup_case(arena, &left, &right, &source, &destination);
    left->accumC = -1;
    right->accumC = 4;
    snapshot_records(*left, *right, left_bytes, right_bytes);
    sub_61f800(left, right);
    assert_destination(destination, 50, 50);
    assert_records_unchanged(*left, *right, left_bytes, right_bytes);

    setup_case(arena, &left, &right, &source, &destination);
    left->accumC = -1;
    right->accumC = 4;
    row_offset_value = static_cast<int>(reinterpret_cast<uintptr_t>(destination));
    snapshot_records(*left, *right, left_bytes, right_bytes);
    sub_61fe70(static_cast<int>(reinterpret_cast<uintptr_t>(left)),
               static_cast<int>(reinterpret_cast<uintptr_t>(right)));
    assert_destination(destination, 50, 50);
    assert_records_unchanged(*left, *right, left_bytes, right_bytes);

    setup_case(arena, &left, &right, &source, &destination);
    left->accumC = -1;
    right->accumC = 4;
    shade_table = arena + 0x800;
    remap = reinterpret_cast<int *>(arena + 0xc00);
    shade_table[9] = 3;
    remap[3] = 7;
    shade_table_value = shade_table;
    remap_address_value = remap;
    snapshot_records(*left, *right, left_bytes, right_bytes);
    sub_6202e0(left, right);
    assert_destination(destination, 6, 211);
    assert_records_unchanged(*left, *right, left_bytes, right_bytes);

    setup_case(arena, &left, &right, &source, &destination);
    left->accumC = -1;
    right->accumC = 4;
    shade_table = arena + 0x800;
    remap = reinterpret_cast<int *>(arena + 0xc00);
    dither = arena + 0x1000;
    memset(dither, 0, 512);
    shade_table[9] = 3;
    remap[3] = 7;
    dither[source[9]] = 1;
    shade_table_value = shade_table;
    remap_address_value = remap;
    dither_address_value = static_cast<int>(reinterpret_cast<uintptr_t>(dither));
    snapshot_records(*left, *right, left_bytes, right_bytes);
    sub_620640(static_cast<int>(reinterpret_cast<uintptr_t>(left)),
               static_cast<int>(reinterpret_cast<uintptr_t>(right)));
    assert_destination(destination, 6, 211);
    assert_records_unchanged(*left, *right, left_bytes, right_bytes);
#endif
    return 0;
}
'''


def _mask(source):
    return COMMENTS_OR_STRINGS.sub(
        lambda match: re.sub(r"[^\n]", " ", match[0]), source
    )


def _extract_definition(source, signature):
    view = _mask(source)
    matches = list(re.finditer(re.escape(signature), view))
    definitions = []
    for match in matches:
        opening = view.find("{", match.end())
        semicolon = view.find(";", match.end())
        if opening < 0 or (semicolon >= 0 and semicolon < opening):
            continue
        depth = 0
        for index in range(opening, len(view)):
            if view[index] == "{":
                depth += 1
            elif view[index] == "}":
                depth -= 1
                if depth == 0:
                    definitions.append(source[match.start():index + 1])
                    break
    assert len(definitions) == 1, signature
    return definitions[0]


def _extract_edgescan(source):
    view = _mask(source)
    match = re.search(r"\bstruct\s+EdgeScan\s*\{", view)
    assert match, "actual EdgeScan declaration missing"
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
                return source[match.start():end + 1]
    raise AssertionError("unterminated EdgeScan declaration")


def _definitions(source):
    definitions = [
        _extract_definition(source, signature) for signature in POINTER_HELPERS
    ]
    definitions += [
        _extract_definition(source, signature) for signature, _ in FUNCTIONS
    ]
    return definitions


def _translation_unit(source):
    fixture = FIXTURE
    assert fixture.count(DECLARATION_SLOT) == 1
    assert fixture.count(DEFINITIONS_SLOT) == 1
    fixture = fixture.replace(DECLARATION_SLOT, _extract_edgescan(HEADER.read_text()), 1)
    return fixture.replace(DEFINITIONS_SLOT, "\n\n".join(_definitions(source)), 1)


def _mutate_member(source, function_names, member, replacement):
    for _, name in FUNCTIONS:
        if name not in function_names:
            continue
        body = _extract_definition(source, f"void __cdecl {name}(")
        mutated = body.replace(f"left->{member}", f"left->{replacement}")
        mutated = mutated.replace(f"right->{member}", f"right->{replacement}")
        assert mutated != body, f"{name} did not expose {member} accesses"
        source = source.replace(body, mutated, 1)
    return source


def _compile_and_run(source, compiler_path, tmp_path, label):
    cpp = tmp_path / f"{label}.cpp"
    binary = tmp_path / label
    cpp.write_text(_translation_unit(source), encoding="utf-8")
    built = subprocess.run(
        [compiler_path, "-std=c++11", "-Wall", "-Wextra",
         "-Wno-unused-parameter", str(cpp), "-o", str(binary)],
        cwd=ROOT, check=False, capture_output=True, text=True, timeout=45,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(binary)], check=False, capture_output=True,
                          text=True, timeout=20)


def _require_low_address_host():
    if sys.platform != "linux" or platform.machine().lower() not in ("x86_64", "amd64"):
        pytest.skip("the int-address host control requires Linux x86-64 MAP_32BIT")


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
def test_four_actual_edge_raster_bodies_use_typed_fields(compiler, tmp_path):
    _require_low_address_host()
    compiler_path = shutil.which(compiler)
    if compiler_path is None:
        pytest.skip(f"{compiler} is unavailable")
    source = TEXTURE.read_text(encoding="utf-8")
    for signature, name in FUNCTIONS:
        body = _extract_definition(source, signature)
        assert "EdgeScan *left" in body and "EdgeScan *right" in body
        assert not re.search(
            r"reinterpret_cast<int \*>\((?:p1|p2|param_1|param_2)\s*\+",
            body,
        )
        assert all(f"->{member}" in body for member in
                   (("accumA", "accumB", "accumC") if name == "sub_61f800"
                    else ("accumA", "accumB", "accumC", "accumD")))
    ran = _compile_and_run(source, compiler_path, tmp_path, f"actual_{compiler}")
    assert ran.returncode == 0, ran.stdout + ran.stderr


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
def test_dword_remap_stride_mutant_fails_actual_pixel_fixture(compiler, tmp_path):
    _require_low_address_host()
    compiler_path = shutil.which(compiler)
    if compiler_path is None:
        pytest.skip(f"{compiler} is unavailable")
    source = TEXTURE.read_text(encoding="utf-8")
    before = "texture_pointer_offset(*blit_remap_table, texel * 4)"
    assert source.count(before) == 4
    mutated = source.replace(before, "texture_pointer_offset(*blit_remap_table, texel)")
    ran = _compile_and_run(mutated, compiler_path, tmp_path,
                           f"mutant_dword_remap_stride_{compiler}")
    assert ran.returncode != 0, "byte-stride table mutant unexpectedly passed"


def test_texture_mapping_identity_and_claims_survive_pointer_cell_migration():
    baseline = subprocess.check_output(
        ["git", "show", "eb5295280:src/texture.cpp"], cwd=ROOT, text=True)
    current = TEXTURE.read_text(encoding="utf-8")
    before_records = {record.address: record for record in read_text(baseline, TEXTURE)}
    after_records = {record.address: record for record in read_text(current, TEXTURE)}
    assert set(before_records) == set(after_records)
    for address, record in before_records.items():
        assert _continuity_fields(record) == _continuity_fields(after_records[address]), hex(address)
        assert record.symbol == after_records[address].symbol, hex(address)
        assert record.function_fragment == after_records[address].function_fragment, hex(address)


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
@pytest.mark.parametrize("mutation", ("accumA", "accumC", "accumD"))
def test_actual_edge_field_mutants_fail_pixel_controls(compiler, mutation, tmp_path):
    _require_low_address_host()
    compiler_path = shutil.which(compiler)
    if compiler_path is None:
        pytest.skip(f"{compiler} is unavailable")
    source = TEXTURE.read_text(encoding="utf-8")
    if mutation == "accumA":
        mutated = _mutate_member(source, {name for _, name in FUNCTIONS},
                                 "accumA", "accumB")
    elif mutation == "accumC":
        mutated = _mutate_member(source, {name for _, name in FUNCTIONS},
                                 "accumC", "accumA")
    else:
        gouraud = {"sub_61fe70", "sub_6202e0", "sub_620640"}
        mutated = _mutate_member(source, gouraud, "accumD", "deltaD")
    ran = _compile_and_run(mutated, compiler_path, tmp_path,
                           f"mutant_{mutation}_{compiler}")
    assert ran.returncode != 0, f"{mutation} mutant unexpectedly passed"
