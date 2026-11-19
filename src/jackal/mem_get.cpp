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
#include <stdlib.h>

// Inferred original member: mem_get alone at 0x005D4510..0x005D453C, the first
// JACKAL.LIB contribution (int3 fill before it, after ddraw.lib's
// DirectDrawCreate thunk). It is not part of Heap's member: in Heap's
// translation unit VC6 /Ob2 would inline it into Heap::init, which the image
// calls. The original filename is unknown.

// Original mem_get at 0x005D4510 reports failure through this USER32 export; no function alias.
// RECOVERY_IMPORT: v1 __imp__MessageBoxA@16 USER32.dll name:MessageBoxA
// RECOVERY_LITERAL: v1 0x00696A38 0x00696A44 $SG18068 0x005D4510
// RECOVERY_LITERAL: v1 0x00696A44 0x00696B14 $SG18069 0x005D4510
/*
Purpose: Allocate memory with error checking.
// ORIGINAL: 0x005D4510 ?mem_get@@YAPAXH@Z 0x005D4510-0x005D453C BYTE_EXACT
// symbol    ?mem_get@@YAPAXH@Z
// size      44 bytes
// prototype void* (__cdecl ?mem_get@@YAPAXH@Z)(int size)
// callers   88   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00644DFF 0x006470A6
// indirect  0x005D452E
Return Value: Pointer to allocated memory
Status: Complete. Historical note: an earlier DLL/CRT migration used two
        malloc wrappers to avoid a reported SDK incompatibility; the recovered
        callers now use the canonical pinned int-sized mem_get contract.
*/
LPVOID __cdecl mem_get(int size) {
    LPVOID result = malloc(size);
    if (!result) {
        MessageBoxA(NULL,
            "Windows has run out of virtual memory; the game will exit. "
            "Recommendations: defragment your hard drive; use Control Panel "
            "(System/Performance/Virtual Memory) to increase your virtual memory to at least 128MB",
            "FATAL ERROR", MB_ICONWARNING);
        exit(4);
    }
    return result;
}
