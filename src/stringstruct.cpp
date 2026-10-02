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
StringAllocationBase::~StringAllocationBase() {
    StringAllocationHeap = allocation_owner_;
}

// The original primary table uses these shared defaults. Hook names describe
// their list role; the catalogue's unrelated class names are retained as seeds.
// RECOVERY_SHARED: v1 0x00404250 0x00404255 ?on_insert@StringStruct@@UAEHPAVStringStructPayload@@@Z 0x006086F0
int StringStruct::on_insert(StringStructPayload *) { return 0; }
// RECOVERY_SHARED: v1 0x00404270 0x00404273 ?on_remove@StringStruct@@UAEXPAVStringStructPayload@@@Z 0x006086F0
void StringStruct::on_remove(StringStructPayload *) { }

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
#include "spot.h"      // Spot - MultiWin's cursor-hotspot array
#include "veh.h"       // Vehs - the deletion list walks its 0x34-stride records
#include "lock.h"      // LockEnableMask - the deletion-record occupancy mask
#include "log.h"       // log_say - both deletion paths report through it
#include "fx.h"        // FX - play is defined in this wave
#include "deletionlist.h"  // DeletionList conversion and deletion enqueue
#include "pulldown.h"   // PullDown - the menu handlers are defined in this wave
#include "reportwin.h"  // ReportWin - on_redraw is defined in this wave
#include "diplopop.h"   // DiploPop - on_redraw is defined in this wave
#include "setupwin.h"   // SetupWin - close is defined in this wave
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
static char *const CaretSeparator = (char *)0x006867C0;         // "^"

// The local faction id the game layer keeps at 0x00939284 - base.cpp and
// faction.cpp both reach it by address; nothing declares it yet.
static int *const LocalFactionSlot = (int *)0x00939284;

// The spot label is built in strings.h's StringTemp (0x009B86A0), the same
// shared scratch the sibling alpha.cpp prefs paths fill.

// The unit draw-state transparency byte the image keeps beside the wrappers.
}

// The shared surface Sprite buffer.cpp constructs at 0x009B3A50 (its own
// TU-local extern there) and the tech-wizard facility sprites sprite.cpp
// defines; the unit draw wrappers and the wizard painter draw through them.
extern Sprite g_BUFFER_SPRITE;        // 0x009B3A50
extern Sprite g_TECH_ICON_SPRITES[89];  // 0x00759E28, sprite.cpp's definition

namespace {
// The published unit draw-state block: the words the surface sprite's
// draw_dest_unk1 readers consume, 0x009B3A54..0x009B3A78.

// VC6 rejects a free __thiscall function pointer (C4234), so every indirect
// virtual dispatch below goes through a member-fn-pointer union: the union's
// `raw` word is the vtable slot, and only DECLARATION ORDER decides which
// slot that is. The same pattern dialogs.cpp, checkbox.cpp and editgroup.cpp
// already carry.
class NullSlotObj { public: void slot(); };
union NullSlotShim {
    void (NullSlotObj::*fn)();
    void *raw;
};
class ReturnSlotObj { public: int slot(); };
union ReturnSlotShim {
    int (ReturnSlotObj::*fn)();
    void *raw;
};
class ArgSlotObj { public: void slot(int); };
union ArgSlotShim {
    void (ArgSlotObj::*fn)(int);
    void *raw;
};
class ModeReleaseObj { public: void release(void *); };
}

// ---------------------------------------------------------------------------
// FX::play, 0x00446A00. The bank's sound trigger: gate on the sound bit of
// GamePreferences, re-arm the popup wave owner for the first three effect
// kinds, then release/restart the effect record. fx.h declares the member;
// its forwarder retires from pending_bodies.cpp in this change.

// Interlude members and compiler-generated lifetime code are owned by interlude.cpp.

// ---------------------------------------------------------------------------
// sub_5cbae0, 0x005CBAE0. The DirectDraw primary-surface blit wrapper: build
// the DDBLTFX record, then the surface's slot-33 blit, mapping the two
// documented COM failure codes onto the caller's retry/lost-surface flags.
/*
Purpose: Blit the frame through the primary surface's blit slot, translating
         the two failure codes the device reports back into caller flags.
// ORIGINAL: 0x005CBAE0 sub_5cbae0 0x005CBAE0-0x005CBBB6 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Original5CBAE0..5CBBB6; retired seed git:117282ee:src/unrecovered/005cbae0.cpp records the same identity and spans.", "The receiver table is the wrapper block the window layer keeps: slot 0 is the primary surface, words 3..6 the destination rectangle; the DDBLTFX-shaped stack record carries dwSize 0x38 and, on the fill path, DDBLT_DDFX 0x84000 with rotation flag word 14.", "0x80070057 (E_INVALIDARG) latches the wrapper's +0x4C error word and 0x887601c2 (DDERR_SURFACELOST) restores through the device's slot-27 restore before returning 1; both dispatches run through member-fn-pointer unions."], "reason": "Coverage transcription of the primary-surface blit wrapper with the COM failure mapping the seed measures."}
Status: Coverage transcription - unclaimed body
*/
namespace {
class BltSlotObj { public: int blit(RECT *, int, RECT *, int, void *); };
union BltSlotShim {
    int (BltSlotObj::*fn)(RECT *, int, RECT *, int, void *);
    void *raw;
};
class RestoreSlotObj { public: void restore(); };
union RestoreSlotShim {
    void (RestoreSlotObj::*fn)();
    void *raw;
};
}

