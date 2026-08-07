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

 /*
  * MapWin class
  *
  * The original derives this from GraphicWin *virtually*, and that cannot be
  * written as `: virtual GraphicWin` here. MSVC places a virtual base at the
  * offset its vbtable names, while the Itanium ABI this toolchain follows
  * places it after the derived object entirely - so the faithful-looking
  * declaration is the one that would silently produce the wrong layout. The
  * base is therefore held as a member at the offset MSVC put it, which
  * reproduces the original addresses under either ABI.
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
class DLLEXPORT MapWin {
 public:
  void on_resize(int a1, int a2);
  void on_redraw();
  // ?draw_map@MapWin@@QAEXH@Z at 0x0046A550, still an original body: declared
  // here so on_redraw's direct call compiles, resolved at link time.
  void draw_map(int draw_type);
  MapWin() { ; }
  ~MapWin() { ; }
  int UNK1();
  void UNK3();
  void do_image_buttons();
  void main_caption();
  void on_left_click(int a1, int a2);
  void on_right_click(int a1, int a2);
  void on_left_double_click(int, int);
  void on_left_up(int, int);
  void close();
  int UNK2();

 private:
  // The vbtable pointer opens the object; a heap pointer close() frees sits at
  // 0x4. Both are carved out of the derived storage, keeping the total the
  // static_assert pins.
  uint32_t vbtable_pointer_;
  void *owned_;
  uint8_t derived_tail_[0x21A6C - 0x8];
  GraphicWin virtual_base_;
};

static_assert(sizeof(MapWin) == 0x22480, "MapWin layout must match terranx.exe");

int __fastcall map_win_unk1_redirect(MapWin *self, void *);
void __fastcall map_win_unk3_redirect(MapWin *self, void *);
void __fastcall map_win_do_image_buttons_redirect(MapWin *self, void *);

// MapWin::main_caption sets the date on the one MainInterface the original
// keeps at a fixed address, using a caption that also lives at one.
typedef void (OriginalObject::*func_set_date)(char *);
extern func_set_date MainInterfaceOriginalSetDate;
extern void *MainInterfaceGlobal;
extern char *MapWinMainCaption;

void __fastcall map_win_main_caption_redirect(MapWin *self, void *);

// The heap pointer at 0x4 is freed through the game's CRT boundary, the same
// address buffer.cpp frees through. Rebindable so tests observe it.
typedef void *func_map_win_free(void *);
extern func_map_win_free *MapWinFree;

void __fastcall map_win_close_redirect(MapWin *self, void *);
int __fastcall map_win_unk2_redirect(MapWin *self, void *);

// MapWin's click handler and the input-enable flag it gates on are not
// recovered; the flag lives at a fixed address and is rebindable for tests.
typedef void (OriginalObject::*func_map_win_click)(int a1, int a2, int button);
extern func_map_win_click MapWinClick;
extern int32_t *MapWinInputEnabled;

void __fastcall map_win_on_left_click_redirect(MapWin *self, void *, int a1, int a2);
void __fastcall map_win_on_right_click_redirect(MapWin *self, void *, int a1, int a2);
void __fastcall map_win_on_left_double_click_redirect(MapWin *self, void *, int a1, int a2);
void __fastcall map_win_on_left_up_redirect(MapWin *self, void *, int a1, int a2);

// The map-window slot array the free drawing helpers broadcast to: eight
// MapWin* in the image's zero-initialised .data at 0x007D3C3C, ending at the
// exclusive 0x007D3C5C that draw_tile compares against
// (`cmp esi, 0x7d3c5c` at 0x0046AF79 -> (0x7D3C5C - 0x7D3C3C) / 4 == 8).
// The element type is pinned by ?mapwin_system_init@@YAXH@Z at 0x0047107F,
// `mov ecx, dword ptr [esi*4 + 0x7d3c3c]`, and by ?zoom@BaseWin@@QAEXHH@Z at
// 0x0041AAC3, which walks slot 0's vbtable. Populated at run time, so this is
// a mutable global; rebindable so leaf tests can seed a controlled table.
extern MapWin **MapWinTable;                  // 0x007D3C3C
constexpr size_t MapWinTableSlots = 8;        // (0x007D3C5C - 0x007D3C3C) / 4

// Per-window "this window is live" dword. The MapWin constructor clears it
// (0x00462822), MapWin::clear writes it (0x004628AF), and mapwin_system_init
// sets it per slot (0x00471090, 0x0047109E). Every free drawing helper gates
// on it for slots 1..7. Kept as a documented raw offset rather than carved
// into MapWin: the field is read from outside the class, MapWin's 0x21A6C
// bytes of derived storage are deliberately unmapped, and carving it would
// force inventing a public accessor for a field whose full meaning is not
// recovered. Same idiom as scroll.cpp's read_volatile_bits.
constexpr size_t MapWinActiveOffset = 0x1DD74;

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
DLLEXPORT void __cdecl draw_tile(int x_coord, int y_coord, int draw_type);
DLLEXPORT void __cdecl draw_tiles(int x_coord, int y_coord, int draw_type);

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
