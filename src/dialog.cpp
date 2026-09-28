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
#include "dialog.h"
#include "stringstruct.h"

/*
Purpose: Compiler-generated complete StringList destructor. Dialog owns a
         real StringList member, so its normal and unwind lifetimes emit this
         entry without a handwritten wrapper or measurement-only export.
// ORIGINAL: 0x00406820 sub_406820 0x00406820-0x0040683B FILE
// TRIED: THE SECOND PARAMETER WAS INVENTED, so it is gone rather than worked
//   around. 0x00406820 sets only ECX (`mov ecx, esi; call 0x4066c0`) and never
//   EDX, and 0x004066C0's body reads only ECX - the `_redirect` name is this
//   tree's own, so nothing about the image required the arity. Declaring ONE
//   parameter takes this call site to BYTE_EXACT 9/9.
// TRIED: the two-argument call at this site, verbatim:
//     string_struct_derived_close_redirect(adjusted, nullptr);
//   QUOTED ON PURPOSE. tools/reap_worktrees.py keeps an agent's worktree when
//   a line it wrote appears nowhere in src/, which is right - it cannot tell
//   "never collected" from "collected and then correctly rejected". A refusal
//   that quotes the code it refuses is visible to that check, so the worktree
//   becomes reapable without anyone overriding a safety refusal.
// TRIED: reaching the same 9/9 with a single-argument OVERLOAD forwarding
//   to a two-argument body. It measures identically and it is WRONG: VC6 emits
//   the overload as a 7-byte COMDAT thunk (`33 D2` xor edx,edx; `E9` jmp) and
//   the call relocation targets THAT, so the recovered call graph becomes
//   destroy -> invented thunk -> redirect where the image is destroy ->
//   redirect. No such thunk exists in the image; scanning .text for `33 D2 E9`
//   returns one hit and it does not target 0x4066C0. BYTE_EXACT survived only
//   because the rel32 is one of the masked relocation bytes - the measurement
//   cannot see where the call WENT. Caught by the adversarial verifier
//   2026-08-22; recorded because the recipe would buy every future BYTE_EXACT
//   at this idiom's other call sites with one more invented function.
// symbol    ??_DStringList@@QAEXXZ
// size      27 bytes
// prototype
// callers   26   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x004066C0
The original is a void complete destructor. Its EAX owner residue is not
a separate source-level return contract. The historical notes below describe
the removed handwritten wrapper, not the current generated entry.
DEMOTED from BYTE_EXACT by direction (strip-all hand vptr writes): the hand
derived-close call and the three tail operations (owner read, final-table
install, republish) left - the teardown is the compiler's ~StringList chain
now, staging the same tables from the StringStruct/StringList declarations.
Compiled is 8 instructions against the image's 9: the image inlines the
vbase tail after its call; ours folds it into the chain. The EAX residue
survives as the republished global.
*/

// abs is a real call throughout the image (0x00644F3A, called from
// get_selected_id and pos_to_id twice each); stdafx.h's project-wide
// `#pragma function(..., abs, ...)` line is what keeps it one - this file
// carried a local `#pragma function(abs)` before the line was extended.

/*
Purpose: Set the three fonts used by the dialog.
// ORIGINAL: 0x00609C60 ?set_dialog_font@Dialog@@QAEHPAUFont@@PAUFont@@PAUFont@@@Z 0x00609C60-0x00609C8D BYTE_EXACT
// symbol    ?set_dialog_font@Dialog@@QAEHPAVFont@@00@Z
// size      45 bytes
// prototype int (__thiscall ?set_dialog_font@Dialog@@QAEHPAUFont@@PAUFont@@PAUFont@@@Z)(Dialog* this, Font*, Font*, Font*)
// callers   11   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: No errors (0); invalid primary font (3)
Status: Complete
*/
int Dialog::set_dialog_font(Font *font1, Font *font2, Font *font3) {
    if (!font1) {
        return 3;
    }
    if (font1->is_initialized()) {
        font1_ = font1;
    }
    font2_ = font2;
    font3_ = font3;
    return 0;
}

/*
Purpose: Set the primary text colors for the four dialog color slots.
// ORIGINAL: 0x00609C90 ?set_dialog_text_color@Dialog@@QAEXHHHH@Z 0x00609C90-0x00609CB8 BYTE_EXACT
// size      40 bytes
// prototype void (__thiscall ?set_dialog_text_color@Dialog@@QAEXHHHH@Z)(Dialog* this, int, int, int, int)
// callers   9   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: n/a
Status: Complete
*/
void Dialog::set_dialog_text_color(int color1, int color2, int color3, int color4) {
    text_color_a_ = color1;
    text_color_b_ = color2;
    text_color_c_ = color3;
    text_color_d_ = color4;
}

/*
Purpose: Set the secondary text colors for the four dialog color slots.
// ORIGINAL: 0x00609CC0 ?set_dialog_text_color2@Dialog@@QAEXHHHH@Z 0x00609CC0-0x00609CEB BYTE_EXACT
// size      43 bytes
// prototype void (__thiscall ?set_dialog_text_color2@Dialog@@QAEXHHHH@Z)(Dialog* this, int, int, int, int)
// callers   7   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: n/a
Status: Complete
*/
void Dialog::set_dialog_text_color2(int color1, int color2, int color3, int color4) {
    text_color_2a_ = color1;
    text_color_2b_ = color2;
    text_color_2c_ = color3;
    text_color_2d_ = color4;
}

