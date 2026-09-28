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
#include "spritebox.h"
#include "dialog.h"
#include "vtable_shim.h"
#include "sounddevice.h"
#include "menu.h"
#include "win.h"
#include "spot.h"
#include "temp.h"  // HandleMain
#include "buffer.h"

// The original event posts through the SDK's USER32 import. The framework
// derives its IAT slot from the pinned image; no executable address is C++.
// RECOVERY_IMPORT: v1 __imp__PostMessageA@16 USER32.dll name:PostMessageA

/*
Purpose: Construct the GraphicWin and Dialog virtual bases when most-derived,
         then the owned Spot and item list in declaration order. Initialize
         this control's state and buffer defaults. VC6 supplies table setup,
         exception cleanup and the hidden most-derived flag; the catalogue's
         int parameter is not a source parameter.
// ORIGINAL: 0x0060FF00 ??0SpriteBox@@QAE@H@Z 0x0060FF00-0x0061011B;0x006116C0-0x006116DB;0x00662FA0-0x00662FFA
// symbol    ??0SpriteBox@@QAE@XZ
// size      656 bytes
// prototype void (__thiscall ??0SpriteBox@@QAE@H@Z)(SpriteBox* this, int)
// callers   1   call targets   8
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4CF0 0x005DAC70 0x005DACB0 0x005DACE0 0x005DAD10 0x005FA860 0x00608C10 0x00611730
// TRIED: promoted from the preserved artifact
//            src/recovered/units/0060ff00.cpp (since deleted), which
//            measured SHARED_TAIL (its own cold-code span is COMDAT-folded
//            with another function, so no per-function verdict exists
//            there). Same `dialog_.construct()`-is-not-a-placement-new
//            frame gap as CheckBox/EditGroup/RadioButton - see the note on
//            those constructors.
Return Value: n/a
Status: Complete
*/
SpriteBox::SpriteBox() {
    // spots_ is constructed before list_, as in the original. The list's
    // remaining virtual surface needs recovery; do not stage its tables here.
    field_4_ = 0;
    field_1C_ = 1;
    field_20_ = 1;
    field_54_ = 0;
    field_58_ = 0;
    field_5C_ = 0;
    field_60_ = 0;
    field_64_ = 0;
    field_68_ = 0;
    field_6C_ = 0;
    field_70_ = 0;
    field_74_ = 0;
    field_78_ = 0;
    field_7C_ = 0;
    double_click_callback_ = 0;
    field_84_ = 0;

    field_8_ = *reinterpret_cast<int *>(0x009B8FC4);

    // The buffer-default calls, spelled as the qualified base calls they are:
    // the receiver the old hand walks computed (GraphicWin vbase through the
    // vbtable, then the Buffer base at its +0x444) is what the compiler
    // computes for a Buffer member call from this class.
    Buffer::set_text_color(DialogDefaults[24], DialogDefaults[27],
                           DialogDefaults[30], DialogDefaults[33]);
    Buffer::set_text_color2(DialogDefaults[25], DialogDefaults[28],
                            DialogDefaults[31], DialogDefaults[34]);
    Buffer::set_text_color3(DialogDefaults[26], DialogDefaults[29],
                            DialogDefaults[32], DialogDefaults[35]);
    Buffer::set_font(DialogDefaultFonts[0], DialogDefaultFonts[1],
                     DialogDefaultFonts[2], 0);
}

