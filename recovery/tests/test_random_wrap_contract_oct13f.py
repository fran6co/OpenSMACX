"""Defined-C++ form of Random's pinned 32-bit wrapping range arithmetic.

The class and free wrapper below are extracted from the product header/source.
The oracle models the observed x86 low-word arithmetic without signed overflow;
it does not claim full original caller closure or linked/runtime equivalence.
"""
from __future__ import annotations

from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def extract_braced(text: str, marker: str) -> str:
    start = text.index(marker)
    opening = text.index("{", start)
    depth = 0
    for end in range(opening, len(text)):
        if text[end] == "{":
            depth += 1
        elif text[end] == "}":
            depth -= 1
            if depth == 0:
                return text[start:end + 1]
    raise AssertionError("unterminated source construct: " + marker)


def product_fragments():
    header = (ROOT / "src/random.h").read_text()
    implementation = (ROOT / "src/random.cpp").read_text()
    random_class = extract_braced(header, "class Random {")
    get = extract_braced(random_class, "MEASURED uint32_t get(int min, int max)")
    wrapper = extract_braced(implementation,
                             "uint32_t __cdecl random(int min, int max)")
    seed_getter = extract_braced(implementation, "uint32_t __cdecl random_get()")
    return random_class, get, wrapper, seed_getter


HARNESS = r"""
#include <stdint.h>
#include <limits.h>
#include <cstring>

#define MEASURED
#define __cdecl

typedef char require_32_bit_int[(sizeof(int) == 4) ? 1 : -1];

__RANDOM_CLASS__
Random Rand;
__RANDOM_GETTER__
__RANDOM_WRAPPER__

static uint32_t oracle(uint32_t seed, int low, int high) {
    if (low > high) {
        const int temp = low;
        low = high;
        high = temp;
    }
    const uint32_t range = static_cast<uint32_t>(high)
                         - static_cast<uint32_t>(low);
    const uint32_t next = seed * 0x19660Du + 0x3C6EF35Fu;
    const uint32_t product = range * (next & 0xFFFFu);
    return (product >> 16) + static_cast<uint32_t>(low);
}

int main() {
    const int endpoints[][2] = {
        {INT_MIN, INT_MAX}, {INT_MAX, INT_MIN},
        {INT_MIN, 0}, {0, INT_MIN}, {0, INT_MAX}, {INT_MAX, 0},
        {INT_MIN, -1}, {-1, INT_MIN}, {-1, INT_MAX}, {INT_MAX, -1},
        {-17, 23}, {23, -17}, {INT_MIN, INT_MIN},
        {INT_MAX, INT_MAX}, {0, 0}, {100, 101}, {101, 100}
    };
    const uint32_t seeds[] = {
        0u, 1u, 0x7FFFFFFFu, 0x80000000u, 0xFFFFFFFFu, 0x12345678u
    };
    const unsigned endpoint_count = sizeof(endpoints) / sizeof(endpoints[0]);
    const unsigned seed_count = sizeof(seeds) / sizeof(seeds[0]);
    for (unsigned e = 0; e < endpoint_count; ++e) {
        for (unsigned s = 0; s < seed_count; ++s) {
            const int low = endpoints[e][0];
            const int high = endpoints[e][1];
            const uint32_t seed = seeds[s];
            const uint32_t expected = oracle(seed, low, high);

            Random local;
            local.reseed(seed);
            if (local.get(low, high) != expected) return 11;
            if (local.get_seed() != seed * 0x19660Du + 0x3C6EF35Fu)
                return 12;

            Rand.reseed(seed);
            if (random(low, high) != expected) return 13;
            if (random_get() != seed * 0x19660Du + 0x3C6EF35Fu)
                return 14;
        }
    }
    return 0;
}
"""


def translation_unit(*, get_body: str | None = None,
                     wrapper_body: str | None = None) -> str:
    random_class, actual_get, actual_wrapper, seed_getter = product_fragments()
    if get_body is not None:
        actual_get = get_body
        class_start = random_class.index("MEASURED uint32_t get(int min, int max)")
        class_end = random_class.index("  // IN-CLASS", class_start)
        random_class = random_class[:class_start] + actual_get + random_class[class_end:]
    if wrapper_body is not None:
        actual_wrapper = wrapper_body
    return (HARNESS.replace("__RANDOM_CLASS__", random_class + ";")
            .replace("__RANDOM_GETTER__", seed_getter)
            .replace("__RANDOM_WRAPPER__", actual_wrapper))


def compile_and_run(compiler: str, source: str, tmp_path: Path):
    source_path = tmp_path / "random_wrap.cpp"
    executable = tmp_path / "random_wrap"
    source_path.write_text(source)
    built = subprocess.run(
        [compiler, "-std=c++98", "-O1", "-Wall", "-Wextra",
         "-fsanitize=undefined", "-fno-sanitize-recover=undefined",
         str(source_path), "-o", str(executable)],
        text=True, capture_output=True, timeout=30,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(executable)], text=True, capture_output=True,
                          timeout=10)


COMPILERS = ["g++", "clang++"]


@pytest.mark.parametrize("compiler", COMPILERS)
def test_actual_random_header_and_free_wrapper_match_wrapping_oracle(
        compiler, tmp_path):
    if shutil.which(compiler) is None:
        pytest.skip(compiler + " unavailable")
    random_class, get, wrapper, _ = product_fragments()
    assert "static_cast<uint32_t>(max) - min" in get
    assert "if (min > max)" in get
    assert "seed_ = seed_ * 0x19660D + 0x3C6EF35F" in get
    assert "(new_seed & 0xFFFF)) >> 16" in get
    assert "Rand.get(min, max)" in wrapper
    result = compile_and_run(compiler, translation_unit(), tmp_path)
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler", COMPILERS)
def test_old_signed_endpoint_subtraction_is_rejected_by_ubsan(compiler, tmp_path):
    if shutil.which(compiler) is None:
        pytest.skip(compiler + " unavailable")
    _, get, _, _ = product_fragments()
    old = get.replace(
        "static_cast<uint32_t>(max) - min",
        "static_cast<uint32_t>(max - min)")
    assert old != get
    result = compile_and_run(compiler, translation_unit(get_body=old), tmp_path)
    assert result.returncode != 0
    assert "signed integer overflow" in result.stderr


@pytest.mark.parametrize("compiler", COMPILERS)
@pytest.mark.parametrize("damage", ["unsigned_order", "wide_product", "wrong_shift",
                                     "wrong_seed", "wrong_swap"])
def test_arithmetic_mutants_are_killed_by_boundary_vectors(
        compiler, damage, tmp_path):
    if shutil.which(compiler) is None:
        pytest.skip(compiler + " unavailable")
    _, get, _, _ = product_fragments()
    if damage == "unsigned_order":
        mutant = get.replace(
            "static_cast<uint32_t>(max) - min",
            "static_cast<uint32_t>(min) - static_cast<uint32_t>(max)")
    elif damage == "wide_product":
        mutant = get.replace(
            "((range * (new_seed & 0xFFFF)) >> 16)",
            "((static_cast<uint64_t>(range) * (new_seed & 0xFFFF)) >> 16)")
        assert "static_cast<uint64_t>" in mutant
    elif damage == "wrong_shift":
        mutant = get.replace(">> 16", ">> 15")
    elif damage == "wrong_seed":
        mutant = get.replace("0x3C6EF35F", "0x3C6EF35E")
    else:
        mutant = get.replace("if (min > max)", "if (min < max)")
    assert mutant != get
    result = compile_and_run(compiler, translation_unit(get_body=mutant), tmp_path)
    assert result.returncode != 0
