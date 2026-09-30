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

#include "original_seam.h"
#include "graphicwin.h"
#include "texture.h"    // TextureStore textures_a_/textures_b_/texture_store_
#include "font.h"       // font_a_/font_b_/font_c_
#include "imagebutton.h"  // image_buttons_[4]
#include "maininterface.h"   // MainInterfaceGlobal is one of these

// Shared scenario-editor selection state. Console chooses the tool/brush;
// MapWin applies it to tiles. The mask helpers model the tools' terrain rules,
// not a pointer back into the original executable's UI descriptor table.
extern int MapEditorPaintId;
extern int MapEditorBrushId;
uint32_t map_editor_paint_mask(int tool_id);
uint32_t map_editor_erase_mask(int tool_id);

// One owned terrain-cache record per map tile. The resets at
// 46FDA9..46FDC4 and 46FDDC..46FDF8 walk twelve-byte records, writing
// bytes 0..3 and 4..8 separately. 4705AF/4705B5 identify FF as uncached
// brightness (80 encodes neutral zero); 46FF10 loads signed altitude.
// 470F3D..470F57 initializes byte 9 from the first map tile's climate,
// while 46FE4D reads the detail word. Names describe these uses, not
// recovered historical C++ spellings. No constructor or initialization
// of the untouched detail bytes is implied.
struct TerrainCacheEntry {
    // RECOVERY_MEMBER: v1 TerrainCacheEntry brightness 0x0 0x0046FD90
    unsigned char brightness[4];
    // RECOVERY_MEMBER: v1 TerrainCacheEntry altitude 0x4 0x0046FD90
    signed char altitude[5];
    // RECOVERY_MEMBER: v1 TerrainCacheEntry terrain_bits 0x9 0x00470ED0
    unsigned char terrain_bits;
    // RECOVERY_MEMBER: v1 TerrainCacheEntry altitude_detail 0xA 0x0046FE70
    unsigned short altitude_detail;
};
static_assert(sizeof(TerrainCacheEntry) == 12,
              "terrain cache records have twelve-byte stride");

// Tile-selection bounds shared by draw_map and gen_map, not pixel clipping.
// X is half-open and may wrap around the map seam; Y includes both endpoints.
// Original46AC00..46ACCD saves/restores each four-word region independently.
struct MapTileDamageRect {
    int left;
    int top;
    int right;
    int bottom;
};
static_assert(sizeof(MapTileDamageRect) == 16, "four signed tile bounds");

// The two panel rectangles selected by MapWin::init(0, ...) and filled by
// MapWin_UNK5. The original uses a 16-byte stride from 0x00939F48.
struct MapPanelRect {
  int left;
  int top;
  int right;
  int bottom;
};
static_assert(sizeof(MapPanelRect) == 16, "map panel rectangle stride");
extern MapPanelRect MapPanelRects[2];
extern uint32_t MapPanelDirtyMask;

 /*
  * MapWin class
  *
  * The original derives this from GraphicWin *virtually*, and it is written
  * that way. (The comment that used to stand here said it "cannot be written
  * as `: virtual GraphicWin`" because "the Itanium ABI this toolchain
  * follows" would place the base after the derived object; the declaration
  * below has said `: virtual GraphicWin` since d39dac11, VC6 is the only
  * compiler this tree targets, and VC6 puts the base exactly where the
  * vbtable names. The static_assert is what withdrew the claim.)
  *
  * The base is spelled `public` for the same class of reason graphicwin.h
  * spells its own base `public`: a virtual base is initialised by the MOST
  * DERIVED class, so `class Console : MapWin` has to reach GraphicWin's
  * constructor itself, and anything below `public` is C2243/C2244 on the
  * constructors VC6 emits wherever they are reached - and, measured
  * 2026-08-29, on control_game's `ConsoleGlobal->Win::hide()` in game.cpp,
  * the one free-function caller the image has on this path. (`protected`
  * stood here for a while under the theory encapsulation was free; the
  * virtual-base-adjusted call from outside the hierarchy is what it cost.)
  * Access changes no offset;
  * sizeof(MapWin) and sizeof(Console) are both unmoved by it.
  *
  * The original vbtable at 0x66C870 reads {0, 0x21A6C}. Original heap
  * construction at 0x58F177 allocates 0x22480 bytes for this class, and the
  * GraphicWin constructor's last DWORD store reaches the complete object's
  * offset 0x2247F. These bound the real FarmMapWindow as well; a gap before
  * the next known global does not enlarge its storage.
  *
  * The region before the virtual base contains the declared members and
  * explicitly unfinished fields below. Further recovery must establish each
  * member within this layout, not append storage and move the virtual base.
  */
