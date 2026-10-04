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
class GraphicWin;
class Texture;
class CheckBox;  // controls may draw their selected-state sprites
class RadioButton;  // controls may draw their selected-state sprites

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
  // MainInterface's panel initialization adjusts displayed dimensions.
  friend class MainInterface;
 public:
  // Original5E4ADA is a genuine seven-argument zero-return entry.
  // Buffer5D8740 supplies its actual Buffer destination and six scalars;
  // the scalar meanings and descriptive method name remain interpretations.
  // Superseded header interpretation (history only):
  // The remaining UNK methods are EVIDENCE-FREE: their empty bodies do not
  // establish original names or parameter meanings. UNK1 returns zero and
  // pops seven arguments; identical bytes alone do not establish ownership.
  // The five-argument zero-return hook below also has no original name, but
  // Buffer's 5D8650 call supplies the actual shared Sprite receiver and a
  // Buffer destination, so that specific call contract is recoverable.
  // UNK3 and UNK4 have EMPTY bodies, so their two ints are unused and
  // nothing in the body can name them either.
  int draw_noop(Buffer *destination, int b, int c, int d, int e, int f, int g);
  // Original 5E4AEA returns zero; Buffer 5D8650 supplies the destination.
  // Descriptive provisional name, not a recovered original source spelling.
  int draw_noop(Buffer *destination, int transparent, int x, int y, int table);
  int draw(Buffer *destination, int transparent, int x, int y,
           int numerator, int denominator);
  int draw(Buffer *destination, int transparent, int x, int y,
           int numerator, int denominator, unsigned char *table);
  int draw(Buffer *destination, int transparent, int x, int y);
  // Source remap; the historical four-row expansion maps destination instead.
  int draw(Buffer *destination, int transparent, int x, int y,
           unsigned char *table);
  // Terrain painters pass the stored unsigned palette key to draw().
  unsigned char transparent_index() const { return cTransparentIndex_; }
  int width() const { return iWidth_; }
  int height() const { return iHeight_; }
  // Pixel width before the displayed dimensions/scaling are applied (+0x10).
  int source_width() const { return iSpriteWidth_; }
  int source_height() const { return iSpriteHeight_; }
  int left_offset() const { return iLeftOffset_; }
  int top_offset() const { return iTopOffset_; }
  // Zero source pixels remap the destination; other nontransparent pixels
  // retain their source colour. Wider overloads temporarily replace N / D.
  int draw_dest(Buffer *destination, int transparent, int x, int y,
                unsigned char *table);
  int draw_dest(Buffer *destination, int transparent, int x, int y,
                int numerator, int denominator, unsigned char *table);
  // Destination remap with no transparency key of its own: a zero source
  // byte sends the destination pixel through the table and every other
  // destination pixel is left untouched. The six-argument overload
  // substitutes a temporary scale, like the wider draw overloads.
  int draw_dest_unk1(Buffer *destination, int x, int y, unsigned char *table);
  int draw_dest_unk1(Buffer *destination, int x, int y, int numerator,
                     int denominator, unsigned char *table);
  int draw_dest_unk2(Buffer *buffer, int a, int b, int c, unsigned char *bits);
  int draw_dest_unk2(Buffer *buffer, int a, int b, int c, int x, int y,
                     unsigned char *bits);
  int draw_multi_table_dest(Buffer *buffer, int a, int b, int c,
                            unsigned char *bits);
  int draw_multi_table_dest(Buffer *buffer, int a, int b, int c, int x, int y,
                            unsigned char *bits);
  // Buffer's software drawing wrappers borrow the shared Sprite descriptor.
  // The catalogue's five-argument form takes the transparency key and the
  // remap table (`HHHPAE`), not four ints.
  int draw_dest_source(Buffer *buffer, int transparent, int x, int y,
                       unsigned char *table);
  int draw_dest_source(Buffer *buffer, int a, int b, int c, int d, int e,
                       unsigned char *bits);
  int draw_mono(Buffer *destination, int transparent, int x, int y, int color);
  int draw_mono(Buffer *destination, int transparent, int x, int y, int color,
                int numerator, int denominator);
  // Sprite-sheet extraction used by the per-control init_class bodies. The
  // body lives in sprite.cpp beside its marker. `extract`'s last parameter is
  // a texture-heap pointer most callers pass as null.
  int extract(Buffer *buffer, int a, int b, int c, int width, int height,
              TexHeap *heap);
  int extract(Buffer *buffer, int transparent, RECT *region, TexHeap *heap);
  int create_blank(int width, int height, int depth);
  // Copy this sprite's pixels/metadata into the destination, not vice versa.
  int copy(Sprite *destination);
  int change_color(int old_color, int new_color);
  // The Filemap-backed persistence pair; the image spells the single
  // parameter `int` and hands it to Filemap::create/open as the mapped file's
  // name. load() parses into the EXISTING pixel block: its head is close()'s
  // cleanup, so pcBits_ only survives it when fObj1Exists_ marks the pixels
  // borrowed - that is the caller domain the 0x19-byte header parse runs in.
  int save(int filename);
  int load(int filename);
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
  void fill_scaling_table();
  int *get_clipped_x_table_expand(RECT *full, RECT *clipped, int *source_skip,
      int *destination_skip, int *source_count, int *destination_count);
  int *get_clipped_y_table_expand(RECT *full, RECT *clipped, int *source_skip,
      int *destination_skip, int *source_count, int *destination_count);
  int *get_clipped_x_table_shrink(RECT *full, RECT *clipped, int *destination_skip,
      int *source_skip, int *destination_count, int *source_count);
  int *get_clipped_y_table_shrink(RECT *full, RECT *clipped, int *destination_skip,
      int *source_skip, int *destination_count, int *source_count);
 private:
  friend int __cdecl veh_draw(Buffer *, int, int, int, int, int, int);
  // Cursor initialization reads pixels, dimensions and transparency.
  friend class Cursor;
  friend class CheckBox;
  friend class RadioButton;
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
  // RECOVERY_MEMBER: v1 Sprite cTransparentIndex_ 0x8 0x00408720
  unsigned char cTransparentIndex_;
  char pad1_;
  char pad2_;
  char pad3_;
  int iSpriteWidth2_;
  // RECOVERY_MEMBER: v1 Sprite iSpriteWidth_ 0x10 0x006253D0
  int iSpriteWidth_;
  int iSpriteHeight_;
  // RECOVERY_MEMBER: v1 Sprite iWidth_ 0x18 0x00408B20
  int iWidth_;
  // RECOVERY_MEMBER: v1 Sprite iHeight_ 0x1C 0x00408B20
  int iHeight_;
  int iLeftOffset_;
  int iTopOffset_;
  int fObj1Exists_;
};

