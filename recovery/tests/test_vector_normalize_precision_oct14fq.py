"""Actual Vector normalization precision and legacy edge behavior.

The fixture compiles the production Vector header and translation unit, with
only the platform precompiled-header include omitted. Host tests exercise
source behavior, not the original x87 instruction schedule or native ABI.
"""

from pathlib import Path
import hashlib
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
ORIGINAL = (
    "// ORIGINAL: 0x00634520 ?normalize@Vector@@QAE?AV1@XZ "
    "0x00634520-0x006345DD"
)
NORMALIZE_BINDING = (
    '// RECOVERY_BINDING: v1 {"evidence": ["Pinned original0x00634531-'
    '0x00634567 computes (x*x+y*y)+z*z with no intermediate float stores, '
    'passes a double to sqrt0x00646544, and0x0063456F-0x0063457D tests x87 '
    'C3 so equal and unordered lengths copy the input. The other path has no '
    'zero guard, multiplies x/y/z by the reciprocal and rounds each result to '
    'float. Both returns supply the result pointer in EAX. Complete span '
    'agrees with src/unrecovered/00634520.cpp; its synthetic output-parameter '
    'body is seed evidence only."], "reason": "The original computes a '
    'separate normalized three-float Vector using sqrt and a reciprocal, '
    'never mutating its receiver. Normalize is an operation-derived name, '
    'not an original debug symbol; the stack result pointer and ret4 indicate '
    'a hidden class-value result."}'
)
DELETED_SEED = (
    "// Deleted seed provenance: git:b18d3ac864e30cf0876946b9d54717a3d999c691:"
    "src/unrecovered/00634520.cpp"
)
# SHA-256 of src/vector.cpp at immutable task base cfe9967126d59604c39bb76475662989e77f4d1b,
# after replacing only the complete normalize() definition with this sentinel.
OUTSIDE_NORMALIZE_SHA256 = (
    "6ccc2e26210c6c025fc3d95b9d514a397a30cdb1ad855d1c98d49f5bf4cf29d0"
)
COMPILERS = [
    (name, shutil.which(name)) for name in ("g++", "clang++")
]


def product():
    header = (ROOT / "src/vector.h").read_text()
    source = (ROOT / "src/vector.cpp").read_text()
    assert source.count('#include "stdafx.h"') == 1
    assert source.count(ORIGINAL) == 1
    assert source.count(NORMALIZE_BINDING) == 1
    assert source.count(DELETED_SEED) == 1
    assert outside_normalize_sha256(source) == OUTSIDE_NORMALIZE_SHA256
    assert "Vector Vector::normalize()" in source
    assert "float length = static_cast<float>(sqrt(" not in source
    return header, source.replace('#include "stdafx.h"', '')


def outside_normalize_sha256(source):
    signature = "Vector Vector::normalize() {"
    assert source.count(signature) == 1
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        if source[end] == "{":
            depth += 1
        elif source[end] == "}":
            depth -= 1
            if depth == 0:
                projection = source[:start] + "/* NORMALIZE BODY */" + source[end + 1:]
                return hashlib.sha256(projection.encode()).hexdigest()
    raise AssertionError("normalize() definition has no matching closing brace")


