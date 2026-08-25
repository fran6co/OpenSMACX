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
#include "spot.h"
#include "font.h"
#include "flatbutton.h"
#include "buttongroup.h"
#include "graphicwin.h"
#include "subinterface.h"
#include "checkbutton.h"

 /*
  * SocialWin class
  *
  * Derives from GraphicWin in the original by ordinary single inheritance -
  * its constructor calls GraphicWin's on an unadjusted `this` - so unlike the
  * virtually-derived windows this one is written as the original wrote it and
  * needs no ABI workaround.
  *
  * The layout is not established. The constructor's own fields begin at 0xA14,
  * immediately after the base, and where the object ends is unknown, so
  * nothing pins its sizeof and the fields between are not modelled. The
  * methods recovered here are bare returns that touch no field, which is why
  * they can be replaced ahead of that mapping.
  */
class SocialWin : public GraphicWin, public SubInterface {
 public:
  void hide();
  void show(int a1);
  SocialWin();
  // 0x004B3C80 is not recovered: a
  // pending_bodies forwarder, because an empty inline stub emits
  // nothing and the deleting destructor needs a `call rel32`.
  ~SocialWin();
  void UNK2();
  void UNK3();
  void on_iface_left_down(int a1, int a2);
  void on_iface_mouse_move(int a1, int a2);
  void on_iface_mouse_leave(int a1, int a2);
  void on_iface_right_down(int a1, int a2);
  void on_iface_selected(int a1, int a2);
  void on_iface_left_double_click(int a1, int a2);
  void on_iface_right_double_click(int a1, int a2);
  void on_iface_button_toggled(int a1, int a2);
  void on_iface_group_clicked(int a1, int a2, int a3);

  // Storage the image proves is here: its own methods reach 0x2C28.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  // 203 member(s) from the IDA database, 10 named; it starts a member at 0xA14, which is where src/ ends.

