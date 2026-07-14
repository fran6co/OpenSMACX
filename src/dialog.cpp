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
#include "dialog.h"

/*
Purpose: Set the three fonts used by the dialog.
Original Offset: 00609C60
Return Value: No errors (0); invalid primary font (3)
Status: Complete
*/
int Dialog::set_dialog_font(Font *font1, Font *font2, Font *font3) {
    if (!font1) {
        return 3;
    }
    if (font1->is_initialized()) {
        font1_ = font1;
    }
    font2_ = font2;
    font3_ = font3;
    return 0;
}

/*
Purpose: Set the primary text colors for the four dialog color slots.
Original Offset: 00609C90
Return Value: n/a
Status: Complete
*/
void Dialog::set_dialog_text_color(int color1, int color2, int color3, int color4) {
    text_color_a_ = color1;
    text_color_b_ = color2;
    text_color_c_ = color3;
    text_color_d_ = color4;
}

/*
Purpose: Set the secondary text colors for the four dialog color slots.
Original Offset: 00609CC0
Return Value: n/a
Status: Complete
*/
void Dialog::set_dialog_text_color2(int color1, int color2, int color3, int color4) {
    text_color_2a_ = color1;
    text_color_2b_ = color2;
    text_color_2c_ = color3;
    text_color_2d_ = color4;
}

/*
Purpose: Set the tertiary text colors for the four dialog color slots.
Original Offset: 00609CF0
Return Value: n/a
Status: Complete
*/
void Dialog::set_dialog_text_color3(int color1, int color2, int color3, int color4) {
    text_color_3a_ = color1;
    text_color_3b_ = color2;
    text_color_3c_ = color3;
    text_color_3d_ = color4;
}

/*
Purpose: Find an item ID's bounded position in the dialog string list.
Original Offset: 00609AF0
Return Value: Matching position, or the configured entry count on a miss
Status: Complete
*/
int Dialog::id_to_pos(int id) {
    if (entry_head_) {
        int traversed = 0;
        int count = entry_count_;
        entry_position_ = 0;
        current_entry_ = entry_head_;
        if (count > 0) {
            do {
                DialogEntry *entry = current_entry_;
                if (entry->id == id) {
                    break;
                }
                entry_position_++;
                traversed++;
                current_entry_ = entry->next;
            } while (traversed < count);
        }
    }
    return entry_position_;
}

/*
Purpose: Select a dialog item by its bounded list ID.
Original Offset: 006099D0
Return Value: n/a
Status: Complete
*/
void Dialog::set_selected_id(int id) {
    selected_position_ = id_to_pos(id);
}

/*
Purpose: Restore the selected list position and return its item ID.
Original Offset: 00609A50
Return Value: Selected item ID, or zero when the list head is null
Status: Complete
*/
int Dialog::get_selected_id() {
    int position = selected_position_;
    int count = entry_count_;
    if (count == INT_MIN || position < count) {
        current_entry_ = entry_head_;
        if (position < 0) {
            int distance = position == INT_MIN ? INT_MIN : -position;
            if (distance <= count) {
                while (distance > 0) {
                    current_entry_ = current_entry_->previous;
                    distance--;
                }
                uint32_t normalized =
                    static_cast<uint32_t>(position) + static_cast<uint32_t>(count);
                memcpy(&position, &normalized, sizeof(position));
                entry_position_ = position;
            }
        } else {
            int distance = position;
            while (distance > 0) {
                current_entry_ = current_entry_->next;
                distance--;
            }
            entry_position_ = position;
        }
    }
    return entry_head_ ? current_entry_->id : 0;
}

int __fastcall dialog_set_font_redirect(
    Dialog *self, void *, Font *font1, Font *font2, Font *font3) {
    return self->set_dialog_font(font1, font2, font3);
}

void __fastcall dialog_set_text_color_redirect(
    Dialog *self, void *, int color1, int color2, int color3, int color4) {
    self->set_dialog_text_color(color1, color2, color3, color4);
}

void __fastcall dialog_set_text_color2_redirect(
    Dialog *self, void *, int color1, int color2, int color3, int color4) {
    self->set_dialog_text_color2(color1, color2, color3, color4);
}

void __fastcall dialog_set_text_color3_redirect(
    Dialog *self, void *, int color1, int color2, int color3, int color4) {
    self->set_dialog_text_color3(color1, color2, color3, color4);
}


int __fastcall dialog_id_to_pos_redirect(Dialog *self, void *, int id) {
    return self->id_to_pos(id);
}

void __fastcall dialog_set_selected_id_redirect(Dialog *self, void *, int id) {
    self->set_selected_id(id);
}

int __fastcall dialog_get_selected_id_redirect(Dialog *self, void *) {
    return self->get_selected_id();
}