static_assert(sizeof(Sprite) == 0x2C, "Sprite layout must match the legacy ABI");

// Vehicle and project icon tables used by ProdPicker. Definitions and their
// bounded extents are owned by sprite.cpp.
extern Sprite g_FACILITY_ICON_SPRITES[70];
extern Sprite g_SECRET_PROJECT_ICON_SPRITES[64];

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
extern Sprite g_RAINFALL_DOUBLE_TILE_SPRITES[2];
extern Sprite g_GLOW_SPRITES[2];
extern Sprite g_TER1_UNUSED_SPRITES2[2];
extern Sprite g_TER1_TUT_BLANK_SPRITE;
extern Sprite g_TER1_UNUSED_SPRITES1[5];

// Shared interface choices: DipEdit uses elements 1 and 2 for its checkboxes.
extern Sprite g_IFACE_CLOSE_X_SPRITES[3];

// Existing resource/control assets populated by load_icons. No second
// storage or per-consumer copies: original counts are owned in sprite.cpp.
extern Sprite g_ICONS_GENERAL_SPRITES[16];
extern Sprite g_RESOURCE_ICON_SPRITES[32];
extern Sprite g_RED_ALIEN_HEAD_ICON_SPRITE;
extern Sprite g_SILVER_MENU_ICON_SPRITES[4];
extern Sprite g_SILVER_CHECKBOX_ICON_SPRITES[2];
extern Sprite g_RED_MALE_HEAD_ICON_SPRITE;
extern Sprite g_NULL_RESOURCE_ICON_SPRITE;
extern Sprite g_PEACE_SIGN_SPRITES[2];
extern Sprite g_ICON_TILE_SQUARE_SPRITE;
extern Sprite g_SCROLL_BAR_ARROW_ICON_SPRITES[12];
extern Sprite g_SCROLL_BAR_SMALL_ARROW_ICON_SPRITES[12];
extern Sprite g_SCROLL_BAR_FILLER_ICON_SPRITES;
extern int InterfaceCaptionHeight;
extern int InterfaceBorderThickness;
extern int InterfaceBottomBorderThickness;
extern int InterfaceAltCaptionHeight;
extern int InterfaceAltBorderThickness;
extern int InterfaceAltBottomBorderThickness;
extern int InterfaceThirdCaptionHeight;
extern int InterfaceThirdBottomBorderThickness;
extern int InterfaceFourthBorderThickness;
extern int InterfaceFourthBottomBorderThickness;
extern int InterfaceDefaultMetric;
extern int InterfaceTopLeftMetric;
extern int InterfaceMidRightMetric;
extern Sprite g_IFACE_GENERAL_WINDOWS_TOP_LEFT_SPRITE;
extern Sprite g_IFACE_GENERAL_WINDOWS_TOP_RIGHT_SPRITE;
extern Sprite g_IFACE_GENERAL_WINDOWS_BOT_LEFT_SPRITE;
extern Sprite g_IFACE_GENERAL_WINDOWS_BOT_RIGHT_SPRITE;
extern Sprite g_IFACE_GENERAL_WINDOWS_TOP_MID_SPRITE;
extern Sprite g_IFACE_GENERAL_WINDOWS_BOT_MID_SPRITE;
extern Sprite g_IFACE_GENERAL_WINDOWS_MID_LEFT_SPRITE;
extern Sprite g_IFACE_GENERAL_WINDOWS_MID_RIGHT_SPRITE;
extern Sprite g_IFACE_GENERAL_WINDOWS_NONCAP_LEFT_SPRITE;
extern Sprite g_IFACE_GENERAL_WINDOWS_NONCAP_MID_SPRITE;
extern Sprite g_IFACE_GENERAL_WINDOWS_NONCAP_RIGHT_SPRITE;
extern Sprite g_BASEWIN_SPRITES[27];
extern Sprite g_UNUSED_SPRITE_VAR04;
extern Sprite g_UNUSED_SPRITE_VAR08;
extern Sprite g_UNUSED_SPRITE_VAR13;
extern Sprite g_UNUSED_SPRITE_VAR16;
extern Sprite g_UNUSED_SPRITE_VAR27;
extern Sprite g_UNUSED_SPRITE_VAR29;
extern Sprite g_UNUSED_SPRITE_VAR30;
extern Sprite g_UNUSED_SPRITE_VAR31;
extern Sprite g_SCROLL_BAR_SMALL_FILLER_ICON_SPRITE;

