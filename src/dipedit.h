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
#include "graphicwin.h"

 /*
  * DipEdit class
  *
  * Derives from GraphicWin by ordinary single inheritance. `??1DipEdit@@QAE@XZ`
  * (0x004E2240) parks the receiver with `mov esi, ecx` at 0x004E225D and never
  * reloads esi; after the last member subobject is torn down - `??1Font@@QAE@XZ`
  * on `lea ecx, [esi + 0xa24]` at 0x004E2396 - it issues
  *
  *     0x004E239B  mov ecx, esi
  *     0x004E23A4  call ??1GraphicWin@@QAE@XZ
  *
  * with no lea in between. An UNADJUSTED `this`, and last of all the teardown
  * calls: that is the base subobject's destructor. `??_GDipEdit@@UAEPAXI@Z` is
  * decorated `U`, so DipEdit has a vptr of its own; GraphicWin sitting at
  * offset 0 rather than 4 means DipEdit shares GraphicWin's vptr, which an
  * embedded first member could not do. So this is inheritance, not containment.
  *
  * That measurement WITHDRAWS the previous "Layout not established" reading
  * and the seven anonymous declarations it justified. They covered exactly
  * [0, 0xA14) - field_0_, field_4_[0x470], field_474_, field_478_, field_47C_,
  * field_480_, field_484_[0x590] - which is exactly sizeof(GraphicWin) (0xA14),
  * so they were the base's bytes described as unmapped storage; the base
  * declaration replaces them and moves nothing. Everything from 0xA14 on is
  * DipEdit's own and is untouched, and sizeof(DipEdit) is 0x5240 before and
  * after.
  *
  * What is still NOT established is where the object ends: its methods reach
  * as far as 0x5240, the class carries no size assertion, and the runs above
  * 0xA14 remain an extent floor rather than a modelled layout.
  */
class DipEdit : GraphicWin {
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
  // The GraphicWin base occupies [0, 0xA14). DipEdit's own storage starts here.
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
