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
#include <cstring>
#include <stdlib.h>

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

void __fastcall map_win_unk3_redirect(MapWin *self, void *) {
    self->UNK3();
}

void __fastcall map_win_do_image_buttons_redirect(MapWin *self, void *) {
    self->do_image_buttons();
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

void __fastcall map_win_main_caption_redirect(MapWin *self, void *) {
    self->main_caption();
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
    const int32_t *const vbtable =
        *reinterpret_cast<const int32_t *const *>(this);
    reinterpret_cast<GraphicWin *>(
        reinterpret_cast<uint8_t *>(this) + vbtable[1])->close();
}

void __fastcall map_win_close_redirect(MapWin *self, void *) {
    self->close();
}

func_map_win_click MapWinClick = original_method<func_map_win_click>(0x0046D5D0);

/*
Purpose: Report a left click on the map, but only when map input is enabled.
         Reached through the GraphicWin virtual base, so `this` points there
         and is adjusted back 0x21A6C to the MapWin before dispatching.
// ORIGINAL: 0x0046EBA0 ?on_left_click@MapWin@@QAEXHH@Z 0x0046EBA0-0x0046EBC5
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
    (ORIGINAL(base)->*MapWinClick)(a1, a2, 0);
}

/*
Purpose: Report a right click on the map, but only when map input is enabled.
         Same virtual-base adjustment as the left click.
// ORIGINAL: 0x0046EBE0 ?on_right_click@MapWin@@QAEXHH@Z 0x0046EBE0-0x0046EC05
// size      37 bytes
// prototype void (__thiscall ?on_right_click@MapWin@@QAEXHH@Z)(MapWin* this, int, int)
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0046D5D0
Return Value: n/a
Status: Complete
*/
void MapWin::on_right_click(int a1, int a2) {
    if (*MapWinInputEnabled == 0) {
        return;
    }
    MapWin *const base = reinterpret_cast<MapWin *>(
        reinterpret_cast<uint8_t *>(this) - 0x21A6C);
    (ORIGINAL(base)->*MapWinClick)(a1, a2, 1);
}

void __fastcall map_win_on_left_click_redirect(MapWin *self, void *, int a1, int a2) {
    self->on_left_click(a1, a2);
}

void __fastcall map_win_on_right_click_redirect(MapWin *self, void *, int a1, int a2) {
    self->on_right_click(a1, a2);
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

void __fastcall map_win_on_left_double_click_redirect(MapWin *self, void *, int a1, int a2) {
    self->on_left_double_click(a1, a2);
}

void __fastcall map_win_on_left_up_redirect(MapWin *self, void *, int a1, int a2) {
    self->on_left_up(a1, a2);
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

int __fastcall map_win_unk2_redirect(MapWin *self, void *) {
    return self->UNK2();
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
// ORIGINAL: 0x0046AF40 ?draw_tile@@YAXHHH@Z 0x0046AF40-0x0046AF86
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
// ORIGINAL: 0x0046B140 ?draw_tiles@@YAXHHH@Z 0x0046B140-0x0046B186
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
// ORIGINAL: 0x00462960 ?UNK1@MapWin@@QAEHXZ 0x00462960-0x0046296C
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
    return this == reinterpret_cast<MapWin *>(ConsoleGlobal) ? 1 : 0;
}

int __fastcall map_win_unk1_redirect(MapWin *self, void *) {
    return self->UNK1();
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
#pragma auto_inline(on)
