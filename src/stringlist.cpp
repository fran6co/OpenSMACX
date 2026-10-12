/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */
#include "stdafx.h"
#include "stringstruct.h"
#include "heap.h"
#include "general.h"
#include "text.h"

#include <cstring>

namespace {
inline void release_text(char *text) {
    if (text != 0) {
        free(text);
    }
}
}

// ORIGINAL: 0x006086F0 ?on_insert@StringList@@UAEHPAVStringStructPayload@@@Z 0x006086F0-0x00608761 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Primary table 0x6698C4 slot0 targets this body. Original StringStruct::add at 0x4012ED/0x4014F7 passes its 0x18 allocation as the one hook argument; this body writes text at +4 and metadata at +8/+C. The retired seed identity and extent are retained by this compiled owner.", "Original 0x6086FD bypasses the text store when the source is null; preserve that behavior without assuming zeroed allocation. Receiver +0x18 is passed to Heap::get. No byte claim is banked by this binding.", "git:293937ab6f3be75999e0c0a4c9c110c02930624c:src/unrecovered/006086f0.cpp", "Historical seed spelling ?fill_entry@StringList@@QAEHPAUStringStruct@@@Z; prior typed-payload source mapping ?on_insert@StringList@@UAEHPAVStringStructPayload@@@Z; prior emitted symbol ?on_insert@StringList@@UAEHPAVStringStructPayload@@@Z.", "Current erased hook is ?on_insert@StringList@@UAEHPAX@Z; real StringStructPayload remains the concrete typed sibling behind ListPayloadBase. Native COFF confirms this current symbol, not a new byte grant."], "reason": "Recover the StringList insertion virtual using its allocated payload type; on_insert is a descriptive name, not a recovered original spelling. Superseded seed retired after owner review; original spans, existing claims and useful seed findings remain in source. This cleanup grants no new match or acceptance claim. The shared erased hook accepts either concrete payload sibling; this method restores its genuine string payload view."}
// symbol    ?on_insert@StringList@@UAEHPAX@Z
int StringList::on_insert(void *raw_payload) {
    StringStructPayload *const payload =
        static_cast<StringStructPayload *>(raw_payload);
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
// RECOVERY_BINDING: v1 {"evidence": ["Primary table 0x6698C4 slot1 targets this body; original 0x608775 loads payload text at +4, then 0x60877C checks the list allocator at +0x18. Only a null allocator permits free and clearing text. on_remove is descriptive; the historical kill_entry claim is retained by this compiled owner.", "git:293937ab6f3be75999e0c0a4c9c110c02930624c:src/recovered/00608770.cpp", "Historical seed spelling ?kill_entry@StringList@@QAEXPAUStringStruct@@@Z; prior typed-payload source mapping ?on_remove@StringList@@UAEXPAVStringStructPayload@@@Z; prior emitted symbol ?on_remove@StringList@@UAEXPAVStringStructPayload@@@Z.", "Current erased hook is ?on_remove@StringList@@UAEXPAX@Z; real StringStructPayload remains the concrete typed sibling behind ListPayloadBase. Native COFF confirms this current symbol, not a new byte grant."], "reason": "Recover allocator-aware StringList text cleanup as a real virtual method; the payload node lifetime remains owned by enclosing list teardown. Superseded seed retired after owner review; original spans, existing claims and useful seed findings remain in source. This cleanup grants no new match or acceptance claim. The shared erased hook accepts either concrete payload sibling; this method restores its genuine string payload view."}
// symbol    ?on_remove@StringList@@UAEXPAX@Z
void StringList::on_remove(void *raw_payload) {
    StringStructPayload *const payload =
        static_cast<StringStructPayload *>(raw_payload);
    if (payload->text != 0 && allocator_ == 0) {
        release_text(payload->text);
        payload->text = 0;
    }
}

// Parse the caret-prefix convention into pending insertion fields, then
// return the original normalized StringStruct::add result.
// ORIGINAL: 0x00608BA0 ?add_special@StringList@@QAEHPAD@Z 0x00608BA0-0x00608BFC BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:93d09e9c95484465004f450350b6cffe1e09f40e:src/recovered/units/00608ba0.cpp; src/stringstruct.h StringList pending fields and inherited StringStruct::add"], "reason": "Recover caret-prefix parsing and normalized ID-zero insertion on the real StringList owner."}
int StringList::add_special(char *text) {
    int kind = 0;
    if (*text == '^') {
        if (text[1] != '\0') {
            if (text[1] != '^') {
                kind = 1;
                ++text;
            } else if (text[2] == '^') {
                kind = 2;
                text += 3;
            } else {
                kind = 2;
                text += 2;
            }
        } else {
            kind = 3;
            ++text;
        }
    }
    pending_text_ = text;
    field_20_ = static_cast<uint32_t>(kind);
    field_24_ = 0;
    return add(0) != 0;
}

/*
Purpose: Sort the circular list by payload text, keeping each ID with its
         payload and retaining the original cursor side effects.
// ORIGINAL: 0x006087A0 ?sort@StringList@@QAEXH@Z 0x006087A0-0x00608980
// RECOVERY_BINDING: v1 {"evidence": ["git:caf4963a retained the historical selection-pass seed at 0x006087A0-0x00608980 before its product migration", "image:0x006087A0 scans StringStructEntry next links, compares payload text with strcmp, then swaps the winning entry's id and payload", "source:src/stringstruct.h declares the real circular StringStructEntry and StringStructPayload members and the inherited seek_pos cursor operation"], "reason": "Replace Datalink's fixed-image sort forwarder with a typed StringList selection sort in the list's owning translation unit."}
// size      480 bytes
// callers   3   call targets   3
// calls     0x004015B0 0x00644F3A 0x00645660
Return Value: n/a
Status: Product behavior; historical seed remains an unclaimed measurement.
*/
void StringList::sort(int) {
    for (int position = 0; position < entry_count_ - 1; ++position) {
        seek_pos(position);
        StringStructEntry *const first = current_;
        StringStructEntry *smallest = first;

        for (int candidate = position + 1; candidate < entry_count_;
             ++candidate) {
            next_entry();
            if (strcmp(current_->string_payload()->text,
                       smallest->string_payload()->text) < 0) {
                smallest = current_;
            }
        }

        if (smallest != first) {
            seek_pos(position);
            const int id = first->id;
            StringStructPayload *const payload = first->string_payload();
            first->id = smallest->id;
            first->payload = smallest->string_payload();
            smallest->id = id;
            smallest->payload = payload;
        }
    }
}

// Read a text section into the list, preserving the caret-prefixed value
// conventions used by the original string tables. The callback parameter is
// part of the original interface but is not used by this body.
// ORIGINAL: 0x00608980 ?load@StringList@@QAEHPADPADHP6AXPAD@Z@Z 0x00608980-0x00608B9B;0x00662EA0-0x00662EB5
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/00608980.cpp", "source:src/stringstruct.h:StringList"], "reason": "Home StringList section loading with typed Text lifecycle and inherited StringStruct entry/payload ownership"}
// symbol    ?load@StringList@@QAEHPAD0HP6AX0@Z@Z
int StringList::load(LPSTR file_name, LPSTR section_id, int clear_existing,
                     void (__cdecl *)(LPSTR)) {
    Text text(0x200);
    bool saw_non_hash = false;

    if (clear_existing != 0 && head_ != nullptr) {
        remove_all();
    }

    if (section_id == nullptr) {
        return 3;
    }
    if (text.open(file_name, section_id) != 0) {
        return 1;
    }

    bool has_more = true;
    // The native parser output has a 0x100-byte window before its Text
    // local. Historical array spelling/padding and overflow layout are unknown.
    char parse_buffer[256];
    for (;;) {
        char *line;
        for (;;) {
            line = text.get();
            if (line == nullptr) {
                text.close();
                return 1;
            }
            if (*line == '#') {
                if (saw_non_hash) {
                    text.close();
                    return 0;
                }
            } else {
                saw_non_hash = true;
            }
            if (*line != '\0') {
                break;
            }
            has_more = false;
        }

        parse_buffer[0] = '\0';
        parse_string(line, parse_buffer);
        if (parse_buffer[0] == '#' || !has_more) {
            continue;
        }

        int kind = 0;
        LPSTR value = parse_buffer;
        if (parse_buffer[0] == '^') {
            if (parse_buffer[1] == '\0') {
                kind = 3;
            } else if (parse_buffer[1] == '^') {
                kind = 2;
                value = parse_buffer[2] == '^' ? &parse_buffer[3]
                                                : &parse_buffer[2];
            } else {
                kind = 1;
            }
            if (kind != 2) {
                value = &parse_buffer[1];
            }
        }

        pending_text_ = value;
        field_20_ = static_cast<uint32_t>(kind);
        field_24_ = 0;
        const int add_result = add(0);
        // on_insert copies the text into its payload, but the original keeps
        // this borrowed stack slice, including its expired value after return.
        if (add_result != 0) {
            text.close();
            return 1;
        }
    }
}

// Derived cleanup uses virtual remove_all() before the base destructor changes
// dispatch, then restores the cursor position even when the list was empty.
// ORIGINAL: 0x004066C0 ??1StringList@@UAE@XZ 0x004066C0-0x00406818;0x00650980-0x00650995
// RECOVERY_BINDING: v1 {"evidence": ["Original 4066C0 installs derived tables6698C4/6698C0 and clears entries before switching to base tables6693A4/6693A0; primary removal slot selects the text-cleanup hook during the derived stage. Implicit base-only cleanup cannot preserve that lifecycle.", "Original bounds4066C0-406818 and650980-650995 retained in 7eb1b2b0 parent src/stringstruct.cpp. Support code650980 adjusts the saved virtual-base receiver by -0xC before jumping to StringStruct base teardown401060. These are original identity/ownership interpretations, not a matching claim."], "reason": "Restore the original two-stage StringList destructor identity to its real C++ destructor, including its historical EH support span."}
StringList::~StringList() {
    remove_all();
    current_position_ = 0;
}
