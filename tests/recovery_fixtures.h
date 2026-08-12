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

/*
 * Fixtures shared by the recovery test binaries.
 *
 * Every recovery test asks the same three questions - did the assertion hold,
 * did the function write outside the fields it owns, and did a dispatch land on
 * the slot the original uses - so the helpers that answer them are here rather
 * than copied into each suite.
 *
 * Everything is header-only and internal-linkage, because these binaries link a
 * large slice of the game and a test helper with external linkage would collide
 * with a real symbol of the same name. That has already happened once: the
 * counters below are the reason the leaf suite cannot simply link map.cpp.
 */

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

namespace recovery_fixtures {

inline int &failure_count() {
    static int failures = 0;
    return failures;
}

/*
 * Assertions EVALUATED, not assertions passed. This exists so that a refactor
 * of the suites can be proved neutral: run the binary before and after, and
 * require the two totals to be identical. A test that stops running, or a loop
 * that stops iterating, moves this number even though every remaining
 * assertion still passes.
 *
 * It is committed and printed on every run deliberately. The leaf-test split
 * proved its own neutrality with a count of 17,409,009 evaluations, and that
 * proof is now unreproducible: the counter was instrumented, read once and
 * thrown away, so no commit contains it and `git log -S` finds nothing. A
 * proof nobody can re-run is a claim.
 */
// `int64_t`, not `long long`: cl 12.00.8168 rejects the latter outright with
// `C2632: 'long' followed by 'long' is illegal` (src/vc6_compat.h:69-70 records
// the same measurement), and these two lines were the only `long long` in the
// tree - proof this header had never been through that compiler. `int64_t` is
// 64-bit everywhere, and vc6_compat.h typedefs it to `__int64` for VC6, so the
// counter keeps its width on every toolchain and needs no `#if`.
inline int64_t &expect_evaluations() {
    static int64_t evaluated = 0;
    return evaluated;
}

inline void expect(bool condition) {
    ++expect_evaluations();
    if (!condition) {
        ++failure_count();
    }
}

/*
 * Fill an object's backing storage with a recognisable pattern and snapshot it.
 *
 * The pattern matters: it is non-zero everywhere, so a recovery that reads a
 * field it should not have touched gets garbage rather than a plausible zero,
 * and it is position-dependent, so a copy that lands at the wrong offset does
 * not match by accident.
 *
 * Seed BEFORE writing the fields a fixture needs. Seeding afterwards overwrites
 * them - including any pointer field, which then dispatches into the pattern.
 */
inline void seed_storage(uint8_t *storage, uint8_t *expected, size_t size) {
    for (size_t index = 0; index < size; ++index) {
        storage[index] = static_cast<uint8_t>(0x35U + index * 17U);
        expected[index] = storage[index];
    }
}

inline void expect_storage_bytes(
        const uint8_t *storage, const uint8_t *expected, size_t size) {
    for (size_t index = 0; index < size; ++index) {
        expect(storage[index] == expected[index]);
    }
}

inline void report_storage_mismatch(
        const char *fixture, const uint8_t *storage, const uint8_t *expected,
        size_t size) {
    for (size_t index = 0; index < size; ++index) {
        if (storage[index] != expected[index]) {
            std::fprintf(stderr, "%s mismatch at 0x%zx: 0x%02x != 0x%02x\n",
                         fixture, index, storage[index], expected[index]);
            return;
        }
    }
}

/*
 * A vtable whose unused slots are null.
 *
 * Poisoning is the point. A recovery that dispatches one slot off the original's
 * lands on a null pointer and faults, which the suite reports, instead of
 * landing on a neighbouring observer and quietly passing. Install only the slots
 * the original actually uses.
 */
template <size_t Slots>
class VtableFixture {
 public:
    VtableFixture() { std::memset(slots_, 0, sizeof(slots_)); }

    // Byte displacement, matching how the disassembly names the slot
    // (`call dword ptr [edx+0x5C]`), so the test reads like the original.
    void install(size_t byte_offset, void *function) {
        slots_[byte_offset / sizeof(void *)] = function;
    }

    void *table() { return slots_; }

    // Publish this table into an object's vtable pointer at the given offset.
    void publish_at(void *object, size_t byte_offset = 0) {
        void *pointer = slots_;
        std::memcpy(static_cast<uint8_t *>(object) + byte_offset, &pointer,
                    sizeof(pointer));
    }

 private:
    void *slots_[Slots];
};

/*
 * An MSVC vbtable for a fixture object.
 *
 * A method on a virtually-derived class must take the virtual base offset from
 * the object's own vbtable at run time, because the same class embedded in a
 * larger one has a different one. Tests must therefore install at least two
 * different vbtables; a fixture built only on a most-derived object cannot
 * observe a hardcoded offset at all.
 */
class VbtableFixture {
 public:
    explicit VbtableFixture(int32_t virtual_base_offset,
                            int32_t subobject_offset = 0) {
        entries_[0] = subobject_offset;
        entries_[1] = virtual_base_offset;
    }

    void publish_at(void *object, size_t byte_offset = 0) {
        const int32_t *pointer = entries_;
        std::memcpy(static_cast<uint8_t *>(object) + byte_offset, &pointer,
                    sizeof(pointer));
    }

 private:
    int32_t entries_[2];
};

// Read/write a field by displacement, which is how the disassembly names them
// and how a test can reach a field the class keeps private.
inline uint32_t read_field32(const void *object, size_t byte_offset) {
    uint32_t value = 0;
    std::memcpy(&value,
                static_cast<const uint8_t *>(object) + byte_offset,
                sizeof(value));
    return value;
}

inline void write_field32(void *object, size_t byte_offset, uint32_t value) {
    std::memcpy(static_cast<uint8_t *>(object) + byte_offset, &value,
                sizeof(value));
}

inline void write_pointer(void *object, size_t byte_offset, void *value) {
    std::memcpy(static_cast<uint8_t *>(object) + byte_offset, &value,
                sizeof(value));
}

}  // namespace recovery_fixtures
