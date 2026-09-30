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
#include "stdafx.h"
#include "alphamenu.h"
#include "console.h"
#include "game.h"
#include "maininterface.h"
#include "palette.h"
#include "pulldown.h"
#include "wave.h"

// AlphaMenu's four foreground/shadow pairs are the words at indices 30-37
// in the shared text-colour run built by textcolor_init (palette.cpp).
extern int TextColourTable[195];
extern int WindowAnimationFrameDelay;
extern Wave g_ALPHAMENU_WAVE;

namespace {

int animation_frame_x() {
  return WinScreenWidth == 0x320 ? 0 : (WinScreenWidth - 0x400) / 2;
}

void wait_for_animation_frame(DWORD started) {
  while (timeGetTime() - started < static_cast<DWORD>(WindowAnimationFrameDelay)) {
    do_sound();
  }
}

}  // namespace

// The compiler destroys sprites_[2..0], sprite1_, buffer_, then the Menu
// base in the order observed at 0x0051DC10. Keep this as C++ ownership,
// not a manual walk over the object bytes.
// ORIGINAL: 0x0051DC10 ??1AlphaMenu@@QAE@XZ 0x0051DC10-0x0051DC86;0x0065D220-0x0065D24E
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/0051dc10.cpp"], "reason": "C++ AlphaMenu owns Menu, Buffer and Sprite members whose reverse lifetime order follows the original destructor and exception-unwind trace."}
// symbol    ??1AlphaMenu@@UAE@XZ
AlphaMenu::~AlphaMenu() {}

// The initialization loads the screen-size-specific menu sheet into a
// temporary Buffer, extracts the button sprites, then initializes the real
// GraphicWin base. The local's automatic lifetime also covers the early
// null/error returns in the same order as the original C++ object.
// ORIGINAL: 0x00402F40 ?init@AlphaMenu@@QAEHPAUWin@@@Z 0x00402F40-0x00403199;0x006505C0-0x006505D5 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Retained seeds src/unrecovered/00402f40.cpp,004032a0.cpp,00403af0.cpp and src/recovered/units/00403340.cpp,00403680.cpp,00403830.cpp; pinned original callback style27/29, positive hide height, branch-only clip restoration and actual caller dispatch audited by coordinator."], "reason": "Recover AlphaMenu owner behavior through existing Menu/GraphicWin/Buffer inheritance and owned Sprite lifetimes; historical seed identity and extents remain obligations, not measurement claims."}
// symbol    ?init@AlphaMenu@@QAEHPAVWin@@@Z
int AlphaMenu::init(Win *parent) {
  Buffer sheet;
  if (parent == nullptr) return 3;

  // PullDown copies these shared callback defaults into its trailing fields;
  // AlphaMenu installs the two actual cdecl callbacks before any menus open.
  PullDownFieldF38Default = &AlphaMenu::fill_func;
  PullDownFieldF3CDefault = &AlphaMenu::backfill_func;
  field_A28_ = 5;

  const bool compact = WinScreenWidth == 0x320;
  const int result = sheet.load_pcx(compact ? "menu.pcx" : "menu2.pcx",
                                   nullptr, 10, 0xEC);
  const unsigned char transparent = WinFillColour;
  const int sprite_y = compact ? 0x8A : 0x8B;
  sprite1_.extract(&sheet, transparent, 1, 1, sprite_y, 6, nullptr);
  int frame_y = 8;
  for (int i = 0; i != 3; ++i, frame_y += 0x13) {
    sprites_[i].extract(&sheet, transparent, 1, frame_y, sprite_y, 0x12,
                        nullptr);
  }
  if (result != 0) return result;

  sheet.close();
  const int init_result = GraphicWin::init(
      0, 0, 0x51, 0x8F, nullptr, 0x1000000, &MainInterfaceGlobal,
      nullptr, nullptr);
  if (init_result != 0) return init_result;

  static_cast<Buffer *>(this)->set_markup_enabled(0);
  vslot_93();
  return 0;
}

