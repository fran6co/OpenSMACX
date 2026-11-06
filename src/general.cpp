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

// Distinct writable string cells observed at the original call sites.
// These role names do not claim original source names or allocation bounds.
// RECOVERY_OBJECT: v1 0x00690F64 _InitialAutoSaveDeletePath
static char InitialAutoSaveDeletePath[] = "saves\\auto\\Alpha Centauri Autosave 1.SAV";

// Original mem_get at 0x005D4510 reports failure through this USER32 export; no function alias.
// RECOVERY_IMPORT: v1 __imp__MessageBoxA@16 USER32.dll name:MessageBoxA
#include "council.h"
#include <math.h> // sin(double), lowered to the CRT's _sin
#include <cstring>
#include <ddraw.h>  // IDirectDraw::Release in the DirectDraw teardown stage
// _disable / _enable for sub_5d4240, the VGA DAC loader - real VC6 intrinsics
// reproducing the image's cli/sti, not inline assembly.
#include <dos.h>
// _outp for the same body's DAC port writes.
#include <conio.h>
// VC6's real intrinsic lowering emits cli/out/sti without importing
// nonexistent _enable helpers. pushad/popad remain a documented
// source-vs-original caller-register-preservation obligation.
#pragma intrinsic(_disable, _enable, _outp)
#include "sprite.h"
#include "popup.h"
#include "buffer.h"
#include "main.h"  // legacy include; PopupAllocHook is owned by BasePop via popup.h

#include "general.h"
// Source-owned Filefind storage; original last_path immediates identify
// &FilefindPath.last_path at 0x009B8198 + 0x200, not a pointer variable.
// RECOVERY_OBJECT: v1 0x009B8198 ?FilefindPath@@3UFilefind@@A
Filefind FilefindPath;  // 0x009B8198
// RECOVERY_OBJECT: v1 0x009BB570 ?ParseStrPlurality@@3PAHA
int ParseStrPlurality[10];  // 0x009BB570; parse-family table, zero-initialized.
// RECOVERY_OBJECT: v2 0x009BB598 0x009BB5C0 ?ParseNumTable@@3PAHA 0x00625E30
int ParseNumTable[10];      // 0x009BB598
// RECOVERY_OBJECT: v1 0x009BB5C0 ?ParseStrGender@@3PAHA
int ParseStrGender[10];     // 0x009BB5C0
// RECOVERY_OBJECT: v1 0x009BB5E8 ?ParseStrBuffer@@3PAUchar256@@A
char256 ParseStrBuffer[10]; // 0x009BB5E8
#include "movie_media.h"
#include "alpha.h"
#include "game.h"
#include "log.h" // log_say
#include "mapwin.h" // draw_map, promoted out of temp.h
#include "strings.h"
#include "text.h"
// free bodies homed here out of the archive reach these
#include "map.h"     // zoc_any
#include "veh.h"     // can_arty
#include "alphamovie.h"  // MCIVideo - 0x00600330 is one of its members
#include "netdaemon.h"  // Shared receiver hierarchy and NetDaemonNet storage.
#include "xpops.h"   // X_pop
#include "filemap.h" // Filemap::open_read
// Utility bodies below use these source-owned drawing interfaces.
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

// The default noun forms parse_set/parse_say/parse_says/get_noun fall back to,
// at 0x009BBFEC and 0x009BBFF0 in the shipped image - REAL OBJECTS (zero at
// load there, as here; see general.h for the conversion from the `static
// T *const` bindings and the measurement behind it).
// RECOVERY_OBJECT: v2 0x009BBFEC 0x009BBFF0 ?GenderDefault@@3HA 0x0055D190
int GenderDefault;       // 0x009BBFEC
// RECOVERY_OBJECT: v2 0x009BBFF0 0x009BBFF4 ?PluralityDefault@@3HA 0x0055D190
BOOL PluralityDefault;   // 0x009BBFF0





/*
Purpose: Trim the trailing spaces in-line from the end of the string.
// ORIGINAL: 0x00600780 ?purge_trailing@@YAXPAD@Z 0x00600780-0x006007AD BYTE_EXACT
// body      src/general.cpp
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
MEASURED void __cdecl purge_trailing(LPSTR input) {
    // IT OVERWRITES EACH TRAILING SPACE, walking back, rather than placing a
    // single terminator: `mov byte ptr [eax], 0` at 0x006007A1 is inside the
    // loop. The empty string leaves early - `test eax, eax; je` at 0x0060078E
    // - and the `p == input` test comes AFTER the store.
    const size_t length = strlen(input);
    if (length == 0) {
        return;
    }
    char *end = input + length - 1;
    while (end >= input && *end == ' ') {
        *end = 0;
        if (end == input) {
            return;
        }
        --end;
    }
}

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
// `MEASURED inline`, not a bare definition: the image expands this body in
// place at its same-TU caller (purge_spaces 0x006007B0 folds the leading scan
// straight into its own frame, 19 of 39 agreeing with the `call` spelling),
// so the /Ob1 auto-inliner has to see the keyword - AND it still keeps the
// standalone copy at 0x00600760, which the bare `inline` dropped entirely once
// both of this unit's callers folded it and left the COMDAT unreferenced.
// dllexport forces the emission; nothing about the body changes. Declared
// MEASURED inline in general.h to match, which VC6 requires (C2375 otherwise).
MEASURED inline LPSTR __cdecl purge_leading(LPSTR input) {
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
// ORIGINAL: 0x00600840 ?add_lf@@YAXPAD@Z 0x00600840-0x00600859 BYTE_EXACT
// LEVER (2026-09-03, BYTE_EXACT 10/10 under the plain pair): the classic
//   post-increment walk - `*end++ = '\n'; *end = '\0';` - is the whole
//   lever. It changes the register allocation enough that VC6's /G5
//   scheduler puts the dependent `add eax, esi` BEFORE the independent
//   `add esp, 4` cleanup after `call strlen`, the image's order.
//
// HISTORY, kept because the misattribution is the lesson: six spellings
//   were measured in the other direction first - the index forms
//   (`str[n]`, 4/10), the pointer-with-subscript forms (`end[0]`/`end[1]`,
//   8/10), `*end`/`*(end+1)` (8/10), a named `const size_t`, a separate
//   `end += strlen(str)` increment, and writing through the parameter
//   (7/10) - and the residual swap was attributed to /G6 Pentium-Pro
//   scheduling (`/O2 /G6` does give 10/10; every OTHER body in this file
//   gets worse under it, so it was correctly kept out of FLAG_SETS) and
//   declared allocator-owned, "no source order reaches it". The sixth
//   spelling was simply the first idiomatic one. `osmx semantic` had
//   also refused the 8/10 form ("add operand is a different KIND").
//   Idiomatic-first, proven a fourth time: the hand-tuned shape was the
//   hypothesis-stop, and the mismatch was a diagnostic pointing at the
//   original's spelling.
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
    // THE POST-INCREMENT WALK IS THE LEVER. `*end++ = '\n'; *end = '\0';`
    // is what makes VC6's /G5 scheduler put `add eax, esi` BEFORE the
    // `add esp, 4` cleanup after `call strlen` - the image's order, 10 of
    // 10. Six spellings measured before this one: the pointer-with-index
    // forms all score 8 of 10 with the cleanup first, and writing through
    // the parameter is 7 of 10. The gap was misattributed to /G6 (Pentium
    // Pro scheduling - `/O2 /G6` does give 10/10) and declared
    // allocator-owned; the idiomatic spelling was the answer all along.
    // Idiomatic-first, again: the byte mismatch was a diagnostic, and the
    // hand-tuned shape was the hypothesis-stop.
    LPSTR end = str + strlen(str);
    *end++ = '\n';
    *end = '\0';
}

MEASURED void __cdecl swap(uint8_t *var1, uint8_t *var2) {
    *var1 ^= *var2;
    *var2 ^= *var1;
    *var1 ^= *var2;
}
// RECOVERY_LITERAL: v1 0x00696A38 0x00696A44 ??_C@_0M@MFAG@FATAL?5ERROR?$AA@ 0x005D4510
// RECOVERY_LITERAL: v1 0x00696A44 0x00696B14 ??_C@_0NA@POMN@Windows?5has?5run?5out?5of?5virtual?5m@ 0x005D4510
/*
Purpose: Allocate memory with error checking.
// ORIGINAL: 0x005D4510 ?mem_get@@YAPAXH@Z 0x005D4510-0x005D453C BYTE_EXACT
// symbol    ?mem_get@@YAPAXH@Z
// size      44 bytes
// prototype void* (__cdecl ?mem_get@@YAPAXH@Z)(int size)
// callers   88   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00644DFF 0x006470A6
// indirect  0x005D452E
Return Value: Pointer to allocated memory
Status: Complete. Historical note: an earlier DLL/CRT migration used two
        malloc wrappers to avoid a reported SDK incompatibility; the recovered
        callers now use the canonical pinned int-sized mem_get contract.
*/
LPVOID __cdecl mem_get(int size) {
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
    // Preserve the pinned signed guard: negative IDs pass and index before slot 0.
    if (id > 9) {
        return 3;
    }
    ParseNumTable[id] = value;
    return 0;
}

