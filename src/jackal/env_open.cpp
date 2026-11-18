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
#include "general.h"
#include <stdio.h>

// Inferred original member: env_open at 0x00634BB0, between Jackal's matrix
// code (in Vector's member) and FileBox. Membership with a neighbour is not
// established; the original filename is unknown.

/*
Purpose: Check the source file path and attempt to open a handle to the file.
// ORIGINAL: 0x00634BB0 ?env_open@@YAHPADPAD@Z 0x00634BB0-0x00634BD4 BYTE_EXACT
// symbol    ?env_open@@YAPAU_iobuf@@PBD0@Z
// size      36 bytes
// prototype 
// callers   18   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x006005D0 0x00645646
Return Value: FILE pointer
Status: Complete with two versions of fopen to prevent a crash. Incompatibility with newer SDK 
        version of fopen/fopen_s. Revisit once more code is redirected to dll.
*/
FILE *__cdecl env_open_old(LPCSTR source, LPCSTR mode) {
    LPCSTR src_check = filefind_get(source);
    if (!src_check) {
        src_check = source;
    }
    return fopen(src_check, mode);
}

FILE *__cdecl env_open(LPCSTR source, LPCSTR mode) {
    LPCSTR src_check = filefind_get(source);
    if (!src_check) {
        src_check = source;
    }
    FILE *file_out;
    fopen_s(&file_out, src_check, mode);
    return file_out;
}