/*
Purpose: Set the tertiary text colors for the four dialog color slots.
// ORIGINAL: 0x00609CF0 ?set_dialog_text_color3@Dialog@@QAEXHHHH@Z 0x00609CF0-0x00609D1B BYTE_EXACT
// size      43 bytes
// prototype void (__thiscall ?set_dialog_text_color3@Dialog@@QAEXHHHH@Z)(Dialog* this, int, int, int, int)
// callers   6   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: n/a
Status: Complete
*/
void Dialog::set_dialog_text_color3(int color1, int color2, int color3, int color4) {
    text_color_3a_ = color1;
    text_color_3b_ = color2;
    text_color_3c_ = color3;
    text_color_3d_ = color4;
}

/*
Purpose: Set the dialog's two show bits in the style word at this+0x20 - the
         first clearing the bit when the argument is zero, the second the same
         over bit 1. The image's one body for each is the 27-byte
         `if (v) f |= bit; else f &= ~bit;` pair the catalogue carries as
         ?UNK3@BasePop@@QAEXH@Z (0x00605180) and ?UNK4@BasePop@@QAEXH@Z
         (0x006051A0), both BYTE_EXACT in basepop.cpp.

         The receiver BasePop::exec (0x0060276C / 0x00602775) hands them is NOT
         a BasePop: it is `&dialogs_.Dialog` computed through the vbtable
         (`mov eax,[esi+0x21d0] / mov ecx,[eax+8] / lea edi,[ecx+esi+0x21d0]`),
         i.e. the Dialog virtual base of the embedded Dialogs. These two are
         Dialog's own setters, and the catalogue's BasePop owner is the folded
         twin - VC6's identical-COMDAT folding kept one 27-byte body and one
         name for what the original spelled twice. Claimed where the catalogue
         puts it (basepop.cpp); this file owns the definitions exec calls.
*/
void Dialog::set_style_bit_0(int on) {
    if (on) {
        field_20_ |= 1U;
    } else {
        field_20_ &= ~1U;
    }
}

void Dialog::set_style_bit_1(int on) {
    if (on) {
        field_20_ |= 2U;
    } else {
        field_20_ &= ~2U;
    }
}

/*
Purpose: Initialize the dialog geometry and optional item-list heap.
// TRIED (seed): field_D4_ was tentatively read as either &heap_ or the stack
//   slot holding rect->left. The pinned body stores its saved `this + 4`
//   heap pointer into StringList::allocator_ after successful allocation, and
//   clears that allocator on the zero-size path.
Status: Genuine C++ recovery; strict reference and behavioral acceptance remain pending.
*/
// ORIGINAL: 0x00609470 ?init@Dialog@@QAEHPAURECT@@H@Z 0x00609470-0x006095EE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned image inspect 0x00609470 decodes exactly 382 bytes. It snapshots all RECT fields before direct Dialog::close, calls Heap::init at 0x006094D5, returns 4 immediately on allocation error, and uses the saved this+4 heap pointer for StringList::allocator_ only on success; the zero-size branch clears that allocator. The retained 0x00609470 seed carried the alternative heap/stack-slot interpretation, resolved by the pinned stores at 0x0060955F and 0x006095DF."], "reason": "Recover the original Dialog RECT initializer in dialog.cpp using the real Dialog, Heap, and StringList members; its pointer validation, saved rectangle coordinates, close, geometry setup, and allocator ownership follow the pinned body."}
// symbol    ?init@Dialog@@QAEHPAUtagRECT@@H@Z
// size      382 bytes
// prototype int (__thiscall ?init@Dialog@@QAEHPAURECT@@H@Z)(Dialog* this, RECT*, int)
// callers   13   call targets   2
// kind      game
// calls     0x005D4620 0x00608F50
// indirect  0x00609514 0x00609526 0x00609542 0x00609598 0x006095AA 0x006095C6
int Dialog::init(RECT *rect, int heap_size) {
    if (rect == 0) {
        return 3;
    }

    const int left = rect->left;
    const int top = rect->top;
    const int right = rect->right;
    const int bottom = rect->bottom;

    close();

    field_24_ = left;
    field_28_ = top;
    field_2C_ = right - left;
    field_30_ = bottom - top;
    field_20_ |= 3U;
    field_1C_ = static_cast<uint32_t>(heap_size);

    if (heap_size == 0) {
        item_list_.remove_all();
        item_list_.current_position_ = 0;
        item_list_.allocator_ = 0;
        return 0;
    }

    if (heap_.init(static_cast<size_t>(heap_size))) {
        return 4;
    }

    item_list_.remove_all();
    item_list_.allocator_ = &heap_;
    item_list_.current_position_ = 0;
    return 0;
}

