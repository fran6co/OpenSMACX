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
#include "temp.h"
#include "basebutton.h"

#include <new>

const uint32_t BaseButtonPrimaryVtable = 0x00670290;
const uint32_t BaseButtonBufferVtable = 0x00670288;
uint32_t *BaseButtonStaticDefaults = (uint32_t *)0x0069704C;
uint32_t *BaseButtonDynamicDefaults = (uint32_t *)0x009B8E2C;

/*
Purpose: Construct the GraphicWin base and two Time members, then install the
         BaseButton tables and process defaults.
Original Offset: 00606F30
Status: Complete
Verification note: the surviving swap mutants reorder construction of the
GraphicWin base against the Time members, which occupy disjoint storage, so
their order is not observable.
*/
void BaseButton::construct() {
    static_cast<GraphicWin *>(this)->construct();
    new (&time1_) Time();
    new (&time2_) Time();
    volatile uint32_t *const object =
        reinterpret_cast<volatile uint32_t *>(this);
    volatile const uint32_t *const fixed = BaseButtonStaticDefaults;
    volatile const uint32_t *const dynamic = BaseButtonDynamicDefaults;
    object[0x000 / 4] = BaseButtonPrimaryVtable;
    object[0x444 / 4] = BaseButtonBufferVtable;
    object[0xA74 / 4] = 0;
    object[0xA44 / 4] = 0xFFFFFFFFU;
    object[0xA48 / 4] = 0xFFFFFFFFU;
    object[0xA78 / 4] = 0;
    object[0xA9C / 4] = 0;
    object[0xA7C / 4] = 0;
    object[0xA80 / 4] = 0;
    object[0xAA8 / 4] = 0;
    object[0xAAC / 4] = 0;
    object[0xAB0 / 4] = 0;
    object[0xAB4 / 4] = 0;
    object[0xA94 / 4] = dynamic[0];
    object[0xA84 / 4] = fixed[0];
    object[0xA88 / 4] = fixed[1];
    object[0xA8C / 4] = fixed[2];
    object[0xA90 / 4] = fixed[3];
    object[0xA98 / 4] = dynamic[1];
    object[0xAA4 / 4] = 0;
    object[0xAA0 / 4] = fixed[4];
#if defined(__GNUC__) && defined(__i386__)
    __asm__ __volatile__("" : : "a"(this) : "memory");
#endif
}

BaseButton *__fastcall base_button_construct_redirect(
        BaseButton *self, void *) {
    self->construct();
    return self;
}

/*
Purpose: Close the GraphicWin base, reset BaseButton-owned state from the
         process defaults, then release the owned name and bubble strings.
Original Offset: 006070C0
Return Value: Zero when no bubble string is released; otherwise the executable
              free routine's EAX residue
Status: Complete; string storage remains owned by the executable CRT
*/
uint32_t BaseButton::close() {
    GraphicWin::close();

    volatile uint32_t *const object =
        reinterpret_cast<volatile uint32_t *>(this);
    volatile uint32_t *const fixed =
        reinterpret_cast<volatile uint32_t *>(BaseButtonStaticDefaults);
    volatile uint32_t *const dynamic =
        reinterpret_cast<volatile uint32_t *>(BaseButtonDynamicDefaults);
    object[0xA74 / 4] = 0;
    object[0xA9C / 4] = 0;
    object[0xA78 / 4] = 0;
    object[0xA44 / 4] = 0xFFFFFFFFU;
    object[0xA48 / 4] = 0xFFFFFFFFU;
    object[0xAAC / 4] = 0;
    object[0xAB0 / 4] = 0;
    object[0xAB4 / 4] = 0;
    object[0xA94 / 4] = dynamic[0];
    object[0xA84 / 4] = fixed[0];
    object[0xA88 / 4] = fixed[1];
    object[0xA8C / 4] = fixed[2];
    object[0xA90 / 4] = fixed[3];
    object[0xA98 / 4] = dynamic[1];
    object[0xAA4 / 4] = 0;
    object[0xAA0 / 4] = fixed[4];

    const uint32_t name = object[0xA7C / 4];
    if (name != 0) {
        _free(reinterpret_cast<void *>(static_cast<uintptr_t>(name)));
        object[0xA7C / 4] = 0;
    }
    uint32_t result = 0;
    const uint32_t bubble = object[0xA80 / 4];
    if (bubble != 0) {
        result = reinterpret_cast<uintptr_t>(_free(
            reinterpret_cast<void *>(static_cast<uintptr_t>(bubble))));
        object[0xA80 / 4] = 0;
    }
    object[0xAA8 / 4] = 0;
    return result;
}

