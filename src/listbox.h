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
#include "dialog.h"
#include "graphicwin.h"
#include "radiobutton.h"

 /*
  * ListBox class
  *
  * MSVC virtual inheritance: ListBox virtually derives from BOTH GraphicWin and
  * Dialog, each held as a member at the offset the object's own vbtable names
  * (vbtable 0x00670584 = { 0, 0x48, 0xA60 }) rather than written as
  * ": virtual GraphicWin, virtual Dialog". MEASURED under VC6 12.00.8168,
  * which is now the only compiler here: that declaration gives 0xB4C against
  * a real 0xB54, and a virtual function on the DERIVED class gives 0xB50 -
  * that +4 is its own vfptr, which this class does not have (offset 0 holds
  * the vbtable pointer, not a vftable pointer).
  *
  * THE MISSING EIGHT BYTES ARE TWO VTORDISP FIELDS after all. A note here
  * withdrew that reading in favour of "the two BASES' own vtable pointers";
  * the withdrawal was itself wrong and is now withdrawn, because the probe
  * behind it was confounded. It gave each base a virtual destructor WHILE the
  * base still carried its modelled vtable dword, so each base grew by four
  * (0x10 -> 0x14) and the derived total grew by the same eight - the bases
  * getting bigger, not a displacement appearing - and the derived class in
  * that probe overrode nothing, which is the one condition a vtordisp needs.
  * Re-measured on this exact shape, with the virtual function REPLACING the
  * modelled dword so the bases stay 0xA14 and 0xF4:
  *
  *   bases model the vtable as a dword, derived virtually derives  0xB4C
  *   bases declare a real virtual, derived does NOT override it    0xB4C
  *   bases declare a real virtual, derived OVERRIDES it            0xB54
  *
  * and the derived-of-derived shape (Dialogs, src/dialogs.h) moves 0xC90 ->
  * 0xC94 on the same switch. Four bytes per virtual base, present only when
  * the derived class overrides - that is a vtordisp and nothing else. The
  * image says so independently: ??0Dialogs (0x00612938, 0x00612947) and
  * ~Dialogs (0x00406977, 0x00406989) write those very slots at vbase-4 with
  * the vbase displacement, and this class's own 0x44 and 0xA5C dwords are the
  * same slot.
  *
  * So this is blocked on something specific and fixable: GraphicWin and
  * Dialog model their vtable as an opaque dword the original installs by
  * hand, so neither declares a virtual for the compiler to place, and neither
  * can be overridden. Giving them real virtual functions is a COUPLED edit -
  * the compiler then emits the vfptr, so the modelled dword must come out in
  * the same change - and `sizeof(GraphicWin) == 0xA14` is the check that
  * catches it going wrong. MapWin and Console have a single virtual base, no
  * such dword and no override, which is why they convert today. Do not read
  * this as "cannot be done". Note that radiobutton.h, checkbox.h and
  * editgroup.h still carry a copy of the withdrawn paragraph.
  * This mirrors CheckBox (src/checkbox.h), whose
  * vbtable {0,0x1C,0xA34} has the identical shape. close()/destroy() resolve
  * both base subobjects through the runtime vbtable, never through these
  * compile-time offsets, so an embedded ListBox with a different vbtable still
  * reaches the right subobjects (the RadioButton-crash rule in AGENTS.md).
  *
  * The 0x44 and 0xA5C dwords are the GraphicWin/Dialog vbase-adjust slots that
  * sit four bytes below each virtual base; they are 0 in a most-derived ListBox.
  */
