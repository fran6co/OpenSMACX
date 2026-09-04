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
 * THE TERRAIN TEXTURES, the 0x70-stride band 0x0076E8A0..0x007AD010 that
 * `load_terrain` (0x00451C50) extracts out of the terrain PCX sheets.
 * Declared here because the map renderer - gen_terrain_poly (0x004632D0) and
 * the gen_radius / gen_tile family - reaches them from another TU by absolute
 * address; the definitions and the stride comments live in texture.cpp beside
 * the extract calls that size them. Unlike the Sprite band there is no +8
 * companion read: a Texture carries no transparent index, so the renderer
 * passes the address of the object itself to every draw_* variant.
 */
extern Texture g_RAINFALL_SINGLE_TILE_TEXTURES[2];  // 0x0076E8A0
extern Texture g_ROCKY_TEXTURES[4];                 // 0x0076E9E0
extern Texture g_FUNGUS_TEXTURES[30];               // 0x00776A80
extern Texture g_SUNNY_MESA_TEXTURES[8];            // 0x007797F8
extern Texture g_GARLAND_CRATER_TEXTURES[3];        // 0x00788100
extern Texture g_JUNGLE_LAND_TEXTURES[15];          // 0x00789C28
extern Texture g_MOUNT_PLANET_TEXTURES[3];          // 0x0078A340
extern Texture g_FOREST_TEXTURES[16];               // 0x0078A758
extern Texture g_RAINFALL_SINGLE_TILE_TEXTURE;      // 0x0078DD80
extern Texture g_ROAD_TEXTURES[9];                  // 0x00792218
extern Texture g_MAGTUBE_TEXTURES[9];               // 0x00798E08
extern Texture g_FARM_TEXTURES[9];                  // 0x00799238
extern Texture g_RAINY_LAND_TEXTURES[16];           // 0x00799738
extern Texture g_FLAT_ARID_LAND_TEXTURE;            // 0x00799E48
extern Texture g_MOIST_LAND_TEXTURES[16];           // 0x00799EB8
extern Texture g_OCEAN_TEXTURES[2];                 // 0x007A7820
extern Texture g_RIVER_TEXTURES[16];                // 0x007A7AA0
extern Texture g_DUNE_LAND_TEXTURE;                 // 0x007AC220

 /*
  * TextureStore class
  *
  * Only its destructor is recovered. That writes two fields and calls nothing,
  * leaving a count of 3 at offset 0 and clearing the field at 4 - so the
  * object reaches at least 8. Nothing pins its size.
  */
class TextureStore {
 public:
  // REAL AND OUT OF LINE. A body spelled `{ ; }` inline is trivial to VC6 and
  // MapWin's constructor then emits no call at all for the lone TextureStore
  // at front+0x1DB0C, which the image does emit (`call ??0TextureStore` there,
  // its only caller). Out of line, VC6's own constructor - which returns the
  // object in eax exactly as the image does - is called both there and for
  // the two arrays through the vector-construction helper.
  TextureStore();
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


