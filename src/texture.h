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
// The rasteriser's own types, reached only through pointers here. `Vert`
// stays opaque everywhere in this tree - the blitters walk it as a raw int
// stream - so a forward declaration is all the class model has.
class Buffer;
struct Vert;
struct TexHeap;

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
    int f0;           // 0x00 the Y the walk starts from; callers pre-set it
    int counter;      // 0x04 rows this edge spans, the DDA's divisor
    int id;           // 0x08 the Y the walk landed on
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

/*
 * THE VERTEX, 8 BYTES, TWO INTS. `setup_edge` indexes the two arrays it is
 * handed at `[i * 2]` and `[i * 2 + 1]`, and the vertex-array base the
 * rasterisers read off the texture at 0x9BB490/0x9BB4F4 is stepped by 8, so
 * this is the shape those arrays hold. It stays a separate type from `Vert`
 * above because `Vert` is the image's own tag for the polygon vertex the
 * blitters are passed and this tree has never had to define it.
 */
struct EdgePoint {
    int a;
    int b;
};

class Texture {
 public:
  // homed from 00622db0.cpp
  int draw_trans_3(Buffer *a1, Vert *a2, int *a3, int a4, Vert *a5, int a6);

 public:
  // homed from 00619d80.cpp
  int draw_trans2(Buffer *a1, int *a2, int a3);

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
  int extract(Buffer *buffer, int a2, int a3, int a4, int a5, TexHeap *heap);
  void draw_trans(Buffer *buffer, Vert *verts, int count, Vert *last);
  void draw_trans(Buffer *buffer, Vert *verts, int count);
  void draw_trans(Buffer *buffer, Vert *verts, int a3, Vert *a4, int a5);
  void draw_upper_threshold_trans(Buffer *buffer, Vert *verts, int *a3,
                                  int a4);
  void draw(Buffer *buffer, Vert *verts, int *a3, int a4, Vert *a5);
  void draw_trans(Buffer *buffer, Vert *verts, int *a3, int a4, Vert *a5);
  void draw_mask_dest(Buffer *buffer, Vert *verts, uint8_t *a3, uint8_t *a4,
                      int a5, Vert *a6);
  void draw_coast(Buffer *buffer, Vert *verts, int *a3, Texture *coast,
                  uint8_t *a5, int a6, Vert *a7);
  void draw(Buffer *buffer, Vert *verts, int *a3, int a4, Vert *a5, int a6);
  void draw_coast(Buffer *buffer, Vert *verts, int *a3, Texture *coast,
                  uint8_t *a5, int a6, Vert *a7, int a8);
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
  //   * `sub_625080` is the per-span flat-fill helper the third group hands
  //     each scanline to.
  // Names marked with an address are this tree's, not the image's: those
  // bodies carry no symbol in the shipped binary, so the name says what the
  // body does and the address is the only handle the catalogue has.
  void draw(Buffer *buffer, Vert *verts, int *zvals, int count, int a5,
            int light_thresh);                                   // 0x0061AC60
  int draw2(Buffer *a1, int *a2, int *a3, int a4, int *a5, int a6,
            int a7);                                            // 0x00624670
  // THE SIX THE CATALOGUE NEVER NAMED. Their members keep the address-derived
  // name the catalogue gave them - it is the only handle the archive has for
  // them, and homing looks the definition up by it. What they are:
  //   sub_61b780  gouraud affine rasteriser; `mode` picks the 4- or
  //               8-vertex vertex-array base at 0x0c / 0x2c
  //   sub_61c630  per-span fill across two textures with a shade table
  //   sub_61cb50  setup_edge family, five ints, no Buffer of its own
  //   sub_61d990  gouraud-lit draw over a second texture's shade table
  //   sub_61df60  nine ints, gouraud family, depth-tested
  //   sub_61f0d0  gouraud family, hands each span to sub_620100
  //   sub_620a10  affine rasteriser, setup_edge family + sub_624eb0
  //   sub_622200  flat-fill family: four sub_625080 span calls
  void sub_61b780(Buffer *bufParam, int *verts, int a3, int mode);
  void sub_61c630(Buffer *bufIn, int *verts, Texture *texB, uint8_t *shadeTable,
                  int vertCount, int unusedIn);
  int sub_61cb50(int a1, int a2, int a3, int a4, int a5);
  int sub_61d990(Buffer *a1, int *vertArray, int a3, Texture *a4,
                 uint8_t *table, int a6);
  int sub_61df60(int a1, int a2, int a3, int a4, int a5, int a6, int a7,
                 int a8, int a9);
  void sub_61f0d0(Buffer *param_2, int param_3, int param_4, int param_5,
                  int param_6, uint32_t param_7, int param_8);
  void sub_620a10(Buffer *bufParam, int vertsParam, int countParam, int a4,
                  int *outFlag);
  void sub_622200(int a2, int a3, int a4, int a5, int a6, int a7, int a8,
                  int a9);
  // 0x00623F50. Seven ints, the seventh the -1 sentinel that selects the
  // forward into the span filler - the same idiom `draw2` carries.
  void sub_623f50(int a1, int a2, int a3, int a4, int a5, int a6, int a7);
  // 0x00624CA0. Six ints, fifth-arg -1 sentinel, forward into the affine
  // rasteriser.
  void sub_624ca0(int a1, int a2, int a3, int a4, int a5, int a6);
  void sub_6212b0(Buffer *buf, Vert *verts, int *zvals, int count, int a5,
                  int *out_flag);                                // 0x006212B0
  void sub_622810(Buffer *buf, Vert *verts, int *zvals, int count, int a5,
                  int light_thresh, int *out_flag);              // 0x00622810
  void draw_upper_threshold_trans2(Buffer *buf, Vert *verts, int aux,
                                   int count, int threshold_color);  // 0x006239E0

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