class MapWin : public virtual GraphicWin {
  // setup_player5B334B/5B3356 writes its actual map-coordinate members.
  friend int __cdecl setup_player(int, int, int);
 public:
  // homed from 00463040.cpp
  int pixel_to_tile(int x_pixel, int y_pixel, int *out_tile_x,
                    int *out_tile_y);

 public:
  // homed from 00462980.cpp
  void calculate_dimensions();

 public:
  // homed from 0046fe00.cpp
  int get_relative_alt(int x, int y, int detail);

 public:
  // homed from 0046fce0.cpp
  void UNK7();

 public:
  // 0x0046F880, a pending_bodies forwarder.
  void on_sys_close();

 public:
  // THE ENTERED-RECEIVER WALK. The image's Q-spelled handlers (on_sys_close,
  // on_left_click, ...) are entered with `this` on the GraphicWin virtual
  // base and fold the walk back to the MapWin front into every access -
  // `mov eax, [ecx - 0x3cf4]` in on_sys_close, `add ecx, 0xfffde594` in
  // on_left_click. A Q member's source cannot say "this arrives as the
  // base", so the walk is spelled: pass the entered receiver, get the front.
  // The cast lands on the PARAMETER, not on `this`, which keeps the
  // class_debt raw-self-access census at its ceiling for a pun that is
  // measured and load-bearing.
  static MapWin *from_graphic_base(void *entered) {
    return reinterpret_cast<MapWin *>(
        static_cast<uint8_t *>(entered) - 0x21A6C);
  };

 public:
  // 0x0046EC10, a pending_bodies forwarder.
  void on_right_down(int a, int b);

 public:
  // 0x0046F700, a pending_bodies forwarder.
  void on_nc_left_down(int x_coord, int y_coord);

 public:
  // 0x0046F660, the custom non-client title-strip test; other points defer
  // to Win::on_nc_hittest through the real virtual base.
  int on_nc_hittest(int x, int y);

 public:
  // 0x0046F000: map cursor, route preview and hover feedback.
  // Win slot17; the catalogue called this on_mouse_move. Keep it distinct
  // from the inherited four-argument event router at slot16.
  virtual void on_mouse_feedback(int pixel_x, int pixel_y);
  void dest_line(int x1, int y1, int x2, int y2, int prototype,
                 int faction, int triad, int repeat, int pattern_on, int pattern_off,
                 int cover);
  void direct_dest_line(int x1, int y1, int x2, int y2,
                        int triad, int pattern_on, int pattern_off, int cover);
  void editor(int tile_x, int tile_y, int erase);

 public:
  // 0x0046EB90 delegates to go_reset, including its reentrant capture cleanup.
  // Win slot 4. MapWin, PlanWin and Console install compiler-generated
  // vtordisp thunks to this override (0x00421870/0x0048BF60/0x0051DE30).
  virtual void on_lose_mouse_capture();

 public:
  // 0x0046ED30, a pending_bodies forwarder.
  void on_left_down(int a1, int a2);

 public:
  // 0x0046F8C0, a pending_bodies forwarder.
  void on_button_clicked(int a1);

 public:
  // 0x0046B310, a pending_bodies forwarder.
  int focus(int x_coord, int y_coord);
  // 0x0046B1F0. Console::center and the editor's goto paths recenter the map
  // through the source-owned body in mapwin.cpp.
  void set_center(int x_coord, int y_coord, int draw_type);
  void gen_bases_only();
  void draw_bases(int tile_x, int tile_y, int pixel_x, int pixel_y,
                  int faction_id);

 public:
  // 0x0046D5D0, a pending_bodies forwarder. The two click handlers reach it
  // on a virtual-base-adjusted receiver and used to do so through a pointer.
  void click(int a1, int a2, int a3);

