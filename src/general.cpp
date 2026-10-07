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
#include "main.h"  // PopupAllocHook - the tree's own 0x00696ECC slot

// close_class methods, homed to checkbox.cpp / radiobutton.cpp
void __cdecl teardown_0060fd60();
void __cdecl teardown_0060e5d0();
#include "temp.h"
#include "general.h"
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
// The default noun forms parse_set/parse_say/parse_says/get_noun fall back to,
// at 0x009BBFEC and 0x009BBFF0 in the shipped image - REAL OBJECTS (zero at
// load there, as here; see general.h for the conversion from the `static
// T *const` bindings and the measurement behind it).
int GenderDefault;       // 0x009BBFEC
BOOL PluralityDefault;   // 0x009BBFF0

/*
Purpose: Divide, rounding the quotient AWAY from zero when there is a
         remainder, and report both.

             mov eax,ecx / cdq / idiv esi / mov edx,eax / imul edx,esi
             sub ecx,edx / mov [edx],ecx / je zero / inc eax / mov [ecx],eax
             ret 0x10   zero: mov [edx],eax / ret 0x10

         The `je` is three instructions after the `sub` that sets it, with two
         `mov`s and a `pop` in between - none of which touch flags. It tests
         whether the REMAINDER was zero, not anything nearer to it.

         `idiv` truncates toward zero, so the +1 rounds away from zero rather
         than up: -7 by 2 gives a remainder of -1 and a quotient of -3, which
         becomes -2. Both traps of the original are kept, because both
         spellings go through one division: a zero divisor and INT_MIN / -1.

// HISTORICAL scaffold role: SCAFFOLD Transitional leaf collection, not completed genuine owner recovery; preserve body evidence during owner migration.
// ORIGINAL: 0x00532A50 sub_532a50 0x00532A50-0x00532A81 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned terranx_original.exe SHA-256 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf at 0x00532A50: decoded complete span accesses only explicit stack arguments, without an ECX receiver or object storage.", "Source-owned general utility implementation; current source family placement does not establish the original translation unit.", "Whole file-backed PE examination found no HIGHLOW operand or raw E8-relative target for this entry; indirect calls and non-relocated references remain unresolved."], "reason": "Own the primitive-only operation as a complete source-level general utility. No class, global, object layout, or original source-TU identity is claimed."}
// symbol    ?divide_and_increment_quotient_if_remainder@@YGHHPAH0H@Z
// historical original marker: 0x00532A50 sub_532a50 0x00532A50-0x00532A81 BYTE_EXACT
// historical scaffold emitted-symbol hint: ?leaf_00532a50_redirect@@YGHHPAH0H@Z
// size      49 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the quotient that was stored
Status: Complete
*/
// HISTORICAL correction: the former prose called this away-from-zero; that label was inaccurate.
// Preserve the exact operation below: truncating IDIV quotient, remainder store, then quotient increment on any nonzero remainder.


int __stdcall divide_and_increment_quotient_if_remainder(int value, int *quotient_out,
                                     int *remainder_out, int divisor) {
    // Increment the quotient IN PLACE. A `rounded` temp - whether built by a
    // ternary or by an `if` - keeps one more value live, and VC6 then stops
    // spilling the divisor to esi, which is the `push` at index 3.
    int quotient = value / divisor;
    const int remainder = value - quotient * divisor;
    *remainder_out = remainder;
    if (remainder != 0) {
        ++quotient;
    }
    *quotient_out = quotient;
    return quotient;
}

/*
Purpose: Is a point inside an inclusive rectangle?

             mov eax,[ebp+8] / mov ecx,[ebp+0x10] / cmp eax,ecx / jl no
             cmp eax,[ebp+0x18] / jg no / mov eax,[ebp+0xc]
             mov ecx,[ebp+0x14] / cmp eax,ecx / jl no / cmp eax,[ebp+0x1c]
             jg no / mov eax,1 / ret     no: xor eax,eax / ret

         Six arguments in the order x, y, left, top, right, bottom - the two
         coordinates first and the bounds interleaved after, which is not the
         (left, top, right, bottom, x, y) an eye expects.

         `jl` and `jg` are SIGNED, and both bounds are INCLUSIVE: a point
         exactly on an edge is inside.

// HISTORICAL scaffold role: SCAFFOLD Transitional leaf collection, not completed genuine owner recovery; preserve body evidence during owner migration.
// ORIGINAL: 0x00592DB0 sub_592db0 0x00592DB0-0x00592DDC BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned terranx_original.exe SHA-256 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf at 0x00592DB0: decoded complete span accesses only explicit stack arguments, without an ECX receiver or object storage.", "Source-owned general utility implementation; current source family placement does not establish the original translation unit.", "Whole file-backed PE examination found no HIGHLOW operand or raw E8-relative target for this entry; indirect calls and non-relocated references remain unresolved."], "reason": "Own the primitive-only operation as a complete source-level general utility. No class, global, object layout, or original source-TU identity is claimed."}
// symbol    ?point_in_inclusive_rect@@YAHHHHHHH@Z
// historical original marker: 0x00592DB0 sub_592db0 0x00592DB0-0x00592DDC BYTE_EXACT
// historical scaffold emitted-symbol hint: ?leaf_00592db0_redirect@@YAHHHHHHH@Z
// size      44 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 1 when inside, 0 otherwise
Status: Complete
*/

int __cdecl point_in_inclusive_rect(int x, int y, int left, int top,
                                   int right, int bottom) {
    if (x < left || x > right) {
        return 0;
    }
    if (y < top || y > bottom) {
        return 0;
    }
    return 1;
}