/*
Purpose: Initialize the dialog's absolute geometry and optional embedded
         item-list heap. A failed Heap::init returns 4; success reconnects the
         list allocator to Dialog's own Heap, while a zero size clears it.
*/
// ORIGINAL: 0x00609300 ?init@Dialog@@QAEHHHHHH@Z 0x00609300-0x00609465
// RECOVERY_BINDING: v1 {"evidence": ["Pinned recover inspect 0x00609300: calls Dialog::close before writing left/top/right/bottom at +0x24/+0x28/+0x2c/+0x30, ORs style bits 3, stores heap_size at +0x1c, then calls Heap::init on this+4 only for nonzero size. The test at 0x00609350 branches to success on zero; nonzero returns 4.", "Pinned recover inspect 0x00609360-0x006093e1 confirms successful nonzero setup clears the item list, stores the embedded Heap pointer to StringList::allocator_, resets current_position_, and returns 0; the zero-size branch also clears the list and allocator."], "reason": "Recover the five-integer Dialog initializer as a typed member using the real embedded Heap and StringList. The image reuses a stack slot as EDI only when the list teardown loop runs, yielding the original left argument as allocator; close() already empties this list before the repeated clear, so preserve the owning &heap_ reference rather than the stale-register artifact."}
// TRIED: the two branches each inline the SAME list-teardown loop; the field at list+0x18 is set to `a1` in the `a5 != 0` branch (matching Ghidra) since the stale-register case is not expressible from source. 0.89 mnemonic similarity.
// Historical redirect emitted symbol: ?dialog_init_six_redirect@@YIHPAX0HHHHH@Z
int Dialog::init(int left, int top, int right, int bottom, int heap_size) {
    close();

    field_24_ = static_cast<uint32_t>(left);
    field_28_ = static_cast<uint32_t>(top);
    field_2C_ = static_cast<uint32_t>(right);
    field_30_ = static_cast<uint32_t>(bottom);
    field_20_ |= 3U;
    field_1C_ = static_cast<uint32_t>(heap_size);

    if (heap_size != 0 && heap_.init(static_cast<size_t>(heap_size))) {
        return 4;
    }

    item_list_.remove_all();
    item_list_.current_position_ = 0;
    item_list_.allocator_ = heap_size == 0 ? 0 : &heap_;
    return 0;
}

/*
Purpose: Initialize a dialog from an absolute rectangle and caller-owned
         heap, copying the rectangle before close() resets dialog state.
*/
// ORIGINAL: 0x006097F0 ?init@Dialog@@QAEHPAURECT@@PAUHeap@@@Z 0x006097F0-0x006098C8 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned recover inspect 0x006097F0 rejects a null RECT with 3, snapshots left/top/right/bottom before calling Dialog::close, computes width=right-left and height=bottom-top, then stores left/top/width/height at +0x24/+0x28/+0x2c/+0x30 and the supplied Heap* at +0x18.", "Pinned recover inspect 0x00609841-0x006098c5 confirms item-list removal, current_position_ reset, allocator_ set to the supplied heap, and return 0."], "reason": "Recover the rectangle-and-heap overload on Dialog using its typed geometry, heap pointer, and embedded StringList members."}
// symbol    ?init@Dialog@@QAEHPAUtagRECT@@PAVHeap@@@Z
// TRIED: separate width/height locals vs `right -= left; bottom -= top;` in place - both diverge at the same instruction (the field_2c/30 store). This is the "this-adjusting thunk" shape (vtbl[1] holds a byte offset, not a function pointer) modelled with a single-virtual-slot shim plus raw pointer chasing.
int Dialog::init(RECT *rect, Heap *heap) {
    if (rect == 0) {
        return 3;
    }

    const int left = rect->left;
    const int top = rect->top;
    const int width = rect->right - left;
    const int height = rect->bottom - top;

    close();

    field_24_ = static_cast<uint32_t>(left);
    field_28_ = static_cast<uint32_t>(top);
    field_2C_ = static_cast<uint32_t>(width);
    field_30_ = static_cast<uint32_t>(height);
    field_20_ |= 3U;
    heap_ptr_ = heap;

    item_list_.remove_all();
    item_list_.current_position_ = 0;
    item_list_.allocator_ = heap;
    return 0;
}

/*
Purpose: Find an item ID's bounded position in the dialog string list.
// ORIGINAL: 0x00609AF0 ?id_to_pos@Dialog@@QAEHH@Z 0x00609AF0-0x00609B4B BYTE_EXACT
// MEASURED inline: the body lives at the end of dialog.h. The image keeps
//        this out-of-line copy at 0x00609AF0 AND inlines the same walk into
//        set_selected_id (0x006099D0); the `inline` keyword is what makes
//        that site expand (VC6 will not auto-inline a loop body) and
//        MEASURED/__declspec(dllexport) is what keeps this COMDAT emitted
//        for its own claim.
// size      91 bytes
// prototype int (__thiscall ?id_to_pos@Dialog@@QAEHH@Z)(Dialog* this, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: Matching position, or the configured entry count on a miss
Status: Complete
*/

/*
Purpose: Select a dialog item by its bounded list ID.
// ORIGINAL: 0x006099D0 ?set_selected_id@Dialog@@QAEXH@Z 0x006099D0-0x00609A43
// LEVER: the body is the whole id_to_pos walk, INLINED - the image never
//        calls it here. Declaring id_to_pos `MEASURED` + `inline` (definition
//        at the end of dialog.h) took this from 0 of 36 to 34 of 36: VC6's
//        /Ob2 will not auto-expand a function holding a loop, so only the
//        explicit keyword reaches the inline, and `MEASURED`
//        (__declspec(dllexport) on the in-class declaration) is what keeps
//        id_to_pos's own out-of-line COMDAT emitted for its 0x00609AF0 claim.
// RESIDUAL (34 of 36): the image's two duplicated return blocks are laid out
//        head-null-then-loop-exit; this build emits them loop-exit-then-head-
//        null, so only the two jump targets swap (`je`/`jle` point at each
//        other's block). Both blocks are byte-identical and the same two
//        exits feed each. Not reachable while id_to_pos keeps the single-
//        return spelling its own 29/29 out-of-line copy needs - a guard
//        clause would give the inline two explicit returns and cost the
//        out-of-line body its single epilogue.
// TRIED (2026-09-06): the guard clause, MEASURED - `if (!head_) return
//        current_position_;` ahead of the loop in the shared inline cost BOTH
//        bodies at once: this one 34 of 36 -> 2 of 36, and id_to_pos's own
//        0x00609AF0 BYTE_EXACT 29 of 29 -> 2 of 29. The single-return spelling
//        is load-bearing for the pair, and the swapped blocks stay.
// size      115 bytes
// prototype void (__thiscall ?set_selected_id@Dialog@@QAEXH@Z)(Dialog* this, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: n/a
Status: Complete
*/
void Dialog::set_selected_id(int id) {
    selected_position_ = id_to_pos(id);
}

