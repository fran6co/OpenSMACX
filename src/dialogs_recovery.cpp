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
#include <cstring>
#include "dialogs.h"
#include "listbox.h"      // ListBox::destroy (source-owned)
#include "radiobutton.h"  // RadioButton::close (source-owned)

func_dialog_item DialogOriginalItem = original_method<func_dialog_item>(0x00609990);
func_list_box_item ListBoxOriginalItem = original_method<func_list_box_item>(0x0060C920);

namespace {

// The jump table indexes on the kind minus one, and anything outside one to
// sixteen falls through to the default arm.
bool kind_in_range(int32_t kind) {
    return static_cast<uint32_t>(kind - 1) <= 0xF;
}

// Where the Dialog subobject is, according to this object's own vbtable.
Dialog *dialog_of(void *self) {
    auto *const bytes = reinterpret_cast<uint8_t *>(self);
    const int32_t *const vbtable = *reinterpret_cast<const int32_t *const *>(bytes);
    return reinterpret_cast<Dialog *>(bytes + vbtable[2]);
}

}  // namespace

/*
Purpose: Fetch one item's text, from whichever widget this kind of dialog
         keeps its items in.
Original Offset: 00612A70
Return Value: the widget's result, zero for kinds that hold no items
Status: Complete
*/
int Dialogs::item(char *text, int index) {
    if (!kind_in_range(kind_)) {
        return 0;
    }
    switch (kind_) {
        case 1:
        case 4:
        case 8:
        case 16:
            return (ORIGINAL(dialog_of(this))->*DialogOriginalItem)(text, index);
        case 2:
            return (ORIGINAL(this)->*ListBoxOriginalItem)(text, index);
        default:
            return 0;
    }
}

/*
Purpose: Count the items this kind of dialog holds.
Original Offset: 00613740
Return Value: the count, zero for kinds that hold no items
Status: Complete
*/
int Dialogs::get_num_items() {
    if (!kind_in_range(kind_)) {
        return 0;
    }
    switch (kind_) {
        case 1:
        case 2:
        case 4:
        case 16: {
            int32_t count;
            std::memcpy(&count,
                        reinterpret_cast<uint8_t *>(dialog_of(this)) + 0xCC,
                        sizeof(count));
            return count;
        }
        case 8:
            return field_A4_;
        default:
            return 0;
    }
}

int __fastcall dialogs_item_redirect(Dialogs *self, void *, char *text, int index) {
    return self->item(text, index);
}

int __fastcall dialogs_get_num_items_redirect(Dialogs *self, void *) {
    return self->get_num_items();
}

func_dialogs_fwd2 DialogsSpriteBoxOnRightDown = original_method<func_dialogs_fwd2>(0x00611240);
func_dialogs_fwd2 DialogsSpriteBoxOnRightDoubleClick = original_method<func_dialogs_fwd2>(0x00611330);
func_dialogs_fwd2 DialogsSpriteBoxOnLeftUp = original_method<func_dialogs_fwd2>(0x006111A0);
func_dialogs_fwd2 DialogsSpriteBoxOnRightUp = original_method<func_dialogs_fwd2>(0x00611290);
func_dialogs_fwd2 DialogsSpriteBoxOnRightClick = original_method<func_dialogs_fwd2>(0x006111F0);
func_dialogs_fwd2 DialogsListBoxOnScrolling = original_method<func_dialogs_fwd2>(0x0060C5D0);
func_dialogs_fwd1 DialogsListBoxOnMousewheel = original_method<func_dialogs_fwd1>(0x0060CB70);

/*
Purpose: Forward on right down to the embedded widget, but only when the
         active dialog is the sprite-box kind (8). `this` arrives at the interface subobject, so
         the discriminator sits 8 bytes before it and the target is reached by
         adjusting back 0x8C.
Original Offset: 00612ED0
Return Value: n/a
Status: Complete
*/
void Dialogs::on_right_down(int a1, int a2) {
    auto *const bytes = reinterpret_cast<uint8_t *>(this);
    int discriminator;
    std::memcpy(&discriminator, bytes - 8, sizeof(discriminator));
    if (discriminator == 8) {
        (ORIGINAL(bytes - 0x8C)->*DialogsSpriteBoxOnRightDown)(a1, a2);
    }
}

