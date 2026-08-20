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

void __fastcall button_group_add_redirect(ButtonGroup *self, void *, BaseButton *button) {
    self->add(button);
}

ButtonGroup *__fastcall button_group_construct_redirect(ButtonGroup *self, void *) {
    return new (self) ButtonGroup;
}

uint32_t __fastcall button_group_close_redirect(ButtonGroup *self, void *) {
    self->close();
    return 0;
}

int __fastcall button_group_init_redirect(
        ButtonGroup *self, void *, int group_id, int flags) {
    self->init(group_id, flags);
    return group_id;
}

func_button_group_click ButtonGroupOriginalButtonClick =
    original_method<func_button_group_click>(0x0062B8A0);

/*
Purpose: Activate a button in the group, suppressing the group's own
         notification while the click is dispatched when notify is zero.
// ORIGINAL: 0x0062B870 ?set@ButtonGroup@@QAEHHH@Z 0x0062B870-0x0062B89F
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
    const int result = (ORIGINAL(this)->*ButtonGroupOriginalButtonClick)(button_id);
    field_90_ = 0;
    return result;
}

int __fastcall button_group_set_redirect(ButtonGroup *self, void *,
                                         int button_id, int notify) {
    return self->set(button_id, notify);
}
