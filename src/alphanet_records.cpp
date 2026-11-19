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
#include "alphanet.h"
#include "stringstruct.h"
#include "heap.h"
#include "general.h"
#include <cstdlib>
#include <cstring>
#include "alphanet_inline.h"

// Source-owned records and lifecycle mappings; this TU retains default /O2.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00402B40 SessionStruct::dt 0x00402B40-0x00402BA6 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00402b40.cpp: retained original spans and historical seed interpretation", "Pinned original receiver/table/call sequence reviewed in the Alpha ownership binding decision.", "Current C++ class/lifetime source owns the separately observed emitted symbol; this binding is unclaimed and preserves the independent historical exact seed."], "reason": "Receiver-1C, stages session-base669408/669404, clear402970, owner publication then bit1/free."}
// compile_context src/alphanet.cpp
// symbol    ??_GAlphaNetStringStruct@@UAEPAXI@Z
// ORIGINAL: 0x004E3300 sub_4e3300 0x004E3300-0x004E334B FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:b2de7f29042d247ee9b448c814029bbbbc35f230:src/recovered/units/004e3300.cpp: retained original spans and historical seed interpretation", "Pinned original receiver/table/call sequence reviewed in the Alpha ownership binding decision.", "Current C++ class/lifetime source owns the separately observed emitted symbol; this binding is unclaimed and preserves the independent historical exact seed.", "Genuine declared C++ destructor emits the independently named generated COFF body; original receiver/table/lifetime interpretation and current emitted ABI remain separate from the historical byte obligation.", "Pinned original picker0x633010/0x663730 and ledger0x4E3350/0x65C7F0 preserve separate ledger and base cleanup roles."], "reason": "Receiver-38 then calls4E3350; publishes ledger owner and conditionally frees complete object on bit1. Superseded scaffold retired after observing this independently emitted generated body; historical claims and findings transfer without a new matching or linked-preservation claim. Original picker and ledger normal/cold paths retain distinct derived/base cleanup stages. Class-defined unchanged destructors expose that nested lifetime; the callback-sensitive walker remains inline. The canonical records owner retains this identity; the named compilation context physically emits the body. Definition visibility and emission do not establish original TU, helper identity or linked selection."}
// compile_context src/alphanet.cpp
// symbol    ??_GAlphaNetLedger@@UAEPAXI@Z
// ORIGINAL: 0x004E3350 sub_4e3350 0x004E3350-0x004E34A8;0x0065C7F0-0x0065C805 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:0e184947304cbb843b26264d5145e9b9b452cf16:src/unrecovered/004e3350.cpp: retained original spans and historical seed interpretation", "Pinned original receiver/table/call sequence reviewed in the Alpha ownership binding decision.", "Current C++ class/lifetime source owns the separately observed emitted symbol; this binding is unclaimed and preserves the transferred historical seed findings and complete original spans.", "git:0e184947304cbb843b26264d5145e9b9b452cf16:src/unrecovered/004e3350.cpp", "Historical seed spelling sub_4e3350; current source mapping sub_4e3350; emitted symbol ??1AlphaNetLedger@@UAE@XZ.", "Pinned original picker0x633010/0x663730 and ledger0x4E3350/0x65C7F0 preserve separate ledger and base cleanup roles."], "reason": "Receiver-38 stages session-derived tables, walk then session-base cleanup; retained65C7F0 EH span. Superseded seed retired after owner review; original spans, existing claims and useful seed findings remain in source. This cleanup grants no new match or acceptance claim. Original picker and ledger normal/cold paths retain distinct derived/base cleanup stages. Class-defined unchanged destructors expose that nested lifetime; the callback-sensitive walker remains inline. The canonical records owner retains this identity; the named compilation context physically emits the body. Definition visibility and emission do not establish original TU, helper identity or linked selection."}
// compile_context src/alphanet.cpp
// body src/alphanet.h
// symbol    ??1AlphaNetLedger@@UAE@XZ
// TRIED: no SEH-frame modelling attempted - the function installs a real fs:[0] frame (push -1; push 0x65c7fb; ...) whose unwind funclet at 0x0065C7F0 calls the real `SessionStruct::close` (0x00401ce0, see src/alphanet.cpp) on `this-0x1c`, matching the precedent at 0x004C86D0/0x00538D10: none of the calls in the body throws in practice, so the frame is unreachable and is dropped. `ecx` does real work (`lea edi,[ecx-0x38]`, no stack access) so this is __thiscall, written as `ClassX::sub_4e3350()` per the 0x004E3300 precedent, which already calls `sub->sub_4e3350()` on the same class. The body is TWO back-to-back rounds of the same list-teardown loop (own-stage vtable 0x66eae8/0x66eae4 first, then the same 0x669408/0x669404 pair `SessionStruct::close` itself uses), both operating on the SAME member at `this-0x38`; reproduced as two literal blocks (not a loop, matching the original's duplication) via a `VCall` shim for the two indirect calls per node (slot1 with `this=self,payload`, slot0 with an adjusted `this` and `arg=1`), simplified to skip the raw adjustor-thunk arithmetic (same simplification already accepted at 0x0062E010's `Net::close`). 0.496 mnemonic similarity - the dropped SEH prologue alone accounts for most of the gap - not chased further.
// ORIGINAL: 0x004E34B0 sub_4e34b0 0x004E34B0-0x004E34CB FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:b2de7f29042d247ee9b448c814029bbbbc35f230:src/recovered/units/004e34b0.cpp: retained original spans and historical seed interpretation", "Pinned original receiver/table/call sequence reviewed in the Alpha ownership binding decision.", "Current C++ class/lifetime source owns the separately observed emitted symbol; this binding is unclaimed and preserves the independent historical exact seed.", "Genuine declared C++ destructor emits the independently named generated COFF body; original receiver/table/lifetime interpretation and current emitted ABI remain separate from the historical byte obligation."], "reason": "Complete receiver+38 calls4E3350 then publishes virtual allocation owner; no delete flag/free. Superseded scaffold retired after observing this independently emitted generated body; historical claims and findings transfer without a new matching or linked-preservation claim."}
// compile_context src/alphanet.cpp
// symbol    ??_DAlphaNetLedger@@QAEXXZ

