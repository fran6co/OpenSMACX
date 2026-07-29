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
#include "basebutton_oracle.h"

#include "basebutton.h"
#include "flatbutton.h"
#include "runtime_oracle.h"
#include "temp.h"

// What this oracle PROVES: each function below is executed here in its
// ORIGINAL form, at its canonical address inside the hybrid process, and
// compared against the recovered implementation. These markers are the
// input to `unproven_recovered` in docs/recovery/summary.json; see
// tools/export_proven_functions.py for what does and does not count.
//
// PROVEN-AGAINST-ORIGINAL: 0x006070C0  ?close@BaseButton@@QAEXXZ
// PROVEN-AGAINST-ORIGINAL: 0x00607DA0  ?close@FlatButton@@QAEXXZ


namespace {

using BaseButtonFixture = runtime_oracle::Fixture<BaseButton>;
using FlatButtonFixture = runtime_oracle::Fixture<FlatButton>;

constexpr size_t VtableOffsets[] = {0};
const runtime_oracle::ClassSpec BaseButtonSpec = {
    sizeof(BaseButton), 0x0C, 0x08,
    VtableOffsets, ARRAYSIZE(VtableOffsets),
};
const runtime_oracle::ClassSpec FlatButtonSpec = {
    sizeof(FlatButton), 0x0C, 0x08,
    VtableOffsets, ARRAYSIZE(VtableOffsets),
};
const size_t TraceOffsets[] = {
    0x000, 0x444, 0xA14, 0xA18, 0xA44, 0xA74, 0xAB8, 0xABC,
};

constexpr uintptr_t BaseButtonDestructorAddress = 0x00607040U;
constexpr uintptr_t BaseButtonCloseAddress = 0x006070C0U;
constexpr uintptr_t FlatButtonCloseAddress = 0x00607DA0U;
constexpr uintptr_t FlatButtonDestructorAddress = 0x00406880U;

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
#endif
typedef uint32_t (__thiscall *OriginalBaseNoArg)(BaseButton *);
typedef uint32_t (__thiscall *OriginalFlatNoArg)(FlatButton *);
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

template <typename Fixture>
void write_field(Fixture &fixture, size_t offset, uint32_t value) {
    memcpy(fixture.storage + runtime_oracle::CanarySize + offset,
           &value, sizeof(value));
}

template <typename Fixture>
uint32_t read_field(const Fixture &fixture, size_t offset) {
    uint32_t value;
    memcpy(&value,
           fixture.storage + runtime_oracle::CanarySize + offset,
           sizeof(value));
    return value;
}

template <typename Fixture>
void prepare_fixture(Fixture &fixture, uintptr_t *vtable, bool flat) {
    uint8_t *const object =
        fixture.storage + runtime_oracle::CanarySize;
    memset(object, 0, sizeof(*fixture.object()));
    memcpy(object, &vtable, sizeof(vtable));

    // Poison every field owned by the recovered closes while leaving all Win,
    // Buffer, and allocation resources empty. The two Time objects use an odd
    // mode and null event ID so close() cannot enter either OS timer path.
    for (size_t offset = 0xA14; offset < sizeof(BaseButton); offset += 4) {
        const uint32_t offset_bits = offset;
        write_field(fixture, offset,
                    0xA5000000U ^ offset_bits * 0x1021U);
    }
    write_field(fixture, 0xA1C, 1U);
    write_field(fixture, 0xA20, 0U);
    write_field(fixture, 0xA4C, 1U);
    write_field(fixture, 0xA50, 0U);
    write_field(fixture, 0xA7C, 0U);
    write_field(fixture, 0xA80, 0U);
    if (flat) {
        for (size_t offset = sizeof(BaseButton);
             offset < sizeof(FlatButton); offset += 4) {
            const uint32_t offset_bits = offset;
            write_field(fixture, offset,
                        0x5A000000U ^ offset_bits * 0x2101U);
        }
    }
}

template <typename Fixture>
bool equivalent(Fixture &legacy, Fixture &source,
                uint32_t legacy_result, uint32_t source_result,
                const runtime_oracle::Trace &legacy_trace,
                const runtime_oracle::Trace &source_trace) {
    const bool self_pointers_match =
        read_field(legacy, 0xA8)
            == reinterpret_cast<uintptr_t>(legacy.object())
        && read_field(source, 0xA8)
            == reinterpret_cast<uintptr_t>(source.object());
    write_field(legacy, 0xA8, 0U);
    write_field(source, 0xA8, 0U);

    const uintptr_t legacy_base =
        reinterpret_cast<uintptr_t>(legacy.object());
    const uintptr_t source_base =
        reinterpret_cast<uintptr_t>(source.object());
    if (legacy_result >= legacy_base
            && legacy_result < legacy_base + sizeof(*legacy.object())) {
        legacy_result -= legacy_base;
    }
    if (source_result >= source_base
            && source_result < source_base + sizeof(*source.object())) {
        source_result -= source_base;
    }
    return self_pointers_match
        && runtime_oracle::equivalent(
            legacy.storage, source.storage, sizeof(legacy.storage),
            legacy_result, source_result, legacy_trace, source_trace);
}

struct SavedDefaults {
    uint32_t base_fixed[5];
    uint32_t base_dynamic[2];
    uint32_t flat[27];
};

void seed_defaults(SavedDefaults &saved) {
    volatile uint32_t *const base_fixed = BaseButtonStaticDefaults;
    volatile uint32_t *const base_dynamic = BaseButtonDynamicDefaults;
    volatile uint32_t *const flat = FlatButtonDefaults;
    for (size_t index = 0; index < ARRAYSIZE(saved.base_fixed); ++index) {
        saved.base_fixed[index] = base_fixed[index];
        base_fixed[index] = 0x51000000U
            + static_cast<uint32_t>(index) * 0x010203U;
    }
    for (size_t index = 0; index < ARRAYSIZE(saved.base_dynamic); ++index) {
        saved.base_dynamic[index] = base_dynamic[index];
        base_dynamic[index] = 0xA1000000U
            + static_cast<uint32_t>(index) * 0x011011U;
    }
    for (size_t index = 0; index < ARRAYSIZE(saved.flat); ++index) {
        saved.flat[index] = flat[index];
        flat[index] = 0xD1000000U
            + static_cast<uint32_t>(index) * 0x010101U;
    }
}

void restore_defaults(const SavedDefaults &saved) {
    volatile uint32_t *const base_fixed = BaseButtonStaticDefaults;
    volatile uint32_t *const base_dynamic = BaseButtonDynamicDefaults;
    volatile uint32_t *const flat = FlatButtonDefaults;
    for (size_t index = 0; index < ARRAYSIZE(saved.base_fixed); ++index) {
        base_fixed[index] = saved.base_fixed[index];
    }
    for (size_t index = 0; index < ARRAYSIZE(saved.base_dynamic); ++index) {
        base_dynamic[index] = saved.base_dynamic[index];
    }
    for (size_t index = 0; index < ARRAYSIZE(saved.flat); ++index) {
        flat[index] = saved.flat[index];
    }
}

bool verify_base_close(uintptr_t *vtable) {
    BaseButtonFixture legacy;
    BaseButtonFixture source;
    runtime_oracle::initialize_pair(
        legacy.storage, source.storage, BaseButtonSpec, vtable);
    prepare_fixture(legacy, vtable, false);
    prepare_fixture(source, vtable, false);
    auto original = reinterpret_cast<OriginalBaseNoArg>(
        BaseButtonCloseAddress);

    runtime_oracle::begin_trace(
        legacy.object(), TraceOffsets, ARRAYSIZE(TraceOffsets));
    const uint32_t legacy_result = original(legacy.object());
    const runtime_oracle::Trace legacy_trace = runtime_oracle::current_trace();
    runtime_oracle::begin_trace(
        source.object(), TraceOffsets, ARRAYSIZE(TraceOffsets));
    const uint32_t source_result = source.object()->close();
    const runtime_oracle::Trace source_trace = runtime_oracle::current_trace();
    return equivalent(
        legacy, source, legacy_result, source_result,
        legacy_trace, source_trace);
}

bool verify_flat_close(uintptr_t *vtable) {
    FlatButtonFixture legacy;
    FlatButtonFixture source;
    runtime_oracle::initialize_pair(
        legacy.storage, source.storage, FlatButtonSpec, vtable);
    prepare_fixture(legacy, vtable, true);
    prepare_fixture(source, vtable, true);
    auto original = reinterpret_cast<OriginalFlatNoArg>(FlatButtonCloseAddress);

    runtime_oracle::begin_trace(
        legacy.object(), TraceOffsets, ARRAYSIZE(TraceOffsets));
    const uint32_t legacy_result = original(legacy.object());
    const runtime_oracle::Trace legacy_trace = runtime_oracle::current_trace();
    runtime_oracle::begin_trace(
        source.object(), TraceOffsets, ARRAYSIZE(TraceOffsets));
    const uint32_t source_result = source.object()->close();
    const runtime_oracle::Trace source_trace = runtime_oracle::current_trace();
    return equivalent(
        legacy, source, legacy_result, source_result,
        legacy_trace, source_trace);
}

bool verify_base_destructor(uintptr_t *vtable) {
    BaseButtonFixture legacy;
    BaseButtonFixture source;
    runtime_oracle::initialize_pair(
        legacy.storage, source.storage, BaseButtonSpec, vtable);
    prepare_fixture(legacy, vtable, false);
    prepare_fixture(source, vtable, false);
    auto original = reinterpret_cast<OriginalBaseNoArg>(
        BaseButtonDestructorAddress);

    runtime_oracle::begin_trace(
        legacy.object(), TraceOffsets, ARRAYSIZE(TraceOffsets));
    const uint32_t legacy_result = original(legacy.object());
    const runtime_oracle::Trace legacy_trace = runtime_oracle::current_trace();
    runtime_oracle::begin_trace(
        source.object(), TraceOffsets, ARRAYSIZE(TraceOffsets));
    const uint32_t source_result = reinterpret_cast<uintptr_t>(
        base_button_destructor_redirect(source.object(), nullptr));
    const runtime_oracle::Trace source_trace = runtime_oracle::current_trace();
    return equivalent(
        legacy, source, legacy_result, source_result,
        legacy_trace, source_trace);
}

bool verify_flat_destructor(uintptr_t *vtable) {
    FlatButtonFixture legacy;
    FlatButtonFixture source;
    runtime_oracle::initialize_pair(
        legacy.storage, source.storage, FlatButtonSpec, vtable);
    prepare_fixture(legacy, vtable, true);
    prepare_fixture(source, vtable, true);
    auto original = reinterpret_cast<OriginalFlatNoArg>(
        FlatButtonDestructorAddress);

    runtime_oracle::begin_trace(
        legacy.object(), TraceOffsets, ARRAYSIZE(TraceOffsets));
    const uint32_t legacy_result = original(legacy.object());
    const runtime_oracle::Trace legacy_trace = runtime_oracle::current_trace();
    runtime_oracle::begin_trace(
        source.object(), TraceOffsets, ARRAYSIZE(TraceOffsets));
    const uint32_t source_result = reinterpret_cast<uintptr_t>(
        flat_button_destructor_redirect(source.object(), nullptr));
    const runtime_oracle::Trace source_trace = runtime_oracle::current_trace();
    return equivalent(
        legacy, source, legacy_result, source_result,
        legacy_trace, source_trace);
}

}  // namespace