extern "C" int __fastcall sub_5cbae0(int *wrapper, int fill) {
    int fx[16] = {0};
    fx[2] = 0x38;  // DDBLTFX.dwSize

    int mode;
    if (fill == 0) {
        mode = 0x200;  // DDBLT_WAIT
    } else {
        mode = 0x84000;  // DDBLT_DDFX
        fx[14] = 1;      // DDBLTFX.dwDDFX rotation flag
    }

    const int y = wrapper[6];
    const int x = wrapper[5];
    RECT rect;
    rect.left = x;
    rect.top = y;
    rect.right = wrapper[3] + x;
    rect.bottom = wrapper[4] + y;

    int *const device = reinterpret_cast<int *>(wrapper[0]);
    fx[0] = device[25] - 1;
    fx[1] = device[24] - 1;
    void *const surface = reinterpret_cast<void *>(device[7]);
    const int fill_color = device[5];

    void **const surface_vtable =
        *reinterpret_cast<void ***>(surface);
    BltSlotShim blit;
    blit.raw = surface_vtable[33];
    const int result = (reinterpret_cast<BltSlotObj *>(surface)->*blit.fn)(
        &rect, fill_color, &rect, mode, &fx[2]);
    if (result != 0) {
        if (result == static_cast<int>(0x80070057)) {  // E_INVALIDARG
            wrapper[0x13] = 1;
            return 1;
        }
        if (result == static_cast<int>(0x887601c2)) {  // DDERR_SURFACELOST
            void *const device_iface = reinterpret_cast<void *>(wrapper[0]);
            RestoreSlotShim restore;
            restore.raw = (*reinterpret_cast<void ***>(device_iface))[27];
            (reinterpret_cast<RestoreSlotObj *>(device_iface)->*restore.fn)();
        }
        return 1;
    }
    return 0;
}


// ---------------------------------------------------------------------------
// MultiWin::UNK1, 0x00479270. The multiplayer scoreboard's spot label: clip
// the given rectangle into the window's hotspot array, then build the
// difficulty caret row into the shared scratch and centre it over the spot.
// MultiWin's layout is a hypothesis and the member is undeclared, so the
// receiver travels explicitly; the original is a __thiscall member.
/*
Purpose: Draw one multiplayer window spot and centre its difficulty caret
         row over it.
// ORIGINAL: 0x00479270 ?UNK1@MultiWin@@QAEXPAURECT@@@Z 0x00479270-0x0047932C FILE
// RECOVERY_BINDING: v1 {"evidence": ["Original4792CF pushes682820 into strcat645470 at4792E0; pinned bytes at682820 are one space followed by NUL, identical to the Buffer/SocialWin shared join object.", "This narrow reference migration leaves the existing explicit MultiWin receiver, Spot/difficulty walk and caret/text procedure ordering unchanged; it does not establish full MultiWin hierarchy or runtime acceptance."], "reason": "Replace this existing MultiWin scaffold shared-string alias with the same genuine source-owned join-space object."}
// symbol    @MultiWin_UNK1@12
// SUPERSEDED_BINDING_HISTORY: v1 {"evidence": ["Original479270..47932C; retired seed git:117282ee:src/unrecovered/00479270.cpp records the same identity and spans.", "The receiver's Spot subobject at +0x28AC (spot.h's own add/3,2 overload) and the +0x444 Buffer (write_cent_l is buffer.h's own) carry the walk; the difficulty table is the 0x833-dword PlayerData stride faction.h pins, read at the +0x2054 record offset through the local-faction slot 0x00939284.", "The caret row is built into strings.h's StringTemp (0x009B86A0) from the one-space and caret .rdata strings exactly as the seed strcat-walks them."], "reason": "Coverage transcription of the multiplayer spot label with the receiver spelled explicitly because MultiWin has no header declaration yet."}
Status: Coverage transcription - unclaimed body
*/
extern "C" void __fastcall MultiWin_UNK1(MultiWin *self_raw, void *, RECT *rect) {
    char *const self = reinterpret_cast<char *>(self_raw);
    const int top = rect->top;
    const int height = rect->bottom - top;
    const int left = rect->left;
    const int width = rect->right - left;
    reinterpret_cast<Spot *>(self + 0x28AC)->add(3, 2, left, top, width, height);

    const int difficulty = reinterpret_cast<const int *>(
        reinterpret_cast<const char *>(&PlayersData[*LocalFactionSlot]))[0x2054 / 4];
    if (difficulty >= 0) {
        int carets = (difficulty > 3) ? 3 : difficulty;
        if (carets != 0) {
            StringTemp[0] = 0;
            strcat(StringTemp, BufferJoinSpace);
            for (; carets != 0; --carets) {
                strcat(StringTemp, CaretSeparator);
            }
            const unsigned int len = strlen(StringTemp);
            reinterpret_cast<Buffer *>(self + 0x444)->write_cent_l(
                StringTemp, rect, len);
        }
    }
}

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
// stride) and the transparency bytes beside it (0x00759E30).
static unsigned char *const WizardTransTable = (unsigned char *)0x00759E30;

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
        const unsigned char transparency = WizardTransTable[entry * 4];
        const int sprite_index = (entry * 4) / 0x2C;
        g_TECH_ICON_SPRITES[sprite_index].draw(target, transparency, x, y);

        SpriteScaleNumerator = 1;
        SpriteScaleDenominator = 1;
    }
}

