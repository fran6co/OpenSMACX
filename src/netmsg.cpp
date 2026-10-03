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
#include "netmsg.h"
#include "spritebox.h"
#include "net_class.h"
#include "popup.h"
#include "time.h"
#include "alpha.h"
#include "buffer.h"
#include "console.h"
#include "game.h"
#include "maininterface.h"

// ORIGINAL: 0x0047A800 ?timer_daemon@NetMsg@@QAAXH@Z 0x0047A800-0x0047A85F
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/0047a800.cpp"], "reason": "Original callback token46 routes primary, other tokens route secondary; genuine static cdecl member matches native calling convention, catalogue identity preserved separately. Seed retained, no byte claim by binding."}
// symbol    ?timer_daemon@NetMsg@@CAXH@Z
void __cdecl NetMsg::timer_daemon(int token) {
    if (token == 0x46) {
        NetMsgPrimary.close();
    } else {
        NetMsgSecondary.close();
    }
}

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
//        `new (ptr) NetMsg(...)` in the retired init_thunks.cpp, the two dynamic
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
    field_5384_ = 0;
    field_5388_ = a3;
}

/*
Purpose: Close an outstanding network message - stop its timer and hide the
         popup - doing nothing when none is showing.
The historical forwarding notes below predate the separate timer lifecycle TU;
this definition now calls the actual Time member.
// ORIGINAL: 0x0047A7D0 ?close@NetMsg@@QAEXXZ 0x0047A7D0-0x0047A7FB BYTE_EXACT
// LEVER (2026-09-03): time_close_call, not timer_.close(). The image CALLS
//        0x00616780 here where time.h's in-class `MEASURED close` otherwise
//        expands at this site too - 42 compiled instructions against the
//        image's 12, 0/12 agreeing. flatbutton.cpp carries the same
//        forwarder for the same reason; this one is static because
//        flatbutton.cpp's is external and one name cannot be defined twice.
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

/*
Purpose: Stop the message timer and hide the popup when a message is active.
*/
// ORIGINAL: 0x0047A860 ?on_button_clicked@NetMsg@@QAEXH@Z 0x0047A860-0x0047A88D BYTE_EXACT
// symbol    ?on_button_clicked@NetMsg@@UAEXH@Z
// Win's recovered slot-44 declaration makes this a virtual override in the
// current class model; the original catalogue spelling is retained above.
void NetMsg::on_button_clicked(int) {
    if (field_5384_) {
        field_5384_ = 0;
        timer_.close();
        Popup::hide();
    }
}


