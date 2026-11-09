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
// ORIGINAL: 0x00456280 ??_GGamma@@UAEPAXI@Z 0x00456280-0x004562A1 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical deleting-thunk source src/deleting_thunks.cpp; original full spans retained: 0x00456280-0x004562A1", "Pinned class constructor ??0Gamma@@QAE@XZ at 0x005C8DA0 stores original class/view table VA 0x66c3d8 at 0x005C8E2E (mov dword ptr [esi], 0x66c3d8); the current genuine owner vftable ??_7Gamma@@6BWin@@@ selects generated ??_GGamma@@UAEPAXI@Z in slot 0 (+0x0) through weak entry ??_EGamma@@UAEPAXI@Z (mode 2).", "Pinned ??_GGamma@@UAEPAXI@Z at 0x00456280 calls the source-mapped ??1Gamma@@QAE@XZ at 0x00456110, then tests byte ptr [ebp + 8], 1; je 0x45629a bypasses scalar delete when bit 0 is clear, while bit 0 set calls 0x0064557F; it save original this; call destructor; delete only when flags bit 0 is set; return saved this with ret 4.", "The real class lifetime is owned by src/gamma.cpp; its compiler-emitted deleting symbol and vftable slot carry this original identity. Hand-written adapter bodies with live adjustor callers remain until those callers are independently qualified and transferred. The linked weak/COMDAT copy is not used to choose source ownership.", "the source writer preserves the historical mapping's exactness state (historical exactness obligation retained); current compiler extent is not asserted equal to the original span"], "reason": "Bind the original Gamma scalar deleting-destructor identity to the actual compiler-emitted class deleting destructor in its genuine owner translation unit. The original constructor's pinned vptr store and the current owner's physical table slot identify this class/view; no callback/thunk body is hand-authored here."}
// compile_context src/interface_assets.cpp
// symbol    ??_GGamma@@UAEPAXI@Z
// kind      game
#include "stdafx.h"
#include "gamma.h"
#include "worldwin.h"
#include "basepop.h"
#include "win.h"
#include <math.h>
#include <stddef.h>

// These pinned sites call the ordinary CRT pow entry with double arguments.
#ifdef _MSC_VER
#pragma function(pow)
#endif