 public:
  // Win slot13, through generated MapWin/PlanWin/Console vtordisp thunks.
  virtual void on_resize(int width, int height);
  void on_redraw();
  // 0x00467970, a pending_bodies forwarder. line_timer (time.cpp) calls it
  // per live map-window slot with the literal 1.
  void draw_base_dest(int draw_type);
  // 0x0046A550: scroll the cached image and regenerate exposed tile regions.
  void draw_map(int draw_type);
  // Original radius controller and clipped terrain/vehicle/overlay scans.
  void draw_radius(int x, int y, int radius, int draw_type);
  void draw_radius(int x, int y, int draw_type);
  void gen_radius(int x, int y, int radius, int faction, int skip_terrain);
  void draw_vehicles(int x, int y, int radius, int faction);
  // Original4216F0: a genuine MapWin receiver, despite the seed's stdcall.
  int tile_in_view(int x, int y, int margin);
  // 0x0046FD90, resets brightness and altitude in the owned cache in two
  // complete passes. world_climate (src/map.cpp) calls it by name.
  void clear_terrain();
  // OVERLOAD BLOCK homed from the archive in the mapwin homing pass. Each
  // body lives beside its marker in mapwin.cpp.
  int compute_clip(int a1, int a2, int a3, RECT *a4);
  void draw_tile(int a1, int a2, int a3);
  void tile_to_pixel(int a1, int a2, int *a3, int *a4);
  void init_dummy();
  void init(int mode, int update);
  void init(RECT *bounds, char *caption, int flags);
  int get_alt(int a1, int a2, int a3);
  // 0x00470550 and 0x00470790, homed from the archive in the mapwin homing
  // pass. get_brighting returns one corner's cached brightness in the
  // twelve-byte terrain record owned_ points at (FF means "uncached";
  // 80 encodes neutral zero), computing it from the two
  // altitude-detail nibbles get_alt has just filled and the corner's run of
  // CornerLightTable. get_point_light walks those corners for a tile: corner
  // 0 averages all four, the rest sum the two neighbours the tile shares with
  // that corner, and both add the per-flag point-light delta the tile's
  // `bit2` byte and code select from AltDirectionTable.
  int get_brighting(int x, int y, int corner);
  int get_point_light(int x, int y, int corner);
  // 0x00470420 builds the four 7x7 corner-light tables consumed by
  // get_brighting. The generated bytes are owned by MapLightingTable.
  void compute_lighting_table();

  // THE GENERATION PAIR, homed 2026-09-21 from the original evidence
  // (journal discovery mapwin-gen-tile-map-20260921). gen_tile renders one
  // tile: tile_to_pixel transforms its coordinates IN PLACE on the parameter
  // homes (the pre-call pair survives in registers to become arguments 7/8
  // of gen_terrain_poly while the post-call pair becomes 9/10), and the
  // vehicle/overlay tail runs only for original coordinates that are on the
  // map with a nonzero fourth argument. gen_map drives the visible map in
  // three scans (terrain, vehicles, overlays) plus base names, legend and
  // wars; its bodies live beside their markers in mapwin.cpp.
  void gen_tile(int x, int y, int a3, int a4);
  void gen_map(int a1, int a2);
  // 0x00466CA0, still an original body (src/recovered/units/00466ca0.cpp):
  // declared so gen_tile's and gen_map's vehicle scan compile; the pushes at
  // 0x00468CA3..AD and 0x0046A023..32 fix this five-argument shape.
  void draw_vehicles(int x, int y, int pixel_x, int pixel_y, int draw_type);
  // 0x004688E0, no seed mapping yet: the name and the void(int, int) shape
  // come from the two call sites that push exactly (x, y) after gen_tile's
  // vehicle draw and inside gen_map's overlay scan.
  void gen_overlays(int x, int y);
  // 0x00467DE0 / 0x00469720 / 0x004698D0, still original bodies: declared so
  // gen_map's unconditional base-name pass and its 0x2000/0x20000/0x100000
  // gated legend and war passes compile.
  void draw_base_names();
  void draw_legend();
  void draw_wars();