// Source-only typed algorithms shared by genuine list owners. They own no
// executable identity. Each instantiated body/reference requires measurement;
// sharing source does not prove the original compiler folded any particular pair.
template<class List> inline typename List::Payload *next_owned_list(List *list) {
    if (list->head_ == 0) return 0;
    typename List::Entry *next = list->current_->next_;
    list->current_ = next;
    const int count = list->entry_count_;
    const int position = ++list->current_position_;
    if (position == count) list->current_position_ = 0;
    return list->current_->payload_;
}


// The ledger's five list fields, zeroed; the rest of the construction (the
// vtable staging and the owner capture) belongs to the compiler.

// The base table's defaults are shared leaves, independently checked;
// the unrelated AlphaMovie catalogue identities remain unchanged.
// RECOVERY_SHARED: v1 0x00404250 0x00404255 ?on_insert@AlphaNetStringStruct@@UAEHPAVNetAllocationNode@@@Z 0x004E2490
int AlphaNetStringStruct::on_insert(NetAllocationNode *) { return 0; }
// RECOVERY_SHARED: v1 0x00404270 0x00404273 ?on_remove@AlphaNetStringStruct@@UAEXPAVNetAllocationNode@@@Z 0x004E2490
void AlphaNetStringStruct::on_remove(NetAllocationNode *) { }

// Original AlphaNet teardown clears the list in both derived and base stages,
// then lets the virtual allocation base publish its saved owner. The former
// cleanup bridge is retired in favor of genuine list/member destruction.
// Typed payload fields and hooks still do not prove unseen factory lifetimes.











