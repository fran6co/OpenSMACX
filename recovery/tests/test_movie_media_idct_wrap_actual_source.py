"""Bounded x86-wrap controls extracted from actual decoder source.

These do not qualify x87 precision, custom-register ABI or decoder acceptance.
"""
from __future__ import annotations

import shutil
import subprocess
from pathlib import Path
import re

import pytest


ROOT = next(
    parent for parent in Path(__file__).resolve().parents
    if (parent / "src/movie_media_decoder.cpp").is_file()
)
DECODER = ROOT / "src/movie_media_decoder.cpp"
OWNER_HEADER = ROOT / "src/movie_decoder_owner.h"


def definition(source: str, signature: str) -> str:
    assert source.count(signature) == 1, f"missing/duplicate source definition: {signature}"
    start = source.index(signature)
    opening = source.index("{", start + len(signature))
    depth = 0
    for end in range(opening, len(source)):
        if source[end] == "{":
            depth += 1
        elif source[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start : end + 1]
    raise AssertionError(f"unterminated installed definition: {signature}")


def declaration(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        if source[end] == "{":
            depth += 1
        elif source[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start : end + 1]
    raise AssertionError(f"unterminated actual declaration: {signature}")


SPECS = (
    "static int *movie_media_idct_output_pointer()",
    "static void movie_media_idct_set_output_pointer(int *value)",
    "static float movie_media_idct_float_constant(int index)",
    "static movie_decoder_idct::IdctConstants movie_media_idct_constants()",
    "static DWORD movie_media_int_bits(int value)",
    "static int movie_media_signed32(DWORD value)",
    "static int movie_media_add32(int left, int right)",
    "static int movie_media_sub32(int left, int right)",
    "static int movie_media_mul2(int value)",
    "static int movie_media_signed_high_product(int left, int right)",
    "void __cdecl sub_5cee80(int *src)",
    "void __cdecl sub_5cef8a(unsigned char *row_base, int stride_arg)",
    "void __cdecl sub_5ced1d(int *esi_ptr)",
)


def host_source(source: str, mutation: str | None = None) -> str:
    bodies = [definition(source, signature) for signature in SPECS]
    idct_storage = declaration(
        OWNER_HEADER.read_text(encoding="utf-8"), "struct MediaDecoderIdctStorage"
    )
    assert "int *current_output_pointer;" in idct_storage
    if mutation == "add32_or":
        needle = "movie_media_int_bits(left) +\n                                 movie_media_int_bits(right)"
        assert bodies[6].count(needle) == 1
        bodies[6] = bodies[6].replace(
            needle,
            "movie_media_int_bits(left) |\n                                 movie_media_int_bits(right)",
            1,
        )
    elif mutation == "pitch_shift":
        needle = "static_cast<DWORD>(stride_arg) << 2;"
        assert bodies[11].count(needle) == 1
        bodies[11] = bodies[11].replace(
            needle, "static_cast<DWORD>(stride_arg) << 1;", 1
        )
    elif mutation == "advance_after_final_row":
        needle = "if (row != 7)"
        assert bodies[11].count(needle) == 1
        bodies[11] = bodies[11].replace(needle, "if (row != 8)", 1)

    return r'''#include <cassert>
#include <climits>
#include <cstdint>
#include <cstring>
#define __cdecl
using BYTE = std::uint8_t;
using DWORD = std::uint32_t;
using LONG = std::int32_t;
using __int64 = long long;
''' + idct_storage + ";" + r'''
struct MediaDecoderTableStorage { std::int32_t decoded_coefficient_words[64]; };
MediaDecoderIdctStorage movie_media_idct_storage = {};
MediaDecoderTableStorage movie_media_decoder_tables = {};
#include "movie_decoder_idct.h"
void __cdecl sub_5cee80(int *);
void __cdecl sub_5ced1d(int *);
''' + "\n".join(body for signature, body in sorted(
    zip(SPECS, bodies), key=lambda item: source.index(item[0]))) + r'''
int main() {
  // Direct modular-helper oracles exercise both signs and both boundaries.
  assert(movie_media_add32(INT_MAX, 1) == INT_MIN);
  assert(movie_media_add32(INT_MIN, -1) == INT_MAX);
  assert(movie_media_sub32(INT_MIN, 1) == INT_MAX);
  assert(movie_media_sub32(INT_MAX, -1) == INT_MIN);
  assert(movie_media_mul2(INT_MAX) == -2);
  assert(movie_media_mul2(INT_MIN) == 0);
  assert(movie_media_signed_high_product(INT_MAX, 2) == 0);
  assert(movie_media_signed_high_product(INT_MIN, 1) == -1);
  assert(movie_media_signed_high_product(INT_MIN, INT_MIN) == 1073741824);
  assert(movie_media_signed_high_product(INT_MAX, INT_MAX) == 1073741823);

  // Zero floating constants isolate the integer butterfly. Boundary inputs
  // exercise wrapped ADD/SUB in the extracted 5CEE80 body.
  int source[8] = {INT_MAX, 1, INT_MAX, INT_MAX, 1, 1, 1, INT_MAX};
  int row_output[8] = {};
  for (int i = 0; i != 5; ++i) movie_media_idct_storage.constant_words[i] = 0;
  movie_media_idct_set_output_pointer(row_output);
  sub_5cee80(source);
  const int expected[8] = {0, 2147483646, 2147483646, 0,
                           0, 2147483646, 2147483646, 0};
  for (int i = 0; i != 8; ++i) assert(row_output[i] == expected[i]);

  // Positive large stride wraps to a four-byte pitch. The allocated output
  // covers every overlapping row write; no invalid pointer walk is exercised.
  int output[16];
  for (int i = 0; i != 16; ++i) output[i] = -7;
  for (int i = 0; i != 64; ++i)
    movie_media_decoder_tables.decoded_coefficient_words[i] = 0;
  movie_media_idct_set_output_pointer(output);
  sub_5cef8a(reinterpret_cast<BYTE *>(output), 0x40000001);
  assert(movie_media_idct_storage.output_pitch_bytes == 4);
  assert(movie_media_idct_output_pointer() == output + 7);
  for (int i = 0; i != 15; ++i) assert(output[i] == 0);
  assert(output[15] == -7);
}
'''


def compile_and_run(
    compiler_name: str, tmp_path: Path, source: str, *, succeeds: bool
) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    unit = tmp_path / f"installed_{compiler_name.replace('+', 'p')}.cpp"
    binary = tmp_path / f"installed_{compiler_name.replace('+', 'p')}"
    unit.write_text(source, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror", "-pedantic",
         "-fsanitize=address,undefined", "-fno-sanitize-recover=all",
         "-iquote", str(ROOT / "src"), str(ROOT / "src/movie_decoder_idct.cpp"),
         str(unit), "-o", str(binary)],
        capture_output=True, text=True,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)
    if succeeds:
        assert ran.returncode == 0, ran.stdout + ran.stderr
    else:
        assert ran.returncode != 0, "mutated installed-source control unexpectedly passed"


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_installed_idct_wrap_helpers_and_bodies(
    compiler_name: str, tmp_path: Path
) -> None:
    compile_and_run(compiler_name, tmp_path, host_source(DECODER.read_text()), succeeds=True)


