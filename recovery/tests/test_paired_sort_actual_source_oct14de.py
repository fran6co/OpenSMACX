"""Actual-source controls for the paired descending int-array sort.

The host harness observes the real source body and the real XOR swap helper.
It is a bounded behavioral witness, not an original-TU, caller-domain,
strict-reference, linked-selection, or application-runtime proof.
"""
from dataclasses import fields
from pathlib import Path
import shutil
import subprocess

import pytest

from match_recovery.source import read_text


ROOT = Path(__file__).resolve().parents[2]
GENERAL = ROOT / "src" / "general.cpp"
GENERAL_HEADER = ROOT / "src" / "general.h"
STRINGSTRUCT = ROOT / "src" / "stringstruct.cpp"
BASELINE = "c7a89397488f209346cc3bbc5329d7489b5a0e81"
SORT_SIGNATURE = "void __cdecl sub_4d5f70(int *parallel, int *values, int count)"
SWAP_SIGNATURE = "MEASURED inline void __cdecl swap(int *var1, int *var2)"


def extract_function(source, signature):
    start = source.find(signature)
    assert start >= 0, "actual source definition missing: " + signature
    opening = source.find("{", start)
    assert opening >= 0, "actual source body missing: " + signature
    depth = 0
    state = "code"
    index = opening
    while index < len(source):
        char = source[index]
        following = source[index + 1] if index + 1 < len(source) else ""
        if state == "code":
            if char == "/" and following == "/":
                state = "line"
                index += 2
                continue
            if char == "/" and following == "*":
                state = "block"
                index += 2
                continue
            if char == '"':
                state = "string"
            elif char == "'":
                state = "char"
            elif char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return source[start:index + 1]
        elif state == "line":
            if char == "\n":
                state = "code"
        elif state == "block":
            if char == "*" and following == "/":
                state = "code"
                index += 2
                continue
        elif char == "\\":
            index += 2
            continue
        elif ((state == "string" and char == '"')
              or (state == "char" and char == "'")):
            state = "code"
        index += 1
    raise AssertionError("unterminated actual source body: " + signature)


def actual_definitions():
    source = GENERAL.read_text(encoding="utf-8")
    helper_header = GENERAL_HEADER.read_text(encoding="utf-8")
    body = extract_function(source, SORT_SIGNATURE)
    helper = extract_function(helper_header, SWAP_SIGNATURE)
    # The fixture adds trace instrumentation at the real helper entry; the
    # helper's three original XOR writes remain verbatim and ordered.
    helper = helper.replace("MEASURED inline ", "inline ", 1)
    assert "*var1 ^= *var2;\n    *var2 ^= *var1;\n    *var1 ^= *var2;" in helper
    helper = helper.replace(
        "{\n    *var1 ^= *var2;",
        "{\n    record_swap(var1, var2);\n    *var1 ^= *var2;", 1)
    return body, helper


