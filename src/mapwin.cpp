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
#include "mapwin.h"
#include "console.h"
#include "statuswin.h"
#include "worldwin.h"
#include "spritebox.h"
#include "net_class.h"
#include "texture.h"
#include "imagebutton.h"
#include "map.h"       // MapLongitudeBounds / MapLatitudeBounds / MapIsFlat / map_tiles
#include "general.h"   // mem_get, and the game-state flags the painters read
#include <cstring>
#include <stdlib.h>


/*
Purpose: Build a map window - attach its embedded GraphicWin virtual base,
         then its own TextureStore/Buffer/Font/ImageButton members.
// ORIGINAL: 0x004626E0 ??0MapWin@@QAE@H@Z 0x004626E0-0x00462868;0x00655860-0x00655920 BYTE_EXACT
// symbol    ??0MapWin@@QAE@XZ
// size      584 bytes
// prototype void (__thiscall ??0MapWin@@QAE@H@Z)(MapWin* this, int)
// callers   3   call targets   9
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005D4CF0 0x006252A0 0x005D7210 0x00618EA0 0x006456E4 0x006457C2
Return Value: this

The compiler owns the construction machinery from the declarations in
mapwin.h: the vbtable store, the virtual GraphicWin base at +0x21A6C under
the most-derived flag, the two TextureStore vector ranges, the lone
TextureStore, the three Buffers, the three Fonts, the ImageButton array, and
the vbase-branch vtable pair the old hand body spelled at graphic_win and
graphic_win+0x444. The body keeps the eight scalar clears, which the image
runs after the members.
*/
MapWin::MapWin() {
    field_1DD74_ = 0;
    owned_ = 0;
    field_1DD80_ = 0;
    field_1DD8C_ = 0;
    field_21A44_ = 0;
    field_1EF4C_ = 0;
    field_1EEC0_ = static_cast<uint32_t>(0xffffd8f1);
    field_1EEC4_ = 0xffffffff;
}

