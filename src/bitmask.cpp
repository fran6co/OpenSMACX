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
Purpose: Count the number of unsigned bits set. Replaced the original code with Brian Kernighan's 
// ORIGINAL: 0x0050BA30 ?bit_count@@YAHH@Z 0x0050BA30-0x0050BA4E BYTE_EXACT
// TRIED (2026-09-03, probed against cl 12.00.8168 directly): the image's
//   `shr ebx, 1 / adc eax, 0` carry-chain fusion is NOT producible from VC6
//   C++, so this is a second case of hand-written assembly that
//   `tools/handwritten_asm.py` does not catch - it scans for `loop`, bare
//   `lodsb`, `xlatb`, `pushf/popf` and has no fingerprint for a shift-carry
//   accumulator. Twelve spellings compiled to assembly in isolation and
//   every one emitted `mov edx, ecx; and edx, 1; add eax, edx; shr ecx, 1;
//   jne`: do-while/while/for, accumulate-before-shift and after, `unsigned`
//   vs `int` counter, `if (bits & 1) count++`, the shift moved into the loop
//   CONDITION (`while (bits >>= 1)`), a 64-bit `unsigned __int64` accumulator
//   (that one does emit `adc`, but only as the HIGH half alongside the
//   `add eax, esi`, so the shape cannot converge), assignment to the
//   parameter before `return` to reach the image's
//   `mov [ebp+8],eax; mov eax,[ebp+8]` tail - which is reachable and is the
//   one half of the shape worth keeping. The image's loop is also rotated
//   with a duplicated trailing `adc` that no source form produced. Ceiling is
//   semantic equivalence, as for `checksum`/`checksum_password` above; 3/15
//   stands. Worth adding the fingerprint to handwritten_asm.py.
// size      30 bytes
// prototype
// callers   34   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// TRIED: sixteen source shapes (do-while/while, `count += bits & 1`,
//   `if (bits & 1) count++`, a captured-carry temp, shift-before-test) all
//   land on the same 3/15, `push ebx` vs `mov ecx, [ebp+8]` at instruction 2
//   - none coax VC6 into the image's shr/adc carry-chain fusion; this is the
//   right algorithm (shift-and-accumulate, matching the shr/adc chain, not
//   Kernighan's clear-lowest-set-bit trick) and it scores strictly better
//   than the `n &= n - 1` form it replaced, but still MISMATCH.
Return Value: the number of set bits
Status: Complete
*/
// Hand-written assembly: shr/adc carry-count loop, result stored back to the parameter.
int __cdecl bit_count(int bitfield) {
    __asm {
        xor eax, eax
        mov ebx, bitfield
    count_loop:
        shr ebx, 1
        je count_done
        adc eax, 0
        jmp count_loop
    count_done:
        adc eax, 0
        mov bitfield, eax
    }
    return bitfield;
}
