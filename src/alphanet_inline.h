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
// The standalone 0x00401C80 entry is the out-of-line copy of the
// 0x00401000 game unit (/Oy-), the first object that referenced it.
// WORKAROUND(inline): no recovered function of that unit (alphanet_records.cpp,
// stringstruct.cpp) references the constructor out of line, so the marker is
// hosted by net_class.cpp (/Oy) and this region gives the copy that unit's frame.
#pragma optimize("y", off)
inline AlphaNetStringStruct::AlphaNetStringStruct() {
    head_ = 0;
    current_ = 0;
    entry_count_ = 0;
    current_position_ = 0;
    allocator_ = 0;
}
#pragma optimize("", on)

inline void *NetAllocationNode::operator new(size_t size, Heap *owner) throw() {
    void *memory;
    if (owner != 0) {
        memory = owner->get(size);
        if (memory != 0) {
            StringAllocationHeap = owner;
            return memory;
        }
        MessageBoxA(0, "out of heap space in new function", "Aborting", MB_OK);
        exit(3);
    }
    memory = mem_get(size);
    if (memory == 0) return 0;
    StringAllocationHeap = owner;
    return memory;
}

inline void *NetAllocationNode::operator new(size_t size, StringAllocationBase *base) throw() {
    Heap *owner = base->allocation_owner_;
    void *memory;
    if (owner != 0) {
        memory = owner->get(size);
        if (memory != 0) {
            StringAllocationHeap = owner;
            return memory;
        }
        MessageBoxA(0, "out of heap space in new function", "Aborting", MB_OK);
        exit(3);
    }
    memory = mem_get(size);
    if (memory == 0) return 0;
    StringAllocationHeap = owner;
    return memory;
}

inline void *NetAllocationNode::operator new(size_t size, AlphaNetStringStruct *list) throw() {
    Heap *owner = list->allocation_owner_;
    void *memory;
    if (owner != 0) {
        memory = owner->get(size);
        if (memory != 0) {
            StringAllocationHeap = owner;
            return memory;
        }
        MessageBoxA(0, "out of heap space in new function", "Aborting", MB_OK);
        exit(3);
    }
    memory = mem_get(size);
    if (memory == 0) return 0;
    StringAllocationHeap = owner;
    return memory;
}

inline NetTextListBase::NetTextListBase() {
    head_ = 0;
    current_ = 0;
    entry_count_ = 0;
    current_position_ = 0;
    allocator_ = 0;
}

inline NetUnk72C::NetUnk72C() {
    // Source text and field_20_ remain uninitialized, as in 402D10.
}

inline NetAllocationNode::NetAllocationNode() {
    // No scalar field stores exist in either inline original construction.
    // Its own allocation base captures the pending Heap BEFORE entries_'s
    // independent virtual base consumes the now-cleared handoff.
}

// Visible to alphanet.cpp: ~ServiceList (0x004E3730) expands it there.
// 0x004016C0 is the standalone copy.
inline ServiceProviderList::~ServiceProviderList() {
    close();
}
