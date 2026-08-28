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
class BaseButton : public GraphicWin {
  friend class ButtonGroup;
  friend class FlatButton;
  friend class Scroll;

 public:
  void on_right_click(int a, int b);
  // 0x00607B30  ?timer_callback@BaseButton@@QAEXH@Z - public, __thiscall,
  // void(int). Public is load-bearing: the non-member daemon fn_00607b10
  // reaches it through a BaseButton * it was handed.
  void timer_callback(int a1);
  // These return `int`, not `void`. Each body is `xor eax, eax; ret N`, and
  // the sibling class BasePop declares the same three handlers `int`. The
  // catalogued mangled names spelled `X` and are corrected in
  // tools/catalogue_corrections.py.
  int on_key_click(int a, int b);
  int on_key_down(int a);
  int on_key_up(int a);
  BaseButton() { ; }
  ~BaseButton() { ; }

  BaseButton *construct();
  // VOID, AND THE IMAGE IS WHAT SAYS SO: the catalogue spells it
  // ?close@BaseButton@@QAEXXZ. The `uint32_t` this used to return was the
  // free() residue in EAX that the image never sets on purpose, and spelling
  // it cost one instruction (`xor eax,eax`) at the close of the body.
  void close();
  int init(LPCSTR name, int id, int x, int y, int width, int height,
           Win *parent, int style_flag);
  BaseButton *destroy();
  void set_text_color(int color1, int color2, int color3, int color4);
  void set_text_color2(int color1, int color2, int color3, int color4);
  void set_text_color3(int color1, int color2, int color3, int color4);
  int set_bubble_text(LPCSTR input);
  int set_name(LPCSTR input);
  void set(int value);

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
  LPSTR name_;
  LPSTR bubble_text_;
  uint32_t color_;
  uint32_t bevel_upper_;
  uint32_t bevel_lower_;
  uint32_t bevel_thickness_;
  uint32_t field_A94_;
  uint32_t field_A98_;
  uint32_t field_A9C_;
  uint32_t field_AA0_;
  uint32_t field_AA4_;
  ButtonGroup *group_;
  uint32_t field_AAC_;
  uint32_t field_AB0_;
  uint32_t field_AB4_;
};

static_assert(sizeof(BaseButton) == 0xAB8,
              "BaseButton layout must match the legacy ABI");

extern const uint32_t BaseButtonPrimaryVtable;
extern const uint32_t BaseButtonBufferVtable;
uint32_t *const BaseButtonStaticDefaults = (uint32_t *)0x0069704C;
uint32_t *const BaseButtonDynamicDefaults = (uint32_t *)0x009B8E2C;

BaseButton *__fastcall base_button_construct_redirect(BaseButton *self, void *);
uint32_t __fastcall base_button_close_redirect(BaseButton *self, void *);
int __fastcall base_button_init_redirect(
    BaseButton *self, void *, LPCSTR name, int id, int x, int y, int width,
    int height, Win *parent, int style_flag);


// Interleaved 3x4 default colour table and the three default font slots the
// setters publish; tests outside the hybrid process rebind them.
uint32_t *const BaseButtonDefaultTextColors = (uint32_t *)0x00697060;
Font **const BaseButtonDefaultFonts = (Font **)0x009B8E34;


// The palette these setters publish before recolouring is `PaletteActive`
// (palette.h, 0x009B8180) - distinct from `PaletteCurrent` at 0x009B8174.
// The `BaseButtonActivePalette()` pun accessor that used to sit here read
// the same address through a cast.


// The timer tick declared above is not recovered; it forwards to the original
// image. ?timer_callback@BaseButton@@QAEXH@Z is public, __thiscall, void(int).
