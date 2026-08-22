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
#include "dialog.h"   // func_dialog_close
#include "graphicwin.h"


 /*
  * RadioButton class
  *
  * Virtually derives from GraphicWin, so the base is held as a member at the
  * offset MSVC's vbtable names rather than written as `: virtual GraphicWin` -
  * the Itanium ABI this toolchain follows would place it after the derived
  * object instead. The vbtable at 0x00670590 reads {0, 0x18, 0xA30}, and
  * close() resolves both of its calls through exactly those two entries.
  *
  * A Dialog follows the base at 0xA30, four bytes past where the base ends.
  * Nothing pins this sizeof: no method touches past the Dialog, but unlike
  * MapWin and Console there is no global instance to bound the object from
  * above, and "nothing touches it" would not notice a trailing field that is
  * never used.
  *
  *   * MEASURED under VC6 12.00.8168, and the conclusion is OPEN, not closed.
  * `: virtual GraphicWin, virtual Dialog` gives 0xB4C against the 0xB54 this
  * class needs, and adding a virtual function to the DERIVED class gives
  * 0xB50 - that +4 is its own vfptr. An earlier note here blamed vtordisp
  * fields; that was WRONG and is withdrawn. A probe settles it: two bases
  * with a virtual destructor grow 0x10 -> 0x14 EACH, and the derived total
  * moves 0x34 -> 0x3C, so the missing 8 bytes are the two BASES' own vtable
  * pointers, not a displacement.
  *
  * Which means the conversion is probably reachable and is blocked on
  * something specific: GraphicWin and Dialog model their vtable as an opaque
  * dword the original installs by hand, so neither declares a virtual. Giving
  * them real virtual destructors is a coupled edit - the compiler then emits
  * the vfptr, so the modelled dword has to come out in the same change - and
  * `sizeof(GraphicWin) == 0xA14` is the check that would catch it going wrong.
  * Do NOT read this paragraph as "cannot be done".
  */
class RadioButton : public virtual GraphicWin, public virtual Dialog {
 public:
  // 0x0060E220, a pending_bodies forwarder.
  void on_redraw();

 public:
  // 0x0060E320, a pending_bodies forwarder.
  void on_mouse_move(int a1, int a2);

 public:
  // 0x0060DE10, a pending_bodies forwarder.
  void on_left_down(int a1, int a2);

 public:
  // 0x0060E1E0, a pending_bodies forwarder.
  void on_left_double_click(int a1, int a2);

 public:
  // 0x0060E020, a pending_bodies forwarder.
  int on_key_down(int a1);

 public:
  // 0x0060D1F0, a pending_bodies forwarder.
  int attach(void * a1, int a2, int a3, int a4);

 public:
  // `static`: the image's name ends in `QAA` - a public member declared
  // __cdecl, taking no receiver - and jackal_init_real calls it with no
  // ecx set up, so `Class::method()` is the only legal spelling.
  static int init_class();   // 0060E4D0
  void on_mouse_leave(int a1, int a2);
  void on_dialog_focus(int a1);
  // ??0RadioButton@@QAE@H@Z at 0x0060D0E0. The `int` is NOT a user
  // parameter - RadioButton's layout is composed by hand (see the class
  // comment above), so VC6 cannot inject the most-derived flag the way it
  // would for a real `: virtual GraphicWin, virtual Dialog` declaration;
  // this constructor takes an explicit flag instead and mirrors what the
  // image's own guard does with it.
  RadioButton(int a1);
  // VIRTUAL, and that is what earns the vtordisp. A displacement is emitted
  // for a virtual base only when the derived class OVERRIDES one of its
  // virtuals; inheriting Dialog's destructor implicitly is not enough for
  // VC6. The image has ??1RadioButton@@QAE@H@Z at 0x00406F60, so a real
  // destructor is what belongs here regardless.
  virtual ~RadioButton() { ; }
  void close();

 private:
  uint32_t field_4_;
  uint32_t field_8_;
  uint32_t field_C_;
  uint32_t field_10_;
  // The vbtable puts the base at 0x18; the declared fields reach
  // 0x14, so 4 bytes sit between them.
  uint8_t gap_14_[0x18 - 0x14];
};

// PINNED BEFORE CHANGING THE DECLARATION, so that replacing the hand-composed
// vbtable pointer and embedded subobjects with a real
// `: public virtual GraphicWin, public virtual Dialog` cannot move the layout
// without failing here. 0x18 of own data, then GraphicWin 0xA14, then the
// 4-byte vtordisp this header already carries as a gap, then Dialog 0xF4.
static_assert(sizeof(RadioButton) == 0xB24,
              "RadioButton layout must match the original executable");

// RadioButton's Dialog::close is not recovered yet.
extern uint32_t RadioButtonDefault1;
extern uint32_t RadioButtonDefault2;

void __fastcall radio_button_close_redirect(RadioButton *self, void *);

// ??1RadioButton@@QAE@H@Z at 0x00406F60 - the DESTRUCTOR entry that also
// conditionally frees the storage. C++ forbids a user-declared destructor
// from taking a parameter (that spelling is reserved for the compiler's own
// virtual-base-destroy flag, and RadioButton cannot get VC6 to synthesise
// one - see the constructor's own note above), so this is an ordinary
// __fastcall free function with the same (adjusted-this, unused, mode) shape
// dialogs_scalar_dtor_redirect (src/dialogs.cpp) already uses for the
// identical situation on a real `??_G` scalar deleting destructor. Entered
// on `this - 0x18` (the GraphicWin-subobject-adjusted pointer), matching
// close()'s own vbtable-relative arithmetic.
RadioButton *__fastcall radio_button_teardown_redirect(void *adjusted, void *,
                                                        unsigned int mode);
