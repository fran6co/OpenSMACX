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
#include "pushbutton.h"
#include "gamma.h"
#include "worldwin.h"
#include "basepop.h"
#include "win.h"
#include <math.h>

// ORIGINAL: 0x0062C830 ??_GPushButton@@UAEPAXI@Z 0x0062C830-0x0062C84E FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical deleting-thunk source src/deleting_thunks.cpp; original full spans retained: 0x0062C830-0x0062C84E", "Pinned class constructor ??0PushButton@@QAE@XZ at 0x0062BF20 stores original class/view table VA 0x670c60 at 0x0062BF28 (mov dword ptr [esi], 0x670c60); the current genuine owner vftable ??_7PushButton@@6BWin@@@ selects generated ??_GPushButton@@UAEPAXI@Z in slot 0 (+0x0) through weak entry ??_EPushButton@@UAEPAXI@Z (mode 2).", "Pinned ??_GPushButton@@UAEPAXI@Z at 0x0062C830 calls the source-mapped ??1PushButton@@QAE@XZ at 0x0062C010, then tests byte ptr [esp + 8], 1; je 0x62c848 bypasses scalar delete when bit 0 is clear, while bit 0 set calls 0x0064557F; it save original this; call destructor; delete only when flags bit 0 is set; return saved this with ret 4.", "The real class lifetime is owned by src/pushbutton.cpp; its compiler-emitted deleting symbol and vftable slot carry this original identity. Hand-written adapter bodies with live adjustor callers remain until those callers are independently qualified and transferred. The linked weak/COMDAT copy is not used to choose source ownership.", "the source writer preserves the historical mapping's exactness state (historical exactness obligation retained); current compiler extent is not asserted equal to the original span"], "reason": "Bind the original PushButton scalar deleting-destructor identity to the actual compiler-emitted class deleting destructor in its genuine PushButton owner translation unit. The original constructor's pinned vptr store and the current owner's physical table slot identify this class/view; no callback/thunk body is hand-authored here."}
// symbol    ??_GPushButton@@UAEPAXI@Z
// kind      game
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

// WORKAROUND(inline): expand_rect (scroll.cpp) is a plain out-of-line
// function in this tree; on_redraw expands it inline (0x0062C194: the negated
// bevel thickness held in edx, applied left/right/top/bottom; each `++left;
// --right; ++top; --bottom` run after a box is its (-1, -1) form).
static inline RECT *pushbutton_expand_rect(RECT *rect, int horizontal, int vertical) {
    rect->left -= horizontal;
    rect->right += horizontal;
    rect->top -= vertical;
    rect->bottom += vertical;
    return rect;
}

