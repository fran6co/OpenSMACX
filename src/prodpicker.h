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
#include "graphicwin.h"

 /*
  * ProdPicker class
  *
  * Derives from GraphicWin by ordinary single inheritance - its constructor calls
  * GraphicWin's on an unadjusted `this` - so it is written as the original wrote
  * it and needs no ABI workaround. GraphicWin is pinned here, so the base region
  * is exact; ProdPicker's own extent is not established and nothing pins its
  * sizeof.
  *
  * Most methods below ignore their arguments and return a constant, which is
  * why they could be replaced ahead of that mapping. close reaches one real
  * field: because GraphicWin is pinned at exactly 0xA14, the dword it clears
  * at 0xA14 is ProdPicker's own first field, which is exact rather than a
  * guess. Nothing past it is established.
  */
class DLLEXPORT ProdPicker : GraphicWin {
 public:
  ProdPicker() { ; }
  ~ProdPicker() { ; }
  int UNK1(int a1);
  void on_mouse_move(int a1, int a2);
  void UNK3(int a1);
  void close();
  void on_redraw_nc(int, int);

 private:
  int32_t field_A14_;   // first field past the pinned GraphicWin base

  // Storage the image proves is here: its own methods reach 0x7124.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  // 22 member(s) from the IDA database, 18 named; it starts a member at 0xA18, which is where src/ ends.

  uint8_t sprite1_[0x2C];  // 0xA18
  uint8_t sprite2_[0x2C];  // 0xA44
  uint8_t sprite3_[0x2C];  // 0xA70
  uint8_t font1_[0x28];  // 0xA9C
  uint8_t font2_[0x28];  // 0xAC4
  uint8_t font3_[0x28];  // 0xAEC
  uint8_t font4_[0x28];  // 0xB14
  uint8_t time_[0x28];  // 0xB3C
  uint32_t field_B64_;  // 0xB64
  uint32_t field_B68_;  // 0xB68
  uint32_t field_B6C_;  // 0xB6C
  uint32_t field_B70_;  // 0xB70
  uint8_t flatButton1_[0xB4C];  // 0xB74
  uint8_t flatButton2_[0xB4C];  // 0x16C0
  uint8_t flatButton3_[0xB4C];  // 0x220C
  uint8_t flatButton4_[0xB4C];  // 0x2D58
  uint8_t flatButton5_[0xB4C];  // 0x38A4
  uint8_t flatButton6_[0xB4C];  // 0x43F0
  uint8_t flatButton7_[0xB4C];  // 0x4F3C
  uint8_t flatButton8_[0xB4C];  // 0x5A88
  uint8_t flatButton9_[0xB4C];  // 0x65D4
  uint8_t field_7120_[0x4];  // 0x7120

  // Storage the image proves is here: its own methods reach 0xA840.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  uint8_t field_7124_[0x371C];
};

static_assert(sizeof(ProdPicker) == 0xA840,
              "ProdPicker layout must match the original executable");

int __fastcall prod_picker_unk1_redirect(ProdPicker *self, void *, int a1);
void __fastcall prod_picker_on_mouse_move_redirect(ProdPicker *self, void *, int a1, int a2);
void __fastcall prod_picker_unk3_redirect(ProdPicker *self, void *, int a1);
void __fastcall prod_picker_close_redirect(ProdPicker *self, void *);
void __fastcall prod_picker_on_redraw_nc_redirect(
    ProdPicker *self, void *, int a1, int a2);
