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
#include "original_seam.h"
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
ORIGINAL: 0x00606F30
// name      ??0BaseButton@@QAE@XZ
// size      291 bytes
// spans     0x00606F30-0x00607033;0x00662E50-0x00662E70
// prototype void (__thiscall ??0BaseButton@@QAE@XZ)(BaseButton* this)
// callers   3   call targets   2
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
}

BaseButton *__fastcall base_button_construct_redirect(
        BaseButton *self, void *) {
    self->construct();
    return self;
}

/*
Purpose: Close the GraphicWin base, reset BaseButton-owned state from the
         process defaults, then release the owned name and bubble strings.
ORIGINAL: 0x006070C0
// name      ?close@BaseButton@@QAEXXZ
// size      208 bytes
// spans     0x006070C0-0x00607190
// prototype void (__thiscall ?close@BaseButton@@QAEXXZ)(BaseButton* this)
// callers   3   call targets   2
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
ORIGINAL: 0x00607040
// name      ??1BaseButton@@QAE@XZ
// size      167 bytes
// spans     0x00607040-0x006070B9;0x00662E70-0x00662E9E
// prototype void (__thiscall ??1BaseButton@@QAE@XZ)(BaseButton* this)
// callers   134   call targets   3
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
ORIGINAL: 0x00607550
// name      ?set_bubble_text@BaseButton@@QAEHPAD@Z
// size      103 bytes
// spans     0x00607550-0x006075B7
// prototype int (__thiscall ?set_bubble_text@BaseButton@@QAEHPAD@Z)(BaseButton* this, int8*)
// callers   8   call targets   4
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
ORIGINAL: 0x006074E0
// name      ?set_name@BaseButton@@QAEHPAD@Z
// size      109 bytes
// spans     0x006074E0-0x0060754D
// prototype int (__thiscall ?set_name@BaseButton@@QAEHPAD@Z)(BaseButton* this, int8*)
// callers   4   call targets   4
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
ORIGINAL: 0x00607420
// name      ?set_def_text_color@BaseButton@@QAAXHHHH@Z
// size      39 bytes
// spans     0x00607420-0x00607447
// prototype 
// callers   3   call targets   0
Status: Complete
*/
void BaseButton::set_def_text_color(int color1, int color2, int color3, int color4) {
    store_default_text_colors(0, color1, color2, color3, color4);
}

/*
Purpose: Set the secondary default text colours shared by every button.
ORIGINAL: 0x00607450
// name      ?set_def_text_color2@BaseButton@@QAAXHHHH@Z
// size      39 bytes
// spans     0x00607450-0x00607477
// prototype 
// callers   0   call targets   0
Status: Complete
*/
void BaseButton::set_def_text_color2(int color1, int color2, int color3, int color4) {
    store_default_text_colors(1, color1, color2, color3, color4);
}

/*
Purpose: Set the tertiary default text colours shared by every button.
ORIGINAL: 0x00607480
// name      ?set_def_text_color3@BaseButton@@QAAXHHHH@Z
// size      39 bytes
// spans     0x00607480-0x006074A7
// prototype 
// callers   0   call targets   0
Status: Complete
*/
void BaseButton::set_def_text_color3(int color1, int color2, int color3, int color4) {
    store_default_text_colors(2, color1, color2, color3, color4);
}

