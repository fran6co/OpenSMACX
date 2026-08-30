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
#include "stdafx.h"
#include "temp.h"
#include "log.h"

/*
Purpose: Initialize a log file.
// ORIGINAL: 0x00626040 ?init@Log@@QAEHPAD@Z 0x00626040-0x006260C3 BYTE_EXACT
// LEVER: same idiom as Log::construct - clear the first byte, then `strcat`
// into it (0x645470) - rather than `strcpy_s`, which the image does not call.
// symbol    ?init@Log@@QAEHPBD@Z
// size      131 bytes
// prototype int (__thiscall ?init@Log@@QAEHPAD@Z)(Log* this, int8*)
// callers   2   call targets   6
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4510 0x00634BB0 0x00644EF2 0x006453E0 0x00645470 0x00645598
Return Value: Zero on success, non-zero on error
Status: Complete
*/
int Log::init(LPCSTR input) {
    if (!input) {
        return 16;
    }
    this->~Log();
    size_t len = strlen(input) + 1;
    log_file_ = (LPSTR)mem_get(len);
    if (!log_file_) {
        return 4;
    }
    *log_file_ = '\0';
    strcat(log_file_, input);
    FILE *file = env_open(input, "wt");
    if (!file) {
        return 6;
    }
    fclose(file);
    return 0;
}

// global
// AN OBJECT, NOT A POINTER TO A FIXED ADDRESS: the pointer form costs a
// load at every use where the image addresses the storage directly, and
// the address is terranx.exe's data, unmapped in a standalone build.
BOOL IsLoggingDisabled;  // 0x009BC004

// THE OBJECT, at 0x009BBFF8 in the image - was `Log *const Logging =
// (Log *)0x009BBFF8` (see log.h). Zero at load there, as here; the image's
// ??__ELogging (log_logging below) reaches `construct` as an ordinary method
// against this storage, and the real constructor runs ahead of it in both
// binaries.
Log Logging;  // 0x009BBFF8

// ORIGINAL: 0x00625F20 ??__ELogging@@YAXXZ 0x00625F20-0x00625F8B BYTE_EXACT
// symbol    ?log_logging@@YAXXZ
// size      107 bytes
// prototype 
// callers   0   call targets   6
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4510 0x00634BB0 0x00645398 0x006453E0 0x00645470 0x00645598
// notes     Staged hybrid export redirect calls the source-owned initializer
// LEVER: `Logging.construct("logfile.txt")` (an ordinary method, see
//        log.h) rather than `new (Logging) Log(...)` - the constructor
//        spelling wraps the body in an SEH frame the image does not have.
// LEVER: Logging as the REAL OBJECT (log.h/log.cpp, 2026-08-29) instead of
//        the `Log *const Logging = (Log *)0x009BBFF8` fixed-address binding.
//        Was 2/107 through the binding; the object's folded receiver measured
//        this in, and `record` banked it on that change.
void __cdecl log_logging() {
    Logging.construct("logfile.txt");
    atexit(log_logging_exit);
}

// ORIGINAL: 0x00625F90 ??__FLogging@@YAXXZ 0x00625F90-0x00625FAD BYTE_EXACT
// symbol    ?log_logging_exit@@YAXXZ
// size      29 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00644EF2
// notes     Staged hybrid export redirect calls the source-owned exit cleanup
// LEVER: Logging as the REAL OBJECT (log.h/log.cpp, 2026-08-29) instead of
//        the `Log *const` fixed-address binding; measured in on that change.
void __cdecl log_logging_exit() { Logging.~Log(); }

// ORIGINAL: 0x00626230 ?log_reset@@YAXXZ 0x00626230-0x0062624F BYTE_EXACT
// size      31 bytes
// prototype 
// callers   2   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00634BB0 0x00645598
// notes     Staged hybrid export redirect calls the source-owned reset wrapper
void __cdecl log_reset() { Logging.reset(); }

