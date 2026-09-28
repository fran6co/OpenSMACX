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

// PlanWin constructor-installed Win branch: capture4, resize13, mouse17, update63.
// Table start is observed in the original constructor; full extent remains unknown.
// RECOVERY_GENERATED: v1 vtable 0x0066D2AC ? ??_7PlanWin@@6BWin@@@ 0x0048BCD0
#include "original_seam.h"
#include "planwin.h"
#include "mapwin.h"
#include "base.h"
#include "spritebox.h"
#include "net_class.h"
#include "texture.h"
#include "imagebutton.h"
#include "temp.h"
#include "vector_teardown.h"
#include <cstring>

// The element dtors for PlanWin's shared MapWin-layout arrays - see
// mapwin.cpp's callback note.
static void __fastcall planwin_texture_store_element_dtor(void *self) {
    reinterpret_cast<TextureStore *>(self)->~TextureStore();
}
static void __fastcall planwin_image_button_element_dtor(void *self) {
    reinterpret_cast<ImageButton *>(self)->~ImageButton();
}


// The dword immediately ahead of MapWin's virtual base. MapWin used to
// declare it as a member; VC6 owns those four bytes as MapWin's vtordisp
// when MapWin is the most-derived object, but inside PlanWin the base is
// not most-derived and emits no vtordisp there, so PlanWin names the slot
// itself (field_21A68_ in planwin.h) to hold the layout. This reaches it
// the same way `virtual_base_of` reaches the base: through the parameter,
// so no body has to pun its own `this`.
static int32_t &pre_vbase_dword(void *self) {
    return *reinterpret_cast<int32_t *>(
        reinterpret_cast<uint8_t *>(self) + 0x21A68);
}


/*
Purpose: Build a plan window - attach its embedded GraphicWin, the MapWin
         base beneath it, and its own Buffer.
// ORIGINAL: 0x0048BCD0 ??0PlanWin@@QAE@H@Z 0x0048BCD0-0x0048BD7E;0x006581D0-0x00658205 BYTE_EXACT
// LEVER: store order, not declaration order - the image writes field_21A6C_
// before field_21A68_, so the two clears sit in that order in the body. 46/48
// SHAPE_EXACT 1.000-similar flipped to 48/48 on the swap alone; nothing else
// in the constructor moved.
// symbol    ??0PlanWin@@QAE@XZ
// size      227 bytes
// prototype void (__thiscall ??0PlanWin@@QAE@H@Z)(PlanWin* this, int)
// callers   1   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004626E0 0x005D4CF0 0x005D7210
Return Value: this

The compiler owns the construction machinery from the declarations in
planwin.h: the vbtable store, the MapWin base stage (the hidden flag
pushed 0, where the image's ??0PlanWin pushes it once), the virtual
GraphicWin base at +0x22050 under PlanWin's own flag, buffer_, and the
vbase-branch vtable pair plus vtordisp the old hand body spelled at
graphic_win. The body keeps the two scalar clears. One accepted divergence
by direction: the image builds the +0x22050 GraphicWin through the
`GraphicWin::construct()` method (0x005D4CF0 in its call list) - the same
hand-rolled vtable idiom this sweep exists to remove - where the real
constructor chain calls ??0GraphicWin.
*/
PlanWin::PlanWin() {
    field_21A6C_ = 0;
    field_21A68_ = 0;
}

