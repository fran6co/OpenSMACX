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
    // Truncate then `strcat`, exactly as `parse_says` does: `mov byte ptr
    // [esi], 0` at 0x00625E9D and `call 0x645470`, which is `strcat`.
    char *const dest = ParseStrBuffer[id].str;
    dest[0] = 0;
    strcat(dest, StringTable->get(input));
    return 0;
}

/*
Purpose: Copies the input string into the global message buffer.
// ORIGINAL: 0x00625EC0 ?parse_says@@YAHHPADHH@Z 0x00625EC0-0x00625F1A
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
    while ((*str < '0') || (*str > '9')) {
        if (*str == 0) {
            return 0;
        }
        str++;
    }
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
            strncpy_s(output, 1024, input, len);
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
                strncpy_s(output, 1024, input, len);
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
                strncpy_s(output, 1024, input, len);
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
                strncpy_s(output, 1024, input, len);
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
                strncpy_s(output, 1024, input, len);
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
                strncpy_s(output, 1024, input, len);
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
            strncpy_s(output, 1024, input, len);
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
                strncpy_s(output, 1024, input, len);
                output += len;
                *output = 0;
                input = num + 1;
                strcat_s(output, 1024, ParseStrBuffer[number].str);
                output += strlen(output);
            } else {
                int len = (var - input) + 1;
                strncpy_s(output, 1024, input, len);
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
            strncpy_s(output, 1024, input, len);
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
    FilefindPath.alt_path[0] = 0;
    GetCurrentDirectoryA(256, FilefindPath.exe_dir);
    strcat_s(FilefindPath.exe_dir, 256, "\\");

    if (is_complete) {
        return 0; // complete install, no need for further checks
    }
    if (!file_check) {
        return 16; // error, file_check shouldn't be NULL
    }
    WIN32_FIND_DATAA find_file_data;
    strcpy_s(FilefindPath.last_path, 256, FilefindPath.exe_dir);
    strcat_s(FilefindPath.last_path, file_check);
    HANDLE file_found = FindFirstFileA(FilefindPath.last_path, &find_file_data);
    FindClose(file_found);
    if (file_found != INVALID_HANDLE_VALUE) {
        return 0; // complete install on HDD, no need for CD
    }

    /*
    JACKAL_CLASS callBack = JACKAL_init();
    if(!callBack)
    return 4;
    */
    char root_path[5];
    do {
        strcpy_s(root_path, 5, "A:\\");
        for (int i = 0; i < 26; i++) {
            if (GetDriveTypeA(root_path) == DRIVE_CDROM) {
                // problem if drive was disconnected
                strcpy_s(FilefindPath.last_path, 256, root_path);
                strcat_s(FilefindPath.last_path, file_check);
                //WIN32_FIND_DATA find_file_data;
                //HANDLE file_found = FindFirstFile(g_filefind.last_path, &find_file_data);
                file_found = FindFirstFileA(FilefindPath.last_path, &find_file_data);
                FindClose(file_found);
                if (file_found != INVALID_HANDLE_VALUE) {
                    strcpy_s(FilefindPath.cd_path, 256, root_path);
                    // destroy JACKAL callBack
                    return 0;
                }
            }
            root_path[0]++;
        }
        // send FILEFIND_NOCD message -> if doesn't exist (removed from jackal.txt) -> exit
        // if user response is to retry -> loop around again
        // if user response is "ok", exit loop
    } while (0);
    // destroy JACKAL callBack
    return 0;
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
Return Value: the number of set bits
Status: Complete
*/
int __cdecl bit_count(int bitfield) {
    // Shift-and-accumulate, not Kernighan's clear-lowest-set-bit trick: the
    // original's shr/adc carry chain is this algorithm, and the committed
    // `n &= n - 1` form is a different one. Still not byte-exact - eleven
    // source shapes all land on the same `#2 push vs mov`, which is the
    // register-allocation class - but it is the right algorithm and it scores
    // strictly better.
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
// ORIGINAL: 0x00538FB0 ?my_srand@@YAHH@Z 0x00538FB0-0x00538FD2
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
    log_say("Reseed to", reseed, 0, 0);
    srand(reseed);
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
Purpose: Shift the numerator to the left by 16 then divide by the denominator. Added a check to 
         prevent a divide by zero crash.
// ORIGINAL: 0x00628AD0 ?fixed_div@@YAHJJ@Z 0x00628AD0-0x00628AEC
// symbol    ?fixed_div@@YAHHH@Z
// size      28 bytes
// prototype 
// callers   7   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Quotient
Status: Complete
*/
int __cdecl fixed_div(int numerator, int denominator) {
    if (!denominator) {
        return 0;
    }
    return ((int64_t)numerator << 16) / denominator;
}

/*
Purpose: Reverse string search for the last occurrence of the specified character. Replaced the
         original searching logic with strrchr() that does same thing. The end parameter can be 
         removed in the future.
// ORIGINAL: 0x00628AF0 ?memrchr@@YAHPBX0H@Z 0x00628AF0-0x00628B23
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
    if (!start || !end || start == end) {
        return 0;
    }
    return strrchr(start, value);
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


/*
Purpose: Calculate a basic XOR checksum for the data buffer.
// ORIGINAL: 0x00539090 ?checksum@@YAEPADHE@Z 0x00539090-0x005390B4
// size      36 bytes
// prototype 
// callers   2   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
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
// ORIGINAL: 0x005390C0 ?checksum_password@@YAHPAD@Z 0x005390C0-0x00539157
// symbol    ?checksum_password@@YAIPBD@Z
// size      151 bytes
// prototype 
// callers   2   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x006453E0 0x00645460
// indirect  0x005390F6
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
// ORIGINAL: 0x00579770 ?rnd@@YAHHPAD@Z 0x00579770-0x00579790
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
    return (bounds - 1 > 0) ? rand() % bounds : 0;
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
// ORIGINAL: 0x005ABE40 ?load_undo@@YAXH@Z 0x005ABE40-0x005ABEBF
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
    if (type == -1 && ScenEditorUndoPosition == 1) {
        return; // bug fix: skip redo if undo hasn't been triggered yet or on 1st undo
    }
    if (type < 0 && ScenEditorUndoPosition > 1) {
        ScenEditorUndoPosition--;
    }
    char load_path[38];
    sprintf_s(load_path, 38, "saves\\auto\\Scenario Editor Undo %d.SAV", ScenEditorUndoPosition);
    if (type > 0 && ScenEditorUndoPosition < 9) {
        ScenEditorUndoPosition++;
    }
    load_daemon(load_path, false);
    draw_map(true); // Bug fix: Map artifacts display issue; TODO: best method of refreshing map?
}

/*
Purpose: Remove all the existing Scenario Editor undo auto-saves.
// ORIGINAL: 0x005ABEC0 ?wipe_undo@@YAXXZ 0x005ABEC0-0x005ABF14
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
    char undo_path[38];
    for (int i = 9; i >= 1; i--) {
        sprintf_s(undo_path, 38, "saves\\auto\\Scenario Editor Undo %d.SAV", i);
        remove(undo_path);
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
// ORIGINAL: 0x0057D1F0 ?header_check@@YAXPADPAUFILE@@@Z 0x0057D1F0-0x0057D235
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
    int header_chr = fgetc(file);
    *header++ = (char)header_chr;
    if (header_chr) {
        int i = 0;
        do {
            if (++i >= 256) {
                break;
            }
            header_chr = fgetc(file);
            *header++ = (char)header_chr;
        } while (header_chr);
    }
    fgetc(file);
}

/*
Purpose: Write the specified header to a file. TODO: Replace built-in versions of _fputc.
// ORIGINAL: 0x0057D240 ?header_write@@YAXPADPAUFILE@@@Z 0x0057D240-0x0057D270
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
    int header_chr;
    do {
        header_chr = *header++;
        fputc(header_chr, file);
    } while (header_chr);
    fputc(0x1A, file);
}

/*
Purpose: For the count, sort both id and value arrays by the least to greatest value (ascending).
// ORIGINAL: 0x005B5690 ?sort@@YAXHPAHPAH@Z 0x005B5690-0x005B56F5
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
    // `int`, not `BOOL`. Identical type, but the verification scaffolding
    // forward-declares only types reachable from a signature, so a Windows
    // typedef on a LOCAL makes the whole body NO_COMPILE and unscoreable.
    int has_swapped;
    do {
        has_swapped = false;
        for (int i = 0; i < bounds; i++) {
            if (value[i] > value[i + 1]) {
                has_swapped = true;
                swap(&value[i], &value[i + 1]);
                swap(&id[i], &id[i + 1]);
            }
        }
    } while (has_swapped);
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
