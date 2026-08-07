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
  uint8_t unmapped_532_[0x33C4 - 0x532];
};

static_assert(sizeof(FileWin) == 0x33C4,
              "FileWin layout must match the original executable");

void __fastcall file_win_unk6_redirect(FileWin *self, void *);
void __fastcall file_win_unk2_redirect(FileWin *self, void *, int a1);
void __fastcall file_win_unk1_redirect(FileWin *self, void *);
