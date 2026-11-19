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
// ORIGINAL: 0x004BE9F0 ??_GTutWin@@UAEPAXI@Z 0x004BE9F0-0x004BEA11 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical deleting-thunk source src/deleting_thunks.cpp; original full spans retained: 0x004BE9F0-0x004BEA11", "Pinned class constructor ??0TutWin@@QAE@XZ at 0x004BA6B0 stores original class/view table VA 0x66dd84 at 0x004BA6BD (mov dword ptr [esi], 0x66dd84); the current genuine owner vftable ??_7TutWin@@6BWin@@@ selects generated ??_GTutWin@@UAEPAXI@Z in slot 0 (+0x0) through weak entry ??_ETutWin@@UAEPAXI@Z (mode 2).", "Pinned ??_GTutWin@@UAEPAXI@Z at 0x004BE9F0 calls the source-mapped ??1TutWin@@QAE@XZ at 0x004BE7D0, then tests byte ptr [ebp + 8], 1; je 0x4bea0a bypasses scalar delete when bit 0 is clear, while bit 0 set calls 0x0064557F; it save original this; call destructor; delete only when flags bit 0 is set; return saved this with ret 4.", "The real class lifetime is owned by src/tutwin.cpp; its compiler-emitted deleting symbol and vftable slot carry this original identity. Hand-written adapter bodies with live adjustor callers remain until those callers are independently qualified and transferred. The linked weak/COMDAT copy is not used to choose source ownership.", "the source writer preserves the historical mapping's exactness state (historical exactness obligation retained); current compiler extent is not asserted equal to the original span"], "reason": "Bind the original TutWin scalar deleting-destructor identity to the actual compiler-emitted class deleting destructor in its genuine owner translation unit. The original constructor's pinned vptr store and the current owner's physical table slot identify this class/view; no callback/thunk body is hand-authored here."}
// symbol    ??_GTutWin@@UAEPAXI@Z
// kind      game
// ORIGINAL: 0x004BA5C0 _$E21 0x004BA5C0-0x004BA5D6 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot0x6825f8 selects original initializer0x4ba5c0..0x4ba5d6, registering fini0x4ba5e0..0x4ba5ea; complete independent decode establishes exclusive boundaries.", "Source-owned object?TutorialWindow@@3VTutWin@@A identifies original storage0x8c6e68. Startup arguments/calls: call 0x4ba6b0; push 0x4ba5e0. Cleanup receiver/calls: mov ecx, 0x8c6e68; jmp 0x4be7d0. Constructor/destructor targets retain their existing class-owned source mappings.", "Fresh physical src/tutwin.cpp COFF selects _$E21 through .CRT$XCU and registers _$E19, both referencing the genuine source global. Original NetMsg argument triples and implicit PlanWin/Console most-derived flags are preserved by the existing declarations; complete virtual-base teardown is compiler-owned, not authored offsets.", "Current local helper symbols interpret present emission, not original debug names or historical TU boundaries. Object starts do not prove allocation extents. This binding establishes neither cross-TU global order, masked-reference correctness, strict linked preservation, dependency-provider equivalence nor runtime/game acceptance; excluded guarded sprite cleanup remains separate."], "reason": "Bind original startup identity to the genuine compiler-generated lifetime of ?TutorialWindow@@3VTutWin@@A in its actual source owner; no manual lifecycle callback is authored."}
// symbol    _$E16
// ORIGINAL: 0x004BA5E0 _$E19 0x004BA5E0-0x004BA5EA FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot0x6825f8 selects original initializer0x4ba5c0..0x4ba5d6, registering fini0x4ba5e0..0x4ba5ea; complete independent decode establishes exclusive boundaries.", "Source-owned object?TutorialWindow@@3VTutWin@@A identifies original storage0x8c6e68. Startup arguments/calls: call 0x4ba6b0; push 0x4ba5e0. Cleanup receiver/calls: mov ecx, 0x8c6e68; jmp 0x4be7d0. Constructor/destructor targets retain their existing class-owned source mappings.", "Fresh physical src/tutwin.cpp COFF selects _$E21 through .CRT$XCU and registers _$E19, both referencing the genuine source global. Original NetMsg argument triples and implicit PlanWin/Console most-derived flags are preserved by the existing declarations; complete virtual-base teardown is compiler-owned, not authored offsets.", "Current local helper symbols interpret present emission, not original debug names or historical TU boundaries. Object starts do not prove allocation extents. This binding establishes neither cross-TU global order, masked-reference correctness, strict linked preservation, dependency-provider equivalence nor runtime/game acceptance; excluded guarded sprite cleanup remains separate."], "reason": "Bind original exit identity to the genuine compiler-generated lifetime of ?TutorialWindow@@3VTutWin@@A in its actual source owner; no manual lifecycle callback is authored."}
// symbol    _$E14
#include "stdafx.h"
#include "tutwin.h"
#include "basewin.h"
#include "maininterface.h"
#include "socialwin.h"
#include "designwin.h"

// The tutorial overlay used by InfoWin and MapWin; a real process-lifetime
// object rather than a pointer into the original executable's BSS.
// RECOVERY_OBJECT: v2 0x008C6E68 0x008CC240 ?TutorialWindow@@3VTutWin@@A 0x00458420
TutWin TutorialWindow;
#include "win.h"
#include "mapwin.h"

#include "game.h"     // IsMultiplayerNet, GamePreferences - tut_check's gates
#include "faction.h"  // Players/LocalFaction/RFLAG_ALIEN - the expansion gate
#include "console.h"  // ConsoleGlobal.faction_id_ supplies the active faction
#include "tutorial_script.h"
#include "veh.h"      // the recovered vehicle painter
#include "strings.h"  // StringTemp and the tutorial's decimal label suffixes
#include "map.h"
#include "base.h"
#include "general.h"
#include "technology.h"
#include "worldwin.h"
#include "multiwin.h"
#include "random.h"
#include "text.h"
#include "alpha.h"      // Labels and the typed tutorial-label table

// Defined beside tut_win below: its callers (tour6, tour1) precede it in
// address order, and defined after them it still expands at every site.
static int tut_callout(TutWin &tutorial, Win &coordinates, RECT &rect,
                       const char *label);


// ORIGINAL: 0x004BA5F0 ?tut_check@@YAHH@Z 0x004BA5F0-0x004BA64E FILE BYTE_EXACT
// LEVER: test/push the flag word as a named `extern int`, not a const-address pointer - the pointer spelling lets VC6 prove no alias with the table read, cache it in eax and push/pop esi; a real global may alias, so it reloads and `|=` lowers to load/or/store. Transferred from 0x004BA650, then MEASURED here.
// size      94 bytes
// prototype 
// callers   7   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)

tut_check(int flag) {
    // Gates, in the image's order: no tutorial hints in multiplayer, none
    // unless GamePreferences keeps the hints switch on, none for a hint bit
    // already shown, and none in an expansion alien faction (bit 0x80 of
    // rule_flags is RFLAG_ALIEN - the same gate X_pop's mod-caption check
    // makes). Players is walked by faction because 0x00946F58 is
    // Players[0].rule_flags and the image scales its index by 0x59C.
    if (IsMultiplayerNet != 0) {
        return 0;
    }
    if ((GameDataStateStorage.fields_.game_preferences_ & 0x20) == 0) {
        return 0;
    }
    if ((flag & GameDataStateStorage.fields_.tutorial_flags_) != 0) {
        return 0;
    }
    if ((Players[ConsoleGlobal.faction_id_].rule_flags & RFLAG_ALIEN) != 0) {
        return 0;
    }
    GameDataStateStorage.fields_.tutorial_flags_ |= flag;
    return 1;
}

// ORIGINAL: 0x004BA650 ?tut_check2@@YAHH@Z 0x004BA650-0x004BA6AE FILE BYTE_EXACT
// LEVER: test/push 0x9A64B0 as a named `extern int`, not a const-address pointer: the pointer spelling let VC6 cache the flag word in eax across the table read and push/pop esi for the parameter; a real global may be aliased by the table pointer, so it reloads, keeps `a1` in scratch edx, and `|=` lowers to load/or/store instead of `or [mem], reg`.
// size      94 bytes
// prototype 
// callers   9   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)

tut_check2(int flag) {
    // The second gate, identical in shape to tut_check but with its own
    // shown-bit word, which is what makes the two hint sequences independent.
    if (IsMultiplayerNet != 0) {
        return 0;
    }
    if ((GameDataStateStorage.fields_.game_preferences_ & 0x20) == 0) {
        return 0;
    }
    if ((flag & GameDataStateStorage.fields_.tutorial_flags_2_) != 0) {
        return 0;
    }
    if ((Players[ConsoleGlobal.faction_id_].rule_flags & RFLAG_ALIEN) != 0) {
        return 0;
    }
    GameDataStateStorage.fields_.tutorial_flags_2_ |= flag;
    return 1;
}



// ---------------------------------------------------------------------------
// DEFINED IN THE HEADER, CLAIMED HERE.
//
// These pieces are written in-class so the image's own inlining reproduces -
// a constructor or destructor the compiler is expected to fold into its
// caller. A marker cannot live beside them: `decomp.reader` globs `*.cpp`
// and `*.c`, and a comparison compiles a TRANSLATION UNIT, so a marker in a
// header could be neither read nor measured. VC6 emits each of them into
// this unit's object as its own COMDAT anyway, which is what the comparison
// pulls out, and the `body` fact says where to go to edit one.
//
// The ratchet still covers the header: this unit includes it, so breaking an
// in-class body here fails the claim below. Measured, not assumed.
// ---------------------------------------------------------------------------

/*
// ORIGINAL: 0x004BA6B0 ??0TutWin@@QAE@XZ 0x004BA6B0-0x004BA71E BYTE_EXACT
// body      src/tutwin.h
// size      110 bytes
// prototype void (__thiscall ??0TutWin@@QAE@XZ)(TutWin* this)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004048A0
*/

// LEVER: `: public Popup` real inheritance, so the single call is the
// implicit base constructor (now BYTE_EXACT, see popup.cpp) - no explicit
// `Popup::Popup()` call needed, and (matching Popup's own fix) the trailing
// stores use a non-volatile pointer.
TutWin::TutWin() {
    field_53B4_ = 0;
    field_53C0_ = 0;
    field_53D0_ = 0;
    field_53D4_ = 0;
    field_53A4_ = 0;
    field_5380_ = -1;
    field_537C_ = -1;
    field_539C_ = -1;
    field_53A8_ = 0;
    field_53AC_ = -1;
    field_53B8_ = 0;
    field_53C4_ = 0;
    TutWinShownFlag = 0;
}

