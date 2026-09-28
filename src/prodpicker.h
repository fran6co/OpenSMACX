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
#include "time.h"
#include "sprite.h"
#include "scroll.h"
#include "caviar.h"
#include "font.h"
#include "flatbutton.h"
#include "graphicwin.h"
#include "stringstruct.h"

 /*
  * ProdPicker class
  *
  * Uses ConstructedGraphicWin to express the image's explicit GraphicWin
  * construction before the real embedded members. GraphicWin's base region is
  * pinned; ProdPicker's member extent is now layout-asserted at 0xA840.
  *
  * Several methods remain simple stubs, while close and lifecycle now use
  * source-owned state. The member map is based on the constructor/destructor
  * sequence and methods that reach those offsets.
  */
class ProdPicker : public ConstructedGraphicWin {
 public:
  ProdPicker();
  // The destructor closes picker behavior before its typed members tear down.
  ~ProdPicker();
  int UNK1(int pos);
  void on_mouse_move(int a1, int a2);
  void UNK3(int a1);
  void close();
  void draw_item(int item, int update);
  int edbc_fac(int facility_id);   // 0x00493B70, editor category
  int edbc_unit(int prototype_id); // 0x00493AF0, prototype category
  void on_mousewheel(int delta);   // 0x00493C20, embedded Scroll
  void on_left_click(int x, int y);
  void on_redraw();
  void on_left_double_click(int x, int y);
  void on_button_clicked(int button_id);
  void on_scrolled(int code, int position);
  // PAURECT@@H, per ?on_redraw_nc@ProdPicker@@QAEXPAURECT@@H@Z; the body is
  // `ret 8` either way and only the decorated name moves.
  void on_redraw_nc(RECT *, int);

 private:
  int32_t field_A14_;   // first field past the pinned GraphicWin base

  // Partial named field coverage within a class whose total image extent is
  // asserted below; several interior regions remain byte storage pending
  // stronger member evidence.

  // Real members keep the image's address layout and, with
  // ConstructedGraphicWin as the base, construct after the GraphicWin base.
  // Their declaration order matches the observed constructor order. This
  // intentionally replaces raw storage/placement-new with C++ lifetime.
  Sprite sprites_[3];  // 0xA18
  Font font1_;  // 0xA9C
  Font font2_;  // 0xAC4
  Font font3_;  // 0xAEC
  Font font4_;  // 0xB14
  Time time_;  // 0xB3C
  uint32_t field_B64_;  // 0xB64
  uint32_t field_B68_;  // 0xB68
  uint32_t field_B6C_;  // 0xB6C
  uint32_t field_B70_;  // 0xB70
  FlatButton flatButtons_[9];  // 0xB74
  Scroll scroll_;  // 0x7120

  Caviar caviar_;  // 0x926C

  // Remaining interior fields retain measured offsets and byte extent where
  // no stronger source type is established.
  uint8_t field_A63C_[0x100];  // 0xA63C
  uint32_t field_A73C_;  // 0xA73C
  uint32_t field_A740_;  // 0xA740
  uint32_t field_A744_;  // 0xA744
  uint8_t field_A748_[0x18];  // 0xA748
  float field_A760_;  // 0xA760
  float field_A764_;  // 0xA764
  float field_A768_;  // 0xA768
  uint32_t field_A76C_;  // 0xA76C
  uint32_t field_A770_;  // 0xA770
  uint32_t field_A774_;  // 0xA774
  uint32_t field_A778_;  // 0xA778
  uint32_t field_A77C_;  // 0xA77C
  // THE THREE ITEM LISTS, carved 2026-09-01 from what was the thirty-six
  // dwords field_A780_..field_A80C_: three consecutive StringLists (3 * 0x30),
  // proven by prod_picker_unknown_a7_block's own store sequence - vbptr
  // ({-4, +0x24}, BasePop's copy of StringList's vbtable), the
  // StringAllocationBase capture at +0x28/+0x2C, the primary vptr, the five
  // list-field zeros, then the derived-table refresh - exactly the
  // compiler-emitted StringList construction the declaration below now owns.
  // Real members: their construction/destruction follows C++ declaration
  // order. The earlier placement-new experiment's byte score is historical;
  // structural ownership takes precedence over that local similarity result.
  StringList prod_a_;  // 0xA780
  StringList prod_b_;  // 0xA7B0
  StringList prod_c_;  // 0xA7E0
  uint32_t field_A810_;  // 0xA810
  uint32_t field_A814_;  // 0xA814
  uint32_t field_A818_;  // 0xA818
  uint32_t field_A81C_;  // 0xA81C
  uint32_t field_A820_;  // 0xA820
  uint32_t field_A824_;  // 0xA824
  uint32_t field_A828_;  // 0xA828
  uint32_t field_A82C_;  // 0xA82C
  uint32_t field_A830_;  // 0xA830
  uint32_t field_A834_;  // 0xA834
  uint32_t field_A838_;  // 0xA838
  uint32_t field_A83C_;  // 0xA83C
};

static_assert(sizeof(ProdPicker) == 0xA840,
              "ProdPicker layout must match the original executable");
