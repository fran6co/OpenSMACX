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
#include "prodpicker.h"
#include <cstring>

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns 1.
Original Offset: 00493C10
Return Value: 1, always
Status: Complete
*/
int ProdPicker::UNK1(int) {
    return 1;
}

int __fastcall prod_picker_unk1_redirect(ProdPicker *self, void *, int a1) {
    return self->UNK1(a1);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
Original Offset: 00493E90
Return Value: n/a
Status: Complete
*/
void ProdPicker::on_mouse_move(int, int) {
}

void __fastcall prod_picker_on_mouse_move_redirect(ProdPicker *self, void *, int a1, int a2) {
    self->on_mouse_move(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
Original Offset: 00495160
Return Value: n/a
Status: Complete
*/
void ProdPicker::UNK3(int) {
}

void __fastcall prod_picker_unk3_redirect(ProdPicker *self, void *, int a1) {
    self->UNK3(a1);
}

/*
Purpose: Close the picker - clear the field at 0xA14 that marks it live, then
         run the GraphicWin close it inherits.
Original Offset: 004926B0
Return Value: n/a
Status: Complete
*/
void ProdPicker::close() {
    field_A14_ = 0;
    GraphicWin::close();
}

void __fastcall prod_picker_close_redirect(ProdPicker *self, void *) {
    self->close();
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
Original Offset: 004945C0
Return Value: n/a
Status: Complete
*/
void ProdPicker::on_redraw_nc(int, int) {
}

void __fastcall prod_picker_on_redraw_nc_redirect(
        ProdPicker *self, void *, int a1, int a2) {
    self->on_redraw_nc(a1, a2);
}