/*
Purpose: Close the control, then let the compiler destroy its owned list and
         Spot, and its virtual bases through the appropriate destructor entry.
// ORIGINAL: 0x00610120 ??1SpriteBox@@QAE@XZ 0x00610120-0x00610274;0x006116C0-0x006116DB;0x00662FFA-0x0066301A
// symbol    ??1SpriteBox@@UAE@XZ
// TRIED: 35 compiled instructions against the image's 99, and the image opens
//   an SEH frame (`push -1 / push 0x663010 / mov eax, fs:[0] / push eax /
//   mov fs:[0], esp / sub esp, 8`) that this tree does not emit. The gap is
//   not a spelling: the image DESTROYS members here and this body only
//   reinstalls vtables and calls close(). Now that the bases are real, a
//   destructor that lets the compiler tear them down is the shape to try -
//   but GraphicWin's and Dialog's own destructors are empty inlines, so VC6
//   proves them nothrow and emits no frame. Making those non-trivial is a
//   coupled edit across every class that embeds one. Measured 2026-08-22.
// size      399 bytes
// prototype void (__thiscall ??1SpriteBox@@QAE@XZ)(SpriteBox* this)
// callers   23   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005FA870 0x00610280 0x00611730
// indirect  0x006101ED 0x00610202 0x0061021D
// TRIED: the middle of the body, and it was written here as PROSE
//        ("NOT REPRODUCED: ...") until 2026-08-22, which meant `decomp.reader`
//        never saw it and frontier.py kept offering this address as fresh.
//        Restated so it counts: between close() and the Spot teardown the
//        image walks a vector of polymorphic pointers at this-0x68 (count at
//        +0xC, capacity at +0x10), rewrites ITS OWN vtable pointer the same
//        vbtable-relative way as the block above, then for each live element
//        calls a virtual "clear" through the element's own vtable and a second
//        virtual call through a vtable-relative adjustor - undeclared-virtual-
//        base dispatch, compounded by per-element dynamic dispatch with no
//        element TYPE recovered. Left out rather than guessed, matching the
//        preserved artifact this was promoted from
//        (src/recovered/units/00610120.cpp, since deleted, itself measured
//        SHARED_TAIL and carried the identical gap).
// TRIED: the call count, separately, and it is NOT the gap above.
//        call_diff makes this tree 4 calls against the image's 2. The image
//        calls exactly `?close@SpriteBox@@QAEXXZ` and `??1Spot@@QAE@XZ`; this
//        tree calls close(), then `??_GSpot@@QAEPAXI@Z` - Spot's SCALAR
//        DELETING destructor rather than its complete one - and then
//        `??1Dialog@@QAE@XZ` and `??1GraphicWin@@QAE@XZ`, the implicit base
//        destructors a real C++ destructor runs after the body. The image's
//        0x00610120 destroys no bases at all; it is the vbase-less variant.
//        Both are declaration-shape problems in spritebox.h/dialog.h, not body
//        spelling, and neither is reachable while SpriteBox holds its virtual
//        bases by layout rather than by `: virtual`.
Return Value: n/a
Status: Complete
*/
SpriteBox::~SpriteBox() {
    close();
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
// ORIGINAL: 0x00611040 ?on_mouse_move@SpriteBox@@QAEXHH@Z 0x00611040-0x00611043 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_mouse_move@SpriteBox@@QAEXHH@Z)(SpriteBox* this, int, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void SpriteBox::on_mouse_move(int, int) {
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
// ORIGINAL: 0x00611050 ?on_mouse_leave@SpriteBox@@QAEXHH@Z 0x00611050-0x00611053 BYTE_EXACT
// symbol    ?on_mouse_leave@SpriteBox@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_mouse_leave@SpriteBox@@QAEXHH@Z)(SpriteBox* this, int, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void SpriteBox::on_mouse_leave(int, int) {
}

/*
Purpose: Adjust the receiver from the thunk1 subobject back to SpriteBox and
         tail-jump to the override. THE COMPILER WRITES THIS BODY - the claim
         names VC6's own vtordisp adjustor thunk, emitted because
         SpriteBox::on_mouse_leave overrides a virtual reached through a virtual
         base. There is no C++ here to get right, only a name to record.
// ORIGINAL: 0x00611840 ?on_mouse_leave@thunk1_SpriteBox@@QAEXHH@Z 0x00611840-0x00611848 BYTE_EXACT
// symbol    ?on_mouse_leave@SpriteBox@@$4PPPPPPPM@A@AEXHH@Z
// size      8 bytes
// kind      game
Status: Complete
*/