  // ??0MapWin@@QAE@H@Z at 0x004626E0, spelled as the real constructor it is.
  // ATTEMPT #2. The first try built `MapWin(int a1)`, measured VC6 emitting
  // BOTH the hidden most-derived flag at [ebp+0xc] and a1 at [ebp+8] (every
  // caller pushing twice), and concluded "no constructor here at all" - but
  // it inverted the measurement it made. tools/most_derived_flag.py is
  // unambiguous about the body: the image's two vbtable-relative stores are
  // the tell no real `int` produces, and a bare constructor on a class with
  // a genuine virtual base receives the hidden flag AS its [ebp+8] argument,
  // mangled `??0MapWin@@QAE@XZ`. The image is stripped - docs/DECOMP_MAP.md
  // - and the catalogue's `H` is IDA's guess at that one stack dword, so the
  // two-fact rule carries the name mismatch: the marker keeps the catalogue
  // name, the `// symbol` line records what this compiler emits. The body
  // keeps only the scalar clears; the vbtable, the vbase construction, every
  // member/vector ctor call, and the three vtable stores the old hand body
  // spelled are the compiler's, from the declarations below.
  MapWin();
  // Original 0x420F90 destroys the own part, from the GraphicWin virtual-base
  // receiver. FarmMapWindow's registered exit then destroys that virtual base
  // separately. Genuine C++ supplies both stages for a complete MapWin, and
  // only the own part when destroying a MapWin base of Console/PlanWin.
  MEASURED ~MapWin() { clear(0); }
  int UNK1();
  void UNK3();
  // 0x0046FC80, homed from the archive. Walks RadiusRange/RadiusOffsetX/Y
  // (map.h) clearing only brightness. The original ignores x/y and uses
  // offsets around owned_ directly, including potentially negative indices.
  void UNK6(int UNUSED(x) x, int UNUSED(y) y, int radius);
  // 0x0046FB90, homed from the archive. Same three radius tables, walked to
  // reset the 0xC-stride visibility record `owned_` points at.
  void clear_alt(int x, int y, int radius);
  void UNK8(int x, int y, int radius);
  void do_image_buttons();
  void main_caption();
  void standard_caption();
  // 0x0046AD60, NOT the free ::draw_cursor at 0x0046AE00 that mapwin.cpp
  // defines. The cursor's own repaint runs per window and reads the shared
  // ConsoleGlobal cursor history plus this window's clip fields, so it
  // needs the receiver; Console::set_cursor, move_cursor,
  // prev_cursor and set_view all call it on the current map window. Body
  // still unrecovered - a pending_bodies forwarder.
  void draw_cursor();
  void on_left_click(int a1, int a2);
  void on_right_click(int a, int b);
  void on_left_double_click(int, int);
  void on_left_up(int, int);
  void close();
  int UNK2();
  // Resets the "this window is live" flag (MapWinActiveOffset) and other
  // per-window state; reached from this class's destructor, including the
  // compiler-generated MapWin destruction within Console and PlanWin.
  void clear(int index);

