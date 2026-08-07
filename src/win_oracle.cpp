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
#include "win_oracle.h"

#include "runtime_oracle.h"
#include "win.h"

// What this oracle PROVES: each function below is executed here in its
// ORIGINAL form, at its canonical address inside the hybrid process, and
// compared against the recovered implementation. These markers are the
// input to `unproven_recovered` in docs/recovery/summary.json; see
// tools/export_proven_functions.py for what does and does not count.
//
// PROVEN-AGAINST-ORIGINAL: 0x005ED240  ?client_to_screen@Win@@QAEXPAH0@Z
// PROVEN-AGAINST-ORIGINAL: 0x005F7E90  ?is_visible@Win@@QAEHXZ


namespace {

typedef runtime_oracle::Fixture<Win> WinFixture;

// is_visible never dispatches virtually, so the shared fixture initializer
// only needs a one-entry vtable and installs no vtable pointers.
const runtime_oracle::ClassSpec WinSpec = {sizeof(Win), sizeof(uintptr_t), 0, nullptr, 0};

constexpr size_t VisibleFlagsOffset = 0x9C;
constexpr size_t CoordinateFlagsOffset = 0x98;
constexpr size_t ParentOffset = 0xC4;
constexpr size_t OuterLeftOffset = 0x13C;
constexpr size_t OuterTopOffset = 0x140;
constexpr size_t ClientLeftOffset = 0x14C;
constexpr size_t ClientTopOffset = 0x150;
constexpr size_t MaximumDepth = 3;

// GCC emits the correct ECX ABI for this raw thiscall pointer but warns
// because it is not a C++ member-pointer type.
typedef uint32_t (OriginalObject::*OriginalNoArg)();
typedef void (OriginalObject::*OriginalCoordinates)(int *, int *);

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
    OriginalNoArg original = reinterpret_cast<OriginalNoArg>(0x005F7E90U);
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

struct CoordinateNode {
    uint32_t flags;
    uint32_t client_x;
    uint32_t client_y;
    uint32_t outer_x;
    uint32_t outer_y;
};

void build_coordinate_chain(
        WinFixture *nodes,
        uint8_t (*snapshots)[sizeof(Win) + runtime_oracle::CanarySize * 2],
        const CoordinateNode *values, size_t depth, uintptr_t *vtable) {
    for (size_t index = 0; index < depth; ++index) {
        WinFixture unused;
        runtime_oracle::initialize_pair(
            nodes[index].storage, unused.storage, WinSpec, vtable);
        write_object(nodes[index], CoordinateFlagsOffset, values[index].flags);
        write_object(nodes[index], ClientLeftOffset, values[index].client_x);
        write_object(nodes[index], ClientTopOffset, values[index].client_y);
        write_object(nodes[index], OuterLeftOffset, values[index].outer_x);
        write_object(nodes[index], OuterTopOffset, values[index].outer_y);
        Win *parent = (index + 1 < depth) ? nodes[index + 1].object() : nullptr;
        write_object(nodes[index], ParentOffset, parent);
    }
    for (size_t index = 0; index < depth; ++index) {
        memcpy(snapshots[index], nodes[index].storage,
               sizeof(nodes[index].storage));
    }
}

int coordinate_from_bits(uint32_t bits) {
    int value;
    static_assert(sizeof(value) == sizeof(bits),
                  "Win coordinate oracle requires 32-bit int");
    memcpy(&value, &bits, sizeof(value));
    return value;
}

uint32_t coordinate_bits(int value) {
    uint32_t bits;
    memcpy(&bits, &value, sizeof(bits));
    return bits;
}

bool verify_client_to_screen() {
    struct CoordinateCase {
        size_t depth;
        CoordinateNode nodes[MaximumDepth];
        uint32_t start_x;
        uint32_t start_y;
        uint32_t expected_x;
        uint32_t expected_y;
        bool alias_outputs;
    };
    const CoordinateCase cases[] = {
        // Three levels exercise both recursive calls and both parent-origin
        // subtractions with distinct, nonzero terms at every level.
        {3,
         {{0x00008020U, 100, 200, 10, 20},
          {0x00008020U, 200, 400, 20, 40},
          {0x00000000U, 300, 600, 30, 60}},
         0, 0, 610, 1220, false},
        // Local additions cross both signed boundaries.
        {1,
         {{0, 0x00000001U, 0xFFFFFFFFU, 0x00000001U, 0xFFFFFFFFU},
          {}, {}},
         0x7FFFFFFFU, 0x80000000U,
         0x80000001U, 0x7FFFFFFEU, false},
        // Parent additions wrap coordinates already adjusted by the child.
        {2,
         {{0x00000020U, 0, 0, 0, 0},
          {0, 0x00000001U, 0xFFFFFFFFU, 0x00000001U, 0xFFFFFFFFU},
          {}},
         0x7FFFFFFFU, 0x80000000U,
         0x80000001U, 0x7FFFFFFEU, false},
        // Parent terms cancel during recursion, making the later subtraction
        // itself cross the signed boundary for both axes.
        {2,
         {{0x00008020U, 0, 0, 0, 0},
          {0, 0xFFFFFFFFU, 0x00000001U, 0x00000001U, 0xFFFFFFFFU},
          {}},
         0x80000000U, 0x7FFFFFFFU,
         0x7FFFFFFFU, 0x80000000U, false},
        // The original updates x before reading y, which is observable when
        // both output pointers alias the same coordinate.
        {1,
         {{0, 100, 200, 10, 20}, {}, {}},
         7, 0xDEADBEEFU, 337, 337, true},
    };
    OriginalCoordinates original = reinterpret_cast<OriginalCoordinates>(0x005ED240U);
    for (const CoordinateCase &test : cases) {
        static WinFixture legacy_nodes[MaximumDepth];
        static WinFixture source_nodes[MaximumDepth];
        static uint8_t legacy_snapshots[MaximumDepth][sizeof(WinFixture)];
        static uint8_t source_snapshots[MaximumDepth][sizeof(WinFixture)];
        uintptr_t vtable[1];

        build_coordinate_chain(
            legacy_nodes, legacy_snapshots, test.nodes, test.depth, vtable);
        int legacy_x = coordinate_from_bits(test.start_x);
        int legacy_y = coordinate_from_bits(test.start_y);
        int *legacy_y_output = test.alias_outputs ? &legacy_x : &legacy_y;
        original(legacy_nodes[0].object(), &legacy_x, legacy_y_output);

        build_coordinate_chain(
            source_nodes, source_snapshots, test.nodes, test.depth, vtable);
        int source_x = coordinate_from_bits(test.start_x);
        int source_y = coordinate_from_bits(test.start_y);
        int *source_y_output = test.alias_outputs ? &source_x : &source_y;
        source_nodes[0].object()->client_to_screen(&source_x, source_y_output);

        const uint32_t legacy_x_bits = coordinate_bits(legacy_x);
        const uint32_t legacy_y_bits = coordinate_bits(*legacy_y_output);
        const uint32_t source_x_bits = coordinate_bits(source_x);
        const uint32_t source_y_bits = coordinate_bits(*source_y_output);
        if (legacy_x_bits != test.expected_x
                || legacy_y_bits != test.expected_y
                || source_x_bits != legacy_x_bits
                || source_y_bits != legacy_y_bits) {
            return false;
        }
        // Translation is a pure query. Parent pointers differ between sides,
        // so compare each chain with its own complete pre-call snapshot.
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
    return verify_is_visible() && verify_client_to_screen();
}