/*
Purpose: Forward on right double click to the embedded widget, but only when the
         active dialog is the sprite-box kind (8). `this` arrives at the interface subobject, so
         the discriminator sits 8 bytes before it and the target is reached by
         adjusting back 0x8C.
Original Offset: 00612EF0
Return Value: n/a
Status: Complete
*/
void Dialogs::on_right_double_click(int a1, int a2) {
    auto *const bytes = reinterpret_cast<uint8_t *>(this);
    int discriminator;
    std::memcpy(&discriminator, bytes - 8, sizeof(discriminator));
    if (discriminator == 8) {
        (ORIGINAL(bytes - 0x8C)->*DialogsSpriteBoxOnRightDoubleClick)(a1, a2);
    }
}

/*
Purpose: Forward on left up to the embedded widget, but only when the
         active dialog is the sprite-box kind (8). `this` arrives at the interface subobject, so
         the discriminator sits 8 bytes before it and the target is reached by
         adjusting back 0x8C.
Original Offset: 00612F10
Return Value: n/a
Status: Complete
*/
void Dialogs::on_left_up(int a1, int a2) {
    auto *const bytes = reinterpret_cast<uint8_t *>(this);
    int discriminator;
    std::memcpy(&discriminator, bytes - 8, sizeof(discriminator));
    if (discriminator == 8) {
        (ORIGINAL(bytes - 0x8C)->*DialogsSpriteBoxOnLeftUp)(a1, a2);
    }
}

/*
Purpose: Forward on right up to the embedded widget, but only when the
         active dialog is the sprite-box kind (8). `this` arrives at the interface subobject, so
         the discriminator sits 8 bytes before it and the target is reached by
         adjusting back 0x8C.
Original Offset: 00612F40
Return Value: n/a
Status: Complete
*/
void Dialogs::on_right_up(int a1, int a2) {
    auto *const bytes = reinterpret_cast<uint8_t *>(this);
    int discriminator;
    std::memcpy(&discriminator, bytes - 8, sizeof(discriminator));
    if (discriminator == 8) {
        (ORIGINAL(bytes - 0x8C)->*DialogsSpriteBoxOnRightUp)(a1, a2);
    }
}

/*
Purpose: Forward on right click to the embedded widget, but only when the
         active dialog is the sprite-box kind (8). `this` arrives at the interface subobject, so
         the discriminator sits 8 bytes before it and the target is reached by
         adjusting back 0x8C.
Original Offset: 00612F60
Return Value: n/a
Status: Complete
*/
void Dialogs::on_right_click(int a1, int a2) {
    auto *const bytes = reinterpret_cast<uint8_t *>(this);
    int discriminator;
    std::memcpy(&discriminator, bytes - 8, sizeof(discriminator));
    if (discriminator == 8) {
        (ORIGINAL(bytes - 0x8C)->*DialogsSpriteBoxOnRightClick)(a1, a2);
    }
}

/*
Purpose: Forward on scrolled to the embedded widget, but only when the
         active dialog is the list-box kind (2). `this` arrives at the interface subobject, so
         the discriminator sits 8 bytes before it and the target is reached by
         adjusting back 0x140.
Original Offset: 00612F80
Return Value: n/a
Status: Complete
*/
void Dialogs::on_scrolled(int a1, int a2) {
    auto *const bytes = reinterpret_cast<uint8_t *>(this);
    int discriminator;
    std::memcpy(&discriminator, bytes - 8, sizeof(discriminator));
    if (discriminator == 2) {
        (ORIGINAL(bytes - 0x140)->*DialogsListBoxOnScrolling)(a1, a2);
    }
}

/*
Purpose: Forward on scrolling to the embedded widget, but only when the
         active dialog is the list-box kind (2). `this` arrives at the interface subobject, so
         the discriminator sits 8 bytes before it and the target is reached by
         adjusting back 0x140.
Original Offset: 00612FA0
Return Value: n/a
Status: Complete
*/
void Dialogs::on_scrolling(int a1, int a2) {
    auto *const bytes = reinterpret_cast<uint8_t *>(this);
    int discriminator;
    std::memcpy(&discriminator, bytes - 8, sizeof(discriminator));
    if (discriminator == 2) {
        (ORIGINAL(bytes - 0x140)->*DialogsListBoxOnScrolling)(a1, a2);
    }
}

