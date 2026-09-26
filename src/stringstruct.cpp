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

// ===========================================================================
// Coverage-first transcriptions (wave 1, journal 137/138). Each entry below
// lands a real compiling body for a seed that the census assigned to this TU
// by marker range, retiring the seed file in the same change. These are
// unclaimed FILE mappings: no byte claim is made or implied, and no spelling
// iteration was performed in this pass. Provenance for every retired seed is
// git:b18d3ac8:src/unrecovered/<addr>.cpp.
// ===========================================================================

#include "buffer.h"
#include "graphicwin.h"
#include "worldwin.h"  // WorldClimateWorldWin, the caption-painter guard
#include "font.h"      // FontDefault
#include "alpha.h"     // Labels
#include "strings.h"   // StringTable
#include "faction.h"   // Players
#include "game.h"      // PronounBuffer
#include "temp.h"      // BufferStrHeight

/*
Purpose: Paint a window caption's text through the caller's canvas, honouring
         the caption alignment flags, then restore the canvas clip.

         The receiver is the window being captioned; the guard at entry
         declines to paint the process-wide climate window, whose caption is
         drawn by its own owner.
// ORIGINAL: 0x004554C0 ?def_caption_draw@@YAHPAUGraphicWin@@PADPAUBuffer@@@Z 0x004554C0-0x00455606 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Original4554C0..455606; retired seed git:b18d3ac8:src/unrecovered/004554c0.cpp records the same identity and spans with its historical register-schedule TRIED note (kept below).", "The receiver state is Win's protected caption block (style word 0x98, font 0xF8, four text colours 0x104..0x110), GraphicWin's private frame sprites 0x9DC/0x9E8 with their +0x20/+0x24 offsets, and Buffer's protected rect1_ at 0x30: all spelled as documented offsets because this free painter is not a member or friend of those classes.", "0x008E9F60 is worldwin.h's WorldClimateWorldWin. All five callees are buffer.cpp's set_clip/set_font/set_text_color/write_l/write_cent_l/write_right_l overloads."], "reason": "Coverage transcription of the free caption painter with typed receivers; the emitted name spells class GraphicWin where the catalogue wrote the struct tag."}
// symbol    ?def_caption_draw@@YAHPAVGraphicWin@@PADPAVBuffer@@@Z
// TRIED: the four locals (local_14/local_10/local_c/local_8, adjacent on the stack in that order) are exactly a RECT (left, top, right, bottom) in memory order, so assigning through a local RECT was tried directly - it compiles but the register-save schedule differs (rebuilt saves ebx/edi immediately, the original defers them past both early-exit guards). Best reached: MISMATCH, edit_count 25, 317 bytes vs 326, mnemonic similarity ~0.73.
Return Value: the restored set_clip verdict
Status: Coverage transcription - unclaimed body
*/
int __cdecl def_caption_draw(GraphicWin *window, char *text, Buffer *canvas) {
    if (window == &WorldClimateWorldWin || text == 0) {
        return 0;
    }

    char *const window_base = reinterpret_cast<char *>(window);
    char *const canvas_base = reinterpret_cast<char *>(canvas);
    // The caption origin follows the left/top frame sprites' own offsets:
    // GraphicWin's private frame_left_/frame_top_ (0x9DC/0x9E8) and their
    // iLeftOffset_/iTopOffset_ (+0x20/+0x24).
    const int frame_left_offset = *reinterpret_cast<int *>(
        *reinterpret_cast<char **>(window_base + 0x9DC) + 0x20);
    const int frame_top_offset = *reinterpret_cast<int *>(
        *reinterpret_cast<char **>(window_base + 0x9E8) + 0x24);
    // Buffer's protected clip rectangle, rect1_ at +0x30.
    RECT *const canvas_rect = reinterpret_cast<RECT *>(canvas_base + 0x30);

    RECT area;
    area.right = canvas_rect->right - 0x28;
    area.left = canvas_rect->left + 0x23 + frame_left_offset;
    area.top = canvas_rect->top + 0xB + frame_top_offset;
    area.bottom = canvas_rect->bottom - 0xC;

    // Win's protected caption state: the four text colours at
    // +0x104..+0x110, the font at +0xF8 and the style word at +0x98.
    canvas->set_text_color(
        *reinterpret_cast<int *>(window_base + 0x104),
        *reinterpret_cast<int *>(window_base + 0x108),
        *reinterpret_cast<int *>(window_base + 0x10C),
        *reinterpret_cast<int *>(window_base + 0x110));
    canvas->set_font(
        *reinterpret_cast<Font **>(window_base + 0xF8), 0, 0, 0);
    canvas->set_clip(&area);

    const unsigned int len = strlen(text);
    const unsigned int style = *reinterpret_cast<unsigned int *>(
        window_base + 0x98);
    if ((style & 0x10000) != 0) {
        canvas->write_l(text, &area, len);
    } else if ((style & 0x40000) != 0) {
        canvas->write_right_l(text, &area, len);
    } else {
        canvas->write_cent_l(text, &area, len);
    }
    return canvas->set_clip(canvas_rect);
}

