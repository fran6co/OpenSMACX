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

// THE GAME AND JACKAL WERE COMPILED WITHOUT /Oi: NO INTRINSIC EXPANDS.
//
// Image census outside the separately built Caviar and UV libraries
// (2026-11-18): 4,332 `call _strcat`, 875 `_strlen`, 339 `_strcpy`, 155
// `_strcmp`, 125 `_memcpy` (77 with a constant size, 48 variable), 69
// `_memset` (50 constant), 18 `_memcmp`, and not one expansion: no
// `repne scasb`, no `fsqrt`/`fsin`/`fcos`/`fpatan`/`fabs`, no variable-size
// `rep movsd`/`rep movsb` pair. Every `rep movsd`/`rep stosd` the image has
// there is a structure assignment, an aggregate initializer or a copy/zero
// loop that VC6's /Og turns into a block operation (message_big_data
// 0x00592FF0 is two such loops), none of which needs /Oi.
//
// WORKAROUND(decl): the unit flags still carry /Oi (the build is the
// coordinator's), so this line names every intrinsic the headers declare
// instead. It stands in for dropping /Oi from the game and Jackal units;
// once that lands this line and the Caviar/UV `#pragma intrinsic` lines
// that undo it go away together. A body that needs `memset`/`memcpy` to
// expand here is spelled wrong: write the loop or the assignment.
#if defined(_MSC_VER) && _MSC_VER <= 1200
#pragma function(strcat, strcpy, strlen, strcmp, abs, sin, cos, memcmp)
#pragma function(memcpy, memset, sqrt, atan, atan2, pow, fabs, labs, exp, log, log10, tan, acos, asin, cosh, sinh, tanh, fmod)
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

