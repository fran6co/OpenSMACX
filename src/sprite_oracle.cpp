/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */
#include "stdafx.h"
#include "original_seam.h"
#include "sprite_oracle.h"

#include "runtime_oracle.h"
#include "sprite.h"
#include "temp.h"

#include <new>

namespace {

using SpriteFixture = runtime_oracle::Fixture<Sprite>;

// The constructor never dispatches virtually, so the shared fixture
// initializer only needs a one-entry vtable and installs no vtable pointers.
const runtime_oracle::ClassSpec SpriteSpec = {
    sizeof(Sprite), sizeof(uintptr_t), 0, nullptr, 0};

// GCC emits the correct ECX ABI for this raw thiscall pointer but warns
// because it is not a C++ member-pointer type.
typedef uint32_t (OriginalObject::*OriginalNoArg)();

int FreeCalls = 0;
void *FreeTargets[4] = {};

void *recording_free(void *block) {
    // Fixtures never own real allocations, so releases are recorded rather
    // than forwarded to the executable's heap.
    if (FreeCalls < 4) {
        FreeTargets[FreeCalls] = block;
    }
    ++FreeCalls;
    return nullptr;
}

bool verify_construct() {
    const int32_t starting_totals[] = {
        0, 1, -1, 0x7FFFFFFF, static_cast<int32_t>(0x80000000U),
    };
    auto original = reinterpret_cast<OriginalNoArg>(0x005E37E0U);
    const int saved_total = *SpriteMemoryUsed;
    bool passed = true;
    for (int32_t starting_total : starting_totals) {
        SpriteFixture legacy;
        SpriteFixture source;
        uintptr_t vtable[1];
        runtime_oracle::initialize_pair(
            legacy.storage, source.storage, SpriteSpec, vtable);

        *SpriteMemoryUsed = starting_total;
        const uint32_t legacy_result = original(legacy.object());
        const int legacy_total = *SpriteMemoryUsed;

        *SpriteMemoryUsed = starting_total;
        new (source.object()) Sprite();
        const uint32_t source_result = reinterpret_cast<uintptr_t>(source.object());
        const int source_total = *SpriteMemoryUsed;

        // The legacy body returns the instance pointer in EAX, so compare the
        // return residue as an offset from each fixture's own base.
        const uint32_t legacy_offset = legacy_result
            - reinterpret_cast<uintptr_t>(legacy.object());
        const uint32_t source_offset = source_result
            - reinterpret_cast<uintptr_t>(source.object());
        if (legacy_offset != source_offset
                || legacy_total != source_total
                || memcmp(legacy.storage, source.storage,
                          sizeof(legacy.storage)) != 0) {
            passed = false;
            break;
        }
    }
    *SpriteMemoryUsed = saved_total;
    return passed;
}

bool verify_close() {
    struct CloseCase {
        uint32_t field_00;
        uint32_t field_04;
        uint32_t field_10;
        uint32_t field_14;
        uint32_t field_28;
    };
    // The legacy body calls the executable's free at its own fixed address,
    // which this suite cannot redirect, and the executable CRT heap is not
    // initialised while DllMain runs. Every fixture here therefore takes only
    // branches that release nothing: a null primary allocation, plus either a
    // null pixel buffer or the borrowed flag that suppresses the pixel branch.
    // The accounting arithmetic and the release ordering are covered
    // differentially at source level in tests/recovery_leaf_tests.cpp, where a
    // recording free stands in for the executable's.
    const CloseCase cases[] = {
        {0, 0, 7, 11, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0xFFFFFFFFU, 2, 0},
        {0, 0x11110000U, 7, 11, 1},
        {0, 0x11110000U, 0xFFFFFFFFU, 0x80000000U, 1},
        {0, 0x11110000U, 0, 0, 0x80000000U},
        {0, 0, 0x80000000U, 0x80000000U, 1},
    };
    auto original = reinterpret_cast<OriginalNoArg>(0x005E3820U);
    const int saved_total = *SpriteMemoryUsed;
    func_sprite_free *const saved_free = SpriteFree;
    bool passed = true;
    for (const CloseCase &test : cases) {
        SpriteFixture legacy;
        SpriteFixture source;
        uintptr_t vtable[1];
        runtime_oracle::initialize_pair(
            legacy.storage, source.storage, SpriteSpec, vtable);
        const size_t fields[] = {0x00, 0x04, 0x10, 0x14, 0x28};
        const uint32_t values[] = {
            test.field_00, test.field_04, test.field_10,
            test.field_14, test.field_28,
        };
        for (size_t index = 0; index < ARRAYSIZE(fields); ++index) {
            memcpy(legacy.storage + runtime_oracle::CanarySize + fields[index],
                   &values[index], sizeof(uint32_t));
        }
        memcpy(source.storage, legacy.storage, sizeof(source.storage));

        // The legacy body calls the executable's own free directly, so the
        // fixture pointers are sentinels the recording free never dereferences
        // and the legacy side is compared on state and accounting alone.
        SpriteFree = &recording_free;
        FreeCalls = 0;

        *SpriteMemoryUsed = 0x1000;
        original(legacy.object());
        const int legacy_total = *SpriteMemoryUsed;

        FreeCalls = 0;
        *SpriteMemoryUsed = 0x1000;
        source.object()->close();
        const int source_total = *SpriteMemoryUsed;

        // A fixture that reached a release would have run the executable's
        // free on a sentinel; fail loudly rather than corrupt the heap.
        if (FreeCalls != 0
                || legacy_total != source_total
                || memcmp(legacy.storage, source.storage,
                          sizeof(legacy.storage)) != 0) {
            passed = false;
            break;
        }
    }
    SpriteFree = saved_free;
    *SpriteMemoryUsed = saved_total;
    return passed;
}

}  // namespace

