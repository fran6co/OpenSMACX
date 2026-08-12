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
  uint32_t field_0_;  // 0x0
  uint8_t field_4_[0x470];  // 0x4
  uint32_t field_474_;  // 0x474
  uint32_t field_478_;  // 0x478
  uint32_t field_47C_;  // 0x47C
  uint32_t field_480_;  // 0x480
  uint8_t field_484_[0x590];  // 0x484
  uint32_t field_A14_;  // 0xA14
  uint32_t field_A18_;  // 0xA18
  uint32_t field_A1C_;  // 0xA1C
  // 0xA20, the IDB's `field_A20`. `?on_selected@DipEdit@@QAEXH@Z` at
  // 0x004DA740 stores its incoming argument straight here:
  // `mov dword ptr [esi + 0xa20], eax`. The IDB's next member is `font` at
  // 0xA24, so this is a standalone dword rather than the head of a run.
  int32_t field_A20_;
  uint8_t field_A24_[0x28];  // 0xA24
  uint32_t field_A4C_;  // 0xA4C
  uint8_t field_A50_[0x28];  // 0xA50
  uint32_t field_A78_;  // 0xA78
  uint8_t field_A7C_[0xB24];  // 0xA7C
  uint32_t field_15A0_;  // 0x15A0
  uint8_t field_15A4_[0xC];  // 0x15A4
  uint32_t field_15B0_;  // 0x15B0
  uint32_t field_15B4_;  // 0x15B4
  uint8_t field_15B8_[0xB10];  // 0x15B8
  uint32_t field_20C8_;  // 0x20C8
  uint8_t field_20CC_[0x440];  // 0x20CC
  uint32_t field_250C_;  // 0x250C
  uint8_t field_2510_[0x704];  // 0x2510
  uint32_t field_2C14_;  // 0x2C14
  uint8_t field_2C18_[0x440];  // 0x2C18
  uint32_t field_3058_;  // 0x3058
  uint8_t field_305C_[0x704];  // 0x305C
  uint32_t field_3760_;  // 0x3760
  uint8_t field_3764_[0x440];  // 0x3764
  uint32_t field_3BA4_;  // 0x3BA4
  uint8_t field_3BA8_[0x704];  // 0x3BA8
  uint32_t field_42AC_;  // 0x42AC
  uint8_t field_42B0_[0x440];  // 0x42B0
  uint32_t field_46F0_;  // 0x46F0
  uint8_t field_46F4_[0x704];  // 0x46F4
  uint32_t field_4DF8_;  // 0x4DF8
  uint8_t field_4DFC_[0x14];  // 0x4DFC

  // 0xA20 + 4 + 0x43EC == 0x4E10: the split names storage that was already
  // here and moves nothing after it. The body still reaches the field by
  // offset, because DipEdit carries no size assertion and so is absent from
  // docs/recovery/verified-layouts.txt - the scaffolding a measured body is
  // compiled in would give this class an opaque shell, and a named member
  // there is `C2065: undeclared identifier`. 0x004DA740 is BYTE_EXACT;
  // naming it in the body would cost that.

  // Storage the image proves is here: its own methods reach 0x5240.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  uint8_t field_4E10_[0x42C];  // 0x4E10
  uint32_t field_523C_;  // 0x523C
};

void __fastcall dip_edit_on_left_click_redirect(DipEdit *self, void *, int a1, int a2);
void __fastcall dip_edit_on_redraw_redirect(DipEdit *self, void *);

// The two siblings on_selected calls are still in the original image, and
// share one signature: ?read_check@DipEdit@@QAEXXZ (0x004DA990) and
// ?do_check@DipEdit@@QAEXXZ (0x004DADA0) are both public __thiscall void(void).
typedef void (OriginalObject::*func_dip_edit_check)();
extern func_dip_edit_check DipEditReadCheck;
extern func_dip_edit_check DipEditDoCheck;
