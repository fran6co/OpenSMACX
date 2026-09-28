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
#include "checkbutton.h"
#include "checkbox.h"

// The two defaults the constructor reads once and copies into the object:
// pointers to the checkbox sprites the whole check family shares -
// g_CHECKBOX_SPRITE_1 / g_CHECKBOX_SPRITE_2 (checkbox.h), the instances the
// image holds at 0x009B8F60 / 0x009B8F90 and CheckBox::init_class populates
// from jackal.pcx. The image's slots at 0x00697F20 / 0x00697F24 hold these
// two addresses, and nothing but this constructor ever reads them.
//
// SPELLED AS BINDINGS, MEASURED: the image LOADS these two slots
// (`mov eax,[0x697f20]` then stores the copy) - taking &instance directly
// compiles one store-immediate instead of that load/store pair and the
// ctor drops from 17 instructions to 15, 5/17.
Sprite *const CheckButtonDefaultA24 = &g_CHECKBOX_SPRITE_1;
Sprite *const CheckButtonDefaultA28 = &g_CHECKBOX_SPRITE_2;

/*
Purpose: Construct the GraphicWin base, then install CheckButton's own
         vtables and the trailing toggle-state fields.
// ORIGINAL: 0x00633750 ??0CheckButton@@QAE@XZ 0x00633750-0x0063379D BYTE_EXACT
// symbol    ??0CheckButton@@QAE@XZ
// size      77 bytes
// prototype void (__thiscall ??0CheckButton@@QAE@XZ)(CheckButton* this)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4CF0
Return Value: n/a
Status: Complete
*/
CheckButton::CheckButton() {
    field_A14_ = 0;
    isToggled_ = 0;
    field_A1C_ = 0;
    field_A20_ = 0;
    field_A28_ = CheckButtonDefaultA28;
    field_A24_ = CheckButtonDefaultA24;
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x006339B0 ?close_class@CheckButton@@QAAXXZ 0x006339B0-0x006339B1 BYTE_EXACT
// symbol    ?close_class@CheckButton@@SAXXZ
// size      1 bytes
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void CheckButton::close_class() {
}


/*
Purpose: Unknown; the legacy implementation is a constant return that returns 0.
// ORIGINAL: 0x006339A0 ?init_class@CheckButton@@QAAHXZ 0x006339A0-0x006339A3 BYTE_EXACT
// symbol    ?init_class@CheckButton@@SAHXZ
// size      3 bytes
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0, always
Status: Complete
*/
int CheckButton::init_class() {
    return 0;
}

// CheckButton's parent is a GraphicWin because this control's init contract
// takes that exact type. GraphicWin::init sets up the base window and the
// image then hides the newly initialized control until its caller shows it.
// ORIGINAL: 0x006337E0 ?init@CheckButton@@QAEHHHPAUGraphicWin@@H@Z 0x006337E0-0x00633841
// RECOVERY_BINDING: v1 {"evidence":["partition class:CheckButton at master 9db3d883","git:9db3d883:src/unrecovered/006337e0.cpp original signature, spans, calls and member accesses","src/graphicwin.h GraphicWin::init signature","src/sprite.h Sprite::width/height accessors and +0x18/+0x1C member offsets"],"reason":"The existing seed identity is implemented by the typed CheckButton member. Sprite::width and Sprite::height preserve the seed's reads from the selected Sprite members at +0x18 and +0x1C; GraphicWin::init and Win::show express its base setup and final hide operation."}
// symbol    ?init@CheckButton@@QAEHHHPAVGraphicWin@@H@Z
int CheckButton::init(int x, int y, GraphicWin *parent, int toggle_argument) {
    if (parent == 0) {
        return 3;
    }

    // The seed reads the checked Sprite's iWidth_ at +0x18 and the
    // unchecked Sprite's iHeight_ at +0x1C.
    const int width = field_A28_->width();
    const int height = field_A24_->height();
    const int result = GraphicWin::init(x, y, width, height, 0, 0x1020,
                                        parent, 0, 0);
    if (result != 0) {
        return result;
    }

    isToggled_ = toggle_argument;
    show(0);
    return 0;
}

// A click toggles the stored state, repaints around the callback, and sends
// the button event through the parent's established Win virtual slot.
// ORIGINAL: 0x00633920 ?on_left_click@CheckButton@@QAEXHH@Z 0x00633920-0x0063397F
// RECOVERY_BINDING: v1 {"evidence":["partition class:CheckButton at master 9db3d883","src/recovered/units/00633920.cpp original signature, spans, call order and member accesses","src/win.h Win::redraw and Win::on_button_clicked virtual slots"],"reason":"The existing seed identity is implemented by the typed CheckButton member. It preserves the seed's parent guard, toggle, redraw, callback, parent notification and final redraw sequence through current Win APIs."}
void CheckButton::on_left_click(int, int) {
    if (win_parent_ == 0) {
        return;
    }

    field_A14_ = field_A14_ == 0;
    redraw();
    if (field_A1C_ != 0) {
        field_A1C_(isToggled_);
    }
    win_parent_->on_button_clicked(isToggled_);
    redraw();
}


/*
Purpose: Step the receiver back to the subobject ??_GCheckButton@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x004B3F80 ??_GCheckButton@@WEEE@AEPAXI@Z 0x004B3F80-0x004B3F8B BYTE_EXACT
// symbol    ??_ECheckButton@@WEEE@AEPAXI@Z
// CORRECTED from ??3CheckButton@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x004B3F20` into
//   ??_GCheckButton@@UAEPAXI@Z, which executes `ret 4`; no stack access
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