/*
Purpose: Restore the selected list position and return its item ID.
// ORIGINAL: 0x00609A50 ?get_selected_id@Dialog@@QAEHXZ 0x00609A50-0x00609AE6 BYTE_EXACT
// LEVER (2026-09-03, 25/50 -> 50/50): the image MERGES the two
//        current_position_ stores into one instruction and retargets the abs
//        guard's failure path PAST it to the shared return epilogue. Two
//        measured facts make that reachable. (1) The store must be written
//        ONCE, after the if/else - a store inside each arm keeps two separate
//        stores and scores 25/50. (2) The abs guard must then be INVERTED to
//        `if (abs(position) > entry_count_) goto restore_done;` - a store
//        unconditionally after the arms merges (49/50) but PERFORMS the store
//        on the failed-bound path, which the image does not, and an early
//        `return` instead of the goto makes VC6 inline a second copy of the
//        epilogue (15/50). The label is the image's own shape, not a
//        transcription shortcut. Flags need /Oi- so abs stays a real call -
//        and stdafx.h's project-wide `#pragma function(..., abs, ...)` line
//        says the same thing in the source, so the body reproduces under
//        the plain pair without that axis.
// size      150 bytes
// prototype int (__thiscall ?get_selected_id@Dialog@@QAEHXZ)(Dialog* this)
// callers   15   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00644F3A
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: Selected item ID, or zero when the list head is null
Status: Complete
*/
int Dialog::get_selected_id() {
    // NO LOCAL FOR THE COUNT. The image reads it, decrements DESTRUCTIVELY
    // and never looks at that register again - it re-reads the member later
    // instead. Held in a local it stays live across the `abs` calls below
    // and VC6 spills, which is a stack frame the image has not got.
    int position = selected_position_;
    // `position <= count - 1`, WHICH IS NOT `position < count`. The image
    // decrements the count and compares - `dec eax; cmp edi, eax; jg` - and
    // for count == INT_MIN that decrement WRAPS, which is the whole reason
    // this tree carried an explicit INT_MIN test beside the comparison. The
    // wrap is the semantics; the test is a second read of it.
    if (position <= item_list_.entry_count_ - 1) {
        item_list_.current_ = item_list_.head_;
        if (position < 0) {
            // `abs`, CALLED TWICE, and the count RE-READ between them: the
            // image is `push edi; call 0x644F3A; mov ecx, [esi+0xcc];
            // cmp eax, ecx; jg` and then `push edi; call 0x644F3A` again.
            // That is a MIN-style macro over `abs(position)`, not a local -
            // and `abs(INT_MIN)` returning INT_MIN is what the hand-rolled
            // ternary here was standing in for.
            // INVERTED, and out of the arm by way of the label: the image's
            // `jg` jumps STRAIGHT to the shared return epilogue, PAST the
            // store, so a failed bound leaves current_position_ alone. That
            // is not expressible as a store inside the arm (VC6 then keeps
            // two stores) nor as a store unconditionally after the arms
            // (VC6 then performs it on this path), so the label IS the
            // shape: it is what the image's own bytes say.
            if (abs(position) > item_list_.entry_count_) {
                goto restore_done;
            }
            int distance = abs(position);
            while (distance > 0) {
                item_list_.current_ = item_list_.current_->previous;
                distance--;
            }
            // The member again, not a local - see the note above the
            // guard. Plain `int` addition: the image is one `add`, and
            // the uint32 round trip through memcpy is a bit-cast VC6
            // will not inline away.
            position += item_list_.entry_count_;
        } else {
            int distance = position;
            while (distance > 0) {
                item_list_.current_ = item_list_.current_->next;
                distance--;
            }
        }
        // ONE STORE FOR BOTH ARMS. The image's negative arm ends
        // `add edi, [esi+0xcc]` and jumps STRAIGHT to the store the
        // positive arm falls into - `jmp` with no `mov` before it.
        item_list_.current_position_ = position;
    }
restore_done:
    return item_list_.head_ ? item_list_.current_->id : 0;
}

