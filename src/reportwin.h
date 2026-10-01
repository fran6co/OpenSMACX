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
#include "time.h"
#include "spot.h"
#include "listbox.h"
#include "flatbutton.h"
#include "buttongroup.h"
#include "buffer.h"
#include "graphicwin.h"
#include "scroll.h"
#include "flic.h"

 /*
  * ReportWin class
  *
  * Derives from GraphicWin by ordinary single inheritance - its constructor calls
  * GraphicWin's on an unadjusted `this` - so it is written as the original wrote
  * it and needs no ABI workaround. GraphicWin and ReportWin's current 0xFABC
  * extent are pinned by their layout assertions. Member declaration order
  * follows the constructor and reverse destructor calls in the original.
  */
class ReportWin : public GraphicWin {
  friend class ReportIf;
 public:
  // The report interface and Datalink borrow this window's research selection.
  // Original singleton876478+FAB8=885F30; not independent process storage.
  int &shared_research_focus() { return field_FAB8_; }
  // The same +59AC word is used by the energy selection and research transition.
  // Singleton876478+59AC=87BE24; report() tests its signed -1 sentinel.
  int &shared_report_selection() { return field_59AC_; }
  // Original8846E8=876478+E270; report_intel writes the owning window's flag.
  void select_intel_topic() { field_E270_ = 1; }
  ReportWin();
  // Ordinary out-of-line body; C++ owns reverse member/base teardown.
  ~ReportWin();
  void init();
  // Win slot 12: reset and repaint the active ReportIf category.
  virtual void on_redraw();
  void on_left_click(int x, int y);
  void right_menu(int x, int y, int base_id);
  void on_right_click(int x, int y);
  void on_button_clicked(int id);
  void on_group_clicked(int group, int id, int state);
  void draw_ops();
  void draw_sat();
  void draw_security();
  void draw_score();
  void draw_intel();
  void start_attack(int attacker, int target, int attack, int defense);
  void on_mouse_move(int a1, int a2);
  void on_mouse_leave(int a1, int a2);
  // 0x0049B220 highlights report-category buttons and redraws their lab
  // connectors on this window or Datalink's canvas.
  void on_button_passover(int button_id, int state);
  // The two animation ticks the daemons in leaf_recoveries.cpp drive. Public,
  // __thiscall, void(void). Both use genuine animation members and the
  // existing explosion sprite run; action_sat_attack remains a dependency.
  void sat_anim();  // 0x0049FE40  ?sat_anim@ReportWin@@QAEXXZ
  void exp_anim();  // 0x004A0100  ?exp_anim@ReportWin@@QAEXXZ
  // 0x00496BB0 ?reset@ReportWin@@QAEXXZ resets the report controls, working
  // rectangle and the two report panel frames.
  void reset();
  // 0x00498B30: referenced by Datalink; full report-page recovery is pending.
  void draw_labs();
  void show(int visible);
  void hide();
  void on_left_double_click(int x, int y);
  void on_scrolled(int code, int pos);
  void on_scrolling(int code, int pos);
  // 0x00498110 ?UNK1@ReportWin@@QAEXHHHH@Z sets the embedded scroll range,
  // page size and selected position.
  void UNK1(int position, int minimum, int maximum, int page_size);
  // 0x00498280 ?on_key_click@ReportWin@@QAEHHH@Z - public, __thiscall,
  // int(int, int). Always answers 1; Enter dismisses, F1-F8 (0x70-0x77) pick
  // the report category, 0x21-0x28 drive the scroll, 0x30-0x5A/Escape
  // dismiss.
  int on_key_click(int key, int code);
  // 0x0049BCD0 ?draw_energy@ReportWin@@QAEXXZ - public, __thiscall,
  // void(void), unrecovered: the energy panel redraw. on_selected refreshes
  // it after every topic selection.
  void draw_energy();
  // 0x0049EB10 ?draw_projects@ReportWin@@QAEXXZ - projects-page renderer;
  // on_scrolled/on_scrolling select this page renderer at page 5.
  void draw_projects();
  // 0x0049B450 ?labs_line@ReportWin@@QAEXHHHHHHH@Z - public, __thiscall,
  // void(int, int, int, int, int, int, int), unrecovered: one lab-connection
  // line of the research tree. The third argument is unused by the body
  // (ret 0x20); the last selects the box-style pair.
  void labs_line(int first_id, int second_id, int, int branch_id,
                 int x_offset, int line_height, int style_select);
  // 0x0049D490 ?on_selected@ReportWin@@QAEXHH@Z - public, __thiscall,
  // void(int, int). The topic-panel selection callback: the id below 0x400
  // is a base id, above it a tech id; the pair at 0x59AC/0x59B0 remembers
  // the selection kind. The second argument is unused (ret 8).
  void on_selected(int selection_id, int);

