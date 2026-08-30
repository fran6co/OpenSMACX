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

// THE DEFAULT NOUN FORMS, at 0x009BBFEC and 0x009BBFF0 in the shipped image -
// REAL OBJECTS, defined in general.cpp. They used to be
// `static T *const GenderDefault = (T *)0x009BBFEC` bindings: the address is
// a compile-time constant and always was, and the `*const` spelling folded the
// indirection into the image's own direct absolute access, which is what closed
// `parse_set` (measured 2026-08-15, three spellings scored together:
//
//   static int *const g = (int *)0x9BBFEC;   BYTE_EXACT, 22 of 22 bytes
//   extern int *g;                           MISMATCH #6, `pop` vs `mov`
//   extern int *const g;                     MISMATCH #6, `pop` vs `mov`
//
// - the two `extern` POINTER forms cost a pointer load per access; the object
// form keeps the direct access and only swaps the literal immediate for a
// relocated one, which is discounted like any data relocation). The addresses
// are terranx.exe's data, unmapped in a standalone build: every read and write
// through the binding corrupted live memory, so the objects now carry the
// storage and the call sites name it directly. `mov [0x9bbfec],eax` and
// `mov [GenderDefault],eax` are the same instruction; only the displacement's
// provenance differs.
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
extern int GenderDefault;       // 0x009BBFEC
extern BOOL PluralityDefault;   // 0x009BBFF0

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

// The fixed-point trig table bring-up (0x0063B940, BYTE_EXACT - marker and
// body in general.cpp). Builds SinTable below, then exercises the image's
// own int sin(angle, scale) twice for its return value.
int __cdecl trig_init();

// The fixed-point sine table trig_init builds at 0x009BE6DC: 256 truncated
// int samples of sin(i * 2pi/1020) scaled to 65535. The game's own
// `int sin(int angle, int scale)` (0x0063B9B0) interpolates adjacent entries
// - `SinTable[i]` and `SinTable[i + 1]` - so nothing else may write it.
extern int SinTable[256]; // 0x009BE6DC

// jackal_close's own callees that are not yet recovered anywhere else.
// Each is a pending_bodies forwarder; neither carries a catalogued name, the
// same as `sub_63ce20` above. Two former entries here were promoted out of
// pending_bodies.cpp: `sub_62d100` (now defined in general.cpp above
// jackal_close) and the 0x00635750 teardown, which is DDInit::teardown on
// the WinDisplayInit object itself - see win.h.
void __cdecl filewin_close_class();
void __cdecl basepop_close_class();

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
// BYTE_EXACT in general.cpp, spelled `alternative` there: its `// symbol`
// fact records that this tree emits ?filefind_set_alternative@@YAXPBD@Z
// where the image's own mangled name spells set_alternate/PAD. control_game
// (game.cpp) feeds it the savegame name before each reload.
void __cdecl filefind_set_alternative(LPCSTR path);
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
    GenderDefault = gender;
    PluralityDefault = plurality;
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