/*
Purpose: Set the default fonts shared by every button.
ORIGINAL: 0x006074B0
// name      ?set_def_font@BaseButton@@QAAHPAUFont@@PAUFont@@PAUFont@@@Z
// size      48 bytes
// spans     0x006074B0-0x006074E0
// prototype 
// callers   4   call targets   0
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
ORIGINAL: 0x00607360
// name      ?set_text_color@BaseButton@@QAEXHHHH@Z
// size      61 bytes
// spans     0x00607360-0x0060739D
// prototype void (__thiscall ?set_text_color@BaseButton@@QAEXHHHH@Z)(BaseButton* this, int, int, int, int)
// callers   11   call targets   2
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
ORIGINAL: 0x006073A0
// name      ?set_text_color2@BaseButton@@QAEXHHHH@Z
// size      61 bytes
// spans     0x006073A0-0x006073DD
// prototype void (__thiscall ?set_text_color2@BaseButton@@QAEXHHHH@Z)(BaseButton* this, int, int, int, int)
// callers   1   call targets   2
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
ORIGINAL: 0x006073E0
// name      ?set_text_color3@BaseButton@@QAEXHHHH@Z
// size      61 bytes
// spans     0x006073E0-0x0060741D
// prototype void (__thiscall ?set_text_color3@BaseButton@@QAEXHHHH@Z)(BaseButton* this, int, int, int, int)
// callers   1   call targets   2
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

typedef void (OriginalObject::*func_button_refresh_slot)();
typedef void (OriginalObject::*func_parent_notify_slot)(int, int);

// Both dispatches read the live vtable of whatever object is actually there
// rather than going through a C++ virtual call, so no table this toolchain
// lays out can disagree with the original's.
constexpr size_t BaseButtonRefreshSlot = 0xF8;
constexpr size_t WinValueChangedSlot = 0xB4;

}  // namespace

/*
Purpose: Give the button a new value, redrawing and notifying its parent only
         when the value actually changes.
ORIGINAL: 0x00607C80
// name      ?set@BaseButton@@QAEXH@Z
// size      61 bytes
// spans     0x00607C80-0x00607CBD
// prototype void (__thiscall ?set@BaseButton@@QAEXH@Z)(BaseButton* this, int)
// callers   7   call targets   0
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
    (ORIGINAL(this)->*original_method<func_button_refresh_slot>(reinterpret_cast<unsigned long>(own_vtable[BaseButtonRefreshSlot / sizeof(void *)])))();

    if (!win_parent_) {
        return;
    }
    void **const parent_vtable = *reinterpret_cast<void ***>(win_parent_);
    (ORIGINAL(win_parent_)->*original_method<func_parent_notify_slot>(reinterpret_cast<unsigned long>(parent_vtable[WinValueChangedSlot / sizeof(void *)])))(static_cast<int>(field_A78_), value);
}

void __fastcall base_button_set_redirect(BaseButton *self, void *, int value) {
    self->set(value);
}

/*
Purpose: Legacy stub; the original body returns nothing without reading its
ORIGINAL: 0x006077F0 BYTE_EXACT
// name      ?on_key_click@BaseButton@@QAEHHH@Z
// size      5 bytes
// spans     0x006077F0-0x006077F5
// prototype void (__thiscall ?on_key_click@BaseButton@@QAEXHH@Z)(BaseButton* this, int, int)
// callers   0   call targets   0
Return Value: zero - the base class handles no key click
Status: Complete
*/
int BaseButton::on_key_click(int a, int b) {
    return 0;
}

void __fastcall base_button_on_key_click_redirect(
        BaseButton *self, void *, int a, int b) {
    self->on_key_click(a, b);
}

/*
Purpose: Legacy stub; the original body returns nothing without reading its
ORIGINAL: 0x00607800 BYTE_EXACT
// name      ?on_key_down@BaseButton@@QAEHH@Z
// size      5 bytes
// spans     0x00607800-0x00607805
// prototype void (__thiscall ?on_key_down@BaseButton@@QAEXH@Z)(BaseButton* this, int)
// callers   0   call targets   0
Return Value: zero - the base class handles no key down
Status: Complete
*/
int BaseButton::on_key_down(int a) {
    return 0;
}

void __fastcall base_button_on_key_down_redirect(
        BaseButton *self, void *, int a) {
    self->on_key_down(a);
}

/*
Purpose: Legacy stub; the original body returns nothing without reading its
ORIGINAL: 0x00607810 BYTE_EXACT
// name      ?on_key_up@BaseButton@@QAEHH@Z
// size      5 bytes
// spans     0x00607810-0x00607815
// prototype void (__thiscall ?on_key_up@BaseButton@@QAEXH@Z)(BaseButton* this, int)
// callers   0   call targets   0
Return Value: zero - the base class handles no key up
Status: Complete
*/
int BaseButton::on_key_up(int a) {
    return 0;
}

