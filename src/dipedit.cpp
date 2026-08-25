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
// The int is an INDEX: the body stores it in field_A20_, which then indexes
// SpyingStatusTable[field_A1C_ * SpyingStatusStride + field_A20_].
void DipEdit::on_selected(int index) {
    read_check();
    field_A20_ = index;
    do_check();
}



/*
Purpose: Sync eleven checkbox bits on the embedded CheckBox to the current
         diplomatic status word (SpyingStatusTable[subject * 2099 +
         faction]) - one set_state_id call per treaty flag - then redraw the
         CheckBox subobject through its own vtable-relative slot062. Each of
         the twelve set_state_id calls RE-READS field_A1C_/field_A20_ from
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
    char *const self = reinterpret_cast<char *>(this);
    CheckBox *const cb = reinterpret_cast<CheckBox *>(self + 0x15A0);

    cb->set_state_id(DTREATY_COMMLINK,
        SpyingStatusTable[field_A1C_ * SpyingStatusStride
            + field_A20_] & DTREATY_COMMLINK);
    cb->set_state_id(DTREATY_VENDETTA,
        SpyingStatusTable[field_A1C_ * SpyingStatusStride
            + field_A20_] & DTREATY_VENDETTA);
    cb->set_state_id(DTREATY_TRUCE,
        SpyingStatusTable[field_A1C_ * SpyingStatusStride
            + field_A20_] & DTREATY_TRUCE);
    cb->set_state_id(DTREATY_TREATY,
        SpyingStatusTable[field_A1C_ * SpyingStatusStride
            + field_A20_] & DTREATY_TREATY);
    cb->set_state_id(DTREATY_PACT,
        SpyingStatusTable[field_A1C_ * SpyingStatusStride
            + field_A20_] & DTREATY_PACT);
    cb->set_state_id(DTREATY_HAVE_INFILTRATOR,
        SpyingStatusTable[field_A1C_ * SpyingStatusStride
            + field_A20_] & DTREATY_HAVE_INFILTRATOR);
    cb->set_state_id(DTREATY_WANT_TO_TALK,
        SpyingStatusTable[field_A1C_ * SpyingStatusStride
            + field_A20_] & DTREATY_WANT_TO_TALK);
    cb->set_state_id(DTREATY_WANT_REVENGE,
        SpyingStatusTable[field_A1C_ * SpyingStatusStride
            + field_A20_] & DTREATY_WANT_REVENGE);
    cb->set_state_id(DTREATY_HAVE_SURRENDERED,
        SpyingStatusTable[field_A1C_ * SpyingStatusStride
            + field_A20_] & DTREATY_HAVE_SURRENDERED);
    cb->set_state_id(DTREATY_SHALL_BETRAY,
        SpyingStatusTable[field_A1C_ * SpyingStatusStride
            + field_A20_] & DTREATY_SHALL_BETRAY);
    cb->set_state_id(DTREATY_ATROCITY_VICTIM,
        SpyingStatusTable[field_A1C_ * SpyingStatusStride
            + field_A20_] & DTREATY_ATROCITY_VICTIM);

    // The second table view, eight uint32_t elements (0x20 bytes) further
    // into the same SpyingStatusTable storage.
    cb->set_state_id(static_cast<int>(0x80000000U) | DTREATY_COMMLINK,
        SpyingStatusTable[field_A1C_ * SpyingStatusStride
            + field_A20_ + 8] & DTREATY_COMMLINK);
    cb->set_state_id(static_cast<int>(0x80000000U) | DTREATY_WANT_TO_TALK,
        SpyingStatusTable[field_A1C_ * SpyingStatusStride
            + field_A20_ + 8] & DTREATY_WANT_TO_TALK);

    reinterpret_cast<VCall *>(
        self + 0x15A0
        + *reinterpret_cast<int *>(reinterpret_cast<char *>(
            field_15A0_) + 4))
        ->slot062();
}

void __fastcall dip_edit_do_check_redirect(DipEdit *self, void *) {
    self->do_check();
}
