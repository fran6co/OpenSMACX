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
#include "general.h"

/*
Purpose: Divide, rounding the quotient AWAY from zero when there is a
         remainder, and report both.

             mov eax,ecx / cdq / idiv esi / mov edx,eax / imul edx,esi
             sub ecx,edx / mov [edx],ecx / je zero / inc eax / mov [ecx],eax
             ret 0x10   zero: mov [edx],eax / ret 0x10

         The `je` is three instructions after the `sub` that sets it, with two
         `mov`s and a `pop` in between - none of which touch flags. It tests
         whether the REMAINDER was zero, not anything nearer to it.

         `idiv` truncates toward zero, so the +1 rounds away from zero rather
         than up: -7 by 2 gives a remainder of -1 and a quotient of -3, which
         becomes -2. Both traps of the original are kept, because both
         spellings go through one division: a zero divisor and INT_MIN / -1.

// HISTORICAL scaffold role: SCAFFOLD Transitional leaf collection, not completed genuine owner recovery; preserve body evidence during owner migration.
// ORIGINAL: 0x00532A50 sub_532a50 0x00532A50-0x00532A81 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned terranx_original.exe SHA-256 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf at 0x00532A50: decoded complete span accesses only explicit stack arguments, without an ECX receiver or object storage.", "Source-owned general utility implementation; current source family placement does not establish the original translation unit.", "Whole file-backed PE examination found no HIGHLOW operand or raw E8-relative target for this entry; indirect calls and non-relocated references remain unresolved."], "reason": "Own the primitive-only operation as a complete source-level general utility. No class, global, object layout, or original source-TU identity is claimed."}
// symbol    ?divide_and_increment_quotient_if_remainder@@YGHHPAH0H@Z
// historical original marker: 0x00532A50 sub_532a50 0x00532A50-0x00532A81 BYTE_EXACT
// historical scaffold emitted-symbol hint: ?leaf_00532a50_redirect@@YGHHPAH0H@Z
// size      49 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the quotient that was stored
Status: Complete
*/
// HISTORICAL correction: the former prose called this away-from-zero; that label was inaccurate.
// Preserve the exact operation below: truncating IDIV quotient, remainder store, then quotient increment on any nonzero remainder.


int __stdcall divide_and_increment_quotient_if_remainder(int value, int *quotient_out,
                                     int *remainder_out, int divisor) {
    // Increment the quotient IN PLACE. A `rounded` temp - whether built by a
    // ternary or by an `if` - keeps one more value live, and VC6 then stops
    // spilling the divisor to esi, which is the `push` at index 3.
    int quotient = value / divisor;
    const int remainder = value - quotient * divisor;
    *remainder_out = remainder;
    if (remainder != 0) {
        ++quotient;
    }
    *quotient_out = quotient;
    return quotient;
}
/*
Purpose: Is a point inside an inclusive rectangle?

             mov eax,[ebp+8] / mov ecx,[ebp+0x10] / cmp eax,ecx / jl no
             cmp eax,[ebp+0x18] / jg no / mov eax,[ebp+0xc]
             mov ecx,[ebp+0x14] / cmp eax,ecx / jl no / cmp eax,[ebp+0x1c]
             jg no / mov eax,1 / ret     no: xor eax,eax / ret

         Six arguments in the order x, y, left, top, right, bottom - the two
         coordinates first and the bounds interleaved after, which is not the
         (left, top, right, bottom, x, y) an eye expects.

         `jl` and `jg` are SIGNED, and both bounds are INCLUSIVE: a point
         exactly on an edge is inside.

// HISTORICAL scaffold role: SCAFFOLD Transitional leaf collection, not completed genuine owner recovery; preserve body evidence during owner migration.
// ORIGINAL: 0x00592DB0 sub_592db0 0x00592DB0-0x00592DDC BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned terranx_original.exe SHA-256 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf at 0x00592DB0: decoded complete span accesses only explicit stack arguments, without an ECX receiver or object storage.", "Source-owned general utility implementation; current source family placement does not establish the original translation unit.", "Whole file-backed PE examination found no HIGHLOW operand or raw E8-relative target for this entry; indirect calls and non-relocated references remain unresolved."], "reason": "Own the primitive-only operation as a complete source-level general utility. No class, global, object layout, or original source-TU identity is claimed."}
// symbol    ?point_in_inclusive_rect@@YAHHHHHHH@Z
// historical original marker: 0x00592DB0 sub_592db0 0x00592DB0-0x00592DDC BYTE_EXACT
// historical scaffold emitted-symbol hint: ?leaf_00592db0_redirect@@YAHHHHHHH@Z
// size      44 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 1 when inside, 0 otherwise
Status: Complete
*/

