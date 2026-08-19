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

int *SpriteMemoryUsed = reinterpret_cast<int *>(0x009B6618);
func_sprite_free *SpriteFree = (func_sprite_free *)0x00644EF2;

/*
Purpose: Initialize an empty sprite and charge its own size to the sprite
         memory total.
// ORIGINAL: 0x005E37E0 ??0Sprite@@QAE@XZ 0x005E37E0-0x005E3815
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
    volatile uint32_t *ordered = reinterpret_cast<volatile uint32_t *>(this);
    ordered[0x00 / 4] = 0;
    *SpriteMemoryUsed = static_cast<int>(
        static_cast<uint32_t>(*SpriteMemoryUsed) + sizeof(Sprite));
    ordered[0x04 / 4] = 0;
    ordered[0x0C / 4] = 0;
    ordered[0x10 / 4] = 0;
    ordered[0x14 / 4] = 0;
    ordered[0x18 / 4] = 0;
    ordered[0x1C / 4] = 0;
    ordered[0x20 / 4] = 0;
    ordered[0x24 / 4] = 0;
    *reinterpret_cast<volatile uint8_t *>(
        reinterpret_cast<uint8_t *>(this) + 0x08) = 9;
    ordered[0x28 / 4] = 0;
}

Sprite *__fastcall sprite_construct_redirect(Sprite *self, void *) {
    new (self) Sprite();
    return self;
}

/*
Purpose: Release a sprite's allocations, discount its pixel memory, and clear
         every field except the type byte.
// ORIGINAL: 0x005E3820 ?close@Sprite@@QAEXXZ 0x005E3820-0x005E3884
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
    volatile uint32_t *ordered = reinterpret_cast<volatile uint32_t *>(this);
    // The pixel buffer is only owned, accounted, and released while the
    // borrowed flag at 0x28 is clear.
    if (ordered[0x28 / 4] == 0 && ordered[0x04 / 4] != 0) {
        *SpriteMemoryUsed = static_cast<int>(
            static_cast<uint32_t>(*SpriteMemoryUsed)
            - ordered[0x14 / 4] * ordered[0x10 / 4]);
        SpriteFree(reinterpret_cast<void *>(ordered[0x04 / 4]));
        ordered[0x04 / 4] = 0;
    }
    if (ordered[0x00 / 4] != 0) {
        SpriteFree(reinterpret_cast<void *>(ordered[0x00 / 4]));
        ordered[0x00 / 4] = 0;
    }
    // The type byte at 0x08 is deliberately preserved.
    ordered[0x0C / 4] = 0;
    ordered[0x10 / 4] = 0;
    ordered[0x14 / 4] = 0;
    ordered[0x18 / 4] = 0;
    ordered[0x1C / 4] = 0;
    ordered[0x20 / 4] = 0;
    ordered[0x24 / 4] = 0;
    ordered[0x28 / 4] = 0;
}

void __fastcall sprite_close_redirect(Sprite *self, void *) {
    self->close();
}

int *SpriteDrawOriginX = reinterpret_cast<int *>(0x00696D18);
int *SpriteDrawOriginY = reinterpret_cast<int *>(0x00696D1C);
func_sprite_draw_original SpriteDrawOriginal = original_method<func_sprite_draw_original>(0x005E4B9A);

/*
Purpose: Draw the sprite with a temporarily substituted draw origin.
// ORIGINAL: 0x005E4B4A ?draw@Sprite@@QAEHPAUBuffer@@HHHHH@Z 0x005E4B4A-0x005E4B95
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
    const int saved_x = *SpriteDrawOriginX;
    const int saved_y = *SpriteDrawOriginY;
    *SpriteDrawOriginX = x;
    *SpriteDrawOriginY = y;
    const int result = (ORIGINAL(this)->*SpriteDrawOriginal)(buffer, a, b, c);
    *SpriteDrawOriginY = saved_y;
    *SpriteDrawOriginX = saved_x;
    return result;
}

int __fastcall sprite_draw_redirect(
        Sprite *self, void *, Buffer *buffer, int a, int b, int c, int x, int y) {
    return self->draw(buffer, a, b, c, x, y);
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

int __fastcall sprite_unk1_redirect(
        Sprite *self, void *, int a, int b, int c, int d, int e, int f, int g) {
    return self->UNK1(a, b, c, d, e, f, g);
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

int __fastcall sprite_unk2_redirect(
        Sprite *self, void *, int a, int b, int c, int d, int e) {
    return self->UNK2(a, b, c, d, e);
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

void __fastcall sprite_unk3_redirect(Sprite *self, void *, int a1, int a2) {
    self->UNK3(a1, a2);
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

void __fastcall sprite_unk4_redirect(Sprite *self, void *, int a1, int a2) {
    self->UNK4(a1, a2);
}

/*
// ORIGINAL: 0x0063CE20 sub_63ce20 0x0063CE20-0x0063CEE7;0x00663998-0x006639AD
// symbol    ?sub_63ce20@@YAHXZ
// RULED-OUT: the original wraps the local Buffer in an fs:[0] SEH frame; no swept flag set reproduces that prologue/epilogue or the secondary 0x663998 span, so the SEH shape stays unmatched.
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

// Sheet extraction / blank creation reached by the per-control init_class
// bodies. Bodies not yet recovered; each forwards through the seam.
typedef int (OriginalObject::*func_sprite_extract)(Buffer *, int, int, int, int, int, TexHeap *);
typedef int (OriginalObject::*func_sprite_create_blank)(int, int, int);
static func_sprite_extract SpriteExtractOriginal = original_method<func_sprite_extract>(0x005E39A0);
static func_sprite_create_blank SpriteCreateBlankOriginal = original_method<func_sprite_create_blank>(0x005EAF3F);

int Sprite::extract(Buffer *buffer, int a, int b, int c, int width, int height,
                    TexHeap *heap) {
    return (ORIGINAL(this)->*SpriteExtractOriginal)(buffer, a, b, c, width, height, heap);
}

int Sprite::create_blank(int width, int height, int depth) {
    return (ORIGINAL(this)->*SpriteCreateBlankOriginal)(width, height, depth);
}
