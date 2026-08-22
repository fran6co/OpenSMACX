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
int StringStruct::current_entry() {
    if (head_) {
        return current_->payload;
    }
    return 0;
}

/*
Purpose: Advance the current string-list entry and return its payload.
// ORIGINAL: 0x00402500 ?next_entry@StringStruct@@QAEHXZ 0x00402500-0x0040252F BYTE_EXACT
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
int StringStruct::next_entry() {
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
int StringStruct::seek_id(int a1) {
    if (head_) {
        current_position_ = 0;
        current_ = head_;
        for (int traversed = 0; traversed < entry_count_; traversed++) {
            int *entry = reinterpret_cast<int*>(current_);
            if (entry[1] == a1) {
                return 1;
            }
            current_position_++;
            current_ = reinterpret_cast<StringStructEntry*>(entry[3]);
        }
    }
    return 0;
}

int __fastcall string_struct_current_id_redirect(StringStruct *self, void *) {
    return self->current_id();
}

int __fastcall string_struct_current_entry_redirect(StringStruct *self, void *) {
    return self->current_entry();
}

int __fastcall string_struct_next_entry_redirect(StringStruct *self, void *) {
    return self->next_entry();
}

extern "C" int __cdecl string_struct_seek_id_source(StringStruct *self, int id) {
    return self->seek_id(id);
}

__declspec(naked) int __fastcall string_struct_seek_id_redirect(
        StringStruct *, void *, int) {
    __asm {
        push dword ptr [esp + 4]
        push ecx
        call string_struct_seek_id_source
        add esp, 8
        cmp eax, eax
        ret 4
    }
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

// `inline`: the image reads entry->payload directly with no call at all - a
// pure identity cast, which only disappears when this folds into its caller.
inline void *payload_pointer(int payload) {
    return reinterpret_cast<void *>(
        static_cast<uintptr_t>(static_cast<uint32_t>(payload)));
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
            void *const payload = payload_pointer(entry->payload);
            uint32_t *const vtable = *reinterpret_cast<uint32_t **>(this);
            (ORIGINAL(this)->*original_method<func_entry_visitor>(static_cast<unsigned long>(vtable[1])))(payload);
            if (payload) {
                destroy_virtual_base(payload);
            }
            head_->payload = 0;
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

void __fastcall string_struct_remove_all_redirect(StringStruct *self, void *) {
    self->remove_all();
}

const uint32_t StringStructVtable = 0x006693A4;
const uint32_t StringStructVirtualBaseVtable = 0x006693A0;

// The shared body of every virtual-base close: installs the pair of virtual
// tables, releases the entries, then clears the position. `inline` because
// 0x004066C0 writes both of its stages out in place and makes no real call.
inline void StringStruct::close_with_tables(uint32_t primary, uint32_t virtual_base) {
    uint8_t *const base = reinterpret_cast<uint8_t *>(this);
    *reinterpret_cast<volatile uint32_t *>(base) = primary;
    // The virtual base's table is reached through the displacement held in the
    // second slot of the vbtable pointed at by offset 4.
    const uint32_t *const vbtable =
        *reinterpret_cast<uint32_t **>(base + 4);
    const uint32_t displacement = vbtable[1];
    *reinterpret_cast<volatile uint32_t *>(base + 4 + displacement) =
        virtual_base;
    // The legacy bodies inline the entry walk; it clears the same fields in
    // the same order, and does nothing at all when the list is already empty.
    // The sweep's surviving swap of the walk against the position reset is
    // observable only to a visitor reading current_position_ mid-walk, and no
    // leaf fixture can walk through close: the real (unmapped) tables are
    // installed first, so the walk tier belongs to the in-process oracle.
    remove_all();
    current_position_ = 0;
}

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
    // ENTERED ON THE VIRTUAL BASE, NOT ON THE OBJECT. The first instruction
    // after the prologue is `lea esi, [ecx - 0x1c]` at 0x00401066, and every
    // field it then touches is [esi + N] - the list head at +8, the count at
    // +0x10, the position at +0x14 - so ECX arrives pointing at the two-word
    // virtual base this class holds at 0x1C, exactly as the adjustor-entered
    // destructors in guarded_teardowns.cpp do. The adjustment therefore
    // belongs HERE and not in the redirect: with the redirect subtracting it
    // instead, the compiled body starts `mov esi, ecx` and diverges at
    // instruction 0.
    uint8_t *const vbase = reinterpret_cast<uint8_t *>(this);
    StringStruct *const self = reinterpret_cast<StringStruct *>(
        vbase - StringStructCloseAdjustment);
    // The two table stores are written out here rather than delegated to
    // close_with_tables because the image reaches the vbtable through the
    // INCOMING pointer - `mov eax, [ecx - 0x18]`, `mov [edx + ecx - 0x18],
    // 0x6693a0` - not through the recovered object base. close_with_tables is
    // entered on the object at its other two call sites and cannot say both.
    // NOT `volatile`, unlike close_with_tables' pair: the image hoists its
    // zero register (`xor ebx, ebx` at 0x00401069) BETWEEN the `lea` and the
    // first table store, and a volatile store pins the schedule so the xor
    // lands six instructions late. 50 of 64 with the qualifier, 56 without.
    // Both stores survive - the walk that follows calls through a vtable VC6
    // cannot see through.
    *reinterpret_cast<uint32_t *>(vbase - StringStructCloseAdjustment) =
        StringStructVtable;
    const uint32_t *const vbtable =
        *reinterpret_cast<uint32_t **>(vbase - StringStructCloseAdjustment + 4);
    const uint32_t displacement = vbtable[1];
    *reinterpret_cast<uint32_t *>(
        vbase - StringStructCloseAdjustment + 4 + displacement) =
        StringStructVirtualBaseVtable;
    self->remove_all();
    self->current_position_ = 0;
}

// The redirect hands the adjusted pointer straight through - close() itself
// performs the 0x1C, as the image does.
void __fastcall string_struct_close_redirect(void *adjusted, void *) {
    reinterpret_cast<StringStruct *>(adjusted)->close();
}

const uint32_t StringStructDerivedVtable = 0x006698C4;
const uint32_t StringStructDerivedVirtualBaseVtable = 0x006698C0;

/*
Purpose: Close a derived string list, releasing its entries under its own
         virtual tables before closing its StringStruct base the same way.
// ORIGINAL: 0x004066C0 sub_4066c0 0x004066C0-0x00406818;0x00650980-0x00650995
// symbol    ?string_struct_derived_close_redirect@@YIXPAX@Z
// size      365 bytes
// prototype 
// callers   61   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00406721 0x00406735 0x00406754 0x004067B7 0x004067C9 0x004067E9
// notes     Runtime redirect installed by DllMain after byte-signature validation
// LEVER: 2 of 126 -> 6 of 126, 112 compiled instructions -> 114, on the same
//        two fixes StringStruct::close records: the second stage is
//        close_with_tables written out (0x00406778 recomputes the object with
//        the SAME `lea esi, [ebx - 0x28]` the first stage used, so close()'s
//        own 0x1C entry adjustment has no part in it), and remove_all's
//        `int index = 0` moved out of the count test.
// TRIED: the ceiling is THE EH FRAME, and it is worth 14 instructions
//            before anything else can line up. The image opens `push -1; push
//            0x65098b; mov eax, fs:[0]; push eax; mov fs:[0], esp` - the
//            second span, 0x00650980-0x00650995, is that handler's scope
//            table - and it moves an unwind state through [ebp - 4]: 0 at
//            0x00406701, just before the DERIVED walk, and -1 at 0x0040677B,
//            just before the base stage. It also parks the adjusted receiver
//            at [ebp - 0x14] for the unwind funclet. VC6 emits none of that
//            for a plain function: under /GX the frame appears only when
//            something in scope has to be unwound, which means this body is a
//            COMPILER-GENERATED teardown of a class whose base still needs
//            destroying while the derived stage runs - the same shape
//            ??1Net@@QAE@XZ documents at 0x004E34D0. No spelling of a free
//            `__fastcall` redirect can produce it, and `/GX-` is ruled out
//            tree-wide in AGENT_BRIEF.md.
Status: Complete
Verification note: the base stage overwrites the derived tables, so with the
non-walking fixtures the oracle can safely drive, the derived stage leaves no
observable trace and dropping it still compares equal. The derived table
addresses and the 0x28 adjustor were instead confirmed by reading the
instruction bytes directly (`mov [ebx-0x28], 0x6698C4` and
`mov [ecx+ebx-0x24], 0x6698C0`).
*/
void __fastcall string_struct_derived_close_redirect(void *adjusted) {
    StringStruct *self = reinterpret_cast<StringStruct *>(
        static_cast<uint8_t *>(adjusted) - StringStructDerivedCloseAdjustment);
    self->close_with_tables(
        StringStructDerivedVtable, StringStructDerivedVirtualBaseVtable);
    // The second stage is close()'s body written out, NOT a call to close():
    // 0x00406778 recomputes the object with the SAME `lea esi, [ebx - 0x28]`
    // the first stage used, so the 0x1C adjustment close() performs on its own
    // entry has no part in it. src/dialog.cpp stages its embedded list the
    // same way.
    self->close_with_tables(
        StringStructVtable, StringStructVirtualBaseVtable);
}

