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
// symbol    ??_GGamma@@UAEPAXI@Z
// ORIGINAL: 0x0062C830 ??_GPushButton@@UAEPAXI@Z 0x0062C830-0x0062C84E FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical deleting-thunk source src/deleting_thunks.cpp; original full spans retained: 0x0062C830-0x0062C84E", "Pinned class constructor ??0PushButton@@QAE@XZ at 0x0062BF20 stores original class/view table VA 0x670c60 at 0x0062BF28 (mov dword ptr [esi], 0x670c60); the current genuine owner vftable ??_7PushButton@@6BWin@@@ selects generated ??_GPushButton@@UAEPAXI@Z in slot 0 (+0x0) through weak entry ??_EPushButton@@UAEPAXI@Z (mode 2).", "Pinned ??_GPushButton@@UAEPAXI@Z at 0x0062C830 calls the source-mapped ??1PushButton@@QAE@XZ at 0x0062C010, then tests byte ptr [esp + 8], 1; je 0x62c848 bypasses scalar delete when bit 0 is clear, while bit 0 set calls 0x0064557F; it save original this; call destructor; delete only when flags bit 0 is set; return saved this with ret 4.", "The real class lifetime is owned by src/gamma.cpp; its compiler-emitted deleting symbol and vftable slot carry this original identity. Hand-written adapter bodies with live adjustor callers remain until those callers are independently qualified and transferred. The linked weak/COMDAT copy is not used to choose source ownership.", "the source writer preserves the historical mapping's exactness state (historical exactness obligation retained); current compiler extent is not asserted equal to the original span"], "reason": "Bind the original PushButton scalar deleting-destructor identity to the actual compiler-emitted class deleting destructor in its genuine owner translation unit. The original constructor's pinned vptr store and the current owner's physical table slot identify this class/view; no callback/thunk body is hand-authored here."}
// symbol    ??_GPushButton@@UAEPAXI@Z
#include "stdafx.h"
#include "gamma.h"
#include "worldwin.h"
#include "basepop.h"
#include "temp.h"
#include <math.h>

namespace {
const double GammaPositionScale = 0.01;
const double GammaComponentScale = 1.0 / 235.0;
const double GammaByteScale = 235.99;
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

