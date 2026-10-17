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
#include "graphics_language.h"

// AN OBJECT, NOT A POINTER TO A FIXED ADDRESS: the pointer form costs a
// load at every use where the image addresses the storage directly, and
// the address is terranx.exe's data, unmapped in a standalone build.
// set_language (0x00627100) is exactly `mov eax, [esp+4];
// mov dword ptr [0x9BC054], eax; ret`.
// RECOVERY_OBJECT: v1 0x009BC054 ?Language@@3IA
uint32_t Language;  // 0x009BC054

/*
Purpose: Set the game's language.
// ORIGINAL: 0x00627100 ?set_language@@YAXH@Z 0x00627100-0x0062710A BYTE_EXACT
// size      10 bytes
// prototype void (__cdecl ?set_language@@YAXH@Z)(int lang)
// callers   2   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl set_language(int language) {
    Language = language;
}
