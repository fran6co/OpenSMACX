"""Host controls for the distinct Texture raster entry at 0x61FA30."""
from pathlib import Path
import re
import shutil
import subprocess

import pytest
from match_recovery.source import Mode, read_file
from match_recovery.includes import COMMENTS_OR_STRINGS

ROOT = next(p for p in Path(__file__).resolve().parents if (p / "src/texture.cpp").is_file())
SOURCE = ROOT / "src/texture.cpp"
SPAN = ((0x0061FA30, 0x0061FC86),)


def _definition(source: str, name: str) -> str:
    masked = COMMENTS_OR_STRINGS.sub(
        lambda match: re.sub(r"[^\n]", " ", match[0]), source
    )
    match = re.search(
        rf"(?m)^(?:static[ \t]+)?[^\n;{{}}]*\b{re.escape(name)}\s*\([^;{{}}]*\)\s*\{{",
        masked,
    )
    assert match, f"missing actual source definition: {name}"
    opening = masked.index("{", match.start())
    depth = 0
    for end in range(opening, len(source)):
        if masked[end] == "{":
            depth += 1
        elif masked[end] == "}":
            depth -= 1
            if depth == 0:
                return source[match.start():end + 1]
    raise AssertionError(f"unterminated source definition: {name}")


def _struct(source: str, name: str) -> str:
    start = source.index(f"struct {name} {{")
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        if source[end] == "{":
            depth += 1
        elif source[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start:end + 1] + ";"
    raise AssertionError(f"unterminated source struct: {name}")


PREAMBLE = r"""
#include <cassert>
#include <stdint.h>
struct EdgeScan {
    int f0, counter, id, accumA, accumB, deltaA, deltaB, accumC, deltaC;
    int extraDeltaC, fracAccum, fracDelta, fracSub, accumD, deltaD;
};
static int texture_difference(int a, int b) {
    return static_cast<int>(static_cast<unsigned int>(a) - static_cast<unsigned int>(b));
}
"""

