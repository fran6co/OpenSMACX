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
#include "scroll.h"
#include <cstring>

/*
Purpose: Close the dialog manager; the legacy implementation is intentionally empty.
// ORIGINAL: 0x00612960 ?close@Dialogs@@QAEXXZ 0x00612960-0x00612961 BYTE_EXACT
// symbol    ?close@Dialogs@@QAEXXZ
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
/*
Purpose: Construct the dialog manager: the five vbtable pointers, the two
         virtual-base constructions, and the five base constructors.
// ORIGINAL: 0x00612830 ??0Dialogs@@QAE@H@Z 0x00612830-0x00612AE0
// symbol    ??0Dialogs@@QAE@XZ
// size      688 bytes
// prototype void (__thiscall ??0Dialogs@@QAE@H@Z)(Dialogs* this, int)
// callers   1   call targets   7
// kind      game
// flags     frame;sp_ready;purged_ok
Return Value: n/a

The image's span 0x00612830..0x00612AE0 covers this ctor (through its
`ret 4` at 0x0061295C) PLUS the jump-dispatch function at 0x00612970 and its
case stubs, which our binary lays out elsewhere; this ctor's own body
converges through its full length.
*/
Dialogs::Dialogs() { }

void Dialogs::close() {
}

// ORIGINAL: 0x00613320 ?init@Dialogs@@QAEHHHHHH@Z 0x00613320-0x006133F2
// RECOVERY_BINDING: v1 {"evidence":["Retained 0x00613320 seed records the five-integer Dialogs initializer's direct call order: RadioButton::close, shared Dialog initialization, ListBox::close, shared Dialog initialization, CheckBox::close, shared Dialog initialization, SpriteBox initialization, EditGroup::close, then shared Dialog initialization.","The pinned image's callee 0x00609300 is Dialog::init(int,int,int,int,int); the corresponding typed overload is declared and defined in dialog.h/dialog.cpp."],"reason":"Home the five-integer Dialogs initializer through its real widget base subobjects and shared virtual Dialog."}
// TRIED: 208 vs 210 bytes; the RadioButton::close() receiver compiles to `mov ecx,eax; call` instead of a direct `lea ecx,[..]; call`. Tried a shared base-address local, no local at all, and a named typed-pointer local for the receiver; all three landed on the same divergence, so it looks like inherent codegen for a computed `this` into a non-virtual thiscall call, not a source-form choice.
int Dialogs::init(int left, int top, int right, int bottom, int heap_size) {
    RadioButton::close();
    int result = Dialog::init(left, top, right, bottom, heap_size);
    if (result != 0) return result;

    ListBox::close();
    result = Dialog::init(left, top, right, bottom, heap_size);
    if (result != 0) return result;

    CheckBox::close();
    result = Dialog::init(left, top, right, bottom, heap_size);
    if (result != 0) return result;

    result = SpriteBox::init(left, top, right, bottom, heap_size);
    if (result != 0) return result;

    EditGroup::close();
    return Dialog::init(left, top, right, bottom, heap_size);
}

// ORIGINAL: 0x00613610 ?init@Dialogs@@QAEHPAURECT@@PAUHeap@@@Z 0x00613610-0x006136A1
// RECOVERY_BINDING: v1 {"evidence":["Retained 0x00613610 seed plus pinned inspect show the first argument remains RECT* through each direct Dialog call to 0x006097F0 and through SpriteBox::init at 0x00610510.","Dialog::init(RECT*,Heap*) is source-owned in dialog.cpp; the retained generated scaffold's reinterpret_cast<Heap*>(rect) is disproved by those target signatures."],"reason":"Home the rectangle-and-heap Dialogs initializer with the image's genuine RECT* and Heap* arguments."}
// symbol    ?init@Dialogs@@QAEHPAUtagRECT@@PAVHeap@@@Z
int Dialogs::init(RECT *rect, Heap *heap) {
    RadioButton::close();
    int result = Dialog::init(rect, heap);
    if (result != 0) return result;

    ListBox::close();
    result = Dialog::init(rect, heap);
    if (result != 0) return result;

    CheckBox::close();
    result = Dialog::init(rect, heap);
    if (result != 0) return result;

    result = SpriteBox::init(rect, heap);
    if (result != 0) return result;

    EditGroup::close();
    return Dialog::init(rect, heap);
}

// Seed catalogue: 0 callers; callees 0x00609470, 0x00609F20, 0x0060D1B0,
// 0x0060E7C0, 0x00610480, 0x00611A90.
// ORIGINAL: 0x00613400 ?init@Dialogs@@QAEHPAURECT@@H@Z 0x00613400-0x00613491
// RECOVERY_BINDING: v1 {"evidence":["Retained 0x00613400 seed and direct-call order in pinned image; Dialogs' declared shared virtual Dialog and five widget bases"],"reason":"Home the RECT/flags initializer as typed Dialogs base operations"}
// symbol    ?init@Dialogs@@QAEHPAUtagRECT@@H@Z
int Dialogs::init(RECT *group_id, int flags) {
    RadioButton::close();
    int result = Dialog::init(group_id, flags);
    if (result != 0) return result;

    ListBox::close();
    result = Dialog::init(group_id, flags);
    if (result != 0) return result;

    CheckBox::close();
    result = Dialog::init(group_id, flags);
    if (result != 0) return result;

    result = SpriteBox::init(group_id, flags);
    if (result != 0) return result;

    EditGroup::close();
    return Dialog::init(group_id, flags);
}

