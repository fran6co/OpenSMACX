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
#include "listbox.h"
#include "flatbutton.h"
#include "graphicwin.h"
#include "stringbox.h"
#include "editbox.h"
#include "subinterface.h"

// Shared palette cell published by load_interface and consumed by both
// diplomacy rendering and popup menu defaults; owned in diplowin.cpp.
extern int DiploKnownTechColor;

 /*
  * DiploWin class
  *
  * The native constructor at 0x444FC0 constructs GraphicWin at offset zero
  * and SubInterface at +0xA14. Its nineteen-slot secondary table is 0x66B5E0.
  * C++ owns the adjusted interface dispatch and both base lifetimes. Known
  * controls retain their observed offsets; unnamed scalar fields are provisional.
  */
class DiploWin : public GraphicWin, public SubInterface {
 public:
  // homed from 00442510.cpp
  void on_left_click(int a1, int a2) override;
  int on_key_click(int repeat, int key);
  int on_key_down(int key);
  void done_deal();
  void check_done();
  void do_it();
  void draw_who();
  void draw_whom();
  void draw_offers();
  void UNK1();
  void new_chat(char *text, int faction_id);
  static void __cdecl string_callback(int faction_id, int unused);
  void exec(int faction_id, int faction_id_with);
  StringBox &shared_string_box() { return stringBox_; }
  EditBox &shared_edit_box() { return editBox_; }

 public:
  // homed from 00441490.cpp
  void add_offer(int a1, int a2, int a3);

 public:
  // homed from 004413f0.cpp
  void kill_offer(int a1, int a2);

 public:
  void hide();
  void show(int visible);
  DiploWin();
  void on_redraw() override;
  void on_button_clicked(int button_id) override;
  void on_status(int mode) override;
  void on_iface_scrolled(int, int) override;
  void on_iface_scrolling(int, int) override;
  int on_iface_button_pressed(int) override;
  void on_iface_group_clicked(int, int, int) override;
  int on_iface_dialog_back_draw(GraphicWin *) override;
  int on_iface_dialog_item_back_draw(GraphicWin *, int, int, RECT *) override;
  ~DiploWin();
  void UNK3(int a1);
  void UNK5();
  void UNK2();
  void UNK4(int a1);
  void on_iface_left_click(int a1, int a2);
  void on_iface_left_up(int a1, int a2);
  void on_iface_right_click(int a1, int a2);
  void on_iface_left_down(int a1, int a2);
  void on_iface_right_down(int a1, int a2);
  void on_iface_mouse_move(int a1, int a2);
  void on_iface_mouse_leave(int a1, int a2);
  void on_iface_selected(int a1, int a2);
  void on_iface_left_double_click(int a1, int a2);
  void on_iface_right_double_click(int a1, int a2);
  void on_iface_button_clicked(int a1);
  void on_iface_button_toggled(int a1, int a2);
  int on_iface_dialog_back_draw(int a1);

 private:
  // UNK2 zeroes the two dwords at 0xA24 and 0xA28, just past the
  // GraphicWin base; everything between is unmapped.
  // SubInterface occupies 0xA14..0xA1B; these are not two scalar members.
  int offer_count_[2];  // 0xA1C: one count for each party
  int32_t field_A24_;
  int32_t field_A28_;

  // Storage the image proves is here: its own methods reach 0xDA80.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  // 418 member(s) from the IDA database, 5 named; it starts a member at 0xA2C, which is where src/ ends.

