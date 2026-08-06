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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned int   uint32_t;
typedef signed int     int32_t;
typedef unsigned short uint16_t;
typedef signed short   int16_t;
typedef unsigned char  uint8_t;
typedef signed char    int8_t;

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
 * `typedef char x[cond ? 1 : -1]` - a negative array bound is an error, so a
 * false condition fails the compile. __LINE__ keeps two assertions in one
 * scope from colliding.
 */
#define OPENSMACX_ASSERT_JOIN(a, b) OPENSMACX_ASSERT_JOIN2(a, b)
#define OPENSMACX_ASSERT_JOIN2(a, b) a##b
#define static_assert(condition, message) \
  typedef char OPENSMACX_ASSERT_JOIN(vc6_static_assert_, __LINE__)[(condition) ? 1 : -1]

#define constexpr const

#endif  // _MSC_VER <= 1200
