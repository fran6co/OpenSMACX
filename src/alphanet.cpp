/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00401C80 SessionStruct::SessionStruct 0x00401C80-0x00401CD8 FILE
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00401c80.cpp: retained original spans and historical seed interpretation", "Pinned original receiver/table/call sequence reviewed in the Alpha ownership binding decision.", "Current C++ class/lifetime source owns the separately observed emitted symbol; this binding is unclaimed and preserves the independent historical exact seed."], "reason": "Session base tables669408/669404 and five cleared circular-list fields."}
// symbol    ??0AlphaNetStringStruct@@QAE@XZ
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00402B40 SessionStruct::dt 0x00402B40-0x00402BA6 FILE
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00402b40.cpp: retained original spans and historical seed interpretation", "Pinned original receiver/table/call sequence reviewed in the Alpha ownership binding decision.", "Current C++ class/lifetime source owns the separately observed emitted symbol; this binding is unclaimed and preserves the independent historical exact seed."], "reason": "Receiver-1C, stages session-base669408/669404, clear402970, owner publication then bit1/free."}
// symbol    ??_GAlphaNetStringStruct@@UAEPAXI@Z
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004E2400 sub_4e2400 0x004E2400-0x004E2444 FILE
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/004e2400.cpp: retained original spans and historical seed interpretation", "Pinned original receiver/table/call sequence reviewed in the Alpha ownership binding decision.", "Current C++ class/lifetime source owns the separately observed emitted symbol; this binding is unclaimed and preserves the independent historical exact seed."], "reason": "Writes selected bytes at90E8E0..EB, leaving unwritten bytes intact."}
// symbol    ?reset_net_session_options@@YAXXZ
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004E2460 sub_4e2460 0x004E2460-0x004E248F FILE
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/004e2460.cpp: retained original spans and historical seed interpretation", "Pinned original receiver/table/call sequence reviewed in the Alpha ownership binding decision.", "Current C++ class/lifetime source owns the separately observed emitted symbol; this binding is unclaimed and preserves the independent historical exact seed."], "reason": "Eight rows, stride17C at90DB98; selected byte/DWORD stores only."}
// symbol    ?reset_net_lobby_rows@@YAXXZ
// ORIGINAL: 0x004E29E0 ?do_join_or_create@AlphaNet@@QAEHXZ 0x004E29E0-0x004E2A2F FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/recovered/units/004e29e0.cpp: retained original spans and historical seed interpretation", "Pinned original receiver/table/call sequence reviewed in the Alpha ownership binding decision.", "Current C++ class/lifetime source owns the separately observed emitted symbol; this binding is unclaimed and preserves the independent historical exact seed.", "git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/recovered/units/004e29e0.cpp"], "reason": "Existing real xpops4args wrapper drives whole create/join operation. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
// kind      game
// symbol    ?do_join_or_create@AlphaNet@@QAEHXZ
// ORIGINAL: 0x004E2A30 ?create@AlphaNet@@QAEHXZ 0x004E2A30-0x004E2E4B;0x0065C664-0x0065C771 FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:293937ab6f3be75999e0c0a4c9c110c02930624c:src/unrecovered/004e2a30.cpp: retained original spans and historical seed interpretation", "Pinned original receiver/table/call sequence reviewed in the Alpha ownership binding decision.", "Current C++ class/lifetime source owns the separately observed emitted symbol; this binding is unclaimed and preserves the transferred historical seed findings and complete original spans.", "git:293937ab6f3be75999e0c0a4c9c110c02930624c:src/unrecovered/004e2a30.cpp", "Historical seed spelling ?create@AlphaNet@@QAEHXZ; current source mapping ?create@AlphaNet@@QAEHXZ; emitted symbol ?create@AlphaNet@@QAEHXZ."], "reason": "Create flow with typed node/session fields; preserve complete seed EH spans. Superseded seed retired after owner review; original spans, existing claims and useful seed findings remain in source. This cleanup grants no new match or acceptance claim."}
// symbol    ?create@AlphaNet@@QAEHXZ
// Superseded seed deleted; seed paths below are historical Git provenance.
// Historical seed observations (not fresh measurements or acceptance):
// TRIED: automatic Popup dtor cascade for the SEH frame (LEVER #1); direct pushes for start()/X_pop() arg order re-derived from disasm; join_session/positioning logic transcribed twice (matches two inlined copies in the original). Diverges at instruction #15 (register alloc order for the shared cleanup path), not re-derived further.
// TRIED: mirrors the recovered sibling ?join@AlphaNet@@QAEHXZ (src/unrecovered/004e2e50.cpp) - a stack `Popup popup;` gives the SEH frame/alloca/dtor cascade for free (LEVER #1). Reaches MISMATCH #10 (`lea` vs `push`, a register-allocation detail a few instructions into the body) with this shape. Measured: capturing join_session's result AND the *g_0093d4f0 read into two named locals and branching/returning from BOTH (`int savedResult = *g_0093d4f0; if(joined==0){...return 0;} return savedResult;`) makes the compiler drop the SEH frame entirely (regresses to MISMATCH #0, `push`->`mov` at the very first instruction) even though the ORIGINAL disassembly has this exact duplicated-cleanup shape; inlining the join_session call straight into the `if` condition and reading *g_0093d4f0 only on the return statement (no intermediate locals) keeps the frame. Not chased further - a VC6 /GX codegen threshold this pass's budget does not cover.
// ORIGINAL: 0x004E2E50 ?join@AlphaNet@@QAEHXZ 0x004E2E50-0x004E31D6;0x0065C771-0x0065C7EC FILE
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/004e2e50.cpp: retained original spans and historical seed interpretation", "Pinned original receiver/table/call sequence reviewed in the Alpha ownership binding decision.", "Current C++ class/lifetime source owns the separately observed emitted symbol; this binding is unclaimed and preserves the independent historical exact seed."], "reason": "Session enumeration/cursor/selection flow; negative popup branch bypasses seek in original and reviewed correction."}
// symbol    ?join@AlphaNet@@QAEHXZ
// Superseded seed deleted; seed paths below are historical Git provenance.
// Historical seed observations (not fresh measurements or acceptance):
// TRIED: outer `if (a1 == 0) { big-init-block } else { field_D8_ |= 1 }` (ghidra's polarity, not the naive `!= 0`) plus a duplicated success-tail (field_48_/4C_ writes + return 0) inside the `if (rc >= 0)` arm rather than one shared tail reached via a unified `if (r2 == 0) return 1;` after the join/create if-else - 0.73 -> 0.98 mnemonic similarity. AlphaNet reaches Net::close()/ init() through reinterpret_cast<Net*>(this) (a zero-offset base).
// TRIED: one remaining `test/je` original has on the join() path only (create() path just has `test/jne`) that a single shared `if (r2 != 0)` after the join/create if-else does not reproduce; not chased further.
// ORIGINAL: 0x004E31E0 ?setup@AlphaNet@@QAEHH@Z 0x004E31E0-0x004E32BA FILE
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/004e31e0.cpp: retained original spans and historical seed interpretation", "Pinned original receiver/table/call sequence reviewed in the Alpha ownership binding decision.", "Current C++ class/lifetime source owns the separately observed emitted symbol; this binding is unclaimed and preserves the independent historical exact seed."], "reason": "Existing actual LobbyAppGuid689218 and provider selection/setup state."}
// symbol    ?setup@AlphaNet@@QAEHH@Z
// ORIGINAL: 0x004E3300 sub_4e3300 0x004E3300-0x004E334B FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/recovered/units/004e3300.cpp: retained original spans and historical seed interpretation", "Pinned original receiver/table/call sequence reviewed in the Alpha ownership binding decision.", "Current C++ class/lifetime source owns the separately observed emitted symbol; this binding is unclaimed and preserves the independent historical exact seed.", "Genuine declared C++ destructor emits the independently named generated COFF body; original receiver/table/lifetime interpretation and current emitted ABI remain separate from the historical byte obligation."], "reason": "Receiver-38 then calls4E3350; publishes ledger owner and conditionally frees complete object on bit1. Superseded scaffold retired after observing this independently emitted generated body; historical claims and findings transfer without a new matching or linked-preservation claim."}
// symbol    ??_GAlphaNetLedger@@UAEPAXI@Z
// ORIGINAL: 0x004E3350 sub_4e3350 0x004E3350-0x004E34A8;0x0065C7F0-0x0065C805 FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:293937ab6f3be75999e0c0a4c9c110c02930624c:src/unrecovered/004e3350.cpp: retained original spans and historical seed interpretation", "Pinned original receiver/table/call sequence reviewed in the Alpha ownership binding decision.", "Current C++ class/lifetime source owns the separately observed emitted symbol; this binding is unclaimed and preserves the transferred historical seed findings and complete original spans.", "git:293937ab6f3be75999e0c0a4c9c110c02930624c:src/unrecovered/004e3350.cpp", "Historical seed spelling sub_4e3350; current source mapping sub_4e3350; emitted symbol ??1AlphaNetLedger@@UAE@XZ."], "reason": "Receiver-38 stages session-derived tables, walk then session-base cleanup; retained65C7F0 EH span. Superseded seed retired after owner review; original spans, existing claims and useful seed findings remain in source. This cleanup grants no new match or acceptance claim."}
// TRIED: no SEH-frame modelling attempted - the function installs a real fs:[0] frame (push -1; push 0x65c7fb; ...) whose unwind funclet at 0x0065C7F0 calls the real `SessionStruct::close` (0x00401ce0, see src/alphanet.cpp) on `this-0x1c`, matching the precedent at 0x004C86D0/0x00538D10: none of the calls in the body throws in practice, so the frame is unreachable and is dropped. `ecx` does real work (`lea edi,[ecx-0x38]`, no stack access) so this is __thiscall, written as `ClassX::sub_4e3350()` per the 0x004E3300 precedent, which already calls `sub->sub_4e3350()` on the same class. The body is TWO back-to-back rounds of the same list-teardown loop (own-stage vtable 0x66eae8/0x66eae4 first, then the same 0x669408/0x669404 pair `SessionStruct::close` itself uses), both operating on the SAME member at `this-0x38`; reproduced as two literal blocks (not a loop, matching the original's duplication) via a `VCall` shim for the two indirect calls per node (slot1 with `this=self,payload`, slot0 with an adjusted `this` and `arg=1`), simplified to skip the raw adjustor-thunk arithmetic (same simplification already accepted at 0x0062E010's `Net::close`). 0.496 mnemonic similarity - the dropped SEH prologue alone accounts for most of the gap - not chased further.
// symbol    ??1AlphaNetLedger@@UAE@XZ
// ORIGINAL: 0x004E34B0 sub_4e34b0 0x004E34B0-0x004E34CB FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/recovered/units/004e34b0.cpp: retained original spans and historical seed interpretation", "Pinned original receiver/table/call sequence reviewed in the Alpha ownership binding decision.", "Current C++ class/lifetime source owns the separately observed emitted symbol; this binding is unclaimed and preserves the independent historical exact seed.", "Genuine declared C++ destructor emits the independently named generated COFF body; original receiver/table/lifetime interpretation and current emitted ABI remain separate from the historical byte obligation."], "reason": "Complete receiver+38 calls4E3350 then publishes virtual allocation owner; no delete flag/free. Superseded scaffold retired after observing this independently emitted generated body; historical claims and findings transfer without a new matching or linked-preservation claim."}
// symbol    ??_DAlphaNetLedger@@QAEXXZ
#include "stdafx.h"
#include "original_seam.h"
#include "alphanet.h"
#include "spritebox.h"
#include "net_class.h"

