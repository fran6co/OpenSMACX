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
extern int MainInterfaceTileFlag;
extern int MainInterfaceStatusRedrawFlag;
extern int MainInterfaceRedrawCount;
extern int MainInterfaceTimersStopped;
extern int MainInterfaceEndTurnAnimation;
// Shared palette indices, one foreground/shadow pair per faction.
extern int ChatItemIdTable[8];
extern int ChatShadowColors[8];
#include "buttongroup.h"
#include "flatbutton.h"
#include "flic.h"
#include "graphicwin.h"
#include "stringbox.h"
#include "subinterface.h"
#include "time.h"

/*
 * THE RECEIVER MAP
 * ================
 *
 * MainInterface is ONE object with TWO receivers, and which one a method
 * gets decides every offset in its body. Measured out of the image, not
 * inferred:
 *
 *   The OUTER receiver is the window front. MainInterface's constructor
 *   (0x0045EF20, `mov esi, ecx` at entry) builds flat_button_[42] at
 *   esi+0xE00, button_group_[4] at esi+0x1E878, string_box_ at esi+0x1EACC,
 *   the three fonts at esi+0x2166C/94/BC, the sprite arrays from
 *   esi+0x216F4, and closes with three vftable stores - esi+0x0 (0x66C598),
 *   esi+0x444 (0x66C590) and esi+0xA14 (0x66C544). Every field in the class
 *   below is named from that front, and every BYTE_EXACT body in
 *   maininterface.cpp uses it: redraw_complete (0x0045C3A0) reaches spot_
 *   with `lea ecx, [esi + 0x25034]`, which is exactly where the member map
 *   puts Spot.
 *
 *   The INNER receiver is the SubInterface subobject at front+0xA14, which
 *   is the placement established by this object's constructor and the
 *   type the 12-slot registry at 0x007AE7F0 holds. Its vfptr and state word
 *   occupy the former field_A14_ and field_A18_ stand-ins: the real base
 *   is eight bytes, so subsequent members retain their front offsets.
 *   redraw_complete proves the overlap from the other side - it compares
 *   `dword ptr [0x7AF238]` against 0xA, and 0x7AF238 is front+0xA18, the
 *   SubInterface dword the interface-mode methods read and write.
 *
 *   The on_iface_* family takes the INNER receiver. on_iface_mouse_leave
 *   (0x0045D110) is `mov esi, ecx` then `cmp [esi+0x20CD8], 1` then
 *   `lea ecx, [esi-0xA14]` before calling redraw_complete: esi is the
 *   subobject, esi+0x20CD8 is front+0x216EC (iface_redraw_pending_), and
 *   esi-0xA14 is the front. The MainInterface secondary vftable at 0x66C544
 *   holds the family's RAW addresses and there is no `sub ecx, 0xA14`
 *   adjustor thunk anywhere in .text.
 *
 *   RESOLVED 2026-09-05, and the reason is worth keeping: VC6 compiles an
 *   override of a SECONDARY base's virtual with the BASE SUBOBJECT as `this`.
 *   The body reads its own members base-relative, the secondary vftable holds
 *   the override's raw address, and no adjustor thunk is emitted - probed
 *   against cl 12.00.8168 under /O2 /Gy /GR- /GX. MainInterface therefore
 *   derives directly from SubInterface, the real nineteen-slot base declared
 *   below; every offset in the class is unchanged: 0xA14 + 8 == 0xA1C, exactly
 *   where field_A14_ and field_A18_ used to sit. on_iface_mouse_leave is
 *   BYTE_EXACT homed on it, `iface_redraw_pending_` compiling to
 *   `[esi+0x20CD8]` and `redraw_complete()` to `lea ecx,[esi-0xA14]`.
 *   CouncWin's table at 0x66AA94 holds the same kind of entry (slot 3 is
 *   ?on_iface_left_click@CouncWin@@QAEXHH@Z at 0x00427060, a bare `ret 8`),
 *   and slots 6, 17 and 18 are the same three shared no-ops (0x404260,
 *   0x404250, 0x404240) in every window's table - so this is the pattern for
 *   the other seven windows, not a MainInterface special case.
 *
 * WHAT IS DECLARED HERE AND WHY. SubInterface below carries the nineteen
 * pure slots, so registry dispatch - the image's `mov edx,[ecx] / call dword
 * ptr [edx+0x34]` - is a call by name and the VCall shims it stood in for are
 * gone. The slots MainInterface implements are pure, which is what the image's
 * own base table (0x66A6E4, nineteen consecutive __purecall slots) says they
 * are. Direct calls to a virtual callback use explicit C++ qualification;
 * virtuality must not be suppressed to protect a caller's spelling.
 */
