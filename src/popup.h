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
#include "basepop.h"
#include "scroll.h"

 /*
  * Popup class
  *
  * Derives from BasePop on an unadjusted `this`, and holds a Scroll at 0x3230
  * - exactly sizeof(BasePop), so the member follows the base with nothing
  * between. Both are pinned here, at 0x3230 and 0x214C, so this much of the
  * layout is exact; where the object ends is not established.
  */
class DLLEXPORT Popup : BasePop {
 public:
  Popup() { ; }
  ~Popup() { ; }
  void close();
  void start(char *a1, const char *a2, int a3, char *a4, int a5);

 private:
  Scroll scroll_;
};

// BasePop::close is not recovered yet.
typedef void (__thiscall func_base_pop_close)(BasePop *);
extern func_base_pop_close *BasePopOriginalClose;

void __fastcall popup_close_redirect(Popup *self, void *);

// The six-argument Popup::start is not recovered; the five-argument form
// forwards to it with a null final GraphicWin argument.
typedef void (__thiscall func_popup_start_full)(Popup *, char *, const char *,
                                                int, char *, int, void *);
extern func_popup_start_full *PopupOriginalStartFull;

void __fastcall popup_start_redirect(Popup *self, void *, char *a1,
                                     const char *a2, int a3, char *a4, int a5);