bool run_base_button_oracle_suite() {
    uintptr_t vtable[3] = {};
    vtable[2] = reinterpret_cast<uintptr_t>(&runtime_oracle::probe);
    runtime_oracle::set_watched_global(nullptr);
    SavedDefaults saved;
    seed_defaults(saved);
    const bool passed = verify_base_close(vtable)
        && verify_flat_close(vtable)
        && verify_base_destructor(vtable)
        && verify_flat_destructor(vtable);
    restore_defaults(saved);
    return passed;
}

bool run_base_button_release_suite() {
    if (!suspend_redirect_at(BaseButtonCloseAddress)) {
        return false;
    }
    uintptr_t vtable[3] = {};
    vtable[2] = reinterpret_cast<uintptr_t>(&runtime_oracle::probe);
    auto original = reinterpret_cast<OriginalBaseNoArg>(
        BaseButtonCloseAddress);
    SavedDefaults saved;
    seed_defaults(saved);
    bool passed = true;

    for (int allocation_mask = 1; allocation_mask < 4 && passed;
         ++allocation_mask) {
        BaseButtonFixture legacy;
        BaseButtonFixture source;
        runtime_oracle::initialize_pair(
            legacy.storage, source.storage, BaseButtonSpec, vtable);
        prepare_fixture(legacy, vtable, false);
        prepare_fixture(source, vtable, false);
        void *const legacy_name = allocation_mask & 1 ? _malloc(48) : nullptr;
        void *const source_name = allocation_mask & 1 ? _malloc(48) : nullptr;
        void *const legacy_bubble = allocation_mask & 2 ? _malloc(64) : nullptr;
        void *const source_bubble = allocation_mask & 2 ? _malloc(64) : nullptr;
        if (((allocation_mask & 1) && (!legacy_name || !source_name))
                || ((allocation_mask & 2)
                    && (!legacy_bubble || !source_bubble))) {
            if (legacy_name) {
                _free(legacy_name);
            }
            if (source_name) {
                _free(source_name);
            }
            if (legacy_bubble) {
                _free(legacy_bubble);
            }
            if (source_bubble) {
                _free(source_bubble);
            }
            passed = false;
            break;
        }
        write_field(legacy, 0xA7C,
                    reinterpret_cast<uintptr_t>(legacy_name));
        write_field(source, 0xA7C,
                    reinterpret_cast<uintptr_t>(source_name));
        write_field(legacy, 0xA80,
                    reinterpret_cast<uintptr_t>(legacy_bubble));
        write_field(source, 0xA80,
                    reinterpret_cast<uintptr_t>(source_bubble));

        runtime_oracle::begin_trace(
            legacy.object(), TraceOffsets, ARRAYSIZE(TraceOffsets));
        const uint32_t legacy_result = original(legacy.object());
        const runtime_oracle::Trace legacy_trace =
            runtime_oracle::current_trace();
        runtime_oracle::begin_trace(
            source.object(), TraceOffsets, ARRAYSIZE(TraceOffsets));
        const uint32_t source_result = source.object()->close();
        const runtime_oracle::Trace source_trace =
            runtime_oracle::current_trace();
        passed = equivalent(
            legacy, source, legacy_result, source_result,
            legacy_trace, source_trace);
    }
    restore_defaults(saved);
    if (!resume_redirect_at(BaseButtonCloseAddress)) {
        return false;
    }
    return passed;
}
