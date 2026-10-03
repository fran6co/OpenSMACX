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
#include "general.h"  // mem_get, for the two coverage bodies below
#include "temp.h" // HandleMain

namespace {
struct WinNodeListFocusEntry {
    uint32_t reserved_prefix;
    Win *window;
};
}

// EditGroupDefault - 0x009B8FCC. Read at construction time into field_84_;
// a rebindable seam so tests can observe/control it, the same idiom
// CheckBoxDefault1/2 use.
uint32_t EditGroupDefault;

/*
Purpose: Construct the shared GraphicWin and Dialog virtual bases, initialize
         the separate control/text/limit arrays and read the process default.
         The compiler owns the hidden most-derived flag, vbtables and vtordisp;
         catalogue H is not a source parameter. Historical experiments below
         predate this genuine hierarchy and do not establish current matching.
// ORIGINAL: 0x00611940 ??0EditGroup@@QAE@H@Z 0x00611940-0x00611A15;0x00663020-0x00663047 SEMANTIC
// symbol    ??0EditGroup@@QAE@XZ
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
EditGroup::EditGroup() {
    // THE COMPILER OWNS THE VIRTUAL-BASE SETUP, end to end - see checkbox.cpp.
    // Strip-all direction: the image's five derived-stage stores remain
    // unreproduced; the constructor is unclaimed.

    for (int i = 0; i < 10; ++i) {
        boxes_[i] = nullptr;
        text_buffers_[i] = nullptr;
        limits_[i] = 10;
    }

    field_84_ = EditGroupDefault;
}

/*
Purpose: Tear down an EditGroup: reinstall the base subobjects' own
         vtable/vtordisp values, then close the object through its own
         `close()`.
// ORIGINAL: 0x00611A20 ??1EditGroup@@QAE@XZ 0x00611A20-0x00611A88
// TRIED: TWO changes, measured separately. (1) A real `~EditGroup()` has to destroy `virtual_base_` and `dialog_`, which under /GX is an SEH frame plus a thirty-instruction member-teardown tail - 67 compiled instructions against the image's 23, agreeing 0. The image reinstalls the base vtables and calls `close()` and destroys nothing, so this is a METHOD; spelling it `destruct()` drops the frame and the tail and lands exactly 23. (2) With that done the only remaining difference was register allocation, and a NAMED `base` local was causing it: `char *const base = self - 0x8C;` gets EAX and costs a `mov ecx, eax` before the call, 3 of 23. Writing `self - 0x8C` at each use lets VC6 common it into ECX - where the thiscall receiver has to be anyway - and spill `this` to EAX, which is the image's opening `mov eax, ecx`. 23 of 23, BYTE_EXACT.
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
    // Strip-all direction: the image's five derived-stage re-stores here -
    // the whole content this BYTE_EXACT claim used to bank - are gone, as in
    // CheckBox and RadioButton. What remains is the close() dispatch the
    // image ends with.
    char *const self = reinterpret_cast<char *>(this);

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
    EditBox *const box = boxes_[index];
    if (box) {
        return box->text_;
    }
    return nullptr;
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00612080 ?append_text@EditGroup@@QAEXPADH@Z 0x00612080-0x006120AA BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00612080.cpp retains the exact seed", "src/editgroup.h boxes_[10] are EditBox owners at offset 0x4", "src/editbox.h EditBox::text_ starts at offset 0xA14"], "reason": "Copy the selected editor's text through its real EditBox member instead of indexing receiver bytes."}
// symbol    ?append_text@EditGroup@@QAEXPADH@Z
void EditGroup::append_text(char *text, int index) {
    EditBox *const box = boxes_[index];
    if (box != 0 && text != 0) {
        text[0] = 0;
        strcat(text, box->text_);
    }
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
    EditBox *const box = boxes_[index];
    if (box) {
        // A NAMED METHOD, not the seam: the image emits `call rel32` here.
        box->EditBox::set_text(text);
    }
}




/*
Purpose: Adjust the receiver from the thunk1 subobject back to EditGroup and
         tail-jump to the override. THE COMPILER WRITES THIS BODY - the claim
         names VC6's own vtordisp adjustor thunk, emitted because
         EditGroup::attach overrides a virtual reached through a virtual
         base. There is no C++ here to get right, only a name to record.
// ORIGINAL: 0x00612810 ?attach@thunk1_EditGroup@@QAEHPAUGraphicWin@@HHH@Z 0x00612810-0x00612818 BYTE_EXACT
// symbol    ?attach@EditGroup@@$4PPPPPPPM@A@AEHPAVGraphicWin@@HHH@Z
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

/*
Purpose: Adjust the receiver from the Win/GraphicWin-path thunk1 subobject
         back to EditGroup and tail-jump to the override. THE COMPILER WRITES
         THIS BODY - the claim names VC6's own vtordisp adjustor thunk, emitted
         because the single EditGroup::on_redraw declaration overrides Win
         slot12 through the virtual GraphicWin base as well as Dialog slot2.
         There is no C++ here to get right, only a name to record.
// ORIGINAL: 0x006127B0 ?on_redraw@thunk1_EditGroup@@QAEXXZ 0x006127B0-0x006127B8 BYTE_EXACT
// symbol    ?on_redraw@EditGroup@@$4PPPPPPPM@A@AEXXZ
// size      8 bytes
// kind      game
// TRIED 2026-09-26 (EditGroup adjustor retirement batch): a handwritten walk
//            carried this claim until the rename; it walked
//            `object - vtordisp - 0xAA8` (1/3, stale pre-hierarchy Dialog
//            offset), then the repaired `- 0xAA4` fold cancellation, and after
//            the tree-wide Win slot-12 on_redraw rename `- 0x8C` (the
//            GraphicWin fold), all measured BYTE_EXACT from
//            src/adjustor_thunks.cpp. The generated thunk measures the same
//            span (`2b 49 fc e9 <rel32>`); the marker moved here when the
//            handwritten body was retired.
Status: Complete
*/

