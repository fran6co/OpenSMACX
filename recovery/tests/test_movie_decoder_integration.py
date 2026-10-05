from __future__ import annotations

import subprocess
from pathlib import Path


ROOT = Path(__file__).resolve().parent
PRODUCT = ROOT.parents[1]
SRC = PRODUCT / "src"


def _extract_mapped_blender() -> str:
    source = (SRC / "general.cpp").read_text()
    start = source.index("int __cdecl sub_5d2761(")
    end_marker = "\n}\n\n// Fixed-slot bindings carried from 005d2834.cpp"
    end = source.index(end_marker, start) + 2
    body = source[start:end]
    assert "MovieColourTableByte1" in body
    assert "MovieBlendTableHi" in body
    assert "return 0;" in body
    return body


def _extract_mapped_delta_helpers() -> str:
    source = (SRC / "general.cpp").read_text()

    def extract(signature: str) -> str:
        start = source.index(signature)
        opening = source.index("{", start)
        depth = 0
        for index in range(opening, len(source)):
            if source[index] == "{":
                depth += 1
            elif source[index] == "}":
                depth -= 1
                if depth == 0:
                    return source[start:index + 1]
        raise AssertionError(f"unterminated function {signature}")

    return (
        "#define __fastcall\n"
        + extract('extern "C" void __fastcall sub_5d2d20(')
        + "\n"
        + extract('void __fastcall sub_5d2e00(')
        + "\n"
    )


def test_combined_frame_huffman_idct_and_product_link(tmp_path: Path) -> None:
    # The mapped product body is compiled from its actual current source, with
    # test-owned table arrays substituted for its normal shared table aliases.
    # Missing proposed owner/product definitions therefore fail at compile/link.
    product = """\
#include <cstdint>
#define __cdecl
using DWORD = std::uint32_t;
static DWORD color_byte0[256] = {};
static DWORD color_byte1[256] = {};
static DWORD color_byte3[256] = {};
static DWORD blend_high[256] = {};
static DWORD blend_mid[512] = {};
static DWORD blend_low[512] = {};
static DWORD *const MovieColourTableByte0 = color_byte0;
static DWORD *const MovieColourTableByte1 = color_byte1;
static DWORD *const MovieColourTableByte3 = color_byte3;
static DWORD *const MovieBlendTableHi = blend_high;
static DWORD *const MovieBlendTableMid = blend_mid;
static DWORD *const MovieBlendTableLow = blend_low;
""" + _extract_mapped_blender()
    delta_helpers = tmp_path / "mapped_delta_helpers.cpp"
    delta_helpers.write_text(_extract_mapped_delta_helpers())
    product_path = tmp_path / "mapped_blender.cpp"
    product_path.write_text(product)
    executable = tmp_path / "combined_decoder_host"
    compile_result = subprocess.run(
        [
            "g++",
            "-std=c++17",
            "-D__fastcall=",
            "-Wall",
            "-Wextra",
            "-Werror",
            "-pedantic",
            "-iquote",
            str(SRC),
            "-iquote",
            str(SRC),
            "-iquote",
            str(SRC),
            str(SRC / "movie_decoder_frame.cpp"),
            str(SRC / "movie_decoder_huffman.cpp"),
            str(SRC / "movie_decoder_idct.cpp"),
            str(product_path),
            str(delta_helpers),
            str(ROOT / "movie_decoder_host.cpp"),
            "-o",
            str(executable),
        ],
        capture_output=True,
        text=True,
        check=False,
    )
    assert compile_result.returncode == 0, compile_result.stderr
    execution = subprocess.run(
        [str(executable)], capture_output=True, text=True, check=False
    )
    assert execution.returncode == 0, execution.stderr
