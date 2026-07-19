/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */
#pragma once

#include <stdint.h>

/*
 * Shared in-process differential oracle machinery. Suites run untouched
 * original functions at their canonical addresses inside the verified hybrid
 * process before any redirect is installed and compare full object state,
 * virtual-callback traces, watched globals, and return residues against the
 * source replacements. Each class family contributes one suite; the smoke
 * gate refuses to pass unless every registered suite reports success.
 */
namespace runtime_oracle {

constexpr size_t CanarySize = 16;
constexpr size_t MaximumCalls = 2;
constexpr size_t MaximumSnapshots = 8;

struct Trace {
    uint32_t calls;
    uint32_t self_offsets[MaximumCalls];
    uint32_t snapshots[MaximumCalls][MaximumSnapshots];
    uint32_t watched_values[MaximumCalls];
};

// Canary-guarded storage for one object under test.
template <typename T>
struct Fixture {
    alignas(T) uint8_t storage[sizeof(T) + CanarySize * 2];

    T *object() {
        return reinterpret_cast<T *>(storage + CanarySize);
    }
};

// Descriptor for one class family's fixture construction.
struct ClassSpec {
    size_t object_size;
    size_t vtable_bytes;               // legacy vtable size in bytes
    size_t probe_slot;                 // byte offset of the probed virtual slot
    const size_t *vtable_ref_offsets;  // object offsets holding vtable copies
    size_t vtable_ref_count;
};

// Fill both fixtures with identical canaries and a deterministic byte
// pattern, then install a zeroed vtable whose probe slot traces callbacks.
void initialize_pair(uint8_t *legacy_storage, uint8_t *source_storage,
                     const ClassSpec &spec, uintptr_t *vtable);

// Register the global pointer sampled by the probe on every callback.
void set_watched_global(void **watched);

void begin_trace(void *object, const size_t *offsets, size_t offset_count);
const Trace &current_trace();

uint32_t __fastcall probe(void *self, void *);

bool equivalent(const uint8_t *legacy_storage, const uint8_t *source_storage,
                size_t storage_size, uint32_t legacy_result,
                uint32_t source_result, const Trace &legacy_trace,
                const Trace &source_trace);

struct Suite {
    const char *name;
    bool (*run)();
};

}  // namespace runtime_oracle

// Runs every registered suite when OPENSMACX_RUNTIME_ORACLE_RESULT names a
// result path, writing one "<suite> passed|failed" line per suite and a
// terminal "all passed" line only when every suite succeeded.
bool run_runtime_oracles();

// Lift and reinstall a single installed redirect so a deferred suite can call
// the original body. Defined in src/dllmain.cpp.
bool suspend_redirect_at(uintptr_t address);
bool resume_redirect_at(uintptr_t address);

// Second oracle phase, triggered from a startup call-site hook once the
// executable's CRT is running so release paths can be exercised against real
// allocations. Rewrites the result file to include its own suite lines.
void run_deferred_oracles();
