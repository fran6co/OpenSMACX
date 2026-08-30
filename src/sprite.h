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
  // THE FOUR UNK METHODS ARE EVIDENCE-FREE, and for a reason that is worth
  // stating precisely because it also disqualifies the one clue they seem to
  // have. UNK1 is `xor eax,eax; ret 0x1c` and UNK2 `xor eax,eax; ret 0x14`:
  // five-byte stubs. VC6 FOLDS IDENTICAL BODIES PROGRAM-WIDE, so those bytes
  // are not uniquely Sprite's - every 7-argument and 5-argument stub in the
  // image shares them - and the single `caller` each records is whichever
  // one the catalogue happened to attribute, not evidence about Sprite.
  // win.h records the same folding for the 51 vtable slots the catalogue
  // hands to AlphaMovie.
  // UNK3 and UNK4 have EMPTY bodies, so their two ints are unused and
  // nothing in the body can name them either.
  int UNK1(int a, int b, int c, int d, int e, int f, int g);
  int UNK2(int a, int b, int c, int d, int e);
  int draw(Buffer *buffer, int a, int b, int c, int x, int y);
  int draw(Buffer *buffer, int a, int b, int c);
  // Sprite-sheet extraction used by the per-control init_class bodies. The
  // body lives in sprite.cpp beside its marker. `extract`'s last parameter is
  // a texture-heap pointer most callers pass as null.
  int extract(Buffer *buffer, int a, int b, int c, int width, int height,
              TexHeap *heap);
  int create_blank(int width, int height, int depth);
  // THE CLASS BRING-UP, named 2026-08-30 from its call site: jackal_init_real
  // runs it in the X::init_class() chain between FileWin::init_class and the
  // font bring-up, and it seeds the class-shared sprite the tree names
  // g_BLANK_SPRITE (0x009BEAE8) - jackal.pcx's 16x16 cell, or a blank when
  // the PCX is missing. jackal_close's mirror teardown is close_class below.
  // The catalogue carries no name for it, so there is no catalogued spelling
  // to conflict with; the body and marker stay in sprite.cpp.
  static int init_class();
  // The mirror teardown: jackal_close tail-calls it as the sprite's
  // teardown stage (0x0063CEF0, BYTE_EXACT as a free function before the
  // 2026-08-30 naming - the body is the one close() and the bytes do not
  // change with the static-member spelling).
  static void close_class();
  Sprite();
  // `jmp ?close@Sprite@@QAEXXZ` in the image - see Palette.
  MEASURED ~Sprite() { close(); }   // 00406850

  void close();

  void UNK3(int menu_id, int item_id);
  void UNK4(int a1, int a2);
 private:
  // `Win::flip` draws the overlay sprite and takes its frame index from
  // `cTransparentIndex_`, which is the byte the image reads at +8.
  friend class Win;
  char *ppszFileName_;  // 0x0000 - heap copy of the source file's name;
                        // released with the CRT free
  char *pcBits_;        // 0x0004 - the pixel block, same allocator
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
// release paths subtract the object plus its pixel buffer. The image holds
// the counter in .bss at 0x009B6618 - zero at load - so the tree owns it as
// a real zero-initialised global (sprite.cpp) rather than a raw address.
extern int SpriteMemoryUsed;

// Sprite allocations come from the executable's CRT, so they must be released
// through its free rather than this module's. Tests outside the hybrid process
// rebind this.
typedef void *func_sprite_free(void *);



// The draw origin this overload substitutes for the duration of the call.
extern int SpriteDrawOriginX;
extern int SpriteDrawOriginY;

// The four-argument overload this one wraps is a 3225-byte body with eleven
// call targets, still an original dependency. Tests rebind this seam.

