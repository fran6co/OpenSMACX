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
  FileWin() { ; }
  ~FileWin() { ; }
  void UNK6();
  void UNK2(int a1);

 private:
  // Not a base class: the constructor builds a FlatButton at +0x660 on an
  // offset `this`. The constructor reaches 0x33C0, so the object is at least
  // that large; nothing else about it is established.
  uint8_t unmapped_0_[0x33C4];
};

void __fastcall file_win_unk6_redirect(FileWin *self, void *);
void __fastcall file_win_unk2_redirect(FileWin *self, void *, int a1);
