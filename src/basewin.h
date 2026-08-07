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
  * BaseWin class
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
class DLLEXPORT BaseWin : GraphicWin {
 public:
  void on_scrolled(int a1, int a2);
  void UNK2(int a1, int a2, int a3, int a4);
  // Two siblings the recovered bodies above reach with a direct `call rel32`
  // - UNK2 calls garrison_click, on_scrolled calls draw_facilities. Both are
  // unrecovered; declared so their callers can name them.
  void garrison_click(int vehID, int a2, int a3, int a4);  // 0x0040B140
  void draw_facilities(int a1);                            // 0x0040FCC0
  void show(int a1);
  BaseWin() { ; }
  ~BaseWin() { ; }
  void close();
  void UNK4();
  void UNK6();
  void UNK7();
  void UNK5(int a1);
  void on_button_toggled(int a1, int a2);
  void on_mouse_leave(int a1, int a2);
  void on_iface_right_down(int a1, int a2);
  void on_iface_selected(int a1, int a2);
  void on_iface_left_click(int a1, int a2);
  void on_iface_right_click(int a1, int a2);
  void on_iface_left_double_click(int a1, int a2);
  void on_iface_right_double_click(int a1, int a2);
  void on_iface_scrolled(int a1, int a2);
  static void timer_callback(int a1, int a2);
  void on_left_click(int a1, int a2);
  void on_right_click(int a1, int a2);
  void on_left_double_click(int a1, int a2);

  // Storage the image proves is here: its own methods reach 0x40D20.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  // 293 member(s) from the IDA database, 30 named; it starts a member at 0xA14, which is where src/ ends.

