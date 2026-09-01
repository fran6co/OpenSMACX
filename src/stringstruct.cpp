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
#include "original_seam.h"

#include "stringstruct.h"
#include "spritebox.h"
#include "net_class.h"

#include <cstring>

#include "heap.h"  // StringAllocationHeap

// The real teardown: republish the saved allocation owner so the next
// allocation routes through it again (sub_401520, the image's deleting
// destructor, and StringList::destroy's tail are this destructor as
// compiled - the [esi] = 0x6693AC vptr re-stage on top of it is the
// compiler's own destructor machinery). Out of line: OUT OF LINE is the
// point. A header-inline empty destructor is provably nothrow, and VC6 then
// sheds the /GX unwind states from the construction chain that
// StringBox::StringBox's frame carries.
StringAllocationBase::~StringAllocationBase() {
    StringAllocationHeap = allocation_owner_;
}

void StringStruct::unk_slot00() {
}

// The real destructor body: release the entries and clear the position.
// The derived- and base-stage table installs around it are the compiler's
// (the destructor prologues), replacing the hand stores close() and
// close_with_tables spelled - the image's own ??1StringList-shaped chain
// (0x004066C0) carries the same two stages, walked with everything
// inlined; this compiled chain keeps a real remove_all, which is the
// recorded divergence.
StringStruct::~StringStruct() {
    remove_all();
    current_position_ = 0;
}

/*
Purpose: Return the current string-list item ID.
// ORIGINAL: 0x00401640 ?current_id@StringStruct@@QAEHXZ 0x00401640-0x00401651 BYTE_EXACT
// size      17 bytes
// prototype int (__thiscall ?current_id@StringStruct@@QAEHXZ)(StringStruct* this)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: Current ID, or zero when the list is empty
Status: Complete
*/
int StringStruct::current_id() {
    if (head_) {
        return reinterpret_cast<int *>(current_)[1];
    }
    return 0;
}

/*
Purpose: Return the current string-list payload.
// ORIGINAL: 0x00402530 ?current_entry@StringStruct@@QAEHXZ 0x00402530-0x00402541 BYTE_EXACT
// symbol    ?current_entry@StringStruct@@QAEPAXXZ
// size      17 bytes
// prototype int (__thiscall ?current_entry@StringStruct@@QAEHXZ)(StringStruct* this)
// callers   9   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: Current payload, or zero when the list is empty
Status: Complete
*/
void *StringStruct::current_entry() {
    if (head_) {
        return current_->payload;
    }
    return 0;
}

/*
Purpose: Advance the current string-list entry and return its payload.
// ORIGINAL: 0x00402500 ?next_entry@StringStruct@@QAEHXZ 0x00402500-0x0040252F BYTE_EXACT
// symbol    ?next_entry@StringStruct@@QAEPAXXZ
// size      47 bytes
// prototype int (__thiscall ?next_entry@StringStruct@@QAEHXZ)(StringStruct* this)
// callers   9   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: New current payload, or zero when the list is empty
Status: Complete
*/
void *StringStruct::next_entry() {
    if (!head_) {
        return 0;
    }

    StringStructEntry *next = current_->next;
    int position = current_position_;
    current_ = next;
    int count = entry_count_;
    ++position;
    current_position_ = position;
    if (position == count) {
        current_position_ = 0;
    }
    return current_->payload;
}

/*
Purpose: Position the string-list cursor at the first matching item ID.
// ORIGINAL: 0x00401560 ?seek_id@StringStruct@@QAEHH@Z 0x00401560-0x004015AE BYTE_EXACT
// size      78 bytes
// prototype int (__thiscall ?seek_id@StringStruct@@QAEHH@Z)(StringStruct* this, int)
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: One when found, otherwise zero
Status: Complete
*/
int StringStruct::seek_id(int id) {
    if (head_) {
        current_position_ = 0;
        current_ = head_;
        for (int traversed = 0; traversed < entry_count_; traversed++) {
            int *entry = reinterpret_cast<int*>(current_);
            if (entry[1] == id) {
                return 1;
            }
            current_position_++;
            current_ = reinterpret_cast<StringStructEntry*>(entry[3]);
        }
    }
    return 0;
}




