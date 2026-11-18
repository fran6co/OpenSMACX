// WORKAROUND(noinline): this TU owns the out-of-line StringStruct constructor
// body that Datalink calls (0x00401000); see datalink.cpp.
#define STRINGSTRUCT_CTOR_OUT_OF_LINE
// WORKAROUND(decl): only this unit sees the payloads' direct
// StringAllocationBase derivation (stringstruct.h explains the gate).
#define STRINGSTRUCT_PAYLOAD_DIRECT_BASE
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
#include "multiwin.h"  // MultiWindow.draw requires its complete owner declaration.

// ORIGINAL: 0x00402C20 ??_GStringStructEntry@@UAEPAXI@Z 0x00402C20-0x00402C62 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original table installed by 401100 construction dispatches deletion through StringStructEntry allocation base at +0x14. This original body publishes saved Heap owner, conditionally frees the complete allocation and clears owner on deleting paths. Same-span seed retired; the complete original identity and spans remain on this compiled owner.", "Original construction at 401100 and typed deletion at 402970 establish the owning node lifecycle. The canonical source host is stringstruct.cpp; the compiler owns the virtual-base adjustment and deleting-flag entry.", "git:0e184947304cbb843b26264d5145e9b9b452cf16:src/recovered/units/00402c20.cpp", "Historical seed spelling sub_402c20; current source mapping ??_GStringStructEntry@@UAEPAXI@Z; emitted symbol ??_GStringStructEntry@@UAEPAXI@Z."], "reason": "Map the compiler-generated scalar deleting destructor emitted by actual list-node construction, not a handwritten wrapper. Superseded seed retired after owner review; original spans, existing claims and useful seed findings remain in source. This cleanup grants no new match or acceptance claim."}
// ORIGINAL: 0x00402C70 ??_GStringStructPayload@@UAEPAXI@Z 0x00402C70-0x00402CB2 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original table installed by 401100 construction dispatches deletion through StringStructPayload allocation base at +0x10. This original body publishes saved Heap owner, conditionally frees the complete allocation and clears owner on deleting paths. Same-span seed retired; the complete original identity and spans remain on this compiled owner.", "Original construction at 401100 and typed deletion at 402970 establish the owning node lifecycle. The canonical source host is stringstruct.cpp; the compiler owns the virtual-base adjustment and deleting-flag entry.", "git:0e184947304cbb843b26264d5145e9b9b452cf16:src/recovered/units/00402c70.cpp", "Historical seed spelling sub_402c70; current source mapping ??_GStringStructPayload@@UAEPAXI@Z; emitted symbol ??_GStringStructPayload@@UAEPAXI@Z."], "reason": "Map the compiler-generated scalar deleting destructor emitted by actual list-node construction, not a handwritten wrapper. Superseded seed retired after owner review; original spans, existing claims and useful seed findings remain in source. This cleanup grants no new match or acceptance claim."}
// ORIGINAL: 0x004029F0 ??1StringStruct@@QAE@H@Z 0x004029F0-0x00402A56 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:stringstruct-lifecycle-coverage", "Pinned original0x4029f0", "historical seed:src/recovered/units/004029f0.cpp", "Real virtual-base construction and physical compiler-generated lifecycle symbols."], "reason": "Original allocation-base receiver adjustment, list teardown and owner-aware conditional deallocation belong to the real compiler-generated scalar deleting destructor. Source virtual inheritance, destructor and inherited operator delete supply lifetime; catalogue destructor spelling remains historical. No handwritten adjustment or assembly."}
// compile_context src/alphanet.cpp
// symbol    ??_GStringStruct@@UAEPAXI@Z
#include "stringstruct.h"
#include "spritebox.h"
#include "net_class.h"
#include "technology.h"
#include "random.h"
#include "popup.h"
#include "strings.h"

#include <cstring>
#include <new>

