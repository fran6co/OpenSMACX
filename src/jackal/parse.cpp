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
#include "strings.h"
#include "graphics_language.h"  // Language

// Inferred TU 0x00625880-0x00625F1A: after Random, ended by the Logging
// initializer at 0x00625F20. Its .bss (0x009BB570-0x009BBFF4) sits between
// Rand and Logging, and part_speech's .data before Log's. It may instead be
// the tail of the Random TU.

// Extent: compiled sizeof 0x28; next mapped object at 0x009BB598.
// RECOVERY_OBJECT: v2 0x009BB570 0x009BB598 ?ParseStrPlurality@@3PAHA 0x00625B42
int ParseStrPlurality[10];  // 0x009BB570; parse-family table, zero-initialized.
// RECOVERY_OBJECT: v2 0x009BB598 0x009BB5C0 ?ParseNumTable@@3PAHA 0x00625E30
int ParseNumTable[10];      // 0x009BB598
// Extent: compiled sizeof 0x28; next mapped object at 0x009BB5E8.
// RECOVERY_OBJECT: v2 0x009BB5C0 0x009BB5E8 ?ParseStrGender@@3PAHA 0x00625B3B
int ParseStrGender[10];     // 0x009BB5C0
// Extent: compiled sizeof 0xa00; next mapped object 0x009BBFEC, gap unreferenced.
// RECOVERY_OBJECT: v2 0x009BB5E8 0x009BBFE8 ?ParseStrBuffer@@3PAUchar256@@A 0x00409B5E
char256 ParseStrBuffer[10]; // 0x009BB5E8
// The default noun forms parse_set/parse_say/parse_says/get_noun fall back to,
// at 0x009BBFEC and 0x009BBFF0 in the shipped image - REAL OBJECTS (zero at
// load there, as here; see general.h for the conversion from the `static
// T *const` bindings and the measurement behind it).
// RECOVERY_OBJECT: v2 0x009BBFEC 0x009BBFF0 ?GenderDefault@@3HA 0x0055D190
int GenderDefault;       // 0x009BBFEC
// RECOVERY_OBJECT: v2 0x009BBFF0 0x009BBFF4 ?PluralityDefault@@3HA 0x0055D190
BOOL PluralityDefault;   // 0x009BBFF0

