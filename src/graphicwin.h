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
#include "buffer.h"
#include "win.h"

 /*
  * GraphicWin class
  */
// The base is spelled `public` deliberately. Win grants `friend class
// BaseButton;` / `friend class Scroll;` so those classes may touch
// win_parent_, but VC6 does not implement the friend clause of
// [class.access.base]/5: a friend of the derived class still cannot see
// through a PRIVATE base, and reaching win_parent_ from BaseButton or Scroll
// gave C2248/C2247 while the base stayed private. Access to the member itself
// is unchanged - win_parent_ is still private in Win - and a base's
// accessibility affects no offset, so the layout is untouched.
class GraphicWin : public Win {
 public:
  // 0x005D7030, a pending_bodies forwarder.
  void compute_min_size();

 public:
  // 0x005D6AC0, a pending_bodies forwarder.
  void overlay_nonclient(RECT *area);
  friend class Scroll;
  // BaseButton's colour setters drive this buffer directly.
  friend class BaseButton;

 public:
  void on_mouse_move(int a1, int a2, unsigned int a3, int a4);
  GraphicWin() { ; }
  ~GraphicWin() { ; }
  void construct();
  uint32_t close();
  int init(int x, int y, int width, int height, LPSTR title, int flags,
           Win *parent, Menu *menu, BorderSizing *border);
  int fill(int x1, int y1, int x2, int y2, int color);
  void fill(int color);
  void redraw();
  // 0x005D56B0  ?update@GraphicWin@@QAEXPAUGraphicWin@@@Z - public,
  // __thiscall, void(GraphicWin *). The one-argument overload; the siblings
  // take (int, int, int, int, GraphicWin *) and (RECT *, GraphicWin *).
  void update(GraphicWin *target);
  // 0x005D5890  ?soft_update@GraphicWin@@QAEXXZ - public, __thiscall,
  // void(void). Called directly (not virtually) by WorldWin, so it must not
  // be virtual here. Still an original body; the definition at the end of
  // graphicwin.cpp forwards to it.
  void soft_update();
  // 0x005D5930, still an original body - a distinct overload from the
  // zero-argument one above. StatusWin::redraw calls it BY NAME.
  void soft_update(RECT *area);
 private:
  Buffer buffer_;
  uint32_t field_9CC_;
  uint32_t field_9D0_;
  uint32_t field_9D4_;
  uint32_t field_9D8_;
  uint32_t field_9DC_;
  uint32_t field_9E0_;
  uint32_t field_9E4_;
  uint32_t field_9E8_;
  uint32_t field_9EC_;
  uint32_t field_9F0_;
  uint32_t field_9F4_;
  uint32_t field_9F8_;
  uint32_t field_9FC_;
  uint32_t field_A00_;
  uint32_t field_A04_;
  uint32_t poCanvas_;
  uint32_t field_A0C_;
  uint32_t field_A10_;
};

// Buffer::fill is not recovered yet, so it is reached through a seam on the
// window's own buffer at 0x444.
typedef int (OriginalObject::*func_buffer_fill)(int, int, int, int, int);

int __fastcall graphic_win_fill_redirect(GraphicWin *self, void *,
                                         int x1, int y1, int x2, int y2,
                                         int color);

static_assert(sizeof(GraphicWin) == 0xA14,
              "GraphicWin layout must match the legacy ABI");

typedef void (OriginalObject::*func_subobject_destructor)();
typedef void (OriginalObject::*func_subobject_close)();

// The Buffer subobject destructor is source-owned; the seam stays bindable so
// tests outside the hybrid process can substitute it. The Win subobject
// destructor is still a temporary original dependency.
// tear down GDI and DirectDraw resources through the executable's own CRT and
// stay at their original addresses until that closure is source-owned. Tests
// outside the hybrid process rebind them.
extern func_subobject_destructor BufferSubobjectDestructor;
extern func_subobject_destructor WinOriginalDestructor;

// GraphicWin::close keeps Win::close as a temporary original dependency;
// Buffer::close is source-owned. Both remain bindable so source-level tests
// can verify delegation order without entering process-owned teardown code.
extern func_subobject_close BufferSubobjectClose;
extern func_subobject_close WinOriginalClose;

// Process default copied into field_A0C_ by close().
extern uint32_t GraphicWinFieldA0CDefault;

// Original virtual table addresses the destructor installs before delegating.
extern const uint32_t GraphicWinPrimaryVtable;
extern const uint32_t GraphicWinBufferVtable;

