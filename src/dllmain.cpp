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
#include "ambience.h"
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
#include "mapwin.h"
#include "basewin.h"
#include "datalink.h"
#include "netwin.h"
#include "socialwin.h"
#include "designwin.h"
#include "worldwin.h"
#include "diplowin.h"
#include "alphamovie.h"
#include "checkbutton.h"
#include "cursor.h"
#include "infowin.h"
#include "statuswin.h"
#include "filewin.h"
#include "wave.h"
#include "battlewin.h"
#include "councwin.h"
#include "credits.h"
#include "diplopop.h"
#include "gamma.h"
#include "messagewin.h"
#include "newtechwin.h"
#include "pickwin.h"
#include "prodpicker.h"
#include "reportwin.h"
#include "setupwin.h"
#include "multidebug.h"
#include "tutwin.h"
#include "popmenu.h"
#include "popup.h"
#include "netmsg.h"
#include "radiobutton.h"
#include "checkbox.h"
#include "editgroup.h"
#include "xpops.h"
#include "planwin.h"
#include "playerlock.h"
#include "netdaemon.h"
#include "console.h"
#include "replaywin.h"
#include "reportif.h"
#include "dipedit.h"
#include "sound.h"
#include "wave_device.h"
#include "spritebox.h"
#include "listbox.h"
#include "net_class.h"
#include "squarelock.h"
#include "deletionlist.h"
#include "lock.h"
#include "menu.h"
#include "palette.h"
#include "pulldown.h"
#include "scenario.h"
#include "font.h"
#include "flatbutton.h"
#include "graphicwin.h"
#include "scroll.h"
#include "sounddevice.h"
#include "spying_recovery.h"
#include "sprite.h"
#include "redirect_signatures.h"
#include "runtime_oracle.h"
#include "stringbox.h"
#include "stringstruct.h"
#include "vector.h"
#include "win.h"

#include <cstdint>
#include <cstring>