FIXTURE = r'''#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define __cdecl

struct SwapEvent { int *first; int *second; };
static SwapEvent actual_events[4096];
static SwapEvent reference_events[4096];
static int actual_event_count;
static int reference_event_count;

static void fail(const char *message) {
    fprintf(stderr, "SEMANTIC97: %s\n", message);
    exit(97);
}
static void require(int condition, const char *message) {
    if (!condition) fail(message);
}
static void record_swap(int *first, int *second) {
    if (actual_event_count >= 4096) fail("actual swap trace capacity exceeded");
    actual_events[actual_event_count].first = first;
    actual_events[actual_event_count].second = second;
    ++actual_event_count;
}
static void reference_xor_swap(int *first, int *second) {
    if (reference_event_count >= 4096) fail("reference trace capacity exceeded");
    reference_events[reference_event_count].first = first;
    reference_events[reference_event_count].second = second;
    ++reference_event_count;
    *first ^= *second;
    *second ^= *first;
    *first ^= *second;
}

// ACTUAL_SWAP_HELPER
// ACTUAL_SORT_BODY

static void reference_sort(int *parallel, int *values, int count) {
    const int limit = count - 1;
    if (limit < 1) return;
    for (int pass = 0; pass < 64; ++pass) {
        int did_swap = 0;
        for (int index = 0; index < limit; ++index) {
            if (values[index] < values[index + 1]) {
                did_swap = 1;
                reference_xor_swap(values + index, values + index + 1);
                reference_xor_swap(parallel + index, parallel + index + 1);
            }
        }
        if (!did_swap) return;
    }
    fail("bounded reference did not converge for fixture input");
}

static void run_case(const char *name, const int *initial, int value_offset,
                     int parallel_offset, int count, const int *known_values,
                     const int *known_parallel) {
    int actual[32];
    int expected[32];
    memcpy(actual, initial, sizeof(actual));
    memcpy(expected, initial, sizeof(expected));

    reference_event_count = 0;
    reference_sort(expected + parallel_offset, expected + value_offset, count);
    const int expected_event_count = reference_event_count;
    SwapEvent expected_events[4096];
    memcpy(expected_events, reference_events,
           sizeof(SwapEvent) * expected_event_count);

    actual_event_count = 0;
    sub_4d5f70(actual + parallel_offset, actual + value_offset, count);
    for (int index = 0; index < 32; ++index) {
        if (actual[index] != expected[index])
            fail("actual array result differs from bounded reference");
    }
    if (actual_event_count != expected_event_count)
        fail("swap order or pointer advancement differs from reference");
    for (int index = 0; index < actual_event_count; ++index) {
        if (actual_events[index].first - actual
                != expected_events[index].first - expected
            || actual_events[index].second - actual
                != expected_events[index].second - expected)
            fail("swap order or pointer advancement differs from reference");
    }
    if (known_values != 0) {
        for (int index = 0; index < count; ++index)
            if (actual[value_offset + index] != known_values[index])
                fail("known descending result changed");
    }
    if (known_parallel != 0) {
        for (int index = 0; index < count; ++index)
            if (actual[parallel_offset + index] != known_parallel[index])
                fail("known parallel-tag result changed");
    }
    (void)name;
}

static void guards_and_pairs() {
    int storage[32] = {0};
    for (int count_index = 0; count_index < 3; ++count_index) {
        const int counts[3] = {0, 1, -7};
        int before[32];
        memcpy(before, storage, sizeof(storage));
        actual_event_count = 0;
        sub_4d5f70(storage + 8, storage + 4, counts[count_index]);
        require(actual_event_count == 0, "zero/one/negative count was not guarded");
        require(memcmp(storage, before, sizeof(storage)) == 0,
                "guarded count mutated storage");
    }
    {
        const int ascending[2] = {1, 8};
        const int descending[2] = {8, 1};
        const int equal[2] = {5, 5};
        const int tags[2] = {90, 91};
        const int swapped_tags[2] = {91, 90};
        int initial[32] = {0};
        initial[4] = ascending[0]; initial[5] = ascending[1];
        initial[16] = tags[0]; initial[17] = tags[1];
        run_case("ascending pair", initial, 4, 16, 2, descending, swapped_tags);
        initial[4] = descending[0]; initial[5] = descending[1];
        run_case("descending pair", initial, 4, 16, 2, descending, tags);
        initial[4] = equal[0]; initial[5] = equal[1];
        run_case("equal pair", initial, 4, 16, 2, equal, tags);
    }
}

static void distinct_multi_pass() {
    int initial[32] = {0};
    const int values[5] = {3, 1, 2, 2, 4};
    const int tags[5] = {30, 10, 20, 21, 40};
    const int descending[5] = {4, 3, 2, 2, 1};
    const int sorted_tags[5] = {40, 30, 20, 21, 10};
    memcpy(initial + 4, values, sizeof(values));
    memcpy(initial + 16, tags, sizeof(tags));
    run_case("distinct multipass", initial, 4, 16, 5, descending, sorted_tags);
}

static void bounded_overlaps() {
    // Offset +1: two overlapping exchanges per inversion, then a terminating
    // pass. This observes sequential primary-then-parallel XOR effects.
    {
        int initial[32] = {0};
        const int input[3] = {1, 2, 3};
        const int result[3] = {3, 1, 2};
        memcpy(initial + 8, input, sizeof(input));
        run_case("overlap plus one", initial, 8, 9, 2, result, 0);
    }
    // Offset -1: the parallel swap overlaps the primary pair from the other
    // side and the next primary pass terminates without another exchange.
    {
        int initial[32] = {0};
        const int input[3] = {3, 1, 2};
        const int result[2] = {3, 1};
        memcpy(initial + 8, input, sizeof(input));
        run_case("overlap minus one", initial, 9, 8, 2, result, 0);
    }
    // A same-array descending input is a bounded no-op. The separately tested
    // helper self-alias below pins the real XOR helper's no-guard behavior.
    {
        int initial[32] = {0};
        const int input[3] = {3, 2, 1};
        memcpy(initial + 8, input, sizeof(input));
        run_case("same-array no-op", initial, 8, 8, 3, input, input);
    }
}

static void helper_self_alias() {
    int value = 0x12345678;
    actual_event_count = 0;
    swap(&value, &value);
    require(value == 0, "actual XOR helper alias behavior changed");
    require(actual_event_count == 1 && actual_events[0].first == &value
            && actual_events[0].second == &value,
            "actual XOR helper alias trace changed");
}

int main() {
    guards_and_pairs();
    distinct_multi_pass();
    bounded_overlaps();
    helper_self_alias();
    puts("paired sort actual-source controls: PASS");
    return 0;
}
'''


