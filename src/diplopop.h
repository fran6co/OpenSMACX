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
#include "font.h"
#include "graphicwin.h"
#include "subinterface.h"
#include "flic.h"
#include "flatbutton.h"

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
class DiploPop : GraphicWin, SubInterface {
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
  // The SubInterface that used to sit here as a member is now the SECOND
  // BASE, above. MSVC lays non-virtual bases in declaration order, so it is
  // at sizeof(GraphicWin) == 0xA14 either way and no offset moves.
  Font font1_;  // 0xA1C
  Font font2_;  // 0xA44
  Font font3_;  // 0xA6C
  Time time1_;  // 0xA94
  Time time2_;  // 0xABC
  Flic flic1_;  // 0xAE4, declared Flic extent == 0xAE4
  Flic flic2_;  // 0x15C8, declared Flic extent == 0xAE4
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
  RECT rect1_;  // 0x20F4, IDB `rect1`
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
  // The IDB's last member: three FlatButtons ending at 0x433C, which is
  // 0x130 past the hole this used to declare; the class carries no size
  // assertion, so the declaration follows the member table.
  FlatButton flatButtons_[3];  // 0x2158, IDB `flatButtons`, 3 * sizeof(FlatButton)
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
