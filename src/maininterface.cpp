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
#include "maininterface.h"
#include "messagewin.h"
#include "worldwin.h"
#include "graphicwin.h"
#include "filewin.h"
// Homed MainInterface bodies call the free helpers the archive binds by
// address: say_year (game.h), base_at (base.h). veh.h is left out on
// purpose - its `range` collides with general.h's, and the two headers
// cannot both be read here.
#include "game.h"
#include "base.h"
#include "map.h"  // WorldClimateSkipTerrainClear
#include "statuswin.h"  // StatusWinRedrawRect1, the redraw rects
#include "console.h"   // ConsoleStatusWin, 0x008C5568

 /*
 Purpose: Set the flat button's bubble text.
 Original Offset: n/a
 Return Value: Zero on success, non-zero on error
 Status: Complete
 */
int MainInterface::set_bubble_text(uint32_t button_id, LPCSTR text) {
    return flat_button_[button_id].set_bubble_text(text);
}

/*
Purpose: Clear the interface message; the legacy implementation is intentionally empty.
// ORIGINAL: 0x0045C170 ?clear_message@MainInterface@@QAEXXZ 0x0045C170-0x0045C171 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?clear_message@MainInterface@@QAEXXZ)(MainInterface* this)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: n/a
Status: Complete
*/
void MainInterface::clear_message() {
}

