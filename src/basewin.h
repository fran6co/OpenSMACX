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
  * BaseWin class
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
class DLLEXPORT BaseWin : GraphicWin {
 public:
  BaseWin() { ; }
  ~BaseWin() { ; }
  void close();
  void UNK4();
  void UNK6();
  void UNK7();
  void UNK5(int a1);
  void on_button_toggled(int a1, int a2);
  void on_iface_right_down(int a1, int a2);
  void on_iface_selected(int a1, int a2);
  void on_iface_left_click(int a1, int a2);
  void on_iface_right_click(int a1, int a2);
  void on_iface_left_double_click(int a1, int a2);
  void on_iface_right_double_click(int a1, int a2);
  void on_iface_scrolled(int a1, int a2);
};

void __fastcall base_win_close_redirect(BaseWin *self, void *);
void __fastcall base_win_unk4_redirect(BaseWin *self, void *);
void __fastcall base_win_unk6_redirect(BaseWin *self, void *);
void __fastcall base_win_unk7_redirect(BaseWin *self, void *);
void __fastcall base_win_unk5_redirect(BaseWin *self, void *, int a1);
void __fastcall base_win_on_button_toggled_redirect(BaseWin *self, void *, int a1, int a2);
void __fastcall base_win_on_iface_right_down_redirect(BaseWin *self, void *, int a1, int a2);
void __fastcall base_win_on_iface_selected_redirect(BaseWin *self, void *, int a1, int a2);

// BaseWin embeds an interface subobject at 0xA14; these four are reached
// through its vtable, so their `this` points there and is adjusted back to
// the BaseWin before dispatching to iface_click, which is not recovered.
typedef void (__thiscall func_base_win_iface_click)(BaseWin *self, int a1,
                                                    int a2, int button,
                                                    int is_double);
extern func_base_win_iface_click *BaseWinIfaceClick;

// draw_supported is not recovered.
typedef void (__thiscall func_base_win_draw_supported)(BaseWin *self, int a1);
extern func_base_win_draw_supported *BaseWinDrawSupported;

void __fastcall base_win_on_iface_left_click_redirect(BaseWin *self, void *, int a1, int a2);
void __fastcall base_win_on_iface_right_click_redirect(BaseWin *self, void *, int a1, int a2);
void __fastcall base_win_on_iface_left_double_click_redirect(BaseWin *self, void *, int a1, int a2);
void __fastcall base_win_on_iface_right_double_click_redirect(BaseWin *self, void *, int a1, int a2);
void __fastcall base_win_on_iface_scrolled_redirect(BaseWin *self, void *, int a1, int a2);
