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
#include "base.h"

#include <cstdint>
#include <cstring>

namespace {

constexpr size_t PatchSize = 5;
constexpr size_t SignatureSize = 16;

struct RedirectState {
    uint8_t *address;
    uint8_t original[PatchSize];
    DWORD original_protection;
    bool installed;
};

RedirectState BaseAtRedirect = {};

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

} // namespace

BOOL APIENTRY DllMain(HMODULE hModule, uint32_t ul_reason_for_call, LPVOID reserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        {
            const uint8_t base_at_signature[16] = {
                0x55, 0x8B, 0xEC, 0x53, 0x56, 0x57, 0x8B, 0x7D,
                0x0C, 0x85, 0xFF, 0x0F, 0x8C, 0x16, 0x01, 0x00,
            };
            return redirect_function(
                BaseAtRedirect,
                0x004E3A50, reinterpret_cast<uintptr_t>(&base_at), base_at_signature);
        }
    case DLL_PROCESS_DETACH:
        if (!reserved && !restore_redirect(BaseAtRedirect)) {
            TerminateProcess(GetCurrentProcess(), ERROR_INVALID_STATE);
        }
        return true;
    default:
        return true;
    }
}
