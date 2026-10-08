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

#include "vert.h"

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
// The rasteriser uses the shared polygon vertex declaration in vert.h.
// The remaining types are reached only through pointers here.
class Buffer;
class TextureStore;
struct TexHeap;

// Shared raster key/threshold; terrain initialization supplies its value.
// Original storage and writer evidence are bound beside its definition.
extern unsigned int TextureRasterKey;
extern int TextureLightFloor;
extern int TextureLightCeiling;
extern int TextureRowPitchScratch;

/*
 * THE EDGE SCANNER, one per polygon side, and the state `setup_edge` and
 * `setup_edge_gouraud` fill in for the scanline loop that follows them.
 *
 * SIZED BY WHAT THE TWO SETUPS WRITE, and they agree with the callers: the
 * plain `setup_edge` writes 0x00..0x30 and stops, `setup_edge_gouraud` adds
 * the two fixed-point colour fields at 0x34 and 0x38 and stops, and the
 * largest caller (draw at 0x006233C0) declares `int edge[15]` = 0x3C bytes
 * for the pair of them. Fifteen ints is the superset, to the byte.
 */
struct EdgeScan {
    // ONE EDGE OF THE POLYGON, and the state the two setups leave for the
    // scanline loop. Named from what `setup_edge`, `setup_edge_gouraud` and
    // `edge_step` do to each slot; the three read as one DDA once the names
    // are in - `accumA += deltaA`, and so on down the channels.
    int f0;           // 0x00 vertex traversal direction, -1 or +1
    int counter;      // 0x04 rows this edge spans, the DDA's divisor
    int id;           // 0x08 next endpoint's vertex index
    int accumA;       // 0x0c first interpolant, accumulator
    int accumB;       // 0x10 second interpolant, accumulator
    int deltaA;       // 0x14 first interpolant's step
    int deltaB;       // 0x18 second interpolant's step
    int accumC;       // 0x1c third channel, accumulator
    int deltaC;       // 0x20 third channel's step
    int extraDeltaC;  // 0x24 added to accumC when the fraction wraps
    int fracAccum;    // 0x28 the DDA's fractional accumulator
    int fracDelta;    // 0x2c its step
    int fracSub;      // 0x30 what comes back off it on a wrap
    int accumD;       // 0x34 gouraud light, accumulator
    int deltaD;       // 0x38 gouraud light, step
};
static_assert(sizeof(EdgeScan) == 0x3C, "cached edge record stride");

/*
 * The texture's coordinate pairs, eight bytes each. setup_edge reads the
 * two components at [i * 2] and [i * 2 + 1]. These embedded interpolation
 * coordinates retain the current EdgePoint name. Their shape agrees with
 * Vert, but this separate declaration does not prove a distinct original type.
 */
struct EdgePoint {
    int a;
    int b;
};

class Texture {
 public:
  // homed from 00622db0.cpp
  int draw_trans_3(Buffer *buffer, Vert *verts, int *zvals, int a4,
                   Vert *edgeVerts, int a6);

 public:
  // homed from 00619d80.cpp
  int draw_trans2(Buffer *buffer, int *verts, int a3);

 public:
  Texture();
  ~Texture();
  void close();