void __fastcall base_button_on_key_up_redirect(
        BaseButton *self, void *, int a) {
    self->on_key_up(a);
}

namespace {

typedef uint32_t (OriginalObject::*func_button_init_close_slot)();
typedef void (OriginalObject::*func_button_init_show_slot)(int);

// Both dispatches read the live vtable of whatever object is actually there
// rather than going through a C++ virtual call, so a FlatButton entering this
// body closes and shows through its own overrides. In the BaseButton table
// slot 0x168 holds BaseButton::close and slot 0x04 holds Win::show, which
// takes one int.
constexpr size_t BaseButtonInitCloseSlot = 0x168;
constexpr size_t BaseButtonInitShowSlot = 0x04;

}  // namespace

/*
Purpose: Reinitialise a button - close whatever it currently holds, take a
         private copy of its name, build the GraphicWin base with the button
         style word, publish the shared default colours and fonts into the
         window buffer, then show it.
ORIGINAL: 0x00607210
// name      ?init@BaseButton@@QAEHPADHHHHHPAUWin@@H@Z
// size      322 bytes
// spans     0x00607210-0x00607352
// prototype int (__thiscall ?init@BaseButton@@QAEHPADHHHHHPAUWin@@H@Z)(BaseButton* this, int8*, int, int, int, int, int, Win*, int)
// callers   30   call targets   8
Return Value: No parent (3); name allocation failed (4); otherwise
              GraphicWin::init's own code, which is zero on success
Status: Complete
Verification note: the `return 4` arm is unreachable through the recovered
         mem_get_old, which pops a message box and calls exit(4) rather than
         answering null, exactly as the original 0x005D4510 does. The
         store-then-test order is transcribed from the bytes - 0x00607248
         stores, 0x0060724E tests - but no leaf fixture can drive that arm
         without replacing mem_get_old itself, so it is unobserved by
         construction rather than by omission.
Verification note: mutating BaseButtonInitShowSlot from 0x04 to 0x05 folds to
         the same `/ sizeof(void *)` index, so that mutant is equivalent by
         construction - the same known-noise class as WinValueChangedSlot.
Verification note: the eight arguments do NOT map straight through to
         GraphicWin::init. The name is not forwarded at all - a literal null
         goes into the caption slot at 0x00607296, and the Menu and
         BorderSizing slots take literal nulls at 0x00607282 and 0x0060727E -
         while the four geometry arguments arrive as arguments 3 to 6 of this
         function, not 1 to 4.
*/
int BaseButton::init(LPCSTR name, int id, int x, int y, int width, int height,
                     Win *parent, int style_flag) {
    // The close runs before every guard, so even a rejected init leaves the
    // button torn down. The original dispatches straight through `this` and
    // its vtable at 0x00607215 with no null check on either; none is added.
    void **const close_vtable = *reinterpret_cast<void ***>(this);
    (ORIGINAL(this)->*original_method<func_button_init_close_slot>(reinterpret_cast<unsigned long>(close_vtable[BaseButtonInitCloseSlot / sizeof(void *)])))();

    // Both remaining arguments are read after the close, not cached across it.
    if (!parent) {
        return 3;
    }

    if (name) {
        const size_t len = strlen(name) + 1;
        // name_ is overwritten without being freed first. The slot-0x168 close
        // above is what released and zeroed the previous string, and the
        // original relies on that instead of repeating set_name's guard.
        LPSTR const allocated = static_cast<LPSTR>(mem_get_old(len));
        name_ = allocated;
        if (!allocated) {
            return 4;
        }
        // The original empties the buffer and concatenates rather than
        // copying, the same shape set_name above uses. The emptying goes
        // through the allocation register at 0x0060725D, but the
        // concatenation destination is RE-READ from 0xA7C at 0x00607260 - do
        // not collapse the two into one local.
        allocated[0] = '\0';
        strcat_s(name_, len, name);
    }

    // neg/sbb/and/or at 0x00607278-0x0060728B: `neg edx` sets the carry from
    // style_flag != 0 and `sbb edx, edx` broadcasts it, so any nonzero flag
    // contributes 0x8000 and nothing else - the value is normalised to one
    // bit rather than masked in.
    const int style = 0x01000320 | (style_flag ? 0x8000 : 0);
    const int status = GraphicWin::init(
        x, y, width, height, nullptr, style, parent, nullptr, nullptr);
    if (status) {
        return status;
    }

    field_A78_ = static_cast<uint32_t>(id);

    // Slot s tier t of the shared default table lives at
    // 0x00697060 + s * 0xC + t * 4, so tier 0 reads 0x00697060, 0x0069706C,
    // 0x00697078 and 0x00697084. The table is read here rather than cached at
    // construction, so a set_def_text_color issued between construct and init
    // is the one that lands.
    volatile const uint32_t *const colors = BaseButtonDefaultTextColors;
    buffer_.set_text_color(
        static_cast<int>(colors[0]), static_cast<int>(colors[3]),
        static_cast<int>(colors[6]), static_cast<int>(colors[9]));
    buffer_.set_text_color2(
        static_cast<int>(colors[1]), static_cast<int>(colors[4]),
        static_cast<int>(colors[7]), static_cast<int>(colors[10]));
    buffer_.set_text_color3(
        static_cast<int>(colors[2]), static_cast<int>(colors[5]),
        static_cast<int>(colors[8]), static_cast<int>(colors[11]));

    // The fourth font slot is a literal null pushed at 0x00607335, not a
    // fourth default. set_font's result is discarded, so an uninitialised
    // primary font - its code 3 - does not fail init.
    Font **const fonts = BaseButtonDefaultFonts;
    buffer_.set_font(fonts[0], fonts[1], fonts[2], nullptr);

    // Re-read, do NOT hoist onto the close dispatch above: the original
    // reloads the vtable pointer at 0x00607341, so a GraphicWin::init or a
    // Buffer setter that restaged the table is seen by the show.
    void **const show_vtable = *reinterpret_cast<void ***>(this);
    (ORIGINAL(this)->*original_method<func_button_init_show_slot>(reinterpret_cast<unsigned long>(show_vtable[BaseButtonInitShowSlot / sizeof(void *)])))(0);
    return 0;
}

