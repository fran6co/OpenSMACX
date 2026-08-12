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

 /*
  * DipEdit class
  *
  * Layout not established; its methods reach as far as 0x4E00, so the
  * object is at least that large. The stubs recovered here are constant
  * returns touching no field, so the opaque storage below is only an object
  * for the canary to seed, not a modelled layout.
  */
class DLLEXPORT DipEdit {
 public:
  void on_selected(int a1);
  // Siblings on_selected calls. Bodies unrecovered - 0x004DA990 (?read_check@
  // DipEdit@@QAEXXZ) and 0x004DADA0 (?do_check@DipEdit@@QAEXXZ) - so these are
  // declarations only, resolved at link time like the other unrecovered methods.
  void read_check();
  void do_check();
  DipEdit() { ; }
  ~DipEdit() { ; }
  void on_left_click(int, int);
  void on_redraw();

 private:
  uint8_t unmapped_[0xA20];
  // 0xA20, the IDB's `field_A20`. `?on_selected@DipEdit@@QAEXH@Z` at
  // 0x004DA740 stores its incoming argument straight here:
  // `mov dword ptr [esi + 0xa20], eax`. The IDB's next member is `font` at
  // 0xA24, so this is a standalone dword rather than the head of a run.
  int32_t field_A20_;
  uint8_t unmapped_A24_[0x43EC];  // 0xA24, where the IDB's `font` begins

  // 0xA20 + 4 + 0x43EC == 0x4E10: the split names storage that was already
  // here and moves nothing after it. The body still reaches the field by
  // offset, because DipEdit carries no size assertion and so is absent from
  // docs/recovery/verified-layouts.txt - the scaffolding a measured body is
  // compiled in would give this class an opaque shell, and a named member
  // there is `C2065: undeclared identifier`. 0x004DA740 is BYTE_EXACT;
  // naming it in the body would cost that.

  // Storage the image proves is here: its own methods reach 0x5240.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  uint8_t field_4E10_[0x430];
};

void __fastcall dip_edit_on_left_click_redirect(DipEdit *self, void *, int a1, int a2);
void __fastcall dip_edit_on_redraw_redirect(DipEdit *self, void *);

// The two siblings on_selected calls are still in the original image, and
// share one signature: ?read_check@DipEdit@@QAEXXZ (0x004DA990) and
// ?do_check@DipEdit@@QAEXXZ (0x004DADA0) are both public __thiscall void(void).
typedef void (OriginalObject::*func_dip_edit_check)();
extern func_dip_edit_check DipEditReadCheck;
extern func_dip_edit_check DipEditDoCheck;
