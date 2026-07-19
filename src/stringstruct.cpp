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

namespace {

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
#endif
typedef void(__thiscall func_entry_visitor)(void *, void *);
typedef void(__thiscall func_scalar_deleting_destructor)(void *, int);
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

// MSVC reaches these destructors through the virtual-base displacement stored
// in the object's second vtable slot: the displacement selects a subobject
// whose own first vtable slot is the scalar deleting destructor, invoked with
// the deleting flag set.
void destroy_virtual_base(void *object) {
    uint32_t *const vtable = *reinterpret_cast<uint32_t **>(object);
    uint8_t *const subobject =
        static_cast<uint8_t *>(object) + vtable[1];
    uint32_t *const subobject_vtable = *reinterpret_cast<uint32_t **>(subobject);
    reinterpret_cast<func_scalar_deleting_destructor *>(
        subobject_vtable[0])(subobject, 1);
}

void *payload_pointer(int payload) {
    return reinterpret_cast<void *>(
        static_cast<uintptr_t>(static_cast<uint32_t>(payload)));
}

}  // namespace

/*
Purpose: Release every entry in the list, notifying the owner about each
         payload before destroying the payload and its entry.
Original Offset: 00402970
Status: Complete
*/
void StringStruct::remove_all() {
    if (!head_) {
        return;
    }
    if (entry_count_ > 0) {
        int index = 0;
        do {
            StringStructEntry *const entry = head_;
            current_ = entry->next;
            void *const payload = payload_pointer(entry->payload);
            uint32_t *const vtable = *reinterpret_cast<uint32_t **>(this);
            reinterpret_cast<func_entry_visitor *>(vtable[1])(this, payload);
            if (payload) {
                destroy_virtual_base(payload);
            }
            head_->payload = 0;
            if (head_) {
                destroy_virtual_base(head_);
            }
            ++index;
            head_ = current_;
            // The loop bound is re-read every iteration, so a visitor that
            // changes the count changes how far the walk goes.
        } while (index < entry_count_);
    }
    head_ = nullptr;
    current_position_ = 0;
    entry_count_ = 0;
}

void __fastcall string_struct_remove_all_redirect(StringStruct *self, void *) {
    self->remove_all();
}
