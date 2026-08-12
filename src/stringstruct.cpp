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
#include "original_seam.h"

#include "stringstruct.h"

#include <cstring>

/*
Purpose: Return the current string-list item ID.
ORIGINAL: 0x00401640 BYTE_EXACT
// name      ?current_id@StringStruct@@QAEHXZ
// size      17 bytes
// spans     0x00401640-0x00401651
// prototype int (__thiscall ?current_id@StringStruct@@QAEHXZ)(StringStruct* this)
// callers   1   call targets   0
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
ORIGINAL: 0x00402530 BYTE_EXACT
// name      ?current_entry@StringStruct@@QAEHXZ
// size      17 bytes
// spans     0x00402530-0x00402541
// prototype int (__thiscall ?current_entry@StringStruct@@QAEHXZ)(StringStruct* this)
// callers   9   call targets   0
Return Value: Current payload, or zero when the list is empty
Status: Complete
*/
int StringStruct::current_entry() {
    if (head_) {
        return current_->payload;
    }
    return 0;
}

/*
Purpose: Advance the current string-list entry and return its payload.
ORIGINAL: 0x00402500 BYTE_EXACT
// name      ?next_entry@StringStruct@@QAEHXZ
// size      47 bytes
// spans     0x00402500-0x0040252F
// prototype int (__thiscall ?next_entry@StringStruct@@QAEHXZ)(StringStruct* this)
// callers   9   call targets   0
Return Value: New current payload, or zero when the list is empty
Status: Complete
*/
int StringStruct::next_entry() {
    if (!head_) {
        return 0;
    }

    StringStructEntry *next = current_->next;
    int position = current_position_;
    current_ = next;
    int count = entry_count_;
    ++position;
    current_position_ = position;
    if (position == count) {
        current_position_ = 0;
    }
    return current_->payload;
}