/*
Purpose: Tear down a map window's own members. Nothing here calls
         MapWin::clear as MapWin - it is reached by name.
// ORIGINAL: 0x00420F90 ??1MapWin@@QAE@XZ 0x00420F90-0x004210CB;0x00651190-0x00651250 BYTE_EXACT
// TRIED: the three Buffer teardowns are the only divergence - the image makes
//            three `call ??1Buffer` (0x005D7410) where the in-class
//            `MEASURED virtual ~Buffer() { close(); }` in buffer.h expands
//            into each of them, and the expansion is what carries this body's
//            `sub esp, 8` for the spare slot, the `mov dword ptr [edi], 0`
//            vtable-null store and the `call 0x421088` unwind funclet - 93
//            instructions against the image's 73. Defining ~Buffer out of
//            line in buffer.cpp made THIS body byte-exact, 73 of 73, and
//            silenced the compiler's own deleting-destructor trio in every
//            other TU: deleting_thunks.cpp's ??_GBuffer claim at 0x005E3610
//            went UNRESOLVED, and that trade is not this file's to make.
//            Reverted; recorded here because the fix is one move away and
//            belongs to whoever owns both buffer.h and deleting_thunks.cpp.
// symbol    ??1MapWin@@UAE@XZ
// body      src/mapwin.h
// size      315 bytes
// prototype void (__thiscall ??1MapWin@@QAE@XZ)(MapWin* this)
// callers   1   call targets   6
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00462870 0x005D7410 0x00618EE0 0x006252B0 0x006456E4
Return Value: n/a

`this` IS NOT MapWin's own front, matching `guarded_teardowns.cpp`'s
`reinterpret_cast<MapWin *>(TeardownObject007F5ACC)->MapWin::~MapWin()`:
`TeardownObject007F5ACC` is `g_MAPWIN`'s own front (0x007D4060) plus
0x21A6C, the offset of MapWin's own embedded GraphicWin virtual base. The
destructor re-stores the same manual vtable pointers the constructor wrote,
then reaches its own front (`self - 0x21a6c`) only to call `clear(0)`.

HOMED INTO THE CLASS 2026-08-30: the hand-spelled body this spot carried
measured MISMATCH (1/73) because everything but `clear` is compiler-owned -
the EH frame, the vtable re-stores, the vtordisp zero, and the member
teardown of the TextureStore/Buffer/Font/ImageButton members mapwin.h now
declares. `MEASURED ~MapWin() { clear(0); }` in mapwin.h emits THIS piece
as the out-of-line COMDAT `??_DMapWin@@QAEXXZ` (the base-object destructor;
the `symbol` line above names it, per DECOMP_MAP's own rule for
compiler-generated constructs the catalogue spells ??1) and inlines the
same body into ??_DConsole. The funclet span 0x00651190-0x00651250 is that
EH machinery's own.
*/
/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x00467960 ?UNK3@MapWin@@QAEXXZ 0x00467960-0x00467961 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?UNK3@MapWin@@QAEXXZ)(MapWin* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void MapWin::UNK3() {
}

/*
Purpose: Refresh the map's image buttons; the legacy implementation is a bare
         return, so the buttons are never rebuilt here.
// ORIGINAL: 0x0046FB80 ?do_image_buttons@MapWin@@QAEXXZ 0x0046FB80-0x0046FB81 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?do_image_buttons@MapWin@@QAEXXZ)(MapWin* this)
// callers   2   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void MapWin::do_image_buttons() {
}


MainInterface MainInterfaceGlobal;  // 0x007AE820
// AN ARRAY: the image pushes 0x009B86A0 as an immediate. Extent is a
// floor - nothing recovered bounds the writes.
char MapWinMainCaption[256];  // 0x009B86A0

/*
Purpose: Push the map's caption into the main interface's date field. The
         original acts on the interface and caption it keeps at fixed
         addresses rather than on anything reached through the map window.
// ORIGINAL: 0x0046FB10 ?main_caption@MapWin@@QAEXXZ 0x0046FB10-0x0046FB20 BYTE_EXACT
// size      16 bytes
// prototype void (__thiscall ?main_caption@MapWin@@QAEXXZ)(MapWin* this)
// callers   11   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0045BE80
Return Value: n/a
Status: Complete
*/
void MapWin::main_caption() {
    MainInterfaceGlobal.set_date(MapWinMainCaption);
}


/*
Purpose: Close the map window - free the buffer it owns at 0x4, then close its
         graphic base. The base is located through the object's own vbtable,
         not a fixed offset, so an embedded MapWin closes its own base.
// ORIGINAL: 0x00470F70 ?close@MapWin@@QAEXXZ 0x00470F70-0x00470F98 BYTE_EXACT
// size      40 bytes
// prototype void (__thiscall ?close@MapWin@@QAEXXZ)(MapWin* this)
// callers   4   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4E40 0x00644EF2
Return Value: n/a
Status: Complete
*/
void MapWin::close() {
    if (owned_) {
        free(owned_);
        owned_ = nullptr;
    }
    // THE IDIOMATIC SPELLING, BY DIRECTION (2026-09-01): call the base's
    // close, no cast. This body's claim was demoted for it - and that is
    // the point. The compiled form of this line disagrees with the image's
    // 15-instruction dispatch, which means THIS TREE IS MISSING A
    // RELATIONSHIP the original's declarations had (candidate hypotheses on
    // the marker below). The hand vbtable walk the old body spelled went
    // byte-exact, but it was bad C++ that masked the gap; the mismatch is
    // the signal to explore, not a wall to transcribe.
    GraphicWin::close();
}

/*
Purpose: Answer WM_SYSCLOSE: clear the window's per-slot state words, then run
         close() - the same owned-pointer free and vbtable walk, inlined.
// ORIGINAL: 0x0046F880 ?on_sys_close@MapWin@@QAEXXZ 0x0046F880-0x0046F8C0 BYTE_EXACT
// LEVER: byte-exact on promotion from the 0046f880 archive unit. The Q
//   spelling enters with `this` on the GraphicWin virtual base, and the image
//   folds the guard and clear off the ENTERED this ([ecx - 0x3cf4],
//   [ecx - 0x3cf8]) while materializing the walked-back front in esi only for
//   the inlined close tail. Spell each flag access as a ONE-SHOT
//   `((MapWin *)((uint8_t *)this - 0x21A6C))->field...` conversion and VC6
//   folds it the same way; naming one `base` local up front materializes esi
//   at the top and puts the guard/store on [esi + 0x1dd78] instead
//   (TRIED: single base local for everything - 17 of 19, two addressing
//   divergences). The image has close() INLINED here - its two callees
//   (0x005D4E40 GraphicWin::close, 0x00644EF2 free) are the only calls - so
//   the body spells the free and the vbtable walk out rather than calling
//   close(), whose E8 would be an edge the image lacks.
// size      64 bytes
// prototype void (__thiscall ?on_sys_close@MapWin@@QAEXXZ)(MapWin* this)
// callers   0   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005D4E40 0x00644EF2
Return Value: n/a
Status: Complete
*/
void MapWin::on_sys_close() {
    if (from_graphic_base(this)->field_1DD78_ != 0) {
        from_graphic_base(this)->field_1DD74_ = 0;
        MapWin *const base = from_graphic_base(this);
        if (base->owned_ != nullptr) {
            free(base->owned_);
            base->owned_ = nullptr;
        }
        // The idiomatic spelling, by direction - see close() above.
        base->GraphicWin::close();
    }
}


/*
Purpose: Report a left click on the map, but only when map input is enabled.
         Reached through the GraphicWin virtual base, so `this` points there
         and is adjusted back 0x21A6C to the MapWin before dispatching.
// ORIGINAL: 0x0046EBA0 ?on_left_click@MapWin@@QAEXHH@Z 0x0046EBA0-0x0046EBC5 BYTE_EXACT
// size      37 bytes
// prototype void (__thiscall ?on_left_click@MapWin@@QAEXHH@Z)(MapWin* this, int, int)
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0046D5D0
Return Value: n/a
Status: Complete
*/
void MapWin::on_left_click(int a1, int a2) {
    if (*MapWinInputEnabled == 0) {
        return;
    }
    MapWin *const base = reinterpret_cast<MapWin *>(
        reinterpret_cast<uint8_t *>(this) - 0x21A6C);
    base->click(a1, a2, 0);
}

/*
Purpose: Report a right click on the map, but only when map input is enabled.
         Same virtual-base adjustment as the left click.
// ORIGINAL: 0x0046EBE0 ?on_right_click@MapWin@@QAEXHH@Z 0x0046EBE0-0x0046EC05 BYTE_EXACT
// size      37 bytes
// prototype void (__thiscall ?on_right_click@MapWin@@QAEXHH@Z)(MapWin* this, int, int)
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0046D5D0
Return Value: n/a
Status: Complete
*/
void MapWin::on_right_click(int a, int b) {
    if (*MapWinInputEnabled == 0) {
        return;
    }
    MapWin *const base = reinterpret_cast<MapWin *>(
        reinterpret_cast<uint8_t *>(this) - 0x21A6C);
    base->click(a, b, 1);
}


/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
// ORIGINAL: 0x0046EBD0 ?on_left_double_click@MapWin@@QAEXHH@Z 0x0046EBD0-0x0046EBD3 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_left_double_click@MapWin@@QAEXHH@Z)(MapWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void MapWin::on_left_double_click(int, int) {
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
// ORIGINAL: 0x0046EFF0 ?on_left_up@MapWin@@QAEXHH@Z 0x0046EFF0-0x0046EFF3 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_left_up@MapWin@@QAEXHH@Z)(MapWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void MapWin::on_left_up(int, int) {
}