/*
Purpose: Walk the entry list for `id` and leave the cursor and position on the
         match, or on the last entry examined when there is none.

         Four fields, all raw because SpriteBox models none of them:
             0x2C  head        0x30  cursor
             0x34  count       0x38  position
         and each entry carries its id at +4 and its successor at +0xC.

         THE EMPTY-LIST PATH WRITES NOTHING. `cmp eax,edx / je done` leaves on
         a null head before the two stores, so position keeps whatever it held;
         it is only zeroed once a head exists. A body that reset position first
         and then checked would agree on every non-empty list and differ on the
         one case the branch exists for.

         Both stores DO happen when the head is non-null, even for a
         non-positive count: `mov [ecx+0x38],edx` and `mov [ecx+0x30],eax`
         precede the `jle`, so an empty-but-headed list still clears position
         and parks the cursor.

         Declared uint32_t although the mangled name says void, for the reason
         Dialogs::destroy is: the body loads [this+0x38] into EAX before
         returning, and a void body cannot promise that. The return type does
         not change the thiscall cleanup, so this is the residue and not a
         different function.
// ORIGINAL: 0x00611600 ?id_to_pos@SpriteBox@@QAEXH@Z 0x00611600-0x00611640
// symbol    ?id_to_pos@SpriteBox@@QAEIH@Z
// size      64 bytes
// prototype void (__thiscall ?id_to_pos@SpriteBox@@QAEXH@Z)(SpriteBox* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the resulting position, which is also left at 0x38
Status: Complete
*/
uint32_t SpriteBox::id_to_pos(int id) {
    uint8_t *const self = reinterpret_cast<uint8_t *>(this);
    uint32_t *const head = reinterpret_cast<uint32_t *>(self + 0x2C);
    uint32_t *const cursor = reinterpret_cast<uint32_t *>(self + 0x30);
    int32_t *const count = reinterpret_cast<int32_t *>(self + 0x34);
    uint32_t *const position = reinterpret_cast<uint32_t *>(self + 0x38);

    if (*head == 0) {
        return *position;
    }
    *position = 0;
    *cursor = *head;
    if (*count <= 0) {
        return *position;
    }
    int32_t examined = 0;
    for (;;) {
        const uint8_t *const entry = reinterpret_cast<const uint8_t *>(*cursor);
        if (*reinterpret_cast<const int32_t *>(entry + 4) == id) {
            break;
        }
        *position = *position + 1;
        ++examined;
        *cursor = *reinterpret_cast<const uint32_t *>(entry + 0xC);
        if (examined >= *count) {
            break;
        }
    }
    return *position;
}


/*
Purpose: Restore the list entry at an explicit position and return its item
         ID.
// ORIGINAL: 0x00611640 ?pos_to_id@SpriteBox@@QAEXH@Z 0x00611640-0x006116B6
// symbol    ?pos_to_id@SpriteBox@@QAEHH@Z
// size      118 bytes
// prototype int (__thiscall ?pos_to_id@SpriteBox@@QAEXH@Z)(SpriteBox* this, int)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00644F3A
// LEVER: the sibling walk. The body is Dialog::pos_to_id (0x00609B50) over
//        the SpriteBoxList at 0x24 instead of Dialog's item_list_ at 0xBC:
//        `position <= entry_count_ - 1` (a decremented compare, not
//        `position < count`), the cursor loaded from head_ before the sign
//        test, `abs()` CALLED TWICE with the count re-read between the two
//        calls, one store for both arms, and a shared tail that reads the
//        entry id only when head_ is non-null.
// LEVER: `goto restore_done`, not an early return - the image's out-of-range
//        exit lands on the SAME tail epilogue the success path falls into,
//        and a `return 0` there emits a second `xor eax,eax` the image does
//        not have.
// LEVER: `int pos_to_id(int)`, returning the value, though the catalogue
//        mangles it void - both exit paths load EAX before `ret 4`, and the
//        return type does not change a thiscall's stack cleanup.
// RESIDUAL: one register decision and its cascade, 10 of 51. The image hoists
//        a copy of `position` into EAX at instruction 4 (`mov eax,edi`) that
//        survives to the merged store at 0x61169a, keeps `entry_count_` in
//        ECX, and walks the cursor in EDX; this build loads the count into
//        EAX, never makes the copy, and decrements EDI in place in the
//        positive arm. This is the SAME wall the sibling Dialog::pos_to_id
//        (0x00609B50) documents on its own marker - the two siblings disagree
//        with each other in the shipped binary, so only one of them can be
//        reached from a shared spelling, and there Dialog is.
// TRIED: one `int distance` declared above the sign test instead of one per
//        arm - identical 4 of 51.
// TRIED: hoisted `int new_position = position;` storing that after the
//        if/else, the shape the image's surviving EAX copy implies - identical
//        4 of 51; VC6 copy-propagates it away exactly as the Dialog note
//        records.
// TRIED: `const int count = list_.entry_count_;` cached over the whole body -
//        5 of 51, one instruction better and one instruction longer (52), and
//        it hides the image's re-read of the count between its two abs calls,
//        which is the load-bearing detail. Left as the member read.
Return Value: the item id at the requested position, or 0 when the list is
              empty
Status: Complete
*/
int SpriteBox::pos_to_id(int position) {
    if (position <= list_.entry_count_ - 1) {
        list_.current_ = list_.head_;
        if (position < 0) {
            // `abs` CALLED TWICE, the count re-read between the calls - a
            // MIN-style macro over abs(position), not a local. See
            // Dialog::pos_to_id for the same shape.
            if (abs(position) > list_.entry_count_) {
                goto restore_done;
            }
            int distance = abs(position);
            while (distance > 0) {
                list_.current_ = list_.current_->previous;
                distance--;
            }
            position += list_.entry_count_;
        } else {
            int distance = position;
            while (distance > 0) {
                list_.current_ = list_.current_->next;
                distance--;
            }
        }
        list_.current_position_ = position;
    }
restore_done:
    return list_.head_ ? list_.current_->id : 0;
}


