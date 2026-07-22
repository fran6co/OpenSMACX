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
#include "flatbutton.h"

 /*
  * FileWin class
  *
  * Derives from FlatButton by ordinary single inheritance. FlatButton is
  * pinned at 0xB4C, so the base region is exact; FileWin's own extent is not
  * established.
  *
  * The methods recovered here are bare returns or a bare constant, touching
  * no field, which is why they can be replaced ahead of that mapping.
  */
class DLLEXPORT FileWin : FlatButton {
 public:
  FileWin() { ; }
  ~FileWin() { ; }
  void UNK6();
  void UNK2(int a1);
};

void __fastcall file_win_unk6_redirect(FileWin *self, void *);
void __fastcall file_win_unk2_redirect(FileWin *self, void *, int a1);
