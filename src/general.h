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

class Font;     // forward declaration
class Palette;  // forward declaration

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
// THE CENSUS SAID MISMATCH FOR TWO WEEKS, AND THE INSTRUMENT WAS WRONG. It
// does not include this header; `src_declarations.py` re-derived a declaration
// from it and dropped the initialiser (`statement.split("=", 1)[0]`), so the
// constant was hidden again in the unit the census measured, and a real
// improvement in the shipped object read as a defect in the body.
//
// Fixed 2026-08-15: `CONSTANT_ADDRESS` in that module carries a file-scope
// `static T *const NAME = (T *)0xLITERAL;` across verbatim, and `parse_set`
// measured BYTE_EXACT the moment it did. The narrowness is deliberate - an
// initialiser naming another symbol would demand that symbol's definition in
// a preamble built to have none - and both halves are held by
// `test_src_declarations.py`. The same fix is what closed the last
// instruction between `jackal_init_real` and its match, via `StringTable`.
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

// purge_trailing(LPSTR input) is defined at the end of this header - see the
// LEVER note there.
// Returns the advanced pointer, which the catalogued name spells `X`
// (void) - see the body. Callers that ignore it are the image's own.
LPSTR __cdecl purge_leading(LPSTR input);
void __cdecl purge_spaces(LPSTR input);
void __cdecl kill_lf(LPSTR str);
void __cdecl kill_nl(LPSTR str);
void __cdecl add_lf(LPSTR str);
LPVOID __cdecl mem_get_old(size_t size);
LPVOID __cdecl mem_get(size_t size);
FILE *__cdecl env_open_old(LPCSTR source, LPCSTR mode);
FILE *__cdecl env_open(LPCSTR source, LPCSTR mode);
int __cdecl parse_num(int id, int value);
int __cdecl parse_say(int id, int input, int gender, int pluralality);
int __cdecl parse_says(int id, LPCSTR input, int gender, int pluralality);
int __cdecl stoi(LPCSTR str);
LPSTR __cdecl findnum(LPSTR str);
BOOL __cdecl jackal_version_check(LPCSTR version);
// The Jackal engine's bring-up and teardown, called once each from WinMain.
// `jackal_init_real` returns ZERO on success - WinMain's `test eax, eax` at
// 0x0045FA50 jumps INTO the game when the result is zero and returns 0 from
// WinMain when it is not, which is the opposite sense of every neighbouring
// call and the one thing the first transcription of that body got backwards.
// `LPSTR` rather than `LPCSTR` for the window title because the image's own
// name for it - ?jackal_init_real@@YAHPAUPalette@@PAUFont@@PADHHHH@Z - spells
// that parameter `PAD`, and the mangling is what the byte match will look the
// body up by when 0x0062D3A0 lands.
int __cdecl jackal_init_real(Palette *palette, Font *font,
                                       LPSTR window_name, int tgl_direct_draw,
                                       int display_width, int display_height,
                                       int colour_depth);
void __cdecl jackal_close();

// Called once from jackal_init_real, between the FileWin and Cursor class
// bring-ups. `sub_63ce20` has no catalogued name - the image carries no
// symbols and IDA reconstructed none for it - so it keeps the address.
int __cdecl trig_init();
int __cdecl sub_63ce20();

// 0x009BC4B0. jackal_init_real stores its `tgl_direct_draw` argument here on
// entry and sets bit 0 once every subsystem has come up, so it is the engine's
// own state word: the mode it was asked for, plus "initialised".
extern int JackalInitFlags;
int __cdecl filefind_cd_drive_letter();
void __cdecl filefind_set_alternative(LPCSTR path);
LPSTR __cdecl filefind_get(LPCSTR file_name);
int __cdecl bit_count(int bitfield);
uint32_t __cdecl bit_count_signed(int bitfield);
void __cdecl my_srand(int reseed);
int __cdecl fixed_div(int numerator, int denominator);
const char *__cdecl memrchr(LPCSTR start, LPCSTR end, char value);
int __cdecl quick_root(int input);
uint8_t __cdecl checksum(char *input, int length, uint8_t seed);
uint32_t __cdecl checksum_password(LPCSTR password);
uint32_t __cdecl rnd(int bounds, LPSTR UNUSED(input) input);
void __cdecl danger(LPCSTR msg1, LPCSTR msg2, int num1, int num2, int num3);
void __cdecl kill_auto_save();
void __cdecl auto_save();
void __cdecl auto_save_debug();
void __cdecl load_undo(int type);
void __cdecl wipe_undo();
void __cdecl auto_undo();
void __cdecl header_check(LPSTR header, FILE *file);
void __cdecl header_write(LPCSTR header, FILE *file);
void __cdecl sort(int count, int *id, int *value);
void __cdecl sort_descending(uint32_t count, int *id, int *value);

// WIP
int __cdecl filefind_init(LPCSTR file_check, BOOL is_complete);
int __cdecl parse_string(LPSTR input, LPSTR output);

MEASURED inline int __cdecl range(int input, int min, int max) {
    if (input < min || max < min) {
        return min;
    }
    if (input > max) {
        return max;
    }
    return input;
}

MEASURED inline void __cdecl parse_set(int gender, BOOL plurality) {
    *GenderDefault = gender;
    *PluralityDefault = plurality;
}

MEASURED inline void __cdecl swap(uint8_t *var1, uint8_t *var2) {
    *var1 ^= *var2;
    *var2 ^= *var1;
    *var1 ^= *var2;
}

MEASURED inline void __cdecl swap(int *var1, int *var2) {
    *var1 ^= *var2;
    *var2 ^= *var1;
    *var1 ^= *var2;
}

MEASURED inline void __cdecl bitmask(int input, int *offset, int *mask) {
    *offset = input / 8;
    *mask = 1 << (input & 7);
}

// Non-inline forwarder to `bitmask` above: a handful of call sites in
// base.cpp (cost_factor, pop_goal, num_objectives) need a real
// `call 0x50ba00` the way the image emits there, unlike the rest of
// bitmask's 103 callers, most of which genuinely inline it. The E8 target
// is a relocation on both sides and is discounted, so this symbol's own
// name costs nothing.
void __cdecl bitmask_call(int input, int *offset, int *mask);

MEASURED inline int __cdecl btoi(LPCSTR str) {
    int result = 0;
    while (*str == '0' || *str == '1') {
        result = *str++ - '0' + 2 * result;
    }
    return result;
}

MEASURED inline int __cdecl htoi(LPCSTR str) {
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

// LEVER: this had a `// ORIGINAL:` marker and was `__forceinline` but defined
// in general.cpp, so it never satisfies `inline` for callers in OTHER
// translation units (alpha.cpp) - VC6 still emits a real `call` there. Moved
// here (MEASURED inline keeps its own claim measurable) so chas_name,
// weap_name and arm_name fold it in place, matching the image's inlined
// four-`strlen`-call body. See general.cpp for the ORIGINAL marker.
MEASURED inline void __cdecl purge_trailing(LPSTR input) {
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