int __cdecl point_in_inclusive_rect(int x, int y, int left, int top,
                                   int right, int bottom) {
    if (x < left || x > right) {
        return 0;
    }
    if (y < top || y > bottom) {
        return 0;
    }
    return 1;
}
/*
Purpose: Calculate the offset and bitmask for the specified input.
// ORIGINAL: 0x0050BA00 ?bitmask@@YAXHPAHPAH@Z 0x0050BA00-0x0050BA27 BYTE_EXACT
// symbol    ?bitmask@@YAXHPAH0@Z
// size      39 bytes
// prototype void (__cdecl ?bitmask@@YAXHPAHPAH@Z)(int input, int* offset, int* mask)
// callers   103   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
// Pinned 0x50BA00..0x50BA27 performs signed /8, then writes offset before
// the mask. The by-value input preserves callers that alias an out slot.
// Original mapped callers use this shared utility; the older header-inline
// policy was not supported by positive inline evidence.
void __cdecl bitmask(int input, int *offset, int *mask) {
    const int shift = input & 7;
    *offset = input / 8;
    *mask = 1 << shift;
}
/*
Purpose: Use the min and max parameters to bound the input.
// ORIGINAL: 0x00422F00 ?range@@YAHHHH@Z 0x00422F00-0x00422F1E BYTE_EXACT
// body      src/general.h
// size      30 bytes
// prototype int (__cdecl ?range@@YAHHHH@Z)(int input, int min, int max)
// callers   10   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Bounded input
Status: Complete
*/
// BODY IN general.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.

// Current structural interpretation supersedes the seed-history binding's
// integer parallel-base rationale: pinned 4D5F92 loads arg0 as the parallel
// cursor and 4D5F95 loads arg1 as the sorted values. The body advances both by
// four bytes and passes adjacent int cells to swap(int *, int *).
// This sorting family is homed beside application numeric primitives; this does
// not assert an original translation-unit boundary or identify either array's
// application-level owner. No direct native caller establishes such a domain.
// The original integer address arithmetic's invalid-pointer/address-wrap
// domain and 32-bit count-1 overflow behavior are not resolved by this typed
// source contract. This is not an exactness or bug-fix claim.
/*
Purpose: Bubble-sort one int array descending while applying the identical
         exchanges to a parallel array addressed relative to it.
// ORIGINAL: 0x004D5F70 sub_4d5f70 0x004D5F70-0x004D5FE6 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Original4D5F70..4D5FE6; retired seed git:b18d3ac8:src/unrecovered/004d5f70.cpp records the same identity and spans.", "The pinned body calls integer swap at 0x628A50; the typed declaration and implementation are owned by general.h/general.cpp.", "The parallel array is reached by the same base-plus-index arithmetic the seed measured."], "reason": "Coverage transcription of the paired bubble sort with the source-owned integer swap; the parallel-array base stays an int because no owning array is named anywhere in the tree."}
// symbol    _sub_4d5f70
Status: Coverage transcription - unclaimed body
*/
extern "C" void __cdecl sub_4d5f70(int *parallel, int *values, int count) {
    int limit = count - 1;
    int swapped;
    do {
        swapped = 0;
        if (limit < 1) {
            return;
        }
        int *cursor = parallel;
        int *value = values;
        int remaining = limit;
        do {
            int *next = value + 1;
            if (*value < *next) {
                swapped = 1;
                swap(value, next);
                swap(cursor, cursor + 1);
            }
            cursor = cursor + 1;
            remaining = remaining - 1;
            value = next;
        } while (remaining != 0);
    } while (swapped != 0);
}

/*
Purpose: For the count, sort both id and value arrays by the least to greatest value (ascending).
// ORIGINAL: 0x005B5690 ?sort@@YAXHPAHPAH@Z 0x005B5690-0x005B56F5 BYTE_EXACT
// TRIED (2026-09-03): dropping this body's `int bounds = count - 1` local
//   and writing `i < count - 1` inline, because the image reloads `count`
//   from its argument slot at the top of every pass iteration
//   (`mov ecx, [ebp+8]` at 0x005B56A9, then `dec ecx`) and so cannot have a
//   `bounds` variable. Measured 5/47 before and after - the compiler keeps
//   `count - 1` in a register either way - so the inline spelling is kept
//   only because it stops declaring a local the image has not got; the
//   unreproduced shape is the memory-homed `i` (`push ecx` slot at [ebp-4],
//   written and re-read around the swap) plus the running pointers
//   `esi`/`edi`/`ebx`, which is a strength-reduction choice, not a source
//   one. Left at the plateau.
// The pinned original resets a pass flag at its head, sets it after both
// swaps, and restarts immediately after the first swap in the pass.
// symbol    ?sort@@YAXHPAH0@Z
// size      101 bytes
// prototype void (__cdecl ?sort@@YAXHPAHPAH@Z)(int count, int* id, int* value)
// callers   2   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00628A50
Return Value: n/a
Status: Complete
*/
void __cdecl sort(int count, int *id, int *value) {
    int swapped;
    do {
        swapped = 0;
        for (int i = 0; !swapped && i < count - 1; ++i) {
            if (value[i] > value[i + 1]) {
                swap(&value[i], &value[i + 1]);
                swap(&id[i], &id[i + 1]);
                swapped = 1;
            }
        }
    } while (swapped != 0);
}

/*
Purpose: For the count, sort both id and value arrays by the greatest to least value (descending).
Original Offset: n/a
Return Value: n/a
Status: Complete
*/
void __cdecl sort_descending(uint32_t count, int *id, int *value) {
    int bounds = count - 1;
    BOOL has_swapped;
    do {
        has_swapped = false;
        for (int i = 0; i < bounds; i++) {
            if (value[i] < value[i + 1]) {
                has_swapped = true;
                swap(&value[i], &value[i + 1]);
                swap(&id[i], &id[i + 1]);
            }
        }
    } while (has_swapped);
}