/*
Purpose: Reset the tutorial window - four fields to zero, four to -1, and the
         shared shown-marker cleared with them.
// ORIGINAL: 0x004BA720 ?UNK1@TutWin@@QAEXXZ 0x004BA720-0x004BA761 BYTE_EXACT
// size      65 bytes
// prototype void (__thiscall ?UNK1@TutWin@@QAEXXZ)(TutWin* this)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void TutWin::UNK1() {
    field_53D4_ = 0;
    field_53A4_ = 0;
    field_5380_ = -1;
    field_537C_ = -1;
    field_539C_ = -1;
    field_53A8_ = 0;
    field_53AC_ = -1;
    field_53B8_ = 0;
    field_53C4_ = 0;
    TutWinShownFlag = 0;
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004BA770 ?on_key_click@TutWin@@QAEHHH@Z 0x004BA770-0x004BA82E BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x004BA770-0x004BA82E; seed:src/unrecovered/004ba770.cpp; source:src/win.h:Win::release_modal,Win::hide,Win::on_update"], "reason": "The tutorial key callback owns its modal dismissal, saved key tuple, and TutWin state reset."}
// symbol    ?on_key_click@TutWin@@UAEHHH@Z
int TutWin::on_key_click(int key, int code) {
    if (field_53B8_ != 0) {
        release_modal();
        if (field_53D4_ != 0) {
            field_53A4_ = 0;
            hide();
            field_53D4_->on_update();
            do_all_draws();
        }
        field_53D4_ = 0;
        field_5380_ = -1;
        field_537C_ = -1;
        field_539C_ = -1;
        field_53AC_ = -1;
        field_53A4_ = 0;
        field_53A8_ = 0;
        field_53B8_ = 0;
        field_53C4_ = 0;
        TutWinShownFlag = 0;
        if (code == 0x1b) {
            field_53B4_ = 0;
        }
        field_53C4_ = 1;
        field_53C8_ = key;
        field_53CC_ = code;
        return 1;
    }
    return 0;
}

// Reset at 0x004BDDE0 clears this process-wide marker after the tutorial's
// own state; it is separate from TutorialWindow, whose extent ends at 8CC240.
// RECOVERY_OBJECT: v2 0x008CC244 0x008CC248 ?TutWinShownFlag@@3IA 0x004BDDE0
uint32_t TutWinShownFlag;
// The adjacent process flags are independently owned by the tutorial
// window. Their fixed-image identities are recorded by the coordinator.
// RECOVERY_OBJECT: v1 0x008CC248 ?TutWinTourPending@@3HA
int TutWinTourPending;
// Historical object identity moved to GameDataStateStorage; see the transport manifest.

// Tutorial tour's original 0x945830 gate controls WorldClimateWorldWin's
// restoration. It is source-owned, writable state with loader-zero initial
// storage; no writer or larger object owner is established.
// RECOVERY_OBJECT: v1 0x00945830 ?TutWinClimateGate945830@@3HA
int TutWinClimateGate945830;

// THE TWO TUTORIAL-SHOWN BIT SETS, one per checker. tut_check and tut_check2
// each OR their argument in and refuse to fire again while the bit is set;
// the split into two words is what makes them independent hint sequences.
// Both must be real globals (not const-address bindings) or VC6 caches the
// word across the table read and the `|=` degrades to a load/or/store.
// 0x009A64AC
// 0x009A64B0

/*
Purpose: Compute a rectangle center with wrapping subtraction and truncation toward zero.
// ORIGINAL: 0x004BA830 ?UNK2@TutWin@@QAEXPAURECT@@PAHPAH@Z 0x004BA830-0x004BA863 BYTE_EXACT
// LEVER: MORE (2 calls vs image's 0: midpoint/int_from_bits helpers never inline) - the image writes the signed halving directly (`sub;cdq;sub;sar;add`), not through any helper, and the tail is `ret 0xc` (callee-cleans), not `ret` - the ORIGINAL is a thiscall member ignoring `this` entirely, and this free-function stand-in needs `__stdcall` (not `__cdecl`) to match that stack cleanup. Rewriting the body as plain field arithmetic and switching the calling convention took 0/25 (0.553 best) to 24/25 (0.960 best, /Oy- flag set) with 0 calls disagreeing.
// symbol    ?rect_center@@YGHPAUtagRECT@@PAH1@Z
// size      51 bytes
// prototype void (__thiscall ?UNK2@TutWin@@QAEXPAURECT@@PAHPAH@Z)(TutWin* this, RECT*, int*, int*)
// callers   0   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
int __stdcall rect_center(RECT *rect, int *x, int *y) {
    *x = rect->left + (rect->right - rect->left) / 2;
    int center_y = rect->top + (rect->bottom - rect->top) / 2;
    *y = center_y;
    return center_y;
}
// WORKAROUND(noinline): tour1/tour5/tour7 call do_base/do_iface/do_soc/do_des
// out of line (and BaseWin::zoom calls do_base); /Ob2 expands them here.
#pragma auto_inline(off)


/*
Purpose: Centre the rectangle, convert it through the base window, and
         show the tutorial text there against the primary map window.
// ORIGINAL: 0x004BA870 ?do_base@TutWin@@QAEXPAURECT@@PBDH@Z 0x004BA870-0x004BA8EB BYTE_EXACT
// LEVER: two levers stacked, 4/56 -> 45/56 MNEMONIC_ONLY. (1) plain `left + (right - left) / 2` signed division instead of the unsigned-with-sign-carried trick, same as `iface_rect`. (2) `primary` read BEFORE `window` is given any value, and its null-check written `if (primary == nullptr) { window = nullptr; } else { ...compute... }` (primary-first declaration order, and the null case as the FIRST arm) - the image reads the global first and only zeroes the result register on the branch that skips the vtable dereference, both of which a `window = nullptr;`-then-test body reorders.
// TRIED: 45/56 plateau - the remainder is a consistent eax/ecx role swap through the vbtable-dereference block and the final argument push order, tried: `uint8_t *` instead of `void *` for `window`, and folding the `vbtable` local into the expression directly - neither changes it. VC6 register allocation, not a source-form lever found here.
// symbol    ?do_base@TutWin@@QAEHPAUtagRECT@@PBDH@Z
// size      123 bytes
// prototype void (__thiscall ?do_base@TutWin@@QAEXPAURECT@@PBDH@Z)(TutWin* this, RECT*, int8*, int)
// callers   2   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004BDFE0 0x005ED240
Return Value: the callout result, consumed by the tutorial's continuation gates
Verification note: the GraphicWin handed to the show is MapWinTable[0]
         VIRTUAL-BASE ADJUSTED - `mov edx,[ecx]` / `mov eax,[edx+4]` /
         `add eax,ecx` - and a null slot passes a null through rather than
         faulting, which is the only branch in the body.
Verification note: the original writes the y centre over its own first
         argument slot before taking its address, so the RECT pointer is dead
         from that point. A local is equivalent and is what is used here.
*/
int TutWin::do_base(RECT *rect, const char *text, int flag) {
    // Plain signed division: the image emits `cdq; sub eax,edx; sar eax,1`
    // (round-toward-zero) for each half, not a rounding bit-trick.
    int x = static_cast<int32_t>(static_cast<uint32_t>(rect->left) +
        static_cast<uint32_t>(static_cast<int32_t>(
            static_cast<uint32_t>(rect->right) - static_cast<uint32_t>(rect->left)) / 2));
    int y = static_cast<int32_t>(static_cast<uint32_t>(rect->top) +
        static_cast<uint32_t>(static_cast<int32_t>(
            static_cast<uint32_t>(rect->bottom) - static_cast<uint32_t>(rect->top)) / 2));
    TutWinBaseWindow.client_to_screen(&x, &y);
    return this->tut_win(MapWinTable[0], text, x, y, nullptr, flag, -1, -1);
}


/*
Purpose: Centre the rectangle, convert it through the iface window, and
         show the tutorial text there against the primary map window.
// ORIGINAL: 0x004BA8F0 ?do_iface@TutWin@@QAEXPAURECT@@PBDH@Z 0x004BA8F0-0x004BA96B BYTE_EXACT
// LEVER: same two levers as `do_base` (see that marker) - plain signed division, and `primary` read/null-checked before `window` is given any value. 4/56 -> 45/56 MNEMONIC_ONLY.
// TRIED: same 45/56 plateau as `do_base` - a consistent eax/ecx register swap through the vbtable block; see that marker for what was tried.
// symbol    ?do_iface@TutWin@@QAEHPAUtagRECT@@PBDH@Z
// size      123 bytes
// prototype void (__thiscall ?do_iface@TutWin@@QAEXPAURECT@@PBDH@Z)(TutWin* this, RECT*, int8*, int)
// callers   1   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004BDFE0 0x005ED240
Return Value: the callout result, consumed by the tutorial's continuation gates
Verification note: the GraphicWin handed to the show is MapWinTable[0]
         VIRTUAL-BASE ADJUSTED - `mov edx,[ecx]` / `mov eax,[edx+4]` /
         `add eax,ecx` - and a null slot passes a null through rather than
         faulting, which is the only branch in the body.
Verification note: the original writes the y centre over its own first
         argument slot before taking its address, so the RECT pointer is dead
         from that point. A local is equivalent and is what is used here.
*/
int TutWin::do_iface(RECT *rect, const char *text, int flag) {
    // Plain signed division: the image emits `cdq; sub eax,edx; sar eax,1`
    // (round-toward-zero) for each half, not a rounding bit-trick.
    int x = static_cast<int32_t>(static_cast<uint32_t>(rect->left) +
        static_cast<uint32_t>(static_cast<int32_t>(
            static_cast<uint32_t>(rect->right) - static_cast<uint32_t>(rect->left)) / 2));
    int y = static_cast<int32_t>(static_cast<uint32_t>(rect->top) +
        static_cast<uint32_t>(static_cast<int32_t>(
            static_cast<uint32_t>(rect->bottom) - static_cast<uint32_t>(rect->top)) / 2));
    MainInterfaceGlobal.client_to_screen(&x, &y);
    return this->tut_win(MapWinTable[0], text, x, y, nullptr, flag, -1, -1);
}


/*
Purpose: Centre the rectangle, convert it through the soc window, and
         show the tutorial text there against the primary map window.
// ORIGINAL: 0x004BA970 ?do_soc@TutWin@@QAEXPAURECT@@PBDH@Z 0x004BA970-0x004BA9EB BYTE_EXACT
// LEVER: same two levers as `do_base` (see that marker) - plain signed division, and `primary` read/null-checked before `window` is given any value. 4/56 -> 45/56 MNEMONIC_ONLY.
// TRIED: same 45/56 plateau as `do_base` - a consistent eax/ecx register swap through the vbtable block; see that marker for what was tried.
// symbol    ?do_soc@TutWin@@QAEHPAUtagRECT@@PBDH@Z
// size      123 bytes
// prototype void (__thiscall ?do_soc@TutWin@@QAEXPAURECT@@PBDH@Z)(TutWin* this, RECT*, int8*, int)
// callers   1   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004BDFE0 0x005ED240
Return Value: the callout result, consumed by the tutorial's continuation gates
Verification note: the GraphicWin handed to the show is MapWinTable[0]
         VIRTUAL-BASE ADJUSTED - `mov edx,[ecx]` / `mov eax,[edx+4]` /
         `add eax,ecx` - and a null slot passes a null through rather than
         faulting, which is the only branch in the body.
Verification note: the original writes the y centre over its own first
         argument slot before taking its address, so the RECT pointer is dead
         from that point. A local is equivalent and is what is used here.
*/
int TutWin::do_soc(RECT *rect, const char *text, int flag) {
    // Plain signed division: the image emits `cdq; sub eax,edx; sar eax,1`
    // (round-toward-zero) for each half, not a rounding bit-trick.
    int x = static_cast<int32_t>(static_cast<uint32_t>(rect->left) +
        static_cast<uint32_t>(static_cast<int32_t>(
            static_cast<uint32_t>(rect->right) - static_cast<uint32_t>(rect->left)) / 2));
    int y = static_cast<int32_t>(static_cast<uint32_t>(rect->top) +
        static_cast<uint32_t>(static_cast<int32_t>(
            static_cast<uint32_t>(rect->bottom) - static_cast<uint32_t>(rect->top)) / 2));
    TutWinSocWindow.client_to_screen(&x, &y);
    return this->tut_win(MapWinTable[0], text, x, y, nullptr, flag, -1, -1);
}