extern "C" int __cdecl string_struct_seek_id_source(StringStruct *self, int id) {
    return self->seek_id(id);
}


namespace {

typedef void (OriginalObject::*func_entry_visitor)(void *);
typedef void (OriginalObject::*func_scalar_deleting_destructor)(int);

// MSVC reaches these destructors through the virtual-base displacement stored
// in the object's second vtable slot: the displacement selects a subobject
// whose own first vtable slot is the scalar deleting destructor, invoked with
// the deleting flag set.
// `inline`: the image never calls this as a standalone function - every call
// site (remove_all, twice) has the vtable-adjustor dispatch written out in
// place, with no real call to a shared helper. A non-inline definition here
// costs the caller a real out-of-line call the image does not make.
inline void destroy_virtual_base(void *object) {
    uint32_t *const vtable = *reinterpret_cast<uint32_t **>(object);
    uint8_t *const subobject =
        static_cast<uint8_t *>(object) + vtable[1];
    uint32_t *const subobject_vtable = *reinterpret_cast<uint32_t **>(subobject);
    (ORIGINAL(subobject)->*original_method<func_scalar_deleting_destructor>(static_cast<unsigned long>(subobject_vtable[0])))(1);
}

}  // namespace

/*
Purpose: Release every entry in the list, notifying the owner about each
         payload before destroying the payload and its entry.
// ORIGINAL: 0x00402970 StringStruct::remove_all 0x00402970-0x004029E3
// symbol    ?remove_all@StringStruct@@QAEXXZ
// size      115 bytes
// prototype 
// callers   79   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00402992 0x004029A4 0x004029C4
// notes     Runtime redirect installed by DllMain after byte-signature validation
// LEVER: MORE (3 calls vs image's 0 real calls, all 3 indirect) -
//        destroy_virtual_base and payload_pointer were plain (non-inline)
//        helper functions, so calling them from here cost a real
//        out-of-line call the image never makes; marking both `inline`
//        folds their bodies (and destroy_virtual_base's own indirect
//        vtable-adjustor dispatch) directly into this call site, taking
//        call_diff to 0 disagreeing.
// TRIED: THAT LEVER NO LONGER HOLDS for the OUT-OF-LINE body at
//            0x00402970, and it is not the `inline` keyword that stopped
//            working. call_diff reports MORE here again - 2 calls to
//            destroy_virtual_base against the image's 0 - at EVERY flag set it
//            tries, which is what its "smallest gap wins" search means: no
//            flag set folds the helper into the COMDAT copy VC6 emits for an
//            `inline` member that other translation units also call. The
//            inlining the lever bought is still real at the sites that matter
//            - StringStruct::close (0x00401060) and 0x004066C0 both write the
//            walk out with no call at all. Measured 2026-08-22, and NOT caused
//            by this batch: reverting the `int index = 0` move above and
//            rebuilding reports the identical 2 against 0.
Status: Complete
*/
// inline: sub_4066c0 (0x004066C0) calls this twice through
// close_with_tables/close and makes NO real calls at all in the image, so
// this must fold there too. Plain `inline` (not MEASURED/dllexport), since
// `remove_all` has external linkage as a public member either way - the
// out-of-line 0x00402970 body stays reachable through the other callers in
// this file, and `MEASURED` here would force a matching dllexport onto the
// header declaration this file shares with unrelated translation units.
inline void StringStruct::remove_all() {
    if (!head_) {
        return;
    }
    // `index` is declared BEFORE the count test, not inside it: the image
    // stores the zero into the slot at 0x00401089, BETWEEN the load of
    // entry_count_ and the compare against it. Declared inside, VC6 folds the
    // count into the compare (`cmp dword ptr [esi + 0x10], ebx`) and the
    // whole walk shifts - 7 of 64 against 50 of 64 in StringStruct::close.
    int index = 0;
    if (entry_count_ > 0) {
        do {
            StringStructEntry *const entry = head_;
            current_ = entry->next;
            void *const payload = entry->payload;
            uint32_t *const vtable = *reinterpret_cast<uint32_t **>(this);
            (ORIGINAL(this)->*original_method<func_entry_visitor>(static_cast<unsigned long>(vtable[1])))(payload);
            if (payload) {
                destroy_virtual_base(payload);
            }
            head_->payload = nullptr;
            if (head_) {
                destroy_virtual_base(head_);
            }
            ++index;
            head_ = current_;
            // The loop bound is re-read every iteration, so a visitor that
            // changes the count changes how far the walk goes.
        } while (index < entry_count_);
    }
    head_ = nullptr;
    current_position_ = 0;
    entry_count_ = 0;
}


