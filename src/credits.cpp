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
#include "wave.h"
#include "credits.h"
#include "text.h"
#include "strings.h"
#include "game.h"

int __cdecl pop_wait();

extern Wave g_CREDITS_WAVE;

// ORIGINAL: 0x00428780 ??0Credits@@QAE@XZ 0x00428780-0x00428830;0x00653110-0x0065315A FILE
// RECOVERY_BINDING: v1 {"evidence": ["retired seed git:024b38ed:src/unrecovered/00428780.cpp records the member construction order and zero-filled line array", "Credits embeds real GraphicWin, Font, Buffer, FlatButton and Time members of the original sizes"], "reason": "Use genuine C++ base and member construction instead of fixed vtable stores and placement-new at raw offsets."}
Credits::Credits() : frame_count_(0), line_count_(0) {
  memset(lines_, 0, sizeof(lines_));
}

// ORIGINAL: 0x00428E90 ??1Credits@@QAE@XZ 0x00428E90-0x00428F72;0x00653170-0x006531D0 FILE
// RECOVERY_BINDING: v1 {"evidence": ["retired seed git:024b38ed:src/recovered/units/00428e90.cpp records close followed by reverse member/base teardown", "The class declares the original typed owned subobjects"], "reason": "Let VC6 destroy the real C++ members and base after the image's explicit close phase."}
// symbol    ??1Credits@@UAE@XZ
// GraphicWin's inherited virtual destructor makes this destructor virtual.
Credits::~Credits() {
  close();
}

static void __cdecl credits_timer_tick(int owner) {
  reinterpret_cast<Credits *>(owner)->timer_callback_daemon(0);
}

// Credits' list is populated with owned C-heap strings by load(); unload()
// releases every slot and clears both count fields before releasing audio.
// ORIGINAL: 0x00428830 ?unload@Credits@@QAEXXZ 0x00428830-0x0042887D BYTE_EXACT
void Credits::unload() {
  for (int i = 0; i < 512; ++i) {
    if (lines_[i] != 0) {
      free(lines_[i]);
      lines_[i] = 0;
    }
  }
  line_count_ = 0;
  frame_count_ = 0;
  g_CREDITS_WAVE.unload();
}

// The credits text section owns up to 512 heap-allocated lines. The source
// fields and the Text buffer pointer replace the retained seed's raw offsets
// and fixed executable addresses; its original error paths are preserved.
// ORIGINAL: 0x00428880 ?load@Credits@@QAEXXZ 0x00428880-0x004289BA FILE
// RECOVERY_BINDING: v1 {"evidence": ["retired seed git:9957b40d:src/recovered/units/00428880.cpp preserves the original call order and field offsets", "Credits::unload and Credits::draw_credits establish the owned 512-line array and font usage"], "reason": "Home the credits text loader on its real Credits owner with typed members and project globals."}
void Credits::load() {
  unload();
  g_CREDITS_WAVE.load(const_cast<char *>("fx\\opening menu.wav"), 2);

  if (text_open("CREDITS", "CREDITS") == 0) {
    field_1230_ = 0;
    while (*text_get() != '#') {
      char *line = TextBufferGetPtr;
      char *stored = static_cast<char *>(mem_get(strlen(line) + 1));
      lines_[line_count_] = stored;
      if (stored != 0) {
        ++line_count_;
        strcpy(stored, line);
        const int width = font_.width(stored);
        if (static_cast<int>(field_1230_) <= width) {
          field_1230_ = width;
        }
      }
    }
    text_close();
  }
}

// Close the owned timer and graphics window, then the embedded button.
// ORIGINAL: 0x004289C0 ?close@Credits@@QAEXXZ 0x004289C0-0x004289E9 BYTE_EXACT
void Credits::close() {
  time_.close();
  GraphicWin::close();
  flatButton_.close();
}

// The roll owns its outer window, a child drawing surface, font, timer and
// line storage. The retained seed's virtual slots 1/57/90 are Win::show,
// Win::set_modal and FlatButton::close on these real subobjects.
// ORIGINAL: 0x004289F0 ?exec@Credits@@QAEXXZ 0x004289F0-0x00428BF2 FILE
// RECOVERY_BINDING: v1 {"evidence": ["retired seed git:024b38ed:src/recovered/units/004289f0.cpp records the 514-byte call order", "Win virtual slots 1 and 57 are show and set_modal; Credits::close owns the same Time/GraphicWin/FlatButton teardown"], "reason": "Run the credits lifecycle through real C++ owners instead of an empty link placeholder and hand-built vtable calls."}
void Credits::exec() {
  close();
  GraphicWin::init(0, 0, WinScreenWidth, WinScreenHeight, 0, 0, 0, 0, 0);

  StringTemp[0] = 0;
  strcat(StringTemp, WinScreenWidth == 800 ? "credits.pcx" : "credits2.pcx");
  Buffer::load_pcx(StringTemp, 0, 10, 0xEC);

  graphicWin2_.init(30, WinScreenWidth == 800 ? 100 : 200,
                    400, 400, 0, 0x1000020, this, 0, 0);
  graphicWin2_.fill(9);
  graphicWin2_.field_A4_ = reinterpret_cast<uint32_t>(FadeColorTables);
  graphicWin2_.show(0);

  frame_count_ = 0;
  font_.init("Alpha Centauri", 14, 1);
  line_height_ = font_.line_advance();
  load();
  static_cast<Buffer *>(&graphicWin2_)->set_font(&font_, 0, 0, 0);
  static_cast<Buffer *>(this)->set_font(&font_, 0, 0, 0);

  time_.set_modal();
  time_.start(credits_timer_tick, reinterpret_cast<int>(this), 30, 5);
  if ((GamePreferences & 0x800) != 0) {
    g_CREDITS_WAVE.play();
  }
  show(0);
  set_modal(0, pop_wait, 0);
  time_.release_modal();
  unload();
  close();
}