  // THE FIELD BLOCK IS PUBLIC, deliberately. go_reset (time.cpp, a free
  // function) resets several of these dwords on every window of the table,
  // and the mapwin_terrain_fixup / draw_map family reads 0x1DD74 from
  // outside through the documented raw-offset idiom; the offsets are
  // already carved members, so hiding the reset targets behind raw-offset
  // writes would model the same bytes twice. Access changes no offset; the
  // static_assert below still pins the layout.
  // RECOVERY_MEMBER: v3 MapWin owned_ TerrainCacheEntry* 0x4 0x0046FD90
  TerrainCacheEntry *owned_;
  uint32_t field_8_;  // 0x8
  // THE MANAGED OBJECTS, CARVED 2026-08-30 out of what was the
  // field_8_[0x1DD64] slab. The image's own ??_M vector-destructor calls
  // name the model: both TextureStore ranges carry ??1TextureStore
  // (0x006252B0) as their element destructor, so the elements ARE
  // TextureStore objects, and the arithmetic closes exactly -
  // 0xC + 4*0x260 = 0x98C and 0x98C + 0xC4*0x260 = 0x1DB0C, where the
  // lone store sits, ending at the already-named field_1DD6C_. The
  // destructors and constructors in mapwin.cpp spelled these same
  // addresses through VectorCtorIterator/VectorDtorIterator before the
  // carve; declaring the members hands the array lifetimes to the
  // compiler, which is what ??_DMapWin's own bytes are.
  TextureStore textures_a_[4];      // 0xC
  TextureStore textures_b_[0xC4];   // 0x98C
  TextureStore texture_store_;      // 0x1DB0C
  uint32_t field_1DD6C_;  // 0x1DD6C
  // on_resize suppresses image-button refresh when the high bit is set.
  // RECOVERY_MEMBER: v1 MapWin field_1DD70_ 0x1DD70 0x0046FA00
  uint32_t field_1DD70_;  // 0x1DD70
  uint32_t field_1DD74_;  // 0x1DD74
  uint32_t field_1DD78_;  // 0x1DD78
  // DWORD, not uint8_t[4]: the image writes it whole
  // (`mov dword ptr [entered - 0x3cf0], 0` in on_right_down), and the
  // byte-array form made that a member pun.
  uint32_t field_1DD7C_;  // 0x1DD7C
  uint32_t field_1DD80_;  // 0x1DD80
  uint32_t field_1DD84_;  // 0x1DD84
  uint32_t field_1DD88_;  // 0x1DD88
  uint32_t field_1DD8C_;  // 0x1DD8C
  uint32_t field_1DD90_;  // 0x1DD90
  uint32_t field_1DD94_;  // 0x1DD94
  uint32_t field_1DD98_;  // 0x1DD98
  uint32_t field_1DD9C_;  // 0x1DD9C
  uint32_t field_1DDA0_;  // 0x1DDA0
  // Tile origins and visible extents are signed: tile_to_pixel at0x00462F00
  // compares them using JL/JGE, including maps whose viewport crosses zero.
  // RECOVERY_MEMBER: v1 MapWin field_1DDA4_ 0x1DDA4 0x00462F00
  int field_1DDA4_;
  // RECOVERY_MEMBER: v1 MapWin field_1DDA8_ 0x1DDA8 0x00462F00
  int field_1DDA8_;
  uint32_t field_1DDAC_;  // 0x1DDAC
  uint32_t field_1DDB0_;  // 0x1DDB0
  uint32_t field_1DDB4_;  // 0x1DDB4
  uint32_t field_1DDB8_;  // 0x1DDB8
  uint32_t field_1DDBC_;  // 0x1DDBC
  uint32_t field_1DDC0_;  // 0x1DDC0
  uint32_t field_1DDC4_;  // 0x1DDC4
  uint32_t field_1DDC8_;  // 0x1DDC8
  // RECOVERY_MEMBER: v1 MapWin field_1DDCC_ 0x1DDCC 0x00462F00
  int field_1DDCC_;
  // RECOVERY_MEMBER: v1 MapWin field_1DDD0_ 0x1DDD0 0x00462F00
  int field_1DDD0_;
  // RECOVERY_MEMBER: v1 MapWin field_1DDD4_ 0x1DDD4 0x00462F00
  int field_1DDD4_;
  // RECOVERY_MEMBER: v1 MapWin field_1DDD8_ 0x1DDD8 0x00462F00
  int field_1DDD8_;
  uint32_t field_1DDDC_;  // 0x1DDDC
  uint32_t field_1DDE0_;  // 0x1DDE0
  // Route endpoint sequences. Mode8/count3 can write one endpoint beyond
  // each logical five-element sequence: original x[5] aliases y[0], and
  // y[5] aliases the hover-X cache below. The handler spells those physical
  // fields explicitly rather than performing out-of-bounds C++ accesses.
  // RECOVERY_MEMBER: v1 MapWin feedback_x_ 0x1DDE4 0x0046F000
  int feedback_x_[5];
  // RECOVERY_MEMBER: v1 MapWin feedback_y_ 0x1DDF8 0x0046F000
  int feedback_y_[5];
  // Cached hover X; go_reset invalidates it with -1, mouse feedback reads
  // and writes it. It also occupies the original y[5] endpoint address.
  // RECOVERY_MEMBER: v1 MapWin field_1DE0C_ 0x1DE0C 0x0046F000
  uint32_t field_1DE0C_;  // 0x1DE0C
  // RECOVERY_MEMBER: v1 MapWin feedback_hover_y_ 0x1DE10 0x0046F000
  int feedback_hover_y_;
  // RECOVERY_MEMBER: v1 MapWin feedback_proto_ 0x1DE14 0x0046F000
  int feedback_proto_;
  // RECOVERY_MEMBER: v1 MapWin feedback_triad_ 0x1DE18 0x0046F000
  int feedback_triad_;
  // RECOVERY_MEMBER: v1 MapWin feedback_repeat_ 0x1DE1C 0x0046F000
  int feedback_repeat_;
  // RECOVERY_MEMBER: v1 MapWin feedback_segment_count_ 0x1DE20 0x0046F000
  int feedback_segment_count_;
  // Base index used when no vehicle is selected.
  // RECOVERY_MEMBER: v1 MapWin field_1DE24_ 0x1DE24 0x0046F000
  int field_1DE24_;
  // THE THREE BUFFERS, carved from what was field_1DE28_[0x1098]: three
  // consecutive 0x588-byte Buffer objects, ??1Buffer@front+0x1E938/
  // +0x1E3B0/+0x1DE28 in ??_DMapWin's own call list. Declared in the
  // order destruction reverses (highest address first), so the compiler's
  // member teardown runs buffer_a_, buffer_b_, buffer_c_ - the image's
  // order.
  Buffer buffer_c_;  // 0x1DE28
  Buffer buffer_b_;  // 0x1E3B0
  Buffer buffer_a_;  // 0x1E938
  uint32_t field_1EEC0_;  // 0x1EEC0
  uint32_t field_1EEC4_;  // 0x1EEC4
  Font font_a_;  // 0x1EEC8 - three Fonts, ??1Font calls at these offsets in
                 // ??_DMapWin; each followed by one dword the map named.
  uint32_t field_1EEF0_;  // 0x1EEF0
  Font font_b_;  // 0x1EEF4
  uint32_t field_1EF1C_;  // 0x1EF1C
  Font font_c_;  // 0x1EF20
  uint32_t field_1EF48_;  // 0x1EF48
  // Borrowed cursor cache: Console::cursor_on stores a g_CURSOR_SPRITES
  // element here; resource-cursor resets clear it without destroying a Sprite.
  // RECOVERY_MEMBER: v3 MapWin cursor_sprite_ Sprite* 0x1EF4C 0x00510400
  Sprite *cursor_sprite_;  // 0x1EF4C
  // RECOVERY_MEMBER: v1 MapWin cursor_resource_id_ 0x1EF50 0x00462870
  uint32_t cursor_resource_id_;  // 0x1EF50
  // THE IMAGE BUTTONS: 4 * 0xABC (BaseButton 0xAB8 + the field imagebutton.h
  // names) = the 0x2AF0 this replaces, ending at 0x21A44 where the dword run
  // the map already names picks up. The image's ??_M arg here is
  // ??1ImageButton (0x00625310).
  ImageButton image_buttons_[4];  // 0x1EF54
  uint32_t field_21A44_;  // 0x21A44
  // RECOVERY_MEMBER: v1 MapWin horizontal_damage_ 0x21A48 0x0046A550
  MapTileDamageRect horizontal_damage_;
  // RECOVERY_MEMBER: v1 MapWin vertical_damage_ 0x21A58 0x0046A550
  MapTileDamageRect vertical_damage_;