  uint32_t subIFace_;  // 0xA14
  uint8_t prodPicker_[0xA840];  // 0xA18
  uint32_t field_B258_;  // 0xB258
  uint8_t graphicWin2_[0xA14];  // 0xB25C
  uint8_t sprites1_[0x58];  // 0xBC70
  uint8_t sprites2_[0x58];  // 0xBCC8
  uint8_t sprites3_[0x58];  // 0xBD20
  uint8_t sprite4_[0x2C];  // 0xBD78
  uint8_t sprite5_[0x2C];  // 0xBDA4
  uint8_t sprite6_[0x2C];  // 0xBDD0
  uint8_t sprites7_[0x84];  // 0xBDFC
  uint8_t flatButton_[0xB4C];  // 0xBE80
  uint8_t sprites8_[0x738];  // 0xC9CC
  uint8_t sprite9_[0x2C];  // 0xD104
  uint8_t sprite10_[0x2C];  // 0xD130
  uint8_t sprite11_[0x2C];  // 0xD15C
  uint8_t sprite12_[0x2C];  // 0xD188
  uint8_t sprite13_[0x2C];  // 0xD1B4
  uint8_t sprite14_[0x2C];  // 0xD1E0
  uint8_t sprite15_[0x2C];  // 0xD20C
  uint8_t buffer1_[0x588];  // 0xD238
  uint8_t flatButtons_[0xF888];  // 0xD7C0
  uint32_t field_1D048_;  // 0x1D048
  uint32_t field_1D04C_;  // 0x1D04C
  uint32_t field_1D050_;  // 0x1D050
  uint32_t field_1D054_;  // 0x1D054
  uint32_t field_1D058_;  // 0x1D058
  uint32_t field_1D05C_;  // 0x1D05C
  uint32_t field_1D060_;  // 0x1D060
  uint32_t field_1D064_;  // 0x1D064
  uint32_t field_1D068_;  // 0x1D068
  uint32_t field_1D06C_;  // 0x1D06C
  uint32_t field_1D070_;  // 0x1D070
  uint32_t field_1D074_;  // 0x1D074
  uint32_t field_1D078_;  // 0x1D078
  uint32_t field_1D07C_;  // 0x1D07C
  uint32_t field_1D080_;  // 0x1D080
  uint32_t field_1D084_;  // 0x1D084
  uint32_t field_1D088_;  // 0x1D088
  uint32_t field_1D08C_;  // 0x1D08C
  uint32_t field_1D090_;  // 0x1D090
  uint32_t field_1D094_;  // 0x1D094
  uint32_t field_1D098_;  // 0x1D098
  uint32_t field_1D09C_;  // 0x1D09C
  uint32_t field_1D0A0_;  // 0x1D0A0
  uint32_t field_1D0A4_;  // 0x1D0A4
  uint32_t field_1D0A8_;  // 0x1D0A8
  uint32_t field_1D0AC_;  // 0x1D0AC
  uint32_t field_1D0B0_;  // 0x1D0B0
  uint32_t field_1D0B4_;  // 0x1D0B4
  uint32_t field_1D0B8_;  // 0x1D0B8
  uint32_t field_1D0BC_;  // 0x1D0BC
  uint32_t field_1D0C0_;  // 0x1D0C0
  uint32_t field_1D0C4_;  // 0x1D0C4
  uint32_t field_1D0C8_;  // 0x1D0C8
  uint32_t field_1D0CC_;  // 0x1D0CC
  uint32_t field_1D0D0_;  // 0x1D0D0
  uint32_t field_1D0D4_;  // 0x1D0D4
  uint32_t field_1D0D8_;  // 0x1D0D8
  uint32_t field_1D0DC_;  // 0x1D0DC
  uint32_t field_1D0E0_;  // 0x1D0E0
  uint32_t field_1D0E4_;  // 0x1D0E4
  uint32_t field_1D0E8_;  // 0x1D0E8
  uint32_t field_1D0EC_;  // 0x1D0EC
  uint32_t field_1D0F0_;  // 0x1D0F0
  uint32_t field_1D0F4_;  // 0x1D0F4
  uint32_t field_1D0F8_;  // 0x1D0F8
  uint32_t field_1D0FC_;  // 0x1D0FC
  uint32_t field_1D100_;  // 0x1D100
  uint32_t field_1D104_;  // 0x1D104
  uint32_t field_1D108_;  // 0x1D108
  uint32_t field_1D10C_;  // 0x1D10C
  uint32_t field_1D110_;  // 0x1D110
  uint32_t field_1D114_;  // 0x1D114
  uint32_t field_1D118_;  // 0x1D118
  uint32_t field_1D11C_;  // 0x1D11C
  uint32_t field_1D120_;  // 0x1D120
  uint32_t field_1D124_;  // 0x1D124
  uint32_t field_1D128_;  // 0x1D128
  uint32_t field_1D12C_;  // 0x1D12C
  uint32_t field_1D130_;  // 0x1D130
  uint32_t field_1D134_;  // 0x1D134
  uint32_t field_1D138_;  // 0x1D138
  uint32_t field_1D13C_;  // 0x1D13C
  uint32_t field_1D140_;  // 0x1D140
  uint32_t field_1D144_;  // 0x1D144
  uint8_t time_[0x28];  // 0x1D148
  uint8_t caviar_[0x13D0];  // 0x1D170
  uint32_t field_1E540_;  // 0x1E540
  uint32_t field_1E544_;  // 0x1E544
  uint32_t field_1E548_;  // 0x1E548
  uint32_t field_1E54C_;  // 0x1E54C
  uint32_t field_1E550_;  // 0x1E550
  uint32_t field_1E554_;  // 0x1E554
  uint32_t field_1E558_;  // 0x1E558
  uint32_t field_1E55C_;  // 0x1E55C
  uint32_t field_1E560_;  // 0x1E560
  uint32_t field_1E564_;  // 0x1E564
  uint32_t field_1E568_;  // 0x1E568
  uint32_t field_1E56C_;  // 0x1E56C
  uint32_t field_1E570_;  // 0x1E570
  uint32_t field_1E574_;  // 0x1E574
  uint32_t field_1E578_;  // 0x1E578
  uint32_t field_1E57C_;  // 0x1E57C
  uint32_t field_1E580_;  // 0x1E580
  uint32_t field_1E584_;  // 0x1E584
  uint32_t field_1E588_;  // 0x1E588
  uint32_t field_1E58C_;  // 0x1E58C
  uint32_t field_1E590_;  // 0x1E590
  uint32_t field_1E594_;  // 0x1E594
  uint32_t field_1E598_;  // 0x1E598
  uint32_t field_1E59C_;  // 0x1E59C
  uint32_t field_1E5A0_;  // 0x1E5A0
  uint32_t field_1E5A4_;  // 0x1E5A4
  uint32_t field_1E5A8_;  // 0x1E5A8
  uint32_t field_1E5AC_;  // 0x1E5AC
  uint32_t field_1E5B0_;  // 0x1E5B0
  uint32_t field_1E5B4_;  // 0x1E5B4
  uint32_t field_1E5B8_;  // 0x1E5B8
  uint32_t field_1E5BC_;  // 0x1E5BC
  uint32_t field_1E5C0_;  // 0x1E5C0
  uint32_t field_1E5C4_;  // 0x1E5C4
  uint32_t field_1E5C8_;  // 0x1E5C8
  uint32_t field_1E5CC_;  // 0x1E5CC
  uint32_t field_1E5D0_;  // 0x1E5D0
  uint32_t field_1E5D4_;  // 0x1E5D4
  uint32_t field_1E5D8_;  // 0x1E5D8
  uint32_t field_1E5DC_;  // 0x1E5DC
  uint32_t field_1E5E0_;  // 0x1E5E0
  uint32_t field_1E5E4_;  // 0x1E5E4
  uint32_t field_1E5E8_;  // 0x1E5E8
  uint32_t field_1E5EC_;  // 0x1E5EC
  uint32_t field_1E5F0_;  // 0x1E5F0
  uint32_t field_1E5F4_;  // 0x1E5F4
  uint32_t field_1E5F8_;  // 0x1E5F8
  uint32_t field_1E5FC_;  // 0x1E5FC
  uint32_t field_1E600_;  // 0x1E600
  uint32_t field_1E604_;  // 0x1E604
  uint32_t field_1E608_;  // 0x1E608
  uint32_t field_1E60C_;  // 0x1E60C
  uint32_t field_1E610_;  // 0x1E610
  uint32_t field_1E614_;  // 0x1E614
  uint32_t field_1E618_;  // 0x1E618
  uint32_t field_1E61C_;  // 0x1E61C
  uint32_t field_1E620_;  // 0x1E620
  uint32_t field_1E624_;  // 0x1E624
  uint32_t field_1E628_;  // 0x1E628
  uint32_t field_1E62C_;  // 0x1E62C
  uint32_t field_1E630_;  // 0x1E630
  uint32_t field_1E634_;  // 0x1E634
  uint32_t field_1E638_;  // 0x1E638
  uint32_t field_1E63C_;  // 0x1E63C
  uint32_t field_1E640_;  // 0x1E640
  uint32_t field_1E644_;  // 0x1E644
  uint32_t field_1E648_;  // 0x1E648
  uint32_t field_1E64C_;  // 0x1E64C
  uint32_t field_1E650_;  // 0x1E650
  uint32_t field_1E654_;  // 0x1E654
  uint32_t field_1E658_;  // 0x1E658
  uint32_t field_1E65C_;  // 0x1E65C
  uint32_t field_1E660_;  // 0x1E660
  uint32_t field_1E664_;  // 0x1E664
  uint32_t field_1E668_;  // 0x1E668
  uint32_t field_1E66C_;  // 0x1E66C
  uint32_t field_1E670_;  // 0x1E670
  uint8_t mapWin_[0x22480];  // 0x1E674
  uint32_t field_40AF4_;  // 0x40AF4
  uint32_t field_40AF8_;  // 0x40AF8
  uint32_t field_40AFC_;  // 0x40AFC
  uint32_t field_40B00_;  // 0x40B00
  uint32_t field_40B04_;  // 0x40B04
  uint32_t field_40B08_;  // 0x40B08
  uint32_t focusedBaseID_;  // 0x40B0C
  uint32_t field_40B10_;  // 0x40B10
  uint32_t field_40B14_;  // 0x40B14
  uint32_t field_40B18_;  // 0x40B18
  uint32_t field_40B1C_;  // 0x40B1C
  uint32_t field_40B20_;  // 0x40B20
  uint32_t field_40B24_;  // 0x40B24
  uint32_t field_40B28_;  // 0x40B28
  uint32_t field_40B2C_;  // 0x40B2C
  uint32_t field_40B30_;  // 0x40B30
  uint32_t field_40B34_;  // 0x40B34
  uint32_t field_40B38_;  // 0x40B38
  uint32_t field_40B3C_;  // 0x40B3C
  uint32_t field_40B40_;  // 0x40B40
  uint32_t field_40B44_;  // 0x40B44
  uint32_t field_40B48_;  // 0x40B48
  uint32_t field_40B4C_;  // 0x40B4C
  uint32_t field_40B50_;  // 0x40B50
  uint32_t field_40B54_;  // 0x40B54
  uint32_t field_40B58_;  // 0x40B58
  uint32_t field_40B5C_;  // 0x40B5C
  uint32_t field_40B60_;  // 0x40B60
  uint32_t field_40B64_;  // 0x40B64
  uint32_t field_40B68_;  // 0x40B68
  uint32_t field_40B6C_;  // 0x40B6C
  uint32_t field_40B70_;  // 0x40B70
  uint32_t field_40B74_;  // 0x40B74
  uint32_t field_40B78_;  // 0x40B78
  uint32_t field_40B7C_;  // 0x40B7C
  uint32_t field_40B80_;  // 0x40B80
  uint32_t field_40B84_;  // 0x40B84
  uint32_t field_40B88_;  // 0x40B88
  uint8_t rect1_[0x10];  // 0x40B8C
  uint32_t field_40B9C_;  // 0x40B9C
  uint32_t field_40BA0_;  // 0x40BA0
  uint32_t field_40BA4_;  // 0x40BA4
  uint32_t field_40BA8_;  // 0x40BA8
  uint8_t rect2_[0x10];  // 0x40BAC
  uint32_t field_40BBC_;  // 0x40BBC
  uint32_t field_40BC0_;  // 0x40BC0
  uint32_t field_40BC4_;  // 0x40BC4
  uint32_t field_40BC8_;  // 0x40BC8
  uint32_t field_40BCC_;  // 0x40BCC
  uint32_t field_40BD0_;  // 0x40BD0
  uint32_t field_40BD4_;  // 0x40BD4
  uint32_t field_40BD8_;  // 0x40BD8
  uint8_t rect3_[0x10];  // 0x40BDC
  uint32_t field_40BEC_;  // 0x40BEC
  uint32_t field_40BF0_;  // 0x40BF0
  uint32_t field_40BF4_;  // 0x40BF4
  uint32_t field_40BF8_;  // 0x40BF8
  uint8_t rect4_[0x10];  // 0x40BFC
  uint32_t field_40C0C_;  // 0x40C0C
  uint32_t field_40C10_;  // 0x40C10
  uint32_t field_40C14_;  // 0x40C14
  uint32_t field_40C18_;  // 0x40C18
  uint32_t field_40C1C_;  // 0x40C1C
  uint32_t field_40C20_;  // 0x40C20
  uint32_t field_40C24_;  // 0x40C24
  uint32_t field_40C28_;  // 0x40C28
  uint32_t field_40C2C_;  // 0x40C2C
  uint32_t field_40C30_;  // 0x40C30
  uint32_t field_40C34_;  // 0x40C34
  uint32_t field_40C38_;  // 0x40C38
  uint32_t field_40C3C_;  // 0x40C3C
  uint32_t field_40C40_;  // 0x40C40
  uint32_t field_40C44_;  // 0x40C44
  uint32_t field_40C48_;  // 0x40C48
  uint32_t field_40C4C_;  // 0x40C4C
  uint32_t field_40C50_;  // 0x40C50
  uint32_t field_40C54_;  // 0x40C54
  uint32_t field_40C58_;  // 0x40C58
  uint32_t field_40C5C_;  // 0x40C5C
  uint32_t field_40C60_;  // 0x40C60
  uint32_t field_40C64_;  // 0x40C64
  uint32_t field_40C68_;  // 0x40C68
  uint32_t field_40C6C_;  // 0x40C6C
  uint32_t field_40C70_;  // 0x40C70
  uint32_t field_40C74_;  // 0x40C74
  uint32_t field_40C78_;  // 0x40C78
  uint32_t field_40C7C_;  // 0x40C7C
  uint32_t field_40C80_;  // 0x40C80
  uint32_t field_40C84_;  // 0x40C84
  uint32_t field_40C88_;  // 0x40C88
  uint32_t field_40C8C_;  // 0x40C8C
  uint32_t field_40C90_;  // 0x40C90
  uint32_t field_40C94_;  // 0x40C94
  uint32_t field_40C98_;  // 0x40C98
  uint32_t field_40C9C_;  // 0x40C9C
  uint32_t field_40CA0_;  // 0x40CA0
  uint32_t field_40CA4_;  // 0x40CA4
  uint32_t field_40CA8_;  // 0x40CA8
  uint32_t field_40CAC_;  // 0x40CAC
  uint32_t field_40CB0_;  // 0x40CB0
  uint32_t field_40CB4_;  // 0x40CB4
  uint32_t field_40CB8_;  // 0x40CB8
  uint32_t field_40CBC_;  // 0x40CBC
  uint32_t field_40CC0_;  // 0x40CC0
  uint32_t field_40CC4_;  // 0x40CC4
  uint32_t field_40CC8_;  // 0x40CC8
  uint32_t field_40CCC_;  // 0x40CCC
  uint32_t field_40CD0_;  // 0x40CD0
  uint32_t field_40CD4_;  // 0x40CD4
  uint32_t field_40CD8_;  // 0x40CD8
  uint32_t field_40CDC_;  // 0x40CDC
  uint32_t field_40CE0_;  // 0x40CE0
  uint32_t field_40CE4_;  // 0x40CE4
  uint32_t field_40CE8_;  // 0x40CE8
  uint32_t field_40CEC_;  // 0x40CEC
  uint32_t field_40CF0_;  // 0x40CF0
  uint32_t field_40CF4_;  // 0x40CF4
  uint32_t field_40CF8_;  // 0x40CF8
  uint32_t field_40CFC_;  // 0x40CFC
  uint32_t field_40D00_;  // 0x40D00
  uint32_t field_40D04_;  // 0x40D04
  uint32_t field_40D08_;  // 0x40D08
  uint32_t field_40D0C_;  // 0x40D0C
  uint32_t field_40D10_;  // 0x40D10
  uint32_t field_40D14_;  // 0x40D14
  uint32_t field_40D18_;  // 0x40D18
  uint8_t field_40D1C_[0x4];  // 0x40D1C

