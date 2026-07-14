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

#include "stringstruct.h"

#include <cstring>

/*
Purpose: Return the current string-list item ID.
Original Offset: 00401640
Return Value: Current ID, or zero when the list is empty
Status: Complete
*/
int StringStruct::current_id() {
    return head_ ? current_->id : 0;
}

/*
Purpose: Return the current string-list payload.
Original Offset: 00402530
Return Value: Current payload, or zero when the list is empty
Status: Complete
*/
int StringStruct::current_entry() {
    return head_ ? current_->payload : 0;
}

/*
Purpose: Advance the current string-list entry and return its payload.
Original Offset: 00402500
Return Value: New current payload, or zero when the list is empty
Status: Complete
*/
int StringStruct::next_entry() {
    if (!head_) {
        return 0;
    }

    StringStructEntry *next = current_->next;
    uint32_t position = static_cast<uint32_t>(current_position_) + 1U;
    current_ = next;
    memcpy(&current_position_, &position, sizeof(current_position_));
    if (position == static_cast<uint32_t>(entry_count_)) {
        current_position_ = 0;
    }
    return current_->payload;
}

/*
Purpose: Position the string-list cursor at the first matching item ID.
Original Offset: 00401560
Return Value: One when found, otherwise zero
Status: Complete
*/
int StringStruct::seek_id(int id) {
    int result = 0;
    if (head_) {
        current_position_ = 0;
        current_ = head_;
        if (entry_count_ > 0) {
            int traversed = 0;
            do {
                StringStructEntry *entry = current_;
                if (entry->id == id) {
                    result = 1;
                    break;
                }
                current_position_++;
                traversed++;
                current_ = entry->next;
            } while (traversed < entry_count_);
        }
    }
    return result;
}

int __fastcall string_struct_current_id_redirect(StringStruct *self, void *) {
    return self->current_id();
}

int __fastcall string_struct_current_entry_redirect(StringStruct *self, void *) {
    return self->current_entry();
}

int __fastcall string_struct_next_entry_redirect(StringStruct *self, void *) {
    return self->next_entry();
}

extern "C" int __cdecl string_struct_seek_id_source(StringStruct *self, int id) {
    return self->seek_id(id);
}

#ifdef __GNUC__
int __fastcall string_struct_seek_id_redirect(StringStruct *, void *, int) {
    __asm__(
        "pushl 4(%esp)\n\t"
        "pushl %ecx\n\t"
        "call _string_struct_seek_id_source\n\t"
        "addl $8, %esp\n\t"
        "cmpl %eax, %eax\n\t"
        "ret $4\n\t");
}
#else
__declspec(naked) int __fastcall string_struct_seek_id_redirect(
        StringStruct *, void *, int) {
    __asm {
        push dword ptr [esp + 4]
        push ecx
        call string_struct_seek_id_source
        add esp, 8
        cmp eax, eax
        ret 4
    }
}
#endif
