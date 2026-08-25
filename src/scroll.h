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
#include "flatbutton.h"
#include "graphicwin.h"
#include "sprite.h"

 /*
  * Scroll class
  */
// THE BASE IS CONSTRUCTED BY A BASE, because C++'s own ordering rule is what
// the image's order requires. 0x006051D0 runs GraphicWin::construct() BEFORE
// the two FlatButton members (0x6051ee, then 0x6051ff/0x60520f); a
// `GraphicWin::construct()` written in Scroll's constructor BODY cannot get
// there, because member constructors always run first. Base subobject
// constructors, however, run before members - so putting the call in a base
// puts it where the image has it, with no raw storage and no placement-new.
// Adds no data and no virtual of its own, so the layout is untouched, which
// the static_assert below is what actually checks.
struct ScrollGraphicWin : public GraphicWin {
  // Empty: GraphicWin::GraphicWin() is a real constructor now, so the
  // base runs implicitly at exactly the point this idiom existed to
  // reach.
  ScrollGraphicWin() { ; }
};

class Scroll : public ScrollGraphicWin {
 public:
  // 0x006051D0. Defined in scroll.cpp, alongside close() which resets the
  // same fields from the same two process-default tables.
  Scroll();
  ~Scroll() { ; }

  uint32_t close();
  Scroll *destroy();
  // 0x006054D0, still a pending_bodies forwarder. The overloads that
  // wrap it call it BY NAME, so they emit the image's `call rel32`
  // rather than the `call [ptr]` a function-pointer seam compiles.
  int init(int x, int y, int width, int height, Win *parent,
           int setting, int options);
  int init(RECT *rect, Win *parent, int setting, int options);
  int init_vert(int x, int y, int length, Win *parent, int setting);
  int init_horz(int x, int y, int length, Win *parent, int setting);
  int init_vert_nc(int x, int y, int length, Win *parent, int setting);
  int init_horz_nc(int x, int y, int length, Win *parent, int setting);
  uint32_t set_range(int minimum, int maximum);
  uint32_t set_button_color(int color);
  uint32_t set_bevel_thickness(int thickness);
  uint32_t set_bevel_upper(int color);
  uint32_t set_bevel_lower(int color);
  uint32_t set_bar_thickness(int thickness);
  void set_border_color(int color);
  void set_sprite_left(Sprite *sprite1, Sprite *sprite2, Sprite *sprite3);
  void set_sprite_right(Sprite *sprite1, Sprite *sprite2, Sprite *sprite3);
  void set_sprite_up(Sprite *sprite1, Sprite *sprite2, Sprite *sprite3);
  void set_sprite_down(Sprite *sprite1, Sprite *sprite2, Sprite *sprite3);
  uint32_t set_pos(int position);
  void compute_thumb_rect(RECT *rect);
  uint32_t set_thumb_rect();

  // ?on_left_click@Scroll@@QAEHHH@Z returns H; the body is empty and VC6
  // emits the same `ret 8`.
  int on_left_click(int a1, int a2);
  // 0x00606320 ?on_mousewheel_down@Scroll@@QAEXH@Z and
  // 0x00606440 ?on_mousewheel_up@Scroll@@QAEXH@Z - public, __thiscall,
  // void(int). Both are still unrecovered; the declarations exist because the
  // recovered Win::on_mousewheel_* wrappers call them directly, and the
  // definitions at the end of scroll.cpp forward to the original image.
  void on_mousewheel_down(int delta);
  void on_mousewheel_up(int delta);
 private:
  friend class Win;

  uint32_t field_A14_;  // 0x0a14
  uint32_t field_A18_;
  int border_color_;
  int range_minimum_;
  int range_maximum_;
  uint32_t range_reversed_;
  int position_;
  uint32_t field_A30_;
  uint32_t field_A34_;
  uint32_t field_A38_;
  uint32_t field_A3C_;
  int paging_;
  uint32_t field_A44_;
  uint32_t field_A48_;
  RECT thumb_rect_;
  uint32_t color_;
  int bar_thickness_;
  Buffer *buffer_;
  uint32_t bevel_thickness_;
  uint32_t bevel_upper_;
  uint32_t bevel_lower_;
  uint32_t field_A74_;
  Sprite *field_A78_;
  Sprite *sprite_left1_;
  Sprite *sprite_left2_;
  Sprite *sprite_left3_;
  Sprite *sprite_up1_;
  Sprite *sprite_up2_;
  Sprite *sprite_up3_;
  Sprite *sprite_right1_;
  Sprite *sprite_right2_;
  Sprite *sprite_right3_;
  Sprite *sprite_down1_;
  Sprite *sprite_down2_;
  Sprite *sprite_down3_;
  FlatButton flat_button_left_;
  FlatButton flat_button_right_;
  uint32_t field_2144_;
  uint32_t field_2148_;
};

static_assert(sizeof(Scroll) == 0x214C,
              "Scroll layout must match the legacy ABI");

inline Win *&ScrollCurrentWin() { return *reinterpret_cast<Win **>(0x009B7AB8); }
extern int ScrollDefaultThickness;
extern int ScrollNonClientInit;
// Defaults copied by close(). The static table contains eleven process
// constants; the dynamic table spans seventeen process-owned slots, including
// the sprite defaults and one intentionally unused hole.
extern uint32_t ScrollCloseStaticDefaults;
extern uint32_t ScrollCloseDynamicDefaults;
// The two virtual tables the destructor stages before running close.
// DEFINED HERE, NOT `extern`, because the value has to be VISIBLE at the use
// site to fold. 0x006051D0 stores both as immediates
// (`mov dword ptr [esi], 0x669d58`); with the definition down in scroll.cpp
// and only `extern const` in scope at the constructor, VC6 must emit a load
// from the global and then a store - two instructions where the image has one.
// A namespace-scope `const` has internal linkage, so defining it in the header
// gives every translation unit its own foldable copy rather than a symbol
// clash.
const uint32_t ScrollPrimaryVtable = 0x00669D58;
const uint32_t ScrollBufferVtable = 0x00669D50;

RECT *__cdecl expand_rect(RECT *rect, int horizontal, int vertical);
void *__fastcall scroll_scalar_dtor_redirect(Scroll *self, void *,
                                             unsigned int mode);

// The two mouse-wheel handlers are still original bodies: 286 and 284 bytes of
// paging arithmetic that reach the unrecovered thumb and repaint paths. Both
// are public, __thiscall, void(int), and both are forwarded to the original
// image. Bound through rebindable seams so tests can substitute probes and so
// each seam can later be repointed at a recovered body.
