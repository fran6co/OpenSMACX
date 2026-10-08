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
#include "palette.h"
#include "interface_palette.h"
#include "statuswin.h"
#include "worldwin.h"
#include "checkbox.h"
#include "vtable_shim.h"
#include "faction.h"  // DTREATY_* treaty bit constants
#include "alpha.h"     // Labels/label_get - the atrocity captions
#include "popup.h"     // the local Popup, PopupStartCaption, PopupModalState
#include "random.h"    // random()
#include "strings.h"   // StringTemp, StringTable
#include "general.h"
#include "xpops.h"     // X_pop_ask_number

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
         the former hand shim `scalar_delete_dip_edit` was retired when the
         compiler-generated entry became source-owned, the same move
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
// symbol    ?on_left_click@DipEdit@@UAEXHH@Z
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
// symbol    ?on_redraw@DipEdit@@UAEXXZ
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
Purpose: Push the checkboxes OUT to the diplomacy status word - the write-back
         half of the do_check pair, and the reason on_selected commits the
         outgoing target's edits before it stores the new index (DipEdit::init
         is the other caller). For the first seven treaty bits the dialog
         edits it reads the box whose id IS the bit - check_box_.get_state_id
         at 0x0060EC10, set_state_id's read-side twin - and settles that bit
         against SpyingStatusTable[faction_index_ * SpyingStatusStride +
         selected_target_]: checked against a clear bit calls treaty_on (or
         set_treaty .., TRUE), unchecked against a set bit calls treaty_off
         (or set_treaty .., FALSE). HAVE_INFILTRATOR and WANT_TO_TALK spell
         the same decision through set_treaty; the four bits after them go
         through the one-flag helpers - set_treaty_bit,
         set_agenda_bit and sync_agenda_bit, this pattern folded
         into methods of their own. The unconditional tail applies
         three consequences of the word the commit left behind: a
         HAVE_SURRENDERED opponent keeps the PACT and drops the 0xc20 treaty
         mask and the FIGHT_TO_DEATH agenda; a COMMLINK in the +8 view of the
         same table earns the VENDETTA; a clear WANT_REVENGE drops the 0xc00
         agenda.
