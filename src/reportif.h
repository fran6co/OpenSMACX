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
#include "listbox.h"
#include "subinterface.h"

 /*
  * ReportIf class
  *
  * Layout not established; its methods reach as far as 0x202E0, so the
  * object is at least that large. The stubs recovered here are constant
  * returns touching no field, so the opaque storage below is only an object
  * for the canary to seed, not a modelled layout.
  */
class ReportIf : SubInterface {
 public:
  void done();
  ReportIf() { ; }
  ~ReportIf() { ; }
  void close_energy();
  void close_intel();
  void on_iface_right_click(int, int);
  void on_iface_left_double_click(int, int);
  void on_iface_right_double_click(int, int);
  void on_iface_left_down(int, int);
  void on_iface_right_down(int, int);
  void on_iface_mouse_move(int, int);
  void on_iface_mouse_leave(int, int);
  void on_iface_button_toggled(int, int);
  void close_score();
  // PAUGraphicWin@@HHPAURECT@@, per the catalogued mangled name.
  int on_iface_dialog_item_back_draw(::GraphicWin *, int, int, RECT *);
  // 0x004A4060  ?bl_anim@ReportIf@@QAEXXZ - public, __thiscall, void(void),
  // unrecovered. The bl_daemon thunk in leaf_recoveries.cpp calls it.
  void bl_anim();

