from __future__ import annotations

import shutil
import subprocess
from pathlib import Path


ROOT = Path(__file__).resolve().parent
SRC = ROOT.parents[1] / "src"


def _function(source: str, signature: str) -> str:
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


def test_huffman_host_links_actual_product_delta_helpers(tmp_path: Path) -> None:
    compiler = shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        return
    general = (SRC / "general.cpp").read_text()
    helper_4x = _function(general, 'extern "C" void __fastcall sub_5d2d20(')
    helper_2x = _function(general, 'void __fastcall sub_5d2e00(')
    product = tmp_path / "mapped_delta_helpers.cpp"
    product.write_text(
        "#define __fastcall\n"
        + helper_4x
        + "\n"
        + helper_2x
        + "\n"
    )
    binary = tmp_path / "huffman_host"
    subprocess.run(
        [
            compiler,
            "-std=c++17",
            "-D__fastcall=",
            "-Wall",
            "-Wextra",
            "-Werror",
            "-pedantic",
            "-iquote",
            str(SRC),
            str(SRC / "movie_decoder_huffman.cpp"),
            str(ROOT / "movie_decoder_huffman_host.cpp"),
            str(product),
            "-o",
            str(binary),
        ],
        check=True,
    )
    subprocess.run([str(binary)], check=True)
