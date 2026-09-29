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
#include "map.h"
#include "base.h"
#include "spritebox.h"
#include "net_class.h"
#include "temp.h"
#include <cstring>

// Original CRT slot 0x00682570 selects the PlanWin initializer at 0x0048AE00.
// Its most-derived construction begins at 0x00834D70 and the registered
// teardown destroys that same complete object through its virtual base.
// RECOVERY_OBJECT: v2 0x00834D70 0x008577D4 ?PlanWindow@@3VPlanWin@@A 0x0048AE00
PlanWin PlanWindow;

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
Purpose: Destroy the complete PlanWin through C++ member and base lifetimes.
// ORIGINAL: 0x0048BD80 ??1PlanWin@@QAE@XZ 0x0048BD80-0x0048BED6;0x00658210-0x006582DE
// symbol    ??1PlanWin@@UAE@XZ
// size      548 bytes
// prototype void (__thiscall ??1PlanWin@@QAE@XZ)(PlanWin* this)
// callers   2   call targets   5
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00462870 0x005D7410 0x00618EE0 0x006252B0 0x006456E4
Return Value: n/a

The original exit wrapper enters through the virtual GraphicWin base at
0x00856DC0 and manually destroys the MapWin members before tearing down that
base. The old source copied those destructor calls into this body while also
declaring real C++ members and a real MapWin base. VC6 then emitted automatic
teardown as well, so the source would destroy those objects twice. The
complete PlanWindow above owns the lifetime; its generated destructor now
performs the member, MapWin and virtual GraphicWin stages once. This changes
the unclaimed body shape while preserving the original object relationship.
*/
PlanWin::~PlanWin() {
    // The declared Buffer, MapWin base and virtual GraphicWin base own their
    // destruction in reverse construction order. The old hand teardown
    // invoked those same destructors a second time before this epilogue.
}

// Initialize the plan display using its inherited MapWin and virtual
// GraphicWin state. The image does not read the caption argument.
// ORIGINAL: 0x0048BC60 ?init@PlanWin@@QAEXPAD@Z 0x0048BC60-0x0048BCC5
// RECOVERY_BINDING: v1 {"evidence": ["image:0x0048BC60-0x0048BCC5;git:a899c9ea:src/recovered/units/0048bc60.cpp", "source:src/planwin.h:PlanWin;source:src/mapwin.h:MapWin;source:src/buffer.h:Buffer"], "reason": "The PlanWin member initializes its inherited MapWin fields and its own Buffer from the virtual GraphicWin dimensions, preserving the observed object relationships."}
void PlanWin::init(char *) {
    field_21A68_ = 1;
    line_count_ = 0;
    field_1DD78_ = 1;
    MapWin::clear(1);
    MapWin::init(3, 0);
    field_1DD98_ = -14;

    GraphicWin *graphic = this;
    buffer_.init(graphic->dib_.bmiHeader.biWidth,
                 -graphic->dib_.bmiHeader.biHeight, 0, 0);
}

// Midpoint along the shorter horizontal interval on a wrapped map. The
// original uses the same three-stage calculation for two input pairs and
// then for their midpoints; on a flat map it does not wrap the result.
static int plan_horizontal_midpoint(int left, int right) {
    const int distance = abs(right - left);
    const int width = MapLongitudeBounds;
    const bool flat = (MapIsFlat & 1) != 0;
    int center;
    if (!flat && distance > width / 2) {
        center = (distance - width / 2) / 2
                 + (left > right ? left : right);
    } else {
        center = distance / 2 + (left < right ? left : right);
    }
    if (!flat) {
        if (center < 0) {
            center += width;
        } else if (center >= width) {
            center -= width;
        }
    }
    return center;
}

// Center the view between two horizontal pairs and their vertical spans.
// ORIGINAL: 0x0048B450 ?find_view@PlanWin@@QAEXHHHHHHHH@Z 0x0048B450-0x0048B629
// RECOVERY_BINDING: v1 {"evidence": ["image:0x0048B450-0x0048B629;git:a899c9ea:src/recovered/units/0048b450.cpp", "source:src/map.h:MapLongitudeBounds,MapIsFlat;source:src/mapwin.h:field_1DD9C_,field_1DDA0_"], "reason": "The eight-argument PlanWin member selects the horizontal center of two map-view pairs and their vertical midpoint using inherited MapWin view fields."}
void PlanWin::find_view(int x1, int y1, int x2, int y2,
                        int x3, int y3, int x4, int y4) {
    const int first_x = plan_horizontal_midpoint(x1, x2);
    const int second_x = plan_horizontal_midpoint(x3, x4);
    field_1DD9C_ = plan_horizontal_midpoint(first_x, second_x);
    field_1DDA0_ = ((y1 + y2) / 2 + (y3 + y4) / 2) / 2;
}

