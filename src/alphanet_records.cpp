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

int __stdcall compare_session_guid(const GUID *left, const GUID *right);

// Source-owned records and lifecycle mappings; this TU retains default /O2.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00402B40 SessionStruct::dt 0x00402B40-0x00402BA6 FILE
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00402b40.cpp: retained original spans and historical seed interpretation", "Pinned original receiver/table/call sequence reviewed in the Alpha ownership binding decision.", "Current C++ class/lifetime source owns the separately observed emitted symbol; this binding is unclaimed and preserves the independent historical exact seed."], "reason": "Receiver-1C, stages session-base669408/669404, clear402970, owner publication then bit1/free."}
// compile_context src/alphanet.cpp
// symbol    ??_GAlphaNetStringStruct@@UAEPAXI@Z
// ORIGINAL: 0x004E3300 sub_4e3300 0x004E3300-0x004E334B FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/recovered/units/004e3300.cpp: retained original spans and historical seed interpretation", "Pinned original receiver/table/call sequence reviewed in the Alpha ownership binding decision.", "Current C++ class/lifetime source owns the separately observed emitted symbol; this binding is unclaimed and preserves the independent historical exact seed.", "Genuine declared C++ destructor emits the independently named generated COFF body; original receiver/table/lifetime interpretation and current emitted ABI remain separate from the historical byte obligation.", "Pinned original picker0x633010/0x663730 and ledger0x4E3350/0x65C7F0 preserve separate ledger and base cleanup roles."], "reason": "Receiver-38 then calls4E3350; publishes ledger owner and conditionally frees complete object on bit1. Superseded scaffold retired after observing this independently emitted generated body; historical claims and findings transfer without a new matching or linked-preservation claim. Original picker and ledger normal/cold paths retain distinct derived/base cleanup stages. Class-defined unchanged destructors expose that nested lifetime; the callback-sensitive walker remains inline. The canonical records owner retains this identity; the named compilation context physically emits the body. Definition visibility and emission do not establish original TU, helper identity or linked selection."}
// compile_context src/alphanet.cpp
// symbol    ??_GAlphaNetLedger@@UAEPAXI@Z
// ORIGINAL: 0x004E3350 sub_4e3350 0x004E3350-0x004E34A8;0x0065C7F0-0x0065C805 FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:293937ab6f3be75999e0c0a4c9c110c02930624c:src/unrecovered/004e3350.cpp: retained original spans and historical seed interpretation", "Pinned original receiver/table/call sequence reviewed in the Alpha ownership binding decision.", "Current C++ class/lifetime source owns the separately observed emitted symbol; this binding is unclaimed and preserves the transferred historical seed findings and complete original spans.", "git:293937ab6f3be75999e0c0a4c9c110c02930624c:src/unrecovered/004e3350.cpp", "Historical seed spelling sub_4e3350; current source mapping sub_4e3350; emitted symbol ??1AlphaNetLedger@@UAE@XZ.", "Pinned original picker0x633010/0x663730 and ledger0x4E3350/0x65C7F0 preserve separate ledger and base cleanup roles."], "reason": "Receiver-38 stages session-derived tables, walk then session-base cleanup; retained65C7F0 EH span. Superseded seed retired after owner review; original spans, existing claims and useful seed findings remain in source. This cleanup grants no new match or acceptance claim. Original picker and ledger normal/cold paths retain distinct derived/base cleanup stages. Class-defined unchanged destructors expose that nested lifetime; the callback-sensitive walker remains inline. The canonical records owner retains this identity; the named compilation context physically emits the body. Definition visibility and emission do not establish original TU, helper identity or linked selection."}
// compile_context src/alphanet.cpp
// body src/alphanet.h
// symbol    ??1AlphaNetLedger@@UAE@XZ
// TRIED: no SEH-frame modelling attempted - the function installs a real fs:[0] frame (push -1; push 0x65c7fb; ...) whose unwind funclet at 0x0065C7F0 calls the real `SessionStruct::close` (0x00401ce0, see src/alphanet.cpp) on `this-0x1c`, matching the precedent at 0x004C86D0/0x00538D10: none of the calls in the body throws in practice, so the frame is unreachable and is dropped. `ecx` does real work (`lea edi,[ecx-0x38]`, no stack access) so this is __thiscall, written as `ClassX::sub_4e3350()` per the 0x004E3300 precedent, which already calls `sub->sub_4e3350()` on the same class. The body is TWO back-to-back rounds of the same list-teardown loop (own-stage vtable 0x66eae8/0x66eae4 first, then the same 0x669408/0x669404 pair `SessionStruct::close` itself uses), both operating on the SAME member at `this-0x38`; reproduced as two literal blocks (not a loop, matching the original's duplication) via a `VCall` shim for the two indirect calls per node (slot1 with `this=self,payload`, slot0 with an adjusted `this` and `arg=1`), simplified to skip the raw adjustor-thunk arithmetic (same simplification already accepted at 0x0062E010's `Net::close`). 0.496 mnemonic similarity - the dropped SEH prologue alone accounts for most of the gap - not chased further.
// ORIGINAL: 0x004E34B0 sub_4e34b0 0x004E34B0-0x004E34CB FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/recovered/units/004e34b0.cpp: retained original spans and historical seed interpretation", "Pinned original receiver/table/call sequence reviewed in the Alpha ownership binding decision.", "Current C++ class/lifetime source owns the separately observed emitted symbol; this binding is unclaimed and preserves the independent historical exact seed.", "Genuine declared C++ destructor emits the independently named generated COFF body; original receiver/table/lifetime interpretation and current emitted ABI remain separate from the historical byte obligation."], "reason": "Complete receiver+38 calls4E3350 then publishes virtual allocation owner; no delete flag/free. Superseded scaffold retired after observing this independently emitted generated body; historical claims and findings transfer without a new matching or linked-preservation claim."}
// compile_context src/alphanet.cpp
// symbol    ??_DAlphaNetLedger@@QAEXXZ