// The bases are REAL. `graphic_vbase_adjust_` and `dialog_vbase_adjust_` were
// this header's own names for the two vtordisps, so it already knew what they
// were - the compiler emits the GraphicWin one, because this class overrides
// GraphicWin's on_dialog_focus and on_mouse_leave.
// TRIED: `close()` and `item()` as the Dialog virtual that earns this class's
//   Dialog vtordisp. BOTH ARE REFUTED BY THE IMAGE, on the same test: a body
//   entered on a subobject opens by adjusting `this` back by that subobject's
//   offset, and neither does. ListBox::item (0x0060C920) opens
//   `push ebx / mov esi, ecx`, Dialogs::item (0x00612A70) opens `mov eax, ecx`,
//   ListBox::close and CheckBox::close likewise - all unadjusted receivers.
//
//   THE VTORDISP ITSELF IS NOT IN DOUBT. FileWin's constructor writes
//   `mov dword ptr [esi + 0x33c0], ebx` at 0x006138E4, and its ListBox starts
//   at 0x286C, so this class is 0xB54 - four more than the 0xB50 it measures
//   with only the GraphicWin displacement. Datalink and MessageWin embed one
//   too and agree.
//
//   So the slot is proven and WHICH of ListBox's ~40 methods fills it is not.
//   Only four of them are recovered and just one enters adjusted, on GraphicWin.
//
//   READ THE VTABLE, 2026-08-22, AND THE SLOT IS PROVEN EVEN THOUGH THE
//   DECLARATION STILL CANNOT EXPRESS IT. This class installs its Dialog vtable
//   at 0x00670408; its entries point back into ListBox code through ADJUSTOR
//   THUNKS, and 0x0060D030 opens `sub ecx, dword ptr [ecx - 4]` - which IS the
//   vtordisp adjustment, reading the displacement stored four bytes before the
//   base and subtracting it from `this`. adjustor_thunks.cpp catalogues them
//   as `?on_redraw@thunk3_ListBox@@` and `?attach@thunk1_ListBox@@`, so those
//   are the overridden slots.
//
//   Declaring either `virtual` on Dialog was measured and BREAKS THE SIBLINGS:
//   CheckBox, RadioButton, SpriteBox and EditGroup all declare on_redraw and
//   attach too, so each would gain a Dialog vtordisp the image says it does
//   not have - 62 and 63 failed size assertions respectively. Their on_redraw
//   and attach are GRAPHICWIN's virtuals; ListBox's are in the DIALOG vtable.
//   One flat declaration per name cannot say which base's slot it fills, and
//   that - not the identity of the method - is what still blocks this.
//   `Dialog::item` is declared virtual here because it produces the proven
//   layout, NOT because the image says it is the override - and the price is
//   the two forwarders below.
class ListBox : public virtual GraphicWin, public virtual Dialog {
 public:
  // AlphaSave's show/hide tests the Win state of this embedded list, reached
  // through ListBox's GraphicWin virtual base in the original image.
  bool has_scroll_flags() const { return (iFlags_ & 0xC) != 0; }

  typedef int (__cdecl *DrawCallback)(GraphicWin *, int, int, int, int, int);
  typedef int (__cdecl *WriteCallback)(GraphicWin *, int, int, RECT *, char *);
  typedef int (__cdecl *FillCallback)(GraphicWin *, int, int, RECT *);
  // MessageWin::init installs the three callback slots at ListBox +0x30,
  // +0x34 and +0x38. The image writes function pointers, not integer state.
  void set_callbacks(DrawCallback draw, WriteCallback write,
                     FillCallback fill) {
    draw_callback_ = draw;
    write_callback_ = write;
    fill_callback_ = fill;
  }
  void set_message_initial_state() {
    field_10_ = 0;
    field_14_ = static_cast<uint32_t>(-1);
    field_2C_ = 0;
  }
  // ReportIf 0x004AC140 sets the text palette before inserting rows, then
  // clears selection/row state afterwards. Keep those two effects separate.
  void set_report_text_color(int color) { field_10_ = color; }
  void reset_report_selection() {
    field_14_ = static_cast<uint32_t>(-1);
    field_2C_ = 0;
  }
  // ReportIf's lab color callback does not replace drawing/filling hooks.
  void set_write_callback(WriteCallback callback) { write_callback_ = callback; }
  void set_selection_callback(WinNotificationCallback callback) {
    selection_callback_ = callback;
  }
  // THE ENTERED-RECEIVER WALK. The image's Q-spelled handlers (on_right_down,
  // on_scrolled, on_right_double_click) are entered with `this` on the
  // GraphicWin virtual base and fold the walk back to the ListBox front into
  // every access - `mov eax, [ecx - 0x48]` and receiver
  // `lea ecx, [edx + ecx - 0x48]`. A Q member's source cannot say "this
  // arrives as the base", so the walk is spelled: pass the entered receiver,
  // get the front. The cast lands on the PARAMETER, not on `this`, which
  // keeps the class_debt raw-self-access census at its ceiling for a pun
  // that is measured and load-bearing.
  static ListBox *from_graphic_base(void *entered) {
    return reinterpret_cast<ListBox *>(
        static_cast<uint8_t *>(entered) - 0x48);
  };

 public:
  // 0x0060C6A0, a pending_bodies forwarder.
  // (code, pos), the Win32 WM_VSCROLL shape the image mirrors. Evidence
  // is in the bodies: BaseWin::on_scrolled does `switch (a1)` and then
  // stores a2 in field_40B10_; StringBox::on_scrolled stores a2 in
  // field_A1C_; BaseWin::on_iface_scrolled tests `a1 != 2` and copies a2.
  // The first is switched on, the second is kept - a code and a position.
  void on_scrolled(int code, int pos);

