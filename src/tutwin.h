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
  uint8_t unmapped_0_[0x537C];
  int32_t field_537C_;
  int32_t field_5380_;
  uint8_t unmapped_5384_[0x539C - 0x5384];
  int32_t field_539C_;
  uint8_t unmapped_53A0_[0x53A4 - 0x53A0];
  int32_t field_53A4_;
  int32_t field_53A8_;
  int32_t field_53AC_;
  uint8_t unmapped_53B0_[0x53B8 - 0x53B0];
  int32_t field_53B8_;
  uint8_t unmapped_53BC_[0x53C4 - 0x53BC];
  int32_t field_53C4_;
  uint8_t unmapped_53C8_[0x53D4 - 0x53C8];
  int32_t field_53D4_;
};

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

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
#endif
// The shared tail of the four do_* helpers: 0x004BDFE0, still original.
typedef int(__thiscall func_tut_win_show)(void *self, void *window,
                                          const char *text, int x, int y,
                                          void *sprite, int flag, int a7,
                                          int a8);
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
extern func_tut_win_show *TutWinOriginalShow;

void __fastcall tut_win_do_base_redirect(
    TutWin *self, void *, RECT *rect, const char *text, int flag);
void __fastcall tut_win_do_iface_redirect(
    TutWin *self, void *, RECT *rect, const char *text, int flag);
void __fastcall tut_win_do_soc_redirect(
    TutWin *self, void *, RECT *rect, const char *text, int flag);
void __fastcall tut_win_do_des_redirect(
    TutWin *self, void *, RECT *rect, const char *text, int flag);
