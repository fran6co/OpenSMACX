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
#include "maininterface.h"  // MainInterfaceGlobal and shared interface storage
#include "time.h"
#include "font.h"
#include "graphicwin.h"
#include "subinterface.h"
#include "flic.h"
#include "flatbutton.h"

 /*
  * DiploPop class
  *
  * GraphicWin is the primary base; the interface-mode base begins at 0xA14.
  * Construction at 0x0043EFF0 and destruction at 0x00440D50 establish the
  * member lifetimes below. In particular, 0xA94 is a faction id, not a Time.
  */
// RECOVERY_BASE: v1 DiploPop SubInterface 0xA14 0x0043EFF0
// Constructor installs this original interface table at the declared base.
// Its nineteen observed slots do not independently establish its end bound.
// RECOVERY_GENERATED: v1 vtable 0x0066B424 ? ??_7DiploPop@@6B@ 0x0043EFF0
class DiploPop : public GraphicWin, public SubInterface {
 public:
  // 0x004406D0. `poll` is the Win32 modal-pump callback Win::set_modal stores.
  int set_modal(int flag, int (__cdecl *poll)(), Win *owner);
  // 0x004406F0
  void release_modal();
  // homed from 0043f290.cpp
  void on_button_clicked(int a1);

 public:
  // homed from 0043f5c0.cpp
  void stop();

 public:
  // homed from 0043f140.cpp
  void show(int a1);

 public:
  // homed from 0043f0f0.cpp
  void close();

 public:
  void hide();
  DiploPop();
  ~DiploPop();
  void draw_hq();
  void draw_info();
  void portrait_anim();
  void bioscan_anim();
  void on_iface_left_click(int, int);
  void on_iface_right_click(int, int);
  void on_iface_left_down(int, int);
  void on_iface_right_down(int, int);
  void on_iface_mouse_move(int, int);
  void on_iface_mouse_leave(int, int);
  void on_iface_selected(int, int);
  void on_iface_left_double_click(int, int);
  void on_iface_right_double_click(int, int);
  void on_iface_button_toggled(int, int);

  // ---- the interface table, slot for slot against 0x66B424 ----------------
  // Slots 1, 2, 6, 14, 17 and 18 hold the shared no-ops every window carries
  // (0x00404260 / 0x00404250 / 0x00404240); 6, 17 and 18 are
  // Shared-stub SubInterface slots are named here so DiploPop's actual table
  // contains the measured targets.
  void on_iface_scrolled(int, int) override;
  void on_iface_scrolling(int, int) override;
  void on_iface_left_up(int, int) override;
  int on_iface_button_pressed(int) override;
  int on_iface_dialog_back_draw(GraphicWin *) override;
  int on_iface_dialog_item_back_draw(GraphicWin *, int, int,
                                    RECT *) override;
  void on_status(int mode);
  // Slot13, 0x00440550: typed behavior recovered in diplopop.cpp.
  void on_iface_button_clicked(int button_id);
  // Slot 16, 0x00440620.
  void on_iface_group_clicked(int group, int x_coord, int y_coord);

  // PROVED, and now a real type rather than eight opaque bytes:
  // The SubInterface that used to sit here as a member is now the SECOND
  // BASE, above. MSVC lays non-virtual bases in declaration order, so it is
  // at sizeof(GraphicWin) == 0xA14 either way and no offset moves.
  // RECOVERY_MEMBER: v2 DiploPop font1_ Font 0xA1C 0x0043EFF0
  Font font1_;
  // RECOVERY_MEMBER: v2 DiploPop font2_ Font 0xA44 0x0043EFF0
  Font font2_;
  // RECOVERY_MEMBER: v2 DiploPop font3_ Font 0xA6C 0x0043EFF0
  Font font3_;
  // start stores its faction argument at 0x0043F30D. Construction, close
  // and stop reset it to -1; show tests that sentinel before displaying.
  // RECOVERY_MEMBER: v1 DiploPop counterpart_faction_ 0xA94 0x0043EFF0
  int counterpart_faction_;
  // RECOVERY_MEMBER: v2 DiploPop time1_ Time 0xA98 0x0043EFF0
  Time time1_;
  // RECOVERY_MEMBER: v2 DiploPop time2_ Time 0xAC0 0x0043EFF0
  Time time2_;
  // RECOVERY_MEMBER: v2 DiploPop flic1_ Flic 0xAE8 0x0043EFF0
  Flic flic1_;
  // The second Flic ends at 0x20B0; 0x20AC belongs to it, not a separate field.
  // RECOVERY_MEMBER: v2 DiploPop flic2_ Flic 0x15CC 0x0043EFF0
  Flic flic2_;
  uint32_t field_20B0_;  // 0x20B0
  uint32_t field_20B4_;  // 0x20B4
  uint32_t field_20B8_;  // 0x20B8
  uint32_t field_20BC_;  // 0x20BC
  // Borrowed faction-HQ portrait used by draw_hq; not owned or destroyed.
  Sprite *hq_sprite_;  // 0x20C0
  uint32_t field_20C4_;  // 0x20C4
  uint32_t field_20C8_;  // 0x20C8
  uint32_t field_20CC_;  // 0x20CC
  uint32_t field_20D0_;  // 0x20D0
  uint32_t field_20D4_;  // 0x20D4
  uint32_t field_20D8_;  // 0x20D8
  uint32_t field_20DC_;  // 0x20DC
  uint32_t field_20E0_;  // 0x20E0
  uint32_t field_20E4_;  // 0x20E4
  uint32_t field_20E8_;  // 0x20E8
  uint32_t field_20EC_;  // 0x20EC
  uint32_t field_20F0_;  // 0x20F0
  RECT rect1_;  // 0x20F4, IDB `rect1`
  uint32_t field_2104_;  // 0x2104
  uint32_t field_2108_;  // 0x2108
  uint32_t field_210C_;  // 0x210C
  uint32_t field_2110_;  // 0x2110
  uint32_t field_2114_;  // 0x2114
  uint32_t field_2118_;  // 0x2118
  uint32_t field_211C_;  // 0x211C
  uint32_t field_2120_;  // 0x2120
  uint32_t field_2124_;  // 0x2124
  uint32_t field_2128_;  // 0x2128
  uint32_t field_212C_;  // 0x212C
  uint32_t field_2130_;  // 0x2130
  uint32_t field_2134_;  // 0x2134
  uint32_t field_2138_;  // 0x2138
  uint32_t field_213C_;  // 0x213C
  uint32_t field_2140_;  // 0x2140
  uint32_t field_2144_;  // 0x2144
  uint32_t field_2148_;  // 0x2148
  uint32_t field_214C_;  // 0x214C
  uint32_t field_2150_;  // 0x2150
  uint32_t field_2154_;  // 0x2154
  // Constructor 0x0043F08A..0x0043F0A6 supplies count 3 and stride 0xB4C;
  // destructor 0x00440DBA..0x00440DEF tears down the same complete array.
  // RECOVERY_MEMBER: v4 DiploPop flatButtons_ FlatButton[3] 0x2158 0x21E4 0x0043EFF0
  FlatButton flatButtons_[3];
};

static_assert(sizeof(DiploPop) == 0x433C,
              "DiploPop member lifetimes must cover the original footprint");

extern DiploPop DiplomacyPopup;

// Time invokes these as plain two-argument __cdecl callbacks. The second
// stack argument is the DiploPop receiver; neither callback has hidden this.
void __cdecl timer_callback_daemon(int unused, DiploPop *popup);
void __cdecl timer_callback_daemon2(int unused, DiploPop *popup);
