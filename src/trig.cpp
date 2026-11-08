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
#include <math.h> // sin(double), lowered to the CRT's _sin
#include "general.h"

// The game's fixed-point sine, defined below in image order.
int __cdecl sin(int angle, int scale);

// Inferred TU 0x0063B940-0x0063BAD9: after Cursor, ended by the Sprite
// initializer at 0x0063BAE0. SinTable follows CursorWidth/CursorHeight in
// .bss, so it may instead be the tail of the Cursor TU.

// Original trig_init at 0x0063B950 selects the output cursor; sin at 0x0063B9E0 indexes this base; no extent claim.
// RECOVERY_OBJECT: v1 0x009BE6DC ?SinTable@@3PAHA
int SinTable[256]; // 0x009BE6DC

/*
Purpose: Fill SinTable, the 256-entry fixed-point sine table the game's own
         `int sin(int angle, int scale)` (0x0063B9B0) interpolates, then make
         two fixed calls through it. The table step is 2pi/1020 and the scale
         is 65535.0, both read as .rdata doubles in the image at 0x00671030
         and 0x00671028; each entry is sin(sample * step) * scale truncated
         to int by the CRT's __ftol. The two trailing calls pass angle 2.0
         and 0xe02d82d7 with scale 100; the first result is overwritten by
         the second, which stays in eax as this function's return.
// ORIGINAL: 0x0063B940 ?trig_init@@YAHXZ 0x0063B940-0x0063B9AE FILE BYTE_EXACT
// TRIED 2026-09-03 (pair /O2 and /O2 /Oy- only; STOPPED, evidence recorded):
//   the body is RIGHT and the gap is the /Oi intrinsic axis, which no source
//   spelling reaches. The image CALLS the CRT `sin` (0x006463E4); both allowed
//   sets imply /Oi, and /Oi turns `sin(double)` into the x87 `fsin` - which
//   also deletes the image's `and esp, 0xfffffff8`, because that alignment
//   exists only to receive the stack-passed double the CRT call needs. 3 of 33
//   agreeing, compiled 29 against the image's 33. Measured directly that
//   `/c /O2 /Oi- /Gy /GR- /GX` on this exact body gives the image's
//   instruction sequence back byte for byte, `and esp` and the separate
//   `call _sin` / `call __ftol` included, so it is the intrinsic axis and
//   nothing else. `#pragma function(sin)` beside the `strcat`/`strcpy`/
//   `strlen`/`strcmp` list in stdafx.h is the source-side fix, and
//   the retired 0x00634F20 seed (git fee2514f) and 00627440.cpp both show per-file
//   `#pragma function` is an established device in this tree - but it is
//   either a shared-file edit or a pragma, and this pass is barred from both.
// LEVER: index the table (`SinTable[sample]`) and test the int counter signed;
// a separate `int *entry` cursor makes VC6 compare the pointer unsigned
// (`jb` where the image has `jl`) and hold the counter in ecx instead of edx.
// The loop is a plain `for` - VC6 bottom-tests it itself.
// size      110 bytes
// prototype int __cdecl trig_init(void)
// callers   1   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0063B9B0 0x006458C8 0x006463E4
Return Value: n/a
Status: Complete
*/
int __cdecl trig_init() {
// RECOVERY_LITERAL: v1 0x00671028 0x00671030 __real@8@400effff000000000000 0x0063B940
// RECOVERY_LITERAL: v1 0x00671030 0x00671038 __real@8@3ff7c9d9b456e9139000 0x0063B940
    for (int sample = 0; sample < 256; sample++) {
        SinTable[sample] = static_cast<int>(
            sin(static_cast<double>(sample) * 0.006159985596078431) * 65535.0);
    }
    sin(0x40000000, 100);
    return sin(static_cast<int>(0xe02d82d7), 100);
}

/*
// ORIGINAL: 0x0063B9B0 ?sin@@YAHHH@Z 0x0063B9B0-0x0063BABB BYTE_EXACT
// PROMOTED 2026-08-30 from src/unrecovered/0063b9b0.cpp: the edi pressure came
// from NAMING the second table value. Loading SinTable[index + 1] straight
// into the expression that subtracts t0 gives the value's whole life to eax,
// and shifting `scale` in place before the loads (not at the use) puts the
// `sar edx, N` where the image has it.
// size      267 bytes
// prototype
// callers   2   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
int __cdecl sin(int angle, int scale) {
    if (angle & 0x80000000) {
        scale = -scale;
    }
    const int quad = angle & 0x40000000;
    const int masked = angle & 0x3fffffff;
    const int index = masked >> 22;
    // The table loads are spelled in BOTH arms and let the compiler CSE them
    // to the merge point after the quadrant test - the image's order is test,
    // load t0, load t1, branch, which is what frees esi for t0.
    if (scale < 0xffff) {
        if (quad != 0) {
            const int t0 = SinTable[index];
            return (((SinTable[index + 1] - t0) * (masked & 0x3fffff) >> 22)
                     - t0 + 0xffff)
                   * scale
                   >> 16;
        }
        const int t0 = SinTable[index];
        return (((SinTable[index + 1] - t0) * (masked & 0x3fffff) >> 22) + t0)
               * scale
               >> 16;
    }
    if (scale < 0xffffff) {
        scale >>= 8;
        if (quad != 0) {
            const int t0 = SinTable[index];
            return (((SinTable[index + 1] - t0) * (masked & 0x3fffff) >> 22)
                     - t0 + 0xffff)
                   * scale
                   >> 8;
        }
        const int t0 = SinTable[index];
        return (((SinTable[index + 1] - t0) * (masked & 0x3fffff) >> 22) + t0)
               * scale
               >> 8;
    }
    scale >>= 16;
    if (quad != 0) {
        const int t0 = SinTable[index];
        return (((SinTable[index + 1] - t0) * (masked & 0x3fffff) >> 22)
                 - t0 + 0xffff)
               * scale;
    }
    const int t0 = SinTable[index];
    return (((SinTable[index + 1] - t0) * (masked & 0x3fffff) >> 22) + t0)
           * scale;
}

/*
// ORIGINAL: 0x0063BAC0 ?cos@@YAHHH@Z 0x0063BAC0-0x0063BAD9 BYTE_EXACT
// symbol    ?cos@@YAHHH@Z
// size      25 bytes
// prototype
// callers   0   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x0063B9B0
Status: Complete
*/
int __cdecl cos(int a1, int a2) {
    return sin(static_cast<int>(static_cast<uint32_t>(a1) + 0x3fffffffU), a2);
}