/*
Purpose: Adjust the receiver from the Dialog-path thunk3 subobject back to
         EditGroup and tail-jump to the override. THE COMPILER WRITES THIS
         BODY - the claim names VC6's own two-step vtordisp adjustor thunk,
         emitted because the single EditGroup::on_redraw declaration overrides
         Dialog slot2 (table6708C0 at complete+0xAA4) as well as Win slot12;
         only a function overriding both virtual bases carries the static
         0xA18 Dialog-to-GraphicWin delta.
         There is no C++ here to get right, only a name to record.
// ORIGINAL: 0x00612820 ?on_redraw@thunk3_EditGroup@@QAEXXZ 0x00612820-0x0061282E BYTE_EXACT
// symbol    ?on_redraw@EditGroup@@$4PPPPPPPM@KBI@AEXXZ
// size      14 bytes
// kind      game
// TRIED 2026-09-26 (EditGroup adjustor retirement batch): a handwritten walk
//            carried this claim until the rename; it walked
//            `object - vtordisp - 2584 - 0xAA8` (2/3, wrong 0xa1c immediate),
//            then the repaired `- 2584 - 0xAA4`, and after the rename
//            `- 0xAA4` alone, all measured BYTE_EXACT from
//            src/adjustor_thunks.cpp. The generated thunk measures the same
//            span (`2b 49 fc 81 e9 18 0a 00 00 e9 <rel32>`); the marker moved
//            here when the handwritten body was retired.
Status: Complete
*/

