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
#include "gamma.h"

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns 1.
// ORIGINAL: 0x005C9410 ?on_key_click@Gamma@@QAEHHH@Z 0x005C9410-0x005C9418 BYTE_EXACT
// size      8 bytes
// prototype int (__thiscall ?on_key_click@Gamma@@QAEHHH@Z)(Gamma* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 1, always
Status: Complete
*/
int Gamma::on_key_click(int, int) {
    return 1;
}

int __fastcall gamma_on_key_click_redirect(Gamma *self, void *, int a1, int a2) {
    return self->on_key_click(a1, a2);
}

func_gamma_adjust_palette GammaOriginalAdjustPalette =
    original_method<func_gamma_adjust_palette>(0x005C9520);

/*
// ORIGINAL: 0x005C9390 ?on_scrolled@Gamma@@QAEXHH@Z 0x005C9390-0x005C9398
// size      8 bytes
// prototype void (__thiscall ?on_scrolled@Gamma@@QAEXHH@Z)(Gamma* this, int, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005C9520
Status: Complete
*/
void Gamma::on_scrolled(int a1, int a2) {
    (ORIGINAL(this)->*GammaOriginalAdjustPalette)();
}


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
// ORIGINAL: 0x0062C010 ??1PushButton@@QAE@XZ 0x0062C010-0x0062C025
// body      src/pushbutton.h
// size      21 bytes
// prototype void (__thiscall ??1PushButton@@QAE@XZ)(PushButton* this)
// callers   7   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
*/
