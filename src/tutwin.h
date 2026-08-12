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
#include "win.h"

 /*
  * TutWin class
  *
  * The original derives this from Popup on an unadjusted `this`, so Popup is
  * a genuine base - but Popup's size is not established, so the base cannot
  * be modelled and everything ahead of these fields is held as opaque storage
  * instead. That is enough to place them: the offsets below are absolute from
  * the object's start either way.
  *
  * Nothing pins this class's sizeof.
  */
class DLLEXPORT TutWin {
 public:
  void on_move(int a1, int a2);
  TutWin() { ; }
  ~TutWin() { ; }
  void UNK1();
  void UNK3(int a1);
  void do_base(RECT *rect, const char *text, int flag);
  void do_iface(RECT *rect, const char *text, int flag);
  void do_soc(RECT *rect, const char *text, int flag);
  void do_des(RECT *rect, const char *text, int flag);
  void iface_rect(RECT *rect, int *x, int *y);
  void base_rect(RECT *rect, int *x, int *y);
  void soc_rect(RECT *rect, int *x, int *y);
  void des_rect(RECT *rect, int *x, int *y);

 private:
  uint32_t field_0_;  // 0x0
  uint8_t field_4_[0x440];  // 0x4
  uint32_t field_444_;  // 0x444
  uint8_t field_448_[0x614];  // 0x448
  uint32_t field_A5C_;  // 0xA5C
  uint8_t field_A60_[0x440];  // 0xA60
  uint32_t field_EA0_;  // 0xEA0
  uint8_t field_EA4_[0x704];  // 0xEA4
  uint32_t field_15A8_;  // 0x15A8
  uint8_t field_15AC_[0x440];  // 0x15AC
  uint32_t field_19EC_;  // 0x19EC
  uint8_t field_19F0_[0x754];  // 0x19F0
  uint32_t field_2144_;  // 0x2144
  uint8_t field_2148_[0x30];  // 0x2148
  uint32_t field_2178_;  // 0x2178
  uint32_t field_217C_;  // 0x217C
  uint32_t field_2180_;  // 0x2180
  uint32_t field_2184_;  // 0x2184
  uint8_t field_2188_[0xC];  // 0x2188
  uint32_t field_2194_;  // 0x2194
  uint8_t field_2198_[0x10];  // 0x2198
  uint32_t field_21A8_;  // 0x21A8
  uint32_t field_21AC_;  // 0x21AC
  uint8_t field_21B0_[0xC4];  // 0x21B0
  uint32_t field_2274_;  // 0x2274
  uint8_t field_2278_[0xE44];  // 0x2278
  uint32_t field_30BC_;  // 0x30BC
  uint32_t field_30C0_;  // 0x30C0
  uint8_t field_30C4_[0xA4];  // 0x30C4
  uint32_t field_3168_;  // 0x3168
  uint8_t field_316C_[0xC4];  // 0x316C
  uint32_t field_3230_;  // 0x3230
  uint8_t field_3234_[0x440];  // 0x3234
  uint32_t field_3674_;  // 0x3674
  uint8_t field_3678_[0x664];  // 0x3678
  uint32_t field_3CDC_;  // 0x3CDC
  uint8_t field_3CE0_[0x440];  // 0x3CE0
  uint32_t field_4120_;  // 0x4120
  uint8_t field_4124_[0x704];  // 0x4124
  uint32_t field_4828_;  // 0x4828
  uint8_t field_482C_[0x440];  // 0x482C
  uint32_t field_4C6C_;  // 0x4C6C
  uint8_t field_4C70_[0x70C];  // 0x4C70
  int32_t field_537C_;
  int32_t field_5380_;
  uint32_t field_5384_;  // 0x5384
  uint32_t field_5388_;  // 0x5388
  uint32_t field_538C_;  // 0x538C
  uint32_t field_5390_;  // 0x5390
  uint32_t field_5394_;  // 0x5394
  uint32_t field_5398_;  // 0x5398
  int32_t field_539C_;
  uint8_t unmapped_53A0_[0x53A4 - 0x53A0];
  int32_t field_53A4_;
  int32_t field_53A8_;
  int32_t field_53AC_;
  uint8_t field_53B0_[0x4];  // 0x53B0
  uint32_t field_53B4_;  // 0x53B4
  int32_t field_53B8_;
  uint32_t field_53BC_;  // 0x53BC
  uint32_t field_53C0_;  // 0x53C0
  int32_t field_53C4_;
  uint32_t field_53C8_;  // 0x53C8
  uint32_t field_53CC_;  // 0x53CC
  uint32_t field_53D0_;  // 0x53D0
  int32_t field_53D4_;
};

static_assert(sizeof(TutWin) == 0x53D8,
              "TutWin layout must match the original executable");

extern uint32_t *TutWinShownFlag;

void __fastcall tut_win_unk1_redirect(TutWin *self, void *);
void __fastcall tut_win_unk3_redirect(TutWin *self, void *, int a1);

// The fixed window iface_rect centres onto, at 0x007AE820.
extern Win *TutWinIfaceWindow;
// The fixed window base_rect centres onto, at 0x006A7628.
extern Win *TutWinBaseWindow;
// The fixed window soc_rect centres onto, at 0x008A6270.
extern Win *TutWinSocWindow;
// The fixed window des_rect centres onto, at 0x0071F2B0.
extern Win *TutWinDesWindow;

void __fastcall tut_win_iface_rect_redirect(
    TutWin *self, void *, RECT *rect, int *x, int *y);
void __fastcall tut_win_base_rect_redirect(
    TutWin *self, void *, RECT *rect, int *x, int *y);
void __fastcall tut_win_soc_rect_redirect(
    TutWin *self, void *, RECT *rect, int *x, int *y);
void __fastcall tut_win_des_rect_redirect(
    TutWin *self, void *, RECT *rect, int *x, int *y);

// The shared tail of the four do_* helpers: 0x004BDFE0, still original.
typedef int (OriginalObject::*func_tut_win_show)(void *window, const char *text, int x, int y, void *sprite, int flag, int a7, int a8);
extern func_tut_win_show TutWinOriginalShow;

void __fastcall tut_win_do_base_redirect(
    TutWin *self, void *, RECT *rect, const char *text, int flag);
void __fastcall tut_win_do_iface_redirect(
    TutWin *self, void *, RECT *rect, const char *text, int flag);
void __fastcall tut_win_do_soc_redirect(
    TutWin *self, void *, RECT *rect, const char *text, int flag);
void __fastcall tut_win_do_des_redirect(
    TutWin *self, void *, RECT *rect, const char *text, int flag);
