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
#include "worldwin.h"
#include "spritebox.h"
#include "net_class.h"
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


// ---------------------------------------------------------------------------
// Merged from the retired src/dialogs_recovery.cpp on 2026-08-20, following
// src/text_recovery.cpp, which went into text.cpp on 2026-08-15. A class's
// bodies in two translation units is not a neutral choice: which unit a
// piece compiles in decides what can be inlined into it.
// ---------------------------------------------------------------------------


namespace {


// Where the Dialog subobject is, according to this object's own vbtable.
__forceinline Dialog *dialog_of(void *self) {
    uint8_t *const bytes = reinterpret_cast<uint8_t *>(self);
    const int32_t *const vbtable = *reinterpret_cast<const int32_t *const *>(bytes);
    return reinterpret_cast<Dialog *>(bytes + vbtable[2]);
}

}  // namespace

/*
Purpose: Fetch one item's text, from whichever widget this kind of dialog
         keeps its items in.
// ORIGINAL: 0x00612A70 ?item@Dialogs@@QAEHPADH@Z 0x00612A70-0x00612ABF
// LEVER: `case 2` moved AHEAD of the `case 1/4/8/16` group - source case
//        order changed which block VC6 places (and schedules) first; moved
//        12/26 -> 20/26 and removed a spurious internal `call`+`ret 8` the
//        shared-tail block was reached through.
// TRIED: 20/26 plateau on the remaining block - image loads `index`
//            ([esp+8]) before starting the receiver's vtable dereference,
//            pushes it, THEN loads/pushes `text`, interleaved with
//            finishing the receiver. Tried: reversing the 16/8/4/1 case
//            order, and copying `text`/`index` into locals before the call
//            - both score identically to the committed body. VC6
//            call-argument scheduling, not a source-form lever found here.
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
    switch (EditGroup::kind_) {
        case 2:
            // THE RECEIVER IS THIS OBJECT, reinterpreted: the image calls
            // ListBox::item with the Dialogs pointer unchanged, and
            // `class Dialogs : ListBox` cannot be spelled here - see the
            // measured sizes in dialogs.h.
            return reinterpret_cast<ListBox *>(this)->item(text, index);
        case 1:
        case 4:
        case 8:
        case 16:
            return dialog_of(this)->item(text, index);
        default:
            return 0;
    }
}

/*
Purpose: Count the items this kind of dialog holds.
// ORIGINAL: 0x00613740 ?get_num_items@Dialogs@@QAEHXZ 0x00613740-0x00613772 BYTE_EXACT
// LEVER: `dialog_of` marked `__forceinline` - the image has zero call
//        targets here, so its vbtable lookup is inlined, not called.
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
    switch (EditGroup::kind_) {
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
            return SpriteBox::list_.entry_count_;
        default:
            return 0;
    }
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
void Dialogs::on_right_down(int a, int b) {
    uint8_t *const bytes = reinterpret_cast<uint8_t *>(this);
    int discriminator;
    std::memcpy(&discriminator, bytes - 8, sizeof(discriminator));
    if (discriminator == 8) {
        reinterpret_cast<SpriteBox *>(bytes - 0x8C)->on_right_down(a, b);
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
void Dialogs::on_right_double_click(int a, int b) {
    uint8_t *const bytes = reinterpret_cast<uint8_t *>(this);
    int discriminator;
    std::memcpy(&discriminator, bytes - 8, sizeof(discriminator));
    if (discriminator == 8) {
        reinterpret_cast<SpriteBox *>(bytes - 0x8C)->on_right_double_click(a, b);
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
void Dialogs::on_left_up(int a, int b) {
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
        reinterpret_cast<SpriteBox *>(bytes - 0x8C)->on_left_up(a, b);
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
void Dialogs::on_right_up(int a, int b) {
    uint8_t *const bytes = reinterpret_cast<uint8_t *>(this);
    int discriminator;
    std::memcpy(&discriminator, bytes - 8, sizeof(discriminator));
    if (discriminator == 8) {
        reinterpret_cast<SpriteBox *>(bytes - 0x8C)->on_right_up(a, b);
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
void Dialogs::on_right_click(int a, int b) {
    uint8_t *const bytes = reinterpret_cast<uint8_t *>(this);
    int discriminator;
    std::memcpy(&discriminator, bytes - 8, sizeof(discriminator));
    if (discriminator == 8) {
        reinterpret_cast<SpriteBox *>(bytes - 0x8C)->on_right_click(a, b);
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
void Dialogs::on_scrolled(int code, int pos) {
    uint8_t *const bytes = reinterpret_cast<uint8_t *>(this);
    int discriminator;
    std::memcpy(&discriminator, bytes - 8, sizeof(discriminator));
    // A SWITCH, NOT AN `if`: the image is `sub eax, 2; jne`, which is
    // what VC6 emits for a one-case switch. An equality test compiles
    // `cmp dword ptr [ecx-8], 2` and reads the field a second time.
    switch (discriminator) {
        case 2:
            reinterpret_cast<ListBox *>(bytes - 0x140)->on_scrolling(code, pos);
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
void Dialogs::on_scrolling(int a, int b) {
    uint8_t *const bytes = reinterpret_cast<uint8_t *>(this);
    int discriminator;
    std::memcpy(&discriminator, bytes - 8, sizeof(discriminator));
    // A SWITCH, NOT AN `if`: the image is `sub eax, 2; jne`, which is
    // what VC6 emits for a one-case switch. An equality test compiles
    // `cmp dword ptr [ecx-8], 2` and reads the field a second time.
    switch (discriminator) {
        case 2:
            reinterpret_cast<ListBox *>(bytes - 0x140)->on_scrolling(a, b);
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
void Dialogs::on_mousewheel(int a) {
    uint8_t *const bytes = reinterpret_cast<uint8_t *>(this);
    int discriminator;
    std::memcpy(&discriminator, bytes - 8, sizeof(discriminator));
    // A SWITCH, NOT AN `if`: the image is `sub eax, 2; jne`, which is
    // what VC6 emits for a one-case switch. An equality test compiles
    // `cmp dword ptr [ecx-8], 2` and reads the field a second time.
    switch (discriminator) {
        case 2:
            reinterpret_cast<ListBox *>(bytes - 0x140)->on_mousewheel(a);
            break;
        default:
            break;
    }
}









func_dialogs_teardown DialogsEditGroupDestructor =
    original_method<func_dialogs_teardown>(0x00611A20);
func_dialogs_teardown DialogsSpriteBoxDestructor =
    original_method<func_dialogs_teardown>(0x00610120);
func_dialogs_teardown DialogsCheckBoxDestructor =
    original_method<func_dialogs_teardown>(0x0060E740);


namespace {

// stage_virtual_tables and its six function-local constants are gone: the
// two staging rounds (Dialogs' own GraphicWin/Win virtual base at 0x188/0xBA0
// and the embedded RadioButton at 0x18/0xA30) were the derived-stage overlay
// idiom - hand writes the image itself performs - and they left with the
// strip-all direction, exactly as the AlphaMovie and Ambience overlays did.

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
// symbol    ?destroy@Dialogs@@QAEIXZ
// TRIED: 3/92 - same SEH-frame symptom as `Dialog::~Dialog()` (this
//            file's sibling class in dialog.cpp) and FlatButton's own
//            constructor/destructor pair. Not attempted: the comment
//            above claims the frame is "unreachable, omitted per policy",
//            which is UNVERIFIED here (flagged, not trusted - see the
//            note on Dialog::~Dialog for why). This body is the most
//            complex of the three (six member teardowns across five
//            classes chained together); reproducing the frame plus
//            whatever member-destruction reordering it would take is out
//            of scope for this pass's budget.
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

    // Both staging rounds left (see stage_virtual_tables' note above): the
    // object runs on whatever its constructing chain installed.

    close();

    // The three widget members, each entered at its fixed adjustment past the
    // member base, guarded as the original guards them. Integer arithmetic so
    // the unreachable null branch stays exactly the original's `0 + adjust`.
    const uintptr_t guard =
        base != nullptr ? reinterpret_cast<uintptr_t>(base) : 0;
    (ORIGINAL(guarded_member(guard, 0xF8, 0x8C))->*DialogsEditGroupDestructor)();
    (ORIGINAL(guarded_member(guard, 0x70, 0x8C))->*DialogsSpriteBoxDestructor)();
    (ORIGINAL(guarded_member(guard, 0x58, 0x1C))->*DialogsCheckBoxDestructor)();

    // The embedded RadioButton at base+0x44: its recovered close walks the
    // shared virtual bases through its own vbtable.
    uint8_t *const radio = static_cast<uint8_t *>(guarded_member(guard, 0x44, 0));
    reinterpret_cast<RadioButton *>(radio)->close();

    // The embedded ListBox's base coincides with the allocation base; its
    // recovered destroy closes both shared bases. Unguarded, as the original
    // left it.
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
    reinterpret_cast<GraphicWin *>(base + 0x188)->~GraphicWin();
    if (mode & 1) {
        operator delete(base);
    }
    return base;
}

/*
Purpose: Route a mouse move to whichever control this Dialogs subobject is
         embedded in - the `kind_` at this-8 says which, and each case steps
         back to the enclosing control's own base. HOMED from
         src/unrecovered/00612b80.cpp on 2026-08-25 to retire its
         pending_bodies forwarder. The artifact's binding at 0x00612BF8 was
         unused scaffold pointing at this function's own end address and
         did not come with it.
*/
// ORIGINAL: 0x00612B80 ?on_mouse_move@Dialogs@@QAEXHH@Z 0x00612B80-0x00612BF8 FILE BYTE_EXACT
// symbol    ?on_mouse_move@Dialogs@@QAEXHH@Z
// size      120 bytes
// kind      game
void Dialogs::on_mouse_move(int a1, int a2) {
    char *self = reinterpret_cast<char *>(this);
    int type = *(int *)(self - 8);
    switch (type) {
        case 16:
            ((RadioButton *)(self - 0x12c))->on_mouse_move(a1, a2);
            break;
        case 2:
            ((ListBox *)(self - 0x140))->on_mouse_move(a1, a2);
            break;
        case 1:
            ((CheckBox *)(self - 0x114))->on_mouse_move(a1, a2);
            break;
        case 8:
            ((SpriteBox *)(self - 0x8c))->on_mouse_move(a1, a2);
            break;
        default:
            break;
    }
}

