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
#include "subinterface.h"

 /*
  * DiploPop class
  *
  * Derives from GraphicWin by ordinary single inheritance - its constructor calls
  * GraphicWin's on an unadjusted `this` - so it is written as the original wrote
  * it and needs no ABI workaround. GraphicWin is pinned here, so the base region
  * is exact; DiploPop's own extent is not established and nothing pins its
  * sizeof.
  *
  * The methods below ignore their arguments and return a constant, which is
  * why they can be replaced ahead of that mapping.
  */
class DLLEXPORT DiploPop : GraphicWin {
 public:
  void hide();
  DiploPop() { ; }
  ~DiploPop() { ; }
  void on_iface_left_click(int a1, int a2);
  void on_iface_right_click(int a1, int a2);
  void on_iface_left_down(int a1, int a2);
  void on_iface_right_down(int a1, int a2);
  void on_iface_mouse_move(int a1, int a2);
  void on_iface_mouse_leave(int a1, int a2);
  void on_iface_selected(int a1, int a2);
  void on_iface_left_double_click(int a1, int a2);
  void on_iface_right_double_click(int a1, int a2);
  void on_iface_button_toggled(int a1, int a2);

  // Storage the image proves is here: its own methods reach 0x420C.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  // 49 member(s) from the IDA database, 10 named; it starts a member at 0xA14, which is where src/ ends.

  // PROVED, and now a real type rather than eight opaque bytes:
  // src/subinterface.h measures SubInterface at 0x8 from its own
  // methods (both reach +0x4 with a 4-byte access) and asserts it, so
  // this substitution is exactly size-neutral. The byte comparison is
  // the arbiter and it does not move.
  SubInterface subIFace_;  // 0xA14
  uint8_t font1_[0x28];  // 0xA1C
  uint8_t font2_[0x28];  // 0xA44
  uint8_t font3_[0x28];  // 0xA6C
  uint8_t time1_[0x28];  // 0xA94
  uint8_t time2_[0x28];  // 0xABC
  uint8_t flic1_[0xAE4];  // 0xAE4
  uint8_t flic2_[0xAE4];  // 0x15C8
  uint32_t field_20AC_;  // 0x20AC
  uint32_t field_20B0_;  // 0x20B0
  uint32_t field_20B4_;  // 0x20B4
  uint32_t field_20B8_;  // 0x20B8
  uint32_t field_20BC_;  // 0x20BC
  uint32_t field_20C0_;  // 0x20C0
  uint32_t field_20C4_;  // 0x20C4
  uint32_t field_20C8_;  // 0x20C8
  uint32_t field_20CC_;  // 0x20CC
  uint32_t field_20D0_;  // 0x20D0
  uint32_t field_20D4_;  // 0x20D4
  uint32_t field_20D8_;  // 0x20D8
  uint32_t field_20DC_;  // 0x20DC
  uint32_t field_20E0_;  // 0x20E0
  uint32_t field_20E4_;  // 0x20E4
  uint32_t field_20E8_;  // 0x20E8
  uint32_t field_20EC_;  // 0x20EC
  uint32_t field_20F0_;  // 0x20F0
  uint8_t rect1_[0x10];  // 0x20F4
  uint32_t field_2104_;  // 0x2104
  uint32_t field_2108_;  // 0x2108
  uint32_t field_210C_;  // 0x210C
  uint32_t field_2110_;  // 0x2110
  uint32_t field_2114_;  // 0x2114
  uint32_t field_2118_;  // 0x2118
  uint32_t field_211C_;  // 0x211C
  uint32_t field_2120_;  // 0x2120
  uint32_t field_2124_;  // 0x2124
  uint32_t field_2128_;  // 0x2128
  uint32_t field_212C_;  // 0x212C
  uint32_t field_2130_;  // 0x2130
  uint32_t field_2134_;  // 0x2134
  uint32_t field_2138_;  // 0x2138
  uint32_t field_213C_;  // 0x213C
  uint32_t field_2140_;  // 0x2140
  uint32_t field_2144_;  // 0x2144
  uint32_t field_2148_;  // 0x2148
  uint32_t field_214C_;  // 0x214C
  uint32_t field_2150_;  // 0x2150
  uint32_t field_2154_;  // 0x2154
  uint8_t field_2158_[0x20B4];  // 0x2158
};

void __fastcall diplo_pop_on_iface_left_click_redirect(DiploPop *self, void *, int a1, int a2);
void __fastcall diplo_pop_on_iface_right_click_redirect(DiploPop *self, void *, int a1, int a2);
void __fastcall diplo_pop_on_iface_left_down_redirect(DiploPop *self, void *, int a1, int a2);
void __fastcall diplo_pop_on_iface_right_down_redirect(DiploPop *self, void *, int a1, int a2);
void __fastcall diplo_pop_on_iface_mouse_move_redirect(DiploPop *self, void *, int a1, int a2);
void __fastcall diplo_pop_on_iface_mouse_leave_redirect(DiploPop *self, void *, int a1, int a2);
void __fastcall diplo_pop_on_iface_selected_redirect(DiploPop *self, void *, int a1, int a2);
void __fastcall diplo_pop_on_iface_left_double_click_redirect(DiploPop *self, void *, int a1, int a2);
void __fastcall diplo_pop_on_iface_right_double_click_redirect(DiploPop *self, void *, int a1, int a2);
void __fastcall diplo_pop_on_iface_button_toggled_redirect(DiploPop *self, void *, int a1, int a2);
