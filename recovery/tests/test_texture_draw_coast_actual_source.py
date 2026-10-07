from __future__ import annotations

import shutil
import subprocess
import tempfile
from pathlib import Path

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/texture.cpp"


def definition(source: str, signature: str) -> str:
    start = source.index(signature)
    brace = source.index("{", start)
    depth = 0
    for index in range(brace, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start : index + 1]
    raise AssertionError("unterminated definition: " + signature)


def actual_parts() -> tuple[str, str, str, str]:
    source = SOURCE.read_text(encoding="utf-8")
    signature = ("void Texture::draw_coast(Buffer * a1, Vert * a2, int * a3, "
                 "Texture * a4, unsigned char * a5, int a6, Vert * a7)")
    assert source.count(signature) == 1, "mapped coast owner must have one definition"
    helper_start = source.index("static unsigned int texture_draw_source_step(")
    helper_end = source.index("static void texture_draw_keyed_lit_span(", helper_start)
    helpers = source[helper_start:helper_end]
    result_type = definition(source, "struct TextureDrawSpanResult") + ";"
    coast = definition(source, "static void texture_draw_coast_span(")
    owner_helpers = definition(source, "static int texture_coast_advance_edge(")
    setup = definition(source, "int __cdecl Texture::setup_edge_gouraud(")
    setup = setup.replace("{", "{ ++setup_calls;", 1)
    method = definition(source, "void Texture::draw_coast(Buffer * a1,")
    return result_type + "\n" + helpers + coast, setup, owner_helpers, method


