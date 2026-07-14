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

int __fastcall string_struct_current_id_redirect(StringStruct *self, void *) {
    return self->current_id();
}

int __fastcall string_struct_current_entry_redirect(StringStruct *self, void *) {
    return self->current_entry();
}
