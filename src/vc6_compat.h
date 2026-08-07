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
 * Lets this tree compile with cl 12.00.8168 - the compiler that built the
 * original image, and so the only one whose output can say whether a
 * recovered body is RIGHT rather than merely valid.
 *
 * Everything here is inert on any other compiler. It exists because the
 * source was written for a modern one and the matching work needs the old
 * one; when the reverse engineering is finished and the tree is modernised,
 * this file is what gets deleted.
 *
 * Four gaps, each measured against the real compiler rather than assumed
 * from a standard document:
 *
 * 1. NO <cstdint>. It is C++11. VC6 has the types, just not the header or
 *    the names.
 *
 * 2. THE C LIBRARY IS NOT IN NAMESPACE std. VC6 ships <cstdio> and <cstring>
 *    but they leave their contents in the global namespace, so `std::printf`
 *    is `error C2039: 'printf' : is not a member of 'std'` - 1,862 of those
 *    across src/. Pulling the names in with `using` costs nothing and beats
 *    rewriting every call site, which would have to be rewritten back during
 *    modernisation. <vector>, <string> and <sstream> need no help; VC6's are
 *    old but they work.
 *
 * 3. NO static_assert. The classic array-size trick gives the same
 *    compile-time failure, and these assertions matter more here than
 *    anywhere else in the tree: they pin the object layouts the recovered
 *    bodies index into.
 *
 * 4. NO constexpr. `const` is enough for the integral constants this tree
 *    uses it for.
 */

#if defined(_MSC_VER) && _MSC_VER <= 1200

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

typedef unsigned int   uint32_t;
typedef signed int     int32_t;
typedef unsigned short uint16_t;
typedef signed short   int16_t;
typedef unsigned char  uint8_t;
typedef signed char    int8_t;
/*
 * VC6 predates `long long` - `error C2632: 'long' followed by 'long' is
 * illegal` - and spells the 64-bit type `__int64`. These two were missing
 * from the list above, which is why general.cpp:852 could cast to `int64_t`
 * with nothing defining it.
 */
typedef          __int64 int64_t;
typedef unsigned __int64 uint64_t;

namespace std {
using ::fflush;
using ::fprintf;
using ::printf;
using ::sprintf;

using ::memcmp;
using ::memcpy;
using ::memmove;
using ::memset;

using ::strcmp;
using ::strcpy;
using ::strlen;
using ::strncmp;

using ::free;
using ::malloc;
using ::realloc;

using ::abs;
using ::sqrt;
}  // namespace std

/*
 * Types the VC6 SDK predates. DWORD_PTR is pointer-sized and this is a
 * 32-bit target, so `unsigned long` is exact rather than approximate.
 */
typedef unsigned long DWORD_PTR;
typedef unsigned long ULONG_PTR;
typedef long LONG_PTR;
typedef unsigned int uintptr_t;
typedef int intptr_t;

/* C++11. `0` is what VC6-era code used and converts to every pointer. */
#define nullptr 0

/*
 * The bounds-checked CRT arrived with VC8. These forward to the unchecked
 * originals, which is what this code did before the `_s` names existed - the
 * bound is dropped, not honoured. Fine for a tree being byte-matched against
 * a binary that had no bounds checks either, and one more thing to delete
 * during modernisation.
 */
inline int strcpy_s(char *destination, size_t, const char *source) {
  strcpy(destination, source);
  return 0;
}
inline int strcat_s(char *destination, size_t, const char *source) {
  strcat(destination, source);
  return 0;
}
/* Some call sites pass no bound at all. */
inline int strcat_s(char *destination, const char *source) {
  strcat(destination, source);
  return 0;
}
inline int strcpy_s(char *destination, const char *source) {
  strcpy(destination, source);
  return 0;
}

/*
 * std::to_string is C++11. VC6's <sstream> works, so this is the same
 * conversion the tree would have written by hand at the time.
 */
namespace std {
inline string to_string(int value) {
  char text[32];
  sprintf(text, "%d", value);
  return string(text);
}
inline string to_string(unsigned int value) {
  char text[32];
  sprintf(text, "%u", value);
  return string(text);
}
}  // namespace std
inline int sprintf_s(char *destination, size_t, const char *format, ...) {
  va_list arguments;
  va_start(arguments, format);
  int written = vsprintf(destination, format, arguments);
  va_end(arguments);
  return written;
}

/*
 * The rest of the `_s` family the tree reaches for, on the same terms as the
 * four above: the bound is dropped, not honoured, because the binary being
 * matched had no bounds checks either and these call sites were `strncpy`,
 * `_itoa` and `memcpy` before the names existed.
 *
 * `strncpy_s` comes in both spellings the tree uses - with an explicit
 * destination size and without, which on a real CRT is the template overload
 * that deduces it from an array parameter. Neither forces a terminator, and
 * that is deliberate: `strncpy_s(BonusName[i].key, text_item(), 24)` fills a
 * 24-byte field exactly, and writing a NUL at [24] to look more like the
 * modern CRT would run off the end of the very field it is copying into.
 */
