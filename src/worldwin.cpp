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

const uint32_t WorldWinPrimaryVtable = 0x0066DF30;
const uint32_t WorldWinBufferVtable = 0x0066DF28;

/*
Purpose: Construct the GraphicWin base and the three embedded subobjects
         (buffer1_, buffer2_, mapWin_), then install WorldWin's own vtables.
// ORIGINAL: 0x004C4BF0 ??0WorldWin@@QAE@XZ 0x004C4BF0-0x004C4C67;0x00659E50-0x00659E7E
// symbol    ?construct@WorldWin@@QAEPAV1@XZ
// size      119 bytes
// prototype void (__thiscall ??0WorldWin@@QAE@XZ)(WorldWin* this)
// callers   0   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok;frame
// calls     0x005D4CF0 0x005D7210 0x004626E0
// TRIED: best reached is 13/31, 0.882 similar (best of 10 flag sets).
//        The residual is placement-new's null guard - see
//        multidebug.cpp, where converting to a real constructor closed it
//        outright. NOT APPLIED HERE: `mapWin_` is built with
//        `MapWin::construct(1)`, and as an ordinary member of a real
//        constructor it would be built by `MapWin`'s own constructor instead,
//        which is blocked on the hidden most-derived flag
//        (tools/most_derived_flag.py names it). The lever needs that first.
//        Same residual as NetMsg::construct() and MultiDebug::construct()
//        (netmsg.cpp, multidebug.cpp): image has `push ecx` at instruction
//        7 where this body has `sub esp, 8` - a stack-frame-size
//        difference from spill-slot count, not control flow or field
//        order. Not chased further.
Return Value: n/a
Status: Complete
*/
WorldWin *WorldWin::construct() {
    new (static_cast<GraphicWin *>(this)) GraphicWin();
    new (&buffer1_) Buffer();
    new (&buffer2_) Buffer();
    mapWin_.construct(1);
    uint32_t *const object = reinterpret_cast<uint32_t *>(this);
    object[0x000 / 4] = WorldWinPrimaryVtable;
    object[0x444 / 4] = WorldWinBufferVtable;
    return this;
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x004C45E0 ?clear_terrain@WorldWin@@QAEXXZ 0x004C45E0-0x004C45E1 BYTE_EXACT
// size      1 bytes
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
// ORIGINAL: 0x004C3E90 ?on_left_double_click@WorldWin@@QAEXHH@Z 0x004C3E90-0x004C3E93 BYTE_EXACT
// size      3 bytes
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


/*
Purpose: Report a left click to the world window's shared click handler.
// ORIGINAL: 0x004C3E50 ?on_left_click@WorldWin@@QAEXHH@Z 0x004C3E50-0x004C3E68 BYTE_EXACT
// size      24 bytes
// prototype void (__thiscall ?on_left_click@WorldWin@@QAEXHH@Z)(WorldWin* this, int, int)
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004C3D40
Return Value: n/a
Status: Complete
*/
void WorldWin::on_left_click(int a1, int a2) {
    click(a1, a2, 0, 0);
}

/*
Purpose: Report a right click to the world window's shared click handler.
// ORIGINAL: 0x004C3E70 ?on_right_click@WorldWin@@QAEXHH@Z 0x004C3E70-0x004C3E88 BYTE_EXACT
// size      24 bytes
// prototype void (__thiscall ?on_right_click@WorldWin@@QAEXHH@Z)(WorldWin* this, int, int)
// callers   0   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004C3D40
Return Value: n/a
Status: Complete
*/
void WorldWin::on_right_click(int a1, int a2) {
    click(a1, a2, 1, 0);
}

void __fastcall world_win_on_left_click_redirect(WorldWin *self, void *, int a1, int a2) {
    self->on_left_click(a1, a2);
}

void __fastcall world_win_on_right_click_redirect(WorldWin *self, void *, int a1, int a2) {
    self->on_right_click(a1, a2);
}

/*
// ORIGINAL: 0x004C45F0 ?set_center@WorldWin@@QAEXHH@Z 0x004C45F0-0x004C4603 BYTE_EXACT
// size      19 bytes
// prototype void (__thiscall ?set_center@WorldWin@@QAEXHH@Z)(WorldWin* this, int, int)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D5890
// indirect  0x004C45F5
Status: Complete
*/
void WorldWin::set_center(int a1, int a2) {
    reinterpret_cast<VCall *>(this)->slot012();
    reinterpret_cast<GraphicWin *>(this)->soft_update();
}

/*
// ORIGINAL: 0x004C4610 ?focus@WorldWin@@QAEXHH@Z 0x004C4610-0x004C4623 BYTE_EXACT
// size      19 bytes
// prototype void (__thiscall ?focus@WorldWin@@QAEXHH@Z)(WorldWin* this, int, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D5890
// indirect  0x004C4615
Status: Complete
*/
void WorldWin::focus(int a1, int a2) {
    reinterpret_cast<VCall *>(this)->slot012();
    reinterpret_cast<GraphicWin *>(this)->soft_update();
}

/*
// ORIGINAL: 0x004C3EA0 ?follow_center@WorldWin@@QAEXHH@Z 0x004C3EA0-0x004C3EB3 BYTE_EXACT
// size      19 bytes
// prototype void (__thiscall ?follow_center@WorldWin@@QAEXHH@Z)(WorldWin* this, int xCoord, int yCoord)
// callers   1   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005D5890
// indirect  0x004C3EA5
Status: Complete
*/
void WorldWin::follow_center(int xCoord, int yCoord) {
    char *self = reinterpret_cast<char *>(this);
    reinterpret_cast<VCall *>(self)->slot012();
    reinterpret_cast<GraphicWin *>(self)->soft_update();
}
