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
  */
class DLLEXPORT EditGroup {
 public:
  EditGroup() { ; }
  ~EditGroup() { ; }
  void set_text_limits(int limit);
  char *get_text(int index);
  void set_text(char *text, int index);

 private:
  uint32_t vbtable_pointer_;
  // Indexed from 0x4 by both text accessors. The count is not established -
  // the span up to the limits array holds twenty pointers, and only that the
  // array starts at 0x4 is evidenced.
  void *boxes_[20];
  uint32_t limits_[10];
  uint8_t unmapped_7C_[0x8C - 0x7C];
  GraphicWin virtual_base_;
  uint8_t gap_AA0_[4];
  Dialog dialog_;
};

// EditBox is not declared yet; its text setter is reached at its address, and
// its text buffer sits 0xA14 into it.
typedef void (OriginalObject::*func_edit_box_set_text)(char *);
extern func_edit_box_set_text EditBoxOriginalSetText;

void __fastcall edit_group_set_text_limits_redirect(EditGroup *self, void *, int limit);
char *__fastcall edit_group_get_text_redirect(EditGroup *self, void *, int index);
void __fastcall edit_group_set_text_redirect(EditGroup *self, void *, char *text, int index);
