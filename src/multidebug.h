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
#include "font.h"
#include "time.h"

class Net;

 /*
  * MultiDebug class
  *
  * Derives from GraphicWin by ordinary single inheritance: the constructor
  * calls GraphicWin's on an unadjusted `this`, which is checked rather than
  * inferred from the call alone - four headers here previously declared a
  * base that was really a member at a nonzero offset.
  *
  * GraphicWin is pinned at 0xA14, which fixes the origin for the two fields
  * below. Where the object ends is not established.
  */
// RECOVERY_BASE: v1 MultiDebug GraphicWin 0x0 0x005C97F0
class MultiDebug : public GraphicWin {
 public:
  // Time passes one cdecl integer payload, not a hidden member receiver.
  static void __cdecl timer_callback_daemon(int context);
  // The original instance callback dispatches the inherited redraw slot.
  void timer_callback();
  // Original primary vtable slot 12 paints the network diagnostics.
  virtual void on_redraw();
  MultiDebug();
  // Clear the session, then destroy the declared timer, font and base.
  ~MultiDebug();
  void close();
  // Bring the debug window up on its GraphicWin base: 0x96 by 0x12c in the
  // parent given, style 0x1000002, then the face, the text buffer and the
  // once-a-second tick. `net` is remembered here and cleared by close, which
  // is what says the window belongs to one network session.
  int init(Net *net, Win *window, unsigned char *text);

 private:
  // RECOVERY_MEMBER: v2 MultiDebug font_ Font 0xA14 0x005C97F0
  Font font_;  // 0xA14, the IDB's 40-byte member covering 0xA14..0xA3C
  // RECOVERY_MEMBER: v3 MultiDebug net_ Net* 0xA3C 0x005C9860
  Net *net_;   // 0xA3C, the IDB's field_A3C: zeroed by the constructor and
               // by close, set to the session by init
  // The constructor places a Time HERE (`lea ecx,[esi+0xa40]; call
  // ??0Time@@QAE@XZ`), right after net_, before ever storing to net_
  // itself - see MultiDebug::MultiDebug() in multidebug.cpp.
  // RECOVERY_MEMBER: v2 MultiDebug timer_ Time 0xA40 0x005C97F0
  Time timer_;  // 0xA40
};

// The static debug window, including its inherited Win and Buffer subobjects.
extern MultiDebug WinBubbleWindow;