#include "heap.h"  // StringAllocationHeap
#include "general.h"  // mem_get
#include "faction.h"  // Players, for get_he_she below
#include "alpha.h"  // Labels, same body
#include "game.h"  // PronounBuffer, same body
#include "reportwin.h"  // ReportWinGlobal owns the original 0x00876478 window
#include "text.h"
#include "console.h"  // ConsoleGlobal owns the cursor history read by draw_cursor.

// The allocation-base table at 0x6693AC dispatches this scalar deleting
// entry. The compiler supplies its flag handling and class-specific delete;
// the original identity is retained from the retired handwritten wrapper.
/*
// ORIGINAL: 0x00401520 sub_401520 0x00401520-0x0040155E FILE BYTE_EXACT
// symbol    ??_GStringAllocationBase@@UAEPAXI@Z
// size      62 bytes
// prototype
// callers   0   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00644EF2
*/

// Keep this original identity in its canonical mapping host while the real
// virtual allocation-base destructor lives in the class declaration. Its body
// republishes the captured owner; the compiler supplies the destructor-table
// stage and virtual-base adjustments at each actual lifetime site. The
// compile_context below records a current physical emitting TU, not original
// translation-unit ownership.
// ORIGINAL: 0x00402310 sub_402310 0x00402310-0x0040231F FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:allocation-base-destructor-owner", "Original 0x00402310..0x0040231F reads the captured Heap at this+4, stages allocation-base table 0x006693AC and publishes owner 0x009B3374; StringAllocationBase and StringAllocationHeap own these relationships.", "historical seed:src/recovered/00402310.cpp"], "reason": "The retained synthetic owner-publication seed is the existing genuine allocation-base destructor. Real member/global and compiler-owned destructor table stage replace fixed executable addresses; retain original identity and historical exact obligation."}
// compile_context src/alphanet.cpp
// body src/jackal/stringstruct.h
// symbol    ??1StringAllocationBase@@UAE@XZ

// ORIGINAL: 0x00401000 ??0StringStruct@@QAE@H@Z 0x00401000-0x00401058 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:stringstruct-lifecycle-coverage", "Pinned original0x401000", "historical seed:src/recovered/00401000.cpp", "Real virtual-base construction and physical compiler-generated lifecycle symbols."], "reason": "Original constructor initializes five list fields after genuine virtual-base owner capture and table staging. The seed explicit integer is the compiler-supplied most-derived flag, not a source parameter. Real out-of-line constructor owns this entry; retain historical matching obligation."}
// symbol    ??0StringStruct@@QAE@XZ
StringStruct::StringStruct() {
    head_ = 0;
    current_ = 0;
    entry_count_ = 0;
    current_position_ = 0;
    allocator_ = 0;
}

// The original primary table uses these shared defaults. Hook names describe
// their list role; the catalogue's unrelated class names are retained as seeds.
// RECOVERY_SHARED: v1 0x00404250 0x00404255 ?on_insert@StringStruct@@UAEHPAX@Z 0x006086F0
int StringStruct::on_insert(void *) { return 0; }
// RECOVERY_SHARED: v1 0x00404270 0x00404273 ?on_remove@StringStruct@@UAEXPAX@Z 0x006086F0
void StringStruct::on_remove(void *) { }