// Seed catalogue: 1 caller; callees 0x006095F0, 0x00609F20, 0x0060D1B0,
// 0x0060E7C0, 0x00610420, 0x00611A90.
// ORIGINAL: 0x006134A0 ?init@Dialogs@@QAEHH@Z 0x006134A0-0x00613524
// RECOVERY_BINDING: v1 {"evidence":["Retained 0x006134A0 seed and direct-call order in pinned image; Dialogs' declared shared virtual Dialog and five widget bases"],"reason":"Home the file-name initializer as typed Dialogs base operations"}
int Dialogs::init(int file_name) {
    RadioButton::close();
    int result = Dialog::init(file_name);
    if (result != 0) return result;

    ListBox::close();
    result = Dialog::init(file_name);
    if (result != 0) return result;

    CheckBox::close();
    result = Dialog::init(file_name);
    if (result != 0) return result;

    result = SpriteBox::init(file_name);
    if (result != 0) return result;

    EditGroup::close();
    return Dialog::init(file_name);
}

// Seed catalogue: 0 callers; callees 0x00609730, 0x00609F20, 0x0060D1B0,
// 0x0060E7C0, 0x006104D0, 0x00611A90.
// ORIGINAL: 0x00613530 ?init@Dialogs@@QAEHHHHHPAUHeap@@@Z 0x00613530-0x00613602
// RECOVERY_BINDING: v1 {"evidence":["Retained 0x00613530 seed and direct-call order in pinned image; Dialogs' declared shared virtual Dialog and five widget bases"],"reason":"Home the geometry and heap initializer as typed Dialogs base operations"}
// symbol    ?init@Dialogs@@QAEHHHHHPAVHeap@@@Z
int Dialogs::init(int x, int y, int width, int height, Heap *heap) {
    RadioButton::close();
    int result = Dialog::init(x, y, width, height, heap);
    if (result != 0) return result;

    ListBox::close();
    result = Dialog::init(x, y, width, height, heap);
    if (result != 0) return result;

    CheckBox::close();
    result = Dialog::init(x, y, width, height, heap);
    if (result != 0) return result;

    result = SpriteBox::init(x, y, width, height, heap);
    if (result != 0) return result;

    EditGroup::close();
    return Dialog::init(x, y, width, height, heap);
}

// Seed catalogue: 1 caller; callees 0x006098D0, 0x00609F20, 0x0060D1B0,
// 0x0060E7C0, 0x006104B0, 0x00611A90.
// ORIGINAL: 0x006136B0 ?init@Dialogs@@QAEHPAUHeap@@@Z 0x006136B0-0x00613734 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence":["Retained 0x006136B0 seed and direct-call order in pinned image; Dialogs' declared shared virtual Dialog and five widget bases"],"reason":"Home the heap initializer as typed Dialogs base operations while retaining its historical byte claim"}
// symbol    ?init@Dialogs@@QAEHPAVHeap@@@Z
int Dialogs::init(Heap *heap) {
    RadioButton::close();
    int result = Dialog::init(heap);
    if (result != 0) return result;

    ListBox::close();
    result = Dialog::init(heap);
    if (result != 0) return result;

    CheckBox::close();
    result = Dialog::init(heap);
    if (result != 0) return result;

    result = SpriteBox::init(heap);
    if (result != 0) return result;

    EditGroup::close();
    return Dialog::init(heap);
}