namespace {
// Source-only typed algorithms shared by genuine list owners. They own no
// executable identity. Each instantiated body/reference requires measurement;
// sharing source does not prove the original compiler folded any particular pair.
template<class List> inline typename List::Payload *next_owned_list(List *list) {
    if (list->head_ == 0) return 0;
    typename List::Entry *next = list->current_->next_;
    int position = list->current_position_;
    list->current_ = next;
    const int count = list->entry_count_;
    ++position;
    list->current_position_ = position;
    if (position == count) list->current_position_ = 0;
    return list->current_->payload_;
}

}

// The ledger's five list fields, zeroed; the rest of the construction (the
// vtable staging and the owner capture) belongs to the compiler.

// The base table's defaults are shared leaves, independently checked;
// the unrelated AlphaMovie catalogue identities remain unchanged.
// RECOVERY_SHARED: v1 0x00404250 0x00404255 ?on_insert@AlphaNetStringStruct@@UAEHPAVNetAllocationNode@@@Z 0x004E2490
int AlphaNetStringStruct::on_insert(NetAllocationNode *) { return 0; }
// RECOVERY_SHARED: v1 0x00404270 0x00404273 ?on_remove@AlphaNetStringStruct@@UAEXPAVNetAllocationNode@@@Z 0x004E2490
void AlphaNetStringStruct::on_remove(NetAllocationNode *) { }

// ORIGINAL: 0x005D4890 ?on_insert@AlphaNetLedger@@UAEHPAVNetAllocationNode@@@Z 0x005D4890-0x005D498D
// RECOVERY_BINDING: v1 {"evidence": ["Original table 0x66EAE8 slot 0 points here. Insertion 0x401D80 allocates a 0x60 NetAllocationNode, constructs its nested list and invokes this hook; enumeration 0x630C30 supplies DPSESSIONDESC2 fields. Seed extent is retained; this binding makes no matching claim."], "reason": "Recover the session-ledger insertion virtual from its retained seed using the existing allocation-owning payload class."}
// symbol    ?on_insert@AlphaNetLedger@@UAEHPAVNetAllocationNode@@@Z
int AlphaNetLedger::on_insert(NetAllocationNode *payload) {
    if (session_name_ == 0) {
        payload->session_name_ = 0;
    } else {
        char *copy;
        if (allocator_ == 0) {
            copy = static_cast<char *>(mem_get(strlen(session_name_) + 1));
        } else {
            const size_t length = strlen(session_name_) + 1;
            copy = static_cast<char *>(allocator_->get(length));
        }
        payload->session_name_ = copy;
        if (copy == 0) {
            return 4;
        }
        *copy = 0;
        strcat(payload->session_name_, session_name_);
    }
    if (password_ == 0) {
        payload->password_ = 0;
    } else {
        char *copy;
        if (allocator_ == 0) {
            copy = static_cast<char *>(mem_get(strlen(password_) + 1));
        } else {
            const size_t length = strlen(password_) + 1;
            copy = static_cast<char *>(allocator_->get(length));
        }
        payload->password_ = copy;
        if (copy == 0) {
            return 4;
        }
        *copy = 0;
        strcat(payload->password_, password_);
    }
    payload->instance_guid_ = *instance_guid_;
    payload->session_flags_ = session_flags_;
    payload->max_players_ = max_players_;
    payload->current_players_ = current_players_;
    payload->field_28_ = field_34_;
    return 0;
}

