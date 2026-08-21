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
#include "spritebox.h"

/*
Purpose: Compose a StatusWin from its Caviar, three Font and one Spot
         subobjects. All four are ordinary typed members now (see the class
         comment in statuswin.h), so the implicit member-init sequence -
         built in declaration order, with no null-pointer guard around any
         of it - reproduces the image's four real constructor calls with an
         empty body.
// ORIGINAL: 0x004BA1A0 ??0StatusWin@@QAE@XZ 0x004BA1A0-0x004BA217;0x00659A60-0x00659A9F BYTE_EXACT
// LEVER: a placement-new version of this body (since replaced) measured 7/32 - matching the SEH prologue through instruction 6, then diverging on register allocation. Every `new (self+off) T()` VC6 guards with a null-pointer `cmp`/`je`, which the image's real member-init sequence never emits; real typed members in declaration order drop the guard entirely.
// size      182 bytes
// prototype void (__thiscall ??0StatusWin@@QAE@XZ)(StatusWin* this)
// callers   1   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005FA860 0x00616DA0 0x00618EA0
Return Value: n/a
Status: Complete
*/
StatusWin::StatusWin() {
}

/*
Purpose: Tear down a StatusWin. All five subobjects are ordinary typed
         members (see the class comment in statuswin.h), so the implicit
         destruction sequence - reverse declaration order: Spot, font3,
         font2, font1, then Caviar's own destructor, which forwards to
         close() - reproduces the image's exact five-call sequence with an
         empty body.
// ORIGINAL: 0x004BA120 ??1StatusWin@@QAE@XZ 0x004BA120-0x004BA19E BYTE_EXACT
// LEVER: an explicit-call version of this body (since replaced) measured 0/32 - the image opens `push ebp; mov ebp,esp; push -1; push 0x659a55`, a real SEH frame protecting the four-subobject unwind, which straight-line explicit `~Font()`/`~Spot()`/`close()` calls never reproduced. The compiler's own implicit per-member destruction gets that protection for free.
// size      126 bytes
// prototype void (__thiscall ??1StatusWin@@QAE@XZ)(StatusWin* this)
// callers   0   call targets   5
// kind      game
// flags     hidden;sp_ready;purged_ok
Return Value: n/a
Status: Complete
*/
StatusWin::~StatusWin() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x004B9F80 ?close@StatusWin@@QAEXXZ 0x004B9F80-0x004B9F81 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?close@StatusWin@@QAEXXZ)(StatusWin* this)
// callers   3   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
// ORIGINAL: 0x004B9F90 ?set_loc@StatusWin@@QAEXHH@Z 0x004B9F90-0x004B9FB8 BYTE_EXACT
// size      40 bytes
// prototype void (__thiscall ?set_loc@StatusWin@@QAEXHH@Z)(StatusWin* this, int, int)
// callers   8   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
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


/*
Purpose: Release the status window's hold on the interface mode, if it holds
         one. The original tail-jumps into the release, so nothing follows it.
// ORIGINAL: 0x004B8970 ?reset@StatusWin@@QAEXXZ 0x004B8970-0x004B898F BYTE_EXACT
// size      31 bytes
// prototype void (__thiscall ?reset@StatusWin@@QAEXXZ)(StatusWin* this)
// callers   8   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void StatusWin::reset() {
    if (field_15D4_) {
        field_15D4_ = 0;
        sub_interface_global()->release_iface_mode();
    }
}

void __fastcall status_win_reset_redirect(StatusWin *self, void *) {
    self->reset();
}
