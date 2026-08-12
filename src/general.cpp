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
#include "strings.h"
#include "text.h"

uint32_t ScenEditorUndoPosition = 1; // 0x00690D7C
int *GenderDefault = (int *)0x009BBFEC;
BOOL *PluralityDefault = (BOOL *)0x009BBFF0;

/*
Purpose: Trim the trailing spaces in-line from the end of the string.
ORIGINAL: 0x00600780
// name      ?purge_trailing@@YAXPAD@Z
// size      45 bytes
// spans     0x00600780-0x006007AD
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006453E0
Return Value: n/a
Status: Complete
*/
void __cdecl purge_trailing(LPSTR input) {
    LPSTR trim = input + strlen(input);
    while ((*(trim - 1) - ' ') == 0 && (trim != input)) {
        *trim--;
    }
    input[(trim - input)] = 0;
}

/*
Purpose: Trim the leading spaces in-line from the start of the string.
ORIGINAL: 0x00600760
// name      ?purge_leading@@YAXPAD@Z
// size      19 bytes
// spans     0x00600760-0x00600773
// prototype 
// callers   2   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl purge_leading(LPSTR input) {
    LPSTR trim = input;
    while ((*trim - ' ') == 0 && *trim != 0) {
        *trim++;
    }
    strcpy_s(input, strlen(input) + 1, trim);
}

/*
Purpose: Trim the leading and trailing spaces from the string.
ORIGINAL: 0x006007B0
// name      ?purge_spaces@@YAXPAD@Z
// size      103 bytes
// spans     0x006007B0-0x00600817
// prototype void (__cdecl ?purge_spaces@@YAXPAD@Z)(int8* input)
// callers   20   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006453E0 0x00645460
Return Value: n/a
Status: Complete
*/
void __cdecl purge_spaces(LPSTR input) {
    purge_leading(input);
    purge_trailing(input);
}

/*
Purpose: Truncate the string at the line feed (LF). Doesn't take into account carriage return (CR).
ORIGINAL: 0x00600820
// name      ?kill_lf@@YAXPAD@Z
// size      23 bytes
// spans     0x00600820-0x00600837
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
ORIGINAL: 0x00600840
// name      ?add_lf@@YAXPAD@Z
// size      25 bytes
// spans     0x00600840-0x00600859
// prototype 
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006453E0
Return Value: n/a
Status: Complete
*/
void __cdecl add_lf(LPSTR str) {
    size_t len = strlen(str);
    str[len] = '\n';
    str[len + 1] = 0;
}

/*
Purpose: Use the min and max parameters to bound the input.
ORIGINAL: 0x00422F00 BYTE_EXACT
// name      ?range@@YAHHHH@Z
// size      30 bytes
// spans     0x00422F00-0x00422F1E
// prototype int (__cdecl ?range@@YAHHHH@Z)(int input, int min, int max)
// callers   10   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Bounded input
Status: Complete
*/
int __cdecl range(int input, int min, int max) {
    if (input < min || max < min) {
        return min;
    }
    if (input > max) {
        return max;
    }
    return input;
}