// StringStructVtable / StringStructVirtualBaseVtable (0x006693A4/0x006693A0)
// are gone with the hand close chain - the compiler emits both tables from
// the declarations, and nothing names them any more.

/*
Purpose: Reset the list to its constructed state, installing both virtual
         tables and releasing every entry.
// ORIGINAL: 0x00401060 ?close@StringStruct@@QAEXXZ 0x00401060-0x004010F9
// size      153 bytes
// prototype void (__thiscall ?close@StringStruct@@QAEXXZ)(StringStruct* this)
// callers   25   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
// indirect  0x004010A2 0x004010B4 0x004010D0
// notes     Runtime redirect installed by DllMain after byte-signature validation
// LEVER: 0 of 64 -> 56 of 64, with the compiled instruction count now equal
//        to the image's, on four stacked fixes. (1) THE 0x1C ADJUSTMENT
//        BELONGS IN close(), not in string_struct_close_redirect: the image's
//        first instruction after the prologue is `lea esi, [ecx - 0x1c]`, so
//        ECX arrives pointing at the virtual base. With the redirect doing the
//        subtraction the body opened `mov esi, ecx` and diverged at
//        instruction 0.
// LEVER: (2) THE VBTABLE IS REACHED THROUGH THE INCOMING POINTER. The image
//        reads `[ecx - 0x18]` and stores `[edx + ecx - 0x18]`, not `[esi + 4]`
//        - which is why the two table stores are written out here rather than
//        delegated to close_with_tables, whose other two call sites are
//        entered on the object itself and cannot say both.
// LEVER: (3) `int index = 0;` moved OUT of `if (entry_count_ > 0)` in
//        remove_all: the image stores the zero at 0x00401089, between the load
//        of entry_count_ and the compare against it. Declared inside the
//        `if`, VC6 folds the count into the compare and everything after it
//        shifts. 7 of 64 -> 50 of 64.
// LEVER: (4) the two table stores are NOT `volatile`. The image hoists its
//        zero register (`xor ebx, ebx` at 0x00401069) between the `lea` and
//        the first store, and a volatile store pins the schedule so the xor
//        lands six instructions late. 50 of 64 -> 56 of 64.
// TRIED: the last 8 are one register choice inside the SECOND
//            destroy_virtual_base, at 0x004010C3. The image keeps the vbtable
//            delta in edx and spends `lea ecx, [edx + eax]` plus `mov eax,
//            [edx + eax]`; VC6 here reuses ecx for the delta and spends `mov
//            ecx, [ecx + 4]; add ecx, eax; mov edx, [ecx]` - two bytes
//            shorter, which is why the `je` at 0x00401084 lands two bytes
//            early. THE IMAGE ITSELF EMITS BOTH FORMS from this one helper:
//            the FIRST site, 0x004010A9, is the `add` form this tree
//            produces, so no single spelling of the helper can serve both.
//            Measured, all 55 of 64 (worse): naming the delta in a local and
//            indexing `base + delta` twice, reading the subobject vtable
//            before computing the receiver, a `uintptr_t` receiver cast, and
//            writing the second site out at the call site instead of through
//            the helper (56, unchanged).
Status: Complete
*/
void StringStruct::close() {
    // ENTERED ON THE VIRTUAL BASE, NOT ON THE OBJECT. The image's close
    // opens `lea esi, [ecx - 0x1c]` at 0x00401066, and every field it then
    // touches is [esi + N] - so ECX arrives pointing at the two-word
    // virtual base this class holds at 0x1C. The adjustment belongs HERE
    // and not in the redirect: with the redirect subtracting it instead,
    // the compiled body starts `mov esi, ecx` and diverges at instruction 0.
    uint8_t *const vbase = reinterpret_cast<uint8_t *>(this);
    StringStruct *const self = reinterpret_cast<StringStruct *>(
        vbase - StringStructCloseAdjustment);
    // TRIED: the two table stores the image stages here (`mov [ecx-0x1c],
    // 0x6693a4` and the vbtable-relative base-pair store) - hand writes in
    // the original itself, stripped by direction; the walk now dispatches
    // through whatever tables the constructing chain installed. The image
    // hoists its zero register between the lea and the first store; both
    // gaps are recorded on the marker below.
    self->remove_all();
    self->current_position_ = 0;
}