// ---------------------------------------------------------------------------
// sub_447d20, 0x00447D20. The settings-list checkbox cascade: whichever of
// the first two slots is off gets the next increment of the shared 0x12C
// delay value and repaints; the second slot also repaints on its own path.
// The receiver is an unnamed list object whose slots 67/69/70 do the work;
// they run through member-fn-pointer unions at their measured indices.
/*
Purpose: Advance the shared delay value across the settings list's first two
         entries, repainting whichever entry turned off.
// ORIGINAL: 0x00447D20 sub_447d20 0x00447D20-0x00447DA5 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Original447D20..447DA5; retired seed git:117282ee:src/unrecovered/00447d20.cpp records the same identity and spans.", "The shared delay counter is 0x00683C2C, stepped by 0x12C and floored at -0x4B0 exactly as the seed walks it; the receiver's slots 67/69/70 are dispatched at their measured indices through member-fn-pointer unions because the object's class is unnamed in the catalogue."], "reason": "Coverage transcription of the settings cascade with the delay counter bound and the receiver's slots dispatched at their measured indices."}
Status: Coverage transcription - unclaimed body
*/
namespace {
static int *const CascadeDelaySlot = (int *)0x00683C2C;

class IntSlotObj { public: int slot(int); };
union IntSlotShim {
    int (IntSlotObj::*fn)(int);
    void *raw;
};
class IntIntSlotObj { public: void slot(int, int); };
union IntIntSlotShim {
    void (IntIntSlotObj::*fn)(int, int);
    void *raw;
};
}

extern "C" void __fastcall sub_447d20(void *self_raw, void *) {
    char *const self = reinterpret_cast<char *>(self_raw);
    void **const vtable = *reinterpret_cast<void ***>(self);
    IntSlotShim state;
    state.raw = reinterpret_cast<void *>(vtable[70]);
    IntIntSlotShim store;
    store.raw = reinterpret_cast<void *>(vtable[69]);
    NullSlotShim repaint;
    repaint.raw = reinterpret_cast<void *>(vtable[67]);
    IntSlotObj *const receiver = reinterpret_cast<IntSlotObj *>(self);
    if ((receiver->*state.fn)(1) == 0) {
        if ((receiver->*state.fn)(2) == 0) {
            if (*CascadeDelaySlot == 0) {
                *CascadeDelaySlot = -0x4B0;
            }
            *CascadeDelaySlot = *CascadeDelaySlot + 0x12C;
            (reinterpret_cast<IntIntSlotObj *>(self)->*store.fn)(
                1, *CascadeDelaySlot);
            (reinterpret_cast<NullSlotObj *>(self)->*repaint.fn)();
            return;
        }
    }
    if ((receiver->*state.fn)(2) == 0) {
        (reinterpret_cast<IntIntSlotObj *>(self)->*store.fn)(2, *CascadeDelaySlot);
        (reinterpret_cast<NullSlotObj *>(self)->*repaint.fn)();
    }
}


// ---------------------------------------------------------------------------
// DiploPop::on_redraw, 0x004402A0. The diplomacy pop's repaint: restore the
// desktop backdrop, frame the union of the desktop, list and panel
// rectangles with the shared panel styles, then run the four sub-drawers.
// The member is undeclared in diplopop.h, so the receiver travels
// explicitly; the original is a __thiscall member.
/*
Purpose: Repaint the diplomacy pop's backdrop and framed panel.
// ORIGINAL: 0x004402A0 ?on_redraw@DiploPop@@QAEXXZ 0x004402A0-0x00440379 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Original4402A0..440379; retired seed git:117282ee:src/unrecovered/004402a0.cpp records the same identity and spans with its RECT-local TRIED note kept below.", "The opener is maininterface.h's restore_back on mapwin.h's MainInterfaceGlobal (0x007AE820); the frame styles are buffer.h's InterfaceBoxStyles[3] (0x0078D5A0) and [21] (0x0078D870) against the interface Buffer 0x007AEC64; the UnionRect slot 0x00669328 is the runtime import slot console.h documents.", "The four sub-drawers are the seed's measured call targets 0x0043F5F0/0x0043F790/0x0043FA90/0x00440080 in the body's call order (bioscan/portrait/info/hq); 0x00440080 is now the source-owned DiploPop::draw_hq body, while the first three helper bodies remain outside this task's source ownership. The receiver's panel rect at +0x2144 remains the seed's measured walk."], "reason": "Recover the diplomacy pop repaint through its typed interface state and panel members, including the now-source-owned HQ drawer; the first three helper bodies remain an explicit outside-owner dependency."}
Status: Coverage transcription - unclaimed body
*/
namespace {
// The desktop rectangle block the interface layer keeps beside the
// interface Buffer: the desktop edges and the two panel rectangles the
// pop's union walks.
static int *const DesktopRectBlock = (int *)0x007AF5E0;
static RECT *const DiplomacyListRect = (RECT *)0x007AF2AC;
static RECT *const DiplomacyPanelRect = (RECT *)0x007AF2BC;
}