// Read a script section into the shared substitution buffer. The text
// reader owns TextBufferGetPtr; parse_string supplies the substitutions.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0048C060 ?parse_it@@YAXPADPBD@Z 0x0048C060-0x0048C099 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned48C060..48C099 clears StringTemp9B86A0, calls X_text_open5BECA0, then text_get5FD570 and parse_string625880(TextBufferGetPtr9B7D00,StringTemp) on success.", "src/recovered/units/0048c060.cpp retains this same original identity, complete57-byte span and historical BYTE_EXACT claim; Quayle redraw4956D0 passes Holobook filename and BOOKTITLE/BOOKINTRO/BOOKLIST section names."], "reason": "Recover the filename/section wrapper using the real shared text and substitution owners; the existing seed retains its historical matching obligation."}
void __cdecl parse_it(char *file_name, const char *section) {
    StringTemp[0] = 0;
    if (X_text_open(file_name, section) == 0) {
        text_get();
        parse_string(TextBufferGetPtr, StringTemp);
    }
}

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
// ORIGINAL: 0x00625E50 ?parse_say@@YAHHHHH@Z 0x00625E50-0x00625EB3 BYTE_EXACT
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
int __cdecl parse_say(int id, int input, int gender, int pluralality) {
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
// The digit bounds, read from MEMORY: the image's .rdata carries the literal
// "09" at 0x670C1C and both comparisons load through it. The literal used to
// be reached as `char *const digits = (char *)0x00670C1C`, naming storage
// inside terranx.exe - unmapped here, so the body read garbage. This array is
// that storage: the image's own bytes ('0', '9', NUL), and the loads fold to
// the same `byte ptr [disp32]` form, the displacement relocated.
char findnum_digits[] = "09";  // 0x00670C1C

char *__cdecl findnum(char *str) {
    if (!str) {
        return 0;
    }
    char *const digits = findnum_digits;
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
            save_daemon(const_cast<char *>("saves\\auto\\Alpha Centauri Autosave 1"));
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
            save_daemon(const_cast<char *>("saves\\auto\\Alpha Centauri Autosave 1"));
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
    if (!(GameDataStateStorage.fields_.game_preferences_ & PREF_BSC_AUTOSAVE_EACH_TURN)) {
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
// TRIED (2026-09-03): dropping this body's `int bounds = count - 1` local
//   and writing `i < count - 1` inline, because the image reloads `count`
//   from its argument slot at the top of every pass iteration
//   (`mov ecx, [ebp+8]` at 0x005B56A9, then `dec ecx`) and so cannot have a
//   `bounds` variable. Measured 5/47 before and after - the compiler keeps
//   `count - 1` in a register either way - so the inline spelling is kept
//   only because it stops declaring a local the image has not got; the
//   unreproduced shape is the memory-homed `i` (`push ecx` slot at [ebp-4],
//   written and re-read around the swap) plus the running pointers
//   `esi`/`edi`/`ebx`, which is a strength-reduction choice, not a source
//   one. Left at the plateau.
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
    // BUG IN THE ORIGINAL (reproduced): `test eax,eax` right after every
    // swap jumps straight back to the function's own top (0x5b5697) the
    // moment a swap happens, abandoning the rest of this pass instead of
    // finishing it. A correct bubble sort would keep scanning to the end of
    // the pass; this one restarts from i=0 after its FIRST swap each pass,
    // which is why there is no `has_swapped` flag left to track here - the
    // loop completing on its own IS the "nothing left to swap" signal.
restart:
    for (int i = 0; i < count - 1; i++) {
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

// ---------------------------------------------------------------------------
// The Jackal engine's bring-up, promoted out of src/unrecovered/0062d3a0.cpp
// on 2026-08-15. WinMain calls it once; everything the game draws with exists
// because this returned zero.
// ---------------------------------------------------------------------------

// 0x009B7AF4 is WinDynamicDefaults.default_font_ (win.h) - the store below
// seeds it from FontDefault (0x009BB484), and config_popups later re-points
// it via find_font; the two readers are the Win constructor and Win::close,
// both of which move it into field_F8_.

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
    result = Sprite::init_class();
    if (result != 0) {
        return result;
    }

    // THE DEFAULT WINDOW FONT (win.h's WinDynamicDefaults.default_font_,
    // 0x009B7AF4): every Win the game opens takes its field_F8_ font from
    // this slot until config_popups re-points it at the popup 12px font.
    // It used to be a second object named jackal_unk_9b7af4 - a duplicate
    // of the defaults table's own storage - so the font never flowed.
    WinDynamicDefaults.default_font_ = FontDefault;
    trig_init();
    result = Cursor::init_cursor_class();
    if (result != 0) {
        return result;
    }

    if (StringTable.init(0x8000) != 0) {
        return 1;
    }

    result = Time::init_class();
    if (result != 0) {
        return result;
    }

    JackalInitFlags |= 1;
    return 0;
}

// SDK imports, never raw IAT pointers.
// Shared IntersectRect import identity is owned by graphicwin.cpp.
// RECOVERY_IMPORT: v1 __imp__EqualRect@8 USER32.dll name:EqualRect
// RECOVERY_IMPORT: v1 __imp__DirectDrawEnumerateA@8 DDRAW.dll name:DirectDrawEnumerateA
// RECOVERY_IMPORT: v1 __imp__DirectDrawCreate@12 DDRAW.dll name:DirectDrawCreate
// SDK dxguid-owned identities already consumed by UV2Player and MediaRuntime:
// IID_IDirectDrawSurface2 = 57805885-6eec-11cf-9441-a82303c10e27 (66FC28);
// IID_IDirectDraw2 = b3a6f3e0-2b43-11cf-a2de-00aa00b93356 (66FC38).
// No new GUID definitions or fixed-address aliases are introduced.

// Shared display owner, proposed in general.cpp beside its teardown.
// All newly owned storage is native zero-fill; no explicit initialization fixes.
// RECOVERY_OBJECT: v2 0x009BC160 0x009BC2CC ?DirectDrawEmulationCaps@@3U_DDCAPS@@A 0x0062CD50
DDCAPS DirectDrawEmulationCaps;
// RECOVERY_OBJECT: v2 0x009BC2CC 0x009BC2D0 ?DirectDrawDeviceCount@@3HA 0x0062C860
int DirectDrawDeviceCount;
// A provisional ten-slot source capacity, covering the access window before
// the independently identified caps object. Adjacency does NOT prove an
// original allocation extent or bound. The callback has NO bounds check;
// overflow effects and the original capacity remain restoration obligations.
// RECOVERY_OBJECT: v1 0x009BC2E0 ?DirectDrawDeviceGuids@@3PAPAU_GUID@@A
GUID *DirectDrawDeviceGuids[10];
// RECOVERY_OBJECT: v2 0x009BC308 0x009BC474 ?DirectDrawDriverCaps@@3U_DDCAPS@@A 0x0062CD50
DDCAPS DirectDrawDriverCaps;
// RECOVERY_OBJECT: v2 0x009BC478 0x009BC47C ?DDActive@@3HA 0x0062CD50
int DDActive;
// RECOVERY_OBJECT: v2 0x009BC484 0x009BC488 ?DirectDrawDensityLimit@@3KA 0x0062CD10
DWORD DirectDrawDensityLimit;
// Separate from WinSharedHdc9B7B2C despite sharing WinHdcRefCount9B3AB0.
// RECOVERY_OBJECT: v2 0x009BC48C 0x009BC490 ?DirectDrawHdc@@3PAUHDC__@@A 0x0062D160
HDC DirectDrawHdc;
// RECOVERY_OBJECT: v2 0x009BC4A4 0x009BC4A8 ?DirectDrawClipper@@3PAUIDirectDrawClipper@@A 0x0062CD50
IDirectDrawClipper *DirectDrawClipper;
// RECOVERY_OBJECT: v2 0x009BC4A8 0x009BC4AC ?DirectDrawSelectionCancelled@@3HA 0x0062C8E0
int DirectDrawSelectionCancelled;

static_assert(sizeof(DDCAPS) == 0x16C,
              "shared display caps require the pinned SDK DirectDraw5 layout");
static_assert(sizeof(DDSURFACEDESC) == 0x6C,
              "shared display description must retain the native layout");

// Typed product bodies. Mapping records are generated separately by the native
// source writer from the complete existing seed/owner records.

// ORIGINAL: 0x0062C860 Callback 0x0062C860-0x0062C8D7
// RECOVERY_BINDING: v1 {"evidence": ["Pinned terranx_original.exe SHA01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:62C860..62D38E", "62CE7F incoming ESP+90 after net ESP-8C proves bit_depth argument, not uninitialized EDI", "62CD70/62D118 and62D0D6 write ScreenBuffer+58/+5C; these are members, not DDCached globals", "SDK DDRAW.H Draw2/Surface2 IIDs and genuine slot signatures;62D297/62D2A9 are Clipper::SetClipList/Surface2::SetClipper", "Original selector cold lifetime span663340..663494 retained through real automatic BasePop", "62D160/62D1A0 use separate HDC9BC48C but shared WinHdcRefCount9B3AB0;62D2C0 retains final HRESULT"], "reason": "Recover shared DirectDraw selection/init/DC/clip lifetime as a complete typed owner in general.cpp, reuse actual Buffer/Win/Palette storage, and retain all original claims, spans and historical seed findings. Gamma adjacency routing is not ownership proof. No byte, linked-preservation or runtime acceptance is claimed."}
// symbol    ?direct_draw_device_callback@@YGHPAU_GUID@@PAD1PAX@Z
// kind game
BOOL CALLBACK direct_draw_device_callback(GUID *guid, LPSTR description,
                                         LPSTR /*driver_name*/, LPVOID context) {
    BasePop *popup = static_cast<BasePop *>(context);
    popup->dialogs().item(description, DirectDrawDeviceCount);
    if (guid == 0) {
        DirectDrawDeviceGuids[DirectDrawDeviceCount] = 0;
        ++DirectDrawDeviceCount;
        return 1;
    }
    GUID *allocated = static_cast<GUID *>(mem_get(16));
    DirectDrawDeviceGuids[DirectDrawDeviceCount] = allocated;
    if (allocated != 0) {
        ++DirectDrawDeviceCount;
        memcpy(allocated, guid, 16);
    }
    return 1;
}

// ORIGINAL: 0x0062C8E0 sub_62c8e0 0x0062C8E0-0x0062CD02;0x00663340-0x00663494
// symbol    ?select_direct_draw_device@@YAPAU_GUID@@XZ
// TRIED: a giant (0x3234-byte) BasePop+Spot+Dialogs(embeds Dialog)+two list entries+Sprite+two FlatButtons+Heap local, driven by direct calls at disassembly offsets into a raw `unsigned char buf[]` rather than as real RAII C++ locals - the three tails' explicit vtable-patch-then-close()/dtor sequences (e.g. writing g_006698d4/g_006698cc into the BasePop object right before close()) are themselves that literal, so the raw-offset transcription matches the disassembly's own shape. Did NOT attempt real nested locals of type BasePop/Dialogs/etc: the emitted `Dialogs` class embeds `GraphicWin virtual_base_` at 0x188, but the disassembly calls `??1Dialogs@@QAE@XZ` and `??1GraphicWin@@QAE@XZ` on the IDENTICAL address twice running, which only matches non-virtual `Dialogs : public GraphicWin` (base at offset 0) - the two class shapes disagree, so a real local risked wrong offsets on top of not getting the SEH frame either. MISMATCH #0 (`mov` vs `push`, no SEH frame at all is generated without a real object of non-trivial type as a local) - the whole-function control flow (three-way branch on BasePop::exec()'s result, matching Ghidra's own decompilation) and every call's argument order (BasePop::init/exec, DirectDrawEnumerateA, the two sub_4066c0 calls whose result feeds g_009b3374) is re-derived from the push order, not chased to byte-exactness.
// kind game
GUID *__cdecl select_direct_draw_device() {
    BasePop popup;
    popup.init(0x42, 0);
    DirectDrawDeviceCount = 0;
    DirectDrawEnumerateA(direct_draw_device_callback, &popup);
    const int selected = popup.exec(0, 0);
    if (selected == -1) {
        DirectDrawSelectionCancelled = 1;
        return 0;
    }
    if (selected >= 0 && selected < DirectDrawDeviceCount) {
        return DirectDrawDeviceGuids[selected];
    }
    return 0;
}

// ORIGINAL: 0x0062CD10 sub_62cd10 0x0062CD10-0x0062CD48 BYTE_EXACT
// symbol    ?sub_62cd10@@YGJPAU_DDSURFACEDESC@@PAX@Z
// kind game
HRESULT CALLBACK sub_62cd10(LPDDSURFACEDESC description, LPVOID context) {
    const DWORD density = (description->dwBackBufferCount + 1u) *
        static_cast<DWORD>(description->lPitch) * description->dwHeight * 2u;
    if (density < DirectDrawDensityLimit) {
        const DWORD width = description->dwWidth;
        if (width <= 0x400u) {
            DWORD *selected_width = static_cast<DWORD *>(context);
            if (width > *selected_width) {
                *selected_width = width;
            }
        }
    }
    return 1;
}

// ORIGINAL: 0x0062CD50 sub_62cd50 0x0062CD50-0x0062D0ED
// symbol    ?sub_62cd50@@YAHH@Z
// TRIED: nullary/int-arg per the contract/IDA guess - no stack param reads exist, so kept nullary; DirectDraw init sequence with slots {0,2,4,5,6,8,11,12,20,21,31} matches the real IDirectDraw2/IDirectDrawSurface vtable orders exactly (QueryInterface/Release/CreateClipper/CreatePalette/ CreateSurface/EnumDisplayModes/GetCaps/GetDisplayMode/ SetCooperativeLevel/SetDisplayMode/SetPalette), confirming the derived arg counts per push-site. One value ([esp+0x90], Ghidra's unaff_EDI) reads stack memory past this function's own frame in the original and is not reproducible in C; replaced with a zero-initialized local. Frame layout differs from instruction #0 given the size/local count; not pursued further.
// kind game
int __cdecl sub_62cd50(int bit_depth) {
    if (BufferDirectDraw != 0) {
        BufferDirectDraw->Release();
        BufferDirectDraw = 0;
    }
    ScreenBuffer.surface_ = 0;
    ScreenBuffer.clipper_ = 0;
    DirectDrawSurface = 0;
    DirectDrawBackBuffer = 0;
    DDActive = 0;
    DirectDrawSelectionCancelled = 0;
    GUID *selected = select_direct_draw_device();
    if (DirectDrawSelectionCancelled != 0) {
        return 0;
    }

    IDirectDraw *created;
    if (DirectDrawCreate(selected, &created, 0) != 0) {
        return 0x12;
    }
    if (created->QueryInterface(IID_IDirectDraw2,
                               reinterpret_cast<void **>(&BufferDirectDraw)) != 0) {
        created->Release();
        return 0x12;
    }
    created->Release();

    // One reused native DDSURFACEDESC, not three zero-filled descriptors.
    DDSURFACEDESC description;
    description.dwSize = 0x6C;
    if (BufferDirectDraw->GetDisplayMode(&description) != 0) {
        goto failure;
    }
    DirectDrawDriverCaps.dwSize = 0x16C;
    DirectDrawEmulationCaps.dwSize = 0x16C;
    if (BufferDirectDraw->GetCaps(&DirectDrawDriverCaps,
                                 &DirectDrawEmulationCaps) != 0) {
        goto failure;
    }
    DirectDrawDensityLimit = (description.dwBackBufferCount + 1u) *
        static_cast<DWORD>(description.lPitch) * description.dwHeight +
        DirectDrawDriverCaps.dwVidMemFree;

    {
        DWORD selected_width = 0;
        description.dwSize = 0x6C;
        description.dwFlags = 0x1000;
        description.ddpfPixelFormat.dwSize = 0x20;
        description.ddpfPixelFormat.dwFlags = 0x40;
        description.ddpfPixelFormat.dwRGBBitCount = bit_depth;
        if (BufferDirectDraw->EnumDisplayModes(0, &description, &selected_width,
                                              sub_62cd10) != 0) {
            goto failure;
        }
        if (selected_width == 0) {
            goto failure;
        }
        if (BufferDirectDraw->SetCooperativeLevel(HandleMain, 0x13) != 0) {
            goto failure;
        }
        // VC6's signed (width * 3) / 4, even if a callback supplied a
        // width outside the ordinary 0..1024 callback range.
        const int product = static_cast<int>(selected_width * 3u);
        if (BufferDirectDraw->SetDisplayMode(selected_width, product / 4,
                                            bit_depth, 0, 0) != 0) {
            goto failure;
        }
        // Reload selected_width after SetDisplayMode: callbacks may change it.
        DirectDrawClipRect.right = selected_width;
        DirectDrawClipRect.left = 0;
        DirectDrawClipRect.top = 0;
        DirectDrawClipRect.bottom =
            static_cast<int>(selected_width * 3u) / 4;

        IDirectDrawSurface *primary;
        IDirectDrawSurface *backbuffer;
        description.dwSize = 0x6C;
        description.dwFlags = 0x21;
        description.ddsCaps.dwCaps = 0x4218;
        description.dwBackBufferCount = 1;
        if (BufferDirectDraw->CreateSurface(&description, &primary, 0) != 0) {
            goto failure;
        }
        DDSCAPS backbuffer_caps;
        backbuffer_caps.dwCaps = 4;
        if (primary->GetAttachedSurface(&backbuffer_caps, &backbuffer) != 0) {
            goto failure;
        }
        primary->QueryInterface(IID_IDirectDrawSurface2,
                               reinterpret_cast<void **>(&DirectDrawSurface));
        backbuffer->QueryInterface(IID_IDirectDrawSurface2,
                                  reinterpret_cast<void **>(&DirectDrawBackBuffer));
        if (bit_depth == 8 &&
            BufferDirectDraw->CreatePalette(
                // Native forwards the Palette's proven entries_ prefix (+0),
                // including a null PaletteActive; no null-this accessor call.
                4, reinterpret_cast<PALETTEENTRY *>(PaletteActive),
                &DirectDrawPalette, 0) != 0) {
            goto failure;
        }
        DirectDrawSurface->SetPalette(DirectDrawPalette);
        DirectDrawBackBuffer->SetPalette(DirectDrawPalette);
        if (BufferDirectDraw->CreateClipper(0, &DirectDrawClipper, 0) != 0) {
            goto failure;
        }
        DDActive = 1;
        ScreenBuffer.close();
        ScreenBuffer.locked_bits_ = 0;
        ScreenBuffer.hdc2_ = 0;
        // Buffer's existing API uses the Surface1 prefix of the Surface2
        // interface, just as UV2Player's ExtDirectDraw bridge already does.
        ScreenBuffer.surface_ =
            reinterpret_cast<IDirectDrawSurface *>(DirectDrawBackBuffer);
        ScreenBuffer.clipper_ = DirectDrawClipper;
    }
    return 0;

failure:
    BufferDirectDraw->Release();
    BufferDirectDraw = 0;
    return 0x12;
}

// ORIGINAL: 0x0062D0F0 sub_62d0f0 0x0062D0F0-0x0062D0F8 BYTE_EXACT
// symbol    ?sub_62d0f0@@YAHXZ
// TRIED: homed into general.cpp 2026-09-03 - does not link: the body's only call
// kind game
int __cdecl sub_62d0f0() {
    select_direct_draw_device();
    return 0;
}

// ORIGINAL: 0x0062D100 sub_62d100 0x0062D100-0x0062D13A BYTE_EXACT
// symbol    _sub_62d100
// kind game
extern "C" int __cdecl sub_62d100() {
    if (BufferDirectDraw != 0) {
        BufferDirectDraw->Release();
        BufferDirectDraw = 0;
    }
    ScreenBuffer.surface_ = 0;
    ScreenBuffer.clipper_ = 0;
    DirectDrawSurface = 0;
    DirectDrawBackBuffer = 0;
    DDActive = 0;
    return 0;
}

// ORIGINAL: 0x0062D160 sub_62d160 0x0062D160-0x0062D191
// symbol    ?sub_62d160@@YAPAUHDC__@@XZ
// kind game
HDC __cdecl sub_62d160() {
    HDC handle = DirectDrawHdc;
    WinHdcRefCount = static_cast<int>(static_cast<DWORD>(WinHdcRefCount) + 1u);
    IDirectDrawSurface2 *surface = DirectDrawSurface;
    if (surface != 0 && handle == 0) {
        surface->GetDC(&DirectDrawHdc);
        handle = DirectDrawHdc;
    }
    return handle;
}

// ORIGINAL: 0x0062D1A0 sub_62d1a0 0x0062D1A0-0x0062D1D2 BYTE_EXACT
// symbol    ?sub_62d1a0@@YAXXZ
// TRIED: homed into general.cpp 2026-09-03 - does not link: the body reads and
// kind game
void __cdecl sub_62d1a0() {
    const int count = WinHdcRefCount;
    IDirectDrawSurface2 *surface = DirectDrawSurface;
    WinHdcRefCount = static_cast<int>(static_cast<DWORD>(count) - 1u);
    if (surface != 0) {
        HDC handle = DirectDrawHdc;
        if (handle != 0) {
            surface->ReleaseDC(handle);
            DirectDrawHdc = 0;
        }
    }
}

// RGNDATA has a variable-sized trailing byte buffer. This owns the exact
// native header and its one RECT without pretending Buffer[1] is large enough.
struct DirectDrawClipRegion {
    RGNDATAHEADER header;
    RECT rectangle;
};

// ORIGINAL: 0x0062D1E0 sub_62d1e0 0x0062D1E0-0x0062D2B1
// symbol    ?sub_62d1e0@@YAXPAUtagRECT@@@Z
// TRIED: 209 vs 204 bytes; struct-literal field order for the 0x30-byte DrawRegion local drifts the compiler's address-of scheduling (lea moves relative to the field stores) versus the original's register reuse. Tried plain field assigns and an early `DrawRegion *preg = &region;` alias; neither matched.
// kind game
void __cdecl sub_62d1e0(RECT *clip_rect) {
    if (DirectDrawBackBuffer == 0 || clip_rect == 0) {
        return;
    }
    RECT intersection;
    if (!IntersectRect(&intersection, &DirectDrawClipRect, clip_rect)) {
        return;
    }
    if (EqualRect(&DirectDrawClipRect, &intersection)) {
        DirectDrawBackBuffer->SetClipper(0);
        return;
    }
    DirectDrawClipRegion region;
    region.header.dwSize = 0x20;
    region.header.iType = 1;
    region.header.nCount = 1;
    region.header.nRgnSize = 0x10;
    region.header.rcBound = intersection;
    region.rectangle = intersection;
    DirectDrawClipper->SetClipList(reinterpret_cast<RGNDATA *>(&region), 0);
    // Both globals are reloaded after SetClipList, not cached before it.
    DirectDrawBackBuffer->SetClipper(DirectDrawClipper);
}

// ORIGINAL: 0x0062D2C0 sub_62d2c0 0x0062D2C0-0x0062D38E
// symbol    ?sub_62d2c0@@YAHXZ
// TRIED: homed into general.cpp (its real TU) 2026-09-03 and reverted for one reason:
// TRIED: IntersectRect/EqualRect are called through the fixed IAT-style slots (0x669338/0x6692bc) with `this` pushed as an explicit stdcall arg, not via C++ virtual dispatch - modelled with raw `PFN_*` casts. Every early-out in the original shares ONE epilogue (add esp,0x40; ret); separate `if (x) return 0;` statements each get their own inline return instead of jumping to a shared tail, which is most of the 0.82 mnemonic-similarity gap.
// kind game
int __cdecl sub_62d2c0() {
    if (DirectDrawBackBuffer == 0) {
        return 0;
    }
    RECT intersection;
    if (!IntersectRect(&intersection, &DirectDrawClipRect, &DirectDrawClipRect)) {
        return 0;
    }
    if (EqualRect(&DirectDrawClipRect, &intersection)) {
        return DirectDrawBackBuffer->SetClipper(0);
    }
    DirectDrawClipRegion region;
    region.header.dwSize = 0x20;
    region.header.iType = 1;
    region.header.nCount = 1;
    region.header.nRgnSize = 0x10;
    region.header.rcBound = intersection;
    region.rectangle = intersection;
    DirectDrawClipper->SetClipList(reinterpret_cast<RGNDATA *>(&region), 0);
    return DirectDrawBackBuffer->SetClipper(DirectDrawClipper);
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
Purpose: close_class - close the blank sprite (0x009BEAE8 in the image, the
         real object g_BLANK_SPRITE here), unguarded; the caller
         (jackal_close) tail-calls it as the sprite's teardown stage. Named
         Sprite::close_class 2026-08-30 as the mirror of Sprite::init_class
         (0x0063CE20) - the catalogue carries no name for either.
// ORIGINAL: 0x0063CEF0 sub_63cef0 0x0063CEF0-0x0063CEFA BYTE_EXACT
// symbol    ?close_class@Sprite@@SAXXZ
// size      10 bytes
// prototype
// callers   1   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Sprite::close_class() {
    blank_sprite().close();
}

void __cdecl jackal_close() {
    sub_62d100();
    Time::close_class();
    WinDisplayInit.teardown();
    CheckButton::close_class();
    buffer_close_class_redirect();
    teardown_0060fd60();
    teardown_0060e5d0();
    Font::close_font_class();
    Sprite::close_class();
    FileWin::close_class();
    BasePop::close_class();
    Cursor::close_cursor_class();
    Win::close_class();
    Palette::close_palette_class();
    StringTable.shutdown();
    JackalInitFlags &= ~1;
}

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
    for (int sample = 0; sample < 256; sample++) {
        SinTable[sample] = static_cast<int>(
            sin(static_cast<double>(sample) * 0.006159985596078431) * 65535.0);
    }
    sin(0x40000000, 100);
    return sin(static_cast<int>(0xe02d82d7), 100);
}

// ORIGINAL: 0x00538EC0 ??1JackalVoiceRx@@QAE@XZ 0x00538EC0-0x00538F2E FILE BYTE_EXACT
// kind      game
// symbol    ?clear@JackalVoiceRx@@QAEXXZ
// The catalogue destructor label names this shared list-clearing helper.
// Callers stage receiver/base tables and allocation-owner handoff outside it
// (0x00538E5A..0x00538E9E); the complete receiver destructor is separate
// (0x004E3660). Keep the original identity, not the skeleton's class fiction.
// Insertion 0x00633180 establishes distinct 0x1C circular entries and 0x60
// voice payloads. The three virtual calls below are respectively a receiver
// hook, Sound-family destruction, and entry virtual-base destruction.
// This does not complete Net/AlphaNet/NetDaemon lifetime recovery.
void JackalVoiceRx::clear() {
    if (head_) {
        for (int removed = 0; removed < count_; ++removed) {
            cursor_ = head_->next_;
            Sound *payload = head_->payload_;
            on_remove(payload);
            delete payload;
            head_->payload_ = 0;
            delete head_;
            head_ = cursor_;
        }
        head_ = 0;
        cursor_index_ = 0;
        count_ = 0;
    }
}

// ORIGINAL: 0x005D271D sub_5d271d 0x005D271D-0x005D2761 FILE
// size      68 bytes
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x005D271D
// measured tier  MISMATCH
// divergence     1
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005d271d/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_5d271d  at 0x005D271D  (68 bytes)
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

// Each source dword is 4 bytes b0..b3 (little-endian); each byte is
// written out twice adjacently to the destination (2x horizontal pixel
// doubling: b0,b0,b1,b1,b2,b2,b3,b3), 4 source bytes producing 8
// destination bytes per iteration.
//
// The original assembles each destination dword in a register via
// 8/16-bit sub-register moves (mov bl,ah / mov bh,ah / shl ebx,0x10 /
// mov bl,al / mov bh,al) before a single 32-bit store. Every arithmetic
// rendering tried here (shift-or splat, multiply-by-0x101 splat) makes
// VC6 emit `and`/`imul`/extra `shl`+`or` chains instead of reproducing
// that register shuffle, so this settles for the byte-store form below,
// which is shorter and closer in instruction count than the arithmetic
// attempts (26 mnemonics / 0.73 similarity, vs 34-43 / 0.58-0.6 for the
// splat versions) even though it stores 8 bytes individually rather
// than 2 dwords.
extern "C" void __cdecl sub_5d271d(unsigned int *src, unsigned char *dst,
                                    int count) {
    do {
        unsigned int word = *src;
        ++src;
        unsigned char b0 = static_cast<unsigned char>(word);
        unsigned char b1 = static_cast<unsigned char>(word >> 8);
        dst[0] = b0; dst[1] = b0; dst[2] = b1; dst[3] = b1;
        word >>= 16;
        unsigned char b2 = static_cast<unsigned char>(word);
        unsigned char b3 = static_cast<unsigned char>(word >> 8);
        dst[4] = b2; dst[5] = b2; dst[6] = b3; dst[7] = b3;
        dst += 8;
        count -= 4;
    } while (count != 0);
}

// ORIGINAL: 0x005D2B20 sub_5d2b20 0x005D2B20-0x005D2C0A FILE
// TRIED: BYTE_EXACT is likely structurally unreachable here without `__asm` (banned). The disassembly reads `esi` as an 8-int input array from instruction #1 (`mov ebx,[esi+0x14]`) and NEVER writes esi before that first read anywhere in the function - Ghidra names it `unaff_ESI` for exactly this reason. That means the real calling convention hands this function its input array already sitting in esi, which none of __cdecl/__stdcall/__fastcall/__thiscall can express - every standard convention either uses the stack or ecx/edx for the first argument, never esi with zero setup. This is plausible as a compiler-chosen convention specific to this internal, 2-caller helper (proven safe only in the context of its real callers, which this recovery unit does not see), but nothing in valid VC6 C++ can request it. Landed body takes the input array as a normal second parameter instead (MISMATCH from the very first instruction, since the prologue differs immediately) so the arithmetic shape is on record: an 8-point butterfly (pairwise sum/difference of src[0],src[2],src[4], src[6] and separately src[1],src[3],src[5],src[7]) combining one `__int64` fixed-point multiply-by-IdctSin45Q31 (the `imul dword ptr [addr]` high-dword-times-2 pattern) with float arithmetic against IdctInv2CosPi8/IdctInv2Cos3Pi8/IdctCos3Pi8, each float result added to IdctAanScale and reduced to an int via `fstp qword` followed by reading only the low dword - this looks like the classic "magic-constant" double-to-int rounding idiom (add a bias, read the mantissa's low bits) but a plain `(int)(double)expr` cast was NOT enough to reproduce it; the exact bias/rounding trick was not reverse-engineered.
// size      234 bytes
// prototype 
// callers   2   call targets   0
// kind      
// flags     
// calls     (none)
// working copy - scaffold materialised by --work

// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const IdctSin45Q31 = (int *)0x009C9594;
static int *const IdctInv2CosPi8 = (int *)0x009C9598;
static int *const IdctInv2Cos3Pi8 = (int *)0x009C959C;
static int *const IdctCos3Pi8 = (int *)0x009C95A0;
static int *const IdctAanScale = (int *)0x009C95A4;
extern "C" void __cdecl sub_5d2b20(int *dst, int *src) {
    int iVar9 = src[3] + src[5];
    int iVar8 = src[7] + src[1];
    float fVar2 = (float)(src[5] - src[3]);
    float fVar3 = (float)(src[1] - src[7]);
    float fVar1 = (fVar3 + fVar2) * *(float *)IdctCos3Pi8;
    int iVar6 = (int)(((__int64)(iVar8 - iVar9) * (__int64)*IdctSin45Q31) >> 32) * 2;
    int iStack8 = (int)(double)(fVar2 * *(float *)IdctInv2CosPi8 + fVar1 + *(float *)IdctAanScale);
    int iStack10 = (int)(double)((fVar3 * *(float *)IdctInv2Cos3Pi8 - fVar1) + *(float *)IdctAanScale);
    int iVar4 = iStack8 + iVar6;
    iVar6 = iVar6 + iStack10;
    iStack10 = iStack10 + iVar9 + iVar8;
    iVar9 = src[4] + src[0];
    int iVar5 = src[0] - src[4];
    int iVar7 = (int)(((__int64)(src[2] - src[6]) * (__int64)*IdctSin45Q31) >> 32);
    iVar8 = iVar7 * 2;
    int iVar10 = src[6] + src[2] + iVar8;
    iVar8 = iVar8 + iVar5;
    iVar5 = iVar5 + iVar7 * -2;
    iVar7 = iVar10 + iVar9;
    iVar9 = iVar9 - iVar10;
    dst[2] = iVar5 + iVar4;
    dst[5] = iVar5 - iVar4;
    dst[3] = iVar9 + iStack8;
    dst[4] = iVar9 - iStack8;
    dst[0] = iVar7 + iStack10;
    dst[1] = iVar6 + iVar8;
    dst[6] = iVar8 - iVar6;
    dst[7] = iVar7 - iStack10;
}

// ORIGINAL: 0x005D2C0A sub_5d2c0a 0x005D2C0A-0x005D2D13
// TRIED: BYTE_EXACT is structurally unreachable here without `__asm` (banned), same finding as the sibling 8-point butterfly at 0x005D2B20 (see the sibling 0x005D2B20 body (homed here)): the disassembly reads `esi` (src) from the very first real instruction and `edi` (dst, saved via `push edi` at entry and reloaded from the stack near the end) with no stack setup for either, which none of __cdecl/__stdcall/__fastcall/__thiscall can request. Landed as `sub_5d2c0a(int *dst, int *src)` (MISMATCH from instruction 0, the prologue) so the arithmetic is on record: identical sum/difference/`__int64` fixed-point-multiply butterfly to the sibling, but every combined pair is additionally scaled by `>> 17` before it is stored, which the sibling's version does not do.
// size      265 bytes
// prototype
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)

extern "C" void __cdecl sub_5d2c0a(int *dst, int *src) {
    int iVar9 = src[3] + src[5];
    int iVar8 = src[7] + src[1];
    float fVar2 = (float)(src[5] - src[3]);
    float fVar3 = (float)(src[1] - src[7]);
    float fVar1 = (fVar3 + fVar2) * *(float *)IdctCos3Pi8;
    int iVar6 = (int)(((__int64)(iVar8 - iVar9) * (__int64)*IdctSin45Q31) >> 32) * 2;
    int local_1c = (int)(double)(fVar2 * *(float *)IdctInv2CosPi8 + fVar1 + *(float *)IdctAanScale);
    int local_14 = (int)(double)((fVar3 * *(float *)IdctInv2Cos3Pi8 - fVar1) + *(float *)IdctAanScale);
    int iVar4 = local_1c + iVar6;
    iVar6 = iVar6 + local_14;
    local_14 = local_14 + iVar9 + iVar8;
    iVar9 = src[4] + src[0];
    int iVar5 = src[0] - src[4];
    int iVar7 = (int)(((__int64)(src[2] - src[6]) * (__int64)*IdctSin45Q31) >> 32);
    iVar8 = iVar7 * 2;
    int iVar10 = src[6] + src[2] + iVar8;
    iVar8 = iVar8 + iVar5;
    iVar5 = iVar5 + iVar7 * -2;
    iVar7 = iVar10 + iVar9;
    iVar9 = iVar9 - iVar10;
    dst[2] = (iVar5 + iVar4) >> 17;
    dst[5] = (iVar5 - iVar4) >> 17;
    dst[3] = (iVar9 + local_1c) >> 17;
    dst[4] = (iVar9 - local_1c) >> 17;
    dst[0] = (iVar7 + local_14) >> 17;
    dst[1] = (iVar6 + iVar8) >> 17;
    dst[6] = (iVar8 - iVar6) >> 17;
    dst[7] = (iVar7 - local_14) >> 17;
}

// ORIGINAL: 0x005D2D20 sub_5d2d20 0x005D2D20-0x005D2DF2 FILE
// TRIED: `extern "C" int __stdcall sub_5d2d20(int a1, int a2, int a3)` per the given def head - `test edx,edx` and `lea eax,[ecx+7]` read both registers with no matching stack loads, so two args (src ptr in ecx, mode in edx) are __fastcall register params, not part of a 3-int __stdcall; changed to `void __fastcall(char*,int,int,char*,char)` with 5 total params (2 register + 3 stack), void return (eax never set). `int step = stride * 4;` hoisted above the `if` (matching the unconditional `lea edi,[eax*4]` before the branch) gets close; best found is 83.6% mnemonic similarity (`--dir` picked /O1 framed over /O2 frameless) - remaining diff is a 4-instruction reorder of which register (edx vs ecx) holds `dst` while `adjust`/`i` are set up, which is register-allocation scheduling, not a control-flow gap.
// working copy - scaffold materialised by --work
// size      210 bytes
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_5d2d20  at 0x005D2D20  (210 bytes)
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
// `src` (ecx) and `mode` (edx) are __fastcall register args; `stride`,
// `dst` and `adjust` are the three stack args the `ret 0xc` pops - see
// TRIED above for the signature evidence.
extern "C" void __fastcall sub_5d2d20(char *src, int mode, int stride,
                                      char *dst, char adjust) {
    int step = stride * 4;
    char *p;
    char *q;
    int i;
    if (mode != 0) {
        p = src + 7;
        q = dst + 6;
        i = 8;
        do {
            q[-4] = p[-5] + adjust;
            q[0] = p[0] + adjust;
            q[4] = p[-1] + adjust;
            q[8] = p[4] + adjust;
            q[0xc] = p[3] + adjust;
            q[0x10] = p[8] + adjust;
            q[0x14] = p[7] + adjust;
            q[0x18] = p[0xc] + adjust;
            p = p + step;
            --i;
            q = q + 0x40;
        } while (i != 0);
        return;
    }
    p = src + 2;
    q = dst + 6;
    i = 8;
    do {
        q[-4] = p[1] + adjust;
        q[0] = p[0] + adjust;
        q[4] = p[5] + adjust;
        q[8] = p[4] + adjust;
        q[0xc] = p[9] + adjust;
        q[0x10] = p[8] + adjust;
        q[0x14] = p[0xd] + adjust;
        q[0x18] = p[0xc] + adjust;
        p = p + step;
        --i;
        q = q + 0x40;
    } while (i != 0);
}

// ORIGINAL: 0x005D3F50 sub_5d3f50 0x005D3F50-0x005D4014 FILE
// TRIED: signature cdecl() -> fastcall(int *param_1); evidence: ecx read directly at entry (dereferenced once for an object pointer, re-read every loop iteration), no stack access, plain `ret` (no callee-pop). Dropped Ghidra's explicit `& 0x1f` shift masks per the known lever (extra `and`); still MISMATCH at #4, likely register-allocation/prologue shape for the three precomputed shift amounts.
// working copy - scaffold materialised by --work
// size      196 bytes
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_5d3f50  at 0x005D3F50  (196 bytes)
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
extern "C" void __fastcall sub_5d3f50(int *param_1) {
    int obj0 = *param_1;
    unsigned char shiftB = (unsigned char)(8 - *(int *)(obj0 + 0x30));
    unsigned char shiftC = (unsigned char)(8 - *(int *)(obj0 + 0x2c));
    unsigned char shiftA = (unsigned char)(8 - *(int *)(obj0 + 0x34));

    int i = 0;
    int outOff = 0x3b0;
    do {
        int obj = *param_1;
        unsigned int term1 = *(unsigned char *)(obj + 0xb1 + i);
        term1 = (term1 >> shiftB) << *(int *)(obj + 0x3c);

        unsigned int term2 = *(unsigned char *)(obj + 0xb0 + i);
        term2 = (term2 >> shiftC) << *(int *)(obj + 0x38);

        unsigned int result = term1 | term2;

        unsigned int term3 = *(unsigned char *)(obj + 0xb2 + i);
        term3 = (term3 >> shiftA) << *(int *)(obj + 0x40);

        result |= term3;

        if (*(unsigned char *)(obj + 4) & 0x40) {
            result |= result << 0x10;
        }
        *(unsigned int *)(obj + outOff) = result;
        outOff += 4;
        i += 3;
    } while (outOff < 0x7b0);
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

// ORIGINAL: 0x005D427C sub_5d427c 0x005D427C-0x005D439F
// TRIED: entry uses pushal/popal (all eight registers saved and restored) around the whole body with cld and only rep movsb/ rep movsd inside, which VC6 never emits for ordinary compiled C++ - this is very likely inline assembly in the original source, which the rules here bar reproducing. Transcribed Ghidra's decompilation (already a faithful read of the byte stream) into plain C++ instead: a byte-oriented decoder that walks a control word one bit at a time, alternating literal dword/byte copies from the source with back-reference copies from already-written output, matching an LZ-style unpacker. No `__asm`; this compiles but will not reach the original's register-saving shape.
// size      291 bytes
// prototype
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)

extern "C" int __cdecl sub_5d427c(unsigned short *samples, unsigned char *steps,
                                  int a3) {
  unsigned short *param_3 = samples;
  unsigned char *param_4 = steps;
  int param_5 = a3;

  unsigned int uVar4;
  unsigned int uVar2;
  int iVar3;
  signed char cVar5;
  unsigned char bVar6;
  unsigned char uVar1;
  unsigned char *puVar8;
  unsigned char *puVar9;

  if (param_3 != 0) {
    unsigned char *puVar7 = (unsigned char *)(param_3 + 1);
    if (*param_3 & 1) {
      puVar7 = (unsigned char *)param_3 + 5;
    }
    puVar8 = puVar7 + 3;
    if (param_5 != 0) {
      do {
        uVar4 = *(unsigned int *)puVar8;
        cVar5 = (signed char)uVar4;
        while (uVar2 = uVar4 & 0xff, cVar5 < 0) {
          for (;;) {
            bVar6 = (unsigned char)(uVar4 >> 8);
            uVar1 = (unsigned char)(uVar4 >> 0x10);
            if ((signed char)((signed char)uVar2 * 2) >= 0) {
              break;
            }
            if ((signed char)((signed char)uVar2 << 2) < 0) {
              if ((unsigned char)uVar4 > 0xfb) {
                unsigned int n = uVar4 & 3;
                puVar8 = puVar8 + 1;
                while (n != 0) {
                  *param_4 = *puVar8;
                  param_4 = param_4 + 1;
                  puVar8 = puVar8 + 1;
                  n = n - 1;
                }
                goto label_5d4352;
              }
              puVar8 = puVar8 + 1;
              for (iVar3 = (int)(uVar4 & 0x1f) + 1; iVar3 != 0; iVar3 = iVar3 - 1) {
                *(unsigned int *)param_4 = *(unsigned int *)puVar8;
                puVar8 = puVar8 + 4;
                param_4 = param_4 + 4;
              }
              uVar4 = *(unsigned int *)puVar8;
              cVar5 = (signed char)uVar4;
            } else {
              puVar8 = puVar8 + 1;
              for (uVar2 = uVar4 & 3; uVar2 != 0; uVar2 = uVar2 - 1) {
                *param_4 = *puVar8;
                puVar8 = puVar8 + 1;
                param_4 = param_4 + 1;
              }
              {
                unsigned int bit4 = (uVar4 & 0x10) >> 4;
                unsigned int dist16 = (unsigned int)((bVar6 << 8) | uVar1);
                unsigned int distance = (bit4 << 0x10) | dist16;
                puVar9 = (param_4 - 1) - distance;
              }
              iVar3 = (int)((((unsigned char)uVar4 >> 2) << 8) |
                             (unsigned char)(uVar4 >> 0x18)) &
                          0x3ff;
              iVar3 = iVar3 + 5;
              for (; iVar3 != 0; iVar3 = iVar3 - 1) {
                *param_4 = *puVar9;
                puVar9 = puVar9 + 1;
                param_4 = param_4 + 1;
              }
              uVar4 = *(unsigned int *)puVar8;
              cVar5 = (signed char)uVar4;
            }
            uVar2 = uVar4 & 0xff;
            if (cVar5 >= 0) {
              goto label_5d42cc;
            }
          }
          puVar8 = puVar8 + 3;
          for (uVar2 = (unsigned int)(bVar6 >> 6); uVar2 != 0; uVar2 = uVar2 - 1) {
            *param_4 = *puVar8;
            puVar8 = puVar8 + 1;
            param_4 = param_4 + 1;
          }
          {
            unsigned int concat16 = (unsigned int)((bVar6 << 8) | uVar1);
            unsigned int distance = concat16 & 0xffff3fff;
            puVar9 = (param_4 - 1) - distance;
          }
          for (iVar3 = (int)(uVar4 & 0x3f) + 4; iVar3 != 0; iVar3 = iVar3 - 1) {
            *param_4 = *puVar9;
            puVar9 = puVar9 + 1;
            param_4 = param_4 + 1;
          }
          uVar4 = *(unsigned int *)puVar8;
          cVar5 = (signed char)uVar4;
        }
      label_5d42cc:
        puVar8 = puVar8 + 2;
        for (uVar2 = uVar2 & 3; uVar2 != 0; uVar2 = uVar2 - 1) {
          *param_4 = *puVar8;
          puVar8 = puVar8 + 1;
          param_4 = param_4 + 1;
        }
        {
          unsigned int concat16 =
              (unsigned int)((((unsigned char)uVar4 >> 5) << 8) |
                             (unsigned char)(uVar4 >> 8));
          puVar9 = (param_4 - 1) - concat16;
        }
        for (iVar3 = (int)((uVar4 & 0x1c) >> 2) + 3; iVar3 != 0; iVar3 = iVar3 - 1) {
          *param_4 = *puVar9;
          puVar9 = puVar9 + 1;
          param_4 = param_4 + 1;
        }
      } while (true);
    }
  }
label_5d4352:
  return 0;
}

// ORIGINAL: 0x005D439F sub_5d439f 0x005D439F-0x005D4448 FILE
// working copy - scaffold materialised by --work
// size      169 bytes
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_5d439f  at 0x005D439F  (169 bytes)
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
// IDA guessed int __cdecl(int,int,int,int); the tail never sets eax, so the
// real return type is void, and evidence (only [esp+N] reads, no ecx use)
// backs the __cdecl free-function shape with 4 stack ints.
extern "C" void __cdecl sub_5d439f(unsigned int *a1, int a2, int a3, int a4) {
    unsigned int bitpos;
    unsigned int mask;
    unsigned char width;
    unsigned char shift;
    unsigned int word;

    bitpos = 0;
    width = (unsigned char)a4;
    mask = (1 << (width & 0x1f)) - 1;
    while (a3 > 3) {
        shift = (unsigned char)bitpos & 7;
        word = *(unsigned int *)((bitpos >> 3) + a2);
        word = (word >> shift) | (word << (0x20 - shift));
        *a1 = word & mask;
        a1[1] = ((word >> (width & 0x1f)) | (word << (0x20 - (width & 0x1f)))) & mask;

        shift = (unsigned char)(bitpos + width * 2) & 7;
        word = *(unsigned int *)(((bitpos + a4 * 2) >> 3) + a2);
        word = (word >> shift) | (word << (0x20 - shift));
        a1[2] = word & mask;
        a1[3] = ((word >> (width & 0x1f)) | (word << (0x20 - (width & 0x1f)))) & mask;

        a1 += 4;
        bitpos += a4 * 4;
        a3 -= 4;
    }
    if (a3 - 4 > -4) {
        do {
            a3--;
            shift = (unsigned char)bitpos & 7;
            word = *(unsigned int *)((bitpos >> 3) + a2);
            *a1 = ((word >> shift) | (word << (0x20 - shift))) & mask;
            bitpos += a4;
            a1++;
        } while (a3 > 0);
    }
}

// ORIGINAL: 0x005D4448 sub_5d4448 0x005D4448-0x005D44FC FILE
// TRIED: byte-exact - `pushal`/`popal` at entry has no C++ spelling (documented wall). Landed the nested do/while-with-goto gather loop structurally instead: 9 stack args, an outer per-run counter reset from a saved [ebp+0x24] and an inner index-table lookup vs a direct 16-byte-stride struct-array fallback.
// working copy - scaffold materialised by --work
// size      180 bytes
// prototype 
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_5d4448  at 0x005D4448  (180 bytes)
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
extern "C" void __cdecl sub_5d4448(
    int *p1, int p2, int p3, int p4, int *p5, int p6, int p7, int p8, int p9)
{
    int saved_count = p8;
    int base8 = p6 - (p4 << 4);
    int base_c = p2 - p3;
    int stride = p7;
    int stride3 = stride * 3;
    int *src = p1;
    char *dst = reinterpret_cast<char *>(p3);
    int count;

    do {
        count = saved_count;
        for (;;) {
            int idx = *src;
            src += 1;
            if (idx < p4) {
                do {
                    int entry = p5[idx] + base_c + reinterpret_cast<int>(dst);
                    *reinterpret_cast<int *>(dst) = *reinterpret_cast<int *>(entry);
                    *reinterpret_cast<int *>(dst + stride) = *reinterpret_cast<int *>(entry + stride);
                    *reinterpret_cast<int *>(dst + stride * 2) = *reinterpret_cast<int *>(entry + stride * 2);
                    *reinterpret_cast<int *>(dst + stride3) = *reinterpret_cast<int *>(entry + stride3);
                    dst += 4;
                    count -= 1;
                    if (count <= 0) goto end_of_run;
                    idx = *src;
                    src += 1;
                } while (idx < p4);
            }
            {
                int entry2 = base8 + idx * 16;
                *reinterpret_cast<int *>(dst) = *reinterpret_cast<int *>(entry2);
                *reinterpret_cast<int *>(dst + stride) = *reinterpret_cast<int *>(entry2 + 4);
                *reinterpret_cast<int *>(dst + stride * 2) = *reinterpret_cast<int *>(entry2 + 8);
                *reinterpret_cast<int *>(dst + stride3) = *reinterpret_cast<int *>(entry2 + 0xc);
                dst += 4;
                count -= 1;
            }
            if (count <= 0) break;
        }
    end_of_run:
        dst += stride3;
        p9 -= 1;
    } while (p9 > 0);
}

// RECOVERY_ROLE: SCAFFOLD Mat3, Matrix3 and Mat3x3 are provisional 3x3 numerical views; original class identity, complete extent, callers, lifetime and x87 equivalence remain unresolved.
// ORIGINAL: 0x00634980 sub_634980 0x00634980-0x00634A50 FILE
// symbol    ?multiply@Mat3@@QAEXPAM0@Z
// TRIED: `extern "C" int __stdcall sub_634980(int a1, int a2)` per the given def head - disasm reads *ecx directly as a float* with no matching stack slot (a 3x3 matrix "this"), so the receiver is a __thiscall member, not a 2-arg __stdcall free function; changed to a `Mat3::multiply` member. `dst`/`a` tracked as independent incrementing pointers gives a MISMATCH at the prologue (#4, 87% similarity); an `int off = dst - a` delta recomputed each outer iteration (matching the original's spill of that delta to `[esp+0x14]` and its `add esi, ebx` re-derivation) gets to 89.5% similarity, 7 edits, all register-allocation choices (which callee-saved reg holds which pointer) rather than control-flow shape.
// working copy - scaffold materialised by --work
// size      208 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_634980  at 0x00634980  (208 bytes)
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
// A 3x3 row-major float matrix. `this` is the left operand (matches ecx
// dereferenced directly as a float base, no field offsets), `other` is the
// right operand, `out` receives the product - see TRIED above for the
// signature evidence.
class Mat3 {
public:
    // 3x3 row-major, which is the whole object: the image dereferences `this`
    // directly as a float base and never offsets past nine floats.
    float v[9];
    void multiply(float *out, float *other);
    // 0x00634920, homed here beside multiply: the same nine-float walk with
    // the same receiver-as-a-plain-float-base signature. Elementwise, so it
    // is the matrix equality and not an epsilon compare.
    bool equals(const float *other);
};

void Mat3::multiply(float *out, float *other) {
    float *a = v;
    float local[9] = {
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f
    };
    int off = reinterpret_cast<int>(local) - reinterpret_cast<int>(a);
    int row = 3;
    do {
        float *dst = reinterpret_cast<float *>(off + reinterpret_cast<int>(a));
        float *b = other;
        int col = 3;
        do {
            *dst = 0.0f;
            float sum = *dst;
            const float *pa = a;
            const float *pb = b;
            int k = 3;
            do {
                sum = *pa * *pb + sum;
                pa = pa + 1;
                pb = pb + 3;
                k = k - 1;
            } while (k != 0);
            *dst = sum;
            dst = dst + 1;
            b = other + (3 - col) + 1;
            col = col - 1;
        } while (col != 0);
        a += 3;
        --row;
    } while (row != 0);
    for (int i = 0; i < 9; ++i) {
        out[i] = local[i];
    }
}

// RECOVERY_ROLE: SCAFFOLD Mat3, Matrix3 and Mat3x3 are provisional 3x3 numerical views; original class identity, complete extent, callers, lifetime and x87 equivalence remain unresolved.
// ORIGINAL: 0x00634920 sub_634920 0x00634920-0x00634975 BYTE_EXACT
// symbol    ?equals@Mat3@@QAE_NPBM@Z
// LEVER: two indexed reads, not a hoisted difference. The image keeps `this`
//        and `other` apart and re-derives their difference inside the outer
//        loop (`sub edi, ebp` at 0x634939, one per row); spelling that
//        difference out as a local lets /O2 prove it loop-invariant and hoist
//        it past the loop it belongs to, which is 69 bytes against 85.
bool Mat3::equals(const float *other) {
    const float *mine = v;
    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) {
            if (mine[row * 3 + col] != other[row * 3 + col]) {
                return false;
            }
        }
    }
    return true;
}

