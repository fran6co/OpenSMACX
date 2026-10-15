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
// GraphicWin is the genuine base. C++ constructs it before Scroll's
// FlatButton members, preserving the pinned base-before-member order.
class Scroll : public GraphicWin {
  friend class ReportWin;  // ReportWin::UNK1 sets this embedded scroll's paging_ field
  friend class Gamma;  // Gamma::exec configures its embedded scroll's defaults
 public:
  // 0x006051D0. Defined in scroll.cpp, alongside close() which resets the
  // same fields from the same two process-default tables.
  Scroll();
  ~Scroll();

  uint32_t close();
  // 0x006054D0 is the primary initializer. The overloads that wrap it call
  // it BY NAME, so they emit the image's `call rel32` rather than the
  // `call [ptr]` a function-pointer seam compiles.
  int init(int x, int y, int width, int height, Win *parent,
           int setting, int options);
  int init(RECT *rect, Win *parent, int setting, int options);
  int init_vert(int x, int y, int length, Win *parent, int setting);
  int init_horz(int x, int y, int length, Win *parent, int setting);
  int init_vert_nc(int x, int y, int length, Win *parent, int setting);
  int init_horz_nc(int x, int y, int length, Win *parent, int setting);
  void set_range(int minimum, int maximum);
  void set_button_color(int color);
  void set_bevel_thickness(int thickness);
  void set_bevel_upper(int color);
  void set_bevel_lower(int color);
  uint32_t set_bar_thickness(int thickness);
  void set_border_color(int color);
  void set_sprite_left(Sprite *sprite1, Sprite *sprite2, Sprite *sprite3);
  void set_sprite_right(Sprite *sprite1, Sprite *sprite2, Sprite *sprite3);
  void set_sprite_up(Sprite *sprite1, Sprite *sprite2, Sprite *sprite3);
  void set_sprite_down(Sprite *sprite1, Sprite *sprite2, Sprite *sprite3);
  uint32_t set_pos(int position);
  int position() const { return position_; }
  int paging() const { return paging_; }
  void set_paging(int value) { paging_ = value; }
  void set_field_A30(int value) { field_A30_ = static_cast<uint32_t>(value); }
  void compute_thumb_rect(RECT *rect);
  uint32_t set_thumb_rect();

  // The catalogued int prototype is historical. This is the inherited
  // void Win slot-19 callback, whose pinned body returns with ret 8.
  void on_left_click(int a1, int a2) override;
  void on_mouse_move(int x, int y);
  void on_left_down(int x, int y);
  void on_left_up(int x, int y);
  // Input handlers reached by recovered Win and FlatButton dispatch paths.
  void on_mousewheel_down(int delta);
  void on_mousewheel_up(int delta);
  int on_key_click(int repeat, int key) override;
  int on_button_pressed(int button_id) override;
  void on_button_clicked(int button_id);
  void on_redraw();
  void on_resize(int width, int height);
 private:
  friend class Win;
  friend class SelectPartWin;  // key navigation reads the clamped position after set_pos
  // StringBox embeds this Scroll and the original fit/draw paths read its
  // live range, position, paging and bar-thickness words directly.
  friend class StringBox;

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
  // Gamma::exec writes palette index 0xF8 here; this word is not a Buffer*.
  // RECOVERY_MEMBER: v1 Scroll field_A64_ 0xA64 0x005C8E60
  int field_A64_;
  uint32_t bevel_thickness_;
  uint32_t bevel_upper_;
  uint32_t bevel_lower_;
  // RECOVERY_MEMBER: v3 Scroll thumb_sprite_ Sprite* 0xA74 0x006066F0
  Sprite *thumb_sprite_;
  Sprite *field_A78_;
  Sprite *sprite_left_[3];
  Sprite *sprite_up_[3];
  Sprite *sprite_right_[3];
  Sprite *sprite_down_[3];
  FlatButton flat_button_left_;
  FlatButton flat_button_right_;
  void (__cdecl *field_2144_)(int, int);
  void (__cdecl *field_2148_)(int, int);
};

static_assert(sizeof(Scroll) == 0x214C,
              "Scroll layout must match the legacy ABI");

extern int ScrollDefaultThickness;
extern int ScrollNonClientInit;
// Defaults copied by construction and close(). These named fields represent
// only the eleven observed words at 0x697020..0x69704B; they do not claim the
// original aggregate type or its complete allocation boundary.
struct ScrollCloseStaticDefaultsData {
  int paging;
  uint32_t field_A34;
  uint32_t field_A30;
  int border_color;
  int range_maximum;
  uint32_t field_A48;
  uint32_t color;
  int field_A64;
  uint32_t bevel_thickness;
  uint32_t bevel_upper;
  uint32_t bevel_lower;
};
static_assert(sizeof(ScrollCloseStaticDefaultsData) == 11 * sizeof(uint32_t),
              "Scroll defaults must retain the observed eleven-word window");
extern ScrollCloseStaticDefaultsData ScrollCloseStaticDefaults;
struct ScrollCloseDynamicDefaultsData {
  uint32_t flags;
  uint32_t position;
  Sprite *sprite_left[3];
  Sprite *sprite_up[3];
  Sprite *sprite_right[3];
  Sprite *sprite_down[3];
  uint32_t reserved_38;
  Sprite *thumb;
  Sprite *background;
};
extern ScrollCloseDynamicDefaultsData ScrollCloseDynamicDefaults;
void __cdecl scroll_normal();
void __cdecl scroll_small();
RECT *__cdecl expand_rect(RECT *rect, int horizontal, int vertical);