// ORIGINAL: 0x005D4990 ?on_remove@AlphaNetLedger@@UAEXPAVNetAllocationNode@@@Z 0x005D4990-0x005D49CE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Derived table 0x66EAE8 slot 1 points here. Original checks receiver allocator at +0x18 before freeing and nulling payload strings +4 and +8. The exact seed git:293937ab6f3be75999e0c0a4c9c110c02930624c:src/recovered/005d4990.cpp is retired; its existing restoration claim remains on this compiled owner.", "git:293937ab6f3be75999e0c0a4c9c110c02930624c:src/recovered/005d4990.cpp", "Historical seed spelling sub_5d4990; current source mapping ?on_remove@AlphaNetLedger@@UAEXPAVNetAllocationNode@@@Z; emitted symbol ?on_remove@AlphaNetLedger@@UAEXPAVNetAllocationNode@@@Z."], "reason": "Recover allocator-aware session payload cleanup as the real ledger virtual, retaining the historical exact restoration obligation in source. Superseded seed retired after owner review; original spans, existing claims and useful seed findings remain in source. This cleanup grants no new match or acceptance claim."}
void AlphaNetLedger::on_remove(NetAllocationNode *payload) {
    if (allocator_ == 0) {
        if (payload->session_name_ != 0) {
            free(payload->session_name_);
            payload->session_name_ = 0;
        }
        if (payload->password_ != 0) {
            free(payload->password_);
            payload->password_ = 0;
        }
    }
}

// Original AlphaNet teardown clears the list in both derived and base stages,
// then lets the virtual allocation base publish its saved owner. The former
// cleanup bridge is retired in favor of genuine list/member destruction.
// Typed payload fields and hooks still do not prove unseen factory lifetimes.











/*
Purpose: Build and link one selection node for the container this owns:
         the same two-stage allocate (0x1c node, 0x60 payload) with the
         process-heap fallback and the fatal box, both virtual-base table
         installs, the 0x401b80/0x402d10 list hooks, and the finished node
         handed to this's slot 0.

// ORIGINAL: 0x00401D80 sub_401d80 0x00401D80-0x0040230F;0x006504E0-0x00650571 FILE
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
        Heap *const payload_owner = allocator_;
        head_->payload_ = payload_owner != 0
            ? new(payload_owner) NetAllocationNode
            : new(this) NetAllocationNode;
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
        Heap *const payload_owner = allocator_;
        current_->payload_ = payload_owner != 0
            ? new(payload_owner) NetAllocationNode
            : new(this) NetAllocationNode;
        if (current_->payload_ == 0) return 4;
        on_insert(current_->payload_);
    }
    current_position_ = entry_count_;
    ++entry_count_;
    return 0;
}


/*
// ORIGINAL: 0x00401CE0 SessionStruct::close 0x00401CE0-0x00401D79 FILE
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


/*
Purpose: Sort one pass of the string list: walk the ring from the current
         position, select the smallest record under the three-way record
         comparator, and swap the winning entry's payload text and metadata
         into the walk position.
// ORIGINAL: 0x005D49D0 sub_5d49d0 0x005D49D0-0x005D4BA7 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Original 5D49D0-5D4BA7 and retained seed/history preserve the same identity and spans.", "The Alpha join call chain 4E2E50 uses session ledger enumeration 62E420 and comparator 5D4BB0 on payload GUID at +0xC. Circular receiver fields +8/+C/+10/+14 and entry next/previous +0xC/+0x10 establish a list protocol, not StringStruct nominal identity.", "Original winning-entry exchange swaps entry id +4 and payload pointer +8, not text/metadata inside a payload. Reviewed source uses AlphaNetLedgerEntry and NetAllocationNode::instance_guid_; count is reread each scan iteration."], "reason": "Recover the session-ledger GUID selection pass on AlphaNetStringStruct, keeping entry ID and payload ownership paired."}
// Historical superseded binding interpretation: {"evidence": ["Original5D49D0..5D4BA7; retired seed git:b18d3ac8:src/unrecovered/005d49d0.cpp records the same identity and spans.", "The receiver fields at +8/+C/+10/+14 are StringStruct's own head_/current_/entry_count_/current_position_ and the body calls StringStruct::seek_pos, so the receiver is typed as the list itself; the comparator is sub_5d4bb0 above, and the node walks use StringStructEntry's next(+0xC)/previous(+0x10) exactly as StringStruct::add installs them.", "The swapped payload words at +4/+8 are the payload's text pointer and first metadata word; the walk re-reads entry_count_ every iteration exactly as the seed measured."], "reason": "Coverage transcription of the list's selection pass on the typed StringStruct receiver; the payload internals stay documented offsets."}
// symbol    ?sort_sessions@AlphaNetStringStruct@@QAEXXZ
Status: Coverage transcription - unclaimed body
*/
void AlphaNetStringStruct::sort_sessions() {
    int total = entry_count_;
    int limit = total - 1;
    int walk_index = 0;
    if (limit <= 0) return;
    do {
        // Native inlines this same signed circular seek, including its abs
        // behavior; the callback-free scan starts only from valid list state.
        seek_pos(walk_index);
        const GUID *best = head_ == 0 ? 0 : &current_->payload_->instance_guid_;
        int best_index = walk_index;
        int scan_index = walk_index + 1;
        const int next_walk = scan_index;
        if (scan_index < total) {
            do {
                if (head_ != 0) {
                    current_ = current_->next_;
                    ++current_position_;
                    if (current_position_ == total) current_position_ = 0;
                }
                const GUID *candidate = 0;
                if (current_ != 0) {
                    // If count/head/current disagree, native may pass null+C
                    // to the comparator. This named-member expression retains
                    // that invalid-domain UB; it does not clamp or repair it.
                    NetAllocationNode *payload = head_ == 0 ? 0 : current_->payload_;
                    candidate = &payload->instance_guid_;
                }
                if (compare_session_guid(candidate, best) < 0) {
                    best = head_ == 0 ? 0 : &current_->payload_->instance_guid_;
                    best_index = current_position_;
                }
                total = entry_count_;
                ++scan_index;
            } while (scan_index < total);
            if (best_index != walk_index && best_index <= total - 1) {
                if (seek_pos(best_index) != 0) {
                    AlphaNetLedgerEntry *winner = current_;
                    if (seek_pos(walk_index) != 0) {
                        AlphaNetLedgerEntry *walked = current_;
                        const int saved_id = winner->id_;
                        NetAllocationNode *saved_payload = winner->payload_;
                        winner->id_ = walked->id_;
                        winner->payload_ = walked->payload_;
                        walked->id_ = saved_id;
                        walked->payload_ = saved_payload;
                    }
                }
            }
        }
        total = entry_count_;
        limit = total - 1;
        walk_index = next_walk;
    } while (walk_index < limit);
}


