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

// ORIGINAL: 0x00402C20 ??_GStringStructEntry@@UAEPAXI@Z 0x00402C20-0x00402C62 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Original table installed by 401100 construction dispatches deletion through StringStructEntry allocation base at +0x14. This original body publishes saved Heap owner, conditionally frees the complete allocation and clears owner on deleting paths. Same-span seed remains.", "Original construction at 401100 and typed deletion at 402970 establish the owning node lifecycle. The canonical source host is stringstruct.cpp; the compiler owns the virtual-base adjustment and deleting-flag entry."], "reason": "Map the compiler-generated scalar deleting destructor emitted by actual list-node construction, not a handwritten wrapper."}
// ORIGINAL: 0x00402C70 ??_GStringStructPayload@@UAEPAXI@Z 0x00402C70-0x00402CB2 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Original table installed by 401100 construction dispatches deletion through StringStructPayload allocation base at +0x10. This original body publishes saved Heap owner, conditionally frees the complete allocation and clears owner on deleting paths. Same-span seed remains.", "Original construction at 401100 and typed deletion at 402970 establish the owning node lifecycle. The canonical source host is stringstruct.cpp; the compiler owns the virtual-base adjustment and deleting-flag entry."], "reason": "Map the compiler-generated scalar deleting destructor emitted by actual list-node construction, not a handwritten wrapper."}
#include "stringstruct.h"
#include "spritebox.h"
#include "net_class.h"

#include <cstring>
#include <new>

#include "heap.h"  // StringAllocationHeap
#include "general.h"  // mem_get

// The allocation-base table at 0x6693AC dispatches this scalar deleting
// entry. The compiler supplies its flag handling and class-specific delete;
// the original identity is retained from the retired handwritten wrapper.
/*
// ORIGINAL: 0x00401520 sub_401520 0x00401520-0x0040155E FILE
// symbol    ??_GStringAllocationBase@@UAEPAXI@Z
// size      62 bytes
// prototype
// callers   0   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00644EF2
*/

// Republish the saved owner for the deallocation decision (0x401520, the
// image's deleting destructor, and StringList's complete tail are this destructor as
// compiled - the [esi] = 0x6693AC vptr re-stage on top of it is the
// compiler's own destructor machinery). Out of line: OUT OF LINE is the
// point. A header-inline empty destructor is provably nothrow, and VC6 then
// sheds the /GX unwind states from the construction chain that
// StringBox::StringBox's frame carries.
StringAllocationBase::~StringAllocationBase() {
    StringAllocationHeap = allocation_owner_;
}

namespace {
// The cleanup path at 0x00608783 tests the captured text pointer again before
// calling free. This is a local null-safe release idiom, not an original symbol.
inline void release_text(char *text) {
    if (text != 0) {
        free(text);
    }
}
}

// The original primary table uses these shared defaults. Hook names describe
// their list role; the catalogue's unrelated class names are retained as seeds.
// RECOVERY_SHARED: v1 0x00404250 0x00404255 ?on_insert@StringStruct@@UAEHPAVStringStructPayload@@@Z 0x006086F0
int StringStruct::on_insert(StringStructPayload *) { return 0; }
// RECOVERY_SHARED: v1 0x00404270 0x00404273 ?on_remove@StringStruct@@UAEXPAVStringStructPayload@@@Z 0x006086F0
void StringStruct::on_remove(StringStructPayload *) { }

