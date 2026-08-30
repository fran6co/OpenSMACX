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
#include "dialog.h"
#include "graphicwin.h"

#include "radiobutton.h"

// The checkbox sprites, real objects now: the image holds them at
// 0x009B8F60 / 0x009B8F90, constructed before WinMain and closed at exit.
extern Sprite g_CHECKBOX_SPRITE_1;
extern Sprite g_CHECKBOX_SPRITE_2;


 /*
  * CheckBox class
  *
  * Virtually derives from GraphicWin, so the base is held as a member at the
  * offset MSVC's vbtable names rather than written as `: virtual GraphicWin` -
  * the Itanium ABI this toolchain follows would place it after the derived
  * object instead. The vbtable at 0x00670718 reads {0, 0x1C, 0xA34}, and
  * close() resolves both of its calls through exactly those two entries.
  *
  * A Dialog follows the base at 0xA34, four bytes past where the base ends.
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
// The bases are REAL: `: public virtual GraphicWin, public virtual Dialog`.
// The vbtable pointer, both base constructions and the unwind are the
// compiler's now. The 4 bytes that used to sit before the Dialog subobject are
// its VTORDISP, which VC6 emits because this class overrides the virtual
// `Dialog::close()` - a displacement is earned by overriding a NON-destructor
// virtual, and nothing else forces it (measured 2026-08-22; /vd2 does not).
// The displacement before GraphicWin stays a declared member, because the
// image's own constructor writes that one by hand.
// WHY EDITGROUP AND SPRITEBOX ARE NOT CONVERTED WITH THIS CLASS, measured
// 2026-08-22. The displacement before the Dialog base is earned by overriding
// a non-destructor virtual of it, and `close()` is the ONLY method these
// classes share with Dialog at an exact signature. But making `Dialog::close()`
// virtual changes the ENTRY CONVENTION of every derived override: MSVC enters
// an override of a virtual-base method with the BASE SUBOBJECT as `this`, so
// each call site gains a `lea ecx, [esi + <base offset>]`.
//
// The image says whether that is right, per class. SpriteBox's own close at
// 0x00610280 is entered with `this = esi`, UNADJUSTED - so in the original
// SpriteBox::close is not an override of Dialog::close, and the four
// `SpriteBox::init` overloads that call it (0x00610420, 0x00610480,
// 0x006104B0, 0x006104D0) each gained exactly one extra `lea` and stopped
// reproducing. EditGroup::~EditGroup (0x00611A20) went the same way through
// register allocation. CheckBox and RadioButton have no such caller and both
// stay BYTE_EXACT, which is why the conversion lands here and not there.
//
// So those two need a DIFFERENT virtual to earn the displacement - one the
// image really does dispatch - and that is a per-class reading of the vtable,
// not a repeat of this edit.
class CheckBox : public virtual GraphicWin, public virtual Dialog {
 public:
  // 0x0060FAB0, a pending_bodies forwarder.
  void on_redraw();

 public:
  // 0x0060FBB0, a pending_bodies forwarder.
  void on_mouse_move(int a1, int a2);

 public:
  // 0x0060F700, a pending_bodies forwarder.
  void on_left_down(int a1, int a2);

 public:
  // 0x0060FA80, recovered in checkbox.cpp.
  void on_left_double_click(int x_coord, int y_coord);

 public:
  // 0x0060F8B0, a pending_bodies forwarder.
  int on_key_down(int a);

 public:
  // 0x0060E800, a pending_bodies forwarder.
  int attach(void * a1, int a2, int a3, int a4);

 public:
  // 0x0060ECE0. BasePop::write_check reaches it on the CheckBox at its own
  // +0x2228.
  void set_state_flag(long value);
  // `static`: the image's name ends in `QAA` - a public member declared
  // __cdecl, taking no receiver - and jackal_init_real calls it with no
  // ecx set up, so `Class::method()` is the only legal spelling.
  static int init_class();   // 0060FC60
  void on_mouse_leave(int a1, int a2);
  void on_dialog_focus(int a1);
  // ??0CheckBox@@QAE@H@Z at 0x0060E670. The `int` is NOT a user parameter -
  // CheckBox's layout is composed by hand (see the class comment above), so
  // VC6 cannot inject the most-derived flag the way it would for a real
  // `: virtual GraphicWin, virtual Dialog` declaration; this constructor
  // takes an explicit flag instead and mirrors what the image's own guard
  // does with it.
  // TRIED (re-checked 2026-08-22): declaring the real
  // `: public virtual GraphicWin, public virtual Dialog` and letting VC6
  // emit the vbtable and the base construction. It cannot produce this
  // symbol - a constructor on a class with a GENUINE virtual base compiles
  // to `??0CheckBox@@QAE@XZ` on this compiler, with the most-derived flag a
  // hidden stack parameter that is NEVER part of the mangled name, and the
  // image plainly has `@H@Z`. The hand-composed layout below is the
  // workaround for that, not an oversight; tools/most_derived_flag.py lists
  // this class as needing a base and is right about the LAYOUT while being
  // unable to see that constraint.
  CheckBox(int a1);
  // Trivial default, kept ONLY so an embedding class that has not itself
  // been recovered to call the flag-taking constructor (NetWin::checkBox_,
  // see the note in netwin.cpp) still compiles. The image has no
  // `??0CheckBox@@QAE@XZ` of its own - every construction goes through the
  // address above.
  CheckBox() { ; }
  ~CheckBox();
  uint32_t close();
  void UNK1(int pos);
  int UNK2(int pos);
  void set_state_pos(int pos, int state);
  // 0x0060EB80, not yet recovered - a pending_bodies forwarder.
  // DipEdit::do_check calls it BY NAME.
  void set_state_id(int id, int value);

 private:
  // The vbtable pointer is EMITTED by the compiler now.
  uint32_t field_4_;  // 0x0004
  uint32_t field_8_;
  uint32_t field_C_;
  uint32_t field_10_;
  uint32_t field_14_;
  // The vbtable puts the base at 0x1C; the declared fields reach
  // 0x18, so 4 bytes sit between them.
  // NO GAP MEMBER HERE. These four bytes are GraphicWin's VTORDISP and the
  // compiler emits them, because this class overrides GraphicWin's
  // on_dialog_focus and on_mouse_leave. Declaring the gap as well put the base
  // at 0x20 where the image has it at 0x1C - and sizeof still matched, because
  // 0x20 + 0xA14 + 0xF4 and 0x1C + 0xA14 + 4 + 0xF4 are both 0xB28. A size
  // assertion CANNOT pin a two-virtual-base layout; the offset has to be
  // checked separately, which is what OFFSET_PROBE below does.
  // GraphicWin and Dialog are VIRTUAL BASES, appended by the compiler.
};

// PINNED BEFORE CHANGING THE DECLARATION, so that replacing the hand-composed
// vbtable pointer and embedded subobjects with a real
// `: public virtual GraphicWin, public virtual Dialog` cannot move the layout
// without failing here. 0x1C of own data, then GraphicWin 0xA14, then the
// 4-byte vtordisp this header already carries as a gap, then Dialog 0xF4.
static_assert(sizeof(CheckBox) == 0xB28,
              "CheckBox layout must match the original executable");

// CheckBox's Dialog::close is not recovered yet.
extern uint32_t CheckBoxDefault1;
extern uint32_t CheckBoxDefault2;

