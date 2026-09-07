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

// THE SHEET HEAP. 0x10 bytes, four uint32 members, the IDB's layout. It used
// to be declared beside its only members in sprite.cpp; the header is where a
// class belongs, and Sprite::extract takes a TexHeap* as its last argument.
// The blocks it hands out are Heap objects - get_mem walks the block table and
// takes the first block whose free tail fits.
struct TexHeap {
    uint32_t basePtr_;     // 0x0
    uint32_t baseSize_;    // 0x4
    uint32_t currentPtr_;  // 0x8
    uint32_t count_;       // 0xC
    int get_mem(int size);
    // THE SECOND BLOCK-TABLE WALK, 0x00635370, and the one Texture::extract
    // asks for. Identical head and stride to get_mem above, one extra test in
    // the filter: a block only qualifies when the low half of its `current_`
    // cursor is clear as well as its free tail fitting the request.
    int get_tex_mem(int size);
    void shutdown();
    int UNK1(int size);
};

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
  // The draw_dest family: the five-argument form does the work, and each
  // wider sibling is a save/set/call/restore wrapper that substitutes a draw
  // origin or scroll pair for the duration of the call. The five-argument
  // bodies are still original, so each is reached through a pending_bodies
  // forwarder - exactly the arrangement draw's own four-argument overload
  // has, and for the same reason.
  int draw_dest(Buffer *buffer, int a, int b, int c, unsigned char *bits);
  int draw_dest(Buffer *buffer, int a, int b, int c, int x, int y,
                unsigned char *bits);
  int draw_dest_unk2(Buffer *buffer, int a, int b, int c, unsigned char *bits);
  int draw_dest_unk2(Buffer *buffer, int a, int b, int c, int x, int y,
                     unsigned char *bits);
  int draw_multi_table_dest(Buffer *buffer, int a, int b, int c,
                            unsigned char *bits);
  int draw_multi_table_dest(Buffer *buffer, int a, int b, int c, int x, int y,
                            unsigned char *bits);
  // DECLARED FOR BUFFER::draw_dest_source AND Buffer::draw_mono, homed into
  // buffer.cpp by the homing pass: those wrappers tail into these two the way
  // draw_dest tails into draw_dest above. Additive declarations only - no
  // offset moves, so no body in sprite.cpp changes a byte.
  int draw_dest_source(Buffer *buffer, int a, int b, int c, int d);
  int draw_dest_source(Buffer *buffer, int a, int b, int c, int d, int e,
                       unsigned char *bits);
  int draw_mono(Buffer *buffer, int a, int b, int c, int d);
  int draw_mono(Buffer *buffer, int a, int b, int c, int d, int e, int f);
  // Sprite-sheet extraction used by the per-control init_class bodies. The
  // body lives in sprite.cpp beside its marker. `extract`'s last parameter is
  // a texture-heap pointer most callers pass as null.
  int extract(Buffer *buffer, int a, int b, int c, int width, int height,
              TexHeap *heap);
  int create_blank(int width, int height, int depth);
  // 0x005E3890, still FILE-only in src/unrecovered/005e3890.cpp. DECLARED
  // here because BasePop::start (0x00601BF0) calls it by name on the popup's
  // own `sprite_` - the access costs nothing and the definition stays where
  // it is; pending_bodies.cpp carries the forwarder the call links against.
  int init(char *filename, int width, int height);
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
  // `Buffer::box_sprite` reads the same byte for every frame element it
  // draws and masks, and reads `iWidth_`/`iHeight_` to place the corners and
  // to inset the edges by. Access is compile-time only: no offset moves and
  // no emitted byte changes.
  friend class Buffer;
  // The frame overlay reads the same transparency byte and dimensions.
  friend class GraphicWin;
  // `BasePop::on_redraw` (0x00604AB0) centres and draws the two popup sprites
  // the class carries at 0x2148 and 0x214c, reading cTransparentIndex_,
  // iSpriteWidth_, iSpriteHeight_, iWidth_ and iHeight_ off them. Compile-time
  // access only.
  friend class BasePop;
  char *ppszFileName_;  // 0x0000 - heap copy of the source file's name;
                        // released with the CRT free
  char *pcBits_;        // 0x0004 - the pixel block, same allocator
  // UNSIGNED, and this is measured not stylistic: the image reads the byte
  // as `xor eax,eax; mov al,[sprite+8]` at every site that passes it to
  // `Sprite::draw` as an int (0x005E2E07 here, 0x005EFF02 in Win::flip). A
  // `char` compiles to `movsx`, which is a different instruction.
  unsigned char cTransparentIndex_;
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