/*
Purpose: Order three-way comparison of two fixed-shape name records: an
         unsigned word, two unsigned shorts and eight bytes, each field
         compared lexicographically.
// ORIGINAL: 0x005D4BB0 sub_5d4bb0 0x005D4BB0-0x005D4C41 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Original5D4BB0..5D4C41; retired seed git:b18d3ac8:src/unrecovered/005d4bb0.cpp records the same identity and spans.", "The body reads only its two stack arguments and makes no calls; the field walks (word at +0, shorts at +4/+6, eight bytes at +8) are transcribed exactly as the seed measured them, with no record type invented for them."], "reason": "Coverage transcription of the unnamed record comparator as a stdcall free function; no names exist for the record fields, so the reads stay spelled by offset."}
// symbol    _sub_5d4bb0@8
Return Value: -1, 0 or 1 as the first record orders before, equal to or after the second
Status: Coverage transcription - unclaimed body
*/
extern "C" int __stdcall sub_5d4bb0(int left, int right) {
    const unsigned int *left_word = reinterpret_cast<const unsigned int *>(left);
    const unsigned int *right_word = reinterpret_cast<const unsigned int *>(right);
    if (*left_word < *right_word) {
        return -1;
    }
    if (*left_word != *right_word) {
        return 1;
    }

    const unsigned short *left_mid =
        reinterpret_cast<const unsigned short *>(left + 4);
    const unsigned short *right_mid =
        reinterpret_cast<const unsigned short *>(right + 4);
    if (*left_mid < *right_mid) {
        return -1;
    }
    if (*left_mid != *right_mid) {
        return 1;
    }

    const unsigned short *left_tail =
        reinterpret_cast<const unsigned short *>(left + 6);
    const unsigned short *right_tail =
        reinterpret_cast<const unsigned short *>(right + 6);
    if (*left_tail < *right_tail) {
        return -1;
    }
    if (*left_tail != *right_tail) {
        return 1;
    }

    const unsigned char *left_bytes =
        reinterpret_cast<const unsigned char *>(left + 8);
    const unsigned char *right_bytes =
        reinterpret_cast<const unsigned char *>(right + 8);
    for (int i = 0; i < 8; i++) {
        if (left_bytes[i] < right_bytes[i]) {
            return -1;
        }
        if (left_bytes[i] != right_bytes[i]) {
            return 1;
        }
    }
    return 0;
}

/*
Purpose: Bubble-sort one int array descending while applying the identical
         exchanges to a parallel array addressed relative to it.
// ORIGINAL: 0x004D5F70 sub_4d5f70 0x004D5F70-0x004D5FE6 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Original4D5F70..4D5FE6; retired seed git:b18d3ac8:src/unrecovered/004d5f70.cpp records the same identity and spans.", "The sole callee 0x00628A50 is general.h's inline swap(int *, int *), declared in the header this TU already includes; the parallel array is reached by the same base-plus-index arithmetic the seed measured."], "reason": "Coverage transcription of the paired bubble sort with the in-tree swap; the parallel-array base stays an int because no owning array is named anywhere in the tree."}
// symbol    _sub_4d5f70
Status: Coverage transcription - unclaimed body
*/
extern "C" void __cdecl sub_4d5f70(int parallel_base, int *values, int count) {
    int limit = count - 1;
    int swapped;
    do {
        swapped = 0;
        if (limit < 1) {
            return;
        }
        int *cursor = reinterpret_cast<int *>(parallel_base);
        int *value = values;
        int remaining = limit;
        do {
            int *next = value + 1;
            if (*value < *next) {
                swapped = 1;
                swap(value, next);
                swap(cursor, reinterpret_cast<int *>(
                    parallel_base - reinterpret_cast<int>(values)
                    + reinterpret_cast<int>(next)));
            }
            cursor = cursor + 1;
            remaining = remaining - 1;
            value = next;
        } while (remaining != 0);
    } while (swapped != 0);
}

/*
Purpose: Release every bucket allocation recorded in a 0x80-slot pointer
         table plus its overflow slot, clearing the table as it goes.
// ORIGINAL: 0x004C7300 sub_4c7300 0x004C7300-0x004C7373 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Original4C7300..4C7373; retired seed git:b18d3ac8:src/unrecovered/004c7300.cpp records the same identity and spans with the fastcall-receiver TRIED note kept below.", "The sole call target 0x0064557F is the CRT operator delete, spelled here as the genuine deletion expression; each populated slot stores a record whose first two words are a successor index and a start index, exactly as the seed walked them."], "reason": "Coverage transcription of the bucket-table teardown with the receiver as an explicit table pointer; no bucket record type exists in-tree, so the index walks stay spelled by word offsets."}
// symbol    @sub_4c7300@4
// TRIED: scaffold had 0 params/cdecl; disassembly reads ecx with no stack access -> changed to `__fastcall sub_4c7300(int param_1)`. Redeclared the callee fn_0064557f (operator delete@0x64557f) to take `void *` instead of the scaffold's 0-arg stub. for-loop over the outer bucket scan diverges at instr #24 ('mov' vs rebuilt 'sub').
Status: Coverage transcription - unclaimed body
*/
extern "C" int __fastcall sub_4c7300(int table) {
    for (int i = 0; i < 0x80; i = i + 1) {
        int *bucket = *reinterpret_cast<int **>(table + i * 4);
        if (bucket != 0) {
            i = *bucket;
            int *record = *reinterpret_cast<int **>(table + i * 4);
            unsigned int start = static_cast<unsigned int>(record[1]);
            unsigned int end = static_cast<unsigned int>(record[0]) + 1;
            int *slot = reinterpret_cast<int *>(table + start * 4);
            ::operator delete(reinterpret_cast<void *>(*slot));
            if (start < end) {
                unsigned int count = end - start;
                while (count != 0) {
                    *slot = 0;
                    slot = slot + 1;
                    count = count - 1;
                }
            }
        }
    }
    if (*reinterpret_cast<int *>(table + 0x200) != 0) {
        ::operator delete(
            reinterpret_cast<void *>(*reinterpret_cast<int *>(table + 0x200)));
        *reinterpret_cast<int *>(table + 0x200) = 0;
    }
    return 0;
}

