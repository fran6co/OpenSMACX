"""Actual-source controls for sub_622200's staged gradient publication."""

from __future__ import annotations

from pathlib import Path
import shutil
import subprocess

import pytest
from texture_direct_storage_host import direct_storage_prelude


ROOT = next(
    parent for parent in Path(__file__).resolve().parents
    if (parent / "src/IMAGE_PIN").is_file()
)
SOURCE = ROOT / "src/texture.cpp"


def owner_body() -> str:
    text = SOURCE.read_text()
    start = text.index("void Texture::sub_622200(")
    opening = text.index("{", start)
    depth = 0
    for position in range(opening, len(text)):
        if text[position] == "{":
            depth += 1
        elif text[position] == "}":
            depth -= 1
            if depth == 0:
                return text[opening + 1:position]
    raise AssertionError("sub_622200 body is not closed")


def gradient_blocks(body: str) -> list[tuple[int, int, str]]:
    blocks = []
    offset = 0
    marker = "if (0 < edgeSpan) {"
    while True:
        start = body.find(marker, offset)
        if start < 0:
            break
        block = _extract_at(body, start)
        blocks.append(block)
        offset = block[1]
    assert len(blocks) == 3, "expected initial, publish, and next-row gradient blocks"
    assert body.count("TextureRasterScratch4E4.dword = static_cast<int>(\n") == 1
    return blocks


def _extract_at(body: str, start: int) -> tuple[int, int, str]:
    opening = body.index("{", start)
    depth = 0
    for position in range(opening, len(body)):
        if body[position] == "{":
            depth += 1
        elif body[position] == "}":
            depth -= 1
            if depth == 0:
                return start, position + 1, body[start:position + 1]
    raise AssertionError("unterminated gradient publication block")


def test_actual_source_keeps_gradient_phase_and_fpu_envelope() -> None:
    body = owner_body()
    blocks = gradient_blocks(body)
    assert "_control87(_PC_24 | _RC_CHOP, _MCW_PC | _MCW_RC);" in body
    control = body.index("_control87(_PC_24 | _RC_CHOP, _MCW_PC | _MCW_RC);")
    initial = body.index("int edgeSpan = static_cast<int>(")
    assert control < initial
    first_reset = body.index("texture_reset_fpu();")
    assert body.index("->get_data();") < first_reset < control
    assert body.index("texture_reset_fpu();", body.index("free_and_finish:")) > body.index(
        "if (local_8 == 0)"
    )

    publish_start, publish_end, publish = blocks[1]
    advance = body.index("--right.counter;")
    next_span = body.index("edgeSpan = static_cast<int>(", publish_end)
    next_gradient = body.index("dU = static_cast<long double>(texture_difference(", next_span)
    pixel_row = body.index("if (TextureRasterClipBottom <= TextureRasterRowOffset)", next_gradient)
    assert advance < publish_start < publish_end < next_span < next_gradient < pixel_row

    # The original SUB/SHL pairs are 32-bit wrapping arithmetic, while all
    # three finite differences must likewise wrap before x87 conversion.
    assert body.count("static_cast<unsigned int>(texture_difference(") >= 2
    assert "texture_difference(right.accumD, left.accumD)" in body
    assert "texture_difference(right.accumB, left.accumB)" in body
    assert "texture_difference(right.accumA, left.accumA)" in body
    assert "dZ * invDx" in publish
    assert "dV * invDx" in publish
    assert "dU * invDx" in publish


def harness(block: str) -> str:
    return r"""
#include <cassert>
#include <cstring>
#include <stdint.h>
struct Vert { int x, y; };
struct EdgeScan { int accumA, accumB, accumC, accumD; };
static int texture_difference(int a, int b) {
    return static_cast<int>(static_cast<unsigned int>(a) -
                            static_cast<unsigned int>(b));
}
// TEXTURE_DIRECT_STORAGE_PRELUDE

static void run(int edgeSpan, int u_delta, int v_delta, int z_delta) {
    EdgeScan left, right;
    left.accumA = left.accumB = left.accumD = 0;
    right.accumA = z_delta;
    right.accumB = v_delta;
    right.accumD = u_delta;
    long double invDx = 1.0;
    long double dU = 1.0, dV = 1.0, dZ = 1.0;
""".replace("// TEXTURE_DIRECT_STORAGE_PRELUDE",
           direct_storage_prelude(SOURCE.read_text()), 1) + block + "\n" + r"""
}

int main() {
    const int positive_span = 4 * 0x10000;
    TextureRasterScratch4E4.dword = TextureRasterScratch4C8.dword = 0;
    TextureRasterLightPackA.low_word = 0;
    TextureRasterLightPackA.high_word = 0x5a5a;
    run(positive_span, 1030, 407, 803);
    assert(TextureRasterScratch4E4.dword == 200); // dZ * reciprocal * 65536, truncate
    assert(TextureRasterScratch4C8.dword == 101); // dV * reciprocal * 65536, truncate
    assert(static_cast<short>(TextureRasterLightPackA.low_word) == 1); // dU * reciprocal * 256
    assert(TextureRasterLightPackA.high_word == 0x5a5au);

    TextureRasterScratch4E4.dword = TextureRasterScratch4C8.dword = 0;
    TextureRasterLightPackA.low_word = 0;
    TextureRasterLightPackA.high_word = 0x5a5a;
    run(positive_span, -1030, -407, -803);
    assert(TextureRasterScratch4E4.dword == -200);
    assert(TextureRasterScratch4C8.dword == -101);
    assert(static_cast<short>(TextureRasterLightPackA.low_word) == -1);
    assert(TextureRasterLightPackA.high_word == 0x5a5au);

    TextureRasterScratch4E4.dword = 701;
    TextureRasterScratch4C8.dword = 702;
    TextureRasterLightPackA.low_word = 703;
    TextureRasterLightPackA.high_word = 0;
    run(0, 1024, 512, 256);
    assert(TextureRasterScratch4E4.dword == 701 &&
           TextureRasterScratch4C8.dword == 702 &&
           TextureRasterLightPackA.low_word == 703 &&
           TextureRasterLightPackA.high_word == 0);
}
"""


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
@pytest.mark.parametrize("damage_word_store", [False, True])
def test_actual_sub622200_gradient_store_mapping(tmp_path: Path, compiler: str, damage_word_store: bool) -> None:
    command = shutil.which(compiler)
    if command is None:
        pytest.skip(f"host compiler unavailable: {compiler}")
    blocks = gradient_blocks(owner_body())
    block = blocks[0][2] + "\n" + blocks[1][2]
    if damage_word_store:
        old = "TextureRasterLightPackA.low_word = static_cast<unsigned short>("
        assert old in block
        block = block.replace(old, "*reinterpret_cast<unsigned int *>(&TextureRasterLightPackA) = static_cast<unsigned int>(", 1)
    source = tmp_path / "sub_622200_gradient.cpp"
    executable = tmp_path / "sub_622200_gradient"
    source.write_text(harness(block))
    built = subprocess.run(
        [command, "-std=c++98", "-Wall", "-Wextra", "-Werror",
         "-Wno-unused-function", "-Wno-unused-variable", "-O0",
         str(source), "-o", str(executable)],
        capture_output=True, text=True, timeout=30,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    run_result = subprocess.run(
        [str(executable)], capture_output=True, text=True, timeout=10,
    )
    if damage_word_store:
        assert run_result.returncode != 0, "wide light-step store escaped the sentinel control"
    else:
        assert run_result.returncode == 0, run_result.stdout + run_result.stderr