extern "C" void __fastcall DiploPop_on_redraw(DiploPop *self, void *) {
    RECT rc;
    char *const base = reinterpret_cast<char *>(self);

    MainInterfaceGlobal.restore_back(1);

    rc.left = DesktopRectBlock[0] + 4;
    rc.top = DesktopRectBlock[1] + 4;
    rc.right = DesktopRectBlock[2] - 4;
    rc.bottom = DesktopRectBlock[3] - 4;
    reinterpret_cast<Buffer *>(0x007AEC64)->box_sprite(
        &rc, &InterfaceBoxStyles[3]);

    typedef int (__stdcall *UnionRectFn)(RECT *, RECT *, RECT *);
    UnionRectFn union_rect = *reinterpret_cast<UnionRectFn *>(0x00669328);
    union_rect(&rc, DiplomacyListRect, DiplomacyPanelRect);
    union_rect(&rc, &rc, reinterpret_cast<RECT *>(base + 0x2144));

    rc.left -= 3;
    rc.right += 3;
    rc.bottom += 3;
    rc.top -= 3;

    reinterpret_cast<Buffer *>(0x007AEC64)->box_sprite(
        &rc, &InterfaceBoxStyles[21]);

    typedef void (__fastcall *PopDraw)(DiploPop *, void *);
    reinterpret_cast<PopDraw>(0x0043F5F0)(self, nullptr);  // draw_bioscan
    reinterpret_cast<PopDraw>(0x0043F790)(self, nullptr);  // draw_portrait
    reinterpret_cast<PopDraw>(0x0043FA90)(self, nullptr);  // draw_info
    self->draw_hq();
}

// ---------------------------------------------------------------------------
// SetupWin::close, 0x004AD9C0. The setup window's teardown: clear the
// per-window state words, close the owned buffer, shut the hotspot array
// down, then walk the interface-mode list releasing every entry's payload
// before clearing the list head. The member is undeclared in setupwin.h, so
// the receiver travels explicitly; the original is a __thiscall member.
/*
Purpose: Tear the setup window down: clear state, close the owned buffer,
         release the hotspot array and drain the interface-mode list.
// ORIGINAL: 0x004AD9C0 ?close@SetupWin@@QAEXXZ 0x004AD9C0-0x004ADA7B FILE
// RECOVERY_BINDING: v1 {"evidence": ["Original4AD9C0..4ADA7B; retired seed git:117282ee:src/unrecovered/004ad9c0.cpp records the same identity and spans with its adjustor-thunk TRIED note kept below.", "The storage is setupwin.h's own model: state words 0xA14/0xA18, the Buffer at 0xA44 and Spot at 0xFD4 are declared members, and the interface-mode manager block sits at 0xFE0 with its head/count/cursor at +8/+0xC/+0x10.", "The list payload release and the deleting-destructor dispatch run through member-fn-pointer unions because the payload's class is unnamed; the adjusted-receiver dispatch is the seed's vtbl+4 walk."], "reason": "Coverage transcription of the setup teardown on the typed SetupWin storage with the mode-list walk kept at the seed's shape."}
Status: Coverage transcription - unclaimed body
*/
extern "C" void __fastcall SetupWin_close(SetupWin *self, void *) {
    char *const base = reinterpret_cast<char *>(self);
    self->selected_row_ = 0;
    self->row_count_ = 0;
    self->field_A14_ = 0;
    self->field_A18_ = 0;
    self->buffer_.close();
    self->spot_.shutdown();

    char *const mgr = base + 0xFE0;
    if (*reinterpret_cast<int *>(mgr + 8) != 0) {
        // The mode list's slot-1 release takes the entry payload.
        typedef void (ModeReleaseObj::*ModeReleaseFn)(void *);
        union ModeReleaseShim {
            ModeReleaseFn fn;
            void *raw;
        };
        ModeReleaseShim release;
        release.raw = (*reinterpret_cast<void ***>(mgr))[1];
        int i;
        for (i = 0; i < *reinterpret_cast<int *>(mgr + 0x10); ++i) {
            char *const item = *reinterpret_cast<char **>(mgr + 8);
            *reinterpret_cast<int *>(mgr + 0xC) =
                *reinterpret_cast<int *>(item + 0xC);
            void *const payload = *reinterpret_cast<void **>(item + 8);
            (reinterpret_cast<ModeReleaseObj *>(mgr)->*release.fn)(payload);
            if (payload != 0) {
                // The payload's deleting destructor enters through its own
                // adjusted receiver (vtbl slot 1 displacement).
                const int displacement =
                    *reinterpret_cast<int *>(*reinterpret_cast<char **>(payload) + 4);
                ArgSlotShim delete_payload;
                delete_payload.raw =
                    (*reinterpret_cast<void ***>(payload))[0];
                char *const adjusted =
                    reinterpret_cast<char *>(payload) + displacement;
                (reinterpret_cast<ArgSlotObj *>(adjusted)->*delete_payload.fn)(1);
            }
            *reinterpret_cast<int *>(
                *reinterpret_cast<char **>(mgr + 8) + 8) = 0;
            char *const item2 = *reinterpret_cast<char **>(mgr + 8);
            if (item2 != 0) {
                const int displacement2 =
                    *reinterpret_cast<int *>(*reinterpret_cast<char **>(item2) + 4);
                ArgSlotShim delete_item;
                delete_item.raw =
                    (*reinterpret_cast<void ***>(item2))[0];
                char *const adjusted2 = item2 + displacement2;
                (reinterpret_cast<ArgSlotObj *>(adjusted2)->*delete_item.fn)(1);
            }
            *reinterpret_cast<int *>(mgr + 8) = *reinterpret_cast<int *>(mgr + 0xC);
        }
        *reinterpret_cast<int *>(mgr + 8) = 0;
        *reinterpret_cast<int *>(mgr + 0x14) = 0;
        *reinterpret_cast<int *>(mgr + 0x10) = 0;
    }
    *reinterpret_cast<int *>(base + 0xFF4) = 0;
}