// RECOVERY_ASSOCIATED: v1 switch_targets 0x00625880 0x00625DE0 0x00625DF8 0x006258E3
// RECOVERY_ASSOCIATED: v1 switch_selector 0x00625880 0x00625DF8 0x00625E23 0x006258DD
/*
Purpose: This handles parsing the input string and storing it in the output.
// ORIGINAL: 0x00625880 ?parse_string@@YAHPADPAD@Z 0x00625880-0x00625DDD BYTE_EXACT
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
    if (!input || !output) {
        return 3;
    }
    LPSTR output_base = output;
    LPSTR var;
    LPSTR num;
    int number;
    int len;
    // Three separate buffers: the image's $NUMBER, $NUM and $HEX conversions
    // use distinct frame arrays.
    char number_text[80];
    char num_text[80];
    char hex_text[80];
    while (1) {
        // RECOVERY_LITERAL: v1 0x0069734C 0x0069734E $SG18204 0x00625880
        var = strstr(input, "$");
        if (!var) {
            break;
        }
        LPSTR next = var + 1;
        switch (*next) {
          case 'N':
            // RECOVERY_LITERAL: v1 0x00697350 0x00697358 $SG18213 0x00625880
            if (!strncmp(var, "$NUMBER", 7)) {
                number = var[7] - '0';
                if (number > 9) {
                    return 14; // parse error
                }
                len = var - input;
                memcpy(output, input, len);
                output += len;
                *output = 0;
                input = var + 8;
                _itoa(ParseNumTable[number], number_text, 10);
                strcat(output, number_text);
                output += strlen(output);
                continue;
            // RECOVERY_LITERAL: v1 0x00697358 0x0069735D $SG18217 0x00625880
            } else if (!strncmp(var, "$NUM", 4)) {
                number = var[4] - '0';
                if (number > 9) {
                    return 14; // parse error
                }
                len = var - input;
                memcpy(output, input, len);
                output += len;
                *output = 0;
                input = var + 5;
                _itoa(ParseNumTable[number], num_text, 10);
                strcat(output, num_text);
                output += strlen(output);
                continue;
            } else {
                num = findnum(var);
                if (!num) {
                    break;
                }
                number = num[0] - '0';
                if (number > 9) {
                    return 14; // parse error
                }
                len = var - input;
                memcpy(output, input, len);
                output += len;
                *output = 0;
                input = num + 1;
                strcat(output, ParseStrBuffer[number].str);
                output += strlen(output);
                continue;
            }
            break;

          case 'H':
            // RECOVERY_LITERAL: v1 0x00697360 0x00697365 $SG18224 0x00625880
            if (!strncmp(var, "$HEX", 4)) {
                // The image formats $HEX in decimal, like $NUM.
                number = var[4] - '0';
                if (number > 9) {
                    return 14; // parse error
                }
                len = var - input;
                memcpy(output, input, len);
                output += len;
                *output = 0;
                input = var + 5;
                _itoa(ParseNumTable[number], hex_text, 10);
                strcat(output, hex_text);
                output += strlen(output);
                continue;
            } else {
                num = findnum(var);
                if (!num) {
                    break;
                }
                number = num[0] - '0';
                if (number > 9) {
                    return 14; // parse error
                }
                len = var - input;
                memcpy(output, input, len);
                output += len;
                *output = 0;
                input = num + 1;
                strcat(output, ParseStrBuffer[number].str);
                output += strlen(output);
                continue;
            }
            break;

          case '$': {
            len = var - input;
            memcpy(output, input, len + 1);
            output += len + 1;
            input = var + 2;
            *output = 0;
            break;
          }

          case '<': {
            len = var - input;
            memcpy(output, input, len);
            output += len;
            *output = 0;
            // RECOVERY_LITERAL: v1 0x00697368 0x0069736A $SG18232 0x00625880
            LPSTR end_bracket = strstr(var, ">");
            if (!end_bracket) {
                input = next;
                break;
            }
            // A second pointer walks the alternatives: `var` keeps too few
            // references to outrank `output` for esi (MATCHING_GUIDE 2.7).
            LPSTR p = var + 2;
            input = end_bracket + 1;
            while (*p == ' ') {
                ++p;
            }
            number = -1;
            int plurality = 0;
            int gender; // The '#' route leaves this local unset.
            if (*p == 'M' || *p == 'm') {
                gender = 0;
            } else if (*p == 'F' || *p == 'f') {
                gender = 1;
            } else if (*p == 'N' || *p == 'n') {
                gender = 2;
            } else if (*p == '#') {
                ++p;
                number = *p - '0';
                plurality = ParseNumTable[number] != 1;
            } else {
                number = *p - '0';
                if (number >= 10) {
                    continue;
                }
                gender = ParseStrGender[number];
                plurality = ParseStrPlurality[number];
            }
            ++p;
            if (number < 0 && *p >= '0' && *p <= '9') {
                plurality = *p - '1';
                ++p;
            }
            while (*p == ' ') {
                ++p;
            }
            if (*p == ':') {
                ++p;
            }
            // The scan runs up to the text after '>', not to the bracket.
            int count = 0;
            for (LPSTR scan = p; scan < input; ++scan) {
                if (*scan == ':') {
                    ++count;
                }
            }
            if (!count && findnum(p)) {
                const int index = *findnum(p) - '0';
                if (index >= 10) {
                    continue;
                }
                p = ParseStrBuffer[index].str;
                for (LPSTR text = p; *text; ++text) {
                    if (*text == ':') {
                        ++count;
                    }
                }
            }
            if (count == 1) {
                count = plurality != 0;
            } else if (count == 2) {
                count = gender;
            } else if (count == 3) {
                count = (gender != 0) + (plurality != 0 ? 2 : 0);
            } else if (count == 5) {
                count = gender + (plurality != 0 ? 3 : 0);
            }
            // No terminator test: a short alternative list reads past it.
            while (count) {
                if (*p++ == ':') {
                    --count;
                }
            }
            while (*p && *p != ':' && *p != '>') {
                *output++ = *p++;
            }
            *output = 0;
            break;
          }

          case 'L':
            // RECOVERY_LITERAL: v1 0x0069736C 0x00697373 $SG18287 0x00625880
            if (!strncmp(var, "$LINK<", 6)) {
                len = var - input;
                memcpy(output, input, len + 1);
                output += len + 1;
                input = next;
                *output = 0;
                break;
            }
            // Any other $L... falls into the numbered-string route.
          default: {
            num = findnum(var);
            if (!num) {
                goto done;
            }
            number = num[0] - '0';
            if (number > 9) {
                return 14; // parse error
            }
            len = var - input;
            memcpy(output, input, len);
            output += len;
            *output = 0;
            input = num + 1;
            strcat(output, ParseStrBuffer[number].str);
            output += strlen(output);
            continue;
          }
        }
    }

done:
    strcat(output, input);

    if (Language == 1) { // French : handling for poor translations
        // Six inline 20-byte French prefix rows; the observed table window
        // does not establish any broader original allocation or source name.
        // The rows sit in writable .data (0x006972D4), so the static is not
        // const: a const local static would land in .rdata.
        // RECOVERY_OBJECT: v2 0x006972D4 0x0069734C _?part_speech@?IF@??parse_string@@YAHPAD0@Z@4PAY0BE@DA 0x00625880
        static char part_speech[6][20] = {
            " de ", " le ", " la ", "De ", "Le ", "La "
        };
        LPSTR first = output_base;
        do {
            LPSTR search_from = first;
            first = 0;
            int offset;
            for (int i = 0; i < 6; ++i) {
                LPSTR found = strstr(search_from, part_speech[i]);
                if (found && (!first || found < first)) {
                    offset = (i < 3) + 1;
                    first = found;
                }
            }
            if (!first) {
                break;
            }
            char next_char = first[offset + 2];
            if (next_char == 'a' || next_char == 'e' || next_char == 'i'
                || next_char == 'o' || next_char == 'u' || next_char == 'y'
                || next_char == 'h' || next_char == 'H' || next_char == 'A'
                || next_char == 'E' || next_char == 'I' || next_char == 'O'
                || next_char == 'U' || next_char == 'Y') {
                LPSTR elide = first + offset;
                *elide = '\'';
                char moved;
                do {
                    moved = elide[2];
                    *++elide = moved;
                } while (moved);
            }
            // The next scan starts one past this match.
        } while (++first);
    }

    return 0;
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
