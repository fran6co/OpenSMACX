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
#include "graphicwin_oracle.h"

#include "graphicwin.h"
#include "runtime_oracle.h"

// What this oracle PROVES: each function below is executed here in its
// ORIGINAL form, at its canonical address inside the hybrid process, and
// compared against the recovered implementation. These markers are the
// input to `unproven_recovered` in docs/recovery/summary.json; see
// tools/export_proven_functions.py for what does and does not count.
//
// PROVEN-AGAINST-ORIGINAL: 0x005D4E40  ?close@GraphicWin@@QAEXXZ


namespace {

typedef runtime_oracle::Fixture<GraphicWin> GraphicWinFixture;

constexpr size_t VtableOffsets[] = {0};
const runtime_oracle::ClassSpec GraphicWinSpec = {
    sizeof(GraphicWin), 0x0C, 0x08, VtableOffsets, ARRAYSIZE(VtableOffsets)};

constexpr size_t TraceOffsets[] = {
    0x134, 0x138, 0x9CC, 0xA08, 0xA0C, 0xA10,
};

typedef uint32_t (OriginalObject::*OriginalNoArg)();

struct FakeReleaseObject {
    uintptr_t *vtable;
};

int ReleaseCalls = 0;
void *ReleaseTarget = nullptr;
uint32_t ReleaseFlags = 0;

uint32_t __fastcall recording_release(void *target, int /* edx */, uint32_t flags) {
    ++ReleaseCalls;
    ReleaseTarget = target;
    ReleaseFlags = flags;
    return 0x7B3D19E5U;
}

void write_field(GraphicWinFixture &fixture, size_t offset, uint32_t value) {
    memcpy(fixture.storage + runtime_oracle::CanarySize + offset,
           &value, sizeof(value));
}

uint32_t read_field(const GraphicWinFixture &fixture, size_t offset) {
    uint32_t value;
    memcpy(&value,
           fixture.storage + runtime_oracle::CanarySize + offset,
           sizeof(value));
    return value;
}

void prepare_fixture(GraphicWinFixture &fixture, uintptr_t *win_vtable,
                     FakeReleaseObject *release_target) {
    // Win::close and Buffer::close are dependencies of the recovered body.
    // Keep their resource-owning fields empty so both can run safely while
    // DllMain holds the loader lock, then poison every GraphicWin-owned field
    // this recovery is responsible for resetting.
    uint8_t *const object =
        fixture.storage + runtime_oracle::CanarySize;
    memset(object, 0, sizeof(GraphicWin));
    memcpy(object, &win_vtable, sizeof(win_vtable));
    write_field(fixture, 0x134, 0x13579BDFU);
    write_field(fixture, 0x138, 0x2468ACE0U);
    for (size_t offset = 0x9CC; offset <= 0xA10; offset += 4) {
        write_field(fixture, offset,
                    0xA5000000U ^ (offset * 0x1021U));
    }
    write_field(fixture, 0xA08,
                reinterpret_cast<uintptr_t>(release_target));
}

bool verify_close() {
    uintptr_t win_vtable[3] = {0};
    win_vtable[2] = reinterpret_cast<uintptr_t>(&runtime_oracle::probe);
    uintptr_t release_vtable[] = {
        reinterpret_cast<uintptr_t>(&recording_release),
    };
    FakeReleaseObject release_object = {release_vtable};
    OriginalNoArg original = original_method<OriginalNoArg>(0x005D4E40U);

    for (int owns_release_target = 0; owns_release_target < 2;
         ++owns_release_target) {
        GraphicWinFixture legacy;
        GraphicWinFixture source;
        runtime_oracle::initialize_pair(
            legacy.storage, source.storage, GraphicWinSpec, win_vtable);
        FakeReleaseObject *const release_target = owns_release_target
            ? &release_object : nullptr;
        prepare_fixture(legacy, win_vtable, release_target);
        prepare_fixture(source, win_vtable, release_target);

        ReleaseCalls = 0;
        ReleaseTarget = nullptr;
        ReleaseFlags = 0;
        runtime_oracle::begin_trace(
            legacy.object(), TraceOffsets, ARRAYSIZE(TraceOffsets));
        const uint32_t legacy_result = (ORIGINAL(legacy.object())->*original)();
        const runtime_oracle::Trace legacy_trace =
            runtime_oracle::current_trace();
        const int legacy_release_calls = ReleaseCalls;
        void *const legacy_release_target = ReleaseTarget;
        const uint32_t legacy_release_flags = ReleaseFlags;

        ReleaseCalls = 0;
        ReleaseTarget = nullptr;
        ReleaseFlags = 0;
        runtime_oracle::begin_trace(
            source.object(), TraceOffsets, ARRAYSIZE(TraceOffsets));
        const uint32_t source_result = source.object()->close();
        const runtime_oracle::Trace source_trace =
            runtime_oracle::current_trace();

        const int expected_release_calls = owns_release_target ? 1 : 0;
        // The shared Win::close dependency publishes `this` at offset 0xA8.
        // Validate that fixture-relative result on each side, then normalize
        // the necessarily different addresses before the complete byte
        // comparison. No GraphicWin-owned field is excluded.
        const bool self_pointers_match =
            read_field(legacy, 0xA8) == reinterpret_cast<uintptr_t>(
                legacy.object())
            && read_field(source, 0xA8) == reinterpret_cast<uintptr_t>(
                source.object());
        write_field(legacy, 0xA8, 0);
        write_field(source, 0xA8, 0);
        if (!self_pointers_match
                || !runtime_oracle::equivalent(
                legacy.storage, source.storage, sizeof(legacy.storage),
                legacy_result, source_result, legacy_trace, source_trace)
                || legacy_release_calls != expected_release_calls
                || ReleaseCalls != expected_release_calls
                || legacy_release_target != ReleaseTarget
                || legacy_release_flags != ReleaseFlags
                || (owns_release_target
                    && (ReleaseTarget != &release_object || ReleaseFlags != 1U))) {
            return false;
        }
    }
    return true;
}

}  // namespace

bool run_graphic_win_oracle_suite() {
    return verify_close();
}