def harness(helper_text: str, setup: str, owner_helper: str, method: str) -> str:
    return r"""
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <string.h>
#define __cdecl

struct RECT { int left, top, right, bottom; };
struct Vert { int x, y; };
struct EdgeScan {
    int f0, counter, id, accumA, accumB, deltaA, deltaB, accumC, deltaC;
    int extraDeltaC, fracAccum, fracDelta, fracSub, accumD, deltaD;
};
struct Texture {
    void *pixels_;
    unsigned int iWidth_, iHeight_;
    static int setup_edge_gouraud(EdgeScan *, int);
    void draw_coast(struct Buffer *, Vert *, int *, Texture *, unsigned char *, int, Vert *);
};
struct Buffer {
    int stride_;
    RECT rect1_;
    int data;
    int free_calls;
    int get_data() { return data; }
    void free_data(int n) { assert(n == 1); ++free_calls; }
};

static int pixel_base_value, remap_value;
static int texture_pixels_value, vertex_count_value;
static int buffer_stride_value, texture_width_value, max_y_index_value;
static int row_offset_value, clip_left_value, clip_top_value, clip_right_value, clip_bottom_value;
static int step_u_value, step_v_value, coast_pixels_value;
static int slot_498_value, slot_4f8_value, slot_4c4_value, slot_530_value;
static int slot_4a4_value, slot_504_value, slot_558_value;
static int slot_53c_value, span_left_value, span_right_value;
static int *blit_pixel_base = &pixel_base_value;
static int *uv_vertices_cell = 0;
static int **blit_edge_uvs_b = &uv_vertices_cell;
static int *z_values_cell = 0;
static int **blit_z_values = &z_values_cell;
static int *blit_remap_table = &remap_value;
static int *blit_texture_pixels = &texture_pixels_value;
static int TextureRowPitchScratch;
static int texture_height_value, *blit_texture_height = &texture_height_value;
static int *blit_vertex_count = &vertex_count_value;
static int *uv_edge_cell = 0;
static int **blit_edge_uvs_a = &uv_edge_cell;
static int *blit_buffer_stride = &buffer_stride_value;
static int *blit_texture_width = &texture_width_value;
static int *blit_shade_table = &coast_pixels_value;
static int *blit_max_y_index = &max_y_index_value;
static int *blit_row_offset = &row_offset_value;
static int *blit_clip_left = &clip_left_value, *blit_clip_top = &clip_top_value;
static int *blit_clip_right = &clip_right_value, *blit_clip_bottom = &clip_bottom_value;
static int *blit_slot_4e4 = &step_u_value, *blit_slot_4c8 = &step_v_value;
static unsigned short light_pack_words[2] = {};
static unsigned short *blit_light_pack_a = &light_pack_words[0];
static int *blit_slot_498 = &slot_498_value, *blit_slot_4f8 = &slot_4f8_value;
static int *blit_slot_4c4 = &slot_4c4_value, *blit_slot_530 = &slot_530_value;
static int *blit_slot_53c = &slot_53c_value;
static int *blit_slot_4a4 = &slot_4a4_value, *blit_slot_504 = &slot_504_value;
static int *blit_slot_558 = &slot_558_value;
static int read_packed_uv_sink_for_mutation_control() { return *blit_slot_4a4; }
static int *blit_span_left = &span_left_value, *blit_span_right = &span_right_value;
static int reset_calls;
static int setup_calls;
static unsigned int control_calls, control_value, control_mask;
static void texture_reset_fpu() { ++reset_calls; }
static void _control87(unsigned value, unsigned mask) {
    ++control_calls; control_value = value; control_mask = mask;
}
static const unsigned _PC_24 = 0x200, _RC_CHOP = 0xc00;
static const unsigned _MCW_PC = 0x300, _MCW_RC = 0xc00;
static int fixed_div(int numerator, int denominator) {
    return static_cast<int>((static_cast<int64_t>(numerator) << 16) / denominator);
}
static int texture_difference(int a, int b) {
    return static_cast<int>(static_cast<unsigned int>(a) - static_cast<unsigned int>(b));
}
static int texture_span_step(int delta, int width) { return delta / width; }
static unsigned short texture_light_step(int delta, int width) {
    return static_cast<unsigned short>(delta / width);
}
static unsigned int texture_span_pixels(int delta) {
    const unsigned int n = static_cast<unsigned int>(delta) & 0xffffu;
    return n - 1u < 0x8001u ? n : 1u;
}

""" + helper_text + "\n\n" + setup + "\n\n" + owner_helper + "\n\n" + method + r"""

static unsigned char *low_memory(size_t bytes) {
    void *p = mmap(0, bytes, PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS | MAP_32BIT, -1, 0);
    assert(p != MAP_FAILED && reinterpret_cast<uintptr_t>(p) <= 0x7fffffff);
    return static_cast<unsigned char *>(p);
}

static void render_case(unsigned char mapped, unsigned char expected) {
    unsigned char *arena = low_memory(4096);
    unsigned char *canvas = arena;
    unsigned char *base_pixels = arena + 512;
    unsigned char *coast_pixels = arena + 1024;
    unsigned char *remap = arena + 1536;
    Vert *vertices = reinterpret_cast<Vert *>(arena + 2048);
    Vert *uv = reinterpret_cast<Vert *>(arena + 2112);
    int *light = reinterpret_cast<int *>(arena + 2176);
    memset(canvas, 0x5a, 64);
    memset(base_pixels, 10, 64);
    memset(coast_pixels, 1, 64);
    remap[1] = mapped;
    vertices[0] = Vert{0, 0}; vertices[1] = Vert{6, 0}; vertices[2] = Vert{0, 5};
    uv[0] = Vert{0, 0}; uv[1] = Vert{6, 0}; uv[2] = Vert{0, 5};
    light[0] = 10; light[1] = 10; light[2] = 10;
    Texture texture;
    texture.pixels_ = base_pixels; texture.iWidth_ = 8; texture.iHeight_ = 8;
    Texture coast;
    coast.pixels_ = coast_pixels; coast.iWidth_ = 8; coast.iHeight_ = 8;
    Buffer buffer;
    buffer.stride_ = 8; buffer.rect1_ = RECT{2, 2, 5, 4};
    buffer.data = static_cast<int>(reinterpret_cast<intptr_t>(canvas));
    buffer.free_calls = 0; reset_calls = 0; control_calls = 0;
    slot_4a4_value = 0x13572468; slot_504_value = 0; slot_558_value = 0;
    texture.draw_coast(&buffer, vertices, light, &coast, remap, 3, uv);
    assert(buffer.free_calls == 1 && reset_calls == 2);
    assert(texture_height_value == 8);
    assert(control_calls == 1 && control_value == (_PC_24 | _RC_CHOP) &&
           control_mask == (_MCW_PC | _MCW_RC));
    assert(slot_4a4_value != 0x13572468);
    assert(slot_504_value - static_cast<int>(reinterpret_cast<intptr_t>(base_pixels)) ==
           slot_558_value - static_cast<int>(reinterpret_cast<intptr_t>(coast_pixels)));
    assert(canvas[2 * 8 + 2] == expected);
    assert(canvas[2 * 8 + 1] == 0x5a && canvas[2 * 8 + 5] == 0x5a);
    assert(canvas[1 * 8 + 2] == 0x5a && canvas[4 * 8 + 2] == 0x5a);
    texture.draw_coast(&buffer, 0, light, &coast, remap, 3, uv);
    assert(buffer.free_calls == 2 && reset_calls == 3);
    texture.draw_coast(0, vertices, light, &coast, remap, 3, uv);
    assert(buffer.free_calls == 2 && reset_calls == 4);
    munmap(arena, 4096);
}

static void render_phase_case() {
    unsigned char *arena = low_memory(4096);
    unsigned char *canvas = arena;
    unsigned char *base_pixels = arena + 512;
    unsigned char *coast_pixels = arena + 1024;
    unsigned char *remap = arena + 1536;
    Vert *vertices = reinterpret_cast<Vert *>(arena + 2048);
    Vert *uv = reinterpret_cast<Vert *>(arena + 2112);
    int *light = reinterpret_cast<int *>(arena + 2176);
    memset(canvas, 0x5a, 64);
    for (int i = 0; i < 64; ++i) base_pixels[i] = static_cast<unsigned char>(i);
    for (int i = 0; i < 64; ++i) coast_pixels[i] = static_cast<unsigned char>((i % 4) + 1);
    remap[1] = remap[2] = remap[3] = remap[4] = 1;
    vertices[0] = Vert{0, 0}; vertices[1] = Vert{6, 0};
    vertices[2] = Vert{5, 4}; vertices[3] = Vert{0, 4};
    uv[0] = Vert{0, 0}; uv[1] = Vert{6, 0};
    uv[2] = Vert{5, 4}; uv[3] = Vert{0, 4};
    light[0] = 10; light[1] = 30; light[2] = 80; light[3] = 20;
    Texture texture;
    texture.pixels_ = base_pixels; texture.iWidth_ = 8; texture.iHeight_ = 8;
    Texture coast;
    coast.pixels_ = coast_pixels; coast.iWidth_ = 8; coast.iHeight_ = 8;
    Buffer buffer;
    buffer.stride_ = 8; buffer.rect1_ = RECT{0, 0, 8, 4};
    buffer.data = static_cast<int>(reinterpret_cast<intptr_t>(canvas));
    buffer.free_calls = 0; reset_calls = 0; control_calls = 0;
    slot_4a4_value = 0x13572468; slot_504_value = 0; slot_558_value = 0;
    texture.draw_coast(&buffer, vertices, light, &coast, remap, 4, uv);
    assert(buffer.free_calls == 1 && reset_calls == 2);
    assert(texture_height_value == 8);
    assert(control_calls == 1 && control_value == (_PC_24 | _RC_CHOP) &&
           control_mask == (_MCW_PC | _MCW_RC));
    assert(slot_4a4_value != 0x13572468);
    assert(slot_504_value - static_cast<int>(reinterpret_cast<intptr_t>(base_pixels)) ==
           slot_558_value - static_cast<int>(reinterpret_cast<intptr_t>(coast_pixels)));
    // Different edge U/V/light deltas make current-row versus next-row
    // gradient selection observable.
    assert(canvas[8 + 2] == 22);
    assert(canvas[16 + 2] == 118);
    assert(canvas[24 + 2] == 213);
    munmap(arena, 4096);
}

static void render_initially_clipped_case() {
    unsigned char *arena = low_memory(4096);
    unsigned char *canvas = arena;
    unsigned char *base_pixels = arena + 512;
    unsigned char *coast_pixels = arena + 1024;
    unsigned char *remap = arena + 1536;
    Vert *vertices = reinterpret_cast<Vert *>(arena + 2048);
    Vert *uv = reinterpret_cast<Vert *>(arena + 2112);
    int *light = reinterpret_cast<int *>(arena + 2176);
    memset(canvas, 0x5a, 64);
    memset(base_pixels, 10, 64);
    memset(coast_pixels, 1, 64);
    remap[1] = 1;
    vertices[0] = Vert{0, 5}; vertices[1] = Vert{6, 5}; vertices[2] = Vert{0, 7};
    uv[0] = Vert{0, 5}; uv[1] = Vert{6, 5}; uv[2] = Vert{0, 7};
    light[0] = light[1] = light[2] = 10;
    Texture texture;
    texture.pixels_ = base_pixels; texture.iWidth_ = 8; texture.iHeight_ = 8;
    Texture coast;
    coast.pixels_ = coast_pixels; coast.iWidth_ = 8; coast.iHeight_ = 8;
    Buffer buffer;
    buffer.stride_ = 8; buffer.rect1_ = RECT{0, 0, 8, 4};
    buffer.data = static_cast<int>(reinterpret_cast<intptr_t>(canvas));
    buffer.free_calls = 0; reset_calls = 0; setup_calls = 0; control_calls = 0;
    slot_4a4_value = 0x13572468; slot_504_value = 0x24681357; slot_558_value = 0x35792468;
    slot_4c4_value = 0x12345678; slot_4f8_value = 0x23456789;
    texture.draw_coast(&buffer, vertices, light, &coast, remap, 3, uv);
    assert(buffer.free_calls == 1 && reset_calls == 2);
    assert(setup_calls == 2);
    assert(control_calls == 1 && control_value == (_PC_24 | _RC_CHOP) &&
           control_mask == (_MCW_PC | _MCW_RC));
    assert(slot_4a4_value == 0x13572468 && slot_504_value == 0x24681357 &&
           slot_558_value == 0x35792468);
    assert(slot_4c4_value == 0x12345678 && slot_4f8_value == 0x23456789);
    for (int i = 0; i < 64; ++i) assert(canvas[i] == 0x5a);
    munmap(arena, 4096);
}

int main() {
    render_case(0, 0x5a);     // mapped zero preserves the destination
    render_case(1, 20);       // mapped one is base texel plus CH
    render_case(0xe7, 0xe7);  // other mapped values replace the destination
    render_phase_case();
    render_initially_clipped_case();
    return 0;
}
"""