/*
Purpose: Step the receiver back to the subobject ??_GNetMsg@@UAEPAXI@Z expects,
         then forward unchanged.
// ORIGINAL: 0x0047AFE0 ??_GNetMsg@@WEEE@AEPAXI@Z 0x0047AFE0-0x0047AFEB BYTE_EXACT
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

// Definition order follows the original NETMSG1, then NETMSG2 constructors.
// Static destruction is therefore NETMSG2, then NETMSG1, like the atexit list.
// Original constructor/teardown47A770/47A790 identify this object.
// Start-only original binding; complete allocation extent is not claimed.
// RECOVERY_OBJECT: v1 0x00805338 ?NetMsgPrimary@@3VNetMsg@@A
NetMsg NetMsgPrimary(4096, 8, 0);
// Original constructor/teardown47A7A0/47A7C0 identify this object.
// RECOVERY_OBJECT: v1 0x007FFF80 ?NetMsgSecondary@@3VNetMsg@@A
NetMsg NetMsgSecondary(-5, 40, 1);

bool NetMsg::is_active() const {
    return field_5384_ != 0;
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0047ADA0 ??1NetMsg@@QAE@XZ 0x0047ADA0-0x0047AFD2;0x00656460-0x0065654F
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/0047ada0.cpp"], "reason": "Complete genuine NetMsg owner implementation replaces executable aliases/forwarders; original seed establishes catalogue identity and spans, native emitted symbols are separately recorded. Full private application compile/link and both native flag sets checked; preserve seed and historical claims, no exactness granted by binding."}
// symbol    ??1NetMsg@@UAE@XZ
NetMsg::~NetMsg() {
    // Time is destroyed before the Popup base. Popup::~Popup calls close(),
    // then normal C++ member/base teardown handles Scroll and BasePop.
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0047A890 ?pop@NetMsg@@QAEXPBDHHPBD@Z 0x0047A890-0x0047ACE3;0x00656550-0x0065664C
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/0047a890.cpp"], "reason": "Complete genuine NetMsg owner implementation replaces executable aliases/forwarders; original seed establishes catalogue identity and spans, native emitted symbols are separately recorded. Full private application compile/link and both native flag sets checked; preserve seed and historical claims, no exactness granted by binding."}
// symbol    ?pop@NetMsg@@QAEXPBDHH0@Z
void NetMsg::pop(const char *label, int duration, int channel,
                 const char *target) {
    // The original constructs this local Popup before either mode branch.
    // It is the non-full dialog; the full branch operates on this NetMsg's
    // inherited Popup subobject instead.
    Popup popup;

    bool full = false;
    if (channel != 0 &&
        (channel <= 1 ||
         IfaceIdToSlotStorage[ActiveIfaceIdStorage] != 0)) {
        full = true;
    }
    if (!full && ConsoleGlobal.is_visible() == 0 && IsMultiplayerNet != 0) {
        full = true;
    }

    if (full) {
        if (field_5384_ != 0) {
            field_5384_ = 0;
            timer_.close();
            Popup::hide();
        }

        PopupModalState = 6;
        BasePop::init(0x100, 0);
        BasePop::set_loc(static_cast<int>(field_537C_),
                         static_cast<int>(field_5380_));
        if (Popup::start(PopupStartCaption, label, -1, nullptr, 0x180,
                         nullptr) == 0) {
            char button_label[64];
            button_label[0] = '\0';
            strcat(button_label, label_get(219));
            BasePop::button(button_label);

            field_5384_ = 1;
            BasePop::exec(0, 0);
            PopupModalState = 0;

            if (duration > 0) {
                timer_.start(NetMsg::timer_daemon, field_5388_ + 0x46,
                             static_cast<uint32_t>(duration), 0x3c);
            }
        }
        PopupModalState = 0;
        return;
    }

    popup.start(PopupStartCaption, label, -1, nullptr, 0, nullptr);

    // Native predicates: active is false only for a non-network game with a
    // negative duration (47ABD3..47ABE9).
    const bool active = IsMultiplayerNet != 0 || duration >= 0;
    popup.set_auto_hide_value(active ? 1u : 0u);
    if (target != nullptr && !active) {
        int width = 0;
        int height = 0;
        if (Buffer::get_pcx_dimensions(target, &width, &height) == 0 &&
            popup.owned_picture().init(const_cast<char *>(target), width,
                                       height) == 0) {
            popup.select_owned_picture();
        }
    }

    popup.BasePop::exec(0, 0);
}

// 0x0050B9E0 ignores its receiver and dispatches to the primary NetMsg.
// ORIGINAL: 0x0050B9E0 ?pop@NetMsg@@QAAXPBDPBD@Z 0x0050B9E0-0x0050B9FE
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/0050b9e0.cpp"], "reason": "Complete genuine NetMsg owner implementation replaces executable aliases/forwarders; original seed establishes catalogue identity and spans, native emitted symbols are separately recorded. Full private application compile/link and both native flag sets checked; preserve seed and historical claims, no exactness granted by binding."}
// symbol    ?pop@NetMsg@@QAEXPBD0@Z
void NetMsg::pop(const char *label, const char *target) {
    NetMsgPrimary.pop(label, -5000, 0, target);
}