bool run_sprite_oracle_suite() {
    return verify_construct() && verify_close();
}

namespace {

typedef void (OriginalObject::*OriginalClose)();

constexpr uintptr_t SpriteCloseAddress = 0x005E3820U;

}  // namespace

bool run_sprite_release_suite() {
    // The redirect is already installed by now, so the original body has to be
    // lifted for the differential and reinstalled afterwards.
    if (!suspend_redirect_at(SpriteCloseAddress)) {
        return false;
    }
    auto original = reinterpret_cast<OriginalClose>(SpriteCloseAddress);
    const int saved_total = *SpriteMemoryUsed;
    bool passed = true;

    struct ReleaseCase { uint32_t width; uint32_t height; bool primary; bool pixels; };
    const ReleaseCase cases[] = {
        {4, 8, true, true},
        {4, 8, false, true},
        {4, 8, true, false},
        {0, 0, true, true},
        {0xFFFFFFFFU, 2, true, true},
    };
    for (const ReleaseCase &test : cases) {
        SpriteFixture legacy;
        SpriteFixture source;
        uintptr_t vtable[1];
        runtime_oracle::initialize_pair(
            legacy.storage, source.storage, SpriteSpec, vtable);

        // Each side owns its own blocks so both can genuinely free.
        void *legacy_primary = test.primary ? _malloc(64) : nullptr;
        void *source_primary = test.primary ? _malloc(64) : nullptr;
        void *legacy_pixels = test.pixels ? _malloc(64) : nullptr;
        void *source_pixels = test.pixels ? _malloc(64) : nullptr;
        if ((test.primary && (!legacy_primary || !source_primary))
                || (test.pixels && (!legacy_pixels || !source_pixels))) {
            passed = false;
            break;
        }

        const uint32_t legacy_fields[] = {
            static_cast<uint32_t>(reinterpret_cast<uintptr_t>(legacy_primary)),
            static_cast<uint32_t>(reinterpret_cast<uintptr_t>(legacy_pixels)),
            test.width, test.height,
        };
        const uint32_t source_fields[] = {
            static_cast<uint32_t>(reinterpret_cast<uintptr_t>(source_primary)),
            static_cast<uint32_t>(reinterpret_cast<uintptr_t>(source_pixels)),
            test.width, test.height,
        };
        const size_t offsets[] = {0x00, 0x04, 0x10, 0x14};
        for (size_t index = 0; index < ARRAYSIZE(offsets); ++index) {
            memcpy(legacy.storage + runtime_oracle::CanarySize + offsets[index],
                   &legacy_fields[index], sizeof(uint32_t));
            memcpy(source.storage + runtime_oracle::CanarySize + offsets[index],
                   &source_fields[index], sizeof(uint32_t));
        }
        const uint32_t zero = 0;
        memcpy(legacy.storage + runtime_oracle::CanarySize + 0x28, &zero, sizeof(zero));
        memcpy(source.storage + runtime_oracle::CanarySize + 0x28, &zero, sizeof(zero));

        *SpriteMemoryUsed = 0x1000;
        original(legacy.object());
        const int legacy_total = *SpriteMemoryUsed;

        *SpriteMemoryUsed = 0x1000;
        source.object()->close();
        const int source_total = *SpriteMemoryUsed;

        // Both pointer fields are cleared by a successful release, so the
        // whole object is directly comparable afterwards.
        if (legacy_total != source_total
                || memcmp(legacy.storage, source.storage,
                          sizeof(legacy.storage)) != 0) {
            passed = false;
            break;
        }
    }
    *SpriteMemoryUsed = saved_total;
    if (!resume_redirect_at(SpriteCloseAddress)) {
        return false;
    }
    return passed;
}
