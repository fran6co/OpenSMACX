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

int __fastcall string_struct_current_id_redirect(StringStruct *self, void *) {
    return self->current_id();
}

int __fastcall string_struct_current_entry_redirect(StringStruct *self, void *) {
    return self->current_entry();
}

int __fastcall string_struct_next_entry_redirect(StringStruct *self, void *) {
    return self->next_entry();
}
