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
#include "time.h"

class ButtonGroup; // forward declaration
class Scroll; // forward declaration

 /*
  * BaseButton class
  */
// The zero-init .bss slots BaseButton's setup reads its field defaults from
// (0x009B8E2C/0x009B8E30), rebindable like CheckBoxDefault1/2.
extern uint32_t BaseButtonA94Default;  // 0x009B8E2C
extern uint32_t BaseButtonA98Default;  // 0x009B8E30

class BaseButton : public GraphicWin {
  friend class ButtonGroup;
  friend class FlatButton;
  friend class Scroll;
  friend class PrefWin;
  friend class DesignWin;

 public:
  virtual void on_dialog_focus(int focus);
  void on_left_down(int x, int y);
  void on_left_up(int x, int y);
  void on_left_click(int x, int y) override;
  void on_mouse_move(int x, int y);
  void on_right_click(int a, int b) override;
  // 0x00607B30  ?timer_callback@BaseButton@@QAEXH@Z - public, __thiscall,
  // void(int). Public is load-bearing: the non-member daemon fn_00607b10
  // reaches it through a BaseButton * it was handed.
  // BaseWin::timer_callback's `(int key, int context)` is a different arity
  // on a different class and is not evidence for this callback's argument.
  void timer_callback(int a1);
  // These return `int`, not `void`. Each body is `xor eax, eax; ret N`, and
  // the sibling class BasePop declares the same three handlers `int`. The
  // catalogued mangled names spelled `X` and are corrected in
  // tools/catalogue_corrections.py.
  int on_key_click(int a, int b);
  int on_key_down(int a);
  int on_key_up(int a);
  // OUT OF LINE (defined in basebutton.cpp), 2026-09-03, superseding the
  // in-class `{ ; }`. An in-class constructor is expanded by /Ob2 at every
  // implicit base construction, and because that expansion holds the two
  // Time member constructions VC6 wraps the whole site in an SEH frame the
  // image does not pay: ??0ImageButton@@QAE@XZ compiled to 25 instructions
  // of frame-and-inline where the image makes one `call ??0BaseButton`.
  // Out of line there is nothing to fold, so the derived site becomes the
  // call. Same lever as ~Buffer (buffer.h) and Heap's constructor (heap.h).
  BaseButton();
  ~BaseButton();

  BaseButton *construct();
  // BaseButton introduces slot 90 after GraphicWin's 88/89. Constructor
  // 606F77 installs table 670290, whose +168 entry selects close6070C0;
  // init607217 dispatches it before testing its arguments. Derived buttons
  // override this cleanup; GraphicWin::close itself is not virtual.
  virtual void close();
  virtual void hide();
  int init(LPCSTR name, int id, int x, int y, int width, int height,
           Win *parent, int style_flag);
  int UNK1(Font *font, char *name, int id, int x, int y, int width,
           Win *parent, int style_flag);
  void UNK2();
  void on_mouse_leave(int x, int y);
  BaseButton *destroy();
  void set_text_color(int color1, int color2, int color3, int color4);
  void set_text_color2(int color1, int color2, int color3, int color4);
  void set_text_color3(int color1, int color2, int color3, int color4);
  int set_bubble_text(LPCSTR input);
  int set_name(LPCSTR input);
  void set(int value);
  // AlphaMenu toggles the visual pressed state of its owner button while the
  // menu is shown/hidden. Keep the legacy word encapsulated in BaseButton.
  void set_pressed_visual(int value) { field_A9C_ = static_cast<uint32_t>(value); }
  int selected_state() const { return static_cast<int>(field_A18_); }
  // Original BaseWin init41FB4B writes this still-unnamed button state.
  void set_field_A14(int value) { field_A14_ = static_cast<uint32_t>(value); }
  // The base governor panel clears this auxiliary state when a priority
  // becomes unavailable, then repaints through Win's virtual redraw slot.
  void clear_auxiliary_state_for_disabled_button() {
    if (field_A74_ != 0) {
      field_A74_ = 0;
      redraw();
    }
  }
  // ReportIf::close_ops (0x004A9F90) clears this established state word
  // only when the button is not attached to a ButtonGroup. Keep both fields
  // encapsulated here rather than exposing their legacy storage offsets.
  void reset_field_A9C_if_ungrouped() {
    if (group_ == 0) {
      field_A9C_ = 0;
    }
  }

