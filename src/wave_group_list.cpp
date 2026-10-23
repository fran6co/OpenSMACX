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
#include "stdafx.h"
#include "wave.h"
#include "wave_device.h"

/*
Purpose: Append a wave to a group's node list. A fresh 12-byte node comes
         from the game heap and joins at the tail; an empty list makes it
         both ends. The original presses on when the allocation fails: a
         live tail would fault writing the null node's prev, and an empty
         list quietly "appends" the null while still counting it - both
         shapes are kept. With a live tail the new node's prev is the
         RE-READ tail field, after the old tail's next was written - an
         order the original's aliasing permits to matter.
// ORIGINAL: 0x004C5BF0 sub_4c5bf0 0x004C5BF0-0x004C5C4A BYTE_EXACT
// symbol    ?insert@WaveGroupList@@QAEXPAVWave@@@Z
// kind      game
// size      90 bytes
// callers   1   call targets   1
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0064558A
// LEVER: `WaveGroupNode *const node` -> non-const with an explicit
// `else { node = 0; }`. With the pointer const-initialized from the `if
// (node) {...}` alone, this tree let the null path fall straight into the
// tail-check with `eax` already zero from the allocator's own return,
// dropping the image's own `jmp`/`xor eax,eax` merge pair (0x004C5C17-
// 0x004C5C19) - 34/36 instructions, 0.971 similar. Materializing the
// redundant zero explicitly reproduces the image's (needless) re-zero and
// closes the last two instructions.
Return Value: n/a
Status: Complete
*/
void WaveGroupList::insert(Wave *a1) {
    WaveGroupNode *node =
        static_cast<WaveGroupNode *>(operator new(0xC));
    if (node) {
        node->prev = nullptr;
        node->next = nullptr;
        node->wave = a1;
    } else {
        node = 0;
    }
    WaveGroupNode *const tail = this->tail;
    if (tail) {
        // The old tail's next is written first and the tail field re-read
        // second, exactly as the original orders its memory operations; the
        // volatile views keep the pair honest when the two locations alias
        // (the optimizer's type-based analysis would otherwise fold the
        // re-read).
        *reinterpret_cast<WaveGroupNode *volatile *>(&tail->next) = node;
        node->prev = *reinterpret_cast<WaveGroupNode *volatile *>(&this->tail);
        this->tail = node;
        this->count += 1;
    } else {
        this->head = node;
        this->tail = node;
        this->count += 1;
    }
}