/*
Purpose: Unknown; the legacy implementation is a constant return that returns 0.
// ORIGINAL: 0x00462970 ?UNK2@MapWin@@QAEHXZ 0x00462970-0x00462973 BYTE_EXACT
// size      3 bytes
// prototype int (__thiscall ?UNK2@MapWin@@QAEHXZ)(MapWin* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0, always
Status: Complete
*/
int MapWin::UNK2() {
    return 0;
}


func_map_win_draw_radius MapWinOriginalDrawRadius =
    original_method<func_map_win_draw_radius>(0x0046A2A0);

/*
Purpose: Broadcast a single-tile redraw to every live map window. Walks the
         eight map-window slots at 0x007D3C3C, skips empty ones, and for every
         slot past the first also skips windows whose 0x1DD74 activity dword is
         clear - slot 0 is the primary map window and always draws. Each
         surviving window is handed the tile through MapWin::draw_radius with a
         radius argument of 0; ?draw_tiles@@YAXHHH@Z at 0x0046B140 is the same
         body with a 1 there, and that single byte is the whole difference.
         The original opens no exception frame; there is nothing to omit.

         Kept as a full transcription rather than sharing a helper with
         draw_tiles, even though the two bodies differ in one byte. The
         originals are two separately compiled 70-byte functions, and
         tools/mutate_and_verify.py only derives mutants from bodies carrying
         an `Original Offset:` comment - and skips one-line bodies outright.
         Folding the walk into a shared helper would therefore hide the slot
         walk, the null guard, the slot-0 exemption, the activity gate and the
         argument order from mutation testing entirely, leaving both functions
         with zero mutants. The duplication buys verification.
// ORIGINAL: 0x0046AF40 ?draw_tile@@YAXHHH@Z 0x0046AF40-0x0046AF86 BYTE_EXACT
// LEVER: `window->draw_radius(...)` (the METHOD) in place of
//        `(ORIGINAL(window)->*MapWinOriginalDrawRadius)(...)` (the seam
//        variable) - the image's own call site is a direct `call 0x46a2a0`,
//        which the method compiles to; the seam load compiles an indirect
//        call. Same lever already used by the free `draw_radius` below.
// size      70 bytes
// prototype void (__cdecl ?draw_tile@@YAXHHH@Z)(int xCoord, int yCoord, int drawType)
// callers   34   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0046A2A0
Return Value: n/a. EAX on return is path-dependent leftover (draw_radius's own
              residue, the zero flag read at 0x0046AF5F, or a stale value when
              the last slot is empty), not a computed result, so unlike
              GraphicWin::close there is no residue to preserve and the void
              return is faithful.
Status: Complete with temporary MapWin::draw_radius original dependency
*/
// BODY IN mapwin.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


/*
Purpose: The radius-1 sibling of draw_tile - the identical 70-byte broadcast,
         but each live map window redraws the tile together with its
         surrounding ring rather than the single tile. The only difference in
         the originals is `push 1` at 0x0046B16D against `push 0` at
         0x0046AF6D; prologue, slot walk, null test, slot-0 exemption,
         activity gate, argument order, loop bound and epilogue all match
         instruction for instruction. Transcribed in full for the mutation
         coverage reason recorded on draw_tile.
// ORIGINAL: 0x0046B140 ?draw_tiles@@YAXHHH@Z 0x0046B140-0x0046B186 BYTE_EXACT
// LEVER: same as draw_tile above - `window->draw_radius(...)` in place of
//        the `MapWinOriginalDrawRadius` seam dispatch.
// size      70 bytes
// prototype 
// callers   10   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0046A2A0
Return Value: n/a; same path-dependent EAX leftover as draw_tile.
Status: Complete with temporary MapWin::draw_radius original dependency
*/
// BODY IN mapwin.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


// No redirect adapter for either. The original entry convention is __cdecl with
// three stack arguments and a caller-cleaned stack, and there is no
// this-adjustment to undo (contrast ListBox's destructor, entered at
// this + 0x48), so DllMain points 0x0046AF40 and 0x0046B140 straight at the
// recovered functions - the same shape as spying (0x0055BC00) and find_font
// (0x005882F0).

/*
Purpose: Report whether this object IS the process-wide Console.

             xor eax,eax / cmp ecx,0x9156B0 / sete al / ret

         An identity comparison against a fixed address, normalised to 0 or 1
         by `sete`. It dereferences nothing, so there is no memory access to
         model - only the address itself.

         0x009156B0 is already bound and named: ConsoleGlobal in console.h. So
         this adds no NEW fixed-address binding, it reuses the one the Console
         recovery already declared, and the comparison is written against the
         name rather than the literal.
// ORIGINAL: 0x00462960 ?UNK1@MapWin@@QAEHXZ 0x00462960-0x0046296C BYTE_EXACT
// size      12 bytes
// prototype int (__thiscall ?UNK1@MapWin@@QAEHXZ)(MapWin* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 1 when this is ConsoleGlobal, 0 otherwise
Status: Complete
*/
int MapWin::UNK1() {
    return this == reinterpret_cast<MapWin *>(&ConsoleGlobal) ? 1 : 0;
}


/*
// ORIGINAL: 0x0046B1D0 ?on_redraw@MapWin@@QAEXXZ 0x0046B1D0-0x0046B1E8 BYTE_EXACT
// size      24 bytes
// prototype void (__thiscall ?on_redraw@MapWin@@QAEXXZ)(MapWin* this)
// callers   0   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x0046A550
Status: Complete
*/
void MapWin::on_redraw() {
    if (*reinterpret_cast<int *>(reinterpret_cast<char *>(this) - 0x21a68)) {
        reinterpret_cast<MapWin *>(reinterpret_cast<char *>(this) - 0x21a6c)->draw_map(1);
    }
}

