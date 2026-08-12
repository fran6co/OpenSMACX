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

 /*
  * FileWin class
  *
  * The original does not derive this from FlatButton: the constructor builds
  * one at +0x660 on an offset `this`, making it a member. That distinction
  * was missed when this header was first written, because the check looked
  * only at which constructor ran first and not at whether `this` had been
  * adjusted before it.
  *
  * The layout is not established and nothing pins its sizeof.
*/
class DLLEXPORT FileWin {
 public:
  void on_double_clicked(int a1);
  FileWin() { ; }
  ~FileWin() { ; }
  void UNK6();
  void UNK1();
  void UNK2(int a1);
  // 0x006146A0 ?UNK4@FileWin@@QAEXXZ - public, __thiscall, void(void). Still
  // unrecovered; declared because the recovered on_double_clicked calls it
  // with a direct `call rel32`, which is what makes that body byte-exact.
  void UNK4();

 private:
  // Not a base class: the constructor builds a FlatButton at +0x660 on an
  // offset `this`. The constructor reaches 0x33C0, so the object is at least
  // that large; nothing else about it is established.
  // The FlatButton member sits at 0x660, so these five all precede it and
  // belong to FileWin itself. The spans hold their offsets in place; the test
  // reads each back where the original writes it.
  uint8_t unmapped_0_[0x208];
  uint8_t field_208_;
  uint8_t unmapped_209_[0x30C - 0x209];
  uint8_t field_30C_;
  uint8_t unmapped_30D_[0x410 - 0x30D];
  uint8_t field_410_;
  uint8_t unmapped_411_[0x514 - 0x411];
  int32_t field_514_;
  uint8_t unmapped_518_[0x531 - 0x518];
  uint8_t field_531_;
  uint8_t field_532_[0x106];  // 0x532
  uint32_t field_638_;  // 0x638
  uint32_t field_63C_;  // 0x63C
  uint32_t field_640_;  // 0x640
  uint32_t field_644_;  // 0x644
  uint32_t field_648_;  // 0x648
  uint32_t field_64C_;  // 0x64C
  uint32_t field_650_;  // 0x650
  uint32_t field_654_;  // 0x654
  uint32_t field_658_;  // 0x658
  uint32_t field_65C_;  // 0x65C
  uint32_t field_660_;  // 0x660
  uint8_t field_664_[0xB48];  // 0x664
  uint32_t field_11AC_;  // 0x11AC
  uint8_t field_11B0_[0x440];  // 0x11B0
  uint32_t field_15F0_;  // 0x15F0
  uint8_t field_15F4_[0x704];  // 0x15F4
  uint32_t field_1CF8_;  // 0x1CF8
  uint8_t field_1CFC_[0x440];  // 0x1CFC
  uint32_t field_213C_;  // 0x213C
  uint8_t field_2140_[0x6CC];  // 0x2140
  uint32_t field_280C_;  // 0x280C
  uint8_t field_2810_[0x28];  // 0x2810
  uint32_t field_2838_;  // 0x2838
  uint32_t field_283C_;  // 0x283C
  uint8_t field_2840_[0x2C];  // 0x2840
  uint32_t field_286C_;  // 0x286C
  uint8_t field_2870_[0x4];  // 0x2870
  uint32_t field_2874_;  // 0x2874
  uint8_t field_2878_[0x24];  // 0x2878
  uint32_t field_289C_;  // 0x289C
  uint8_t field_28A0_[0xB20];  // 0x28A0
  uint32_t field_33C0_;  // 0x33C0
};

static_assert(sizeof(FileWin) == 0x33C4,
              "FileWin layout must match the original executable");

void __fastcall file_win_unk6_redirect(FileWin *self, void *);
void __fastcall file_win_unk2_redirect(FileWin *self, void *, int a1);
void __fastcall file_win_unk1_redirect(FileWin *self, void *);

// ?UNK4@FileWin@@QAEXXZ (0x006146A0). Unrecovered; on_double_clicked calls it,
// so its definition is a seam into the original image.
typedef void (OriginalObject::*func_file_win_unk4)();
extern func_file_win_unk4 FileWinUNK4;
