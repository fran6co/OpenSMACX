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
#include "original_seam.h"
#include "graphicwin.h"

 /*
  * SetupWin class
  *
  * Derives from GraphicWin by ordinary single inheritance - its constructor calls
  * GraphicWin's on an unadjusted `this` - so it is written as the original wrote
  * it and needs no ABI workaround. GraphicWin is pinned here, so the base region
  * is exact; SetupWin's own extent is not established and nothing pins its
  * sizeof.
  *
  * The methods below ignore their arguments and return a constant, which is
  * why they can be replaced ahead of that mapping.
  */
class DLLEXPORT SetupWin : GraphicWin {
 public:
  int do_menu(char* a1, int a2, int a3);
  // 0x004ADB70  ?do_menu_rightside@SetupWin@@QAEHPADH@Z - public, __thiscall,
  // int(char*, int), unrecovered. Declared so do_menu, which reaches it with a
  // direct `call rel32`, can name it.
  int do_menu_rightside(char* a1, int a2);
  SetupWin() { ; }
  ~SetupWin() { ; }
  int UNK1(int a1);
  int UNK2(int a1);
  int UNK3(int a1);
  int UNK4(int a1);

  // Storage the image proves is here: its own methods reach 0xA1C.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  // 2 member(s) from the IDA database, 0 named; it starts a member at 0xA14, which is where src/ ends.

  uint32_t field_A14_;  // 0xA14
  uint32_t field_A18_;  // 0xA18

  // Storage the image proves is here: its own methods reach 0x11A0.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  // 93 member(s) from the IDA database, 3 named; it starts a member at 0xA1C, which is where src/ ends.

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
  uint8_t buffer_[0x588];  // 0xA44
  uint32_t field_FCC_;  // 0xFCC
  uint32_t field_FD0_;  // 0xFD0
  uint8_t spot_[0xC];  // 0xFD4
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
  uint32_t field_10AC_;  // 0x10AC
  uint32_t field_10B0_;  // 0x10B0
  uint32_t field_10B4_;  // 0x10B4
  uint32_t field_10B8_;  // 0x10B8
  uint32_t field_10BC_;  // 0x10BC
  uint32_t field_10C0_;  // 0x10C0
  uint32_t field_10C4_;  // 0x10C4
  uint32_t field_10C8_;  // 0x10C8
  uint32_t field_10CC_;  // 0x10CC
  uint32_t field_10D0_;  // 0x10D0
  uint32_t field_10D4_;  // 0x10D4
  uint32_t field_10D8_;  // 0x10D8
  uint32_t field_10DC_;  // 0x10DC
  uint32_t field_10E0_;  // 0x10E0
  uint32_t field_10E4_;  // 0x10E4
  uint32_t field_10E8_;  // 0x10E8
  uint32_t field_10EC_;  // 0x10EC
  uint32_t field_10F0_;  // 0x10F0
  uint32_t field_10F4_;  // 0x10F4
  uint32_t field_10F8_;  // 0x10F8
  uint32_t field_10FC_;  // 0x10FC
  uint32_t field_1100_;  // 0x1100
  uint32_t field_1104_;  // 0x1104
  uint32_t field_1108_;  // 0x1108
  uint32_t field_110C_;  // 0x110C
  uint32_t field_1110_;  // 0x1110
  uint32_t field_1114_;  // 0x1114
  uint8_t field_1118_[0x88];  // 0x1118
};

int __fastcall setup_win_unk1_redirect(SetupWin *self, void *, int a1);
int __fastcall setup_win_unk2_redirect(SetupWin *self, void *, int a1);
int __fastcall setup_win_unk3_redirect(SetupWin *self, void *, int a1);
int __fastcall setup_win_unk4_redirect(SetupWin *self, void *, int a1);

// do_menu_rightside is declared on the class because the recovered do_menu
// reaches it with a direct `call rel32`, but its 2129-byte body is not
// recovered, so the DLL has nothing to link that call against. It stands in
// with a forward to the original image until the body lands.
//   ?do_menu_rightside@SetupWin@@QAEHPADH@Z  0x004ADB70
//     public, __thiscall, int(char *, int)
typedef int (OriginalObject::*func_setup_win_do_menu_rightside)(char *a1, int a2);
extern func_setup_win_do_menu_rightside SetupWinDoMenuRightside;
