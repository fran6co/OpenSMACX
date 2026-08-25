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
#include "time.h"
#include "sprite.h"
#include "scroll.h"
#include "caviar.h"
#include "font.h"
#include "flatbutton.h"
#include "graphicwin.h"

 /*
  * ProdPicker class
  *
  * Derives from GraphicWin by ordinary single inheritance - its constructor calls
  * GraphicWin's on an unadjusted `this` - so it is written as the original wrote
  * it and needs no ABI workaround. GraphicWin is pinned here, so the base region
  * is exact; ProdPicker's own extent is not established and nothing pins its
  * sizeof.
  *
  * Most methods below ignore their arguments and return a constant, which is
  * why they could be replaced ahead of that mapping. close reaches one real
  * field: because GraphicWin is pinned at exactly 0xA14, the dword it clears
  * at 0xA14 is ProdPicker's own first field, which is exact rather than a
  * guess. Nothing past it is established.
  */
class ProdPicker : public GraphicWin {
 public:
  ProdPicker();
  // 0x00421100 is not recovered: a
  // pending_bodies forwarder, because an empty inline stub emits
  // nothing and the deleting destructor needs a `call rel32`.
  ~ProdPicker();
  int UNK1(int pos);
  void on_mouse_move(int a1, int a2);
  void UNK3(int a1);
  void close();
  // PAURECT@@H, per ?on_redraw_nc@ProdPicker@@QAEXPAURECT@@H@Z; the body is
  // `ret 8` either way and only the decorated name moves.
  void on_redraw_nc(RECT *, int);

 private:
  int32_t field_A14_;   // first field past the pinned GraphicWin base

  // Storage the image proves is here: its own methods reach 0x7124.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  // 22 member(s) from the IDA database, 18 named; it starts a member at 0xA18, which is where src/ ends.

  // The constructor placement-news every sub-object below EXPLICITLY, in the
  // exact order the disassembly shows - GraphicWin::construct() has to run
  // BEFORE any of them, and ordinary by-value members with a real constructor
  // are default-constructed by C++ before the constructor BODY ever runs, so
  // implicit member construction cannot reproduce this order. Raw storage
  // placement-new'd from the .cpp; every offset above lands on a 4-byte
  // boundary already, from the field sizes alone, so a plain `uint8_t[]`
  // needs no explicit alignment (VC6 has no `__declspec(align)` to give it
  // one - see vc6_compat.h).
  //
  // MEASURED: converting these to real declared members (Sprite[3], Font x4,
  // Time, FlatButton[9], Scroll, Caviar), built implicitly, was tried and
  // made this WORSE - 8/127 agreeing instructions against this baseline's
  // 11/127. The image's own `sub esp, 8` at instruction 7 (matching what
  // explicit placement-new already reproduces here) is NOT the null-guard
  // spill slot the PLACEMENT-NEW-COSTS-A-NULL-GUARD lever targets elsewhere
  // in this batch (ReportIf, SocialWin, PickWin, ReportWin): this
  // constructor's SEH prologue already agrees with the image 7/7 either way,
  // and real members instead cost the push ebx/esi/edi register-save order
  // the image actually has. Reverted.
  uint8_t sprites_[3 * 0x2C];  // 0xA18, 3 * sizeof(Sprite)
  uint8_t font1_[0x28];  // 0xA9C, sizeof(Font)
  uint8_t font2_[0x28];  // 0xAC4, sizeof(Font)
  uint8_t font3_[0x28];  // 0xAEC, sizeof(Font)
  uint8_t font4_[0x28];  // 0xB14, sizeof(Font)
  uint8_t time_[0x28];  // 0xB3C, sizeof(Time)
  uint32_t field_B64_;  // 0xB64
  uint32_t field_B68_;  // 0xB68
  uint32_t field_B6C_;  // 0xB6C
  uint32_t field_B70_;  // 0xB70
  uint8_t flatButtons_[9 * 0xB4C];  // 0xB74, 9 * sizeof(FlatButton)
  uint8_t scroll_[0x214C];  // 0x7120, sizeof(Scroll)

  uint8_t caviar_[0x13D0];  // 0x926C, sizeof(Caviar)

  // Storage the image proves is here: its own methods reach 0xA840.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  uint8_t field_A63C_[0x100];  // 0xA63C
  uint32_t field_A73C_;  // 0xA73C
  uint32_t field_A740_;  // 0xA740
  uint32_t field_A744_;  // 0xA744
  uint8_t field_A748_[0x18];  // 0xA748
  float field_A760_;  // 0xA760
  float field_A764_;  // 0xA764
  float field_A768_;  // 0xA768
  uint32_t field_A76C_;  // 0xA76C
  uint32_t field_A770_;  // 0xA770
  uint32_t field_A774_;  // 0xA774
  uint32_t field_A778_;  // 0xA778
  uint32_t field_A77C_;  // 0xA77C
  uint32_t field_A780_;  // 0xA780
  uint32_t field_A784_;  // 0xA784
  uint32_t field_A788_;  // 0xA788
  uint32_t field_A78C_;  // 0xA78C
  uint32_t field_A790_;  // 0xA790
  uint32_t field_A794_;  // 0xA794
  uint32_t field_A798_;  // 0xA798
  uint32_t field_A79C_;  // 0xA79C
  uint32_t field_A7A0_;  // 0xA7A0
  uint32_t field_A7A4_;  // 0xA7A4
  uint32_t field_A7A8_;  // 0xA7A8
  uint32_t field_A7AC_;  // 0xA7AC
  uint32_t field_A7B0_;  // 0xA7B0
  uint32_t field_A7B4_;  // 0xA7B4
  uint32_t field_A7B8_;  // 0xA7B8
  uint32_t field_A7BC_;  // 0xA7BC
  uint32_t field_A7C0_;  // 0xA7C0
  uint32_t field_A7C4_;  // 0xA7C4
  uint32_t field_A7C8_;  // 0xA7C8
  uint32_t field_A7CC_;  // 0xA7CC
  uint32_t field_A7D0_;  // 0xA7D0
  uint32_t field_A7D4_;  // 0xA7D4
  uint32_t field_A7D8_;  // 0xA7D8
  uint32_t field_A7DC_;  // 0xA7DC
  uint32_t field_A7E0_;  // 0xA7E0
  uint32_t field_A7E4_;  // 0xA7E4
  uint32_t field_A7E8_;  // 0xA7E8
  uint32_t field_A7EC_;  // 0xA7EC
  uint32_t field_A7F0_;  // 0xA7F0
  uint32_t field_A7F4_;  // 0xA7F4
  uint32_t field_A7F8_;  // 0xA7F8
  uint32_t field_A7FC_;  // 0xA7FC
  uint32_t field_A800_;  // 0xA800
  uint32_t field_A804_;  // 0xA804
  uint32_t field_A808_;  // 0xA808
  uint32_t field_A80C_;  // 0xA80C
  uint32_t field_A810_;  // 0xA810
  uint32_t field_A814_;  // 0xA814
  uint32_t field_A818_;  // 0xA818
  uint32_t field_A81C_;  // 0xA81C
  uint32_t field_A820_;  // 0xA820
  uint32_t field_A824_;  // 0xA824
  uint32_t field_A828_;  // 0xA828
  uint32_t field_A82C_;  // 0xA82C
  uint32_t field_A830_;  // 0xA830
  uint32_t field_A834_;  // 0xA834
  uint32_t field_A838_;  // 0xA838
  uint32_t field_A83C_;  // 0xA83C
};

static_assert(sizeof(ProdPicker) == 0xA840,
              "ProdPicker layout must match the original executable");

