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
#include "stringstruct_oracle.h"

#include "runtime_oracle.h"
#include "stringstruct.h"

namespace {

typedef runtime_oracle::Fixture<StringStruct> ListFixture;

// The list dispatches through its own vtable and through stand-in entry and
// payload objects the fixture supplies, so the shared initializer installs no
// vtable of its own.
const runtime_oracle::ClassSpec ListSpec = {
    sizeof(StringStruct), sizeof(uintptr_t), 0, nullptr, 0};

typedef void (OriginalObject::*OriginalNoArg)();

struct Probe {
    int visits;
    int payload_destroys;
    int entry_destroys;
    int flags;
};

Probe Record = {0};

void __fastcall probe_visit(void *, int /* edx */, void *) {
    ++Record.visits;
}

void __fastcall probe_payload_destroy(void *, int /* edx */, int flags) {
    ++Record.payload_destroys;
    Record.flags = flags;
}

void __fastcall probe_entry_destroy(void *, int /* edx */, int flags) {
    ++Record.entry_destroys;
    Record.flags = flags;
}

// Slot 1 carries the virtual-base displacement, which is zero so the object is
// its own destructible subobject; slot 0 is the scalar deleting destructor.
struct Destructible {
    uint32_t *vptr;
    uint32_t vtable[2];
};

void arm(Destructible &object, void *destructor) {
    object.vtable[0] = static_cast<uint32_t>(
        reinterpret_cast<uintptr_t>(destructor));
    object.vtable[1] = 0;
    object.vptr = object.vtable;
}

struct ListScenario {
    StringStructEntry entries[3];
    Destructible entry_objects[3];
    Destructible payloads[3];
    uint32_t owner_vtable[2];
};

void build(ListScenario &scenario, int count, bool with_payloads) {
    memset(&scenario, 0, sizeof(scenario));
    scenario.owner_vtable[1] = static_cast<uint32_t>(
        reinterpret_cast<uintptr_t>(&probe_visit));
    for (int index = 0; index < count; ++index) {
        arm(scenario.entry_objects[index],
            reinterpret_cast<void *>(&probe_entry_destroy));
        arm(scenario.payloads[index],
            reinterpret_cast<void *>(&probe_payload_destroy));
        scenario.entries[index].abi_word = static_cast<uint32_t>(
            reinterpret_cast<uintptr_t>(scenario.entry_objects[index].vtable));
        scenario.entries[index].payload = with_payloads
            ? static_cast<int>(
                  reinterpret_cast<uintptr_t>(&scenario.payloads[index]))
            : 0;
        scenario.entries[index].next = (index + 1 < count)
            ? &scenario.entries[index + 1] : nullptr;
    }
}

bool verify_remove_all() {
    struct RemoveCase {
        int entries;
        int count;
        bool with_payloads;
    };
    const RemoveCase cases[] = {
        {0, 0, false},
        {3, 0, true},
        {3, -1, true},
        {1, 1, true},
        {3, 3, true},
        {3, 3, false},
        {3, 2, true},
    };
    OriginalNoArg original = reinterpret_cast<OriginalNoArg>(0x00402970U);
    for (size_t test_index = 0;
         test_index < sizeof(cases) / sizeof(cases[0]); ++test_index) {
        const RemoveCase &test = cases[test_index];
        ListFixture legacy;
        ListFixture source;
        uintptr_t vtable[1];
        runtime_oracle::initialize_pair(
            legacy.storage, source.storage, ListSpec, vtable);

        // Each side walks its own entry graph, so only the list state, the
        // callback record, and the cleared payload fields are compared.
        ListScenario legacy_scenario;
        ListScenario source_scenario;
        build(legacy_scenario, test.entries, test.with_payloads);
        build(source_scenario, test.entries, test.with_payloads);

        for (int side = 0; side < 2; ++side) {
            ListFixture &fixture = side ? source : legacy;
            ListScenario &scenario = side ? source_scenario : legacy_scenario;
            uint8_t *base = fixture.storage + runtime_oracle::CanarySize;
            const uint32_t owner = static_cast<uint32_t>(
                reinterpret_cast<uintptr_t>(scenario.owner_vtable));
            const uint32_t head = test.entries
                ? static_cast<uint32_t>(
                      reinterpret_cast<uintptr_t>(&scenario.entries[0])) : 0U;
            const uint32_t count = static_cast<uint32_t>(test.count);
            const uint32_t zero = 0;
            memcpy(base + 0x00, &owner, sizeof(owner));
            memcpy(base + 0x08, &head, sizeof(head));
            memcpy(base + 0x0C, &zero, sizeof(zero));
            memcpy(base + 0x10, &count, sizeof(count));
            memcpy(base + 0x14, &zero, sizeof(zero));
        }

        Record = Probe();
        original(legacy.object());
        const Probe legacy_record = Record;

        Record = Probe();
        source.object()->remove_all();

        if (memcmp(&legacy_record, &Record, sizeof(Record)) != 0) {
            return false;
        }
        // Compare list state apart from the two fields that necessarily hold
        // side-specific entry addresses.
        size_t offset_cases[] = {size_t(0x10), size_t(0x14),
                                 size_t(0x18), size_t(0x1C), size_t(0x20)};
        for (size_t offset_index = 0;
             offset_index < sizeof(offset_cases) / sizeof(offset_cases[0]);
             ++offset_index) {
            size_t offset = offset_cases[offset_index];
            if (memcmp(legacy.storage + runtime_oracle::CanarySize + offset,
                       source.storage + runtime_oracle::CanarySize + offset,
                       sizeof(uint32_t)) != 0) {
                return false;
            }
        }
        // Payload fields hold addresses inside each side's own scenario, so
        // only whether the walk cleared them is comparable.
        for (int index = 0; index < test.entries; ++index) {
            const bool legacy_cleared =
                legacy_scenario.entries[index].payload == 0;
            const bool source_cleared =
                source_scenario.entries[index].payload == 0;
            if (legacy_cleared != source_cleared) {
                return false;
            }
        }
        // The surviving head must be the same position in each side's list.
        int legacy_head_index = -1;
        int source_head_index = -1;
        uint32_t legacy_head = 0;
        uint32_t source_head = 0;
        memcpy(&legacy_head,
               legacy.storage + runtime_oracle::CanarySize + 0x08,
               sizeof(legacy_head));
        memcpy(&source_head,
               source.storage + runtime_oracle::CanarySize + 0x08,
               sizeof(source_head));
        for (int index = 0; index < test.entries; ++index) {
            if (legacy_head == static_cast<uint32_t>(
                    reinterpret_cast<uintptr_t>(&legacy_scenario.entries[index]))) {
                legacy_head_index = index;
            }
            if (source_head == static_cast<uint32_t>(
                    reinterpret_cast<uintptr_t>(&source_scenario.entries[index]))) {
                source_head_index = index;
            }
        }
        if (legacy_head_index != source_head_index) {
            return false;
        }
        // Canaries must be untouched on both sides.
        if (memcmp(legacy.storage, source.storage,
                   runtime_oracle::CanarySize) != 0
                || memcmp(legacy.storage + runtime_oracle::CanarySize
                              + sizeof(StringStruct),
                          source.storage + runtime_oracle::CanarySize
                              + sizeof(StringStruct),
                          runtime_oracle::CanarySize) != 0) {
            return false;
        }
    }
    return true;
}

bool verify_close() {
    struct CloseCase { int entries; int count; bool with_payloads; };
    // These bodies install the real virtual table before walking, so the walk
    // would dispatch through original game code rather than the fixture's
    // stand-in and is unsafe to drive here. Only non-walking shapes are used:
    // an empty list, and a populated list whose count stops the walk. The walk
    // itself is covered by verify_remove_all, which leaves the table alone.
    const CloseCase cases[] = {
        {0, 0, false},
        {3, 0, true},
        {3, -1, true},
        {3, 0, false},
    };
    typedef void (OriginalObject::*OriginalClose)();
    // Both the base close and the derived two-stage close share the same
    // fixture shape; only the entry address, adjustor and expected primary
    // table differ.
    struct Variant {
        uintptr_t address;
        size_t adjustment;
        uint32_t expected_primary;
    };
    const Variant variants[] = {
        {0x00401060U, StringStructCloseAdjustment, StringStructVtable},
        {0x004066C0U, StringStructDerivedCloseAdjustment, StringStructVtable},
    };
    for (size_t variant_index = 0;
         variant_index < sizeof(variants) / sizeof(variants[0]);
         ++variant_index) {
    const Variant &variant = variants[variant_index];
    OriginalClose original = reinterpret_cast<OriginalClose>(variant.address);
    for (size_t test_index = 0;
         test_index < sizeof(cases) / sizeof(cases[0]); ++test_index) {
        const CloseCase &test = cases[test_index];
        ListFixture legacy;
        ListFixture source;
        uintptr_t vtable[1];
        runtime_oracle::initialize_pair(
            legacy.storage, source.storage, ListSpec, vtable);
        ListScenario legacy_scenario;
        ListScenario source_scenario;
        build(legacy_scenario, test.entries, test.with_payloads);
        build(source_scenario, test.entries, test.with_payloads);

        // The virtual-base table each side uses: slot one holds the
        // displacement from offset 4 to the subobject whose table is written.
        // Displacement 0x10 would land the write at offset 0x14
        // (current_position_'s slot), which close_with_tables unconditionally
        // overwrites with zero two lines later - making the entire read of
        // vbtable[1] and the write it drives unobservable regardless of
        // whether the displacement is computed correctly. 0x14 lands the
        // write at offset 0x18 (allocator_), which close_with_tables never
        // touches and which the comparison loop below already checks.
        uint32_t legacy_vbtable[2] = {0, 0x14};
        uint32_t source_vbtable[2] = {0, 0x14};

        for (int side = 0; side < 2; ++side) {
            ListFixture &fixture = side ? source : legacy;
            ListScenario &scenario = side ? source_scenario : legacy_scenario;
            uint32_t *vb = side ? source_vbtable : legacy_vbtable;
            uint8_t *base = fixture.storage + runtime_oracle::CanarySize;
            const uint32_t owner = static_cast<uint32_t>(
                reinterpret_cast<uintptr_t>(scenario.owner_vtable));
            const uint32_t head = test.entries
                ? static_cast<uint32_t>(
                      reinterpret_cast<uintptr_t>(&scenario.entries[0])) : 0U;
            const uint32_t count = static_cast<uint32_t>(test.count);
            const uint32_t vb_pointer = static_cast<uint32_t>(
                reinterpret_cast<uintptr_t>(vb));
            const uint32_t zero = 0;
            // current_position_ is seeded to a nonzero sentinel rather than
            // zero: close_with_tables resets it unconditionally, so a
            // zero-seeded fixture can never observe whether that reset fired.
            const uint32_t stale_position = 0x77777777U;
            memcpy(base + 0x00, &owner, sizeof(owner));
            memcpy(base + 0x04, &vb_pointer, sizeof(vb_pointer));
            memcpy(base + 0x08, &head, sizeof(head));
            memcpy(base + 0x0C, &zero, sizeof(zero));
            memcpy(base + 0x10, &count, sizeof(count));
            memcpy(base + 0x14, &stale_position, sizeof(stale_position));
        }

        Record = Probe();
        original(legacy.storage + runtime_oracle::CanarySize
                 + variant.adjustment);
        const Probe legacy_record = Record;

        Record = Probe();
        void *const source_entry = source.storage + runtime_oracle::CanarySize
            + variant.adjustment;
        if (variant.address == 0x00401060U) {
            string_struct_close_redirect(source_entry, nullptr);
        } else {
            string_struct_derived_close_redirect(source_entry, nullptr);
        }

        if (memcmp(&legacy_record, &Record, sizeof(Record)) != 0) {
            return false;
        }
        // Offsets 0x00 and 0x04 hold side-specific table pointers; compare the
        // installed primary table and the remaining state directly.
        uint32_t legacy_primary = 0;
        uint32_t source_primary = 0;
        memcpy(&legacy_primary,
               legacy.storage + runtime_oracle::CanarySize, sizeof(uint32_t));
        memcpy(&source_primary,
               source.storage + runtime_oracle::CanarySize, sizeof(uint32_t));
        if (legacy_primary != source_primary
                || legacy_primary != variant.expected_primary) {
            return false;
        }
        if (legacy_vbtable[1] != source_vbtable[1]) {
            return false;
        }
        size_t offset_cases[] = {size_t(0x10), size_t(0x14),
                                 size_t(0x18), size_t(0x1C), size_t(0x20)};
        for (size_t offset_index = 0;
             offset_index < sizeof(offset_cases) / sizeof(offset_cases[0]);
             ++offset_index) {
            size_t offset = offset_cases[offset_index];
            if (memcmp(legacy.storage + runtime_oracle::CanarySize + offset,
                       source.storage + runtime_oracle::CanarySize + offset,
                       sizeof(uint32_t)) != 0) {
                return false;
            }
        }
        for (int index = 0; index < test.entries; ++index) {
            if ((legacy_scenario.entries[index].payload == 0)
                    != (source_scenario.entries[index].payload == 0)) {
                return false;
            }
        }
    }
    }
    return true;
}

}  // namespace

bool run_string_struct_oracle_suite() {
    return verify_remove_all() && verify_close();
}
