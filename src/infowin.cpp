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
#include "infowin.h"
#include "statuswin.h"
#include "worldwin.h"
#include "time.h"
#include "font.h"
#include "pushbutton.h"
#include <new>

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x004562F0 ?UNK1@InfoWin@@QAEXXZ 0x004562F0-0x004562F1 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?UNK1@InfoWin@@QAEXXZ)(InfoWin* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void InfoWin::UNK1() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x00458910 ?on_redraw@InfoWin@@QAEXXZ 0x00458910-0x00458911 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?on_redraw@InfoWin@@QAEXXZ)(InfoWin* this)
// callers   6   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void InfoWin::on_redraw() {
}

void __fastcall info_win_unk1_redirect(InfoWin *self, void *) {
    self->UNK1();
}

void __fastcall info_win_on_redraw_redirect(InfoWin *self, void *) {
    self->on_redraw();
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00458900 ?change@InfoWin@@QAEXH@Z 0x00458900-0x00458903 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?change@InfoWin@@QAEXH@Z)(InfoWin* this, int)
// callers   10   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void InfoWin::change(int) {
}

void __fastcall info_win_change_redirect(InfoWin *self, void *, int a1) {
    self->change(a1);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00458920 ?UNK3@InfoWin@@QAEXHH@Z 0x00458920-0x00458923 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?UNK3@InfoWin@@QAEXHH@Z)(InfoWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void InfoWin::UNK3(int, int) {
}

void __fastcall info_win_unk3_redirect(InfoWin *self, void *, int a1, int a2) {
    self->UNK3(a1, a2);
}


/*
Purpose: Cancel the pending info-window timer, if one is outstanding.
// ORIGINAL: 0x00459280 ?reset@InfoWin@@QAEXXZ 0x00459280-0x0045929C BYTE_EXACT
// size      28 bytes
// prototype void (__thiscall ?reset@InfoWin@@QAEXXZ)(InfoWin* this)
// callers   7   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00459150
Return Value: n/a
Status: Complete
*/
void InfoWin::reset() {
    if (field_A8_) {
        field_A8_ = 0;
        timer_proc(1);
    }
}

void __fastcall info_win_reset_redirect(InfoWin *self, void *) {
    self->reset();
}

/*
// ORIGINAL: 0x00458D40 ?on_right_click@InfoWin@@QAEXHH@Z 0x00458D40-0x00458D54 BYTE_EXACT
// size      20 bytes
// prototype void (__thiscall ?on_right_click@InfoWin@@QAEXHH@Z)(InfoWin* this, int, int)
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004589C0
Status: Complete
*/
void InfoWin::on_right_click(int a1, int a2) {
    right_menu(a1, a2);
}

func_info_win_right_menu InfoWinOriginalRightMenu =
    original_method<func_info_win_right_menu>(0x004589C0);

/*
Purpose: Build and run the tile context menu.
Forwards To: 004589C0
Return Value: n/a
Status: Original dependency - forwards to the original image.

The auto_inline(off) is load-bearing, and being defined after the caller is
NOT enough on its own. VC6 at /Ob2 defers codegen to the end of the
translation unit, so it happily folds a forwarder defined later in the file
back into a caller defined earlier: measured, on_right_click came out as

    mov eax,[esp+8] / mov edx,[esp+4] / push eax / push edx
    call dword ptr [InfoWinOriginalRightMenu] / ret 8

which is the seam inlined and the 20-byte BYTE_EXACT match
(byte-match.csv:3554) gone. With the pragma the caller keeps its
`push a2; push a1; call rel32` and this body stays a separate `jmp`-shaped
thunk.
*/
#pragma auto_inline(off)
void InfoWin::right_menu(int a1, int a2) {
    (ORIGINAL(this)->*InfoWinOriginalRightMenu)(a1, a2);
}
#pragma auto_inline(on)


// ---------------------------------------------------------------------------
// DEFINED IN THE HEADER, CLAIMED HERE.
//
// These pieces are written in-class so the image's own inlining reproduces -
// a constructor or destructor the compiler is expected to fold into its
// caller. A marker cannot live beside them: `decomp.reader` globs `*.cpp`
// and `*.c`, and a comparison compiles a TRANSLATION UNIT, so a marker in a
// header could be neither read nor measured. VC6 emits each of them into
// this unit's object as its own COMDAT anyway, which is what the comparison
// pulls out, and the `body` fact says where to go to edit one.
//
// The ratchet still covers the header: this unit includes it, so breaking an
// in-class body here fails the claim below. Measured, not assumed.
// ---------------------------------------------------------------------------

/*
// ORIGINAL: 0x00459500 ??0InfoWin@@QAE@XZ 0x00459500-0x00459556;0x00655260-0x00655280
// size      118 bytes
// prototype void (__thiscall ??0InfoWin@@QAE@XZ)(InfoWin* this)
// callers   1   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x006161D0 0x00618EA0 0x0062BF20
*/

// TRIED: byte-exact is not reachable from this marker alone.
// PushButton::PushButton() (0x0062BF20, at field_9D0_) is still an
// unrecovered stub (src/unrecovered/0062bf20.cpp), so the image's third
// call cannot be reproduced from here; not one of this pass's seven
// targets. Time (0x006161D0) and Font (0x00618EA0) are both already
// CLAIMED, so those two placement-new calls should match.
InfoWin::InfoWin() {
    char *const self = reinterpret_cast<char *>(this);
    new (self + 0x30) Time();
    new (self + 0x58) Font();
    new (self + 0x9D0) PushButton();
}

/*
Purpose: Tear down an InfoWin: destroy the PushButton, Font and Time
         subobjects in reverse construction order.
// ORIGINAL: 0x004594A0 ??1InfoWin@@QAE@XZ 0x004594A0-0x004594FB;0x00655240-0x00655260
// TRIED: MEASURED 0/26 agreeing. The image opens `push ebp; mov
//            ebp,esp; push -1; push 0x655256` - a real SEH frame - which
//            this straight-line body does not reproduce; every instruction
//            is offset from #0. Time/Font/PushButton here are all raw
//            offsets (not real typed members, per the constructor's own
//            `new (self+off) T()` shape), so there is no member for the
//            compiler to hang automatic unwind protection on the way
//            UV2Player::~UV2Player's real `Buffer buffer_` member does.
// size      123 bytes
// prototype void (__thiscall ??1InfoWin@@QAE@XZ)(InfoWin* this)
// callers   0   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00616200 0x00618EE0 0x0062C010
Return Value: n/a
Status: Complete
*/
InfoWin::~InfoWin() {
    char *const self = reinterpret_cast<char *>(this);
    reinterpret_cast<PushButton *>(self + 0x9D0)->~PushButton();
    reinterpret_cast<Font *>(self + 0x58)->~Font();
    reinterpret_cast<Time *>(self + 0x30)->~Time();
}
