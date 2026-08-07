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
#include "constructor_oracle.h"

#include "autosound.h"
#include "basebutton.h"
#include "buffer.h"
#include "font.h"
#include "graphicwin.h"
#include "palette.h"
#include "runtime_oracle.h"
#include "win.h"

// What this oracle PROVES: each function below is executed here in its
// ORIGINAL form, at its canonical address inside the hybrid process, and
// compared against the recovered implementation. These markers are the
// input to `unproven_recovered` in docs/recovery/summary.json; see
// tools/export_proven_functions.py for what does and does not count.
//
// PROVEN-AGAINST-ORIGINAL: 0x005D4CF0  ??0GraphicWin@@QAE@XZ
// PROVEN-AGAINST-ORIGINAL: 0x005D7210  ??0Buffer@@QAE@XZ
// PROVEN-AGAINST-ORIGINAL: 0x005EB3D0  ??0Win@@QAE@XZ
// PROVEN-AGAINST-ORIGINAL: 0x00606F30  ??0BaseButton@@QAE@XZ
// PROVEN-AGAINST-ORIGINAL: 0x0062BA80  ??0AutoSound@@QAE@XZ


namespace {

typedef uint32_t (OriginalObject::*OriginalConstructor)();
typedef uint32_t (OriginalObject::*OriginalPaletteGet)(RGBQUAD *, int, int);

constexpr uintptr_t AutoSoundConstructorAddress = 0x0062BA80U;
constexpr uintptr_t WinConstructorAddress = 0x005EB3D0U;
constexpr uintptr_t PaletteGetAddress = 0x005FE560U;
constexpr uintptr_t BufferConstructorAddress = 0x005D7210U;
constexpr uintptr_t GraphicWinConstructorAddress = 0x005D4CF0U;
constexpr uintptr_t BaseButtonConstructorAddress = 0x00606F30U;

template <typename T>
void initialize(runtime_oracle::Fixture<T> &legacy,
                runtime_oracle::Fixture<T> &source) {
    const runtime_oracle::ClassSpec spec = {
        sizeof(T), sizeof(uintptr_t), 0, nullptr, 0};
    uintptr_t scratch_vtable[1];
    runtime_oracle::initialize_pair(
        legacy.storage, source.storage, spec, scratch_vtable);
}

template <typename T>
bool equivalent_constructor(
        runtime_oracle::Fixture<T> &legacy,
        runtime_oracle::Fixture<T> &source,
        uint32_t legacy_result, uint32_t source_result,
        int self_pointer_offset = -1) {
    if (legacy_result - reinterpret_cast<uintptr_t>(legacy.object())
            != source_result - reinterpret_cast<uintptr_t>(source.object())) {
        return false;
    }
    if (self_pointer_offset >= 0) {
        uint32_t legacy_self;
        uint32_t source_self;
        memcpy(&legacy_self,
               legacy.storage + runtime_oracle::CanarySize
                   + self_pointer_offset,
               sizeof(legacy_self));
        memcpy(&source_self,
               source.storage + runtime_oracle::CanarySize
                   + self_pointer_offset,
               sizeof(source_self));
        if (legacy_self != reinterpret_cast<uintptr_t>(legacy.object())
                || source_self != reinterpret_cast<uintptr_t>(source.object())) {
            return false;
        }
        const uint32_t normalized = 0x13579BDFU;
        memcpy(legacy.storage + runtime_oracle::CanarySize
                   + self_pointer_offset,
               &normalized, sizeof(normalized));
        memcpy(source.storage + runtime_oracle::CanarySize
                   + self_pointer_offset,
               &normalized, sizeof(normalized));
    }
    return memcmp(legacy.storage, source.storage,
                  sizeof(legacy.storage)) == 0;
}

struct SavedDefaults {
    uint32_t sound[37];
    uint32_t win_fixed[9];
    uint32_t win_dynamic[4];
    uint32_t buffer_reset;
    Font *font;
    Palette *palette;
    int palette_initialized;
    uint32_t graphic;
    uint32_t button_fixed[5];
    uint32_t button_dynamic[2];
};

void save_defaults(SavedDefaults &saved) {
    memcpy(saved.sound, AutoSoundDefaults, sizeof(saved.sound));
    memcpy(saved.win_fixed, WinStaticDefaults, sizeof(saved.win_fixed));
    memcpy(saved.win_dynamic, WinDynamicDefaults, sizeof(saved.win_dynamic));
    saved.buffer_reset = *BufferResetValue520;
    saved.font = *FontDefaultPtr;
    saved.palette = *BufferPalette;
    saved.palette_initialized = *PaletteInitialized;
    saved.graphic = *GraphicWinFieldA0CDefault;
    memcpy(saved.button_fixed, BaseButtonStaticDefaults,
           sizeof(saved.button_fixed));
    memcpy(saved.button_dynamic, BaseButtonDynamicDefaults,
           sizeof(saved.button_dynamic));
}

void restore_defaults(const SavedDefaults &saved) {
    memcpy(AutoSoundDefaults, saved.sound, sizeof(saved.sound));
    memcpy(WinStaticDefaults, saved.win_fixed, sizeof(saved.win_fixed));
    memcpy(WinDynamicDefaults, saved.win_dynamic, sizeof(saved.win_dynamic));
    *BufferResetValue520 = saved.buffer_reset;
    *FontDefaultPtr = saved.font;
    *BufferPalette = saved.palette;
    *PaletteInitialized = saved.palette_initialized;
    *GraphicWinFieldA0CDefault = saved.graphic;
    memcpy(BaseButtonStaticDefaults, saved.button_fixed,
           sizeof(saved.button_fixed));
    memcpy(BaseButtonDynamicDefaults, saved.button_dynamic,
           sizeof(saved.button_dynamic));
}

void seed_defaults(Palette *palette, int initialized) {
    for (size_t index = 0; index < 37; ++index) {
        AutoSoundDefaults[index] =
            0x51000000U + index * 0x10101U;
    }
    for (size_t index = 0; index < 9; ++index) {
        WinStaticDefaults[index] =
            0x52000000U + index * 0x11011U;
    }
    for (size_t index = 0; index < 4; ++index) {
        WinDynamicDefaults[index] =
            0x53000000U + index * 0x11111U;
    }
    *BufferResetValue520 = 0x54123456U;
    *FontDefaultPtr = reinterpret_cast<Font *>(0x55234567U);
    *BufferPalette = palette;
    *PaletteInitialized = initialized;
    *GraphicWinFieldA0CDefault = 0x56345678U;
    for (size_t index = 0; index < 5; ++index) {
        BaseButtonStaticDefaults[index] =
            0x57000000U + index * 0x10203U;
    }
    for (size_t index = 0; index < 2; ++index) {
        BaseButtonDynamicDefaults[index] =
            0x58000000U + index * 0x20304U;
    }
}

template <typename T>
bool verify_constructor(
        uintptr_t address,
        T *(__fastcall *source_construct)(T *, void *),
        int self_pointer_offset = -1) {
    runtime_oracle::Fixture<T> legacy;
    runtime_oracle::Fixture<T> source;
    initialize(legacy, source);
    const uint32_t legacy_result =
        (ORIGINAL(legacy.object())->*original_method<OriginalConstructor>(reinterpret_cast<unsigned long>(address)))();
    const uint32_t source_result = reinterpret_cast<uintptr_t>(
        source_construct(source.object(), nullptr));
    return equivalent_constructor(
        legacy, source, legacy_result, source_result, self_pointer_offset);
}

bool verify_palette() {
    OriginalPaletteGet original = reinterpret_cast<OriginalPaletteGet>(PaletteGetAddress);
    bool passed = true;
    for (int initialized : {0, 1}) {
        *PaletteInitialized = initialized;
        runtime_oracle::Fixture<Palette> legacy;
        runtime_oracle::Fixture<Palette> source;
        initialize(legacy, source);
        alignas(RGBQUAD) uint8_t legacy_output[64];
        alignas(RGBQUAD) uint8_t source_output[64];
        memset(legacy_output, 0xA5, sizeof(legacy_output));
        memcpy(source_output, legacy_output, sizeof(source_output));
        RGBQUAD *const legacy_quads =
            reinterpret_cast<RGBQUAD *>(legacy_output + 16);
        RGBQUAD *const source_quads =
            reinterpret_cast<RGBQUAD *>(source_output + 16);
        const uint32_t legacy_result =
            original(legacy.object(), legacy_quads, 3, 5);
        const uint32_t source_result = palette_get_rgbquad_redirect(
            source.object(), nullptr, source_quads, 3, 5);
        passed = passed
            && legacy_result == source_result
            && memcmp(legacy.storage, source.storage,
                      sizeof(legacy.storage)) == 0
            && memcmp(legacy_output, source_output,
                      sizeof(legacy_output)) == 0;
    }

    *PaletteInitialized = 1;
    runtime_oracle::Fixture<Palette> legacy;
    runtime_oracle::Fixture<Palette> source;
    initialize(legacy, source);
    RGBQUAD *const legacy_alias =
        reinterpret_cast<RGBQUAD *>(legacy.object());
    RGBQUAD *const source_alias =
        reinterpret_cast<RGBQUAD *>(source.object());
    const uint32_t legacy_result =
        original(legacy.object(), legacy_alias, 0, 4);
    const uint32_t source_result = palette_get_rgbquad_redirect(
        source.object(), nullptr, source_alias, 0, 4);
    return passed
        && legacy_result == source_result
        && memcmp(legacy.storage, source.storage,
                  sizeof(legacy.storage)) == 0;
}

bool verify_buffer_cases(Palette *palette) {
    bool passed = true;
    struct Case { Palette *palette; int initialized; };
    const Case cases[] = {{nullptr, 0}, {palette, 0}, {palette, 1}};
    for (const Case &test : cases) {
        *BufferPalette = test.palette;
        *PaletteInitialized = test.initialized;
        passed = passed && verify_constructor<Buffer>(
            BufferConstructorAddress, &buffer_construct_redirect);
    }
    return passed;
}

}  // namespace

bool run_constructor_oracle_suite() {
    SavedDefaults saved;
    save_defaults(saved);

    alignas(Palette) uint8_t palette_storage[sizeof(Palette)];
    for (size_t index = 0; index < sizeof(palette_storage); ++index) {
        palette_storage[index] = static_cast<uint8_t>(0x31U + index * 13U);
    }
    Palette *const palette = reinterpret_cast<Palette *>(palette_storage);
    seed_defaults(palette, 1);

    bool passed = verify_constructor<AutoSound>(
        AutoSoundConstructorAddress, &auto_sound_construct_redirect);
    passed = passed && verify_constructor<Win>(
        WinConstructorAddress, &win_construct_redirect, 0x0A8);
    passed = passed && verify_palette();
    passed = passed && verify_buffer_cases(palette);
    *BufferPalette = palette;
    *PaletteInitialized = 1;
    passed = passed && verify_constructor<GraphicWin>(
        GraphicWinConstructorAddress, &graphic_win_construct_redirect, 0x0A8);
    passed = passed && verify_constructor<BaseButton>(
        BaseButtonConstructorAddress, &base_button_construct_redirect, 0x0A8);

    restore_defaults(saved);
    return passed;
}
