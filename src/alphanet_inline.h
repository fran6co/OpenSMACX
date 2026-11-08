/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */
#pragma once

#include "alphanet.h"
#include "general.h"
#include <cstdlib>

// The picker at 0x0063BB20 constructs this base inline without a later
// throwing helper call; keep the same ordinary C++ lifetime visible here.
// The standalone 0x00401C80 entry retains its argument frame even when this
// visible definition is emitted by a caller TU that otherwise omits frames.
#pragma optimize("y", off)
inline AlphaNetStringStruct::AlphaNetStringStruct() {
    head_ = 0;
    current_ = 0;
    entry_count_ = 0;
    current_position_ = 0;
    allocator_ = 0;
}
#pragma optimize("", on)

// The ordinary allocation and construction definitions are shared by the
// actual node callers; C++ owns base/member construction and unwind.
inline void *allocate_net_node(size_t size, Heap *owner) {
    void *memory;
    if (owner != 0) {
        memory = owner->get(size);
        if (memory == 0) {
            MessageBoxA(0, "out of heap space in new function", "Aborting", MB_OK);
            exit(3);
        }
    } else {
        memory = mem_get(size);
        if (memory == 0) return 0;
    }
    StringAllocationHeap = owner;
    return memory;
}

inline void *NetAllocationNode::operator new(size_t size, Heap *owner) throw() {
    return allocate_net_node(size, owner);
}

inline void *NetAllocationNode::operator new(size_t size, AlphaNetStringStruct *list) throw() {
    // Native receiver-context overload is the fallback ONLY, selected after
    // the caller has found allocator_==0. Do not re-read allocator_ here.
    return allocate_net_node(size, list->allocation_owner_);
}

#pragma optimize("y", off)
inline NetTextListBase::NetTextListBase() {
    head_ = 0;
    current_ = 0;
    entry_count_ = 0;
    current_position_ = 0;
    allocator_ = 0;
}
#pragma optimize("", on)

#pragma optimize("y", off)
inline NetUnk72C::NetUnk72C() {
    // Source text and field_20_ remain uninitialized, as in 402D10.
}
#pragma optimize("", on)

#pragma optimize("y", off)
inline NetAllocationNode::NetAllocationNode() {
    // No scalar field stores exist in either inline original construction.
    // Its own allocation base captures the pending Heap BEFORE entries_'s
    // independent virtual base consumes the now-cleared handoff.
}
#pragma optimize("", on)

// Visible to alphanet.cpp: ~ServiceList (0x004E3730) expands it there.
// 0x004016C0 is the standalone copy.
#pragma optimize("y", off)
inline ServiceProviderList::~ServiceProviderList() {
    close();
}
#pragma optimize("", on)
