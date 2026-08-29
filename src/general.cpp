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
#include <math.h> // sin(double), lowered to the CRT's _sin
#include "sprite.h"
#include "popup.h"
#include "buffer.h"
#include "main.h"  // PopupAllocHook - the tree's own 0x00696ECC slot

// close_class methods, homed to checkbox.cpp / radiobutton.cpp
void __cdecl teardown_0060fd60();
void __cdecl teardown_0060e5d0();
extern Sprite g_BLANK_SPRITE;
void __cdecl teardown_0063cef0();
#include "temp.h"
#include "general.h"
#include "alpha.h"
#include "game.h"
#include "log.h" // log_say
#include "mapwin.h" // draw_map, promoted out of temp.h
#include "strings.h"
#include "text.h"
// jackal_init_real brings all of these up, in this order.
#include "palette.h"
#include "win.h"
#include "buffer.h"
#include "checkbox.h"
#include "checkbutton.h"
#include "radiobutton.h"
#include "basebutton.h"
#include "font.h"
#include "basepop.h"
#include "filewin.h"
#include "cursor.h"
#include "time.h"

uint32_t ScenEditorUndoPosition = 1; // 0x00690D7C
// GenderDefault (0x009BBFEC) and PluralityDefault (0x009BBFF0) are defined in
// general.h as `static T *const`, so the compiler can see the address. See the
// comment there for the measurement.

/*
Purpose: Trim the trailing spaces in-line from the end of the string.
// ORIGINAL: 0x00600780 ?purge_trailing@@YAXPAD@Z 0x00600780-0x006007AD BYTE_EXACT
// size      45 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006453E0
Return Value: n/a
Status: Complete
*/
// LEVER: definition moved to general.h (MEASURED inline) so callers in
// alpha.cpp fold it in place instead of emitting a `call`; chas_name,
// weap_name and arm_name each showed a real `call` to this address where the
// image inlines the four-`strlen`-call body directly. See general.h.

/*
Purpose: Trim the leading spaces in-line from the start of the string.
// ORIGINAL: 0x00600760 ?purge_leading@@YAXPAD@Z 0x00600760-0x00600773 BYTE_EXACT
// symbol    ?purge_leading@@YAPADPAD@Z
// size      19 bytes
// prototype 
// callers   2   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
__forceinline LPSTR __cdecl purge_leading(LPSTR input) {
    // IT DOES NOT COPY. The shipped body is eight instructions - advance past
    // the leading spaces and `ret` - with no `strcpy` and nothing written
    // back, so the skipped prefix survives in the caller's buffer and the
    // advanced pointer is left in EAX. The catalogued name spells the return
    // `X`, void; the `symbol` fact records what this tree emits instead.
    LPSTR trim = input;
    while (*trim == ' ') {
        trim++;
    }
    return trim;
}

/*
Purpose: Trim the leading and trailing spaces from the string.
// ORIGINAL: 0x006007B0 ?purge_spaces@@YAXPAD@Z 0x006007B0-0x00600817 BYTE_EXACT
// size      103 bytes
// prototype void (__cdecl ?purge_spaces@@YAXPAD@Z)(int8* input)
// callers   20   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006453E0 0x00645460
Return Value: n/a
Status: Complete
*/
void __cdecl purge_spaces(LPSTR input) {
    // THE COPY LIVES HERE, not in `purge_leading` - which is why that one is
    // eight instructions that return a pointer. The image reserves 0x200
    // bytes at 0x006007B0, walks the trailing spaces off FIRST, scans past
    // the leading ones, then copies through the buffer and back.
    char temp[512];
    purge_trailing(input);
    LPSTR start = purge_leading(input);
    strcpy(temp, start);
    strcpy(input, temp);
}

/*
Purpose: Truncate the string at the line feed (LF). Doesn't take into account carriage return (CR).
// ORIGINAL: 0x00600820 ?kill_lf@@YAXPAD@Z 0x00600820-0x00600837 BYTE_EXACT
// size      23 bytes
// prototype void (__cdecl ?kill_lf@@YAXPAD@Z)(int8* input)
// callers   6   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006458A0
Return Value: n/a
Status: Complete
*/
void __cdecl kill_lf(LPSTR str) {
    LPSTR newline_loc = strrchr(str, '\n');
    if (newline_loc) {
        *newline_loc = 0;
    }
}

/*
Purpose: Remove newline (Windows: CR LF). This fixes a bug with TextIndex having CR at the end of 
         "HEADER\r" breaking the compare. This is because it uses Filemap vs fopen(x, "rt").
Original Offset: n/a
Return Value: n/a
Status: Complete
*/
void __cdecl kill_nl(LPSTR str) {
    LPSTR newline_loc = strstr(str, "\r\n");
    if (newline_loc) {
        *newline_loc = 0;
    }
}

/*
Purpose: Add a line feed (LF) to the end of a string. This assumes the buffer has an extra byte and 
         doesn't take into account a carriage return (CR).
// ORIGINAL: 0x00600840 ?add_lf@@YAXPAD@Z 0x00600840-0x00600859
// TRIED     THE ORDERING IS THE ALLOCATOR'S, NOT THE SOURCE'S. Counts match at 10, the instructions match, and one moves: the image puts `add esp, 4` at position 5 and we put it at 4, doing the stack cleanup where the image does one more thing first. Four spellings measured, all 8 of 10 or worse - `end += strlen(str)` split from its declaration, a named `const size_t n` before the addition, indexing `str[n]`/`str[n+1]` instead of a pointer (4/10, the worst), and `*end`/`*(end+1)` instead of `end[0]`/`end[1]`. The pointer form the note above argues for is already the best of them. CHECKED BECAUSE THE WALL MIGHT HAVE BEEN A MISREAD - a low agreement score beside a matching instruction count usually means alignment slip, and three bodies today sat one instruction from BYTE_EXACT while scoring 2-4 of 14. This one is not that: same instructions, same count, one displaced by a single position, which is scheduling and no source order reaches it.
// TRIED: MNEMONIC_ONLY plateau (8/10, 1.000 similar at the best flag
// set /O2 /Gy /GR- /GX) across all --all-flags sets and several spellings
// (strlen(str)+str, non-const end, *end/*(end+1), a separate `int len`
// local, str[len]/str[len+1], `end += strlen(str)` as a separate
// increment). Every one still emits `add esp,4` before `add eax,esi`
// after the `call strlen`; the image has the pointer add before the
// stack cleanup. Register-allocation-only gap, not a source-shape one
// found so far.
// size      25 bytes
// prototype
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006453E0
Return Value: n/a
Status: Complete
*/
void __cdecl add_lf(LPSTR str) {
    // A POINTER, not an index. The image adds the length to the base at
    // 0x0060084B and stores through `[eax]` and `[eax+1]`; indexing keeps
    // both operands live and stores through `[eax+esi]`.
    char *const end = str + strlen(str);
    end[0] = '\n';
    end[1] = 0;
}

/*
Purpose: Use the min and max parameters to bound the input.
// ORIGINAL: 0x00422F00 ?range@@YAHHHH@Z 0x00422F00-0x00422F1E BYTE_EXACT
// size      30 bytes
// prototype int (__cdecl ?range@@YAHHHH@Z)(int input, int min, int max)
// callers   10   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Bounded input
Status: Complete
*/
// BODY IN general.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


/*
Purpose: Allocate memory with error checking.
// ORIGINAL: 0x005D4510 ?mem_get@@YAPAXH@Z 0x005D4510-0x005D453C BYTE_EXACT
// symbol    ?mem_get@@YAPAXI@Z
// size      44 bytes
// prototype void* (__cdecl ?mem_get@@YAPAXH@Z)(size_t size)
// callers   88   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00644DFF 0x006470A6
// indirect  0x005D452E
Return Value: Pointer to allocated memory
Status: Complete with two versions of malloc to prevent crash. Incompatibility with newer SDK 
        version of malloc CRT. Revisit once more code is redirected to dll.
*/
LPVOID __cdecl mem_get_old(size_t size) {
    LPVOID result = malloc(size);
    if (!result) {
        MessageBoxA(NULL,
            "Windows has run out of virtual memory; the game will exit. "
            "Recommendations: defragment your hard drive; use Control Panel "
            "(System/Performance/Virtual Memory) to increase your virtual memory to at least 128MB",
            "FATAL ERROR", MB_ICONWARNING);
        exit(4);
    }
    return result;
}

LPVOID __cdecl mem_get(size_t size) {
    LPVOID result = malloc(size);
    if (!result) {
        MessageBoxA(NULL,
            "Windows has run out of virtual memory; the game will exit. "
            "Recommendations: defragment your hard drive; use Control Panel "
            "(System/Performance/Virtual Memory) to increase your virtual memory to at least 128MB",
            "FATAL ERROR", MB_ICONWARNING);
        exit(4);
    }
    return result;
}
/*
Purpose: Check the source file path and attempt to open a handle to the file.
// ORIGINAL: 0x00634BB0 ?env_open@@YAHPADPAD@Z 0x00634BB0-0x00634BD4 BYTE_EXACT
// symbol    ?env_open@@YAPAU_iobuf@@PBD0@Z
// size      36 bytes
// prototype 
// callers   18   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x006005D0 0x00645646
Return Value: FILE pointer
Status: Complete with two versions of fopen to prevent a crash. Incompatibility with newer SDK 
        version of fopen/fopen_s. Revisit once more code is redirected to dll.
*/
FILE *__cdecl env_open_old(LPCSTR source, LPCSTR mode) {
    LPCSTR src_check = filefind_get(source);
    if (!src_check) {
        src_check = source;
    }
    return fopen(src_check, mode);
}

