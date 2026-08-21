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
#include "editbox.h"

func_edit_box_set_text EditBoxOriginalSetText =
    original_method<func_edit_box_set_text>(0x006151E0);

/*
Purpose: Apply one length limit to all ten edit boxes in the group.
// ORIGINAL: 0x006120B0 ?set_text_limits@EditGroup@@QAEXH@Z 0x006120B0-0x006120C3 BYTE_EXACT
// size      19 bytes
// prototype void (__thiscall ?set_text_limits@EditGroup@@QAEXH@Z)(EditGroup* this, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
// ORIGINAL: 0x00612060 ?get_text@EditGroup@@QAEPADH@Z 0x00612060-0x00612079
// size      25 bytes
// prototype int8* (__thiscall ?get_text@EditGroup@@QAEPADH@Z)(EditGroup* this, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
// ORIGINAL: 0x00612040 ?set_text@EditGroup@@QAEXPADH@Z 0x00612040-0x00612059 BYTE_EXACT
// size      25 bytes
// prototype void (__thiscall ?set_text@EditGroup@@QAEXPADH@Z)(EditGroup* this, int8*, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006151E0
Return Value: n/a
Status: Complete
*/
void EditGroup::set_text(char *text, int index) {
    void *const box = boxes_[index];
    if (box) {
        // A NAMED METHOD, not the seam: the image emits `call rel32` here.
        reinterpret_cast<EditBox *>(box)->EditBox::set_text(text);
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