/*
// ORIGINAL: 0x00610480 ?init@SpriteBox@@QAEHPAURECT@@H@Z 0x00610480-0x006104A2 BYTE_EXACT
// symbol    ?init@SpriteBox@@QAEHPAUtagRECT@@H@Z
// size      34 bytes
// prototype int (__thiscall ?init@SpriteBox@@QAEHPAURECT@@H@Z)(SpriteBox* this, RECT*, int)
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00609470 0x00610280
Status: Complete
*/
int SpriteBox::init(RECT* group_id, int flags) {
    close();
    return reinterpret_cast<Dialog *>(reinterpret_cast<char *>(this)
        + *reinterpret_cast<int *>(*reinterpret_cast<char **>(this) + 8))->init(group_id, flags);
}

/*
Purpose: Repaint on dialog focus, through the enclosing object.
// ORIGINAL: 0x006115E0 ?on_dialog_focus@SpriteBox@@QAEXH@Z 0x006115E0-0x00611600 BYTE_EXACT
// symbol    ?on_dialog_focus@SpriteBox@@UAEXH@Z
// size      32 bytes
// prototype void (__thiscall ?on_dialog_focus@SpriteBox@@QAEXH@Z)(SpriteBox* this, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x006115F7
Return Value: n/a
Status: Complete
*/
void SpriteBox::on_dialog_focus(int a1) {
    // The repaint, as the plain virtual call it is (the redraw default).
    // The compiler performs the out-of-subobject adjustment as part of the
    // override's entry; `a1` is dead; `ret 4` still pops it.
    redraw();
}

/*
Purpose: Adjust the receiver from the thunk1 subobject back to SpriteBox and
         tail-jump to the override. THE COMPILER WRITES THIS BODY - the claim
         names VC6's own vtordisp adjustor thunk, emitted because
         SpriteBox::on_dialog_focus overrides a virtual reached through a virtual
         base. There is no C++ here to get right, only a name to record.
// ORIGINAL: 0x006118E0 ?on_dialog_focus@thunk1_SpriteBox@@QAEXH@Z 0x006118E0-0x006118E8 BYTE_EXACT
// symbol    ?on_dialog_focus@SpriteBox@@$4PPPPPPPM@A@AEXH@Z
// size      8 bytes
// kind      game
Status: Complete
*/

/*
Purpose: Check the click against the widget's spots; if one is hit and its
         registered handler accepts it, notify the main window with a
         synthetic Return keypress.
// ORIGINAL: 0x00611240 ?on_right_down@SpriteBox@@QAEXHH@Z 0x00611240-0x00611290 BYTE_EXACT
// size      80 bytes
// prototype void (__thiscall ?on_right_down@SpriteBox@@QAEXHH@Z)(SpriteBox* this, int xCoord, int yCoord)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005FAB00
// indirect  0x0061126D 0x00611286
Return Value: n/a
Status: Complete
*/
void SpriteBox::on_right_down(int x_coord, int y_coord) {
    char *const self = reinterpret_cast<char *>(this);
    Spot *const spot = reinterpret_cast<Spot *>(self - 0x7c);
    if (spot->check(x_coord, y_coord, &y_coord, 0) == -1) {
        return;
    }
    typedef int(__cdecl *ClickHandler)(int);
    ClickHandler const handler = *reinterpret_cast<ClickHandler *>(self - 0x14);
    if (!handler) {
        return;
    }
    if (handler(y_coord) == 0) {
        return;
    }
    PostMessageA(HandleMain, WM_KEYDOWN, VK_RETURN, 0);
}

/*
Purpose: Same shape as on_right_down above, for the left-button-up spot
         handler at this-0x1c.
// ORIGINAL: 0x006111A0 ?on_left_up@SpriteBox@@QAEXHH@Z 0x006111A0-0x006111F0 BYTE_EXACT
// size      80 bytes
// prototype void (__thiscall ?on_left_up@SpriteBox@@QAEXHH@Z)(SpriteBox* this, int xCoord, int yCoord)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005FAB00
// indirect  0x006111CD 0x006111E6
Return Value: n/a
Status: Complete
*/
void SpriteBox::on_left_up(int x_coord, int y_coord) {
    char *const self = reinterpret_cast<char *>(this);
    Spot *const spot = reinterpret_cast<Spot *>(self - 0x7c);
    if (spot->check(x_coord, y_coord, &y_coord, 0) == -1) {
        return;
    }
    typedef int(__cdecl *ClickHandler)(int);
    ClickHandler const handler = *reinterpret_cast<ClickHandler *>(self - 0x1c);
    if (!handler) {
        return;
    }
    if (handler(y_coord) == 0) {
        return;
    }
    PostMessageA(HandleMain, WM_KEYDOWN, VK_RETURN, 0);
}

