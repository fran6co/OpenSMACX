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

// Sound type dispatch and query are compiled apart from their lifecycle
// callers. The pinned constructors retain calls to set_type; making its
// definition visible there lets VC6 inline them instead. This is an inferred
// compiler-visibility boundary, not a recovered historical filename or a
// complete original translation-unit boundary. Keep the dispatch table with
// its owning method and preserve the original selector effects.
#include "stdafx.h"
#include "sound.h"

// RECOVERY_ASSOCIATED: v1 switch_targets 0x004C61E0 0x004C6260 0x004C627C 0x004C61EE
/*
Purpose: Record the sound's type. Types 1..7 - except 3, which the original's
         jump table routes to the invalid arm - store the type at 0x50 and OR
         a per-type class bit into the flag dword at 0x40: 1 -> 4, 2 -> 8,
         4 -> 0x10, 5 -> 0x28, 6 -> 0x100, 7 -> 0x80. Anything else stores
         type 0 and leaves the flags alone.
// ORIGINAL: 0x004C61E0 ?set_type@Sound@@QAEXI@Z 0x004C61E0-0x004C6260 BYTE_EXACT
// symbol    ?set_type@Sound@@UAEXI@Z
// size      128 bytes
// prototype void (__thiscall ?set_type@Sound@@QAEXI@Z)(Sound* this, SOUNDTYPE)
// callers   5   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// LEVER, two parts. (1) each `case` inlines its own `type_ = a1;
//        flags_40_ |= N; return;` rather than sharing a `class_bit` local
//        and a common tail - the image duplicates the four-instruction
//        block per arm with no merged epilogue. (2) the CATALOGUED mapping
//        1->4 / 4->0x10 is backwards: the jump table's own byte order says
//        1->0x10 and 4->4 - `constant #9/#21 or` swapped is what measure
//        named it, and swapping the two immediates is what closed it.
Return Value: n/a
Status: Complete
*/
// The historical LEVER above mistook instruction order for selector order.
// The original seven-entry switch table at 4C6260..4C627C lies outside the
// recorded instruction span: its type-1 entry reaches 4C6215 (OR 4), and
// its type-4 entry reaches 4C61F5 (OR 0x10). Keep that dispatch as well as
// the original arm order; matching the instructions alone misses this error.
void Sound::set_type(unsigned int type) {
    // EACH ARM INLINE, not a shared `type_ = type; flags_40_ |= class_bit;`
    // tail: the image duplicates the store-and-OR in every case block and
    // returns from each directly, rather than merging them.
    switch (type) {
    case 4:
        flags_40_ |= 0x10;
        type_ = type;
        return;
    case 2:
        flags_40_ |= 8;
        type_ = type;
        return;
    case 1:
        flags_40_ |= 4;
        type_ = type;
        return;
    case 5:
        flags_40_ |= 0x28;
        type_ = type;
        return;
    case 6:
        flags_40_ |= 0x100;
        type_ = type;
        return;
    case 7:
        flags_40_ |= 0x80;
        type_ = type;
        return;
    default:
        type_ = 0;
        return;
    }
}

// Purpose: Return the type field written by Sound::set_type (family slot 30).
// ORIGINAL: 0x00448330 sub_448330 0x00448330-0x00448334 BYTE_EXACT
// symbol    ?get_type@Sound@@UAEIXZ
uint32_t Sound::get_type() {
    return type_;
}