uint32_t __fastcall base_button_close_redirect(BaseButton *self, void *) {
    return self->close();
}

/*
Purpose: Destroy a BaseButton by installing its two virtual tables, closing
         it, destroying Time2 then Time1, and finally destroying GraphicWin.
Original Offset: 00607040
Return Value: Instance pointer in EAX
Status: Complete
*/
BaseButton *BaseButton::destroy() {
    volatile uint32_t *const object =
        reinterpret_cast<volatile uint32_t *>(this);
    object[0x000 / 4] = BaseButtonPrimaryVtable;
    object[0x444 / 4] = BaseButtonBufferVtable;
    close();
    time2_.~Time();
    time1_.~Time();
    graphic_win_destructor_redirect(
        static_cast<GraphicWin *>(this), nullptr);
    return this;
}

BaseButton *__fastcall base_button_destructor_redirect(
        BaseButton *self, void *) {
    return self->destroy();
}

/*
Purpose: Set the button's bubble text.
Original Offset: 00607550
Return Value: Zero on success, non-zero on error
Status: Complete with redirect for free to prevent hang/freeze. Incompatibility between older
        version of free with newer SDK version of free CRT. Revisit once more of code is redirected
        to dll.
*/
int BaseButton::set_bubble_text(LPCSTR input) {
    if (bubble_text_) {
        _free(bubble_text_);
        bubble_text_ = 0;
    }
    if (input) {
        size_t len = strlen(input) + 1;
        bubble_text_ = (LPSTR)mem_get_old(len);
        if (!bubble_text_) {
            return 4;
        }
        strcpy_s(bubble_text_, len, input);
    }
    return 0;
}

/*
Purpose: Set the button's name string.
Original Offset: 006074E0
Return Value: Zero on success, non-zero on error
Status: Complete with redirect for free to prevent hang/freeze. Incompatibility between older
        version of free with newer SDK version of free CRT. Revisit once more of code is redirected
        to dll.
*/
int BaseButton::set_name(LPCSTR input) {
    // Bug fix: Fixed crash if input parameter was null. Original code had string copy outside last 
    // if statement causing potential write to null name variable.
    if (name_) {
        _free(name_);
        name_ = 0;
    }
    if (input) {
        size_t len = strlen(input) + 1;
        name_ = (LPSTR)mem_get_old(len);
        if (!name_) {
            return 4;
        }
        strcpy_s(name_, len, input);
    }
    return 0;
}

// The three default colour tiers interleave within one table: slot s and
// tier t live at 0x00697060 + s * 0xC + t * 4, so each setter strides 0xC.
uint32_t *BaseButtonDefaultTextColors = reinterpret_cast<uint32_t *>(0x00697060);
// Default font1/font2/font3 at 0x009B8E34, 0x009B8E38 and 0x009B8E3C.
Font **BaseButtonDefaultFonts = reinterpret_cast<Font **>(0x009B8E34);

namespace {

void store_default_text_colors(size_t tier, int color1, int color2,
                               int color3, int color4) {
    volatile uint32_t *const table = BaseButtonDefaultTextColors;
    const int colors[4] = {color1, color2, color3, color4};
    for (size_t slot = 0; slot < 4; ++slot) {
        table[(slot * 0xC + tier * 4) / 4] = static_cast<uint32_t>(colors[slot]);
    }
}

}  // namespace