  // Storage the image proves is here: its own methods reach 0xFABC.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  // 100 member(s) from the IDA database, 21 named; it starts a member at 0xA14, which is where src/ ends.

  RECT rect1_;  // 0xA14, IDB `rect1`
  uint32_t field_A24_;  // 0xA24
  uint32_t field_A28_;  // 0xA28
  uint32_t field_A2C_;  // 0xA2C
  uint32_t field_A30_;  // 0xA30
  uint32_t field_A34_;  // 0xA34
  // Genuine members are constructed after GraphicWin, in declaration order.
  FlatButton flatButtons1_[7];  // 0xA38
  uint32_t field_594C_;  // 0x594C
  uint32_t field_5950_;  // 0x5950
  uint32_t field_5954_;  // 0x5954
  uint32_t field_5958_;  // 0x5958
  uint32_t field_595C_;  // 0x595C
  uint32_t field_5960_;  // 0x5960
  uint32_t field_5964_;  // 0x5964
  uint32_t field_5968_;  // 0x5968
  uint32_t field_596C_;  // 0x596C
  uint32_t field_5970_;  // 0x5970
  // RECOVERY_MEMBER: v1 ReportWin energy_panel_rect_ 0x5974 0x0049BCD0
  RECT energy_panel_rect_;  // 0x5974; all four edges form one frame
  RECT report_panel_rect_;  // 0x5984, passed to Buffer::box_sprite
  Spot spot_;  // 0x5994
  uint32_t field_59A0_;  // 0x59A0
  uint32_t field_59A4_;  // 0x59A4
  uint32_t field_59A8_;  // 0x59A8
  int field_59AC_;  // 0x59AC; singleton87BE24, signed selection/sentinel
  uint32_t field_59B0_;  // 0x59B0
  uint32_t selected_bases_[16];  // 0x59B4, original 512-base selection bitmap
  uint32_t selected_prototypes_[4];  // 0x59F4, original 128-row selection bitmap
  uint32_t field_5A04_;  // 0x5A04
  uint32_t field_5A08_;  // 0x5A08
  // Original87BE84=ReportWinGlobal876478+5A0C. ReportIf borrows this member.
  ListBox listBox_;  // 0x5A0C
  FlatButton flatButton2_;  // 0x6560
  FlatButton flatButton3_;  // 0x70AC
  RECT report_frame_rect_;  // 0x7BF8, expanded and restored around box_sprite
  ButtonGroup buttonGroup1_;  // 0x7C08
  FlatButton flatButton4_;  // 0x7C9C
  FlatButton flatButton5_;  // 0x87E8
  FlatButton flatButton6_;  // 0x9334
  ButtonGroup buttonGroup2_;  // 0x9E80
  FlatButton flatButton7_;  // 0x9F14
  FlatButton flatButton8_;  // 0xAA60
  FlatButton flatButton9_;  // 0xB5AC
  uint32_t field_C0F8_;  // 0xC0F8
  uint32_t field_C0FC_;  // 0xC0FC, selected scroll position
  uint32_t field_C100_;  // 0xC100
  uint32_t field_C104_;  // 0xC104
  uint32_t field_C108_;  // 0xC108
  Scroll scroll_;  // 0xC10C
  // The original redraw and passover paths share this full rectangle.
  // RECOVERY_MEMBER: v1 ReportWin report_work_rect_ 0xE258 0x0049B220
  RECT report_work_rect_;  // 0xE258
  uint32_t field_E268_;  // 0xE268
  uint32_t field_E26C_;  // 0xE26C
  uint32_t field_E270_;  // 0xE270
  Time time1_;  // 0xE274
  uint32_t field_E29C_;  // 0xE29C
  uint32_t field_E2A0_;  // 0xE2A0
  uint32_t field_E2A4_;  // 0xE2A4
  uint32_t field_E2A8_;  // 0xE2A8
  uint32_t field_E2AC_;  // 0xE2AC
  RECT satellite_rect_;  // 0xE2B0
  Time time2_;  // 0xE2C0
  Time time3_;  // 0xE2E8
  RECT explosion_rect_;  // 0xE310
  // Declaration order constructs these Buffers after Scroll and Time.
  Buffer buffer1_;  // 0xE320
  Buffer buffer2_;  // 0xE8A8
  // 0xEE30 is a float of ReportWin's own, NOT the head of the Flic. The Flic
  // starts four bytes later, and the image says so four separate times:
  //   ?sat_anim@ReportWin@@QAEXXZ multiplies by 0xEE30 as a scalar
  //     (fmul dword ptr [esi + 0xee30] at 0x0049FEAD) and then, in the same
  //     body, hands lea ecx, [esi + 0xee34] to ?decode_frame@Flic@@QAEHHH@Z
  //     at 0x0049FF0E - one function, both facts.
  //   ??0ReportWin@@QAE@XZ: lea ecx, [esi + 0xee34] -> ??0Flic@@QAE@XZ at
  //     0x004AD812, straight after lea ecx, [esi + 0xe8a8] -> ??0Buffer.
  //   ??1ReportWin@@QAE@XZ: lea edi, [esi + 0xee34] -> ?close@Flic@@QAEXXZ at
  //     0x004AD3E9, then lea ecx, [edi + 4] -> ??1Buffer@@QAE@XZ and
  //     lea ecx, [edi + 0x5bc] -> ??1Palette@@QAE@XZ, which land exactly on
  //     Flic::buffer_ (0x4) and Flic::palette_ (0x5BC) in src/flic.h.
  // So the fmul is not a mistyped Flic::field_0_ and not a Flic that is really
  // a float; it is a member the old layout swallowed.
  // docs/recovery/idb-members.csv still carries `ReportWin,0xEE30,flic,2788`.
  // That row is four bytes early, and the receiver of ??0Flic@@QAE@XZ
  // withdraws it - an IDB member row is a name, not a measurement.
  float field_EE30_;  // 0xEE30, x87 dword load -> float
  Flic flic_;  // 0xEE34, ends at 0xF918
  // 0xF918, not 0xF914. The first own-access past the Flic is
  // mov eax, dword ptr [esi + 0xf918] at 0x0049FFF4 in
  // ?start_attack@ReportWin@@QAEXHHHH@Z, which then writes 0xF91C..0xF934;
  // nothing in the image ever touches 0xF914. The trailing extent to 0xFABC
  // is unchanged, so sizeof(ReportWin) is what it was.
  uint32_t field_F918_;  // 0xF918
  uint32_t field_F91C_;  // 0xF91C
  uint32_t field_F920_;  // 0xF920
  uint32_t field_F924_;  // 0xF924
  uint32_t field_F928_;  // 0xF928
  uint32_t field_F92C_;  // 0xF92C
  uint32_t field_F930_;  // 0xF930
  uint32_t field_F934_;  // 0xF934
  float satellite_x_[16];  // 0xF938
  float satellite_y_[16];  // 0xF978
  float satellite_dx_[16];  // 0xF9B8
  float satellite_dy_[16];  // 0xF9F8
  float satellite_acceleration_[16];  // 0xFA38
  int satellite_live_[16];  // 0xFA78
  int field_FAB8_;  // 0xFAB8; singleton885F30, signed research selection
};