GraphicWin *__fastcall graphic_win_destructor_redirect(GraphicWin *self, void *);
// A GraphicWin THAT CONSTRUCTS ITSELF, for the derived classes whose image
// constructor calls GraphicWin::construct() BEFORE its own members.
//
// C++ runs member constructors before the constructor BODY, so
// `GraphicWin::construct()` written in the body of a class that has members
// emits AFTER them - the call targets match the image and the call ORDER does
// not, and no amount of spelling inside the body can fix it. Base subobjects,
// however, ARE constructed before members. Deriving from this instead of from
// GraphicWin directly puts the call where the image has it, using the language
// rule rather than raw storage and placement-new.
//
// Adds no data and no virtual of its own, so every derived layout is
// unchanged - which each class's own static_assert on sizeof is what checks.
// Measured on Scroll (scroll.cpp) 2026-08-22: 21 -> 81 of 83, and the reorder
// was worth 1 instruction by itself. What it bought was VISIBILITY of two more
// levers underneath, the larger worth 55, which were unreachable until the
// order was right. Expect the same shape here: re-measure and look again after
// switching a class over, do not stop at the first number.
//
// Use it ONLY where the image really does construct first - check with
// `uv run tools/osmx.py show <ctor>`, whose call to 0x005D4CF0 should come
// before the first `lea ecx, [esi + N]`. tools/body_construct_order.py lists
// the candidates.
struct ConstructedGraphicWin : public GraphicWin {
  ConstructedGraphicWin() { construct(); }
};

GraphicWin *__fastcall graphic_win_construct_redirect(GraphicWin *self, void *);
uint32_t __fastcall graphic_win_close_redirect(GraphicWin *self, void *);

// Test seams: the subobject destructors are original dependencies, so tests
// substitute recording stubs to observe delegation targets and ordering.
void graphic_win_destructor_probe_reset();
int graphic_win_destructor_probe_buffer_calls();
int graphic_win_destructor_probe_win_calls();
void *graphic_win_destructor_probe_buffer_target();
void *graphic_win_destructor_probe_win_target();
int graphic_win_destructor_probe_order();

// Buffer::fill(int) at 0x005DFB50 (ret 4) paints the whole surface in one
// colour; Buffer::map_colors at 0x005DA330 (ret 0x14) remaps a region through
// a translation table. Both are still original bodies. The fill's extents are
// INCLUSIVE: the recovered caller passes width-1 and -1-height, which the
// RECT sibling at 0x005D5350 independently confirms by passing right-1 and
// bottom-1.
typedef int (OriginalObject::*func_graphic_win_buffer_fill_color)(int);
typedef int (OriginalObject::*func_graphic_win_map_colors)(int, int, int, int, void *);
// GraphicWin::overlay_nonclient 0x005D6AC0 (ret 4) repaints the frame.
typedef void (OriginalObject::*func_graphic_win_overlay_nonclient)(RECT *);

// The translation table fill consults before remapping. A null table means
// the plain blit is the whole operation, which is the common case.
inline void *&GraphicWinColorMapTable() { return *reinterpret_cast<void **>(0x009B3390); }
// The optional hook stored at 0xA10. See redraw's Verification note: it is
// entered by a bare `call eax` with nothing pushed and ECX not set, so it is
// not __thiscall, and zero-argument __cdecl and __stdcall are
// indistinguishable here.
typedef void(__cdecl func_graphic_win_paint_hook)();

void __fastcall graphic_win_fill_color_redirect(GraphicWin *self, void *,
                                                int color);
void __fastcall graphic_win_redraw_redirect(GraphicWin *self, void *);

// GraphicWin::init's four remaining original dependencies. Win::init at
// 0x005EBD80 (ret 0x24) is the 2285-byte window-creation body,
// GraphicWin::compute_min_size at 0x005D7030 (bare ret) is 272 bytes of frame
// arithmetic, Win::nonclient_to_client at 0x005EEF60 (ret 8) converts an outer
// size to a client size in place, and Buffer::init at 0x005D7670 (ret 0x10) is
// the 847-byte DirectDraw/GDI surface creator. Tests rebind all four.
typedef int (OriginalObject::*func_win_init)(int, int, int, int, LPSTR, int, Win *, Menu *, BorderSizing *);
typedef void (OriginalObject::*func_graphic_win_compute_min_size)();
typedef void (OriginalObject::*func_win_nonclient_to_client)(int *, int *);
typedef int (OriginalObject::*func_buffer_init)(int, int, int, void *);
extern func_win_init WinOriginalInit;
extern func_graphic_win_compute_min_size GraphicWinOriginalComputeMinSize;
extern func_win_nonclient_to_client WinOriginalNonclientToClient;
extern func_buffer_init BufferOriginalInit;

// The eleven process window defaults init republishes, at 0x009B3394 - one
// contiguous block between the colour-map slot GraphicWinColorMapTable
// already binds at 0x009B3390 and GraphicWinFieldA0CDefault at 0x009B33C0.
uint32_t *const GraphicWinInitDefaults = (uint32_t *)0x009B3394;

int __fastcall graphic_win_init_redirect(GraphicWin *self, void *,
                                         int x, int y, int width, int height,
                                         LPSTR title, int flags, Win *parent,
                                         Menu *menu, BorderSizing *border);

// GraphicWin::soft_update at 0x005D5890 (157 bytes, bare ret) blits the
// window's own buffer without the full repaint path. Still an original body -
// MapWin::draw_radius, WorldWin and StatusWin::redraw all reach it - so it is
// forwarded. Rebindable so tests can substitute a probe and so the seam can
// later point at a recovered body.