/*
Purpose: Set the primary default text colours shared by every button.
Original Offset: 00607420
Status: Complete
*/
void BaseButton::set_def_text_color(int color1, int color2, int color3, int color4) {
    store_default_text_colors(0, color1, color2, color3, color4);
}

/*
Purpose: Set the secondary default text colours shared by every button.
Original Offset: 00607450
Status: Complete
*/
void BaseButton::set_def_text_color2(int color1, int color2, int color3, int color4) {
    store_default_text_colors(1, color1, color2, color3, color4);
}

/*
Purpose: Set the tertiary default text colours shared by every button.
Original Offset: 00607480
Status: Complete
*/
void BaseButton::set_def_text_color3(int color1, int color2, int color3, int color4) {
    store_default_text_colors(2, color1, color2, color3, color4);
}

/*
Purpose: Set the default fonts shared by every button.
Original Offset: 006074B0
Return Value: No errors (0); invalid primary font (3)
Status: Complete
*/
int BaseButton::set_def_font(Font *font1, Font *font2, Font *font3) {
    if (!font1) {
        return 3;
    }
    volatile Font **const fonts =
        const_cast<volatile Font **>(BaseButtonDefaultFonts);
    // Only an initialized primary font is published, but the secondary and
    // tertiary slots are stored either way and the call still succeeds.
    if (font1->is_initialized()) {
        fonts[0] = font1;
    }
    fonts[1] = font2;
    fonts[2] = font3;
    return 0;
}

void __cdecl base_button_set_def_text_color_redirect(
        int color1, int color2, int color3, int color4) {
    BaseButton::set_def_text_color(color1, color2, color3, color4);
}

void __cdecl base_button_set_def_text_color2_redirect(
        int color1, int color2, int color3, int color4) {
    BaseButton::set_def_text_color2(color1, color2, color3, color4);
}

void __cdecl base_button_set_def_text_color3_redirect(
        int color1, int color2, int color3, int color4) {
    BaseButton::set_def_text_color3(color1, color2, color3, color4);
}

int __cdecl base_button_set_def_font_redirect(
        Font *font1, Font *font2, Font *font3) {
    return BaseButton::set_def_font(font1, font2, font3);
}

// The active palette these setters publish before recolouring. This is a
// different global from Buffer's own BufferPalette at 0x009B8174.
Palette **BaseButtonActivePalette = reinterpret_cast<Palette **>(0x009B8180);

namespace {

// The three tiers differ only in which Buffer setter they end with, and each
// republishes the active palette first so the new colours resolve against it.
typedef void (Buffer::*BufferColourSetter)(int, int, int, int);

void recolour(Buffer &buffer, BufferColourSetter setter,
              int c1, int c2, int c3, int c4) {
    buffer.sync_to_palette(*BaseButtonActivePalette);
    (buffer.*setter)(c1, c2, c3, c4);
}

}  // namespace

/*
Purpose: Set the button's primary text colours.
Original Offset: 00607360
Status: Complete
Verification note: the parentless guard on all three setters carries no
literal or comparison operator, so the mutation harness cannot perturb it;
the no-parent case is covered behaviourally instead, asserting the whole
object is untouched. The dispatch is deliberately one statement per setter so
dropping it is a valid mutant - split across lines it produced only
uncompilable fragments and the sweep reported no signal at all.
*/
void BaseButton::set_text_color(int color1, int color2, int color3, int color4) {
    // A parentless button has nothing to draw into, so the legacy body skips
    // the palette sync and the recolour entirely.
    if (!win_parent_) {
        return;
    }
    recolour(buffer_, &Buffer::set_text_color, color1, color2, color3, color4);
}

