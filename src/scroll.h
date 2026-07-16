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

  void set_border_color(int color);
  void set_sprite_left(Sprite *sprite1, Sprite *sprite2, Sprite *sprite3);
  void set_sprite_right(Sprite *sprite1, Sprite *sprite2, Sprite *sprite3);
  void compute_thumb_rect(RECT *rect);

 private:
  friend class Win;

  uint32_t field_A14_;
  uint32_t field_A18_;
  int border_color_;
  uint32_t field_A20_;
  uint32_t field_A24_;
  uint32_t field_A28_;
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
  uint32_t field_A88_;
  uint32_t field_A8C_;
  uint32_t field_A90_;
  Sprite *sprite_right1_;
  Sprite *sprite_right2_;
  Sprite *sprite_right3_;
  uint32_t field_AA0_;
  uint32_t field_AA4_;
  uint32_t field_AA8_;
  FlatButton flat_button_left_;
  FlatButton flat_button_right_;
  uint32_t field_2144_;
  uint32_t field_2148_;
};

static_assert(sizeof(Scroll) == 0x214C,
              "Scroll layout must match the legacy ABI");

void __fastcall scroll_set_border_color_redirect(
    Scroll *self, void *, int color);
Sprite *__fastcall scroll_set_sprite_left_redirect(
    Scroll *self, void *, Sprite *sprite1, Sprite *sprite2, Sprite *sprite3);
Sprite *__fastcall scroll_set_sprite_right_redirect(
    Scroll *self, void *, Sprite *sprite1, Sprite *sprite2, Sprite *sprite3);
RECT *__fastcall scroll_compute_thumb_rect_redirect(
    Scroll *self, void *, RECT *rect);