/*
Purpose: Update the desktop; the legacy implementation is intentionally empty.
// ORIGINAL: 0x0058EE50 ?desktop_update@@YAXXZ 0x0058EE50-0x0058EE51 BYTE_EXACT
// size      1 bytes
// prototype 
// callers   6   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: n/a
Status: Complete
*/
void __cdecl desktop_update() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x0045C5B0 ?UNK2@MainInterface@@QAEXXZ 0x0045C5B0-0x0045C5B1 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?UNK2@MainInterface@@QAEXXZ)(MainInterface* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void MainInterface::UNK2() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x0045EB60 ?UNK3@MainInterface@@QAEXXZ 0x0045EB60-0x0045EB61 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?UNK3@MainInterface@@QAEXXZ)(MainInterface* this)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void MainInterface::UNK3() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x0045EB70 ?UNK4@MainInterface@@QAEXXZ 0x0045EB70-0x0045EB71 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?UNK4@MainInterface@@QAEXXZ)(MainInterface* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void MainInterface::UNK4() {
}





/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x0045C280 ?UNK1@MainInterface@@QAEXH@Z 0x0045C280-0x0045C283 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?UNK1@MainInterface@@QAEXH@Z)(MainInterface* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void MainInterface::UNK1(int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x0045CC80 ?on_iface_scrolled@MainInterface@@QAEXHH@Z 0x0045CC80-0x0045CC83 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_scrolled@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void MainInterface::on_iface_scrolled(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x0045CC90 ?on_iface_scrolling@MainInterface@@QAEXHH@Z 0x0045CC90-0x0045CC93 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_scrolling@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void MainInterface::on_iface_scrolling(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x0045CCA0 ?on_iface_button_toggled@MainInterface@@QAEXHH@Z 0x0045CCA0-0x0045CCA3 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_button_toggled@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void MainInterface::on_iface_button_toggled(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x0045CDB0 ?on_iface_right_double_click@MainInterface@@QAEXHH@Z 0x0045CDB0-0x0045CDB3 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_right_double_click@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void MainInterface::on_iface_right_double_click(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns 0.
// ORIGINAL: 0x0045D160 ?on_iface_button_pressed@MainInterface@@QAEHH@Z 0x0045D160-0x0045D165 BYTE_EXACT
// size      5 bytes
// prototype int (__thiscall ?on_iface_button_pressed@MainInterface@@QAEHH@Z)(MainInterface* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0, always
Status: Complete
*/
int MainInterface::on_iface_button_pressed(int) {
    return 0;
}

// Fixed-slot bindings carried from 0045c290.cpp
static int *const MainInterfaceTileSource = (int *)0x0078D6B0;
static int *const InterfaceBuffer = (int *)0x007AEC64;
static int *const InterfaceClipRect = (int *)0x007AEC94;
static int *const MainInterfaceTileFlag = (int *)0x007D3930;

// ORIGINAL: 0x0045C290 ?restore_back@MainInterface@@QAEXH@Z 0x0045C290-0x0045C393 FILE
// size      259 bytes
// prototype void (__thiscall ?restore_back@MainInterface@@QAEXH@Z)(MainInterface* this, int)
// callers   12   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005D8000 0x005DA860
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0045C290
// measured tier  MISMATCH
// divergence     4
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0045c290/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?restore_back@MainInterface@@QAEXH@Z  at 0x0045C290  (259 bytes)
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


void MainInterface::restore_back(int a1) {
    char *self = reinterpret_cast<char *>(this);
    if (a1 != 0) {
        reinterpret_cast<Buffer *>(InterfaceBuffer)->set_clip(reinterpret_cast<RECT *>(InterfaceClipRect));
    }

    {
        int *r = reinterpret_cast<int *>(self + 0xd80);
        int x = r[0] + 1;
        int y = r[1] + 1;
        int w = (r[2] - 1) - x;
        int h = (r[3] - 1) - y;
        reinterpret_cast<Buffer *>(InterfaceBuffer)->tile(reinterpret_cast<Buffer *>(*MainInterfaceTileSource), x, y, x, y, w, h);
    }

    {
        int *r = *MainInterfaceTileFlag != 0
            ? reinterpret_cast<int *>(self + 0xdd0)
            : reinterpret_cast<int *>(self + 0xdb0);
        int x = r[0];
        int y = r[1] + 1;
        int w = (r[2] - 1) - x;
        int h = (r[3] - 1) - y;
        reinterpret_cast<Buffer *>(InterfaceBuffer)->tile(reinterpret_cast<Buffer *>(*MainInterfaceTileSource), x, y, x, y, w, h);
    }

    {
        int *r = *MainInterfaceTileFlag != 0
            ? reinterpret_cast<int *>(self + 0xdc0)
            : reinterpret_cast<int *>(self + 0xda0);
        int x = r[0] + 1;
        int y = r[1] + 1;
        int w = (r[2] - 1) - x;
        int h = (r[3] - 1) - y;
        reinterpret_cast<Buffer *>(InterfaceBuffer)->tile(reinterpret_cast<Buffer *>(*MainInterfaceTileSource), x, y, x, y, w, h);
    }

    {
        int *r = reinterpret_cast<int *>(self + 0xd90);
        int x = r[0] + 1;
        int y = r[1] + 1;
        int w = (r[2] - 1) - x;
        int h = (r[3] - 1) - y;
        reinterpret_cast<Buffer *>(InterfaceBuffer)->tile(reinterpret_cast<Buffer *>(*MainInterfaceTileSource), x, y, x, y, w, h);
    }
}


/*
// ORIGINAL: 0x0045C180 ?timer_callback_daemon@MainInterface@@QAEXH@Z 0x0045C180-0x0045C1F4 FILE
// TRIED: the scaffold bound the IAT slot as `static int *const g_00669328`
// and dispatched through its own `UnionRectProc` typedef; the slot is
// USER32!UnionRect (tools/iat_names.py), and windows.h already declares it, so
// TRIED: 2026-09-03 homing pass. The body is faithful - alert_/time1_/time2_
//        by name, the switch and the UnionRect call all land - and it still
//        measures MISMATCH 6/42, because the image CALLS Time::stop
//        (0x00616730) here while time.h keeps `stop` in-class MEASURED so
//        that `close` and `~Time` inline it. VC6 has no __declspec(noinline),
//        so the one source cannot serve both call sites: this tree expands
//        the whole teardown (the `not edi / and edi, 1` flush test) in both
//        switch arms, 80 instructions against the image's 42. Same ceiling
//        time.h already documents for port_to_port / has_tech /
//        do_all_non_input. Fixing it means splitting `stop` into an in-class
//        inline for `close`/`~Time` and an out-of-line definition for the
//        daemon callers - time.h, not this TU.
// TRIED: the scaffold bound the IAT slot as `static int *const g_00669328`
// `call dword ptr [__imp_UnionRect]`. The offsets the scaffold cast through
// are the declared members alert_ / time1_ / time2_
// (tools/header_offsets.py: 0x216f0 / 0x25040 / 0x25068), so the body reads
// them by name now.
*/
void MainInterface::timer_callback_daemon(int a1) {
    RECT local;
    switch (a1) {
    case 0:
        time1_.stop();
        break;
    case 1:
        if (alert_ == -1) {
            time2_.stop();
            return;
        }
        alert_ += 1;
        UnionRect(&local,
                  reinterpret_cast<RECT *>(&field_DE0_),
                  reinterpret_cast<RECT *>(&field_DF0_));
        GraphicWin::soft_update(&local);
        break;
    }
}

// Fixed-slot bindings carried from 0045c5c0.cpp
static int *const IfaceIdToSlot = (int *)0x007AE778;
static int *const IfaceRegistry = (int *)0x007AE7F0;
static int *const ActiveIfaceId = (int *)0x007D392C;

class VCall { public:
    virtual void slot000();
    virtual void slot001(int, int);  // <-- used
    virtual void slot002(int, int);  // <-- used
    virtual void slot003();
    virtual void slot004(int, int);  // <-- used
    virtual void slot005(int, int);  // <-- used
    virtual void slot006(int, int);  // <-- used
    virtual void slot007();
    virtual void slot008();
    virtual void slot009();
    virtual void slot010(int, int);  // <-- used
    virtual void slot011(int, int);  // <-- used
    virtual void slot012(int, int);  // <-- used
    virtual void slot013();
    virtual void slot014();
    virtual void slot015(int, int);  // <-- used
    virtual void slot016(int, int, int);  // <-- used
    virtual void slot017();
    virtual int slot018(GraphicWin *, int, int, RECT *);  // <-- used
    virtual void slot019();
    virtual void slot020();
};

// ORIGINAL: 0x0045C5C0 ?on_scrolling@MainInterface@@QAEXHH@Z 0x0045C5C0-0x0045C5F4 FILE BYTE_EXACT
// size      52 bytes
// prototype void (__thiscall ?on_scrolling@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0045C5ED
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0045C5C0
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0045c5c0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_scrolling@MainInterface@@QAEXHH@Z  at 0x0045C5C0  (52 bytes)
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


void MainInterface::on_scrolling(int a1, int a2) {
    if (WorldClimateSkipTerrainClear != 1u) {
        int index = *ActiveIfaceId;
        int subindex = reinterpret_cast<int *>(IfaceIdToSlot)[index];
        VCall *target = reinterpret_cast<VCall **>(IfaceRegistry)[subindex];
        if (target != 0) {
            target->slot002(a1, a2);
        }
    }
}

class VCall2 { public:
    virtual void slot000();
    virtual void slot001(int, int);
    virtual void slot002();
    virtual void slot003();
    virtual void slot004();
    virtual void slot005();
    virtual void slot006();
    virtual void slot007();
    virtual void slot008();
    virtual void slot009();
    virtual void slot010();
    virtual void slot011(int, int);
};

// ORIGINAL: 0x0045C640 ?on_left_double_click@MainInterface@@QAEXHH@Z 0x0045C640-0x0045C674 FILE BYTE_EXACT
// size      52 bytes
// prototype void (__thiscall ?on_left_double_click@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0045C66D
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0045C640
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0045c640/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_left_double_click@MainInterface@@QAEXHH@Z  at 0x0045C640  (52 bytes)
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


void MainInterface::on_left_double_click(int a1, int a2) {
    if (WorldClimateSkipTerrainClear != 1u) {
        int index = *ActiveIfaceId;
        int subindex = reinterpret_cast<int *>(IfaceIdToSlot)[index];
        VCall2 *target = reinterpret_cast<VCall2 **>(IfaceRegistry)[subindex];
        if (target != 0) {
            target->slot011(a1, a2);
        }
    }
}

// ORIGINAL: 0x0045C680 ?on_right_double_click@MainInterface@@QAEXHH@Z 0x0045C680-0x0045C6B4 FILE BYTE_EXACT
// size      52 bytes
// prototype void (__thiscall ?on_right_double_click@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0045C6AD
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0045C680
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0045c680/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_right_double_click@MainInterface@@QAEXHH@Z  at 0x0045C680  (52 bytes)
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


void MainInterface::on_right_double_click(int a1, int a2) {
    if (WorldClimateSkipTerrainClear != 1u) {
        int interface_index = IfaceIdToSlot[*ActiveIfaceId];
        VCall *target = reinterpret_cast<VCall **>(IfaceRegistry)[interface_index];
        if (target != 0) {
            target->slot012(a1, a2);
        }
    }
}

// ORIGINAL: 0x0045C6C0 ?on_left_up@MainInterface@@QAEXHH@Z 0x0045C6C0-0x0045C6F4 FILE BYTE_EXACT
// size      52 bytes
// prototype void (__thiscall ?on_left_up@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0045C6ED
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0045C6C0
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0045c6c0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_left_up@MainInterface@@QAEXHH@Z  at 0x0045C6C0  (52 bytes)
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


void MainInterface::on_left_up(int a1, int a2) {
    if (WorldClimateSkipTerrainClear != 1u) {
        int group = IfaceIdToSlot[*ActiveIfaceId];
        VCall *target = reinterpret_cast<VCall *>(IfaceRegistry[group]);
        if (target) {
            target->slot006(a1, a2);
        }
    }
}

class VCallLeftClick { public:
    virtual void slot00();
    virtual void slot01();
    virtual void slot02();
    virtual void slot03(int, int);
};

// ORIGINAL: 0x0045C700 ?on_left_click@MainInterface@@QAEXHH@Z 0x0045C700-0x0045C734 FILE BYTE_EXACT
// size      52 bytes
// prototype void (__thiscall ?on_left_click@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0045C72D
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0045C700
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0045c700/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_left_click@MainInterface@@QAEXHH@Z  at 0x0045C700  (52 bytes)
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


void MainInterface::on_left_click(int a1, int a2) {
    if (WorldClimateSkipTerrainClear != 1u) {
        int index = *ActiveIfaceId;
        int subindex = reinterpret_cast<int *>(IfaceIdToSlot)[index];
        VCallLeftClick *target = reinterpret_cast<VCallLeftClick **>(IfaceRegistry)[subindex];
        if (target != 0) {
            target->slot03(a1, a2);
        }
    }
}

// ORIGINAL: 0x0045C740 ?on_right_click@MainInterface@@QAEXHH@Z 0x0045C740-0x0045C774 FILE BYTE_EXACT
// size      52 bytes
// prototype void (__thiscall ?on_right_click@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0045C76D
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0045C740
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0045c740/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_right_click@MainInterface@@QAEXHH@Z  at 0x0045C740  (52 bytes)
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


void MainInterface::on_right_click(int a1, int a2) {
    if (WorldClimateSkipTerrainClear != 1u) {
        int interface_index = IfaceIdToSlot[*ActiveIfaceId];
        VCall *target = reinterpret_cast<VCall **>(IfaceRegistry)[interface_index];
        if (target != 0) {
            target->slot004(a1, a2);
        }
    }
}

// ORIGINAL: 0x0045C780 ?on_left_down@MainInterface@@QAEXHH@Z 0x0045C780-0x0045C7B4 FILE BYTE_EXACT
// size      52 bytes
// prototype void (__thiscall ?on_left_down@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0045C7AD
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0045C780
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0045c780/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_left_down@MainInterface@@QAEXHH@Z  at 0x0045C780  (52 bytes)
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


void MainInterface::on_left_down(int a1, int a2) {
    if (WorldClimateSkipTerrainClear != 1u) {
        int group = IfaceIdToSlot[*ActiveIfaceId];
        VCall *target = reinterpret_cast<VCall *>(IfaceRegistry[group]);
        if (target) {
            target->slot005(a1, a2);
        }
    }
}

// ORIGINAL: 0x0045C7C0 ?on_right_down@MainInterface@@QAEXHH@Z 0x0045C7C0-0x0045C7F4 FILE BYTE_EXACT
// size      52 bytes
// prototype void (__thiscall ?on_right_down@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0045C7ED
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0045C7C0
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0045c7c0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_right_down@MainInterface@@QAEXHH@Z  at 0x0045C7C0  (52 bytes)
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

void MainInterface::on_right_down(int a1, int a2) {
    if (WorldClimateSkipTerrainClear != 1u) {
        int index = *ActiveIfaceId;
        int subindex = reinterpret_cast<int *>(IfaceIdToSlot)[index];
        VCall *target = reinterpret_cast<VCall **>(IfaceRegistry)[subindex];
        if (target != 0) {
            target->slot004(a1, a2);
        }
    }
}

// Fixed-slot bindings carried from 0045c830.cpp

class VCallRet { public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();
    virtual void slot003();
    virtual void slot004();
    virtual void slot005();
    virtual void slot006();
    virtual void slot007();
    virtual void slot008();
    virtual void slot009();
    virtual void slot010();
    virtual void slot011();
    virtual void slot012();
    virtual void slot013();
    virtual int slot014(int a);
};

// ORIGINAL: 0x0045C830 ?on_button_pressed@MainInterface@@QAEHH@Z 0x0045C830-0x0045C866 FILE BYTE_EXACT
// size      54 bytes
// prototype int (__thiscall ?on_button_pressed@MainInterface@@QAEHH@Z)(MainInterface* this, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0045C859
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0045C830
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0045c830/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_button_pressed@MainInterface@@QAEHH@Z  at 0x0045C830  (54 bytes)
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


int MainInterface::on_button_pressed(int a1) {
    if (WorldClimateSkipTerrainClear != 1u) {
        int index1 = *ActiveIfaceId;
        int index2 = IfaceIdToSlot[index1];
        int *obj = reinterpret_cast<int *>(IfaceRegistry[index2]);
        if (obj != 0) {
            return reinterpret_cast<VCallRet *>(obj)->slot014(a1);
        }
    }
    return 0;
}

// ORIGINAL: 0x0045C870 ?on_button_toggled@MainInterface@@QAEXHH@Z 0x0045C870-0x0045C8A4 FILE BYTE_EXACT
// size      52 bytes
// prototype void (__thiscall ?on_button_toggled@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0045C89D
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0045C870
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0045c870/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_button_toggled@MainInterface@@QAEXHH@Z  at 0x0045C870  (52 bytes)
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


void MainInterface::on_button_toggled(int a1, int a2) {
    if (WorldClimateSkipTerrainClear != 1u) {
        int interface_index = IfaceIdToSlot[*ActiveIfaceId];
        VCall *target = reinterpret_cast<VCall **>(IfaceRegistry)[interface_index];
        if (target != 0) {
            target->slot015(a1, a2);
        }
    }
}

class ADBackDrawVCall {
public:
    virtual void slot00();
    virtual void slot01();
    virtual void slot02();
    virtual void slot03();
    virtual void slot04();
    virtual void slot05();
    virtual void slot06();
    virtual void slot07();
    virtual void slot08();
    virtual void slot09();
    virtual void slot10();
    virtual void slot11();
    virtual void slot12();
    virtual void slot13();
    virtual void slot14();
    virtual void slot15();
    virtual void slot16();
    virtual int slot17(GraphicWin *);
};

// ORIGINAL: 0x0045C8B0 ?on_dialog_back_draw@MainInterface@@QAEHPAUGraphicWin@@@Z 0x0045C8B0-0x0045C8E6 FILE BYTE_EXACT
// symbol    ?on_dialog_back_draw@MainInterface@@QAEHPAVGraphicWin@@@Z
// size      54 bytes
// prototype int (__thiscall ?on_dialog_back_draw@MainInterface@@QAEHPAUGraphicWin@@@Z)(MainInterface* this, GraphicWin*)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0045C8D9
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0045C8B0
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0045c8b0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_dialog_back_draw@MainInterface@@QAEHPAUGraphicWin@@@Z  at 0x0045C8B0  (54 bytes)
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


int MainInterface::on_dialog_back_draw(GraphicWin* a1) {
    if (WorldClimateSkipTerrainClear != 1u) {
        int index2 = IfaceIdToSlot[*ActiveIfaceId];
        int ptr_val = IfaceRegistry[index2];
        if (ptr_val != 0) {
            return reinterpret_cast<ADBackDrawVCall *>(ptr_val)->slot17(a1);
        }
    }
    return 0;
}

// ORIGINAL: 0x0045C8F0 ?on_dialog_item_back_draw@MainInterface@@QAEHPAUGraphicWin@@HHPAURECT@@@Z 0x0045C8F0-0x0045C932 FILE BYTE_EXACT
// symbol    ?on_dialog_item_back_draw@MainInterface@@QAEHPAVGraphicWin@@HHPAUtagRECT@@@Z
// size      66 bytes
// prototype int (__thiscall ?on_dialog_item_back_draw@MainInterface@@QAEHPAUGraphicWin@@HHPAURECT@@@Z)(MainInterface* this, GraphicWin*, int, int, RECT*)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0045C925
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0045C8F0
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0045c8f0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_dialog_item_back_draw@MainInterface@@QAEHPAUGraphicWin@@HHPAURECT@@@Z  at 0x0045C8F0  (66 bytes)
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


int MainInterface::on_dialog_item_back_draw(GraphicWin* a1, int a2, int a3, RECT* a4) {
    if (WorldClimateSkipTerrainClear != 1u) {
        int idx1 = *ActiveIfaceId;
        int idx2 = IfaceIdToSlot[idx1];
        VCall *obj = reinterpret_cast<VCall *>(IfaceRegistry[idx2]);
        if (obj != 0) {
            return obj->slot018(a1, a2, a3, a4);
        }
    }
    return 0;
}

class OnMouseMoveVCall {
 public:
  virtual void slot000();
  virtual void slot001();
  virtual void slot002();
  virtual void slot003();
  virtual void slot004();
  virtual void slot005();
  virtual void slot006();
  virtual void slot007();
  virtual void slot008(int, int);
};

// ORIGINAL: 0x0045C980 ?on_mouse_move@MainInterface@@QAEXHH@Z 0x0045C980-0x0045C9B4 BYTE_EXACT
// size      52 bytes
// prototype void (__thiscall ?on_mouse_move@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0045C9AD
// 0x0045C980  ?on_mouse_move@MainInterface@@QAEXHH@Z  ->  ?on_mouse_move@MainInterface@@QAEXHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.


void MainInterface::on_mouse_move(int a1, int a2) {
    if (WorldClimateSkipTerrainClear != 1u) {
        int mid = IfaceIdToSlot[*ActiveIfaceId];
        int obj = IfaceRegistry[mid];
        if (obj != 0) {
            reinterpret_cast<OnMouseMoveVCall *>(obj)->slot008(a1, a2);
        }
    }
}

// ORIGINAL: 0x0045C940 ?on_group_clicked@MainInterface@@QAEXHHH@Z 0x0045C940-0x0045C978 FILE BYTE_EXACT
// size      56 bytes
// prototype void (__thiscall ?on_group_clicked@MainInterface@@QAEXHHH@Z)(MainInterface* this, int, int, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0045C971
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0045C940
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0045c940/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_group_clicked@MainInterface@@QAEXHHH@Z  at 0x0045C940  (56 bytes)
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


void MainInterface::on_group_clicked(int a1, int a2, int a3) {
    if (WorldClimateSkipTerrainClear != 1u) {
        VCall *target = reinterpret_cast<VCall *>(IfaceRegistry[IfaceIdToSlot[*ActiveIfaceId]]);
        if (target != 0) {
            target->slot016(a1, a2, a3);
        }
    }
}

class VCallII { public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();
    virtual void slot003();
    virtual void slot004();
    virtual void slot005();
    virtual void slot006();
    virtual void slot007();
    virtual void slot008();
    virtual void slot009(int, int);
};

// ORIGINAL: 0x0045C9C0 ?on_mouse_leave@MainInterface@@QAEXHH@Z 0x0045C9C0-0x0045C9F4 BYTE_EXACT
// symbol    ?on_mouse_leave@MainInterface@@UAEXHH@Z
// size      52 bytes
// prototype void (__thiscall ?on_mouse_leave@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0045C9ED
// 0x0045C9C0  ?on_mouse_leave@MainInterface@@QAEXHH@Z  ->  ?on_mouse_leave@MainInterface@@QAEXHH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.


void MainInterface::on_mouse_leave(int a1, int a2) {
    if (WorldClimateSkipTerrainClear != 1u) {
        int index = *ActiveIfaceId;
        int subindex = reinterpret_cast<int *>(IfaceIdToSlot)[index];
        VCallII *target = reinterpret_cast<VCallII **>(IfaceRegistry)[subindex];
        if (target != 0) {
            target->slot009(a1, a2);
        }
    }
}

// ORIGINAL: 0x0045CA00 ?on_selected@MainInterface@@QAEXHH@Z 0x0045CA00-0x0045CA34 FILE BYTE_EXACT
// size      52 bytes
// prototype void (__thiscall ?on_selected@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0045CA2D
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0045CA00
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0045ca00/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_selected@MainInterface@@QAEXHH@Z  at 0x0045CA00  (52 bytes)
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


void MainInterface::on_selected(int a1, int a2) {
    if (WorldClimateSkipTerrainClear != 1u) {
        int interface_index = IfaceIdToSlot[*ActiveIfaceId];
        VCall *target = reinterpret_cast<VCall **>(IfaceRegistry)[interface_index];
        if (target != 0) {
            target->slot010(a1, a2);
        }
    }
}

// ORIGINAL: 0x0045D2A0 ?set_alert@MainInterface@@QAEXH@Z 0x0045D2A0-0x0045D2D1 BYTE_EXACT
// size      49 bytes
// prototype void (__thiscall ?set_alert@MainInterface@@QAEXH@Z)(MainInterface* this, int)
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00616650
// 0x0045D2A0  ?set_alert@MainInterface@@QAEXH@Z  ->  ?set_alert@MainInterface@@QAEXH@Z
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.


void MainInterface::set_alert(int a1) {
    char *self = reinterpret_cast<char *>(this);
    if (a1 != 0) {
        *reinterpret_cast<int *>(self + 0x216f0) = 0;
        reinterpret_cast<Time *>(self + 0x25068)->start();
        return;
    }
    *reinterpret_cast<int *>(self + 0x216f0) = -1;
}

// Fixed-slot bindings carried from 0045c600.cpp

// ORIGINAL: 0x0045C600 ?on_scrolled@MainInterface@@QAEXHH@Z 0x0045C600-0x0045C634 FILE BYTE_EXACT
// size      52 bytes
// prototype void (__thiscall ?on_scrolled@MainInterface@@QAEXHH@Z)(MainInterface* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0045C62D
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0045C600
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0045c600/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_scrolled@MainInterface@@QAEXHH@Z  at 0x0045C600  (52 bytes)
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


void MainInterface::on_scrolled(int a1, int a2) {
    if (WorldClimateSkipTerrainClear != 1u) {
        int group = IfaceIdToSlot[*ActiveIfaceId];
        VCall *target = reinterpret_cast<VCall *>(IfaceRegistry[group]);
        if (target) {
            target->slot001(a1, a2);
        }
    }
}


/*
// ORIGINAL: 0x0045C200 ?timer_callback_daemon@MainInterface@@QAAXHH@Z 0x0045C200-0x0045C273
// symbol    ?timer_callback_daemon@@YAXHH@Z
// TRIED: both stack parameters are read directly ([ebp+8], [ebp+0xc]) with no
//        `this` anywhere, so the scaffold's MainInterface member cannot be
//        defined out-of-line - that adds an implicit `this` ahead of them and
//        shifts both to +0xc/+0x10. It is a free function here.
// TRIED: the same `Time::stop` ceiling as 0x0045C180. The image CALLS
//        0x00616730; time.h keeps `stop` in-class MEASURED so `close`/`~Time`
//        inline it, and VC6 has no noinline. Demoted rather than mismatched
//        forever; split `stop` in time.h and both bodies re-earn.
*/
void __cdecl timer_callback_daemon(int a1, int a2) {
    if (a2 == 0) return;

    char *self = reinterpret_cast<char *>(a2);
    switch (a1) {
    case 0:
        reinterpret_cast<Time *>(self + 0x25040)->stop();
        break;
    case 1: {
        int *count = reinterpret_cast<int *>(self + 0x216f0);
        if (*count == -1) {
            reinterpret_cast<Time *>(self + 0x25068)->stop();
            return;
        }
        *count = *count + 1;

        RECT rc;
        UnionRect(&rc, reinterpret_cast<RECT *>(self + 0xde0),
                  reinterpret_cast<RECT *>(self + 0xdf0));
        reinterpret_cast<GraphicWin *>(self)->soft_update(&rc);
        break;
    }
    }
}