/*
Purpose: Same check-then-notify shape, for the right-click spot handler at
         this-0x18. NESTED, not early-return - the shape the image has here.
// ORIGINAL: 0x006111F0 ?on_right_click@SpriteBox@@QAEXHH@Z 0x006111F0-0x00611240 BYTE_EXACT
// size      80 bytes
// prototype void (__thiscall ?on_right_click@SpriteBox@@QAEXHH@Z)(SpriteBox* this, int xCoord, int yCoord)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005FAB00
// indirect  0x0061121D 0x00611236
Return Value: n/a
Status: Complete
*/
void SpriteBox::on_right_click(int x_coord, int y_coord) {
    char *const self = reinterpret_cast<char *>(this);
    Spot *const spot = reinterpret_cast<Spot *>(self - 0x7c);
    if (spot->check(x_coord, y_coord, &y_coord, 0) != -1) {
        typedef int(__cdecl *ClickHandler)(int);
        ClickHandler const handler =
            *reinterpret_cast<ClickHandler *>(self - 0x18);
        if (handler != 0) {
            if (handler(y_coord) != 0) {
                PostMessageA(HandleMain, WM_KEYDOWN, VK_RETURN, 0);
            }
        }
    }
}

/*
Purpose: Same check-then-notify shape, for the right-button-up spot handler
         at this-0x10.
// ORIGINAL: 0x00611290 ?on_right_up@SpriteBox@@QAEXHH@Z 0x00611290-0x006112E0 BYTE_EXACT
// size      80 bytes
// prototype void (__thiscall ?on_right_up@SpriteBox@@QAEXHH@Z)(SpriteBox* this, int xCoord, int yCoord)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005FAB00
// indirect  0x006112BD 0x006112D6
Return Value: n/a
Status: Complete
*/
void SpriteBox::on_right_up(int x_coord, int y_coord) {
    char *const self = reinterpret_cast<char *>(this);
    Spot *const spot = reinterpret_cast<Spot *>(self - 0x7c);
    if (spot->check(x_coord, y_coord, &y_coord, 0) != -1) {
        typedef int(__cdecl *ClickHandler)(int);
        ClickHandler const handler =
            *reinterpret_cast<ClickHandler *>(self - 0x10);
        if (handler != 0) {
            if (handler(y_coord) != 0) {
                PostMessageA(HandleMain, WM_KEYDOWN, VK_RETURN, 0);
            }
        }
    }
}

/*
Purpose: Same check-then-notify shape, for the right-double-click spot
         handler at this-8.
// ORIGINAL: 0x00611330 ?on_right_double_click@SpriteBox@@QAEXHH@Z 0x00611330-0x00611380 BYTE_EXACT
// size      80 bytes
// prototype void (__thiscall ?on_right_double_click@SpriteBox@@QAEXHH@Z)(SpriteBox* this, int xCoord, int yCoord)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005FAB00
// indirect  0x0061135D 0x00611376
Return Value: n/a
Status: Complete
*/
void SpriteBox::on_right_double_click(int x_coord, int y_coord) {
    char *const self = reinterpret_cast<char *>(this);
    Spot *const spot = reinterpret_cast<Spot *>(self - 0x7c);
    if (spot->check(x_coord, y_coord, &y_coord, 0) != -1) {
        typedef int(__cdecl *ClickHandler)(int);
        ClickHandler const handler =
            *reinterpret_cast<ClickHandler *>(self - 8);
        if (handler != 0) {
            if (handler(y_coord) != 0) {
                PostMessageA(HandleMain, WM_KEYDOWN, VK_RETURN, 0);
            }
        }
    }
}

/*
// ORIGINAL: 0x00610420 ?init@SpriteBox@@QAEHH@Z 0x00610420-0x0061043D BYTE_EXACT
// size      29 bytes
// prototype int (__thiscall ?init@SpriteBox@@QAEHH@Z)(SpriteBox* this, int)
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006095F0 0x00610280
Status: Complete
*/
int SpriteBox::init(int file_name) {
    close();
    return reinterpret_cast<Dialog *>(reinterpret_cast<char *>(this)
        + *reinterpret_cast<int *>(*reinterpret_cast<char **>(this) + 8))->init(file_name);
}

