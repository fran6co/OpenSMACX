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
  * Datalink class
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
class DLLEXPORT Datalink : GraphicWin {
 public:
  Datalink() { ; }
  ~Datalink() { ; }
  void UNK6();
  void UNK8();
  void UNK9();
  void on_left_click(int a1, int a2);
  void on_group_clicked(int a1, int a2);
  void on_mouse_move(int a1, int a2);
  void on_iface_left_click(int a1, int a2);
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
};

void __fastcall datalink_unk6_redirect(Datalink *self, void *);
void __fastcall datalink_unk8_redirect(Datalink *self, void *);
void __fastcall datalink_unk9_redirect(Datalink *self, void *);
void __fastcall datalink_on_left_click_redirect(Datalink *self, void *, int a1, int a2);
void __fastcall datalink_on_group_clicked_redirect(Datalink *self, void *, int a1, int a2);
void __fastcall datalink_on_mouse_move_redirect(Datalink *self, void *, int a1, int a2);
void __fastcall datalink_on_iface_left_click_redirect(Datalink *self, void *, int a1, int a2);
void __fastcall datalink_on_iface_right_click_redirect(Datalink *self, void *, int a1, int a2);
void __fastcall datalink_on_iface_left_down_redirect(Datalink *self, void *, int a1, int a2);
void __fastcall datalink_on_iface_right_down_redirect(Datalink *self, void *, int a1, int a2);
void __fastcall datalink_on_iface_mouse_move_redirect(Datalink *self, void *, int a1, int a2);
void __fastcall datalink_on_iface_mouse_leave_redirect(Datalink *self, void *, int a1, int a2);
void __fastcall datalink_on_iface_selected_redirect(Datalink *self, void *, int a1, int a2);
void __fastcall datalink_on_iface_left_double_click_redirect(Datalink *self, void *, int a1, int a2);
void __fastcall datalink_on_iface_right_double_click_redirect(Datalink *self, void *, int a1, int a2);
void __fastcall datalink_on_iface_button_clicked_redirect(Datalink *self, void *, int a1);
void __fastcall datalink_on_iface_button_toggled_redirect(Datalink *self, void *, int a1, int a2);
