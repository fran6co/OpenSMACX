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
class DLLEXPORT ReportIf : SubInterface {
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
  uint8_t unmapped_[0x202E8];  // 0x8
};

// 0x8 + 0x202E8 == 0x202F0, the extent this header modelled as one blob, so
// naming the head moved nothing. The size is a floor, not a pin: no
// static_assert, because nothing establishes where ReportIf ends.

// The energy report's third list box is a PROCESS-WIDE object at a fixed
// address, not a subobject: close_energy closes it alongside the two this
// object owns. Named here so the address appears once, the way
// ConsoleGlobal does for 0x009156B0, and so a fixture can repoint it.
extern ListBox *ReportIfEnergyListBox;  // 0x0087BE84

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

// ReportIf::bl_anim is not recovered, and the class is DLLEXPORT - which
// demands a definition for every member - so the body at the end of
// reportif.cpp forwards through this seam.
typedef void (OriginalObject::*func_report_if_bl_anim)();
extern func_report_if_bl_anim ReportIfBlAnim;  // 0x004A4060