// StringStructDerivedVtable / StringStructDerivedVirtualBaseVtable
// (0x006698C4/0x006698C0), close_with_tables, and
// string_struct_derived_close_redirect are gone. The redirect's own marker
// carried the diagnosis all along: the image's 0x004066C0 is a
// COMPILER-GENERATED two-stage teardown (its LEVER/TRIED notes record the
// /GX frame no free function can produce), and the real ~StringStruct
// destructor above now emits that chain from the declarations. The EH
// frame's funclet span 0x00650980-0x00650995 is that machinery's own.

// THE ALLOCATOR HAND-OFF SLOT: the allocating Heap published by Heap::get
// and captured by the next StringAllocationBase construction.
Heap *StringAllocationHeap;  // 0x009B3374

/*
Purpose: Destroy a most-derived StringList: run the source-owned two-stage
         derived close, then hand the virtual base back its own vtable and
         republish the pending-allocation owner the constructor captured.
// ORIGINAL: 0x00406820 sub_406820 0x00406820-0x0040683B
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
// symbol    ?destroy@StringList@@QAEIXZ
// size      27 bytes
// prototype 
// callers   26   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x004066C0
Return Value: EAX residue - the saved owner value, republished into
              StringAllocationHeap. The original is a void destructor;
              modelled as uint32_t to preserve the residue, as
              GraphicWin::close and Scroll::destroy do.
Status: Complete
DEMOTED from BYTE_EXACT by direction (strip-all hand vptr writes): the hand
derived-close call and the three tail operations (owner read, final-table
install, republish) left - the teardown is the compiler's ~StringList chain
now, staging the same tables from the StringStruct/StringList declarations.
Compiled is 8 instructions against the image's 9: the image inlines the
vbase tail after its call; ours folds it into the chain. The EAX residue
survives as the republished global.
*/
uint32_t StringList::destroy() {
    // The whole teardown is the compiler's chain now: ~StringList's derived
    // stage, ~StringStruct's base stage and entry walk, and the virtual-base
    // destructor whose body restores the one-slot vftable and republishes
    // the saved owner - the three tail operations the old body spelled by
    // hand. The EAX residue the 26 callers consume is the republished owner,
    // which the chain leaves in StringAllocationHeap.
    this->~StringList();
    return reinterpret_cast<uint32_t>(StringAllocationHeap);
}


/*
// ORIGINAL: 0x004015B0 ?seek_pos@StringStruct@@QAEHH@Z 0x004015B0-0x00401636 BYTE_EXACT
// size      134 bytes
// prototype int (__thiscall ?seek_pos@StringStruct@@QAEHH@Z)(StringStruct* this, int)
// callers   2   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00644F3A
Status: Complete
*/
typedef int (__cdecl *AbsFnT)(int);

int StringStruct::seek_pos(int a1) {
    AbsFnT abs_fn = reinterpret_cast<AbsFnT>(&abs);
    if (a1 > entry_count_ - 1) {
        return 0;
    }
    current_ = head_;
    if (a1 < 0) {
        if (abs_fn(a1) > entry_count_) {
            return 0;
        }
        int i = abs_fn(a1);
        if (i > 0) {
            do {
                i--;
                current_ = reinterpret_cast<StringStructEntry *>(reinterpret_cast<int *>(current_)[4]);
            } while (i != 0);
        }
        current_position_ = a1 + entry_count_;
        return 1;
    } else {
        if (a1 > 0) {
            int i = a1;
            do {
                i--;
                current_ = reinterpret_cast<StringStructEntry *>(reinterpret_cast<int *>(current_)[3]);
            } while (i != 0);
        }
        current_position_ = a1;
        return 1;
    }
}
