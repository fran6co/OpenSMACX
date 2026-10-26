"""Behavioral host controls for the pinned encrypt_read byte-count quirk."""
from pathlib import Path
import os
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/map.cpp"


def actual_definition() -> str:
    text = SOURCE.read_text(encoding="utf-8")
    signature = "int __cdecl encrypt_read("
    assert text.count(signature) == 1, "expected one actual encrypt_read definition"
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for index in range(opening, len(text)):
        if text[index] == "{":
            depth += 1
        elif text[index] == "}":
            depth -= 1
            if depth == 0:
                return text[start:index + 1]
    raise AssertionError("unterminated encrypt_read definition")


HARNESS = r"""
#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstring>

#ifndef __cdecl
#define __cdecl
#endif

struct FakeFile { int tag; };
static unsigned char allocation[64];
static unsigned char input_bytes[64];
static unsigned char output_bytes[64];
static unsigned int input_length;
static unsigned int last_alloc_size;
static unsigned int last_read_size;
static unsigned int last_read_count;
static FakeFile *last_read_file;
static int alloc_calls, read_calls, free_calls;
static bool allocation_fails;
static int read_result;
static FakeFile file_under_test = { 7 };

static void reset(unsigned int total_input, bool fail_alloc, int result) {
    std::memset(allocation, 0, sizeof(allocation));
    std::memset(input_bytes, 0, sizeof(input_bytes));
    std::memset(output_bytes, 0xCC, sizeof(output_bytes));
    input_length = total_input;
    last_alloc_size = last_read_size = last_read_count = 0;
    last_read_file = 0;
    alloc_calls = read_calls = free_calls = 0;
    allocation_fails = fail_alloc;
    read_result = result;
}

static void *host_mem_get(unsigned int size) {
    ++alloc_calls;
    last_alloc_size = size;
    return allocation_fails ? 0 : allocation;
}

static int host_fread(void *destination, unsigned int size,
                      unsigned int count, FakeFile *file) {
    ++read_calls;
    last_read_size = size;
    last_read_count = count;
    last_read_file = file;
    if (size == 0 || count == 0) return 0;
    if (read_result != 0) {
        const unsigned int bytes = size * count;
        assert(bytes == input_length);
        assert(bytes <= sizeof(input_bytes));
        std::memcpy(destination, input_bytes, bytes);
    }
    return read_result;
}

static void host_free(void *pointer) {
    ++free_calls;
    assert(pointer == allocation);
}

// ACTUAL_SOURCE

static bool equals(const unsigned char *actual,
                   const unsigned char *expected, unsigned int count) {
    return std::memcmp(actual, expected, count) == 0;
}

static int test_even_pairs() {
    reset(4, false, 1);
    input_bytes[0] = 0x6F; input_bytes[1] = 0xA1;
    input_bytes[2] = 0x3E; input_bytes[3] = 0xB2;
    const unsigned char expected[] = {0x20, 0x10, 0x30, 0x40, 0xCC};
    const int result = encrypt_read(output_bytes, 2, 2, &file_under_test);
    if (result != 1 || !equals(output_bytes, expected, sizeof(expected))) return 86;
    assert(alloc_calls == 1 && last_alloc_size == 4);
    assert(read_calls == 1 && last_read_size == 4 && last_read_count == 1);
    assert(last_read_file == &file_under_test && free_calls == 1);
    return 0;
}

static int test_odd_padded_tail() {
    reset(4, false, 1);
    input_bytes[0] = 0x6F; input_bytes[1] = 0xA1;
    input_bytes[2] = 0x3E; input_bytes[3] = 0xB2;
    // The original writes the decoded padded fourth byte for total > 1.
    const unsigned char expected[] = {0x20, 0x10, 0x30, 0x40, 0xCC};
    const int result = encrypt_read(output_bytes, 3, 1, &file_under_test);
    if (result != 1) return 85;
    if (!equals(output_bytes, expected, sizeof(expected))) return 86;
    assert(alloc_calls == 1 && last_alloc_size == 4);
    assert(read_calls == 1 && last_read_size == 4 && last_read_count == 1);
    assert(last_read_file == &file_under_test && free_calls == 1);
    return 0;
}

static int test_single_byte_suppresses_second_store() {
    reset(2, false, 1);
    input_bytes[0] = 0x7F; input_bytes[1] = 0x81;
    const unsigned char expected[] = {0x81, 0xCC};
    const int result = encrypt_read(output_bytes, 1, 1, &file_under_test);
    if (result != 1 || !equals(output_bytes, expected, sizeof(expected))) return 86;
    assert(alloc_calls == 1 && last_alloc_size == 2);
    assert(read_calls == 1 && last_read_size == 2 && last_read_count == 1);
    assert(last_read_file == &file_under_test && free_calls == 1);
    return 0;
}

static int test_zero_and_unsigned_product_overflow() {
    reset(0, false, 1);
    int result = encrypt_read(output_bytes, 0, 19, &file_under_test);
    if (result != 0) return 86;
    assert(alloc_calls == 1 && last_alloc_size == 0);
    assert(read_calls == 1 && last_read_size == 0 && last_read_count == 1);
    assert(last_read_file == &file_under_test && free_calls == 1);
    assert(output_bytes[0] == 0xCC && output_bytes[1] == 0xCC);

    reset(0, false, 1);
    result = encrypt_read(output_bytes, 0x80000000u, 2, &file_under_test);
    if (result != 0) return 86;
    assert(alloc_calls == 1 && last_alloc_size == 0);
    assert(read_calls == 1 && last_read_size == 0 && last_read_count == 1);
    assert(last_read_file == &file_under_test && free_calls == 1);
    assert(output_bytes[0] == 0xCC && output_bytes[1] == 0xCC);
    return 0;
}

static int test_allocation_failure() {
    reset(0, true, 1);
    const int result = encrypt_read(output_bytes, 3, 1, &file_under_test);
    if (result != 0) return 86;
    assert(alloc_calls == 1 && last_alloc_size == 4);
    assert(read_calls == 0 && free_calls == 0);
    return 0;
}

static int test_read_failure_frees_buffer() {
    reset(0, false, 0);
    const int result = encrypt_read(output_bytes, 3, 1, &file_under_test);
    if (result != 0) return 86;
    assert(alloc_calls == 1 && last_alloc_size == 4);
    assert(read_calls == 1 && last_read_size == 4 && last_read_count == 1);
    assert(last_read_file == &file_under_test && free_calls == 1);
    assert(output_bytes[0] == 0xCC && output_bytes[1] == 0xCC);
    return 0;
}

int main(int argc, char **argv) {
    assert(argc == 2);
    if (std::strcmp(argv[1], "even") == 0) return test_even_pairs();
    if (std::strcmp(argv[1], "odd") == 0) return test_odd_padded_tail();
    if (std::strcmp(argv[1], "single") == 0) return test_single_byte_suppresses_second_store();
    if (std::strcmp(argv[1], "zero-overflow") == 0) return test_zero_and_unsigned_product_overflow();
    if (std::strcmp(argv[1], "allocation-failure") == 0) return test_allocation_failure();
    if (std::strcmp(argv[1], "read-failure") == 0) return test_read_failure_frees_buffer();
    return 92;
}
"""


