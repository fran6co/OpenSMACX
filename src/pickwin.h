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
#include "sprite.h"
#include "listbox.h"
#include "flatbutton.h"
#include "graphicwin.h"
#include "buttongroup.h"
#include "popup.h"

 /*
  * PickWin class
  *
  * Derives from GraphicWin by ordinary single inheritance - its constructor calls
  * GraphicWin's on an unadjusted `this` - so it is written as the original wrote
  * it and needs no ABI workaround. GraphicWin is pinned here, so the base region
  * is exact; PickWin's own extent is not established and nothing pins its
  * sizeof.
  *
  * The methods below ignore their arguments and return a constant, which is
  * why they can be replaced ahead of that mapping.
  */
class PickWin : public GraphicWin {
 public:
  PickWin() { ; }
  // 0x0048A8E0 is not recovered: a
  // pending_bodies forwarder, because an empty inline stub emits
  // nothing and the deleting destructor needs a `call rel32`.
  ~PickWin();
  void on_button_passover(int a1, int a2);

  // Storage the image proves is here: its own methods reach 0x12644.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  // 393 member(s) from the IDA database, 18 named; it starts a member at 0xA14, which is where src/ ends.

  uint32_t field_A14_;  // 0xA14
  uint32_t field_A18_;  // 0xA18
  uint32_t field_A1C_;  // 0xA1C
  uint32_t field_A20_;  // 0xA20
  Popup popup_;  // 0xA24, declared Popup extent == sizeof(BasePop) + sizeof(Scroll)
  uint32_t field_5DA0_;  // 0x5DA0
  uint32_t field_5DA4_;  // 0x5DA4
  uint32_t field_5DA8_;  // 0x5DA8
  uint32_t field_5DAC_;  // 0x5DAC
  uint32_t field_5DB0_;  // 0x5DB0
  uint32_t field_5DB4_;  // 0x5DB4
  uint32_t field_5DB8_;  // 0x5DB8
  uint32_t field_5DBC_;  // 0x5DBC
  uint32_t field_5DC0_;  // 0x5DC0
  uint32_t field_5DC4_;  // 0x5DC4
  uint32_t field_5DC8_;  // 0x5DC8
  uint32_t field_5DCC_;  // 0x5DCC
  uint32_t field_5DD0_;  // 0x5DD0
  uint32_t field_5DD4_;  // 0x5DD4
  uint32_t field_5DD8_;  // 0x5DD8
  uint32_t field_5DDC_;  // 0x5DDC
  uint32_t field_5DE0_;  // 0x5DE0
  uint32_t field_5DE4_;  // 0x5DE4
  uint32_t field_5DE8_;  // 0x5DE8
  uint32_t field_5DEC_;  // 0x5DEC
  uint32_t field_5DF0_;  // 0x5DF0
  uint32_t field_5DF4_;  // 0x5DF4
  uint32_t field_5DF8_;  // 0x5DF8
  uint32_t field_5DFC_;  // 0x5DFC
  uint32_t field_5E00_;  // 0x5E00
  uint32_t field_5E04_;  // 0x5E04
  uint32_t field_5E08_;  // 0x5E08
  uint32_t field_5E0C_;  // 0x5E0C
  uint32_t field_5E10_;  // 0x5E10
  uint32_t field_5E14_;  // 0x5E14
  uint32_t field_5E18_;  // 0x5E18
  uint32_t field_5E1C_;  // 0x5E1C
  uint32_t field_5E20_;  // 0x5E20
  uint32_t field_5E24_;  // 0x5E24
  uint32_t field_5E28_;  // 0x5E28
  uint32_t field_5E2C_;  // 0x5E2C
  uint32_t field_5E30_;  // 0x5E30
  uint32_t field_5E34_;  // 0x5E34
  uint32_t field_5E38_;  // 0x5E38
  uint32_t field_5E3C_;  // 0x5E3C
  uint32_t field_5E40_;  // 0x5E40
  uint32_t field_5E44_;  // 0x5E44
  uint32_t field_5E48_;  // 0x5E48
  uint32_t field_5E4C_;  // 0x5E4C
  uint32_t field_5E50_;  // 0x5E50
  uint32_t field_5E54_;  // 0x5E54
  uint32_t field_5E58_;  // 0x5E58
  uint32_t field_5E5C_;  // 0x5E5C
  uint32_t field_5E60_;  // 0x5E60
  uint32_t field_5E64_;  // 0x5E64
  uint32_t field_5E68_;  // 0x5E68
  uint32_t field_5E6C_;  // 0x5E6C
  uint32_t field_5E70_;  // 0x5E70
  uint32_t field_5E74_;  // 0x5E74
  uint32_t field_5E78_;  // 0x5E78
  uint32_t field_5E7C_;  // 0x5E7C
  uint32_t field_5E80_;  // 0x5E80
  uint32_t field_5E84_;  // 0x5E84
  uint32_t field_5E88_;  // 0x5E88
  uint32_t field_5E8C_;  // 0x5E8C
  uint32_t field_5E90_;  // 0x5E90
  uint32_t field_5E94_;  // 0x5E94
  uint32_t field_5E98_;  // 0x5E98
  uint32_t field_5E9C_;  // 0x5E9C
  uint32_t field_5EA0_;  // 0x5EA0
  uint32_t field_5EA4_;  // 0x5EA4
  uint32_t field_5EA8_;  // 0x5EA8
  uint32_t field_5EAC_;  // 0x5EAC
  uint32_t field_5EB0_;  // 0x5EB0
  uint32_t field_5EB4_;  // 0x5EB4
  uint32_t field_5EB8_;  // 0x5EB8
  uint32_t field_5EBC_;  // 0x5EBC
  uint32_t field_5EC0_;  // 0x5EC0
  uint32_t field_5EC4_;  // 0x5EC4
  uint32_t field_5EC8_;  // 0x5EC8
  uint32_t field_5ECC_;  // 0x5ECC
  uint32_t field_5ED0_;  // 0x5ED0
  uint32_t field_5ED4_;  // 0x5ED4
  uint32_t field_5ED8_;  // 0x5ED8
  uint32_t field_5EDC_;  // 0x5EDC
  uint32_t field_5EE0_;  // 0x5EE0
  uint32_t field_5EE4_;  // 0x5EE4
  uint32_t field_5EE8_;  // 0x5EE8
  uint32_t field_5EEC_;  // 0x5EEC
  uint32_t field_5EF0_;  // 0x5EF0
  uint32_t field_5EF4_;  // 0x5EF4
  uint32_t field_5EF8_;  // 0x5EF8
  uint32_t field_5EFC_;  // 0x5EFC
  uint32_t field_5F00_;  // 0x5F00
  uint32_t field_5F04_;  // 0x5F04
  uint32_t field_5F08_;  // 0x5F08
  uint32_t field_5F0C_;  // 0x5F0C
  uint32_t field_5F10_;  // 0x5F10
  uint32_t field_5F14_;  // 0x5F14
  uint32_t field_5F18_;  // 0x5F18
  uint32_t field_5F1C_;  // 0x5F1C
  uint32_t field_5F20_;  // 0x5F20
  uint32_t field_5F24_;  // 0x5F24
  uint32_t field_5F28_;  // 0x5F28
  uint32_t field_5F2C_;  // 0x5F2C
  uint32_t field_5F30_;  // 0x5F30
  uint32_t field_5F34_;  // 0x5F34
  uint32_t field_5F38_;  // 0x5F38
  uint32_t field_5F3C_;  // 0x5F3C
  uint32_t field_5F40_;  // 0x5F40
  uint32_t field_5F44_;  // 0x5F44
  uint32_t field_5F48_;  // 0x5F48
  uint32_t field_5F4C_;  // 0x5F4C
  uint32_t field_5F50_;  // 0x5F50
  uint32_t field_5F54_;  // 0x5F54
  uint32_t field_5F58_;  // 0x5F58
  uint32_t field_5F5C_;  // 0x5F5C
  uint32_t field_5F60_;  // 0x5F60
  uint32_t field_5F64_;  // 0x5F64
  uint32_t field_5F68_;  // 0x5F68
  uint32_t field_5F6C_;  // 0x5F6C
  uint32_t field_5F70_;  // 0x5F70
  uint32_t field_5F74_;  // 0x5F74
  uint32_t field_5F78_;  // 0x5F78
  uint32_t field_5F7C_;  // 0x5F7C
  uint32_t field_5F80_;  // 0x5F80
  uint32_t field_5F84_;  // 0x5F84
  uint32_t field_5F88_;  // 0x5F88
  uint32_t field_5F8C_;  // 0x5F8C
  uint32_t field_5F90_;  // 0x5F90
  uint32_t field_5F94_;  // 0x5F94
  uint32_t field_5F98_;  // 0x5F98
  uint32_t field_5F9C_;  // 0x5F9C
  uint32_t field_5FA0_;  // 0x5FA0
  uint32_t field_5FA4_;  // 0x5FA4
  uint32_t field_5FA8_;  // 0x5FA8
  uint32_t field_5FAC_;  // 0x5FAC
  uint32_t field_5FB0_;  // 0x5FB0
  uint32_t field_5FB4_;  // 0x5FB4
  uint32_t field_5FB8_;  // 0x5FB8
  uint32_t field_5FBC_;  // 0x5FBC
  uint32_t field_5FC0_;  // 0x5FC0
  uint32_t field_5FC4_;  // 0x5FC4
  uint32_t field_5FC8_;  // 0x5FC8
  uint32_t field_5FCC_;  // 0x5FCC
  uint32_t field_5FD0_;  // 0x5FD0
  uint32_t field_5FD4_;  // 0x5FD4
  uint32_t field_5FD8_;  // 0x5FD8
  uint32_t field_5FDC_;  // 0x5FDC
  uint32_t field_5FE0_;  // 0x5FE0
  uint32_t field_5FE4_;  // 0x5FE4
  uint32_t field_5FE8_;  // 0x5FE8
  uint32_t field_5FEC_;  // 0x5FEC
  uint32_t field_5FF0_;  // 0x5FF0
  uint32_t field_5FF4_;  // 0x5FF4
  uint32_t field_5FF8_;  // 0x5FF8
  uint32_t field_5FFC_;  // 0x5FFC
  uint32_t field_6000_;  // 0x6000
  uint32_t field_6004_;  // 0x6004
  uint32_t field_6008_;  // 0x6008
  uint32_t field_600C_;  // 0x600C
  uint32_t field_6010_;  // 0x6010
  uint32_t field_6014_;  // 0x6014
  uint32_t field_6018_;  // 0x6018
  uint32_t field_601C_;  // 0x601C
  uint32_t field_6020_;  // 0x6020
  uint32_t field_6024_;  // 0x6024
  uint32_t field_6028_;  // 0x6028
  uint32_t field_602C_;  // 0x602C
  uint32_t field_6030_;  // 0x6030
  uint32_t field_6034_;  // 0x6034
  uint32_t field_6038_;  // 0x6038
  uint32_t field_603C_;  // 0x603C
  uint32_t field_6040_;  // 0x6040
  uint32_t field_6044_;  // 0x6044
  uint32_t field_6048_;  // 0x6048
  uint32_t field_604C_;  // 0x604C
  uint32_t field_6050_;  // 0x6050
  uint32_t field_6054_;  // 0x6054
  uint32_t field_6058_;  // 0x6058
  uint32_t field_605C_;  // 0x605C
  uint32_t field_6060_;  // 0x6060
  uint32_t field_6064_;  // 0x6064
  uint32_t field_6068_;  // 0x6068
  uint32_t field_606C_;  // 0x606C
  uint32_t field_6070_;  // 0x6070
  uint32_t field_6074_;  // 0x6074
  uint32_t field_6078_;  // 0x6078
  uint32_t field_607C_;  // 0x607C
  uint32_t field_6080_;  // 0x6080
  uint32_t field_6084_;  // 0x6084
  uint32_t field_6088_;  // 0x6088
  uint32_t field_608C_;  // 0x608C
  uint32_t field_6090_;  // 0x6090
  uint32_t field_6094_;  // 0x6094
  uint32_t field_6098_;  // 0x6098
  uint32_t field_609C_;  // 0x609C
  uint32_t field_60A0_;  // 0x60A0
  uint32_t field_60A4_;  // 0x60A4
  uint32_t field_60A8_;  // 0x60A8
  uint32_t field_60AC_;  // 0x60AC
  uint32_t field_60B0_;  // 0x60B0
  uint32_t field_60B4_;  // 0x60B4
  uint32_t field_60B8_;  // 0x60B8
  uint32_t field_60BC_;  // 0x60BC
  uint32_t field_60C0_;  // 0x60C0
  uint32_t field_60C4_;  // 0x60C4
  uint32_t field_60C8_;  // 0x60C8
  uint32_t field_60CC_;  // 0x60CC
  uint32_t field_60D0_;  // 0x60D0
  uint32_t field_60D4_;  // 0x60D4
  uint32_t field_60D8_;  // 0x60D8
  uint32_t field_60DC_;  // 0x60DC
  uint32_t field_60E0_;  // 0x60E0
  uint32_t field_60E4_;  // 0x60E4
  uint32_t field_60E8_;  // 0x60E8
  uint32_t field_60EC_;  // 0x60EC
  uint32_t field_60F0_;  // 0x60F0
  uint32_t field_60F4_;  // 0x60F4
  uint32_t field_60F8_;  // 0x60F8
  uint32_t field_60FC_;  // 0x60FC
  uint32_t field_6100_;  // 0x6100
  uint32_t field_6104_;  // 0x6104
  uint32_t field_6108_;  // 0x6108
  uint32_t field_610C_;  // 0x610C
  uint32_t field_6110_;  // 0x6110
  uint32_t field_6114_;  // 0x6114
  uint32_t field_6118_;  // 0x6118
  uint32_t field_611C_;  // 0x611C
  uint32_t field_6120_;  // 0x6120
  uint32_t field_6124_;  // 0x6124
  uint32_t field_6128_;  // 0x6128
  uint32_t field_612C_;  // 0x612C
  uint32_t field_6130_;  // 0x6130
  uint32_t field_6134_;  // 0x6134
  uint32_t field_6138_;  // 0x6138
  uint32_t field_613C_;  // 0x613C
  uint32_t field_6140_;  // 0x6140
  uint32_t field_6144_;  // 0x6144
  uint32_t field_6148_;  // 0x6148
  uint32_t field_614C_;  // 0x614C
  uint32_t field_6150_;  // 0x6150
  uint32_t field_6154_;  // 0x6154
  uint32_t field_6158_;  // 0x6158
  uint32_t field_615C_;  // 0x615C
  uint32_t field_6160_;  // 0x6160
  uint32_t field_6164_;  // 0x6164
  uint32_t field_6168_;  // 0x6168
  uint32_t field_616C_;  // 0x616C
  uint32_t field_6170_;  // 0x6170
  uint32_t field_6174_;  // 0x6174
  uint32_t field_6178_;  // 0x6178
  uint32_t field_617C_;  // 0x617C
  uint32_t field_6180_;  // 0x6180
  uint32_t field_6184_;  // 0x6184
  uint32_t field_6188_;  // 0x6188
  uint32_t field_618C_;  // 0x618C
  uint32_t field_6190_;  // 0x6190
  uint32_t field_6194_;  // 0x6194
  uint32_t field_6198_;  // 0x6198
  uint32_t field_619C_;  // 0x619C
  uint32_t field_61A0_;  // 0x61A0
  uint32_t field_61A4_;  // 0x61A4
  uint32_t field_61A8_;  // 0x61A8
  uint32_t field_61AC_;  // 0x61AC
  uint32_t field_61B0_;  // 0x61B0
  uint32_t field_61B4_;  // 0x61B4
  uint32_t field_61B8_;  // 0x61B8
  uint32_t field_61BC_;  // 0x61BC
  uint32_t field_61C0_;  // 0x61C0
  uint32_t field_61C4_;  // 0x61C4
  uint32_t field_61C8_;  // 0x61C8
  uint32_t field_61CC_;  // 0x61CC
  uint32_t field_61D0_;  // 0x61D0
  uint32_t field_61D4_;  // 0x61D4
  uint32_t field_61D8_;  // 0x61D8
  uint32_t field_61DC_;  // 0x61DC
  uint32_t field_61E0_;  // 0x61E0
  uint32_t field_61E4_;  // 0x61E4
  uint32_t field_61E8_;  // 0x61E8
  uint32_t field_61EC_;  // 0x61EC
  uint32_t field_61F0_;  // 0x61F0
  uint32_t field_61F4_;  // 0x61F4
  uint32_t field_61F8_;  // 0x61F8
  uint32_t field_61FC_;  // 0x61FC
  uint32_t field_6200_;  // 0x6200
  uint32_t field_6204_;  // 0x6204
  uint32_t field_6208_;  // 0x6208
  uint32_t field_620C_;  // 0x620C
  uint32_t field_6210_;  // 0x6210
  uint32_t field_6214_;  // 0x6214
  uint32_t field_6218_;  // 0x6218
  uint32_t field_621C_;  // 0x621C
  uint32_t field_6220_;  // 0x6220
  uint32_t field_6224_;  // 0x6224
  uint32_t field_6228_;  // 0x6228
  uint32_t field_622C_;  // 0x622C
  uint32_t field_6230_;  // 0x6230
  uint32_t field_6234_;  // 0x6234
  uint32_t field_6238_;  // 0x6238
  uint32_t field_623C_;  // 0x623C
  uint32_t field_6240_;  // 0x6240
  uint32_t field_6244_;  // 0x6244
  uint32_t field_6248_;  // 0x6248
  uint32_t field_624C_;  // 0x624C
  uint32_t field_6250_;  // 0x6250
  uint32_t field_6254_;  // 0x6254
  uint32_t field_6258_;  // 0x6258
  uint32_t field_625C_;  // 0x625C
  uint32_t field_6260_;  // 0x6260
  uint32_t field_6264_;  // 0x6264
  uint32_t field_6268_;  // 0x6268
  uint32_t field_626C_;  // 0x626C
  uint32_t field_6270_;  // 0x6270
  uint32_t field_6274_;  // 0x6274
  uint32_t field_6278_;  // 0x6278
  uint32_t field_627C_;  // 0x627C
  uint32_t field_6280_;  // 0x6280
  uint32_t field_6284_;  // 0x6284
  uint32_t field_6288_;  // 0x6288
  FlatButton flatButton1_;  // 0x628C
  FlatButton flatButton2_;  // 0x6DD8
  FlatButton flatButton3_;  // 0x7924
  Sprite sprites1_[3];  // 0x8470
  Sprite sprites2_[3];  // 0x84F4
  Sprite sprites3_[3];  // 0x8578
  Sprite sprites4_[3];  // 0x85FC
  Sprite sprite5_;  // 0x8680
  uint32_t field_86AC_;  // 0x86AC
  uint32_t field_86B0_;  // 0x86B0
  uint32_t field_86B4_;  // 0x86B4
  uint32_t field_86B8_;  // 0x86B8
  ListBox listBox_;  // 0x86BC
  uint32_t field_9210_;  // 0x9210
  uint32_t field_9214_;  // 0x9214
  uint32_t field_9218_;  // 0x9218
  uint32_t field_921C_;  // 0x921C
  uint32_t field_9220_;  // 0x9220
  uint32_t field_9224_;  // 0x9224
  uint32_t field_9228_;  // 0x9228
  uint32_t field_922C_;  // 0x922C
  uint32_t field_9230_;  // 0x9230
  uint32_t field_9234_;  // 0x9234
  uint32_t field_9238_;  // 0x9238
  uint32_t field_923C_;  // 0x923C
  uint32_t field_9240_;  // 0x9240
  uint32_t field_9244_;  // 0x9244
  uint32_t field_9248_;  // 0x9248
  uint32_t field_924C_;  // 0x924C
  uint32_t field_9250_;  // 0x9250
  uint32_t field_9254_;  // 0x9254
  uint32_t field_9258_;  // 0x9258
  uint32_t field_925C_;  // 0x925C
  uint32_t field_9260_;  // 0x9260
  uint32_t field_9264_;  // 0x9264
  uint32_t field_9268_;  // 0x9268
  uint32_t field_926C_;  // 0x926C
  FlatButton flatButton4_;  // 0x9270
  FlatButton flatButton5_;  // 0x9DBC
  FlatButton flatButton6_;  // 0xA908
  FlatButton flatButton7_;  // 0xB454
  FlatButton flatButton8_;  // 0xBFA0
  FlatButton flatButton9_;  // 0xCAEC
  uint32_t field_D638_;  // 0xD638
  uint32_t field_D63C_;  // 0xD63C
  uint32_t field_D640_;  // 0xD640
  uint32_t field_D644_;  // 0xD644
  uint32_t field_D648_;  // 0xD648
  uint32_t field_D64C_;  // 0xD64C
  uint32_t field_D650_;  // 0xD650
  uint32_t field_D654_;  // 0xD654
  uint32_t field_D658_;  // 0xD658
  uint32_t field_D65C_;  // 0xD65C
  uint32_t field_D660_;  // 0xD660
  uint32_t field_D664_;  // 0xD664
  uint32_t field_D668_;  // 0xD668
  uint32_t field_D66C_;  // 0xD66C
  uint32_t field_D670_;  // 0xD670
  uint32_t field_D674_;  // 0xD674
  uint32_t field_D678_;  // 0xD678
  uint32_t field_D67C_;  // 0xD67C
  uint32_t field_D680_;  // 0xD680
  uint32_t field_D684_;  // 0xD684
  uint32_t field_D688_;  // 0xD688
  uint32_t field_D68C_;  // 0xD68C
  uint32_t field_D690_;  // 0xD690
  uint32_t field_D694_;  // 0xD694
  uint32_t field_D698_;  // 0xD698
  uint32_t field_D69C_;  // 0xD69C
  uint32_t field_D6A0_;  // 0xD6A0
  uint32_t field_D6A4_;  // 0xD6A4
  FlatButton flatButtons10_[7];  // 0xD6A8
  ButtonGroup buttonGroup_;  // 0x125BC, IDB `buttonGroup`, size == sizeof(ButtonGroup)
};

void __fastcall pick_win_on_button_passover_redirect(PickWin *self, void *, int a1, int a2);