// RECOVERY_ROLE: SCAFFOLD Mat3, Matrix3 and Mat3x3 are provisional 3x3 numerical views; original class identity, complete extent, callers, lifetime and x87 equivalence remain unresolved.
// ORIGINAL: 0x00634A50 sub_634a50 0x00634A50-0x00634AC9 FILE
// symbol    ?transform@Matrix3@@QAEXPAMPBM@Z
// working copy - scaffold materialised by --work
// size      121 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_634a50  at 0x00634A50  (121 bytes)
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
class Matrix3 { public:
    float v[9];
    void transform(float *out, const float *in);
};

void Matrix3::transform(float *out, const float *in) {
    const float *m = v;
    float result[3];
    result[0] = m[1] * in[1] + m[2] * in[2] + m[0] * in[0];
    result[1] = m[4] * in[1] + m[3] * in[0] + m[5] * in[2];
    result[2] = m[7] * in[1] + m[6] * in[0] + m[8] * in[2];
    out[0] = result[0];
    out[1] = result[1];
    out[2] = result[2];
}

// RECOVERY_ROLE: SCAFFOLD Mat3, Matrix3 and Mat3x3 are provisional 3x3 numerical views; original class identity, complete extent, callers, lifetime and x87 equivalence remain unresolved.
// ORIGINAL: 0x00634AD0 sub_634ad0 0x00634AD0-0x00634B67 FILE
// symbol    ?scale_into@Mat3x3@@QAEXPAMM@Z
// TRIED: extern "C" __stdcall(int,int) contract head - disasm reads ecx as an implicit receiver (thiscall) into a 3x3 float matrix, not a stack int; rewrote as Mat3x3::scale_into(float*,float). MISMATCH #1 push/lea, prologue layout only, structure otherwise matches Ghidra's nested-loop scale+copy.
// working copy - scaffold materialised by --work
// size      151 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_634ad0  at 0x00634AD0  (151 bytes)
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
class Mat3x3 { public:
    float m_[9];
    void scale_into(float *dest, float scale);
};

