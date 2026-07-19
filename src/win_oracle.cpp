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
#include "win_oracle.h"

#include "runtime_oracle.h"
#include "win.h"

namespace {

using WinFixture = runtime_oracle::Fixture<Win>;

// is_visible never dispatches virtually, so the shared fixture initializer
// only needs a one-entry vtable and installs no vtable pointers.
const runtime_oracle::ClassSpec WinSpec = {sizeof(Win), sizeof(uintptr_t), 0, nullptr, 0};

constexpr size_t VisibleFlagsOffset = 0x9C;
constexpr size_t ParentOffset = 0xC4;
constexpr size_t MaximumDepth = 3;

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
#endif
// GCC emits the correct ECX ABI for this raw thiscall pointer but warns
// because it is not a C++ member-pointer type.
typedef uint32_t (__thiscall *OriginalNoArg)(Win *);
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

template <typename T>
void write_object(WinFixture &fixture, size_t offset, const T &value) {
    memcpy(fixture.storage + runtime_oracle::CanarySize + offset,
           &value, sizeof(value));
}

// Build one parent chain. Parent pointers necessarily differ between the
// legacy and source chains, so each side is linked into its own nodes.
void build_chain(WinFixture *nodes, uint8_t (*snapshots)[sizeof(Win) + runtime_oracle::CanarySize * 2],
                 const uint32_t *flags, size_t depth, uintptr_t *vtable) {
    for (size_t index = 0; index < depth; ++index) {
        WinFixture unused;
        runtime_oracle::initialize_pair(
            nodes[index].storage, unused.storage, WinSpec, vtable);
        write_object(nodes[index], VisibleFlagsOffset, flags[index]);
        Win *parent = (index + 1 < depth) ? nodes[index + 1].object() : nullptr;
        write_object(nodes[index], ParentOffset, parent);
    }
    for (size_t index = 0; index < depth; ++index) {
        memcpy(snapshots[index], nodes[index].storage,
               sizeof(nodes[index].storage));
    }
}

bool verify_is_visible() {
    struct ChainCase {
        size_t depth;
        uint32_t flags[MaximumDepth];
    };
    const ChainCase cases[] = {
        {1, {0x00000000U, 0, 0}},
        {1, {0x00000001U, 0, 0}},
        {1, {0xFFFFFFFEU, 0, 0}},
        {1, {0xFFFFFFFFU, 0, 0}},
        {1, {0xA55AA55BU, 0, 0}},
        {2, {0x00000001U, 0x00000001U, 0}},
        {2, {0x00000001U, 0x00000000U, 0}},
        {2, {0x00000000U, 0x00000001U, 0}},
        {3, {0x00000001U, 0x00000001U, 0x00000001U}},
        {3, {0x00000001U, 0x00000001U, 0x00000000U}},
        {3, {0x00000001U, 0x00000000U, 0x00000001U}},
        {3, {0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFEU}},
    };
    auto original = reinterpret_cast<OriginalNoArg>(0x005F7E90U);
    for (const ChainCase &test : cases) {
        static WinFixture legacy_nodes[MaximumDepth];
        static WinFixture source_nodes[MaximumDepth];
        static uint8_t legacy_snapshots[MaximumDepth][sizeof(WinFixture)];
        static uint8_t source_snapshots[MaximumDepth][sizeof(WinFixture)];
        uintptr_t vtable[1];

        build_chain(legacy_nodes, legacy_snapshots, test.flags, test.depth, vtable);
        const uint32_t legacy_result = original(legacy_nodes[0].object());

        build_chain(source_nodes, source_snapshots, test.flags, test.depth, vtable);
        const uint32_t source_result = static_cast<uint32_t>(
            source_nodes[0].object()->is_visible());

        if (legacy_result != source_result) {
            return false;
        }
        // Visibility is a pure query: every node on both chains, including
        // its canaries, must be byte-identical to its pre-call snapshot.
        for (size_t index = 0; index < test.depth; ++index) {
            if (memcmp(legacy_nodes[index].storage, legacy_snapshots[index],
                       sizeof(legacy_nodes[index].storage)) != 0
                    || memcmp(source_nodes[index].storage, source_snapshots[index],
                              sizeof(source_nodes[index].storage)) != 0) {
                return false;
            }
        }
    }
    return true;
}

}  // namespace

bool run_win_oracle_suite() {
    return verify_is_visible();
}
