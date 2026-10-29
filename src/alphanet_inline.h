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
