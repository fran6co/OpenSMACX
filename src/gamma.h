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
#include "font.h"
#include "graphicwin.h"

 /*
  * Gamma class
  *
  * Derives from GraphicWin by ordinary single inheritance - its constructor calls
  * GraphicWin's on an unadjusted `this` - so it is written as the original wrote
  * it and needs no ABI workaround. GraphicWin is pinned here, so the base region
  * is exact; Gamma's own extent is not established and nothing pins its
  * sizeof.
  *
  * The methods below ignore their arguments and return a constant, which is
  * why they can be replaced ahead of that mapping.
  */
class DLLEXPORT Gamma : GraphicWin {
 public:
  void on_scrolled(int a1, int a2);
  Gamma() { ; }
  ~Gamma() { ; }
  int on_key_click(int a1, int a2);

  // Storage the image proves is here: its own methods reach 0x1920.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  // 24 member(s) from the IDA database, 4 named; it starts a member at 0xA14, which is where src/ ends.

  uint8_t rect1_[0x10];  // 0xA14
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
  uint8_t palette_[0x454];  // 0xA68
  Font font_;  // 0xEBC
  uint32_t field_EE4_;  // 0xEE4
  uint32_t field_EE8_;  // 0xEE8
  uint32_t field_EEC_;  // 0xEEC
  uint8_t field_EF0_[0xA30];  // 0xEF0

  // Storage the image proves is here: its own methods reach 0x46B4.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  uint8_t field_1920_[0x2D94];
};

int __fastcall gamma_on_key_click_redirect(Gamma *self, void *, int a1, int a2);

// ?adjust_palette@Gamma@@QAEXXZ (0x005C9520) is not recovered. Gamma::on_scrolled
// is nothing but a call to it, so the call goes through a rebindable seam
// rather than a member this DLL cannot define.
typedef void (OriginalObject::*func_gamma_adjust_palette)();
extern func_gamma_adjust_palette GammaOriginalAdjustPalette;
