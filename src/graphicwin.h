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

// The graphics loader at 0x004559A4 supplies eleven Sprite pointers.
// Input and process-default layouts differ; the publisher at 0x005D6A50
// establishes this permutation. Neither record owns the Sprite lifetimes.
class Sprite;
struct NCSprites {
    // RECOVERY_MEMBER: v3 NCSprites top_left Sprite* 0x0 0x005D6A50
    Sprite *top_left;
    // RECOVERY_MEMBER: v3 NCSprites top_right Sprite* 0x4 0x005D6A50
    Sprite *top_right;
    // RECOVERY_MEMBER: v3 NCSprites bottom_left Sprite* 0x8 0x005D6A50
    Sprite *bottom_left;
    // RECOVERY_MEMBER: v3 NCSprites bottom_right Sprite* 0xC 0x005D6A50
    Sprite *bottom_right;
    // RECOVERY_MEMBER: v3 NCSprites top Sprite* 0x10 0x005D6A50
    Sprite *top;
    // RECOVERY_MEMBER: v3 NCSprites left Sprite* 0x14 0x005D6A50
    Sprite *left;
    // RECOVERY_MEMBER: v3 NCSprites right Sprite* 0x18 0x005D6A50
    Sprite *right;
    // RECOVERY_MEMBER: v3 NCSprites bottom Sprite* 0x1C 0x005D6A50
    Sprite *bottom;
    // RECOVERY_MEMBER: v3 NCSprites noncaption_middle Sprite* 0x20 0x005D6A50
    Sprite *noncaption_middle;
    // RECOVERY_MEMBER: v3 NCSprites noncaption_left Sprite* 0x24 0x005D6A50
    Sprite *noncaption_left;
    // RECOVERY_MEMBER: v3 NCSprites noncaption_right Sprite* 0x28 0x005D6A50
    Sprite *noncaption_right;
};

struct GraphicWinFrameDefaults {
    // RECOVERY_MEMBER: v3 GraphicWinFrameDefaults top_left Sprite* 0x0 0x005D6A50
    Sprite *top_left;
    // RECOVERY_MEMBER: v3 GraphicWinFrameDefaults bottom_left Sprite* 0x4 0x005D6A50
    Sprite *bottom_left;
    // RECOVERY_MEMBER: v3 GraphicWinFrameDefaults top_right Sprite* 0x8 0x005D6A50
    Sprite *top_right;
    // RECOVERY_MEMBER: v3 GraphicWinFrameDefaults bottom_right Sprite* 0xC 0x005D6A50
    Sprite *bottom_right;
    // RECOVERY_MEMBER: v3 GraphicWinFrameDefaults top Sprite* 0x10 0x005D6A50
    Sprite *top;
    // RECOVERY_MEMBER: v3 GraphicWinFrameDefaults bottom Sprite* 0x14 0x005D6A50
    Sprite *bottom;
    // RECOVERY_MEMBER: v3 GraphicWinFrameDefaults left Sprite* 0x18 0x005D6A50
    Sprite *left;
    // RECOVERY_MEMBER: v3 GraphicWinFrameDefaults right Sprite* 0x1C 0x005D6A50
    Sprite *right;
    // RECOVERY_MEMBER: v3 GraphicWinFrameDefaults noncaption_right Sprite* 0x20 0x005D6A50
    Sprite *noncaption_right;
    // RECOVERY_MEMBER: v3 GraphicWinFrameDefaults noncaption_left Sprite* 0x24 0x005D6A50
    Sprite *noncaption_left;
    // RECOVERY_MEMBER: v3 GraphicWinFrameDefaults noncaption_middle Sprite* 0x28 0x005D6A50
    Sprite *noncaption_middle;
};


 /*
  * GraphicWin class
  */
