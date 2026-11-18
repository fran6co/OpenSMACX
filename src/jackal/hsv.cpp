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
#include "palette.h"
#include "win.h"
#include "popup.h"
#include "basepop.h"
#include <math.h>
#include <cstring>

// Inferred original member(s) at 0x00628B70..0x00629109, between number
// parsing and StringBox. In address order: HSV_to_RGB, RGB_to_HSV (with the
// named constants it reads), the install-media check cd_check, and
// quick_root. No int3 edge separates them, so they may be one object or
// several; the original filename is unknown.

// HSV_to_RGB's six double constants, read at its operands. 0.0, 360.0 and 1.0
// are linker-folded with other copies (RGB_to_HSV's hsv_zero and
// hsv_hue_wrap_degrees below, random.cpp's 1.0), so this unit declares them
// merged.
// RECOVERY_LITERAL: v2 merged 0x0066D8D8 0x0066D8E0 __real@8@3fff8000000000000000 0x00628B70
// RECOVERY_LITERAL: v2 merged 0x0066FA70 0x0066FA78 __real@8@00000000000000000000 0x00628B70
// RECOVERY_LITERAL: v1 0x00670C20 0x00670C28 __real@8@40078000000000000000 0x00628B70
// RECOVERY_LITERAL: v1 0x00670C28 0x00670C30 __real@8@3ff98888888888888800 0x00628B70
// RECOVERY_LITERAL: v2 merged 0x00670C30 0x00670C38 __real@8@4007b400000000000000 0x00628B70
// RECOVERY_LITERAL: v1 0x00670C38 0x00670C40 __real@8@bfff8000000000000000 0x00628B70
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

// RGB_to_HSV's named double operands. 0x00670C48 and 0x00670C50 sit among
// HSV_to_RGB's own constants (0x00670C20..0x00670C38) in this member's
// .rdata, so the function and its constants belong here, not with Palette.
// 0.0 and 360.0 are the copies HSV_to_RGB's literals fold onto.
namespace {
// 0x0066FA70 and 0x00670C30 are bound once, by the merged HSV_to_RGB
// literals above: in one unit the named copy and the literal share the cell.
const double hsv_zero = 0.0;                        // 0x0066FA70
const double hsv_sector_offset_b_minus_r = 2.0;     // 0x00670C48
const double hsv_sector_offset_r_minus_g = 4.0;     // 0x0066FDC8
const double hsv_hue_wrap_degrees = 360.0;          // 0x00670C30
// RECOVERY_OBJECT: v1 0x00670C50 _hsv_value_byte_scale
const double hsv_value_byte_scale = 0.00390625;     // 1/256, 0x00670C50
}

// ORIGINAL: 0x00628DB0 ?RGB_to_HSV@@YAXPAUPALETTEENTRY@@PAUHSV@@@Z 0x00628DB0-0x00628F27 FILE BYTE_EXACT
// symbol    ?RGB_to_HSV@@YAXPBUtagPALETTEENTRY@@PAUHSV@@@Z
// size      375 bytes
// prototype 
// callers   3   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
// TRIED: 2026-08-24, from a 6/133 baseline to 96/133 (0.894 similar, best of all ten
//   flag sets) on three levers, each independently measured: (1) dropped the
//   `const double scale/zero` locals and the named `int ir/ig/ib` temporaries in
//   favour of `hsv_value_byte_scale`/`hsv_zero` and inline `entry->peRed` casts -
//   the named copies alone cost an extra callee-saved register (`push esi`) and
//   12 bytes more stack frame the image never pays for, even though they are
//   themselves `const` and should have folded; (2) wrote `out->h = ...;` directly
//   in each arm of the `if (r==vmax)/else if (g==vmax)/else` chain instead of
//   assigning a local `h` and storing it once after the chain - the image's
//   asymmetric tail (branch 1 stores-and-jumps-around, branches 2/3 share a
//   tail-merged store) only appears when each arm owns its own store; (3) replaced
//   `bool neg = h2 < zero; out->h = h2; if (neg) out->h = h2 + wrap;` with a direct
//   `if (h2 < hsv_zero)` on the raw comparison - the image never materialises the
//   condition as a byte (no `test al,al`), it stores h2 unconditionally then
//   conditionally re-stores. Two plateaus remain, both isolated to single spots:
//   the first vmax comparison (`(r<g)?g:r`) compiles `fld st(1); fcomp st(1)` where
//   the image has one `fcom st(1)` - tried `(g>r)?g:r`, `(g>=r)?g:r`, `(r<=g)?g:r`;
//   the `?g:r` forms regress hard (44-45/133, the frame-size problem returns), the
//   `<=` form is byte-identical to `<` (VC6 treats them the same for a pure
//   max-select). And `double h2 = out->h * hsv_degrees_per_sector;` folds the
//   CONSTANT into an explicit `fld` and the pointer read into the `fmul`'s memory
//   operand (`fld [0]; fmul [ecx]`) where the image folds the opposite way
//   (`fld [ecx]; fmul [0x670c40]`) - swapping the written multiplicand order
//   (`hsv_degrees_per_sector * out->h`) compiles byte-IDENTICAL to the unswapped
//   form, so this is VC6's own operand-selection heuristic, not a source-order
//   question this tree's spellings reach.
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00628DB0
// measured tier  MISMATCH
// divergence     2
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00628db0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?RGB_to_HSV@@YAXPAUPALETTEENTRY@@PAUHSV@@@Z  at 0x00628DB0  (375 bytes)
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

