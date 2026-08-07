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
#include "buffer_oracle.h"

#include "buffer.h"
#include "font.h"
#include "runtime_oracle.h"
#include "temp.h"

// What this oracle PROVES: each function below is executed here in its
// ORIGINAL form, at its canonical address inside the hybrid process, and
// compared against the recovered implementation. These markers are the
// input to `unproven_recovered` in docs/recovery/summary.json; see
// tools/export_proven_functions.py for what does and does not count.
//
// PROVEN-AGAINST-ORIGINAL: 0x005E3373  ?get_data@Buffer@@QAEHXZ


namespace {

typedef runtime_oracle::Fixture<Buffer> BufferFixture;

// Neither method dispatches through the Buffer vtable; both dispatch through a
// DirectDraw surface interface the fixture supplies instead.
const runtime_oracle::ClassSpec BufferSpec = {
    sizeof(Buffer), sizeof(uintptr_t), 0, nullptr, 0};

typedef uint32_t (OriginalObject::*OriginalNoArg)();
typedef void (OriginalObject::*OriginalOneArg)(int);

// A stand-in DirectDraw surface. Locking never touches real video memory: the
// probe reports a fixed pitch and data pointer so both sides observe identical
// results, and unlocking only records its arguments.
struct SurfaceProbe {
    int lock_calls;
    int unlock_calls;
    uint32_t descriptor_size;
    void *unlock_data;
};

SurfaceProbe Probe = {};
long ProbeLockResult = 0;
uint32_t ProbeLockPitch = 0;
uint32_t ProbeLockData = 0;

long __stdcall probe_lock(
        void *, void *rect, void *descriptor, uint32_t flags, void *event) {
    ++Probe.lock_calls;
    uint8_t *bytes = static_cast<uint8_t *>(descriptor);
    memcpy(&Probe.descriptor_size, bytes, sizeof(Probe.descriptor_size));
    if (rect != nullptr || flags != 1U || event != nullptr) {
        return -1;
    }
    if (ProbeLockResult == 0) {
        memcpy(bytes + 0x10, &ProbeLockPitch, sizeof(ProbeLockPitch));
        memcpy(bytes + 0x24, &ProbeLockData, sizeof(ProbeLockData));
    }
    return ProbeLockResult;
}

long __stdcall probe_unlock(void *, void *data) {
    ++Probe.unlock_calls;
    Probe.unlock_data = data;
    return 0;
}

struct FakeSurface {
    void **vtable;
};

void *SurfaceVtable[(0x80 / sizeof(void *)) + 1] = {};
FakeSurface Surface = {SurfaceVtable};

void install_surface() {
    SurfaceVtable[BufferSurfaceLockSlot / sizeof(void *)] =
        reinterpret_cast<void *>(&probe_lock);
    SurfaceVtable[BufferSurfaceUnlockSlot / sizeof(void *)] =
        reinterpret_cast<void *>(&probe_unlock);
}

void write_field(BufferFixture &fixture, size_t offset, uint32_t value) {
    memcpy(fixture.storage + runtime_oracle::CanarySize + offset,
           &value, sizeof(value));
}

bool verify_get_data() {
    struct GetCase {
        bool has_surface;
        uint32_t field_50;
        uint32_t field_54;
        uint32_t references;
        long lock_result;
        uint32_t pitch;
        uint32_t data;
    };
    const GetCase cases[] = {
        {false, 0x9999U, 0, 4, 0, 0, 0},
        {false, 0x9999U, 0x4444U, 4, 0, 0, 0},
        {false, 0, 0xFFFFFFFFU, 0, 0, 0, 0},
        {true, 0x5555U, 0, 7, 0, 0, 0},
        {true, 0, 0, 0, 0, 0x1234U, 0x8888U},
        {true, 0, 0, 3, 1, 0x1234U, 0x8888U},
        {true, 0, 0, 0xFFFFFFFFU, 0, 0xFFFFFFFFU, 0xABCDU},
    };
    install_surface();
    OriginalNoArg original = reinterpret_cast<OriginalNoArg>(0x005E3373U);
    for (const GetCase &test : cases) {
        BufferFixture legacy;
        BufferFixture source;
        uintptr_t vtable[1];
        runtime_oracle::initialize_pair(
            legacy.storage, source.storage, BufferSpec, vtable);
        const uint32_t surface = test.has_surface
            ? static_cast<uint32_t>(reinterpret_cast<uintptr_t>(&Surface)) : 0U;
        for (BufferFixture *fixture : {&legacy, &source}) {
            write_field(*fixture, 0x50, test.field_50);
            write_field(*fixture, 0x54, test.field_54);
            write_field(*fixture, 0x58, surface);
            write_field(*fixture, 0x6C, test.references);
            write_field(*fixture, 0x4A8, 0);
        }
        ProbeLockResult = test.lock_result;
        ProbeLockPitch = test.pitch;
        ProbeLockData = test.data;

        Probe = SurfaceProbe();
        const uint32_t legacy_result = original(legacy.object());
        const SurfaceProbe legacy_probe = Probe;

        Probe = SurfaceProbe();
        const uint32_t source_result =
            static_cast<uint32_t>(source.object()->get_data());

        if (legacy_result != source_result
                || memcmp(&legacy_probe, &Probe, sizeof(Probe)) != 0
                || memcmp(legacy.storage, source.storage,
                          sizeof(legacy.storage)) != 0) {
            return false;
        }
    }
    return true;
}

bool verify_free_data() {
    struct FreeCase {
        bool has_surface;
        uint32_t field_50;
        uint32_t references;
        int count;
    };
    const FreeCase cases[] = {
        {false, 0x5555U, 3, 1},
        {false, 0x5555U, 1, 1},
        {false, 0x5555U, 1, 4},
        {false, 0, 1, 1},
        {true, 0x5555U, 3, 1},
        {true, 0x5555U, 1, 1},
        {true, 0, 1, 1},
        {true, 0x5555U, 0, static_cast<int>(0x80000000U)},
        {true, 0x5555U, 0x7FFFFFFFU, 1},
        {false, 0x5555U, 0x80000000U, static_cast<int>(0xFFFFFFFFU)},
    };
    install_surface();
    OriginalOneArg original = reinterpret_cast<OriginalOneArg>(0x005E34A3U);
    for (const FreeCase &test : cases) {
        BufferFixture legacy;
        BufferFixture source;
        uintptr_t vtable[1];
        runtime_oracle::initialize_pair(
            legacy.storage, source.storage, BufferSpec, vtable);
        const uint32_t surface = test.has_surface
            ? static_cast<uint32_t>(reinterpret_cast<uintptr_t>(&Surface)) : 0U;
        for (BufferFixture *fixture : {&legacy, &source}) {
            write_field(*fixture, 0x50, test.field_50);
            write_field(*fixture, 0x58, surface);
            write_field(*fixture, 0x6C, test.references);
        }

        Probe = SurfaceProbe();
        original(legacy.object(), test.count);
        const SurfaceProbe legacy_probe = Probe;

        Probe = SurfaceProbe();
        source.object()->free_data(test.count);

        if (memcmp(&legacy_probe, &Probe, sizeof(Probe)) != 0
                || memcmp(legacy.storage, source.storage,
                          sizeof(legacy.storage)) != 0) {
            return false;
        }
    }
    return true;
}

bool verify_text_line_height() {
    struct HeightCase { int override_value; int line_height; int height; };
    const HeightCase cases[] = {
        {0, 111, 222}, {5, 111, 222}, {-1, 111, 222},
        {INT_MIN, 111, 222}, {INT_MAX, 111, 1}, {1, INT_MAX, INT_MAX},
    };
    OriginalNoArg original = reinterpret_cast<OriginalNoArg>(0x005DCAB0U);
    Font *const saved_default = *FontDefaultPtr;
    bool passed = true;
    for (const HeightCase &test : cases) {
        for (int preset_font = 0; preset_font < 2 && passed; ++preset_font) {
            BufferFixture legacy;
            BufferFixture source;
            uintptr_t vtable[1];
            runtime_oracle::initialize_pair(
                legacy.storage, source.storage, BufferSpec, vtable);

            alignas(Font) uint8_t font_storage[sizeof(Font)] = {};
            int *const fields = reinterpret_cast<int *>(font_storage);
            fields[0x00 / 4] = test.override_value;
            fields[0x0C / 4] = test.line_height;
            fields[0x10 / 4] = test.height;
            const uint32_t font = static_cast<uint32_t>(
                reinterpret_cast<uintptr_t>(font_storage));

            // Both sides share one font object and one default, so the
            // published pointer is directly comparable.
            *FontDefaultPtr = reinterpret_cast<Font *>(font_storage);
            write_field(legacy, 0x52C, preset_font ? font : 0U);
            write_field(source, 0x52C, preset_font ? font : 0U);

            const uint32_t legacy_result = original(legacy.object());
            const uint32_t source_result = static_cast<uint32_t>(
                source.object()->text_line_height());
            if (legacy_result != source_result
                    || memcmp(legacy.storage, source.storage,
                              sizeof(legacy.storage)) != 0) {
                passed = false;
            }
        }
    }
    *FontDefaultPtr = saved_default;
    return passed;
}

bool verify_find_font() {
    const int sizes[] = {
        INT_MIN, -100000, -5, 0, 8, 9, 10, 11, 12, 13, 22, 26, 31, 40, 48,
        49, 10046, 10047, 100000, INT_MAX,
    };
    for (int size : sizes) {
        for (int style = 0; style < 4; ++style) {
            // find_font is cdecl and reads only original-image tables, so both
            // sides observe identical inputs.
            const Font *const legacy_result =
                reinterpret_cast<Font *(__cdecl *)(int, int)>(0x005882F0U)(size, style);
            const Font *const source_result = find_font(size, style);
            if (legacy_result != source_result) {
                return false;
            }
        }
    }
    return true;
}

bool verify_close_reset() {
    // Fixtures own no resources: null device context, null surfaces and GDI
    // objects, and null owned allocations. That exercises the twenty-entry
    // release loop's skip path, both mode branches' guards, and the whole
    // field reset, which is where the transcription risk lives. Releases with
    // real allocations belong to the deferred phase.
    OriginalNoArg original = reinterpret_cast<OriginalNoArg>(0x005D7470U);
    const int saved_mode = *BufferDirectDrawActive;
    bool passed = true;
    for (int direct_draw = 0; direct_draw < 2 && passed; ++direct_draw) {
        for (int flag_bit = 0; flag_bit < 2 && passed; ++flag_bit) {
            BufferFixture legacy;
            BufferFixture source;
            uintptr_t vtable[1];
            runtime_oracle::initialize_pair(
                legacy.storage, source.storage, BufferSpec, vtable);
            for (BufferFixture *fixture : {&legacy, &source}) {
                for (size_t index = 0; index < 20; ++index) {
                    write_field(*fixture, 0x4BC + index * 4, 0);
                }
                write_field(*fixture, 0x1C, flag_bit ? 4U : 0U);
                for (size_t offset : {size_t(0x58), size_t(0x5C), size_t(0x60),
                                      size_t(0x64), size_t(0x70), size_t(0x74),
                                      size_t(0x78)}) {
                    write_field(*fixture, offset, 0);
                }
            }
            *BufferDirectDrawActive = direct_draw;
            original(legacy.object());
            source.object()->close();
            if (memcmp(legacy.storage, source.storage,
                       sizeof(legacy.storage)) != 0) {
                passed = false;
            }
        }
    }
    *BufferDirectDrawActive = saved_mode;
    return passed;
}

bool verify_destroy() {
    // Same resource-free shape as the close reset, plus the vtable store and
    // the Spot subobject teardown the destructor adds around it.
    OriginalNoArg original = reinterpret_cast<OriginalNoArg>(0x005D7410U);
    const int saved_mode = *BufferDirectDrawActive;
    bool passed = true;
    for (int direct_draw = 0; direct_draw < 2 && passed; ++direct_draw) {
        BufferFixture legacy;
        BufferFixture source;
        uintptr_t vtable[1];
        runtime_oracle::initialize_pair(
            legacy.storage, source.storage, BufferSpec, vtable);
        for (BufferFixture *fixture : {&legacy, &source}) {
            for (size_t index = 0; index < 20; ++index) {
                write_field(*fixture, 0x4BC + index * 4, 0);
            }
            write_field(*fixture, 0x1C, 0);
            for (size_t offset : {size_t(0x58), size_t(0x5C), size_t(0x60),
                                  size_t(0x64), size_t(0x70), size_t(0x74),
                                  size_t(0x78)}) {
                write_field(*fixture, offset, 0);
            }
            // The Spot subobject owns resources of its own, so its fields
            // are left empty rather than seeded with sentinel pointers.
            for (size_t offset : {size_t(0x4B0), size_t(0x4B4), size_t(0x4B8)}) {
                write_field(*fixture, offset, 0);
            }
        }
        *BufferDirectDrawActive = direct_draw;
        original(legacy.object());
        source.object()->destroy();
        if (memcmp(legacy.storage, source.storage,
                   sizeof(legacy.storage)) != 0) {
            passed = false;
        }
    }
    *BufferDirectDrawActive = saved_mode;
    return passed;
}

}  // namespace

