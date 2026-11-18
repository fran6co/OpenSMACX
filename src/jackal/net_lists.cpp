/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenSMACX is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenSMACX. If not, see <http://www.gnu.org/licenses/>.
 */
// WORKAROUND(decl): same declaration context as src/jackal/net_class.cpp.
#define NET_CLASS_VOICE_NODE_NEW
#define ALPHANET_CURRENT_ENTRY_DECL  // see alphanet.h
#include "stdafx.h"
#include "alphanet.h"
#include "stringstruct.h"
#include "heap.h"
#include "general.h"
#include "net_class.h"
#include <cstdlib>
#include <cstring>
#include "alphanet_inline.h"

// Inferred original member: the allocation-list virtuals at
// 0x005D47D0..0x005D4CE7, directly after Heap's member and before
// GraphicWin's. Nothing calls them directly; their classes' tables (built by
// Net and the AlphaNet lists) reach them. In address order:
// ServiceList::on_insert/on_remove, AlphaNetLedger::on_insert/on_remove,
// AlphaNetStringStruct::sort_sessions and compare_session_guid, and
// NetUnk72C::on_insert/on_remove. The original filename is unknown. The two
// on_remove bodies are inline in net_class.h and are measured where their
// tables are emitted.

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


/*
Purpose: Copy one text record's string into fresh storage chosen by the
         record's own allocator (the CRT heap when none is installed) and
         hand the copy plus the four metadata words to the caller's array.
// ORIGINAL: 0x005D47D0 sub_5d47d0 0x005D47D0-0x005D4851 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original 5D47D0-5D4851 and its retained seed/history preserve identity and extent.", "Provider callback 630BF0 borrows name and provider GUID through ServiceList +1C/+20 before add; receiver +18 selects Heap versus mem_get. Payload owns copied name +4 and GUID +8. These are ServiceList/ServiceStruct roles, not the sibling nested-text owner.", "Native v3 emits the genuine virtual ServiceList insertion symbol; source nominal names remain evidence-backed interpretations, not new byte claims."], "reason": "Recover provider-list insertion as ServiceList::on_insert on the real enumerated provider payload."}
// Historical superseded binding interpretation: {"evidence": ["Original5D47D0..5D4851; retired seed git:da8a4b7f:src/unrecovered/005d47d0.cpp records the same identity and spans.", "The receiver is the +0x18 Heap pointer, +0x1C string and +0x20 metadata record the sibling 0x005D4C50 installs into; mem_get is general.cpp's and Heap::get is heap.h's own, both already linked by this TU's list machinery."], "reason": "Coverage transcription of the record copy-out with the thiscall receiver spelled as an explicit first argument; no in-tree class owns the record yet, so its slots stay documented offsets."}
// symbol    ?on_insert@ServiceList@@UAEHPAUServiceStruct@@@Z
Status: Coverage transcription - unclaimed body
*/
int ServiceList::on_insert(ServiceStruct *payload) {
    const char *text = name_;
    if (text != 0) {
        char *copy;
        if (allocator_ != 0) {
            const size_t length = strlen(text) + 1;
            copy = static_cast<char *>(allocator_->get(length));
        } else {
            copy = static_cast<char *>(mem_get(strlen(text) + 1));
        }
        payload->name_ = copy;
        if (copy == 0) return 4;
        *copy = 0;
        // Native re-reads the borrowed input after allocation and the output
        // pointer after publication; neither can be cached across the helper.
        strcat(payload->name_, name_);
    }
    // Four scalar words copied in the native hook; GUID value assignment may
    // lower as a block copy and therefore needs native reference measurement.
    payload->provider_guid_ = *provider_guid_;
    return 0;
}


// ORIGINAL: 0x005D4860 sub_5d4860 0x005D4860-0x005D4887 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:b2de7f29042d247ee9b448c814029bbbbc35f230:src/recovered/005d4860.cpp: retained original spans and historical seed interpretation", "Pinned original receiver/table/call sequence reviewed in the Alpha ownership binding decision.", "Current C++ class/lifetime source owns the separately observed emitted symbol; this binding is unclaimed and preserves the independent historical exact seed.", "git:b2de7f29042d247ee9b448c814029bbbbc35f230:src/recovered/005d4860.cpp"], "reason": "Provider table slot1 frees/nulls name +4 only when allocator+18 null; retained exact seed. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
// compile_context src/jackal/net_class.cpp
// kind      game
// symbol    ?on_remove@ServiceList@@UAEXPAUServiceStruct@@@Z