static_assert(sizeof(ReportWin) == 0xFABC,
              "ReportWin layout must match the legacy ABI");


// The process-wide report window. THE OBJECT, at 0x00876478 in the image -
// REAL STORAGE, defined in reportwin.cpp: the original's dynamic initializer
// at 0x00496930 constructs it (`mov ecx, 0x876478; call ??0ReportWin@@QAE@XZ`
// plus the atexit registration), the WorldClimateWorldWin shape.
// reportif.cpp's done() releases its modal loop, ReportWin::on_key_click
// closes it, and on_selected/on_button_clicked drive its two list boxes
// below.
extern ReportWin ReportWinGlobal;  // 0x00876478

// The fixed890208/890D5C list receivers belong to ReportIfGlobal's
// listBox1_/listBox2_. ReportWin borrows them; no independent storage/lifetime.


// Neither animation tick is recovered; both forward to the original image.
// ?sat_anim@ReportWin@@QAEXXZ and ?exp_anim@ReportWin@@QAEXXZ are public,
// __thiscall, void(void), so one pointer-to-member shape serves both.
// sat_anim is homed in reportwin.cpp; exp_anim still forwards to the
// original image through pending_bodies.cpp. Both are public, __thiscall,
// void(void), so one pointer-to-member shape serves the remaining seam.