// Existing interface assets populated by load_box_sprites; each numbered
// array supplies the corresponding corner/edge of all 51 panel styles.
extern Sprite g_IFACE_BOX_SPRITES1[51];
extern Sprite g_IFACE_BOX_SPRITES2[51];
extern Sprite g_IFACE_BOX_SPRITES3[51];
extern Sprite g_IFACE_BOX_SPRITES4[51];
extern Sprite g_IFACE_BOX_SPRITES5[51];
extern Sprite g_IFACE_BOX_SPRITES6[51];
extern Sprite g_IFACE_BOX_SPRITES7[51];
extern Sprite g_IFACE_BOX_SPRITES8[51];
extern Sprite g_IFACE_LOCK_SPRITES[2];
extern Sprite g_IFACE_GREEN_RIGHT_ARROW_SPRITE[1];
extern Sprite g_IFACE_MP_COMBO_ARROW_SPRITES[3];
extern Sprite g_IFACE_TECH_TREE_ARROW_SPRITES[3];
extern Sprite g_IFACE_STD_POPUPS_TOP_LEFT_SPRITE;
extern Sprite g_IFACE_STD_POPUPS_TOP_MID_SPRITE;
extern Sprite g_IFACE_STD_POPUPS_TOP_RIGHT_SPRITE;
extern Sprite g_IFACE_STD_POPUPS_MID_LEFT_SPRITE;
extern Sprite g_IFACE_STD_POPUPS_MID_RIGHT_SPRITE;
extern Sprite g_IFACE_STD_POPUPS_BOT_LEFT_SPRITE;
extern Sprite g_IFACE_STD_POPUPS_BOT_MID_SPRITE;
extern Sprite g_IFACE_STD_POPUPS_BOT_RIGHT_SPRITE;