// ====================
// Window-event handlers, promoted out of the artifact archives
// ====================

namespace {
// A LOCAL SHIM, not vtable_shim.h's VCall: only DECLARATION ORDER decides a
// slot, and the image's call site (vtable slot 0x24/4 = 9) takes one pointer
// as an int and returns int, where the shared VCall's slot009 is nullary.
class ScrollVCall { public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();
    virtual void slot003();
    virtual void slot004();
    virtual void slot005();
    virtual void slot006();
    virtual void slot007();
    virtual void slot008();
    virtual int slot009(int);  // <-- used
};
}  // namespace

/*
Purpose: Hand a closing scroll control to the list box: only when the active
         dialog is the list-box kind (2) and the scroll's flags byte has bit
         0x40 set does the scroll's own vtable slot 9 take it, and its return
         is this function's return; otherwise the argument passes back
         unchanged. Entered on the interface subobject (the GraphicWin virtual
         base at +0x188 in a most-derived Dialogs), so the discriminator sits
         8 bytes below `this` and the ListBox's vbtable names where its
         scroll machinery sits.
// ORIGINAL: 0x00613260 ?on_scroll_delete@Dialogs@@QAEHPAUScroll@@@Z 0x00613260-0x0061329F BYTE_EXACT
// symbol    ?on_scroll_delete@Dialogs@@QAEHPAX@Z
// size      63 bytes
// prototype int (__thiscall ?on_scroll_delete@Dialogs@@QAEHPAUScroll@@@Z)(Dialogs* this, Scroll*)
// callers   0   call targets   0
// kind      game
// indirect  0x00613292
//
// PROMOTED out of the archived verification unit for this address, which proved the body
// against the same shim shape under its own name. The pointer parameter
// travels as the int the vtable slot takes; the header spells it void *
// and the catalogued name spells it Scroll * - the bytes are the same.
Return Value: the scroll, or the slot's answer
Status: Complete
*/
int Dialogs::on_scroll_delete(void *scroll) {
    char *self = reinterpret_cast<char *>(this);
    // A SWITCH, NOT AN `if`: the image is `sub eax, 2; jne`, which is what
    // VC6 emits for a one-case switch; an equality test folds to a fused
    // `cmp [mem], imm` instead.
    switch (*reinterpret_cast<int *>(self - 8)) {
    case 2: {
        char *base = *reinterpret_cast<char **>(self - 0x188);
        int off = *reinterpret_cast<int *>(base + 4);
        char *mid = self + off - 0x140;
        unsigned char flags = *reinterpret_cast<unsigned char *>(mid + 0x50);
        if (flags & 0x40) {
            // The scroll is a Win-family object: slot 9 is the declared
            // vslot_09(int), whose default returns its argument - which is
            // exactly this body's documented pass-back behaviour.
            Scroll *scroll_box = *reinterpret_cast<Scroll **>(mid + 0x7c);
            if (scroll_box != 0) {
                return reinterpret_cast<int>(scroll_box->vslot_09(scroll));
            }
        }
    }
    }
    return reinterpret_cast<int>(scroll);
}

