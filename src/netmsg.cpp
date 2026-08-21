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
#include "netmsg.h"
#include "spritebox.h"
#include "net_class.h"
#include "popup.h"


/*
Purpose: Close an outstanding network message - stop its timer and hide the
         popup - doing nothing when none is showing.
// ORIGINAL: 0x0047A7D0 ?close@NetMsg@@QAEXXZ 0x0047A7D0-0x0047A7FB BYTE_EXACT
// size      43 bytes
// prototype void (__thiscall ?close@NetMsg@@QAEXXZ)(NetMsg* this)
// callers   20   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00404A80 0x00616780
Return Value: n/a
Status: Complete
*/
void NetMsg::close() {
    if (field_5384_) {
        field_5384_ = 0;
        timer_.close();
        Popup::hide();
    }
}

void __fastcall net_msg_close_redirect(NetMsg *self, void *) {
    self->close();
}