// The list-node allocation template (MATCHING_GUIDE 2.6), the same pair as
// ServiceListEntry/ServiceStruct in net_class.cpp: the list's explicit Heap
// is used by one overload and is fatal when it cannot satisfy the request;
// the other takes the list, tries its inherited allocation owner (also
// fatal) and then the process heap, which may fail quietly. The chosen Heap
// is published for the node's allocation base. The matching placement
// deletes give each new-expression the image's cleanup flag
// `mov [ebp-4], ebx` (0x0040110B).
// WORKAROUND(decl): the original overloads were probably class members of
// the list nodes; these global forms stand in for them without a header
// change.
// The inline allocators' fatal-box text is one linker-merged literal pair
// shared by every unit that expands them (stringstruct, alphanet_records,
// net_class): the image pushes 0x00682700 and 0x00682724 from all of them.
// RECOVERY_LITERAL: v2 merged 0x00682700 0x00682722 ??_C@_0CC@JCEM@out?5of?5heap?5space?5in?5new?5functio@ 0x00401100
// RECOVERY_LITERAL: v2 merged 0x00682724 0x0068272D ??_C@_08JLBK@Aborting?$AA@ 0x00401100
inline void *__cdecl operator new(size_t size, Heap *heap) throw() {
    void *memory = heap->get(size);
    if (memory != 0) {
        StringAllocationHeap = heap;
        return memory;
    }
    MessageBoxA(NULL, "out of heap space in new function", "Aborting", MB_OK);
    exit(3);
    return 0;
}
inline void *__cdecl operator new(size_t size, StringStruct *list) throw() {
    void *memory;
    Heap *owner = list->allocation_owner_;
    if (owner != 0) {
        memory = owner->get(size);
        if (memory != 0) {
            StringAllocationHeap = owner;
            return memory;
        }
        MessageBoxA(NULL, "out of heap space in new function", "Aborting", MB_OK);
        exit(3);
    }
    memory = mem_get(size);
    if (memory == 0) return 0;
    StringAllocationHeap = owner;
    return memory;
}
inline void __cdecl operator delete(void *memory, Heap *) {
    if (memory != 0) {
        if (StringAllocationHeap == 0) free(memory);
        StringAllocationHeap = 0;
    }
}
inline void __cdecl operator delete(void *memory, StringStruct *) {
    if (memory != 0) {
        if (StringAllocationHeap == 0) free(memory);
        StringAllocationHeap = 0;
    }
}

// Append before the circular head. Preserve the original partial mutations on
// allocation failure and ignore the insertion hook's return value: neither
// rollback nor propagating that value is part of 0x00401100's behavior.
// Superseded seed deleted; seed paths below are historical Git provenance.
// Historical seed observations (not fresh measurements or acceptance):
// TRIED: alloc_entry_node/alloc_id_node/report_fatal helper functions factoring the repeated Heap::get-or-mem_get pattern; introduces real CALLs the inlined original does not have, diverges at insn #4
// ORIGINAL: 0x00401100 ?add@StringStruct@@QAEHH@Z 0x00401100-0x00401513 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original 401100 selects explicit allocator then inherited allocation owner then mem_get, constructs entry size1C and payload size18 with distinct virtual-base tables. Native construction replaces seed vptr writes; source class names remain interpretations.", "Original 4013B1 publishes the entry allocation into the old tail next link before the null test; 4012D2/4014DC publish null payloads before returning4. Insertion hook result is ignored; count and position update only after it returns. Same-span legacy seed retained."], "reason": "Recover circular-list insertion with real node construction and the original partial updates on allocation failure."}
int StringStruct::add(int id) {
    if (entry_count_ == 0) {
        head_ = allocator_ ? ::new(allocator_) StringStructEntry : ::new(this) StringStructEntry;
        if (head_ == 0) return 4;
        head_->id = id;
        head_->next = head_;
        head_->previous = head_;
        if (allocator_) head_->payload = ::new(allocator_) StringStructPayload;
        else head_->payload = ::new(this) StringStructPayload;
        if (head_->payload == 0) return 4;
        on_insert(head_->payload);
        current_ = head_;
    } else {
        head_->previous->next = allocator_ ? ::new(allocator_) StringStructEntry : ::new(this) StringStructEntry;
        if (head_->previous->next == 0) return 4;
        head_->previous->next->previous = head_->previous;
        head_->previous->next->next = head_;
        head_->previous = head_->previous->next;
        current_ = head_->previous;
        current_->id = id;
        if (allocator_) current_->payload = ::new(allocator_) StringStructPayload;
        else current_->payload = ::new(this) StringStructPayload;
        if (current_->payload == 0) return 4;
        on_insert(current_->payload);
    }
    current_position_ = entry_count_;
    ++entry_count_;
    return 0;
}