// The 12 interface-mode slots at 0x007AE7F0, each either null or a live
// SubInterface subobject; delete_iface_mode (0x0045D2E0) walks the table
// zeroing the slot that matches its own `this`. The registry, the id-to-slot
// map at 0x007AE778 and active id at 0x007D392C have source-owned storage
// in maininterface.cpp. The registry is a typed run of real SubInterface
// subobjects; no extra registry-entry wrapper owns or adjusts its callbacks.
extern int IfaceIdToSlotStorage[30];  // 0x007AE778, before registry at 0x007AE7F0
extern int ActiveIfaceIdStorage;      // 0x007D392C
extern SubInterface *InterfaceRegistry[12];
void register_iface_mode(int slot, SubInterface *interface_entry);

 /*
  * MainInterface class
  */
// RECOVERY_BASE: v1 MainInterface SubInterface 0xA14 0x0045EF10
// Constructor installs this original interface table at the declared base.
// Its nineteen observed slots do not independently establish its end bound.
// RECOVERY_GENERATED: v1 vtable 0x0066C544 ? ??_7MainInterface@@6B@ 0x0045EF10
class MainInterface : public GraphicWin, public SubInterface {
  friend class AlphaMenu;  // AlphaMenu uses the owned menu-window rectangle and state.
  friend class TutWin;  // Tutorial callouts read the owned panel rectangles.
  friend class DesignWin;  // DesignWin init reads existing panel rectangles.
  friend class CouncWin;  // Council init reads the existing panel height metric.
  friend class BaseWin;  // draw_garrison uses the adjacent panel rectangles.
  friend class WorldWin;  // UNK1 copies the owned tutorial rectangle.
  // MessageWin::init copies the pane bounds at +0xD1C..+0xD28 into its
  // own RECT. This grants that owner typed access while this region awaits
  // a named MainInterface rectangle member.
  friend class MessageWin;
 public:
  // The shared UI font at global 0x007CFE8C is this object's font1_ at
  // +0x2166C. ReplayWin's title and other windows borrow it, not own it.
  Font *primary_font() { return &font1_; }
  Font *secondary_font() { return &font2_; }
  FlatButton &flat_button(int index) { return flat_button_[index]; }
  const RECT &main_frame_rect() const { return rect4_; }
  const RECT &main_layout_rect() const { return rect1_; }
  const RECT &bottom_bounds() const { return bottom_bounds_; }
  Sprite &slide_frame_sprite(int index) { return sprites2_[index]; }
  // Original sprites1_ starts at 0x7CFF14; Sprite height is +0x1C.
  // Popup 0x406260 reads elements 2/0/1, not three standalone globals.
  Sprite &scroll_frame_sprite(int index) { return sprites1_[index]; }
  Sprite &interface_down_sprite() { return sprites13_[1]; }
  // slide_hide 0x45DDF3 draws the existing iface_up Sprite at 0x7D1750.
  // MainInterface::init loads this element beside iface_down, not a new object.
  Sprite &interface_up_sprite() { return sprites13_[0]; }
  const RECT &rect2() const { return rect2_; }
  const RECT &rect3() const { return rect3_; }
  const RECT &info_frame_rect() const { return info_panel_frame_; }
  const RECT &right_frame_rect() const { return right_frame_rect_; }
  const RECT &lower_frame_rect() const { return lower_frame_rect_; }
  // Console screen geometry reads the original panel height at +0xD7C.
  int panel_height() const { return static_cast<int>(field_D7C_); }
  // homed from 00459680.cpp
  int init(int a1);

 public:

  // homed from 0045c600.cpp
  void on_scrolled(int a1, int a2);

 public:
  // homed from 0045d2a0.cpp
  void set_alert(int a1);

 public:
  // homed from 0045ca00.cpp
  void on_selected(int a1, int a2);

 public:
  // homed from 0045c9c0.cpp
  void on_mouse_leave(int a1, int a2);

 public:
  // homed from 0045c940.cpp
  void on_group_clicked(int a1, int a2, int a3);

 public:
  // homed from 0045c980.cpp
  void on_mouse_move(int a1, int a2);

 public:
  // homed from 0045c8f0.cpp
  int on_dialog_item_back_draw(GraphicWin* a1, int a2, int a3, RECT* a4);

 public:
  // homed from 0045c8b0.cpp
  int on_dialog_back_draw(GraphicWin* a1);