// The optional paint hook stored at 0xA10. See redraw's Verification note:
// it is entered by a bare `call eax` with nothing pushed and ECX not set, so
// it is not __thiscall, and zero-argument __cdecl and __stdcall are
// indistinguishable here.
typedef void(__cdecl func_graphic_win_paint_hook)();
// The base is spelled `public` deliberately. Win grants `friend class
// BaseButton;` / `friend class Scroll;` so those classes may touch
// win_parent_, but VC6 does not implement the friend clause of
// [class.access.base]/5: a friend of the derived class still cannot see
// through a PRIVATE base, and reaching win_parent_ from BaseButton or Scroll
// gave C2248/C2247 while the base stayed private. Access to the member itself
// is unchanged - win_parent_ is still private in Win - and a base's
// accessibility affects no offset, so the layout is untouched.
class GraphicWin : public Win, public Buffer {
 public:
  // Recompute frame minima only when all four selected corner sprites exist.
  void compute_min_size();

 public:
  // Paint frame sprites or the owned-buffer overlay, preserving the clip.
  void overlay_nonclient(RECT *area);
  // Original caller supplies one explicit pointer and no receiver.
  static void __cdecl set_def_nc_sprites(NCSprites *sprites);
  void load_nc_sprites_from_defs();
  static void load_all_nc_sprites_from_defs();
  friend class Scroll;
  // BaseButton's colour setters drive this buffer directly.
  friend class BaseButton;

 public:
  void on_mouse_move(int x, int y, unsigned int keys, int from_parent);
  GraphicWin();
  ~GraphicWin();
  // One override serves Win slot 61 and Buffer slot 1. The secondary-base
  // entry adjusts Buffer's receiver back to this GraphicWin automatically.
  virtual GraphicWin *get_graphic_win();
  virtual int resize(int width, int height, int client_size);
  virtual int on_set_cursor(void *window, unsigned int hit, unsigned int message);
  virtual void on_size(unsigned int flags, int width, int height);
  // Win slot 73: GraphicWin and EditBox tables select 0x005D5CF0.
  virtual void on_paint(RECT *area);
  // Win slot 66: the original PushButton table word at 0x00670D68 selects
  // this derived entry (0x005D63E0), which delegates to Win::set_rects.
  virtual void set_rects();

