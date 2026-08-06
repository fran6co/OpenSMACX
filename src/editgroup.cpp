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
#include "editgroup.h"

func_edit_box_set_text EditBoxOriginalSetText =
    original_method<func_edit_box_set_text>(0x006151E0);

/*
Purpose: Apply one length limit to all ten edit boxes in the group.
Original Offset: 006120B0
Return Value: n/a
Status: Complete
*/
void EditGroup::set_text_limits(int limit) {
    for (size_t index = 0; index < ARRAYSIZE(limits_); ++index) {
        limits_[index] = static_cast<uint32_t>(limit);
    }
}

/*
Purpose: Return one box's text buffer, or nothing when that box is absent.
Original Offset: 00612060
Return Value: pointer to the box's text, null when there is no box
Status: Complete
*/
char *EditGroup::get_text(int index) {
    void *const box = boxes_[index];
    if (!box) {
        return nullptr;
    }
    return reinterpret_cast<char *>(box) + 0xA14;
}

/*
Purpose: Set one box's text, doing nothing when that box is absent.
Original Offset: 00612040
Return Value: n/a
Status: Complete
*/
void EditGroup::set_text(char *text, int index) {
    void *const box = boxes_[index];
    if (box) {
        (ORIGINAL(box)->*EditBoxOriginalSetText)(text);
    }
}

void __fastcall edit_group_set_text_limits_redirect(EditGroup *self, void *, int limit) {
    self->set_text_limits(limit);
}

char *__fastcall edit_group_get_text_redirect(EditGroup *self, void *, int index) {
    return self->get_text(index);
}

void __fastcall edit_group_set_text_redirect(EditGroup *self, void *, char *text, int index) {
    self->set_text(text, index);
}
