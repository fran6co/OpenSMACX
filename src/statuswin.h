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

#include "caviar.h"
#include "subinterface.h"
#include "font.h"
#include "spot.h"

 /*
  * StatusWin class
  *
  * The original does not derive this from Caviar: the constructor builds
  * one at +0x30 on an offset `this`, making it a member. That distinction
  * was missed when this header was first written, because the check looked
  * only at which constructor ran first and not at whether `this` had been
  * adjusted before it.
  *
  * Native compiler evidence preserves sizeof(StatusWin) == 0x15EC and the
  * mapped camera vector/matrix at +0x1500/+0x150C. Unknown fields remain
  * explicit; this is not complete behavioral or runtime layout proof.
*/
class StatusWin {
  // MainInterface redraw completion reads the actual selected tile members.
  friend class MainInterface;
 public:
  // homed from 004b4990.cpp
  void UNK1();

 public:
  void redraw();
  // 0x004B6570, 8885 original bytes. The typed HUD renderer is owned here;
  // redraw() and navigation call it by name.
  void draw_status(int a1, int a2, int a3, int a4, int a5);

 public:
  StatusWin();
  ~StatusWin();
  void close();
  void set_loc(int x, int y);
  void reset();
  // The click/redraw family. Declared ahead of their bodies landing so the
  // homed MainInterface handlers can call them by name - the image reaches
  // these on the process-wide StatusWin at 0x008C5568.
  void on_left_click(int x, int y);            // 0x004B8E40
  void on_right_click(int a1, int a2, int a3); // 0x004B9010
  void on_left_double_click(int x, int y);     // 0x004B8990
  void on_redraw();                            // 0x004B8890
  void draw(int a1, int a2, int a3, int a4, int a5);  // 0x004B8830
  void reset_active();                         // 0x004B6530
  void draw_active(int a1, int a2, int a3);    // 0x004B4CF0
  void arrow_left();                           // 0x004B89C0
  void arrow_right();                          // 0x004B8BD0
  void init();                                 // 0x004B9FC0
  void UNK2();                                 // 0x004B4CD0
  void UNK3();                                 // 0x004B8DF0
  void UNK4();                                 // 0x004B9E90
  // 0x004B4A20, 451 bytes - homed in statuswin.cpp. go_reset calls this
  // member twice; the typed body still has a byte-restoration obligation.
  void set_title(int title_id);
  // Native combat stores at 507D83/507EC7/50888F/508C32 use this existing member.
  // Name remains provisional; preserve original storage and unconditional write.
  void set_field_15D4(int value) { field_15D4_ = value; }


 private:
  // Not a base class: the constructor builds a Caviar at +0x30, on an
  // offset `this`, so Caviar is a member and the bytes before it are
  // something else. What matters for set_loc is only that the four fields
  // below sit where the original puts them, which the unmapped span holds in
  // place and the test checks directly.
  uint8_t field_0_[0x30];  // 0x0

  // Caviar, Font x3 and Spot are ordinary typed members, in declaration
  // order matching the image's own construction order exactly (Caviar,
  // then the three Fonts, then Spot) and matching the DESTRUCTION order in
  // reverse (Spot, font3, font2, font1, then Caviar's own destructor, which
  // forwards to close() - see the note on Caviar's destructor). A real
  // member built implicitly gets no null-pointer guard around it; a
  // placement-new expression does (VC6 guards every `new (p) T()` with a
  // `cmp/je` on the pointer, since `operator new` may return null), which
  // is what an earlier raw-storage version of this class paid for no
  // reason - construction here never actually uses `operator new`.
  Caviar caviar_;  // 0x30, sizeof(Caviar)
  uint8_t field_1400_[0x100];  // 0x1400
  VOX_Vect camera_angles_;  // 0x1500
  VOX_Matrix camera_rotation_;  // 0x150C
  Font font1_;  // 0x1530
  Font font2_;  // 0x1558
  Font font3_;  // 0x1580
  uint32_t field_15A8_;  // 0x15A8
  // Cached height passed to all three Font::init calls; -1 forces refresh.
  int32_t font_size_;  // 0x15AC
  uint8_t field_15B0_[0x4];  // 0x15B0
  int32_t iTileX_;
  int32_t iTileY_;
  int32_t iUnitIndex_;
  int32_t iCityIndex_;
  // Signed: reset_active tests both against zero with the image's signed
  // `jge`/`jl`, and -1 is the "nothing active" value they hold between uses.
  int32_t field_15C4_;  // 0x15C4
  int32_t field_15C8_;  // 0x15C8
  int32_t field_15CC_;  // 0x15CC
  int32_t field_15D0_;  // 0x15D0
  int32_t field_15D4_;

  uint32_t fInOnClick_;  // 0x15D8
  uint32_t field_15DC_;  // 0x15DC

  // The image proves this is here: the constructor builds a Spot at exactly
  // this offset, right where the previously-mapped span ended.
  Spot spot_;  // 0x15E0
};


// 0x004B4970 ?say_orders@@YAXH@Z. Prints a vehicle's order line into
// StringTemp and hands it to the text form below, which does the drawing.
void say_orders(int veh_id);
// 0x004B43E0 ?say_orders@@YAXPADH@Z, 1421 bytes - not recovered, forwarded in
// statuswin.cpp so the int form can call it by name.
void say_orders(char *text, int veh_id);

// The process-wide SubInterface view is BattleWin's real global object.
// Defined by its owner in battlewin.cpp to keep this header's dependency
// direction acyclic.
SubInterface *sub_interface_global();


// The process-wide status/layout initialization bits at original 0x00939ED8.
// InfoWin sets bit 4, WorldWin bit 8, and StatusWin bit 0x20. This is real
// source storage, not a pointer back into the retired executable image.
extern uint32_t StatusFlags;

int __cdecl reset_status_and_initialize_desktop();
