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
ORIGINAL: 0x00493C10 BYTE_EXACT
// name      ?UNK1@ProdPicker@@QAEHH@Z
// size      8 bytes
// spans     0x00493C10-0x00493C18
// prototype int (__thiscall ?UNK1@ProdPicker@@QAEHH@Z)(ProdPicker* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
ORIGINAL: 0x00493E90 BYTE_EXACT
// name      ?on_mouse_move@ProdPicker@@QAEXHH@Z
// size      3 bytes
// spans     0x00493E90-0x00493E93
// prototype void (__thiscall ?on_mouse_move@ProdPicker@@QAEXHH@Z)(ProdPicker* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
ORIGINAL: 0x00495160 BYTE_EXACT
// name      ?UNK3@ProdPicker@@QAEXH@Z
// size      3 bytes
// spans     0x00495160-0x00495163
// prototype void (__thiscall ?UNK3@ProdPicker@@QAEXH@Z)(ProdPicker* this, int)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
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
ORIGINAL: 0x004926B0
// name      ?close@ProdPicker@@QAEXXZ
// size      15 bytes
// spans     0x004926B0-0x004926BF
// prototype void (__thiscall ?close@ProdPicker@@QAEXXZ)(ProdPicker* this)
// callers   2   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
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
ORIGINAL: 0x004945C0 BYTE_EXACT
// name      ?on_redraw_nc@ProdPicker@@QAEXPAURECT@@H@Z
// size      3 bytes
// spans     0x004945C0-0x004945C3
// prototype void (__thiscall ?on_redraw_nc@ProdPicker@@QAEXPAURECT@@H@Z)(ProdPicker* this, RECT*, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void ProdPicker::on_redraw_nc(RECT *, int) {
}

void __fastcall prod_picker_on_redraw_nc_redirect(
        ProdPicker *self, void *, RECT *a1, int a2) {
    self->on_redraw_nc(a1, a2);
}