// ---------------------------------------------------------------------------
// MapWin::draw_cursor, 0x0046AD60. The map's terrain cursor repaint: fetch
// the current cursor tile from the parallel x/y tables, clip it against the
// view window (with the cylindrical wrap when the map is round), and hand
// the tile to draw_radius as a radius-0 repaint. mapwin.h declares the
// member; its forwarder retires from pending_bodies.cpp in this change.
/*
Purpose: Repaint the map tile under the terrain cursor.
// ORIGINAL: 0x0046AD60 ?draw_cursor@MapWin@@QAEXXZ 0x0046AD60-0x0046ADF1 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Original46AD60..46ADF1; retired seed git:117282ee:src/unrecovered/0046ad60.cpp records the same identity and spans with its table-addressing TRIED note kept below.", "The receiver is mapwin.h's own window: the view fields 0x1DDA4..0x1DDD8 are declared members and draw_radius is declared beside them; 0x0094988C is map.cpp's MapIsFlat and 0x00949870 its MapLongitudeBounds.", "The cursor index and parallel x/y arrays at 0x009392B8/0x009392C0/0x00939340 are ConsoleGlobal members, as declared in console.h."], "reason": "Coverage transcription of terrain cursor repaint on the typed MapWin receiver using the Console owner for cursor history."}
Status: Coverage transcription - unclaimed body
*/
void MapWin::draw_cursor() {
    const int index = ConsoleGlobal.cursor_history_index_;
    const int x = ConsoleGlobal.cursor_history_x_[index];
    const int y = ConsoleGlobal.cursor_history_y_[index];

    if (y < field_1DDA8_) {
        return;
    }
    if (y >= field_1DDD8_ + field_1DDD0_ + field_1DDA8_) {
        return;
    }

    int adjusted_x = x;
    if ((MapIsFlat & 1) == 0) {
        if (x < field_1DDA4_) {
            adjusted_x = MapLongitudeBounds + x;
        }
        if (adjusted_x >= field_1DDD4_ + field_1DDCC_ + field_1DDA4_) {
            adjusted_x = adjusted_x - MapLongitudeBounds;
        }
    }

    if (adjusted_x < field_1DDA4_) {
        return;
    }
    if (adjusted_x >= field_1DDD4_ + field_1DDCC_ + field_1DDA4_) {
        return;
    }

    draw_radius(x, y, 0, 2);
}

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
        const unsigned char mask = static_cast<unsigned char>(LockEnableMask);
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
// PrevWndFunc, 0x005D5C50. The shared window-procedure pre-filter: route the
// two messages the owner handles itself (the scroll range and the mouse
// move) into the object stored in the window's userdata, and hand every
// other message to Win::window_proc through CallWindowProcA.
/*
Purpose: Pre-filter the owner window's messages before the shared window
         procedure sees them.
// ORIGINAL: 0x005D5C50 PrevWndFunc 0x005D5C50-0x005D5CEA FILE
// RECOVERY_BINDING: v1 {"evidence": ["Original5D5C50..5D5CEA; retired seed git:117282ee:src/unrecovered/005d5c50.cpp records the same identity and spans.", "The userdata slot is GetWindowLongA(hwnd, GWL_USERDATA) exactly as win.cpp's adjust_menus reads it; the two dispatches are vtable slots 71 (the scroll, message 5) and 85 (the mouse move, message 0x20), and the default path hands the message to win.h's own static Win::window_proc through CallWindowProcA.", "The dispatches run through member-fn-pointer unions because the object's class is unnamed in the catalogue."], "reason": "Coverage transcription of the message pre-filter with the two handled messages dispatched at their measured slots."}
Status: Coverage transcription - unclaimed body
*/
namespace {
class ScrollSlotObj { public: void slot(int, short, short); };
union ScrollSlotShim {
    void (ScrollSlotObj::*fn)(int, short, short);
    void *raw;
};
class MouseMoveSlotObj { public: int slot(int, unsigned short, unsigned short); };
union MouseMoveSlotShim {
    int (MouseMoveSlotObj::*fn)(int, unsigned short, unsigned short);
    void *raw;
};
}