// Moved from spritebox.cpp: 0x00402550 and 0x00402BB0 lie in this unit's
// address range (0x00401000-0x00402CB2) and, like StringStruct::add, keep
// the frame pointer this unit's /Oy- recipe gives.
// SpriteList's half of the list-node allocation template: the explicit-Heap
// overload above serves its allocator_, this one its inherited allocation
// owner and then mem_get (0x0040259E..0x004025E6).
inline void *__cdecl operator new(size_t size, SpriteListBase *list) throw() {
    void *memory;
    Heap *owner = list->allocation_owner_;
    if (owner != 0) {
        memory = owner->get(size);
        if (memory != 0) {
            StringAllocationHeap = owner;
            return memory;
        }
        MessageBoxA(NULL, "out of heap space in new function", "Aborting", MB_OK);
        exit(3);
    }
    memory = mem_get(size);
    if (memory == 0) return 0;
    StringAllocationHeap = owner;
    return memory;
}
inline void __cdecl operator delete(void *memory, SpriteListBase *) {
    if (memory != 0) {
        if (StringAllocationHeap == 0) free(memory);
        StringAllocationHeap = 0;
    }
}

// ORIGINAL: 0x00402550 SpriteStruct::add 0x00402550-0x00402963 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:1240d9eead700ef28006f35018fca68d1123defd:src/leaf_recoveries.cpp", "pinned:SpriteList tables6708A0/6708A4;6116E0 calls611730 with virtual-baseview+28", "pinned:SpriteListBase tables66943C/669438 and list fields +8..+18; StringStruct independently uses 6693A4/6693A0", "source:src/spritebox.h;source:src/stringstruct.h", "source:recovery/tests/test_sprite_list_producer_actual_source.py"], "reason": "Recover SpriteList insertion through its sibling ring owner and distinct polymorphic string/sprite payload siblings, with compiler-owned virtual allocation-base lifetime. Combined constructor/destructor evidence supports the separate SpriteListBase owner model; the ring fields remain layout-compatible and separately owned. Historical identities, spans, claims and seed discoveries remain obligations. Producer partial-state behavior and null/error returns remain unchanged."}
// symbol    ?add@SpriteList@@QAEHH@Z
// Historical seed finding: TRIED fallback __stdcall(int) signature; native ECX/prologue supports thiscall
// Historical seed finding: signature divergence from ServiceStruct::add sibling begins at instruction 7
// Historical seed finding: native body allocates the 0x1C entry and 0x20 sprite payload, with calls at 0x5D4510, 0x5D4680 and 0x644DFF
// Historical seed finding: old fixed-image seam/class attribution was provisional; retain as history, not current hierarchy proof
// TRIED: fallback __stdcall(int) signature - disasm has `mov esi,ecx` in the prologue (this-in-ecx), matched as __thiscall SpriteStruct::add per the ServiceStruct::add sibling shape; diverges at insn #7, same prologue-order divergence as 0x00401760
int SpriteList::add(int id) {
    if (entry_count_ == 0) {
        head_ = allocator_ ? ::new(allocator_) StringStructEntry : ::new(this) StringStructEntry;
        if (head_ == 0) return 4;
        head_->id = id;
        head_->next = head_;
        head_->previous = head_;
        if (allocator_) head_->payload = ::new(allocator_) SpriteStruct;
        else head_->payload = ::new(this) SpriteStruct;
        if (head_->payload == 0) return 4;
        on_insert(head_->payload);
        current_ = head_;
    } else {
        head_->previous->next = allocator_ ? ::new(allocator_) StringStructEntry : ::new(this) StringStructEntry;
        if (head_->previous->next == 0) return 4;
        head_->previous->next->previous = head_->previous;
        head_->previous->next->next = head_;
        head_->previous = head_->previous->next;
        current_ = head_->previous;
        current_->id = id;
        if (allocator_) current_->payload = ::new(allocator_) SpriteStruct;
        else current_->payload = ::new(this) SpriteStruct;
        if (current_->payload == 0) return 4;
        on_insert(current_->payload);
    }
    current_position_ = entry_count_;
    ++entry_count_;
    return 0;
}

