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

func_set_date MainInterfaceOriginalSetDate = original_method<func_set_date>(0x0045BE80);
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
    (ORIGINAL(MainInterfaceGlobal)->*MainInterfaceOriginalSetDate)(MapWinMainCaption);
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

func_map_win_click MapWinClick = original_method<func_map_win_click>(0x0046D5D0);
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
    (ORIGINAL(base)->*MapWinClick)(a1, a2, 0);
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

MapWin **MapWinTable = reinterpret_cast<MapWin **>(0x007D3C3C);
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
Original Offset: 0046AF40
Return Value: n/a. EAX on return is path-dependent leftover (draw_radius's own
              residue, the zero flag read at 0x0046AF5F, or a stale value when
              the last slot is empty), not a computed result, so unlike
              GraphicWin::close there is no residue to preserve and the void
              return is faithful.
Status: Complete with temporary MapWin::draw_radius original dependency
*/
void __cdecl draw_tile(int x_coord, int y_coord, int draw_type) {
    for (size_t slot = 0; slot < MapWinTableSlots; ++slot) {
        // Re-read every iteration, as `mov ecx, dword ptr [esi]` at 0x0046AF51
        // does: a callee that rewrites the table is seen by later slots.
        MapWin *const window = MapWinTable[slot];
        // 0x0046AF53 test / 0x0046AF55 je - the null test comes first, so an
        // empty slot 0 draws nothing despite the exemption below.
        if (window == nullptr) {
            continue;
        }
        // 0x0046AF57 `cmp esi, 0x7d3c3c` / 0x0046AF5D je. The cursor only ever
        // walks forward from the table base, so comparing it against that base
        // is exactly "is this slot index 0?" - the primary map window is
        // exempt from the activity gate.
        if (slot != 0) {
            const uint32_t active = *reinterpret_cast<const volatile uint32_t *>(
                reinterpret_cast<const uint8_t *>(window) + MapWinActiveOffset);
            if (active == 0) {
                continue;
            }
        }
        // 0x0046AF71: __thiscall on the slot value still live in ECX, with the
        // pushes at 0x0046AF6C..0x0046AF70 giving the stack order
        // (x_coord, y_coord, 0, draw_type) - the pushes run right to left, so
        // the last one (`push eax`, [ebp+8]) is the first stack argument. The
        // literal 0 is this function's discriminator; see draw_tiles.
        (ORIGINAL(window)->*MapWinOriginalDrawRadius)(x_coord, y_coord, 0, draw_type);
    }
}

/*
Purpose: The radius-1 sibling of draw_tile - the identical 70-byte broadcast,
         but each live map window redraws the tile together with its
         surrounding ring rather than the single tile. The only difference in
         the originals is `push 1` at 0x0046B16D against `push 0` at
         0x0046AF6D; prologue, slot walk, null test, slot-0 exemption,
         activity gate, argument order, loop bound and epilogue all match
         instruction for instruction. Transcribed in full for the mutation
         coverage reason recorded on draw_tile.
Original Offset: 0046B140
Return Value: n/a; same path-dependent EAX leftover as draw_tile.
Status: Complete with temporary MapWin::draw_radius original dependency
*/
void __cdecl draw_tiles(int x_coord, int y_coord, int draw_type) {
    for (size_t slot = 0; slot < MapWinTableSlots; ++slot) {
        // 0x0046B151, mirroring 0x0046AF51.
        MapWin *const window = MapWinTable[slot];
        // 0x0046B153 test / 0x0046B155 je.
        if (window == nullptr) {
            continue;
        }
        // 0x0046B157 `cmp esi, 0x7d3c3c` / 0x0046B15D je.
        if (slot != 0) {
            const uint32_t active = *reinterpret_cast<const volatile uint32_t *>(
                reinterpret_cast<const uint8_t *>(window) + MapWinActiveOffset);
            if (active == 0) {
                continue;
            }
        }
        // 0x0046B171, with `push 1` at 0x0046B16D supplying the radius.
        (ORIGINAL(window)->*MapWinOriginalDrawRadius)(x_coord, y_coord, 1, draw_type);
    }
}

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
Original Offset: 00462960
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
Original Offset: 0046B1D0
Status: Complete
*/
void MapWin::on_redraw() {
    if (*reinterpret_cast<int *>(reinterpret_cast<char *>(this) - 0x21a68)) {
        reinterpret_cast<MapWin *>(reinterpret_cast<char *>(this) - 0x21a6c)->draw_map(1);
    }
}

/*
Original Offset: 0046FA00
Status: Complete
*/
void MapWin::on_resize(int a1, int a2) {
    char *self = reinterpret_cast<char *>(this);
    if ((*reinterpret_cast<unsigned int *>(self - 0x3cfc) & 0x80000000) == 0) {
        reinterpret_cast<MapWin *>(self - 0x21a6c)->do_image_buttons();
    }
}
