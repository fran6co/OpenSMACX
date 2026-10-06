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
#include "buttongroup.h"
#include "worldwin.h"

#include <new>

/*
Purpose: Initialize a button group while preserving the original selected ID and flags slots.
// ORIGINAL: 0x0062B7C0 ??0ButtonGroup@@QAE@XZ 0x0062B7C0-0x0062B7E4 BYTE_EXACT
// size      36 bytes
// prototype void (__thiscall ??0ButtonGroup@@QAE@XZ)(ButtonGroup* this)
// callers   8   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: this
Status: Complete
*/
ButtonGroup::ButtonGroup() {
    count_ = 0;
    field_8C_ = 0;
    field_90_ = 0;
    memset(buttons_, 0, sizeof(buttons_));
}

/*
// ORIGINAL: 0x004325B0 ??1ButtonGroup@@QAE@XZ 0x004325B0-0x004325B5 BYTE_EXACT
// size      5 bytes
// prototype void (__thiscall ??1ButtonGroup@@QAE@XZ)(ButtonGroup* this)
// callers   0   call targets   0
// kind      thunk
// flags     hidden;thunk;sp_ready;purged_ok
// calls     (none)
*/
ButtonGroup::~ButtonGroup() {
    close();
}

/*
Purpose: Clear the active group while retaining the original selected ID and flags slots.
// ORIGINAL: 0x0062B7F0 ?close@ButtonGroup@@QAEXXZ 0x0062B7F0-0x0062B810 BYTE_EXACT
// size      32 bytes
// prototype void (__thiscall ?close@ButtonGroup@@QAEXXZ)(ButtonGroup* this)
// callers   9   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: n/a
Status: Complete
*/
OPENSMACX_NOINLINE
void ButtonGroup::close() {
    count_ = 0;
    field_8C_ = 0;
    field_90_ = 0;
    memset(buttons_, 0, sizeof(buttons_));
}

/*
Purpose: Initialize all button-group state.
// ORIGINAL: 0x0062B810 ?init@ButtonGroup@@QAEXHH@Z 0x0062B810-0x0062B84C BYTE_EXACT
// size      60 bytes
// prototype void (__thiscall ?init@ButtonGroup@@QAEXHH@Z)(ButtonGroup* this, int, int)
// callers   9   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: n/a
Status: Complete
*/
void ButtonGroup::init(int group_id, int flags) {
    count_ = 0;
    field_90_ = 0;
    memset(buttons_, 0, sizeof(buttons_));
    field_8C_ = static_cast<uint32_t>(group_id);
    field_84_ = 0xFFFFFFFEU;
    field_88_ = static_cast<uint32_t>(flags);
}

/*
Purpose: Add a button to the group and link the button back to it.
// ORIGINAL: 0x0062B850 ?add@ButtonGroup@@QAEXPAUBaseButton@@@Z 0x0062B850-0x0062B86C BYTE_EXACT
// symbol    ?add@ButtonGroup@@QAEXPAVBaseButton@@@Z
// size      28 bytes
// prototype void (__thiscall ?add@ButtonGroup@@QAEXPAUBaseButton@@@Z)(ButtonGroup* this, BaseButton* button)
// callers   9   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: n/a
Status: Complete
*/
void ButtonGroup::add(BaseButton *button) {
    buttons_[count_] = button;
    button->group_ = this;
    count_++;
}






/*
Purpose: Activate a button in the group, suppressing the group's own
         notification while the click is dispatched when notify is zero.
// ORIGINAL: 0x0062B870 ?set@ButtonGroup@@QAEHHH@Z 0x0062B870-0x0062B89F BYTE_EXACT
// size      47 bytes
// prototype int (__thiscall ?set@ButtonGroup@@QAEHHH@Z)(ButtonGroup* this, int, int)
// callers   35   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0062B8A0
Return Value: whatever button_click returns
Status: Complete
*/
int ButtonGroup::set(int button_id, int notify) {
    if (!notify) {
        field_90_ = 1;
    }
    const int result = button_click(button_id);
    field_90_ = 0;
    return result;
}

// The original PrefWin ButtonGroup object is solely owned as
// PrefWinButtonGroup in src/prefwin.cpp; do not define a duplicate here.