FILE *__cdecl env_open(LPCSTR source, LPCSTR mode) {
    LPCSTR src_check = filefind_get(source);
    if (!src_check) {
        src_check = source;
    }
    FILE *file_out;
    fopen_s(&file_out, src_check, mode);
    return file_out;
}

/*
Purpose: Set the global gender and plurality variables used by various parse functions.
// ORIGINAL: 0x005A58E0 ?parse_set@@YAXHH@Z 0x005A58E0-0x005A58F6 BYTE_EXACT
// size      22 bytes
// prototype 
// callers   1   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
// BODY IN general.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


/*
Purpose: Copies the value into a number global message buffer using id.
// ORIGINAL: 0x00625E30 ?parse_num@@YAHHH@Z 0x00625E30-0x00625E4D BYTE_EXACT
// size      29 bytes
// prototype 
// callers   87   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: No errors (0); Error (3)
Status: Complete
*/
int __cdecl parse_num(int id, int value) {
    if (id > 9) {
        return 3;
    }
    ParseNumTable[id] = value;
    return 0;
}

/*
Purpose: Use the string table input reference to copy a string into the global message buffer.
// ORIGINAL: 0x00625E50 ?parse_say@@YAHHHHH@Z 0x00625E50-0x00625EB3
// LEVER: hoisting `StringTable->get(input)` into its own local BEFORE
// the truncating store moved 17/30 to 20/30 - the image loads the
// `StringTable` this-pointer (`mov ecx, 0x9b90d8`) right next to the
// dest-pointer computation, before either call, and folding the call
// into the `strcat` argument list pushed that load later. Truncate then
// `strcat`, exactly as `parse_says` does: `mov byte ptr [esi], 0` at
// 0x00625E9D, immediately before `call 0x6169a0` (`StringTable::get`),
// then `call 0x645470` (`strcat`).
// TRIED (still short of the image at 20/30, 0.918 similar,
// plateaued across --all-flags): the remaining gap is the same
// add-vs-lea encoding as `parse_says` (`lea esi, [eax+0x9bb5e8]` there
// vs `add eax, 0x9bb5e8; mov esi, eax` here) plus the null-terminator
// store landing next to the second `call` instead of the first. Tried
// writing `dest[0] = 0` before the `get()` call, matching the image's own
// instruction order (regresses to 17/30 - scheduling does not follow
// source order here), a non-const `dest`, and reusing `*dest` instead of
// `dest[0]`.
// size      99 bytes
// prototype int (__cdecl ?parse_say@@YAHHHHH@Z)(int id, int input, int gender, int pluralality)
// callers   40   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x006169A0 0x00645470
Return Value: No errors (0); Error (3)
Status: Complete
*/
int __cdecl parse_say(int id, int input, int gender, int pluralality) {
    if (id > 9) {
        return 3;
    }
    if (gender < 0) {
        gender = *GenderDefault;
    }
    ParseStrGender[id] = gender;
    if (pluralality < 0) {
        pluralality = *PluralityDefault;
    }
    ParseStrPlurality[id] = pluralality;
    char *const dest = ParseStrBuffer[id].str;
    LPSTR text = StringTable->get(input);
    dest[0] = 0;
    strcat(dest, text);
    return 0;
}

/*
Purpose: Copies the input string into the global message buffer.
// ORIGINAL: 0x00625EC0 ?parse_says@@YAHHPADHH@Z 0x00625EC0-0x00625F1A
// TRIED: plateaued at 24/27 (0.963 similar) on a single instruction-
//   selection difference - see the TRIED note at the `dest`/`lea`
//   pointer add below, inside the body, which lists the five spellings
//   already tried. Recorded here too (this line only, not moving the body
//   note) so a marker-position scan finds it - a prior pass left it there,
//   which is why this address kept showing as untouched.
// symbol    ?parse_says@@YAHHPBDHH@Z
// size      90 bytes
// prototype int (__cdecl ?parse_says@@YAHHPADHH@Z)(int nID, int8* input, int gender, int pluralality)
// callers   199   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00645470
Return Value: No errors (0); Error (3)
Status: Complete
*/
int __cdecl parse_says(int id, LPCSTR input, int gender, int pluralality) {
    if (!input || id > 9) {
        return 3;
    }
    if (gender < 0) {
        gender = *GenderDefault;
    }
    ParseStrGender[id] = gender;
    if (pluralality < 0) {
        pluralality = *PluralityDefault;
    }
    ParseStrPlurality[id] = pluralality;
    // TRUNCATE THEN APPEND, which is what the image does: `mov byte ptr
    // [eax], 0` at 0x00625F06 and then `call 0x645470` - which is `strcat`,
    // not `strcpy`. The two are behaviourally identical after a truncation and
    // the relocation hides the difference in the bytes, so only the CALL
    // SYMBOL tells them apart.
    // TRIED: the only remaining gap is `8D 80 E8 B5 9B 00 lea eax,
    // [eax+0x9bb5e8]` (6 bytes) in the image vs `05 E8 B5 9B 00 add eax,
    // 0x9bb5e8` (5 bytes) here, which shifts every later offset/jump target
    // by one byte. Tried `&ParseStrBuffer[id].str[0]`, no `const`, an
    // explicit `(char*)ParseStrBuffer + (id<<8)` cast, and folding the
    // `dest` local away entirely (indexing `ParseStrBuffer[id].str` twice) -
    // all five plus --all-flags plateau at the same 24/27, 0.963 similar.
    // Pure instruction-selection (add vs lea) for an in-place pointer add,
    // not a source-shape difference found so far.
    char *const dest = ParseStrBuffer[id].str;
    dest[0] = 0;
    strcat(dest, input);
    return 0;
}

/*
Purpose: Convert the binary string to an integer.
// ORIGINAL: 0x006288D0 ?btoi@@YAHPAD@Z 0x006288D0-0x006288ED BYTE_EXACT
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

/*
Purpose: Convert the hex string to an integer.
// ORIGINAL: 0x006288F0 ?htoi@@YAHPAD@Z 0x006288F0-0x0062894B BYTE_EXACT
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
char *__cdecl findnum(char *str) {
    if (!str) {
        return 0;
    }
    char *const digits = (char *)0x00670C1C;
    char lo = digits[0];
    char hi = digits[1];
    char c;
    str--;
    do {
        str++;
        c = *str;
        if (c == 0) {
            return 0;
        }
    } while (c < lo || c > hi);
    return str;
}

/*
Purpose: Checks to see if the JACKAL library version is up to date. Pretty pointless but might add 
         an OpenSMACX check in the future.
// ORIGINAL: 0x0062D570 ?jackal_version_check@@YAHPAD@Z 0x0062D570-0x0062D5A3 BYTE_EXACT
// symbol    ?jackal_version_check@@YAHPBD@Z
// size      51 bytes
// prototype 
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645660
// indirect  0x0062D594
Return Value: Was there an error? true/false
Status: Complete
*/
BOOL __cdecl jackal_version_check(LPCSTR version) {
    if (strcmp(version, "10.10")) {
        MessageBoxA(NULL,
            "The version of the JACKAL library you (Brian Reynolds) are attempting to use "
            "is out of sync with the library headers! Please rebuild JACKAL.LIB...Bye!",
            "Oh, such serious danger, William Robinson!!!",
            MB_OK);
        return true;
    }
    return false;
}