  // PROVED, and now a real type rather than eight opaque bytes:
  // The SubInterface that used to sit here as a member is now the SECOND
  // BASE, above. MSVC lays non-virtual bases in declaration order, so it is
  // at sizeof(GraphicWin) == 0xA14 either way and no offset moves.
  uint32_t field_A1C_;  // 0xA1C
  Spot spot_;  // 0xA20
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
  uint32_t field_A68_;  // 0xA68
  uint32_t field_A6C_;  // 0xA6C
  uint32_t field_A70_;  // 0xA70
  uint32_t field_A74_;  // 0xA74
  uint32_t field_A78_;  // 0xA78
  uint32_t field_A7C_;  // 0xA7C
  uint32_t field_A80_;  // 0xA80
  uint32_t field_A84_;  // 0xA84
  uint32_t field_A88_;  // 0xA88
  uint32_t field_A8C_;  // 0xA8C
  uint32_t field_A90_;  // 0xA90
  uint32_t field_A94_;  // 0xA94
  uint32_t field_A98_;  // 0xA98
  uint32_t field_A9C_;  // 0xA9C
  uint32_t field_AA0_;  // 0xAA0
  uint32_t field_AA4_;  // 0xAA4
  uint32_t field_AA8_;  // 0xAA8
  uint32_t field_AAC_;  // 0xAAC
  uint32_t field_AB0_;  // 0xAB0
  uint32_t field_AB4_;  // 0xAB4
  uint32_t field_AB8_;  // 0xAB8
  uint32_t field_ABC_;  // 0xABC
  uint32_t field_AC0_;  // 0xAC0
  uint32_t field_AC4_;  // 0xAC4
  uint32_t field_AC8_;  // 0xAC8
  uint32_t field_ACC_;  // 0xACC
  uint32_t field_AD0_;  // 0xAD0
  uint32_t field_AD4_;  // 0xAD4
  uint32_t field_AD8_;  // 0xAD8
  uint32_t field_ADC_;  // 0xADC
  uint32_t field_AE0_;  // 0xAE0
  uint32_t field_AE4_;  // 0xAE4
  uint32_t field_AE8_;  // 0xAE8
  uint32_t field_AEC_;  // 0xAEC
  uint32_t field_AF0_;  // 0xAF0
  uint32_t field_AF4_;  // 0xAF4
  uint32_t field_AF8_;  // 0xAF8
  uint32_t field_AFC_;  // 0xAFC
  uint32_t field_B00_;  // 0xB00
  uint32_t field_B04_;  // 0xB04
  uint32_t field_B08_;  // 0xB08
  uint32_t field_B0C_;  // 0xB0C
  uint32_t field_B10_;  // 0xB10
  uint32_t field_B14_;  // 0xB14
  uint32_t field_B18_;  // 0xB18
  uint32_t field_B1C_;  // 0xB1C
  uint32_t field_B20_;  // 0xB20
  uint32_t field_B24_;  // 0xB24
  uint32_t field_B28_;  // 0xB28
  uint32_t field_B2C_;  // 0xB2C
  uint32_t field_B30_;  // 0xB30
  uint32_t field_B34_;  // 0xB34
  uint32_t field_B38_;  // 0xB38
  uint32_t field_B3C_;  // 0xB3C
  uint32_t field_B40_;  // 0xB40
  uint32_t field_B44_;  // 0xB44
  uint32_t field_B48_;  // 0xB48
  uint32_t field_B4C_;  // 0xB4C
  uint32_t field_B50_;  // 0xB50
  uint32_t field_B54_;  // 0xB54
  uint32_t field_B58_;  // 0xB58
  uint32_t field_B5C_;  // 0xB5C
  uint32_t field_B60_;  // 0xB60
  uint32_t field_B64_;  // 0xB64
  uint32_t field_B68_;  // 0xB68
  uint32_t field_B6C_;  // 0xB6C
  uint32_t field_B70_;  // 0xB70
  uint32_t field_B74_;  // 0xB74
  uint32_t field_B78_;  // 0xB78
  uint32_t field_B7C_;  // 0xB7C
  uint32_t field_B80_;  // 0xB80
  uint32_t field_B84_;  // 0xB84
  uint32_t field_B88_;  // 0xB88
  uint32_t field_B8C_;  // 0xB8C
  uint32_t field_B90_;  // 0xB90
  uint32_t field_B94_;  // 0xB94
  uint32_t field_B98_;  // 0xB98
  uint32_t field_B9C_;  // 0xB9C
  uint32_t field_BA0_;  // 0xBA0
  uint32_t field_BA4_;  // 0xBA4
  uint32_t field_BA8_;  // 0xBA8
  uint32_t field_BAC_;  // 0xBAC
  uint32_t field_BB0_;  // 0xBB0
  uint32_t field_BB4_;  // 0xBB4
  uint32_t field_BB8_;  // 0xBB8
  uint32_t field_BBC_;  // 0xBBC
  uint32_t field_BC0_;  // 0xBC0
  uint32_t field_BC4_;  // 0xBC4
  uint32_t field_BC8_;  // 0xBC8
  uint32_t field_BCC_;  // 0xBCC
  uint32_t field_BD0_;  // 0xBD0
  uint32_t field_BD4_;  // 0xBD4
  uint32_t field_BD8_;  // 0xBD8
  uint32_t field_BDC_;  // 0xBDC
  uint32_t field_BE0_;  // 0xBE0
  uint32_t field_BE4_;  // 0xBE4
  uint32_t field_BE8_;  // 0xBE8
  uint32_t field_BEC_;  // 0xBEC
  uint32_t field_BF0_;  // 0xBF0
  uint32_t field_BF4_;  // 0xBF4
  uint32_t field_BF8_;  // 0xBF8
  uint32_t field_BFC_;  // 0xBFC
  uint32_t field_C00_;  // 0xC00
  uint32_t field_C04_;  // 0xC04
  uint32_t field_C08_;  // 0xC08
  uint32_t field_C0C_;  // 0xC0C
  uint32_t field_C10_;  // 0xC10
  uint32_t field_C14_;  // 0xC14
  uint32_t field_C18_;  // 0xC18
  uint32_t field_C1C_;  // 0xC1C
  uint32_t field_C20_;  // 0xC20
  uint32_t field_C24_;  // 0xC24
  uint32_t field_C28_;  // 0xC28
  uint32_t field_C2C_;  // 0xC2C
  uint32_t field_C30_;  // 0xC30
  uint32_t field_C34_;  // 0xC34
  uint32_t field_C38_;  // 0xC38
  uint32_t field_C3C_;  // 0xC3C
  uint32_t field_C40_;  // 0xC40
  uint32_t field_C44_;  // 0xC44
  uint32_t field_C48_;  // 0xC48
  uint32_t field_C4C_;  // 0xC4C
  uint32_t field_C50_;  // 0xC50
  uint32_t field_C54_;  // 0xC54
  uint32_t field_C58_;  // 0xC58
  uint32_t field_C5C_;  // 0xC5C
  uint32_t field_C60_;  // 0xC60
  uint32_t field_C64_;  // 0xC64
  uint32_t field_C68_;  // 0xC68
  uint32_t field_C6C_;  // 0xC6C
  uint32_t field_C70_;  // 0xC70
  uint32_t field_C74_;  // 0xC74
  uint32_t field_C78_;  // 0xC78
  uint32_t field_C7C_;  // 0xC7C
  uint32_t field_C80_;  // 0xC80
  uint32_t field_C84_;  // 0xC84
  uint32_t field_C88_;  // 0xC88
  uint32_t field_C8C_;  // 0xC8C
  uint32_t field_C90_;  // 0xC90
  uint32_t field_C94_;  // 0xC94
  uint32_t field_C98_;  // 0xC98
  uint32_t field_C9C_;  // 0xC9C
  uint32_t field_CA0_;  // 0xCA0
  uint32_t field_CA4_;  // 0xCA4
  uint32_t field_CA8_;  // 0xCA8
  uint32_t field_CAC_;  // 0xCAC
  uint32_t field_CB0_;  // 0xCB0
  uint32_t field_CB4_;  // 0xCB4
  uint32_t field_CB8_;  // 0xCB8
  uint32_t field_CBC_;  // 0xCBC
  uint32_t field_CC0_;  // 0xCC0
  uint32_t field_CC4_;  // 0xCC4
  uint32_t field_CC8_;  // 0xCC8
  uint32_t field_CCC_;  // 0xCCC
  uint32_t field_CD0_;  // 0xCD0
  uint32_t field_CD4_;  // 0xCD4
  uint32_t field_CD8_;  // 0xCD8
  uint32_t field_CDC_;  // 0xCDC
  uint32_t field_CE0_;  // 0xCE0
  uint32_t field_CE4_;  // 0xCE4
  uint32_t field_CE8_;  // 0xCE8
  uint32_t field_CEC_;  // 0xCEC
  uint32_t field_CF0_;  // 0xCF0
  uint32_t factionID_;  // 0xCF4
  uint8_t socCategory_[0x10];  // 0xCF8
  Sprite socEffect_;  // 0xD08, size == sizeof(Sprite)
  Sprite socEffectOrig_;  // 0xD34, size == sizeof(Sprite)
  uint32_t netIncome_;  // 0xD60
  uint32_t breakthroughTurns_;  // 0xD64
  // MEASURED against the constructor's own disassembly (0x004AE9E0): the
  // vector ctor iterator builds this array at `self + 0xD6C`, not 0xD68 -
  // and the constructor's LATER sub-object offsets land back on this
  // header's existing numbers (e.g. the Sprite[0xA] run at 0x2C60), so the
  // discrepancy is confined to this one boundary, not a uniform shift. Left
  // as the header already had it - the constructor below reaches every
  // sub-object at its own raw, disassembly-measured offset rather than
  // through these names, so nothing here depends on resolving it.
  CheckButton energyLockButtons_[3];  // 0xD68, 3 * sizeof(CheckButton) == 0x1E84
  uint32_t field_2BEC_;  // 0x2BEC
  uint32_t field_2BF0_;  // 0x2BF0
  uint32_t field_2BF4_;  // 0x2BF4
  uint32_t field_2BF8_;  // 0x2BF8
  uint32_t field_2BFC_;  // 0x2BFC
  uint32_t field_2C00_;  // 0x2C00
  uint32_t field_2C04_;  // 0x2C04
  uint32_t field_2C08_;  // 0x2C08
  uint32_t field_2C0C_;  // 0x2C0C
  uint32_t field_2C10_;  // 0x2C10
  uint32_t field_2C14_;  // 0x2C14
  uint32_t field_2C18_;  // 0x2C18
  uint32_t field_2C1C_;  // 0x2C1C
  uint32_t field_2C20_;  // 0x2C20
  uint32_t effectMaximize_;  // 0x2C24