// 2026-11-06: exact with `>` max-selects, a ternary s, compound *=/+= on
//   out->h and literal 60.0 / 0.0 in the hue tail.
void __cdecl RGB_to_HSV(const PALETTEENTRY *entry, HSV *out) {
    if (out != 0 && entry != 0) {
        double r = static_cast<double>(static_cast<int>(entry->peRed)) * hsv_value_byte_scale;
        double g = static_cast<double>(static_cast<int>(entry->peGreen)) * hsv_value_byte_scale;
        double b = static_cast<double>(static_cast<int>(entry->peBlue)) * hsv_value_byte_scale;
        double vmax = (g > r) ? g : r;
        vmax = (b > vmax) ? b : vmax;
        double vmin = (g < r) ? g : r;
        vmin = (b < vmax) ? b : vmin;
        out->v = vmax;
        double s = (vmax != hsv_zero) ? (vmax - vmin) / vmax : hsv_zero;
        out->s = s;
        if (s == hsv_zero) {
            // The transcription wrote this as two int stores of 0x00000000
            // and 0xBFF00000 - which are exactly the bit halves of the
            // double -1.0. Say the constant; the compiler emits the halves.
            out->h = -1.0;
            return;
        }
        double delta = vmax - vmin;
        if (r == vmax) {
            out->h = (g - b) / delta;
        } else if (g == vmax) {
            out->h = (b - r) / delta + hsv_sector_offset_b_minus_r;
        } else {
            out->h = (r - g) / delta + hsv_sector_offset_r_minus_g;
        }
        // Literals: named const doubles reorder these two (fmul operand
        // order, fst/fcom order); 0x66FA70/0x670C40 are shared constants.
        out->h *= 60.0;
        if (out->h < 0.0) {
            out->h += hsv_hue_wrap_degrees;
        }
    }
}