namespace {

constexpr size_t PatchSize = 5;
constexpr size_t SignatureSize = 16;
constexpr size_t SignatureExtensionSize = 6;
constexpr size_t RedirectCount = 535;
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
            0x00404220,
            reinterpret_cast<uintptr_t>(&alpha_movie_unk2_redirect),
            OPENSMACX_SIGNATURE_00404220,
        },
        {
            0x00404240,
            reinterpret_cast<uintptr_t>(&alpha_movie_unk4_redirect),
            OPENSMACX_SIGNATURE_00404240,
        },
        {
            0x00404250,
            reinterpret_cast<uintptr_t>(&alpha_movie_unk5_redirect),
            OPENSMACX_SIGNATURE_00404250,
        },
        {
            0x00404260,
            reinterpret_cast<uintptr_t>(&alpha_movie_unk6_00404260_redirect),
            OPENSMACX_SIGNATURE_00404260,
        },
        {
            0x00404270,
            reinterpret_cast<uintptr_t>(&alpha_movie_unk6_00404270_redirect),
            OPENSMACX_SIGNATURE_00404270,
        },
        {
            0x00404280,
            reinterpret_cast<uintptr_t>(&alpha_movie_unk7_redirect),
            OPENSMACX_SIGNATURE_00404280,
        },
        {
            0x004042A0,
            reinterpret_cast<uintptr_t>(&alpha_movie_unk8_redirect),
            OPENSMACX_SIGNATURE_004042A0,
        },
        {
            0x00404900,
            reinterpret_cast<uintptr_t>(&popup_close_redirect),
            OPENSMACX_SIGNATURE_00404900,
        },
        {
            0x00405B10,
            reinterpret_cast<uintptr_t>(&popup_on_dialog_back_draw_redirect),
            OPENSMACX_SIGNATURE_00405B10,
        },
        {
            0x00408710,
            reinterpret_cast<uintptr_t>(&base_win_close_redirect),
            OPENSMACX_SIGNATURE_00408710,
        },
        {
            0x00416DD0,
            reinterpret_cast<uintptr_t>(&base_win_unk4_redirect),
            OPENSMACX_SIGNATURE_00416DD0,
        },
        {
            0x0041A610,
            reinterpret_cast<uintptr_t>(&base_win_unk5_redirect),
            OPENSMACX_SIGNATURE_0041A610,
        },
        {
            0x0041AF20,
            reinterpret_cast<uintptr_t>(&base_win_unk6_redirect),
            OPENSMACX_SIGNATURE_0041AF20,
        },
        {
            0x0041AF30,
            reinterpret_cast<uintptr_t>(&base_win_unk7_redirect),
            OPENSMACX_SIGNATURE_0041AF30,
        },
        {
            0x0041AF40,
            reinterpret_cast<uintptr_t>(&base_win_timer_callback_redirect),
            OPENSMACX_SIGNATURE_0041AF40,
        },
        {
            0x0041AF70,
            reinterpret_cast<uintptr_t>(&base_win_on_left_click_redirect),
            OPENSMACX_SIGNATURE_0041AF70,
        },
        {
            0x0041AF90,
            reinterpret_cast<uintptr_t>(&base_win_on_right_click_redirect),
            OPENSMACX_SIGNATURE_0041AF90,
        },
        {
            0x0041AFB0,
            reinterpret_cast<uintptr_t>(&base_win_on_left_double_click_redirect),
            OPENSMACX_SIGNATURE_0041AFB0,
        },
        {
            0x0041B4E0,
            reinterpret_cast<uintptr_t>(&base_win_on_button_toggled_redirect),
            OPENSMACX_SIGNATURE_0041B4E0,
        },
        {
            0x0041DC80,
            reinterpret_cast<uintptr_t>(&base_win_on_iface_scrolled_redirect),
            OPENSMACX_SIGNATURE_0041DC80,
        },
        {
            0x0041DCD0,
            reinterpret_cast<uintptr_t>(&base_win_on_iface_left_click_redirect),
            OPENSMACX_SIGNATURE_0041DCD0,
        },
        {
            0x0041DCF0,
            reinterpret_cast<uintptr_t>(&base_win_on_iface_right_click_redirect),
            OPENSMACX_SIGNATURE_0041DCF0,
        },
        {
            0x0041DF40,
            reinterpret_cast<uintptr_t>(&base_win_on_iface_right_down_redirect),
            OPENSMACX_SIGNATURE_0041DF40,
        },
        {
            0x0041E4C0,
            reinterpret_cast<uintptr_t>(&base_win_on_iface_selected_redirect),
            OPENSMACX_SIGNATURE_0041E4C0,
        },
        {
            0x0041E4D0,
            reinterpret_cast<uintptr_t>(&base_win_on_iface_left_double_click_redirect),
            OPENSMACX_SIGNATURE_0041E4D0,
        },
        {
            0x0041E4F0,
            reinterpret_cast<uintptr_t>(&base_win_on_iface_right_double_click_redirect),
            OPENSMACX_SIGNATURE_0041E4F0,
        },
        {
            0x00422E10,
            reinterpret_cast<uintptr_t>(&battle_win_on_iface_left_click_redirect),
            OPENSMACX_SIGNATURE_00422E10,
        },
        {
            0x00422E20,
            reinterpret_cast<uintptr_t>(&battle_win_on_iface_right_click_redirect),
            OPENSMACX_SIGNATURE_00422E20,
        },
        {
            0x00422E70,
            reinterpret_cast<uintptr_t>(&battle_win_on_iface_right_down_redirect),
            OPENSMACX_SIGNATURE_00422E70,
        },
        {
            0x00422E80,
            reinterpret_cast<uintptr_t>(&battle_win_on_iface_mouse_move_redirect),
            OPENSMACX_SIGNATURE_00422E80,
        },
        {
            0x00422E90,
            reinterpret_cast<uintptr_t>(&battle_win_on_iface_mouse_leave_redirect),
            OPENSMACX_SIGNATURE_00422E90,
        },
        {
            0x00422EA0,
            reinterpret_cast<uintptr_t>(&battle_win_on_iface_selected_redirect),
            OPENSMACX_SIGNATURE_00422EA0,
        },
        {
            0x00426EB0,
            reinterpret_cast<uintptr_t>(&counc_win_on_group_clicked_redirect),
            OPENSMACX_SIGNATURE_00426EB0,
        },
        {
            0x00426F80,
            reinterpret_cast<uintptr_t>(&counc_win_on_button_passover_redirect),
            OPENSMACX_SIGNATURE_00426F80,
        },
        {
            0x00427060,
            reinterpret_cast<uintptr_t>(&counc_win_on_iface_left_click_redirect),
            OPENSMACX_SIGNATURE_00427060,
        },
        {
            0x00427070,
            reinterpret_cast<uintptr_t>(&counc_win_on_iface_right_click_redirect),
            OPENSMACX_SIGNATURE_00427070,
        },
        {
            0x00427080,
            reinterpret_cast<uintptr_t>(&counc_win_on_iface_left_down_redirect),
            OPENSMACX_SIGNATURE_00427080,
        },
        {
            0x00427090,
            reinterpret_cast<uintptr_t>(&counc_win_on_iface_right_down_redirect),
            OPENSMACX_SIGNATURE_00427090,
        },
        {
            0x004270A0,
            reinterpret_cast<uintptr_t>(&counc_win_on_iface_mouse_move_redirect),
            OPENSMACX_SIGNATURE_004270A0,
        },
        {
            0x004270B0,
            reinterpret_cast<uintptr_t>(&counc_win_on_iface_mouse_leave_redirect),
            OPENSMACX_SIGNATURE_004270B0,
        },
        {
            0x004270C0,
            reinterpret_cast<uintptr_t>(&counc_win_on_iface_selected_redirect),
            OPENSMACX_SIGNATURE_004270C0,
        },
        {
            0x004270D0,
            reinterpret_cast<uintptr_t>(&counc_win_on_iface_left_double_click_redirect),
            OPENSMACX_SIGNATURE_004270D0,
        },
        {
            0x004270E0,
            reinterpret_cast<uintptr_t>(&counc_win_on_iface_right_double_click_redirect),
            OPENSMACX_SIGNATURE_004270E0,
        },
        {
            0x00427110,
            reinterpret_cast<uintptr_t>(&counc_win_on_iface_button_toggled_redirect),
            OPENSMACX_SIGNATURE_00427110,
        },
        {
            0x00427120,
            reinterpret_cast<uintptr_t>(&counc_win_on_iface_group_clicked_redirect),
            OPENSMACX_SIGNATURE_00427120,
        },
        {
            0x00428D80,
            reinterpret_cast<uintptr_t>(&credits_on_left_down_redirect),
            OPENSMACX_SIGNATURE_00428D80,
        },
        {
            0x00428D90,
            reinterpret_cast<uintptr_t>(&credits_on_right_down_redirect),
            OPENSMACX_SIGNATURE_00428D90,
        },
        {
            0x00428DA0,
            reinterpret_cast<uintptr_t>(&credits_on_mouse_move_redirect),
            OPENSMACX_SIGNATURE_00428DA0,
        },
        {
            0x00428DB0,
            reinterpret_cast<uintptr_t>(&credits_on_button_clicked_redirect),
            OPENSMACX_SIGNATURE_00428DB0,
        },
        {
            0x0042BD20,
            reinterpret_cast<uintptr_t>(&datalink_close_redirect),
            OPENSMACX_SIGNATURE_0042BD20,
        },
        {
            0x0042BEA0,
            reinterpret_cast<uintptr_t>(&datalink_unk6_redirect),
            OPENSMACX_SIGNATURE_0042BEA0,
        },
        {
            0x0042BF00,
            reinterpret_cast<uintptr_t>(&datalink_unk8_redirect),
            OPENSMACX_SIGNATURE_0042BF00,
        },
        {
            0x00431DB0,
            reinterpret_cast<uintptr_t>(&datalink_unk9_redirect),
            OPENSMACX_SIGNATURE_00431DB0,
        },
        {
            0x00431EB0,
            reinterpret_cast<uintptr_t>(&datalink_on_left_click_redirect),
            OPENSMACX_SIGNATURE_00431EB0,
        },
        {
            0x00431FB0,
            reinterpret_cast<uintptr_t>(&datalink_on_group_clicked_redirect),
            OPENSMACX_SIGNATURE_00431FB0,
        },
        {
            0x00432000,
            reinterpret_cast<uintptr_t>(&datalink_on_mouse_move_redirect),
            OPENSMACX_SIGNATURE_00432000,
        },
        {
            0x00432160,
            reinterpret_cast<uintptr_t>(&datalink_on_iface_left_click_redirect),
            OPENSMACX_SIGNATURE_00432160,
        },
        {
            0x00432170,
            reinterpret_cast<uintptr_t>(&datalink_on_iface_right_click_redirect),
            OPENSMACX_SIGNATURE_00432170,
        },
        {
            0x00432180,
            reinterpret_cast<uintptr_t>(&datalink_on_iface_left_down_redirect),
            OPENSMACX_SIGNATURE_00432180,
        },
        {
            0x00432190,
            reinterpret_cast<uintptr_t>(&datalink_on_iface_right_down_redirect),
            OPENSMACX_SIGNATURE_00432190,
        },
        {
            0x004321A0,
            reinterpret_cast<uintptr_t>(&datalink_on_iface_mouse_move_redirect),
            OPENSMACX_SIGNATURE_004321A0,
        },
        {
            0x004321B0,
            reinterpret_cast<uintptr_t>(&datalink_on_iface_mouse_leave_redirect),
            OPENSMACX_SIGNATURE_004321B0,
        },
        {
            0x004321C0,
            reinterpret_cast<uintptr_t>(&datalink_on_iface_selected_redirect),
            OPENSMACX_SIGNATURE_004321C0,
        },
        {
            0x004321D0,
            reinterpret_cast<uintptr_t>(&datalink_on_iface_left_double_click_redirect),
            OPENSMACX_SIGNATURE_004321D0,
        },
        {
            0x004321E0,
            reinterpret_cast<uintptr_t>(&datalink_on_iface_right_double_click_redirect),
            OPENSMACX_SIGNATURE_004321E0,
        },
        {
            0x004321F0,
            reinterpret_cast<uintptr_t>(&datalink_on_iface_button_clicked_redirect),
            OPENSMACX_SIGNATURE_004321F0,
        },
        {
            0x00432200,
            reinterpret_cast<uintptr_t>(&datalink_on_iface_button_toggled_redirect),
            OPENSMACX_SIGNATURE_00432200,
        },
        {
            0x004345C0,
            reinterpret_cast<uintptr_t>(&design_win_unk1_redirect),
            OPENSMACX_SIGNATURE_004345C0,
        },
        {
            0x0043D330,
            reinterpret_cast<uintptr_t>(&design_win_on_iface_left_down_redirect),
            OPENSMACX_SIGNATURE_0043D330,
        },
        {
            0x0043D340,
            reinterpret_cast<uintptr_t>(&design_win_on_iface_right_down_redirect),
            OPENSMACX_SIGNATURE_0043D340,
        },
        {
            0x0043D400,
            reinterpret_cast<uintptr_t>(&design_win_on_iface_selected_redirect),
            OPENSMACX_SIGNATURE_0043D400,
        },
        {
            0x0043D410,
            reinterpret_cast<uintptr_t>(&design_win_on_iface_left_double_click_redirect),
            OPENSMACX_SIGNATURE_0043D410,
        },
        {
            0x0043D420,
            reinterpret_cast<uintptr_t>(&design_win_on_iface_right_double_click_redirect),
            OPENSMACX_SIGNATURE_0043D420,
        },
        {
            0x0043D470,
            reinterpret_cast<uintptr_t>(&design_win_on_iface_button_toggled_redirect),
            OPENSMACX_SIGNATURE_0043D470,
        },
        {
            0x0043D480,
            reinterpret_cast<uintptr_t>(&design_win_on_iface_group_clicked_redirect),
            OPENSMACX_SIGNATURE_0043D480,
        },
        {
            0x0043EF70,
            reinterpret_cast<uintptr_t>(&popup_start_redirect),
            OPENSMACX_SIGNATURE_0043EF70,
        },
        {
            0x004404C0,
            reinterpret_cast<uintptr_t>(&diplo_pop_on_iface_left_click_redirect),
            OPENSMACX_SIGNATURE_004404C0,
        },
        {
            0x004404D0,
            reinterpret_cast<uintptr_t>(&diplo_pop_on_iface_right_click_redirect),
            OPENSMACX_SIGNATURE_004404D0,
        },
        {
            0x004404E0,
            reinterpret_cast<uintptr_t>(&diplo_pop_on_iface_left_down_redirect),
            OPENSMACX_SIGNATURE_004404E0,
        },
        {
            0x004404F0,
            reinterpret_cast<uintptr_t>(&diplo_pop_on_iface_right_down_redirect),
            OPENSMACX_SIGNATURE_004404F0,
        },
        {
            0x00440500,
            reinterpret_cast<uintptr_t>(&diplo_pop_on_iface_mouse_move_redirect),
            OPENSMACX_SIGNATURE_00440500,
        },
        {
            0x00440510,
            reinterpret_cast<uintptr_t>(&diplo_pop_on_iface_mouse_leave_redirect),
            OPENSMACX_SIGNATURE_00440510,
        },
        {
            0x00440520,
            reinterpret_cast<uintptr_t>(&diplo_pop_on_iface_selected_redirect),
            OPENSMACX_SIGNATURE_00440520,
        },
        {
            0x00440530,
            reinterpret_cast<uintptr_t>(&diplo_pop_on_iface_left_double_click_redirect),
            OPENSMACX_SIGNATURE_00440530,
        },
        {
            0x00440540,
            reinterpret_cast<uintptr_t>(&diplo_pop_on_iface_right_double_click_redirect),
            OPENSMACX_SIGNATURE_00440540,
        },
        {
            0x00440610,
            reinterpret_cast<uintptr_t>(&diplo_pop_on_iface_button_toggled_redirect),
            OPENSMACX_SIGNATURE_00440610,
        },
        {
            0x004413B0,
            reinterpret_cast<uintptr_t>(&diplo_win_unk2_redirect),
            OPENSMACX_SIGNATURE_004413B0,
        },
        {
            0x004429F0,
            reinterpret_cast<uintptr_t>(&diplo_win_unk4_redirect),
            OPENSMACX_SIGNATURE_004429F0,
        },
        {
            0x00442A00,
            reinterpret_cast<uintptr_t>(&diplo_win_unk5_redirect),
            OPENSMACX_SIGNATURE_00442A00,
        },
        {
            0x00443D80,
            reinterpret_cast<uintptr_t>(&diplo_win_on_iface_left_click_redirect),
            OPENSMACX_SIGNATURE_00443D80,
        },
        {
            0x00443D90,
            reinterpret_cast<uintptr_t>(&diplo_win_on_iface_left_up_redirect),
            OPENSMACX_SIGNATURE_00443D90,
        },
        {
            0x00443DA0,
            reinterpret_cast<uintptr_t>(&diplo_win_on_iface_right_click_redirect),
            OPENSMACX_SIGNATURE_00443DA0,
        },
        {
            0x00443DB0,
            reinterpret_cast<uintptr_t>(&diplo_win_on_iface_left_down_redirect),
            OPENSMACX_SIGNATURE_00443DB0,
        },
        {
            0x00443DC0,
            reinterpret_cast<uintptr_t>(&diplo_win_on_iface_right_down_redirect),
            OPENSMACX_SIGNATURE_00443DC0,
        },
        {
            0x00443DD0,
            reinterpret_cast<uintptr_t>(&diplo_win_on_iface_mouse_move_redirect),
            OPENSMACX_SIGNATURE_00443DD0,
        },
        {
            0x00443DE0,
            reinterpret_cast<uintptr_t>(&diplo_win_on_iface_mouse_leave_redirect),
            OPENSMACX_SIGNATURE_00443DE0,
        },
        {
            0x00443DF0,
            reinterpret_cast<uintptr_t>(&diplo_win_on_iface_selected_redirect),
            OPENSMACX_SIGNATURE_00443DF0,
        },
        {
            0x00443E00,
            reinterpret_cast<uintptr_t>(&diplo_win_on_iface_left_double_click_redirect),
            OPENSMACX_SIGNATURE_00443E00,
        },
        {
            0x00443E10,
            reinterpret_cast<uintptr_t>(&diplo_win_on_iface_right_double_click_redirect),
            OPENSMACX_SIGNATURE_00443E10,
        },
        {
            0x00443E20,
            reinterpret_cast<uintptr_t>(&diplo_win_on_iface_button_clicked_redirect),
            OPENSMACX_SIGNATURE_00443E20,
        },
        {
            0x00443E30,
            reinterpret_cast<uintptr_t>(&diplo_win_on_iface_button_toggled_redirect),
            OPENSMACX_SIGNATURE_00443E30,
        },
        {
            0x00443E40,
            reinterpret_cast<uintptr_t>(&diplo_win_on_iface_dialog_back_draw_redirect),
            OPENSMACX_SIGNATURE_00443E40,
        },
        {
            0x00447220,
            reinterpret_cast<uintptr_t>(&faction_ambience_begin_redirect),
            OPENSMACX_SIGNATURE_00447220,
        },
        {
            0x00447230,
            reinterpret_cast<uintptr_t>(&faction_ambience_tech_redirect),
            OPENSMACX_SIGNATURE_00447230,
        },
        {
            0x00447240,
            reinterpret_cast<uintptr_t>(&faction_ambience_terraform_redirect),
            OPENSMACX_SIGNATURE_00447240,
        },
        {
            0x00447250,
            reinterpret_cast<uintptr_t>(&faction_ambience_production_redirect),
            OPENSMACX_SIGNATURE_00447250,
        },
        {
            0x00447260,
            reinterpret_cast<uintptr_t>(&faction_ambience_general_redirect),
            OPENSMACX_SIGNATURE_00447260,
        },
        {
            0x004472B0,
            reinterpret_cast<uintptr_t>(&faction_ambience_new_base_redirect),
            OPENSMACX_SIGNATURE_004472B0,
        },
        {
            0x004472C0,
            reinterpret_cast<uintptr_t>(&faction_ambience_popup1_redirect),
            OPENSMACX_SIGNATURE_004472C0,
        },
        {
            0x004472D0,
            reinterpret_cast<uintptr_t>(&faction_ambience_eot_redirect),
            OPENSMACX_SIGNATURE_004472D0,
        },
        {
            0x004472E0,
            reinterpret_cast<uintptr_t>(&faction_ambience_hostility_redirect),
            OPENSMACX_SIGNATURE_004472E0,
        },
        {
            0x004472F0,
            reinterpret_cast<uintptr_t>(&faction_ambience_energy_resources_redirect),
            OPENSMACX_SIGNATURE_004472F0,
        },
        {
            0x00447300,
            reinterpret_cast<uintptr_t>(&faction_ambience_base_liberated_redirect),
            OPENSMACX_SIGNATURE_00447300,
        },
        {
            0x00447690,
            reinterpret_cast<uintptr_t>(&u_ambience_tech_redirect),
            OPENSMACX_SIGNATURE_00447690,
        },
        {
            0x004476A0,
            reinterpret_cast<uintptr_t>(&u_ambience_popup1_redirect),
            OPENSMACX_SIGNATURE_004476A0,
        },
        {
            0x004476B0,
            reinterpret_cast<uintptr_t>(&u_ambience_eot_redirect),
            OPENSMACX_SIGNATURE_004476B0,
        },
        {
            0x00447C60,
            reinterpret_cast<uintptr_t>(&g_ambience_tech_redirect),
            OPENSMACX_SIGNATURE_00447C60,
        },
        {
            0x00447CA0,
            reinterpret_cast<uintptr_t>(&g_ambience_production_redirect),
            OPENSMACX_SIGNATURE_00447CA0,
        },
        {
            0x00447CB0,
            reinterpret_cast<uintptr_t>(&g_ambience_popup1_redirect),
            OPENSMACX_SIGNATURE_00447CB0,
        },
        {
            0x00447CC0,
            reinterpret_cast<uintptr_t>(&g_ambience_eot_redirect),
            OPENSMACX_SIGNATURE_00447CC0,
        },
        {
            0x004562F0,
            reinterpret_cast<uintptr_t>(&info_win_unk1_redirect),
            OPENSMACX_SIGNATURE_004562F0,
        },
        {
            0x00458900,
            reinterpret_cast<uintptr_t>(&info_win_change_redirect),
            OPENSMACX_SIGNATURE_00458900,
        },
        {
            0x00458910,
            reinterpret_cast<uintptr_t>(&info_win_on_redraw_redirect),
            OPENSMACX_SIGNATURE_00458910,
        },
        {
            0x00458920,
            reinterpret_cast<uintptr_t>(&info_win_unk3_redirect),
            OPENSMACX_SIGNATURE_00458920,
        },
        {
            0x00459280,
            reinterpret_cast<uintptr_t>(&info_win_reset_redirect),
            OPENSMACX_SIGNATURE_00459280,
        },
        {
            0x0045C280,
            reinterpret_cast<uintptr_t>(&main_interface_unk1_redirect),
            OPENSMACX_SIGNATURE_0045C280,
        },
        {
            0x0045C5B0,
            reinterpret_cast<uintptr_t>(&main_interface_unk2_redirect),
            OPENSMACX_SIGNATURE_0045C5B0,
        },
        {
            0x0045CC80,
            reinterpret_cast<uintptr_t>(&main_interface_on_iface_scrolled_redirect),
            OPENSMACX_SIGNATURE_0045CC80,
        },
        {
            0x0045CC90,
            reinterpret_cast<uintptr_t>(&main_interface_on_iface_scrolling_redirect),
            OPENSMACX_SIGNATURE_0045CC90,
        },
        {
            0x0045CCA0,
            reinterpret_cast<uintptr_t>(&main_interface_on_iface_button_toggled_redirect),
            OPENSMACX_SIGNATURE_0045CCA0,
        },
        {
            0x0045CDB0,
            reinterpret_cast<uintptr_t>(&main_interface_on_iface_right_double_click_redirect),
            OPENSMACX_SIGNATURE_0045CDB0,
        },
        {
            0x0045D160,
            reinterpret_cast<uintptr_t>(&main_interface_on_iface_button_pressed_redirect),
            OPENSMACX_SIGNATURE_0045D160,
        },
        {
            0x0045EB60,
            reinterpret_cast<uintptr_t>(&main_interface_unk3_redirect),
            OPENSMACX_SIGNATURE_0045EB60,
        },
        {
            0x0045EB70,
            reinterpret_cast<uintptr_t>(&main_interface_unk4_redirect),
            OPENSMACX_SIGNATURE_0045EB70,
        },
        {
            0x00462970,
            reinterpret_cast<uintptr_t>(&map_win_unk2_redirect),
            OPENSMACX_SIGNATURE_00462970,
        },
        {
            0x00467960,
            reinterpret_cast<uintptr_t>(&map_win_unk3_redirect),
            OPENSMACX_SIGNATURE_00467960,
        },
        {
            0x0046EBA0,
            reinterpret_cast<uintptr_t>(&map_win_on_left_click_redirect),
            OPENSMACX_SIGNATURE_0046EBA0,
        },
        {
            0x0046EBD0,
            reinterpret_cast<uintptr_t>(&map_win_on_left_double_click_redirect),
            OPENSMACX_SIGNATURE_0046EBD0,
        },
        {
            0x0046EBE0,
            reinterpret_cast<uintptr_t>(&map_win_on_right_click_redirect),
            OPENSMACX_SIGNATURE_0046EBE0,
        },
        {
            0x0046EFF0,
            reinterpret_cast<uintptr_t>(&map_win_on_left_up_redirect),
            OPENSMACX_SIGNATURE_0046EFF0,
        },
        {
            0x0046FB10,
            reinterpret_cast<uintptr_t>(&map_win_main_caption_redirect),
            OPENSMACX_SIGNATURE_0046FB10,
        },
        {
            0x0046FB80,
            reinterpret_cast<uintptr_t>(&map_win_do_image_buttons_redirect),
            OPENSMACX_SIGNATURE_0046FB80,
        },
        {
            0x00470F70,
            reinterpret_cast<uintptr_t>(&map_win_close_redirect),
            OPENSMACX_SIGNATURE_00470F70,
        },
        {
            0x004713A0,
            reinterpret_cast<uintptr_t>(&message_win_unk1_redirect),
            OPENSMACX_SIGNATURE_004713A0,
        },
        {
            0x00471D40,
            reinterpret_cast<uintptr_t>(&message_win_unk4_redirect),
            OPENSMACX_SIGNATURE_00471D40,
        },
        {
            0x0047A7D0,
            reinterpret_cast<uintptr_t>(&net_msg_close_redirect),
            OPENSMACX_SIGNATURE_0047A7D0,
        },
        {
            0x0047B750,
            reinterpret_cast<uintptr_t>(&net_win_on_key_click_redirect),
            OPENSMACX_SIGNATURE_0047B750,
        },
        {
            0x0047F920,
            reinterpret_cast<uintptr_t>(&net_win_unk1_redirect),
            OPENSMACX_SIGNATURE_0047F920,
        },
        {
            0x0047F930,
            reinterpret_cast<uintptr_t>(&net_win_unk2_redirect),
            OPENSMACX_SIGNATURE_0047F930,
        },
        {
            0x0047F940,
            reinterpret_cast<uintptr_t>(&net_win_on_mouse_move_redirect),
            OPENSMACX_SIGNATURE_0047F940,
        },
        {
            0x0047F950,
            reinterpret_cast<uintptr_t>(&net_win_on_mouse_leave_redirect),
            OPENSMACX_SIGNATURE_0047F950,
        },
        {
            0x004834D0,
            reinterpret_cast<uintptr_t>(&net_win_alloc_slots_redirect),
            OPENSMACX_SIGNATURE_004834D0,
        },
        {
            0x00483E90,
            reinterpret_cast<uintptr_t>(&new_tech_win_on_left_click_redirect),
            OPENSMACX_SIGNATURE_00483E90,
        },
        {
            0x00483EA0,
            reinterpret_cast<uintptr_t>(&new_tech_win_on_key_click_redirect),
            OPENSMACX_SIGNATURE_00483EA0,
        },
        {
            0x00489B10,
            reinterpret_cast<uintptr_t>(&pick_win_on_button_passover_redirect),
            OPENSMACX_SIGNATURE_00489B10,
        },
        {
            0x0048AF20,
            reinterpret_cast<uintptr_t>(&plan_win_clear_lines_redirect),
            OPENSMACX_SIGNATURE_0048AF20,
        },
        {
            0x00493C10,
            reinterpret_cast<uintptr_t>(&prod_picker_unk1_redirect),
            OPENSMACX_SIGNATURE_00493C10,
        },
        {
            0x00493E90,
            reinterpret_cast<uintptr_t>(&prod_picker_on_mouse_move_redirect),
            OPENSMACX_SIGNATURE_00493E90,
        },
        {
            0x00495160,
            reinterpret_cast<uintptr_t>(&prod_picker_unk3_redirect),
            OPENSMACX_SIGNATURE_00495160,
        },
        {
            0x004980F0,
            reinterpret_cast<uintptr_t>(&report_win_on_mouse_move_redirect),
            OPENSMACX_SIGNATURE_004980F0,
        },
        {
            0x00498100,
            reinterpret_cast<uintptr_t>(&report_win_on_mouse_leave_redirect),
            OPENSMACX_SIGNATURE_00498100,
        },
        {
            0x004A5FE0,
            reinterpret_cast<uintptr_t>(&report_if_on_iface_right_click_redirect),
            OPENSMACX_SIGNATURE_004A5FE0,
        },
        {
            0x004A5FF0,
            reinterpret_cast<uintptr_t>(&report_if_on_iface_left_double_click_redirect),
            OPENSMACX_SIGNATURE_004A5FF0,
        },
        {
            0x004A6000,
            reinterpret_cast<uintptr_t>(&report_if_on_iface_right_double_click_redirect),
            OPENSMACX_SIGNATURE_004A6000,
        },
        {
            0x004A6010,
            reinterpret_cast<uintptr_t>(&report_if_on_iface_left_down_redirect),
            OPENSMACX_SIGNATURE_004A6010,
        },
        {
            0x004A6020,
            reinterpret_cast<uintptr_t>(&report_if_on_iface_right_down_redirect),
            OPENSMACX_SIGNATURE_004A6020,
        },
        {
            0x004A6030,
            reinterpret_cast<uintptr_t>(&report_if_on_iface_mouse_move_redirect),
            OPENSMACX_SIGNATURE_004A6030,
        },
        {
            0x004A6040,
            reinterpret_cast<uintptr_t>(&report_if_on_iface_mouse_leave_redirect),
            OPENSMACX_SIGNATURE_004A6040,
        },
        {
            0x004A6D60,
            reinterpret_cast<uintptr_t>(&report_if_on_iface_button_toggled_redirect),
            OPENSMACX_SIGNATURE_004A6D60,
        },
        {
            0x004AC130,
            reinterpret_cast<uintptr_t>(&report_if_close_score_redirect),
            OPENSMACX_SIGNATURE_004AC130,
        },
        {
            0x004ADAD0,
            reinterpret_cast<uintptr_t>(&setup_win_unk3_redirect),
            OPENSMACX_SIGNATURE_004ADAD0,
        },
        {
            0x004ADAE0,
            reinterpret_cast<uintptr_t>(&setup_win_unk4_redirect),
            OPENSMACX_SIGNATURE_004ADAE0,
        },
        {
            0x004AEF00,
            reinterpret_cast<uintptr_t>(&social_win_unk2_redirect),
            OPENSMACX_SIGNATURE_004AEF00,
        },
        {
            0x004B1790,
            reinterpret_cast<uintptr_t>(&social_win_unk3_redirect),
            OPENSMACX_SIGNATURE_004B1790,
        },
        {
            0x004B2400,
            reinterpret_cast<uintptr_t>(&social_win_on_iface_left_down_redirect),
            OPENSMACX_SIGNATURE_004B2400,
        },
        {
            0x004B2410,
            reinterpret_cast<uintptr_t>(&social_win_on_iface_right_down_redirect),
            OPENSMACX_SIGNATURE_004B2410,
        },
        {
            0x004B2440,
            reinterpret_cast<uintptr_t>(&social_win_on_iface_selected_redirect),
            OPENSMACX_SIGNATURE_004B2440,
        },
        {
            0x004B2450,
            reinterpret_cast<uintptr_t>(&social_win_on_iface_left_double_click_redirect),
            OPENSMACX_SIGNATURE_004B2450,
        },
        {
            0x004B2460,
            reinterpret_cast<uintptr_t>(&social_win_on_iface_right_double_click_redirect),
            OPENSMACX_SIGNATURE_004B2460,
        },
        {
            0x004B2670,
            reinterpret_cast<uintptr_t>(&social_win_on_iface_button_toggled_redirect),
            OPENSMACX_SIGNATURE_004B2670,
        },
        {
            0x004B2680,
            reinterpret_cast<uintptr_t>(&social_win_on_iface_group_clicked_redirect),
            OPENSMACX_SIGNATURE_004B2680,
        },
        {
            0x004B8970,
            reinterpret_cast<uintptr_t>(&status_win_reset_redirect),
            OPENSMACX_SIGNATURE_004B8970,
        },
        {
            0x004B9F80,
            reinterpret_cast<uintptr_t>(&status_win_close_redirect),
            OPENSMACX_SIGNATURE_004B9F80,
        },
        {
            0x004B9F90,
            reinterpret_cast<uintptr_t>(&status_win_set_loc_redirect),
            OPENSMACX_SIGNATURE_004B9F90,
        },
        {
            0x004BA720,
            reinterpret_cast<uintptr_t>(&tut_win_unk1_redirect),
            OPENSMACX_SIGNATURE_004BA720,
        },
        {
            0x004BDDD0,
            reinterpret_cast<uintptr_t>(&tut_win_unk3_redirect),
            OPENSMACX_SIGNATURE_004BDDD0,
        },
        {
            0x004C3E50,
            reinterpret_cast<uintptr_t>(&world_win_on_left_click_redirect),
            OPENSMACX_SIGNATURE_004C3E50,
        },
        {
            0x004C3E70,
            reinterpret_cast<uintptr_t>(&world_win_on_right_click_redirect),
            OPENSMACX_SIGNATURE_004C3E70,
        },
        {
            0x004C3E90,
            reinterpret_cast<uintptr_t>(&world_win_on_left_double_click_redirect),
            OPENSMACX_SIGNATURE_004C3E90,
        },
        {
            0x004C45E0,
            reinterpret_cast<uintptr_t>(&world_win_clear_terrain_redirect),
            OPENSMACX_SIGNATURE_004C45E0,
        },
        {
            0x004C4FC0,
            reinterpret_cast<uintptr_t>(&wave_device_suspend_redirect),
            OPENSMACX_SIGNATURE_004C4FC0,
        },
        {
            0x004C4FD0,
            reinterpret_cast<uintptr_t>(&wave_device_restart_redirect),
            OPENSMACX_SIGNATURE_004C4FD0,
        },
        {
            0x004C4FE0,
            reinterpret_cast<uintptr_t>(&wave_device_update_sound_redirect),
            OPENSMACX_SIGNATURE_004C4FE0,
        },
        {
            0x004C4FF0,
            reinterpret_cast<uintptr_t>(&wave_device_get_ndevices_redirect),
            OPENSMACX_SIGNATURE_004C4FF0,
        },
        {
            0x004C5000,
            reinterpret_cast<uintptr_t>(&wave_device_set_hwnd_redirect),
            OPENSMACX_SIGNATURE_004C5000,
        },
        {
            0x004C5020,
            reinterpret_cast<uintptr_t>(&wave_device_get_hw_mem_size_redirect),
            OPENSMACX_SIGNATURE_004C5020,
        },
        {
            0x004C5120,
            reinterpret_cast<uintptr_t>(&wave_device_set_rate_redirect),
            OPENSMACX_SIGNATURE_004C5120,
        },
        {
            0x004C5140,
            reinterpret_cast<uintptr_t>(&wave_device_get_rate_redirect),
            OPENSMACX_SIGNATURE_004C5140,
        },
        {
            0x004C5150,
            reinterpret_cast<uintptr_t>(&wave_device_set_volume_redirect),
            OPENSMACX_SIGNATURE_004C5150,
        },
        {
            0x004C5170,
            reinterpret_cast<uintptr_t>(&wave_device_get_volume_redirect),
            OPENSMACX_SIGNATURE_004C5170,
        },
        {
            0x004C5180,
            reinterpret_cast<uintptr_t>(&wave_device_set_pan_redirect),
            OPENSMACX_SIGNATURE_004C5180,
        },
        {
            0x004C5190,
            reinterpret_cast<uintptr_t>(&wave_device_get_ds_redirect),
            OPENSMACX_SIGNATURE_004C5190,
        },
        {
            0x004C51A0,
            reinterpret_cast<uintptr_t>(&wave_device_fade_redirect),
            OPENSMACX_SIGNATURE_004C51A0,
        },
        {
            0x004C51B0,
            reinterpret_cast<uintptr_t>(&wave_device_stop_redirect),
            OPENSMACX_SIGNATURE_004C51B0,
        },
        {
            0x004C51C0,
            reinterpret_cast<uintptr_t>(&wave_device_enable_redirect),
            OPENSMACX_SIGNATURE_004C51C0,
        },
        {
            0x004C51D0,
            reinterpret_cast<uintptr_t>(&wave_device_disable_redirect),
            OPENSMACX_SIGNATURE_004C51D0,
        },
        {
            0x004C51E0,
            reinterpret_cast<uintptr_t>(&wave_device_is_disabled_redirect),
            OPENSMACX_SIGNATURE_004C51E0,
        },
        {
            0x004C5220,
            reinterpret_cast<uintptr_t>(&wave_device_stop_raw_dump_redirect),
            OPENSMACX_SIGNATURE_004C5220,
        },
        {
            0x004C5380,
            reinterpret_cast<uintptr_t>(&wave_device_get_group_volume_redirect),
            OPENSMACX_SIGNATURE_004C5380,
        },
        {
            0x004C54B0,
            reinterpret_cast<uintptr_t>(&wave_device_is_eax_redirect),
            OPENSMACX_SIGNATURE_004C54B0,
        },
        {
            0x004C5530,
            reinterpret_cast<uintptr_t>(&wave_device_is_3d_redirect),
            OPENSMACX_SIGNATURE_004C5530,
        },
        {
            0x004C5830,
            reinterpret_cast<uintptr_t>(&midi_device_get_ndevices_redirect),
            OPENSMACX_SIGNATURE_004C5830,
        },
        {
            0x004C5840,
            reinterpret_cast<uintptr_t>(&midi_device_select_redirect),
            OPENSMACX_SIGNATURE_004C5840,
        },
        {
            0x004C5850,
            reinterpret_cast<uintptr_t>(&midi_device_get_description_redirect),
            OPENSMACX_SIGNATURE_004C5850,
        },
        {
            0x004C5860,
            reinterpret_cast<uintptr_t>(&midi_device_set_volume_redirect),
            OPENSMACX_SIGNATURE_004C5860,
        },
        {
            0x004C5870,
            reinterpret_cast<uintptr_t>(&midi_device_get_volume_redirect),
            OPENSMACX_SIGNATURE_004C5870,
        },
        {
            0x004C5880,
            reinterpret_cast<uintptr_t>(&midi_device_set_pan_redirect),
            OPENSMACX_SIGNATURE_004C5880,
        },
        {
            0x004C5890,
            reinterpret_cast<uintptr_t>(&midi_device_fade_redirect),
            OPENSMACX_SIGNATURE_004C5890,
        },
        {
            0x004C58A0,
            reinterpret_cast<uintptr_t>(&midi_device_stop_redirect),
            OPENSMACX_SIGNATURE_004C58A0,
        },
        {
            0x004C58B0,
            reinterpret_cast<uintptr_t>(&midi_device_update_sound_redirect),
            OPENSMACX_SIGNATURE_004C58B0,
        },
        {
            0x004C58C0,
            reinterpret_cast<uintptr_t>(&midi_device_set_rate_redirect),
            OPENSMACX_SIGNATURE_004C58C0,
        },
        {
            0x004C58D0,
            reinterpret_cast<uintptr_t>(&midi_device_get_rate_redirect),
            OPENSMACX_SIGNATURE_004C58D0,
        },
        {
            0x004C58E0,
            reinterpret_cast<uintptr_t>(&midi_device_suspend_redirect),
            OPENSMACX_SIGNATURE_004C58E0,
        },
        {
            0x004C58F0,
            reinterpret_cast<uintptr_t>(&midi_device_restart_redirect),
            OPENSMACX_SIGNATURE_004C58F0,
        },
        {
            0x004C5900,
            reinterpret_cast<uintptr_t>(&midi_device_enable_redirect),
            OPENSMACX_SIGNATURE_004C5900,
        },
        {
            0x004C5910,
            reinterpret_cast<uintptr_t>(&midi_device_disable_redirect),
            OPENSMACX_SIGNATURE_004C5910,
        },
        {
            0x004C5920,
            reinterpret_cast<uintptr_t>(&midi_device_is_disabled_redirect),
            OPENSMACX_SIGNATURE_004C5920,
        },
        {
            0x004C5AC0,
            reinterpret_cast<uintptr_t>(&wave_in_device_get_ndevices_redirect),
            OPENSMACX_SIGNATURE_004C5AC0,
        },
        {
            0x004C5AD0,
            reinterpret_cast<uintptr_t>(&wave_in_device_select_redirect),
            OPENSMACX_SIGNATURE_004C5AD0,
        },
        {
            0x004C5AE0,
            reinterpret_cast<uintptr_t>(&wave_in_device_get_description_redirect),
            OPENSMACX_SIGNATURE_004C5AE0,
        },
        {
            0x004C5AF0,
            reinterpret_cast<uintptr_t>(&wave_in_device_get_caps_redirect),
            OPENSMACX_SIGNATURE_004C5AF0,
        },
        {
            0x004C5B00,
            reinterpret_cast<uintptr_t>(&wave_in_device_stop_redirect),
            OPENSMACX_SIGNATURE_004C5B00,
        },
        {
            0x004C5B10,
            reinterpret_cast<uintptr_t>(&wave_in_device_update_sound_redirect),
            OPENSMACX_SIGNATURE_004C5B10,
        },
        {
            0x004C5B20,
            reinterpret_cast<uintptr_t>(&wave_in_device_set_rate_redirect),
            OPENSMACX_SIGNATURE_004C5B20,
        },
        {
            0x004C5B30,
            reinterpret_cast<uintptr_t>(&wave_in_device_get_rate_redirect),
            OPENSMACX_SIGNATURE_004C5B30,
        },
        {
            0x004C5B40,
            reinterpret_cast<uintptr_t>(&wave_in_device_suspend_redirect),
            OPENSMACX_SIGNATURE_004C5B40,
        },
        {
            0x004C5B50,
            reinterpret_cast<uintptr_t>(&wave_in_device_restart_redirect),
            OPENSMACX_SIGNATURE_004C5B50,
        },
        {
            0x004C5B60,
            reinterpret_cast<uintptr_t>(&wave_in_device_start_record_redirect),
            OPENSMACX_SIGNATURE_004C5B60,
        },
        {
            0x004C5B70,
            reinterpret_cast<uintptr_t>(&wave_in_device_end_record_redirect),
            OPENSMACX_SIGNATURE_004C5B70,
        },
        {
            0x004C6430,
            reinterpret_cast<uintptr_t>(&sound_unk1_redirect),
            OPENSMACX_SIGNATURE_004C6430,
        },
        {
            0x004C6480,
            reinterpret_cast<uintptr_t>(&sound_play_redirect),
            OPENSMACX_SIGNATURE_004C6480,
        },
        {
            0x004C64A0,
            reinterpret_cast<uintptr_t>(&sound_play_arg_redirect),
            OPENSMACX_SIGNATURE_004C64A0,
        },
        {
            0x004C64C0,
            reinterpret_cast<uintptr_t>(&sound_is_playing_redirect),
            OPENSMACX_SIGNATURE_004C64C0,
        },
        {
            0x004C64D0,
            reinterpret_cast<uintptr_t>(&sound_stop_redirect),
            OPENSMACX_SIGNATURE_004C64D0,
        },
        {
            0x004C64F0,
            reinterpret_cast<uintptr_t>(&sound_release_redirect),
            OPENSMACX_SIGNATURE_004C64F0,
        },
        {
            0x004C6540,
            reinterpret_cast<uintptr_t>(&sound_set_loop_state_redirect),
            OPENSMACX_SIGNATURE_004C6540,
        },
        {
            0x004C6560,
            reinterpret_cast<uintptr_t>(&sound_set_delay_redirect),
            OPENSMACX_SIGNATURE_004C6560,
        },
        {
            0x004C6600,
            reinterpret_cast<uintptr_t>(&sound_fade_redirect),
            OPENSMACX_SIGNATURE_004C6600,
        },
        {
            0x004C6690,
            reinterpret_cast<uintptr_t>(&sound_is_looping_redirect),
            OPENSMACX_SIGNATURE_004C6690,
        },
        {
            0x004C66A0,
            reinterpret_cast<uintptr_t>(&sound_get_time_redirect),
            OPENSMACX_SIGNATURE_004C66A0,
        },
        {
            0x004C6B00,
            reinterpret_cast<uintptr_t>(&wave_get_ms_length_redirect),
            OPENSMACX_SIGNATURE_004C6B00,
        },
        {
            0x004C6DB0,
            reinterpret_cast<uintptr_t>(&wave_load_redirect),
            OPENSMACX_SIGNATURE_004C6DB0,
        },
        {
            0x004C6EA0,
            reinterpret_cast<uintptr_t>(&wave_unload_redirect),
            OPENSMACX_SIGNATURE_004C6EA0,
        },
        {
            0x004C6EE0,
            reinterpret_cast<uintptr_t>(&wave_set_pitch_redirect),
            OPENSMACX_SIGNATURE_004C6EE0,
        },
        {
            0x004C70D0,
            reinterpret_cast<uintptr_t>(&wave_set_bufflimit_redirect),
            OPENSMACX_SIGNATURE_004C70D0,
        },
        {
            0x004C70E0,
            reinterpret_cast<uintptr_t>(&wave_set_asdr_redirect),
            OPENSMACX_SIGNATURE_004C70E0,
        },
        {
            0x004C70F0,
            reinterpret_cast<uintptr_t>(&wave_set_attack_redirect),
            OPENSMACX_SIGNATURE_004C70F0,
        },
        {
            0x004C7100,
            reinterpret_cast<uintptr_t>(&wave_set_sustain_redirect),
            OPENSMACX_SIGNATURE_004C7100,
        },
        {
            0x004C7110,
            reinterpret_cast<uintptr_t>(&wave_set_decay_redirect),
            OPENSMACX_SIGNATURE_004C7110,
        },
        {
            0x004C7120,
            reinterpret_cast<uintptr_t>(&wave_set_release_redirect),
            OPENSMACX_SIGNATURE_004C7120,
        },
        {
            0x004D9DC0,
            reinterpret_cast<uintptr_t>(&dip_edit_on_left_click_redirect),
            OPENSMACX_SIGNATURE_004D9DC0,
        },
        {
            0x004D9DD0,
            reinterpret_cast<uintptr_t>(&dip_edit_on_redraw_redirect),
            OPENSMACX_SIGNATURE_004D9DD0,
        },
        {
            0x004E1F40,
            reinterpret_cast<uintptr_t>(&console_edit_lock_redirect),
            OPENSMACX_SIGNATURE_004E1F40,
        },
        {
            0x0050F650,
            reinterpret_cast<uintptr_t>(&console_clear_group_redirect),
            OPENSMACX_SIGNATURE_0050F650,
        },
        {
            0x00514EF0,
            reinterpret_cast<uintptr_t>(&console_set_preferences_redirect),
            OPENSMACX_SIGNATURE_00514EF0,
        },
        {
            0x00514F10,
            reinterpret_cast<uintptr_t>(&console_set_auto_preferences_redirect),
            OPENSMACX_SIGNATURE_00514F10,
        },
        {
            0x00514F20,
            reinterpret_cast<uintptr_t>(&console_set_base_preferences_redirect),
            OPENSMACX_SIGNATURE_00514F20,
        },
        {
            0x00514F30,
            reinterpret_cast<uintptr_t>(&console_set_audiovisual_redirect),
            OPENSMACX_SIGNATURE_00514F30,
        },
        {
            0x00514F40,
            reinterpret_cast<uintptr_t>(&console_set_map_display_redirect),
            OPENSMACX_SIGNATURE_00514F40,
        },
        {
            0x0052DCD0,
            reinterpret_cast<uintptr_t>(&deletion_list_clear_redirect),
            OPENSMACX_SIGNATURE_0052DCD0,
        },
        {
            0x00530320,
            reinterpret_cast<uintptr_t>(&net_daemon_receive_redirect),
            OPENSMACX_SIGNATURE_00530320,
        },
        {
            0x00558FC0,
            reinterpret_cast<uintptr_t>(&base_pop_exec_callback_redirect),
            OPENSMACX_SIGNATURE_00558FC0,
        },
        {
            0x00559040,
            reinterpret_cast<uintptr_t>(&popup_start_label_value_redirect),
            OPENSMACX_SIGNATURE_00559040,
        },
        {
            0x0058FD70,
            reinterpret_cast<uintptr_t>(&square_lock_clear_redirect),
            OPENSMACX_SIGNATURE_0058FD70,
        },
        {
            0x0058FD90,
            reinterpret_cast<uintptr_t>(&square_lock_unlock_redirect),
            OPENSMACX_SIGNATURE_0058FD90,
        },
        {
            0x0058FE80,
            reinterpret_cast<uintptr_t>(&square_lock_lock_redirect),
            OPENSMACX_SIGNATURE_0058FE80,
        },
        {
            0x0058FF70,
            reinterpret_cast<uintptr_t>(&player_lock_clear_redirect),
            OPENSMACX_SIGNATURE_0058FF70,
        },
        {
            0x0058FFA0,
            reinterpret_cast<uintptr_t>(&player_lock_active_redirect),
            OPENSMACX_SIGNATURE_0058FFA0,
        },
        {
            0x005900D0,
            reinterpret_cast<uintptr_t>(&lock_clear_redirect),
            OPENSMACX_SIGNATURE_005900D0,
        },
        {
            0x00590140,
            reinterpret_cast<uintptr_t>(&lock_reset_map_redirect),
            OPENSMACX_SIGNATURE_00590140,
        },
        {
            0x00590170,
            reinterpret_cast<uintptr_t>(&lock_unlock_redirect),
            OPENSMACX_SIGNATURE_00590170,
        },
        {
            0x005901D0,
            reinterpret_cast<uintptr_t>(&lock_check_global_redirect),
            OPENSMACX_SIGNATURE_005901D0,
        },
        {
            0x00590240,
            reinterpret_cast<uintptr_t>(&lock_check_global_2_redirect),
            OPENSMACX_SIGNATURE_00590240,
        },
        {
            0x005902C0,
            reinterpret_cast<uintptr_t>(&lock_global_lock_redirect),
            OPENSMACX_SIGNATURE_005902C0,
        },
        {
            0x00590300,
            reinterpret_cast<uintptr_t>(&lock_lock_redirect),
            OPENSMACX_SIGNATURE_00590300,
        },
        {
            0x00590470,
            reinterpret_cast<uintptr_t>(&lock_add_lock_redirect),
            OPENSMACX_SIGNATURE_00590470,
        },
        {
            0x005904A0,
            reinterpret_cast<uintptr_t>(&lock_any_locks_redirect),
            OPENSMACX_SIGNATURE_005904A0,
        },
        {
            0x0059D3A0,
            reinterpret_cast<uintptr_t>(&pop_menu_init_redirect),
            OPENSMACX_SIGNATURE_0059D3A0,
        },
        {
            0x005A5900,
            reinterpret_cast<uintptr_t>(&base_pop_exec_void_redirect),
            OPENSMACX_SIGNATURE_005A5900,
        },
        {
            0x005A5990,
            reinterpret_cast<uintptr_t>(&popup_start_label_redirect),
            OPENSMACX_SIGNATURE_005A5990,
        },
        {
            0x005AD3D0,
            reinterpret_cast<uintptr_t>(&replay_win_on_left_double_click_redirect),
            OPENSMACX_SIGNATURE_005AD3D0,
        },
        {
            0x005AD3E0,
            reinterpret_cast<uintptr_t>(&replay_win_on_right_double_click_redirect),
            OPENSMACX_SIGNATURE_005AD3E0,
        },
        {
            0x005AD3F0,
            reinterpret_cast<uintptr_t>(&replay_win_on_mouse_move_redirect),
            OPENSMACX_SIGNATURE_005AD3F0,
        },
        {
            0x005AD400,
            reinterpret_cast<uintptr_t>(&replay_win_on_right_down_redirect),
            OPENSMACX_SIGNATURE_005AD400,
        },
        {
            0x005AD410,
            reinterpret_cast<uintptr_t>(&replay_win_on_left_down_redirect),
            OPENSMACX_SIGNATURE_005AD410,
        },
        {
            0x005AD420,
            reinterpret_cast<uintptr_t>(&replay_win_on_left_up_redirect),
            OPENSMACX_SIGNATURE_005AD420,
        },
        {
            0x005BF5D0,
            reinterpret_cast<uintptr_t>(&x_pops_minimal),
            OPENSMACX_SIGNATURE_005BF5D0,
        },
        {
            0x005BF7D0,
            reinterpret_cast<uintptr_t>(&x_pops_short),
            OPENSMACX_SIGNATURE_005BF7D0,
        },
        {
            0x005C9410,
            reinterpret_cast<uintptr_t>(&gamma_on_key_click_redirect),
            OPENSMACX_SIGNATURE_005C9410,
        },
        {
            0x005C98E0,
            reinterpret_cast<uintptr_t>(&multi_debug_close_redirect),
            OPENSMACX_SIGNATURE_005C98E0,
        },
        {
            0x005D5440,
            reinterpret_cast<uintptr_t>(&graphic_win_fill_redirect),
            OPENSMACX_SIGNATURE_005D5440,
        },
        {
            0x005DEF90,
            reinterpret_cast<uintptr_t>(&buffer_clear_links_redirect),
            OPENSMACX_SIGNATURE_005DEF90,
        },
        {
            0x005E4ADA,
            reinterpret_cast<uintptr_t>(&sprite_unk1_redirect),
            OPENSMACX_SIGNATURE_005E4ADA,
        },
        {
            0x005E4AEA,
            reinterpret_cast<uintptr_t>(&sprite_unk2_redirect),
            OPENSMACX_SIGNATURE_005E4AEA,
        },
        {
            0x005EAAFF,
            reinterpret_cast<uintptr_t>(&sprite_unk3_redirect),
            OPENSMACX_SIGNATURE_005EAAFF,
        },
        {
            0x005EAD8F,
            reinterpret_cast<uintptr_t>(&sprite_unk4_redirect),
            OPENSMACX_SIGNATURE_005EAD8F,
        },
        {
            0x005EC680,
            reinterpret_cast<uintptr_t>(&win_unk1_redirect),
            OPENSMACX_SIGNATURE_005EC680,
        },
        {
            0x005ECE80,
            reinterpret_cast<uintptr_t>(&win_unk3_redirect),
            OPENSMACX_SIGNATURE_005ECE80,
        },
        {
            0x005ED7C0,
            reinterpret_cast<uintptr_t>(&win_unk5_redirect),
            OPENSMACX_SIGNATURE_005ED7C0,
        },
        {
            0x005EDFE0,
            reinterpret_cast<uintptr_t>(&win_unk6_redirect),
            OPENSMACX_SIGNATURE_005EDFE0,
        },
        {
            0x005EE020,
            reinterpret_cast<uintptr_t>(&win_reset_window_clip_redirect),
            OPENSMACX_SIGNATURE_005EE020,
        },
        {
            0x005EE030,
            reinterpret_cast<uintptr_t>(&win_set_vert_pos_redirect),
            OPENSMACX_SIGNATURE_005EE030,
        },
        {
            0x005EE070,
            reinterpret_cast<uintptr_t>(&win_set_horz_pos_redirect),
            OPENSMACX_SIGNATURE_005EE070,
        },
        {
            0x005EE0B0,
            reinterpret_cast<uintptr_t>(&win_set_vert_range_redirect),
            OPENSMACX_SIGNATURE_005EE0B0,
        },
        {
            0x005EE0D0,
            reinterpret_cast<uintptr_t>(&win_set_horz_range_redirect),
            OPENSMACX_SIGNATURE_005EE0D0,
        },
        {
            0x005EE130,
            reinterpret_cast<uintptr_t>(&win_unk8_redirect),
            OPENSMACX_SIGNATURE_005EE130,
        },
        {
            0x005EE160,
            reinterpret_cast<uintptr_t>(&win_unk9_redirect),
            OPENSMACX_SIGNATURE_005EE160,
        },
        {
            0x005F2670,
            reinterpret_cast<uintptr_t>(&win_onsetcursor_redirect),
            OPENSMACX_SIGNATURE_005F2670,
        },
        {
            0x005F2C60,
            reinterpret_cast<uintptr_t>(&win_sync_palette_redirect),
            OPENSMACX_SIGNATURE_005F2C60,
        },
        {
            0x005F2CE0,
            reinterpret_cast<uintptr_t>(&win_set_def_focus_redirect),
            OPENSMACX_SIGNATURE_005F2CE0,
        },
        {
            0x005F54B0,
            reinterpret_cast<uintptr_t>(&win_on_move_redirect),
            OPENSMACX_SIGNATURE_005F54B0,
        },
        {
            0x005F54C0,
            reinterpret_cast<uintptr_t>(&win_on_size_redirect),
            OPENSMACX_SIGNATURE_005F54C0,
        },
        {
            0x005F54D0,
            reinterpret_cast<uintptr_t>(&win_on_size_nc_redirect),
            OPENSMACX_SIGNATURE_005F54D0,
        },
        {
            0x005F6A30,
            reinterpret_cast<uintptr_t>(&win_on_set_cursor_redirect),
            OPENSMACX_SIGNATURE_005F6A30,
        },
        {
            0x005F6A40,
            reinterpret_cast<uintptr_t>(&win_on_sys_command_redirect),
            OPENSMACX_SIGNATURE_005F6A40,
        },
        {
            0x005F8C90,
            reinterpret_cast<uintptr_t>(&pull_down_unk2_redirect),
            OPENSMACX_SIGNATURE_005F8C90,
        },
        {
            0x005F8CA0,
            reinterpret_cast<uintptr_t>(&pull_down_unk3_redirect),
            OPENSMACX_SIGNATURE_005F8CA0,
        },
        {
            0x005F9D00,
            reinterpret_cast<uintptr_t>(&pull_down_id_to_index_redirect),
            OPENSMACX_SIGNATURE_005F9D00,
        },
        {
            0x005FA740,
            reinterpret_cast<uintptr_t>(&pull_down_on_mouse_leave_redirect),
            OPENSMACX_SIGNATURE_005FA740,
        },
        {
            0x005FA750,
            reinterpret_cast<uintptr_t>(&pull_down_unk5_redirect),
            OPENSMACX_SIGNATURE_005FA750,
        },
        {
            0x005FA760,
            reinterpret_cast<uintptr_t>(&pull_down_unk6_redirect),
            OPENSMACX_SIGNATURE_005FA760,
        },
        {
            0x005FB1C0,
            reinterpret_cast<uintptr_t>(&menu_unk2_redirect),
            OPENSMACX_SIGNATURE_005FB1C0,
        },
        {
            0x005FB230,
            reinterpret_cast<uintptr_t>(&menu_unk4_redirect),
            OPENSMACX_SIGNATURE_005FB230,
        },
        {
            0x005FC6A0,
            reinterpret_cast<uintptr_t>(&menu_requested_height_redirect),
            OPENSMACX_SIGNATURE_005FC6A0,
        },
        {
            0x005FED10,
            reinterpret_cast<uintptr_t>(&palette_get_pos_redirect),
            OPENSMACX_SIGNATURE_005FED10,
        },
        {
            0x00601B20,
            reinterpret_cast<uintptr_t>(&base_pop_set_width_redirect),
            OPENSMACX_SIGNATURE_00601B20,
        },
        {
            0x00601BB0,
            reinterpret_cast<uintptr_t>(&base_pop_write_check_redirect),
            OPENSMACX_SIGNATURE_00601BB0,
        },
        {
            0x00604730,
            reinterpret_cast<uintptr_t>(&base_pop_set_string_color_redirect),
            OPENSMACX_SIGNATURE_00604730,
        },
        {
            0x00604760,
            reinterpret_cast<uintptr_t>(&base_pop_set_string_color2_redirect),
            OPENSMACX_SIGNATURE_00604760,
        },
        {
            0x00604790,
            reinterpret_cast<uintptr_t>(&base_pop_set_string_color3_redirect),
            OPENSMACX_SIGNATURE_00604790,
        },
        {
            0x006047C0,
            reinterpret_cast<uintptr_t>(&base_pop_set_string_color_hyper_redirect),
            OPENSMACX_SIGNATURE_006047C0,
        },
        {
            0x00604830,
            reinterpret_cast<uintptr_t>(&base_pop_set_button_color_redirect),
            OPENSMACX_SIGNATURE_00604830,
        },
        {
            0x00604860,
            reinterpret_cast<uintptr_t>(&base_pop_set_button_color2_redirect),
            OPENSMACX_SIGNATURE_00604860,
        },
        {
            0x00604890,
            reinterpret_cast<uintptr_t>(&base_pop_set_button_color3_redirect),
            OPENSMACX_SIGNATURE_00604890,
        },
        {
            0x00604900,
            reinterpret_cast<uintptr_t>(&base_pop_set_def_string_color_redirect),
            OPENSMACX_SIGNATURE_00604900,
        },
        {
            0x00604930,
            reinterpret_cast<uintptr_t>(&base_pop_set_def_string_color2_redirect),
            OPENSMACX_SIGNATURE_00604930,
        },
        {
            0x00604960,
            reinterpret_cast<uintptr_t>(&base_pop_set_def_string_color3_redirect),
            OPENSMACX_SIGNATURE_00604960,
        },
        {
            0x00604990,
            reinterpret_cast<uintptr_t>(&base_pop_set_def_string_color_hyper_redirect),
            OPENSMACX_SIGNATURE_00604990,
        },
        {
            0x006049F0,
            reinterpret_cast<uintptr_t>(&base_pop_set_def_button_color_redirect),
            OPENSMACX_SIGNATURE_006049F0,
        },
        {
            0x00604A20,
            reinterpret_cast<uintptr_t>(&base_pop_set_def_button_color2_redirect),
            OPENSMACX_SIGNATURE_00604A20,
        },
        {
            0x00604A50,
            reinterpret_cast<uintptr_t>(&base_pop_set_def_button_color3_redirect),
            OPENSMACX_SIGNATURE_00604A50,
        },
        {
            0x00604A90,
            reinterpret_cast<uintptr_t>(&base_pop_fallout_redirect),
            OPENSMACX_SIGNATURE_00604A90,
        },
        {
            0x00606310,
            reinterpret_cast<uintptr_t>(&scroll_on_left_click_redirect),
            OPENSMACX_SIGNATURE_00606310,
        },
        {
            0x006077F0,
            reinterpret_cast<uintptr_t>(&base_button_on_key_click_redirect),
            OPENSMACX_SIGNATURE_006077F0,
        },
        {
            0x00607800,
            reinterpret_cast<uintptr_t>(&base_button_on_key_down_redirect),
            OPENSMACX_SIGNATURE_00607800,
        },
        {
            0x00607810,
            reinterpret_cast<uintptr_t>(&base_button_on_key_up_redirect),
            OPENSMACX_SIGNATURE_00607810,
        },
        {
            0x0060C7C0,
            reinterpret_cast<uintptr_t>(&list_box_on_dialog_focus_redirect),
            OPENSMACX_SIGNATURE_0060C7C0,
        },
        {
            0x0060D1B0,
            reinterpret_cast<uintptr_t>(&radio_button_close_redirect),
            OPENSMACX_SIGNATURE_0060D1B0,
        },
        {
            0x0060E7C0,
            reinterpret_cast<uintptr_t>(&check_box_close_redirect),
            OPENSMACX_SIGNATURE_0060E7C0,
        },
        {
            0x00611040,
            reinterpret_cast<uintptr_t>(&sprite_box_on_mouse_move_redirect),
            OPENSMACX_SIGNATURE_00611040,
        },
        {
            0x00611050,
            reinterpret_cast<uintptr_t>(&sprite_box_on_mouse_leave_redirect),
            OPENSMACX_SIGNATURE_00611050,
        },
        {
            0x00612040,
            reinterpret_cast<uintptr_t>(&edit_group_set_text_redirect),
            OPENSMACX_SIGNATURE_00612040,
        },
        {
            0x00612060,
            reinterpret_cast<uintptr_t>(&edit_group_get_text_redirect),
            OPENSMACX_SIGNATURE_00612060,
        },
        {
            0x006120B0,
            reinterpret_cast<uintptr_t>(&edit_group_set_text_limits_redirect),
            OPENSMACX_SIGNATURE_006120B0,
        },
        {
            0x00612A70,
            reinterpret_cast<uintptr_t>(&dialogs_item_redirect),
            OPENSMACX_SIGNATURE_00612A70,
        },
        {
            0x00612ED0,
            reinterpret_cast<uintptr_t>(&dialogs_on_right_down_redirect),
            OPENSMACX_SIGNATURE_00612ED0,
        },
        {
            0x00612EF0,
            reinterpret_cast<uintptr_t>(&dialogs_on_right_double_click_redirect),
            OPENSMACX_SIGNATURE_00612EF0,
        },
        {
            0x00612F10,
            reinterpret_cast<uintptr_t>(&dialogs_on_left_up_redirect),
            OPENSMACX_SIGNATURE_00612F10,
        },
        {
            0x00612F40,
            reinterpret_cast<uintptr_t>(&dialogs_on_right_up_redirect),
            OPENSMACX_SIGNATURE_00612F40,
        },
        {
            0x00612F60,
            reinterpret_cast<uintptr_t>(&dialogs_on_right_click_redirect),
            OPENSMACX_SIGNATURE_00612F60,
        },
        {
            0x00612F80,
            reinterpret_cast<uintptr_t>(&dialogs_on_scrolled_redirect),
            OPENSMACX_SIGNATURE_00612F80,
        },
        {
            0x00612FA0,
            reinterpret_cast<uintptr_t>(&dialogs_on_scrolling_redirect),
            OPENSMACX_SIGNATURE_00612FA0,
        },
        {
            0x00612FC0,
            reinterpret_cast<uintptr_t>(&dialogs_on_mousewheel_redirect),
            OPENSMACX_SIGNATURE_00612FC0,
        },
        {
            0x00613740,
            reinterpret_cast<uintptr_t>(&dialogs_get_num_items_redirect),
            OPENSMACX_SIGNATURE_00613740,
        },
        {
            0x00614320,
            reinterpret_cast<uintptr_t>(&file_win_unk1_redirect),
            OPENSMACX_SIGNATURE_00614320,
        },
        {
            0x00614350,
            reinterpret_cast<uintptr_t>(&file_win_unk2_redirect),
            OPENSMACX_SIGNATURE_00614350,
        },
        {
            0x00614B80,
            reinterpret_cast<uintptr_t>(&file_win_unk6_redirect),
            OPENSMACX_SIGNATURE_00614B80,
        },
        {
            0x006176C0,
            reinterpret_cast<uintptr_t>(&caviar_unk3_redirect),
            OPENSMACX_SIGNATURE_006176C0,
        },
        {
            0x006176D0,
            reinterpret_cast<uintptr_t>(&caviar_unk4_redirect),
            OPENSMACX_SIGNATURE_006176D0,
        },
        {
            0x00618250,
            reinterpret_cast<uintptr_t>(&caviar_unk5_redirect),
            OPENSMACX_SIGNATURE_00618250,
        },
        {
            0x00618260,
            reinterpret_cast<uintptr_t>(&caviar_unk6_redirect),
            OPENSMACX_SIGNATURE_00618260,
        },
        {
            0x00618270,
            reinterpret_cast<uintptr_t>(&caviar_unk7_redirect),
            OPENSMACX_SIGNATURE_00618270,
        },
        {
            0x00618280,
            reinterpret_cast<uintptr_t>(&caviar_unk8_redirect),
            OPENSMACX_SIGNATURE_00618280,
        },
        {
            0x00618290,
            reinterpret_cast<uintptr_t>(&caviar_unk9_redirect),
            OPENSMACX_SIGNATURE_00618290,
        },
        {
            0x00618320,
            reinterpret_cast<uintptr_t>(&caviar_unk10_redirect),
            OPENSMACX_SIGNATURE_00618320,
        },
        {
            0x00618370,
            reinterpret_cast<uintptr_t>(&caviar_set_scene_rotation_redirect),
            OPENSMACX_SIGNATURE_00618370,
        },
        {
            0x006183A0,
            reinterpret_cast<uintptr_t>(&caviar_unk12_redirect),
            OPENSMACX_SIGNATURE_006183A0,
        },
        {
            0x00618F30,
            reinterpret_cast<uintptr_t>(&font_unk1_redirect),
            OPENSMACX_SIGNATURE_00618F30,
        },
        {
            0x00629710,
            reinterpret_cast<uintptr_t>(&string_box_add_redirect),
            OPENSMACX_SIGNATURE_00629710,
        },
        {
            0x0062A710,
            reinterpret_cast<uintptr_t>(&flic_unk4_redirect),
            OPENSMACX_SIGNATURE_0062A710,
        },
        {
            0x0062AAF0,
            reinterpret_cast<uintptr_t>(&flic_unk5_redirect),
            OPENSMACX_SIGNATURE_0062AAF0,
        },
        {
            0x0062AB00,
            reinterpret_cast<uintptr_t>(&flic_unk6_redirect),
            OPENSMACX_SIGNATURE_0062AB00,
        },
        {
            0x0062ACC0,
            reinterpret_cast<uintptr_t>(&flic_unk7_redirect),
            OPENSMACX_SIGNATURE_0062ACC0,
        },
        {
            0x0062B5F0,
            reinterpret_cast<uintptr_t>(&flic_unk8_redirect),
            OPENSMACX_SIGNATURE_0062B5F0,
        },
        {
            0x0062B870,
            reinterpret_cast<uintptr_t>(&button_group_set_redirect),
            OPENSMACX_SIGNATURE_0062B870,
        },
        {
            0x0062BA80,
            reinterpret_cast<uintptr_t>(&auto_sound_construct_redirect),
            OPENSMACX_SIGNATURE_0062BA80,
        },
        {
            0x005E4B4A,
            reinterpret_cast<uintptr_t>(&sprite_draw_redirect),
            OPENSMACX_SIGNATURE_005E4B4A,
        },
        {
            0x005EB3D0,
            reinterpret_cast<uintptr_t>(&win_construct_redirect),
            OPENSMACX_SIGNATURE_005EB3D0,
        },
        {
            0x005FE560,
            reinterpret_cast<uintptr_t>(&palette_get_rgbquad_redirect),
            OPENSMACX_SIGNATURE_005FE560,
        },
        {
            0x005D7210,
            reinterpret_cast<uintptr_t>(&buffer_construct_redirect),
            OPENSMACX_SIGNATURE_005D7210,
        },
        {
            0x005D4CF0,
            reinterpret_cast<uintptr_t>(&graphic_win_construct_redirect),
            OPENSMACX_SIGNATURE_005D4CF0,
        },
        {
            0x00606F30,
            reinterpret_cast<uintptr_t>(&base_button_construct_redirect),
            OPENSMACX_SIGNATURE_00606F30,
        },
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
            0x005D8000,
            reinterpret_cast<uintptr_t>(&buffer_set_clip_redirect),
            OPENSMACX_SIGNATURE_005D8000,
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
            0x005DC790,
            reinterpret_cast<uintptr_t>(&buffer_text_width_redirect),
            OPENSMACX_SIGNATURE_005DC790,
        },
        {
            0x005DCA80,
            reinterpret_cast<uintptr_t>(&buffer_text_height_redirect),
            OPENSMACX_SIGNATURE_005DCA80,
        },
        {
            0x005DE8F0,
            reinterpret_cast<uintptr_t>(&buffer_sync_to_palette_redirect),
            OPENSMACX_SIGNATURE_005DE8F0,
        },
        {
            0x005E3503,
            reinterpret_cast<uintptr_t>(&buffer_get_hdc_redirect),
            OPENSMACX_SIGNATURE_005E3503,
        },
        {
            0x005E3563,
            reinterpret_cast<uintptr_t>(&buffer_release_hdc_redirect),
            OPENSMACX_SIGNATURE_005E3563,
        },
        {
            0x005E3820,
            reinterpret_cast<uintptr_t>(&sprite_close_redirect),
            OPENSMACX_SIGNATURE_005E3820,
        },
        {
            0x005F2CA0,
            reinterpret_cast<uintptr_t>(&win_is_dialog_focus_redirect),
            OPENSMACX_SIGNATURE_005F2CA0,
        },
        {
            0x005FE4F0,
            reinterpret_cast<uintptr_t>(&palette_set_active_window_redirect),
            OPENSMACX_SIGNATURE_005FE4F0,
        },
        {
            0x005F7E90,
            reinterpret_cast<uintptr_t>(&win_is_visible_redirect),
            OPENSMACX_SIGNATURE_005F7E90,
        },
        {
            0x005F8500,
            reinterpret_cast<uintptr_t>(&win_clear_bubble_text_redirect),
            OPENSMACX_SIGNATURE_005F8500,
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
            0x005EC690,
            reinterpret_cast<uintptr_t>(&win_get_hdc_redirect),
            OPENSMACX_SIGNATURE_005EC690,
        },
        {
            0x005EC6F0,
            reinterpret_cast<uintptr_t>(&win_release_hdc_redirect),
            OPENSMACX_SIGNATURE_005EC6F0,
        },
        {
            0x005EC7C0,
            reinterpret_cast<uintptr_t>(&win_set_cursor_redirect),
            OPENSMACX_SIGNATURE_005EC7C0,
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
            0x006048C0,
            reinterpret_cast<uintptr_t>(&base_pop_set_def_string_font_redirect),
            OPENSMACX_SIGNATURE_006048C0,
        },
        {
            0x006049C0,
            reinterpret_cast<uintptr_t>(&base_pop_set_def_button_font_redirect),
            OPENSMACX_SIGNATURE_006049C0,
        },
        {
            0x00609D20,
            reinterpret_cast<uintptr_t>(&dialog_set_def_dialog_font_redirect),
            OPENSMACX_SIGNATURE_00609D20,
        },
        {
            0x006070C0,
            reinterpret_cast<uintptr_t>(&base_button_close_redirect),
            OPENSMACX_SIGNATURE_006070C0,
        },
        {
            0x00607360,
            reinterpret_cast<uintptr_t>(&base_button_set_text_color_redirect),
            OPENSMACX_SIGNATURE_00607360,
        },
        {
            0x006073A0,
            reinterpret_cast<uintptr_t>(&base_button_set_text_color2_redirect),
            OPENSMACX_SIGNATURE_006073A0,
        },
        {
            0x006073E0,
            reinterpret_cast<uintptr_t>(&base_button_set_text_color3_redirect),
            OPENSMACX_SIGNATURE_006073E0,
        },
        {
            0x00607420,
            reinterpret_cast<uintptr_t>(&base_button_set_def_text_color_redirect),
            OPENSMACX_SIGNATURE_00607420,
        },
        {
            0x00607450,
            reinterpret_cast<uintptr_t>(&base_button_set_def_text_color2_redirect),
            OPENSMACX_SIGNATURE_00607450,
        },
        {
            0x00607480,
            reinterpret_cast<uintptr_t>(&base_button_set_def_text_color3_redirect),
            OPENSMACX_SIGNATURE_00607480,
        },
        {
            0x006074B0,
            reinterpret_cast<uintptr_t>(&base_button_set_def_font_redirect),
            OPENSMACX_SIGNATURE_006074B0,
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
            0x004E25B0,
            reinterpret_cast<uintptr_t>(&alpha_net_close_redirect),
            OPENSMACX_SIGNATURE_004E25B0,
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
            0x0062ED30,
            reinterpret_cast<uintptr_t>(&net_poll_players_redirect),
            OPENSMACX_SIGNATURE_0062ED30,
        },
        {
            0x006339A0,
            reinterpret_cast<uintptr_t>(&check_button_init_class_redirect),
            OPENSMACX_SIGNATURE_006339A0,
        },
        {
            0x006339B0,
            reinterpret_cast<uintptr_t>(&check_button_close_class_redirect),
            OPENSMACX_SIGNATURE_006339B0,
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
        {
            0x0063B930,
            reinterpret_cast<uintptr_t>(&cursor_close_cursor_class_redirect),
            OPENSMACX_SIGNATURE_0063B930,
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