// ORIGINAL: 0x006086F0 ?on_insert@StringList@@UAEHPAVStringStructPayload@@@Z 0x006086F0-0x00608761 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Primary table 0x6698C4 slot0 targets this body. Original StringStruct::add at 0x4012ED/0x4014F7 passes its 0x18 allocation as the one hook argument; this body writes text at +4 and metadata at +8/+C. The retained seed and its extent remain unchanged.", "Original 0x6086FD bypasses the text store when the source is null; preserve that behavior without assuming zeroed allocation. Receiver +0x18 is passed to Heap::get. No byte claim is banked by this binding."], "reason": "Recover the StringList insertion virtual using its allocated payload type; on_insert is a descriptive name, not a recovered original spelling."}
int StringList::on_insert(StringStructPayload *payload) {
    if (pending_text_ != 0) {
        char *copy;
        if (allocator_ != 0) {
            copy = static_cast<char *>(allocator_->get(strlen(pending_text_) + 1));
        } else {
            copy = static_cast<char *>(mem_get(strlen(pending_text_) + 1));
        }
        payload->text = copy;
        if (copy == 0) {
            return 4;
        }
        *payload->text = 0;
        strcat(payload->text, pending_text_);
    }
    payload->field_8_ = field_20_;
    payload->field_C_ = field_24_;
    return 0;
}

// ORIGINAL: 0x00608770 ?on_remove@StringList@@UAEXPAVStringStructPayload@@@Z 0x00608770-0x0060879B BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Primary table 0x6698C4 slot1 targets this body; original 0x608775 loads payload text at +4, then 0x60877C checks the list allocator at +0x18. Only a null allocator permits free and clearing text. on_remove is descriptive; the kill_entry seed and its historical claim remain intact."], "reason": "Recover allocator-aware StringList text cleanup as a real virtual method; the payload node lifetime remains owned by enclosing list teardown."}
void StringList::on_remove(StringStructPayload *payload) {
    if (payload->text != 0 && allocator_ == 0) {
        release_text(payload->text);
        payload->text = 0;
    }
}

namespace {
// 0x00401100 selects the list's allocator before its inherited allocation
// owner. Heap exhaustion is fatal; CRT allocation failure returns a null node.
// Construction captures the handoff in the real virtual base and emits the
// node's tables/destructor naturally. This helper has no original identity.
template<class T> inline T *allocate_list_node(StringStruct *list, T *) {
    Heap *owner = list->allocator_;
    if (owner == 0) {
        owner = list->allocation_owner_;
    }
    void *memory;
    if (owner != 0) {
        memory = owner->get(sizeof(T));
        if (memory == 0) {
            MessageBoxA(NULL, "out of heap space in new function", "Aborting", MB_OK);
            exit(3);
        }
    } else {
        memory = mem_get(sizeof(T));
        if (memory == 0) {
            return 0;
        }
    }
    StringAllocationHeap = owner;
    return ::new(memory) T;
}
}

// Append before the circular head. Preserve the original partial mutations on
// allocation failure and ignore the insertion hook's return value: neither
// rollback nor propagating that value is part of 0x00401100's behavior.
// ORIGINAL: 0x00401100 ?add@StringStruct@@QAEHH@Z 0x00401100-0x00401513
// RECOVERY_BINDING: v1 {"evidence": ["Original 401100 selects explicit allocator then inherited allocation owner then mem_get, constructs entry size1C and payload size18 with distinct virtual-base tables. Native construction replaces seed vptr writes; source class names remain interpretations.", "Original 4013B1 publishes the entry allocation into the old tail next link before the null test; 4012D2/4014DC publish null payloads before returning4. Insertion hook result is ignored; count and position update only after it returns. Same-span legacy seed retained."], "reason": "Recover circular-list insertion with real node construction and the original partial updates on allocation failure."}
int StringStruct::add(int id) {
    if (entry_count_ == 0) {
        head_ = allocate_list_node(this, static_cast<StringStructEntry *>(0));
        if (head_ == 0) {
            return 4;
        }
        head_->id = id;
        head_->next = head_;
        head_->previous = head_;
        head_->payload = allocate_list_node(this, static_cast<StringStructPayload *>(0));
        if (head_->payload == 0) {
            return 4;
        }
        on_insert(head_->payload);
        current_ = head_;
    } else {
        head_->previous->next = allocate_list_node(this, static_cast<StringStructEntry *>(0));
        if (head_->previous->next == 0) {
            return 4;
        }
        head_->previous->next->previous = head_->previous;
        head_->previous->next->next = head_;
        head_->previous = head_->previous->next;
        current_ = head_->previous;
        current_->id = id;
        current_->payload = allocate_list_node(this, static_cast<StringStructPayload *>(0));
        if (current_->payload == 0) {
            return 4;
        }
        on_insert(current_->payload);
    }
    current_position_ = entry_count_;
    ++entry_count_;
    return 0;
}

