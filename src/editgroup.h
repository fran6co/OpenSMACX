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
#include "dialog.h"
#include "graphicwin.h"

 /*
  * EditGroup class
  *
  * Virtually derives from GraphicWin: the vbtable at 0x00670A3C reads
  * {0, 0x8C, 0xAA4}, so the base is held as a member at 0x8C and a Dialog
  * follows four bytes past where it ends, the same shape as RadioButton and
  * CheckBox.
  *
  * The three methods here are recoverable without that indirection because
  * none of them touches a virtual base - they work on this class's own boxes
  * and limits at 0x4 and 0x54. Anything on this class that does reach the
  * base or the Dialog must read the offsets from the object's own vbtable at
  * run time rather than from where they sit here; hardcoding them looks
  * correct, passes tests built on a most-derived object, and breaks whenever
  * the class is embedded in a larger one.
  *
  * Nothing pins this sizeof.
  *
  *   * MEASURED under VC6 12.00.8168, and the conclusion is OPEN, not closed.
  * `: virtual GraphicWin, virtual Dialog` gives 0xB4C against the 0xB54 this
  * class needs, and adding a virtual function to the DERIVED class gives
  * 0xB50 - that +4 is its own vfptr. An earlier note here blamed vtordisp
  * fields; that was WRONG and is withdrawn. A probe settles it: two bases
  * with a virtual destructor grow 0x10 -> 0x14 EACH, and the derived total
  * moves 0x34 -> 0x3C, so the missing 8 bytes are the two BASES' own vtable
  * pointers, not a displacement.
  *
  * Which means the conversion is probably reachable and is blocked on
  * something specific: GraphicWin and Dialog model their vtable as an opaque
  * dword the original installs by hand, so neither declares a virtual. Giving
  * them real virtual destructors is a coupled edit - the compiler then emits
  * the vfptr, so the modelled dword has to come out in the same change - and
  * `sizeof(GraphicWin) == 0xA14` is the check that would catch it going wrong.
  * Do NOT read this paragraph as "cannot be done".
  */
// The bases are REAL. The vbtable pointer, the base constructions, the
// vtordisp and the unwind all come from this declaration now.
class EditGroup : public virtual GraphicWin, public virtual Dialog {
 public:
  // 0x006126C0, a pending_bodies forwarder.
  void pass_dialog_focus();

 public:
  // 0x00612450, a pending_bodies forwarder.
  void on_redraw();

 public:
  // 0x00612670, a pending_bodies forwarder.
  void on_dialog_focus(int a1);

 public:
  // 0x00611AF0, a pending_bodies forwarder.
  int attach(void * a1, int a2, int a3, int a4);

 public:
  // ??0EditGroup@@QAE@H@Z at 0x00611940. The `int` is NOT a user parameter -
  // EditGroup's layout is composed by hand (see the class comment above), so
  // VC6 cannot inject the most-derived flag the way it would for a real
  // `: virtual GraphicWin, virtual Dialog` declaration; this constructor
  // takes an explicit flag instead and mirrors what the image's own guard
  // does with it.
  EditGroup(int a1);
  // The catalogued `??1EditGroup@@QAE@XZ` at 0x00611A20 is spelled as a
  // METHOD, not a destructor: the image's body reinstalls the base
  // vtables and calls `close()`, and never destroys `virtual_base_` or
  // `dialog_`. A real `~EditGroup()` has to destroy both, which is an
  // SEH frame and thirty extra instructions the image does not have.
  void destruct();
  // 0x00611A90, a pending_bodies forwarder. ~EditGroup calls it on the
  // adjusted (true) object pointer.
  void close();
  void set_text_limits(int limit);
  char *get_text(int index);
  void set_text(char *text, int index);

 private:
  // The vbtable pointer is EMITTED by the compiler now.
  // Indexed from 0x4 by both text accessors. The count is not established -
  // the span up to the limits array holds twenty pointers, and only that the
  // array starts at 0x4 is evidenced.
  void *boxes_[20];
  uint32_t limits_[10];
  uint8_t field_7C_[0x8];  // 0x7C
  uint32_t field_84_;  // 0x84
  // 0x88. Dialogs dispatches on this - at 0xF8 + 0x88 == 0x180, which that
  // header called `kind_` while it could not name the base it belongs to.
 protected:
  int32_t kind_;  // 0x88
 private:
  // GraphicWin and Dialog are VIRTUAL BASES, appended by the compiler.
};

// PINNED BEFORE CHANGING THE DECLARATION, so that replacing the hand-composed
// vbtable pointer and embedded subobjects with a real
// `: public virtual GraphicWin, public virtual Dialog` cannot move the layout
// without failing here. 0x8C of own data, then GraphicWin 0xA14, then the
// 4-byte vtordisp this header already carries as a gap, then Dialog 0xF4.
static_assert(sizeof(EditGroup) == 0xB9C,
              "EditGroup layout must match the original executable");

// EditBox is not declared yet; its text setter is reached at its address, and
// its text buffer sits 0xA14 into it.
typedef void (OriginalObject::*func_edit_box_set_text)(char *);
extern func_edit_box_set_text EditBoxOriginalSetText;

