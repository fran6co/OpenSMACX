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
#include "sprite_oracle.h"

#include "runtime_oracle.h"
#include "sprite.h"

#include <new>

namespace {

using SpriteFixture = runtime_oracle::Fixture<Sprite>;

// The constructor never dispatches virtually, so the shared fixture
// initializer only needs a one-entry vtable and installs no vtable pointers.
const runtime_oracle::ClassSpec SpriteSpec = {
    sizeof(Sprite), sizeof(uintptr_t), 0, nullptr, 0};

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
#endif
// GCC emits the correct ECX ABI for this raw thiscall pointer but warns
// because it is not a C++ member-pointer type.
typedef uint32_t (__thiscall *OriginalNoArg)(Sprite *);
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

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

}  // namespace

bool run_sprite_oracle_suite() {
    return verify_construct();
}
