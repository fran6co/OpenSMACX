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
class DLLEXPORT MultiDebug : GraphicWin {
 public:
  void __cdecl timer_callback_daemon(int a2);
  MultiDebug() { ; }
  ~MultiDebug() { ; }
  void close();

 private:
  uint32_t field_A14_;
  uint32_t field_A18_;
  uint32_t field_A1C_;
  uint32_t field_A20_;
  uint32_t field_A24_;
  uint32_t field_A28_;
  uint32_t field_A2C_;
  uint32_t field_A30_;
  uint32_t field_A34_;
  uint32_t field_A38_;
  int32_t field_A3C_;
};

void __fastcall multi_debug_close_redirect(MultiDebug *self, void *);
