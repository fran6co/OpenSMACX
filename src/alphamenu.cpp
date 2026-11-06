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
#include "popup_fonts.h"
#include "alphamenu.h"
#include "netdaemon.h"
#include "alpha.h"
#include "strings.h"
#include "text.h"
#include "terraforming.h"
#include "technology.h"
#include "veh.h"

#include "console.h"
#include "game.h"
#include "interface_palette.h"
#include "maininterface.h"
#include "palette.h"
#include "pulldown.h"
#include "wave.h"

// AlphaMenu's four foreground/shadow pairs are the words at indices 30-37
// in the shared text-colour run built by textcolor_init (palette.cpp).
extern int TextColourTable[195];
extern int WindowAnimationFrameDelay;
// RECOVERY_OBJECT: v1 0x006A7090 ?g_ALPHAMENU_WAVE@@3VWave@@A
Wave g_ALPHAMENU_WAVE;  // 0x006A7090

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
// ORIGINAL: 0x0051DC10 ??1AlphaMenu@@QAE@XZ 0x0051DC10-0x0051DC86;0x0065D220-0x0065D24E FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/0051dc10.cpp"], "reason": "C++ AlphaMenu owns Menu, Buffer and Sprite members whose reverse lifetime order follows the original destructor and exception-unwind trace."}
// compile_context src/console.cpp
// TRIED: computed that AlphaMenu : public Menu (Menu ends at exactly 0xB64, where Buffer buffer_ starts, which ends at exactly 0x10EC, where Sprite sprite_ starts, ending at 0x1118 where Sprite sprites_[3] starts) - a real member layout that would let the SEH prologue emerge automatically, but AlphaMenu is the fixed scaffold class and cannot carry real members here. Explicit dtor calls at the right offsets/order compile and are structurally faithful; the SEH frame (push -1/handler, unwind funclets) is not reproduced - same open item as 0x00629D60 (Flic::Flic) and 0047acf0.cpp.
// symbol    ??1AlphaMenu@@UAE@XZ
// kind game

// The initialization loads the screen-size-specific menu sheet into a
// temporary Buffer, extracts the button sprites, then initializes the real
// GraphicWin base. The local's automatic lifetime also covers the early
// null/error returns in the same order as the original C++ object.
// Superseded seed deleted; seed paths below are historical Git provenance.
// Historical seed observations (not fresh measurements or acceptance):
// TRIED: MISMATCH #58 (push vs mov) - a genuine local `Buffer` (RAII) reproduces the SEH try-frame/dtor exactly for 57 instructions (prologue, ctor, null check+early return, globals store, first branch's load_pcx+extract, loop setup); the do/while loop body's byte-read-then-extract() call diverges on push order. Tried inlining the byte read into the call expression (same as the first, unlooped, extract call) - made it worse (#43), so a separate local before the call is closer; kept.
// ORIGINAL: 0x00402F10 _$E21 0x00402F10-0x00402F26 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT dispatcher 0x00644DEC-0x00644DFB supplies [0x00682000,0x006826CC) to 0x00644ED8, which calls each non-null dword; original slot 0x00682004 selects initializer 0x00402F10 and it registers finalizer 0x00402F30.", "Pinned original initializer 0x00402F10-0x00402F26 and finalizer 0x00402F30-0x00402F3A independently load receiver 0x006A7090 and call Wave constructor/destructor 0x004C66E0/0x004C67C0.", "AlphaMenu::play in src/alphamenu.cpp calls the source-owned g_ALPHAMENU_WAVE in both observed playback branches; this TU now owns its genuine Wave storage and the paired original lifetime mappings.", "Historical prior-owner observation only: before this source move, a physical wave.cpp COFF selected local STATIC helpers for this global. That observation does not establish current alphamenu.cpp emission or linked helper identity.", "Individual original lifetime ownership is retained; this binding does not establish complete CRT order, current linked selection, full data layout or runtime acceptance."], "reason": "Bind the original initializer to the genuine g_ALPHAMENU_WAVE Wave object now owned with its AlphaMenu playback consumer in alphamenu.cpp. The original CRT slot and constructor/destructor receiver remain pinned evidence; no callback body is authored."}
// symbol    _$E12
// ORIGINAL: 0x00402F30 _$E19 0x00402F30-0x00402F3A FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT dispatcher 0x00644DEC-0x00644DFB supplies [0x00682000,0x006826CC) to 0x00644ED8, which calls each non-null dword; original slot 0x00682004 selects initializer 0x00402F10 and it registers finalizer 0x00402F30.", "Pinned original initializer 0x00402F10-0x00402F26 and finalizer 0x00402F30-0x00402F3A independently load receiver 0x006A7090 and call Wave constructor/destructor 0x004C66E0/0x004C67C0.", "AlphaMenu::play in src/alphamenu.cpp calls the source-owned g_ALPHAMENU_WAVE in both observed playback branches; this TU now owns its genuine Wave storage and the paired original lifetime mappings.", "Historical prior-owner observation only: before this source move, a physical wave.cpp COFF selected local STATIC helpers for this global. That observation does not establish current alphamenu.cpp emission or linked helper identity.", "Individual original lifetime ownership is retained; this binding does not establish complete CRT order, current linked selection, full data layout or runtime acceptance."], "reason": "Bind the original finalizer to the genuine g_ALPHAMENU_WAVE Wave object now owned with its AlphaMenu playback consumer in alphamenu.cpp. The original CRT slot and constructor/destructor receiver remain pinned evidence; no callback body is authored."}
// symbol    _$E10