/*
Purpose: Use the string table input reference to copy a string into the global message buffer.
// ORIGINAL: 0x00625E50 ?parse_say@@YAHHHHH@Z 0x00625E50-0x00625EB3 BYTE_EXACT
// symbol    ?parse_say@@YAHHPADHH@Z
// LEVER: hoisting `StringTable.get(input)` into its own local BEFORE
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
int __cdecl parse_say(int id, LPSTR input, int gender, int pluralality) {
    if (id > 9) {
        return 3;
    }
    if (gender < 0) {
        gender = GenderDefault;
    }
    ParseStrGender[id] = gender;
    if (pluralality < 0) {
        pluralality = PluralityDefault;
    }
    ParseStrPlurality[id] = pluralality;
    // LEVER: NO `dest` LOCAL. Naming one costs the image's `lea` - a pointer
    // value assigned to a local is computed with `add eax, imm32` and then
    // copied into its home register, where the same address written out at
    // BOTH of its uses is one common subexpression off the array and the base
    // folds into the displacement (`lea esi, [eax + 0x9bb5e8]`). Same lever as
    // technology.cpp's SUBSCRIPTS note, and it also puts the `mov ecx,
    // 0x9b90d8` StringTable load back before the `lea`, where the local pushed
    // it after. 20/30 -> BYTE_EXACT.
    ParseStrBuffer[id].str[0] = 0;
    strcat(ParseStrBuffer[id].str, StringTable.get(input));
    return 0;
}

/*
Purpose: Copies the input string into the global message buffer.
// ORIGINAL: 0x00625EC0 ?parse_says@@YAHHPADHH@Z 0x00625EC0-0x00625F1A BYTE_EXACT
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
        gender = GenderDefault;
    }
    ParseStrGender[id] = gender;
    if (pluralality < 0) {
        pluralality = PluralityDefault;
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
    // TRIED (2026-08-29): declaring ParseStrBuffer as a real array object
    // (`extern char256 ParseStrBuffer[10]` defined in temp.cpp) - the
    // address-constant form still lowers to `add` here and in parse_say
    // (both unchanged). LANDED anyway that same day: the pointer form wrote
    // through unmapped image memory at runtime, and parse_num's BYTE_EXACT
    // claim survived the conversion. The image ITSELF lowers this same
    // access as `shl edi, 8; add edi, 0x9bb5e8` in parse_string
    // (0x00625BD8), so the lea at 0x00625EFF is VC6 scheduling context,
    // not reachable by the declaration or by any dest spelling measured.
    // LEVER (2026-09-03, 24/27 -> BYTE_EXACT): the "folding the dest local
    // away" attempt above must have kept a spelling that still named the
    // member through something other than a subscript. `dest[0] = 0` plus
    // `strcat(dest, input)` over a `char *const dest` local lowers the address
    // as `add eax, imm32` + a copy; writing BOTH uses as the array subscript
    // `ParseStrBuffer[id].str[...]` makes the address one common subexpression
    // and VC6 folds the base into the displacement:
    // `lea eax, [eax + 0x9bb5e8]`. Same lever as technology.cpp's SUBSCRIPTS.
    ParseStrBuffer[id].str[0] = 0;
    strcat(ParseStrBuffer[id].str, input);
    return 0;
}

// RECOVERY_ASSOCIATED: v1 switch_targets 0x00625880 0x00625DE0 0x00625DF8 0x006258E3
// RECOVERY_ASSOCIATED: v1 switch_selector 0x00625880 0x00625DF8 0x00625E23 0x006258DD
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
#if defined(_MSC_VER) && _MSC_VER == 1200
#pragma function(memcpy)
#endif
int __cdecl parse_string(LPSTR input, LPSTR output) {
    if (!input || !output) { // EBX || ESI
        return 3;
    }
    LPSTR output_base = output;
    LPSTR var; // EDI
    do {
        var = strstr(input, "$");
        if (!var) {
            break;
        }
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
            if (!strncmp(var, "$HEX", 4)) {
                int number = var[4] - '0';
                if (number > 9) {
                    return 14; // parse error
                }
                int len = var - input;
                memcpy(output, input, len);
                output += len;
                *output = 0;
                input = var + 5;
                char output_num[80];
                _itoa(ParseNumTable[number], output_num, 10);
                strcat(output, output_num);
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
                strcat(output, ParseStrBuffer[number].str);
                output += strlen(output);
            }
            break;
          }
          case 'N': { // done -> needs testing
            if (!strncmp(var, "$NUMBER", 7)) {
                int number = var[7] - '0';
                if (number > 9) {
                    return 14; // parse error
                }
                int len = var - input;
                memcpy(output, input, len);
                output += len;
                *output = 0;
                input = var + 8;
                char output_num[80];
                _itoa(ParseNumTable[number], output_num, 10);
                strcat(output, output_num);
                output += strlen(output);
            } else if (!strncmp(var, "$NUM", 4)) {
                int number = var[4] - '0';
                if (number > 9) {
                    return 14; // parse error
                }
                int len = var - input;
                memcpy(output, input, len);
                output += len;
                *output = 0;
                input = var + 5;
                char output_num[80];
                _itoa(ParseNumTable[number], output_num, 10);
                strcat(output, output_num);
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
                strcat(output, ParseStrBuffer[number].str);
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
            var = purge_leading(var);
            int gender; // The native '#' route leaves this local slot untouched.
            int plurality = 0;
            int number = -1;
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
                number = var[1] - '0';
                plurality = ParseNumTable[number] != 1;
                ++var;
                break;
              default:
                number = var[0] - '0';
                if (number >= 10) continue;
                gender = ParseStrGender[number];
                plurality = ParseStrPlurality[number];
                break;
            }
            ++var;
            if (number < 0 && var[0] >= '0' && var[0] <= '9') {
                plurality = var[0] - '1';
                ++var;
            }
            var = purge_leading(var);
            if (var[0] == ':') ++var;
            var = purge_leading(var);

            LPSTR selected_text = var;
            int colon_count = 0;
            for (LPSTR scan = var; scan < end_bracket; ++scan) {
                if (*scan == ':') ++colon_count;
            }
            if (colon_count == 0) {
                LPSTR number_text = findnum(var);
                if (number_text) {
                    number = number_text[0] - '0';
                    if (number >= 10) continue;
                    selected_text = ParseStrBuffer[number].str;
                    colon_count = 0;
                    for (LPSTR scan = selected_text; *scan; ++scan) {
                        if (*scan == ':') ++colon_count;
                    }
                }
            }
            int alternative = 0;
            if (colon_count == 1) {
                alternative = plurality != 0;
            } else if (colon_count == 2) {
                alternative = gender;
            } else if (colon_count == 3) {
                alternative = (gender != 0) + (plurality != 0 ? 2 : 0);
            } else if (colon_count == 5) {
                alternative = gender + (plurality != 0 ? 3 : 0);
            }
            while (alternative > 0) {
                while (*selected_text && *selected_text != ':') ++selected_text;
                if (!*selected_text) break;
                ++selected_text;
                --alternative;
            }
            while (*selected_text && *selected_text != ':' && *selected_text != '>') {
                *output++ = *selected_text++;
            }
            *output = 0;
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
                strcat(output, ParseStrBuffer[number].str);
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
            strcat(output, ParseStrBuffer[number].str);
            output += strlen(output);
            break;
          }
        }
    } while (var);

    strcat(output, input);

    if (Language == 1) { // French : handling for poor translations
        // Six inline 20-byte French prefix rows; the observed table window
        // does not establish any broader original allocation or source name.
        // RECOVERY_OBJECT: v2 0x006972D4 0x0069734C _?part_speech@?HL@??parse_string@@YAHPAD0@Z@4QAY0BE@$$CBDA 0x00625880
        static const char part_speech[6][20] = {
            " de ", " le ", " la ", "De ", "Le ", "La "
        };
        LPSTR search_from = output_base;
        for (;;) {
            LPSTR first = 0;
            int first_index = 0;
            for (int i = 0; i < 6; ++i) {
                LPSTR found = strstr(search_from, part_speech[i]);
                if (found && (!first || found < first)) {
                    first = found;
                    first_index = i;
                }
            }
            if (!first) break;
            int offset = first_index < 3 ? 2 : 1;
            char next = first[offset + 2];
            if (next == 'a' || next == 'e' || next == 'i' || next == 'o' ||
                next == 'u' || next == 'y' || next == 'h' || next == 'A' ||
                next == 'E' || next == 'I' || next == 'O' || next == 'U' ||
                next == 'Y' || next == 'H') {
                first[offset] = '\'';
                char *from = first + offset + 2;
                char *to = first + offset + 1;
                do { *to++ = *from; } while (*from++);
            }
            search_from = first + 1;
        }

    }

    return false;
}
#if defined(_MSC_VER) && _MSC_VER == 1200
#pragma intrinsic(memcpy)
#endif

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
        // Plain `strcat`, not `strcat_s`: the image calls 0x645470 at both
        // sites, and the `_s` spelling reaches `strcat` through the inline in
        // vc6_compat.h, which the optimiser then folds in WITH strcat's own
        // `repne scasb` + `rep movsd` block expansion. Naming `strcat`
        // directly keeps it a real call - the same idiom as the sibling
        // `filefind_init` and `Font::init`.
        if (!strchr(path, ':') && path[0] != '\\') {
            strcat(FilefindPath.alt_path, FilefindPath.exe_dir);
        }
        strcat(FilefindPath.alt_path, path);
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
//        (`??_GBasePop@@UAEPAXI@Z`, now compiler-generated in
//        `src/basepop.cpp`), dispatched here the same way `sound.cpp`
//        dispatches slot 0 - through
//        `vtable_method`, not by name, because the call site is an indirect
//        `call dword ptr [edx]`). `is_complete` is read only ONCE, after the
//        26-drive scan comes up empty, to decide whether to show that popup
//        at all - not as an up-front bypass. `field_3100_` (already a named
//        BasePop member) is the popup's own response code: 1 means Retry
//        (loop back to rescan, `jmp` target is BEFORE the `root_path` reset,
//        which an outer `for(;;)` with `continue` reproduces), anything else
//        falls out of the loop.
// TRIED (2026-09-03): `char root_path[8]` instead of `[5]` - the image's
//        frame is 0x24c against this tree's 0x248, four bytes, and a
//        char-array size was the cheapest candidate. The compiled frame does
//        not move (VC6 was already rounding the 5-byte array up), so the
//        missing four bytes are a separate 4-byte local the image declares
//        and this body does not; reverted to [5]. Frame measured after the
//        edit, 84/154 unchanged.
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
    // name. The BasePop library owns the real `PopupAllocHook` global;
    // calling it directly is the image's single memory-indirect.
    BasePop *const popup = PopupAllocHook();
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
                    delete popup;
                    return 0;
                }
            }
            root_path[0]++;
        }
        if (is_complete) {
            delete popup;
            return 0;
        }
        // send FILEFIND_NOCD message -> if doesn't exist (removed from jackal.txt) -> exit
        const int response = popup->start(const_cast<LPSTR>("jackal"), "FILEFIND_NOCD", -1, 0, 0, 0);
        if (response) {
            delete popup;
            return response;
        }
        // The popup's own input state is independent of start's return.
        if (popup->input_poll_state() == 1) {
            // if user response is to retry -> loop around again
            continue;
        }
        // if user response is "ok", exit loop
        SetCurrentDirectoryA(saved_dir);
        delete popup;
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
// TRIED (2026-09-03, probed against cl 12.00.8168 directly): the image's
//   `shr ebx, 1 / adc eax, 0` carry-chain fusion is NOT producible from VC6
//   C++, so this is a second case of hand-written assembly that
//   `tools/handwritten_asm.py` does not catch - it scans for `loop`, bare
//   `lodsb`, `xlatb`, `pushf/popf` and has no fingerprint for a shift-carry
//   accumulator. Twelve spellings compiled to assembly in isolation and
//   every one emitted `mov edx, ecx; and edx, 1; add eax, edx; shr ecx, 1;
//   jne`: do-while/while/for, accumulate-before-shift and after, `unsigned`
//   vs `int` counter, `if (bits & 1) count++`, the shift moved into the loop
//   CONDITION (`while (bits >>= 1)`), a 64-bit `unsigned __int64` accumulator
//   (that one does emit `adc`, but only as the HIGH half alongside the
//   `add eax, esi`, so the shape cannot converge), assignment to the
//   parameter before `return` to reach the image's
//   `mov [ebp+8],eax; mov eax,[ebp+8]` tail - which is reachable and is the
//   one half of the shape worth keeping. The image's loop is also rotated
//   with a duplicated trailing `adc` that no source form produced. Ceiling is
//   semantic equivalence, as for `checksum`/`checksum_password` above; 3/15
//   stands. Worth adding the fingerprint to handwritten_asm.py.
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
// symbol    ?bit_count_signed@@YAHH@Z
// size      18 bytes
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Bit count
Status: Complete
*/
int __cdecl bit_count_signed(int input) {
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
Purpose: Swap two 32-bit variables with the original XOR sequence; if both
         pointers alias, the sequence produces zero.
// ORIGINAL: 0x00628A50 ?swap@@YAXPAHPAH@Z 0x00628A50-0x00628A73 BYTE_EXACT
// body      src/general.cpp
// symbol    ?swap@@YAXPAH0@Z
// size      35 bytes
// prototype void (__cdecl ?swap@@YAXPAHPAH@Z)(int*, int*)
// callers   14   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
// Integer swap's source owner is out of line here; the byte overload stays
// inline in general.h. This is a source ownership decision, not proof of the
// original translation-unit boundary.
void __cdecl swap(int *var1, int *var2) {
    *var1 ^= *var2;
    *var2 ^= *var1;
    *var1 ^= *var2;
}


/*
Purpose: Swap the values of two 8-bit variables. Added an additional check when swapping the same
         memory location.
// ORIGINAL: 0x00628A80 ?swap@@YAXPAEPAE@Z 0x00628A80-0x00628AA3 BYTE_EXACT
// body      src/general.cpp
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
// Fidelity limitation: the pinned 0x00628ADF instruction is a 32-bit IDIV,
// which also faults when the quotient does not fit EAX. This wide division
// followed by an int conversion does not express that quotient-overflow trap;
// the out-of-range quotient domain remains an unresolved original-fidelity
// obligation. Do not synthesize a fault path here.
int __cdecl fixed_div(int numerator, int denominator) {
    // EDX:EAX contains the signed numerator scaled by 65536. Multiplication
    // preserves that dividend for negative gradients without C++'s undefined
    // left shift of a negative value. As in the original IDIV, the divisor
    // must be nonzero and the quotient must fit the signed 32-bit result.
    return (int)(((int64_t)numerator * 65536) / denominator);
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
// TRIED (2026-09-03, probed against cl 12.00.8168 in isolation): six single-
// exit shapes meant to reach the image's shared epilogue and its
// `mov [ebp-4],eax; mov eax,[ebp-4]` store-reload of the return value -
// a `result` local assigned on every path then returned once, an
// initialised `result = 0` with `break` out of the loop, a `goto done`
// past the assignment, a `while (count-- > 0)` form, a ternary for the
// not-found value, and a `p` local returned from the outer scope - none
// produced a memory-homed return value; VC6 kept every one in esi/eax and
// emitted a separate early-return epilogue instead of a shared one. The
// image also loads BOTH parameters into registers before testing
// (`mov ebx,[ebp+8]; and ebx,ebx` / `mov ecx,[ebp+0xc]; and ecx,ecx`),
// packs `value` into `bl` over the dead `start`, and scans with
// `dec eax; dec ecx; jne` and no peeled compare - none of which any flag
// set or spelling here reaches. CONTEXT WORTH WEIGHING BEFORE GRINDING:
// this address sits between `fixed_div` (0x00628AD0, `idiv` on a
// hand-built edx:eax dividend) and `findnum` (0x00628B30), with
// `checksum`/`checksum_file`/`checksum_password` (0x00538FE0-0x005390C0,
// confirmed hand-written by handwritten_asm.py) a screen away - five small
// utilities whose shipped bytes are all compiler-unreachable idioms. Treat
// the ceiling here as semantic equivalence and say so, rather than
// continuing to search source shapes.
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
Purpose: Calculate a basic XOR checksum for the data buffer.
// The pinned body uses forward LODSB and a 32-bit ECX LOOP counter.
// Investigated VC6 C++ recipes emit a different loop; matching remains open.
// Preserve the original traversal/count semantics in ordinary C++, without
// inline assembly or source changes made solely to imitate instruction length.
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
    uint32_t remaining = static_cast<uint32_t>(size);
    while (remaining != 0) {
        seed ^= *buffer;
        ++buffer;
        --remaining;
    }
    return seed;
}

/*
Purpose: Calculate a basic XOR checksum for a password string.
// The pinned body uses forward LODSB and a 32-bit ECX LOOP counter.
// Investigated VC6 C++ recipes emit a different loop; matching remains open.
// Preserve the original traversal/count semantics in ordinary C++, without
// inline assembly or source changes made solely to imitate instruction length.
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
    remove(InitialAutoSaveDeletePath);
}

/*
Purpose: Handle the creation and management of the auto-save game files.
// ORIGINAL: 0x005ABD20 ?auto_save@@YAXXZ 0x005ABD20-0x005ABE39
// TRIED (2026-09-03, probed against cl 12.00.8168 in isolation): the
//   image's split cleanup - a batched `add esp, 0x1c` covering the seven
//   `remove` calls and then `pop ecx` for the trailing `save_daemon`, so 0x1c
//   + 4 rather than this tree's single `add esp, 0x20` - is the per-call
//   cleanup merged by peephole with a size-shaped `pop ecx` on the last one.
//   Probed with a two-branch function shaped like this one (7 same-callee
//   calls then a different one-arg callee, then a mixed run): /O2 defers all
//   eight into one `add esp, 0x20` and elides nothing else; /O1, /O2 /Os and
//   /O1 /Oy keep the `pop ecx` but replace the pushes with
//   `mov dword ptr [esp], imm32` argument-slot reuse, which the image does
//   NOT have (it pushes every argument); /O2 /Os and /O2 /G6 likewise. No
//   combination of /O1//O2 /Os//Ot /Ob0 /Oi- /Oy- /G5//G6 in FLAG_SETS or
//   outside them produces fresh pushes, a partial batched `add esp`, AND a
//   trailing `pop ecx` - which reads as this translation unit having been
//   compiled with the stack-pop deferral off, a setting the flag space here
//   does not model. listing_diff agrees the ONLY two differences in the body
//   are those cleanups (image 66 instructions, this tree 64); 28/66 is the
//   byte-shift cost of the two bytes they add, not four divergent
//   instructions.
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
        if (GameDataStateStorage.fields_.game_rules_ & RULES_IRONMAN && !(GameDataStateStorage.fields_.game_state_ & STATE_SCENARIO_EDITOR)) {
            remove("saves\\auto\\Alpha Centauri Autosave 30.SAV");
            remove("saves\\auto\\Alpha Centauri Autosave 20.SAV");
            remove("saves\\auto\\Alpha Centauri Autosave 10.SAV");
            remove("saves\\auto\\Alpha Centauri Autosave 5.SAV");
            remove("saves\\auto\\Alpha Centauri Autosave 4.SAV");
            remove("saves\\auto\\Alpha Centauri Autosave 3.SAV");
            remove("saves\\auto\\Alpha Centauri Autosave 2.SAV");
        } else { // standard auto saves
            if (!(GameDataStateStorage.fields_.turn_current_num_ % 10)) {
                remove("saves\\auto\\Alpha Centauri Autosave 30.SAV");
                rename("saves\\auto\\Alpha Centauri Autosave 20.SAV",
                    "saves\\auto\\Alpha Centauri Autosave 30.SAV");
                rename("saves\\auto\\Alpha Centauri Autosave 10.SAV",
                    "saves\\auto\\Alpha Centauri Autosave 20.SAV");
                save_daemon(const_cast<char *>("saves\\auto\\Alpha Centauri Autosave 10"));
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
        }
        save_daemon(const_cast<char *>("saves\\auto\\Alpha Centauri Autosave 1"));
    }
}

/*
Purpose: Extended auto-saving regardless of the game type or settings for debug purposes.
Original Offset: n/a
Return Value: n/a
Status: Complete
*/
void __cdecl auto_save_debug() {
    if (GameDataStateStorage.fields_.turn_current_num_ == 1) {
        remove("saves\\auto\\Alpha Centauri Autosave Turn 1.SAV");
        save_daemon(const_cast<char *>("saves\\auto\\Alpha Centauri Autosave Turn 1"));
    }
    if (!(GameDataStateStorage.fields_.turn_current_num_ % 10)) {
        remove("saves\\auto\\Alpha Centauri Autosave 500.SAV");
        char save_path_new[45];
        char save_path_old[45];
        for (int i = 0; i < 490; i += 10) {
            sprintf_s(save_path_old, 45, "saves\\auto\\Alpha Centauri Autosave %d.SAV", 490 - i);
            sprintf_s(save_path_new, 45, "saves\\auto\\Alpha Centauri Autosave %d.SAV", 500 - i);
            rename(save_path_old, save_path_new);
        }
        save_daemon(const_cast<char *>("saves\\auto\\Alpha Centauri Autosave 10"));
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
    save_daemon(const_cast<char *>("saves\\auto\\Alpha Centauri Autosave 1"));
}



/*
Purpose: The sound-header version the game was built with. The whole image
         body is `mov eax,1 / ret` - the compiler folded the header's version
         constant to the literal - and load_sound_dll compares its answer
         against the sound DLL's own reported version and against 1, showing
         the "check all sound.h and sound device.h versions" complaint when
         they disagree. Recovered as the plain __cdecl function the image's
         only caller calls through, replacing the generator's
         field_accessor_0062d390_redirect: that adapter's two dead __fastcall
         slots would have cost the call site the two register setups the
         image does not have, and a redirect nothing calls is class debt. Was
         a BYTE_EXACT claim in field_accessors.cpp under the redirect's
         symbol; re-record here against the real name.
// ORIGINAL: 0x0062D390 sub_62d390 0x0062D390-0x0062D396 BYTE_EXACT
// symbol    _sound_header_version
// size      6 bytes
// prototype
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 1
Status: Complete
*/
extern "C" int __cdecl sound_header_version() {
    return 1;
}

// SDK imports, never raw IAT pointers.
// RECOVERY_IMPORT: v1 __imp__FindClose@4 KERNEL32.dll name:FindClose
// RECOVERY_IMPORT: v1 __imp__FindFirstFileA@8 KERNEL32.dll name:FindFirstFileA
// RECOVERY_IMPORT: v1 __imp__GetDC@4 USER32.dll name:GetDC
// RECOVERY_IMPORT: v1 __imp__ReleaseDC@8 USER32.dll name:ReleaseDC
// ===== MANAGED GLOBALS - real objects, homed to their domain =====
// In the shipped image these live at fixed data addresses and are
// constructed before WinMain by the CRT's dynamic-initializer walk
// (winedbg-confirmed: walker return 0x00644EEB, table at 0x682568..).
// Here the same recovered constructors run through this build's own
// startup, and the matching destructors close them at exit.
// Faction artwork is owned by the genuine FactionArt array in faction.cpp;
// its Sprite members now participate in C++ construction and destruction.

/*
// ORIGINAL: 0x0063B9B0 ?sin@@YAHHH@Z 0x0063B9B0-0x0063BABB BYTE_EXACT
// PROMOTED 2026-08-30 from src/unrecovered/0063b9b0.cpp: the edi pressure came
// from NAMING the second table value. Loading SinTable[index + 1] straight
// into the expression that subtracts t0 gives the value's whole life to eax,
// and shifting `scale` in place before the loads (not at the use) puts the
// `sar edx, N` where the image has it.
// size      267 bytes
// prototype
// callers   2   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
int __cdecl sin(int angle, int scale) {
    if (angle & 0x80000000) {
        scale = -scale;
    }
    const int quad = angle & 0x40000000;
    const int masked = angle & 0x3fffffff;
    const int index = masked >> 22;
    // The table loads are spelled in BOTH arms and let the compiler CSE them
    // to the merge point after the quadrant test - the image's order is test,
    // load t0, load t1, branch, which is what frees esi for t0.
    if (scale < 0xffff) {
        if (quad != 0) {
            const int t0 = SinTable[index];
            return (((SinTable[index + 1] - t0) * (masked & 0x3fffff) >> 22)
                     - t0 + 0xffff)
                   * scale
                   >> 16;
        }
        const int t0 = SinTable[index];
        return (((SinTable[index + 1] - t0) * (masked & 0x3fffff) >> 22) + t0)
               * scale
               >> 16;
    }
    if (scale < 0xffffff) {
        scale >>= 8;
        if (quad != 0) {
            const int t0 = SinTable[index];
            return (((SinTable[index + 1] - t0) * (masked & 0x3fffff) >> 22)
                     - t0 + 0xffff)
                   * scale
                   >> 8;
        }
        const int t0 = SinTable[index];
        return (((SinTable[index + 1] - t0) * (masked & 0x3fffff) >> 22) + t0)
               * scale
               >> 8;
    }
    scale >>= 16;
    if (quad != 0) {
        const int t0 = SinTable[index];
        return (((SinTable[index + 1] - t0) * (masked & 0x3fffff) >> 22)
                 - t0 + 0xffff)
               * scale;
    }
    const int t0 = SinTable[index];
    return (((SinTable[index + 1] - t0) * (masked & 0x3fffff) >> 22) + t0)
           * scale;
}

// Original trig_init at 0x0063B950 selects the output cursor; sin at 0x0063B9E0 indexes this base; no extent claim.
// RECOVERY_OBJECT: v1 0x009BE6DC ?SinTable@@3PAHA
int SinTable[256]; // 0x009BE6DC

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
// TRIED 2026-09-03 (pair /O2 and /O2 /Oy- only; STOPPED, evidence recorded):
//   the body is RIGHT and the gap is the /Oi intrinsic axis, which no source
//   spelling reaches. The image CALLS the CRT `sin` (0x006463E4); both allowed
//   sets imply /Oi, and /Oi turns `sin(double)` into the x87 `fsin` - which
//   also deletes the image's `and esp, 0xfffffff8`, because that alignment
//   exists only to receive the stack-passed double the CRT call needs. 3 of 33
//   agreeing, compiled 29 against the image's 33. Measured directly that
//   `/c /O2 /Oi- /Gy /GR- /GX` on this exact body gives the image's
//   instruction sequence back byte for byte, `and esp` and the separate
//   `call _sin` / `call __ftol` included, so it is the intrinsic axis and
//   nothing else. `#pragma function(sin)` beside the `strcat`/`strcpy`/
//   `strlen`/`strcmp` list in stdafx.h is the source-side fix, and
//   the retired 0x00634F20 seed (git fee2514f) and 00627440.cpp both show per-file
//   `#pragma function` is an established device in this tree - but it is
//   either a shared-file edit or a pragma, and this pass is barred from both.
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
int __cdecl trig_init() {
// RECOVERY_LITERAL: v1 0x00671028 0x00671030 __real@8@400effff000000000000 0x0063B940
// RECOVERY_LITERAL: v1 0x00671030 0x00671038 __real@8@3ff7c9d9b456e9139000 0x0063B940
    for (int sample = 0; sample < 256; sample++) {
        SinTable[sample] = static_cast<int>(
            sin(static_cast<double>(sample) * 0.006159985596078431) * 65535.0);
    }
    sin(0x40000000, 100);
    return sin(static_cast<int>(0xe02d82d7), 100);
}

// Historical TRIED (2026-09-06): the first general.cpp trial failed
// at link because the CRT did not resolve _enable; the old note also
// recorded that dos.h treated _disable/_enable as calls and VC6 had no
// intrin.h declaration. Retained finding: this body requires a pragma
// intrinsic, not the header's callable fallback. The explicit pragma
// above selects the compiler's instruction lowering instead of CRT calls.
// Original pushad/popad restores volatile caller registers too; ordinary
// C++ does not promise that stronger behavior. The conversion and I/O order
// have a C++ owner; native exactness and extra register preservation stay open.
// ORIGINAL: 0x005D4240 sub_5d4240 0x005D4240-0x005D427C
// RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x005D4240-0x005D427C: complete60-byte listing performs cli, selects DAC index0, writes256 RGB triplets from four-byte-spaced input, then sti", "build/recovery/palette-intrinsic-control-oct6.cpp: VC6 pragma intrinsic control has no unresolved helpers and emits physical cli/out/sti; it does not execute privileged I/O", "src/general.h: existing free __cdecl sub_5d4240(unsigned char*) declaration; source preserves that ABI rather than fabricating a Palette receiver or caller", "src/general.cpp: existing DOS/conio includes were specifically retained for this hardware routine"], "reason": "Own the original VGA DAC operation in the existing general hardware-helper TU. Preserve index selection, 256 four-byte-stride RGB conversions, output order and interrupt enable; retain the original full span/history while explicitly leaving pushad/popad volatile-register restoration as a native exactness obligation. No hardware port access is run by the host control."}
// symbol    ?sub_5d4240@@YAXPAE@Z
void __cdecl sub_5d4240(unsigned char *palette_rgb) {
    _disable();
    _outp(0x3c8, 0);
    int n = 0x100;
    do {
        _outp(0x3c9, palette_rgb[0] >> 2);
        _outp(0x3c9, palette_rgb[1] >> 2);
        _outp(0x3c9, palette_rgb[2] >> 2);
        palette_rgb += 4;
        n--;
    } while (n != 0);
    _enable();
}

// ORIGINAL: 0x00589550 j_GraphicWin::close 0x00589550-0x00589555 FILE BYTE_EXACT
// symbol    ?j_close@GraphicWin@@QAEXXZ
// LEVER: implicit-thiscall member forwarding; explicit __thiscall on a free fn is C4234
// working copy - scaffold materialised by --work
// size      5 bytes
// prototype 
// callers   0   call targets   0
// kind      thunk
// flags     hidden;thunk;sp_ready;purged_ok
// calls     (none)


// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: j_GraphicWin::close  at 0x00589550  (5 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// The VC6 dialect limits and the source-form rules used to live here.
// They are knowledge, not scaffolding, so they now live in the agent
// system prompt (mizuchi.yaml, plugins.claude-runner.systemPrompt),
// where they can be edited without regenerating anything and are in
// context from the first token rather than behind a file read. This
// emitter computes declarations; it does not carry lessons.

typedef int int32_t;
typedef unsigned int uint32_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef int int32;
typedef unsigned int uint32;
typedef short int16;
typedef unsigned short uint16;
// `char`, NOT `signed char`. They are distinct MSVC types and mangle
// differently - D against C - and the catalogue's `int8` means the first:
// counted over every catalogued mangled name, `PAD` appears 508 times and
// `PAC` once. Spelling it `signed char` made 150 derived prototypes emit a
// symbol no target object holds. `int8_t` keeps its C meaning below; neither
// catalogue ever uses it.
typedef char int8;
typedef unsigned char uint8;
// GraphicWin is graphicwin.h's real class, which general.cpp already includes -
// the thunk is a member of it, not of a local shim.
void GraphicWin::j_close() {
    close();
}

// RECOVERY_ASSOCIATED: v1 switch_targets 0x00628B70 0x00628D94 0x00628DAC 0x00628C65
// ORIGINAL: 0x00628B70 ?HSV_to_RGB@@YAXPAUHSV@@PAUPALETTEENTRY@@@Z 0x00628B70-0x00628D92 FILE BYTE_EXACT
// symbol    ?HSV_to_RGB@@YAXPAUHSV@@PAUtagPALETTEENTRY@@@Z
// size      546 bytes
// prototype
// callers   0   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x006458C8 0x00647851
// Purpose: Convert one HSV triplet (palette.h's 0x18-byte double record) into
//          the palette entry the game stores. Parameters named from use:
//          `hsv_in` is read only, `entry_out` is written only.
// NOTE: the image's own mangled name spells the second parameter `PALETTEENTRY`;
// this tree emits `tagPALETTEENTRY` because the type comes from the Windows
// header rather than a local struct - see the `symbol` fact above.
void __cdecl HSV_to_RGB(HSV *hsv_in, PALETTEENTRY *entry_out) {
    if (hsv_in != 0 && entry_out != 0) {
        HSV hsv = *hsv_in;
        double r, g, b;

        if (hsv.s == 0.0) {
            if (hsv.h == -1.0) {
                r = hsv.v;
                g = hsv.v;
                b = hsv.v;
            }
        } else {
            if (hsv.h == 360.0)
                hsv.h = 0.0;
            hsv.h *= 0.016666666666666666;
            int i = (int)floor(hsv.h);
            double f = hsv.h - i;
            double p = hsv.v * (1.0 - hsv.s);
            double q = hsv.v * (1.0 - hsv.s * f);
            double t = hsv.v * (1.0 - hsv.s * (1.0 - f));
            switch (i) {
            case 0: r = hsv.v; g = t; b = p; break;
            case 1: r = q; g = hsv.v; b = p; break;
            case 2: r = p; g = hsv.v; b = t; break;
            case 3: r = p; g = q; b = hsv.v; break;
            case 4: r = t; g = p; b = hsv.v; break;
            case 5: r = hsv.v; g = p; b = q; break;
            }
        }

        entry_out->peRed = (unsigned char)(r * 256.0);
        entry_out->peGreen = (unsigned char)(g * 256.0);
        entry_out->peBlue = (unsigned char)(b * 256.0);
    }
}


// ORIGINAL: 0x00538FE0 ?checksum_file@@YAEPAD@Z 0x00538FE0-0x0053908F;0x0065E450-0x0065E462 FILE
// working copy - scaffold materialised by --work
// size      193 bytes
// prototype 
// callers   2   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00628380 0x006283E0 0x00628430 0x006287C0
// indirect  0x0053904A

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?checksum_file@@YAEPAD@Z  at 0x00538FE0  (193 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// The VC6 dialect limits and the source-form rules used to live here.
// They are knowledge, not scaffolding, so they now live in the agent
// system prompt (mizuchi.yaml, plugins.claude-runner.systemPrompt),
// where they can be edited without regenerating anything and are in
// context from the first token rather than behind a file read. This
// emitter computes declarations; it does not carry lessons.

unsigned char __cdecl checksum_file(char *file_path) {
    Filemap fm;
    char *data = (char *)fm.open_read(file_path, 1);
    if (data == 0) {
        return 0;
    }
    unsigned int size = GetFileSize(fm.get_handle(), 0);
    unsigned char sum = 0;
    do {
        sum ^= *data;
        --size;
        ++data;
    } while (size != 0);
    fm.close();
    return sum;
}

// ---------------------------------------------------------------------------
// THE ENEMY AI MOVE DRIVER (0x00579240 / 0x00579510), homed 2026-09-06.
//
// enemy_turn sweeps the vehicle table ten times, once per priority band, and
// hands each of the faction's vehicles to enemy_veh; enemy_veh drives one
// vehicle's move and reports whether the turn moved on. enemy_move, the
// per-vehicle mover both of them reach, now has its complete source owner in
// src/enemy.cpp.
// ---------------------------------------------------------------------------

// Original direct-address writable .data strings passed to the logging calls.
// RECOVERY_OBJECT: v2 0x0068EAFC 0x0068EB0F ?LogEnemyVehicleDone@@3PADA 0x00579240
char LogEnemyVehicleDone[] = "Enemy Vehicle DONE";
// RECOVERY_OBJECT: v2 0x0068EB10 0x0068EB25 ?LogEnemyVehicleAction@@3PADA 0x00579240
char LogEnemyVehicleAction[] = "Enemy Vehicle ACTION";
// RECOVERY_OBJECT: v2 0x0068EB28 0x0068EB36 ?LogEnemyVehicle@@3PADA 0x00579240
char LogEnemyVehicle[] = "Enemy Vehicle";

// ORIGINAL: 0x00579240 ?enemy_veh@@YAHH@Z 0x00579240-0x005794C8
// TRIED: `Veh *veh = &Vehs[veh_id]` - the image never materialises the array
//        base, it folds 0x9528xx into every access displacement and keeps only
//        `unit_id * sizeof(Veh)` live in edi; a pointer local puts the base in
//        the register and the field offset in the displacement instead, so
//        every member access disagrees. The array spelling `Vehs[unit_id].f`
//        is what is here.
// TRIED: reassigning the parameter (the image reuses [ebp+8] for the
//        local-faction flag, so it must have done) - no change, still 4/208
//        with the same push/mov order, so the readable named local stays.
// LEVER: not reproduced - the image's two jump tables on the faction id are
//        32 instructions that both map 0..7 onto themselves (identity, so no
//        source computes anything) and this body has nothing where they are.
//        Everything else is register placement: image `mov ebx,[ebp+8]` /
//        ours `push esi; push edi; mov edi,[ebp+8]`, and the base-fold above.
int __cdecl enemy_veh(int veh_id) {
    // The image keeps the id in ebx for the whole body and reuses the
    // parameter's own stack slot for the local-faction flag, which is why the
    // id is held in a local of its own. Every field is read through
    // `Vehs[unit_id]` rather than through a Veh * local: the image folds
    // Vehs's address into each access displacement and keeps only
    // `unit_id * sizeof(Veh)` live in edi, which is what the array spelling
    // compiles to and what a pointer local does not.
    const int unit_id = veh_id;
    const uint8_t unit_faction = Vehs[unit_id].faction_id;

    bool is_local_turn;
    if (unit_faction == ConsoleGlobal.faction_id_ && IsMultiplayerNet != 0) {
        is_local_turn = true;
    } else {
        is_local_turn = false;
    }

    if (IsMultiplayerNet != 0) {
        NetDaemonNet.channels_[0].leading_words_[3] = 3;
    }

    do_all_non_input();

    // The image dispatches twice through jump tables on the faction id here,
    // and both tables map every value 0..7 onto itself: the first loads the
    // same value it was handed, the second does it again. Nothing observable
    // is computed, so there is no source to write.

    if (Vehs[unit_id].moves_expended != 0
            && Vehs[unit_id].order == ORDER_MOVE_TO
            && VehPrototypes[Vehs[unit_id].proto_id].plan != 7
            && !(Vehs[unit_id].state & VSTATE_EXPLORE)) {
        if (zoc_any(Vehs[unit_id].x, Vehs[unit_id].y,
                    unit_faction)
            == 0) {
            goto after_move;  // blocked by a zone of control
        }
    }

    if (enemy_move(unit_id) != 0) {
        return 1;
    }

    {
        int moves_left = speed(unit_id, 0) - Vehs[unit_id].moves_expended;
        if (moves_left < 0 || (moves_left < 1000 && moves_left == 0)) {
            if (IsMultiplayerNet != 0) {
                log_say(LogEnemyVehicleDone, unit_id, 0, 0);
            }
            if (is_local_turn) {
                synch_veh(unit_id);
                NetDaemonNet.await_synch();
                NetDaemonNet.unlock_veh();
            }
            return 1;
        }
    }

after_move:
    if (is_local_turn) {
        synch_veh(unit_id);
        NetDaemonNet.await_synch();
        if (NetDaemonNet.action(unit_id, 1) == 0) {
            return 0;
        }
        return 1;
    }

    if (IsMultiplayerNet != 0) {
        char number_text[0x50];
        char message[8];
        message[0] = 0;
        _itoa(unit_id, number_text, 10);
        strcat(message, number_text);
        log_say(LogEnemyVehicleAction, message, Vehs[unit_id].order,
                Vehs[unit_id].waypoint_x[0], Vehs[unit_id].waypoint_y[0]);
    }

    if (action(unit_id) == 0) {
        veh_skip(unit_id);
    }

    if (IsMultiplayerNet != 0) {
        char state_text[2];
        state_text[0] = Vehs[unit_id].move_to_ai_type;
        state_text[1] = 0;
        log_say(LogEnemyVehicle, state_text, unit_id, Vehs[unit_id].x,
                Vehs[unit_id].y);
    }

    return 0;
}

// ---------------------------------------------------------------------------
// THE MOVIE FRAME BLIT (0x00600330), homed 2026-09-06.
//
// An MCIVideo member, not a Win one: the receiver's +8 is the GetDC/ReleaseDC
// window and its +0x46C..+0x478 are the four BitBlt destination arguments,
// which is MCIVideo's field run (alphamovie.h), not Win's (sizeof(Win) is
// 0x444, short of every offset here). get_hdc/release_hdc ARE Win's - they are
// static, so any member can call them.
// ---------------------------------------------------------------------------

// ORIGINAL: 0x00600330 sub_600330 0x00600330-0x00600393 BYTE_EXACT
// symbol    ?blit_frame@MCIVideo@@QAEXXZ
// LEVER: the owner is the field run, not the catalogue - the image's span
//        0x00600320 (is_playing) is the byte before it and MCIVideo is the
//        only class in the tree whose fields reach 0x478.
void MCIVideo::blit_frame() {
    if (target_window_ == 0) {
        return;
    }

    HDC hdc = GetDC(window_handle_);
    if (hdc == nullptr) {
        return;
    }

    // The image pushes the eight BitBlt arguments first, calls get_hdc in the
    // middle of them and pushes its return as BitBlt's own first argument - a
    // static call with no arguments of its own, wedged into the argument
    // build. Spelled as the nested call it is.
    BitBlt(Win::get_hdc(), blit_x_, blit_y_, blit_width_, blit_height_, hdc, 0,
           0, SRCCOPY);

    Win::release_hdc();
    ReleaseDC(window_handle_, hdc);
}

// ORIGINAL: 0x00579510 ?enemy_turn@@YAHH@Z 0x00579510-0x00579769
// RECOVERY_BINDING: v1 {"evidence": ["Pinned 0x00579643-0x00579687 routes priorities7..9 around the DROP_POD, plan4 and unk_1==0x15 filters used by priorities0..6; priority8 alone then requires plan7 and nonzero faction at0x0057968A-0x005796A5.", "Original rejected priority8 paths jump0x005795A7 to decrement the vehicle index and pointer; the for-loop increment clause preserves this tail for continue as well as ordinary completion."], "reason": "Preserve original enemy-turn early-band filters, late-band bypass and common descending-vehicle tail."}
// symbol    ?enemy_turn@@YAXH@Z
// LEVER: void, not the catalogue's int - the body's only `ret` leaves eax
//        holding whatever the last address calculation left there
//        (`faction_id * 0x20CC`, from the PlayersData store), not a return
//        value. See the same recast on 0x00579510's archived transcription.
// TRIED: `bool matched` locals - the image has no such variable. Each band
//        ends in two bare jumps (one to the matched point 0x579648, one to the
//        next-vehicle tail), with no setcc and no store, so a `matched`
//        local costs a `setne al; test al,al` pair the image does not have.
//        Faithful source here would be goto-per-band, which this pass did not
//        spend the budget on: the body is 12/184 on register placement alone.
// TRIED: `Veh *veh` across the sweep - the image keeps exactly one such
//        pointer (esi, `sub esi,0x34`), this tree spills a second to
//        [ebp-0xc], so the frame is 0xc against the image's 8.
void __cdecl enemy_turn(int faction_id) {
    for (int priority = 0; priority < 10; priority++) {
        int veh_id = GameDataStateStorage.fields_.vehicle_current_count_ - 1;
        if (veh_id < 0) {
            continue;
        }
        Veh *veh = &Vehs[veh_id];
        for (; veh_id >= 0; --veh_id, --veh) {
            if (veh_id < GameDataStateStorage.fields_.vehicle_current_count_ && veh->faction_id == faction_id) {
                bool matched = false;
                if (priority == 0) {
                    matched = can_arty(veh->proto_id, 1) != 0;
                } else if (priority == 1) {
                    if (VehPrototypes[veh->proto_id].plan == 2) {
                        matched = true;
                    } else if (VehPrototypes[veh->proto_id].plan == 7) {
                        matched = stack_check(veh_id, 3, 0, -1, -1) != 0;
                    }
                } else if (priority == 2) {
                    if (Chassis[VehPrototypes[veh->proto_id].chassis_id].triad
                            == priority) {
                        matched = VehPrototypes[veh->proto_id].plan != 4;
                    }
                } else if (priority == 3) {
                    matched = VehPrototypes[veh->proto_id].plan == 1;
                } else if (priority == 4) {
                    matched = veh->dmg_incurred == 0;
                } else {
                    matched = true;
                }

                if (matched && (priority >= 7
                        || (!has_abil(veh->proto_id, ABL_DROP_POD)
                        && VehPrototypes[veh->proto_id].plan != 4
                        && VehPrototypes[veh->proto_id].unk_1 != 0x15))) {
                    if (priority == 8
                            && (VehPrototypes[veh->proto_id].plan != 7
                                   || faction_id == 0)) {
                        continue;
                    }

                    if (veh_id >= 0) {
                        int turns_spent = 0;
                        while (veh->order != ORDER_SENTRY_BOARD) {
                            if (priority != 8 || turns_spent != 0) {
                                int moves_left =
                                    speed(veh_id, 0) - veh->moves_expended;
                                if (moves_left < 0
                                        || (moves_left < 1000
                                               && moves_left == 0)) {
                                    break;
                                }
                            }

                            const int veh_count_before = GameDataStateStorage.fields_.vehicle_current_count_;
                            if (enemy_veh(veh_id) != 0
                                    || veh_count_before != GameDataStateStorage.fields_.vehicle_current_count_) {
                                break;
                            }

                            if (veh->order != ORDER_MOVE_TO
                                    && Chassis[VehPrototypes[veh->proto_id]
                                                   .chassis_id]
                                           .triad
                                       != 2) {
                                turns_spent++;
                            }
                            turns_spent++;
                            if (turns_spent > 0x20) {
                                veh_skip(veh_id);
                            }
                        }
                    }
                }
            }
        }
    }

    PlayersData[faction_id].flags &= ~PFLAG_UNK_10000;
}


// ORIGINAL: 0x005391A0 sub_5391a0 0x005391A0-0x005391C0 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:f178e8db:src/recovered/005391a0.cpp complete original identity/spans and historical claims/findings.", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x5391a0 full native body review.", "source:src/general.cpp existing real state owners; no executable-address alias or synthetic receiver."], "reason": "Recover the complete short helper through established source-owned diplomacy, vehicle, text or window state; preserve signed sentinel behavior, original receiver/argument order and conditional callbacks."}
// symbol    ?read_and_parse_text_line@@YAXXZ
void __cdecl read_and_parse_text_line() {
  text_get();
  char *input = TextBufferGetPtr;
  StringTemp[0] = 0;
  parse_string(input, StringTemp);
}

/*
// ORIGINAL: 0x0063BAC0 ?cos@@YAHHH@Z 0x0063BAC0-0x0063BAD9 BYTE_EXACT
// symbol    ?cos@@YAHHH@Z
// size      25 bytes
// prototype
// callers   0   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x0063B9B0
Status: Complete
*/
int __cdecl cos(int a1, int a2) {
    return sin(static_cast<int>(static_cast<uint32_t>(a1) + 0x3fffffffU), a2);
}
