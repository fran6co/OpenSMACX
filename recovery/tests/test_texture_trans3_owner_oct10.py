"""Actual renderer controls; edge setup is a controlled input, not a proof.

Pinned 622DB0..6233C0 supplies the sequencing and pixel oracle. These tests
exercise recovered source/helpers, including compiling semantic mutants.
They do not establish complete x87 status, allocation or return-value fidelity.
"""
import pytest
from test_texture_edge_cache_actual_source import _declaration

from test_texture_sub61f0d0_actual_source import (
    SOURCE, build_and_run, extract_method, translation_unit,
)


SIGNATURE = "int Texture::draw_trans_3(Buffer *a1, Vert *a2, int *a3, int a4, Vert *a5, int a6)"
SETUP = "int Texture::setup_edge_gouraud(EdgeScan *, int index)"
CONTROLLED_SETUP = """
static EdgeScan planned_left, planned_right;
static int left_refreshes, right_refreshes;
int Texture::setup_edge_gouraud(EdgeScan *edge, int index) {
    ++setup_calls;
    assert(index == 0 || index == 2);
    if (setup_calls == 1) {
        assert(edge->f0 == -1);
        *edge = planned_left;
        return 1;
    }
    if (setup_calls == 2) {
        assert(edge->f0 == 1);
        *edge = planned_right;
        return 1;
    }
    if (edge->f0 == -1) ++left_refreshes;
    else ++right_refreshes;
    return 0;
}
"""

