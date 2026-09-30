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
  * The palette, scrollbar, editor and button members below are now used by
  * the recovered dialog lifecycle and gamma-ramp methods.
  */
class Gamma : public GraphicWin {
 public:
  // 0x005C9520, a pending_bodies forwarder.
  void adjust_palette();

 public:
  // (code, pos), the Win32 WM_VSCROLL shape the image mirrors. Evidence
  // is in the bodies: BaseWin::on_scrolled does `switch (a1)` and then
  // stores a2 in field_40B10_; StringBox::on_scrolled stores a2 in
  // field_A1C_; BaseWin::on_iface_scrolled tests `a1 != 2` and copies a2.
  // The first is switched on, the second is kept - a code and a position.
  void on_scrolled(int code, int pos);
  Gamma();
  // 0x00456110 lives in gamma.cpp. C++ emits the reverse member teardown;
  // the body first closes the dialog's owned controls.
  ~Gamma();
  void close();
  void on_button_clicked(int button_id);
  void adjust_pal(double gamma);
  virtual void on_redraw();
  // 0x005C8E60 returns the initial value in apply-only mode and the final
  // double-valued dialog gamma otherwise; PrefWin rounds it to float.
  double exec(Palette *target, float initial_gamma, int apply_only);
  int on_key_click(int a, int b);

  // Storage the image proves is here: its own methods reach 0x1920.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  // 24 member(s) from the IDA database, 4 named; it starts a member at 0xA14, which is where src/ ends.

  // exec builds five consecutive RECTs from the screen dimensions. Their
  // offsets and four-word copy/adjustment pattern are explicit in the seed.
  // RECOVERY_MEMBER: v2 Gamma rect1_ tagRECT 0xA14 0x005C8E60
  RECT rect1_;  // 0xA14, palette strip
  // RECOVERY_MEMBER: v2 Gamma rect2_ tagRECT 0xA24 0x005C8E60
  RECT rect2_;  // 0xA24, scrollbar
  // RECOVERY_MEMBER: v2 Gamma rect3_ tagRECT 0xA34 0x005C8E60
  RECT rect3_;  // 0xA34, scrollbar bounds
  // RECOVERY_MEMBER: v2 Gamma rect4_ tagRECT 0xA44 0x005C8E60
  RECT rect4_;  // 0xA44, accept button bounds
  // RECOVERY_MEMBER: v2 Gamma rect5_ tagRECT 0xA54 0x005C8E60
  RECT rect5_;  // 0xA54, cancel button bounds
  // exec stores its Palette* argument here and adjust_pal edits that palette.
  // RECOVERY_MEMBER: v3 Gamma target_palette_ Palette* 0xA64 0x005C8E60
  Palette *target_palette_;  // 0xA64
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

  // A BODY NOW REACHES IT, which is the condition the note here used to set
  // for declaring this. `??0Gamma@@QAE@XZ` constructs a PushButton at 0x46B0
  // outright - `lea ecx, [esi + 0x46b0]; call 0x62bf20` at 0x005C8E12, the
  // same 0x62BF20 it calls for pushButton1_ eight bytes earlier - so the IDB's
  // `pushButton2` claim is no longer beyond the proven extent. `?exec@Gamma@@`
  // then drives it at 0x005C91E5/0x005C91F8 with id -2, the dialog's cancel,
  // which is what `initialGamma_` above exists to restore.
  //
  // This also removes the four bytes of tail padding the placeholder forced:
  // `double gamma_` makes the class 8-byte aligned and 0x46B4 is not a
  // multiple of 8, so sizeof(Gamma) was 0x46B8. It now ends at 0x51B0, which
  // is 8-aligned and needs none - exactly as the withdrawn note predicted.
  PushButton pushButton2_;  // 0x46B0, IDB `pushButton2`, 0xB00
};


// ?adjust_palette@Gamma@@QAEXXZ (0x005C9520) is source-owned in gamma.cpp.
// Gamma::on_scrolled calls it by name.
