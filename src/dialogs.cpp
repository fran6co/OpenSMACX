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
#include "radiobutton.h"  // RadioButton::close (source-owned)
#include "worldwin.h"
#include "spritebox.h"
#include "net_class.h"
#include "scroll.h"
#include <cstring>

/*
Purpose: Close the dialog manager; the legacy implementation is intentionally empty.
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
// ORIGINAL: 0x00612830 ??0Dialogs@@QAE@H@Z 0x00612830-0x00612AE0;0x00663060-0x006630D3
// RECOVERY_BINDING: v1 {"evidence": ["pinned:prolog 0x612832 registers handler 0x6630C9; FuncInfo 0x67FD68 and unwind map 0x67FD88 contain six HIGHLOW-backed actions in the completely decoded cold region 0x663060..0x6630D3."], "reason": "The constructor owns its partial-construction unwind actions and handler through the original exception metadata; retain their complete generated-lifetime matching obligation."}
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

// ORIGINAL: 0x00612960 ?close@Dialogs@@QAEXXZ 0x00612960-0x00612961 BYTE_EXACT
// kind      game
// symbol    ?close@Dialogs@@QAEXXZ
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
            return list_box_view().ListBox::item(text, index);
        case 1:
        case 4:
        case 8:
        case 16:
            return static_cast<Dialog &>(*this).Dialog::item(text, index);
        default:
            return 0;
    }
}

/*
Purpose: Count the items this kind of dialog holds.
// ORIGINAL: 0x00613740 ?get_num_items@Dialogs@@QAEHXZ 0x00613740-0x00613772 BYTE_EXACT
// LEVER: `dialog_of` marked `__forceinline` - the image has zero call
//        targets here, so its vbtable lookup is inlined, not called.
// The original count read is the shared Dialog::item_list_ entry_count_.
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
            return Dialog::item_list_.entry_count_;
        }
        case 8:
            return SpriteBox::list_.entry_count_;
        default:
            return 0;
    }
}




/*
Purpose: Forward on right down to the embedded widget, but only when the
         active dialog is the sprite-box kind (8). The original interface
         receiver reads kind at -8 and adjusts by -0x8C for SpriteBox.
         Source uses kind_ and the genuine SpriteBox base; compiler-owned
         adjustors retain responsibility for the original interface view.
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
    if (kind_ == 8) {
        sprite_box_view().SpriteBox::on_right_down(a, b);
    }
}

/*
Purpose: Forward on right double click to the embedded widget, but only when the
         active dialog is the sprite-box kind (8). The original interface
         receiver reads kind at -8 and adjusts by -0x8C for SpriteBox.
         Source uses kind_ and the genuine SpriteBox base; compiler-owned
         adjustors retain responsibility for the original interface view.
// ORIGINAL: 0x00612EF0 ?on_right_double_click@Dialogs@@QAEXHH@Z 0x00612EF0-0x00612F0E BYTE_EXACT
// symbol    ?on_right_double_click@Dialogs@@UAEXHH@Z
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
    if (kind_ == 8) {
        sprite_box_view().SpriteBox::on_right_double_click(a, b);
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
         active dialog is the sprite-box kind (8). The original interface
         receiver reads kind at -8 and adjusts by -0x8C for SpriteBox.
         Source uses kind_ and the genuine SpriteBox base; compiler-owned
         adjustors retain responsibility for the original interface view.
// ORIGINAL: 0x00612F40 ?on_right_up@Dialogs@@QAEXHH@Z 0x00612F40-0x00612F5E BYTE_EXACT
// symbol    ?on_right_up@Dialogs@@UAEXHH@Z
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
    if (kind_ == 8) {
        sprite_box_view().SpriteBox::on_right_up(a, b);
    }
}

/*
Purpose: Forward on right click to the embedded widget, but only when the
         active dialog is the sprite-box kind (8). The original interface
         receiver reads kind at -8 and adjusts by -0x8C for SpriteBox.
         Source uses kind_ and the genuine SpriteBox base; compiler-owned
         adjustors retain responsibility for the original interface view.
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
    if (kind_ == 8) {
        sprite_box_view().SpriteBox::on_right_click(a, b);
    }
}

/*
Purpose: Route the scrolled event by the Dialogs kind to its declared
         ListBox base. The generated interface receiver adjustment is
         separate from this ordinary source member's complete-object this.
// ORIGINAL: 0x00612F80 ?on_scrolled@Dialogs@@QAEXHH@Z 0x00612F80-0x00612FA0 BYTE_EXACT
// symbol    ?on_scrolled@Dialogs@@UAEXHH@Z
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
            static_cast<ListBox *>(this)->ListBox::on_scrolling(code, pos);
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
// symbol    ?on_scrolling@Dialogs@@UAEXHH@Z
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
            static_cast<ListBox *>(this)->ListBox::on_scrolling(a, b);
            break;
        default:
            break;
    }
}

/*
Purpose: Forward on mousewheel to the embedded widget, but only when the
         active dialog is the list-box kind (2). The pinned receiver is the
         interface view; this complete-object method reads kind_ and calls
         the actual ListBox base handler.
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
    switch (kind_) {
        case 2:
            list_box_view().ListBox::on_mousewheel(a);
            break;
        default:
            break;
    }
}










/*
 * RECOVERY_HISTORY: v1 address=0x00406910 source_sha256=b9ca59ee315f7c22463bf4789d49e0ed78c55d78f6773313c380cdddcd2de80a envelope_sha256=825c7a3f5bdb2b6a203f894fe3864115c7aeccdbeb950a4d203646540761b48f
 * [ARCHIVE] /*
 * [ARCHIVE] Purpose: Destroy a Dialogs. Stage the Dialogs tables into the GraphicWin/Win
 * [ARCHIVE]          virtual base through that base's own vbtable, run the empty close,
 * [ARCHIVE]          destroy the EditGroup, SpriteBox and CheckBox members through their
 * [ARCHIVE]          original destructors, stage the embedded RadioButton's tables through
 * [ARCHIVE]          ITS own vbtable and run its recovered close, then run the embedded
 * [ARCHIVE]          ListBox's recovered teardown - whose base coincides with the
 * [ARCHIVE]          allocation base, so each staging overwrites the previous tables in
 * [ARCHIVE]          the original's exact order. The original's C++ exception frame
 * [ARCHIVE]          targets __CxxFrameHandler and is omitted as unreachable per policy.
 * [ARCHIVE] // original marker: 0x00406910 ??1Dialogs@@QAE@XZ 0x00406910-0x00406A74;0x006509C0-0x00650AAA
 * [ARCHIVE] // symbol    ?destroy@Dialogs@@QAEIXZ
 * [ARCHIVE] // TRIED: 3/92 - same SEH-frame symptom as `Dialog::~Dialog()` (this
 * [ARCHIVE] //            file's sibling class in dialog.cpp) and FlatButton's own
 * [ARCHIVE] //            constructor/destructor pair. Not attempted: the comment
 * [ARCHIVE] //            above claims the frame is "unreachable, omitted per policy",
 * [ARCHIVE] //            which is UNVERIFIED here (flagged, not trusted - see the
 * [ARCHIVE] //            note on Dialog::~Dialog for why). This body is the most
 * [ARCHIVE] //            complex of the three (six member teardowns across five
 * [ARCHIVE] //            classes chained together); reproducing the frame plus
 * [ARCHIVE] //            whatever member-destruction reordering it would take is out
 * [ARCHIVE] //            of scope for this pass's budget.
 * [ARCHIVE] // size      590 bytes
 * [ARCHIVE] // prototype void (__thiscall ??1Dialogs@@QAE@XZ)(Dialogs* this)
 * [ARCHIVE] // callers   72   call targets   6
 * [ARCHIVE] // kind      game
 * [ARCHIVE] // flags     frame;hidden;sp_ready;purged_ok
 * [ARCHIVE] // calls     0x00609EC0 0x0060D1B0 0x0060E740 0x00610120 0x00611A20 0x00612960
 * [ARCHIVE] Return Value: EAX residue (ListBox::destroy's constant 0); declared void in
 * [ARCHIVE]               the mangled name, modelled as uint32_t like ListBox::destroy.
 * [ARCHIVE] Status: Complete with temporary EditGroup, SpriteBox and CheckBox destructor
 * [ARCHIVE]         dependencies
 * [ARCHIVE] Verification note: every member teardown is guarded on the allocation base
 * [ARCHIVE] being non-null (the original tests edi after `lea edi,[esi-0x188]`); the null
 * [ARCHIVE] branch feeds the bare adjustment offset and is unreachable for any real
 * [ARCHIVE] object, so no test drives it, and the sweep's surviving constant mutants in
 * [ARCHIVE] that branch are equivalent by unreachability. The surviving drop of the
 * [ARCHIVE] close() call is equivalent by construction: Dialogs::close is an empty body.
 * [ARCHIVE] * /
 */
