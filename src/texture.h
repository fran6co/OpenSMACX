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
class DLLEXPORT Texture {
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
class DLLEXPORT TextureStore {
 public:
  TextureStore() { ; }
  // A method rather than a real constructor, as Cursor::construct is: the
  // inline TextureStore() above is relied on elsewhere.
  void construct();
  ~TextureStore();

 private:
  uint32_t field_0_;
  uint32_t iWidth_;
};

// close releases the pixels through the executable's CRT free, as the other
// classes that own heap blocks do, until that ownership boundary moves.
// Returns the object, preserving the legacy EAX = this residue.
TextureStore *__fastcall texture_store_construct_redirect(TextureStore *self, void *);

typedef void *func_texture_free(void *);
extern func_texture_free *TextureFree;

void __fastcall texture_dtor_redirect(Texture *self, void *);

// The original constructor returns `this` in eax, as MSVC constructors do.
Texture *__fastcall texture_ctor_redirect(Texture *self, void *);
void __fastcall texture_close_redirect(Texture *self, void *);
void __fastcall texture_store_dtor_redirect(TextureStore *self, void *);