  // ===== THE SOFTWARE BLITTERS =====
  // Every one of these is a scanline polygon fill over `Buffer`'s pixel
  // block, differing only in the blend it applies per texel: plain copy,
  // colour-key transparency, alpha, mask-from-source, mask-from-destination,
  // and the two gouraud-shaded variants that interpolate a colour along each
  // edge. `draw_coast` is the coastline overlay, which blends the edge band
  // of a neighbouring texture. The two `setup_edge` helpers are the shared
  // DDA edge walker, `static` because it reads and writes no Texture field -
  // the image's names end `QAA`, a static __cdecl member.
  // The parameter names below are read off the bodies, not guessed: every
  // one of these opens by storing its arguments into the fixed blit slots,
  // and the slot names say what went there. `zvals` is the per-vertex depth
  // array the light-threshold scan walks, `remap` is the colour-key table,
  // and `edgeVerts` is the second vertex-array base - the one that is either
  // `corners_` or `cornersAndMids_` on the texture itself, or a caller's
  // array when the polygon is not the whole texture.
  int extract(Buffer *buffer, int x, int y, int width, int height,
              TexHeap *heap);
  // 0x006196D0: rectangle forwarding overload. The third incoming 32-bit
  // slot is unused; int preserves its ABI without claiming its original type.
  int extract(Buffer *buffer, RECT *rect, int unused);
  // 0x006198E0 uses explicit UV vertices and opaque, inclusive spans from
  // borrowed 256-byte-pitch texture pages; unlike the color-key overload.
  void draw_trans(Buffer *buffer, Vert *verts, int count, Vert *uv);
  // 0x0061A281..0x0061A2AA selects four vertices for zero, eight otherwise.
  void draw_trans(Buffer *buffer, Vert *verts, int use_eight_vertices);
  // 0x006239B0, the narrowest of the sentinel wrappers: a5 == -1 forwards
  // straight to the four-argument overload above.
  void draw_trans(Buffer *buffer, Vert *verts, int count, Vert *last,
                  int sentinel);
  // 0x0061A760: the color-key raster over an arbitrary caller vertex/UV pair.
  // Four arguments (ret 0x10): the count is a plain scalar the catalogue had
  // as `int *`, and the last argument is the explicit UV vertex base. The
  // kernel skips only texels equal to the low byte of TextureRasterKey; the
  // provisional draw_color_key name records the behaviour, not an original
  // debug symbol.
  void draw_color_key(Buffer *buffer, Vert *verts, int count, Vert *uv);
  // 0x006239E0: the alternating-row variant. color == -1 forwards to the
  // four-argument overload before any lock or FPU reset; every other value
  // runs the keyed raster with absolute even screen rows recolored to the
  // color's low byte and odd rows copied, keeping color-key holes in both.
  void draw_color_key(Buffer *buffer, Vert *verts, int count, Vert *uv,
                      int color);
  void draw(Buffer *buffer, Vert *verts, int *zvals, int a4, Vert *edgeVerts);
  void draw_trans(Buffer *buffer, Vert *verts, int *zvals, int a4,
                  Vert *edgeVerts);
  void draw_mask_dest(Buffer *buffer, Vert *verts, uint8_t *remap,
                      uint8_t *a4, int count, Vert *edgeVerts);
  void draw_coast(Buffer *buffer, Vert *verts, int *zvals, Texture *coast,
                  uint8_t *remap, int count, Vert *edgeVerts);
  void draw(Buffer *buffer, Vert *verts, int *zvals, int a4, Vert *edgeVerts,
            int lightThresh);
  void draw_coast(Buffer *buffer, Vert *verts, int *zvals, Texture *coast,
                  uint8_t *remap, int count, Vert *edgeVerts, int a8);
  static int setup_edge_gouraud(EdgeScan *edge, int index);
  static int setup_edge(EdgeScan *edge, int index);

