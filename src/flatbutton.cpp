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
#include "flatbutton.h"

const uint32_t FlatButtonPrimaryVtable = 0x00669754;
const uint32_t FlatButtonBufferVtable = 0x0066974C;
uint32_t FlatButtonDefaults;  // 0x009B8E44

/*
Purpose: Close the primary Time member, reset FlatButton orientation state,
         close the BaseButton base, and reload all nine three-entry tables.
// ORIGINAL: 0x00607DA0 ?close@FlatButton@@QAEXXZ 0x00607DA0-0x00607E41
// symbol    ?close@FlatButton@@QAEIXZ
// size      161 bytes
// prototype void (__thiscall ?close@FlatButton@@QAEXXZ)(FlatButton* this)
// callers   134   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006070C0 0x00616780
Return Value: Legacy final-loop residue (this + 0xAEC)
Status: Complete
*/
uint32_t FlatButton::close() {
    time1_.close();
    volatile uint32_t *const object =
        reinterpret_cast<volatile uint32_t *>(this);
    object[0xA18 / 4] = 0;
    object[0xA14 / 4] = 0;
    object[0xAB8 / 4] = 0xFFFFFFFFU;
    BaseButton::close();

    volatile uint32_t *const defaults =
        reinterpret_cast<volatile uint32_t *>(FlatButtonDefaults);
    for (size_t index = 0; index < 3; ++index) {
        object[(0xABC / 4) + index] = 0xFFFFFFFFU;
        for (size_t group = 0; group < 9; ++group) {
            object[(0xAE0 / 4) + group * 3 + index] =
                defaults[group * 3 + index];
        }
        object[(0xAD4 / 4) + index] = 0;
        object[(0xAC8 / 4) + index] = 0;
    }
    return reinterpret_cast<uintptr_t>(this) + 0xAECU;
}

uint32_t __fastcall flat_button_close_redirect(FlatButton *self, void *) {
    return self->close();
}

/*
Purpose: Destroy a FlatButton by installing its two virtual tables, closing
         the derived stage, then running the complete BaseButton destructor.
// ORIGINAL: 0x00406880 ??1FlatButton@@QAE@XZ 0x00406880-0x004068D8;0x006509A0-0x006509B2
// size      106 bytes
// prototype void (__thiscall ??1FlatButton@@QAE@XZ)(FlatButton* this)
// callers   93   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00607040 0x00607DA0
Return Value: Instance pointer in EAX
Status: Complete
*/
FlatButton *FlatButton::destroy() {
    volatile uint32_t *const object =
        reinterpret_cast<volatile uint32_t *>(this);
    object[0x000 / 4] = FlatButtonPrimaryVtable;
    object[0x444 / 4] = FlatButtonBufferVtable;
    close();
    BaseButton::destroy();
    return this;
}

FlatButton *__fastcall flat_button_destructor_redirect(
        FlatButton *self, void *) {
    return self->destroy();
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
// ORIGINAL: 0x00607CF0 ??0FlatButton@@QAE@XZ 0x00607CF0-0x00607D96
// body      src/flatbutton.h
// size      166 bytes
// prototype void (__thiscall ??0FlatButton@@QAE@XZ)(FlatButton* this)
// callers   17   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00606F30
*/
