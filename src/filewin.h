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
#include "flatbutton.h"
#include "editbox.h"
#include "listbox.h"

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
class FileWin {
 public:
  // `static`: the image's name ends in `QAA` - a public member declared
  // __cdecl, taking no receiver - and jackal_init_real calls it with no
  // ecx set up, so `Class::method()` is the only legal spelling.
  static int init_class();   // 00614D90
  void on_double_clicked(int a1);
  FileWin();
  ~FileWin();
  void UNK6();
  void UNK1();
  void UNK2(int a1);
  // 0x006146A0 ?UNK4@FileWin@@QAEXXZ - public, __thiscall, void(void). Still
  // unrecovered; declared because the recovered on_double_clicked calls it
  // with a direct `call rel32`, which is what makes that body byte-exact.
  void UNK4();
  // 0x00613900, a pending_bodies forwarder. ~FileWin calls it first.
  void close();

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
  // Four member subobjects, evidenced by the constructor's placement-new
  // sites and the destructor's teardown sequence (0x00611940-family:
  // FlatButton@0x660/0x11AC, EditBox@0x1CF8, ListBox@0x286C - each span
  // exactly matches that member type's own established sizeof, which is
  // what pins these offsets rather than the IDB alone).
  FlatButton flat_button2_;  // 0x660, sizeof 0xB4C, ends 0x11AC
  FlatButton flat_button1_;  // 0x11AC, sizeof 0xB4C, ends 0x1CF8
  EditBox edit_box_;  // 0x1CF8, sizeof 0xB74, ends 0x286C
  ListBox list_box_;  // 0x286C, sizeof 0xB54, ends 0x33C0
  uint32_t field_33C0_;  // 0x33C0
};

static_assert(sizeof(FileWin) == 0x33C4,
              "FileWin layout must match the original executable");

void __fastcall file_win_unk6_redirect(FileWin *self, void *);
void __fastcall file_win_unk2_redirect(FileWin *self, void *, int a1);
void __fastcall file_win_unk1_redirect(FileWin *self, void *);

// ?UNK4@FileWin@@QAEXXZ (0x006146A0). Unrecovered; on_double_clicked calls it,
// so its definition is a seam into the original image.
