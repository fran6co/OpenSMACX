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
#include "editgroup.h"
#include "editbox.h"

func_edit_box_set_text EditBoxOriginalSetText =
    original_method<func_edit_box_set_text>(0x006151E0);

// EditGroupDefault - 0x009B8FCC. Read at construction time into field_84_;
// a rebindable seam so tests can observe/control it, the same idiom
// CheckBoxDefault1/2 use.
uint32_t EditGroupDefault;

/*
Purpose: Compose an EditGroup from its two base-shaped subobjects
         (GraphicWin, Dialog), install this class's own vtable/vtordisp
         values into the object's vbtable-named slots, clear the box/limit
         tables to their defaults, and read one more default field.

         `a1` is the compiler's most-derived flag in the image
         (??0EditGroup@@QAE@H@Z): nonzero means construct the two bases,
         zero means skip straight to installing this class's own tables -
         the same "am I the most-derived object" guard ListBox's constructor
         uses. This class cannot let VC6 synthesise that flag (see the class
         comment in editgroup.h), so it is modelled as an explicit parameter
         instead and the guard is written out by hand.
// ORIGINAL: 0x00611940 ??0EditGroup@@QAE@H@Z 0x00611940-0x00611A15;0x00663020-0x00663047
// size      252 bytes
// prototype void (__thiscall ??0EditGroup@@QAE@H@Z)(EditGroup* this, int)
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4CF0 0x00608C10
// TRIED: an earlier scratch artifact (src/unrecovered/00611940.cpp) used
//            the same placement-new-with-try/catch shape and reported the
//            image never establishes an ebp frame ahead of `push -1; push
//            0x66303d` - pure esp-relative addressing - while every flag set
//            tried gives either an ebp-framed prolog or an EH-less one.
//            MEASURED here too: matches that finding. ROOT CAUSE (narrowed
//            after measuring CheckBox/RadioButton's identical shape):
//            `dialog_.construct()` is an ORDINARY METHOD CALL, not a
//            placement-new constructor call, so VC6 has nothing
//            partially-constructed to protect and emits no frame at all.
//            A real frame needs Dialog's own constructor AND GraphicWin's
//            own destructor to both be non-trivial - a coupled, tree-wide
//            edit flagged as OPEN elsewhere, not attempted here.
Return Value: n/a
Status: Complete
*/
EditGroup::EditGroup(int a1) {
    char *const self = reinterpret_cast<char *>(this);

    // THE COMPILER OWNS THE VIRTUAL-BASE SETUP - see the class declaration.
    // Only the two `construct()` calls remain, because those bases model
    // construction as a method rather than a constructor.
    if (a1 != 0) {
        GraphicWin::construct();
        Dialog::construct();
    }

    {
        const int32_t *const vbtable = *reinterpret_cast<const int32_t *const *>(self);
        const int32_t off1 = vbtable[1];
        *reinterpret_cast<int32_t *>(self + off1) = 0x006708D4;
    }
    {
        const int32_t *const vbtable = *reinterpret_cast<const int32_t *const *>(self);
        const int32_t off1 = vbtable[1];
        *reinterpret_cast<int32_t *>(self + off1 + 0x444) = 0x006708CC;
    }
    {
        const int32_t *const vbtable = *reinterpret_cast<const int32_t *const *>(self);
        const int32_t off2 = vbtable[2];
        *reinterpret_cast<int32_t *>(self + off2) = 0x006708C0;
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

    for (int i = 0; i < 10; ++i) {
        boxes_[i] = nullptr;
        boxes_[10 + i] = nullptr;
        limits_[i] = 10;
    }

    field_84_ = EditGroupDefault;
}

/*
Purpose: Tear down an EditGroup: reinstall the base subobjects' own
         vtable/vtordisp values, then close the object through its own
         `close()`.
// ORIGINAL: 0x00611A20 ??1EditGroup@@QAE@XZ 0x00611A20-0x00611A88 BYTE_EXACT
// LEVER: TWO changes, measured separately. (1) A real `~EditGroup()` has to destroy `virtual_base_` and `dialog_`, which under /GX is an SEH frame plus a thirty-instruction member-teardown tail - 67 compiled instructions against the image's 23, agreeing 0. The image reinstalls the base vtables and calls `close()` and destroys nothing, so this is a METHOD; spelling it `destruct()` drops the frame and the tail and lands exactly 23. (2) With that done the only remaining difference was register allocation, and a NAMED `base` local was causing it: `char *const base = self - 0x8C;` gets EAX and costs a `mov ecx, eax` before the call, 3 of 23. Writing `self - 0x8C` at each use lets VC6 common it into ECX - where the thiscall receiver has to be anyway - and spill `this` to EAX, which is the image's opening `mov eax, ecx`. 23 of 23, BYTE_EXACT.
// TRIED: naming the local and calling through `self - 0x8C` anyway (3/23), a typed `EditGroup *const base_object` (3/23), an explicitly qualified `->EditGroup::close()` (3/23), and deriving `self` from `base` rather than the other way round, which loses four instructions outright (0 of 23, 19 compiled).
// TRIED: `record` DROPPED the line above the first time it was written. Re-recording a body whose tier changes rewrites its annotation and keeps only the LEVER. Re-added after the claim was banked.
// symbol    ?destruct@EditGroup@@QAEXXZ
// size      104 bytes
// prototype void (__thiscall ??1EditGroup@@QAE@XZ)(EditGroup* this)
// callers   20   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00611A90
// notes     Entered on the ADJUSTED (GraphicWin-subobject) `this`: the
//           disassembly reaches every field through `this - 0x8C`, the
//           same vtordisp adjustor the constructor's own vtordisp slot
//           names, and the closing call goes out on that adjusted (true
//           object) pointer, not on `this` itself.
Return Value: n/a
Status: Complete
*/
void EditGroup::destruct() {
    char *const self = reinterpret_cast<char *>(this);

    {
        const int32_t *const vbtable = *reinterpret_cast<const int32_t *const *>(self - 0x8C);
        const int32_t off1 = vbtable[1];
        *reinterpret_cast<int32_t *>(self - 0x8C + off1) = 0x006708D4;
    }
    {
        const int32_t *const vbtable = *reinterpret_cast<const int32_t *const *>(self - 0x8C);
        const int32_t off1 = vbtable[1];
        *reinterpret_cast<int32_t *>(self + off1 + 0x3B8) = 0x006708CC;
    }
    {
        const int32_t *const vbtable = *reinterpret_cast<const int32_t *const *>(self - 0x8C);
        const int32_t off2 = vbtable[2];
        *reinterpret_cast<int32_t *>(self - 0x8C + off2) = 0x006708C0;
    }
    {
        const int32_t *const vbtable = *reinterpret_cast<const int32_t *const *>(self - 0x8C);
        const int32_t off1 = vbtable[1];
        *reinterpret_cast<int32_t *>(self + off1 - 0x90) = off1 - 0x8C;
    }
    {
        const int32_t *const vbtable = *reinterpret_cast<const int32_t *const *>(self - 0x8C);
        const int32_t off2 = vbtable[2];
        *reinterpret_cast<int32_t *>(self + off2 - 0x90) = off2 - 0xAA4;
    }

    // NO `base` LOCAL. Naming `self - 0x8C` pins it to EAX and forces a
    // `mov ecx, eax` before the call; letting VC6 common the subexpression
    // itself puts it in ECX, where the receiver already has to be, and moves
    // `this` to EAX - the image's own `mov eax, ecx` opening.
    reinterpret_cast<EditGroup *>(self - 0x8C)->close();
}

/*
Purpose: Apply one length limit to all ten edit boxes in the group.
// ORIGINAL: 0x006120B0 ?set_text_limits@EditGroup@@QAEXH@Z 0x006120B0-0x006120C3 BYTE_EXACT
// size      19 bytes
// prototype void (__thiscall ?set_text_limits@EditGroup@@QAEXH@Z)(EditGroup* this, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void EditGroup::set_text_limits(int limit) {
    for (size_t index = 0; index < ARRAYSIZE(limits_); ++index) {
        limits_[index] = static_cast<uint32_t>(limit);
    }
}

/*
Purpose: Return one box's text buffer, or nothing when that box is absent.
// ORIGINAL: 0x00612060 ?get_text@EditGroup@@QAEPADH@Z 0x00612060-0x00612079 BYTE_EXACT
// LEVER: branch polarity. The guard-clause form `if (!box) return nullptr;` lets VC6 prove eax is already zero and MERGE both returns onto one `ret 4` (5 of 8, `je` to the shared ret). The image keeps a separate `xor eax, eax; ret 4` tail, which is what `if (box) return ...; return nullptr;` produces - 8 of 8, BYTE_EXACT. A ternary and `return 0` score the same 8 of 8; re-reading `boxes_[index]` instead of caching it stays at 5 of 8.
// size      25 bytes
// prototype int8* (__thiscall ?get_text@EditGroup@@QAEPADH@Z)(EditGroup* this, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: pointer to the box's text, null when there is no box
Status: Complete
*/
char *EditGroup::get_text(int index) {
    void *const box = boxes_[index];
    if (box) {
        return reinterpret_cast<char *>(box) + 0xA14;
    }
    return nullptr;
}

/*
Purpose: Set one box's text, doing nothing when that box is absent.
// ORIGINAL: 0x00612040 ?set_text@EditGroup@@QAEXPADH@Z 0x00612040-0x00612059 BYTE_EXACT
// size      25 bytes
// prototype void (__thiscall ?set_text@EditGroup@@QAEXPADH@Z)(EditGroup* this, int8*, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006151E0
Return Value: n/a
Status: Complete
*/
void EditGroup::set_text(char *text, int index) {
    void *const box = boxes_[index];
    if (box) {
        // A NAMED METHOD, not the seam: the image emits `call rel32` here.
        reinterpret_cast<EditBox *>(box)->EditBox::set_text(text);
    }
}

void __fastcall edit_group_set_text_limits_redirect(EditGroup *self, void *, int limit) {
    self->set_text_limits(limit);
}

char *__fastcall edit_group_get_text_redirect(EditGroup *self, void *, int index) {
    return self->get_text(index);
}

void __fastcall edit_group_set_text_redirect(EditGroup *self, void *, char *text, int index) {
    self->set_text(text, index);
}

/*
Purpose: Adjust the receiver from the thunk1 subobject back to EditGroup and
         tail-jump to the override. THE COMPILER WRITES THIS BODY - the claim
         names VC6's own vtordisp adjustor thunk, emitted because
         EditGroup::attach overrides a virtual reached through a virtual
         base. There is no C++ here to get right, only a name to record.
// ORIGINAL: 0x00612810 ?attach@thunk1_EditGroup@@QAEHPAUGraphicWin@@HHH@Z 0x00612810-0x00612818 BYTE_EXACT
// symbol    ?attach@EditGroup@@$4PPPPPPPM@A@AEHPAXHHH@Z
// size      8 bytes
// kind      game
Status: Complete
*/

/*
Purpose: Adjust the receiver from the thunk1 subobject back to EditGroup and
         tail-jump to the override. THE COMPILER WRITES THIS BODY - the claim
         names VC6's own vtordisp adjustor thunk, emitted because
         EditGroup::on_dialog_focus overrides a virtual reached through a virtual
         base. There is no C++ here to get right, only a name to record.
// ORIGINAL: 0x006127C0 ?on_dialog_focus@thunk1_EditGroup@@QAEXH@Z 0x006127C0-0x006127C8 BYTE_EXACT
// symbol    ?on_dialog_focus@EditGroup@@$4PPPPPPPM@A@AEXH@Z
// size      8 bytes
// kind      game
Status: Complete
*/