 public:
  // homed from 0045c870.cpp
  void on_button_toggled(int a1, int a2);

 public:
  // homed from 0045c830.cpp
  int on_button_pressed(int a1);

 public:
  // homed from 0045c7c0.cpp
  void on_right_down(int a1, int a2);

 public:
  // homed from 0045c780.cpp
  void on_left_down(int a1, int a2);

 public:
  // homed from 0045c740.cpp
  void on_right_click(int a1, int a2);

 public:
  // homed from 0045c700.cpp
  void on_left_click(int a1, int a2);

 public:
  // homed from 0045c6c0.cpp
  void on_left_up(int a1, int a2);

 public:
  // homed from 0045c680.cpp
  void on_right_double_click(int a1, int a2);

 public:
  // homed from 0045c640.cpp
  void on_left_double_click(int a1, int a2);

 public:
  // homed from 0045c5c0.cpp
  void on_scrolling(int a1, int a2);

 public:
  // homed from 0045c290.cpp
  void restore_back(int a1);

 public:
  // 0x0045C800. Looks the active interface up in the registry and hands the
  // button id to its slot 13. The window event handlers dispatch the same way
  // - on_left_click, on_button_pressed and friends all read ActiveIfaceId,
  // then IfaceIdToSlot, then IfaceRegistry.
  void on_button_clicked(int a1);

 public:
  // 0x0045C180. Time's tick dispatcher: case 0 stops time1_, case 1 steps
  // the alert counter and unions rect1_ with rect2_.
  void timer_callback_daemon(int a1);

 public:
  // 0x0045BE80: redraw date and available energy for the current interface.
  void set_date(char *text);
  MainInterface();
  // Ordinary C++ teardown releases owned members in reverse declaration order.
  ~MainInterface();

  int set_bubble_text(uint32_t button_id, LPCSTR text);
  void clear_message();
  // 0x0045B550. The editor-mode button pane: Console::editor_paints,
  // editor_lock2, editor_set_brush and set_view all redraw it after turning
  // an editor flag.
  void draw_edit_pane();
  virtual void on_redraw();
  // 0x0045C3A0. blink2_timer (time.cpp) reaches
  // it on the fixed MainInterface at 0x007AE820 as its whole tick body.
  void redraw_complete();
  void UNK2();
  void UNK3();
  void UNK4();

  void UNK1(int pos);
  // (code, pos), the Win32 WM_VSCROLL shape the image mirrors. Evidence
  // is in the bodies: BaseWin::on_scrolled does `switch (a1)` and then
  // stores a2 in field_40B10_; StringBox::on_scrolled stores a2 in
  // field_A1C_; BaseWin::on_iface_scrolled tests `a1 != 2` and copies a2.
  // The first is switched on, the second is kept - a code and a position.
  void on_iface_scrolled(int code, int pos);
  void on_iface_scrolling(int a1, int a2);
  void on_iface_left_up(int, int) override;
  void on_iface_button_toggled(int a1, int a2);
  void on_iface_right_double_click(int a1, int a2);
  void on_iface_selected(int, int) override;
  void on_iface_left_double_click(int, int) override;
  int on_iface_dialog_back_draw(GraphicWin *) override;
  int on_iface_dialog_item_back_draw(GraphicWin *, int, int,
                                    RECT *) override;
  // 0x0045CA40, secondary slot0: show/hide owned interface controls.
  void on_status(int a1);
  int on_iface_button_pressed(int a1);
  // 0x0045D170: genuine slot13 override, also reached by qualified calls.
  void on_iface_button_clicked(int a1);
  // The rest of the interface table - slots 3, 4, 5, 7, 8, 9 and 16. All
  // seven take the ENTRY as their receiver, and all seven reach the front's
  // members through the -0xA14 the compiler emits on its own now that the
  // entry is a declared base: spot_ is entry+0x24620, iface_redraw_pending_
  // entry+0x20CD8, and redraw_complete is called on entry-0xA14.
  void on_iface_left_click(int a1, int a2);       // 0x0045CDC0, slot 3
  void on_iface_right_click(int a1, int a2);      // 0x0045CE80, slot 4
  void on_iface_left_down(int a1, int a2);        // 0x0045CF40, slot 5
  void on_iface_right_down(int a1, int a2);       // 0x0045CF80, slot 7
  void on_iface_mouse_move(int a1, int a2);       // 0x0045D000, slot 8
  void on_iface_mouse_leave(int a1, int a2);      // 0x0045D110, slot 9
  void on_iface_group_clicked(int a1, int a2, int a3);  // 0x0045CCB0, slot 16
  // 0x0045C0F0 ?set_message@MainInterface@@QAEXPADHHHH@Z. The three ints
  // after the text are read by no instruction in the body - only the char *
  // reaches string_box_ and the desktop's MessageWin.
  void set_message(char *text, int, int, int, int);
  // 0x0045C0B0 ?chat@MainInterface@@QAEXPADH@Z. The second int selects an
  // item id out of ChatItemIdTable (maininterface.cpp); the text goes to the
  // message pane and to the desktop's MessageWin.
  void chat(char *text, int channel);