void Mat3x3::scale_into(float *dest, float scale) {
    float tmp[9] = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    float *pf1 = tmp;
    int base = (int)this - (int)tmp;
    int i4 = 3;
    do {
        int i3 = 3;
        float *pf2 = pf1;
        do {
            pf1 = pf2 + 1;
            i3 = i3 - 1;
            *pf2 = scale * *(float *)(base + (int)pf2);
            pf2 = pf1;
        } while (i3 != 0);
        i4 = i4 - 1;
    } while (i4 != 0);
    pf1 = tmp;
    for (i4 = 9; i4 != 0; i4 = i4 - 1) {
        *dest = *pf1;
        pf1 = pf1 + 1;
        dest = dest + 1;
    }
}

// ORIGINAL: 0x005D29E1 sub_5d29e1 0x005D29E1-0x005D2B20
// TRIED: this is not addressable with a normal calling convention - the body reads unaff_ESI/unaff_EDI with no stack access and no register setup of its own (first instruction is `push edi`, `mov ebx,[esi+0x14]` immediately after), so the 3 callers must be pre-loading esi/edi before `call`ing in. That is an interprocedural register-allocation artifact of compiling the 3 call sites and this body together as one translation unit; it cannot be reproduced by compiling this function alone under any ordinary C++ signature, since VC6 only picks a non-standard register ABI for an internal-linkage callee when it can see all of its callers. Declared it as a plain two-pointer __cdecl function instead (src, dst) for a faithful, compiling body. Also not reproduced: the fadd-then- reinterpret-low-dword pattern for double->int (`fstp qword ptr`, then `mov` of the low 4 bytes) is the classic magic- number float truncation trick that avoids calling `_ftol`; mimicked with a raw double-then-reinterpret-cast here. Best reached: MISMATCH, edit_count 7, 455 bytes vs 319 - the whole prologue/epilogue and call sequencing differ because of the calling-convention gap above.
// size      319 bytes
// prototype
// callers   3   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
extern "C" void __cdecl sub_5d29e1(int *src, int *dst) {
    int iVar2, iVar3, iVar4, iVar5, iVar6, iVar7, iVar8;
    double d1, d2;
    int local_1c, local_14;
    float fVar1;

    iVar7 = src[5];
    iVar6 = src[3];
    iVar5 = src[1];
    iVar3 = src[7];
    if (src[6] || iVar6 || iVar3 || src[2] || iVar7 || iVar5 || src[4]) {
        fVar1 = ((float)(iVar5 - iVar3) + (float)(iVar7 - iVar6)) * *(float *)IdctCos3Pi8;
        iVar4 = (int)(((__int64)(iVar3 + iVar5 - iVar6 - iVar7) * *IdctSin45Q31) >> 32) * 2;
        d1 = (double)((float)(iVar7 - iVar6) * *(float *)IdctInv2CosPi8 + fVar1) + *(float *)IdctAanScale;
        local_1c = *(int *)&d1;
        d2 = (double)((float)(iVar5 - iVar3) * *(float *)IdctInv2Cos3Pi8 - fVar1) + *(float *)IdctAanScale;
        local_14 = *(int *)&d2;
        iVar2 = local_1c + iVar4;
        iVar4 = iVar4 + local_14;
        local_14 = local_14 + iVar6 + iVar7 + iVar3 + iVar5;
        iVar7 = src[4] + src[0];
        iVar3 = src[0] - src[4];
        iVar5 = (int)(((__int64)(src[2] - src[6]) * *IdctSin45Q31) >> 32);
        iVar6 = iVar5 * 2;
        iVar8 = src[6] + src[2] + iVar6;
        iVar6 = iVar6 + iVar3;
        iVar3 = iVar3 - iVar5 * 2;
        iVar5 = iVar8 + iVar7;
        iVar7 = iVar7 - iVar8;
        dst[0x12] = iVar3 + iVar2;
        dst[0x2d] = iVar3 - iVar2;
        dst[0x1b] = iVar7 + local_1c;
        dst[0x24] = iVar7 - local_1c;
        dst[0] = iVar5 + local_14;
        dst[9] = iVar4 + iVar6;
        dst[0x36] = iVar6 - iVar4;
        dst[0x3f] = iVar5 - local_14;
        return;
    }
    iVar7 = src[0];
    dst[0] = iVar7;
    dst[9] = iVar7;
    dst[0x12] = iVar7;
    dst[0x1b] = iVar7;
    dst[0x24] = iVar7;
    dst[0x2d] = iVar7;
    dst[0x36] = iVar7;
    dst[0x3f] = iVar7;
    return;
}

