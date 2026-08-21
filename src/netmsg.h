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
  // PUBLIC: access specifiers change nothing about layout, and this
  // tree's bodies reach base methods the image reaches with a direct
  // `call rel32`. Spelled `class X : Base` - private, since that is
  // what `class` means - those calls do not compile at all, and the
  // seam that stood in for them cost the caller `call [ptr]`.
class NetMsg : public Popup {
 public:
  NetMsg(int a1, int a2, int a3);
  // A `construct` method, not a constructor - see the note in `mapwin.h`. The
  // real body is 0x0047ACF0 and takes three ints; `??__Eg_NETMSG1` and
  // `??__Eg_NETMSG2` reach it, and a pointer-to-member seam cost both the
  // image's `E8`. Forwarded in `pending_bodies.cpp`.
  // 0x0047ADA0 is not recovered: a
  // pending_bodies forwarder, because an empty inline stub emits
  // nothing and the deleting destructor needs a `call rel32`.
  ~NetMsg();
  void close();

 private:
  uint32_t field_537C_;  // 0x537C, seeded from the constructor's a1
  uint32_t field_5380_;  // 0x5380, seeded from the constructor's a2
  int32_t field_5384_;   // 0x5384, seeded 0
  int32_t field_5388_;   // 0x5388, seeded from the constructor's a3 - was
                          // `unmapped_5388_[4]`; the constructor stores a
                          // real dword there (`mov [esi+0x5388], edx`).
  Time timer_;
};

// Popup::hide is not recovered yet.

void __fastcall net_msg_close_redirect(NetMsg *self, void *);
