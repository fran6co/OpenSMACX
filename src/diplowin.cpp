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
#include "diplowin.h"
#include "vtable_shim.h"
#include "netdaemon.h"
#include "basepop.h"

// Slot 62, 0xF8 - the DiploWin front vtable's update entry, the one
// done_deal tails into with the image's own `jmp dword ptr [eax + 0xf8]`.
namespace {
typedef void (__fastcall *func_diplo_win_slot062)(void *);
}
// DiploWin::show reaches the SubInterface subobject the original places at
// +0xA14 by casting a hand-computed address, so the type must be complete.
#include "subinterface.h"

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x00442A00 ?UNK5@DiploWin@@QAEXXZ 0x00442A00-0x00442A01 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?UNK5@DiploWin@@QAEXXZ)(DiploWin* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploWin::UNK5() {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x004429F0 ?UNK4@DiploWin@@QAEXH@Z 0x004429F0-0x004429F3 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?UNK4@DiploWin@@QAEXH@Z)(DiploWin* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploWin::UNK4(int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00443D80 ?on_iface_left_click@DiploWin@@QAEXHH@Z 0x00443D80-0x00443D83 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_left_click@DiploWin@@QAEXHH@Z)(DiploWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploWin::on_iface_left_click(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00443D90 ?on_iface_left_up@DiploWin@@QAEXHH@Z 0x00443D90-0x00443D93 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_left_up@DiploWin@@QAEXHH@Z)(DiploWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploWin::on_iface_left_up(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00443DA0 ?on_iface_right_click@DiploWin@@QAEXHH@Z 0x00443DA0-0x00443DA3 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_right_click@DiploWin@@QAEXHH@Z)(DiploWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploWin::on_iface_right_click(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00443DB0 ?on_iface_left_down@DiploWin@@QAEXHH@Z 0x00443DB0-0x00443DB3 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_left_down@DiploWin@@QAEXHH@Z)(DiploWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploWin::on_iface_left_down(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00443DC0 ?on_iface_right_down@DiploWin@@QAEXHH@Z 0x00443DC0-0x00443DC3 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_right_down@DiploWin@@QAEXHH@Z)(DiploWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploWin::on_iface_right_down(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00443DD0 ?on_iface_mouse_move@DiploWin@@QAEXHH@Z 0x00443DD0-0x00443DD3 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_mouse_move@DiploWin@@QAEXHH@Z)(DiploWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploWin::on_iface_mouse_move(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00443DE0 ?on_iface_mouse_leave@DiploWin@@QAEXHH@Z 0x00443DE0-0x00443DE3 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_mouse_leave@DiploWin@@QAEXHH@Z)(DiploWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploWin::on_iface_mouse_leave(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00443DF0 ?on_iface_selected@DiploWin@@QAEXHH@Z 0x00443DF0-0x00443DF3 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_selected@DiploWin@@QAEXHH@Z)(DiploWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploWin::on_iface_selected(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00443E00 ?on_iface_left_double_click@DiploWin@@QAEXHH@Z 0x00443E00-0x00443E03 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_left_double_click@DiploWin@@QAEXHH@Z)(DiploWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploWin::on_iface_left_double_click(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00443E10 ?on_iface_right_double_click@DiploWin@@QAEXHH@Z 0x00443E10-0x00443E13 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_right_double_click@DiploWin@@QAEXHH@Z)(DiploWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploWin::on_iface_right_double_click(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00443E20 ?on_iface_button_clicked@DiploWin@@QAEXH@Z 0x00443E20-0x00443E23 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_button_clicked@DiploWin@@QAEXH@Z)(DiploWin* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploWin::on_iface_button_clicked(int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00443E30 ?on_iface_button_toggled@DiploWin@@QAEXHH@Z 0x00443E30-0x00443E33 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?on_iface_button_toggled@DiploWin@@QAEXHH@Z)(DiploWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploWin::on_iface_button_toggled(int, int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns 0.
// ORIGINAL: 0x00443E40 ?on_iface_dialog_back_draw@DiploWin@@QAEHH@Z 0x00443E40-0x00443E45 BYTE_EXACT
// size      5 bytes
// prototype int (__thiscall ?on_iface_dialog_back_draw@DiploWin@@QAEHH@Z)(DiploWin* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0, always
Status: Complete
*/
int DiploWin::on_iface_dialog_back_draw(int) {
    return 0;
}


/*
Purpose: Clear the two cached values at 0xA24 and 0xA28.
// ORIGINAL: 0x004413B0 ?UNK2@DiploWin@@QAEXXZ 0x004413B0-0x004413BE
// size      14 bytes
// prototype void (__thiscall ?UNK2@DiploWin@@QAEXXZ)(DiploWin* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploWin::UNK2() {
    field_A24_ = 0;
    field_A28_ = 0;
}


/*
Purpose: Clear one entry of the array at 0xA1C, then clear the two words at
         0xA24 and 0xA28.

             mov dword [ecx+eax*4+0xA1C],0
             lea eax,[ecx+0xA24] / xor ecx,ecx
             mov [eax],ecx / mov [eax+4],ecx

         The index is NOT bounds checked - the original stores through
         `eax*4` whatever the caller passed - so this does not add a guard the
         program does not have.

         Note the array at 0xA1C overlaps the two words cleared afterwards:
         index 2 IS the word at 0xA24, and index 3 the one at 0xA28. So calling
         with 2 or 3 writes the same slot twice, which is visible only because
         the fixture checks exact bytes rather than the cleared entry alone.
// ORIGINAL: 0x004413C0 ?UNK3@DiploWin@@QAEXH@Z 0x004413C0-0x004413E2
// size      34 bytes
// prototype void (__thiscall ?UNK3@DiploWin@@QAEXH@Z)(DiploWin* this, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void DiploWin::UNK3(int a1) {
    uint8_t *const self = reinterpret_cast<uint8_t *>(this);
    *reinterpret_cast<int32_t *>(self + 0xA1C + a1 * 4) = 0;
    int32_t *const pair = reinterpret_cast<int32_t *>(self + 0xA24);
    pair[0] = 0;
    pair[1] = 0;
}


/*
// ORIGINAL: 0x00443C30 ?show@DiploWin@@QAEXH@Z 0x00443C30-0x00443C4B BYTE_EXACT
// symbol    ?show@DiploWin@@UAEXH@Z
// size      27 bytes
// prototype void (__thiscall ?show@DiploWin@@QAEXH@Z)(DiploWin* this, int)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0045D310 0x005F7E90
Status: Complete
*/
void DiploWin::show(int visible) {
    if (!reinterpret_cast<Win *>(this)->is_visible()) {
        reinterpret_cast<SubInterface *>(reinterpret_cast<char *>(this) + 0xa14)->set_iface_mode();
    }
}

/*
// ORIGINAL: 0x00443C50 ?hide@DiploWin@@QAEXXZ 0x00443C50-0x00443C68 BYTE_EXACT
// symbol    ?hide@DiploWin@@UAEXXZ
// size      24 bytes
// prototype void (__thiscall ?hide@DiploWin@@QAEXXZ)(DiploWin* this)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005F7E90
// indirect  0x00443C60
Status: Complete
*/
void DiploWin::hide() {
    if (reinterpret_cast<Win *>(this)->is_visible()) {
        release_modal();
    }
}

// DiploOffersDirty (0x007492D0) - the diplomacy-window redraw latch. add_offer,
// kill_offer, new_chat and on_button_clicked all set it to 1 as they change the
// offer lists; exec (0x00443F10) tests it and clears it. `int *const` folds to
// the bare immediate, so the rename moves no byte.
static int *const DiploOffersDirty = (int *)0x007492D0;

// ORIGINAL: 0x004413F0 ?kill_offer@DiploWin@@QAEXHH@Z 0x004413F0-0x0044148C FILE
// TRIED: `int idx = a2;` declared inside the inner `if (a2 < count - 1)` (mismatch, idx assign shifted 5 instructions late) and hoisted above it with a separate `countMinus1` temp (MNEMONIC_ONLY, worse). Landed the inner-if placement: MISMATCH #23, mnemonic similarity 0.98, same byte count (156/156).
// working copy - scaffold materialised by --work
// size      156 bytes
// prototype void (__thiscall ?kill_offer@DiploWin@@QAEXHH@Z)(DiploWin* this, int, int)
// callers   3   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00592EE0
// indirect  0x0044147F

void DiploWin::kill_offer(int a1, int a2) {
    char *self = reinterpret_cast<char *>(this);
    if (a1 == 0) {
        message_data(0x1507, *reinterpret_cast<int *>(self + 0xAB8), a2, 0, 0, 0);
    }
    int count = *reinterpret_cast<int *>(self + 0xA1C + a1 * 4);
    if (a2 < count) {
        if (a2 < count - 1) {
            char *rec = self + 0xA2C + a1 * 32 + a2 * 4;
            int idx = a2;
            do {
                idx = idx + 1;
                *reinterpret_cast<int *>(rec) = *reinterpret_cast<int *>(rec + 4);
                *reinterpret_cast<int *>(rec + 0x40) = *reinterpret_cast<int *>(rec + 0x44);
                rec += 4;
                count = *reinterpret_cast<int *>(self + 0xA1C + a1 * 4);
            } while (idx < count - 1);
        }
        count = *reinterpret_cast<int *>(self + 0xA1C + a1 * 4) - 1;
        *reinterpret_cast<int *>(self + 0xA1C + a1 * 4) = count;
        *DiploOffersDirty = 1;
        *reinterpret_cast<int *>(self + 0xA24) = 0;
        *reinterpret_cast<int *>(self + 0xA28) = 0;
        vtable_slot<func_diplo_win_slot062>(this, 0xf8)(this);
    }

}

// ORIGINAL: 0x00441490 ?add_offer@DiploWin@@QAEXHHH@Z 0x00441490-0x004415B9 FILE
// size      297 bytes
// prototype void (__thiscall ?add_offer@DiploWin@@QAEXHHH@Z)(DiploWin* this, int, int, int)
// callers   3   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004413F0 0x00592EE0
// indirect  0x00441598
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00441490
// measured tier  MISMATCH
// divergence     2
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00441490/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?add_offer@DiploWin@@QAEXHHH@Z  at 0x00441490  (297 bytes)
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


void DiploWin::add_offer(int a1, int a2, int a3) {
    char *self = reinterpret_cast<char *>(this);
    int *count_arr = reinterpret_cast<int *>(self + 0xa1c);
    int *type_arr = reinterpret_cast<int *>(self + 0xa2c);
    int *val_arr = reinterpret_cast<int *>(self + 0xa6c);

    *DiploOffersDirty = 1;

    int i;
    for (i = 0; i < count_arr[a1]; i++) {
        if (a2 == 4 || a2 == 2 || a2 == 3) {
            int existing = type_arr[i + a1 * 8];
            if (existing == 4 || existing == 2 || existing == 3) {
                goto found;
            }
        } else if (a2 == 1) {
            if (type_arr[i + a1 * 8] == 1) {
                if (a3 <= 0) {
                    kill_offer(a1, i);
                    return;
                }
                goto found;
            }
        } else if (a2 == 0) {
            if (type_arr[i + a1 * 8] == 0 && val_arr[i + a1 * 8] == a3) {
                return;
            }
        } else if (a2 == 5) {
            if (type_arr[i + a1 * 8] == 5 && val_arr[i + a1 * 8] == a3) {
                return;
            }
        }
    }

    {
        int count = count_arr[a1];
        if (count >= 8) {
            return;
        }
        count_arr[a1] = count + 1;
        i = count;
    }

found:
    {
        int slot = i + a1 * 8;
        type_arr[slot] = a2;
        val_arr[slot] = a3;
        *reinterpret_cast<int *>(self + 0xa24) = 0;
        *reinterpret_cast<int *>(self + 0xa28) = 0;
        if (a1 == 0) {
            message_data(0x1506, *reinterpret_cast<int *>(self + 0xab8),
                        a2, a3, a1, a1);
        }
        vtable_slot<func_diplo_win_slot062>(this, 0xf8)(this);
    }
}

// ORIGINAL: 0x00442510 ?on_left_click@DiploWin@@QAEXHH@Z 0x00442510-0x00442558 FILE BYTE_EXACT
// LEVER: homed from units/00442510.cpp once kill_offer (0x004413F0) landed in
// this TU - the call was the only thing blocking it. The declared `spot_`
// member at 0xED0 and the same vtable_slot dispatch done_deal uses replaced
// the artifact's cast of `this` and its VCall shim.
// size      72 bytes
// prototype void (__thiscall ?on_left_click@DiploWin@@QAEXHH@Z)(DiploWin* this, int xCoord, int yCoord)
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004413F0 0x005FAB00
// indirect  0x0044254D
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00442510
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00442510/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_left_click@DiploWin@@QAEXHH@Z  at 0x00442510  (72 bytes)
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


void DiploWin::on_left_click(int a1, int a2) {
    // `check`'s two int* out-params reuse the parameter slots, so the same
    // names come back carrying the spot position and the button kind.
    if (spot_.check(a1, a2, &a1, &a2) >= 0 && a2 == 0) {
        kill_offer(0, a1);
        vtable_slot<func_diplo_win_slot062>(this, 0xf8)(this);
    }
}

