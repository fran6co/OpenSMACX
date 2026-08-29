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
#include "caviar.h"
#include "font.h"
#include "game.h"     // control_game
#include "general.h"  // jackal_version_check, jackal_init_real, jackal_close
#include "basepop.h"   // BasePop::basepop_alloc
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
 * THE TWO POPUP ALLOCATORS, and the receiver one of them never had.
 *
 *   0x00404FB0  ?alloc@Popup@@QAAHXZ
 *   0x00604E40  ?basepop_alloc@BasePop@@SAHXZ
 *
 * Both are taken by ADDRESS here, and for the same reason: neither reads a
 * receiver, so both are declarable `static` and `&Class::method` is a
 * compile-time constant the compiler emits as `mov eax, OFFSET`.
 *
 * THIS FILE USED TO SAY THAT WAS IMPOSSIBLE. It argued from the census name
 * `?basepop_alloc@BasePop@@QAEHXZ` - `QAE` is `__thiscall`, C++ cannot take a
 * plain code address of a non-static member, therefore the call site had to
 * spell 0x00604E40 as a literal behind a macro, therefore "what cannot be
 * recovered is the NAME at this call site". Every step followed from the
 * first, and the first was wrong: the image is stripped, so `QAE` is a
 * RECONSTRUCTION, and the bytes deny it. `push ecx` at 0x00604E55 is VC6
 * reserving four stack bytes and `mov [esp], eax` overwrites it two
 * instructions later; all seven `call dword ptr [0x696ecc]` sites set up no
 * ecx at all. basepop.cpp records the correction, and the name is `SA`.
 *
 * So the store below is `&BasePop::basepop_alloc` and WinMain is still
 * BYTE_EXACT. The macro that stood here is gone, and with it the claim that
 * a name in this tree could be unrecoverable.
 */

char CommandLineText[0x108];  // 0x007D3970
// 0x00696ECC, and the image's own .data carries this initial value -
// tools/data_values.py reads 0x00604E40 there, which is
// `BasePop::basepop_alloc`. The assignment below at start-up is the
// image's too; the STATIC value was simply dropped in recovery.
func_popup_alloc *PopupAllocHook = &BasePop::basepop_alloc;
int PopupModalActive;  // 0x009469FC
int DialogDefaultStyle = 3;  // 0x006970DC

/*
Purpose: Bring up the process - preferences, display, sound, fonts - run the
         game, and tear it back down.
// ORIGINAL: 0x0045F950 _WinMain@16 0x0045F950-0x0045FB1B FILE BYTE_EXACT
// size      459 bytes
// prototype int (__stdcall _WinMain@16)(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
// callers   1   call targets   11
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004C5CE0 0x0052AA30 0x0059DB40 0x0059E530 0x006185A0 0x00618D20 0x006190D0 0x0062D3A0 0x0062D500 0x0062D570 0x00645470
// indirect  0x0045F9DE 0x0045FAC9
Return Value: 1 once the game has run and shut down; 0 if any stage of the
              bring-up refuses.
Status: Complete

TRIED: `reinterpret_cast<Caviar *>(0)->init_class()`. The image's names for
           Caviar::init_class and ::close_class end in `QAA` - a public member
           declared __cdecl - and 0x0045FA5D calls the first with no ecx set
           up at all, so there is no receiver and a non-static declaration
           makes the only legal spelling C2352. Calling through a null
           `Caviar *` was the way around that while caviar.h declared them as
           instance methods; they are `static` there now, which is the rule
           tools/emit_translation_unit.py already applies to every other
           `*::init_class`, so the cast is gone.

TRIED: eleven source spellings for the ONE instruction still differing.
           This body is MNEMONIC_ONLY - every mnemonic agrees, in order, 141
           against 141 - and 458 bytes against the image's 459. The whole of
           the remaining byte is #52:

               image  mov ebx, dword ptr [ebp + 0x10]   8B 5D 10   3 bytes
               here   mov ebx, edi                      8B DF      2 bytes

           Both sides put `colour_depth` in the dead parameter's home slot and
           both load it into edi first. The image LOADS IT AGAIN for ebx; VC6
           here reuses the register it already has. Every other difference in
           the body is a relocated operand the comparison masks.

           Tried, all still MNEMONIC_ONLY at 458 bytes: assigning height
           before width; routing one through a second local; chaining
           `width = height = depth`; reading the second through `*&depth`;
           assigning `height = width`; and all three declaration orders,
           including one on a single line - the slot a local gets is decided
           there, so it was the one axis with a mechanism behind it.

           Two made it worse. `volatile int colour_depth` drops to MISMATCH at
           #59 - it forces the DirectDraw path's store and the argument reload
           as well, which the image does not have. Reading `lpCmdLine` for both
           is MISMATCH at 452 bytes, because the parameter then lives across
           the strcat and the register saves move back into the prologue.

           `*static_cast<volatile int *>(&depth)` does not compile on VC6.

           What is left is a register allocator choosing a copy over a reload
           with no operand, ordering or storage class that reaches it - the
           class docs/recovery/agent-structure-observations.csv already records
           twice as unreachable from source. Iterate with
           `tools/verify_recovered_function.py 0x0045F950 --dir <candidates>`;
           it scores a whole directory of spellings in one call.

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
    PopupAllocHook = &BasePop::basepop_alloc;
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
        colour_depth = 8;
    } else {
        // BUG IN THE ORIGINAL: with `DirectDraw=0` in the ini, the window is
        // sized by whatever happened to be in a dead stack slot. Nothing on
        // this path writes the width, the height or the depth, and all three
        // are passed to jackal_init_real below.
        //
        // TWO DEAD LOCALS, NOT ONE READ TWICE, and the difference is the last
        // three bytes of the function. The image reads `[ebp+0x10]` TWICE:
        //
        //   0x0045FA14  8B 7D 10   mov edi, [ebp+0x10]
        //   0x0045FA17  8B 5D 10   mov ebx, [ebp+0x10]
        //
        // `display_width = colour_depth; display_height = colour_depth;`
        // cannot produce that - VC6 loads once and copies, `8B DF`, and the
        // body stops at MNEMONIC_ONLY 138/141 with both jump displacements
        // one short. Two SEPARATE uninitialised variables have nothing to
        // common-subexpression, and VC6 folds them onto the same dead slot -
        // `lpCmdLine`'s home, dead since the strcat above - which is why the
        // one address is read twice. Six spellings were compiled; this is the
        // only one that reproduces the shipped bytes.
        //
        // LEFT AS IT IS ON PURPOSE, because byte-exactness is the goal and a
        // fix here would be a different program. Worth fixing once the tree
        // compiles to the shipped bytes and can be diffed against them.
        int uninitialised_width;
        int uninitialised_height;
        display_width = uninitialised_width;
        display_height = uninitialised_height;
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
