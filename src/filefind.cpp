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
#include "basepop.h"

// Inferred TU 0x006003A0-0x00600859 (filefind_* and the string trimmers),
// between MCIVideo and BasePop; FilefindPath follows MCIVideo's .bss
// directly. No init/CC boundary on either side, so the extent is inferred.

// RECOVERY_IMPORT: v1 __imp__FindClose@4 KERNEL32.dll name:FindClose
// RECOVERY_IMPORT: v1 __imp__FindFirstFileA@8 KERNEL32.dll name:FindFirstFileA
// Source-owned Filefind storage; original last_path immediates identify
// &FilefindPath.last_path at 0x009B8198 + 0x200, not a pointer variable.
// RECOVERY_OBJECT: v1 0x009B8198 ?FilefindPath@@3UFilefind@@A
Filefind FilefindPath;  // 0x009B8198

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

    char root_path[12];
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
Purpose: Trim the trailing spaces in-line from the end of the string.
// ORIGINAL: 0x00600780 ?purge_trailing@@YAXPAD@Z 0x00600780-0x006007AD BYTE_EXACT
// body      src/filefind.cpp
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
