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
  * DiploWin class
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
class DLLEXPORT DiploWin : GraphicWin {
 public:
  DiploWin() { ; }
  ~DiploWin() { ; }
  void UNK3(int a1);
  void UNK5();
  void UNK2();
  void UNK4(int a1);
  void on_iface_left_click(int a1, int a2);
  void on_iface_left_up(int a1, int a2);
  void on_iface_right_click(int a1, int a2);
  void on_iface_left_down(int a1, int a2);
  void on_iface_right_down(int a1, int a2);
  void on_iface_mouse_move(int a1, int a2);
  void on_iface_mouse_leave(int a1, int a2);
  void on_iface_selected(int a1, int a2);
  void on_iface_left_double_click(int a1, int a2);
  void on_iface_right_double_click(int a1, int a2);
  void on_iface_button_clicked(int a1);
  void on_iface_button_toggled(int a1, int a2);
  int on_iface_dialog_back_draw(int a1);

 private:
  // UNK2 zeroes the two dwords at 0xA24 and 0xA28, just past the
  // GraphicWin base; everything between is unmapped.
  uint8_t unmapped_A14_[0xA24 - 0xA14];
  int32_t field_A24_;
  int32_t field_A28_;

  // Storage the image proves is here: its own methods reach 0xDA80.
  // Extent only - nothing here is named, and this class carries no size assertion.
  uint8_t field_A2C_[0xD054];
};

void __fastcall diplo_win_unk3_redirect(DiploWin *self, void *, int a1);
void __fastcall diplo_win_unk5_redirect(DiploWin *self, void *);
void __fastcall diplo_win_unk4_redirect(DiploWin *self, void *, int a1);
void __fastcall diplo_win_on_iface_left_click_redirect(DiploWin *self, void *, int a1, int a2);
void __fastcall diplo_win_on_iface_left_up_redirect(DiploWin *self, void *, int a1, int a2);
void __fastcall diplo_win_on_iface_right_click_redirect(DiploWin *self, void *, int a1, int a2);
void __fastcall diplo_win_on_iface_left_down_redirect(DiploWin *self, void *, int a1, int a2);
void __fastcall diplo_win_on_iface_right_down_redirect(DiploWin *self, void *, int a1, int a2);
void __fastcall diplo_win_on_iface_mouse_move_redirect(DiploWin *self, void *, int a1, int a2);
void __fastcall diplo_win_on_iface_mouse_leave_redirect(DiploWin *self, void *, int a1, int a2);
void __fastcall diplo_win_on_iface_selected_redirect(DiploWin *self, void *, int a1, int a2);
void __fastcall diplo_win_on_iface_left_double_click_redirect(DiploWin *self, void *, int a1, int a2);
void __fastcall diplo_win_on_iface_right_double_click_redirect(DiploWin *self, void *, int a1, int a2);
void __fastcall diplo_win_on_iface_button_clicked_redirect(DiploWin *self, void *, int a1);
void __fastcall diplo_win_on_iface_button_toggled_redirect(DiploWin *self, void *, int a1, int a2);
int __fastcall diplo_win_on_iface_dialog_back_draw_redirect(DiploWin *self, void *, int a1);
void __fastcall diplo_win_unk2_redirect(DiploWin *self, void *);
