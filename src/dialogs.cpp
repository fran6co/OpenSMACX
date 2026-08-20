/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */
#include "stdafx.h"
#include "dialogs.h"
#include "listbox.h"      // ListBox::destroy (source-owned)
#include "spritebox.h"    // the SpriteBox handlers this dispatches to
#include "original_seam.h"
#include "radiobutton.h"  // RadioButton::close (source-owned)
#include <cstring>

/*
Purpose: Close the dialog manager; the legacy implementation is intentionally empty.
// ORIGINAL: 0x00612960 ?close@Dialogs@@QAEXXZ 0x00612960-0x00612961 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?close@Dialogs@@QAEXXZ)(Dialogs* this)
// callers   20   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: n/a
Status: Complete
*/
void Dialogs::close() {
}

void __fastcall dialogs_close_redirect(Dialogs *self, void *) {
    self->close();
}

// ---------------------------------------------------------------------------
// Merged from the retired src/dialogs_recovery.cpp on 2026-08-20, following
// src/text_recovery.cpp, which went into text.cpp on 2026-08-15. A class's
// bodies in two translation units is not a neutral choice: which unit a
// piece compiles in decides what can be inlined into it.
// ---------------------------------------------------------------------------


namespace {


// Where the Dialog subobject is, according to this object's own vbtable.
Dialog *dialog_of(void *self) {
    uint8_t *const bytes = reinterpret_cast<uint8_t *>(self);
    const int32_t *const vbtable = *reinterpret_cast<const int32_t *const *>(bytes);
    return reinterpret_cast<Dialog *>(bytes + vbtable[2]);
}

}  // namespace

/*
Purpose: Fetch one item's text, from whichever widget this kind of dialog
         keeps its items in.
// ORIGINAL: 0x00612A70 ?item@Dialogs@@QAEHPADH@Z 0x00612A70-0x00612ABF
// size      79 bytes
// prototype int (__thiscall ?item@Dialogs@@QAEHPADH@Z)(Dialogs* this, int8* lpString, int position)
// callers   72   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00609990 0x0060C920
Return Value: the widget's result, zero for kinds that hold no items
Status: Complete
*/
int Dialogs::item(char *text, int index) {
    switch (kind_) {
        case 1:
        case 4:
        case 8:
        case 16:
            return dialog_of(this)->item(text, index);
        case 2:
            // THE RECEIVER IS THIS OBJECT, reinterpreted: the image calls
            // ListBox::item with the Dialogs pointer unchanged, and
            // `class Dialogs : ListBox` cannot be spelled here - see the
            // measured sizes in dialogs.h.
            return reinterpret_cast<ListBox *>(this)->item(text, index);
        default:
            return 0;
    }
}