CASES = """
static void configure_span(int clip_left = 0, int clip_top = 0) {
    reset_case();
    left_refreshes = right_refreshes = 0;
    cell_z_key = 250;
    cell_slot_4e4 = 12345;
    cell_slot_4c8 = 67890;
    buffer.stride_ = 8;
    buffer.rect1_.left = clip_left;
    buffer.rect1_.right = 6;
    buffer.rect1_.top = clip_top;
    buffer.rect1_.bottom = 3;
    texture.iWidth_ = 8;
    std::memset(pixels, 99, sizeof(pixels));
    for (int i = 0; i < 256; ++i) texels[i] = static_cast<unsigned char>(i * 10 - 80);
    texels[10] = 250;
    std::memset(&planned_left, 0, sizeof(planned_left));
    planned_left.f0 = -1;
    planned_left.counter = 3;
    planned_left.id = 2;
    planned_left.accumA = 2 * 65536;
    planned_left.accumB = 65536;
    planned_left.accumC = 1;
    planned_left.accumD = 5 * 65536;
    planned_right = planned_left;
    planned_right.f0 = 1;
    planned_right.accumC = 5;
    planned_right.accumA = 6 * 65536;
    planned_right.accumD = 9 * 65536;
    planned_right.deltaA = 4 * 65536;
    planned_right.deltaD = 4 * 65536;
    input_vertices[0].x = 1; input_vertices[0].y = 0;
    input_vertices[1].x = 5; input_vertices[1].y = 3;
    input_vertices[2].x = 1; input_vertices[2].y = 3;
    input_z[0] = 300; input_z[1] = -300; input_z[2] = 7;
}

static void render() {
    texture.draw_trans_3(&buffer, input_vertices, input_z, 3, input_uv, 42);
    assert(input_z[0] == 256 && input_z[1] == -256 && input_z[2] == 7);
    assert(reset_calls == 2);
    assert(control_calls == 1 && release_fpu == (_PC_24 | _RC_CHOP) && current_fpu == 0);
    assert(buffer.events_.size() == 2 && buffer.events_[0] == 1 && buffer.events_[1] == 3);
    assert(left_refreshes == 1 && right_refreshes == 0);
    assert(cell_slot_4e4 == 3 * 65536); // final row, not fourth-row slopes
    assert(cell_slot_4c8 == 0);
    assert(TextureRasterAlternateRowNext.value == 3);
    assert(TextureRasterAlternateRowBase.value == 11);
    assert(cell_slot_528 == 0);
}

int main() {
    configure_span();
    render();
    // The keyed first texel leaves the destination unchanged. Even rows
    // fill; the odd row advances UV by two and light by two, not screen x.
    assert(pixels[1] == 99 && pixels[2] == 42 && pixels[3] == 42 && pixels[4] == 42);
    assert(pixels[9] == 99 && pixels[10] == 47 && pixels[11] == 69 && pixels[12] == 91);
    assert(pixels[17] == 99 && pixels[18] == 42 && pixels[19] == 42 && pixels[20] == 42);
    assert(pixels[0] == 99 && pixels[5] == 99 && pixels[13] == 99);

    configure_span(2, 1);
    render();
    // Skipping the top row still advances dither and edges. Horizontal
    // clipping advances U and light before sampling at the new start.
    for (int i = 0; i < 8; ++i) assert(pixels[i] == 99);
    assert(pixels[9] == 99 && pixels[10] == 47 && pixels[11] == 69 && pixels[12] == 91);
    assert(cell_span_right == 2);

    configure_span();
    buffer.rect1_.right = 3;
    render();
    assert(pixels[2] == 42 && pixels[3] == 99 && pixels[4] == 99);
    assert(pixels[10] == 47 && pixels[11] == 99 && pixels[12] == 99);
    assert(pixels[18] == 42 && pixels[19] == 99 && pixels[20] == 99);

    configure_span();
    buffer.rect1_.bottom = 1;
    texture.draw_trans_3(&buffer, input_vertices, input_z, 3, input_uv, 42);
    assert(setup_calls == 2 && left_refreshes == 0 && right_refreshes == 0);
    assert(pixels[2] == 42 && pixels[3] == 42 && pixels[4] == 42);
    for (int i = 8; i < 64; ++i) assert(pixels[i] == 99);
    assert(cell_slot_4e4 == 2 * 65536); // bottom stop follows next row's advancement

    configure_span();
    buffer.rect1_.bottom = 0;
    texture.draw_trans_3(&buffer, input_vertices, input_z, 3, input_uv, 42);
    assert(setup_calls == 2 && left_refreshes == 0 && right_refreshes == 0);
    assert(cell_slot_4e4 == 12345 && cell_slot_4c8 == 67890);
    for (int i = 0; i < 64; ++i) assert(pixels[i] == 99);
    assert(control_calls == 1 && release_fpu == (_PC_24 | _RC_CHOP) && current_fpu == 0);

    configure_span();
    const int before = reset_calls;
    texture.draw_trans_3(&buffer, input_vertices, input_z, 3, input_uv, -1);
    assert(forward_calls == 1 && setup_calls == 0 && buffer.events_.empty());
    assert(reset_calls == before && input_z[0] == 300);
}
"""


def unit(body: str | None = None) -> str:
    source = SOURCE.read_text()
    result = translation_unit(extract_method(source), draw_trans3=body)
    for name, header in (("EdgeScan", "texture.h"), ("Vert", "vert.h")):
        old = _declaration(result, "struct", name)
        actual = _declaration((SOURCE.parent / header).read_text(), "struct", name)
        result = result.replace(old, actual, 1)
    old_setup = extract_method(result, SETUP)
    result = result.replace(old_setup, CONTROLLED_SETUP, 1)
    start = result.index("int main() {")
    return result[:start] + CASES


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
def test_actual_renderer_rows_clipping_and_refresh_order(compiler: str) -> None:
    build_and_run(unit(), compiler, expect_success=True)


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
@pytest.mark.parametrize("old,new", (
    ("if (!done) {", "if (true) {"),
    ("TextureRasterUAccumulator.dword = left.accumA;", "TextureRasterUAccumulator.dword = right.accumA;"),
    ("TextureRasterVAccumulator.dword = left.accumB;", "TextureRasterVAccumulator.dword = right.accumD;"),
    ("const int width = pending_width >> 16;", "const int width = 1;"),
    ("if ((TextureRasterDither & 1) != 0)", "if ((TextureRasterDither & 1) == 0)"),
    ("if (TextureRasterSpanRight < TextureRasterClipLeft)", "if (false)"),
    ("if (end >= TextureRasterClipRight)", "if (false)"),
    ("if (TextureRasterRowOffset < TextureRasterClipBottom)", "if (true)"),
    ("if (TextureRasterClipBottom <= TextureRasterRowOffset) break;", "if (false) break;"),
    ("if (TextureRasterClipTop <= TextureRasterRowOffset &&", "if (true &&"),
    ("_control87(_PC_24 | _RC_CHOP, _MCW_PC | _MCW_RC);", "(void)0;"),
    ("texture_reset_fpu();", "(void)0;"),
))
def test_actual_renderer_rejects_semantic_mutants(compiler: str, old: str, new: str) -> None:
    body = extract_method(SOURCE.read_text(), SIGNATURE)
    assert old in body
    build_and_run(unit(body.replace(old, new, 1)), compiler, expect_success=False)


