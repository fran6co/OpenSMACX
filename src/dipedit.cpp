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
#include "dipedit.h"
#include "statuswin.h"
#include "worldwin.h"
#include "checkbox.h"
#include "vtable_shim.h"
#include "spying_recovery.h"  // SpyingStatusTable, SpyingStatusStride
#include "faction.h"  // DTREATY_* treaty bit constants

/*
// ORIGINAL: 0x004E2240 ??1DipEdit@@QAE@XZ 0x004E2240-0x004E23BA;0x0065C570-0x0065C60C
// body      src/dipedit.h
// symbol    ??1DipEdit@@UAE@XZ
//   the catalogue spells the plain dtor `QAE`; this compiler emits it
//   `UAE` (virtual through virtual ~AutoSound), as for ??1FlatButton,
//   whose image body is likewise Q-catalogued and U-emitted.
// size      378 bytes
// prototype void (__thiscall ??1DipEdit@@QAE@XZ)(DipEdit* this)
// callers   2   call targets   7
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005D4DD0 0x00607040 0x00607DA0 0x00608E10 0x00609EC0 0x0060E740 0x00618EE0
Return Value: n/a

The body is EMPTY (DipEdit::~DipEdit() below) - every byte of 0x004E2240 is
the compiler's, generated from the member declarations: five
expanded ~FlatButtons (0x4DF8 down to 0x20C8, each the 0x669754/0x66974c
stage pair plus `call ?close@FlatButton` plus `call ??1BaseButton`), the
CheckBox member (??1CheckBox entered on its GraphicWin vbase at 0x15bc,
then ??1Dialog at 0x1fd4 and ??1GraphicWin at 0x15bc - its virtual bases),
the ListBox member (??1ListBox entered on 0xa94, ??1Dialog at 0x14ac,
??1GraphicWin at 0xa94), the Font member, and the GraphicWin base. The
EH states in the image's frame (0x8/0x7, 0x9/0x6, 0xa/0x5, 0xb/0x4, 0xc/0x3,
0x2, 0x1, 0x0, -1) step through those same teardowns in that same order.

The image has no standalone ??0DipEdit to pair with it: the constructor is
fully inlined into its only caller (sub_4DB870, 0x004DB870), which builds a
stack DipEdit, calls ?init@DipEdit@@QAEXH@Z on it, and destroys it with the
??1DipEdit recovered here.
*/

/*
Purpose: The compiler-generated scalar deleting destructor at
         ??_GDipEdit@@UAEPAXI@Z: run the complete destructor, then release
         the storage through the executable's operator delete only when bit 0
         of the flags asks. COMPILER-EMITTED from the real ~DipEdit below;
         the hand shim this claim used to live on (deleting_thunks.cpp's
         scalar_delete_dip_edit) is retired with the emission, the same move
         the adjustor above made.
// ORIGINAL: 0x004E23C0 ??_GDipEdit@@UAEPAXI@Z 0x004E23C0-0x004E23E1 BYTE_EXACT
// symbol    ??_GDipEdit@@UAEPAXI@Z
// size      33 bytes
// prototype void* (__thiscall ??_GDipEdit@@UAEPAXI@Z)(DipEdit* this, unsigned int)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004E2240 0x0064557F
Return Value: the object pointer
Status: Complete
*/

/*
Purpose: Step the receiver back to the subobject ??_GDipEdit@@UAEPAXI@Z
         expects, then forward unchanged. COMPILER-EMITTED from the real
         ~DipEdit below (the forwarder era emitted it from
         pending_bodies.cpp; the marker moved with the emission).
// ORIGINAL: 0x004E23F0 ??_GDipEdit@@WEEE@AEPAXI@Z 0x004E23F0-0x004E23FB BYTE_EXACT
// symbol    ??_EDipEdit@@WEEE@AEPAXI@Z
// CORRECTED from ??3DipEdit@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x004E23C0` into
//   ??_GDipEdit@@UAEPAXI@Z, which executes `ret 4`; no stack access and
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

DipEdit::~DipEdit() {
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
// ORIGINAL: 0x004D9DC0 ?on_left_click@DipEdit@@QAEXHH@Z 0x004D9DC0-0x004D9DC3 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_left_click@DipEdit@@QAEXHH@Z)(DipEdit* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DipEdit::on_left_click(int, int) {
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
// ORIGINAL: 0x004D9DD0 ?on_redraw@DipEdit@@QAEXXZ 0x004D9DD0-0x004D9DD1 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?on_redraw@DipEdit@@QAEXXZ)(DipEdit* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DipEdit::on_redraw() {
}



/*
// ORIGINAL: 0x004DA740 ?on_selected@DipEdit@@QAEXH@Z 0x004DA740-0x004DA760 BYTE_EXACT
// size      32 bytes
// prototype void (__thiscall ?on_selected@DipEdit@@QAEXH@Z)(DipEdit* this, int)
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004DA990 0x004DADA0
Status: Complete
*/
// The int is an INDEX: the body stores it in selected_target_, which then indexes
// SpyingStatusTable[faction_index_ * SpyingStatusStride + selected_target_].
void DipEdit::on_selected(int index) {
    read_check();
    selected_target_ = index;
    do_check();
}