// SpriteList owns the sprite-specific payload lifecycle. Ordinary close is a
// separate list clear with no original identity; 0x611730 is associated with
// the compiler-generated virtual-base destructor entry by the native writer
// after caller/layout review. C++ owns all vptr/vbtable staging.
// Pinned 0x00402BB0 receiver is the allocation virtual-base view (+0x1C);
// it adjusts to this list, clears through the shared 0x00402970 ring walker,
// restores the StringAllocationBase table/owner, then honors deleting bit 0.
// The 0x66943C/0x669438 primary/vbase stage is distinct from StringStruct's
// 0x6693A4/0x6693A0 stage; shared ring layout does not imply inheritance.
// 0x00611730 is the SpriteList lifecycle's expanded ring-cleanup body.
// ORIGINAL: 0x00402BB0 sub_402bb0 0x00402BB0-0x00402C16 FILE BYTE_EXACT
// kind game
// symbol    ??_GSpriteListBase@@UAEPAXI@Z
// Historical seed fact (inactive): size      102 bytes
// Historical seed fact (inactive): prototype
// Historical seed fact (inactive): callers   0   call targets   2
// Historical seed fact (inactive): kind      game
// Historical seed fact (inactive): flags     frame;sp_ready;purged_ok
// Historical seed fact (inactive): calls     0x00402970 0x00644EF2
// Historical seed fact (inactive): PRESERVED UNIT - measured MNEMONIC_ONLY.
// Historical seed fact (inactive): Historical seed mode (inactive): the artifact record was FILE; this proposal uses BODY because the identity is transferred to a genuine C++ method owner.
// Historical seed fact (inactive): Historical owner evidence: receiver is the StringAllocationBase virtual view +0x1C.
// Historical seed fact (inactive): Historical owner evidence: restores the 0x669438/0x66943C list stage, calls 0x402970, clears +0x14, restores 0x6693AC/owner, then tests deleting bit 0 and conditionally frees.
// Historical seed fact (inactive): Historical measurement note: the seed reported MNEMONIC_ONLY; no BYTE_EXACT or SEMANTIC claim is transferred.
SpriteList::SpriteList() {}

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
// WORKAROUND(inline): pick-any copy of the header inline, kept emitted by the
// MEASURED declaration (see current_entry below).
inline int StringStruct::current_id() {
    if (head_) {
        return current_->id;
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
// WORKAROUND(inline): the original header defined current_entry inline in
// every unit; 0x00402530 is a pick-any copy, and buffer.cpp both expands it
// (current_text 0x005E3637) and calls it (0x005DAE38, 0x005DB118), so its
// own copy is a COMDAT too. The MEASURED declaration in stringstruct.h keeps
// this unit's copy emitted without a caller.
inline StringStructPayload *StringStruct::current_entry() {
    if (head_) {
        return current_->string_payload();
    }
    return 0;
}

/*
Purpose: Advance the current string-list entry and return its payload.
// ORIGINAL: 0x00402500 ?next_entry@StringStruct@@QAEHXZ 0x00402500-0x0040252F BYTE_EXACT
// body      src/jackal/stringstruct.h
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
// body src/jackal/stringstruct.h
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
// MEASURED inline, defined in stringstruct.h: the image calls this from
// CheckBox::on_left_down and inlines it in Datalink::draw_concept and
// draw_adv_concept. The definition moved; the claim is unchanged.


extern "C" int __cdecl string_struct_seek_id_source(StringStruct *self, int id) {
    return self->seek_id(id);
}


/*
Purpose: Release every entry in the list, notifying the owner about each
         payload before destroying the payload and its entry.
// ORIGINAL: 0x00402970 StringStruct::remove_all 0x00402970-0x004029E3 FILE BYTE_EXACT
// compile_context src/stringstruct.cpp
// symbol    ?remove_all@StringStruct@@QAEXXZ
// body      src/jackal/stringstruct.h
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
// MEASURED inline, defined in stringstruct.h: the image both calls this
// out of line and expands it inside StringList's destructor.

// StringStructVtable / StringStructVirtualBaseVtable (0x006693A4/0x006693A0)
// are gone with the hand close chain - the compiler emits both tables from
// the declarations, and nothing names them any more.

/*
Purpose: Destroy the StringStruct subobject, releasing entries under the base
         hooks. The original receiver is its allocation-tracking virtual base;
         the compiler, not a handwritten close wrapper, owns that adjustment.
// ORIGINAL: 0x00401060 ?close@StringStruct@@QAEXXZ 0x00401060-0x004010F9 FILE BYTE_EXACT
// compile_context src/alphanet.cpp
// symbol    ??1StringStruct@@UAE@XZ
// body      src/jackal/stringstruct.h
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
// StringStructDerivedVtable / StringStructDerivedVirtualBaseVtable
// (0x006698C4/0x006698C0), close_with_tables, and
// string_struct_derived_close_redirect are gone. The redirect's own marker
// carried the diagnosis all along: the image's 0x004066C0 is a
// COMPILER-GENERATED two-stage teardown (its LEVER/TRIED notes record the
// /GX frame no free function can produce). The real ~StringList destructor
// above owns the derived cleanup before ~StringStruct's base stage. The EH
// frame's funclet span 0x00650980-0x00650995 is that machinery's own.


/*
// ORIGINAL: 0x004015B0 ?seek_pos@StringStruct@@QAEHH@Z 0x004015B0-0x00401636 BYTE_EXACT
// body      src/jackal/stringstruct.h
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

// ===========================================================================
// Coverage-first transcriptions (wave 1, journal 137/138). Each entry below
// lands a real compiling body for a seed that the census assigned to this TU
// by marker range, retiring the seed file in the same change. These are
// unclaimed FILE mappings: no byte claim is made or implied, and no spelling
// iteration was performed in this pass. Provenance for every retired seed is
// git:da8a4b7f:src/unrecovered/<addr>.cpp.
// ===========================================================================

#include "buffer.h"
#include "graphicwin.h"
#include "worldwin.h"  // WorldClimateWorldWin, the caption-painter guard
#include "font.h"      // FontDefault
#include "alpha.h"     // Labels
#include "strings.h"   // StringTable
#include "faction.h"   // Players
#include "game.h"      // PronounBuffer



// ===========================================================================
// Coverage-first transcriptions (wave 3, journal w3-stringstruct2-scope).
// Each entry lands a real compiling body for a seed the census assigned to
// this TU by marker range (the [0x00401060, 0x00608770] span of this file's
// ORIGINAL markers, the same rule that gave wave 1 its sub-200-line queue),
// retiring the seed in the same change. These are unclaimed FILE mappings:
// no byte claim is made or implied, and no spelling iteration was performed
// in this pass. Provenance for every retired seed is
// git:ee30a69d:src/unrecovered/<addr>.cpp.
// ===========================================================================

#include "sprite.h"    // Sprite - the surface sprite the unit wrappers draw
#include "veh.h"       // Vehs - the deletion list walks its 0x34-stride records
#include "lock.h"      // LockEnableMask - the deletion-record occupancy mask
#include "log.h"       // log_say - both deletion paths report through it
#include "fx.h"        // FX - play is defined in this wave
#include "pulldown.h"   // PullDown - the menu handlers are defined in this wave
#include "reportwin.h"  // ReportWin - on_redraw is defined in this wave
#include "diplopop.h"   // DiploPop - on_redraw is defined in this wave
#include "mapwin.h"    // MapWin + MainInterfaceGlobal (0x007AE820)
#include "map.h"       // MapIsFlat / MapLongitudeBounds - the cursor wrap
#include "maininterface.h"  // MainInterface::restore_back - DiploPop's opener


// The one-space join string this file's flying-text wrapper already binds as
// SpaceText, and the caret separator MultiWin's spot label builds from.

// The local faction at 0x00939284 is ConsoleGlobal.faction_id_:
// source-owned ConsoleGlobal at 0x009156B0 plus member offset 0x23BD4.

// The spot label is built in strings.h's StringTemp (0x009B86A0), the same
// shared scratch the sibling alpha.cpp prefs paths fill.


// Buffer owns its shared surface Sprite at 0x009B3A50; this unit does not
// access that object directly. Sprite owns the tech-wizard facility icons.
extern Sprite g_TECH_ICON_SPRITES[89];  // 0x00759E28, sprite.cpp's definition

// The published unit draw-state block: the words the surface sprite's
// draw_dest_unk1 readers consume, 0x009B3A54..0x009B3A78.


// ---------------------------------------------------------------------------
// FX::play, 0x00446A00. The bank's sound trigger: gate on the sound bit of
// GamePreferences, re-arm the popup wave owner for the first three effect
// kinds, then release/restart the effect record. fx.h declares the member;
// its forwarder retires from pending_bodies.cpp in this change.

// Interlude members and compiler-generated lifetime code are owned by interlude.cpp.

// Media overlay update is owned by MediaRuntime in movie_media.cpp.


// Original pointer cell 0x00691B18 selects the WIZARD text at 0x00691B94.
// Keep that pointer/text relationship in source-owned storage. Names are
// descriptive; bindings establish starts, not enclosing allocation bounds.
// RECOVERY_OBJECT: v1 0x00691B94 ?TechWizardPopupFileText@@3PADA
char TechWizardPopupFileText[7] = "WIZARD";
// RECOVERY_OBJECT: v1 0x00691B18 ?TechWizardPopupFile@@3PADA
char *TechWizardPopupFile = TechWizardPopupFileText;


// The old 447D20 binding in this file was a settings-checkbox/repaint seed
// hypothesis. The pinned body instead calls Midi's trackset status,
// transpose and play operations; the original identity now lives with Midi.
// Superseded RECOVERY_BINDING history retained verbatim:
// {"evidence": ["Original447D20..447DA5; retired seed git:ee30a69d:src/unrecovered/00447d20.cpp records the same identity and spans.", "The shared delay counter is 0x00683C2C, stepped by 0x12C and floored at -0x4B0 exactly as the seed walks it; the receiver's slots 67/69/70 are dispatched at their measured indices through member-fn-pointer unions because the object's class is unnamed in the catalogue."], "reason": "Coverage transcription of the settings cascade with the delay counter bound and the receiver's slots dispatched at their measured indices."}


// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------


// ---------------------------------------------------------------------------


// ---------------------------------------------------------------------------
// sub_5d8c20, 0x005D8C20. The unit-draw wrapper's four-argument form:
// publish the draw-state block (transparency byte, source offsets) beside
// the shared surface sprite, run the receiver's actor-release countdown,
// then draw through the sprite's table-driven draw_dest_unk1. The receiver
// is an unnamed actor object (fields at 0x50/0x54/0x58/0x6C/0x80/0x84/0x4A8)
// and the original is a __thiscall member.


// ---------------------------------------------------------------------------
// sub_5d8d00, 0x005D8D00. The same wrapper's six-argument form: the
// identical state publish and actor countdown, then the table-driven
// draw_dest_unk1 with the two extra drawing arguments.

#include "win.h"       // Win::window_proc, the default path of PrevWndFunc
#include "console.h"   // ConsoleGlobal - net_not_my_turn hands the view back
#include "netdaemon.h" // NetDaemonNet - the turn wait pumps its net_tasks


// ---------------------------------------------------------------------------

// Interlude members and compiler-generated lifetime code are owned by interlude.cpp.

// ---------------------------------------------------------------------------


// ---------------------------------------------------------------------------
class Int2SlotObj { public: void slot(int, int); };


// ---------------------------------------------------------------------------


// ---------------------------------------------------------------------------