 private:
  // ??0ReportIf@@QAE@XZ installs the SubInterface vftable 0x0066A6E4 into
  // [this + 0] at 0x004AD1A7, before it constructs anything else - the same
  // table seven window classes store at their own +0xA14. The IDB agrees
  // (`ReportIf,0x0,subIFace,8`), which is why src/reportif.cpp already has to
  // reach the sub-object with `reinterpret_cast<SubInterface *>(this)`.
  // SubInterface is the BASE, above, so its 8 bytes are at 0.
  uint32_t field_8_;  // 0x8
  uint32_t field_C_;  // 0xC
  uint32_t field_10_;  // 0x10
  uint32_t field_14_;  // 0x14
  uint32_t field_18_;  // 0x18
  uint32_t field_1C_;  // 0x1C
  uint32_t field_20_;  // 0x20
  uint32_t field_24_;  // 0x24
  uint32_t field_28_;  // 0x28
  uint32_t field_2C_;  // 0x2C
  uint32_t field_30_;  // 0x30
  uint32_t field_34_;  // 0x34
  uint32_t field_38_;  // 0x38
  uint32_t field_3C_;  // 0x3C
  uint32_t field_40_;  // 0x40
  uint32_t field_44_;  // 0x44
  uint32_t field_48_;  // 0x48
  uint32_t field_4C_;  // 0x4C
  uint32_t field_50_;  // 0x50
  uint32_t field_54_;  // 0x54
  uint32_t field_58_;  // 0x58
  uint32_t field_5C_;  // 0x5C
  uint32_t field_60_;  // 0x60
  uint32_t field_64_;  // 0x64
  uint32_t field_68_;  // 0x68
  uint32_t field_6C_;  // 0x6C
  uint32_t field_70_;  // 0x70
  uint32_t field_74_;  // 0x74
  uint32_t field_78_;  // 0x78
  uint32_t field_7C_;  // 0x7C
  uint8_t field_80_[0x4F2C];  // 0x80
  uint32_t field_4FAC_;  // 0x4FAC
  uint32_t field_4FB0_;  // 0x4FB0
  uint8_t field_4FB4_[0x52E4];  // 0x4FB4
  uint32_t field_A298_;  // 0xA298
  uint8_t field_A29C_[0x30];  // 0xA29C
  uint32_t field_A2CC_;  // 0xA2CC
  uint32_t field_A2D0_;  // 0xA2D0
  uint8_t field_A2D4_[0xC];  // 0xA2D4
  uint32_t field_A2E0_;  // 0xA2E0
  uint32_t field_A2E4_;  // 0xA2E4
  uint8_t field_A2E8_[0x14];  // 0xA2E8
  uint32_t field_A2FC_;  // 0xA2FC
  uint8_t field_A300_[0x4];  // 0xA300
  uint32_t field_A304_;  // 0xA304
  uint8_t field_A308_[0xB1C];  // 0xA308
  uint32_t field_AE24_;  // 0xAE24
  uint8_t field_AE28_[0xC];  // 0xAE28
  uint32_t field_AE34_;  // 0xAE34
  uint32_t field_AE38_;  // 0xAE38
  uint8_t field_AE3C_[0x14];  // 0xAE3C
  uint32_t field_AE50_;  // 0xAE50
  uint8_t field_AE54_[0xBB8];  // 0xAE54
  uint32_t field_BA0C_;  // 0xBA0C
  uint8_t field_BA10_[0x440];  // 0xBA10
  uint32_t field_BE50_;  // 0xBE50
  uint8_t field_BE54_[0x704];  // 0xBE54
  uint32_t field_C558_;  // 0xC558
  uint8_t field_C55C_[0x440];  // 0xC55C
  uint32_t field_C99C_;  // 0xC99C
  uint8_t field_C9A0_[0x788];  // 0xC9A0
  uint32_t field_D128_;  // 0xD128
  uint8_t field_D12C_[0xC];  // 0xD12C
  uint32_t field_D138_;  // 0xD138
  uint8_t field_D13C_[0x440];  // 0xD13C
  uint32_t field_D57C_;  // 0xD57C
  uint8_t field_D580_[0x704];  // 0xD580
  uint32_t field_DC84_;  // 0xDC84
  uint8_t field_DC88_[0x440];  // 0xDC88
  uint32_t field_E0C8_;  // 0xE0C8
  uint8_t field_E0CC_[0x704];  // 0xE0CC
  uint32_t field_E7D0_;  // 0xE7D0
  uint8_t field_E7D4_[0x440];  // 0xE7D4
  uint32_t field_EC14_;  // 0xEC14
  uint8_t field_EC18_[0x704];  // 0xEC18
  uint32_t field_F31C_;  // 0xF31C
  uint8_t field_F320_[0x440];  // 0xF320
  uint32_t field_F760_;  // 0xF760
  uint8_t field_F764_[0x704];  // 0xF764
  uint32_t field_FE68_;  // 0xFE68
  uint8_t field_FE6C_[0x440];  // 0xFE6C
  uint32_t field_102AC_;  // 0x102AC
  uint8_t field_102B0_[0x704];  // 0x102B0
  uint32_t field_109B4_;  // 0x109B4
  uint8_t field_109B8_[0x440];  // 0x109B8
  uint32_t field_10DF8_;  // 0x10DF8
  uint8_t field_10DFC_[0x704];  // 0x10DFC
  uint32_t field_11500_;  // 0x11500
  uint8_t field_11504_[0x440];  // 0x11504
  uint32_t field_11944_;  // 0x11944
  uint8_t field_11948_[0x704];  // 0x11948
  uint32_t field_1204C_;  // 0x1204C
  uint8_t field_12050_[0x440];  // 0x12050
  uint32_t field_12490_;  // 0x12490
  uint8_t field_12494_[0x704];  // 0x12494
  uint32_t field_12B98_;  // 0x12B98
  uint8_t field_12B9C_[0x440];  // 0x12B9C
  uint32_t field_12FDC_;  // 0x12FDC
  uint8_t field_12FE0_[0x788];  // 0x12FE0
  uint32_t field_13768_;  // 0x13768
  uint8_t field_1376C_[0xC];  // 0x1376C
  uint32_t field_13778_;  // 0x13778
  uint8_t field_1377C_[0x4F94];  // 0x1377C
  uint32_t field_18710_;  // 0x18710
  uint8_t field_18714_[0xC];  // 0x18714
  uint32_t field_18720_;  // 0x18720
  uint8_t field_18724_[0x2D2C];  // 0x18724
  uint32_t field_1B450_;  // 0x1B450
  uint8_t field_1B454_[0xB48];  // 0x1B454
  uint32_t field_1BF9C_;  // 0x1BF9C
  uint8_t field_1BFA0_[0x440];  // 0x1BFA0
  uint32_t field_1C3E0_;  // 0x1C3E0
  uint8_t field_1C3E4_[0x704];  // 0x1C3E4
  uint32_t field_1CAE8_;  // 0x1CAE8
  uint8_t field_1CAEC_[0x440];  // 0x1CAEC
  uint32_t field_1CF2C_;  // 0x1CF2C
  uint8_t field_1CF30_[0x704];  // 0x1CF30
  uint32_t field_1D634_;  // 0x1D634
  uint8_t field_1D638_[0x440];  // 0x1D638
  uint32_t field_1DA78_;  // 0x1DA78
  uint8_t field_1DA7C_[0x704];  // 0x1DA7C
  uint32_t field_1E180_;  // 0x1E180
  uint32_t field_1E184_;  // 0x1E184
  uint32_t field_1E188_;  // 0x1E188
  uint32_t field_1E18C_;  // 0x1E18C
  uint32_t field_1E190_;  // 0x1E190
  uint32_t field_1E194_;  // 0x1E194
  uint32_t field_1E198_;  // 0x1E198
  uint32_t field_1E19C_;  // 0x1E19C
  uint32_t field_1E1A0_;  // 0x1E1A0
  uint32_t field_1E1A4_;  // 0x1E1A4
  uint32_t field_1E1A8_;  // 0x1E1A8
  uint32_t field_1E1AC_;  // 0x1E1AC
  uint32_t field_1E1B0_;  // 0x1E1B0
  uint32_t field_1E1B4_;  // 0x1E1B4
  uint32_t field_1E1B8_;  // 0x1E1B8
  uint8_t field_1E1BC_[0xC];  // 0x1E1BC
  uint32_t field_1E1C8_;  // 0x1E1C8
  uint32_t field_1E1CC_;  // 0x1E1CC
  uint32_t field_1E1D0_;  // 0x1E1D0
  uint32_t field_1E1D4_;  // 0x1E1D4
  uint32_t field_1E1D8_;  // 0x1E1D8
  uint32_t field_1E1DC_;  // 0x1E1DC
  uint32_t field_1E1E0_;  // 0x1E1E0
  uint8_t field_1E1E4_[0x3C];  // 0x1E1E4
  uint32_t field_1E220_;  // 0x1E220
  uint32_t field_1E224_;  // 0x1E224
  uint32_t field_1E228_;  // 0x1E228
  uint32_t field_1E22C_;  // 0x1E22C
  uint32_t field_1E230_;  // 0x1E230
  uint32_t field_1E234_;  // 0x1E234
  uint32_t field_1E238_;  // 0x1E238
  uint32_t field_1E23C_;  // 0x1E23C
  uint32_t field_1E240_;  // 0x1E240
  uint32_t field_1E244_;  // 0x1E244
  uint32_t field_1E248_;  // 0x1E248
  uint32_t field_1E24C_;  // 0x1E24C
  uint32_t field_1E250_;  // 0x1E250
  uint32_t field_1E254_;  // 0x1E254
  uint32_t field_1E258_;  // 0x1E258
  uint32_t field_1E25C_;  // 0x1E25C
  uint32_t field_1E260_;  // 0x1E260
  uint8_t field_1E264_[0x1FF8];  // 0x1E264
  uint8_t field_2025C_;  // 0x2025C
  uint8_t field_2025D_[0x93];  // 0x2025D
};

