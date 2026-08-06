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
  * ": virtual GraphicWin, virtual Dialog" - the Itanium ABI this toolchain
  * follows would place the bases after the derived object instead, silently
  * producing the wrong layout. This mirrors CheckBox (src/checkbox.h), whose
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
constexpr size_t ListBoxDestructorAdjustment = 0x48;

void __fastcall list_box_on_dialog_focus_redirect(ListBox *self, void *, int a1);
uint32_t __fastcall list_box_close_redirect(ListBox *self, void *);
uint32_t __fastcall list_box_destructor_redirect(void *adjusted, void *);