// ORIGINAL: 0x00626250 ?log_say@@YAXPADPADHHH@Z 0x00626250-0x006262E9 BYTE_EXACT
// symbol    ?log_say@@YAXPBD0HHH@Z
// size      153 bytes
// prototype 
// callers   25   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00634BB0 0x00645598 0x00647815
// notes     Staged hybrid export redirect calls the source-owned wrapper
// LEVER: Logging as the REAL OBJECT (log.h/log.cpp, 2026-08-29) instead of the `Log *const` binding - through the binding the first log_file_ read materialised the receiver (`mov eax,0x9bbff8; mov eax,[eax]`) where the image folds `mov eax,[0x9bbff8]`; the object folds the same direct load and each of these four measured in on the change.
void __cdecl log_say(LPCSTR str1, LPCSTR str2, int num1, int num2, int num3) {
    Logging.say(str1, str2, num1, num2, num3);
}

// ORIGINAL: 0x006262F0 ?log_say@@YAXPADHHH@Z 0x006262F0-0x0062634C BYTE_EXACT
// symbol    ?log_say@@YAXPBDHHH@Z
// size      92 bytes
// prototype 
// callers   98   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00634BB0 0x00645598 0x00647815
// notes     Staged hybrid export redirect calls the source-owned wrapper
// LEVER: Logging as the REAL OBJECT (log.h/log.cpp, 2026-08-29) instead of the `Log *const` binding - through the binding the first log_file_ read materialised the receiver (`mov eax,0x9bbff8; mov eax,[eax]`) where the image folds `mov eax,[0x9bbff8]`; the object folds the same direct load and each of these four measured in on the change.
void __cdecl log_say(LPCSTR str1, int num1, int num2, int num3) {
    Logging.say(str1, NULL, num1, num2, num3);
}

// ORIGINAL: 0x00626350 ?log_say_hex@@YAXPADPADHHH@Z 0x00626350-0x006263E9 BYTE_EXACT
// symbol    ?log_say_hex@@YAXPBD0HHH@Z
// size      153 bytes
// prototype 
// callers   1   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00634BB0 0x00645598 0x00647815
// notes     Staged hybrid export redirect calls the source-owned wrapper
// LEVER: Logging as the REAL OBJECT (log.h/log.cpp, 2026-08-29) instead of the `Log *const` binding - through the binding the first log_file_ read materialised the receiver (`mov eax,0x9bbff8; mov eax,[eax]`) where the image folds `mov eax,[0x9bbff8]`; the object folds the same direct load and each of these four measured in on the change.
void __cdecl log_say_hex(LPCSTR str1, LPCSTR str2, int num1, int num2, int num3) {
    Logging.say_hex(str1, str2, num1, num2, num3);
}

// ORIGINAL: 0x006263F0 ?log_say_hex@@YAXPADHHH@Z 0x006263F0-0x0062644C BYTE_EXACT
// symbol    ?log_say_hex@@YAXPBDHHH@Z
// size      92 bytes
// prototype 
// callers   4   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00634BB0 0x00645598 0x00647815
// notes     Staged hybrid export redirect calls the source-owned wrapper
// LEVER: Logging as the REAL OBJECT (log.h/log.cpp, 2026-08-29) instead of the `Log *const` binding - through the binding the first log_file_ read materialised the receiver (`mov eax,0x9bbff8; mov eax,[eax]`) where the image folds `mov eax,[0x9bbff8]`; the object folds the same direct load and each of these four measured in on the change.
void __cdecl log_say_hex(LPCSTR str1, int num1, int num2, int num3) {
    Logging.say_hex(str1, NULL, num1, num2, num3);
}

// ORIGINAL: 0x00626450 ?log_set_state@@YAXH@Z 0x00626450-0x00626461 BYTE_EXACT
// LEVER: Log::set_state as `is_disabled_ = (state == 0);` rather than the
// ternary `state ? false : true;` - the comparison form is the xor/sete idiom
// (`xor eax,eax; test ecx,ecx; sete al`), the ternary needed a trailing
// `and eax, 0xff` the image does not have.
// size      17 bytes
// prototype 
// callers   5   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
// notes     Staged hybrid export redirect calls the source-owned state wrapper
void __cdecl log_set_state(BOOL state) { Logging.set_state(state); }


