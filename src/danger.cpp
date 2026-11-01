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
// The pinned diagnostic wrapper calls the parsing helpers out of line.
// This separate recovery owner preserves that boundary; its historical TU name is unknown.
#include "stdafx.h"
#include "general.h"
#include "game.h"
#include "log.h"
#include "popup.h"

/*
Purpose: Create a debug error pop-up then write all the parameters to the log file.
// ORIGINAL: 0x00538F30 ?danger@@YAXPADPADHHH@Z 0x00538F30-0x00538FAA BYTE_EXACT
// symbol    ?danger@@YAXPBD0HHH@Z
// size      122 bytes
// prototype 
// callers   3   call targets   4
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0048C0A0 0x00625E30 0x00625EC0 0x00626250
Return Value: n/a
Status: Complete
*/
void __cdecl danger(LPCSTR msg1, LPCSTR msg2, int num1, int num2, int num3) {
    parse_says(0, msg1, -1, -1);
    parse_says(1, msg2, -1, -1);
    parse_num(0, num1);
    parse_num(1, num2);
    parse_num(2, num3);
    log_say(msg1, msg2, num1, num2, num3);
    // The original immediates at 0x00538F90 and 0x00538F97 point to the
    // independently bounded DANGER and hasty_sm.pcx literals below.
// RECOVERY_LITERAL: v1 0x0068D428 0x0068D42F ??_C@_06LCPB@DANGER?$AA@ 0x00538F30
// RECOVERY_LITERAL: v1 0x0068D418 0x0068D425 ??_C@_0N@LFMH@hasty_sm?4pcx?$AA@ 0x00538F30
    popp(ScriptPopupFile, "DANGER", 0, "hasty_sm.pcx", 0);
}