FILL_CONTROLS = """
static void fill_guard(bool skipped) {
    const long page = sysconf(_SC_PAGESIZE);
    assert(page > 0);
    unsigned char *mapping = static_cast<unsigned char *>(mmap(
        0, page * 2, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0));
    assert(mapping != MAP_FAILED);
    assert(mprotect(mapping + page, page, PROT_NONE) == 0);
    unsigned char *source = mapping + page - 1;
    source[0] = 23;
    unsigned char *output = static_cast<unsigned char *>(mmap(
        0, page, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0));
    assert(output != MAP_FAILED);
    output[0] = 77;
    const pid_t child = fork();
    assert(child >= 0);
    if (child == 0) {
        const rlimit limit = {0, 0};
        assert(setrlimit(RLIMIT_CORE, &limit) == 0);
        texture_draw_keyed_fill_span(output, source, 1, 0, 0, 65536, 0,
                                    1, skipped ? 23 : 0, 42);
        _exit(0);
    }
    int status;
    assert(waitpid(child, &status, 0) == child);
    assert(WIFSIGNALED(status) && WTERMSIG(status) == SIGSEGV);
    assert(output[0] == (skipped ? 77 : 42));
    assert(munmap(output, page) == 0 && munmap(mapping, page * 2) == 0);
}

int main() {
    unsigned char source[32], output[4] = {99, 99, 99, 99};
    for (int i = 0; i < 32; ++i) source[i] = static_cast<unsigned char>(20 + i);
    source[5] = 250;
    texture_draw_keyed_fill_span(output, source, 4, 0xc000, 0xc000, 0x8000, 0x8000,
                                4, 250, 42);
    assert(output[0] == 42 && output[1] == 99 && output[2] == 99 && output[3] == 42);
    unsigned char alias[5] = {7, 99, 99, 99, 99};
    texture_draw_keyed_fill_span(alias + 1, alias, 1, 0, 0, 65536, 0, 3, 42, 42);
    assert(alias[1] == 42 && alias[2] == 99 && alias[3] == 42);
    fill_guard(false);
    fill_guard(true);
}
"""


def fill_unit() -> str:
    source = unit()
    headers = (
        "#include <signal.h>\n#include <sys/mman.h>\n#include <sys/resource.h>\n"
        "#include <sys/wait.h>\n#include <unistd.h>\n"
    )
    return headers + source[:source.index("int main() {")] + FILL_CONTROLS


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
def test_actual_even_fill_carries_alias_and_trailing_read(compiler: str) -> None:
    build_and_run(fill_unit(), compiler, expect_success=True)


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
def test_even_fill_rejects_removed_trailing_preload(compiler: str) -> None:
    source = fill_unit()
    signature = "static void texture_draw_keyed_fill_span("
    body = extract_method(source, signature)
    # Remove the loop's preload, not the initial byte acquisition.
    anchor = "        source_texel = source[source_offset];"
    assert body.count(anchor) == 1
    mutant = body.replace(anchor, "        (void)source_offset;", 1)
    build_and_run(source.replace(body, mutant, 1), compiler, expect_success=False)
