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
#include "buttongroup.h"
#include "listbox.h"
#include "font.h"

#include "original_seam.h"
#include "graphicwin.h"

 /*
  * Datalink class
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
// Declared and deliberately NOT defined. parse_id writes only its first four
// bytes, and nothing available says how large it is or what follows: the type
// exists in the original's mangled name and in no analysis output here. An
// incomplete type expresses exactly that - the parameter is this type, and
// this translation unit knows nothing else about it - where inventing a
// one-int struct would state a size the evidence does not support.
struct DatalinkID;

class Datalink : GraphicWin {
 public:
  // 0x00429180, a pending_bodies forwarder.
  void exec(unsigned int topic, int index);

 public:
  void hide();
  void show(int a1);
  void on_selected(int a1);
  // 0x0042BF10  ?draw_entry@Datalink@@QAEXXZ - public, __thiscall, void(void),
  // unrecovered. Declared so on_selected, which reaches it with a direct
  // `call rel32` on an unadjusted `this`, can name it.
  void draw_entry();
  Datalink() { ; }
  ~Datalink() { ; }
  int UNK1(int a1, int a2);
  void parse_id(int id, DatalinkID *out_id, int *out_remainder);
  void UNK6();
  void UNK8();
  void UNK9();
  void close();
  void on_left_click(int a1, int a2);
  void on_group_clicked(int a1, int a2);
  void on_mouse_move(int a1, int a2);
  void on_iface_left_click(int a1, int a2);
  void on_iface_right_click(int a1, int a2);
  void on_iface_left_down(int a1, int a2);
  void on_iface_right_down(int a1, int a2);
  void on_iface_mouse_move(int a1, int a2);
  void on_iface_mouse_leave(int a1, int a2);
  void on_iface_selected(int a1, int a2);
  void on_iface_left_double_click(int a1, int a2);
  void on_iface_right_double_click(int a1, int a2);
  void on_iface_button_clicked(int a1);
  void on_iface_button_toggled(int a1, int a2);

 private:
  // GraphicWin is pinned at 0xA14, which fixes the origin. Everything between
  // there and the three fields close() clears is unmapped; the span holds
  // their offsets in place and the test checks them directly.
  uint32_t field_A14_;  // 0xA14
  uint32_t field_A18_;  // 0xA18
  uint32_t field_A1C_;  // 0xA1C
  uint32_t field_A20_;  // 0xA20
  uint32_t field_A24_;  // 0xA24
  uint8_t field_A28_[0xB44];  // 0xA28
  uint32_t field_156C_;  // 0x156C
  uint32_t field_1570_;  // 0x1570
  uint32_t field_1574_;  // 0x1574
  uint32_t field_1578_;  // 0x1578
  uint32_t field_157C_;  // 0x157C
  uint32_t field_1580_;  // 0x1580
  uint8_t field_1584_[0x2C];  // 0x1584
  uint8_t field_15B0_;  // 0x15B0
  uint8_t field_15B1_[0xA73];  // 0x15B1
  uint32_t field_2024_;  // 0x2024
  uint32_t field_2028_;  // 0x2028
  uint8_t field_202C_[0x8AC];  // 0x202C
  uint32_t field_28D8_;  // 0x28D8
  uint8_t field_28DC_[0x100];  // 0x28DC
  uint32_t field_29DC_;  // 0x29DC
  int32_t facilityID_;
  uint32_t field_29E4_;  // 0x29E4
  uint32_t field_29E8_;  // 0x29E8
  uint8_t field_29EC_[0x48];  // 0x29EC
  int32_t field_2A34_;
  int32_t field_2A38_;

  // Storage the image proves is here: its own methods reach 0xF62C.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  // 20 member(s) from the IDA database, 20 named; it starts a member at 0x2A3C, which is where src/ ends.

  FlatButton flatButton1_;  // 0x2A3C
  FlatButton flatButton2_;  // 0x3588
  FlatButton flatButton3_;  // 0x40D4
  FlatButton flatButton4_;  // 0x4C20
  FlatButton flatButton5_;  // 0x576C
  FlatButton flatButton6_;  // 0x62B8
  FlatButton flatButton7_;  // 0x6E04
  FlatButton flatButton8_;  // 0x7950
  FlatButton flatButton9_;  // 0x849C
  FlatButton flatButton10_;  // 0x8FE8
  FlatButton flatButton11_;  // 0x9B34
  FlatButton flatButton12_;  // 0xA680
  FlatButton flatButton13_;  // 0xB1CC
  FlatButton flatButton14_;  // 0xBD18
  FlatButton flatButton15_;  // 0xC864
  FlatButton flatButton16_;  // 0xD3B0
  FlatButton flatButton17_;  // 0xDEFC
  FlatButton flatButton18_;  // 0xEA48
  ButtonGroup buttonGroup_;  // 0xF594
  ListBox listBox_;  // 0xF628

  Font font1_;  // 0x1017C, IDB `font1`, size == sizeof(Font)

  Font font2_;  // 0x101A4, IDB `font2`, size == sizeof(Font)

  Font font3_;  // 0x101CC, IDB `font3`, size == sizeof(Font)

  // Storage the image proves is here: its own methods reach 0x1B394.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  uint8_t field_101F4_[0x50];  // 0x101F4
  uint32_t field_10244_;  // 0x10244
  uint8_t field_10248_[0xC];  // 0x10248
  uint32_t field_10254_;  // 0x10254
  uint8_t field_10258_[0x3C];  // 0x10258
  uint32_t field_10294_;  // 0x10294
  uint8_t field_10298_[0x9C];  // 0x10298
  uint32_t field_10334_;  // 0x10334
  uint32_t field_10338_;  // 0x10338
  uint32_t field_1033C_;  // 0x1033C
  uint32_t field_10340_;  // 0x10340
  uint8_t field_10344_[0x10];  // 0x10344
  uint32_t field_10354_;  // 0x10354
  uint32_t field_10358_;  // 0x10358
  uint32_t field_1035C_;  // 0x1035C
  uint32_t field_10360_;  // 0x10360
  uint32_t field_10364_;  // 0x10364
  uint32_t field_10368_;  // 0x10368
  uint32_t field_1036C_;  // 0x1036C
  uint32_t field_10370_;  // 0x10370
  uint32_t field_10374_;  // 0x10374
  uint32_t field_10378_;  // 0x10378
  uint32_t field_1037C_;  // 0x1037C
  uint32_t field_10380_;  // 0x10380
  uint32_t field_10384_;  // 0x10384
  uint32_t field_10388_;  // 0x10388
  uint32_t field_1038C_;  // 0x1038C
  uint32_t field_10390_;  // 0x10390
  uint32_t field_10394_;  // 0x10394
  uint32_t field_10398_;  // 0x10398
  uint32_t field_1039C_;  // 0x1039C
  uint32_t field_103A0_;  // 0x103A0
  uint32_t field_103A4_;  // 0x103A4
  uint32_t field_103A8_;  // 0x103A8
  uint32_t field_103AC_;  // 0x103AC
  uint32_t field_103B0_;  // 0x103B0
  uint32_t field_103B4_;  // 0x103B4
  uint32_t field_103B8_;  // 0x103B8
  uint32_t field_103BC_;  // 0x103BC
  uint32_t field_103C0_;  // 0x103C0
  uint32_t field_103C4_;  // 0x103C4
  uint32_t field_103C8_;  // 0x103C8
  uint32_t field_103CC_;  // 0x103CC
  uint32_t field_103D0_;  // 0x103D0
  uint32_t field_103D4_;  // 0x103D4
  uint32_t field_103D8_;  // 0x103D8
  uint32_t field_103DC_;  // 0x103DC
  uint32_t field_103E0_;  // 0x103E0
  uint32_t field_103E4_;  // 0x103E4
  uint32_t field_103E8_;  // 0x103E8
  uint32_t field_103EC_;  // 0x103EC
  uint32_t field_103F0_;  // 0x103F0
  uint8_t field_103F4_[0x70];  // 0x103F4
  uint32_t field_10464_;  // 0x10464
  uint32_t field_10468_;  // 0x10468
  uint32_t field_1046C_;  // 0x1046C
  uint32_t field_10470_;  // 0x10470
  uint32_t field_10474_;  // 0x10474
  uint32_t field_10478_;  // 0x10478
  uint32_t field_1047C_;  // 0x1047C
  uint32_t field_10480_;  // 0x10480
  uint8_t field_10484_[0x70];  // 0x10484
  uint32_t field_104F4_;  // 0x104F4
  uint32_t field_104F8_;  // 0x104F8
  uint32_t field_104FC_;  // 0x104FC
  uint32_t field_10500_;  // 0x10500
  uint32_t field_10504_;  // 0x10504
  uint32_t field_10508_;  // 0x10508
  uint32_t field_1050C_;  // 0x1050C
  uint32_t field_10510_;  // 0x10510
  uint32_t field_10514_;  // 0x10514
  uint8_t field_10518_[0x440];  // 0x10518
  uint32_t field_10958_;  // 0x10958
  uint8_t field_1095C_[0x2750];  // 0x1095C
  uint32_t field_130AC_;  // 0x130AC
  uint32_t field_130B0_;  // 0x130B0
  uint32_t field_130B4_;  // 0x130B4
  uint8_t field_130B8_[0x440];  // 0x130B8
  uint32_t field_134F8_;  // 0x134F8
  uint8_t field_134FC_[0x2750];  // 0x134FC
  uint32_t field_15C4C_;  // 0x15C4C
  uint32_t field_15C50_;  // 0x15C50
  uint32_t field_15C54_;  // 0x15C54
  uint8_t field_15C58_[0x440];  // 0x15C58
  uint32_t field_16098_;  // 0x16098
  uint8_t field_1609C_[0x2750];  // 0x1609C
  uint32_t field_187EC_;  // 0x187EC
  uint32_t field_187F0_;  // 0x187F0
  uint32_t field_187F4_;  // 0x187F4
  uint8_t field_187F8_[0x440];  // 0x187F8
  uint32_t field_18C38_;  // 0x18C38
  uint8_t field_18C3C_[0x2750];  // 0x18C3C
  uint32_t field_1B38C_;  // 0x1B38C
  uint32_t field_1B390_;  // 0x1B390
};

static_assert(sizeof(Datalink) == 0x1B394,
              "Datalink layout must match the original executable");

int __fastcall datalink_unk1_redirect(Datalink *self, void *, int a1, int a2);
void __fastcall datalink_parse_id_redirect(Datalink *self, void *, int id, DatalinkID *out_id, int *out_remainder);
void __fastcall datalink_unk6_redirect(Datalink *self, void *);
void __fastcall datalink_unk8_redirect(Datalink *self, void *);
void __fastcall datalink_unk9_redirect(Datalink *self, void *);
void __fastcall datalink_on_left_click_redirect(Datalink *self, void *, int a1, int a2);
void __fastcall datalink_on_group_clicked_redirect(Datalink *self, void *, int a1, int a2);
void __fastcall datalink_on_mouse_move_redirect(Datalink *self, void *, int a1, int a2);
void __fastcall datalink_on_iface_left_click_redirect(Datalink *self, void *, int a1, int a2);
void __fastcall datalink_on_iface_right_click_redirect(Datalink *self, void *, int a1, int a2);
void __fastcall datalink_on_iface_left_down_redirect(Datalink *self, void *, int a1, int a2);
void __fastcall datalink_on_iface_right_down_redirect(Datalink *self, void *, int a1, int a2);
void __fastcall datalink_on_iface_mouse_move_redirect(Datalink *self, void *, int a1, int a2);
void __fastcall datalink_on_iface_mouse_leave_redirect(Datalink *self, void *, int a1, int a2);
void __fastcall datalink_on_iface_selected_redirect(Datalink *self, void *, int a1, int a2);
void __fastcall datalink_on_iface_left_double_click_redirect(Datalink *self, void *, int a1, int a2);
void __fastcall datalink_on_iface_right_double_click_redirect(Datalink *self, void *, int a1, int a2);
void __fastcall datalink_on_iface_button_clicked_redirect(Datalink *self, void *, int a1);
void __fastcall datalink_on_iface_button_toggled_redirect(Datalink *self, void *, int a1, int a2);
void __fastcall datalink_close_redirect(Datalink *self, void *);

// Datalink::exec (thiscall, `void exec(unsigned int topic, int index)`) is not
// yet source-owned; every help_* forwarder below dispatches through it
// against the fixed-address Datalink singleton, so both the exec entry point
// and the singleton object are seams here rather than baked-in literals.

// The Datalink singleton the forwarders dispatch against - plain data, not a
// call target, so it stays an unclassified rebindable seam rather than a
// classified dependency.
extern Datalink *DatalinkMain;

// Datalink::draw_entry (0x0042BF10) is likewise unrecovered; on_selected calls
// it, so its definition is a seam into the original image.
typedef void (OriginalObject::*func_datalink_draw_entry)();
extern func_datalink_draw_entry DatalinkDrawEntry;

void __cdecl help_tech(int id);
void __cdecl help_weapon(int id);
void __cdecl help_armor(int id);
void __cdecl help_chassis(int id);
void __cdecl help_facility(int id);
void __cdecl help_abil(int id);
void __cdecl help_social(int id);
void __cdecl help_faction(int id);
void __cdecl help_veh(int id);
void __cdecl help_topic(unsigned int topic, int index);
