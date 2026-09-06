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
#include "lock.h"      // LockMapTable - terrain_fixup walks its records
#include "console.h"
#include "tutwin.h"    // TutWinGlobal - draw_cursor ends on its draw_arrow
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

// 0x00939E5C - the main interface's own copy of the screen width, taken when
// the main window is sized: the setter at 0x0051D760 does
// `*this_screen_width_ = WinScreenWidth; this_dword_ = WinScreenWidth;` in
// that order, and `*g_00939EF0 = *this` copies it back out. Every reader
// tests it against 1000 to pick between the two fixed layouts - 550x300
// below, 704x384 at and above (0x00589760), 200x180 / 250x212 in
// InfoWin::UNK4 - which is 0.6875 of 800 and of 1024 alike, so the threshold
// is "is the desktop the wide one". A named binding, not an object, for the
// same reason as MapRowStride below: the image holds the dword at this fixed
// address and nothing here maps that storage.
static int *const IfaceScreenWidth = (int *)0x00939E5C;

// THE POINT-LIGHT TABLES. Three parallel signed-byte tables, one per tile
// flag, each indexed by the byte `AltDirectionTable` picks for the tile's
// code and the corner being lit - exactly the indexing the AltBonus* family
// below gets, so they are the same shape of table in the same family:
//
//     mov cl, byte ptr [eax + edx + 0x685684]   ; AltDirectionTable[code*5 + corner]
//     movsx edx, byte ptr [ecx + 0x685a24]      ; -> one of the three here
//
// `get_point_light` (0x00470790) picks between them on three low bits of the
// tile's `bit2` dword - 0x1, 0x2 and 0x200 - and nothing else in the tree
// names what those flags mean, so the names carry the selecting bit rather
// than a guessed terrain class. 0x00685A24 and 0x00685B60 are in the file
// image's .data and their bytes read as small signed deltas (-3..+3); the
// third, 0x007F6750, is past the bytes the file carries and is zero until
// something fills it.
static const signed char *const AltLightBit1 = (const signed char *)0x00685A24;
static const signed char *const AltLightBit2 = (const signed char *)0x00685B60;
static const signed char *const AltLightBit200 = (const signed char *)0x007F6750;

// 0x007F6680 - THE CORNER SLOPE-LIGHT TABLES. Four 0x31-byte 7x7 runs, one
// per tile corner, each read as
//
//     lea ebx, [eax*8] / sub ebx, eax            ; v * 7
//     movsx ebx, byte ptr [ebx + eax + 0x7f6680] ; [v*7 + w]
//
// where v and w are the two altitude-detail nibbles the visibility record
// holds for the two tiles that meet at that corner. `get_brighting`
// (0x00470550) indexes the four runs with the constants 0x7F6680, 0x7F66B1,
// 0x7F66E2 and 0x7F6713, so the stride is 0x31 = 49 = 7*7 and this one base
// serves all four.
//
// NOT the table `compute_lighting_table` (0x00470420) builds: that one walks
// `lea eax, [eax + ebx + 0x7f6688]` with eax = 0x30*q - the same 4 x 49 shape
// built from `-3*(x+y)/sqrt(x*x+y*y)` over a 7x7 slope grid, but starting
// EIGHT BYTES HIGHER. The two runs overlap for 188 of their 196 bytes, so
// they are not the same storage, and only the 0x7F6688 one has a recovered
// writer. Bound here at the address `get_brighting` actually reads.
static const signed char *const CornerLightTable = (const signed char *)0x007F6680;

// 0x00685684 and the five tables behind it - THE ALTITUDE BONUS LOOKUP
// `get_alt` walks. 0x00685684 is the direction table (five bytes a direction,
// the first naming it and the four after it the neighbours it fans out to);
// the other five are the per-terrain bonus rows that direction's byte is
// indexed into. Named bindings, not objects, for the same reason as
// MapRowStride above: the image holds them at these fixed addresses and
// nothing here maps that storage.
// 0x00685EA0 / 0x00685EE0 - the two coordinate-stride tables the same walk
// adds to a tile's x and y as it steps outward, indexed by byte offset.
// 0x00691E6C / 0x00691E70 - the scale pair the altitude shading divides by:
// a numerator and a denominator, 1/0 meaning "take the fixed 1/16 step".
// THE VIRTUAL-BASE DISPLACEMENT the compiler lays MapWin down with: the
// vbtable it emits holds {0, 0x21A6C} (mapwin.h), so every handler the
// image enters on the GraphicWin base walks `entered - 0x21A6C` to get to
// the front, and every vbase-relative field sits at front + 0x21A6C + N.
// Held as a constant rather than read back out of the vbtable: the table
// is the compiler's, and reading it costs a this-pun at each site.
static const int32_t MapWinEnteredOffset = 0x21A6C;

// The receiver the image hands every Q-spelled MapWin handler: the GraphicWin
// virtual base, not the front. Named once here because four recovered bodies
// walk it once by name rather than re-deriving the displacement at each
// call site, where it reads as four unrelated puns instead of the one
// documented walk.
static char *entered_receiver(MapWin *window) {
    return reinterpret_cast<char *>(window) + MapWinEnteredOffset;
}

static int *const MapWinCaptionScale = (int *)0x00691E6C;
static int *const MapWinCaptionDivisor = (int *)0x00691E70;
static const unsigned char *const AltStepY = (const unsigned char *)0x00685EE0;

// 0x007D3C28..0x007D3C34 - FOUR CONSECUTIVE INTS the primary window publishes
// about itself. `calculate_dimensions` writes them only when `this` is
// ConsoleGlobal, and only here: first the window's centre tile
// (field_1DD9C_ / field_1DDA0_, i.e. x then y), then the same two again with
// the tile-step counts folded in -
//
//     [0] = (x_centre - tiles_across) + 4
//     [1] = (y_centre - tiles_down ) + 4
//     [2] = (tiles_across + x_centre) - 4
//     [3] = (tiles_down  + y_centre) - 6
//
// which reads as the west, north, east and south limits of the region the
// primary window is currently drawing. They sit immediately below the map
// window slot table at 0x007D3C3C, and nothing else in the recovered tree
// reads or writes them, so the "corner limits of the primary window" reading
// is what the arithmetic says and no more.
static int *const MapWinPrimaryCorner = (int *)0x007D3C28;

// 0x009393C0 - in the Console state block beside CursorLastIndex/CursorLastX.
// on_nc_left_down reads it once and returns without touching the keyboard when
// it is nonzero, and otherwise polls and writes the modifier-key state, so the
// nonzero value is "do not refresh the modifier state". Bound here because
// nothing in the tree names it; the Console side owns the address.
static int *const KeyStatePollSuppressed = (int *)0x009393C0;