#include "stringstruct.h"  // StringAllocationBase, the ledger's virtual base
#include "heap.h"
#include "general.h"  // mem_get, the fallback when the ledger has no Heap


#include "stdafx.h"
#include "alphanet.h"
#include "netdaemon.h"
#include "netwin.h"
#include "popup.h"
#include "xpops.h"
#include "main.h"
#include "alpha.h"

#include "text.h"
#include "console.h"
#include "game.h"
#include "general.h"
#include <new>
#include "net_class.h"
#include "heap.h"
#include <cstdlib>
#include <cstring>
int __stdcall compare_session_guid(const GUID *, const GUID *);
namespace {
// Source-only typed algorithms shared by genuine list owners. They own no
// executable identity. Each instantiated body/reference requires measurement;
// sharing source does not prove the original compiler folded any particular pair.
template<class List> inline void clear_owned_list(List *list) {
    if (list->head_ != 0) {
        for (int removed = 0; removed < list->entry_count_; ++removed) {
            list->current_ = list->head_->next_;
            typename List::Payload *payload = list->head_->payload_;
            list->on_remove(payload);
            delete payload;
            // These are intentionally RE-READS after callback and destruction.
            list->head_->payload_ = 0;
            delete list->head_;
            list->head_ = list->current_;
        }
        list->head_ = 0;
        list->current_position_ = 0;
        list->entry_count_ = 0;
    }
}

template<class List, class Node> inline Node *allocate_owned_node(List *list, Node *) {
    Heap *const explicit_owner = list->allocator_;
    if (explicit_owner != 0) return new(explicit_owner) Node;
    return new(list) Node;
}

template<class List> inline int seek_owned_list(List *list, int position) {
    AbsFnT abs_fn = &abs;
    if (position > list->entry_count_ - 1) return 0;
    list->current_ = list->head_;
    if (position < 0) {
        if (abs_fn(position) > list->entry_count_) return 0;
        int remaining = abs_fn(position);
        while (remaining > 0) {
            --remaining;
            list->current_ = list->current_->previous_;
        }
        list->current_position_ = list->entry_count_ + position;
    } else {
        int remaining = position;
        while (remaining > 0) {
            --remaining;
            list->current_ = list->current_->next_;
        }
        list->current_position_ = position;
    }
    return 1;
}

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



/*
Purpose: Convert a multiplayer process ID to its one-based player index.
// ORIGINAL: 0x004E25E0 ?pid_2_idx@AlphaNet@@QAEHK@Z 0x004E25E0-0x004E2607 BYTE_EXACT
// symbol    ?pid_2_idx@AlphaNet@@QAEHI@Z
// size      39 bytes
// prototype int (__thiscall ?pid_2_idx@AlphaNet@@QAEHK@Z)(AlphaNet* this, unsigned int)
// callers   20   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: Player index (1-7); process ID not found (0)
Status: Complete
*/
int AlphaNet::pid_2_idx(uint32_t process_id) {
    for (int index = 1; index < 8; index++) {
        if (process_slots_[index].pid_ == process_id) {
            return index;
        }
    }
    return 0;
}

/*
Purpose: Convert a multiplayer process ID to its signed player identity.
// ORIGINAL: 0x004E2610 ?pid_2_who@AlphaNet@@QAEHK@Z 0x004E2610-0x004E2653 BYTE_EXACT
// symbol    ?pid_2_who@AlphaNet@@QAEHI@Z
// size      67 bytes
// prototype int (__thiscall ?pid_2_who@AlphaNet@@QAEHK@Z)(AlphaNet* this, unsigned int)
// callers   2   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: Player identity; process ID not found (0)
Status: Complete
*/
int AlphaNet::pid_2_who(uint32_t process_id) {
    for (int index = 1; index < 8; index++) {
        if (process_slots_[index].pid_ == process_id) {
            return process_slots_[index].identity_;
        }
    }
    return 0;
}

/*
Purpose: Convert a signed multiplayer player identity to its process ID.
// ORIGINAL: 0x004E2660 ?who_2_pid@AlphaNet@@QAEHH@Z 0x004E2660-0x004E26A8 BYTE_EXACT
// size      72 bytes
// prototype int (__thiscall ?who_2_pid@AlphaNet@@QAEHH@Z)(AlphaNet* this, int)
// callers   4   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: Process ID; player identity not found (0)
Status: Complete
*/
int AlphaNet::who_2_pid(int identity) {
    for (int index = 1; index < 8; index++) {
        if (process_slots_[index].identity_ == identity) {
            return process_slots_[index].pid_;
        }
    }
    return 0;
}

/*
Purpose: Convert a signed multiplayer player identity to its one-based index.
// ORIGINAL: 0x004E26B0 ?who_2_idx@AlphaNet@@QAEXH@Z 0x004E26B0-0x004E26DC BYTE_EXACT
// symbol    ?who_2_idx@AlphaNet@@QAEHH@Z
// size      44 bytes
// prototype void (__thiscall ?who_2_idx@AlphaNet@@QAEXH@Z)(AlphaNet* this, int)
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: Player index (1-7); player identity not found (0)
Status: Complete
*/
int AlphaNet::who_2_idx(int identity) {
    for (int index = 1; index < 8; index++) {
        if (process_slots_[index].identity_ == identity) {
            return index;
        }
    }
    return 0;
}






/*
Purpose: Clear every player's process slot and hand off to the network close.
// size      40 bytes
// prototype void (__thiscall ?close@AlphaNet@@QAEXXZ)(AlphaNet* this)
// callers   14   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Status: Complete with a temporary dependency on the network close
*/


/*
Purpose: Build an AlphaNet: the Net base (implicit), the eight process
         slots' {FF,0,0,FF,2} patterns, the ledger, and the trailing Heap -
         all through its typed members.
// ORIGINAL: 0x004E2490 ??0AlphaNet@@QAE@XZ 0x004E2490-0x004E25A5;0x0065C610-0x0065C630
// symbol    ??0AlphaNet@@QAE@XZ
// size      277 bytes
// kind      game
// Player record construction owns the five initial identity/status bytes.
// The ledger and Heap now construct/destruct as ordinary members. The
// image's final [this]=0x66EACC store is now emitted from AlphaNet's real
// overrides. The constructor's remaining source and lifetime remain unfinished.
Return Value: n/a
*/
AlphaNet::AlphaNet() {
    for (int j = 0; j < 8; ++j) {
        process_slots_[j].pid_ = 0;
    }
    player_count_ = 0;
}

// The original closes transport before destroying Heap, ledger, then Net.
// The members/base own those lifetimes; NetDaemon must not repeat them.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00538B60 ??1AlphaNet@@QAE@XZ 0x00538B60-0x00538D0E;0x004E3710-0x004E372B;0x0065DF70-0x0065E005
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00538b60.cpp: existing identity and complete recorded spans", "Original AlphaNet destruction closes transport, then destroys Heap at +0x148C and ledger at +0x144C before Net; normal members now own this chain."], "reason": "Adopt the retained destructor seed identity for the real AlphaNet owner. Existing shared cleanup spans are preserved, not newly inferred or claimed exact; the native one-entry/new-binding route deliberately does not infer overlapping ownership."}
AlphaNet::~AlphaNet() {
    close();
}

// Intentional no-op overrides, independently present in AlphaNet's original
// table. The preserved original identities belong to these methods, not
// free fastcall functions with invented receiver arguments.
// ORIGINAL: 0x004E32F0 nullsub_20 0x004E32F0-0x004E32F3 BYTE_EXACT
// symbol    ?on_net_event_0@AlphaNet@@UAEXHH@Z
void AlphaNet::on_net_event_0(int, int) { }

// ORIGINAL: 0x004E32C0 nullsub_164 0x004E32C0-0x004E32C3 BYTE_EXACT
// symbol    ?on_player_name_changed@AlphaNet@@UAEXPADK@Z
void AlphaNet::on_player_name_changed(char *, unsigned long) { }

// ORIGINAL: 0x004E32D0 nullsub_165 0x004E32D0-0x004E32D1 BYTE_EXACT
// symbol    ?on_session_lost@AlphaNet@@UAEXXZ
void AlphaNet::on_session_lost() { }

// ORIGINAL: 0x004E32E0 nullsub_166 0x004E32E0-0x004E32E1 BYTE_EXACT
// symbol    ?on_host_changed@AlphaNet@@UAEXXZ
void AlphaNet::on_host_changed() { }

// The ledger's five list fields, zeroed; the rest of the construction (the
// vtable staging and the owner capture) belongs to the compiler.
AlphaNetStringStruct::AlphaNetStringStruct() {
    head_ = 0;
    current_ = 0;
    entry_count_ = 0;
    current_position_ = 0;
    allocator_ = 0;
}

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




// ORIGINAL: 0x004E25B0 ?close@AlphaNet@@QAEXXZ 0x004E25B0-0x004E25D8 BYTE_EXACT
// kind      game
void AlphaNet::close() {
    // Eight process-ID slots at 0x78C, one per player, stride 0x19C.
    for (size_t slot = 0; slot < 8; ++slot) {
        process_slots_[slot].pid_ = 0;
    }
    player_count_ = 0;
    // The legacy body tail-jumps here with this unchanged, so the network
    // close runs against the same object.
    Net::close();
}
// ALPHANET_CLOSE_END






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
    if (entry_count_ == 0) {
        head_ = allocate_owned_node(this, static_cast<AlphaNetLedgerEntry *>(0));
        if (head_ == 0) return 4;
        head_->id_ = id;
        head_->next_ = head_;
        head_->previous_ = head_;
        head_->payload_ = allocate_owned_node(this, static_cast<NetAllocationNode *>(0));
        if (head_->payload_ == 0) return 4;
        on_insert(head_->payload_);
        current_ = head_;
    } else {
        head_->previous_->next_ = allocate_owned_node(this, static_cast<AlphaNetLedgerEntry *>(0));
        if (head_->previous_->next_ == 0) return 4;
        head_->previous_->next_->previous_ = head_->previous_;
        head_->previous_->next_->next_ = head_;
        head_->previous_ = head_->previous_->next_;
        current_ = head_->previous_;
        current_->id_ = id;
        current_->payload_ = allocate_owned_node(this, static_cast<NetAllocationNode *>(0));
        if (current_->payload_ == 0) return 4;
        on_insert(current_->payload_);
    }
    current_position_ = entry_count_;
    ++entry_count_;
    return 0;
}


