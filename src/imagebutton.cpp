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
#include "palette.h"
#include <cstring>
#include "imagebutton.h"

// Constructor 6252E8 installs this Win table; slot12 paints, and slots62/63
// retain GraphicWin redraw/update. Full table extent remains unestablished.
// RECOVERY_GENERATED: v1 vtable 0x00670A94 ? ??_7ImageButton@@6BWin@@@ 0x006252E0

/*
Purpose: Construct the button on its BaseButton base and clear its one field.
// ORIGINAL: 0x006252E0 ??0ImageButton@@QAE@XZ 0x006252E0-0x00625306 BYTE_EXACT
// LEVER: real-base the class derives from BaseButton now, so `call 0x00606F30` and both vtable stores are the compiler's. The old free-function redirect made all three by hand out of a `void *`.
// LEVER (2026-09-03, the /Ob0 dependency closed): BaseButton's constructor is
//        OUT OF LINE (basebutton.cpp) instead of the in-class `{ ; }`. In
//        class it was EXPANDED at this implicit base construction, and
//        because that expansion carries the two Time constructions VC6 added
//        the SEH unwind frame the image does not pay - 25 compiled
//        instructions, 0 of 9 agreeing, first divergence at instruction 0.
//        Out of line there is no body to fold, so the site is the image's own
//        9 instructions. Same lever as ~Buffer (buffer.h) and Heap's ctor.
// symbol    ??0ImageButton@@QAE@XZ
// size      38 bytes
// kind      game
Return Value: `this`
Status: Complete
*/
ImageButton::ImageButton() {
    // IN THE BODY, not the member-initialiser list. The image stores this
    // AFTER both vfptrs (0x006252F8, after 0x006252E8 and 0x006252EE), and
    // MSVC runs the list BEFORE the vfptr assignment - which is exactly how
    // ??0Win@@QAE@XZ's six stores had to move the other way.
    sprite_ = 0;
}

/*
Purpose: Restore both vtables and tail-call the base destructor.
// ORIGINAL: 0x00625310 ??1ImageButton@@QAE@XZ 0x00625310-0x00625325 BYTE_EXACT
// LEVER: real-base an empty body is correct here: with BaseButton a real base, VC6 emits both vtable restores and then TAIL-JUMPS to ??1BaseButton@@QAE@XZ, which is the image's `jmp 0x00607040`. Spelled against raw storage the destructor was trivial and compiled to a bare `ret`.
// symbol    ??1ImageButton@@UAE@XZ
// size      21 bytes
// kind      game
Return Value: n/a
Status: Complete
*/
ImageButton::~ImageButton() {
}

/*
Purpose: Step the receiver back to the subobject ??_GImageButton@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x006256F0 ??_GImageButton@@WEEE@AEPAXI@Z 0x006256F0-0x006256FB BYTE_EXACT
// symbol    ??_EImageButton@@WEEE@AEPAXI@Z
// CORRECTED from ??3ImageButton@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x006256D0` into
//   ??_GImageButton@@UAEPAXI@Z, which executes `ret 4`; no stack access
//   and the receiver stays in ECX. `WEEE@` re-demangles to
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

/*
Purpose: Initialise the button from a borrowed Sprite. Store it in sprite_,
         as the explicit-size overload does, then pass its displayed width
         and height at 0x18 and 0x1c to the base initialiser.
// ORIGINAL: 0x00625380 sub_625380 0x00625380-0x006253C6 BYTE_EXACT
// symbol    ?init@ImageButton@@QAEHPAVSprite@@PBDHHHPAVWin@@H@Z
// size      70 bytes
// prototype int (__thiscall sub_625380)(ImageButton* this, const int*, LPCSTR, int, int, int, Win*, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00607210
// LEVER: the receiver is an ImageButton, not a Texture and not BaseButton.
//   Three facts, and the last one is the one the shortlist missed: the store
//   lands at [this+0xAB8], which is `field_AB8_` - a member BaseButton does
//   not have, because sizeof(BaseButton) is exactly 0xAB8 and 0xAB8 is
//   therefore PAST the end of one; 0x00625380 sits immediately after
//   0x00625340, ?init@ImageButton@@QAEHHPADHHHHHPAUWin@@H@Z, in the image;
//   and the same slot is what the nine-argument overload stores. Spelled as
//   a member, the body needs no `this` pun and no ButtonInit stand-in class.
Return Value: whatever BaseButton::init returns
Status: Complete
*/
int ImageButton::init(Sprite *sprite, LPCSTR name, int id, int x, int y,
                      Win *parent, int style_flag) {
    if (sprite == 0) {
        return 3;
    }
    sprite_ = sprite;
    int height = sprite->height();
    int width = sprite->width();
    return BaseButton::init(name, id, x, y, width, height, parent,
                            style_flag);
}