/*
// ORIGINAL: 0x0046FA00 ?on_resize@MapWin@@QAEXHH@Z 0x0046FA00-0x0046FA1A BYTE_EXACT
// size      26 bytes
// prototype void (__thiscall ?on_resize@MapWin@@QAEXHH@Z)(MapWin* this, int, int)
// callers   0   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x0046FB80
Status: Complete
*/
void MapWin::on_resize(int a1, int a2) {
    char *self = reinterpret_cast<char *>(this);
    if ((*reinterpret_cast<unsigned int *>(self - 0x3cfc) & 0x80000000) == 0) {
        reinterpret_cast<MapWin *>(self - 0x21a6c)->do_image_buttons();
    }
}

// The seam and definition for the unrecovered draw_map, at the end of the file
// so no recovered body above shifts.
//
// auto_inline(off) is load-bearing, not tidiness. Left alone, VC6 expands this
// one-line forwarder into MapWin::on_redraw (0x0046B1D0), reported as C4711 at
// mapwin.cpp(313). on_redraw is BYTE_EXACT today precisely because the original
// emits `call rel32` there, and an inlined `mov ecx / call [seam]` would break
// it. OPENSMACX_NOINLINE is the tree's spelling for this, but it expands to
// nothing on VC6 - the pragma is the only form cl 12.00 honours.
#pragma auto_inline(off)
func_map_win_draw_map MapWinOriginalDrawMap =
    original_method<func_map_win_draw_map>(0x0046A550);

void MapWin::draw_map(int draw_type) {
    (ORIGINAL(this)->*MapWinOriginalDrawMap)(draw_type);
}

void MapWin::draw_radius(int x_coord, int y_coord, int a3, int draw_type) {
    (ORIGINAL(this)->*MapWinOriginalDrawRadius)(x_coord, y_coord, a3, draw_type);
}
#pragma auto_inline(on)

/*
Purpose: Repaint every live MapWin slot - the FREE draw_map, not the method
         above. Mirrors draw_tile/draw_tiles in mapwin.h: slot 0 is exempt
         from the activity gate, every other slot only draws when its
         MapWinActiveOffset dword is non-zero.
// ORIGINAL: 0x0046B190 ?draw_map@@YAXH@Z 0x0046B190-0x0046B1CA BYTE_EXACT
// size      58 bytes
// prototype void (__cdecl ?draw_map@@YAXH@Z)(int)
// callers   68   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0046A550
Return Value: n/a
Status: Complete
*/
void __cdecl draw_map(int draw_type) {
    // A do-while walking a raw cursor over the table, not draw_tile's
    // indexed for-loop: the image parks the table's BASE ADDRESS in a
    // register (`mov esi, 0x7d3c3c`) and compares the cursor against it
    // directly (`cmp esi, 0x7d3c3c`) to recognise slot 0, rather than
    // comparing an index against 0. `int`, not a pointer: the loop
    // termination check is `jl`, a SIGNED compare, which a real pointer
    // comparison would not give.
    int cursor = reinterpret_cast<int>(MapWinTable);
    const int end = cursor + static_cast<int>(MapWinTableSlots) * 4;
    do {
        MapWin *const window = *reinterpret_cast<MapWin *const *>(cursor);
        if (window != nullptr) {
            if (cursor == reinterpret_cast<int>(MapWinTable) ||
                *reinterpret_cast<const volatile uint32_t *>(
                    reinterpret_cast<const uint8_t *>(window) +
                    MapWinActiveOffset) != 0) {
                // THE METHOD, not the seam directly: `window->draw_map(...)`
                // compiles the image's `call rel32` to
                // `?draw_map@MapWin@@QAEXH@Z`, where dispatching through
                // MapWinOriginalDrawMap here would be an indirect call
                // through the seam's own storage.
                window->draw_map(draw_type);
            }
        }
        cursor += 4;
    } while (cursor < end);
}

/*
Purpose: Repaint the radius around a tile on every live MapWin slot. Same
         shape as the free draw_map above, and the same fix: dispatch through
         the METHOD (MapWin::draw_radius), not the MapWinOriginalDrawRadius
         seam directly - the image's own call site is `call 0x46a2a0`, a
         direct rel32, where the seam variable loads through an indirect
         call.
// ORIGINAL: 0x0046AEF0 ?draw_radius@@YAXHHHH@Z 0x0046AEF0-0x0046AF38 BYTE_EXACT
// size      72 bytes
// prototype void (__cdecl ?draw_radius@@YAXHHHH@Z)(int, int, int, int)
// callers   2   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0046A2A0
Return Value: n/a
Status: Complete
*/
void __cdecl draw_radius(int x_coord, int y_coord, int a3, int draw_type) {
    int cursor = reinterpret_cast<int>(MapWinTable);
    const int end = cursor + static_cast<int>(MapWinTableSlots) * 4;
    do {
        MapWin *const window = *reinterpret_cast<MapWin *const *>(cursor);
        if (window != nullptr) {
            if (cursor == reinterpret_cast<int>(MapWinTable) ||
                *reinterpret_cast<const volatile uint32_t *>(
                    reinterpret_cast<const uint8_t *>(window) +
                    MapWinActiveOffset) != 0) {
                window->draw_radius(x_coord, y_coord, a3, draw_type);
            }
        }
        cursor += 4;
    } while (cursor < end);
}

// Three fixed-address windows terrain_fixup() also reaches, each pushed as a
// bare constant in the image (`push 0x8eb48c` etc.) rather than loaded
// through a variable - none has an established identity, so each is a raw
// address rather than a named accessor.
static MapWin *const MapWinTerrainFixupExtra1 =
    reinterpret_cast<MapWin *>(0x008EB48C);
static MapWin *const MapWinTerrainFixupExtra2 =
    reinterpret_cast<MapWin *>(0x006C5C9C);
static MapWin *const MapWinTerrainFixupExtra3 =
    reinterpret_cast<MapWin *>(0x007D4060);