/*
Purpose: Count the items this kind of dialog holds.
// ORIGINAL: 0x00613740 ?get_num_items@Dialogs@@QAEHXZ 0x00613740-0x00613772
// size      50 bytes
// prototype int (__thiscall ?get_num_items@Dialogs@@QAEHXZ)(Dialogs* this)
// callers   4   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the count, zero for kinds that hold no items
Status: Complete
*/
int Dialogs::get_num_items() {
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


/*
Purpose: Forward on right down to the embedded widget, but only when the
         active dialog is the sprite-box kind (8). `this` arrives at the interface subobject, so
         the discriminator sits 8 bytes before it and the target is reached by
         adjusting back 0x8C.
// ORIGINAL: 0x00612ED0 ?on_right_down@Dialogs@@QAEXHH@Z 0x00612ED0-0x00612EEE BYTE_EXACT
// size      30 bytes
// prototype void (__thiscall ?on_right_down@Dialogs@@QAEXHH@Z)(Dialogs* this, int xCoord, int yCoord)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00611240
Return Value: n/a
Status: Complete
*/
void Dialogs::on_right_down(int a1, int a2) {
    uint8_t *const bytes = reinterpret_cast<uint8_t *>(this);
    int discriminator;
    std::memcpy(&discriminator, bytes - 8, sizeof(discriminator));
    if (discriminator == 8) {
        reinterpret_cast<SpriteBox *>(bytes - 0x8C)->on_right_down(a1, a2);
    }
}

/*
Purpose: Forward on right double click to the embedded widget, but only when the
         active dialog is the sprite-box kind (8). `this` arrives at the interface subobject, so
         the discriminator sits 8 bytes before it and the target is reached by
         adjusting back 0x8C.
// ORIGINAL: 0x00612EF0 ?on_right_double_click@Dialogs@@QAEXHH@Z 0x00612EF0-0x00612F0E BYTE_EXACT
// size      30 bytes
// prototype void (__thiscall ?on_right_double_click@Dialogs@@QAEXHH@Z)(Dialogs* this, int xCoord, int yCoord)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00611330
Return Value: n/a
Status: Complete
*/
void Dialogs::on_right_double_click(int a1, int a2) {
    uint8_t *const bytes = reinterpret_cast<uint8_t *>(this);
    int discriminator;
    std::memcpy(&discriminator, bytes - 8, sizeof(discriminator));
    if (discriminator == 8) {
        reinterpret_cast<SpriteBox *>(bytes - 0x8C)->on_right_double_click(a1, a2);
    }
}

/*
Purpose: Forward on left up to the embedded widget, but only when the
         active dialog is the sprite-box kind (8). `this` arrives at the interface subobject, so
         the discriminator sits 8 bytes before it and the target is reached by
         adjusting back 0x8C.
// ORIGINAL: 0x00612F10 ?on_left_up@Dialogs@@QAEXHH@Z 0x00612F10-0x00612F35 BYTE_EXACT
// size      37 bytes
// prototype void (__thiscall ?on_left_up@Dialogs@@QAEXHH@Z)(Dialogs* this, int xCoord, int yCoord)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006111A0
Return Value: n/a
Status: Complete
*/
void Dialogs::on_left_up(int a1, int a2) {
    uint8_t *const bytes = reinterpret_cast<uint8_t *>(this);
    int discriminator;
    std::memcpy(&discriminator, bytes - 8, sizeof(discriminator));
    // A DEAD TEST FOR KIND 4, and it is in the shipped bytes. Both jumps go
    // to the SAME place - 0x00612F32, past the call - so `cmp eax, 4; je`
    // excludes a kind that `cmp eax, 8; jne` would have excluded anyway.
    // Reproduced because it is three instructions of the original; it
    // changes no behaviour, which is exactly why it survived.
    if (discriminator == 4) {
        return;
    }
    if (discriminator == 8) {
        reinterpret_cast<SpriteBox *>(bytes - 0x8C)->on_left_up(a1, a2);
    }
}

/*
Purpose: Forward on right up to the embedded widget, but only when the
         active dialog is the sprite-box kind (8). `this` arrives at the interface subobject, so
         the discriminator sits 8 bytes before it and the target is reached by
         adjusting back 0x8C.
// ORIGINAL: 0x00612F40 ?on_right_up@Dialogs@@QAEXHH@Z 0x00612F40-0x00612F5E BYTE_EXACT
// size      30 bytes
// prototype void (__thiscall ?on_right_up@Dialogs@@QAEXHH@Z)(Dialogs* this, int xCoord, int yCoord)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00611290
Return Value: n/a
Status: Complete
*/
void Dialogs::on_right_up(int a1, int a2) {
    uint8_t *const bytes = reinterpret_cast<uint8_t *>(this);
    int discriminator;
    std::memcpy(&discriminator, bytes - 8, sizeof(discriminator));
    if (discriminator == 8) {
        reinterpret_cast<SpriteBox *>(bytes - 0x8C)->on_right_up(a1, a2);
    }
}

/*
Purpose: Forward on right click to the embedded widget, but only when the
         active dialog is the sprite-box kind (8). `this` arrives at the interface subobject, so
         the discriminator sits 8 bytes before it and the target is reached by
         adjusting back 0x8C.
// ORIGINAL: 0x00612F60 ?on_right_click@Dialogs@@QAEXHH@Z 0x00612F60-0x00612F7E BYTE_EXACT
// size      30 bytes
// prototype void (__thiscall ?on_right_click@Dialogs@@QAEXHH@Z)(Dialogs* this, int xCoord, int yCoord)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006111F0
Return Value: n/a
Status: Complete
*/
void Dialogs::on_right_click(int a1, int a2) {
    uint8_t *const bytes = reinterpret_cast<uint8_t *>(this);
    int discriminator;
    std::memcpy(&discriminator, bytes - 8, sizeof(discriminator));
    if (discriminator == 8) {
        reinterpret_cast<SpriteBox *>(bytes - 0x8C)->on_right_click(a1, a2);
    }
}

/*
Purpose: Forward on scrolled to the embedded widget, but only when the
         active dialog is the list-box kind (2). `this` arrives at the interface subobject, so
         the discriminator sits 8 bytes before it and the target is reached by
         adjusting back 0x140.
// ORIGINAL: 0x00612F80 ?on_scrolled@Dialogs@@QAEXHH@Z 0x00612F80-0x00612FA0 BYTE_EXACT
// size      32 bytes
// prototype void (__thiscall ?on_scrolled@Dialogs@@QAEXHH@Z)(Dialogs* this, int, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0060C5D0
Return Value: n/a
Status: Complete
*/
void Dialogs::on_scrolled(int a1, int a2) {
    uint8_t *const bytes = reinterpret_cast<uint8_t *>(this);
    int discriminator;
    std::memcpy(&discriminator, bytes - 8, sizeof(discriminator));
    // A SWITCH, NOT AN `if`: the image is `sub eax, 2; jne`, which is
    // what VC6 emits for a one-case switch. An equality test compiles
    // `cmp dword ptr [ecx-8], 2` and reads the field a second time.
    switch (discriminator) {
        case 2:
            reinterpret_cast<ListBox *>(bytes - 0x140)->on_scrolling(a1, a2);
            break;
        default:
            break;
    }
}

/*
Purpose: Forward on scrolling to the embedded widget, but only when the
         active dialog is the list-box kind (2). `this` arrives at the interface subobject, so
         the discriminator sits 8 bytes before it and the target is reached by
         adjusting back 0x140.
// ORIGINAL: 0x00612FA0 ?on_scrolling@Dialogs@@QAEXHH@Z 0x00612FA0-0x00612FC0 BYTE_EXACT
// size      32 bytes
// prototype void (__thiscall ?on_scrolling@Dialogs@@QAEXHH@Z)(Dialogs* this, int, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0060C5D0
Return Value: n/a
Status: Complete
*/
void Dialogs::on_scrolling(int a1, int a2) {
    uint8_t *const bytes = reinterpret_cast<uint8_t *>(this);
    int discriminator;
    std::memcpy(&discriminator, bytes - 8, sizeof(discriminator));
    // A SWITCH, NOT AN `if`: the image is `sub eax, 2; jne`, which is
    // what VC6 emits for a one-case switch. An equality test compiles
    // `cmp dword ptr [ecx-8], 2` and reads the field a second time.
    switch (discriminator) {
        case 2:
            reinterpret_cast<ListBox *>(bytes - 0x140)->on_scrolling(a1, a2);
            break;
        default:
            break;
    }
}

/*
Purpose: Forward on mousewheel to the embedded widget, but only when the
         active dialog is the list-box kind (2). `this` arrives at the interface subobject, so
         the discriminator sits 8 bytes before it and the target is reached by
         adjusting back 0x140.
// ORIGINAL: 0x00612FC0 ?on_mousewheel@Dialogs@@QAEXH@Z 0x00612FC0-0x00612FDB BYTE_EXACT
// size      27 bytes
// prototype void (__thiscall ?on_mousewheel@Dialogs@@QAEXH@Z)(Dialogs* this, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0060CB70
Return Value: n/a
Status: Complete
*/
void Dialogs::on_mousewheel(int a1) {
    uint8_t *const bytes = reinterpret_cast<uint8_t *>(this);
    int discriminator;
    std::memcpy(&discriminator, bytes - 8, sizeof(discriminator));
    // A SWITCH, NOT AN `if`: the image is `sub eax, 2; jne`, which is
    // what VC6 emits for a one-case switch. An equality test compiles
    // `cmp dword ptr [ecx-8], 2` and reads the field a second time.
    switch (discriminator) {
        case 2:
            reinterpret_cast<ListBox *>(bytes - 0x140)->on_mousewheel(a1);
            break;
        default:
            break;
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

// A member's entry address, guarded the way the original guards it. A plain
// function rather than the lambda this was: VC6 has no lambdas, and the
// capture was one value. Integer arithmetic, so the unreachable null branch
// stays exactly the original's `0 + adjust`.
void *guarded_member(uintptr_t guard, uintptr_t member, uintptr_t adjust) {
    return reinterpret_cast<void *>((guard ? guard + member : 0) + adjust);
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
// ORIGINAL: 0x00406910 ??1Dialogs@@QAE@XZ 0x00406910-0x00406A74;0x006509C0-0x00650AAA
// size      590 bytes
// prototype void (__thiscall ??1Dialogs@@QAE@XZ)(Dialogs* this)
// callers   72   call targets   6
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00609EC0 0x0060D1B0 0x0060E740 0x00610120 0x00611A20 0x00612960
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
    (ORIGINAL(guarded_member(guard, 0xF8, 0x8C))->*DialogsEditGroupDestructor)();
    (ORIGINAL(guarded_member(guard, 0x70, 0x8C))->*DialogsSpriteBoxDestructor)();
    (ORIGINAL(guarded_member(guard, 0x58, 0x1C))->*DialogsCheckBoxDestructor)();

    // The embedded RadioButton at base+0x44: its vbtable names the SHARED
    // virtual bases, so this staging overwrites the step-one tables, and its
    // recovered close walks them through the same vbtable.
    uint8_t *const radio = static_cast<uint8_t *>(guarded_member(guard, 0x44, 0));
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
    Dialogs *self = reinterpret_cast<Dialogs *>(
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
// ORIGINAL: 0x00407100 ??_GDialogs@@UAEPAXI@Z 0x00407100-0x00407141
// symbol    ?dialogs_scalar_dtor_redirect@@YIPAXPAX0I@Z
// size      65 bytes
// prototype void* (__thiscall ??_GDialogs@@UAEPAXI@Z)(Dialogs* this, unsigned int)
// callers   0   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00406910 0x005D4DD0 0x00608E10 0x0064557F
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