  const RECT &stack_button_rect() const { return stack_button_rect_; }
  const RECT &adjacent_panel_right_rect() const { return adjacent_panel_right_rect_; }
  const RECT &status_hit_rect() const { return status_hit_rect_; }
  const RECT &status_left_rect() const { return status_left_rect_; }
  const RECT &status_right_rect() const { return status_right_rect_; }
  const RECT &info_panel_frame() const { return info_panel_frame_; }
  ButtonGroup &button_group(int index) { return button_group_[index]; }

 private:
  // 0xA14 is the real SubInterface base - its vptr and one dword occupy
  // exactly the eight bytes field_A14_ and field_A18_ used to stand in for.
  // MEASURED 2026-09-05, and the earlier revert's second reason was
  // WRONG. Probing cl 12.00.8168 (/O2 /Gy /GR- /GX) with a GraphicWin-sized
  // first base and a secondary base carrying the nineteen slots: a derived
  // override of a SECONDARY base's virtual is compiled with the BASE
  // SUBOBJECT as `this` - the body reads its own members base-relative, the
  // secondary vftable holds the override's RAW address, and no adjustor thunk
  // is emitted anywhere. That is exactly the image: on_iface_mouse_leave
  // (0x0045D110) is `cmp dword ptr [esi+0x20cd8],1` with esi the entry, the
  // table at 0x66C544 holds the raw bodies, and .text has no
  // `sub ecx,0xA14` thunk. So declaring the base is what makes the family
  // spellable: `iface_redraw_pending_` compiles to [esi+0x20cd8] and an
  // unqualified `redraw_complete()` to `lea ecx,[esi-0xa14]`, both of which
  // the earlier field_A14_/field_A18_ spelling could not produce at any
  // offset. Every offset after 0xA1C is unchanged, 0xA14 + 8 == 0xA1C.
  RECT window_bounds_;  // A1C, init passes the four coordinates to GraphicWin
  RECT scroll_up_rect_;
  RECT scroll_down_rect_;
  RECT editor_tab_rect_;
  RECT menu_button_rect_;
  RECT multiplayer_button_rect_;
  RECT end_turn_rect_;
  RECT rect2_;
  RECT rect3_;
  RECT stack_button_rect_;
  // 0x0043C89C copies the second four-word run as one RECT. The garrison
  // overflow path at 0x004105A7 reads the gap from the first run's right
  // edge (+0xAC4) to the second run's left edge (+0xACC).
  // RECOVERY_MEMBER: v1 MainInterface adjacent_panel_left_rect_ 0xABC 0x004105A7
  RECT adjacent_panel_left_rect_;
  // RECOVERY_MEMBER: v1 MainInterface adjacent_panel_right_rect_ 0xACC 0x0043C89C
  RECT adjacent_panel_right_rect_;
  RECT editor_button_rects_[25];  // ADC..C6B, 25 rectangles written by init
  RECT brush_button_rects_[6];    // C6C..CCB, six rectangles written by init
  // init (0x00459680) retains the outer receiver in esi, copies four
  // coordinates at +0xCCC/+0xCDC/+0xCEC and insets their edges. Its split
  // puts +0xCCC to the right of +0xCDC; the separate +0xCEC rectangle is
  // the double-click hit region (0x0045CD8B). These are owned rectangles,
  // not independent globals at MainInterfaceGlobal + those offsets.
  // RECOVERY_MEMBER: v2 MainInterface status_right_rect_ tagRECT 0xCCC 0x00459680
  RECT status_right_rect_;
  // RECOVERY_MEMBER: v2 MainInterface status_left_rect_ tagRECT 0xCDC 0x00459680
  RECT status_left_rect_;
  // RECOVERY_MEMBER: v2 MainInterface status_hit_rect_ tagRECT 0xCEC 0x00459680
  RECT status_hit_rect_;
  // Global 0x007AF51C is this member of MainInterfaceGlobal. Tutorial
  // drawing consumes all four coordinates as a RECT; WorldWin copies it
  // to the active map bounds.
  // RECOVERY_MEMBER: v2 MainInterface tutorial_bounds_ tagRECT 0xCFC 0x004C3A70
  RECT tutorial_bounds_;
  RECT date_rect_;
  RECT message_bounds_;
  RECT title_bounds_;
  RECT bottom_bounds_;
  RECT editor_panel_rect_;
  RECT editor_tools_rect_;
  RECT editor_text_rect_;
  uint32_t field_D7C_;
  RECT rect4_;  // 0xD80, the IDB names the 16 bytes at 0xD80 rect4
  RECT right_frame_rect_;
  // InfoWin::init reads all four coordinates at MainInterfaceGlobal+0xDA0;
  // SocialWin copies the same original interval as one panel rectangle.
  // RECOVERY_MEMBER: v2 MainInterface info_panel_frame_ tagRECT 0xDA0 0x004592A0
  RECT info_panel_frame_;
  RECT lower_frame_rect_;
  RECT rect1_;
  RECT lower_animation_bounds_;
  RECT menu_animation_bounds_;
  RECT multiplayer_animation_bounds_;
 public:
  // PUBLIC 2026-09-06, the same compile-time-only move spot_ needed below:
  // the four buttons the interface shows and hides when a battle goes up and
  // comes down (BattleWin::on_status 0x00422CC0 drives flat_button_[3],
  // [4], [6] and [7] through Win's own show(int)/hide() slots) are reached as
  // `mov ecx, <address>` immediates, and only a direct member path folds to
  // one - a `FlatButton *const` binding costs the load of the binding first,
  // the indirection the BaseCurrent() note in base.h records. Widening access
  // moves no offset and no byte.
  FlatButton flat_button_[42];
 private:
  // The constructor at 0x0045EF10 builds four 0x94-byte groups here.
  // WorldWin::set_detail_map reaches element 1 at global+0x1E878+0x94.
  // RECOVERY_MEMBER: v4 MainInterface button_group_ ButtonGroup[4] 0x1E878 0x250 0x004C3950
  ButtonGroup button_group_[4];
  uint32_t field_1EAC8_;
  StringBox string_box_;
  Font font1_;
  Font font2_;
  Font font3_;
  uint32_t field_216E4_;
  uint32_t field_216E8_;
  // 0x216EC. on_iface_mouse_leave tests it against 1, clears it and redraws
  // when it is set, through the SubInterface subobject, where the same dword
  // is subobject+0x20CD8. It lived beside redraw_complete in the public
  // section until 2026-09-05: there it landed at 0xA14 - the SubInterface
  // vftable slot - and pushed every member from field_A14_ through
  // string_box_ four bytes past the image's own offsets, which is what
  // MainInterface::chat was measuring as `add ecx, 0x1ead0` against the
  // image's `add ecx, 0x1eacc`. Measured by probing the compiled object:
  // flat_button_ 0xE04, button_group_ 0x1E87C, string_box_ 0x1EAD0,
  // font1_ 0x21670 - all four the image's offset plus four, and all four
  // back on the documented values once the dword returns here.
  uint32_t iface_redraw_pending_;  // 0x216EC
  uint32_t alert_;
  Sprite sprites1_[4];
  Sprite sprites2_[4];
  Sprite sprites3_[2];
  Sprite sprites4_[2];
  Sprite sprites5_[10];
  Sprite sprites6_[10];
  Sprite sprites7_[6];
  Sprite sprites8_[3];
  Sprite sprites9_[75];
  Sprite sprites10_[18];
  Sprite sprite11_;
  Sprite sprites12_[6];
  Sprite sprites13_[2];
  Flic flics1_[2];
  Flic flic2_;

 public:
  Sprite *diplomacy_frame_sprite() { return &sprite11_; }
  // PUBLIC 2026-09-06: BattleWin::on_iface_left_down (0x00422E30) reaches this
  // Spot directly - `mov ecx, 0x7d3854`, which is front+0x25034, spot_'s own
  // address - and an accessor would cost that body a call the image does not
  // make. Compile-time only; no offset and no byte moves.
  Spot spot_;
 private:
  Time time1_;
  Time time2_;
};

#include <stddef.h>


void __cdecl desktop_update();

extern MainInterface MainInterfaceGlobal;

// The active interface's registry slot is owned by the interface dispatcher.
// Other windows ask this owner rather than duplicating its provisional
// fixed-address bindings.
bool __cdecl active_interface_slot_occupied();