    rect1_.left = 0;
    rect1_.top = 0;
    rect1_.right = 236;
    rect1_.bottom = 32;
    const int horizontal_offset =
        (WinScreenWidth - rect1_.right + rect1_.left) / 2;
    rect1_.left += horizontal_offset;
    rect1_.right += horizontal_offset;
    const int vertical_offset =
        (WinScreenHeight - rect1_.bottom + rect1_.top) / 2;
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
    GraphicWin::init(0, 0, WinScreenWidth, WinScreenHeight, 0, 0,
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

// ORIGINAL: 0x005C9520 ?adjust_palette@Gamma@@QAEXXZ 0x005C9520-0x005C95E0
// RECOVERY_BINDING: v1 {"evidence": ["Earlier source-owned body in src/stringstruct.cpp and retained original seed at git:117282ee", "Gamma's Scroll position occupies Gamma+0x191C and Palette entries begin at Gamma+0xA68", "Original ramp writes 236 RGB triples, reseeds and sets the Palette, then calls Win slot 12"], "reason": "Keep gamma-ramp behavior in its owning translation unit and operate on Palette entries and Scroll state rather than executable addresses or object offsets."}
// symbol    ?adjust_palette@Gamma@@QAEXXZ
void Gamma::adjust_palette() {
    gamma_ = static_cast<double>(scroll_.position()) * GammaPositionScale;
    for (int i = 0; i < 236; ++i) {
        double value = pow(static_cast<double>(i) * GammaComponentScale,
                           1.0 / gamma_);
        if (value < 0.0) value = 0.0;
        else if (value > 1.0) value = 1.0;
        const unsigned char shade = static_cast<unsigned char>(
            static_cast<int>(value * GammaByteScale));
        PALETTEENTRY &entry = palette_.entries_[i + 10];
        entry.peRed = shade;
        entry.peGreen = shade;
        entry.peBlue = shade;
    }
    palette_.reseed();
    palette_.set();
    on_redraw();
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// Historical seed observations (not fresh measurements or acceptance):
// TRIED: `RECT rect = *(RECT*)(self+0xa14);` (struct assignment through one base pointer, not 4 independent offset loads - the latter drops the original's `lea`+shared-`ecx` shape entirely) gets every field store byte-exact through the loop setup. The y2 = (bottom-top)+top redundant identity the ASM computes (sub ecx,eax; add ecx,eax) is always constant-folded to a plain `bottom` by every /O flag combination tried, whether written inline or through a named `height` temp - MISMATCH #19 is that fold, not a missed field.
// ORIGINAL: 0x005C9420 ?on_redraw@Gamma@@QAEXXZ 0x005C9420-0x005C951D
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/005c9420.cpp records the 236-line palette strip and the gamma label", "src/gamma.h owns rect1_, font_, gamma_ and the Buffer base at their measured offsets", "Original format string at 0x00691E7C is %f"], "reason": "Draw the dialog's colour ramp and current gamma through its actual GraphicWin and Buffer bases, overriding Win's slot 12."}
// Gamma overrides the existing Win redraw slot; its emitted method is virtual.
// symbol    ?on_redraw@Gamma@@UAEXXZ
void Gamma::on_redraw() {
    RECT label_area = rect1_;
    int x = label_area.left;
    for (int color = 10; color < 246; ++color) {
        Buffer::line(x, label_area.top, x, label_area.bottom, color);
        ++x;
    }

    const int delta = label_area.top - label_area.bottom - 3;
    label_area.top += delta;
    label_area.bottom += delta;
    GraphicWin::fill(&label_area, 0xF8);
    Buffer::set_font(&font_, 0, 0, 0);
    Buffer::set_text_color(0, -1, 1, 1);

    char value_text[80];
    sprintf(value_text, "%f", gamma_);
    Buffer::write_cent_l(value_text, &label_area, strlen(value_text));
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
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/005c95e0.cpp walks the target Palette from entry 10 through 245", "src/recovered/units/005c8e60.cpp stores the Palette* argument at Gamma+0xA64", "src/palette.h places PALETTEENTRY records at the start of Palette and exposes reseed/set"], "reason": "Apply gamma to the target palette's RGB entries as typed palette data, then publish its new seed and colours."}
// symbol    ?adjust_pal@Gamma@@QAEXN@Z
void Gamma::adjust_pal(double gamma) {
    Palette *const target = target_palette_;
    if (target == 0) return;

    const double inverse_gamma = 1.0 / gamma;
    for (int index = 10; index < 246; ++index) {
        PALETTEENTRY &entry = target->entries_[index];
        double red = pow(entry.peRed * GammaComponentScale, inverse_gamma);
        double green = pow(entry.peGreen * GammaComponentScale, inverse_gamma);
        double blue = pow(entry.peBlue * GammaComponentScale, inverse_gamma);

        if (red < 0.0) red = 0.0;
        else if (red > 1.0) red = 1.0;
        if (green < 0.0) green = 0.0;
        else if (green > 1.0) green = 1.0;
        if (blue < 0.0) blue = 0.0;
        else if (blue > 1.0) blue = 1.0;

        entry.peRed = static_cast<unsigned char>(
            static_cast<int>(red * GammaByteScale));
        entry.peGreen = static_cast<unsigned char>(
            static_cast<int>(green * GammaByteScale));
        entry.peBlue = static_cast<unsigned char>(
            static_cast<int>(blue * GammaByteScale));
    }
    target->reseed();
    target->set();
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00456110 ??1Gamma@@QAE@XZ 0x00456110-0x00456275;0x00655190-0x00655240
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00456110.cpp shows a Gamma::close call followed by reverse subobject teardown", "src/gamma.h declares the real member order and GraphicWin base", "C++ destruction emits subobject and EH cleanup without writing historical vtable addresses"], "reason": "Use compiler-owned lifetime for Gamma's embedded controls and base, preserving the explicit dialog close before automatic destruction."}
// symbol    ??1Gamma@@UAE@XZ
Gamma::~Gamma() {
    close();
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x005C93C0 ?on_button_clicked@Gamma@@QAEXH@Z 0x005C93C0-0x005C9406
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/005c93c0.cpp separates button -1 scroll acceptance from button -2 initial-gamma restoration", "src/gamma.h identifies gamma_, initialGamma_ and the embedded Scroll", "Win slot 58 is release_modal, the original indirect target"], "reason": "Apply or cancel the gamma adjustment and close the modal dialog through its real virtual method."}
// symbol    ?on_button_clicked@Gamma@@UAEXH@Z
void Gamma::on_button_clicked(int button_id) {
    if (button_id != -2) {
        if (button_id == -1) {
            gamma_ = static_cast<double>(scroll_.position()) * GammaPositionScale;
            release_modal();
            return;
        }
    } else {
        gamma_ = initialGamma_;
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
// symbol    ?vslot_48@Gamma@@UAEXHH@Z
// size      8 bytes
// prototype void (__thiscall ?on_scrolled@Gamma@@QAEXHH@Z)(Gamma* this, int, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005C9520
Status: Complete
*/
void Gamma::vslot_48(int code, int pos) {
    adjust_palette();
}


// The zero-init .bss table PushButton's constructor loads its field defaults
// from. Zero at load in the image, as here - the same idiom as
// BaseButtonA94Default (basebutton.cpp) and `Log Logging` (log.cpp).
// RECOVERY_OBJECT: v1 0x009BC118 ?PushButtonDefaults@@3PAPAVSprite@@A
Sprite *PushButtonDefaults[18];  // 0x009BC118


/*
Purpose: Construct the BaseButton base, stage both virtual tables, then load
         the eighteen field defaults from the process table.
// ORIGINAL: 0x0062BF20 ??0PushButton@@QAE@XZ 0x0062BF20-0x0062C00E BYTE_EXACT
// PROMOTED from src/unrecovered/0062bf20.cpp, 2026-09-03. That scaffold's
//        blocker is now gone: `class PushButton` derives from BaseButton for
//        real and `BaseButton::BaseButton()` is defined OUT OF LINE, so the
//        implicit base construction is the image's own `call ??0BaseButton`
//        and the two vtable stores are the compiler's - no `new (this) T()`
//        and no operator-new call. What remains of the scaffold's
//        hand-written body is the eighteen field defaults.
// symbol    ??0PushButton@@QAE@XZ
// size      238 bytes
// prototype void (__thiscall ??0PushButton@@QAE@XZ)(PushButton* this)
// callers   3   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00606F30
Status: Complete
*/
PushButton::PushButton() {
    field_AB8_ = PushButtonDefaults[0];
    field_ABC_ = PushButtonDefaults[1];
    field_AC0_ = PushButtonDefaults[2];
    field_AC4_ = PushButtonDefaults[3];
    field_AC8_ = PushButtonDefaults[4];
    field_ACC_ = PushButtonDefaults[5];
    field_AD0_ = PushButtonDefaults[6];
    field_AD4_ = PushButtonDefaults[7];
    field_AD8_ = PushButtonDefaults[8];
    field_ADC_ = PushButtonDefaults[9];
    field_AE0_ = PushButtonDefaults[10];
    field_AE4_ = PushButtonDefaults[11];
    field_AE8_ = PushButtonDefaults[12];
    field_AEC_ = PushButtonDefaults[13];
    field_AF0_ = PushButtonDefaults[14];
    field_AF4_ = PushButtonDefaults[15];
    field_AF8_ = PushButtonDefaults[16];
    field_AFC_ = PushButtonDefaults[17];
}


/*
Purpose: Destroy PushButton through its real BaseButton subobject.
// ORIGINAL: 0x0062C010 ??1PushButton@@QAE@XZ 0x0062C010-0x0062C025 BYTE_EXACT
// The current hierarchy makes this virtual (UAE); the original catalogue
// spells QAE. Bind the actual emitted symbol rather than changing inheritance
// to fit that label. C++ owns vptr staging and the single base teardown.
// symbol    ??1PushButton@@UAE@XZ
// size      21 bytes
// prototype void (__thiscall ??1PushButton@@QAE@XZ)(PushButton* this)
// callers   7   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
*/
PushButton::~PushButton() {
}

// ORIGINAL: 0x0062C110 ?on_redraw@PushButton@@QAEXXZ 0x0062C110-0x0062C82D
// RECOVERY_BINDING: v1 {"reason":"Recover the complete inherited-buffer painter in PushButton, using typed Sprite consumers and its actual BaseButton/GraphicWin hierarchy.","evidence":["Pinned PushButton table 0x670C60 slot 12 selects 0x62C110.","The original body consumes all eighteen AB8..AFC words as Sprite receivers/tile arguments. gamma.cpp copies the table into those words; game.cpp publishes six borrowed sprite pointers and nulls the other twelve.","The retained seed TRIED note documents the two parallel nine-sprite sets and pressed-state nesting; both branches remain in this source body."]}
// kind      game
// symbol    ?on_redraw@PushButton@@UAEXXZ
// TRIED: transcribed from Ghidra's decompile, fixing a real structural miss on the first pass - the sprite/tile draw block (0xac8!=0 branch) nests on `*(self+0xa9c)==0`, selecting between TWO parallel 9-draw sets (0xac8.. vs 0xaec..) that share one closing `tile()` call after the if/else, not one flat sequence. `this`-null guards (`-(uint)(this!=0)&buf`) are dropped in favour of `buf` directly - a member fn's `this` is never null, and the brief warns the ternary form of this idiom gets optimized away anyway. Frameless build compiles and matches through instr #2 (mov vs push) - not chased further, large member function with many null-guarded draws, landed for coverage.
void PushButton::on_redraw() {
    WinCallbackWindow = parent_window();
    if (HandleMain == 0) return;

    Buffer *buffer = static_cast<Buffer *>(this);
    buffer->sync_to_palette(PaletteActive);
    RECT rectangle = *buffer_working_rect();
    fill(static_cast<int>(color_));

    if (field_AC8_ == 0) {
        int bevel_count = 0;
        if (field_A94_ != 0xFFFFFFFFU) {
            buffer->box(&rectangle, static_cast<int>(field_A94_),
                        static_cast<int>(field_A94_));
            ++rectangle.left;
            --rectangle.right;
            ++rectangle.top;
            --rectangle.bottom;
            if (parent_window()->has_dialog_focus_flag() == false &&
                is_dialog_focus() != 0) {
                buffer->box(&rectangle, static_cast<int>(field_A94_),
                            static_cast<int>(field_A94_));
                ++rectangle.left;
                --rectangle.right;
                ++rectangle.top;
                --rectangle.bottom;
                bevel_count = 1;
            }
        }

        if (field_A9C_ == 0) {
            while (bevel_count < static_cast<int>(bevel_thickness_)) {
                buffer->box(&rectangle, static_cast<int>(bevel_upper_),
                            static_cast<int>(bevel_lower_));
                ++rectangle.left;
                --rectangle.right;
                ++rectangle.top;
                --rectangle.bottom;
                ++bevel_count;
            }
        } else {
            while (bevel_count < static_cast<int>(bevel_thickness_)) {
                buffer->box(&rectangle, static_cast<int>(bevel_lower_),
                            static_cast<int>(bevel_upper_));
                ++rectangle.left;
                --rectangle.right;
                ++rectangle.top;
                --rectangle.bottom;
                ++bevel_count;
            }
            rectangle.left += static_cast<LONG>(bevel_thickness_);
            rectangle.right += static_cast<LONG>(bevel_thickness_);
            rectangle.top += static_cast<LONG>(bevel_thickness_);
            rectangle.bottom += static_cast<LONG>(bevel_thickness_);
        }
    } else {
        const int inset = static_cast<int>(bevel_thickness_);
        rectangle.left += inset;
        rectangle.right -= inset;
        rectangle.top += inset;
        rectangle.bottom -= inset;

        Sprite *top_left;
        Sprite *top_right;
        Sprite *bottom_left;
        Sprite *bottom_right;
        Sprite *top_edge;
        Sprite *bottom_edge;
        Sprite *left_edge;
        Sprite *right_edge;
        Sprite *center;
        if (field_A9C_ == 0) {
            top_left = field_AC8_; top_right = field_ACC_;
            bottom_left = field_AD0_; bottom_right = field_AD4_;
            top_edge = field_AB8_; bottom_edge = field_ABC_;
            left_edge = field_AC0_; right_edge = field_AC4_; center = field_AD8_;
        } else {
            rectangle.left += inset;
            rectangle.right += inset;
            rectangle.top += inset;
            rectangle.bottom += inset;
            top_left = field_AEC_; top_right = field_AF0_;
            bottom_left = field_AF4_; bottom_right = field_AF8_;
            top_edge = field_ADC_; bottom_edge = field_AE0_;
            left_edge = field_AE4_; right_edge = field_AE8_; center = field_AFC_;
        }

        int draw_count = 0;
        if (top_left != 0) {
            top_left->draw(buffer, top_left->transparent_index(), 0, 0, 1, 1);
            draw_count = 1;
        }
        const int width = buffer->pixel_width();
        const int height = buffer->pixel_height();
        if (top_right != 0) {
            ++draw_count;
            top_right->draw(buffer, top_right->transparent_index(),
                            width - top_right->width(), 0, 1, 1);
        }
        if (bottom_left != 0) {
            ++draw_count;
            bottom_left->draw(buffer, bottom_left->transparent_index(), 0,
                              -(bottom_left->height() + height), 1, 1);
        }
        if (bottom_right != 0) {
            ++draw_count;
            bottom_right->draw(buffer, bottom_right->transparent_index(),
                               width - bottom_right->width(),
                               -(bottom_right->height() + height), 1, 1);
        }
        if (top_edge != 0 && top_left != 0 && bottom_left != 0) {
            const int source_height = top_left->height();
            ++draw_count;
            buffer->tile(top_edge, 0, 0, 0, source_height,
                         top_edge->width(),
                         -(bottom_left->height() + height + source_height));
        }
        if (bottom_edge != 0 && top_right != 0 && bottom_right != 0) {
            const int source_height = top_right->height();
            ++draw_count;
            buffer->tile(bottom_edge, 0, 0, width - bottom_edge->width(),
                         source_height, bottom_edge->width(),
                         -(bottom_right->height() + height + source_height));
        }
        if (left_edge != 0 && top_left != 0 && top_right != 0) {
            const int source_width = top_left->width();
            ++draw_count;
            buffer->tile(left_edge, 0, 0, source_width, 0,
                         (width - top_right->width()) - source_width,
                         left_edge->height());
        }
        if (right_edge != 0 && bottom_left != 0 && bottom_right != 0) {
            const int source_width = bottom_left->width();
            ++draw_count;
            buffer->tile(right_edge, 0, 0, source_width,
                         -(height + right_edge->height()),
                         (width - bottom_right->width()) - source_width,
                         right_edge->height());
        }

        if (center != 0 && draw_count == 8 && top_edge != 0 && left_edge != 0 &&
            right_edge != 0 && bottom_edge != 0 && top_right != 0 &&
            bottom_left != 0 && bottom_right != 0) {
            const int source_y = left_edge->height();
            const int source_x = bottom_edge->width();
            const int tile_height = height + right_edge->height() + source_y;
            buffer->tile(center, 0, 0, source_x, source_y,
                         (width - top_edge->width()) - source_x, -tile_height);
        }
    }

    if (name_ != 0) {
        buffer->write_cent_l(name_, &rectangle,
                             static_cast<unsigned int>(strlen(name_)));
    }
}

// Reset the base first, then consume the current shared defaults. This is
// slot90 of the PushButton primary table, not part of its destructor.
// Superseded seed deleted; seed paths below are historical Git provenance.
// Historical seed observations (not fresh measurements or acceptance):
// LEVER: reinterpret_cast<BaseButton*>(this)->close() for the zero-offset base call, then 18 individual field-by-field assignments from the 18 fixed globals (fully unrolled in the original, no rep movsd), matching disasm order exactly.
// ORIGINAL: 0x0062C030 ?close@PushButton@@QAEXXZ 0x0062C030-0x0062C10C BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original constructor62BF28 stages table670C60, whose slot90 targets complete body62C030..62C10C. GraphicWin ends at slot89; BaseButton introduces virtual void close and this body overrides it.", "Original62C033 calls BaseButton6070C0 directly before eighteen sequential default loads9BC118..9BC160 into existing fieldsAB8..B00. Source uses the existing PushButtonDefaults owner without caching across the base cleanup. Its load_interface producer remains unrecovered; this binding is not a full startup/storage-closure claim.", "Original62C010 destructor only stages tables and delegates to BaseButton destruction; no extra PushButton close belongs in the destructor. Preserve the exact seed identity/obligation while measuring the real void override."], "reason": "Recover the missing PushButton virtual cleanup in its genuine BaseButton hierarchy and owning translation unit."}
// symbol    ?close@PushButton@@UAEXXZ
void PushButton::close() {
    BaseButton::close();
    field_AB8_ = PushButtonDefaults[0];
    field_ABC_ = PushButtonDefaults[1];
    field_AC0_ = PushButtonDefaults[2];
    field_AC4_ = PushButtonDefaults[3];
    field_AC8_ = PushButtonDefaults[4];
    field_ACC_ = PushButtonDefaults[5];
    field_AD0_ = PushButtonDefaults[6];
    field_AD4_ = PushButtonDefaults[7];
    field_AD8_ = PushButtonDefaults[8];
    field_ADC_ = PushButtonDefaults[9];
    field_AE0_ = PushButtonDefaults[10];
    field_AE4_ = PushButtonDefaults[11];
    field_AE8_ = PushButtonDefaults[12];
    field_AEC_ = PushButtonDefaults[13];
    field_AF0_ = PushButtonDefaults[14];
    field_AF4_ = PushButtonDefaults[15];
    field_AF8_ = PushButtonDefaults[16];
    field_AFC_ = PushButtonDefaults[17];
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
// symbol    ?vslot_49@Gamma@@UAEXHH@Z
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
void Gamma::vslot_49(int code, int position) {
  vslot_48(code, position);
}

/*
Purpose: Step the receiver back to the subobject ??_GPushButton@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x0062C850 ??_GPushButton@@WEEE@AEPAXI@Z 0x0062C850-0x0062C85B BYTE_EXACT
// symbol    ??_EPushButton@@WEEE@AEPAXI@Z
// CORRECTED from ??3PushButton@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x0062C830` into
//   ??_GPushButton@@UAEPAXI@Z, which executes `ret 4`; no stack access
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
