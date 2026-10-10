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
#include <new>
#include <stddef.h>

// Two columns of three technology-popup icons, populated by load_box_sprites.
// The separate original guard at 0x822604 is not part of NewTechWindow.
extern uint8_t NewTechSpritesCleanupGuard;
struct NewTechSpriteArray {
  Sprite elements_[6];
};
union NewTechSpriteStorage {
  char bytes[sizeof(NewTechSpriteArray)];
  uint32_t alignment_[sizeof(NewTechSpriteArray) / sizeof(uint32_t)];
};
// Chosen lifetime implementation, not a recovered original class name.
// A real embedded array gives the compiler prefix unwind and reverse cleanup;
// placement construction of its enclosing aggregate adds no array cookie.
struct NewTechSpriteLifetime {
  NewTechSpriteStorage storage_;
  NewTechSpriteLifetime() { new (storage_.bytes) NewTechSpriteArray; }
  ~NewTechSpriteLifetime() {
    if ((NewTechSpritesCleanupGuard & 1) != 0) return;
    NewTechSpritesCleanupGuard |= 1;
    array().NewTechSpriteArray::~NewTechSpriteArray();
  }
  NewTechSpriteArray &array() {
    return *reinterpret_cast<NewTechSpriteArray *>(storage_.bytes);
  }
  Sprite &at(int index) { return array().elements_[index]; }
};
static_assert(sizeof(NewTechSpriteLifetime) == 6 * sizeof(Sprite),
              "six Sprite lifetime storage extent");
static_assert(offsetof(NewTechSpriteLifetime, storage_) == 0 &&
              offsetof(NewTechSpriteArray, elements_) == 0,
              "technology Sprite array starts at its owner identity");
extern NewTechSpriteLifetime NewTechSpritesOwner;

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
  NewTechWin();
  void exec(int technology_id, int other_faction_id);
  // Real member/base destruction is emitted from the out-of-line body.
  ~NewTechWin();
  virtual int on_link_clicked(int id, char *link);
  virtual void on_redraw();  // original vtable slot 12, 0x00483950
  void on_left_click(int a1, int a2) override;
  virtual void on_button_clicked(int button_id);
  void on_group_clicked(int, int);
  int on_key_click(int a, int b);

  // Original exec uses these rectangle words and technology/faction IDs.
  // Lifecycle accesses establish the embedded arrays and final object end.

  uint32_t field_A14_;  // 0xA14
  uint32_t field_A18_;  // 0xA18
  uint32_t field_A1C_;  // 0xA1C
  uint32_t field_A20_;  // 0xA20
  uint32_t field_A24_;  // 0xA24
  uint32_t field_A28_;  // 0xA28
  uint32_t field_A2C_;  // 0xA2C
  uint32_t field_A30_;  // 0xA30
  uint32_t field_A34_;  // 0xA34
  uint32_t field_A38_;  // 0xA38
  uint32_t field_A3C_;  // 0xA3C
  uint32_t field_A40_;  // 0xA40
  uint32_t field_A44_;  // 0xA44
  uint32_t field_A48_;  // 0xA48
  uint32_t field_A4C_;  // 0xA4C
  uint32_t field_A50_;  // 0xA50
  uint32_t field_A54_;  // 0xA54
  uint32_t field_A58_;  // 0xA58
  uint32_t field_A5C_;  // 0xA5C
  uint32_t field_A60_;  // 0xA60
  uint32_t field_A64_;  // 0xA64
  uint32_t field_A68_;  // 0xA68
  uint32_t field_A6C_;  // 0xA6C
  uint32_t field_A70_;  // 0xA70
  uint32_t field_A74_;  // 0xA74
  uint32_t field_A78_;  // 0xA78
  uint32_t field_A7C_;  // 0xA7C
  uint32_t field_A80_;  // 0xA80
  uint32_t field_A84_;  // 0xA84
  uint32_t field_A88_;  // 0xA88
  uint32_t field_A8C_;  // 0xA8C
  uint32_t field_A90_;  // 0xA90
  uint32_t field_A94_;  // 0xA94
  uint32_t field_A98_;  // 0xA98
  uint32_t field_A9C_;  // 0xA9C
  uint32_t field_AA0_;  // 0xAA0
  uint32_t field_AA4_;  // 0xAA4
  uint32_t field_AA8_;  // 0xAA8
  uint32_t field_AAC_;  // 0xAAC
  uint32_t field_AB0_;  // 0xAB0
  uint32_t field_AB4_;  // 0xAB4
  uint32_t field_AB8_;  // 0xAB8
  uint32_t field_ABC_;  // 0xABC
  uint32_t field_AC0_;  // 0xAC0
  uint32_t field_AC4_;  // 0xAC4
  uint32_t field_AC8_;  // 0xAC8
  uint32_t field_ACC_;  // 0xACC
  uint32_t field_AD0_;  // 0xAD0
  uint32_t field_AD4_;  // 0xAD4
  uint32_t field_AD8_;  // 0xAD8
  uint32_t field_ADC_;  // 0xADC
  uint32_t field_AE0_;  // 0xAE0
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

// Original technology-popup singleton, owned by newtechwin.cpp.
extern NewTechWin NewTechWindow;
