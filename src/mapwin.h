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
#include "maininterface.h"   // MainInterfaceGlobal is one of these

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
  * The size is the vbtable's, not a guess: the table at 0x0066C870 reads
  * {0, 0x21A6C}, placing the virtual base at 0x21A6C, and GraphicWin is
  * already pinned at 0xA14. Those sum to the 0x22480 asserted below.
  * Independently, g_MAPWIN's global slot bounds the object above at 0x22798,
  * which is consistent with 0x318 to spare - two readings sharing no evidence.
  *
  * The 0x21A6C bytes ahead of the base are not mapped. Fields must be carved
  * out of derived_storage_ as methods are recovered, keeping the total fixed,
  * rather than appended - appending would move the virtual base and break
  * every offset in the class.
  */
class MapWin : public virtual GraphicWin {
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
  void on_nc_left_down(int a1, int a2);

 public:
  // 0x0046F660, a pending_bodies forwarder.
  int on_nc_hittest(int x, int y);

 public:
  // 0x0046F000, a pending_bodies forwarder.
  void on_mouse_move(int a1, int a2);

 public:
  // 0x0046EB90, a pending_bodies forwarder.
  void on_lose_mouse_capture();

 public:
  // 0x0046ED30, a pending_bodies forwarder.
  void on_left_down(int a1, int a2);

 public:
  // 0x0046F8C0, a pending_bodies forwarder.
  void on_button_clicked(int a1);

 public:
  // 0x0046B310, a pending_bodies forwarder.
  int focus(int x_coord, int y_coord);

 public:
  // 0x0046D5D0, a pending_bodies forwarder. The two click handlers reach it
  // on a virtual-base-adjusted receiver and used to do so through a pointer.
  void click(int a1, int a2, int a3);

 public:
  void on_resize(int a1, int a2);
  void on_redraw();
  // 0x00467970, a pending_bodies forwarder. line_timer (time.cpp) calls it
  // per live map-window slot with the literal 1.
  void draw_base_dest(int draw_type);
  // ?draw_map@MapWin@@QAEXH@Z at 0x0046A550, still an original body: declared
  // here so on_redraw's direct call compiles, resolved at link time.
  void draw_map(int draw_type);
  // ?draw_radius@MapWin@@QAEXHHHH@Z at 0x0046A2A0, still an original body -
  // same idiom as draw_map above, declared so the FREE draw_radius (end of
  // mapwin.cpp) reaches it with a `call rel32` instead of loading
  // MapWinOriginalDrawRadius through an indirect call.
  void draw_radius(int x_coord, int y_coord, int a3, int draw_type);
  // 0x0046FD90, not yet recovered - a pending_bodies forwarder.
  // world_climate (src/map.cpp) calls it BY NAME.
  void clear_terrain();
  // MEASURED: a genuine `MapWin(int a1)` constructor was tried first, on the
  // theory the mangled `H` on `??0MapWin@@QAE@H@Z` is VC6's own
  // most-derived flag for the virtual `GraphicWin` base declared above,
  // needing no parameter here at all. Built and disassembled with this
  // project's own `cl`: a class that genuinely has a virtual base gets the
  // hidden flag INSTEAD of a name change, not IN ADDITION to one - a bare
  // `MapWin()` mangles `??0MapWin@@QAE@XZ` (no `H`), and adding an explicit
  // `int a1` alongside the real virtual base makes VC6 emit BOTH: the flag
  // arrives at [ebp+0xc], a1 at [ebp+8], and a caller doing
  // `->MapWin::MapWin(1)` pushes 1 twice - the exact defect this file used
  // to warn about, now reproduced and confirmed rather than assumed. So: no
  // constructor here at all (the implicit default is never called), and
  // `construct` below carries the recovered body by hand.
  void construct(int input);
  ~MapWin();
  int UNK1();
  void UNK3();
  void do_image_buttons();
  void main_caption();
  void on_left_click(int a1, int a2);
  void on_right_click(int a, int b);
  void on_left_double_click(int, int);
  void on_left_up(int, int);
  void close();
  int UNK2();
  // Resets the "this window is live" flag (MapWinActiveOffset) and other
  // per-window state; called by this class's own destructor and, directly,
  // by PlanWin's and Console's (they never call MapWin::~MapWin() - see
  // mapwin.cpp).
  void clear(int index);