// ORIGINAL: 0x005D4890 ?on_insert@AlphaNetLedger@@UAEHPAVNetAllocationNode@@@Z 0x005D4890-0x005D498D BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original table 0x66EAE8 slot 0 points here. Insertion 0x401D80 allocates a 0x60 NetAllocationNode, constructs its nested list and invokes this hook; enumeration 0x630C30 supplies DPSESSIONDESC2 fields. Seed extent is retained; this binding makes no matching claim."], "reason": "Recover the session-ledger insertion virtual from its retained seed using the existing allocation-owning payload class."}
// symbol    ?on_insert@AlphaNetLedger@@UAEHPAVNetAllocationNode@@@Z
int AlphaNetLedger::on_insert(NetAllocationNode *payload) {
    if (session_name_ == 0) {
        payload->session_name_ = 0;
    } else {
        if (allocator_ != 0) {
            payload->session_name_ = static_cast<char *>(allocator_->get(strlen(session_name_) + 1));
        } else {
            payload->session_name_ = static_cast<char *>(mem_get(strlen(session_name_) + 1));
        }
        if (payload->session_name_ == 0) {
            return 4;
        }
        *payload->session_name_ = 0;
        strcat(payload->session_name_, session_name_);
    }
    if (password_ == 0) {
        payload->password_ = 0;
    } else {
        if (allocator_ != 0) {
            payload->password_ = static_cast<char *>(allocator_->get(strlen(password_) + 1));
        } else {
            payload->password_ = static_cast<char *>(mem_get(strlen(password_) + 1));
        }
        if (payload->password_ == 0) {
            return 4;
        }
        *payload->password_ = 0;
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
// RECOVERY_BINDING: v1 {"evidence": ["Derived table 0x66EAE8 slot 1 points here. Original checks receiver allocator at +0x18 before freeing and nulling payload strings +4 and +8. The exact seed git:0e184947304cbb843b26264d5145e9b9b452cf16:src/recovered/005d4990.cpp is retired; its existing restoration claim remains on this compiled owner.", "git:0e184947304cbb843b26264d5145e9b9b452cf16:src/recovered/005d4990.cpp", "Historical seed spelling sub_5d4990; current source mapping ?on_remove@AlphaNetLedger@@UAEXPAVNetAllocationNode@@@Z; emitted symbol ?on_remove@AlphaNetLedger@@UAEXPAVNetAllocationNode@@@Z."], "reason": "Recover allocator-aware session payload cleanup as the real ledger virtual, retaining the historical exact restoration obligation in source. Superseded seed retired after owner review; original spans, existing claims and useful seed findings remain in source. This cleanup grants no new match or acceptance claim."}
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

/*
Purpose: Sort one pass of the string list: walk the ring from the current
         position, select the smallest record under the three-way record
         comparator, and swap the winning entry's payload text and metadata
         into the walk position.
// ORIGINAL: 0x005D49D0 sub_5d49d0 0x005D49D0-0x005D4BA7 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original 5D49D0-5D4BA7 and retained seed/history preserve the same identity and spans.", "The Alpha join call chain 4E2E50 uses session ledger enumeration 62E420 and comparator 5D4BB0 on payload GUID at +0xC. Circular receiver fields +8/+C/+10/+14 and entry next/previous +0xC/+0x10 establish a list protocol, not StringStruct nominal identity.", "Original winning-entry exchange swaps entry id +4 and payload pointer +8, not text/metadata inside a payload. Reviewed source uses AlphaNetLedgerEntry and NetAllocationNode::instance_guid_; count is reread each scan iteration."], "reason": "Recover the session-ledger GUID selection pass on AlphaNetStringStruct, keeping entry ID and payload ownership paired."}
// Historical superseded binding interpretation: {"evidence": ["Original5D49D0..5D4BA7; retired seed git:da8a4b7f:src/unrecovered/005d49d0.cpp records the same identity and spans.", "The receiver fields at +8/+C/+10/+14 are StringStruct's own head_/current_/entry_count_/current_position_ and the body calls StringStruct::seek_pos, so the receiver is typed as the list itself; the comparator is sub_5d4bb0 above, and the node walks use StringStructEntry's next(+0xC)/previous(+0x10) exactly as StringStruct::add installs them.", "The swapped payload words at +4/+8 are the payload's text pointer and first metadata word; the walk re-reads entry_count_ every iteration exactly as the seed measured."], "reason": "Coverage transcription of the list's selection pass on the typed StringStruct receiver; the payload internals stay documented offsets."}
// symbol    ?sort_sessions@AlphaNetStringStruct@@QAEXXZ
Status: Coverage transcription - unclaimed body
*/
// WORKAROUND(decl): sort_sessions expands the list's current_entry() under
// each head test: the image reuses EAX for every link and forms the GUID
// address with lea (0x005D4A4E..0x005D4A54, 0x005D4AC8..0x005D4ACE), which
// only the if/return inline body gives (a ?: helper and the written-out
// chain compile to mov/add in the result register and swap EBX/EBP through
// the whole function). AlphaNetStringStruct::current_entry is declared here
// but defined out of line in net_class.cpp, so this twin carries its body.
static inline NetAllocationNode *sort_sessions_current(AlphaNetStringStruct *list)
{
    if (list->head_) {
        return list->current_->payload_;
    }
    return 0;
}
// WORKAROUND(inline): dead /Ob2 spend; with it sort_sessions keeps the image's
// out-of-line seek_pos(walk_index) call (0x005D4B50) and its block layout.
// The original spelling that spent this budget is unknown.
static int sort_sessions_budget_probe(int x)
{
    if (x == 0) x = 1;
    if (x == 1) x = 4;
    if (x == 2) x = 7;
    if (x == 3) x = 10;
    if (x == 4) x = 13;
    if (x == 5) x = 16;
    if (x == 6) x = 19;
    if (x == 7) x = 22;
    if (x == 8) x = 25;
    if (x == 9) x = 28;
    if (x == 10) x = 31;
    if (x == 11) x = 34;
    if (x == 12) x = 37;
    if (x == 13) x = 40;
    if (x == 14) x = 43;
    if (x == 15) x = 46;
    if (x == 16) x = 49;
    if (x == 17) x = 52;
    if (x == 18) x = 55;
    if (x == 19) x = 58;
    if (x == 20) x = 61;
    if (x == 21) x = 64;
    if (x == 22) x = 67;
    if (x == 23) x = 70;
    return x;
}


void AlphaNetStringStruct::sort_sessions() {
    for (int walk_index = 0; walk_index < entry_count_ - 1; walk_index++) {
        // Native inlines this same signed circular seek, including its abs
        // behavior; the callback-free scan starts only from valid list state.
        sort_sessions_budget_probe(2); // WORKAROUND(inline): see above
        seek_pos(walk_index);
        const GUID *best = head_ ? &sort_sessions_current(this)->instance_guid_ : 0;
        int best_index = walk_index;
        for (int scan_index = walk_index + 1; scan_index < entry_count_; scan_index++) {
            next_owned_list(this);
            // If count/head/current disagree, native may pass null+C to the
            // comparator. This named-member expression retains that
            // invalid-domain UB; it does not clamp or repair it.
            const GUID *candidate = current_
                ? &sort_sessions_current(this)->instance_guid_ : 0;
            if (compare_session_guid(candidate, best) < 0) {
                best = head_ ? &sort_sessions_current(this)->instance_guid_ : 0;
                best_index = current_position_;
            }
        }
        if (best_index != walk_index) {
            if (seek_pos(best_index)) {
                AlphaNetLedgerEntry *winner = current_;
                if (seek_pos(walk_index)) {
                    AlphaNetLedgerEntry *walked = current_;
                    // The image exchanges the ids with three XORs.
                    winner->id_ ^= walked->id_;
                    walked->id_ ^= winner->id_;
                    winner->id_ ^= walked->id_;
                    NetAllocationNode *saved_payload = winner->payload_;
                    winner->payload_ = walked->payload_;
                    walked->payload_ = saved_payload;
                }
            }
        }
    }
}


/*
Purpose: Order three-way comparison of two fixed-shape name records: an
         unsigned word, two unsigned shorts and eight bytes, each field
         compared lexicographically.
// ORIGINAL: 0x005D4BB0 sub_5d4bb0 0x005D4BB0-0x005D4C41 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original 5D4BB0-5D4C41 retains its two stack arguments, no calls, and unsigned comparisons: Data1 +0, Data2 +4, Data3 +6, then Data4 eight bytes at +8.", "Call chain 5D49D0 compares pointers to enumerated session instance GUID at payload +0xC; SDK session callback 630C30 and ledger insertion 5D4890 establish the GUID role. Field shape alone was insufficient to establish nominal identity.", "Native v3 objects independently emit compare_session_guid with stdcall GUID pointer arguments; this observes source emission, not original matching or complete owner acceptance."], "reason": "Recover the unsigned lexicographic GUID comparator used by the Alpha session-ledger selection pass."}
// Historical superseded binding interpretation: {"evidence": ["Original5D4BB0..5D4C41; retired seed git:da8a4b7f:src/unrecovered/005d4bb0.cpp records the same identity and spans.", "The body reads only its two stack arguments and makes no calls; the field walks (word at +0, shorts at +4/+6, eight bytes at +8) are transcribed exactly as the seed measured them, with no record type invented for them."], "reason": "Coverage transcription of the unnamed record comparator as a stdcall free function; no names exist for the record fields, so the reads stay spelled by offset."}
// symbol    ?compare_session_guid@AlphaNetStringStruct@@AAEHPBU_GUID@@0@Z
Return Value: -1, 0 or 1 as the first record orders before, equal to or after the second
Status: Coverage transcription - unclaimed body
*/
int AlphaNetStringStruct::compare_session_guid(const GUID *left,
                                               const GUID *right) {
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

/*
Purpose: Copy one text record's string into fresh storage chosen by the
         record's own allocator and park the copy in the caller's output
         block; the metadata half is left to the caller.
// ORIGINAL: 0x005D4C50 sub_5d4c50 0x005D4C50-0x005D4CBC FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original 5D4C50-5D4CBC and its retained seed/history preserve identity and extent.", "The nested list receiver uses Heap +18 and borrowed text +1C to copy text into payload +4 through mem_get or Heap::get. Genuine NetUnk72C and NetTextPayload declarations replace the old raw receiver facade.", "The unseen factory, original nominal payload identity and complete allocation extent remain unresolved; observed prefix and delete protocol do not identify them."], "reason": "Recover NetUnk72C::on_insert for its dedicated observed-prefix NetTextPayload role."}
// Historical superseded binding interpretation: {"evidence": ["Original5D4C50..5D4CBC; retired seed git:da8a4b7f:src/unrecovered/005d4c50.cpp records the same identity and spans.", "Same +0x18/+0x1C receiver record as sub_5d47d0 above, with mem_get and Heap::get resolving exactly as there; the seed's function-pointer CRT spellings are ordinary strlen/strcat calls here."], "reason": "Coverage transcription of the record copy-out's string half with the receiver as an explicit first argument."}
// symbol    ?on_insert@NetUnk72C@@UAEHPAVNetTextPayload@@@Z
Status: Coverage transcription - unclaimed body
*/
int NetUnk72C::on_insert(Payload *payload) {
    payload->field_8_ = 0;
    if (source_text_ != 0) {
        if (allocator_ != 0) {
            size_t const length = strlen(source_text_) + 1;
            // Native 5D4C73 reloads allocator_ AFTER the strlen call.
            payload->text = static_cast<char *>(allocator_->get(length));
        } else {
            payload->text = static_cast<char *>(mem_get(strlen(source_text_) + 1));
        }
        if (payload->text == 0) {
            return 4;
        }
        *payload->text = 0;
        strcat(payload->text, source_text_);
    }
    // Null source does NOT clear payload->text; no unseen payload tail is named.
    return 0;
}

// ORIGINAL: 0x005D4CC0 sub_5d4cc0 0x005D4CC0-0x005D4CE7 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:b2de7f29042d247ee9b448c814029bbbbc35f230:src/recovered/005d4cc0.cpp: retained original spans and historical seed interpretation", "Pinned original receiver/table/call sequence reviewed in the Alpha ownership binding decision.", "Current C++ class/lifetime source owns the separately observed emitted symbol; this binding is unclaimed and preserves the independent historical exact seed.", "git:b2de7f29042d247ee9b448c814029bbbbc35f230:src/recovered/005d4cc0.cpp"], "reason": "Nested list slot1 cleanup at observed payload+4; original factory/complete extent unknown, retain exact seed. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
// compile_context src/jackal/net_class.cpp
// kind      game
// symbol    ?on_remove@NetUnk72C@@UAEXPAVNetTextPayload@@@Z