// ORIGINAL: 0x004243F0 ?clear_council_notify@@YAXXZ 0x004243F0-0x0042440D FILE BYTE_EXACT
// size      29 bytes
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x004243F0
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004243f0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?clear_council_notify@@YAXXZ  at 0x004243F0  (29 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    an indirect virtual call use the generated VCall shim below; for a
//    thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.

void __cdecl clear_council_notify() {
    memset(CouncilNotifyPluralSlots, 0, 8 * sizeof(int));
    memset(CouncilNotifyFormSlots, 0, 8 * sizeof(int));
}

// ORIGINAL: 0x005D2E00 sub_5d2e00 0x005D2E00-0x005D2E6E FILE BYTE_EXACT
// symbol    ?sub_5d2e00@@YIXPADHH0D@Z
// working copy - scaffold materialised by --work
// size      110 bytes
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_5d2e00  at 0x005D2E00  (110 bytes)
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
// `lea eax, [ecx+edx]` with no stack access for the first two operands is
// __fastcall (ecx=a1, edx=a2); three more come off the stack, the last a
// single byte - matches `ret 0xc` (3 stack slots popped by the callee).
// a1 and a4 are pointers the census's H was only guessing at; the body adds a
// byte offset to each, which is the proof.
void __fastcall sub_5d2e00(char *a1, int a2, int a3, char *a4, char a5) {
    char *src = a1 + a2;
    int stride = a3 * 8;
    char *dst = a4 + 6;
    int count = 8;
    do {
        dst[-4] = src[0] + a5;
        dst[0] = src[4] + a5;
        dst[4] = src[8] + a5;
        dst[8] = src[0xc] + a5;
        dst[0xc] = src[0x10] + a5;
        dst[0x10] = src[0x14] + a5;
        dst[0x14] = src[0x18] + a5;
        dst[0x18] = src[0x1c] + a5;
        src = src + stride;
        count = count - 1;
        dst = dst + 0x20;
    } while (count != 0);
}

// ORIGINAL: 0x004243D0 ?clear_council_notify@@YAXH@Z 0x004243D0-0x004243E8 BYTE_EXACT
// size      24 bytes
// prototype 
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// 0x004243D0  ?clear_council_notify@@YAXH@Z  ->  ?clear_council_notify@@YAXH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.


void __cdecl clear_council_notify(int a1) {
    CouncilNotifyFormSlots[a1] = 0;
    CouncilNotifyPluralSlots[a1] = 0;
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

// IMA ADPCM nibble -> step-index adjustment, pre-scaled by 64 so the index
// itself travels as a byte offset: the eight dwords are {-1,-1,-1,-1,2,4,6,8}
// times 64, and the 0x1600 clamp below is 88 * 64, the IMA step index's own
// maximum. Image .data at 0x00694DF4.
// Pinned image initial bytes: eight prefix words, 89 rows of 16 deltas,
// eight adjustments, then four zero state words. Every delta was compared
// with the original, including signed nibbles and the final step of 32767.
// Earlier seed notes describing these lookup words as zero were incorrect.
#define MEDIA_ADPCM_MAG(s, n) (((s) >> 3) + (((n) & 1) ? ((s) >> 2) : 0) + (((n) & 2) ? ((s) >> 1) : 0) + (((n) & 4) ? (s) : 0))
#define MEDIA_ADPCM_ROW(s) MEDIA_ADPCM_MAG(s, 0), MEDIA_ADPCM_MAG(s, 1), MEDIA_ADPCM_MAG(s, 2), MEDIA_ADPCM_MAG(s, 3), MEDIA_ADPCM_MAG(s, 4), MEDIA_ADPCM_MAG(s, 5), MEDIA_ADPCM_MAG(s, 6), MEDIA_ADPCM_MAG(s, 7), -MEDIA_ADPCM_MAG(s, 0), -MEDIA_ADPCM_MAG(s, 1), -MEDIA_ADPCM_MAG(s, 2), -MEDIA_ADPCM_MAG(s, 3), -MEDIA_ADPCM_MAG(s, 4), -MEDIA_ADPCM_MAG(s, 5), -MEDIA_ADPCM_MAG(s, 6), -MEDIA_ADPCM_MAG(s, 7)
// Access-window binding: native coefficient readers at 0x005CDB90/0x005CDD10
// overlap the first delta row; 0x005CE6E3 owns the four trailing state writes.
// This groups proven storage accesses, not a recovered original declaration.
// RECOVERY_OBJECT: v2 0x00693794 0x00694E24 ?MediaAdpcmTables@@3UMediaAdpcmStorage@@A 0x005CE6E3
MediaAdpcmStorage MediaAdpcmTables = {
    {0, 240, 460, 392, 0, 0, -208, -220,
     MEDIA_ADPCM_ROW(7), MEDIA_ADPCM_ROW(8), MEDIA_ADPCM_ROW(9),
     MEDIA_ADPCM_ROW(10), MEDIA_ADPCM_ROW(11), MEDIA_ADPCM_ROW(12),
     MEDIA_ADPCM_ROW(13), MEDIA_ADPCM_ROW(14), MEDIA_ADPCM_ROW(16),
     MEDIA_ADPCM_ROW(17), MEDIA_ADPCM_ROW(19), MEDIA_ADPCM_ROW(21),
     MEDIA_ADPCM_ROW(23), MEDIA_ADPCM_ROW(25), MEDIA_ADPCM_ROW(28),
     MEDIA_ADPCM_ROW(31), MEDIA_ADPCM_ROW(34), MEDIA_ADPCM_ROW(37),
     MEDIA_ADPCM_ROW(41), MEDIA_ADPCM_ROW(45), MEDIA_ADPCM_ROW(50),
     MEDIA_ADPCM_ROW(55), MEDIA_ADPCM_ROW(60), MEDIA_ADPCM_ROW(66),
     MEDIA_ADPCM_ROW(73), MEDIA_ADPCM_ROW(80), MEDIA_ADPCM_ROW(88),
     MEDIA_ADPCM_ROW(97), MEDIA_ADPCM_ROW(107), MEDIA_ADPCM_ROW(118),
     MEDIA_ADPCM_ROW(130), MEDIA_ADPCM_ROW(143), MEDIA_ADPCM_ROW(157),
     MEDIA_ADPCM_ROW(173), MEDIA_ADPCM_ROW(190), MEDIA_ADPCM_ROW(209),
     MEDIA_ADPCM_ROW(230), MEDIA_ADPCM_ROW(253), MEDIA_ADPCM_ROW(279),
     MEDIA_ADPCM_ROW(307), MEDIA_ADPCM_ROW(337), MEDIA_ADPCM_ROW(371),
     MEDIA_ADPCM_ROW(408), MEDIA_ADPCM_ROW(449), MEDIA_ADPCM_ROW(494),
     MEDIA_ADPCM_ROW(544), MEDIA_ADPCM_ROW(598), MEDIA_ADPCM_ROW(658),
     MEDIA_ADPCM_ROW(724), MEDIA_ADPCM_ROW(796), MEDIA_ADPCM_ROW(876),
     MEDIA_ADPCM_ROW(963), MEDIA_ADPCM_ROW(1060), MEDIA_ADPCM_ROW(1166),
     MEDIA_ADPCM_ROW(1282), MEDIA_ADPCM_ROW(1411), MEDIA_ADPCM_ROW(1552),
     MEDIA_ADPCM_ROW(1707), MEDIA_ADPCM_ROW(1878), MEDIA_ADPCM_ROW(2066),
     MEDIA_ADPCM_ROW(2272), MEDIA_ADPCM_ROW(2499), MEDIA_ADPCM_ROW(2749),
     MEDIA_ADPCM_ROW(3024), MEDIA_ADPCM_ROW(3327), MEDIA_ADPCM_ROW(3660),
     MEDIA_ADPCM_ROW(4026), MEDIA_ADPCM_ROW(4428), MEDIA_ADPCM_ROW(4871),
     MEDIA_ADPCM_ROW(5358), MEDIA_ADPCM_ROW(5894), MEDIA_ADPCM_ROW(6484),
     MEDIA_ADPCM_ROW(7132), MEDIA_ADPCM_ROW(7845), MEDIA_ADPCM_ROW(8630),
     MEDIA_ADPCM_ROW(9493), MEDIA_ADPCM_ROW(10442), MEDIA_ADPCM_ROW(11487),
     MEDIA_ADPCM_ROW(12635), MEDIA_ADPCM_ROW(13899), MEDIA_ADPCM_ROW(15289),
     MEDIA_ADPCM_ROW(16818), MEDIA_ADPCM_ROW(18500), MEDIA_ADPCM_ROW(20350),
     MEDIA_ADPCM_ROW(22385), MEDIA_ADPCM_ROW(24623), MEDIA_ADPCM_ROW(27086),
     MEDIA_ADPCM_ROW(29794), MEDIA_ADPCM_ROW(32767)},
    {-64, -64, -64, -64, 128, 256, 384, 512}, 0, 0, 0, 0
};
#undef MEDIA_ADPCM_ROW
#undef MEDIA_ADPCM_MAG

static int *const AdpcmIndexAdjust = MediaAdpcmTables.index_adjust;
// THE REST OF THE IMA ADPCM DECODER'S STATE, and the delta table. All four
// state words are zero in the shipped .data (they are decoder state, not
// data), so the evidence for the names is the arithmetic:
//   * 0x00694E14 and 0x00694E18 are filled from the two HALVES of one dword
//     the caller hands in - `(short)*param_6` and `(short)(*param_6 >> 16)` -
//     are clamped to [-32768, 32767] and written back as the two halves of
//     the caller's dword. That is a stereo predictor pair.
//   * 0x00694E1C and 0x00694E20 do the same with the second dword, are
//     adjusted by AdpcmIndexAdjust and clamped to [0, 0x1600] = [0, 88 * 64],
//     the IMA step index. That is the matching step-index pair.
//   * 0x006937B4 is indexed `(nibble * 4) + step_index`, i.e. [code][index],
//     and its first row reads 0,1,3,4,7,8,10,11 - exactly step 7 split by the
//     three magnitude bits of an IMA nibble. 89 rows of 16 dwords.
static int *const AdpcmPredictorLeft = &MediaAdpcmTables.predictor_left;
static int *const AdpcmPredictorRight = &MediaAdpcmTables.predictor_right;
static int *const AdpcmStepIndexLeft = &MediaAdpcmTables.step_index_left;
static int *const AdpcmStepIndexRight = &MediaAdpcmTables.step_index_right;

// Preserve the native byte-offset interpretation, including unaligned input
// offsets, without an unaligned int dereference. This is not a bounds waiver.
static int media_adpcm_delta(int step_byte_offset, unsigned int nibble) {
    int delta;
    memcpy(&delta, reinterpret_cast<const unsigned char *>(MediaAdpcmTables.lookup_and_delta_words + 8) +
           step_byte_offset + nibble * sizeof(int), sizeof(delta));
    return delta;
}


// ORIGINAL: 0x005CE640 sub_5ce640 0x005CE640-0x005CE6E3 FILE
// symbol    ?sub_5ce640@@YAXHHHHPAH0@Z
// size      163 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// Purpose: Decode `sample_total - first_sample` samples of IMA ADPCM audio
//          into 16-bit output. `codes` is the packed nibble stream, `samples`
//          the destination; `predictor` and `step_index` are the caller's two
//          words of decoder state, both read AND written, so the decode
//          resumes where the previous block left off. Evidence for the
//          identification: `predicted` is clamped to [-32768, 32767], the
//          delta comes out of the 8x88 table at 0x006937B4 whose first row
//          (0,1,3,4,7,8,10,11) is exactly step 7 scaled by the three
//          magnitude bits, and the adjustment table is the IMA one.
// TRIED: homed into general.cpp (its real TU) 2026-09-03 and reverted because
// its one binding was then thought evidence-free. It is not: image_data.py at
// 0x00694DF4 reads {-64,-64,-64,-64,128,256,384,512}, which is the IMA index
// adjustment {-1,-1,-1,-1,2,4,6,8} scaled by 64, and 0x1600 is 88 * 64. Named
// AdpcmIndexAdjust, 2026-09-06.
// TRIED: byte-exactness is out of reach here - the image's fourth instruction
// is `pushal` (0x60), which VC6 never emits for any C++ body; this is a
// handwritten_asm finding, so the ceiling is semantic equivalence.
void __cdecl sub_5ce640(int first_sample, int codes, int samples,
                        int sample_total, int *predictor, int *step_index) {
    int count;
    unsigned char *src;
    short *dst;
    int step;
    int predicted;
    unsigned char nibble;

    count = 0;
    count += first_sample;
    sample_total += first_sample;
    step = *step_index;
    predicted = (short)*predictor;
    src = (unsigned char *)codes;
    dst = (short *)samples;

    do {
        nibble = *src;
        if ((count & 1) == 0) {
            nibble >>= 4;
        } else {
            nibble &= 0xF;
            src++;
        }
        predicted += media_adpcm_delta(step, nibble);
        if (predicted > 0x7FFF) {
            predicted = 0x7FFF;
        } else if (predicted < (int)0xFFFF8000) {
            predicted = (int)0xFFFF8000;
        }
        *dst = (short)predicted;
        dst++;
        count++;
        nibble &= 7;
        step += AdpcmIndexAdjust[nibble];
        if (step > 0x1600) {
            step = 0x1600;
        } else if (step < 0) {
            step = 0;
        }
    } while (count < sample_total);

    *predictor = predicted;
    *step_index = step;
}

// THE MOVIE CODEC'S RUNTIME-FILLED LOOKUP TABLES. All four are zero in the
// shipped .data and filled before use, so image_data.py has nothing to read
// at any of them; the names come from how the bodies index them, which is the
// evidence the anonymous-global rule asks for.
//
// Three 512-entry word tables indexed by three DISJOINT 9-bit fields of one
// accumulated sum - sub_5d2761 forms `(sum >> 23)`, `(sum >> 11) & 0x1ff` and
// `sum & 0x1ff` and adds one entry from each - plus a 256-entry byte remap
// that sub_5ceb12 packs two at a time into each output word.
static DWORD *const MovieBlendTableHi =
    movie_media_conversion_tables.blend_high;
static DWORD *const MovieBlendTableMid =
    movie_media_conversion_tables.blend_mid;
static DWORD *const MovieBlendTableLow =
    movie_media_conversion_tables.blend_low;
static unsigned char *const MovieByteRemapLut =
    reinterpret_cast<unsigned char *>(movie_media_conversion_tables.remap_or_lookup_storage.words);

// A second tier of the same family, named from the lane each one indexes.
// sub_5d2834 / sub_5d2901 build a sum and pull one entry per BYTE lane out of
// it - `[sum >> 0x18]`, `[sum & 0xff]`, `[(sum >> 8) & 0xff]` - so each of
// these is a 256-entry-per-byte-value contribution table for one lane.
static DWORD *const MovieColourTableByte3 =
    movie_media_conversion_tables.contribution_byte3;
static DWORD *const MovieColourTableByte1 =
    movie_media_conversion_tables.contribution_byte1;
static DWORD *const MovieColourTableByte0 =
    movie_media_conversion_tables.contribution_byte0;
// sub_5d1b88 / sub_5d2037 index these three by three DIFFERENT coefficient
// streams rather than by byte lanes: `data[i]`, `tables[i]` and
// `tables[0x40 + i]` respectively.
static DWORD *const MovieBlendTableData =
    movie_media_conversion_tables.contribution_byte3 + 256;
static DWORD *const MovieBlendTableQuant =
    movie_media_conversion_tables.contribution_byte1 + 256;
static DWORD *const MovieBlendTableQuant2 =
    movie_media_conversion_tables.contribution_byte0 + 256;

// ORIGINAL: 0x005D3DF0 sub_5d3df0 0x005D3DF0-0x005D3F48 FILE
// symbol    ?sub_5d3df0@@YIHPAHH@Z
// size      344 bytes
// prototype
// callers   ?   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D427C 0x006470A6 0x00644EF2
// TRIED: __fastcall(this,a2) signature chosen from the ecx=this / edx=a2 register reads at entry; PROPOSAL below. Landed at 0.88 mnemonic similarity - remaining divergence is prologue register-save scheduling, not logic.
typedef int (__cdecl *sub5d427c_fn)(int, int, int);

int __fastcall sub_5d3df0(int *thisPtr, int frame_base) {
  int savedIdx = thisPtr[0x187];
  int *inner = reinterpret_cast<int *>(thisPtr[0]);
  unsigned int allocSize = static_cast<unsigned int>(inner[0x19] * inner[0x18] + 0xf) & 0xfffffff0u;

  if (thisPtr[0x18e] != 0) {
    free(reinterpret_cast<void *>(thisPtr[0x18e]));
    thisPtr[0x18e] = 0;
  }
  if (thisPtr[399] != 0) {
    free(reinterpret_cast<void *>(thisPtr[399]));
    thisPtr[399] = 0;
  }
  if (thisPtr[0x191] != 0) {
    free(reinterpret_cast<void *>(thisPtr[0x191]));
    thisPtr[0x191] = 0;
  }
  if (thisPtr[400] != 0) {
    free(reinterpret_cast<void *>(thisPtr[400]));
    thisPtr[400] = 0;
  }

  thisPtr[0x18e] = reinterpret_cast<int>(malloc(allocSize));
  thisPtr[399] = reinterpret_cast<int>(malloc(allocSize));
  thisPtr[0x191] = reinterpret_cast<int>(malloc(inner[0x18] << 3));
  thisPtr[400] = reinterpret_cast<int>(malloc(((allocSize + 0xf) >> 4) << 2));

  if (thisPtr[0x18e] != 0 && thisPtr[399] != 0 && thisPtr[0x191] != 0 && thisPtr[400] != 0) {
    int stride = inner[0x18];
    int value = -(inner[0x19] * stride);
    unsigned int idx = 0;
    if ((stride & 0x7fffffff) != 0) {
      do {
        *reinterpret_cast<int *>(thisPtr[0x191] + idx * 4) = value;
        value += inner[0x19];
        idx++;
      } while (idx < static_cast<unsigned int>(inner[0x18] << 1));
    }
    reinterpret_cast<sub5d427c_fn>(sub_5d427c)(frame_base + (savedIdx + 4) * 3, thisPtr[0x18e], 1);
    return 1;
  }
  return 0;
}

// ORIGINAL: 0x005CEB12 sub_5ceb12 0x005CEB12-0x005CED1D FILE
// symbol    ?sub_5ceb12@@YAHPBE0PAI@Z
// TRIED: no C-callable convention passes 3 pointers in esi/edi/ebp with zero prologue and no stack args - the original has no `push ebp`/frame at all and even leaves ecx/edx's high 24 bits as whatever the caller left them (Ghidra's CONCAT31 "uVar1/uVar2" tracking), which only a hand-written leaf/tail-continuation does. Landed as a real 3-pointer function reproducing the LUT-pack arithmetic; first divergence is the (unavoidable) prologue itself.
// working copy - scaffold materialised by --work
// size      523 bytes
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_5ceb12  at 0x005CEB12  (523 bytes)
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

int __cdecl sub_5ceb12(const unsigned char *srcA, const unsigned char *srcB, unsigned int *dst) {
    for (int i = 0; i < 8; ++i) {
        int aIdx1 = 2 + i * 8;
        int aIdx2 = aIdx1 + 4;
        int bIdx1 = 2 + i * 4;
        int bIdx2 = bIdx1 + 0x100;
        unsigned int hi = (static_cast<unsigned int>(MovieByteRemapLut[srcA[aIdx1]]) << 8) | MovieByteRemapLut[srcA[aIdx2]];
        unsigned int lo = (static_cast<unsigned int>(MovieByteRemapLut[srcB[bIdx1]]) << 8) | MovieByteRemapLut[srcB[bIdx2]];
        dst[i] = (hi << 0x10) | lo;
    }
    return 0;
}

// ORIGINAL: 0x005D2037 sub_5d2037 0x005D2037-0x005D2536 FILE
// symbol    ?sub_5d2037@@YAXPAH0PAI@Z
// TRIED: nullary per the contract head - no [esp+N]/[ebp+N] read exists anywhere in the body, but esi/edi/ebp are read from the very first instruction with no prologue and no assignment into them first, so nullary leaves the data unreachable. Proposed a 3-pointer signature (data/tables/out standing in for esi/edi/ebp) so the 8 unrolled DES-style S-box rounds compile and read correctly; this is a PROPOSAL, not decoded evidence - no standard calling convention passes 3 live registers with zero prologue, so BYTE_EXACT is structurally unreachable here short of __asm (refused). MISMATCH #0 is the prologue itself.
// working copy - scaffold materialised by --work
// size      1279 bytes
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_5d2037  at 0x005D2037  (1279 bytes)
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

void __cdecl sub_5d2037(int *data, int *tables, unsigned int *out) {
    for (int i = 0; i < 8; i++) {
        unsigned int left = MovieBlendTableQuant[tables[i]] + MovieBlendTableData[data[i * 2]] +
                             MovieBlendTableQuant2[tables[0x40 + i]];
        unsigned int right = MovieBlendTableData[data[i * 2 + 1]] + MovieBlendTableQuant2[tables[0x40 + i]] +
                              MovieBlendTableQuant[tables[i]];

        out[i] = ((MovieBlendTableHi[right >> 0x17] | MovieBlendTableLow[right & 0x1ff] |
                   MovieBlendTableMid[(right & 0xff800) >> 0xb]) &
                  0xffff0000) |
                 ((MovieBlendTableMid[(left & 0xff800) >> 0xb] | MovieBlendTableHi[left >> 0x17] |
                   MovieBlendTableLow[left & 0x1ff]) &
                  0xffff);
    }
}

// ORIGINAL: 0x005D2761 sub_5d2761 0x005D2761-0x005D2834 FILE
// symbol    ?sub_5d2761@@YAHPAI0H@Z
// LEVER: the four tbl_ globals are four of EIGHT 0x400-byte tables laid
//         end to end at 0x009C672C + k*0x400 for k = 0..7, all ZERO in the
//         file and filled at run time (image_data.py reads 0 at every one).
//         0x5CC4E0..0x5CC590 is their builder: a 0x100-iteration loop with
//         variable-count shifts (sar/shl by cl) writing through
//         `mov [edi+0x9c7f2c]` - a canonical VLC/Huffman code table
//         builder, not a cipher S-box. sub_5d2761 consumes four of them by
//         byte-lane, which is a decode-table lookup, so the whole
//         0x005CE85C..0x005D4502 span is a block-DCT movie decoder and not
//         part of the DirectDraw half of this TU.
// TRIED: original prologue saves 6 registers (ebx,ecx,edx,esi,edi,ebp); plain __cdecl(uint*,uint*,int) body only pressures 1-2, so the reservation never reproduces. Explicit per-register temps for each S-box term didn't move it either.
// working copy - scaffold materialised by --work
// size      211 bytes
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_5d2761  at 0x005D2761  (211 bytes)
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

int __cdecl sub_5d2761(unsigned int *src, unsigned int *dst, int count) {
    do {
        unsigned int w = *src;
        unsigned int sum1 = MovieColourTableByte1[(w & 0xff00) >> 8] + MovieColourTableByte3[w >> 24] + MovieColourTableByte0[w & 0xff];
        unsigned int sum2 = MovieColourTableByte3[((unsigned char *)src)[2]] + MovieColourTableByte0[w & 0xff] + MovieColourTableByte1[(w & 0xff00) >> 8];
        count = count - 1;
        *dst = ((MovieBlendTableHi[sum2 >> 0x17] + MovieBlendTableLow[sum2 & 0x1ff] + MovieBlendTableMid[(sum2 & 0xff800) >> 0xb]) & 0xffff0000)
             | ((MovieBlendTableHi[sum1 >> 0x17] + MovieBlendTableMid[(sum1 & 0xff800) >> 0xb] + MovieBlendTableLow[sum1 & 0x1ff]) & 0xffff);
        dst++;
        src++;
    } while (count != 0);
    return 0;
}

// Fixed-slot bindings carried from 005d2834.cpp

// ORIGINAL: 0x005D2834 sub_5d2834 0x005D2834-0x005D2901 FILE
// symbol    ?sub_5d2834@@YAHPAIPAHH@Z
// TRIED: literal Ghidra-order locals (uVar1/uVar2) and inline expr order both produce a tight 191-byte body without the 6-register push prologue the original has; register pressure not reproduced.
// working copy - scaffold materialised by --work
// size      205 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_5d2834  at 0x005D2834  (205 bytes)
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

int __cdecl sub_5d2834(unsigned int *param_1, int *param_2, int param_3) {
    unsigned int uVar1;
    unsigned int uVar2;

    do {
        uVar2 = *param_1;
        uVar1 = MovieColourTableByte1[(uVar2 & 0xff00) >> 8] +
                MovieColourTableByte3[uVar2 >> 0x18] +
                MovieColourTableByte0[uVar2 & 0xff];
        uVar2 = MovieColourTableByte3[*(unsigned char *)((int)param_1 + 2)] +
                MovieColourTableByte0[uVar2 & 0xff] +
                MovieColourTableByte1[(uVar2 & 0xff00) >> 8];
        *param_2 = MovieBlendTableHi[uVar1 >> 0x17] + MovieBlendTableMid[(uVar1 & 0xff800) >> 0xb] +
                   MovieBlendTableLow[uVar1 & 0x1ff];
        param_3 = param_3 + -1;
        param_2[1] = MovieBlendTableHi[uVar2 >> 0x17] + MovieBlendTableLow[uVar2 & 0x1ff] +
                     MovieBlendTableMid[(uVar2 & 0xff800) >> 0xb];
        param_2 = param_2 + 2;
        param_1 = param_1 + 1;
    } while (param_3 != 0);
    return 0;
}

// ORIGINAL: 0x005D2901 sub_5d2901 0x005D2901-0x005D29E1 FILE
// symbol    ?sub_5d2901@@YAHPAH00H@Z
// TRIED: extern int name[] for the six indexed lookup tables (same tables as the sibling sub_5d2834 at 0x005D2834), not the given `static int *const` - these are used as `table[idx]`, an indexed base. Signature widened to the IDA-guessed 4 params (int*,int*,int*,int) since the given head has none.
// TRIED: could not reproduce the 6-register push prologue (push ebx,ecx,edx,esi,edi,ebp - ecx/edx pushed as scratch stack slots, not preservation); same wall the sibling function at 0x005D2834 hit. Hoisting the shared table_7f2c+table_772c subexpression into named locals `a`/`b` (to force it live across both blend computations, matching the original's edx reuse) made similarity slightly worse (0.61 -> 0.60), not better.
// working copy - scaffold materialised by --work
// size      224 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_5d2901  at 0x005D2901  (224 bytes)
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

int __cdecl sub_5d2901(int *param_1, int *param_2, int *param_3, int param_4) {
    unsigned int uVar3;
    unsigned int uVar4;
    unsigned int uVar5;

    do {
        int iVar1 = *param_1;
        int iVar2 = *param_2;
        param_1 = param_1 + 1;
        uVar3 = (unsigned int)(iVar1 + iVar2) >> 1;
        param_2 = param_2 + 1;
        uVar4 = uVar3 & 0x7f7f7f7f;

        uVar5 = MovieColourTableByte0[uVar4 & 0xff] + MovieColourTableByte1[(uVar4 >> 8) & 0xff] +
                MovieColourTableByte3[uVar4 >> 0x18];
        uVar3 = MovieColourTableByte0[uVar4 & 0xff] + MovieColourTableByte1[(uVar4 >> 8) & 0xff] +
                MovieColourTableByte3[(uVar4 >> 0x10) & 0xff];

        *param_3 = MovieBlendTableLow[uVar5 & 0x1ff] + MovieBlendTableHi[uVar5 >> 0x17] +
                   MovieBlendTableMid[(uVar5 >> 0xb) & 0x1ff];
        param_4 = param_4 - 1;
        param_3[1] = MovieBlendTableMid[(uVar3 >> 0xb) & 0x1ff] +
                     MovieBlendTableHi[uVar3 >> 0x17] + MovieBlendTableLow[uVar3 & 0x1ff];
        param_3 = param_3 + 2;
    } while (param_4 != 0);

    return 0;
}

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

// ORIGINAL: 0x005D4020 sub_5d4020 0x005D4020-0x005D4231 FILE
// symbol    ?sub_5d4020@@YIXPAXPAEHH@Z
// TRIED: not chased past the first instruction (MISMATCH #0 'push' vs 'sub', 62% mnemonic similarity) - __fastcall(this, dest, int, int) reproduces the `ret 8` pop count and the edx-as-second- register-arg evidence (no `mov ebp,esp` frame, `[esp+0x10]` live from entry with no stack spill) but the prologue's register-save order still differs. Structurally follows the raw disassembly's three shapes (mode==8 raw copy via sub_5d271d, else 2-byte-per-pixel CLUT unrolled x8, else 4-byte-per-pixel CLUT unrolled x8).
// working copy - scaffold materialised by --work
// size      529 bytes
// prototype 
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D271D

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_5d4020  at 0x005D4020  (529 bytes)
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

void __fastcall sub_5d4020(void *thisPtr, unsigned char *dest,
                                       int unusedStackArg, int destStride) {
    unsigned char *self = (unsigned char *)thisPtr;
    unsigned char *src = *(unsigned char **)(self + 0x638);
    unsigned char *info = *(unsigned char **)self;
    if (*(int *)(info + 0x48) == 8) {
        if ((*(unsigned char *)(info + 4) & 0x40) != 0) {
            sub_5d271d((unsigned int *)src, dest, *(int *)(info + 0x64) * *(int *)(info + 0x60));
            return;
        }
        if (*(int *)(info + 0x60) == 0) {
            return;
        }
        {
            unsigned int rowCount = *(unsigned int *)(info + 0x60);
            unsigned int rowBytes = *(unsigned int *)(info + 0x64);
            unsigned int row = 0;
            unsigned char *s = src;
            unsigned char *d = dest;
            do {
                unsigned char *sp = s;
                unsigned char *dp = d;
                unsigned int n4 = rowBytes >> 2;
                for (; n4 != 0; --n4) {
                    *(unsigned int *)dp = *(unsigned int *)sp;
                    sp += 4;
                    dp += 4;
                }
                unsigned int n1 = rowBytes & 3;
                for (; n1 != 0; --n1) {
                    *dp = *sp;
                    ++sp;
                    ++dp;
                }
                info = *(unsigned char **)self;
                rowBytes = *(unsigned int *)(info + 0x64);
                d += (destStride >> 2) * 4;
                s += rowBytes;
                ++row;
            } while (row < *(unsigned int *)(info + 0x60));
        }
        return;
    }

    unsigned char *clut = info + 0x3b0;
    int row = 0;
    if ((*(unsigned char *)(info + 4) & 0x40) == 0) {
        if (*(int *)(info + 0x60) == 0) {
            return;
        }
        do {
            unsigned int col = 0;
            unsigned char *sp = src;
            unsigned char *dp = dest;
            if (*(int *)(info + 0x64) != 0) {
                do {
                    col += 8;
                    src = sp + 8;
                    dest = dp + 0x10;
                    for (int k = 0; k < 8; ++k) {
                        *(unsigned short *)(dp + k * 2) =
                            *(unsigned short *)(clut + sp[k] * 4);
                    }
                    sp = src;
                    dp = dest;
                } while (col < *(unsigned int *)(*(unsigned char **)self + 0x64));
            }
            info = *(unsigned char **)self;
            dest = dest + ((destStride - *(int *)(info + 0x64) * 2) & ~1);
            ++row;
        } while ((unsigned int)row < *(unsigned int *)(info + 0x60));
        return;
    }

    if (*(int *)(info + 0x60) == 0) {
        return;
    }
    do {
        unsigned int col = 0;
        unsigned char *sp = src;
        unsigned char *dp = dest;
        if (*(int *)(info + 0x64) != 0) {
            do {
                col += 8;
                src = sp + 8;
                dest = dp + 0x20;
                for (int k = 0; k < 8; ++k) {
                    *(unsigned int *)(dp + k * 4) =
                        *(unsigned int *)(clut + sp[k] * 4);
                }
                sp = src;
                dp = dest;
            } while (col < *(unsigned int *)(*(unsigned char **)self + 0x64));
        }
        info = *(unsigned char **)self;
        dest = dest + ((destStride - *(int *)(info + 0x64) * 4) & ~3);
        ++row;
    } while ((unsigned int)row < *(unsigned int *)(info + 0x60));
}

// ORIGINAL: 0x005D3980 sub_5d3980 0x005D3980-0x005D3DF0 FILE
// symbol    ?sub_5d3980@@YIHPAHPAI@Z
// TRIED: __fastcall(int *, unsigned int *) direct transcription of the Ghidra decompile (malloc/free node-buffer growth, 10-bit signed coefficient unpack, memcpy raw blocks, 2-bit nibble pixel expansion, two helper calls); diverges at insn #4 in the prologue register-save order, not in control flow
// working copy - scaffold materialised by --work
// size      1136 bytes
// prototype 
// callers   1   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005D439F 0x005D4448 0x00644EF2 0x006470A6

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_5d3980  at 0x005D3980  (1136 bytes)
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

int __fastcall sub_5d3980(int *param_1, unsigned int *param_2) {
    int *piVar1;
    int iVar2, iVar11, iVar13;
    unsigned int uVar3, uVar4, uVar5, uVar6, uVar8, uVar9, uVar10, uVar15;
    void *pvVar7;
    unsigned int *puVar12, *puVar14, *puVar16, *puVar18;
    unsigned char *puVar17;
    int flagOK;

    iVar13 = param_1[399];
    flagOK = 1;
    param_1[399] = param_1[0x18e];
    param_1[0x18e] = iVar13;
    uVar3 = *(unsigned int *)((int)param_2 + -2) >> 0x10;
    param_1[0x188] = uVar3;
    uVar4 = *param_2 >> 0x10;
    param_1[0x189] = uVar4;
    uVar5 = *(unsigned int *)((int)param_2 + 2) >> 0x10;
    param_1[0x18a] = uVar5;
    uVar6 = param_2[1] >> 0x10;
    param_1[0x18b] = uVar6;
    if (uVar3 != 0) {
        if (param_1[0x18c] < (int)uVar3) {
            if (param_1[0x193] != 0) {
                free((void *)param_1[0x193]);
                param_1[0x193] = 0;
                param_1[0x18c] = 0;
            }
            if (param_1[0x194] != 0) {
                free((void *)param_1[0x194]);
                param_1[0x194] = 0;
            }
        }
        if (param_1[0x193] == 0) {
            pvVar7 = malloc(uVar3 * 8);
            param_1[0x193] = (int)pvVar7;
            param_1[0x18c] = uVar3;
        }
        if (param_1[0x194] == 0) {
            pvVar7 = malloc(uVar3 * 4);
            param_1[0x194] = (int)pvVar7;
            param_1[0x18c] = uVar3;
        }
        flagOK = param_1[0x193] != 0;
        if (param_1[0x194] == 0) {
            flagOK = 0;
        }
    }
    iVar13 = uVar5 + uVar4;
    if (iVar13 != 0) {
        if ((param_1[0x18d] < iVar13) && (param_1[0x192] != 0)) {
            free((void *)param_1[0x192]);
            param_1[0x192] = 0;
        }
        if (param_1[0x192] == 0) {
            pvVar7 = malloc(iVar13 * 0x10);
            param_1[0x192] = (int)pvVar7;
            param_1[0x18d] = iVar13;
        }
        if (param_1[0x192] == 0) {
            return 0;
        }
    }
    if (flagOK != 0) {
        uVar10 = uVar3 * 2;
        uVar15 = uVar3 * 0x14;
        puVar12 = (unsigned int *)(param_1[0x193] + uVar3 * 8);
        uVar8 = uVar3;
        while (uVar8 != 0) {
            uVar15 = uVar15 - 10;
            uVar8 = *(unsigned int *)((int)param_2 + (uVar15 >> 3) + 0xc) >> (uVar15 & 7);
            uVar9 = uVar8 & 0x3ff;
            if ((uVar8 & 0x200) != 0) {
                uVar9 = uVar9 | 0xfffffc00;
            }
            puVar12 = puVar12 - 1;
            *puVar12 = uVar9;
            uVar10 = uVar10 - 1;
            uVar8 = uVar10;
        }
        iVar13 = *(int *)(*param_1 + 0x60);
        iVar2 = param_1[0x191];
        iVar11 = 0;
        if (uVar3 != 0) {
            do {
                piVar1 = (int *)(param_1[0x193] + iVar11 * 8);
                *(int *)(param_1[0x194] + iVar11 * 4) =
                    *(int *)(iVar2 + iVar13 * 4 + piVar1[1] * 4) + *piVar1;
                iVar11 = iVar11 + 1;
            } while (iVar11 < (int)uVar3);
        }
        puVar14 = (unsigned int *)((int)param_2 + (((int)(uVar3 * 0x14 + 0x1f) >> 3) & 0xfffffffc) + 0xc);
        if (uVar4 != 0) {
            memcpy((void *)param_1[0x192], puVar14, uVar4 * 0x10);
        }
        puVar14 = puVar14 + uVar4 * 4;
        if (uVar5 != 0) {
            puVar17 = (unsigned char *)(param_1[0x192] + uVar4 * 0x10);
            puVar16 = puVar14;
            for (uVar3 = uVar5; uVar3 != 0; uVar3 = uVar3 - 1) {
                uVar4 = puVar16[1];
                puVar17[0xf] = *((unsigned char *)puVar16 + (uVar4 & 3));
                puVar17[0xe] = *((unsigned char *)puVar16 + (uVar4 >> 2 & 3));
                puVar17[0xd] = *((unsigned char *)puVar16 + (uVar4 >> 4 & 3));
                puVar17[0xc] = *((unsigned char *)puVar16 + (uVar4 >> 6 & 3));
                puVar17[0xb] = *((unsigned char *)puVar16 + (uVar4 >> 8 & 3));
                puVar17[10] = *((unsigned char *)puVar16 + (uVar4 >> 10 & 3));
                puVar17[9]  = *((unsigned char *)puVar16 + (uVar4 >> 0xc & 3));
                puVar17[8]  = *((unsigned char *)puVar16 + (uVar4 >> 0xe & 3));
                puVar17[7]  = *((unsigned char *)puVar16 + (uVar4 >> 0x10 & 3));
                puVar17[6]  = *((unsigned char *)puVar16 + (uVar4 >> 0x12 & 3));
                puVar17[5]  = *((unsigned char *)puVar16 + (uVar4 >> 0x14 & 3));
                puVar17[4]  = *((unsigned char *)puVar16 + (uVar4 >> 0x16 & 3));
                puVar17[3]  = *((unsigned char *)puVar16 + (uVar4 >> 0x18 & 3));
                puVar17[2]  = *((unsigned char *)puVar16 + (uVar4 >> 0x1a & 3));
                puVar17[1]  = *((unsigned char *)puVar16 + (uVar4 >> 0x1c & 3));
                puVar17[0]  = *((unsigned char *)puVar16 + (uVar4 >> 0x1e));
                puVar16 = puVar16 + 2;
                puVar17 = puVar17 + 0x10;
            }
        }
        uVar3 = (unsigned int)(*(int *)(*param_1 + 100) * *(int *)(*param_1 + 0x60)) >> 4;
        if ((uVar6 * uVar3 + 0x1f & 0xffffffe0) != 0) {
            sub_5d439f((unsigned int *)param_1[400], (int)(puVar14 + uVar5 * 2), uVar3, uVar6);
        }
        uVar3 = *(unsigned int *)(*param_1 + 100);
        sub_5d4448((int *)param_1[400], param_1[399], param_1[0x18e], param_1[0x188], (int *)param_1[0x194],
                    param_1[0x192], uVar3, uVar3 >> 2, *(unsigned int *)(*param_1 + 0x60) >> 2);
    }
    return flagOK;
}

// ORIGINAL: 0x005D1B88 sub_5d1b88 0x005D1B88-0x005D2037 FILE
// symbol    ?sub_5d1b88@@YAXPAIPAH1@Z
// TRIED: homed into general.cpp (its real TU) 2026-09-03 and reverted for one reason:
// its fixed-address bindings have no naming evidence. image_data.py reads
// 6 of them and every one is zero at load - runtime-filled lookup tables
// with no .data content, no string, no arithmetic identity beyond 'a table
// indexed here' - and none is modelled anywhere in src. The homing-first rule
// does not ask for them to be invented: an unclaimed body with evidence-free
// bindings waits for the class pass that measures it in context.
// TRIED: no conventional signature can match - the original has NO prologue/epilogue and reads esi/edi/ebp as inputs set up by the CALLER (Ghidra: unaff_ESI/unaff_EDI/unaff_EBP), which is not a calling convention VC6 can target without __asm/naked (barred). Landed as a 3-pointer __cdecl function with an 8-iteration loop reproducing the exact table math (16 unrolled blocks in the original, all structurally identical) so the algorithm is faithfully documented even though the prologue can never match.
// working copy - scaffold materialised by --work
// size      1199 bytes
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_5d1b88  at 0x005D1B88  (1199 bytes)
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

void __cdecl sub_5d1b88(unsigned int *outp, int *srcESI, int *srcEDI) {
    int i;
    for (i = 0; i < 8; i++) {
        int iVar1 = ((int *)MovieBlendTableQuant)[srcEDI[i]];
        int iVar2 = ((int *)MovieBlendTableQuant2)[srcEDI[0x40 + i]];
        unsigned int uVar4 = (unsigned int)(iVar1 + ((int *)MovieBlendTableData)[srcESI[2 * i]] + iVar2);
        outp[2 * i] = ((unsigned int *)MovieBlendTableMid)[(uVar4 & 0xff800) >> 0xb] |
                      ((unsigned int *)MovieBlendTableHi)[uVar4 >> 0x17] |
                      ((unsigned int *)MovieBlendTableLow)[uVar4 & 0x1ff];
        uVar4 = (unsigned int)(((int *)MovieBlendTableData)[srcESI[2 * i + 1]] + iVar2 + iVar1);
        outp[2 * i + 1] = ((unsigned int *)MovieBlendTableHi)[uVar4 >> 0x17] |
                          ((unsigned int *)MovieBlendTableLow)[uVar4 & 0x1ff] |
                          ((unsigned int *)MovieBlendTableMid)[(uVar4 & 0xff800) >> 0xb];
    }
}

// ORIGINAL: 0x005D2536 sub_5d2536 0x005D2536-0x005D271D FILE
// symbol    ?sub_5d2536@@YAXPAH00@Z
// TRIED: homed into general.cpp (its real TU) 2026-09-03 and reverted for one reason:
// its fixed-address bindings have no naming evidence. image_data.py reads
// 1 of them and every one is zero at load - runtime-filled lookup tables
// with no .data content, no string, no arithmetic identity beyond 'a table
// indexed here' - and none is modelled anywhere in src. The homing-first rule
// does not ask for them to be invented: an unclaimed body with evidence-free
// bindings waits for the class pass that measures it in context.
// TRIED: no C++ calling convention passes edi/esi/ebp as params with no prologue; __asm is banned, so the register-implicit entry (no push/pop, bare ret) cannot be reproduced byte-exact. Reshaped as three pointer params for a faithful (non-exact) transcription.
// size      487 bytes
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_5d2536  at 0x005D2536  (487 bytes)
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

static DWORD movie_coefficient_pack_word(int index) {
    DWORD value;
    const BYTE *owner = reinterpret_cast<const BYTE *>(
        &movie_media_conversion_tables);
    const DWORD byte_offset = 0x90c + (DWORD)index * 8;
    memcpy(&value, owner + byte_offset, sizeof(value));
    return value;
}

void __cdecl sub_5d2536(int *coeff_a, int *coeff_b, int *out_words) {
    for (int index = 0; index != 8; ++index) {
        const DWORD upper_left = movie_coefficient_pack_word(coeff_a[0x40 + index]);
        const DWORD upper_right = movie_coefficient_pack_word(coeff_b[1 + index * 2]);
        const DWORD lower_left = movie_coefficient_pack_word(coeff_a[index]);
        const DWORD lower_right = movie_coefficient_pack_word(coeff_b[index * 2]);
        const DWORD upper = (upper_left << 8) | upper_right;
        const DWORD lower = (lower_left << 8) | lower_right;
        const DWORD result = (upper << 16) | lower;
        memcpy(&out_words[index], &result, sizeof(result));
    }
}

// ORIGINAL: 0x005CE6E3 sub_5ce6e3 0x005CE6E3-0x005CE85A FILE
// symbol    ?sub_5ce6e3@@YAXPAEPAFHPAH2@Z
// TRIED: homed into general.cpp (its real TU) 2026-09-03 and reverted for one reason:
// its fixed-address bindings have no naming evidence. image_data.py reads
// 5 of them and every one is zero at load - runtime-filled lookup tables
// with no .data content, no string, no arithmetic identity beyond 'a table
// indexed here' - and none is modelled anywhere in src. The homing-first rule
// does not ask for them to be invented: an unclaimed body with evidence-free
// bindings waits for the class pass that measures it in context.
// TRIED: original uses `pushal`/`popal` around the whole body (5 stack args, no `this`); no portable C source form reproduces that register-save shape, so #0 mismatches in the prologue.
// The preceding fixed-address/naming objection is superseded by the shared
// MediaAdpcmTables owner above: its complete initial bytes and observed state
// offsets are verified. The pushal/popal matching limitation remains.
// size      375 bytes
// prototype 
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_5ce6e3  at 0x005CE6E3  (375 bytes)
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

void __cdecl sub_5ce6e3(unsigned char *codes, short *samples, int sample_total, int *predictor_pair, int *step_index_pair) {
    int iVar2 = 0;
    *AdpcmStepIndexLeft = (int)(short)*step_index_pair;
    *AdpcmStepIndexRight = (int)(short)(*reinterpret_cast<unsigned int *>(step_index_pair) >> 16);
    *AdpcmPredictorLeft = (int)(short)*predictor_pair;
    *AdpcmPredictorRight = (int)(short)(*reinterpret_cast<unsigned int *>(predictor_pair) >> 16);
    do {
        unsigned char bVar1 = *codes;
        *AdpcmPredictorLeft += media_adpcm_delta(*AdpcmStepIndexLeft, bVar1 >> 4);
        if (*AdpcmPredictorLeft < 0x8000) {
            if (*AdpcmPredictorLeft < -0x8000) {
                *AdpcmPredictorLeft = -0x8000;
            }
        } else {
            *AdpcmPredictorLeft = 0x7fff;
        }
        *reinterpret_cast<short *>(reinterpret_cast<char *>(samples) + iVar2 * 4) =
            static_cast<short>(*AdpcmPredictorLeft);
        *AdpcmStepIndexLeft = *AdpcmStepIndexLeft + *reinterpret_cast<int *>(
            reinterpret_cast<char *>(AdpcmIndexAdjust) + ((bVar1 >> 4) & 7) * 4);
        if (*AdpcmStepIndexLeft < 0x1601) {
            if (*AdpcmStepIndexLeft < 0) {
                *AdpcmStepIndexLeft = 0;
            }
        } else {
            *AdpcmStepIndexLeft = 0x1600;
        }
        bVar1 = *codes;
        *AdpcmPredictorRight += media_adpcm_delta(*AdpcmStepIndexRight, bVar1 & 0x0f);
        if (*AdpcmPredictorRight < 0x8000) {
            if (*AdpcmPredictorRight < -0x8000) {
                *AdpcmPredictorRight = -0x8000;
            }
        } else {
            *AdpcmPredictorRight = 0x7fff;
        }
        *reinterpret_cast<short *>(reinterpret_cast<char *>(samples) + 2 + iVar2 * 4) =
            static_cast<short>(*AdpcmPredictorRight);
        *AdpcmStepIndexRight = *AdpcmStepIndexRight + *reinterpret_cast<int *>(
            reinterpret_cast<char *>(AdpcmIndexAdjust) + (bVar1 & 0x07) * 4);
        if (*AdpcmStepIndexRight < 0x1601) {
            if (*AdpcmStepIndexRight < 0) {
                *AdpcmStepIndexRight = 0;
            }
        } else {
            *AdpcmStepIndexRight = 0x1600;
        }
        iVar2 = iVar2 + 1;
        codes = codes + 1;
    } while (iVar2 < sample_total);
    *predictor_pair = ((*AdpcmPredictorRight & 0xffff) << 16) | (*AdpcmPredictorLeft & 0xffff);
    *step_index_pair = ((*AdpcmStepIndexRight & 0xffff) << 16) | (*AdpcmStepIndexLeft & 0xffff);
}

// ---------------------------------------------------------------------------
// THE ENEMY AI MOVE DRIVER (0x00579240 / 0x00579510), homed 2026-09-06.
//
// enemy_turn sweeps the vehicle table ten times, once per priority band, and
// hands each of the faction's vehicles to enemy_veh; enemy_veh drives one
// vehicle's move and reports whether the turn moved on. enemy_move (the
// per-vehicle mover both of them reach) is still the image's own bytes and is
// forwarded from pending_bodies.cpp.
// ---------------------------------------------------------------------------

char *const LogEnemyVehicleDone = (char *)0x0068EAFC;    // "Enemy Vehicle DONE"
char *const LogEnemyVehicleAction = (char *)0x0068EB10;  // "Enemy Vehicle ACTION"
char *const LogEnemyVehicle = (char *)0x0068EB28;        // "Enemy Vehicle"

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

    bool is_local_turn;
    if (Vehs[unit_id].faction_id == ConsoleGlobal.faction_id_ && IsMultiplayerNet != 0) {
        is_local_turn = true;
    } else {
        is_local_turn = false;
    }

    if (IsMultiplayerNet != 0) {
        NetDaemonNet.lock_state_[1].leading_words_[3] = 3;
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
                    Vehs[unit_id].faction_id)
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
        do {
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
                } else if (priority < 7) {
                    matched = true;
                }

                if (matched && !has_abil(veh->proto_id, ABL_DROP_POD)
                        && VehPrototypes[veh->proto_id].plan != 4
                        && VehPrototypes[veh->proto_id].unk_1 != 0x15) {
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
            --veh_id;
            --veh;
        } while (veh_id >= 0);
    }

    PlayersData[faction_id].flags &= ~PFLAG_UNK_10000;
}