/*
Purpose: Tear down all ten controls - close each one, free its heap text - then
         republish the default and delegate to the Dialog base.
// ORIGINAL: 0x00611A90 ?close@EditGroup@@QAEXXZ 0x00611A90-0x00611AF0 FILE BYTE_EXACT
// LEVER: PROMOTED out of src/recovered/units/00611a90.cpp, whose claim proved the ARTIFACT reproduced while the shipped program held a pending_bodies forwarder. The global is named - `EditGroupDefault` rather than `*g_009b8fcc` - but the OFFSET stays raw.
// TRIED: `field_84_ = EditGroupDefault` for that store, the spelling this file uses elsewhere - measured identical at 25 of 34, so it was NOT the cause of the ebx/ebp difference and the raw offset is kept only because it is what the artifact proved.
// LEVER: return-type-is-void the header declared `uint32_t close()` and the census reconstructs `?close@EditGroup@@QAEXXZ`. The census is right and the header was wrong: as `uint32_t` this is 25 of 34, as `void` it is 34 of 34. I had reasoned it could not matter - the body never sets eax and the trailing `call` to Dialog::close leaves its own return value there - which was true about the LAST instruction and wrong about the whole body: propagating a return value keeps `this` live to the end, so VC6 holds it in ebx where the image uses ebp.
// symbol    ?close@EditGroup@@QAEXXZ
// size      96 bytes
// kind      game
Return Value: n/a
Status: Complete
*/
namespace {
// A LOCAL SHIM, not vtable_shim.h's VCall. Only DECLARATION ORDER fixes a
// slot; a slot's signature is chosen to match the call site, and the shared
// VCall spells slot000 with no arguments where this one pushes an int.
class CloseVCall { public:
    virtual void slot000(int);
};
}  // namespace

void EditGroup::close() {
    char *self = reinterpret_cast<char *>(this);
    char *esi = self + 4;
    int ebx = 10;
    do {
        int *slot_a = reinterpret_cast<int *>(esi);
        int ctrl = *slot_a;
        if (ctrl != 0) {
            reinterpret_cast<CloseVCall *>(ctrl)->slot000(1);
            *slot_a = 0;
            *reinterpret_cast<int *>(esi + 0x50) = 10;
        }
        int *slot_b = reinterpret_cast<int *>(esi + 0x28);
        int freeable = *slot_b;
        if (freeable != 0) {
            free(reinterpret_cast<void *>(freeable));
            *slot_b = 0;
        }
        esi += 4;
    } while (--ebx != 0);

    // THE RAW OFFSET, not `field_84_ = EditGroupDefault`. Both read the
    // same global into the same slot, but a member access keeps `this`
    // live differently and VC6 then holds it in ebx where the image uses
    // ebp - 25 of 34 instead of 34 of 34.
    field_84_ = EditGroupDefault;

    char *vtable = *reinterpret_cast<char **>(self);
    int voffset = *reinterpret_cast<int *>(vtable + 8);
    reinterpret_cast<Dialog *>(self + voffset)->close();
}

// ====================
// Window-event handlers
// ====================

/*
Purpose: Hand dialog focus to the group's edit boxes: with a nonzero value,
         to the first box only; with zero, to each of the ten in turn, each
         through its own vtable slot 51 and only when installed. Entered on
         the GraphicWin-subobject adjusted pointer (the family's thunk
         convention), so the box-pointer array sits 0x88 below `this`.
// ORIGINAL: 0x00612670 ?on_dialog_focus@EditGroup@@QAEXH@Z 0x00612670-0x006126B6 BYTE_EXACT
// symbol    ?on_dialog_focus@EditGroup@@UAEXH@Z
// size      70 bytes
// prototype void (__thiscall ?on_dialog_focus@EditGroup@@QAEXH@Z)(EditGroup* this, int)
// callers   1   call targets   0
// kind      game
// indirect  0x00612685 0x006126A5
//
// PROMOTED out of the archived verification unit for this address, which proved the body
// against the same shim shape under its own name.
// TRIED (measured 2x at -0x118 vs the image's -0x88, SHAPE_EXACT 24/26):
// spelling the body as a GraphicWin override - the header had inherited the
// virtual - makes VC6 enter it 0x90 high (vtordisp-adjusted) and fold that
// into every displacement; inline casts vs a named `self` local made no
// difference to it.
// Original table6708D4 slot51 selects this body through thunk6127C0.
// The old long overload matched a raw-offset body but left actual virtual
// dispatch empty. Correct EditGroup ownership puts GraphicWin at8C, so the
// compiler's real override can express the original member displacements.
Return Value: n/a
Status: Complete
*/
void EditGroup::on_dialog_focus(int focus_id) {
    if (focus_id != 0) {
        EditBox *box = boxes_[0];
        if (box != 0) {
            box->on_dialog_focus(focus_id);
        }
        return;
    }
    EditBox **slot = boxes_;
    int count = 10;
    do {
        EditBox *box = *slot;
        if (box != 0) {
            box->on_dialog_focus(0);
        }
        ++slot;
    } while (--count);
}

