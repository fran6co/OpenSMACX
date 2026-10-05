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
#include "original_seam.h"
#include "delegation_thunks.h"

// Transitional Wave_In_Device forwards: the receiver's delegate at +0x14
// and its interface ownership still need recovery. Typed window callbacks
// belong to their real class owners; no retired generator owns this file.

/*
Purpose: ?set_codec@Wave_In_Device@@QAEHK@Z - forward 1 argument(s) to slot 0x60 of the
         object at +0x14, answering 0x13 when that object is absent.
// ORIGINAL: 0x004C5A80 ?set_codec@Wave_In_Device@@QAEHK@Z 0x004C5A80-0x004C5AA0
// symbol    ?wave_in_device_set_codec_redirect@@YIHPAX0I@Z
// size      32 bytes
// prototype int (__thiscall ?set_codec@Wave_In_Device@@QAEHK@Z)(Wave_In_Device* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C5A90
Return Value: the delegate's, or 0x13 when the member is null
Status: Complete
*/
int __fastcall wave_in_device_set_codec_redirect(void *self, void *, uint32_t a1) {
    void *const member = *reinterpret_cast<void **>(
        static_cast<uint8_t *>(self) + 0x14);
    if (member == nullptr) {
        return 0x13;
    }
    void **const vtable = *reinterpret_cast<void ***>(member);
    return (ORIGINAL(member)->*original_method<func_delegation_1>(reinterpret_cast<unsigned long>(vtable[0x60 / sizeof(void *)])))(a1);
}

/*
Purpose: ?set_vxw_key@Wave_In_Device@@QAEHPAC@Z - forward 1 argument(s) to slot 0x64 of the
         object at +0x14, answering 0x13 when that object is absent.
// ORIGINAL: 0x004C5AA0 ?set_vxw_key@Wave_In_Device@@QAEHPAC@Z 0x004C5AA0-0x004C5AC0
// symbol    ?wave_in_device_set_vxw_key_redirect@@YIHPAX0H@Z
// size      32 bytes
// prototype int (__thiscall ?set_vxw_key@Wave_In_Device@@QAEHPAC@Z)(Wave_In_Device* this, int8*)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C5AB0
Return Value: the delegate's, or 0x13 when the member is null
Status: Complete
*/
int __fastcall wave_in_device_set_vxw_key_redirect(void *self, void *, int a1) {
    void *const member = *reinterpret_cast<void **>(
        static_cast<uint8_t *>(self) + 0x14);
    if (member == nullptr) {
        return 0x13;
    }
    void **const vtable = *reinterpret_cast<void ***>(member);
    return (ORIGINAL(member)->*original_method<func_delegation_1>(reinterpret_cast<unsigned long>(vtable[0x64 / sizeof(void *)])))(a1);
}
