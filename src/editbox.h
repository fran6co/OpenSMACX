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

#include "time.h"
#include "graphicwin.h"
#include "vc6_compat.h"

/*
 * EditBox owns a GraphicWin base and trailing Time member. Original 614E50
 * constructs GraphicWin on the unadjusted receiver and Time at +B4C;
 * 408010 destroys Time then GraphicWin after close. Size B74 agrees with
 * its FileWin and DiploWin member extents. Table 66A038 differs from the
 * GraphicWin table at the event slots declared below and its deleting entry.
 */
// RECOVERY_BASE: v1 EditBox GraphicWin 0x0 0x00614E50
// RECOVERY_GENERATED: v1 vtable 0x0066A038 0x0066A1A0 ??_7EditBox@@6BWin@@@ 0x00614E50
// RECOVERY_GENERATED: v1 vtable 0x0066A030 0x0066A038 ??_7EditBox@@6BBuffer@@@ 0x00614E50
class EditBox : public GraphicWin {
 public:
  ~EditBox();

  // ??0EditBox@@QAE@XZ (0x00614E50), the real bare constructor - the body
  // is recovered in editbox.cpp, with compiler-owned base/member construction.
  EditBox();

  // Override the existing Win slots; focus belongs to Win slot51, not an
  // additional GraphicWin slot. Other unnamed events retain their positions.
  virtual void on_redraw();            // slot 12
  virtual void on_mouse_move(int x, int y); // Win slot17
  virtual void on_left_down(int x, int y);   // on_left_down
  virtual void on_left_up(int x, int y);   // on_left_up
  int on_key_click(int key, int repeat) override;
  virtual int on_key_down(int key) override;         // on_key_down
  virtual void on_dialog_focus(int focused);    // on_dialog_focus
  virtual void pass_dialog_focus();     // slot 64
  int text_fits();
  int text_position(int x);
  int init(int x, int y, int width, int height, Win *parent, int focus_wrap);
  int init_with_font(Font *font, int x, int y, int width, Win *parent,
                     int unused);
  void add_window_flags(uint32_t flags) { iFlags_ |= flags; }

  // 0x006151E0, a pending_bodies forwarder; EditGroup calls it directly.
  void set_text(char *text);

  // File selectors replace the name and select its entire editable range.
  // Keep the cursor bookkeeping with the editor rather than exposing its
  // storage offsets to each dialog that performs this operation.
  void select_all_text() {
    field_B40_ = 0;
    field_B44_ = strlen(text_);
  }

  // Reusable close at 0x00614F30; this does not end member lifetimes.
  void close();

 public:
  // 614E94 initializes only the first byte; 615630 inserts into the bounded
  // 256-byte string and B14 holds its length limit.
  // RECOVERY_MEMBER: v1 EditBox text_ 0xA14 0x00614E50
  char text_[256];  // 0xA14
  uint32_t field_B14_;  // 0xB14
  uint32_t field_B18_;  // 0xB18
  uint32_t field_B1C_;  // 0xB1C
  uint32_t field_B20_;  // 0xB20
  uint32_t field_B24_;  // 0xB24
  uint32_t field_B28_;  // 0xB28
  uint32_t field_B2C_;  // 0xB2C
  uint32_t field_B30_;  // 0xB30
  uint32_t field_B34_;  // 0xB34
  uint32_t field_B38_;  // 0xB38
  uint32_t field_B3C_;  // 0xB3C
  uint32_t field_B40_;  // 0xB40
  uint32_t field_B44_;  // 0xB44
  uint32_t field_B48_;  // 0xB48
  // RECOVERY_MEMBER: v2 EditBox time_ Time 0xB4C 0x00614E50
  Time time_;  // 0xB4C, the IDB's trailing member; the class ends at 0xB74
};

// Timer callback at 0x00615E40. The image takes an integer pointer payload
// from the stack; this is not an EditBox member despite its catalogue label.
void __cdecl edit_box_caret_callback(int edit_box_address);


static_assert(sizeof(EditBox) == 0xb74,
              "EditBox size is the span edit_box_ occupies in filewin.h");