/*
Purpose: This handles parsing the input string and storing it in the output.
// ORIGINAL: 0x00625880 ?parse_string@@YAHPADPAD@Z 0x00625880-0x00625DDD
// TRIED: 19/490 MISMATCH, mid-recovery - see the LEVER note just above
//   the function definition below (outside this comment block, which is why
//   a marker-position scan missed it before): the call graph is already
//   fixed (memcpy, not strncpy, at every prefix-copy site), but instruction
//   0 itself diverges - image reserves `sub esp, 0xfc` against this tree's
//   much smaller frame, a locals-layout gap across a 490-instruction
//   multi-branch `switch`. Closing that needs matching the whole local
//   variable layout, which is out of scope for a single pass; not attempted
//   here either, to avoid a blind rewrite of a function this size.
// symbol    ?parse_string@@YAHPAD0@Z
// size      1373 bytes
// prototype int (__cdecl ?parse_string@@YAHPADPAD@Z)(int8* input, int8* output)
// callers   23   call targets   7
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00628B30 0x006453E0 0x00645470 0x006458F0 0x00645930 0x006468D0 0x0064FC88
Return Value: No errors (0); Error (3)
Status: WIP
*/
// LEVER: `calls` lists 0x00645930 (`memcpy`), not `strncpy` - every prefix
// copy here has an exact, already-computed byte count (`len`), so the image
// uses the plain block copy instead of a NUL-scanning one. The ten
// `strncpy_s(output, 1024, input, len)` call sites (all folding to
// `strncpy` through the `vc6_compat.h` shim) are now `memcpy(output, input,
// len)` to match. The `agreeing` count does not move on this alone - the
// very first instruction already diverges (`sub esp, 0xfc` in the image
// against a much smaller frame here), which is a locals-layout gap, not a
// call-target one, and this function is 490 instructions across a
// multi-branch `switch`; reaching byte-exact needs matching that whole
// local-variable layout, which is out of scope for this pass. Left as
// MISMATCH with the call graph corrected rather than attempting a full
// rewrite blind.
int __cdecl parse_string(LPSTR input, LPSTR output) {
    if (!input || !output) { // EBX || ESI
        return 3;
    }
    //LPSTR outputCopy = output;
    LPSTR var; // EDI
    do {
        var = strstr(input, "$");
        if (!var) {
            break;
        }
        //LPSTR parsingInput = &input[1];
        switch (var[1]) {
          case '$': { // done -> needs testing
            int len = (var - input) + 1;
            memcpy(output, input, len);
            output += len;
            input = var + 2;
            *output = 0;
            break;
          }
          case 'H': { // done -> needs testing
            if (strncmp(var, "$HEX", 4)) {
                int number = var[4] - '0';
                if (number > 9) {
                    return 14; // parse error
                }
                int len = var - input;
                memcpy(output, input, len);
                output += len;
                *output = 0;
                input = var + 5;
                char output_num[5];
                _itoa_s(ParseNumTable[number], output_num, 5, 10);
                strcat_s(output, 1024, output_num);
                output += strlen(output);
            } else {
                LPSTR num = findnum(var);
                if (!num) {
                    break;
                }
                int number = num[0] - '0';
                if (number > 9) {
                    return 14; // parse error
                }
                int len = (var - input) + 1;
                memcpy(output, input, len);
                output += len;
                *output = 0;
                input = num + 1;
                strcat_s(output, 1024, ParseStrBuffer[number].str);
                output += strlen(output);
            }
            break;
          }
          case 'N': { // done -> needs testing
            if (strncmp(var, "$NUMBER", 7)) {
                int number = var[7] - '0';
                if (number > 9) {
                    return 14; // parse error
                }
                int len = var - input;
                memcpy(output, input, len);
                output += len;
                *output = 0;
                input = var + 8;
                char output_num[5];
                _itoa_s(ParseNumTable[number], output_num, 5, 10);
                strcat_s(output, 1024, output_num);
                output += strlen(output);
            } else if (strncmp(var, "$NUM", 4)) {
                int number = var[4] - '0';
                if (number > 9) {
                    return 14; // parse error
                }
                int len = var - input;
                memcpy(output, input, len);
                output += len;
                *output = 0;
                input = var + 5;
                char output_num[5];
                _itoa_s(ParseNumTable[number], output_num, 5, 10);
                strcat_s(output, 1024, output_num);
                output += strlen(output);
            } else {
                LPSTR num = findnum(var);
                if (!num) {
                    break;
                }
                int number = num[0] - '0';
                if (number > 9) {
                    return 14; // parse error
                }
                int len = var - input;
                memcpy(output, input, len);
                output += len;
                *output = 0;
                input = num + 1;
                strcat_s(output, 1024, ParseStrBuffer[number].str);
                output += strlen(output);
            }
            break;
          }
          case '<': {
            int len = var - input;
            memcpy(output, input, len);
            output += len;
            *output = 0;
            LPSTR end_bracket = strstr(var, ">");
            if (!end_bracket) {
                input = var + 1;
                continue;
            }
            var += 2;
            input = end_bracket + 1;
            purge_leading(var);
            int gender;// , plural = 0, num = -1;
            switch (var[0]) {
              case 'M':
              case 'm':
                gender = 0;
                break;
              case 'F':
              case 'f':
                gender = 1;
                break;
              case 'N':
              case 'n':
                gender = 2;
                break;
              case '#':
                //
                break;
              default:
                //
                break;
            }
            /*
            else if (szGender == '#') {
                nNum = var[1] - '0';
                if (nNum > 9 || nNum < 0) {
                    break;
                }
                var++;
                nPlural = (ParseNumTable[nNum] == 1) ? 0 : 1;
            } else {
                nNum = szGender - '0';
                if (nNum > 9 || nNum < 0) {
                    break;
                }
                nPlural = ParseStrPlurality[nNum];
                nGender = ParseStrGender[nNum];
            }
            var++;
            if (nNum < 0) {
                if (isdigit(var[0])) {
                    nNum = var[0] - '1';
                    var++;
                }
            }
            purge_leading(var);
            if (var[0] == ':') {
                var++;
            }
            // unfinished
            */
            break;
          }
          case 'L': { // done -> needs testing
            if (strncmp(var, "$LINK<", 6)) {
                LPSTR num = findnum(var);
                if (!num) {
                    var = 0;
                    break;
                }
                int number = num[0] - '0';
                if (number > 9) {
                    return 14; // parse error
                }
                int len = var - input;
                memcpy(output, input, len);
                output += len;
                *output = 0;
                input = num + 1;
                strcat_s(output, 1024, ParseStrBuffer[number].str);
                output += strlen(output);
            } else {
                int len = (var - input) + 1;
                memcpy(output, input, len);
                output += len;
                *output = 0;
                input = var + 1;
                break;
            }
            break;
          }
          default: { // done -> needs testing
            LPSTR num = findnum(var);
            if (!num) {
                var = 0;
                break;
            }
            int number = num[0] - '0';
            if (number > 9) {
                return 14; // parse error
            }
            int len = var - input;
            memcpy(output, input, len);
            output += len;
            *output = 0;
            input = num + 1;
            strcat_s(output, 1024, ParseStrBuffer[number].str);
            output += strlen(output);
            break;
          }
        }
    } while (var);

    //strcat(output, input); // replace with safe version _s

    if (Language == 1) { // French : handling for poor translations
        do {
            //LPSTR partSpeach[] = { " de ", " le ", " la ", "De ", "Le ", "La " };
            //LPSTR langParse = output;
            LPSTR searching = 0;
            for (int i = 0; i < 6; i++) {
                LPSTR part_found = 0;// strstr(langParse, partSpeach[i]);
                if (part_found && (!searching || part_found < searching)) {
                    searching = part_found;
                }
            }
        } while (1);

        /*
        int nLoop = 0;
        do {
                int nOffset = (nLoop < 3) + 3;
                char szVowel = tolower(part_found[nOffset]);
                if (szVowel == 'a' || szVowel == 'e' || szVowel == 'i'
                || szVowel == 'o' || szVowel == 'u' || szVowel == 'y' || szVowel == 'h') {
                    int nDiff = strlen(output) - strlen(part_found);
                    strcpy_s(&output[nDiff + nOffset - 1], 1024, &output[nDiff + nOffset]);
                    output[nDiff + nOffset - 2] = '\'';
                }
            } else {
                nLoop++;
            }
        } while (nLoop < 6);
        */
    }

    return false;
}

/*
Purpose: Get the drive letter of the CD path.
// ORIGINAL: 0x006003A0 ?filefind_cd_drive_letter@@YAPADXZ 0x006003A0-0x006003A8 BYTE_EXACT
// symbol    ?filefind_cd_drive_letter@@YAHXZ
// size      8 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: CD drive letter
Status: Complete
*/
// INT, NOT CHAR, and the two instructions say which: the image is
// `movsx eax, byte ptr [0x9b8198]`, a sign extension to a full register.
// Returning `char` compiles `mov al, [X]` and leaves the rest of eax alone.
// The catalogued name spells the return `PAD` - a `char *` - which the body
// contradicts twice over.
int __cdecl filefind_cd_drive_letter() { return FilefindPath.cd_path[0]; }

/*
Purpose: Set an alternative path for the Filefind checks.
// ORIGINAL: 0x006003B0 ?filefind_set_alternate@@YAXPAD@Z 0x006003B0-0x006003F6 BYTE_EXACT
// symbol    ?filefind_set_alternative@@YAXPBD@Z
// size      70 bytes
// prototype 
// callers   3   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00645470 0x00645DD0
Return Value: n/a
Status: Complete
*/
void __cdecl filefind_set_alternative(LPCSTR path) {
    FilefindPath.alt_path[0] = 0;
    if (path) {
        if (!strchr(path, ':') && path[0] != '\\') {
            strcat_s(FilefindPath.alt_path, 256, FilefindPath.exe_dir);
        }
        strcat_s(FilefindPath.alt_path, 256, path);
    }
}