/*
Purpose: Sync eleven checkbox bits on the embedded CheckBox to the current
         diplomatic status word (SpyingStatusTable[subject * 2099 +
         faction]) - one set_state_id call per treaty flag - then redraw the
         CheckBox subobject through its own vtable-relative slot062. Each of
         the twelve set_state_id calls RE-READS faction_index_/selected_target_ from
         `this` rather than reusing a cached value, matching the image.
// ORIGINAL: 0x004DADA0 ?do_check@DipEdit@@QAEXXZ 0x004DADA0-0x004DB071 BYTE_EXACT
// size      721 bytes
// prototype void (__thiscall ?do_check@DipEdit@@QAEXXZ)(DipEdit* this)
// callers   3   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x0060EB80
// indirect  0x004DB068
Return Value: n/a
Status: Complete
*/
void DipEdit::do_check() {
    CheckBox *const cb = &check_box_;

    cb->set_state_id(DTREATY_COMMLINK,
        SpyingStatusTable[faction_index_ * SpyingStatusStride
            + selected_target_] & DTREATY_COMMLINK);
    cb->set_state_id(DTREATY_VENDETTA,
        SpyingStatusTable[faction_index_ * SpyingStatusStride
            + selected_target_] & DTREATY_VENDETTA);
    cb->set_state_id(DTREATY_TRUCE,
        SpyingStatusTable[faction_index_ * SpyingStatusStride
            + selected_target_] & DTREATY_TRUCE);
    cb->set_state_id(DTREATY_TREATY,
        SpyingStatusTable[faction_index_ * SpyingStatusStride
            + selected_target_] & DTREATY_TREATY);
    cb->set_state_id(DTREATY_PACT,
        SpyingStatusTable[faction_index_ * SpyingStatusStride
            + selected_target_] & DTREATY_PACT);
    cb->set_state_id(DTREATY_HAVE_INFILTRATOR,
        SpyingStatusTable[faction_index_ * SpyingStatusStride
            + selected_target_] & DTREATY_HAVE_INFILTRATOR);
    cb->set_state_id(DTREATY_WANT_TO_TALK,
        SpyingStatusTable[faction_index_ * SpyingStatusStride
            + selected_target_] & DTREATY_WANT_TO_TALK);
    cb->set_state_id(DTREATY_WANT_REVENGE,
        SpyingStatusTable[faction_index_ * SpyingStatusStride
            + selected_target_] & DTREATY_WANT_REVENGE);
    cb->set_state_id(DTREATY_HAVE_SURRENDERED,
        SpyingStatusTable[faction_index_ * SpyingStatusStride
            + selected_target_] & DTREATY_HAVE_SURRENDERED);
    cb->set_state_id(DTREATY_SHALL_BETRAY,
        SpyingStatusTable[faction_index_ * SpyingStatusStride
            + selected_target_] & DTREATY_SHALL_BETRAY);
    cb->set_state_id(DTREATY_ATROCITY_VICTIM,
        SpyingStatusTable[faction_index_ * SpyingStatusStride
            + selected_target_] & DTREATY_ATROCITY_VICTIM);

    // The second table view, eight uint32_t elements (0x20 bytes) further
    // into the same SpyingStatusTable storage.
    cb->set_state_id(static_cast<int>(0x80000000U) | DTREATY_COMMLINK,
        SpyingStatusTable[faction_index_ * SpyingStatusStride
            + selected_target_ + 8] & DTREATY_COMMLINK);
    cb->set_state_id(static_cast<int>(0x80000000U) | DTREATY_WANT_TO_TALK,
        SpyingStatusTable[faction_index_ * SpyingStatusStride
            + selected_target_ + 8] & DTREATY_WANT_TO_TALK);

    // The redraw goes through the embedded CheckBox's own virtual dispatch
    // (win-table slot 62). Re-derived here rather than through the cached
    // `cb`: the image folds the +0x15A0 into the call's own receiver
    // computation (`lea ecx, [edx + esi + 0x15a0]`).
    check_box_.vslot_62();
}