/*
Purpose: Centre the rectangle, convert it through the des window, and
         show the tutorial text there against the primary map window.
// ORIGINAL: 0x004BA9F0 ?do_des@TutWin@@QAEXPAURECT@@PBDH@Z 0x004BA9F0-0x004BAA6B BYTE_EXACT
// LEVER: same two levers as `do_base` (see that marker) - plain signed division, and `primary` read/null-checked before `window` is given any value. 4/56 -> 45/56 MNEMONIC_ONLY.
// TRIED: same 45/56 plateau as `do_base` - a consistent eax/ecx register swap through the vbtable block; see that marker for what was tried.
// symbol    ?do_des@TutWin@@QAEHPAUtagRECT@@PBDH@Z
// size      123 bytes
// prototype void (__thiscall ?do_des@TutWin@@QAEXPAURECT@@PBDH@Z)(TutWin* this, RECT*, int8*, int)
// callers   1   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004BDFE0 0x005ED240
Return Value: the callout result, consumed by the tutorial's continuation gates
Verification note: the GraphicWin handed to the show is MapWinTable[0]
         VIRTUAL-BASE ADJUSTED - `mov edx,[ecx]` / `mov eax,[edx+4]` /
         `add eax,ecx` - and a null slot passes a null through rather than
         faulting, which is the only branch in the body.
Verification note: the original writes the y centre over its own first
         argument slot before taking its address, so the RECT pointer is dead
         from that point. A local is equivalent and is what is used here.
*/
int TutWin::do_des(RECT *rect, const char *text, int flag) {
    // Plain signed division: the image emits `cdq; sub eax,edx; sar eax,1`
    // (round-toward-zero) for each half, not a rounding bit-trick.
    int x = static_cast<int32_t>(static_cast<uint32_t>(rect->left) +
        static_cast<uint32_t>(static_cast<int32_t>(
            static_cast<uint32_t>(rect->right) - static_cast<uint32_t>(rect->left)) / 2));
    int y = static_cast<int32_t>(static_cast<uint32_t>(rect->top) +
        static_cast<uint32_t>(static_cast<int32_t>(
            static_cast<uint32_t>(rect->bottom) - static_cast<uint32_t>(rect->top)) / 2));
    TutWinDesWindow.client_to_screen(&x, &y);
    return this->tut_win(MapWinTable[0], text, x, y, nullptr, flag, -1, -1);
}
#pragma auto_inline(on)

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004BAA70 ?tour6@TutWin@@QAEXXZ 0x004BAA70-0x004BAF12 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["seed:src/recovered/units/004baa70.cpp", "image:0x004baa70; original call/rectangle/lifetime review"], "reason": "Restore the original ownership-group implementation through real tutorial/editor windows, declared members and normal C++ lifecycle; retained seed extents include the associated exception cleanup where present."}
void TutWin::tour6() {
    if (!TutWinBaseWindow.is_visible()) {
        for (int i = 0; i < GameDataStateStorage.fields_.base_current_count_; ++i) {
            if (Bases[i].faction_id_current == ConsoleGlobal.faction_id_) {
                TutWinBaseWindow.zoom(i, 0);
                break;
            }
        }
        if (!TutWinBaseWindow.is_visible()) return;
    }

    TutWinBaseWindow.focus();
    parse_says(0, BaseCurrent->name_string, -1, -1);
    int x;
    int y;
    // DRONETOUR1/5 pass the unconverted client left edge + 5 as x.
    x = TutWinBaseWindow.population_rect_.left + (TutWinBaseWindow.population_rect_.right - TutWinBaseWindow.population_rect_.left) / 2;
    y = TutWinBaseWindow.population_rect_.top + (TutWinBaseWindow.population_rect_.bottom - TutWinBaseWindow.population_rect_.top) / 2;
    MainInterfaceGlobal.client_to_screen(&x, &y);
    if (!tut_win(MapWinTable[0], "DRONETOUR1", TutWinBaseWindow.population_rect_.left + 5, y, 0, 2, -1, -1)) return;
    if (!tut_callout(*this, MainInterfaceGlobal, TutWinBaseWindow.garrison_rect_, "DRONETOUR2")) return;
    if (!tut_callout(*this, MainInterfaceGlobal, TutWinBaseWindow.rect12_, "DRONETOUR3")) return;
    if (!tut_callout(*this, MainInterfaceGlobal, TutWinBaseWindow.flash_rect_a_, "DRONETOUR4")) return;
    x = TutWinBaseWindow.population_rect_.left + (TutWinBaseWindow.population_rect_.right - TutWinBaseWindow.population_rect_.left) / 2;
    y = TutWinBaseWindow.population_rect_.top + (TutWinBaseWindow.population_rect_.bottom - TutWinBaseWindow.population_rect_.top) / 2;
    MainInterfaceGlobal.client_to_screen(&x, &y);
    if (!tut_win(MapWinTable[0], "DRONETOUR5", TutWinBaseWindow.population_rect_.left + 5, y, 0, 2, -1, -1)) return;

    TutWinBaseWindow.draw_energy_alloc(2);
    if (!TutWinBaseWindow.flatButtons_[17].is_visible()) {
        x = TutWinBaseWindow.expenses_help_rects_[5].left + (TutWinBaseWindow.expenses_help_rects_[5].right - TutWinBaseWindow.expenses_help_rects_[5].left) / 2;
        y = TutWinBaseWindow.expenses_help_rects_[5].top + (TutWinBaseWindow.expenses_help_rects_[5].bottom - TutWinBaseWindow.expenses_help_rects_[5].top) / 2;
        TutWinBaseWindow.client_to_screen(&x, &y);
        if (!tut_win(MapWinTable[0], "DRONETOUR6", x, y, 0, 2, -1, -1)) return;
    } else {
        if (!tut_callout(*this, TutWinBaseWindow, TutWinBaseWindow.rect13_, "DRONETOUR6")) return;
    }
    TutWinBaseWindow.draw_expenses(1);
    TutWinBaseWindow.draw_farm(2);
    if (tut_callout(*this, TutWinBaseWindow, TutWinBaseWindow.rect11_, "TOUR1PSYCHBUTTON")) {
        TutWinBaseWindow.draw_farm(1);
    }
}


// Superseded seed deleted; seed paths below are historical Git provenance.
// RECOVERY_LITERAL: v1 0x00687630 0x0068763B $SG55756 0x004BAF20
// RECOVERY_LITERAL: v1 0x00687590 0x0068759B $SG55705 0x004BAF20
// RECOVERY_LITERAL: v1 0x006875D8 0x006875E3 $SG55730 0x004BAF20
// RECOVERY_LITERAL: v1 0x00687604 0x00687610 $SG55745 0x004BAF20
// RECOVERY_LITERAL: v1 0x00687584 0x00687590 $SG55700 0x004BAF20
// RECOVERY_LITERAL: v1 0x006875BC 0x006875C8 $SG55720 0x004BAF20
// RECOVERY_LITERAL: v1 0x00687578 0x00687584 $SG55694 0x004BAF20
// RECOVERY_LITERAL: v1 0x006875C8 0x006875D5 $SG55725 0x004BAF20
// RECOVERY_LITERAL: v1 0x00687610 0x0068761D $SG55750 0x004BAF20
// RECOVERY_LITERAL: v1 0x0068764C 0x0068765A $SG55762 0x004BAF20
// RECOVERY_LITERAL: v1 0x00687620 0x0068762E $SG55753 0x004BAF20
// RECOVERY_LITERAL: v1 0x006875E4 0x006875F2 $SG55735 0x004BAF20
// RECOVERY_LITERAL: v1 0x0068765C 0x0068766A $SG55764 0x004BAF20
// RECOVERY_LITERAL: v1 0x0068763C 0x0068764B $SG55759 0x004BAF20
// RECOVERY_LITERAL: v1 0x006875AC 0x006875BB $SG55715 0x004BAF20
// RECOVERY_LITERAL: v1 0x006875F4 0x00687603 $SG55740 0x004BAF20
// RECOVERY_LITERAL: v1 0x0068759C 0x006875AB $SG55710 0x004BAF20
// ORIGINAL: 0x004BAF20 ?tour7@TutWin@@QAEXXZ 0x004BAF20-0x004BB373 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["seed:src/recovered/units/004baf20.cpp", "image:0x004baf20; original call/rectangle/lifetime review"], "reason": "Restore the original ownership-group implementation through real tutorial/editor windows, declared members and normal C++ lifecycle; retained seed extents include the associated exception cleanup where present."}
void TutWin::tour7() {
    const int was_visible = TutWinDesWindow.is_visible();

    if (!was_visible) {
        TutWinDesWindow.exec(ConsoleGlobal.faction_id_, -2);
    }


    {
        int x;
        int y;
        des_rect(&TutWinDesWindow.component_panel_rects_[
            DesignWin::component_panel_chassis], &x, &y);
        if (tut_win(MapWinTable[0],
        "DESTOURCHAS", x, y, 0, 2, -1, -1) == 0) goto tour7_cleanup;
    }
    {
        int y;
        int x;
        des_rect(&TutWinDesWindow.component_panel_rects_[
            DesignWin::component_panel_weapon], &x, &y);
        if (tut_win(MapWinTable[0],
        "DESTOURWEAP", x, y, 0, 2, -1, -1) == 0) goto tour7_cleanup;
    }
    {
        int y;
        int x;
        des_rect(&TutWinDesWindow.component_panel_rects_[
            DesignWin::component_panel_armor], &x, &y);
        if (tut_win(MapWinTable[0],
        "DESTOURARM", x, y, 0, 2, -1, -1) == 0) goto tour7_cleanup;
    }
    {
        int y;
        int x;
        des_rect(&TutWinDesWindow.component_panel_rects_[
            DesignWin::component_panel_reactor], &x, &y);
        if (tut_win(MapWinTable[0],
        "DESTOURREACTOR", x, y, 0, 2, -1, -1) == 0) goto tour7_cleanup;
    }
    {
        int y;
        int x;
        des_rect(&TutWinDesWindow.component_panel_rects_[
            DesignWin::component_panel_special_one], &x, &y);
        if (tut_win(MapWinTable[0],
        "DESTOURABILITY", x, y, 0, 2, -1, -1) == 0) goto tour7_cleanup;
    }
    {
        int y;
        int x;
        des_rect(&TutWinDesWindow.field_B2C_rect_, &x, &y);
        if (tut_win(MapWinTable[0],
        "DESTOURCOST", x, y, 0, 2, -1, -1) == 0) goto tour7_cleanup;
    }
    {
        int y;
        int x;
        des_rect(&TutWinDesWindow.stats_rect_, &x, &y);
        if (tut_win(MapWinTable[0],
        "DESTOURSTATS", x, y, 0, 2, -1, -1) == 0) goto tour7_cleanup;
    }
    {
        int y;
        int x;
        iface_rect(&TutWinDesWindow.field_B0C_rect_, &x, &y);
        if (tut_win(MapWinTable[0],
        "DESTOURBAR", x, y, 0, 2, -1, -1) == 0) goto tour7_cleanup;
    }
    {
        int y;
        int x;
        iface_rect(&TutWinDesWindow.field_CBC_rect_, &x, &y);
        if (tut_win(MapWinTable[0],
        "DESTOURARROWS", x, y, 0, 2, -1, -1) == 0) goto tour7_cleanup;
    }
    {
        int y;
        int x;
        iface_rect(&TutWinDesWindow.caption_rect_, &x, &y);
        if (tut_win(MapWinTable[0],
        "DESTOURPRESETS", x, y, 0, 2, -1, -1) == 0) goto tour7_cleanup;
    }
    {
        int y;
        int x;
        des_rect(&TutWinDesWindow.lower_panel_rows_[DesignWin::lower_panel_done], &x, &y);
        if (tut_win(MapWinTable[0],
        "DESTOURDONE", x, y, 0, 2, -1, -1) == 0) goto tour7_cleanup;
    }
    {
        int y;
        int x;
        des_rect(&TutWinDesWindow.field_B5C_rect_, &x, &y);
        if (tut_win(MapWinTable[0],
        "DESTOURAPPLY", x, y, 0, 2, -1, -1) == 0) goto tour7_cleanup;
    }
    if (!do_des(&TutWinDesWindow.lower_panel_rows_[DesignWin::lower_panel_rename],
                "DESTOURRENAME", 2))
        goto tour7_cleanup;
    if (!do_des(&TutWinDesWindow.lower_panel_rows_[DesignWin::lower_panel_obsolete],
                "DESTOUROBS", 2))
        goto tour7_cleanup;
    if (!do_des(&TutWinDesWindow.lower_panel_rows_[DesignWin::lower_panel_upgrade],
                "DESTOURUPGRADE", 2))
        goto tour7_cleanup;
    if (!do_des(&TutWinDesWindow.field_B4C_rect_, "DESTOURRETIRE", 2))
        goto tour7_cleanup;
    do_des(&TutWinDesWindow.lower_panel_rows_[DesignWin::lower_panel_cancel],
           "DESTOURCANCEL", 2);

tour7_cleanup:
    if (!was_visible) {
        TutWinDesWindow.shut_that_badboy_down();
    }
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004BB380 ?tour5@TutWin@@QAEXXZ 0x004BB380-0x004BB791 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["seed:src/recovered/units/004bb380.cpp", "image:0x004bb380; original call/rectangle/lifetime review"], "reason": "Restore the original ownership-group implementation through real tutorial/editor windows, declared members and normal C++ lifecycle; retained seed extents include the associated exception cleanup where present."}
void TutWin::tour5() {
    const int was_visible = TutWinSocWindow.is_visible();
    int i;

    if (!was_visible) {
        TutWinSocWindow.init(-ConsoleGlobal.faction_id_);
        TutWinSocWindow.show(0);
    }


    {
        int x;
        int y;
        soc_rect(&TutWinSocWindow.categoryHeaders_[1], &x, &y);
        if (tut_win(MapWinTable[0],
        "SOCTOURMAIN", x, y, 0, 2, -1, -1) == 0) goto tour5_cleanup;
    }
    {
        int x;
        int y;
        soc_rect(&TutWinSocWindow.tooltipFactionPower_, &x, &y);
        if (tut_win(MapWinTable[0],
        "SOCTOURFACTION", x, y, 0, 2, -1, -1) == 0) goto tour5_cleanup;
    }
    {
        int x;
        int y;
        iface_rect(&TutWinSocWindow.tooltipEffects_, &x, &y);
        if (tut_win(MapWinTable[0],
        "SOCTOUROVERVIEW", x, y, 0, 2, -1, -1) == 0) goto tour5_cleanup;
    }
    {
        int x;
        int y;
        soc_rect(&TutWinSocWindow.tutRect1_, &x, &y);
        if (tut_win(MapWinTable[0],
        "SOCTOUREFFECTS", x, y, 0, 2, -1, -1) == 0) goto tour5_cleanup;
    }
    for (i = 0; i < 10; ++i) {
        StringTemp[0] = 0;
        strcat(StringTemp, "SOCTOUR");
        say_num(i);
        int x;
        int y;
        soc_rect(&TutWinSocWindow.tutRects2_[i], &x, &y);
        if (tut_win(MapWinTable[0],
                    StringTemp, x, y, 0, 2, -1, -1) == 0) goto tour5_cleanup;
    }

    {
        int x;
        int y;
        iface_rect(&TutWinSocWindow.tooltipEconomy_, &x, &y);
        if (tut_win(MapWinTable[0],
        "SOCTOURECON", x, y, 0, 2, -1, -1) == 0) goto tour5_cleanup;
    }
    {
        int x;
        int y;
        iface_rect(&TutWinSocWindow.tooltipPsych_, &x, &y);
        if (tut_win(MapWinTable[0],
        "SOCTOURPSYCH", x, y, 0, 2, -1, -1) == 0) goto tour5_cleanup;
    }
    {
        int x;
        int y;
        iface_rect(&TutWinSocWindow.tooltipLabs_, &x, &y);
        if (tut_win(MapWinTable[0],
        "SOCTOURLABS", x, y, 0, 2, -1, -1) == 0) goto tour5_cleanup;
    }
    {
        int x;
        int y;
        iface_rect(&TutWinSocWindow.tutRect14_, &x, &y);
        if (tut_win(MapWinTable[0],
        "SOCTOURINFO", x, y, 0, 2, -1, -1) == 0) goto tour5_cleanup;
    }
    {
        int x;
        int y;
        soc_rect(&TutWinSocWindow.tutRect11_, &x, &y);
        if (tut_win(MapWinTable[0],
        "SOCTOURCANCEL", x, y, 0, 2, -1, -1) == 0) goto tour5_cleanup;
    }
    {
        int x;
        int y;
        soc_rect(&TutWinSocWindow.tutRect12_, &x, &y);
        if (tut_win(MapWinTable[0],
        "SOCTOURRESET", x, y, 0, 2, -1, -1) == 0) goto tour5_cleanup;
    }
    do_soc(&TutWinSocWindow.tutRect13_, "SOCTOUROK", 2);

tour5_cleanup:
    if (!was_visible) {
        TutWinSocWindow.hide();
        TutWinSocWindow.release_iface_mode();
        TutWinSocWindow.close();
    }
}