// The inline allocators' fatal-box text is one linker-merged literal pair
// shared by every unit that expands them (stringstruct, alphanet_records,
// net_class): the image pushes 0x00682700 and 0x00682724 from all of them.
// RECOVERY_LITERAL: v2 merged 0x00682700 0x00682722 ??_C@_0CC@JCEM@out?5of?5heap?5space?5in?5new?5functio@ 0x00401D80
// RECOVERY_LITERAL: v2 merged 0x00682724 0x0068272D ??_C@_08JLBK@Aborting?$AA@ 0x00401D80
// WORKAROUND(inline): the payload's fallback placement argument is a value,
// not an lvalue: the cleanup for the second payload fallback (state 2,
// 0x0065050E) pushes the owner the body spilled at 0x00401F4A, and the last
// one keeps it in [ebp+8] (0x0040220B); `new(allocation_owner_)` re-reads the
// virtual base in the funclet instead. Same shape as JackalVoiceRx::add_player
// (voice_owner, net_class.cpp). The original's spelling is not known.
static inline Heap *ledger_owner(AlphaNetStringStruct *list) { return list->allocation_owner_; }

/*
Purpose: Build and link one selection node for the container this owns:
         the same two-stage allocate (0x1c node, 0x60 payload) with the
         process-heap fallback and the fatal box, both virtual-base table
         installs, the 0x401b80/0x402d10 list hooks, and the finished node
         handed to this's slot 0.

// ORIGINAL: 0x00401D80 sub_401d80 0x00401D80-0x0040230F;0x006504E0-0x00650571 FILE
// LEVER  2026-11-19: EH states. The image's unwind map has six states, three
//        payload sites of two (placement delete, then the node's virtual
//        base under its construction flag 4 / 8 / 0x40). State 2's delete
//        pushes the spilled owner VALUE ([ebp-0x14], written at 0x00401F4A);
//        `new(allocation_owner_)` there made the funclet re-read the owner
//        through the virtual base (five instructions the image lacks, and a
//        cleanup action `recover measure` refused to decode). With the
//        first-branch fallback taking a block-scope copy the unwind region
//        0x006504E0 is the image's.
// TRIED: the residual is the second branch's shared tail. The image writes
//        both payload arms and then ONE `if (current_->payload_ == 0) return
//        4; on_insert(...)` (0x004021F8 `jmp 0x4022C7`); here each arm keeps
//        its own copy. Written merged, the last payload site stops
//        expanding NetUnk72C() and gains two EH states (8 against 6): the
//        image is the suffix regime of MATCHING_GUIDE 2.8 (third payload
//        site calls 0x00402D10, the fourth expands it), while every merged
//        spelling here is either "neither expands" or, with a larger caller,
//        the prefix regime (third expands, fourth calls). Tried merged:
//        new(this) / ledger_owner(this) / a block-scope copy / the lvalue at
//        either fallback, both ternary forms, the arms reversed, free and
//        charged dead probes at every statement boundary. The duplicated
//        tail is kept because it keeps the EH states right.
// TRIED: reproducing the /GX SEH prologue (push -1/funclet/fs:[0] chain) by hand - the earlier lever (a real local object with a destructor makes the compiler emit its own matching frame) needs the ACTUAL class whose ctor/dtor this uses, which is not recoverable from this file alone; landed the full two-branch allocate-and-link body (both the 0x1c-node and 0x60-node paths, both empty/non-empty container cases) as plain control flow instead, diverging from instruction #5 (no SEH frame set up) but matching the field offsets and call order throughout.
// size      1568 bytes
// prototype
// callers   1   call targets   6
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00401B80 0x00402320 0x00402D10 0x005D4510 0x005D4680 0x00644DFF
// symbol   ?add@AlphaNetStringStruct@@QAEHH@Z
// indirect  0x00401E0D 0x00401E49 0x00401F2F 0x00401F6E 0x00402017 0x0040207B 0x004020B7 0x004021DC 0x00402232 0x004022EE
Status: Coverage transcription - unclaimed body; the two list hooks link
        through the continuity placeholders above, and the seed's
        instruction-#5 SEH-frame divergence stands.
*/
// WORKAROUND(inline): frame-pointer stand-in. The image's functions in this
// unit from the 0x00401xxx-0x00402xxx run (0x00401CE0, 0x00401D80,
// 0x00402B40) and 0x004E3300/0x004E3350 keep an EBP frame, while the
// 0x005D4xxx ones omit it; this TU's recipe (/Oy) only reproduces the latter.
// add() is EH-framed in the image (push ebp; push -1; ...) and its cleanup
// actions address the frame through EBP (0x006504E0).
int AlphaNetStringStruct::add(int id) {
    // The cold actions at 0x006504E0 unwind a partially constructed heap
    // payload, including its virtual allocation base at +0x58. Keep typed
    // new expressions here so construction-failure cleanup belongs to this
    // caller; the seed history's stack-local hypothesis is superseded.
    if (entry_count_ == 0) {
        Heap *const entry_owner = allocator_;
        head_ = entry_owner != 0
            ? new(entry_owner) AlphaNetLedgerEntry
            : new(this) AlphaNetLedgerEntry;
        if (head_ == 0) return 4;
        head_->id_ = id;
        head_->next_ = head_;
        head_->previous_ = head_;
        // If/else, as ServiceProviderList::add: the image tail-merges both
        // arms into one ??0NetUnk72C call (0x00401FCE).
        // The fallback's placement argument is a value: the image spills it
        // at 0x00401F4A and the state-2 cleanup (0x0065050E) pushes that
        // slot. A block-scope copy gives the value without ledger_owner()'s
        // /Ob2 cost, which would stop the last payload site expanding
        // NetUnk72C() (0x00402270).
        if (allocator_) head_->payload_ = new(allocator_) NetAllocationNode;
        else { Heap *const owner = allocation_owner_; head_->payload_ = new(owner) NetAllocationNode; }
        if (head_->payload_ == 0) return 4;
        on_insert(head_->payload_);
        current_ = head_;
    } else {
        Heap *const entry_owner = allocator_;
        head_->previous_->next_ = entry_owner != 0
            ? new(entry_owner) AlphaNetLedgerEntry
            : new(this) AlphaNetLedgerEntry;
        if (head_->previous_->next_ == 0) return 4;
        head_->previous_->next_->previous_ = head_->previous_;
        head_->previous_->next_->next_ = head_;
        head_->previous_ = head_->previous_->next_;
        current_ = head_->previous_;
        current_->id_ = id;
        if (allocator_ != 0) {
            current_->payload_ = new(allocator_) NetAllocationNode;
            if (current_->payload_ == 0) return 4;
            on_insert(current_->payload_);
        } else {
            current_->payload_ = new(ledger_owner(this)) NetAllocationNode;
            if (current_->payload_ == 0) return 4;
            on_insert(current_->payload_);
        }
    }
    current_position_ = entry_count_;
    ++entry_count_;
    return 0;
}


