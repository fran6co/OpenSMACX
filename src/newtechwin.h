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
#include "flatbutton.h"
#include "buttongroup.h"
#include "graphicwin.h"
#include "stringbox.h"
#include "sprite.h"

 /*
  * NewTechWin class
  *
  * The original constructor calls GraphicWin on the unadjusted receiver,
  * then constructs three FlatButtons, a ButtonGroup, four StringBoxes and
  * the final StringBox. Ordinary C++ inheritance and members own that lifetime;
  * the singleton and construction/destruction evidence live in newtechwin.cpp.
  */
class NewTechWin : public GraphicWin {
 public:
  // The six technology-popup icons are process-start storage owned with the
  // technology window's source contribution, not instance data.
  static Sprite technology_sprites_[6];

  NewTechWin();
  void exec(int technology_id, int other_faction_id);
  // Real member/base destruction is emitted from the out-of-line body.
  ~NewTechWin();
  virtual int on_link_clicked(int id, char *link);
  virtual void on_redraw();  // original vtable slot 12, 0x00483950
  void on_left_click(int a1, int a2) override;
  virtual void on_button_clicked(int button_id);
  void on_group_clicked(int, int) override;
  int on_key_click(int a, int b);

  // Original exec uses these rectangle words and technology/faction IDs.
  // Lifecycle accesses establish the embedded arrays and final object end.

  uint32_t field_A14_;  // 0xA14
  uint32_t field_A18_;  // 0xA18
  uint32_t field_A1C_;  // 0xA1C
  uint32_t field_A20_;  // 0xA20
  // Each mapped redraw call passes the address of this four-word object
  // directly as RECT*. The observed address/extent does not establish the
  // original source member spelling.
  // RECOVERY_MEMBER: v2 NewTechWin main_rect_ tagRECT 0xA24 0x004839A3
  RECT main_rect_;  // 0xA24
  // RECOVERY_MEMBER: v2 NewTechWin title_rect_ tagRECT 0xA34 0x00483A17
  RECT title_rect_;  // 0xA34
  // RECOVERY_MEMBER: v2 NewTechWin category_rect_ tagRECT 0xA44 0x00483CA9
  RECT category_rect_;  // 0xA44
  // RECOVERY_MEMBER: v4 NewTechWin row_rects_ tagRECT[4] 0xA54 0x40 0x00483C5D
  RECT row_rects_[4];  // 0xA54
  // RECOVERY_MEMBER: v4 NewTechWin label_rects_ tagRECT[4] 0xA94 0x40 0x00483AAE
  RECT label_rects_[4];  // 0xA94
  // RECOVERY_MEMBER: v2 NewTechWin bottom_rect_ tagRECT 0xAD4 0x00483E73
  RECT bottom_rect_;  // 0xAD4
  uint32_t field_AE4_;  // 0xAE4
  uint32_t field_AE8_;  // 0xAE8
  uint32_t field_AEC_;  // 0xAEC
  uint32_t field_AF0_;  // 0xAF0
  uint32_t field_AF4_;  // 0xAF4
  uint32_t field_AF8_;  // 0xAF8
  uint32_t field_AFC_;  // 0xAFC
  uint32_t field_B00_;  // 0xB00
  uint32_t field_B04_;  // 0xB04
  uint32_t field_B08_;  // 0xB08
  uint32_t field_B0C_;  // 0xB0C
  uint32_t field_B10_;  // 0xB10
  uint32_t field_B14_;  // 0xB14
  uint32_t field_B18_;  // 0xB18
  uint32_t field_B1C_;  // 0xB1C
  uint32_t field_B20_;  // 0xB20
  uint32_t field_B24_;  // 0xB24
  uint32_t field_B28_;  // 0xB28
  FlatButton flatButtons_[3];  // 0xB2C
  ButtonGroup buttonGroup_;  // 0x2D10
  StringBox stringBoxes_[4];  // 0x2DA4, 4 * sizeof(StringBox) == 0xAE80
  StringBox stringBox2_;  // 0xDC24, size == sizeof(StringBox)
};

static_assert(sizeof(NewTechWin) == 0x107C4,
              "NewTechWin layout must match the original executable");
static_assert(sizeof(NewTechWin::technology_sprites_) == 6 * sizeof(Sprite),
              "technology popup storage is six contiguous Sprites");

// Original technology-popup singleton, owned by newtechwin.cpp.
extern NewTechWin NewTechWindow;