// The timer entry points and renderer are expressed through the Credits
// members and embedded GraphicWin/Buffer rather than seed pointer arithmetic.
// ORIGINAL: 0x00428DC0 ?timer_callback_daemon@Credits@@QAEXXZ 0x00428DC0-0x00428DCB BYTE_EXACT
void Credits::timer_callback_daemon() {
  ++frame_count_;
  draw_credits();
}

// ORIGINAL: 0x00428DD0 ?timer_callback_daemon@Credits@@QAAXH@Z 0x00428DD0-0x00428DE7 BYTE_EXACT
void __cdecl Credits::timer_callback_daemon(int) {
  if (this) {
    ++frame_count_;
    draw_credits();
  }
}

// ORIGINAL: 0x00428C00 ?draw_credits@Credits@@QAEXXZ 0x00428C00-0x00428D5C
void Credits::draw_credits() {
  Buffer *canvas = static_cast<Buffer *>(&graphicWin2_);
  graphicWin2_.fill(9);

  const int height = graphicWin2_.dib_.bmiHeader.biHeight;
  const int width = graphicWin2_.dib_.bmiHeader.biWidth;
  int y = -(static_cast<int>(frame_count_) + height);
  int line_index = 0;
  if (y <= -static_cast<int>(line_height_)) {
    line_index = -(y / static_cast<int>(line_height_));
    y %= static_cast<int>(line_height_);
  }
  if (line_index >= static_cast<int>(line_count_)) {
    release_modal();
    return;
  }

  if (y < -height) {
    int index = line_index;
    do {
      if (index >= static_cast<int>(line_count_)) {
        break;
      }
      canvas->set_text_color(0, -1, 1, 1);
      char *line = lines_[index];
      if (line != 0) {
        canvas->write_cent_l(line, 0, y, width, strlen(line));
      }
      ++index;
      y += static_cast<int>(line_height_);
      ++line_index;
    } while (y < -height);
  }

  int x = 0;
  int y2 = 0;
  for (int count = 7; count >= 0; --count) {
    canvas->change_color(0, x, width, 2, 0, count);
    const int lower_y = y2 - height - 1;
    canvas->change_color(0, lower_y, width, 2, 0, count);
    x += 2;
    y2 -= 2;
  }
  graphicWin2_.on_update();
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00428D80 ?on_left_down@Credits@@QAEXHH@Z 0x00428D80-0x00428D83 BYTE_EXACT
// symbol    ?on_left_down@Credits@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_left_down@Credits@@QAEXHH@Z)(Credits* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Credits::on_left_down(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00428D90 ?on_right_down@Credits@@QAEXHH@Z 0x00428D90-0x00428D93 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_right_down@Credits@@QAEXHH@Z)(Credits* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Credits::on_right_down(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00428DA0 ?on_mouse_move@Credits@@QAEXHH@Z 0x00428DA0-0x00428DA3 BYTE_EXACT
// symbol    ?on_mouse_move@Credits@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_mouse_move@Credits@@QAEXHH@Z)(Credits* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Credits::on_mouse_move(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00428DB0 ?on_button_clicked@Credits@@QAEXH@Z 0x00428DB0-0x00428DB3 BYTE_EXACT
// symbol    ?on_button_clicked@Credits@@UAEXH@Z
//           Q->U 2026-09-06, Win's front slots 44/45 taking the image's
//           own names (on_button_clicked / on_button_toggled): the body is
//           now an override of the window's slot, so VC6 mangles it virtual
//           and enters it on the front table, while the image spells the
//           same body Q and its own table holds the raw address.
// size      3 bytes
// prototype void (__thiscall ?on_button_clicked@Credits@@QAEXH@Z)(Credits* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Credits::on_button_clicked(int) {
}

// ===== MANAGED GLOBALS - real objects, homed to their domain =====
// In the shipped image these live at fixed data addresses and are
// constructed before WinMain by the CRT's dynamic-initializer walk
// (winedbg-confirmed: walker return 0x00644EEB, table at 0x682568..).
// Here the same recovered constructors run through this build's own
// startup, and the matching destructors close them at exit. Addresses of
// the ones documented individually live beside their definitions.
Wave g_CREDITS_WAVE;  // 0x00703E30
// ===== MANAGED GLOBALS - real objects, homed to their domain =====
// In the shipped image these live at fixed data addresses and are
// constructed before WinMain by the CRT's dynamic-initializer walk
// (winedbg-confirmed: walker return 0x00644EEB, table at 0x682568..).
// Here the same recovered constructors run through this build's own
// startup, and the matching destructors close them at exit.
