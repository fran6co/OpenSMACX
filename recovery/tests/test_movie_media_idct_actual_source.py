from __future__ import annotations

import shutil
import subprocess
from pathlib import Path

import pytest
from match_recovery.source import SourceRole, read_file


ROOT = next(
    parent for parent in Path(__file__).resolve().parents
    if (parent / "src" / "movie_media_decoder.cpp").is_file()
)
SRC = ROOT / "src"


def _body(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for pos in range(opening, len(source)):
        if source[pos] == "{":
            depth += 1
        elif source[pos] == "}":
            depth -= 1
            if depth == 0:
                return source[start : pos + 1]
    raise AssertionError(f"unterminated actual C++ body: {signature}")


def _actual_unit(mutant: str | None = None) -> str:
    media = (SRC / "movie_media_decoder.cpp").read_text()
    shared = (SRC / "movie_decoder_idct.cpp").read_text()
    header = (SRC / "movie_decoder_idct.h").read_text()
    signatures = (
        "static int movie_media_signed32(DWORD value)",
        "static DWORD movie_media_int_bits(int value)",
        "static int movie_media_add32(int left, int right)",
        "static int movie_media_sub32(int left, int right)",
        "static int movie_media_mul2(int value)",
        "static int movie_media_signed_high_product(int left, int right)",
        "static int *movie_media_idct_output_pointer()",
        "static void movie_media_idct_set_output_pointer(int *value)",
        "static float movie_media_idct_float_constant(int index)",
        "static movie_decoder_idct::IdctConstants movie_media_idct_constants()",
    )
    helpers = "\n\n".join(_body(media, signature) for signature in signatures)
    row = _body(media, "void __cdecl sub_5cee80(int *src)")
    column = _body(media, "void __cdecl sub_5ced1d(int *esi_ptr)")
    column = column.replace(
        "dst[row * 9] = transformed[row];",
        "movie_media_test_column_store(dst, row, transformed[row]);",
    )
    if mutant == "binary32-row":
        for old, new in (
            ("double diff_a_fp;", "float diff_a_fp;"),
            ("double diff_b_fp;", "float diff_b_fp;"),
            ("double mid;", "float mid;"),
            ("static_cast<double>(diff_a)",
             "static_cast<float>(movie_media_sub32(src[5], src[3]))"),
            ("static_cast<double>(diff_b)",
             "static_cast<float>(movie_media_sub32(src[1], src[7]))"),
        ):
            assert old in row
            row = row.replace(old, new, 1)
    elif mutant == "omitted-column-scratch":
        old = """  scratch_words[1] = half_a;
  scratch_words[2] = rotated;
  scratch_words[3] = output_b;
"""
        assert old in shared
        shared = shared.replace(old, "  (void)scratch_words; // mutant: intentionally omit proven writes\n", 1)
    elif mutant == "row-source-too-early":
        old = "LiveSourcePair load_source_pair = {input};"
        new = "SavedSourcePair load_source_pair = {input[2], input[6]};"
        assert old in shared
        shared = shared.replace(old, new, 1)
    elif mutant == "wrong-column-store-order":
        old = "const int store_order[8] = {2, 5, 3, 4, 0, 1, 6, 7};"
        assert old in column
        column = column.replace(
            old, "const int store_order[8] = {5, 2, 3, 4, 0, 1, 6, 7};", 1
        )

    prelude = r'''#include <cassert>
#include <cstdint>
#include <cstring>
using BYTE=uint8_t; using DWORD=uint32_t; using LONG=int32_t;
using __int64=long long;
#define __cdecl
struct MediaDecoderTableStorage { int32_t decoded_coefficient_words[64]; };
struct MediaDecoderIdctStorage {
  DWORD output_pitch_bytes; uintptr_t current_output_pointer_bits;
  DWORD butterfly_fp_scratch_words[4]; DWORD constant_words[5];
  int intermediate_words[72];
};
MediaDecoderTableStorage movie_media_decoder_tables={};
MediaDecoderIdctStorage movie_media_idct_storage={};
static int movie_media_test_column_store_order[8];
static int movie_media_test_column_store_count;
static void movie_media_test_column_store(int *dst, int row, int value) {
  movie_media_test_column_store_order[movie_media_test_column_store_count++] = row;
  dst[row * 9] = value;
}
'''
    main = r'''
static void set_constants() {
  movie_media_idct_storage.constant_words[0]=0x5a82799a;
  movie_media_idct_storage.constant_words[1]=0x3f0a8bd4;
  movie_media_idct_storage.constant_words[2]=0x3fa73d75;
  movie_media_idct_storage.constant_words[3]=0x3ec3ef15;
  movie_media_idct_storage.constant_words[4]=0x59c00000;
}
static bool expected_scratch() {
  return movie_media_idct_storage.butterfly_fp_scratch_words[0]==0x00ec835f &&
         movie_media_idct_storage.butterfly_fp_scratch_words[1]==0x00377e6b &&
         movie_media_idct_storage.butterfly_fp_scratch_words[2]==0xfee90381 &&
         movie_media_idct_storage.butterfly_fp_scratch_words[3]==0x009e0876;
}
int main() {
  set_constants();
  int input[8]={}; input[5]=16777217;
  int output[72]={}; movie_media_idct_set_output_pointer(output);
  sub_5cee80(input);
  if (!expected_scratch()) return 11;
  for (int i=0;i<4;++i) movie_media_idct_storage.butterfly_fp_scratch_words[i]=0xdead0000+i;
  movie_media_idct_set_output_pointer(output);
  sub_5ced1d(input);
  if (!expected_scratch()) return 12;
  const int native_column_store_order[8]={2,5,3,4,0,1,6,7};
  if (movie_media_test_column_store_count!=8) return 16;
  for(int i=0;i<8;++i)
    if(movie_media_test_column_store_order[i]!=native_column_store_order[i]) return 17;
  int dc[8]={5};
  for (int i=0;i<4;++i) movie_media_idct_storage.butterfly_fp_scratch_words[i]=0xdead0000+i;
  movie_media_idct_set_output_pointer(output);
  sub_5ced1d(dc);
  for (int i=0;i<4;++i)
    if (movie_media_idct_storage.butterfly_fp_scratch_words[i]!=0xdead0000u+(unsigned)i) return 13;

  // Source indices 2..5 overlap the shared four-word scratch window. The
  // native column path snapshots source[2] before scratch writes, reads
  // source[4] after the writes, and must retain that asymmetric ordering.
  int32_t alias_storage[16]={11,13,0x40000000,19,0x20000000,29,-0x10000000,37};
  int32_t stable_input[8];
  for(int i=0;i<8;++i) stable_input[i]=alias_storage[i];
  int32_t alias_output[8]={}, stable_output[8]={}, stable_scratch[4]={};
  bool alias_dc=movie_decoder_idct::transform_idct_column_values(
      alias_storage,alias_output,movie_media_idct_constants(),alias_storage+2);
  int32_t post_source_four=alias_storage[4];
  stable_input[4]=post_source_four;
  bool stable_dc=movie_decoder_idct::transform_idct_column_values(
      stable_input,stable_output,movie_media_idct_constants(),stable_scratch);
  if(alias_dc || stable_dc) return 14;
  for(int i=0;i<8;++i) if(alias_output[i]!=stable_output[i]) return 15;

  // The row entry differs: initial diff stores overwrite source[2] before
  // the native +8/+18 loads; source[4] is read after final scratch writes.
  int32_t row_alias_storage[16]={11,13,0x40000000,19,0x20000000,29,-0x10000000,37};
  int32_t row_stable_input[8];
  for(int i=0;i<8;++i) row_stable_input[i]=row_alias_storage[i];
  const int32_t row_late_source_two=row_alias_storage[5]-row_alias_storage[3];
  int32_t row_alias_output[8]={}, row_stable_output[8]={}, row_stable_scratch[4]={};
  movie_decoder_idct::transform_idct_row(row_alias_storage,row_alias_output,
      movie_media_idct_constants(),row_alias_storage+2);
  row_stable_input[2]=row_late_source_two;
  row_stable_input[4]=row_alias_storage[4];
  movie_decoder_idct::transform_idct_row(row_stable_input,row_stable_output,
      movie_media_idct_constants(),row_stable_scratch);
  for(int i=0;i<8;++i) if(row_alias_output[i]!=row_stable_output[i]) return 18;
  return 0;
}
'''
    return prelude + shared + "\n" + helpers + "\n" + row + "\n" + column + main


def _run(source: str, compiler: str, tmp_path: Path) -> int:
    compiler_path = shutil.which(compiler)
    if compiler_path is None:
        pytest.skip(f"{compiler} unavailable")
    cpp = tmp_path / "idct_actual.cpp"
    exe = tmp_path / "idct_actual"
    cpp.write_text(source)
    built = subprocess.run(
        [compiler_path, "-std=c++17", "-Wall", "-Wextra", "-Werror", "-pedantic",
         "-iquote", str(SRC),
         str(cpp), "-o", str(exe)], capture_output=True, text=True
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(exe)], capture_output=True, text=True).returncode


