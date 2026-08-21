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
#include "worldwin.h"

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


/*
// ORIGINAL: 0x005C9390 ?on_scrolled@Gamma@@QAEXHH@Z 0x005C9390-0x005C9398 BYTE_EXACT
// size      8 bytes
// prototype void (__thiscall ?on_scrolled@Gamma@@QAEXHH@Z)(Gamma* this, int, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005C9520
Status: Complete
*/
void Gamma::on_scrolled(int a1, int a2) {
    adjust_palette();
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

// Gamma's own virtual table addresses, installed by the constructor. The pair
// mirrors GraphicWin's exactly - primary at +0, the Buffer subobject's at
// +0x444, eight bytes apart - because Gamma inherits that layout unchanged.
const uint32_t GammaPrimaryVtable = 0x0066C3D8;
const uint32_t GammaBufferVtable = 0x0066C3D0;

/*
Purpose: Construct the gamma-correction dialog.
// ORIGINAL: 0x005C8DA0 ??0Gamma@@QAE@XZ 0x005C8DA0-0x005C8E60;0x00662A70-0x00662AC8 BYTE_EXACT
// size      280 bytes
// prototype void (__thiscall ??0Gamma@@QAE@XZ)(Gamma* this)
// callers   2   call targets   6
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005D4CF0 0x005FE2A0 0x006051D0 0x00614E50 0x00618EA0 0x0062BF20
Return Value: None
Status: Complete
*/
Gamma::Gamma() {
    // The vtable slots are compiler-managed, not ordinary members a class can
    // name - the same reason GraphicWin::construct writes its own pair at
    // these exact two offsets. Gamma has its own vtable because it overrides;
    // nothing in this chain is declared `virtual`, so the store is explicit
    // rather than emitted by a constructor VC6 would generate.
    //
    // LEVER: these two stores must come FIRST, before `gamma_ = 1.0`. Written
    //   after it the body is SHAPE_EXACT at 39/42; written before it, 42/42.
    //   The image interleaves them - `[esi+0xee8] = 0`, both vtables, then
    //   `[esi+0xeec] = 0x3ff00000` - which reads like the vtable stores land
    //   in the middle of the double, impossible to write as source. It is the
    //   other way round: VC6 hoists the double's ZERO half ahead of whatever
    //   precedes it and leaves the 0x3ff00000 half in place, so putting the
    //   vtable stores first is what produces the image's own order.
    uint32_t *const object = reinterpret_cast<uint32_t *>(this);
    object[0x000 / 4] = GammaPrimaryVtable;
    object[0x444 / 4] = GammaBufferVtable;
    gamma_ = 1.0;
    field_A64_ = 0;
}
