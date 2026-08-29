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
#include <stdlib.h>

/*
Purpose: Start an empty texture - no pixels, no descriptors, not borrowed.
// ORIGINAL: 0x00619650 ??0Texture@@QAE@XZ 0x00619650-0x00619660 BYTE_EXACT
// size      16 bytes
// prototype void (__thiscall ??0Texture@@QAE@XZ)(Texture* this)
// callers   5   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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
// ORIGINAL: 0x00619660 ??1Texture@@QAE@XZ 0x00619660-0x00619685
// size      37 bytes
// prototype void (__thiscall ??1Texture@@QAE@XZ)(Texture* this)
// callers   0   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00644EF2
Return Value: n/a
Status: Complete
*/
Texture::~Texture() {
    if (pixels_ && !borrowed_) {
        free(pixels_);
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
// ORIGINAL: 0x00619690 ?close@Texture@@QAEXXZ 0x00619690-0x006196CA
// size      58 bytes
// prototype void (__thiscall ?close@Texture@@QAEXXZ)(Texture* this)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00644EF2
Return Value: n/a
Status: Complete
*/
void Texture::close() {
    if (pixels_ && !borrowed_) {
        free(pixels_);
        pixels_ = nullptr;
    }
    iWidth_ = 0;
    iHeight_ = 0;
    borrowed_ = 0;
}

/*
Purpose: Tear down the store, leaving a count of 3 at offset 0 and clearing
         the field at 4. Calls nothing.
// ORIGINAL: 0x006252B0 ??1TextureStore@@QAE@XZ 0x006252B0-0x006252BE BYTE_EXACT
// size      14 bytes
// prototype void (__thiscall ??1TextureStore@@QAE@XZ)(TextureStore* this)
// callers   6   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
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





/*
Purpose: Set the store's two fields to 3 and 0.

             mov eax,ecx / mov [eax],3 / mov [eax+4],0 / ret

         `mov eax,ecx` first is the legacy EAX = this residue, carried by the
         redirect. The 3 is a constant the original writes and is reproduced as
         one; nothing here says what it means.
// ORIGINAL: 0x006252A0 ??0TextureStore@@QAE@XZ 0x006252A0-0x006252B0 BYTE_EXACT
// LEVER: a `construct` METHOD returning `TextureStore *`, not a real constructor and not `void`. The image opens `mov eax, ecx` and stores through eax - that is the return of `this`, and a `void construct()` emits the two stores through ecx and never writes eax.
// symbol    ?construct@TextureStore@@QAEPAV1@XZ
// size      16 bytes
// prototype void (__thiscall ??0TextureStore@@QAE@XZ)(TextureStore* this)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: this
Status: Complete
*/
TextureStore *TextureStore::construct() {
    field_0_ = 3;
    iWidth_ = 0;
    return this;
}

// ===== MANAGED GLOBALS - real objects, homed to their domain =====
// In the shipped image these live at fixed data addresses and are
// constructed before WinMain by the CRT's dynamic-initializer walk
// (winedbg-confirmed: walker return 0x00644EEB, table at 0x682568..).
// Here the same recovered constructors run through this build's own
// startup, and the matching destructors close them at exit.
Texture g_RADIUS1_TEXTURE;  // 0x00787FB8
Texture g_RADIUS2_TEXTURE;  // 0x0075B858
Texture g_ROCKY_TEXTURES[4];  // 0x0076E9E0, 0x70 stride
Texture g_OCEAN_TEXTURES[2];  // 0x007A7820, 0x70 stride
Texture g_FLAT_ARID_LAND_TEXTURE;  // 0x00799E48
Texture g_MOIST_LAND_TEXTURES[16];  // 0x00799EB8, 0x70 stride
Texture g_RAINY_LAND_TEXTURES[16];  // 0x00799738, 0x70 stride
Texture g_JUNGLE_LAND_TEXTURES[15];  // 0x00789C28, 0x70 stride
Texture g_DUNE_LAND_TEXTURE;  // 0x007AC220
Texture g_SUNNY_MESA_TEXTURES[8];  // 0x007797F8, 0x70 stride
Texture g_RAINFALL_SINGLE_TILE_TEXTURES[2];  // 0x0076E8A0, 0x70 stride
Texture g_ROAD_TEXTURES[9];  // 0x00792218, 0x70 stride
Texture g_MAGTUBE_TEXTURES[9];  // 0x00798E08, 0x70 stride
Texture g_RIVER_TEXTURES[16];  // 0x007A7AA0, 0x70 stride
Texture g_MOUNT_PLANET_TEXTURES[3];  // 0x0078A340, 0x70 stride
Texture g_GARLAND_CRATER_TEXTURES[3];  // 0x00788100, 0x70 stride
Texture g_FUNGUS_TEXTURES[30];  // 0x00776A80, 0x70 stride
Texture g_FARM_TEXTURES[9];  // 0x00799238, 0x70 stride
Texture g_FOREST_TEXTURES[16];  // 0x0078A758, 0x70 stride
Texture g_RAINFALL_SINGLE_TILE_TEXTURE;  // 0x0078DD80
