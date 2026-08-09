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
#include "spot.h"
#include "font.h"
#include "flatbutton.h"
#include "graphicwin.h"
#include "subinterface.h"

 /*
  * CouncWin class
  *
  * Derives from GraphicWin by ordinary single inheritance - its constructor calls
  * GraphicWin's on an unadjusted `this` - so it is written as the original wrote
  * it and needs no ABI workaround. GraphicWin is pinned here, so the base region
  * is exact; CouncWin's own extent is not established and nothing pins its
  * sizeof.
  *
  * The methods below ignore their arguments and return a constant, which is
  * why they can be replaced ahead of that mapping.
  */
class DLLEXPORT CouncWin : GraphicWin {
 public:
  // 0x00425DB0  ?draw_leader@CouncWin@@QAEXH@Z - public, __thiscall,
  // void(int), unrecovered. Declared so draw_all_leaders and on_mouse_leave,
  // which each reach it with a direct `call rel32`, can name it.
  void draw_leader(int factionID);
  int show(int a1);
  void draw_all_leaders();
  void on_mouse_leave(int a1, int a2);
  CouncWin() { ; }
  ~CouncWin() { ; }
  void on_group_clicked(int a1, int a2);
  void on_button_passover(int a1, int a2);
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
  void on_iface_group_clicked(int a1, int a2, int a3);
  int on_button_text_draw(int, int, int);

  // Storage the image proves is here: its own methods reach 0xA38.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  // 8 member(s) from the IDA database, 1 named; it starts a member at 0xA14, which is where src/ ends.

  // PROVED, and now a real type rather than eight opaque bytes:
  // src/subinterface.h measures SubInterface at 0x8 from its own
  // methods (both reach +0x4 with a 4-byte access) and asserts it, so
  // this substitution is exactly size-neutral. The byte comparison is
  // the arbiter and it does not move.
  SubInterface subIFace_;  // 0xA14
  uint32_t field_A1C_;  // 0xA1C
  uint32_t field_A20_;  // 0xA20
  uint32_t field_A24_;  // 0xA24
  uint32_t field_A28_;  // 0xA28
  uint32_t field_A2C_;  // 0xA2C
  uint32_t field_A30_;  // 0xA30
  uint32_t field_A34_;  // 0xA34

  // Storage the image proves is here: its own methods reach 0x4614.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  // 165 member(s) from the IDA database, 13 named; it starts a member at 0xA38, which is where src/ ends.