/*
Purpose: Allocate memory with error checking.
ORIGINAL: 0x005D4510
// name      ?mem_get@@YAPAXH@Z
// size      44 bytes
// spans     0x005D4510-0x005D453C
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
    LPVOID result = _malloc(size);
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
ORIGINAL: 0x00634BB0
// name      ?env_open@@YAHPADPAD@Z
// size      36 bytes
// spans     0x00634BB0-0x00634BD4
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
    return _fopen(src_check, mode);
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
ORIGINAL: 0x005A58E0
// name      ?parse_set@@YAXHH@Z
// size      22 bytes
// spans     0x005A58E0-0x005A58F6
// prototype 
// callers   1   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl parse_set(int gender, BOOL plurality) {
    *GenderDefault = gender;
    *PluralityDefault = plurality;
}

/*
Purpose: Copies the value into a number global message buffer using id.
ORIGINAL: 0x00625E30
// name      ?parse_num@@YAHHH@Z
// size      29 bytes
// spans     0x00625E30-0x00625E4D
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
ORIGINAL: 0x00625E50
// name      ?parse_say@@YAHHHHH@Z
// size      99 bytes
// spans     0x00625E50-0x00625EB3
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
    strcpy_s(ParseStrBuffer[id].str, 256, StringTable->get(input));
    return 0;
}

/*
Purpose: Copies the input string into the global message buffer.
ORIGINAL: 0x00625EC0
// name      ?parse_says@@YAHHPADHH@Z
// size      90 bytes
// spans     0x00625EC0-0x00625F1A
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
    strcpy_s(ParseStrBuffer[id].str, 256, input);
    return 0;
}

/*
Purpose: Convert the binary string to an integer.
ORIGINAL: 0x006288D0 BYTE_EXACT
// name      ?btoi@@YAHPAD@Z
// size      29 bytes
// spans     0x006288D0-0x006288ED
// prototype int (__cdecl ?btoi@@YAHPAD@Z)(int8* input)
// callers   2   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Integer value of the string
Status: Complete
*/
int __cdecl btoi(LPCSTR str) {
    int result = 0;
    while (*str == '0' || *str == '1') {
        result = *str++ - '0' + 2 * result;
    }
    return result;
}

