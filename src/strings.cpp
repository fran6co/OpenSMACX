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
#include "strings.h"

/*
Purpose: Initialize the class instance with a new string table of the specified size.
ORIGINAL: 0x006168F0
// name      ?init@Strings@@QAEHH@Z
// size      96 bytes
// spans     0x006168F0-0x00616950
// prototype int (__thiscall ?init@Strings@@QAEHH@Z)(Strings* this, int)
// callers   4   call targets   5
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4580 0x005D4620 0x005D4680 0x006453E0 0x00645460
Return Value: Was there an error? true/false
Status: Complete
*/
BOOL Strings::init(size_t mem_size) {
    if (is_populated_) {
        shutdown();
    }
    if (Heap::init(mem_size)) {
        return true; // allocation failed
    }
    put("-Nil-");
    is_populated_ = true;
    return false; // successful
}

/*
Purpose: Shutdown the class instance.
ORIGINAL: 0x00616950
// name      ?shutdown@Strings@@QAEHXZ
// size      19 bytes
// spans     0x00616950-0x00616963
// prototype int (__thiscall ?shutdown@Strings@@QAEHXZ)(Strings* this)
// callers   4   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4580
Return Value: n/a
Status: Complete
*/
void Strings::shutdown() {
    Heap::shutdown();
    is_populated_ = false;
}

/*
Purpose: Put the input string into the string table.
ORIGINAL: 0x00616970
// name      ?put@Strings@@QAEHPAD@Z
// size      45 bytes
// spans     0x00616970-0x0061699D
// prototype int (__thiscall ?put@Strings@@QAEHPAD@Z)(Strings* this, int8*)
// callers   5   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4680 0x006453E0 0x00645460
Return Value: Address of the stored string
Status: Complete
*/
LPSTR Strings::put(LPCSTR input) {
    size_t len = strlen(input) + 1;
    LPSTR table_addr = LPSTR(Heap::get(len));
    strcpy_s(table_addr, len, input);
    return table_addr;
}

/*
Purpose: Check if the string address is within the table.
ORIGINAL: 0x006169A0
// name      ?get@Strings@@QAEHH@Z
// size      27 bytes
// spans     0x006169A0-0x006169BB
// prototype int (__thiscall ?get@Strings@@QAEHH@Z)(Strings* this, int)
// callers   290   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: Address of the stored string
Status: Complete
*/
LPSTR Strings::get(int address) {
    if ((LPVOID)address > get_base()) { // checking if ptr is after base
        // checking ptr is not after end of table
        return ((LPSTR)address <= LPSTR(size_t(get_base()) + get_base_size())) 
            ? (LPSTR)address : NULL;
    }
    return LPSTR(get_base()); // return base
}

// global
Strings *StringTable = (Strings *)0x009B90D8;