/*
Purpose: Add one labelled edit field to the group: the group keeps ten boxes,
         and this refuses the eleventh. The box's stored text is replaced (or
         allocated) from the initial text, the label is published for the item
         list's insertion hook, the entry is appended under the field's
         ordinal, and the character limit lands in the group's limit array.

// ORIGINAL: 0x00611F80 ?edit@EditGroup@@QAEHPADPADH@Z 0x00611F80-0x00612040 FILE
// symbol    ?edit@EditGroup@@QAEHPAD0H@Z
// size      192 bytes
// prototype int (__thiscall ?edit@EditGroup@@QAEHPADPADH@Z)(EditGroup* this, int8*, int8*, int)
// callers   1   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00401100
// TRIED (forwarder): DipEdit::on_button_clicked drives it twice on BasePop's
//         EditGroup subobject at 0x22C8 (see BasePop::edit_group), once per
//         atrocity-prompt field.
Status: Coverage transcription - unclaimed body
*/
int EditGroup::edit(char *label_text, char *initial_text, int limit_value) {
    const int index = item_list_.entry_count_;
    if (index == 10) {
        return 1;
    }
    if (initial_text != 0) {
        if (text_buffers_[index] != 0) {
            free(text_buffers_[index]);
        }
        text_buffers_[index] =
            static_cast<char *>(mem_get(strlen(initial_text) + 1));
        if (text_buffers_[index] == 0) {
            return 4;
        }
        text_buffers_[index][0] = 0;
        strcat(text_buffers_[index], initial_text);
    }
    item_list_.pending_text_ = label_text;
    item_list_.field_20_ = 0;
    item_list_.field_24_ = 0;
    if (item_list_.add(index) != 0) {
        return 4;
    }
    limits_[index] = limit_value;
    return 0;
}

/*
Purpose: Replace the standing default text of one field: a null label clears
         the stored copy in place, otherwise the stored copy is reallocated
         to the new text. The entry list is not touched.

// ORIGINAL: 0x006120D0 ?set_default@EditGroup@@QAEHPADH@Z 0x006120D0-0x00612144 FILE
// size      116 bytes
// prototype int (__thiscall ?set_default@EditGroup@@QAEHPADH@Z)(EditGroup* this, int8*, int)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// TRIED (forwarder): BasePop::start (0x00601BF0) calls it BY NAME with the
//         caller's override text, so the call site needs the member's real
//         address to emit the image's `E8`.
Status: Coverage transcription - unclaimed body
*/
int EditGroup::set_default(char *default_text, int index) {
    if (default_text == 0) {
        if (text_buffers_[index] != 0) {
            text_buffers_[index][0] = 0;
        }
        return 0;
    }
    if (text_buffers_[index] != 0) {
        free(text_buffers_[index]);
    }
    text_buffers_[index] =
        static_cast<char *>(mem_get(strlen(default_text) + 1));
    if (text_buffers_[index] == 0) {
        return 4;
    }
    text_buffers_[index][0] = 0;
    strcat(text_buffers_[index], default_text);
    return 0;
}