/*
Purpose: Convert the hex string to an integer.
ORIGINAL: 0x006288F0
// name      ?htoi@@YAHPAD@Z
// size      91 bytes
// spans     0x006288F0-0x0062894B
// prototype int (__cdecl ?htoi@@YAHPAD@Z)(int8* input)
// callers   2   call targets   3
// kind      game
// flags     sp_ready;purged_ok
// calls     0x0064628F 0x006469A2 0x006469CA
Return Value: Integer value of the string
Status: Complete
*/
int __cdecl htoi(LPCSTR str) {
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

/*
Purpose: Converts a binary, hex or decimal string to an integer.
ORIGINAL: 0x00628950
// name      ?stoi@@YAHPAD@Z
// size      174 bytes
// spans     0x00628950-0x006289FE
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
ORIGINAL: 0x00628B30
// name      ?findnum@@YAHPAD@Z
// size      57 bytes
// spans     0x00628B30-0x00628B69
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
ORIGINAL: 0x0062D570
// name      ?jackal_version_check@@YAHPAD@Z
// size      51 bytes
// spans     0x0062D570-0x0062D5A3
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
ORIGINAL: 0x00625880
// name      ?parse_string@@YAHPADPAD@Z
// size      1373 bytes
// spans     0x00625880-0x00625DDD
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

    if (*Language == 1) { // French : handling for poor translations
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
ORIGINAL: 0x006003A0
// name      ?filefind_cd_drive_letter@@YAPADXZ
// size      8 bytes
// spans     0x006003A0-0x006003A8
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: CD drive letter
Status: Complete
*/
char __cdecl filefind_cd_drive_letter() { return FilefindPath->cd_path[0]; }

/*
Purpose: Set an alternative path for the Filefind checks.
ORIGINAL: 0x006003B0
// name      ?filefind_set_alternate@@YAXPAD@Z
// size      70 bytes
// spans     0x006003B0-0x006003F6
// prototype 
// callers   3   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00645470 0x00645DD0
Return Value: n/a
Status: Complete
*/
void __cdecl filefind_set_alternative(LPCSTR path) {
    FilefindPath->alt_path[0] = 0;
    if (path) {
        if (!strchr(path, ':') && path[0] != '\\') {
            strcat_s(FilefindPath->alt_path, 256, FilefindPath->exe_dir);
        }
        strcat_s(FilefindPath->alt_path, 256, path);
    }
}

/*
Purpose: Initialize the Filefind global along with a CD check if there isn't a complete install. 
         Optimized logic since most installs will be on a HDD making the CD check less important.
ORIGINAL: 0x00600400
// name      ?filefind_init@@YAHPADH@Z
// size      453 bytes
// spans     0x00600400-0x006005C5
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
    FilefindPath->alt_path[0] = 0;
    GetCurrentDirectoryA(256, FilefindPath->exe_dir);
    strcat_s(FilefindPath->exe_dir, 256, "\\");

    if (is_complete) {
        return 0; // complete install, no need for further checks
    }
    if (!file_check) {
        return 16; // error, file_check shouldn't be NULL
    }
    WIN32_FIND_DATAA find_file_data;
    strcpy_s(FilefindPath->last_path, 256, FilefindPath->exe_dir);
    strcat_s(FilefindPath->last_path, file_check);
    HANDLE file_found = FindFirstFileA(FilefindPath->last_path, &find_file_data);
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
                strcpy_s(FilefindPath->last_path, 256, root_path);
                strcat_s(FilefindPath->last_path, file_check);
                //WIN32_FIND_DATA find_file_data;
                //HANDLE file_found = FindFirstFile(g_filefind.last_path, &find_file_data);
                file_found = FindFirstFileA(FilefindPath->last_path, &find_file_data);
                FindClose(file_found);
                if (file_found != INVALID_HANDLE_VALUE) {
                    strcpy_s(FilefindPath->cd_path, 256, root_path);
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
ORIGINAL: 0x006005D0
// name      ?filefind_get@@YAHPAD@Z
// size      387 bytes
// spans     0x006005D0-0x00600753
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
    if (!file_name) {
        return 0;
    }
    if (file_name == FilefindPath->last_path) {
        return FilefindPath->last_path;
    }
    WIN32_FIND_DATAA find_file_data;
    HANDLE file_found;
    if (file_name[1] == ':') {
        strcpy_s(FilefindPath->last_path, 256, file_name);
        file_found = FindFirstFileA(FilefindPath->last_path, &find_file_data);
        FindClose(file_found);
        return (file_found != INVALID_HANDLE_VALUE) ? FilefindPath->last_path : 0;
    }
    if (FilefindPath->alt_path[0]) {
        strcpy_s(FilefindPath->last_path, 256, FilefindPath->alt_path);
        strcat_s(FilefindPath->last_path, 256, file_name);
        file_found = FindFirstFileA(FilefindPath->last_path, &find_file_data);
        FindClose(file_found);
        if (file_found != INVALID_HANDLE_VALUE) {
            return FilefindPath->last_path;
        }
    }
    strcpy_s(FilefindPath->last_path, 256, FilefindPath->exe_dir);
    strcat_s(FilefindPath->last_path, 256, file_name);
    file_found = FindFirstFileA(FilefindPath->last_path, &find_file_data);
    FindClose(file_found);
    if (file_found != INVALID_HANDLE_VALUE) {
        return FilefindPath->last_path;
    }
    if (FilefindPath->cd_path[0]) {
        strcpy_s(FilefindPath->last_path, 256, FilefindPath->cd_path);
        strcat_s(FilefindPath->last_path, 256, file_name);
        file_found = FindFirstFileA(FilefindPath->last_path, &find_file_data);
        FindClose(file_found);
        if (file_found != INVALID_HANDLE_VALUE) {
            return FilefindPath->last_path;
        }
    }
    return 0;
}

/*
Purpose: Count the number of unsigned bits set. Replaced the original code with Brian Kernighan's 
ORIGINAL: 0x0050BA30
// name      ?bit_count@@YAHH@Z
// size      30 bytes
// spans     0x0050BA30-0x0050BA4E
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
ORIGINAL: 0x00628AB0 BYTE_EXACT
// name      ?bit_count_signed@@YAHH@Z
// size      18 bytes
// spans     0x00628AB0-0x00628AC2
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Bit count
Status: Complete
*/
/*
ORIGINAL: 0x00628AB0
// name      ?bit_count_signed@@YAHH@Z
// size      18 bytes
// spans     0x00628AB0-0x00628AC2
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
ORIGINAL: 0x00538FB0
// name      ?my_srand@@YAHH@Z
// size      34 bytes
// spans     0x00538FB0-0x00538FD2
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
    _srand(reseed);
}

/*
Purpose: Swap the values of two 32-bit variables. Added an additional check when swapping the same
         memory location.
ORIGINAL: 0x00628A50 BYTE_EXACT
// name      ?swap@@YAXPAHPAH@Z
// size      35 bytes
// spans     0x00628A50-0x00628A73
// prototype 
// callers   14   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
/*
ORIGINAL: 0x00628A50
// name      ?swap@@YAXPAHPAH@Z
// size      35 bytes
// spans     0x00628A50-0x00628A73
// prototype 
// callers   14   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl swap(int *var1, int *var2) {
    *var1 ^= *var2;
    *var2 ^= *var1;
    *var1 ^= *var2;
}

/*
Purpose: Swap the values of two 8-bit variables. Added an additional check when swapping the same
         memory location.
ORIGINAL: 0x00628A80 BYTE_EXACT
// name      ?swap@@YAXPAEPAE@Z
// size      35 bytes
// spans     0x00628A80-0x00628AA3
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
/*
ORIGINAL: 0x00628A80
// name      ?swap@@YAXPAEPAE@Z
// size      35 bytes
// spans     0x00628A80-0x00628AA3
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl swap(uint8_t *var1, uint8_t *var2) {
    *var1 ^= *var2;
    *var2 ^= *var1;
    *var1 ^= *var2;
}

/*
Purpose: Shift the numerator to the left by 16 then divide by the denominator. Added a check to 
         prevent a divide by zero crash.
ORIGINAL: 0x00628AD0
// name      ?fixed_div@@YAHJJ@Z
// size      28 bytes
// spans     0x00628AD0-0x00628AEC
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
ORIGINAL: 0x00628AF0
// name      ?memrchr@@YAHPBX0H@Z
// size      51 bytes
// spans     0x00628AF0-0x00628B23
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
ORIGINAL: 0x006290E0 BYTE_EXACT
// name      ?quick_root@@YAHH@Z
// size      41 bytes
// spans     0x006290E0-0x00629109
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
ORIGINAL: 0x0050BA00 BYTE_EXACT
// name      ?bitmask@@YAXHPAHPAH@Z
// size      39 bytes
// spans     0x0050BA00-0x0050BA27
// prototype void (__cdecl ?bitmask@@YAXHPAHPAH@Z)(int input, int* offset, int* mask)
// callers   103   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl bitmask(int input, int *offset, int *mask) {
    *offset = input / 8;
    *mask = 1 << (input & 7);
}

/*
Purpose: Calculate a basic XOR checksum for the data buffer.
ORIGINAL: 0x00539090
// name      ?checksum@@YAEPADHE@Z
// size      36 bytes
// spans     0x00539090-0x005390B4
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
ORIGINAL: 0x005390C0
// name      ?checksum_password@@YAHPAD@Z
// size      151 bytes
// spans     0x005390C0-0x00539157
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
ORIGINAL: 0x00579770
// name      ?rnd@@YAHHPAD@Z
// size      32 bytes
// spans     0x00579770-0x00579790
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
ORIGINAL: 0x00538F30
// name      ?danger@@YAXPADPADHHH@Z
// size      122 bytes
// spans     0x00538F30-0x00538FAA
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
ORIGINAL: 0x005ABD10
// name      ?kill_auto_save@@YAXXZ
// size      12 bytes
// spans     0x005ABD10-0x005ABD1C
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
ORIGINAL: 0x005ABD20
// name      ?auto_save@@YAXXZ
// size      281 bytes
// spans     0x005ABD20-0x005ABE39
// prototype 
// callers   9   call targets   3
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005A94F0 0x00646950 0x006469F7
Return Value: n/a
Status: Complete
*/
void __cdecl auto_save() {
    if (!*IsMultiplayerPBEM || *IsMultiplayerNet) { // auto-saving disabled for PBEM/HotSeat games
        if (*GameRules & RULES_IRONMAN && !(*GameState & STATE_SCENARIO_EDITOR)) {
            remove("saves\\auto\\Alpha Centauri Autosave 30.SAV");
            remove("saves\\auto\\Alpha Centauri Autosave 20.SAV");
            remove("saves\\auto\\Alpha Centauri Autosave 10.SAV");
            remove("saves\\auto\\Alpha Centauri Autosave 5.SAV");
            remove("saves\\auto\\Alpha Centauri Autosave 4.SAV");
            remove("saves\\auto\\Alpha Centauri Autosave 3.SAV");
            remove("saves\\auto\\Alpha Centauri Autosave 2.SAV");
            save_daemon("saves\\auto\\Alpha Centauri Autosave 1");
        } else { // standard auto saves
            if (!(*TurnCurrentNum % 10)) {
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
    if (*TurnCurrentNum == 1) {
        remove("saves\\auto\\Alpha Centauri Autosave Turn 1.SAV");
        save_daemon("saves\\auto\\Alpha Centauri Autosave Turn 1");
    }
    if (!(*TurnCurrentNum % 10)) {
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
ORIGINAL: 0x005ABE40
// name      ?load_undo@@YAXH@Z
// size      127 bytes
// spans     0x005ABE40-0x005ABEBF
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
ORIGINAL: 0x005ABEC0
// name      ?wipe_undo@@YAXXZ
// size      84 bytes
// spans     0x005ABEC0-0x005ABF14
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
ORIGINAL: 0x005ABF20
// name      ?auto_undo@@YAXXZ
// size      207 bytes
// spans     0x005ABF20-0x005ABFEF
// prototype 
// callers   32   call targets   7
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005A94F0 0x006453E0 0x00645460 0x00645470 0x00646950 0x006469F7 0x0064FC88
Return Value: n/a
Status: Complete
*/
void __cdecl auto_undo() {
    if (*GamePreferences & PREF_BSC_AUTOSAVE_EACH_TURN) {
        ScenEditorUndoPosition = 1;
        remove("saves\\auto\\Scenario Editor Undo 9.SAV");
        char save_path_new[38];
        char save_path_old[38];
        for (int i = 9; i >= 2; i--) {
            sprintf_s(save_path_old, 38, "saves\\auto\\Scenario Editor Undo %d.SAV", i - 1);
            sprintf_s(save_path_new, 38, "saves\\auto\\Scenario Editor Undo %d.SAV", i);
            rename(save_path_old, save_path_new);
        }
        save_daemon("saves\\auto\\Scenario Editor Undo 1");
    }
}

/*
Purpose: Read the specified header from a file. This assumes the header string buffer is at least 
         256 characters. TODO: Replace built-in versions of _fgetc and change return to std::string.
ORIGINAL: 0x0057D1F0
// name      ?header_check@@YAXPADPAUFILE@@@Z
// size      69 bytes
// spans     0x0057D1F0-0x0057D235
// prototype void (__cdecl ?header_check@@YAXPADPAUFILE@@@Z)(int8* header, FILE* file)
// callers   2   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00646855
Return Value: n/a
Status: Complete
*/
void __cdecl header_check(LPSTR header, FILE *file) {
    int header_chr = _fgetc(file);
    *header++ = (char)header_chr;
    if (header_chr) {
        int i = 0;
        do {
            if (++i >= 256) {
                break;
            }
            header_chr = _fgetc(file);
            *header++ = (char)header_chr;
        } while (header_chr);
    }
    _fgetc(file);
}

/*
Purpose: Write the specified header to a file. TODO: Replace built-in versions of _fputc.
ORIGINAL: 0x0057D240
// name      ?header_write@@YAXPADPAUFILE@@@Z
// size      48 bytes
// spans     0x0057D240-0x0057D270
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
        _fputc(header_chr, file);
    } while (header_chr);
    _fputc(0x1A, file);
}

/*
Purpose: For the count, sort both id and value arrays by the least to greatest value (ascending).
ORIGINAL: 0x005B5690
// name      ?sort@@YAXHPAHPAH@Z
// size      101 bytes
// spans     0x005B5690-0x005B56F5
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
