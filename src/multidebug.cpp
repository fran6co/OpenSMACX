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
#include "multidebug.h"
#include "vtable_shim.h"

const uint32_t MultiDebugPrimaryVtable = 0x0066FA88;
const uint32_t MultiDebugBufferVtable = 0x0066FA80;

/*
Purpose: Construct the GraphicWin base, the embedded Font and Time, then
         install MultiDebug's own vtables and zero the flag field.
// ORIGINAL: 0x005C97F0 ??0MultiDebug@@QAE@XZ 0x005C97F0-0x005C9860;0x00662AF0-0x00662B10 BYTE_EXACT
// LEVER: `new (&font_) Font(); new (&timer_) Time();` - a `new`-expression, not the member-function-call syntax used elsewhere (MCIVideo's Palette) - is what picks up the image's SEH frame here; the member-call-syntax spelling compiled with NO frame at all (0/28 agreeing, diverging at instruction 0).
// size      112 bytes
// prototype void (__thiscall ??0MultiDebug@@QAE@XZ)(MultiDebug* this)
// callers   0   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok;frame
// calls     0x005D4CF0 0x00618EA0 0x006161D0
Return Value: n/a
Status: Complete
*/
MultiDebug::MultiDebug() {
    uint32_t *const object = reinterpret_cast<uint32_t *>(this);
    object[0x000 / 4] = MultiDebugPrimaryVtable;
    object[0x444 / 4] = MultiDebugBufferVtable;
    field_A3C_ = 0;
}

/*
Purpose: Close the debug window by clearing its single active flag.
// ORIGINAL: 0x005C98E0 ?close@MultiDebug@@QAEXXZ 0x005C98E0-0x005C98EB BYTE_EXACT
// size      11 bytes
// prototype void (__thiscall ?close@MultiDebug@@QAEXXZ)(MultiDebug* this)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void MultiDebug::close() {
    field_A3C_ = 0;
}

void __fastcall multi_debug_close_redirect(MultiDebug *self, void *) {
    self->close();
}

/*
Purpose: Forward the timer tick to the object's own virtual slot 62.
// ORIGINAL: 0x005C9900 ?timer_callback_daemon@MultiDebug@@QAAXH@Z 0x005C9900-0x005C9914 BYTE_EXACT
// size      20 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005C990C
Return Value: n/a
Status: Complete
*/
void __cdecl MultiDebug::timer_callback_daemon(int a2) {
    if (this) {
        reinterpret_cast<VCall *>(this)->slot062();
    }
}
