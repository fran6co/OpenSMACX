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
#pragma once

 /*
  * General related objects, variables and functions.
  */
struct Filefind {
    char cd_path[256];
    char alt_path[256];
    char last_path[256];
    char exe_dir[256];
    char relative_path[256];
};

extern uint32_t ScenEditorUndoPosition;

// THESE WERE `extern T *Name;` AND THAT COST AN INSTRUCTION EVERY TIME.
//
// The address is a compile-time constant and always was; `extern` was the only
// thing hiding it, so `*GenderDefault = gender` had to load the pointer and
// store through it. Making the constant visible lets the compiler fold the
// indirection into a direct absolute store - which is what the original does.
// Measured on ?parse_set@@YAXHH@Z (0x005A58E0), three spellings of the same
// body scored together by `verify_recovered_function.py --dir`:
//
//   static int *const g = (int *)0x9BBFEC;   BYTE_EXACT, 22 of 22 bytes
//   extern int *g;                           MISMATCH #6, `pop` vs `mov`
//   extern int *const g;                     MISMATCH #6, `pop` vs `mov`
//
// and in the real translation unit, compiled with this repository's own VC6
// flags: `mov [0x9bbfec],eax` / `mov [0x9bbff0],ecx`, four instructions and no
// relocation, against six instructions and two relocations before.
//
// THE THIRD SPELLING IS WHY THE CENSUS STILL SAYS MISMATCH. It does not
// include this header; `src_declarations.py` re-derives a declaration from it
// and drops the initialiser (`statement.split("=", 1)[0]`), so the constant is
// hidden again in the unit the census measures. The improvement is real in the
// shipped object and invisible to that instrument until it stops re-externing
// a `static T *const` definition.
//
// Call sites are unchanged - `*GenderDefault` still reads and writes the same
// address. `static` is safe here only because nothing takes `&GenderDefault`
// itself; a header-scope `static` gives each translation unit its own copy, so
// any global whose POINTER address is taken must stay `extern`. It is also
// only safe for a plain load or store: a read-modify-write through a
// const-pointer spelling makes VC6 emit load/dec/store where the original has
// an in-place `dec`, so those stay `extern T Name;` (see
// tools/emit_translation_unit.py, decision 2).
static int *const GenderDefault = (int *)0x009BBFEC;
static BOOL *const PluralityDefault = (BOOL *)0x009BBFF0;

DLLEXPORT void __cdecl purge_trailing(LPSTR input);
DLLEXPORT void __cdecl purge_leading(LPSTR input);
DLLEXPORT void __cdecl purge_spaces(LPSTR input);
DLLEXPORT void __cdecl kill_lf(LPSTR str);
DLLEXPORT void __cdecl kill_nl(LPSTR str);
DLLEXPORT void __cdecl add_lf(LPSTR str);
DLLEXPORT int __cdecl range(int input, int min, int max);
DLLEXPORT LPVOID __cdecl mem_get_old(size_t size);
DLLEXPORT LPVOID __cdecl mem_get(size_t size);
DLLEXPORT FILE *__cdecl env_open_old(LPCSTR source, LPCSTR mode);
DLLEXPORT FILE *__cdecl env_open(LPCSTR source, LPCSTR mode);
DLLEXPORT void __cdecl parse_set(int gender, BOOL plurality);
DLLEXPORT int __cdecl parse_num(int id, int value);
DLLEXPORT int __cdecl parse_say(int id, int input, int gender, int pluralality);
DLLEXPORT int __cdecl parse_says(int id, LPCSTR input, int gender, int pluralality);
DLLEXPORT int __cdecl btoi(LPCSTR str);
DLLEXPORT int __cdecl htoi(LPCSTR str);
DLLEXPORT int __cdecl stoi(LPCSTR str);
DLLEXPORT LPSTR __cdecl findnum(LPSTR str);
DLLEXPORT BOOL __cdecl jackal_version_check(LPCSTR version);
DLLEXPORT char __cdecl filefind_cd_drive_letter();
DLLEXPORT void __cdecl filefind_set_alternative(LPCSTR path);
DLLEXPORT LPSTR __cdecl filefind_get(LPCSTR file_name);
DLLEXPORT int __cdecl bit_count(int bitfield);
DLLEXPORT uint32_t __cdecl bit_count_signed(int bitfield);
DLLEXPORT void __cdecl my_srand(int reseed);
DLLEXPORT void __cdecl swap(int *var1, int *var2);
DLLEXPORT void __cdecl swap(uint8_t *var1, uint8_t *var2);
DLLEXPORT int __cdecl fixed_div(int numerator, int denominator);
DLLEXPORT const char *__cdecl memrchr(LPCSTR start, LPCSTR end, char value);
DLLEXPORT int __cdecl quick_root(int input);
DLLEXPORT void __cdecl bitmask(int input, int *offset, int *mask);
DLLEXPORT uint8_t __cdecl checksum(char *input, int length, uint8_t seed);
DLLEXPORT uint32_t __cdecl checksum_password(LPCSTR password);
DLLEXPORT uint32_t __cdecl rnd(int bounds, LPSTR UNUSED(input) input);
DLLEXPORT void __cdecl danger(LPCSTR msg1, LPCSTR msg2, int num1, int num2, int num3);
DLLEXPORT void __cdecl kill_auto_save();
DLLEXPORT void __cdecl auto_save();
DLLEXPORT void __cdecl auto_save_debug();
DLLEXPORT void __cdecl load_undo(int type);
DLLEXPORT void __cdecl wipe_undo();
DLLEXPORT void __cdecl auto_undo();
DLLEXPORT void __cdecl header_check(LPSTR header, FILE *file);
DLLEXPORT void __cdecl header_write(LPCSTR header, FILE *file);
DLLEXPORT void __cdecl sort(int count, int *id, int *value);
DLLEXPORT void __cdecl sort_descending(uint32_t count, int *id, int *value);

// WIP
int __cdecl filefind_init(LPCSTR file_check, BOOL is_complete);
int __cdecl parse_string(LPSTR input, LPSTR output);
