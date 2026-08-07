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
#include "statuswin.h"

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
Original Offset: 004B9F80
Return Value: n/a
Status: Complete
*/
void StatusWin::close() {
}

void __fastcall status_win_close_redirect(StatusWin *self, void *) {
    self->close();
}

/*
Purpose: Place the status window, invalidating the two cached values that
         follow its position.
Original Offset: 004B9F90
Return Value: n/a
Status: Complete
*/
void StatusWin::set_loc(int x, int y) {
    iTileX_ = x;
    iTileY_ = y;
    iCityIndex_ = -1;
    iUnitIndex_ = -1;
}

void __fastcall status_win_set_loc_redirect(StatusWin *self, void *, int x, int y) {
    self->set_loc(x, y);
}

func_release_iface_mode SubInterfaceOriginalReleaseIfaceMode =
    original_method<func_release_iface_mode>(0x0045D380);
void *SubInterfaceGlobal = (void *)0x006EEED8;

/*
Purpose: Release the status window's hold on the interface mode, if it holds
         one. The original tail-jumps into the release, so nothing follows it.
Original Offset: 004B8970
Return Value: n/a
Status: Complete
*/
void StatusWin::reset() {
    if (field_15D4_) {
        field_15D4_ = 0;
        (ORIGINAL(SubInterfaceGlobal)->*SubInterfaceOriginalReleaseIfaceMode)();
    }
}

void __fastcall status_win_reset_redirect(StatusWin *self, void *) {
    self->reset();
}