  // ===== THE SPAN RASTERISER FAMILY =====
  // Every one of these opens `mov esi, ecx` (or `mov edi, ecx`) and never
  // reads a stack slot for the receiver, so each is a __thiscall member -
  // the archive had them as `__stdcall` free functions or as invented
  // one-method classes (RasterSrc, RasterSrc2, GouraudSrc, TriRender) before
  // the prologues settled it. They differ only in the setup they drive and
  // the blend the inner loop applies:
  //   * the `setup_edge` group interpolates position only, flat colour;
  //   * the `setup_edge_gouraud` group adds a per-pixel interpolant
  //     (u/v, or a light value on top);
  //   * `sub_625080` sets up Gouraud polygon edges through TextureStore's
  //     independent geometry/light cache channels.
  // Names marked with an address are this tree's, not the image's: those
  // bodies carry no symbol in the shipped binary, so the name says what the
  // body does and the address is the only handle the catalogue has.
  void draw_below_cutoff(Buffer *buffer, Vert *verts, int *zvals, int count,
                         Vert *edgeVerts, int light_thresh);                    // 0x0061AC60
  // Alternating-row depth-cutoff fill; fillColor == -1 delegates to the
  // ordinary cutoff rasterizer. UVs are coordinate pairs, not opaque words.
  void draw2(Buffer *buffer, Vert *verts, int *zvals, int count,
             Vert *uv, int zCutoff,
             int fillColor);                                    // 0x00624670
  // THE SIX THE CATALOGUE NEVER NAMED. Their members keep the address-derived
  // name the catalogue gave them - it is the only handle the archive has for
  // them, and homing looks the definition up by it. What they are:
  //   sub_61b780  gouraud affine rasteriser; `mode` picks the 4- or
  //               8-vertex vertex-array base at 0x0c / 0x2c
  //   sub_61c630  per-span fill across two textures with a shade table
  //   sub_61cb50  dual-texture remap raster over Buffer rows; mode selects
  //               the four-corner or eight-point UV perimeter.
  //   sub_61d990  gouraud-lit draw over a second texture's shade table
  //   sub_61df60  per-vertex Gouraud light, mask/remap and palette compositor
  //   sub_61f0d0  gouraud family, hands each span to sub_620100
  //   sub_620a10  affine rasteriser, setup_edge family + sub_624eb0
  //   sub_622200  cached-edge family: four sub_625080 setup call sites
  void sub_61b780(Buffer *bufParam, int *verts, int zvals, int mode);
  void sub_61c630(Buffer *bufIn, int *verts, Texture *texB, uint8_t *shadeTable,
                  int vertCount, int unusedIn);
  void sub_61cb50(Buffer *buffer, Vert *vertices, Texture *shade,
                 unsigned char *remap, int use_eight_vertices);
  void sub_61d990(Buffer *buffer, int *vertArray, int *zvals, Texture *texB,
                 uint8_t *table, int wideFlag);
  void sub_61df60(Buffer *buffer, Vert *vertices, int *zvals,
                 Texture *mask_texture, unsigned char *remap, int count,
                 Vert *uv, int threshold, unsigned char *palette);
  void sub_61f0d0(Buffer *param_2, int param_3, int param_4, int param_5,
                  int param_6, uint32_t param_7, int param_8);
  // The final pointer is the shared ten-record cache consumed by sub_624eb0;
  // flags_ selects geometry-cache replay/fill and next_record_ is its cursor.
  void sub_620a10(Buffer *bufParam, int vertsParam, int countParam,
                  int edgeVerts, TextureStore *cache);
  // 0x0061D090: receiver loads and RET 0x10 establish Texture this + four
  // stack arguments; this replaces the old free-function catalogue shape.
  void sub_61d090(Buffer *buffer, int *vertices, int *zvals, unsigned int flag);
  void sub_622200(Buffer *buffer, Vert *vertices, int *zvals,
                  Texture *shade_texture, unsigned char *remap, int count,
                  Vert *edge_vertices, TextureStore *cache);
  // 0x00623F50. Seven ints, the seventh the -1 sentinel that selects the
  // forward into the span filler - the same idiom `draw2` carries.
  void sub_623f50(Buffer *buffer, int *verts, Texture *texB,
                  uint8_t *shadeTable, int vertCount, int unusedIn,
                  int sentinel);
  // 0x00624CA0. Six stack arguments, with the fifth a TextureStore* and the
  // sixth the -1 sentinel; the historical int* guess is retained in the
  // source annotation comments beside the body.
  void sub_624ca0(Buffer *buffer, int verts, int count, int a4,
                  TextureStore *cache, int sentinel);
  void sub_6212b0(Buffer *buf, Vert *verts, int *zvals, int count,
                  Vert *edgeVerts, TextureStore *cache);                       // 0x006212B0
  void sub_622810(Buffer *buf, Vert *verts, int *zvals, int count,
                  Vert *edgeVerts, int light_thresh,
                  TextureStore *cache);                              // 0x00622810

  // ===== THE -1 SENTINEL WRAPPERS =====
  // Five thin members that all read the same way: the LAST argument is a
  // sentinel, and when it is -1 the rest are forwarded to the rasteriser of
  // the same arity. The image never loads ECX in any of them, and every
  // callee is a __thiscall member - so the receiver travels through ECX
  // untouched, which is what makes these members of Texture rather than
  // __stdcall free functions the archive had them as. The arities below are
  // read off the image, not the archive: each wrapper's `ret N` is N dwords
  // and its callee's own `ret` accounts for the N-1 forwarded arguments
  // (sub_6212b0 ret 0x18 for sub_624c70's 7, sub_622200's 8 for
  // sub_624d00's 9, and so on), so the two ends agree for the first time.
  void sub_624c70(Buffer *buf, Vert *verts, int *zvals, int count,
                  Vert *edgeVerts, TextureStore *cache,
                  int sentinel);                                 // 0x00624C70
  // The sentinel overload forwards only for -1; the cache owns ten
  // lazily populated edge records, not a scalar output flag.
  void draw_cached_lit(Buffer *buf, Vert *verts, int *zvals, int count,
                        Vert *edgeVerts, TextureStore *cache,
                        int sentinel);                                 // 0x00624CD0
  void sub_624d00(Buffer *buffer, Vert *vertices, int *zvals,
                  Texture *shade_texture, unsigned char *remap, int count,
                  Vert *edge_vertices, TextureStore *cache,
                  int sentinel);                                  // 0x00624D00
  void sub_624d40(Buffer *buf, Vert *verts, int *zvals, int count,
                  Vert *edgeVerts, int light_thresh,
                  TextureStore *cache, int sentinel);            // 0x00624D40
  void sub_624630(Buffer *buffer, Vert *vertices, int *zvals,
                 Texture *mask_texture, unsigned char *remap, int count,
                 Vert *uv, int threshold, unsigned char *palette, int sentinel);
  // 0x00621870: cached Gouraud polygon, keyed borrowed storage and
  // unkeyed owned storage with the original integer-light arithmetic.
  void draw_cached_lit(Buffer *buffer, Vert *vertices, int *light, int count,
                        Vert *uv, TextureStore *cache);