/*
Purpose: Fix up terrain on every live MapWin slot, then on three more
         fixed-address windows.
// ORIGINAL: 0x00471240 ?mapwin_terrain_fixup@@YAXXZ 0x00471240-0x00471293 BYTE_EXACT
// size      83 bytes
// prototype void (__cdecl ?mapwin_terrain_fixup@@YAXXZ)()
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004711A0
Return Value: n/a
Status: Complete
*/
void __cdecl mapwin_terrain_fixup() {
    int cursor = reinterpret_cast<int>(MapWinTable);
    const int end = cursor + static_cast<int>(MapWinTableSlots) * 4;
    do {
        // BUG IN THE ORIGINAL: unlike draw_tile/draw_tiles/draw_radius, this
        // reads `(*cursor)->field_at(0x1DD74)` for every non-zero slot
        // WITHOUT first checking that the slot itself is non-null - only
        // slot 0 (`cursor == MapWinTable`) skips the read. An unpopulated
        // later slot would dereference a null MapWin*. Reproduced as-is.
        if (cursor == reinterpret_cast<int>(MapWinTable) ||
            *reinterpret_cast<int *>(
                reinterpret_cast<char *>(
                    *reinterpret_cast<MapWin *const *>(cursor)) +
                0x1DD74) != 0) {
            terrain_fixup(*reinterpret_cast<MapWin *const *>(cursor));
        }
        cursor += 4;
    } while (cursor < end);
    terrain_fixup(MapWinTerrainFixupExtra1);
    terrain_fixup(MapWinTerrainFixupExtra2);
    terrain_fixup(MapWinTerrainFixupExtra3);
}