def test_host_projection_does_not_hide_helper_definition_after_first_use(tmp_path):
    source = DECODER.read_text()
    helper = definition(source, "static int movie_media_add32(int left, int right)")
    assert source.count(helper) == 1
    damaged = source.replace(helper, "", 1) + "\n" + helper + "\n"
    with pytest.raises(AssertionError, match="not declared|undeclared identifier"):
        compile_and_run("g++", tmp_path, host_source(damaged), succeeds=True)


@pytest.mark.parametrize(
    "compiler_name,mutation", [("g++", "add32_or"), ("clang++", "pitch_shift"),
                              ("g++", "advance_after_final_row"),
                              ("clang++", "advance_after_final_row")]
)
def test_installed_idct_wrap_mutations_fail(
    compiler_name: str, mutation: str, tmp_path: Path
) -> None:
    compile_and_run(
        compiler_name, tmp_path,
        host_source(DECODER.read_text(), mutation), succeeds=False,
    )


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_negative_stride_pitch_uses_actual_unsigned_shift_expression(
    compiler_name: str, tmp_path: Path
) -> None:
    body = definition(
        DECODER.read_text(),
        "void __cdecl sub_5cef8a(unsigned char *row_base, int stride_arg)",
    )
    assignment = re.search(
        r"static_cast<DWORD>\(stride_arg\)\s*<<\s*2;", body
    )
    assert assignment is not None
    source = r'''#include <cassert>
#include <climits>
#include <cstdint>
using DWORD = std::uint32_t;
DWORD pitch(int stride_arg) { return ''' + assignment.group(0) + r''' }
int main() {
  assert(pitch(-1) == 0xfffffffcu);
  assert(pitch(INT_MIN) == 0);
  assert(pitch(INT_MAX) == 0xfffffffcu);
}
'''
    compile_and_run(compiler_name, tmp_path, source, succeeds=True)
