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
Purpose: Apply one treaty bit's checkbox to the diplomatic status the dialog is
         editing. Ask the embedded CheckBox whether the item whose id is that
         bit is checked, then compare with the status word -
         SpyingStatusTable[faction_index_ * SpyingStatusStride +
         selected_target_], which is PlayersData[faction_index_]
         .diplo_treaties[selected_target_] (sizeof(PlayerData) == 0x20CC ==
         2099 * 4, and diplo_treaties sits at 0x18). Checked and the bit clear
         -> treaty_on; unchecked and the bit set -> treaty_off; a checkbox that
         already agrees with the status word does nothing. The bit is both the
         checkbox id and the mask, so a caller names the DTREATY_* constant
         once.
// ORIGINAL: 0x004DA760 ?UNK1@DipEdit@@QAEXH@Z 0x004DA760-0x004DA7E0 BYTE_EXACT
// size      128 bytes
// prototype void (__thiscall ?UNK1@DipEdit@@QAEXH@Z)(DipEdit* this, int)
// callers   0   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0055B760 0x0055B870 0x0060EC10
// LEVER: the idiomatic spelling is exact first try - real
//        `check_box_.get_state_id(bit)` plus the very subscript do_check
//        uses - where the src/unrecovered/004da760.cpp scaffold this body was
//        homed from sat NOT_MATCHING on its `(char *)this + 0xa1c` form. Worth
//        one line because the index lea chain here is NOT do_check's: it folds
//        (selected + faction) in before the *2 instead of adding the member
//        after. Same source, different schedule - so the schedule is context,
//        not something to chase with a spelling.
Return Value: n/a
Status: Complete
*/
void DipEdit::UNK1(int bit) {
    if (check_box_.get_state_id(bit)) {
        if (!(SpyingStatusTable[faction_index_ * SpyingStatusStride
            + selected_target_] & bit)) {
            treaty_on(faction_index_, selected_target_, bit);
        }
    } else {
        if (SpyingStatusTable[faction_index_ * SpyingStatusStride
            + selected_target_] & bit) {
            treaty_off(faction_index_, selected_target_, bit);
        }
    }
}



/*
Purpose: UNK1 for the AGENDA word. The checkbox id carries 0x80000000, which
         do_check also does when it writes the second table view back, and
         that view is diplo_agenda - 8 dwords past diplo_treaties in
         PlayerData, which is what moves the table base 0x96C9F8 -> 0x96CA18.
         The writers are agenda_on/agenda_off rather than
         treaty_on/treaty_off.
// ORIGINAL: 0x004DA7E0 ?UNK2@DipEdit@@QAEXH@Z 0x004DA7E0-0x004DA867 BYTE_EXACT
// size      135 bytes
// prototype void (__thiscall ?UNK2@DipEdit@@QAEXH@Z)(DipEdit* this, int)
// callers   1   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0055B820 0x0055BA80 0x0060EC10
// LEVER: naming the checkbox id in a local ahead of the call - the idiom
//        do_check already uses for these ids - is exact first try, and it is
//        also what the image's order shows: `or eax, 0x80000000` sits BEFORE
//        the `lea`/`push`, not inside the argument expression.
Return Value: n/a
Status: Complete
*/
void DipEdit::UNK2(int bit) {
    int id = static_cast<int>(0x80000000U) | bit;

    if (check_box_.get_state_id(id)) {
        if (!(SpyingStatusTable[faction_index_ * SpyingStatusStride
            + selected_target_ + 8] & bit)) {
            agenda_on(faction_index_, selected_target_, bit);
        }
    } else {
        if (SpyingStatusTable[faction_index_ * SpyingStatusStride
            + selected_target_ + 8] & bit) {
            agenda_off(faction_index_, selected_target_, bit);
        }
    }
}



/*
Purpose: UNK1 narrowed to ONE direction. Same checkbox id and same treaty word
         as UNK1, but the writer is set_treaty(faction_index_, selected_target_
         , bit, TRUE/FALSE) - which writes only PlayersData[faction_index_]
         .diplo_treaties[selected_target_] and leaves the mirror entry alone -
         and it is called only when it would change something, where UNK1's
         writers decide that for themselves.
// ORIGINAL: 0x004DA870 ?UNK3@DipEdit@@QAEXH@Z 0x004DA870-0x004DA8F4 BYTE_EXACT
// size      132 bytes
// prototype void (__thiscall ?UNK3@DipEdit@@QAEXH@Z)(DipEdit* this, int)
// callers   1   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0055BB30 0x0060EC10
// LEVER: `TRUE`/`FALSE` as set_treaty's fourth argument are what push the 1/0
//        the image pushes; no local for the boolean is wanted.
Return Value: n/a
Status: Complete
*/
void DipEdit::UNK3(int bit) {
    if (check_box_.get_state_id(bit)) {
        if (!(SpyingStatusTable[faction_index_ * SpyingStatusStride
            + selected_target_] & bit)) {
            set_treaty(faction_index_, selected_target_, bit, TRUE);
        }
    } else {
        if (SpyingStatusTable[faction_index_ * SpyingStatusStride
            + selected_target_] & bit) {
            set_treaty(faction_index_, selected_target_, bit, FALSE);
        }
    }
}



/*
Purpose: UNK3 for the AGENDA word: the 0x80000000 checkbox id, the +8 dword
         view, and set_agenda as the writer instead of set_treaty.
// ORIGINAL: 0x004DA900 ?UNK4@DipEdit@@QAEXH@Z 0x004DA900-0x004DA98B BYTE_EXACT
// size      139 bytes
// prototype void (__thiscall ?UNK4@DipEdit@@QAEXH@Z)(DipEdit* this, int)
// callers   1   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0055BBA0 0x0060EC10
// LEVER: answers the TRIED the archive this body was homed from carried -
//        src/unrecovered/004da900.cpp, whose raw-pointer translation
//        ("(char *)this + 0xa1c", fixed globals) plateaued at "MISMATCH #4
//        'mov' vs 'push'". The member spelling - real
//        `check_box_.get_state_id(id)`, real set_agenda, the subscript -
//        is exact first try; no pointer arithmetic anywhere.
Return Value: n/a
Status: Complete
*/
void DipEdit::UNK4(int bit) {
    int id = static_cast<int>(0x80000000U) | bit;

    if (check_box_.get_state_id(id)) {
        if (!(SpyingStatusTable[faction_index_ * SpyingStatusStride
            + selected_target_ + 8] & bit)) {
            set_agenda(faction_index_, selected_target_, bit, TRUE);
        }
    } else {
        if (SpyingStatusTable[faction_index_ * SpyingStatusStride
            + selected_target_ + 8] & bit) {
            set_agenda(faction_index_, selected_target_, bit, FALSE);
        }
    }
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
