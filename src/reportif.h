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
#include "flatbutton.h"
#include "listbox.h"
#include "subinterface.h"

 /*
  * ReportIf class
  *
  * The constructor's own disassembly (0x004AD170) and the destructor's
  * (0x004ACDA0) between them place every sub-object below at an exact,
  * cross-checked offset: the constructor placement-news each one in image
  * order, the destructor tears the same run down in reverse, and the two
  * agree on every boundary. The object's own last construction reaches
  * 0x20304 (a Sprite[4] ending there); nothing past it is established, so
  * there is no static_assert on sizeof(ReportIf).
  */
class ReportIf : public SubInterface {
 public:
  void done();
  ReportIf();
  ~ReportIf();
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

  // Everything below is placement-new'd EXPLICITLY from the constructor
  // body (0x004AD170), in image order, and torn down in the mirrored order
  // by the destructor (0x004ACDA0) - the two cross-check every boundary
  // here. Raw storage, not the real sub-object type: C++'s implicit member
  // construction would build these all before the constructor body ever
  // runs, which cannot reproduce the image's exact call order (SubInterface
  // itself is the empty-inline base, and nothing here is built through it).
  // MEASURED: real declared members (FlatButton[7], Sprite[0x15],
  // FlatButton[7]) built implicitly instead of placement-new - made this
  // WORSE, 26/132 against this baseline's 34/132, and grew the compiled
  // instruction count (189 -> 195). The base's `object[0] = vtable` store
  // must run explicitly in this body (SubInterface's own ctor is not
  // parametrised per host class, and that header is out of this batch's
  // scope), and standard C++ always runs implicit member construction
  // BEFORE any body statement, so converting these three pushes that store
  // after all three members instead of interleaved with the first - which is
  // not what the image does. Reverted.
  FlatButton flatButtonsA_[7];  // 0x80, 7 * sizeof(FlatButton), vector ctor iterator
  Sprite spritesA_[0x15];  // 0x4F94, 0x15 * sizeof(Sprite), vector ctor iterator
  uint8_t gap_5330_[0x54];  // 0x5330
  FlatButton flatButtonsB_[7];  // 0x5384, 7 * sizeof(FlatButton), vector ctor iterator
  uint8_t gap_A298_[0x38];  // 0xA298
  // The two ListBox members below are constructed with a BOOL argument
  // (`ListBox(1)` in the disassembly's call at 0x00609DB0) that has no
  // declared overload yet - that recovery is out of this batch's scope, so
  // the constructor reaches it at the raw image address, same as any other
  // not-yet-promoted callee.
  uint8_t listBox1_[0xB54];  // 0xA2D0, sizeof(ListBox)
  uint8_t listBox2_[0xB54];  // 0xAE24, sizeof(ListBox)
  uint8_t buttonGroup1_[0x94];  // 0xB978, sizeof(ButtonGroup)
  uint8_t flatButton1_[0xB4C];  // 0xBA0C
  uint8_t flatButton2_[0xB4C];  // 0xC558
  uint8_t buttonGroup2_[0x94];  // 0xD0A4
  uint8_t flatButton3_[0xB4C];  // 0xD138
  uint8_t flatButton4_[0xB4C];  // 0xDC84
  uint8_t flatButton5_[0xB4C];  // 0xE7D0
  uint8_t flatButton6_[0xB4C];  // 0xF31C
  uint8_t flatButton7_[0xB4C];  // 0xFE68
  uint8_t flatButton8_[0xB4C];  // 0x109B4
  uint8_t flatButton9_[0xB4C];  // 0x11500
  uint8_t flatButton10_[0xB4C];  // 0x1204C
  uint8_t flatButton11_[0xB4C];  // 0x12B98
  uint8_t buttonGroup3_[0x94];  // 0x136E4
  FlatButton flatButtonsC_[7];  // 0x13778, 7 * sizeof(FlatButton), vector ctor iterator
  uint8_t buttonGroup4_[0x94];  // 0x1868C
  FlatButton flatButtonsD_[5];  // 0x18720, 5 * sizeof(FlatButton), vector ctor iterator
  uint8_t flatButton12_[0xB4C];  // 0x1BF9C
  uint8_t flatButton13_[0xB4C];  // 0x1CAE8
  uint8_t flatButton14_[0xB4C];  // 0x1D634
  uint8_t gap_1E180_[0x3C];  // 0x1E180
  uint8_t spot_[0xC];  // 0x1E1BC, sizeof(Spot)
  uint8_t gap_1E1C8_[0x208C];  // 0x1E1C8
  Sprite spritesB_[4];  // 0x20254, 4 * sizeof(Sprite), vector ctor iterator
};

// The constructor's own last store reaches 0x20254 + 0xB0 == 0x20304; there
// is no static_assert on sizeof(ReportIf) because nothing establishes where
// the object actually ends past that.

// The energy report's third list box is a PROCESS-WIDE object at a fixed
// address, not a subobject: close_energy closes it alongside the two this
// object owns. Named here so the address appears once, the way
// ConsoleGlobal does for 0x009156B0, and so a fixture can repoint it.
ListBox *const ReportIfEnergyListBox = (ListBox *)0x0087BE84;  // 0x0087BE84


// ReportIf::bl_anim is not recovered, so the body at the end of reportif.cpp
// forwards through this seam. leaf_recoveries.cpp calls it, which is what
// keeps the definition: `__declspec(dllexport)` on the class once demanded a
// definition for every member, and that demand is gone.