  // Original table66FC50 ends with these two slots after Win's88 slots.
  // Win::on_l_button_down5F652E consumes slot88's result: nonzero continues
  // ordinary left-down delivery. StringBox432720 forwards the same hook.
  // Slot89 is invoked at5D6591 after the caption callback, with live text
  // and canvas pointers. All observed original derived tables inherit it.
  virtual int on_link_clicked(int, char *) { return 1; }  // slot88,4042A0
  virtual void on_draw_caption(char *, Buffer *) {}       // slot89,404260
  // Focus and mouse-leave override Win slots51 and18 respectively. They
  // must not be appended here merely to produce matching adjustor thunks.
  // Nonvirtual reusable cleanup. Its artificial integer return and manual
  // owned-buffer deletion still need the separately reviewed void migration.
  uint32_t close();
  // 0x00589550, a 5-byte `jmp` to GraphicWin::close, homed into general.cpp.
  // A plain member: no virtual, so nothing adjusts the receiver and VC6 can
  // tail-jump it.
  void j_close();
  int init(int x, int y, int width, int height, LPSTR title, int flags,
           Win *parent, Menu *menu, BorderSizing *border);
  int init(RECT *bounds, char *caption, int flags, Win *parent,
           Menu *menu, BorderSizing *border);
  int fill(int x1, int y1, int x2, int y2, int color);
  int fill(RECT *area, int color);
  void fill(int color);
  // Win slot 62; original PushButton entry 0x00670D58 selects 0x005D5A70.
  virtual void redraw();
  // Win slot 63: constructor-installed table 0x0066FC50 selects 0x00404290.
  // AlphaMovie, MapWin and Console inherit this same surface-update callback.
  virtual void on_update();
  // 0x005D56B0  ?update@GraphicWin@@QAEXPAUGraphicWin@@@Z - public,
  // __thiscall, void(GraphicWin *). The one-argument overload; the siblings
  // take (int, int, int, int, GraphicWin *) and (RECT *, GraphicWin *).
  void update(GraphicWin *target);
  // 0x005D55D0 ?update@GraphicWin@@QAEXHHHHPAUGraphicWin@@@Z (95 bytes).
  // The five-argument overload: build the client rectangle (left, top,
  // right = x+width, bottom = y+height), update_window it, convert it to
  // screen coordinates and static-flip that region. The fifth argument is
  // part of the symbol but dead in the body (verified against the image:
  // every one of the 95 bytes is spelled by the four calls and the
  // rectangle arithmetic). The body is homed in leaf_recoveries.cpp;
  // Console's editor commands (console.cpp:4314..4323) still spell the two
  // calls as raw-address casts, which this declaration now unblocks
  // repointing.
  void update(int x, int y, int width, int height, GraphicWin *source);
  // 0x005D5630 returns zero on both paths; the second argument is unused.
  int update(RECT *area, GraphicWin *target);
  // Invalidate the full Buffer rectangle, or a clipped client rectangle.
  // Direct nonvirtual calls at 0x005D5890 and 0x005D5930. Neither overload
  // repaints synchronously or modifies a borrowed rectangle.
  void soft_update();
  void soft_update(RECT *area);
  // The AlphaMenu backfill callback draws into the supplied window's Buffer
  // working rectangle (Buffer::rect2_, original offset 0x474).
  RECT *buffer_working_rect() { return &rect2_; }
 private:
  // Borrowed frame sprites. overlay_nonclient supplies each as Sprite's
  // receiver or Buffer::tile's Sprite argument, reading +8/+0x18/+0x1C.
  // Corner names follow their actual placement, not the default-table order.
  // RECOVERY_MEMBER: v3 GraphicWin frame_top_left_ Sprite* 0x9CC 0x005D6AC0
  Sprite *frame_top_left_;
  // RECOVERY_MEMBER: v3 GraphicWin frame_bottom_left_ Sprite* 0x9D0 0x005D6AC0
  Sprite *frame_bottom_left_;
  // RECOVERY_MEMBER: v3 GraphicWin frame_top_right_ Sprite* 0x9D4 0x005D6AC0
  Sprite *frame_top_right_;
  // RECOVERY_MEMBER: v3 GraphicWin frame_bottom_right_ Sprite* 0x9D8 0x005D6AC0
  Sprite *frame_bottom_right_;
  // RECOVERY_MEMBER: v3 GraphicWin frame_left_ Sprite* 0x9DC 0x005D6AC0
  Sprite *frame_left_;
  // RECOVERY_MEMBER: v3 GraphicWin frame_right_ Sprite* 0x9E0 0x005D6AC0
  Sprite *frame_right_;
  // RECOVERY_MEMBER: v3 GraphicWin frame_bottom_ Sprite* 0x9E4 0x005D6AC0
  Sprite *frame_bottom_;
  // RECOVERY_MEMBER: v3 GraphicWin frame_top_ Sprite* 0x9E8 0x005D6AC0
  Sprite *frame_top_;
  // The loader supplies the existing NONCAP_LEFT/RIGHT/MID Sprite objects.
  // RECOVERY_MEMBER: v3 GraphicWin frame_noncaption_left_ Sprite* 0x9EC 0x005D6EC0
  Sprite *frame_noncaption_left_;
  // RECOVERY_MEMBER: v3 GraphicWin frame_noncaption_right_ Sprite* 0x9F0 0x005D6EC0
  Sprite *frame_noncaption_right_;
  // RECOVERY_MEMBER: v3 GraphicWin frame_noncaption_middle_ Sprite* 0x9F4 0x005D6EC0
  Sprite *frame_noncaption_middle_;
  // Optional alternate corners; null selects the matching frame corner.
  // RECOVERY_MEMBER: v3 GraphicWin overlay_top_left_ Sprite* 0x9F8 0x005D6AC0
  Sprite *overlay_top_left_;
  // RECOVERY_MEMBER: v3 GraphicWin overlay_bottom_left_ Sprite* 0x9FC 0x005D6AC0
  Sprite *overlay_bottom_left_;
  // RECOVERY_MEMBER: v3 GraphicWin overlay_top_right_ Sprite* 0xA00 0x005D6AC0
  Sprite *overlay_top_right_;
  // RECOVERY_MEMBER: v3 GraphicWin overlay_bottom_right_ Sprite* 0xA04 0x005D6AC0
  Sprite *overlay_bottom_right_;
  // Optional separately allocated Buffer, distinct from the Buffer base.
  // init creates it; on_size resizes it; close releases it through slot 0.
  // Original init calls Buffer's constructor at 0x005D4FC5, then stores
  // its returned pointer at 0x005D4FD6. on_size loads that receiver at
  // 0x005D628D before calling Buffer::resize at 0x005D6299.
  // RECOVERY_MEMBER: v3 GraphicWin owned_buffer_ Buffer* 0xA08 0x005D4EF0
  Buffer *owned_buffer_;
  uint32_t field_A0C_;
  // 0xA10. THE PAINT HOOK `redraw` calls before painting, not storage: null
  // by default, installed from outside, entered with no arguments. Was
  // `uint32_t field_A10_` - same four bytes, and the caller stops casting.
  func_graphic_win_paint_hook *field_A10_;
};

