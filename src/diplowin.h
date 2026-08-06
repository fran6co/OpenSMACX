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
  * DiploWin class
  *
  * Derives from GraphicWin in the original by ordinary single inheritance -
  * its constructor calls GraphicWin's on an unadjusted `this` - so unlike the
  * virtually-derived windows this one is written as the original wrote it and
  * needs no ABI workaround.
  *
  * The layout is not established. The constructor's own fields begin at 0xA14,
  * immediately after the base, and where the object ends is unknown, so
  * nothing pins its sizeof and the fields between are not modelled. The
  * methods recovered here are bare returns that touch no field, which is why
  * they can be replaced ahead of that mapping.
  */
class DLLEXPORT DiploWin : GraphicWin {
 public:
  DiploWin() { ; }
  ~DiploWin() { ; }
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
  uint8_t unmapped_A14_[0xA24 - 0xA14];
  int32_t field_A24_;
  int32_t field_A28_;

  // Storage the image proves is here: its own methods reach 0xDA80.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  // 418 member(s) from the IDA database, 5 named; it starts a member at 0xA2C, which is where src/ ends.

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
  uint8_t spot_[0xC];  // 0xED0
  uint32_t field_EDC_;  // 0xEDC
  uint32_t field_EE0_;  // 0xEE0
  uint32_t field_EE4_;  // 0xEE4
  uint32_t field_EE8_;  // 0xEE8
  uint32_t field_EEC_;  // 0xEEC
  uint32_t field_EF0_;  // 0xEF0
  uint32_t field_EF4_;  // 0xEF4
  uint32_t field_EF8_;  // 0xEF8
  uint32_t field_EFC_;  // 0xEFC
  uint32_t field_F00_;  // 0xF00
  uint32_t field_F04_;  // 0xF04
  uint32_t field_F08_;  // 0xF08
  uint32_t field_F0C_;  // 0xF0C
  uint32_t field_F10_;  // 0xF10
  uint32_t field_F14_;  // 0xF14
  uint32_t field_F18_;  // 0xF18
  uint32_t field_F1C_;  // 0xF1C
  uint32_t field_F20_;  // 0xF20
  uint32_t field_F24_;  // 0xF24
  uint32_t field_F28_;  // 0xF28
  uint32_t field_F2C_;  // 0xF2C
  uint32_t field_F30_;  // 0xF30
  uint32_t field_F34_;  // 0xF34
  uint32_t field_F38_;  // 0xF38
  uint32_t field_F3C_;  // 0xF3C
  uint32_t field_F40_;  // 0xF40
  uint32_t field_F44_;  // 0xF44
  uint32_t field_F48_;  // 0xF48
  uint32_t field_F4C_;  // 0xF4C
  uint32_t field_F50_;  // 0xF50
  uint32_t field_F54_;  // 0xF54
  uint32_t field_F58_;  // 0xF58
  uint32_t field_F5C_;  // 0xF5C
  uint32_t field_F60_;  // 0xF60
  uint32_t field_F64_;  // 0xF64
  uint32_t field_F68_;  // 0xF68
  uint32_t field_F6C_;  // 0xF6C
  uint32_t field_F70_;  // 0xF70
  uint32_t field_F74_;  // 0xF74
  uint32_t field_F78_;  // 0xF78
  uint32_t field_F7C_;  // 0xF7C
  uint32_t field_F80_;  // 0xF80
  uint32_t field_F84_;  // 0xF84
  uint32_t field_F88_;  // 0xF88
  uint32_t field_F8C_;  // 0xF8C
  uint32_t field_F90_;  // 0xF90
  uint32_t field_F94_;  // 0xF94
  uint32_t field_F98_;  // 0xF98
  uint32_t field_F9C_;  // 0xF9C
  uint32_t field_FA0_;  // 0xFA0
  uint32_t field_FA4_;  // 0xFA4
  uint32_t field_FA8_;  // 0xFA8
  uint32_t field_FAC_;  // 0xFAC
  uint32_t field_FB0_;  // 0xFB0
  uint32_t field_FB4_;  // 0xFB4
  uint32_t field_FB8_;  // 0xFB8
  uint32_t field_FBC_;  // 0xFBC
  uint32_t field_FC0_;  // 0xFC0
  uint32_t field_FC4_;  // 0xFC4
  uint32_t field_FC8_;  // 0xFC8
  uint32_t field_FCC_;  // 0xFCC
  uint32_t field_FD0_;  // 0xFD0
  uint32_t field_FD4_;  // 0xFD4
  uint32_t field_FD8_;  // 0xFD8
  uint32_t field_FDC_;  // 0xFDC
  uint32_t field_FE0_;  // 0xFE0
  uint32_t field_FE4_;  // 0xFE4
  uint32_t field_FE8_;  // 0xFE8
  uint32_t field_FEC_;  // 0xFEC
  uint32_t field_FF0_;  // 0xFF0
  uint32_t field_FF4_;  // 0xFF4
  uint32_t field_FF8_;  // 0xFF8
  uint32_t field_FFC_;  // 0xFFC
  uint32_t field_1000_;  // 0x1000
  uint32_t field_1004_;  // 0x1004
  uint32_t field_1008_;  // 0x1008
  uint32_t field_100C_;  // 0x100C
  uint32_t field_1010_;  // 0x1010
  uint32_t field_1014_;  // 0x1014
  uint32_t field_1018_;  // 0x1018
  uint32_t field_101C_;  // 0x101C
  uint32_t field_1020_;  // 0x1020
  uint32_t field_1024_;  // 0x1024
  uint32_t field_1028_;  // 0x1028
  uint32_t field_102C_;  // 0x102C
  uint32_t field_1030_;  // 0x1030
  uint32_t field_1034_;  // 0x1034
  uint32_t field_1038_;  // 0x1038
  uint32_t field_103C_;  // 0x103C
  uint32_t field_1040_;  // 0x1040
  uint32_t field_1044_;  // 0x1044
  uint32_t field_1048_;  // 0x1048
  uint32_t field_104C_;  // 0x104C
  uint32_t field_1050_;  // 0x1050
  uint32_t field_1054_;  // 0x1054
  uint32_t field_1058_;  // 0x1058
  uint32_t field_105C_;  // 0x105C
  uint32_t field_1060_;  // 0x1060
  uint32_t field_1064_;  // 0x1064
  uint32_t field_1068_;  // 0x1068
  uint32_t field_106C_;  // 0x106C
  uint32_t field_1070_;  // 0x1070
  uint32_t field_1074_;  // 0x1074
  uint32_t field_1078_;  // 0x1078
  uint32_t field_107C_;  // 0x107C
  uint32_t field_1080_;  // 0x1080
  uint32_t field_1084_;  // 0x1084
  uint32_t field_1088_;  // 0x1088
  uint32_t field_108C_;  // 0x108C
  uint32_t field_1090_;  // 0x1090
  uint32_t field_1094_;  // 0x1094
  uint32_t field_1098_;  // 0x1098
  uint32_t field_109C_;  // 0x109C
  uint32_t field_10A0_;  // 0x10A0
  uint32_t field_10A4_;  // 0x10A4
  uint32_t field_10A8_;  // 0x10A8
  uint8_t flatButtons_[0x92DC];  // 0x10AC
  uint8_t stringBox_[0x2BA0];  // 0xA388
  uint8_t listBox_[0xB54];  // 0xCF28
  uint8_t field_DA7C_[0x4];  // 0xDA7C

