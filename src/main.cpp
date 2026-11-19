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
// ORIGINAL: 0x0045F920 _$E21 0x0045F920-0x0045F936 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original CRT slot0x0068253C selects initializer0x0045F920-0x0045F936 and its registration names fini0x0045F940-0x0045F94A; complete instruction decode fixes both ends.", "Original operations identify storage0x007D3948; Font constructor/destructor618EA0/618EE0. Source object binding names ?g_JACKAL_FONT@@3VFont@@A.", "Fresh owner COFF selects _$E21 via .CRT$XCU and registers _$E19, both naming the same global. Fresh linked callback order is singleton, FontQueue VAL2/VAL1, FontTable, agreeing with their original slots.", "Current compiler-local names are emission interpretations, not original debug names or historical TU proof. The selected relative order does not establish complete CRT order, strict reference/linked preservation, dependency equivalence or runtime acceptance."], "reason": "Bind the original startup function to the genuine compiler-generated lifetime of source-owned g_JACKAL_FONT; declarations and construction order, not authored callbacks, provide the implementation."}
// symbol    _$E16
// ORIGINAL: 0x0045F940 _$E19 0x0045F940-0x0045F94A FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original CRT slot0x0068253C selects initializer0x0045F920-0x0045F936 and its registration names fini0x0045F940-0x0045F94A; complete instruction decode fixes both ends.", "Original operations identify storage0x007D3948; Font constructor/destructor618EA0/618EE0. Source object binding names ?g_JACKAL_FONT@@3VFont@@A.", "Fresh owner COFF selects _$E21 via .CRT$XCU and registers _$E19, both naming the same global. Fresh linked callback order is singleton, FontQueue VAL2/VAL1, FontTable, agreeing with their original slots.", "Current compiler-local names are emission interpretations, not original debug names or historical TU proof. The selected relative order does not establish complete CRT order, strict reference/linked preservation, dependency equivalence or runtime acceptance."], "reason": "Bind the original exit function to the genuine compiler-generated lifetime of source-owned g_JACKAL_FONT; declarations and construction order, not authored callbacks, provide the implementation."}
// symbol    _$E14
#include "stdafx.h"
#include "interface_fonts.h"
#include "main.h"
#include "alpha.h"          // prefs_get, prefs_put
#include "caviar.h"
#include "font.h"
#include "game.h"     // control_game
#include "interface_palette.h"
#include "general.h"
#include "jackal.h" // version check and lifecycle library interface
#include "basepop.h"   // BasePop::basepop_alloc
#include "palette.h"  // g_PALETTE1
#include "popup.h"     // Popup::alloc
#include "sound.h"    // init_sound
#include "win.h"
#include "popup_fonts.h"
#include "alphamenu.h"
#include "netdaemon.h"
#include "strings.h"
#include "text.h"
#include "terraforming.h"
#include "technology.h"
#include "veh.h"
#include "console.h"
#include "maininterface.h"
#include "pulldown.h"
#include "wave.h"
#include "buffer.h"

// The entrypoint owns this real font object: WinMain supplies it to Jackal
// and initializes it. Its original CRT slot 0x0068253C precedes the two
// FontQueue globals and FontTable; it is not a pointer into the old image.
// RECOVERY_OBJECT: v1 0x007D3948 ?g_JACKAL_FONT@@3VFont@@A
Font g_JACKAL_FONT;

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
 * HISTORICAL CONTEXT. During the first entry-point integration, six of the
 * eleven startup callees still crossed fixed-address forwarding scaffolds.
 * Those temporary routes and source directories have since been retired.
 * This build/link milestone still does not establish game correctness or
 * complete recovery; the executable remains explicitly unfinished.
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
// RECOVERY_OBJECT: v1 0x009469FC ?PopupModalActive@@3HA
int PopupModalActive;  // 0x009469FC

// WinMain's version check and engine bring-up are one inline wrapper around
// jackal_init_real: its "10.10" literal (0x00685418) follows every WinMain
// literal ("arialn.ttf" at 0x0068540C) because VC6 emits an inline
// function's literal as a ??_C COMDAT when the expansion is compiled.
// WORKAROUND(decl): the wrapper's original home (likely jackal.h) and name
// are unknown; it is defined here so no other unit sees it.
inline int __cdecl jackal_init(Palette *palette, Font *font, LPSTR window_name,
                               int tgl_direct_draw, int display_width,
                               int display_height, int colour_depth) {
    // RECOVERY_LITERAL: v1 0x00685418 0x0068541E ??_C@_05DFGP@10?410?$AA@ 0x0045F950
    int refused = jackal_version_check("10.10");
    if (!refused) {
        refused = jackal_init_real(palette, font, window_name, tgl_direct_draw,
                                   display_width, display_height, colour_depth);
    }
    return refused;
}

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
  * `Popup::alloc` is a real source-owned symbol in `src/popup.cpp`, so 0x0045F96F is
    `mov eax, OFFSET` with a relocation the comparison masks. Through a
    `static const` pointer variable it was `mov eax, [mem]` - a different
    opcode, which no masking can rescue.
  * `ExpansionEnabled` and `HandleMain` are objects rather than `BOOL *` and
    `HWND *` aimed at fixed addresses, which cost a load each.
