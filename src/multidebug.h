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

 /*
  * MultiDebug class
  *
  * Derives from GraphicWin by ordinary single inheritance: the constructor
  * calls GraphicWin's on an unadjusted `this`, which is checked rather than
  * inferred from the call alone - four headers here previously declared a
  * base that was really a member at a nonzero offset.
  *
  * GraphicWin is pinned at 0xA14, which fixes the origin for the one field
  * below. Where the object ends is not established.
  */
class MultiDebug : public GraphicWin {
 public:
  void __cdecl timer_callback_daemon(int a2);
  MultiDebug() { ; }
  // NOT a constructor: see the "NOT a constructor" note in log.h - an
  // ordinary method drops the SEH frame a real derived GraphicWin
  // constructor picks up under /GX (FlatButton's/PullDown's own notes).
  MultiDebug *construct();
  // 0x005C9E00 is not recovered: a
  // pending_bodies forwarder, because an empty inline stub emits
  // nothing and the deleting destructor needs a `call rel32`.
  ~MultiDebug();
  void close();

 private:
  Font font_;  // 0xA14, the IDB's 40-byte member covering 0xA14..0xA3C
  int32_t field_A3C_;  // 0xA3C
  // The constructor places a Time HERE (`lea ecx,[esi+0xa40]; call
  // ??0Time@@QAE@XZ`), right after field_A3C_, before ever storing to
  // field_A3C_ itself - see MultiDebug::construct() in multidebug.cpp.
  Time timer_;  // 0xA40
};

void __fastcall multi_debug_close_redirect(MultiDebug *self, void *);
