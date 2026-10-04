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
#include "map.h"
#include "alpha.h"
#include "basewin.h"
#include "console.h"
#include "faction.h"
#include "game.h"
#include "infowin.h"
#include "pulldown.h"
#include "palette.h"
#include "statuswin.h"
#include "strings.h"
#include "tutwin.h"

void __cdecl help_any();

// Four consecutive original dwords at 0x00939F08..14 are written by
// WorldWin::UNK1 and read as one RECT by MapWin::init. This interval lies in
// the pinned image's zero-fill data tail; the original starts all zero.
// RECOVERY_OBJECT: v2 0x00939F08 0x00939F18 ?WorldMapBounds@@3UtagRECT@@A 0x004C3A70
RECT WorldMapBounds;

int __cdecl pop_expire();

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004C3A70 ?UNK1@WorldWin@@QAEXXZ 0x004C3A70-0x004C3AA9
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/004c3a70.cpp and original disassembly 0x004C3A70-0x004C3AA9; MainInterfaceGlobal + 0xCFC is 0x007AF51C."], "reason": "Move the existing seed into the WorldWin owner using the MainInterface tutorial RECT and active map bounds, preserving the status bit operation."}
void WorldWin::UNK1() {
    WorldMapBounds = MainInterfaceGlobal.tutorial_bounds_;
    StatusFlags |= 8;
}