// 0x00612150. Measure the caption and current edit contents independently,
// then retain the dialog's explicit row/column constraints.
// ORIGINAL: 0x00612150 ?calculate_dimensions@EditGroup@@QAEHXZ 0x00612150-0x0061244A FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/00612150.cpp retained original identity, full spans and prior recovery evidence", "recovery/tasks/dialog-control-coverage-wave.json authorized coupled control ownership", "original:0x612150 complete control/data-access review and src/editgroup.h genuine declaration; cross-owner discoveries preserved in task history"], "reason": "Complete Dialog-control behavior belongs to this genuine control owner, with real inherited members, embedded controls, Win receivers and callback dispatch rather than fixed executable calls. Compatible seed identity and historical evidence remain retained; catalogue decorations and emitted ABI are separate facts."}
// TRIED: MISMATCH #1 push/mov - re-deriving the vbtable-offset macros at every access (matching the original's frequent [esi] re-fetch) gave the allocator no reason to keep `this` pinned in esi or the 0 constant pinned in ebp across the body.
int EditGroup::calculate_dimensions() {
    Dialog &dialog = *this;
    Buffer *const buffer = static_cast<Buffer *>(static_cast<GraphicWin *>(this));
    if (dialog.item_list_.entry_count_ == 0) {
        return 7;
    }
    buffer->set_text_color(text_color_a_, text_color_b_, text_color_c_, text_color_d_);
    buffer->set_text_color2(text_color_2a_, text_color_2b_, text_color_2c_, text_color_2d_);
    buffer->set_text_color3(text_color_3a_, text_color_3b_, text_color_3c_, text_color_3d_);
    buffer->set_font(Dialog::font1_, Dialog::font2_, Dialog::font3_, 0);

    dialog.field_58_ = dialog.item_list_.entry_count_;
    if (dialog.field_58_ > dialog.field_50_) {
        dialog.field_58_ = dialog.field_50_;
    }
    dialog.item_list_.current_ = dialog.item_list_.head_;
    if (dialog.item_list_.entry_count_ > 0) {
        int step = abs(-1);
        if (step <= dialog.item_list_.entry_count_) {
            step = abs(-1);
            while (step > 0) {
                --step;
                dialog.item_list_.current_ = dialog.item_list_.current_->previous;
            }
            dialog.item_list_.current_position_ = dialog.item_list_.entry_count_ - 1;
        }
    }

    label_width_ = 0;
    edit_width_ = 0;
    for (int i = 0; i < dialog.item_list_.entry_count_; ++i) {
        if (dialog.item_list_.head_ != 0) {
            dialog.item_list_.current_ = dialog.item_list_.current_->next;
            ++dialog.item_list_.current_position_;
            if (dialog.item_list_.current_position_ == dialog.item_list_.entry_count_) {
                dialog.item_list_.current_position_ = 0;
            }
        }
        StringStructEntry *const entry = dialog.item_list_.current_;
        char *label = entry == 0 || entry->payload == 0 ? 0 : entry->payload->text;
        const int measured_label = buffer->text_width(label);
        if (label_width_ < measured_label) label_width_ = measured_label;
        const int measured_edit = buffer->text_width(text_buffers_[i]);
        if (edit_width_ < measured_edit) {
            edit_width_ = measured_edit + dialog.field_40_ * 2;
        }
    }

    if (edit_width_ <= 10) edit_width_ = 10;
    dialog.field_64_ = dialog.field_34_
        + (dialog.field_44_ + dialog.field_40_) * 2
        + edit_width_ + label_width_;
    dialog.field_68_ = buffer->text_line_height() + dialog.field_44_;

    if ((dialog.field_20_ & 2) == 0 || dialog.field_2C_ == 0) {
        int width = dialog.field_64_ * dialog.field_4C_;
        if (width < dialog.field_2C_) width = dialog.field_2C_;
        dialog.field_5C_ = width;
        dialog.field_64_ = width / dialog.field_4C_;
        dialog.field_54_ = dialog.field_4C_;
    } else {
        dialog.field_5C_ = dialog.field_2C_;
        dialog.field_54_ = dialog.field_2C_ / dialog.field_64_;
    }
    if ((dialog.field_20_ & 1) != 0 && dialog.field_30_ != 0) {
        dialog.field_60_ = dialog.field_30_;
        dialog.field_58_ = dialog.field_30_ / dialog.field_68_;
    } else {
        dialog.field_60_ = dialog.field_68_ * dialog.field_58_ + dialog.field_44_;
    }
    return 0;
}