  // Storage the image proves is here: its own methods reach 0xE5B0.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  uint8_t field_DA80_[0xB30];
};

void __fastcall diplo_win_unk3_redirect(DiploWin *self, void *, int a1);
void __fastcall diplo_win_unk5_redirect(DiploWin *self, void *);
void __fastcall diplo_win_unk4_redirect(DiploWin *self, void *, int a1);
void __fastcall diplo_win_on_iface_left_click_redirect(DiploWin *self, void *, int a1, int a2);
void __fastcall diplo_win_on_iface_left_up_redirect(DiploWin *self, void *, int a1, int a2);
void __fastcall diplo_win_on_iface_right_click_redirect(DiploWin *self, void *, int a1, int a2);
void __fastcall diplo_win_on_iface_left_down_redirect(DiploWin *self, void *, int a1, int a2);
void __fastcall diplo_win_on_iface_right_down_redirect(DiploWin *self, void *, int a1, int a2);
void __fastcall diplo_win_on_iface_mouse_move_redirect(DiploWin *self, void *, int a1, int a2);
void __fastcall diplo_win_on_iface_mouse_leave_redirect(DiploWin *self, void *, int a1, int a2);
void __fastcall diplo_win_on_iface_selected_redirect(DiploWin *self, void *, int a1, int a2);
void __fastcall diplo_win_on_iface_left_double_click_redirect(DiploWin *self, void *, int a1, int a2);
void __fastcall diplo_win_on_iface_right_double_click_redirect(DiploWin *self, void *, int a1, int a2);
void __fastcall diplo_win_on_iface_button_clicked_redirect(DiploWin *self, void *, int a1);
void __fastcall diplo_win_on_iface_button_toggled_redirect(DiploWin *self, void *, int a1, int a2);
int __fastcall diplo_win_on_iface_dialog_back_draw_redirect(DiploWin *self, void *, int a1);
void __fastcall diplo_win_unk2_redirect(DiploWin *self, void *);
