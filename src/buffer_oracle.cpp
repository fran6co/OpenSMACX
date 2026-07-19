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
#include "buffer_oracle.h"

#include "buffer.h"
#include "runtime_oracle.h"

namespace {

using BufferFixture = runtime_oracle::Fixture<Buffer>;

// Neither method dispatches through the Buffer vtable; both dispatch through a
// DirectDraw surface interface the fixture supplies instead.
const runtime_oracle::ClassSpec BufferSpec = {
    sizeof(Buffer), sizeof(uintptr_t), 0, nullptr, 0};

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
#endif
typedef uint32_t (__thiscall *OriginalNoArg)(Buffer *);
typedef void (__thiscall *OriginalOneArg)(Buffer *, int);
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

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
    auto *bytes = static_cast<uint8_t *>(descriptor);
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
    auto original = reinterpret_cast<OriginalNoArg>(0x005E3373U);
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
    auto original = reinterpret_cast<OriginalOneArg>(0x005E34A3U);
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

}  // namespace

bool run_buffer_oracle_suite() {
    return verify_get_data() && verify_free_data();
}
