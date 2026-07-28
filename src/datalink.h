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

class DLLEXPORT Datalink : GraphicWin {
 public:
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
  uint8_t unmapped_A14_[0x29E0 - 0xA14];
  int32_t field_29E0_;
  uint8_t unmapped_29E4_[0x2A34 - 0x29E4];
  int32_t field_2A34_;
  int32_t field_2A38_;
};

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
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
#endif
typedef void(__thiscall func_datalink_exec)(void *datalink, unsigned int topic,
                                            int index);
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
extern func_datalink_exec *DatalinkExec;

// The Datalink singleton the forwarders dispatch against - plain data, not a
// call target, so it stays an unclassified rebindable seam rather than a
// classified dependency.
extern void *DatalinkMain;

DLLEXPORT void __cdecl help_tech(int id);
DLLEXPORT void __cdecl help_weapon(int id);
DLLEXPORT void __cdecl help_armor(int id);
DLLEXPORT void __cdecl help_chassis(int id);
DLLEXPORT void __cdecl help_facility(int id);
DLLEXPORT void __cdecl help_abil(int id);
DLLEXPORT void __cdecl help_social(int id);
DLLEXPORT void __cdecl help_faction(int id);
DLLEXPORT void __cdecl help_veh(int id);
DLLEXPORT void __cdecl help_topic(unsigned int topic, int index);