/*
Purpose: Install one name record across a pointer table's index range,
         cloning the record first and freeing every record the range
         displaces.

         The record carries two index words, one status word and a
         0x104-byte name buffer; the clone keeps the name and status and
         rebuilds the index words from the installed range.
// ORIGINAL: 0x004C73C0 sub_4c73c0 0x004C73C0-0x004C74A4 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Original4C73C0..4C74A4; retired seed git:b18d3ac8:src/unrecovered/004c73c0.cpp records the same identity and spans, including the unconditional-strcpy TRIED note kept below.", "The call targets are the CRT operator new(0x110)/operator delete/strcpy/memset the seed measured; the record shape (three words plus a 0x104 name buffer) is the seed's own measured layout, spelled here as a local POD so the allocation and copies are genuine C++."], "reason": "Coverage transcription of the table installer with the thiscall receiver spelled as an explicit first argument; the original receiver is a raw int table, so no class stand-in is introduced."}
// symbol    @sub_4c73c0@8
// TRIED: this is a __thiscall member whose `this` (ecx) is a raw `int *` array (indexed `param_1[uVar6]`, no fixed-offset fields), and param_2 is `Entry*` (3 uints + a 0x104-byte name buffer = 0x110 bytes, matching the `operator new(0x110)` allocation size exactly - `local_8` and `param_2` share this same layout). operator new/delete (mangled ??2@.../??3@...) reached via differently-named externs `op_new`/`op_delete` with the real signatures, since the scaffold's fn_0064557f/fn_0064558a are nullary and the call targets are masked from comparison anyway. strcpy is called UNCONDITIONALLY on `local_8->name` even on the `operator new` failure path (ghidra confirms this, not an artifact) - preserved as-is rather than "fixed".
Status: Coverage transcription - unclaimed body
*/
namespace {
// The installed record's measured shape: two index words, one status word
// and the 0x104-byte name buffer. 0x110 bytes total.
struct NameTableEntry {
    unsigned int first_index_;
    unsigned int last_index_;
    unsigned int status_;
    char name_[0x104];
};
}

extern "C" void __fastcall sub_4c73c0(int *table, void *entry_raw) {
    NameTableEntry *const entry = static_cast<NameTableEntry *>(entry_raw);
    const unsigned int new_last = entry->last_index_;
    const unsigned int new_first = entry->first_index_;

    // The original clones unconditionally, and its strcpy runs even when
    // operator new returned null - preserved exactly as measured.
    NameTableEntry *clone = new NameTableEntry;
    if (clone != 0) {
        memset(clone->name_, 0, 0x104);
        clone->last_index_ = 0;
        clone->first_index_ = 0;
        clone->status_ = 1;
    }

    strcpy(clone->name_, entry->name_);

    if (clone->first_index_ < new_last) {
        clone->first_index_ = new_last;
    }
    clone->last_index_ = new_last;
    if (new_first < new_last) {
        clone->last_index_ = new_first;
    }
    clone->first_index_ = new_first;
    clone->status_ = entry->status_;

    unsigned int slot = new_last;
    if (new_last <= new_first) {
        do {
            int *record = reinterpret_cast<int *>(table[slot]);
            if (record != 0) {
                unsigned int freed = static_cast<unsigned int>(record[1]);
                slot = static_cast<unsigned int>(record[0]) + 1;
                delete reinterpret_cast<NameTableEntry *>(table[freed]);
                if (freed < slot) {
                    int *clear = table + freed;
                    for (int remaining = slot - freed; remaining != 0;
                         remaining--) {
                        *clear = 0;
                        clear++;
                    }
                }
            }
            slot = slot + 1;
        } while (slot <= new_first);

        if (new_last <= new_first) {
            int *install = table + new_last;
            for (int remaining = (new_first - new_last) + 1; remaining != 0;
                 remaining--) {
                *install = reinterpret_cast<int>(clone);
                install++;
            }
        }
    }
}

/*
Purpose: Rewrite every filename recorded in a 0x80-slot table to live under a
         directory: append the directory (with a trailing separator added
         when missing) to each entry's basename.
// ORIGINAL: 0x004C74F0 sub_4c74f0 0x004C74F0-0x004C75A0 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Original4C74F0..4C75A0; retired seed git:b18d3ac8:src/unrecovered/004c74f0.cpp records the same identity and spans with the thiscall-receiver TRIED note kept below.", "The callees are the CRT strcpy/strcat/strlen/strrchr the seed measured; each table slot holds a record whose name string starts 0xC bytes in, exactly as the seed walked it."], "reason": "Coverage transcription of the directory rewriter with the receiver spelled as an explicit first argument over the slot table."}
// symbol    @sub_4c74f0@8
// TRIED: [ecx+N] reads with no matching stack slot -> a __thiscall receiver (Obj4c74f0), not the given __stdcall(int). 0.89 mnemonic sim; first divergence is prologue register/stack-slot allocation for the `this`/`a1` locals, not chased further.
Status: Coverage transcription - unclaimed body
*/
extern "C" void __fastcall sub_4c74f0(char **table, char *directory) {
    char dir_buf[0x100];
    strcpy(dir_buf, directory);
    const unsigned int len = strlen(dir_buf);
    if (dir_buf[len - 1] != '\\') {
        dir_buf[len] = '\\';
        dir_buf[len + 1] = 0;
    }

    for (int i = 0x80; i != 0; --i) {
        char *entry = *table;
        if (entry != 0) {
            char *name = entry + 0xC;
            if (name != 0) {
                char rewritten[0x100];
                strcpy(rewritten, dir_buf);
                char *slash = strrchr(name, '\\');
                char *basename = (slash != 0) ? slash + 1 : name;
                strcat(rewritten, basename);
                strcpy(name, rewritten);
            }
        }
        table = table + 1;
    }
}