// HISTORICAL NOTE: the archived TRIED text predates the real Dialogs base/member ownership. SpriteBox::close and its Spot/SpriteList members now own this teardown; the old manual member calls are retired.
// ORIGINAL: 0x00406910 ??1Dialogs@@QAE@XZ 0x00406910-0x00406A74;0x006509C0-0x00650AAA FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original complete-object destructor identity 0x00406910 has full spans (4221200, 4221556) and (6621632, 6621866).", "Current genuine owner has the five real direct Dialogs bases and shared virtual Dialog/GraphicWin bases; normal destruction runs reverse direct-base order and one virtual-base chain.", "Fresh baseline `dialogs.cpp.obj` emits the natural non-deleting destructor as ??1Dialogs@@UAE@XZ; adding the explicit close() body requires a fresh post-edit emitted-symbol and body check."], "reason": "Transfer the complete original Dialogs destructor identity to the actual class destructor. Preserve the original close() call; let real C++ bases and virtual bases own their destruction."}
// symbol    ??1Dialogs@@UAE@XZ
// kind      game
// HISTORICAL TRIED (superseded by recovered Dialogs/SpriteBox ownership): same SEH-frame symptom as `Dialog::~Dialog()` (this file's sibling class in dialog.cpp) and FlatButton's own constructor/destructor pair. Not attempted: the comment above claims the frame is "unreachable, omitted per policy", which is UNVERIFIED here (flagged, not trusted - see the note on Dialog::~Dialog for why). This body is the most complex of the three (six member teardowns across five classes chained together); reproducing the frame plus whatever member-destruction reordering it would take is out of scope for this pass's budget.
Dialogs::~Dialogs() {
    close();
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

/*
Purpose: Hand a closing scroll control to the list box: only when the active
         dialog is the list-box kind (2) and the scroll's flags byte has bit
         0x40 set does the scroll's own vtable slot 9 take it, and its return
         is this function's return; otherwise the argument passes back
         unchanged. The pinned receiver walk reaches the shared GraphicWin/Win
         state; `iFlags_` and `win_parent_` name the recovered flag and slot-9
         receiver directly.
// ORIGINAL: 0x00613260 ?on_scroll_delete@Dialogs@@QAEHPAUScroll@@@Z 0x00613260-0x0061329F BYTE_EXACT
// symbol    ?on_scroll_delete@Dialogs@@UAEPAVScroll@@PAV2@@Z
// size      63 bytes
// prototype int (__thiscall ?on_scroll_delete@Dialogs@@QAEHPAUScroll@@@Z)(Dialogs* this, Scroll*)
// callers   0   call targets   0
// kind      game
// indirect  0x00613292
//
// PROMOTED from the archived verification unit. Its slot-9 dispatch and
// pointer pass-back agree with the pinned span; the current owner calls the
// declared Win slot through the recovered parent pointer. The historical
// int return spelling is retained in the original identity; this source uses
// Scroll* because Win::close consumes the returned value as its Scroll*.
Return Value: the scroll, or the slot's answer
Status: Complete
*/
Scroll *Dialogs::on_scroll_delete(Scroll *scroll) {
    switch (kind_) {
    case 2: {
        if (iFlags_ & 0x40) {
            Win *parent = win_parent_;
            if (parent != 0) {
                return parent->on_listbox_scroll_delete(scroll);
            }
        }
    }
    }
    return scroll;
}

/*
Purpose: Forward a left click to the embedded sprite box for every dialog
         kind in the pinned case map (1 through 7 and 9 through 16), to the
         SpriteBox base.
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
    switch (kind_) {
    case 1: case 2: case 3: case 4: case 5: case 6: case 7:
    case 9: case 10: case 11: case 12: case 13: case 14: case 15: case 16:
        sprite_box_view().SpriteBox::on_left_click(a, b);
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
// symbol    ?on_scroll_create@Dialogs@@UAEPAVScroll@@XZ
Scroll *Dialogs::on_scroll_create() {
    if (kind_ == 2 && (iFlags_ & 0x40) != 0 && win_parent_ != 0) {
        return win_parent_->on_listbox_scroll_create();
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

// ORIGINAL: 0x00407200 ?on_right_up@thunk1_Dialogs@@QAEXHH@Z 0x00407200-0x00407208 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned image 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf: Dialogs constructor span contains the store of table 0x00669BE8; the pinned dword at 0x00669C48 is 0x00407200.", "The original 8-byte receiver thunk at 0x00407200 tail-jumps to 0x00612F40, mapped as ?on_right_up@Dialogs@@QAEXHH@Z in this class owner.", "The actual class owner is src/dialogs.cpp; its current owner object vftable relocation at byte offset 0x60 names the generated thunk, whose terminal relocation targets ?on_right_up@Dialogs@@UAEXHH@Z.", "The source identity retains its exact historical kind fact `game`; its full original spans, claims, lessons, and findings are checked unchanged by the writer readback."], "reason": "The pinned Dialogs constructor at 0x00612830 installs its Win view at 0x00669BE8; the original slot cell at +0x60 selects this full identity; it is also the secondary address point 0x00669BD4 plus 0x74. The real Dialogs virtual member and owner-TU vftable are already source-owned; current owner COFF evidence binds this FILE identity to the compiler-emitted receiver adjustor tailing to ?on_right_up@Dialogs@@UAEXHH@Z."}
// kind      game
// symbol    ?on_right_up@Dialogs@@$4PPPPPPPM@A@AEXHH@Z

// ORIGINAL: 0x00407230 ?on_right_double_click@thunk1_Dialogs@@QAEXHH@Z 0x00407230-0x00407238 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned image 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf: Dialogs constructor span contains the store of table 0x00669BE8; the pinned dword at 0x00669C5C is 0x00407230.", "The original 8-byte receiver thunk at 0x00407230 tail-jumps to 0x00612EF0, mapped as ?on_right_double_click@Dialogs@@QAEXHH@Z in this class owner.", "The actual class owner is src/dialogs.cpp; its current owner object vftable relocation at byte offset 0x74 names the generated thunk, whose terminal relocation targets ?on_right_double_click@Dialogs@@UAEXHH@Z.", "The source identity retains its exact historical kind fact `game`; its full original spans, claims, lessons, and findings are checked unchanged by the writer readback."], "reason": "The pinned Dialogs constructor at 0x00612830 installs its Win view at 0x00669BE8; the original slot cell at +0x74 selects this full identity. The real Dialogs virtual member and owner-TU vftable are already source-owned; current owner COFF evidence binds this FILE identity to the compiler-emitted receiver adjustor tailing to ?on_right_double_click@Dialogs@@UAEXHH@Z."}
// kind      game
// symbol    ?on_right_double_click@Dialogs@@$4PPPPPPPM@A@AEXHH@Z

// ORIGINAL: 0x00407240 ?on_scrolled@thunk1_Dialogs@@QAEXHH@Z 0x00407240-0x00407248 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned image 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf: Dialogs constructor span contains the store of table 0x00669BE8; the pinned dword at 0x00669CA8 is 0x00407240.", "The original 8-byte receiver thunk at 0x00407240 tail-jumps to 0x00612F80, mapped as ?on_scrolled@Dialogs@@QAEXHH@Z in this class owner.", "The actual class owner is src/dialogs.cpp; its current owner object vftable relocation at byte offset 0xC0 names the generated thunk, whose terminal relocation targets ?on_scrolled@Dialogs@@UAEXHH@Z.", "The source identity retains its exact historical kind fact `game`; its full original spans, claims, lessons, and findings are checked unchanged by the writer readback."], "reason": "The pinned Dialogs constructor at 0x00612830 installs its Win view at 0x00669BE8; the original slot cell at +0xC0 selects this full identity. The real Dialogs virtual member and owner-TU vftable are already source-owned; current owner COFF evidence binds this FILE identity to the compiler-emitted receiver adjustor tailing to ?on_scrolled@Dialogs@@UAEXHH@Z."}
// kind      game
// symbol    ?on_scrolled@Dialogs@@$4PPPPPPPM@A@AEXHH@Z

// ORIGINAL: 0x00407250 ?on_scrolling@thunk1_Dialogs@@QAEXHH@Z 0x00407250-0x00407258 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned image 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf: Dialogs constructor span contains the store of table 0x00669BE8; the pinned dword at 0x00669CAC is 0x00407250.", "The original 8-byte receiver thunk at 0x00407250 tail-jumps to 0x00612FA0, mapped as ?on_scrolling@Dialogs@@QAEXHH@Z in this class owner.", "The actual class owner is src/dialogs.cpp; its current owner object vftable relocation at byte offset 0xC4 names the generated thunk, whose terminal relocation targets ?on_scrolling@Dialogs@@UAEXHH@Z.", "The source identity retains its exact historical kind fact `game`; its full original spans, claims, lessons, and findings are checked unchanged by the writer readback."], "reason": "The pinned Dialogs constructor at 0x00612830 installs its Win view at 0x00669BE8; the original slot cell at +0xC4 selects this full identity. The real Dialogs virtual member and owner-TU vftable are already source-owned; current owner COFF evidence binds this FILE identity to the compiler-emitted receiver adjustor tailing to ?on_scrolling@Dialogs@@UAEXHH@Z."}
// kind      game
// symbol    ?on_scrolling@Dialogs@@$4PPPPPPPM@A@AEXHH@Z

/*
 * RECOVERY_HISTORY: v1 address=0x00407150 source_sha256=5d8f8a1d88dea99894d068cf22393a1686f86c8deab34d7b504349c1d2069549 envelope_sha256=1c61ef27792b94d97baf7e4eeeb2e729fa6bcaf698d2c35e24918f5d64099826
 * [ARCHIVE] /*
 * [ARCHIVE] Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
 * [ARCHIVE]          Dialogs::on_scroll_create expects, then forward unchanged.
 * [ARCHIVE] // RECOVERY_ROLE: SCAFFOLD Hand-written receiver-adjustment adapter; original compiler thunk still needs a qualified genuine class/view binding.
 * [ARCHIVE] // original marker: 0x00407150 ?on_scroll_create@thunk1_Dialogs@@QAEXXZ 0x00407150-0x00407158 BYTE_EXACT
 * [ARCHIVE] // symbol    ?adjust_dialogs1_on_scroll_create@@YIXPAX0@Z
 * [ARCHIVE] // size      8 bytes
 * [ARCHIVE] // prototype void (__thiscall ?on_scroll_create@thunk1_Dialogs@@QAEXXZ)(Dialogs* this)
 * [ARCHIVE] // callers   0   call targets   0
 * [ARCHIVE] // kind      game
 * [ARCHIVE] // flags     hidden;sp_ready;purged_ok
 * [ARCHIVE] // calls     (none)
 * [ARCHIVE] Return Value: n/a
 * [ARCHIVE] Status: Complete
 * [ARCHIVE] * /
 */
// ORIGINAL: 0x00407150 ?on_scroll_create@thunk1_Dialogs@@QAEXXZ 0x00407150-0x00407158 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original image 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf: constructor-installed class table cell binds 0x00407150 to the complete original thunk span.", "The retained terminal artifact set at build/recovery/builds/build-8b7ms_6y recorded the observed compiler symbol ?on_scroll_create@Dialogs@@$4PPPPPPPM@A@AEHXZ in the class owner's physical COFF vftable relocation.", "That artifact set's MAP/PE cell selects the thunk and its PDB XFIXUP plus linked tail resolve to the mapped typed method.", "The original identity, spans, catalog purpose, source kind, claim state and formal lessons are retained; this binding is not a new byte claim."], "reason": "Bind the preserved original receiver-adjustor identity to the compiler-emitted class-view entry observed in the actual owner object and linked table."}
// symbol    ?on_scroll_create@Dialogs@@$4PPPPPPPM@A@AEPAVScroll@@XZ
// kind      game

/*
 * RECOVERY_HISTORY: v1 address=0x00407160 source_sha256=5d8f8a1d88dea99894d068cf22393a1686f86c8deab34d7b504349c1d2069549 envelope_sha256=18eb40a5eb6529a13a790b90880a878d82f37e3b1324491eb4c2b7f3aadd384b
 * [ARCHIVE] /*
 * [ARCHIVE] Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
 * [ARCHIVE]          Dialogs::on_scroll_delete expects, then forward unchanged.
 * [ARCHIVE] // RECOVERY_ROLE: SCAFFOLD Hand-written receiver-adjustment adapter; original compiler thunk still needs a qualified genuine class/view binding.
 * [ARCHIVE] // original marker: 0x00407160 ?on_scroll_delete@thunk1_Dialogs@@QAEHPAUScroll@@@Z 0x00407160-0x00407168
 * [ARCHIVE] // symbol    ?adjust_dialogs1_on_scroll_delete@@YIHPAX00@Z
 * [ARCHIVE] // size      8 bytes
 * [ARCHIVE] // prototype int (__thiscall ?on_scroll_delete@thunk1_Dialogs@@QAEHPAUScroll@@@Z)(Dialogs* this, Scroll*)
 * [ARCHIVE] // callers   0   call targets   0
 * [ARCHIVE] // kind      game
 * [ARCHIVE] // flags     hidden;sp_ready;purged_ok
 * [ARCHIVE] // calls     (none)
 * [ARCHIVE] Return Value: the forwarded call's
 * [ARCHIVE] Status: Complete
 * [ARCHIVE] * /
 */
// ORIGINAL: 0x00407160 ?on_scroll_delete@thunk1_Dialogs@@QAEHPAUScroll@@@Z 0x00407160-0x00407168 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original image 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf: constructor-installed class table cell binds 0x00407160 to the complete original thunk span.", "The retained terminal artifact set at build/recovery/builds/build-8b7ms_6y recorded the observed compiler symbol ?on_scroll_delete@Dialogs@@$4PPPPPPPM@A@AEHPAVScroll@@@Z in the class owner's physical COFF vftable relocation.", "That artifact set's MAP/PE cell selects the thunk and its PDB XFIXUP plus linked tail resolve to the mapped typed method.", "The original identity, spans, catalog purpose, source kind, claim state and formal lessons are retained; this binding is not a new byte claim."], "reason": "Bind the preserved original receiver-adjustor identity to the compiler-emitted class-view entry observed in the actual owner object and linked table."}
// symbol    ?on_scroll_delete@Dialogs@@$4PPPPPPPM@A@AEPAVScroll@@PAV2@@Z
// kind      game

/*
 * RECOVERY_HISTORY: v1 address=0x00407180 source_sha256=5d8f8a1d88dea99894d068cf22393a1686f86c8deab34d7b504349c1d2069549 envelope_sha256=eec3a684474f320524f5a5a5646f28e479addd3d27da661c7df6fd78bb664400
 * [ARCHIVE] /*
 * [ARCHIVE] Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
 * [ARCHIVE]          Dialogs::on_mousewheel expects, then forward unchanged.
 * [ARCHIVE] // RECOVERY_ROLE: SCAFFOLD Hand-written receiver-adjustment adapter; original compiler thunk still needs a qualified genuine class/view binding.
 * [ARCHIVE] // original marker: 0x00407180 ?on_mousewheel@thunk1_Dialogs@@QAEXH@Z 0x00407180-0x00407188
 * [ARCHIVE] // symbol    ?adjust_dialogs1_on_mousewheel@@YIXPAX0H@Z
 * [ARCHIVE] // size      8 bytes
 * [ARCHIVE] // prototype void (__thiscall ?on_mousewheel@thunk1_Dialogs@@QAEXH@Z)(Dialogs* this, int)
 * [ARCHIVE] // callers   0   call targets   0
 * [ARCHIVE] // kind      game
 * [ARCHIVE] // flags     hidden;sp_ready;purged_ok
 * [ARCHIVE] // calls     (none)
 * [ARCHIVE] Return Value: n/a
 * [ARCHIVE] Status: Complete
 * [ARCHIVE] * /
 */
// ORIGINAL: 0x00407180 ?on_mousewheel@thunk1_Dialogs@@QAEXH@Z 0x00407180-0x00407188 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original image 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf: constructor-installed class table cell binds 0x00407180 to the complete original thunk span.", "The retained terminal artifact set at build/recovery/builds/build-8b7ms_6y recorded the observed compiler symbol ?on_mousewheel@Dialogs@@$4PPPPPPPM@A@AEXH@Z in the class owner's physical COFF vftable relocation.", "That artifact set's MAP/PE cell selects the thunk and its PDB XFIXUP plus linked tail resolve to the mapped typed method.", "The original identity, spans, catalog purpose, source kind, claim state and formal lessons are retained; this binding is not a new byte claim."], "reason": "Bind the preserved original receiver-adjustor identity to the compiler-emitted class-view entry observed in the actual owner object and linked table."}
// symbol    ?on_mousewheel@Dialogs@@$4PPPPPPPM@A@AEXH@Z
// kind      game

/*
 * RECOVERY_HISTORY: v1 address=0x004071B0 source_sha256=5d8f8a1d88dea99894d068cf22393a1686f86c8deab34d7b504349c1d2069549 envelope_sha256=311d6ed9fbd175ec5ccbbefdccb5abeca800bcc344d9479e6d9a6fbb18da1ed3
 * [ARCHIVE] /*
 * [ARCHIVE] Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
 * [ARCHIVE]          Dialogs::on_left_click expects, then forward unchanged.
 * [ARCHIVE] // RECOVERY_ROLE: SCAFFOLD Hand-written receiver-adjustment adapter; original compiler thunk still needs a qualified genuine class/view binding.
 * [ARCHIVE] // original marker: 0x004071B0 ?on_left_click@thunk1_Dialogs@@QAEXHH@Z 0x004071B0-0x004071B8
 * [ARCHIVE] // symbol    ?adjust_dialogs1_on_left_click@@YIXPAX0HH@Z
 * [ARCHIVE] // size      8 bytes
 * [ARCHIVE] // prototype void (__thiscall ?on_left_click@thunk1_Dialogs@@QAEXHH@Z)(Dialogs* this, int, int)
 * [ARCHIVE] // callers   0   call targets   0
 * [ARCHIVE] // kind      game
 * [ARCHIVE] // flags     hidden;sp_ready;purged_ok
 * [ARCHIVE] // calls     (none)
 * [ARCHIVE] Return Value: n/a
 * [ARCHIVE] Status: Complete
 * [ARCHIVE] * /
 */
// ORIGINAL: 0x004071B0 ?on_left_click@thunk1_Dialogs@@QAEXHH@Z 0x004071B0-0x004071B8 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original image 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf: constructor-installed class table cell binds 0x004071B0 to the complete original thunk span.", "The retained terminal artifact set at build/recovery/builds/build-8b7ms_6y recorded the observed compiler symbol ?on_left_click@Dialogs@@$4PPPPPPPM@A@AEXHH@Z in the class owner's physical COFF vftable relocation.", "That artifact set's MAP/PE cell selects the thunk and its PDB XFIXUP plus linked tail resolve to the mapped typed method.", "The original identity, spans, catalog purpose, source kind, claim state and formal lessons are retained; this binding is not a new byte claim."], "reason": "Bind the preserved original receiver-adjustor identity to the compiler-emitted class-view entry observed in the actual owner object and linked table."}
// symbol    ?on_left_click@Dialogs@@$4PPPPPPPM@A@AEXHH@Z
// kind      game

/*
 * RECOVERY_HISTORY: v1 address=0x004071E0 source_sha256=5d8f8a1d88dea99894d068cf22393a1686f86c8deab34d7b504349c1d2069549 envelope_sha256=04ac05c124f77eaaa62921668d02a1c75f9ba667e2633e70a70be460a890422c
 * [ARCHIVE] /*
 * [ARCHIVE] Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
 * [ARCHIVE]          Dialogs::on_right_click expects, then forward unchanged.
 * [ARCHIVE] // RECOVERY_ROLE: SCAFFOLD Hand-written receiver-adjustment adapter; original compiler thunk still needs a qualified genuine class/view binding.
 * [ARCHIVE] // original marker: 0x004071E0 ?on_right_click@thunk1_Dialogs@@QAEXHH@Z 0x004071E0-0x004071E8
 * [ARCHIVE] // symbol    ?adjust_dialogs1_on_right_click@@YIXPAX0HH@Z
 * [ARCHIVE] // size      8 bytes
 * [ARCHIVE] // prototype void (__thiscall ?on_right_click@thunk1_Dialogs@@QAEXHH@Z)(Dialogs* this, int, int)
 * [ARCHIVE] // callers   0   call targets   0
 * [ARCHIVE] // kind      game
 * [ARCHIVE] // flags     hidden;sp_ready;purged_ok
 * [ARCHIVE] // calls     (none)
 * [ARCHIVE] Return Value: n/a
 * [ARCHIVE] Status: Complete
 * [ARCHIVE] * /
 */
// ORIGINAL: 0x004071E0 ?on_right_click@thunk1_Dialogs@@QAEXHH@Z 0x004071E0-0x004071E8 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original image 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf: constructor-installed class table cell binds 0x004071E0 to the complete original thunk span.", "The retained terminal artifact set at build/recovery/builds/build-8b7ms_6y recorded the observed compiler symbol ?on_right_click@Dialogs@@$4PPPPPPPM@A@AEXHH@Z in the class owner's physical COFF vftable relocation.", "That artifact set's MAP/PE cell selects the thunk and its PDB XFIXUP plus linked tail resolve to the mapped typed method.", "The original identity, spans, catalog purpose, source kind, claim state and formal lessons are retained; this binding is not a new byte claim."], "reason": "Bind the preserved original receiver-adjustor identity to the compiler-emitted class-view entry observed in the actual owner object and linked table."}
// symbol    ?on_right_click@Dialogs@@$4PPPPPPPM@A@AEXHH@Z
// kind      game

/*
 * RECOVERY_HISTORY: v1 address=0x00407210 source_sha256=5d8f8a1d88dea99894d068cf22393a1686f86c8deab34d7b504349c1d2069549 envelope_sha256=70dac72bf15018a278389576c3676553978f6797532eb074f8425aab32a3065f
 * [ARCHIVE] /*
 * [ARCHIVE] Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
 * [ARCHIVE]          Dialogs::on_key_down expects, then forward unchanged.
 * [ARCHIVE] // RECOVERY_ROLE: SCAFFOLD Hand-written receiver-adjustment adapter; original compiler thunk still needs a qualified genuine class/view binding.
 * [ARCHIVE] // original marker: 0x00407210 ?on_key_down@thunk1_Dialogs@@QAEHH@Z 0x00407210-0x00407218
 * [ARCHIVE] // symbol    ?adjust_dialogs1_on_key_down@@YIHPAX0H@Z
 * [ARCHIVE] // size      8 bytes
 * [ARCHIVE] // prototype int (__thiscall ?on_key_down@thunk1_Dialogs@@QAEHH@Z)(Dialogs* this, int)
 * [ARCHIVE] // callers   0   call targets   0
 * [ARCHIVE] // kind      game
 * [ARCHIVE] // flags     hidden;sp_ready;purged_ok
 * [ARCHIVE] // calls     (none)
 * [ARCHIVE] Return Value: the forwarded call's
 * [ARCHIVE] Status: Complete
 * [ARCHIVE] * /
 */
// ORIGINAL: 0x00407210 ?on_key_down@thunk1_Dialogs@@QAEHH@Z 0x00407210-0x00407218 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original image 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf: constructor-installed class table cell binds 0x00407210 to the complete original thunk span.", "The retained terminal artifact set at build/recovery/builds/build-8b7ms_6y recorded the observed compiler symbol ?on_key_down@Dialogs@@$4PPPPPPPM@A@AEHH@Z in the class owner's physical COFF vftable relocation.", "That artifact set's MAP/PE cell selects the thunk and its PDB XFIXUP plus linked tail resolve to the mapped typed method.", "The original identity, spans, catalog purpose, source kind, claim state and formal lessons are retained; this binding is not a new byte claim."], "reason": "Bind the preserved original receiver-adjustor identity to the compiler-emitted class-view entry observed in the actual owner object and linked table."}
// symbol    ?on_key_down@Dialogs@@$4PPPPPPPM@A@AEHH@Z
// kind      game

/*
 * RECOVERY_HISTORY: v1 address=0x004072B0 source_sha256=5d8f8a1d88dea99894d068cf22393a1686f86c8deab34d7b504349c1d2069549 envelope_sha256=8e2b34215067873fa587d91183d644ed7c4f81587bc029482942fc941f96b566
 * [ARCHIVE] /*
 * [ARCHIVE] Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
 * [ARCHIVE]          Dialogs::attach expects, then forward unchanged.
 * [ARCHIVE] // RECOVERY_ROLE: SCAFFOLD Hand-written receiver-adjustment adapter; original compiler thunk still needs a qualified genuine class/view binding.
 * [ARCHIVE] // original marker: 0x004072B0 ?attach@thunk1_Dialogs@@QAEHPAUGraphicWin@@HHH@Z 0x004072B0-0x004072B8
 * [ARCHIVE] // symbol    ?adjust_dialogs1_attach@@YIHPAX00HHH@Z
 * [ARCHIVE] // size      8 bytes
 * [ARCHIVE] // prototype int (__thiscall ?attach@thunk1_Dialogs@@QAEHPAUGraphicWin@@HHH@Z)(Dialogs* this, GraphicWin*, int, int, int)
 * [ARCHIVE] // callers   0   call targets   0
 * [ARCHIVE] // kind      game
 * [ARCHIVE] // flags     hidden;sp_ready;purged_ok
 * [ARCHIVE] // calls     (none)
 * [ARCHIVE] Return Value: the forwarded call's
 * [ARCHIVE] Status: Complete
 * [ARCHIVE] * /
 */
// ORIGINAL: 0x004072B0 ?attach@thunk1_Dialogs@@QAEHPAUGraphicWin@@HHH@Z 0x004072B0-0x004072B8 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original image 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf: constructor-installed class table cell binds 0x004072B0 to the complete original thunk span.", "The retained terminal artifact set at build/recovery/builds/build-8b7ms_6y recorded the observed compiler symbol ?attach@Dialogs@@$4PPPPPPPM@A@AEHPAVGraphicWin@@HHH@Z in the class owner's physical COFF vftable relocation.", "That artifact set's MAP/PE cell selects the thunk and its PDB XFIXUP plus linked tail resolve to the mapped typed method.", "The original identity, spans, catalog purpose, source kind, claim state and formal lessons are retained; this binding is not a new byte claim."], "reason": "Bind the preserved original receiver-adjustor identity to the compiler-emitted class-view entry observed in the actual owner object and linked table."}
// symbol    ?attach@Dialogs@@$4PPPPPPPM@A@AEHPAVGraphicWin@@HHH@Z
// kind      game

/*
 * RECOVERY_HISTORY: v1 address=0x00407100 source_sha256=b9ca59ee315f7c22463bf4789d49e0ed78c55d78f6773313c380cdddcd2de80a envelope_sha256=f6c92f763fd2071956b26fc3f28fe0498d7ac5e2e01b32f42563ad5d7993cc09
 * [ARCHIVE] /*
 * [ARCHIVE] Purpose: The compiler-generated scalar deleting destructor: run ~Dialogs, the
 * [ARCHIVE]          trailing Dialog's recovered destructor, and the GraphicWin virtual
 * [ARCHIVE]          base's recovered destructor, then free the allocation base through
 * [ARCHIVE]          the executable's operator delete only when bit 0 of the mode asks.
 * [ARCHIVE]          Always returns the allocation base (this - 0x188), the original's
 * [ARCHIVE]          EAX. The fixed +0xBA0/+0x188 subobject offsets are complete-object
 * [ARCHIVE]          facts: this thunk is only ever entered on a most-derived Dialogs.
 * [ARCHIVE] // original marker: 0x00407100 ??_GDialogs@@UAEPAXI@Z 0x00407100-0x00407141
 * [ARCHIVE] // symbol    ?dialogs_scalar_dtor_redirect@@YIPAXPAX0I@Z
 * [ARCHIVE] // size      65 bytes
 * [ARCHIVE] // prototype void* (__thiscall ??_GDialogs@@UAEPAXI@Z)(Dialogs* this, unsigned int)
 * [ARCHIVE] // callers   0   call targets   4
 * [ARCHIVE] // kind      game
 * [ARCHIVE] // flags     frame;hidden;sp_ready;purged_ok
 * [ARCHIVE] // calls     0x00406910 0x005D4DD0 0x00608E10 0x0064557F
 * [ARCHIVE] Return Value: the allocation base
 * [ARCHIVE] Status: Complete
 * [ARCHIVE] * /
 */

/*
 * RECOVERY_HISTORY: v1 address=0x004070F0 source_sha256=1f689b45149ddffc4ecf76f0f96de5afd71750e7583a10b2280e620778ea1744 envelope_sha256=b867910ad5b2481ef1c45a4fc9b3bec9c5dcebb3918f46b1aaabe77bf098b580
 * [ARCHIVE] /*
 * [ARCHIVE] Purpose: Adjust the receiver from Dialogs's thunk1 subobject to the one
 * [ARCHIVE]          Dialogs::scalar_delete expects, then forward unchanged.
 * [ARCHIVE] // RECOVERY_ROLE: SCAFFOLD Hand-written receiver-adjustment adapter; original compiler thunk still needs a qualified genuine class/view binding.
 * [ARCHIVE] // original marker: 0x004070F0 ??_Gthunk1_Dialogs@@UAEPAXI@Z 0x004070F0-0x004070F8
 * [ARCHIVE] // symbol    ?adjust_dialogs1_scalar_delete@@YIPAXPAX0I@Z
 * [ARCHIVE] // size      8 bytes
 * [ARCHIVE] // prototype void* (__thiscall ??_Gthunk1_Dialogs@@UAEPAXI@Z)(Dialogs* this, unsigned int)
 * [ARCHIVE] // callers   0   call targets   0
 * [ARCHIVE] // kind      game
 * [ARCHIVE] // flags     hidden;sp_ready;purged_ok
 * [ARCHIVE] // calls     (none)
 * [ARCHIVE] Return Value: the forwarded call's
 * [ARCHIVE] Status: Complete
 * [ARCHIVE] * /
 */

/*
 * RECOVERY_HISTORY: v1 address=0x00407280 source_sha256=1f689b45149ddffc4ecf76f0f96de5afd71750e7583a10b2280e620778ea1744 envelope_sha256=55cd2124c2bfa6ec11be5acbd66ee4fdda7f5704e6167015c2942ef1c466bd5f
 * [ARCHIVE] /*
 * [ARCHIVE] Purpose: Adjust the receiver from Dialogs's thunk2 subobject to the one
 * [ARCHIVE]          Dialogs::scalar_delete expects, then forward unchanged.
 * [ARCHIVE] // RECOVERY_ROLE: SCAFFOLD Hand-written receiver-adjustment adapter; original compiler thunk still needs a qualified genuine class/view binding.
 * [ARCHIVE] // original marker: 0x00407280 ??_Gthunk2_Dialogs@@UAEPAXI@Z 0x00407280-0x00407291
 * [ARCHIVE] // symbol    ?adjust_dialogs2_scalar_delete@@YIPAXPAX0I@Z
 * [ARCHIVE] // size      17 bytes
 * [ARCHIVE] // prototype void* (__thiscall ??_Gthunk2_Dialogs@@UAEPAXI@Z)(Dialogs* this, unsigned int)
 * [ARCHIVE] // callers   0   call targets   0
 * [ARCHIVE] // kind      game
 * [ARCHIVE] // flags     hidden;sp_ready;purged_ok
 * [ARCHIVE] // calls     (none)
 * [ARCHIVE] Return Value: the forwarded call's
 * [ARCHIVE] Status: Complete
 * [ARCHIVE] * /
 */

/*
 * RECOVERY_HISTORY: v1 address=0x004072A0 source_sha256=1f689b45149ddffc4ecf76f0f96de5afd71750e7583a10b2280e620778ea1744 envelope_sha256=98cca5e449d7b140cf58b902a229006cfdc9e9b39c5699a1b1fe0d4df3abfa8a
 * [ARCHIVE] /*
 * [ARCHIVE] Purpose: Adjust the receiver from Dialogs's thunk3 subobject to the one
 * [ARCHIVE]          Dialogs::scalar_delete expects, then forward unchanged.
 * [ARCHIVE] // RECOVERY_ROLE: SCAFFOLD Hand-written receiver-adjustment adapter; original compiler thunk still needs a qualified genuine class/view binding.
 * [ARCHIVE] // original marker: 0x004072A0 ??_Gthunk3_Dialogs@@UAEPAXI@Z 0x004072A0-0x004072AE
 * [ARCHIVE] // symbol    ?adjust_dialogs3_scalar_delete@@YIPAXPAX0I@Z
 * [ARCHIVE] // size      14 bytes
 * [ARCHIVE] // prototype void* (__thiscall ??_Gthunk3_Dialogs@@UAEPAXI@Z)(Dialogs* this, unsigned int)
 * [ARCHIVE] // callers   0   call targets   0
 * [ARCHIVE] // kind      game
 * [ARCHIVE] // flags     hidden;sp_ready;purged_ok
 * [ARCHIVE] // calls     (none)
 * [ARCHIVE] Return Value: the forwarded call's
 * [ARCHIVE] Status: Complete
 * [ARCHIVE] * /
 */
// ORIGINAL: 0x00407100 ??_GDialogs@@UAEPAXI@Z 0x00407100-0x00407141 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original image 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf: the primary Dialogs deleting-entry identity is 0x00407100.", "Fresh baseline build `build-cwf5qovz` owner COFF emits ??_GDialogs@@UAEPAXI@Z as the Dialogs primary deleting entry.", "This generated symbol is a baseline expectation only; the post-edit build must reselect the actual current symbol and linked target."], "reason": "Bind the preserved primary scalar-deleting identity to the real Dialogs compiler deleting owner; current emitted identity is provisional until the post-edit native check."}
// symbol    ??_GDialogs@@UAEPAXI@Z
// kind      game
// ORIGINAL: 0x004070F0 ??_Gthunk1_Dialogs@@UAEPAXI@Z 0x004070F0-0x004070F8 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original image 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf: constructor-installed table address point 0x00669BE8 selects original deleting thunk 0x004070F0.", "Fresh baseline `dialogs.cpp.obj` table ??_7Dialogs@@6BWin@@@ first absolute relocation selects ??_EDialogs@@$4PPPPPPPM@A@AEPAXI@Z; that E COMDAT's terminal REL32 selects ??_GDialogs@@UAEPAXI@Z.", "The generated E COMDAT is a baseline expectation; re-read post-edit COFF/MAP/PE/PDB before treating ??_EDialogs@@$4PPPPPPPM@A@AEPAXI@Z as current."], "reason": "Bind this complete original adjustor identity to the genuine Dialogs class-view deleting entry selected by its constructor-installed table; refresh the emitted-symbol fact after the post-edit build."}
// symbol    ??_EDialogs@@$4PPPPPPPM@A@AEPAXI@Z
// kind      game
// ORIGINAL: 0x00407280 ??_Gthunk2_Dialogs@@UAEPAXI@Z 0x00407280-0x00407291 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original image 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf: constructor-installed table address point 0x00669BE0 selects original deleting thunk 0x00407280.", "Fresh baseline `dialogs.cpp.obj` table ??_7Dialogs@@6BBuffer@@@ first absolute relocation selects ??_EDialogs@@$4PPPPPLLI@EEE@AEPAXI@Z; that E COMDAT's terminal REL32 selects ??_GDialogs@@UAEPAXI@Z.", "The generated E COMDAT is a baseline expectation; re-read post-edit COFF/MAP/PE/PDB before treating ??_EDialogs@@$4PPPPPLLI@EEE@AEPAXI@Z as current."], "reason": "Bind this complete original adjustor identity to the genuine Dialogs class-view deleting entry selected by its constructor-installed table; refresh the emitted-symbol fact after the post-edit build."}
// symbol    ??_EDialogs@@$4PPPPPLLI@EEE@AEPAXI@Z
// kind      game
// ORIGINAL: 0x004072A0 ??_Gthunk3_Dialogs@@UAEPAXI@Z 0x004072A0-0x004072AE FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original image 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf: constructor-installed table address point 0x00669BD4 selects original deleting thunk 0x004072A0.", "Fresh baseline `dialogs.cpp.obj` table ??_7Dialogs@@6B@ first absolute relocation selects ??_EDialogs@@$4PPPPPPPM@KBI@AEPAXI@Z; that E COMDAT's terminal REL32 selects ??_GDialogs@@UAEPAXI@Z.", "The generated E COMDAT is a baseline expectation; re-read post-edit COFF/MAP/PE/PDB before treating ??_EDialogs@@$4PPPPPPPM@KBI@AEPAXI@Z as current."], "reason": "Bind this complete original adjustor identity to the genuine Dialogs class-view deleting entry selected by its constructor-installed table; refresh the emitted-symbol fact after the post-edit build."}
// symbol    ??_EDialogs@@$4PPPPPPPM@KBI@AEPAXI@Z
// kind      game