// The original startup path copies the tutorial bounds once, resets the
// embedded map and passes the same typed rectangle to the Win base init.
// ORIGINAL: 0x004C49A0 ?init@WorldWin@@QAEXH@Z 0x004C49A0-0x004C4A66
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/004c49a0.cpp complete seed; original 0x004C49A0-0x004C4A66 shows tutorial RECT copy, map clear/init, Win::init and virtual show."], "reason": "Home WorldWin startup through owned map, tutorial bounds and Win base; MapWin::init remains an explicit unfinished dependency."}
// TRIED: custom VCallW(int) shim for slot001 (scaffold's slot001 is nullary); byte-read/dword-write split for g_00939ed8's flag bit. MISMATCH from #4: register push scheduling around the four af51c/af520/af524/af528 locals differs
void WorldWin::init(int reset_map) {
    if ((StatusFlags & 8) == 0) {
        UNK1();
    }
    MapInputEnabled = 1;
    field_239BC_ = 0;
    field_239C0_ = 0;
    detail_map_selected_ = 0;
    viewport_outline_enabled_ = 0;
    mapWin_.field_1DD74_ = 1;
    if (reset_map != 0) {
        mapWin_.clear(1);
    }
    mapWin_.init(1, 0);
    field_239BC_ = 8;
    Win::init(&WorldMapBounds, 0, 0x20, &MainInterfaceGlobal, 0, 0);
    show(0);
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004C3AB0 ?right_menu@WorldWin@@QAEXHH@Z 0x004C3AB0-0x004C3D0C;0x00659E80-0x00659E95
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/004c3ab0.cpp and original 0x004C3AB0 disassembly; PullDown::get_selected_id owns 0xA20 item indexing."], "reason": "Home the retained world context menu over PullDown and owned map windows, preserving the EH cleanup span."}
void WorldWin::right_menu(int x, int y) {
    PullDown menu;
    int tile_x = 0;
    int tile_y = 0;
    bool in_range = pixel_to_tile(x, y, &tile_x, &tile_y) == 0;
    if (tile_x < 0 || tile_x >= MapLongitudeBounds ||
        tile_y < 0 || tile_y >= MapLatitudeBounds) {
        in_range = false;
    }
    Win::client_to_screen(&x, &y);
    menu.init(0);
    menu.add_item(4, label_get(0x300));
    menu.add_separator(0x63);
    if (in_range) {
        menu.add_item(1, label_get(0x244));
        menu.add_item(2, label_get(0x245));
    }
    menu.add_separator(0x63);
    menu.add_item(0x14, label_get(0x23F));
    menu.show(x, y, pop_expire);

    if ((GameState & 0x800) != 0) {
        return;
    }
    switch (menu.get_selected_id()) {
    case 1:
        MapWinTable[0]->set_center(tile_x, tile_y, 1);
        break;
    case 2:
        on_redraw();
        GraphicWin::soft_update();
        break;
    case 4:
        MapWinTable[1]->set_center(MapWinTable[0]->field_1DD9C_,
                                   MapWinTable[0]->field_1DDA0_, 1);
        set_detail_map();
        MainInterfaceGlobal.button_group(1).set(1007, 1);
        break;
    case 0x14:
        help_any();
        break;
    }
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004C3D40 ?click@WorldWin@@QAEXHHHH@Z 0x004C3D40-0x004C3E48
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/004c3d40.cpp; original disassembly 0x004C3D40-0x004C3E48; BaseWin+0x40B28 is 0x006E8150."], "reason": "Home the shared world click handler using owned tutorial state and map windows."}
void WorldWin::click(int x, int y, int button, int is_double) {
    TutWinBaseWindow.cancel_zoom_wait();
    if (button != 0) {
        right_menu(x, y);
        return;
    }
    const bool detail_click = (GetKeyState(VK_SHIFT) & 0x8000) != 0 || is_double != 0;
    int tile_x;
    int tile_y;
    if (pixel_to_tile(x, y, &tile_x, &tile_y) != 0 ||
        tile_y < 0 || tile_y >= MapLatitudeBounds ||
        tile_x < 0 || tile_x >= MapLongitudeBounds) {
        return;
    }
    if (detail_click) {
        MapWinTable[1]->set_center(MapWinTable[0]->field_1DD9C_,
                                   MapWinTable[0]->field_1DDA0_, 1);
        MainInterfaceGlobal.button_group(1).set(1007, 0);
        detail_map_selected_ = 1;
        MapWinTable[1]->show(0);
        WorldClimateWorldWin.Win::hide();
    } else {
        MapWinTable[0]->set_center(tile_x, tile_y, 1);
    }
}

// 0x004C4230 reads this flag and stores one/zero around the overview rebuild.
// Its address is adjacent to, but distinct from, MapInputEnabled at 0x0090D938.
// RECOVERY_OBJECT: v2 0x0090D93C 0x0090D940 ?WorldWinRebuildBusy@@3HA 0x004C4230
int WorldWinRebuildBusy = 0;

void __cdecl set_dirty();
void __cdecl clear_dirty();


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

// Superseded seed deleted; seed paths below are historical Git provenance.
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
    on_redraw();
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
    on_redraw();
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
    on_redraw();
    GraphicWin::soft_update();
}

// ORIGINAL: 0x004C43F0 ?pixel_to_tile@WorldWin@@QAEHHHPAH0@Z 0x004C43F0-0x004C44D3
// frame retention and the right clip-test polarity were sensitive. Its last
// mismatch was spill/register scheduling. This typed owner body is a fresh
// structural expression, not a claim that the old attempt's bytes match.
int WorldWin::pixel_to_tile(int pixel_x, int pixel_y, int *tile_x, int *tile_y) {
    const int origin_x = field_239C4_;
    int width;
    int negative_height;
    int relative_x;
    int relative_y;

    if (origin_x == 0 && field_239C8_ == 0) {
        relative_x = pixel_x;
        relative_y = pixel_y;
        width = Buffer::dib_.bmiHeader.biWidth;
        negative_height = -Buffer::dib_.bmiHeader.biHeight;
    } else {
        if (pixel_x < origin_x) {
            return 1;
        }
        const int origin_y = field_239C8_;
        if (pixel_y < origin_y) {
            return 1;
        }

        const int view_width = mapWin_.GraphicWin::dib_.bmiHeader.biWidth;
        if (pixel_x >= view_width + origin_x) {
            return 1;
        }
        const int view_height = mapWin_.GraphicWin::dib_.bmiHeader.biHeight;
        if (pixel_y >= origin_y - view_height) {
            return 1;
        }
        width = view_width;
        negative_height = -view_height;
        relative_x = pixel_x - origin_x;
        relative_y = pixel_y - origin_y;
    }

    *tile_x = (MapLongitudeBounds * relative_x) / width;
    int row = (MapLatitudeBounds * relative_y) / negative_height;
    *tile_y = row;
    if ((*tile_x & 1) == 0) {
        row &= 0xFFFFFFFE;
    } else {
        row |= 1;
    }
    *tile_y = row;

    if (row >= 0 && row < MapLatitudeBounds &&
        *tile_x >= 0 && *tile_x < MapLongitudeBounds) {
        return 0;
    }
    return 1;
}

// ORIGINAL: 0x004C44E0 ?tile_to_pixel@WorldWin@@QAEHHHPAH0@Z 0x004C44E0-0x004C45D2
// TRIED: nested `if`/`else if` clamp plus a single `||`-chained bounds check (`if (row<0 || row>=numRows || a1<0 || a1>=numCols) return 1;`) matches the prologue's push/pop set exactly (fixed by caching `numCols` in a local reused for BOTH final `idiv`s, matching `ebx` being loaded once and never reloaded) but stalls at 258B vs 242B: the last bounds check (`a1 < numCols`) always compiles to the INVERSE polarity (`jge`) of the original's `jl`, regardless of whether it is spelled as part of one big `||`, as a chain of nested positive `if`s, or with explicit `goto fail;` for the first three conditions - all three spellings produced byte-identical output, so the compiler's block layout here does not appear to be steerable from this function's own source text. Adding a `goto check_upper;` that skips the (proveably redundant) `row < 0` re-check on the zero-clamp path - mirroring the original's own `jmp` that skips exactly that check - got closest (240B vs 242B, divergence moves later) but the compiler then proves the SAME check dead on the OTHER path too (max-clamp / unclamped), deleting it where the original keeps it; splitting `row<0 || maxRow<0` into two independent `if`s so each one only proves ITS OWN half made this worse, not better. Best landed: the `goto check_upper` version. numCols is cached (`ebx`-shaped); numRows is cached for the bounds check (`edi`-shaped) but read fresh from `*g_00949874` for the final divisions, matching `idiv dword ptr [0x949874]` reloading rather than reusing a register.
// compiler inverted the final upper-bound branch, and a goto attempt caused
// a redundant lower-bound test to be optimized away. Kept as seed history;
// the typed body below has not yet been byte-measured.
int WorldWin::tile_to_pixel(int tile_x, int tile_y, int *pixel_x, int *pixel_y) {
    const int columns = MapLongitudeBounds;
    if ((MapIsFlat & 1) == 0) {
        if (tile_x < 0) {
            tile_x += columns;
        } else if (tile_x >= columns) {
            tile_x -= columns;
        }
    }

    const int rows = MapLatitudeBounds;
    const int max_row = rows - 1;
    int row = tile_y;
    if (row < 0 || max_row < 0) {
        row = 0;
        goto check_upper;
    }
    if (row > max_row) {
        row = max_row;
    }
    if (row < 0) {
        goto fail;
    }
check_upper:
    if (row >= rows || tile_x < 0 || tile_x >= columns) {
        goto fail;
    }

    if (field_239C4_ == 0 && field_239C8_ == 0) {
        *pixel_x = (Buffer::dib_.bmiHeader.biWidth * tile_x) / columns;
        *pixel_y = -((Buffer::dib_.bmiHeader.biHeight * row) /
                     MapLatitudeBounds);
        return 0;
    }

    *pixel_x = (mapWin_.GraphicWin::dib_.bmiHeader.biWidth * tile_x) /
                   columns + field_239C4_;
    *pixel_y = field_239C8_ -
               (mapWin_.GraphicWin::dib_.bmiHeader.biHeight * row) /
                   MapLatitudeBounds;
    return 0;

fail:
    return 1;
}

// ORIGINAL: 0x004C47F0 ?UNK3@WorldWin@@QAEXXZ 0x004C47F0-0x004C499C
// symbol    ?draw_viewport_outline@WorldWin@@QAEXXZ
void WorldWin::draw_viewport_outline() {
    if (viewport_outline_enabled_ == 0) {
        return;
    }

    MapWin *detail_map = MapWinTable[1];
    RECT rect;
    int left;
    int top;
    int right;
    int bottom;
    if (tile_to_pixel(detail_map->field_1DDA4_, detail_map->field_1DDA8_,
                      &left, &top) != 0) {
        return;
    }
    if (tile_to_pixel(detail_map->field_1DDD4_ + detail_map->field_1DDCC_ +
                          detail_map->field_1DDA4_,
                      detail_map->field_1DDD8_ + detail_map->field_1DDD0_ +
                          detail_map->field_1DDA8_,
                      &right, &bottom) != 0) {
        return;
    }
    rect.left = left;
    rect.top = top;
    rect.right = right;
    rect.bottom = bottom;

    if (rect.left <= rect.right) {
        Buffer::box(&rect, 0xFB, 0xFB);
        return;
    }

    const int line_top = rect.top;
    const int line_left = rect.left;
    const int line_right = rect.right - 1;
    const int line_bottom = rect.bottom - 1;
    Buffer::hline(field_239C4_, line_right, line_top, 0xFB);
    Buffer::hline(field_239C4_, line_right - 1, line_bottom, 0xFB);
    Buffer::vline(line_right, line_top + 1, line_bottom, 0xFB);

    const int right_edge = field_239C4_ != 0
                               ? mapWin_.GraphicWin::dib_.bmiHeader.biWidth +
                                     field_239C4_ - 1
                               : Buffer::dib_.bmiHeader.biWidth - 1;
    Buffer::hline(line_left + 1, right_edge, line_top, 0xFB);

    const int bottom_edge = field_239C4_ != 0
                                ? mapWin_.GraphicWin::dib_.bmiHeader.biWidth +
                                      field_239C4_ - 1
                                : Buffer::dib_.bmiHeader.biWidth - 1;
    Buffer::hline(line_right, bottom_edge, line_bottom, 0xFB);
    Buffer::vline(line_right, line_top, line_bottom - 1, 0xFB);
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004C4630 ?draw_box@WorldWin@@QAEXXZ 0x004C4630-0x004C47E7
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/004c4630.cpp; original 0x0093347C is ConsoleGlobal / MapWinTable[0] plus member offset 0x1DDCC."], "reason": "Home the selection outline over the primary MapWin member state and Buffer drawing."}
void WorldWin::draw_box() {
    MapWin *primary_map = MapWinTable[0];
    if (primary_map->field_1DDCC_ == 0 ||
        primary_map->field_1DDAC_ != 0 ||
        primary_map->field_1DDB0_ != 0) {
        return;
    }
    RECT rect;
    int left;
    int top;
    int right;
    int bottom;
    if (tile_to_pixel(primary_map->field_1DDA4_, primary_map->field_1DDA8_,
                      &left, &top) != 0 ||
        tile_to_pixel(primary_map->field_1DDD4_ + primary_map->field_1DDA4_ +
                          primary_map->field_1DDCC_,
                      primary_map->field_1DDD8_ + primary_map->field_1DDD0_ +
                          primary_map->field_1DDA8_,
                      &right, &bottom) != 0) {
        return;
    }
    rect.left = left;
    rect.top = top;
    rect.right = right;
    rect.bottom = bottom;
    if (rect.left <= rect.right) {
        Buffer::box(&rect, 0xFF, 0xFF);
        return;
    }
    Buffer::hline(field_239C4_, rect.right - 1, rect.top, 0xFF);
    Buffer::hline(field_239C4_, rect.right - 2, rect.bottom - 1, 0xFF);
    Buffer::vline(rect.right - 1, rect.top + 1, rect.bottom - 1, 0xFF);
    const int edge = field_239C4_ != 0
                         ? mapWin_.GraphicWin::dib_.bmiHeader.biWidth +
                               field_239C4_ - 1
                         : Buffer::dib_.bmiHeader.biWidth - 1;
    Buffer::hline(rect.left + 1, edge, rect.top, 0xFF);
    Buffer::hline(rect.left, edge, rect.bottom - 1, 0xFF);
    Buffer::vline(rect.left, rect.top, rect.bottom - 2, 0xFF);
}

// The redraw rebuilds the owned MapWin and buffers according to field_239BC_.
// All receiver offsets in the original are these declared subobjects.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004C3EC0 ?on_redraw@WorldWin@@QAEXXZ 0x004C3EC0-0x004C4227
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original 0x004C3EC0-0x004C4227 disassembly via recover inspect; src/recovered/units/004c3ec0.cpp seed has no implementation."], "reason": "Recover WorldWin redraw control flow through owned MapWin and Buffer subobjects while recording base-only overlay as an unfinished dependency."}
// symbol    ?on_redraw@WorldWin@@UAEXXZ
void WorldWin::on_redraw() {
    int scale = 6;
    if (MapWinCaptionScale > 0) {
        scale = MapWinCaptionScale * 50 / MapWinCaptionDivisor;
    }
    const int cell_size = ((scale + 1) / 4) * 4;
    Buffer *map_surface = static_cast<Buffer *>(
        static_cast<GraphicWin *>(&mapWin_));
    if (field_239BC_ & 8) {
        field_239BC_ |= 4;
        field_A14_ = (MapLongitudeBounds / 2) * cell_size * 2;
        field_A18_ = (MapLatitudeBounds / 2 - 2) * cell_size;
        mapWin_.field_1DDAC_ = 0;
        mapWin_.field_1DDB0_ = 0;
        mapWin_.field_1DD9C_ = MapLongitudeBounds / 2;
        mapWin_.field_1DD98_ = -14;
        mapWin_.field_1DDA0_ = MapLatitudeBounds / 2;
        mapWin_.field_1DD70_ = 0x80000000;
        mapWin_.resize(field_A14_, field_A18_, 1);
        buffer2_.init(field_A14_, field_A18_, 0, 0);
        buffer2_.sync_to_palette(PaletteActive);
        buffer1_.init(field_A14_, field_A18_, 0, 0);
        buffer1_.sync_to_palette(PaletteActive);
    }
    if (field_239BC_ & 5) {
        if (field_239BC_ & 4) {
            set_dirty();
            mapWin_.GraphicWin::fill(0);
        } else {
            buffer2_.copy(map_surface, 0, 0, 0, 0, field_A14_, field_A18_);
        }
        mapWin_.field_1DD70_ = 0x80000000;
        mapWin_.draw_map(0);
        buffer2_.copy(map_surface, 0, 0, 0, 0, field_A14_, field_A18_);
        clear_dirty();
    }
    if (field_239BC_ & 6) {
        buffer2_.copy(map_surface, 0, 0, 0, 0, field_A14_, field_A18_);
        mapWin_.field_1DD70_ = 0x80000080;
        mapWin_.gen_bases_only();
        buffer1_.copy(map_surface, 0, 0, 0, 0, field_A14_, field_A18_);
    }
    const int client_width = Buffer::dib_.bmiHeader.biWidth;
    const int client_height = -Buffer::dib_.bmiHeader.biHeight;
    if (client_width >= static_cast<int>(field_A14_) &&
        client_height >= static_cast<int>(field_A18_)) {
        field_239C4_ = (client_width - field_A14_) / 2;
        field_239C8_ = -(Buffer::dib_.bmiHeader.biHeight + field_A18_) / 2;
        buffer1_.copy(static_cast<Buffer *>(this), 0, 0,
                      field_239C4_, field_239C8_,
                      mapWin_.GraphicWin::dib_.bmiHeader.biWidth,
                      -mapWin_.GraphicWin::dib_.bmiHeader.biHeight);
    } else {
        SetStretchBltMode(Buffer::get_hdc(), 0);
        Buffer::release_hdc(1);
        buffer1_.copy(static_cast<Buffer *>(this), 0, 0,
                      field_A14_, field_A18_, 0, 0,
                      client_width, client_height);
        field_239C4_ = 0;
        field_239C8_ = 0;
    }
    draw_box();
    field_239BC_ = 0;
}

// ORIGINAL: 0x004C4230 ?UNK2@WorldWin@@QAEXH@Z 0x004C4230-0x004C43F0
// symbol    ?refresh_world_map@WorldWin@@QAEXH@Z
void WorldWin::refresh_world_map(int resize_buffers) {
    if (WorldWinRebuildBusy != 0) {
        return;
    }
    WorldWinRebuildBusy = 1;

    int scale = 6;
    if (MapWinCaptionScale > 0) {
        scale = (MapWinCaptionScale * 50) / MapWinCaptionDivisor;
    }
    const int cell_size = ((scale + 1) / 4) * 4;

    field_A14_ = (MapLongitudeBounds / 2) * cell_size * 2;
    field_A18_ = (MapLatitudeBounds / 2 - 2) * cell_size;
    mapWin_.field_1DD9C_ = MapLongitudeBounds / 2;
    mapWin_.field_1DD98_ = -14;
    mapWin_.field_1DDA0_ = MapLatitudeBounds / 2;
    mapWin_.field_1DD70_ = 0x80000000;

    Buffer *map_surface = static_cast<Buffer *>(
        static_cast<GraphicWin *>(&mapWin_));
    if (resize_buffers != 0) {
        set_dirty();
        mapWin_.resize(static_cast<int>(field_A14_),
                       static_cast<int>(field_A18_), 1);
        buffer2_.resize(static_cast<int>(field_A14_),
                         static_cast<int>(field_A18_));
        buffer1_.resize(static_cast<int>(field_A14_),
                         static_cast<int>(field_A18_));
        static_cast<GraphicWin &>(mapWin_).fill(0);
    } else {
        buffer2_.copy(map_surface, 0, 0, 0, 0,
                      static_cast<int>(field_A14_),
                      static_cast<int>(field_A18_));
    }

    mapWin_.field_1DD70_ = 0x80000000;
    mapWin_.draw_map(0);
    map_surface->copy(&buffer2_, 0, 0, 0, 0,
                      static_cast<int>(field_A14_),
                      static_cast<int>(field_A18_));

    clear_dirty();
    WorldWinRebuildBusy = 0;
}

// ORIGINAL: 0x004C38E0 sub_4c38e0 0x004C38E0-0x004C3910 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:world-map-view-callbacks", "Pinned original004c38e0; ButtonGroup7CD12C = MainInterface7AE820 +1E878 +94; MapWinTable[1]=7D3C40; global WorldWin8E9F60", "historical seed:src/recovered/004c38e0.cpp"], "reason": "Recover the complete global map-view operation through actual source-owned button group and windows. This callback does not change WorldWin selection state; preserve the historical byte obligation independently of current matching."}
// symbol    ?show_detail_map_view@@YAXXZ
void __cdecl show_detail_map_view() {
    MainInterfaceGlobal.button_group(1).set(1007, 0);
    MapWinTable[1]->show(0);
    WorldClimateWorldWin.Win::hide();
}

// ORIGINAL: 0x004C3910 sub_4c3910 0x004C3910-0x004C3941 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:world-map-view-callbacks", "Pinned original004c3910; ButtonGroup7CD12C = MainInterface7AE820 +1E878 +94; MapWinTable[1]=7D3C40; global WorldWin8E9F60", "historical seed:src/recovered/004c3910.cpp"], "reason": "Recover the complete global map-view operation through actual source-owned button group and windows. This callback does not change WorldWin selection state; preserve the historical byte obligation independently of current matching."}
// symbol    ?show_world_map_view@@YAXXZ
void __cdecl show_world_map_view() {
    MainInterfaceGlobal.button_group(1).set(1006, 0);
    MapWinTable[1]->hide();
    WorldClimateWorldWin.Win::show(0);
}