/*
// ORIGINAL: 0x00401CE0 SessionStruct::close 0x00401CE0-0x00401D79
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
AlphaNetStringStruct::~AlphaNetStringStruct() {
    remove_all();
    current_position_ = 0;
}


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
        seek_owned_list(this, walk_index);
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
                if (seek_owned_list(this, best_index) != 0) {
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

void reset_net_session_options() {
    NetWinSessionOptions.status_ = 0xFF;
    NetWinSessionOptions.player_count_ = 4;
    NetWinSessionOptions.difficulty_ = 1;
    NetWinSessionOptions.option_4_ = 0;
    NetWinSessionOptions.option_5_ = 0;
    NetWinSessionOptions.planet_size_ = 1;
    NetWinSessionOptions.ocean_coverage_ = 1;
    NetWinSessionOptions.erosion_ = 1;
    NetWinSessionOptions.native_life_ = 1;
    NetWinSessionOptions.cloud_cover_ = 1;
    NetWinSessionOptions.reserved_B_ = 0xFF;
}

void reset_net_lobby_rows() {
    for (int row = 0; row < 8; ++row) {
        NetWinLobbyRows[row].owner_ = 0;
        NetWinLobbyRows[row].prefix_ = 0xFF;
        NetWinLobbyRows[row].ready_or_occupied_ = 0;
        NetWinLobbyRows[row].field_2_ = 0;
        NetWinLobbyRows[row].faction_or_color_ = -1;
        NetWinLobbyRows[row].field_4_ = 2;
    }
}

void AlphaNetStringStruct::remove_all() { clear_owned_list(this); }

int AlphaNetStringStruct::seek_pos(int position) { return seek_owned_list(this, position); }

NetAllocationNode *AlphaNetStringStruct::next_entry() { return next_owned_list(this); }

AlphaNetLedger::~AlphaNetLedger() {
    remove_all();
    current_position_ = 0;
}

// ORIGINAL: 0x004E26E0 ?pick_service@AlphaNet@@QAEHPAH@Z 0x004E26E0-0x004E29DF;0x004E38E0-0x004E38FB;0x0065C630-0x0065C664
// kind      game
// TRIED: full transcription (guard/save-global, ServiceStruct local, BasePop::start/exec, Dialogs::item list build, join_service) compiles but this address is catalogued SHARED_TAIL - one of its spans is COMDAT-folded with another function, so no body, not even a one-line stub, gets a MISMATCH/BYTE_EXACT verdict
int AlphaNet::pick_service(int *allow_pbem) {
    // The original saves and clears the allocation handoff before creating
    // the local list, then restores it after list destruction (0x004E26E0).
    // An outer pending Heap must not become this list's allocation owner.
    struct AllocationHandoffScope {
        Heap *saved;
        AllocationHandoffScope() : saved(StringAllocationHeap) {
            StringAllocationHeap = 0;
        }
        ~AllocationHandoffScope() { StringAllocationHeap = saved; }
    } allocation_handoff;
    ServiceList providers;
    if (allow_pbem != 0) *allow_pbem = 0;
    if (buffer_6E0_ == 0) return 7;
    BasePop *popup = PopupAllocHook();
    if (popup == 0) return 4;
    int result;
    if (poll_services(&providers) != 0 ||
        popup->start("jackal", "SELECTSERVICE", -1, 0, 0x42, 0) != 0) {
        result = 1;
    } else {
        providers.seek_pos(-1);
        for (int row = 0; row < providers.entry_count_; ++row) {
            providers.next_entry();
            char *name;
            if (providers.current_ == 0) name = 0;
            else {
                ServiceStruct *payload = providers.head_ == 0 ? 0 : providers.current_->payload_;
                // Original inconsistent-list path dereferences null+4 here.
                name = payload->name_;
            }
            popup->dialogs().Dialogs::item(name, row);
        }
        if (allow_pbem != 0) popup->dialogs().Dialogs::item(label_get(0x1128 / 4), 9999);
        const int selected = popup->exec(0, 0);
        if (selected == -1) result = 1;
        else if (selected == 9999) {
            result = 0;
            if (allow_pbem != 0) *allow_pbem = 1;
        } else {
            // Native selection starts at the head even when the popup's row
            // lies outside the circular list's signed indexing domain.
            if (providers.seek_pos(selected) == 0) providers.seek_pos(0);
            ServiceStruct *payload = providers.head_ == 0 ? 0 : providers.current_->payload_;
            result = join_service(payload);
        }
    }
    delete popup;
    return result;
}

int AlphaNet::do_join_or_create() {
    const int selected = x_pop_caption_flags("jackal", "NETCONNECT_JOIN_OR_CREATE", 0x50, 0);
    if (selected < 0) return 1;
    if (selected == 1) {
        if (join() == 0) return 1;
    } else {
        if (create() == 0) return 1;
    }
    return 0;
}

int AlphaNet::create() {
    Popup popup;
    prefs_get("Net Session", "---", 0);
    strcpy(ParseStrBuffer[0].str, TextBufferGetPtr);
    prefs_get("Net Name", "---", 0);
    strcpy(ParseStrBuffer[1].str, TextBufferGetPtr);
    int selected = x_pop_caption_value_flags("jackal", "NETCONNECT_CREATE", 0x1F, 0x44, 0);
    while (selected != -1) {
        if (ParseStrBuffer[0].str[0] == 0) X_pop("jackal", "NETCONNECT_NEED_SESSION", 0);
        else if (ParseStrBuffer[1].str[0] == 0) X_pop("jackal", "NETCONNECT_NEED_PLAYER", 0);
        else break;
        selected = x_pop_caption_value_flags("jackal", "NETCONNECT_CREATE", 0x1F, 0x44, 0);
    }
    if (selected == -1) return 0;
    prefs_put("Net Session", ParseStrBuffer[0].str);
    prefs_put("Net Name", ParseStrBuffer[1].str);
    // Proposed correction of the existing pointer-return declaration preserves the actual native answer,
    // rather than truncating a host pointer through the current catalogue int.
    NetAllocationNode *session = NetDaemonNet.create_session(ParseStrBuffer[0].str, 7, 0);
    if (session == 0) return 0;
    if (NetDaemonNet.join_session(session, ParseStrBuffer[1].str, 0) == 0) return 0;
    return NetDaemonNet.local_player_id();
}

int AlphaNet::join() {
    Popup popup;
    prefs_get("Net Name", "---", 0);
    strcpy(ParseStrBuffer[0].str, TextBufferGetPtr);
    if (x_pop_caption_value_flags("jackal", "NETCONNECT_JOIN", 0x1F, 0x44, 0) == -1) return 0;
    prefs_put("Net Name", ParseStrBuffer[0].str);
    int selected;
    do {
        for (;;) {
            ledger_.remove_all();
            ledger_.current_position_ = 0;
            ledger_.allocator_ = 0;
            NetDaemonNet.enumerate_sessions(&ledger_, 0);
            if (ledger_.entry_count_ != 0) break;
            X_pop("jackal", "NETCONNECT_SESSION_NOT_FOUND", 0);
            if (ConsoleGroupAction != 1) return 0;
        }
        popup.start("jackal", "NETCONNECT_SESSIONS", -1, 0, 0x42);
        ledger_.seek_pos(-1);
        for (int row = 0; row < ledger_.entry_count_; ++row) {
            ledger_.next_entry();
            char *name;
            if (ledger_.current_ == 0) name = 0;
            else {
                NetAllocationNode *payload = ledger_.head_ == 0 ? 0 : ledger_.current_->payload_;
                name = payload->session_name_;
            }
            popup.dialogs().Dialogs::item(name, row);
        }
        selected = popup.exec(0, 0);
        if (selected < 0) break;
    } while (popup.input_poll_state() == 1);
    // Original 4E3006 branches negative cancellation directly to 4E311C.
    // Seek only in the nonnegative join arm; cancellation must not alter the
    // cursor before removal hooks, or evaluate abs(INT_MIN).
    int result;
    if (selected >= 0) {
        ledger_.seek_pos(selected);
        NetAllocationNode *session = ledger_.head_ == 0 ? 0 : ledger_.current_->payload_;
        result = NetDaemonNet.join_session(session, ParseStrBuffer[0].str, 0);
    } else result = 0;
    ledger_.remove_all();
    ledger_.current_position_ = 0;
    return result;
}

int AlphaNet::setup(int option) {
    for (int slot = 0; slot < 8; ++slot) process_slots_[slot].pid_ = 0;
    player_count_ = 0;
    Net::close();
    IsMultiplayerPBEM = 0;
    if (option == 0) {
        // Original 4E3225 passes game-owned storage 689218, not the
        // value-equal but independent NetLobbyAppGuid at 697548.
        Net::init(const_cast<GUID *>(&LobbyAppGuid), 3, 9);
        const int picked = pick_service(&IsMultiplayerPBEM);
        if (picked != 0) return picked;
        if (IsMultiplayerPBEM != 0) {
            Net::close();
            return 0;
        }
        const int selected = x_pop_caption_flags("jackal", "NETCONNECT_JOIN_OR_CREATE", 0x50, 0);
        if (selected < 0) return 1;
        if (selected == 1) {
            if (join() == 0) return 1;
        } else {
            if (create() == 0) return 1;
        }
    } else voice_flags_D8_ |= 1;
    voice_hotkey_48_ = 0xDC;
    voice_hotkey_4c_ = 0x100DC;
    return 0;
}
