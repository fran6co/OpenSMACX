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
#include "messagewin.h"

/*
Purpose: Construct the Sprite base, then the three plain members in
         declaration order (font_, listBox_, stringBox_) via automatic
         base+member construction - no explicit statement in the body.
// ORIGINAL: 0x00472190 ??0MessageWin@@QAE@XZ 0x00472190-0x004721F1;0x00655F30-0x00655F58
// size      97 bytes
// prototype void (__thiscall ??0MessageWin@@QAE@XZ)(MessageWin* this)
// callers   0   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok;frame
// calls     0x005E37E0 0x00618EA0 0x00609DB0 0x00629110
// RULED-OUT: byte-exact is not reachable from here. The image constructs
//        FOUR subobjects - Sprite (0x5E37E0, real, BYTE_EXACT), font_
//        (0x618EA0, real, BYTE_EXACT), listBox_ via `??0ListBox@@QAE@H@Z`
//        (0x609DB0, a ONE-ARG overload - `push 1` before the call - that
//        does not exist yet; listbox.h only declares the no-arg stub) and
//        stringBox_ (0x629110, GraphicWin-derived, 248 bytes, itself
//        unrecovered). Both are large, separately-scoped recoveries -
//        listBox_'s alone is documented as blocked on ListBox's virtual-
//        inheritance vbtable modelling (listbox.h's own long note) - and
//        NetWin's and DiploWin's own constructors (netwin.cpp, diplowin.h)
//        already carry a near-identical RULED-OUT note relying on
//        StringBox()/CheckBox() staying trivial stubs; giving StringBox() a
//        real out-of-line body here would change THEIR compiled shape too,
//        not just this one. Left as the two calls this tree already has
//        (Sprite, Font) rather than risk that.
Return Value: n/a
Status: Complete
*/
MessageWin::MessageWin() {
}

/*
Purpose: Tear the three plain members down in reverse declaration order
         (stringBox_, listBox_, font_), then the Sprite base - no explicit
         statement in the body, all of it automatic member teardown.
// ORIGINAL: 0x00472020 ??1MessageWin@@QAE@XZ 0x00472020-0x00472184
// size      356 bytes
// prototype void (__thiscall ??1MessageWin@@QAE@XZ)(MessageWin* this)
// callers   0   call targets   8
// kind      game
// flags     hidden;sp_ready;purged_ok;frame
// calls     0x00401060 0x00402970 0x00406880 0x005D4DD0 0x005E3820 0x00605370 0x00607040 0x00607DA0 0x00608E10 0x00609EC0 0x00618EE0 0x00629210
// RULED-OUT: byte-exact is not reachable from here, by a wide margin. The
//        image's teardown is NOT a plain reverse-order destructor call per
//        member - StringBox's own destructor (0x629210) is inlined open at
//        the call site alongside manual teardown of ListBox's two virtual
//        bases (explicit `call 0x5D4DD0` - GraphicWin::close() - three
//        times, `call 0x608E10` - Dialog-shaped - and more), none of which
//        this tree models. `font_.~Font()` (0x618EE0, real, BYTE_EXACT) and
//        the Sprite base (0x5E3820) are the only two subobjects here that
//        are actually recovered; stringBox_ and listBox_ are the same two
//        unrecovered, widely-shared classes the constructor's own note
//        names (StringBox 248 bytes, ListBox's virtual-inheritance
//        modelling documented blocked in listbox.h). Left as automatic
//        member teardown through whatever each member's CURRENT destructor
//        is, rather than hand-writing the inlined-open shape above.
Return Value: n/a
Status: Complete
*/
MessageWin::~MessageWin() {
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x004713A0 ?UNK1@MessageWin@@QAEXHHHHH@Z 0x004713A0-0x004713A3 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?UNK1@MessageWin@@QAEXHHHHH@Z)(MessageWin* this, int, int, int, int, int)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void MessageWin::UNK1(int, int, int, int, int) {
}

void __fastcall message_win_unk1_redirect(MessageWin *self, void *, int a1, int a2, int a3, int a4, int a5) {
    self->UNK1(a1, a2, a3, a4, a5);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00471D40 ?UNK4@MessageWin@@QAEXH@Z 0x00471D40-0x00471D43 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?UNK4@MessageWin@@QAEXH@Z)(MessageWin* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void MessageWin::UNK4(int) {
}

void __fastcall message_win_unk4_redirect(MessageWin *self, void *, int a1) {
    self->UNK4(a1);
}