/*
Purpose: Fetch the "him/her" pronoun for a faction's leader into the shared
         pronoun buffer, optionally upper-casing it (first letter only when
         sentence_form is one).
// ORIGINAL: 0x0058FC50 ?get_him_her@@YAPADHH@Z 0x0058FC50-0x0058FCDB FILE
// RECOVERY_BINDING: v1 {"evidence": ["Original58FC50..58FCDB; retired seed git:b18d3ac8:src/unrecovered/0058fc50.cpp records the same identity and spans.", "The fixed-address bindings rename to the tree's own models exactly as game.cpp's sibling get_his_her (0x0058FBC0) already does: Players (faction.h, the 0x167-stride first word is is_leader_female), Labels (alpha.h), StringTable (strings.h), PronounBuffer (game.h, 0x00945838) and the CharUpperA import the header supplies.", "Label code 0xC7/0xC8 and the sentence-form handling are the seed's measured behaviour."], "reason": "Coverage transcription of the pronoun fetcher in the established get_his_her idiom of the owning string family."}
Return Value: PronounBuffer
Status: Coverage transcription - unclaimed body
*/
char *__cdecl get_him_her(int faction_id, int sentence_form) {
    const int code = 0xC7 + (Players[faction_id].is_leader_female != 0);
    LPSTR *labels = (LPSTR *)Labels.strings_ptr;
    strcpy(PronounBuffer, StringTable.get((int)labels[code]));
    if (sentence_form != 0) {
        if (sentence_form == 1) {
            char buf[2];
            buf[0] = PronounBuffer[0];
            buf[1] = 0;
            CharUpperA(buf);
            PronounBuffer[0] = buf[0];
            return PronounBuffer;
        }
        CharUpperA(PronounBuffer);
    }
    return PronounBuffer;
}

/*
Purpose: Install a search path's plain half: release the previous string,
         then store a fresh copy of the given path (or, when one is already
         present in the directory half, that directory with the argument's
         basename appended).
// ORIGINAL: 0x004C8290 sub_4c8290 0x004C8290-0x004C8348 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Original4C8290..4C8348; retired seed git:b18d3ac8:src/unrecovered/004c8290.cpp records the same identity and spans.", "The receiver's two heap strings live at +0x1C/+0x20 exactly as the sibling installer 0x004C81C0 reads them; the callees are the CRT strcpy/strcat/strlen/strrchr plus operator new/delete, all spelled here as genuine C++ expressions."], "reason": "Coverage transcription of the path-half installer with the thiscall receiver spelled as an explicit first argument; no in-tree class owns the receiver yet, so the two string slots stay documented offsets."}
// symbol    @sub_4c8290@8
Status: Coverage transcription - unclaimed body
*/
extern "C" void __fastcall sub_4c8290(char *self, char *path) {
    char **plain = reinterpret_cast<char **>(self + 0x1C);
    if (*plain != 0) {
        ::operator delete(*plain);
    }
    *plain = 0;

    if (path != 0) {
        char buf[256];
        char *const directory = *reinterpret_cast<char **>(self + 0x20);
        if (directory != 0) {
            strcpy(buf, directory);
            char *slash = strrchr(path, '\\');
            if (slash != 0) {
                strcat(buf, slash + 1);
            } else {
                strcat(buf, path);
            }
        } else {
            strcpy(buf, path);
        }
        const unsigned int len = strlen(buf);
        char *fresh = static_cast<char *>(::operator new(len + 1));
        *plain = fresh;
        strcpy(fresh, buf);
    }
}

/*
Purpose: Install a search path's directory half: release the previous
         directory string, store the given directory with a trailing
         separator, and when a plain-half string already exists rebuild it
         under the new directory through sub_4c8290.
// ORIGINAL: 0x004C81C0 sub_4c81c0 0x004C81C0-0x004C8286 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Original4C81C0..4C8286; retired seed git:b18d3ac8:src/unrecovered/004c81c0.cpp records the same identity and spans.", "The receiver is the same +0x1C/+0x20 two-string object sub_4c8290 installs into; the original dispatches to 0x004C8290 on its own receiver, spelled here as the direct same-TU call, and the callees are CRT strlen/strcpy/strcat plus operator new/delete."], "reason": "Coverage transcription of the directory-half installer with the receiver as an explicit first argument."}
// symbol    @sub_4c81c0@8
Status: Coverage transcription - unclaimed body
*/
extern "C" void __fastcall sub_4c81c0(char *self, char *directory) {
    char **slot = reinterpret_cast<char **>(self + 0x20);
    if (*slot != 0) {
        ::operator delete(*slot);
    }
    *slot = 0;

    char buf[256];
    unsigned int len = strlen(directory);
    strcpy(buf, directory);
    if (buf[len - 1] != '\\') {
        buf[len] = '\\';
        buf[len + 1] = 0;
        ++len;
    }
    ++len;
    *slot = static_cast<char *>(::operator new(len));
    strcpy(*slot, buf);

    char **plain = reinterpret_cast<char **>(self + 0x1C);
    if (*plain != 0) {
        const unsigned int plain_len = strlen(*plain);
        char *cursor = *plain + plain_len;
        char *scan = cursor;
        while (scan != *plain) {
            if (*(scan - 1) == '\\') {
                break;
            }
            --scan;
        }
        strcat(buf, scan);
        sub_4c8290(self, buf);
    }
}