/*
Purpose: Tear down a plan window's own buffer and the MapWin fields it holds
         directly, without a separate call into MapWin's own destructor.
// ORIGINAL: 0x0048BD80 ??1PlanWin@@QAE@XZ 0x0048BD80-0x0048BED6;0x00658210-0x006582DE
// symbol    ??1PlanWin@@UAE@XZ
// size      548 bytes
// prototype void (__thiscall ??1PlanWin@@QAE@XZ)(PlanWin* this)
// callers   2   call targets   5
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00462870 0x005D7410 0x00618EE0 0x006252B0 0x006456E4
Return Value: n/a

`this` IS NOT PlanWin's own front: `guarded_teardowns.cpp` already calls
`reinterpret_cast<PlanWin *>(TeardownObject00856DC0)->PlanWin::~PlanWin()`
with the address of the embedded GraphicWin virtual base
(front + 0x22050 - `TeardownObject00856DC0` minus `g_PLANWIN`'s own
0x00834D70 is exactly that), matching what this body's own arithmetic reads
throughout. That is the image's own convention, not a bug reproduced here -
`??__Fg_PLANWIN` calls this and then `GraphicWin::~GraphicWin()` on the same
address, in that order.

KNOWN DIVERGENCE: `PlanWin` publicly inherits `MapWin`, and `MapWin` now has
a genuine `~MapWin()` (needed so `guarded_teardowns.cpp`'s own already-matching
`->MapWin::~MapWin()` keeps compiling), so this destructor's epilogue gets an
EXTRA, compiler-inserted call into `MapWin::~MapWin()` that the image does not
make. The image tears down MapWin's own fields here directly, calling
`MapWin::clear` for the "this window is live" flag - PlanWin's destructor never
calls MapWin's own separate destructor at 0x00420F90.
// TRIED: public inheritance of MapWin with its real destructor; the
//        compiler-inserted ~MapWin epilogue call is the divergence - the
//        MapWin/PlanWin boundary needs class modelling, not a body edit
*/
PlanWin::~PlanWin() {
    char *const self = reinterpret_cast<char *>(this);
    reinterpret_cast<Buffer *>(self - 0x5e0)->~Buffer();

    char *const base = self - 0x5e4;
    reinterpret_cast<MapWin *>(base - 0x21a6c)->clear(0);

    
VectorDtorIterator(base - 0x2b18, 0xabc, 4,
                        planwin_image_button_element_dtor);
    reinterpret_cast<Font *>(base - 0x2b4c)->~Font();
    reinterpret_cast<Font *>(base - 0x2b78)->~Font();
    reinterpret_cast<Font *>(base - 0x2ba4)->~Font();
    reinterpret_cast<Buffer *>(base - 0x3134)->~Buffer();
    reinterpret_cast<Buffer *>(base - 0x36bc)->~Buffer();
    reinterpret_cast<Buffer *>(base - 0x3c44)->~Buffer();
    reinterpret_cast<TextureStore *>(base - 0x3f60)->~TextureStore();
    VectorDtorIterator(base - 0x210e0, 0x260, 0xc4,
                        planwin_texture_store_element_dtor);
    VectorDtorIterator(base - 0x21a60, 0x260, 4,
                        planwin_texture_store_element_dtor);
}

