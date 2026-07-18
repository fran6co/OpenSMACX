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
#include "menu.h"
#include "pulldown.h"
#include "scenario.h"
#include "scroll.h"
#include "runtime_oracle.h"
#include "stringstruct.h"
#include "vector.h"
#include "win.h"

#include <cstdint>
#include <cstring>

namespace {

constexpr size_t PatchSize = 5;
constexpr size_t SignatureSize = 16;
constexpr size_t SignatureExtensionSize = 6;
constexpr size_t RedirectCount = 83;
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

struct SignatureExtension {
    uintptr_t original_address;
    size_t offset;
    uint8_t signature[SignatureExtensionSize];
};

RedirectState Redirects[RedirectCount] = {};
RedirectState CallRedirects[CallRedirectCount] = {};

bool validate_signature(uintptr_t address, const uint8_t *expected, size_t size) {
    auto *original = reinterpret_cast<uint8_t *>(address);
    MEMORY_BASIC_INFORMATION memory_info;
    if (VirtualQuery(original, &memory_info, sizeof(memory_info)) != sizeof(memory_info)
        || memory_info.State != MEM_COMMIT
        || (memory_info.Protect & PAGE_GUARD) != 0) {
        return false;
    }
    const DWORD basic_protection = memory_info.Protect & 0xFF;
    if (basic_protection != PAGE_READONLY && basic_protection != PAGE_READWRITE
        && basic_protection != PAGE_WRITECOPY && basic_protection != PAGE_EXECUTE_READ
        && basic_protection != PAGE_EXECUTE_READWRITE
        && basic_protection != PAGE_EXECUTE_WRITECOPY) {
        return false;
    }
    const uintptr_t region_end = reinterpret_cast<uintptr_t>(memory_info.BaseAddress)
        + memory_info.RegionSize;
    return address <= region_end && size <= region_end - address
        && memcmp(original, expected, size) == 0;
}

bool validate_redirect_spec(const RedirectSpec &spec) {
    if (!validate_signature(spec.original_address, spec.signature, SignatureSize)) {
        return false;
    }
    const SignatureExtension extensions[] = {
        {0x00605BE0, 15, {0x89, 0x81, 0x7C, 0x0A, 0x00, 0x00}},
        {0x00605C30, 15, {0x89, 0x81, 0x94, 0x0A, 0x00, 0x00}},
        {0x00605C80, 15, {0x89, 0x81, 0x88, 0x0A, 0x00, 0x00}},
        {0x00605CD0, 15, {0x89, 0x81, 0xA0, 0x0A, 0x00, 0x00}},
    };
    for (const SignatureExtension &extension : extensions) {
        if (extension.original_address == spec.original_address) {
            return validate_signature(
                spec.original_address + extension.offset,
                extension.signature, SignatureExtensionSize);
        }
    }
    return true;
}

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

bool redirect_function(RedirectState &state, const RedirectSpec &spec) {
    static_assert(sizeof(uintptr_t) == sizeof(uint32_t), "runtime redirects require PE32");
    if (!validate_redirect_spec(spec)) {
        return false;
    }
    auto *original = reinterpret_cast<uint8_t *>(spec.original_address);

    uint8_t patch[PatchSize] = {0xE9, 0, 0, 0, 0};
    uint32_t relative_jump =
        spec.replacement_address - (spec.original_address + PatchSize);
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

bool redirect_call(RedirectState &state, const RedirectSpec &spec) {
    static_assert(sizeof(uintptr_t) == sizeof(uint32_t), "runtime redirects require PE32");
    if (!validate_redirect_spec(spec)) {
        return false;
    }
    auto *original = reinterpret_cast<uint8_t *>(spec.original_address);

    uint8_t patch[PatchSize] = {0xE8, 0, 0, 0, 0};
    uint32_t relative_call =
        spec.replacement_address - (spec.original_address + PatchSize);
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
    const RedirectSpec specs[] = {
        {
            0x004BA830,
            reinterpret_cast<uintptr_t>(&tutwin_rect_center_redirect),
            {0x55, 0x8B, 0xEC, 0x8B, 0x4D, 0x08, 0x56, 0x8B,
             0x31, 0x8B, 0x41, 0x08, 0x2B, 0xC6, 0x99, 0x2B},
        },
        {
            0x00401560,
            reinterpret_cast<uintptr_t>(&string_struct_seek_id_redirect),
            {0x55, 0x8B, 0xEC, 0x8B, 0x41, 0x08, 0x33, 0xD2,
             0x53, 0x56, 0x3B, 0xC2, 0x57, 0x74, 0x2A, 0x8B},
        },
        {
            0x00401640,
            reinterpret_cast<uintptr_t>(&string_struct_current_id_redirect),
            {0x8B, 0x41, 0x08, 0x85, 0xC0, 0x74, 0x07, 0x8B,
             0x41, 0x0C, 0x8B, 0x40, 0x04, 0xC3, 0x33, 0xC0},
        },
        {
            0x00402530,
            reinterpret_cast<uintptr_t>(&string_struct_current_entry_redirect),
            {0x8B, 0x41, 0x08, 0x85, 0xC0, 0x74, 0x07, 0x8B,
             0x41, 0x0C, 0x8B, 0x40, 0x08, 0xC3, 0x33, 0xC0},
        },
        {
            0x00402500,
            reinterpret_cast<uintptr_t>(&string_struct_next_entry_redirect),
            {0x8B, 0x41, 0x08, 0x85, 0xC0, 0x75, 0x03, 0x33,
             0xC0, 0xC3, 0x8B, 0x41, 0x0C, 0x8B, 0x50, 0x0C},
        },
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
            0x006099D0,
            reinterpret_cast<uintptr_t>(&dialog_set_selected_id_redirect),
            {0x8B, 0x81, 0xC4, 0x00, 0x00, 0x00, 0x33, 0xD2,
             0x53, 0x56, 0x3B, 0xC2, 0x57, 0x74, 0x40, 0x8B},
        },
        {
            0x00609A50,
            reinterpret_cast<uintptr_t>(&dialog_get_selected_id_redirect),
            {0x56, 0x8B, 0xF1, 0x57, 0x8B, 0x86, 0xCC, 0x00,
             0x00, 0x00, 0x8B, 0xBE, 0xEC, 0x00, 0x00, 0x00},
        },
        {
            0x00609AF0,
            reinterpret_cast<uintptr_t>(&dialog_id_to_pos_redirect),
            {0x8B, 0x81, 0xC4, 0x00, 0x00, 0x00, 0x33, 0xD2,
             0x53, 0x56, 0x3B, 0xC2, 0x57, 0x74, 0x40, 0x8B},
        },
        {
            0x00609B50,
            reinterpret_cast<uintptr_t>(&dialog_pos_to_id_redirect),
            {0x56, 0x8B, 0xF1, 0x57, 0x8B, 0x7C, 0x24, 0x0C,
             0x8B, 0x8E, 0xCC, 0x00, 0x00, 0x00, 0x8B, 0xC7},
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
            0x005F86C0,
            reinterpret_cast<uintptr_t>(&make_rect),
            {0x8B, 0x44, 0x24, 0x04, 0x8B, 0x4C, 0x24, 0x08,
             0x8B, 0x54, 0x24, 0x0C, 0x56, 0x8B, 0x74, 0x24},
        },
        {
            0x005FA7A0,
            reinterpret_cast<uintptr_t>(
                static_cast<int (__cdecl *)(int, int, int, int, int, int)>(&in_box)),
            {0x8B, 0x44, 0x24, 0x04, 0x8B, 0x4C, 0x24, 0x0C,
             0x3B, 0xC1, 0x7D, 0x03, 0x33, 0xC0, 0xC3, 0x8B},
        },
        {
            0x005FA7E0,
            reinterpret_cast<uintptr_t>(
                static_cast<BOOL (__cdecl *)(int, int, const RECT *)>(&in_box)),
            {0x8B, 0x44, 0x24, 0x0C, 0x8B, 0x4C, 0x24, 0x04,
             0x3B, 0x08, 0x7D, 0x03, 0x33, 0xC0, 0xC3, 0x3B},
        },
        {
            0x005ED7D0,
            reinterpret_cast<uintptr_t>(&win_move_redirect),
            {0x8A, 0x81, 0x9C, 0x00, 0x00, 0x00, 0x56, 0xA8,
             0x02, 0x8B, 0x44, 0x24, 0x0C, 0x74, 0x4C, 0x8B},
        },
        {
            0x005EE0F0,
            reinterpret_cast<uintptr_t>(&win_set_vert_paging_redirect),
            {0x8B, 0x81, 0x3C, 0x04, 0x00, 0x00, 0x85, 0xC0,
             0x74, 0x0A, 0x8B, 0x4C, 0x24, 0x04, 0x89, 0x88},
        },
        {
            0x005EE110,
            reinterpret_cast<uintptr_t>(&win_set_horz_paging_redirect),
            {0x8B, 0x81, 0x40, 0x04, 0x00, 0x00, 0x85, 0xC0,
             0x74, 0x0A, 0x8B, 0x4C, 0x24, 0x04, 0x89, 0x88},
        },
        {
            0x005F8CB0,
            reinterpret_cast<uintptr_t>(&pull_down_hide_item_redirect),
            {0x56, 0x57, 0x8B, 0x7C, 0x24, 0x0C, 0x33, 0xC0,
             0x8D, 0xB1, 0x20, 0x0A, 0x00, 0x00, 0x8B, 0x16},
        },
        {
            0x005F8D20,
            reinterpret_cast<uintptr_t>(&pull_down_show_item_redirect),
            {0x56, 0x57, 0x8B, 0x7C, 0x24, 0x0C, 0x33, 0xC0,
             0x8D, 0xB1, 0x20, 0x0A, 0x00, 0x00, 0x8B, 0x16},
        },
        {
            0x005F8D90,
            reinterpret_cast<uintptr_t>(&pull_down_disable_item_redirect),
            {0x56, 0x57, 0x8B, 0x7C, 0x24, 0x0C, 0x33, 0xC0,
             0x8D, 0xB1, 0x20, 0x0A, 0x00, 0x00, 0x8B, 0x16},
        },
        {
            0x005F8DF0,
            reinterpret_cast<uintptr_t>(&pull_down_enable_item_redirect),
            {0x56, 0x57, 0x8B, 0x7C, 0x24, 0x0C, 0x33, 0xC0,
             0x8D, 0xB1, 0x20, 0x0A, 0x00, 0x00, 0x8B, 0x16},
        },
        {
            0x005F9040,
            reinterpret_cast<uintptr_t>(&pull_down_check_item_redirect),
            {0x56, 0x57, 0x8B, 0x7C, 0x24, 0x0C, 0x33, 0xC0,
             0x8D, 0xB1, 0x20, 0x0A, 0x00, 0x00, 0x8B, 0x16},
        },
        {
            0x005F90A0,
            reinterpret_cast<uintptr_t>(&pull_down_uncheck_item_redirect),
            {0x56, 0x57, 0x8B, 0x7C, 0x24, 0x0C, 0x33, 0xC0,
             0x8D, 0xB1, 0x20, 0x0A, 0x00, 0x00, 0x8B, 0x16},
        },
        {
            0x005F9F40,
            reinterpret_cast<uintptr_t>(&pull_down_get_selected_redirect),
            {0x8B, 0x81, 0x28, 0x0F, 0x00, 0x00, 0x83, 0xF8,
             0xFF, 0x74, 0x0D, 0x8D, 0x14, 0x80, 0xF6, 0x84},
        },
        {
            0x005FB820,
            reinterpret_cast<uintptr_t>(&menu_set_menu_proc_redirect),
            {0x8B, 0x44, 0x24, 0x04, 0x89, 0x81, 0x14, 0x0A,
             0x00, 0x00, 0xC2, 0x04, 0x00, 0x90, 0x90, 0x90},
        },
        {
            0x005FB990,
            reinterpret_cast<uintptr_t>(&menu_id_to_index_redirect),
            {0x56, 0x8B, 0x74, 0x24, 0x08, 0x33, 0xC0, 0x8D,
             0x91, 0x38, 0x0A, 0x00, 0x00, 0x8B, 0x0A, 0x83},
        },
        {
            0x00605840,
            reinterpret_cast<uintptr_t>(&scroll_init_rect_redirect),
            {0x8B, 0x44, 0x24, 0x04, 0x57, 0x85, 0xC0, 0x74,
             0x33, 0x8B, 0x7C, 0x24, 0x0C, 0x85, 0xFF, 0x74},
        },
        {
            0x00605890,
            reinterpret_cast<uintptr_t>(&scroll_init_vert_redirect),
            {0x8B, 0x54, 0x24, 0x10, 0x85, 0xD2, 0x74, 0x2B,
             0x8B, 0x44, 0x24, 0x0C, 0x85, 0xC0, 0x74, 0x23},
        },
        {
            0x006058D0,
            reinterpret_cast<uintptr_t>(&scroll_init_horz_redirect),
            {0x8B, 0x54, 0x24, 0x10, 0x85, 0xD2, 0x74, 0x2C,
             0x8B, 0x44, 0x24, 0x0C, 0x85, 0xC0, 0x74, 0x24},
        },
        {
            0x00605910,
            reinterpret_cast<uintptr_t>(&scroll_init_vert_nc_redirect),
            {0x8B, 0x54, 0x24, 0x10, 0xC7, 0x05, 0x24, 0x8E,
             0x9B, 0x00, 0x01, 0x00, 0x00, 0x00, 0x85, 0xD2},
        },
        {
            0x00605960,
            reinterpret_cast<uintptr_t>(&scroll_init_horz_nc_redirect),
            {0x8B, 0x54, 0x24, 0x10, 0xC7, 0x05, 0x24, 0x8E,
             0x9B, 0x00, 0x01, 0x00, 0x00, 0x00, 0x85, 0xD2},
        },
        {
            0x006059B0,
            reinterpret_cast<uintptr_t>(&scroll_set_range_redirect),
            {0x8B, 0x44, 0x24, 0x04, 0x56, 0x8B, 0xF1, 0x3B,
             0x44, 0x24, 0x0C, 0x7D, 0x0C, 0xC7, 0x86, 0x28},
        },
        {
            0x00605A10,
            reinterpret_cast<uintptr_t>(&scroll_set_button_color_redirect),
            {0x8B, 0x44, 0x24, 0x04, 0x56, 0x8B, 0xF1, 0x89,
             0x86, 0x5C, 0x0A, 0x00, 0x00, 0x8D, 0x8E, 0xAC},
        },
        {
            0x00605A50,
            reinterpret_cast<uintptr_t>(&scroll_set_bevel_thickness_redirect),
            {0x8B, 0x44, 0x24, 0x04, 0x56, 0x8B, 0xF1, 0x89,
             0x86, 0x68, 0x0A, 0x00, 0x00, 0x8D, 0x8E, 0xAC},
        },
        {
            0x00605A90,
            reinterpret_cast<uintptr_t>(&scroll_set_bevel_upper_redirect),
            {0x8B, 0x44, 0x24, 0x04, 0x56, 0x8B, 0xF1, 0x89,
             0x86, 0x6C, 0x0A, 0x00, 0x00, 0x8D, 0x8E, 0xAC},
        },
        {
            0x00605AD0,
            reinterpret_cast<uintptr_t>(&scroll_set_bevel_lower_redirect),
            {0x8B, 0x44, 0x24, 0x04, 0x56, 0x8B, 0xF1, 0x89,
             0x86, 0x70, 0x0A, 0x00, 0x00, 0x8D, 0x8E, 0xAC},
        },
        {
            0x00605B10,
            reinterpret_cast<uintptr_t>(&scroll_set_border_color_redirect),
            {0x8B, 0x44, 0x24, 0x04, 0x33, 0xD2, 0x89, 0x81,
             0x1C, 0x0A, 0x00, 0x00, 0x8B, 0x81, 0x60, 0x0A},
        },
        {
            0x00605B80,
            reinterpret_cast<uintptr_t>(&scroll_set_bar_thickness_redirect),
            {0x8B, 0x44, 0x24, 0x04, 0x33, 0xD2, 0x89, 0x81,
             0x60, 0x0A, 0x00, 0x00, 0x89, 0x91, 0x4C, 0x0A},
        },
        {
            0x00605BE0,
            reinterpret_cast<uintptr_t>(&scroll_set_sprite_left_redirect),
            {0x8B, 0x44, 0x24, 0x04, 0x8B, 0x54, 0x24, 0x08,
             0x53, 0x56, 0x8B, 0x74, 0x24, 0x14, 0x57, 0x89},
        },
        {
            0x00605C30,
            reinterpret_cast<uintptr_t>(&scroll_set_sprite_right_redirect),
            {0x8B, 0x44, 0x24, 0x04, 0x8B, 0x54, 0x24, 0x08,
             0x53, 0x56, 0x8B, 0x74, 0x24, 0x14, 0x57, 0x89},
        },
        {
            0x00605C80,
            reinterpret_cast<uintptr_t>(&scroll_set_sprite_up_redirect),
            {0x8B, 0x44, 0x24, 0x04, 0x8B, 0x54, 0x24, 0x08,
             0x53, 0x56, 0x8B, 0x74, 0x24, 0x14, 0x57, 0x89},
        },
        {
            0x00605CD0,
            reinterpret_cast<uintptr_t>(&scroll_set_sprite_down_redirect),
            {0x8B, 0x44, 0x24, 0x04, 0x8B, 0x54, 0x24, 0x08,
             0x53, 0x56, 0x8B, 0x74, 0x24, 0x14, 0x57, 0x89},
        },
        {
            0x00605D20,
            reinterpret_cast<uintptr_t>(&scroll_set_pos_redirect),
            {0x8B, 0x81, 0xC4, 0x00, 0x00, 0x00, 0x85, 0xC0,
             0x74, 0x5D, 0x56, 0xA3, 0xB8, 0x7A, 0x9B, 0x00},
        },
        {
            0x00606C50,
            reinterpret_cast<uintptr_t>(&scroll_compute_thumb_rect_redirect),
            {0x83, 0xEC, 0x08, 0x53, 0x55, 0x56, 0x8B, 0x74,
             0x24, 0x18, 0x8D, 0x81, 0x4C, 0x0A, 0x00, 0x00},
        },
        {
            0x00606EA0,
            reinterpret_cast<uintptr_t>(&scroll_set_thumb_rect_redirect),
            {0x8B, 0x81, 0x60, 0x0A, 0x00, 0x00, 0x33, 0xD2,
             0x89, 0x91, 0x4C, 0x0A, 0x00, 0x00, 0x89, 0x91},
        },
        {
            0x00606F00,
            reinterpret_cast<uintptr_t>(&expand_rect),
            {0x8B, 0x44, 0x24, 0x04, 0x8B, 0x4C, 0x24, 0x08,
             0x8B, 0x10, 0x2B, 0xD1, 0x89, 0x10, 0x8B, 0x50},
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
            0x006046F0,
            reinterpret_cast<uintptr_t>(&base_pop_set_string_font_redirect),
            {0x8B, 0x44, 0x24, 0x04, 0x85, 0xC0, 0x75, 0x08,
             0xB8, 0x03, 0x00, 0x00, 0x00, 0xC2, 0x10, 0x00},
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
            0x004E2610,
            reinterpret_cast<uintptr_t>(&alpha_net_pid_to_who_redirect),
            {0x55, 0x8B, 0xEC, 0x56, 0x8B, 0x75, 0x08, 0xB8,
             0x01, 0x00, 0x00, 0x00, 0x8D, 0x91, 0x28, 0x09},
        },
        {
            0x004E2660,
            reinterpret_cast<uintptr_t>(&alpha_net_who_to_pid_redirect),
            {0x55, 0x8B, 0xEC, 0x56, 0x8B, 0x75, 0x08, 0x57,
             0xB8, 0x01, 0x00, 0x00, 0x00, 0x8D, 0x91, 0x2C},
        },
        {
            0x004E26B0,
            reinterpret_cast<uintptr_t>(&alpha_net_who_to_idx_redirect),
            {0x55, 0x8B, 0xEC, 0x8B, 0x55, 0x08, 0x56, 0xB8,
             0x01, 0x00, 0x00, 0x00, 0x81, 0xC1, 0x2C, 0x09},
        },
        {
            0x006343C0,
            reinterpret_cast<uintptr_t>(&vector_construct_redirect),
            {0x8B, 0xC1, 0x33, 0xC9, 0x89, 0x08, 0x89, 0x48,
             0x04, 0x89, 0x48, 0x08, 0xC3, 0x90, 0x90, 0x90},
        },
        {
            0x006343D0,
            reinterpret_cast<uintptr_t>(&vector_close_redirect),
            {0x33, 0xC0, 0x89, 0x01, 0x89, 0x41, 0x04, 0x89,
             0x41, 0x08, 0xC3, 0x90, 0x90, 0x90, 0x90, 0x90},
        },
        {
            0x00634430,
            reinterpret_cast<uintptr_t>(&vector_subtract_redirect),
            {0x83, 0xEC, 0x10, 0x8B, 0x44, 0x24, 0x18, 0xC7,
             0x44, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD9},
        },
        {
            0x00634480,
            reinterpret_cast<uintptr_t>(&vector_add_assign_redirect),
            {0x8B, 0xC1, 0x8B, 0x4C, 0x24, 0x04, 0xD9, 0x01,
             0xD8, 0x00, 0xD9, 0x18, 0xD9, 0x41, 0x04, 0xD8},
        },
        {
            0x006344B0,
            reinterpret_cast<uintptr_t>(&vector_subtract_assign_redirect),
            {0x8B, 0xC1, 0x8B, 0x4C, 0x24, 0x04, 0xD9, 0x00,
             0xD8, 0x21, 0xD9, 0x18, 0xD9, 0x40, 0x04, 0xD8},
        },
        {
            0x00634670,
            reinterpret_cast<uintptr_t>(&vector_scale_redirect),
            {0x51, 0xD9, 0x44, 0x24, 0x0C, 0xD8, 0x09, 0x8B,
             0x44, 0x24, 0x08, 0x56, 0x8B, 0xD0, 0xC7, 0x44},
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
    static_assert(sizeof(specs) / sizeof(specs[0]) == RedirectCount,
                  "redirect state count must match the redirect catalog");
    for (const RedirectSpec &spec : specs) {
        if (!validate_redirect_spec(spec)) {
            return false;
        }
    }
    const uint8_t scroll_primary_init_signature[SignatureSize] = {
        0x8B, 0x44, 0x24, 0x14, 0x53, 0x55, 0x56, 0x85,
        0xC0, 0x57, 0x8B, 0xF1, 0x0F, 0x84, 0x51, 0x03,
    };
    if (!validate_signature(
            0x006054D0, scroll_primary_init_signature, SignatureSize)) {
        return false;
    }
    if (!run_runtime_oracles()) {
        return false;
    }
    for (size_t index = 0; index < RedirectCount; index++) {
        if (redirect_function(Redirects[index], specs[index])) {
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

    const RedirectSpec call_specs[] = {
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
    static_assert(sizeof(call_specs) / sizeof(call_specs[0]) == CallRedirectCount,
                  "call redirect state count must match the redirect catalog");
    for (size_t index = 0; index < CallRedirectCount; index++) {
        if (redirect_call(CallRedirects[index], call_specs[index])) {
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
