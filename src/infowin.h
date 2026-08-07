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
  * InfoWin class
  *
  * The original does not derive this from Time: the constructor builds
  * one at +0x30 on an offset `this`, making it a member. That distinction
  * was missed when this header was first written, because the check looked
  * only at which constructor ran first and not at whether `this` had been
  * adjusted before it.
  *
  * The layout is not established and nothing pins its sizeof.
*/
class DLLEXPORT InfoWin {
 public:
  void on_right_click(int a1, int a2);
  // 0x004589C0  ?right_menu@InfoWin@@QAEXHH@Z - public, __thiscall,
  // void(int, int). Still unrecovered, so this is a declaration without a
  // definition and the DLL will not link until it is recovered. Declared
  // rather than routed through an original_seam.h seam ON PURPOSE:
  // on_right_click's whole 20-byte body is `push a2; push a1; call rel32`
  // (byte-match.csv:3554 BYTE_EXACT 20/20), and a seam turns that direct
  // call into `call dword ptr [global]`, which loses the match and takes the
  // body out of the census - every one of the 753 seam-using bodies scores
  // NO_COMPILE there because the scaffolding cannot see the seam global.
  void right_menu(int a1, int a2);
  InfoWin() { ; }
  ~InfoWin() { ; }
  void UNK1();
  void on_redraw();
  void reset();
  void change(int a1);
  void UNK3(int a1, int a2);

 private:
  // Not a base class: the constructor builds a Time at +0x30 on an offset
  // `this`. The constructor reaches 0x9D0, so the object is at least that
  // large; nothing else about it is established.
  uint8_t unmapped_0_[0xA8];
  int32_t field_A8_;
  uint8_t unmapped_AC_[0x9D4 - 0xAC];
};

void __fastcall info_win_unk1_redirect(InfoWin *self, void *);
void __fastcall info_win_on_redraw_redirect(InfoWin *self, void *);
void __fastcall info_win_change_redirect(InfoWin *self, void *, int a1);
void __fastcall info_win_unk3_redirect(InfoWin *self, void *, int a1, int a2);

// InfoWin::timer_proc is not recovered yet.
typedef void (OriginalObject::*func_info_win_timer_proc)(int);
extern func_info_win_timer_proc InfoWinOriginalTimerProc;

void __fastcall info_win_reset_redirect(InfoWin *self, void *);