/*
Purpose: Clear the plan window's line count.
// ORIGINAL: 0x0048AF20 ?clear_lines@PlanWin@@QAEXXZ 0x0048AF20-0x0048AF2B BYTE_EXACT
// size      11 bytes
// prototype void (__thiscall ?clear_lines@PlanWin@@QAEXXZ)(PlanWin* this)
// callers   5   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/

// ORIGINAL: 0x0048AE40 ?add_line@PlanWin@@QAEXHHHHH@Z 0x0048AE40-0x0048AE90 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["The original accepts four signed coordinates and one color, storing the coordinates at +0x2200C + count*0x10 and the color at +0x21FFC + count*4.", "PlanWin's line_count_, line_colors_, and line_records_ declarations occupy those measured offsets and preserve the 0x50-byte derived tail."], "reason": "Home the coordinate-based plan-line insertion through typed PlanWin line storage."}
void PlanWin::add_line(int x1, int y1, int x2, int y2, int color) {
    if (line_count_ >= 4) {
        return;
    }

    PlanLineRecord &line = line_records_[line_count_];
    line.x1 = x1;
    line.y1 = y1;
    line.x2 = x2;
    line.y2 = y2;
    line_colors_[line_count_] = color;
    ++line_count_;
}

// ORIGINAL: 0x0048AE90 ?add_line@PlanWin@@QAEXHHH@Z 0x0048AE90-0x0048AF14 FILE
// RECOVERY_BINDING: v1 {"evidence": ["The original overload treats its first two arguments as Base indices and reads their source-owned signed x/y members before storing the four coordinates and color in the same PlanWin arrays.", "Base::x and Base::y are declared at offsets 0 and 2 with RECOVERY_MEMBER evidence; PlanWin's line arrays occupy +0x21FFC and +0x2200C."], "reason": "Home the base-to-base plan line using the existing Base and PlanWin members instead of address arithmetic."}
void PlanWin::add_line(int base_id1, int base_id2, int color) {
    if (line_count_ >= 4) {
        return;
    }

    const Base &first = Bases[base_id1];
    const Base &second = Bases[base_id2];
    PlanLineRecord &line = line_records_[line_count_];
    line.x1 = first.x;
    line.y1 = first.y;
    line.x2 = second.x;
    line.y2 = second.y;
    line_colors_[line_count_] = color;
    ++line_count_;
}

// The original body at 0x0048BB80 projects both tile endpoints through the
// MapWin receiver, offsets them by the map's current pixel center, then draws
// a four-on/four-off patterned line on the virtual GraphicWin Buffer. The
// nine-argument Buffer overload remains an explicit unfinished dependency.
// ORIGINAL: 0x0048BB80 ?draw_line@PlanWin@@QAEXHHHHH@Z 0x0048BB80-0x0048BC14
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/0048bb80.cpp"], "reason": "Home tile-projected patterned PlanWin line in its real Buffer base path"}
void PlanWin::draw_line(int tile_x1, int tile_y1, int tile_x2, int tile_y2,
                        int color) {
    int pixel_x1;
    int pixel_y1;
    int pixel_x2;
    int pixel_y2;
    tile_to_pixel(tile_x1, tile_y1, &pixel_x1, &pixel_y1);
    tile_to_pixel(tile_x2, tile_y2, &pixel_x2, &pixel_y2);

    pixel_x1 += field_1DDBC_;
    pixel_y1 += field_1DDC0_;
    pixel_x2 += field_1DDBC_;
    pixel_y2 += field_1DDC0_;

    GraphicWin *const graphics = static_cast<GraphicWin *>(this);
    Buffer *const drawing_buffer = static_cast<Buffer *>(graphics);
    drawing_buffer->line(pixel_x1, pixel_y1, pixel_x2, pixel_y2, 0, color,
                         4, 4, PlanLineCounter);
}

// ORIGINAL: 0x0048B3F0 ?back_redraw@PlanWin@@QAEXXZ 0x0048B3F0-0x0048B445 FILE
// RECOVERY_BINDING: v1 {"evidence": ["The body sets MapWin +0x1DD70, fills the GraphicWin virtual base, redraws the MapWin, then copies that base's Buffer into PlanWin's own Buffer using DIB width and top-down height.", "MapWin virtually inherits GraphicWin and PlanWin declares its own Buffer at +0x21A70. Typed base conversions follow the vbtable; Buffer::dib_ supplies the observed +0x4C4/+0x4C8 dimensions."], "reason": "Recover PlanWin's backing-image refresh through its real MapWin/GraphicWin hierarchy and typed buffers."}
void PlanWin::back_redraw() {
    field_1DD70_ = 0x84010002;

    // This conversion follows MapWin's virtual GraphicWin base (the image's
    // vbtable-selected +0x22050 subobject), then calls its inherited Buffer.
    GraphicWin *graphics = static_cast<GraphicWin *>(this);
    graphics->fill(0);
    draw_map(0);

    // Recompute the virtual-base path after draw_map just as the original
    // reloads the vbtable before accessing the source surface.
    graphics = static_cast<GraphicWin *>(this);
    Buffer *source = static_cast<Buffer *>(graphics);
    buffer_.copy(source, 0, 0, source->dib_.bmiHeader.biWidth,
                 -source->dib_.bmiHeader.biHeight);
}
void PlanWin::clear_lines() {
    line_count_ = 0;
}


/*
Purpose: Close the plan window by dropping its blink state and running the
         close it inherits from MapWin.
// ORIGINAL: 0x0048BC50 ?close@PlanWin@@QAEXXZ 0x0048BC50-0x0048BC5F BYTE_EXACT
// size      15 bytes
// prototype void (__thiscall ?close@PlanWin@@QAEXXZ)(PlanWin* this)
// callers   3   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete

The original clears the field and tail-jumps into MapWin::close with `this`
untouched, which is a plain base-class call: the MapWin subobject opens a
PlanWin at offset 0 - and it is spelled as one now. The claim that stood
here, that inheritance was impossible because MapWin held its virtual base
as a member, went with the member: VC6 places a virtual base where the
vbtable names it, so PlanWin reaches GraphicWin at its own 0x22050.
*/
void PlanWin::close() {
    // WRITTEN INLINE, not through `pre_vbase_dword`. The image emits
    // `mov [ecx + 0x21a68], 0` then TAIL-JUMPS to MapWin::close; routing the
    // store through a helper call blocks that tail call and costs the claim.
    // The pun of `this` is the price of the tail jump, and it is measured.
    field_21A68_ = 0;
    MapWin::close();
}