 protected:
  // NOT MapWin data, and the only member here a derived class may name.
  //
  // MapWin's own data stops at 0x21A68: the highest access any MapWin method
  // makes is ?gen_map@MapWin@@QAEXHH@Z's four-byte `[esi + 0x21a64]` at
  // 0x00469D9A, and nothing in the class reads or writes 0x21A68 at all.
  // ??0MapWin@@QAE@H@Z touches it only through the vtordisp idiom - `mov eax,
  // [esi]` / `mov eax, [eax+4]` / `lea edx, [eax - 0x21a6c]` / `mov [eax + esi
  // - 4], edx`, i.e. [vbase - 4] - so in the image these four bytes are the
  // vtordisp MSVC reserves immediately ahead of a virtual base, and they are
  // MapWin's only while MapWin is the most-derived object.
  //
  // A class that derives MapWin moves the virtual base, and the vtordisp with
  // it, so the derived class's own first field lands here instead: PlanWin
  // puts its vtordisp at 0x2204C (`mov [eax + esi - 4], edx` at 0x0048BD5D,
  // eax = 0x22050) and writes 0x21A68 as data at 0x0048BD67, reads it in
  // ?blink@PlanWin@@QAEXXZ at 0x0048BC20 and clears it in
  // ?close@PlanWin@@QAEXXZ at 0x0048BC50.
  //
  // Neither MapWin nor anything derived from it declares a virtual function
  // here, so VC6 emits no vtordisp of its own and this member is what holds
  // the virtual base at the 0x21A6C the vbtable names. It therefore sits
  // inside the MapWin subobject of every derived class, one dword further out
  // than the original put the boundary, and the derived class has to reach
  // its field through it. `int32_t`, not `uint8_t[4]`, because every access
  // the image makes to it is a four-byte one.
  // THE COMPILER OWNS THIS DWORD NOW. It was modelled by hand as an
  // `int32_t` because MapWin declared no virtual function, so VC6 emitted no
  // vtordisp and something had to hold the virtual base at the 0x21A6C the
  // vbtable names. MapWin's methods override Win's virtuals now, so VC6
  // emits the real vtordisp here itself - two of them would push the vbase
  // four bytes out and break every offset past it.
};

static_assert(sizeof(MapWin) == 0x22480, "MapWin layout must match terranx.exe");


