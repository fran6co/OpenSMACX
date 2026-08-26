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
static int *const g_00698cac = (int *)0x00698CAC;
static int *const g_009beae8 = (int *)0x009BEAE8;

int __cdecl sub_63ce20() {
    Buffer buf;
    int result;
    if (buf.load_pcx(reinterpret_cast<const char *>(g_00698cac), 0, 10, 0xec) != 0) {
        result = reinterpret_cast<Sprite *>(g_009beae8)->create_blank(0x10, 0x10, 9);
    } else {
        result = reinterpret_cast<Sprite *>(g_009beae8)->extract(&buf, 9, 1, 0x12, 0x10, 0x10, 0);
    }
    return result;
}

// Sheet extraction reached by the per-control init_class bodies. Not yet
// recovered; it forwards through the seam. `create_blank` used to sit
// here beside it and is promoted below.




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
