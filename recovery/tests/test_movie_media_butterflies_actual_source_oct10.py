"""Bounded host controls for the three moved movie IDCT butterflies.

The harness compiles the actual mapped bodies and their actual movie-media
bit/coefficient helpers. It controls storage and table contents locally; it
does not establish the original ESI/EDI ABI, caller buffer overlap, x87 control
word/80-bit equivalence, or the original allocation lifetime/bounds.
"""

from __future__ import annotations

import shutil
import subprocess
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
GENERAL = ROOT / "src/general.cpp"
DECODER = ROOT / "src/movie_media_decoder.cpp"


def definition(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for offset in range(opening, len(source)):
        if source[offset] == "{":
            depth += 1
        elif source[offset] == "}":
            depth -= 1
            if depth == 0:
                return source[start : offset + 1]
    raise AssertionError(f"unterminated definition: {signature}")


def source_fragments() -> dict[str, str]:
    decoder = DECODER.read_text(encoding="utf-8")
    return {
        "signed32": definition(decoder, "static int movie_media_signed32(DWORD value)"),
        "int_bits": definition(decoder, "static DWORD movie_media_int_bits(int value)"),
        "add32": definition(decoder, "static int movie_media_add32(int left, int right)"),
        "sub32": definition(decoder, "static int movie_media_sub32(int left, int right)"),
        "mul2": definition(decoder, "static int movie_media_mul2(int value)"),
        "high_product": definition(
            decoder, "static int movie_media_signed_high_product(int left, int right)"),
        "sar17": definition(decoder, "static int movie_media_sar17(DWORD value)"),
        "coefficient_integer": definition(
            decoder, "static int movie_media_idct_coefficient_integer(unsigned index)"),
        "coefficient_float": definition(
            decoder, "static float movie_media_idct_coefficient_float(unsigned index)"),
        "low_dword": definition(
            decoder, "static int movie_media_idct_biased_low_dword(const double &value)"),
        "row": definition(
            decoder, "extern \"C\" void __cdecl sub_5d29e1(int *src, int *dst) {"),
        "unscaled": definition(
            decoder, "extern \"C\" void __cdecl sub_5d2b20(int *dst, int *src) {"),
        "scaled": definition(
            decoder, "extern \"C\" void __cdecl sub_5d2c0a(int *dst, int *src) {"),
    }


HARNESS = r'''#include <cassert>
#include <cfloat>
#include <climits>
#include <cmath>
#include <cstdint>
#include <cstring>

using DWORD = uint32_t;
using __int64 = long long;
#define __cdecl
static_assert(LDBL_MANT_DIG >= 64, "oracle needs x87-width significand");

struct MediaCoefficientTableStorage { DWORD words[325]; };
static_assert(sizeof(MediaCoefficientTableStorage) == 0x514, "accessed window");
MediaCoefficientTableStorage movie_media_coefficient_tables = {};
DWORD unrelated_coefficient_words[325] = {};

__SIGNED32__
__INT_BITS__
__ADD32__
__SUB32__
__MUL2__
__HIGH_PRODUCT__
__SAR17__
__COEFFICIENT_INTEGER__
__COEFFICIENT_FLOAT__
__LOW_DWORD__
__ROW__
__UNSCALED__
__SCALED__

static void set_coefficients() {
    movie_media_coefficient_tables.words[0] = 0x5A82799A;
    movie_media_coefficient_tables.words[1] = 0x3F0A8BD4;
    movie_media_coefficient_tables.words[2] = 0x3FA73D75;
    movie_media_coefficient_tables.words[3] = 0x3EC3EF15;
    movie_media_coefficient_tables.words[4] = 0x59C00000;
}

// Independent instruction-shaped oracle: signed dword differences stay
// integral until converted to long double (the host's x87-like reference),
// binary32 table words are loaded separately, the final FSTP qword is modeled
// by conversion to double, and only that stored qword's low DWORD is consumed.
static int oracle_wrap(int64_t value) {
    const uint32_t bits = static_cast<uint32_t>(static_cast<uint64_t>(value));
    int result;
    std::memcpy(&result, &bits, sizeof(result));
    return result;
}

static int oracle_high(int left, int right) {
    const int64_t product = static_cast<int64_t>(left) * right;
    const uint64_t bits = static_cast<uint64_t>(product);
    const uint32_t high = static_cast<uint32_t>(bits >> 32);
    int result;
    std::memcpy(&result, &high, sizeof(result));
    return result;
}

static long double oracle_coefficient(unsigned index) {
    float value;
    std::memcpy(&value, &movie_media_coefficient_tables.words[index], sizeof(value));
    return static_cast<long double>(value);
}

static int oracle_integer_coefficient() {
    int value;
    std::memcpy(&value, &movie_media_coefficient_tables.words[0], sizeof(value));
    return value;
}

static int oracle_low_dword(long double expression) {
    const double stored_qword = static_cast<double>(expression);
    uint32_t low;
    std::memcpy(&low, &stored_qword, sizeof(low));
    int result;
    std::memcpy(&result, &low, sizeof(result));
    return result;
}

static void oracle_transform(const int *src, int *dst, bool scaled) {
    const int i9 = oracle_wrap(static_cast<int64_t>(src[3]) + src[5]);
    const int i8 = oracle_wrap(static_cast<int64_t>(src[7]) + src[1]);
    const int f2_integer = oracle_wrap(static_cast<int64_t>(src[5]) - src[3]);
    const int f3_integer = oracle_wrap(static_cast<int64_t>(src[1]) - src[7]);
    const long double f2 = static_cast<long double>(f2_integer);
    const long double f3 = static_cast<long double>(f3_integer);
    const long double f1 = (f3 + f2) * oracle_coefficient(3);
    const int i6 = oracle_wrap(static_cast<int64_t>(oracle_high(
        oracle_wrap(static_cast<int64_t>(i8) - i9),
        oracle_integer_coefficient())) * 2);
    const int first = oracle_low_dword(
        f2 * oracle_coefficient(1) + f1 + oracle_coefficient(4));
    const int second = oracle_low_dword(
        (f3 * oracle_coefficient(2) - f1) + oracle_coefficient(4));
    const int i4 = oracle_wrap(static_cast<int64_t>(first) + i6);
    const int i6b = oracle_wrap(static_cast<int64_t>(i6) + second);
    const int second_sum = oracle_wrap(
        static_cast<int64_t>(oracle_wrap(static_cast<int64_t>(second) + i9)) + i8);
    const int i9b = oracle_wrap(static_cast<int64_t>(src[4]) + src[0]);
    const int i5 = oracle_wrap(static_cast<int64_t>(src[0]) - src[4]);
    const int i7 = oracle_high(
        oracle_wrap(static_cast<int64_t>(src[2]) - src[6]),
        oracle_integer_coefficient());
    const int i8b = oracle_wrap(static_cast<int64_t>(i7) * 2);
    const int i10 = oracle_wrap(
        static_cast<int64_t>(oracle_wrap(static_cast<int64_t>(src[6]) + src[2])) + i8b);
    const int i8c = oracle_wrap(static_cast<int64_t>(i8b) + i5);
    const int i5b = oracle_wrap(static_cast<int64_t>(i5) -
                                  oracle_wrap(static_cast<int64_t>(i7) * 2));
    const int i7b = oracle_wrap(static_cast<int64_t>(i10) + i9b);
    const int i9c = oracle_wrap(static_cast<int64_t>(i9b) - i10);
    int values[8] = {
        oracle_wrap(static_cast<int64_t>(i7b) + second_sum),
        oracle_wrap(static_cast<int64_t>(i6b) + i8c),
        oracle_wrap(static_cast<int64_t>(i5b) + i4),
        oracle_wrap(static_cast<int64_t>(i9c) + first),
        oracle_wrap(static_cast<int64_t>(i9c) - first),
        oracle_wrap(static_cast<int64_t>(i5b) - i4),
        oracle_wrap(static_cast<int64_t>(i8c) - i6b),
        oracle_wrap(static_cast<int64_t>(i7b) - second_sum),
    };
    for (int index = 0; index != 8; ++index) {
        if (scaled) {
            const int value = values[index];
            values[index] = value >= 0 ? value / 131072
                                       : -1 - ((~value) / 131072);
        }
        dst[index] = values[index];
    }
}

static void oracle_row(const int *src, int *dst) {
    const int i7 = src[5], i6 = src[3], i5 = src[1], i3 = src[7];
    if (src[6] || i6 || i3 || src[2] || i7 || i5 || src[4]) {
        const int odd_a = oracle_wrap(static_cast<int64_t>(i5) - i3);
        const int odd_b = oracle_wrap(static_cast<int64_t>(i7) - i6);
        const long double f1 = (static_cast<long double>(odd_a) + odd_b) *
                               oracle_coefficient(3);
        const int i4 = oracle_wrap(static_cast<int64_t>(oracle_high(
            oracle_wrap(static_cast<int64_t>(oracle_wrap(
                static_cast<int64_t>(oracle_wrap(static_cast<int64_t>(i3) + i5)) - i6)) - i7),
            oracle_integer_coefficient())) * 2);
        const int first = oracle_low_dword(
            static_cast<long double>(oracle_wrap(static_cast<int64_t>(i7) - i6)) *
                oracle_coefficient(1) + f1 + oracle_coefficient(4));
        const int second = oracle_low_dword(
            static_cast<long double>(odd_a) * oracle_coefficient(2) - f1 +
            oracle_coefficient(4));
        const int i2 = oracle_wrap(static_cast<int64_t>(first) + i4);
        const int i4b = oracle_wrap(static_cast<int64_t>(i4) + second);
        const int second_sum = oracle_wrap(
            static_cast<int64_t>(oracle_wrap(static_cast<int64_t>(second) + i6)) + i7 + i3 + i5);
        const int i7b = oracle_wrap(static_cast<int64_t>(src[4]) + src[0]);
        const int i3b = oracle_wrap(static_cast<int64_t>(src[0]) - src[4]);
        const int i5b = oracle_high(
            oracle_wrap(static_cast<int64_t>(src[2]) - src[6]),
            oracle_integer_coefficient());
        const int i6b = oracle_wrap(static_cast<int64_t>(i5b) * 2);
        const int i8 = oracle_wrap(
            static_cast<int64_t>(oracle_wrap(static_cast<int64_t>(src[6]) + src[2])) + i6b);
        const int i6c = oracle_wrap(static_cast<int64_t>(i6b) + i3b);
        const int i3c = oracle_wrap(static_cast<int64_t>(i3b) -
                                     oracle_wrap(static_cast<int64_t>(i5b) * 2));
        const int i5c = oracle_wrap(static_cast<int64_t>(i8) + i7b);
        const int i7c = oracle_wrap(static_cast<int64_t>(i7b) - i8);
        dst[18] = oracle_wrap(static_cast<int64_t>(i3c) + i2);
        dst[45] = oracle_wrap(static_cast<int64_t>(i3c) - i2);
        dst[27] = oracle_wrap(static_cast<int64_t>(i7c) + first);
        dst[36] = oracle_wrap(static_cast<int64_t>(i7c) - first);
        dst[0] = oracle_wrap(static_cast<int64_t>(i5c) + second_sum);
        dst[9] = oracle_wrap(static_cast<int64_t>(i4b) + i6c);
        dst[54] = oracle_wrap(static_cast<int64_t>(i6c) - i4b);
        dst[63] = oracle_wrap(static_cast<int64_t>(i5c) - second_sum);
    } else {
        for (int row = 0; row != 8; ++row) dst[row * 9] = src[0];
    }
}

int main() {
    set_coefficients();
    assert(movie_media_idct_coefficient_integer(0) == 0x5A82799A);
    assert(movie_media_idct_coefficient_float(1) > 0.54f);
    assert(movie_media_idct_coefficient_float(2) > 1.30f);
    assert(movie_media_idct_coefficient_float(3) > 0.38f);
    assert(movie_media_idct_coefficient_float(4) == 6755399441055744.0f);

    // The pinned ADD/SUB/SHL and signed high-product are explicit 32-bit ops.
    assert(movie_media_add32(INT_MAX, 1) == INT_MIN);
    assert(movie_media_sub32(INT_MIN, 1) == INT_MAX);
    assert(movie_media_mul2(INT_MIN) == 0);
    assert(movie_media_signed_high_product(INT_MIN, INT_MAX) == -1073741824);
    assert(movie_media_sar17(0xFFFF0000u) == -1);

    const double bias = static_cast<double>(movie_media_idct_coefficient_float(4));
    assert(movie_media_idct_biased_low_dword(bias) == 0);
    assert(movie_media_idct_biased_low_dword(bias - 1.0) == -1);
    assert(movie_media_idct_biased_low_dword(bias + 1.25) == 1);

    // The row form has an original DC-only branch and writes only the 9-word
    // scatter positions; intervening words remain untouched.
    int source[8] = {-5, 0, 0, 0, 0, 0, 0, 0};
    int row_output[72];
    for (int &word : row_output) word = 0x12345678;
    sub_5d29e1(source, row_output);
    int expected_row[72] = {};
    oracle_row(source, expected_row);
    for (int row = 0; row != 8; ++row) {
        assert(row_output[row * 9] == expected_row[row * 9]);
        assert(row_output[row * 9] == -5);
        if (row != 7) assert(row_output[row * 9 + 1] == 0x12345678);
    }

    // Non-DC, bounded source words exercise both qword-bias/low-DWORD paths.
    // The two contiguous transforms share all intermediates; the native 2C
    // store applies arithmetic SAR 17 independently to each 2B output.
    const int non_dc[8] = {3, -9, 15, 2, -4, 7, -6, 12};
    int unscaled[8] = {};
    int scaled[8] = {};
    sub_5d2b20(unscaled, const_cast<int *>(non_dc));
    sub_5d2c0a(scaled, const_cast<int *>(non_dc));
    int expected_unscaled[8] = {}, expected_scaled[8] = {};
    oracle_transform(non_dc, expected_unscaled, false);
    oracle_transform(non_dc, expected_scaled, true);
    bool saw_nonzero = false;
    bool saw_negative = false;
    for (int index = 0; index != 8; ++index) {
        const int expected = movie_media_sar17(movie_media_int_bits(unscaled[index]));
        assert(scaled[index] == expected);
        assert(unscaled[index] == expected_unscaled[index]);
        assert(scaled[index] == expected_scaled[index]);
        saw_nonzero = saw_nonzero || unscaled[index] != 0;
        saw_negative = saw_negative || unscaled[index] < 0;
    }
    assert(saw_nonzero && saw_negative);

    // Exact pinned odd-input vector, plus large signed dwords whose low bits
    // would be lost by an FILD-to-binary32 conversion.
    const int odd_input[8] = {0, 4, 0, 0, 0, 0, 0, 0};
    int odd_row[72] = {}, odd_unscaled[8] = {}, odd_scaled[8] = {};
    sub_5d29e1(const_cast<int *>(odd_input), odd_row);
    sub_5d2b20(odd_unscaled, const_cast<int *>(odd_input));
    sub_5d2c0a(odd_scaled, const_cast<int *>(odd_input));
    const int odd_expected[8] = {8, 6, 4, 2, -2, -4, -6, -8};
    const int odd_scaled_expected[8] = {0, 0, 0, 0, -1, -1, -1, -1};
    int odd_reference[72] = {}, odd_reference_unscaled[8] = {};
    int odd_reference_scaled[8] = {};
    oracle_row(odd_input, odd_reference);
    oracle_transform(odd_input, odd_reference_unscaled, false);
    oracle_transform(odd_input, odd_reference_scaled, true);
    for (int index = 0; index != 8; ++index) {
        assert(odd_reference[index * 9] == odd_expected[index]);
        assert(odd_row[index * 9] == odd_expected[index]);
        assert(odd_reference_unscaled[index] == odd_expected[index]);
        assert(odd_unscaled[index] == odd_expected[index]);
        assert(odd_reference_scaled[index] == odd_scaled_expected[index]);
        assert(odd_scaled[index] == odd_scaled_expected[index]);
    }

    const int large_input[8] = {
        16777219, 33554431, 33554431, -33554429,
        16777223, 33554429, 33554427, -33554425
    };
    int large_row[72] = {}, large_expected_row[72] = {};
    int large_unscaled[8] = {}, large_scaled[8] = {};
    int large_expected_unscaled[8] = {}, large_expected_scaled[8] = {};
    sub_5d29e1(const_cast<int *>(large_input), large_row);
    sub_5d2b20(large_unscaled, const_cast<int *>(large_input));
    sub_5d2c0a(large_scaled, const_cast<int *>(large_input));
    oracle_row(large_input, large_expected_row);
    oracle_transform(large_input, large_expected_unscaled, false);
    oracle_transform(large_input, large_expected_scaled, true);
    for (int index = 0; index != 8; ++index) {
        assert(large_row[index * 9] == large_expected_row[index * 9]);
        assert(large_unscaled[index] == large_expected_unscaled[index]);
        assert(large_scaled[index] == large_expected_scaled[index]);
    }

    // A non-DC row preserves its sparse scatter layout and does not modify
    // the slots between the eight output words.
    for (int &word : row_output) word = 0x12345678;
    int row_source[8] = {3, -9, 15, 2, -4, 7, -6, 12};
    sub_5d29e1(row_source, row_output);
    for (int row = 0; row != 8; ++row) {
        if (row != 7) assert(row_output[row * 9 + 1] == 0x12345678);
    }

    return 0;
}
'''


def compile_and_run(tmp_path: Path,
                    fragments: dict[str, str]) -> subprocess.CompletedProcess[str]:
    compiler = shutil.which("c++") or shutil.which("clang++") or shutil.which("g++")
    assert compiler, "a host C++ compiler is required for these actual-source controls"
    source = HARNESS
    for key, value in fragments.items():
        source = source.replace(f"__{key.upper()}__", value)
    unit = tmp_path / "movie_media_butterflies.cpp"
    binary = tmp_path / "movie_media_butterflies"
    unit.write_text(source, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++17", "-O0", "-fsanitize=undefined",
         "-fno-sanitize-recover=undefined", str(unit), "-o", str(binary)],
        text=True, capture_output=True)
    assert built.returncode == 0, "actual-source control/mutant must compile: " + built.stderr
    return subprocess.run([str(binary)], text=True, capture_output=True)


