"""Actual-source controls for Texture::sub_61d990 and its shade span."""
from pathlib import Path
import re
import shutil
import subprocess

import pytest
from match_recovery.source import Mode, read_file

ROOT = next(p for p in Path(__file__).resolve().parents if (p / "src/texture.cpp").is_file())
SOURCE = ROOT / "src/texture.cpp"
HEADER = ROOT / "src/texture.h"
SPAN = ((0x0061D990, 0x0061DF5C),)


def _definition(source: str, name: str, prefix: str = "static") -> str:
    match = re.search(
        rf"(?m)^{prefix}\s+[^;{{}}]*\b{re.escape(name)}\s*\([^;{{}}]*\)\s*\{{",
        source,
    )
    assert match, f"missing actual source definition: {name}"
    opening = source.index("{", match.start())
    depth = 0
    for end in range(opening, len(source)):
        if source[end] == "{":
            depth += 1
        elif source[end] == "}":
            depth -= 1
            if depth == 0:
                return source[match.start():end + 1]
    raise AssertionError(f"unterminated source definition: {name}")


def _member(source: str) -> str:
    signature = "void Texture::sub_61d990("
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        if source[end] == "{":
            depth += 1
        elif source[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start:end + 1]
    raise AssertionError("unterminated actual Texture::sub_61d990")


PREAMBLE = r"""
#include <cassert>
#include <stdint.h>
"""

OWNER_PREAMBLE = r"""
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <cassert>
#include <stdint.h>
#include <sys/mman.h>
#define _PC_24 0x0000u
#define _RC_CHOP 0x0c00u
#define _MCW_PC 0x0300u
#define _MCW_RC 0x0c00u
static int fpu_reset_count = 0;
static int fpu_control_count = 0;
static void texture_reset_fpu() { ++fpu_reset_count; }
static unsigned int _control87(unsigned int, unsigned int) {
    ++fpu_control_count;
    return 0;
}
struct RECT { int left, top, right, bottom; };
struct EdgeScan {
    int f0, counter, id, accumA, accumB, deltaA, deltaB, accumC, deltaC;
    int extraDeltaC, fracAccum, fracDelta, fracSub, accumD, deltaD;
};
static int setup_count = 0;
static int setup_edge_gouraud(EdgeScan *edge, int) {
    ++setup_count;
    edge->accumA = edge->f0 < 0 ? 0x20000 : 0x20002;
    edge->accumB = 0x10000;
    edge->accumC = edge->f0 < 0 ? 0 : 2;
    edge->accumD = edge->f0 < 0 ? 0 : 2;
    return 1;
}
static int texture_raster_step_gouraud(EdgeScan &, class Texture *);
struct Buffer {
    RECT rect1_;
    int stride_;
    void *data_;
    int get_count;
    int free_count;
    Buffer() : rect1_(), stride_(4), data_(0), get_count(0), free_count(0) {}
    void *get_data() { ++get_count; return data_; }
    void free_data(int) { ++free_count; }
};
class Texture {
public:
    unsigned char *pixels_;
    int iWidth_, iHeight_, borrowed_;
    int *corners_, *cornersAndMids_;
    Texture() : pixels_(0), iWidth_(4), iHeight_(4), borrowed_(1),
                corners_(0), cornersAndMids_(0) {}
    void sub_61d990(Buffer *, int *, int *, Texture *, unsigned char *, int);
};
static int slot_values[32] = {0};
#define SLOT(name, index) static int *const name = &slot_values[index]
static int *z_values_cell = 0;
static int **blit_z_values = &z_values_cell;
static int *edge_uvs_b_cell = 0;
static int **blit_edge_uvs_b = &edge_uvs_b_cell;
static void *pixel_base_cell;
static void **blit_pixel_base = &pixel_base_cell;
SLOT(blit_remap_table, 3);
static void *texture_pixels_cell;
static void **blit_texture_pixels = &texture_pixels_cell;
SLOT(blit_texture_height, 5);
SLOT(blit_texture_width, 6);
static int *edge_uvs_a_cell = 0;
static int **blit_edge_uvs_a = &edge_uvs_a_cell;
SLOT(blit_vertex_count, 8);
SLOT(blit_buffer_stride, 9);
static void *shade_table_cell;
static void **blit_shade_table = &shade_table_cell;
SLOT(blit_clip_right, 11);
SLOT(blit_clip_left, 12); SLOT(blit_clip_bottom, 13); SLOT(blit_clip_top, 14);
SLOT(blit_max_y_index, 15); SLOT(blit_row_offset, 16); SLOT(blit_slot_4c4, 17);
SLOT(blit_slot_530, 18); SLOT(blit_span_left, 19); SLOT(blit_slot_4f8, 20);
SLOT(blit_slot_53c, 21); SLOT(blit_span_right, 22); SLOT(blit_slot_498, 23);
SLOT(blit_slot_4e4, 24); SLOT(blit_slot_4c8, 25);
static unsigned short light_pack_words[2] = {};
static unsigned short *blit_light_pack_a = &light_pack_words[0];
#undef SLOT
static int TextureRowPitchScratch = 0;
static int texture_difference(int a, int b) {
    return static_cast<int>(static_cast<unsigned int>(a) -
                            static_cast<unsigned int>(b));
}
static int texture_span_step(int difference, int span) {
    return static_cast<int>((static_cast<int64_t>(difference) << 16) / span);
}
static uintptr_t texture_pointer_address(const void *pointer) {
    return reinterpret_cast<uintptr_t>(pointer);
}
static unsigned short texture_light_step(int difference, int span) {
    return static_cast<unsigned short>((static_cast<int64_t>(difference) << 8) / span);
}
static int texture_raster_step_gouraud(EdgeScan &, Texture *) { return 1; }
"""

