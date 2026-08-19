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

#include <new>

/*
Purpose: Initialize a log file.
// ORIGINAL: 0x00626040 ?init@Log@@QAEHPAD@Z 0x00626040-0x006260C3
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
    strcpy_s(log_file_, len, input);
    FILE *file = env_open(input, "wt");
    if (!file) {
        return 6;
    }
    fclose(file);
    return 0;
}

/*
Purpose: Reset the log file.
// ORIGINAL: 0x006260D0 ?reset@Log@@QAEXXZ 0x006260D0-0x006260EC BYTE_EXACT
// size      28 bytes
// prototype void (__thiscall ?reset@Log@@QAEXXZ)(Log* this)
// callers   2   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00634BB0 0x00645598
Return Value: n/a
Status: Complete
*/
void Log::reset() {
    FILE *file = env_open(log_file_, "wt");
    if (file) {
        fclose(file);
    }
}

/*
Purpose: Write to the log file with the numbers displayed in base 10.
// ORIGINAL: 0x006260F0 ?say@Log@@QAEXPADPADHHH@Z 0x006260F0-0x0062618B
// symbol    ?say@Log@@QAEXPBD0HHH@Z
// size      155 bytes
// prototype void (__thiscall ?say@Log@@QAEXPADPADHHH@Z)(Log* this, int8*, int8*, int, int, int)
// callers   1   call targets   3
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00634BB0 0x00645598 0x00647815
Return Value: n/a
Status: Complete
*/
void Log::say(LPCSTR str1, LPCSTR str2, int num1, int num2, int num3) {
    if (!log_file_ || is_disabled_ || *IsLoggingDisabled) {
        return;
    }
    FILE *file = env_open(log_file_, "at");
    if (file) {
        str2 ? fprintf_s(file, "%s %s %d %d %d\n", str1, str2, num1, num2, num3) 
            : fprintf_s(file, "%s %d %d %d\n", str1, num1, num2, num3);
        fclose(file);
    }
}

/*
Purpose: Write to the log file with the numbers displayed in base 16.
// ORIGINAL: 0x00626190 ?say_hex@Log@@QAEXPADPADHHH@Z 0x00626190-0x0062622B
// symbol    ?say_hex@Log@@QAEXPBD0HHH@Z
// size      155 bytes
// prototype void (__thiscall ?say_hex@Log@@QAEXPADPADHHH@Z)(Log* this, int8*, int8*, int, int, int)
// callers   0   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00634BB0 0x00645598 0x00647815
Return Value: n/a
Status: Complete
*/
void Log::say_hex(LPCSTR str1, LPCSTR str2, int num1, int num2, int num3) {
    if (!log_file_ || is_disabled_ || *IsLoggingDisabled) {
        return;
    }
    FILE *file = env_open(log_file_, "at");
    if (file) {
        str2 ? fprintf_s(file, "%s %s %04x %04x %04x\n", str1, str2, num1, num2, num3) 
            : fprintf_s(file, "%s %04x %04x %04x\n", str1, num1, num2, num3);
        fclose(file);
    }
}

// global
Log *Logging = (Log *)0x009BBFF8;
BOOL *IsLoggingDisabled = (BOOL *)0x009BC004;

// ORIGINAL: 0x00625F20 ??__ELogging@@YAXXZ 0x00625F20-0x00625F8B
// symbol    ?log_logging@@YAXXZ
// size      107 bytes
// prototype 
// callers   0   call targets   6
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4510 0x00634BB0 0x00645398 0x006453E0 0x00645470 0x00645598
// notes     Staged hybrid export redirect calls the source-owned initializer
void __cdecl log_logging() {
    new (Logging) Log("logfile.txt");
    atexit(log_logging_exit);
}

// ORIGINAL: 0x00625F90 ??__FLogging@@YAXXZ 0x00625F90-0x00625FAD
// symbol    ?log_logging_exit@@YAXXZ
// size      29 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00644EF2
// notes     Staged hybrid export redirect calls the source-owned exit cleanup
void __cdecl log_logging_exit() { Logging->~Log(); }

// ORIGINAL: 0x00626230 ?log_reset@@YAXXZ 0x00626230-0x0062624F
// size      31 bytes
// prototype 
// callers   2   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00634BB0 0x00645598
// notes     Staged hybrid export redirect calls the source-owned reset wrapper
void __cdecl log_reset() { Logging->reset(); }

// ORIGINAL: 0x00626250 ?log_say@@YAXPADPADHHH@Z 0x00626250-0x006262E9
// size      153 bytes
// prototype 
// callers   25   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00634BB0 0x00645598 0x00647815
// notes     Staged hybrid export redirect calls the source-owned wrapper
void __cdecl log_say(LPCSTR str1, LPCSTR str2, int num1, int num2, int num3) {
    Logging->say(str1, str2, num1, num2, num3);
}

// ORIGINAL: 0x006262F0 ?log_say@@YAXPADHHH@Z 0x006262F0-0x0062634C
// size      92 bytes
// prototype 
// callers   98   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00634BB0 0x00645598 0x00647815
// notes     Staged hybrid export redirect calls the source-owned wrapper
void __cdecl log_say(LPCSTR str1, int num1, int num2, int num3) {
    Logging->say(str1, NULL, num1, num2, num3);
}

// ORIGINAL: 0x00626350 ?log_say_hex@@YAXPADPADHHH@Z 0x00626350-0x006263E9
// size      153 bytes
// prototype 
// callers   1   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00634BB0 0x00645598 0x00647815
// notes     Staged hybrid export redirect calls the source-owned wrapper
void __cdecl log_say_hex(LPCSTR str1, LPCSTR str2, int num1, int num2, int num3) {
    Logging->say_hex(str1, str2, num1, num2, num3);
}

// ORIGINAL: 0x006263F0 ?log_say_hex@@YAXPADHHH@Z 0x006263F0-0x0062644C
// size      92 bytes
// prototype 
// callers   4   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00634BB0 0x00645598 0x00647815
// notes     Staged hybrid export redirect calls the source-owned wrapper
void __cdecl log_say_hex(LPCSTR str1, int num1, int num2, int num3) {
    Logging->say_hex(str1, NULL, num1, num2, num3);
}

// ORIGINAL: 0x00626450 ?log_set_state@@YAXH@Z 0x00626450-0x00626461
// size      17 bytes
// prototype 
// callers   5   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
// notes     Staged hybrid export redirect calls the source-owned state wrapper
void __cdecl log_set_state(BOOL state) { Logging->set_state(state); }