/*
Purpose: Write a vertical column of single-character centred lines: each
         character of the text becomes its own centred line, advancing by the
         font's line advance, which is first shrunk until the whole column
         fits the rectangle's height.
// ORIGINAL: 0x005DB450 sub_5db450 0x005DB450-0x005DB571 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Original5DB450..5DB571; retired seed git:b18d3ac8:src/unrecovered/005db450.cpp records the same identity and spans with its receiver-analysis TRIED note kept below.", "The receiver is a Buffer (rect1_ at +0x30, font1_ at +0x52C); the two public members used are set_clip and the five-int write_cent_l, and the default font comes from font.h's FontDefault at 0x009BB484. rect1_ and font1_ are spelled as documented offsets because this free function is not a member or friend of Buffer.", "The line-advance arithmetic (line_height_ when the font's unk_1_ is negative, height_ + unk_1_ otherwise, then shrunk against the font's own height_) is Font's private layout at +0xC/+0x10 with unk_1_ at +0, exactly as the seed measured."], "reason": "Coverage transcription of the vertical caption writer with an explicit Buffer receiver; the original is thiscall, so the free spelling notes the callee-pop divergence."}
// symbol    _sub_5db450@12
// TRIED: the fallback stack-only signature has the wrong arity - the real function reads ecx as a Buffer receiver (rect1_ at 0x20, font1_ at 0x52c, both matching Buffer's already-catalogued layout), then a text pointer and a RECT pointer on the stack. It draws each character of the text into a one-byte local buffer and calls write_cent_l once per character, advancing y by the font's line height each time - a vertical column of single-character lines, not a normal horizontal string write. Landed as a free stdcall function taking the receiver explicitly, rather than a Buffer member, because a new top-level type declared here to carry the method would truncate the region this file's own extractor reads at that type's own closing brace, before the real definition.
Status: Coverage transcription - unclaimed body
*/
extern "C" int __stdcall sub_5db450(Buffer *self, char *text, RECT *rect) {
    if (rect == 0) {
        return 0;
    }
    RECT saved = *reinterpret_cast<RECT *>(
        reinterpret_cast<char *>(self) + 0x30);
    self->set_clip(rect);
    const size_t len = strlen(text);

    Font **font_slot = reinterpret_cast<Font **>(
        reinterpret_cast<char *>(self) + 0x52C);
    if (*font_slot == 0) {
        *font_slot = FontDefault;
    }
    // Font's private advance block: unk_1_ at +0, line_height_ at +0xC,
    // height_ at +0x10. line_advance() spells the selection; the shrink
    // loop still compares against height_ itself.
    int line_height = (*font_slot)->line_advance();
    const int *font_words = reinterpret_cast<const int *>(*font_slot);
    int total = line_height * static_cast<int>(len);
    for (;;) {
        if (*font_slot == 0) {
            *font_slot = FontDefault;
        }
        font_words = reinterpret_cast<const int *>(*font_slot);
        if (line_height <= font_words[4] || total <= rect->bottom - rect->top) {
            break;
        }
        line_height = line_height - 1;
        total = total - static_cast<int>(len);
    }

    int y = rect->top;
    const int x = rect->left;
    const int width = rect->right - rect->left;
    char ch = *text;
    while (ch != 0) {
        ch = *text;
        text = text + 1;
        char onechar[2];
        onechar[0] = ch;
        onechar[1] = 0;
        const size_t one_len = strlen(onechar);
        self->write_cent_l(onechar, x, y, width, static_cast<int>(one_len));
        y = y + line_height;
        ch = *text;
    }

    self->set_clip(&saved);
    return y;
}

/*
Purpose: Wrap and centre text inside the flying-text block: measure the
         running line against the available width, emit each completed line
         centred, advance the pen by one line height, and keep the trailing
         partial line in the accumulator.
// ORIGINAL: 0x005DE320 sub_5de320 0x005DE320-0x005DE4C5 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Original5DE320..5DE4C5; retired seed git:b18d3ac8:src/unrecovered/005de320.cpp records the same identity and spans with its receiver TRIED note kept below.", "The flying-text state is buffer.cpp's own named binding block: FlyingTextBuf 0x009B33CC, FlyingTextY 0x009B3A94, FlyingTextHeight 0x009B3A98, with temp.h's BufferStrHeight (0x009B3A9C) as the running width accumulator; the bindings are redeclared here with the same names and addresses because they are TU-private there.", "The two one-space strings are 0x00682820 (socialwin.cpp's SpaceText) and 0x00696CE0, both measured as single-space .rdata in the pinned image beside the 0x00696CD8/CDC pair buffer.cpp's census already records."], "reason": "Coverage transcription of the flying-text wrapper with an explicit Buffer receiver; the original is thiscall, so the free spelling notes the callee-pop divergence."}
// symbol    @sub_5de320@8
// TRIED: MISMATCH #2 push/sub - the disassembly reads no stack args and takes the receiver in ecx (no [ecx+N] though; it is only used to call Buffer methods), so this is modelled as a Buffer __thiscall member instead of the placeholder's free __stdcall function; stack-frame shape still differs after that.
Status: Coverage transcription - unclaimed body
*/
static int *const FlyingTextBuf = (int *)0x009B33CC;
static int *const FlyingTextX = (int *)0x009B3A90;
static int *const FlyingTextY = (int *)0x009B3A94;
static int *const FlyingTextHeight = (int *)0x009B3A98;
// 0x00682820, socialwin.cpp's SpaceText: the one-space join string.
static int *const SpaceText = (int *)0x00682820;
// 0x00696CE0, measured " " in the pinned image; the census in buffer.cpp
// records its one-space neighbours 0x00696CD8/0x00696CDC. socialwin.cpp's
// SpaceText names the other space string this family joins with.
static int *const WrapFlyingSpace = (int *)0x00696CE0;
extern "C" int __fastcall sub_5de320(Buffer *self, const char *a1) {
    if (*FlyingTextHeight == 0) {
        return 0;
    }
    if (a1 == 0) {
        return *FlyingTextY;
    }
    if (*a1 == 0) {
        return *FlyingTextY;
    }

    const char *text = a1;
    while (true) {
        if (*reinterpret_cast<char *>(FlyingTextBuf) != 0) {
            strcat(reinterpret_cast<char *>(FlyingTextBuf),
                   reinterpret_cast<char *>(SpaceText));
            const unsigned int space_len =
                strlen(reinterpret_cast<char *>(WrapFlyingSpace));
            const int space_width = self->text_width(
                reinterpret_cast<char *>(WrapFlyingSpace),
                static_cast<int>(space_len));
            BufferStrHeight = BufferStrHeight + space_width;
        }
        const char *next = text;
        if (BufferStrHeight < *FlyingTextHeight) {
            const unsigned int len = strlen(text);
            int room = *FlyingTextHeight - BufferStrHeight;
            next = self->find_line_break_l(const_cast<char *>(text), &room,
                                           static_cast<int>(len));
        }
        if (next == 0) {
            break;
        }
        const unsigned int prefix_len =
            strlen(reinterpret_cast<char *>(FlyingTextBuf));
        memcpy(reinterpret_cast<char *>(FlyingTextBuf) + prefix_len, text,
               static_cast<unsigned int>(next - text));
        (reinterpret_cast<char *>(FlyingTextBuf) + prefix_len)[next - text] = 0;

        const int w = *FlyingTextHeight;
        const int h = *FlyingTextY;
        const int v = *FlyingTextX;
        const unsigned int total_len =
            strlen(reinterpret_cast<char *>(FlyingTextBuf));
        self->write_cent_l(reinterpret_cast<char *>(FlyingTextBuf), v, h, w,
                           static_cast<int>(total_len));

        *reinterpret_cast<char *>(FlyingTextBuf) = 0;
        BufferStrHeight = 0;

        // Buffer's protected line counter, field_18_ at +0x18.
        *reinterpret_cast<int *>(reinterpret_cast<char *>(self) + 0x18) =
            *reinterpret_cast<int *>(reinterpret_cast<char *>(self) + 0x18) + 1;
        *FlyingTextY = *FlyingTextY + self->text_line_height();

        text = next;
    }

    strcat(reinterpret_cast<char *>(FlyingTextBuf), const_cast<char *>(text));
    return *FlyingTextY;
}