/*
Purpose: Forward on mousewheel to the embedded widget, but only when the
         active dialog is the list-box kind (2). `this` arrives at the interface subobject, so
         the discriminator sits 8 bytes before it and the target is reached by
         adjusting back 0x140.
Original Offset: 00612FC0
Return Value: n/a
Status: Complete
*/
void Dialogs::on_mousewheel(int a1) {
    auto *const bytes = reinterpret_cast<uint8_t *>(this);
    int discriminator;
    std::memcpy(&discriminator, bytes - 8, sizeof(discriminator));
    if (discriminator == 2) {
        (ORIGINAL(bytes - 0x140)->*DialogsListBoxOnMousewheel)(a1);
    }
}

void __fastcall dialogs_on_right_down_redirect(Dialogs *self, void *, int a1, int a2) {
    self->on_right_down(a1, a2);
}

void __fastcall dialogs_on_right_double_click_redirect(Dialogs *self, void *, int a1, int a2) {
    self->on_right_double_click(a1, a2);
}

void __fastcall dialogs_on_left_up_redirect(Dialogs *self, void *, int a1, int a2) {
    self->on_left_up(a1, a2);
}

void __fastcall dialogs_on_right_up_redirect(Dialogs *self, void *, int a1, int a2) {
    self->on_right_up(a1, a2);
}

void __fastcall dialogs_on_right_click_redirect(Dialogs *self, void *, int a1, int a2) {
    self->on_right_click(a1, a2);
}

void __fastcall dialogs_on_scrolled_redirect(Dialogs *self, void *, int a1, int a2) {
    self->on_scrolled(a1, a2);
}

void __fastcall dialogs_on_scrolling_redirect(Dialogs *self, void *, int a1, int a2) {
    self->on_scrolling(a1, a2);
}

void __fastcall dialogs_on_mousewheel_redirect(Dialogs *self, void *, int a1) {
    self->on_mousewheel(a1);
}

func_dialogs_teardown DialogsEditGroupDestructor =
    original_method<func_dialogs_teardown>(0x00611A20);
func_dialogs_teardown DialogsSpriteBoxDestructor =
    original_method<func_dialogs_teardown>(0x00610120);
func_dialogs_teardown DialogsCheckBoxDestructor =
    original_method<func_dialogs_teardown>(0x0060E740);
func_operator_delete *DialogsOperatorDelete =
    (func_operator_delete *)0x0064557F;

const uint32_t DialogsVbaseGraphicWinVtable = 0x00669BE8;
const uint32_t DialogsVbaseBufferVtable = 0x00669BE0;
const uint32_t DialogsVbaseWinVtable = 0x00669BD4;
const uint32_t DialogsRadioPrimaryVtable = 0x00669A6C;
const uint32_t DialogsRadioBufferVtable = 0x00669A64;
const uint32_t DialogsRadioWinVtable = 0x00669A58;

namespace {

// Stage a virtually-derived subobject's three tables plus its two vbase-adjust
// words, every slot located through THAT subobject's own vbtable at run time
// (the RadioButton-at-0x44 hazard: an embedded vbtable names different offsets
// than a most-derived one, and hardcoding aims every store at the wrong
// address). The buffer table always sits 0x444 past the primary; each adjust
// word, four bytes below its slot, records the live entry minus the class's
// own most-derived entry.
void stage_virtual_tables(uint8_t *subobject, uint32_t primary_vtable,
                          uint32_t buffer_vtable, uint32_t win_vtable,
                          int32_t primary_own_offset, int32_t win_own_offset) {
    const int32_t *const vbtable =
        *reinterpret_cast<const int32_t *const *>(subobject);
    const int32_t primary = vbtable[1];
    const int32_t win = vbtable[2];
    *reinterpret_cast<volatile uint32_t *>(subobject + primary) = primary_vtable;
    *reinterpret_cast<volatile uint32_t *>(subobject + primary + 0x444) =
        buffer_vtable;
    *reinterpret_cast<volatile uint32_t *>(subobject + win) = win_vtable;
    *reinterpret_cast<volatile int32_t *>(subobject + primary - 4) =
        primary - primary_own_offset;
    *reinterpret_cast<volatile int32_t *>(subobject + win - 4) =
        win - win_own_offset;
}

}  // namespace