  // Storage the image proves is here: its own methods reach 0x45B34.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  uint8_t field_40D20_[0x4E14];
};

void __fastcall base_win_close_redirect(BaseWin *self, void *);
void __fastcall base_win_unk4_redirect(BaseWin *self, void *);
void __fastcall base_win_unk6_redirect(BaseWin *self, void *);
void __fastcall base_win_unk7_redirect(BaseWin *self, void *);
void __fastcall base_win_unk5_redirect(BaseWin *self, void *, int a1);
void __fastcall base_win_on_button_toggled_redirect(BaseWin *self, void *, int a1, int a2);
void __fastcall base_win_on_iface_right_down_redirect(BaseWin *self, void *, int a1, int a2);
void __fastcall base_win_on_iface_selected_redirect(BaseWin *self, void *, int a1, int a2);

// BaseWin embeds an interface subobject at 0xA14; these four are reached
// through its vtable, so their `this` points there and is adjusted back to
// the BaseWin before dispatching to iface_click, which is not recovered.
typedef void (OriginalObject::*func_base_win_iface_click)(int a1, int a2, int button, int is_double);
extern func_base_win_iface_click BaseWinIfaceClick;

// draw_supported is not recovered.
typedef void (OriginalObject::*func_base_win_draw_supported)(int a1);
extern func_base_win_draw_supported BaseWinDrawSupported;

