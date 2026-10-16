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

#include "alpha.h"
#include "text.h"

// Labels storage and its lifecycle are kept together as one source owner.
// The mapping is semantic ownership evidence, not a claim about the original TU.
// labels_shutdown (0x006169D0) and labels_init (0x00616A00) address the
// struct in place: `mov eax, dword ptr [0x9B90F8]` (strings_ptr) and
// `mov dword ptr [0x9B90FC], 0` (count).
// RECOVERY_OBJECT: v1 0x009B90F8 ?Labels@@3ULabel@@A
Label Labels;                                       // 0x009B90F8

/*
Purpose: Shutdown or reset the labels global variable.
// ORIGINAL: 0x006169D0 ?labels_shutdown@@YAXXZ 0x006169D0-0x006169F7 BYTE_EXACT
// body      src/labels.cpp
// size      39 bytes
// prototype
// callers   4   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00644EF2
Return Value: n/a
Status: Complete
*/
void __cdecl labels_shutdown() {
    if (Labels.strings_ptr) {
        free(Labels.strings_ptr);
        Labels.strings_ptr = 0;
    }
    Labels.count = 0;
}

/*
Purpose: Parse the #LABELS section inside the labels.txt file.
// ORIGINAL: 0x00616A00 ?labels_init@@YAHXZ 0x00616A00-0x00616A93 BYTE_EXACT
// LEVER: `Labels` as a typed-pointer binding plateaued at 40/45 - every source
// form computed `count*4` via `lea` before the store. Converting it to the real
// object (defined above) let VC6 fold `shl eax,2` into the store exactly as the
// image does. (Promoted from TRIED when the body matched, 2026-08-29.)
// size      147 bytes
// prototype 
// callers   3   call targets   7
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4510 0x005FD530 0x005FD550 0x005FD570 0x005FD5E0 0x005FD740 0x00644EF2
Return Value: Was there an error? true/false
Status: Complete
*/
BOOL __cdecl labels_init() {
    labels_shutdown();
    if (text_open("labels", "labels")) {
        return true;
    }
    text_get();
    Labels.count = text_item_number();
    Labels.strings_ptr = (LPSTR *)mem_get(Labels.count * 4);
    if (!Labels.strings_ptr) {
        return true;
    }
    for (int i = 0; i < Labels.count; i++) {
        Labels.strings_ptr[i] = text_string();
    }
    text_close();
    return false;
}