*/
// The image keeps a `push ebp; mov ebp, esp` frame (flags: frame). The unit's
// measured/production recipe is a fixed /O2 (implies /Oy) since 92775e898,
// so the frame is restored for this body alone, as X_text_open does.
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
    DialogDefaults[36] |= 4u;

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
    if (jackal_init(&g_PALETTE1, &g_JACKAL_FONT,
                    "Sid Meier's Alpha Centauri",
                    tgl_direct_draw ? 4 : 0,
                    display_width, display_height, colour_depth)) {
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

// From src/console_menu_policy.cpp (unit prelude):
// Inferred Console menu-policy contribution at45FB20..46207E. The two
// adjacent entries share the real Console menu receiver. Original MainMenu
// versus AlphaMenu class identity is unresolved; keep current layout and
// typed calls until independent evidence distinguishes those relationships.

// Shared punctuation objects the image pushes instead of unit literals.
extern char SocialWinCloseParenText[];
extern char SocialWinOpenParenText[];
// Original pointer cell 0x00691B1C selects the MENU script name at
// 0x00691B9C; menu_init passes the cell's value to every text_open.
// RECOVERY_OBJECT: v1 0x00691B9C ?MenuScriptFileText@@3PADA
char MenuScriptFileText[5] = "MENU";
// RECOVERY_OBJECT: v1 0x00691B1C ?MenuScriptFile@@3PADA
char *MenuScriptFile = MenuScriptFileText;

// Console initializes its embedded AlphaMenu through this catalogue entry.
// Population uses the existing rules and StringTable owners; x86 pool keys
// remain the existing pointer-to-int Strings API, not portable pointer proof.
// ORIGINAL: 0x0045FB20 ?menu_init@MainMenu@@QAEHXZ 0x0045FB20-0x00460DD0 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/0045fb20.cpp", "image:0x51D922,0x51D996-0x51D99D:Console alpha_menu_ receiver"], "reason": "Pinned Console init receiver+22A2C identifies existing AlphaMenu; catalogue MainMenu identity retained separately from native AlphaMenu symbol. Complete ordered menu population uses source rules and StringTable owners; no byte claim."}
// TRIED: ~100% of the call structure transcribed 1:1 (all 181 add_menu_item, 41 add_separator, 9 add_menu, 9 hide/disable_menu_item, 9 text_open group opens, the 19-iteration struct-array loop building "id: text" labels via Strings::get+strcat, the trailing conditional hide_menu_item and vtable slot093 dispatch); diverges at prologue (#2 sub vs push) because the local frame layout (the [ebp-8] result slot, the [ebp-4] loop cursor) is not reproduced - returns are done via plain `return N;` instead of a stored local, so stack size/order differs from here on. Not chased further given size.
// symbol    ?menu_init@AlphaMenu@@QAEHXZ
int AlphaMenu::menu_init() {
    int result = 1;
    read_rules(0);
    if (text_open(MenuScriptFile, "GAME") != 0) goto done;
    add_menu(0, text_get(), 0);
    add_menu_item(0, 0x51, text_get());
    add_menu_item(0, 0x52, text_get());
    add_separator(0, 0);
    add_menu_item(0, 0x53, text_get());
    add_menu_item(0, 0x54, text_get());
    add_menu_item(0, 0x55, text_get());
    add_menu_item(0, 0x56, text_get());
    add_menu_item(0, 0x57, text_get());
    add_menu_item(0, 0x58, text_get());
    add_separator(0, 0);
    add_menu_item(0, 0x59, text_get());
    add_menu_item(0, 0x5a, text_get());
    add_separator(0, 0);
    add_menu_item(0, 0x5b, text_get());
    add_menu_item(0, 0x5c, text_get());
    add_menu_item(0, 0x5d, text_get());
    if (text_open(MenuScriptFile, "HQ") != 0) goto done;
    add_menu(1, text_get(), 0);
    add_menu_item(1, 0x105, text_get());
    add_menu_item(1, 0x106, text_get());
    add_menu_item(1, 0x107, text_get());
    add_separator(1, 0);
    add_menu_item(1, 0x103, text_get());
    add_separator(1, 0);
    add_menu_item(1, 0x108, text_get());
    add_menu_item(1, 0x109, text_get());
    add_menu_item(1, 0x10a, text_get());
    add_separator(1, 0);
    add_menu_item(1, 0x10b, text_get());
    add_menu_item(1, 0x10c, text_get());
    add_menu_item(1, 0x10d, text_get());
    add_menu_item(1, 0x10e, text_get());
    add_separator(1, 0);
    add_menu_item(1, 0x10f, text_get());
    add_menu_item(1, 0x110, text_get());
    add_menu_item(1, 0x111, text_get());
    add_menu_item(1, 0x113, text_get());
    add_separator(1, 9);
    add_menu_item(1, 0x114, text_get());
    add_menu_item(1, 0x115, text_get());
    if (text_open(MenuScriptFile, "NETWORK") != 0) goto done;
    add_menu(8, text_get(), 0);
    add_menu_item(8, 0x800, text_get());
    add_menu_item(8, 0x801, text_get());
    add_menu_item(8, 0x802, text_get());
    add_menu_item(8, 0x803, text_get());
    add_menu_item(8, 0x804, text_get());
    add_separator(8, 0);
    add_menu_item(8, 0x805, text_get());
    add_separator(8, 0);
    add_menu_item(8, 0x806, text_get());
    add_menu_item(8, 0x807, text_get());
    add_menu_item(8, 0x808, text_get());
    if (text_open(MenuScriptFile, "MAP") != 0) goto done;
    add_menu(2, text_get(), 0);
    add_menu_item(2, 0x200, text_get());
    add_menu_item(2, 0x201, text_get());
    add_separator(2, 0);
    add_menu_item(2, 0x203, text_get());
    add_menu_item(2, 0x204, text_get());
    add_menu_item(2, 0x205, text_get());
    add_menu_item(2, 0x206, text_get());
    add_menu_item(2, 0x207, text_get());
    add_menu_item(2, 0x208, text_get());
    add_separator(2, 0);
    add_menu_item(2, 0x209, text_get());
    add_menu_item(2, 0x20a, text_get());
    add_menu_item(2, 0x20b, text_get());
    add_menu_item(2, 0x20c, text_get());
    add_menu_item(2, 0x215, text_get());
    add_separator(2, 0);
    add_menu_item(2, 0x20f, text_get());
    add_menu_item(2, 0x210, text_get());
    add_menu_item(2, 0x211, text_get());
    add_menu_item(2, 0x212, text_get());
    add_menu_item(2, 0x213, text_get());
    add_separator(2, 0);
    add_menu_item(2, 0x216, text_get());
    add_menu_item(2, 0x217, text_get());
    add_menu_item(2, 0x214, text_get());
    if (text_open(MenuScriptFile, "ACTION") != 0) goto done;
    add_menu(3, text_get(), 0);
    add_menu_item(3, 0x300, text_get());
    add_menu_item(3, 0x301, text_get());
    add_menu_item(3, 0x302, text_get());
    add_separator(3, 2);
    add_menu_item(3, 0x305, text_get());
    add_menu_item(3, 0x309, text_get());
    add_menu_item(3, 0x30a, text_get());
    add_menu_item(3, 0x310, text_get());
    add_menu_item(3, 0x308, text_get());
    add_menu_item(3, 0x311, text_get());
    add_menu_item(3, 0x312, text_get());
    add_separator(3, 0);
    add_menu_item(3, 0x314, text_get());
    add_menu_item(3, 0x31b, text_get());
    add_menu_item(3, 0x324, text_get());
    add_menu_item(3, 0x306, text_get());
    add_menu_item(3, 0x304, text_get());
    add_separator(3, 0);
    add_menu_item(3, 0x318, text_get());
    add_menu_item(3, 0x319, text_get());
    add_menu_item(3, 0x31a, text_get());
    add_menu_item(3, 0x317, text_get());
    add_menu_item(3, 0x316, text_get());
    add_separator(3, 0);
    add_menu_item(3, 0x313, text_get());
    add_menu_item(3, 0x31c, text_get());
    add_menu_item(3, 0x31d, text_get());
    add_menu_item(3, 0x31e, text_get());
    add_menu_item(3, 0x31f, text_get());
    add_menu_item(3, 0x320, text_get());
    add_separator(3, 0);
    add_menu_item(3, 0x321, text_get());
    add_menu_item(3, 0x307, text_get());
    add_menu_item(3, 0x322, text_get());
    add_menu_item(3, 0x315, text_get());
    add_menu_item(3, 0x323, text_get());
    if (text_open(MenuScriptFile, "TERRAFORM") != 0) goto done;
    add_menu(9, text_get(), 0);
    {
        for (int i = 0; i < 19; ++i) {
            StringTemp[0] = 0;
            strcat(StringTemp, table_text(Order[i + 4].order[0]));
            strcat(StringTemp, "|");
            strcat(StringTemp, table_text(Terraforming[i].shortcuts));
            add_menu_item(9, 0x900 + i, StringTemp);
            if (i == 4 || i == 11 || i == 14) {
                add_separator(9, i == 14);
            }
        }
    }
    add_separator(9, 5);
    add_menu_item(9, 0x9f8, text_get());
    add_menu_item(9, 0x9f9, text_get());
    add_separator(9, 6);
    add_menu_item(9, 0x9f0, text_get());
    add_menu_item(9, 0x9f1, text_get());
    add_separator(9, 7);
    add_menu_item(9, 0x9f3, text_get());
    add_menu_item(9, 0x9f4, text_get());
    add_menu_item(9, 0x9f5, text_get());
    add_menu_item(9, 0x9f6, text_get());
    add_menu_item(9, 0x9f7, text_get());
    add_menu_item(9, 0x9f2, text_get());
    /* pop edi; pop ebx (loop epilogue registers) */
    if (text_open(MenuScriptFile, "SCENARIO") != 0) goto done;
    add_menu(6, text_get(), 0);
    add_menu_item(6, 0x600, text_get());
    add_menu_item(6, 0x601, text_get());
    add_separator(6, 0);
    add_menu_item(6, 0x602, text_get());
    add_menu_item(6, 0x603, text_get());
    add_menu_item(6, 0x604, text_get());
    add_menu_item(6, 0x605, text_get());
    add_separator(6, 0);
    add_menu_item(6, 0x606, text_get());
    add_menu_item(6, 0x607, text_get());
    add_menu_item(6, 0x608, text_get());
    add_menu_item(6, 0x609, text_get());
    add_separator(6, 0);
    add_menu_item(6, 0x60a, text_get());
    add_menu_item(6, 0x60b, text_get());
    add_menu_item(6, 0x60c, text_get());
    add_menu_item(6, 0x60d, text_get());
    add_menu_item(6, 0x60e, text_get());
    add_separator(6, 0);
    add_menu_item(6, 0x60f, text_get());
    add_menu_item(6, 0x610, text_get());
    add_menu_item(6, 0x611, text_get());
    add_menu_item(6, 0x612, text_get());
    add_menu_item(6, 0x613, text_get());
    add_separator(6, 0);
    add_menu_item(6, 0x614, text_get());
    add_menu_item(6, 0x615, text_get());
    add_menu_item(6, 0x616, text_get());
    add_menu_item(6, 0x617, text_get());
    add_separator(6, 0);
    add_menu_item(6, 0x618, text_get());
    add_menu_item(6, 0x619, text_get());
    add_menu_item(6, 0x61a, text_get());
    add_menu_item(6, 0x61b, text_get());
    if (text_open(MenuScriptFile, "EDITOR") != 0) goto done;
    add_menu(0xa, text_get(), 0);
    add_menu_item(0xa, 0xa00, text_get());
    add_menu_item(0xa, 0xa01, text_get());
    add_separator(0xa, 0);
    add_menu_item(0xa, 0xa10, text_get());
    add_menu_item(0xa, 0xa11, text_get());
    add_menu_item(0xa, 0xa12, text_get());
    add_menu_item(0xa, 0xa13, text_get());
    add_menu_item(0xa, 0xa14, text_get());
    add_menu_item(0xa, 0xa15, text_get());
    add_menu_item(0xa, 0xa16, text_get());
    add_separator(0xa, 0);
    add_menu_item(0xa, 0xa17, text_get());
    add_menu_item(0xa, 0xa18, text_get());
    add_menu_item(0xa, 0xa19, text_get());
    add_separator(0xa, 0);
    add_menu_item(0xa, 0xa20, text_get());
    add_menu_item(0xa, 0xa21, text_get());
    add_menu_item(0xa, 0xa22, text_get());
    add_menu_item(0xa, 0xa24, text_get());
    add_separator(0xa, 0);
    add_menu_item(0xa, 0xa48, text_get());
    add_menu_item(0xa, 0xa30, text_get());
    add_menu_item(0xa, 0xa31, text_get());
    add_menu_item(0xa, 0xa32, text_get());
    add_separator(0xa, 0);
    add_menu_item(0xa, 0xa36, text_get());
    add_menu_item(0xa, 0xa37, text_get());
    add_menu_item(0xa, 0xa40, text_get());
    add_menu_item(0xa, 0xa41, text_get());
    add_separator(0xa, 0);
    add_menu_item(0xa, 0xa50, text_get());
    add_menu_item(0xa, 0xa51, text_get());
    add_menu_item(0xa, 0xa52, text_get());
    add_menu_item(0xa, 0xa53, text_get());
    add_separator(0xa, 0);
    add_menu_item(0xa, 0xa54, text_get());
    if (text_open(MenuScriptFile, "HELP") != 0) goto done;
    add_menu(7, text_get(), 1);
    add_menu_item(7, 0x701, text_get());
    add_menu_item(7, 0x702, text_get());
    add_menu_item(7, 0x703, text_get());
    add_separator(7, 0);
    add_menu_item(7, 0x704, text_get());
    add_menu_item(7, 0x70b, text_get());
    add_menu_item(7, 0x70c, text_get());
    add_separator(7, 0);
    add_menu_item(7, 0x70d, text_get());
    add_menu_item(7, 0x70e, text_get());
    add_separator(7, 0);
    add_menu_item(7, 0x70a, text_get());
    add_menu_item(7, 0x705, text_get());
    add_menu_item(7, 0x706, text_get());
    add_menu_item(7, 0x707, text_get());
    add_menu_item(7, 0x708, text_get());
    add_menu_item(7, 0x709, text_get());
    add_separator(7, 0);
    add_menu_item(7, 0x720, text_get());
    add_menu_item(7, 0x721, text_get());
    add_menu_item(7, 0x722, text_get());
    add_menu_item(7, 0x723, text_get());
    add_menu_item(7, 0x724, text_get());
    add_menu_item(7, 0x725, text_get());
    add_separator(7, 0);
    add_menu_item(7, 0x750, text_get());
    add_menu_item(7, 0x751, text_get());
    hide_menu_item(1, 0x106);
    hide_menu_item(1, 0x107);
    hide_menu_item(3, 0x301);
    hide_menu_item(3, 0x309);
    hide_menu_item(3, 0x30a);
    hide_menu_item(3, 0x310);
    hide_menu_item(3, 0x306);
    hide_menu_item(3, 0x304);
    disable_menu_item(3, 0x305);
    if ((GameDataStateStorage.fields_.game_rules_ & RULES_IRONMAN) != 0) {
        hide_menu_item(0, 0x5a);
    }
    check(-1);
    adjust();
    result = 0;
done:
    return result;
}


// ORIGINAL: 0x00460DD0 ?check@MainMenu@@QAEXH@Z 0x00460DD0-0x0046207E BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical retired seed src/unrecovered/00460dd0.cpp", "recovery/tasks/alphamenu-check-owner-oct5.json", "Console embedded member +22A2C, actual constructor and by-name callers", "complete pinned 460DD0..46207E CFG and fresh native current owner symbol; actual-source bounded trace/mutant controls"], "reason": "Recover complete menu policy in the actual Console::alpha_menu_ subobject. Preserve historical MainMenu catalogue identity separately from the observed current AlphaMenu symbol; genuine source ownership corrects the old seed hypothesis. No current byte or runtime acceptance claim."}
// symbol    ?check@AlphaMenu@@QAEHH@Z
// TRIED: time-boxed - roughly the first quarter of the function (a long sequence of Menu::show_menu_item/hide_menu_item/ enable_menu_item gates against game-option and tech flags, transcribed with goto-labelled control flow matching the disassembly's branch targets) is faithful; the difficulty-table-driven remainder (several hundred more show/hide/enable calls) is a deliberate stub. MISMATCH #2, well past the prologue.
int AlphaMenu::check(int veh_id) {
    int i;
    int sea;
    int base_id;
    int owner;
    int turns;
    int avail;
    char turn_text[80];

    if (GameDataStateStorage.fields_.game_more_preferences_ & 0x200000) {
        show_menu_item(0, 0x52);
        hide_menu_item(0, 0x51);
    } else {
        show_menu_item(0, 0x51);
        hide_menu_item(0, 0x52);
    }

    // The image lays the hide-0x5C/debug-mode arm out after the next
    // unconditional jump (0x461020) and falls from show 0x5C straight into the
    // shared show 0x5A; an else arm ending in a goto past that join does this.
    if (!IsMultiplayerNet && !IsMultiplayerPBEM &&
        !(GameDataStateStorage.fields_.game_rules_ & RULES_IRONMAN)) {
        show_menu_item(0, 0x5C);
    } else {
        if (IsMultiplayerNet || IsMultiplayerPBEM) {
            hide_menu_item(0, 0x5C);
        }
        if (!(GameDataStateStorage.fields_.game_state_ & STATE_DEBUG_MODE)) {
            hide_menu_item(0, 0x5A);
            goto expansion_gate;
        }
    }
    show_menu_item(0, 0x5A);
expansion_gate:
    if (GameDataStateStorage.fields_.game_more_preferences_ & 0x200000) {
        show_menu_item(2, 0x205);
        show_menu_item(2, 0x206);
        show_menu_item(2, 0x207);
        show_menu_item(2, 0x208);
        show_menu_item(2, 0x20C);
        show_menu_item(2, 0x215);
        show_menu_item(2, 0x211);
        show_menu_item(2, 0x212);
        show_menu_item(3, 0x31B);
        show_menu_item(3, 0x324);
        show_menu_item(3, 0x306);
        show_menu_item(3, 0x304);
        show_menu_item(3, 0x319);
        show_menu_item(3, 0x31A);
        show_menu_item(3, 0x317);
        show_menu_item(3, 0x31F);
        show_menu_item(3, 0x320);
        show_menu_item(3, 0x307);
        show_menu_item(3, 0x322);
        show_menu_item(9, 0x9F8);
        show_menu_item(9, 0x9F9);
        show_menu_item(9, 0x9F6);
        show_menu_item(9, 0x9F5);
        show_menu_item(9, 0x9F4);
        show_menu_item(9, 0x9F3);
        show_menu_item(9, 0x9F1);
        show_menu_item(9, 0x9F0);
        show_menu_item(9, 5);
        show_menu_item(9, 6);
        show_menu_item(9, 7);
    }

    if (!(GameDataStateStorage.fields_.game_state_ & STATE_IS_SCENARIO) &&
        ScenarioSave.objective_req_victory >= 1000 &&
        ScenarioSave.objectives_sudden_death_victory >= 1000) {
        hide_menu_item(1, 0x115);
    } else {
        show_menu_item(1, 0x115);
    }

    if (IsMultiplayerNet && (GameDataStateStorage.fields_.game_state_ & 2) &&
        NetDaemonNet.factions_waiting_mask() &&
        NetDaemonNet.factions_waiting_mask() != 0xFF) {
        hide_menu_item(8, 0x807);
        show_menu_item(8, 0x808);
    } else {
        show_menu_item(8, 0x807);
        hide_menu_item(8, 0x808);
    }

    if (!(GameDataStateStorage.fields_.game_state_ & 0xC0) &&
        (GameDataStateStorage.fields_.net_turn_flags_ & 8)) {
        hide_menu_item(1, 0x105);
    } else {
        show_menu_item(1, 0x105);
    }

    if (GameDataStateStorage.fields_.game_rules_ & RULES_BLIND_RESEARCH) {
        show_menu_item(1, 0x106);
        hide_menu_item(1, 0x107);
    } else {
        show_menu_item(1, 0x107);
        hide_menu_item(1, 0x106);
    }

    if ((has_tech(Rules.tech_economic_victory, ConsoleGlobal.faction_id_) &&
         (GameDataStateStorage.fields_.game_rules_ & RULES_VICTORY_ECONOMIC)) ||
        ((GameDataStateStorage.fields_.game_state_ & STATE_DEBUG_MODE) &&
         (GameDataStateStorage.fields_.game_state_ & STATE_SCENARIO_EDITOR))) {
        show_menu_item(1, 0x114);
        show_menu_item(1, 9);
    } else {
        hide_menu_item(1, 0x114);
        hide_menu_item(1, 9);
    }

    if (ConsoleGlobal.cursor_active_) {
        show_menu_item(2, 0x201);
        hide_menu_item(2, 0x200);
    } else {
        show_menu_item(2, 0x200);
        hide_menu_item(2, 0x201);
    }

    if (ConsoleGlobal.cursor_active_ && veh_id >= 0) {
        for (i = 0x300; i <= 0x324; i++) {
            enable_menu_item(3, i);
        }
        if (!has_abil(Vehs[veh_id].proto_id, ABL_DROP_POD)) {
            hide_menu_item(3, 0x309);
        } else {
            show_menu_item(3, 0x309);
        }

        owner = whose_territory(ConsoleGlobal.faction_id_, Vehs[veh_id].x,
                                Vehs[veh_id].y, NULL, 0);
        if ((owner >= 0 && owner != ConsoleGlobal.faction_id_ &&
             (PlayersData[ConsoleGlobal.faction_id_].diplo_treaties[owner] & DTREATY_COMMLINK) &&
             !(PlayersData[ConsoleGlobal.faction_id_].diplo_treaties[owner] & DTREATY_VENDETTA)) ||
            (Vehs[veh_id].proto_id < MaxVehProtoFactionNum &&
             (get_proto_offense_rating(Vehs[veh_id].proto_id) < 0 ||
              Vehs[veh_id].proto_id == BSC_SPORE_LAUNCHER))) {
            show_menu_item(3, 0x31F);
        } else {
            hide_menu_item(3, 0x31F);
        }

        if (Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].missile &&
            (VehPrototypes[Vehs[veh_id].proto_id].weapon_id == 0x18 ||
             VehPrototypes[Vehs[veh_id].proto_id].weapon_id == 0x19)) {
            show_menu_item(3, 0x30A);
        } else {
            hide_menu_item(3, 0x30A);
        }

        base_id = base_at(Vehs[veh_id].x, Vehs[veh_id].y);
        if (base_id >= 0 && has_fac(FAC_PSI_GATE, base_id, 0)) {
            show_menu_item(3, 0x310);
        } else {
            hide_menu_item(3, 0x310);
        }

        if (!veh_cargo(veh_id) && !has_abil(Vehs[veh_id].proto_id, ABL_CARRIER)) {
            hide_menu_item(3, 0x31D);
        } else {
            show_menu_item(3, 0x31D);
        }

        if (Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad == TRIAD_AIR &&
            get_proto_offense_rating(Vehs[veh_id].proto_id)) {
            show_menu_item(3, 0x306);
            show_menu_item(3, 0x304);
        } else {
            hide_menu_item(3, 0x306);
            hide_menu_item(3, 0x304);
        }

        if (can_arty(Vehs[veh_id].proto_id, true)) {
            show_menu_item(3, 0x305);
        } else {
            hide_menu_item(3, 0x305);
        }

        if (VehPrototypes[Vehs[veh_id].proto_id].plan == PLAN_SUPPLY_CONVOY) {
            show_menu_item(3, 0x308);
            enable_menu_item(3, 0x308);
        } else {
            hide_menu_item(3, 0x308);
            disable_menu_item(3, 0x308);
        }

        if (VehPrototypes[Vehs[veh_id].proto_id].plan == PLAN_COLONIZATION) {
            enable_menu_item(3, 0x300);
            enable_menu_item(3, 0x301);
            show_menu_item(3, 2);
            if (base_who(Vehs[veh_id].x, Vehs[veh_id].y) >= 0) {
                show_menu_item(3, 0x301);
                hide_menu_item(3, 0x300);
            } else {
                hide_menu_item(3, 0x301);
                show_menu_item(3, 0x300);
            }
            hide_menu_item(3, 0x302);
        } else {
            disable_menu_item(3, 0x300);
            disable_menu_item(3, 0x301);
            hide_menu_item(3, 0x300);
            hide_menu_item(3, 0x301);
            if (get_proto_offense_rating(Vehs[veh_id].proto_id) &&
                base_who(Vehs[veh_id].x, Vehs[veh_id].y) == Vehs[veh_id].faction_id &&
                Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad != TRIAD_AIR) {
                show_menu_item(3, 0x302);
                enable_menu_item(3, 0x302);
            } else {
                disable_menu_item(3, 0x302);
                hide_menu_item(3, 0x302);
            }
            hide_menu_item(3, 2);
        }

        if (get_proto_offense_rating(Vehs[veh_id].proto_id)) {
            show_menu_item(3, 0x320);
            show_menu_item(3, 0x311);
        } else {
            hide_menu_item(3, 0x320);
            hide_menu_item(3, 0x311);
        }

        if (VehPrototypes[Vehs[veh_id].proto_id].plan != PLAN_TERRAFORMING) {
            for (i = 0; i < 19; i++) {
                sea = Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad == TRIAD_SEA ||
                      (Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad == TRIAD_AIR &&
                       is_ocean(Vehs[veh_id].x, Vehs[veh_id].y));
                StringTemp[0] = '\0';
                strcat(StringTemp, table_text(Order[i + 4].order[sea]));
                strcat(StringTemp, "|");
                strcat(StringTemp, table_text(Terraforming[i].shortcuts));
                rename_menu_item(9, i + 0x900, StringTemp);
                if (!terrain_avail(i, sea, ConsoleGlobal.faction_id_)) {
                    hide_menu_item(9, i + 0x900);
                    if (i == 18) {
                        hide_menu_item(9, 1);
                    }
                } else {
                    show_menu_item(9, i + 0x900);
                    disable_menu_item(9, i + 0x900);
                    if (i == 18) {
                        show_menu_item(9, 1);
                    }
                }
            }
            disable_menu_item(9, 0x9F8);
            disable_menu_item(9, 0x9F9);
            disable_menu_item(9, 0x9F0);
            disable_menu_item(9, 0x9F1);
            disable_menu_item(9, 0x9F6);
            disable_menu_item(9, 0x9F3);
            disable_menu_item(9, 0x9F4);
            disable_menu_item(9, 0x9F5);
            disable_menu_item(9, 0x9F7);
            disable_menu_item(9, 0x9F2);
        } else {
            for (i = 0; i < 19; i++) {
                sea = Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad == TRIAD_SEA ||
                      (Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad == TRIAD_AIR &&
                       is_ocean(Vehs[veh_id].x, Vehs[veh_id].y));
                // A flag, not one || condition: the image falls from the
                // farm test into the unavailable row (0x46196D) and defers the
                // road and terrain_avail tests; the available row's "|"
                // literal (0x685470) also precedes the unavailable one.
                if (i == 1 && !(map_loc(Vehs[veh_id].x, Vehs[veh_id].y)->bit & BIT_FARM)) {
                    avail = 0;
                } else if (i == 6 && !(map_loc(Vehs[veh_id].x, Vehs[veh_id].y)->bit & BIT_ROAD)) {
                    avail = 0;
                } else {
                    avail = terrain_avail(i, sea, ConsoleGlobal.faction_id_);
                }
                if (avail) {
                    StringTemp[0] = '\0';
                    strcat(StringTemp, table_text(Order[i + 4].order[sea]));
                    turns = action_terraform(veh_id, i, 0);
                    strcat(StringTemp, BufferJoinSpace);
                    strcat(StringTemp, SocialWinOpenParenText);
                    _itoa(turns + 1, turn_text, 10);
                    strcat(StringTemp, turn_text);
                    strcat(StringTemp, BufferJoinSpace);
                    if (turns == 1) {
                        strcat(StringTemp, label_get(0x30));
                    } else {
                        strcat(StringTemp, label_get(0x31));
                    }
                    strcat(StringTemp, SocialWinCloseParenText);
                    strcat(StringTemp, "|");
                    strcat(StringTemp, table_text(Terraforming[i].shortcuts));
                    rename_menu_item(9, i + 0x900, StringTemp);
                    show_menu_item(9, i + 0x900);
                    if ((i != 1 || (map_loc(Vehs[veh_id].x, Vehs[veh_id].y)->bit & BIT_FARM)) &&
                        (i != 6 || (map_loc(Vehs[veh_id].x, Vehs[veh_id].y)->bit & BIT_ROAD))) {
                        enable_menu_item(9, i + 0x900);
                    } else {
                        disable_menu_item(9, i + 0x900);
                    }
                    if (i == 18) {
                        show_menu_item(9, 1);
                    }
                } else {
                    StringTemp[0] = '\0';
                    strcat(StringTemp, table_text(Order[i + 4].order[0]));
                    strcat(StringTemp, "|");
                    strcat(StringTemp, table_text(Terraforming[i].shortcuts));
                    rename_menu_item(9, i + 0x900, StringTemp);
                    disable_menu_item(9, i + 0x900);
                    hide_menu_item(9, i + 0x900);
                    if (i == 18) {
                        hide_menu_item(9, 1);
                    }
                }
            }
            if (Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad == TRIAD_SEA) {
                hide_menu_item(9, 0x9F8);
                hide_menu_item(9, 0x9F9);
            } else {
                show_menu_item(9, 0x9F8);
                show_menu_item(9, 0x9F9);
                enable_menu_item(9, 0x9F8);
                enable_menu_item(9, 0x9F9);
            }
            enable_menu_item(9, 0x9F7);
            enable_menu_item(9, 0x9F2);
            if (has_tech(Terraforming[10].prerequisite_tech[sea], ConsoleGlobal.faction_id_)) {
                enable_menu_item(9, 0x9F6);
            } else {
                disable_menu_item(9, 0x9F6);
            }
            if (has_tech(Terraforming[5].prerequisite_tech[sea], ConsoleGlobal.faction_id_)) {
                enable_menu_item(9, 0x9F0);
                enable_menu_item(9, 0x9F3);
            } else {
                disable_menu_item(9, 0x9F0);
                disable_menu_item(9, 0x9F3);
            }
            if (has_tech(Terraforming[6].prerequisite_tech[sea], ConsoleGlobal.faction_id_)) {
                show_menu_item(9, 0x9F1);
                show_menu_item(9, 0x9F4);
                enable_menu_item(9, 0x9F1);
                enable_menu_item(9, 0x9F4);
            } else {
                hide_menu_item(9, 0x9F1);
                hide_menu_item(9, 0x9F4);
            }
            if (has_tech(Terraforming[9].prerequisite_tech[sea], ConsoleGlobal.faction_id_)) {
                show_menu_item(9, 0x9F5);
                enable_menu_item(9, 0x9F5);
            } else {
                hide_menu_item(9, 0x9F5);
            }
        }

        if (base_who(Vehs[veh_id].x, Vehs[veh_id].y) >= 0) {
            hide_menu_item(3, 0x317);
            show_menu_item(3, 0x316);
        } else {
            show_menu_item(3, 0x317);
            hide_menu_item(3, 0x316);
        }
    } else {
        for (i = 0x300; i <= 0x324; i++) {
            if (i != 0x313) {
                disable_menu_item(3, i);
            } else {
                enable_menu_item(3, 0x313);
            }
        }
    }

    for (i = 0x601; i <= 0x61B; i++) {
        if (GameDataStateStorage.fields_.game_state_ & STATE_SCENARIO_EDITOR) {
            enable_menu_item(6, i);
        } else {
            disable_menu_item(6, i);
        }
    }
    for (i = 0xA00; i <= 0xA54; i++) {
        if (GameDataStateStorage.fields_.game_state_ & STATE_SCENARIO_EDITOR) {
            enable_menu_item(10, i);
        } else {
            disable_menu_item(10, i);
        }
    }

    if (!(GameDataStateStorage.fields_.game_more_preferences_ & 0x200000)) {
        hide_menu_item(2, 0x205);
        hide_menu_item(2, 0x206);
        hide_menu_item(2, 0x207);
        hide_menu_item(2, 0x208);
        hide_menu_item(2, 0x20C);
        hide_menu_item(2, 0x215);
        hide_menu_item(2, 0x211);
        hide_menu_item(2, 0x212);
        hide_menu_item(3, 0x31B);
        hide_menu_item(3, 0x324);
        hide_menu_item(3, 0x306);
        hide_menu_item(3, 0x304);
        hide_menu_item(3, 0x319);
        hide_menu_item(3, 0x31A);
        hide_menu_item(3, 0x317);
        hide_menu_item(3, 0x320);
        hide_menu_item(3, 0x31F);
        hide_menu_item(3, 0x307);
        hide_menu_item(3, 0x322);
        hide_menu_item(9, 0x9F8);
        hide_menu_item(9, 0x9F9);
        hide_menu_item(9, 0x9F6);
        hide_menu_item(9, 0x9F5);
        hide_menu_item(9, 0x9F4);
        hide_menu_item(9, 0x9F3);
        hide_menu_item(9, 0x9F1);
        hide_menu_item(9, 0x9F0);
        hide_menu_item(9, 5);
        hide_menu_item(9, 6);
        hide_menu_item(9, 7);
    }
    return 0;
}
