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
#include "original_seam.h"
#include "sprite.h"
#include "buffer.h"

#include <new>
#include "general.h"   // mem_get, for the placeholder pixel
#include "heap.h"      // Heap::get, behind TexHeap::get_mem below
#include "filewin.h"
#include "spritebox.h"
#include <stdlib.h>

/*
Purpose: Initialize an empty sprite and charge its own size to the sprite
         memory total.
// ORIGINAL: 0x005E37E0 ??0Sprite@@QAE@XZ 0x005E37E0-0x005E3815 BYTE_EXACT
// size      53 bytes
// prototype void (__thiscall ??0Sprite@@QAE@XZ)(Sprite* this)
// callers   154   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
Sprite::Sprite() {
    // The legacy body interleaves the accounting update between the first two
    // field stores and writes the type byte late, so the observable store
    // order is pinned rather than left to the compiler.
    // THE MEMBERS, still volatile: the store ORDER is what is pinned here,
    // not the punning. Each `&member` keeps the exact width and the exact
    // sequence the image writes, and the class stops walking itself.
    *reinterpret_cast<volatile uint32_t *>(&ppszFileName_) = 0;
    *SpriteMemoryUsed = static_cast<int>(
        static_cast<uint32_t>(*SpriteMemoryUsed) + sizeof(Sprite));
    *reinterpret_cast<volatile uint32_t *>(&pcBits_) = 0;
    *reinterpret_cast<volatile uint32_t *>(&iSpriteWidth2_) = 0;
    *reinterpret_cast<volatile uint32_t *>(&iSpriteWidth_) = 0;
    *reinterpret_cast<volatile uint32_t *>(&iSpriteHeight_) = 0;
    *reinterpret_cast<volatile uint32_t *>(&iWidth_) = 0;
    *reinterpret_cast<volatile uint32_t *>(&iHeight_) = 0;
    *reinterpret_cast<volatile uint32_t *>(&iLeftOffset_) = 0;
    *reinterpret_cast<volatile uint32_t *>(&iTopOffset_) = 0;
    // THE MEMBER, kept volatile: 0x08 is cTransparentIndex_, and the
    // volatile is what holds this store between the two `ordered[]` writes
    // either side of it rather than letting VC6 sink it.
    *reinterpret_cast<volatile uint8_t *>(&cTransparentIndex_) = 9;
    *reinterpret_cast<volatile uint32_t *>(&fObj1Exists_) = 0;
}


/*
Purpose: Release a sprite's allocations, discount its pixel memory, and clear
         every field except the type byte.
// ORIGINAL: 0x005E3820 ?close@Sprite@@QAEXXZ 0x005E3820-0x005E3884 SEMANTIC
// TRIED: byte-exactness, on ONE REGISTER NAME. 35 of 38 instructions
//   agree and similarity is 1.000 under every /O2 flag set; the whole
//   divergence is that VC6 holds the loaded `*SpriteMemoryUsed` in ecx where
//   the image holds it in edx (`mov edx, [0x9b6618]` / `sub edx, eax` /
//   `mov [0x9b6618], edx`). Everything else, including the image's second
//   redundant `cmp [esi+4]` before the free, already reproduces.
//   Ten spellings measured, all 35 of 38: `-=` compound assignment, the
//   unsigned-cast form committed here, `a - b` written out, a local for the
//   product, a local for the loaded value, a local `int *const` alias,
//   `SpriteMemoryUsed[0]`, the multiplicands swapped, and splitting the
//   chained `fObj1Exists_ == 0 && pcBits_ != 0` guard into two nested `if`s.
//   /O1 is worse (27 of 38); /Ob0 and /Oi- change nothing.
// size      100 bytes
// prototype void (__thiscall ?close@Sprite@@QAEXXZ)(Sprite* this)
// callers   111   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00644EF2
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
void Sprite::close() {
    // REAL MEMBERS, NOT A `volatile uint32_t *` ALIAS. Reading the object
    // through a volatile handle blocks VC6's memory-operand folding: the image
    // emits `cmp dword ptr [esi+0x28], edi` and `imul eax, dword ptr [esi+0x10]`
    // where an aliased read forces `mov` into a register first, which was four
    // extra instructions across this body.
    //
    // The pixel buffer is only owned, accounted, and released while the
    // borrowed flag at 0x28 is clear.
    if (fObj1Exists_ == 0 && pcBits_ != 0) {
        *SpriteMemoryUsed = static_cast<int>(
            static_cast<uint32_t>(*SpriteMemoryUsed)
            - static_cast<uint32_t>(iSpriteHeight_)
                  * static_cast<uint32_t>(iSpriteWidth_));
        // A SECOND TEST OF THE SAME POINTER, which the image really does: it
        // re-tests `[esi+4]` at 0x005E3845 before the free, even though the
        // outer condition already proved it non-zero, and the `je` target is
        // the zero-store - so that store sits OUTSIDE the inner guard.
        if (pcBits_ != 0) {
            free(reinterpret_cast<void *>(pcBits_));
        }
        pcBits_ = 0;
    }
    if (ppszFileName_ != 0) {
        free(reinterpret_cast<void *>(ppszFileName_));
        ppszFileName_ = 0;
    }
    // The type byte at 0x08 is deliberately preserved.
    iSpriteWidth2_ = 0;
    iSpriteWidth_ = 0;
    iSpriteHeight_ = 0;
    iWidth_ = 0;
    iHeight_ = 0;
    iLeftOffset_ = 0;
    iTopOffset_ = 0;
    fObj1Exists_ = 0;
}


// OBJECTS, NOT POINTERS: the image addresses 0x00696D18 and 0x00696D1C
// directly - `mov esi, dword ptr [0x696d18]` - where a pointer variable
// loads the pointer first.
int SpriteDrawOriginX = 1;  // 0x00696D18
int SpriteDrawOriginY = 1;  // 0x00696D1C

/*
Purpose: Draw the sprite with a temporarily substituted draw origin.
// ORIGINAL: 0x005E4B4A ?draw@Sprite@@QAEHPAUBuffer@@HHHHH@Z 0x005E4B4A-0x005E4B95 BYTE_EXACT
// symbol    ?draw@Sprite@@QAEHPAVBuffer@@HHHHH@Z
// size      75 bytes
// prototype int (__thiscall ?draw@Sprite@@QAEHPAUBuffer@@HHHHH@Z)(Sprite* this, Buffer*, int, int, int, int, int)
// callers   48   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005E4B9A
Return Value: Whatever the four-argument overload returns
Status: Complete with a temporary dependency on the four-argument overload
*/
int Sprite::draw(Buffer *buffer, int a, int b, int c, int x, int y) {
    // The origin is swapped for the duration of the call and restored in the
    // reverse order the legacy body uses.
    const int saved_x = SpriteDrawOriginX;
    const int saved_y = SpriteDrawOriginY;
    SpriteDrawOriginX = x;
    SpriteDrawOriginY = y;
    const int result = draw(buffer, a, b, c);
    SpriteDrawOriginY = saved_y;
    SpriteDrawOriginX = saved_x;
    return result;
}


