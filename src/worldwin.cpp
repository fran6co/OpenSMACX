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
#include "worldwin.h"
#include "maininterface.h"

// The primary deleting destructor and the Buffer-base adjustor are emitted by
// WorldWin's genuine inherited virtual destructor. Constructor 4C4C49 installs
// table 66DF30 at this; 4C4C4F installs table 66DF28 at this+444. Their first
// slots select 4C4C70 and 4C4CA0, respectively. The adjustor is not related to
// the separately embedded MapWin's virtual GraphicWin base.
// ORIGINAL: 0x004C4C70 ??_GWorldWin@@UAEPAXI@Z 0x004C4C70-0x004C4C91 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason":"The real inherited virtual destructor emits WorldWin's primary scalar deleting operation; replace the unused hand-written wrapper while preserving its original identity and historical obligation.","evidence":["Original 4C4C70-4C4C91 calls complete destructor4C4A70, conditionally calls operator delete64557F on flags bit0, returns the original receiver and pops4 bytes. Constructor4C4C49 and original primary table66DF30 independently select this operation.","The canonical mapping and protected history were previously carried by scalar_delete_world_win in deleting_thunks.cpp; its synthetic fastcall receiver is not the original thiscall ABI. Genuine GraphicWin inheritance supplies the actual generated entry without a source wrapper."]}
// symbol    ??_GWorldWin@@UAEPAXI@Z

// ORIGINAL: 0x004C4CA0 ??_GWorldWin@@WEEE@AEPAXI@Z 0x004C4CA0-0x004C4CAB FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason":"Restore the lost compiler-generated Buffer-base deleting adjustor mapping to its actual WorldWin owner, not a hand-written receiver-arithmetic function.","evidence":["Original4C4CA0-4C4CAB subtracts444h from ECX and jumps to4C4C70 without changing the flags stack argument. Constructor4C4C4F installs the secondary table66DF28 at this+444h; its first slot selects this adjustor.","Historical canonical identity, extent and corrected-signature lesson are preserved in24472a5a^:src/init_thunks.cpp. The actual emitted secondary_E relocation names a SEARCH_LIBRARY weak primary_E with fallback to primary_G; COFF fallback alone does not prove final linker selection."]}
// symbol    ??_EWorldWin@@WEEE@AEPAXI@Z
// CORRECTED from ??3WorldWin@@SAXPAXI@Z: the 11-byte operation keeps the
// receiver in ECX and subtracts 0x444 before forwarding to a ret4 destructor.
// WEEE denotes adjustor{1092}; this is not a static operator delete overload.

// ORIGINAL: 0x004C3950 ?set_detail_map@WorldWin@@QAEXXZ 0x004C3950-0x004C398A BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original0x004C3950 writes1 to receiver+0x239CC;0x004C395A-0x004C3966 calls ButtonGroup::set(1007,0) on0x7CD12C, equal to MainInterfaceGlobal0x7AE820 plus its constructor-established group-array offset0x1E878 plus one0x94-byte element.0x004C396B-0x004C397D calls Win slot1 through MapWinTable[1] with its real virtual-base adjustment.0x004C3980-0x004C3985 directly tail-calls Win::hide on WorldClimateWorldWin0x8E9F60, not on this. Complete span agrees with src/recovered/units/004c3950.cpp."], "reason": "Recover the complete WorldWin detail-map operation using actual owned members, genuine virtual dispatch and the distinct global climate-window receiver; replace the seed-only raw-offset and fake-vtable model without deleting the seed or its historical obligation."}
void WorldWin::set_detail_map() {
    detail_map_selected_ = 1;
    MainInterfaceGlobal.button_group(1).set(1007, 0);
    // The map slot dispatches through Win; the climate window is a separate
    // global receiver whose hide call explicitly targets the Win base body.
    MapWinTable[1]->show(0);
    WorldClimateWorldWin.Win::hide();
}

// ORIGINAL: 0x004C3990 ?set_world_map@WorldWin@@QAEXXZ 0x004C3990-0x004C39CB BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original0x004C3990-0x004C39CB fully decodes59bytes and agrees with the retained004c3990 seed. Stores this+0x239CC=0, calls ButtonGroup::set(1006,0) at0x7CD12C, invokes Win hide slot2 through MapWinTable[1] virtual-base adjustment, then directly calls Win::show(0) at0x005ED9D0 on0x008E9F60."], "reason": "The world-map setter changes the remembered choice to zero, selects button1006, virtually hides the detail MapWin and directly shows the distinct global WorldWin through Win. Preserve the protected seed as a historical obligation."}
void WorldWin::set_world_map() {
    detail_map_selected_ = 0;
    MainInterfaceGlobal.button_group(1).set(1006, 0);
    MapWinTable[1]->hide();
    WorldClimateWorldWin.Win::show(0);
}