def test_mapped_butterflies_are_owned_by_the_movie_decoder() -> None:
    general = GENERAL.read_text(encoding="utf-8")
    decoder = DECODER.read_text(encoding="utf-8")
    for identity, name in (
        ("0x005D29E1", "sub_5d29e1"),
        ("0x005D2B20", "sub_5d2b20"),
        ("0x005D2C0A", "sub_5d2c0a"),
    ):
        assert f"// ORIGINAL: {identity}" not in general
        assert f"void __cdecl {name}(" not in general
        assert decoder.count(f"// ORIGINAL: {identity}") == 1
    for old_alias in (
        "IdctSin45Q31", "IdctInv2CosPi8", "IdctInv2Cos3Pi8",
        "IdctCos3Pi8", "IdctAanScale", "0x009C9594",
    ):
        assert old_alias not in general
    assert "movie_media_coefficient_tables.words[index]" in decoder
    assert "movie_media_idct_biased_low_dword(d1)" in decoder
    assert "movie_media_idct_biased_low_dword(biased_first)" in decoder
    assert "movie_media_idct_biased_low_dword(biased_second)" in decoder
    assert "movie_media_add32" in decoder and "movie_media_sub32" in decoder
    assert "movie_media_signed_high_product" in decoder
    assert "movie_media_sar17" in decoder


