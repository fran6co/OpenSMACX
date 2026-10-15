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

#include "popup.h"
#include "time.h"

 /*
  * NetMsg class
  *
  * Derives from Popup on an unadjusted `this`. Popup is modelled here as
  * BasePop followed by its Scroll, which comes to 0x537C, and NetMsg's own
  * coordinate fields begin at 0x537C, followed by active state at 0x5384
  * and callback token at 0x5388. The owned Time sits at 0x538C.
  *
  * Current native CodeView observes sizeof(NetMsg)==0x53B4. Original
  * constructor/teardown and accessed fields prove identity/lifecycle, not
  * the complete original allocation extent or cross-TU startup ordering.
  */
  // PUBLIC: access specifiers change nothing about layout, and this
  // tree's bodies reach base methods the image reaches with a direct
  // `call rel32`. Spelled `class X : Base` - private, since that is
  // what `class` means - those calls do not compile at all, and the
  // seam that stood in for them cost the caller `call [ptr]`.
class NetMsg : public Popup {
 public:
  NetMsg(int a1, int a2, int a3);
  // Actual C++ member/base teardown, implemented in netmsg.cpp.
  ~NetMsg();
  void close();
  bool is_active() const;
  void on_button_clicked(int button_id);
  // 0x0047A890: real receiver-owned popup implementation in netmsg.cpp.
  // Declared so the diplomacy bodies can name it: every one of them reaches
  // it with `mov ecx, <singleton>; call rel32`, so the receiver has to be a
  // real NetMsg and the call a real member call.
  void pop(const char *label, int duration, int channel,
           const char *target);
  static void __cdecl pop(const char *label, const char *target);

 private:
  static void __cdecl timer_daemon(int token);
  uint32_t field_537C_;  // 0x537C, seeded from the constructor's a1
  uint32_t field_5380_;  // 0x5380, seeded from the constructor's a2
  int32_t field_5384_;   // 0x5384, seeded 0
  int32_t field_5388_;   // 0x5388, seeded from the constructor's a3 - was
                          // `unmapped_5388_[4]`; the constructor stores a
                          // real dword there (`mov [esi+0x5388], edx`).
  Time timer_;
};

// Genuine source-owned objects; constructor order and reverse teardown
// follow the original dynamic initializer/atexit evidence.
extern NetMsg NetMsgPrimary;
extern NetMsg NetMsgSecondary;
