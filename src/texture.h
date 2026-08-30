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

 /*
  * Texture class
  *
  * Layout not established. The constructor and close agree on exactly four
  * fields and touch nothing else: a pixel pointer at 0, two descriptors at 4
  * and 8, and a borrowed flag at 0x6C. close frees the pixels only when that
  * flag is clear, which is what marks it a flag rather than another
  * descriptor - a borrowed texture is released without freeing. The object
  * therefore reaches at least 0x70; that is a floor, not a size, and the
  * storage below is what the canary seeds rather than a modelled layout.
  */
class Texture {
 public:
  Texture();
  ~Texture();
  void close();

 private:
  void *pixels_;        // 0x00
  uint32_t iWidth_;
  uint32_t iHeight_;
  uint8_t unmapped_[0x60];
  uint32_t borrowed_;   // 0x6C, nonzero when the pixels are not ours to free
};

static_assert(sizeof(Texture) == 0x70,
              "Texture layout must match the original executable");

 /*
  * TextureStore class
  *
  * Only its destructor is recovered. That writes two fields and calls nothing,
  * leaving a count of 3 at offset 0 and clearing the field at 4 - so the
  * object reaches at least 8. Nothing pins its size.
  */
class TextureStore {
 public:
  TextureStore() { ; }
  // A method rather than a real constructor, as Cursor::construct is: the
  // inline TextureStore() above is relied on elsewhere. It answers `this`
  // because the original's closing value in eax is the object.
  TextureStore *construct();
  ~TextureStore();

 private:
  // ANCHORED FROM THE CONSTRUCTOR. 0x006252A0 opens `mov eax, ecx` and
  // writes `[eax] = 3` then `[eax+4] = 0` - this member and iWidth_, at
  // the two offsets the walk gives them, and field_0_'s own name agrees.
  uint32_t field_0_;  // 0x0
  uint32_t iWidth_;
  // SIZED BY THE EMBEDDED ARRAYS, 2026-08-30. MapWin and Console carry
  // TextureStore BY VALUE as TextureStore[4] at front+0xC and
  // TextureStore[0xC4] at front+0x98C - the image's own ??_M element
  // destructor is ??1TextureStore (0x006252B0) in both - and the second
  // array's end lands exactly on the lone TextureStore at front+0x1DB0C,
  // whose end is the field the map already names field_1DD6C_. That pins
  // the stride at 0x260 per element: 0x98C = 0xC + 4*0x260 and
  // 0x1DB0C = 0x98C + 0xC4*0x260. Nothing recovered touches bytes 8..0x260,
  // so the tail stays an unnamed slab - a floor from the dtor's own two
  // fields, a size from the two arrays' arithmetic.
  uint8_t unmapped_[0x260 - 0x8];
};

static_assert(sizeof(TextureStore) == 0x260,
              "TextureStore must match the embedded array stride in MapWin");


typedef void *func_texture_free(void *);


