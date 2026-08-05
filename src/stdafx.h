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

// Lower case, and that is not a style choice. The Windows SDK ships this as
// SDKDDKVer.h and mingw-w64 ships it as sdkddkver.h; both spellings resolve on
// Windows and on a case-insensitive macOS filesystem, and only the lower-case
// one resolves on Linux. With the upper-case spelling every target that pulls
// in this header - the DLL, recovery-leaf-tests, the oracle tests - fails to
// compile here, which is the whole recovery gate. Same class of defect as the
// path casefolding in bb95bb5, and invisible for the same reason.
#include <sdkddkver.h>
#include <random>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <windows.h>
#include <vector>

#define UNUSED(x) UNUSED_ ## x
