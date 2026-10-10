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
// ORIGINAL: 0x004B3F20 ??_GCheckButton@@UAEPAXI@Z 0x004B3F20-0x004B3F41 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical deleting-thunk source src/deleting_thunks.cpp; original full spans retained: 0x004B3F20-0x004B3F41", "Pinned class constructor ??0CheckButton@@QAE@XZ at 0x00633750 stores original class/view table VA 0x66dc14 at 0x0063375A (mov dword ptr [esi], 0x66dc14); the current genuine owner vftable ??_7CheckButton@@6BWin@@@ selects generated ??_GCheckButton@@UAEPAXI@Z in slot 0 (+0x0) through weak entry ??_ECheckButton@@UAEPAXI@Z (mode 2).", "Pinned ??_GCheckButton@@UAEPAXI@Z at 0x004B3F20 calls the source-mapped ??1CheckButton@@QAE@XZ at 0x004B3EC0, then tests byte ptr [ebp + 8], 1; je 0x4b3f3a bypasses scalar delete when bit 0 is clear, while bit 0 set calls 0x0064557F; it save original this; call destructor; delete only when flags bit 0 is set; return saved this with ret 4.", "The real class lifetime is owned by src/jackal/checkbutton.cpp; its compiler-emitted deleting symbol and vftable slot carry this original identity. Hand-written adapter bodies with live adjustor callers remain until those callers are independently qualified and transferred. The linked weak/COMDAT copy is not used to choose source ownership.", "the source writer preserves the historical mapping's exactness state (historical exactness obligation retained); current compiler extent is not asserted equal to the original span"], "reason": "Bind the original CheckButton scalar deleting-destructor identity to the actual compiler-emitted class deleting destructor in its genuine owner translation unit. The original constructor's pinned vptr store and the current owner's physical table slot identify this class/view; no callback/thunk body is hand-authored here."}
// symbol    ??_GCheckButton@@UAEPAXI@Z
// kind      game
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
// Direct static storage addresses: no accessor call or extra dynamic initializer.
Sprite *const CheckButtonDefaultA24 =
    reinterpret_cast<Sprite *>(g_CHECKBOX_SPRITE_1.storage_.bytes);
Sprite *const CheckButtonDefaultA28 =
    reinterpret_cast<Sprite *>(g_CHECKBOX_SPRITE_2.storage_.bytes);

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
    checked_ = 0;
    toggle_argument_ = 0;
    click_callback_ = 0;
    double_click_callback_ = 0;
    checked_sprite_ = CheckButtonDefaultA28;
    unchecked_sprite_ = CheckButtonDefaultA24;
}

/*
Purpose: Reset CheckButton's toggle and callback state, restore its two
         borrowed default sprites, and close the GraphicWin base.
// ORIGINAL: 0x006337A0 ?close@CheckButton@@QAEXXZ 0x006337A0-0x006337D6 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence":["source:src/recovered/units/006337a0.cpp","source:src/jackal/checkbutton.cpp:CheckButton::CheckButton","source:src/jackal/checkbutton.h:CheckButton"],"reason":"Use the real CheckButton members and source-owned default sprite pointers instead of the seed's raw offsets and executable data addresses."}
*/
void CheckButton::close() {
    checked_ = 0;
    toggle_argument_ = 0;
    click_callback_ = nullptr;
    double_click_callback_ = nullptr;
    checked_sprite_ = CheckButtonDefaultA28;
    unchecked_sprite_ = CheckButtonDefaultA24;
    GraphicWin::close();
}

/*
Purpose: Tear down CheckButton and its GraphicWin base through ordinary C++
         destruction, preserving the compiler's exception-cleanup path.
// ORIGINAL: 0x004B3EC0 ??1CheckButton@@QAE@XZ 0x004B3EC0-0x004B3F18;0x00659880-0x00659892 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence":["source:src/recovered/units/004b3ec0.cpp","original:0x004B3EC0-0x004B3F18","source:src/jackal/checkbutton.h:CheckButton"],"reason":"Let C++ emit the base teardown and exception cleanup instead of patching vtable addresses and manually invoking a base destructor."}
// symbol    ??1CheckButton@@UAE@XZ
*/
CheckButton::~CheckButton() {
    close();
}

