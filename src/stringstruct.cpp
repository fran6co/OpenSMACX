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

// ORIGINAL: 0x00402C20 ??_GStringStructEntry@@UAEPAXI@Z 0x00402C20-0x00402C62 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Original table installed by 401100 construction dispatches deletion through StringStructEntry allocation base at +0x14. This original body publishes saved Heap owner, conditionally frees the complete allocation and clears owner on deleting paths. Same-span seed retired; the complete original identity and spans remain on this compiled owner.", "Original construction at 401100 and typed deletion at 402970 establish the owning node lifecycle. The canonical source host is stringstruct.cpp; the compiler owns the virtual-base adjustment and deleting-flag entry.", "git:293937ab6f3be75999e0c0a4c9c110c02930624c:src/recovered/units/00402c20.cpp", "Historical seed spelling sub_402c20; current source mapping ??_GStringStructEntry@@UAEPAXI@Z; emitted symbol ??_GStringStructEntry@@UAEPAXI@Z."], "reason": "Map the compiler-generated scalar deleting destructor emitted by actual list-node construction, not a handwritten wrapper. Superseded seed retired after owner review; original spans, existing claims and useful seed findings remain in source. This cleanup grants no new match or acceptance claim."}
// ORIGINAL: 0x00402C70 ??_GStringStructPayload@@UAEPAXI@Z 0x00402C70-0x00402CB2 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Original table installed by 401100 construction dispatches deletion through StringStructPayload allocation base at +0x10. This original body publishes saved Heap owner, conditionally frees the complete allocation and clears owner on deleting paths. Same-span seed retired; the complete original identity and spans remain on this compiled owner.", "Original construction at 401100 and typed deletion at 402970 establish the owning node lifecycle. The canonical source host is stringstruct.cpp; the compiler owns the virtual-base adjustment and deleting-flag entry.", "git:293937ab6f3be75999e0c0a4c9c110c02930624c:src/recovered/units/00402c70.cpp", "Historical seed spelling sub_402c70; current source mapping ??_GStringStructPayload@@UAEPAXI@Z; emitted symbol ??_GStringStructPayload@@UAEPAXI@Z."], "reason": "Map the compiler-generated scalar deleting destructor emitted by actual list-node construction, not a handwritten wrapper. Superseded seed retired after owner review; original spans, existing claims and useful seed findings remain in source. This cleanup grants no new match or acceptance claim."}
// ORIGINAL: 0x004029F0 ??1StringStruct@@QAE@H@Z 0x004029F0-0x00402A56 FILE
// RECOVERY_BINDING: v1 {"evidence": ["task:stringstruct-lifecycle-coverage", "Pinned original0x4029f0", "historical seed:src/recovered/units/004029f0.cpp", "Real virtual-base construction and physical compiler-generated lifecycle symbols."], "reason": "Original allocation-base receiver adjustment, list teardown and owner-aware conditional deallocation belong to the real compiler-generated scalar deleting destructor. Source virtual inheritance, destructor and inherited operator delete supply lifetime; catalogue destructor spelling remains historical. No handwritten adjustment or assembly."}
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
// ORIGINAL: 0x00402310 sub_402310 0x00402310-0x0040231F BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:allocation-base-destructor-owner", "Original 0x00402310..0x0040231F reads the captured Heap at this+4, stages allocation-base table 0x006693AC and publishes owner 0x009B3374; StringAllocationBase and StringAllocationHeap own these relationships.", "historical seed:src/recovered/00402310.cpp"], "reason": "The retained synthetic owner-publication seed is the existing genuine allocation-base destructor. Real member/global and compiler-owned destructor table stage replace fixed executable addresses; retain original identity and historical exact obligation."}
// symbol    ??1StringAllocationBase@@UAE@XZ
StringAllocationBase::~StringAllocationBase() {
    StringAllocationHeap = allocation_owner_;
}

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
// Superseded seed deleted; seed paths below are historical Git provenance.
// Historical seed observations (not fresh measurements or acceptance):
// TRIED: alloc_entry_node/alloc_id_node/report_fatal helper functions factoring the repeated Heap::get-or-mem_get pattern; introduces real CALLs the inlined original does not have, diverges at insn #4
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
        on_insert(head_->string_payload());
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
        on_insert(current_->string_payload());
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
            ListPayloadBase *const payload = entry->payload;
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
StringStructPayload *StringStruct::current_entry() {
    if (head_) {
        return current_->string_payload();
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
            if (current_->id == id) {
                return 1;
            }
            current_position_++;
            current_ = current_->next;
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
    // The historical offset-based access rationale above is superseded by
    // narrow friendship with the existing owners, not by a guessed layout.
    // Matching obligation: the pinned climate-window exit at 4554CA leaves
    // EAX unspecified. This transcription's zero result is not proof of the
    // original return ABI; redraw_nc_buffer ignores the callback result.
    if (window == &WorldClimateWorldWin || text == 0) {
        return 0;
    }

    // Pinned4554E6 copies Buffer+0x30 before reading the two frame sprites.
    RECT *const canvas_rect = &canvas->rect1_;
    RECT area = *canvas_rect;
    area.right -= 0x28;
    area.left += 0x23 + window->frame_left_->left_offset();
    area.top += 0xB + window->frame_top_->top_offset();
    area.bottom -= 0xC;

    canvas->set_text_color(window->field_104_, window->field_108_,
                           window->field_10C_, window->field_110_);
    canvas->set_font(window->field_F8_, 0, 0, 0);
    canvas->set_clip(&area);

    const unsigned int style = window->iFlags_;
    const unsigned int len = strlen(text);
    if ((style & 0x10000) != 0) {
        canvas->write_l(text, &area, len);
    } else if ((style & 0x40000) != 0) {
        canvas->write_right_l(text, &area, len);
    } else {
        canvas->write_cent_l(text, &area, len);
    }
    // The original final calls pass live Buffer+0x30, not a saved rectangle.
    // Preserve that alias even when setting the caption clip changed rect1_.
    return canvas->set_clip(canvas_rect);
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
Purpose: The third pronoun fetcher beside game.cpp's get_his_her/get_him_her:
         pick the he/she label by the faction leader's gender, copy the string
         table's text into the shared pronoun buffer, and uppercase it in
         place when the sentence form asks.

// ORIGINAL: 0x0058FCE0 ?get_he_she@@YAPADHH@Z 0x0058FCE0-0x0058FD6B FILE
// size      139 bytes
// prototype char* (__cdecl ?get_he_she@@YAPADHH@Z)(int, int)
// callers   6   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x006169A0 0x00645460
// indirect  0x0058FD43 0x0058FD5E
// TRIED (seed): caching the CharUpperA pointer in a local vs calling through
//         the import slot at each site - both land 18 bytes over (157-160 vs
//         139), extra push/push in the prologue before the a1*0x167 multiply;
//         root cause not found. The fixed-address bindings are renamed to the
//         tree's own models: Players (faction.h), Labels (alpha.h),
//         StringTable (strings.h), the CharUpperA import slot (windows.h) and
//         0x00945838 as PronounBuffer (game.h) - the same census-named buffer
//         get_his_her and get_him_her fill.
Status: Coverage transcription - unclaimed body
*/
char *__cdecl get_he_she(int faction_id, int sentence_form) {
    const int code = 0xd1 + (Players[faction_id].is_leader_female != 0);
    LPSTR *labels = Labels.strings_ptr;
    strcpy(PronounBuffer, StringTable.get(labels[code]));
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

// ===========================================================================
// Coverage-first transcriptions (wave 3, journal w3-stringstruct2-scope).
// Each entry lands a real compiling body for a seed the census assigned to
// this TU by marker range (the [0x00401060, 0x00608770] span of this file's
// ORIGINAL markers, the same rule that gave wave 1 its sub-200-line queue),
// retiring the seed in the same change. These are unclaimed FILE mappings:
// no byte claim is made or implied, and no spelling iteration was performed
// in this pass. Provenance for every retired seed is
// git:117282ee:src/unrecovered/<addr>.cpp.
// ===========================================================================

#include "sprite.h"    // Sprite - the surface sprite the unit wrappers draw
#include "veh.h"       // Vehs - the deletion list walks its 0x34-stride records
#include "lock.h"      // LockEnableMask - the deletion-record occupancy mask
#include "log.h"       // log_say - both deletion paths report through it
#include "fx.h"        // FX - play is defined in this wave
#include "deletionlist.h"  // DeletionList conversion and deletion enqueue
#include "pulldown.h"   // PullDown - the menu handlers are defined in this wave
#include "reportwin.h"  // ReportWin - on_redraw is defined in this wave
#include "diplopop.h"   // DiploPop - on_redraw is defined in this wave
#include "mapwin.h"    // MapWin + MainInterfaceGlobal (0x007AE820)
#include "map.h"       // MapIsFlat / MapLongitudeBounds - the cursor wrap
#include "maininterface.h"  // MainInterface::restore_back - DiploPop's opener

namespace {
// The .rdata strings the deletion paths hand to log_say, read out of the
// image beside the bindings the same family already carries in veh.cpp.
static char *const StrDeletingVehicle = (char *)0x0068C134;    // "Deleting vehicle"
static char *const StrDelConvertedIndex = (char *)0x0068C120;  // "DelConverted index"

// The one-space join string this file's flying-text wrapper already binds as
// SpaceText, and the caret separator MultiWin's spot label builds from.

// The local faction at 0x00939284 is ConsoleGlobal.faction_id_:
// source-owned ConsoleGlobal at 0x009156B0 plus member offset 0x23BD4.

// The spot label is built in strings.h's StringTemp (0x009B86A0), the same
// shared scratch the sibling alpha.cpp prefs paths fill.

}

// Buffer owns its shared surface Sprite at 0x009B3A50; this unit does not
// access that object directly. Sprite owns the tech-wizard facility icons.
extern Sprite g_TECH_ICON_SPRITES[89];  // 0x00759E28, sprite.cpp's definition

namespace {
// The published unit draw-state block: the words the surface sprite's
// draw_dest_unk1 readers consume, 0x009B3A54..0x009B3A78.

}

// ---------------------------------------------------------------------------
// FX::play, 0x00446A00. The bank's sound trigger: gate on the sound bit of
// GamePreferences, re-arm the popup wave owner for the first three effect
// kinds, then release/restart the effect record. fx.h declares the member;
// its forwarder retires from pending_bodies.cpp in this change.

// Interlude members and compiler-generated lifetime code are owned by interlude.cpp.

// Media overlay update is owned by MediaRuntime in movie_media.cpp.


// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
// tech_wizard_unk2, 0x005BE2A0. The tech wizard's facility sprite painter:
// convert the page's structure size into the isometric x/y offsets, publish
// the drawing scale pair, then draw the facility sprite with its
// transparency byte.
/*
Purpose: Draw one tech-wizard facility sprite at the isometric position its
         record's footprint implies.
// ORIGINAL: 0x005BE2A0 tech_wizard_unk2 0x005BE2A0-0x005BE378 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Original5BE2A0..5BE378; retired seed git:117282ee:src/unrecovered/005be2a0.cpp records the same identity and spans with its register-save TRIED note kept below.", "The scale pair is sprite.h's own SpriteScaleNumerator/Denominator (0x00696D18/0x00696D1C), the size/divisor pair is mapwin.h's MapWinCaptionScale/MapWinCaptionDivisor (0x00691E6C/0x00691E70), and the facility tables are sprite.cpp's g_TECH_ICON_SPRITES (0x00759E28, whose 0x2C stride is the seed's 0x11-int entry) with the transparency bytes at 0x00759E30 - all read exactly as the seed measures them.", "The sprite draw is sprite.h's own Sprite::draw(Buffer *, int, int, int)."], "reason": "Coverage transcription of the facility sprite painter with the record-table arithmetic kept at the seed's measured strides."}
// TRIED (seed): 6-param __cdecl void, if/else matching Ghidra's arithmetic
//         structure reaches 0.84 mnemonic similarity; the original saves
//         ebx/esi before the al&1 test while the scaffold defers it.
Status: Coverage transcription - unclaimed body
*/
// The scale pair and size pair are the tree's own bindings: sprite.h's
// SpriteScaleNumerator/Denominator (0x00696D18/0x696D1C) and mapwin.h's
// MapWinCaptionScale/MapWinCaptionDivisor (0x00691E6C/0x00691E70). The
// facility tables are sprite.cpp's g_TECH_ICON_SPRITES (0x00759E28, 0x2c
// stride); each Sprite owns its transparency byte at +8 (first 0x00759E30).

extern "C" void __cdecl tech_wizard_unk2(int canvas, int index, int odd,
                                         int base_x, int base_y, int base_bottom) {
    if (index < 0x59) {
        const int size = MapWinCaptionScale;
        const int divisor = MapWinCaptionDivisor;
        int x_offset;
        if ((odd & 1) == 0) {
            x_offset = 0;
        } else if (size <= 0) {
            x_offset = 0x6C;
        } else {
            x_offset = (size * 0xD9) / divisor;
        }
        const int x = base_x + x_offset;

        int y_offset;
        if (size <= 0) {
            y_offset = 0x6B;
        } else {
            y_offset = (size * 0xD7) / divisor;
        }
        const int dy = base_bottom - y_offset;
        const int y = base_y + (dy / 2);

        if (size > 0) {
            SpriteScaleNumerator = size;
            SpriteScaleDenominator = divisor;
        } else {
            SpriteScaleNumerator = 8;
            SpriteScaleDenominator = 0x10;
        }

        Buffer *const target = (canvas != 0)
            ? reinterpret_cast<Buffer *>(canvas + 0x444)
            : reinterpret_cast<Buffer *>(0);

        const int entry = index * 11;
        const int sprite_index = (entry * 4) / 0x2C;
        // Pinned 5BE34F reads array base759E28 + element stride2C + member8.
        const unsigned char transparency =
            g_TECH_ICON_SPRITES[sprite_index].transparent_index();
        g_TECH_ICON_SPRITES[sprite_index].draw(target, transparency, x, y);

        SpriteScaleNumerator = 1;
        SpriteScaleDenominator = 1;
    }
}

// Original pointer cell 0x00691B18 selects the WIZARD text at 0x00691B94.
// Keep that pointer/text relationship in source-owned storage. Names are
// descriptive; bindings establish starts, not enclosing allocation bounds.
// RECOVERY_OBJECT: v1 0x00691B94 ?TechWizardPopupFileText@@3PADA
char TechWizardPopupFileText[7] = "WIZARD";
// RECOVERY_OBJECT: v1 0x00691B18 ?TechWizardPopupFile@@3PADA
char *TechWizardPopupFile = TechWizardPopupFileText;

// The seed scaffold says int 0; the negative edge only restores
// registers and the normal edge leaves Popup::exec immediately before
// RET. Keep that interpretation as seed history,
// but do not promise a return value without a caller/result consumer.
// ORIGINAL: 0x005BDE20 tech_wizard_unk1 0x005BDE20-0x005BE0F8 FILE
// RECOVERY_BINDING: v1 {"evidence":["task:tech-wizard-owner-oct5","Original 0x005BDE20-0x005BE0F8, 728-byte FILE body; complete original seed header/TRIED/call inventory retained beside this definition.","Pinned raw flow 0x5BDE20..0x5BE0F7: two negative guards; original popup stack arguments; technology lookup and icon calls; normal tail calls second Popup::exec before register/frame restore and RET.","Technology table 0x94F358 stride 0x2C; lookup fields growth +0x14, tech +0x18, wealth +0x1C. Native strict ranking: result1 when growth<wealth && wealth>tech; result0 when growth>wealth && growth>tech; else result2, after sum/equal-ID result9 path.","Popup receiver member evidence: caller_sprite_ +0x2144; Dialogs +0x21D0; SpriteBox +0x2240; SpriteList entry_count +0x2274.","Native pointer cell 0x691B18 selects WIZARD text at 0x691B94; neighboring TUTOR caption uses a distinct cell/text owner.","Seed catalog lists zero direct callers; mapped caller search does not establish absence of indirect callers. Five-argument cdecl stack shape and typed Popup* source parameters are provisional; no return-value contract is asserted."],"reason":"Recover the complete helper through existing typed source owners, preserving the pinned branches, calls, side-effect order and original historical evidence. The source home is a technology-helper grouping, not a claimed historical translation-unit boundary. No new match, direct-caller, or game-acceptance claim."}
// symbol    _tech_wizard_unk1
// TRIED: `void` return instead of `int 0`; splitting the two negative-parameter guards into separate `if`s instead of `||` - neither moved the divergence, which sits at #4 (`mov ebx,[ebp+0x10]` vs a hoisted `xor ebx,ebx`), already before the first call; likely register allocation pressure from the many later 0-argument pushes, not the guard shape. Control flow, field offsets (field_2144_, dialogs_+0x2274/+0x2240) and the lookupA/2+lookupB+lookupMid tie-break were hand-traced from the raw disasm, not Ghidra's decompile.
// size      728 bytes
// prototype
// callers   0   call targets   8
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00406380 0x005B9C40 0x00602600 0x006108E0 0x00625810 0x00625EC0 0x00645470 0x0064FC88
extern "C" void __cdecl tech_wizard_unk1(Popup *first_popup,
                                          Popup *second_popup,
                                          int first_tech, int second_tech,
                                          int force_roll) {
    if (first_tech < 0 || second_tech < 0) {
        return;
    }

    char number[0x50];

    StringTemp[0] = '\0';
    say_tech(StringTemp, first_tech, 0);
    parse_says(0, StringTemp, -1, -1);

    StringTemp[0] = '\0';
    say_tech(StringTemp, second_tech, 0);
    parse_says(1, StringTemp, -1, -1);

    int lookup_a = 1;
    int lookup_mid = 1;
    int lookup_b = 1;
    if (second_tech < MaxTechnologyNum) {
        lookup_a = Technology[second_tech].tech_value;
        lookup_mid = Technology[second_tech].wealth_value;
        lookup_b = Technology[second_tech].growth_value;
    }

    first_popup->start(TechWizardPopupFile, "TECHMIL", -1, 0, 0x100, 0);

    if (first_tech < MaxTechnologyNum) {
        Sprite *const icon = &g_TECH_ICON_SPRITES[first_tech];
        if (first_popup->dialogs().SpriteBox::list_.entry_count_ == 0)
            first_popup->set_caller_picture(icon);
        else
            first_popup->set_caller_picture(0);
        first_popup->dialogs().sprite(icon, 0, 0);
    }

    StringTemp[0] = '\0';
    strcat(StringTemp, "MIL");
    const int roll = force_roll != 0 ? 3 : static_cast<int>(random(0, 3));
    _itoa(roll, number, 10);
    strcat(StringTemp, number);
    first_popup->start(TechWizardPopupFile, StringTemp, -1, 0, 0x80, 0);
    first_popup->exec(0, 0);

    second_popup->start(TechWizardPopupFile, "TECHINF", -1, 0, 0x100, 0);

    if (second_tech < MaxTechnologyNum) {
        Sprite *const icon = &g_TECH_ICON_SPRITES[second_tech];
        if (second_popup->dialogs().SpriteBox::list_.entry_count_ == 0)
            second_popup->set_caller_picture(icon);
        else
            second_popup->set_caller_picture(0);
        second_popup->dialogs().sprite(icon, 0, 0);
    }

    StringTemp[0] = '\0';
    strcat(StringTemp, "INF");

    int result;
    if (lookup_a / 2 + lookup_b + lookup_mid < 1 ||
        second_tech == first_tech) {
        result = 9;
    } else if (force_roll == 0) {
        if (lookup_b < lookup_mid && lookup_a < lookup_mid) {
            result = 1;
        } else if (lookup_mid < lookup_b && lookup_a < lookup_b) {
            result = 0;
        } else {
            result = 2;
        }
    } else {
        result = 3;
    }

    _itoa(result, number, 10);
    strcat(StringTemp, number);
    second_popup->start(TechWizardPopupFile, StringTemp, -1, 0, 0x80, 0);
    second_popup->exec(0, 0);
}

// The old 447D20 binding in this file was a settings-checkbox/repaint seed
// hypothesis. The pinned body instead calls Midi's trackset status,
// transpose and play operations; the original identity now lives with Midi.
// Superseded RECOVERY_BINDING history retained verbatim:
// {"evidence": ["Original447D20..447DA5; retired seed git:117282ee:src/unrecovered/00447d20.cpp records the same identity and spans.", "The shared delay counter is 0x00683C2C, stepped by 0x12C and floored at -0x4B0 exactly as the seed walks it; the receiver's slots 67/69/70 are dispatched at their measured indices through member-fn-pointer unions because the object's class is unnamed in the catalogue."], "reason": "Coverage transcription of the settings cascade with the delay counter bound and the receiver's slots dispatched at their measured indices."}


// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// DeletionList::convert, 0x0052DDC0. Multiplayer id conversion: remap a
// vehicle id across the deletion list's occupied records, walking every
// record between the old and new owner and subtracting each queued id below
// the value; both paths report through log_say. deletionlist.h declares the
// member; its forwarder retires from pending_bodies.cpp in this change.
/*
Purpose: Convert a vehicle id across the deletion list's queued deletions.
// ORIGINAL: 0x0052DDC0 ?convert@DeletionList@@QAEHHH@Z 0x0052DDC0-0x0052DE65 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Original52DDC0..52DE65; retired seed git:117282ee:src/unrecovered/0052ddc0.cpp records the same identity and spans.", "The records are deletionlist.h's own 0x3C-stride entries with the occupancy mask lock.h's LockEnableMask (0x009A64E8), the owner count at +0x5A0 and the per-record id run at +0xA; the log_say format is the image's DelConverted string bound beside this body."], "reason": "Coverage transcription of the id conversion on the typed DeletionList receiver with the subtract-below walk exactly as the seed orders it."}
Status: Coverage transcription - unclaimed body
*/
int DeletionList::convert(int original_id, int new_owner) {
    int result = original_id;
    int search = new_owner + 1;
    const int total = current_owner_;
    if (search <= total) {
        const unsigned char mask = static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_);
        do {
            for (int slot = 0; slot < 0x18; slot++) {
                const DeletionRecord &record = records_[slot];
                if ((record.acknowledgements_ & mask) != mask
                    && record.owner_ == search) {
                    int count = record.count_;
                    if (count > 0) {
                        for (int index = 0; index < count; ++index) {
                            const int deleted_id = record.vehicle_ids_[index];
                            if (deleted_id == result) {
                                result = -1;
                            } else if (deleted_id < result) {
                                result--;
                            }
                        }
                    }
                    break;
                }
            }
            search++;
        } while (search <= total);
        if (original_id != result) {
            log_say(StrDelConvertedIndex, original_id, result, 0);
        }
    }
    return result;
}

// ---------------------------------------------------------------------------
// DeletionList::add_deletion, 0x0052DE70. Enqueue one multiplayer deletion:
// report the vehicle's position, find its record by owner (falling back to
// the first free record), and append the id to that record's run.
// deletionlist.h declares the member; its forwarder retires from
// pending_bodies.cpp in this change.
/*
Purpose: Queue one vehicle deletion against its owner's deletion record.
// ORIGINAL: 0x0052DE70 ?add_deletion@DeletionList@@QAEHH@Z 0x0052DE70-0x0052DF2E FILE
// RECOVERY_BINDING: v1 {"evidence": ["Original52DE70..52DF2E; retired seed git:117282ee:src/unrecovered/0052de70.cpp records the same identity and spans with its raw-offset TRIED note kept below.", "The vehicles are veh.h's own Vehs (0x00952828, 0x34 stride) whose x/y words open the record; the occupancy mask is lock.h's LockEnableMask and the owner count sits at +0x5A0. The original uses 0x1E shorts as each 0x3C-byte record stride; 25 shorts fit after its 0xA-byte header."], "reason": "Coverage transcription of the deletion enqueue on the typed DeletionList receiver with Vehs and LockEnableMask named from their owners."}
Status: Coverage transcription - unclaimed body
*/
int DeletionList::add_deletion(int veh_id) {
    log_say(StrDeletingVehicle, veh_id, Vehs[veh_id].x, Vehs[veh_id].y);

    int slot = find_owner_slot(current_owner_);
    if (slot < 0) {
        slot = find_free_slot();
        DeletionRecord &new_record = records_[slot];
        new_record.acknowledgements_ = 0xFF;
        new_record.count_ = 0;
        new_record.owner_ = current_owner_;
        new_record.acknowledgements_ = 0;
    }

    DeletionRecord &record = records_[slot];
    const int old_count = record.count_;
    record.vehicle_ids_[old_count] = static_cast<short>(veh_id);
    record.count_ = static_cast<short>(old_count + 1);
    return old_count;
}

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
// sat_edit, 0x004A0810. The satellite editor's click handler: with the shift
// key up, step the faction's satellite field for the chosen body up (or down
// under shift, clamped at zero); on the last body the downward step also
// pulls the seen-map target back and repaints the topic window.
/*
Purpose: Step one faction's satellite count for a body, with the shift-key
         downward path clamped and the seen map refreshed.
// ORIGINAL: 0x004A0810 ?sat_edit@@YAXHHH@Z 0x004A0810-0x004A08D2 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Original4A0810..4A08D2; retired seed git:117282ee:src/unrecovered/004a0810.cpp records the same identity and spans.", "The three per-faction fields are PlayerData (faction.h, the 0x833-dword stride the header pins) at +0x808 (the satellite count stepped by body), +0x814 (the seen-map target) and +0x364 (the seen-map floor); the shift state comes through the USER32 GetAsyncKeyState import slot 0x00669330 that mapwin.cpp binds the same way.", "The repaint is see_map_check (faction.h's own) and redraw on the source-owned ReportWinGlobal (reportwin.h, original 0x00876478) through its inherited virtual slot."], "reason": "Coverage transcription of the satellite editor click with the PlayerData offsets documented and the repaint through the tree's own names."}
Status: Coverage transcription - unclaimed body
*/

void __cdecl sat_edit(int body, int faction_id, int type) {
    if (type < 4 && body != 0) {
        const unsigned char shift_high = static_cast<unsigned char>(
            ::GetAsyncKeyState(0x10) >> 8);
        if (shift_high == 0) {
            reinterpret_cast<int *>(&PlayersData[faction_id])[0x808 / 4 + type]
                = reinterpret_cast<int *>(&PlayersData[faction_id])[0x808 / 4 + type]
                  + 1;
        } else {
            int *const field =
                &reinterpret_cast<int *>(&PlayersData[faction_id])[0x808 / 4 + type];
            if (*field != 0) {
                *field = *field - 1;
            }
            if (type == 3
                && (reinterpret_cast<const int *>(&PlayersData[faction_id])[0x814 / 4]
                    - reinterpret_cast<const int *>(&PlayersData[faction_id])[0x364 / 4])
                   < 0) {
                reinterpret_cast<int *>(&PlayersData[faction_id])[0x364 / 4]
                    = reinterpret_cast<const int *>(&PlayersData[faction_id])[0x814 / 4];
                see_map_check();
                ReportWinGlobal.redraw();
                return;
            }
        }
        see_map_check();
        ReportWinGlobal.redraw();
    }
}


// ---------------------------------------------------------------------------
// net_not_my_turn, 0x00529E70. The end-of-turn wait for the local player in
// a network game: mark the turn as handed over, redraw the multiplayer
// window, latch this faction into the waiting set, then pump the network
// daemon and skip forward through absent factions until the active turn
// comes back around.
/*
Purpose: Wait out the other factions' turns in a network game, skipping
         forward through absent players.
// ORIGINAL: 0x00529E70 ?net_not_my_turn@@YAXXZ 0x00529E70-0x00529F2C FILE
// RECOVERY_BINDING: v1 {"evidence": ["Original529E70..529F2C; retired seed git:117282ee:src/unrecovered/00529e70.cpp records the same identity and spans with its scheduling TRIED note kept below.", "The turn block is console.h's own ConsoleGlobal (0x009156B0, set_view), game.h's MultiWindow (0x007FD648, the hypothesis layout's draw), netdaemon.h's NetDaemonNet (0x0093CD90, net_tasks) and game.h's ExitTurnLoop (0x009B2068); the waiting latch at 0x0093E8EC takes this faction's bit and 0x00939294/0x00939298 are the two turn-handover flags.", "game.cpp binds 0x0093A950 as NextTurnReady and 0x0093E8C0 as TurnsAssigned; 0x009A6820 is game.cpp's NetTurnFaction and the skip tests are lock.h's LockEnableMask bytes at 0x009A64E8/0x009A64E9.", "next_player_turn (game.cpp's 0x0052DBF0) has no header declaration yet, so the declaration travels with this body."], "reason": "Coverage transcription of the network turn wait on the tree's own named objects."}
// TRIED (seed): byte-exact to 94.5% mnemonic similarity; the remaining
//         divergence is the arg-0 push order for set_view against the two
//         independent global stores.
Status: Coverage transcription - unclaimed body
*/
void __cdecl net_not_my_turn() {
    ConsoleGlobal.field_23BE4_ = 0;
    ConsoleGlobal.field_23BE8_ = 1;
    ConsoleGlobal.set_view(0);
    MultiWindow.draw(0);
    NetDaemonNet.factions_waiting_mask() |=
        (1u << (ConsoleGlobal.faction_id_ & 31));
    while (ExitTurnLoop == 0 && NextTurnReady == 0
           && GameDataStateStorage.fields_.net_turn_faction_ != ConsoleGlobal.faction_id_) {
        ConsoleGlobal.field_23BE4_ = 1;
        NetDaemonNet.net_tasks();
        if (NetDaemonNet.lock_state_[0].leading_words_[1] != 0) {
            const int level = GameDataStateStorage.fields_.net_turn_faction_;
            if (level <= 0) {
                next_player_turn();
            } else {
                const unsigned char bit = static_cast<unsigned char>(1u << (level & 31));
                const unsigned char *const lock_bytes =
                    reinterpret_cast<const unsigned char *>(&GameDataStateStorage.fields_.lock_masks_.enable_mask_);
                if ((lock_bytes[0] & bit) == 0) {
                    next_player_turn();
                } else if ((lock_bytes[1] & bit) == 0) {
                    next_player_turn();
                }
            }
        }
    }
    ConsoleGlobal.field_23BE4_ = 0;
    ConsoleGlobal.field_23BE8_ = 0;
}


// ---------------------------------------------------------------------------
namespace {
class Int2SlotObj { public: void slot(int, int); };
}



// ---------------------------------------------------------------------------


// ---------------------------------------------------------------------------


// These original initializers call CRT functions rather than intrinsic copies.
#pragma function(memset, strlen, strcpy)









// ORIGINAL: 0x004C8150 sub_4c8150 0x004C8150-0x004C8188 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:path-record-owner", "Pinned4C8150 initializes0x28 bytes;4C8190 frees1C/24 without clearing or freeing20;4C81C0 calls4C8290 on its receiver;4C8290/83A0 replace owned path/name strings.", "historical source:src/recovered/004c8150.cpp"], "reason": "Recover demonstrated path/directory/name pointer operations as genuine members under a provisional record name; initializer/close identity does not prove a constructor/destructor or historical TU. Preserve original storage and cleanup omissions."}
// symbol    ?initialize@PathRecord@@QAEPAU1@XZ
PathRecord *PathRecord::initialize() {
    // Native 0x004C815D clears four bytes beginning at +4.
    memset(&unknown_04_, 0, sizeof(unknown_04_));
    unknown_00_ = 0;
    unknown_08_ = 0;
    unknown_0C_ = 0;
    unknown_10_ = 0;
    unknown_14_ = 0;
    path_ = 0;
    directory_ = 0;
    name_ = 0;
    unknown_18_ = 0xfa;
    return this;
}

// ORIGINAL: 0x004C8190 sub_4c8190 0x004C8190-0x004C81B5 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:path-record-owner", "Pinned4C8150 initializes0x28 bytes;4C8190 frees1C/24 without clearing or freeing20;4C81C0 calls4C8290 on its receiver;4C8290/83A0 replace owned path/name strings.", "historical source:src/recovered/004c8190.cpp"], "reason": "Recover demonstrated path/directory/name pointer operations as genuine members under a provisional record name; initializer/close identity does not prove a constructor/destructor or historical TU. Preserve original storage and cleanup omissions."}
// symbol    ?close@PathRecord@@QAEXXZ
// Original close leaves the directory allocation and both freed pointer
// values intact. Do not turn this operation into an inferred RAII destructor.
void PathRecord::close() {
    if (path_ != 0) {
        ::operator delete(path_);
    }
    if (name_ != 0) {
        ::operator delete(name_);
    }
}

// ORIGINAL: 0x004C81C0 sub_4c81c0 0x004C81C0-0x004C8286
// RECOVERY_BINDING: v1 {"evidence": ["task:path-record-owner", "Pinned4C8150 initializes0x28 bytes;4C8190 frees1C/24 without clearing or freeing20;4C81C0 calls4C8290 on its receiver;4C8290/83A0 replace owned path/name strings.", "historical source:src/stringstruct.cpp"], "reason": "Recover demonstrated path/directory/name pointer operations as genuine members under a provisional record name; initializer/close identity does not prove a constructor/destructor or historical TU. Preserve original storage and cleanup omissions."}
// symbol    ?set_directory@PathRecord@@QAEXPAD@Z
void PathRecord::set_directory(char *directory) {
    if (directory_ != 0) {
        ::operator delete(directory_);
    }
    directory_ = 0;

    char buffer[256];
    unsigned int length = strlen(directory);
    strcpy(buffer, directory);
    if (buffer[length - 1] != '\\') {
        buffer[length] = '\\';
        buffer[length + 1] = 0;
        ++length;
    }
    ++length;
    directory_ = static_cast<char *>(::operator new(length));
    strcpy(directory_, buffer);

    if (path_ != 0) {
        const unsigned int path_length = strlen(path_);
        char *cursor = path_ + path_length;
        char *scan = cursor;
        while (scan != path_) {
            if (*(scan - 1) == '\\') {
                break;
            }
            --scan;
        }
        strcat(buffer, scan);
        set_path(buffer);
    }
}

// ORIGINAL: 0x004C8290 sub_4c8290 0x004C8290-0x004C8348
// RECOVERY_BINDING: v1 {"evidence": ["task:path-record-owner", "Pinned4C8150 initializes0x28 bytes;4C8190 frees1C/24 without clearing or freeing20;4C81C0 calls4C8290 on its receiver;4C8290/83A0 replace owned path/name strings.", "historical source:src/stringstruct.cpp"], "reason": "Recover demonstrated path/directory/name pointer operations as genuine members under a provisional record name; initializer/close identity does not prove a constructor/destructor or historical TU. Preserve original storage and cleanup omissions."}
// symbol    ?set_path@PathRecord@@QAEXPAD@Z
void PathRecord::set_path(char *path) {
    if (path_ != 0) {
        ::operator delete(path_);
    }
    path_ = 0;

    if (path != 0) {
        char buffer[256];
        if (directory_ != 0) {
            strcpy(buffer, directory_);
            char *slash = strrchr(path, '\\');
            if (slash != 0) {
                strcat(buffer, slash + 1);
            } else {
                strcat(buffer, path);
            }
        } else {
            strcpy(buffer, path);
        }
        const unsigned int length = strlen(buffer);
        char *fresh = static_cast<char *>(::operator new(length + 1));
        path_ = fresh;
        strcpy(fresh, buffer);
    }
}

// ORIGINAL: 0x004C83A0 sub_4c83a0 0x004C83A0-0x004C83EB BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:path-record-owner", "Pinned4C8150 initializes0x28 bytes;4C8190 frees1C/24 without clearing or freeing20;4C81C0 calls4C8290 on its receiver;4C8290/83A0 replace owned path/name strings.", "historical source:src/recovered/004c83a0.cpp"], "reason": "Recover demonstrated path/directory/name pointer operations as genuine members under a provisional record name; initializer/close identity does not prove a constructor/destructor or historical TU. Preserve original storage and cleanup omissions."}
// symbol    ?set_name@PathRecord@@QAEXPAD@Z
void PathRecord::set_name(char *name) {
    if (name_ != 0) {
        ::operator delete(name_);
    }
    if (name != 0) {
        const unsigned int length = strlen(name) + 1;
        void *fresh = ::operator new(length);
        name_ = static_cast<char *>(fresh);
        strcpy(name_, name);
    } else {
        name_ = 0;
    }
}

// ORIGINAL: 0x0058FB70 sub_58fb70 0x0058FB70-0x0058FBB2 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical seed:src/recovered/units/0058fb70.cpp", "task:owned-helper-coverage-batch", "Native58FB70 mode0 returns unchanged, mode1 passes a two-byte terminated buffer and writes the first byte back; other nonzero modes call USER32 CharUpperA on the whole text. No mapped-caller claim."], "reason": "Recover a complete native helper through genuine source owners and SDK calls, preserving original identity, extent and historical obligations. Source home is not proof of original TU or complete-game acceptance."}
// symbol    ?uppercase_text@@YAPADPADH@Z
char *__cdecl uppercase_text(char *text, int mode) {
    if (mode == 0) {
        return text;
    }
    if (mode == 1) {
        char first_character[2];
        first_character[0] = *text;
        first_character[1] = '\0';
        CharUpperA(first_character);
        *text = first_character[0];
        return text;
    }
    CharUpperA(text);
    return text;
}