inline int strncpy_s(char *destination, size_t, const char *source, size_t count) {
  strncpy(destination, source, count);
  return 0;
}
inline int strncpy_s(char *destination, const char *source, size_t count) {
  strncpy(destination, source, count);
  return 0;
}
inline int _itoa_s(int value, char *destination, size_t, int radix) {
  _itoa(value, destination, radix);
  return 0;
}
inline int memcpy_s(void *destination, size_t, const void *source, size_t count) {
  memcpy(destination, source, count);
  return 0;
}
inline int fopen_s(FILE **stream, const char *name, const char *mode) {
  *stream = fopen(name, mode);
  return *stream ? 0 : -1;
}
inline int fprintf_s(FILE *stream, const char *format, ...) {
  va_list arguments;
  va_start(arguments, format);
  int written = vfprintf(stream, format, arguments);
  va_end(arguments);
  return written;
}

/*
 * `_snprintf_s(buffer, size, _TRUNCATE, ...)` IS THE ONE WHERE THE RETURN
 * VALUE IS LOAD-BEARING. runtime_oracle.cpp:179 relies on a truncated write
 * reporting -1 so an overflowing report is caught instead of silently
 * standing as a short one - the comment there says so. VC6's `_vsnprintf`
 * has exactly that contract, returning -1 rather than the length it wanted,
 * so the check keeps working; it also leaves the buffer unterminated on
 * truncation, which is fine because that path is taken as a failure.
 */
#define _TRUNCATE ((size_t)-1)
inline int _snprintf_s(char *destination, size_t size, size_t, const char *format, ...) {
  va_list arguments;
  va_start(arguments, format);
  int written = _vsnprintf(destination, size, format, arguments);
  va_end(arguments);
  return written;
}

/*
 * The VC6 SDK returns failure from SetFilePointer as -1 without naming it;
 * the name arrived with a later SDK. Spelt `unsigned long` rather than
 * `DWORD` because this header is included before <windows.h> and DWORD does
 * not exist yet - it is the same type on Win32.
 */
#ifndef INVALID_SET_FILE_POINTER
#define INVALID_SET_FILE_POINTER ((unsigned long)-1)
#endif

/* ARRAYSIZE arrived in a later Platform SDK than the one VC6 shipped with. */
#ifndef ARRAYSIZE
#define ARRAYSIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif

/*
 * `typedef char x[cond ? 1 : -1]` - a negative array bound is an error, so a
 * false condition fails the compile. __LINE__ keeps two assertions in one
 * scope from colliding.
 */
#define OPENSMACX_ASSERT_JOIN(a, b) OPENSMACX_ASSERT_JOIN2(a, b)
#define OPENSMACX_ASSERT_JOIN2(a, b) a##b
#define static_assert(condition, message) \
  typedef char OPENSMACX_ASSERT_JOIN(vc6_static_assert_, __LINE__)[(condition) ? 1 : -1]

#define constexpr const

/*
 * `override` and `final` are C++11 spellings of things the compiler already
 * checks or does not need to. Erasing them changes no layout and no code:
 * `override` only asks to be told when a signature stops matching a virtual,
 * and `final` only forbids further derivation. Both keep their meaning on
 * every other compiler, which is where the checking still happens.
 */
#define override
#define final

/*
 * `alignas` IS ERASED, NOT TRANSLATED, and that is a real reduction rather
 * than a rename. VC6 has no `alignas` and no `__declspec(align(n))` either -
 * the latter is `error C2485: 'align' : unrecognized extended attribute`, so
 * there is nothing to forward to. Every use in this tree over-provisions a
 * `static uint8_t[]` staging buffer to 16 bytes for objects whose widest
 * member is a 4-byte int or pointer, so the alignment the compiler gives a
 * static array is already sufficient and dropping the request costs nothing
 * that is used. It is still a downgrade, and if a staged object ever gains a
 * member that genuinely needs 8- or 16-byte alignment, this is where it
 * breaks - silently, because a misaligned x86 access faults nowhere.
 */
#define alignas(n)

/*
 * 5. `for (int i = ...)` LEAKS `i` INTO THE ENCLOSING SCOPE - see
 *    OPENSMACX_VC6_FIX_FOR_SCOPE at the bottom of stdafx.h. The macro that
 *    repairs it deliberately does NOT live here, because this header is
 *    included before the STL is and the macro must not reach it.
 */

#endif  // _MSC_VER <= 1200

/*
 * `__declspec(noinline)` arrived with VC7 and `__attribute__` is GCC's. The
 * tree marks a handful of bodies noinline so the original's call survives
 * being matched; on VC6 the attribute is simply unavailable, and inlining a
 * body the original did not inline shows up as a diff rather than silently.
 */
/* `noexcept` is C++11; an exception specification changes no layout. */
#if defined(_MSC_VER) && _MSC_VER <= 1200
#define OPENSMACX_NOEXCEPT_FALSE
#else
#define OPENSMACX_NOEXCEPT_FALSE noexcept(false)
#endif

#if defined(_MSC_VER) && _MSC_VER > 1200
#define OPENSMACX_NOINLINE __declspec(noinline)
#elif defined(__GNUC__)
#define OPENSMACX_NOINLINE __attribute__((noinline))
#else
#define OPENSMACX_NOINLINE
#endif
