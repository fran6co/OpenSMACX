/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenSMACX is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenSMACX. If not, see <http://www.gnu.org/licenses/>.
 */
// The bisect in src/globals_diff.h reports which byte of a 3.4 MB snapshot
// moved, and that address is the entire value of a FAIL verdict. An off-by-one
// in it would name the wrong global while still reporting a difference - a
// plausible, wrong answer, which is the worst kind here. So it is tested
// behaviourally rather than by asserting on generated text.
// Relative, and with no -I src on this target, deliberately: putting src/ on the
// include path makes the project's own src/strings.h shadow the <strings.h> that
// glibc's string.h includes, and <cstring> then fails to compile. The sibling
// tests/lifted_oracle_fold_tests.cpp reaches its header the same way.
#include "../src/globals_diff.h"

#include <cstdio>
#include <vector>

namespace {

int failures = 0;

void expect(bool condition, const char *what) {
    if (!condition) {
        std::printf("FAIL: %s\n", what);
        ++failures;
    }
}

// A sentinel the helper must overwrite, so a test cannot pass by the helper
// leaving `first` untouched at a value the test happened to expect.
constexpr size_t Untouched = static_cast<size_t>(-1);

void test_identical_buffers_are_equal() {
    std::vector<unsigned char> left(4096, 0x5A);
    std::vector<unsigned char> right(left);
    size_t first = Untouched;
    expect(globals_diff::equal(&left[0], &right[0], left.size(), &first),
           "identical buffers compare equal");
    expect(first == Untouched, "an equal comparison does not write `first`");
}

void test_mismatch_at_each_boundary() {
    struct Case { size_t size; size_t at; const char *what; };
    const Case cases[] = {
        {4096, 0, "mismatch at the first byte"},
        {4096, 4095, "mismatch at the last byte"},
        {4096, 2048, "mismatch at the midpoint"},
        {4096, 2049, "mismatch one past the midpoint"},
        {4096, 1, "mismatch at the second byte"},
        {4096, 4094, "mismatch one before the last"},
        {4095, 2731, "mismatch in an odd-sized buffer"},
        {1, 0, "mismatch in a one-byte buffer"},
    };
    // Index loop, not range-for: cl 12.00.8168 has neither.
    for (size_t index = 0; index < sizeof(cases) / sizeof(cases[0]); ++index) {
        const Case &item = cases[index];
        std::vector<unsigned char> left(item.size, 0x11);
        std::vector<unsigned char> right(left);
        right[item.at] = 0x22;
        size_t first = Untouched;
        const bool equal = globals_diff::equal(
            &left[0], &right[0], item.size, &first);
        expect(!equal, item.what);
        if (first != item.at) {
            std::printf("FAIL: %s -- reported %zu, expected %zu\n",
                        item.what, first, item.at);
            ++failures;
        }
    }
}

void test_the_FIRST_of_several_mismatches_is_reported() {
    // The property the bisect exists for. Reporting any differing byte would
    // pass a naive test; reporting the lowest one is what makes the address in
    // a FAIL message mean something.
    std::vector<unsigned char> left(8192, 0);
    std::vector<unsigned char> right(left);
    right[6000] = 1;
    right[100] = 1;
    right[7000] = 1;
    size_t first = Untouched;
    expect(!globals_diff::equal(&left[0], &right[0], left.size(), &first),
           "several mismatches compare unequal");
    expect(first == 100, "the LOWEST differing byte is reported");
}

void test_every_single_byte_position_is_located_exactly() {
    // Exhaustive over a small span: an off-by-one that only shows at certain
    // alignments cannot hide from this.
    constexpr size_t Size = 512;
    for (size_t at = 0; at < Size; ++at) {
        std::vector<unsigned char> left(Size, 0xAA);
        std::vector<unsigned char> right(left);
        right[at] = 0xAB;
        size_t first = Untouched;
        globals_diff::equal(&left[0], &right[0], Size, &first);
        if (first != at) {
            std::printf("FAIL: exhaustive scan -- byte %zu reported as %zu\n",
                        at, first);
            ++failures;
            return;
        }
    }
}

void test_an_empty_span_is_equal() {
    unsigned char left[1] = {0};
    unsigned char right[1] = {1};
    size_t first = Untouched;
    expect(globals_diff::equal(left, right, 0, &first),
           "a zero-length span compares equal whatever follows it");
}

void test_a_null_first_is_accepted() {
    std::vector<unsigned char> left(64, 3);
    std::vector<unsigned char> right(left);
    right[9] = 4;
    expect(!globals_diff::equal(&left[0], &right[0], left.size(), nullptr),
           "a caller that does not want the position still gets the answer");
}

}  // namespace

int main() {
    test_identical_buffers_are_equal();
    test_mismatch_at_each_boundary();
    test_the_FIRST_of_several_mismatches_is_reported();
    test_every_single_byte_position_is_located_exactly();
    test_an_empty_span_is_equal();
    test_a_null_first_is_accepted();
    if (failures != 0) {
        std::printf("globals-diff-tests: %d failure(s)\n", failures);
        return 1;
    }
    std::printf("globals-diff-tests: all passed\n");
    return 0;
}
