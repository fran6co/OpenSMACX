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
class InfoWin {
 public:
  // 0x00459150, a pending_bodies forwarder.
  void timer_proc(int a1);

 public:
  void on_right_click(int a1, int a2);
  // 0x004589C0  ?right_menu@InfoWin@@QAEXHH@Z - public, __thiscall,
  // void(int, int). Still an original body, so the definition at the END of
  // infowin.cpp forwards to it. on_right_click is NOT routed through that seam
  // itself, ON PURPOSE: its whole 20-byte body is `push a2; push a1; call
  // rel32` (byte-match.csv:3554 BYTE_EXACT 20/20), and calling the seam from
  // inside it turns that direct call into `call dword ptr [global]`, which
  // loses the match and takes the body out of the census - every one of the
  // 753 seam-using bodies scores NO_COMPILE there because the scaffolding
  // cannot see the seam global. Giving right_menu its own out-of-line
  // forwarder keeps on_right_click's `call rel32` intact; the forwarder is
  // placed after on_right_click so /Ob2 cannot inline it back in.
  void right_menu(int a1, int a2);
  InfoWin();
  ~InfoWin();
  void UNK1();
  void on_redraw();
  void reset();
  void change(int a1);
  void UNK3(int a1, int a2);

 private:
  // Not a base class: the constructor builds a Time at +0x30 on an offset
  // `this`. The constructor reaches 0x9D0, so the object is at least that
  // large; nothing else about it is established.
  uint8_t field_0_[0x10];  // 0x0
  uint32_t field_10_;  // 0x10
  uint32_t field_14_;  // 0x14
  uint32_t field_18_;  // 0x18
  uint32_t field_1C_;  // 0x1C
  uint32_t field_20_;  // 0x20
  uint32_t field_24_;  // 0x24
  uint32_t field_28_;  // 0x28
  uint32_t field_2C_;  // 0x2C
  uint8_t field_30_[0x28];  // 0x30
  uint32_t field_58_;  // 0x58
  uint8_t field_5C_[0x8];  // 0x5C
  uint32_t field_64_;  // 0x64
  uint32_t field_68_;  // 0x68
  uint8_t field_6C_[0x14];  // 0x6C
  uint32_t field_80_;  // 0x80
  uint32_t field_84_;  // 0x84
  uint32_t field_88_;  // 0x88
  uint32_t field_8C_;  // 0x8C
  uint32_t field_90_;  // 0x90
  uint32_t field_94_;  // 0x94
  uint8_t field_98_[0x10];  // 0x98
  int32_t field_A8_;
  uint32_t field_AC_;  // 0xAC
  uint32_t field_B0_;  // 0xB0
  uint32_t field_B4_;  // 0xB4
  uint8_t field_B8_;  // 0xB8
  uint8_t field_B9_[0x77F];  // 0xB9
  uint32_t field_838_;  // 0x838
  uint8_t field_83C_[0x5C];  // 0x83C
  uint32_t field_898_;  // 0x898
  uint8_t field_89C_[0x5C];  // 0x89C
  uint32_t field_8F8_;  // 0x8F8
  uint8_t field_8FC_[0x5C];  // 0x8FC
  uint32_t field_958_;  // 0x958
  uint8_t field_95C_[0x64];  // 0x95C
  uint32_t field_9C0_;  // 0x9C0
  uint32_t field_9C4_;  // 0x9C4
  uint32_t field_9C8_;  // 0x9C8
  uint32_t field_9CC_;  // 0x9CC
  uint8_t field_9D0_[0x4];  // 0x9D0
};

void __fastcall info_win_unk1_redirect(InfoWin *self, void *);
void __fastcall info_win_on_redraw_redirect(InfoWin *self, void *);
void __fastcall info_win_change_redirect(InfoWin *self, void *, int a1);
void __fastcall info_win_unk3_redirect(InfoWin *self, void *, int a1, int a2);

// InfoWin::timer_proc is not recovered yet.

// InfoWin::right_menu is not recovered yet - 911 bytes that build and run the
// tile context menu. Rebindable so tests can substitute a probe and so the
// seam can later point at a recovered body.
typedef void (OriginalObject::*func_info_win_right_menu)(int, int);
extern func_info_win_right_menu InfoWinOriginalRightMenu;  // 0x004589C0

void __fastcall info_win_reset_redirect(InfoWin *self, void *);