 private:
  void *pixels_;        // 0x00
  uint32_t iWidth_;
  uint32_t iHeight_;
  // THE TWO VERTEX-ARRAY BASES, 0x0c and 0x2c. Evidenced three ways: the
  // rasterisers take a base off `this` at one of exactly those two offsets
  // and index it as EdgePoint[4] or EdgePoint[8] (`sub_61d990` and
  // `sub_61cb50` both pick `+0xc` with a vertex count of 4 and `+0x2c` with
  // 8); `extract` fills the whole 0x60 bytes between 0x0c and 0x6c with
  // exactly 24 ints; and 0x0c + 4*8 is 0x2c, 0x2c + 8*8 is 0x6c, which is
  // `borrowed_` below. What goes in them is `extract`'s to say: `corners_`
  // gets the four corners (0,0), (w-1,0), (w-1,h-1), (0,h-1) and the wide
  // array gets those four plus the four edge midpoints, which is the 8-point
  // perimeter the gouraud rasterisers interpolate across.
  EdgePoint corners_[4];        // 0x0c .. 0x2b
  EdgePoint cornersAndMids_[8];  // 0x2c .. 0x6b
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

// Shared radius overlays, selected by the signed map-window zoom.
extern Texture g_RADIUS1_TEXTURE;
extern Texture g_RADIUS2_TEXTURE;

 /*
  * TextureStore class
  *
  * Reusable polygon-edge cache. Original 625080 uses flag bits 1/2 to
  * recompute light/geometry channels independently, and advances a ten-slot
  * ring of 0x3C-byte records after each admitted positive-height edge.
  * Both lifetime bodies set flags=3 and cursor=0 without clearing records.
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
  EdgeScan *sub_6252c0();

  // Source names express the cache protocol, not recovered debug names.
  // 625139/6251AC/62523B read flag bits; 6250E0..6250FA advances and wraps
  // the cursor at exactly ten. A replay requires previously initialized
  // selected channels; unused record fields intentionally remain untouched.
  // RECOVERY_MEMBER: v1 TextureStore flags_ 0x0 0x00625080
  unsigned int flags_;
  // RECOVERY_MEMBER: v1 TextureStore next_record_ 0x4 0x00625080
  int next_record_;
  // SIZED BY THE EMBEDDED ARRAYS, 2026-08-30. MapWin and Console carry
  // TextureStore BY VALUE as TextureStore[4] at front+0xC and
  // TextureStore[0xC4] at front+0x98C - the image's own ??_M element
  // destructor is ??1TextureStore (0x006252B0) in both - and the second
  // array's end lands exactly on the lone TextureStore at front+0x1DB0C,
  // whose end is the field the map already names field_1DD6C_. This pins
  // stride 0x260 independently of the helper: 0x98C = 0xC + 4*0x260 and
  // 0x1DB0C = 0x98C + 0xC4*0x260. Helper6250EA..F4 selects +8+cursor*3C,
  // and accesses the same UV/X/light fields as an EdgeScan. Do not zero
  // the array in construction: the original initializes channels lazily.
  // RECOVERY_MEMBER: v4 TextureStore edges_ EdgeScan[10] 0x8 0x258 0x00625080
  EdgeScan edges_[10];
};

static_assert(sizeof(TextureStore) == 0x260,
              "TextureStore must match the embedded array stride in MapWin");


typedef void *func_texture_free(void *);