/*
// ORIGINAL: 0x00401CE0 SessionStruct::close 0x00401CE0-0x00401D79 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original picker0x633010/0x663730 and ledger0x4E3350/0x65C7F0 preserve separate ledger and base cleanup roles."], "reason": "Original picker and ledger normal/cold paths retain distinct derived/base cleanup stages. Class-defined unchanged destructors expose that nested lifetime; the callback-sensitive walker remains inline. The canonical records owner retains this identity; the named compilation context physically emits the body. Definition visibility and emission do not establish original TU, helper identity or linked selection."}
// compile_context src/alphanet.cpp
// body src/alphanet.h
// symbol    ??1AlphaNetStringStruct@@UAE@XZ
// size      153 bytes
// prototype 
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00401D22 0x00401D34 0x00401D50
Status: Complete
*/


void *AlphaNetLedgerEntry::operator new(size_t size, Heap *owner) throw() {
    void *memory = owner->get(size);
    if (memory != 0) {
        StringAllocationHeap = owner;
        return memory;
    }
    MessageBoxA(0, "out of heap space in new function", "Aborting", MB_OK);
    exit(3);
    return 0;
}
void *AlphaNetLedgerEntry::operator new(size_t size, AlphaNetStringStruct *list) throw() {
    void *memory;
    Heap *owner = list->allocation_owner_;
    if (owner != 0) {
        memory = owner->get(size);
        if (memory != 0) {
            StringAllocationHeap = owner;
            return memory;
        }
        MessageBoxA(0, "out of heap space in new function", "Aborting", MB_OK);
        exit(3);
    }
    memory = mem_get(size);
    if (memory == 0) return 0;
    StringAllocationHeap = owner;
    return memory;
}
void AlphaNetLedgerEntry::operator delete(void *memory, Heap *) {
    cleanup_failed_net_construction(memory);
}
void AlphaNetLedgerEntry::operator delete(void *memory, AlphaNetStringStruct *) {
    cleanup_failed_net_construction(memory);
}
void AlphaNetLedgerEntry::operator delete(void *memory) {
    cleanup_failed_net_construction(memory);
}