/*
Purpose: Release the object parked in the receiver's +0x3C slot by
         dispatching its vtable slot 37 and returning its verdict.
// ORIGINAL: 0x004C8910 sub_4c8910 0x004C8910-0x004C891F FILE
// RECOVERY_BINDING: v1 {"evidence": ["Original4C8910..4C891F; retired seed git:b18d3ac8:src/unrecovered/004c8910.cpp records the same identity and spans.", "The body reads one receiver field and makes one indirect dispatch; the slot-37 call passes the object itself as the receiver with no stack arguments, spelled here through a __fastcall function pointer so the object travels in ecx exactly as the original thiscall dispatch does."], "reason": "Coverage transcription of the single-slot release forwarder with the receiver as an explicit first argument; no in-tree class owns the receiver, so the slot stays a documented vtable index."}
// symbol    @sub_4c8910@4
// TRIED: the tail-called vtable slot37 matches exactly (mov/test/je/mov/jmp, 5/5 mnemonics), so the divergence is entirely the null branch: the original's `return -1` JUMPS OUT of this function into a shared tail block at 0x4C891F, four bytes past its own 15, and no C++ spelling reaches another function's interior. This body inlines `or eax,-1; ret` instead and is 4 bytes long. Not a lever - it names why the remainder cannot close from here.
Status: Coverage transcription - unclaimed body
*/
extern "C" int __fastcall sub_4c8910(char *self) {
    void *const target = *reinterpret_cast<void **>(self + 0x3C);
    if (target != 0) {
        typedef int (*ReleaseSlot)(void *);
        ReleaseSlot *const vtable =
            *reinterpret_cast<ReleaseSlot **>(target);
        return vtable[37](target);
    }
    return -1;
}

/*
Purpose: Copy one text record's string into fresh storage chosen by the
         record's own allocator (the CRT heap when none is installed) and
         hand the copy plus the four metadata words to the caller's array.
// ORIGINAL: 0x005D47D0 sub_5d47d0 0x005D47D0-0x005D4851 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Original5D47D0..5D4851; retired seed git:b18d3ac8:src/unrecovered/005d47d0.cpp records the same identity and spans.", "The receiver is the +0x18 Heap pointer, +0x1C string and +0x20 metadata record the sibling 0x005D4C50 installs into; mem_get is general.cpp's and Heap::get is heap.h's own, both already linked by this TU's list machinery."], "reason": "Coverage transcription of the record copy-out with the thiscall receiver spelled as an explicit first argument; no in-tree class owns the record yet, so its slots stay documented offsets."}
// symbol    @sub_5d47d0@8
Status: Coverage transcription - unclaimed body
*/
extern "C" int __fastcall sub_5d47d0(char *self, int *out) {
    char *const str = *reinterpret_cast<char **>(self + 0x1C);
    if (str != 0) {
        Heap *const heap = *reinterpret_cast<Heap **>(self + 0x18);
        char *dest;
        if (heap == 0) {
            dest = static_cast<char *>(mem_get(strlen(str) + 1));
        } else {
            dest = static_cast<char *>(heap->get(strlen(str) + 1));
        }
        out[1] = reinterpret_cast<int>(dest);
        if (dest == 0) {
            return 4;
        }
        *dest = 0;
        strcat(dest, str);
    }
    int *const src = *reinterpret_cast<int **>(self + 0x20);
    out[2] = src[0];
    out[3] = src[1];
    out[4] = src[2];
    out[5] = src[3];
    return 0;
}