// 0x8 + 0x202E8 == 0x202F0, the extent this header modelled as one blob, so
// naming the head moved nothing. The size is a floor, not a pin: no
// static_assert, because nothing establishes where ReportIf ends.

// The energy report's third list box is a PROCESS-WIDE object at a fixed
// address, not a subobject: close_energy closes it alongside the two this
// object owns. Named here so the address appears once, the way
// ConsoleGlobal does for 0x009156B0, and so a fixture can repoint it.
ListBox *const ReportIfEnergyListBox = (ListBox *)0x0087BE84;  // 0x0087BE84

void __fastcall report_if_close_energy_redirect(ReportIf *self, void *);
void __fastcall report_if_close_intel_redirect(ReportIf *self, void *);
void __fastcall report_if_on_iface_right_click_redirect(ReportIf *self, void *, int a1, int a2);
void __fastcall report_if_on_iface_left_double_click_redirect(ReportIf *self, void *, int a1, int a2);
void __fastcall report_if_on_iface_right_double_click_redirect(ReportIf *self, void *, int a1, int a2);
void __fastcall report_if_on_iface_left_down_redirect(ReportIf *self, void *, int a1, int a2);
void __fastcall report_if_on_iface_right_down_redirect(ReportIf *self, void *, int a1, int a2);
void __fastcall report_if_on_iface_mouse_move_redirect(ReportIf *self, void *, int a1, int a2);
void __fastcall report_if_on_iface_mouse_leave_redirect(ReportIf *self, void *, int a1, int a2);
void __fastcall report_if_on_iface_button_toggled_redirect(ReportIf *self, void *, int a1, int a2);
void __fastcall report_if_close_score_redirect(ReportIf *self, void *);
int __fastcall report_if_on_iface_dialog_item_back_draw_redirect(
    ReportIf *self, void *, ::GraphicWin *a1, int a2, int a3, RECT *a4);

// ReportIf::bl_anim is not recovered, so the body at the end of reportif.cpp
// forwards through this seam. leaf_recoveries.cpp calls it, which is what
// keeps the definition: `__declspec(dllexport)` on the class once demanded a
// definition for every member, and that demand is gone.