/*
Purpose: Destroy a Dialogs. Stage the Dialogs tables into the GraphicWin/Win
         virtual base through that base's own vbtable, run the empty close,
         destroy the EditGroup, SpriteBox and CheckBox members through their
         original destructors, stage the embedded RadioButton's tables through
         ITS own vbtable and run its recovered close, then run the embedded
         ListBox's recovered teardown - whose base coincides with the
         allocation base, so each staging overwrites the previous tables in
         the original's exact order. The original's C++ exception frame
         targets __CxxFrameHandler and is omitted as unreachable per policy.
Original Offset: 00406910
Return Value: EAX residue (ListBox::destroy's constant 0); declared void in
              the mangled name, modelled as uint32_t like ListBox::destroy.
Status: Complete with temporary EditGroup, SpriteBox and CheckBox destructor
        dependencies
Verification note: every member teardown is guarded on the allocation base
being non-null (the original tests edi after `lea edi,[esi-0x188]`); the null
branch feeds the bare adjustment offset and is unreachable for any real
object, so no test drives it, and the sweep's surviving constant mutants in
that branch are equivalent by unreachability. The surviving drop of the
close() call is equivalent by construction: Dialogs::close is an empty body.
*/
uint32_t Dialogs::destroy() {
    uint8_t *const base = reinterpret_cast<uint8_t *>(this);

    // Dialogs' own vbtable sits at the allocation base; the 0x188/0xBA0
    // subtrahends are its most-derived entries, baked in by the original.
    stage_virtual_tables(base, DialogsVbaseGraphicWinVtable,
                         DialogsVbaseBufferVtable, DialogsVbaseWinVtable,
                         0x188, 0xBA0);
    close();

    // The three widget members, each entered at its fixed adjustment past the
    // member base, guarded as the original guards them. Integer arithmetic so
    // the unreachable null branch stays exactly the original's `0 + adjust`.
    const uintptr_t guard =
        base != nullptr ? reinterpret_cast<uintptr_t>(base) : 0;
    const auto guarded = [guard](uintptr_t member, uintptr_t adjust) {
        return reinterpret_cast<void *>((guard ? guard + member : 0) + adjust);
    };
    (ORIGINAL(guarded(0xF8, 0x8C))->*DialogsEditGroupDestructor)();
    (ORIGINAL(guarded(0x70, 0x8C))->*DialogsSpriteBoxDestructor)();
    (ORIGINAL(guarded(0x58, 0x1C))->*DialogsCheckBoxDestructor)();

    // The embedded RadioButton at base+0x44: its vbtable names the SHARED
    // virtual bases, so this staging overwrites the step-one tables, and its
    // recovered close walks them through the same vbtable.
    auto *const radio = static_cast<uint8_t *>(guarded(0x44, 0));
    stage_virtual_tables(radio, DialogsRadioPrimaryVtable,
                         DialogsRadioBufferVtable, DialogsRadioWinVtable,
                         0x18, 0xA30);
    reinterpret_cast<RadioButton *>(radio)->close();

    // The embedded ListBox's base coincides with the allocation base; its
    // recovered destroy stages the ListBox tables - overwriting once more -
    // and closes both shared bases. Unguarded, as the original left it.
    return reinterpret_cast<ListBox *>(base)->destroy();
}

// ~Dialogs is entered with this = B + 0x188; recover the base first.
uint32_t __fastcall dialogs_destructor_redirect(void *adjusted, void *) {
    auto *self = reinterpret_cast<Dialogs *>(
        static_cast<uint8_t *>(adjusted) - DialogsDestructorAdjustment);
    return self->destroy();
}

/*
Purpose: The compiler-generated scalar deleting destructor: run ~Dialogs, the
         trailing Dialog's recovered destructor, and the GraphicWin virtual
         base's recovered destructor, then free the allocation base through
         the executable's operator delete only when bit 0 of the mode asks.
         Always returns the allocation base (this - 0x188), the original's
         EAX. The fixed +0xBA0/+0x188 subobject offsets are complete-object
         facts: this thunk is only ever entered on a most-derived Dialogs.
Original Offset: 00407100
Return Value: the allocation base
Status: Complete
*/
void *__fastcall dialogs_scalar_dtor_redirect(void *adjusted, void *,
                                              unsigned int mode) {
    uint8_t *const base =
        static_cast<uint8_t *>(adjusted) - DialogsDestructorAdjustment;
    reinterpret_cast<Dialogs *>(base)->destroy();
    reinterpret_cast<Dialog *>(base + 0xBA0)->destroy();
    graphic_win_destructor_redirect(
        reinterpret_cast<GraphicWin *>(base + 0x188), nullptr);
    if (mode & 1) {
        DialogsOperatorDelete(base);
    }
    return base;
}