// ORIGINAL: 0x0062C110 ?on_redraw@PushButton@@QAEXXZ 0x0062C110-0x0062C82D
// RECOVERY_BINDING: v1 {"evidence": ["Pinned PushButton table 0x670C60 slot 12 selects 0x62C110.", "The original body consumes all eighteen AB8..AFC words as Sprite receivers/tile arguments. pushbutton.cpp copies the table into those words; game.cpp publishes six borrowed sprite pointers and nulls the other twelve.", "The retained seed TRIED note documents the two parallel nine-sprite sets and pressed-state nesting; both branches remain in this source body."], "reason": "Recover the complete inherited-buffer painter in PushButton, using typed Sprite consumers and its actual BaseButton/GraphicWin hierarchy."}
// kind      game
// symbol    ?on_redraw@PushButton@@UAEXXZ
// TRIED: transcribed from Ghidra's decompile, fixing a real structural miss on the first pass - the sprite/tile draw block (0xac8!=0 branch) nests on `*(self+0xa9c)==0`, selecting between TWO parallel 9-draw sets (0xac8.. vs 0xaec..) that share one closing `tile()` call after the if/else, not one flat sequence. `this`-null guards (`-(uint)(this!=0)&buf`) are dropped in favour of `buf` directly - a member fn's `this` is never null, and the brief warns the ternary form of this idiom gets optimized away anyway. Frameless build compiles and matches through instr #2 (mov vs push) - not chased further, large member function with many null-guarded draws, landed for coverage.
void PushButton::on_redraw() {
    WinCallbackWindow = parent_window();
    if (HandleMain == 0 || parent_window() == 0) return;

    sync_to_palette(PaletteActive);
    RECT rectangle = *buffer_working_rect();
    fill(static_cast<int>(color_));

    if (field_AC8_ != 0) {
        int draw_count = 0;
        pushbutton_expand_rect(&rectangle, -static_cast<int>(bevel_thickness_),
                               -static_cast<int>(bevel_thickness_));

        // Keep the original signed, top-down DIB member at each bottom/tile
        // coordinate site. The pinned body reloads biHeight and adds it to
        // Sprite heights before negating; pixel_height() is its positive
        // magnitude and changes those coordinates when substituted here.
        if (field_A9C_ == 0) {
            if (field_AC8_ != 0) {
                draw_count = 1;
                field_AC8_->draw(this, field_AC8_->transparent_index(), 0, 0, 1, 1);
            }
            if (field_ACC_ != 0) {
                ++draw_count;
                field_ACC_->draw(this, field_ACC_->transparent_index(),
                                 dib_.bmiHeader.biWidth - field_ACC_->width(), 0, 1, 1);
            }
            if (field_AD0_ != 0) {
                ++draw_count;
                field_AD0_->draw(this, field_AD0_->transparent_index(), 0,
                                 -(field_AD0_->height() + dib_.bmiHeader.biHeight), 1, 1);
            }
            if (field_AD4_ != 0) {
                ++draw_count;
                field_AD4_->draw(this, field_AD4_->transparent_index(),
                                 dib_.bmiHeader.biWidth - field_AD4_->width(),
                                 -(field_AD4_->height() + dib_.bmiHeader.biHeight), 1, 1);
            }
            if (field_AB8_ != 0 && field_AC8_ != 0 && field_AD0_ != 0) {
                ++draw_count;
                tile(field_AB8_, 0, 0, 0, field_AC8_->height(), field_AB8_->width(),
                     -(field_AD0_->height() + dib_.bmiHeader.biHeight
                       + field_AC8_->height()));
            }
            if (field_ABC_ != 0 && field_ACC_ != 0 && field_AD4_ != 0) {
                ++draw_count;
                tile(field_ABC_, 0, 0, dib_.bmiHeader.biWidth - field_ABC_->width(),
                     field_ACC_->height(), field_ABC_->width(),
                     -(field_AD4_->height() + dib_.bmiHeader.biHeight
                       + field_ACC_->height()));
            }
            if (field_AC0_ != 0 && field_AC8_ != 0 && field_ACC_ != 0) {
                ++draw_count;
                tile(field_AC0_, 0, 0, field_AC8_->width(), 0,
                     dib_.bmiHeader.biWidth - field_ACC_->width() - field_AC8_->width(),
                     field_AC0_->height());
            }
            if (field_AC4_ != 0 && field_AD0_ != 0 && field_AD4_ != 0) {
                ++draw_count;
                tile(field_AC4_, 0, 0, field_AD0_->width(),
                     -(dib_.bmiHeader.biHeight + field_AC4_->height()),
                     dib_.bmiHeader.biWidth - field_AD4_->width() - field_AD0_->width(),
                     field_AC4_->height());
            }
            if (field_AD8_ != 0 && draw_count == 8) {
                tile(field_AD8_, 0, 0, field_ABC_->width(), field_AC0_->height(),
                     dib_.bmiHeader.biWidth - field_AB8_->width() - field_ABC_->width(),
                     -(dib_.bmiHeader.biHeight + field_AC4_->height()
                       + field_AC0_->height()));
            }
        } else {
            offset_rect(&rectangle, bevel_thickness_, bevel_thickness_);
            if (field_AEC_ != 0) {
                draw_count = 1;
                field_AEC_->draw(this, field_AEC_->transparent_index(), 0, 0, 1, 1);
            }
            if (field_AF0_ != 0) {
                ++draw_count;
                field_AF0_->draw(this, field_AF0_->transparent_index(),
                                 dib_.bmiHeader.biWidth - field_AF0_->width(), 0, 1, 1);
            }
            if (field_AF4_ != 0) {
                ++draw_count;
                field_AF4_->draw(this, field_AF4_->transparent_index(), 0,
                                 -(field_AF4_->height() + dib_.bmiHeader.biHeight), 1, 1);
            }
            if (field_AF8_ != 0) {
                ++draw_count;
                field_AF8_->draw(this, field_AF8_->transparent_index(),
                                 dib_.bmiHeader.biWidth - field_AF8_->width(),
                                 -(field_AF8_->height() + dib_.bmiHeader.biHeight), 1, 1);
            }
            if (field_ADC_ != 0 && field_AEC_ != 0 && field_AF4_ != 0) {
                ++draw_count;
                tile(field_ADC_, 0, 0, 0, field_AEC_->height(), field_ADC_->width(),
                     -(field_AF4_->height() + dib_.bmiHeader.biHeight
                       + field_AEC_->height()));
            }
            if (field_AE0_ != 0 && field_AF0_ != 0 && field_AF8_ != 0) {
                ++draw_count;
                tile(field_AE0_, 0, 0, dib_.bmiHeader.biWidth - field_AE0_->width(),
                     field_AF0_->height(), field_AE0_->width(),
                     -(field_AF8_->height() + dib_.bmiHeader.biHeight
                       + field_AF0_->height()));
            }
            if (field_AE4_ != 0 && field_AEC_ != 0 && field_AF0_ != 0) {
                ++draw_count;
                tile(field_AE4_, 0, 0, field_AEC_->width(), 0,
                     dib_.bmiHeader.biWidth - field_AF0_->width() - field_AEC_->width(),
                     field_AE4_->height());
            }
            if (field_AE8_ != 0 && field_AF4_ != 0 && field_AF8_ != 0) {
                ++draw_count;
                tile(field_AE8_, 0, 0, field_AF4_->width(),
                     -(dib_.bmiHeader.biHeight + field_AE8_->height()),
                     dib_.bmiHeader.biWidth - field_AF8_->width() - field_AF4_->width(),
                     field_AE8_->height());
            }
            if (field_AFC_ != 0 && draw_count == 8) {
                tile(field_AFC_, 0, 0, field_AE0_->width(), field_AE4_->height(),
                     dib_.bmiHeader.biWidth - field_ADC_->width() - field_AE0_->width(),
                     -(dib_.bmiHeader.biHeight + field_AE8_->height()
                       + field_AE4_->height()));
            }
        }
    } else {
        int bevel_count = 0;
        if (field_A94_ != 0xFFFFFFFFU) {
            box(&rectangle, static_cast<int>(field_A94_),
                        static_cast<int>(field_A94_));
            pushbutton_expand_rect(&rectangle, -1, -1);
            // WORKAROUND(decl): the image tests `not ecx; test ch,0x10` on the
            // parent's flags (0x0062C6D7), the form of `~iFlags_ & 0x1000`; Win
            // has no accessor for it and iFlags_ is protected.
            if ((~parent_window()->has_dialog_focus_flag() & 0x1000) &&
                is_dialog_focus() != 0) {
                box(&rectangle, static_cast<int>(field_A94_),
                            static_cast<int>(field_A94_));
                pushbutton_expand_rect(&rectangle, -1, -1);
                bevel_count = 1;
            }
        }

        if (field_A9C_ == 0) {
            while (bevel_count < static_cast<int>(bevel_thickness_)) {
                box(&rectangle, static_cast<int>(bevel_upper_),
                            static_cast<int>(bevel_lower_));
                pushbutton_expand_rect(&rectangle, -1, -1);
                ++bevel_count;
            }
        } else {
            while (bevel_count < static_cast<int>(bevel_thickness_)) {
                box(&rectangle, static_cast<int>(bevel_lower_),
                            static_cast<int>(bevel_upper_));
                pushbutton_expand_rect(&rectangle, -1, -1);
                ++bevel_count;
            }
            offset_rect(&rectangle, bevel_thickness_, bevel_thickness_);
        }
    }

    write_cent_l(name_, &rectangle);
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