OWNER_MAIN = r"""
int main() {
    assert(texture_span_pixels(0) == 1);
    assert(texture_span_pixels(-1) == 1);
    assert(texture_span_pixels(32769) == 32769);
    assert(texture_span_pixels(32770) == 1);
    unsigned char pixels[16] = {0};
    unsigned char remap[256] = {0};
    int vertices[8] = {0,1, 2,1, 2,2, 0,2};
    int z_values[4] = {0,0,0,0};
    Buffer buffer;
    buffer.rect1_.left = 0;
    buffer.rect1_.top = 2;
    buffer.rect1_.right = 4;
    buffer.rect1_.bottom = 2;
    Texture owner, shade;
    owner.pixels_ = pixels;
    owner.corners_ = vertices;
    owner.cornersAndMids_ = vertices;
    shade.pixels_ = pixels;
    owner.sub_61d990(&buffer, vertices, z_values, &shade, remap, 0);
    assert(buffer.get_count == 1);
    assert(buffer.free_count == 1);
    assert(setup_count == 2);
    assert(fpu_reset_count == 2);
    assert(fpu_control_count == 1);
    assert(*blit_vertex_count == 4);
    assert(*blit_row_offset == *blit_clip_bottom);
    assert(*blit_clip_top == *blit_clip_bottom);
    assert(*blit_z_values == z_values);
    assert(*blit_slot_4e4 == 0x10000);
    assert(*blit_slot_4c8 == 0);
    assert(*blit_light_pack_a == 0x100);
    owner.sub_61d990(0, vertices, z_values, &shade, remap, 0);
    assert(fpu_reset_count == 3);
    assert(buffer.free_count == 1);

#if defined(MAP_32BIT)
    static unsigned char pixel_store[131072];
    static unsigned char shade_store[131072];
    void *output_page = mmap(0, 65536, PROT_READ | PROT_WRITE,
                             MAP_PRIVATE | MAP_ANONYMOUS | MAP_32BIT, -1, 0);
    assert(output_page != MAP_FAILED);
    const uintptr_t pixel_page =
        (reinterpret_cast<uintptr_t>(pixel_store) + 65535u) & ~uintptr_t(0xffffu);
    const uintptr_t shade_page =
        (reinterpret_cast<uintptr_t>(shade_store) + 65535u) & ~uintptr_t(0xffffu);
    unsigned char *owner_pixels = reinterpret_cast<unsigned char *>(pixel_page + 0x35u);
    unsigned char *shade_pixels = reinterpret_cast<unsigned char *>(shade_page + 0x71u);
    reinterpret_cast<unsigned char *>(pixel_page)[0x138] = 7;
    reinterpret_cast<unsigned char *>(shade_page)[0x138] = 2;
    unsigned char remap_table[256] = {0};
    remap_table[2] = 4;
    owner.pixels_ = owner_pixels;
    shade.pixels_ = shade_pixels;
    Buffer visible;
    visible.data_ = output_page;
    visible.rect1_.left = 1;
    visible.rect1_.top = 1;
    visible.rect1_.right = 2;
    visible.rect1_.bottom = 2;
    owner.sub_61d990(&visible, vertices, z_values, &shade, remap_table, 0);
    assert(static_cast<unsigned char *>(output_page)[5] == 12);
    assert(*blit_slot_4f8 == 0x30000);
    assert(*blit_slot_53c == 0x10000);
    assert(*blit_slot_498 == 0x100);
    assert(*blit_span_right == 1);
    assert(visible.get_count == 1);
    assert(visible.free_count == 1);
    assert(munmap(output_page, 65536) == 0);
#endif
}
"""

