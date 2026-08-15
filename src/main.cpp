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
#include "main.h"
#include "alpha.h"          // prefs_get, prefs_put
#include "atexit_thunks.h"  // g_JACKAL_FONT
#include "caviar.h"
#include "font.h"
#include "game.h"     // control_game
#include "general.h"  // jackal_version_check, jackal_init_real, jackal_close
#include "palette.h"  // g_PALETTE1
#include "popup.h"     // Popup::alloc
#include "sound.h"    // init_sound
#include "temp.h"     // HandleMain

/*
 * NOTHING SUPPRESSES THE `strcat` INTRINSIC HERE, and nothing needs to. This
 * unit is an `/O1` one.
 *
 * `/O2` implies `/Oi`, which compiles the `strcat` below into an inlined
 * `repne scasb` walk - 38 instructions where 0x0045F967 is a five-byte
 * `call _strcat` - and drags the register saves into the prologue with it,
 * since the intrinsic needs esi and edi where the image defers all three
 * pushes to 0x0045F9A0. `/O1` implies no `/Oi`, so the same source emits the
 * call. Measured with capstone over the whole emitted symbol:
 *
 *     image                  142 instructions, calls _strcat
 *     /O2  (MEASURED_FLAGS)  180                inlined
 *     /O2  (FRAMELESS)       167                inlined
 *     /O1  (SIZE_FLAGS)      142                calls _strcat
 *     /O1  (SIZE_FRAMELESS)  142                calls _strcat
 *
 * tools/byte_match.py tries all four in order, so this is already what the
 * measurement sees; there was never a flag to fix or a pragma to add. A
 * `#pragma function(strcat)` sat here briefly and was removed: it bought
 * nothing at `/O1`, where the call is what the compiler emits anyway, and at
 * `/O2` it papered over a wrong flag set with a source edit.
 *
 * THE IMAGE IS A MIX OF `/O1` AND `/O2` UNITS, which is the fact underneath
 * all of this and the reason four flag sets exist. `_strcat` has 4,332 direct
 * call sites in .text, `_strlen` 875, `_memcpy` 130, `_memset` 73 - and yet
 * 19 of the BYTE_EXACT bodies contain an inlined `rep stosd` or `rep movsd`,
 * which only `/Oi` produces. Measured: adding `/Oi-` to all four flag sets
 * breaks ten of those nineteen and fixes nothing, because the answer is not
 * one setting for the program.
 */

/*
 * THE ENTRY POINT, and the top of the recovery.
 *
 * This file used to hold a `main()` that returned 0, so that `cmake --build`
 * answered "does the recovered tree compile, and does every symbol it names
 * resolve". It still answers that - every object is linked directly, so an
 * undefined symbol anywhere is a link error - but it now answers it about the
 * program's own entry point rather than a placeholder beside it.
 *
 * WHAT STILL DOES NOT RUN. Six of the eleven functions WinMain calls have
 * bodies in src/unrecovered/ and src/recovered/ that are not in this build
 * yet, and reach it through src/pending_bodies.cpp instead. Their transitive
 * closure is 1,489 catalogued addresses, so they arrive a subtree at a time
 * rather than all at once. Until then this links and starts and stops at the
 * first of them; that is a worse program than the shipped one and a better
 * signal than a binary that pretends to be finished.
 */

/*
 * The two candidate popup allocators, spelt as the addresses the image stores
 * because storing them is all WinMain does - neither is called from here.
 *
 *   0x00404FB0  ?alloc@Popup@@QAAHXZ
 *   0x00604E40  ?basepop_alloc@BasePop@@QAEHXZ
 */
// A MACRO, not a `const` variable, so the store below is an immediate. The
// image writes `mov dword ptr [0x696ecc], 0x604e40`; through a file-scope
// variable VC6 loads it first and the store becomes two instructions.
#define BASEPOP_ALLOC reinterpret_cast<func_popup_alloc *>(0x00604E40)

char CommandLineText[0x108];  // 0x007D3970
func_popup_alloc *PopupAllocHook;  // 0x00696ECC
int PopupModalActive;  // 0x009469FC
int DialogDefaultStyle = 3;  // 0x006970DC

