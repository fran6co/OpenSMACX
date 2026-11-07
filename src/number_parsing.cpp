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
#include "general.h"

/*
Purpose: Convert the binary string to an integer.
// ORIGINAL: 0x006288D0 ?btoi@@YAHPAD@Z 0x006288D0-0x006288ED BYTE_EXACT
// body      src/number_parsing.cpp
// symbol    ?btoi@@YAHPBD@Z
// size      29 bytes
// prototype int (__cdecl ?btoi@@YAHPAD@Z)(int8* input)
// callers   2   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Integer value of the string
Status: Complete
*/
// BODY IN general.h, as `MEASURED inline`: `stoi` at 0x00628950
// makes no call to it - the image writes it out inside the jump
// table's arms - while 0x006288D0 is a real body of its own.
MEASURED int __cdecl btoi(LPCSTR str) {
    int result = 0;
    while (*str == '0' || *str == '1') {
        result = *str++ - '0' + 2 * result;
    }
    return result;
}

/*
Purpose: Convert the hex string to an integer.
// ORIGINAL: 0x006288F0 ?htoi@@YAHPAD@Z 0x006288F0-0x0062894B BYTE_EXACT
// body      src/number_parsing.cpp
// symbol    ?htoi@@YAHPBD@Z
// size      91 bytes
// prototype int (__cdecl ?htoi@@YAHPAD@Z)(int8* input)
// callers   2   call targets   3
// kind      game
// flags     sp_ready;purged_ok
// calls     0x0064628F 0x006469A2 0x006469CA
Return Value: Integer value of the string
Status: Complete
*/
// BODY IN general.h, as `MEASURED inline`: `stoi` at 0x00628950
// makes no call to it - the image writes it out inside the jump
// table's arms - while 0x00628910 is a real body of its own.
MEASURED int __cdecl htoi(LPCSTR str) {
    int result = 0;
    while (isxdigit(*str)) {
        result *= 16;
        if (isdigit(*str)) {
            result += *str - '0';
        } else {
            result += toupper(*str) - '7';
        }
        *str++;
    }
    return result;
}

// RECOVERY_ASSOCIATED: v1 switch_targets 0x00628950 0x00628A00 0x00628A10 0x00628976
// RECOVERY_ASSOCIATED: v1 switch_selector 0x00628950 0x00628A10 0x00628A47 0x00628970
/*
Purpose: Converts a binary, hex or decimal string to an integer.
// ORIGINAL: 0x00628950 ?stoi@@YAHPAD@Z 0x00628950-0x006289FE BYTE_EXACT
// symbol    ?stoi@@YAHPBD@Z
// size      174 bytes
// prototype int (__cdecl ?stoi@@YAHPAD@Z)(int8* input)
// callers   2   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645DA7 0x0064628F 0x006469A2 0x006469CA
Return Value: Integer value of the string
Status: Complete
*/
int __cdecl stoi(LPCSTR str) {
    if (*str == '0') {
        *str++;
        switch (*str) {
          case 'B':
          case 'b':
            *str++;
            return btoi(str);
          case 'X':
          case 'x':
            *str++;
            return htoi(str);
          case 'D':
          case 'd':
            *str++;
            return atoi(str);
          default:
            return atoi(str);
        }
    }
    return atoi(str);
}

/*
Purpose: Locates the first number in a string.
// ORIGINAL: 0x00628B30 ?findnum@@YAHPAD@Z 0x00628B30-0x00628B69
// LEVER: the image reads its digit bounds from memory - `mov dl, byte ptr
// [0x670c1c]` / `mov dh, byte ptr [0x670c1d]`, an .rdata pair holding '0'
// and '9' - not immediates, so `*str < '0' || *str > '9'` (a pure literal
// compare) can never reproduce that. Binding a `char *const` to the fixed
// image address (the same "const pointer folds to the image's immediate"
// lever as `StringTable`) gets the identical `cmp cl, byte ptr [0x670c1c]`
// encoding; hoisting `digits[0]`/`digits[1]` into `lo`/`hi` locals BEFORE
// the loop (rather than re-reading `digits[]` each iteration) matches the
// image's loop-invariant load and moved 2/27 to 3/27 (0.667 similar under
// /O1 /Oy-, the closest of any flag set tried).
// TRIED (still short of the image): a `goto`-based single-exit form and
// a `char *result` single-return form, both meant to reproduce the image's
// shared `xor eax,eax` tail that both failure paths jump to and the
// redundant `mov [ebp-4],eax; mov eax,[ebp-4]` roundtrip before the
// epilogue - neither beat the committed body. The image also keeps the
// scanned character in a loop-top register (`dec eax; inc eax` then loop
// back to the `inc`) with an `and bl,bl` zero test and a `push ecx` frame
// slot this tree's codegen never reproduces at any flag set tried; the
// remaining gap looks like a loop-rotation/tail-merge choice the optimizer
// made under settings outside `/O1`, `/O2`, `/Ob0`, `/Oi-`, `/Oy-`, `/GX`.
// symbol    ?findnum@@YAPADPAD@Z
// size      57 bytes
// prototype
// callers   1   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: Pointer to the first number, otherwise zero
Status: Complete
*/
// `char *`, not `LPSTR`. The same type, but the verification scaffolding
// forward-declares only types reachable from a signature, so the Windows
// typedef made this body NO_COMPILE and unscoreable.
// The digit bounds, read from MEMORY: the image's .rdata carries the literal
// "09" at 0x670C1C and both comparisons load through it. The literal used to
// be reached as `char *const digits = (char *)0x00670C1C`, naming storage
// inside terranx.exe - unmapped here, so the body read garbage. This array is
// that storage: the image's own bytes ('0', '9', NUL), and the loads fold to
// the same `byte ptr [disp32]` form, the displacement relocated.
char findnum_digits[] = "09";  // 0x00670C1C

// Hand-written assembly: `and reg,reg` tests, dl/dh digit bounds, result via local.
char *__cdecl findnum(char *str) {
    char *result;
    __asm {
        mov eax, str
        and eax, eax
        je not_found
        mov dl, byte ptr [findnum_digits]
        mov dh, byte ptr [findnum_digits + 1]
        xor ebx, ebx
        dec eax
    next_char:
        inc eax
        mov bl, [eax]
        and bl, bl
        je not_found
        cmp bl, dl
        jl next_char
        cmp bl, dh
        jg next_char
        jmp done
    not_found:
        xor eax, eax
    done:
        mov result, eax
    }
    return result;
}

