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
#include "header_io.h"

/*
Purpose: Read the specified header from a file. This assumes the header string buffer is at least 
         256 characters. TODO: Replace built-in versions of _fgetc and change return to std::string.
// ORIGINAL: 0x0057D1F0 ?header_check@@YAXPADPAUFILE@@@Z 0x0057D1F0-0x0057D235 BYTE_EXACT
// LEVER: `header_chr` as `char` (not `int`) so the compare/store stay byte
//        width; `int i` declared and zeroed BEFORE the first `fgetc` call,
//        not inside the guard block; and the do-while-with-break rewritten
//        as a plain `while (header_chr) { if (++i >= 256) break; ... }` -
//        the do-while shape let VC6 fold the loop counter's first
//        increment into its initial value and rotate the loop, which the
//        image's own top-tested-counter / bottom-tested-char shape never
//        does.
// symbol    ?header_check@@YAXPADPAU_iobuf@@@Z
// size      69 bytes
// prototype void (__cdecl ?header_check@@YAXPADPAUFILE@@@Z)(int8* header, FILE* file)
// callers   2   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00646855
Return Value: n/a
Status: Complete
*/
void __cdecl header_check(LPSTR header, FILE *file) {
    int i = 0;
    char header_chr = (char)fgetc(file);
    *header++ = header_chr;
    while (header_chr) {
        if (++i >= 256) {
            break;
        }
        header_chr = (char)fgetc(file);
        *header++ = header_chr;
    }
    fgetc(file);
}

/*
Purpose: Write the specified header to a file. TODO: Replace built-in versions of _fputc.
// ORIGINAL: 0x0057D240 ?header_write@@YAXPADPAUFILE@@@Z 0x0057D240-0x0057D270 BYTE_EXACT
// LEVER: `header_chr` as `char`, not `int` - the image keeps the byte in
//        `bl` for the loop's `test bl,bl` and only sign-extends it
//        (`movsx eax, bl`) at the `fputc` push, which is the local matching
//        the width it reads from.
// symbol    ?header_write@@YAXPBDPAU_iobuf@@@Z
// size      48 bytes
// prototype 
// callers   2   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00646886
Return Value: n/a
Status: Complete
*/
void __cdecl header_write(LPCSTR header, FILE *file) {
    char header_chr;
    do {
        header_chr = *header++;
        fputc(header_chr, file);
    } while (header_chr);
    fputc(0x1A, file);
}