// The shared click handler these three forward to is not recovered. Unlike
// the iface_click family, these carry no this-adjustment.
typedef void (OriginalObject::*func_base_win_click)(int a1, int a2, int button, int is_double);
extern func_base_win_click BaseWinClick;

void __fastcall base_win_on_iface_left_click_redirect(BaseWin *self, void *, int a1, int a2);
void __fastcall base_win_on_iface_right_click_redirect(BaseWin *self, void *, int a1, int a2);
void __fastcall base_win_on_iface_left_double_click_redirect(BaseWin *self, void *, int a1, int a2);
void __fastcall base_win_on_iface_right_double_click_redirect(BaseWin *self, void *, int a1, int a2);
void __fastcall base_win_on_iface_scrolled_redirect(BaseWin *self, void *, int a1, int a2);
void __fastcall base_win_on_left_click_redirect(BaseWin *self, void *, int a1, int a2);
void __fastcall base_win_on_right_click_redirect(BaseWin *self, void *, int a1, int a2);
void __fastcall base_win_on_left_double_click_redirect(BaseWin *self, void *, int a1, int a2);
void __cdecl base_win_timer_callback_redirect(int a1, int a2);
void __fastcall base_win_on_mouse_leave_redirect(BaseWin *self, void *, int a1, int a2);

// draw_facilities and garrison_click are declared on the class because the
// recovered on_scrolled and UNK2 reach them with a direct `call rel32`, but
// neither body is recovered - 1788 and 3768 bytes respectively - so the DLL
// has nothing to link the calls against. Both stand in with a forward to the
// original image until the bodies land.
//   ?draw_facilities@BaseWin@@QAEXH@Z    0x0040FCC0  public __thiscall void(int)
//   ?garrison_click@BaseWin@@QAEXHHHH@Z  0x0040B140  public __thiscall
//                                                    void(int, int, int, int)
typedef void (OriginalObject::*func_base_win_draw_facilities)(int a1);
extern func_base_win_draw_facilities BaseWinDrawFacilities;

typedef void (OriginalObject::*func_base_win_garrison_click)(int vehID, int a2,
                                                             int a3, int a4);
extern func_base_win_garrison_click BaseWinGarrisonClick;