HARNESS = r"""
#include "vector.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <type_traits>
#include <utility>

void check_at(bool condition, int line) {
    if (!condition) {
        std::fprintf(stderr, "normalization assertion line %d\n", line);
        std::exit(91);
    }
}
#define check(x) check_at((x), __LINE__)

void check_named(bool condition, const char *name) {
    if (!condition) {
        std::fprintf(stderr, "FAIL:%s\n", name);
        std::exit(91);
    }
}

void set(Vector &value, float x, float y, float z) {
    value.x_ = x; value.y_ = y; value.z_ = z;
}

int main() {
    static_assert(std::is_same<
        decltype(std::declval<Vector &>().normalize()), Vector>::value,
        "normalize returns an independent Vector value");

    Vector value;
    set(value, 3.0f, 4.0f, 0.0f);
    Vector ordinary = value.normalize();
    check(std::fabs(ordinary.x_ - 0.6f) < 0.000001f);
    check(std::fabs(ordinary.y_ - 0.8f) < 0.000001f);
    check(ordinary.z_ == 0.0f);
    check(value.x_ == 3.0f && value.y_ == 4.0f && value.z_ == 0.0f);
    ordinary.x_ = 17.0f;
    check(value.x_ == 3.0f);

    // The exact norm is just above one but rounds to float 1.0. The original
    // compares the un-narrowed sqrt result, so it normalizes instead of
    // returning the input unchanged. This distinguishes double from the old
    // float length temporary on ordinary IEEE hosts.
    set(value, 0.5f, 0.86602545f, 0.0f);
    Vector near_unit = value.normalize();
    check_named(near_unit.x_ < value.x_, "double_length_preserved");
    check(value.x_ == 0.5f && value.y_ == 0.86602545f);

    // A non-unit norm where narrowing only the reciprocal changes the final
    // rounded y component by one float ULP.
    set(value, 1.0f, 3.0f, 0.0f);
    Vector reciprocal_precision = value.normalize();
    const float expected_y = static_cast<float>(3.0 / std::sqrt(10.0));
    check_named(reciprocal_precision.y_ == expected_y,
                "double_reciprocal_preserved");
    check(value.x_ == 1.0f && value.y_ == 3.0f);

    // Exact unit length uses the copy path, preserving the receiver's -0.
    set(value, -0.0f, 1.0f, 0.0f);
    Vector unit = value.normalize();
    check(std::signbit(unit.x_));
    check(value.x_ == 0.0f && value.y_ == 1.0f);

    // The x87 C3 branch also accepts unordered comparison; it copies all
    // source fields, including the NaN, rather than normalizing them.
    const float nan = std::numeric_limits<float>::quiet_NaN();
    set(value, nan, -7.0f, 9.0f);
    Vector unordered = value.normalize();
    check_named(std::isnan(unordered.x_) && unordered.y_ == -7.0f &&
                unordered.z_ == 9.0f, "unordered_returns_input");
    unordered.y_ = 81.0f;
    check(value.y_ == -7.0f && value.z_ == 9.0f);

    // Zero length has no guard: division by zero flows into 0*infinity.
    set(value, 0.0f, 0.0f, 0.0f);
    Vector zero = value.normalize();
    check_named(std::isnan(zero.x_) && std::isnan(zero.y_) &&
                std::isnan(zero.z_), "zero_length_remains_unguarded");
    check(value.x_ == 0.0f && value.y_ == 0.0f && value.z_ == 0.0f);
}
"""


def run(tmp_path, header, source, compiler):
    if not compiler:
        pytest.skip("requested host compiler is unavailable")
    (tmp_path / "vector.h").write_text(header)
    (tmp_path / "vector_product.cpp").write_text(source)
    (tmp_path / "normalize.cpp").write_text(HARNESS)
    compiler_name = Path(compiler).name.replace("+", "p")
    executable = tmp_path / ("normalize-" + compiler_name)
    result = subprocess.run(
        [compiler, "-std=c++11", "-O1", "-fsanitize=undefined",
         "-fno-sanitize-recover=undefined", "-D__cdecl=", "-I",
         str(tmp_path), str(tmp_path / "vector_product.cpp"),
         str(tmp_path / "normalize.cpp"), "-o", str(executable)],
        capture_output=True, text=True, timeout=30)
    assert result.returncode == 0, result.stdout + result.stderr
    return subprocess.run([str(executable)], capture_output=True,
                          text=True, timeout=10)


@pytest.mark.parametrize("compiler_name,compiler", COMPILERS,
                         ids=[name for name, _ in COMPILERS])
def test_actual_vector_normalize_precision_and_edges(tmp_path, compiler_name,
                                                       compiler):
    result = run(tmp_path, *product(), compiler)
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler_name,compiler", COMPILERS,
                         ids=[name for name, _ in COMPILERS])
@pytest.mark.parametrize("damage,expected_failure", [
    ("float_length", "FAIL:double_length_preserved"),
    ("float_reciprocal", "FAIL:double_reciprocal_preserved"),
    ("zero_guard", "FAIL:zero_length_remains_unguarded"),
    ("unordered_guard", "FAIL:unordered_returns_input"),
])
def test_normalize_precision_damage_is_detected(tmp_path, damage,
                                                expected_failure,
                                                compiler_name, compiler):
    header, source = product()
    changes = {
        "float_length": (
            "double length = sqrt((static_cast<double>(x) * x\n"
            "                        + static_cast<double>(y) * y)\n"
            "                        + static_cast<double>(z) * z);",
            "float length = static_cast<float>(sqrt((static_cast<double>(x) * x\n"
            "                        + static_cast<double>(y) * y)\n"
            "                        + static_cast<double>(z) * z));"),
        "float_reciprocal": ("double reciprocal = 1.0f / length;",
                             "float reciprocal = 1.0f / length;"),
        "zero_guard": ("    double reciprocal = 1.0f / length;",
                       "    if (length == 0.0) return *this;\n"
                       "    double reciprocal = 1.0f / length;"),
        "unordered_guard": ("    if (length != length) {\n"
                            "        return *this;\n"
                            "    }\n    if (length == 1.0f) {",
                            "    if (length == 1.0f) {"),
    }
    old, new = changes[damage]
    assert source.count(old) == 1, damage
    source = source.replace(old, new)
    result = run(tmp_path, header, source, compiler)
    assert result.returncode == 91, result.stdout + result.stderr
    assert expected_failure in result.stderr, result.stdout + result.stderr
