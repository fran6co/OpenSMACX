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

// SpriteBox's own vbtable {0, 0x8C, 0xAA4} and the vtables it installs into
// the two virtual-base slots. Same idiom as CheckBoxDefault1/2 - a plain
// literal, since these are byte-visible immediates in the image, not
// runtime-read globals.
static const uint32_t SpriteBoxVbtable = 0x006708B4;

/*
Purpose: Compose a SpriteBox from its GraphicWin and Dialog virtual-base-
         shaped subobjects (both constructed only when most-derived) and an
         unconditional Spot, install this class's own vtable/vtordisp
         values and a secondary "field_28_" table's own slot, clear the
         intrusive-list fields, and set the buffer's default text colours
         and font.

         `a1` is the compiler's most-derived flag in the image
         (??0SpriteBox@@QAE@H@Z): nonzero means construct the two bases,
         zero means skip both - the same guard CheckBox/EditGroup/
         RadioButton use. This class cannot let VC6 synthesise that flag
         (see the class comment in spritebox.h), so it is modelled as an
         explicit parameter instead.
// ORIGINAL: 0x0060FF00 ??0SpriteBox@@QAE@H@Z 0x0060FF00-0x0061011B;0x006116C0-0x006116DB;0x00662FA0-0x00662FFA
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
SpriteBox::SpriteBox(int a1) {
    char *const self = reinterpret_cast<char *>(this);

    if (a1 != 0) {
        new (static_cast<GraphicWin *>(this)) GraphicWin();
        Dialog::construct();
    }

    new (self + 0x10) Spot();

    field_28_ = 0x006708AC;
    field_4C_ = 0x006693AC;
    field_50_ = *reinterpret_cast<int *>(0x009B3374);
    *reinterpret_cast<int *>(0x009B3374) = 0;

    field_24_ = 0x0066943C;
    {
        const int32_t *const table = reinterpret_cast<const int32_t *>(field_28_);
        const int32_t voff = table[1];
        *reinterpret_cast<int32_t *>(self + 0x28 + voff) = 0x00669438;
    }

    field_2C_ = 0;
    field_30_ = 0;
    field_34_ = 0;
    field_38_ = 0;
    field_3C_ = 0;

    field_24_ = 0x006708A4;
    {
        const int32_t *const table = reinterpret_cast<const int32_t *>(field_28_);
        const int32_t voff = table[1];
        *reinterpret_cast<int32_t *>(self + 0x28 + voff) = 0x006708A0;
    }

    {
        const int32_t *const vbtable = *reinterpret_cast<const int32_t *const *>(self);
        const int32_t off1 = vbtable[1];
        *reinterpret_cast<int32_t *>(self + off1) = 0x00670738;
    }
    {
        const int32_t *const vbtable = *reinterpret_cast<const int32_t *const *>(self);
        const int32_t off1 = vbtable[1];
        *reinterpret_cast<int32_t *>(self + off1 + 0x444) = 0x00670730;
    }
    {
        const int32_t *const vbtable = *reinterpret_cast<const int32_t *const *>(self);
        const int32_t off2 = vbtable[2];
        *reinterpret_cast<int32_t *>(self + off2) = 0x00670724;
    }
    {
        const int32_t *const vbtable = *reinterpret_cast<const int32_t *const *>(self);
        const int32_t off1 = vbtable[1];
        *reinterpret_cast<int32_t *>(self + off1 - 4) = off1 - 0x8C;
    }
    {
        const int32_t *const vbtable = *reinterpret_cast<const int32_t *const *>(self);
        const int32_t off2 = vbtable[2];
        *reinterpret_cast<int32_t *>(self + off2 - 4) = off2 - 0xAA4;
    }

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
    field_80_ = 0;
    field_84_ = 0;

    field_8_ = *reinterpret_cast<int *>(0x009B8FC4);

    {
        const int32_t *const vbtable = *reinterpret_cast<const int32_t *const *>(self);
        const int32_t off1 = vbtable[1];
        Buffer *const buf = reinterpret_cast<Buffer *>(self + off1 + 0x444);
        buf->set_text_color(*reinterpret_cast<int *>(0x006970AC),
                             *reinterpret_cast<int *>(0x006970B8),
                             *reinterpret_cast<int *>(0x006970C4),
                             *reinterpret_cast<int *>(0x006970D0));
    }
    {
        const int32_t *const vbtable = *reinterpret_cast<const int32_t *const *>(self);
        const int32_t off1 = vbtable[1];
        Buffer *const buf = reinterpret_cast<Buffer *>(self + off1 + 0x444);
        buf->set_text_color2(*reinterpret_cast<int *>(0x006970B0),
                              *reinterpret_cast<int *>(0x006970BC),
                              *reinterpret_cast<int *>(0x006970C8),
                              *reinterpret_cast<int *>(0x006970D4));
    }
    {
        const int32_t *const vbtable = *reinterpret_cast<const int32_t *const *>(self);
        const int32_t off1 = vbtable[1];
        Buffer *const buf = reinterpret_cast<Buffer *>(self + off1 + 0x444);
        buf->set_text_color3(*reinterpret_cast<int *>(0x006970B4),
                              *reinterpret_cast<int *>(0x006970C0),
                              *reinterpret_cast<int *>(0x006970CC),
                              *reinterpret_cast<int *>(0x006970D8));
    }
    {
        const int32_t *const vbtable = *reinterpret_cast<const int32_t *const *>(self);
        const int32_t off1 = vbtable[1];
        Buffer *const buf = reinterpret_cast<Buffer *>(self + off1 + 0x444);
        buf->set_font(*reinterpret_cast<Font **>(0x009B8EC0),
                      *reinterpret_cast<Font **>(0x009B8EC4),
                      *reinterpret_cast<Font **>(0x009B8EC8),
                      0);
    }
}

/*
Purpose: Tear down a SpriteBox: reinstall the base subobjects' own
         vtable/vtordisp values, close it, then destroy the Spot member.
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
    char *const self = reinterpret_cast<char *>(this) - 0x8C;

    {
        const int32_t *const vbtable = *reinterpret_cast<const int32_t *const *>(self);
        *reinterpret_cast<int32_t *>(self + vbtable[1]) = 0x00670738;
    }
    {
        const int32_t *const vbtable = *reinterpret_cast<const int32_t *const *>(self);
        *reinterpret_cast<int32_t *>(self + vbtable[1] + 0x444) = 0x00670730;
    }
    {
        const int32_t *const vbtable = *reinterpret_cast<const int32_t *const *>(self);
        *reinterpret_cast<int32_t *>(self + vbtable[2]) = 0x00670724;
    }
    {
        const int32_t *const vbtable = *reinterpret_cast<const int32_t *const *>(self);
        const int32_t off1 = vbtable[1];
        *reinterpret_cast<int32_t *>(self + off1 - 4) = off1 - 0x8C;
    }
    {
        const int32_t *const vbtable = *reinterpret_cast<const int32_t *const *>(self);
        const int32_t off2 = vbtable[2];
        *reinterpret_cast<int32_t *>(self + off2 - 4) = off2 - 0xAA4;
    }

    reinterpret_cast<SpriteBox *>(self)->close();

    reinterpret_cast<Spot *>(self + 0x10)->~Spot();
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

void __fastcall sprite_box_on_mouse_move_redirect(SpriteBox *self, void *, int a1, int a2) {
    self->on_mouse_move(a1, a2);
}

void __fastcall sprite_box_on_mouse_leave_redirect(SpriteBox *self, void *, int a1, int a2) {
    self->on_mouse_leave(a1, a2);
}

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

uint32_t __fastcall sprite_box_id_to_pos_redirect(SpriteBox *self, void *, int id) {
    return self->id_to_pos(id);
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
int SpriteBox::init(RECT* a1, int a2) {
    close();
    return reinterpret_cast<Dialog *>(reinterpret_cast<char *>(this)
        + *reinterpret_cast<int *>(*reinterpret_cast<char **>(this) + 8))->init(a1, a2);
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
    // THE COMPILER WALKS OUT OF THE SUBOBJECT NOW. This body used to write
    // `this - 0x8c` by hand; with GraphicWin a real virtual base and this an
    // override of its `on_dialog_focus`, VC6 performs that adjustment as part
    // of the entry, and subtracting it again in source double-counts it
    // (`-0x11c` where the image has `-0x8c`). One expression still: naming an
    // intermediate changes the register choice. `a1` is dead; `ret 4` still
    // pops it.
    reinterpret_cast<VCall *>(
        reinterpret_cast<char *>(this)
        + *reinterpret_cast<int *>(
            *reinterpret_cast<int *>(
                reinterpret_cast<char *>(this)) + 4))->slot062();
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
int SpriteBox::init(int a1) {
    close();
    return reinterpret_cast<Dialog *>(reinterpret_cast<char *>(this)
        + *reinterpret_cast<int *>(*reinterpret_cast<char **>(this) + 8))->init(a1);
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
int SpriteBox::init(Heap *a1) {
    close();
    return reinterpret_cast<Dialog *>(reinterpret_cast<char *>(this)
        + *reinterpret_cast<int *>(*reinterpret_cast<char **>(this) + 8))->init(a1);
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
// symbol    ?attach@SpriteBox@@$4PPPPPPPM@A@AEHPAXHHH@Z
// size      8 bytes
// kind      game
Status: Complete
*/
