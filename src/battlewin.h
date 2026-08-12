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
  void on_iface_button_clicked(int a1);
  BattleWin() { ; }
  ~BattleWin();
  void on_iface_left_click(int a1, int a2);
  void on_iface_right_click(int a1, int a2);
  void on_iface_right_down(int a1, int a2);
  void on_iface_mouse_move(int a1, int a2);
  void on_iface_mouse_leave(int a1, int a2);
  void on_iface_selected(int a1, int a2);

 private:
  // Not a Time-derived class, and the Time is at +8 rather than at +0.
  // ??0BattleWin@@QAE@XZ (0x00422EE0) is 27 bytes and settles both:
  // `lea ecx, [esi + 8]` at 0x00422EE3, then `mov dword ptr [esi], 0x66a6e4`
  // - the SubInterface vftable eight other window constructors store at
  // their +0xA14 - then `call ??0Time@@QAE@XZ`. ??1BattleWin (0x00422ED0),
  // ?stop_timer (0x00421B40) and ?pulse_timer (0x00421B20) each reach the
  // Time the same way, with `add ecx, 8`.
  //
  // The IDB disagrees, placing `time` at 0x4 with a 4-byte `subIFace` before
  // it; the three `add ecx, 8` sites are the image and win. sizeof(Time) is
  // pinned at 0x28, so 8 + 0x28 == 0x30 and the static_assert below is what
  // checks this split.
  //
  // Held as STORAGE, not as `SubInterface subIFace_; Time time_;`: a real
  // Time member would give BattleWin a constructor and destructor the
  // original inlines, and neither class is named by this header today.
  uint8_t unmapped_0_[0x8];
  uint8_t time_storage_[0x28];  // 0x8

  // Storage the image proves is here: its own methods reach 0xA0.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  // 10 member(s) from the IDA database, 0 named; it starts a member at 0x30, which is where src/ ends.

  uint8_t field_30_[0x10];  // 0x30
  uint8_t field_40_[0x10];  // 0x40
  uint8_t field_50_[0x10];  // 0x50
  uint8_t field_60_[0x10];  // 0x60
  uint8_t field_70_[0x10];  // 0x70
  uint32_t field_80_;  // 0x80
  uint32_t field_84_;  // 0x84
  uint32_t field_88_;  // 0x88
  uint32_t field_8C_;  // 0x8C
  uint8_t field_90_[0x10];  // 0x90
};

static_assert(sizeof(BattleWin) == 0xA0,
              "BattleWin layout must match the original executable");

void __fastcall battle_win_dtor_redirect(BattleWin *self, void *);
void __fastcall battle_win_on_iface_left_click_redirect(BattleWin *self, void *, int a1, int a2);
void __fastcall battle_win_on_iface_right_click_redirect(BattleWin *self, void *, int a1, int a2);
void __fastcall battle_win_on_iface_right_down_redirect(BattleWin *self, void *, int a1, int a2);
void __fastcall battle_win_on_iface_mouse_move_redirect(BattleWin *self, void *, int a1, int a2);
void __fastcall battle_win_on_iface_mouse_leave_redirect(BattleWin *self, void *, int a1, int a2);
void __fastcall battle_win_on_iface_selected_redirect(BattleWin *self, void *, int a1, int a2);