NetAllocationNode *AlphaNetStringStruct::next_entry() { return next_owned_list(this); }
ServiceStruct *ServiceProviderList::next_entry() { return next_owned_list(this); }

// ~NetTextListBase belongs to this game unit: ??_GNetTextListBase 0x00402AD0
// and ~NetUnk72C 0x004023A0 here expand it, while ~Net, ~AlphaNet and
// ~NetDaemon call 0x00401BE0 out of line.
/*
// ORIGINAL: 0x00401BE0 sub_401be0 0x00401BE0-0x00401C79 BYTE_EXACT
// symbol    ??1NetTextListBase@@UAE@XZ
// size      153 bytes
// prototype
// callers   3   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
// indirect  0x00401C22 0x00401C34 0x00401C50
Status: Complete
*/
// The receiver arrives in ECX pointing 0x1c into the real object - a
// virtual-base subobject - so the body first recovers the object's own
// start (`self`). It then writes two vtable pointers: its own, and a
// second one reached through a displacement read out of a small table at
// self+4 (a vbtable: entry [1] holds the byte offset from self+4 to the
// virtual base's vtable slot, because that offset is only known once the
// most-derived class is). The same "read a vtable, take its [1] entry as
// a THIS-adjustment rather than a function, add it to the pointer, then
// call through the adjusted object's own vtable[0]" shape recurs for
// every node torn down in the loop below - that is the scalar deleting
// destructor called through a pointer typed as a virtual base, which is
// why the adjustment has to be data instead of a compile-time constant.
//
// MISMATCH, 52/64 (0.984 similar), best over every flag set this tree
// tries. The one surviving divergence is the SECOND per-node teardown
// call (deleting the list node itself, `node` below): the image computes
// the adjusted `this` with `lea ecx,[edx+eax]` and then re-derives the
// target vtable via `[edx+eax]` addressing instead of reading `[ecx]`,
// where the FIRST teardown call (on `sub_obj`, same shape) compiles
// byte-exact with `add`+`[ecx]`. TRIED: a named `int *vtbl`/`int
// disp` pair, a fused single expression, and routing the call through
// `vtable_method` all reproduce the identical `add`+`[ecx]` form as the
// first call - the allocator's choice between the two forms tracks
// register pressure at the call site, not source spelling, and no
// spelling tried moves it. Everything else - both vtable-pointer writes,
// the loop's trip count re-read each iteration, the first per-node call,
// and the trailing field clears - is byte-exact.
class SubBase401be0 { public:
    virtual void slot0(int);   // vtable[0]: scalar-deleting-destructor style
    virtual void slot1(int);   // vtable[4]: notifies `self` a node is going
};

NetTextListBase::~NetTextListBase() {
    close(); // also when empty; current_ is NOT cleared
}