 public:
  // 0x0060AA20, a pending_bodies forwarder.
  void on_right_down(int a, int b);

 public:
  // 0x0060C6D0, a pending_bodies forwarder.
  void on_right_double_click(int a, int b);

 public:
  // 0x0060C350, a pending_bodies forwarder.
  void on_redraw();

 public:
  // 0x0060CBC0, a pending_bodies forwarder.
  void on_mouse_move(int a1, int a2);

 public:
  // 0x0060AA60, a pending_bodies forwarder.
  void on_left_down(int a1, int a2);

 public:
  // 0x0060C710, source-owned typed callback body in listbox.cpp. The retained
  // seed preserves its historical matching obligation during restoration.
  void on_left_double_click(int a1, int a2);

 public:
  // 0x0060AF90, a pending_bodies forwarder.
  void on_key_down(int a);

 public:
  // 0x0060A670, a pending_bodies forwarder.
  int attach(GraphicWin * a1, int a2, int a3, int a4);

 public:
  // 0x0060C5D0, homed in leaf_recoveries.cpp (2026-09-27): the entered-
  // receiver WM_VSCROLL step over the Dialog entry list.
  void on_scrolling(int a, int b);
  // 0x0060CB70, homed in leaf_recoveries.cpp (2026-09-27): the wheel-notch
  // loop over the Win virtual base's two scrollbar clicks.
  void on_mousewheel(int a);
  // 0x0060B530 ?draw_item@ListBox@@QAEXH@Z (3240 bytes), source-owned in
  // leaf_recoveries.cpp. on_scrolling reaches it twice on the front receiver.
  // The original raw-offset transcription survives in Git history at
  // e853f54d^:src/unrecovered/0060b530.cpp for restoration work.
  void draw_item(int index);
  // 0x0060C7D0 ?UNK2@ListBox@@QAEHPAD@Z (336 bytes), source-owned in
  // leaf_recoveries.cpp. Its retired seed is retrievable from Git history.
  int UNK2(char *text);
 public:
  // 0x0060C920, still a pending_bodies forwarder.
  int item(char *text, int index);
  // 0x00609FA0, homed in leaf_recoveries.cpp (2026-09-27). DipEdit::init
  // adds one item per rival faction and then lands the selection on the
  // matching one through this id walk.
  void set_selected_id(int id);
  // 0x0060A020 ?set_selected_pos@ListBox@@QAEXHH@Z, the sibling of the above
  // and the only one of the pair BasePop::calculate_dimensions reaches (it
  // lands the popup's own list selection at two arguments). Its typed body
  // is in listbox.cpp; the retained seed preserves historical match evidence.
  // The second argument gates the notify dispatches the move ends with:
  // nonzero makes the body run the item's three notify slots (0x54, 0x53
  // and the 0x23/0x07 pair) after the scroll position is settled.
  void set_selected_pos(int position, int notify);
  // 0x0060C9B0 ?update_changes@ListBox@@QAEHXZ, unrecovered. DECLARATION
  // ONLY: Datalink::set_cat re-reads the entry list through it (and through
  // StringList::sort) once the category's own filler has run.
  int update_changes();
  // 0x0060A1E0 ?calculate_dimensions@ListBox@@QAEHXZ, still FILE-only in
  // src/unrecovered/0060a1e0.cpp. DECLARATION ONLY, added because Dialogs
  // ::calculate_dimensions (0x006132A0) tail-calls it for kind 2 - the access
  // costs nothing and the definition stays where it is.
  int calculate_dimensions();

 public:
  void on_mouse_leave(int a1, int a2);
  // DECLARATION-ONLY (2026-09-03), for the same reason as ~ListBox below and
  // from the same evidence. Editor_diplomacy (console.cpp) builds a stack
  // DipEdit, and the image's inlined DipEdit constructor reaches this member
  // with the three instructions `push 1 / lea ecx, [member] / call
  // ??0ListBox@@QAE@H@Z` (0x004DB8B1..0x004DB8BD) - one call, the virtual
  // bases built inside the callee. Spelled `{ ; }` here, VC6 expanded the
  // whole virtual-base stage inline at that site instead: the vbtable store,
  // both vbase constructors, and the two vtordisp writes, some 25
  // instructions the image does not have, and the enclosing frame grew four
  // bytes for the spill slot the expansion needed. Defined in listbox.cpp.
  ListBox();
  // DECLARATION-ONLY (2026-09-02): ??1ListBox is called out-of-line 35 times
  // in the image, and ??1DipEdit/??1FileWin tear their ListBox members down
  // with `call ??1ListBox` entered on the GraphicWin vbase - an inline-visible
  // definition makes those sites expand the body plus the compiler's
  // own vbase/vtordisp staging instead. Defined in listbox.cpp.
  ~ListBox();
  // close() and destroy() are void in the mangled names but are modelled as
  // uint32_t to preserve the EAX residue (both leave EAX = 0), as GraphicWin
  // and Scroll do for their teardown pair.
  uint32_t close();
  uint32_t destroy();
  void on_dialog_focus(int);