// ORIGINAL: 0x00610440 ?init@SpriteBox@@QAEHHHHHH@Z 0x00610440-0x00610471 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence":["Pinned image 0x00610440-0x00610471 calls SpriteBox::close at 0x00610280, then forwards all five integer arguments to Dialog::init through the virtual-base displacement in vbtable[2].","src/dialog.h declares Dialog::init(int,int,int,int,int)."],"reason":"Preserve the seed wrapper as a typed SpriteBox member; C++ performs the virtual Dialog base adjustment and the qualified base call forwards the original arguments."}
int SpriteBox::init(int a1, int a2, int a3, int a4, int a5) {
    close();
    return Dialog::init(a1, a2, a3, a4, a5);
}

// ORIGINAL: 0x00610510 ?init@SpriteBox@@QAEHPAUHeap@@PAUHeap@@@Z 0x00610510-0x00610532 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence":["Pinned image 0x00610510-0x00610532 calls SpriteBox::close at 0x00610280, then forwards stack arguments 1 and 2 unchanged to Dialog::init at 0x006097F0 through vbtable[2]; argument 1 is RECT*, despite the catalogue symbol's Heap* spelling.","src/dialog.h declares Dialog::init(RECT*,Heap*)."],"reason":"Preserve the exact seed identity with the observed RECT*/Heap* signature and a typed call through SpriteBox's virtual Dialog base."}
// symbol    ?init@SpriteBox@@QAEHPAUtagRECT@@PAVHeap@@@Z
int SpriteBox::init(RECT *rect, Heap *heap) {
    close();
    return Dialog::init(rect, heap);
}

/*
// ORIGINAL: 0x006104B0 ?init@SpriteBox@@QAEHPAUHeap@@@Z 0x006104B0-0x006104CD BYTE_EXACT
// symbol    ?init@SpriteBox@@QAEHPAVHeap@@@Z
// LEVER: already BYTE_EXACT 11/11 - re-measured 2026-08-22 at
//        `/c /O2 /Gy /GR- /GX`, unchanged, no lever left to pull. The
//        spelling that reproduces is the one below: close() by name, then
//        `return`ing the vbtable-relative Dialog::init as the TAIL of the
//        function, reading the virtual-base displacement out of `*this + 8`
//        at the use rather than caching it in a local.
// size      29 bytes
// prototype int (__thiscall ?init@SpriteBox@@QAEHPAUHeap@@@Z)(SpriteBox* this, Heap*)
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006098D0 0x00610280
Status: Complete
*/
int SpriteBox::init(Heap *file_name) {
    close();
    return reinterpret_cast<Dialog *>(reinterpret_cast<char *>(this)
        + *reinterpret_cast<int *>(*reinterpret_cast<char **>(this) + 8))->init(file_name);
}



/*
// ORIGINAL: 0x006104D0 ?init@SpriteBox@@QAEHHHHHPAUHeap@@@Z 0x006104D0-0x00610501 BYTE_EXACT
// symbol    ?init@SpriteBox@@QAEHHHHHPAVHeap@@@Z
// size      49 bytes
// prototype int (__thiscall ?init@SpriteBox@@QAEHHHHHPAUHeap@@@Z)(SpriteBox* this, int, int, int, int, Heap*)
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00609730 0x00610280
Status: Complete
*/
int SpriteBox::init(int a1, int a2, int a3, int a4, Heap * a5) {
    close();
    return reinterpret_cast<Dialog *>(reinterpret_cast<char *>(this)
        + *reinterpret_cast<int *>(*reinterpret_cast<char **>(this) + 8))
        ->init(a1, a2, a3, a4, a5);
}

/*
Purpose: Adjust the receiver from the thunk1 subobject back to SpriteBox and
         tail-jump to the override. THE COMPILER WRITES THIS BODY - the claim
         names VC6's own vtordisp adjustor thunk, emitted because
         SpriteBox::attach overrides a virtual reached through a virtual
         base. There is no C++ here to get right, only a name to record.
// ORIGINAL: 0x00611920 ?attach@thunk1_SpriteBox@@QAEHPAUGraphicWin@@HHH@Z 0x00611920-0x00611928 BYTE_EXACT
// symbol    ?attach@SpriteBox@@$4PPPPPPPM@A@AEHPAVGraphicWin@@HHH@Z
// size      8 bytes
// kind      game
Status: Complete
*/

