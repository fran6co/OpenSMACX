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
#include "scroll.h"

 /*
  * StringBox class
  */
class DLLEXPORT StringBox : GraphicWin {
 public:
  StringBox() { ; }
  ~StringBox() { ; }
  void add(char *text, int index, int flag);

 private:
  uint32_t field_A14_;
  uint32_t field_A18_;
  uint32_t field_A1C_;
  Scroll scroll_;
  uint32_t field_2B6C_;
  uint32_t field_2B70_;
  uint32_t field_2B74_;
  uint32_t field_2B78_;
  uint32_t field_2B7C_;
  uint32_t field_2B80_;
  uint32_t field_2B84_;
  uint32_t field_2B88_;
  uint32_t field_2B8C_;
  uint32_t field_2B90_;
  uint32_t field_2B94_;
  uint32_t field_2B98_;
  uint32_t field_2B9C_;
};

static_assert(sizeof(StringBox) == 0x2BA0,
              "StringBox layout must match the original executable");

// add() stages three fields into the embedded string struct at 0x2B70 and
// then calls its add, falling back to a fixup pass when that reports the
// entry did not fit. Neither callee is recovered yet.
typedef int (OriginalObject::*func_string_struct_add)(int);
typedef void (OriginalObject::*func_string_box_add_fixup)();
extern func_string_struct_add StringBoxStructAdd;
extern func_string_box_add_fixup StringBoxAddFixup;

void __fastcall string_box_add_redirect(StringBox *self, void *, char *text,
                                        int index, int flag);