// Everything after the prologue is the compiler's: the five vbtable
// pointers, the GraphicWin and Dialog virtual-base construction, and the
// five base constructors (ListBox/RadioButton/CheckBox/SpriteBox/EditGroup,
// each run with a cleared flag). The image's 0x2B0-byte ??0Dialogs spends
// itself entirely on that machinery; this body has nothing to add.


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
    switch (kind_) {
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
// symbol    ?on_right_down@Dialogs@@UAEXHH@Z
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
Purpose: Forward left up to the SpriteBox base only for kind 8. The native
         612F10 receiver is GraphicWin+188, with kind_ at -8; its call uses
         the standalone SpriteBox receiver convention at Dialogs+FC.
         Source uses the genuine shared virtual base and a qualified call:
         compiler-owned adjustment replaces that synthetic receiver value.
         The historical byte obligation remains a restoration requirement.
// ORIGINAL: 0x00612F10 ?on_left_up@Dialogs@@QAEXHH@Z 0x00612F10-0x00612F35 BYTE_EXACT
// symbol    ?on_left_up@Dialogs@@UAEXHH@Z
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
    // A DEAD TEST FOR KIND 4, and it is in the shipped bytes. Both jumps go
    // to the SAME place - 0x00612F32, past the call - so `cmp eax, 4; je`
    // excludes a kind that `cmp eax, 8; jne` would have excluded anyway.
    // Reproduced because it is three instructions of the original; it
    // changes no behaviour, which is exactly why it survived.
    if (kind_ == 4) {
        return;
    }
    if (kind_ == 8) {
        sprite_box_view().SpriteBox::on_left_up(a, b);
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
// symbol    ?on_right_click@Dialogs@@UAEXHH@Z
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
Purpose: Route the scrolled event by the Dialogs kind to its declared
         ListBox base. The generated interface receiver adjustment is
         separate from this ordinary source member's complete-object this.
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
    switch (kind_) {
        case 2:
            static_cast<ListBox *>(this)->on_scrolling(code, pos);
            break;
        default:
            break;
    }
}

/*
Purpose: Route the scrolling event by the Dialogs kind to its declared
         ListBox base. The generated interface receiver adjustment is
         separate from this ordinary source member's complete-object this.
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
    switch (kind_) {
        case 2:
            static_cast<ListBox *>(this)->on_scrolling(a, b);
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
// symbol    ?on_mousewheel@Dialogs@@UAEXH@Z
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
    reinterpret_cast<Dialog *>(base + 0xBA0)->Dialog::~Dialog();
    reinterpret_cast<GraphicWin *>(base + 0x188)->~GraphicWin();
    if (mode & 1) {
        operator delete(base);
    }
    return base;
}

/*
Purpose: Route mouse move by the owned kind_ through qualified widget bases.
         Native 612B80 reads kind_ at GraphicWin receiver-8, then passes
         standalone receiver values +5C/+48/+74/+FC for kinds16/2/1/8.
         These are not the shared GraphicWin subobject at Dialogs+188;
         real base conversions must be compiler-owned, not raw offsets.
         Preserve the historical byte obligation while restoring the genuine
         hierarchy. HOMED from
         src/unrecovered/00612b80.cpp on 2026-08-25 to retire its
         pending_bodies forwarder. The artifact's binding at 0x00612BF8 was
         unused scaffold pointing at this function's own end address and
         did not come with it.
*/
// ORIGINAL: 0x00612B80 ?on_mouse_move@Dialogs@@QAEXHH@Z 0x00612B80-0x00612BF8 FILE BYTE_EXACT
// symbol    ?on_mouse_move@Dialogs@@UAEXHH@Z
// size      120 bytes
// kind      game
void Dialogs::on_mouse_move(int a1, int a2) {
    switch (kind_) {
        case 16:
            radio_button_view().RadioButton::on_mouse_move(a1, a2);
            break;
        case 2:
            list_box_view().ListBox::on_mouse_move(a1, a2);
            break;
        case 1:
            check_box_view().CheckBox::on_mouse_move(a1, a2);
            break;
        case 8:
            sprite_box_view().SpriteBox::on_mouse_move(a1, a2);
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
// symbol    ?on_scroll_delete@Dialogs@@UAEHPAVScroll@@@Z
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
int Dialogs::on_scroll_delete(Scroll *scroll) {
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
// ORIGINAL: 0x00612E80 ?on_left_click@Dialogs@@QAEXHH@Z 0x00612E80-0x00612EB0 SEMANTIC
// symbol    ?on_left_click@Dialogs@@UAEXHH@Z
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

/*
Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
         Dialogs::on_redraw expects, then forward unchanged.
// ORIGINAL: 0x00407170 ?on_redraw@thunk1_Dialogs@@QAEXXZ 0x00407170-0x00407178 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned Dialogs constructor ??0Dialogs@@QAE@H@Z at 0x00612830 captures incoming this in esi at 0x0061284D and installs table 0x00669BE8 at 0x0061290A through the constructor's resolved subobject view; the pinned slot +0x30 contains 0x00407170. Original thunk behavior: sub ecx, dword ptr [ecx - 4]; jmp 0x612e00; tail target 0x00612E00.", "Current genuine owner is src/dialogs.cpp; retained owner-object vftable ??_7Dialogs@@6BWin@@@ has an absolute relocation at +0x30 to ?on_redraw@Dialogs@@$4PPPPPPPM@A@AEXXZ, whose COMDAT preserves the pinned receiver-adjustment bytes and whose terminal relative relocation selects the source-owned ?on_redraw@Dialogs@@UAEXXZ method. This source binding records the measured generated owner identity; its physical relocation must be rechecked after source changes."], "reason": "Bind the full original identity and spans to the existing compiler-generated class-view adjustor selected by the constructor-installed original slot; retain its historical claims and lessons unchanged."}
// symbol    ?on_redraw@Dialogs@@$4PPPPPPPM@A@AEXXZ
// size      8 bytes
// prototype void (__thiscall ?on_redraw@thunk1_Dialogs@@QAEXXZ)(Dialogs* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// TRIED 2026-09-26 (Win slot-12 on_redraw rename batch): with Win slot12 renamed
//            on_redraw, Dialogs::on_redraw overrides that slot through GraphicWin as
//            well as Dialog slot2, so the qualified call's receiver fold moved from
//            the Dialog offset 0xBA0 to the GraphicWin offset 0x188. The walk now
//            pre-subtracts 0x188 to cancel it, leaving the image's bare vtordisp thunk;
//            the old 0xBA0 walk emitted an extra `sub ecx, 0xa18` here. The compiler
//            now emits its own vtordisp thunk for this slot; this handwritten walk
//            stays the claim host pending coordinated retirement onto it.
Return Value: n/a
Status: Complete
*/

/*
Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
         Dialogs::on_mouse_move expects, then forward unchanged.
// ORIGINAL: 0x00407190 ?on_mouse_move@thunk1_Dialogs@@QAEXHH@Z 0x00407190-0x00407198 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned Dialogs constructor ??0Dialogs@@QAE@H@Z at 0x00612830 captures incoming this in esi at 0x0061284D and installs table 0x00669BE8 at 0x0061290A through the constructor's resolved subobject view; the pinned slot +0x44 contains 0x00407190. Original thunk behavior: sub ecx, dword ptr [ecx - 4]; jmp 0x612b80; tail target 0x00612B80.", "Current genuine owner is src/dialogs.cpp; retained owner-object vftable ??_7Dialogs@@6BWin@@@ has an absolute relocation at +0x44 to ?on_mouse_move@Dialogs@@$4PPPPPPPM@A@AEXHH@Z, whose COMDAT preserves the pinned receiver-adjustment bytes and whose terminal relative relocation selects the source-owned ?on_mouse_move@Dialogs@@UAEXHH@Z method. This source binding records the measured generated owner identity; its physical relocation must be rechecked after source changes."], "reason": "Bind the full original identity and spans to the existing compiler-generated class-view adjustor selected by the constructor-installed original slot; retain its historical claims and lessons unchanged."}
// symbol    ?on_mouse_move@Dialogs@@$4PPPPPPPM@A@AEXHH@Z
// size      8 bytes
// prototype void (__thiscall ?on_mouse_move@thunk1_Dialogs@@QAEXHH@Z)(Dialogs* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/

/*
Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
         Dialogs::on_mouse_leave expects, then forward unchanged.
// ORIGINAL: 0x004071A0 ?on_mouse_leave@thunk1_Dialogs@@QAEXHH@Z 0x004071A0-0x004071A8 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned Dialogs constructor ??0Dialogs@@QAE@H@Z at 0x00612830 captures incoming this in esi at 0x0061284D and installs table 0x00669BE8 at 0x0061290A through the constructor's resolved subobject view; the pinned slot +0x48 contains 0x004071A0. Original thunk behavior: sub ecx, dword ptr [ecx - 4]; jmp 0x612ae0; tail target 0x00612AE0.", "Current genuine owner is src/dialogs.cpp; retained owner-object vftable ??_7Dialogs@@6BWin@@@ has an absolute relocation at +0x48 to ?on_mouse_leave@Dialogs@@$4PPPPPPPM@A@AEXHH@Z, whose COMDAT preserves the pinned receiver-adjustment bytes and whose terminal relative relocation selects the source-owned ?on_mouse_leave@Dialogs@@UAEXHH@Z method. This source binding records the measured generated owner identity; its physical relocation must be rechecked after source changes."], "reason": "Bind the full original identity and spans to the existing compiler-generated class-view adjustor selected by the constructor-installed original slot; retain its historical claims and lessons unchanged."}
// symbol    ?on_mouse_leave@Dialogs@@$4PPPPPPPM@A@AEXHH@Z
// size      8 bytes
// prototype void (__thiscall ?on_mouse_leave@thunk1_Dialogs@@QAEXHH@Z)(Dialogs* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/

/*
Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
         Dialogs::on_left_down expects, then forward unchanged.
// ORIGINAL: 0x004071C0 ?on_left_down@thunk1_Dialogs@@QAEXHH@Z 0x004071C0-0x004071C8 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned Dialogs constructor ??0Dialogs@@QAE@H@Z at 0x00612830 captures incoming this in esi at 0x0061284D and installs table 0x00669BE8 at 0x0061290A through the constructor's resolved subobject view; the pinned slot +0x50 contains 0x004071C0. Original thunk behavior: sub ecx, dword ptr [ecx - 4]; jmp 0x612c20; tail target 0x00612C20.", "Current genuine owner is src/dialogs.cpp; retained owner-object vftable ??_7Dialogs@@6BWin@@@ has an absolute relocation at +0x50 to ?on_left_down@Dialogs@@$4PPPPPPPM@A@AEXHH@Z, whose COMDAT preserves the pinned receiver-adjustment bytes and whose terminal relative relocation selects the source-owned ?on_left_down@Dialogs@@UAEXHH@Z method. This source binding records the measured generated owner identity; its physical relocation must be rechecked after source changes."], "reason": "Bind the full original identity and spans to the existing compiler-generated class-view adjustor selected by the constructor-installed original slot; retain its historical claims and lessons unchanged."}
// symbol    ?on_left_down@Dialogs@@$4PPPPPPPM@A@AEXHH@Z
// size      8 bytes
// prototype void (__thiscall ?on_left_down@thunk1_Dialogs@@QAEXHH@Z)(Dialogs* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/

/*
Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
         Dialogs::on_left_up expects, then forward unchanged.
// ORIGINAL: 0x004071D0 ?on_left_up@thunk1_Dialogs@@QAEXHH@Z 0x004071D0-0x004071D8 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned Dialogs constructor ??0Dialogs@@QAE@H@Z at 0x00612830 captures incoming this in esi at 0x0061284D and installs table 0x00669BE8 at 0x0061290A through the constructor's resolved subobject view; the pinned slot +0x54 contains 0x004071D0. Original thunk behavior: sub ecx, dword ptr [ecx - 4]; jmp 0x612f10; tail target 0x00612F10.", "Current genuine owner is src/dialogs.cpp; retained owner-object vftable ??_7Dialogs@@6BWin@@@ has an absolute relocation at +0x54 to ?on_left_up@Dialogs@@$4PPPPPPPM@A@AEXHH@Z, whose COMDAT preserves the pinned receiver-adjustment bytes and whose terminal relative relocation selects the source-owned ?on_left_up@Dialogs@@UAEXHH@Z method. This source binding records the measured generated owner identity; its physical relocation must be rechecked after source changes."], "reason": "Bind the full original identity and spans to the existing compiler-generated class-view adjustor selected by the constructor-installed original slot; retain its historical claims and lessons unchanged."}
// symbol    ?on_left_up@Dialogs@@$4PPPPPPPM@A@AEXHH@Z
// size      8 bytes
// prototype void (__thiscall ?on_left_up@thunk1_Dialogs@@QAEXHH@Z)(Dialogs* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/

/*
Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
         Dialogs::on_right_down expects, then forward unchanged.
// ORIGINAL: 0x004071F0 ?on_right_down@thunk1_Dialogs@@QAEXHH@Z 0x004071F0-0x004071F8 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned Dialogs constructor ??0Dialogs@@QAE@H@Z at 0x00612830 captures incoming this in esi at 0x0061284D and installs table 0x00669BE8 at 0x0061290A through the constructor's resolved subobject view; the pinned slot +0x5C contains 0x004071F0. Original thunk behavior: sub ecx, dword ptr [ecx - 4]; jmp 0x612ed0; tail target 0x00612ED0.", "Current genuine owner is src/dialogs.cpp; retained owner-object vftable ??_7Dialogs@@6BWin@@@ has an absolute relocation at +0x5C to ?on_right_down@Dialogs@@$4PPPPPPPM@A@AEXHH@Z, whose COMDAT preserves the pinned receiver-adjustment bytes and whose terminal relative relocation selects the source-owned ?on_right_down@Dialogs@@UAEXHH@Z method. This source binding records the measured generated owner identity; its physical relocation must be rechecked after source changes."], "reason": "Bind the full original identity and spans to the existing compiler-generated class-view adjustor selected by the constructor-installed original slot; retain its historical claims and lessons unchanged."}
// symbol    ?on_right_down@Dialogs@@$4PPPPPPPM@A@AEXHH@Z
// size      8 bytes
// prototype void (__thiscall ?on_right_down@thunk1_Dialogs@@QAEXHH@Z)(Dialogs* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/

/*
Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
         Dialogs::on_left_double_click expects, then forward unchanged.
// ORIGINAL: 0x00407220 ?on_left_double_click@thunk1_Dialogs@@QAEXHH@Z 0x00407220-0x00407228 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned Dialogs constructor ??0Dialogs@@QAE@H@Z at 0x00612830 captures incoming this in esi at 0x0061284D and installs table 0x00669BE8 at 0x0061290A through the constructor's resolved subobject view; the pinned slot +0x70 contains 0x00407220. Original thunk behavior: sub ecx, dword ptr [ecx - 4]; jmp 0x612d60; tail target 0x00612D60.", "Current genuine owner is src/dialogs.cpp; retained owner-object vftable ??_7Dialogs@@6BWin@@@ has an absolute relocation at +0x70 to ?on_left_double_click@Dialogs@@$4PPPPPPPM@A@AEXHH@Z, whose COMDAT preserves the pinned receiver-adjustment bytes and whose terminal relative relocation selects the source-owned ?on_left_double_click@Dialogs@@UAEXHH@Z method. This source binding records the measured generated owner identity; its physical relocation must be rechecked after source changes."], "reason": "Bind the full original identity and spans to the existing compiler-generated class-view adjustor selected by the constructor-installed original slot; retain its historical claims and lessons unchanged."}
// symbol    ?on_left_double_click@Dialogs@@$4PPPPPPPM@A@AEXHH@Z
// size      8 bytes
// prototype void (__thiscall ?on_left_double_click@thunk1_Dialogs@@QAEXHH@Z)(Dialogs* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/

/*
Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
         Dialogs::on_dialog_focus expects, then forward unchanged.
// ORIGINAL: 0x00407260 ?on_dialog_focus@thunk1_Dialogs@@QAEXH@Z 0x00407260-0x00407268 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned Dialogs constructor ??0Dialogs@@QAE@H@Z at 0x00612830 captures incoming this in esi at 0x0061284D and installs table 0x00669BE8 at 0x0061290A through the constructor's resolved subobject view; the pinned slot +0xCC contains 0x00407260. Original thunk behavior: sub ecx, dword ptr [ecx - 4]; jmp 0x6130e0; tail target 0x006130E0.", "Current genuine owner is src/dialogs.cpp; retained owner-object vftable ??_7Dialogs@@6BWin@@@ has an absolute relocation at +0xCC to ?on_dialog_focus@Dialogs@@$4PPPPPPPM@A@AEXH@Z, whose COMDAT preserves the pinned receiver-adjustment bytes and whose terminal relative relocation selects the source-owned ?on_dialog_focus@Dialogs@@UAEXH@Z method. This source binding records the measured generated owner identity; its physical relocation must be rechecked after source changes."], "reason": "Bind the full original identity and spans to the existing compiler-generated class-view adjustor selected by the constructor-installed original slot; retain its historical claims and lessons unchanged."}
// symbol    ?on_dialog_focus@Dialogs@@$4PPPPPPPM@A@AEXH@Z
// size      8 bytes
// prototype void (__thiscall ?on_dialog_focus@thunk1_Dialogs@@QAEXH@Z)(Dialogs* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/

/*
Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
         Dialogs::pass_dialog_focus expects, then forward unchanged.
// ORIGINAL: 0x00407270 ?pass_dialog_focus@thunk1_Dialogs@@QAEXXZ 0x00407270-0x00407278 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned Dialogs constructor ??0Dialogs@@QAE@H@Z at 0x00612830 captures incoming this in esi at 0x0061284D and installs table 0x00669BE8 at 0x0061290A through the constructor's resolved subobject view; the pinned slot +0x100 contains 0x00407270. Original thunk behavior: sub ecx, dword ptr [ecx - 4]; jmp 0x613180; tail target 0x00613180.", "Current genuine owner is src/dialogs.cpp; retained owner-object vftable ??_7Dialogs@@6BWin@@@ has an absolute relocation at +0x100 to ?pass_dialog_focus@Dialogs@@$4PPPPPPPM@A@AEXXZ, whose COMDAT preserves the pinned receiver-adjustment bytes and whose terminal relative relocation selects the source-owned ?pass_dialog_focus@Dialogs@@UAEXXZ method. This source binding records the measured generated owner identity; its physical relocation must be rechecked after source changes."], "reason": "Bind the full original identity and spans to the existing compiler-generated class-view adjustor selected by the constructor-installed original slot; retain its historical claims and lessons unchanged."}
// symbol    ?pass_dialog_focus@Dialogs@@$4PPPPPPPM@A@AEXXZ
// size      8 bytes
// prototype void (__thiscall ?pass_dialog_focus@thunk1_Dialogs@@QAEXXZ)(Dialogs* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/

/*
Purpose: Adjust the receiver from Dialogs's thunk3 subobject to the one
         Dialogs::on_redraw expects, then forward unchanged.
// ORIGINAL: 0x004072C0 ?on_redraw@thunk3_Dialogs@@QAEXXZ 0x004072C0-0x004072CE FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned Dialogs constructor ??0Dialogs@@QAE@H@Z at 0x00612830 captures incoming this in esi at 0x0061284D and installs table 0x00669BD4 at 0x00612926 through the constructor's resolved subobject view; the pinned slot +0x8 contains 0x004072C0. Original thunk behavior: sub ecx, dword ptr [ecx - 4]; sub ecx, 0xa18; jmp 0x612e00; tail target 0x00612E00.", "Current genuine owner is src/dialogs.cpp; retained owner-object vftable ??_7Dialogs@@6B@ has an absolute relocation at +0x8 to ?on_redraw@Dialogs@@$4PPPPPPPM@KBI@AEXXZ, whose COMDAT preserves the pinned receiver-adjustment bytes and whose terminal relative relocation selects the source-owned ?on_redraw@Dialogs@@UAEXXZ method. This source binding records the measured generated owner identity; its physical relocation must be rechecked after source changes."], "reason": "Bind the full original identity and spans to the existing compiler-generated class-view adjustor selected by the constructor-installed original slot; retain its historical claims and lessons unchanged."}
// symbol    ?on_redraw@Dialogs@@$4PPPPPPPM@KBI@AEXXZ
// size      14 bytes
// prototype void (__thiscall ?on_redraw@thunk3_Dialogs@@QAEXXZ)(Dialogs* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// TRIED 2026-09-26 (Win slot-12 on_redraw rename batch): the receiver fold is now
//            +0x188 (GraphicWin), so the walk keeps only the Dialog offset 0xBA0; the
//            net adjustment is the image's `sub ecx, 0xa18` (the family-wide
//            Dialog-to-GraphicWin gap 0xBA0 - 0x188). The pre-rename spelling also
//            pre-subtracted 2584 and emitted `sub ecx, 0x1430` after the rename.
//            The compiler now emits its own two-step thunk for this slot; this
//            handwritten walk stays the claim host pending coordinated retirement.
Return Value: n/a
Status: Complete
*/

// ORIGINAL: 0x00613180 ?pass_dialog_focus@Dialogs@@QAEXXZ 0x00613180-0x006131F5
// RECOVERY_BINDING: v1 {"reason":"Recover the complete body in its genuine C++ owner, with real embedded members, shared virtual bases, lifecycle and parent operations. Compiler-generated overrides/adjustors replace executable-address or vbtable stand-ins; no current matching claim is granted.","evidence":["git:de91fffe:src/recovered/units/00613180.cpp","image:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf","recovery/tasks/ui-eight-owner-coverage.json","Original entry 0x00613180; physical VC6 COFF emitted symbol ?pass_dialog_focus@Dialogs@@UAEXXZ."]}
// symbol    ?pass_dialog_focus@Dialogs@@UAEXXZ
void Dialogs::pass_dialog_focus() {
    switch (kind_) {
    case 1:
    case 2:
    case 8:
    case 16:
        GraphicWin::pass_dialog_focus();
        break;
    case 4:
        EditGroup::pass_dialog_focus();
        break;
    }
}

// ORIGINAL: 0x00613220 ?on_scroll_create@Dialogs@@QAEHXZ 0x00613220-0x00613254
// RECOVERY_BINDING: v1 {"reason":"Recover the complete body in its genuine C++ owner, with real embedded members, shared virtual bases, lifecycle and parent operations. Compiler-generated overrides/adjustors replace executable-address or vbtable stand-ins; no current matching claim is granted.","evidence":["git:de91fffe:src/recovered/units/00613220.cpp","image:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf","recovery/tasks/ui-eight-owner-coverage.json","Original entry 0x00613220; physical VC6 COFF emitted symbol ?on_scroll_create@Dialogs@@QAEHXZ."]}
// symbol    ?on_scroll_create@Dialogs@@UAEHXZ
int Dialogs::on_scroll_create() {
    if (kind_ == 2 && (iFlags_ & 0x40) != 0 && win_parent_ != 0) {
        return win_parent_->vslot_08();
    }
    return 0;
}

// ORIGINAL: 0x006132A0 ?calculate_dimensions@Dialogs@@QAEHXZ 0x006132A0-0x006132F8 BYTE_EXACT
// size      88 bytes
// prototype int (__thiscall ?calculate_dimensions@Dialogs@@QAEHXZ)(Dialogs* this)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// 0x006132A0  ?calculate_dimensions@Dialogs@@QAEHXZ  ->  ?calculate_dimensions@Dialogs@@QAEHXZ
//
// Re-derived in this tree's own style (2026-09-06) ahead of homing into
// dialogs.cpp. The kind at kind_ (Dialogs+0x180) selects which of
// the five base subobjects lays itself out, and the dispatcher is a tail
// jump per arm: `add ecx, <base offset>` then `jmp` the base's own
// calculate_dimensions, because the caller has nothing on the stack to
// unwind and the callee's `ret` is the caller's.
//
// The default arm is the no-kind case and it does NOT dispatch: it zeroes
// the embedded Dialog virtual base's two edge fields, reached as a QUALIFIED
// MEMBER ACCESS so VC6 walks the vbtable for the virtual base's offset
// (`mov eax,[ecx] / mov edx,[eax+8] / mov [edx+ecx+0x5c], eax`). It re-walks
// for the second store rather than keeping the offset, which is what two
// independent member writes compile to - do not hoist either field into a
// local.


int Dialogs::calculate_dimensions() {
    switch (kind_) {
      case 0x10:
        return RadioButton::calculate_dimensions();
      case 2:
        return ListBox::calculate_dimensions();
      case 1:
        return CheckBox::calculate_dimensions();
      case 8:
        return SpriteBox::calculate_dimensions();
      case 4:
        return EditGroup::calculate_dimensions();
      default:
        Dialog::field_5C_ = 0;
        Dialog::field_60_ = 0;
        return 0;
    }
}

// These recovered dispatch bodies preserve the archived case maps but route
// through declared C++ subobjects. The image's per-base thunks account for the
// different receiver seen by the old bodies (the GraphicWin view is +0x188);
// this source implementation begins with a proper Dialogs receiver, reads
// kind_ and lets qualified base calls perform the compiler-known adjustment.
// ORIGINAL: 0x00612C20 ?on_left_down@Dialogs@@QAEXHH@Z 0x00612C20-0x00612C98
// RECOVERY_BINDING: v1 {"evidence": ["Retained 0x00612C20 seed; shared virtual Dialog and GraphicWin bases; Dialogs kind_ and named widget base APIs"], "reason": "Home on_left_down as a typed Dialogs kind dispatcher"}
// symbol    ?on_left_down@Dialogs@@UAEXHH@Z
void Dialogs::on_left_down(int x, int y) {
    switch (kind_) {
    case 1:
        check_box_view().CheckBox::on_left_down(x, y);
        break;
    case 2:
        list_box_view().ListBox::on_left_down(x, y);
        break;
    case 8:
        sprite_box_view().SpriteBox::on_left_down(x, y);
        break;
    case 16:
        radio_button_view().RadioButton::on_left_down(x, y);
        break;
    }
}

// ORIGINAL: 0x00612D60 ?on_left_double_click@Dialogs@@QAEXHH@Z 0x00612D60-0x00612DD8
// RECOVERY_BINDING: v1 {"evidence": ["Retained 0x00612D60 seed; shared virtual Dialog and GraphicWin bases; Dialogs kind_ and named widget base APIs"], "reason": "Home on_left_double_click as a typed Dialogs kind dispatcher"}
// symbol    ?on_left_double_click@Dialogs@@UAEXHH@Z
void Dialogs::on_left_double_click(int x, int y) {
    switch (kind_) {
    case 1:
        check_box_view().CheckBox::on_left_double_click(x, y);
        break;
    case 2:
        list_box_view().ListBox::on_left_double_click(x, y);
        break;
    case 8:
        sprite_box_view().SpriteBox::on_left_double_click(x, y);
        break;
    case 16:
        radio_button_view().RadioButton::on_left_double_click(x, y);
        break;
    }
}

// ORIGINAL: 0x00612AE0 ?on_mouse_leave@Dialogs@@QAEXHH@Z 0x00612AE0-0x00612B58
// RECOVERY_BINDING: v1 {"evidence": ["Retained 0x00612AE0 seed; shared virtual Dialog and GraphicWin bases; Dialogs kind_ and named widget base APIs"], "reason": "Home on_mouse_leave as a typed Dialogs kind dispatcher"}
// symbol    ?on_mouse_leave@Dialogs@@UAEXHH@Z
void Dialogs::on_mouse_leave(int x, int y) {
    switch (kind_) {
    case 1:
        check_box_view().CheckBox::on_mouse_leave(x, y);
        break;
    case 2:
        list_box_view().ListBox::on_mouse_leave(x, y);
        break;
    case 8:
        sprite_box_view().SpriteBox::on_mouse_leave(x, y);
        break;
    case 16:
        radio_button_view().RadioButton::on_mouse_leave(x, y);
        break;
    }
}

// ORIGINAL: 0x00612CC0 ?on_key_down@Dialogs@@QAEHH@Z 0x00612CC0-0x00612D29
// RECOVERY_BINDING: v1 {"evidence": ["Retained 0x00612CC0 seed; shared virtual Dialog and GraphicWin bases; Dialogs kind_ and named widget base APIs"], "reason": "Home on_key_down as a typed Dialogs kind dispatcher"}
// symbol    ?on_key_down@Dialogs@@UAEHH@Z
int Dialogs::on_key_down(int key) {
    switch (kind_) {
    case 16:
        return radio_button_view().RadioButton::on_key_down(key);
    case 2:
        return list_box_view().ListBox::on_key_down(key);
    case 1:
        return check_box_view().CheckBox::on_key_down(key);
    case 8:
        return sprite_box_view().SpriteBox::on_key_down(key);
    default:
        return 0;
    }
    return 0;
}

// ORIGINAL: 0x006130E0 ?on_dialog_focus@Dialogs@@QAEXH@Z 0x006130E0-0x00613154
// RECOVERY_BINDING: v1 {"evidence": ["Retained 0x006130E0 seed; shared virtual Dialog and GraphicWin bases; Dialogs kind_ and named widget base APIs"], "reason": "Home on_dialog_focus as a typed Dialogs kind dispatcher"}
// symbol    ?on_dialog_focus@Dialogs@@UAEXH@Z
void Dialogs::on_dialog_focus(int focus_id) {
    switch (kind_) {
    case 1: case 6: case 11:
        radio_button_view().RadioButton::on_dialog_focus(focus_id);
        break;
    case 2: case 7: case 12:
        list_box_view().ListBox::on_dialog_focus(focus_id);
        break;
    case 3: case 8: case 13:
        check_box_view().CheckBox::on_dialog_focus(focus_id);
        break;
    case 4: case 9: case 14:
        sprite_box_view().SpriteBox::on_dialog_focus(focus_id);
        break;
    case 5: case 10: case 15: case 16:
        edit_group_view().EditGroup::on_dialog_focus(focus_id);
        break;
    }
}

// ORIGINAL: 0x00612FE0 ?attach@Dialogs@@QAEHPAUGraphicWin@@HHH@Z 0x00612FE0-0x006130AE
// RECOVERY_BINDING: v1 {"evidence": ["Retained 0x00612FE0 seed; shared virtual Dialog and GraphicWin bases; Dialogs kind_ and named widget base APIs"], "reason": "Home attach as a typed Dialogs kind dispatcher"}
// symbol    ?attach@Dialogs@@UAEHPAVGraphicWin@@HHH@Z
int Dialogs::attach(GraphicWin *parent, int x, int y, int flags) {
    switch (kind_) {
    case 1: case 2: case 3:
        return radio_button_view().RadioButton::attach(parent, x, y, flags);
    case 4: case 5: case 6:
        return list_box_view().ListBox::attach(parent, x, y, flags);
    case 7: case 8: case 9:
        return check_box_view().CheckBox::attach(parent, x, y, flags);
    case 10: case 11: case 12:
        return sprite_box_view().SpriteBox::attach(parent, x, y, flags);
    case 13: case 14: case 15: case 16:
        return edit_group_view().EditGroup::attach(parent, x, y, flags);
    default:
        return 7;
    }
}

// ORIGINAL: 0x00612970 ?attach@Dialogs@@QAEHHPAUGraphicWin@@HHH@Z 0x00612970-0x00612A48 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence":["Pinned image 0x00612970 masks its first argument with 0x1f, stores the result in Dialogs::kind_, then directly dispatches cases 16, 2, 1, 8 and 4 to RadioButton, ListBox, CheckBox, SpriteBox and EditGroup::attach respectively; the default returns 3.","The source declares those five direct widget bases and their attach(GraphicWin*,int,int,int) methods. Qualified base calls let C++ select the existing subobjects and apply their compiler-known adjustments."],"reason":"Home the kind-selecting attach entry through Dialogs::kind_ and its real widget base subobjects."}
// symbol    ?attach@Dialogs@@QAEHHPAVGraphicWin@@HHH@Z
// LEVER: switch case DECLARATION ORDER matched the jump-table's physical block layout (0x10, 2, 1, 8, 4, default) - not case-value order and not Ghidra's own printed order (1, 2, default, 4, 8, 0x10).
int Dialogs::attach(int kind, GraphicWin *parent, int x, int y, int flags) {
    kind_ = kind & 0x1f;
    switch (kind_) {
    case 16:
        return radio_button_view().RadioButton::attach(parent, x, y, flags);
    case 2:
        return list_box_view().ListBox::attach(parent, x, y, flags);
    case 1:
        return check_box_view().CheckBox::attach(parent, x, y, flags);
    case 8:
        return sprite_box_view().SpriteBox::attach(parent, x, y, flags);
    case 4:
        return edit_group_view().EditGroup::attach(parent, x, y, flags);
    default:
        return 3;
    }
}

// ORIGINAL: 0x00612E00 ?on_redraw@Dialogs@@QAEXXZ 0x00612E00-0x00612E4D
// RECOVERY_BINDING: v1 {"evidence":["Pinned image 0x00612E00 reads the selector from the shared GraphicWin receiver's -8 location, corresponding to Dialogs::kind_ at complete-object +0x180.","The retained seed's direct tail targets identify cases 1/2/4/8/16 as CheckBox/ListBox/EditGroup/SpriteBox/RadioButton::on_redraw. The five typed base views are already declared by Dialogs."],"reason":"Home the redraw kind dispatcher through its named selector and declared widget base subobjects."}
// symbol    ?on_redraw@Dialogs@@UAEXXZ
void Dialogs::on_redraw() {
    switch (kind_) {
    case 1:
        check_box_view().CheckBox::on_redraw();
        break;
    case 2:
        list_box_view().ListBox::on_redraw();
        break;
    case 4:
        edit_group_view().EditGroup::on_redraw();
        break;
    case 8:
        sprite_box_view().SpriteBox::on_redraw();
        break;
    case 16:
        radio_button_view().RadioButton::on_redraw();
        break;
    }
}