/*
Purpose: Initialize the Filefind global along with a CD check if there isn't a complete install. 
         Optimized logic since most installs will be on a HDD making the CD check less important.
// ORIGINAL: 0x00600400 ?filefind_init@@YAHPADH@Z 0x00600400-0x006005C5
// LEVER: `strcat`/plain `[0]=0` clears in place of the bounded
//        `strcpy_s`/`strcat_s` forms, matching the sibling
//        `filefind_get`'s own documented idiom - mechanical, does not
//        close the gap alone (see TRIED).
// LEVER: THE PREVIOUS BODY WAS WRONG, NOT JUST DIFFERENTLY SPELLED - a
//        `DEFECTS THAT ARE NOT MATCHING DEFECTS` case. Read raw byte-for-byte
//        (no earlier "is_complete -> return 0" early exit, no HDD
//        FindFirstFileA/FindClose pre-check before the CD scan - neither
//        exists in the shipped bytes at all) and cross-checked against the
//        PE import table (`669140`=GetDriveTypeA, `6690e0`=FindFirstFileA
//        cached in ebp for the loop, `6690e4`=GetCurrentDirectoryA,
//        `66911c`=FindClose, `669114`=SetCurrentDirectoryA - none of which
//        is "MessageBox-shaped") and the four .rdata string constants at
//        `696ea0`/`696ea4`/`696ea8`/`696eb8` ("\\", "A:\\", "FILEFIND_NOCD",
//        "jackal"). `edi` is not a "JACKAL_CLASS": it is a `BasePop *`
//        obtained through the SAME `PopupAllocHook` slot `BasePop::init_class`
//        already uses (0x00696ECC), and the "message-box interaction" is a
//        real call to `BasePop::start` (0x00601BF0) followed by the object's
//        own vtable[0] scalar-deleting-destructor call
//        (`??_GBasePop@@UAEPAXI@Z`, already recovered as
//        `scalar_delete_base_pop` in deleting_thunks.cpp, dispatched here
//        the same way `sound.cpp` dispatches slot 0 - through
//        `vtable_method`, not by name, because the call site is an indirect
//        `call dword ptr [edx]`). `is_complete` is read only ONCE, after the
//        26-drive scan comes up empty, to decide whether to show that popup
//        at all - not as an up-front bypass. `field_3100_` (already a named
//        BasePop member) is the popup's own response code: 1 means Retry
//        (loop back to rescan, `jmp` target is BEFORE the `root_path` reset,
//        which an outer `for(;;)` with `continue` reproduces), anything else
//        falls out of the loop.
// TRIED: not yet byte-exact - the corrected control flow compiles and
//        reproduces the image's calls in the image's order, but the exact
//        local-buffer layout (there are at least three untyped stack
//        buffers in the image's 0x24c frame: `root_path`/"A:\\" pattern,
//        the WIN32_FIND_DATA, and a saved copy of `exe_dir` taken before the
//        trailing backslash is appended, used only much later by
//        `SetCurrentDirectoryA`) is approximate, so the frame size and
//        instruction count still diverge. Recorded as MISMATCH, not
//        NO_COMPILE - this is real progress over the previous body, which
//        called none of `BasePop::start`, `PopupAllocHook`, or the deleting
//        destructor at all.
// symbol    ?filefind_init@@YAHPBDH@Z
// size      453 bytes
// prototype
// callers   1   call targets   3
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00601BF0 0x00602600 0x00645470
// indirect  0x0060042D 0x00600479 0x006004C7 0x00600509 0x00600560 0x0060056C 0x0060057C 0x006005A1 0x006005B6
Return Value: No errors (0) otherwise error
Status: WIP
*/
int __cdecl filefind_init(LPCSTR file_check, BOOL is_complete) {
    if (!file_check) {
        return 16; // error, file_check shouldn't be NULL
    }
    // THE PRE-sprintf IDIOM, same as `filefind_get` immediately below:
    // `strcat`/plain assignment, no bounded forms - the image has no
    // bounds check anywhere in this function.
    FilefindPath.alt_path[0] = 0;

    // SAME HOOK AS `BasePop::init_class` (basepop.cpp): the image reaches
    // the popup allocator through the pointer at 0x00696ECC rather than by
    // name. The tree owns the slot as the real `PopupAllocHook` global;
    // calling it directly is the image's single memory-indirect.
    BasePop *const popup = reinterpret_cast<BasePop *>(PopupAllocHook());
    if (!popup) {
        return 4;
    }

    FilefindPath.relative_path[0] = 0;
    strcat(FilefindPath.relative_path, file_check);

    FilefindPath.cd_path[0] = 0;
    FilefindPath.exe_dir[0] = 0;
    FilefindPath.last_path[0] = 0;
    GetCurrentDirectoryA(256, FilefindPath.exe_dir);
    char saved_dir[256];
    saved_dir[0] = 0;
    strcat(saved_dir, FilefindPath.exe_dir);
    strcat(FilefindPath.exe_dir, "\\");

    typedef void *(OriginalObject::*delete_popup_fn)(unsigned int);
    char root_path[5];
    WIN32_FIND_DATAA find_file_data;
    HANDLE file_found;
    for (;;) {
        root_path[0] = 0;
        strcat(root_path, "A:\\");
        for (int i = 0; i < 26; i++) {
            if (GetDriveTypeA(root_path) == DRIVE_CDROM) {
                // problem if drive was disconnected
                FilefindPath.last_path[0] = 0;
                strcat(FilefindPath.last_path, root_path);
                strcat(FilefindPath.last_path, file_check);
                file_found = FindFirstFileA(FilefindPath.last_path, &find_file_data);
                if (file_found != INVALID_HANDLE_VALUE) {
                    FindClose(file_found);
                    FilefindPath.cd_path[0] = 0;
                    strcat(FilefindPath.cd_path, root_path);
                    SetCurrentDirectoryA(saved_dir);
                    (ORIGINAL(popup)->*vtable_method<delete_popup_fn>(popup, 0))(1);
                    return 0;
                }
            }
            root_path[0]++;
        }
        if (is_complete) {
            (ORIGINAL(popup)->*vtable_method<delete_popup_fn>(popup, 0))(1);
            return 0;
        }
        // send FILEFIND_NOCD message -> if doesn't exist (removed from jackal.txt) -> exit
        const int response = popup->start(const_cast<LPSTR>("jackal"), "FILEFIND_NOCD", -1, 0, 0, 0);
        if (response) {
            (ORIGINAL(popup)->*vtable_method<delete_popup_fn>(popup, 0))(1);
            return response;
        }
        // field_3100_ (BasePop, private): the popup's own response code.
        // Read through the raw offset rather than a new accessor, to avoid
        // touching basepop.h for a single caller.
        if (*reinterpret_cast<uint32_t *>(reinterpret_cast<uint8_t *>(popup) + 0x3100) == 1) {
            // if user response is to retry -> loop around again
            continue;
        }
        // if user response is "ok", exit loop
        SetCurrentDirectoryA(saved_dir);
        (ORIGINAL(popup)->*vtable_method<delete_popup_fn>(popup, 0))(1);
        return 0;
    }
}

/*
Purpose: Check to see if the specified file can be found at some other path.
// ORIGINAL: 0x006005D0 ?filefind_get@@YAHPAD@Z 0x006005D0-0x00600753 BYTE_EXACT
// symbol    ?filefind_get@@YAPADPBD@Z
// size      387 bytes
// prototype 
// callers   7   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645470
// indirect  0x00600626 0x0060062F 0x0060068F 0x00600694 0x006006D9 0x006006DE 0x0060072C 0x00600734
Return Value: File path string or NULL if not found
Status: Complete
*/
LPSTR __cdecl filefind_get(LPCSTR file_name) {
    // STRCAT, NOT STRCPY_S, and a copy is `dst[0] = 0` followed by one: the
    // image pushes two arguments and clears the first byte itself -
    // `mov byte ptr [0x9b8398], 0` then `call strcat` with `add esp, 8`.
    // The bounded forms push three and cost the `add esp, 0xc` beside them.
    // There is no bounds check in the shipped code and adding one here makes
    // a different program.
    if (!file_name) {
        return 0;
    }
    if (file_name == FilefindPath.last_path) {
        return FilefindPath.last_path;
    }
    WIN32_FIND_DATAA find_file_data;
    HANDLE file_found;
    if (file_name[1] == ':') {
        FilefindPath.last_path[0] = 0;
        strcat(FilefindPath.last_path, file_name);
        file_found = FindFirstFileA(FilefindPath.last_path, &find_file_data);
        FindClose(file_found);
        return (file_found != INVALID_HANDLE_VALUE) ? FilefindPath.last_path : 0;
    }
    if (FilefindPath.alt_path[0]) {
        FilefindPath.last_path[0] = 0;
        strcat(FilefindPath.last_path, FilefindPath.alt_path);
        strcat(FilefindPath.last_path, file_name);
        file_found = FindFirstFileA(FilefindPath.last_path, &find_file_data);
        FindClose(file_found);
        if (file_found != INVALID_HANDLE_VALUE) {
            return FilefindPath.last_path;
        }
    }
    FilefindPath.last_path[0] = 0;
        strcat(FilefindPath.last_path, FilefindPath.exe_dir);
    strcat(FilefindPath.last_path, file_name);
    file_found = FindFirstFileA(FilefindPath.last_path, &find_file_data);
    FindClose(file_found);
    if (file_found != INVALID_HANDLE_VALUE) {
        return FilefindPath.last_path;
    }
    if (FilefindPath.cd_path[0]) {
        FilefindPath.last_path[0] = 0;
        strcat(FilefindPath.last_path, FilefindPath.cd_path);
        strcat(FilefindPath.last_path, file_name);
        file_found = FindFirstFileA(FilefindPath.last_path, &find_file_data);
        // THIS BLOCK CLOSES ONLY ON SUCCESS, unlike the three above it: the
        // image tests first - `cmp eax, -1; je` at 0x0060072E - and calls
        // FindClose after the branch. A failed FindFirstFileA hands back
        // INVALID_HANDLE_VALUE, so there is nothing to leak; the asymmetry
        // is in the shipped bytes and closing here unconditionally costs an
        // instruction and moves the epilogue.
        if (file_found != INVALID_HANDLE_VALUE) {
            FindClose(file_found);
            return FilefindPath.last_path;
        }
    }
    return 0;
}