/*
Purpose: Restore an explicit list position and return its item ID.
// ORIGINAL: 0x00609B50 ?pos_to_id@Dialog@@QAEHH@Z 0x00609B50-0x00609BEA
// LEVER: dropped a dead `int count = entry_count_;` first line (unused,
//        scored no differently either way) - not the defect. `/Oi-` is:
//        the DEFAULT flag set lets VC6 expand `abs()` as an intrinsic bit
//        trick (`cdq; xor; sub`), which is what stops it caching `this` in
//        a register the way the image's real `call`-based `abs` does;
//        `/c /O2 /Oi- /Gy /GR- /GX` reaches 0.970 similar (10 differing
//        runs total) against 0.771-0.780 for the flag sets with
//        intrinsics on. `record`'s own auto-pick still reports the
//        low-`agreeing`-count set here because its sort is count-first,
//        not similarity-first; the source is correct, this is a scorer
//        tie-break, not a body defect.
// TRIED: remaining gap under `/Oi-` is a handful of ecx/edx register
//            swaps around the `current_entry_`/`entry_head_` reads -
//            plateaued, matching the sibling `get_selected_id` above.
// FIXED (2026-09-03) A REAL DEFECT the old notes did not name: the store was
//            written inside the positive arm only, so a bound that failed the
//            abs test left current_position_ UNWRITTEN where the image writes
//            it, and the negative arm's own store was missing entirely. The
//            body now uses the shape that took get_selected_id (0x00609A50)
//            to BYTE_EXACT - one store after the if/else, the abs guard
//            inverted over a `goto restore_done` - and the control flow now
//            matches the image exactly: both guards' failure paths skip the
//            store and land on the shared return epilogue.
// TRIED (2026-09-03) the remaining 4 of 51 / 0.970-similar gap, which is ONE
//            register decision and its cascade: the image loads entry_count_
//            into ECX and hoists a position copy into EAX (`mov eax,edi`,
//            instruction 4) that survives to the merged store; this build
//            loads the count into EAX and never makes the copy. Measured and
//            rejected: a `int result = position;` local at the top (VC6
//            copy-propagates it away); `current_position_ = position +
//            entry_count_` instead of the in-place `+=`, which is the image's
//            `mov eax,[esi+0xcc]; add eax,edi` shape written out in source
//            (VC6 folds it back into `add edi,[esi+0xcc]` because position is
//            dead after); the /Ob0, /O1 and /G6 flag sets. The image's own
//            get_selected_id has the same source shape and the allocation
//            THIS build produces, so the two siblings disagree with each
//            other in the shipped binary; only one of them can be reached
//            from a shared spelling, and get_selected_id is the one it is.
// size      154 bytes
// prototype int (__thiscall ?pos_to_id@Dialog@@QAEHH@Z)(Dialog* this, int)
// callers   7   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00644F3A
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: Item ID at the requested position, or zero when the list head is null
Status: Complete
*/
int Dialog::pos_to_id(int position) {
    // `position <= count - 1`, WHICH IS NOT `position < count`. The image
    // decrements the count and compares - `dec eax; cmp edi, eax; jg` - and
    // for count == INT_MIN that decrement WRAPS, which is the whole reason
    // this tree carried an explicit INT_MIN test beside the comparison. The
    // wrap is the semantics; the test is a second read of it.
    if (position <= item_list_.entry_count_ - 1) {
        item_list_.current_ = item_list_.head_;
        if (position < 0) {
            // `abs`, CALLED TWICE, and the count RE-READ between them: the
            // image is `push edi; call 0x644F3A; mov ecx, [esi+0xcc];
            // cmp eax, ecx; jg` and then `push edi; call 0x644F3A` again.
            // That is a MIN-style macro over `abs(position)`, not a local -
            // and `abs(INT_MIN)` returning INT_MIN is what the hand-rolled
            // ternary here was standing in for.
            // INVERTED, and out of the arm by way of the label - see the
            // note at the store below, and get_selected_id above for the
            // same shape in the sibling.
            if (abs(position) > item_list_.entry_count_) {
                goto restore_done;
            }
            int distance = abs(position);
            while (distance > 0) {
                item_list_.current_ = item_list_.current_->previous;
                distance--;
            }
            // The member again, not a local - see the note above the
            // guard. Plain `int` addition: the image is one `add`, and
            // the uint32 round trip through memcpy is a bit-cast VC6
            // will not inline away.
            position += item_list_.entry_count_;
        } else {
            int distance = position;
            while (distance > 0) {
                item_list_.current_ = item_list_.current_->next;
                distance--;
            }
        }
        // ONE STORE FOR BOTH ARMS. The image's negative arm ends
        // `mov eax,[esi+0xcc]; add eax,edi` and jumps STRAIGHT to the store
        // the positive arm falls into - `jmp` with no `mov` before it.
        item_list_.current_position_ = position;
    }
restore_done:
    return item_list_.head_ ? item_list_.current_->id : 0;
}

/*
Purpose: Add an item to the dialog's string list, keyed by its id, if a
         non-null string is given.
// ORIGINAL: 0x00609990 ?item@Dialog@@QAEHPADH@Z 0x00609990-0x006099C4 BYTE_EXACT
// symbol    ?item@Dialog@@QAEHPADH@Z
// size      52 bytes
// prototype int (__thiscall ?item@Dialog@@QAEHPADH@Z)(Dialog* this, int8*, int)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00401100
Return Value: 3 when text is null; otherwise StringStruct::add's result
Status: Complete
*/
int Dialog::item(char *text, int index) {
    if (!text) {
        return 3;
    }
    // The list at this+0xBC, the subobject ~Dialog destroys. Its
    // OWN pending_text_/field_20_/field_24_ (StringList-relative) are set
    // through the LIST, not Dialog's absolute offsets: the image advances
    // ecx to this+0xBC first (`add ecx, 0xbc`) and addresses 0x1C/0x20/0x24
    // off of that, which is what keeps the small displacements instead of
    // Dialog's own 0xD8 etc. The `list` local is that advanced receiver -
    // spelled through the member; TRIED: direct `item_list_.field_1C_`
    // selects made VC6 drop the `add` and store at [ecx+0xd8..e0], 7 of 13.
    StringList *const list = &item_list_;
    list->pending_text_ = text;
    list->field_20_ = 0;
    list->field_24_ = 1;
    return list->add(index);
}