// Running total of Sprite-owned bytes. The constructor adds one object; the
// release paths subtract the object plus its pixel buffer. The image holds
// the counter in .bss at 0x009B6618 - zero at load - so the tree owns it as
// a real zero-initialised global (sprite.cpp) rather than a raw address.
extern int SpriteMemoryUsed;

// Sprite allocations come from the executable's CRT, so they must be released
// through its free rather than this module's. Tests outside the hybrid process
// rebind this.
typedef void *func_sprite_free(void *);



// Uniform scaling ratio, temporarily substituted by the wider draw overloads.
// These are not independent horizontal/vertical origins: both axes use N / D.
extern int SpriteScaleNumerator;    // 0x00696D18, initially 1
extern Sprite g_VEH_SPRITES[152];        // 0x007777A0
extern Sprite g_FLAGS_VEH_SPRITES[112];  // 0x0078B778
extern Sprite g_XI_BOOM_VEH_SPRITES[144];
extern Sprite g_XF_BOOM_VEH_SPRITES[64];
extern int SpriteScaleDenominator;  // 0x00696D1C, initially 1
extern int SpriteCachedScaleNumerator;    // 0x00696D20, initially -1
extern int SpriteCachedScaleDenominator;  // 0x00696D24, initially -1
// load_cursors populates these existing sprite objects. Human citizens have
// four moods in each of two variants; alien citizens have one four-mood set.
extern Sprite g_CURSOR_SPRITES[12];
extern Sprite g_CITIZEN_LG_CURSOR_SPRITES[8];
extern Sprite g_SPECIALIST_LG_CURSOR_SPRITES[7];
extern Sprite g_CITIZEN_SM_CURSOR_SPRITES[8];
extern Sprite g_SPECIALIST_SM_CURSOR_SPRITES[7];
extern Sprite g_AL_CITIZEN_LG_CURSOR_SPRITES[4];
extern Sprite g_AL_SPECIALIST_LG_CURSOR_SPRITES[7];
extern Sprite g_AL_CITIZEN_SM_CURSOR_SPRITES[4];
extern Sprite g_AL_SPECIALIST_SM_CURSOR_SPRITES[7];

extern float SpriteScalingRatio;          // 0x009B6624
extern int SpriteMinimumScaledWidth;     // 0x009B661C
extern int SpriteMinimumScaledHeight;    // 0x009B6620
enum { SpriteScalingTableCount = 800 };
extern int SpriteScalingSteps[SpriteScalingTableCount];   // 0x009B3AD8
extern int SpriteClippedXSteps[SpriteScalingTableCount];  // 0x009B53EC
extern int SpriteClippedYSteps[SpriteScalingTableCount];  // 0x009B4760

// Original draw callback and shared PCX scratch-buffer extraction helpers.
void __cdecl sprite_draw_func(Sprite *sprite, GraphicWin *owner, int unused, int x, int y);
extern "C" {
int __cdecl sub_450de0(Sprite *sprite, int x, int y);
int __cdecl sub_450e30(Sprite *sprite, int x, int y);
int __cdecl sub_450e80(Sprite *sprite, int x, int y);
int __cdecl sub_450ed0(Sprite *sprite, int x, int y, int width, int height);
void __cdecl sub_450f30(Sprite *sprite, int x, int y, int width, int height);
int __cdecl sub_450f90(Texture *texture, int x, int y);
}