  // THE FIELD BLOCK IS PUBLIC, deliberately. go_reset (time.cpp, a free
  // function) resets several of these dwords on every window of the table,
  // and the mapwin_terrain_fixup / draw_map family reads 0x1DD74 from
  // outside through the documented raw-offset idiom; the offsets are
  // already carved members, so hiding the reset targets behind raw-offset
  // writes would model the same bytes twice. Access changes no offset; the
  // static_assert below still pins the layout.
  void *owned_;
  uint8_t field_8_[0x1DD64];  // 0x8
  uint32_t field_1DD6C_;  // 0x1DD6C
  uint32_t field_1DD70_;  // 0x1DD70
  uint32_t field_1DD74_;  // 0x1DD74
  uint32_t field_1DD78_;  // 0x1DD78
  uint8_t field_1DD7C_[0x4];  // 0x1DD7C
  uint32_t field_1DD80_;  // 0x1DD80
  uint32_t field_1DD84_;  // 0x1DD84
  uint32_t field_1DD88_;  // 0x1DD88
  uint32_t field_1DD8C_;  // 0x1DD8C
  uint32_t field_1DD90_;  // 0x1DD90
  uint32_t field_1DD94_;  // 0x1DD94
  uint32_t field_1DD98_;  // 0x1DD98
  uint32_t field_1DD9C_;  // 0x1DD9C
  uint32_t field_1DDA0_;  // 0x1DDA0
  uint32_t field_1DDA4_;  // 0x1DDA4
  uint32_t field_1DDA8_;  // 0x1DDA8
  uint32_t field_1DDAC_;  // 0x1DDAC
  uint32_t field_1DDB0_;  // 0x1DDB0
  uint32_t field_1DDB4_;  // 0x1DDB4
  uint32_t field_1DDB8_;  // 0x1DDB8
  uint32_t field_1DDBC_;  // 0x1DDBC
  uint32_t field_1DDC0_;  // 0x1DDC0
  uint32_t field_1DDC4_;  // 0x1DDC4
  uint32_t field_1DDC8_;  // 0x1DDC8
  uint32_t field_1DDCC_;  // 0x1DDCC
  uint32_t field_1DDD0_;  // 0x1DDD0
  uint32_t field_1DDD4_;  // 0x1DDD4
  uint32_t field_1DDD8_;  // 0x1DDD8
  uint32_t field_1DDDC_;  // 0x1DDDC
  uint32_t field_1DDE0_;  // 0x1DDE0
  uint8_t field_1DDE4_[0x28];  // 0x1DDE4
  // Carved out of what was field_1DDE4_[0x40] for go_reset (0x0050EF50), which
  // writes -1 here for every window it resets - the only access the image makes
  // to the offset. The trailing pad keeps the array's 0x40 extent, so no offset
  // past it moves.
  uint32_t field_1DE0C_;  // 0x1DE0C
  uint8_t field_1DE10_[0x14];  // 0x1DE10
  uint32_t field_1DE24_;  // 0x1DE24
  uint8_t field_1DE28_[0x1098];  // 0x1DE28
  uint32_t field_1EEC0_;  // 0x1EEC0
  uint32_t field_1EEC4_;  // 0x1EEC4
  uint8_t field_1EEC8_[0x28];  // 0x1EEC8
  uint32_t field_1EEF0_;  // 0x1EEF0
  uint32_t field_1EEF4_;  // 0x1EEF4
  uint8_t field_1EEF8_[0x8];  // 0x1EEF8
  uint32_t field_1EF00_;  // 0x1EF00
  uint32_t field_1EF04_;  // 0x1EF04
  uint8_t field_1EF08_[0x14];  // 0x1EF08
  uint32_t field_1EF1C_;  // 0x1EF1C
  uint8_t field_1EF20_[0x28];  // 0x1EF20
  uint32_t field_1EF48_;  // 0x1EF48
  uint32_t field_1EF4C_;  // 0x1EF4C
  uint32_t field_1EF50_;  // 0x1EF50
  uint8_t field_1EF54_[0x2AF0];  // 0x1EF54
  uint32_t field_21A44_;  // 0x21A44
  uint32_t field_21A48_;  // 0x21A48
  uint32_t field_21A4C_;  // 0x21A4C
  uint32_t field_21A50_;  // 0x21A50
  uint32_t field_21A54_;  // 0x21A54
  uint32_t field_21A58_;  // 0x21A58
  uint32_t field_21A5C_;  // 0x21A5C
  uint32_t field_21A60_;  // 0x21A60
  uint32_t field_21A64_;  // 0x21A64

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


// MapWin::main_caption sets the date on the one MainInterface the original
// keeps at a fixed address, using a caption that also lives at one.
typedef void (OriginalObject::*func_set_date)(char *);
extern func_set_date MainInterfaceOriginalSetDate;
// AN OBJECT AT 0x007AE820, not a `void *`: `MapWin::main_caption` is
// `mov ecx, 0x7ae820` - the address as an immediate - where a pointer
// variable loads it. Its constructor is trivial, so making it real
// adds no dynamic initialiser.
extern MainInterface MainInterfaceGlobal;
extern char MapWinMainCaption[256];

// 0x004710E0, still an original body (src/unrecovered/004710e0.cpp); the
// definition in pending_bodies.cpp forwards to it. desktop_close tears the
// map-window system down between the window closes and the final hide.
void __cdecl mapwin_system_shutdown();


// The heap pointer at 0x4 is freed through the game's CRT boundary, the same
// address buffer.cpp frees through. Rebindable so tests observe it.
typedef void *func_map_win_free(void *);


// MapWin's click handler and the input-enable flag it gates on are not
// recovered; the flag lives at a fixed address and is rebindable for tests.
int32_t *const MapWinInputEnabled = (int32_t *)0x0090D938;


// The map-window slot array the free drawing helpers broadcast to: eight
// MapWin* in the image's zero-initialised .data at 0x007D3C3C, ending at the
// exclusive 0x007D3C5C that draw_tile compares against
// (`cmp esi, 0x7d3c5c` at 0x0046AF79 -> (0x7D3C5C - 0x7D3C3C) / 4 == 8).
// The element type is pinned by ?mapwin_system_init@@YAXH@Z at 0x0047107F,
// `mov ecx, dword ptr [esi*4 + 0x7d3c3c]`, and by ?zoom@BaseWin@@QAEXHH@Z at
// 0x0041AAC3, which walks slot 0's vbtable. Populated at run time, so this is
// a mutable global; rebindable so leaf tests can seed a controlled table.
// A REFERENCE TO THE ARRAY, not a pointer to its first element. The image
// indexes it absolutely - `mov eax, [esi*4 + 0x7d3c3c]` at 0x005108DE, one
// instruction per read - and any spelling that yields a POINTER VALUE lets
// VC6 park the base in a register across a loop, which the image never does.
// An array reference has no value to park: the subscript folds the address in.
//
// TRIED: an accessor returning `reinterpret_cast<MapWin **>(0x007D3C3C)`
// is a pointer value and measures WORSE than the `MapWin **const` it replaced
// - 19 of 85 instructions in `Console::focus` against 29.
typedef MapWin *MapWinSlots[8];
inline MapWinSlots &map_win_table() {
  // THROUGH A NAMED POINTER. VC6 rejects dereferencing a cast integer literal
  // of array-pointer type outright - `C2101: '&' on constant` - so the cast
  // lands in a variable first, which it accepts and folds.
  MapWinSlots *const table = (MapWinSlots *)0x007D3C3C;
  return *table;
}
#define MapWinTable (map_win_table())
static const size_t MapWinTableSlots = 8;        // (0x007D3C5C - 0x007D3C3C) / 4

// 0x0068A5CC. THE SLOT OF THE WINDOW BEING WORKED WITH, or -1 for none.
// go_reset (0x0050EF50) proves the pairing with the table above: its walk
// skips the slot whose index equals this (`cmp ecx, edx / je` at 0x0050EF9C,
// the index and this side by side), then resets THIS slot separately at
// 0x0050F083 (`mov edi, [eax*4 + 0x7d3c3c]`) and stores -1 back at the end
// (0x0050F155). Every read is a signed compare against zero (`jl` at
// 0x0050F07D, and 0x00517BF6 the same), so `int`, not unsigned.
extern int MapWinSelectedSlot;  // 0x0068A5CC

// Per-window "this window is live" dword. The MapWin constructor clears it
// (0x00462822), MapWin::clear writes it (0x004628AF), and mapwin_system_init
// sets it per slot (0x00471090, 0x0047109E). Every free drawing helper gates
// on it for slots 1..7. Kept as a documented raw offset rather than carved
// into MapWin: the field is read from outside the class, MapWin's 0x21A6C
// bytes of derived storage are deliberately unmapped, and carving it would
// force inventing a public accessor for a field whose full meaning is not
// recovered. Same idiom as scroll.cpp's read_volatile_bits.
static const size_t MapWinActiveOffset = 0x1DD74;

// MapWin::draw_radius at 0x0046A2A0 is still an original body: `ret 0x10` at
// 0x0046A493 makes it __thiscall with four stack arguments, and `mov esi, ecx`
// at 0x0046A2AF plus the vbtable walk at 0x0046A3FB confirm the incoming
// pointer is an unadjusted MapWin*. It reaches gen_radius, Texture::draw_trans,
// MapWin::compute_clip, GraphicWin::soft_update and do_all_draws, so it stays
// original; bound through a rebindable seam so tests can substitute a probe
// and the seam can later point at a recovered body.
typedef void (OriginalObject::*func_map_win_draw_radius)(int x_coord, int y_coord, int a3, int draw_type);
extern func_map_win_draw_radius MapWinOriginalDrawRadius;  // 0x0046A2A0

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

// ?draw_map@MapWin@@QAEXH@Z at 0x0046A550 - public, __thiscall, void(int) by
// the mangled name, `ret 4` by the image - is 2049 bytes and still an original
// body. The receiver is an unadjusted MapWin*, which is why MapWin::on_redraw
// backs the 0x21A6C virtual-base displacement out before calling it. Defined
// at the end of mapwin.cpp as a forwarder so that call links.
// Console holds its own seam on the same address
// (console.h, ConsoleOriginalMapWinDrawMap); the two are independent by
// design - that one is a call site's, this one is the class's.
typedef void (OriginalObject::*func_map_win_draw_map)(int draw_type);
extern func_map_win_draw_map MapWinOriginalDrawMap;  // 0x0046A550

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