 private:
  // 0x00 is the vbtable pointer, EMITTED by the compiler now.
  uint32_t field_4_;              // 0x04
  uint32_t field_8_;              // 0x08
  uint32_t field_C_;              // 0x0C
  uint32_t field_10_;             // 0x10
  uint32_t field_14_;             // 0x14
 public:
  // PUBLIC for the same reason field_2C_ below is: Datalink::set_cat clears
  // both straight after emptying the entry list (`mov [esi+0x18], eax` /
  // `mov [esi+0x20], eax`, 0x0042A3C1..0x0042A3C6), with no ListBox call in
  // between, so the stores have to be spelled from outside the class. Access
  // is compile-time only; the static_assert pins the layout.
  uint32_t field_18_;             // 0x18
 private:
  uint32_t field_1C_;             // 0x1C
 public:
  uint32_t field_20_;             // 0x20
 private:
  uint32_t field_24_;             // 0x24
  uint32_t field_28_;             // 0x28
 public:
  // PUBLIC, like Dialog's own carved-out fields: DipEdit::init clears this
  // through the ListBox front (`mov dword ptr [esi+0xa78], eax` at
  // 0x004DB39C) with no call in between, so the access has to be spelled
  // from outside the class. Access is compile-time only; the static_assert
  // below pins the layout.
  uint32_t field_2C_;             // 0x2C
 private:
  DrawCallback draw_callback_;    // 0x30
  WriteCallback write_callback_;  // 0x34
  FillCallback fill_callback_;    // 0x38
  // go_to 0x004D7806 publishes its cdecl(int) selection notification here.
  // RECOVERY_MEMBER: v1 ListBox selection_callback_ 0x3C 0x004D61A0
  WinNotificationCallback selection_callback_;  // 0x3C
 public:
  // Notification installed by BasePop::calculate_dimensions and invoked
  // with Dialog::get_selected_id() by the list's double-click entry.
  // RECOVERY_MEMBER: v1 ListBox double_click_callback_ 0x40 0x0060C710
  WinNotificationCallback double_click_callback_;  // 0x40
 private:
  // 0x44 is GraphicWin's VTORDISP and 0x48 the base itself - both the
  // compiler's now, as is the Dialog base that follows.
};

static_assert(sizeof(ListBox) == 0xB54,
              "ListBox layout must match the legacy ABI");
// offsetof asserts are omitted deliberately: GraphicWin privately derives Win,
// so ListBox is not standard-layout and offsetof would warn. CheckBox
// (src/checkbox.h) makes the same choice; member ordering plus the two-vbtable
// test pin the offsets instead.

extern uint32_t ListBoxCloseDynamicDefault;            // 0x009B8EE0
// The current ListBox whose draw callback is running. Original draw_item
// stores its front receiver here at 0x0060BCAD, before invoking callbacks;
// the source row renderer now performs that write. The callback at 0x004072D0
// walks this object's Dialog virtual base for selection state.
extern ListBox *CurrentListBoxDraw;
// Shared row offset in the original scrolling/redraw paths. Its full set of
// source writers is not yet restored; the default zero is still meaningful.
extern int ListBoxDrawOffset;
// RECOVERY_OBJECT binding is in listbox.cpp: Scroll and ListBox's event
// paths share this process-wide notification gate.
extern int ListBoxGlobalDDC;
// Raised to 1 around on_right_down's and on_right_double_click's virtual-base
// dispatch, dropped to 0 after (0x0060AA20, 0x0060C6D0).
extern int ListBoxClickGuard;                          // 0x009B8EEC
extern uint32_t ListBoxCloseDynamicDefault;
extern uint32_t ListBoxDefault24;
extern uint32_t ListBoxDefault2C;
extern uint32_t ListBoxDefault10;
extern uint32_t ListBoxDefault14;

// Fixed most-derived offset from L to the controlling vftable (GraphicWin
// subobject); the destructor is always entered at L + this adjustment.
static const size_t ListBoxDestructorAdjustment = 0x48;

uint32_t __fastcall list_box_destructor_redirect(void *adjusted, void *);