// 0x0066932C / 0x00669330 - the USER32 import-table slots the tree reads as
// `int (__stdcall *)(int)` and calls with a virtual key:
//   GetKeyState(0x91)     (VK_SCROLL) - is Scroll Lock latched
//   GetAsyncKeyState(0x10)(VK_SHIFT)  - the high byte is the pressed bit
// Named for the import `iat_names.py` resolves each address to, because a
// fixed-address binding to an IAT slot is a function pointer and the tree has
// no other spelling for one.
static int *const IAT_GetKeyState = (int *)0x0066932C;
static int *const IAT_GetAsyncKeyState = (int *)0x00669330;

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

    int hit = IntersectRect(a4, &rect, &rect2_);
    if (hit != 0) {
        if (a3 == 0) {
            int vgrow = (rect.bottom - rect.top) / 4;
            int hgrow = (rect.right - rect.left) / 4;
            rect.left = rect.left - hgrow;
            rect.right = rect.right + hgrow;
            rect.top = rect.top - vgrow;
            rect.bottom = rect.bottom + vgrow;
        }
        IntersectRect(a4, &rect, &rect2_);
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
    int rowStart = field_1DDA8_;
    if (a2 >= rowStart &&
        a2 < field_1DDD8_ + field_1DDD0_ + rowStart) {
        int col = a1;
        if ((MapIsFlat & 1) == 0) {
            int colStart = field_1DDA4_;
            if (a1 < colStart) {
                col = MapLongitudeBounds + a1;
            }
            if (field_1DDD4_ + field_1DDCC_ + colStart <= col) {
                col = col - MapLongitudeBounds;
            }
        }
        int colStart2 = field_1DDA4_;
        if (col >= colStart2 &&
            col < field_1DDD4_ + field_1DDCC_ + colStart2) {
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

// ORIGINAL: 0x004711A0 ?terrain_fixup@@YAXPAUMapWin@@@Z 0x004711A0-0x0047123B FILE
// symbol    ?terrain_fixup@@YAXPAVMapWin@@@Z
// MapWin is a class here and a struct in the image, so the tag differs.
// TRIED: hoisting `base = owned_+offset` into its own statement before the `j = j+1` increment, to force add-before-inc ordering - same MISMATCH #30 'add' vs 'inc' either way (/O2 schedules inc first regardless of source order). similarity 0.9667, rebuilt_bytes == original_bytes (155).
// working copy - scaffold materialised by --work
// size      155 bytes
// prototype 
// callers   1   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005D4510 0x00644EF2
void __cdecl terrain_fixup(MapWin * window) {
    int count2 = 0;
    if (window != 0) {
        if (window->owned_ != 0) {
            free(window->owned_);
            window->owned_ = 0;
        }
        window->owned_ = mem_get(MapArea * 0xc);
        if (MapArea > 0) {
            int offset = 0;
            do {
                int j = 0;
                do {
                    j = j + 1;
                    *(reinterpret_cast<unsigned char *>(window->owned_) + offset - 1 + j) = 0xff;
                } while (j < 4);
                j = 0;
                do {
                    j = j + 1;
                    *(reinterpret_cast<unsigned char *>(window->owned_) + offset + 3 + j) = 0xff;
                } while (j < 5);
                *(reinterpret_cast<unsigned char *>(window->owned_) + 9 + offset) = 0;
                unsigned char *record = reinterpret_cast<unsigned char *>(window->owned_) + 9 + offset;
                offset = offset + 0xc;
                *record = *record | (*LockMapTable & 0xd0);
                count2 = count2 + 1;
            } while (count2 < MapArea);
        }
    }
}

// ORIGINAL: 0x0046FC80 ?UNK6@MapWin@@QAEXHHH@Z 0x0046FC80-0x0046FCD3 FILE
// TRIED: naming the three tables `static int *const` - /O2 then folds the
//   0x66F440-0x66EFBC pair into one walking pointer with an `add`, which the
//   image never does: it keeps each table as its own SIB-indexed base. Spelled
//   `extern int NAME[]` (map.h's arrays) the addresses stay unknown at compile
//   time and the folding cannot happen. MISMATCH #23.
// size      83 bytes
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// TRIED: `reinterpret_cast<int *>(owned_)[idx * 3]` (one load, the image's
//   shape) versus `(*reinterpret_cast<int **>(owned_))[idx * 3]` (two) - the
//   second scores 26 of 30, the first 21 of 30, because with it VC6 folds
//   `RadiusRange[a3 + 1]` into `[eax*4 + 4]` where the image holds
//   `[eax*4 + 0x66f8c8]`: map.h's RadiusRange is a const header array, so its
//   address is a per-TU constant and the +1 becomes a displacement instead of
//   part of an absolute base. An `extern` RadiusRange would give the image's
//   disp32 shape but moves the array's linkage in every TU that uses it.
void MapWin::UNK6(int UNUSED(x) x, int UNUSED(y) y, int radius) {
    if (owned_ != 0) {
        int i = 0;
        int count = RadiusRange[radius + 1];
        if (count > 0) {
            do {
                // the abstract-map index: y halved-width rows, x halved
                int idx = RadiusOffsetY[i] * (static_cast<int>(MapLongitude) >> 1)
                        + (RadiusOffsetX[i] >> 1);
                i++;
                (*reinterpret_cast<int **>(owned_))[idx * 3] = -1;
            } while (i < count);
        }
    }
}

// ORIGINAL: 0x0046FB90 ?clear_alt@MapWin@@QAEXHHH@Z 0x0046FB90-0x0046FC29 FILE
// TRIED: 153/153 bytes (edit distance 1 pair, same total size).
// Original writes the -1 fill via `or edx,0xffffffff` into a register
// it already holds, then stores that register (2 ops); every source
// form tried here - a direct `*rec = -1`, a named `fill` temp shared
// between the int32 and byte stores, writing the byte store before the
// int32 store - still lowers to a single fused `mov dword ptr[rec],-1`
// (equal cost, different bytes). ~10 tries; not found.
// size      153 bytes
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
void MapWin::clear_alt(int x, int y, int radius) {
    if (owned_ != 0) {
        int i = 0;
        int count = RadiusRange[radius + 1];
        if (count > 0) {
            int width = MapLongitude;
            do {
                int dx = RadiusOffsetX[i] + x;
                if ((MapIsFlat & 1) == 0) {
                    if (dx < 0) {
                        dx += width;
                    } else if (dx >= width) {
                        dx -= width;
                    }
                }
                int dy = RadiusOffsetY[i] + y;
                if (dy >= 0 && dy < MapLatitudeBounds && dx >= 0 && dx < width) {
                    char *rec = reinterpret_cast<char *>(owned_)
                                + ((width >> 1) * dy + (dx >> 1)) * 0xc + 4;
                    *reinterpret_cast<int32_t *>(rec) = -1;
                    *reinterpret_cast<uint8_t *>(rec + 4) = 0xff;
                    width = MapLongitude;
                }
                i++;
            } while (i < count);
        }
    }
}

// ORIGINAL: 0x0046FCE0 ?UNK7@MapWin@@QAEXXZ 0x0046FCE0-0x0046FD1A FILE BYTE_EXACT
// size      58 bytes
// prototype void (__thiscall ?UNK7@MapWin@@QAEXXZ)(MapWin* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// PRESERVED UNIT - measured NO_COMPILE.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0046FCE0
// measured tier  NO_COMPILE
// refusal        u0046fce0.cpp(1) : error C2653: 'MapWin' : is not a class or namespace name u0046fce0.cpp(2) : error C2065: 'owned_' : undeclared identifier u0046fce0.cpp(3) : 
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0046fce0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?UNK7@MapWin@@QAEXXZ  at 0x0046FCE0  (58 bytes)
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


void MapWin::UNK7() {
    if (owned_ != 0) {
        for (int i = 0; i < static_cast<int>(MapArea); ++i) {
            for (int j = 0; j < 4; ++j) {
                reinterpret_cast<char *>(owned_)[i * 0xC + j] = static_cast<char>(0xFF);
            }
        }
    }
}

// ORIGINAL: 0x0046FE00 ?get_relative_alt@MapWin@@QAEHHHH@Z 0x0046FE00-0x0046FE67 FILE BYTE_EXACT
// working copy - scaffold materialised by --work
// size      103 bytes
// prototype int (__thiscall ?get_relative_alt@MapWin@@QAEHHHH@Z)(MapWin* this, int, int, int)
// callers   4   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0046FE70

int MapWin::get_relative_alt(int x, int y, int detail) {
    get_alt(x, y, detail);
    if (owned_ == 0) {
        return 0;
    }
    if (detail == 0) {
        return 4;
    }
    int row = (static_cast<int>(MapLongitude) >> 1) * y + (x >> 1);
    unsigned short val = *reinterpret_cast<unsigned short *>((char *)owned_ + 10 + row * 12);
    int shift = (detail - 1) * 4;
    return (val >> shift) & 0xf;
}

// ORIGINAL: 0x0046AE00 ?draw_cursor@@YAXXZ 0x0046AE00-0x0046AEED FILE
// NOTE ON THE HANDOFF: this was queued as "declare it on MapWin", but the
// image's mangled name is ?draw_cursor@@YAXXZ - a FREE function taking
// nothing, not a member. It walks every slot of the map-window table rather
// than acting on one window, which is what the name is hiding. So it lives at
// file scope beside the other free drawing helpers, not on the class.
//
// TRIED: MapWinTable spelled as the header's `MapWinSlots &` accessor. The
// image walks a live cursor (`add ebx,4 / cmp ebx,0x7d3c5c`) and an index loop
// compares a different thing; the walk is kept, the end bound spelled as the
// table's own one-past-the-end slot.
// size      237 bytes
// callers   2   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0046A2A0 0x004BDEA0
// The one guard this body reads before drawing anything: zero means the cursor
// is clear to draw. No other site in the tree names it, and the archive only
// ever binds it anonymously, so the polarity above is the whole evidence.
int *const CursorDrawSuppressed = (int *)0x009392B0;

void __cdecl draw_cursor() {
    if (*CursorDrawSuppressed == 0) {
        MapWinSlots &table = map_win_table();
        MapWin **slot = table;
        do {
            MapWin *window = *slot;
            if (window != 0 &&
                (slot == table ||
                 window->field_1DD74_ != 0)) {
                int index = CursorLastIndex;
                int tile_y = CursorLastY[index];
                int tile_x = CursorLastX[index];
                if (window->field_1DDA8_ <= tile_y &&
                    tile_y < window->field_1DDD8_ +
                             window->field_1DDD0_ +
                             window->field_1DDA8_) {
                    int wrapped_x = tile_x;
                    if ((MapIsFlat & 1) == 0) {
                        if (tile_x < window->field_1DDA4_) {
                            wrapped_x = MapLongitude + tile_x;
                        }
                        if (window->field_1DDD4_ +
                            window->field_1DDCC_ +
                            window->field_1DDA4_ <= wrapped_x) {
                            wrapped_x = wrapped_x - MapLongitude;
                        }
                    }
                    if (window->field_1DDA4_ <= wrapped_x &&
                        wrapped_x < window->field_1DDD4_ +
                                    window->field_1DDCC_ +
                                    window->field_1DDA4_) {
                        window->draw_radius(tile_x, tile_y, 0, 2);
                    }
                }
            }
            slot++;
        } while (slot < table + MapWinTableSlots);
    }
    TutWinGlobal->draw_arrow();
}

// Fixed-slot bindings carried from 00462870.cpp

// ORIGINAL: 0x00462870 ?clear@MapWin@@QAEXH@Z 0x00462870-0x00462959 FILE
// HOMED 2026-09-04 from the 0x00462870 artifact; the artifact's
// `g_9392c0`/`g_939340`/`g_009392b8`/`g_00949870`/`g_00949874` are the
// tree's CursorLastX / CursorLastY / CursorLastIndex (console.h) and
// MapLongitudeBounds / MapLatitudeBounds (map.h), and `g_00939e5c` is the new
// IfaceScreenWidth binding above. Measured in the tree: 8/65.
// TRIED: `g_939340`/`g_9392c0` as the scaffold's `int *const` globals - the address does work (indexed by `*g_009392b8`), so switched to `extern int name[]`. Landed at 97.7%: the original loads `*g_00939e5c` into a register before comparing it to 1000, where this source's compiler folds the load into `cmp dword ptr[addr],1000` directly - tried both an inline dereference and a named local holding the read first; the compiler made the same choice either way. The `((t<1000)-1)&6` then `-6` also comes out as `sub eax,6` here against the original's `add eax,-6` (same value, different opcode).
// TRIED: `int *volatile const IfaceScreenWidth` - volatile on the pointee to
// force the image's `mov edx,[0x939e5c]` ahead of the `cmp` - measured no
// change at all (still 8/65, still the folded memory-operand `cmp`): VC6
// folds a volatile read into a compare operand just as it folds a plain one,
// so volatile buys nothing at this site. Reverted.
// working copy - scaffold materialised by --work
// size      233 bytes
// prototype void (__thiscall ?clear@MapWin@@QAEXH@Z)(MapWin* this, int)
// callers   12   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00644EF2

void MapWin::clear(int index) {
    field_1DD78_ = index;
    field_1DD70_ = 0;
    field_1DD98_ = (((*IfaceScreenWidth < 1000) - 1) & 6) - 6;
    field_1DD74_ = (index == 0);

    int halfHeight = MapLatitudeBounds / 2;
    field_1DDA0_ = halfHeight;
    int halfWidth = MapLongitudeBounds / 2;
    field_1DD9C_ = (halfWidth ^ halfHeight) & 1 ^ halfWidth;

    if (index == 0) {
        int player = CursorLastIndex;
        int cy = CursorLastY[player];
        int cx = CursorLastX[player];
        if (cy >= 0 && cy < MapLatitudeBounds && cx >= 0 && cx < MapLongitudeBounds) {
            field_1DD9C_ = cx;
            field_1DDA0_ = CursorLastY[CursorLastIndex];
        }
    }

    field_1EF50_ = 0x7f00;
    field_1EF4C_ = 0;
    if (owned_ != 0) {
        free(owned_);
        owned_ = 0;
    }
}


// THE VISIBILITY RECORD, one 0xC-byte slot per half-map column and row, that
// `owned_` points at. Bytes 0..3 hold the four cached corner brightnesses
// (0 = "not computed", otherwise the value biased by 0x80) and bytes 10 and 11
// hold the four altitude-detail nibbles `get_alt` leaves behind for the same
// four corners. get_brighting walks it, so the helper is spelled once here and
// expanded at every site - the image computes the address inline at each of
// them, and a plain .cpp function would cost a `call` the image does not make.
static __forceinline char *brighting_tile(void *owned, int row, int col_half) {
    return (char *)owned + ((MapLongitudeBounds >> 1) * row + col_half) * 0xC;
}

// Fixed-slot bindings carried from 00470550.cpp

// ORIGINAL: 0x00470550 ?get_brighting@MapWin@@QAEHHHH@Z 0x00470550-0x00470776 FILE
// HOMED 2026-09-04 from the 0x00470550 artifact. 16/202 in the tree. The
// four absolute table constants 0x7F6680/0x7F66B1/0x7F66E2/0x7F6713 are
// spelled as ONE binding, CornerLightTable, plus the stride 0x31 - the image's
// four displacements are that base plus corner*0x31, and the scorer discounts
// relocations so the single base reproduces the `[reg+reg+disp]` shape.
// TRIED: chained-OR bounds check vs nested ifs, and caching `this` in a named local before the owned_==0 check - none change when the compiler spills `this` to [ebp-4] relative to the row/col bound-check jumps (MISMATCH #6 'push' vs 'mov'); not chased further. Switch-table index math (v1..v4 nibble packing, per-case table*7 stride) is a faithful transcription.
// working copy - scaffold materialised by --work
// size      550 bytes
// prototype int (__thiscall ?get_brighting@MapWin@@QAEHHHH@Z)(MapWin* this, int, int, int)
// callers   1   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0046FE70


int MapWin::get_brighting(int x, int y, int corner) {
    int row = y;
    int col = x;
    if (row < 0 || row >= MapLatitudeBounds || col < 0 || col >= MapLongitudeBounds) {
        return 0;
    }
    if (owned_ == 0) {
        return -0x80;
    }
    int colHalf = col >> 1;
    char *tile = brighting_tile(owned_, row, colHalf);
    unsigned int v0 = (unsigned char)tile[corner];
    char *tileByteAddr = tile + corner;
    if (v0 != 0xff) {
        return (int)v0 - 0x80;
    }
    unsigned char terrain = *(unsigned char *)map_loc(col, row);
    if ((terrain & 0xe0) < 0x60) {
        *tileByteAddr = (char)0x80;
        return 0;
    }
    get_alt(col, row, 1);
    int v1;
    if (owned_ == 0) {
        v1 = 0;
    } else {
        char *t = brighting_tile(owned_, row, colHalf);
        v1 = (unsigned char)t[10] & 0xf;
    }
    get_alt(col, row, 2);
    int v2;
    if (owned_ == 0) {
        v2 = 0;
    } else {
        char *t = brighting_tile(owned_, row, colHalf);
        v2 = ((unsigned char)t[10] >> 4) & 0xf;
    }
    get_alt(col, row, 3);
    int v3;
    if (owned_ == 0) {
        v3 = 0;
    } else {
        char *t = brighting_tile(owned_, row, colHalf);
        v3 = (unsigned char)t[11] & 0xf;
    }
    get_alt(col, row, 4);
    int v4;
    if (owned_ == 0) {
        v4 = 0;
    } else {
        char *t = brighting_tile(owned_, row, colHalf);
        v4 = (unsigned char)t[11] >> 4;
    }
    int result;
    switch (corner) {
    case 0:
        result = CornerLightTable[v1 * 7 + v2];
        break;
    case 1:
        result = CornerLightTable[0x31 + v2 * 7 + v3];
        break;
    case 2:
        result = CornerLightTable[0x62 + v3 * 7 + v4];
        break;
    case 3:
        result = CornerLightTable[0x93 + v4 * 7 + v1];
        break;
    default:
        result = (int)v0;
        break;
    }
    char *finalTile = brighting_tile(owned_, row, colHalf);
    finalTile[corner] = (char)(result + 0x80);
    return result;
}

// Fixed-slot bindings carried from 00470790.cpp
// ORIGINAL: 0x00470790 ?get_point_light@MapWin@@QAEHHHH@Z 0x00470790-0x00470917 FILE
// HOMED 2026-09-04 from the 0x00470790 artifact. 22/139 under /O2 /GX.
// The scaffold's seven fixed globals all have names in this file now:
// g_00685684 = AltDirectionTable, g_00685A24/g_00685B60/g_007F6750 =
// AltLightBit1/AltLightBit2/AltLightBit200, g_00685EA0/g_00685EE0 = the two
// CornerOffset tables map.h defines, g_0094988C = MapIsFlat (map.h), and the
// tile read is `*map_tiles`/`*MapRowStride`. Two spellings measured:
// TRIED: `map_loc(a1, a2)->bit2` for the tile read - 6/139, and the image's
//            `mov eax,[0x68faf0]` came out relocation-masked to `[0]`; the
//            explicit `*map_tiles` + `*MapRowStride` form reads the same two
//            globals in the image's order and scored 9/139.
// LEVER: the neighbour walk as an up-counting `for (j = 0; j < 4; ++j)` over
//            `CornerOffsetX[a3-1]` / `CornerOffsetY[a3-1]` with the corner
//            arguments `(j+1)&3` and `(j+2)&3`, instead of the do/while that
//            carries a down-counting 0xfffffffe counter and its `check` temp
//            - 9/139 -> 22/139, and the frame drops two slots.
// TRIED: none confirmed byte-exact; MISMATCH #4 'mov' vs 'push' - the do/while neighbour-sum loop's register pressure does not reproduce the original's ebp-frame local layout.
// size      391 bytes
// prototype int (__thiscall ?get_point_light@MapWin@@QAEHHHH@Z)(MapWin* this, int, int, int)
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00470550

int MapWin::get_point_light(int x, int y, int corner) {
    int total = 0;
    int localLight = 0;

    unsigned int rec =
        *reinterpret_cast<const unsigned int *>(reinterpret_cast<const uint8_t *>(*map_tiles) +
            (*MapRowStride * y + (x >> 1)) * 0x2c + 0xc);
    int shade = static_cast<int>(rec) >> 0x18;

    if ((rec & 1) == 0) {
        if ((rec & 2) == 0) {
            if ((rec & 0x200) != 0 && shade >= 0) {
                unsigned char idx = AltDirectionTable[corner + shade * 5];
                localLight = AltLightBit200[idx];
            }
        } else if (shade >= 0) {
            unsigned char idx = AltDirectionTable[corner + shade * 5];
            localLight = AltLightBit2[idx];
        }
    } else if (shade >= 0) {
        unsigned char idx = AltDirectionTable[corner + shade * 5];
        localLight = AltLightBit1[idx];
    }

    if (corner == 0) {
        for (int i = 0; i < 4; ++i) {
            total += get_brighting(x, y, i);
        }
        return (total >> 2) + localLight;
    }

    const int (*corner_x)[4] = CornerOffsetX + corner - 1;
    const int (*corner_y)[4] = CornerOffsetY + corner - 1;
    for (int j = 0; j < 4; ++j) {
        int px = x + (*corner_x)[j];
        if ((MapIsFlat & 1) == 0) {
            if (px < 0) {
                px += MapLongitudeBounds;
            } else if (px >= MapLongitudeBounds) {
                px -= MapLongitudeBounds;
            }
        }
        int py = y + (*corner_y)[j];
        total += get_brighting(px, py, (j + 1) & 3);

        px = x + (*corner_x)[j];
        if ((MapIsFlat & 1) == 0) {
            if (px < 0) {
                px += MapLongitudeBounds;
            } else if (px >= MapLongitudeBounds) {
                px -= MapLongitudeBounds;
            }
        }
        total += get_brighting(px, py, (j + 2) & 3);
    }

    return (total >> 3) + localLight;
}

// Fixed-slot bindings carried from 00462980.cpp

// ORIGINAL: 0x00462980 ?calculate_dimensions@MapWin@@QAEXXZ 0x00462980-0x00462EFA FILE
// size      1402 bytes
// prototype void (__thiscall ?calculate_dimensions@MapWin@@QAEXXZ)(MapWin* this)
// callers   3   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00618F40
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00462980
// measured tier  MISMATCH
// divergence     1
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00462980/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?calculate_dimensions@MapWin@@QAEXXZ  at 0x00462980  (1402 bytes)
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


void MapWin::calculate_dimensions() {
    field_1DDAC_ = 0;
    field_1DDB0_ = 0;

    int32_t v;
    if (*MapWinCaptionScale > 0) {
        v = (*MapWinCaptionScale * 50) / *MapWinCaptionDivisor;
    } else {
        v = ((field_1DD98_ + 0x10) * 50) / 16;
    }
    v = (v + 1) / 4;
    field_1DDC8_ = v;
    if (v < 1) {
        field_1DDC8_ = 1;
    }
    int32_t T = field_1DDC8_;
    int32_t c2 = T * 2;
    field_1DDC4_ = c2;
    int32_t c4a = c2 * 2;
    int32_t c8 = c2 * 4;
    field_1DDBC_ = c4a;
    field_1DDC0_ = c2;
    field_1DDB4_ = c8;
    field_1DDB8_ = T * 4;

    char *const entered = entered_receiver(this);
    int32_t w = *reinterpret_cast<int32_t *>(entered + 0x4c4);
    int32_t h = *reinterpret_cast<int32_t *>(entered + 0x4c8);

    field_1DDCC_ = (w + c8 - 1) / c8;
    field_1DDD0_ = ((T * 4 - h) + c2 - 1) / (T * 4);
    field_1DDD4_ = (w + T * 4 + c8 - 1) / c8;
    field_1DDD8_ = ((T * 4 - h) + c2 - 1) / (T * 4);

    if (this == reinterpret_cast<MapWin *>(&ConsoleGlobal)) {
        int32_t a = field_1DDA0_;
        int32_t b = field_1DD9C_;
        MapWinPrimaryCorner[0] = b;
        MapWinPrimaryCorner[1] = a;
        MapWinPrimaryCorner[2] = b;
        MapWinPrimaryCorner[3] = a;
        int32_t f73 = field_1DDCC_;
        int32_t f74 = field_1DDD0_;
        int32_t e1 = (b - f73) + 4;
        int32_t e2 = (f73 + b) - 4;
        int32_t e3 = (a - f74) + 4;
        int32_t e4 = (a + f74) - 6;
        MapWinPrimaryCorner[0] = e1;
        MapWinPrimaryCorner[2] = e2;
        MapWinPrimaryCorner[1] = e3;
        MapWinPrimaryCorner[3] = e4;
    }

    int32_t halfW = MapLongitudeBounds / 2;
    if (field_1DDCC_ > halfW) {
        field_1DDCC_ = halfW;
        int32_t tileProduct = field_1DDCC_ * field_1DDB4_;
        field_1DDD4_ = halfW + 1;
        field_1DDAC_ = (w - tileProduct) / 2;
    }
    int32_t halfH = MapLatitudeBounds / 2;
    if (field_1DDD0_ > halfH) {
        field_1DDD0_ = halfH;
        int32_t tileProduct2 = field_1DDD0_ * field_1DDB8_;
        field_1DDD8_ = halfH + 1;
        field_1DDB0_ = -((h + tileProduct2) / 2);
    }

    int32_t halfW2 = MapLongitudeBounds / 2;
    field_1DDCC_ = (field_1DDCC_ < halfW2) ? field_1DDCC_ : halfW2;
    int32_t halfW3 = MapLongitudeBounds / 2;
    field_1DDD4_ = (field_1DDD4_ < halfW3) ? field_1DDD4_ : halfW3;

    int32_t f74_ = field_1DDD0_;
    int32_t aVal = field_1DDA0_;
    int32_t panYinit = aVal - f74_;
    field_1DDA8_ = panYinit;
    int32_t altY = MapLatitudeBounds + (2 - f74_) * 2;
    int32_t panY;
    if (panYinit < -2 || altY < -2) {
        panY = -2;
    } else if (altY < panYinit) {
        panY = altY;
    } else {
        panY = panYinit;
    }
    field_1DDA8_ = panY;

    int32_t f73_ = field_1DDCC_;
    int32_t bVal = field_1DD9C_;
    int32_t panXinit = bVal - f73_;
    uint8_t wrapFlag = *reinterpret_cast<uint8_t *>(&MapIsFlat);
    int32_t panX;
    if ((wrapFlag & 1) == 0) {
        int32_t screenW = MapLongitudeBounds;
        if (panXinit < 0) {
            panX = panXinit + screenW;
        } else if (panXinit >= screenW) {
            panX = panXinit - screenW;
        } else {
            panX = panXinit;
        }
    } else {
        panX = panXinit;
    }
    field_1DDA4_ = panX;

    if ((wrapFlag & 1) != 0) {
        int32_t altX = MapLongitudeBounds + (2 - f73_) * 2;
        if (panX < -4 || altX < -4) {
            panX = -4;
        } else if (altX < panX) {
            panX = altX;
        }
        field_1DDA4_ = panX;
    }

    field_1DDA4_ = ((field_1DDA4_ ^ field_1DDA8_) & 1) ^ field_1DDA4_;

    int32_t fogW;
    if ((field_1DD70_ & 2) == 0 || field_1DD98_ < -10) {
        fogW = 0;
    } else if (*MapWinCaptionScale < 1) {
        fogW = ((field_1DD98_ * 3 + 0x30) * 4) / 16;
    } else {
        fogW = (*MapWinCaptionScale * 12) / *MapWinCaptionDivisor;
    }
    field_1DDDC_ = fogW;

    int32_t fogH;
    if ((field_1DD70_ & 2) == 0 || field_1DD98_ < -10) {
        fogH = 0;
    } else if (*MapWinCaptionScale < 1) {
        fogH = (field_1DD98_ * 4 + 0x40) / 16;
    } else {
        fogH = (*MapWinCaptionScale * 4) / *MapWinCaptionDivisor;
    }
    field_1DDE0_ = fogH;

    field_1DD6C_ = 0x19;
    if ((GameMorePreferences & 0x10000) != 0) {
        field_1DD6C_ = 8;
        field_1DDDC_ = fogW / 3;
        field_1DDE0_ = fogH / 3;
    }

    if (field_1DD6C_ != field_1EEC4_) {
        field_1EEC4_ = field_1DD6C_;
        if ((int)(intptr_t)owned_ != 0) {
            int32_t count = (int32_t)MapArea;
            char *base = reinterpret_cast<char *>(owned_);
            for (int32_t ia = 0; ia < count; ++ia) {
                for (int32_t ja = 0; ja < 5; ++ja) {
                    base[ia * 0xc + ja + 4] = static_cast<char>(0xff);
                }
            }
        }
    }

    if (field_1DD98_ != field_1EEC0_) {
        if ((int)(intptr_t)owned_ != 0) {
            int32_t count = (int32_t)MapArea;
            char *base = reinterpret_cast<char *>(owned_);
            for (int32_t ib = 0; ib < count; ++ib) {
                for (int32_t jb = 0; jb < 4; ++jb) {
                    base[ib * 0xc + jb] = static_cast<char>(0xff);
                }
            }
            count = (int32_t)MapArea;
            base = reinterpret_cast<char *>(owned_);
            for (int32_t ic = 0; ic < count; ++ic) {
                for (int32_t jc = 0; jc < 5; ++jc) {
                    base[ic * 0xc + jc + 4] = static_cast<char>(0xff);
                }
            }
        }
        field_1EEC0_ = field_1DD98_;

        int32_t fs1 = (field_1DDB8_ * 5) / 16;
        if (fs1 > 0xc) {
            fs1 = 0xc;
        }
        if (field_1EEF0_ != fs1) {
            field_1EEF0_ = fs1;
            font_a_.init(reinterpret_cast<char *>(*DefaultFontFace), fs1, 0);
        }
        int32_t fs2 = (field_1DDB8_ * 5) / 16;
        if (fs2 > 0xc) {
            fs2 = 0xc;
        }
        if (field_1EF48_ != fs2) {
            field_1EF48_ = fs2;
            font_c_.init(reinterpret_cast<char *>(*DefaultFontFace), fs2, 2);
        }
        int32_t fs3 = (field_1DDB8_ * 2) / 5;
        if (fs3 > 0xc) {
            fs3 = 0xc;
        }
        if (field_1EF1C_ != fs3) {
            field_1EF1C_ = fs3;
            font_b_.init(reinterpret_cast<char *>(*DefaultFontFace), fs3, 1);
        }
    }

#undef I32
}

// ORIGINAL: 0x00463040 ?pixel_to_tile@MapWin@@QAEHHHPAHPAH@Z 0x00463040-0x004632CC FILE
// symbol    ?pixel_to_tile@MapWin@@QAEHHHPAH0@Z
//           VC6 merges the two identical `int *` argument types into the
//           back-reference `0`, so the definition emits `PAH0` where the
//           catalogue spells `PAHPAH`. Same two pointers, one mangling.
// TRIED: MISMATCH #10 'lea' vs 'cmp' - transcribed the Ghidra pseudocode almost verbatim (including the vbtable-indirected bounds check via *(int*)(*this+4)); the entry sequence loading eax/ecx from [edi] and [eax+4] diverges around instruction 10, not chased further.
// working copy - scaffold materialised by --work
// size      652 bytes
// prototype int (__thiscall ?pixel_to_tile@MapWin@@QAEHHHPAHPAH@Z)(MapWin* this, int, int, int*, int*)
// callers   7   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0046FE70

int MapWin::pixel_to_tile(int x_pixel, int y_pixel, int *out_tile_x,
                          int *out_tile_y) {
    // The GraphicWin virtual base's client width (0x4C4) and height (0x4C8).
    char *const entered = entered_receiver(this);
    if ((x_pixel < *reinterpret_cast<int *>(entered + 0x4C4)) &&
        (y_pixel < -*reinterpret_cast<int *>(entered + 0x4C8))) {
        x_pixel = x_pixel - field_1DDAC_;
        y_pixel = y_pixel - field_1DDB0_;
        if ((-1 < x_pixel) && (-1 < y_pixel)) {
            int iVar1;
            int iVar2;
            int iVar3;
            int iVar4;
            int iVar5;
            unsigned int uVar6;
            int iVar7;
            int iVar8;
            bool bVar9;
            int local_8;

            iVar5 = field_1DDA4_;
            iVar3 = iVar5 + 1 + (x_pixel / field_1DDB4_) * 2;
            uVar6 = MapIsFlat & 1;
            if (uVar6 == 0) {
                if (iVar3 < 0) {
                    iVar3 = MapLongitudeBounds + iVar3;
                } else if (MapLongitudeBounds <= iVar3) {
                    iVar3 = iVar3 - MapLongitudeBounds;
                }
            }
            iVar1 = field_1DDBC_;
            iVar2 = iVar5 + 1 + ((x_pixel - iVar1) / field_1DDB4_) * 2;
            if (uVar6 == 0) {
                if (iVar2 < 0) {
                    iVar2 = iVar2 + MapLongitudeBounds;
                } else if (MapLongitudeBounds <= iVar2) {
                    iVar2 = iVar2 - MapLongitudeBounds;
                }
            }
            bVar9 = iVar2 != iVar3;
            if (bVar9) {
                local_8 = iVar3 - 1;
            } else {
                iVar3 = iVar3 + 1;
                local_8 = iVar3;
            }
            bVar9 = !bVar9;
            iVar3 = iVar3 - iVar5;
            iVar5 = iVar3 - 1;
            if (uVar6 == 0) {
                if (iVar5 < 0) {
                    iVar5 = iVar5 + MapLongitudeBounds;
                } else if (MapLongitudeBounds <= iVar5) {
                    iVar5 = iVar5 - MapLongitudeBounds;
                }
            }
            if (uVar6 == 0) {
                if (iVar3 < 0) {
                    iVar3 = iVar3 + MapLongitudeBounds;
                } else if (MapLongitudeBounds <= iVar3) {
                    iVar3 = iVar3 - MapLongitudeBounds;
                }
            }
            iVar2 = field_1DDA8_ + (y_pixel / field_1DDB8_) * 2;
            do {
                iVar8 = iVar2;
                bVar9 = (bVar9 == false);
                if (bVar9) {
                    iVar2 = this->get_alt(local_8, iVar8, 2);
                    iVar2 = ((iVar8 - field_1DDA8_) - 1) * field_1DDC0_ - iVar2;
                    iVar4 = this->get_alt(local_8, iVar8, 3);
                    iVar4 = (iVar8 - field_1DDA8_) * field_1DDC0_ - iVar4;
                    local_8 = local_8 + 1;
                } else {
                    iVar2 = this->get_alt(local_8, iVar8, 1);
                    iVar2 = (iVar8 - field_1DDA8_) * field_1DDC0_ - iVar2;
                    iVar4 = this->get_alt(local_8, iVar8, 2);
                    iVar4 = ((iVar8 - field_1DDA8_) - 1) * field_1DDC0_ - iVar4;
                    local_8 = local_8 - 1;
                }
                iVar4 = (iVar4 - iVar2) * (x_pixel - iVar1 * iVar5);
                iVar7 = (y_pixel - iVar2) * (iVar1 * iVar3 - iVar1 * iVar5);
                iVar2 = iVar8 + 1;
            } while (iVar4 - iVar7 == 0 || iVar4 < iVar7);
            iVar8 = iVar8 - 1;
            if ((MapIsFlat & 1) == 0) {
                if (local_8 < 0) {
                    local_8 = local_8 + MapLongitudeBounds;
                } else if (MapLongitudeBounds <= local_8) {
                    local_8 = local_8 - MapLongitudeBounds;
                }
            }
            *out_tile_x = local_8;
            *out_tile_y = iVar8;
            if ((-1 < iVar8) && (iVar8 < MapLatitudeBounds) && (-1 < *out_tile_x) && (*out_tile_x < MapLongitudeBounds)) {
                return 0;
            }
        }
    }
    return 1;
}

// Fixed-slot bindings carried from 0046ec10.cpp

typedef short (__stdcall *KeyStateFn)(int);

class Slot17Shim { public:
    virtual void slot00(); virtual void slot01(); virtual void slot02();
    virtual void slot03(); virtual void slot04(); virtual void slot05();
    virtual void slot06(); virtual void slot07(); virtual void slot08();
    virtual void slot09(); virtual void slot10(); virtual void slot11();
    virtual void slot12(); virtual void slot13(); virtual void slot14();
    virtual void slot15(); virtual void slot16();
    virtual void slot17(int, int);  // <-- used, offset 0x44
};

// ORIGINAL: 0x0046EC10 ?on_right_down@MapWin@@QAEXHH@Z 0x0046EC10-0x0046ED21 FILE
// size      273 bytes
// prototype void (__thiscall ?on_right_down@MapWin@@QAEXHH@Z)(MapWin* this, int, int)
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004B4A20
// indirect  0x0046EC76 0x0046EC82 0x0046EC91 0x0046EC9D 0x0046ED0C 0x0046ED18
// PRESERVED UNIT - measured MNEMONIC_ONLY.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0046EC10
// measured tier  MNEMONIC_ONLY
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0046ec10/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_right_down@MapWin@@QAEXHH@Z  at 0x0046EC10  (273 bytes)
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


void MapWin::on_right_down(int x_coord, int y_coord) {
    if (*MapWinInputEnabled == 0) {
        return;
    }

    if (this->field_1DD80_ != 0) {
        ConsoleStatusWin.set_title(0);
    }

    this->field_1DD7C_ = 0;
    this->field_1DD80_ = 0;
    this->field_1DD8C_ = 0;
    this->field_1DD84_ = 0;
    this->field_1DE0C_ = -1;
    this->field_1DD88_ = 0;

    if (*KeyStatePollSuppressed != 0) {
        return;
    }

    KeyStateFn get_key_state = *reinterpret_cast<KeyStateFn *>(IAT_GetAsyncKeyState);

    uint8_t shift_down = 0;
    shift_down = static_cast<uint8_t>(static_cast<uint16_t>(get_key_state(0x10)) >> 8);
    if (shift_down) {
        uint8_t ctrl_down = 0;
        ctrl_down = static_cast<uint8_t>(static_cast<uint16_t>(get_key_state(0x11)) >> 8);
        if (ctrl_down) {
            goto check_flag;
        }
    }

    if (!((*reinterpret_cast<KeyStateFn *>(IAT_GetKeyState))(0x91) & 1)) {
        goto abort;
    }

    {
        uint8_t ctrl_down2 = 0;
        ctrl_down2 = static_cast<uint8_t>(static_cast<uint16_t>(get_key_state(0x11)) >> 8);
        if (ctrl_down2) {
            goto abort;
        }
    }

check_flag:
    if (GameState & 0x40) {
        this->field_1DD8C_ = 2;
        this->field_1DD94_ = -1;
        this->field_1DD90_ = -1;

        char *const entered = entered_receiver(this);

        if (WinPointerOwner3 == 0) {
            WinPointerOwner4 = 0;
            WinPointerOwner3 = reinterpret_cast<Win *>(
                *reinterpret_cast<int32_t *>(entered) + entered);
        }

        {
            int32_t offset =
                *reinterpret_cast<int32_t *>(entered + MapWinEnteredOffset);
            reinterpret_cast<Slot17Shim *>(entered + offset)->slot17(x_coord, y_coord);
        }
        return;
    }

abort:
    get_key_state(0x11);
}

class VCallI { public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();
    virtual void slot003();
    virtual void slot004();
    virtual void slot005();
    virtual void slot006();
    virtual void slot007();
    virtual void slot008();
    virtual void slot009();
    virtual void slot010();
    virtual void slot011();
    virtual void slot012();
    virtual void slot013();
    virtual void slot014();
    virtual void slot015();
    virtual void slot016();
    virtual void slot017();
    virtual void slot018();
    virtual void slot019();
    virtual void slot020();
    virtual void slot021();
    virtual void slot022();
    virtual void slot023();
    virtual void slot024();
    virtual void slot025();
    virtual void slot026();
    virtual void slot027();
    virtual void slot028();
    virtual void slot029();
    virtual void slot030();
    virtual void slot031();
    virtual void slot032();
    virtual void slot033();
    virtual void slot034();
    virtual void slot035();
    virtual void slot036();
    virtual void slot037();
    virtual void slot038();
    virtual void slot039();
    virtual void slot040();
    virtual void slot041();
    virtual void slot042();
    virtual void slot043();
    virtual void slot044(int);  // <-- used, takes the region id
};

// ORIGINAL: 0x0046F700 ?on_nc_left_down@MapWin@@QAEXHH@Z 0x0046F700-0x0046F880 FILE
// size      384 bytes
// prototype void (__thiscall ?on_nc_left_down@MapWin@@QAEXHH@Z)(MapWin* this, int, int)
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005FA7E0
// indirect  0x0046F784 0x0046F7D3 0x0046F822 0x0046F871
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0046F700
// measured tier  MISMATCH
// divergence     11
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0046f700/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_nc_left_down@MapWin@@QAEXHH@Z  at 0x0046F700  (384 bytes)
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


void MapWin::on_nc_left_down(int x_coord, int y_coord) {
    RECT rect = {0, 0xe, 0x11, 0x1f};

    // `entered` is the GraphicWin virtual base the image hands this
    // handler: front + 0x21A6C. Every offset below is relative to it.
    char *const entered = entered_receiver(this);

    int32_t t1 = *reinterpret_cast<int32_t *>(
        *reinterpret_cast<int32_t *>(entered - 0x21A6C) + 4);
    int32_t fA = *reinterpret_cast<int32_t *>(entered + t1 - 0x21918);
    int32_t fB = *reinterpret_cast<int32_t *>(entered + t1 - 0x21920);
    int32_t diff = fA - fB;
    diff -= 0x5b;
    rect.left = diff;
    rect.right = diff + 0x11;
    if (in_box(x_coord, y_coord, &rect)) {
        reinterpret_cast<VCallI *>(
            entered + *reinterpret_cast<int32_t *>(*reinterpret_cast<int32_t *>(entered - 0x21A6C) + 4) -
            0x21a6c)
            ->slot044(3);
        return;
    }

    rect.left = rect.left + 0x15;
    rect.right = rect.right + 0x15;
    if (in_box(x_coord, y_coord, &rect)) {
        reinterpret_cast<VCallI *>(
            entered + *reinterpret_cast<int32_t *>(*reinterpret_cast<int32_t *>(entered - 0x21A6C) + 4) -
            0x21a6c)
            ->slot044(2);
        return;
    }

    rect.right = rect.right + 0x15;
    rect.left = rect.left + 0x15;
    if (in_box(x_coord, y_coord, &rect)) {
        reinterpret_cast<VCallI *>(
            entered + *reinterpret_cast<int32_t *>(*reinterpret_cast<int32_t *>(entered - 0x21A6C) + 4) -
            0x21a6c)
            ->slot044(1);
        return;
    }

    rect.left = rect.left + 0x15;
    rect.right = rect.right + 0x15;
    if (in_box(x_coord, y_coord, &rect)) {
        reinterpret_cast<VCallI *>(
            entered + *reinterpret_cast<int32_t *>(*reinterpret_cast<int32_t *>(entered - 0x21A6C) + 4) -
            0x21a6c)
            ->slot044(0);
    }
}

// ORIGINAL: 0x0046FD90 ?clear_terrain@MapWin@@QAEXXZ 0x0046FD90-0x0046FDFE FILE
// size      110 bytes
// prototype void (__thiscall ?clear_terrain@MapWin@@QAEXXZ)(MapWin* this)
// callers   4   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0046FD90
// measured tier  MISMATCH
// divergence     0
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0046fd90/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?clear_terrain@MapWin@@QAEXXZ  at 0x0046FD90  (110 bytes)
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

void MapWin::clear_terrain() {
    unsigned char *field4 = *reinterpret_cast<unsigned char **>(owned_);
    if (field4 != 0) {
        if (0 < (int32_t)MapArea) {
            int count = 0;
            int base = 0;
            do {
                int i = 0;
                do {
                    int idx = base + i;
                    ++i;
                    field4[idx] = 0xFF;
                } while (i < 4);
                ++count;
                base += 0xC;
            } while (count < (int32_t)MapArea);
        }
        field4 = *reinterpret_cast<unsigned char **>(owned_);
        if (field4 != 0 && 0 < (int32_t)MapArea) {
            int count = 0;
            int base = 0;
            do {
                int i = 0;
                do {
                    int idx = base + i;
                    ++i;
                    field4[idx + 4] = 0xFF;
                } while (i < 5);
                ++count;
                base += 0xC;
            } while (count < (int32_t)MapArea);
        }
    }
}

