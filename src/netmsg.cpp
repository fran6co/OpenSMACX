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
#include "time.h"

const uint32_t NetMsgPrimaryVtable = 0x0066CB60;
const uint32_t NetMsgBufferVtable = 0x0066CB58;

/*
Purpose: Construct the Popup base and the embedded Time, then install
         NetMsg's own vtables and the three constructor arguments.
// ORIGINAL: 0x0047ACF0 ??0NetMsg@@QAE@HHH@Z 0x0047ACF0-0x0047AD6E;0x00656440-0x00656452
// LEVER: a real `NetMsg(int, int, int)` constructor, MISMATCH 11/31 -> SHAPE_EXACT 27/31, because the placement-new null guard disappears - the same lever that took MultiDebug::construct() to BYTE_EXACT. The two dynamic initialisers that build the NetMsg globals reach it as `ptr->NetMsg::NetMsg(...)`, explicit constructor call syntax with no new-expression, and stay BYTE_EXACT; written `new (ptr) NetMsg(...)` they pay the guard instead and 0x0047A7A0 falls to 1 of 9. `new (ptr) T()` and `ptr->T::T()` are not interchangeable here.
// TRIED: the residual is register allocation past the prologue. An EARLIER refusal here blamed `push ecx` against `sub esp, 8` on "how many spill slots the two placement-news plus the `object` local need, not a control-flow difference" - that was backwards, and the placement-new null guard it could not see IS control flow. It is gone; what is left is not.
// size      126 bytes
// prototype void (__thiscall ??0NetMsg@@QAE@HHH@Z)(NetMsg* this, int, int, int)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok;frame
// calls     0x004048A0 0x006161D0
//
//        AND THE CALL SITES COST NOTHING, which took two attempts to get
//        right and is the part worth remembering. Written
//        `new (ptr) NetMsg(...)` in init_thunks.cpp, the two dynamic
//        initialisers pay the guard instead: 0x0047A7A0 went BYTE_EXACT ->
//        1 of 9 agreeing, 17 compiled instructions against an image of 9,
//        and the whole conversion was reverted as a bad trade on that basis.
//        It was not a bad trade, it was the wrong spelling. Written
//        `ptr->NetMsg::NetMsg(...)` - explicit constructor call syntax, no
//        new-expression, which is what every other initialiser in that file
//        already uses - there is no guard and both initialisers stay
//        BYTE_EXACT. `new (ptr) T()` and `ptr->T::T()` are not
//        interchangeable here: only the second is free.
Return Value: n/a
Status: Complete
*/
NetMsg::NetMsg(int a1, int a2, int a3) {
    field_5380_ = a2;
    field_537C_ = a1;
    uint32_t *const object = reinterpret_cast<uint32_t *>(this);
    object[0x000 / 4] = NetMsgPrimaryVtable;
    object[0x444 / 4] = NetMsgBufferVtable;
    field_5384_ = 0;
    field_5388_ = a3;
}

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

/*
Purpose: Step the receiver back to the subobject ??_GNetMsg@@UAEPAXI@Z expects,
         then forward unchanged.
// ORIGINAL: 0x0047AFE0 ??_GNetMsg@@WEEE@AEPAXI@Z 0x0047AFE0-0x0047AFEB
// symbol    ??_ENetMsg@@WEEE@AEPAXI@Z
// CORRECTED from ??3NetMsg@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x0047AD70` into
//   ??_GNetMsg@@UAEPAXI@Z, which executes `ret 4`; no stack access and
//   the receiver stays in ECX. `WEEE@` re-demangles to adjustor{1092}
//   and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/