// ORIGINAL: 0x004032A0 ?adjust@AlphaMenu@@QAEXXZ 0x004032A0-0x00403335 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Retained seeds src/unrecovered/00402f40.cpp,004032a0.cpp,00403af0.cpp and src/recovered/units/00403340.cpp,00403680.cpp,00403830.cpp; pinned original callback style27/29, positive hide height, branch-only clip restoration and actual caller dispatch audited by coordinator."], "reason": "Recover AlphaMenu owner behavior through existing Menu/GraphicWin/Buffer inheritance and owned Sprite lifetimes; historical seed identity and extents remain obligations, not measurement claims."}
void AlphaMenu::adjust() {
  const int height = sprites_[0].height() * count_ + sprite1_.height();
  resize(sprite1_.width(), height, 0);

  const int x = WinScreenWidth == 0x320 ? 0 : (WinScreenWidth - 0x400) / 2;
  const int y = MainInterfaceGlobal.window_bounds_.top
      - MainInterfaceGlobal.window_bounds_.bottom + WinScreenHeight - height;
  Win::move(x, y);
  redraw();
  vslot_92();
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns 0.
// ORIGINAL: 0x00403610 ?requested_height@AlphaMenu@@QAEHXZ 0x00403610-0x00403613 BYTE_EXACT
// size      3 bytes
// prototype int (__thiscall ?requested_height@AlphaMenu@@QAEHXZ)(AlphaMenu* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0
Status: Complete
*/
int AlphaMenu::requested_height() {
    return 0;
}

// ORIGINAL: 0x00403620 ?on_adjust_pulldown_pos@AlphaMenu@@QAEXPAHPAH@Z 0x00403620-0x00403677
// RECOVERY_BINDING: v1 {"evidence": ["Pinned image 0x00403620 reads Menu field_A24_ and SpotInternal[index].rect at stride 0x18; Win outer_rect_ is at 0x13c and caption_height_ at 0x118."], "reason": "Recover AlphaMenu pull-down placement through its inherited Menu, Spot and Win state."}
// symbol    ?on_adjust_pulldown_pos@AlphaMenu@@QAEXPAH0@Z
void AlphaMenu::on_adjust_pulldown_pos(int *x, int *y) {
    if (field_A24_ != -1) {
        RECT spot_rect;
        spot_.get_rect_at(field_A24_, &spot_rect);
        *x = outer_rect_.right;
        *y = spot_rect.top + caption_height_ + outer_rect_.top;
    }
}

// ORIGINAL: 0x00403B70 ?backfill_func@AlphaMenu@@QAAHPAUGraphicWin@@@Z 0x00403B70-0x00403B94 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned image 0x00403B70 calls Buffer::box_sprite at 0x005E2DD3 with the target Buffer::rect2_ at target+0x474 and style record 0x0078D960, which is InterfaceBoxStyles[27]."], "reason": "Recover AlphaMenu static backfill callback using the supplied GraphicWin and its typed Buffer subobject."}
// symbol    ?backfill_func@AlphaMenu@@SAHPAVGraphicWin@@@Z
int AlphaMenu::backfill_func(GraphicWin *target) {
    target->Buffer::box_sprite(target->buffer_working_rect(), &InterfaceBoxStyles[27]);
    return 1;
}

// ORIGINAL: 0x00403AF0 ?fill_func@AlphaMenu@@QAAHPAUGraphicWin@@HPAURECT@@@Z 0x00403AF0-0x00403B70 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Retained seeds src/unrecovered/00402f40.cpp,004032a0.cpp,00403af0.cpp and src/recovered/units/00403340.cpp,00403680.cpp,00403830.cpp; pinned original callback style27/29, positive hide height, branch-only clip restoration and actual caller dispatch audited by coordinator."], "reason": "Recover AlphaMenu owner behavior through existing Menu/GraphicWin/Buffer inheritance and owned Sprite lifetimes; historical seed identity and extents remain obligations, not measurement claims."}
// symbol    ?fill_func@AlphaMenu@@SAHPAVGraphicWin@@HPAUtagRECT@@@Z
int __cdecl AlphaMenu::fill_func(GraphicWin *target, int mode, RECT *clip) {
  Buffer *const canvas = static_cast<Buffer *>(target);
  RECT draw_clip = *clip;
  draw_clip.left = 1;
  draw_clip.right = target->buffer_working_rect()->right - 1;
  canvas->set_clip(&draw_clip);
  canvas->box_sprite(target->buffer_working_rect(),
                     &InterfaceBoxStyles[mode != 0 ? 29 : 27]);
  canvas->set_clip(target->buffer_working_rect());
  return 1;
}

// ORIGINAL: 0x00403340 ?on_redraw@AlphaMenu@@QAEXXZ 0x00403340-0x00403544 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Retained seeds src/unrecovered/00402f40.cpp,004032a0.cpp,00403af0.cpp and src/recovered/units/00403340.cpp,00403680.cpp,00403830.cpp; pinned original callback style27/29, positive hide height, branch-only clip restoration and actual caller dispatch audited by coordinator."], "reason": "Recover AlphaMenu owner behavior through existing Menu/GraphicWin/Buffer inheritance and owned Sprite lifetimes; historical seed identity and extents remain obligations, not measurement claims."}
// symbol    ?on_redraw@AlphaMenu@@UAEXXZ
void AlphaMenu::on_redraw() {
  Buffer *const canvas = static_cast<Buffer *>(this);
  GraphicWin::fill(9);
  sprite1_.draw(canvas, sprite1_.transparent_index(), 0, 0, 1, 1);
  canvas->set_font(PopupMenuBoldFont, nullptr, nullptr, nullptr);

  int y = sprite1_.height();
  for (int sprite_index = 0; sprite_index < count_; ++sprite_index) {
    Sprite *sprite = &sprites_[0];
    if (sprite_index == field_A24_) {
      sprite = &sprites_[2];
    } else if (sprite_index == MenuSelection) {
      sprite = &sprites_[1];
    }
    sprite->draw(canvas, sprite->transparent_index(), 0, y, 1, 1);
    y += sprites_[0].height();
  }

  for (int label_index = 0; label_index < count_; ++label_index) {
    const uint8_t flags = entries_[label_index].flags;
    if ((flags & 1) == 0) continue;

    const int color_pair = label_index == field_A24_
        ? ((flags & 2) ? 34 : 30)
        : ((flags & 2) ? 36 : 32);
    canvas->set_text_color(TextColourTable[color_pair],
                           TextColourTable[color_pair + 1], 1, 1);

    char text[256];
    text[0] = '\0';
    strcat(text, entries_[label_index].text);
    CharUpperA(text);

    RECT row;
    spot_.get_rect_at(label_index, &row);
    row.left += 8;
    row.right -= 8;
    canvas->write_l(text, &row, strlen(text));
  }
}

// ORIGINAL: 0x00403680 ?show@AlphaMenu@@QAEXXZ 0x00403680-0x0040382F FILE
// RECOVERY_BINDING: v1 {"evidence": ["Retained seeds src/unrecovered/00402f40.cpp,004032a0.cpp,00403af0.cpp and src/recovered/units/00403340.cpp,00403680.cpp,00403830.cpp; pinned original callback style27/29, positive hide height, branch-only clip restoration and actual caller dispatch audited by coordinator."], "reason": "Recover AlphaMenu owner behavior through existing Menu/GraphicWin/Buffer inheritance and owned Sprite lifetimes; historical seed identity and extents remain obligations, not measurement claims."}
void AlphaMenu::show() {
  if (is_visible()) return;

  if (GamePreferences & 0x400) g_ALPHAMENU_WAVE.play();
  on_redraw();

  RECT screen_rect = *buffer_working_rect();
  client_to_screen(&screen_rect);
  int y = screen_rect.bottom - sprite1_.height();
  const int step = (screen_rect.bottom - screen_rect.top) / 5;
  ScreenBuffer.set_clip(&screen_rect);

  if ((GamePreferences & 0x8000000) != 0 && IsMultiplayerNet == 0) {
    const int frame_x = animation_frame_x();
    WinFlipClipped = 1;
    for (int frame = 0; frame != 6; ++frame) {
      const DWORD started = timeGetTime();
      static_cast<Buffer *>(this)->draw(&ScreenBuffer, 9,
                                        frame_x, y, 1, 1);
      flip(&screen_rect);
      y -= step;
      do_sound();
      wait_for_animation_frame(started);
    }
    ScreenBuffer.set_clip(&WinScreenClipRect);
    WinFlipClipped = 0;
  }

  ScreenBuffer.set_clip(&WinScreenClipRect);
  WinFlipClipped = 0;
  Win::show(3);
  MainInterfaceGlobal.flat_button_[3].set_pressed_visual(1);
  MainInterfaceGlobal.flat_button_[3].redraw();
}

// ORIGINAL: 0x00403830 ?hide@AlphaMenu@@QAEXXZ 0x00403830-0x00403AE5 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Retained seeds src/unrecovered/00402f40.cpp,004032a0.cpp,00403af0.cpp and src/recovered/units/00403340.cpp,00403680.cpp,00403830.cpp; pinned original callback style27/29, positive hide height, branch-only clip restoration and actual caller dispatch audited by coordinator."], "reason": "Recover AlphaMenu owner behavior through existing Menu/GraphicWin/Buffer inheritance and owned Sprite lifetimes; historical seed identity and extents remain obligations, not measurement claims."}
// symbol    ?hide@AlphaMenu@@UAEXXZ
void AlphaMenu::hide() {
  if (!is_visible()) return;

  if (GamePreferences & 0x400) g_ALPHAMENU_WAVE.play();
  MenuSelection = -1;
  MainInterfaceGlobal.field_216E8_ = 0;
  on_redraw();

  RECT screen_rect = *buffer_working_rect();
  client_to_screen(&screen_rect);
  RECT console_rect = screen_rect;
  ConsoleGlobal.screen_to_client(&console_rect);

  const int width = pixel_width();
  const int height = pixel_height();
  buffer_.init(width, height, 0, 0);
  static_cast<Buffer *>(&ConsoleGlobal)->copy(
      &buffer_, console_rect.left, console_rect.top, 0, 0, width, height);

  int y = screen_rect.top;
  const int target_y = screen_rect.bottom - sprite1_.height();
  const int step = (screen_rect.bottom - screen_rect.top) / 5;
  ScreenBuffer.set_clip(&screen_rect);
  if ((GamePreferences & 0x8000000) != 0 && IsMultiplayerNet == 0) {
    const int frame_x = animation_frame_x();
    WinFlipClipped = 1;
    for (int frame = 0; frame != 6; ++frame) {
      const DWORD started = timeGetTime();
      buffer_.copy(&ScreenBuffer, 0, 0, screen_rect.left, screen_rect.top,
                   width, height);
      static_cast<Buffer *>(this)->draw(&ScreenBuffer, 9, frame_x, y, 1, 1);
      flip(&screen_rect);
      y += step;
      if (y >= target_y) y = target_y;
      do_sound();
      wait_for_animation_frame(started);
    }
    if (y < target_y) {
      buffer_.copy(&ScreenBuffer, 0, 0, screen_rect.left, screen_rect.top,
                   width, height);
      static_cast<Buffer *>(this)->draw(&ScreenBuffer, 9,
                                        screen_rect.left, target_y, 1, 1);
      flip(&screen_rect);
    }
    ScreenBuffer.set_clip(&WinScreenClipRect);
    WinFlipClipped = 0;
  }

  Win::hide();
  buffer_.close();
  MainInterfaceGlobal.flat_button_[3].set_pressed_visual(0);
  MainInterfaceGlobal.flat_button_[3].redraw();
}

// The image's on_mouse_move identity is Win slot 17: it tracks the
// highlighted Spot and releases capture when the pointer leaves the menu.
// Unlike the old seed scaffold, this uses Menu's actual Spot and Win's
// source-owned capture globals and virtual callbacks.
// ORIGINAL: 0x00403550 ?on_mouse_move@AlphaMenu@@QAEXHH@Z 0x00403550-0x0040360F
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/00403550.cpp"], "reason": "The original menu mouse body updates Menu spot selection and Win pointer capture through the inherited slot-17 callback, whose product spelling is on_mouse_feedback."}
// symbol    ?on_mouse_feedback@AlphaMenu@@UAEXHH@Z
void AlphaMenu::on_mouse_feedback(int x, int y) {
    if (field_A24_ != -1) {
        Menu::on_mouse_move(x, y);
    } else if (WinPointerOwner3 == 0) {
        WinPointerOwner3 = this;
        WinPointerOwner4 = 0;
    }

    int position = y;
    const int hit = spot_.check(x, y, &position, 0);
    if (hit == -1) {
        MenuSelection = -1;
        redraw();
        on_update();
        if (WinPointerOwner3 == this) {
            WinPointerOwner3 = 0;
            on_lose_mouse_capture();
        }
        if (WinPointerOwner4 == this) {
            WinPointerOwner4 = 0;
        }
    } else {
        MenuSelection = position;
        redraw();
        on_update();
    }
}
