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
#include "texture.h"
#include <new>

func_texture_free *TextureFree = (func_texture_free *)0x00644EF2;

/*
Purpose: Start an empty texture - no pixels, no descriptors, not borrowed.
ORIGINAL: 0x00619650 BYTE_EXACT
Return Value: n/a
Status: Complete
*/
Texture::Texture() {
    pixels_ = nullptr;
    iWidth_ = 0;
    iHeight_ = 0;
    borrowed_ = 0;
}

/*
Purpose: Destroy the texture. Unlike close, which resets every field, the
         destructor only settles the pixel block: freed and cleared when there
         are pixels and they are ours, left entirely alone otherwise. The
         descriptor fields and the borrowed flag keep whatever they held.
ORIGINAL: 0x00619660
Return Value: n/a
Status: Complete
*/
Texture::~Texture() {
    if (pixels_ && !borrowed_) {
        TextureFree(pixels_);
        // Written through a volatile view for the same reason
        // TextureStore::~TextureStore does: this is the last store of a
        // destructor, so nothing in C++ can observe it and an optimising
        // build is entitled to drop it. The original performs it, and the
        // caller's memory keeps the cleared pointer afterwards, so it has
        // to survive.
        *reinterpret_cast<void *volatile *>(&pixels_) = nullptr;
    }
}

/*
Purpose: Release the texture. The pixels are freed only when they are ours -
         that is, when there are pixels at all and the borrowed flag is clear;
         a borrowed texture keeps its block and is merely forgotten. Either way
         the same four fields the constructor set are reset, except that a
         borrowed texture leaves its pixel pointer alone.
ORIGINAL: 0x00619690
Return Value: n/a
Status: Complete
*/
void Texture::close() {
    if (pixels_ && !borrowed_) {
        TextureFree(pixels_);
        pixels_ = nullptr;
    }
    iWidth_ = 0;
    iHeight_ = 0;
    borrowed_ = 0;
}

/*
Purpose: Tear down the store, leaving a count of 3 at offset 0 and clearing
         the field at 4. Calls nothing.
ORIGINAL: 0x006252B0 BYTE_EXACT
Return Value: n/a
Status: Complete
*/
TextureStore::~TextureStore() {
    // Written through a volatile view for the same reason GraphicWin::close
    // does: these are the last stores of a destructor, so nothing in C++ can
    // observe them and an optimising build is entitled to drop them. The
    // original performs them, and callers read the fields afterwards, so they
    // have to survive.
    volatile uint32_t *const fields = reinterpret_cast<volatile uint32_t *>(this);
    fields[0] = 3;
    fields[1] = 0;
}

Texture *__fastcall texture_ctor_redirect(Texture *self, void *) {
    new (self) Texture();
    return self;
}

void __fastcall texture_close_redirect(Texture *self, void *) {
    self->close();
}

void __fastcall texture_dtor_redirect(Texture *self, void *) {
    self->~Texture();
}

void __fastcall texture_store_dtor_redirect(TextureStore *self, void *) {
    self->~TextureStore();
}

/*
Purpose: Set the store's two fields to 3 and 0.

             mov eax,ecx / mov [eax],3 / mov [eax+4],0 / ret

         `mov eax,ecx` first is the legacy EAX = this residue, carried by the
         redirect. The 3 is a constant the original writes and is reproduced as
         one; nothing here says what it means.
ORIGINAL: 0x006252A0
Return Value: this
Status: Complete
*/
void TextureStore::construct() {
    field_0_ = 3;
    iWidth_ = 0;
}

TextureStore *__fastcall texture_store_construct_redirect(TextureStore *self,
                                                          void *) {
    self->construct();
    return self;
}