// ORIGINAL: 0x004DA990 ?read_check@DipEdit@@QAEXXZ 0x004DA990-0x004DAD9B BYTE_EXACT
// size      1035 bytes
// prototype void (__thiscall ?read_check@DipEdit@@QAEXXZ)(DipEdit* this)
// callers   2   call targets   8
// kind      game
// flags     sp_ready;purged_ok
// calls     0x004DA7E0 0x004DA870 0x004DA900 0x0055B760 0x0055B870 0x0055BB30 0x0055BBA0 0x0060EC10
// LEVER: the copy-paste block, spelled as the nested if/else the branches
// describe - `if (get_state_id(bit)) { if (!(word & bit)) on(); } else
// { if (word & bit) off(); }` - one block per treaty bit, nothing hoisted
// into a loop or a helper. VC6 then hoists the two member loads above the
// first branch itself, keeps the bit constant in a callee-saved register
// (ebx for the treaty_on/off blocks, edi from HAVE_INFILTRATOR on, which is
// why the last sync_agenda_bit call pushes edi), and narrows the masks below 0x100 to
// `test byte ptr`. First spelling measured BYTE_EXACT; no divergence to try
// against.
Return Value: n/a
Status: Complete
*/
void DipEdit::read_check() {
    if (check_box_.get_state_id(DTREATY_COMMLINK)) {
        if (!(PlayersData[faction_index_].diplo_treaties[selected_target_] & DTREATY_COMMLINK)) {
            treaty_on(faction_index_, selected_target_, DTREATY_COMMLINK);
        }
    } else {
        if (PlayersData[faction_index_].diplo_treaties[selected_target_] & DTREATY_COMMLINK) {
            treaty_off(faction_index_, selected_target_, DTREATY_COMMLINK);
        }
    }

    if (check_box_.get_state_id(DTREATY_VENDETTA)) {
        if (!(PlayersData[faction_index_].diplo_treaties[selected_target_] & DTREATY_VENDETTA)) {
            treaty_on(faction_index_, selected_target_, DTREATY_VENDETTA);
        }
    } else {
        if (PlayersData[faction_index_].diplo_treaties[selected_target_] & DTREATY_VENDETTA) {
            treaty_off(faction_index_, selected_target_, DTREATY_VENDETTA);
        }
    }

    if (check_box_.get_state_id(DTREATY_TRUCE)) {
        if (!(PlayersData[faction_index_].diplo_treaties[selected_target_] & DTREATY_TRUCE)) {
            treaty_on(faction_index_, selected_target_, DTREATY_TRUCE);
        }
    } else {
        if (PlayersData[faction_index_].diplo_treaties[selected_target_] & DTREATY_TRUCE) {
            treaty_off(faction_index_, selected_target_, DTREATY_TRUCE);
        }
    }

    if (check_box_.get_state_id(DTREATY_TREATY)) {
        if (!(PlayersData[faction_index_].diplo_treaties[selected_target_] & DTREATY_TREATY)) {
            treaty_on(faction_index_, selected_target_, DTREATY_TREATY);
        }
    } else {
        if (PlayersData[faction_index_].diplo_treaties[selected_target_] & DTREATY_TREATY) {
            treaty_off(faction_index_, selected_target_, DTREATY_TREATY);
        }
    }

    if (check_box_.get_state_id(DTREATY_PACT)) {
        if (!(PlayersData[faction_index_].diplo_treaties[selected_target_] & DTREATY_PACT)) {
            treaty_on(faction_index_, selected_target_, DTREATY_PACT);
        }
    } else {
        if (PlayersData[faction_index_].diplo_treaties[selected_target_] & DTREATY_PACT) {
            treaty_off(faction_index_, selected_target_, DTREATY_PACT);
        }
    }

    if (check_box_.get_state_id(DTREATY_HAVE_INFILTRATOR)) {
        if (!(PlayersData[faction_index_].diplo_treaties[selected_target_] & DTREATY_HAVE_INFILTRATOR)) {
            set_treaty(faction_index_, selected_target_,
                       DTREATY_HAVE_INFILTRATOR, TRUE);
        }
    } else {
        if (PlayersData[faction_index_].diplo_treaties[selected_target_] & DTREATY_HAVE_INFILTRATOR) {
            set_treaty(faction_index_, selected_target_,
                       DTREATY_HAVE_INFILTRATOR, FALSE);
        }
    }

    if (check_box_.get_state_id(DTREATY_WANT_TO_TALK)) {
        if (!(PlayersData[faction_index_].diplo_treaties[selected_target_] & DTREATY_WANT_TO_TALK)) {
            set_treaty(faction_index_, selected_target_,
                       DTREATY_WANT_TO_TALK, TRUE);
        }
    } else {
        if (PlayersData[faction_index_].diplo_treaties[selected_target_] & DTREATY_WANT_TO_TALK) {
            set_treaty(faction_index_, selected_target_,
                       DTREATY_WANT_TO_TALK, FALSE);
        }
    }

    // The four editable bits whose apply step is a helper of its own, in the
    // image's order: set_treaty_bit on WANT_REVENGE / HAVE_SURRENDERED /
    // SHALL_BETRAY / ATROCITY_VICTIM, then set_agenda_bit on
    // COMMLINK, then sync_agenda_bit on WANT_TO_TALK.
    set_treaty_bit(DTREATY_WANT_REVENGE);
    set_treaty_bit(DTREATY_HAVE_SURRENDERED);
    set_treaty_bit(DTREATY_SHALL_BETRAY);
    set_treaty_bit(DTREATY_ATROCITY_VICTIM);
    set_agenda_bit(DTREATY_COMMLINK);
    sync_agenda_bit(DTREATY_WANT_TO_TALK);

    if (PlayersData[faction_index_].diplo_treaties[selected_target_] & DTREATY_HAVE_SURRENDERED) {
        treaty_on(faction_index_, selected_target_, DTREATY_PACT);
        // 0xc20 == DTREATY_WANT_REVENGE | DTREATY_SHALL_BETRAY | 0x800, the
        // three-bit treaty mask surrender clears; no name covers the 0x800.
        set_treaty(faction_index_, selected_target_, 0xc20, FALSE);
        set_agenda(faction_index_, selected_target_, DAGENDA_FIGHT_TO_DEATH,
                   FALSE);
    }

    // The +8 view of the same table, the other direction's word - the one
    // do_check reads for the 0x80000000-id boxes.
    if (PlayersData[faction_index_].diplo_agenda[selected_target_] & DTREATY_COMMLINK) {
        treaty_on(faction_index_, selected_target_, DTREATY_VENDETTA);
    }

    if (!(PlayersData[faction_index_].diplo_treaties[selected_target_] & DTREATY_WANT_REVENGE)) {
        // 0xc00 == DTREATY_SHALL_BETRAY | 0x800.
        set_agenda(faction_index_, selected_target_, 0xc00, FALSE);
    }
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
// symbol    ?sync_treaty_bit@DipEdit@@QAEXH@Z
//   renamed from the catalogue's UNK1 placeholder 2026-09-02; the image
//   carries no symbol of its own for this body (see the class block in
//   dipedit.h), so the claim resolves through this emitted name instead.
// size      128 bytes
// prototype void (__thiscall ?sync_treaty_bit@DipEdit@@QAEXH@Z)(DipEdit* this, int)
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
void DipEdit::sync_treaty_bit(int bit) {
    if (check_box_.get_state_id(bit)) {
        if (!(PlayersData[faction_index_].diplo_treaties[selected_target_] & bit)) {
            treaty_on(faction_index_, selected_target_, bit);
        }
    } else {
        if (PlayersData[faction_index_].diplo_treaties[selected_target_] & bit) {
            treaty_off(faction_index_, selected_target_, bit);
        }
    }
}



/*
Purpose: sync_treaty_bit for the AGENDA word. The checkbox id carries 0x80000000, which
         do_check also does when it writes the second table view back, and
         that view is diplo_agenda - 8 dwords past diplo_treaties in
         PlayerData, which is what moves the table base 0x96C9F8 -> 0x96CA18.
         The writers are agenda_on/agenda_off rather than
         treaty_on/treaty_off.
// ORIGINAL: 0x004DA7E0 ?UNK2@DipEdit@@QAEXH@Z 0x004DA7E0-0x004DA867 BYTE_EXACT
// symbol    ?sync_agenda_bit@DipEdit@@QAEXH@Z
//   renamed from the catalogue's UNK2 placeholder 2026-09-02; see the
//   class block in dipedit.h
// size      135 bytes
// prototype void (__thiscall ?sync_agenda_bit@DipEdit@@QAEXH@Z)(DipEdit* this, int)
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
void DipEdit::sync_agenda_bit(int bit) {
    int id = static_cast<int>(0x80000000U) | bit;

    if (check_box_.get_state_id(id)) {
        if (!(PlayersData[faction_index_].diplo_agenda[selected_target_] & bit)) {
            agenda_on(faction_index_, selected_target_, bit);
        }
    } else {
        if (PlayersData[faction_index_].diplo_agenda[selected_target_] & bit) {
            agenda_off(faction_index_, selected_target_, bit);
        }
    }
}



/*
Purpose: sync_treaty_bit narrowed to ONE direction. Same checkbox id and same treaty word
         as sync_treaty_bit, but the writer is set_treaty(faction_index_, selected_target_
         , bit, TRUE/FALSE) - which writes only PlayersData[faction_index_]
         .diplo_treaties[selected_target_] and leaves the mirror entry alone -
         and it is called only when it would change something, where sync_treaty_bit's
         writers decide that for themselves.
// ORIGINAL: 0x004DA870 ?UNK3@DipEdit@@QAEXH@Z 0x004DA870-0x004DA8F4 BYTE_EXACT
// symbol    ?set_treaty_bit@DipEdit@@QAEXH@Z
//   renamed from the catalogue's UNK3 placeholder 2026-09-02; see the
//   class block in dipedit.h
// size      132 bytes
// prototype void (__thiscall ?set_treaty_bit@DipEdit@@QAEXH@Z)(DipEdit* this, int)
// callers   1   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0055BB30 0x0060EC10
// LEVER: `TRUE`/`FALSE` as set_treaty's fourth argument are what push the 1/0
//        the image pushes; no local for the boolean is wanted.
Return Value: n/a
Status: Complete
*/
void DipEdit::set_treaty_bit(int bit) {
    if (check_box_.get_state_id(bit)) {
        if (!(PlayersData[faction_index_].diplo_treaties[selected_target_] & bit)) {
            set_treaty(faction_index_, selected_target_, bit, TRUE);
        }
    } else {
        if (PlayersData[faction_index_].diplo_treaties[selected_target_] & bit) {
            set_treaty(faction_index_, selected_target_, bit, FALSE);
        }
    }
}



/*
Purpose: set_treaty_bit for the AGENDA word: the 0x80000000 checkbox id, the +8 dword
         view, and set_agenda as the writer instead of set_treaty.
// ORIGINAL: 0x004DA900 ?UNK4@DipEdit@@QAEXH@Z 0x004DA900-0x004DA98B BYTE_EXACT
// symbol    ?set_agenda_bit@DipEdit@@QAEXH@Z
//   renamed from the catalogue's UNK4 placeholder 2026-09-02; see the
//   class block in dipedit.h
// size      139 bytes
// prototype void (__thiscall ?set_agenda_bit@DipEdit@@QAEXH@Z)(DipEdit* this, int)
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
void DipEdit::set_agenda_bit(int bit) {
    int id = static_cast<int>(0x80000000U) | bit;

    if (check_box_.get_state_id(id)) {
        if (!(PlayersData[faction_index_].diplo_agenda[selected_target_] & bit)) {
            set_agenda(faction_index_, selected_target_, bit, TRUE);
        }
    } else {
        if (PlayersData[faction_index_].diplo_agenda[selected_target_] & bit) {
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
        PlayersData[faction_index_].diplo_treaties[selected_target_] & DTREATY_COMMLINK);
    cb->set_state_id(DTREATY_VENDETTA,
        PlayersData[faction_index_].diplo_treaties[selected_target_] & DTREATY_VENDETTA);
    cb->set_state_id(DTREATY_TRUCE,
        PlayersData[faction_index_].diplo_treaties[selected_target_] & DTREATY_TRUCE);
    cb->set_state_id(DTREATY_TREATY,
        PlayersData[faction_index_].diplo_treaties[selected_target_] & DTREATY_TREATY);
    cb->set_state_id(DTREATY_PACT,
        PlayersData[faction_index_].diplo_treaties[selected_target_] & DTREATY_PACT);
    cb->set_state_id(DTREATY_HAVE_INFILTRATOR,
        PlayersData[faction_index_].diplo_treaties[selected_target_] & DTREATY_HAVE_INFILTRATOR);
    cb->set_state_id(DTREATY_WANT_TO_TALK,
        PlayersData[faction_index_].diplo_treaties[selected_target_] & DTREATY_WANT_TO_TALK);
    cb->set_state_id(DTREATY_WANT_REVENGE,
        PlayersData[faction_index_].diplo_treaties[selected_target_] & DTREATY_WANT_REVENGE);
    cb->set_state_id(DTREATY_HAVE_SURRENDERED,
        PlayersData[faction_index_].diplo_treaties[selected_target_] & DTREATY_HAVE_SURRENDERED);
    cb->set_state_id(DTREATY_SHALL_BETRAY,
        PlayersData[faction_index_].diplo_treaties[selected_target_] & DTREATY_SHALL_BETRAY);
    cb->set_state_id(DTREATY_ATROCITY_VICTIM,
        PlayersData[faction_index_].diplo_treaties[selected_target_] & DTREATY_ATROCITY_VICTIM);

    // The second table view, eight uint32_t elements (0x20 bytes) further
    // into the same SpyingStatusTable storage.
    cb->set_state_id(static_cast<int>(0x80000000U) | DTREATY_COMMLINK,
        PlayersData[faction_index_].diplo_agenda[selected_target_] & DTREATY_COMMLINK);
    cb->set_state_id(static_cast<int>(0x80000000U) | DTREATY_WANT_TO_TALK,
        PlayersData[faction_index_].diplo_agenda[selected_target_] & DTREATY_WANT_TO_TALK);

    // The redraw goes through the embedded CheckBox's own virtual dispatch
    // (win-table slot 62). Re-derived here rather than through the cached
    // `cb`: the image folds the +0x15A0 into the call's own receiver
    // computation (`lea ecx, [edx + esi + 0x15a0]`).
    check_box_.redraw();
}

/*
Purpose: The handler the diplomatic-edit window's FlatButtons share. The int is
         the control id the caller passes, and the four values the body
         distinguishes are -1 (the window's own close row: release the modal
         and nothing else), 1 (the treaty/agenda row: clear every treaty bit
         the pair holds, re-score the vendetta intensity in BOTH directions of
         the status table, wipe the six per-pair detail words both ways, and
         repaint the checkboxes), 2 (the integrity prompt: parse_says the
         faction noun, ask for a number, store ParseNumTable[0]) and 3 (the
         atrocity prompt: a two-field Popup editing both atrocity counts).
         The local Popup is 0x537C bytes - the whole prompt window, Scroll,
         buttons, dialogs and all - so this frame is the one that pays
         __alloca_probe.

         `PopupModalState` is the modal-popup flag: parked at 6 while the
         popup runs, cleared as soon as exec returns. The -1 row returns
         WITHOUT clearing it, which the image does too (0x004DA45B is on the
         common exit path only).
// ORIGINAL: 0x004D9DE0 ?on_button_clicked@DipEdit@@QAEXH@Z 0x004D9DE0-0x004DA73F;0x0065B347-0x0065B469
// symbol    ?on_button_clicked@DipEdit@@UAEXH@Z
//           Q->U 2026-09-06, Win's front slots 44/45 taking the image's
//           own names (on_button_clicked / on_button_toggled): the body is
//           now an override of the window's slot, so VC6 mangles it virtual
//           and enters it on the front table, while the image spells the
//           same body Q and its own table holds the raw address.
// size      2399 bytes
// prototype void (__thiscall ?on_button_clicked@DipEdit@@QAEXH@Z)(DipEdit* this, int)
// callers   0   call targets   30
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00402970 0x004048A0 0x00404900 0x00406380 0x004064D0 0x00406880 0x00406910 0x004DADA0 0x0055B760 0x0055BB30 0x0055BBA0 0x005BFD10 0x005D45B0 0x005D4DD0 0x005E3820 0x005FA870 0x00600F00 0x00602600 0x00605370 0x00607040 0x00607DA0 0x00608E10 0x00611F80 0x006169A0 0x00625810 0x00625EC0 0x00645470 0x00645550 0x00645DA7 0x0064FC88
// indirect  0x004D9E30
// LEVER: the frame is the source's doing, and it lands exactly. A single
//   `Popup popup;` local (0x537C == sizeof(Popup)) plus one `char number_text
//   [0x50]` compiles to the image's own prologue under /O2 /Oy- /GX: the EBP
//   frame, `mov eax, 0x53d0 / call __alloca_probe`, `push ebx / push esi` after
//   it, `Popup::Popup` on ebp-0x53DC, the `cmp eax, -1` read of the argument
//   and the `mov ds:0x6a721c, 6` - all 23 leading instructions agree, and the
//   frame size rounds to the same 0x53D0. Under /O2 without /Oy- the frame
//   pointer goes away and the whole body shifts, so the EBP form is the
//   image's.
// LEVER: get_noun folds at the parse_says sites. `parse_says(0,
//   get_noun(faction_index_), -1, -1)` is the image's own 0x59C-stride multiply
//   (`lea eax,[ecx+ecx*4]; lea eax,[eax+eax*8]; shl eax,3; sub eax,ecx; shl
//   eax,2`), the two GenderDefault/PluralityDefault stores and the noun pointer
//   pushed - MEASURED inline in faction.h is what makes that fold.
// TRIED: caching `faction_index_`/`selected_target_` in locals through the
//   treaty block. The image re-reads both members off `this` at every one of
//   the four treaty/agenda calls and at every table index it builds (the calls
//   can edit the record), so the member names stay in the source.
// CEILING, NOT A SPELLING: the last ~160 of the image's 576 instructions are
//   the destructor of the local Popup, expanded INLINE at two sites, with
//   different depth at each - the control-id -1 row expands `scroll_.~Scroll()` and
//   calls ??1BasePop out of line (0x004D9E36..0x004D9EC2), the common exit
//   expands both ~Scroll and ~BasePop all the way down to Heap::shutdown and
//   the GraphicWin base (0x004DA461..0x004DA727). This tree's ~Popup, ~Scroll
//   and ~BasePop are out-of-line definitions (pending_bodies forwarders, the
//   image's own ??1 copies live at 0x00406C00/0x00406E60/0x004064D0), so VC6
//   emits one shared cleanup holding a single `call ??1Popup` where the image
//   holds 160 instructions, and the compiled body is 32 calls to the image's
//   55 - the whole gap is that expansion, and `call_diff` confirms no business
//   call is missing. Closing it needs ~Popup/~Scroll/~BasePop defined inline in
//   their headers so the compiler can expand them per site; that moves
//   ??1TutWin, ??1PickWin and every other Popup/Scroll/BasePop destroyer in the
//   tree, and VC6's per-site choice of how deep to expand is not reachable from
//   source. Not chased further this pass. The 4-byte difference in the local
//   area (image puts the popup at ebp-0x53DC, this tree at ebp-0x53D8) is the
//   same cause: the image's spill slot at ebp-0x10 exists only for the inlined
//   teardown.
Return Value: n/a
Status: Complete
*/
void DipEdit::on_button_clicked(int button_id) {
    Popup popup;
    char number_text[0x50];

    PopupModalState = 6;

    if (button_id == -1) {
        release_modal();
        return;
    }

    if (button_id == 1) {
        treaty_off(faction_index_, selected_target_, 0x1f);
        set_treaty(faction_index_, selected_target_, 0x02043c20, FALSE);
        set_treaty(faction_index_, selected_target_,
                   static_cast<int>(0xdd9bc3c0), FALSE);
        set_agenda(faction_index_, selected_target_, 0x1f89, FALSE);

        // The pair's shared status word decides what the vendetta-intensity
        // word becomes; the value lands in BOTH directions of the table.
        uint32_t status = PlayersData[faction_index_].diplo_treaties[selected_target_];
        if (status & DTREATY_VENDETTA) {
            PlayersData[faction_index_].diplo_friction[selected_target_] = 0x14;
            PlayersData[selected_target_].diplo_friction[faction_index_] = 0x14;
        } else if (status & DTREATY_PACT) {
            PlayersData[faction_index_].diplo_friction[selected_target_] = 2;
            PlayersData[selected_target_].diplo_friction[faction_index_] = 2;
        } else if (status & DTREATY_WANT_REVENGE) {
            PlayersData[faction_index_].diplo_friction[selected_target_] = random(0, 10) + 5;
            PlayersData[selected_target_].diplo_friction[faction_index_] = random(0, 10) + 5;
        } else {
            // Nothing settled between the pair: the intensity is a fresh
            // random score in both directions, and each direction rolls its
            // own.
            PlayersData[faction_index_].diplo_friction[selected_target_] = random(0, 15);
            PlayersData[selected_target_].diplo_friction[faction_index_] = random(0, 15);
        }

        // The six per-pair detail words, wiped both ways.
        PlayersData[faction_index_].loan_balance[selected_target_] = 0;
        PlayersData[faction_index_].diplo_unk1[selected_target_] = 0;
        PlayersData[faction_index_].diplo_wrongs[selected_target_] = 0;
        PlayersData[faction_index_].diplo_betrayed[selected_target_] = 0;
        PlayersData[faction_index_].diplo_unk3[selected_target_] = 0;
        PlayersData[faction_index_].diplo_unk4[selected_target_] = 0;
        PlayersData[selected_target_].loan_balance[faction_index_] = 0;
        PlayersData[selected_target_].diplo_unk1[faction_index_] = 0;
        PlayersData[selected_target_].diplo_wrongs[faction_index_] = 0;
        PlayersData[selected_target_].diplo_betrayed[faction_index_] = 0;
        PlayersData[selected_target_].diplo_unk3[faction_index_] = 0;
        PlayersData[selected_target_].diplo_unk4[faction_index_] = 0;

        do_check();
    } else if (button_id == 2) {
        parse_says(0, get_noun(faction_index_), -1, -1);
        if (X_pop_ask_number("EDITINTEGRITY",
                PlayersData[faction_index_].integrity_blemishes,
                0) == 0) {
            PlayersData[faction_index_].integrity_blemishes
                = ParseNumTable[0];
        }
    } else if (button_id == 3) {
        parse_says(0, get_noun(faction_index_), -1, -1);
        popup.start(PopupStartCaption, "EDITATROCITY", -1, nullptr, 0x44,
                    nullptr);

        StringTemp[0] = '\0';
        _itoa(PlayersData[faction_index_].atrocities,
              number_text, 10);
        strcat(StringTemp, number_text);
        // The caption id is read out of the label blob the way the image reads it:
        // `mov eax, ds:0x9b90f8; mov ecx, [eax + 0xdb4]`.
        popup.dialogs().edit(StringTable.get(
                                     (int)*((LPSTR *)Labels.strings_ptr + 877)),
            StringTemp, 20);

        StringTemp[0] = '\0';
        _itoa(PlayersData[faction_index_].major_atrocities,
              number_text, 10);
        strcat(StringTemp, number_text);
        popup.dialogs().edit(StringTable.get(
                                     (int)*((LPSTR *)Labels.strings_ptr + 878)),
            StringTemp, 20);

        if (popup.exec(0, 0) == 0) {
            PlayersData[faction_index_].atrocities
                = atoi(ParseStrBuffer[0].str);
            PlayersData[faction_index_].major_atrocities
                = atoi(ParseStrBuffer[1].str);
        }
    }

    PopupModalState = 0;
}

/*
Purpose: Build the diplomacy-status window. Sizes it, fills it, paints the
         sunken frame and the centred title into the GraphicWin buffer, then
         populates the three controls: one ListBox row per rival faction, one
         CheckBox item per treaty flag, and the row of FlatButtons along the
         bottom edge - and finally runs the first read_check.
// ORIGINAL: 0x004DB080 ?init@DipEdit@@QAEXH@Z 0x004DB080-0x004DB868
// size      2024 bytes
// prototype void (__thiscall ?init@DipEdit@@QAEXH@Z)(DipEdit* this, int)
// callers   1   call targets   24
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004DA990 0x004DADA0 0x005882F0 0x005ABF20 0x005D4EF0 0x005D5250 0x005DAC70 0x005DACB0 0x005DD130 0x005E2DD3 0x005ED5C0 0x00607210 0x00609470 0x00609C60 0x00609F20 0x00609FA0 0x0060C920 0x0060E7C0 0x0060E980 0x006169A0 0x00618F40 0x006453E0 0x00645470
// indirect  0x004DB1A0 0x004DB3F5 0x004DB40D 0x004DB65A 0x004DB672 0x004DB6EF 0x004DB6FB 0x004DB756 0x004DB762 0x004DB7BD 0x004DB7C9 0x004DB824 0x004DB830 0x004DB840 0x004DB852
Return Value: n/a

The window is built entirely out of named member calls: the Font member, the
GraphicWin base's own init, the Buffer base at +0x444 (the three box_sprite
frames, set_font, set_text_color and write_cent_l), the ListBox and CheckBox
members through their VIRTUAL Dialog base (`list_box_.Dialog::init` walks the
ListBox vbtable to find the base at +0xA60 and calls 0x00609470 through it),
and the four FlatButtons that get BaseButton::init. flat_button2_ is the one
control init does NOT touch.

The title text colours at 0x008C6E2C/0x008C6E30 are TextColourTable[180]/[181],
initialized by textcolor_init; shared InterfaceBoxStyles[9]/[0] supply the frame and title-field
records originally at 0x78D690/0x78D528.

// LEVER: every control reached as a named member - `list_box_.Dialog::init`
//   and `check_box_.Dialog::init` are QUALIFIED on purpose, because the
//   receiver is the ListBox's/CheckBox's VIRTUAL Dialog base: the image walks
//   the vbtable (`mov eax,[edi]; mov ecx,[eax+8]; add ecx,edi`) and then makes
//   a direct E8 into 0x00609470. The qualified spelling is what produces that
//   walk; an unqualified `list_box_.init(...)` would be ambiguous against
//   GraphicWin::init anyway.
// LEVER: `width_`/`height_` are SIGNED (dipedit.h) - the /3 at 0x004DB259 and
//   the /2 at 0x004DB426 both compile to the signed division sequences.
// TRIED: `Buffer *buffer = this;` for the six Buffer calls at +0x444 - the
//   conversion emits a null test on `this`, a spill slot and a re-load
//   (instructions 33-35 of the image have none of them, and the frame grows
//   0x14 -> 0x18), so the calls are spelled on the inherited members instead;
//   that alone moved the body 41 -> 86 agreeing instructions.
// TRIED: `const RECT &frame = rect2_;` to hold &rect2_ live in a register the
//   way the image keeps it in ebx from 0x004DB0EF to 0x004DB675 - 100 -> 96
//   agreeing, reverted.
// TRIED: `check_box_.item(text, flag, flag)` - the third argument is 0 at
//   twelve of the thirteen sites and DTREATY_WANT_REVENGE at the eighth
//   (0x004DB563 pushes 0x20 twice); only the SECOND argument is the treaty
//   flag everywhere.
// TRIED: the ordering of `list_box_.Dialog::field_20_ &= ~4` (0x004DB38D) -
//   both spellings materialise a pointer, the image folds the base into the
//   `lea` and 0x20 into the store, this tree folds 0xa6c into the `lea` and
//   stores through it. Six bytes against three, same semantics, not chased.
// CEILING: 100 of 614 instructions agree at /O2 /Gy /GR- /Oy- /GX. The
//   remaining divergence is register allocation, not structure: every call
//   target, argument list and field store matches, but this tree picks
//   different registers for the ~20 inlined `label_get` expansions (the image
//   reserves ecx for the StringTable receiver before evaluating the argument,
//   this tree computes the argument first and loads the receiver last) and for
//   the `if (StringTemp)` test (ecx in the image, eax here). The compiled body
//   is 594 instructions against the image's 614.
Status: In progress - full transcription, MISMATCH on register allocation.
*/
void DipEdit::init(int faction_index) {
    font_.init(DefaultFontFace, 0xE, 0);

    width_ = 0x2F6;
    height_ = 0x18E;
    faction_index_ = faction_index;

    GraphicWin::init(0, 0, 0x2F6, 0x18E, 0, 0x1000000, 0, 0, 0);
    center();
    fill(9);

    RECT rect = rect2_;
    box_sprite(&rect, &InterfaceBoxStyles[9]);

    // The window title: "<label 866> <faction noun>", upper-cased in place.
    StringTemp[0] = 0;
    strcat(StringTemp, label_get(866));
    strcat(StringTemp, " ");
    strcat(StringTemp, get_noun(faction_index_));
    CharUpperA(StringTemp);

    rect.left += 4;
    rect.right -= 4;
    rect.top += 4;
    rect.bottom = rect.top + 0x14;
    box_sprite(&rect, &InterfaceBoxStyles[0]);

    set_font(find_font(0x10, 0), 0, 0, 0);
    set_text_color(TextColourTable[180], TextColourTable[181], 1, 1);

    if (StringTemp) {
        write_cent_l(StringTemp, &rect, strlen(StringTemp));
    }

    rect.left = 4;
    rect.right = width_ / 3 - 4;
    rect.bottom = 0x172;
    rect.top = 0x1F;
    box_sprite(&rect, &InterfaceBoxStyles[0]);

    // The item area is the box just drawn, inset by the 3-pixel bevel.
    rect.left += 3;
    rect.right -= 3;
    rect.top += 3;
    rect.bottom -= 3;

    // The list of rival factions: every index but our own, the selected one
    // re-armed last so the row ends up highlighted.
    list_box_.close();
    list_box_.Dialog::init(&rect, 0);
    list_box_.field_6C_ = 9;
    list_box_.Dialog::field_20_ &= ~4;
    list_box_.field_2C_ = 0;
    list_box_.field_44_ = 0;
    list_box_.field_B4_ = 0;
    list_box_.set_dialog_font(&font_, 0, 0);
    list_box_.attach(this, rect.left, rect.bottom, 0x1000120);
    list_box_.show(0);

    selected_target_ = faction_index_;
    for (int i = 1; i < MaxPlayerNum; i++) {
        if (i == static_cast<int>(faction_index_)) {
            continue;
        }
        StringTemp[0] = 0;
        strcat(StringTemp, get_noun(i));
        list_box_.item(StringTemp, i);
        if (selected_target_ == static_cast<int>(faction_index_)) {
            selected_target_ = i;
            list_box_.set_selected_id(i);
        }
    }

    // The treaty checkboxes: one item per DTREATY flag, the last two in the
    // second (+8) bank of the status table, exactly the set do_check writes
    // back out.
    rect.bottom = 0x172;
    rect.top = 0x1F;
    rect.left = width_ / 2 + 4;
    rect.right = width_ - 4;
    check_box_.close();
    check_box_.Dialog::init(&rect, 0);
    check_box_.unchecked_sprite_ = &g_IFACE_CLOSE_X_SPRITES[1];
    check_box_.checked_sprite_ = &g_IFACE_CLOSE_X_SPRITES[2];
    check_box_.item(label_get(875), DTREATY_COMMLINK, 0);
    check_box_.item(label_get(646), DTREATY_VENDETTA, 0);
    check_box_.item(label_get(643), DTREATY_TRUCE, 0);
    check_box_.item(label_get(644), DTREATY_TREATY, 0);
    check_box_.item(label_get(645), DTREATY_PACT, 0);
    check_box_.item(label_get(868), DTREATY_HAVE_INFILTRATOR, 0);
    check_box_.item(label_get(869), DTREATY_WANT_TO_TALK, 0);
    check_box_.item(label_get(870), DTREATY_WANT_REVENGE,
                    DTREATY_WANT_REVENGE);
    check_box_.item(label_get(871), DTREATY_HAVE_SURRENDERED, 0);
    check_box_.item(label_get(872), DTREATY_SHALL_BETRAY, 0);
    check_box_.item(label_get(873), DTREATY_ATROCITY_VICTIM, 0);
    check_box_.item(label_get(874), static_cast<int>(0x80000000U)
                    | DTREATY_COMMLINK, 0);
    check_box_.item(label_get(894), static_cast<int>(0x80000000U)
                    | DTREATY_WANT_TO_TALK, 0);
    check_box_.attach(this, rect.left, rect.bottom, DTREATY_WANT_REVENGE);
    check_box_.show(0);

    // The button row: start from the frame rectangle, inset it to the strip,
    // and size each button as a quarter of the width. Each button then steps
    // right by (width + 2). flat_button2_ is the one control init never
    // touches - the image drives it from somewhere else.
    rect = rect2_;
    rect.left += 4;
    rect.right -= 4;
    rect.bottom -= 4;
    rect.right = (rect.right - rect.left + 2) / 4 + rect.left - 2;
    rect.top = rect2_.bottom - 24;

    int step = rect.right - rect.left + 2;
    flat_button5_.init(label_get(876), 3, rect.left, rect.top,
                       rect.right - rect.left, rect.bottom - rect.top, this, 0);
    flat_button5_.redraw();
    flat_button5_.show(0);
    rect.left += step;
    rect.right += step;
    flat_button4_.init(label_get(222), 2, rect.left, rect.top,
                       rect.right - rect.left, rect.bottom - rect.top, this, 0);
    flat_button4_.redraw();
    flat_button4_.show(0);
    rect.left += step;
    rect.right += step;
    flat_button3_.init(label_get(863), 1, rect.left, rect.top,
                       rect.right - rect.left, rect.bottom - rect.top, this, 0);
    flat_button3_.redraw();
    flat_button3_.show(0);
    rect.left += step;
    rect.right += step;
    flat_button1_.init(label_get(101), -1, rect.left, rect.top,
                       rect.right - rect.left, rect.bottom - rect.top, this, 0);
    flat_button1_.redraw();
    flat_button1_.show(0);

    do_check();
    show(0);
    auto_undo();
    set_modal(0, 0, 0);
    read_check();
}