// 0x004710E0; desktop_close tears the map-window system down between the
// window closes and the final hide. The source-owned body is in mapwin.cpp.
void __cdecl mapwin_system_shutdown();
void __cdecl mapwin_system_init(int reset_windows);
void __cdecl mapwin_activate_free_slot(int tile_x, int tile_y);


// The heap pointer at 0x4 is freed through the game's CRT boundary, the same
// address buffer.cpp frees through. Rebindable so tests observe it.
typedef void *func_map_win_free(void *);


// The original main_game publisher enables input at4C49ED. Its unfinished
// startup path is a dependency; this flag's initial image value is zero.
extern int MapInputEnabled;
extern int MapMouseFeedbackBusy;
// Produced by MapWin::dest_line and displayed by mouse feedback.
extern int MapDestinationCost;


// The map-window slot array the free drawing helpers broadcast to: eight
// MapWin* in the image's zero-initialised .data at 0x007D3C3C, ending at the
// exclusive 0x007D3C5C that draw_tile compares against
// (`cmp esi, 0x7d3c5c` at 0x0046AF79 -> (0x7D3C5C - 0x7D3C3C) / 4 == 8).
// The element type is pinned by ?mapwin_system_init@@YAXH@Z at 0x0047107F,
// `mov ecx, dword ptr [esi*4 + 0x7d3c3c]`, and by ?zoom@BaseWin@@QAEXHH@Z at
// 0x0041AAC3, which walks slot 0's vbtable. Populated at run time, so this is
// a mutable global. system_init installs seven allocated MapWins and the
// actual ConsoleGlobal in slot zero; all consumers use the same storage.
//
// TRIED: an accessor returning `reinterpret_cast<MapWin **>(0x007D3C3C)`
// is a pointer value and measures WORSE than the `MapWin **const` it replaced
// - 19 of 85 instructions in `Console::focus` against 29.
typedef MapWin *MapWinSlots[8];
extern MapWinSlots MapWinTable;
// StatusWin clears this signed vehicle ID to -1 when no route destination
// remains. MapWin::set_center uses it to redraw a selected destination.
extern int DestinationVehicleId;
// One most-derived off-screen window, not an alias into the old executable.
extern MapWin FarmMapWindow;
static const size_t MapWinTableSlots = 8;        // (0x007D3C5C - 0x007D3C3C) / 4

// 0x0068A5CC. THE SLOT OF THE WINDOW BEING WORKED WITH, or -1 for none.
// go_reset (0x0050EF50) proves the pairing with the table above: its walk
// skips the slot whose index equals this (`cmp ecx, edx / je` at 0x0050EF9C,
// the index and this side by side), then resets THIS slot separately at
// 0x0050F083 (`mov edi, [eax*4 + 0x7d3c3c]`) and stores -1 back at the end
// (0x0050F155). Every read is a signed compare against zero (`jl` at
// 0x0050F07D, and 0x00517BF6 the same), so `int`, not unsigned.
extern int MapWinSelectedSlot;  // 0x0068A5CC

// Shared map/caption zoom override; nonpositive scale uses the window zoom.
// FontQueue::get_zoom also uses this pair to select its cached font height.
extern int MapWinCaptionScale;    // 0x00691E6C, initially -1
extern int MapWinCaptionDivisor;  // 0x00691E70, initially -1

// Per-window "this window is live" dword. The MapWin constructor clears it
// (0x00462822), MapWin::clear writes it (0x004628AF), and mapwin_system_init
// sets it per slot (0x00471090, 0x0047109E). Every free drawing helper gates
// on it for slots 1..7. MapWin now declares this as field_1DD74_; the offset
// remains for older, not-yet-migrated consumers. New code uses the member.
static const size_t MapWinActiveOffset = 0x1DD74;

// Original4712D0 performs a nullable MapWin-to-Buffer base conversion before
// entering the common terrain renderer. It does not own another canvas.
int __cdecl gen_window_terrain(MapWin *window, int zoom, int faction,
                              int base_id, int flags, int x, int y,
                              int pixel_x, int pixel_y);

// Free functions, __cdecl (the `ret` at 0x0046AF85 / 0x0046B185 pops nothing).
// Redirected directly with no adapter; see src/mapwin.cpp.
// ?draw_map@@YAXH@Z at 0x0046B190 - the FREE function that repaints every
// live MapWin, not `MapWin::draw_map` at 0x0046A550 below. Declared here
// rather than bound as a pointer in temp.h so its callers emit the `E8` the
// image emits; defined at the end of src/mapwin.cpp.
void __cdecl draw_map(int draw_type);