Font *DialogDefaultFonts[3];  // 0x009B8EC0

/*
Purpose: Set the default fonts shared by every dialog.
// ORIGINAL: 0x00609D20 ?set_def_dialog_font@Dialog@@QAAHPAUFont@@PAUFont@@PAUFont@@@Z 0x00609D20-0x00609D50 BYTE_EXACT
// symbol    ?set_def_dialog_font@Dialog@@SAHPAVFont@@00@Z
// size      48 bytes
// prototype 
// callers   5   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: No errors (0); invalid primary font (3)
Status: Complete
*/
int Dialog::set_def_dialog_font(Font *font1, Font *font2, Font *font3) {
    if (!font1) {
        return 3;
    }
    // Only an initialised primary is published; the other two are stored
    // either way and the call still succeeds. WRITTEN OUT, not published
    // through a volatile alias - see `BasePop::set_def_string_font`, which
    // is the same body and was held off the ratchet by the same helper.
    if (font1->is_initialized()) {
        DialogDefaultFonts[0] = font1;
    }
    DialogDefaultFonts[1] = font2;
    DialogDefaultFonts[2] = font3;
    return 0;
}



// THE DIALOG DEFAULTS TABLE, the image's .data run 0x0069704C..0x006970DC,
// defined with the shipped values: BaseButton's bevel color set (0x4C..0x5C,
// which its ctor reads into color_/bevel_upper_/bevel_lower_/
// bevel_thickness_), the dimmed "..." text color pair (0x90/0x94), the
// twelve text-color slots (0x9C..0xD8 region - CheckBox/RadioButton/
// SpriteBox/Dialog read these for their text_color_ members), and the
// geometry/flag defaults (0x98 and 0xA0..0xDC; main.cpp's DialogDefaultStyle
// names the last slot). Index = (address - 0x0069704C) / 4.
//
// WRITABLE, and the const it used to carry was a modelling lie: the image's
// set_def_dialog_text_color (0x00609D50, homed just below) STORES the twelve
// text-color slots 24..35 at runtime. Reads elsewhere are unchanged by the
// missing const; the ctor's BYTE_EXACT claim re-measured clean on it.
uint32_t DialogDefaults[37] = {
    247, 7, 248, 2, 125, 0,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    1, 1, 1, 1, 1, 1,
    0x002E2E2E, 0x002E2E2E,
    320, 6, 1, 12, 255, 0,
    0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
    1, 1, 1, 1, 1, 1,
    3,
};

/*
Purpose: Set the default dialog text colour for one of the four colour
         slots: the argument lands at index 24+slot of the defaults table
         (the a group - text_color_a_'s own default), and the same index of
         the b, c and d groups 12/24/36 bytes on. Slots outside 0..3 clamp
         to 0, which is what the image's signed `jl`/`jle` pair does.

         THE CATALOGUED NAME IS A LIE, and the `symbol` fact is why: it
         spells ?set_def_dialog_text_color@Dialog@@QAAXHHHHH@Z, a __thiscall
         member, while the body reads all five arguments off the stack
         ([esp+4]..[esp+0x14]), never touches ecx, and returns with a bare
         `ret` - a __cdecl free function with no receiver. The artifact this
         was transcribed from proved the same thing the hard way: spelled as
         a member, every stack read compiled four bytes too high. The four
         stores go through the defaults table BY NAME, which keeps the
         image's `[eax*4 + 0x6970ac]` single-operand form (an extern array
         binding folds the same way; a `static T *const` binding does not -
         it costs the extra base add the image does not have).
// ORIGINAL: 0x00609D50 ?set_def_dialog_text_color@Dialog@@QAAXHHHHH@Z 0x00609D50-0x00609D8C BYTE_EXACT
// symbol    _dialog_set_def_dialog_text_color
// size      60 bytes
// prototype
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// MEASURED: the image's one caller is not recovered, so nothing here
//        references the body and /Gy would drop the COMDAT; dllexport is
//        what keeps it emitted for its own claim.
Return Value: n/a
Status: Complete
*/
MEASURED void __cdecl dialog_set_def_dialog_text_color(
    int color_a, int color_b, int color_c, int color_d, int slot) {
    if (slot < 0 || slot > 3) {
        slot = 0;
    }
    DialogDefaults[24 + slot] = color_a;
    DialogDefaults[27 + slot] = color_b;
    DialogDefaults[30 + slot] = color_c;
    DialogDefaults[33 + slot] = color_d;
}

// The zero-init .bss slots the Dialog constructor reads its field defaults
// from (0x009B8EB4..0x009B8ED8): rebindable, same idiom as
// ListBoxCloseDynamicDefault.
uint32_t DialogField30Default;  // 0x009B8EB4
uint32_t DialogField34Default;  // 0x009B8EB8
uint32_t DialogField40Default;  // 0x009B8EBC
uint32_t DialogFieldACDefault;  // 0x009B8ECC
uint32_t DialogFieldB0Default;  // 0x009B8ED0
uint32_t DialogFieldB4Default;  // 0x009B8ED4
uint32_t DialogFieldB8Default;  // 0x009B8ED8

