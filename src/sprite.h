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
#pragma once
#include "stdafx.h"
#include "original_seam.h"

class Buffer;  // forward declaration
struct TexHeap;  // texture-heap handle, only ever passed through as a pointer

 /*
  * Sprite class
  */
class Sprite {
 public:
  int UNK1(int a, int b, int c, int d, int e, int f, int g);
  int UNK2(int a, int b, int c, int d, int e);
  int draw(Buffer *buffer, int a, int b, int c, int x, int y);
  int draw(Buffer *buffer, int a, int b, int c);
  // Sprite-sheet extraction used by the per-control init_class bodies; declared
  // here so they compile, still pending_bodies forwarders. `extract`'s last
  // parameter is a texture-heap pointer the callers pass as null.
  int extract(Buffer *buffer, int a, int b, int c, int width, int height,
              TexHeap *heap);
  int create_blank(int width, int height, int depth);
  Sprite();
  // `jmp ?close@Sprite@@QAEXXZ` in the image - see Palette.
  MEASURED ~Sprite() { close(); }   // 00406850

  void close();

  void UNK3(int a1, int a2);
  void UNK4(int a1, int a2);
 private:
  // `Win::flip` draws the overlay sprite and takes its frame index from
  // `cTransparentIndex_`, which is the byte the image reads at +8.
  friend class Win;
  int ppszFileName_;
  int pcBits_;
  char cTransparentIndex_;
  char pad1_;
  char pad2_;
  char pad3_;
  int iSpriteWidth2_;
  int iSpriteWidth_;
  int iSpriteHeight_;
  int iWidth_;
  int iHeight_;
  int iLeftOffset_;
  int iTopOffset_;
  int fObj1Exists_;
};

static_assert(sizeof(Sprite) == 0x2C, "Sprite layout must match the legacy ABI");

// Running total of Sprite-owned bytes. The constructor adds one object; the
// release paths subtract the object plus its pixel buffer.
int *const SpriteMemoryUsed = (int *)0x009B6618;

// Sprite allocations come from the executable's CRT, so they must be released
// through its free rather than this module's. Tests outside the hybrid process
// rebind this.
typedef void *func_sprite_free(void *);

Sprite *__fastcall sprite_construct_redirect(Sprite *self, void *);
void __fastcall sprite_close_redirect(Sprite *self, void *);

int __fastcall sprite_draw_redirect(
    Sprite *self, void *, Buffer *buffer, int a, int b, int c, int x, int y);

// The draw origin this overload substitutes for the duration of the call.
extern int SpriteDrawOriginX;
extern int SpriteDrawOriginY;

// The four-argument overload this one wraps is a 3225-byte body with eleven
// call targets, still an original dependency. Tests rebind this seam.

int __fastcall sprite_unk1_redirect(
    Sprite *self, void *, int a, int b, int c, int d, int e, int f, int g);
int __fastcall sprite_unk2_redirect(
    Sprite *self, void *, int a, int b, int c, int d, int e);
void __fastcall sprite_unk3_redirect(Sprite *self, void *, int a1, int a2);
void __fastcall sprite_unk4_redirect(Sprite *self, void *, int a1, int a2);