// cleanup_failed_net_construction and NetAllocationNode's deletes belong to
// this game unit too: ??_GNetAllocationNode 0x00402CC0 expands both levels.
/*
// ORIGINAL: 0x00402320 sub_402320 0x00402320-0x00402349 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:30611dd8dcac1d36c7ad6321170c4bf4cc359561:src/recovered/00402320.cpp", "pinned:terranx_original:0x402320", "source:src/net_class.cpp"], "reason": "Original cleanup preserves StringAllocationHeap on null input; otherwise free only when owner is null, then clear handoff. Real Net/AlphaNet placement-delete callers discard its result. Indirect return consumers remain unverified. Preserve all original spans, names, claims and seed discoveries; no new byte grant."}
// symbol    ?cleanup_failed_net_construction@@YAXPAX@Z
*/
void cleanup_failed_net_construction(void *memory) {
    if (memory != 0) {
        if (StringAllocationHeap == 0) free(memory);
        StringAllocationHeap = 0;
    }
    // Native 402320 leaves the handoff untouched when memory is null.
}
void NetAllocationNode::operator delete(void *memory, Heap *) {
    cleanup_failed_net_construction(memory);
}
void NetAllocationNode::operator delete(void *memory, AlphaNetStringStruct *) {
    cleanup_failed_net_construction(memory);
}
void NetAllocationNode::operator delete(void *memory) {
    cleanup_failed_net_construction(memory);
}

// ServiceProviderList's constructor and add are this game unit's too: both
// keep the unit's ebp frame, and Jackal's Net code calls 0x00401660 and
// 0x00401760 out of line.
/*
// ORIGINAL: 0x00401660 ??0ServiceStruct@@QAE@H@Z 0x00401660-0x004016B8 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00401660.cpp: retained original spans and historical seed interpretation", "Pinned original receiver/table/call sequence reviewed in the Alpha ownership binding decision.", "Current C++ class/lifetime source owns the separately observed emitted symbol; this binding is unclaimed and preserves the independent historical exact seed."], "reason": "Original provider base tables6693D4/6693D0 and five cleared receiver fields; legacy ServiceStruct catalogue name is not payload ownership."}
// symbol    ??0ServiceProviderList@@QAE@XZ
// ORIGINAL: 0x00401760 ?add@ServiceStruct@@QAEHH@Z 0x00401760-0x00401B73 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:0e184947304cbb843b26264d5145e9b9b452cf16:src/unrecovered/00401760.cpp: retained original spans and historical seed interpretation", "Pinned original receiver/table/call sequence reviewed in the Alpha ownership binding decision.", "Current C++ class/lifetime source owns the separately observed emitted symbol; this binding is unclaimed and preserves the transferred historical seed findings and complete original spans.", "git:0e184947304cbb843b26264d5145e9b9b452cf16:src/unrecovered/00401760.cpp", "Historical seed spelling ?add@ServiceStruct@@QAEHH@Z; current source mapping ?add@ServiceStruct@@QAEHH@Z; emitted symbol ?add@ServiceProviderList@@QAEHH@Z."], "reason": "Original allocates entry1C/payload20 and invokes provider hook. Superseded seed retired after owner review; original spans, existing claims and useful seed findings remain in source. This cleanup grants no new match or acceptance claim."}
// TRIED: goto-preserving transcription of the Ghidra decompile (3-tier Heap::get/mem_get fallback, twice per node-type A/B, VCall slot0 insert callback); diverges at insn #7 in the prologue/local register allocation, not in the control-flow shape
// symbol    ?add@ServiceProviderList@@QAEHH@Z
*/
// ServiceProviderList::add (0x00401760) allocates each node
// `allocator_ ? new(allocator_) T : new(this) T` (payloads as if/else, whose
// explicit-heap arm keeps its own construction copy, 0x00401863): the Heap
// overload is fatal on failure; the list overload tries the inherited owner
// (fatal on failure), then mem_get. Each fatal block sits directly before
// the next arm's code in the image.
inline void *ServiceStruct::operator new(size_t size, Heap *owner) throw() {
    void *memory = owner->get(size);
    if (memory != 0) {
        StringAllocationHeap = owner;
        return memory;
    }
    MessageBoxA(0, "out of heap space in new function", "Aborting", MB_OK);
    exit(3);
    return 0;
}
inline void *ServiceStruct::operator new(size_t size, ServiceProviderList *list) throw() {
    void *memory;
    Heap *owner = list->allocation_owner_;
    if (owner != 0) {
        memory = owner->get(size);
        if (memory != 0) {
            StringAllocationHeap = owner;
            return memory;
        }
        MessageBoxA(0, "out of heap space in new function", "Aborting", MB_OK);
        exit(3);
    }
    memory = mem_get(size);
    if (memory == 0) return 0;
    StringAllocationHeap = owner;
    return memory;
}
void ServiceStruct::operator delete(void *memory, Heap *) { cleanup_failed_net_construction(memory); }
void ServiceStruct::operator delete(void *memory, ServiceProviderList *) { cleanup_failed_net_construction(memory); }
void ServiceStruct::operator delete(void *memory) { cleanup_failed_net_construction(memory); }

