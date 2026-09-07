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

#include "sprite.h"

// The radiobutton sprites, real objects now: the image holds them at
// 0x009B8EF8 / 0x009B8F28.
extern Sprite g_RADIOBUTTON_SPRITE_1;
extern Sprite g_RADIOBUTTON_SPRITE_2;
#include "dialog.h"   // func_dialog_close
#include "graphicwin.h"


// The original vbtable at 0x00670590 describes GraphicWin at +0x18 and
// Dialog at +0xA30. VC6 owns the virtual-base setup; no embedded substitutes
// or hand-installed table fields belong in the declaration.
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
  // 0x0060E1E0, recovered in radiobutton.cpp.
  void on_left_double_click(int x_coord, int y_coord);

 public:
  // 0x0060E020, a pending_bodies forwarder.
  int on_key_down(int a);

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
  // 0x0060D380 ?calculate_dimensions@RadioButton@@QAEHXZ, still FILE-only in
  // src/unrecovered/0060d380.cpp. DECLARATION ONLY, added because Dialogs
  // ::calculate_dimensions (0x006132A0) tail-calls it for kind 0x10 - the
  // access costs nothing and the definition stays where it is.
  int calculate_dimensions();
  // The catalogue's @H@Z at 0x0060D0E0 describes VC6's hidden most-derived
  // flag, not a source parameter. The emitted symbol is mapped in the TU.
  RadioButton();
  // VIRTUAL, and that is what earns the vtordisp. A displacement is emitted
  // for a virtual base only when the derived class OVERRIDES one of its
  // virtuals; inheriting Dialog's destructor implicitly is not enough for
  // VC6. The image has ??1RadioButton@@QAE@H@Z at 0x00406F60, so a real
  // destructor is what belongs here regardless.
  virtual ~RadioButton() { ; }
  uint32_t close();

 public:
  // draw_item selects +8 for a nonzero state argument and +4 for zero
  // (0x0060DAE8-0x0060DB77), then calls Sprite::draw on that receiver.
  // Popup::on_post_window_init clears both to suppress the sprite pair.
  // RECOVERY_MEMBER: v3 RadioButton unchecked_sprite_ Sprite* 0x4 0x0060D750
  Sprite *unchecked_sprite_;
  // RECOVERY_MEMBER: v3 RadioButton checked_sprite_ Sprite* 0x8 0x0060D750
  Sprite *checked_sprite_;
 private:
  uint32_t field_C_;
 public:
  // PUBLIC, like CheckBox's sprite pointers for DipEdit::init: BasePop
  // ::calculate_dimensions installs the popup's double-click handler straight
  // into it (Dialogs+0x2224, the push-button kind's notification slot) with
  // no call in between, so the access has to be spelled from outside the
  // class. Access is compile-time only; the static_assert below pins the
  // layout.
  uint32_t field_10_;
  // NO GAP MEMBER: those four bytes are GraphicWin's VTORDISP and the compiler
  // emits them, because this class overrides GraphicWin's on_dialog_focus and
  // on_mouse_leave. Declaring the gap as well pushes the base to 0x1C where
  // the image has it at 0x18 - and sizeof still matches, which is why a size
  // assertion alone cannot pin a two-virtual-base layout.
};

// Complete-object extent: own data and compiler displacement to +0x18,
// GraphicWin (0xA14), another compiler displacement, then Dialog (0xF4).
// Extent alone does not verify base offsets or virtual dispatch.
static_assert(sizeof(RadioButton) == 0xB24,
              "RadioButton layout must match the original executable");

// Rebindable defaults, backed by original pointer-cell bindings in the TU.
extern Sprite *RadioButtonDefault1;
extern Sprite *RadioButtonDefault2;


// Unfinished teardown adapter for 0x00406F60, entered on the GraphicWin
// subobject at +0x18. Its explicit teardown and optional deletion still need
// recovery as compiler-owned destruction; the catalogue signature is not a
// reason to preserve this shim or to add a source-level destructor parameter.
RadioButton *__fastcall radio_button_teardown_redirect(void *adjusted, void *,
                                                        unsigned int mode);
