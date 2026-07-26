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

 /*
  * BattleWin class
  *
  * The original does not derive this from Time: the constructor builds
  * one at +0x0 on an offset `this`, making it a member. That distinction
  * was missed when this header was first written, because the check looked
  * only at which constructor ran first and not at whether `this` had been
  * adjusted before it.
  *
  * The layout is not established and nothing pins its sizeof.
*/
class DLLEXPORT BattleWin {
 public:
  BattleWin() { ; }
  ~BattleWin();
  void on_iface_left_click(int a1, int a2);
  void on_iface_right_click(int a1, int a2);
  void on_iface_right_down(int a1, int a2);
  void on_iface_mouse_move(int a1, int a2);
  void on_iface_mouse_leave(int a1, int a2);
  void on_iface_selected(int a1, int a2);

 private:
  // Not a base class: the constructor builds a Time at +0x0 on an offset
  // `this`, which puts a Time member at the start rather than a Time base.
  // The destructor tears down a Time at +8, so the object reaches at least
  // 0x30; nothing further about the extent is established.
  uint8_t unmapped_0_[0x30];
};

void __fastcall battle_win_dtor_redirect(BattleWin *self, void *);
void __fastcall battle_win_on_iface_left_click_redirect(BattleWin *self, void *, int a1, int a2);
void __fastcall battle_win_on_iface_right_click_redirect(BattleWin *self, void *, int a1, int a2);
void __fastcall battle_win_on_iface_right_down_redirect(BattleWin *self, void *, int a1, int a2);
void __fastcall battle_win_on_iface_mouse_move_redirect(BattleWin *self, void *, int a1, int a2);
void __fastcall battle_win_on_iface_mouse_leave_redirect(BattleWin *self, void *, int a1, int a2);
void __fastcall battle_win_on_iface_selected_redirect(BattleWin *self, void *, int a1, int a2);
