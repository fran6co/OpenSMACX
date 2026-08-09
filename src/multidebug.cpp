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

/*
Purpose: Close the debug window by clearing its single active flag.
ORIGINAL: 0x005C98E0
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
ORIGINAL: 0x005C9900
Return Value: n/a
Status: Complete
*/
void __cdecl MultiDebug::timer_callback_daemon(int a2) {
    if (this) {
        reinterpret_cast<VCall *>(this)->slot062();
    }
}