/*
Purpose: Bring up the process - preferences, display, sound, fonts - run the
         game, and tear it back down.
ORIGINAL: 0x0045F950 FILE
// name      _WinMain@16
// size      459 bytes
// spans     0x0045F950-0x0045FB1B
// prototype int (__stdcall _WinMain@16)(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
// callers   1   call targets   11
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004C5CE0 0x0052AA30 0x0059DB40 0x0059E530 0x006185A0 0x00618D20 0x006190D0 0x0062D3A0 0x0062D500 0x0062D570 0x00645470
// indirect  0x0045F9DE 0x0045FAC9
Return Value: 1 once the game has run and shut down; 0 if any stage of the
              bring-up refuses.
Status: Complete

RULED-OUT: `reinterpret_cast<Caviar *>(0)->init_class()`. The image's names for
           Caviar::init_class and ::close_class end in `QAA` - a public member
           declared __cdecl - and 0x0045FA5D calls the first with no ecx set
           up at all, so there is no receiver and a non-static declaration
           makes the only legal spelling C2352. Calling through a null
           `Caviar *` was the way around that while caviar.h declared them as
           instance methods; they are `static` there now, which is the rule
           tools/emit_translation_unit.py already applies to every other
           `*::init_class`, so the cast is gone.

RULED-OUT: every source spelling tried for the ONE instruction still
           differing. This body is MNEMONIC_ONLY - every mnemonic agrees, in
           order - and 458 bytes against the image's 459. The whole of the
           remaining byte is #52:

               image  mov ebx, dword ptr [ebp + 0x10]     3 bytes
               here   mov ebx, edi                        2 bytes

           Both sides put `colour_depth` in the dead parameter's home slot and
           both read it into edi first; the image then RE-READS the slot for
           ebx where VC6 copies the register it already has. Tried and
           measured, all four MNEMONIC_ONLY at 458 bytes: assigning height
           before width, routing one of them through a second local, and
           reading `lpCmdLine` directly for both (that one is worse - MISMATCH
           at 452 bytes, because the parameter then lives across the strcat
           and the register saves move back into the prologue).

           It is a register-allocator decision with no operand to change and
           no ordering that reaches it, which is the class byte_match's own
           notes call unreachable from source.

WHAT GOT IT HERE, since four of the five were wrong in the first draft:

  * `colour_depth` is a SEPARATE LOCAL, not the `lpCmdLine` parameter reused.
    The image reads `[ebp+0x10]` for it, which looks like the parameter and is
    VC6 putting a local in the dead parameter's home slot - and on the
    non-DirectDraw path it is read WITHOUT EVER BEING WRITTEN. That is a bug
    in the shipped game, reachable only with DirectDraw off in the ini, and
    reproducing it is what moved the first divergence from #2 to #32.
  * `tgl_direct_draw ? 4 : 0`, NOT `-(x != 0) & 4`. The comment that used to
    sit here said a ternary compiles to a branch; measured, the ternary is
    what produces the image's `neg; sbb; and` and the hand-written branchless
    form produces `xor; test; setne`.
  * ONE `refused` variable, not two `if`s and not `||`. See the note at the
    statement itself - the three read the same in C and produce three
    different block layouts.
  * `Popup::alloc` is a real symbol (src/pending_bodies.cpp), so 0x0045F96F is
    `mov eax, OFFSET` with a relocation the comparison masks. Through a
    `static const` pointer variable it was `mov eax, [mem]` - a different
    opcode, which no masking can rescue.
  * `ExpansionEnabled` and `HandleMain` are objects rather than `BOOL *` and
    `HWND *` aimed at fixed addresses, which cost a load each.
*/
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine,
                   int nShowCmd) {
    CommandLineText[0] = '\0';
    if (lpCmdLine) {
        strcat(CommandLineText, lpCmdLine);
    }

    ExpansionEnabled = TRUE;
    PopupModalActive = 0;
    // The base game's allocator, then the expansion's over the top of it. The
    // guard is a test of the expansion allocator's own ADDRESS, which cannot
    // be null - so the first store is dead, and this pair is the seam the
    // expansion pack was linked through rather than a runtime choice.
    PopupAllocHook = BASEPOP_ALLOC;
    if (&Popup::alloc) {
        PopupAllocHook = &Popup::alloc;
    }
    DialogDefaultStyle |= 4;

    int video_mode = prefs_get("Video Mode", 0, false);
    int tgl_direct_draw = prefs_get("DirectDraw", 1, false);

    int display_width;
    int display_height;
    int colour_depth;
    if (tgl_direct_draw) {
        if (video_mode == 0) {
            video_mode = GetSystemMetrics(SM_CXSCREEN);
        }
        if (video_mode >= 1024) {
            display_width = 1024;
            display_height = 768;
        } else {
            display_width = 800;
            display_height = 600;
        }
        // The colour depth, in `lpCmdLine`'s stack slot. The original reuses
        // the parameter rather than spending a local on it, and 0x0045FA2B
        // reads the slot back to pass it on - so the reuse is load-bearing
        // and not an artifact of the decompilation.
        colour_depth = 8;
    } else {
        // Without DirectDraw the window takes its geometry and its depth from
        // the same slot, whatever the command line left in it.
        display_width = colour_depth;
        display_height = colour_depth;
    }

    // ONE STATUS, TWO TESTS - not `if (a) return 0; if (b) return 0;` and not
    // `if (a || b) return 0;`. All three read the same in C and none of the
    // other two is what VC6 emitted:
    //
    //   two ifs   the first `return 0` gets its own inline epilogue; the
    //             image jumps to a shared one              1 mnemonic edit
    //   `||`      both tests jump to an epilogue at the END of the function;
    //             the image's sits between them            3 mnemonic edits
    //   this      every mnemonic agrees
    //
    // The image tests once at 0x0045FA27 and jumps past the `jackal_init_real`
    // call to 0x0045FA54, and the second test at 0x0045FA50 falls INTO that
    // same block. That is one condition variable written twice and tested
    // once, which is this.
    int refused = jackal_version_check("10.10");
    if (!refused) {
        refused = jackal_init_real(&g_PALETTE1, &g_JACKAL_FONT,
                                   "Sid Meier's Alpha Centauri",
                                   tgl_direct_draw ? 4 : 0,
                                   display_width, display_height, colour_depth);
    }
    if (refused) {
        return 0;
    }

    if (Caviar::init_class()) {
        return 0;
    }

    unsigned long sound_backends = 10;
    if (prefs_get("ds3d", 1, false)) {
        sound_backends = 0x2A;
        prefs_put("ds3d", 1, false);
        if (prefs_get("eax", 1, false)) {
            sound_backends = 0x6A;
            prefs_put("eax", 1, false);
        }
    }

    // Holding Shift through startup skips sound entirely. The image tests the
    // whole high byte of the result rather than just the 0x8000 down bit.
    if (HIBYTE(GetAsyncKeyState(VK_SHIFT)) == 0) {
        init_sound(HandleMain, sound_backends);
    }

    g_JACKAL_FONT.init("arialn.ttf", DefaultFontFace, 12, 0);
    control_game();
    Caviar::close_class();
    jackal_close();
    return 1;
}