namespace {
// Share the original entry walk without making the public remove_all symbol
// inline-only. Destructors select their own stage's hooks before this walk;
// each still clears its position even when the empty-head guard returns here.
inline void clear_list_entries(StringStruct *self) {
    if (!self->head_) {
        return;
    }
    // The original initializes its index before testing the entry count.
    int index = 0;
    if (self->entry_count_ > 0) {
        do {
            StringStructEntry *const entry = self->head_;
            self->current_ = entry->next;
            StringStructPayload *const payload = entry->payload;
            self->on_remove(payload);
            delete payload;
            self->head_->payload = nullptr;
            delete self->head_;
            ++index;
            self->head_ = self->current_;
            // The loop bound is re-read every iteration, so a visitor that
            // changes the count changes how far the walk goes.
        } while (index < self->entry_count_);
    }
    self->head_ = nullptr;
    self->current_position_ = 0;
    self->entry_count_ = 0;
}

inline void finish_list_teardown(StringStruct *self) {
    clear_list_entries(self);
    self->current_position_ = 0;
}
}

// Derived cleanup must run before the base destructor changes virtual dispatch:
// 0x004066C0 first selects StringList's hooks, then enters the StringStruct stage.
// ORIGINAL: 0x004066C0 ??1StringList@@UAE@XZ 0x004066C0-0x00406818;0x00650980-0x00650995
// RECOVERY_BINDING: v1 {"evidence": ["Original 4066C0 installs derived tables6698C4/6698C0 and clears entries before switching to base tables6693A4/6693A0; primary removal slot selects the text-cleanup hook during the derived stage. Implicit base-only cleanup cannot preserve that lifecycle.", "Original bounds4066C0-406818 and650980-650995 retained in 7eb1b2b0 parent src/stringstruct.cpp. Support code650980 adjusts the saved virtual-base receiver by -0xC before jumping to StringStruct base teardown401060. These are original identity/ownership interpretations, not a matching claim."], "reason": "Restore the original two-stage StringList destructor identity to its real C++ destructor, including its historical EH support span."}
StringList::~StringList() {
    finish_list_teardown(this);
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
// symbol    ?current_entry@StringStruct@@QAEPAVStringStructPayload@@XZ
//   the image's own symbol spells the return PAX (void *); this tree types
//   it StringStructPayload * for the string at +4, and the fact above is
//   what our object emits for it now.
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
StringStructPayload *StringStruct::current_entry() {
    if (head_) {
        return current_->payload;
    }
    return 0;
}

/*
Purpose: Advance the current string-list entry and return its payload.
// ORIGINAL: 0x00402500 ?next_entry@StringStruct@@QAEHXZ 0x00402500-0x0040252F BYTE_EXACT
// symbol    ?next_entry@StringStruct@@QAEPAVStringStructPayload@@XZ
//   as current_entry above: typed for the string, resolved through this
//   emitted name.
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

// MEASURED inline, defined at the end of stringstruct.h: the image both
// calls this (9 callers) and inlines it whole (BasePop::exec 0x0060305C,
// Caviar::init_class). The definition moved; the claim is unchanged.

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


/*
Purpose: Release every entry in the list, notifying the owner about each
         payload before destroying the payload and its entry.
// ORIGINAL: 0x00402970 StringStruct::remove_all 0x00402970-0x004029E3 BYTE_EXACT
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
// The definition is private to this translation unit, while BasePop and
// Datalink call the public declaration. It must provide an out-of-line symbol;
// an inline definition here is not available to those translation units.
void StringStruct::remove_all() {
    clear_list_entries(this);
}


// StringStructVtable / StringStructVirtualBaseVtable (0x006693A4/0x006693A0)
// are gone with the hand close chain - the compiler emits both tables from
// the declarations, and nothing names them any more.

/*
Purpose: Destroy the StringStruct subobject, releasing entries under the base
         hooks. The original receiver is its allocation-tracking virtual base;
         the compiler, not a handwritten close wrapper, owns that adjustment.
// ORIGINAL: 0x00401060 ?close@StringStruct@@QAEXXZ 0x00401060-0x004010F9 BYTE_EXACT
// symbol    ??1StringStruct@@UAE@XZ
// size      153 bytes
// prototype void (__thiscall ?close@StringStruct@@QAEXXZ)(StringStruct* this)
// callers   25   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
// indirect  0x004010A2 0x004010B4 0x004010D0
// Historical LEVER/TRIED observations below describe the retired close wrapper.
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
Status: Genuine destructor; original inlining and strict matching remain unproved.
*/
// Original 4066C0's EH cleanup adjusts its saved virtual-base receiver by -0xC
// and jumps to 401060. The emitted base destructor makes the remaining -0x1C
// adjustment and stages the two base tables itself, as in the original body.
StringStruct::~StringStruct() {
    finish_list_teardown(this);
}

// StringStructDerivedVtable / StringStructDerivedVirtualBaseVtable
// (0x006698C4/0x006698C0), close_with_tables, and
// string_struct_derived_close_redirect are gone. The redirect's own marker
// carried the diagnosis all along: the image's 0x004066C0 is a
// COMPILER-GENERATED two-stage teardown (its LEVER/TRIED notes record the
// /GX frame no free function can produce). The real ~StringList destructor
// above owns the derived cleanup before ~StringStruct's base stage. The EH
// frame's funclet span 0x00650980-0x00650995 is that machinery's own.

// THE ALLOCATOR HAND-OFF SLOT: the Heap published by the allocation wrapper
// and captured by the next StringAllocationBase construction.
Heap *StringAllocationHeap;  // 0x009B3374



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

// MEASURED inline, defined at the end of stringstruct.h: the image both
// calls this (2 callers) and inlines it whole (BasePop::exec 0x00602FC5, abs
// calls and all). The definition moved; the claim is unchanged.

/*
Purpose: Return the current StringList text, or null when the list is empty.

             mov eax, [ecx+8] / test eax,eax / je zero
             mov eax, [ecx+0xc] / mov eax, [eax+8] / mov eax, [eax+4] / ret
       zero: xor eax,eax / ret

         The guard reads field 8 but the chase starts from field 0xc - two
         DIFFERENT fields, which is easy to lose when transcribing and produces
         a body that works whenever the two happen to agree.

// ORIGINAL: 0x005E3630 sub_5e3630 0x005E3630-0x005E3644 SEMANTIC
// TRIED: 8 source shapes measured, plateaus at MNEMONIC_ONLY 7/9; the image reuses eax across the
//            whole chase and every guard-clause / named-link / walking-
//            pointer spelling still routes the middle link through ecx.
// symbol    ?current_text@StringList@@QAEPADXZ
// body      src/stringstruct.h
// size      20 bytes
// prototype
// callers   2   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: mutable text, or null when head_ is null
Status: Complete
*/
// Header-visible inline definition in stringstruct.h. current_text is a
// provisional descriptive name, not a recovered original spelling. Original
// Buffer::write_strings/height callers (0x005DB3C4, 0x005DB00E) pass their
// StringList receiver in ECX and consume the result as text. The head guard
// and current-entry chase also occur inline in CheckBox/RadioButton drawing.