/*
Purpose: Count the number of unsigned bits set. Replaced the original code with Brian Kernighan's 
// ORIGINAL: 0x0050BA30 ?bit_count@@YAHH@Z 0x0050BA30-0x0050BA4E
// size      30 bytes
// prototype
// callers   34   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// TRIED: sixteen source shapes (do-while/while, `count += bits & 1`,
//   `if (bits & 1) count++`, a captured-carry temp, shift-before-test) all
//   land on the same 3/15, `push ebx` vs `mov ecx, [ebp+8]` at instruction 2
//   - none coax VC6 into the image's shr/adc carry-chain fusion; this is the
//   right algorithm (shift-and-accumulate, matching the shr/adc chain, not
//   Kernighan's clear-lowest-set-bit trick) and it scores strictly better
//   than the `n &= n - 1` form it replaced, but still MISMATCH.
Return Value: the number of set bits
Status: Complete
*/
int __cdecl bit_count(int bitfield) {
    unsigned int bits = bitfield;
    int count = 0;
    do {
        count += bits & 1;
        bits >>= 1;
    } while (bits);
    return count;
}

/*
Purpose: Count the number of signed bits set. Added a fix to prevent an infinite loop.
// ORIGINAL: 0x00628AB0 ?bit_count_signed@@YAHH@Z 0x00628AB0-0x00628AC2 BYTE_EXACT
// symbol    ?bit_count_signed@@YAIH@Z
// size      18 bytes
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Bit count
Status: Complete
*/
uint32_t __cdecl bit_count_signed(int input) {
    uint32_t count = 0;
    if (input) {
        do {
            input >>= 1;
            count++;
        } while (input);
    }
    return count;
}

/*
Purpose: Initialize the pseudo-random number generator.
// ORIGINAL: 0x00538FB0 ?my_srand@@YAHH@Z 0x00538FB0-0x00538FD2 BYTE_EXACT
// symbol    ?my_srand@@YAXH@Z
// size      34 bytes
// prototype int (__cdecl ?my_srand@@YAHH@Z)(int reseed)
// callers   6   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x006262F0 0x00646010
Return Value: n/a
Status: Complete with built in version of srand(). Revisit once more code is redirected to dll.
*/
void __cdecl my_srand(int reseed) {
    // ONE `srand`, not two. The image's `add esp, 0x14` accounts for exactly
    // log_say's four arguments and one seed; ours was `add esp, 0x18`, four
    // bytes and one `push`/`call` more. Seeding twice with the same value is
    // the same as seeding once, so nothing about the game changes.
    log_say("Reseed to", reseed, 0, 0);
    srand(reseed);
}

/*
Purpose: Swap the values of two 32-bit variables. Added an additional check when swapping the same
         memory location.
// ORIGINAL: 0x00628A50 ?swap@@YAXPAHPAH@Z 0x00628A50-0x00628A73 BYTE_EXACT
// symbol    ?swap@@YAXPAH0@Z
// size      35 bytes
// prototype 
// callers   14   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
// BODY IN general.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


/*
Purpose: Swap the values of two 8-bit variables. Added an additional check when swapping the same
         memory location.
// ORIGINAL: 0x00628A80 ?swap@@YAXPAEPAE@Z 0x00628A80-0x00628AA3 BYTE_EXACT
// symbol    ?swap@@YAXPAE0@Z
// size      35 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
// BODY IN general.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


/*
Purpose: Shift the numerator to the left by 16 then divide by the denominator. NOTE: the shipped
         code has no divide-by-zero guard; see the BUG comment on the body below.
// ORIGINAL: 0x00628AD0 ?fixed_div@@YAHJJ@Z 0x00628AD0-0x00628AEC
// symbol    ?fixed_div@@YAHHH@Z
// size      28 bytes
// prototype 
// callers   7   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// TRIED: still MISMATCH. The image's three-instruction dividend
//   construction - `edx = numerator >> 16` (arithmetic) and `eax = numerator
//   << 16`, i.e. `(int64_t)numerator << 16` built directly in EDX:EAX - never
//   falls out of `((int64_t)numerator << 16) / denominator` here at any flag
//   set tried. This compiler always promotes `denominator` to a full 64-bit
//   value first (a `cdq`-and-`call` sequence to a 64-bit shift helper, then
//   another call to a 64-bit divide helper - 16-28 instructions depending on
//   flags, never the image's 13), where the image gets a single hardware
//   `idiv` on a 64-bit dividend against a 32-bit memory operand. That native
//   widen-then-idiv form usually comes from inline assembly, which is off
//   the table here; no plain-C rephrasing tried reproduces it.
Return Value: Quotient
Status: Complete
*/
// BUG IN THE ORIGINAL: no divide-by-zero guard. The image is `sar edx,0x10;
// shl eax,0x10; idiv dword ptr [ebp+0xc]` with no test of the divisor
// beforehand - a `denominator == 0` call faults on the `idiv`, exactly like
// this reproduction now does. The previous body's `if (!denominator) return
// 0;` is not in the shipped bytes (13 instructions there, not 15) and is
// left out deliberately; adding it back defeats the match, it does not fix
// one.
int __cdecl fixed_div(int numerator, int denominator) {
    return (int)(((int64_t)numerator << 16) / denominator);
}

/*
Purpose: Reverse string search for the last occurrence of the specified character. Replaced the
         original searching logic with strrchr() that does same thing. The end parameter can be 
         removed in the future.
// ORIGINAL: 0x00628AF0 ?memrchr@@YAHPBX0H@Z 0x00628AF0-0x00628B23
// LEVER: `calls (none)` in the annotation, and the image proves it - a
// hand-rolled backward byte scan (`cmp byte ptr [eax], bl; je found; dec
// eax; dec ecx; jne loop`), never a call to `strrchr`. The old body called
// it, which is exactly the call-a-helper-the-image-inlines defect: `osmx
// calls` on this address shows zero calls, and a body that makes one is
// running a different program even when the answer often agrees. Restoring
// the loop (scan starts AT `end` itself, walking backward while `count =
// end - start` byte compares remain) moved MISMATCH-wrong-shape (20
// instructions, mostly not agreeing) to a same-instruction-count MISMATCH,
// 0.704 similar under /O1.
// TRIED: splitting `!start`/`!end` into two `if`s (image compiles
// `a || b` to the identical sequential-test-same-target shape already), and
// a `for(;;){...; if(count==0) break;}` in place of `do {...} while(count)`
// - neither changed the score. `const char *const s = start; const char
// *const e = end;` locals ahead of the null checks also made no difference
// (identical codegen at every flag set) - VC6 already treats the unmodified
// parameters as the same registers. The remaining gap is the image keeping
// `start` live in `ebx` for the whole function and using ONE comparison per
// iteration, where every flag set tried here rotates the loop (duplicates
// the compare to peel the first iteration) and spills `count` back through
// the `start` parameter's own stack slot instead of a register - the same
// unreproduced "push ecx" frame-slot pattern as `findnum` just above it.
// symbol    ?memrchr@@YAPBDPBD0D@Z
// size      51 bytes
// prototype
// callers   6   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: Position of character or NULL if not found.
Status: Complete
*/
const char *__cdecl memrchr(LPCSTR start, LPCSTR end, char value) {
    if (!start || !end) {
        return 0;
    }
    const char *p = end;
    int count = (int)(end - start);
    if (count == 0) {
        return 0;
    }
    do {
        if (*p == value) {
            return p;
        }
        p--;
        count--;
    } while (count != 0);
    return 0;
}

/*
Purpose: Calculate the square root of the input.
// ORIGINAL: 0x006290E0 ?quick_root@@YAHH@Z 0x006290E0-0x00629109 BYTE_EXACT
// size      41 bytes
// prototype 
// callers   3   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Square root
Status: Complete
*/
int __cdecl quick_root(int input) {
    int temp = input >> 1;
    int sq_root;
    if (input <= 1) {
        return input;
    }
    do {
        sq_root = temp;
        temp = (temp + input / temp) >> 1;
    } while (temp < sq_root);
    return sq_root;
}