const uint32_t StringVirtualBaseVtable = 0x006693AC;
// AN OBJECT, NOT A POINTER TO A FIXED ADDRESS: the pointer form costs a
// load at every use where the image addresses the storage directly, and
// the address is terranx.exe's data, unmapped in a standalone build.
uint32_t StringVirtualBaseOwner;  // 0x009B3374

/*
Purpose: Destroy a most-derived StringList: run the source-owned two-stage
         derived close, then hand the virtual base back its own vtable and
         republish the pending-allocation owner the constructor captured.
// ORIGINAL: 0x00406820 sub_406820 0x00406820-0x0040683B BYTE_EXACT
// LEVER: THE SECOND PARAMETER WAS INVENTED, so it is gone rather than worked
//   around. 0x00406820 sets only ECX (`mov ecx, esi; call 0x4066c0`) and never
//   EDX, and 0x004066C0's body reads only ECX - the `_redirect` name is this
//   tree's own, so nothing about the image required the arity. Declaring ONE
//   parameter takes this call site to BYTE_EXACT 9/9.
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
              StringVirtualBaseOwner. The original is a void destructor;
              modelled as uint32_t to preserve the residue, as
              GraphicWin::close and Scroll::destroy do.
Status: Complete
Verification note: the delegated close installs the real 0x006698C4 /
0x006693A4 table addresses into [this] before remove_all dispatches through
vtable[1], so the entry walk cannot be driven from the leaf suite - those
addresses are unmapped outside the game process. The sweep's three surviving
constant mutants rewrite hex values that occur only in the margin comments
naming each instruction; the code reads the named constants, so the mutants
are byte-identical and equivalent by construction. The leaf test drives only
non-walking list shapes, which reach every effect of this function; the walk
belongs to 0x004066C0 and is covered in-process by the stringstruct
runtime-oracle suite.
*/
uint32_t StringList::destroy() {
    uint8_t *const base = reinterpret_cast<uint8_t *>(this);
    // `lea esi, [ecx + 0x28]` / `mov ecx, esi` / `call 0x004066C0`. The
    // source-owned derived close is entered on the virtual base and recovers
    // the object by subtracting the same 0x28, which is why the raw pointer
    // is handed to the recovered entry point rather than cast to StringStruct.
    uint8_t *const virtual_base = base + StringListVirtualBaseOffset;
    string_struct_derived_close_redirect(virtual_base);

    // The three tail operations, in the original's order. The read of
    // [esi + 4] precedes the store to [esi]; both are volatile so an
    // optimised build keeps the legacy access order.
    volatile uint32_t *const virtual_base_slots =
        reinterpret_cast<volatile uint32_t *>(virtual_base);
    const uint32_t owner = virtual_base_slots[1];    // mov eax, [esi + 4]
    virtual_base_slots[0] = StringVirtualBaseVtable; // mov [esi], 0x006693AC
    StringVirtualBaseOwner = owner;                 // mov [0x9B3374], eax
    return owner;                                    // EAX at the ret
}

// self == the StringList base: `ret` pops nothing (0xC3), `this` arrives in
// ECX unadjusted, and there are no stack arguments, so the fastcall adapter
// is a straight delegation with no this-adjustment.
uint32_t __fastcall string_list_destructor_redirect(StringList *self, void *) {
    return self->destroy();
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