/*
Purpose: Order three-way comparison of two fixed-shape name records: an
         unsigned word, two unsigned shorts and eight bytes, each field
         compared lexicographically.
// ORIGINAL: 0x005D4BB0 sub_5d4bb0 0x005D4BB0-0x005D4C41 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original 5D4BB0-5D4C41 retains its two stack arguments, no calls, and unsigned comparisons: Data1 +0, Data2 +4, Data3 +6, then Data4 eight bytes at +8.", "Call chain 5D49D0 compares pointers to enumerated session instance GUID at payload +0xC; SDK session callback 630C30 and ledger insertion 5D4890 establish the GUID role. Field shape alone was insufficient to establish nominal identity.", "Native v3 objects independently emit compare_session_guid with stdcall GUID pointer arguments; this observes source emission, not original matching or complete owner acceptance."], "reason": "Recover the unsigned lexicographic GUID comparator used by the Alpha session-ledger selection pass."}
// Historical superseded binding interpretation: {"evidence": ["Original5D4BB0..5D4C41; retired seed git:b18d3ac8:src/unrecovered/005d4bb0.cpp records the same identity and spans.", "The body reads only its two stack arguments and makes no calls; the field walks (word at +0, shorts at +4/+6, eight bytes at +8) are transcribed exactly as the seed measured them, with no record type invented for them."], "reason": "Coverage transcription of the unnamed record comparator as a stdcall free function; no names exist for the record fields, so the reads stay spelled by offset."}
// symbol    ?compare_session_guid@@YGHPBU_GUID@@0@Z
Return Value: -1, 0 or 1 as the first record orders before, equal to or after the second
Status: Coverage transcription - unclaimed body
*/
int __stdcall compare_session_guid(const GUID *left, const GUID *right) {
    if (left->Data1 < right->Data1) return -1;
    if (left->Data1 != right->Data1) return 1;
    if (left->Data2 < right->Data2) return -1;
    if (left->Data2 != right->Data2) return 1;
    if (left->Data3 < right->Data3) return -1;
    if (left->Data3 != right->Data3) return 1;
    for (int byte = 0; byte < 8; ++byte) {
        if (left->Data4[byte] < right->Data4[byte]) return -1;
        if (left->Data4[byte] != right->Data4[byte]) return 1;
    }
    return 0;
}

void *AlphaNetLedgerEntry::operator new(size_t size, Heap *owner) throw() {
    return allocate_net_node(size, owner);
}
void *AlphaNetLedgerEntry::operator new(size_t size, AlphaNetStringStruct *list) throw() {
    return allocate_net_node(size, list->allocation_owner_);
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