// ?terrain_fixup@@YAXPAUMapWin@@@Z at 0x004711A0 - not yet recovered, a
// pending_bodies forwarder. mapwin_terrain_fixup (end of mapwin.cpp) calls
// it once per live MapWin slot and once more for each of three
// fixed-address windows.
void __cdecl terrain_fixup(MapWin *window);

// ?gen_terrain_poly@MapWin@@QAAHPAUBuffer@@PAUMapWin@@HHHHHHHH@Z at
// 0x004632D0 - the terrain polygon renderer. TRANSCRIBED 2026-09-21 into
// src/mapwin.cpp (journal 56, terrain-poly-transcription-scope-20260921):
// the 44 blitter calls are __thiscall member calls on the terrain-band
// globals, and the catalogued member mangle above stays the identity of
// record for the span while the body is the free __cdecl function the two
// compiled call sites (gen_tile at 0x00468C73 and gen_map's terrain scan
// at 0x00469E47) spell - they push exactly ten words with the window as
// the SECOND argument and clean up 0x28 bytes themselves, so no implicit
// receiver rides the call. Arguments 3..10 are, in order, field_1DD98_,
// the caller's draw_type, the literal -1, field_1DD70_, the original tile
// x, the original tile y, and the pair tile_to_pixel wrote back.
int __cdecl gen_terrain_poly(Buffer *buffer, MapWin *window, int a3,
                             int a4, int a5, int a6, int a7, int a8,
                             int a9, int a10);

// The terrain producer and its map/farm consumers share these four-corner
// arrays. They are not aliases of the adjacent primary-view rectangle.
extern Vert MapCornerCopy[4];
extern Vert MapCornerScratch[4];

MEASURED inline void __cdecl draw_tile(int x_coord, int y_coord, int draw_type) {
    for (size_t slot = 0; slot < MapWinTableSlots; ++slot) {
        // Re-read every iteration, as `mov ecx, dword ptr [esi]` at 0x0046AF51
        // does: a callee that rewrites the table is seen by later slots.
        MapWin *const window = MapWinTable[slot];
        // 0x0046AF53 test / 0x0046AF55 je - the null test comes first, so an
        // empty slot 0 draws nothing despite the exemption below.
        if (window == nullptr) {
            continue;
        }
        // 0x0046AF57 `cmp esi, 0x7d3c3c` / 0x0046AF5D je. The cursor only ever
        // walks forward from the table base, so comparing it against that base
        // is exactly "is this slot index 0?" - the primary map window is
        // exempt from the activity gate.
        if (slot != 0) {
            const uint32_t active = *reinterpret_cast<const volatile uint32_t *>(
                reinterpret_cast<const uint8_t *>(window) + MapWinActiveOffset);
            if (active == 0) {
                continue;
            }
        }
        // 0x0046AF71: __thiscall on the slot value still live in ECX, with the
        // pushes at 0x0046AF6C..0x0046AF70 giving the stack order
        // (x_coord, y_coord, 0, draw_type) - the pushes run right to left, so
        // the last one (`push eax`, [ebp+8]) is the first stack argument. The
        // literal 0 is this function's discriminator; see draw_tiles.
        // THE METHOD, not the seam directly - the image's own call site is
        // `call 0x46a2a0`, a direct rel32; the free `draw_radius` (mapwin.cpp)
        // is the worked example of the same lever.
        window->draw_radius(x_coord, y_coord, 0, draw_type);
    }
}

MEASURED inline void __cdecl draw_tiles(int x_coord, int y_coord, int draw_type) {
    for (size_t slot = 0; slot < MapWinTableSlots; ++slot) {
        // 0x0046B151, mirroring 0x0046AF51.
        MapWin *const window = MapWinTable[slot];
        // 0x0046B153 test / 0x0046B155 je.
        if (window == nullptr) {
            continue;
        }
        // 0x0046B157 `cmp esi, 0x7d3c3c` / 0x0046B15D je.
        if (slot != 0) {
            const uint32_t active = *reinterpret_cast<const volatile uint32_t *>(
                reinterpret_cast<const uint8_t *>(window) + MapWinActiveOffset);
            if (active == 0) {
                continue;
            }
        }
        // 0x0046B171, with `push 1` at 0x0046B16D supplying the radius.
        // THE METHOD, not the seam directly - same lever as draw_tile above.
        window->draw_radius(x_coord, y_coord, 1, draw_type);
    }
}