/*
Purpose: Clear the field, then close as a BaseButton.

             mov dword [ecx+0xab8],0 / jmp ?close@BaseButton@@QAEXXZ

         A TAIL jump, so BaseButton::close's return value is this function's -
         but the mangled name says `X`, void, so nothing reads it. Declared
         void to match the name, and the value is discarded rather than
         quietly re-typed.

// ORIGINAL: 0x00625330 ?close@ImageButton@@QAEXXZ 0x00625330-0x0062533F BYTE_EXACT
// symbol    ?close@ImageButton@@UAEXXZ
// size      15 bytes
// prototype void (__thiscall ?close@ImageButton@@QAEXXZ)(ImageButton* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void ImageButton::close() {
    sprite_ = 0;
    BaseButton::close();
}

/*
Purpose: Keep the first argument, hand the other eight to BaseButton::init.

             mov eax,[esp+4] / mov edx,[esp+0x24] / mov [ecx+0xab8],eax
             ...eight interleaved reads and pushes... / call BaseButton::init
             ret 0x24

         The reads and pushes alternate because each `[esp+0x20]` is taken
         AFTER a push has moved ESP - so they walk backwards through the
         arguments and re-push them in order. Nine arguments in (`ret 0x24`),
         the first kept at 0xab8 and the remaining eight passed straight on.

// ORIGINAL: 0x00625340 ?init@ImageButton@@QAEHHPADHHHHHPAUWin@@H@Z 0x00625340-0x0062537A BYTE_EXACT
// symbol    ?init@ImageButton@@QAEHPAVSprite@@PBDHHHHHPAVWin@@H@Z
// size      58 bytes
// prototype int (__thiscall ?init@ImageButton@@QAEHHPADHHHHHPAUWin@@H@Z)(ImageButton* this, int, int8*, int, int, int, int, int, Win*, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00607210
Return Value: whatever BaseButton::init returns
Status: Complete
*/
int ImageButton::init(Sprite *sprite, LPCSTR name, int id, int x, int y,
                      int width, int height, Win *parent, int style_flag) {
    sprite_ = sprite;
    return BaseButton::init(name, id, x, y, width, height, parent, style_flag);
}

// Original 6253D0 paints through the inherited Buffer. Centers are defined
// only when a Sprite exists at the centering step; a later null-to-nonnull
// replacement during bevel drawing exposes uninitialized coordinates in the
// original too. No zero-center behavior is invented here.
namespace {
int image_button_paint_signed(uint32_t value) {
    return value <= 0x7fffffffU
        ? static_cast<int>(value)
        : -1 - static_cast<int>(~value);
}

int image_button_paint_add(int left, int right) {
    return image_button_paint_signed(static_cast<uint32_t>(left) +
                                     static_cast<uint32_t>(right));
}

int image_button_paint_sub(int left, int right) {
    return image_button_paint_signed(static_cast<uint32_t>(left) -
                                     static_cast<uint32_t>(right));
}

int image_button_paint_half(int value) {
    // SAR by one, including negative odd dimensions; not truncation to zero.
    return value >= 0 ? value / 2 : -1 - ((-1 - value) / 2);
}

void image_button_paint_inset(RECT *area) {
    area->left = image_button_paint_add(area->left, 1);
    area->right = image_button_paint_sub(area->right, 1);
    area->top = image_button_paint_add(area->top, 1);
    area->bottom = image_button_paint_sub(area->bottom, 1);
}
}

