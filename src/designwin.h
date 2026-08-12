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
#include "font.h"
#include "flatbutton.h"
#include "caviar.h"
#include "buffer.h"
#include "graphicwin.h"
#include "subinterface.h"

 /*
  * DesignWin class
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
class DLLEXPORT DesignWin : GraphicWin, SubInterface {
 public:
  // The shared body both select_special_N thunks tail into:
  // ?select_special@DesignWin@@QAEXH@Z at 0x0043AC70, still an original body,
  // so this is a declaration only, resolved at link time.
  void select_special(int a1);
  void select_special_2();
  void select_special_1();
  DesignWin() { ; }
  ~DesignWin() { ; }
  void UNK1();
  void on_iface_left_down(int a1, int a2);
  void on_mouse_leave(int a1, int a2);
  void on_iface_right_down(int a1, int a2);
  void on_iface_selected(int a1, int a2);
  void on_iface_left_double_click(int a1, int a2);
  void on_iface_right_double_click(int a1, int a2);
  void on_iface_button_toggled(int a1, int a2);
  void on_iface_group_clicked(int a1, int a2, int a3);

  // Storage the image proves is here: its own methods reach 0x14210.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  // 259 member(s) from the IDA database, 14 named; it starts a member at 0xA14, which is where src/ ends.

  // PROVED, and now a real type rather than eight opaque bytes:
  // The SubInterface that used to sit here as a member is now the SECOND
  // BASE, above. MSVC lays non-virtual bases in declaration order, so it is
  // at sizeof(GraphicWin) == 0xA14 either way and no offset moves.
  uint32_t field_A1C_;  // 0xA1C
  uint32_t field_A20_;  // 0xA20
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
  FlatButton flatButtons_[25];  // 0xCEC
  Spot spot1_;  // 0x12758
  Spot spot2_;  // 0x12764
  uint32_t field_12770_;  // 0x12770
  Time time_;  // 0x12774
  Caviar caviar_;  // 0x1279C
  uint32_t field_13B6C_;  // 0x13B6C
  uint32_t field_13B70_;  // 0x13B70
  uint32_t field_13B74_;  // 0x13B74
  uint32_t field_13B78_;  // 0x13B78
  uint32_t field_13B7C_;  // 0x13B7C
  uint32_t field_13B80_;  // 0x13B80
  uint32_t field_13B84_;  // 0x13B84
  uint32_t field_13B88_;  // 0x13B88
  uint32_t field_13B8C_;  // 0x13B8C
  uint32_t field_13B90_;  // 0x13B90
  uint32_t field_13B94_;  // 0x13B94
  uint32_t field_13B98_;  // 0x13B98
  uint32_t field_13B9C_;  // 0x13B9C
  uint32_t field_13BA0_;  // 0x13BA0
  uint32_t field_13BA4_;  // 0x13BA4
  uint32_t field_13BA8_;  // 0x13BA8
  uint32_t field_13BAC_;  // 0x13BAC
  uint32_t field_13BB0_;  // 0x13BB0
  uint32_t field_13BB4_;  // 0x13BB4
  uint32_t field_13BB8_;  // 0x13BB8
  uint32_t field_13BBC_;  // 0x13BBC
  uint32_t field_13BC0_;  // 0x13BC0
  uint32_t field_13BC4_;  // 0x13BC4
  uint32_t field_13BC8_;  // 0x13BC8
  uint32_t field_13BCC_;  // 0x13BCC
  uint32_t field_13BD0_;  // 0x13BD0
  uint32_t field_13BD4_;  // 0x13BD4
  uint32_t field_13BD8_;  // 0x13BD8
  uint32_t field_13BDC_;  // 0x13BDC
  uint32_t field_13BE0_;  // 0x13BE0
  uint32_t field_13BE4_;  // 0x13BE4
  uint32_t field_13BE8_;  // 0x13BE8
  uint32_t field_13BEC_;  // 0x13BEC
  uint32_t field_13BF0_;  // 0x13BF0
  uint32_t field_13BF4_;  // 0x13BF4
  uint32_t field_13BF8_;  // 0x13BF8
  uint32_t field_13BFC_;  // 0x13BFC
  uint32_t field_13C00_;  // 0x13C00
  uint32_t field_13C04_;  // 0x13C04
  uint32_t field_13C08_;  // 0x13C08
  uint32_t field_13C0C_;  // 0x13C0C
  uint32_t field_13C10_;  // 0x13C10
  uint32_t field_13C14_;  // 0x13C14
  uint32_t field_13C18_;  // 0x13C18
  uint32_t field_13C1C_;  // 0x13C1C
  uint32_t field_13C20_;  // 0x13C20
  uint32_t field_13C24_;  // 0x13C24
  uint32_t field_13C28_;  // 0x13C28
  uint32_t field_13C2C_;  // 0x13C2C
  uint32_t field_13C30_;  // 0x13C30
  uint32_t field_13C34_;  // 0x13C34
  uint32_t field_13C38_;  // 0x13C38
  uint32_t field_13C3C_;  // 0x13C3C
  uint32_t field_13C40_;  // 0x13C40
  uint32_t field_13C44_;  // 0x13C44
  uint32_t field_13C48_;  // 0x13C48
  uint32_t field_13C4C_;  // 0x13C4C
  uint32_t field_13C50_;  // 0x13C50
  uint32_t field_13C54_;  // 0x13C54
  uint32_t field_13C58_;  // 0x13C58
  uint32_t field_13C5C_;  // 0x13C5C
  uint32_t field_13C60_;  // 0x13C60
  uint32_t field_13C64_;  // 0x13C64
  uint32_t field_13C68_;  // 0x13C68
  Buffer buffer_;  // 0x13C6C
  uint32_t factionID_;  // 0x141F4
  uint32_t protoID_;  // 0x141F8
  uint32_t protoChassisType_;  // 0x141FC
  uint32_t protoWeaponType_;  // 0x14200
  uint32_t protoArmorType_;  // 0x14204
  uint32_t protoAbilityFlags_;  // 0x14208
  uint32_t protoReactorType_;  // 0x1420C

  // Storage the image proves is here: its own methods reach 0x145A4.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  // 28 member(s) from the IDA database, 6 named; it starts a member at 0x14210, which is where src/ ends.

  uint32_t field_14210_;  // 0x14210
  uint32_t mineralCostFactor_;  // 0x14214
  uint32_t protoAbilityID1_;  // 0x14218
  uint32_t protoAbilityID2_;  // 0x1421C
  uint32_t field_14220_;  // 0x14220
  uint32_t field_14224_;  // 0x14224
  uint32_t field_14228_;  // 0x14228
  uint32_t field_1422C_;  // 0x1422C
  uint32_t field_14230_;  // 0x14230
  uint32_t field_14234_;  // 0x14234
  uint32_t field_14238_;  // 0x14238
  uint32_t field_1423C_;  // 0x1423C
  uint32_t field_14240_;  // 0x14240
  uint32_t field_14244_;  // 0x14244
  uint32_t field_14248_;  // 0x14248
  uint32_t field_1424C_;  // 0x1424C
  uint32_t field_14250_;  // 0x14250
  uint32_t field_14254_;  // 0x14254
  uint32_t field_14258_;  // 0x14258
  uint32_t field_1425C_;  // 0x1425C
  uint32_t field_14260_;  // 0x14260
  uint32_t field_14264_;  // 0x14264
  uint32_t field_14268_;  // 0x14268
  uint32_t field_1426C_;  // 0x1426C
  uint8_t field_14270_[0x2D0];  // 0x14270
  Font font1_;  // 0x14540
  Font font2_;  // 0x14568
  Font font3_;  // 0x14590, the IDB's third font; the class grows by 0x14
};

void __fastcall design_win_unk1_redirect(DesignWin *self, void *);
void __fastcall design_win_on_iface_left_down_redirect(DesignWin *self, void *, int a1, int a2);
void __fastcall design_win_on_iface_right_down_redirect(DesignWin *self, void *, int a1, int a2);
void __fastcall design_win_on_iface_selected_redirect(DesignWin *self, void *, int a1, int a2);
void __fastcall design_win_on_iface_left_double_click_redirect(DesignWin *self, void *, int a1, int a2);
void __fastcall design_win_on_iface_right_double_click_redirect(DesignWin *self, void *, int a1, int a2);
void __fastcall design_win_on_iface_button_toggled_redirect(DesignWin *self, void *, int a1, int a2);
void __fastcall design_win_on_iface_group_clicked_redirect(DesignWin *self, void *, int a1, int a2, int a3);
void __fastcall design_win_on_mouse_leave_redirect(DesignWin *self, void *, int a1, int a2);

// DesignWin::select_special is not recovered, and DesignWin is DLLEXPORT -
// which demands a definition for every member - so the body at the end of
// designwin.cpp forwards through this seam. The `H` in
// ?select_special@DesignWin@@QAEXH@Z is the one int both select_special_N
// thunks pass.
typedef void (OriginalObject::*func_design_win_select_special)(int);
extern func_design_win_select_special DesignWinSelectSpecial;  // 0x0043AC70