MAIN = r"""
int main() {
    assert(texture_raster_gradient_span_admitted(1));
    assert(texture_raster_gradient_span_admitted(32767));
    assert(!texture_raster_gradient_span_admitted(32768));
    assert(!texture_raster_gradient_span_admitted(65536));
    static unsigned char pixel_storage[131072];
    static unsigned char shade_storage[131072];
    const uintptr_t pixel_page =
        (reinterpret_cast<uintptr_t>(pixel_storage) + 65535u) & ~uintptr_t(0xffffu);
    const uintptr_t shade_page =
        (reinterpret_cast<uintptr_t>(shade_storage) + 65535u) & ~uintptr_t(0xffffu);
    unsigned char *pixels = reinterpret_cast<unsigned char *>(pixel_page + 0x35u);
    unsigned char *shade = reinterpret_cast<unsigned char *>(shade_page + 0x71u);
    reinterpret_cast<unsigned char *>(pixel_page)[0x137] = 7;
    reinterpret_cast<unsigned char *>(pixel_page)[0x138] = 8;
    reinterpret_cast<unsigned char *>(shade_page)[0x137] = 2;
    reinterpret_cast<unsigned char *>(shade_page)[0x138] = 3;
    reinterpret_cast<unsigned char *>(pixel_page)[0x03a] = 44;
    reinterpret_cast<unsigned char *>(shade_page)[0x03a] = 5;
    unsigned char remap[256] = {0};
    remap[2] = 4;
    remap[3] = 1;
    remap[5] = 99;
    unsigned char output[3] = {0,0,0};
    texture_draw_shade_span(output, pixels, shade, remap, 4,
                            2 * 0x10000, 0x10000, 0x10000, 0,
                            0x20000, 0x100, 2);
    assert(output[0] == 13);  // base 7 + light 2 + mapped shade 4
    assert(output[1] == 12);  // base 8 + light 3 + mapped shade 1
    texture_draw_shade_span(output + 2, pixels, shade, remap, 1024,
                            5 * 0x10000, 0, 0, 0, 0x7f00, 0, 1);
    assert(output[2] == 99);  // selectors >=5 bypass base/light addition
}
"""


def _program(source: str) -> str:
    pieces = [PREAMBLE]
    for name in ("texture_raster_gradient_span_admitted", "texture_span_pixels"):
        pieces.append(_definition(source, name))
    pieces.extend((_definition(source, "texture_draw_shade_span"), MAIN))
    return "\n".join(pieces)


def _owner_program(source: str) -> str:
    pieces = [OWNER_PREAMBLE]
    pieces.extend(
        (_definition(source, "texture_raster_gradient_span_admitted"),
         _definition(source, "texture_span_pixels"),
         _definition(source, "texture_draw_shade_span"),
         _member(source), OWNER_MAIN)
    )
    return "\n".join(pieces)


def _run(program: str, tmp_path: Path, compiler: str) -> subprocess.CompletedProcess[str]:
    source_path = tmp_path / "texture_shade_host.cpp"
    binary_path = tmp_path / "texture_shade_host"
    source_path.write_text(program, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", str(source_path), "-o", str(binary_path)],
        text=True,
        capture_output=True,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(binary_path)], text=True, capture_output=True)


