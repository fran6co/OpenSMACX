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
// RULED-OUT: SEH frame - the image has NO unwind frame here (flags carry
// no `frame`, unlike Popup::Popup() at 0x004048A0 which does). This tree's
// compiled body gets one anyway - `push -1 / push handler / mov eax,
// fs:[0] / push eax / mov fs:[0], esp` at instruction 0.
//
// MEASURED 2026-08-21, correcting an earlier version of this note that
// blamed `Buffer buffer_` / `Heap heap_` / `AutoSound auto_sound_` being
// held BY VALUE. That is not it: swapping all three (plus BaseButton's own
// `Time time1_`/`time2_`) for raw `unsigned char[sizeof(T)]` storage the
// whole way up the chain - GraphicWin, Win, BaseButton at once - does NOT
// drop the frame. It is still there, byte for byte, with every by-value
// member gone.
//
// What actually does it, isolated with a direct `/GX-` recompile of this
// file (`tools/listing_diff.py 0x00607CF0 --flags '/c /O2 /Gy /GR- /GX-'`):
// disabling C++ EH removes the frame outright (45 instructions instead of
// 62, first divergence moves from instruction 0 to instruction 1). So the
// trigger is `/GX` plus a REAL derived-class constructor whose base
// (`BaseButton`, transitively `Win`) has a destructor the compiler cannot
// see is trivial - `Win::~Win()` is declared, not `{ ; }`, and does real
// external teardown at 0x005EBC90, so it can never BE trivial - combined
// with a call in the constructor body (`BaseButton::construct()`) that VC6
// assumes can throw. Declaring that call `__declspec(nothrow)` or
// `throw()` at its `basebutton.h` declaration does NOT suppress the frame
// either (measured both ways); VC6 does not appear to trust a caller-side
// nothrow annotation for this decision.
//
// `/GX-` WOULD NOT HELP EVEN IF IT WERE REACHABLE, which is a stronger
// statement than the one this note used to make, and it is measured rather
// than argued. Added as a per-function flag axis and scored against every
// other set: this body's best remains `/O1 /GX` at 0.565 similar, while the
// four `/GX-` variants land at 0.440, 0.465, 0.482 and 0.513. Removing the
// frame moves every later instruction, so positional agreement gets WORSE
// even though the instruction count gets closer - 50 against the image's 41
// under `/GX-`, where `/GX` overshoots. `Scroll::Scroll` (0x006051D0) says
// the same thing more sharply: `/GX-` compiles 79 instructions against an
// image of 83 and still scores 0.848, against 0.959 for `/O2 /Ob0 /GX`.
// Fewer instructions is not a better match. The axis was reverted rather
// than kept, because it costs 40% more measurement on every gate run and
// wins nothing.
//
// `/GX-` is also not reachable from here: it is a whole-translation-unit flag,
// outside `src/`, and it is not free even in principle. `Buffer::~Buffer()`
// (0x005D7410, `buffer.h`, inline `MEASURED virtual ~Buffer() { close(); }`)
// is currently BYTE_EXACT (20/20 instructions) specifically because the
// IMAGE's own `??1Buffer@@QAE@XZ` carries this exact SEH prologue; a direct
// `/GX-` recompile of that same address drops it from 20/20 agreeing to a
// near-total mismatch (image 20 instructions, compiled 8). So the ORIGINAL
// project genuinely needs `/GX` elsewhere in this same header's closure,
// and turning it off to fix this constructor would regress an existing
// claim. Left as a MISMATCH; the ORIGINAL's own FlatButton constructor must
// have reached `BaseButton::construct()` some other way that avoids this
// entirely - not found by any of the above. Contrast Popup::Popup() at
// 0x004048A0 (see popup.cpp): real implicit base+member construction
// (`BasePop::BasePop()`, `Scroll::Scroll()`), and the image DOES carry a
// frame there, correctly reproduced. FlatButton/PullDown's constructors
// are catalogued WITHOUT one, and that absence is not reproduced by any
// src/-only change tried here.
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