/*
Purpose: Forward a left click to the embedded sprite box for every dialog
         kind except the list box (0), the text kinds (8's neighbours at 0)
         and the unused 8 - the same sixteen-way discrimination the sibling
         on_right_down makes with a single compare. Entered on the interface
         subobject, so the discriminator sits 8 bytes below `this` and the
         sprite box is reached by adjusting back 0x8C.
// ORIGINAL: 0x00612E80 ?on_left_click@Dialogs@@QAEXHH@Z 0x00612E80-0x00612EB0
// size      48 bytes
// prototype void (__thiscall ?on_left_click@Dialogs@@QAEXHH@Z)(Dialogs* this, int xCoord, int yCoord)
// callers   0   call targets   1
// kind      game
// calls     0x00611060
//
// PROMOTED out of the archived verification unit for this address, which proved the sixteen-case
// switch against the image's own jump-table and case-map bytes.
// MEASURED SHAPE_EXACT 14/14, 1.000 similar: every in-span byte agrees
// modulo relocations (the case-map address, the jmp-table address and the
// call to SpriteBox::on_left_click). The 24 bytes past the span - the image's
// inline case map and jump table - differ only because this tree's /Gy object
// puts them in their own comdat elsewhere instead of inline after the
// function; that is section placement, not source form. The artifact's own
// BYTE_EXACT came from a single-function unit where the tables landed inline.
Return Value: n/a
Status: Complete
*/
void Dialogs::on_left_click(int a, int b) {
    char *const self = reinterpret_cast<char *>(this);
    switch (*reinterpret_cast<int *>(self - 8)) {
    case 1: case 2: case 3: case 4: case 5: case 6: case 7:
    case 9: case 10: case 11: case 12: case 13: case 14: case 15: case 16:
        reinterpret_cast<SpriteBox *>(self - 0x8C)->on_left_click(a, b);
        break;
    }
}