def project_source(body, helper, mutation=None):
    if mutation is not None:
        target, before, after = mutation
        source = body if target == "sort" else helper
        assert before in source, "mutation anchor missing: " + before
        source = source.replace(before, after, 1)
        if target == "sort":
            body = source
        else:
            helper = source
    return (FIXTURE.replace("// ACTUAL_SWAP_HELPER", helper)
            .replace("// ACTUAL_SORT_BODY", body))


def compile_and_run(tmp_path, compiler, mutation=None):
    body, helper = actual_definitions()
    projected = project_source(body, helper, mutation)
    source_path = tmp_path / "paired-sort.cpp"
    executable = tmp_path / "paired-sort"
    source_path.write_text(projected, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
         str(source_path), "-o", str(executable)],
        capture_output=True, text=True, timeout=30,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(executable)], capture_output=True, text=True,
                          timeout=5)


def test_sort_identity_moves_to_general_without_losing_source_history():
    previous_struct = subprocess.run(
        ["git", "show", f"{BASELINE}:src/stringstruct.cpp"], cwd=ROOT,
        check=True, text=True, capture_output=True, timeout=30,
    ).stdout
    previous_general = subprocess.run(
        ["git", "show", f"{BASELINE}:src/general.cpp"], cwd=ROOT,
        check=True, text=True, capture_output=True, timeout=30,
    ).stdout
    old_rows = [row for row in read_text(previous_struct, STRINGSTRUCT)
                if row.address == 0x004D5F70]
    assert len(old_rows) == 1
    assert not [row for row in read_text(previous_general, GENERAL)
                if row.address == 0x004D5F70]
    current_string_rows = [row for row in read_text(
        STRINGSTRUCT.read_text(encoding="utf-8"), STRINGSTRUCT)
        if row.address == 0x004D5F70]
    current_general_rows = [row for row in read_text(
        GENERAL.read_text(encoding="utf-8"), GENERAL)
        if row.address == 0x004D5F70]
    assert current_string_rows == []
    assert len(current_general_rows) == 1
    old = old_rows[0]
    current = current_general_rows[0]
    # The parser's enclosing region and line are source-location-derived; all
    # load-bearing identity, claim, symbol and historical fields stay exact.
    for field in fields(old):
        if field.name in ("path", "line", "region"):
            continue
        assert getattr(old, field.name) == getattr(current, field.name), field.name
    assert old.symbol == current.symbol == "_sub_4d5f70"
    assert Path(old.path).as_posix().endswith("/src/stringstruct.cpp")
    assert Path(current.path).as_posix().endswith("/src/general.cpp")
    assert old.image_spans == current.image_spans
    assert old.address == current.address == 0x004D5F70


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_sort_and_actual_xor_helper_behavior(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    result = compile_and_run(tmp_path, compiler)
    assert result.returncode == 0, result.stdout + result.stderr
    assert "paired sort actual-source controls: PASS" in result.stdout


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize("mutation,diagnostic", [
    (("sort", "if (*value < *next)", "if (*value > *next)"),
     "actual array result differs from bounded reference"),
    (("sort", "swap(value, next);\n                swap(cursor, cursor + 1);",
      "swap(cursor, cursor + 1);\n                swap(value, next);"),
     "swap order or pointer advancement differs from reference"),
])
def test_compile_success_mutants_fail_with_exact_semantic_diagnostic(
        tmp_path, compiler_name, mutation, diagnostic):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    result = compile_and_run(tmp_path, compiler, mutation)
    assert result.returncode == 97, (result.returncode, result.stdout, result.stderr)
    assert "SEMANTIC97: " + diagnostic in result.stderr