// ORIGINAL: 0x004BB7A0 ?tour1@TutWin@@QAEXXZ 0x004BB7A0-0x004BC591 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["seed:src/unrecovered/004bb7a0.cpp", "image:0x004bb7a0; original call/rectangle/lifetime review"], "reason": "Restore the original ownership-group implementation through real tutorial/editor windows, declared members and normal C++ lifecycle; retained seed extents include the associated exception cleanup where present."}
// TRIED: inline-casting the 0x6a7628/0x6c0df4/0x7ae820 receivers at every call site (no cached local pointer) matches from #0; remaining divergence is that /O2 and /O1 both hoist the ebx/edi push for the zoom-search loop into the true prologue, while the original defers it to just inside the `!is_visible()` branch and pops it back out before the giant guard-clause chain. Tried plain casts, reordering locals (count/target/rec/idx) to match register assignment order - none moved the push. Flat guard-clause control flow (if(!x) return;) reproduces the CFG; all ~30 repeated tut_win/iface_rect/base_rect/client_to_screen blocks and the ALLOC/ALLOC2 branch and do_iface/do_base tail are transcribed directly from the disassembly.
// Callouts reload the primary map owner after each coordinate conversion;
// preceding tutorial callbacks can replace that window. Rectangle-helper
// operands are the actual BaseWin members, not snapshots of their storage.
void TutWin::tour1() {
    if (!TutWinBaseWindow.is_visible()) {
        for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; ++base_id) {
            if (Bases[base_id].faction_id_current == ConsoleGlobal.faction_id_) {
                TutWinBaseWindow.zoom(base_id, 0);
                break;
            }
        }
        if (!TutWinBaseWindow.is_visible()) return;
    }

    TutWinBaseWindow.focus();
    parse_says(0, BaseCurrent->name_string, -1, -1);
    TutWinBaseWindow.draw_farm(4);

    int x;
    int y;

    if (!tut_callout(*this, TutWinBaseWindow, TutWinBaseWindow.rect8_, "TOUR1GOVERNOR")) return;
    if (!tut_callout(*this, TutWinBaseWindow, TutWinBaseWindow.rect14_, "TOUR1EXPLORE")) return;
    if (!tut_callout(*this, TutWinBaseWindow, TutWinBaseWindow.rect15_, "TOUR1DISCOVER")) return;
    if (!tut_callout(*this, TutWinBaseWindow, TutWinBaseWindow.rect18_, "TOUR1BUILD")) return;
    if (!tut_callout(*this, TutWinBaseWindow, TutWinBaseWindow.rect19_, "TOUR1CONQUER")) return;
    if (!tut_callout(*this, TutWinBaseWindow, TutWinBaseWindow.rect8_, "TOUR1GOVERNOR2")) return;
    if (!tut_callout(*this, TutWinBaseWindow, TutWinBaseWindow.rect16_, "TOUR1ADVANCED")) return;

    {
        int x;
        int y;
        iface_rect(&TutWinBaseWindow.info_title_rect_, &x, &y);
        if (tut_win(MapWinTable[0], "TOUR1CAPTION", x, y, 0, 2, -1, -1) == 0) return;
    }
    {
        int x;
        int y;
        base_rect(&TutWinBaseWindow.rect5_, &x, &y);
        if (tut_win(MapWinTable[0], "TOUR1ECO", x, y, 0, 2, -1, -1) == 0) return;
    }

    x = TutWinBaseWindow.population_rect_.left +
        (TutWinBaseWindow.population_rect_.right - TutWinBaseWindow.population_rect_.left) / 2;
    y = TutWinBaseWindow.population_rect_.top +
        (TutWinBaseWindow.population_rect_.bottom - TutWinBaseWindow.population_rect_.top) / 2;
    MainInterfaceGlobal.client_to_screen(&x, &y);
    if (tut_win(MapWinTable[0], "TOUR1POP", x, y, 0, 2, -1, -1) == 0) return;
    if (tut_win(MapWinTable[0], "TOUR1POP2", x, y, 0, 2, -1, -1) == 0) return;

    {
        int x;
        int y;
        iface_rect(&TutWinBaseWindow.rect12_, &x, &y);
        if (tut_win(MapWinTable[0], "TOUR1STAPLE", x, y, 0, 2, -1, -1) == 0) return;
    }
    {
        int x;
        int y;
        iface_rect(&TutWinBaseWindow.queue_help_rect_, &x, &y);
        if (tut_win(MapWinTable[0], "TOUR1PRODUCTION", x, y, 0, 2, -1, -1) == 0) return;
    }
    {
        int x;
        int y;
        iface_rect(&TutWinBaseWindow.queue_content_rect_, &x, &y);
        if (tut_win(MapWinTable[0], "TOUR1QUEUE", x, y, 0, 2, -1, -1) == 0) return;
    }
    {
        int x;
        int y;
        iface_rect(&TutWinBaseWindow.flash_rect_a_, &x, &y);
        if (tut_win(MapWinTable[0], "TOUR1CHANGE", x, y, 0, 2, -1, -1) == 0) return;
    }
    {
        int x;
        int y;
        iface_rect(&TutWinBaseWindow.flash_rect_b_, &x, &y);
        if (tut_win(MapWinTable[0], "TOUR1HURRY", x, y, 0, 2, -1, -1) == 0) return;
    }
    {
        int x;
        int y;
        iface_rect(&TutWinBaseWindow.garrison_rect_, &x, &y);
        if (tut_win(MapWinTable[0], "TOUR1GARRISON", x, y, 0, 2, -1, -1) == 0) return;
    }
    {
        int x;
        int y;
        iface_rect(&TutWinBaseWindow.supported_content_rect_, &x, &y);
        if (tut_win(MapWinTable[0], "TOUR1SUPPORTED", x, y, 0, 2, -1, -1) == 0) return;
    }
    {
        int x;
        int y;
        base_rect(&TutWinBaseWindow.rect2_, &x, &y);
        if (tut_win(MapWinTable[0], "TOUR1FACILITIES", x, y, 0, 2, -1, -1) == 0) return;
    }
    {
        int x;
        int y;
        base_rect(&TutWinBaseWindow.commerce_report_rect_, &x, &y);
        if (tut_win(MapWinTable[0], "TOUR1COMMERCE", x, y, 0, 2, -1, -1) == 0) return;
    }
    {
        int x;
        int y;
        base_rect(&TutWinBaseWindow.nutrient_content_rect_, &x, &y);
        if (tut_win(MapWinTable[0], "TOUR1NUTRIENTS", x, y, 0, 2, -1, -1) == 0) return;
    }

    {
        int x;
        int y;
        base_rect(&TutWinBaseWindow.rect1_, &x, &y);
        if (tut_win(MapWinTable[0], "TOUR1AREA", x, y, 0, 2, -1, -1) == 0) return;
    }
    TutWinBaseWindow.draw_expenses(2);
    x = TutWinBaseWindow.expenses_help_rects_[1].left + (TutWinBaseWindow.expenses_help_rects_[1].right - TutWinBaseWindow.expenses_help_rects_[1].left) / 2;
    y = TutWinBaseWindow.expenses_help_rects_[1].top + (TutWinBaseWindow.expenses_help_rects_[1].bottom - TutWinBaseWindow.expenses_help_rects_[1].top) / 2;
    TutWinBaseWindow.client_to_screen(&x, &y);
    parse_num(0, Rules.nutrient_req_citizen);
    if (tut_win(MapWinTable[0], "TOUR1NUT", x, y, 0, 2, -1, -1) == 0) return;
    x = TutWinBaseWindow.expenses_help_rects_[2].left + (TutWinBaseWindow.expenses_help_rects_[2].right - TutWinBaseWindow.expenses_help_rects_[2].left) / 2;
    y = TutWinBaseWindow.expenses_help_rects_[2].top + (TutWinBaseWindow.expenses_help_rects_[2].bottom - TutWinBaseWindow.expenses_help_rects_[2].top) / 2;
    TutWinBaseWindow.client_to_screen(&x, &y);
    if (tut_win(MapWinTable[0], "TOUR1MIN", x, y, 0, 2, -1, -1) == 0) return;

    x = TutWinBaseWindow.expenses_help_rects_[0].left + (TutWinBaseWindow.expenses_help_rects_[0].right - TutWinBaseWindow.expenses_help_rects_[0].left) / 2;
    y = TutWinBaseWindow.expenses_help_rects_[0].top + (TutWinBaseWindow.expenses_help_rects_[0].bottom - TutWinBaseWindow.expenses_help_rects_[0].top) / 2;
    TutWinBaseWindow.client_to_screen(&x, &y);
    if (tut_win(MapWinTable[0], "TOUR1ENERGY", x, y, 0, 2, -1, -1) == 0) return;
    TutWinBaseWindow.draw_energy_alloc(2);

    if (!TutWinBaseWindow.flatButtons_[17].is_visible()) {
        x = TutWinBaseWindow.expenses_help_rects_[5].left + (TutWinBaseWindow.expenses_help_rects_[5].right - TutWinBaseWindow.expenses_help_rects_[5].left) / 2;
        y = TutWinBaseWindow.expenses_help_rects_[5].top + (TutWinBaseWindow.expenses_help_rects_[5].bottom - TutWinBaseWindow.expenses_help_rects_[5].top) / 2;
        TutWinBaseWindow.client_to_screen(&x, &y);
        if (tut_win(MapWinTable[0], "TOUR1ALLOC", x, y, 0, 2, -1, -1) == 0) return;
    } else {
        {
            int x;
            int y;
            base_rect(&TutWinBaseWindow.rect13_, &x, &y);
            if (tut_win(MapWinTable[0], "TOUR1ALLOC2", x, y, 0, 2, -1, -1) == 0) return;
        }
    }
    x = TutWinBaseWindow.expenses_help_rects_[5].left + (TutWinBaseWindow.expenses_help_rects_[5].right - TutWinBaseWindow.expenses_help_rects_[5].left) / 2;
    y = TutWinBaseWindow.expenses_help_rects_[5].top + (TutWinBaseWindow.expenses_help_rects_[5].bottom - TutWinBaseWindow.expenses_help_rects_[5].top) / 2;
    TutWinBaseWindow.client_to_screen(&x, &y);
    if (tut_win(MapWinTable[0], "TOUR1ECON", x, y, 0, 2, -1, -1) == 0) return;
    x = TutWinBaseWindow.expenses_help_rects_[3].left + (TutWinBaseWindow.expenses_help_rects_[3].right - TutWinBaseWindow.expenses_help_rects_[3].left) / 2;
    y = TutWinBaseWindow.expenses_help_rects_[3].top + (TutWinBaseWindow.expenses_help_rects_[3].bottom - TutWinBaseWindow.expenses_help_rects_[3].top) / 2;
    TutWinBaseWindow.client_to_screen(&x, &y);
    if (tut_win(MapWinTable[0], "TOUR1PSYCH", x, y, 0, 2, -1, -1) == 0) return;
    x = TutWinBaseWindow.expenses_help_rects_[4].left + (TutWinBaseWindow.expenses_help_rects_[4].right - TutWinBaseWindow.expenses_help_rects_[4].left) / 2;
    y = TutWinBaseWindow.expenses_help_rects_[4].top + (TutWinBaseWindow.expenses_help_rects_[4].bottom - TutWinBaseWindow.expenses_help_rects_[4].top) / 2;
    TutWinBaseWindow.client_to_screen(&x, &y);
    if (tut_win(MapWinTable[0], "TOUR1LABS", x, y, 0, 2, -1, -1) == 0) return;
    TutWinBaseWindow.draw_expenses(1);
    TutWinBaseWindow.draw_farm(3);
    {
        int x;
        int y;
        base_rect(&TutWinBaseWindow.rect9_, &x, &y);
        if (tut_win(MapWinTable[0], "TOUR1SUPPORTBUTTON", x, y, 0, 2, -1, -1) == 0) return;
    }
    TutWinBaseWindow.draw_farm(2);
    {
        int x;
        int y;
        base_rect(&TutWinBaseWindow.rect11_, &x, &y);
        if (tut_win(MapWinTable[0], "TOUR1PSYCHBUTTON", x, y, 0, 2, -1, -1) == 0) return;
    }
    TutWinBaseWindow.draw_farm(4);
    {
        int x;
        int y;
        base_rect(&TutWinBaseWindow.rect10_, &x, &y);
        if (tut_win(MapWinTable[0], "TOUR1RESOURCEBUTTON", x, y, 0, 2, -1, -1) == 0) return;
    }

    if (do_iface(&TutWinBaseWindow.previous_button_rect_, "TOUR1PREV", 2) == 0) return;
    if (do_iface(&TutWinBaseWindow.next_button_rect_, "TOUR1NEXT", 2) == 0) return;
    if (do_base(&TutWinBaseWindow.rect7_, "TOUR1RENAME", 2) == 0) return;
    // tutor.txt names this final OK-button card independently of the emitted
    // literal. Its pinned NUL-terminated storage is 68790C..687913; the final
    // original do_base gate must succeed before the farm view is restored.
    // RECOVERY_LITERAL: v1 0x0068790C 0x00687914 $SG55954 0x004BB7A0
    if (do_base(&TutWinBaseWindow.rect6_, "TOUR1OK", 2) == 0) return;
    TutWinBaseWindow.draw_farm(1);
}
// RECOVERY_GENERATED: v1 vtable 0x0066DD84 ? ??_7TutWin@@6BWin@@@ 0x004BA6B0
// RECOVERY_GENERATED: v1 vtable 0x0066DD7C ? ??_7TutWin@@6BBuffer@@@ 0x004BA6B0


