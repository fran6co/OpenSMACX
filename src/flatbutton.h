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
#include "basebutton.h"

class Sprite;
// Nine groups of three borrowed sprite pointers, read by the redraw method.
extern Sprite *FlatButtonDefaultsTable[27];
static_assert(sizeof(Sprite *) == 4, "FlatButton sprite table requires x86 pointers");

 /*
  * FlatButton class
  */
class FlatButton : public BaseButton {
  // MainInterface initializes these owned controls' colors and sprite links.
  friend class MainInterface;
  friend class DesignWin;
 public:
  FlatButton();
  int field_AB8() const { return static_cast<int>(field_AB8_); }
  // DEFINED IN-CLASS so enclosing destructors can expand it, which is what
  // the image does: ??1DipEdit carries five expanded copies (the
  // 0x669754/0x66974c stage pair, `call ?close@FlatButton`, `call
  // ??1BaseButton`, the enclosing EH state written between them), and so do
  // ??1BasePop and ??1FileWin. Historical compiler measurement after the
  // 2026-09-02 move: the class's own COMDAT measured 24/24 against
  // ??1FlatButton@@QAE@XZ at 0x00406880 in flatbutton.cpp. That observation
  // records the then-measured owner copy; it is not a current claim that
  // forced export is needed. The inline definition also lets enclosing sites
  // expand the actual lifetime.
  ~FlatButton() { close(); }

  // Constructor607CFB installs669754: slot90 selects607DA0.
  virtual void close();
  FlatButton *destroy();
  void on_redraw() override;
  void on_mouse_move(int, int);
  void on_mouse_leave(int, int) override;
  // ReportIf::report writes the per-page state at FlatButton+0xAB8 before
  // redrawing its seven category controls (original 0x004A3950).
  void set_report_page_state(int state) {
    field_AB8_ = static_cast<uint32_t>(state);
  }
  // Scroll60569A..6056A6 and6057A1..6057AD write the borrowed arrow
  // sprites to +AD4/+AD8/+ADC, the same run consumed by on_redraw.
  // These are not AutoSound's unrelated +28/+2C/+30 settings.
  void set_scroll_icons(Sprite *normal, Sprite *hover, Sprite *pressed) {
    field_AD4_[0] = normal;
    field_AD4_[1] = hover;
    field_AD4_[2] = pressed;
  }
  // ReportIf and Scroll both use this real borrowed-sprite run.
  void set_report_icons(Sprite *normal, Sprite *hover, Sprite *pressed) {
    field_AD4_[0] = normal;
    field_AD4_[1] = hover;
    field_AD4_[2] = pressed;
  }
  // Borrowed slice configuration used by BaseWin init and the redraw method.
  void set_slice_colours(int normal, int hover, int pressed) {
    field_ABC_[0] = static_cast<uint32_t>(normal);
    field_ABC_[1] = static_cast<uint32_t>(hover);
    field_ABC_[2] = static_cast<uint32_t>(pressed);
  }
  void set_slice_icons(Sprite *left_normal, Sprite *left_hover, Sprite *left_pressed,
                       Sprite *middle_normal, Sprite *middle_hover, Sprite *middle_pressed,
                       Sprite *right_normal, Sprite *right_hover, Sprite *right_pressed) {
    config[12] = left_normal;
    config[13] = left_hover;
    config[14] = left_pressed;
    config[6] = middle_normal;
    config[7] = middle_hover;
    config[8] = middle_pressed;
    config[15] = right_normal;
    config[16] = right_hover;
    config[17] = right_pressed;
  }
  void set_border_styles(BoxSpriteParams *normal, BoxSpriteParams *hover,
                         BoxSpriteParams *pressed) {
    border_styles_[0] = normal;
    border_styles_[1] = hover;
    border_styles_[2] = pressed;
  }

 private:
  uint32_t field_AB8_;  // 0x0ab8
  // Original 607CF0 walks three columns, resetting each of these runs.
  uint32_t field_ABC_[3];
  BoxSpriteParams *border_styles_[3];  // 0xAC8, borrowed style records
  // ReportIf4A431B..432F stores three borrowed Sprite pointers here;
  // constructor/close clear them without destroying the sprites.
  Sprite *field_AD4_[3];
 public:
  // Nine borrowed sprite groups, each with three state columns. The complete
  // redraw null-tests/dereferences all27slots; defaults and BasePop copy the
  // same indices. The pointer table occupies0x6C bytes with unchanged offsets.
  Sprite *config[27];  // 0xAE0..0xB48
};

static_assert(sizeof(FlatButton) == 0xB4C,
              "FlatButton layout must match the legacy ABI");