namespace {

// blink dispatches through the virtual base's own vtable rather than a C++
// virtual call, so no vtable this toolchain lays out can disagree with the
// original's. Slot 0x30 is read from whatever object is actually there.
typedef void (OriginalObject::*func_base_vtable_slot)();

static const size_t PlanWinBlinkSlot = 0x30;

// Both methods reach the virtual base the way the original does - through the
// vbtable rather than through the member - so they stay correct for any class
// that inherits this one and moves the base.
uint8_t *virtual_base_of(void *self) {
    const int32_t *const vbtable =
        *reinterpret_cast<const int32_t *const *>(self);
    return reinterpret_cast<uint8_t *>(self) + vbtable[1];
}

}  // namespace

/*
Purpose: Toggle the plan window's blink phase and redraw, when blinking is on.
// ORIGINAL: 0x0048BC20 ?blink@PlanWin@@QAEXXZ 0x0048BC20-0x0048BC4B
// size      43 bytes
// prototype void (__thiscall ?blink@PlanWin@@QAEXXZ)(PlanWin* this)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void PlanWin::blink() {
    if (!pre_vbase_dword(this)) {
        return;
    }
    field_21A6C_ = (field_21A6C_ == 0) ? 1 : 0;
    uint8_t *const base = virtual_base_of(this);
    void **const vtable = *reinterpret_cast<void ***>(base);
    (ORIGINAL(base)->*original_method<func_base_vtable_slot>(reinterpret_cast<unsigned long>(vtable[PlanWinBlinkSlot / sizeof(void *)])))();
}


/*
Purpose: Restore the window's backing image from the plan window's own buffer.
// ORIGINAL: 0x0048B3C0 ?UNK1@PlanWin@@QAEXXZ 0x0048B3C0-0x0048B3EF
// size      47 bytes
// prototype void (__thiscall ?UNK1@PlanWin@@QAEXXZ)(PlanWin* this)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D95B0
Return Value: n/a
Status: Complete

The height is passed negated. That is the original's, not a transcription
slip: it loads the buffer's field at 0x84 and runs `neg edx` before pushing.
*/
void PlanWin::UNK1() {
    uint8_t *const base = virtual_base_of(this);
    Buffer *const window_buffer = reinterpret_cast<Buffer *>(base + 0x444);
    int32_t width;
    int32_t height;
    std::memcpy(&width, base + 0x4C4, sizeof(width));
    std::memcpy(&height, base + 0x4C8, sizeof(height));
    window_buffer->copy(&buffer_, 0, 0, width,
                        -height);
}


// Compiler-generated Win-branch capture adjustor from the genuine
// MapWin override and virtual GraphicWin base; no handwritten thunk body.
/*
Purpose: Adjust the receiver from MapWin's thunk5 subobject to the one
         MapWin::on_lose_mouse_capture expects, then forward unchanged.
// ORIGINAL: 0x0048BF60 ?on_lose_mouse_capture@thunk5_MapWin@@QAEXXZ 0x0048BF60-0x0048BF6E FILE BYTE_EXACT
// symbol    ?on_lose_mouse_capture@MapWin@@$4PPPPPPPM@FOE@AEXXZ
// size      14 bytes
// prototype void (__thiscall ?on_lose_mouse_capture@thunk5_MapWin@@QAEXXZ)(MapWin* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/

// Compiler-generated Win slot13 adjustor for the genuine MapWin resize
// override. The virtual-base layout owns the receiver adjustment.
/*
Purpose: Adjust the receiver from MapWin's thunk5 subobject to the one
         MapWin::on_resize expects, then forward unchanged.
// ORIGINAL: 0x0048BF80 ?on_resize@thunk5_MapWin@@QAEXHH@Z 0x0048BF80-0x0048BF8E FILE BYTE_EXACT
// symbol    ?on_resize@MapWin@@$4PPPPPPPM@FOE@AEXHH@Z
// size      14 bytes
// prototype void (__thiscall ?on_resize@thunk5_MapWin@@QAEXHH@Z)(MapWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/

// Compiler-generated Win slot17 receiver adjustment for MapWin mouse feedback.
// The original catalogue name remains on the mapping below.
/*
Purpose: Adjust the receiver from MapWin's thunk5 subobject to the one
         MapWin::on_mouse_move expects, then forward unchanged.
// ORIGINAL: 0x0048BFA0 ?on_mouse_move@thunk5_MapWin@@QAEXHH@Z 0x0048BFA0-0x0048BFAE FILE BYTE_EXACT
// symbol    ?on_mouse_feedback@MapWin@@$4PPPPPPPM@FOE@AEXHH@Z
// size      14 bytes
// prototype void (__thiscall ?on_mouse_move@thunk5_MapWin@@QAEXHH@Z)(MapWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