MAIN = r"""
static void run_diagonal_key_and_pitch() {
    unsigned char pixels[64];
    for (int i = 0; i < 64; ++i) pixels[i] = static_cast<unsigned char>(i + 1);
    pixels[9] = 0xee;
    unsigned char output[8] = {70, 71, 72, 73, 74, 75, 76, 77};
    int signed_pitch = 0, packed_step = 0, colour_step = 0;
    TextureKeyedEdgeSpanState state = {output, pixels, 8, 0, 8, 0xee,
                                       &signed_pitch, &packed_step, &colour_step};
    EdgeScan left = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    EdgeScan right = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    left.accumA = 0;
    left.accumB = 0;
    left.accumC = 0;
    right.accumA = 3 * 0x10000;
    right.accumB = 3 * 0x10000;
    right.accumC = 3;
    texture_draw_keyed_edge_span(&left, &right, state);
    assert(output[0] == 70);  // first sample is the key texel
    assert(output[1] == pixels[18]);
    assert(output[2] == pixels[27]);
    assert(signed_pitch == 8);
    assert(colour_step == 9); // one U plus one V row per pixel
    assert(packed_step == 0); // integral one-pixel slopes have no packed fraction
}

static void run_left_clip_and_negative_v() {
    unsigned char pixels[64];
    for (int i = 0; i < 64; ++i) pixels[i] = static_cast<unsigned char>(i + 1);
    unsigned char storage[9] = {0x55, 80, 81, 82, 83, 84, 85, 86, 87};
    unsigned char *output = storage + 1;
    int signed_pitch = 0, packed_step = 0, colour_step = 0;
    TextureKeyedEdgeSpanState state = {output, pixels, 8, 0, 8, 0xff,
                                       &signed_pitch, &packed_step, &colour_step};
    EdgeScan left = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    EdgeScan right = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    left.accumA = 0;
    left.accumB = 3 * 0x10000;
    left.accumC = -1;
    right.accumA = 4 * 0x10000;
    right.accumB = 0;
    right.accumC = 3;
    texture_draw_keyed_edge_span(&left, &right, state);
    assert(output[0] == pixels[18]);
    assert(output[1] == pixels[19]);
    assert(output[2] == pixels[12]);
    assert(storage[0] == 0x55);  // clipped-out destination remains untouched
    assert(signed_pitch == -8);
}

static void run_negative_fraction_rounding() {
    unsigned char pixels[16];
    for (int i = 0; i < 16; ++i) pixels[i] = static_cast<unsigned char>(i + 1);
    unsigned char output[2] = {90, 91};
    int signed_pitch = 0, packed_step = 0, colour_step = 0;
    TextureKeyedEdgeSpanState state = {output, pixels, 4, 0, 4, 0xff,
                                       &signed_pitch, &packed_step, &colour_step};
    EdgeScan left = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    EdgeScan right = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    left.accumA = 0x4000;
    left.accumB = 0;
    left.accumC = 0;
    right.accumA = -0x4000;
    right.accumB = 0;
    right.accumC = 1;
    texture_draw_keyed_edge_span(&left, &right, state);
    assert(output[0] == pixels[1]);
    assert(packed_step != 0);
}

static void run_native_15_bit_v_fraction() {
    unsigned char pixels[16];
    for (int i = 0; i < 16; ++i) pixels[i] = static_cast<unsigned char>(i + 10);
    unsigned char output[2] = {0, 0};
    int signed_pitch = 0, packed_step = 0, colour_step = 0;
    TextureKeyedEdgeSpanState state = {output, pixels, 4, 0, 4, 0xff,
                                       &signed_pitch, &packed_step, &colour_step};
    EdgeScan left = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    EdgeScan right = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    left.accumA = 0;
    left.accumB = 0x7fff;   // half-pixel seed becomes 0xffff
    left.accumC = 0;
    right.accumA = 0;
    right.accumB = 0x8001;  // V step is one low-bit unit per pixel
    right.accumC = 2;
    texture_draw_keyed_edge_span(&left, &right, state);
    assert(output[0] == pixels[0]);
    assert(output[1] == pixels[0]); // native >>=1 packing does not carry pitch
}

static void run_wrapped_native_divisor() {
    const unsigned int divisor_bits = static_cast<unsigned int>(40000) << 16;
    const int divisor = static_cast<int>(divisor_bits);
    const int64_t numerator = static_cast<int64_t>(0x40000000) * 0x10000;
    assert(texture_keyed_span_quotient(0x40000000, 40000) ==
           static_cast<int>(numerator / divisor));
    assert(texture_keyed_span_quotient(0x40000000, 40000) !=
           0x40000000 / 40000);
}

static void run_negative_u_packed_direction() {
    unsigned char pixels[16];
    for (int i = 0; i < 16; ++i) pixels[i] = static_cast<unsigned char>(i + 30);
    unsigned char output[2] = {0, 0};
    int signed_pitch = 0, packed_step = 0, colour_step = 0;
    TextureKeyedEdgeSpanState state = {output, pixels, 4, 0, 4, 0xff,
                                       &signed_pitch, &packed_step, &colour_step};
    EdgeScan left = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    EdgeScan right = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    left.accumA = 2 * 0x10000;
    left.accumB = 0;
    left.accumC = 0;
    right.accumA = 0;
    right.accumB = 0;
    right.accumC = 2;
    texture_draw_keyed_edge_span(&left, &right, state);
    assert(output[0] == pixels[2]);
    assert(output[1] == pixels[5]);
}

static void run_positive_u_negative_one_v_step() {
    unsigned char pixels[16];
    for (int i = 0; i < 16; ++i) pixels[i] = static_cast<unsigned char>(i + 50);
    unsigned char output[1] = {0};
    int signed_pitch = 0, packed_step = 0, colour_step = 0;
    TextureKeyedEdgeSpanState state = {output, pixels, 4, 0, 4, 0xff,
                                       &signed_pitch, &packed_step, &colour_step};
    EdgeScan left = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    EdgeScan right = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    left.accumA = 0;
    left.accumB = 0;
    left.accumC = 0;
    right.accumA = 0;
    right.accumB = -2;
    right.accumC = 1;
    texture_draw_keyed_edge_span(&left, &right, state);
    assert(output[0] == pixels[0]);
    assert(static_cast<unsigned int>(packed_step) == 0x00000001u);
    assert(signed_pitch == -4);
}

static void run_negative_pitch_prepack_transform_positive_u() {
    unsigned char pixels[16];
    for (int i = 0; i < 16; ++i) pixels[i] = static_cast<unsigned char>(i + 90);
    unsigned char output[2] = {0, 0};
    int signed_pitch = 0, packed_step = 0, colour_step = 0;
    TextureKeyedEdgeSpanState state = {output, pixels, 4, 0, 4, 0xff,
                                       &signed_pitch, &packed_step, &colour_step};
    EdgeScan left = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    EdgeScan right = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    left.accumA = 0;
    left.accumB = 0;
    left.accumC = 0;
    right.accumA = 0;
    right.accumB = -2;
    right.accumC = 2;
    texture_draw_keyed_edge_span(&left, &right, state);
    assert(signed_pitch == -4);
    assert(static_cast<unsigned int>(packed_step) == 0x00000000u);
    assert(output[0] == pixels[0]);
    assert(output[1] == pixels[0]);
}

static void run_negative_pitch_prepack_transform_negative_u() {
    unsigned char pixels[32];
    for (int i = 0; i < 32; ++i) pixels[i] = static_cast<unsigned char>(i + 110);
    unsigned char output[2] = {0, 0};
    int signed_pitch = 0, packed_step = 0, colour_step = 0;
    TextureKeyedEdgeSpanState state = {output, pixels, 4, 0, 4, 0xff,
                                       &signed_pitch, &packed_step, &colour_step};
    EdgeScan left = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    EdgeScan right = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    left.accumA = 2 * 0x10000;
    left.accumB = 0;
    left.accumC = 0;
    right.accumA = 0;
    right.accumB = -2;
    right.accumC = 2;
    texture_draw_keyed_edge_span(&left, &right, state);
    assert(signed_pitch == -4);
    assert(static_cast<unsigned int>(packed_step) == 0x00000000u);
    assert(output[0] == pixels[2]);
}

static void run_native_16_bit_initial_row_product() {
    static unsigned char pixels[80000];
    for (int i = 0; i < 80000; ++i) pixels[i] = 0;
    pixels[11264] = 0x6d;
    pixels[76800] = 0x3a;
    unsigned char output[1] = {0};
    int signed_pitch = 0, packed_step = 0, colour_step = 0;
    TextureKeyedEdgeSpanState state = {output, pixels, 256, 0, 1, 0xff,
                                       &signed_pitch, &packed_step, &colour_step};
    EdgeScan left = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    EdgeScan right = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    left.accumA = 0;
    left.accumB = 300 * 0x10000;
    left.accumC = 0;
    right.accumA = left.accumA;
    right.accumB = left.accumB;
    right.accumC = 1;
    texture_draw_keyed_edge_span(&left, &right, state);
    assert(output[0] == 0x6d);
}

int main() {
    run_diagonal_key_and_pitch();
    run_left_clip_and_negative_v();
    run_negative_fraction_rounding();
    run_native_15_bit_v_fraction();
    run_wrapped_native_divisor();
    run_negative_u_packed_direction();
    run_positive_u_negative_one_v_step();
    run_negative_pitch_prepack_transform_positive_u();
    run_negative_pitch_prepack_transform_negative_u();
    run_native_16_bit_initial_row_product();
}
"""