// ---------------------------------------------------------------------------
// DEFINED IN THE HEADER, CLAIMED HERE.
//
// These pieces are written in-class so the image's own inlining reproduces -
// a constructor or destructor the compiler is expected to fold into its
// caller. A marker cannot live beside them: `decomp.reader` globs `*.cpp`
// and `*.c`, and a comparison compiles a TRANSLATION UNIT, so a marker in a
// header could be neither read nor measured. VC6 emits each of them into
// this unit's object as its own COMDAT anyway, which is what the comparison
// pulls out, and the `body` fact says where to go to edit one.
//
// The ratchet still covers the header: this unit includes it, so breaking an
// in-class body here fails the claim below. Measured, not assumed.
// ---------------------------------------------------------------------------

/*
// ORIGINAL: 0x00625FB0 ??0Log@@QAE@XZ 0x00625FB0-0x00625FC0 BYTE_EXACT
// body      src/log.h
// size      16 bytes
// prototype Log* (__thiscall ??0Log@@QAE@XZ)(Log* this)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Staged hybrid export redirect calls the source-owned constructor
*/

/*
// ORIGINAL: 0x00625FC0 ??0Log@@QAE@PAD@Z 0x00625FC0-0x00626016 BYTE_EXACT
// LEVER: not a constructor - see the "NOT a constructor" note in log.h. `Logging.construct(input)` is an ordinary method (`??__ELogging` reaches it that way, not through a `new`-expression), which drops the SEH frame VC6 wraps around a placement-new'd non-trivial destructor. Inlines env_open/fclose directly on `input` rather than calling `reset()`, because the image keeps the incoming pointer live in edi and reopens THAT, not a reload of log_file_.
// body      src/log.h
// symbol    ?construct@Log@@QAEPAV1@PBD@Z
// size      86 bytes
// prototype Log* (__thiscall ??0Log@@QAE@PAD@Z)(Log* this, int8*)
// callers   0   call targets   5
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005D4510 0x00634BB0 0x006453E0 0x00645470 0x00645598
// notes     Staged hybrid export redirect calls the source-owned filename constructor
*/

/*
// ORIGINAL: 0x00626020 ??1Log@@QAE@XZ 0x00626020-0x0062603A BYTE_EXACT
// body      src/log.h
// size      26 bytes
// prototype void (__thiscall ??1Log@@QAE@XZ)(Log* this)
// callers   1   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00644EF2
// notes     Staged hybrid export redirect calls the source-owned destructor
*/

/*
Purpose: Reset the log file.
// ORIGINAL: 0x006260D0 ?reset@Log@@QAEXXZ 0x006260D0-0x006260EC BYTE_EXACT
// body      src/log.h
// LEVER: moved in-class (MEASURED) - the free-function wrappers in this file
//        inline it, matching the image's own inlining at those call sites.
// size      28 bytes
// prototype void (__thiscall ?reset@Log@@QAEXXZ)(Log* this)
// callers   2   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00634BB0 0x00645598
Return Value: n/a
*/

/*
Purpose: Write to the log file with the numbers displayed in base 10.
// ORIGINAL: 0x006260F0 ?say@Log@@QAEXPADPADHHH@Z 0x006260F0-0x0062618B BYTE_EXACT
// body      src/log.h
// LEVER: moved in-class (MEASURED) - the free-function wrappers in this file
//        inline it, matching the image's own inlining at those call sites.
// symbol    ?say@Log@@QAEXPBD0HHH@Z
// size      155 bytes
// prototype void (__thiscall ?say@Log@@QAEXPADPADHHH@Z)(Log* this, int8*, int8*, int, int, int)
// callers   1   call targets   3
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00634BB0 0x00645598 0x00647815
Return Value: n/a
*/

/*
Purpose: Write to the log file with the numbers displayed in base 16.
// ORIGINAL: 0x00626190 ?say_hex@Log@@QAEXPADPADHHH@Z 0x00626190-0x0062622B BYTE_EXACT
// body      src/log.h
// LEVER: moved in-class (MEASURED) - the free-function wrappers in this file
//        inline it, matching the image's own inlining at those call sites.
// symbol    ?say_hex@Log@@QAEXPBD0HHH@Z
// size      155 bytes
// prototype void (__thiscall ?say_hex@Log@@QAEXPADPADHHH@Z)(Log* this, int8*, int8*, int, int, int)
// callers   0   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00634BB0 0x00645598 0x00647815
Return Value: n/a
*/