def _assert_owner_shape(source: str, header: str) -> None:
    body = _member(source)
    assert "Texture *shade_texture, unsigned char *remap" in body
    assert "int *zvals" in body
    assert "*blit_z_values = reinterpret_cast<int *>(zvals)" in body
    assert "wide_flag != 0 ? cornersAndMids_ : corners_" in body
    assert "wide_flag != 0 ? 8 : 4" in body
    acquired = body.index("buffer->get_data()")
    assert acquired < body.index("texture_reset_fpu()", acquired)
    assert body.rindex("buffer->free_data(1);") < body.rindex("texture_reset_fpu()")
    assert body.index("const EdgeScan row_left = left") < body.index(
        "texture_raster_step_gouraud(left, this)"
    )
    assert "setup_edge_gouraud(&left, top_index)" in body
    assert "setup_edge_gouraud(&right, top_index)" in body
    assert "texture_draw_shade_span(" in body
    assert "texture_raster_gradient_span_admitted(span)" in body
    assert "texture_draw_shade_span(" in body
    assert "if (clipped_span > 0)" not in body
    assert body.index("const int clipped_span") < body.index(
        "texture_draw_shade_span(", body.index("const int clipped_span")
    )
    assert "else if (!texture_raster_step_gouraud(right, this))" in body
    assert "*blit_slot_4c4 = right.accumA" in body
    assert "*blit_span_right = left.accumC" in body
    span_begin = body.index("const int span = texture_difference(")
    gradient_store = body.index("texture_raster_gradient_span_admitted(span)", span_begin)
    row_clip = body.index("if (*blit_clip_top <= *blit_row_offset", span_begin)
    assert gradient_store < row_clip
    assert "*blit_light_pack_a =" in body
    assert body.index("*blit_slot_4f8 = u") < body.index("*blit_slot_498 = light >> 8")
    assert "if (done) break;" in body
    assert "void sub_61d990(Buffer *buffer, int *vertArray, int *zvals, Texture *texB," in header
    assert "return 0;" not in body
    assert "if (buffer != 0) buffer->free_data(1);\n        texture_reset_fpu();\n        return;" in body


def test_identity_and_complete_owner_flow_are_source_owned() -> None:
    source = SOURCE.read_text(encoding="utf-8")
    header = HEADER.read_text(encoding="utf-8")
    record, = [row for row in read_file(SOURCE) if row.address == 0x0061D990]
    assert record.mode is Mode.FILE
    assert record.image_spans == SPAN
    assert not record.byte_exact
    _assert_owner_shape(source, header)


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_actual_shade_span_pixels(compiler: str, tmp_path: Path) -> None:
    if shutil.which(compiler) is None:
        pytest.skip(f"{compiler} unavailable")
    result = _run(_program(SOURCE.read_text(encoding="utf-8")), tmp_path, compiler)
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_actual_owner_host_lifecycle(compiler: str, tmp_path: Path) -> None:
    if shutil.which(compiler) is None:
        pytest.skip(f"{compiler} unavailable")
    result = _run(_owner_program(SOURCE.read_text(encoding="utf-8")), tmp_path, compiler)
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize(
    ("old", "new"),
    (
    ("if (selector >= 5)", "if (selector < 5)"),
    ("static_cast<unsigned int>(base) +\n                static_cast<unsigned int>(light_accumulator >> 8) +",
     "static_cast<unsigned int>(base) +"),
    ("remap[selector]", "remap[selector + 1]"),
    ("(source_base & ~static_cast<uintptr_t>(0xffffu)) |\n            (static_cast<uintptr_t>(source_v) << 8) | source_u",
     "source_base + (static_cast<uintptr_t>(source_v) << 8) + source_u"),
    ("(shade_base & ~static_cast<uintptr_t>(0xffffu)) |\n            (source_address & 0xffffu)",
     "shade_base + (source_address & 0xffffu)"),
    ),
)
def test_actual_shade_mutants_are_rejected(old: str, new: str, tmp_path: Path) -> None:
    source = SOURCE.read_text(encoding="utf-8")
    assert old in source
    damaged = source.replace(old, new, 1)
    result = _run(_program(damaged), tmp_path, "g++")
    assert result.returncode != 0


def test_gradient_admission_mutant_is_rejected(tmp_path: Path) -> None:
    source = SOURCE.read_text(encoding="utf-8")
    old = "return scaled > 0;"
    new = "return span > 0 && scaled != 0;"
    assert old in source
    result = _run(_program(source.replace(old, new, 1)), tmp_path, "g++")
    assert result.returncode != 0


@pytest.mark.parametrize(
    ("old", "new"),
    (
        ("wide_flag != 0 ? cornersAndMids_ : corners_", "corners_"),
        ("if (done) break;", "if (false) break;"),
        ("buffer->free_data(1);\n        texture_reset_fpu();\n        return;",
         "buffer->free_data(1);\n        return;"),
    ),
)
def test_actual_owner_mutants_are_rejected(old: str, new: str) -> None:
    source = SOURCE.read_text(encoding="utf-8")
    header = HEADER.read_text(encoding="utf-8")
    body = _member(source)
    assert old in body
    damaged = source.replace(body, body.replace(old, new, 1), 1)
    with pytest.raises(AssertionError):
        _assert_owner_shape(damaged, header)