def _program(source: str) -> str:
    pieces = [PREAMBLE]
    for name in ("texture_keyed_initial_source_index",):
        pieces.append(_definition(source, name))
    pieces.extend(
        (
            _struct(source, "TextureKeyedEdgeSpanState"),
            _definition(source, "texture_keyed_shift_product"),
            _definition(source, "texture_keyed_wrap_add"),
            _definition(source, "texture_keyed_span_quotient"),
            _definition(source, "texture_draw_keyed_edge_span"),
            MAIN,
        )
    )
    return "\n".join(pieces)


def _run(program: str, tmp_path: Path, compiler: str) -> subprocess.CompletedProcess[str]:
    source_path = tmp_path / "texture_keyed_edge_host.cpp"
    binary_path = tmp_path / "texture_keyed_edge_host"
    source_path.write_text(program, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", str(source_path), "-o", str(binary_path)],
        text=True,
        capture_output=True,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(binary_path)], text=True, capture_output=True)


def test_new_identity_boundary_and_owner_adapter() -> None:
    source = SOURCE.read_text(encoding="utf-8")
    record, = [row for row in read_file(SOURCE) if row.address == 0x0061FA30]
    assert record.mode is Mode.FILE
    assert record.image_spans == SPAN
    assert not record.byte_exact
    entry = _definition(source, "sub_61fa30")
    assert entry.startswith("void __cdecl sub_61fa30(")
    for token in (
        "EdgeScan *left", "EdgeScan *right", "TextureRasterPixelBase",
        "TextureRasterRowOffset", "TextureRasterBufferStride", "TextureRowPitchScratch",
        "TextureRasterScratch544", "TextureRasterLightThreshold",
        "TextureRasterSignedPitch", "TextureRasterPackedStep",
        "TextureRasterScratch4B8",
    ):
        assert token in entry


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_actual_source_kernel_executes_span_clip_key_and_directions(
    compiler: str, tmp_path: Path
) -> None:
    if shutil.which(compiler) is None:
        pytest.skip(f"{compiler} unavailable")
    source = SOURCE.read_text(encoding="utf-8")
    result = _run(_program(source), tmp_path, compiler)
    assert result.returncode == 0, result.stdout + result.stderr