// ORIGINAL: 0x00611AF0 ?attach@EditGroup@@QAEHPAUGraphicWin@@HHH@Z 0x00611AF0-0x00611F79;0x00663047-0x0066305C FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/00611af0.cpp retained original identity, full spans and prior recovery evidence", "recovery/tasks/dialog-control-coverage-wave.json authorized coupled control ownership", "original:0x611af0 complete control/data-access review and src/editgroup.h genuine declaration; cross-owner discoveries preserved in task history"], "reason": "Complete Dialog-control behavior belongs to this genuine control owner, with real inherited members, embedded controls, Win receivers and callback dispatch rather than fixed executable calls. Compatible seed identity and historical evidence remain retained; catalogue decorations and emitted ABI are separate facts."}
// TRIED: no source-level trick reproduced the SEH frame (the trigger here is a heap `new EditBox` + placement ctor, not a local by-value object of non-trivial type, so LEVER #1 does not apply); the two virtual-base displacements (vtable[4], vtable[8], read fresh per the disassembly's own re-fetch pattern) were kept as raw offset arithmetic rather than named fields, matching the "class deliberately empty" note; VCall extended to slot062 (used on the freshly built EditBox) purely by declaration-order padding. The row/column geometry (x0/cellRight/halfStore/y0) is a best-effort re-derivation from the register shuffle and likely has the least fidelity of the eight; the call ORDER and TARGETS (calculate_dimensions, GraphicWin::init, three set_text_color + set_font pairs, the delete-old/new/ctor/init/set_text sequence per cell) are transcribed directly from the disassembly. Diverges at #0.
// symbol    ?attach@EditGroup@@UAEHPAVGraphicWin@@HHH@Z
int EditGroup::attach(GraphicWin *parent, int x, int y, int flags) {
    if (parent == 0) return 3;
    calculate_dimensions();
    Dialog::field_24_ = x;
    Dialog::field_28_ = y;
    if ((parent->iFlags_ & 0x1000) == 0) flags |= 0x100000;
    const int result = GraphicWin::init(x, flags, Dialog::field_5C_,
                                        Dialog::field_60_, 0, y, parent, 0, 0);
    if (result != 0) return result;

    Buffer *const buffer = static_cast<Buffer *>(static_cast<GraphicWin *>(this));
    buffer->set_text_color(text_color_a_, text_color_b_, text_color_c_, text_color_d_);
    buffer->set_text_color2(text_color_2a_, text_color_2b_, text_color_2c_, text_color_2d_);
    buffer->set_text_color3(text_color_3a_, text_color_3b_, text_color_3c_, text_color_3d_);
    buffer->set_font(Dialog::font1_, Dialog::font2_, Dialog::font3_, 0);

    int child_y = static_cast<int>(Dialog::field_68_) -
                  static_cast<int>(Dialog::field_44_);
    int child_x = static_cast<int>(Dialog::field_44_) >> 1;
    int index = 0;
    for (int row = 0; row < static_cast<int>(Dialog::field_54_); ++row) {
        const int cell_width = static_cast<int>(Dialog::field_64_);
        const int label_width = static_cast<int>(Dialog::field_34_);
        const int label_x = cell_width * row + label_width +
                            static_cast<int>(Dialog::field_44_) * 2 + label_width_;
        const int label_right = cell_width + label_width + cell_width * row;
        for (int column = 0; column < static_cast<int>(Dialog::field_58_); ++column) {
            if (boxes_[index] != 0) {
                delete boxes_[index];
                boxes_[index] = 0;
            }
            boxes_[index] = new EditBox();
            if (boxes_[index] == 0) return 4;

            const int box_width = label_x - child_x;
            const int box_height = label_right - child_y;
            const int box_result = boxes_[index]->init(
                child_y, child_x, box_width, box_height, this, 1);
            if (box_result != 0) return box_result;

            if ((Dialog::field_20_ & 0x100000) != 0) {
                boxes_[index]->field_B3C_ |= 2;
            }
            Buffer *const edit_buffer = static_cast<Buffer *>(boxes_[index]);
            edit_buffer->set_text_color(text_color_a_, text_color_b_,
                                        text_color_c_, text_color_d_);
            edit_buffer->set_text_color2(text_color_2a_, text_color_2b_,
                                         text_color_2c_, text_color_2d_);
            edit_buffer->set_text_color3(text_color_3a_, text_color_3b_,
                                         text_color_3c_, text_color_3d_);
            edit_buffer->set_font(Dialog::font1_, Dialog::font2_, Dialog::font3_, 0);
            boxes_[index]->field_B14_ = limits_[index];
            boxes_[index]->set_text(text_buffers_[index]);
            boxes_[index]->field_B1C_ = Dialog::field_6C_;
            boxes_[index]->field_B28_ = field_AC_;
            boxes_[index]->field_B2C_ = field_B0_;
            boxes_[index]->field_B30_ = field_B4_;
            boxes_[index]->field_B34_ = Dialog::field_40_;
            boxes_[index]->field_B40_ = 0;
            boxes_[index]->field_B44_ = strlen(boxes_[index]->text_);
            boxes_[index]->redraw();
            boxes_[index]->show(0);

            child_y += Dialog::field_68_;
            child_x += Dialog::field_68_;
            ++index;
        }
    }
    return 0;
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00612450 ?on_redraw@EditGroup@@QAEXXZ 0x00612450-0x00612669 FILE
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00612450.cpp retained original identity, full spans and prior recovery evidence", "recovery/tasks/dialog-control-coverage-wave.json authorized coupled control ownership", "original:0x612450 complete control/data-access review and src/editgroup.h genuine declaration; cross-owner discoveries preserved in task history"], "reason": "Complete Dialog-control behavior belongs to this genuine control owner, with real inherited members, embedded controls, Win receivers and callback dispatch rather than fixed executable calls. Compatible seed identity and historical evidence remain retained; catalogue decorations and emitted ABI are separate facts."}
// symbol    ?on_redraw@EditGroup@@UAEXXZ
void EditGroup::on_redraw() {
    Win *const parent = win_parent_;
    if (parent == 0) return;
    WinCallbackWindow = parent;
    const int handled = parent->vslot_10(
        static_cast<int>(reinterpret_cast<uintptr_t>(
            static_cast<GraphicWin *>(this))));
    if (handled == 0) {
        static_cast<GraphicWin *>(parent)->copy(
            this, Dialog::field_24_, Dialog::field_28_, 0, 0,
            static_cast<int>(dib_.bmiHeader.biWidth),
            -static_cast<int>(dib_.bmiHeader.biHeight));
    }

    const int columns = static_cast<int>(Dialog::field_54_);
    const int rows = static_cast<int>(Dialog::field_58_);
    int index = 0;
    for (int column = 0; column < columns; ++column) {
        for (int row = 0; row < rows && index < item_list_.entry_count_; ++row) {
            if (item_list_.seek_pos(index)) {
                RECT label_area;
                label_area.left = column * static_cast<int>(Dialog::field_64_) +
                                  static_cast<int>(Dialog::field_40_);
                label_area.top = row * (static_cast<int>(Dialog::field_68_) +
                                        static_cast<int>(Dialog::field_44_)) +
                                 (static_cast<int>(Dialog::field_44_) >> 1);
                label_area.right = label_area.left + label_width_;
                label_area.bottom = label_area.top + Dialog::field_68_;
                char *const label = item_list_.current_text();
                if (label != 0) {
                    static_cast<Buffer *>(static_cast<GraphicWin *>(this))->write_l(
                        label, &label_area, strlen(label));
                }
            }
            if (boxes_[index] != 0) boxes_[index]->on_redraw();
            ++index;
        }
    }
    soft_update();
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x006126C0 ?pass_dialog_focus@EditGroup@@QAEXXZ 0x006126C0-0x006126F8 FILE
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/006126c0.cpp retained original identity, full spans and prior recovery evidence", "recovery/tasks/dialog-control-coverage-wave.json authorized coupled control ownership", "original:0x6126c0 complete control/data-access review and src/editgroup.h genuine declaration; cross-owner discoveries preserved in task history"], "reason": "Complete Dialog-control behavior belongs to this genuine control owner, with real inherited members, embedded controls, Win receivers and callback dispatch rather than fixed executable calls. Compatible seed identity and historical evidence remain retained; catalogue decorations and emitted ABI are separate facts."}
// symbol    ?pass_dialog_focus@EditGroup@@UAEXXZ
void EditGroup::pass_dialog_focus() {
    if (list_.count_ != 0 && list_.head_ != 0) {
        Win *const focused =
            static_cast<WinNodeListFocusEntry *>(list_.current_)->window;
        if (focused != 0) {
            focused->pass_dialog_focus();
            return;
        }
    }
    Win::pass_dialog_focus();
}