/*
Purpose: The install-media check `DDInit::report_error` tails into. Scan
         drive letters A.. for a CD carrying directx\dsetup.dll, ask the
         player to insert it (`FILEFIND_NOCD`) when none has it, and when one
         does, LoadLibrary it and run the DirectXSetup entry points it
         exports (`DIRECTXNOVERSION` / `DIRECTXVERSION` popups report how
         that went). Returns 1 only when the setup run completed.
// ORIGINAL: 0x00628F30 sub_628f30 0x00628F30-0x006290E0 FILE BYTE_EXACT
// symbol    ?cd_check@@YAHXZ
// LEVER: seed find_handle from an unwritten local (`HANDLE seeded;`) and
//        compare the NOCD pop's answer against `(int)find_handle` rather than
//        -1 - that keeps find_handle live in esi through the test and is what
//        puts module/GetProcAddress/SetCurrentDirectoryA in the image's own
//        edi/esi registers (measured: 51/139 both with and without).
// TRIED: `--BaseAtKeyPollFlag` stores the decrement back; the image only
//        reads the slot (`mov eax,[0x9bc070]; dec eax; je`) - the store-back
//        spelling scores 53/139 but corrupts the shared slot, so the plain
//        read stands at 51/139.
// TRIED: `if (--response == 0) continue;` / `!`-form / `-= 1` / inverted
//        arm order / do-while bottom test - all 51/139; VC6 keeps the
//        `dec;test;jne;jmp` shape where the image has `dec;je` falling into a
//        mid-function shared return-0 epilogue (image 139, ours 141).
// LEVER: `switch (ConsoleGroupAction) { case 1: continue; }` gives dec;je.
// size      432 bytes
// prototype int __cdecl sub_628f30(void)
// callers   2   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00625E30 0x00627260 0x00645470
// indirect  0x00628F68 0x00628F92 0x00628F97 0x00628FF3 0x0062900B 0x00629019
//           0x00629026 0x0062908F 0x006290A0 0x006290BD 0x006290C0 0x006290CE
Return Value: 1 when DirectXSetupA ran; 0 otherwise
Status: Complete
*/
// The tree's name for 0x009BC070 (base.cpp), the retry budget this body
// decrements; base.h does not declare it, popup.cpp takes the same extern.
extern int ConsoleGroupAction;
int __cdecl cd_check() {
    int version;
    char root_path[256];      // 0x00628F4A: "A:\", then the dsetup.dll path
    char saved_dir[260];      // 0x104: GetCurrentDirectoryA's own buffer size
    WIN32_FIND_DATAA find_data;
    // BUG IN THE ORIGINAL: the image seeds esi (find_handle) from an
    // UNWRITTEN stack slot at entry (`mov esi,[esp + 0xc]`), so on a machine
    // with NO cd-rom drives at all the post-loop find_handle test reads
    // garbage and can enter the setup path unbidden. Reproduced, not fixed:
    // `seeded` is never written either, and the FILEFIND_NOCD test below
    // compares pop's response against find_handle (esi, holding
    // INVALID_HANDLE_VALUE on every path that reaches it) rather than against
    // -1, which is what keeps find_handle live in esi through that test.
    HANDLE seeded;
    HANDLE find_handle = seeded;
    int response;
    for (;;) {
        root_path[0] = 0;
        strcat(root_path, "A:\\");
        for (int drive = 0; drive < 26; drive++) {
            if (GetDriveTypeA(root_path) == DRIVE_CDROM) {
                strcat(root_path, "directx\\dsetup.dll");
                find_handle = FindFirstFileA(root_path, &find_data);
                FindClose(find_handle);
                if (find_handle != INVALID_HANDLE_VALUE) {
                    goto found;
                }
                // strip the appended path back to "A:\" for the next letter
                root_path[3] = 0;
            }
            root_path[0]++;
        }
        if (find_handle != INVALID_HANDLE_VALUE) {
            goto found;
        }
        if (pop_caption_title(const_cast<char *>("jackal"),
                              const_cast<char *>("FILEFIND_NOCD"), 0x10040,
                              nullptr) == (int)find_handle) {
            return 0;
        }
        // the pop system leaves the response in the shared slot at
        // 0x009BC070 (base.cpp's BaseAtKeyPollFlag); rescan while it is 1.
        switch (ConsoleGroupAction) {
        case 1:
            continue;
        }
        return 0;
    }
found:
    HMODULE module = LoadLibraryA(root_path);
    if (!module) {
        return 0;
    }
    FARPROC setup_a = GetProcAddress(module, "DirectXSetupA");
    if (!setup_a) {
        return 0;
    }
    FARPROC setup_get_version = GetProcAddress(module, "DirectXSetupGetVersion");
    if (!setup_get_version) {
        return 0;
    }
    // dsetup's entry points are __stdcall: the image leaves the two pushed
    // arguments for the callee to pop, and reads `version` through the
    // un-shifted [esp + 0x10] afterwards.
    typedef int(__stdcall *setup_get_version_fn)(int *, int);
    if ((reinterpret_cast<setup_get_version_fn>(setup_get_version))(&version,
                                                                    0) == 0) {
        response = pop_caption_title(const_cast<char *>("jackal"),
                                     const_cast<char *>("DIRECTXNOVERSION"),
                                     0x40, nullptr);
    } else {
        parse_num(0, version & 0xffff);
        response = pop_caption_title(const_cast<char *>("jackal"),
                                     const_cast<char *>("DIRECTXVERSION"), 0x40,
                                     nullptr);
    }
    if (response == -1) {
        return 0;
    }
    if (response == 1) {
        return 0;
    }
    root_path[3] = 0; // SetCurrentDirectory wants the drive root, not the dll
    GetCurrentDirectoryA(260, saved_dir);
    SetCurrentDirectoryA(root_path);
    // The pinned helper gates the HWND argument on the published default Win,
    // not on whether the process main-window handle has been created.
    typedef int(__stdcall *setup_a_fn)(HWND, int, int);
    if (BasePopDefaultField322C != 0) {
        (reinterpret_cast<setup_a_fn>(setup_a))(HandleMain, 0, 0x10018);
    } else {
        (reinterpret_cast<setup_a_fn>(setup_a))(0, 0, 0x10018);
    }
    FreeLibrary(module);
    SetCurrentDirectoryA(saved_dir);
    return 1;
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