/*
Purpose: Copy one text record's string into fresh storage chosen by the
         record's own allocator and park the copy in the caller's output
         block; the metadata half is left to the caller.
// ORIGINAL: 0x005D4C50 sub_5d4c50 0x005D4C50-0x005D4CBC FILE
// RECOVERY_BINDING: v1 {"evidence": ["Original5D4C50..5D4CBC; retired seed git:b18d3ac8:src/unrecovered/005d4c50.cpp records the same identity and spans.", "Same +0x18/+0x1C receiver record as sub_5d47d0 above, with mem_get and Heap::get resolving exactly as there; the seed's function-pointer CRT spellings are ordinary strlen/strcat calls here."], "reason": "Coverage transcription of the record copy-out's string half with the receiver as an explicit first argument."}
// symbol    @sub_5d4c50@8
Status: Coverage transcription - unclaimed body
*/
extern "C" int __fastcall sub_5d4c50(char *self, int out) {
    char *const block = reinterpret_cast<char *>(out);
    *reinterpret_cast<int *>(block + 8) = 0;
    char *const str = *reinterpret_cast<char **>(self + 0x1C);
    if (str != 0) {
        void *result;
        if (*reinterpret_cast<int *>(self + 0x18) == 0) {
            result = mem_get(strlen(str) + 1);
        } else {
            result = reinterpret_cast<Heap *>(*reinterpret_cast<int *>(self + 0x18))
                ->get(strlen(str) + 1);
        }
        *reinterpret_cast<void **>(block + 4) = result;
        if (result == 0) {
            return 4;
        }
        *static_cast<char *>(result) = 0;
        strcat(static_cast<char *>(*reinterpret_cast<char **>(block + 4)), str);
    }
    return 0;
}

/*
Purpose: Sort one pass of the string list: walk the ring from the current
         position, select the smallest record under the three-way record
         comparator, and swap the winning entry's payload text and metadata
         into the walk position.
// ORIGINAL: 0x005D49D0 sub_5d49d0 0x005D49D0-0x005D4BA7 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Original5D49D0..5D4BA7; retired seed git:b18d3ac8:src/unrecovered/005d49d0.cpp records the same identity and spans.", "The receiver fields at +8/+C/+10/+14 are StringStruct's own head_/current_/entry_count_/current_position_ and the body calls StringStruct::seek_pos, so the receiver is typed as the list itself; the comparator is sub_5d4bb0 above, and the node walks use StringStructEntry's next(+0xC)/previous(+0x10) exactly as StringStruct::add installs them.", "The swapped payload words at +4/+8 are the payload's text pointer and first metadata word; the walk re-reads entry_count_ every iteration exactly as the seed measured."], "reason": "Coverage transcription of the list's selection pass on the typed StringStruct receiver; the payload internals stay documented offsets."}
// symbol    @sub_5d49d0@4
Status: Coverage transcription - unclaimed body
*/
extern "C" void __fastcall sub_5d49d0(StringStruct *self) {
    // RESTRUCTURE NOTE. The seed expressed its exits as `goto LAB_join` and
    // `goto LAB_tail`; those are spelled here as the `reached_end` and
    // `swap_out_of_range` flags so the walk stays one structured loop.
    //
    // The seed's else-branch, negative-index and swap-out-of-range paths
    // are unreachable from this loop's invariants: the do/while is entered
    // only with limit = entry_count_ - 1 >= 1 and continues only while
    // walk_index < limit, so walk_index >= 0 on every iteration and the
    // best_index swap target can only be a position the forward scan
    // produced, where *position >= 0 because positions start at zero and
    // only advance. On the reachable paths - non-negative walk_index and an
    // in-range winner - the flags never latch, the seek and scan blocks run
    // exactly when the seed's fall-through runs them, and the swapped
    // words are the same id/payload pair the seed's xor chains exchange.
    StringStructEntry **const head = &self->head_;
    StringStructEntry **const current = &self->current_;
    int *const count = &self->entry_count_;
    int *const position = &self->current_position_;

    // The comparator's record starts twelve bytes into the payload node.
    const int record_of = 0xC;

    int total = *count;
    int walk_index = 0;
    int limit = total - 1;
    if (limit <= 0) {
        return;
    }

    int scan_index = walk_index + 1;
    do {
        bool reached_end = false;
        if (walk_index <= limit) {
            *current = *head;
            if (walk_index < 0) {
                total = *count;
                int steps = abs(walk_index);
                if (total < steps) {
                    reached_end = true;
                } else {
                    steps = abs(walk_index);
                    if (steps > 0) {
                        do {
                            --steps;
                            *current = (*current)->previous;
                        } while (steps != 0);
                    }
                    *position = *count + walk_index;
                }
            } else {
                *position = walk_index;
                if (walk_index > 0) {
                    int steps = walk_index;
                    do {
                        --steps;
                        *current = (*current)->next;
                    } while (steps != 0);
                }
            }
        } else {
            reached_end = true;
        }

        int best_record;
        if (*head == 0) {
            best_record = 0;
        } else {
            best_record = reinterpret_cast<int>((*current)->payload)
                + record_of;
        }
        int best_index = walk_index;
        int scan_cursor = scan_index;
        if (!reached_end && scan_index < total) {
            do {
                if (*head != 0) {
                    const int next_position = *position + 1;
                    *current = (*current)->next;
                    *position = next_position;
                    if (next_position == *count) {
                        *position = 0;
                    }
                }
                int scan_record;
                if (*current == 0) {
                    scan_record = 0;
                } else if (*head == 0) {
                    scan_record = 0xC;
                } else {
                    scan_record = reinterpret_cast<int>((*current)->payload)
                        + record_of;
                }
                if (sub_5d4bb0(scan_record, best_record) < 0) {
                    if (*head == 0) {
                        best_record = 0;
                    } else {
                        best_record =
                            reinterpret_cast<int>((*current)->payload)
                            + record_of;
                    }
                    best_index = *position;
                }
                total = *count;
                ++scan_cursor;
            } while (scan_cursor < total);

            if (best_index != walk_index && best_index <= total - 1) {
                *current = *head;
                bool swap_out_of_range = false;
                if (best_index < 0) {
                    int steps = abs(best_index);
                    if (*count < steps) {
                        swap_out_of_range = true;
                    } else {
                        steps = abs(best_index);
                        if (steps > 0) {
                            do {
                                --steps;
                                *current = (*current)->previous;
                            } while (steps != 0);
                        }
                        best_index = best_index + *count;
                    }
                } else if (best_index > 0) {
                    int steps = best_index;
                    do {
                        --steps;
                        *current = (*current)->next;
                    } while (steps != 0);
                }
                if (!swap_out_of_range) {
                    StringStructEntry *const winner = *current;
                    *position = best_index;
                    if (self->seek_pos(walk_index) != 0) {
                        StringStructEntry *const walked = *current;
                        // The original juggles the two words through xor
                        // chains; the effect is a plain swap of each
                        // entry's id and payload.
                        const int saved_id = winner->id;
                        StringStructPayload *const saved_payload =
                            winner->payload;
                        winner->id = walked->id;
                        winner->payload = walked->payload;
                        walked->id = saved_id;
                        walked->payload = saved_payload;
                    }
                }
            }
        }

        total = *count;
        limit = total - 1;
        walk_index = scan_index;
        scan_index = scan_index + 1;
    } while (walk_index < limit);
}