def test_actual_bodies_compile_and_reject_behavior_mutants(tmp_path: Path) -> None:
    fragments = source_fragments()
    baseline = compile_and_run(tmp_path, fragments)
    assert baseline.returncode == 0, baseline.stdout + baseline.stderr

    mutants: tuple[tuple[str, str], ...] = (
        ("scaled", fragments["scaled"].replace(
            "movie_media_sar17(movie_media_int_bits(", "movie_media_signed32(movie_media_int_bits(")),
        ("row", fragments["row"].replace("dst[9] =", "dst[1] =")),
        ("coefficient_float", fragments["coefficient_float"].replace(
            "words[index]", "words[index + 1]")),
        ("coefficient_float", fragments["coefficient_float"].replace(
            "movie_media_coefficient_tables.words[index]",
            "unrelated_coefficient_words[index]")),
        ("add32", fragments["add32"].replace(
            "return movie_media_signed32(movie_media_int_bits(left) +\n                                 movie_media_int_bits(right));",
            "return left + right;")),
    )
    for key, mutant in mutants:
        changed = dict(fragments)
        changed[key] = mutant
        rejected = compile_and_run(tmp_path, changed)
        assert rejected.returncode != 0, f"{key} mutant unexpectedly passed"

    old_numeric_conversion = dict(fragments)
    old_numeric_conversion["low_dword"] = fragments["low_dword"].replace(
        "return movie_media_signed32(low_dword);", "return static_cast<int>(value);")
    rejected = compile_and_run(tmp_path, old_numeric_conversion)
    assert rejected.returncode != 0, "old numeric int-conversion mutant unexpectedly passed"

    fild_to_float = dict(fragments)
    fild_to_float["unscaled"] = fragments["unscaled"].replace(
        "double fVar2 = static_cast<double>(movie_media_sub32(src[5], src[3]));",
        "float fVar2 = static_cast<float>(movie_media_sub32(src[5], src[3]));")
    rejected = compile_and_run(tmp_path, fild_to_float)
    assert rejected.returncode != 0, "FILD-to-float mutant unexpectedly passed"

    float_before_bias = dict(fragments)
    float_before_bias["unscaled"] = (
        fragments["unscaled"]
        .replace("double fVar2 =", "float fVar2 =")
        .replace("double fVar3 =", "float fVar3 =")
        .replace("double fVar1 =", "float fVar1 =")
        .replace("static_cast<double>(movie_media_sub32(",
                 "static_cast<float>(movie_media_sub32(")
        .replace("static_cast<double>(movie_media_idct_coefficient_float(",
                 "static_cast<float>(movie_media_idct_coefficient_float("))
    rejected = compile_and_run(tmp_path, float_before_bias)
    assert rejected.returncode != 0, "float-before-bias mutant unexpectedly passed"

    row_fild_to_float = dict(fragments)
    row_fild_to_float["row"] = (
        fragments["row"]
        .replace("double fVar1;", "float fVar1;")
        .replace("static_cast<double>(movie_media_sub32(",
                 "static_cast<float>(movie_media_sub32("))
    rejected = compile_and_run(tmp_path, row_fild_to_float)
    assert rejected.returncode != 0, "row FILD-to-float mutant unexpectedly passed"