int __fastcall base_button_init_redirect(
        BaseButton *self, void *, LPCSTR name, int id, int x, int y,
        int width, int height, Win *parent, int style_flag) {
    return self->init(name, id, x, y, width, height, parent, style_flag);
}

/*
ORIGINAL: 0x00607A00 BYTE_EXACT
// name      ?on_right_click@BaseButton@@QAEXHH@Z
// size      20 bytes
// spans     0x00607A00-0x00607A14
// prototype void (__thiscall ?on_right_click@BaseButton@@QAEXHH@Z)(BaseButton* this, int, int)
// callers   0   call targets   1
Status: Complete
*/
void BaseButton::on_right_click(int a1, int a2) {
    char *self = reinterpret_cast<char *>(this);
    int v = *reinterpret_cast<int *>(self + 0xa80);
    if (v) {
        timer_callback(0);
    }
}

/*
ORIGINAL: 0x00607B10 BYTE_EXACT
// name      ?timer_callback_daemon@BaseButton@@QAAXHH@Z
// size      19 bytes
// spans     0x00607B10-0x00607B23
// prototype 
// callers   0   call targets   1
Status: Complete
*/
void __cdecl fn_00607b10(int a1, BaseButton* a2) {
    if (a2) {
        a2->timer_callback(a1);
    }
}

func_base_button_timer_callback BaseButtonTimerCallback =
    original_method<func_base_button_timer_callback>(0x00607B30);

/*
Purpose: Run the button's timer tick; on_right_click and the daemon
         fn_00607b10 above are its two callers.
         Body unrecovered; forwards to the original image.
Forwards To: 00607B30
Return Value: n/a
Status: Forwarder
*/
void BaseButton::timer_callback(int a1) {
    (ORIGINAL(this)->*BaseButtonTimerCallback)(a1);
}
