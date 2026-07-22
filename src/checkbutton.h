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
  * CheckButton class
  *
  * Derives from GraphicWin by ordinary single inheritance - its constructor
  * calls GraphicWin's on an unadjusted `this`, so it needs no ABI workaround
  * despite sitting beside the virtually-derived buttons. Its own fields begin
  * at 0xA14; where the object ends is not established.
  *
  * The methods recovered here are bare returns or a bare constant, touching
  * no field, which is why they can be replaced ahead of that mapping.
  */
class DLLEXPORT CheckButton : GraphicWin {
 public:
  CheckButton() { ; }
  ~CheckButton() { ; }
  static void close_class();
};

void __cdecl check_button_close_class_redirect();