/*
Purpose: Calculate the offset and bitmask for the specified input.
// ORIGINAL: 0x0050BA00 ?bitmask@@YAXHPAHPAH@Z 0x0050BA00-0x0050BA27 BYTE_EXACT
// symbol    ?bitmask@@YAXHPAH0@Z
// size      39 bytes
// prototype void (__cdecl ?bitmask@@YAXHPAHPAH@Z)(int input, int* offset, int* mask)
// callers   103   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
// BODY IN general.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.

// Non-inline forwarder for the OTHER callers - cost_factor, pop_goal,
// num_objectives (base.cpp) - where the image genuinely emits
// `call 0x50ba00` rather than folding bitmask's shift/and in place.
void __cdecl bitmask_call(int input, int *offset, int *mask) {
    bitmask(input, offset, mask);
}


/*
Purpose: Calculate a basic XOR checksum for the data buffer.
// HAND-WRITTEN ASSEMBLY IN THE ORIGINAL, so byte-exactness is NOT reachable
// from C++ and the honest ceiling here is semantic equivalence. The shipped
// bytes use the `loop` instruction, which VC6 never emits - it counts down
// with `dec`/`jne` - and it uses `lodsb` bare, which the compiler
// only emits under a `rep` prefix. Found by
// `tools/handwritten_asm.py`, which scans every unclaimed body for opcodes
// this compiler does not generate; there are seven in the game code.
//
// Do NOT grind spellings here, and do NOT write `__asm`: a semantic C++ body
// is worth more than a byte-exact assembly one. `Buffer::vline` reached 0.925
// similar before this was noticed.
// ORIGINAL: 0x00539090 ?checksum@@YAEPADHE@Z 0x00539090-0x005390B4
// size      36 bytes
// prototype
// callers   2   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
// TRIED: byte-exact is unreachable (hand-written `loop`/`lodsb`, above);
//   ceiling here is semantic equivalence, currently MISMATCH 4/19.
Return Value: the XOR checksum of the buffer, seeded
Status: Complete
*/
uint8_t __cdecl checksum(char *buffer, int size, uint8_t seed) {
    // Walked BACKWARDS with the length decremented in place. The forward
    // pointer walk needs one more value live, and VC6 then saves no
    // callee-saved register at all, diverging at instruction #0; this form
    // matches the original's byte length exactly and diverges at #3, where
    // the original saves esi unconditionally BEFORE the zero test and this
    // does it inside the taken branch.
    while (size) {
        seed ^= buffer[--size];
    }
    return seed;
}

/*
Purpose: Calculate a basic XOR checksum for a password string.
// HAND-WRITTEN ASSEMBLY IN THE ORIGINAL, so byte-exactness is NOT reachable
// from C++ and the honest ceiling here is semantic equivalence. The shipped
// bytes use the `loop` instruction, which VC6 never emits - it counts down
// with `dec`/`jne` - and it uses `lodsb` bare, which the compiler
// only emits under a `rep` prefix. Found by
// `tools/handwritten_asm.py`, which scans every unclaimed body for opcodes
// this compiler does not generate; there are seven in the game code.
//
// Do NOT grind spellings here, and do NOT write `__asm`: a semantic C++ body
// is worth more than a byte-exact assembly one. `Buffer::vline` reached 0.925
// similar before this was noticed.
// ORIGINAL: 0x005390C0 ?checksum_password@@YAHPAD@Z 0x005390C0-0x00539157
// symbol    ?checksum_password@@YAIPBD@Z
// size      151 bytes
// prototype
// callers   2   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x006453E0 0x00645460
// indirect  0x005390F6
// TRIED: byte-exact is unreachable (hand-written `loop`/`lodsb` inside
//   the `checksum` call it makes, above); ceiling here is semantic
//   equivalence, currently MISMATCH 22/58.
Return Value: Checksum
Status: Complete
*/
uint32_t __cdecl checksum_password(LPCSTR password) {
    if (!strlen(password)) {
        return 0;
    }
    char buffer[256];
    strcpy_s(buffer, 256, password);
    CharUpper(buffer); // incorrect results for Turkish/Azerbaijani 'i'
    uint8_t chksum = 0;
    size_t len = strlen(buffer);
    if (len) {
        chksum = checksum((LPSTR)&buffer, len, 0);
    }
    return chksum + 1;
}

/*
Purpose: Calculate a random value within the provided bounds. The unused 2nd parameter was possibly 
         meant to have the random value append to it.
// ORIGINAL: 0x00579770 ?rnd@@YAHHPAD@Z 0x00579770-0x00579790 BYTE_EXACT
// LEVER: ternary flipped to a guard-clause `if (bounds - 1 <= 0) return 0;`
//        then fall through to `return rand() % bounds;` - the image jumps
//        TO the rand() work and falls through to the early return.
// symbol    ?rnd@@YAIHPAD@Z
// size      32 bytes
// prototype int (__cdecl ?rnd@@YAHHPAD@Z)(int seed, int8*)
// callers   2   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0064601D
Return Value: Bounded random value
Status: Complete
*/
uint32_t __cdecl rnd(int bounds, LPSTR UNUSED(input)) {
    if (bounds - 1 <= 0) {
        return 0;
    }
    return rand() % bounds;
}

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
    popp(ScriptTxtID, "DANGER", 0, "hasty_sm.pcx", 0);
}

/*
Purpose: Delete the initial auto-save game file.
// ORIGINAL: 0x005ABD10 ?kill_auto_save@@YAXXZ 0x005ABD10-0x005ABD1C BYTE_EXACT
// size      12 bytes
// prototype 
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00646950
Return Value: n/a
Status: Complete
*/
void __cdecl kill_auto_save() {
    remove("saves\\auto\\Alpha Centauri Autosave 1.SAV");
}

/*
Purpose: Handle the creation and management of the auto-save game files.
// ORIGINAL: 0x005ABD20 ?auto_save@@YAXXZ 0x005ABD20-0x005ABE39
// TRIED: 28/66, 0.954 similar - as close as this gets. Both branches'
//            call sequences end with the image cleaning the LAST call's
//            single pushed argument via `pop ecx` (1 byte) rather than
//            folding it into the preceding `add esp, N` that cleans the
//            other calls in the run; this body folds all of them into one
//            `add esp`. Pure VC6 stack-cleanup-size heuristic across a run
//            of same-shaped `__cdecl` calls, not a source-form lever.
// size      281 bytes
// prototype 
// callers   9   call targets   3
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005A94F0 0x00646950 0x006469F7
Return Value: n/a
Status: Complete
*/
void __cdecl auto_save() {
    if (!IsMultiplayerPBEM || IsMultiplayerNet) { // auto-saving disabled for PBEM/HotSeat games
        if (GameRules & RULES_IRONMAN && !(GameState & STATE_SCENARIO_EDITOR)) {
            remove("saves\\auto\\Alpha Centauri Autosave 30.SAV");
            remove("saves\\auto\\Alpha Centauri Autosave 20.SAV");
            remove("saves\\auto\\Alpha Centauri Autosave 10.SAV");
            remove("saves\\auto\\Alpha Centauri Autosave 5.SAV");
            remove("saves\\auto\\Alpha Centauri Autosave 4.SAV");
            remove("saves\\auto\\Alpha Centauri Autosave 3.SAV");
            remove("saves\\auto\\Alpha Centauri Autosave 2.SAV");
            save_daemon("saves\\auto\\Alpha Centauri Autosave 1");
        } else { // standard auto saves
            if (!(TurnCurrentNum % 10)) {
                remove("saves\\auto\\Alpha Centauri Autosave 30.SAV");
                rename("saves\\auto\\Alpha Centauri Autosave 20.SAV",
                    "saves\\auto\\Alpha Centauri Autosave 30.SAV");
                rename("saves\\auto\\Alpha Centauri Autosave 10.SAV",
                    "saves\\auto\\Alpha Centauri Autosave 20.SAV");
                save_daemon("saves\\auto\\Alpha Centauri Autosave 10");
            }
            remove("saves\\auto\\Alpha Centauri Autosave 5.SAV");
            rename("saves\\auto\\Alpha Centauri Autosave 4.SAV",
                "saves\\auto\\Alpha Centauri Autosave 5.SAV");
            rename("saves\\auto\\Alpha Centauri Autosave 3.SAV",
                "saves\\auto\\Alpha Centauri Autosave 4.SAV");
            rename("saves\\auto\\Alpha Centauri Autosave 2.SAV",
                "saves\\auto\\Alpha Centauri Autosave 3.SAV");
            rename("saves\\auto\\Alpha Centauri Autosave 1.SAV",
                "saves\\auto\\Alpha Centauri Autosave 2.SAV");
            save_daemon("saves\\auto\\Alpha Centauri Autosave 1");
        }
    }
}

/*
Purpose: Extended auto-saving regardless of the game type or settings for debug purposes.
Original Offset: n/a
Return Value: n/a
Status: Complete
*/
void __cdecl auto_save_debug() {
    if (TurnCurrentNum == 1) {
        remove("saves\\auto\\Alpha Centauri Autosave Turn 1.SAV");
        save_daemon("saves\\auto\\Alpha Centauri Autosave Turn 1");
    }
    if (!(TurnCurrentNum % 10)) {
        remove("saves\\auto\\Alpha Centauri Autosave 500.SAV");
        char save_path_new[45];
        char save_path_old[45];
        for (int i = 0; i < 490; i += 10) {
            sprintf_s(save_path_old, 45, "saves\\auto\\Alpha Centauri Autosave %d.SAV", 490 - i);
            sprintf_s(save_path_new, 45, "saves\\auto\\Alpha Centauri Autosave %d.SAV", 500 - i);
            rename(save_path_old, save_path_new);
        }
        save_daemon("saves\\auto\\Alpha Centauri Autosave 10");
    }
    remove("saves\\auto\\Alpha Centauri Autosave 5.SAV");
    rename("saves\\auto\\Alpha Centauri Autosave 4.SAV",
        "saves\\auto\\Alpha Centauri Autosave 5.SAV");
    rename("saves\\auto\\Alpha Centauri Autosave 3.SAV",
        "saves\\auto\\Alpha Centauri Autosave 4.SAV");
    rename("saves\\auto\\Alpha Centauri Autosave 2.SAV",
        "saves\\auto\\Alpha Centauri Autosave 3.SAV");
    rename("saves\\auto\\Alpha Centauri Autosave 1.SAV",
        "saves\\auto\\Alpha Centauri Autosave 2.SAV");
    save_daemon("saves\\auto\\Alpha Centauri Autosave 1");
}