  int offer_type_[2][8];  // 0xA2C
  int offer_value_[2][8];  // 0xA6C
  uint32_t field_AAC_;  // 0xAAC
  uint32_t field_AB0_;  // 0xAB0
  int faction_id_;  // 0xAB4, the local party to the exchange
  int32_t faction_id_with_;  // 0xAB8, the diplomacy counterpart used by offer/chat messages
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
  uint32_t field_B2C_;  // 0xB2C
  uint32_t field_B30_;  // 0xB30
  uint32_t field_B34_;  // 0xB34
  uint32_t field_B38_;  // 0xB38
  uint32_t field_B3C_;  // 0xB3C
  uint32_t field_B40_;  // 0xB40
  uint32_t field_B44_;  // 0xB44
  uint32_t field_B48_;  // 0xB48
  uint32_t field_B4C_;  // 0xB4C
  uint32_t field_B50_;  // 0xB50
  uint32_t field_B54_;  // 0xB54
  uint32_t field_B58_;  // 0xB58
  uint32_t field_B5C_;  // 0xB5C
  uint32_t field_B60_;  // 0xB60
  uint32_t field_B64_;  // 0xB64
  uint32_t field_B68_;  // 0xB68
  uint32_t field_B6C_;  // 0xB6C
  uint32_t field_B70_;  // 0xB70
  uint32_t field_B74_;  // 0xB74
  uint32_t field_B78_;  // 0xB78
  uint32_t field_B7C_;  // 0xB7C
  uint32_t field_B80_;  // 0xB80
  uint32_t field_B84_;  // 0xB84
  uint32_t field_B88_;  // 0xB88
  uint32_t field_B8C_;  // 0xB8C
  uint32_t field_B90_;  // 0xB90
  uint32_t field_B94_;  // 0xB94
  uint32_t field_B98_;  // 0xB98
  uint32_t field_B9C_;  // 0xB9C
  uint32_t field_BA0_;  // 0xBA0
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
  uint32_t field_D94_;  // 0xD94
  uint32_t field_D98_;  // 0xD98
  uint32_t field_D9C_;  // 0xD9C
  uint32_t field_DA0_;  // 0xDA0
  uint32_t field_DA4_;  // 0xDA4
  uint32_t field_DA8_;  // 0xDA8
  uint32_t field_DAC_;  // 0xDAC
  uint32_t field_DB0_;  // 0xDB0
  uint32_t field_DB4_;  // 0xDB4
  uint32_t field_DB8_;  // 0xDB8
  uint32_t field_DBC_;  // 0xDBC
  uint32_t field_DC0_;  // 0xDC0
  uint32_t field_DC4_;  // 0xDC4
  uint32_t field_DC8_;  // 0xDC8
  uint32_t field_DCC_;  // 0xDCC
  uint32_t field_DD0_;  // 0xDD0
  uint32_t field_DD4_;  // 0xDD4
  uint32_t field_DD8_;  // 0xDD8
  uint32_t field_DDC_;  // 0xDDC
  uint32_t field_DE0_;  // 0xDE0
  uint32_t field_DE4_;  // 0xDE4
  uint32_t field_DE8_;  // 0xDE8
  uint32_t field_DEC_;  // 0xDEC
  uint32_t field_DF0_;  // 0xDF0
  uint32_t field_DF4_;  // 0xDF4
  uint32_t field_DF8_;  // 0xDF8
  uint32_t field_DFC_;  // 0xDFC
  uint32_t field_E00_;  // 0xE00
  uint32_t field_E04_;  // 0xE04
  uint32_t field_E08_;  // 0xE08
  uint32_t field_E0C_;  // 0xE0C
  uint32_t field_E10_;  // 0xE10
  uint32_t field_E14_;  // 0xE14
  uint32_t field_E18_;  // 0xE18
  uint32_t field_E1C_;  // 0xE1C
  uint32_t field_E20_;  // 0xE20
  uint32_t field_E24_;  // 0xE24
  uint32_t field_E28_;  // 0xE28
  uint32_t field_E2C_;  // 0xE2C
  uint32_t field_E30_;  // 0xE30
  uint32_t field_E34_;  // 0xE34
  uint32_t field_E38_;  // 0xE38
  uint32_t field_E3C_;  // 0xE3C
  uint32_t field_E40_;  // 0xE40
  uint32_t field_E44_;  // 0xE44
  uint32_t field_E48_;  // 0xE48
  uint32_t field_E4C_;  // 0xE4C
  uint32_t field_E50_;  // 0xE50
  uint32_t field_E54_;  // 0xE54
  uint32_t field_E58_;  // 0xE58
  uint32_t field_E5C_;  // 0xE5C
  uint32_t field_E60_;  // 0xE60
  uint32_t field_E64_;  // 0xE64
  uint32_t field_E68_;  // 0xE68
  uint32_t field_E6C_;  // 0xE6C
  uint32_t field_E70_;  // 0xE70
  uint32_t field_E74_;  // 0xE74
  uint32_t field_E78_;  // 0xE78
  uint32_t field_E7C_;  // 0xE7C
  uint32_t field_E80_;  // 0xE80
  uint32_t field_E84_;  // 0xE84
  uint32_t field_E88_;  // 0xE88
  uint32_t field_E8C_;  // 0xE8C
  uint32_t field_E90_;  // 0xE90
  uint32_t field_E94_;  // 0xE94
  uint32_t field_E98_;  // 0xE98
  uint32_t field_E9C_;  // 0xE9C
  uint32_t field_EA0_;  // 0xEA0
  uint32_t field_EA4_;  // 0xEA4
  uint32_t field_EA8_;  // 0xEA8
  uint32_t field_EAC_;  // 0xEAC
  uint32_t field_EB0_;  // 0xEB0
  uint32_t field_EB4_;  // 0xEB4
  uint32_t field_EB8_;  // 0xEB8
  uint32_t field_EBC_;  // 0xEBC
  uint32_t field_EC0_;  // 0xEC0
  uint32_t field_EC4_;  // 0xEC4
  uint32_t field_EC8_;  // 0xEC8
  uint32_t field_ECC_;  // 0xECC
  Spot spot_;  // 0xED0
  RECT rect_EDC_;  // 0xEDC
  RECT rect_EEC_;  // 0xEEC
  RECT rect_EFC_;  // 0xEFC
  RECT rect_F0C_;  // 0xF0C
  RECT rect_F1C_;  // 0xF1C
  RECT rect_F2C_;  // 0xF2C
  RECT rect_F3C_;  // 0xF3C
  RECT rect_F4C_;  // 0xF4C
  RECT portrait_rect_[2];  // 0xF5C, 0xF6C
  RECT leader_rect_[2];  // 0xF7C, 0xF8C
  RECT rect_F9C_;  // 0xF9C
  RECT rect_FAC_;  // 0xFAC
  RECT rect_FBC_;  // 0xFBC
  RECT rect_FCC_;  // 0xFCC
  RECT button_rects_[13];  // 0xFDC..0x10AB

  FlatButton flatButtons_[13];  // 0x10AC
  StringBox stringBox_;  // 0xA388, size == sizeof(StringBox)
  ListBox listBox_;  // 0xCF28
  // The IDB's last member: the class ends where editBox_ ends, 0xE5F0.
  EditBox editBox_;  // 0xDA7C, IDB `editBox`, size == sizeof(EditBox) == 0xB74
};

void __cdecl clear_hotseat_chat();
extern "C" void __cdecl sub_441040(int a1, int a2);
extern "C" void __cdecl sub_441100(int faction_id, int faction_id_with);

extern DiploWin DiploWindow;
extern StringList DiplomacyTranscriptLists[8][8];
