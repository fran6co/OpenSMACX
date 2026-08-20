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
#define DLLEXPORT __declspec(dllexport)

#include "vc6_compat.h"

// `<sdkddkver.h>` and `<random>` stood here behind
// `#if !defined(_MSC_VER) || _MSC_VER > 1200`, so they were reached by every
// compiler EXCEPT the only one this project builds with. VC6's SDK predates
// both: sdkddkver.h selects a target Windows version, already decided for a
// compiler that shipped in 1998, and <random> is C++11 and is used nowhere in
// this tree. Removed with the second compiler they existed for.
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <windows.h>
#include <vector>

// THE STRING ROUTINES ARE CALLS IN THE SHIPPED IMAGE, NOT INTRINSICS, and one
// line here is the whole of reproducing that.
//
// `/O2` implies `/Oi`, which expands `strcat` into an inlined `repne scasb`
// walk. The image does not have one: it makes 4,332 direct `call _strcat`,
// 875 `_strlen`, 339 `_strcpy` and 155 `_strcmp`. It DID inline the block
// routines - 19 BYTE_EXACT bodies carry a `rep stosd` or `rep movsd` that
// only `/Oi` produces - and 34 functions call `strcat` and inline a block op
// in the SAME body, which no per-file compiler flag can produce. So the split
// is not in the invocation, it is here, and it is the ordinary 1990s move:
// turn off the string expansion that bloats a binary and keep `memset`.
//
// It has to follow <string.h> (via <windows.h> above), which declares the
// four; `#pragma function` on an undeclared name is C4163. The scaffold the
// byte match compiles carries the same line in `PRELUDE`
// (tools/emit_translation_unit.py), so the measured unit and the built one
// agree about it.
#if defined(_MSC_VER) && _MSC_VER <= 1200
#pragma function(strcat, strcpy, strlen, strcmp)
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