  uint32_t field_A38_;  // 0xA38
  uint32_t field_A3C_;  // 0xA3C
  uint32_t field_A40_;  // 0xA40
  uint32_t field_A44_;  // 0xA44
  Spot spot_;  // 0xA48
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
  Font font1_;  // 0xAB4
  Font font2_;  // 0xADC
  Font font3_;  // 0xB04
  Font font4_;  // 0xB2C
  Font font5_;  // 0xB54
  Font font6_;  // 0xB7C
  uint32_t field_BA4_;  // 0xBA4
  uint32_t field_BA8_;  // 0xBA8
  uint32_t field_BAC_;  // 0xBAC
  uint32_t field_BB0_;  // 0xBB0
  uint32_t field_BB4_;  // 0xBB4
  uint32_t field_BB8_;  // 0xBB8
  uint32_t field_BBC_;  // 0xBBC
  uint32_t field_BC0_;  // 0xBC0
  uint32_t field_BC4_;  // 0xBC4
  uint32_t field_BC8_;  // 0xBC8
  uint32_t field_BCC_;  // 0xBCC
  uint32_t field_BD0_;  // 0xBD0
  uint32_t field_BD4_;  // 0xBD4
  uint32_t field_BD8_;  // 0xBD8
  uint32_t field_BDC_;  // 0xBDC
  uint32_t field_BE0_;  // 0xBE0
  uint32_t field_BE4_;  // 0xBE4
  uint32_t field_BE8_;  // 0xBE8
  uint32_t field_BEC_;  // 0xBEC
  uint32_t field_BF0_;  // 0xBF0
  uint32_t field_BF4_;  // 0xBF4
  uint32_t field_BF8_;  // 0xBF8
  uint32_t field_BFC_;  // 0xBFC
  uint32_t field_C00_;  // 0xC00
  uint32_t field_C04_;  // 0xC04
  uint32_t field_C08_;  // 0xC08
  uint32_t field_C0C_;  // 0xC0C
  uint32_t field_C10_;  // 0xC10
  uint32_t field_C14_;  // 0xC14
  uint32_t field_C18_;  // 0xC18
  uint32_t field_C1C_;  // 0xC1C
  uint32_t field_C20_;  // 0xC20
  uint32_t field_C24_;  // 0xC24
  uint32_t field_C28_;  // 0xC28
  uint32_t field_C2C_;  // 0xC2C
  uint32_t field_C30_;  // 0xC30
  uint32_t field_C34_;  // 0xC34
  uint32_t field_C38_;  // 0xC38
  uint32_t field_C3C_;  // 0xC3C
  uint32_t field_C40_;  // 0xC40
  uint32_t field_C44_;  // 0xC44
  uint32_t field_C48_;  // 0xC48
  uint32_t field_C4C_;  // 0xC4C
  uint32_t field_C50_;  // 0xC50
  uint32_t field_C54_;  // 0xC54
  uint32_t field_C58_;  // 0xC58
  uint32_t field_C5C_;  // 0xC5C
  uint32_t field_C60_;  // 0xC60
  uint32_t field_C64_;  // 0xC64
  uint32_t field_C68_;  // 0xC68
  uint32_t field_C6C_;  // 0xC6C
  uint32_t field_C70_;  // 0xC70
  uint32_t field_C74_;  // 0xC74
  uint32_t field_C78_;  // 0xC78
  uint32_t field_C7C_;  // 0xC7C
  uint32_t field_C80_;  // 0xC80
  uint32_t field_C84_;  // 0xC84
  uint32_t field_C88_;  // 0xC88
  uint32_t field_C8C_;  // 0xC8C
  uint32_t field_C90_;  // 0xC90
  uint32_t field_C94_;  // 0xC94
  uint32_t field_C98_;  // 0xC98
  uint32_t field_C9C_;  // 0xC9C
  uint32_t field_CA0_;  // 0xCA0
  uint32_t field_CA4_;  // 0xCA4
  uint32_t field_CA8_;  // 0xCA8
  uint32_t field_CAC_;  // 0xCAC
  uint32_t field_CB0_;  // 0xCB0
  uint32_t field_CB4_;  // 0xCB4
  uint32_t field_CB8_;  // 0xCB8
  uint32_t field_CBC_;  // 0xCBC
  uint32_t field_CC0_;  // 0xCC0
  uint32_t field_CC4_;  // 0xCC4
  uint32_t field_CC8_;  // 0xCC8
  uint32_t field_CCC_;  // 0xCCC
  uint32_t field_CD0_;  // 0xCD0
  uint32_t field_CD4_;  // 0xCD4
  uint32_t field_CD8_;  // 0xCD8
  uint32_t field_CDC_;  // 0xCDC
  uint32_t field_CE0_;  // 0xCE0
  uint32_t field_CE4_;  // 0xCE4
  uint32_t field_CE8_;  // 0xCE8
  uint32_t field_CEC_;  // 0xCEC
  uint32_t field_CF0_;  // 0xCF0
  uint32_t field_CF4_;  // 0xCF4
  uint32_t field_CF8_;  // 0xCF8
  uint32_t field_CFC_;  // 0xCFC
  uint32_t field_D00_;  // 0xD00
  uint32_t field_D04_;  // 0xD04
  uint32_t field_D08_;  // 0xD08
  uint32_t field_D0C_;  // 0xD0C
  uint32_t field_D10_;  // 0xD10
  uint32_t field_D14_;  // 0xD14
  uint32_t field_D18_;  // 0xD18
  uint32_t field_D1C_;  // 0xD1C
  uint32_t field_D20_;  // 0xD20
  uint32_t field_D24_;  // 0xD24
  uint32_t field_D28_;  // 0xD28
  uint32_t field_D2C_;  // 0xD2C
  uint32_t field_D30_;  // 0xD30
  uint32_t field_D34_;  // 0xD34
  uint32_t field_D38_;  // 0xD38
  uint32_t field_D3C_;  // 0xD3C
  uint32_t field_D40_;  // 0xD40
  uint32_t field_D44_;  // 0xD44
  uint32_t field_D48_;  // 0xD48
  uint32_t field_D4C_;  // 0xD4C
  uint32_t field_D50_;  // 0xD50
  uint32_t field_D54_;  // 0xD54
  uint32_t field_D58_;  // 0xD58
  uint32_t field_D5C_;  // 0xD5C
  uint32_t field_D60_;  // 0xD60
  uint32_t field_D64_;  // 0xD64
  uint32_t field_D68_;  // 0xD68
  uint32_t field_D6C_;  // 0xD6C
  uint32_t field_D70_;  // 0xD70
  uint32_t field_D74_;  // 0xD74
  uint32_t field_D78_;  // 0xD78
  uint32_t field_D7C_;  // 0xD7C
  uint32_t field_D80_;  // 0xD80
  uint32_t field_D84_;  // 0xD84
  uint32_t field_D88_;  // 0xD88
  uint32_t field_D8C_;  // 0xD8C
  uint32_t field_D90_;  // 0xD90
  FlatButton flatButton1_;  // 0xD94
  FlatButton flatButton2_;  // 0x18E0
  FlatButton flatButton3_;  // 0x242C
  FlatButton flatButton4_;  // 0x2F78
  FlatButton flatButton5_;  // 0x3AC4
  FlatButton flatButton6_;  // 0x4610, the IDB's last member; ends 0x515C
};