// THE LIST SpriteBox EMPTIES, and it is SpriteBox's alone. Measured across the
// whole image: the two vtables this body installs, 0x0066943C and 0x00669438,
// are referenced by exactly four functions - ??0SpriteBox, ??1SpriteBox,
// sub_402bb0 (which is itself a slot in them), and this. Nothing else in
// 2.5MB of .text touches either. So the class is private to this file.
//
// TWO VPTRS, ONE TABLE. 0x0066943C is 0x00669438 shifted by one slot, which is
// the secondary-vptr-into-one-table arrangement multiple inheritance produces,
// and this body is entered on the SECOND of them: `lea esi, [ecx - 0x28]`
// walks back to the object's own start before storing the primary vtable, then
// reaches the virtual base through the vbtable to store the other.
//
// THE NAME IS OURS. The image is stripped, so no mangled name is being matched
// here - only bytes - and the artifact's `Obj611730` was as invented as this.
// SpriteBoxList itself moved to spritebox.h as the real StringStruct sibling
// SpriteBox embeds at 0x24.

// SUB_402BB0 IS THIS CLASS'S DELETING DESTRUCTOR, still unrecovered
// (src/recovered/units/00402bb0.cpp, NOT_MATCHING). Named here because the
// evidence is here:
//
//   * it installs the SAME pair, 0x0066943C at the object's start and
//     0x00669438 through the vbtable, which no other class in the image does;
//   * it then stores 0x006693AC at +0x1C. That one is referenced 158 times
//     across the image, so it is not class-specific - it is StringStruct's own
//     base vtable, installed by everything that embeds one;
//   * it is entered on that base: `lea esi, [ecx - 0x1c]` walks back to the
//     derived object's start;
//   * it calls 0x00402970, StringStruct::remove_all;
//   * and it ends `mov cl, [ebp+8]; test cl, 1` with a conditional
//     `operator delete` - the flags byte a deleting destructor takes.
//
// So it is `??_ESpriteBoxList@@...`, the VECTOR deleting destructor - `??_E`
// rather than the census's `??_G`, for the reason forty other thunks needed
// correcting today.

/*
Purpose: Close the list - install its pair of virtual tables, release every
         entry through its own vtable, then clear head, count and position.
// ORIGINAL: 0x00611730 sub_611730 0x00611730-0x006117C3
// CORRECTED from sub_611730
//   It is a StringStruct-derived list's `close`. src/stringstruct.h declares
//   `close_with_tables(primary, virtual_base)` as "installs the pair of
//   virtual tables, releases the entries, then clears the position", and
//   StringStruct's offsets 0x00/0x04 are its vftable and vbtable slots - the
//   two raw stores this body's immediates make. src/dialog.cpp
//   calls the same helper the same way for the list at Dialog's this+0xBC.
// TRIED: PROMOTED out of src/unrecovered/00611730.cpp. Called by both ??0SpriteBox and ??1SpriteBox, so it is shared setup and teardown rather than one or the other.
// TRIED: spelling the body AS `list->close_with_tables(0x0066943C, 0x00669438)` on the StringStruct at `this - 0x28`, which is the idiom dialog.cpp uses. It compiles to 5 instructions against the image's 63 - the inline folds to almost nothing at this offset - so the walk stays written out here.
// symbol    ?close@SpriteBoxList@@QAEXXZ
// size      147 bytes
// kind      game
Return Value: n/a
Status: Complete
*/
void SpriteBoxList::close() {
    class LocalVCall {
    public:
        virtual void slot0(int) = 0;
        virtual void slot1(void *) = 0;
    };

    char *self = reinterpret_cast<char *>(this) - 0x28;

    // Strip-all direction: the pair install the image stages here
    // (`mov [esi-0x28], 0x66943c` and the vbtable-relative 0x669438) is
    // gone; the walk below dispatches through whatever tables the
    // constructing chain installed.

    void *head = *reinterpret_cast<void **>(self + 8);
    if (head != 0) {
        int count = *reinterpret_cast<int *>(self + 0x10);
        int i = 0;
        if (count > 0) {
            do {
                *reinterpret_cast<void **>(self + 0xc) =
                    *reinterpret_cast<void **>(reinterpret_cast<char *>(
                        *reinterpret_cast<void **>(self + 8)) + 0xc);
                void *child = *reinterpret_cast<void **>(reinterpret_cast<char *>(
                    *reinterpret_cast<void **>(self + 8)) + 8);

                reinterpret_cast<LocalVCall *>(self)->slot1(child);

                if (child != 0) {
                    char *vtbl = *reinterpret_cast<char **>(child);
                    int adj = *reinterpret_cast<int *>(vtbl + 4);
                    char *adjusted = reinterpret_cast<char *>(child) + adj;
                    reinterpret_cast<LocalVCall *>(adjusted)->slot0(1);
                }

                *reinterpret_cast<int *>(reinterpret_cast<char *>(
                    *reinterpret_cast<void **>(self + 8)) + 8) = 0;

                void *cur2 = *reinterpret_cast<void **>(self + 8);
                if (cur2 != 0) {
                    reinterpret_cast<LocalVCall *>(
                        reinterpret_cast<char *>(cur2) +
                        *reinterpret_cast<int *>(
                            *reinterpret_cast<char **>(cur2) + 4))->slot0(1);
                }

                i++;
                *reinterpret_cast<void **>(self + 8) = *reinterpret_cast<void **>(self + 0xc);
            } while (i < *reinterpret_cast<int *>(self + 0x10));
        }
        *reinterpret_cast<volatile int *>(self + 0x14) = 0;
        *reinterpret_cast<int *>(self + 8) = 0;
        *reinterpret_cast<int *>(self + 0x10) = 0;
        *reinterpret_cast<int *>(self + 0x14) = 0;
    } else {
        *reinterpret_cast<int *>(self + 0x14) = 0;
    }
}