// Buffer::fill is not recovered yet, so it is reached through a seam on the
// window's own buffer at 0x444.
typedef int (OriginalObject::*func_buffer_fill)(int, int, int, int, int);


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
// `uv run recover inspect <ctor> --image <original>`: the call to 0x005D4CF0
// should come before the first `lea ecx, [esi + N]`. Establish this order from
// original constructor/caller evidence before choosing a candidate class.
struct ConstructedGraphicWin : public GraphicWin {
  // Empty: GraphicWin::GraphicWin() is a REAL constructor now, so the
  // base runs implicitly and calling it again would double-construct.
  ConstructedGraphicWin() { ; }
};


// Test seams: the subobject destructors are original dependencies, so tests
// substitute recording stubs to observe delegation targets and ordering.
void graphic_win_destructor_probe_reset();
int graphic_win_destructor_probe_buffer_calls();
int graphic_win_destructor_probe_win_calls();
void *graphic_win_destructor_probe_buffer_target();
void *graphic_win_destructor_probe_win_target();
int graphic_win_destructor_probe_order();

// Borrowed process colour-map pointer, initially null in the pinned image.
// Both transparent fill overloads consult it after copying the parent surface.
// A null pointer leaves the copied pixels unchanged.
extern unsigned char *GraphicWinColorMapTable;


// Historical seam declarations for GraphicWin::init dependencies. Win::init at
// 0x005EBD80 (ret 0x24) is the 2285-byte window-creation body,
// Win::nonclient_to_client at 0x005EEF60 (ret 8) converts an outer
// size to a client size in place, and Buffer::init at 0x005D7670 (ret 0x10) is
// the 847-byte DirectDraw/GDI surface creator. Frame minima use real sprites.
typedef int (OriginalObject::*func_win_init)(int, int, int, int, LPSTR, int, Win *, Menu *, BorderSizing *);
typedef void (OriginalObject::*func_win_nonclient_to_client)(int *, int *);
typedef int (OriginalObject::*func_buffer_init)(int, int, int, void *);
extern func_win_init WinOriginalInit;
extern func_win_nonclient_to_client WinOriginalNonclientToClient;
extern func_buffer_init BufferOriginalInit;

// The eleven process window defaults init republishes, at 0x009B3394 - one
// contiguous block between the colour-map slot GraphicWinColorMapTable
// already binds at 0x009B3390 and GraphicWinFieldA0CDefault at 0x009B33C0.
extern GraphicWinFrameDefaults GraphicWinInitDefaults;