def compile_and_run(compiler: str, text: str) -> subprocess.CompletedProcess[str]:
    with tempfile.TemporaryDirectory(prefix="texture-coast-host-") as temporary:
        cpp = Path(temporary) / "control.cpp"
        exe = Path(temporary) / "control"
        cpp.write_text(text, encoding="utf-8")
        built = subprocess.run(
            [compiler, "-std=c++11", "-D_GNU_SOURCE", "-Wall", "-Wextra",
             "-Werror", "-Wno-unused-function", str(cpp), "-o", str(exe)],
            text=True, capture_output=True, timeout=30,
        )
        if built.returncode:
            return built
        return subprocess.run([str(exe)], text=True, capture_output=True, timeout=10)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_coast_body_clips_and_dispatches_mapping(compiler_name: str) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")
    helper, setup, owner_helper, method = actual_parts()
    result = compile_and_run(compiler, harness(helper, setup, owner_helper, method))
    assert result.returncode == 0, result.stdout + result.stderr

    broken_method = method.replace(
        "clipped_left = blit_clip_left[0]", "clipped_left = left_x", 1)
    assert broken_method != method
    mutant = compile_and_run(compiler, harness(helper, setup, owner_helper, broken_method))
    assert mutant.returncode != 0
    assert "Assertion" in mutant.stderr

    # A tempting transcription derives the row slopes from the next edge
    # endpoints after advancement. The varying multi-row fixture rejects it.
    update_start = method.index("        if (update_steps) {")
    done_start = method.index("        bool done_edges = false;", update_start)
    publish_start = method.index("        if (update_steps) {", done_start)
    current_edge_gradients = method[update_start:done_start]
    wrong_phase = method[:update_start] + method[done_start:publish_start]
    wrong_phase += current_edge_gradients + method[publish_start:]
    assert wrong_phase != method
    phase_mutant = compile_and_run(compiler, harness(helper, setup, owner_helper, wrong_phase))
    assert phase_mutant.returncode != 0
    assert "Assertion" in phase_mutant.stderr

    initial_guard = "    if (blit_row_offset[0] >= blit_clip_bottom[0]) break;\n"
    assert method.count(initial_guard) == 1
    no_initial_guard = method.replace(initial_guard, "", 1)
    clipped_mutant = compile_and_run(compiler, harness(helper, setup, owner_helper, no_initial_guard))
    assert clipped_mutant.returncode != 0
    assert "Assertion" in clipped_mutant.stderr

    no_packed_sink = method.replace(
        "span_result.packed_uv_sink = blit_slot_4a4;",
        "span_result.packed_uv_sink = 0;", 1)
    assert no_packed_sink != method
    scratch_mutant = compile_and_run(compiler, harness(helper, setup, owner_helper, no_packed_sink))
    assert scratch_mutant.returncode != 0
    assert "Assertion" in scratch_mutant.stderr


def test_actual_owner_retains_original_identity_and_records_prefetch_debt() -> None:
    source = SOURCE.read_text(encoding="utf-8")
    identity = source.index("// ORIGINAL: 0x0061EAD0")
    body = source.index("void Texture::draw_coast(Buffer * a1,", identity)
    assert "0x0061EAD0-0x0061F0C6" in source[identity:body]
    assert "reads the next source texel at 0x61F078 after every" in source
    assert "span_result.preload_pixels = reinterpret_cast<volatile const unsigned char *>" in source[body:body + 14000]
    assert "span_result.prefetched_source =" in source[body:body + 14000]
    assert "texture_draw_coast_span" in source[body:body + 14000]