/*
Purpose: Step the receiver back to the subobject ??_GMainInterface@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x0045F1F0 ??_GMainInterface@@WEEE@AEPAXI@Z 0x0045F1F0-0x0045F1FB BYTE_EXACT
// symbol    ??_EMainInterface@@WEEE@AEPAXI@Z
// CORRECTED from ??3MainInterface@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x0045F1C0` into
//   ??_GMainInterface@@UAEPAXI@Z, which executes `ret 4`; no stack
//   access and the receiver stays in ECX. `WEEE@` re-demangles to
//   adjustor{1092} and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/

// THE SLOT INDEX go_reset (time.cpp) works against, defined here beside the
// table it indexes. The image seeds the dword at 0x0068A5CC with 0xffffffff -
// no window selected - so the initialiser folds straight into .data and pays
// no dynamic initialiser.
int MapWinSelectedSlot = -1;

/*
// ORIGINAL: 0x0046EB90 ?on_lose_mouse_capture@MapWin@@QAEXXZ 0x0046EB90-0x0046EB95 BYTE_EXACT
// size      5 bytes
// prototype void (__thiscall ?on_lose_mouse_capture@MapWin@@QAEXXZ)(MapWin* this)
// callers   0   call targets   0
// kind      thunk
// flags     thunk;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
// Losing the pointer cancels the go mode wholesale: the whole body is one
// tail `jmp go_reset` (E9 at 0x0046EB90), which a void function whose only
// statement is the void call compiles to under /O2 - the same sibcall merge
// game_close (game.cpp) documents.
void MapWin::on_lose_mouse_capture() {
    go_reset();
}

// ====================
// 0x0068FAF0 - THE TILE ARRAY'S ROW STRIDE, the multiplier in every
// `x + stride * y` tile lookup this file does. A named binding rather than an
// object, the same convention map.h's AltNatural uses for its neighbour at
// 0x0068FB4C: the image holds it at this fixed address and nothing here maps
// that storage.
static int *const MapRowStride = (int *)0x0068FAF0;

// 0x00685684 and the five tables behind it - THE ALTITUDE BONUS LOOKUP
// `get_alt` walks. 0x00685684 is the direction table (five bytes a direction,
// the first naming it and the four after it the neighbours it fans out to);
// the other five are the per-terrain bonus rows that direction's byte is
// indexed into. Named bindings, not objects, for the same reason as
// MapRowStride above: the image holds them at these fixed addresses and
// nothing here maps that storage.
static const unsigned char *const AltDirectionTable = (const unsigned char *)0x00685684;
static const unsigned char *const AltBonusArid = (const unsigned char *)0x007D3B74;
static const unsigned char *const AltBonusMoist = (const unsigned char *)0x007F65D0;
static const unsigned char *const AltBonusRainy = (const unsigned char *)0x007F655C;
static const unsigned char *const AltBonusFlat = (const unsigned char *)0x007D3A78;
static const unsigned char *const AltBonusOcean = (const unsigned char *)0x007F64E8;
// 0x00685EA0 / 0x00685EE0 - the two coordinate-stride tables the same walk
// adds to a tile's x and y as it steps outward, indexed by byte offset.
static const unsigned char *const AltStepX = (const unsigned char *)0x00685EA0;
// 0x00691E6C / 0x00691E70 - the scale pair the altitude shading divides by:
// a numerator and a denominator, 1/0 meaning "take the fixed 1/16 step".
static int *const MapWinCaptionScale = (int *)0x00691E6C;
static int *const MapWinCaptionDivisor = (int *)0x00691E70;
static const unsigned char *const AltStepY = (const unsigned char *)0x00685EE0;

// ORIGINAL: 0x00468CD0 ?compute_clip@MapWin@@QAEHHHHPAURECT@@@Z 0x00468CD0-0x00468F8D FILE
// symbol    ?compute_clip@MapWin@@QAEHHHHPAUtagRECT@@@Z
//           The same windows.h `typedef struct tagRECT` mangling every other
//           `RECT *` parameter in this tree carries; the bytes are the body below.
// TRIED: this->field_1DDC0_ (offset 0x1DDC0, matches self[0x7770] and the class's own field name) and the virtual-base offset table at *(int*)this (vbptr[1] gives the displacement, then +this+0x474 or +0x4c8) reproduce the two IntersectRect-source-rect and default-bottom reads; diverges early (#6) given the function's size (701 bytes, four scan loops) - not chased further.
// working copy - scaffold materialised by --work
// size      701 bytes
// prototype int (__thiscall ?compute_clip@MapWin@@QAEHHHHPAURECT@@@Z)(MapWin* this, int, int, int, RECT*)
// callers   3   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00462F00
// indirect  0x00468EF0 0x00468F58


int MapWin::compute_clip(int a1, int a2, int a3, RECT * a4) {
    int *self = reinterpret_cast<int *>(this);
    RECT rect;
    int px, py;
    int count;
    int row;

    // LEFT edge
    int leftCol = a1 - a3 * 2;
    rect.left = 100000;
    if ((MapIsFlat & 1) == 0) {
        if (leftCol < 0) {
            leftCol = MapLongitudeBounds + leftCol;
        } else if (MapLongitudeBounds <= leftCol) {
            leftCol = leftCol - MapLongitudeBounds;
        }
    }
    if (a3 >= 0) {
        row = a2 - a3 * 2;
        count = a3 * 2 + 1;
        do {
            if (row >= 0 && row < MapLatitudeBounds && leftCol >= 0 && leftCol < MapLongitudeBounds) {
                tile_to_pixel(leftCol, row, &px, &py);
                if (px < rect.left) {
                    rect.left = px;
                }
            }
            row += 2;
            count--;
        } while (count != 0);
    }

    // RIGHT edge
    rect.right = 0;
    int rightCol = a1 + 2 + a3 * 2;
    if ((MapIsFlat & 1) == 0) {
        if (rightCol < 0) {
            rightCol = MapLongitudeBounds + rightCol;
        } else if (MapLongitudeBounds <= rightCol) {
            rightCol = rightCol - MapLongitudeBounds;
        }
    }
    if (a3 >= 0) {
        row = a2 - a3 * 2;
        count = a3 * 2 + 1;
        do {
            if (row >= 0 && row < MapLatitudeBounds && rightCol >= 0 && rightCol < MapLongitudeBounds) {
                tile_to_pixel(rightCol, row, &px, &py);
                if (rect.right < px) {
                    rect.right = px;
                }
            }
            row += 2;
            count--;
        } while (count != 0);
    }

    // TOP edge
    int topRow = a2 - a3 * 2;
    rect.top = 100000;
    if (topRow < 0 || MapLatitudeBounds <= topRow || a1 < 0 || MapLongitudeBounds <= a1) {
        rect.top = 0;
    } else if (a3 >= 0) {
        int col = a1 - a3 * 2;
        count = a3 * 2 + 1;
        do {
            tile_to_pixel(col, topRow, &px, &py);
            if (py < rect.top) {
                rect.top = py;
            }
            col += 2;
            count--;
        } while (count != 0);
    }

    // BOTTOM edge
    rect.bottom = 0;
    int bottomRow = a2 + 2 + a3 * 2;
    if (bottomRow < 0 || MapLatitudeBounds <= bottomRow || a1 < 0 || MapLongitudeBounds <= a1) {
        rect.bottom = -*reinterpret_cast<int *>(*reinterpret_cast<int *>(*self + 4) + 0x4c8 + reinterpret_cast<int>(self));
    } else if (a3 >= 0) {
        int col = a1 - a3 * 2;
        count = a3 * 2 + 1;
        do {
            tile_to_pixel(col, bottomRow, &px, &py);
            if (rect.bottom < py) {
                rect.bottom = py;
            }
            col += 2;
            count--;
        } while (count != 0);
    }

    rect.top = rect.top - self[0x7770];

    RECT *vbaseRect = reinterpret_cast<RECT *>(*reinterpret_cast<int *>(*self + 4) + reinterpret_cast<int>(self) + 0x474);
    int hit = IntersectRect(a4, &rect, vbaseRect);
    if (hit != 0) {
        if (a3 == 0) {
            int vgrow = (rect.bottom - rect.top) / 4;
            int hgrow = (rect.right - rect.left) / 4;
            rect.left = rect.left - hgrow;
            rect.right = rect.right + hgrow;
            rect.top = rect.top - vgrow;
            rect.bottom = rect.bottom + vgrow;
        }
        vbaseRect = reinterpret_cast<RECT *>(*reinterpret_cast<int *>(*self + 4) + reinterpret_cast<int>(self) + 0x474);
        IntersectRect(a4, &rect, vbaseRect);
    }

    if (a4->right != a4->left && a4->right - a4->left >= 0 && a4->bottom - a4->top > 0) {
        return 1;
    }
    return 0;
}

// ORIGINAL: 0x0046A4A0 ?draw_tile@MapWin@@QAEXHHH@Z 0x0046A4A0-0x0046A52F FILE
// working copy - scaffold materialised by --work
// size      143 bytes
// prototype void (__thiscall ?draw_tile@MapWin@@QAEXHHH@Z)(MapWin* this, int xCoord, int yCoord, int)
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0046A2A0

void MapWin::draw_tile(int a1, int a2, int a3) {
    char *self = reinterpret_cast<char *>(this);
    int rowStart = *reinterpret_cast<int *>(self + 0x1dda8);
    if (a2 >= rowStart &&
        a2 < *reinterpret_cast<int *>(self + 0x1ddd8) + *reinterpret_cast<int *>(self + 0x1ddd0) + rowStart) {
        int col = a1;
        if ((MapIsFlat & 1) == 0) {
            int colStart = *reinterpret_cast<int *>(self + 0x1dda4);
            if (a1 < colStart) {
                col = MapLongitudeBounds + a1;
            }
            if (*reinterpret_cast<int *>(self + 0x1ddd4) + *reinterpret_cast<int *>(self + 0x1ddcc) + colStart <= col) {
                col = col - MapLongitudeBounds;
            }
        }
        int colStart2 = *reinterpret_cast<int *>(self + 0x1dda4);
        if (col >= colStart2 &&
            col < *reinterpret_cast<int *>(self + 0x1ddd4) + *reinterpret_cast<int *>(self + 0x1ddcc) + colStart2) {
            draw_radius(a1, a2, 0, a3);
        }
    }
}

// ORIGINAL: 0x00470ED0 ?init_dummy@MapWin@@QAEXXZ 0x00470ED0-0x00470F68 FILE
// working copy - scaffold materialised by --work
// size      152 bytes
// prototype void (__thiscall ?init_dummy@MapWin@@QAEXXZ)(MapWin* this)
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4510 0x00644EF2

void MapWin::init_dummy() {
    unsigned char *base;
    int count;
    int offset;
    int i;
    int j;

    field_1DD6C_ = 0x19;
    if (owned_ != 0) {
        free(owned_);
        owned_ = 0;
    }
    owned_ = mem_get((MapArea + MapArea * 2) << 2);
    count = MapArea;
    if (count > 0) {
        offset = 0;
        i = 0;
        do {
            for (j = 1; j <= 4; j++) {
                base = (unsigned char *)owned_;
                base[offset + j - 1] = 0xFF;
            }
            for (j = 1; j <= 5; j++) {
                base = (unsigned char *)owned_;
                base[offset + j + 3] = 0xFF;
            }
            base = (unsigned char *)owned_;
            base[offset + 9] = 0;
            base = (unsigned char *)owned_;
            base[offset + 9] |= (*(unsigned char *)*map_tiles) & 0xD0;
            offset += 0xC;
            i++;
        } while (i < MapArea);
    }
}

static int get_alt_ratio(MapWin * w, int count) {
    if (*MapWinCaptionScale < 1) {
        int v = ((int)w->field_1DD98_ + 0x10) * count;
        return (v + ((v >> 0x1f) & 0xf)) >> 4;
    }
    return (*MapWinCaptionScale * count) / *MapWinCaptionDivisor;
}

// ORIGINAL: 0x0046FE70 ?get_alt@MapWin@@QAEHHHH@Z 0x0046FE70-0x00470414 FILE
// TRIED: get_alt_ratio() helper factoring the repeated scale-ratio computation (4 call sites collapsed to 1 call + reuse); recursive self-call kept inline per original; diverges at insn #3
// working copy - scaffold materialised by --work
// size      1444 bytes
// prototype int (__thiscall ?get_alt@MapWin@@QAEHHHH@Z)(MapWin* this, int, int, int)
// callers   8   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0046FE70


int MapWin::get_alt(int a1, int a2, int a3) {
    int dirIndex = a3;

    if (a2 < 0 || MapLatitudeBounds <= a2 || a1 < 0 || MapLongitudeBounds <= a1) {
        return 0;
    }
    unsigned char * tile = (unsigned char *)(*map_tiles + (*MapRowStride * a2 + (a1 >> 1)) * 0x2c);
    unsigned char terrain = *tile;
    if ((terrain & 0xe0) < 0x60) {
        return 0;
    }
    if (this->owned_ == 0) {
        return 0;
    }
    char * rec = (char *)this->owned_ + ((MapLongitudeBounds >> 1) * a2 + (a1 >> 1)) * 0xc;
    signed char cached = *(signed char *)(rec + 4 + a3);
    if (cached != -1) {
        return (int)cached;
    }

    unsigned int flags = *(unsigned int *)(tile + 0xc);
    int bonus = 0;
    int dirVal = (int)flags >> 0x18;
    unsigned char byteVal = AltDirectionTable[dirIndex + dirVal * 5];
    if ((flags & 2) != 0) {
        bonus = (int)AltBonusArid[byteVal];
    } else if ((flags & 1) != 0) {
        bonus = (int)AltBonusMoist[byteVal];
    } else if ((flags & 0x200) != 0) {
        bonus = (int)AltBonusRainy[byteVal];
    } else if ((flags & 0x100) != 0) {
        bonus = (int)AltBonusFlat[byteVal];
    } else if ((flags & 0x800) != 0) {
        bonus = (int)AltBonusOcean[byteVal];
    }

    if (dirIndex == 0) {
        int count = this->field_1DD6C_;
        int ratio = get_alt_ratio(this, count);
        int val = (((int)(terrain >> 5)) - 3) * ratio;
        if (bonus < 1) {
            if (bonus < 0) {
                int r = get_alt_ratio(this, count);
                val = val + (r * bonus) / 100;
                if (val < -0x7e) {
                    val = -0x7f;
                }
            }
        } else {
            int r = get_alt_ratio(this, count);
            val = val + (r * bonus) / 100;
            if (val > 0x7e) {
                val = 0x7f;
            }
        }
        int clamped = val;
        if (clamped > 0x7e) {
            clamped = 0x7f;
        }
        rec[4] = (char)clamped;
        return val;
    }

    int iVar9 = dirIndex * 0x10 - 0x10;
    bool useAltMode = false;
    int local_c = 0;
    int local_10 = 0;
    int local_14 = 0;
    int iVar6 = iVar9;
    for (;;) {
        int nx = *(int32_t *)((char *)AltStepX + iVar6) + a1;
        if ((*(unsigned char *)MapIsFlat & 1) == 0) {
            if (nx < 0) {
                nx = MapLongitudeBounds + nx;
            } else if (MapLongitudeBounds <= nx) {
                nx = nx - MapLongitudeBounds;
            }
        }
        int ny = *(int32_t *)((char *)AltStepY + iVar6) + a2;
        if (ny < 0 || MapLatitudeBounds <= ny || nx < 0 || MapLongitudeBounds <= nx ||
            (*(unsigned char *)(*map_tiles + (*MapRowStride * ny + (nx >> 1)) * 0x2c) & 0xe0) < 0x60) {
            break;
        }
        int sub = this->get_alt(nx, ny, 0);
        local_c = local_c + sub;
        local_10 = local_10 + (*(unsigned char *)(*map_tiles + (*MapRowStride * ny + (nx >> 1)) * 0x2c) >> 5);
        local_14 = local_14 + 1;
        iVar6 = iVar6 + 4;
        if (local_14 > 3) {
            goto merge;
        }
    }
    local_c = 0;
    local_10 = 0xc;
    useAltMode = true;

merge:
    local_c = local_c >> 2;
    {
        int count = this->field_1DD6C_;
        if (bonus < 1) {
            if (bonus < 0) {
                int r = get_alt_ratio(this, count);
                local_c = local_c + (r * bonus) / 100;
                if (local_c < -0x7e) {
                    local_c = -0x7f;
                }
            }
        } else {
            int r = get_alt_ratio(this, count);
            local_c = local_c + (r * bonus) / 100;
            if (local_c > 0x7e) {
                local_c = 0x7f;
            }
        }
    }
    dirIndex = -2;
    {
        int width = MapLongitudeBounds;
        int loopCheck;
        do {
            int nx = *(int32_t *)((char *)AltStepX + iVar9) + a1;
            if ((*(unsigned char *)MapIsFlat & 1) == 0) {
                if (nx < 0) {
                    nx = nx + width;
                } else if (width <= nx) {
                    nx = nx - width;
                }
            }
            int ny = *(int32_t *)((char *)AltStepY + iVar9) + a2;
            if (ny >= 0 && ny < MapLatitudeBounds && nx >= 0 && nx < width) {
                nx = nx >> 1;
                if (this->owned_ != 0) {
                    int stored = local_c;
                    if (local_c > 0x7e) {
                        stored = 0x7f;
                    }
                    *((char *)this->owned_ + (dirIndex & 3) + 5 + ((width >> 1) * ny + nx) * 0xc) = (char)stored;
                }
                width = MapLongitudeBounds;
                if (useAltMode) {
                    int slot = (dirIndex & 3) + 1;
                    if (this->owned_ != 0 && slot != 0) {
                        int shift = slot * 4 - 4;
                        char * cell = (char *)this->owned_ + ((MapLongitudeBounds >> 1) * ny + nx) * 0xc;
                        unsigned short * dst = (unsigned short *)(cell + 10);
                        *dst = (unsigned short)((~(0xf << (shift & 0x1f)) & *dst) | (4 << (shift & 0x1f)));
                        width = MapLongitudeBounds;
                    }
                } else {
                    int slot = (dirIndex & 3) + 1;
                    if (this->owned_ != 0 && slot != 0) {
                        int shift = slot * 4 - 4;
                        char * cell = (char *)this->owned_ + ((MapLongitudeBounds >> 1) * ny + nx) * 0xc;
                        unsigned short * dst = (unsigned short *)(cell + 10);
                        unsigned char t = *(unsigned char *)(*map_tiles + (*MapRowStride * ny + nx) * 0x2c);
                        int v = ((local_10 - ((t >> 3) & 0xfffffffc)) + 4) & 0xf;
                        *dst = (unsigned short)((~(0xf << (shift & 0x1f)) & *dst) | (v << (shift & 0x1f)));
                        width = MapLongitudeBounds;
                    }
                }
            }
            iVar9 = iVar9 + 4;
            loopCheck = dirIndex + 3;
            dirIndex = dirIndex + 1;
        } while (loopCheck < 4);
    }
    return local_c;
}

// ORIGINAL: 0x00462F00 ?tile_to_pixel@MapWin@@QAEXHHPAHPAH@Z 0x00462F00-0x0046303C FILE
// size      316 bytes
// prototype void (__thiscall ?tile_to_pixel@MapWin@@QAEXHHPAHPAH@Z)(MapWin* this, int xCoord, int yCoord, int*, int*)
// callers   18   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0046FE70
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00462F00
// measured tier  MISMATCH
// divergence     103
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00462f00/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?tile_to_pixel@MapWin@@QAEXHHPAHPAH@Z  at 0x00462F00  (316 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// The VC6 dialect limits and the source-form rules used to live here.
// They are knowledge, not scaffolding, so they now live in the agent
// system prompt (mizuchi.yaml, plugins.claude-runner.systemPrompt),
// where they can be edited without regenerating anything and are in
// context from the first token rather than behind a file read. This
// emitter computes declarations; it does not carry lessons.

void MapWin::tile_to_pixel(int a1, int a2, int * a3, int * a4) {
        int wrapped_x = a1;
    int map_width = MapLongitudeBounds;
    int y;

    if (a2 < field_1DDA8_ ||
        a2 >= field_1DDD8_ +
                  field_1DDD0_ +
                  field_1DDA8_) {
        goto out_of_range;
    }

    {
        int flag = MapIsFlat & 1;
        if (flag == 0) {
            if (a1 < field_1DDA4_) {
                wrapped_x = map_width + a1;
            }
            if (wrapped_x >= *reinterpret_cast<int32_t *>(0x1ddd4) +
                                  field_1DDCC_ +
                                  field_1DDA4_) {
                wrapped_x -= map_width;
            }
        }

        int x_min = field_1DDA4_;
        if (wrapped_x < x_min ||
            wrapped_x >= *reinterpret_cast<int32_t *>(0x1ddd4) +
                             field_1DDCC_ + x_min) {
            goto out_of_range;
        }

        y = a1 - x_min;
        if (flag != 0) {
            goto have_y;
        }
        if (y >= 0) {
            if (y >= map_width) {
                y -= map_width;
            }
            goto have_y;
        }
        y += map_width;
        goto have_y;
    }

out_of_range:
    y = a2 - field_1DDA4_;
    if (y > map_width / 2) {
        y -= map_width;
    }
    if (y >= -(map_width / 2)) {
        goto have_y;
    }
    y += map_width;

have_y:
    *a3 = field_1DDBC_ * (y - 1) +
          field_1DDAC_;

    {
        int sum = get_alt(a1, a2, 4);
        sum += get_alt(a1, a2, 3);
        sum += get_alt(a1, a2, 2);
        sum += get_alt(a1, a2, 1);

        int result_x = a2 - field_1DDA8_;
        int row_base = field_1DDB0_;
        sum >>= 2;
        result_x--;
        result_x *= field_1DDC0_;
        result_x += row_base;
        result_x -= sum;
        *a4 = result_x;
    }
}
