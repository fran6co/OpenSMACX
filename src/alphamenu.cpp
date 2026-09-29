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
#include "alphamenu.h"
#include "buffer.h"

// The compiler destroys sprites_[2..0], sprite1_, buffer_, then the Menu
// base in the order observed at 0x0051DC10. Keep this as C++ ownership,
// not a manual walk over the object bytes.
// ORIGINAL: 0x0051DC10 ??1AlphaMenu@@QAE@XZ 0x0051DC10-0x0051DC86;0x0065D220-0x0065D24E
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/0051dc10.cpp"], "reason": "C++ AlphaMenu owns Menu, Buffer and Sprite members whose reverse lifetime order follows the original destructor and exception-unwind trace."}
// symbol    ??1AlphaMenu@@UAE@XZ
AlphaMenu::~AlphaMenu() {}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns 0.
// ORIGINAL: 0x00403610 ?requested_height@AlphaMenu@@QAEHXZ 0x00403610-0x00403613 BYTE_EXACT
// size      3 bytes
// prototype int (__thiscall ?requested_height@AlphaMenu@@QAEHXZ)(AlphaMenu* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0
Status: Complete
*/
int AlphaMenu::requested_height() {
    return 0;
}

// ORIGINAL: 0x00403620 ?on_adjust_pulldown_pos@AlphaMenu@@QAEXPAHPAH@Z 0x00403620-0x00403677
// RECOVERY_BINDING: v1 {"evidence": ["Pinned image 0x00403620 reads Menu field_A24_ and SpotInternal[index].rect at stride 0x18; Win outer_rect_ is at 0x13c and caption_height_ at 0x118."], "reason": "Recover AlphaMenu pull-down placement through its inherited Menu, Spot and Win state."}
// symbol    ?on_adjust_pulldown_pos@AlphaMenu@@QAEXPAH0@Z
void AlphaMenu::on_adjust_pulldown_pos(int *x, int *y) {
    if (field_A24_ != -1) {
        RECT spot_rect;
        spot_.get_rect_at(field_A24_, &spot_rect);
        *x = outer_rect_.right;
        *y = spot_rect.top + caption_height_ + outer_rect_.top;
    }
}

// ORIGINAL: 0x00403B70 ?backfill_func@AlphaMenu@@QAAHPAUGraphicWin@@@Z 0x00403B70-0x00403B94 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned image 0x00403B70 calls Buffer::box_sprite at 0x005E2DD3 with the target Buffer::rect2_ at target+0x474 and style record 0x0078D960, which is InterfaceBoxStyles[27]."], "reason": "Recover AlphaMenu static backfill callback using the supplied GraphicWin and its typed Buffer subobject."}
// symbol    ?backfill_func@AlphaMenu@@SAHPAVGraphicWin@@@Z
int AlphaMenu::backfill_func(GraphicWin *target) {
    target->Buffer::box_sprite(target->buffer_working_rect(), &InterfaceBoxStyles[27]);
    return 1;
}


// The two vtable slots AlphaMenu overrides, stubbed at their image entry
// points - the bodies are unrecovered (0x00403830 and 0x00403340 sit in
// recovered/units/ as NOT_MATCHING artifacts). The slots stay true so the
// compiler-built vtable keeps the image's shape.
void AlphaMenu::hide() {}
void AlphaMenu::on_redraw() {}

// The image's on_mouse_move identity is Win slot 17: it tracks the
// highlighted Spot and releases capture when the pointer leaves the menu.
// Unlike the old seed scaffold, this uses Menu's actual Spot and Win's
// source-owned capture globals and virtual callbacks.
// ORIGINAL: 0x00403550 ?on_mouse_move@AlphaMenu@@QAEXHH@Z 0x00403550-0x0040360F
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/00403550.cpp"], "reason": "The original menu mouse body updates Menu spot selection and Win pointer capture through the inherited slot-17 callback, whose product spelling is on_mouse_feedback."}
// symbol    ?on_mouse_feedback@AlphaMenu@@UAEXHH@Z
void AlphaMenu::on_mouse_feedback(int x, int y) {
    if (field_A24_ != -1) {
        Menu::on_mouse_move(x, y);
    } else if (WinPointerOwner3 == 0) {
        WinPointerOwner3 = this;
        WinPointerOwner4 = 0;
    }

    int position = y;
    const int hit = spot_.check(x, y, &position, 0);
    if (hit == -1) {
        MenuSelection = -1;
        redraw();
        on_update();
        if (WinPointerOwner3 == this) {
            WinPointerOwner3 = 0;
            on_lose_mouse_capture();
        }
        if (WinPointerOwner4 == this) {
            WinPointerOwner4 = 0;
        }
    } else {
        MenuSelection = position;
        redraw();
        on_update();
    }
}
