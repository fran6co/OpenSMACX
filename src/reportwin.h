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
#include "spot.h"
#include "listbox.h"
#include "flatbutton.h"
#include "buttongroup.h"
#include "buffer.h"
#include "graphicwin.h"
#include "scroll.h"
#include "flic.h"

 /*
  * ReportWin class
  *
  * Derives from GraphicWin by ordinary single inheritance - its constructor calls
  * GraphicWin's on an unadjusted `this` - so it is written as the original wrote
  * it and needs no ABI workaround. GraphicWin is pinned here, so the base region
  * is exact; ReportWin's own extent is not established and nothing pins its
  * sizeof.
  *
  * The methods below ignore their arguments and return a constant, which is
  * why they can be replaced ahead of that mapping.
  */
class DLLEXPORT ReportWin : GraphicWin {
 public:
  ReportWin() { ; }
  ~ReportWin() { ; }
  void on_mouse_move(int a1, int a2);
  void on_mouse_leave(int a1, int a2);
  // The two animation ticks the daemons in leaf_recoveries.cpp drive. Public,
  // __thiscall, void(void); both unrecovered.
  void sat_anim();  // 0x0049FE40  ?sat_anim@ReportWin@@QAEXXZ
  void exp_anim();  // 0x004A0100  ?exp_anim@ReportWin@@QAEXXZ

  // Storage the image proves is here: its own methods reach 0xFABC.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  // 100 member(s) from the IDA database, 21 named; it starts a member at 0xA14, which is where src/ ends.

  RECT rect1_;  // 0xA14, IDB `rect1`
  uint32_t field_A24_;  // 0xA24
  uint32_t field_A28_;  // 0xA28
  uint32_t field_A2C_;  // 0xA2C
  uint32_t field_A30_;  // 0xA30
  uint32_t field_A34_;  // 0xA34
  FlatButton flatButtons1_[7];  // 0xA38
  uint32_t field_594C_;  // 0x594C
  uint32_t field_5950_;  // 0x5950
  uint32_t field_5954_;  // 0x5954
  uint32_t field_5958_;  // 0x5958
  uint32_t field_595C_;  // 0x595C
  uint32_t field_5960_;  // 0x5960
  uint32_t field_5964_;  // 0x5964
  uint32_t field_5968_;  // 0x5968
  uint32_t field_596C_;  // 0x596C
  uint32_t field_5970_;  // 0x5970
  uint32_t field_5974_;  // 0x5974
  uint32_t field_5978_;  // 0x5978
  uint32_t field_597C_;  // 0x597C
  uint32_t field_5980_;  // 0x5980
  uint32_t field_5984_;  // 0x5984
  uint32_t field_5988_;  // 0x5988
  uint32_t field_598C_;  // 0x598C
  uint32_t field_5990_;  // 0x5990
  Spot spot_;  // 0x5994
  uint32_t field_59A0_;  // 0x59A0
  uint32_t field_59A4_;  // 0x59A4
  uint32_t field_59A8_;  // 0x59A8
  uint32_t field_59AC_;  // 0x59AC
  uint32_t field_59B0_;  // 0x59B0
  uint32_t field_59B4_;  // 0x59B4
  uint32_t field_59B8_;  // 0x59B8
  uint32_t field_59BC_;  // 0x59BC
  uint32_t field_59C0_;  // 0x59C0
  uint32_t field_59C4_;  // 0x59C4
  uint32_t field_59C8_;  // 0x59C8
  uint32_t field_59CC_;  // 0x59CC
  uint32_t field_59D0_;  // 0x59D0
  uint32_t field_59D4_;  // 0x59D4
  uint32_t field_59D8_;  // 0x59D8
  uint32_t field_59DC_;  // 0x59DC
  uint32_t field_59E0_;  // 0x59E0
  uint32_t field_59E4_;  // 0x59E4
  uint32_t field_59E8_;  // 0x59E8
  uint32_t field_59EC_;  // 0x59EC
  uint32_t field_59F0_;  // 0x59F0
  uint32_t field_59F4_;  // 0x59F4
  uint32_t field_59F8_;  // 0x59F8
  uint32_t field_59FC_;  // 0x59FC
  uint32_t field_5A00_;  // 0x5A00
  uint32_t field_5A04_;  // 0x5A04
  uint32_t field_5A08_;  // 0x5A08
  ListBox listBox_;  // 0x5A0C
  FlatButton flatButton2_;  // 0x6560
  FlatButton flatButton3_;  // 0x70AC
  uint32_t field_7BF8_;  // 0x7BF8
  uint32_t field_7BFC_;  // 0x7BFC
  uint32_t field_7C00_;  // 0x7C00
  uint32_t field_7C04_;  // 0x7C04
  ButtonGroup buttonGroup1_;  // 0x7C08
  FlatButton flatButton4_;  // 0x7C9C
  FlatButton flatButton5_;  // 0x87E8
  FlatButton flatButton6_;  // 0x9334
  ButtonGroup buttonGroup2_;  // 0x9E80
  FlatButton flatButton7_;  // 0x9F14
  FlatButton flatButton8_;  // 0xAA60
  FlatButton flatButton9_;  // 0xB5AC
  uint32_t field_C0F8_;  // 0xC0F8
  uint32_t field_C0FC_;  // 0xC0FC
  uint32_t field_C100_;  // 0xC100
  uint32_t field_C104_;  // 0xC104
  uint32_t field_C108_;  // 0xC108
  Scroll scroll_;  // 0xC10C, size == sizeof(Scroll)
  uint32_t field_E258_;  // 0xE258
  uint32_t field_E25C_;  // 0xE25C
  uint32_t field_E260_;  // 0xE260
  uint32_t field_E264_;  // 0xE264
  uint32_t field_E268_;  // 0xE268
  uint32_t field_E26C_;  // 0xE26C
  uint32_t field_E270_;  // 0xE270
  Time time1_;  // 0xE274
  uint32_t field_E29C_;  // 0xE29C
  uint32_t field_E2A0_;  // 0xE2A0
  uint32_t field_E2A4_;  // 0xE2A4
  uint32_t field_E2A8_;  // 0xE2A8
  uint32_t field_E2AC_;  // 0xE2AC
  uint32_t field_E2B0_;  // 0xE2B0
  uint32_t field_E2B4_;  // 0xE2B4
  uint32_t field_E2B8_;  // 0xE2B8
  uint32_t field_E2BC_;  // 0xE2BC
  Time time2_;  // 0xE2C0
  Time time3_;  // 0xE2E8
  uint32_t field_E310_;  // 0xE310
  uint32_t field_E314_;  // 0xE314
  uint32_t field_E318_;  // 0xE318
  uint32_t field_E31C_;  // 0xE31C
  Buffer buffer1_;  // 0xE320
  Buffer buffer2_;  // 0xE8A8
  Flic flic_;  // 0xEE30, declared Flic extent == 0xAE4
  uint8_t field_F914_[0x1A8];  // 0xF914
};

void __fastcall report_win_on_mouse_move_redirect(ReportWin *self, void *, int a1, int a2);
void __fastcall report_win_on_mouse_leave_redirect(ReportWin *self, void *, int a1, int a2);

// Neither animation tick is recovered; both forward to the original image.
// ?sat_anim@ReportWin@@QAEXXZ and ?exp_anim@ReportWin@@QAEXXZ are public,
// __thiscall, void(void), so one pointer-to-member shape serves both.
typedef void (OriginalObject::*func_report_win_anim)();
extern func_report_win_anim ReportWinSatAnim;
extern func_report_win_anim ReportWinExpAnim;
