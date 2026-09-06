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
#include "council.h"
#include "datalink.h"
#include "statuswin.h"
#include "worldwin.h"
#include "time.h"
#include "font.h"
#include "pushbutton.h"
#include <cstring>

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
void InfoWin::on_right_click(int a, int b) {
    right_menu(a, b);
}

func_info_win_right_menu InfoWinOriginalRightMenu =
    original_method<func_info_win_right_menu>(0x004589C0);

// right_menu's own body has NOT homed yet: it calls help_any (0x0044CB30),
// which this TU now defines, but it also needs GraphicWin::update(RECT *,
// GraphicWin *), which is neither declared nor defined anywhere in the tree -
// see the HANDOFF in the report. So the out-of-line forwarder stays until that
// declaration lands. The auto_inline(off) is load-bearing and being defined
// after the caller is NOT enough on its own: VC6 at /Ob2 defers codegen to the
// end of the translation unit and would fold the forwarder back into
// on_right_click, losing the 20-byte BYTE_EXACT match.
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
// ORIGINAL: 0x00459500 ??0InfoWin@@QAE@XZ 0x00459500-0x00459556;0x00655260-0x00655280 BYTE_EXACT
// size      118 bytes
// prototype void (__thiscall ??0InfoWin@@QAE@XZ)(InfoWin* this)
// callers   1   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x006161D0 0x00618EA0 0x0062BF20
*/

// Time, Font and PushButton are owned members. Their adjusted receivers
// (+0x30, +0x58, +0x9D0) are explicit in the original construction sequence.
// Ordinary member initialization supplies construction and unwind cleanup.
InfoWin::InfoWin() {
}

/*
Purpose: Tear down an InfoWin: destroy the PushButton, Font and Time
         subobjects in reverse construction order.
// ORIGINAL: 0x004594A0 ??1InfoWin@@QAE@XZ 0x004594A0-0x004594FB;0x00655240-0x00655260
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
}

// ORIGINAL: 0x00459420 ?UNK5@InfoWin@@QAEXHHHHH@Z 0x00459420-0x0045949B;0x00655295-0x006552BA FILE
// working copy - scaffold materialised by --work
// size      160 bytes
// prototype void (__thiscall ?UNK5@InfoWin@@QAEXHHHHH@Z)(InfoWin* this, int, int, int, int, int)
// callers   0   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00618EA0 0x00618EE0
// UNK5 IS EVIDENCE-FREE AS A NAME. Searched 2026-09-05: the catalogue records
// callers 0 and call targets 0 for 0x00459420, so nothing names it from a call
// site; the mangled name is `QAE`, so it is not virtual and no vtable slot
// names it either; docs/recovery/behaviour-member-names.csv has no InfoWin
// entry; and the body is a stub - it builds three `Font`s, tears them down in
// reverse order, and returns through `ret 0x14` with all five arguments
// unread. A name invented to clear the census would be a false statement that
// outlives whoever wrote it.
// The five arguments stay unnamed on purpose: the body never reads one, so
// there is no evidence for any of them, and an unnamed parameter is exactly
// what the tree already writes for the same situation (BaseWin::UNK5(int),
// BaseWin::on_button_toggled(int, int)).
// MISMATCH 17 of 30: the image emits one more EH state store
// (`mov byte ptr [ebp - 4], 1` twice, not once) around the third Font than
// VC6 will produce for three flat locals in one scope. Not closed; the shape
// - three constructed then destroyed in reverse order - is the image's.
void InfoWin::UNK5(int, int, int, int, int) {
    Font font_a;
    Font font_b;
    Font font_c;
}

// ORIGINAL: 0x0044CB30 ?help_any@@YAXXZ 0x0044CB30-0x0044CB3F BYTE_EXACT
// size      15 bytes
// prototype 
// callers   5   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00429180
// 0x0044CB30  ?help_any@@YAXXZ  ->  ?help_any@@YAXXZ
//
// HOMING NOTE: the tree names the singleton, so the body spells it the way
// datalink.h already declares it - `DatalinkMain`, the object at 0x00703EA0 -
// rather than binding the address again under an anonymous name.

void __cdecl help_any() {
    DatalinkMain.exec(0, 0);
}
