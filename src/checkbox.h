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


// The original vbtable at 0x00670718 describes GraphicWin at +0x1C and
// Dialog at +0xA34. VC6 owns the virtual-base setup and displacement fields;
// no embedded substitutes or hand-installed table fields belong here.
// The following experiment predates the current nonvirtual close() and
// compiler-owned displacement fields; retain its caller-convention evidence,
// not its superseded declaration prescription (see GraphicWin's declarations).
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
  // The catalogue's @H@Z at 0x0060E670 describes VC6's hidden most-derived
  // flag, not a source parameter. The emitted symbol is mapped in the TU.
  // Historical rejected interpretation below; superseded by the real bases.
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
  // ??0CheckBox@@QAE@H@Z, the bare real constructor - the catalogue's H is
  // the hidden most-derived flag (the MapWin finding), arriving at [ebp+8]
  // where the image reads it. The compiler owns the virtual-base
  // construction: with GraphicWin() and Dialog() both real constructors,
  // the implicit machinery builds both bases under the flag - the hand
  // placement-new + Dialog::construct() calls and their guard are gone.
  CheckBox();
  // The image has no separate `??0CheckBox@@QAE@XZ` - every construction
  // (NetWin::checkBox_'s embed included) goes through the one bare ctor
  // above.
  ~CheckBox();
  uint32_t close();
  void UNK1(int pos);
  int UNK2(int pos);
  void set_state_pos(int pos, int state);
  void set_state_id(int id, int value);
  // 0x0060EC10, defined in src/checkbox.cpp beside set_state_id above - the
  // read-side twin: walks the same list for `id` and returns
  // `(1 << bit) & flags`, nonzero when the box with this id is checked.
  // `int`, not `bool`: DipEdit::read_check tests the result with
  // `test eax, eax`, which is the dword-wide form.
  int get_state_id(int id);
  // 0x0060ED10 ?calculate_dimensions@CheckBox@@QAEHXZ, still FILE-only in
  // src/unrecovered/0060ed10.cpp. DECLARATION ONLY, added because Dialogs
  // ::calculate_dimensions (0x006132A0) tail-calls it for kind 1 - the access
  // costs nothing and the definition stays where it is.
  int calculate_dimensions();
  // ?item@CheckBox@@QAEHPADHH@Z at 0x0060E980, still a pending_bodies
  // forwarder. Adds `text` to the box's item list for `id` and sets or clears
  // the box's own state bit from `state`. DipEdit::init calls it once per
  // treaty flag with the label string.
  int item(char *text, int id, int state);

 private:
  // The vbtable pointer is EMITTED by the compiler now.
  uint32_t field_4_;  // 0x0004
  uint32_t field_8_;
 public:
  // PUBLIC, like the sprites below: BasePop::calculate_dimensions installs the
  // popup's double-click handler straight into it (Dialogs+0x2234, the check
  // box kind's notification slot) with no call in between, so the access has
  // to be spelled from outside the class. Access is compile-time only; the
  // static_assert below pins the layout.
  uint32_t field_C_;
 public:
  // DipEdit::init substitutes the shared interface array's elements 1 and 2
  // at 0x004DB457 / 0x004DB45E, so these pointers are caller-accessible.
  // draw_item selects +0x14 for a nonzero state argument and +0x10 for
  // zero (0x0060F3C8-0x0060F457), then calls Sprite::draw on that receiver.
  // RECOVERY_MEMBER: v3 CheckBox unchecked_sprite_ Sprite* 0x10 0x0060F030
  Sprite *unchecked_sprite_;
  // RECOVERY_MEMBER: v3 CheckBox checked_sprite_ Sprite* 0x14 0x0060F030
  Sprite *checked_sprite_;
 private:
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

// Rebindable defaults, backed by original pointer-cell bindings in the TU.
extern Sprite *CheckBoxDefault1;
extern Sprite *CheckBoxDefault2;