/*
 * THE TERRAIN-ART SHEET, the band of Sprite storage 0x0075B098..0x007AD1B0
 * that `load_terrain` (0x00451C50) fills and the map renderer draws through.
 * Declared here because `gen_terrain_poly` (0x004632D0), `gen_tile`,
 * `gen_radius` and their neighbours reach every one of them by absolute
 * address from a different TU; the definitions and the 0x2c-stride comments
 * live in sprite.cpp beside the extract calls that size them.
 *
 * HOW THE RENDERER READS THEM, which is why the addresses below come in
 * +8 pairs. A Sprite is 0x2C bytes and its `cTransparentIndex_` is the byte
 * at +8, so a painter that has to hand a blitter the transparent colour
 * alongside the sprite reads exactly two things:
 *
 *     mov  ecx, 0x776a50                    ; &sprite           (the object)
 *     mov  ecx, dword ptr [0x776a58]        ; sprite.cTransparentIndex_
 *     and  ecx, 0xff
 *
 * and the indexed form over an array is the same pair with the 0x2c stride
 * folded in (`mov al, byte ptr [edx*4 + 0x77b000]` beside
 * `lea ecx, [edx*4 + 0x77aff8]`, edx = index * 11). Both spellings are
 * `&name[k].cTransparentIndex_` in source - no new name is needed for the
 * +8 address, which is why the list below is of OBJECTS only.
 */
extern Sprite g_TER1_WHITE_ORG_YEL_TILE_SPRITES[6];   // 0x00791C58
extern Sprite g_TER1_BOTTOM_LEFT_TILE_SPRITES[9];     // 0x007A99A0
extern Sprite g_TER1_MINE_SPRITE;                     // 0x00776A50
extern Sprite g_TER1_SOLAR_COLLECTOR_SPRITE;          // 0x007991F8
extern Sprite g_TER1_TIDAL_HARNESS_SPRITE;            // 0x0078A5B0
extern Sprite g_TER1_MINING_PLATFORM_SPRITE;          // 0x00789B68
extern Sprite g_TER1_KELP_FARM_SPRITE;                // 0x007A6860
extern Sprite g_TER1_CONDENSER_SPRITE;                // 0x00779390
extern Sprite g_TER1_ECHELON_MIRROR_SPRITE;           // 0x007ABF60
extern Sprite g_TER1_BOREHOLE_SPRITE;                 // 0x0078DD20
extern Sprite g_TER1_BOREHOLE_CLUSTER_SPRITE;         // 0x007ACB88
extern Sprite g_TER1_MANIFOLD_NEXUS_SPRITES[6];       // 0x007AC098
extern Sprite g_TER1WRECK_UNITY_WRECKAGE_SPRITES[15];  // 0x00776758
extern Sprite g_TER1WRECK_UNITY_WRECKAGE_ALT_SPRITES[4];  // 0x00799658
extern Sprite g_FOSSIL_FIELD_RIDGE_SPRITES[6];        // 0x007ACD70
extern Sprite g_TER1_FARM_SPRITES[5];                 // 0x00791FC8
extern Sprite g_TER1_SOIL_ENRICHER_SPRITES[5];        // 0x0079A710
extern Sprite g_TER1_SEA_LAND_RESOURCE_SPRITES[12];   // 0x0075B230
extern Sprite g_TER1_LANDMARK_RESOURCE_SPRITES[6];    // 0x0078A650
extern Sprite g_TER1_UNITY_POD_SPRITES[6];            // 0x0077AFF8
extern Sprite g_TER1_MONOLITH_SPRITE;                 // 0x0075B098
extern Sprite g_TER1_BUNKER_SPRITE;                   // 0x007AD010
extern Sprite g_TER1_AIRBASE_SPRITE;                  // 0x007ACC60
extern Sprite g_TER1_SENSOR_ARRAY_SPRITE;             // 0x007A6830

// Shared interface choices: DipEdit uses elements 1 and 2 for its checkboxes.
extern Sprite g_IFACE_CLOSE_X_SPRITES[3];

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