/*
Purpose: Re-initialise a graphic window in place: close the live window,
         install the process frame defaults when the caller asks for a frame,
         run the base init and then size the window's own drawing surface,
         syncing it to the active palette.
// ORIGINAL: 0x005D50F0 sub_5d50f0 0x005D50F0-0x005D5235 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Original5D50F0..5D5235; retired seed git:b18d3ac8:src/unrecovered/005d50f0.cpp records the same identity and spans with its receiver PROPOSAL/TRIED notes kept below.", "The receiver runs GraphicWin::close, GraphicWin::compute_min_size, Win::UNK1 (win.cpp's own stub), Win::nonclient_to_client and the Buffer base's init/sync_to_palette, all linked members; the eleven sprite stores are GraphicWinInitDefaults (0x009B3394) into GraphicWin's private frame block 0x9CC..0x9F4, matching graphicwin.h's own frame model.", "0x009B8DD4 is scroll.cpp's ScrollDefaultThickness and 0x009B8180 is palette.h's PaletteActive; the Buffer subobject is the +0x444 base."], "reason": "Coverage transcription of the in-place re-init with a typed GraphicWin receiver; the private frame block and the style word stay documented offsets."}
// symbol    @sub_5d50f0@40
// PROPOSAL (superseded by this coverage spelling): extern "C" int __stdcall sub_5d50f0(9 ints) -> a __thiscall member (9 forwarded ints, no separate `this`), evidence: `mov esi, ecx` at entry immediately used as the receiver for GraphicWin::close()/compute_min_size(), and `ret 0x24` (36 bytes = 9 ints) is consistent with either reading, but the raw `ecx` use at entry only makes sense for thiscall/fastcall - a free function declared `__thiscall` is refused outright by VC6 (C4234), same as a `__thiscall` function-pointer type. Expressed via an introduced `GWShim` class per the __thiscall carve-out, since the scaffold's own GraphicWin class does not declare this member. The class-close semicolon and the out-of-line definition's open brace share ONE LINE below - not style, the extractor's brace counter runs per line, and a standalone closing line for the class closes the region it is measuring before reaching the function at all, scoring the whole thing NO_COMPILE with zero .text symbols found.
// TRIED: reading `*(uint*)(base+0x98)` twice (once per bit test) added an extra `mov` vs the original's single load reused by both `test al,N`; caching it in a local did not change the rebuilt bytes, so the remaining divergence is scheduling, not this. Best reached: MISMATCH, edit_count 5, 345 bytes vs 325, mnemonic similarity ~0.92.
Status: Coverage transcription - unclaimed body
*/
extern "C" int __fastcall sub_5d50f0(GraphicWin *self, int a1, int a2, int a3,
                                     int a4, int a5, int a6, int a7, int a8,
                                     int a9) {
    char *const base = reinterpret_cast<char *>(self);

    self->close();
    if ((a6 & 0x30000000) != 0) {
        const GraphicWinFrameDefaults &defaults = GraphicWinInitDefaults;
        *reinterpret_cast<Sprite **>(base + 0x9CC) = defaults.top_left;
        *reinterpret_cast<Sprite **>(base + 0x9D4) = defaults.top_right;
        *reinterpret_cast<Sprite **>(base + 0x9D0) = defaults.bottom_left;
        *reinterpret_cast<Sprite **>(base + 0x9D8) = defaults.bottom_right;
        *reinterpret_cast<Sprite **>(base + 0x9E8) = defaults.top;
        *reinterpret_cast<Sprite **>(base + 0x9E4) = defaults.bottom;
        *reinterpret_cast<Sprite **>(base + 0x9DC) = defaults.left;
        *reinterpret_cast<Sprite **>(base + 0x9E0) = defaults.right;
        *reinterpret_cast<Sprite **>(base + 0x9EC) = defaults.noncaption_right;
        *reinterpret_cast<Sprite **>(base + 0x9F0) = defaults.noncaption_left;
        *reinterpret_cast<Sprite **>(base + 0x9F4) = defaults.noncaption_middle;
    }

    int result = static_cast<Win *>(self)->UNK1(a1, a2, a3, a4, a5, a6, a7,
                                                a8, a9);
    if (result == 0) {
        // GraphicWin's owned-buffer self-reference at +0x448.
        *reinterpret_cast<int *>(base + 0x448) = reinterpret_cast<int>(base);
        self->compute_min_size();
        if ((a6 & 0x800) == 0) {
            static_cast<Win *>(self)->nonclient_to_client(&a3, &a4);
        } else {
            const unsigned int style =
                *reinterpret_cast<unsigned int *>(base + 0x98);
            if ((style & 8) != 0) {
                a3 = a3 + ScrollDefaultThickness;
            }
            if ((style & 4) != 0) {
                a4 = a4 + ScrollDefaultThickness;
            }
        }
        Buffer *const canvas = reinterpret_cast<Buffer *>(base + 0x444);
        result = canvas->init(a3, a4, 0, 0);
        if (result == 0) {
            canvas->sync_to_palette(PaletteActive);
            result = 0;
        }
    }
    return result;
}