def test_idct_scaffold_identity_and_full_spans_are_continuous() -> None:
    expected = {
        0x005CED1D: ("sub_5ced1d", ((0x005CED1D, 0x005CEF8A),)),
        0x005CEE80: ("sub_5cee80", ((0x005CEE80, 0x005CEF8A),)),
        0x005CEF8A: ("sub_5cef8a", ((0x005CEF8A, 0x005CF11D),)),
    }
    rows = {row.address: row for row in read_file(SRC / "movie_media_decoder.cpp")}
    for address, (name, spans) in expected.items():
        row = rows[address]
        assert row.name == name
        assert row.image_spans == spans
        assert row.source_role is SourceRole.SCAFFOLD
    header = (SRC / "movie_decoder_idct.h").read_text()
    assert header.count("int32_t *scratch_words") == 4
    assert "scratch_words = nullptr" not in header


def test_original_scratch_alias_and_pointer_load_order_is_retained() -> None:
    media = (SRC / "movie_media_decoder.cpp").read_text()
    shared = (SRC / "movie_decoder_idct.cpp").read_text()
    row = _body(media, "void __cdecl sub_5cee80(int *src)")
    column = _body(media, "void __cdecl sub_5ced1d(int *esi_ptr)")
    transform = _body(shared, "void transform(const int32_t *source,")
    row_transform = _body(shared, "void transform_idct_row(")
    column_values = _body(shared, "bool transform_idct_column_values(")
    assert row.index("butterfly_fp_scratch_words[0] =") < row.index("diff_a_fp = static_cast<double>(diff_a)")
    assert row.index("src_two = src[2]") < row.index("&scratch_b, sizeof(scratch_b)")
    assert row.index("&scratch_b, sizeof(scratch_b)") < row.index("&scratch_a, sizeof(scratch_a)")
    assert row.index("butterfly_fp_scratch_words[3] =") < row.index(
        "sum_lo2 = movie_media_add32(src[4], src[0])"
    )
    assert row.index("dst = movie_media_idct_output_pointer()") > row.index("rot2x2 =")
    assert column.index("transform_idct_column_values(") < column.index(
        "movie_media_idct_output_pointer()"
    )
    assert column_values.index("const int32_t source_two = input[2]") < column_values.index(
        "transform(input, transformed"
    )
    assert transform.index("scratch_words[0] =") < transform.index("const double cross")
    assert transform.index("const double second_float") < transform.index(
        "load_source_pair(source_two, source_six)"
    ) < transform.index("&scratch_words[2], &second_float")
    assert "LiveSourcePair load_source_pair = {input};" in row_transform
    assert column_values.index("const int32_t source_two = input[2]") < column_values.index(
        "for (size_t index = 1;"
    )
    assert column_values.index("for (size_t index = 1;") < column_values.index(
        "SavedSourcePair load_source_pair"
    )


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_actual_idct_bodies_preserve_precision_scratch_and_dc_path(
    compiler: str, tmp_path: Path
) -> None:
    assert _run(_actual_unit(), compiler, tmp_path) == 0


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_binary32_narrowing_mutant_is_detected(compiler: str, tmp_path: Path) -> None:
    assert _run(_actual_unit("binary32-row"), compiler, tmp_path) != 0


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_omitted_shared_scratch_mutant_is_detected(compiler: str, tmp_path: Path) -> None:
    assert _run(_actual_unit("omitted-column-scratch"), compiler, tmp_path) != 0


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_row_early_snapshot_mutant_is_detected(compiler: str, tmp_path: Path) -> None:
    assert _run(_actual_unit("row-source-too-early"), compiler, tmp_path) != 0


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_wrong_column_store_order_mutant_is_detected(compiler: str, tmp_path: Path) -> None:
    assert _run(_actual_unit("wrong-column-store-order"), compiler, tmp_path) != 0


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_shared_idct_owner_stays_in_cxx98_subset(compiler: str) -> None:
    compiler_path = shutil.which(compiler)
    if compiler_path is None:
        pytest.skip(f"{compiler} unavailable")
    checked = subprocess.run(
        [compiler_path, "-std=c++98", "-Wall", "-Wextra", "-Werror", "-pedantic",
         "-iquote", str(SRC), "-fsyntax-only", str(SRC / "movie_decoder_idct.cpp")],
        capture_output=True, text=True,
    )
    assert checked.returncode == 0, checked.stdout + checked.stderr
