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
#include "basebutton.h"
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
#include "font.h"
#include "flatbutton.h"
#include "graphicwin.h"
#include "scroll.h"
#include "spying_recovery.h"
#include "sprite.h"
#include "redirect_signatures.h"
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
constexpr size_t RedirectCount = 106;
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
// Installed specs are retained so a single redirect can be lifted and
// reinstalled later, which the deferred oracle phase needs to call an
// original body after the executable's CRT has started.
RedirectSpec InstalledSpecs[RedirectCount] = {};
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
    const SignatureExtension extensions[] = OPENSMACX_SIGNATURE_EXTENSIONS;
    for (const SignatureExtension &extension : extensions) {
        if (extension.original_address == spec.original_address
                && !validate_signature(
                       spec.original_address + extension.offset,
                       extension.signature, SignatureExtensionSize)) {
            return false;
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
            OPENSMACX_SIGNATURE_004BA830,
        },
        {
            0x004066C0,
            reinterpret_cast<uintptr_t>(&string_struct_derived_close_redirect),
            OPENSMACX_SIGNATURE_004066C0,
        },
        {
            0x00406880,
            reinterpret_cast<uintptr_t>(&flat_button_destructor_redirect),
            OPENSMACX_SIGNATURE_00406880,
        },
        {
            0x00401060,
            reinterpret_cast<uintptr_t>(&string_struct_close_redirect),
            OPENSMACX_SIGNATURE_00401060,
        },
        {
            0x00401560,
            reinterpret_cast<uintptr_t>(&string_struct_seek_id_redirect),
            OPENSMACX_SIGNATURE_00401560,
        },
        {
            0x00401640,
            reinterpret_cast<uintptr_t>(&string_struct_current_id_redirect),
            OPENSMACX_SIGNATURE_00401640,
        },
        {
            0x00402970,
            reinterpret_cast<uintptr_t>(&string_struct_remove_all_redirect),
            OPENSMACX_SIGNATURE_00402970,
        },
        {
            0x00402530,
            reinterpret_cast<uintptr_t>(&string_struct_current_entry_redirect),
            OPENSMACX_SIGNATURE_00402530,
        },
        {
            0x00402500,
            reinterpret_cast<uintptr_t>(&string_struct_next_entry_redirect),
            OPENSMACX_SIGNATURE_00402500,
        },
        {
            0x004E3A50,
            reinterpret_cast<uintptr_t>(&base_at),
            OPENSMACX_SIGNATURE_004E3A50,
        },
        {
            0x005DAC70,
            reinterpret_cast<uintptr_t>(&buffer_set_font_redirect),
            OPENSMACX_SIGNATURE_005DAC70,
        },
        {
            0x005DACB0,
            reinterpret_cast<uintptr_t>(&buffer_set_text_color_redirect),
            OPENSMACX_SIGNATURE_005DACB0,
        },
        {
            0x005DACE0,
            reinterpret_cast<uintptr_t>(&buffer_set_text_color2_redirect),
            OPENSMACX_SIGNATURE_005DACE0,
        },
        {
            0x005DAD10,
            reinterpret_cast<uintptr_t>(&buffer_set_text_color3_redirect),
            OPENSMACX_SIGNATURE_005DAD10,
        },
        {
            0x005DAD40,
            reinterpret_cast<uintptr_t>(&buffer_set_text_color_hyper_redirect),
            OPENSMACX_SIGNATURE_005DAD40,
        },
        {
            0x006099D0,
            reinterpret_cast<uintptr_t>(&dialog_set_selected_id_redirect),
            OPENSMACX_SIGNATURE_006099D0,
        },
        {
            0x00609A50,
            reinterpret_cast<uintptr_t>(&dialog_get_selected_id_redirect),
            OPENSMACX_SIGNATURE_00609A50,
        },
        {
            0x00609AF0,
            reinterpret_cast<uintptr_t>(&dialog_id_to_pos_redirect),
            OPENSMACX_SIGNATURE_00609AF0,
        },
        {
            0x00609B50,
            reinterpret_cast<uintptr_t>(&dialog_pos_to_id_redirect),
            OPENSMACX_SIGNATURE_00609B50,
        },
        {
            0x00609C60,
            reinterpret_cast<uintptr_t>(&dialog_set_font_redirect),
            OPENSMACX_SIGNATURE_00609C60,
        },
        {
            0x00609C90,
            reinterpret_cast<uintptr_t>(&dialog_set_text_color_redirect),
            OPENSMACX_SIGNATURE_00609C90,
        },
        {
            0x00609CC0,
            reinterpret_cast<uintptr_t>(&dialog_set_text_color2_redirect),
            OPENSMACX_SIGNATURE_00609CC0,
        },
        {
            0x00609CF0,
            reinterpret_cast<uintptr_t>(&dialog_set_text_color3_redirect),
            OPENSMACX_SIGNATURE_00609CF0,
        },
        {
            0x005D4DD0,
            reinterpret_cast<uintptr_t>(&graphic_win_destructor_redirect),
            OPENSMACX_SIGNATURE_005D4DD0,
        },
        {
            0x005D4E40,
            reinterpret_cast<uintptr_t>(&graphic_win_close_redirect),
            OPENSMACX_SIGNATURE_005D4E40,
        },
        {
            0x005E37E0,
            reinterpret_cast<uintptr_t>(&sprite_construct_redirect),
            OPENSMACX_SIGNATURE_005E37E0,
        },
        {
            0x0055BC00,
            reinterpret_cast<uintptr_t>(&spying),
            OPENSMACX_SIGNATURE_0055BC00,
        },
        {
            0x005882F0,
            reinterpret_cast<uintptr_t>(&find_font),
            OPENSMACX_SIGNATURE_005882F0,
        },
        {
            0x005D7410,
            reinterpret_cast<uintptr_t>(&buffer_destructor_redirect),
            OPENSMACX_SIGNATURE_005D7410,
        },
        {
            0x005D7470,
            reinterpret_cast<uintptr_t>(&buffer_close_redirect),
            OPENSMACX_SIGNATURE_005D7470,
        },
        {
            0x005DCAB0,
            reinterpret_cast<uintptr_t>(&buffer_text_line_height_redirect),
            OPENSMACX_SIGNATURE_005DCAB0,
        },
        {
            0x005E3373,
            reinterpret_cast<uintptr_t>(&buffer_get_data_redirect),
            OPENSMACX_SIGNATURE_005E3373,
        },
        {
            0x005E34A3,
            reinterpret_cast<uintptr_t>(&buffer_free_data_redirect),
            OPENSMACX_SIGNATURE_005E34A3,
        },
        {
            0x005E3820,
            reinterpret_cast<uintptr_t>(&sprite_close_redirect),
            OPENSMACX_SIGNATURE_005E3820,
        },
        {
            0x005F7E90,
            reinterpret_cast<uintptr_t>(&win_is_visible_redirect),
            OPENSMACX_SIGNATURE_005F7E90,
        },
        {
            0x005F86C0,
            reinterpret_cast<uintptr_t>(&make_rect),
            OPENSMACX_SIGNATURE_005F86C0,
        },
        {
            0x005FA7A0,
            reinterpret_cast<uintptr_t>(
                static_cast<int (__cdecl *)(int, int, int, int, int, int)>(&in_box)),
            OPENSMACX_SIGNATURE_005FA7A0,
        },
        {
            0x005FA7E0,
            reinterpret_cast<uintptr_t>(
                static_cast<BOOL (__cdecl *)(int, int, const RECT *)>(&in_box)),
            OPENSMACX_SIGNATURE_005FA7E0,
        },
        {
            0x005ED7D0,
            reinterpret_cast<uintptr_t>(&win_move_redirect),
            OPENSMACX_SIGNATURE_005ED7D0,
        },
        {
            0x005ED240,
            reinterpret_cast<uintptr_t>(&win_client_to_screen_redirect),
            OPENSMACX_SIGNATURE_005ED240,
        },
        {
            0x005EE0F0,
            reinterpret_cast<uintptr_t>(&win_set_vert_paging_redirect),
            OPENSMACX_SIGNATURE_005EE0F0,
        },
        {
            0x005EE110,
            reinterpret_cast<uintptr_t>(&win_set_horz_paging_redirect),
            OPENSMACX_SIGNATURE_005EE110,
        },
        {
            0x005F88A0,
            reinterpret_cast<uintptr_t>(&pull_down_destructor_redirect),
            OPENSMACX_SIGNATURE_005F88A0,
        },
        {
            0x005F8CB0,
            reinterpret_cast<uintptr_t>(&pull_down_hide_item_redirect),
            OPENSMACX_SIGNATURE_005F8CB0,
        },
        {
            0x005F8D20,
            reinterpret_cast<uintptr_t>(&pull_down_show_item_redirect),
            OPENSMACX_SIGNATURE_005F8D20,
        },
        {
            0x005F8D90,
            reinterpret_cast<uintptr_t>(&pull_down_disable_item_redirect),
            OPENSMACX_SIGNATURE_005F8D90,
        },
        {
            0x005F8DF0,
            reinterpret_cast<uintptr_t>(&pull_down_enable_item_redirect),
            OPENSMACX_SIGNATURE_005F8DF0,
        },
        {
            0x005F9040,
            reinterpret_cast<uintptr_t>(&pull_down_check_item_redirect),
            OPENSMACX_SIGNATURE_005F9040,
        },
        {
            0x005F90A0,
            reinterpret_cast<uintptr_t>(&pull_down_uncheck_item_redirect),
            OPENSMACX_SIGNATURE_005F90A0,
        },
        {
            0x005F9F40,
            reinterpret_cast<uintptr_t>(&pull_down_get_selected_redirect),
            OPENSMACX_SIGNATURE_005F9F40,
        },
        {
            0x005FB820,
            reinterpret_cast<uintptr_t>(&menu_set_menu_proc_redirect),
            OPENSMACX_SIGNATURE_005FB820,
        },
        {
            0x005FB990,
            reinterpret_cast<uintptr_t>(&menu_id_to_index_redirect),
            OPENSMACX_SIGNATURE_005FB990,
        },
        {
            0x00605370,
            reinterpret_cast<uintptr_t>(&scroll_close_redirect),
            OPENSMACX_SIGNATURE_00605370,
        },
        {
            0x00607040,
            reinterpret_cast<uintptr_t>(&base_button_destructor_redirect),
            OPENSMACX_SIGNATURE_00607040,
        },
        {
            0x006070C0,
            reinterpret_cast<uintptr_t>(&base_button_close_redirect),
            OPENSMACX_SIGNATURE_006070C0,
        },
        {
            0x00607DA0,
            reinterpret_cast<uintptr_t>(&flat_button_close_redirect),
            OPENSMACX_SIGNATURE_00607DA0,
        },
        {
            0x00605840,
            reinterpret_cast<uintptr_t>(&scroll_init_rect_redirect),
            OPENSMACX_SIGNATURE_00605840,
        },
        {
            0x00605890,
            reinterpret_cast<uintptr_t>(&scroll_init_vert_redirect),
            OPENSMACX_SIGNATURE_00605890,
        },
        {
            0x006058D0,
            reinterpret_cast<uintptr_t>(&scroll_init_horz_redirect),
            OPENSMACX_SIGNATURE_006058D0,
        },
        {
            0x00605910,
            reinterpret_cast<uintptr_t>(&scroll_init_vert_nc_redirect),
            OPENSMACX_SIGNATURE_00605910,
        },
        {
            0x00605960,
            reinterpret_cast<uintptr_t>(&scroll_init_horz_nc_redirect),
            OPENSMACX_SIGNATURE_00605960,
        },
        {
            0x006059B0,
            reinterpret_cast<uintptr_t>(&scroll_set_range_redirect),
            OPENSMACX_SIGNATURE_006059B0,
        },
        {
            0x00605A10,
            reinterpret_cast<uintptr_t>(&scroll_set_button_color_redirect),
            OPENSMACX_SIGNATURE_00605A10,
        },
        {
            0x00605A50,
            reinterpret_cast<uintptr_t>(&scroll_set_bevel_thickness_redirect),
            OPENSMACX_SIGNATURE_00605A50,
        },
        {
            0x00605A90,
            reinterpret_cast<uintptr_t>(&scroll_set_bevel_upper_redirect),
            OPENSMACX_SIGNATURE_00605A90,
        },
        {
            0x00605AD0,
            reinterpret_cast<uintptr_t>(&scroll_set_bevel_lower_redirect),
            OPENSMACX_SIGNATURE_00605AD0,
        },
        {
            0x00605B10,
            reinterpret_cast<uintptr_t>(&scroll_set_border_color_redirect),
            OPENSMACX_SIGNATURE_00605B10,
        },
        {
            0x00605B80,
            reinterpret_cast<uintptr_t>(&scroll_set_bar_thickness_redirect),
            OPENSMACX_SIGNATURE_00605B80,
        },
        {
            0x00605BE0,
            reinterpret_cast<uintptr_t>(&scroll_set_sprite_left_redirect),
            OPENSMACX_SIGNATURE_00605BE0,
        },
        {
            0x00605C30,
            reinterpret_cast<uintptr_t>(&scroll_set_sprite_right_redirect),
            OPENSMACX_SIGNATURE_00605C30,
        },
        {
            0x00605C80,
            reinterpret_cast<uintptr_t>(&scroll_set_sprite_up_redirect),
            OPENSMACX_SIGNATURE_00605C80,
        },
        {
            0x00605CD0,
            reinterpret_cast<uintptr_t>(&scroll_set_sprite_down_redirect),
            OPENSMACX_SIGNATURE_00605CD0,
        },
        {
            0x00605D20,
            reinterpret_cast<uintptr_t>(&scroll_set_pos_redirect),
            OPENSMACX_SIGNATURE_00605D20,
        },
        {
            0x00606C50,
            reinterpret_cast<uintptr_t>(&scroll_compute_thumb_rect_redirect),
            OPENSMACX_SIGNATURE_00606C50,
        },
        {
            0x00606EA0,
            reinterpret_cast<uintptr_t>(&scroll_set_thumb_rect_redirect),
            OPENSMACX_SIGNATURE_00606EA0,
        },
        {
            0x00606F00,
            reinterpret_cast<uintptr_t>(&expand_rect),
            OPENSMACX_SIGNATURE_00606F00,
        },
        {
            0x005FD2B0,
            reinterpret_cast<uintptr_t>(&do_sound_redirect),
            OPENSMACX_SIGNATURE_005FD2B0,
        },
        {
            0x00601B80,
            reinterpret_cast<uintptr_t>(&base_pop_set_loc_redirect),
            OPENSMACX_SIGNATURE_00601B80,
        },
        {
            0x006046F0,
            reinterpret_cast<uintptr_t>(&base_pop_set_string_font_redirect),
            OPENSMACX_SIGNATURE_006046F0,
        },
        {
            0x0062B7C0,
            reinterpret_cast<uintptr_t>(&button_group_construct_redirect),
            OPENSMACX_SIGNATURE_0062B7C0,
        },
        {
            0x0062B7F0,
            reinterpret_cast<uintptr_t>(&button_group_close_redirect),
            OPENSMACX_SIGNATURE_0062B7F0,
        },
        {
            0x0062B810,
            reinterpret_cast<uintptr_t>(&button_group_init_redirect),
            OPENSMACX_SIGNATURE_0062B810,
        },
        {
            0x0062B850,
            reinterpret_cast<uintptr_t>(&button_group_add_redirect),
            OPENSMACX_SIGNATURE_0062B850,
        },
        {
            0x00616BC0,
            reinterpret_cast<uintptr_t>(&caviar_data_construct_redirect),
            OPENSMACX_SIGNATURE_00616BC0,
        },
        {
            0x00616DA0,
            reinterpret_cast<uintptr_t>(&caviar_construct_redirect),
            OPENSMACX_SIGNATURE_00616DA0,
        },
        {
            0x006183B0,
            reinterpret_cast<uintptr_t>(&caviar_set_scaling_redirect),
            OPENSMACX_SIGNATURE_006183B0,
        },
        {
            0x006182A0,
            reinterpret_cast<uintptr_t>(&caviar_set_camera_direct_redirect),
            OPENSMACX_SIGNATURE_006182A0,
        },
        {
            0x006183C0,
            reinterpret_cast<uintptr_t>(&caviar_get_scaling_redirect),
            OPENSMACX_SIGNATURE_006183C0,
        },
        {
            0x0045C170,
            reinterpret_cast<uintptr_t>(&main_interface_clear_message_redirect),
            OPENSMACX_SIGNATURE_0045C170,
        },
        {
            0x0058EE50,
            reinterpret_cast<uintptr_t>(&desktop_update),
            OPENSMACX_SIGNATURE_0058EE50,
        },
        {
            0x005DF570,
            reinterpret_cast<uintptr_t>(&buffer_init_class_redirect),
            OPENSMACX_SIGNATURE_005DF570,
        },
        {
            0x005DF580,
            reinterpret_cast<uintptr_t>(&buffer_close_class_redirect),
            OPENSMACX_SIGNATURE_005DF580,
        },
        {
            0x00612960,
            reinterpret_cast<uintptr_t>(&dialogs_close_redirect),
            OPENSMACX_SIGNATURE_00612960,
        },
        {
            0x004E25E0,
            reinterpret_cast<uintptr_t>(&alpha_net_pid_to_idx_redirect),
            OPENSMACX_SIGNATURE_004E25E0,
        },
        {
            0x004E2610,
            reinterpret_cast<uintptr_t>(&alpha_net_pid_to_who_redirect),
            OPENSMACX_SIGNATURE_004E2610,
        },
        {
            0x004E2660,
            reinterpret_cast<uintptr_t>(&alpha_net_who_to_pid_redirect),
            OPENSMACX_SIGNATURE_004E2660,
        },
        {
            0x004E26B0,
            reinterpret_cast<uintptr_t>(&alpha_net_who_to_idx_redirect),
            OPENSMACX_SIGNATURE_004E26B0,
        },
        {
            0x006343C0,
            reinterpret_cast<uintptr_t>(&vector_construct_redirect),
            OPENSMACX_SIGNATURE_006343C0,
        },
        {
            0x006343D0,
            reinterpret_cast<uintptr_t>(&vector_close_redirect),
            OPENSMACX_SIGNATURE_006343D0,
        },
        {
            0x00634430,
            reinterpret_cast<uintptr_t>(&vector_subtract_redirect),
            OPENSMACX_SIGNATURE_00634430,
        },
        {
            0x00634480,
            reinterpret_cast<uintptr_t>(&vector_add_assign_redirect),
            OPENSMACX_SIGNATURE_00634480,
        },
        {
            0x006344B0,
            reinterpret_cast<uintptr_t>(&vector_subtract_assign_redirect),
            OPENSMACX_SIGNATURE_006344B0,
        },
        {
            0x00628150,
            reinterpret_cast<uintptr_t>(&vector_add),
            OPENSMACX_SIGNATURE_00628150,
        },
        {
            0x00634670,
            reinterpret_cast<uintptr_t>(&vector_scale_redirect),
            OPENSMACX_SIGNATURE_00634670,
        },
        {
            0x00514189,
            reinterpret_cast<uintptr_t>(&scenario_human_turn_trampoline),
            OPENSMACX_SIGNATURE_00514189,
        },
        {
            0x00525AF4,
            reinterpret_cast<uintptr_t>(&scenario_turn_advance_trampoline),
            OPENSMACX_SIGNATURE_00525AF4,
        },
    };
    static_assert(sizeof(specs) / sizeof(specs[0]) == RedirectCount,
                  "redirect state count must match the redirect catalog");
    for (const RedirectSpec &spec : specs) {
        if (!validate_redirect_spec(spec)) {
            return false;
        }
    }
    const uint8_t scroll_primary_init_signature[SignatureSize] =
        OPENSMACX_SIGNATURE_006054D0;
    if (!validate_signature(
            0x006054D0, scroll_primary_init_signature, SignatureSize)) {
        return false;
    }
    const uint8_t win_close_signature[SignatureSize] =
        OPENSMACX_SIGNATURE_005EB640;
    if (!validate_signature(
            0x005EB640, win_close_signature, SignatureSize)) {
        return false;
    }
    // Buffer teardown is source-owned now. GraphicWin still retains the Win
    // close and destructor bodies as preflighted temporary dependencies.
    const uint8_t win_destructor_signature[SignatureSize] =
        OPENSMACX_SIGNATURE_005EBC90;
    if (!validate_signature(
            0x005EBC90, win_destructor_signature, SignatureSize)) {
        return false;
    }
    if (!run_runtime_oracles()) {
        return false;
    }
    for (size_t index = 0; index < RedirectCount; index++) {
        InstalledSpecs[index] = specs[index];
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
            OPENSMACX_SIGNATURE_0052AB6D,
        },
        {
            0x0052AC4A,
            reinterpret_cast<uintptr_t>(&scenario_top_menu),
            OPENSMACX_SIGNATURE_0052AC4A,
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

}  // namespace

bool suspend_redirect_at(uintptr_t address) {
    for (size_t index = 0; index < RedirectCount; index++) {
        if (InstalledSpecs[index].original_address == address) {
            return Redirects[index].installed
                && restore_redirect(Redirects[index]);
        }
    }
    return false;
}

bool resume_redirect_at(uintptr_t address) {
    for (size_t index = 0; index < RedirectCount; index++) {
        if (InstalledSpecs[index].original_address == address) {
            return redirect_function(Redirects[index], InstalledSpecs[index]);
        }
    }
    return false;
}

namespace {

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
