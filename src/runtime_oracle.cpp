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
#include "runtime_oracle.h"

#include "buffer_oracle.h"
#include "graphicwin_oracle.h"
#include "scroll_oracle.h"
#include "stringstruct_oracle.h"
#include "sprite_oracle.h"
#include "spying_recovery.h"
#include "win_oracle.h"

namespace {

constexpr char ResultEnvironment[] = "OPENSMACX_RUNTIME_ORACLE_RESULT";

// Retained from the first phase so the deferred phase can rewrite the same
// file with its own suite lines appended.
char ResultPath[MAX_PATH] = "";
char PhaseOneReport[1024] = "";

struct ProbeContext {
    uint8_t *base;
    size_t offsets[runtime_oracle::MaximumSnapshots];
    size_t offset_count;
    void **watched;
    runtime_oracle::Trace trace;
};

ProbeContext Context = {};
void **WatchedGlobal = nullptr;

const runtime_oracle::Suite Suites[] = {
    {"scroll", run_scroll_oracle_suite},
    {"win", run_win_oracle_suite},
    {"sprite", run_sprite_oracle_suite},
    {"buffer", run_buffer_oracle_suite},
    {"graphicwin", run_graphic_win_oracle_suite},
    {"stringstruct", run_string_struct_oracle_suite},
    {"spying", run_spying_oracle_suite},
};

bool write_result(const char *path, const char *result) {
    HANDLE file = CreateFileA(path, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
                              FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE) {
        return false;
    }
    const DWORD size = static_cast<DWORD>(strlen(result));
    DWORD written = 0;
    const BOOL write_ok = WriteFile(file, result, size, &written, nullptr);
    const BOOL flush_ok = FlushFileBuffers(file);
    const BOOL close_ok = CloseHandle(file);
    return write_ok && flush_ok && close_ok && written == size;
}

}  // namespace

namespace runtime_oracle {

void initialize_pair(uint8_t *legacy_storage, uint8_t *source_storage,
                     const ClassSpec &spec, uintptr_t *vtable) {
    const size_t storage_size = spec.object_size + CanarySize * 2;
    memset(legacy_storage, 0xA5, storage_size);
    for (size_t offset = 0; offset < spec.object_size; ++offset) {
        legacy_storage[CanarySize + offset] =
            static_cast<uint8_t>(0x35U + offset * 17U);
    }
    const size_t vtable_entries = spec.vtable_bytes / sizeof(uintptr_t);
    memset(vtable, 0, sizeof(uintptr_t) * vtable_entries);
    vtable[spec.probe_slot / sizeof(uintptr_t)] =
        reinterpret_cast<uintptr_t>(&probe);
    for (size_t index = 0; index < spec.vtable_ref_count; ++index) {
        memcpy(legacy_storage + CanarySize + spec.vtable_ref_offsets[index],
               &vtable, sizeof(uintptr_t *));
    }
    memcpy(source_storage, legacy_storage, storage_size);
}

void set_watched_global(void **watched) {
    WatchedGlobal = watched;
}

void begin_trace(void *object, const size_t *offsets, size_t offset_count) {
    memset(&Context, 0, sizeof(Context));
    Context.base = reinterpret_cast<uint8_t *>(object);
    Context.offset_count = offset_count;
    for (size_t index = 0; index < offset_count; ++index) {
        Context.offsets[index] = offsets[index];
    }
    Context.watched = WatchedGlobal;
}

const Trace &current_trace() {
    return Context.trace;
}

uint32_t __fastcall probe(void *self, void *) {
    const uint32_t call = Context.trace.calls++;
    if (call >= MaximumCalls) {
        return 0xDEAD0000U | call;
    }
    Context.trace.self_offsets[call] = static_cast<uint32_t>(
        reinterpret_cast<uint8_t *>(self) - Context.base);
    for (size_t index = 0; index < Context.offset_count; ++index) {
        memcpy(&Context.trace.snapshots[call][index],
               Context.base + Context.offsets[index], sizeof(uint32_t));
    }
    Context.trace.watched_values[call] = Context.watched
        ? reinterpret_cast<uintptr_t>(*Context.watched)
        : 0;
    return 0x13579BDFU ^ (call * 0x3174A10DU);
}

bool equivalent(const uint8_t *legacy_storage, const uint8_t *source_storage,
                size_t storage_size, uint32_t legacy_result,
                uint32_t source_result, const Trace &legacy_trace,
                const Trace &source_trace) {
    return legacy_result == source_result
        && memcmp(legacy_storage, source_storage, storage_size) == 0
        && memcmp(&legacy_trace, &source_trace, sizeof(legacy_trace)) == 0;
}

}  // namespace runtime_oracle

bool run_runtime_oracles() {
    const DWORD length = GetEnvironmentVariableA(
        ResultEnvironment, ResultPath, ARRAYSIZE(ResultPath));
    if (length == 0) {
        return true;
    }
    if (length >= ARRAYSIZE(ResultPath)) {
        ResultPath[0] = '\0';
        return false;
    }
    char report[1024] = "";
    size_t used = 0;
    bool all_passed = true;
    for (const runtime_oracle::Suite &suite : Suites) {
        const bool suite_passed = suite.run();
        all_passed = all_passed && suite_passed;
        const int written = _snprintf_s(
            report + used, sizeof(report) - used, _TRUNCATE, "%s %s\n",
            suite.name, suite_passed ? "passed" : "failed");
        if (written < 0) {
            return false;
        }
        used += static_cast<size_t>(written);
    }
    if (all_passed) {
        const int written = _snprintf_s(
            report + used, sizeof(report) - used, _TRUNCATE, "all passed\n");
        if (written < 0) {
            return false;
        }
    }
    memcpy(PhaseOneReport, report, sizeof(report));
    const bool result_written = write_result(ResultPath, report);
    return all_passed && result_written;
}

namespace {

const runtime_oracle::Suite DeferredSuites[] = {
    {"sprite-release", run_sprite_release_suite},
    {"buffer-release", run_buffer_release_suite},
};

bool DeferredCompleted = false;

}  // namespace

void run_deferred_oracles() {
    // One shot: the trigger sits on a startup call site that can fire more
    // than once, and lifting a redirect twice would be unsafe.
    if (DeferredCompleted || ResultPath[0] == '\0') {
        return;
    }
    DeferredCompleted = true;

    char report[1024] = "";
    size_t used = 0;
    bool all_passed = true;
    // Carry phase one's lines forward minus the terminator so the file stays a
    // single record; if this phase never runs the earlier record still stands.
    for (const char *cursor = PhaseOneReport; *cursor; ++cursor) {
        if (strncmp(cursor, "all passed\n", 11) == 0) {
            break;
        }
        if (used + 2 >= sizeof(report)) {
            return;
        }
        report[used++] = *cursor;
    }
    for (const runtime_oracle::Suite &suite : DeferredSuites) {
        const bool suite_passed = suite.run();
        all_passed = all_passed && suite_passed;
        const int written = _snprintf_s(
            report + used, sizeof(report) - used, _TRUNCATE, "%s %s\n",
            suite.name, suite_passed ? "passed" : "failed");
        if (written < 0) {
            return;
        }
        used += static_cast<size_t>(written);
    }
    if (all_passed) {
        _snprintf_s(report + used, sizeof(report) - used, _TRUNCATE,
                    "all passed\n");
    }
    write_result(ResultPath, report);
}
