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
#include "mapwin.h"
#include <cstring>

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
Original Offset: 00467960
Return Value: n/a
Status: Complete
*/
void MapWin::UNK3() {
}

/*
Purpose: Refresh the map's image buttons; the legacy implementation is a bare
         return, so the buttons are never rebuilt here.
Original Offset: 0046FB80
Return Value: n/a
Status: Complete
*/
void MapWin::do_image_buttons() {
}

void __fastcall map_win_unk3_redirect(MapWin *self, void *) {
    self->UNK3();
}

void __fastcall map_win_do_image_buttons_redirect(MapWin *self, void *) {
    self->do_image_buttons();
}

func_set_date *MainInterfaceOriginalSetDate = (func_set_date *)0x0045BE80;
void *MainInterfaceGlobal = (void *)0x007AE820;
char *MapWinMainCaption = (char *)0x009B86A0;

/*
Purpose: Push the map's caption into the main interface's date field. The
         original acts on the interface and caption it keeps at fixed
         addresses rather than on anything reached through the map window.
Original Offset: 0046FB10
Return Value: n/a
Status: Complete
*/
void MapWin::main_caption() {
    MainInterfaceOriginalSetDate(MainInterfaceGlobal, MapWinMainCaption);
}

void __fastcall map_win_main_caption_redirect(MapWin *self, void *) {
    self->main_caption();
}

func_map_win_free *MapWinFree = (func_map_win_free *)0x00644EF2;

/*
Purpose: Close the map window - free the buffer it owns at 0x4, then close its
         graphic base. The base is located through the object's own vbtable,
         not a fixed offset, so an embedded MapWin closes its own base.
Original Offset: 00470F70
Return Value: n/a
Status: Complete
*/
void MapWin::close() {
    if (owned_) {
        MapWinFree(owned_);
        owned_ = nullptr;
    }
    const int32_t *const vbtable =
        *reinterpret_cast<const int32_t *const *>(this);
    reinterpret_cast<GraphicWin *>(
        reinterpret_cast<uint8_t *>(this) + vbtable[1])->close();
}

void __fastcall map_win_close_redirect(MapWin *self, void *) {
    self->close();
}

func_map_win_click *MapWinClick = (func_map_win_click *)0x0046D5D0;
int32_t *MapWinInputEnabled = reinterpret_cast<int32_t *>(0x0090D938);

/*
Purpose: Report a left click on the map, but only when map input is enabled.
         Reached through the GraphicWin virtual base, so `this` points there
         and is adjusted back 0x21A6C to the MapWin before dispatching.
Original Offset: 0046EBA0
Return Value: n/a
Status: Complete
*/
void MapWin::on_left_click(int a1, int a2) {
    if (*MapWinInputEnabled == 0) {
        return;
    }
    auto *const base = reinterpret_cast<MapWin *>(
        reinterpret_cast<uint8_t *>(this) - 0x21A6C);
    MapWinClick(base, a1, a2, 0);
}

/*
Purpose: Report a right click on the map, but only when map input is enabled.
         Same virtual-base adjustment as the left click.
Original Offset: 0046EBE0
Return Value: n/a
Status: Complete
*/
void MapWin::on_right_click(int a1, int a2) {
    if (*MapWinInputEnabled == 0) {
        return;
    }
    auto *const base = reinterpret_cast<MapWin *>(
        reinterpret_cast<uint8_t *>(this) - 0x21A6C);
    MapWinClick(base, a1, a2, 1);
}

void __fastcall map_win_on_left_click_redirect(MapWin *self, void *, int a1, int a2) {
    self->on_left_click(a1, a2);
}

void __fastcall map_win_on_right_click_redirect(MapWin *self, void *, int a1, int a2) {
    self->on_right_click(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
Original Offset: 0046EBD0
Return Value: n/a
Status: Complete
*/
void MapWin::on_left_double_click(int, int) {
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
Original Offset: 0046EFF0
Return Value: n/a
Status: Complete
*/
void MapWin::on_left_up(int, int) {
}

void __fastcall map_win_on_left_double_click_redirect(MapWin *self, void *, int a1, int a2) {
    self->on_left_double_click(a1, a2);
}

void __fastcall map_win_on_left_up_redirect(MapWin *self, void *, int a1, int a2) {
    self->on_left_up(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns 0.
Original Offset: 00462970
Return Value: 0, always
Status: Complete
*/
int MapWin::UNK2() {
    return 0;
}

int __fastcall map_win_unk2_redirect(MapWin *self, void *) {
    return self->UNK2();
}
