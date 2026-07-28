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
  * NetWin class
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
class DLLEXPORT NetWin : GraphicWin {
 public:
  NetWin() { ; }
  ~NetWin() { ; }
  void UNK5();
  void UNK1();
  void UNK2();
  void alloc_slots();
  int on_key_click(int a1, int a2);
  void on_mouse_move(int a1, int a2);
  void on_mouse_leave(int a1, int a2);
};

void __fastcall net_win_unk5_redirect(NetWin *self, void *);
void __fastcall net_win_unk1_redirect(NetWin *self, void *);
void __fastcall net_win_unk2_redirect(NetWin *self, void *);
void __fastcall net_win_alloc_slots_redirect(NetWin *self, void *);
int __fastcall net_win_on_key_click_redirect(NetWin *self, void *, int a1, int a2);
void __fastcall net_win_on_mouse_move_redirect(NetWin *self, void *, int a1, int a2);
void __fastcall net_win_on_mouse_leave_redirect(NetWin *self, void *, int a1, int a2);