/*
Purpose: A button group click: find the active and the clicked buttons, drop
         the clicked one when the group refuses reselection, repaint and
         notify through the group's two slots on the owner window, then
         repaint every button still flagged dirty.

// ORIGINAL: 0x0062B8A0 ?button_click@ButtonGroup@@QAEHH@Z 0x0062B8A0-0x0062BA77
// RECOVERY_BINDING: v1 {"evidence": ["pinned image 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf; original identity 0x0062B8A0 retains name and all original spans/claims", "src/buttongroup.h declares the actual ButtonGroup::button_click class member; body uses that existing owner API", "original source was FILE-mode in src/leaf_recoveries.cpp; this transfer places the complete member in its actual owner TU and changes measurement mode to BODY explicitly; it does not assert historical recipe equality", "pinned 0x0062B9CE-0x0062B9D9 loads the first button's Win parent; the typed parent_window() accessor replaces the raw +0xC4 access without adding a null guard or changing callback order", "existing TRIED/LEVER/DEFERRED history is carried forward unchanged; its old no-accessor rationale is superseded by the current typed parent access"], "reason": "Transfer ButtonGroup::button_click to its real owner and express the evidenced parent access through the declared hierarchy. Preserve behavior, original spans and historical claims; BODY is the new measurement scope, not proof of old whole-TU recipe equality."}
// size      417 bytes
// prototype int (__thiscall ?button_click@ButtonGroup@@QAEHH@Z)(ButtonGroup* this, int)
// callers   2   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok
// indirect  0x0062B9BE 0x0062BA1D 0x0062BA51 0x0062BA69
// TRIED (seed): the owner notifications are Win slots 47 (two ids) and 46
//         (ids plus the group's third word); the owner is read off the first
//         button's Win parent - a member Win keeps protected, and ButtonGroup
//         is the buttons' friend rather than Win's, so that one read keeps
//         the image's offset (Win+0xC4).
Status: Source-owned member body; behavior retained, exactness not measured
*/
int ButtonGroup::button_click(int clicked_id) {
    int found_active = 0;
    int found_clicked = 0;
    int active_index = 0;
    int clicked_index = 0;

    int total = static_cast<int>(count_);
    if (total > 0) {
        const uint32_t active_id = field_84_;
        for (int index = 0; index < total; ++index) {
            const uint32_t button_id = buttons_[index]->field_A78_;
            if (button_id == active_id) {
                active_index = index;
                found_active = 1;
            }
            if (button_id == static_cast<uint32_t>(clicked_id)) {
                clicked_index = index;
                found_clicked = 1;
            }
        }
    }

    const unsigned char flags = static_cast<unsigned char>(field_88_);
    if ((flags & 2) != 0) {
        if (static_cast<int>(field_84_) == clicked_id) {
            field_84_ = 0xFFFFFFFFu;
            found_clicked = 0;
        }
    }

    if (static_cast<int>(field_84_) != clicked_id || (flags & 1) == 0) {
        if (found_active != 0) {
            buttons_[active_index]->set(0);
            buttons_[active_index]->redraw();
        }
        if (found_clicked != 0) {
            buttons_[clicked_index]->set(1);
            buttons_[clicked_index]->redraw();
            field_84_ = static_cast<uint32_t>(clicked_id);
        } else {
            field_84_ = 0xFFFFFFFFu;
        }

        if (field_90_ == 0 && count_ != 0) {
            // Original 0x62B9CE follows buttons_[0] to its inherited Win
            // parent member (+0xC4). The public accessor owns that read;
            // preserve the original direct dereference without a null guard.
            Win *owner = buttons_[0]->parent_window();
            const int active_id = found_active
                ? static_cast<int>(buttons_[active_index]->field_A78_) : -1;
            const int clicked = found_clicked
                ? static_cast<int>(buttons_[clicked_index]->field_A78_) : -1;
            owner->vslot_47(active_id, clicked);

            if (count_ != 0) {
                const int active_again = found_active
                    ? static_cast<int>(buttons_[active_index]->field_A78_) : -1;
                const int clicked_again = found_clicked
                    ? static_cast<int>(buttons_[clicked_index]->field_A78_) : -1;
                owner->vslot_46(active_again, clicked_again,
                                static_cast<int>(field_8C_));
            }
        }
    }

    total = static_cast<int>(count_);
    for (int index = 0; index < total; ++index) {
        if (buttons_[index]->field_A74_ != 0) {
            buttons_[index]->field_A74_ = 0;
            buttons_[index]->redraw();
        }
        total = static_cast<int>(count_);
    }
    return 0;
}