/*
Purpose: Position the string-list cursor at the first matching item ID.
ORIGINAL: 0x00401560 BYTE_EXACT
// name      ?seek_id@StringStruct@@QAEHH@Z
// size      78 bytes
// spans     0x00401560-0x004015AE
// prototype int (__thiscall ?seek_id@StringStruct@@QAEHH@Z)(StringStruct* this, int)
// callers   1   call targets   0
Return Value: One when found, otherwise zero
Status: Complete
*/
int StringStruct::seek_id(int a1) {
    if (head_) {
        current_position_ = 0;
        current_ = head_;
        for (int traversed = 0; traversed < entry_count_; traversed++) {
            int *entry = reinterpret_cast<int*>(current_);
            if (entry[1] == a1) {
                return 1;
            }
            current_position_++;
            current_ = reinterpret_cast<StringStructEntry*>(entry[3]);
        }
    }
    return 0;
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

namespace {

typedef void (OriginalObject::*func_entry_visitor)(void *);
typedef void (OriginalObject::*func_scalar_deleting_destructor)(int);

// MSVC reaches these destructors through the virtual-base displacement stored
// in the object's second vtable slot: the displacement selects a subobject
// whose own first vtable slot is the scalar deleting destructor, invoked with
// the deleting flag set.
void destroy_virtual_base(void *object) {
    uint32_t *const vtable = *reinterpret_cast<uint32_t **>(object);
    uint8_t *const subobject =
        static_cast<uint8_t *>(object) + vtable[1];
    uint32_t *const subobject_vtable = *reinterpret_cast<uint32_t **>(subobject);
    (ORIGINAL(subobject)->*original_method<func_scalar_deleting_destructor>(static_cast<unsigned long>(subobject_vtable[0])))(1);
}

void *payload_pointer(int payload) {
    return reinterpret_cast<void *>(
        static_cast<uintptr_t>(static_cast<uint32_t>(payload)));
}

}  // namespace

/*
Purpose: Release every entry in the list, notifying the owner about each
         payload before destroying the payload and its entry.
ORIGINAL: 0x00402970
// name      StringStruct::remove_all
// size      115 bytes
// spans     0x00402970-0x004029E3
// prototype 
// callers   79   call targets   0
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
            (ORIGINAL(this)->*original_method<func_entry_visitor>(static_cast<unsigned long>(vtable[1])))(payload);
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

const uint32_t StringStructVtable = 0x006693A4;
const uint32_t StringStructVirtualBaseVtable = 0x006693A0;

/*
Purpose: Reset the list to its constructed state, installing both virtual
         tables and releasing every entry.
ORIGINAL: 0x00401060
// name      ?close@StringStruct@@QAEXXZ
// size      153 bytes
// spans     0x00401060-0x004010F9
// prototype void (__thiscall ?close@StringStruct@@QAEXXZ)(StringStruct* this)
// callers   25   call targets   0
Status: Complete
*/
void StringStruct::close_with_tables(uint32_t primary, uint32_t virtual_base) {
    uint8_t *const base = reinterpret_cast<uint8_t *>(this);
    *reinterpret_cast<volatile uint32_t *>(base) = primary;
    // The virtual base's table is reached through the displacement held in the
    // second slot of the vbtable pointed at by offset 4.
    const uint32_t *const vbtable =
        *reinterpret_cast<uint32_t **>(base + 4);
    const uint32_t displacement = vbtable[1];
    *reinterpret_cast<volatile uint32_t *>(base + 4 + displacement) =
        virtual_base;
    // The legacy bodies inline the entry walk; it clears the same fields in
    // the same order, and does nothing at all when the list is already empty.
    // The sweep's surviving swap of the walk against the position reset is
    // observable only to a visitor reading current_position_ mid-walk, and no
    // leaf fixture can walk through close: the real (unmapped) tables are
    // installed first, so the walk tier belongs to the in-process oracle.
    remove_all();
    current_position_ = 0;
}

void StringStruct::close() {
    close_with_tables(StringStructVtable, StringStructVirtualBaseVtable);
}

void __fastcall string_struct_close_redirect(void *adjusted, void *) {
    StringStruct *self = reinterpret_cast<StringStruct *>(
        static_cast<uint8_t *>(adjusted) - StringStructCloseAdjustment);
    self->close();
}

const uint32_t StringStructDerivedVtable = 0x006698C4;
const uint32_t StringStructDerivedVirtualBaseVtable = 0x006698C0;

/*
Purpose: Close a derived string list, releasing its entries under its own
         virtual tables before closing its StringStruct base the same way.
ORIGINAL: 0x004066C0
// name      sub_4066c0
// size      365 bytes
// spans     0x004066C0-0x00406818;0x00650980-0x00650995
// prototype 
// callers   61   call targets   0
Status: Complete
Verification note: the base stage overwrites the derived tables, so with the
non-walking fixtures the oracle can safely drive, the derived stage leaves no
observable trace and dropping it still compares equal. The derived table
addresses and the 0x28 adjustor were instead confirmed by reading the
instruction bytes directly (`mov [ebx-0x28], 0x6698C4` and
`mov [ecx+ebx-0x24], 0x6698C0`).
*/
void __fastcall string_struct_derived_close_redirect(void *adjusted, void *) {
    StringStruct *self = reinterpret_cast<StringStruct *>(
        static_cast<uint8_t *>(adjusted) - StringStructDerivedCloseAdjustment);
    self->close_with_tables(
        StringStructDerivedVtable, StringStructDerivedVirtualBaseVtable);
    self->close();
}

const uint32_t StringVirtualBaseVtable = 0x006693AC;
uint32_t *StringVirtualBaseOwner = (uint32_t *)0x009B3374;

/*
Purpose: Destroy a most-derived StringList: run the source-owned two-stage
         derived close, then hand the virtual base back its own vtable and
         republish the pending-allocation owner the constructor captured.
ORIGINAL: 0x00406820
// name      sub_406820
// size      27 bytes
// spans     0x00406820-0x0040683B
// prototype 
// callers   26   call targets   1
Return Value: EAX residue - the saved owner value, republished into
              *StringVirtualBaseOwner. The original is a void destructor;
              modelled as uint32_t to preserve the residue, as
              GraphicWin::close and Scroll::destroy do.
Status: Complete
Verification note: the delegated close installs the real 0x006698C4 /
0x006693A4 table addresses into [this] before remove_all dispatches through
vtable[1], so the entry walk cannot be driven from the leaf suite - those
addresses are unmapped outside the game process. The sweep's three surviving
constant mutants rewrite hex values that occur only in the margin comments
naming each instruction; the code reads the named constants, so the mutants
are byte-identical and equivalent by construction. The leaf test drives only
non-walking list shapes, which reach every effect of this function; the walk
belongs to 0x004066C0 and is covered in-process by the stringstruct
runtime-oracle suite.
*/
uint32_t StringList::destroy() {
    uint8_t *const base = reinterpret_cast<uint8_t *>(this);
    // `lea esi, [ecx + 0x28]` / `mov ecx, esi` / `call 0x004066C0`. The
    // source-owned derived close is entered on the virtual base and recovers
    // the object by subtracting the same 0x28, which is why the raw pointer
    // is handed to the recovered entry point rather than cast to StringStruct.
    uint8_t *const virtual_base = base + StringListVirtualBaseOffset;
    string_struct_derived_close_redirect(virtual_base, nullptr);

    // The three tail operations, in the original's order. The read of
    // [esi + 4] precedes the store to [esi]; both are volatile so an
    // optimised build keeps the legacy access order.
    volatile uint32_t *const virtual_base_slots =
        reinterpret_cast<volatile uint32_t *>(virtual_base);
    const uint32_t owner = virtual_base_slots[1];    // mov eax, [esi + 4]
    virtual_base_slots[0] = StringVirtualBaseVtable; // mov [esi], 0x006693AC
    *StringVirtualBaseOwner = owner;                 // mov [0x9B3374], eax
    return owner;                                    // EAX at the ret
}

// self == the StringList base: `ret` pops nothing (0xC3), `this` arrives in
// ECX unadjusted, and there are no stack arguments, so the fastcall adapter
// is a straight delegation with no this-adjustment.
uint32_t __fastcall string_list_destructor_redirect(StringList *self, void *) {
    return self->destroy();
}

/*
ORIGINAL: 0x004015B0 BYTE_EXACT
// name      ?seek_pos@StringStruct@@QAEHH@Z
// size      134 bytes
// spans     0x004015B0-0x00401636
// prototype int (__thiscall ?seek_pos@StringStruct@@QAEHH@Z)(StringStruct* this, int)
// callers   2   call targets   1
Status: Complete
*/
typedef int (__cdecl *AbsFnT)(int);

int StringStruct::seek_pos(int a1) {
    AbsFnT abs_fn = reinterpret_cast<AbsFnT>(&abs);
    if (a1 > entry_count_ - 1) {
        return 0;
    }
    current_ = head_;
    if (a1 < 0) {
        if (abs_fn(a1) > entry_count_) {
            return 0;
        }
        int i = abs_fn(a1);
        if (i > 0) {
            do {
                i--;
                current_ = reinterpret_cast<StringStructEntry *>(reinterpret_cast<int *>(current_)[4]);
            } while (i != 0);
        }
        current_position_ = a1 + entry_count_;
        return 1;
    } else {
        if (a1 > 0) {
            int i = a1;
            do {
                i--;
                current_ = reinterpret_cast<StringStructEntry *>(reinterpret_cast<int *>(current_)[3]);
            } while (i != 0);
        }
        current_position_ = a1;
        return 1;
    }
}
