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
// Generic Jackal control initialized by jackal_init_real.

#include "sprite.h"
#include <new>
#include <stddef.h>

// The radiobutton sprites, real objects now: the image holds them at
// 0x009B8EF8 / 0x009B8F28.
// Chosen private lifetime representation, not an original class-name claim.
// The original registered exits share this independent byte, setting their
// own bit before Sprite::close. Raw storage prevents duplicate implicit cleanup.
extern uint8_t RadioButtonSpritesCleanupGuard;
union RadioButtonSpriteStorage {
  char bytes[sizeof(Sprite)];
  uint32_t alignment_[sizeof(Sprite) / sizeof(uint32_t)];
};
template<unsigned char Mask> struct RadioButtonSpriteLifetime {
  RadioButtonSpriteStorage storage_;
  RadioButtonSpriteLifetime() { new (storage_.bytes) Sprite(); }
  ~RadioButtonSpriteLifetime() {
    if ((RadioButtonSpritesCleanupGuard & Mask) != 0) return;
    RadioButtonSpritesCleanupGuard |= Mask;
    // The actual Sprite destructor is solely close(); qualification ends
    // the placed object's lifetime without any second implicit destruction.
    sprite().Sprite::~Sprite();
  }
  Sprite &sprite() { return *reinterpret_cast<Sprite *>(storage_.bytes); }
};
static_assert(sizeof(RadioButtonSpriteLifetime<1>) == sizeof(Sprite) &&
              sizeof(RadioButtonSpriteLifetime<2>) == sizeof(Sprite),
              "radio Sprite lifetime storage extent");
static_assert(offsetof(RadioButtonSpriteLifetime<1>, storage_) == 0 &&
              offsetof(RadioButtonSpriteLifetime<2>, storage_) == 0,
              "radio Sprite starts at owner offset zero");
extern RadioButtonSpriteLifetime<1> g_RADIOBUTTON_SPRITE_1;
extern RadioButtonSpriteLifetime<2> g_RADIOBUTTON_SPRITE_2;
#include "dialog.h"   // func_dialog_close
#include "graphicwin.h"


// The original vbtable at 0x00670590 describes GraphicWin at +0x18 and
// Dialog at +0xA30. VC6 owns the virtual-base setup; no embedded substitutes
// or hand-installed table fields belong in the declaration.
class RadioButton : public virtual GraphicWin, public virtual Dialog {
 public:
  // Seed identity: ?draw_item@RadioButton@@QAEXHH@Z at 0x0060D750.
  // The second explicit argument is both the selected-state value and the
  // focus/current-row state used by the original draw path.
  void draw_item(int position, int state);

 public:
  // 0x0060E220: implemented in radiobutton.cpp.
  void on_redraw();

 public:
  // 0x0060E320: implemented in radiobutton.cpp.
  void on_mouse_move(int a1, int a2);

 public:
  // 0x0060DE10: implemented in radiobutton.cpp.
  void on_left_down(int a1, int a2);

 public:
  // 0x0060E1E0, recovered in radiobutton.cpp.
  void on_left_double_click(int x_coord, int y_coord);

 public:
  // 0x0060E020, the typed RadioButton body is defined in radiobutton.cpp.
  int on_key_down(int a) override;

 public:
  // 0x0060D1F0: complete attach behavior is owned in radiobutton.cpp.
  int attach(GraphicWin * a1, int a2, int a3, int a4);

 public:
  // `static`: the image's name ends in `QAA` - a public member declared
  // __cdecl, taking no receiver - and jackal_init_real calls it with no
  // ecx set up, so `Class::method()` is the only legal spelling.
  static int init_class();   // 0060E4D0
  void on_mouse_leave(int a1, int a2);
  void on_dialog_focus(int a1);
  // 0x0060D380: complete dimensions behavior is owned in radiobutton.cpp;
  // Dialogs::calculate_dimensions (0x006132A0) calls it for kind 0x10.
  int calculate_dimensions();
  // The catalogue's @H@Z at 0x0060D0E0 describes VC6's hidden most-derived
  // flag, not a source parameter. The emitted symbol is mapped in the TU.
  RadioButton();
  // The original deleting entry closes the control before destroying its
  // virtual bases. VC6 owns their destruction, optional operator delete,
  // and all three receiver adjustors; their image bindings live in the TU.
  virtual ~RadioButton();
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
  // BasePop::calculate_dimensions installs its static notification here;
  // the radio entry invokes it with Dialog::get_selected_id().
  // RECOVERY_MEMBER: v1 RadioButton double_click_callback_ 0x10 0x0060E1E0
  WinNotificationCallback double_click_callback_;  // 0x10
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
