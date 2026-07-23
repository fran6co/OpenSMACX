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
  ReplayWin() { ; }
  ~ReplayWin() { ; }
  void on_left_double_click(int, int);
  void on_right_double_click(int, int);
  void on_mouse_move(int, int);
  void on_right_down(int, int);
  void on_left_down(int, int);
  void on_left_up(int, int);

 private:
  uint8_t unmapped_[0x252F0];
};

void __fastcall replay_win_on_left_double_click_redirect(ReplayWin *self, void *, int a1, int a2);
void __fastcall replay_win_on_right_double_click_redirect(ReplayWin *self, void *, int a1, int a2);
void __fastcall replay_win_on_mouse_move_redirect(ReplayWin *self, void *, int a1, int a2);
void __fastcall replay_win_on_right_down_redirect(ReplayWin *self, void *, int a1, int a2);
void __fastcall replay_win_on_left_down_redirect(ReplayWin *self, void *, int a1, int a2);
void __fastcall replay_win_on_left_up_redirect(ReplayWin *self, void *, int a1, int a2);