def projected(body: str) -> str:
    # Substitute only external dependencies/types; execute the complete actual method.
    body = re.sub(r"\bFILE\s*\*", "FakeFile *", body)
    body = re.sub(r"\bmem_get\s*\(", "host_mem_get(", body)
    body = re.sub(r"\bfread\s*\(", "host_fread(", body)
    body = re.sub(r"\bfree\s*\(", "host_free(", body)
    assert body.count("int __cdecl encrypt_read(") == 1
    assert HARNESS.count("// ACTUAL_SOURCE") == 1
    return HARNESS.replace("// ACTUAL_SOURCE", body)


def decrementing_remaining_mutant(body: str) -> str:
    assert body.count("if (total > 1)") == 1, "expected the admitted total-size guard"
    mutant = body.replace("if (total > 1)", "if (remaining > 1)", 1)
    dst_line = re.search(r"(?m)^[ \t]*unsigned char \*dst = [^;]+;[ \t]*$", mutant)
    assert dst_line is not None, "expected destination initialization"
    mutant = (mutant[:dst_line.end()] + "\n    unsigned int remaining = total;"
              + mutant[dst_line.end():])
    store_advance = "        dst = next;"
    assert mutant.count(store_advance) == 1, "expected one decoded-pair advance"
    mutant = mutant.replace(store_advance,
                            store_advance + "\n        remaining -= 2;", 1)
    return mutant


def compile_fixture(tmp_path: Path, body: str) -> Path:
    compiler = shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        pytest.skip("g++ or clang++ is required for the actual-source host fixture")
    source = tmp_path / "encrypt_read_actual_source.cpp"
    binary = tmp_path / "encrypt_read_actual_source"
    source.write_text(projected(body), encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++17", "-O2", "-Wall", "-Wextra", "-Werror",
         "-fsanitize=address,undefined", "-fno-omit-frame-pointer",
         str(source), "-o", str(binary)],
        capture_output=True,
        text=True,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return binary


def run_case(binary: Path, name: str) -> subprocess.CompletedProcess:
    environment = dict(os.environ)
    environment["ASAN_OPTIONS"] = "detect_leaks=0"
    environment["UBSAN_OPTIONS"] = "halt_on_error=1"
    return subprocess.run([str(binary), name], capture_output=True, text=True,
                          env=environment)


def test_published_encrypt_read_actual_source_contracts(tmp_path):
    binary = compile_fixture(tmp_path, actual_definition())
    for case in ("even", "odd", "single", "zero-overflow", "allocation-failure", "read-failure"):
        result = run_case(binary, case)
        assert result.returncode == 0, f"{case}:\n{result.stdout}{result.stderr}"


def test_old_decrementing_remaining_mutant_fails_odd_tail_assertion(tmp_path):
    body = actual_definition()
    mutant = decrementing_remaining_mutant(body)
    binary = compile_fixture(tmp_path, mutant)
    result = run_case(binary, "odd")
    assert result.returncode == 86, "expected the odd-tail output witness to reject the old mutant"
    assert result.stdout == "" and result.stderr == "", "mutant failed outside the intended output assertion"
