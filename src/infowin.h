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
#include "time.h"

 /*
  * InfoWin class
  *
  * Derives from Time by ordinary single inheritance. Time is pinned at 0x28,
  * so the base region is exact; InfoWin's own extent is not established.
  *
  * The methods recovered here are bare returns or a bare constant, touching
  * no field, which is why they can be replaced ahead of that mapping.
  */
class DLLEXPORT InfoWin : Time {
 public:
  InfoWin() { ; }
  ~InfoWin() { ; }
  void UNK1();
  void on_redraw();
  void change(int a1);
  void UNK3(int a1, int a2);
};

void __fastcall info_win_unk1_redirect(InfoWin *self, void *);
void __fastcall info_win_on_redraw_redirect(InfoWin *self, void *);
void __fastcall info_win_change_redirect(InfoWin *self, void *, int a1);
void __fastcall info_win_unk3_redirect(InfoWin *self, void *, int a1, int a2);