void __fastcall counc_win_on_group_clicked_redirect(CouncWin *self, void *, int a1, int a2);
void __fastcall counc_win_on_button_passover_redirect(CouncWin *self, void *, int a1, int a2);
void __fastcall counc_win_on_iface_left_click_redirect(CouncWin *self, void *, int a1, int a2);
void __fastcall counc_win_on_iface_right_click_redirect(CouncWin *self, void *, int a1, int a2);
void __fastcall counc_win_on_iface_left_down_redirect(CouncWin *self, void *, int a1, int a2);
void __fastcall counc_win_on_iface_right_down_redirect(CouncWin *self, void *, int a1, int a2);
void __fastcall counc_win_on_iface_mouse_move_redirect(CouncWin *self, void *, int a1, int a2);
void __fastcall counc_win_on_iface_mouse_leave_redirect(CouncWin *self, void *, int a1, int a2);
void __fastcall counc_win_on_iface_selected_redirect(CouncWin *self, void *, int a1, int a2);
void __fastcall counc_win_on_iface_left_double_click_redirect(CouncWin *self, void *, int a1, int a2);
void __fastcall counc_win_on_iface_right_double_click_redirect(CouncWin *self, void *, int a1, int a2);
void __fastcall counc_win_on_iface_button_toggled_redirect(CouncWin *self, void *, int a1, int a2);
void __fastcall counc_win_on_iface_group_clicked_redirect(CouncWin *self, void *, int a1, int a2, int a3);
int __fastcall counc_win_on_button_text_draw_redirect(
    CouncWin *self, void *, int a1, int a2, int a3);

// ?draw_leader@CouncWin@@QAEXH@Z at 0x00425DB0 - public, __thiscall,
// void(int) - is 1705 bytes and unrecovered. draw_all_leaders and
// on_mouse_leave both reach it with a direct `call rel32`, so it is declared
// on the class and defined at the end of councwin.cpp as a forwarder.
typedef void (OriginalObject::*func_counc_win_draw_leader)(int factionID);
extern func_counc_win_draw_leader CouncWinOriginalDrawLeader;  // 0x00425DB0