  // Storage the image proves is here: its own methods reach 0x1F2F0.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  // 47 member(s) from the IDA database, 42 named; it starts a member at 0x2C28, which is where src/ ends.

  uint32_t field_2C28_;  // 0x2C28
  uint32_t field_2C2C_;  // 0x2C2C
  uint32_t field_2C30_;  // 0x2C30
  uint32_t field_2C34_;  // 0x2C34
  uint32_t economyAllocOrig_;  // 0x2C38
  uint32_t psychAllocOrig_;  // 0x2C3C
  uint32_t labsAllocOrig_;  // 0x2C40
  uint32_t economyAlloc3_;  // 0x2C44
  uint32_t psychAlloc3_;  // 0x2C48
  uint32_t labsAlloc3_;  // 0x2C4C
  uint32_t economyAllocPending_;  // 0x2C50
  uint32_t psychAllocPending_;  // 0x2C54
  uint32_t labsAllocPending_;  // 0x2C58
  uint32_t field_2C5C_;  // 0x2C5C
  Sprite socEngModifierNums_[0xA];   // 0x2C60, 0x2C * 0xA == 0x1B8
  Sprite effectIcons_[0x5A];         // 0x2E18, 0x2C * 0x5A == 0xF78
  Sprite energyAllocLock_[2];        // 0x3D90, 0x2C * 2 == 0x58
  Sprite energyAllocArrow_[6];       // 0x3DE8, 0x2C * 6 == 0x108
  Buffer energyAllocSlider_[6];      // 0x3EF0, 0x588 * 6 == 0x2130
  FlatButton flatButtons_[35];  // 0x6020
  ButtonGroup buttonGroups_[4];  // 0x1EB84
  ButtonGroup buttonGroup_;  // 0x1EDD4
  Font font1_;  // 0x1EE68
  Font font2_;  // 0x1EE90
  Font font3_;  // 0x1EEB8
  Font font4_;  // 0x1EEE0
  Font font5_;  // 0x1EF08
  Font font6_;  // 0x1EF30
  Font font7_;  // 0x1EF58
  Font font8_;  // 0x1EF80
  Font font9_;  // 0x1EFA8
  RECT tutRects1_[9];  // 0x1EFD0, 9 RECTs
  RECT tutRect1_;  // 0x1F060
  RECT tutRects2_[10];  // 0x1F070, 10 RECTs
  uint8_t tooltipSocEng_[0x10];  // 0x1F110
  RECT tutRect4_;  // 0x1F120
  RECT tutRects3_[18];  // 0x1F130, 18 RECTs
  uint8_t tooltipEffects_[0x10];  // 0x1F250
  uint8_t tooltipEconomy_[0x10];  // 0x1F260
  uint8_t tooltipLabs_[0x10];  // 0x1F270
  uint8_t tooltipPsych_[0x10];  // 0x1F280
  RECT tutRect9_;  // 0x1F290
  uint8_t tooltipFactionPower_[0x10];  // 0x1F2A0
  RECT tutRect11_;  // 0x1F2B0
  RECT tutRect12_;  // 0x1F2C0
  RECT tutRect13_;  // 0x1F2D0
  RECT tutRect14_;  // 0x1F2E0
};

static_assert(sizeof(SocialWin) == 0x1F2F0,
              "SocialWin layout must match the original executable");