// The following eight mapped TutWin members were previously grouped in
// tutwin_callouts.cpp as an investigative source split. That split did not
// establish an original object boundary; these bodies and their evidence now
// share TutWin's recovered translation unit.


/*
 * Four 67-byte clones differing only in which fixed window they finish on.
 * Each centres the rectangle on both axes and hands the pair to that window's
 * client_to_screen. The centring is inlined four times rather than shared,
 * matching the original, which carries four copies.
 *
 * The halving is the compiler's signed divide-by-two - `sub eax,esi; cdq;
 * sub eax,edx; sar eax,1` - so it truncates toward zero rather than flooring,
 * and the subtraction wraps. Modelled here the way win.cpp's own midpoint()
 * models the identical idiom: unsigned arithmetic throughout, with the sign
 * bit carried back over the shift.
 */


// WORKAROUND(noinline): tour1/tour5/tour7 (this band) call these four
// centring helpers out of line at every site; /Ob2 expands them here.
#pragma auto_inline(off)
/*
Purpose: Centre the rectangle on both axes and convert the result to screen
         coordinates through the iface window.
// ORIGINAL: 0x004BC5A0 ?iface_rect@TutWin@@QAEXPAURECT@@PAH1@Z 0x004BC5A0-0x004BC5E3 BYTE_EXACT
// LEVER: plain `left + (right - left) / 2` signed division - the image
//        already emits the `cdq; sub; sar` fixup itself, so the hand-rolled
//        unsigned-with-sign-carried-over-the-shift trick this body used to
//        model the SAME idiom compiled to different instructions. Matches
//        the sibling `base_rect`/`soc_rect`/`des_rect`, already byte-exact
//        on this same lever.
// symbol    ?iface_rect@TutWin@@QAEXPAUtagRECT@@PAH1@Z
// size      67 bytes
// prototype void (__thiscall ?iface_rect@TutWin@@QAEXPAURECT@@PAH1@Z)(TutWin* this, RECT*, int*, int*)
// callers   3   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005ED240
Return Value: n/a
Status: Complete
Verification note: client_to_screen is entered on the FIXED window at 0x007AE820,
         never on `this` - the original loads it as an immediate into ECX. The
         TutWin receiver is not read by this body at all.
*/
void TutWin::iface_rect(RECT *rect, int *x, int *y) {
    *x = static_cast<int32_t>(static_cast<uint32_t>(rect->left) +
        static_cast<uint32_t>(static_cast<int32_t>(
            static_cast<uint32_t>(rect->right) - static_cast<uint32_t>(rect->left)) / 2));
    *y = static_cast<int32_t>(static_cast<uint32_t>(rect->top) +
        static_cast<uint32_t>(static_cast<int32_t>(
            static_cast<uint32_t>(rect->bottom) - static_cast<uint32_t>(rect->top)) / 2));
    MainInterfaceGlobal.client_to_screen(x, y);
}



/*
Purpose: Centre the rectangle on both axes and convert the result to screen
         coordinates through the base window.
// ORIGINAL: 0x004BC5F0 ?base_rect@TutWin@@QAEXPAURECT@@PAH1@Z 0x004BC5F0-0x004BC633 BYTE_EXACT
// symbol    ?base_rect@TutWin@@QAEXPAUtagRECT@@PAH1@Z
// size      67 bytes
// prototype void (__thiscall ?base_rect@TutWin@@QAEXPAURECT@@PAH1@Z)(TutWin* this, RECT*, int*, int*)
// callers   2   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005ED240
Return Value: n/a
Status: Complete
Verification note: client_to_screen is entered on the FIXED window at 0x006A7628,
         never on `this` - the original loads it as an immediate into ECX. The
         TutWin receiver is not read by this body at all.
*/
void TutWin::base_rect(RECT *rect, int *x, int *y) {
    // Plain signed division: the image emits `cdq; sub eax,edx; sar eax,1`
    // (round-toward-zero) for each half, not a rounding bit-trick.
    *x = static_cast<int32_t>(static_cast<uint32_t>(rect->left) +
        static_cast<uint32_t>(static_cast<int32_t>(
            static_cast<uint32_t>(rect->right) - static_cast<uint32_t>(rect->left)) / 2));
    *y = static_cast<int32_t>(static_cast<uint32_t>(rect->top) +
        static_cast<uint32_t>(static_cast<int32_t>(
            static_cast<uint32_t>(rect->bottom) - static_cast<uint32_t>(rect->top)) / 2));
    TutWinBaseWindow.client_to_screen(x, y);
}



/*
Purpose: Centre the rectangle on both axes and convert the result to screen
         coordinates through the soc window.
// ORIGINAL: 0x004BC640 ?soc_rect@TutWin@@QAEXPAURECT@@PAH1@Z 0x004BC640-0x004BC683 BYTE_EXACT
// symbol    ?soc_rect@TutWin@@QAEXPAUtagRECT@@PAH1@Z
// size      67 bytes
// prototype void (__thiscall ?soc_rect@TutWin@@QAEXPAURECT@@PAH1@Z)(TutWin* this, RECT*, int*, int*)
// callers   2   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005ED240
Return Value: n/a
Status: Complete
Verification note: client_to_screen is entered on the FIXED window at 0x008A6270,
         never on `this` - the original loads it as an immediate into ECX. The
         TutWin receiver is not read by this body at all.
*/
void TutWin::soc_rect(RECT *rect, int *x, int *y) {
    // Plain signed division: the image emits `cdq; sub eax,edx; sar eax,1`
    // (round-toward-zero) for each half, not a rounding bit-trick.
    *x = static_cast<int32_t>(static_cast<uint32_t>(rect->left) +
        static_cast<uint32_t>(static_cast<int32_t>(
            static_cast<uint32_t>(rect->right) - static_cast<uint32_t>(rect->left)) / 2));
    *y = static_cast<int32_t>(static_cast<uint32_t>(rect->top) +
        static_cast<uint32_t>(static_cast<int32_t>(
            static_cast<uint32_t>(rect->bottom) - static_cast<uint32_t>(rect->top)) / 2));
    TutWinSocWindow.client_to_screen(x, y);
}



/*
Purpose: Centre the rectangle on both axes and convert the result to screen
         coordinates through the des window.
// ORIGINAL: 0x004BC690 ?des_rect@TutWin@@QAEXPAURECT@@PAH1@Z 0x004BC690-0x004BC6D3 BYTE_EXACT
// symbol    ?des_rect@TutWin@@QAEXPAUtagRECT@@PAH1@Z
// size      67 bytes
// prototype void (__thiscall ?des_rect@TutWin@@QAEXPAURECT@@PAH1@Z)(TutWin* this, RECT*, int*, int*)
// callers   2   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005ED240
Return Value: n/a
Status: Complete
Verification note: client_to_screen is entered on the FIXED window at 0x0071F2B0,
         never on `this` - the original loads it as an immediate into ECX. The
         TutWin receiver is not read by this body at all.
*/
void TutWin::des_rect(RECT *rect, int *x, int *y) {
    // Plain signed division: the image emits `cdq; sub eax,edx; sar eax,1`
    // (round-toward-zero) for each half, not a rounding bit-trick.
    *x = static_cast<int32_t>(static_cast<uint32_t>(rect->left) +
        static_cast<uint32_t>(static_cast<int32_t>(
            static_cast<uint32_t>(rect->right) - static_cast<uint32_t>(rect->left)) / 2));
    *y = static_cast<int32_t>(static_cast<uint32_t>(rect->top) +
        static_cast<uint32_t>(static_cast<int32_t>(
            static_cast<uint32_t>(rect->bottom) - static_cast<uint32_t>(rect->top)) / 2));
    TutWinDesWindow.client_to_screen(x, y);
}



/*
 * The four do_* helpers: 123-byte clones whose ONLY real difference is which
 * fixed window they centre through. The other differing bytes are the two
 * call displacements, which move because the calls are relative and the four
 * bodies sit 0x80 apart - not because either target differs. Both targets are
 * the same for all four.
 */

#pragma auto_inline(on)
// WORKAROUND(inline): tour expands iface_rect at all ten callouts
// (0x004BD395.. pairs a centring with a direct client_to_screen call) while
// tour1/5/6/7 and do_iface call 0x004BC5A0 out of line. VC6 has no noinline,
// so the out-of-line definition stays under auto_inline(off) and tour uses
// this expanded twin of the same body.
static inline void tour_iface_rect(RECT *rect, int *x, int *y) {
    *x = rect->left + (rect->right - rect->left) / 2;
    *y = rect->top + (rect->bottom - rect->top) / 2;
    MainInterfaceGlobal.client_to_screen(x, y);
}

