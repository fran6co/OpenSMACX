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
#include "subinterface.h"
#include "time.h"

 /*
  * BattleWin class
  *
  * The original does not derive this from Time: the constructor builds
  * one at +0x0 on an offset `this`, making it a member. That distinction
  * was missed when this header was first written, because the check looked
  * only at which constructor ran first and not at whether `this` had been
  * adjusted before it.
  *
  * DERIVED 2026-09-06 from the real SubInterface base, and the derivation is
  * what the constructor's second vftable store was asking for: ??0BattleWin
  * writes the all-pure base table 0x66A6E4 at +0, builds the Time member, then
  * overwrites +0 with BattleWin's own nineteen-entry table 0x66AA44 - the
  * base-then-derived pair only a polymorphic base produces. The interface
  * base is 8 bytes here rather than at +0xA14, because BattleWin has no
  * GraphicWin front; the Time follows the interface subobject.
  *
  * The layout is not established beyond the static_assert below.
*/
// RECOVERY_BASE: v1 BattleWin SubInterface 0x0 0x00422EE0
// Constructor installs this original interface table at the declared base.
// Its nineteen observed slots do not independently establish its end bound.
// RECOVERY_GENERATED: v1 vtable 0x0066AA44 ? ??_7BattleWin@@6B@ 0x00422EE0
class BattleWin : public SubInterface {
 public:
  // The interface table, slot for slot against 0x66AA44. Slots 1, 2, 6, 11,
  // 12, 14, 15, 17 and 18 hold the shared no-ops every window carries
  // (0x00404260 / 0x00404250 / 0x00404240); 6, 17 and 18 are
  // These shared-stub slots are concrete SubInterface slots; BattleWin names
  // them here so its actual table has the measured targets.
  void on_iface_scrolled(int, int) override;
  void on_iface_scrolling(int, int) override;
  void on_iface_left_up(int, int) override;
  void on_iface_left_double_click(int, int) override;
  void on_iface_right_double_click(int, int) override;
  int on_iface_button_pressed(int) override;
  void on_iface_button_toggled(int, int) override;
  int on_iface_dialog_back_draw(GraphicWin *) override;
  int on_iface_dialog_item_back_draw(GraphicWin *, int, int,
                                    RECT *) override;

  void on_status(int mode);
  void clear();
  void battle_report(int first_vehicle_id, int second_vehicle_id,
                     int attack_odds, int defense_odds, int flags);
  void pulse_timer();
  void stop_timer();
  void on_iface_left_click(int, int);
  void on_iface_right_click(int, int);
  void on_iface_left_down(int x_coord, int y_coord);
  void on_iface_right_down(int, int);
  void on_iface_mouse_move(int, int);
  void on_iface_mouse_leave(int, int);
  void on_iface_selected(int, int);
  // (group, x, y): the image tests only the first and forwards the other two.
  void on_iface_group_clicked(int group, int x_coord, int y_coord);
  // Slot13, 0x00422DC0: release this mode, then call MainInterface's callback
  // directly through explicit qualification rather than virtual dispatch.
  void on_iface_button_clicked(int button_id);

  // DEFINED IN battlewin.cpp, not empty: its marker claims 27 image bytes and
  // the body was `{ ; }` - the image installs the SubInterface vtable, builds
  // the Time member at +8, then installs BattleWin's own vtable. Promoted from
  // src/recovered/units/00422ee0.cpp.
  BattleWin();
  // NO DECLARED DESTRUCTOR: the image's ??1BattleWin is two instructions -
  // `add ecx, 8` and a tail `jmp` into ~Time - which is the compiler's
  // IMPLICIT destructor destroying the Time member and nothing else. A
  // user-written `~BattleWin() {}` also emits the base vptr restore, which
  // the image does not have; leaving it to the compiler is the whole fix.

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
  // A REAL MEMBER, not storage: the image calls ??0Time out of line from the
  // constructor and tail-jumps into ??1Time from the destructor, which is
  // exactly what a member of a non-trivial class gets. The old
  // `uint8_t time_storage_[0x28]` spelling needed both calls written by hand
  // and cost the destructor its vptr store.
  Time time_;  // 0x8

  // Storage the image proves is here: its own methods reach 0xA0.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  // 10 member(s) from the IDA database, 0 named; it starts a member at 0x30, which is where src/ ends.

  // clear() passes these four consecutive rectangles to the shared canvas.
  // RECOVERY_MEMBER: v1 BattleWin battle_rects_ 0x30 0x00422C50
  RECT battle_rects_[4];
  uint32_t field_70_;  // 0x70
  uint32_t field_74_;  // 0x74
  uint32_t field_78_;  // 0x78
  uint32_t field_7C_;  // 0x7C
  uint32_t field_80_;  // 0x80
  uint32_t field_84_;  // 0x84
  uint32_t field_88_;  // 0x88
  uint32_t field_8C_;  // 0x8C
  uint32_t field_90_;  // 0x90
  uint32_t field_94_;  // 0x94
  uint32_t field_98_;  // 0x98
  uint32_t field_9C_;  // 0x9C
};

static_assert(sizeof(BattleWin) == 0xA0,
              "BattleWin layout must match the original executable");

// Source-owned battle interface used by StatusWin's combat-report fast path.
extern BattleWin g_BattleWin;