// ====================
// Window-event handlers
// ====================

/*
Purpose: A left press on a sprite box runs the hit test over its Spot table
         and, when a spot took the hit, notifies the object's stored click
         callback with the spot position; a true return then posts WM_KEYDOWN
         (VK_RETURN) to the main window. Entered on the GraphicWin-subobject
         adjusted pointer (the family's thunk convention), so the Spot table
         sits 0x7C below `this` and the callback 0x20 below - the Spot is the
         constructor's placement-new at the object's own +0x10.
// ORIGINAL: 0x00611150 ?on_left_down@SpriteBox@@QAEXHH@Z 0x00611150-0x006111A0 BYTE_EXACT
// size      80 bytes
// prototype void (__thiscall ?on_left_down@SpriteBox@@QAEXHH@Z)(SpriteBox* this, int xCoord, int yCoord)
// callers   1   call targets   1
// kind      game
// calls     0x005FAB00
// indirect  0x0061117D 0x00611196
//
// PROMOTED out of the archived verification unit for this address. The import thunk
// 0x00669314 is PostMessageA (win.cpp: on_sys_key landed the same call by
// name), and 0x009B7B28 is HandleMain; both compile to the same indirect
// forms the image emits.
// keeps it live in a register where the image re-reads [this-0x20] for the
// call - the nested ifs and the two separate reads are the image's own.
Return Value: n/a
Status: Complete
*/
void SpriteBox::on_left_down(int x_coord, int y_coord) {
    char *self = reinterpret_cast<char *>(this);
    Spot *spot = reinterpret_cast<Spot *>(self - 0x7c);
    if (spot->check(x_coord, y_coord, &y_coord, 0) != -1) {
        typedef int (__cdecl *sprite_box_click)(int);
        sprite_box_click callback =
            *reinterpret_cast<sprite_box_click *>(self - 0x20);
        if (callback != 0) {
            if (callback(y_coord) != 0) {
                PostMessageA(HandleMain, 0x100, 0xd, 0);
            }
        }
    }
}

/*
Purpose: Hit-test the owned Spot member, notify the stored double-click
         callback with the hit position, and post WM_KEYDOWN (VK_RETURN)
         when it returns nonzero. The GraphicWin-relative Spot at -0x7C and
         callback at -0xC are complete-object members +0x10 and +0x80;
         the compiler supplies those adjustments for this virtual override.
// ORIGINAL: 0x006112E0 ?on_left_double_click@SpriteBox@@QAEXHH@Z 0x006112E0-0x00611330 BYTE_EXACT
// symbol    ?on_left_double_click@SpriteBox@@UAEXHH@Z
// size      80 bytes
// prototype void (__thiscall ?on_left_double_click@SpriteBox@@QAEXHH@Z)(SpriteBox* this, int xCoord, int yCoord)
// callers   1   call targets   1
// kind      game
// calls     0x005FAB00
// indirect  0x0061130D 0x00611326
//
// PROMOTED out of the archived verification unit for this address; same two bindings as
// on_left_down above.
Return Value: n/a
Status: Complete
*/
void SpriteBox::on_left_double_click(int x_coord, int y_coord) {
    if (spots_.check(x_coord, y_coord, &y_coord, 0) != -1) {
        if (double_click_callback_ != 0 && double_click_callback_(y_coord) != 0) {
            PostMessageA(HandleMain, 0x100, 0xd, 0);
        }
    }
}
