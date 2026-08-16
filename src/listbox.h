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
#include "radiobutton.h"  // shared func_dialog_close typedef, as checkbox.h does

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
class DLLEXPORT ListBox {
 public:
  void on_mouse_leave(int a1, int a2);
  ListBox() { ; }
  ~ListBox() { ; }
  // close() and destroy() are void in the mangled names but are modelled as
  // uint32_t to preserve the EAX residue (both leave EAX = 0), as GraphicWin
  // and Scroll do for their teardown pair.
  uint32_t close();
  uint32_t destroy();
  void on_dialog_focus(int);

 private:
  uint32_t vbtable_pointer_;      // 0x00 -> 0x00670584
  uint32_t field_4_;              // 0x04
  uint32_t field_8_;              // 0x08
  uint32_t field_C_;              // 0x0C
  uint32_t field_10_;             // 0x10
  uint32_t field_14_;             // 0x14
  uint32_t field_18_;             // 0x18
  uint32_t field_1C_;             // 0x1C
  uint32_t field_20_;             // 0x20
  uint32_t field_24_;             // 0x24
  uint32_t field_28_;             // 0x28
  uint32_t field_2C_;             // 0x2C
  uint32_t field_30_;             // 0x30
  uint32_t field_34_;             // 0x34
  uint32_t field_38_;             // 0x38
  uint32_t field_3C_;             // 0x3C
  uint32_t field_40_;             // 0x40
  uint32_t graphic_vbase_adjust_; // 0x44  (GraphicWin - 4)
  GraphicWin virtual_base_;       // 0x48  (0xA14; Buffer subobject at +0x444 -> 0x48C)
  uint32_t dialog_vbase_adjust_;  // 0xA5C (Dialog - 4)
  Dialog dialog_;                 // 0xA60 (0xF4; ends 0xB54)
};

static_assert(sizeof(ListBox) == 0xB54,
              "ListBox layout must match the legacy ABI");
// offsetof asserts are omitted deliberately: GraphicWin privately derives Win,
// so ListBox is not standard-layout and offsetof would warn. CheckBox
// (src/checkbox.h) makes the same choice; member ordering plus the two-vbtable
// test pin the offsets instead.

// ListBox::close reaches Dialog::close (0x00608F50, still an original
// dependency) through a rebindable seam, identical to CheckBox/RadioButton, so
// tests can substitute a probe and the seam can later point at a recovered body.
extern func_dialog_close ListBoxOriginalDialogClose;   // default 0x00608F50
extern uint32_t *ListBoxCloseStaticDefaults;            // 0x006970E0 [0..3]
extern uint32_t *ListBoxCloseDynamicDefault;            // 0x009B8EE0

// Fixed most-derived offset from L to the controlling vftable (GraphicWin
// subobject); the destructor is always entered at L + this adjustment.
static const size_t ListBoxDestructorAdjustment = 0x48;

void __fastcall list_box_on_dialog_focus_redirect(ListBox *self, void *, int a1);
uint32_t __fastcall list_box_close_redirect(ListBox *self, void *);
uint32_t __fastcall list_box_destructor_redirect(void *adjusted, void *);