bool run_buffer_oracle_suite() {
    return verify_get_data() && verify_free_data()
        && verify_text_line_height() && verify_find_font()
        && verify_close_reset() && verify_destroy();
}

namespace {

constexpr uintptr_t BufferCloseAddress = 0x005D7470U;

}  // namespace

bool run_buffer_release_suite() {
    if (!suspend_redirect_at(BufferCloseAddress)) {
        return false;
    }
    OriginalNoArg original = reinterpret_cast<OriginalNoArg>(BufferCloseAddress);
    const int saved_mode = *BufferDirectDrawActive;
    bool passed = true;

    // Each side owns distinct allocations so both genuinely release; the loop
    // clears every slot, leaving the objects directly comparable afterwards.
    for (int filled = 0; filled < 3 && passed; ++filled) {
        BufferFixture legacy;
        BufferFixture source;
        uintptr_t vtable[1];
        runtime_oracle::initialize_pair(
            legacy.storage, source.storage, BufferSpec, vtable);
        bool allocation_failed = false;
        for (size_t index = 0; index < 20; ++index) {
            // filled 0: none owned, 1: every other slot, 2: all owned.
            const bool owned = (filled == 2) || (filled == 1 && (index % 2) == 0);
            void *legacy_block = owned ? _malloc(48) : nullptr;
            void *source_block = owned ? _malloc(48) : nullptr;
            if (owned && (!legacy_block || !source_block)) {
                allocation_failed = true;
                break;
            }
            write_field(legacy, 0x4BC + index * 4, static_cast<uint32_t>(
                reinterpret_cast<uintptr_t>(legacy_block)));
            write_field(source, 0x4BC + index * 4, static_cast<uint32_t>(
                reinterpret_cast<uintptr_t>(source_block)));
        }
        if (allocation_failed) {
            passed = false;
            break;
        }
        for (BufferFixture *fixture : {&legacy, &source}) {
            write_field(*fixture, 0x1C, 0);
            for (size_t offset : {size_t(0x58), size_t(0x5C), size_t(0x60),
                                  size_t(0x64), size_t(0x70), size_t(0x74),
                                  size_t(0x78)}) {
                write_field(*fixture, offset, 0);
            }
        }
        *BufferDirectDrawActive = 0;
        original(legacy.object());
        source.object()->close();
        if (memcmp(legacy.storage, source.storage,
                   sizeof(legacy.storage)) != 0) {
            passed = false;
        }
    }
    *BufferDirectDrawActive = saved_mode;
    if (!resume_redirect_at(BufferCloseAddress)) {
        return false;
    }
    return passed;
}