/*
Purpose: Copy the button's surface into its parent and draw the selected
         checked or unchecked sprite onto that surface.
// ORIGINAL: 0x00633850 ?on_redraw@CheckButton@@QAEXXZ 0x00633850-0x00633912 FILE
// RECOVERY_BINDING: v1 {"evidence":["source:src/unrecovered/00633850.cpp","original:0x0066DC40","source:src/jackal/checkbutton.h:CheckButton","source:src/buffer.h:Buffer"],"reason":"Use the confirmed virtual override, inherited GraphicWin/Buffer hierarchy, typed parent and sprite members rather than seed vtable arithmetic."}
// symbol    ?on_redraw@CheckButton@@UAEXXZ
*/
void CheckButton::on_redraw() {
    if (win_parent_ == nullptr) {
        return;
    }

    GraphicWin *const parent = static_cast<GraphicWin *>(win_parent_);
    Buffer *const destination = static_cast<Buffer *>(parent);
    Buffer *const source = static_cast<Buffer *>(this);
    destination->copy(source,
                      client_rect_.left - rect2_.left,
                      client_rect_.top - rect2_.top,
                      0, 0,
                      dib_.bmiHeader.biWidth,
                      -dib_.bmiHeader.biHeight);

    Sprite *const sprite = checked_ != 0 ? checked_sprite_ : unchecked_sprite_;
    sprite->draw(source, sprite->transparent_index(), 0, 0);
}

/*
Purpose: Invoke the registered double-click callback with the stored toggle
         argument; the click coordinates are not consumed by this handler.
// ORIGINAL: 0x00633980 ?on_left_double_click@CheckButton@@QAEXHH@Z 0x00633980-0x00633997 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence":["source:src/recovered/00633980.cpp","original:0x0066DC84","source:src/jackal/checkbutton.h:CheckButton"],"reason":"Use the confirmed virtual override and typed callback/state members; the guarded recorder banks the freshly exact owner source before the duplicate seed is retired."}
// symbol    ?on_left_double_click@CheckButton@@UAEXHH@Z
*/
void CheckButton::on_left_double_click(int, int) {
    if (double_click_callback_ != nullptr) {
        double_click_callback_(toggle_argument_);
    }
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
    const int width = checked_sprite_->width();
    const int height = unchecked_sprite_->height();
    const int result = GraphicWin::init(x, y, width, height, 0, 0x1020,
                                        parent, 0, 0);
    if (result != 0) {
        return result;
    }

    toggle_argument_ = toggle_argument;
    show(0);
    return 0;
}

// A click toggles the stored state, repaints around the callback, and sends
// the button event through the parent's established Win virtual slot.
// ORIGINAL: 0x00633920 ?on_left_click@CheckButton@@QAEXHH@Z 0x00633920-0x0063397F BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["partition class:CheckButton at master 9db3d883", "git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/recovered/units/00633920.cpp original signature, spans, call order and member accesses", "src/win.h Win::redraw and Win::on_button_clicked virtual slots", "git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/recovered/units/00633920.cpp"], "reason": "The existing seed identity is implemented by the typed CheckButton member. It preserves the seed's parent guard, toggle, redraw, callback, parent notification and final redraw sequence through current Win APIs. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
// symbol    ?on_left_click@CheckButton@@UAEXHH@Z
// kind      game
void CheckButton::on_left_click(int, int) {
    if (win_parent_ == 0) {
        return;
    }

    checked_ = checked_ == 0;
    redraw();
    if (click_callback_ != 0) {
        click_callback_(toggle_argument_);
    }
    win_parent_->on_button_clicked(toggle_argument_);
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