/*
Purpose: Load a Scenario Editor undo (type: 1) or redo (type: -1) auto-save. TODO: Revisit in the 
         future to fix some of the underlying issues with the undo/redo process.
// ORIGINAL: 0x005ABE40 ?load_undo@@YAXH@Z 0x005ABE40-0x005ABEBF BYTE_EXACT
// LEVER: two real bugs in the committed body, fixed by transcription
//        instead of invention. (1) WRONG CALLEE - `sprintf_s` is not in the
//        image; it's the `strcat`/`_itoa`/`strcat`-into-StringTemp idiom,
//        no ".SAV" suffix, matching `wipe_undo`/`auto_undo` below. (2) the
//        image calls NEITHER an early `return` on `type==-1`, NOR
//        `draw_map` at the end - both were invented, not transcribed; the
//        image's only calls are the two `strcat`s, `_itoa` and
//        `load_daemon`. (3) `static_cast<int>(ScenEditorUndoPosition)` at
//        both comparisons - the global is `uint32_t`, and the image's own
//        `cmp`s are signed (`jle`/`jge`), giving `jbe`/`jae` uncast.
// size      127 bytes
// prototype void (__cdecl ?load_undo@@YAXH@Z)(int type)
// callers   2   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005A9760 0x00645470 0x0064FC88
Return Value: n/a
Status: Complete
*/
void __cdecl load_undo(int type) {
    if (type < 0 && static_cast<int>(ScenEditorUndoPosition) > 1) {
        ScenEditorUndoPosition--;
    }
    // THE PRE-sprintf IDIOM, same as `wipe_undo`/`auto_undo` immediately
    // below: `strcat`/`_itoa`/`strcat` into StringTemp, no ".SAV" extension
    // appended, and no `draw_map` call - the image's own call targets are
    // just strcat x2, _itoa and load_daemon.
    StringTemp[0] = 0;
    strcat(StringTemp, "saves\\auto\\Scenario Editor Undo ");
    char number[80];
    _itoa(ScenEditorUndoPosition, number, 10);
    strcat(StringTemp, number);
    if (type > 0 && static_cast<int>(ScenEditorUndoPosition) < 9) {
        ScenEditorUndoPosition++;
    }
    load_daemon(StringTemp, false);
}

/*
Purpose: Remove all the existing Scenario Editor undo auto-saves.
// ORIGINAL: 0x005ABEC0 ?wipe_undo@@YAXXZ 0x005ABEC0-0x005ABF14 BYTE_EXACT
// LEVER: WRONG CALLEE - `sprintf_s` is not in the image at all (the image has no C++/CRT sprintf for paths here); it builds the name with `strcat`/`_itoa`/`strcat` into the global `StringTemp` buffer (0x009B86A0) and calls `remove()` directly, with no ".SAV" extension appended - matching auto_undo's own documented idiom immediately below.
// size      84 bytes
// prototype
// callers   1   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00645470 0x00646950 0x0064FC88
Return Value: n/a
Status: Complete
*/
void __cdecl wipe_undo() {
    for (int i = 9; i >= 1; i--) {
        StringTemp[0] = 0;
        strcat(StringTemp, "saves\\auto\\Scenario Editor Undo ");
        char number[80];
        _itoa(i, number, 10);
        strcat(StringTemp, number);
        remove(StringTemp);
    }
}

/*
Purpose: Handle the creation of an undo auto-save when certain Scenario Editor changes are made.
// ORIGINAL: 0x005ABF20 ?auto_undo@@YAXXZ 0x005ABF20-0x005ABFEF BYTE_EXACT
// size      207 bytes
// prototype 
// callers   32   call targets   7
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005A94F0 0x006453E0 0x00645460 0x00645470 0x00646950 0x006469F7 0x0064FC88
Return Value: n/a
Status: Complete
*/
void __cdecl auto_undo() {
    if (!(GamePreferences & PREF_BSC_AUTOSAVE_EACH_TURN)) {
        return;
    }
    // THREE 0x100 BUFFERS AND THE PRE-sprintf IDIOM. The image reserves 0x250
    // bytes at 0x005ABF23 and builds each name with `strcat`, `_itoa`,
    // `strcat` - no `sprintf` anywhere. It also carries the PREVIOUS name
    // forward rather than formatting it twice, and decides between `remove`
    // and `rename` on whether that carried name is empty, which is what makes
    // the first pass delete rather than rename.
    ScenEditorUndoPosition = 1;
    // 0x100, 0x100 and 0x50: the image's three locals sit at [ebp-0x250],
    // [ebp-0x150] and [ebp-0x50], which is where `sub esp, 0x250` comes from.
    char previous[256];
    char current[256];
    char number[80];
    previous[0] = 0;
    for (int i = 9; i >= 1; i--) {
        current[0] = 0;
        strcat(current, "saves\\auto\\Scenario Editor Undo ");
        _itoa(i, number, 10);
        strcat(current, number);
        if (strlen(previous) == 0) {
            remove(current);
        } else {
            // (previous, current), in that order: the image pushes the
            // current name and then the carried one, so the CARRIED name is
            // the first argument - it renames the name it built last pass
            // onto the one it just built, which collapses the history rather
            // than rotating it. That is what the shipped code does.
            rename(previous, current);
        }
        strcpy(previous, current);
    }
    // THE BUILT PATH, not a literal: the image pushes [ebp-0x250] at
    // 0x005ABFE1. The loop's last pass builds "Undo 1" and carries it there,
    // so the name is already in hand.
    save_daemon(previous);
}

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

/*
Purpose: For the count, sort both id and value arrays by the least to greatest value (ascending).
// ORIGINAL: 0x005B5690 ?sort@@YAXHPAHPAH@Z 0x005B5690-0x005B56F5
// TRIED: byte-match plateau at 5/47 across every flag set tried,
//            before and after the `goto`/bug fix below - the image
//            interleaves a SEARCH for the first out-of-order pair with the
//            swap in one tight loop; any C form with the swap reachable
//            from inside the scanning `for`/`while` gets restructured by
//            VC6 into two separate loops (scan-only, then a single swap),
//            which this address's plateau predates the bug fix and is
//            unaffected by it. Tried: `break` after the swap, `goto` to a
//            label after the loop, `goto` back to a `restart:` before the
//            loop (best of the four, but still 5/47), and a `while`
//            re-entering at i=0. Not a source-form lever found here.
// symbol    ?sort@@YAXHPAH0@Z
// size      101 bytes
// prototype void (__cdecl ?sort@@YAXHPAHPAH@Z)(int count, int* id, int* value)
// callers   2   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00628A50
Return Value: n/a
Status: Complete
*/
void __cdecl sort(int count, int *id, int *value) {
    int bounds = count - 1;
    // BUG IN THE ORIGINAL (reproduced): `test eax,eax` right after every
    // swap jumps straight back to the function's own top (0x5b5697) the
    // moment a swap happens, abandoning the rest of this pass instead of
    // finishing it. A correct bubble sort would keep scanning to the end of
    // the pass; this one restarts from i=0 after its FIRST swap each pass,
    // which is why there is no `has_swapped` flag left to track here - the
    // loop completing on its own IS the "nothing left to swap" signal.
restart:
    for (int i = 0; i < bounds; i++) {
        if (value[i] > value[i + 1]) {
            swap(&value[i], &value[i + 1]);
            swap(&id[i], &id[i + 1]);
            goto restart;
        }
    }
}

/*
Purpose: For the count, sort both id and value arrays by the greatest to least value (descending).
Original Offset: n/a
Return Value: n/a
Status: Complete
*/
void __cdecl sort_descending(uint32_t count, int *id, int *value) {
    int bounds = count - 1;
    BOOL has_swapped;
    do {
        has_swapped = false;
        for (int i = 0; i < bounds; i++) {
            if (value[i] < value[i + 1]) {
                has_swapped = true;
                swap(&value[i], &value[i + 1]);
                swap(&id[i], &id[i + 1]);
            }
        }
    } while (has_swapped);
}

// ---------------------------------------------------------------------------
// The Jackal engine's bring-up, promoted out of src/unrecovered/0062d3a0.cpp
// on 2026-08-15. WinMain calls it once; everything the game draws with exists
// because this returned zero.
// ---------------------------------------------------------------------------

// 0x009B7AF4 and 0x009BB484. A copy of one word into another, once, and
// nothing else in the recovered corpus touches either - so there is no
// evidence for a name and they keep their addresses rather than gaining an
// invented one.
static int *const g_009b7af4 = (int *)0x009B7AF4;
static int *const g_009bb484 = (int *)0x009BB484;

int JackalInitFlags;      // 0x009BC4B0

