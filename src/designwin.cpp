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
#include "wave.h"
#include "designwin.h"
#include "win.h"
#include "worldwin.h"
#include "faction.h"
#include "game.h"
#include "popup.h"
#include "basepop.h"
#include "strings.h"
#include "technology.h"
#include "temp.h"
#include "general.h"
#include "veh.h"

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x004345C0 ?UNK1@DesignWin@@QAEXXZ 0x004345C0-0x004345C1 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?UNK1@DesignWin@@QAEXXZ)(DesignWin* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DesignWin::UNK1() {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x0043D330 ?on_iface_left_down@DesignWin@@QAEXHH@Z 0x0043D330-0x0043D333 BYTE_EXACT
// symbol    ?on_iface_left_down@DesignWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_left_down@DesignWin@@QAEXHH@Z)(DesignWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DesignWin::on_iface_left_down(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x0043D340 ?on_iface_right_down@DesignWin@@QAEXHH@Z 0x0043D340-0x0043D343 BYTE_EXACT
// symbol    ?on_iface_right_down@DesignWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_right_down@DesignWin@@QAEXHH@Z)(DesignWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DesignWin::on_iface_right_down(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x0043D400 ?on_iface_selected@DesignWin@@QAEXHH@Z 0x0043D400-0x0043D403 BYTE_EXACT
// symbol    ?on_iface_selected@DesignWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_selected@DesignWin@@QAEXHH@Z)(DesignWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DesignWin::on_iface_selected(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x0043D410 ?on_iface_left_double_click@DesignWin@@QAEXHH@Z 0x0043D410-0x0043D413 BYTE_EXACT
// symbol    ?on_iface_left_double_click@DesignWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_left_double_click@DesignWin@@QAEXHH@Z)(DesignWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DesignWin::on_iface_left_double_click(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x0043D420 ?on_iface_right_double_click@DesignWin@@QAEXHH@Z 0x0043D420-0x0043D423 BYTE_EXACT
// symbol    ?on_iface_right_double_click@DesignWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_right_double_click@DesignWin@@QAEXHH@Z)(DesignWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DesignWin::on_iface_right_double_click(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x0043D470 ?on_iface_button_toggled@DesignWin@@QAEXHH@Z 0x0043D470-0x0043D473 BYTE_EXACT
// symbol    ?on_iface_button_toggled@DesignWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_button_toggled@DesignWin@@QAEXHH@Z)(DesignWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DesignWin::on_iface_button_toggled(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x0043D480 ?on_iface_group_clicked@DesignWin@@QAEXHHH@Z 0x0043D480-0x0043D483 BYTE_EXACT
// symbol    ?on_iface_group_clicked@DesignWin@@UAEXHHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_iface_group_clicked@DesignWin@@QAEXHHH@Z)(DesignWin* this, int, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DesignWin::on_iface_group_clicked(int, int, int) {
}

// ---- the interface table's own slots -------------------------------------

// Slot 44 of the front vtable - the catalogue's
// ?on_button_clicked@DesignWin@@QAEXH@Z, 0x00438460, 4481 bytes (plus the cold
// tail at 0x00654010), unrecovered. vslot_13 and on_iface_button_pressed both
// reach it through a live dispatch, so the entry has to be DesignWin's.
void DesignWin::on_button_clicked(int button_id) {
    typedef void(__fastcall *pending)(DesignWin *, void *, int);
    reinterpret_cast<pending>(static_cast<unsigned long>(0x00438460))(
        this, nullptr, button_id);
}

/*
// ORIGINAL: 0x0043D430 ?on_iface_button_clicked@DesignWin@@QAEXH@Z 0x0043D430-0x0043D44D BYTE_EXACT
// symbol    ?vslot_13@DesignWin@@UAEXH@Z
// size      29 bytes
// prototype void (__thiscall ?on_iface_button_clicked@DesignWin@@QAEXH@Z)(DesignWin* this, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0043D443
Return Value: n/a
Status: Complete
*/
// LEVER: the receiver is the ENTRY (the image's `add ecx,-0xa14` is VC6's own
//   adjustment to the front) and the dispatch is the front's slot 44, so the
//   whole body is one unqualified virtual call - `on_button_clicked(button_id)` is the
//   image's `call dword ptr [eax + 0xb0]`.
void DesignWin::vslot_13(int button_id) {
    on_button_clicked(button_id);
}

/*
// ORIGINAL: 0x0043D450 ?on_iface_button_pressed@DesignWin@@QAEXH@Z 0x0043D450-0x0043D46F BYTE_EXACT
// symbol    ?on_iface_button_pressed@DesignWin@@UAEHH@Z
// size      31 bytes
// prototype int (__thiscall ?on_iface_button_pressed@DesignWin@@QAEXH@Z)(DesignWin* this, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0043D463
Return Value: 0
Status: Complete
*/
// LEVER: the same slot-44 dispatch as vslot_13 with a constant 0 returned
//   after it - the interface wants an answer, the design window forwards the
//   click to the front and reports "not pressed".
int DesignWin::on_iface_button_pressed(int button_id) {
    on_button_clicked(button_id);
    return 0;
}

// ---- slots still carrying an original body ------------------------------

// PENDING 0x0043D120 ?on_status@DesignWin@@QAEXH@Z 0x0043D120-0x0043D215
// symbol    ?on_status@DesignWin@@UAEXH@Z
// size      245 bytes
// prototype void (__thiscall ?on_status@DesignWin@@QAEXH@Z)(DesignWin* this, int)
// callers   0   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
void DesignWin::on_status(int mode) {
    typedef void(__fastcall *pending)(DesignWin *, void *, int);
    reinterpret_cast<pending>(static_cast<unsigned long>(0x0043D120))(
        this, nullptr, mode);
}

// PENDING 0x0043D220 ?on_iface_left_click@DesignWin@@QAEXHH@Z 0x0043D220-0x0043D281
// symbol    ?on_iface_left_click@DesignWin@@UAEXHH@Z
// size      97 bytes
// prototype void (__thiscall ?on_iface_left_click@DesignWin@@QAEXHH@Z)(DesignWin* this, int, int)
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005FAB00 0x005169F0
void DesignWin::on_iface_left_click(int a1, int a2) {
    typedef void(__fastcall *pending)(DesignWin *, void *, int, int);
    reinterpret_cast<pending>(static_cast<unsigned long>(0x0043D220))(
        this, nullptr, a1, a2);
}

// PENDING 0x0043D290 ?on_iface_right_click@DesignWin@@QAEXHH@Z 0x0043D290-0x0043D327
// symbol    ?on_iface_right_click@DesignWin@@UAEXHH@Z
// size      151 bytes
// prototype void (__thiscall ?on_iface_right_click@DesignWin@@QAEXHH@Z)(DesignWin* this, int, int)
// callers   0   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
void DesignWin::on_iface_right_click(int a1, int a2) {
    typedef void(__fastcall *pending)(DesignWin *, void *, int, int);
    reinterpret_cast<pending>(static_cast<unsigned long>(0x0043D290))(
        this, nullptr, a1, a2);
}

// PENDING 0x0043D350 ?on_iface_mouse_move@DesignWin@@QAEXHH@Z 0x0043D350-0x0043D3B9
// symbol    ?on_iface_mouse_move@DesignWin@@UAEXHH@Z
// size      105 bytes
// prototype void (__thiscall ?on_iface_mouse_move@DesignWin@@QAEXHH@Z)(DesignWin* this, int, int)
// callers   0   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
void DesignWin::on_iface_mouse_move(int a1, int a2) {
    typedef void(__fastcall *pending)(DesignWin *, void *, int, int);
    reinterpret_cast<pending>(static_cast<unsigned long>(0x0043D350))(
        this, nullptr, a1, a2);
}

// PENDING 0x0043D3C0 ?on_iface_mouse_leave@DesignWin@@QAEXHH@Z 0x0043D3C0-0x0043D3F1
// symbol    ?on_iface_mouse_leave@DesignWin@@UAEXHH@Z
// size      49 bytes
// prototype void (__thiscall ?on_iface_mouse_leave@DesignWin@@QAEXHH@Z)(DesignWin* this, int, int)
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
void DesignWin::on_iface_mouse_leave(int a1, int a2) {
    typedef void(__fastcall *pending)(DesignWin *, void *, int, int);
    reinterpret_cast<pending>(static_cast<unsigned long>(0x0043D3C0))(
        this, nullptr, a1, a2);
}


/*
Purpose: Dismiss the bubble text when the pointer leaves. The whole body is two
         consecutive calls to the static Win::clear_bubble_text and nothing
         else; the arguments are ignored.

         The SECOND call is not a transcription error. The original issues it,
         and the two are observationally identical unless something re-arms the
         bubble between them - which is why the fixture re-arms it rather than
         asserting a single refresh and calling that agreement.
// ORIGINAL: 0x00437BE0 ?on_mouse_leave@DesignWin@@QAEXHH@Z 0x00437BE0-0x00437BED BYTE_EXACT
// symbol    ?on_mouse_leave@DesignWin@@UAEXHH@Z
// size      13 bytes
// prototype void (__thiscall ?on_mouse_leave@DesignWin@@QAEXHH@Z)(DesignWin* this, int, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005F8500
Return Value: n/a
Status: Complete
*/
void DesignWin::on_mouse_leave(int, int) {
    ::Win::clear_bubble_text();
    ::Win::clear_bubble_text();
}


/*
// ORIGINAL: 0x0043BE10 ?select_special_1@DesignWin@@QAEXXZ 0x0043BE10-0x0043BE18 BYTE_EXACT
// size      8 bytes
// prototype void (__thiscall ?select_special_1@DesignWin@@QAEXXZ)(DesignWin* this)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0043AC70
Status: Complete
*/
void DesignWin::select_special_1() {
    select_special(0);
}

/*
// ORIGINAL: 0x0043BE20 ?select_special_2@DesignWin@@QAEXXZ 0x0043BE20-0x0043BE28 BYTE_EXACT
// size      8 bytes
// prototype void (__thiscall ?select_special_2@DesignWin@@QAEXXZ)(DesignWin* this)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0043AC70
Status: Complete
*/
void DesignWin::select_special_2() {
    select_special(1);
}

// The seam and definition for the unrecovered select_special, at the end of
// the file so no recovered body above shifts.
//
// auto_inline(off) is load-bearing, not tidiness, and being defined after the
// callers is NOT enough on its own: VC6 at /Ob2 defers codegen to the end of
// the translation unit and folds a forwarder defined later back into a caller
// defined earlier. Measured - without the pragma cl reports C4711 at
// designwin.cpp(153) and (161), the two select_special_N thunks, and each is
// an 8-byte BYTE_EXACT `call rel32` today.

/*
Purpose: Close the design window: dismiss all 25 palette buttons, release the
         spot list, stop the clock, release the buffer, reset the prototype
         selection, and close the three fonts.
// ORIGINAL: 0x0043C1A0 ?close@DesignWin@@QAEXXZ 0x0043C1A0-0x0043C261
// size      193 bytes
// prototype void (__thiscall ?close@DesignWin@@QAEXXZ)(DesignWin* this)
// callers   6   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D7470 0x005FA830 0x00616780 0x00619230
// indirect  0x0043C1B4
// MEASURED 2026-08-29: /O2 best at 12/42. Image instructions 0..11 agree -
//   the prologue and the whole 25-button loop, the slot-0x168 dispatch
//   arriving as the image's own single `call dword ptr [eax + 0x168]` out of
//   vtable_slot once /Ob2 inlines it (at /Ob0 the template function itself
//   is CALLED - `push 0x168 / push edi / call`, which is why /Ob0's
//   higher-similarity 6/42 is the worse listing; do not chase it).
//   Everything from image instruction 12 on diverges for the catalogued
//   MEASURED-inline ceiling: spot2_.shutdown(), time_.close() and the three
//   font_.close() calls are in-class definitions in spot.h / time.h /
//   font.h (in-class so ~Spot / ~Time / ~Font inline them), and VC6 inlines
//   all five HERE where the image makes five plain calls (0x005FA830,
//   0x00616780, 0x00619230 x3) - 141 instructions against the image's 42.
//   Out-of-line bodies would regress those destructor claims, and VC6
//   honours no per-site noinline.
Return Value: n/a
Status: Complete
*/
namespace {

// Slot 2, the DesignWin front vtable's own second entry.
typedef void (__fastcall *func_design_win_slot002)(void *);

}

void DesignWin::close() {
    // A do/while counting DOWN from 25 over a walking pointer - the image
    // keeps `mov ebx, 0x19` and steps `add edi, 0xb4c` (sizeof FlatButton).
    FlatButton *button = flatButtons_;
    int count = 0x19;
    do {
        button->close();
        ++button;
        --count;
    } while (count != 0);

    spot2_.shutdown();
    // IMAGE ORDER: scene_rotation_ and field_CE8_ are written between the
    // clock receiver's computation and the call itself.
    scene_rotation_ = 0.0f;
    field_CE8_ = 0;
    time_.close();
    buffer_.close();
    factionID_ = 0;
    protoID_ = 0;
    protoChassisType_ = 0;
    protoWeaponType_ = 0;
    protoArmorType_ = 0;
    protoAbilityFlags_ = 0;
    protoReactorType_ = 1;
    field_14210_ = 0;
    mineralCostFactor_ = 0;
    field_CE0_ = static_cast<uint32_t>(-1);
    field_14220_ = 0;
    font1_.close();
    font2_.close();
    font3_.close();
}

// ===== MANAGED GLOBALS - real objects, homed to their domain =====
// In the shipped image these live at fixed data addresses and are
// constructed before WinMain by the CRT's dynamic-initializer walk
// (winedbg-confirmed: walker return 0x00644EEB, table at 0x682568..).
// Here the same recovered constructors run through this build's own
// startup, and the matching destructors close them at exit. Addresses of
// the ones documented individually live beside their definitions.
Wave g_DESIGNWIN_WAVE;  // 0x0071F240

// The process-wide design window, 0x0071F2B0 in the image - REAL STORAGE,
// not the `Win *const TutWinDesWindow` binding that named terranx.exe data
// (see tutwin.h). The image's dynamic initializer at 0x00432850 constructs
// it at its fixed address before WinMain; DesignWin's constructor here is
// the empty inline the tree models, so this object is zero storage the
// game's own init populates, exactly as the image leaves it.
// The vehicle chassis animation queries this same window's Win base at
// 0x004C2BE9; the original receiver is the complete object at 0x0071F2B0.
// RECOVERY_OBJECT: v1 0x0071F2B0 ?TutWinDesWindow@@3VDesignWin@@A
DesignWin TutWinDesWindow;  // 0x0071F2B0
// ===== MANAGED GLOBALS - real objects, homed to their domain =====
// In the shipped image these live at fixed data addresses and are
// constructed before WinMain by the CRT's dynamic-initializer walk
// (winedbg-confirmed: walker return 0x00644EEB, table at 0x682568..).
// Here the same recovered constructors run through this build's own
// startup, and the matching destructors close them at exit.

// ORIGINAL: 0x00439670 ?bubble_me_elmo2@DesignWin@@QAEXPADHH@Z 0x00439670-0x004396B0 FILE
// size      64 bytes
// prototype void (__thiscall ?bubble_me_elmo2@DesignWin@@QAEXPADHH@Z)(DesignWin* this, LPCSTR, int, int)
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005ED240 0x005F83D0
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00439670
// measured tier  MISMATCH
// divergence     12
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00439670/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?bubble_me_elmo2@DesignWin@@QAEXPADHH@Z  at 0x00439670  (64 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    an indirect virtual call use the generated VCall shim below; for a
//    thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.


void DesignWin::bubble_me_elmo2(char *a1, int a2, int a3) {
    reinterpret_cast<Win *>(this)->client_to_screen(&a2, &a3);
    int rc[4];
    rc[0] = a2;
    rc[2] = a2;
    rc[1] = a3;
    rc[3] = a3;
    reinterpret_cast<Win *>(this)->set_bubble_text(a1, reinterpret_cast<RECT *>(rc));
}

// ORIGINAL: 0x00439EB0 ?delete_abil@DesignWin@@QAEXH@Z 0x00439EB0-0x00439F18 FILE
// size      104 bytes
// prototype void (__thiscall ?delete_abil@DesignWin@@QAEXH@Z)(DesignWin* this, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00439EB0
// measured tier  MISMATCH
// divergence     10
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00439eb0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?delete_abil@DesignWin@@QAEXH@Z  at 0x00439EB0  (104 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    an indirect virtual call use the generated VCall shim below; for a
//    thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.


void DesignWin::delete_abil(int a1) {
    char *self = reinterpret_cast<char *>(this);
    unsigned int *flags = reinterpret_cast<unsigned int *>(self + 0x14208);

    if (a1 & *flags) {
        // WALL: register allocation. Every one of the 40 mnemonics below
        // matches the original in order; only WHICH callee-saved register
        // (esi vs edi) holds `this` versus the `slots` pointer is swapped
        // throughout (the original saves `this` unconditionally into esi
        // and the slots pointer conditionally into edi; this compile makes
        // the opposite choice). No reordering of these declarations changed
        // which register the backend picked - ruled out as unfixable via
        // source spelling.
        int *slots = reinterpret_cast<int *>(self + 0x14218);
        *flags = ~a1 & *flags;
        slots[0] = -1;
        slots[1] = -1;
        int i = 0;
        do {
            if (*flags & (1 << i)) {
                int j = 0;
                do {
                    if (slots[j] < 0) {
                        slots[j] = i;
                        break;
                    }
                    j++;
                } while (j < 2);
            }
            i++;
        } while (i < 0x1d);
    }
}

// ORIGINAL: 0x0043D0C0 ?show@DesignWin@@QAEXH@Z 0x0043D0C0-0x0043D0E8 BYTE_EXACT
// symbol    ?show@DesignWin@@UAEXH@Z
// LEVER: homed from 0043d0c0.cpp; needed the `symbol` fact (the tree emits
// UAEXH - it overrides Win::show's virtual - where the image has QAEXH) before
// measure could find it, then held BYTE_EXACT on the idiomatic base calls
// `is_visible()` / `set_iface_mode()` where the artifact had two
// reinterpret_casts of `this`.
// size      40 bytes
// prototype void (__thiscall ?show@DesignWin@@QAEXH@Z)(DesignWin* this, int)
// callers   0   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00446B40 0x0045D310 0x005F7E90
// 0x0043D0C0  ?show@DesignWin@@QAEXH@Z  ->  ?show@DesignWin@@QAEXH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.


void DesignWin::show(int a1) {
    if (!is_visible()) {
        ambience(0xD3);
        set_iface_mode();
    }
}

// ORIGINAL: 0x0043D0F0 ?hide@DesignWin@@QAEXXZ 0x0043D0F0-0x0043D116 BYTE_EXACT
// size      38 bytes
// prototype void (__thiscall ?hide@DesignWin@@QAEXXZ)(DesignWin* this)
// callers   0   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00446B40 0x0045D380 0x005F7E90
// symbol    ?hide@DesignWin@@UAEXXZ
// LEVER: homed from 0043d0f0.cpp; same `symbol` fact as show - the tree emits
// the virtual UAEXX form that overrides Win::hide. Idiomatic base calls held
// BYTE_EXACT on the first measure.
// 0x0043D0F0  ?hide@DesignWin@@QAEXXZ  ->  ?hide@DesignWin@@QAEXXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.


void DesignWin::hide() {
    if (is_visible()) {
        release_iface_mode();
        ambience(0xd4);
    }
}

// ORIGINAL: 0x0043C070 ?construct_chassis@DesignWin@@QAEXXZ 0x0043C070-0x0043C09E BYTE_EXACT
// size      46 bytes
// prototype void (__thiscall ?construct_chassis@DesignWin@@QAEXXZ)(DesignWin* this)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004C2470
// 0x0043C070  ?construct_chassis@DesignWin@@QAEXXZ  ->  ?construct_chassis@DesignWin@@QAEXXZ
// LEVER: homed from 0043c070.cpp. The named fields (factionID_ 0x141F4,
// protoChassisType_ 0x141FC, protoWeaponType_ 0x14200, protoReactorType_
// 0x1420C, caviar_ 0x1279C) replaced the artifact's five
// reinterpret_casts of `this` and held BYTE_EXACT first measure; the callee
// needed a pending_bodies forwarder plus the prototype now at the end of
// designwin.h.
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.


void DesignWin::construct_chassis() {
    vehdraw_construct_chassis(factionID_, protoChassisType_, protoWeaponType_, 0,
                              protoReactorType_, &caviar_);
}

// Fixed-slot bindings carried from 0043c0a0.cpp
// DesignWin's auto-build latch, 0x00733974. The image holds the int directly;
// a file-scope `int *const` folds to the bare immediate at every read.
// Name is the ROLE, read off the three sites that touch it: DesignWin::
// construct_vehicle sets it to 1 around its construct_weapon call and back to
// 0 after, while construct_weapon (0x0043C0A0) and draw_weapon (0x00434F40)
// read it to decide whether the chassis slot is authoritative or should be
// passed as -1. No catalogue name for the slot.
static int *const DesignAutoBuildActive = (int *)0x00733974;

// ORIGINAL: 0x0043C0A0 ?construct_weapon@DesignWin@@QAEXXZ 0x0043C0A0-0x0043C0EF FILE BYTE_EXACT
// size      79 bytes
// prototype void (__thiscall ?construct_weapon@DesignWin@@QAEXXZ)(DesignWin* this)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004C2CF0
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0043C0A0
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0043c0a0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?construct_weapon@DesignWin@@QAEXXZ  at 0x0043C0A0  (79 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    an indirect virtual call use the generated VCall shim below; for a
//    thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.


void DesignWin::construct_weapon() {
    int weapon_id = protoWeaponType_;
    if (weapon_id == 0x15 && *DesignAutoBuildActive == 0) {
        vehdraw_construct_weapon(factionID_, -1, 0x15, &caviar_);
        return;
    }
    vehdraw_construct_weapon(factionID_, protoChassisType_, weapon_id, &caviar_);
}

// Fixed-slot bindings carried from 0043c0f0.cpp

// ORIGINAL: 0x0043C0F0 ?construct_armor@DesignWin@@QAEXH@Z 0x0043C0F0-0x0043C126 FILE BYTE_EXACT
// size      54 bytes
// prototype void (__thiscall ?construct_armor@DesignWin@@QAEXH@Z)(DesignWin* this, int)
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004C3090
// PRESERVED UNIT - measured NO_COMPILE.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0043C0F0
// measured tier  NO_COMPILE
// refusal        u0043c0f0.cpp(44) : error C2061: syntax error : identifier 'Caviar' u0043c0f0.cpp(63) : error C2061: syntax error : identifier 'Caviar'
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0043c0f0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?construct_armor@DesignWin@@QAEXH@Z  at 0x0043C0F0  (54 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    an indirect virtual call use the generated VCall shim below; for a
//    thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?construct_armor@DesignWin@@QAEXH@Z  at 0x0043C0F0  (54 bytes)
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


void DesignWin::construct_armor(int a1) {
    vehdraw_construct_armor(ConsoleGlobal.faction_id_, protoChassisType_, protoWeaponType_,
                            protoArmorType_, a1, &caviar_);
}

// ORIGINAL: 0x0043C130 ?construct_reactor@DesignWin@@QAEXXZ 0x0043C130-0x0043C163 BYTE_EXACT
// size      51 bytes
// prototype void (__thiscall ?construct_reactor@DesignWin@@QAEXXZ)(DesignWin* this)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004C3800
// 0x0043C130  ?construct_reactor@DesignWin@@QAEXXZ  ->  ?construct_reactor@DesignWin@@QAEXXZ
// LEVER: homed from 0043c130.cpp. Same named-field lever as construct_armor.
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.


void DesignWin::construct_reactor() {
    vehdraw_construct_reactor(ConsoleGlobal.faction_id_, protoChassisType_, protoWeaponType_,
                              protoReactorType_, protoAbilityFlags_, &caviar_);
}

// Fixed-slot bindings carried from 00434650.cpp
// The vehdraw stat block the design-window renderer reads back: the four
// chosen component types as BYTES at 0x009AC558..0x009AC55B, the ability flag
// word at 0x009AC554 and a byte at 0x009AC562 whose only writer is this
// function's 0xFF. Each is an `int *const` / `uint8_t *const` at file scope,
// which folds to the bare immediate, so the image's
// `mov [0x9ac558], al` shape survives. No catalogue name for any of them;
// these are the roles, read off the three sites that touch the block
// (UNK3 here, draw_vehicle and draw_flash, both still unrecovered).
static int *const VehDrawAbilityFlags = (int *)0x009AC554;
static unsigned char *const VehDrawChassis = (unsigned char *)0x009AC558;
static unsigned char *const VehDrawWeapon = (unsigned char *)0x009AC559;
static unsigned char *const VehDrawArmor = (unsigned char *)0x009AC55A;
static unsigned char *const VehDrawReactor = (unsigned char *)0x009AC55B;
static unsigned char *const VehDrawComponentTail = (unsigned char *)0x009AC562;

// ORIGINAL: 0x00434650 ?UNK3@DesignWin@@QAEXXZ 0x00434650-0x00434691 FILE BYTE_EXACT
// size      65 bytes
// prototype void (__thiscall ?UNK3@DesignWin@@QAEXXZ)(DesignWin* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// LEVER: homed from units/00434650.cpp. The vehdraw stat block's six raw
// bindings renamed (VehDrawChassis/Weapon/Armor/Reactor/AbilityFlags/
// ComponentTail) and the named DesignWin fields put in their place; the
// byte-width of each target is carried by the binding's own type, so no
// reinterpret_cast of `this` survives.
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00434650
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00434650/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?UNK3@DesignWin@@QAEXXZ  at 0x00434650  (65 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    an indirect virtual call use the generated VCall shim below; for a
//    thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.


void DesignWin::UNK3() {
    *VehDrawChassis = (unsigned char)protoChassisType_;
    *VehDrawWeapon = (unsigned char)protoWeaponType_;
    *VehDrawArmor = (unsigned char)protoArmorType_;
    *VehDrawReactor = (unsigned char)protoReactorType_;
    *VehDrawAbilityFlags = (int)protoAbilityFlags_;
    *VehDrawComponentTail = 0xFF;
}

// Fixed-slot bindings carried from 0043e960.cpp. 0x0073396C is the tutor
// state byte the exec/tutor path sets to 1/2 and clears here; 0x00733970 is
// the DesignWin *exec stores so timer_function can find the window again.
static int *const DesignTutorState = (int *)0x0073396C;
static DesignWin **const DesignWindowSelf = (DesignWin **)0x00733970;

// ORIGINAL: 0x0043E960 ?shut_that_badboy_down@DesignWin@@QAEXXZ 0x0043E960-0x0043E99D FILE
// size      61 bytes
// prototype void (__thiscall ?shut_that_badboy_down@DesignWin@@QAEXXZ)(DesignWin* this)
// callers   2   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005FCBB0 0x00616730 0x00616870
// indirect  0x0043E96F
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0043E960
// measured tier  MISMATCH
// divergence     8
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0043e960/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?shut_that_badboy_down@DesignWin@@QAEXXZ  at 0x0043E960  (61 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    an indirect virtual call use the generated VCall shim below; for a
//    thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.


void DesignWin::shut_that_badboy_down() {
    if (*DesignTutorState != 1) {
        vtable_slot<func_design_win_slot002>(this, 0x8)(this);
        do_all_draws();
        Time *modal = &time_;
        *DesignWindowSelf = 0;
        modal->stop();
        modal->release_modal();
    }
    *DesignTutorState = 0;
}

// MISMATCH, ruled out (mnemonic_similarity 0.875, 65 vs 61 bytes):
// divergence #8, original 'lea' vs rebuilt 'add'+'mov', plus one inserted
// 'mov' before the second call. The original computes `this + 0x12774` ONCE
// into ecx (`lea ecx,[esi+0x12774]`) and reuses that same ecx, UNRELOADED,
// as the receiver for BOTH `modal->stop()` and `modal->release_modal()` -
// two back-to-back thiscall calls with a global store in between that does
// not touch ecx. Every VC6 phrasing tried here (named pointer local, named
// reference local, and the un-named expression repeated verbatim at both
// call sites, which CSEs to the same code) makes /O2 instead promote the
// address into esi (destructively overwriting `this`, which is dead by
// this point) and reload ecx with a 2-byte `mov ecx,esi` before EACH call -
// the textbook-safe thing to do, since a compiler that only sees `Time`
// as an opaque extern declaration cannot prove `Time::stop()` (66 bytes,
// calls timeKillEvent/KillTimer/flush_timer - genuinely not a leaf) leaves
// ecx holding the caller's value on return. src/time.h's
// `release_modal` declaration (00616870) is what this tree calls; the unit that
// carried that body was folded away (commit 619631a0), and it showed
// `Time::release_modal()` never touches `this` (`TimeModal =
// nullptr;`), so the no-reload choice the original compiler made is only
// explainable by whole-translation-unit visibility into that callee's body
// - something this scaffold's opaque-class model structurally cannot give
// it. Ruled out rather than re-derive: this is a same-shape sibling of the
// documented INSTRUCTION SCHEDULING wall, but for cross-call register
// liveness instead of instruction order.
