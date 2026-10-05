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
// Each pane owns twelve fixed-capacity text rows and parallel row values.
typedef char InfoPaneText[12][80];
typedef int InfoPaneRows[12];
static_assert(sizeof(InfoPaneText) == 0x3C0, "InfoWin pane text stride");
static_assert(sizeof(InfoPaneRows) == 0x30, "InfoWin pane row-value stride");

class InfoWin {
  // MainInterface status controls borrow this pane's rectangles and timer.
  friend class MainInterface;
 public:
  // homed from 00459420.cpp. EVIDENCE-FREE as a name - see the marker in
  // infowin.cpp: no caller, no vtable slot, no CSV row, and the body reads
  // none of its five arguments, so they stay unnamed here too.
  void UNK5(int, int, int, int, int);

 public:
  // 0x00459150, timed pane reveal/advance callback; implemented in infowin.cpp.
  void timer_proc(int a1);
  // The timer's C callback has no receiver. The historical Q-prefixed label
  // does not override the stack-only callback ABI seen at 0x00459260.
  static void __cdecl timer_daemon(int tick);

 public:
  // 0x00458930  ?on_left_click@InfoWin@@QAEXHH@Z - public, __thiscall,
  // void(int, int). MainInterface's two interface click handlers dispatch
  // here. The coordinates are unused: the first unfinished pane is revealed,
  // or the next pane state is selected when both are already complete.
  void on_left_click(int xCoord, int yCoord);

  void on_right_click(int a, int b);
  // 0x004589C0: menu rows are chosen from the two-pane state table, and
  // the selected action either switches a pane or opens help. Kept
  // out-of-line after on_right_click to preserve its direct call.
  void right_menu(int a1, int a2);
  InfoWin();
  ~InfoWin();
  void UNK1();
  void on_redraw();
  const RECT &bounds() const { return bounds_; }
  void set_redraw_phase(int phase) { redraw_phase_ = phase; }
  // control_turn suppresses this owner's pane timer across the faction-switch
  // display transition; timer_proc reads the same +0xAC member.
  void set_timer_suppression(int suppressed) { field_AC_ = suppressed; }
  // Console end-turn clears only the pending timer at InfoWin +0xA8;
  // reset() additionally advances the pane through timer_proc(1).
  void clear_pending_timer() { field_A8_ = 0; }
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
  void UNK2();                                        // 0x004588B0
  // The catalog's UNK4 at 0x00458D60 only configures the global panel RECT
  // and sets its initialization bit. No caller or vtable slot names it;
  // `configure_panel_bounds` is a behavior-based source name.
  void configure_panel_bounds();                       // 0x00458D60
  int state_valid(int state);                         // 0x00458DC0
  void set_state(int a1, int a2);                     // 0x00458F10
  void timer_change(int a1);                          // 0x00458FF0
  void init();                                        // 0x004592A0

 private:
  // Owned subobjects are constructed in declaration order by 0x00459500
  // and destroyed in reverse order by 0x004594A0. Gaps remain unnamed;
  // these offsets do not by themselves prove the complete original extent.
  // init (0x004592A0) writes four coordinates at +0x10 and +0x20.
  // At 0x004593F7, USER32!UnionRect receives those two members and
  // this as its destination: the leading rectangle is their union.
  // The panes coincide at width 800; otherwise the second is to the right.
  // Their displayed-content roles remain unestablished.
  // RECOVERY_MEMBER: v2 InfoWin bounds_ tagRECT 0x0 0x004592A0
  RECT bounds_;
  // draw_text (0x00458420) selects indices [0,2) for both panes, or
  // one requested index. At 0x00458465-0x00458472 it computes
  // this+(index+1)*16 before passing the rectangle to Buffer::set_clip.
  // Together with init's two destinations, this establishes a pair of
  // 16-byte elements at +0x10, not independently addressed loose fields.
  // RECOVERY_MEMBER: v4 InfoWin pane_rects_ tagRECT[2] 0x10 0x20 0x00458420
  RECT pane_rects_[2];
  // RECOVERY_MEMBER: v2 InfoWin timer_ Time 0x30 0x00459500
  Time timer_;
  // RECOVERY_MEMBER: v2 InfoWin font_ Font 0x58 0x00459500
  Font font_;
  int active_pane_;  // 0x80
  uint32_t field_84_;  // 0x84
  int pane_states_[2];  // 0x88, timer_change cycles these modulo eight
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
  // setup_text initializes two groups of twelve rows (0x00456357),
  // using 80-byte text slots and parallel four-byte coordinate/color slots.
  // draw_text consumes those same indexed groups (0x00458462-0x00458513).
  InfoPaneText row_text_[2];     // 0xB8
  InfoPaneRows row_x_[2];        // 0x838
  InfoPaneRows row_y_[2];        // 0x898
  InfoPaneRows row_color_[2];    // 0x8F8
  InfoPaneRows row_reveal_color_[2];  // 0x958
  int row_count_[2];         // 0x9B8
  // setup_text zeros the selected total at 0x004582CB and accumulates
  // strlen results at 0x00458338-0x0045834A. on_left_click walks two
  // four-byte entries at +0x9C0/+0x9C8 (0x00458952-0x0045896B), using
  // a signed comparison. draw_text loads the selected budget at
  // 0x004584B3 and tests it signed at 0x004584DA/0x004584DC.
  // The offset-only annotations do not claim verified primitive-array type
  // or count; the indexed original accesses establish the source model.
  // RECOVERY_MEMBER: v1 InfoWin text_lengths_ 0x9C0 0x00456300
  int text_lengths_[2];
  // RECOVERY_MEMBER: v1 InfoWin visible_characters_ 0x9C8 0x00458930
  int visible_characters_[2];
  // RECOVERY_MEMBER: v2 InfoWin button_ PushButton 0x9D0 0x00459500
  PushButton button_;
};


// Actual palette-index owners shared by information and vehicle overlays.
extern int InfoFactionTextColors[8];
extern int InfoFactionShadowColors[8];