inline void *ServiceListEntry::operator new(size_t size, Heap *owner) throw() {
    void *memory = owner->get(size);
    if (memory != 0) {
        StringAllocationHeap = owner;
        return memory;
    }
    MessageBoxA(0, "out of heap space in new function", "Aborting", MB_OK);
    exit(3);
    return 0;
}
inline void *ServiceListEntry::operator new(size_t size, ServiceProviderList *list) throw() {
    void *memory;
    Heap *owner = list->allocation_owner_;
    if (owner != 0) {
        memory = owner->get(size);
        if (memory != 0) {
            StringAllocationHeap = owner;
            return memory;
        }
        MessageBoxA(0, "out of heap space in new function", "Aborting", MB_OK);
        exit(3);
    }
    memory = mem_get(size);
    if (memory == 0) return 0;
    StringAllocationHeap = owner;
    return memory;
}
void ServiceListEntry::operator delete(void *memory, Heap *) { cleanup_failed_net_construction(memory); }
void ServiceListEntry::operator delete(void *memory, ServiceProviderList *) { cleanup_failed_net_construction(memory); }
void ServiceListEntry::operator delete(void *memory) { cleanup_failed_net_construction(memory); }

ServiceProviderList::ServiceProviderList() {
    head_ = 0;
    current_ = 0;
    entry_count_ = 0;
    current_position_ = 0;
    allocator_ = 0;
}

int ServiceProviderList::on_insert(ServiceStruct *) { return 0; }

void ServiceProviderList::on_remove(ServiceStruct *) { }

// ServiceProviderList::seek_pos is inline in net_class.h; next_entry lives in
// alphanet_records.cpp (choose_service and pick_service call it out of line).

// Original401760 installs6693C4 at each entry's allocation base (+0x14).
// Its deleting slot targets402C20, shared with StringStructEntry. The real
// constructions below emit this additional deleting entry; the canonical
// ORIGINAL and historical claim remain in stringstruct.cpp.
// RECOVERY_SHARED: v1 0x00402C20 0x00402C62 ??_GServiceListEntry@@UAEPAXI@Z 0x00401760
int ServiceProviderList::add(int id) {
    if (entry_count_ == 0) {
        head_ = allocator_ ? new(allocator_) ServiceListEntry : new(this) ServiceListEntry;
        if (head_ == 0) return 4;
        head_->id_ = id;
        head_->next_ = head_;
        head_->previous_ = head_;
        if (allocator_) head_->payload_ = new(allocator_) ServiceStruct;
        else head_->payload_ = new(this) ServiceStruct;
        if (head_->payload_ == 0) return 4;
        on_insert(head_->payload_);
        current_ = head_;
    } else {
        head_->previous_->next_ = allocator_ ? new(allocator_) ServiceListEntry : new(this) ServiceListEntry;
        if (head_->previous_->next_ == 0) return 4;
        head_->previous_->next_->previous_ = head_->previous_;
        head_->previous_->next_->next_ = head_;
        head_->previous_ = head_->previous_->next_;
        current_ = head_->previous_;
        current_->id_ = id;
        if (allocator_) current_->payload_ = new(allocator_) ServiceStruct;
        else current_->payload_ = new(this) ServiceStruct;
        if (current_->payload_ == 0) return 4;
        on_insert(current_->payload_);
    }
    current_position_ = entry_count_;
    ++entry_count_;
    return 0;
}
