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
Original Offset: 0062B7C0
Return Value: this
Status: Complete
*/
ButtonGroup::ButtonGroup() {
    count_ = 0;
    field_8C_ = 0;
    field_90_ = 0;
    memset(buttons_, 0, sizeof(buttons_));
}

ButtonGroup::~ButtonGroup() {
    close();
}

/*
Purpose: Clear the active group while retaining the original selected ID and flags slots.
Original Offset: 0062B7F0
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
Original Offset: 0062B810
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
Original Offset: 0062B850
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
Original Offset: 0062B870
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
