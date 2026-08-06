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
#include "popup.h"
#include "time.h"

 /*
  * NetMsg class
  *
  * Derives from Popup on an unadjusted `this`. Popup is modelled here as
  * BasePop followed by its Scroll, which comes to 0x537C, and NetMsg's own
  * fields begin at 0x5384 - eight bytes further on, which is unmapped. The
  * Time it closes sits at 0x538C.
  *
  * Nothing pins this class's sizeof.
  */
class DLLEXPORT NetMsg : Popup {
 public:
  NetMsg() { ; }
  ~NetMsg() { ; }
  void close();

 private:
  uint8_t unmapped_537C_[0x5384 - 0x537C];
  int32_t field_5384_;
  uint8_t unmapped_5388_[0x538C - 0x5388];
  Time timer_;
};

// Popup::hide is not recovered yet.
typedef void (OriginalObject::*func_popup_hide)();
extern func_popup_hide PopupOriginalHide;

void __fastcall net_msg_close_redirect(NetMsg *self, void *);
