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
#include <cstring>
#include "dialogs.h"

func_dialog_item *DialogOriginalItem = (func_dialog_item *)0x00609990;
func_list_box_item *ListBoxOriginalItem = (func_list_box_item *)0x0060C920;

namespace {

// The jump table indexes on the kind minus one, and anything outside one to
// sixteen falls through to the default arm.
bool kind_in_range(int32_t kind) {
    return static_cast<uint32_t>(kind - 1) <= 0xF;
}

// Where the Dialog subobject is, according to this object's own vbtable.
Dialog *dialog_of(void *self) {
    auto *const bytes = reinterpret_cast<uint8_t *>(self);
    const int32_t *const vbtable = *reinterpret_cast<const int32_t *const *>(bytes);
    return reinterpret_cast<Dialog *>(bytes + vbtable[2]);
}

}  // namespace

/*
Purpose: Fetch one item's text, from whichever widget this kind of dialog
         keeps its items in.
Original Offset: 00612A70
Return Value: the widget's result, zero for kinds that hold no items
Status: Complete
*/
int Dialogs::item(char *text, int index) {
    if (!kind_in_range(kind_)) {
        return 0;
    }
    switch (kind_) {
        case 1:
        case 4:
        case 8:
        case 16:
            return DialogOriginalItem(dialog_of(this), text, index);
        case 2:
            return ListBoxOriginalItem(this, text, index);
        default:
            return 0;
    }
}

/*
Purpose: Count the items this kind of dialog holds.
Original Offset: 00613740
Return Value: the count, zero for kinds that hold no items
Status: Complete
*/
int Dialogs::get_num_items() {
    if (!kind_in_range(kind_)) {
        return 0;
    }
    switch (kind_) {
        case 1:
        case 2:
        case 4:
        case 16: {
            int32_t count = 0;
            std::memcpy(&count,
                        reinterpret_cast<uint8_t *>(dialog_of(this)) + 0xCC,
                        sizeof(count));
            return count;
        }
        case 8:
            return field_A4_;
        default:
            return 0;
    }
}

int __fastcall dialogs_item_redirect(Dialogs *self, void *, char *text, int index) {
    return self->item(text, index);
}

int __fastcall dialogs_get_num_items_redirect(Dialogs *self, void *) {
    return self->get_num_items();
}