// ORIGINAL: 0x004C39D0 ?show_all@WorldWin@@QAEXXZ 0x004C39D0-0x004C3A39 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original0x004C39D0-0x004C3A39 fully decodes105bytes and agrees with the retained004c39d0 seed. Tests this+0x239CC for zero without writing it. Nonzero selects button1007, virtually shows MapWinTable[1] and directly hides WorldClimateWorldWin; zero selects button1006, virtually hides the detail map and directly shows the global Win base."], "reason": "Restore visibility from the remembered map selection without rewriting that selection. The detail and global climate windows are separate receivers with distinct virtual versus direct call forms."}
void WorldWin::show_all() {
    // Restore the remembered selection without changing it. In particular,
    // any nonzero selection chooses the detail map, not only the value one.
    if (detail_map_selected_ != 0) {
        MainInterfaceGlobal.button_group(1).set(1007, 0);
        MapWinTable[1]->show(0);
        WorldClimateWorldWin.Win::hide();
    } else {
        MainInterfaceGlobal.button_group(1).set(1006, 0);
        MapWinTable[1]->hide();
        WorldClimateWorldWin.Win::show(0);
    }
}

// ORIGINAL: 0x004C3A40 ?hide_all@WorldWin@@QAEXXZ 0x004C3A40-0x004C3A6B BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original0x004C3A40-0x004C3A6B fully decodes43bytes and agrees with retained004c3a40 seed. Calls ButtonGroup::set(-1,0), virtually invokes hide through MapWinTable[1] and tail-calls Win::hide0x005EDCD0 on0x008E9F60. No access to the WorldWin selection member occurs."], "reason": "Hide both map views and clear the button selection without changing the remembered map choice. Preserve genuine virtual-base dispatch for the detail map and the direct call on the global climate window."}
void WorldWin::hide_all() {
    // Hiding both views must not overwrite the selection restored by show_all.
    MainInterfaceGlobal.button_group(1).set(-1, 0);
    MapWinTable[1]->hide();
    WorldClimateWorldWin.Win::hide();
}

/*
Purpose: Construct the GraphicWin base and the three embedded subobjects
         (buffer1_, buffer2_, mapWin_), then install WorldWin's own vtables.
// ORIGINAL: 0x004C4BF0 ??0WorldWin@@QAE@XZ 0x004C4BF0-0x004C4C67;0x00659E50-0x00659E7E BYTE_EXACT
// size      119 bytes
// prototype void (__thiscall ??0WorldWin@@QAE@XZ)(WorldWin* this)
// callers   0   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok;frame
// calls     0x005D4CF0 0x005D7210 0x004626E0
Return Value: n/a
Status: Complete
*/
WorldWin::WorldWin() {
    // mapWin_ is a declared member: the compiler constructs it most-derived,
    // pushing the hidden flag 1 exactly where the image's ??0WorldWin pushes
    // it - the old `mapWin_.construct(1)` hand call is gone with MapWin's
    // real constructor.
}

// ORIGINAL: 0x004C4A70 ??1WorldWin@@QAE@XZ 0x004C4A70-0x004C4BEC;0x00659D60-0x00659E44 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence":["Original destruction at 0x004C4A70 clears and destroys mapWin_ at +0x152C, including its virtual GraphicWin at +0x22F98, then buffer2_ at +0xFA4, buffer1_ at +0xA1C and the direct GraphicWin base. Both original body and cleanup spans belong to this complete destruction.","Original FuncInfo and unwind map at 0x00678F88 and 0x00678FA8 describe 13 predecessor-linked cleanup states: the outer GraphicWin and Buffers, then the embedded MapWin TextureStore arrays/member, Buffers, Fonts and ImageButton array. There is no separate MapWin virtual-base cleanup state. These are member/base lifetime obligations, not hand-written destructor operations.","Original initializer 0x004C38B0 constructs WorldClimateWorldWin and registers teardown 0x004C38D0, which destroys the same receiver at 0x008E9F60. Natural C++ construction and implicit destruction express this ownership; bounded structural agreement does not establish throw-path runtime behavior, runtime-component equivalence, complete global extent or whole-image preservation."],"reason":"Recover complete destruction through genuine declared ownership, replacing the original-address forwarder that caused duplicate teardown. Preserve the canonical identity and both original spans while binding the naturally emitted implicit destructor."}
// symbol    ??1WorldWin@@UAE@XZ
// Compiler-generated implicit destructor: mapWin_, buffer2_, buffer1_, then
// GraphicWin own the complete teardown. There is no WorldWin-specific body.

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


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x004C3E90 ?on_left_double_click@WorldWin@@QAEXHH@Z 0x004C3E90-0x004C3E93 BYTE_EXACT
// symbol    ?on_left_double_click@WorldWin@@UAEXHH@Z
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
void WorldWin::on_right_click(int a, int b) {
    click(a, b, 1, 0);
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
    draw_contents();
    GraphicWin::soft_update();
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
void WorldWin::focus(int x_coord, int y_coord) {
    draw_contents();
    GraphicWin::soft_update();
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
    draw_contents();
    GraphicWin::soft_update();
}
