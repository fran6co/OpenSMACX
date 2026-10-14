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

 /*
  * Include file for standard system include files, or project specific include files that are used
  * frequently, but are changed infrequently
  */
//#ifdef _CRT_SECURE_NO_WARNINGS
//#undef _CRT_SECURE_NO_WARNINGS
//#endif
//#define _CRT_SECURE_NO_WARNINGS 1 // eventually remove
#define WIN32_LEAN_AND_MEAN       // Exclude rarely-used stuff from Windows headers

// A body defined IN-CLASS is emitted standalone only where something calls it
// out of line, and a claim measured in a .cpp needs that standalone copy to
// exist. `dllexport` forces the emission, and changes no byte of the body -
// twelve such bodies survived the removal of the blanket DLLEXPORT and still
// measure BYTE_EXACT. Removing it from these cost nine claims: seven the
// compiler stopped emitting at all, and two that then measured MISMATCH
// because the only flag set still emitting them was `/Ob0`, which inlines
// nothing - so `Spot::shutdown` called `clear` where the image inlines it.
//
// This is the whole reason anything here is exported. Nothing in the shipped
// image is.
#define MEASURED __declspec(dllexport)

#include "vc6_compat.h"

// `<sdkddkver.h>` and `<random>` stood here behind
// `#if !defined(_MSC_VER) || _MSC_VER > 1200`, so they were reached by every
// compiler EXCEPT the only one this project builds with. VC6's SDK predates
// both: sdkddkver.h selects a target Windows version, already decided for a
// compiler that shipped in 1998, and <random> is C++11 and is used nowhere in
// this tree. Removed with the second compiler they existed for.
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

// THE LIBRARY ROUTINES ON THIS LINE ARE CALLS IN THE SHIPPED IMAGE, NOT
// INTRINSICS, and one line here is the whole of reproducing that.
//
// `/O2` implies `/Oi`, which expands `strcat` into an inlined `repne scasb`
// walk. The image does not have one: it makes 4,332 direct `call _strcat`,
// 875 `_strlen`, 339 `_strcpy` and 155 `_strcmp`. The same holds for the
// two names added 2026-09-03: `call _abs` at 553 sites with not one
// inlined `cdq / xor / sub`, and `sin` as a real CRT call - `/Oi` would
// emit `fsin` and drop the `and esp,-8` the stack-passed double needs
// (trig_init). Gating the whole project on `/Oi-` instead of naming names
// measures 619 REGRESSED of 2,385 claims against the settled pair's 3 -
// the intrinsic machinery is load bearing, only these names are off.
//
// `memcpy` IS MIXED AND STAYS OFF THE LIST, measured 2026-09-03: adding it
// here flipped 33 verified claims whose small constant-size copies
// (`memcpy(&v, p, sizeof(v))` - Random::get's 4-byte copy is the named one,
// folded into a body the image spells with no call at all) inline as plain
// `mov` pairs the image has, while the large copy (Palette::copy_from's
// constant 0x400) is a real `call _memcpy` in the image. Per-site
// forwarders carry the called minority, the same device path.cpp's
// `memset_call` already is. `memset` is the same story from the start: 19
// BYTE_EXACT bodies carry a `rep stosd`/`rep movsd` that only `/Oi`
// produces (??0ButtonGroup's constant-size `memset` is one, verified at
// the instruction level - `/Oi-` turns it into `call` + `add esp`, the
// image has the `rep stosd`), 34 functions call `strcat` and inline a
// block op in the SAME body, and no per-file compiler flag can produce
// that mix. So the split is not in the invocation, it is here, and it is
// the ordinary 1990s move: turn off the expansion that bloats a binary,
// keep the block routines.
//
// It has to follow the headers that declare the names - <string.h> and
// <stdlib.h> are above (<windows.h> and directly), <math.h> was added for
// `sin`; `#pragma function` on an undeclared name is C4163, and `memcmp` joined
// the list for Net's enumeration callback (net_class.cpp), whose image calls
// the real function rather than letting /Oi expand it inline. The byte-match
// scaffold compiles through this same stdafx.h (build/compile_commands),
// so the measured unit and the built one agree about it.
#if defined(_MSC_VER) && _MSC_VER <= 1200
#pragma function(strcat, strcpy, strlen, strcmp, abs, sin, memcmp)
#endif

// Marks a parameter as deliberately unused. Expands to NOTHING, which
// works for both spellings in this tree: `int UNUSED(x)` leaves an unnamed
// parameter, and `LPSTR UNUSED(input) input` leaves a named one. It used to
// paste `UNUSED_ ## x`, so the second spelling became `LPSTR UNUSED_input
// input` - two identifiers before the name, which nothing accepts, and
// `UNUSED_input` is defined nowhere.
#define UNUSED(x)

// Window messages that postdate VC6's Platform SDK. Their values are fixed by
// the Win32 ABI and the recovered window procedures compare against them, so
// these are constants this SDK is missing rather than anything to derive.
// Defined after <windows.h> for the same reason as the macro below.
#if defined(_MSC_VER) && _MSC_VER <= 1200
#ifndef WM_INPUT
#define WM_INPUT 0x00FF
#endif
#ifndef WM_UNICHAR
#define WM_UNICHAR 0x0109
#endif
#ifndef UNICODE_NOCHAR
#define UNICODE_NOCHAR 0xFFFF
#endif
#endif

