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
#include "buffer.h"

// Shared scratch words used only by the mapped mask-copy routine in the
// observed direct reference set. Names describe roles, not original names;
// separate bindings do not assert an original aggregate or exclude aliases.
// RECOVERY_OBJECT: v2 0x009B3A80 0x009B3A84 ?BufferMaskGroupCount@@3HA 0x005E2B00
int BufferMaskGroupCount = 0;
// RECOVERY_OBJECT: v2 0x009B3A84 0x009B3A88 ?BufferMaskSourceRowGap@@3HA 0x005E2B00
int BufferMaskSourceRowGap = 0;
// RECOVERY_OBJECT: v2 0x009B37D8 0x009B37DC ?BufferMaskDestinationRowGap@@3HA 0x005E2B00
int BufferMaskDestinationRowGap = 0;

/*
Purpose: Blit one rectangle from this buffer into `dest`, replacing every
         destination pixel whose current colour is the mask colour. The
         clipping is done in three stages - negative source coordinates are
         folded into the extent, the extent is clamped to the source's own
         DIB, and the surviving rectangle is intersected with the
         destination's clip - which is why the two `get_data` calls below
         are handed DIFFERENT origins.

         HOMED from src/unrecovered/005e2b00.cpp, which held this
         transcription. The direction is the non-obvious part and the
         archive's own note records it: `dest` (the argument) is the buffer
         WRITTEN and mask-tested, `this` is the one READ.
// ORIGINAL: 0x005E2B00 ?copy_mask@Buffer@@QAEHPAVBuffer@@HHHHHHH@Z 0x005E2B00-0x005E2DD3 FILE
// symbol    ?copy_mask@Buffer@@QAEHPAV1@HHHHHHH@Z
// size      723 bytes
// prototype int (__thiscall ?copy_mask@Buffer@@QAEHPAVBuffer@@HHHHHHH@Z)(Buffer* this, Buffer*, int, int, int, int, int, int, int)
// callers   2   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005E33F3 0x005E34A3
// indirect  0x005E2C27
// TRIED: plain nested for-loop instead of the Duff's-device-style 4x-unrolled byte-compare-copy the original lowers to; edi/esi roles are swapped from the naive reading - `a1` (the argument) is the one WRITTEN and mask-tested, `this` is the one READ; diverges at instr #0 (prologue shape)
// TRIED: the residual is the INNER LOOP, not the prologue, and it is a wall
//        of the same kind as `fill`/`copy` in this file. The image's scan is
//        a DUFF'S DEVICE - `switch (width & 3)` falling into a do/while that
//        runs `width / 4 + (width & 3 ? 1 : 0)` times - and it keeps its
//        state in fixed .data, not in locals: the case counter at 0x009B3A80,
//        the `this`-side row remainder at 0x009B3A84 and the destination-side
//        one at 0x009B37D8, all written with `mov dword ptr [abs]` and
//        incremented inside the dispatch. The dispatch itself is a chain of
//        `lea edx, <case>` / `jmp edx` computed jumps, which is not how VC6
//        lowers a switch (it builds a table), so this block is hand-written
//        assembly in the original and byte-exactness is not reachable from
//        C++. `handwritten_asm.py` does not flag it because `jmp reg` is not
//        on its opcode list. The nested loop here is the same program - the
//        direction was verified against the bytes (`edi` = dest->get_data is
//        compared and written, `esi` = get_data on `this` is read) - and the
//        three extra stride/width locals are the image's three globals.
*/
int Buffer::copy_mask(Buffer *dest, int src_x, int src_y, int x, int y, int w,
                      int h, int color) {
    if (dib_bits_ == 0 && surface_ == 0) {
        return 7;
    }
    if (dest == 0) {
        return 3;
    }

    // Fold negative source coordinates into the extent: the part of the
    // rectangle that falls above/left of the source is not drawn, and the
    // destination origin moves with it.
    if (src_x < 0) {
        w += src_x;
        x -= src_x;
        src_x = 0;
    }
    if (src_y < 0) {
        h += src_y;
        y -= src_y;
        src_y = 0;
    }

    const int source_width = dib_.bmiHeader.biWidth;
    const int source_height = -dib_.bmiHeader.biHeight;

    if (source_width < src_x) {
        return 0;
    }
    if (source_height < src_y) {
        return 0;
    }

    if (source_width < src_x + w) {
        w = source_width - src_x;
    }
    if (source_height < src_y + h) {
        h = source_height - src_y;
    }

    RECT clipped;
    clipped.left = x;
    clipped.top = y;
    clipped.right = x + w;
    clipped.bottom = y + h;

    if (!IntersectRect(&clipped, &clipped, &dest->rect1_)) {
        return 0;
    }

    // Whatever the intersection took off the left and top edges is taken off
    // the source origin too, so the two reads stay in step.
    const int inset_x = clipped.left - x;
    const int inset_y = clipped.top - y;

    unsigned char *write_ptr = dest->get_data(clipped.left, clipped.top);
    if (write_ptr == 0) {
        return 3;
    }
    unsigned char *read_ptr = get_data(src_x + inset_x, src_y + inset_y);
    if (read_ptr == 0) {
        return 3;
    }

    const int write_stride = dest->stride_;
    const int read_stride = stride_;
    const int rows = clipped.bottom - clipped.top;
    const int columns = clipped.right - clipped.left;
    BufferMaskGroupCount = columns >> 2;
    BufferMaskSourceRowGap = read_stride - columns;
    BufferMaskDestinationRowGap = write_stride - columns;

    int groups = BufferMaskGroupCount;
    int rows_left = rows;
    const unsigned char mask = static_cast<unsigned char>(color);
    const int remainder = columns & 3;
    if (remainder != 0) {
        ++groups;
        BufferMaskGroupCount = groups;
    }

    // IntersectRect admitted positive extents. The remainder selects the
    // first pixel of each four-pixel group, including every later row.
    do {
        int groups_left = groups;
        switch (remainder) {
        case 0:
            do {
                if (*write_ptr == mask) {
                    *write_ptr = *read_ptr;
                }
                ++read_ptr;
                ++write_ptr;
        case 3:
                if (*write_ptr == mask) {
                    *write_ptr = *read_ptr;
                }
                ++read_ptr;
                ++write_ptr;
        case 2:
                if (*write_ptr == mask) {
                    *write_ptr = *read_ptr;
                }
                ++read_ptr;
                ++write_ptr;
        case 1:
                if (*write_ptr == mask) {
                    *write_ptr = *read_ptr;
                }
                ++read_ptr;
                ++write_ptr;
            } while (--groups_left != 0);
        }
        groups = BufferMaskGroupCount;
        read_ptr += BufferMaskSourceRowGap;
        write_ptr += BufferMaskDestinationRowGap;
    } while (--rows_left != 0);

    free_data(1);
    dest->free_data(1);
    return 0;
}

/*
Purpose: Acquire the buffer's pixel data, locking the DirectDraw surface on the
         first reference and counting every acquisition.
// ORIGINAL: 0x005E3373 ?get_data@Buffer@@QAEHXZ 0x005E3373-0x005E33EA BYTE_EXACT
// body      src/buffer_mask.cpp
// symbol    ?get_data@Buffer@@QAEPAXXZ
// size      119 bytes
// prototype int (__thiscall ?get_data@Buffer@@QAEHXZ)(Buffer* this)
// callers   48   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005E33BF
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/

// Canonical pixel acquisition keeps the original direct-call boundary.
// Drawing methods with original inline acquisition share acquire_pixels().
void *Buffer::get_data() {
    return acquire_pixels();
}