/*
Purpose: Set the button's secondary text colours.
Original Offset: 006073A0
Status: Complete
*/
void BaseButton::set_text_color2(int color1, int color2, int color3, int color4) {
    if (!win_parent_) {
        return;
    }
    recolour(buffer_, &Buffer::set_text_color2, color1, color2, color3, color4);
}

/*
Purpose: Set the button's tertiary text colours.
Original Offset: 006073E0
Status: Complete
*/
void BaseButton::set_text_color3(int color1, int color2, int color3, int color4) {
    if (!win_parent_) {
        return;
    }
    recolour(buffer_, &Buffer::set_text_color3, color1, color2, color3, color4);
}

void __fastcall base_button_set_text_color_redirect(
        BaseButton *self, void *, int color1, int color2, int color3, int color4) {
    self->set_text_color(color1, color2, color3, color4);
}

void __fastcall base_button_set_text_color2_redirect(
        BaseButton *self, void *, int color1, int color2, int color3, int color4) {
    self->set_text_color2(color1, color2, color3, color4);
}

void __fastcall base_button_set_text_color3_redirect(
        BaseButton *self, void *, int color1, int color2, int color3, int color4) {
    self->set_text_color3(color1, color2, color3, color4);
}

namespace {

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
#endif
typedef void(__thiscall func_button_refresh_slot)(void *);
typedef void(__thiscall func_parent_notify_slot)(void *, int, int);
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

// Both dispatches read the live vtable of whatever object is actually there
// rather than going through a C++ virtual call, so no table this toolchain
// lays out can disagree with the original's.
constexpr size_t BaseButtonRefreshSlot = 0xF8;
constexpr size_t WinValueChangedSlot = 0xB4;

}  // namespace

/*
Purpose: Give the button a new value, redrawing and notifying its parent only
         when the value actually changes.
Original Offset: 00607C80
Return Value: n/a
Status: Complete

Ordering is load-bearing and matches the original instruction for instruction:
the field is stored before the redraw dispatch, and both the button's id and
the parent link are read *after* it. A redraw handler that retargets the
button therefore has its new id and new parent used for the notification,
which is why neither is cached up front.
*/
void BaseButton::set(int value) {
    if (value == field_A18_) {
        return;
    }
    field_A18_ = value;

    void **const own_vtable = *reinterpret_cast<void ***>(this);
    reinterpret_cast<func_button_refresh_slot *>(
        own_vtable[BaseButtonRefreshSlot / sizeof(void *)])(this);

    if (!win_parent_) {
        return;
    }
    void **const parent_vtable = *reinterpret_cast<void ***>(win_parent_);
    reinterpret_cast<func_parent_notify_slot *>(
        parent_vtable[WinValueChangedSlot / sizeof(void *)])(
            win_parent_, static_cast<int>(field_A78_), value);
}

void __fastcall base_button_set_redirect(BaseButton *self, void *, int value) {
    self->set(value);
}

/*
Purpose: Legacy stub; the original body returns nothing without reading its
         arguments.
Original Offset: 006077F0
Status: Complete
*/
void BaseButton::on_key_click(int, int) {
}

void __fastcall base_button_on_key_click_redirect(
        BaseButton *self, void *, int a, int b) {
    self->on_key_click(a, b);
}

/*
Purpose: Legacy stub; the original body returns nothing without reading its
         arguments.
Original Offset: 00607800
Status: Complete
*/
void BaseButton::on_key_down(int) {
}

void __fastcall base_button_on_key_down_redirect(
        BaseButton *self, void *, int a) {
    self->on_key_down(a);
}

/*
Purpose: Legacy stub; the original body returns nothing without reading its
         arguments.
Original Offset: 00607810
Status: Complete
*/
void BaseButton::on_key_up(int) {
}

void __fastcall base_button_on_key_up_redirect(
        BaseButton *self, void *, int a) {
    self->on_key_up(a);
}