// ORIGINAL: 0x004BC6E0 ?tour@TutWin@@QAEXXZ 0x004BC6E0-0x004BDCBE FILE
// RECOVERY_BINDING: v1 {"evidence": ["seed:src/unrecovered/004bc6e0.cpp", "image:0x004bc6e0; original call/rectangle/lifetime review"], "reason": "Restore the original ownership-group implementation through real tutorial/editor windows, declared members and normal C++ lifecycle; retained seed extents include the associated exception cleanup where present."}
// TRIED: full structural transcription of both scoring loops and the 10x tut_win/client_to_screen chain (from raw disasm, cross-checked against Ghidra); diverges at instr #33 (push vs mov) - register allocation for the phase-1 direction-search loop, not a logic gap.
// Both scans call MapWin::tile_in_view(x, y, 2) on MapWinTable[0] (the image
// keeps the inline parameter x in its own slot, [ebp-0x40], at 0x004BC843)
// and wrap x through wrap_map_x; the three small tables are cleared by a
// loop that VC6 unrolls (0x004BCBF8: one register per array). Frame slots
// are not declaration-ordered: VC6 numbers them by descending reference
// count after packing disjoint live ranges, so no variable is shared by hand.
// The image's block order is kept. The
// image expands reset() at its three sites (0x004BD8A6, 0x004BDBB2,
// 0x004BDC43). Both arms of reset end in UNK1 (0x004BA720), which /Ob2
// expands too; with the ten stores written out in each arm reset was too
// costly to expand here.
void TutWin::tour() {
    int x;
    int y;
    Map *tile;
    int special;
    int index;
    int site_x;
    int site_y;
    int fungus;
    int fungus_x;
    int fungus_y;
    int ring;
    int wettest;
    int lower;
    int one_rock_x;
    int higher;
    int flat_y;
    int yield;
    int coldest;
    int peak_x;
    int peak_y;
    int two_rock_score;
    int alt_y;
    int highest;
    int two_rock_x;
    int hottest;
    int two_rock_y;
    int yield1_score;
    int one_rock_y;
    int site_score;
    int flat_x;
    int ocean_y;
    int flat_score;
    int dry_x;
    int yield2_score;
    int ocean_score;
    int one_rock_score;
    int alt_score;
    int ocean_x;
    int alt_x;
    int wet_x;
    int driest;
    int wet_y;
    int temp_score[3];
    int rain_score[3];
    int elevated;
    int bits;
    int rain_kind[3];
    int rain_x[3];
    int rain_y[3];
    int temp_x[3];
    int temp_y[3];
    int temp_kind[3];
    int best_score;
    int best_dir;
    int start;
    int cursor_x;
    int cursor_y;
    int step;

    if (field_53B4_ != 0) {
        return;
    }
    field_53B4_ = 1;
    if (field_53C0_ == 1) {
        tour1();
        goto tour_cleanup;
    }
    if (field_53C0_ == 5) {
        tour5();
        goto tour_cleanup;
    }
    if (field_53C0_ == 6) {
        tour6();
        goto tour_cleanup;
    }
    if (field_53C0_ == 7) {
        tour7();
        goto tour_cleanup;
    }

    WorldClimateWorldWin.hide();
    MapWinTable[0]->set_center(
        ConsoleGlobal.cursor_history_x_[ConsoleGlobal.cursor_history_index_],
        ConsoleGlobal.cursor_history_y_[ConsoleGlobal.cursor_history_index_], 1);

    // THE OPENING DIRECTION: the visible land neighbour the cursor faces,
    // probed from a random start; cardinal steps and open land score best.
    best_score = 0;
    best_dir = 0;
    start = random(0, 8);
    cursor_x = ConsoleGlobal.cursor_history_x_[ConsoleGlobal.cursor_history_index_];
    for (step = 0; step < 8; step++) {
        lower = (step + start) % 8;
        x = wrap_map_x(cursor_x + VehMoveDeltaX[lower]);
        y = ConsoleGlobal.cursor_history_y_[ConsoleGlobal.cursor_history_index_] + VehMoveDeltaY[lower];
        if (on_map(x, y) && MapWinTable[0]->tile_in_view(x, y, 2) && !is_ocean(x, y)) {
            int score = 8;
            if ((bit_at(x, y) & BIT_FUNGUS) && (map_loc(x, y)->climate & ~0x1F) >= 0x40) {
                score = 4;
            }
            if (!VehMoveDeltaX[lower] || !VehMoveDeltaY[lower]) {
                score++;
            }
            if (score >= best_score) {
                best_score = score;
                best_dir = lower;
            }
        }
    }

    if (field_53C0_ == 0) {
        StringTemp[0] = 0;
        strcat(StringTemp, "TOUR1");
        if (!ConsoleGlobal.cursor_active_ || ConsoleGlobal.selected_vehicle_ < 0) {
            strcat(StringTemp, "B");
        } else {
            short proto_id = Vehs[ConsoleGlobal.selected_vehicle_].proto_id;
            if (proto_id != 2) {
                parse_says(0, VehPrototypes[proto_id].veh_name, -1, -1);
                strcat(StringTemp, "A");
            } else {
                parse_says(0, VehPrototypes[2].veh_name, -1, -1);
            }
        }
        if (!tut_map(StringTemp,
                     ConsoleGlobal.cursor_history_x_[ConsoleGlobal.cursor_history_index_],
                     ConsoleGlobal.cursor_history_y_[ConsoleGlobal.cursor_history_index_],
                     ConsoleGlobal.selected_vehicle_, 0, 2, -1, -1)) {
            goto tour_cleanup;
        }
        MapWinTable[0]->field_1DD70_ |= 0x20000;
        MapWinTable[0]->draw_map(1);
        parse_num(0, MapDirectionPermutation[best_dir]);
        if (!text_open(TutorialPopupCaption, "DIRECTIONS")) {
            for (int line = 0; line <= best_dir; line++) {
                text_get();
            }
            text_close();
            parse_says(0, TextBufferGetPtr, -1, -1);
        }
        if (!tut_map("TOUR2",
                     ConsoleGlobal.cursor_history_x_[ConsoleGlobal.cursor_history_index_],
                     ConsoleGlobal.cursor_history_y_[ConsoleGlobal.cursor_history_index_],
                     ConsoleGlobal.selected_vehicle_, 0, 2, -1, -1)) {
            MapWinTable[0]->field_1DD70_ &= ~0x20000;
            MapWinTable[0]->draw_map(1);
            goto tour_cleanup;
        }
    } else if (field_53C0_ == 3 && ConsoleGlobal.selected_vehicle_ >= 0
               && Vehs[ConsoleGlobal.selected_vehicle_].proto_id == 1) {
        if (!tut_map("TERRAFORMER", cursor_x, ConsoleGlobal.cursor_history_y_[ConsoleGlobal.cursor_history_index_], ConsoleGlobal.selected_vehicle_,
                     0, 2, -1, -1)) {
            goto tour_cleanup;
        }
    }
    MapWinTable[0]->field_1DD70_ &= ~0x20000;
    MapWinTable[0]->draw_map(1);

    // THE SURVEY: two passes over the 20 tiles around the cursor, keeping
    // the best example of each feature the tour can point at.
    highest = 3;
    driest = 3;
    coldest = 3;
    ocean_y = -1;
    ocean_x = -1;
    fungus_y = -1;
    fungus_x = -1;
    flat_y = -1;
    flat_x = -1;
    one_rock_y = -1;
    one_rock_x = -1;
    two_rock_y = -1;
    two_rock_x = -1;
    alt_y = -1;
    alt_x = -1;
    peak_y = -1;
    peak_x = -1;
    wet_y = -1;
    wet_x = -1;
    dry_x = -1;
    site_y = -1;
    site_x = -1;
    wettest = -1;
    hottest = -1;
    one_rock_score = -1;
    alt_score = -1;
    ocean_score = -1;
    site_score = -1;
    flat_score = -1;
    two_rock_score = -1;
    yield1_score = -1;
    yield2_score = -1;
    for (index = 0; index < 3; index++) {
        rain_score[index] = -1;
        temp_score[index] = -1;
        rain_kind[index] = -1;
    }
    for (ring = 0; ring < 2; ring++) {
        for (index = 1; index < 21; index++) {
            x = xrange_inline(ConsoleGlobal.cursor_history_x_[ConsoleGlobal.cursor_history_index_]
                              + RadiusOffsetX[index]);
            y = ConsoleGlobal.cursor_history_y_[ConsoleGlobal.cursor_history_index_]
                + RadiusOffsetY[index];
            if (y < 0 || y >= MapLatitudeBounds || x < 0 || x >= MapLongitudeBounds) {
                continue;
            }
            if (!MapWinTable[0]->tile_in_view(x, y, 2)) {
                continue;
            }
            if (!(PlayersData[ConsoleGlobal.faction_id_].flags & RFLAG_FREEPROTO)
                && !(map_loc(x, y)->visibility & (1 << ConsoleGlobal.faction_id_))
                && !(GameDataStateStorage.fields_.game_state_ & 0x80)) {
                continue;
            }
            if (is_ocean(x, y)) {
                // the ocean tile closest to land
                int score;
                if ((bit_at(x, y) & BIT_FUNGUS) && (map_loc(x, y)->climate & ~0x1F) >= 0x40) {
                    score = 1;
                } else {
                    score = 2;
                }
                if (score > ocean_score) {
                    ocean_score = score;
                    ocean_x = x;
                    ocean_y = y;
                }
                continue;
            }
            if ((bit_at(x, y) & BIT_FUNGUS) && (map_loc(x, y)->climate & ~0x1F) >= 0x40) {
                // land fungus, preferring a tile that is not rocky
                if (fungus_y >= 0 && fungus_y < MapLatitudeBounds && fungus_x >= 0
                    && fungus_x < MapLongitudeBounds && (map_loc(x, y)->val3 & ~0x3F) >= 0x80) {
                    continue;
                }
                fungus_x = x;
                fungus_y = y;
                continue;
            }
            special = (goody_at(x, y) || bonus_at(x, y, 0) || (bit_at(x, y) & 0x2000)) ? 1 : 0;
            yield = crop_yield(ConsoleGlobal.faction_id_, -1, x, y, 0);
            tile = map_loc(x, y);
            bits = tile->bit;
            elevated = tile->bit & 0x80;
            if ((bits & BIT_FUNGUS) && (tile->climate & ~0x1F) >= 0x40) {
                fungus = 1;
            } else {
                fungus = 0;
            }
            higher = 0;
            lower = 0;
            for (int dir = 0; dir < 8; dir++) {
                int x_adj = wrap_map_x(x + VehMoveDeltaX[dir]);
                int y_adj = VehMoveDeltaY[dir] + y;
                if (y_adj < 0 || y_adj >= MapLatitudeBounds || x_adj < 0
                    || x_adj >= MapLongitudeBounds) {
                    continue;
                }
                int altitude_adj = map_loc(x_adj, y_adj)->climate & ~0x1F;
                if (altitude_adj < 0x60) {
                    continue;
                }
                if (altitude_adj > (tile->climate & ~0x1F)) {
                    if (!dir || dir >= 6) {
                        higher = 1;
                    }
                    if (index >= 2 && index <= 4) {
                        lower = 1;
                    }
                }
                if (altitude_adj < (tile->climate & ~0x1F)) {
                    if (!dir || dir >= 6) {
                        lower = 1;
                    }
                    if (index >= 2 && index <= 4) {
                        higher = 1;
                    }
                }
            }
            int score = higher && !lower;
            score -= (fungus + special) * 3;
            score -= elevated;
            score += (8 - lower) * 2;
            if (!special) {
                int rocks = tile->val3 & ~0x3F;
                if (!rocks) {
                    if (score > flat_score) {
                        flat_score = score;
                        flat_x = x;
                        flat_y = y;
                    }
                } else if (rocks == 0x40) {
                    score = yield + score * 8;
                    if (score > one_rock_score && (!ring || one_rock_x < 0)) {
                        one_rock_score = score;
                        one_rock_x = x;
                        one_rock_y = y;
                    }
                } else if (rocks == 0x80) {
                    if (score > two_rock_score) {
                        two_rock_score = score;
                        two_rock_x = x;
                        two_rock_y = y;
                    }
                }
            }
            int level = tile->climate >> 5;
            if (level == 3) {
                score = yield
                        + ((((4 - lower) * 2 - fungus - special) * 2 - higher) - elevated) * 8;
                if ((tile->val3 & ~0x3F) > 0x40) {
                    score = 0;
                }
                if (score > alt_score) {
                    alt_score = score;
                    alt_x = x;
                        alt_y = y;
                }
            }
            if (level > highest) {
                highest = level;
                peak_x = x;
                peak_y = y;
            }
            int temperature = tile->climate & 7;
            int rainfall = (tile->climate >> 3) & 3;
            int ridge = higher && !lower;
            score = ridge - (fungus + special) * 3 - (tile->val3 >> 6) - elevated
                    + (8 - lower) * 2;
            if (score > temp_score[temperature]) {
                temp_x[temperature] = x;
                temp_y[temperature] = y;
                temp_kind[temperature] = temperature;
                temp_score[temperature] = temperature;
            }
            if (score > rain_score[rainfall]) {
                rain_x[rainfall] = x;
                rain_y[rainfall] = y;
                rain_kind[rainfall] = rainfall;
                rain_score[rainfall] = score;
            }
            if (rainfall > wettest
                || (rainfall == wettest && score >= rain_score[rainfall])) {
                wettest = rainfall;
                wet_x = x;
                wet_y = y;
            }
            if (rainfall < driest
                || (rainfall == driest && score >= rain_score[rainfall])) {
                driest = rainfall;
                dry_x = x;
            }
            if (temperature > hottest
                || (temperature == hottest && score >= temp_score[temperature])) {
                hottest = temperature;
            }
            if (temperature < coldest
                || (temperature == coldest && score >= temp_score[temperature])) {
                coldest = temperature;
            }
            if (!special && !(bits & 0xA000)) {
                if (yield == 1) {
                    if (score > yield1_score) {
                        yield1_score = score;
                    }
                } else if (yield == 2) {
                    if (score > yield2_score) {
                        yield2_score = score;
                    }
                }
                int site = world_site(x, y, 0);
                if (site > site_score) {
                    site_score = site;
                    site_y = y;
                    site_x = x;
                }
            }
        }
    }

    if (field_53C0_ == 0) {
        if (ocean_x >= 0 && !tut_map("TOUROCEAN", ocean_x, ocean_y, -1, 0, 2, -1, -1)) {
            goto tour_cleanup;
        }
        if (fungus_x >= 0 && !tut_map("TOURFUNGUS", fungus_x, fungus_y, -1, 0, 2, -1, -1)) {
            goto tour_cleanup;
        }
        goto tour_menu;
    }

    if (!tut_map("TOURMAP",
                 ConsoleGlobal.cursor_history_x_[ConsoleGlobal.cursor_history_index_],
                 ConsoleGlobal.cursor_history_y_[ConsoleGlobal.cursor_history_index_],
                 -1, 0, 2, -1, -1)) {
        goto tour_cleanup;
    }
    if (wet_x >= 0 && dry_x >= 0) {
        if (!tut_map("TOURRAIN", wet_x, wet_y, -1, 0, 2, -1, -1)) {
            goto tour_cleanup;
        }
        for (index = 2; index >= 0; index--) {
            if (rain_kind[index] >= 0) {
                StringTemp[0] = 0;
                strcat(StringTemp, "TOURRAIN");
                switch (index) {
                case 0:
                    strcat(StringTemp, wettest ? "DRY" : "DRY2");
                    break;
                case 1:
                    strcat(StringTemp, "MOD");
                    break;
                case 2:
                    strcat(StringTemp, "WET");
                    break;
                }
                if (!tut_map(StringTemp, rain_x[index], rain_y[index], -1, 0, 2, -1, -1)) {
                    goto tour_cleanup;
                }
            }
        }
    }
    if (alt_x >= 0 && peak_x >= 0
        && !tut_map("TOURALT", alt_x, alt_y, -1, 0, 2, peak_x, peak_y)) {
        goto tour_cleanup;
    }
    if (two_rock_x >= 0
        && !tut_map("TOURTWOROCK", two_rock_x, two_rock_y, -1, 0, 2, -1, -1)) {
        goto tour_cleanup;
    }
    if (one_rock_x >= 0
        && !tut_map("TOURONEROCK", one_rock_x, one_rock_y, -1, 0, 2, -1, -1)) {
        goto tour_cleanup;
    }
    if (flat_x >= 0 && !tut_map("TOURFLAT", flat_x, flat_y, -1, 0, 2, -1, -1)) {
        goto tour_cleanup;
    }
    if (site_x <= 0) {
        site_x = ConsoleGlobal.cursor_history_x_[ConsoleGlobal.cursor_history_index_];
        site_y = ConsoleGlobal.cursor_history_y_[ConsoleGlobal.cursor_history_index_];
    }
    if (!tut_map("TOURFOREST", site_x, site_y, -1, 0, 2, -1, -1)) {
        goto tour_cleanup;
    }
    if (!tut_map("TOURREVIEW", site_x, site_y, -1, 0, 2, -1, -1)) {
        goto tour_cleanup;
    }
    if (!tut_map("TOURADVANCED", site_x, site_y, -1, 0, 2, -1, -1)) {
        goto tour_cleanup;
    }
    reset();
    field_53B4_ = 0;
    tut_map("TOURFORMDONE", site_x, site_y, -1, 0, 2, -1, -1);
tour_cleanup:
    desktop_update();
    field_53B4_ = 0;
    reset();
    return;

    // Written after the cleanup: the image's literals for this block follow
    // TOURFORMDONE although the block is laid out first.
tour_menu:
    if (TutWinClimateGate945830) {
        WorldClimateWorldWin.show_all();
    }
    do_all_draws();
    if (!ConsoleGlobal.alpha_menu_.is_visible()) {
        ConsoleGlobal.alpha_menu_.show();
    }
    tour_iface_rect(&MainInterfaceGlobal.menu_button_rect_, &x, &y);
    if (!tut_win(MapWinTable[0], "TOURMENU", x, y, 0, 2, -1, -1)) {
        goto tour_cleanup;
    }
    ConsoleGlobal.alpha_menu_.hide();
    if (!MultiWindow.panel_open()) {
        MultiWindow.show();
    }
    tour_iface_rect(&MainInterfaceGlobal.multiplayer_button_rect_, &x, &y);
    if (!tut_win(MapWinTable[0], "TOURCOMMLINK", x, y, 0, 2, -1, -1)) {
        goto tour_cleanup;
    }
    MultiWindow.hide();
    tour_iface_rect(&MainInterfaceGlobal.status_left_rect_, &x, &y);
    if (!tut_win(MapWinTable[0], "TOURUNITSINFO", x, y, 0, 2, -1, -1)) {
        goto tour_cleanup;
    }
    tour_iface_rect(&MainInterfaceGlobal.status_right_rect_, &x, &y);
    if (!tut_win(MapWinTable[0], "TOURTERRAININFO", x, y, 0, 2, -1, -1)) {
        goto tour_cleanup;
    }
    tour_iface_rect(&MainInterfaceGlobal.status_hit_rect_, &x, &y);
    if (!tut_win(MapWinTable[0], "TOURSTACKINFO", x, y, 0, 2, -1, -1)) {
        goto tour_cleanup;
    }
    tour_iface_rect(&MainInterfaceGlobal.message_bounds_, &x, &y);
    if (!tut_win(MapWinTable[0], "TOURMESSAGEINFO", x, y, 0, 2, -1, -1)) {
        goto tour_cleanup;
    }
    tour_iface_rect(&MainInterfaceGlobal.scroll_up_rect_, &x, &y);
    if (!tut_win(MapWinTable[0], "TOURBUTTONMESSAGE", x, y, 0, 2, -1, -1)) {
        goto tour_cleanup;
    }
    tour_iface_rect(&MainInterfaceGlobal.scroll_down_rect_, &x, &y);
    if (!tut_win(MapWinTable[0], "TOURBUTTONINFO", x, y, 0, 2, -1, -1)) {
        goto tour_cleanup;
    }
    tour_iface_rect(&MainInterfaceGlobal.tutorial_bounds_, &x, &y);
    if (!tut_win(MapWinTable[0], "TOURWORLDINFO", x, y, 0, 2, -1, -1)) {
        goto tour_cleanup;
    }
    tour_iface_rect(&MainInterfaceGlobal.date_rect_, &x, &y);
    if (!tut_win(MapWinTable[0], "TOURDATEINFO", x, y, 0, 2, -1, -1)) {
        goto tour_cleanup;
    }
    if (!ConsoleGlobal.cursor_active_
        || GameDataStateStorage.fields_.turn_current_num_ > 1) {
        goto tour_cleanup;
    }
    reset();
    field_53B4_ = 0;
    if (site_x <= 0) {
        site_x = ConsoleGlobal.cursor_history_x_[ConsoleGlobal.cursor_history_index_];
        site_y = ConsoleGlobal.cursor_history_y_[ConsoleGlobal.cursor_history_index_];
    }
    tut_map("TOUROVER", site_x, site_y, ConsoleGlobal.selected_vehicle_, 0, 0, -1, -1);
    return;
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// Historical seed observations (not fresh measurements or acceptance):
// TRIED: `extern int g_009a64bc_rmw;` for the trailing `or [addr],esi` (RMW on a fixed global) turned the const-pointer's load/or/store into a single `or`, reaching MNEMONIC_ONLY (61/61 mnemonics match); the one remaining divergence is register choice in the tail, where the original reuses esi for child2 and ours picks another.
// ORIGINAL: 0x004BDCC0 ?on_button_clicked@TutWin@@QAEXH@Z 0x004BDCC0-0x004BDDA2 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["seed:src/unrecovered/004bdcc0.cpp", "image:0x004bdcc0; original call/rectangle/lifetime review"], "reason": "Restore the original ownership-group implementation through real tutorial/editor windows, declared members and normal C++ lifecycle; retained seed extents include the associated exception cleanup where present."}
// symbol    ?on_button_clicked@TutWin@@UAEXH@Z
void TutWin::on_button_clicked(int button_id) {
    const uint32_t tour_was_shown = TutWinShownFlag;

    if (field_53B8_ != 0) {
        release_modal();
    }
    reset();

    field_53BC_ = button_id < 0 ? button_id + 1 : button_id;
    if (button_id < -1) {
        field_53B4_ = 0;
    }
    if (button_id <= 0) {
        return;
    }

    if (field_53B4_ == 0) {
        if (tour_was_shown != 0) {
            TutWinTourPending = 1;
        }
        if (field_53B4_ == 0) {
            goto finish_button;
        }
    }
    field_53B4_ = 0;

finish_button:
    if (field_53D0_ != 0) {
        GameDataStateStorage.fields_.tutwin_flags_9a64bc_ |= field_53D0_;
    }
}



/*
Purpose: Tell the attached object the tutorial window moved.
// ORIGINAL: 0x004BDDB0 ?on_move@TutWin@@QAEXHH@Z 0x004BDDB0-0x004BDDC5 BYTE_EXACT
// symbol    ?on_move@TutWin@@UAEXHH@Z
// size      21 bytes
// prototype void (__thiscall ?on_move@TutWin@@QAEXHH@Z)(TutWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004BDDBC
Return Value: n/a
Status: Complete
*/
void TutWin::on_move(int x, int y) {
    // field_53D4_ holds a Win-family object pointer (now typed as one); the
    // repaint is its declared on_update virtual.
    if (field_53D4_) {
        field_53D4_->on_update();
    }
}

/*
Purpose: Store a single value at 0x53D4.
// ORIGINAL: 0x004BDDD0 ?UNK3@TutWin@@QAEXH@Z 0x004BDDD0-0x004BDDE0 BYTE_EXACT
// symbol    ?UNK3@TutWin@@QAEXPAVWin@@@Z
// size      16 bytes
// prototype void (__thiscall ?UNK3@TutWin@@QAEXH@Z)(TutWin* this, int)
// callers   0   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void TutWin::UNK3(Win *a1) {
    field_53D4_ = a1;
}

// The active path clears the arrow state before virtual hide. Hide may
// change the owner, so the owner's update dispatch must reload field_53D4_.
// Drawing completes before either the owner or the shared marker is reset.
// Both arms end in UNK1, the ten-store clear: tut_win's expansion of reset
// inlines it in the owner arm and calls 0x004BA720 in the other, which is
// that copy's single caller.
// ORIGINAL: 0x004BDDE0 ?reset@TutWin@@QAEHXZ 0x004BDDE0-0x004BDE97 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original4BDDEE..4BDE07 clears53A4, calls this slot2, reloads owner53D4 for slot63 and then calls do_all_draws5FCBB0. The initial owner test selects the return value; callbacks precede all final reset stores.", "Both original tails clear owner53D4 and53A4, store minus one to5380/537C/539C, zero53A8, minus one53AC, zero53B8/53C4 and finally real TutWinShownFlag8CC244. Existing TutorialWindow and typed Win owner are preserved; no fixed-address aliases or VCall shim are required."], "reason": "Recover tutorial dismissal and owner notification through real members and virtual Win methods, retaining its original reset ordering and result."}
int TutWin::reset() {
    if (field_53D4_ != 0) {
        field_53A4_ = 0;
        hide();
        field_53D4_->on_update();
        do_all_draws();
        UNK1();
        return 1;
    }
    UNK1();
    return 0;
}

/*
Purpose: Step the receiver back to the subobject ??_GTutWin@@UAEPAXI@Z expects,
         then forward unchanged.
// ORIGINAL: 0x004BEA20 ??_GTutWin@@WEEE@AEPAXI@Z 0x004BEA20-0x004BEA2B BYTE_EXACT
// symbol    ??_ETutWin@@WEEE@AEPAXI@Z
// CORRECTED from ??3TutWin@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x004BE9F0` into
//   ??_GTutWin@@UAEPAXI@Z, which executes `ret 4`; no stack access and
//   the receiver stays in ECX. `WEEE@` re-demangles to adjustor{1092}
//   and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/

// Tutorial arrows use game.cpp's PopupThemeWindowColor (original 0x0068A5A0).
// config_popups updates that shared palette cell; no separate arrow copy exists.

/*
Purpose: Draw the tutorial's pointing arrow: its shaft across the rectangle the
         window records, then its head from the recorded corner, both in the one
         colour index and only when the window says it is on screen at all.
// ORIGINAL: 0x004BDEA0 ?draw_arrow@TutWin@@QAEXXZ 0x004BDEA0-0x004BDF16 FILE BYTE_EXACT
// LEVER: the two calls are Win::window_line_raw on the Win-family object at
//   0x53D4, and the second only fires when field_539C_ is non-negative - the
//   arrow's head, drawn from a different corner of the same rectangle.
// size      118 bytes
// callers   3   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005F2AC0
Return Value: n/a
Status: Complete
*/
void TutWin::draw_arrow() {
    if (field_53A4_ != 0) {
        field_53D4_->window_line_raw(field_538C_, field_5390_, field_5384_,
                                     field_5388_, PopupThemeWindowColor, 2, 2);
        int head = field_539C_;
        if (head >= 0) {
            field_53D4_->window_line_raw(head, field_53A0_, field_5394_,
                                         field_5398_, PopupThemeWindowColor, 2, 2);
        }
    }
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004BDF20 ?terrain_draw@TutWin@@QAAXPAUSprite@@PAUGraphicWin@@HHH@Z 0x004BDF20-0x004BDF66 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x004BDF20-0x004BDF66; seed:src/recovered/004bdf20.cpp; source:src/mapwin.h:gen_terrain_poly; source:src/tutwin.h:TutorialWindow"], "reason": "The draw callback takes no receiver and reads the singleton tutorial tile state while invoking the typed terrain renderer."}
// symbol    ?tut_terrain_draw@@YAXPAVSprite@@PAVGraphicWin@@HHH@Z
void __cdecl tut_terrain_draw(Sprite *, GraphicWin *window, int,
                              int pixel_x, int pixel_y) {
    Buffer *canvas = window ? static_cast<Buffer *>(window) : 0;
    gen_terrain_poly(canvas, 0, 0, ConsoleGlobal.faction_id_, -1, 0x621,
                     TutorialWindow.field_537C_, TutorialWindow.field_5380_,
                     pixel_x, pixel_y);
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004BDF70 ?veh_draw@TutWin@@QAAXPAUSprite@@PAUGraphicWin@@HHH@Z 0x004BDF70-0x004BDFA0 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x004BDF70-0x004BDFA0; seed:src/recovered/004bdf70.cpp; source:src/veh.h:veh_draw; source:src/graphicwin.h:Buffer base"], "reason": "The tutorial vehicle draw callback has no receiver and forwards a typed GraphicWin canvas to the recovered renderer."}
// symbol    ?tut_veh_draw@@YAXPAVSprite@@PAVGraphicWin@@HHH@Z
void __cdecl tut_veh_draw(Sprite *, GraphicWin *window, int vehicle,
                          int pixel_x, int pixel_y) {
    Buffer *canvas = window ? static_cast<Buffer *>(window) : 0;
    veh_draw(canvas, vehicle, pixel_x, pixel_y, 0, 0, 0);
}

// The original inline teardown is exactly Popup::close followed by the
// compiler-owned destructors for Popup's Scroll, BasePop members, and
// GraphicWin bases. TutWin owns no non-trivial member after Popup, so letting
// the declared C++ hierarchy run its normal reverse-lifetime chain preserves
// that ordering without replaying subobject offsets or vptr stores.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004BE7D0 ??1TutWin@@QAE@XZ 0x004BE7D0-0x004BE9EE;0x00659BF0-0x00659CD7 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["seed:src/recovered/units/004be7d0.cpp", "image:0x004be7d0; original call/rectangle/lifetime review"], "reason": "Restore the original ownership-group implementation through real tutorial/editor windows, declared members and normal C++ lifecycle; retained seed extents include the associated exception cleanup where present."}
// symbol    ??1TutWin@@UAE@XZ
TutWin::~TutWin() {}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004BDFA0 ?base_draw@TutWin@@QAAXPAUSprite@@PAUGraphicWin@@HHH@Z 0x004BDFA0-0x004BDFDD FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["seed:src/recovered/units/004bdfa0.cpp", "image:0x004bdfa0; original call/rectangle/lifetime review"], "reason": "Restore the original ownership-group implementation through real tutorial/editor windows, declared members and normal C++ lifecycle; retained seed extents include the associated exception cleanup where present."}
// symbol    ?base_draw@TutWin@@SAXPAVSprite@@PAVGraphicWin@@HHH@Z
void __cdecl TutWin::base_draw(Sprite *, GraphicWin *window, int mode,
                              int pixel_x, int pixel_y) {
    int base_id = mode + 2;
    if (base_id < 0) {
        base_id = -2 - mode;
    }
    Buffer *const destination = window != 0
        ? static_cast<Buffer *>(window) : 0;
    ::base_draw(destination, base_id, pixel_x, pixel_y, 0, 0);
}

// ORIGINAL: 0x004BDFE0 ?tut_win@TutWin@@QAEHPAUGraphicWin@@PBDHHPAUSprite@@HHH@Z 0x004BDFE0-0x004BE643;0x00659CE0-0x00659CF2 FILE
// RECOVERY_BINDING: v1 {"evidence": ["seed:src/unrecovered/004bdfe0.cpp", "image:0x004bdfe0; original call/rectangle/lifetime review"], "reason": "Restore the original ownership-group implementation through real tutorial/editor windows, declared members and normal C++ lifecycle; retained seed extents include the associated exception cleanup where present."}
// TRIED: real `Sprite localSprite;` (no explicit ctor/dtor calls, relying on the scaffold's Sprite having no declared destructor so none is emitted, matching the disassembly which never calls ~Sprite on the normal path) plus field-offset writes transcribed instruction-by-instruction (including the cdq/and-7 rounding idioms rewritten as `(val<0)?7:0` bias terms) compiles and reaches instruction #2 before diverging (no SEH try/finally frame is generated without a non-trivial local destructor, and the original's frame protects the Sprite constructor call).
// symbol    ?tut_win@TutWin@@QAEHPAVGraphicWin@@PBDHHPAVSprite@@HHH@Z
int TutWin::tut_win(GraphicWin *owner, const char *text, int x, int y,
                    Sprite *picture, int flags, int arrow_x, int arrow_y) {
    Sprite local_sprite;
    int mode;
    int popup_flags;
    int screen_width;
    int screen_height;

    field_53C4_ = 0;
    field_53D0_ = 0;
    if (flags >> 16) {
        field_53D0_ = flags & 0xFFFF0000;
        flags &= 0xFFFF;
    }
    mode = flags & 0xFF;
    switch (mode) {
    case 0:
        popup_flags = 0x4000;
        break;
    case 1:
        popup_flags = 0;
        break;
    default:
        popup_flags = 0x40;
        break;
    }
    field_5384_ = x;
    field_5388_ = y;
    screen_width = WinScreenWidth;
    screen_height = WinScreenHeight;

    popups_tutorial();
    reset();
    field_53D4_ = owner;
    if (flags & 0xFF00) {
        TutWinShownFlag = 1;
        flags = mode;
    }

    start(TutorialPopupCaption, text, -1, 0, popup_flags, 0);
    if (flags < 2) {
        set_ok_text(StringTable.get(Labels.strings_ptr[219]));
    } else {
        set_ok_text(StringTable.get(Labels.strings_ptr[236]));
        set_cancel_text(label_get(235));
    }

    if (picture != 0) {
        set_picture(picture, 0, 0);
    } else if (field_53AC_ >= 0) {
        set_picture(&g_VEH_SPRITES[0], 0, 0);
        set_redraw_callback(tut_veh_draw);
    } else if (field_53AC_ < -1) {
        set_picture(&g_VEH_SPRITES[0], 0, 0);
        set_redraw_callback(TutWin::base_draw);
    } else if (field_537C_ >= 0) {
        set_picture(&g_TER1_TUT_BLANK_SPRITE, 0, 0);
        set_redraw_callback(tut_terrain_draw);
    }

    exec(1, 0);

    int vertical_anchor = screen_height * 5 / 8;
    int horizontal_anchor = screen_width / 2;
    int height_eighth = screen_height / 8;
    int width_eighth = screen_width / 8;
    if (flags < 2) {
        horizontal_anchor += width_eighth;
    }
    if (flags == 2 || flags == 5) {
        horizontal_anchor += width_eighth;
    }
    if (flags == 3 || flags == 4) {
        horizontal_anchor -= width_eighth;
    }
    if (flags == 4 || flags == 5) {
        vertical_anchor -= height_eighth;
    }

    if (y <= vertical_anchor) {
        y = screen_height - layout_height() - 8;
        field_5390_ = y;
    } else {
        y = field_5388_ / 8;
        field_5390_ = y + layout_height();
    }

    if (x <= horizontal_anchor) {
        x = 5;
        field_538C_ = static_cast<int>(layout_width()) / 2 + 5;
    } else {
        field_538C_ = screen_width - static_cast<int>(layout_width()) / 2 - 5;
        x = screen_width - layout_width() - 5;
    }

    if (arrow_x >= 0) {
        field_5394_ = arrow_x;
        field_5398_ = arrow_y;
        field_539C_ = field_538C_;
        field_53A0_ = field_5390_;
    }
    field_53B8_ = flags;
    field_53A4_ = 1;
    field_53D4_->window_line_raw(field_538C_, field_5390_, field_5384_,
                                  field_5388_, PopupThemeWindowColor, 2, 2);
    if (field_539C_ >= 0) {
        field_53D4_->window_line_raw(field_539C_, field_53A0_, field_5394_,
                                      field_5398_, PopupThemeWindowColor, 2, 2);
    }

    start(TutorialPopupCaption, text, -1, 0, popup_flags, 0);
    if (flags == 0) {
        set_ok_text(label_get(219));
    } else {
        set_ok_text(label_get(236));
        set_cancel_text(label_get(235));
    }
    if (flags != 6) {
        set_loc(x, y);
    }

    if (picture != 0) {
        set_picture(picture, 0, 0);
    } else if (field_53AC_ >= 0) {
        set_picture(&local_sprite, static_cast<int>(field_53AC_), 0);
        set_redraw_callback(tut_veh_draw);
    } else if (field_53AC_ < -1) {
        set_picture(&local_sprite, static_cast<int>(field_53AC_), 0);
        set_redraw_callback(TutWin::base_draw);
    } else if (field_537C_ >= 0) {
        set_picture(&local_sprite, 0, 0);
        set_redraw_callback(tut_terrain_draw);
    }

    exec(0, 0);
    popups_normal();
    field_53A8_ = 1;
    if (flags == 1) {
        // tour() may change the key fields; the image reads them first.
        int run_tour = field_53BC_;
        int send_key = field_53C4_;
        int key = field_53C8_;
        int key_state = field_53CC_;
        if (run_tour) {
            tour();
        }
        if (send_key) {
            ConsoleGlobal.on_key_click(key, key_state);
        }
    }

    return static_cast<int>(field_53B4_);
}

// ORIGINAL: 0x004BE650 ?tut_map@TutWin@@QAEHPBDHHHPAUSprite@@HHH@Z 0x004BE650-0x004BE7C6 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["seed:src/unrecovered/004be650.cpp", "image:0x004be650; original call/rectangle/lifetime review"], "reason": "Restore the original ownership-group implementation through real tutorial/editor windows, declared members and normal C++ lifecycle; retained seed extents include the associated exception cleanup where present."}
// TRIED: separate `int x = -1; int y = -1;` locals instead of the original's shared `or eax,0xffffffff` init (#3 mismatch, 'or' vs 'mov'); combined declaration compiled the same way.
// symbol    ?tut_map@TutWin@@QAEHPBDHHHPAVSprite@@HHH@Z
int TutWin::tut_map(const char *text, int x_coord, int y_coord, int pixel_x,
                    Sprite *sprite, int forwarded_6, int arrow_x,
                    int arrow_y) {
    int arrow_pixel_x = -1;
    int arrow_pixel_y = -1;

    if (pixel_x >= 0) {
        field_53AC_ = pixel_x;
    } else if (sprite == 0) {
        field_537C_ = x_coord;
        field_5380_ = y_coord;
    }

    MapWinTable[0]->focus(x_coord, y_coord);
    int screen_x;
    int screen_y;
    MapWinTable[0]->tile_to_pixel(x_coord, y_coord, &screen_x, &screen_y);
    MapWin *map_window = MapWinTable[0];
    screen_x += map_window->field_1DDBC_;
    screen_y += map_window->field_1DDC0_;

    if (arrow_x >= 0) {
        if (map_window->tile_in_view(arrow_x, arrow_y, 0)) {
            map_window->tile_to_pixel(arrow_x, arrow_y, &arrow_pixel_x,
                                          &arrow_pixel_y);
            map_window = MapWinTable[0];
            arrow_pixel_x += map_window->field_1DDBC_;
            arrow_pixel_y += map_window->field_1DDC0_;
        }
    }

    GraphicWin *const owner = static_cast<GraphicWin *>(map_window);
    return tut_win(owner, text, screen_x, screen_y, sprite,
                   forwarded_6, arrow_pixel_x, arrow_pixel_y);
}

// The old callout helpers all transform a panel rectangle into screen
// coordinates, then host the popup in MapWin's virtual GraphicWin base.
// Keeping the transformation receiver explicit preserves which window owns
// each rectangle while using the declared virtual-base conversion.
static int tut_callout(TutWin &tutorial, Win &coordinates, RECT &rect,
                       const char *label) {
    int x = rect.left + (rect.right - rect.left) / 2;
    int y = rect.top + (rect.bottom - rect.top) / 2;
    coordinates.client_to_screen(&x, &y);
    return tutorial.tut_win(MapWinTable[0], label, x, y, 0, 2, -1, -1);
}
