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
#include "worldwin.h"
#include "vtable_shim.h"

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
ORIGINAL: 0x004C45E0 BYTE_EXACT
// name      ?clear_terrain@WorldWin@@QAEXXZ
// size      1 bytes
// spans     0x004C45E0-0x004C45E1
// prototype void (__thiscall ?clear_terrain@WorldWin@@QAEXXZ)(WorldWin* this)
// callers   2   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void WorldWin::clear_terrain() {
}

void __fastcall world_win_clear_terrain_redirect(WorldWin *self, void *) {
    self->clear_terrain();
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
ORIGINAL: 0x004C3E90 BYTE_EXACT
// name      ?on_left_double_click@WorldWin@@QAEXHH@Z
// size      3 bytes
// spans     0x004C3E90-0x004C3E93
// prototype void (__thiscall ?on_left_double_click@WorldWin@@QAEXHH@Z)(WorldWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void WorldWin::on_left_double_click(int, int) {
}

void __fastcall world_win_on_left_double_click_redirect(WorldWin *self, void *, int a1, int a2) {
    self->on_left_double_click(a1, a2);
}

func_world_win_click WorldWinClick = original_method<func_world_win_click>(0x004C3D40);

/*
Purpose: Report a left click to the world window's shared click handler.
ORIGINAL: 0x004C3E50
// name      ?on_left_click@WorldWin@@QAEXHH@Z
// size      24 bytes
// spans     0x004C3E50-0x004C3E68
// prototype void (__thiscall ?on_left_click@WorldWin@@QAEXHH@Z)(WorldWin* this, int, int)
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004C3D40
Return Value: n/a
Status: Complete
*/
void WorldWin::on_left_click(int a1, int a2) {
    (ORIGINAL(this)->*WorldWinClick)(a1, a2, 0, 0);
}

/*
Purpose: Report a right click to the world window's shared click handler.
ORIGINAL: 0x004C3E70
// name      ?on_right_click@WorldWin@@QAEXHH@Z
// size      24 bytes
// spans     0x004C3E70-0x004C3E88
// prototype void (__thiscall ?on_right_click@WorldWin@@QAEXHH@Z)(WorldWin* this, int, int)
// callers   0   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004C3D40
Return Value: n/a
Status: Complete
*/
void WorldWin::on_right_click(int a1, int a2) {
    (ORIGINAL(this)->*WorldWinClick)(a1, a2, 1, 0);
}

void __fastcall world_win_on_left_click_redirect(WorldWin *self, void *, int a1, int a2) {
    self->on_left_click(a1, a2);
}

void __fastcall world_win_on_right_click_redirect(WorldWin *self, void *, int a1, int a2) {
    self->on_right_click(a1, a2);
}

/*
ORIGINAL: 0x004C45F0 BYTE_EXACT
// name      ?set_center@WorldWin@@QAEXHH@Z
// size      19 bytes
// spans     0x004C45F0-0x004C4603
// prototype void (__thiscall ?set_center@WorldWin@@QAEXHH@Z)(WorldWin* this, int, int)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D5890
Status: Complete
*/
void WorldWin::set_center(int a1, int a2) {
    reinterpret_cast<VCall *>(this)->slot012();
    reinterpret_cast<GraphicWin *>(this)->soft_update();
}

/*
ORIGINAL: 0x004C4610 BYTE_EXACT
// name      ?focus@WorldWin@@QAEXHH@Z
// size      19 bytes
// spans     0x004C4610-0x004C4623
// prototype void (__thiscall ?focus@WorldWin@@QAEXHH@Z)(WorldWin* this, int, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D5890
Status: Complete
*/
void WorldWin::focus(int a1, int a2) {
    reinterpret_cast<VCall *>(this)->slot012();
    reinterpret_cast<GraphicWin *>(this)->soft_update();
}

/*
ORIGINAL: 0x004C3EA0 BYTE_EXACT
// name      ?follow_center@WorldWin@@QAEXHH@Z
// size      19 bytes
// spans     0x004C3EA0-0x004C3EB3
// prototype void (__thiscall ?follow_center@WorldWin@@QAEXHH@Z)(WorldWin* this, int xCoord, int yCoord)
// callers   1   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005D5890
Status: Complete
*/
void WorldWin::follow_center(int xCoord, int yCoord) {
    char *self = reinterpret_cast<char *>(this);
    reinterpret_cast<VCall *>(self)->slot012();
    reinterpret_cast<GraphicWin *>(self)->soft_update();
}