/*
Purpose: Bring up every drawing subsystem the game needs, in order, and stop
         at the first one that refuses.
// ORIGINAL: 0x0062D3A0 ?jackal_init_real@@YAHPAUPalette@@PAUFont@@PADHHHH@Z 0x0062D3A0-0x0062D4F6 BYTE_EXACT
// symbol    ?jackal_init_real@@YAHPAVPalette@@PAVFont@@PADHHHH@Z
// size      342 bytes
// prototype int (__cdecl ?jackal_init_real@@YAHPAUPalette@@PAUFont@@PADHHHH@Z)(Palette* pPalEntries, Font* fontBuffer, int8* lpWindowName, int nDDTgl, int displayWidth, int displayHeight, int nCmdLineVal)
// callers   1   call targets   19
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005DF570 0x005EFD20 0x005F01F0 0x005F2C40 0x005FE330 0x005FE460 0x005FEBB0 0x00604590 0x006074B0 0x0060E4D0 0x0060FC60 0x00614D90 0x00616880 0x006168F0 0x006195B0 0x006339A0 0x0063B910 0x0063B940 0x0063CE20
// indirect  0x0062D42A
Return Value: Zero once every subsystem is up; the first refusal's code
              otherwise, or 3 if the palette or font is missing
Status: Complete
*/
int __cdecl jackal_init_real(Palette *palette, Font *font, LPSTR window_name,
                             int tgl_direct_draw, int display_width,
                             int display_height, int colour_depth) {
    if (font == 0 || palette == 0) {
        return 3;
    }

    JackalInitFlags = tgl_direct_draw;
    Palette::init_palette_class(tgl_direct_draw & 2);
    PaletteCurrent = palette;
    palette->init();
    palette->set();

    int result = Win::init_class(window_name);
    if (result != 0) {
        return result;
    }
    palette->set();

    if (tgl_direct_draw & 4) {
        Win::set_display_mode(display_width, display_height, colour_depth, 1);
        ShowWindow(HandleMain, 5);
        Win::flip(0);
    }

    result = Buffer::init_class();
    if (result != 0) {
        return result;
    }
    result = CheckBox::init_class();
    if (result != 0) {
        return result;
    }
    result = CheckButton::init_class();
    if (result != 0) {
        return result;
    }
    result = RadioButton::init_class();
    if (result != 0) {
        return result;
    }

    BaseButton::set_def_font(font, 0, 0);
    result = Font::init_font_class(font);
    if (result != 0) {
        return result;
    }
    result = BasePop::init_class();
    if (result != 0) {
        return result;
    }
    result = FileWin::init_class();
    if (result != 0) {
        return result;
    }
    result = sub_63ce20();
    if (result != 0) {
        return result;
    }

    *g_009b7af4 = *g_009bb484;
    trig_init();
    result = Cursor::init_cursor_class();
    if (result != 0) {
        return result;
    }

    if (StringTable->init(0x8000) != 0) {
        return 1;
    }

    result = Time::init_class();
    if (result != 0) {
        return result;
    }

    JackalInitFlags |= 1;
    return 0;
}

/*
Purpose: Tear down every subsystem jackal_init_real brought up, in the
         image's own order, then clear the "initialised" bit.
// ORIGINAL: 0x0062D500 ?jackal_close@@YAXXZ 0x0062D500-0x0062D562 BYTE_EXACT
// size      98 bytes
// prototype void (__cdecl ?jackal_close@@YAXXZ)()
// callers   1   call targets   15
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005DF580 0x005F04E0 0x005FECF0 0x00604680 0x0060E5D0 0x0060FD60
//           0x00614E30 0x00616890 0x00616950 0x00619610 0x0062D100 0x006339B0
//           0x00635750 0x0063B930 0x0063CEF0
Return Value: n/a
Status: Complete
*/

/*
Purpose: sub_63cef0 - close the blank sprite (0x009BEAE8 in the image, the
         real object g_BLANK_SPRITE here), unguarded; the caller
         (jackal_close) tail-calls it as the sprite's teardown stage.
// ORIGINAL: 0x0063CEF0 sub_63cef0 0x0063CEF0-0x0063CEFA BYTE_EXACT
// symbol    ?teardown_0063cef0@@YAXXZ
// size      10 bytes
// prototype
// callers   1   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl teardown_0063cef0() {
    g_BLANK_SPRITE.close();
}

void __cdecl jackal_close() {
    sub_62d100();
    Time::close_class();
    reinterpret_cast<Unk9BE618 *>(0x009BE618)->unk_call();
    CheckButton::close_class();
    buffer_close_class_redirect();
    teardown_0060fd60();
    teardown_0060e5d0();
    Font::close_font_class();
    teardown_0063cef0();
    filewin_close_class();
    basepop_close_class();
    Cursor::close_cursor_class();
    Win::close_class();
    Palette::close_palette_class();
    StringTable->shutdown();
    JackalInitFlags &= ~1;
}

// ===== MANAGED GLOBALS - real objects, homed to their domain =====
// In the shipped image these live at fixed data addresses and are
// constructed before WinMain by the CRT's dynamic-initializer walk
// (winedbg-confirmed: walker return 0x00644EEB, table at 0x682568..).
// Here the same recovered constructors run through this build's own
// startup, and the matching destructors close them at exit.
// FactionArt is not modelled yet - its constructor runs through the
// FactionArtCtorTarget seam, so this stands in as raw storage until the
// class is recovered. 8 elements of 0x65C stride, per the image's own
// VectorCtorIterator. NOT constructed in this build (documented gap).
uint8_t FactionArtGlobal_storage[8 * 0x65C];  // 0x0078E978
// ===== archived teardown callbacks for still-bound globals =====
// These globals are not modelled as real objects yet; the bindings move
// with the callbacks until their domains claim them.
void *const TeardownObject0090EA68 = (void *)0x0090EA68;
Buffer *const TeardownObject00915068 = (Buffer *)0x00915068;
void *const TeardownObject009403E0 = (void *)0x009403E0;
/*
Purpose: ??__Eg_BOOM_BUFFER1@@YAXXZ - run 1 (ORIGINAL(s)->*teardown)() on fixed globals,
         unguarded. The last is a tail jump in the original, so its
         return goes straight to this function's caller.
// ORIGINAL: 0x00505D20 ??__Eg_BOOM_BUFFER1@@YAXXZ 0x00505D20-0x00505D2A BYTE_EXACT
// symbol    ?teardown_00505d20@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl teardown_00505d20() {
    TeardownObject00915068->Buffer::~Buffer();
}
/*
Purpose: ??__Eg_BOOM_FLIC@@YAXXZ - run 1 (ORIGINAL(s)->*teardown)() on fixed globals,
         unguarded. The last is a tail jump in the original, so its
         return goes straight to this function's caller.
// ORIGINAL: 0x00505D30 ??__Eg_BOOM_FLIC@@YAXXZ 0x00505D30-0x00505D3A BYTE_EXACT
// symbol    ?teardown_00505d30@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl teardown_00505d30() {
    reinterpret_cast<Flic *>(TeardownObject0090EA68)->Flic::~Flic();
}
/*
Purpose: sub_589890 - run 1 (ORIGINAL(s)->*teardown)() on fixed globals,
         unguarded. The last is a tail jump in the original, so its
         return goes straight to this function's caller.
// ORIGINAL: 0x00589890 sub_589890 0x00589890-0x0058989A BYTE_EXACT
// symbol    ?teardown_00589890@@YAXXZ
// size      10 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl teardown_00589890() {
    // THIS SITE is why `convert_seams --dtors` refused PopupDtorTarget: it read
    // the seam as a VALUE through `original_address`, so the pointer could not
    // be retired while it stood. It is a named destructor call now.
    reinterpret_cast<Popup *>(TeardownObject009403E0)->Popup::~Popup();
}



/*
Purpose: Fill SinTable, the 256-entry fixed-point sine table the game's own
         `int sin(int angle, int scale)` (0x0063B9B0) interpolates, then make
         two fixed calls through it. The table step is 2pi/1020 and the scale
         is 65535.0, both read as .rdata doubles in the image at 0x00671030
         and 0x00671028; each entry is sin(sample * step) * scale truncated
         to int by the CRT's __ftol. The two trailing calls pass angle 2.0
         and 0xe02d82d7 with scale 100; the first result is overwritten by
         the second, which stays in eax as this function's return.
// ORIGINAL: 0x0063B940 ?trig_init@@YAHXZ 0x0063B940-0x0063B9AE FILE BYTE_EXACT
// LEVER: index the table (`SinTable[sample]`) and test the int counter signed;
// a separate `int *entry` cursor makes VC6 compare the pointer unsigned
// (`jb` where the image has `jl`) and hold the counter in ecx instead of edx.
// The loop is a plain `for` - VC6 bottom-tests it itself.
// size      110 bytes
// prototype int __cdecl trig_init(void)
// callers   1   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0063B9B0 0x006458C8 0x006463E4
Return Value: n/a
Status: Complete
*/
// The game's own fixed-point sine, an int overload of the CRT's double
// sin(double) - the mangled name ?sin@@YAHHH@Z says both. Not recovered; the
// definition is the seam forwarder in leaf_recoveries.cpp.
int __cdecl sin(int angle, int scale);

int SinTable[256]; // 0x009BE6DC
int __cdecl trig_init() {
    for (int sample = 0; sample < 256; sample++) {
        SinTable[sample] = static_cast<int>(
            sin(static_cast<double>(sample) * 0.006159985596078431) * 65535.0);
    }
    sin(0x40000000, 100);
    return sin(static_cast<int>(0xe02d82d7), 100);
}
