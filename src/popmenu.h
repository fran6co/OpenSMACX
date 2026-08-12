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

#include "original_seam.h"
#include "basepop.h"

 /*
  * PopMenu class
  *
  * No constructor for this one survives in the catalog, so the usual check
  * cannot run. What evidences the base is init() itself: it passes `this`
  * through to BasePop::init without adjusting it, which places a BasePop at
  * offset zero exactly as an unadjusted constructor call would.
  *
  * BasePop is pinned at 0x3230. Nothing about PopMenu's own extent is
  * established.
  */
class DLLEXPORT PopMenu : BasePop {
 public:
  PopMenu() { ; }
  ~PopMenu() { ; }
  int init();

  // Storage the image proves is here: its own methods reach 0x4C70.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  uint32_t field_3230_;  // 0x3230
  uint8_t field_3234_[0x440];  // 0x3234
  uint32_t field_3674_;  // 0x3674
  uint8_t field_3678_[0x664];  // 0x3678
  uint32_t field_3CDC_;  // 0x3CDC
  uint8_t field_3CE0_[0x440];  // 0x3CE0
  uint32_t field_4120_;  // 0x4120
  uint8_t field_4124_[0x704];  // 0x4124
  uint32_t field_4828_;  // 0x4828
  uint8_t field_482C_[0x440];  // 0x482C
  uint32_t field_4C6C_;  // 0x4C6C
};

// BasePop::init is not recovered yet.
typedef int (OriginalObject::*func_base_pop_init)(int, long);
extern func_base_pop_init BasePopOriginalInit;

int __fastcall pop_menu_init_redirect(PopMenu *self, void *);