// RECOVERY_LITERAL: v1 0x0066FA68 0x0066FA70 __real@8@4006ebfd70a3d70a4000 0x005C959C
namespace {
const double GammaPositionScale = 0.01;
// Original 5C955F reads this scalar as a qword. Bind only its observed start;
// the enclosing original allocation and typed initializer extent are unproved.
// RECOVERY_OBJECT: v1 0x0066FA78 _GammaComponentScale
const double GammaComponentScale = 1.0 / 235.0;
const float GammaSliderScale = 100.0f;
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x005C8E60 ?exec@Gamma@@QAEXPAUPalette@@MH@Z 0x005C8E60-0x005C9325
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/005c8e60.cpp records the control setup and modal call sequence", "src/gamma.h identifies five consecutive RECTs and the Palette/Scroll/EditBox/PushButton members", "Pinned disassembly 0x005C8F89 and 0x005C9119 shows the accept button uses the RECT at +0xA44, correcting the seed's +0xA34", "At 0x005C8ECC/0x005C8ED1 VC6 overwrites the former argument slots with WinScreenHeight/Width before GraphicWin::init reloads them at 0x005C903B; the call does not pass a Palette pointer as a dimension", "WinScreenWidth/Height and BasePopDefaultOkText/CancelText are existing source-bound globals", "Original format/font bytes at 0x00691E74 read Arial"], "reason": "Run Gamma's palette apply-only or modal editor flow using real controls, owned rectangles and their member APIs rather than raw receiver offsets and vtable arrays."}
// symbol    ?exec@Gamma@@QAENPAVPalette@@MH@Z
// Both original return paths leave a value in ST(0): 0x5C8E8F loads
// the initial float, and 0x5C9318 loads gamma_'s double after cleanup.
// PrefWin consumes that result at 0x491484. The seed's void signature
// discarded it; returning double preserves the modal value until the caller
// performs its own float store.
double Gamma::exec(Palette *target, float initial_gamma, int apply_only) {
    target_palette_ = target;
    initialGamma_ = initial_gamma;
    if (apply_only != 0) {
        adjust_pal(static_cast<double>(initial_gamma));
        return initial_gamma;
    }

    const int screen_height = WinScreenHeight;
    const int screen_width = WinScreenWidth;
    rect1_.left = 0;
    rect1_.top = 0;
    rect1_.right = 236;
    rect1_.bottom = 32;
    const int horizontal_offset =
        (screen_width - rect1_.right + rect1_.left) / 2;
    rect1_.left += horizontal_offset;
    rect1_.right += horizontal_offset;
    const int vertical_offset =
        (screen_height - rect1_.bottom + rect1_.top) / 2;
    rect1_.top += vertical_offset;
    rect1_.bottom += vertical_offset;

    rect2_ = rect1_;
    const int row_offset = rect2_.bottom - rect2_.top + 3;
    rect2_.top += row_offset;
    rect2_.bottom += row_offset;

    rect3_ = rect1_;
    const int scroll_offset = rect1_.bottom - rect1_.top + 3;
    rect3_.top += scroll_offset + 4;
    rect3_.bottom += scroll_offset - 4;

    rect4_ = rect3_;
    const int button_offset = rect4_.bottom - rect4_.top + 3;
    rect4_.top += button_offset;
    rect4_.bottom = rect4_.top + 20;
    rect4_.right = rect4_.left + (rect4_.right - rect4_.left + 3) / 2 - 3;

    rect5_ = rect4_;
    const int second_button_offset = rect5_.right - rect5_.left + 3;
    rect5_.left += second_button_offset;
    rect5_.right += second_button_offset;

    font_.init("Arial", 14, 1);
    GraphicWin::init(0, 0, screen_width, screen_height, 0, 0,
                     0, 0, 0);
    GraphicWin::fill(0xF8);

    scroll_.init(&rect3_, this, 0, 0);
    scroll_.set_border_color(0);
    scroll_.field_A64_ = 0xF8;
    scroll_.field_A48_ = 7;
    scroll_.set_button_color(7);
    scroll_.set_bevel_thickness(2);
    scroll_.set_bevel_upper(7);
    scroll_.set_bevel_lower(0);
    scroll_.paging_ = 10;
    scroll_.thumb_sprite_ = nullptr;
    scroll_.set_sprite_left(0, 0, 0);
    scroll_.set_sprite_right(0, 0, 0);
    scroll_.set_range(1, 400);
    scroll_.set_pos(static_cast<int>(initial_gamma * GammaSliderScale));
    scroll_.show(0);

    pushButton1_.init(BasePopDefaultOkText, -1,
                      rect4_.left, rect4_.top,
                      rect4_.right - rect4_.left,
                      rect4_.bottom - rect4_.top, this, 0);
    pushButton1_.Buffer::set_font(&font_, 0, 0, 0);
    pushButton1_.redraw();
    pushButton1_.set_text_color(0, -1, 1, 1);
    pushButton1_.color_ = 0xF8;
    pushButton1_.bevel_upper_ = 7;
    pushButton1_.bevel_lower_ = 0;
    pushButton1_.bevel_thickness_ = 2;
    pushButton1_.field_AC8_ = 0;
    pushButton1_.field_AEC_ = 0;
    pushButton1_.field_AC0_ = 0;
    pushButton1_.field_AE4_ = 0;
    pushButton1_.field_ACC_ = 0;
    pushButton1_.field_AF0_ = 0;
    pushButton1_.redraw();

    pushButton2_.init(BasePopDefaultCancelText, -2,
                      rect5_.left, rect5_.top,
                      rect5_.right - rect5_.left,
                      rect5_.bottom - rect5_.top, this, 0);
    pushButton2_.Buffer::set_font(&font_, 0, 0, 0);
    pushButton2_.redraw();
    pushButton2_.set_text_color(0, -1, 1, 1);
    pushButton2_.color_ = 0xF8;
    pushButton2_.bevel_upper_ = 7;
    pushButton2_.bevel_lower_ = 0;
    pushButton2_.bevel_thickness_ = 2;
    pushButton2_.field_AC8_ = 0;
    pushButton2_.field_AEC_ = 0;
    pushButton2_.field_AC0_ = 0;
    pushButton2_.field_AE4_ = 0;
    pushButton2_.field_ACC_ = 0;
    pushButton2_.field_AF0_ = 0;
    pushButton2_.redraw();

    show(1);
    do_all_draws();
    adjust_palette();
    redraw();
    set_modal(0, 0, 0);
    hide();
    do_all_draws();
    adjust_pal(gamma_);
    close();
    return gamma_;
}

// ORIGINAL: 0x005C9520 ?adjust_palette@Gamma@@QAEXXZ 0x005C9520-0x005C95E0 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Earlier source-owned body in src/stringstruct.cpp and retained original seed at git:ee30a69d", "Gamma's Scroll position occupies Gamma+0x191C and Palette entries begin at Gamma+0xA68", "Original ramp writes 236 RGB triples, reseeds and sets the Palette, then calls Win slot 12"], "reason": "Keep gamma-ramp behavior in its owning translation unit and operate on Palette entries and Scroll state rather than executable addresses or object offsets."}
// symbol    ?adjust_palette@Gamma@@QAEXXZ
void Gamma::adjust_palette() {
    gamma_ = static_cast<double>(scroll_.position()) * GammaPositionScale;
    unsigned char *green = reinterpret_cast<unsigned char *>(&palette_.entries_) +
        10 * sizeof(PALETTEENTRY) + offsetof(PALETTEENTRY, peGreen);
    int i = 0;
    do {
        double value = pow(static_cast<double>(i) * GammaComponentScale,
                           1.0 / gamma_);
        if (value < 0.0) value = 0.0;
        else if (value > 1.0) value = 1.0;
        const unsigned char shade = static_cast<unsigned char>(
            static_cast<int>(value * 235.99));
        green[-1] = shade;
        green[0] = shade;
        green[1] = shade;
        ++i;
        green += sizeof(PALETTEENTRY);
    } while (i < 236);
    palette_.reseed();
    palette_.set();
    on_redraw();
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// Historical seed observations (not fresh measurements or acceptance):
// TRIED: `RECT rect = *(RECT*)(self+0xa14);` (struct assignment through one base pointer, not 4 independent offset loads - the latter drops the original's `lea`+shared-`ecx` shape entirely) gets every field store byte-exact through the loop setup. The y2 = (bottom-top)+top redundant identity the ASM computes (sub ecx,eax; add ecx,eax) is always constant-folded to a plain `bottom` by every /O flag combination tried, whether written inline or through a named `height` temp - MISMATCH #19 is that fold, not a missed field.
// ORIGINAL: 0x005C9420 ?on_redraw@Gamma@@QAEXXZ 0x005C9420-0x005C951D BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/005c9420.cpp records the 236-line palette strip and the gamma label", "src/gamma.h owns rect1_, font_, gamma_ and the Buffer base at their measured offsets", "Original format string at 0x00691E7C is %f"], "reason": "Draw the dialog's colour ramp and current gamma through its actual GraphicWin and Buffer bases, overriding Win's slot 12."}
// Gamma overrides the existing Win redraw slot; its emitted method is virtual.
// symbol    ?on_redraw@Gamma@@UAEXXZ
// Guarded centered write, inlined: the image keeps the null test on the
// stack buffer before strlen.
static inline void gamma_write_cent(Gamma *win, char *text, RECT *rect) {
    if (text != 0) {
        win->Buffer::write_cent_l(text, rect, strlen(text));
    }
}

void Gamma::on_redraw() {
    RECT label_area = rect1_;
    int x = label_area.left;
    const int top = label_area.top;
    const int height = label_area.bottom - top;
    for (int color = 10; color < 246; ++color) {
        Buffer::line(x, top, x, top + height, color);
        ++x;
    }

    const int delta = label_area.top - label_area.bottom - 3;
    label_area.top += delta;
    label_area.bottom += delta;
    GraphicWin::fill(&label_area, 0xF8);
    Buffer::set_font(&font_, 0, 0, 0);
    Buffer::set_text_color(0, -1, 1, 1);

    char value_text[80];
    value_text[0] = 0;
    sprintf(value_text, "%f", gamma_);
    gamma_write_cent(this, value_text, &label_area);
    GraphicWin::soft_update();
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x005C9330 ?close@Gamma@@QAEXXZ 0x005C9330-0x005C938B BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/005c9330.cpp records the cleanup order", "src/gamma.h owns the Palette, Scroll, EditBox and two PushButton subobjects at the measured offsets", "PushButton::close is the virtual slot 90 selected by the original"], "reason": "Close Gamma's real owned controls and base window rather than dispatching through fabricated vtable arrays and raw offsets."}
// symbol    ?close@Gamma@@QAEXXZ
void Gamma::close() {
    target_palette_ = 0;
    palette_.close();
    scroll_.close();
    editBox_.close();
    pushButton1_.close();
    pushButton2_.close();
    GraphicWin::close();
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x005C95E0 ?adjust_pal@Gamma@@QAEXN@Z 0x005C95E0-0x005C97B1
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/005c95e0.cpp walks the target Palette from entry 10 through 245", "src/recovered/units/005c8e60.cpp stores the Palette* argument at Gamma+0xA64", "src/jackal/palette.h places PALETTEENTRY records at the start of Palette and exposes reseed/set"], "reason": "Apply gamma to the target palette's RGB entries as typed palette data, then publish its new seed and colours."}
// symbol    ?adjust_pal@Gamma@@QAEXN@Z
void Gamma::adjust_pal(double gamma) {
    if (target_palette_ == 0) return;

    for (int index = 0; index < 236; ++index) {
        double red = target_palette_->entries_[index + 10].peRed * GammaComponentScale;
        double green = target_palette_->entries_[index + 10].peGreen * GammaComponentScale;
        double blue = target_palette_->entries_[index + 10].peBlue * GammaComponentScale;
        red = pow(red, 1.0 / gamma);
        green = pow(green, 1.0 / gamma);
        blue = pow(blue, 1.0 / gamma);

        if (red < 0.0) red = 0.0;
        else if (red > 1.0) red = 1.0;
        if (green < 0.0) green = 0.0;
        else if (green > 1.0) green = 1.0;
        if (blue < 0.0) blue = 0.0;
        else if (blue > 1.0) blue = 1.0;

        const int red_byte = static_cast<int>(red * 235.99);
        const int green_byte = static_cast<int>(green * 235.99);
        const int blue_byte = static_cast<int>(blue * 235.99);
        target_palette_->entries_[index + 10].peRed = static_cast<BYTE>(red_byte);
        target_palette_->entries_[index + 10].peGreen = static_cast<BYTE>(green_byte);
        target_palette_->entries_[index + 10].peBlue = static_cast<BYTE>(blue_byte);
    }
    target_palette_->reseed();
    target_palette_->set();
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00456110 ??1Gamma@@QAE@XZ 0x00456110-0x00456275;0x00655190-0x00655240 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00456110.cpp shows a Gamma::close call followed by reverse subobject teardown", "src/gamma.h declares the real member order and GraphicWin base", "C++ destruction emits subobject and EH cleanup without writing historical vtable addresses", "Pinned PrefWin::on_button_clicked 0x491400 and load_palette 0x451A10 normal cleanup expand the Gamma close and reverse member/base lifetime at the actual Gamma receiver.", "Both pinned cold EH cleanups call the standalone destructor at 0x456110; keep that independent exceptional entry and all original spans/claims.", "The current Gamma header owns the actual typed members and GraphicWin base; Gamma::close remains the unchanged mapped body at 0x5C9330.", "Current physical production-recipe screening observed a complete matching destructor copy in both real consumers; src/interface_palette.cpp is a measured current compile context only, not an assertion about original TU ownership."], "reason": "Retain Gamma’s standalone destructor identity in its original owner while exposing the unchanged class lifetime to the measured real consumer; the normal consumer paths inline Gamma::close and member/base teardown, while the original cold EH path enters this emitted destructor. Current compile_context names a present emitting TU, not the original translation unit."}
// compile_context src/interface_assets.cpp
// body src/gamma.h
// symbol    ??1Gamma@@UAE@XZ

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x005C93C0 ?on_button_clicked@Gamma@@QAEXH@Z 0x005C93C0-0x005C9406 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/005c93c0.cpp separates button -1 scroll acceptance from button -2 initial-gamma restoration", "src/gamma.h identifies gamma_, initialGamma_ and the embedded Scroll", "Win slot 58 is release_modal, the original indirect target"], "reason": "Apply or cancel the gamma adjustment and close the modal dialog through its real virtual method."}
// symbol    ?on_button_clicked@Gamma@@UAEXH@Z
void Gamma::on_button_clicked(int button_id) {
    switch (button_id) {
    case -1:
        gamma_ = static_cast<double>(scroll_.position()) * GammaPositionScale;
        break;
    case -2:
        gamma_ = initialGamma_;
        break;
    }
    release_modal();
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns 1.
// ORIGINAL: 0x005C9410 ?on_key_click@Gamma@@QAEHHH@Z 0x005C9410-0x005C9418 BYTE_EXACT
// symbol    ?on_key_click@Gamma@@UAEHHH@Z
// size      8 bytes
// prototype int (__thiscall ?on_key_click@Gamma@@QAEHHH@Z)(Gamma* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 1, always
Status: Complete
*/
int Gamma::on_key_click(int, int) {
    return 1;
}



/*
// ORIGINAL: 0x005C9390 ?on_scrolled@Gamma@@QAEXHH@Z 0x005C9390-0x005C9398 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned original own table 0x0066C3D8, pointer word at +0xC0, identifies original entry 0x005C9390", "source:src/win.h inherited virtual interface; actual emitted method ?vslot_48@Gamma@@UAEXHH@Z in primary Win table ??_7Gamma@@6BWin@@@"], "reason": "The original own Win table binds this entry as virtual slot 48. Use genuine member dispatch, not an opaque redirect; retain the catalogued original spelling as historical evidence. The previous on_scrolled definition is the class slot-48 target; renaming connects the unchanged body to its real virtual slot."}
// symbol    ?on_scrolled@Gamma@@UAEXHH@Z
// size      8 bytes
// prototype void (__thiscall ?on_scrolled@Gamma@@QAEXHH@Z)(Gamma* this, int, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005C9520
Status: Complete
*/
void Gamma::on_scrolled(int code, int pos) {
    adjust_palette();
}


/*
Purpose: Construct the gamma-correction dialog.
// ORIGINAL: 0x005C8DA0 ??0Gamma@@QAE@XZ 0x005C8DA0-0x005C8E60;0x00662A70-0x00662AC8 BYTE_EXACT
// LEVER: the two vtable stores must come FIRST, before `gamma_ = 1.0`. Written after it the body is SHAPE_EXACT 39/42; written before it, BYTE_EXACT 42/42. The image interleaves them - `[esi+0xee8] = 0`, both vtables, then `[esi+0xeec] = 0x3ff00000` - which reads like the vtable stores landing in the middle of the double and impossible to write as source. It is the other way round: VC6 hoists the double's ZERO half ahead of whatever precedes it and leaves the 0x3ff00000 half in place, so putting the vtable stores first is what produces the image's own order. (This lesson was already written out in full INSIDE the function body, ten lines below the marker, where `decomp.reader` - which reads only the comment run immediately after the marker - could not see it: `osmx show` listed no LEVER for this address at all.)
// size      280 bytes
// prototype void (__thiscall ??0Gamma@@QAE@XZ)(Gamma* this)
// callers   2   call targets   6
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005D4CF0 0x005FE2A0 0x006051D0 0x00614E50 0x00618EA0 0x0062BF20
// LEVER (2026-09-03, the /Ob0 dependency closed): of the six member
//        constructors this body calls, five were already out of line
//        (Palette, Font, Scroll, EditBox and the GraphicWin base). The one
//        that was not - `PushButton() { ; }` in pushbutton.h - was EXPANDED
//        twice here, and each expansion carried the whole BaseButton chain,
//        so VC6 paid a second EH spill slot (`sub esp, 8`) plus 22 extra
//        instructions: 64 compiled against the image's 42, 10 of 42 agreeing.
//        Out of line (gamma.cpp, 0x0062BF20 BYTE_EXACT) the site is six
//        `call`s from a one-slot frame, 42 of 42. Nothing in this body
//        changed; the fix is entirely in the callee's definition.
Return Value: None
Status: Complete
*/
// The original constructor installs Gamma's Win and secondary Buffer views
// at object offsets 0 and 0x444. Each emitted identity is compiler-owned;
// their table extents and full contents remain unbounded here.
// RECOVERY_GENERATED: v1 vtable 0x0066C3D8 ? ??_7Gamma@@6BWin@@@ 0x005C8DA0
// RECOVERY_GENERATED: v1 vtable 0x0066C3D0 ? ??_7Gamma@@6BBuffer@@@ 0x005C8DA0
Gamma::Gamma() {
    // The vtable slots are compiler-managed, not ordinary members a class can
    // name - the same reason GraphicWin::construct writes its own pair at
    // these exact two offsets. Gamma has its own vtable because it overrides;
    // nothing in this chain is declared `virtual`, so the store is explicit
    // rather than emitted by a constructor VC6 would generate.
    // The store order matters; see the LEVER line on the marker above.
    gamma_ = 1.0;
    target_palette_ = 0;
}

/*
Purpose: Step the receiver back to the subobject ??_GGamma@@UAEPAXI@Z expects,
         then forward unchanged.
// ORIGINAL: 0x004562B0 ??_GGamma@@WEEE@AEPAXI@Z 0x004562B0-0x004562BB BYTE_EXACT
// compile_context src/interface_assets.cpp
// symbol    ??_EGamma@@WEEE@AEPAXI@Z
// CORRECTED from ??3Gamma@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x00456280` into
//   ??_GGamma@@UAEPAXI@Z, which executes `ret 4`; no stack access and
//   the receiver stays in ECX. `WEEE@` re-demangles to adjustor{1092}
//   and 1092 == 0x444, the constant subtracted
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
Purpose: ?on_scrolling@Gamma@@QAEXHH@Z - dispatch to slot 0xc0 of this object's OWN
         vtable, forwarding 2 argument(s).
// ORIGINAL: 0x005C93A0 ?on_scrolling@Gamma@@QAEXHH@Z 0x005C93A0-0x005C93B7 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned original own table 0x0066C3D8, pointer word at +0xC4, identifies original entry 0x005C93A0", "source:src/win.h inherited virtual interface; actual emitted method ?vslot_49@Gamma@@UAEXHH@Z in primary Win table ??_7Gamma@@6BWin@@@"], "reason": "The original own Win table binds this entry as virtual slot 49. Use genuine member dispatch, not an opaque redirect; retain the catalogued original spelling as historical evidence."}
// symbol    ?on_scrolling@Gamma@@UAEXHH@Z
// size      23 bytes
// prototype void (__thiscall ?on_scrolling@Gamma@@QAEXHH@Z)(Gamma* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005C93AD
Return Value: n/a
Status: Complete
*/
void Gamma::on_scrolling(int code, int position) {
  on_scrolled(code, position);
}