// ORIGINAL: 0x00584D30 sub_584d30 0x00584D30-0x00584D52 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:5167ec56aaa9031de73f42c05fbc3bdd0bad0201:src/recovered/00584d30.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x584d30 complete original body; independent source-owner review.", "source:src/general.cpp and corresponding header own real receiver/dependencies; count-zero copy underflow is intentionally unchanged."], "reason": "Complete typed application helper; retain original call ordering, argument contract and historical obligation."}
// symbol    ?sub_584d30@@YAPADPAD0I@Z
char *__cdecl sub_584d30(char *dest, char *src, unsigned int count) {
    char *result = strncpy(dest, src, count);
    dest[count - 1] = 0;
    return result;
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

// Native592D70/592D90 prove four-byte accesses only. This behavior-derived
// scalar home does not assert the complete original allocation or lifetime.
// RECOVERY_OBJECT: v1 0x00945AEC ?StoredScaleAdjustment@@3HA
int StoredScaleAdjustment = 0;

// ORIGINAL: 0x00592D70 sub_592d70 0x00592D70-0x00592D88 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical seed:src/recovered/00592d70.cpp", "task:owned-helper-coverage-batch", "Native592D70 writes945AEC, clamps nonpositive inputs to zero and returns the stored value. Names describe observed behavior only; containing allocation and lifecycle remain unknown."], "reason": "Recover a complete native helper through genuine source owners and SDK calls, preserving original identity, extent and historical obligations. Source home is not proof of original TU or complete-game acceptance."}
// symbol    ?set_scale_adjustment@@YAHH@Z
int __cdecl set_scale_adjustment(int value) {
    StoredScaleAdjustment = value;
    if (value <= 0) {
        value = 0;
        StoredScaleAdjustment = value;
    }
    return value;
}

// ORIGINAL: 0x00592D90 sub_592d90 0x00592D90-0x00592DAA BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical seed:src/recovered/00592d90.cpp", "task:owned-helper-coverage-batch", "Native592D90 reads945AEC, wraps(add16)*input at32bits and divides the signed product by16 toward zero. memcpy preserves signed representation without undefined C++ signed-overflow arithmetic. Seed zero-argument catalogue conflicts with observed [ebp+8] input; retain finding."], "reason": "Recover a complete native helper through genuine source owners and SDK calls, preserving original identity, extent and historical obligations. Source home is not proof of original TU or complete-game acceptance."}
// symbol    ?apply_scale_adjustment@@YAHH@Z
int __cdecl apply_scale_adjustment(int value) {
    // Original add/imul wrap at32bits before signed division truncates to zero.
    const uint32_t bits = (static_cast<uint32_t>(StoredScaleAdjustment) + 16u)
        * static_cast<uint32_t>(value);
    int product;
    memcpy(&product, &bits, sizeof(product));
    return product / 16;
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
    uint32_t wrapped = static_cast<uint32_t>(a1) + 0x3fffffffU;
    int32_t biased_angle;
    std::memcpy(&biased_angle, &wrapped, sizeof(biased_angle));
    return sin(biased_angle, a2);
}
