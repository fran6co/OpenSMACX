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
// size      166 bytes
// prototype void (__thiscall ??0FlatButton@@QAE@XZ)(FlatButton* this)
// callers   17   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00606F30
*/

// The construction chain is `BaseButton::construct()`, not a base
// constructor call: BaseButton()/GraphicWin()/Win() are all empty inline
// stubs (the "construct method" idiom), and the image's single `call`
// targets 0x00606F30, which is `?construct@BaseButton@@QAEXXZ`.
//
// RULED-OUT (SEH frame): the image has NO unwind frame here (flags carry
// no `frame`... wait, no `frame` isn't the tell; the tell is the absence of
// a `push -1 / push handler / mov fs:[0]` prologue the way NetWin's own
// ctor has one). This tree's compiled body gets one anyway: `BaseButton`'s
// chain embeds `Buffer buffer_` / `Heap heap_` / `AutoSound auto_sound_` by
// VALUE, and Buffer's dtor is `virtual` and non-trivial (calls close()), so
// once BaseButton()/GraphicWin()/Win()'s (empty-bodied but still real)
// default construction of those members completes, VC6 must protect any
// later throwable call in this constructor's body against needing to run
// their destructors on unwind - unconditionally, regardless of whether
// `BaseButton::construct()` can really throw. The image proves the
// ORIGINAL source did not pay this: its ONLY call is to construct() (no
// separate call to Buffer's own real constructor at 0x005D7210 appears
// either), so those members were not real C++ value-subobjects there.
// Reworking `Buffer buffer_`/`Heap heap_`/`AutoSound auto_sound_` to
// pointer or raw-storage members to fix this is a layout change to
// GraphicWin/Win/BaseButton far outside this marker's scope, and risks
// regressing every already-measured method on those classes - left alone.
static int *const g_009b8e44 = (int *)0x009B8E44;

FlatButton::FlatButton() {
    BaseButton::construct();
    volatile uint32_t *const object =
        reinterpret_cast<volatile uint32_t *>(this);
    object[0x000 / 4] = FlatButtonPrimaryVtable;
    object[0x444 / 4] = FlatButtonBufferVtable;
    object[0xA18 / 4] = 0;
    object[0xA14 / 4] = 0;
    object[0xAB8 / 4] = 0xFFFFFFFFU;
    for (size_t index = 0; index < 3; ++index) {
        object[(0xABC / 4) + index] = 0xFFFFFFFFU;
        for (size_t group = 0; group < 9; ++group) {
            object[(0xAE0 / 4) + group * 3 + index] =
                static_cast<uint32_t>(g_009b8e44[group * 3 + index]);
        }
        object[(0xAD4 / 4) + index] = 0;
        object[(0xAC8 / 4) + index] = 0;
    }
}
