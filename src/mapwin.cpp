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
#include "vector_teardown.h"
#include <cstring>
#include <stdlib.h>

// The element callbacks for MapWin's managed arrays (TextureStore and
// ImageButton runs), spelled for real against the recovered lifecycle
// bodies - the image passes 0x006252A0 / 0x006252B0 / 0x006252E0 /
// 0x00625310 at these sites.
static void __fastcall mapwin_texture_store_element_ctor(void *self) {
    reinterpret_cast<TextureStore *>(self)->TextureStore::TextureStore();
}
static void __fastcall mapwin_texture_store_element_dtor(void *self) {
    reinterpret_cast<TextureStore *>(self)->~TextureStore();
}
static void __fastcall mapwin_image_button_element_ctor(void *self) {
    reinterpret_cast<ImageButton *>(self)->ImageButton::ImageButton();
}
static void __fastcall mapwin_image_button_element_dtor(void *self) {
    reinterpret_cast<ImageButton *>(self)->~ImageButton();
}


// The vbtable MapWin stores at its own front when it is the one building
// the embedded GraphicWin, and the two hand-maintained "vtable" pointers
// `GraphicWin::construct`'s own idiom writes when GraphicWin is directly
// the most-derived object (see graphicwin.cpp) - MapWin has to repeat the
// latter pair on its own embedded GraphicWin, because nothing in this chain
// declares a single `virtual` and so VC6 never refreshes them on its own.
static void *const MonuWinVftable = reinterpret_cast<void *>(0x0066C870);
static void *const MapWinVftable = reinterpret_cast<void *>(0x0066A57C);
static void *const MapWinVirtualBaseVftable = reinterpret_cast<void *>(0x0066A574);


/*
Purpose: Build a map window - attach its embedded GraphicWin virtual base,
         then its own TextureStore/Buffer/Font/ImageButton members.
// ORIGINAL: 0x004626E0 ??0MapWin@@QAE@H@Z 0x004626E0-0x00462868;0x00655860-0x00655920
// TRIED: 4/92 - the image carries an SEH unwind frame here (`push -1
//            / push 0x655916 / mov eax,fs:[0] / ...`), same symptom
//            catalogued on FlatButton's constructor/destructor and
//            GraphicWin's destructor. Not attempted: MapWin::construct is
//            already the correct spelling per `most_derived_flag.py`'s
//            confirmed finding (a plain method, because a real
//            constructor would double-push the most-derived flag at every
//            `->MapWin::MapWin(1)` call site) - there is no constructor-
//            vs-method swap available the way FlatButton's destructor
//            had. Reproducing the frame here would need isolating what
//            about this specific `construct()` body triggers it despite
//            being a plain method, which this pass's budget does not
//            cover for an already-92-instruction function.
// symbol    ?construct@MapWin@@QAEXH@Z
// size      584 bytes
// prototype void (__thiscall ??0MapWin@@QAE@H@Z)(MapWin* this, int)
// callers   3   call targets   9
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005D4CF0 0x006252A0 0x005D7210 0x00618EA0 0x006456E4 0x006457C2
Return Value: this

MEASURED: not spelled as a real constructor - see the note in `mapwin.h`. A
plain method never gets VC6's own most-derived-flag treatment, so the single
`a1` here is read and branched on exactly as the image's `[ebp+8]` is, with
no second, compiler-inserted flag arriving alongside it. What is left, since
MapWin's own members are still opaque storage (see the class declaration),
is: the two array-of-TextureStore member ranges (via the CRT's own vector
constructor iterator, `VectorCtorIterator` - see `vector_teardown.h`), the
single TextureStore/Buffer/Buffer/Buffer/Font/Font/Font members, the
array-of-4 ImageButton range, and the manual vtable-pointer stores plus
vtordisp on the embedded GraphicWin.
*/
void MapWin::construct(int input) {
    char *const self = reinterpret_cast<char *>(this);

    if (input) {
        *reinterpret_cast<void **>(self) = MonuWinVftable;
        new (reinterpret_cast<GraphicWin *>(self + 0x21a6c)) GraphicWin();
    }

    
VectorCtorIterator(self + 0xc, 0x260, 4,
                        mapwin_texture_store_element_ctor,
                        mapwin_texture_store_element_dtor);
    VectorCtorIterator(self + 0x98c, 0x260, 0xc4,
                        mapwin_texture_store_element_ctor,
                        mapwin_texture_store_element_dtor);

    reinterpret_cast<TextureStore *>(self + 0x1db0c)->TextureStore::TextureStore();
    reinterpret_cast<Buffer *>(self + 0x1de28)->Buffer::Buffer();
    reinterpret_cast<Buffer *>(self + 0x1e3b0)->Buffer::Buffer();
    reinterpret_cast<Buffer *>(self + 0x1e938)->Buffer::Buffer();
    reinterpret_cast<Font *>(self + 0x1eec8)->Font::Font();
    reinterpret_cast<Font *>(self + 0x1eef4)->Font::Font();
    reinterpret_cast<Font *>(self + 0x1ef20)->Font::Font();

    VectorCtorIterator(self + 0x1ef54, 0xabc, 4,
                        mapwin_image_button_element_ctor,
                        mapwin_image_button_element_dtor);

    char *const graphic_win = self + 0x21a6c;
    *reinterpret_cast<void **>(graphic_win) = MapWinVftable;
    *reinterpret_cast<void **>(graphic_win + 0x444) = MapWinVirtualBaseVftable;
    *reinterpret_cast<int32_t *>(graphic_win - 4) = 0;

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
    const int32_t *const vbtable =
        *reinterpret_cast<const int32_t *const *>(this);
    reinterpret_cast<GraphicWin *>(
        reinterpret_cast<uint8_t *>(this) + vbtable[1])->close();
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
        const int32_t *const vbtable =
            *reinterpret_cast<const int32_t *const *>(base);
        reinterpret_cast<GraphicWin *>(
            reinterpret_cast<uint8_t *>(base) + vbtable[1])->close();
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
