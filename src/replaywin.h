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
  * ReplayWin class
  *
  * No constructor survives in the catalog, so the base cannot be checked and
  * the layout is not established; its methods reach as far as 0x252EC, so the
  * object is at least that large. The stubs recovered here are constant
  * returns that touch no field, which is why they can be replaced without that
  * layout - the opaque storage below is only large enough to give the canary
  * an object to seed.
  */
class DLLEXPORT ReplayWin {
 public:
  // 0x005AD9E0  ?timer_callback@ReplayWin@@QAEXXZ - public, __thiscall,
  // void(void), unrecovered. Declared so timer_callback_daemon, the free
  // forwarder in replaywin.cpp that reaches it with a direct `call rel32`,
  // can name it.
  void timer_callback();
  ReplayWin() { ; }
  ~ReplayWin() { ; }
  void on_left_double_click(int, int);
  void on_right_double_click(int, int);
  void on_mouse_move(int, int);
  void on_right_down(int, int);
  void on_left_down(int, int);
  void on_left_up(int, int);

 private:
  uint32_t field_0_;  // 0x0
  uint8_t field_4_[0x470];  // 0x4
  uint32_t field_474_;  // 0x474
  uint32_t field_478_;  // 0x478
  uint32_t field_47C_;  // 0x47C
  uint32_t field_480_;  // 0x480
  uint8_t field_484_[0x590];  // 0x484
  uint32_t field_A14_;  // 0xA14
  uint32_t field_A18_;  // 0xA18
  uint32_t field_A1C_;  // 0xA1C
  uint32_t field_A20_;  // 0xA20
  uint32_t field_A24_;  // 0xA24
  uint32_t field_A28_;  // 0xA28
  uint8_t field_A2C_[0x17F4];  // 0xA2C
  uint8_t field_2220_;  // 0x2220
  uint8_t field_2221_[0xFF];  // 0x2221
  uint32_t field_2320_;  // 0x2320
  uint32_t field_2324_;  // 0x2324
  uint8_t field_2328_[0x1DD68];  // 0x2328
  uint32_t field_20090_;  // 0x20090
  uint8_t field_20094_[0x24];  // 0x20094
  uint32_t field_200B8_;  // 0x200B8
  uint32_t field_200BC_;  // 0x200BC
  uint32_t field_200C0_;  // 0x200C0
  uint8_t field_200C4_[0x46DC];  // 0x200C4
  uint32_t field_247A0_;  // 0x247A0
  uint8_t field_247A4_[0x440];  // 0x247A4
  uint32_t field_24BE4_;  // 0x24BE4
  uint8_t field_24BE8_[0x680];  // 0x24BE8
  uint32_t field_25268_;  // 0x25268
  uint32_t field_2526C_;  // 0x2526C
  uint32_t field_25270_;  // 0x25270
  uint8_t field_25274_[0x7C];  // 0x25274
};

void __fastcall replay_win_on_left_double_click_redirect(ReplayWin *self, void *, int a1, int a2);
void __fastcall replay_win_on_right_double_click_redirect(ReplayWin *self, void *, int a1, int a2);
void __fastcall replay_win_on_mouse_move_redirect(ReplayWin *self, void *, int a1, int a2);
void __fastcall replay_win_on_right_down_redirect(ReplayWin *self, void *, int a1, int a2);
void __fastcall replay_win_on_left_down_redirect(ReplayWin *self, void *, int a1, int a2);
void __fastcall replay_win_on_left_up_redirect(ReplayWin *self, void *, int a1, int a2);

// timer_callback is declared on the class because fn_005adbd0, the recovered
// free forwarder in replaywin.cpp, reaches it with a direct `call rel32`, but
// its 492-byte body is not recovered, so the DLL has nothing to link that
// call against. It stands in with a forward to the original image until the
// body lands.
//   ?timer_callback@ReplayWin@@QAEXXZ  0x005AD9E0
//     public, __thiscall, void(void)
typedef void (OriginalObject::*func_replay_win_timer_callback)();
extern func_replay_win_timer_callback ReplayWinTimerCallback;
