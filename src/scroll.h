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
#include "buffer.h"
#include "flatbutton.h"
#include "graphicwin.h"
#include "sprite.h"

 /*
  * Scroll class
  */
class DLLEXPORT Scroll : GraphicWin {
 public:
  Scroll() { ; }
  ~Scroll() { ; }

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

 private:
  friend class Win;

  uint32_t field_A14_;
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

extern Win **ScrollCurrentWin;

RECT *__cdecl expand_rect(RECT *rect, int horizontal, int vertical);
uint32_t __fastcall scroll_set_range_redirect(
    Scroll *self, void *, int minimum, int maximum);
uint32_t __fastcall scroll_set_button_color_redirect(
    Scroll *self, void *, int color);
uint32_t __fastcall scroll_set_bevel_thickness_redirect(
    Scroll *self, void *, int thickness);
uint32_t __fastcall scroll_set_bevel_upper_redirect(
    Scroll *self, void *, int color);
uint32_t __fastcall scroll_set_bevel_lower_redirect(
    Scroll *self, void *, int color);
uint32_t __fastcall scroll_set_bar_thickness_redirect(
    Scroll *self, void *, int thickness);
void __fastcall scroll_set_border_color_redirect(
    Scroll *self, void *, int color);
Sprite *__fastcall scroll_set_sprite_left_redirect(
    Scroll *self, void *, Sprite *sprite1, Sprite *sprite2, Sprite *sprite3);
Sprite *__fastcall scroll_set_sprite_right_redirect(
    Scroll *self, void *, Sprite *sprite1, Sprite *sprite2, Sprite *sprite3);
Sprite *__fastcall scroll_set_sprite_up_redirect(
    Scroll *self, void *, Sprite *sprite1, Sprite *sprite2, Sprite *sprite3);
Sprite *__fastcall scroll_set_sprite_down_redirect(
    Scroll *self, void *, Sprite *sprite1, Sprite *sprite2, Sprite *sprite3);
uint32_t __fastcall scroll_set_pos_redirect(
    Scroll *self, void *, int position);
RECT *__fastcall scroll_compute_thumb_rect_redirect(
    Scroll *self, void *, RECT *rect);
uint32_t __fastcall scroll_set_thumb_rect_redirect(Scroll *self, void *);
