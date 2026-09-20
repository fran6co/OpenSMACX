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
 *   is where subinterface.h puts it for the eight window classes and which
 *   is the type the 12-slot registry at 0x007AE7F0 holds. It OVERLAPS the
 *   two dwords field_A14_ and field_A18_ that stand in the class below:
 *   SubInterface is vptr plus one dword, exactly eight bytes, so declaring
 *   the base instead of the two members would move nothing after them.
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
 *   derives from IfaceRegistryEntry (below), the family overrides its slots,
 *   and every offset in the class is unchanged: 0xA14 + 8 == 0xA1C, exactly
 *   where field_A14_ and field_A18_ used to sit. on_iface_mouse_leave is
 *   BYTE_EXACT homed on it, `iface_redraw_pending_` compiling to
 *   `[esi+0x20CD8]` and `redraw_complete()` to `lea ecx,[esi-0xA14]`.
 *   CouncWin's table at 0x66AA94 holds the same kind of entry (slot 3 is
 *   ?on_iface_left_click@CouncWin@@QAEXHH@Z at 0x00427060, a bare `ret 8`),
 *   and slots 6, 17 and 18 are the same three shared no-ops (0x404260,
 *   0x404250, 0x404240) in every window's table - so this is the pattern for
 *   the other seven windows, not a MainInterface special case.
 *
 * WHAT IS DECLARED HERE AND WHY. IfaceRegistryEntry below carries the nineteen
 * slots, so the registry dispatch - the image's `mov edx,[ecx] / call dword
 * ptr [edx+0x34]` - is a call by name and the VCall shims it stood in for are
 * gone. The slots MainInterface implements are pure, which is what the image's
 * own base table (0x66A6E4, nineteen consecutive __purecall slots) says they
 * are; slot 13 stays a defined vslot because the image reaches
 * on_iface_button_clicked DIRECTLY from BattleWin and an override would turn
 * that rel32 into a dispatch.
 */
class IfaceRegistryEntry : public SubInterface {
 public:
  // Slots 1..18 of the interface table. Slot 0 is SubInterface's. The names
  // are the MainInterface methods the table at 0x66C544 points at, slot for
  // slot. The bodies that ARE defined here are the shared no-ops every
  // window's table carries at those slots (0x404260, 0x404250, 0x404240) - a
  // window that does not implement the event gets exactly these - plus
  // on_iface_selected and on_iface_left_double_click, whose MainInterface
  // bodies touch no member and are byte-exact right here.
  //
  // The rest are PURE, and that is not a convenience: the image's own base
  // table (0x66A6E4, the one ten constructors install) is nineteen consecutive
  // __purecall slots, which is what an abstract interface's vftable is. It
  // also keeps the linker honest - a window that forgets to implement one of
  // these does not link, rather than silently inheriting a no-op - and it
  // keeps VC6's key-function rule from pinning this vtable to a TU that never
  // defines its key.
  virtual void on_status(int mode) = 0;          // slot 0  0x45CA40
  virtual void on_iface_scrolled(int, int) = 0;      // slot 1  0x45CC80
  virtual void on_iface_scrolling(int, int) = 0;     // slot 2  0x45CC90
  virtual void on_iface_left_click(int, int) = 0;    // slot 3  0x45CDC0
  virtual void on_iface_right_click(int, int) = 0;   // slot 4  0x45CE80
  virtual void on_iface_left_down(int, int) = 0;     // slot 5  0x45CF40
  // Slot 6 carries a NAME, not a vslot_N placeholder, and BaseWin is why.
  // Its own table (0x66A3B8) holds ?on_iface_left_up@BaseWin@@QAEXHH@Z at
  // 0x18 - slot 6 - where every other window's table holds the shared `ret 8`
  // this definition spells. Slot 5 is on_iface_left_down, so the pair is the
  // interface's left-button press/release, and the name is the image's own.
  // Renamed from `vslot_06` 2026-09-06: no derived class had ever overridden
  // the slot (MainInterface, CouncWin, BattleWin, DesignWin and DiploPop all
  // leave 0x404260 there), so the rename moves no body and gives BaseWin's
  // override its real name instead of a second spelling of it.
  virtual void on_iface_left_up(int, int) {}     // slot 6  0x404260, shared ret 8
  virtual void on_iface_right_down(int, int) = 0;    // slot 7  0x45CF80
  virtual void on_iface_mouse_move(int, int) = 0;    // slot 8  0x45D000
  virtual void on_iface_mouse_leave(int, int) = 0;   // slot 9  0x45D110
  virtual void on_iface_selected(int, int);      // slot 10 0x45D140
  virtual void on_iface_left_double_click(int, int);   // slot 11 0x45CD80
  virtual void on_iface_right_double_click(int, int) = 0;  // 12 0x45CDB0
  // Slot 13 (0x45D170) is deliberately NOT named on_iface_button_clicked.
  // MainInterface does implement that method - it is declared below, and
  // pending_bodies.cpp forwards it to 0x45D170 - but the image reaches it
  // DIRECTLY, never through this slot: BattleWin::on_iface_button_clicked
  // (0x00422DC0) ends in `call 0x45d170`, a rel32, not a dispatch. Naming the
  // slot on_iface_button_clicked makes MainInterface's version an override,
  // which turns that rel32 into `mov eax,[0x7afc48] / call [eax+..]` and
  // costs BattleWin its BYTE_EXACT. So the slot keeps the same vslot_N
  // spelling slot 6 has: present in the table, no MainInterface override, and
  // the real body reachable the way the image reaches it.
  virtual void vslot_13(int button_id) {}  // slot 13 0x45D170, 0x34
  virtual int on_iface_button_pressed(int button_id) = 0;  // 14 0x45D160
  virtual void on_iface_button_toggled(int, int) = 0;      // 15 0x45CCA0
  virtual void on_iface_group_clicked(int, int, int) = 0;  // 16 0x45CCB0
  virtual int on_dialog_back_draw(GraphicWin *a1) { return 0; }  // slot 17 0x404250
  virtual int on_dialog_item_back_draw(GraphicWin *, int, int, RECT *) { return 0; }  // 18
};

// The 12 interface-mode slots at 0x007AE7F0, each either null or a live
// SubInterface subobject; delete_iface_mode (0x0045D2E0) walks the table
// zeroing the slot that matches its own `this`. The registry, the id-to-slot
// map at 0x007AE778 and the active id at 0x007D392C are bound in
// maininterface.cpp - as IfaceRegistry, IfaceIdToSlot and ActiveIfaceId -
// because this header is included by every TU that names a MainInterface and
// a second name per address is a duplicate the gate counts.

 /*
  * MainInterface class
  */
class MainInterface : public GraphicWin, public IfaceRegistryEntry {
 public:
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
  // 0x0045BE80, a pending_bodies forwarder.
  void set_date(char *text);
  MainInterface() { ; }
  // 0x0045EB80 is not recovered: a
  // pending_bodies forwarder, because an empty inline stub emits
  // nothing and the deleting destructor needs a `call rel32`.
  ~MainInterface();

  int set_bubble_text(uint32_t button_id, LPCSTR text);
  void clear_message();
  // 0x0045B550. The editor-mode button pane: Console::editor_paints,
  // editor_lock2, editor_set_brush and set_view all redraw it after turning
  // an editor flag. Body still unrecovered - a pending_bodies forwarder.
  void draw_edit_pane();
  // 0x0045C3A0, a pending_bodies forwarder. blink2_timer (time.cpp) reaches
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
  void on_iface_button_toggled(int a1, int a2);
  void on_iface_right_double_click(int a1, int a2);
  // on_iface_left_double_click (0x0045CD80) and on_iface_selected
  // (0x0045D140) are NOT declared here: both bodies touch no member, so they
  // are byte-exact as IfaceRegistryEntry's own definitions further up this
  // header, and declaring them here would only move them without changing a
  // byte. Every other slot MainInterface implements IS declared here, because
  // an override is what makes the entry's slot link.
  // 0x0045CA40, slot 0 - 568 bytes, still the image's own body; the
  // definition in maininterface.cpp is a forwarder.
  void on_status(int a1);
  int on_iface_button_pressed(int a1);
  // ?on_iface_button_clicked@MainInterface@@QAEXH@Z (0x0045D170), unrecovered.
  // BattleWin::on_iface_button_clicked forwards to it on the fixed-address
  // MainInterface singleton at 0x007AF234. NOT an override of the entry's
  // slot 13 - see vslot_13 above: the image calls it directly, and the direct
  // call is what keeps BattleWin's own body byte-exact.
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

  const RECT &status_hit_rect() const { return status_hit_rect_; }
  const RECT &status_left_rect() const { return status_left_rect_; }
  const RECT &status_right_rect() const { return status_right_rect_; }
  ButtonGroup &button_group(int index) { return button_group_[index]; }

 private:
  // 0xA14 is the IfaceRegistryEntry base - its SubInterface vptr and its one
  // dword, exactly the eight bytes field_A14_ and field_A18_ used to stand
  // in for. MEASURED 2026-09-05, and the earlier revert's second reason was
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
  uint32_t field_A1C_;
  uint32_t field_A20_;
  uint32_t field_A24_;
  uint32_t field_A28_;
  uint32_t field_A2C_;
  uint32_t field_A30_;
  uint32_t field_A34_;
  uint32_t field_A38_;
  uint32_t field_A3C_;
  uint32_t field_A40_;
  uint32_t field_A44_;
  uint32_t field_A48_;
  uint32_t field_A4C_;
  uint32_t field_A50_;
  uint32_t field_A54_;
  uint32_t field_A58_;
  uint32_t field_A5C_;
  uint32_t field_A60_;
  uint32_t field_A64_;
  uint32_t field_A68_;
  uint32_t field_A6C_;
  uint32_t field_A70_;
  uint32_t field_A74_;
  uint32_t field_A78_;
  uint32_t field_A7C_;
  uint32_t field_A80_;
  uint32_t field_A84_;
  uint32_t field_A88_;
  RECT rect2_;
  RECT rect3_;
  uint32_t field_AAC_;
  uint32_t field_AB0_;
  uint32_t field_AB4_;
  uint32_t field_AB8_;
  uint32_t field_ABC_;
  uint32_t field_AC0_;
  uint32_t field_AC4_;
  uint32_t field_AC8_;
  uint32_t field_ACC_;
  uint32_t field_AD0_;
  uint32_t field_AD4_;
  uint32_t field_AD8_;
  uint32_t field_ADC_;
  uint32_t field_AE0_;
  uint32_t field_AE4_;
  uint32_t field_AE8_;
  uint32_t field_AEC_;
  uint32_t field_AF0_;
  uint32_t field_AF4_;
  uint32_t field_AF8_;
  uint32_t field_AFC_;
  uint32_t field_B00_;
  uint32_t field_B04_;
  uint32_t field_B08_;
  uint32_t field_B0C_;
  uint32_t field_B10_;
  uint32_t field_B14_;
  uint32_t field_B18_;
  uint32_t field_B1C_;
  uint32_t field_B20_;
  uint32_t field_B24_;
  uint32_t field_B28_;
  uint32_t field_B2C_;
  uint32_t field_B30_;
  uint32_t field_B34_;
  uint32_t field_B38_;
  uint32_t field_B3C_;
  uint32_t field_B40_;
  uint32_t field_B44_;
  uint32_t field_B48_;
  uint32_t field_B4C_;
  uint32_t field_B50_;
  uint32_t field_B54_;
  uint32_t field_B58_;
  uint32_t field_B5C_;
  uint32_t field_B60_;
  uint32_t field_B64_;
  uint32_t field_B68_;
  uint32_t field_B6C_;
  uint32_t field_B70_;
  uint32_t field_B74_;
  uint32_t field_B78_;
  uint32_t field_B7C_;
  uint32_t field_B80_;
  uint32_t field_B84_;
  uint32_t field_B88_;
  uint32_t field_B8C_;
  uint32_t field_B90_;
  uint32_t field_B94_;
  uint32_t field_B98_;
  uint32_t field_B9C_;
  uint32_t field_BA0_;
  uint32_t field_BA4_;
  uint32_t field_BA8_;
  uint32_t field_BAC_;
  uint32_t field_BB0_;
  uint32_t field_BB4_;
  uint32_t field_BB8_;
  uint32_t field_BBC_;
  uint32_t field_BC0_;
  uint32_t field_BC4_;
  uint32_t field_BC8_;
  uint32_t field_BCC_;
  uint32_t field_BD0_;
  uint32_t field_BD4_;
  uint32_t field_BD8_;
  uint32_t field_BDC_;
  uint32_t field_BE0_;
  uint32_t field_BE4_;
  uint32_t field_BE8_;
  uint32_t field_BEC_;
  uint32_t field_BF0_;
  uint32_t field_BF4_;
  uint32_t field_BF8_;
  uint32_t field_BFC_;
  uint32_t field_C00_;
  uint32_t field_C04_;
  uint32_t field_C08_;
  uint32_t field_C0C_;
  uint32_t field_C10_;
  uint32_t field_C14_;
  uint32_t field_C18_;
  uint32_t field_C1C_;
  uint32_t field_C20_;
  uint32_t field_C24_;
  uint32_t field_C28_;
  uint32_t field_C2C_;
  uint32_t field_C30_;
  uint32_t field_C34_;
  uint32_t field_C38_;
  uint32_t field_C3C_;
  uint32_t field_C40_;
  uint32_t field_C44_;
  uint32_t field_C48_;
  uint32_t field_C4C_;
  uint32_t field_C50_;
  uint32_t field_C54_;
  uint32_t field_C58_;
  uint32_t field_C5C_;
  uint32_t field_C60_;
  uint32_t field_C64_;
  uint32_t field_C68_;
  uint32_t field_C6C_;
  uint32_t field_C70_;
  uint32_t field_C74_;
  uint32_t field_C78_;
  uint32_t field_C7C_;
  uint32_t field_C80_;
  uint32_t field_C84_;
  uint32_t field_C88_;
  uint32_t field_C8C_;
  uint32_t field_C90_;
  uint32_t field_C94_;
  uint32_t field_C98_;
  uint32_t field_C9C_;
  uint32_t field_CA0_;
  uint32_t field_CA4_;
  uint32_t field_CA8_;
  uint32_t field_CAC_;
  uint32_t field_CB0_;
  uint32_t field_CB4_;
  uint32_t field_CB8_;
  uint32_t field_CBC_;
  uint32_t field_CC0_;
  uint32_t field_CC4_;
  uint32_t field_CC8_;
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
  uint32_t field_CFC_;
  uint32_t field_D00_;
  uint32_t field_D04_;
  uint32_t field_D08_;
  uint32_t field_D0C_;
  uint32_t field_D10_;
  uint32_t field_D14_;
  uint32_t field_D18_;
  uint32_t field_D1C_;
  uint32_t field_D20_;
  uint32_t field_D24_;
  uint32_t field_D28_;
  uint32_t field_D2C_;
  uint32_t field_D30_;
  uint32_t field_D34_;
  uint32_t field_D38_;
  uint32_t field_D3C_;
  uint32_t field_D40_;
  uint32_t field_D44_;
  uint32_t field_D48_;
  uint32_t field_D4C_;
  uint32_t field_D50_;
  uint32_t field_D54_;
  uint32_t field_D58_;
  uint32_t field_D5C_;
  uint32_t field_D60_;
  uint32_t field_D64_;
  uint32_t field_D68_;
  uint32_t field_D6C_;
  uint32_t field_D70_;
  uint32_t field_D74_;
  uint32_t field_D78_;
  uint32_t field_D7C_;
  RECT rect4_;  // 0xD80, the IDB names the 16 bytes at 0xD80 rect4
  uint32_t field_D90_;
  uint32_t field_D94_;
  uint32_t field_D98_;
  uint32_t field_D9C_;
  uint32_t field_DA0_;
  uint32_t field_DA4_;
  uint32_t field_DA8_;
  uint32_t field_DAC_;
  uint32_t field_DB0_;
  uint32_t field_DB4_;
  uint32_t field_DB8_;
  uint32_t field_DBC_;
  RECT rect1_;
  uint32_t field_DD0_;
  uint32_t field_DD4_;
  uint32_t field_DD8_;
  uint32_t field_DDC_;
  uint32_t field_DE0_;
  uint32_t field_DE4_;
  uint32_t field_DE8_;
  uint32_t field_DEC_;
  uint32_t field_DF0_;
  uint32_t field_DF4_;
  uint32_t field_DF8_;
  uint32_t field_DFC_;
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