// ORIGINAL: 0x00402F40 ?init@AlphaMenu@@QAEHPAUWin@@@Z 0x00402F40-0x00403199;0x006505C0-0x006505D5 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Retired seed provenance: src/unrecovered/00402f40.cpp,004032a0.cpp,00403af0.cpp and src/recovered/units/00403340.cpp,00403680.cpp,00403830.cpp; pinned original callback style27/29, positive hide height, branch-only clip restoration and actual caller dispatch audited by coordinator."], "reason": "Recover AlphaMenu owner behavior through existing Menu/GraphicWin/Buffer inheritance and owned Sprite lifetimes; historical seed identity and extents remain obligations, not measurement claims."}
// symbol    ?init@AlphaMenu@@UAEHPAVWin@@@Z
int AlphaMenu::init(Win *parent) {
  Buffer sheet;
  if (parent == nullptr) return 3;

  // PullDown copies these shared callback defaults into its trailing fields;
  // AlphaMenu installs the two actual cdecl callbacks before any menus open.
  PullDownFieldF38Default = &AlphaMenu::fill_func;
  PullDownFieldF3CDefault = &AlphaMenu::backfill_func;
  field_A28_ = 5;

  int result;
  int frame_y;
  Sprite *frame;
  int remaining;
  if (WinScreenWidth == 0x320) {
    result = sheet.load_pcx("menu.pcx", nullptr, 10, 0xEC);
    sprite1_.extract(&sheet, WinFillColour, 1, 1, 0x8A, 6, nullptr);
    frame_y = 8;
    frame = sprites_;
    for (remaining = 3; remaining != 0; --remaining) {
      frame->extract(&sheet, WinFillColour, 1, frame_y, 0x8A, 0x12,
                     nullptr);
      frame_y += 0x13;
      ++frame;
    }
  } else {
    result = sheet.load_pcx("menu2.pcx", nullptr, 10, 0xEC);
    sprite1_.extract(&sheet, WinFillColour, 1, 1, 0x8B, 6, nullptr);
    frame_y = 8;
    frame = sprites_;
    for (remaining = 3; remaining != 0; --remaining) {
      frame->extract(&sheet, WinFillColour, 1, frame_y, 0x8B, 0x12,
                     nullptr);
      frame_y += 0x13;
      ++frame;
    }
  }
  if (result != 0) return result;

  sheet.close();
  const int init_result = GraphicWin::init(
      0, 0, 0x51, 0x8F, nullptr, 0x1000000, &MainInterfaceGlobal,
      nullptr, nullptr);
  if (init_result != 0) return init_result;

  Buffer::set_markup_enabled(0);
  adjust();
  return 0;
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004032A0 ?adjust@AlphaMenu@@QAEXXZ 0x004032A0-0x00403335 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Retired seed provenance: src/unrecovered/00402f40.cpp,004032a0.cpp,00403af0.cpp and src/recovered/units/00403340.cpp,00403680.cpp,00403830.cpp; pinned original callback style27/29, positive hide height, branch-only clip restoration and actual caller dispatch audited by coordinator."], "reason": "Recover AlphaMenu owner behavior through existing Menu/GraphicWin/Buffer inheritance and owned Sprite lifetimes; historical seed identity and extents remain obligations, not measurement claims."}
// symbol    ?adjust@AlphaMenu@@UAEXXZ
void AlphaMenu::adjust() {
  const int height = sprites_[0].height() * count_ + sprite1_.height();
  resize(sprite1_.width(), height, 0);
  const int screen_width = WinScreenWidth;
  const RECT &bounds = MainInterfaceGlobal.window_bounds_;
  if (screen_width == 0x320) {
    const int y = bounds.top - bounds.bottom - height;
    Win::move(0, y + WinScreenHeight);
  } else {
    const int y = bounds.top - bounds.bottom - height;
    const int x = (screen_width - 0x400) / 2;
    Win::move(x, y + WinScreenHeight);
  }
  compute();
  redraw();
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004031A0 ?compute@AlphaMenu@@QAEXXZ 0x004031A0-0x00403292 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/004031a0.cpp", "git:5190d27f:src/leaf_recoveries.cpp:AlphaMenu_compute", "image:AlphaMenu66ED88 slot92 selects4031A0;4031FC/403208 walk bytes at+A40 with14 stride, matching Menu entries+A38 plus flags+8", "recovery/tasks/popup-font-configuration-owner-closure.json"], "reason": "Recover the real AlphaMenu compute override through owned sprites, inherited Spot and MenuEntry flags. The retired raw facade's byte-zero wording described the flags cursor, not the entry base; preserve its seed's frame-layout lesson without retaining raw offsets or a duplicate source identity."}
// symbol    ?compute@AlphaMenu@@UAEXXZ
// TRIED: SHAPE_EXACT now - every instruction matches and the only difference left is the FRAME LAYOUT, so these are ruled out for the LAST byte rather than for the shape. All measured: any frameless spelling, since the original keeps ebp; a real object local to force an SEH frame - div 2, this prologue registers no handler at all; declaration order of the locals - all six permutations byte-identical; statement order - 13 of 15 legal orders break the shape and the other 2 give the same slots; extra unreferenced ints or arrays to grow sub esp - reclaimed, frame stays 0x18. VC6 lays locals out by size ASCENDING, ties broken by a fixed intrinsic order top, bottom_end, top_end, i, entry, width that no source edit moved. Aggregates are therefore a real size lever and a 16-byte struct member landed 5 of the 6 slots exactly - but the original wants top_end at ebp-0x10 ABOVE top at ebp-0x18 in a 0x2c frame, which needs top_end allocated FIRST, and that forces size top_end < size top with the three summing to 0x20 and size bottom_end >= 0x10. No integers fit.
void AlphaMenu::compute() {
  const int row_height = sprites_[0].height();
  spot_.init(count_);

  RECT top_rect;
  RECT bottom_rect;
  top_rect.top = sprite1_.height();
  const int width = sprites_[0].width();
  top_rect.bottom = top_rect.top + row_height;
  bottom_rect.top = (count_ - 1) * row_height + top_rect.top;
  bottom_rect.bottom = bottom_rect.top + row_height;

  for (int i = 0; i < count_; ++i) {
    if ((entries_[i].flags & 1) == 0) continue;
    if (entries_[i].flags & 4) {
      spot_.add(i, 0, 0, bottom_rect.top, width, bottom_rect.bottom - bottom_rect.top);
      int dy = -row_height;
      bottom_rect.top += dy;
      bottom_rect.bottom += dy;
    } else {
      spot_.add(i, 0, 0, top_rect.top, width, top_rect.bottom - top_rect.top);
      top_rect.top += row_height;
      top_rect.bottom += row_height;
    }
  }
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns 0.
// ORIGINAL: 0x00403610 ?requested_height@AlphaMenu@@QAEHXZ 0x00403610-0x00403613 BYTE_EXACT
// symbol    ?requested_height@AlphaMenu@@UAEHXZ
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

// ORIGINAL: 0x00403620 ?on_adjust_pulldown_pos@AlphaMenu@@QAEXPAHPAH@Z 0x00403620-0x00403677 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned image 0x00403620 reads Menu field_A24_ and SpotInternal[index].rect at stride 0x18; Win outer_rect_ is at 0x13c and border_thickness_ is at 0x118 (caption_height_ is at 0x114).", "git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/recovered/units/00403620.cpp"], "reason": "Recover AlphaMenu pull-down placement through its inherited Menu, Spot and Win state. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
// kind      game
// symbol    ?on_adjust_pulldown_pos@AlphaMenu@@UAEXPAH0@Z
void AlphaMenu::on_adjust_pulldown_pos(int *x, int *y) {
    if (field_A24_ != -1) {
        const RECT outer_rect_snapshot = outer_rect_;
        *x = outer_rect_snapshot.right;
        *y = spot_.get_rect_at(field_A24_).top + border_thickness_
            + outer_rect_snapshot.top;
    }
}

// ORIGINAL: 0x00403B70 ?backfill_func@AlphaMenu@@QAAHPAUGraphicWin@@@Z 0x00403B70-0x00403B94 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned image 0x00403B70 calls Buffer::box_sprite at 0x005E2DD3 with the target Buffer::rect2_ at target+0x474 and style record 0x0078D960, which is InterfaceBoxStyles[27]."], "reason": "Recover AlphaMenu static backfill callback using the supplied GraphicWin and its typed Buffer subobject."}
// symbol    ?backfill_func@AlphaMenu@@SAHPAVGraphicWin@@@Z
int AlphaMenu::backfill_func(GraphicWin *target) {
    target->Buffer::box_sprite(target->buffer_working_rect(), &InterfaceBoxStyles[27]);
    return 1;
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00403AF0 ?fill_func@AlphaMenu@@QAAHPAUGraphicWin@@HPAURECT@@@Z 0x00403AF0-0x00403B70 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Retired seed provenance: src/unrecovered/00402f40.cpp,004032a0.cpp,00403af0.cpp and src/recovered/units/00403340.cpp,00403680.cpp,00403830.cpp; pinned original callback style27/29, positive hide height, branch-only clip restoration and actual caller dispatch audited by coordinator."], "reason": "Recover AlphaMenu owner behavior through existing Menu/GraphicWin/Buffer inheritance and owned Sprite lifetimes; historical seed identity and extents remain obligations, not measurement claims."}
// symbol    ?fill_func@AlphaMenu@@SAHPAVGraphicWin@@HPAUtagRECT@@@Z
int __cdecl AlphaMenu::fill_func(GraphicWin *target, int mode, RECT *clip) {
  RECT draw_clip = *clip;
  draw_clip.left = 1;
  draw_clip.right = target->buffer_working_rect()->right - 1;
  target->Buffer::set_clip(&draw_clip);
  BoxSpriteParams *styles = &InterfaceBoxStyles[27];
  target->Buffer::box_sprite(target->buffer_working_rect(),
                             styles + (mode != 0 ? 2 : 0));
  target->Buffer::restore_clip();
  return 1;
}

// Draws one menu sprite at the left edge with its own transparent key.
static inline void draw_menu_sprite(Sprite *sprite, Buffer *destination, int y) {
  sprite->draw(destination, sprite->transparent_index(), 0, y, 1, 1);
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00403340 ?on_redraw@AlphaMenu@@QAEXXZ 0x00403340-0x00403544 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Retired seed provenance: src/unrecovered/00402f40.cpp,004032a0.cpp,00403af0.cpp and src/recovered/units/00403340.cpp,00403680.cpp,00403830.cpp; pinned original callback style27/29, positive hide height, branch-only clip restoration and actual caller dispatch audited by coordinator."], "reason": "Recover AlphaMenu owner behavior through existing Menu/GraphicWin/Buffer inheritance and owned Sprite lifetimes; historical seed identity and extents remain obligations, not measurement claims."}
// symbol    ?on_redraw@AlphaMenu@@UAEXXZ
void AlphaMenu::on_redraw() {
  GraphicWin::fill(9);
  draw_menu_sprite(&sprite1_, this, 0);
  Buffer::set_font(PopupMenuBoldFont, nullptr, nullptr, nullptr);

  int y = sprite1_.height();
  int i;
  for (i = 0; i < count_; ++i) {
    if (i == field_A24_) {
      draw_menu_sprite(&sprites_[2], this, y);
    } else if (i == MenuSelection) {
      draw_menu_sprite(&sprites_[1], this, y);
    } else {
      draw_menu_sprite(&sprites_[0], this, y);
    }
    y += sprites_[0].height();
  }

  for (i = 0; i < count_; ++i) {
    if ((entries_[i].flags & 1) == 0) continue;

    if (i == field_A24_) {
      if (entries_[i].flags & 2)
        Buffer::set_text_color(TextColourTable[34], TextColourTable[35], 1, 1);
      else
        Buffer::set_text_color(TextColourTable[30], TextColourTable[31], 1, 1);
    } else {
      if (entries_[i].flags & 2)
        Buffer::set_text_color(TextColourTable[36], TextColourTable[37], 1, 1);
      else
        Buffer::set_text_color(TextColourTable[32], TextColourTable[33], 1, 1);
    }

    char text[256];
    text[0] = '\0';
    strcat(text, entries_[i].text);
    RECT row;
    spot_.get_rect_at(i, &row);
    row.left += 8;
    row.right -= 8;
    CharUpperA(text);
    char *label = text;
    if (label != 0) {
      Buffer::write_l(label, &row, strlen(label));
    }
  }
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00403680 ?show@AlphaMenu@@QAEXXZ 0x00403680-0x0040382F FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Retired seed provenance: src/unrecovered/00402f40.cpp,004032a0.cpp,00403af0.cpp and src/recovered/units/00403340.cpp,00403680.cpp,00403830.cpp; pinned original callback style27/29, positive hide height, branch-only clip restoration and actual caller dispatch audited by coordinator."], "reason": "Recover AlphaMenu owner behavior through existing Menu/GraphicWin/Buffer inheritance and owned Sprite lifetimes; historical seed identity and extents remain obligations, not measurement claims."}
void AlphaMenu::show() {
  if (is_visible()) return;

  if (GameDataStateStorage.fields_.game_preferences_ & 0x400) g_ALPHAMENU_WAVE.play();
  on_redraw();

  RECT screen_rect = *buffer_working_rect();
  client_to_screen(&screen_rect);
  const int frame_x = animation_frame_x();
  int y = screen_rect.bottom - sprite1_.height();
  const int step = (screen_rect.bottom - screen_rect.top) / 5;
  Win::screen_buffer_.set_clip(&screen_rect);

  if ((GameDataStateStorage.fields_.game_preferences_ & 0x8000000) != 0 && IsMultiplayerNet == 0) {
    WinFlipClipped = 1;
    for (int frame = 0; frame != 6; ++frame) {
      const DWORD started = timeGetTime();
      Buffer::draw(&Win::screen_buffer_, 9,
                                        frame_x, y, 1, 1);
      flip(&screen_rect);
      y -= step;
      do_sound();
      wait_for_animation_frame(started);
    }
    Win::screen_buffer_.restore_clip();
    WinFlipClipped = 0;
  }

  Win::screen_buffer_.restore_clip();
  WinFlipClipped = 0;
  Win::show(3);
  MainInterfaceGlobal.flat_button_[3].set_pressed_visual(1);
  MainInterfaceGlobal.flat_button_[3].redraw();
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00403830 ?hide@AlphaMenu@@QAEXXZ 0x00403830-0x00403AE5 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Retired seed provenance: src/unrecovered/00402f40.cpp,004032a0.cpp,00403af0.cpp and src/recovered/units/00403340.cpp,00403680.cpp,00403830.cpp; pinned original callback style27/29, positive hide height, branch-only clip restoration and actual caller dispatch audited by coordinator."], "reason": "Recover AlphaMenu owner behavior through existing Menu/GraphicWin/Buffer inheritance and owned Sprite lifetimes; historical seed identity and extents remain obligations, not measurement claims."}
// symbol    ?hide@AlphaMenu@@UAEXXZ
void AlphaMenu::hide() {
  if (!is_visible()) return;

  if (GameDataStateStorage.fields_.game_preferences_ & 0x400) g_ALPHAMENU_WAVE.play();
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
  const int console_copy_height = pixel_height();
  const int console_copy_width = pixel_width();
  ConsoleGlobal.Buffer::copy(
      &buffer_, console_rect.left, console_rect.top, 0, 0,
      console_copy_width, console_copy_height);

  const int frame_x = animation_frame_x();
  int y = screen_rect.top;
  const int step = (screen_rect.bottom - screen_rect.top) / 5;
  Win::screen_buffer_.set_clip(&screen_rect);
  const int target_y = screen_rect.bottom - sprite1_.height();
  if ((GameDataStateStorage.fields_.game_preferences_ & 0x8000000) != 0 && IsMultiplayerNet == 0) {
    WinFlipClipped = 1;
    for (int frame = 0; frame != 6; ++frame) {
      const DWORD started = timeGetTime();
      const int frame_copy_height = pixel_height();
      const int frame_copy_width = pixel_width();
      buffer_.copy(&Win::screen_buffer_, 0, 0, screen_rect.left, screen_rect.top,
                   frame_copy_width, frame_copy_height);
      Buffer::draw(&Win::screen_buffer_, 9, frame_x, y, 1, 1);
      flip(&screen_rect);
      y += step;
      if (y >= target_y) y = target_y;
      do_sound();
      wait_for_animation_frame(started);
    }
    if (y < target_y) {
      const int final_copy_height = pixel_height();
      const int final_copy_width = pixel_width();
      buffer_.copy(&Win::screen_buffer_, 0, 0, screen_rect.left, screen_rect.top,
                   final_copy_width, final_copy_height);
      Buffer::draw(&Win::screen_buffer_, 9,
                   screen_rect.left, target_y, 1, 1);
      flip(&screen_rect);
    }
    Win::screen_buffer_.restore_clip();
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
// Superseded seed deleted; seed paths below are historical Git provenance.
// Historical seed observations (not fresh measurements or acceptance):
// TRIED: if/else polarity flipped to match layout (jne first, matches); remaining MISMATCH is the compiler omitting the EBP frame that the original allocates (esp-relative &a2 suffices here), not reproduced by reordering statements
// ORIGINAL: 0x00403550 ?on_mouse_move@AlphaMenu@@QAEXHH@Z 0x00403550-0x0040360F BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/00403550.cpp"], "reason": "The original menu mouse body updates Menu spot selection and Win pointer capture through the inherited slot-17 callback, whose product spelling is on_mouse_move."}
// symbol    ?on_mouse_move@AlphaMenu@@UAEXHH@Z
void AlphaMenu::on_mouse_move(int x, int y) {
    if (field_A24_ != -1) {
        Menu::on_mouse_move(x, y);
    } else if (WinPointerOwner3 == 0) {
        WinPointerOwner3 = this;
        WinPointerOwner4 = 0;
    }

    // Spot writes the position on every hit; the miss path never reads it.
    int position;
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


// ORIGINAL: 0x0051DC90 ??_GAlphaMenu@@UAEPAXI@Z 0x0051DC90-0x0051DCB1 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical source:src/deleting_thunks.cpp; preserved full original span 0x0051DC90-0x0051DCB1 and BYTE_EXACT obligation.", "pinned Console constructor 0x0050F52F stores primary AlphaMenu table 0x0066ED88 at alpha_menu_+0; pinned table slot 0 is 0x0051DC90.", "pinned scalar body calls AlphaMenu::~AlphaMenu at 0x0051DC10, tests flags bit 0, conditionally calls 0x0064557F, returns saved this with ret 4.", "the prior retained src/alphamenu.cpp.obj/PE snapshot emitted ??_GAlphaMenu@@UAEPAXI@Z with class-destructor and scalar-delete relocations; its AlphaMenu Win table selected that address. This is historical evidence only, not qualification of the current source.", "the AlphaMenu source owner has typed Menu/Buffer/Sprite members and a C++ destructor; this transfers only the generated scalar identity, not the class's unfinished behavioral claims.", "the byte-exact claim is historical and remains a restoration obligation; no current equality or whole-application acceptance is asserted."], "reason": "Bind AlphaMenu's original primary scalar deleting identity to its compiler-generated virtual-destructor operation in the real AlphaMenu owner TU; this FILE mapping is generated-owner provenance, not a copied body."}
// compile_context src/console.cpp
// symbol    ??_GAlphaMenu@@UAEPAXI@Z
// kind game

// ORIGINAL: 0x0051DF70 ??_GAlphaMenu@@WEEE@AEPAXI@Z 0x0051DF70-0x0051DF7B FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical source:src/deleting_thunks.cpp; preserved full original span 0x0051DF70-0x0051DF7B and its original scalar-adjustor identity.", "pinned Console constructor 0x0050F535 stores AlphaMenu Buffer-view table 0x0066ED80 at alpha_menu_+0x444; table slot 0 is exactly 0x0051DF70. Primary view at +0 stores table 0x0066ED88 with slot 0 = 0x0051DC90.", "pinned 0x0051DF70 subtracts 0x444 from ECX and jumps to scalar ??_GAlphaMenu at 0x0051DC90 without changing the flags argument.", "the prior retained AlphaMenu COFF showed its table relocation selecting ??_EAlphaMenu@@WEEE@AEPAXI@Z; that thunk subtracts 0x444 and jumps to weak primary E, whose mode-2 fallback is exactly ??_GAlphaMenu@@UAEPAXI@Z. The prior linked Buffer-view slot 0 selected that thunk; this is historical evidence, not current qualification or a new vector-deleting original identity.", "the source class's GraphicWin/Buffer secondary view and pinned +0x444 store explain this receiver adjustment; do not generalize the E/G alias to other classes without their own relocation and table proof.", "the byte-exact claim is historical and remains a restoration obligation; no current equality or whole-application acceptance is asserted."], "reason": "Bind AlphaMenu's original Buffer-view scalar deleting adjustor to the compiler-emitted secondary E thunk because the pinned view relationship and prior physical snapshot show its mode-2 weak fallback resolving to the generated scalar primary G body; fresh current-build qualification remains pending."}
// compile_context src/console.cpp
// symbol    ??_EAlphaMenu@@WEEE@AEPAXI@Z
// kind game
// RECOVERY_GENERATED: v1 vtable 0x0066ED88 ? ??_7AlphaMenu@@6BWin@@@ 0x0050F460
// RECOVERY_GENERATED: v1 vtable 0x0066ED80 ? ??_7AlphaMenu@@6BBuffer@@@ 0x0050F460