  // Static defaults shared by every button; the legacy bodies are __cdecl
  // rather than __thiscall because they take no instance.
  static void set_def_text_color(int color1, int color2, int color3, int color4);
  static void set_def_text_color2(int color1, int color2, int color3, int color4);
  static void set_def_text_color3(int color1, int color2, int color3, int color4);
  static int set_def_font(Font *font1, Font *font2, Font *font3);

 private:
  uint32_t field_A14_;  // 0x0a14
  uint32_t field_A18_;
  Time time1_;
  uint32_t field_A44_;
  uint32_t field_A48_;
  Time time2_;
  uint32_t field_A74_;
  uint32_t field_A78_;
 protected:
  // Derived painters read the live label after drawing the button surface.
  // RECOVERY_MEMBER: v1 BaseButton name_ 0xA7C 0x006253D0
  LPSTR name_;
 private:
  LPSTR bubble_text_;
 public:
  // THE COLOUR-AND-BEVEL RUN AT 0xA84..0xA9C, PUBLIC so BasePop::exec's
  // apply_button_bevel can name the words instead of walking the object
  // address. The image writes all six at each of its three button sites
  // (0x00602EF6, 0x00603291, 0x006035DC), and the constructor seeds them from
  // DialogDefaults[0..3] and the two BaseButtonA94/A98 defaults. Access is
  // compile-time only: the members keep their declaration order and offsets,
  // which is what pins the layout.
  // RECOVERY_MEMBER: v1 BaseButton color_ 0xA84 0x006253D0
  uint32_t color_;
  // RECOVERY_MEMBER: v1 BaseButton bevel_upper_ 0xA88 0x006253D0
  uint32_t bevel_upper_;
  // RECOVERY_MEMBER: v1 BaseButton bevel_lower_ 0xA8C 0x006253D0
  uint32_t bevel_lower_;
  // RECOVERY_MEMBER: v1 BaseButton bevel_thickness_ 0xA90 0x006253D0
  uint32_t bevel_thickness_;
  // RECOVERY_MEMBER: v1 BaseButton field_A94_ 0xA94 0x006253D0
  uint32_t field_A94_;
  uint32_t field_A98_;
 protected:
  // ImageButton's painter selects the raised/depressed bevel from this word.
  // RECOVERY_MEMBER: v1 BaseButton field_A9C_ 0xA9C 0x006253D0
  uint32_t field_A9C_;
 private:
  uint32_t field_AA0_;
  uint32_t field_AA4_;
  ButtonGroup *group_;
  uint32_t field_AAC_;
  uint32_t field_AB0_;
  uint32_t field_AB4_;
};

static_assert(sizeof(BaseButton) == 0xAB8,
              "BaseButton layout must match the legacy ABI");

// Same storage as DialogDefaults[0..4], not a second table.
extern uint32_t DialogDefaults[37];
uint32_t *const BaseButtonStaticDefaults = DialogDefaults;
// 0x009B8E2C's two slots are named below (BaseButtonA94Default /
// BaseButtonA98Default); the old `uint32_t *const BaseButtonDynamicDefaults`
// pointer binding that stood here had zero users and is gone.

// Interleaved 3x4 default colour table and the three default font slots the
// setters publish and init consumes.
uint32_t *const BaseButtonDefaultTextColors = DialogDefaults + 5;
extern Font *BaseButtonDefaultFonts[3];  // 0x009B8E34..E3C, defined in basebutton.cpp


// The palette these setters publish before recolouring is `PaletteActive`
// (palette.h, 0x009B8180) - distinct from `PaletteCurrent` at 0x009B8174.
// The `BaseButtonActivePalette()` pun accessor that used to sit here read
// the same address through a cast.


// ?timer_callback@BaseButton@@QAEXH@Z is public, __thiscall, void(int).