/*
Purpose: Legacy stub; the original body returns 0 without reading its
         arguments.
// ORIGINAL: 0x005E4ADA ?UNK1@Sprite@@QAEHHHHHHHH@Z 0x005E4ADA-0x005E4ADF BYTE_EXACT
// size      5 bytes
// prototype int (__thiscall ?UNK1@Sprite@@QAEHHHHHHHH@Z)(Sprite* this, int, int, int, int, int, int, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Status: Complete
*/
int Sprite::UNK1(int, int, int, int, int, int, int) {
    return 0;
}


/*
Purpose: Legacy stub; the original body returns 0 without reading its
         arguments.
// ORIGINAL: 0x005E4AEA ?UNK2@Sprite@@QAEHHHHHH@Z 0x005E4AEA-0x005E4AEF BYTE_EXACT
// size      5 bytes
// prototype int (__thiscall ?UNK2@Sprite@@QAEHHHHHH@Z)(Sprite* this, int, int, int, int, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Status: Complete
*/
int Sprite::UNK2(int, int, int, int, int) {
    return 0;
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x005EAAFF ?UNK3@Sprite@@QAEXHH@Z 0x005EAAFF-0x005EAB02 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?UNK3@Sprite@@QAEXHH@Z)(Sprite* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Sprite::UNK3(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x005EAD8F ?UNK4@Sprite@@QAEXHH@Z 0x005EAD8F-0x005EAD92 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?UNK4@Sprite@@QAEXHH@Z)(Sprite* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Sprite::UNK4(int, int) {
}


/*
// ORIGINAL: 0x0063CE20 sub_63ce20 0x0063CE20-0x0063CEE7;0x00663998-0x006639AD
// symbol    ?sub_63ce20@@YAHXZ
// TRIED: the original wraps the local Buffer in an fs:[0] SEH frame; no swept flag set reproduces that prologue/epilogue or the secondary 0x663998 span, so the SEH shape stays unmatched.
// size      220 bytes
// prototype
// callers   1   call targets   5
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D7210 0x005D7410 0x005D7DE0 0x005E39A0 0x005EAF3F
//
// Promoted 2026-08-15 from src/unrecovered/0063ce20.cpp to retire its
// pending_bodies forwarder. Loads the 16x16 sprite at 0x9BEAE8 from a PCX, or
// creates it blank if the PCX is missing.
Status: Complete
*/
// NAMED FROM THE IMAGE AND FROM THIS BODY'S OWN COMMENT: 0x00698CAC holds
// the string 'jackal.pcx' (image_data --string), and 0x009BEAE8 is the
// 16x16 sprite this loads into - which guarded_teardowns.h ALREADY declares
// as TeardownObject009BEAE8. Two bindings at one address is the shape
// duplicate_globals flags, so this file uses the existing declaration
// rather than minting a second name for the same storage.
static const char *const JackalPcxPath = (const char *)0x00698CAC;

Sprite g_BLANK_SPRITE;  // 0x009BEAE8 - the create_blank scratch sprite

int __cdecl sub_63ce20() {
    Buffer buf;
    int result;
    if (buf.load_pcx(JackalPcxPath, 0, 10, 0xec) != 0) {
        result = g_BLANK_SPRITE.create_blank(0x10, 0x10, 9);
    } else {
        result = g_BLANK_SPRITE.extract(&buf, 9, 1, 0x12, 0x10, 0x10, 0);
    }
    return result;
}

// THE SHEET HEAP, completed here because the class has no other home in the
// tree: 0x10 bytes, four uint32 members, the IDB's layout (it used to live in
// hypothesis_layouts.h, whose line ceiling has no room for a method). The
// blocks it hands out are Heap objects - `get_mem` walks the block table and
// takes the first block whose free tail fits.
struct TexHeap {
    uint32_t basePtr_;     // 0x0
    uint32_t baseSize_;    // 0x4
    uint32_t currentPtr_;  // 0x8
    uint32_t count_;       // 0xC
    int get_mem(int size);
};

/*
Purpose: Take `size` bytes of sheet memory: walk the heap's block table and
         allocate from the first block whose free tail is large enough.
// ORIGINAL: 0x006353C0 ?get_mem@TexHeap@@QAEHH@Z 0x006353C0-0x00635402 FILE
// size      66 bytes
// prototype int (__thiscall ?get_mem@TexHeap@@QAEHH@Z)(#120* this, int)
// callers   1   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005D4680
Status: Promoted 2026-08-29 from src/recovered/units/006353c0.cpp, which
measured MNEMONIC_ONLY against the opaque scaffold and carried no claim. The
`+0x10` read is the block Heap's `free_size_`; the allocation itself is
`Heap::get` (0x005D4680). `Sprite::extract` below is the one caller, and it
only reaches here when a caller passed a heap at all.
*/
int TexHeap::get_mem(int size) {
    if (basePtr_ != 0) {
        int index = 0;
        int count = static_cast<int>(count_) + 1;
        if (count > 0) {
            Heap **blocks = reinterpret_cast<Heap **>(currentPtr_);
            while (index < count) {
                if (static_cast<int>(blocks[index]->free_size_) >= size) {
                    return reinterpret_cast<int>(blocks[index]->get(size));
                }
                ++index;
            }
        }
    }
    return 0;
}

/*
Purpose: Cut the width x height rectangle at (left, top) out of a locked
         buffer's pixels into this sprite, trimming the fully transparent
         border on all four sides, then allocate the trimmed pixel block from
         the sheet heap or the CRT and copy it across.
// ORIGINAL: 0x005E39A0 ?extract@Sprite@@QAEHPAUBuffer@@HHHHHPAUTexHeap@@@Z 0x005E39A0-0x005E3DF1 FILE
// symbol    ?extract@Sprite@@QAEHPAVBuffer@@HHHHHPAUTexHeap@@@Z
// The image spells the buffer pointer `PAU` - Buffer as a STRUCT - where this
// tree declares `class Buffer`, and MSVC decorates the first-seen class-key.
// `Sprite::draw` carries the same fact for the same reason.
// TRIED: direct field-name transcription of Sprite::extract mirroring Ghidra's param_1[N]; frame-pointer/register allocation shape diverges at insn #0
// TRIED: first guard spelled `buffer == 0 || buffer->get_data() == 0`, as two
//   nested `if (x != 0)` blocks, and as one `x != 0 && y != 0` chain - all
//   three compile to the same 44 of 397 agreeing instructions (0.392 similar).
//   Flag sets scored by measure --all-flags: /c /O2 /Ob0 /Gy /GR- /GX is the
//   best at 44/397; every non-/Ob0 set is 7/397 or worse because the in-class
//   `get_data()` then inlines and its E8 disappears. /Ob0 is load-bearing.
// TRIED: the receiver-spill fingerprint, not reached. The image never
//   enregisters `buffer`: it loads it fresh into volatile ecx at each of its
//   ten uses (`mov ecx, [esp+0x1c]`) and spends the callee-saved registers on
//   this=esi, width=ebx, height=edi, zero=ebp - `xor ebp,ebp` sits above the
//   first null test. Every spelling tried instead enregisters `buffer` into
//   ebx at the function's first statement (scheduled before the remaining
//   callee-saved pushes) and shifts every other role one register over
//   (this=ebp, width=esi, zero=edi), renaming every field access and call
//   receiver in the body at once. Same wall as the Palette receiver-spill
//   bodies: guard reordering and condition shape do not move it.
// size      1105 bytes
// prototype int (__thiscall ?extract@Sprite@@QAEHPAUBuffer@@HHHHHPAUTexHeap@@@Z)(Sprite* this, Buffer*, int, int, int, int, int, #120*)
// callers   29   call targets   8
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4510 0x005E3373 0x005E33F3 0x005E34A3 0x005FD2B0 0x006353C0 0x00644EF2 0x00645930
Return Value: 0 extracted; 3 bad arguments or empty rectangle; 4 the CRT
              allocation failed; 7 the buffer would not lock; 9 the rectangle
              is entirely transparent
Status: Semantics transcribed from the image

The four scans walk the requested rectangle four ways: top down for the first
row with a visible pixel, bottom up for the last, and across the kept rows for
the first and last visible columns. `iTopOffset_` counts the rows the top scan
spent; `iSpriteHeight_` opens as the untrimmed height and each bottom-scan row
spends one; `iLeftOffset_` and the right-trim local mirror them on the columns.
The bounds come off `dib_.bmiHeader`, whose `biHeight` is NEGATIVE for a
top-down DIB - so the bottom bound really is `-biHeight`, and the
two-coordinate `get_data` rejects the same way.
*/
int Sprite::extract(Buffer *buffer, int transparent, int left, int top,
                    int width, int height, TexHeap *heap) {
    int stride;
    int col;
    int rows;
    int right_trim;
    int row_index;
    const uint8_t *pixels;

    // ONE LOAD, ONE EXIT: the image tests `buffer` against zero with the same
    // ecx load that then serves as the call's receiver, and both tests share
    // the `return 3` tail with the four guards further down. NESTED, not `||`:
    // the nesting is what ends `buffer`'s live range at the test.
    if (buffer != 0 && buffer->get_data() != 0) {
        // Free the previous contents exactly as Sprite::close opens: the pixel
        // block is only owned, accounted, and released while the borrowed flag is
        // clear, and the image re-tests `pcBits_` before the free.
        if (fObj1Exists_ == 0 && pcBits_ != 0) {
            *SpriteMemoryUsed = static_cast<int>(
                static_cast<uint32_t>(*SpriteMemoryUsed)
                - static_cast<uint32_t>(iSpriteHeight_)
                      * static_cast<uint32_t>(iSpriteWidth_));
            if (pcBits_ != 0) {
                free(reinterpret_cast<void *>(pcBits_));
            }
            pcBits_ = 0;
        }
        if (ppszFileName_ != 0) {
            free(reinterpret_cast<void *>(ppszFileName_));
            ppszFileName_ = 0;
        }
        iSpriteWidth2_ = 0;
        iSpriteWidth_ = 0;
        iSpriteHeight_ = 0;
        iWidth_ = 0;
        iHeight_ = 0;
        iLeftOffset_ = 0;
        iTopOffset_ = 0;
        fObj1Exists_ = 0;

        if (width == 0) {
            return 3;
        }
        if (height == 0) {
            return 3;
        }
        if (left + width > static_cast<int>(buffer->dib_.bmiHeader.biWidth)) {
            return 3;
        }
        if (top + height > -static_cast<int>(buffer->dib_.bmiHeader.biHeight)) {
            return 3;
        }

        transparent &= 0xff;
        cTransparentIndex_ = static_cast<char>(transparent);
        iWidth_ = width;
        iHeight_ = height;
        pixels = reinterpret_cast<const uint8_t *>(buffer->get_data(left, top));
        if (pixels == 0) {
            buffer->free_data(1);
            return 7;
        }
        stride = static_cast<int>(buffer->stride_);
        iTopOffset_ = 0;
        // TOP TRIM: walk down until a row carries a visible pixel.
        for (row_index = 0; row_index < height; row_index++) {
            for (col = 0; col < width; col++) {
                if (pixels[col] != transparent) {
                    goto top_done;
                }
            }
            pixels += stride;
            iTopOffset_++;
        }
    top_done:
        do_sound();
        if (iTopOffset_ == height) {
            buffer->free_data(2);
            return 9;
        }
        buffer->free_data(1);
        // BOTTOM TRIM: walk up from the last row, right to left, until a row
        // carries a visible pixel. The pointer arrives pre-decremented - the
        // image indexes `row[col]` off `pixels - 1`, not `pixels[col - 1]`.
        pixels = reinterpret_cast<const uint8_t *>(
            buffer->get_data(left, top + height - 1));
        iSpriteHeight_ = height - iTopOffset_;
        for (row_index = 0; row_index < height; row_index++) {
            const uint8_t *from_right = pixels - 1;
            for (col = width; col > 0; col--) {
                if (from_right[col] != transparent) {
                    goto bottom_done;
                }
            }
            pixels -= stride;
            iSpriteHeight_--;
        }
    bottom_done:
        if (iSpriteHeight_ == 0) {
            buffer->free_data(2);
            return 9;
        }
        do_sound();
        buffer->free_data(1);
        // LEFT TRIM: on every kept row, find the first visible column.
        pixels = reinterpret_cast<const uint8_t *>(
            buffer->get_data(left, top + iTopOffset_));
        iLeftOffset_ = width;
        for (rows = iSpriteHeight_; rows > 0; rows--) {
            for (col = 0; col < width; col++) {
                if (pixels[col] != transparent) {
                    break;
                }
            }
            if (col < iLeftOffset_) {
                iLeftOffset_ = col;
            }
            pixels += stride;
        }
        do_sound();
        buffer->free_data(1);
        // RIGHT TRIM: on every kept row, count the transparent pixels at the end.
        // THIS scan indexes `pixels[col - 1]` - the image folds the -1 into the
        // addressing here, unlike the bottom trim above.
        pixels = reinterpret_cast<const uint8_t *>(
            buffer->get_data(left, top + iTopOffset_));
        right_trim = width;
        for (rows = iSpriteHeight_; rows > 0; rows--) {
            int counted = 0;
            for (col = width; col > 0; col--) {
                if (pixels[col - 1] != transparent) {
                    break;
                }
                counted++;
            }
            if (counted < right_trim) {
                right_trim = counted;
            }
            pixels += stride;
        }
        buffer->free_data(1);
        iSpriteWidth_ = width - iLeftOffset_ - right_trim;
        if (iSpriteWidth_ == 0) {
            buffer->free_data(1);
            return 9;
        }
        // The sheet heap is tried first and only when the caller named one. The
        // pixels then draw through the heap and `fObj1Exists_` says so; anything
        // else comes off the CRT as plain owned memory.
        pcBits_ = 0;
        if (heap != 0) {
            pcBits_ = heap->get_mem(iSpriteHeight_ * iSpriteWidth_);
            if (pcBits_ != 0) {
                fObj1Exists_ = 1;
            }
        }
        do_sound();
        if (pcBits_ == 0) {
            pcBits_ = reinterpret_cast<int>(mem_get(iSpriteWidth_ * iSpriteHeight_));
            if (pcBits_ == 0) {
                if (ppszFileName_ != 0) {
                    free(reinterpret_cast<void *>(ppszFileName_));
                    ppszFileName_ = 0;
                }
                iSpriteWidth2_ = 0;
                iSpriteWidth_ = 0;
                iSpriteHeight_ = 0;
                iWidth_ = 0;
                iHeight_ = 0;
                iLeftOffset_ = 0;
                iTopOffset_ = 0;
                fObj1Exists_ = 0;
                buffer->free_data(1);
                return 4;
            }
            fObj1Exists_ = 0;
        }
        *SpriteMemoryUsed += iSpriteWidth_ * iSpriteHeight_;
        pixels = reinterpret_cast<const uint8_t *>(
            buffer->get_data(left + iLeftOffset_, top + iTopOffset_));
        uint8_t *out = reinterpret_cast<uint8_t *>(pcBits_);
        do_sound();
        for (rows = iSpriteHeight_; rows > 0; rows--) {
            memcpy(out, pixels, iSpriteWidth_);
            out += iSpriteWidth_;
            pixels += stride;
        }
        iSpriteWidth2_ = iSpriteWidth_;
        buffer->free_data(2);
        return 0;

    }
    return 3;
}




// ---------------------------------------------------------------------------
// DEFINED IN THE HEADER, CLAIMED HERE.
//
// These pieces are written in-class so the image's own inlining reproduces -
// a constructor or destructor the compiler is expected to fold into its
// caller. A marker cannot live beside them: `decomp.reader` globs `*.cpp`
// and `*.c`, and a comparison compiles a TRANSLATION UNIT, so a marker in a
// header could be neither read nor measured. VC6 emits each of them into
// this unit's object as its own COMDAT anyway, which is what the comparison
// pulls out, and the `body` fact says where to go to edit one.
//
// The ratchet still covers the header: this unit includes it, so breaking an
// in-class body here fails the claim below. Measured, not assumed.
// ---------------------------------------------------------------------------

/*
// ORIGINAL: 0x00406850 ??1Sprite@@QAE@XZ 0x00406850-0x00406855 BYTE_EXACT
// body      src/sprite.h
// size      5 bytes
// prototype 
// callers   1   call targets   0
// kind      thunk
// flags     hidden;thunk;sp_ready;purged_ok
// calls     (none)
*/

/*
Purpose: Turn the sprite into a one-pixel placeholder of the given extent.
// ORIGINAL: 0x005EAF3F ?create_blank@Sprite@@QAEHHHH@Z 0x005EAF3F-0x005EAF86
// LEVER: the two `1` stores go BEFORE the two zero stores, and the depth
//   store goes BEFORE the height store. Field order alone moved this from
//   16 of 25 to 23 of 25 (similarity 1.000, every instruction present):
//   putting the zeros first schedules `mov eax, 1` after the `xor eax, eax`
//   and costs the shared `mov ecx, 1` the image materialises up front.
// TRIED: the `const int one = 1;` local the promoted transcription used.
//   It is not what shares the constant - a plain literal `1` at all three
//   uses scores the same, and `const size_t one` / `mem_get(iSpriteWidth_)`
//   score no better either (all 16 of 25 at the old field order).
// TRIED: reaching 25 of 25. The last two stores are `[esi+0x1c]` (height)
//   and `[esi+8]` (depth) and the image emits them in that order while
//   loading height EARLY into edx and depth LATE into al. VC6 here ties the
//   two together: stores follow source order and the loads come out in the
//   REVERSE of it, so `height; depth` buys the store order and loses the
//   registers (21 of 25) while `depth; height` buys the registers and loses
//   the store order (23 of 25). Nine spellings measured - chained zeros,
//   swapped 1-stores, swapped width stores, swapped zero stores, a raw
//   `char *` store, temps for either parameter - and none decouples them.
// size      71 bytes
// prototype int (__thiscall ?create_blank@Sprite@@QAEHHHH@Z)(Sprite* this, int, int, int)
// kind      game
// calls     0x005D4510
Return Value: No errors (0); allocation failed (4)
Status: Semantics transcribed from the image

PROMOTED FROM src/recovered/units/005eaf3f.cpp, which wrote every field as
`iWidth_`.

ONE BYTE OF PIXELS. The sheet is 1x1 and the extent is whatever the caller
asked for, so this is a placeholder that draws a single colour - and
`iSpriteWidth2_` takes the width while `iSpriteWidth_` takes 1, which is the
pair the drawing code reads as "sheet stride" and "frame width".
*/
int Sprite::create_blank(int width, int height, int depth) {
    iWidth_ = width;
    iSpriteWidth2_ = width;
    iSpriteWidth_ = 1;
    iSpriteHeight_ = 1;
    iLeftOffset_ = 0;
    iTopOffset_ = 0;
    cTransparentIndex_ = static_cast<char>(depth);
    iHeight_ = height;
    pcBits_ = reinterpret_cast<int>(mem_get(1));
    return pcBits_ ? 0 : 4;
}

// ===== MANAGED GLOBALS - real objects, homed to their domain =====
// In the shipped image these live at fixed data addresses and are
// constructed before WinMain by the CRT's dynamic-initializer walk
// (winedbg-confirmed: walker return 0x00644EEB, table at 0x682568..).
// Here the same recovered constructors run through this build's own
// startup, and the matching destructors close them at exit. Addresses of
// the ones documented individually live beside their definitions.
Sprite g_UNUSED_SPRITE_VAR02;  // 0x006A7130
Sprite g_UNUSED_SPRITE_VAR11;  // 0x006A72E0
Sprite g_UNUSED_SPRITE_VAR06;  // 0x006A71F0
Sprite g_UNUSED_SPRITE_VAR09;  // 0x006A7280
Sprite g_UNUSED_SPRITE_VAR21;  // 0x006A74C0
Sprite g_UNUSED_SPRITE_VAR05;  // 0x006A71C0
Sprite g_UNUSED_SPRITE_VAR08;  // 0x006A7250
Sprite g_UNUSED_SPRITE_VAR04;  // 0x006A7190
Sprite g_UNUSED_SPRITE_VAR01;  // 0x006A7100
Sprite g_UNUSED_SPRITE_VAR18;  // 0x006A7430
Sprite g_UNUSED_SPRITE_VAR03;  // 0x006A7160
Sprite g_UNUSED_SPRITE_VAR20;  // 0x006A7490
Sprite g_UNUSED_SPRITE_VAR16;  // 0x006A73D0
Sprite g_UNUSED_SPRITE_VAR14;  // 0x006A7370
Sprite g_UNUSED_SPRITE_VAR22;  // 0x006A74F0
Sprite g_UNUSED_SPRITE_VAR10;  // 0x006A72B0
Sprite g_UNUSED_SPRITE_VAR15;  // 0x006A73A0
Sprite g_UNUSED_SPRITE_VAR13;  // 0x006A7340
Sprite g_UNUSED_SPRITE_VAR17;  // 0x006A7400
Sprite g_UNUSED_SPRITE_VAR19;  // 0x006A7460
Sprite g_UNUSED_SPRITE_VAR12;  // 0x006A7310
Sprite g_UNUSED_SPRITE_VAR07;  // 0x006A7220
Sprite g_IFACE_CLOSE_X_SPRITES[3];  // 0x007794D8, 0x2c stride
Sprite g_IFACE_BOX_SPRITES1[51];  // 0x007AC290, 0x2c stride
Sprite g_IFACE_BOX_SPRITES2[51];  // 0x0076DC98, 0x2c stride
Sprite g_IFACE_BOX_SPRITES3[51];  // 0x0078AE58, 0x2c stride
Sprite g_IFACE_BOX_SPRITES4[51];  // 0x0078CC60, 0x2c stride
Sprite g_IFACE_BOX_SPRITES5[51];  // 0x00779CB0, 0x2c stride
Sprite g_IFACE_BOX_SPRITES6[51];  // 0x007A6978, 0x2c stride
Sprite g_IFACE_BOX_SPRITES7[51];  // 0x0078E0A0, 0x2c stride
Sprite g_IFACE_BOX_SPRITES8[51];  // 0x0075B950, 0x2c stride
Sprite g_IFACE_STD_POPUPS_TOP_LEFT_SPRITE;  // 0x007921E8
Sprite g_IFACE_STD_POPUPS_TOP_RIGHT_SPRITE;  // 0x00779478
Sprite g_IFACE_STD_POPUPS_BOT_LEFT_SPRITE;  // 0x007AC1C0
Sprite g_IFACE_STD_POPUPS_BOT_RIGHT_SPRITE;  // 0x007AB400
Sprite g_IFACE_STD_POPUPS_TOP_MID_SPRITE;  // 0x007793F0
Sprite g_IFACE_STD_POPUPS_BOT_MID_SPRITE;  // 0x007871D0
Sprite g_IFACE_STD_POPUPS_MID_LEFT_SPRITE;  // 0x007ACFB0
Sprite g_IFACE_STD_POPUPS_MID_RIGHT_SPRITE;  // 0x0075B058
Sprite g_TER1_WHITE_ORG_YEL_TILE_SPRITES[6];  // 0x00791C58, 0x2c stride
Sprite g_TER1_BOTTOM_LEFT_TILE_SPRITES[9];  // 0x007A99A0, 0x2c stride
Sprite g_TER1_UNUSED_SPRITES2[2];  // 0x00779420, 0x2c stride
Sprite g_TER1_MINE_SPRITE;  // 0x00776A50
Sprite g_TER1_SOLAR_COLLECTOR_SPRITE;  // 0x007991F8
Sprite g_TER1_TIDAL_HARNESS_SPRITE;  // 0x0078A5B0
Sprite g_TER1_MINING_PLATFORM_SPRITE;  // 0x00789B68
Sprite g_TER1_TUT_BLANK_SPRITE;  // 0x00798C40
Sprite g_TER1_KELP_FARM_SPRITE;  // 0x007A6860
Sprite g_TER1_CONDENSER_SPRITE;  // 0x00779390
Sprite g_TER1_ECHELON_MIRROR_SPRITE;  // 0x007ABF60
Sprite g_TER1_BOREHOLE_SPRITE;  // 0x0078DD20
Sprite g_TER1_BOREHOLE_CLUSTER_SPRITE;  // 0x007ACB88
Sprite g_TER1_MANIFOLD_NEXUS_SPRITES[6];  // 0x007AC098, 0x2c stride
Sprite g_TER1WRECK_UNITY_WRECKAGE_SPRITES[15];  // 0x00776758, 0x2c stride
Sprite g_TER1WRECK_UNITY_WRECKAGE_ALT_SPRITES[4];  // 0x00799658, 0x2c stride
Sprite g_FOSSIL_FIELD_RIDGE_SPRITES[6];  // 0x007ACD70, 0x2c stride
Sprite g_TER1_UNUSED_SPRITES1[5];  // 0x007792B0, 0x2c stride
Sprite g_TER1_FARM_SPRITES[5];  // 0x00791FC8, 0x2c stride
Sprite g_TER1_SOIL_ENRICHER_SPRITES[5];  // 0x0079A710, 0x2c stride
Sprite g_TER1_SEA_LAND_RESOURCE_SPRITES[12];  // 0x0075B230, 0x2c stride
Sprite g_TER1_LANDMARK_RESOURCE_SPRITES[6];  // 0x0078A650, 0x2c stride
Sprite g_GLOW_SPRITES[2];  // 0x00779770, 0x2c stride
Sprite g_TER1_UNITY_POD_SPRITES[6];  // 0x0077AFF8, 0x2c stride
Sprite g_TER1_MONOLITH_SPRITE;  // 0x0075B098
Sprite g_TER1_BUNKER_SPRITE;  // 0x007AD010
Sprite g_TER1_AIRBASE_SPRITE;  // 0x007ACC60
Sprite g_TER1_SENSOR_ARRAY_SPRITE;  // 0x007A6830
Sprite g_RAINFALL_DOUBLE_TILE_SPRITES[2];  // 0x007ACBB8, 0x2c stride
Sprite g_VEH_SPRITES[152];  // 0x007777A0, 0x2c stride
Sprite g_FLAGS_VEH_SPRITES[112];  // 0x0078B778, 0x2c stride
Sprite g_ICONS_GENERAL_SPRITES[16];  // 0x0075B450, 0x2c stride
Sprite g_RESOURCE_ICON_SPRITES[32];  // 0x007A72A0, 0x2c stride
Sprite g_CITIZEN_LG_CURSOR_SPRITES[8];  // 0x0078CAD0, 0x2c stride
Sprite g_SPECIALIST_LG_CURSOR_SPRITES[7];  // 0x007765C0, 0x2c stride
Sprite g_CITIZEN_SM_CURSOR_SPRITES[8];  // 0x00776420, 0x2c stride
Sprite g_SPECIALIST_SM_CURSOR_SPRITES[7];  // 0x00779B78, 0x2c stride
Sprite g_AL_CITIZEN_LG_CURSOR_SPRITES[4];  // 0x007AD1B0, 0x2c stride
Sprite g_AL_SPECIALIST_LG_CURSOR_SPRITES[7];  // 0x00798CD0, 0x2c stride
Sprite g_AL_CITIZEN_SM_CURSOR_SPRITES[4];  // 0x0075B180, 0x2c stride
Sprite g_AL_SPECIALIST_SM_CURSOR_SPRITES[7];  // 0x0075B710, 0x2c stride
Sprite g_RED_ALIEN_HEAD_ICON_SPRITE;  // 0x00791F98
Sprite g_SILVER_MENU_ICON_SPRITES[4];  // 0x0075AE20, 0x2c stride
Sprite g_SILVER_CHECKBOX_ICON_SPRITES[2];  // 0x0075AF68, 0x2c stride
Sprite g_RED_MALE_HEAD_ICON_SPRITE;  // 0x0075AD88
Sprite g_NULL_RESOURCE_ICON_SPRITE;  // 0x0078A520
Sprite g_PEACE_SIGN_SPRITES[2];  // 0x0075AF10, 0x2c stride
Sprite g_ICON_TILE_SQUARE_SPRITE;  // 0x00776728
Sprite g_XI_BOOM_VEH_SPRITES[144];  // 0x007A9B30, 0x2c stride
Sprite g_XF_BOOM_VEH_SPRITES[64];  // 0x007AB460, 0x2c stride
Sprite g_BATTLE_MIND_WORM_SPRITE;  // 0x00779200
Sprite g_BATTLE_ISLE_DEEP_SPRITE;  // 0x0076DC30
Sprite g_BATTLE_LOCUSTS_CHIRON_SPRITE;  // 0x0078A4A0
Sprite g_BATTLE_FUNGAL_TOWER_SPRITE;  // 0x00787E70
Sprite g_BATTLE_SPORE_LAUNCHER_SPRITE;  // 0x007AC000
Sprite g_BATTLE_SEALURK_SPRITE;  // 0x00779570
Sprite g_TECH_ICON_SPRITES[89];  // 0x00759E28, 0x2c stride
Sprite g_FACILITY_ICON_SPRITES[70];  // 0x00787200, 0x2c stride
Sprite g_SECRET_PROJECT_ICON_SPRITES[64];  // 0x00759320, 0x2c stride
Sprite g_IFACE_MP_COMBO_ARROW_SPRITES[3];  // 0x00788038, 0x2c stride
Sprite g_SCROLL_BAR_ARROW_ICON_SPRITES[12];  // 0x00791D88, 0x2c stride
Sprite g_SCROLL_BAR_SMALL_ARROW_ICON_SPRITES[12];  // 0x0078DE30, 0x2c stride
Sprite g_SCROLL_BAR_FILLER_ICON_SPRITES;  // 0x0079A5E8
Sprite g_SCROLL_BAR_SMALL_FILLER_ICON_SPRITE;  // 0x007796B0
Sprite g_IFACE_LOCK_SPRITES[2];  // 0x007AC040, 0x2c stride
Sprite g_UNUSED_SPRITES_VAR01[8];  // 0x007A7940, 0x2c stride
Sprite g_IFACE_GENERAL_WINDOWS_TOP_LEFT_SPRITE;  // 0x0078DD50
Sprite g_IFACE_GENERAL_WINDOWS_TOP_RIGHT_SPRITE;  // 0x00789BC8
Sprite g_IFACE_GENERAL_WINDOWS_BOT_LEFT_SPRITE;  // 0x00776580
Sprite g_IFACE_GENERAL_WINDOWS_BOT_RIGHT_SPRITE;  // 0x007ABFD0
Sprite g_IFACE_GENERAL_WINDOWS_MID_LEFT_SPRITE;  // 0x00787F88
Sprite g_IFACE_GENERAL_WINDOWS_MID_RIGHT_SPRITE;  // 0x0078A2D0
Sprite g_IFACE_GENERAL_WINDOWS_TOP_MID_SPRITE;  // 0x0078E070
Sprite g_IFACE_GENERAL_WINDOWS_BOT_MID_SPRITE;  // 0x0078A550
Sprite g_IFACE_GENERAL_WINDOWS_NONCAP_MID_SPRITE;  // 0x00789BF8
Sprite g_IFACE_GENERAL_WINDOWS_NONCAP_LEFT_SPRITE;  // 0x0075B150
Sprite g_IFACE_GENERAL_WINDOWS_NONCAP_RIGHT_SPRITE;  // 0x007AD180
Sprite g_UNUSED_SPRITE_VAR23;  // 0x007880D0
Sprite g_UNUSED_SPRITE_VAR24;  // 0x0079A5B8
Sprite g_UNUSED_SPRITE_VAR25;  // 0x007797C8
Sprite g_UNUSED_SPRITE_VAR26;  // 0x00792638
Sprite g_UNUSED_SPRITE_VAR27;  // 0x0079A650
Sprite g_UNUSED_SPRITE_VAR28;  // 0x00799708
Sprite g_UNUSED_SPRITE_VAR29;  // 0x007ACFE0
Sprite g_UNUSED_SPRITE_VAR30;  // 0x007A7910
Sprite g_UNUSED_SPRITE_VAR31;  // 0x00779240
Sprite g_UNUSED_SPRITE_VAR32;  // 0x007ACD00
Sprite g_UNUSED_SPRITE_VAR33;  // 0x00776A20
Sprite g_UNUSED_SPRITE_VAR34;  // 0x007769F0
Sprite g_UNUSED_SPRITE_VAR35;  // 0x007796E0
Sprite g_UNUSED_SPRITE_VAR36;  // 0x007AC1F0
Sprite g_UNUSED_SPRITE_VAR37;  // 0x00789A60
Sprite g_UNUSED_SPRITE_VAR38;  // 0x0078CC30
Sprite g_UNUSED_SPRITE_VAR39;  // 0x00787F58
Sprite g_UNUSED_SPRITE_VAR40;  // 0x00798C00
Sprite g_UNUSED_SPRITE_VAR41;  // 0x0078A580
Sprite g_UNUSED_SPRITE_VAR42;  // 0x007AD0F0
Sprite g_UNUSED_SPRITE_VAR43;  // 0x007AD080
Sprite g_UNUSED_SPRITE_VAR44;  // 0x00792608
Sprite g_UNUSED_SPRITE_VAR45;  // 0x00787EA0
Sprite g_UNUSED_SPRITE_VAR46;  // 0x007ACCD0
Sprite g_IFACE_TECH_TREE_ARROW_SPRITES[3];  // 0x0075AFD0, 0x2c stride
Sprite g_UNUSED_SPRITE_VAR47;  // 0x00779740
Sprite g_UNUSED_SPRITE_VAR48;  // 0x0078A4D0
Sprite g_UNUSED_SPRITE_VAR49;  // 0x007AD140
Sprite g_UNUSED_SPRITE_VAR50;  // 0x007793C0
Sprite g_UNUSED_SPRITE_VAR51;  // 0x00789B98
Sprite g_UNUSED_SPRITE_VAR52;  // 0x007766F8
Sprite g_UNUSED_SPRITE_VAR53;  // 0x00798C70
Sprite g_UNUSED_SPRITE_VAR54;  // 0x0076EBA0
Sprite g_UNUSED_SPRITE_VAR55;  // 0x007592E0
Sprite g_UNUSED_SPRITE_VAR56;  // 0x007795C0
Sprite g_UNUSED_SPRITE_VAR57;  // 0x00792130
Sprite g_UNUSED_SPRITE_VAR58;  // 0x0075ADF0
Sprite g_UNUSED_SPRITE_VAR59;  // 0x0076E980
Sprite g_UNUSED_SPRITE_VAR60;  // 0x0078A310
Sprite g_UNUSED_SPRITE_VAR61;  // 0x0076DC60
Sprite g_UNUSED_SPRITE_VAR62;  // 0x00789B18
Sprite g_UNUSED_SPRITE_VAR63;  // 0x007A68C0
Sprite g_UNUSED_SPRITE_VAR64;  // 0x0077A588
Sprite g_UNUSED_SPRITE_VAR65;  // 0x00779710
Sprite g_UNUSED_SPRITE_VAR66;  // 0x0079A6C0
Sprite g_UNUSED_SPRITE_VAR67;  // 0x0078DDF0
Sprite g_UNUSED_SPRITE_VAR68;  // 0x007A7250
Sprite g_UNUSED_SPRITE_VAR69;  // 0x007AD0B0
Sprite g_UNUSED_SPRITE_VAR70;  // 0x007AD050
Sprite g_UNUSED_SPRITE_VAR71;  // 0x007A6890
Sprite g_UNUSED_SPRITE_VAR72;  // 0x007763F0
Sprite g_UNUSED_SPRITE_VAR73;  // 0x007ACD30
Sprite g_UNUSED_SPRITE_VAR74;  // 0x007ABF90
Sprite g_UNUSED_SPRITES_VAR02[3];  // 0x007A68F0, 0x2c stride
Sprite g_UNUSED_SPRITES_VAR03[3];  // 0x00789A90, 0x2c stride
Sprite g_UNUSED_SPRITES_VAR04[3];  // 0x00792160, 0x2c stride
Sprite g_UNUSED_SPRITES_VAR05[3];  // 0x0075B0C8, 0x2c stride
Sprite g_UNUSED_SPRITES_VAR06[3];  // 0x0075B8C8, 0x2c stride
Sprite g_UNUSED_SPRITES_VAR07[3];  // 0x00787ED0, 0x2c stride
Sprite g_UNUSED_SPRITES_VAR08[3];  // 0x007ACE80, 0x2c stride
Sprite g_UNUSED_SPRITES_VAR09[3];  // 0x007920A8, 0x2c stride
Sprite g_UNUSED_SPRITES_VAR10[3];  // 0x007ACF08, 0x2c stride
Sprite g_UNUSED_SPRITE_VAR75;  // 0x007794A8
Sprite g_UNUSED_SPRITE_VAR76;  // 0x00779670
Sprite g_UNUSED_SPRITE_VAR77;  // 0x007AB430
Sprite g_UNUSED_SPRITE_VAR78;  // 0x00798CA0
Sprite g_UNUSED_SPRITE_VAR79;  // 0x0078E040
Sprite g_UNUSED_SPRITE_VAR80;  // 0x007A67F0
Sprite g_UNUSED_SPRITE_VAR81;  // 0x00779618
Sprite g_UNUSED_SPRITE_VAR82;  // 0x007ACB58
Sprite g_UNUSED_SPRITE_VAR83;  // 0x007ACC20
Sprite g_BASEWIN_SPRITES[27];  // 0x0077A5C8, 0x2c stride
Sprite g_IFACE_GREEN_RIGHT_ARROW_SPRITE[1];  // 0x007F67C8, 0x2c stride
Sprite g_CURSOR_SPRITES[12];  // 0x0093AA70, 0x2c stride
// ===== MANAGED GLOBALS - real objects, homed to their domain =====
// In the shipped image these live at fixed data addresses and are
// constructed before WinMain by the CRT's dynamic-initializer walk
// (winedbg-confirmed: walker return 0x00644EEB, table at 0x682568..).
// Here the same recovered constructors run through this build's own
// startup, and the matching destructors close them at exit.