extern "C" int __stdcall PrevWndFunc(int window, int message, int wparam,
                                     int lparam) {
    if (message == 5) {
        void *const owner =
            reinterpret_cast<void *>(GetWindowLongA((HWND)window, -0x15));
        if (owner != 0) {
            void *const *vtable = *reinterpret_cast<void ***>(owner);
            ScrollSlotShim scroll;
            scroll.raw = vtable[71];
            (reinterpret_cast<ScrollSlotObj *>(owner)->*scroll.fn)(
                wparam, static_cast<short>(lparam),
                static_cast<short>(lparam >> 16));
        }
        return 0;
    } else if (message == 0x20) {  // WM_SETCURSOR family move message
        void *const owner =
            reinterpret_cast<void *>(GetWindowLongA((HWND)window, -0x15));
        if (owner != 0) {
            void *const *vtable = *reinterpret_cast<void ***>(owner);
            MouseMoveSlotShim move;
            move.raw = vtable[85];
            return (reinterpret_cast<MouseMoveSlotObj *>(owner)->*move.fn)(
                wparam, static_cast<unsigned short>(lparam),
                static_cast<unsigned short>(
                    static_cast<unsigned int>(lparam) >> 16));
        }
        return 0;
    }
    return CallWindowProcA(reinterpret_cast<WNDPROC>(&Win::window_proc),
                           (HWND)window, (UINT)message, (WPARAM)wparam,
                           (LPARAM)lparam);
}

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
namespace {
// The USER32 GetAsyncKeyState import slot, the same one mapwin.cpp binds.
static int *const IAT_GetAsyncKeyState = (int *)0x00669330;
}

