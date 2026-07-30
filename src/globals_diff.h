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
#pragma once

#include <cstddef>
#include <cstring>

// Comparing two snapshots of the game's globals, for the generated oracle
// suite. Hand-written rather than emitted by
// tools/generate_signature_oracles.py, because the bisect below is the only
// subtle line in the whole comparison and a behavioural C++ test can check it
// where a text assertion over generated output cannot.
//
// The generated suite used to walk this span a byte at a time, and it walks it
// twice per case - once to compare the two sides, once to ask whether the
// ORIGINAL did anything observable at all. The span is
// 0x009C21F8 - 0x00682000 = 3,408,376 bytes, and the suite runs 668 cases, so
// the scalar loop reads 4.24 GiB inside a 32-bit process under Wine. `memcmp`
// is one call and reads the same bytes a machine word at a time.
//
// The diagnostic is why this is not a bare memcmp at the call site: when the
// snapshots differ, the caller reports WHICH address moved, and that address is
// the whole value of a FAIL. So compare fast, and pay for the position only on
// the mismatch that is about to be reported anyway.
namespace globals_diff {

// True when the two snapshots are identical. On a mismatch, `*first` receives
// the offset of the lowest differing byte and the result is false.
//
// `first` may be null when the caller only needs the yes/no.
inline bool equal(const unsigned char *left, const unsigned char *right,
                  size_t size, size_t *first) {
    if (std::memcmp(left, right, size) == 0) {
        return true;
    }
    if (first == nullptr) {
        return false;
    }
    // Halve the interval, keeping the invariant that [low, high) contains at
    // least one differing byte. ~22 iterations for a 3.4 MB span, and every
    // comparison is still a memcmp, so locating the byte costs about one more
    // pass over the data rather than a scalar walk of it.
    size_t low = 0;
    size_t high = size;
    while (high - low > 1) {
        const size_t middle = low + (high - low) / 2;
        if (std::memcmp(left + low, right + low, middle - low) != 0) {
            high = middle;
        } else {
            low = middle;
        }
    }
    *first = low;
    return false;
}

}  // namespace globals_diff
