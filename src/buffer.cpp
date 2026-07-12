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
#include "buffer.h"

/*
Purpose: Set the four fonts used by the buffer.
Original Offset: 005DAC70
Return Value: No errors (0); invalid primary font (3)
Status: Complete
*/
int Buffer::set_font(Font *font1, Font *font2, Font *font3, Font *font4) {
    if (!font1) {
        return 3;
    }
    if (font1->is_initialized()) {
        font1_ = font1;
    }
    font2_ = font2;
    font3_ = font3;
    font4_ = font4;
    return 0;
}

/*
Purpose: Set the primary text colors for the four color slots.
Original Offset: 005DACB0
Return Value: n/a
Status: Complete
*/
void Buffer::set_text_color(int color1, int color2, int color3, int color4) {
    color_val_1_ = color1;
    color_val_2_ = color2;
    color_val_3_ = color3;
    color_val_4_ = color4;
}

/*
Purpose: Set the secondary text colors for the four color slots.
Original Offset: 005DACE0
Return Value: n/a
Status: Complete
*/
void Buffer::set_text_color2(int color1, int color2, int color3, int color4) {
    color_2_val_1_ = color1;
    color_2_val_2_ = color2;
    color_2_val_3_ = color3;
    color_2_val_4_ = color4;
}

/*
Purpose: Set the tertiary text colors for the four color slots.
Original Offset: 005DAD10
Return Value: n/a
Status: Complete
*/
void Buffer::set_text_color3(int color1, int color2, int color3, int color4) {
    color_3_val_1_ = color1;
    color_3_val_2_ = color2;
    color_3_val_3_ = color3;
    color_3_val_4_ = color4;
}

/*
Purpose: Set the hyperlink text colors for the four color slots.
Original Offset: 005DAD40
Return Value: n/a
Status: Complete
*/
void Buffer::set_text_color_hyper(int color1, int color2, int color3, int color4) {
    color_hyper_val_1_ = color1;
    color_hyper_val_2_ = color2;
    color_hyper_val_3_ = color3;
    color_hyper_val_4_ = color4;
}

int __fastcall buffer_set_font_redirect(
    Buffer *self, void *, Font *font1, Font *font2, Font *font3, Font *font4) {
    return self->set_font(font1, font2, font3, font4);
}

void __fastcall buffer_set_text_color_redirect(
    Buffer *self, void *, int color1, int color2, int color3, int color4) {
    self->set_text_color(color1, color2, color3, color4);
}

void __fastcall buffer_set_text_color2_redirect(
    Buffer *self, void *, int color1, int color2, int color3, int color4) {
    self->set_text_color2(color1, color2, color3, color4);
}

void __fastcall buffer_set_text_color3_redirect(
    Buffer *self, void *, int color1, int color2, int color3, int color4) {
    self->set_text_color3(color1, color2, color3, color4);
}

void __fastcall buffer_set_text_color_hyper_redirect(
    Buffer *self, void *, int color1, int color2, int color3, int color4) {
    self->set_text_color_hyper(color1, color2, color3, color4);
}