void __cdecl sat_edit(int body, int faction_id, int type) {
    if (type < 4 && body != 0) {
        typedef short (__stdcall *AsyncKeyStateFn)(int);
        AsyncKeyStateFn get_async_key_state =
            reinterpret_cast<AsyncKeyStateFn>(*IAT_GetAsyncKeyState);
        const unsigned char shift_high = static_cast<unsigned char>(
            get_async_key_state(0x10) >> 8);
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
namespace {
// The turn-handover flags beside the local-faction slot.
static int *const TurnHandoverFlag = (int *)0x00939294;
static int *const TurnHandoverFlag2 = (int *)0x00939298;
// The faction-waiting latch and game.cpp's NextTurnReady/TurnsAssigned.
static const int *const TurnsAssigned = (const int *)0x0093E8C0;
}

int __cdecl next_player_turn();  // 0x0052DBF0, game.cpp's definition

void __cdecl net_not_my_turn() {
    *TurnHandoverFlag = 0;
    *TurnHandoverFlag2 = 1;
    ConsoleGlobal.set_view(0);
    MultiWindow.draw(0);
    ConsoleFactionsWaitingMask |= (1 << *LocalFactionSlot);
    while (ExitTurnLoop == 0 && NextTurnReady == 0
           && NetTurnFaction != *LocalFactionSlot) {
        *TurnHandoverFlag = 1;
        NetDaemonNet.net_tasks();
        if (*TurnsAssigned != 0) {
            const int level = NetTurnFaction;
            if (level <= 0) {
                next_player_turn();
            } else {
                const unsigned char bit = static_cast<unsigned char>(1 << level);
                const unsigned char *const lock_bytes =
                    reinterpret_cast<const unsigned char *>(&LockEnableMask);
                if ((lock_bytes[0] & bit) == 0) {
                    next_player_turn();
                } else if ((lock_bytes[1] & bit) == 0) {
                    next_player_turn();
                }
            }
        }
    }
    *TurnHandoverFlag = 0;
    *TurnHandoverFlag2 = 0;
}

// The menu border padding pulldown.cpp keeps beside its defaults; the row
// maths of all three PullDown handlers divides by it against the font.
extern int PullDownBorderPadding;  // 0x00696E1C, pulldown.cpp's definition

// ---------------------------------------------------------------------------
// PullDown::on_left_down, 0x005F93C0. Clicking outside the pulled-down menu
// closes it: hide the window, flush the input queue, run the task pass, and
// hand the click to the owner menu at its own coordinates. The member is
// undeclared in pulldown.h, so the receiver travels explicitly; the original
// is a __thiscall member.
/*
Purpose: Close the pulled-down menu when the click lands outside it.
// ORIGINAL: 0x005F93C0 ?on_left_down@PullDown@@QAEXHH@Z 0x005F93C0-0x005F945B FILE
// RECOVERY_BINDING: v1 {"evidence": ["Original5F93C0..5F945B; retired seed git:117282ee:src/unrecovered/005f93c0.cpp records the same identity and spans.", "The hit test runs against buffer.h's rect2_ (0x474, the offset ten classes' bodies pass), the hide is Win slot 2 and the menu hand-off is the owner Menu's slot 20 with the two outer-rect deltas - left 0x13C and top 0x140, menu value subtracted from the receiver's on each axis before the click coordinate is added (0x005F9414 loads menu_ from +0xF30; the call target is [eax+0x50]) - dispatched through member-fn-pointer unions because neither slot is declared.", "flush_input is console.h's own and do_all_tasks is temp.h's; selected_ and menu_ are pulldown.h's own members."], "reason": "Coverage transcription of the outside-click close on the typed PullDown receiver."}
Status: Coverage transcription - unclaimed body
*/
namespace {
class Int2SlotObj { public: void slot(int, int); };
class MenuClickObj { public: void slot(int, int); };
}

extern "C" void __fastcall PullDown_on_left_down(PullDown *self, void *, int x,
                                                 int y) {
    // The menu's working rectangle: GraphicWin's rect2_ at 0x474, the offset
    // ten classes' bodies pass to RECT * formals (buffer.h documents it).
    const RECT rc = *reinterpret_cast<const RECT *>(
        reinterpret_cast<const char *>(self) + 0x474);
    const POINT pt = {x, y};
    if (PtInRect(&rc, pt) == 0) {
        // Win slot 2: hide.
        NullSlotShim hide;
        hide.raw = (*reinterpret_cast<void ***>(self))[2];
        (reinterpret_cast<NullSlotObj *>(self)->*hide.fn)();
        flush_input();
        do_all_tasks(reinterpret_cast<Win *>(self));
        // menu_ is the pointer at +0xF30 (0x005F9414 loads [esi+0xF30]).
        Menu *const menu = *reinterpret_cast<Menu **>(
            reinterpret_cast<char *>(self) + 0xF30);
        *reinterpret_cast<int *>(reinterpret_cast<char *>(self) + 0xF28) = -1;
        if (menu != 0) {
            void *const *menu_vtable = *reinterpret_cast<void ***>(menu);
            // Menu slot 20 takes the outer-rect deltas: left (0x13C) first,
            // top (0x140) last, the menu's value subtracted from this
            // window's on each axis before the click coordinate is added.
            typedef void (MenuClickObj::*MenuClickFn)(int, int);
            union MenuClickShim {
                MenuClickFn fn;
                void *raw;
            };
            MenuClickShim click;
            click.raw = menu_vtable[0x50 / 4];
            const char *const self_base = reinterpret_cast<const char *>(self);
            const char *const menu_base = reinterpret_cast<const char *>(menu);
            (reinterpret_cast<MenuClickObj *>(menu)->*click.fn)(
                (*reinterpret_cast<const int *>(self_base + 0x13C)
                  - *reinterpret_cast<const int *>(menu_base + 0x13C))
                    + x,
                (*reinterpret_cast<const int *>(self_base + 0x140)
                  - *reinterpret_cast<const int *>(menu_base + 0x140))
                    + y);
        }
    }
}

// ---------------------------------------------------------------------------
// PullDown::on_left_up, 0x005F9480. Releasing the button over the open menu
// selects the row under the cursor: compute the row from the line advance,
// walk the enabled items to its index, and redraw or hide on the empty row.
// The member is undeclared in pulldown.h, so the receiver travels explicitly;
// the original is a __thiscall member.
/*
Purpose: Select the menu row under the mouse when the button is released.
// ORIGINAL: 0x005F9480 ?on_left_up@PullDown@@QAEXHH@Z 0x005F9480-0x005F9563 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Original5F9480..5F9563; retired seed git:117282ee:src/unrecovered/005f9480.cpp records the same identity and spans.", "The row maths is pulldown.h's own model: items_ at 0xA24 in 0x14-byte PullDownItem records (bit 0 enabled, bit 4 disabled-for-pick), item_count_ at 0xF20 and selected_ at 0xF28, with the line advance from pulldown.h's own PullDownDefaultFont and PullDownBorderPadding (0x009B7B68/0x00696E1C).", "The empty-row path is Win slot 62 (redraw) and the hit path is slot 2 (hide), dispatched through member-fn-pointer unions."], "reason": "Coverage transcription of the row pick on the typed PullDown receiver with the item walk exactly as the seed measures it."}
Status: Coverage transcription - unclaimed body
*/
extern "C" void __fastcall PullDown_on_left_up(PullDown *self, void *, int x,
                                               int y) {
    RECT rect = *reinterpret_cast<const RECT *>(
        reinterpret_cast<const char *>(self) + 0x474);
    const POINT pt = {x, y};
    if (PtInRect(&rect, pt) != 0) {
        // The current default font's line advance (font.h's own selection).
        Font *const font = PullDownDefaultFont;
        const int line_height = font->line_advance();
        char *const base = reinterpret_cast<char *>(self);
        int *const selected = reinterpret_cast<int *>(base + 0xF28);
        const int count = *reinterpret_cast<const int *>(base + 0xF20);
        *selected = (y - PullDownBorderPadding) / line_height;
        int index = 0;
        if (count > 0) {
            unsigned char *item =
                reinterpret_cast<unsigned char *>(base + 0xA24);
            int enabled = -1;
            do {
                if ((*item & 1) != 0) {
                    enabled++;
                }
                if (enabled == *selected) {
                    break;
                }
                index++;
                item += 0x14;
            } while (index < count);
        }
        if (index == count) {
            *selected = -1;
        } else {
            *selected = index;
            if ((reinterpret_cast<unsigned char *>(base + 0xA24)
                 + index * 0x14)[0]
                & 4) {
                *selected = -1;
            }
        }
        void *const *vtable = *reinterpret_cast<void ***>(self);
        if (*selected == -1) {
            NullSlotShim redraw;
            redraw.raw = vtable[62];
            (reinterpret_cast<NullSlotObj *>(self)->*redraw.fn)();
            return;
        }
        NullSlotShim hide;
        hide.raw = vtable[2];
        (reinterpret_cast<NullSlotObj *>(self)->*hide.fn)();
    }
}

// ---------------------------------------------------------------------------
// FlatButton::on_mouse_leave, 0x00608660. Leaving a flat button releases its
// pressed look and bubble: skip when this button is the tracked hover pair,
// otherwise stop the hold timer, clear the bubble, notify the buddy control
// and redraw. The member is undeclared in flatbutton.h, so the receiver
// travels explicitly; the original is a __thiscall member.
/*
Purpose: Release a flat button's pressed look and bubble text when the
         mouse leaves it.
// ORIGINAL: 0x00608660 ?on_mouse_leave@FlatButton@@QAEXHH@Z 0x00608660-0x006086EC FILE
// RECOVERY_BINDING: v1 {"evidence": ["Original608660..6086EC; retired seed git:117282ee:src/unrecovered/00608660.cpp records the same identity and spans.", "The pressed/hover state is basebutton.h's own model: time1_ at 0xA1C (time.h's own stop), field_A74_/field_A78_, bubble_text_ at 0xA80, plus the hover-pair slots 0x009B7ACC/0x9B7AD0/0x9B7AD4 and the buddy pointer at +0xC4 in GraphicWin's region.", "The press repaint, buddy notification and redraw run through the receiver's slots 17/40/62 (the last is Win's own redraw) via member-fn-pointer unions; Win::clear_bubble_text is win.h's own static."], "reason": "Coverage transcription of the flat-button leave handler on the typed BaseButton storage."}
Status: Coverage transcription - unclaimed body
*/
extern "C" void __fastcall FlatButton_on_mouse_leave(BaseButton *self, void *,
                                                     int x, int y) {
    int active = *reinterpret_cast<int *>(0x009B7ACC);
    if (active == 0) {
        active = *reinterpret_cast<int *>(0x009B7AD0);
    }
    if (active == reinterpret_cast<int>(self)
        && *reinterpret_cast<int *>(0x009B7AD4) == 1) {
        void *const *vtable = *reinterpret_cast<void ***>(self);
        // The hover pair keeps the press: slot 17 re-enters the press paint.
        typedef void (Int2SlotObj::*Int2Fn)(int, int);
        union Int2Shim {
            Int2Fn fn;
            void *raw;
        };
        Int2Shim repaint;
        repaint.raw = vtable[17];
        (reinterpret_cast<Int2SlotObj *>(self)->*repaint.fn)(x, y);
        return;
    }
    char *const base = reinterpret_cast<char *>(self);
    if (*reinterpret_cast<int *>(base + 0xA74) == 1) {
        // The hold timer is BaseButton's time1_ at 0xA1C (time.h's own stop).
        reinterpret_cast<Time *>(base + 0xA1C)->stop();
        if (*reinterpret_cast<char **>(base + 0xA80) != 0) {
            Win::clear_bubble_text();
        }
        *reinterpret_cast<int *>(base + 0xA74) = 0;
        void *const buddy = *reinterpret_cast<void **>(
            reinterpret_cast<char *>(self) + 0xC4);
        if (buddy != 0) {
            void *const *buddy_vtable = *reinterpret_cast<void ***>(buddy);
            typedef void (Int2SlotObj::*NotifyFn)(int, int);
            union NotifyShim {
                NotifyFn fn;
                void *raw;
            };
            NotifyShim notify;
            notify.raw = buddy_vtable[40];
            (reinterpret_cast<Int2SlotObj *>(buddy)->*notify.fn)(
                *reinterpret_cast<int *>(base + 0xA78), 0);
        }
        NullSlotShim redraw;
        redraw.raw = (*reinterpret_cast<void ***>(self))[62];
        (reinterpret_cast<NullSlotObj *>(self)->*redraw.fn)();
    }
}
