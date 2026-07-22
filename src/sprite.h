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

class Buffer;  // forward declaration

 /*
  * Sprite class
  */
class DLLEXPORT Sprite {
 public:
  int UNK1(int a, int b, int c, int d, int e, int f, int g);
  int UNK2(int a, int b, int c, int d, int e);
  int draw(Buffer *buffer, int a, int b, int c, int x, int y);
  Sprite();
  ~Sprite() { ; }

  void close();

  void UNK3(int a1, int a2);
  void UNK4(int a1, int a2);
 private:
  int field_0_;
  int field_4_;
  char field_8_;
  char pad1_;
  char pad2_;
  char pad3_;
  int field_C_;
  int field_10_;
  int field_14_;
  int field_18_;
  int field_1C_;
  int field_20_;
  int field_24_;
  int field_28_;
};

static_assert(sizeof(Sprite) == 0x2C, "Sprite layout must match the legacy ABI");

// Running total of Sprite-owned bytes. The constructor adds one object; the
// release paths subtract the object plus its pixel buffer.
extern int *SpriteMemoryUsed;

// Sprite allocations come from the executable's CRT, so they must be released
// through its free rather than this module's. Tests outside the hybrid process
// rebind this.
typedef void *func_sprite_free(void *);
extern func_sprite_free *SpriteFree;

Sprite *__fastcall sprite_construct_redirect(Sprite *self, void *);
void __fastcall sprite_close_redirect(Sprite *self, void *);

int __fastcall sprite_draw_redirect(
    Sprite *self, void *, Buffer *buffer, int a, int b, int c, int x, int y);

// The draw origin this overload substitutes for the duration of the call.
extern int *SpriteDrawOriginX;
extern int *SpriteDrawOriginY;

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
#endif
// The four-argument overload this one wraps is a 3225-byte body with eleven
// call targets, still an original dependency. Tests rebind this seam.
typedef int(__thiscall func_sprite_draw_original)(
    Sprite *, Buffer *, int, int, int);
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
extern func_sprite_draw_original *SpriteDrawOriginal;

int __fastcall sprite_unk1_redirect(
    Sprite *self, void *, int a, int b, int c, int d, int e, int f, int g);
int __fastcall sprite_unk2_redirect(
    Sprite *self, void *, int a, int b, int c, int d, int e);
void __fastcall sprite_unk3_redirect(Sprite *self, void *, int a1, int a2);
void __fastcall sprite_unk4_redirect(Sprite *self, void *, int a1, int a2);