def test_scratch_publication_follows_native_acquisition_order() -> None:
    source = SOURCE.read_text(encoding="utf-8")
    helper = _definition(source, "texture_draw_keyed_edge_span")
    assert helper.index("*state.signed_pitch_slot =") < helper.index("int clipped_left =")
    assert helper.index("texture_keyed_span_quotient(") < helper.index("const int u_direction")
    assert "raw_v_fraction >> 1" in helper
    assert helper.index("texture_keyed_initial_source_index(") < helper.index(
        "if (*state.signed_pitch_slot < 0)"
    )
    assert helper.index("if (*state.signed_pitch_slot < 0)") < helper.index(
        "const unsigned int packed_u ="
    )
    assert "0u - static_cast<unsigned int>(raw_v_fraction)" in helper
    assert "v_fraction = static_cast<unsigned short>(~v_fraction)" in helper
    assert "u_direction < 0 ? v_fraction" in helper
    assert "if ((packed_uv & 0x8000u) != 0)" in helper
    assert "texture_keyed_initial_source_index(\n        state.pitch, u, v)" in helper
    assert helper.index("texture_keyed_initial_source_index(") < helper.index(
        "*state.packed_step_slot ="
    )
    assert helper.index("*state.packed_step_slot =") < helper.index("*state.colour_step_slot =")


@pytest.mark.parametrize(
    ("old", "new"),
    (
        ("if (texel != state.key) destination[0] = texel;", "destination[0] = texel;"),
        ("static_cast<unsigned int>(pixel_offset) << 16",
         "static_cast<unsigned int>(pixel_offset)"),
        ("raw_v_fraction >> 1", "raw_v_fraction"),
        ("if (*state.signed_pitch_slot < 0)", "if (false)"),
        ("if ((packed_uv & 0x8000u) != 0)", "if (false)"),
        ("static_cast<unsigned int>(pixel_span) << 16",
         "static_cast<unsigned int>(pixel_span)"),
        ("u_direction < 0 ? v_fraction",
         "u_direction < 0 ? static_cast<unsigned short>(v_fraction >> 1)"),
        ("const unsigned short row_product = static_cast<unsigned short>(\n        static_cast<unsigned int>(v_integer) * pitch_word);",
         "const unsigned int row_product = static_cast<unsigned int>(\n        static_cast<unsigned int>(v_integer) * pitch_word);"),
        ("if (clipped_left < state.clip_left)", "if (clipped_left > state.clip_left)"),
        ("if (du < 0 && (static_cast<unsigned int>(du) & 0xffffu) != 0)",
         "if (du < 0 && false)"),
    ),
)
def test_actual_source_mutations_are_rejected(old: str, new: str, tmp_path: Path) -> None:
    source = SOURCE.read_text(encoding="utf-8")
    assert old in source
    damaged = source.replace(old, new, 1)
    program = _program(damaged)
    if shutil.which("g++") is not None:
        # Mutants may compile; the semantic cases must expose them.
        result = _run(program, tmp_path, "g++")
        assert result.returncode != 0