// ORIGINAL: 0x006253D0 ?on_redraw@ImageButton@@QAEXXZ 0x006253D0-0x006256CC
// RECOVERY_BINDING: v1 {"evidence": ["Original ctor6252E8 installs Win table670A94; its slot12 at670AC4 selects6253D0. GraphicWin::redraw5D5A70 dispatches that slot. The slot-12 virtual now carries the original on_redraw name shared with Dialog slot2 (journal events 134/138), so this override dispatches under the image's own spelling; pending MapWin/PlanWin receiver restoration is unchanged.", "Complete6253D0..6256CC uses HandleMain, WinCallbackWindow, PaletteActive, inherited Win parent, Buffer rect2 and BaseButton label/colors/state. Its second focus outline reloads A94 at625487; all box calls share one mutable local rectangle. Centering reads fresh rect2 after outlines; later draws reload sprite_ but retain those centers.", "Sprite displayed dimensions at18/1C determine centers while source pixel width10 determines horizontal scaling. Pressed state is selected once; bevel colors/thickness are live through box calls and text displacement reloads thickness after Sprite::draw. Signed wrapping arithmetic and arithmetic-half semantics are explicit.", "Preserve src/recovered/units/006253d0.cpp as seed evidence. The original null-at-centering then nonnull-at-draw condition exposes uninitialized coordinates; no zero-coordinate behavior is introduced. That pending behavior boundary, caption creation, remaining hierarchy and dependency runtime remain unproved."], "reason": "Recover the complete ImageButton painter as the genuine Win slot12 override, using existing borrowed Sprite and inherited button/Buffer state rather than raw offsets or image addresses."}
// symbol    ?on_redraw@ImageButton@@UAEXXZ
void ImageButton::on_redraw() {
    if (!HandleMain || !win_parent_) {
        return;
    }
    WinCallbackWindow = win_parent_;
    Buffer *const canvas = static_cast<Buffer *>(this);
    canvas->sync_to_palette(PaletteActive);
    RECT area = rect2_;
    GraphicWin::fill(image_button_paint_signed(color_));

    int layers = 0;
    const int first_outline = image_button_paint_signed(field_A94_);
    if (first_outline != -1) {
        canvas->box(&area, first_outline, first_outline);
        image_button_paint_inset(&area);
        if (is_dialog_focus()) {
            const int focused_outline = image_button_paint_signed(field_A94_);
            canvas->box(&area, focused_outline, focused_outline);
            image_button_paint_inset(&area);
            layers = 1;
        }
    }

    int center_x;
    int center_y;
    Sprite *const centering_sprite = sprite_;
    if (centering_sprite) {
        const int thickness = image_button_paint_signed(bevel_thickness_);
        center_x = image_button_paint_half(image_button_paint_sub(
            image_button_paint_sub(image_button_paint_sub(
                rect2_.right, centering_sprite->width()), thickness),
            rect2_.left));
        center_y = image_button_paint_half(image_button_paint_sub(
            image_button_paint_sub(image_button_paint_sub(
                rect2_.bottom, centering_sprite->height()), rect2_.top),
            thickness));
    }

    // The pressed branch is selected once, but colors and thickness remain
    // live across every box call. The first optional outline did not count
    // toward thickness; only the extra focus outline starts layers at one.
    const bool pressed = field_A9C_ != 0;
    while (layers < image_button_paint_signed(bevel_thickness_)) {
        if (!pressed) {
            canvas->box(&area, image_button_paint_signed(bevel_upper_),
                        image_button_paint_signed(bevel_lower_));
        } else {
            canvas->box(&area, image_button_paint_signed(bevel_lower_),
                        image_button_paint_signed(bevel_upper_));
        }
        image_button_paint_inset(&area);
        layers = image_button_paint_add(layers, 1);
    }

    Sprite *const drawn_sprite = sprite_;
    if (drawn_sprite) {
        // Logical pixel width (+10), not displayed width (+18). The image
        // only limits horizontal scale; no independent height fit occurs.
        int numerator;
        int denominator = drawn_sprite->source_width();
        int x;
        int y;
        if (!pressed) {
            numerator = image_button_paint_sub(area.right, area.left);
            x = center_x;
            y = center_y;
        } else {
            const int thickness = image_button_paint_signed(bevel_thickness_);
            numerator = image_button_paint_sub(
                image_button_paint_sub(area.right, thickness), area.left);
            x = image_button_paint_add(center_x, thickness);
            y = image_button_paint_add(center_y, thickness);
        }
        if (numerator >= denominator) {
            numerator = 1;
            denominator = 1;
        }
        drawn_sprite->draw(canvas, drawn_sprite->transparent_index(),
                           x, y, numerator, denominator);
    }

    if (pressed) {
        // Fresh read after drawing, not the thickness used for the sprite.
        const int thickness = image_button_paint_signed(bevel_thickness_);
        area.left = image_button_paint_add(area.left, thickness);
        area.right = image_button_paint_add(area.right, thickness);
        area.top = image_button_paint_add(area.top, thickness);
        area.bottom = image_button_paint_add(area.bottom, thickness);
    }
    LPSTR const text = name_;
    if (text) {
        canvas->write_cent_l(text, &area, static_cast<int>(strlen(text)));
    }
}
