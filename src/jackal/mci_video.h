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

 /*
  * MCIVideo class
  *
 * The MCI video subobject AlphaMovie embeds at +0xA14. Its member extent is
 * only bounded by the accesses recovered so far.
  */
class MCIVideo {
 public:
  MCIVideo();  // ??0MCIVideo@@QAE@XZ  0x005FFD80
  void close();  // ?close@MCIVideo@@QAEXXZ  0x005FFDB0
  int load(char *movie_path, Win *target_window, int x, int y);
  int play();  // ?play@MCIVideo@@QAEHXZ  0x006001E0
  void stop();  // ?stop@MCIVideo@@QAEXXZ  0x00600270
  int is_playing();
  int status();  // 0x006002F0
  // 0x00600330, homed into general.cpp: copies the posted frame into the
  // window's own device context. No catalogue name - `blit_frame` is the
  // tree's, recorded on the body's `symbol` fact.
  void blit_frame();
  // ??1MCIVideo@@QAE@XZ at 0x004042C0 retains its canonical original mapping
  // in alphamovie.cpp; the in-class user body calls close(), then the compiler
  // destroys palette_.
  ~MCIVideo() { close(); }

  // Storage the image proves is here: its own methods reach 0x47C.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  // 7 member(s) from the IDA database, 2 named; it starts a member at 0x0, which is where src/ ends.

  // RECOVERY_MEMBER: v1 MCIVideo field_0_ 0x0 0x00600320
  uint32_t field_0_;  // 0x0
  // The pinned ctor/load/play/stop/status paths access the device ID as a
  // WORD at +4. No observed access gives +6..+7 a meaning; retain those bytes
  // untouched rather than initializing or interpreting them.
  uint16_t mciId_;  // 0x4
  unsigned char unknown_6_[2];  // 0x6, observed only as untouched adjacent bytes
  // MCI_STATUS returns this handle; load, play, and blit_frame pass it to
  // window/GDI APIs. play also preserves the image's low-word MCI callback.
  HWND window_handle_;  // 0x8
  HPALETTE palette_handle_;  // 0xC
  // Sized and pinned: sizeof(Palette) == 0x454 (palette.h), exactly the
  // extent this field already carried as raw bytes. The constructor at
  // 0x005FFD80 placement-constructs a real Palette here (`call 0x5FE2A0`
  // on `this + 0x10`).
  Palette palette_;  // 0x10
  WNDPROC previous_window_proc_;  // 0x464
  // 0x468, stored by load() from its Win* argument and checked by blit_frame.
  Win *target_window_;  // 0x468
  // 0x46C..0x478, named from 0x00600330: BitBlt's destination x, y, width
  // and height in that order, passed straight through as the four
  // destination arguments.
  int blit_x_;       // 0x46C
  int blit_y_;       // 0x470
  int blit_width_;   // 0x474
  int blit_height_;  // 0x478
};
