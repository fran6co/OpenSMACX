/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenSMACX is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenSMACX. If not, see <http://www.gnu.org/licenses/>.
 */
#include "stdafx.h"
#include "alphanet.h"
#include "autosound.h"
#include "base.h"
#include "basepop.h"
#include "buffer.h"
#include "buttongroup.h"
#include "caviar.h"
#include "dialog.h"
#include "dialogs.h"
#include "maininterface.h"
#include "scenario.h"
#include "win.h"

#include <cstdint>
#include <cstring>

namespace {

constexpr size_t PatchSize = 5;
constexpr size_t SignatureSize = 16;
constexpr size_t RedirectCount = 30;
constexpr size_t CallRedirectCount = 2;

struct RedirectState {
    uint8_t *address;
    uint8_t original[PatchSize];
    DWORD original_protection;
    bool installed;
};

struct RedirectSpec {
    uintptr_t original_address;
    uintptr_t replacement_address;
    uint8_t signature[SignatureSize];
};

RedirectState Redirects[RedirectCount] = {};
RedirectState CallRedirects[CallRedirectCount] = {};

bool restore_redirect(RedirectState &state) {
    if (!state.installed) {
        return true;
    }
    DWORD writable_protection;
    if (!VirtualProtect(
            state.address, PatchSize, PAGE_EXECUTE_READWRITE, &writable_protection)) {
        return false;
    }
    memcpy(state.address, state.original, PatchSize);
    BOOL flushed = FlushInstructionCache(GetCurrentProcess(), state.address, PatchSize);
    DWORD ignored_protection;
    BOOL protected_ = VirtualProtect(
        state.address, PatchSize, state.original_protection, &ignored_protection);
    if (flushed && protected_) {
        state.installed = false;
        return true;
    }
    return false;
}

bool redirect_function(RedirectState &state, uintptr_t original_address,
                       uintptr_t replacement_address,
                       const uint8_t expected_signature[SignatureSize]) {
    static_assert(sizeof(uintptr_t) == sizeof(uint32_t), "runtime redirects require PE32");
    auto *original = reinterpret_cast<uint8_t *>(original_address);
    MEMORY_BASIC_INFORMATION memory_info;
    DWORD basic_protection;
    if (VirtualQuery(original, &memory_info, sizeof(memory_info)) != sizeof(memory_info)
        || memory_info.State != MEM_COMMIT
        || (memory_info.Protect & PAGE_GUARD) != 0) {
        return false;
    }
    basic_protection = memory_info.Protect & 0xFF;
    if (basic_protection != PAGE_READONLY && basic_protection != PAGE_READWRITE
        && basic_protection != PAGE_WRITECOPY && basic_protection != PAGE_EXECUTE_READ
        && basic_protection != PAGE_EXECUTE_READWRITE
        && basic_protection != PAGE_EXECUTE_WRITECOPY) {
        return false;
    }
    uintptr_t region_end = reinterpret_cast<uintptr_t>(memory_info.BaseAddress)
        + memory_info.RegionSize;
    if (original_address > region_end || SignatureSize > region_end - original_address
        || memcmp(original, expected_signature, SignatureSize) != 0) {
        return false;
    }

    uint8_t patch[PatchSize] = {0xE9, 0, 0, 0, 0};
    uint32_t relative_jump = replacement_address - (original_address + PatchSize);
    memcpy(&patch[1], &relative_jump, sizeof(relative_jump));

    state.address = original;
    memcpy(state.original, original, PatchSize);
    if (!VirtualProtect(
            original, PatchSize, PAGE_EXECUTE_READWRITE, &state.original_protection)) {
        return false;
    }
    memcpy(original, patch, PatchSize);
    state.installed = true;
    BOOL flushed = FlushInstructionCache(GetCurrentProcess(), original, PatchSize);
    DWORD ignored_protection;
    BOOL protected_ = VirtualProtect(
        original, PatchSize, state.original_protection, &ignored_protection);
    if (flushed && protected_) {
        return true;
    }
    if (!restore_redirect(state)) {
        TerminateProcess(GetCurrentProcess(), ERROR_INVALID_STATE);
    }
    return false;
}

bool redirect_call(RedirectState &state, uintptr_t call_address,
                   uintptr_t replacement_address,
                   const uint8_t expected_signature[SignatureSize]) {
    static_assert(sizeof(uintptr_t) == sizeof(uint32_t), "runtime redirects require PE32");
    auto *original = reinterpret_cast<uint8_t *>(call_address);
    MEMORY_BASIC_INFORMATION memory_info;
    if (VirtualQuery(original, &memory_info, sizeof(memory_info)) != sizeof(memory_info)
        || memory_info.State != MEM_COMMIT || (memory_info.Protect & PAGE_GUARD) != 0) {
        return false;
    }
    uintptr_t region_end = reinterpret_cast<uintptr_t>(memory_info.BaseAddress)
        + memory_info.RegionSize;
    if (call_address > region_end || SignatureSize > region_end - call_address
        || memcmp(original, expected_signature, SignatureSize) != 0) {
        return false;
    }

    uint8_t patch[PatchSize] = {0xE8, 0, 0, 0, 0};
    uint32_t relative_call = replacement_address - (call_address + PatchSize);
    memcpy(&patch[1], &relative_call, sizeof(relative_call));

    state.address = original;
    memcpy(state.original, original, PatchSize);
    if (!VirtualProtect(
            original, PatchSize, PAGE_EXECUTE_READWRITE, &state.original_protection)) {
        return false;
    }
    memcpy(original, patch, PatchSize);
    state.installed = true;
    BOOL flushed = FlushInstructionCache(GetCurrentProcess(), original, PatchSize);
    DWORD ignored_protection;
    BOOL protected_ = VirtualProtect(
        original, PatchSize, state.original_protection, &ignored_protection);
    if (flushed && protected_) {
        return true;
    }
    if (!restore_redirect(state)) {
        TerminateProcess(GetCurrentProcess(), ERROR_INVALID_STATE);
    }
    return false;
}

bool install_redirects() {
    const RedirectSpec specs[RedirectCount] = {
        {
            0x004E3A50,
            reinterpret_cast<uintptr_t>(&base_at),
            {0x55, 0x8B, 0xEC, 0x53, 0x56, 0x57, 0x8B, 0x7D,
             0x0C, 0x85, 0xFF, 0x0F, 0x8C, 0x16, 0x01, 0x00},
        },
        {
            0x005DAC70,
            reinterpret_cast<uintptr_t>(&buffer_set_font_redirect),
            {0x8B, 0x44, 0x24, 0x04, 0x85, 0xC0, 0x75, 0x08,
             0xB8, 0x03, 0x00, 0x00, 0x00, 0xC2, 0x10, 0x00},
        },
        {
            0x005DACB0,
            reinterpret_cast<uintptr_t>(&buffer_set_text_color_redirect),
            {0x8B, 0x44, 0x24, 0x04, 0x8B, 0x54, 0x24, 0x08,
             0x89, 0x81, 0x3C, 0x05, 0x00, 0x00, 0x8B, 0x44},
        },
        {
            0x005DACE0,
            reinterpret_cast<uintptr_t>(&buffer_set_text_color2_redirect),
            {0x8B, 0x44, 0x24, 0x04, 0x8B, 0x54, 0x24, 0x08,
             0x89, 0x81, 0x40, 0x05, 0x00, 0x00, 0x8B, 0x44},
        },
        {
            0x005DAD10,
            reinterpret_cast<uintptr_t>(&buffer_set_text_color3_redirect),
            {0x8B, 0x44, 0x24, 0x04, 0x8B, 0x54, 0x24, 0x08,
             0x89, 0x81, 0x44, 0x05, 0x00, 0x00, 0x8B, 0x44},
        },
        {
            0x005DAD40,
            reinterpret_cast<uintptr_t>(&buffer_set_text_color_hyper_redirect),
            {0x8B, 0x44, 0x24, 0x04, 0x8B, 0x54, 0x24, 0x08,
             0x89, 0x81, 0x48, 0x05, 0x00, 0x00, 0x8B, 0x44},
        },
        {
            0x00609C60,
            reinterpret_cast<uintptr_t>(&dialog_set_font_redirect),
            {0x8B, 0x44, 0x24, 0x04, 0x85, 0xC0, 0x75, 0x08,
             0xB8, 0x03, 0x00, 0x00, 0x00, 0xC2, 0x0C, 0x00},
        },
        {
            0x00609C90,
            reinterpret_cast<uintptr_t>(&dialog_set_text_color_redirect),
            {0x8B, 0x44, 0x24, 0x04, 0x8B, 0x54, 0x24, 0x08,
             0x89, 0x41, 0x7C, 0x8B, 0x44, 0x24, 0x0C, 0x89},
        },
        {
            0x00609CC0,
            reinterpret_cast<uintptr_t>(&dialog_set_text_color2_redirect),
            {0x8B, 0x44, 0x24, 0x04, 0x8B, 0x54, 0x24, 0x08,
             0x89, 0x81, 0x80, 0x00, 0x00, 0x00, 0x8B, 0x44},
        },
        {
            0x00609CF0,
            reinterpret_cast<uintptr_t>(&dialog_set_text_color3_redirect),
            {0x8B, 0x44, 0x24, 0x04, 0x8B, 0x54, 0x24, 0x08,
             0x89, 0x81, 0x84, 0x00, 0x00, 0x00, 0x8B, 0x44},
        },
        {
            0x005FA7E0,
            reinterpret_cast<uintptr_t>(&in_box),
            {0x8B, 0x44, 0x24, 0x0C, 0x8B, 0x4C, 0x24, 0x04,
             0x3B, 0x08, 0x7D, 0x03, 0x33, 0xC0, 0xC3, 0x3B},
        },
        {
            0x005FD2B0,
            reinterpret_cast<uintptr_t>(&do_sound_redirect),
            {0x33, 0xC0, 0xC3, 0x90, 0x90, 0x90, 0x90, 0x90,
             0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90},
        },
        {
            0x00601B80,
            reinterpret_cast<uintptr_t>(&base_pop_set_loc_redirect),
            {0x8B, 0x44, 0x24, 0x04, 0x3D, 0x00, 0x20, 0x00,
             0x00, 0x74, 0x06, 0x89, 0x81, 0xF8, 0x30, 0x00},
        },
        {
            0x0062B7C0,
            reinterpret_cast<uintptr_t>(&button_group_construct_redirect),
            {0x8B, 0xD1, 0x57, 0x33, 0xC0, 0xB9, 0x20, 0x00,
             0x00, 0x00, 0x8B, 0xFA, 0x89, 0x82, 0x80, 0x00},
        },
        {
            0x0062B7F0,
            reinterpret_cast<uintptr_t>(&button_group_close_redirect),
            {0x57, 0x8B, 0xF9, 0x33, 0xC0, 0xB9, 0x20, 0x00,
             0x00, 0x00, 0x89, 0x87, 0x80, 0x00, 0x00, 0x00},
        },
        {
            0x0062B810,
            reinterpret_cast<uintptr_t>(&button_group_init_redirect),
            {0x8B, 0xD1, 0x57, 0x33, 0xC0, 0xB9, 0x20, 0x00,
             0x00, 0x00, 0x8B, 0xFA, 0x89, 0x82, 0x80, 0x00},
        },
        {
            0x0062B850,
            reinterpret_cast<uintptr_t>(&button_group_add_redirect),
            {0x8B, 0x91, 0x80, 0x00, 0x00, 0x00, 0x8B, 0x44,
             0x24, 0x04, 0x89, 0x04, 0x91, 0x89, 0x88, 0xA8},
        },
        {
            0x00616BC0,
            reinterpret_cast<uintptr_t>(&caviar_data_construct_redirect),
            {0x8B, 0xC1, 0x33, 0xC9, 0x89, 0x08, 0x89, 0x48,
             0x04, 0x89, 0x48, 0x08, 0xC3, 0x90, 0x90, 0x90},
        },
        {
            0x00616DA0,
            reinterpret_cast<uintptr_t>(&caviar_construct_redirect),
            {0x8B, 0xC1, 0x56, 0xBE, 0xC8, 0x00, 0x00, 0x00,
             0x33, 0xC9, 0x8D, 0x90, 0x50, 0x07, 0x00, 0x00},
        },
        {
            0x006183B0,
            reinterpret_cast<uintptr_t>(&caviar_set_scaling_redirect),
            {0x8B, 0x44, 0x24, 0x04, 0x89, 0x81, 0xD5, 0x00,
             0x00, 0x00, 0xC2, 0x04, 0x00, 0x90, 0x90, 0x90},
        },
        {
            0x006182A0,
            reinterpret_cast<uintptr_t>(&caviar_set_camera_direct_redirect),
            {0x8B, 0x54, 0x24, 0x04, 0x85, 0xD2, 0x74, 0x74,
             0x8B, 0x44, 0x24, 0x08, 0x85, 0xC0, 0x74, 0x6C},
        },
        {
            0x006183C0,
            reinterpret_cast<uintptr_t>(&caviar_get_scaling_redirect),
            {0xD9, 0x81, 0xD5, 0x00, 0x00, 0x00, 0xC3, 0x90,
             0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90},
        },
        {
            0x0045C170,
            reinterpret_cast<uintptr_t>(&main_interface_clear_message_redirect),
            {0xC3, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
             0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90},
        },
        {
            0x0058EE50,
            reinterpret_cast<uintptr_t>(&desktop_update),
            {0xC3, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
             0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90},
        },
        {
            0x005DF570,
            reinterpret_cast<uintptr_t>(&buffer_init_class_redirect),
            {0x33, 0xC0, 0xC3, 0x90, 0x90, 0x90, 0x90, 0x90,
             0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90},
        },
        {
            0x005DF580,
            reinterpret_cast<uintptr_t>(&buffer_close_class_redirect),
            {0xC3, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
             0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90},
        },
        {
            0x00612960,
            reinterpret_cast<uintptr_t>(&dialogs_close_redirect),
            {0xC3, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
             0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90},
        },
        {
            0x004E25E0,
            reinterpret_cast<uintptr_t>(&alpha_net_pid_to_idx_redirect),
            {0x55, 0x8B, 0xEC, 0x8B, 0x55, 0x08, 0xB8, 0x01,
              0x00, 0x00, 0x00, 0x81, 0xC1, 0x28, 0x09, 0x00},
        },
        {
            0x00514189,
            reinterpret_cast<uintptr_t>(&scenario_human_turn_trampoline),
            {0x89, 0xBE, 0x24, 0x3D, 0x02, 0x00, 0x39, 0x3D,
             0x70, 0x20, 0x9B, 0x00, 0x0F, 0x84, 0xB4, 0x00},
        },
        {
            0x00525AF4,
            reinterpret_cast<uintptr_t>(&scenario_turn_advance_trampoline),
            {0xE8, 0x17, 0xA0, 0xF4, 0xFF, 0xA1, 0xD8, 0x67,
             0x9A, 0x00, 0x3B, 0xC3, 0x0F, 0x84, 0xA9, 0x00},
        },
    };
    for (size_t index = 0; index < RedirectCount; index++) {
        if (redirect_function(
                Redirects[index], specs[index].original_address,
                specs[index].replacement_address, specs[index].signature)) {
            continue;
        }
        while (index > 0) {
            index--;
            if (!restore_redirect(Redirects[index])) {
                TerminateProcess(GetCurrentProcess(), ERROR_INVALID_STATE);
            }
        }
        return false;
    }

    const RedirectSpec call_specs[CallRedirectCount] = {
        {
            0x0052AB6D,
            reinterpret_cast<uintptr_t>(&scenario_opening_movie),
            {0xE8, 0x6E, 0x90, 0xED, 0xFF, 0x83, 0xC4, 0x04,
             0x6A, 0x10, 0xFF, 0xD3, 0xF6, 0xC4, 0x80, 0x75},
        },
        {
            0x0052AC4A,
            reinterpret_cast<uintptr_t>(&scenario_top_menu),
            {0xE8, 0x11, 0x37, 0x06, 0x00, 0x83, 0xC4, 0x04,
             0x85, 0xC0, 0x0F, 0x85, 0xB9, 0x00, 0x00, 0x00},
        },
    };
    for (size_t index = 0; index < CallRedirectCount; index++) {
        if (redirect_call(CallRedirects[index], call_specs[index].original_address,
                          call_specs[index].replacement_address,
                          call_specs[index].signature)) {
            continue;
        }
        while (index > 0) {
            index--;
            if (!restore_redirect(CallRedirects[index])) {
                TerminateProcess(GetCurrentProcess(), ERROR_INVALID_STATE);
            }
        }
        for (size_t redirect = RedirectCount; redirect > 0; redirect--) {
            if (!restore_redirect(Redirects[redirect - 1])) {
                TerminateProcess(GetCurrentProcess(), ERROR_INVALID_STATE);
            }
        }
        return false;
    }
    return true;
}

bool restore_redirects() {
    bool restored = true;
    for (size_t index = CallRedirectCount; index > 0; index--) {
        if (!restore_redirect(CallRedirects[index - 1])) {
            restored = false;
        }
    }
    for (size_t index = RedirectCount; index > 0; index--) {
        if (!restore_redirect(Redirects[index - 1])) {
            restored = false;
        }
    }
    return restored;
}

} // namespace

BOOL APIENTRY DllMain(HMODULE hModule, uint32_t ul_reason_for_call, LPVOID reserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        return install_redirects();
    case DLL_PROCESS_DETACH:
        if (!reserved && !restore_redirects()) {
            TerminateProcess(GetCurrentProcess(), ERROR_INVALID_STATE);
        }
        return true;
    default:
        return true;
    }
}