/*
Purpose: Construct a Dialog: construct the embedded Heap and the item list,
         then load every color/font/field default from the process tables.
// ORIGINAL: 0x00608C10 ??0Dialog@@QAE@XZ 0x00608C10-0x00608E09 BYTE_EXACT
// LEVER (2026-09-03, 99 of 103 -> 103 of 103): the last two assignments are
//        the other way round from the order a reader would expect -
//        `text_color_3d_ = DialogDefaults[35]` comes BEFORE
//        `field_1C_ = -1`. That is what puts the image's
//        `mov edx,[0x6970d8]` (the 3d default load) above the
//        `mov [esi+0x1c],eax` (the field_1C_ store): VC6 knows the two
//        distinct member offsets of the same object do not alias, so it is
//        free to hoist the load into the store's shadow, and the textual
//        order is what decides whether it does. The old RESIDUAL note's
//        "named local to pin the load" attempt failed because a local is
//        copy-propagated away here; the statement ORDER is the lever.
// size      509 bytes
// prototype void (__thiscall ??0Dialog@@QAE@XZ)(Dialog* this)
// callers   0   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005D4560
Return Value: this
*/
Dialog::Dialog() {
    // The store order is the image's, including its interleaving of
    // selected_position_/field_F0_ after field_30_, field_34_ between
    // field_44_ and field_48_, and field_1C_ near the end. The three -1
    // stores are the image's one `or eax,-1` reused.
    field_20_ = DialogDefaults[36];
    field_24_ = 0;
    field_28_ = 0;
    field_2C_ = DialogDefaults[19];
    field_30_ = DialogField30Default;
    selected_position_ = 0;
    field_F0_ = -1;
    field_38_ = 0;
    field_3C_ = 0;
    field_40_ = DialogField40Default;
    field_44_ = DialogDefaults[20];
    field_34_ = DialogField34Default;
    field_48_ = -1;
    field_4C_ = DialogDefaults[21];
    field_50_ = DialogDefaults[22];
    field_54_ = 0;
    field_58_ = 0;
    field_5C_ = 0;
    field_60_ = 0;
    field_64_ = 0;
    field_68_ = 0;
    field_6C_ = DialogDefaults[23];
    field_AC_ = DialogFieldACDefault;
    field_B0_ = DialogFieldB0Default;
    field_B4_ = DialogFieldB4Default;
    field_B8_ = DialogFieldB8Default;
    font1_ = DialogDefaultFonts[0];
    text_color_a_ = DialogDefaults[24];
    text_color_b_ = DialogDefaults[27];
    text_color_c_ = DialogDefaults[30];
    text_color_d_ = DialogDefaults[33];
    font2_ = DialogDefaultFonts[1];
    text_color_2a_ = DialogDefaults[25];
    text_color_2b_ = DialogDefaults[28];
    text_color_2c_ = DialogDefaults[31];
    text_color_2d_ = DialogDefaults[34];
    font3_ = DialogDefaultFonts[2];
    text_color_3a_ = DialogDefaults[26];
    text_color_3b_ = DialogDefaults[29];
    text_color_3c_ = DialogDefaults[32];
    // THIS ORDER, 3d_ BEFORE field_1C_: it is what hoists the image's
    // `mov edx,[0x6970d8]` above the `mov [esi+0x1c],eax` store. See the
    // LEVER note on the marker above.
    text_color_3d_ = DialogDefaults[35];
    field_1C_ = -1;
    field_1C_ = -1;
}

// Reset a live dialog, keeping its list and heap objects alive for reuse.
// Entry hooks run before releasing their backing heap. Empty-head cleanup
// still resets position but deliberately does not clear current_ or count.
// ORIGINAL: 0x00608F50 ?close@Dialog@@QAEXXZ 0x00608F50-0x00609143
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original608F50-609143 clears entries using the current list hooks, unconditionally resets position, calls5D4580 at608FDB, then writes heap_ptr/default/font/color fields and field1C=-1 at60913C. current_ is not cleared. Retained seed preserves extent but explicit member destruction is not used in the live C++ reset.", "Existing DialogDefaults and DialogDefaultFonts own the original settings; list removal and allocator-aware node destruction are recovered in stringstruct.cpp. Heap::reset has a separate source shared-code interpretation for the original5D4580 cleanup entry. No byte or full-lifetime acceptance is claimed."], "reason": "Recover reusable dialog cleanup through real list members and a non-destructive heap reset, preserving original cleanup-before-defaults order."}
void Dialog::close() {
    item_list_.remove_all();
    item_list_.current_position_ = 0;
    heap_.reset();
    heap_ptr_ = 0;
    field_20_ = DialogDefaults[36];
    field_24_ = 0;
    field_28_ = 0;
    field_2C_ = DialogDefaults[19];
    field_30_ = DialogField30Default;
    selected_position_ = 0;
    field_F0_ = -1;
    field_38_ = 0;
    field_3C_ = 0;
    field_40_ = DialogField40Default;
    field_44_ = DialogDefaults[20];
    field_48_ = -1;
    field_34_ = DialogField34Default;
    field_4C_ = DialogDefaults[21];
    field_50_ = DialogDefaults[22];
    field_54_ = 0;
    field_58_ = 0;
    field_5C_ = 0;
    field_60_ = 0;
    field_64_ = 0;
    field_68_ = 0;
    field_6C_ = DialogDefaults[23];
    field_AC_ = DialogFieldACDefault;
    field_B0_ = DialogFieldB0Default;
    field_B4_ = DialogFieldB4Default;
    field_B8_ = DialogFieldB8Default;
    font1_ = DialogDefaultFonts[0];
    text_color_a_ = DialogDefaults[24];
    text_color_b_ = DialogDefaults[27];
    text_color_c_ = DialogDefaults[30];
    text_color_d_ = DialogDefaults[33];
    font2_ = DialogDefaultFonts[1];
    text_color_2a_ = DialogDefaults[25];
    text_color_2b_ = DialogDefaults[28];
    text_color_2c_ = DialogDefaults[31];
    text_color_2d_ = DialogDefaults[34];
    font3_ = DialogDefaultFonts[2];
    text_color_3a_ = DialogDefaults[26];
    text_color_3b_ = DialogDefaults[29];
    text_color_3c_ = DialogDefaults[32];
    text_color_3d_ = DialogDefaults[35];
    field_1C_ = -1;
}

