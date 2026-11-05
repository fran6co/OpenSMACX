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
#include "stdafx.h"
#include <ddraw.h>
#include <stdint.h>

// The single DirectDraw device object Win::set_display_mode dispatches to.
// 0x009BE618 IS the object's address - the image loads it straight into ecx
// (`mov ecx, 0x9be618`) rather than reading a pointer stored there - so this
// is the actual statically stored object. The call goes through
// `WinDisplayInit.init(...)`, a real member-function
// call by name, so it still emits the image's `call rel32`. `init` itself
// (0x00635510, 587 bytes: DirectDraw surface/window teardown and
// re-creation) is defined in ddinit.cpp.
//
// FIELDS, in image order, from `sub_635450` (0x00635450, unreached in this
// build but still in the image) reading the SAME five offsets off the
// fixed `WinDisplayInit` address rather than off a `this`:
//   0x0  hwnd_           the device window `init` creates and destroys
//   0x4  surf_           the IDirectDraw device object DirectDrawCreate fills
//   0x8  field_8_        never read or written by `init` or `sub_635450`
//   0xC  locked_surface_ a secondary IDirectDrawSurface, locked elsewhere
//   0x10 locked_bits_    the pointer `locked_surface_->Unlock` releases
// `locked_surface_`/`locked_bits_` are proven by the vtable slot `init`
// calls through them: offset 0x80 is index 32, which in the real
// `IDirectDrawSurface` vtable (buffer.h's `#include <ddraw.h>`) is
// `Unlock(LPVOID)` - one pointer argument, matching the single value
// pushed beside the receiver.
class DDInit { public:
  DDInit();
  ~DDInit();
  // Native6354F0 retains ECX while forwarding depth to init(0,0,depth,1).
  // Descriptive name; the old free-stdcall seed lost the receiver contract.
  int init_default_mode(int depth);
  int init(int width, int height, int depth, int tgl);

  // 0x00635870, 1691 bytes - a `switch` over ~99 `DDERR_*` codes that turns
  // a failed HRESULT into a message box. The complete body is in ddinit.cpp;
  // `init`'s own call site proves the shape: `this` in ecx, one explicit
  // stack argument, `call rel32`.
  int report_error(int hr);   // 0x00635870, defined in ddinit.cpp

  // 0x00635750, PROMOTED out of src/unrecovered/00635750.cpp - the teardown
  // stages of `init`'s own mode switch as their own member: unlock and
  // release the locked surface, restore the display mode and release
  // DirectDraw, destroy the mode's window, then refresh the metrics.
  // jackal_close runs it on the global object below.
  void teardown();            // 0x00635750, defined in ddinit.cpp

  // 0x006357D0, initially homed in win.cpp; now owned in ddinit.cpp. Locks
  // `locked_surface_` and
  // caches the descriptor's lpSurface in `locked_bits_` and its lPitch in
  // `pitch_`, returning the bits pointer it already holds on a second call.
  LPVOID lock_surface();      // 0x006357D0, defined in ddinit.cpp

  // 0x00635840, initially homed in win.cpp; now owned in ddinit.cpp. Unlocks
  // `locked_surface_` and
  // clears `locked_bits_` - the lock half of the pair `teardown` runs, as its
  // own entry point.
  void flush();               // 0x00635840, defined in ddinit.cpp

  // SELF-VERIFYING ANCHOR: DDInit has no base and no virtual, so its first
  // member is at 0 - and field_8_, two members later, states its own offset.
  // Annotating hwnd_ at 0x0 is only correct if the walk lands field_8_ on
  // 0x8, which it does.
  // RECOVERY_MEMBER: v1 DDInit hwnd_ 0x0 0x006354D0
  HWND hwnd_;  // 0x0
  // RECOVERY_MEMBER: v1 DDInit surf_ 0x4 0x006354D0
  IDirectDraw *surf_;
  // RECOVERY_MEMBER: v1 DDInit field_8_ 0x8 0x006354D0
  uint32_t field_8_;
  // RECOVERY_MEMBER: v1 DDInit locked_surface_ 0xC 0x006354D0
  IDirectDrawSurface *locked_surface_;
  // RECOVERY_MEMBER: v1 DDInit locked_bits_ 0x10 0x006354D0
  LPVOID locked_bits_;
  // 0x14. The stride `lock_surface` caches out of the descriptor's lPitch,
  // beside the bits pointer it caches in `locked_bits_`. `lock_surface` is
  // the only writer and nothing in the recovered tree reads it, which is why
  // the five-field survey above missed it - 0x006357D0 is what proves it.
  // RECOVERY_MEMBER: v1 DDInit pitch_ 0x14 0x006357D0
  int pitch_;
  // Native6354D0 clears these distinct words after skipping pitch at +14.
  // Their roles and the complete object's upper bound remain unverified.
  // RECOVERY_MEMBER: v1 DDInit unknown_18_ 0x18 0x006354D0
  uint32_t unknown_18_;
  // RECOVERY_MEMBER: v1 DDInit unknown_1C_ 0x1C 0x006354D0
  uint32_t unknown_1C_;
};

// The object itself at 0x009BE618, not a pointer to one.
extern DDInit WinDisplayInit;    // 0x009BE618, the object, not a pointer to it
