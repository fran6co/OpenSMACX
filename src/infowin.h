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
#include "time.h"
#include "font.h"
#include "pushbutton.h"

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
  // homed from 00459420.cpp. EVIDENCE-FREE as a name - see the marker in
  // infowin.cpp: no caller, no vtable slot, no CSV row, and the body reads
  // none of its five arguments, so they stay unnamed here too.
  void UNK5(int, int, int, int, int);

 public:
  // 0x00459150, a pending_bodies forwarder.
  void timer_proc(int a1);

 public:
  // 0x00458930  ?on_left_click@InfoWin@@QAEXHH@Z - public, __thiscall,
  // void(int, int). Declared, not defined: the body (00458930.cpp) is still an
  // artifact, so anything that reaches it goes through the pending_bodies
  // forwarder until it homes. MainInterface's on_iface_left_click /
  // on_iface_right_click dispatch here.
  void on_left_click(int xCoord, int yCoord);

  void on_right_click(int a, int b);
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
  void set_redraw_phase(int phase) { redraw_phase_ = phase; }
  void reset();
  void change(int a1);
  void UNK3(int menu_id, int item_id);

  // THE REST OF THE CLASS, declared ahead of their homing. Each of these
  // calls at least one sibling - draw_text alone is reached by
  // on_left_click, timer_proc and set_state - so no batch can compile while
  // a declaration is missing: the homing tool declares only the body it is
  // moving and reverts the whole batch on the first undeclared name.
  // Arity and return type come off the image's mangled symbols.
  void setup_text();                                  // 0x00456300
  void draw_text(int a1);                             // 0x00458420
  // UNK4 IS EVIDENCE-FREE AS A NAME. Searched 2026-09-06: the catalogue
  // records callers 0 and call targets 0 for 0x00458D60, the mangled name is
  // `QAEX` so it is not virtual and no vtable slot names it, and
  // docs/recovery/behaviour-member-names.csv has no InfoWin entry. The body
  // only re-derives the two info-panel sizes (0xc8 x 0xb4 under 1000 px,
  // 0xfa x 0xd4 at and above) into the same globals InfoWin::init reads, so
  // its purpose is legible even though its name is not.
  void UNK4();                                        // 0x00458D60
  int state_valid(int a1);                            // 0x00458DC0
  void set_state(int a1, int a2);                     // 0x00458F10
  void timer_change(int a1);                          // 0x00458FF0
  void init();                                        // 0x004592A0

 private:
  // Owned subobjects are constructed in declaration order by 0x00459500
  // and destroyed in reverse order by 0x004594A0. Gaps remain unnamed;
  // these offsets do not by themselves prove the complete original extent.
  uint8_t field_0_[0x10];  // 0x0
  uint32_t field_10_;  // 0x10
  uint32_t field_14_;  // 0x14
  uint32_t field_18_;  // 0x18
  uint32_t field_1C_;  // 0x1C
  uint32_t field_20_;  // 0x20
  uint32_t field_24_;  // 0x24
  uint32_t field_28_;  // 0x28
  uint32_t field_2C_;  // 0x2C
  // RECOVERY_MEMBER: v2 InfoWin timer_ Time 0x30 0x00459500
  Time timer_;
  // RECOVERY_MEMBER: v2 InfoWin font_ Font 0x58 0x00459500
  Font font_;
  uint32_t field_80_;  // 0x80
  uint32_t field_84_;  // 0x84
  uint32_t field_88_;  // 0x88
  uint32_t field_8C_;  // 0x8C
  // Original +0x90: Console::editor_save (0x004E09E0) writes 7 here
  // before redrawing the fixed InfoWin at 0x007AD2A0. This is member
  // storage, not a separate global at 0x007AD330; other values are unnamed.
  // RECOVERY_MEMBER: v1 InfoWin redraw_phase_ 0x90 0x004E09E0
  int redraw_phase_;
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
  // RECOVERY_MEMBER: v2 InfoWin button_ PushButton 0x9D0 0x00459500
  PushButton button_;
};


// InfoWin::timer_proc is not recovered yet.

// right_menu is still a seam-forwarded body: it calls help_any and
// GraphicWin::update(RECT *, GraphicWin *), and until that overload is
// declared and defined the real body cannot home. Rebindable so tests can
// substitute a probe.
typedef void (OriginalObject::*func_info_win_right_menu)(int, int);
extern func_info_win_right_menu InfoWinOriginalRightMenu;  // 0x004589C0
