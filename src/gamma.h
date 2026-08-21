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
#include "font.h"
#include "graphicwin.h"
#include "palette.h"
#include "scroll.h"
#include "editbox.h"
#include "pushbutton.h"

 /*
  * Gamma class
  *
  * Derives from GraphicWin by ordinary single inheritance - its constructor calls
  * GraphicWin's on an unadjusted `this` - so it is written as the original wrote
  * it and needs no ABI workaround. GraphicWin is pinned here, so the base region
  * is exact; Gamma's own extent is not established and nothing pins its
  * sizeof.
  *
  * The methods below ignore their arguments and return a constant, which is
  * why they can be replaced ahead of that mapping.
  */
class Gamma : public GraphicWin {
 public:
  // 0x005C9520, a pending_bodies forwarder.
  void adjust_palette();

 public:
  void on_scrolled(int a1, int a2);
  Gamma() { ; }
  ~Gamma() { ; }
  int on_key_click(int a1, int a2);

  // Storage the image proves is here: its own methods reach 0x1920.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  // 24 member(s) from the IDA database, 4 named; it starts a member at 0xA14, which is where src/ ends.

  RECT rect1_;  // 0xA14, IDB `rect1`
  uint32_t field_A24_;  // 0xA24
  uint32_t field_A28_;  // 0xA28
  uint32_t field_A2C_;  // 0xA2C
  uint32_t field_A30_;  // 0xA30
  uint32_t field_A34_;  // 0xA34
  uint32_t field_A38_;  // 0xA38
  uint32_t field_A3C_;  // 0xA3C
  uint32_t field_A40_;  // 0xA40
  uint32_t field_A44_;  // 0xA44
  uint32_t field_A48_;  // 0xA48
  uint32_t field_A4C_;  // 0xA4C
  uint32_t field_A50_;  // 0xA50
  uint32_t field_A54_;  // 0xA54
  uint32_t field_A58_;  // 0xA58
  uint32_t field_A5C_;  // 0xA5C
  uint32_t field_A60_;  // 0xA60
  uint32_t field_A64_;  // 0xA64
  Palette palette_;  // 0xA68, IDB `palette`, size == sizeof(Palette)
  Font font_;  // 0xEBC
  // 0xEE4 and 0xEE8 are floating point, and the IDB's three dwords here were
  // two members, not three. What withdrew the `uint32_t field_EE4_ /
  // field_EE8_ / field_EEC_` spelling:
  //   ?exec@Gamma@@QAEXPAUPalette@@MH@Z  0x005C8E77
  //       mov [esi+0xEE4], ecx      <- the float parameter's raw four bytes
  //   ?on_button_clicked@Gamma@@QAEXH@Z  0x005C93EE
  //       fld dword ptr [ecx+0xEE4] <- read back as a float, so 0xEE4 is float
  //   ?on_button_clicked@Gamma@@QAEXH@Z  0x005C93DE / 0x005C93F4
  //       fstp qword ptr [ecx+0xEE8] <- one EIGHT-byte store, so 0xEE8 and
  //                                     0xEEC are one double, not two dwords
  //   ??0Gamma@@QAE@XZ  0x005C8E24 / 0x005C8E3E
  //       [esi+0xEE8] = 0, [esi+0xEEC] = 0x3FF00000, which is the double 1.0
  //       split into its two halves - the constructor's initialiser
  //   ?on_redraw@Gamma@@QAEXXZ  0x005C94C9
  //       push [esi+0xEEC]; push [esi+0xEE8] into sprintf against "%f"
  //       (0x00691E7C) - one double vararg, low half last
  //
  // What they are FOR. ?adjust_palette@Gamma@@QAEXXZ (0x005C9520) recomputes
  // 0xEE8 as `fild [this+0x191C]; fmul 0.01`. 0x191C is scroll_ + 0xA2C,
  // which is the dword ?set_pos@Scroll@@QAEXH@Z stores the clamped position
  // into (0x00605D3E), and exec sets the range to 1..400 (0x005C90EC) and the
  // position to (int)(the float argument * 100.0f) (0x005C90FA) - so 0xEE8
  // spans 0.01..4.00. adjust_palette then fills 236 palette entries with
  // pow(i * 1/235.0, 1.0 / [0xEE8]), the exponent of a gamma ramp, so 0xEE8
  // is the gamma itself and not its reciprocal.
  // 0xEE4 is only ever written from exec's float argument and only ever read
  // to restore 0xEE8, on button -2 (pushButton2_, id pushed at 0x005C91F3);
  // button -1 (pushButton1_, 0x005C913A) takes gamma from the scroll instead.
  // So 0xEE4 is the value the dialog opened with, and -2 is its cancel.
  float initialGamma_;  // 0xEE4
  double gamma_;  // 0xEE8, spans 0xEE8..0xEEF; 0xEEC is its high half
  Scroll scroll_;  // 0xEF0, IDB `scroll`, size == sizeof(Scroll)
  EditBox editBox_;  // 0x303C, IDB `editBox`, size == sizeof(EditBox)
  PushButton pushButton1_;  // 0x3BB0, IDB `pushButton1`, 0xB00

  // Storage the image proves is here: its own methods reach 0x46B4.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  // The IDB additionally claims pushButton2 (0xB00) at 0x46B0, beyond the
  // proven extent; it is not declared until a body reaches it. (?exec@Gamma@@
  // constructs a BaseButton there at 0x005C91E5/0x005C91F8 with id -2, so it
  // is a button; the extent, not the identity, is what is missing.)
  //
  // sizeof(Gamma) is 0x46B8, not 0x46B4: `double gamma_` above makes the class
  // 8-byte aligned, and the floor this placeholder ends at is not a multiple
  // of 8, so VC6 adds four bytes of tail padding. That is an artefact of where
  // the DECLARATIONS stop, not of the object - with pushButton2 declared the
  // class would end at 0x51B0, which is already 8-aligned and needs no
  // padding. Measured: no other member moved (all 120 shared offsets identical
  // before and after), and the only recorded floor, access-lower-bounds.csv's
  // Gamma >= 0x46B4, is still met.
  uint8_t field_46B0_[0x4];  // 0x46B0
};

int __fastcall gamma_on_key_click_redirect(Gamma *self, void *, int a1, int a2);

// ?adjust_palette@Gamma@@QAEXXZ (0x005C9520) is not recovered. Gamma::on_scrolled
// is nothing but a call to it, so the call goes through a rebindable seam
// rather than a member this DLL cannot define.