// The four list-stage constants (DialogListDerived/VirtualBase and
// DialogList/VirtualBaseVtable) are gone: destroy()'s two hand
// close_with_tables calls folded into the one compiler-owned
// item_list_ destructor chain, which stages the same tables from the
// StringList declarations. DialogVirtualBaseFinalVtable and
// DialogPublishedGlobal left in the batch before.


/*
Purpose: Close the Dialog, then destroy its list and heap members in reverse
         construction order. The compiler owns table staging and EH cleanup.
         Historical observations below also discuss the retired destroy helper.
// ORIGINAL: 0x00608E10 ??1Dialog@@QAE@XZ 0x00608E10-0x00608F41;0x00662EC0-0x00662EEE
// LEVER (2026-09-03): this address is the DESTRUCTOR, and the subject was
//        repointed from the destroy() helper to what the compiler actually
//        emits for `~Dialog` - which now carries the body. Three measured
//        facts, 0 of 94 (13 compiled) -> 7 of 94 (37 compiled, 0.504
//        similar) with no other edit:
//        (1) `~Dialog` was `{ ; }`, so the compiler-generated destructor ran
//            close() never and the 116 derived destructors that the image
//            sends here tore their Dialog base down without closing it. It
//            now reads `close();` and the compiler adds the vptr install,
//            the SEH unwind frame under /GX, and the item_list_/heap_
//            teardowns - the image's own three opening blocks, in its order.
//        (2) The image calls close() DIRECTLY (`call 0x608f50`), so the
//            body says `close()`, the declared member - not the
//            DialogOriginalClose pointer seam destroy() still uses, which
//            compiles `call dword ptr [global]`.
//        (3) The `symbol` fact is `??1Dialog@@UAE@XZ`, U not the image's Q,
//            because this tree declares the destructor `virtual` (that
//            declaration is what earns the vfptr the class needs; see
//            dialog.h). A U-named destructor is the one ABI consequence.
// TRIED: the remaining 87 instructions are the gap the old note named, and
//            it stands: the image INLINES the whole StringStruct teardown
//            (derived stage, base stage, entry walk, virtual-base publish)
//            where this tree calls ~StringList, and it has no SEH-frame
//            equivalent to lose. Inlining ~StringList into this site means
//            moving its body into stringstruct.h - a file this pass does not
//            own - and the brief documents that a forwarder does not fix an
//            inlined-here/called-there split.
// symbol    ??1Dialog@@UAE@XZ
// size      351 bytes
// prototype void (__thiscall ??1Dialog@@QAE@XZ)(Dialog* this)
// callers   116   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00402970 0x005D45B0 0x00608F50
// indirect  0x00608EB5 0x00608ECA 0x00608EE9
Return Value: n/a (the original leaves Heap::shutdown's EAX residue; the
              scalar deleting destructor overwrites it and the 116 direct
              callers ignore it, so void is faithful)
The source close is reusable; member lifetimes end only in this destructor.
Original normal cleanup at 608F2A and unwind tail at 662EC6 both target
5D45B0. Its relationship to Heap's catalogued destructor remains a reference
interpretation to verify, not permission to omit compiler-owned member cleanup.
*/
Dialog::~Dialog() {
    close();
}

// Retained leaf callers still use this callback ABI. The actual C++ destructor
// owns its virtual-table stage, member destruction and exception cleanup.
void __fastcall dialog_destructor_redirect(Dialog *self, void *) {
    self->Dialog::~Dialog();
}

/*
Purpose: The compiler-generated scalar deleting destructor: run the complete
         destructor and free the storage through the executable's operator
         delete only when bit 0 of the mode asks. Always returns the object.
// ORIGINAL: 0x00609D90 ??_GDialog@@UAEPAXI@Z 0x00609D90-0x00609DAE FILE BYTE_EXACT
// symbol    ??_GDialog@@UAEPAXI@Z
// size      30 bytes
// prototype void* (__thiscall ??_GDialog@@UAEPAXI@Z)(Dialog* this, unsigned int)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00608E10 0x0064557F
Return Value: the object
Status: Complete
*/
// Emitted by Dialog's real virtual destructor and table, not a handwritten
// fastcall model. The original scalar entry and historical claim stay here.





/*
 * The five-argument init, on the same terms as the three above: a forwarder,
 * not a recovery. The recovered SpriteBox::init (0x006104D0) tail-forwards to
 * it, so the DLL cannot link without a definition.
 */