// A point's map tiles must fit the calculated viewport. Out-of-map samples
// do not constrain the zoom, as in the original center routines.
static bool plan_tiles_fit(const PlanWin &view, int center_x, int center_y,
                           int sample_count) {
    const bool flat = (MapIsFlat & 1) != 0;
    const int width = MapLongitudeBounds;
    const int height = MapLatitudeBounds;
    const int left = view.field_1DDA4_;
    const int top = view.field_1DDA8_;
    const int right = left + view.field_1DDCC_ + view.field_1DDD4_;
    const int bottom = top + view.field_1DDD0_ + view.field_1DDD8_;
    for (int index = 0; index < sample_count; ++index) {
        int x = center_x + RadiusOffsetX[index];
        if (!flat) {
            if (x < 0) {
                x += width;
            } else if (x >= width) {
                x -= width;
            }
        }
        const int y = center_y + RadiusOffsetY[index];
        if (x < 0 || x >= width || y < 0 || y >= height) {
            continue;
        }
        if (y < top || y >= bottom) {
            return false;
        }
        if (!flat) {
            if (x < left) {
                x += width;
            }
            if (x >= right) {
                x -= width;
            }
        }
        if (x < left || x >= right) {
            return false;
        }
    }
    return true;
}

// Zoom until both selected points and their neighboring tiles fit the view.
// ORIGINAL: 0x0048B630 ?opticenter@PlanWin@@QAEXHHHHH@Z 0x0048B630-0x0048B8D6
// RECOVERY_BINDING: v1 {"evidence": ["image:0x0048B630-0x0048B8D6;git:a899c9ea:src/recovered/units/0048b630.cpp", "source:src/map.h:RadiusOffsetX,RadiusOffsetY;source:src/mapwin.h:viewport fields"], "reason": "The PlanWin member varies the inherited MapWin zoom until the selected two points and their radius samples fit the calculated viewport."}
void PlanWin::opticenter(int x1, int y1, int x2, int y2, int force_redraw) {
    const uint32_t old_zoom = field_1DD98_;
    const uint32_t old_x = field_1DD9C_;
    const uint32_t old_y = field_1DDA0_;
    field_1DD9C_ = x1;
    field_1DDA0_ = y1;
    field_1DD98_ = static_cast<uint32_t>(-14);

    do {
        ++field_1DD98_;
        calculate_dimensions();
        if (!plan_tiles_fit(*this, x1, y1, 25) ||
            !plan_tiles_fit(*this, x2, y2, 25)) {
            --field_1DD98_;
            break;
        }
    } while (static_cast<int>(field_1DD98_) < -10);

    calculate_dimensions();
    if (force_redraw || old_zoom != field_1DD98_ ||
        old_x != field_1DD9C_ || old_y != field_1DDA0_) {
        back_redraw();
    }
}

// Zoom around the current center until one point's neighborhood no longer
// fits, then repaint the inherited GraphicWin surface if the view changed.
// ORIGINAL: 0x0048B8E0 ?monocenter@PlanWin@@QAEXHHH@Z 0x0048B8E0-0x0048BAA1
// RECOVERY_BINDING: v1 {"evidence": ["image:0x0048B8E0-0x0048BAA1;git:a899c9ea:src/recovered/units/0048b8e0.cpp", "source:src/planwin.h:PlanWin;source:src/map.h:RadiusOffsetX,RadiusOffsetY;source:src/buffer.h:Buffer"], "reason": "The PlanWin member varies inherited MapWin zoom until one point and its neighborhood fit, then redraws its real GraphicWin surface and owned Buffer."}
void PlanWin::monocenter(int x, int y, int force_redraw) {
    const uint32_t old_zoom = field_1DD98_;
    const uint32_t old_x = field_1DD9C_;
    const uint32_t old_y = field_1DDA0_;
    field_1DD98_ = static_cast<uint32_t>(-14);

    do {
        ++field_1DD98_;
        calculate_dimensions();
        if (!plan_tiles_fit(*this, x, y, 81)) {
            --field_1DD98_;
            break;
        }
    } while (static_cast<int>(field_1DD98_) <= -11);

    calculate_dimensions();
    if (force_redraw || old_zoom != field_1DD98_ ||
        old_x != field_1DD9C_ || old_y != field_1DDA0_) {
        field_1DD70_ = 0x84010002U;
        GraphicWin *graphic = this;
        graphic->fill(0);
        draw_map(0);
        static_cast<Buffer *>(graphic)->copy(
            &buffer_, 0, 0, graphic->dib_.bmiHeader.biWidth,
            -graphic->dib_.bmiHeader.biHeight);
    }
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
