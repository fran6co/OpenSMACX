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

// OPENSMACX_VC6_FIX_FOR_SCOPE
//
// VC6 uses the pre-standard rule where `for (int i = ...)` leaves `i` alive in
// the enclosing scope, so the second loop in a function that counts with `i`
// twice is `error C2374: 'i' : redefinition; multiple initialization` - 91 of
// those across twelve files, the largest single class of error in this build
// and not one of them a real defect. There is no `/Zc:forScope` on a compiler
// this old; a dead `else` gives the loop the scope the standard asks for.
//
// The `{}` is what makes it safe. Without it `if (c) for (...) x; else y;`
// would bind the `else` to the injected `if`; with it the injected pair is
// already matched, so a trailing `else` still binds where it was written.
//
// IT IS DEFINED HERE, LAST, AND NOT IN vc6_compat.h. Placed there it lands
// ahead of <sstream>, <string>, <vector> and <windows.h> and rewrites every
// loop inside the STL too - which VC6 survives syntactically but not
// otherwise: instantiating the wrapped loops took the compiler from a clean
// precompiled header to `fatal error C1060: compiler is out of heap space` on
// 95 of 138 translation units. Measured, both ways round. Everything the
// macro needs to reach is compiled after this line.
//
// IT CANNOT MOVE THE BYTE MATCH. Verification never compiles this header:
// `emit_translation_unit.py` builds a self-contained unit and `byte_match.py`
// gives it its own `/O2 /Gy /GR- /Oy- /GX`. The macro is confined to the DLL,
// where the dead branch folds away under optimisation and is unreachable
// without it.
#if defined(_MSC_VER) && _MSC_VER <= 1200
#define for if (0) {} else for
#endif
