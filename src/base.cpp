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
#define DIALOGS_DTOR_INLINE  // stand-in: see ~Dialogs in dialogs.h
#include "stdafx.h"
#include "popup.h"
#include "base.h"
#include "planwin.h"
#include "council.h"
#include "alpha.h"
#include "faction.h"
#include "game.h"
#include "log.h"
#include "map.h"
#include "text.h"
#include "technology.h"
#include "veh.h"
#include "console.h"
#include "datalink.h"
#include "netdaemon.h"
#include "xpops.h"
#include "buffer.h"
#include "font.h"
#include "font_queue.h"
#include "infowin.h"
#include "maininterface.h"
#include "mapwin.h"
#include "general.h"
#include "netmsg.h"
#include "replaywin.h"
#include "messagewin.h"
#include "tutwin.h"
#include "path.h"
#include "terraforming.h"
#include "interlude.h"
#include "worldwin.h"
#include "crt_random_inline.h"
#include "alphamovie.h"
#include "strings.h"
#include "wave.h"
#include "win.h"
#include <string.h>
#include <stdlib.h>

// Shared punctuation objects the image pushes instead of unit literals.
extern char SocialWinCloseParenText[];
extern char SocialWinOpenParenText[];
// Recovery spelling (name unknown): has_fac() on the selected base, one
// inline level deep. The image keeps that nested has_fac() as a real call
// where VC6's inline budget runs out (base_minerals, base_doctors).
static inline BOOL selected_base_has_fac(int facility_id) {
    return has_fac(facility_id, BaseIDCurrentSelected, 0);
}


// WORKAROUND(inline): has_project() spelled through base_project(), as the
// header's comment says makes base_minerals (0x004E9CB0) exact; the header
// keeps the operator[] read for allocate_energy (0x005267B0). Only
// base_minerals uses this twin.
static inline BOOL has_project_via_base_project(int project_id, int faction_id) {
    int base_id = base_project(project_id);
    if (base_id < 0) {
        return false;
    }
    return Bases[base_id].faction_id_current == faction_id;
}

// RECOVERY_OBJECT: v2 0x00689370 0x00689374 ?BaseIDCurrentSelected@@3HA 0x004E68B0
int BaseIDCurrentSelected = -1;  // 0x00689370
// set_base publishes the pointer itself; resource computations and the popup
// sound callback load it before accessing the selected Base's members.
// RECOVERY_OBJECT: v2 0x0090EA30 0x0090EA34 ?BaseCurrent@@3PAUBase@@A 0x004E39D0
Base *BaseCurrent;
// production_phase publishes the current faction's reserves here (526FB7);
// allocate_energy reads the retained value (526821), and hurry deducts from it.
// The name is provisional; producer/allocator bodies remain retained seeds.
// RECOVERY_OBJECT: v2 0x0093A958 0x0093A95C ?CurrentFactionEnergyReserves@@3HA 0x00526E70
int CurrentFactionEnergyReserves;
// base_compute publishes the selected id before running the resource passes.
// Window refreshes invalidate this cache without changing the selected base.
// RECOVERY_OBJECT: v2 0x00689374 0x00689378 ?BaseComputedID@@3HA 0x004EC3B0
int BaseComputedID = -1;
// Historical object identity moved to GameDataStateStorage; see the transport manifest.
// 0x009A64CC
// RECOVERY_OBJECT: v2 0x0090EA04 0x0090EA08 ?BaseFindDist@@3HA 0x004CA1CD
int BaseFindDist;  // 0x0090EA04
// 0x009A6814
// RECOVERY_OBJECT: v2 0x0090EA34 0x0090EA38 ?BaseUpkeepStage@@3IA 0x004E68B0
uint32_t BaseUpkeepStage;  // 0x0090EA34

// Shared publications of the base resource-allocation pass, initially zero.
// The adequacy flag is also read by base_doctors; successful reuse of an
// existing allocation deliberately leaves its previous value untouched.
// RECOVERY_OBJECT: v2 0x0090E938 0x0090E93C ?BasePopulationUnassigned@@3HA 0x004E80B0
int BasePopulationUnassigned;
// RECOVERY_OBJECT: v2 0x0090E900 0x0090E904 ?BaseNutrientAdequate@@3HA 0x004E80B0
int BaseNutrientAdequate;

// base_support clears four resource slots in each initially-zero publication.
// Its convoy-order selector and the resource passes use the original 0..3
// resource domain; invalid selectors are not evidence of larger arrays.
// RECOVERY_OBJECT: v2 0x0090E904 0x0090E914 ?BaseCurrentConvoyFrom@@3PAIA 0x004E9550
uint32_t BaseCurrentConvoyFrom[4];
// RECOVERY_OBJECT: v2 0x0090E93C 0x0090E94C ?BaseCurrentConvoyTo@@3PAIA 0x004E9550
uint32_t BaseCurrentConvoyTo[4];

// Original base_energy writes one entry for each partner faction 1..7 and
// draw_commerce reads those same 4-byte slots. The adjacent known objects at
// 0x0090E980 and 0x0090EA30 bound each seven-entry publication.
// RECOVERY_OBJECT: v2 0x0090EA10 0x0090EA2C ?BaseCommerceReceipts@@3PAHA 0x004EB560
int BaseCommerceReceipts[7];
// RECOVERY_OBJECT: v2 0x0090E964 0x0090E980 ?BaseCommercePartnerReceipts@@3PAHA 0x004EB560
int BaseCommercePartnerReceipts[7];

// Defined beside base_claimed in base_build.cpp; base_reset uses them as the four
// mutable candidate-site publications returned by base_build.
extern int BaseSiteClaimX[4];
extern int BaseSiteClaimY[4];


// The scoring expressions use x86 low-word arithmetic followed by signed
// division. Express that wrapping without signed overflow or negative shifts.
static int base_yield_signed_bits(uint32_t value) {
    return value <= 0x7FFFFFFFu ? static_cast<int>(value)
        : -1 - static_cast<int>(0xFFFFFFFFu - value);
}
static int base_yield_add(int left, int right) {
    return base_yield_signed_bits(static_cast<uint32_t>(left) + static_cast<uint32_t>(right));
}








/*
 * Integer swap has a real out-of-line owner in general.cpp so this mapped
 * caller reaches the original helper at 0x00628A50.
 */

/*
 * energy_yield publishes the clamped energy of a square that contains a base
 * here. Its only other reader in the image is base_terraform (0x004ECCBD).
 */
// Bound: dword-only accesses; 0x0090E918 is an independently accessed dword.
// RECOVERY_OBJECT: v2 0x0090E914 0x0090E918 ?BaseSquareEnergy@@3HA 0x004E7D08
int BaseSquareEnergy;  // 0x0090E914

/*
 * The faction elected Planetary Governor: written by CouncWin::make_resolution
 * (0x004246E5), persisted by game_data (0x005A6B61), read by call_council,
 * wants_prop, num_objectives and the council windows. energy_yield grants it a
 * point of energy on a base square.
 *
 * spying.cpp's former SpyingObserverFaction view named this same datum:
 * the Governor may view other non-alien factions' bases. Both Council writes
 * and visibility checks must use this one source object, not independent
 * integers justified by the retired separate-suite build.
 */
// Historical object identity moved to GameDataStateStorage; see the transport manifest.
// 0x009A6614
// Former independent scalar views CommerceDoubleTarget (009A6624) and
// CommerceDoubleProgress (009A6628) actually denote CouncilResolutionCount[3]
// and [4]. council_action increments those proposal counters and setup_game
// resets the whole array. base_energy doubles commerce while enacted trade
// pacts outnumber repeals; separate scalar storage would lose that relationship.

/*
 * A two-turn state advanced by random_events (0x0051F2F4 steps it 1 -> 2 -> 0
 * at turn start and arms it at 0x00520529). While the selected bit is set,
 * energy_yield triples its result.
 */
// 0x009A6818

/*
 * Selects which bit of GlobalEnergyEventState energy_yield consults. Read by
 * Console::on_key_click, faction_upkeep, NetDaemon::lock_base, wait_loop,
 * check_spock, base_yield and base_doctors; sits immediately below
 * ConsoleControlTurnActive (0x0093A938) and near IsMultiplayerPBEM (0x0093A95C).
 * TODO: identify global - its writer was not located, so it is not named.
 */
// Pinned NetDaemon::lock_base reads this source-owned int at 0x0093A934;
// start-only binding, not an allocation extent or recovered writer.
// RECOVERY_OBJECT: v1 0x0093A934 ?UnkGlobal0093A934@@3HA
int UnkGlobal0093A934;  // 0x0093A934

/*
Purpose: Check if the base already has a particular facility built or if it's in the queue.
// ORIGINAL: 0x00421670 ?has_fac@@YA_NHHH@Z 0x00421670-0x004216E9 BYTE_EXACT
// body      src/base.h
// LEVER: deleted the added `queue_count > 10` upper-bounds guard (see the BUG IN THE ORIGINAL comment at the site) - the image has no such check, only the for-loop's own entry test. Best similarity 0.755 -> 0.779 (2/54 agreeing either way). This is a behavioural change, not just a byte-match one: a caller passing queue_count > 10 now walks past the end of queue_production_id, matching the original's own defect.
// LEVER: swapped the inline `has_fac_built(facility_id, base_id)` call for
//   base.h's `has_fac_built_call` (real out-of-line bitmask_call, not the
//   folded-shift `MEASURED inline bitmask`), which the earlier TRIED note
//   on this marker had left unused - moved similarity 2/54 -> 17/54 by
//   itself. The rest closed by changing has_fac_built_call's RETURN TYPE
//   from `bool` to `int`: a `bool`-returning inline forces a `setne al` /
//   `and eax, 0xff` widen-to-BOOL at the call site, where the image's own
//   `(field & mask) != 0` compiles the `neg al; sbb eax, eax; neg eax`
//   idiom instead, so returning `int` (already 0/1 from the boolean
//   expression) drops the widen. 54/54 BYTE_EXACT once both landed.
// symbol    ?has_fac@@YAHHHH@Z
// size      121 bytes
// prototype bool (__cdecl ?has_fac@@YA_NHHH@Z)(int facilityID, int baseID, int queueCount)
// callers   14   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0050BA00
Return Value: Does base already have or planning on building facility? true/false
Status: Complete
*/

// BODY IN base.h, as `MEASURED inline`: callers write has_fac(id, base_id, 0)
// out in place, and the image keeps an out-of-line copy here for the rest.

/*
Purpose: Set the current base globals.
// ORIGINAL: 0x004E39D0 ?set_base@@YAXH@Z 0x004E39D0-0x004E39F3 BYTE_EXACT
// LEVER: under the TU's /Og /Oi /Ot /Gs recipe the pointer store written
//   first keeps base_id live in eax, so VC6 still emits the id store first
//   and computes the pointer in ecx/edx; id-first source reuses eax (8/11).
// size      35 bytes
// prototype void (__cdecl ?set_base@@YAXH@Z)(int baseID)
// callers   40   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl set_base(int base_id) {
    BaseCurrent = &Bases[base_id];
    BaseIDCurrentSelected = base_id;
}


/*
Purpose: Get base name string from base id and store it in base_str. If base id is -1, use 'NONE'.
// ORIGINAL: 0x004E3A00 ?say_base@@YAXPADH@Z 0x004E3A00-0x004E3A4B BYTE_EXACT
// size      75 bytes
// prototype void (__cdecl ?say_base@@YAXPADH@Z)(int8* strBase, int baseID)
// callers   13   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x006169A0 0x00645470
// LEVER: dropped std::string entirely for the image's two-branch form - a
//        real strcat() call in each branch rather than selecting a pointer
//        and calling once - and made label_get __forceinline (alpha.h) so it
//        folds to `StringTable.get(...)` the way the image does. Best
//        similarity 0.964 (16/28 agreeing, /c /O2 /Gy /GR- /Oy- /GX), up
//        from 2/28. Residual: the image keeps a full ebp frame here and
//        this tree's winning flag set omits it despite /Oy-, not chased
//        further.
Return Value: n/a
Status: Complete
*/
void __cdecl say_base(LPSTR base_str, int base_id) {
    if (base_id < 0) {
        strcat(base_str, label_get(25)); // 'NONE'
    } else {
        strcat(base_str, Bases[base_id].name_string);
    }
}

/*
Purpose: Return the base at the specified map coordinates and repair stale base map bits.
// ORIGINAL: 0x004E3A50 ?base_at@@YAHHH@Z 0x004E3A50-0x004E3B7F BYTE_EXACT
// size      303 bytes
// prototype int (__cdecl ?base_at@@YAHHH@Z)(int xCoord, int yCoord)
// callers   62   call targets   5
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005108A0 0x00532B70 0x005BF310 0x00625E30 0x006262F0
// indirect  0x004E3B59
// notes     Runtime redirect installed by DllMain after byte-signature validation
// LEVER: the missing debug path (STATE_DEBUG_MODE gate, reentrancy guard, BASEBIT
//        popup, GetAsyncKeyState(VK_CONTROL) poll, Console::focus) was the whole
//        call-count gap. Once restored, a pointer walk over Bases (lever 7) -
//        `Base *base = Bases; ...; base_id++, base++` instead of `&Bases[base_id]`
//        each iteration - closed the last two instructions.
Return Value: Base id or -1 when no base is present
Status: Complete - testing

The original does display the debug-only BASEBIT popup and focus the internal console when the map
bit has no matching base, gated by STATE_DEBUG_MODE and a reentrancy guard; that path is reproduced
below. BaseAtDebugPopupFlag and BaseAtKeyPollFlag gate a parse_num/X_pop popup and a
GetAsyncKeyState(VK_CONTROL) poll respectively - purpose inferred only from this one call site, the
executable does not name either.
*/
// Reentrancy guard for the debug block below: set while it runs, so a callee
// that re-enters base_at() (e.g. via rebuild_base_bits or Console::focus)
// while it is busy skips straight to returning -1.
// RECOVERY_OBJECT: v1 0x0090EA38 ?BaseAtErrorGuard@@3HA
int BaseAtErrorGuard;  // 0x0090EA38
// RECOVERY_OBJECT: v2 0x0093A94C 0x0093A950 ?BaseAtDebugPopupFlag@@3HA 0x004E3B12
int BaseAtDebugPopupFlag;  // 0x0093A94C
// Shared popup/action result lives in console.cpp as ConsoleGroupAction.

int __cdecl base_at(int x, int y) {
    if (!on_map(x, y) || !(bit_at(x, y) & BIT_BASE_IN_TILE)) {
        return -1;
    }
    Base *base = Bases;
    for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; base_id++, base++) {
        if (base->x == x && base->y == y) {
            return base_id;
        }
    }
    if (!BaseAtErrorGuard) {
        // This base_at diagnostic passes its own format literal to log_say, with x and y as separate values.
// RECOVERY_LITERAL: v1 0x00689390 0x006893A8 $SG62160 0x004E3A50
        log_say("Base Bits Error  (x, y)", x, y, 0);
    }
    if (!(GameDataStateStorage.fields_.game_state_ & STATE_DEBUG_MODE)) {
        rebuild_base_bits();
        return -1;
    }
    if (BaseAtErrorGuard) {
        return -1;
    }
    BaseAtErrorGuard = 1;
    if (BaseAtDebugPopupFlag) {
        parse_num(0, x);
        parse_num(1, y);
        // This base_at popup key is a separate literal argument from the diagnostic format string above.
// RECOVERY_LITERAL: v1 0x006893A8 0x006893B0 $SG62164 0x004E3A50
        X_pop("BASEBIT", NULL);
    }
    BaseAtErrorGuard = 0;
    if (!ConsoleGroupAction) {
        GetAsyncKeyState(VK_CONTROL);
    }
    rebuild_base_bits();
    ConsoleGlobal.focus(x, y, ConsoleGlobal.faction_id_);
    return -1;
}



/*
Purpose: Find the base id closest to the specified coordinates.
// ORIGINAL: 0x004E3B80 ?base_find@@YAHHH@Z 0x004E3B80-0x004E3C5F BYTE_EXACT
// size      223 bytes
// prototype int (__cdecl ?base_find@@YAHHH@Z)(int xCoord, int yCoord)
// callers   23   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00644F3A
// TRIED: vector_dist(x, y, Bases[i].x, Bases[i].y) - the image never calls
//            vector_dist, only abs() x4 (see del_site 0x00579E70 for the same
//            expansion); open-coding it moved the mismatch from the whole
//            function to a prologue scheduling difference (/O2 /Oi- gets the
//            first 9 instructions and the push/pop shape byte-exact) and one
//            residual +4 byte size difference in the loop body, not chased
//            further - del_site's own comment records the same plateau.
//            Indexing Bases[i] directly (not a walking Base* - lever 7)
//            scored higher here; the two functions disagree on that lever.
Return Value: Base id or -1 if not found
Status: Complete
*/
int __cdecl base_find(int x, int y) {
    int proximity = 9999;
    int base_id = -1;
    for (int i = 0; i < GameDataStateStorage.fields_.base_current_count_; i++) {
        // Inline vector_dist copy: the image calls abs() four times.
        int dist = vector_dist(x, y, Bases[i].x, Bases[i].y);
        if (dist <= proximity) {
            proximity = dist;
            base_id = i;
        }
    }
    if (base_id >= 0) {
        BaseFindDist = proximity;
    }
    return base_id;
}

/*
Purpose: Find the base id closest to the specified coordinates owned by faction.
// ORIGINAL: 0x004E3C60 ?base_find@@YAHHHH@Z 0x004E3C60-0x004E3D4B BYTE_EXACT
// size      235 bytes
// prototype int (__cdecl ?base_find@@YAHHHH@Z)(int xCoord, int yCoord, int factionID)
// callers   27   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00644F3A
// LEVER: open-coded the vector_dist(x, y, Bases[i].x, Bases[i].y) call as the
//        x_dist()+abs()x4 expansion (same shape as the sibling base_find
//        overloads) - the image never calls vector_dist, only abs() x4. Best
//        similarity across flag sets moved 0.551 -> 0.920 (/c /O2 /Oi- /Gy
//        /GR- /Oy- /GX, 5/84 agreeing), though the tool's auto-picked flag
//        set only shows 5/84 vs the prior 4/84 - the similarity gain is the
//        real signal.
Return Value: Base id or -1 if not found
Status: Complete
*/
int __cdecl base_find(int x, int y, int faction_id) {
    int proximity = 9999;
    int base_id = -1;
    for (int i = 0; i < GameDataStateStorage.fields_.base_current_count_; i++) {
        if (Bases[i].faction_id_current == faction_id) {
            int dist = vector_dist(x, y, Bases[i].x, Bases[i].y);
            if (dist <= proximity) {
                proximity = dist;
                base_id = i;
            }
        }
    }
    if (base_id >= 0) {
        BaseFindDist = proximity;
    }
    return base_id;
}

/*
Purpose: Find the base id closest to the specified coordinates meeting various conditional checks.
// ORIGINAL: 0x004E3D50 ?base_find@@YAHHHHHHH@Z 0x004E3D50-0x004E3EEB BYTE_EXACT
// TRIED: region_at(base->x, base->y) - the image widens base->y (int16_t)
//   to int with `movsx` BEFORE the `sar edx, 1` inside map_loc's `x >> 1`;
//   this tree's compiled body does the shift in 16-bit (`sar dx, 1`) and
//   widens only afterward. Forcing `region_at((int)base->x, (int)base->y)`
//   made no difference at all to the emitted bytes (same divergence at the
//   same instruction, every flag set) - this is the same register-
//   allocation plateau already documented across region_at/alt_at/is_ocean
//   and their neighbours (see AGENT_BRIEF's "PICKING A BATCH" note), not
//   something reachable from this call site's source shape.
// size      411 bytes
// prototype int (__cdecl ?base_find@@YAHHHHHHH@Z)(int xCoord, int yCoord, int factionID, int region, int factionID2, int factionID3)
// callers   14   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00644F3A
// LEVER: same vector_dist -> abs()x4 open-coding as base_find(x,y) 0x004E3B80.
//        A `Base *base = Bases; ...base++` walk (lever 7) reused across the
//        whole loop body (matching ebx-relative offsets in the disasm) ties
//        with plain `Bases[i]` indexing here - no measured difference either
//        way, kept for the offset match to the disassembly's ebx addressing.
//        /O2 /Oi- gets the first 11 instructions and the push order
//        byte-exact; the rest is a 1-byte body-size difference cascading
//        through every later jump target, not chased further given size.
Return Value: Base id or -1 if not found
Status: Complete
*/
int __cdecl base_find(int x, int y, int faction_id, int region, int faction_id_2,
                      int faction_id_3) {
    int proximity = 9999;
    int base_id = -1;
    BaseFindDist = 9999; // difference from other two functions where this is reset at start
    Base *base = Bases;
    for (int i = 0; i < GameDataStateStorage.fields_.base_current_count_; i++, base++) {
        if (region < 0 || region_at(base->x, base->y) == (uint32_t)region) {
            if (faction_id >= 0) {
                if (base->faction_id_current != faction_id) {
                    if (faction_id_2 == -2) {
                        if (!(PlayersData[faction_id].diplo_treaties[base->faction_id_current] & DTREATY_PACT)) {
                            continue;
                        }
                    } else if (faction_id_2 < 0 || faction_id_2 != base->faction_id_current) {
                        continue;
                    }
                }
            } else if (faction_id_2 >= 0 && base->faction_id_current == faction_id_2) {
                continue;
            }
            if (faction_id_3 < 0 || base->faction_id_current == faction_id_3
                || ((1 << faction_id_3) & base->visibility)) {
                // Inlined vector_dist(x, y, base->x, base->y): the image calls
                // abs() four times, never vector_dist itself.
                int dist = vector_dist(x, y, base->x, base->y);
                if (dist <= proximity) {
                    proximity = dist;
                    base_id = i;
                }
            }
        }
    }
    if (base_id >= 0) {
        BaseFindDist = proximity;
    }
    return base_id;
}

/*
Purpose: Check who owns a tile. Optional parameter to get closest base.
// ORIGINAL: 0x004E3EF0 ?whose_territory@@YAHHHHPAHH@Z 0x004E3EF0-0x004E3F9D BYTE_EXACT
// TRIED: hoisting `Map *tile = map_loc(x, y)` before the territory read
//            (compiler already CSEs it, identical bytes); flattening the
//            nested owner==faction_id check into an early-return guard
//            clause. Both plateau at 0.944 sim / 34 of 74 instructions;
//            divergence starts in the prologue (image keeps x in edi, this
//            tree picks ebx) - a pure register-allocation difference.
// size      173 bytes
// prototype int (__cdecl ?whose_territory@@YAHHHHPAHH@Z)(int factionID, int xCoord, int yCoord, int* baseID, int ignoreComm)
// callers   40   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004E3D50
Return Value: Faction id of the territory's owner or -1 if no owner or unknown faction
Status: Complete
*/
int __cdecl whose_territory(int faction_id, int x, int y, int *base_id, 
                            BOOL ignore_comm) {
    int owner = (int)map_loc(x, y)->territory;
    if (owner <= 0) {
        return -1; // no owner
    }
    if (owner == faction_id) {
        return faction_id;
    }
    if (!ignore_comm && !(GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW)) {
        if ((PlayersData[faction_id].diplo_treaties[owner]
             & (DTREATY_COMMLINK | DTREATY_UNK_8000000))
            != (DTREATY_COMMLINK | DTREATY_UNK_8000000)) {
            return -1; // owner unknown to faction
        }
        if (base_id) {
            *base_id = base_find(x, y, -1, region_at(x, y), -1, -1);
        }
    } else if (base_id) {
        *base_id = base_find(x, y, -1, region_at(x, y), -1, -1);
    }
    return owner;
}

/*
Purpose: Find the best specialist available to the current base with more weight placed on psych.
// ORIGINAL: 0x004E4020 ?best_specialist@@YAHXZ 0x004E4020-0x004E4090 BYTE_EXACT
// LEVER: two fixes. (1) has_tech(...) was inlining its whole preq_tech walk
//   here where the image keeps a real `call 0x5b9f20` - added
//   technology.h/.cpp's `has_tech_call` non-inline forwarder (same idiom as
//   general.h's bitmask_call) and called that instead, which alone moved
//   0/44 -> ~40/44 agreeing. (2) the remaining gap was BaseCurrent()'s two
//   textual reads: the image reads the global ONCE before the loop (priming
//   iteration 1) then reloads it UNCONDITIONALLY right after the has_tech
//   call - even on a false return - which feeds both the current
//   iteration's population_size check AND the next iteration's top-of-loop
//   read, eliminating what would otherwise be a second load. Hoisting
//   `Base *base_current = BaseCurrent();` before the loop and reassigning it
//   right after `has_tech_call` (not inside the `if`, since the image
//   reloads even when the call returns false) reproduces that exactly.
//   44/44 BYTE_EXACT. Direct BaseCurrent reads also measure exact and let
//   base_doctors' inlined copies share the post-has_tech reload.
// symbol    ?best_specialist@@YAIXZ
// size      112 bytes
// prototype
// callers   9   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005B9F20
Return Value: Best citizen id (always going to be 1, 4, or 6 based on default weights)
Status: Complete
*/
// inline: base_doctors (0x004F5F30) expands this body twice; the standalone stays emitted.
inline uint32_t  __cdecl best_specialist() {
    int current_bonus = -999;
    uint32_t citizen_id = 0;
    for (int i = 0; i < MaxSpecialistNum; i++) {
        if (has_tech(Citizen[i].preq_tech, BaseCurrent->faction_id_current)) {
            uint32_t bonus = Citizen[i].bonus[1] * 3;
            if (BaseCurrent->population_size >= (int)Rules.min_base_size_specialists) {
                bonus += Citizen[i].bonus[0] + Citizen[i].bonus[2];
            }
            if ((int)bonus > current_bonus) {
                current_bonus = bonus;
                citizen_id = i;
            }
        }
    }
    return citizen_id;
}

/*
Purpose: Generate a base name. Added some minor randomization for sea bases and ensuring offset gets
         increased for default base name. TODO: Eventually improve base name shuffling.
// ORIGINAL: 0x004E4090 ?name_base@@YAXHPADHH@Z 0x004E4090-0x004E4350 BYTE_EXACT
// size      704 bytes
// prototype void (__cdecl ?name_base@@YAXHPADHH@Z)(int factionID, LPSTR nameOut, BOOL isFinal, BOOL isSeaBase)
// callers   3   call targets   9
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005FD530 0x005FD550 0x005FD570 0x005FD670 0x006453E0 0x00645470 0x00645E90 0x0064FC88 0x0064FDF0
// The sea path has no reseed walk and closes the file only on success; the
// land offset is committed only after a name is copied from the file.
Return Value: n/a
Status: Complete
*/
void __cdecl name_base(int faction_id, LPSTR name_out, BOOL is_final, BOOL is_sea_base) {
    if (is_sea_base && !text_open(Players[faction_id].filename, "WATERBASES")) {
        int offset_sea = PlayersData[faction_id].base_sea_name_offset + 1;
        int count;
        for (count = 0; count < offset_sea; count++) {
            text_get();
            if (!strlen(TextBufferGetPtr) || !_strnicmp(TextBufferGetPtr, "#END", 4)) {
                break;
            }
        }
        if (count == offset_sea) {
            // water base name available
            strncpy(name_out, text_item(), 24);
            name_out[23] = 0;
            if (is_final) {
                PlayersData[faction_id].base_sea_name_offset = offset_sea;
            }
            text_close();
            return;
        }
    }
    // Land base names or generic; the default stays if names are exhausted.
    int offset = PlayersData[faction_id].base_name_offset;
    name_out[0] = 0;
    GenderDefault = Players[faction_id].noun_gender;
    PluralityDefault = Players[faction_id].is_noun_plural;
    strcat(name_out, Players[faction_id].noun_faction);
    strcat(name_out, BufferJoinSpace);
    char digits[80];
    _itoa(++offset, digits, 10);
    strcat(name_out, digits);
    if (text_open(Players[faction_id].filename, "BASES")) {
        return;
    }
    int total = 0;
    while (true) {
        text_get();
        if (!strlen(TextBufferGetPtr) || !_strnicmp(TextBufferGetPtr, "#END", 4)) {
            break;
        }
        total++;
    }
    int line = offset;
    if (offset > 1 && offset <= total) {
        int seed = ((faction_id + MapRandSeed) & 0xFE) | 1;
        int loop = 1;
        do {
            if (seed & 1) {
                seed ^= 0x170;
            }
            seed >>= 1;
        } while (seed >= total || ++loop != offset);
        line = seed + 1;
    }
    if (text_open(Players[faction_id].filename, "BASES")) {
        return;
    }
    int count;
    for (count = 0; count < line; count++) {
        text_get();
        if (!strlen(TextBufferGetPtr) || !_strnicmp(TextBufferGetPtr, "#END", 4)) {
            break;
        }
    }
    if (count != line) {
        if (text_open("BASENAME", "GENERIC")) {
            return;
        }
        for (; count < line; count++) {
            text_get();
            if (!strlen(TextBufferGetPtr) || !_strnicmp(TextBufferGetPtr, "#END", 4)) {
                return;
            }
        }
    }
    strncpy(name_out, text_item(), 24);
    name_out[23] = 0;
    if (is_final) {
        PlayersData[faction_id].base_name_offset = offset;
    }
    text_close();
}

/*
Purpose: TBD
// ORIGINAL: 0x004E4350 ?base_mark@@YAXH@Z 0x004E4350-0x004E442D BYTE_EXACT
// size      221 bytes
// prototype void (__cdecl ?base_mark@@YAXH@Z)(int baseID)
// callers   2   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00579E70 0x00591B50 0x00591C10 0x00591D60
// LEVER: two changes. (1) the BIT_BASE_RADIUS site directly calls the mapped
//        out-of-line `bit_set` owner at 0x591D60, matching the image's real
//        call here without a synthetic forwarding layer. (2) the radius loop bound is
//        the literal `49` (image `cmp ebx, 0xc4`, a byte-offset stride)
//        instead of `RadiusRange[3]`, which the compiler had to reload from
//        the global every iteration and which blocked the walking-byte-
//        offset strength reduction. Best similarity 0.662 -> 0.805; agreeing
//        count unchanged at 2/81 but compiled size moved from 96 to 83
//        instructions against the image's 81.
Return Value: n/a
Status: Complete - testing
*/
void __cdecl base_mark(int base_id) {
    int x_radius = 0;
    int y_radius = 0;
    for (int i = 0; i < 49; i++) {
        x_radius = wrap_map_x(Bases[base_id].x + RadiusOffsetX[i]);
        y_radius = Bases[base_id].y + RadiusOffsetY[i];
        if (on_map(x_radius, y_radius)) {
            if (i < 21) {
                bit_set(x_radius, y_radius, BIT_BASE_RADIUS, true);
            } else {
                site_set(x_radius, y_radius, 0);
            }
            if (i < 21) {
                using_set(x_radius, y_radius, Bases[base_id].faction_id_current);
            }
        }
    }
    for (int f = 0; f < MaxPlayerNum; f++) {
        // TODO: verify if using last radius coordinates is best logic, seems odd
        // It would seem using the Base coordinates would be better suited versus radius
        del_site(f, AI_GOAL_COLONIZE, x_radius, y_radius, 3);
    }
}

// HISTORICAL: this duplicated has_fac_built predicate was moved to base.h
// as `has_fac_built_call` so breed_mod could reach the adapter. The current
// two-argument has_fac_built directly calls the shared out-of-line bitmask;
// the distinct one-argument guarded overload remains below.

// RECOVERY_ASSOCIATED: v1 switch_targets 0x004E4430 0x004E46C4 0x004E46F4 0x004E45DF
/*
Purpose: Calculate the cost factor for the specified faction and resource type. Optional base param.
// ORIGINAL: 0x004E4430 ?cost_factor@@YAHHHH@Z 0x004E4430-0x004E46C2
// TRIED: call count already agrees (call_diff: 0 disagree, both call
// bitmask and 0x539c00 through has_fac_built_call/great_satan), so the
// 9/251 MISMATCH is not a call-shape defect. The image's `is_human(faction_id)`
// ternary loads the bitmask table (`mov eax,[0x9a64e8]`) BEFORE the `push
// esi`/`push edi` register saves; this tree's /O2 schedules that load after
// them - pure instruction scheduling in the preamble. The larger gap is
// past that: image is 251 instructions against this tree's 159 for the
// same body, which the RSC_MINERALS `switch` on
// `soc_effect_pending.industry` (cases -7..4) likely accounts for most of -
// not chased further; a full match would need the same compare-chain-vs-
// jump-table investigation `base_making`'s note describes for its own
// `switch`, at a size (658 image bytes) outside this pass's budget.
// size      658 bytes
// prototype int (__cdecl ?cost_factor@@YAHHHH@Z)(uint32_t factionID, uint32_t rscType, int baseID)
// callers   24   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0050BA00 0x00539C00
Return Value: Cost factor
Status: Complete
*/
// Bound: six difficulty dwords; the next mapped string starts at 0x00689390.
// Writable .data in the image (0x00689378), so not const.
// RECOVERY_OBJECT: v4 0x00689378 0x00689390 _?diff_cost_base@?1??cost_factor@@YAHHHH@Z@4QBHB 0x004E448E target _diff_cost_base
static int diff_cost_base[] = { 13, 12, 11, 10, 8, 7 };
int __cdecl cost_factor(int faction_id, int rsc_type, int base_id) {
    int factor = is_human(faction_id) ? 10 : diff_cost_base[GameDataStateStorage.fields_.diff_level_current_]
        - (great_satan(GameDataStateStorage.fields_.faction_rankings_[7], false) != 0)
        - (!IsMultiplayerNet && is_human(GameDataStateStorage.fields_.faction_rankings_[7]));
    int cost_multiplier = !rsc_type ? Rules.nutrient_cost_multi : Rules.mineral_cost_multi;
    if (cost_multiplier != 10) {
        factor = (cost_multiplier * factor) / 10;
    }
    if (MapSizePlanet == 1) {
        factor = (factor * 9) / 10;
    } else if (!MapSizePlanet) {
        factor = (factor * 8) / 10;
    }
    if (!rsc_type) {
        int growth_factor = PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::GROWTH];
        if (base_id >= 0) {
            if (has_fac(FAC_CHILDREN_CRECHE, base_id, 0)) {
                growth_factor += 2;
            }
            if (Bases[base_id].state & BSTATE_GOLDEN_AGE_ACTIVE) {
                growth_factor += 2;
            }
        }
        factor = (factor * (10 - range(growth_factor, -2, 5)) + 9) / 10;
    } else if (rsc_type == RSC_MINERALS) {
        switch (PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::INDUSTRY]) {
          case -7:
          case -6:
          case -5:
          case -4:
          case -3:
            factor = (factor * 13 + 9) / 10;
            break;
          case -2:
            factor = (factor * 6 + 4) / 5;
            break;
          case -1:
            factor = (factor * 11 + 9) / 10;
            break;
          case 0:
            break;
          case 1:
            factor = (factor * 9 + 9) / 10;
            break;
          case 2:
            factor = (factor * 4 + 4) / 5;
            break;
          case 3:
            factor = (factor * 7 + 9) / 10;
            break;
          case 4:
            factor = (factor * 3 + 4) / 5;
            break;
          default:
            factor = (factor + 1) / 2;
            break;
        }
    }
    return factor;
}

/*
Purpose: Determine if the specified base has any restrictions around production item retooling.
// ORIGINAL: 0x004E4700 ?base_making@@YAHHH@Z 0x004E4700-0x004E4802 BYTE_EXACT
// size      258 bytes
// prototype int (__cdecl ?base_making@@YAHHH@Z)(int productionID, int baseID)
// callers   4   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0050BA00
// LEVER: `int retool` rather than `uint32_t retool` (lever 4) so `retool >=
//        1` compiles a signed compare like the image's, kept WITHOUT
//        touching the guard. Best similarity 0.546 -> 0.555.
// LEVER: WRONG CALLEE - the image's two calls are both to bitmask (0x50ba00,
//        push 0xa then a dynamic id), never to facility_offset or has_tech.
//        The RFLAG_FREEPROTO/facility_offset("Skunkworks")/has_tech disjunct
//        was this tree's own invention (facility_offset is a project helper,
//        "Original Offset: n/a") - the image's guard is plainly
//        `has_fac_built(FAC_SKUNKWORKS, base_id) && retool >= 1`, first push
//        is the literal 0xa (FAC_SKUNKWORKS). Dropping the disjunct AND
//        switching both has_fac_built() calls in this function to
//        has_fac_built_call() (general.h bitmask_call, forcing the real
//        E8 the image emits instead of the folded shift/and) moved best
//        similarity 0.555 -> 0.652, 30/120 agreeing. A prior pass dropped
//        the disjunct alone (without forcing the real bitmask call) and saw
//        0.549, WORSE - that was the wrong half of this fix, not proof the
//        disjunct belongs.
// TRIED: remaining divergence is the switch(retool) codegen - image
//            compiles it as a compare chain, this tree's /O2 default emits a
//            jump table (`jmp dword ptr [ebx*4]`), plus an omit-frame-pointer
//            stack layout (esp-relative) vs the image's ebp frame. Neither
//            responded to the flag sweep (--all-flags best is this same
//            0.652 set); left as source-shaped register/codegen noise.
Return Value: Fixed value (-1, 0, 1, 2, 3, -70) or productionID
Status: Complete
*/
int __cdecl base_making(int production_id, int base_id) {
    int retool = Rules.retool_strictness;
    if (has_fac(FAC_SKUNKWORKS, base_id, 0) // has Skunkworks
        && retool >= 1) { // don't override if retool strictness is already set to always free (0)
        retool = 1; // Skunkworks or FREEPROTO + prerequisite tech > 'Free in Category'
    }
    if (production_id < 0) { // facility or SP to build
        int queue_id = Bases[base_id].queue_production_id[0]; // current production item
        if (queue_id < 0) { // non-Veh
            queue_id = -queue_id;
            if (queue_id < FacilityRepStart && has_fac(queue_id, base_id, 0)) {
                return -1; // facility completed outside normal process, no retool penalty to change
            }
        }
    }
    if (retool == 0) return 0;
    if (retool == 2) {
        if (production_id <= -FacilitySPStart) {
            return -FacilitySPStart;
        }
        return production_id;
    }
    if (retool > 1) return production_id;
    if (production_id >= 0) return 0;
    // SP (1), repeatable facility (2), non-repeatable facility (3)
    if (production_id <= -FacilitySPStart) {
        return 1;
    }
    return (production_id > -FacilityRepStart) + 2;
}

/*
Purpose: Calculate the mineral loss if production were to changed at the specified base.
// ORIGINAL: 0x004E4810 ?base_lose_minerals@@YAHHH@Z 0x004E4810-0x004E48B0 BYTE_EXACT
// size      160 bytes
// prototype int (__cdecl ?base_lose_minerals@@YAHHH@Z)(int baseID, int)
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004E4700
// LEVER: signedness on the /100 divide. `retool_pct_pen_prod_chg` and
//        `retool_exemption` are declared uint32_t in alpha.h, so the return
//        expression was an unsigned division; casting both operands to
//        `(int)` restores the image's signed magic-multiply/sar sequence.
//        41/60 agreeing, up from 10/60.
Return Value: Minerals that would be lost or 0 if not applicable.
Status: Complete
*/
int __cdecl base_lose_minerals(int base_id, int UNUSED(production_id)) {
    int min_accum;
    if (Rules.retool_pct_pen_prod_chg && is_human(Bases[base_id].faction_id_current)) {
        // The original classifies +0x44 before reading and classifying +0x50.
        // Separate statements preserve that order; != operands do not.
        const int previous = base_making(Bases[base_id].production_id_last, base_id);
        const int queued = base_making(Bases[base_id].queue_production_id[0], base_id);
        if (queued != previous
            && (min_accum = Bases[base_id].minerals_accumulated_2,
                min_accum > (int)Rules.retool_exemption)) {
            return min_accum - (100 - (int)Rules.retool_pct_pen_prod_chg)
                * (min_accum - (int)Rules.retool_exemption) / 100 - (int)Rules.retool_exemption;
        }
    }
    return 0;
}

/*
Purpose: Set or unset the specified base's facility.
// ORIGINAL: 0x004E48B0 ?set_fac@@YAXHHH@Z 0x004E48B0-0x004E48FA BYTE_EXACT
// LEVER: bitmask_call, and the offset out-param is THIS parameter's slot.
//   The folded `MEASURED inline bitmask` turned the call into a shift/and
//   pair in place (0/32); general.h's bitmask_call restores the image's
//   `call 0x50ba00`. And the image passes &facility_id - not a separate
//   local - as the offset pointer, which puts offset in the dead parameter
//   slot at [ebp+8] and mask at [ebp-4]. TRIED: `int offset; int mask;`
//   with `&offset`, the shape has_fac_built_call and sea_coasts use - 3/32,
//   because offset lands at [ebp-4] and pushes mask out of the frame.
// size      74 bytes
// prototype void (__cdecl ?set_fac@@YAXHHH@Z)(int facilityID, int baseID, BOOL set)
// callers   10   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0050BA00
Return Value: n/a
Status: Complete
*/
void __cdecl set_fac(int facility_id, int base_id, BOOL set) {
    int mask;
    // An ordinary offset local: VC6 places it in the dead facility_id slot
    // here, and base_maint's inlined copy needs it separate.
    int offset;
    bitmask(facility_id, &offset, &mask);
    if (set) {
        Bases[base_id].facilities_built[offset] |= mask;
    } else {
        Bases[base_id].facilities_built[offset] &= ~mask;
    }
}

/*
Purpose: Check whether the facility audio blurb announcement has played for the faction.
// ORIGINAL: 0x004E4900 has_fac_announced 0x004E4900-0x004E4952 BYTE_EXACT
// LEVER: signedness. `facility_id` is uint32_t, so `facility_id > FacilitySPStart` forced an unsigned `jbe`; casting to `(int)facility_id` before the compare, without changing the parameter's mangled type, restores the image's signed `jle`. BYTE_EXACT.
// LEVER: bitmask_call with &facility_id as the offset out-param, exactly as set_fac records - the image reuses the dead parameter slot at [ebp+0xc] for the offset and keeps mask at [ebp-4]. The `(int *)` cast is the uint32_t the mangled name (?has_fac_announced@@YAHII@Z) fixes.
// symbol    ?has_fac_announced@@YAHII@Z
// size      82 bytes
// prototype int (__cdecl has_fac_announced)(int factionID, int facilityID)
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0050BA00
Return Value: Has the facility audio blurb played? true/false
Status: Complete
*/
BOOL __cdecl has_fac_announced(uint32_t faction_id, uint32_t facility_id) {
    if ((int)facility_id > FacilitySPStart) {
        return true;
    }
    int mask;
    // Same parameter-slot reuse the image shows in set_fac below: bitmask's
    // offset out-param is this function's own facility_id slot, dead after
    // the call. The cast is the uint32_t parameter the mangled name fixes
    // (?has_fac_announced@@YAHII@Z), not a pun of this.
    bitmask(facility_id, (int *)&facility_id, &mask);
    return (PlayersData[faction_id].facility_announced[facility_id] & mask) != 0;
}

/*
Purpose: Set or unset whether a particular facility audio blurb has played for specified faction.
// ORIGINAL: 0x004E4960 set_fac_announced 0x004E4960-0x004E49CA BYTE_EXACT
// LEVER: bitmask_call with &facility_id as the offset out-param - set_fac's
//   note above covers both halves of it. 0/42 -> 42/42.
// symbol    ?set_fac_announced@@YAXIIH@Z
// size      106 bytes
// prototype int (__cdecl set_fac_announced)(int factionID, int facilityID, int set)
// callers   0   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0050BA00
Return Value: n/a
Status: Complete
*/
void __cdecl set_fac_announced(uint32_t faction_id, uint32_t facility_id, BOOL set) {
    int mask;
    // set_fac's parameter-slot reuse again - the image's offset out-param is
    // this facility_id slot itself. The cast is the uint32_t the mangled
    // name (?set_fac_announced@@YAXIIH@Z) fixes, not a pun of this.
    bitmask(facility_id, (int *)&facility_id, &mask);
    if (set) {
        PlayersData[faction_id].facility_announced[facility_id] |= mask;
    } else {
        PlayersData[faction_id].facility_announced[facility_id] &= ~mask;
    }
}

/*
Purpose: Check if coordinates are considered near or on coast. Radius (excludes actual coordinates)
         can either be all the squares directly around the coordinates or same as Base '+' radius.
// ORIGINAL: 0x004E49D0 ?is_coast@@YAHHHH@Z 0x004E49D0-0x004E4A91 BYTE_EXACT
// TRIED: 9/76, 0.84 similar plateau across every flag set. The image
//            uses TWO induction variables for the loop (an unscaled count
//            in [ebp-4] and a byte offset in edi, incremented in lockstep)
//            and defers loading `MapIsFlat` to AFTER the "loop won't run"
//            guard; this tree's -O2 always collapses to one induction
//            variable and hoists the `MapIsFlat` read before that guard,
//            regardless of whether the source calls `xrange`/`on_map` or
//            inlines their logic by hand (tried both, identical score).
//            `is_ocean`'s own comparison also narrows to a byte `cmp/jb`
//            here where the image keeps the 32-bit `cmp edx,0x60`/`jl` -
//            same root cause as is_ocean's own standalone plateau.
// size      193 bytes
// prototype int (__cdecl ?is_coast@@YAHHHH@Z)(int xCoord, int yCoord, int isBaseRadius)
// callers   10   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: Neighbor index 1..8 or 1..20, or 0 if no qualifying ocean neighbor is found.
Status: Complete
*/
// Current restored contract: return the qualifying neighbor's index (1..8 or 1..20), or 0.
int __cdecl is_coast(int x, int y, BOOL is_base_radius) {
    int i = 1;
    int radius = is_base_radius ? 21 : 9;
    if (i >= radius) {
        return 0;
    }
    for (; i < radius; i++) {
        int x_radius = wrap_map_x(x + RadiusOffsetX[i]);
        int y_radius = y + RadiusOffsetY[i];
        if (on_map(x_radius, y_radius) && is_ocean(x_radius, y_radius)) {
            return i;
        }
    }
    return 0;
}

/*
Purpose: Determine what unit the specified base should start building 1st then add it to the queue.
// ORIGINAL: 0x004E4AA0 ?base_first@@YAXH@Z 0x004E4AA0-0x004E4B7B BYTE_EXACT
// size      219 bytes
// prototype void (__cdecl ?base_first@@YAXH@Z)(int baseID)
// callers   2   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005BA910
// TRIED: pre-storing `Bases[base_id].queue_production_id[0] = proto_id;`
//            before the loop (to match the image's early store at
//            0x004E4AA0 instr 7) and reading VehPrototypes[i].plan directly
//            at each compare instead of hoisting it into a local both
//            measured WORSE - best flag set /c /O2 /Gy /GR- /Oy- /GX dropped
//            from 0.912 to 0.883 similar (agreeing stays 4/78 either way).
Return Value: n/a
Status: Complete
*/
void __cdecl base_first(int base_id) {
    int priority = -1;
    Bases[base_id].queue_production_id[0] = BSC_SCOUT_PATROL;
    uint32_t proto_id = BSC_SCOUT_PATROL;
    for (int i = 0; i < MaxVehProtoNum; i++) {
        if (veh_avail(i, Bases[base_id].faction_id_current, base_id)) {
            if (i < MaxVehProtoFactionNum || VehPrototypes[i].flags & PROTO_TYPED_COMPLETE) {
                int compare = get_proto_defense_rating(i) * 32;
                if (VehPrototypes[i].plan == PLAN_DEFENSIVE) {
                    compare *= 4;
                }
                if (VehPrototypes[i].plan == PLAN_COMBAT) {
                    compare *= 3;
                }
                if (VehPrototypes[i].plan == PLAN_RECONNAISANCE) {
                    compare *= 2;
                }
                if (get_proto_triad(i) != TRIAD_LAND) {
                    compare /= 4;
                }
                compare -= VehPrototypes[i].cost;
                if (compare >= priority) {
                    priority = compare;
                    proto_id = i;
                }
            }
        }
    }
    Bases[base_id].queue_production_id[0] = proto_id;
}

// ORIGINAL: 0x004E4B80 ?base_init@@YAHHHH@Z 0x004E4B80-0x004E5250 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/004e4b80.cpp: canonical identity, original spans and retained seed history", "source:src/base.h: real Base storage and lifecycle contracts", "source:recovery/tasks/base-lifecycle-owner-closure.json: bounded coordinator authorization"], "reason": "The original base creation operation owns initialization of Bases records and faction base counts; source expresses real Base/PlayerData fields, facility bitmaps, technology and named UI/replay dependencies."}
// TRIED: `>= 0x200` guard (matches original cmp/jl) over `> 0x1ff` moved divergence #4->#12; raw byte-offset arithmetic on the fixed 0x97dXXX/0x96XXXX globals (record stride 0x134, per-faction stride 0x20cc) rather than the Base struct - field order in src/base.h is only size-checked, not per-field, and several 0x97d0cc-family writes index past their nominal field width (a genuine original OOB-write reproduced faithfully). Not chased to byte-exact given size (1744 bytes / ~330 asm lines).
int __cdecl base_init(int faction_id, int x, int y) {
    if (GameDataStateStorage.fields_.base_current_count_ >= 512) {
        return -1;
    }
    int region = map_loc(x, y)->region;
    bit_set(x, y, BIT_BASE_IN_TILE, TRUE);
    owner_set(x, y, faction_id);
    synch_bit(x, y, faction_id);
    int base_id = GameDataStateStorage.fields_.base_current_count_++;
    PlayersData[faction_id].current_num_bases++;
    PlayersData[faction_id].region_total_bases[region]++;
    replay_base(0, x, y, faction_id);
    Bases[base_id].x = static_cast<int16_t>(x);
    Bases[base_id].y = static_cast<int16_t>(y);
    Bases[base_id].faction_id_current = Bases[base_id].faction_id_former
        = static_cast<uint8_t>(faction_id);
    Bases[base_id].nerve_staple_turns_left = 0;
    Bases[base_id].assimilation_turns_left = 0;
    Bases[base_id].unk_1 = 0;
    Bases[base_id].population_size
        = has_project(SP_PLANETARY_TRANS_SYS, faction_id) ? 3 : 1;
    Bases[base_id].visibility
        = static_cast<uint8_t>(map_loc(x, y)->visibility | (1 << faction_id));
    Bases[base_id].name_string[0] = 0;
    name_base(faction_id, Bases[base_id].name_string, TRUE, is_ocean(x, y));
    Bases[base_id].state = 0;
    Bases[base_id].event = 0;
    Bases[base_id].governor = PlayersData[faction_id].base_governor_adv;
    Bases[base_id].minerals_accumulated = 0;
    Bases[base_id].nutrients_accumulated = 0;
    Bases[base_id].nerve_staple_count = 0;
    for (int slot = 0; slot < Base::AutoForwardSlotCount; slot++) {
        Bases[base_id].auto_forward_slots[slot] = -1;
    }
    Bases[base_id].queue_size = 0;
    base_first(base_id);
    if (!GameDataStateStorage.fields_.turn_current_num_) {
        int production = has_tech(VehPrototypes[1].preq_tech, faction_id)
            ? 1 : (PlayersData[faction_id].diff_level <= 1 ? 0 : 2);
        Bases[base_id].queue_production_id[0] = production;
        Bases[base_id].minerals_accumulated
            = cost_factor(faction_id, 1, -1) * VehPrototypes[production].cost / 2;
    } else {
        Bases[base_id].minerals_accumulated
            = PlayersData[faction_id].soc_effect_active.values_[SocialEffect::SUPPORT] < -1
            ? 0 : Rules.retool_exemption;
    }
    Bases[base_id].resource_sqr_radius = 0;
    Bases[base_id].specialist_total = 0;
    Bases[base_id].specialist_unk_1 = 0;
    Bases[base_id].eco_damage = 0;
    Bases[base_id].unk_x = Bases[base_id].unk_y = -1;
    // The original clears only nine facility bytes, not the entire recycled
    // Base record. Fields it does not initialize deliberately retain their state.
    for (int fac = 0; fac < 9; fac++) {
        Bases[base_id].facilities_built[fac] = 0;
    }
    // BUG(original): nerve_staple_count is cleared a second time here.
    // WORKAROUND(type): the image clears 0x124..0x133 with one fresh zero
    // register in ascending order (0x004E4E20..0x004E4E3A), the unrolled form
    // of a four-int array clear; base.h models these as four scalar fields.
    for (int k = 0; k < 4; k++) {
        (&Bases[base_id].random_event_turns)[k] = 0;
    }
    if (PlayersData[faction_id].current_num_bases == 1) {
        set_fac(FAC_HEADQUARTERS, base_id, TRUE);
        if (IsMultiplayerNet) {
            Bases[base_id].population_size = 3;
            if (Players[faction_id].rule_talent
                || PlayersData[faction_id].soc_effect_base.values_[SocialEffect::PLANET] > 0) {
                Bases[base_id].population_size = 4;
            }
            if (Players[faction_id].rule_drone || Players[faction_id].rule_population) {
                Bases[base_id].population_size--;
            }
            set_fac(FAC_RECYCLING_TANKS, base_id, TRUE);
            if (GameDataStateStorage.fields_.diff_level_current_ >= 3) {
                set_fac(FAC_RECREATION_COMMONS, base_id, TRUE);
            }
            if (PlayersData[faction_id].soc_effect_base.values_[SocialEffect::ECONOMY] > 0) {
                set_fac(FAC_ENERGY_BANK, base_id, TRUE);
            }
        } else {
            Bases[base_id].population_size = static_cast<int8_t>(
                range((GameDataStateStorage.fields_.turn_current_num_ + 49) / 50, 1, 5));
            if (GameDataStateStorage.fields_.turn_current_num_ >= 75) {
                set_fac(FAC_RECYCLING_TANKS, base_id, TRUE);
            }
            if (GameDataStateStorage.fields_.turn_current_num_ >= 100) {
                set_fac(FAC_RECREATION_COMMONS, base_id, TRUE);
            }
            if (GameDataStateStorage.fields_.turn_current_num_ >= 150) {
                set_fac(FAC_ENERGY_BANK, base_id, TRUE);
            }
        }
    }
    if (is_ocean(x, y)) {
        set_fac(FAC_PRESSURE_DOME, base_id, TRUE);
    }
    for (int i = 0; i < 8; i++) {
        // Nested ?: - the image loads population_size once per true arm.
        Bases[base_id].faction_pop_size_intel[i] = i == faction_id
            ? Bases[base_id].population_size
            : ((Bases[base_id].visibility & (1 << i)) ? Bases[base_id].population_size : 0);
    }
    int j;
    for (j = 0; j < Players[faction_id].faction_bonus_count; j++) {
        if (Players[faction_id].faction_bonus_id[j] == 2) {
            set_fac(range(Players[faction_id].faction_bonus_val1[j], 1, 69), base_id, TRUE);
        }
    }
    for (j = 0; j < Players[faction_id].faction_bonus_count; j++) {
        // BUG(original): the facility prerequisite is looked up with the
        // unclamped bonus value; only the set_fac index is clamped.
        if (Players[faction_id].faction_bonus_id[j] == 12
            && has_tech(Facility[Players[faction_id].faction_bonus_val1[j]].preq_tech,
                faction_id)) {
            set_fac(range(Players[faction_id].faction_bonus_val1[j], 1, 69), base_id, TRUE);
        }
    }
    for (j = 1; j < 70; j++) {
        if (has_tech(Facility[j].free_tech, faction_id)) {
            set_fac(j, base_id, TRUE);
        }
    }
    Bases[base_id].production_id_last = Bases[base_id].queue_production_id[0];
    Bases[base_id].minerals_accumulated_2 = Bases[base_id].minerals_accumulated;
    Bases[base_id].mineral_surplus_final = 0;
    base_mark(base_id);
    reset_territory();
    BaseIDCurrentSelected = base_id;
    BaseCurrent = &Bases[base_id];
    base_compute(1);
    WorldClimateWorldWin.field_239BC_ |= 2;
    return base_id;
}

// base_find(x, y, faction_id) as base_kill inlines it; the out-of-line body
// (below) is kept auto_inline(off) for its other callers.
static inline int base_kill_find(int x, int y, int faction_id) {
    int proximity = 9999;
    int base_id = -1;
    for (int i = 0; i < GameDataStateStorage.fields_.base_current_count_; i++) {
        if (Bases[i].faction_id_current == faction_id) {
            int dist = vector_dist(x, y, Bases[i].x, Bases[i].y);
            if (dist <= proximity) {
                proximity = dist;
                base_id = i;
            }
        }
    }
    if (base_id >= 0) {
        BaseFindDist = proximity;
    }
    return base_id;
}

// ORIGINAL: 0x004E5250 ?base_kill@@YAXH@Z 0x004E5250-0x004E5A58
// RECOVERY_BINDING: v1 {"evidence": ["git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/004e5250.cpp: canonical identity, original spans and retained seed history", "source:src/base.h: real Base storage and lifecycle contracts", "source:recovery/tasks/base-lifecycle-owner-closure.json: bounded coordinator authorization"], "reason": "The original base destruction operation belongs to the Base owner and repairs real faction/project/base/vehicle references and count/selection state rather than forwarding into the original executable."}
// TRIED: full line-by-line transcription landed (MISMATCH #3, prologue push/mov divergence); stack-slot reuse across phases not replicated (used per-phase named locals instead), and the two adjustor-thunk virtual calls (vtable[1] read as delta, then slot0 on the adjusted `this`) route through VCall with a manually computed adjusted receiver rather than the shim alone.
// The image calls the CRT memcpy for the record compaction.
void __cdecl base_kill(int base_id) {
    int veh_id;
    int x = Bases[base_id].x;
    int y = Bases[base_id].y;
    int faction_id = Bases[base_id].faction_id_current;
    if (has_fac(FAC_HEADQUARTERS, base_id, 0)
        && PlayersData[faction_id].corner_market_turn > GameDataStateStorage.fields_.turn_current_num_
        && PlayersData[faction_id].corner_market_active) {
        PlayersData[faction_id].corner_market_turn = 0;
        PlayersData[faction_id].corner_market_active = 0;
        if (faction_id == ConsoleGlobal.faction_id_) {
            popp(ScriptPopupFile, "CORNERFOILED", 0, "econwin_sm.pcx", 0);
        } else {
            parse_says(0, Players[faction_id].adj_name_faction, -1, -1);
            popp(ScriptPopupFile, "CORNERTHEMFOILED", 0, "econwin_sm.pcx", 0);
        }
    }
    if (Bases[base_id].population_size < 0) {
        Bases[base_id].population_size = 0;
    }
    replay_base(2, x, y, faction_id);
    PlayersData[faction_id].current_num_bases--;
    PlayersData[faction_id].region_total_bases[region_at(x, y)]--;
    for (veh_id = GameDataStateStorage.fields_.vehicle_current_count_ - 1; veh_id >= 0; veh_id--) {
        if (veh_id < GameDataStateStorage.fields_.vehicle_current_count_
            && Vehs[veh_id].home_base_id == base_id) {
            if (Vehs[veh_id].faction_id != faction_id) {
                Vehs[veh_id].home_base_id = -1;
            } else {
                // Hide the dying base from the search for a new home.
                Bases[base_id].faction_id_current = 0xFF;
                int home_id = base_kill_find(x, y, faction_id);
                Bases[base_id].faction_id_current = static_cast<uint8_t>(faction_id);
                if (home_id >= 0) {
                    Vehs[veh_id].home_base_id = static_cast<int16_t>(home_id);
                } else {
                    kill(veh_id);
                }
            }
        }
    }
    bit_set(x, y, BIT_BASE_IN_TILE, 0);
    // This overlapping copy is memcpy in the original, not memmove.
    if (base_id < GameDataStateStorage.fields_.base_current_count_ - 1) {
        memcpy(&Bases[base_id], &Bases[base_id + 1],
            (GameDataStateStorage.fields_.base_current_count_ - base_id - 1) * sizeof(Base));
    }
    GameDataStateStorage.fields_.base_current_count_--;
    if (base_id == BaseComputedID) {
        BaseComputedID = -1;
    }
    for (veh_id = 0; veh_id < GameDataStateStorage.fields_.vehicle_current_count_; veh_id++) {
        if (Vehs[veh_id].home_base_id > base_id) {
            Vehs[veh_id].home_base_id--;
        }
    }
    int i;
    for (i = 0; i < GameDataStateStorage.fields_.base_current_count_; i++) {
        for (int j = 0; j < 3; j++) {
            if (Bases[i].auto_forward_slots[j] == base_id) {
                Bases[i].auto_forward_slots[j] = -1;
            } else if (Bases[i].auto_forward_slots[j] > base_id) {
                Bases[i].auto_forward_slots[j]--;
            }
        }
    }
    StringList &messages = InterfaceMessageWin.listBox_.item_list_;
    const int message_id = base_id | 0x40000000;
    if (messages.head_) {
        for (int index = 0; index < messages.entry_count_; ++index) {
            StringStructEntry *entry = messages.current_;
            if (entry->id == message_id) {
                entry->next->previous = entry->previous;
                entry->previous->next = entry->next;
                if (entry == messages.head_) messages.head_ = entry->next;
                messages.current_ = entry->next;
                messages.on_remove(entry->string_payload());
                delete entry->string_payload();
                entry->payload = 0;
                delete entry;
                --messages.entry_count_;
                --index;
            } else {
                if (entry->id > message_id) --entry->id;
                messages.current_ = entry->next;
            }
        }
        if (!messages.entry_count_) messages.head_ = 0;
        messages.current_position_ = messages.entry_count_ - 1;
    }
    InterfaceMessageWin.listBox_.update_changes();
    for (i = 0; i < MaxSecretProjectNum; i++) {
        if (GameDataStateStorage.fields_.secret_project_.project_base[i] == base_id) {
            GameDataStateStorage.fields_.secret_project_.project_base[i] = -2;
        } else if (GameDataStateStorage.fields_.secret_project_.project_base[i] > base_id) {
            GameDataStateStorage.fields_.secret_project_.project_base[i]--;
        }
    }
    for (i = 1; i < MaxPlayerNum; i++) {
        if (PlayersData[i].base_id_atk_target == base_id) {
            PlayersData[i].base_id_atk_target = -1;
        } else if (PlayersData[i].base_id_atk_target > base_id) {
            PlayersData[i].base_id_atk_target--;
        }
    }
    for (i = 0; i < 49; i++) {
        int x_radius = wrap_map_x(x + RadiusOffsetX[i]);
        int y_radius = y + RadiusOffsetY[i];
        if (y_radius >= 0 && y_radius < MapLatitudeBounds
            && x_radius >= 0 && x_radius < MapLongitudeBounds) {
            site_set(x_radius, y_radius, 0);
            if (i < 21) {
                bit_set(x_radius, y_radius, BIT_BASE_RADIUS, 0);
                if ((map_loc(x_radius, y_radius)->val3 & 7) == faction_id) {
                    using_set(x_radius, y_radius, 0);
                }
            }
        }
    }
    if (is_ocean(x, y)) {
        for (veh_id = 0; veh_id < GameDataStateStorage.fields_.vehicle_current_count_; veh_id++) {
            if (VehPrototypes[Vehs[veh_id].proto_id].plan == PLAN_NAVAL_TRANSPORT
                && !map_range(Vehs[veh_id].x, Vehs[veh_id].y, x, y)) {
                stack_veh(veh_id, 0);
            }
        }
        // Original terrain cleanup begins at the one-past active vehicle slot.
        // Do not silently repair that stale-slot/error-path read with a new guard.
        for (veh_id = GameDataStateStorage.fields_.vehicle_current_count_; veh_id >= 0; veh_id--) {
            if (get_triad(veh_id) == TRIAD_LAND
                && !map_range(Vehs[veh_id].x, Vehs[veh_id].y, x, y)
                && (Vehs[veh_id].order != ORDER_SENTRY_BOARD || Vehs[veh_id].waypoint_x[0] < 0)) {
                kill(veh_id);
            }
        }
        TutWinBaseWindow.base_killed(base_id);
    } else {
        for (veh_id = GameDataStateStorage.fields_.vehicle_current_count_; veh_id >= 0; veh_id--) {
            if (get_triad(veh_id) == TRIAD_SEA
                && !map_range(Vehs[veh_id].x, Vehs[veh_id].y, x, y)) {
                kill(veh_id);
            }
        }
        TutWinBaseWindow.base_killed(base_id);
    }
    for (i = 0; i < GameDataStateStorage.fields_.base_current_count_; i++) {
        base_mark(i);
    }
    reset_territory();
    WorldClimateWorldWin.field_239BC_ |= 2;
}








// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004E5A60 ?base_change@@YAXHH@Z 0x004E5A60-0x004E5FDC BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/004e5a60.cpp", "source:src/base.cpp:base_reset", "source:src/base.cpp:base_lose_minerals"], "reason": "Recover production selection, retool mineral adjustment and radius redraw through the typed Base and Rules. Project announcement and faction production counts remain unresolved; this is not a byte or behavioral claim."}
void __cdecl base_change(int base_id, int production_id) {
    if (production_id == 99999) return;

    const int previous = Bases[base_id].queue_production_id[0];
    Bases[base_id].queue_production_id[0] = production_id;
    if (production_id == previous) return;
    const int faction = Bases[base_id].faction_id_current;
    if (previous >= 0) --PlayersData[faction].proto_id_queue[previous];
    if (production_id >= 0) ++PlayersData[faction].proto_id_queue[production_id];

    // Switching production applies the rule's retool penalty to the stored
    // minerals, while keeping the unpenalized turn total for later accounting.
    int loss;
    if (Rules.retool_pct_pen_prod_chg
        && (GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & (1 << faction))
        && base_making(Bases[base_id].queue_production_id[0], base_id)
            != base_making(Bases[base_id].production_id_last, base_id)
        && Bases[base_id].minerals_accumulated_2 > static_cast<int>(Rules.retool_exemption)) {
        loss = Bases[base_id].minerals_accumulated_2
             - (Bases[base_id].minerals_accumulated_2 - static_cast<int>(Rules.retool_exemption))
                   * (100 - static_cast<int>(Rules.retool_pct_pen_prod_chg)) / 100
             - static_cast<int>(Rules.retool_exemption);
    } else {
        loss = 0;
    }
    Bases[base_id].minerals_accumulated = Bases[base_id].minerals_accumulated_2 - loss;
    draw_radius(Bases[base_id].x, Bases[base_id].y, 2, 2);
    if (faction == ConsoleGlobal.faction_id_ || AiResearchBatchActive) return;

    const int next = Bases[base_id].queue_production_id[0];
    int new_unbuilt;
    if (next <= -70 && base_project(-next - 70) == -1) {
        new_unbuilt = 1;
    } else {
        new_unbuilt = 0;
    }
    int old_relevant;
    if (previous <= -70 && (new_unbuilt || base_project(-previous - 70) == -1)) {
        old_relevant = 1;
    } else {
        old_relevant = 0;
    }
    if (old_relevant && has_project((previous < 0 ? -previous : previous) - 70, faction)) old_relevant = 0;
    if (!new_unbuilt && !old_relevant) return;
    if ((GameDataStateStorage.fields_.game_state_ & 1) && !(GameDataStateStorage.fields_.game_state_ & 8)) return;
    const int facility = (next < 0 ? -next : next);
    const int project = facility - 70;
    int word, mask;
    if (new_unbuilt) {
        bitmask(project, &word, &mask);
    } else {
        bitmask((previous < 0 ? -previous : previous) - 70, &word, &mask);
    }
    int old_count = 0, new_count = 0;
    int other;
    for (other = 0; other < GameDataStateStorage.fields_.base_current_count_; ++other) {
        if (Bases[other].faction_id_current == Bases[base_id].faction_id_current && other != base_id) {
            if (Bases[other].queue_production_id[0] == previous) ++old_count;
            if (Bases[other].queue_production_id[0] == Bases[base_id].queue_production_id[0]) ++new_count;
        }
    }
    if (!new_unbuilt || new_count) return;
    // Literal order (BEGINPROJECT before CHANGEPROJECT) puts the new-project
    // arm first in the source; VC6 lays the change arm out first.
    if (!old_relevant || (old_count && base_project(-previous - 70) == -1)) {
        if (!(PlayersData[faction].secret_project_intel[word] & mask)) {
            if (GameDataStateStorage.fields_.sunspot_duration_ > 0 && project != 31) return;
            parse_set(Players[faction].is_leader_female, 0);
            parse_says(0, Players[faction].title_leader, -1, -1);
            parse_set(Players[faction].is_leader_female, 0);
            parse_says(1, Players[faction].name_leader, -1, -1);
            parse_set(Players[faction].noun_gender, Players[faction].is_noun_plural);
            parse_says(2, Players[faction].noun_faction, -1, -1);
            parse_say(3, Facility[facility].name, -1, -1);
            if (PlayersData[ConsoleGlobal.faction_id_].diplo_treaties[faction] & 1) {
                wave_it(36);
            } else {
                wave_it(38);
            }
            popp(ScriptPopupFile, "BEGINPROJECT", 0, "secproj_sm.pcx", 0);
            PlayersData[faction].secret_project_intel[word] |= mask;
        }
        if (project == 31) {
            interlude(11, 0, 1, 0);
            GameDataStateStorage.fields_.game_interlude_flags_ |= 0x400;
        }
    } else {
        for (other = 0; other < GameDataStateStorage.fields_.base_current_count_; ++other)
            if (Bases[other].faction_id_current == faction
                && Bases[other].queue_production_id[0] == previous)
                Bases[other].queue_production_id[0] = Bases[base_id].queue_production_id[0];
        if (GameDataStateStorage.fields_.sunspot_duration_ > 0) return;
        parse_says(0, Players[faction].adj_name_faction, -1, -1);
        // RulesFacility's existing name field is already an LPSTR.
        parse_say(1, Facility[(previous < 0 ? -previous : previous)].name, -1, -1);
        parse_set(Players[faction].noun_gender, Players[faction].is_noun_plural);
        parse_says(2, Players[faction].noun_faction, -1, -1);
        parse_say(3, Facility[facility].name, -1, -1);
        popp(ScriptPopupFile, "CHANGEPROJECT", 0, "secproj_sm.pcx", 0);
        PlayersData[faction].secret_project_intel[word] |= mask;
        bitmask((previous < 0 ? -previous : previous) - 70, &word, &mask);
        PlayersData[faction].secret_project_intel[word] &= ~mask;
    }
}

// ORIGINAL: 0x004E5FE0 ?base_reset@@YAXHH@Z 0x004E5FE0-0x004E62FB BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/recovered/units/004e5fe0.cpp", "source:src/base.h", "source:src/base.cpp:BaseSiteClaimX/BaseSiteClaimY are the four 0x0090EA44/54 site-output arrays", "source:src/base.h:Base.location is the 0x130 field read as 0x0097D090 + base_id*0x134", "pinned image:0x00689484 contains secproj_sm.pcx;0x00689494 contains GOVPROJECT"], "reason": "Recover the base reset/site selection operation using the typed Base and faction state, four source-owned site claims, and source-owned popup text. Preserve the existing seed and mismatch obligation; base_build/base_change remain explicit link-only dependencies."}
// The governor prompt's result is popp's return (the image keeps it in esi
// across the help loop), not parse_say's.
void __cdecl base_reset(int base_id, int force) {
    int loop_count = 0;
    int faction = Bases[base_id].faction_id_current;
    if (((1 << faction) & GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_)
        && (Bases[base_id].governor & GOV_ACTIVE) && force) {
        uint32_t governor = Bases[base_id].governor;
        for (int site = 0; site < 4; ++site) {
            BaseSiteClaimY[site] = 999999;
            BaseSiteClaimX[site] = 0;
        }
        if (governor & 0x0F000000u) {
            for (;;) {
                Bases[base_id].governor = (Bases[base_id].governor & 0xF1FFFFFFu) | GOV_PRIORITY_EXPLORE;
                BaseSiteClaimX[0] = base_build(base_id, 0, 0, &BaseSiteClaimY[0]);
                Bases[base_id].governor = (Bases[base_id].governor & 0xF2FFFFFFu) | GOV_PRIORITY_DISCOVER;
                BaseSiteClaimX[1] = base_build(base_id, 0, 0, &BaseSiteClaimY[1]);
                if (BaseSiteClaimX[0] == BaseSiteClaimX[1] && ++loop_count < 8) {
                    continue;
                }
                Bases[base_id].governor = (Bases[base_id].governor & 0xF4FFFFFFu) | GOV_PRIORITY_BUILD;
                BaseSiteClaimX[2] = base_build(base_id, 0, 0, &BaseSiteClaimY[2]);
                if ((BaseSiteClaimX[0] == BaseSiteClaimX[2] || BaseSiteClaimX[1] == BaseSiteClaimX[2])
                    && ++loop_count < 8) {
                    continue;
                }
                Bases[base_id].governor = (Bases[base_id].governor & 0xF8FFFFFFu) | GOV_PRIORITY_CONQUER;
                BaseSiteClaimX[3] = base_build(base_id, 0, 0, &BaseSiteClaimY[3]);
                if ((BaseSiteClaimX[0] == BaseSiteClaimX[3] || BaseSiteClaimX[1] == BaseSiteClaimX[3]
                     || BaseSiteClaimX[2] == BaseSiteClaimX[3])
                    && ++loop_count < 8) {
                    continue;
                }
                break;
            }
            Bases[base_id].governor = governor;
        }
        base_change(base_id, base_build(base_id, 0, 0, &loop_count));
        return;
    }
    int location = Bases[base_id].queue_production_id[0];
    base_change(base_id, base_build(base_id, 0, 0, 0));
    if (!((1 << faction) & GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_)) return;
    if (!(Bases[base_id].governor & GOV_ACTIVE)) return;
    if (location <= -70) return;
    if (Bases[base_id].queue_production_id[0] > -70) return;
    if (IsMultiplayerNet) return;
    if (GameDataStateStorage.fields_.game_state_ & STATE_UNK_1) return;
    if (Bases[base_id].queue_production_id[0] >= 0) {
        location = Bases[base_id].queue_production_id[0];
    } else {
        location = -Bases[base_id].queue_production_id[0];
    }
    int choice = 3;
    while (choice == 3) {
        parse_says(0, Bases[base_id].name_string, -1, -1);
        parse_say(1, Facility[location].name, -1, -1);
        choice = popp(ScriptPopupFile, "GOVPROJECT", 0, "secproj_sm.pcx", 0);
        if (choice == 3) {
            help_topic(0x0B, location);
        }
    }
    if (!choice) return;
    base_change(base_id, base_build(-1 - base_id, 0, 0, 0));
    if (choice <= 1) return;
    BaseCurrent->governor &= ~0x00040000u;
    PlayersData[faction].base_governor_adv &= ~0x00040000u;
}

/*
Purpose: Reset every base of one faction: the faction's bases are visited in
         table order and each is handed to base_reset when the caller's
         context allows it - the faction is not locked out by the human/alive
         status bytes, the base optionally lies in the requested region, and
         with the occupancy flag the tile must be free of vehicles or without
         a current owner.

// ORIGINAL: 0x004E6300 ?bases_reset@@YAXHHH@Z 0x004E6300-0x004E63FC FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence":["src/base.h declares bases_reset and owns the actual Base array and BaseCurrent; its Base members x/y/faction_id_current/state have RECOVERY_MEMBER evidence.","The pinned map-record stride is 0x2C; src/map.h's Map is size 0x2C with climate/contour/val2/region at +0..+3 and bit at +8. Typed indexing preserves the original field reads.","The body keeps both live enable-mask reads, the original short-circuit order, the base_reset callback, and the post-callback MapTiles/MapLongitude reload. No alias or manual table is introduced."],"reason":"Move the existing reset loop into the Base owner and express its already-established Base/Map storage through actual members while preserving branch, callback, reload, and unsafe-input behavior."}
// TRIED: single AND-chained `if` (faction==a2 && (a1<0||idx_byte==a1) && (a3==0||bits check) && (mask flag||special)) reproduces the loop body; `int mask`/`unsigned char mask` both leave the divergence at MISMATCH #2 (original reserves the one local with `push ecx` right after `mov ebp,esp`; every C form tried here gets `sub esp,4` instead) - same prologue-reservation wall as the mon_* family (0x00477840).
// size      252 bytes
// callers   4   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004E5FE0
// notes     Seed provenance (TRIED): the single AND-chained `if` reproduces
//           the loop body; the residual divergence is the prologue local
//           reservation - the original reserves its one local with `push ecx`
//           right after `mov ebp,esp`, and every C form tried reaches for
//           `sub esp,4` (the mon_* family wall at 0x00477840).
Return Value: n/a
Status: Coverage transcription - unclaimed body; the seed
        git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/004e6300.cpp is retained until this product
        definition measures fresh BYTE_EXACT.
*/
void __cdecl bases_reset(int region_id, int faction_id, int skip_occupied) {
    if (is_human(faction_id)) {
        return;
    }
    for (int i = 0; i < GameDataStateStorage.fields_.base_current_count_; i++) {
        if (Bases[i].faction_id_current == faction_id
            && (region_id < 0 || MapTiles[(Bases[i].x >> 1) + Bases[i].y * MapLongitude].region == region_id)
            && (!skip_occupied || veh_who(Bases[i].x, Bases[i].y) < 0)
            && (!is_human(faction_id) || !(BaseCurrent->state & 0x80000000))) {
            base_reset(i, 0);
        }
    }
}

/*
Purpose: Calculate the new unit morale bonus modifier provided by the base and faction for a triad.
// ORIGINAL: 0x004E6400 ?morale_mod@@YAHHHH@Z 0x004E6400-0x004E65B1 BYTE_EXACT
// LEVER: image makes 4 real `call 0x50ba00` (bitmask), one per
// has_fac_built() site; the `inline bool has_fac_built(int, int)` in
// base.h was not being inlined here and compiled to a real call to
// has_fac_built itself instead (call_diff: MORE, wrong callee). Switching
// all four checks to has_fac_built_call() (base.h, __forceinline, the same
// lever recorded on cost_factor/breed_mod) fixes the call count (4 real
// bitmask calls, matching) and moves best similarity from a much lower
// score to 0.715, 7/153 agreeing.
// LEVER: the Bioenhancement test spelled `if (has_fac) ... else if
// (has_project) ...` gives the image's ebx=base_id/edi=modifier allocation;
// with one switch case spelled the same way the AIR/SEA has_project tails
// merge into AIR's copy as in the image (either case form alone works).
// size      433 bytes
// prototype int (__cdecl ?morale_mod@@YAHHHH@Z)(int baseID, int factionID, int triad)
// callers   2   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0050BA00
Return Value: Morale bonus modifier
Status: Complete
*/
int __cdecl morale_mod(int base_id, int faction_id, int triad) {
    int morale_modifier = 0;
    switch (triad) {
    case TRIAD_LAND:
        if (has_fac(FAC_COMMAND_CENTER, base_id, 0)) {
            morale_modifier += 2;
        } else if (has_project(SP_COMMAND_NEXUS, faction_id)) {
            morale_modifier += 2;
        }
        break;
    case TRIAD_SEA:
        if (has_fac(FAC_NAVAL_YARD, base_id, 0)
            || has_project(SP_MARITIME_CONTROL_CENTER, faction_id)) {
            morale_modifier += 2;
        }
        break;
    case TRIAD_AIR:
        if (has_fac(FAC_AEROSPACE_COMPLEX, base_id, 0)
            || has_project(SP_CLOUDBASE_ACADEMY, faction_id)) {
            morale_modifier += 2;
        }
        break;
    }
    if (has_fac(FAC_BIOENHANCEMENT_CENTER, base_id, 0)) {
        morale_modifier += 2;
    } else if (has_project(SP_CYBORG_FACTORY, faction_id)) {
        morale_modifier += 2;
    }
    if (PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::MORALE] < -1) {
        morale_modifier /= 2;
    }
    return morale_modifier;
}

/*
Purpose: Calculate the new native unit lifecycle bonus modifier provided by a base and faction.
// ORIGINAL: 0x004E65C0 ?breed_mod@@YAHHH@Z 0x004E65C0-0x004E673D BYTE_EXACT
// body      src/base.cpp
// LEVER: body lives in base.h (MEASURED inline) as four has_fac_built_call()
//        checks instead of has_fac_built() - fixes breed_mod's OWN call
//        count (0 -> 4 real bitmask() calls) and moves its best similarity
//        to 0.749. TRADE-OFF, measured: since breed_mod is a single shared
//        inline body, the same edit reaches worm_mod's inlined copy and
//        drops ITS best similarity 0.826 -> 0.706 (worm_mod is 0x004E6740,
//        not claimed, so not a tracked regression - full detail at the
//        body in base.h and at worm_mod's own marker below).
// TRIED: register swap in the prologue (image keeps ebx=base_id,
//            esi=faction_id; /O2 default swaps them) and how the bool
//            result of `has_fac_built_call(...) || has_project(...)` is
//            materialised - the image spins it through a stack slot as
//            scratch before the `||`, which a direct `test/jne` never
//            reproduces. Register allocation, not a call-count/lever issue.
// size      381 bytes
// prototype int (__cdecl ?breed_mod@@YAHHH@Z)(int baseID, int factionID)
// callers   2   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0050BA00
Return Value: Lifecycle bonus
Status: Complete
*/
// HISTORICAL: the body was placed in base.h as `MEASURED inline` so worm_mod
// could fold it in place; the image writes breed_mod's whole body there
// rather than calling 0x004E65C0. The four has_fac_built(FAC_X, base_id)
// checks were also previously swapped for has_fac(FAC_X, base_id, 0), based
// on the recorded LEVER on base_minerals (base.cpp:1820 at the time), which
// found that swap moved that body's call count from 1 to 6 of 7.
inline int __cdecl breed_mod(int base_id, int faction_id) {
    int lifecycle_modifier = 0;
    if (has_project(SP_XENOEMPATYH_DOME, faction_id)) {
        lifecycle_modifier++;
    }
    if (has_project(SP_PHOLUS_MUTAGEN, faction_id)) {
        lifecycle_modifier++;
    }
    if (has_project(SP_VOICE_OF_PLANET, faction_id)) {
        lifecycle_modifier++;
    }
    if (has_fac(FAC_CENTAURI_PRESERVE, base_id, 0)) {
        lifecycle_modifier++;
    }
    if (has_fac(FAC_TEMPLE_OF_PLANET, base_id, 0)) {
        lifecycle_modifier++;
    }
    if (has_fac(FAC_BIOLOGY_LAB, base_id, 0)) {
        lifecycle_modifier++;
    }
    if (has_fac(FAC_BIOENHANCEMENT_CENTER, base_id, 0)
        || has_project(SP_CYBORG_FACTORY, faction_id)) {
        lifecycle_modifier++;
    }
    return lifecycle_modifier;
}

/*
Purpose: Calculate the count of lifecycle/psi bonuses that are provided by a base and faction.
// ORIGINAL: 0x004E6740 ?worm_mod@@YAHHH@Z 0x004E6740-0x004E68AB BYTE_EXACT
// size      363 bytes
// prototype int (__cdecl ?worm_mod@@YAHHH@Z)(int baseID, int factionID)
// callers   1   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00421670
// LEVER: made breed_mod `MEASURED inline` at the end of base.h (lever 1) so
//        worm_mod open-codes it as the image does - the image's only calls
//        here are four has_fac (0x421670), never breed_mod out of line -
//        and swapped breed_mod's has_fac_built(FAC_X, base_id) tests for
//        has_fac(FAC_X, base_id, 0) so those four calls actually appear.
//        Best similarity 0.826, agreeing instructions 3->13 of 140.
// REGRESSED BY A SIBLING FIX, not yet re-chased: breed_mod's OWN address
// (0x004E65C0) needed those four has_fac() calls hand-inlined to
// has_fac_built_call() instead (see the LEVER at breed_mod's definition,
// base.h) - since breed_mod is one shared body, that same edit reaches
// worm_mod's inlined copy and drops its best similarity 0.826 -> 0.706.
// Not fixed here: worm_mod was not in the batch that made this trade.
Return Value: Native life modifier count
Status: Complete
*/
int __cdecl worm_mod(int base_id, int faction_id) {
    uint32_t worm_modifier_count = breed_mod(base_id, faction_id);
    if (Players[faction_id].rule_psi) {
        worm_modifier_count++;
    }
    if (has_project(SP_DREAM_TWISTER, faction_id)) {
        worm_modifier_count++;
    }
    if (has_project(SP_NEURAL_AMPLIFIER, faction_id)) {
        worm_modifier_count++;
    }
    return worm_modifier_count;
}

/*
 * THE PER-FACTION PROTOTYPE-STATUS BYTE RUN, as the image addresses it:
 * PlayersData (0x0096C9E0) + 0x858, indexed faction * 0x20CC + prototype id.
 * veh.cpp names the same address FactionProtoSeenCount (it decrements it when
 * a vehicle of that prototype dies); upgrade_prototypes reads it as the gate
 * on whether the faction may still upgrade that prototype at all, which is
 * the same fact read from the other end. Both are internal-linkage
 * definitions of one address because base.h's PlayerData puts its
 * proto_id_active / proto_id_queue arrays at different offsets from this (the
 * struct's middle and tail are where the image's own layout disagrees), so
 * neither file can reach the byte through a field name.
 */
// Current layout evidence supersedes that seed conclusion: 0x0096D238 is
// PlayersData[0].proto_id_active[0] (PlayerData +0x858), with a 0x20CC row stride.

/*
 * "Don't ask again" latch for the prototype-upgrade confirmation. The automatic
 * upgrade sweep (0x004F0650) clears it, upgrade_prototype (0x004EFE80) shows
 * the confirm popup only while it is zero and sets it when the caller answers
 * "yes to all" (popup result > 1); 0x00583E30, 0x005BB000, 0x005290A0 and
 * 0x00438460 clear it at the top of their own upgrade passes.
 */
// RECOVERY_OBJECT: v2 0x0090EA3C 0x0090EA40 ?UpgradePromptAnswered@@3IA 0x00438B19
uint32_t UpgradePromptAnswered;  // 0x0090EA3C

int BaseCurrentGrowthRate;  // 0x0090E918
// RECOVERY_OBJECT: v2 0x0090E980 0x0090E984 ?BaseCurrentVehPacifismCount@@3HA 0x004E9550
int BaseCurrentVehPacifismCount;  // 0x0090E980
// RECOVERY_OBJECT: v2 0x0090E8FC 0x0090E900 ?BaseCurrentForcesSupported@@3HA 0x004E95AB
int BaseCurrentForcesSupported;  // 0x0090E8FC
// RECOVERY_OBJECT: v2 0x0090E91C 0x0090E920 ?BaseCurrentForcesMaintCost@@3HA 0x0040A6A5
int BaseCurrentForcesMaintCost;  // 0x0090E91C

/*
 * How many of the base's supported units are past its free allowance, counted
 * alongside BaseCurrentForcesMaintCost. base_support() (0x004E9550) is the only
 * function in the executable that touches this address at all - it zeroes it,
 * reads it back and stores the increment, and nothing ever reads the total - so
 * the count is written and then dropped. Kept because it is observable state.
 */
// Bound: scalar; every original access is a dword at the start, never indexed, and the following bytes are never referenced.
// RECOVERY_OBJECT: v2 0x0090EA08 0x0090EA0C ?BaseCurrentForcesMaintCount@@3IA 0x004E95A5
uint32_t BaseCurrentForcesMaintCount;  // 0x0090EA08

// Tile base owner, as the yield functions inline it (yield_tile_owner).
static __forceinline int tile_base_owner(const Map *tile) {
    if (!(tile->bit & BIT_BASE_IN_TILE)) {
        return -1;
    }
    int owner = tile->val2 & 0xF;
    return (owner < MaxPlayerNum) ? owner : -1;
}

static inline int farm_stack_top(int veh_id) {
    while (Vehs[veh_id].prev_veh_id_stack >= 0) {
        veh_id = Vehs[veh_id].prev_veh_id_stack;
    }
    return veh_id;
}

static inline BOOL farm_veh_visible(int veh_id, int faction) {
    if (Vehs[veh_id].faction_id == faction && (Vehs[veh_id].flags & 0x440) != 0x440) {
        return true;
    }
    if (Vehs[veh_id].visibility & (1 << faction)) {
        return true;
    }
    return false;
}

/*
 * Out-parameter of the three terrain yield functions: how much of a resource
 * the square lost to the "three per square needs a technology" restriction, and
 * to mine_yield's roadless-mine mineral limit. Callers zero it, ask for a
 * yield, then read it back - BaseWin::draw_farm (0x0040F0F0) does exactly that
 * for each of the three resources, and resource_yield zeroes it for RSC_PSI.
 * The executable's two clear paths identify this start; purpose comes from callers.
 */
// Independent crop_yield accesses at 0x004E72C2/0x004E72D0 read/write a DWORD;
// farm_compute uses separate DWORD storage at 0x0090E99C. This bounds the slot,
// without establishing the original signedness or source-level identifier.
// RECOVERY_OBJECT: v2 0x0090E998 0x0090E99C ?TileYieldRestricted@@3HA 0x004E6E50
int TileYieldRestricted;  // 0x0090E998
// farm_compute publishes 25 status words, including four excluded outer
// positions. The separate mask covers only the first 21 workable positions.
// Both objects are initially zero in the image; their producer, not that
// initial state, determines availability for the current base.
// Status bits: 1 excluded outer position, 2 off-map/unrevealed, 4 base tile,
// 8 stack-check or visible non-allied vehicle, 0x10 another base's worked
// square, 0x20 the foreign-territory rule below. These describe the observed
// tests, not recovered historical names. The recovered base_yield reads the
// mask; the farm_click/draw_farm word consumers remain unfinished obligations.
// RECOVERY_OBJECT: v2 0x0090E99C 0x0090EA00 ?FarmStatus@@3PAIA 0x004E68B0
uint32_t FarmStatus[25];
// RECOVERY_OBJECT: v2 0x0090EA00 0x0090EA04 ?FarmUnavailableMask@@3IA 0x004E68B0
uint32_t FarmUnavailableMask;

// Requires the original game's valid faction, base, tile and vehicle indices
// and well-formed vehicle chains. Keep the entry coordinates/faction separate
// from the later BaseCurrent reload used to select nearby competing bases.
// ORIGINAL: 0x004E68B0 ?farm_compute@@YAXXZ 0x004E68B0-0x004E6E4E BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["4E68B6 loads BaseCurrent pointer before signedshort x/y and unsignedbyte faction.4E68E3/4E6BA5 establish25 DWORD outputs, with final4 set1;4E6E23..43 reduce onlyfirst21 into90EA00.4E6C7A tests Base+78 worked mask, not seed+3C.", "4E6966/4E69C4/4E6B99 reload row stride after calls; map-pointer loads4E697C/4E6A0A/4E6A46 must remain live.4E6B38 exhausts vehicle iterator before territory check4E6B7C.4E6BE7 reloads BaseCurrent for neighbor admission, while inner radius matching retains entry coordinates."], "reason": "This no-argument resource pass computes shared per-radius availability from the selected Base, typed map/vehicle/diplomacy state and neighboring worked squares; it owns25 status words and a separate21-position mask."}
void __cdecl farm_compute() {
    int base_x = BaseCurrent->x;
    int base_y = BaseCurrent->y;
    int faction = BaseCurrent->faction_id_current;
    int position;
    for (position = 0; position < 25; position++) {
        if (position >= 21) {
            FarmStatus[position] = 1;
            continue;
        }
        int x = wrap_map_x(base_x + RadiusOffsetX[position]);
        int y = base_y + RadiusOffsetY[position];
        if (!on_map(x, y)) {
            FarmStatus[position] = 2;
            continue;
        }
        if (position < 21 && BaseUpkeepStage == 1 && goody_at(x, y)
            && (map_tiles()[y * (int)MapLongitude + (x >> 1)].climate & ~0x1F) >= 0x60) {
            add_goal(faction, 3, 1, x, y, -1);
            add_goal(faction, 2, 1, x, y, -1);
            add_goal(faction, 0, 1, x, y, -1);
        }
        if (!(PlayersData[faction].flags & 0x200)
            && !(map_tiles()[y * (int)MapLongitude + (x >> 1)].visibility & (1 << faction))) {
            FarmStatus[position] = 2;
            continue;
        }
        FarmStatus[position] = 0;
        if (tile_base_owner(&map_tiles()[y * (int)MapLongitude + (x >> 1)]) >= 0) {
            FarmStatus[position] = 4;
        }
        int veh_id = stack_fix(veh_at(x, y));
        if (veh_id >= 0) {
            if (stack_check(veh_id, 9, 3, -1, -1)) {
                FarmStatus[position] |= 8;
            }
            veh_id = farm_stack_top(veh_id);
            for (; veh_id >= 0; veh_id = Vehs[veh_id].next_veh_id_stack) {
                int owner = Vehs[veh_id].faction_id;
                if (owner >= 0 && owner != faction && farm_veh_visible(veh_id, faction)
                    && !(PlayersData[faction].diplo_treaties[owner] & 3)) {
                    FarmStatus[position] |= 8;
                }
            }
        }
        // The original uses the exhausted iterator here, not the saved stack
        // head. Do not turn this into a new friendly-occupant exemption.
        int territory = whose_territory(faction, x, y, 0, 0);
        if (territory >= 0 && territory != faction
            && ((PlayersData[faction].diplo_treaties[territory] & 3)
                || veh_id < 0 || Vehs[veh_id].faction_id != faction)) {
            FarmStatus[position] |= 0x20;
        }
    }
    for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; base_id++) {
        if (base_id == BaseIDCurrentSelected) {
            continue;
        }
        Base *other = &Bases[base_id];
        if (vector_dist(BaseCurrent->x, BaseCurrent->y, other->x, other->y) > 5) {
            continue;
        }
        for (int worked = 0; worked < 21; worked++) {
            if (worked && !(other->resource_sqr_radius & (1 << worked))) {
                continue;
            }
            int x = wrap_map_x(other->x + RadiusOffsetX[worked]);
            int y = other->y + RadiusOffsetY[worked];
            if (!on_map(x, y)) {
                continue;
            }
            if (other->faction_id_current != faction
                && (map_tiles()[y * (int)MapLongitude + (x >> 1)].climate & ~0x1F) >= 0x60) {
                continue;
            }
            if (vector_dist(base_x, base_y, x, y) > 2) {
                continue;
            }
            for (int square = 0; square < 21; square++) {
                if (wrap_map_x(base_x + RadiusOffsetX[square]) == x
                    && base_y + RadiusOffsetY[square] == y) {
                    FarmStatus[square] |= 0x10;
                }
            }
        }
    }
    FarmUnavailableMask = 0;
    for (position = 0; position < 21; position++) {
        if (FarmStatus[position]) {
            FarmUnavailableMask |= 1 << position;
        }
    }
}


/*
Purpose: Address the map tile the terrain yield functions price.
Original Offset: n/a
Return Value: Pointer to map tile
Status: Complete

The three yield functions each inline this, and they inline it with signed
arithmetic - `imul` against y and `sar esi, 1` on x. map_loc() takes unsigned
coordinates and therefore shifts x logically, which differs for a negative x.
Neither form bounds-checks, exactly as the original does not.
*/
static __forceinline Map *yield_tile(int x, int y) {
    return &map_tiles()[(x >> 1) + y * (int)MapLongitude];
}

/*
Purpose: Identify the faction owning a base standing on the specified tile.
Original Offset: n/a
Return Value: Faction id of the base in the tile, or -1 for none/unowned
Status: Complete

All three yield functions inline this same sequence: the tile's low `bit` flag
selects it, the owner nibble of val2 supplies the id, and an id at or above
MaxPlayerNum - 0xF is the "unoccupied" marker - is rejected.
*/
static __forceinline int yield_tile_owner(const Map *tile) {
    if (!(tile->bit & BIT_BASE_IN_TILE)) {
        return -1;
    }
    int owner = tile->val2 & 0xF;
    return (owner < MaxPlayerNum) ? owner : -1;
}

/*
Purpose: Calculate the nutrients produced by a single map square.
// ORIGINAL: 0x004E6E50 ?crop_yield@@YAHHHHHH@Z 0x004E6E50-0x004E7306 BYTE_EXACT
// TRIED: caching bit2 to a named local before the landmark_bonus jungle/
//            fresh check (image re-reads tile->climate twice there instead
//            of the cached is_ocean_tile - now written that way, but score
//            unchanged: 0x8-vs-0xC sub esp gap under /Oy- is unexplained,
//            one 4-byte stack slot short of the image with no candidate
//            local found for it in the time spent).
// size      1206 bytes
// prototype int (__cdecl ?crop_yield@@YAHHHHHH@Z)(int factionID, int, int, int, int)
// callers   10   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0050BA00 0x00592030 0x005B9F20
// LEVER: yield_tile/yield_tile_owner (this file) marked `static __forceinline`
//        - shared with mine_yield/energy_yield/base_support, and a clear win
//        for base_support (0.160 -> 0.371 best similarity there). Tied here:
//        this function's own best-across-flags similarity is unchanged
//        (0.292, its winning flag set is /Ob0-based, where yield_tile was
//        already being folded in regardless of the marker) - kept anyway
//        since it does not regress and helps the sibling.
// TRIED: a `bonus_at_call` non-inline forwarder (map.h/map.cpp, since
//            removed) so bonus_at(x, y, 0) would emit `call 0x592030` as
//            the image does - measured NO improvement to best-across-flags
//            similarity (stayed at 0.292) on top of the forceinline change.
// LEVER: FEWER (2 calls vs image's 7: 5 bitmask() + bonus_at + has_tech) -
//        all five `has_fac_built(FAC_X, base_id)` sites constant-fold their
//        offset/mask math away here, unlike the image, which keeps
//        bitmask() out-of-line at every one. Switching all five to
//        `has_fac_built_call` took call_diff to 0 disagreeing with no
//        similarity regression (still 0.292 best-across-flags).
Return Value: Nutrients from the square
Status: Complete

`assume_improved` prices the square as if it had already been terraformed; the
name is inferred from the call sites, where StatusWin::draw_status asks for the
same square twice and base_terraform and the AI previews pass 1. Here it
suppresses the fungus path, an improved square having had its fungus cleared.
*/
int __cdecl crop_yield(int faction_id, int base_id, int x, int y,
                       BOOL assume_improved) {
    Map *tile = yield_tile(x, y);
    uint32_t bit = tile->bit;
    BOOL is_ocean_tile = (tile->climate & ~0x1F) < ALT_BIT_SHORE_LINE;
    // Re-addressed: the image re-reads tile->bit here instead of reusing `bit`.
    BOOL has_base_in_tile = yield_tile_owner(yield_tile(x, y)) >= 0;
    // Bonus id 1 is the nutrient bonus.
    BOOL has_nutrient_bonus = bonus_at(x, y, 0) == 1;
    tile = yield_tile(x, y);
    // Nutrient landmarks: Jungle counts on land, Freshwater Sea on ocean. Not
    // cached to a local: bit2 is read once here (reused via register across
    // both disjuncts within this one expression), and both arms re-test
    // climate directly off tile->climate rather than reusing the cached
    // is_ocean_tile above - the image re-reads that byte twice here instead
    // of referencing the earlier local.
    BOOL landmark_bonus =
        (((tile->bit2 & (BIT2_UNK_80000000 | BIT2_JUNGLE)) == BIT2_JUNGLE)
            && (tile->climate & ~0x1F) >= ALT_BIT_SHORE_LINE)
        || ((tile->bit2 & BIT2_FRESH) && (tile->climate & ~0x1F) < ALT_BIT_SHORE_LINE);
    int crop;

    if (has_base_in_tile) {
        crop = ResourceInfo[RSCINFO_BASE_SQ].nutrients;
        if (has_nutrient_bonus) {
            crop *= 2;
        }
        if (landmark_bonus) {
            crop++;
        }
        if (base_id < 0) {
            return crop; // the original returns here rather than reaching the tail
        }
        if (has_fac(FAC_RECYCLING_TANKS, base_id, 0)
            || has_fac(FAC_PRESSURE_DOME, base_id, 0)) {
            crop += ResourceInfo[RSCINFO_RECYCLING_TANKS].nutrients;
        }
    } else if (bit & BIT_THERMAL_BORE) {
        crop = ResourceInfo[RSCINFO_BOREHOLE_SQ].nutrients;
        // Faithful: 0x004E6FCB is `test esi, esi` on the borehole value that was
        // just loaded, NOT on the resource bonus flag. mine_yield's structurally
        // parallel arm tests the flag. Transcribed literally.
        if (crop) {
            crop += ResourceInfo[RSCINFO_BONUS_SQ].nutrients;
        }
    } else if (bit & BIT_MONOLITH) {
        crop = ResourceInfo[RSCINFO_MONOLITH].nutrients;
        if (has_nutrient_bonus) {
            crop += ResourceInfo[RSCINFO_BONUS_SQ].nutrients;
        }
        if (has_project(SP_MANIFOLD_HARMONICS, faction_id)
            && PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::PLANET] > 0) {
            crop++;
        }
    } else if ((bit & BIT_FUNGUS) && !assume_improved
               && (tile->climate & ~0x1F) >= ALT_BIT_OCEAN_SHELF) {
        int planet = PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::PLANET];
        int planet_mod = (planet >= -3) ? ((planet > 0) ? 0 : planet) : -3;
        crop = range(PlayersData[faction_id].tech_fungus_[RSC_NUTRIENTS] + planet_mod,
                     0, 99);
        if (has_project(SP_MANIFOLD_HARMONICS, faction_id) && planet > 0) {
            crop++;
        }
    } else {
        if (is_ocean_tile) {
            crop = ResourceInfo[RSCINFO_OCEAN_SQ].nutrients;
            if (has_nutrient_bonus) {
                crop += ResourceInfo[RSCINFO_BONUS_SQ].nutrients;
            }
            if (landmark_bonus) {
                crop++;
            }
            if ((tile->climate & ~0x1F) == ALT_BIT_OCEAN_SHELF
                || ExpansionEnabled) {
                // A kelp farm.
                if (bit & BIT_FARM) {
                    crop += ResourceInfo[RSCINFO_IMPROVED_SEA].nutrients;
                    // Legacy: unguarded for base_id < 0, exactly as 0x004E7150.
                    if (has_fac(FAC_AQUAFARM, base_id, 0)) {
                        crop++;
                    }
                }
                if ((bit & BIT_MINE) && crop > 1) {
                    crop += Rules.tgl_nutrient_effect_with_mine;
                }
            }
        } else if (bit & BIT_FOREST) {
            crop = (has_nutrient_bonus ? ResourceInfo[RSCINFO_BONUS_SQ].nutrients : 0)
                   + ResourceInfo[RSCINFO_FOREST_SQ].nutrients;
            if (base_id >= 0) {
                if (has_fac(FAC_HYBRID_FOREST, base_id, 0)) {
                    crop++;
                }
                if (has_fac(FAC_TREE_FARM, base_id, 0)) {
                    crop++;
                }
            }
            if (landmark_bonus) {
                crop++;
            }
        } else {
            int rockiness = tile->val3 >> 6;
            // Rainfall level: arid / moist / rainy.
            crop = (tile->climate >> 3) & 3;
            if (rockiness > ROCKINESS_ROLLING) {
                crop = 0;
            }
            if (has_nutrient_bonus) {
                crop += ResourceInfo[RSCINFO_BONUS_SQ].nutrients;
            }
            if (landmark_bonus) {
                crop++;
            }
            if (crop < 0) {
                crop = 0;
            }
            if ((bit & BIT_FARM) && rockiness <= ROCKINESS_ROLLING) {
                crop += ResourceInfo[RSCINFO_IMPROVED_LAND].nutrients;
            }
            if ((bit & BIT_MINE) && crop > 1) {
                crop += Rules.tgl_nutrient_effect_with_mine;
            }
        }
        if (bit & BIT_SOIL_ENRICHER) {
            crop += crop / 2;
        }
        uint32_t has_condenser = bit & BIT_CONDENSER;
        if (has_condenser) {
            crop += crop / 2;
        }
        // Only the ocean and land arms reach the restriction; the base,
        // borehole, monolith and fungus arms jump past it to the event tail.
        if (crop > 2 && !has_tech(Rules.tech_three_nutrients_sqr, faction_id)
            && !has_nutrient_bonus && !has_condenser) {
            TileYieldRestricted += crop - 2;
            crop = 2;
        }
    }

    if (base_id >= 0) {
        uint32_t event = Bases[base_id].event;
        if (event & BEVENT_BUMPER) {
            crop++;
        }
        if ((event & BEVENT_FAMINE) && crop) {
            crop--;
        }
    }
    return crop;
}

/*
Purpose: Calculate the minerals produced by a single map square.
// ORIGINAL: 0x004E7310 ?mine_yield@@YAHHHHHH@Z 0x004E7310-0x004E7748 BYTE_EXACT
// LEVER: bit2 was cached to a named local and read once; the image reads
//        tile->bit2 fresh at each of the two use sites (code=bit2>>24, then
//        again for the landmark_bonus mask) because it does not survive the
//        bonus_at() call. Also is_ocean_tile was hoisted; the image computes
//        it lazily, right at its one `else if`, not up front. Both fixed:
//        agreeing instructions 2->5 of 355 (/Oy-), similarity 0.379->0.407.
// TRIED: the image needs ZERO stack (reuses the dead x/y/base_id/
//            assume_improved parameter slots as scratch throughout); this
//            tree's body still needs sub esp 0xc for ~8 live locals across
//            the branch chain. That reuse is a VC6 liveness heuristic, not
//            something reachable by reordering source further in the time
//            spent here.
// size      1080 bytes
// prototype int (__cdecl ?mine_yield@@YAHHHHHH@Z)(int factionID, int, int, int, int)
// callers   6   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0050BA00 0x00592030 0x005B9F20
// LEVER: yield_tile/yield_tile_owner (this file) marked `static __forceinline`
//        - shared with crop_yield/energy_yield/base_support, a clear win for
//        base_support (0.160 -> 0.371). Tied here (best-across-flags stays
//        0.388, won by an /Ob0 flag set where yield_tile already folded
//        regardless) - kept anyway since it does not regress.
// TRIED: a `bonus_at_call` non-inline forwarder (map.h/map.cpp, since
//            removed) so bonus_at(x, y, 0) would emit `call 0x592030` -
//            measured no improvement on top of the forceinline change.
// LEVER: FEWER (3 calls vs image's 6: 3 bitmask() + bonus_at + 2 has_tech) -
//        all three `has_fac_built(FAC_X, base_id)` sites constant-fold their
//        offset/mask math away here, unlike the image, which keeps
//        bitmask() out-of-line at every one. Switching all three to
//        `has_fac_built_call` took call_diff to 0 disagreeing with no
//        similarity regression (still 0.388 best-across-flags).
Return Value: Minerals from the square
Status: Complete

`assume_improved` stands in for a mine or borehole on land and a mining platform
at sea, and suppresses the roadless-mine mineral limit.
*/
int __cdecl mine_yield(int faction_id, int base_id, int x, int y,
                       BOOL assume_improved) {
    Map *tile = yield_tile(x, y);
    uint32_t bit = tile->bit; // declared first: the image loads it into edx
    // Landmark sequence code, signed exactly as `sar edi, 0x18`. Bit 31 of bit2
    // is required clear by every test below, so the sign only decides branches
    // that are not taken. Read directly off tile->bit2, not a cached local:
    // the image re-reads bit2 from memory for the landmark_bonus mask below,
    // rather than keeping this value live across the bonus_at() call.
    int code = tile->bit2 >> 24;
    // Bonus id 2 is the mineral bonus.
    BOOL has_mineral_bonus = bonus_at(x, y, 0) == 2;
    tile = yield_tile(x, y);
    BOOL landmark_bonus =
        (((tile->bit2 & (BIT2_UNK_80000000 | BIT2_VOLCANO)) == BIT2_VOLCANO)
            && code < 9)
        || (((tile->bit2 & (BIT2_UNK_80000000 | BIT2_CRATER)) == BIT2_CRATER)
            && code < 9)
        || (((tile->bit2 & (BIT2_UNK_80000000 | BIT2_FOSSIL)) == BIT2_FOSSIL)
            && code < 6)
        || ((tile->bit2 & (BIT2_UNK_80000000 | BIT2_CANYON)) == BIT2_CANYON);
    int mineral = (has_mineral_bonus ? ResourceInfo[RSCINFO_BONUS_SQ].minerals
                                     : 0)
                  + (landmark_bonus ? 1 : 0);
    BOOL restrict_yield = false;

    if (yield_tile_owner(tile) >= 0) {
        mineral += ResourceInfo[RSCINFO_BASE_SQ].minerals;
        if (base_id < 0) {
            return mineral; // the original returns here rather than reaching the tail
        }
        if (has_fac(FAC_RECYCLING_TANKS, base_id, 0)
            || has_fac(FAC_PRESSURE_DOME, base_id, 0)) {
            mineral += ResourceInfo[RSCINFO_RECYCLING_TANKS].minerals;
        }
    } else if (bit & BIT_MONOLITH) {
        // Assignment, not addition: the bonus and landmark prefix is discarded.
        mineral = ResourceInfo[RSCINFO_MONOLITH].minerals;
        if (has_mineral_bonus) {
            mineral += ResourceInfo[RSCINFO_BONUS_SQ].minerals;
        }
        if (has_project(SP_MANIFOLD_HARMONICS, faction_id)
            && PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::PLANET] > 1) {
            mineral++;
        }
    } else if (bit & BIT_THERMAL_BORE) {
        mineral = ResourceInfo[RSCINFO_BOREHOLE_SQ].minerals;
        if (has_mineral_bonus) {
            mineral += ResourceInfo[RSCINFO_BONUS_SQ].minerals;
        }
        // Unlike crop_yield's borehole arm, this one does reach the restriction.
        restrict_yield = true;
    } else if ((bit & BIT_FUNGUS)
               && (tile->climate & ~0x1F) >= ALT_BIT_OCEAN_SHELF) {
        // Note there is no assume_improved suppressor here; crop_yield has one.
        int planet = PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::PLANET];
        int planet_mod = (planet >= -3) ? ((planet > 0) ? 0 : planet) : -3;
        mineral = range(PlayersData[faction_id].tech_fungus_[RSC_MINERALS] + planet_mod,
                        0, 99);
        if (has_project(SP_MANIFOLD_HARMONICS, faction_id) && planet > 1) {
            mineral++;
        }
    } else if ((tile->climate & ~0x1F) < ALT_BIT_SHORE_LINE) {
        // Not hoisted: the image computes this test lazily, right here, rather
        // than caching it in a local up front (unlike crop_yield/energy_yield,
        // which read climate for is_ocean_tile before this branch chain).
        restrict_yield = true;
        mineral += ResourceInfo[RSCINFO_OCEAN_SQ].minerals;
        // Legacy: the aquatic bonus keys off the faction owning BASE_ID, not off
        // faction_id, and is unguarded for base_id < 0 (0x004E75E2).
        int base_owner = Bases[base_id].faction_id_current;
        if ((Players[base_owner].rule_flags & RFLAG_AQUATIC)
            && (tile->climate & ~0x1F) == ALT_BIT_OCEAN_SHELF) {
            mineral++;
        }
        if ((tile->climate & ~0x1F) == ALT_BIT_OCEAN_SHELF || ExpansionEnabled) {
            if ((bit & BIT_MINE) || assume_improved) {
                mineral += ResourceInfo[RSCINFO_IMPROVED_SEA].minerals;
                if (has_tech(Rules.tech_mining_platform_bonus, faction_id)) {
                    mineral++;
                }
                // Legacy: unguarded for base_id < 0, exactly as 0x004E765F.
                if (has_fac(FAC_SUBSEA_TRUNKLINE, base_id, 0)) {
                    mineral++;
                }
            }
        }
    } else {
        restrict_yield = true;
        int rockiness = tile->val3 >> 6;
        if (bit & BIT_FOREST) {
            mineral += ResourceInfo[RSCINFO_FOREST_SQ].minerals;
        } else if ((bit & (BIT_MINE | BIT_THERMAL_BORE)) || assume_improved) {
            mineral += rockiness;
            int extra = rockiness ? rockiness : 1;
            if (has_mineral_bonus || landmark_bonus) {
                extra++;
            }
            if (!(bit & BIT_ROAD) && !assume_improved) {
                int limit = (int)Rules.limit_mineral_mine_sans_road;
                if (extra > limit) {
                    TileYieldRestricted += extra - limit;
                    extra = limit;
                }
            }
            mineral += extra;
        } else if (rockiness) {
            mineral++;
        }
    }

    if (restrict_yield && mineral > 2
        && !has_tech(Rules.tech_three_minerals_sqr, faction_id)
        && !has_mineral_bonus) {
        TileYieldRestricted += mineral - 2;
        mineral = 2;
    }

    if (base_id >= 0) {
        uint32_t event = Bases[base_id].event;
        if (event & BEVENT_INDUSTRY) {
            mineral++;
        }
        if ((event & BEVENT_BUST) && mineral) {
            mineral--;
        }
    }
    return mineral;
}

/*
Purpose: Calculate the energy produced by a single map square.
// ORIGINAL: 0x004E7750 ?energy_yield@@YAHHHHHH@Z 0x004E7750-0x004E7DA3
// LEVER: same bit2 shape as mine_yield - cached to a named local, but the
//        image re-reads tile->bit2 from memory for the landmark_bonus mask
//        (the register holding it was already destroyed by the >>24 shift
//        for `code`, and does not survive the bonus_at() call either way).
//        Fixed: agreeing instructions 4->5 of 536 (/Oy-), and `sub esp,0x18`
//        now matches the image exactly (both sides, 6 stack-resident
//        locals) - the frame allocation itself is no longer the gap here.
// TRIED: the remaining divergence (instr 3) is prologue INSTRUCTION
//            SCHEDULING - the image multiplies width by a direct memory
//            operand on y (`imul eax, [ebp+0x14]`) and only loads y into a
//            register later, right before the bonus_at() call; this tree's
//            body loads y into a register immediately. Not reachable from
//            C source order in the time spent; it is a backend scheduling
//            choice, not a statement-order one.
// size      1619 bytes
// prototype int (__cdecl ?energy_yield@@YAHHHHHH@Z)(int factionID, int, int, int, int)
// callers   8   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0050BA00 0x00592030 0x005B9F20
// LEVER: yield_tile/yield_tile_owner (this file) marked `static __forceinline`
//        - shared with crop_yield/mine_yield/base_support, a clear win for
//        base_support (0.160 -> 0.371). Tied here (best-across-flags stays
//        0.137, won by an /Ob0 flag set where yield_tile already folded
//        regardless) - kept anyway since it does not regress.
// TRIED: a `bonus_at_call` non-inline forwarder (map.h/map.cpp, since
//            removed) so bonus_at(x, y, 0) would emit `call 0x592030` -
//            measured no improvement on top of the forceinline change.
// LEVER: FEWER (2 calls vs image's 8: 6 bitmask() + bonus_at + has_tech) -
//        all six `has_fac_built(FAC_X, base_id)` sites constant-fold their
//        offset/mask math away under this toolchain, unlike the image, which
//        keeps bitmask() out-of-line at every one. Switching all six to
//        `has_fac_built_call` (base.h's bitmask_call-forcing sibling) took
//        call_diff to 0 disagreeing with no similarity regression (still
//        0.137 best-across-flags, matching the tied score noted above).
Return Value: Energy from the square
Status: Complete

`assume_improved` stands in for a solar collector on land and a tidal harness at
sea.

The fungus arm deliberately skips the whole shared tail - river, resource bonus,
landmark, Merchant Exchange, economy and the clip to 2 - by jumping from
0x004E7AD2 straight to the base event tail at 0x004E7D4B. That is what the bytes
do, not a transcription slip.
*/
int __cdecl energy_yield(int faction_id, int base_id, int x, int y,
                         BOOL assume_improved) {
    int energy = 0;
    Map *tile = yield_tile(x, y);
    uint32_t bit = tile->bit;
    // Not cached: bit2 is read directly off tile->bit2 at both use sites
    // below (here, shifted for `code`, and again in landmark_bonus). The
    // image re-reads it from memory the second time rather than keeping the
    // pre-shift value live across the bonus_at() call - same shape as
    // mine_yield's borehole/landmark computation.
    int code = tile->bit2 >> 24;
    BOOL is_ocean_tile = (tile->climate & ~0x1F) < ALT_BIT_SHORE_LINE;
    BOOL has_base_in_tile = yield_tile_owner(yield_tile(x, y)) >= 0;
    // Bonus id 3 is the energy bonus.
    BOOL has_energy_bonus = bonus_at(x, y, 0) == 3;
    tile = yield_tile(x, y);
    BOOL landmark_bonus = false;
    if ((((tile->bit2 & (BIT2_UNK_80000000 | BIT2_VOLCANO)) == BIT2_VOLCANO)
            && code < 9)
        || ((tile->bit2 & (BIT2_UNK_80000000 | BIT2_URANIUM)) == BIT2_URANIUM)
        || ((tile->bit2 & (BIT2_UNK_80000000 | BIT2_GEOTHERMAL)) == BIT2_GEOTHERMAL)
        || ((tile->bit2 & (BIT2_UNK_80000000 | BIT2_RIDGE)) == BIT2_RIDGE)) {
        landmark_bonus = true;
    }
    BOOL skip_shared_tail = false;

    if (has_base_in_tile) {
        energy = ResourceInfo[RSCINFO_BASE_SQ].energy;
        // Legacy: read BEFORE the base_id sign check at 0x004E7896, so a base_id
        // of -1 reads one Base entry below the array, exactly as the original.
        if (has_fac(FAC_HEADQUARTERS, base_id, 0)) {
            energy++;
        }
        if (base_id >= 0
            && (has_fac(FAC_RECYCLING_TANKS, base_id, 0)
                || has_fac(FAC_PRESSURE_DOME, base_id, 0))) {
            energy += ResourceInfo[RSCINFO_RECYCLING_TANKS].energy;
        }
        int economy = PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::ECONOMY]
                      + ((base_id >= 0 && (Bases[base_id].state & BSTATE_GOLDEN_AGE_ACTIVE))
                             ? 1 : 0);
        if (economy < 0) {
            // The original asks for Headquarters again here rather than reusing
            // the answer above.
            if (!has_fac(FAC_HEADQUARTERS, base_id, 0)
                || economy < -1
                || PlayersData[faction_id].current_num_bases == 1u) {
                economy++;
            }
        } else if (economy > 1) {
            economy = range(economy, 2, 4) * 2 - 4;
        }
        energy += economy;
        if (GameDataStateStorage.fields_.governor_faction_ == faction_id) {
            energy++;
        }
    } else if (bit & BIT_MONOLITH) {
        energy = ResourceInfo[RSCINFO_MONOLITH].energy;
        if (has_project(SP_MANIFOLD_HARMONICS, faction_id)) {
            int planet = PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::PLANET];
            if (planet >= 0) {
                energy++;
            }
            if (planet >= 3) {
                energy++;
            }
        }
    } else if (bit & BIT_THERMAL_BORE) {
        energy = ResourceInfo[RSCINFO_BOREHOLE_SQ].energy;
    } else if ((bit & BIT_FUNGUS)
               && (tile->climate & ~0x1F) >= ALT_BIT_OCEAN_SHELF) {
        int planet = PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::PLANET];
        int planet_mod = (planet >= -3) ? ((planet > 0) ? 0 : planet) : -3;
        energy = range(PlayersData[faction_id].tech_fungus_[RSC_ENERGY] + planet_mod,
                       0, 99);
        if (has_project(SP_MANIFOLD_HARMONICS, faction_id)) {
            if (planet >= 0) {
                energy++;
            }
            if (planet >= 3) {
                energy++;
            }
        }
        skip_shared_tail = true;
    } else if (is_ocean_tile) {
        if ((tile->climate & ~0x1F) == ALT_BIT_OCEAN_SHELF || ExpansionEnabled) {
            energy = ResourceInfo[RSCINFO_OCEAN_SQ].energy;
            if ((bit & BIT_SOLAR_TIDAL) || assume_improved) {
                energy += ResourceInfo[RSCINFO_IMPROVED_SEA].energy;
                // Legacy: unguarded for base_id < 0, exactly as 0x004E7B19.
                if (has_fac(FAC_THERMOCLINE_TRANSDUCER, base_id, 0)) {
                    energy++;
                }
            }
        }
        if (bit & BIT_FOREST) {
            energy--;
        }
    } else if (bit & BIT_FOREST) {
        energy = ResourceInfo[RSCINFO_FOREST_SQ].energy;
        if (base_id >= 0 && has_fac(FAC_HYBRID_FOREST, base_id, 0)) {
            energy++;
        }
    } else {
        // Altitude levels above 2.
        int solar = (tile->climate >> 5) - 2;
        solar = (solar < 0) ? 0 : solar;
        energy = ((bit & (BIT_SOLAR_TIDAL | BIT_ECH_MIRROR | BIT_THERMAL_BORE))
                  || assume_improved) ? solar : 0;
        if (bit & BIT_SOLAR_TIDAL) {
            // Each adjacent echelon mirror belonging to the current base's
            // faction reflects one more energy onto this collector.
            for (int i = 0; i < 8; i++) {
                int x_adj = wrap_map_x(x + VehMoveDeltaX[i]);
                int y_adj = y + VehMoveDeltaY[i];
                if (!on_map(x_adj, y_adj)) {
                    continue;
                }
                Map *adjacent = yield_tile(x_adj, y_adj);
                if (!(adjacent->bit & BIT_ECH_MIRROR)) {
                    continue;
                }
                if (adjacent->territory
                    != (int)BaseCurrent->faction_id_current) {
                    continue;
                }
                energy++;
            }
        }
    }

    if (!skip_shared_tail) {
        if ((bit & BIT_RIVER) && !is_ocean_tile) {
            energy++;
        }
        if (has_energy_bonus) {
            energy += ResourceInfo[RSCINFO_BONUS_SQ].energy;
        }
        if (landmark_bonus) {
            energy++;
        }
        if (base_id >= 0
            && GameDataStateStorage.fields_.secret_project_.merchant_exchange() == base_id) {
            energy++;
        }
        if (PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::ECONOMY]
                + ((base_id >= 0 && (Bases[base_id].state & BSTATE_GOLDEN_AGE_ACTIVE)) ? 1 : 0)
            >= 2) {
            energy++;
        }
        if (has_base_in_tile) {
            energy = range(energy, 0, 99);
            BaseSquareEnergy = energy;
        } else if (energy > 2
                   && !has_tech(Rules.tech_three_energy_sqr, faction_id)
                   && !has_energy_bonus) {
            TileYieldRestricted += energy - 2;
            energy = 2;
        }
    }

    if (base_id >= 0) {
        uint32_t event = Bases[base_id].event;
        if (event & BEVENT_HEAT_WAVE) {
            energy++;
        }
        if ((event & BEVENT_CLOUD_COVER) && energy) {
            energy--;
        }
    }
    if (GameDataStateStorage.fields_.dust_cloud_duration_ && energy) {
        energy--;
    }
    int event_active = UnkGlobal0093A934 ? (GameDataStateStorage.fields_.global_energy_event_state_ & 2)
                                          : (GameDataStateStorage.fields_.global_energy_event_state_ & 1);
    return event_active ? energy * 3 : energy;
}

/*
// ORIGINAL: 0x004E7DB0 sub_4e7db0 0x004E7DB0-0x004E7DBD BYTE_EXACT
// symbol    ?sub_4e7db0@@YAHXZ
// size      13 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: zero
Status: Complete
*/
int __cdecl sub_4e7db0() {
    TileYieldRestricted = 0;
    return 0;
}

// BODY IN base.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.



/*
Purpose: Check if the base already has a particular facility built.
Original Offset: n/a
Return Value: Does base have facility built? true/false
Status: Complete
*/
// BODY IN base.h: eleven bodies call it where the image calls nothing -
// `call_diff` names it - so VC6 has to be able to inline it.



// RECOVERY_ASSOCIATED: v1 switch_targets 0x004E7DC0 0x004E7E34 0x004E7E44 0x004E7DCB
// ORIGINAL: 0x004E7DC0 ?resource_yield@@YAHHHHHH@Z 0x004E7DC0-0x004E7E34 FILE BYTE_EXACT
// LEVER: TileYieldRestricted (0x0090E998, already declared in base.h) replaces
//        the carried `g_0090e998` binding; the scaffold's `g_004e7e34` was the
//        function's own end address, referenced by nothing, and is dropped.
// size      116 bytes
// prototype int (__cdecl ?resource_yield@@YAHHHHHH@Z)(int type, int factionID, int baseID, int xCoord, int yCoord)
// callers   7   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004E6E50 0x004E7310 0x004E7750
// Purpose: Dispatch one of the three terrain yields for the base's current
//          square, or - for RSC_PSI (3), which has no per-square yield -
//          clear TileYieldRestricted and return 0.
// Status:  transcription homed from src/unrecovered/004e7dc0.cpp

int __cdecl resource_yield(int rsc_type, int faction_id, int base_id, int x,
                           int y) {
    switch (rsc_type) {
        case 0:
            return crop_yield(faction_id, base_id, x, y, 0);
        case 1:
            return mine_yield(faction_id, base_id, x, y, 0);
        case 2:
            return energy_yield(faction_id, base_id, x, y, 0);
        case 3:
            TileYieldRestricted = 0;
            return 0;
    }
    return 0;
}

// RECOVERY_ASSOCIATED: v1 switch_targets 0x004E7E50 0x004E7F64 0x004E7F74 0x004E7EF7
/*
Purpose: Sum one base's terrain yields into the selected Base's intake_2
         slots: walk the base's centre plus its radius offsets, wrap the x
         edge on a flat map, latch the 0x8000 out-of-bounds state bit when
         the tile is non-fungal rock, and add each of the four yield
         passes' result into BaseCurrent's nutrient/mineral/energy/unused
         intake_2 word, returning the sum.

// ORIGINAL: 0x004E7E50 sub_4e7e50 0x004E7E50-0x004E7F64 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence":["The body reads the existing BaseCurrent/Bases layout (x, y, faction, state and contiguous intake_2 words), Map records, map bounds, and declared crop_yield/mine_yield/energy_yield APIs.","The function's fourth pass deliberately clears TileYieldRestricted; the body and original pass order are carried unchanged.","The identity remains the unmangled global sub_4e7e50; no original class name is inferred from its Base dataflow."],"reason":"Place the complete existing base-radius yield routine beside its genuine Base/Map dependencies without renaming it, changing pass order, or normalizing the original state updates."}
// symbol    ?sub_4e7e50@@YAHH@Z
// TRIED: various expression orderings for the x/y base+offset sums and the wrap-around guard placement all keep the divergence at the first instructions (register allocation for edx/ecx/a1 differs from the original's push-ebx/push-esi-before-movsx order).
// size      276 bytes
// prototype
// callers   0   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004E6E50 0x004E7310 0x004E7750
Return Value: the four-pass yield sum
Status: Coverage transcription - unclaimed body
*/
// stand-in: base_yield expands this body inline while the image still emits
// it standalone (callers 0); VC6 will not auto-inline the extern function
// there, so the body lives in an inline helper both use.
static inline int radius_yield_add(int radius_index) {
    Base *base = BaseCurrent;
    int x = wrap_map_x(base->x + RadiusOffsetX[radius_index]);
    int y = base->y + RadiusOffsetY[radius_index];
    if (!on_map(x, y)) {
        return 0;
    }
    if ((map_loc(x, y)->climate & ~0x1F) < 0x60) {
        base->state |= 0x8000;
    }
    int sum = 0;
    for (int rsc = 0; rsc < 4; ++rsc) {
        int value = resource_yield(rsc, BaseCurrent->faction_id_current, BaseIDCurrentSelected, x, y);
        BaseCurrent->intake_2[rsc] += value;
        sum += value;
    }
    return sum;
}

int __cdecl sub_4e7e50(int radius_index) {
    return radius_yield_add(radius_index);
}

// RECOVERY_ASSOCIATED: v1 switch_targets 0x004E7F80 0x004E8094 0x004E80A4 0x004E8027
/*
Purpose: The sibling pass of sub_4e7e50: identical walk and tile gating,
         but each pass's yield is SUBTRACTED from the matching intake_2
         word while the same value still accumulates into the sum.

// ORIGINAL: 0x004E7F80 sub_4e7f80 0x004E7F80-0x004E8094 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence":["The body reads the existing BaseCurrent/Bases layout, Map records, map bounds, and declared crop_yield/mine_yield/energy_yield APIs.","This is the subtracting sibling of 0x004E7E50; the original sum still accumulates positive yield values while intake_2 is decremented.","The identity remains the unmangled global sub_4e7f80; no original class name is inferred from its Base dataflow."],"reason":"Place the complete subtracting base-radius yield routine beside its genuine Base/Map dependencies, preserving its distinct subtraction and positive return-sum behavior."}
// symbol    ?sub_4e7f80@@YAHH@Z
// TRIED: the same prologue register-allocation divergence as its 0x004E7E50 sibling.
// size      276 bytes
// prototype
// callers   0   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004E6E50 0x004E7310 0x004E7750
Return Value: the negated-form four-pass yield sum
Status: Coverage transcription - unclaimed body
*/
// stand-in: base_yield expands this body inline while the image still emits
// it standalone (callers 0); VC6 will not auto-inline the extern function
// there, so the body lives in an inline helper both use.
static inline int radius_yield_sub(int radius_index) {
    Base *base = BaseCurrent;
    int x = wrap_map_x(base->x + RadiusOffsetX[radius_index]);
    int y = base->y + RadiusOffsetY[radius_index];
    if (!on_map(x, y)) {
        return 0;
    }
    if ((map_loc(x, y)->climate & ~0x1F) < 0x60) {
        base->state |= 0x8000;
    }
    int sum = 0;
    for (int rsc = 0; rsc < 4; ++rsc) {
        int value = resource_yield(rsc, BaseCurrent->faction_id_current, BaseIDCurrentSelected, x, y);
        BaseCurrent->intake_2[rsc] -= value;
        sum += value;
    }
    return sum;
}

int __cdecl sub_4e7f80(int radius_index) {
    return radius_yield_sub(radius_index);
}

// Recovery names (original names unknown): the packed specialist nibbles of
// the selected base. Slots past the sixteen stored nibbles report
// best_specialist(); writes past them are dropped. base_yield expands both
// inline; best_specialist stays a real call inside the getter's copies.
static inline int base_specialist_type(int index) {
    if (index >= 16) {
        return best_specialist();
    }
    return (BaseCurrent->spec[index >> 3] >> ((index & 7) * 4)) & 0xF;
}

static inline void base_specialist_set(int index, int type) {
    if (index < 16) {
        int word = index >> 3;
        BaseCurrent->spec[word] &= ~(0xF << ((index & 7) * 4));
        BaseCurrent->spec[word] |= type << ((index & 7) * 4);
    }
}
// WORKAROUND(inline): dead /Ob2 budget probe, not recovered code; a constant
// argument folds it away. base_yield spends this much more budget in the image:
// without it the radius_yield_add for best_position in the reassignment loop expands
// resource_yield, where the image keeps all seven resource_yield calls out of
// line (0x004E7DC0 has exactly seven callers, all in base_yield). The
// original's spend is not identified.
static inline int base_yield_budget_probe(int x) {
    if (x == 0) x = 1;
    if (x == 1) x = 4;
    if (x == 2) x = 7;
    if (x == 3) x = 10;
    if (x == 4) x = 13;
    if (x == 5) x = 16;
    if (x == 6) x = 19;
    if (x == 7) x = 22;
    if (x == 8) x = 25;
    if (x == 9) x = 28;
    if (x == 10) x = 31;
    if (x == 11) x = 34;
    if (x == 12) x = 37;
    if (x == 13) x = 40;
    if (x == 14) x = 43;
    if (x == 15) x = 46;
    return x;
}


// ORIGINAL: 0x004E80B0 ?base_yield@@YAXXZ 0x004E80B0-0x004E954B
// RECOVERY_BINDING: v1 {"evidence": ["git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/004e80b0.cpp", "source:src/base.h", "image:0x004E80CC-0x004E80D9 signed population plus one publishes90E938;0x004E86D9 resets90E900 only on allocation, whereas0x004E8456 jumps to0x004E923D and retains it.", "image:0x004E83D9-0x004E8426 four resource calls per admitted tile;0x004E8E0C-0x004E923D performs one replacement with fresh-call undo;0x004E930C-0x004E954B publishes mask/specialists/intakes and accounts for all21valid radius positions."], "reason": "Recover the complete selected-base worker and specialist allocation pass with typed resource fields, real shared counters, original selection and replacement behavior, and final ecology accounting. Retain its seed identity and evidence; the former link-only placeholder is replaced, not accepted by a source binding."}
// TRIED: literal register-machine transliteration (pseudo-regs eax..edi + stack slots v04..v58, goto per basic block); covers ~100% of instructions (5804 rebuilt vs 5275 original bytes). cdq/sub/sar-1 divide-by-2 idioms folded to `/2`; neg/sbb/not boolean tricks folded to ternaries; magic-const /3 folded to `/3`. Divergence is from stack/local layout (more temporaries than the original's register allocation), not from control flow or field offsets.
void __cdecl base_yield() {
    base_yield_budget_probe(2);  // WORKAROUND(inline): see base_yield_budget_probe
    farm_compute();
    int faction = BaseCurrent->faction_id_current;
    BasePopulationUnassigned = BaseCurrent->population_size + 1;
    BOOL automatic = !is_human(faction)
        || ((BaseCurrent->governor & GOV_MANAGES_CITIZENS_SPECS) && (BaseCurrent->governor & GOV_ACTIVE));
    uint32_t worked = 0;
    BaseCurrent->state &= ~BSTATE_UNK_8000;
    BaseCurrent->specialist_unk_1 = 0;
    BaseCurrent->resource_sqr_radius &= ~FarmUnavailableMask | 1;

    // VC6 for-scope: this `i` lives to the end of the function. The image
    // keeps it in one slot ([ebp-4]) for this loop, both specialist loops,
    // the radius scan and the tail loops; only the worked-square reuse loop
    // has its own counter.
    for (int i = 0; i < 4; i++) {
        if (i < 3) {
            BaseCurrent->intake_2[i] = PlayersData[faction].satellites_[i];
            if (!has_fac(FAC_AEROSPACE_COMPLEX, BaseIDCurrentSelected, 0)
                && !has_project(SP_SPACE_ELEVATOR, faction)
                && !has_project(SP_CLOUDBASE_ACADEMY, faction)) {
                BaseCurrent->intake_2[i] = (BaseCurrent->intake_2[i] + 1) / 2;
            }
            BaseCurrent->intake_2[i] = (BaseCurrent->population_size < BaseCurrent->intake_2[i])
                ? BaseCurrent->population_size : BaseCurrent->intake_2[i];
        } else {
            BaseCurrent->intake_2[i] = 0;
        }
    }

    // Reuse the base's previous worked squares when every one of them can be staffed.
    if (!automatic || !UnkGlobal0093A934) {
        if (automatic) {
            for (i = 0; i < BaseCurrent->specialist_total; i++) {
                if (!Citizen[base_specialist_type(i)].bonus[1]) {
                    BaseCurrent->specialist_unk_1++;
                }
            }
            BaseCurrent->specialist_total -= BaseCurrent->specialist_unk_1;
        }
        if (BasePopulationUnassigned >= bit_count(BaseCurrent->resource_sqr_radius)) {
            for (int i = 0; i < 21; i++) {
                if (!i || (BaseCurrent->resource_sqr_radius & (1 << i))) {
                    worked |= 1 << i;
                    radius_yield_add(i);
                    if (!--BasePopulationUnassigned) {
                        break;
                    }
                }
            }
            // BaseNutrientAdequate deliberately keeps its previous value on this path.
            if (BasePopulationUnassigned
                == BaseCurrent->specialist_unk_1 + BaseCurrent->specialist_total) {
                goto assign_specialists;
            }
        }
    }

    // Automatic bases release every specialist without a psych bonus.
    // The compaction shifts through the old end slot and then advances the
    // outer index without re-testing the slot that moved into it.
    if (automatic && BaseCurrent->specialist_total) {
        for (i = 0; i < BaseCurrent->specialist_unk_1 + BaseCurrent->specialist_total; i++) {
            if (!Citizen[base_specialist_type(i)].bonus[1]) {
                for (int j = i; j < BaseCurrent->specialist_unk_1 + BaseCurrent->specialist_total; j++) {
                    base_specialist_set(j, base_specialist_type(j + 1));
                }
                if (BaseCurrent->specialist_unk_1) {
                    BaseCurrent->specialist_unk_1--;
                }
            }
        }
    }
    if (!(worked & 1)) {
        radius_yield_add(0);
        worked |= 1;
        BasePopulationUnassigned--;
    }

    BaseNutrientAdequate = 0;
    int fight;
    int growth;
    int tech;
    int wealth;
    int power;
    if (is_human(faction) && (BaseCurrent->governor & GOV_ACTIVE)) {
        growth = (BaseCurrent->governor >> 24) & 1;
        tech = (BaseCurrent->governor >> 24) & 2;
        wealth = (BaseCurrent->governor >> 25) & 2;
        power = (BaseCurrent->governor >> 26) & 2;
        if (power) {
            fight = 2;
        } else if (!growth && !wealth) {
            fight = tech ? -2 : PlayersData[faction].ai_fight;
        } else {
            fight = 0;
        }
    } else if (is_human(faction)) {
        fight = 0;
        power = 0;
        wealth = 0;
        tech = 0;
        growth = 0;
    } else {
        growth = PlayersData[faction].ai_growth;
        wealth = PlayersData[faction].ai_wealth;
        tech = PlayersData[faction].ai_tech;
        power = PlayersData[faction].ai_power;
        fight = PlayersData[faction].ai_fight;
    }

    for (;;) {
        BOOL replacing = BasePopulationUnassigned
            <= BaseCurrent->specialist_unk_1 + BaseCurrent->specialist_total;
        int best_position = 0;
        int best_score = 0;
        int max_nutrients = 0;
        int worst_score = 9999;
        int worst_position = -1;
        uint32_t excluded = replacing ? 0 : worked;
        for (i = 1; i < 21; i++) {
            uint32_t bit = 1 << i;
            if (bit & (excluded | FarmUnavailableMask)) {
                continue;
            }
            int x = wrap_map_x(BaseCurrent->x + RadiusOffsetX[i]);
            int y = BaseCurrent->y + RadiusOffsetY[i];
            if (!on_map(x, y)) {
                continue;
            }
            int nutrients = crop_yield(BaseCurrent->faction_id_current, BaseIDCurrentSelected, x, y, 0);
            int minerals = mine_yield(BaseCurrent->faction_id_current, BaseIDCurrentSelected, x, y, 0);
            int energy = energy_yield(BaseCurrent->faction_id_current, BaseIDCurrentSelected, x, y, 0);
            if (nutrients > max_nutrients) {
                max_nutrients = nutrients;
            }
            int food_bonus = 0;
            int needed = BaseCurrent->population_size * Rules.nutrient_req_citizen;
            if (BasePopulationUnassigned / Rules.nutrient_req_citizen + BaseCurrent->intake_2[0] < needed) {
                food_bonus = (BaseCurrent->intake_2[0] + BasePopulationUnassigned >= needed) ? 2 : 8;
                if (food_bonus) {
                    nutrients += range(nutrients, 0, food_bonus);
                }
            } else {
                BaseNutrientAdequate = 1;
            }
            nutrients *= 2;
            minerals *= 2;
            energy *= 2;
            if (BaseCurrent->intake_2[0] <= BaseCurrent->population_size + needed && !nutrients) {
                minerals /= 2;
                energy /= 2;
            }
            if (!minerals && !food_bonus) {
                nutrients /= 2;
            }
            int score = (growth + 4) * nutrients * 256
                / range((BaseCurrent->intake_2[0] - BaseCurrent->population_size / 3 - needed
                         + BasePopulationUnassigned) * 2 - 2, 1, 99);
            int mineral_weight = (BaseCurrent->queue_production_id[0] < -70) ? 2 : -tech;
            int mineral_divisor = range((BaseCurrent->intake_2[1]
                - BaseCurrentForcesMaintCost + 1) / 2, 1, 99);
            score += (fight + mineral_weight + power + wealth + 6)
                * BaseCurrent->population_size * minerals * 64 / mineral_divisor;
            score += (wealth + tech * 2 + 1) * range(BaseCurrent->population_size - power, 1, 99)
                * energy * 64 / range(BaseCurrent->intake_2[2], 1, 99);
            if (worked & bit) {
                if (score < worst_score) {
                    worst_score = score;
                    worst_position = i;
                }
            } else if (score > best_score) {
                best_score = score;
                best_position = i;
            }
        }

        // Large, well-fed automatic bases late in the game also weigh specialists.
        if (automatic && BaseCurrent->population_size >= 5
            && (BaseCurrent->intake_2[0] > BaseCurrent->population_size * Rules.nutrient_req_citizen
                || !max_nutrients)
            && GameDataStateStorage.fields_.turn_current_num_ > 100) {
            for (int s = 0; s < MaxSpecialistNum; s++) {
                if (has_tech(Citizen[s].preq_tech, faction) && !has_tech(Citizen[s].obsol_tech, faction)) {
                    // Dead in the original: only an empty walk over factions 1..7 survives.
                    int ranking_total = 0;
                    for (int other = 1; other < MaxPlayerNum; other++) {
                        ranking_total += PlayersData[other].tech_ranking;
                    }
                    int score = Citizen[s].bonus[1] * 64
                        + (wealth + 2) * BaseCurrent->population_size * Citizen[s].bonus[0] * 64
                            / range(BaseCurrent->specialist_unk_1 + BaseCurrent->intake_2[2], 1, 99)
                        + (tech + 2) * BaseCurrent->population_size * Citizen[s].bonus[2] * 64
                            / range(BaseCurrent->specialist_unk_1 + BaseCurrent->intake_2[2], 1, 99);
                    if (score > best_score) {
                        best_score = score;
                        best_position = -s - 1;
                    }
                }
            }
        }

        if (!replacing) {
            if (best_position < 0) {
                base_specialist_set(BaseCurrent->specialist_unk_1, -best_position - 1);
                BaseCurrent->specialist_unk_1++;
            } else if (best_position > 0) {
                worked |= 1 << best_position;
                radius_yield_add(best_position);
                BasePopulationUnassigned--;
            } else {
                break;
            }
            if (BasePopulationUnassigned <= BaseCurrent->specialist_unk_1 + BaseCurrent->specialist_total
                && BaseCurrent->intake_2[0] >= BaseCurrent->population_size * Rules.nutrient_req_citizen) {
                break;
            }
        } else {
            // Every worker is placed: try one swap of the worst worked square
            // for the best free one, undone if the base would stop growing.
            if (best_position > 0 && best_score > worst_score && worst_position > 0) {
                uint32_t worst_bit = 1 << worst_position;
                uint32_t best_bit = 1 << best_position;
                worked = (worked & ~worst_bit) | best_bit;
                radius_yield_sub(worst_position);
                radius_yield_add(best_position);
                if (BaseCurrent->intake_2[0]
                    < (BaseCurrent->population_size + 1) * Rules.nutrient_req_citizen) {
                    worked = (worked & ~best_bit) | worst_bit;
                    radius_yield_sub(best_position);
                    radius_yield_add(worst_position);
                }
            }
            break;
        }
    }

assign_specialists:
    for (i = BaseCurrent->specialist_total; i < BasePopulationUnassigned; i++) {
        base_specialist_set(i, best_specialist());
    }
    BaseCurrent->resource_sqr_radius = worked;
    BaseCurrent->specialist_total = BasePopulationUnassigned;
    BaseCurrent->state &= ~BSTATE_UNK_100;
    for (i = 0; i < 4; i++) {
        BaseCurrent->intake_1[i] = BaseCurrent->intake_2[i];
    }

    BaseCurrent->eco_damage = 0;
    for (i = 0; i < 21; i++) {
        int x = wrap_map_x(BaseCurrent->x + RadiusOffsetX[i]);
        int y = BaseCurrent->y + RadiusOffsetY[i];
        if (!on_map(x, y)) {
            continue;
        }
        uint32_t bits = map_loc(x, y)->bit;
        int is_worked = BaseCurrent->resource_sqr_radius & (1 << i);
        int damage = bit_count(bits & 0x1C8805C);
        if (is_worked) {
            damage *= 2;
        }
        if (bits & BIT_THERMAL_BORE) {
            damage += 8;
        }
        if (bits & BIT_ECH_MIRROR) {
            damage += 6;
        }
        if (bits & BIT_CONDENSER) {
            damage += 4;
        }
        if (bits & BIT_FOREST) {
            damage--;
        }
        if (has_fac(FAC_TREE_FARM, BaseIDCurrentSelected, 0)
            && has_fac(FAC_HYBRID_FOREST, BaseIDCurrentSelected, 0)) {
            damage = 0;
        }
        if (has_fac(FAC_TREE_FARM, BaseIDCurrentSelected, 0)
            || has_fac(FAC_HYBRID_FOREST, BaseIDCurrentSelected, 0)) {
            damage /= 2;
        }
        BaseCurrent->eco_damage += damage;
    }
}

// RECOVERY_ASSOCIATED: v1 switch_targets 0x004E9550 0x004E9B4C 0x004E9B5C 0x004E96E1
/*
Purpose: Tally what the current base's forces cost it: the resources its supply
         convoys move in and out, the units it supports, the minerals their
         maintenance takes, and the pacifism drones they cause.
// ORIGINAL: 0x004E9550 ?base_support@@YAXXZ 0x004E9550-0x004E9B4A
// LEVER: the FAC_BROOD_PIT police-modifier check was `has_fac_built(...)`,
//        which constant-folds to a bit test here (same systemic gap as
//        has_fac/base_nutrient/base_minerals - see their notes). Swapping it
//        for `has_fac_built_call(...)` (base.h, forces bitmask_call's real
//        E8) took call_diff from FEWER by 2 to FEWER by 1 and best-across-
//        flags similarity 0.371 -> 0.431 (45/465 raw agreeing, /O2 /Gy /GR- /GX).
// TRIED: hoisting `Base *base_current = BaseCurrent();` above the loop -
//            already ruled out for a sibling function a few hundred lines up
//            in this file (see the TRIED note near line 396): the image
//            re-reads the BaseCurrent() global fresh at every use, so this
//            was not retried here. Under /Oy-, `sub esp` is 0x34 against the
//            image's 0x3c (one 8-byte gap, i.e. two stack slots) - closer
//            than the auto-picked flag set's 0x2c, but no specific local was
//            identified as the source of the gap in the time spent on a
//            465-instruction body with ~20 live locals across the veh loop.
// size      1530 bytes
// prototype
// callers   7   call targets   6
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004E3EF0 0x004E6E50 0x004E7310 0x004E7750 0x0050BA00 0x005BF1F0
// LEVER: unrolled the `for (rsc=0..3) { ConvoyTo[rsc]=0; ConvoyFrom[rsc]=0; }`
//        loop into eight explicit assignments, keeping the pair order
//        (ConvoyTo before ConvoyFrom) the loop already used - the image
//        writes [0x90e93c] before [0x90e904], [0x90e940] before [0x90e908],
//        and so on, fully unrolled. Combined with yield_tile/yield_tile_owner
//        becoming `static __forceinline` (this file, shared with
//        crop_yield/mine_yield/energy_yield), best similarity moved 0.129 ->
//        0.371 and agreeing instructions 9->14 of 465.
Return Value: n/a
Status: Complete

One pass over every Veh of the base's faction, splitting on whether the unit is
homed here. The five accumulators this zeroes are read by base_nutrient
(0x004E9B70), base_minerals (0x004E9CB0) and base_psych, which is why the whole
tally has to be finished before any of them run.

Two of the eight `free_support` entries are not `support + 2`: -4 and -3 both
give nothing, and +3 gives max(4, population), so the table is built rather than
computed. It is indexed by the SUPPORT rating offset by four, -4 at [0].

The original recomputes the Brood Pit police adjustment a second time in the
`police != -3` arm, with a second inlined bitmask() over the same facility and
the same base. Nothing between the two writes facilities_built, so it is written
once here. The three has_abil(ABL_CLEAN_REACTOR) calls are NOT collapsed: the
first one gates the whole support section, so the second and third can only ever
see false, but has_abil() logs on a negative prototype id and the call count is
therefore observable.
*/
void __cdecl base_support() {
    int faction_id = BaseCurrent->faction_id_current;
    for (int rsc = 0; rsc < 4; rsc++) {
        BaseCurrentConvoyTo[rsc] = 0;
        BaseCurrentConvoyFrom[rsc] = 0;
    }
    BaseCurrentVehPacifismCount = 0;
    BaseCurrentForcesMaintCount = 0;
    BaseCurrentForcesSupported = 0;
    BaseCurrentForcesMaintCost = 0;
    // Nine slots, eight used: the image's frame is 0x3C with the table at
    // ebp-0x3C and the ninth dword (ebp-0x1C) never touched.
    int free_support[9]; // units supported for free, by SUPPORT rating + 4
    for (int support = -4; support <= 3; support++) {
        if (support >= 3) {
            free_support[support + 4] = (BaseCurrent->population_size > 4)
                ? BaseCurrent->population_size : 4;
        } else if (support < -2) {
            free_support[support + 4] = 0;
        } else if (support == -2) {
            free_support[support + 4] = 1;
        } else {
            free_support[support + 4] = support + 2;
        }
    }
    for (int veh_id = 0; veh_id < GameDataStateStorage.fields_.vehicle_current_count_;
         veh_id++) {
        if (Vehs[veh_id].faction_id != faction_id) {
            continue;
        }
        if (Vehs[veh_id].home_base_id != BaseIDCurrentSelected) {
            // A convoy homed elsewhere standing on this base's tile counts one.
            if (VehPrototypes[Vehs[veh_id].proto_id].plan == PLAN_SUPPLY_CONVOY
                && Vehs[veh_id].x == BaseCurrent->x && Vehs[veh_id].y == BaseCurrent->y
                && Vehs[veh_id].order == ORDER_CONVOY && Vehs[veh_id].home_base_id >= 0) {
                BaseCurrentConvoyTo[Vehs[veh_id].order_auto_type]++;
            }
            continue;
        }
        if (VehPrototypes[Vehs[veh_id].proto_id].plan == PLAN_SUPPLY_CONVOY
            && base_who(Vehs[veh_id].x, Vehs[veh_id].y) < 0
            && Vehs[veh_id].order == ORDER_CONVOY) {
            int convoyed;
            switch (Vehs[veh_id].order_auto_type) {
              case RSC_NUTRIENTS:
                convoyed = crop_yield(faction_id, BaseIDCurrentSelected, Vehs[veh_id].x,
                                      Vehs[veh_id].y, false);
                break;
              case RSC_MINERALS:
                convoyed = mine_yield(faction_id, BaseIDCurrentSelected, Vehs[veh_id].x,
                                      Vehs[veh_id].y, false);
                break;
              case RSC_ENERGY:
                convoyed = energy_yield(faction_id, BaseIDCurrentSelected, Vehs[veh_id].x,
                                        Vehs[veh_id].y, false);
                break;
              case RSC_PSI:
                TileYieldRestricted = 0;
                convoyed = 0;
                break;
              default:
                convoyed = 0;
                break;
            }
            BaseCurrentConvoyTo[Vehs[veh_id].order_auto_type] += convoyed;
        }
        // AI offensive strength tally; psi weapons count only on custom prototypes.
        int8_t offense = Weapon[VehPrototypes[Vehs[veh_id].proto_id].weapon_id].offense_rating;
        if (offense && (offense > 0 || Vehs[veh_id].proto_id >= MaxVehProtoFactionNum)) {
            uint8_t tally_plan = VehPrototypes[Vehs[veh_id].proto_id].plan;
            if (tally_plan != PLAN_RECONNAISANCE && tally_plan != PLAN_PLANET_BUSTER) {
                PlayersData[faction_id].unk_48++;
                if (tally_plan == PLAN_OFFENSIVE || tally_plan == PLAN_COMBAT) {
                    PlayersData[faction_id].unk_48++;
                }
            }
        }
        Vehs[veh_id].state &= ~(VSTATE_REQUIRES_SUPPORT | VSTATE_PACIFISM_DRONE
                                | VSTATE_PACIFISM_FREE_SKIP);
        uint8_t plan = VehPrototypes[Vehs[veh_id].proto_id].plan;
        // A convoy of ours standing in any base is charged as consumption.
        if (VehPrototypes[Vehs[veh_id].proto_id].plan == PLAN_SUPPLY_CONVOY
            && Vehs[veh_id].order == ORDER_CONVOY
            && base_who(Vehs[veh_id].x, Vehs[veh_id].y) >= 0) {
            BaseCurrentConvoyFrom[Vehs[veh_id].order_auto_type]++;
        }
        if (plan > PLAN_TERRAFORMING) {
            continue; // convoys and above are never supported
        }
        if (!has_abil(Vehs[veh_id].proto_id, ABL_CLEAN_REACTOR)) {
            // Native life in fungus (psi weapon on a predefined prototype, or
            // prototype 15, the Sea Lurk) is supported for free.
            Map *tile;
            if (Vehs[veh_id].proto_id >= MaxVehProtoFactionNum
                || (Weapon[VehPrototypes[Vehs[veh_id].proto_id].weapon_id].offense_rating >= 0
                    && Vehs[veh_id].proto_id != 15)
                || !((tile = map_loc(Vehs[veh_id].x, Vehs[veh_id].y))->bit & BIT_FUNGUS)
                || (tile->climate & ~0x1F) < ALT_BIT_OCEAN_SHELF) {
                BaseCurrentForcesSupported++;
                int support = PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::SUPPORT];
                if (BaseCurrentForcesSupported > free_support[range(support + 4, 0, 7)]
                    && !has_abil(Vehs[veh_id].proto_id, ABL_CLEAN_REACTOR)) {
                    BaseCurrentForcesMaintCount++;
                    BaseCurrentForcesMaintCost += (support <= -4) + 1;
                    Vehs[veh_id].state |= VSTATE_REQUIRES_SUPPORT;
                }
            }
            // Cost of this unit at each of the eight SUPPORT ratings, for social_ai.
            if (BaseUpkeepStage == 1) {
                for (int support = -4; support <= 3; support++) {
                    if (BaseCurrentForcesSupported <= free_support[support + 4]) {
                        continue;
                    }
                    int cost = has_abil(Vehs[veh_id].proto_id, ABL_CLEAN_REACTOR)
                        ? 0 : (support <= -4) + 1;
                    int minerals = BaseCurrent->intake_2[RSC_MINERALS];
                    if (cost >= minerals) {
                        cost += (cost - minerals / 2) * 2;
                    } else if (cost > minerals / 2) {
                        cost = cost * 2 - minerals / 2;
                    }
                    PlayersData[faction_id].unk_38[support + 4] += cost;
                }
            }
        }
        // Pacifism: armed units away from home; air units (except interceptors)
        // count anywhere.
        if (!Weapon[VehPrototypes[Vehs[veh_id].proto_id].weapon_id].offense_rating) {
            continue;
        }
        if (base_who(Vehs[veh_id].x, Vehs[veh_id].y) >= 0
            || whose_territory(faction_id, Vehs[veh_id].x, Vehs[veh_id].y, NULL, false)
                == faction_id) {
            if (Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad != TRIAD_AIR
                || VehPrototypes[Vehs[veh_id].proto_id].plan == PLAN_AIR_SUPERIORITY) {
                continue;
            }
        }
        BaseCurrentVehPacifismCount++;
        // The original evaluates the police rating (with the Brood Pit bonus and
        // its inlined bitmask()) twice: once per arm.
        if (PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::POLICE]
                + has_fac(FAC_BROOD_PIT, BaseIDCurrentSelected, 0) * 2 == -3) {
            if (BaseCurrentVehPacifismCount == 1) {
                Vehs[veh_id].state |= VSTATE_PACIFISM_FREE_SKIP;
            } else {
                Vehs[veh_id].state |= VSTATE_PACIFISM_DRONE;
            }
        } else if (PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::POLICE]
                       + has_fac(FAC_BROOD_PIT, BaseIDCurrentSelected, 0) * 2 < -3) {
            Vehs[veh_id].state |= VSTATE_PACIFISM_DRONE;
        }
    }
}

// Unreferenced in the pinned image (no call, jump or data reference).
// Original effect: return without modifying EAX; callee stack cleanup 0 bytes.
// ORIGINAL: 0x004E9B60 nullsub_220 0x004E9B60-0x004E9B61 BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason": "Bind the unreferenced original leaf as an empty/constant source function in the translation unit whose address band contains it; its original name, prototype and callers remain unknown.", "evidence": ["pinned:terranx_original.exe:0x004E9B60-0x004E9B61 return without modifying EAX; callee stack cleanup 0 bytes", "pinned scan: no rel32 call/jmp and no dword reference to this address anywhere in the image", "address band: nearest mapped neighbours are 0x004E9550 ?base_support@@YAXXZ (src/base.cpp) and 0x004E9B70 ?base_nutrient@@YAXXZ (src/base.cpp)"]}
// symbol    ?nullsub_220@@YAXXZ
// kind      game
void __cdecl nullsub_220() {
}

/*
Purpose: Calculate nutrients and growth for the current base.
// ORIGINAL: 0x004E9B70 ?base_nutrient@@YAXXZ 0x004E9B70-0x004E9CAD BYTE_EXACT
// LEVER: the image's single call is bitmask() with two ebp-relative
//   out-params (the has_fac_built(FAC_CHILDREN_CRECHE,...) shape - an
//   ebp-based frame exists ONLY to hold those two locals). Under this
//   tree's toolchain, has_fac_built(literal, base_id) always constant-folds
//   the offset/mask math to a direct `test byte ptr [...], imm` with no
//   call and no frame at all (same defect as base_minerals's
//   CENTAURI_PRESERVE/TEMPLE_OF_PLANET checks - see its note). has_fac_built_call
//   (base.h, forces bitmask_call's real E8 the same way breed_mod's checks
//   do) is the fix: swapping just this one site took call_diff to 0
//   disagreeing (was FEWER by 1) and best-across-flags similarity 0.790 ->
//   0.951 (10/93 raw agreeing, /O2 /Gy /GR- /Oy- /GX). Residual is the same
//   neg/sbb/neg 0-or-1 materialization plateau documented on is_objective
//   (0x005AC060) - the image boolifies the facilities_built bit test before
//   its `test eax,eax`, this tree emits a plain `test byte ptr [...]`.
// TRIED: `has_fac_built_call`'s own return type is already `int` (see
//   0x00421670's LEVER note); assigning its result to a named `BOOL` local
//   before the `if` here made no difference either - VC6 still proves the
//   value feeds only one branch and drops the materialization. Same
//   plateau, confirmed a second way.
// size      317 bytes
// prototype
// callers   7   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0050BA00
Return Value: n/a
Status: Complete
*/
void __cdecl base_nutrient() {
    uint32_t faction_id = BaseCurrent->faction_id_current;
    BaseCurrentGrowthRate = PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::GROWTH];
    if (has_fac(FAC_CHILDREN_CRECHE, BaseIDCurrentSelected, 0)) {
        BaseCurrentGrowthRate += 2; // +2 on growth scale
    }
    if (BaseCurrent->state & BSTATE_GOLDEN_AGE_ACTIVE) {
        BaseCurrentGrowthRate += 2;
    }
    // Preserve native dword wrapping before the signed surplus branches:
    // MOVSX population_size/IMUL nutrient_req at 004E9C13/17, then SUB at 9C37.
    BaseCurrent->intake_2[RSC_NUTRIENTS] += BaseCurrentConvoyTo[RSC_NUTRIENTS];
    BaseCurrent->nutrient_consumption = BaseCurrent->population_size * Rules.nutrient_req_citizen
        + BaseCurrentConvoyFrom[RSC_NUTRIENTS];
    BaseCurrent->nutrient_surplus = BaseCurrent->intake_2[RSC_NUTRIENTS]
        - BaseCurrent->nutrient_consumption;
    if (BaseCurrent->nutrient_surplus >= 0) {
        if (BaseCurrent->nutrients_accumulated < 0) {
            BaseCurrent->nutrients_accumulated = 0;
        }
    } else if (!(BaseCurrent->nutrients_accumulated)) {
        BaseCurrent->nutrients_accumulated = -1;
    }
    if (BaseUpkeepStage == 1) {
        PlayersData[faction_id].nutrient_surplus_total
            += range(BaseCurrent->nutrient_surplus, 0, 99);
    }
}


/*
Purpose: Calculate minerals and ecological damage for the current base.
// ORIGINAL: 0x004E9CB0 ?base_minerals@@YAXXZ 0x004E9CB0-0x004EA1EF BYTE_EXACT
// LEVER: the five out-of-line has_fac() calls sit one inline level deep
//   (selected_base_has_fac) and exceed VC6's inline budget, which expands
//   shallower levels first, then in source order; the two later direct
//   has_fac() sites still inline. Plain int arithmetic, no wrappers.
//   The has_project() reads go through base_project() (one more inline
//   level); that budget shift is what moves the eco_damage IMUL ahead of
//   diff_factor's, as in the image.
// size      1343 bytes
// prototype
// callers   7   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00421670 0x0050BA00
Return Value: n/a
Status: Complete
*/
void __cdecl base_minerals() {
    uint32_t faction_id = BaseCurrent->faction_id_current;
    BaseCurrent->intake_2[RSC_MINERALS] = (BaseCurrent->intake_2[RSC_MINERALS] + (int)BaseCurrentConvoyTo[RSC_MINERALS]);
    int mineral_bonus = 0;
    if (selected_base_has_fac(FAC_QUANTUM_CONVERTER) || has_project_via_base_project(SP_SINGULARITY_INDUCTOR, faction_id)) {
        mineral_bonus = 1;
    }
    if (selected_base_has_fac(FAC_ROBOTIC_ASSEMBLY_PLANT)) {
        mineral_bonus++;
    }
    if (selected_base_has_fac(FAC_GENEJACK_FACTORY)) {
        mineral_bonus++;
    }
    if (selected_base_has_fac(FAC_NANOREPLICATOR)) {
        mineral_bonus++;
    }
    if (has_project_via_base_project(SP_BULK_MATTER_TRANSMITTER, faction_id)) {
        mineral_bonus++;
    }
    BaseCurrent->intake_2[RSC_MINERALS] = (BaseCurrent->intake_2[RSC_MINERALS] * (mineral_bonus + 2)) / 2;
    BaseCurrent->mineral_consumption = (BaseCurrentForcesMaintCost + (int)BaseCurrentConvoyFrom[RSC_MINERALS]);
    BaseCurrent->mineral_surplus = (BaseCurrent->intake_2[RSC_MINERALS] - BaseCurrent->mineral_consumption);
    BaseCurrent->mineral_inefficiency = 0; // ?
    BaseCurrent->mineral_surplus = (BaseCurrent->mineral_surplus - BaseCurrent->mineral_inefficiency); // ?
    BaseCurrent->mineral_surplus_final = BaseCurrent->mineral_surplus;
    // Native SAR rounds negative values down, not toward zero like /8.
    BaseCurrent->eco_damage = BaseCurrent->eco_damage >> 3;
    int planet_eco_factor = (PlayersData[faction_id].planet_ecology + 16);
    if (BaseCurrent->eco_damage > 0) {
        int planet_modifier = BaseCurrent->eco_damage;
        if (BaseCurrent->eco_damage >= planet_eco_factor) {
            planet_modifier = planet_eco_factor;
        }
        planet_eco_factor = (planet_eco_factor - planet_modifier);
        BaseCurrent->eco_damage = (BaseCurrent->eco_damage - planet_modifier);
    }
    int eco_dmg_reduction = 1;
    if (selected_base_has_fac(FAC_NANOREPLICATOR) || has_project_via_base_project(SP_SINGULARITY_INDUCTOR, faction_id)) {
        eco_dmg_reduction = 2;
    }
    if (has_fac(FAC_CENTAURI_PRESERVE, BaseIDCurrentSelected, 0)) {
        eco_dmg_reduction++;
    }
    if (has_fac(FAC_TEMPLE_OF_PLANET, BaseIDCurrentSelected, 0)) {
        eco_dmg_reduction++;
    }
    if (has_project_via_base_project(SP_PHOLUS_MUTAGEN, faction_id)) {
        eco_dmg_reduction++;
    }
    BaseCurrent->eco_damage = (BaseCurrent->eco_damage + ((((BaseCurrent->intake_2[RSC_MINERALS] - planet_eco_factor)) - (range(PlayersData[faction_id].satellites_[1],
                0, BaseCurrent->population_size))) / eco_dmg_reduction));
    if (is_human(faction_id)) {
        BaseCurrent->eco_damage = (BaseCurrent->eco_damage + ((PlayersData[faction_id].major_atrocities * 5)
                / ((range(MapSeaLevel, 0, 100)
                    / range(WorldBuilder.sea_level_rises, 1, 100)) + eco_dmg_reduction)));
    }
    // The native pass tests the human mask and computes the divisor again;
    // combining these two numerators changes even ordinary rounding.
    if (is_human(faction_id)) {
        BaseCurrent->eco_damage = (BaseCurrent->eco_damage + ((TectonicDetonationCount[faction_id] * 5)
                / ((range(MapSeaLevel, 0, 100)
                    / range(WorldBuilder.sea_level_rises, 1, 100)) + eco_dmg_reduction)));
    }
    if (BaseCurrent->eco_damage < 0) {
        BaseCurrent->eco_damage = 0;
    }
    // Inline Voice of Planet test (ascending's body); the image never calls it here.
    if (base_project(SP_VOICE_OF_PLANET) != SP_Unbuilt && GameDataStateStorage.fields_.game_rules_ & RULES_VICTORY_TRANSCENDENCE) {
        BaseCurrent->eco_damage *= 2;
    }
    if (GameDataStateStorage.fields_.game_state_ & STATE_PERIHELION_ACTIVE) {
        BaseCurrent->eco_damage *= 2;
    }
    int diff_factor;
    if (is_human(faction_id)) {
        int diff_lvl = PlayersData[faction_id].diff_level;
        diff_factor = !diff_lvl ? DLVL_TALENT
            : ((diff_lvl > DLVL_LIBRARIAN) ? DLVL_TRANSCEND : DLVL_LIBRARIAN);
    } else {
        diff_factor = range((6 - GameDataStateStorage.fields_.diff_level_current_),
            DLVL_SPECIALIST, DLVL_LIBRARIAN);
    }
    // Each native IMUL retains its low dword; the following /6 is signed.
    // There is no second zero clamp after this possibly-negative scaling.
    BaseCurrent->eco_damage = (PlayersData[faction_id].tech_ranking
        - PlayersData[faction_id].theory_of_everything)
        * (3 - range(PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::PLANET], -3, 2))
        * (MapNativeLifeForms + 1) * BaseCurrent->eco_damage * diff_factor / 6;
    BaseCurrent->eco_damage = (BaseCurrent->eco_damage + 50) / 100;
    int queue_id;
    if (has_project_via_base_project(SP_SPACE_ELEVATOR, faction_id) // orbital facilities double mineral prod rate
        && (queue_id = BaseCurrent->queue_production_id[0], queue_id == -FAC_SKY_HYDRO_LAB
            || queue_id == -FAC_NESSUS_MINING_STATION || queue_id == -FAC_ORBITAL_POWER_TRANS
            || queue_id == -FAC_ORBITAL_DEFENSE_POD)) {
        BaseCurrent->intake_2[RSC_MINERALS] *= 2;
        BaseCurrent->mineral_surplus = // doesn't update mineral_surplus_final?
            BaseCurrent->intake_2[RSC_MINERALS] - BaseCurrent->mineral_consumption;
    }
}

/*
Purpose: Calculate the current base's energy loss/inefficiency for an amount of energy.
// ORIGINAL: 0x004EA1F0 ?black_market@@YAHH@Z 0x004EA1F0-0x004EA495 BYTE_EXACT
// LEVER: the HQ-distance check called vector_dist(4 args) as a real
//   function; the image never calls it - it open-codes x_dist()+abs()x3,
//   same shape as base_find (0x004E3D50), AND re-reads
//   (BaseCurrent())->x/y fresh inside the loop rather than hoisting them
//   to locals before it. Both fixed together: agreeing went 4/233 -> 8/233.
// TRIED: the remaining gap is the same has_fac_built(literal,...)
//   systemic issue documented on has_fac (0x00421670) and base_minerals
//   (0x004E9CB0) - the image keeps a real bitmask() call for
//   FAC_HEADQUARTERS (inside the loop) and FAC_CHILDREN_CRECHE even though
//   both facility ids are literal; this tree's toolchain always folds
//   them. Not re-derived further here; still a 0x1c vs 0xc stack-size gap
//   (the two ebp-relative bitmask out-params never materialise).
// TRIED (measured): swapping both of those has_fac_built() calls for
//   has_fac_built_call() (the general.h bitmask_call forwarder, base.h) to
//   force the real E8 the image emits. This function's best-scoring flag
//   set has no /Ob0 - plain /O2 implies /Ob2 auto-inline, which folds the
//   supposedly-non-inline bitmask_call() forwarder away too (it is a
//   trivial one-liner, exactly what /Ob2 auto-inlines regardless of the
//   `inline` keyword), so call_diff still reports the same wrong-callee
//   shape under a different name and the byte match gets WORSE across every
//   flag set: best similarity 0.772 (/Oi- /Gy /GR- /Oy- /GX) -> 0.608 (best
//   moves to /O1 /Gy /GR- /Oy- /GX). Reverted to has_fac_built().
// symbol    ?black_market@@YAHH@Z
// size      677 bytes
// prototype
// callers   1   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0050BA00 0x00644F3A
Return Value: Amount of energy inefficiency
Status: Complete
*/
int __cdecl black_market(int energy) {
    if (energy <= 0) {
        return 0;
    }
    uint32_t faction_id = BaseCurrent->faction_id_current;
    int dist_hq = 999;
    for (int i = 0; i < GameDataStateStorage.fields_.base_current_count_; i++) { // modified version of vulnerable()
        if (Bases[i].faction_id_current == faction_id && has_fac(FAC_HEADQUARTERS, i, 0)) {
            // Open-coded vector_dist(x,y,a,b): the image calls abs() four
            // times, never vector_dist itself - see base_find (0x004E3D50)
            // for the same expansion. (BaseCurrent())->x/y are re-read here,
            // not hoisted, matching the image reading the global fresh.
            int dist = vector_dist(Bases[i].x, Bases[i].y, BaseCurrent->x, BaseCurrent->y);
            if (dist < dist_hq) {
                dist_hq = dist;
            }
        }
    }
    if (dist_hq == 999) {
        dist_hq = 16;
    } else if (dist_hq == 0) {
        return 0;
    }
    if (BaseUpkeepStage == 1) {
        for (int j = 0; j <= 8; j++) {
            int ineff = j;
            if (has_fac(FAC_CHILDREN_CRECHE, BaseIDCurrentSelected, 0)) {
                ineff -= 2; // +2 on efficiency scale
            }
            if (ineff >= 8) {
                PlayersData[faction_id].unk_46[j] += energy;
            } else {
                PlayersData[faction_id].unk_46[j] += energy * dist_hq / ((8 - ineff) * 8);
            }
        }
    }
    int ineffciency = 4 - PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::EFFICIENCY];
    if (has_fac(FAC_CHILDREN_CRECHE, BaseIDCurrentSelected, 0)) {
        ineffciency -= 2; // +2 on efficiency scale
    }
    if (ineffciency >= 8) {
        return energy;
    }
    return range(energy * dist_hq / ((8 - ineffciency) * 8), 0, energy);
}

/*
Purpose: Calculate the base amount of talents and drones for the specified faction.
// ORIGINAL: 0x004EA4A0 ?psych_check@@YAXHPAHPAH@Z 0x004EA4A0-0x004EA533 BYTE_EXACT
// LEVER: the image clamps efficiency to zero with a BRANCHLESS
//        `setl dl; dec edx; and edx, ecx` before adding 4 - splitting the
//        combined ternary `efficiency < 0 ? 4 : efficiency + 4` into
//        `(efficiency < 0 ? 0 : efficiency) + 4` isolates the classic
//        clamp-to-zero idiom and reaches it. Moved 21/52 (0.796) -> 25/52
//        (0.871).
// TRIED: the `test al,dl` vs `test dl,al` register-order-only divergence
//            on is_human()'s bit test is the same one already documented at
//            this file's `TRIED` notes near line 179/223 (register
//            allocation, not a source shape this tree controls).
// LEVER (2026-09-03): `static_cast<int>(MapAreaSqRoot)`. map.h declares it
//            `uint32_t`, so this tree's product was UNSIGNED and the whole
//            /56 //2 tail compiled to `mul`/`shr` against the image's
//            `imul`/`sar` - 13 divergent instructions, not a scheduling
//            difference. The cast (which technology.cpp's cost model already
//            makes) moved 33/52 -> 36/52. Naming the clamp `weight` before
//            the multiply tied at 36/52; the last two instructions are the
//            image forming `drones + 2` with `lea ecx,[eax+2]` where this
//            tree adds in place (`add eax,2`) - register selection only.
//            --all-flags: the default /O2 /Gy /GR- /Oy- /GX at 36/52 (0.914)
//            beats every other set tried (best rival 9/52).
// symbol    ?psych_check@@YAXHPAH0@Z
// size      147 bytes
// prototype void (__cdecl ?psych_check@@YAXHPAHPAH@Z)(int factionID, int* drones, int* talents)
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl psych_check(int faction_id, int *drones, int *talents) {
    *drones = 6 - (is_human(faction_id) ? PlayersData[faction_id].diff_level : DLVL_LIBRARIAN);
    // The image forms `*drones + 2` from the value just stored (lea ecx,[eax+2]).
    *talents = ((*drones + 2)
        * ((PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::EFFICIENCY] < 0
            ? 0 : PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::EFFICIENCY]) + 4)
        * MapAreaSqRoot) / 56 / 2;
}

// Five ordered stages: base/social, psych spending, facilities, police,
// projects and suppression. These are independent shared publications, not
// per-call locals; draw_psych and base_growth consume their original storage.
// RECOVERY_OBJECT: v2 0x0090E984 0x0090E998 ?BasePsychTalents@@3PAHA 0x004EA540
int BasePsychTalents[5];
// RECOVERY_OBJECT: v2 0x0090E920 0x0090E934 ?BasePsychDrones@@3PAHA 0x004EA540
int BasePsychDrones[5];
// RECOVERY_OBJECT: v2 0x0090E94C 0x0090E960 ?BasePsychSuperDrones@@3PAHA 0x004EA540
int BasePsychSuperDrones[5];

/*
Purpose: Calculate the current base's psych values.
// ORIGINAL: 0x004EA540 ?base_psych@@YAXXZ 0x004EA540-0x004EB481 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x004EA540-0x004EB481 complete original body; 0x004EA712-0x004EA722 clamps the first NoDrone adjustment immediately, and 0x004EAA23-0x004EAA3E applies signed talent rather than police.", "image:0x004EAB85,0x004EACD1,0x004EAE30,0x004EB209,0x004EB3D1 publish the five independent stage histories; draw_psych and base_growth remain seed-only consumers.", "image:0x004EAEDF-0x004EB0C5 scans fresh vehicle stacks in two passes with repeated ability queries; 0x004EB0CA-0x004EB0E3 uses a signed pacifism-count comparison.", "image:0x004EA946 and 0x004EB44E-0x004EB45E use talent=i-4 and matrix row=i-1; original social_ai at 0x005B50DB-0x005B511A instead consumes row=clamp(talent+4,0,7). Preserve the original one-step discrepancy."], "reason": "Recover the complete five-stage psych calculation and shared stage histories, including social simulations followed by the actual case, using genuine base, faction, project and vehicle state. Existing partial source was not a verified prefix."}
// Historical compiler lesson: the first facility queries are real has_fac
// calls, followed by real bitmask calls for hospitals, punishment and brood
// pit. Earlier inlining experiments lost calls despite similar source forms.
// Historical reconstruction correction: the second has_fac uses facility 6,
// Recreation Commons, not facility 2, Children's Creche. This is not evidence
// of an original game bug. The old partial prefix also substituted police for
// the signed talent effect and omitted the five-stage shared publications.
*/
void __cdecl base_psych() {
    // stand-in: the image calls has_fac out of line here (inline budget); a
    // local const function pointer forces the real call.
    BOOL (__cdecl *const has_fac_call)(int, int, int) = has_fac;
    int faction = BaseCurrent->faction_id_current;
    int content;
    int bureaucracy;
    // 0x004EA546-0x004EA5E4 is psych_check's body, inlined (/Ob2).
    psych_check(faction, &content, &bureaucracy);
    if (bureaucracy) {
        bureaucracy = range((PlayersData[faction].current_num_bases
            + BaseIDCurrentSelected % bureaucracy - bureaucracy) / bureaucracy,
            0, BaseCurrent->population_size);
    }
    int talents = 0;
    int diff = is_human(faction) ? PlayersData[faction].diff_level : DLVL_LIBRARIAN;
    int drones = range(BaseCurrent->population_size - content, 0, BaseCurrent->population_size);
    drones += range((BaseCurrent->assimilation_turns_left + 9) / 10, 0,
        (BaseCurrent->population_size + diff - 2) / 4);
    if (Players[faction].rule_drone) {
        drones += BaseCurrent->population_size / Players[faction].rule_drone;
    }
    if (Players[faction].rule_talent) {
        talents = (Players[faction].rule_talent + BaseCurrent->population_size - 1)
            / Players[faction].rule_talent;
    }
    for (int k = 0; k < Players[faction].faction_bonus_count; k++) {
        if (Players[faction].faction_bonus_id[k] == RULE_NODRONE) {
            drones -= Players[faction].faction_bonus_val1[k];
            if (drones < 0) {
                drones = 0;
            }
            k = Players[faction].faction_bonus_count;
        }
    }

    int fac_drones = 0;
    int fac_talents = 0;
    if (has_fac_call(FAC_GENEJACK_FACTORY, BaseIDCurrentSelected, 0)) {
        fac_drones = Rules.drones_genejack_factory;
    }
    if (has_fac_call(FAC_RECREATION_COMMONS, BaseIDCurrentSelected, 0)) {
        fac_drones -= 2;
    }
    if ((has_fac_call(FAC_NETWORK_NODE, BaseIDCurrentSelected, 0)
        && has_project(SP_VIRTUAL_WORLD, faction))
        || has_fac_call(FAC_HOLOGRAM_THEATRE, BaseIDCurrentSelected, 0)) {
        fac_drones -= 2;
    }
    if (has_project(SP_PLANETARY_TRANS_SYS, faction) && BaseCurrent->population_size <= 3) {
        fac_drones--;
    }
    if (has_fac_call(FAC_PARADISE_GARDEN, BaseIDCurrentSelected, 0)) {
        fac_talents = 2;
    }
    // One variable ([ebp-0x48]) receives all three built-facility tests.
    if (has_fac(FAC_RESEARCH_HOSPITAL, BaseIDCurrentSelected, 0)) {
        fac_drones--;
    }
    if (has_fac(FAC_NANOHOSPITAL, BaseIDCurrentSelected, 0)) {
        fac_drones--;
    }
    int proj_talents = 0;
    if (has_project(SP_HUMAN_GENOME_PROJ, faction)) {
        proj_talents = 1;
    }
    if (has_project(SP_CLINICAL_IMMORTALITY, faction)) {
        proj_talents += 2;
    }
    BOOL punishment = has_fac(FAC_PUNISHMENT_SPHERE, BaseIDCurrentSelected, 0);

    for (int i = 8; i >= 0; i--) {
        int talent;
        if (i) {
            if (BaseUpkeepStage != 1) {
                continue;
            }
            talent = i - 4;
        } else {
            talent = PlayersData[faction].soc_effect_pending.values_[SocialEffect::TALENT];
        }
        for (int j = 9; j >= 0; j--) {
            int police;
            if (j) {
                if (BaseUpkeepStage != 1 || !i) {
                    continue;
                }
                police = j - 6;
            } else {
                police = PlayersData[faction].soc_effect_pending.values_[SocialEffect::POLICE]
                    + 2 * has_fac(FAC_BROOD_PIT, BaseIDCurrentSelected, 0);
                if (i) {
                    continue;
                }
            }
            int police_limit = range(police + 2, 0, 4);
            if (police_limit > 1) {
                police_limit--;
            }

            // Stage 0: base, social talent, bureaucracy.
            BaseCurrent->drone_total = drones;
            BaseCurrent->talent_total = talents;
            if (talent < 0) {
                BaseCurrent->drone_total -= talent;
            } else {
                BaseCurrent->talent_total += talent;
            }
            BaseCurrent->drone_total = range(BaseCurrent->drone_total, 0,
                BaseCurrent->population_size) + bureaucracy;
            BaseCurrent->super_drone_total = range(BaseCurrent->drone_total
                - BaseCurrent->population_size, 0, BaseCurrent->population_size);
            BaseCurrent->drone_total = range(BaseCurrent->drone_total, 0,
                BaseCurrent->population_size);
            while (BaseCurrent->super_drone_total + BaseCurrent->talent_total
                + BaseCurrent->drone_total
                > BaseCurrent->population_size - BaseCurrent->specialist_total) {
                if (BaseCurrent->talent_total) {
                    BaseCurrent->talent_total--;
                    if (BaseCurrent->super_drone_total) {
                        BaseCurrent->super_drone_total--;
                    } else if (BaseCurrent->drone_total) {
                        BaseCurrent->drone_total--;
                    }
                } else if (BaseCurrent->drone_total
                    > BaseCurrent->population_size - BaseCurrent->specialist_total) {
                    BaseCurrent->drone_total--;
                } else {
                    BaseCurrent->super_drone_total = range(BaseCurrent->super_drone_total,
                        0, BaseCurrent->drone_total);
                    break;
                }
            }
            BasePsychTalents[0] = BaseCurrent->talent_total;
            BasePsychDrones[0] = BaseCurrent->drone_total;
            BasePsychSuperDrones[0] = BaseCurrent->super_drone_total;

            // Stage 1: psych spending.
            BaseCurrent->talent_total += BaseCurrent->psych_total / 2;
            BaseCurrent->drone_total = range(BaseCurrent->drone_total, 0,
                BaseCurrent->population_size);
            BaseCurrent->talent_total = range(BaseCurrent->talent_total, 0,
                BaseCurrent->population_size);
            while (BaseCurrent->super_drone_total + BaseCurrent->talent_total
                + BaseCurrent->drone_total
                > BaseCurrent->population_size - BaseCurrent->specialist_total) {
                if (BaseCurrent->talent_total) {
                    BaseCurrent->talent_total--;
                    if (BaseCurrent->super_drone_total) {
                        BaseCurrent->super_drone_total--;
                    } else if (BaseCurrent->drone_total) {
                        BaseCurrent->drone_total--;
                    }
                } else if (BaseCurrent->drone_total
                    > BaseCurrent->population_size - BaseCurrent->specialist_total) {
                    BaseCurrent->drone_total--;
                } else {
                    BaseCurrent->super_drone_total = range(BaseCurrent->super_drone_total,
                        0, BaseCurrent->drone_total);
                    break;
                }
            }
            BasePsychTalents[1] = BaseCurrent->talent_total;
            BasePsychDrones[1] = BaseCurrent->drone_total;
            BasePsychSuperDrones[1] = BaseCurrent->super_drone_total;

            // Stage 2: facilities.
            BaseCurrent->drone_total += fac_drones;
            BaseCurrent->talent_total += fac_talents;
            BaseCurrent->drone_total = range(BaseCurrent->drone_total, 0,
                BaseCurrent->population_size);
            BaseCurrent->talent_total = range(BaseCurrent->talent_total, 0,
                BaseCurrent->population_size);
            while (BaseCurrent->super_drone_total + BaseCurrent->talent_total
                + BaseCurrent->drone_total
                > BaseCurrent->population_size - BaseCurrent->specialist_total) {
                if (BaseCurrent->talent_total) {
                    BaseCurrent->talent_total--;
                    if (BaseCurrent->super_drone_total) {
                        BaseCurrent->super_drone_total--;
                    } else if (BaseCurrent->drone_total) {
                        BaseCurrent->drone_total--;
                    }
                } else if (BaseCurrent->drone_total
                    > BaseCurrent->population_size - BaseCurrent->specialist_total) {
                    BaseCurrent->drone_total--;
                } else {
                    BaseCurrent->super_drone_total = range(BaseCurrent->super_drone_total,
                        0, BaseCurrent->drone_total);
                    break;
                }
            }
            BasePsychTalents[2] = BaseCurrent->talent_total;
            BasePsychDrones[2] = BaseCurrent->drone_total;
            BasePsychSuperDrones[2] = BaseCurrent->super_drone_total;

            // Stage 3: police.
            if (police_limit > 0) {
                int used = 0;
                if (has_project(SP_SELF_AWARE_COLONY, faction)) {
                    used = 1;
                    if (BaseCurrent->drone_total) {
                        BaseCurrent->drone_total--;
                    }
                    if (police >= 3 && BaseCurrent->drone_total) {
                        BaseCurrent->drone_total--;
                    }
                }
                // BUG(original): vehicle ownership is never checked.
                for (int pass = 0; pass < 2 && used < police_limit; pass++) {
                    for (int veh = stack_fix(veh_at(BaseCurrent->x, BaseCurrent->y));
                        veh >= 0; veh = Vehs[veh].next_veh_id_stack) {
                        if (VehPrototypes[Vehs[veh].proto_id].plan > 3) {
                            continue;
                        }
                        // Pass 0 takes enhanced (2x / native) police, pass 1 the rest.
                        if (!has_abil(Vehs[veh].proto_id, ABL_POLICE_2X)
                            && !((Players[faction].rule_flags & RFLAG_WORMPOLICE)
                                && Vehs[veh].proto_id < MaxVehProtoFactionNum
                                && (Weapon[VehPrototypes[Vehs[veh].proto_id].weapon_id].offense_rating < 0
                                    || Vehs[veh].proto_id == BSC_SPORE_LAUNCHER)
                                && Chassis[VehPrototypes[Vehs[veh].proto_id].chassis_id].triad == TRIAD_LAND)) {
                            if (pass) {
                                goto apply;
                            }
                            continue;
                        }
                        if (pass) {
                            continue;
                        }
                    apply:
                        if (BaseCurrent->drone_total) {
                            BaseCurrent->drone_total--;
                        }
                        if (has_abil(Vehs[veh].proto_id, ABL_POLICE_2X)
                            && BaseCurrent->drone_total) {
                            BaseCurrent->drone_total--;
                        }
                        if (police >= 3 && BaseCurrent->drone_total) {
                            BaseCurrent->drone_total--;
                        }
                        if ((Players[faction].rule_flags & RFLAG_WORMPOLICE)
                            && Vehs[veh].proto_id < MaxVehProtoFactionNum
                            && (Weapon[VehPrototypes[Vehs[veh].proto_id].weapon_id].offense_rating < 0
                                || Vehs[veh].proto_id == BSC_SPORE_LAUNCHER)
                            && Chassis[VehPrototypes[Vehs[veh].proto_id].chassis_id].triad == TRIAD_LAND
                            && BaseCurrent->drone_total) {
                            BaseCurrent->drone_total--;
                        }
                        if (++used >= police_limit) {
                            break;
                        }
                    }
                }
            }
            if (police < -2) {
                switch (police) {
                case -3:
                    // Signed compare in the image (jle); the global is declared uint32_t.
                    if (BaseCurrentVehPacifismCount > 1) {
                        BaseCurrent->drone_total += BaseCurrentVehPacifismCount - 1;
                    }
                    break;
                case -4:
                    BaseCurrent->drone_total += BaseCurrentVehPacifismCount;
                    break;
                default:
                    BaseCurrent->drone_total += BaseCurrentVehPacifismCount * 2;
                    break;
                }
            }
            BaseCurrent->drone_total = range(BaseCurrent->drone_total, 0,
                BaseCurrent->population_size);
            BaseCurrent->talent_total = range(BaseCurrent->talent_total, 0,
                BaseCurrent->population_size);
            while (BaseCurrent->super_drone_total + BaseCurrent->talent_total
                + BaseCurrent->drone_total
                > BaseCurrent->population_size - BaseCurrent->specialist_total) {
                if (BaseCurrent->talent_total) {
                    BaseCurrent->talent_total--;
                    if (BaseCurrent->super_drone_total) {
                        BaseCurrent->super_drone_total--;
                    } else if (BaseCurrent->drone_total) {
                        BaseCurrent->drone_total--;
                    }
                } else if (BaseCurrent->drone_total
                    > BaseCurrent->population_size - BaseCurrent->specialist_total) {
                    BaseCurrent->drone_total--;
                } else {
                    BaseCurrent->super_drone_total = range(BaseCurrent->super_drone_total,
                        0, BaseCurrent->drone_total);
                    break;
                }
            }
            BasePsychTalents[3] = BaseCurrent->talent_total;
            BasePsychDrones[3] = BaseCurrent->drone_total;
            BasePsychSuperDrones[3] = BaseCurrent->super_drone_total;

            // Stage 4: secret projects and suppression.
            BaseCurrent->talent_total += proj_talents;
            if (has_project(SP_LONGEVITY_VACCINE, faction)) {
                if (PlayersData[faction].soc_category_pending.models_[SocialCategory::ECONOMICS] == 2) {
                    BaseCurrent->drone_total--;
                }
                if (PlayersData[faction].soc_category_pending.models_[SocialCategory::ECONOMICS] != 1) {
                    BaseCurrent->drone_total--;
                }
            }
            if (punishment || BaseCurrent->nerve_staple_turns_left) {
                BaseCurrent->drone_total = 0;
                BaseCurrent->talent_total = 0;
            }
            BaseCurrent->drone_total = range(BaseCurrent->drone_total, 0,
                BaseCurrent->population_size);
            BaseCurrent->talent_total = range(BaseCurrent->talent_total, 0,
                BaseCurrent->population_size);
            while (BaseCurrent->super_drone_total + BaseCurrent->talent_total
                + BaseCurrent->drone_total
                > BaseCurrent->population_size - BaseCurrent->specialist_total) {
                if (BaseCurrent->talent_total) {
                    BaseCurrent->talent_total--;
                    if (BaseCurrent->super_drone_total) {
                        BaseCurrent->super_drone_total--;
                    } else if (BaseCurrent->drone_total) {
                        BaseCurrent->drone_total--;
                    }
                } else if (BaseCurrent->drone_total
                    > BaseCurrent->population_size - BaseCurrent->specialist_total) {
                    BaseCurrent->drone_total--;
                } else {
                    BaseCurrent->super_drone_total = range(BaseCurrent->super_drone_total,
                        0, BaseCurrent->drone_total);
                    break;
                }
            }
            BasePsychTalents[4] = BaseCurrent->talent_total;
            BasePsychDrones[4] = BaseCurrent->drone_total;
            BasePsychSuperDrones[4] = BaseCurrent->super_drone_total;

            if (i) {
                int score = BaseCurrent->talent_total - BaseCurrent->drone_total;
                if (score > 0 && (BaseCurrent->drone_total
                    || BaseCurrent->talent_total * 2 < BaseCurrent->population_size)) {
                    score /= 2;
                }
                if (score < 0) {
                    score -= BaseCurrent->population_size;
                    if (BaseCurrent->population_size < 6) {
                        score *= 2;
                    }
                }
                // BUG(original): the producer stores talent (i-4) at row i-1;
                // social_ai reads row clamp(talent+4,0,7), one step off.
                PlayersData[faction].unk_39[i - 1][j - 1] += score;
            }
        }
    }
}

/*
Purpose: Determine the faction's base with the specified position sorted by the most energy output.
// ORIGINAL: 0x004EB490 ?base_rank@@YAHHH@Z 0x004EB490-0x004EB551 BYTE_EXACT
// LEVER: the image never calls sort_descending() - it inlines the bubble sort loop directly (two swap() calls, matching general.cpp's body). Writing that loop out here instead of calling sort_descending() took the call set from {sort_descending, __alloca_probe} to {swap x2, __alloca_probe}, matching the image's (call_diff now agrees). Best flag set moved from 0.645 to 0.847 similar. Remaining divergence is instruction-order/register-scheduling around the parameter loads, not chased further.
// LEVER: branch polarity. Deleted the two added bounds-check guards (`rank_position < 0` and `!bases_found`) and replaced them with the image's single guard clause `if (rank_position >= bases_found) return -1;` written immediately after the collection loop - VC6 compiles a guard clause by testing the ORIGINAL condition and jumping forward to the work (jl to the sort), falling through into the early return, which is what the image does; the equivalent `if (rank_position < bases_found) {work} return -1;` form emits the opposite polarity (jge past the work) and does not match. Also reproduces the original's lack of a `rank_position < 0` bounds check - see the BUG IN THE ORIGINAL comment below. Took this body to BYTE_EXACT (70/70), from 4/70.
// HISTORICAL LEVER (superseded by the shared integer-swap owner below): a
// file-local `swap_call` restored the two original 0x628A50 calls and array
// order while swap was header-inline. The source now names the mapped owner
// directly; the old frame-allocation observations remain historical.
// size      193 bytes
// prototype int (__cdecl ?base_rank@@YAHHH@Z)(int factionID, int minBasesToRank)
// callers   1   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00628A50 0x00645550
Return Value: Base id for the specified rank position or -1 for invalid requests
Status: Complete
*/
int __cdecl base_rank(int faction_id, int rank_position) {
    // BUG IN THE ORIGINAL: the shipped code never bounds-checks rank_position
    // against 0, only against bases_found below - a negative rank_position
    // with at least one matching base reads base_id[rank_position] out of
    // bounds. Reproduced deliberately: the added guard here scored worse
    // (see LEVER below), because the image loads rank_position from memory
    // only once, at the single `cmp .../jl` test after the collection loop.
    // base_energy lands at [ebp-0x1000] and base_id above it at [ebp-0x800]
    // once the two swaps call the out-of-line integer swap owner: an address-
    // taken array is placed when its & is first materialised, so the sort's
    // swap order fixes the frame. Declaring or storing energy first, with
    // HISTORICAL: when integer swap was header-inline, folding it did NOT
    // reach that layout - 26/70.
    int base_id[MaxBaseNum];
    int base_energy[MaxBaseNum];
    int bases_found = 0;
    for (int i = 0; i < GameDataStateStorage.fields_.base_current_count_; i++) {
        if (Bases[i].faction_id_current == faction_id) {
            base_id[bases_found] = i;
            base_energy[bases_found++] = (Bases[i].intake_1[RSC_ENERGY] * 512) + i;
        }
    }
    if (rank_position >= bases_found) {
        return -1;
    }
    // Open-coded sort_descending() (see general.cpp): the image never
    // calls it, only swap() twice per inner-loop swap.
    int bounds = bases_found - 1;
    BOOL has_swapped;
    do {
        has_swapped = false;
        for (int i = 0; i < bounds; i++) {
            if (base_energy[i] < base_energy[i + 1]) {
                has_swapped = true;
                swap(&base_energy[i], &base_energy[i + 1]);
                swap(&base_id[i], &base_id[i + 1]);
            }
        }
    } while (has_swapped);
    return base_id[rank_position];
}

// The selected-base energy pass owns convoy/commerce accounting, upkeep-stage
// reserve transfer, allocation and specialist yields, facility/project multipliers,
// faction efficiency adjustment, Network Backbone research credits and the final
// energy-production special case. Historical byte and restoration obligations
// remain attached to the complete original span.
// Recovery name: candidate specialist yields at least the old one's
// ops/psych/research bonuses.
static inline BOOL citizen_covers(int candidate, int specialist) {
    for (int k = 0; k < 3; k++) {
        if (Citizen[candidate].bonus[k] < Citizen[specialist].bonus[k]) {
            return FALSE;
        }
    }
    return TRUE;
}

// ORIGINAL: 0x004EB560 ?base_energy@@YAXXZ 0x004EB560-0x004EC3A6 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:b2de7f29042d247ee9b448c814029bbbbc35f230:src/unrecovered/004eb560.cpp", "image:0x004EB560-0x004EC3A6 ranks the selected base and publishes seven commerce pairs at 0x0090EA10/0x0090E964; its commerce base doubles when 0x009A6628 is below 0x009A6624", "source:src/base.h", "source:src/faction.h"], "reason": "Recover the selected-base energy pass through typed Base, PlayerData, Player, Citizen, secret-project and shared-ledger owners, including trade, upkeep, specialist, facility, research-efficiency and final production stages. This source mapping makes no exactness claim; preserve historical claims and seed findings as restoration obligations."}
// kind      game
// Historical scaffold-era TRIED note follows; it records the earlier offset-transliteration attempt, not this typed owner body.
// TRIED: void return confirmed - the function's only exit is the fall- through after `call base_psych` (itself void, ?base_psych@@YAXXZ) with no eax write in between, so it is a real `ret`, not a status. The 6/40/80/99999 constants the audit flagged are all clamp bounds (min/max against a computed value that is then stored into a per-base or per-faction field) or array strides, never delivered to the caller. Body is a direct, offset-checked transcription of the Ghidra decompilation with DAT_ globals replaced by explicit fixed-address casts (BASE macro re-reads [0x90ea30] at each use rather than caching it, matching the repeated `mov eax,[0x90ea30]` in the disassembly) and callee names resolved to the scaffold's declared black_market/ base_rank/best_specialist/has_tech/bitmask/base_psych. Compiles and preserves every branch, loop and goto (LAB_004ebbde, LAB_004ec1f6, LAB_004ebddd); did not chase the early-prologue register-scheduling divergence given the size already spent.
void __cdecl base_energy() {
    int faction = BaseCurrent->faction_id_current;
    // BUG(original): one stack local carries the commerce base, the selected
    // base's own receipt and the alien facility subtotal. Only a partner that
    // reaches base_rank writes it, so the alien pass and the Network Backbone
    // alien credit read whatever the stack held when no partner qualified.
    int value;
    // BUG(original): assigned only when the scan meets BaseCurrent.
    int base_index;
    BaseCurrent->intake_2[RSC_ENERGY] += BaseCurrentConvoyTo[RSC_ENERGY];
    BaseCurrent->energy_consumption = BaseCurrentConvoyFrom[RSC_ENERGY];
    BaseCurrent->energy_inefficiency =
        black_market(BaseCurrent->intake_2[RSC_ENERGY] - BaseCurrent->energy_consumption);
    BaseCurrent->energy_surplus = BaseCurrent->intake_2[RSC_ENERGY]
        - BaseCurrent->energy_consumption - BaseCurrent->energy_inefficiency;

    int rank = 0;
    for (int i = 0; i < GameDataStateStorage.fields_.base_current_count_; i++) {
        if (Bases[i].faction_id_current == faction) {
            if ((Bases[i].intake_1[RSC_ENERGY] << 9) + i
                > (BaseCurrent->intake_1[RSC_ENERGY] << 9) + BaseIDCurrentSelected) {
                rank++;
            }
            if (&Bases[i] == BaseCurrent) {
                base_index = i;
            }
        }
    }
    for (int partner = 1; partner < MaxPlayerNum; partner++) {
        BaseCommerceReceipts[partner - 1] = 0;
        BaseCommercePartnerReceipts[partner - 1] = 0;
        if (partner == faction) {
            continue;
        }
        if (!(PlayersData[faction].diplo_treaties[partner] & DTREATY_TREATY)
            && !(PlayersData[partner].diplo_treaties[faction] & DTREATY_TREATY)) {
            continue;
        }
        if (!PlayersData[partner].current_num_bases) {
            continue;
        }
        int partner_base = base_rank(partner, rank);
        if (partner_base < 0) {
            continue;
        }
        value = (Bases[partner_base].intake_1[RSC_ENERGY]
                 + BaseCurrent->intake_1[RSC_ENERGY] + 7) / 8;
        if (GameDataStateStorage.fields_.council_resolution_count_[PROP_GLOBAL_TRADE_PACT]
            > GameDataStateStorage.fields_.council_resolution_count_[PROP_REPEAL_GLOBAL_TRADE_PACT]) {
            value *= 2;
        }
        int divisor = TechCommerceCount + 1;
        int partner_receipt =
            ((PlayersData[partner].tech_commerce_bonus + 1) * value + divisor / 2) / divisor;
        value = ((PlayersData[faction].tech_commerce_bonus + 1) * value + divisor / 2) / divisor;
        if (!(PlayersData[faction].diplo_treaties[partner] & DTREATY_PACT)) {
            value /= 2;
            partner_receipt /= 2;
        }
        if (GameDataStateStorage.fields_.governor_faction_ == faction) {
            value++;
        }
        if (GameDataStateStorage.fields_.governor_faction_ == partner) {
            partner_receipt++;
        }
        if (PlayersData[faction].sanction_turns || PlayersData[partner].sanction_turns
            || (Players[faction].rule_flags & RFLAG_ALIEN)
            || (Players[partner].rule_flags & RFLAG_ALIEN)) {
            partner_receipt = 0;
            value = 0;
        }
        BaseCommerceReceipts[partner - 1] = value;
        BaseCommercePartnerReceipts[partner - 1] = partner_receipt;
        BaseCurrent->intake_2[RSC_ENERGY] += value;
    }
    if (is_alien_faction(faction)) {
        for (int fac = 0; fac < 70; fac++) {
            if (BaseCurrent->facilities_built[fac / 8] & (1 << (fac % 8))) {
                value++;
            }
        }
        for (int project = 0; project < 64; project++) {
            if (GameDataStateStorage.fields_.secret_project_.project_base[project] == base_index) {
                value += 5;
            }
        }
        value /= 2;
        if (PlayersData[faction].sanction_turns) {
            value = 0;
        }
        BaseCurrent->intake_2[RSC_ENERGY] += value;
    }
    if (BaseUpkeepStage == 1) {
        PlayersData[faction].unk_13 += range(BaseCurrent->energy_surplus, 0, 99999);
    }
    BaseCurrent->energy_inefficiency =
        black_market(BaseCurrent->intake_2[RSC_ENERGY] - BaseCurrent->energy_consumption);
    BaseCurrent->energy_surplus = BaseCurrent->intake_2[RSC_ENERGY]
        - BaseCurrent->energy_consumption - BaseCurrent->energy_inefficiency;

    int energy = range(BaseCurrent->energy_surplus, 0, 9999);
    BaseCurrent->psych_total =
        range((PlayersData[faction].se_alloc_psych * energy + 4) / 10, 0, energy);
    BaseCurrent->economy_total = range(
        ((10 - PlayersData[faction].se_alloc_labs - PlayersData[faction].se_alloc_psych)
         * energy + 4) / 10,
        0, energy - BaseCurrent->psych_total);
    BaseCurrent->labs_total =
        energy - BaseCurrent->psych_total - BaseCurrent->economy_total;
    BaseCurrent->unk_10 = range(
        ((9 - PlayersData[faction].se_alloc_labs - PlayersData[faction].se_alloc_psych)
         * energy + 4) / 10,
        0, energy - BaseCurrent->psych_total);

    // The original indexes Citizen[].bonus[] and
    // Base::economy/psych/labs_total as three-element arrays.
    for (int citizen = 0; citizen < BaseCurrent->specialist_total; citizen++) {
        int specialist = range(base_specialist_type(citizen), 0, 6);
        if (has_tech(Citizen[specialist].obsol_tech, faction)) {
            for (int candidate = 0; candidate < 7; candidate++) {
                if (has_tech(Citizen[candidate].preq_tech, faction)
                    && !has_tech(Citizen[candidate].obsol_tech, faction)) {
                    if (citizen_covers(candidate, specialist)) {
                        specialist = candidate;
                        base_specialist_set(citizen, specialist);
                        break;
                    }
                }
            }
        }
        for (int k = 0; k < 3; k++) {
            (&BaseCurrent->economy_total)[k] += Citizen[specialist].bonus[k];
        }
    }
    BaseCurrent->labs_total += Players[faction].rule_research;
    if (has_fac(FAC_BIOLOGY_LAB, BaseIDCurrentSelected, 0)) {
        BaseCurrent->labs_total += 2;
    }

    int labs_mult = 4;
    int economy_mult = 4;
    int psych_mult = 4;
    if (has_fac(FAC_ENERGY_BANK, BaseIDCurrentSelected, 0)
        || has_project(SP_PLANETARY_ENERGY_GRID, faction)) {
        economy_mult = 6;
    }
    if (has_fac(FAC_NETWORK_NODE, BaseIDCurrentSelected, 0)) {
        labs_mult = 6;
    }
    if (has_fac(FAC_HOLOGRAM_THEATRE, BaseIDCurrentSelected, 0)
        || (has_fac(FAC_NETWORK_NODE, BaseIDCurrentSelected, 0)
            && has_project(SP_VIRTUAL_WORLD, faction))) {
        psych_mult = 6;
    }
    if (has_fac(FAC_RESEARCH_HOSPITAL, BaseIDCurrentSelected, 0)) {
        psych_mult++;
        labs_mult += 2;
    }
    if (has_fac(FAC_NANOHOSPITAL, BaseIDCurrentSelected, 0)) {
        psych_mult++;
        labs_mult += 2;
    }
    if (has_fac(FAC_TREE_FARM, BaseIDCurrentSelected, 0)) {
        psych_mult += 2;
        economy_mult += 2;
    }
    if (has_fac(FAC_HYBRID_FOREST, BaseIDCurrentSelected, 0)) {
        psych_mult += 2;
        economy_mult += 2;
    }
    if (has_fac(FAC_FUSION_LAB, BaseIDCurrentSelected, 0)) {
        economy_mult += 2;
        labs_mult += 2;
    }
    if (has_fac(FAC_QUANTUM_LAB, BaseIDCurrentSelected, 0)) {
        economy_mult += 2;
        labs_mult += 2;
    }
    BaseCurrent->psych_total = (BaseCurrent->psych_total * psych_mult + 3) / 4;
    BaseCurrent->labs_total = (BaseCurrent->labs_total * labs_mult + 3) / 4;
    if (GameDataStateStorage.fields_.secret_project_[SP_SUPERCOLLIDER] == BaseIDCurrentSelected) {
        BaseCurrent->labs_total *= 2;
    }
    if (GameDataStateStorage.fields_.secret_project_[SP_THEORY_OF_EVERYTHING] == BaseIDCurrentSelected) {
        BaseCurrent->labs_total *= 2;
    }
    BaseCurrent->economy_total = (BaseCurrent->economy_total * economy_mult + 3) / 4;
    if (GameDataStateStorage.fields_.secret_project_[SP_SPACE_ELEVATOR] == BaseIDCurrentSelected) {
        BaseCurrent->economy_total *= 2;
    }
    if (GameDataStateStorage.fields_.secret_project_[SP_LONGEVITY_VACCINE] == BaseIDCurrentSelected
        && PlayersData[faction].soc_category_pending.models_[SocialCategory::ECONOMICS] == 1) {
        BaseCurrent->economy_total += BaseCurrent->economy_total / 2;
    }
    BaseCurrent->unk_10 = (BaseCurrent->unk_10 * economy_mult + 3) / 4;

    if (GameDataStateStorage.fields_.secret_project_[SP_NETWORK_BACKBONE] == BaseIDCurrentSelected) {
        for (int i = 0; i < GameDataStateStorage.fields_.base_current_count_; i++) {
            if (has_fac(FAC_NETWORK_NODE, i, 0)) {
                BaseCurrent->labs_total++;
            }
        }
        if (Players[faction].rule_flags & RFLAG_ALIEN) {
            BaseCurrent->labs_total += value;
        }
        for (int partner = 0; partner < MaxPlayerNum - 1; partner++) {
            if (!PlayersData[faction].sanction_turns && !PlayersData[partner + 1].sanction_turns
                && !(Players[faction].rule_flags & RFLAG_ALIEN)
                && !(Players[partner + 1].rule_flags & RFLAG_ALIEN)) {
                BaseCurrent->labs_total += BaseCommerceReceipts[partner];
            }
        }
    }

    int labs_share = PlayersData[faction].se_alloc_labs;
    int psych_share = PlayersData[faction].se_alloc_psych;
    int imbalance = psych_share + labs_share * 2 - 10;
    if (imbalance < 0) {
        imbalance = (5 - labs_share) * 2 - psych_share;
    }
    imbalance /= 2;
    int efficiency = PlayersData[faction].soc_effect_pending.values_[SocialEffect::EFFICIENCY];
    // The image evaluates the clamp three times (an abs()-style expansion).
    int penalty = (range(efficiency, -4, 4) - 4 >= 0
                   ? range(efficiency, -4, 4) - 4
                   : 4 - range(efficiency, -4, 4)) * 2;
    if (imbalance && penalty) {
        if (labs_share > 10 - PlayersData[faction].se_alloc_labs - PlayersData[faction].se_alloc_psych) {
            BaseCurrent->labs_total =
                ((100 - range(penalty * imbalance, 0, 40)) * BaseCurrent->labs_total + 50) / 100;
            BaseCurrent->economy_total =
                ((100 - range(penalty * imbalance * 2, 0, 80)) * BaseCurrent->economy_total + 50) / 100;
        } else {
            BaseCurrent->labs_total =
                ((100 - range(penalty * imbalance * 2, 0, 80)) * BaseCurrent->labs_total + 50) / 100;
            BaseCurrent->economy_total =
                ((100 - range(penalty * imbalance, 0, 40)) * BaseCurrent->economy_total + 50) / 100;
        }
    }
    if (BaseCurrent->queue_production_id[0] == -FAC_STOCKPILE_ENERGY
        && BaseCurrent->mineral_surplus > 0
        && BaseCurrent->production_id_last == -FAC_STOCKPILE_ENERGY) {
        BaseCurrent->economy_total +=
            has_project(SP_PLANETARY_ENERGY_GRID, BaseCurrent->faction_id_current)
            ? ((BaseCurrent->mineral_surplus + 1) / 2 * 5 + 3) / 4
            : (BaseCurrent->mineral_surplus + 1) / 2;
    }
    base_psych();
}

// The cached path has no result contract: its EAX is the selected id, while
// recomputation leaves the last resource pass's register residue. Callers use
// the published base fields, not a returned integer.
// ORIGINAL: 0x004EC3B0 ?base_compute@@YAHH@Z 0x004EC3B0-0x004EC3E7 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x004EC3B0-0x004EC3E7"], "reason": "Recover the original base-selection/cache operation with source-owned state and typed fields. The archived seed remains an obligation until this product definition is freshly remeasured."}
// symbol    ?base_compute@@YAXH@Z
void __cdecl base_compute(int force) {
    if (force || BaseComputedID != BaseIDCurrentSelected) {
        BaseComputedID = BaseIDCurrentSelected;
        base_support();
        base_yield();
        base_nutrient();
        base_minerals();
        base_energy();
    }
}

// ORIGINAL: 0x004EC3F0 ?base_connect@@YAHH@Z 0x004EC3F0-0x004ECB84
// RECOVERY_BINDING: v1 {"evidence": ["git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/004ec3f0.cpp original signature and complete span; independent pinned instruction review corrected magtube exception and destination population comparison; actual-source route and sensor controls."], "reason": "Base-domain connection planner follows same-region base paths and sensor fallback using existing typed Base, Map and Path owners."}
// TRIED: MISMATCH #0 'push' vs 'sub' (prologue diverges; framed build sim 0.63, first real divergence at #3 'mov'/'movsx'). Base-to-base connection pathfinder: transcribed from the Ghidra pseudocode's control flow, but re-derived every call's ARGUMENT LIST from the raw disassembly rather than trusting Ghidra's - two of its renderings were wrong (has_tech's 3rd arg 0xffffffff is a hallucinated leftover register, the real call pushes 2 args matching the 2-arg catalogue signature; and the sensor-distance comparison used `sensorX` where the source clearly reads Y). Simplified the `neg/sbb/and/add` "tech ? 0 : 4" guard-value idiom to a ternary for readability - MISMATCH-tier regardless of that choice given the size.
int __cdecl base_connect(int priority) {
    int found = 0;
    if (is_ocean(BaseCurrent->x, BaseCurrent->y)) {
        return 0;
    }
    int best_dist = 9999;
    int region = region_at(BaseCurrent->x, BaseCurrent->y);
    int best_x = -1;
    int best_y = -1;
    int x;
    int y;
    for (int i = 0; i < GameDataStateStorage.fields_.base_current_count_; i++) {
        if (i == BaseIDCurrentSelected) {
            continue;
        }
        if (region_at(Bases[i].x, Bases[i].y) != region) {
            continue;
        }
        int dist = map_range(BaseCurrent->x, BaseCurrent->y, Bases[i].x, Bases[i].y);
        if (Bases[i].faction_id_current != BaseCurrent->faction_id_current) {
            if (dist > 5
                || !(PlayersData[BaseCurrent->faction_id_current]
                         .diplo_treaties[Bases[i].faction_id_current] & DTREATY_PACT)) {
                continue;
            }
            dist += MapLongitudeBounds;
        } else if (dist > 8 && !has_fac(FAC_HEADQUARTERS, i, 0)) {
            // A far same-faction base is only a target when it is the largest
            // (and HQ-less) base of the faction in this region. Note: j == i is
            // not excluded.
            for (int j = 0; j < GameDataStateStorage.fields_.base_current_count_; j++) {
                if (Bases[j].faction_id_current == BaseCurrent->faction_id_current
                    && region_at(Bases[j].x, Bases[j].y) == region) {
                    if (Bases[j].population_size > Bases[i].population_size) {
                        goto next_base;
                    }
                    if (has_fac(FAC_HEADQUARTERS, j, 0)) {
                        goto next_base;
                    }
                }
            }
        }
        if (dist >= best_dist) {
            continue;
        }
        {
            int path_length = 0;
            x = BaseCurrent->x;
            y = BaseCurrent->y;
            int dir = Paths.find(x, y, Bases[i].x, Bases[i].y, 0, -1,
                has_tech(Rules.tech_build_roads_fungus, BaseCurrent->faction_id_current) ? 0 : 4,
                -1);
            while (dir >= 0) {
                x = wrap_map_x(x + VehMoveDeltaX[dir]);
                y += VehMoveDeltaY[dir];
                if (!on_map(x, y)) {
                    break;
                }
                if (x == Bases[i].x && y == Bases[i].y) {
                    found = 1;
                    goto path_done;
                }
                int veh_owner;
                int territory;
                if (base_who(x, y) < 0
                    && (!(map_loc(x, y)->bit & BIT_FUNGUS)
                        || (map_loc(x, y)->climate & ~0x1F) < ALT_BIT_OCEAN_SHELF
                        || has_tech(Rules.tech_build_roads_fungus, BaseCurrent->faction_id_current)
                        || has_tech(Terraforming[TERRA_REMOVE_FUNGUS].prerequisite_tech[0],
                                    BaseCurrent->faction_id_current))
                    && ((veh_owner = veh_who(x, y)) < 0
                        || veh_owner == BaseCurrent->faction_id_current
                        || (PlayersData[BaseCurrent->faction_id_current].diplo_treaties[veh_owner]
                            & DTREATY_PACT))
                    // The image tests the zone of control with `dec eax; jns`.
                    && zoc_sea(x, y, BaseCurrent->faction_id_current) - 1 < 0
                    && ((territory = whose_territory(BaseCurrent->faction_id_current, x, y,
                                                     NULL, FALSE)) < 0
                        || territory == BaseCurrent->faction_id_current
                        // BUG(original): the treaty is read from the territory
                        // owner's row (PlayersData[territory].diplo_treaties[faction]),
                        // the transpose of the vehicle-owner test above.
                        || (PlayersData[territory].diplo_treaties[BaseCurrent->faction_id_current]
                            & DTREATY_PACT))
                    && (!(map_loc(x, y)->bit & BIT_ROAD)
                        || ((map_loc(x, y)->bit & BIT_MAGTUBE)
                            && has_tech(Terraforming[TERRA_MAGTUBE].prerequisite_tech[0],
                                        BaseCurrent->faction_id_current)))) {
                    best_dist = dist;
                    best_x = x;
                    best_y = y;
                    break;
                }
                if (++path_length >= MapLongitudeBounds) {
                    break;
                }
                dir = Paths.find(x, y, Bases[i].x, Bases[i].y, 0, -1,
                    has_tech(Rules.tech_build_roads_fungus, BaseCurrent->faction_id_current) ? 0 : 4,
                    -1);
            }
        path_done:;
        }
    next_base:;
    }
    x = BaseCurrent->x;
    y = BaseCurrent->y;
    if (on_map(best_x, best_y)) {
        if (!found
            || (is_human(BaseCurrent->faction_id_current)
                && !(GameDataStateStorage.fields_.game_more_preferences_
                     & MPREF_AUTO_FORMER_BUILD_SENSORS))
            || Paths.sensors(BaseCurrent->faction_id_current, &x, &y)
            || vector_dist(BaseCurrent->x, BaseCurrent->y, x, y)
                >= vector_dist(BaseCurrent->x, BaseCurrent->y, best_x, best_y)) {
            // The image has one add_goal call (0x004ECB67): this arm pushes
            // best_x/best_y and jumps into the shared call. Routing it through
            // x/y gives the single call and the shared tail (0.985 -> 0.994).
            x = best_x;
            y = best_y;
        }
    } else if ((!is_human(BaseCurrent->faction_id_current)
                || (GameDataStateStorage.fields_.game_more_preferences_
                    & MPREF_AUTO_FORMER_BUILD_SENSORS))
               && !Paths.sensors(BaseCurrent->faction_id_current, &x, &y)) {
        // Empty arm: the image falls into the shared add_goal and keeps the
        // failure return last, behind the one it shares with the ocean test.
    } else {
        return 0;
    }
    add_goal(BaseCurrent->faction_id_current, AI_GOAL_TERRAFORM_LAND, priority, x, y,
             BaseIDCurrentSelected);
    return 1;
}

// Inlined twice into base_terraform (0x004EEA85 and 0x004EEBC4): how poorly
// suited a square is to farming (no rainfall, rocky, or flat and arid).
static inline int terraform_arid_count(int x, int y) {
    Map *sq = map_loc(x, y);
    int count = 0;
    uint8_t rain = sq->climate & 0x18;
    if (!rain) {
        count = 1;
    }
    int rocky = sq->val3 & 0xC0;
    if (rocky == 0x80) {
        count++;
    }
    if (!rocky && rain < 0x10) {
        count++;
    }
    return count;
}

// RECOVERY_ASSOCIATED: v1 switch_targets 0x004ECBF0 0x004EEF64 0x004EEF78 0x004EE82B
// ORIGINAL: 0x004ECBF0 ?base_terraform@@YAXH@Z 0x004ECBF0-0x004EEF61
// RECOVERY_BINDING: v1 {"evidence": ["git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/004ecbf0.cpp retained full native span", "Pinned 4ECBF0..4EEF61 instruction review establishes BaseIDCurrentSelected forced-improvement timer and pre-filter improvement-presence latches", "recovery/tests/test_base_terraform_behavior.py actual-source behavioral controls"], "reason": "Base-domain terraforming planner uses genuine Base, Map, Path and faction owners for tile scoring, improvement goals, forced actions and relocation. Historical seed and original extent remain preserved; source translation is not a byte-exact or runtime claim."}
// TRIED: Ghidra ported almost verbatim (DAT_0090ea30 recovered as a real `short *base16` on *BaseCurrent - offsets proven against the ASM at faction_id_current/population_size/state/governor/x/y/ mineral+energy_intake_1/unk_x/unk_y; the 15 FUN_ callees matched to their real, already-recovered names by prototype arity; two Ghidra call-site arg-grouping bugs fixed (abil_index/has_tech argument swap) - diverges at #6 'or eax,-1' vs 'xor eax,eax' register-idiom for the first of three `= 0xffffffff` locals, a compiler-idiom choice not obviously steerable from source.
void __cdecl base_terraform(int force_improve) {
    int mirror_score = -1;
    int bore_score = -1;
    int cond_score = -1;
    int rain_sum = 0;
    int temp_sum = 0;
    int bore_seen = 0;
    int cond_seen = 0;
    int faction = BaseCurrent->faction_id_current;
    int faction_bit = 1 << faction;
    int tech;
    int wealth;
    int power;
    if (GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & faction_bit) {
        uint32_t governor = BaseCurrent->governor;
        power = 0;
        tech = 0;
        wealth = 0;
        if (!(governor & GOV_PRIORITY_EXPLORE)) {
            if (governor & GOV_PRIORITY_DISCOVER) {
                tech = 1;
            } else if (governor & GOV_PRIORITY_BUILD) {
                wealth = 1;
            } else if (governor & GOV_PRIORITY_CONQUER) {
                power = 1;
            }
        }
    } else {
        tech = PlayersData[faction].ai_tech;
        wealth = PlayersData[faction].ai_wealth;
        power = PlayersData[faction].ai_power;
    }
    int pop_size = BaseCurrent->population_size;
    int energy_need = range(((wealth - power + tech + 4) * pop_size + 3) / 5
                                - BaseCurrent->intake_1[RSC_ENERGY] + BaseSquareEnergy,
                            0, 99);
    int mineral_need = range(power * 2 - BaseCurrent->intake_1[RSC_MINERALS] + pop_size + 1,
                             0, 99);
    if (!energy_need || energy_need + 1 <= mineral_need) {
        BaseCurrent->state &= ~BSTATE_UNK_1000;
    } else {
        BaseCurrent->state |= BSTATE_UNK_1000;
    }
    if (!mineral_need || mineral_need + 1 <= energy_need) {
        BaseCurrent->state &= ~BSTATE_UNK_2000;
    } else {
        BaseCurrent->state |= BSTATE_UNK_2000;
    }
    int mineral_low = range(mineral_need, 0, 2);
    int energy_low = range(energy_need, 0, 2);
    int food_want = range(3 - energy_low - mineral_low, 0, 3);
    int food_need;
    if (BaseCurrent->nutrient_surplus < food_want) {
        BaseCurrent->state |= BSTATE_UNK_4000;
        food_need = 1;
        if (BaseCurrent->nutrient_surplus < 0) {
            food_need = 2;
        }
    } else {
        BaseCurrent->state &= ~BSTATE_UNK_4000;
        food_need = 0;
    }

    // Survey of the base's own land squares.
    int sensors = 0;
    int unworked_rich = 0;
    int worked_farmable = 0;
    int unworked_good = 0;
    int i;
    for (i = 0; i < 21; i++) {
        int x = wrap_map_x(BaseCurrent->x + RadiusOffsetX[i]);
        int y = BaseCurrent->y + RadiusOffsetY[i];
        if (!on_map(x, y) || (map_loc(x, y)->climate & ~0x1F) < ALT_BIT_SHORE_LINE) {
            continue;
        }
        if (whose_territory(faction, x, y, NULL, FALSE) != faction) {
            continue;
        }
        if (is_sensor(x, y)) {
            sensors++;
        }
        Map *sq = map_loc(x, y);
        uint32_t items = sq->bit;
        if (items & BIT_FOREST) {
            continue;
        }
        uint32_t fungus = items & BIT_FUNGUS;
        if (fungus && (sq->climate & ~0x1F) >= ALT_BIT_OCEAN_SHELF) {
            continue;
        }
        if ((1 << i) & BaseCurrent->resource_sqr_radius) {
            if (items & BIT_FARM) {
                continue;
            }
            if ((sq->val3 & ~0x3F) >= 0x80) {
                continue;
            }
            if (fungus && (sq->climate & ~0x1F) >= ALT_BIT_OCEAN_SHELF) {
                continue;
            }
            if ((sq->climate & 0x18) >= 0x10
                && !has_tech(Rules.tech_three_nutrients_sqr, faction)
                && bonus_at(x, y, 0) != 1) {
                continue;
            }
            worked_farmable++;
        } else {
            if (((sq->climate >> 3) & 3) + (int)((items >> 15) & 1) < 2) {
                continue;
            }
            int free = TRUE;
            for (int b = 0; b < GameDataStateStorage.fields_.base_current_count_; b++) {
                if (Bases[b].faction_id_current != faction) {
                    continue;
                }
                // WORKAROUND(noinline): the image calls vector_dist(int, int)
                // (0x004F8090) out of line here and expands x_dist.
                int (__cdecl *const vector_dist_call)(int, int) = vector_dist;
                if (vector_dist_call(x_dist(x, Bases[b].x), abs(y - Bases[b].y)) > 2) {
                    continue;
                }
                // BUG(original): no range check on radius_move's result before
                // the shift, and the scan does not stop at the first hit.
                if ((1 << radius_move(Bases[b].x, Bases[b].y, x, y, 21))
                    & Bases[b].resource_sqr_radius) {
                    free = FALSE;
                }
            }
            if (!free) {
                continue;
            }
            unworked_good++;
            if ((map_loc(x, y)->val3 & 0xC0) || bonus_at(x, y, 0)) {
                unworked_rich++;
            }
        }
    }

    // Tidal harness site on the largest ocean region around the base.
    int ocean_tiles = 0;
    int best = 0;
    int site_y = -1;
    int site_x = -1;
    int best_region = -1;
    int score;
    for (i = 0; i < 21; i++) {
        int weight = 21 - i;
        int x = wrap_map_x(BaseCurrent->x + RadiusOffsetX[i]);
        int y = BaseCurrent->y + RadiusOffsetY[i];
        if (!on_map(x, y)) {
            continue;
        }
        Map *sq = map_loc(x, y);
        int alt = sq->climate & ~0x1F;
        if (alt >= ALT_BIT_SHORE_LINE) {
            continue;
        }
        ocean_tiles++;
        if (!i) {
            continue;
        }
        int region = sq->region;
        if (best_region >= 0 && region != best_region
            && Continents[region].tile_count <= Continents[best_region].tile_count) {
            continue;
        }
        best_region = region;
        if (alt != ALT_BIT_OCEAN_SHELF) {
            continue;
        }
        if (!can_terraform(faction, x, y, TRUE, BaseIDCurrentSelected)) {
            return;
        }
        score = weight;
        if (bonus_at(x, y, 0)) {
            score = weight + 21;
        }
        if (score > best) {
            site_x = x;
            site_y = y;
            best = score;
        }
    }
    if (on_map(site_x, site_y) && map_loc(site_x, site_y)->region == best_region) {
        add_goal(faction, AI_GOAL_TERRAFORM_WATER, 2, site_x, site_y, BaseIDCurrentSelected);
    }

    // Survey of rough and already improved land.
    int best_idx = -1;
    best = 0;
    BaseCurrent->unk_x = -1;
    int best_kind = 0;
    int rough_tiles = 0;
    BaseCurrent->unk_y = -1;
    int improved = 0;
    int worked_rough = 0;
    int worked_improved = 0;
    int specials = 0;
    int fungus_yield = PlayersData[faction].tech_fungus_[RSC_ENERGY]
        + PlayersData[faction].tech_fungus_[RSC_MINERALS]
        + PlayersData[faction].tech_fungus_[RSC_NUTRIENTS];
    for (i = 0; i < 21; i++) {
        int x = wrap_map_x(BaseCurrent->x + RadiusOffsetX[i]);
        int y = BaseCurrent->y + RadiusOffsetY[i];
        if (!(y >= 0 && y < (int)MapLatitudeBounds && x >= 0 && x < (int)MapLongitudeBounds)) {
            rain_sum++;
            continue;
        }
        Map *sq = map_loc(x, y);
        uint32_t climate = sq->climate;
        energy_low = climate & ~0x1F;
        if (energy_low < ALT_BIT_SHORE_LINE
            && !(force_improve && (Players[faction].rule_flags & RFLAG_AQUATIC))) {
            continue;
        }
        int temperature = climate & 7;
        int rain = (climate >> 3) & 3;
        int rocky = sq->val3 >> 6;
        int height = (int)(climate >> 5) - 2 < 0 ? 0 : (int)(climate >> 5) - 2;
        uint32_t items = sq->bit;
        uint32_t forest = items & BIT_FOREST;
        uint32_t fungus = items & BIT_FUNGUS;
        int land_fungus = fungus && energy_low >= ALT_BIT_OCEAN_SHELF;
        if (items & (BIT_CONDENSER | BIT_ECH_MIRROR | BIT_THERMAL_BORE)) {
            specials++;
        }
        if (i && (height + rocky + rain >= 4 || forest || (land_fungus && fungus_yield >= 4))) {
            rough_tiles++;
            uint32_t worked = (1 << i) & BaseCurrent->resource_sqr_radius;
            if (worked) {
                worked_rough++;
            }
            if (fungus && energy_low >= ALT_BIT_OCEAN_SHELF) {
                if (fungus_yield >= 4) {
                    // Counted without the worked-square tally the other kinds get.
                    improved++;
                } else if (!has_tech(Terraforming[TERRA_REMOVE_FUNGUS].prerequisite_tech[0],
                                     faction)) {
                    rough_tiles--;
                }
            } else if (forest
                       || (((items & BIT_FARM) || (rocky > 1 && (items & BIT_MINE)))
                           && (rocky > 1 || (items & BIT_SOLAR_TIDAL)
                               || ((items & BIT_MINE) && (items & BIT_FARM))))) {
                improved++;
                if (worked) {
                    worked_improved++;
                }
            }
        }
        rain_sum += rain;
        temp_sum += temperature; // never read
    }
    if (!(GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & faction_bit)
        && BaseCurrent->population_size >= 4) {
        rough_tiles--;
    }
    int wet = 0;
    if (rain_sum >= 15) {
        wet = 1;
    }
    if (rain_sum > 25) {
        wet++;
    }

    // Choose the square to improve and the borehole, condenser and raise-land sites.
    int high_tiles = 0;
    int home_region = map_loc(BaseCurrent->x, BaseCurrent->y)->region;
    int bore_x;
    int bore_y;
    int cond_x;
    int cond_y;
    for (i = 0; i < 21; i++) {
        int x = wrap_map_x(BaseCurrent->x + RadiusOffsetX[i]);
        int y = BaseCurrent->y + RadiusOffsetY[i];
        if (!on_map(x, y)) {
            continue;
        }
        uint32_t items = map_loc(x, y)->bit;
        if (items & BIT_MONOLITH) {
            continue;
        }
        if (items & BIT_THERMAL_BORE) {
            bore_seen = 1;
        }
        if (items & BIT_CONDENSER) {
            cond_seen = 1;
        }
        if (!i) {
            continue;
        }
        if ((items & BIT_FUNGUS) && (map_loc(x, y)->climate & ~0x1F) >= ALT_BIT_OCEAN_SHELF
            && !has_tech(Terraforming[TERRA_REMOVE_FUNGUS].prerequisite_tech[0], faction)) {
            continue;
        }
        if ((map_loc(x, y)->climate & ~0x1F) < ALT_BIT_SHORE_LINE
            && !(force_improve && (Players[faction].rule_flags & RFLAG_AQUATIC))) {
            continue;
        }
        if (map_loc(x, y)->region != home_region) {
            continue;
        }
        int zoc = zoc_move(x, y, faction);
        if (zoc >= 0) {
            if (zoc == 1) {
                continue;
            }
            // BUG(original): zoc 0 reads diplo_treaties[-1] (0x004ED6FA).
            if (PlayersData[faction].diplo_treaties[zoc - 1] & 0xC30) {
                continue;
            }
            if (anything_at(x, y) >= 0) {
                continue;
            }
        }
        int owner = whose_territory(faction, x, y, NULL, FALSE);
        if (owner >= 0 && owner != faction) {
            continue;
        }

        Map *sq = map_loc(x, y);
        int alt = sq->climate >> 5;
        items = sq->bit;
        uint32_t farm = items & BIT_FARM;
        int solar_or_bore = (items & BIT_SOLAR_TIDAL) || (items & BIT_THERMAL_BORE);
        int mine_or_bore = (items & BIT_MINE) || (items & BIT_THERMAL_BORE);
        uint32_t improvements = items & 0x1E00050;
        int food = crop_yield(faction, -1, x, y, TRUE);
        int rocky = map_loc(x, y)->val3 >> 6;
        int bonus = bonus_at(x, y, 0);
        food_want = (map_loc(x, y)->climate >> 3) & 3;
        int sensor = is_sensor(x, y);
        sq = map_loc(x, y);
        items = sq->bit;
        rain_sum = 0;
        if (items & (BIT_CONDENSER | BIT_ECH_MIRROR | BIT_THERMAL_BORE)) {
            rain_sum = 1;
        }
        uint32_t forest = items & BIT_FOREST;
        int fungus = ((items & BIT_FUNGUS) && (sq->climate & ~0x1F) >= ALT_BIT_OCEAN_SHELF) ? 4 : 0;
        uint32_t worked = (1 << i) & BaseCurrent->resource_sqr_radius;
        best_region = alt - 2 < 0 ? 0 : alt - 2;
        if (!worked && worked_improved == worked_rough && !unworked_rich && !forest && !fungus) {
            if (food_want && rocky < 2) {
                worked = 1;
            }
        } else {
            if (forest) {
                // Forest stays unless the square is good enough for something else.
                score = (sensor ? -1 : 1) + best_region + rocky + food_want;
                if (has_tech(Terraforming[TERRA_SOIL_ENR].prerequisite_tech[0], faction)) {
                    score++;
                }
                if (has_fac(FAC_TREE_FARM, BaseIDCurrentSelected, 0)) {
                    score--;
                }
                if (has_fac(FAC_HYBRID_FOREST, BaseIDCurrentSelected, 0)) {
                    score--;
                }
                if (score > 4) {
                    forest = 0;
                }
            }
            if (fungus) {
                int value = (sensor == 0) + best_region + rocky + food_want;
                if (value < fungus_yield) {
                    continue;
                }
                if (value == fungus_yield) {
                    fungus = 1;
                }
            }
        }

        // Sensor array.
        if (!sensors && !sensor && !bonus
            && (!improvements || BaseCurrent->population_size >= 8)
            && (!(GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & faction_bit)
                || (GameDataStateStorage.fields_.game_more_preferences_
                    & MPREF_AUTO_FORMER_BUILD_SENSORS))
            && (!unworked_rich || worked_farmable >= BaseCurrent->population_size - 1
                || BaseCurrent->population_size >= 2)
            && (!fungus || has_tech(Rules.tech_improve_fungus_sqr, faction))
            && (rocky > 1 || forest || !food_want
                || (fungus && has_tech(Rules.tech_improve_fungus_sqr, faction)))) {
            score = (BaseCurrent->population_size + PlayersData[faction].ai_fight * 2 + 2) * 4;
            if (forest || fungus || !food_want) {
                score *= 2;
            }
            // is_coast (0x004E49D0) is inlined here.
            int coast = is_coast(x, y, TRUE);
            if (!coast) {
                score *= 2;
            } else if (coast >= 9) {
                score *= 2;
            }
            if (score > best) {
                best = score;
                best_idx = i;
                best_kind = 9;
            }
        }

        // Solar collector.
        if ((energy_need || (BaseCurrent->state & BSTATE_UNK_8000)) && !forest && !rain_sum
            && (!sensor || sensors > (worked ? 1 : 2))
            && (!mine_or_bore || (!mineral_need && best_region >= 2))) {
            int high = (energy_need > 1 && best_region > 1) ? 1 : 0;
            if (food >= (farm != 0) + 1 - high && best_region) {
                int value = best_region * 3;
                if (bonus == 2 || rocky == 1) {
                    value += 2;
                }
                if (bonus == 1) {
                    value += 2;
                } else if (bonus == 3) {
                    value += farm ? -1 : 1;
                }
                if (farm) {
                    value++;
                }
                if (rocky == 2) {
                    value -= 3;
                }
                if (map_loc(x, y)->bit & BIT_FOREST) {
                    value /= 2;
                }
                score = range(energy_need * 2, 1, 99) * value;
                if (worked) {
                    score *= 2;
                }
                if (fungus) {
                    score += BaseCurrent->eco_damage * fungus * 2;
                }
                if (worked || energy_need > 1) {
                    if (!farm && (bonus == 3 || !energy_need || food_need) && rocky < 2) {
                        if (food_need) {
                            score *= 2;
                        }
                        if (score > best) {
                            best = score;
                            best_idx = i;
                            best_kind = 0;
                        }
                    } else if (!solar_or_bore) {
                        if (score > best) {
                            best = score;
                            best_idx = i;
                            best_kind = 3;
                        }
                    } else if (!farm && food < 3 && rocky < 2) {
                        if (score > best) {
                            best = score;
                            best_idx = i;
                            best_kind = 0;
                        }
                    }
                }
            }
        }

        // Raise land: only one very high square may be considered.
        if (alt >= 5 && ++high_tiles > 1) {
            mirror_score = -99999;
        }
        if (!wet && mirror_score > -99999 && (mirror_score > -9999 || bonus == 1)) {
            score = RadiusOffsetX[i] * RadiusOffsetX[i] * 2;
            if (alt == 3) {
                if (rocky == 1) {
                    score += 3;
                }
                if (bonus == 1) {
                    score += 6;
                }
            } else {
                if (bonus != 1) {
                    score -= food * 3;
                }
                if (rocky == 1) {
                    score -= 4;
                }
            }
            int ocean_left = ocean_tiles;
            for (int r = 1, level = alt; r < alt + 1; r++, level--) {
                int found = FALSE;
                for (int k = RadiusRange[r - 1]; k < RadiusRange[r]; k++) {
                    int x2 = wrap_map_x(x + RadiusOffsetX[k]);
                    int y2 = y + RadiusOffsetY[k];
                    if (!on_map(x2, y2) || (map_loc(x2, y2)->climate >> 5) >= level) {
                        continue;
                    }
                    found = TRUE;
                    // Distance to the base, not to the candidate square.
                    int dist = vector_dist(x2, y2, BaseCurrent->x, BaseCurrent->y);
                    if (dist <= 2) {
                        score++;
                        if (dist <= 1 && (map_loc(x2, y2)->climate & ~0x1F) == ALT_BIT_OCEAN_SHELF) {
                            ocean_left--;
                        }
                    }
                }
                if (!found) {
                    break;
                }
            }
            if (!ocean_tiles || ocean_left > 0) {
                if (mine_or_bore) {
                    score -= 4;
                }
                if (solar_or_bore) {
                    score += 2;
                }
                if (bonus == 1) {
                    score += 8;
                }
                if (i < 8) {
                    score /= 2;
                }
                if (score > mirror_score) {
                    mirror_score = score;
                    site_x = x;
                    site_y = y;
                }
            }
        }

        // Mine.
        if ((!mineral_need && ((food_want < 1 - Rules.tgl_nutrient_effect_with_mine && food_want) || rocky))
            || (sensor && sensors <= (worked ? 1 : 2))
            || (solar_or_bore && (energy_need || best_region >= 2))
            || food_need || forest
            || (rocky == 1 && best_region >= 2 && mineral_need <= 1)
            || (food + mine_or_bore < (farm != 0) + 2 && rocky == 1)) {
            // Not a mine candidate; only the farm check below applies.
        } else {
            switch (rocky) {
              case 0:
                score = food + 1;
                break;
              case 1:
                score = 3 - food;
                break;
              case 2:
                score = 6;
                break;
            }
            if (bonus == 1) {
                score += 4;
            } else if (bonus == 2) {
                score *= 2;
            } else if (bonus == 3) {
                score += 2;
            }
            if (farm) {
                score++;
            }
            Map *sq2 = map_loc(x, y);
            uint32_t items2 = sq2->bit;
            uint32_t road = items2 & BIT_ROAD;
            if (road) {
                score++;
            }
            if (items2 & BIT_FOREST) {
                score--;
            }
            score = score * mineral_need * 3;
            if (worked) {
                score *= 2;
            }
            if (fungus) {
                score += BaseCurrent->eco_damage * fungus * 2;
            }
            if (worked || mineral_need > 1 || (food_want && !rocky)) {
                if (!mine_or_bore && !rain_sum) {
                    if (score > best) {
                        best = score;
                        best_idx = i;
                        best_kind = 2;
                    }
                } else if (!road && (mine_or_bore || (items2 & BIT_THERMAL_BORE))
                           && !((GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_
                                 & faction_bit)
                                && (GameDataStateStorage.fields_.game_more_preferences_
                                    & MPREF_AUTO_FORMER_CANT_BUILD_ROADS))) {
                    if (score > best) {
                        best = score;
                        best_idx = i;
                        best_kind = 5;
                    }
                } else if (mine_or_bore && !rain_sum && !farm && rocky < 2
                           && ((sq2->climate & 0x18) || bonus == 1) && score > best) {
                    best = score;
                    best_idx = i;
                    best_kind = 0;
                }
            }
        }

        // Farm.
        if (!farm && rocky < 2 && !forest
            && (food_need > 1 || !((rocky && food) ? unworked_rich : unworked_good) || worked)
            && (food_need || (mineral_need && rocky && food)
                || (food_want && (rocky == 1 || food_want == 2 || mine_or_bore || solar_or_bore)))) {
            int value = food * 3 + 3;
            if (bonus > 1) {
                value += 3;
            }
            uint32_t items3 = map_loc(x, y)->bit;
            if (items3 & BIT_RIVER) {
                value += energy_need + 1;
            }
            if (rocky == 1) {
                value += mineral_need + 1;
            }
            if (items3 & BIT_FOREST) {
                value--;
            }
            score = range(food_need + 1, 1, 99) * value * 2;
            if (mineral_need && rocky) {
                score *= 2;
            }
            if (worked) {
                score *= 2;
            }
            if (food_want > 1 && !has_tech(Rules.tech_three_nutrients_sqr, faction)) {
                score /= 3;
            }
            if (fungus) {
                score = score / 2 + BaseCurrent->eco_damage * fungus * 2;
            }
            if (score > best) {
                best = score;
                best_idx = i;
                best_kind = 0;
            }
        }

        // Remove fungus.
        Map *sq3 = map_loc(x, y);
        if ((sq3->bit & BIT_FUNGUS) && (sq3->climate & ~0x1F) >= ALT_BIT_OCEAN_SHELF) {
            score = BaseCurrent->eco_damage * fungus * 2;
            if (score > best) {
                best = score;
                best_idx = i;
                best_kind = 10;
            }
        }

        // Thermal borehole.
        if (!bore_seen) {
            score = (rocky == 2 ? 12 : 0) - i + 21;
            score += (rocky == 2 || !food_want) ? 12 : 0;
            if (improvements) {
                score -= 8;
            }
            for (int k = 0; k < 8; k++) {
                int x2 = wrap_map_x(x + VehMoveDeltaX[k]);
                int y2 = y + VehMoveDeltaY[k];
                if (!on_map(x2, y2)) {
                    continue;
                }
                Map *sq2 = map_loc(x2, y2);
                if ((sq2->climate & ~0x1F) < ALT_BIT_SHORE_LINE) {
                    continue;
                }
                if ((sq2->climate >> 5) < alt) {
                    score = -1;
                }
                if (sq2->bit & BIT_THERMAL_BORE) {
                    score = -1;
                }
            }
            if (score > bore_score) {
                bore_score = score;
                bore_y = y;
                bore_x = x;
            }
        }

        // Condenser.
        if (!cond_seen) {
            score = 21 - i;
            if (!improvements) {
                score += 8;
            }
            if (!farm) {
                score += 2;
            }
            score += ((2 - food_want) * (2 - food_want) + rocky * 2) * 2;
            if (!MapCloudCover && food_want < 2) {
                score *= 2;
            }
            if (score > cond_score) {
                cond_score = score;
                cond_x = x;
                cond_y = y;
            }
        }
    }

    int road_priority = 1;
    if (has_tech(Terraforming[TERRA_MAGTUBE].prerequisite_tech[0], faction)) {
        road_priority = 2;
    }
    if (best_idx > 0) {
        int x = wrap_map_x(BaseCurrent->x + RadiusOffsetX[best_idx]);
        int y = BaseCurrent->y + RadiusOffsetY[best_idx];
        if (best_kind == 9) {
            add_goal(faction, AI_GOAL_SENSOR_ARRAY, 2, x, y, BaseIDCurrentSelected);
        } else if (best_kind == 5 || can_terraform(faction, x, y, TRUE, BaseIDCurrentSelected)) {
            add_goal(faction, AI_GOAL_TERRAFORM_LAND, 2, x, y, BaseIDCurrentSelected);
        } else {
            road_priority = 2;
        }
        if (!(GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & faction_bit)
            || !(GameDataStateStorage.fields_.game_more_preferences_
                 & MPREF_AUTO_FORMER_CANT_BUILD_ROADS)) {
            base_connect(road_priority);
        }
        if (force_improve) {
            // Instant terraforming.
            map_loc(x, y)->bit &= ~BIT_FUNGUS;
            int order = can_terraform(faction, x, y, TRUE, BaseIDCurrentSelected);
            if (ExpansionEnabled && (Players[faction].rule_flags & RFLAG_AQUATIC)) {
                order = (timeGetTime() + BaseIDCurrentSelected + y + x) & 3;
                if (order) {
                    order++;
                }
                order += 4;
            }
            switch (order) {
              case ORDER_PLANT_FOREST:
                map_loc(x, y)->bit |= BIT_FOREST;
                break;
              case ORDER_FARM:
                map_loc(x, y)->bit |= BIT_FARM | BIT_ROAD;
                if (!(map_loc(x, y)->climate & 7) && (map_loc(x, y)->climate & ~0x1F) <= 0x60) {
                    map_loc(x, y)->bit |= BIT_MINE;
                } else {
                    map_loc(x, y)->bit |= BIT_SOLAR_TIDAL;
                }
                break;
              case ORDER_MINE:
                map_loc(x, y)->bit |= BIT_MINE | BIT_ROAD;
                if ((map_loc(x, y)->val3 & ~0x3F) < 0x80) {
                    map_loc(x, y)->bit |= BIT_FARM;
                }
                break;
              case ORDER_SOLAR_COLLECTOR:
                map_loc(x, y)->bit |= BIT_SOLAR_TIDAL | BIT_ROAD;
                // BUG(original): adds the solar collector a second time.
                if ((map_loc(x, y)->val3 & ~0x3F) < 0x80) {
                    map_loc(x, y)->bit |= BIT_SOLAR_TIDAL;
                }
                break;
            }
        }
    } else if (!(GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & faction_bit)
               || !(GameDataStateStorage.fields_.game_more_preferences_
                    & MPREF_AUTO_FORMER_CANT_BUILD_ROADS)) {
        base_connect(2);
    }

    int8_t pop = BaseCurrent->population_size;
    int no_raise;
    if (improved < pop + 1 && rough_tiles > improved) {
        no_raise = TRUE;
    } else {
        no_raise = FALSE;
        if (!bore_seen && bore_score > 0 && !BaseCurrent->eco_damage
            && terraform_arid_count(bore_x, bore_y) >= 1
            && (pop >= 8 || energy_need > 1
                || has_tech(Ability[abil_index(1)].preq_tech, faction)
                || GameDataStateStorage.fields_.turn_current_num_ > 200)
            && terrain_avail(TERRA_THERMAL_BORE, FALSE, faction)
            && (!(GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & faction_bit)
                || (faction == ConsoleGlobal.faction_id_
                    && (GameDataStateStorage.fields_.game_preferences_
                        & PREF_AUTO_FORMER_BUILD_ADV)))) {
            add_goal(faction, AI_GOAL_THERMAL_BOREHOLE, 1, bore_x, bore_y, BaseIDCurrentSelected);
            goto relocate;
        }
        if (!cond_seen && cond_score > 0 && !BaseCurrent->eco_damage
            && (!wet
                || (wet < 2 && !specials
                    && (food_need
                        || (terraform_arid_count(cond_x, cond_y) >= 1
                            && (has_tech(Ability[abil_index(1)].preq_tech, faction)
                                || GameDataStateStorage.fields_.turn_current_num_ > 200)))))
            && terrain_avail(TERRA_CONDENSER, FALSE, faction)
            && (!(GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & faction_bit)
                || (faction == ConsoleGlobal.faction_id_
                    && (GameDataStateStorage.fields_.game_preferences_
                        & PREF_AUTO_FORMER_BUILD_ADV)))) {
            add_goal(faction, AI_GOAL_CONDENSER, 1, cond_x, cond_y, BaseIDCurrentSelected);
            goto relocate;
        }
    }
    if (!((wet && energy_need <= 2) || mirror_score <= 0 || no_raise
          || ((GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & faction_bit)
              && (faction != ConsoleGlobal.faction_id_
                  || !(GameDataStateStorage.fields_.game_preferences_
                       & PREF_AUTO_FORMER_RAISE_LWR_TERRAIN)))
          || !terrain_avail(TERRA_RAISE_LAND, FALSE, faction))) {
        add_goal(faction, AI_GOAL_LANDING_SITE, 1, site_x, site_y, BaseIDCurrentSelected);
    }

relocate:
    {
        int x = wrap_map_x(BaseCurrent->x - 8);
        int y = BaseCurrent->y;
        if (on_map(x, y) && (map_loc(x, y)->climate & ~0x1F) >= ALT_BIT_SHORE_LINE && wet
            && (map_loc(x, y)->climate & ~0x1F) < ALT_BIT_3_LEVELS_ABOVE_SEA) {
            BaseCurrent->unk_x = (int16_t)x;
            BaseCurrent->unk_y = (int16_t)y;
        } else {
            BaseCurrent->unk_x = -1;
            BaseCurrent->unk_y = -1;
        }
    }
    if (wet != 2) {
        best = wrap_map_x(BaseCurrent->x + 8);
        energy_low = BaseCurrent->y;
        // BUG(original): the guard is 25 or 50, never zero, so this raise-land
        // goal east of the base is never set.
        if (!(GameDataStateStorage.fields_.turn_current_num_ >= wet ? 25 : 50)
            && on_map(best, energy_low) && (map_loc(best, energy_low)->climate & ~0x1F) >= ALT_BIT_SHORE_LINE
            && (map_loc(best, energy_low)->climate & ~0x1F) <= ALT_BIT_2_LEVELS_ABOVE_SEA) {
            int occupant = anything_at(best, energy_low);
            if (occupant < 0 || occupant == faction) {
                int base_id = base_find(best, energy_low);
                if (base_id < 0 || base_id == BaseIDCurrentSelected
                    || (Bases[base_id].faction_id_current != faction
                        && !(PlayersData[faction].diplo_treaties[Bases[base_id].faction_id_current]
                             & (DTREATY_PACT | DTREATY_TREATY | DTREATY_TRUCE)))) {
                    if ((!(GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & faction_bit)
                         || (faction == ConsoleGlobal.faction_id_
                             && (GameDataStateStorage.fields_.game_preferences_
                                 & PREF_AUTO_FORMER_RAISE_LWR_TERRAIN)))
                        && terrain_avail(TERRA_RAISE_LAND, FALSE, faction)) {
                        add_goal(faction, AI_GOAL_LANDING_SITE, 2, best, energy_low, BaseIDCurrentSelected);
                    }
                }
            }
        }
    }
}

/*
Purpose: Check what facility (if any) a base needs for additional population growth. Stand alone
         function unused in original game and likely optimized out.
// ORIGINAL: 0x004EEF80 ?pop_goal_fac@@YAHH@Z 0x004EEF80-0x004EF087 BYTE_EXACT
// size      263 bytes
// prototype int (__cdecl ?pop_goal_fac@@YAHH@Z)(int baseID)
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0050BA00
// LEVER: both has_fac_built(FAC_HAB_COMPLEX/FAC_HABITATION_DOME, base_id)
//        sites routed through has_fac_built_call (the same lever pop_goal,
//        its twin below, already records) - the folded `MEASURED inline
//        bitmask` turns the two tests into bare `test byte ptr`, where the
//        image makes a real `call 0x50ba00` and keeps the offset/mask local
//        pair. Compiled body 54 -> 81 instructions against the image's 102.
Return Value: Facility id needed for pop growth or zero if base already has Hab Complex and Dome.
Status: Complete
*/
int __cdecl pop_goal_fac(int base_id) {
    uint32_t faction_id = Bases[base_id].faction_id_current;
    uint32_t limit_mod = has_project(SP_ASCETIC_VIRTUES, faction_id) ? 2 : 0;
    if (Bases[base_id].population_size - (int)limit_mod + Players[faction_id].rule_population
            >= (int)Rules.pop_limit_sans_hab_complex
        && !has_fac(FAC_HAB_COMPLEX, base_id, 0)) {
        return FAC_HAB_COMPLEX;
    }
    if (Bases[base_id].population_size - (int)limit_mod + Players[faction_id].rule_population
            >= (int)Rules.pop_limit_sans_hab_dome
        && !has_fac(FAC_HABITATION_DOME, base_id, 0)) {
        return FAC_HABITATION_DOME;
    }
    return 0; // Pop hasn't reached capacity or Base already has Hab Complex and Dome
}

/*
Purpose: Calculate population goal for a base.
// ORIGINAL: 0x004EF090 ?pop_goal@@YAHH@Z 0x004EF090-0x004EF1B2 BYTE_EXACT
// size      290 bytes
// prototype int (__cdecl ?pop_goal@@YAHH@Z)(int baseID)
// callers   1   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0050BA00
// LEVER: both has_fac_built(FAC_HAB_COMPLEX/FAC_HABITATION_DOME, base_id)
//        sites routed through the new `has_fac_built_call` helper (this
//        file, near cost_factor) so bitmask() emits a real `call 0x50ba00`
//        with an [ebp-4]/[ebp-8] offset/mask local pair, matching the
//        image's two `push 0x18`/`push 0x19` + call sequences instead of
//        folding to a byte test. Best similarity 0.910 (38/105 agreeing,
//        /c /O2 /Gy /GR- /Oy- /GX) - the largest jump of the bitmask-call
//        family (cost_factor TRIED the same lever; see that note).
Return Value: Goal population
Status: Complete
*/
int __cdecl pop_goal(int base_id) {
    uint32_t faction_id = Bases[base_id].faction_id_current;
    int limit_mod = has_project(SP_ASCETIC_VIRTUES, faction_id) ? 2 : 0;
    int goal = max((36 - Bases[base_id].population_size) / 6 + Bases[base_id].population_size, 6);
    if (!has_fac(FAC_HAB_COMPLEX, base_id, 0)) {
        goal = min(goal, (int)(Rules.pop_limit_sans_hab_complex - Players[faction_id].rule_population + limit_mod));
    }
    if (!has_fac(FAC_HABITATION_DOME, base_id, 0)) {
        goal = min(goal, (int)(Rules.pop_limit_sans_hab_dome - Players[faction_id].rule_population + limit_mod));
    }
    return goal;
}

// WORKAROUND(inline): dead budget probe (folds to no code). base_growth's two
// pop_goal_fac expansions call 0x00421670 for all four inner has_fac tests in
// the image (has_fac x4, then the TREE_FARM test expanded); with base.h's
// one-level has_fac one inner test still expands unless this spend follows the
// second pop_goal_fac site.
static inline int bg_budget_probe(int x) {
    if (x == 1) {
        x = 2;
    }
    return x;
}

// ORIGINAL: 0x004EF1C0 ?base_growth@@YAHXZ 0x004EF1C0-0x004EFB4E
// RECOVERY_BINDING: v1 {"evidence": ["git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/004ef1c0.cpp", "recovery/tasks/base-colony-economy-coverage-wave.json", "Pinned original01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf; complete original function extent reviewed against native instructions and VC6 CodeView owners.", "recovery/tests/test_base_colony_production.py", "Native4EF3CE/4EF50E selects BaseIDCurrentSelected for has_fac, contradicting the retained TRIED owner-byte hypothesis. 9A6548 is SecretProject.ascetic_virtues(+34), not network_backbone; 97D046 is population_size(+6)."], "reason": "Recover starvation, population growth/limits and nutrient warnings through real Base/faction/facility/project/vehicle owners. Preserve callback reloads and native signed-byte population behavior; seeds remain, no matching or runtime claim."}
// symbol    ?base_growth@@YAHXZ
// TRIED: DAT_0090ea30 is Ghidra-typed `short*` throughout, so every `DAT_0090ea30[N]`/`DAT_0090ea30+N` is byte offset N*2 off the Base* (confirmed against raw disasm: index 0x1e/0x68 -> bytes 0x3c/0xd0, index 3 -> byte 6, the growth-stage counter); two has_fac() calls use the local `owner` byte, not DAT_00689370, per raw push order (Ghidra swapped these). Compiles; diverges at #3 (prologue field-read order).
int __cdecl base_growth() {
    const int owner = BaseCurrent->faction_id_current;
    const int x = BaseCurrent->x;
    const int y = BaseCurrent->y;

    if (BaseCurrent->nutrients_accumulated < 0) {
        if ((GameDataStateStorage.fields_.game_state_ & 1) == 0
            || (GameDataStateStorage.fields_.game_state_ & 8) != 0) {
            if (BaseCurrentConvoyFrom[0] != 0) {
                // Supply crawlers of this base convoying nutrients from inside a
                // base square stop convoying.
                for (int veh_id = 0;
                     veh_id < GameDataStateStorage.fields_.vehicle_current_count_;
                     veh_id++) {
                    if (Vehs[veh_id].home_base_id == BaseIDCurrentSelected
                        && VehPrototypes[Vehs[veh_id].proto_id].plan == 0xA
                        && Vehs[veh_id].order == 3
                        && Vehs[veh_id].order_auto_type == 0
                        && base_who(Vehs[veh_id].x, Vehs[veh_id].y) >= 0) {
                        Vehs[veh_id].order = 0;
                    }
                }
            }
            if (Players[owner].rule_flags & RFLAG_ALIEN) {
                popb("STARVE", 0x4000, 0xE, "al_starv_sm.pcx", 0);
            } else {
                popb("STARVE", 0x4000, 0xE, "starv_sm.pcx", 0);
            }
            if (!is_objective(BaseIDCurrentSelected)
                || BaseCurrent->population_size > 1) {
                BaseCurrent->population_size--;
                if (BaseCurrent->population_size <= 0) {
                    base_kill(BaseIDCurrentSelected);
                    draw_map(1);
                    eliminate_player(owner, 0);
                    return 1;
                }
            }
        }
        BaseCurrent->nutrients_accumulated = BaseCurrent->nutrient_surplus;
        draw_tile(x, y, 2);
        return 0;
    }

    int growth = cost_factor(owner, 0, BaseIDCurrentSelected)
        * (BaseCurrent->population_size + 1);
    if ((has_project(SP_CLONING_VATS, owner) || BaseCurrentGrowthRate >= 6)
        && Rules.nutrient_req_citizen != 0
        && BaseCurrent->nutrient_surplus >= static_cast<int>(Rules.nutrient_req_citizen)
        && !pop_goal_fac(BaseIDCurrentSelected)) {
        // Population boom.
        if (BaseCurrent->population_size < 127) {
            BaseCurrent->population_size++;
        }
        base_compute(1);
        draw_tile(x, y, 2);
        if (BaseCurrent->nutrients_accumulated > growth) {
            BaseCurrent->nutrients_accumulated = growth;
        }
        return 0;
    }

    if (BaseCurrent->nutrients_accumulated >= growth) {
        if (BaseCurrentGrowthRate <= -3) {
            BaseCurrent->nutrients_accumulated = growth;
            return 0;
        }
        int limit_facility = pop_goal_fac(BaseIDCurrentSelected);
        bg_budget_probe(0);  // WORKAROUND(inline): see bg_budget_probe
        if (limit_facility) {
            BaseCurrent->nutrients_accumulated =
                cost_factor(BaseCurrent->faction_id_current, 0, BaseIDCurrentSelected)
                * (BaseCurrent->population_size + 1) / 2;
            parse_say(1, Facility[limit_facility].name, -1, -1);
            if (Players[owner].rule_flags & RFLAG_ALIEN) {
                popb("POPULATIONLIMIT", 0x800, 0x10, "al_pop_sm.pcx", 0);
            } else {
                popb("POPULATIONLIMIT", 0x800, 0x10, "pop_sm.pcx", 0);
            }
        } else {
            BaseCurrent->nutrients_accumulated = 0;
            BaseCurrent->population_size++;
            base_compute(1);
            draw_tile(x, y, 2);
        }
    }

    BaseCurrent->nutrients_accumulated =
        BaseCurrent->nutrients_accumulated + BaseCurrent->nutrient_surplus;
    // Low-nutrient warning: starving within five turns.
    if (BaseCurrent->nutrient_surplus < 0
        && BaseCurrent->nutrients_accumulated - BaseCurrent->nutrient_surplus >= 0
        && BaseCurrent->nutrients_accumulated + BaseCurrent->nutrient_surplus * 5 < 0
        && (BaseCurrent->governor & 0x80000040u) != 0x80000040u) {
        int improvable = 0;  // unworked/fixable squares a former could farm
        int bad_farms = 0;   // BUG(original): counted but never read
        for (int i = 1; i < 21; i++) {
            int x2 = wrap_map_x(x + RadiusOffsetX[i]);
            int y2 = y + RadiusOffsetY[i];
            if (!on_map(x2, y2)) {
                continue;
            }
            if (is_ocean(x2, y2)) {
                if ((BaseCurrent->resource_sqr_radius & (1 << i))
                    && !(bit_at(x2, y2) & BIT_FARM)) {
                    bad_farms++;
                }
                continue;
            }
            if (bit_at(x2, y2) & BIT_FARM) {
                continue;
            }
            if ((bit_at(x2, y2) & BIT_FOREST)
                && has_fac(FAC_TREE_FARM, BaseIDCurrentSelected, 0)) {
                continue;
            }
            int bits = bit_at(x2, y2);
            if ((bits & BIT_FUNGUS) && (map_loc(x2, y2)->climate & ~0x1F) >= 0x40
                && PlayersData[owner].tech_fungus_[RSC_NUTRIENTS] > 1) {
                continue;
            }
            int climate = map_loc(x2, y2)->climate;
            // The image extracts rainfall with `shr` (unsigned) but tests
            // `climate & ~0x1F` with a signed `jge`.
            int rainfall = ((uint32_t)climate >> 3) & 3;
            if (BaseCurrent->resource_sqr_radius & (1 << i)) {
                if (((bits & BIT_FUNGUS) && (climate & ~0x1F) >= 0x40)
                    || (bits & BIT_FOREST)
                    || (map_loc(x2, y2)->val3 & ~0x3F) == 0x80
                    || !rainfall) {
                    bad_farms++;
                } else if (!(bits & BIT_FARM)
                           && (rainfall == 1
                               || has_tech(Rules.tech_three_nutrients_sqr, owner))) {
                    improvable++;
                }
            } else if (!(bits & BIT_FARM)
                       && (rainfall == 1
                           || has_tech(Rules.tech_three_nutrients_sqr, owner))) {
                improvable++;
            }
        }

        if (Players[owner].rule_flags & RFLAG_ALIEN) {
            if (improvable && has_tech(VehPrototypes[1].preq_tech, owner)) {
                if (PlayersData[owner].proto_id_active[1]) {
                    popb("LOWNUTRIENTFARMS", 0, -1, "al_starv_sm.pcx", 0);
                    return 0;
                }
                popb("LOWNUTRIENTFORMERS", 0, -1, "al_starv_sm.pcx", 0);
                return 0;
            }
            if (BaseCurrent->specialist_total > 1
                || (BaseCurrent->specialist_total != 0
                    && BaseCurrent->population_size < 3)) {
                if (BasePsychDrones[0]) {
                    popb("LOWNUTRIENTSPEC1", 0, -1, "al_starv_sm.pcx", 0);
                    return 0;
                }
                popb("LOWNUTRIENTSPEC2", 0, -1, "al_starv_sm.pcx", 0);
                return 0;
            }
            popb("LOWNUTRIENT", 0x80, 0x10, "al_starv_sm.pcx", 0);
            return 0;
        }
        if (improvable && has_tech(VehPrototypes[1].preq_tech, owner)) {
            if (PlayersData[owner].proto_id_active[1]) {
                popb("LOWNUTRIENTFARMS", 0, -1, "starv_sm.pcx", 0);
                return 0;
            }
            popb("LOWNUTRIENTFORMERS", 0, -1, "starv_sm.pcx", 0);
            return 0;
        }
        if (BaseCurrent->specialist_total > 1
            || (BaseCurrent->specialist_total != 0
                && BaseCurrent->population_size < 3)) {
            if (BasePsychDrones[0]) {
                popb("LOWNUTRIENTSPEC1", 0, -1, "starv_sm.pcx", 0);
                return 0;
            }
            popb("LOWNUTRIENTSPEC2", 0, -1, "starv_sm.pcx", 0);
            return 0;
        }
        popb("LOWNUTRIENT", 0x80, 0x10, "starv_sm.pcx", 0);
        // WORKAROUND(decl): a read that both paths fold away keeps the dead
        // bad_farms counter alive. The image keeps its stack slot and both
        // increments (inc [ebp-8] at 0x004EF7D7) but never reads it; without
        // a read VC6 drops the counter and the ocean-farm test (181 -> 75
        // instruction distance). Original spelling unknown; leaves one dead
        // load before the epilogue.
        if (bad_farms) return 0;
    }
    return 0;
}


// ORIGINAL: 0x004EFB50 ?do_upgrade@@YAXHHH@Z 0x004EFB50-0x004EFD4A FILE BYTE_EXACT
// LEVER: every fixed address the archive bound is a named model member -
//        0x0096D238 is veh.cpp's FactionProtoSeenCount, 0x0096CC00 is
//        faction.h's PlayersData[].energy_reserves, 0x009A64C8 is console.h's
//        ConsoleGroupCount, 0x0095284C/.+0x0A/.+0x0E/.+0x24 are Vehs[]
//        proto_id/faction_id/morale read through the record, 0x0097D08C/.-0x48
//        are Bases[] queue_size/faction_id_current, 0x009AB898 is
//        VehPrototypes[].flags, 0x009A64E8 is lock.h's LockEnableMask,
//        0x00939284 is game.h's LocalFaction and 0x009156B0 is console.h's
//        ConsoleGlobal. Nothing new declared but draw_map below.
// size      634 bytes
// prototype void (__cdecl ?do_upgrade@@YAXHHH@Z)(int factionID, int protoIDNew, int protoIDOld)
// callers   1   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004EFD50 0x005BF1F0 0x0064601D
// Purpose: Retire one prototype across the faction: pay energy_reerves for the
//          upgrade, re-equip every vehicle of the old design (and its entries
//          in every base's production queue), adjust morale for the Trained
//          ability moving either way, mark the new design in service and the
//          old one obsolete, then repaint if a human is watching.
// TRIED: hoisting the vehicle-loop counter into a named local `count` and
//        reading ConsoleGroupCount once; the image re-reads [0x9a64c8] at both
//        the test and the loop latch, so the local cost a second stack slot
//        (`sub esp, 8` against the image's single `push ecx`) and 80 agreeing
//        instructions. Reading it at both sites and reordering the prologue to
//        faction_offset, upgrade_cost, then the tally moved 3/169 -> 101/169.
// TRIED: reusing the `proto_id_new` parameter slot as the vehicle-loop counter
//        the way the image does (`mov dword ptr [ebp + 0xc], 0` at instruction
//        23 zeroes it, so the parameter is the counter and its original value
//        lives in a register) - the archive spells this as `a2_saved`, and
//        spelling it here is what is left between this body and the image.
// BUG IN THE ORIGINAL: the production-queue sweep tests queue_production_id
//          [0 .. queue_size] INCLUSIVE - one entry past the queue's own count
//          - because the loop condition is `k <= *countp`. Reproduced
//          deliberately; see has_fac's note at the top of this file for the
//          same defect in a different body.
// ?draw_map@@YAXH@Z (mapwin.h) - declared here rather than included, so this
// file's include graph and every claim that depends on it stay untouched.
void __cdecl draw_map(int draw_type);

void __cdecl do_upgrade(int faction_id, int proto_id_new, int proto_id_old) {
    int cost = upgrade_cost(faction_id, proto_id_new, proto_id_old)
        * PlayersData[faction_id].proto_id_active[proto_id_old] * 10;
    PlayersData[faction_id].energy_reserves -= cost;
    for (int i = 0; i < GameDataStateStorage.fields_.vehicle_current_count_; i++) {
        if (Vehs[i].faction_id == faction_id && Vehs[i].proto_id == proto_id_old) {
            PlayersData[faction_id].proto_id_active[proto_id_old]--;
            PlayersData[faction_id].proto_id_active[proto_id_new]++;
            Vehs[i].proto_id = (int16_t)proto_id_new;
            if (has_abil(proto_id_new, ABL_TRAINED) && !has_abil(proto_id_old, ABL_TRAINED)) {
                Vehs[i].morale = (uint8_t)range(Vehs[i].morale + 1, 0, 6);
            }
            if (!has_abil(proto_id_new, ABL_TRAINED) && has_abil(proto_id_old, ABL_TRAINED)) {
                Vehs[i].morale = (uint8_t)range(Vehs[i].morale - 1, 0, 6);
            }
        }
    }
    for (int j = 0; j < GameDataStateStorage.fields_.base_current_count_; j++) {
        if (Bases[j].faction_id_current == faction_id) {
            for (int k = 0; k <= Bases[j].queue_size; k++) {
                if (Bases[j].queue_production_id[k] == proto_id_old) {
                    Bases[j].queue_production_id[k] = proto_id_new;
                }
            }
        }
    }
    if (proto_id_new >= MaxVehProtoFactionNum && cost) {
        VehPrototypes[proto_id_new].flags |= 4;
    }
    if (proto_id_old >= MaxVehProtoFactionNum) {
        VehPrototypes[proto_id_old].flags &= ~PROTO_ACTIVE;
    }
    if ((GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & (1 << faction_id)) != 0) {
        draw_map(1);
    }
    if (faction_id == ConsoleGlobal.faction_id_) {
        ConsoleGlobal.update_data(0);
    }
}


// ORIGINAL: 0x004EFD50 ?upgrade_cost@@YAHHHH@Z 0x004EFD50-0x004EFE7F FILE BYTE_EXACT
// TRIED: original establishes a true EBP frame (flags: frame) because it spills the reassigned `protoIDInit` parameter to its own stack slot; several transliterations (ternary vs if/else for the augmentation, ternary vs branch for the three clamps, B-then-A vs A-then-B table read order) all landed the value in a GP register (ebp used as scratch, no `mov ebp, esp`) instead. Landed at MISMATCH #3 (first diff is the missing frame prologue); the rest of the instruction stream already lines up closely (same table indices, same clamp-via-mask shape for two of three diffs).
// LEVER: the four fixed-address tables the artifact bound are modelled rules
//        records - 0x009AB868+0x29/.30/.25/.26/.24 is VehPrototypes[]
//        cost/flags/weapon_id/armor_id/chassis_id, 0x0094AE6B is
//        Weapon[].cost, 0x0094F282 is Armor[].cost, 0x0094A378 is
//        Chassis[].speed, 0x009A656C is base_project(SP_NANO_FACTORY) and
//        0x0097D044 is Bases[].faction_id_current - so every `g_9xxxxx`
//        binding drops out in favour of a member read.
// TRIED: writing the `proto_id_init = cost` reassignment the archive spells,
//        to force the image's early store into the dead parameter's home
//        slot - see the BUG IN THE ORIGINAL note below; the honest form is
//        what is kept here.
// TRIED: named `init_slot`/`upg_slot` locals for the two prototype indices, to
//        break VC6's common-subexpression of `&VehPrototypes[slot]` - the image
//        keeps `slot * 0x34` in a register with the TABLE BASE as the
//        displacement (`mov cl, [esi + 0x9ab891]`), where this tree keeps the
//        record pointer in the register with the FIELD OFFSET as the
//        displacement (`mov dl, [ecx + 0x29]`). Same bytes modulo the
//        relocation, different register choice, and the divergence cascades
//        through all five field reads: 3 of 105 either way. Reproducing the
//        image's shape needs `slot * 0x34` spelled as a byte offset, which only
//        a cast of the array base spells; naming `init_slot` scored no better
//        than using the parameters directly, so the parameter form is kept.
// size      303 bytes
// prototype int (__cdecl ?upgrade_cost@@YAHHHH@Z)(int factionID, int protoIDInit, int protoIDUpg)
// callers   3   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
// Purpose: Price one prototype upgrade for a faction: the base prototype's
//          mineral cost (augmented 1.5x and squared when the slot is past the
//          64-slot shared block and carries no VEHFLAG bit 4), plus the
//          positive deltas of the weapon and armour costs and the chassis
//          speed, halved when the faction that owns SP_NANO_FACTORY is asking.
// BUG IN THE ORIGINAL, AS TRANSCRIBED IN THE ARCHIVE: the archive test is
//          `cost > 0x3f`, because it reuses the parameter slot for the cost
//          before the test. The image compares EAX - the prototype ID as the
//          caller passed it, loaded at 0x004EFD53 and never reloaded - against
//          0x40. A caller passing a cheap prototype in a slot past 0x40 pays
//          the unaugmented price here. The comparison is spelled against the
//          prototype ID, which is what the image does; only the parameter
//          reuse is not reproduced.
// Status:  transcription homed from src/unrecovered/004efd50.cpp

int __cdecl upgrade_cost(int faction_id, int proto_id_init, int proto_id_upg) {
    int init_slot = proto_id_init;
    int upg_slot = proto_id_upg;
    int cost = VehPrototypes[init_slot].cost;
    if (init_slot >= 0x40 && (VehPrototypes[init_slot].flags & 4) == 0) {
        cost += (cost + 1) / 2;
        cost = (cost + 1) * cost;
    }
    // Native 004EFDC7/004EFDF9/004EFE36 uses positive second-minus-third
    // component costs. Callers pass the new prototype second and old third;
    // the retained init/upg parameter names above are historical.
    // Each clamp is written on the difference itself (sub; sets; dec; and).
    int part_diff = (Weapon[VehPrototypes[init_slot].weapon_id].cost
                     - Weapon[VehPrototypes[upg_slot].weapon_id].cost < 0)
        ? 0
        : Weapon[VehPrototypes[init_slot].weapon_id].cost
              - Weapon[VehPrototypes[upg_slot].weapon_id].cost;
    part_diff += (Armor[VehPrototypes[init_slot].armor_id].cost
                  - Armor[VehPrototypes[upg_slot].armor_id].cost < 0)
        ? 0
        : Armor[VehPrototypes[init_slot].armor_id].cost
              - Armor[VehPrototypes[upg_slot].armor_id].cost;
    int chassis_diff = (Chassis[VehPrototypes[init_slot].chassis_id].speed
                        - Chassis[VehPrototypes[upg_slot].chassis_id].speed < 0)
        ? 0
        : Chassis[VehPrototypes[init_slot].chassis_id].speed
              - Chassis[VehPrototypes[upg_slot].chassis_id].speed;
    cost += part_diff + chassis_diff;
    if (has_project(SP_NANO_FACTORY, faction_id)) {
        cost /= 2;
    }
    return cost;
}

// ORIGINAL: 0x004EFE80 ?upgrade_prototype@@YAHHHHH@Z 0x004EFE80-0x004F045B
// RECOVERY_BINDING: v1 {"evidence": ["git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/004efe80.cpp", "recovery/tasks/base-colony-economy-coverage-wave.json", "Pinned original01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf; complete original function extent reviewed against native instructions and VC6 CodeView owners.", "recovery/tests/test_base_colony_production.py", "Native4EFE8E..94 rejects statebit1 set with bit8 clear; 4F03A8/4F0414 choose UPGRADEPROTOCOST2 when prototypeflag4 is clear. Ability iterator9AB540 reads abbreviation(+8)."], "reason": "Recover complete prototype-upgrade decision, naming/prompt and network dispatch through typed owners. Four arguments follow the native caller and body; five H tokens include the int return. Replace central executable-address forwarder, preserve seed and historical obligations, no new byte claim."}
// symbol    ?upgrade_prototype@@YAHHHHH@Z
// TRIED: near-verbatim transcription of the (clean) Ghidra output, substituting catalogued globals/callees; the leading `if (cost==0) { if(a4!=0) return 0; if(!bit) return 0; }` three-branch guard was collapsed via De Morgan into one `if (!(cost != 0 || (a4 == 0 && bit))) return 0;` - logically equivalent (confirmed against raw asm) but restructures the control flow, so it does not reproduce the original's branch-by-branch codegen. The four `popp(...)` calls needed the 2nd/4th arguments swapped from Ghidra's printed order (confirmed via push-order tracing: 1st param dereferences g_00691b0c rather than using the address itself). Reaches MISMATCH #10 ('je' vs 'jne'); did not chase the exact branch-for-branch shape further.
int __cdecl upgrade_prototype(int faction_id, int proto_id_new,
                                 int proto_id_old, int set_flag) {
    int flag_clear;
    int cost;
    int faction_bit;
    int minerals;
    int energy;
    if ((GameDataStateStorage.fields_.game_state_ & 1)
        && !(GameDataStateStorage.fields_.game_state_ & 8)) {
        goto fail;
    }
    cost = upgrade_cost(faction_id, proto_id_new, proto_id_old)
        * PlayersData[faction_id].proto_id_active[proto_id_old] * 10;
    if (!cost) {
        if (set_flag) {
            goto fail;
        }
        if (!(GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_
              & (1 << faction_id))) {
            goto fail;
        }
    }
    faction_bit = 1 << faction_id;
    if (!(GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & faction_bit)) {
        minerals = GameDataStateStorage.fields_.diff_level_current_ * cost / 10
            / (8 - PlayersData[GameDataStateStorage.fields_.ranking_faction_id_unk1_].ranking);
        energy = PlayersData[faction_id].energy_reserves;
        if (energy > PlayersData[GameDataStateStorage.fields_.ranking_faction_id_unk2_].energy_reserves) {
            if (energy >= 1000) {
                minerals /= 2;
            }
            if (energy >= 2000) {
                minerals = 0;
            }
        }
        if (cost - minerals <= (((VehPrototypes[proto_id_new].plan == 2)
                + PlayersData[faction_id].ai_power
                + PlayersData[faction_id].ai_tech) * 2 + 1) * energy / 8) {
            goto net_check;
        }
    } else {
        if (faction_id != ConsoleGlobal.faction_id_) {
            goto fail;
        }
        if (cost) {
            int i;
            StringTemp[0] = '\0';
            strcat(StringTemp, VehPrototypes[proto_id_old].veh_name);
            strcat(StringTemp, BufferJoinSpace);
            strcat(StringTemp, SocialWinOpenParenText);
            say_stats_2(StringTemp, proto_id_old);
            if (VehPrototypes[proto_id_old].ability_flags) {
                strcat(StringTemp, BufferJoinSpace);
                for (i = 0; i < MaxAbilityNum; i++) {
                    if (has_abil(proto_id_old, 1 << i)) {
                        strcat(StringTemp, table_text(Ability[i].abbreviation));
                    }
                }
            }
            strcat(StringTemp, SocialWinCloseParenText);
            parse_says(0, StringTemp, -1, -1);

            StringTemp[0] = '\0';
            strcat(StringTemp, VehPrototypes[proto_id_new].veh_name);
            strcat(StringTemp, BufferJoinSpace);
            strcat(StringTemp, SocialWinOpenParenText);
            say_stats_2(StringTemp, proto_id_new);
            if (VehPrototypes[proto_id_new].ability_flags) {
                strcat(StringTemp, BufferJoinSpace);
                for (i = 0; i < MaxAbilityNum; i++) {
                    if (has_abil(proto_id_new, 1 << i)) {
                        strcat(StringTemp, table_text(Ability[i].abbreviation));
                    }
                }
            }
            strcat(StringTemp, SocialWinCloseParenText);
            parse_says(1, StringTemp, -1, -1);

            parse_num(0, cost);
            parse_num(1, PlayersData[faction_id].energy_reserves);
            parse_num(2, PlayersData[faction_id].proto_id_active[proto_id_old]);
            flag_clear = !(VehPrototypes[proto_id_new].flags & 4);
            if (cost <= PlayersData[faction_id].energy_reserves) {
                if (!UpgradePromptAnswered) {
                    int result;
                    if (Players[faction_id].rule_flags & RFLAG_ALIEN) {
                        result = popp(ScriptPopupFile,
                            flag_clear ? "UPGRADEPROTO2" : "UPGRADEPROTO", 0,
                            Chassis[VehPrototypes[proto_id_new].chassis_id].triad == 2 ? "alair_sm.pcx"
                                : Chassis[VehPrototypes[proto_id_new].chassis_id].triad == 1 ? "navun_sm.pcx" : "alunit_sm.pcx", 0);
                    } else {
                        result = popp(ScriptPopupFile,
                            flag_clear ? "UPGRADEPROTO2" : "UPGRADEPROTO", 0,
                            Chassis[VehPrototypes[proto_id_new].chassis_id].triad == 2 ? "air_sm.pcx"
                                : Chassis[VehPrototypes[proto_id_new].chassis_id].triad == 1 ? "navun_sm.pcx" : "unit_sm.pcx", 0);
                    }
                    if (!result) {
                        goto fail;
                    }
                    if (result > 1) {
                        UpgradePromptAnswered = 1;
                    }
                }
            } else {
                goto no_energy;
            }
        }
        goto net_check;
    }
fail:
    return 0;
net_check:
    if ((faction_bit & GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_)
        && IsMultiplayerNet) {
        message_data(0x2411, 0, faction_id, proto_id_new, proto_id_old, 0);
        while (NetDaemonNet.receive()) {
        }
        return 1;
    }
    if (0) {
no_energy:
        // Image places this block out of line, after the network then-block.
        if (Players[faction_id].rule_flags & RFLAG_ALIEN) {
            if (set_flag && !(GameDataStateStorage.fields_.game_warnings_ & 4)) {
                goto fail;
            }
            popp(ScriptPopupFile,
                flag_clear ? "UPGRADEPROTOCOST2" : "UPGRADEPROTOCOST", 0,
                Chassis[VehPrototypes[proto_id_new].chassis_id].triad == 2 ? "alair_sm.pcx"
                    : Chassis[VehPrototypes[proto_id_new].chassis_id].triad == 1 ? "navun_sm.pcx" : "alunit_sm.pcx", 0);
            return 0;
        } else {
            if (set_flag && !(GameDataStateStorage.fields_.game_warnings_ & 4)) {
                goto fail;
            }
            popp(ScriptPopupFile,
                flag_clear ? "UPGRADEPROTOCOST2" : "UPGRADEPROTOCOST", 0,
                Chassis[VehPrototypes[proto_id_new].chassis_id].triad == 2 ? "air_sm.pcx"
                    : Chassis[VehPrototypes[proto_id_new].chassis_id].triad == 1 ? "navun_sm.pcx" : "unit_sm.pcx", 0);
            return 0;
        }
    }
    do_upgrade(faction_id, proto_id_new, proto_id_old);
    return 1;
}

// ORIGINAL: 0x004F0460 ?upgrade_prototypes@@YAXHH@Z 0x004F0460-0x004F064F FILE BYTE_EXACT
// LEVER: every table the archive bound by address is a named model record -
//        0x009AB868+0x24..0x30 is VehPrototypes[] chassis/weapon/armor/plan/
//        unk_1/obsolete_factions/flags, 0x0094AE68 Weapon[].offense_rating,
//        0x0094F280 Armor[].defense_rating, 0x0094A378/.9 Chassis[]
//        .speed/.triad - and 0x0096D238 is veh.cpp's FactionProtoSeenCount.
// TRIED: spelling the `has_abil` argument as the five ABL_ bits the image's
//        0x408418 literal is the OR of (POLICE_2X | ARTILLERY | ANTIGRAV_STRUTS
//        | DROP_POD | AMPHIBIOUS); kept as the explicit OR of the enum.
// LEVER: the weapon guard is `!= 0` with the rating test INLINE and the
//        `== 0` weapon-id test as the ELSE arm - the image's `je` jumps
//        forward over the rating compare to the weapon-id compare. Spelling it
//        the archive's way (`== 0` first, else the rating test) inverted the
//        branch and cost four instructions. Swapping the two has_abil calls
//        (proto_id first, then the slot) fixed the last operand-order
//        divergence: 130/141 -> SHAPE_EXACT 140/141.
// TRIED: `flags &= ~0x100` as a word write - the image narrows it to a single
//        `and byte ptr [.. + 0x9ab899], 0xfe` (the flags field's HIGH byte
//        only); this tree keeps `and word ptr [.. + 0x30], 0xfeff`. That is
//        the one instruction between this and BYTE_EXACT, and reaching it
//        needs a byte-typed view of the field's second byte.
// size      495 bytes
// prototype void (__cdecl ?upgrade_prototypes@@YAXHH@Z)(int factionID, int protoID)
// callers   1   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004EFE80 0x004F0460 0x00625E50
// Purpose: Upgrade one prototype to its current-era replacement: clear the
//          prototype's 0x100 "obsolete" marker, then walk the faction's 128
//          prototype slots for a same-plan, same-chassis successor that is no
//          worse in weapon, armour, speed or triad, and upgrade every unit of
//          the old design to it.
// The current declaration and recovered callee have four native arguments;
// five H tokens in ?upgrade_prototype@@YAHHHHH@Z include the int return type.
// The earlier interpretation below is superseded, not a live ABI workaround:
// BUG IN THE ORIGINAL (historical interpretation): the image calls the five-parameter
//          ?upgrade_prototype@@YAHHHHH@Z here with four pushes (add esp, 0x10
//          at 0x004F0620), so the callee's fifth parameter is read off the
//          caller's stack. Reproduced as a four-parameter call; see
//          pending_bodies.cpp for the forwarder that spells it the same way.

void __cdecl upgrade_prototypes(int faction_id, int proto_id) {
    int self_offset = proto_id * 0x34;
    // Signed 16-bit flags: VC6 narrows this to `and byte ptr [+0x31], 0xfe`.
    VehPrototypes[proto_id].flags &= ~0x100;
    for (int i = 0; i < 0x80; i++) {
        int other = i;
        if (other >= 0x40) {
            other = other % 0x40 + faction_id * 0x40;
        }
        if (other == proto_id) {
            continue;
        }
        if ((VehPrototypes[other].flags & 1) == 0) {
            continue;
        }
        if ((VehPrototypes[other].obsolete_factions & (1 << faction_id)) == 0) {
            continue;
        }
        if (VehPrototypes[other].plan != VehPrototypes[proto_id].plan) {
            continue;
        }
        if (VehPrototypes[other].unk_1 != VehPrototypes[proto_id].unk_1) {
            continue;
        }
        if (Chassis[VehPrototypes[other].chassis_id].triad
            != Chassis[VehPrototypes[proto_id].chassis_id].triad) {
            continue;
        }
        if (PlayersData[faction_id].proto_id_active[other] == 0) {
            continue;
        }
        int8_t self_weapon = Weapon[VehPrototypes[proto_id].weapon_id].offense_rating;
        if (self_weapon != 0) {
            if (self_weapon > Weapon[VehPrototypes[other].weapon_id].offense_rating) {
                continue;
            }
        } else if (VehPrototypes[proto_id].weapon_id != VehPrototypes[other].weapon_id) {
            continue;
        }
        if (has_abil(proto_id, ABL_POLICE_2X | ABL_ARTILLERY | ABL_ANTIGRAV_STRUTS
                                  | ABL_DROP_POD | ABL_AMPHIBIOUS)
            != has_abil(other, ABL_POLICE_2X | ABL_ARTILLERY | ABL_ANTIGRAV_STRUTS
                             | ABL_DROP_POD | ABL_AMPHIBIOUS)) {
            continue;
        }
        if (Armor[VehPrototypes[proto_id].armor_id].defense_rating
            > Armor[VehPrototypes[other].armor_id].defense_rating) {
            continue;
        }
        if (Chassis[VehPrototypes[proto_id].chassis_id].speed
            > Chassis[VehPrototypes[other].chassis_id].speed) {
            continue;
        }
        if (VehPrototypes[proto_id].chassis_id != VehPrototypes[other].chassis_id) {
            continue;
        }
        upgrade_prototype(faction_id, proto_id, other, 1);
    }
}

// ORIGINAL: 0x004F0650 ?upgrade_any_prototypes@@YAXH@Z 0x004F0650-0x004F06D3 FILE BYTE_EXACT
// LEVER: the four carried bindings are all modelled names - 0x0090EA3C is the
//        upgrade-confirmation flag (see UpgradePromptAnswered below),
//        0x009A6494 game.h's GameMorePreferences, 0x009A64E8 lock.h's
//        LockEnableMask read as its byte 0 (the is-human bitfield) and
//        0x009AB898 VehPrototypes[].flags - so no fixed address survives.
// TRIED: `int flags`, `uint16_t flags`, `(uint16_t)(flags & 0x100)` and a bare
//        `flags & 0x100` condition for the third bit test - the image spells it
//        `and ecx, 0x100 / test cx, cx` (two instructions) and this tree spells
//        every one of those source forms as `test ch, 1` (one). 46 of 47, one
//        instruction short, on all four spellings; the 16-bit `and`/`test` pair
//        is an idiom this source cannot be pushed to.
// size      131 bytes
// prototype void (__cdecl ?upgrade_any_prototypes@@YAXH@Z)(int factionID)
// callers   1   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004F0460
// Purpose: The turn-start automatic upgrade sweep. Clears the
//          upgrade-confirmation flag, then - for a faction that is human, or
//          whose "ask before upgrading" preference is off - walks all 128
//          prototype slots and upgrades every obsolete one still in service
//          (flags: in-use, bit 4, and 0x100).

void __cdecl upgrade_any_prototypes(int faction_id) {
    UpgradePromptAnswered = 0;
    if (((1 << faction_id) & GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_) == 0
        || (GameDataStateStorage.fields_.game_more_preferences_ & 0x100000) != 0) {
        int i = 0;
        do {
            int proto_id = i;
            if (proto_id >= 0x40) {
                proto_id = proto_id % 0x40 + faction_id * 0x40;
            }
            uint16_t flags = VehPrototypes[proto_id].flags;
            if ((flags & 1) != 0 && (flags & 4) != 0) {
                const int16_t obsolete = static_cast<int16_t>(flags & 0x100);
                if (obsolete != 0) {
                    upgrade_prototypes(faction_id, proto_id);
                }
            }
            ++i;
        } while (i < 0x80);
    }
}

/*
Purpose: Move the specified base's production queue forward if applicable.
// ORIGINAL: 0x004F06E0 ?base_queue@@YAHH@Z 0x004F06E0-0x004F07D8 BYTE_EXACT
// size      248 bytes
// prototype int (__cdecl ?base_queue@@YAHH@Z)(int baseID)
// callers   1   call targets   4
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004455F0 0x0046AEF0 0x005BA0E0 0x005BA910
// LEVER: `Base *base = &Bases[base_id];` HOISTED, rather than
//        `Bases[base_id]` re-subscripted at every field: the image computes
//        the base's address exactly ONCE (esi = &Bases[base_id]) and reuses
//        it for every field (queue_size, faction, queue_production_id, x,
//        y) through the whole function, where re-subscripting recomputes
//        (and partially re-folds) the base_id*sizeof(Base) multiply at each
//        site. Moved 0/89 (0.804) -> 15/89 (0.859).
Return Value: Is there a valid item in queue to be built? true/false
Status: Complete
*/
BOOL __cdecl base_queue(int base_id) {
    uint32_t faction_id = Bases[base_id].faction_id_current;
    while (Bases[base_id].queue_size) {
        if (Bases[base_id].queue_production_id[0] >= 0) {
            PlayersData[faction_id].proto_id_queue[Bases[base_id].queue_production_id[0]]--;
        }
        for (int i = 0; i < Bases[base_id].queue_size; i++) {
            Bases[base_id].queue_production_id[i] = Bases[base_id].queue_production_id[i + 1];
        }
        Bases[base_id].queue_size--;
        if (Bases[base_id].queue_production_id[0] >= 0) {
            if (!veh_avail(Bases[base_id].queue_production_id[0], faction_id, base_id)) {
                continue;
            }
        } else if (!facility_avail(-Bases[base_id].queue_production_id[0], faction_id, base_id, 0)) {
            continue;
        }
        if (Bases[base_id].queue_production_id[0] <= -FacilitySPStart) {
            wave_it(36); // CPU project initiated
        }
        draw_radius(Bases[base_id].x, Bases[base_id].y, 2, 2);
        return true;
    }
    return false;
}



// Shared separator objects the image pushes instead of unit literals.
extern const char BufferJoinSpace[];

extern int BaseProductionZoomRequested;
extern int BaseProductionUnitCreationDisabled;
extern uint32_t UpgradePromptAnswered;
int __cdecl tech_advance(int faction_id);
void __cdecl mon_secret_project(int faction_id, int project_id);
void __cdecl help_project(int project_id);
void __cdecl set_dirty();
int __cdecl generators(int faction_id, int *count_out);


static int base_yield_add(int left, int right);

enum ProductionReportResume {
    PRODUCTION_REPORT_RESUME_274F,
    PRODUCTION_REPORT_RESUME_276B,
    PRODUCTION_REPORT_RESUME_2752
};






























// ORIGINAL: 0x004F07E0 ?base_production@@YAXXZ 0x004F07E0-0x004F3FD8;0x0065C8A0-0x0065D0D6
// RECOVERY_BINDING: v1 {"evidence": ["original:0x004F07E0-0x004F3FD8;0x0065C8A0-0x0065D0D6", "git:59cb5d02:src/unrecovered/004f07e0.cpp", "source:src/base.h", "source:src/faction.h", "source:src/popup.h"], "reason": "Recover the complete colony production operation with real Base, PlayerData, Popup and project lifetimes; helper decomposition preserves redispatch, reporting, unit/facility creation and cleanup paths, including native wrapping arithmetic."}
void __cdecl base_production() {
    // The only class-typed local: its constructor runs first (0x4F0805) and
    // every return below expands ~Popup in place with its own EH states.
    Popup popup;
    int undersized;
    int height;
    int width;

    int faction_id = BaseCurrent->faction_id_current;
    BaseCurrent->state &= ~BSTATE_PRODUCTION_DONE;
    PlayersData[faction_id].unk_28 = max(PlayersData[faction_id].unk_28,
        BaseCurrent->intake_2[RSC_MINERALS] + BaseCurrent->mineral_surplus * 2);
    if (BaseCurrent->nutrients_accumulated < 0) {
        return;
    }
    if (BaseCurrent->mineral_surplus >= 0
        && BaseCurrent->queue_production_id[0] != -FAC_STOCKPILE_ENERGY
        && !(BaseCurrent->state & BSTATE_DRONE_RIOTS_ACTIVE)) {
        BaseCurrent->minerals_accumulated += BaseCurrent->mineral_surplus;
    }
    if (!is_human(faction_id)
        || (BaseCurrent->governor & GOV_MANAGES_PRODUCTION
            && BaseCurrent->governor & GOV_ACTIVE
            && !(BaseCurrent->state & 0x80000000u))) {
        if (BaseCurrent->state & BSTATE_COMBAT_LOSS_LAST_TURN
            || (veh_who(BaseCurrent->x, BaseCurrent->y) < 0
                && !is_human(faction_id))) {
            base_reset(BaseIDCurrentSelected, 0);
        }
    }

    // The queue loop re-enters here after a redundant facility is dropped
    // from the queue (0x4F0AB3). Stockpile Energy produces nothing; its
    // return is the second teardown site (state 7), ahead of the unit sites.
production_again:
    if (BaseCurrent->queue_production_id[0] == -FAC_STOCKPILE_ENERGY) {
        return;
    }
    if (BaseCurrent->queue_production_id[0] >= 0) {
        // ---- Unit production ----
        int proto_id = BaseCurrent->queue_production_id[0];
        int cost = cost_factor(BaseCurrent->faction_id_current, RSC_MINERALS, -1)
            * veh_cost(proto_id, BaseIDCurrentSelected, 0);
        if (BaseCurrent->minerals_accumulated < cost) {
            return;
        }
        if (VehPrototypes[proto_id].plan == PLAN_COLONIZATION
            && BaseCurrent->population_size == 1
            && PlayersData[faction_id].diff_level > DLVL_SPECIALIST) {
            if (!is_human(faction_id)) {
                if (GameDataStateStorage.fields_.diff_level_current_ > DLVL_LIBRARIAN) {
                    BaseCurrent->nutrients_accumulated +=
                        GameDataStateStorage.fields_.diff_level_current_ - 2;
                }
                return;
            }
            if (BaseCurrent->governor & GOV_MANAGES_PRODUCTION
                && BaseCurrent->governor & GOV_ACTIVE) {
                return;
            }
            if (GameDataStateStorage.fields_.game_state_ & 1
                && !(GameDataStateStorage.fields_.game_state_ & 8)) {
                return;
            }
            parse_says(0, BaseCurrent->name_string, -1, -1);
            parse_says(1, VehPrototypes[proto_id].veh_name, -1, -1);
            clear();
            says("ABANDONBASE");
            int pop_cost = cost_factor(BaseCurrent->faction_id_current, RSC_NUTRIENTS,
                                       BaseIDCurrentSelected)
                * (BaseCurrent->population_size + 1);
            if (BaseCurrent->nutrient_surplus > 0) {
                says("1");
                parse_num(0, range((pop_cost - BaseCurrent->nutrients_accumulated)
                                   / BaseCurrent->nutrient_surplus, 0, 9999));
            }
            int choice;
            if (Players[BaseCurrent->faction_id_current].rule_flags & RFLAG_ALIEN) {
                choice = IsMultiplayerNet ? 0
                    : popp(ScriptPopupFile, "ABANDONBASE", 0, "Alopdir.pcx", 0);
            } else {
                choice = IsMultiplayerNet ? 0
                    : popp(ScriptPopupFile, "ABANDONBASE", 0, "talent_sm.pcx", 0);
            }
            if (IsMultiplayerNet || choice < 2) {
                if (choice) {
                    BaseProductionZoomRequested = 1;
                }
                return;
            }
        }
        int new_proto;
        if (proto_id >= MaxVehProtoFactionNum) {
            new_proto = !(VehPrototypes[proto_id].flags & 4);
            VehPrototypes[proto_id].flags |= 4;
            if (new_proto) {
                int base_id = BaseIDCurrentSelected;
                prune_protos(faction_id, proto_id, 0);
                BaseIDCurrentSelected = base_id;
                BaseCurrent = &Bases[base_id];
                base_compute(0);
            }
        } else {
            new_proto = 0;
        }
        BaseCurrent->minerals_accumulated -= cost;
        if (BaseCurrent->minerals_accumulated
                > min(Rules.retool_exemption, BaseCurrent->mineral_surplus)
            && is_human(faction_id)) {
            BaseCurrent->minerals_accumulated =
                max(0, min(Rules.retool_exemption, BaseCurrent->mineral_surplus));
        }
        BaseCurrent->minerals_accumulated_2 = BaseCurrent->minerals_accumulated;
        if (!BaseProductionUnitCreationDisabled) {
        int veh_id = veh_init(proto_id, faction_id, BaseCurrent->x, BaseCurrent->y);
        if (veh_id < 0) {
            return;
        }
        if (VehPrototypes[proto_id].plan == PLAN_COLONIZATION) {
            if (BaseCurrent->state & 0x2000000
                || !PlayersData[faction_id].ai_power
                // The image reads current_num_bases (+0x105C) here (0x004F1147).
                || PlayersData[faction_id].current_num_bases > 8) {
                BaseCurrent->state |= 0x1000000;
            }
            BaseCurrent->state |= 0x2000000;
        }
        if (is_human(faction_id)
            && BaseCurrent->governor & GOV_NEW_VEH_FULLY_AUTO
            && BaseCurrent->governor & GOV_ACTIVE) {
            Vehs[veh_id].state |= VSTATE_UNK_200;
            Vehs[veh_id].order_auto_type = 0;
        }
        if (proto_id == 8 && faction_id == ConsoleGlobal.faction_id_
            && !(GameDataStateStorage.fields_.game_interlude_flags_ & 0x10)) {
            interlude(4, BaseCurrent->name_string, 1, 0);
            Vehs[veh_id].state |= 0x800000;
        }
        int morale;
        if (proto_id < MaxVehProtoFactionNum
            && Weapon[VehPrototypes[proto_id].weapon_id].offense_rating < 0) {
            morale = breed_mod(BaseIDCurrentSelected, faction_id);
            morale++;
        } else {
            morale = morale_mod(BaseIDCurrentSelected, faction_id,
                Chassis[VehPrototypes[proto_id].chassis_id].triad);
            if (VehPrototypes[proto_id].weapon_id == 21) {
                if (has_fac(FAC_COVERT_OPS_CENTER, BaseIDCurrentSelected, 0)) {
                    morale += 2;
                }
                if (has_project(SP_NETHACK_TERMINUS, faction_id)) {
                    morale++;
                }
            }
            if (has_abil(proto_id, ABL_TRAINED)) {
                morale++;
            }
            if (new_proto) {
                morale++;
            }
            morale++;
        }
        // The image increments the morale variable itself at the join
        // (0x4F13A6) and clamps that register, not a separate temporary.
        Vehs[veh_id].morale = (uint8_t)range(morale, MORALE_VERY_GREEN, MORALE_ELITE);
        Vehs[veh_id].state |= 0x2000;
        if (faction_id == ConsoleGlobal.faction_id_) {
            Vehs[veh_id].flags |= 0x1000;
        }
        if (VehPrototypes[proto_id].plan == PLAN_COLONIZATION) {
            if (BaseCurrent->nutrients_accumulated < 0) {
                BaseCurrent->nutrients_accumulated = 0;
            }
            if (BaseCurrent->population_size > 1
                || (PlayersData[faction_id].diff_level > DLVL_SPECIALIST
                    && !is_objective(BaseIDCurrentSelected))) {
                BaseCurrent->population_size--;
            }
            if (BaseCurrent->population_size <= 0) {
                Vehs[veh_id].home_base_id = -1;
                base_kill(BaseIDCurrentSelected);
                draw_map(1);
                eliminate_player(faction_id, 0);
                return;
            }
        }
        BaseCurrent->state |= BSTATE_PRODUCTION_DONE;
        BaseCurrent->state &= ~0x80000000u;
        if (!is_human(faction_id)) {
            base_reset(BaseIDCurrentSelected, 0);
        } else {
            int queued = base_queue(BaseIDCurrentSelected);
            parse_says(1, VehPrototypes[proto_id].veh_name, -1, -1);
            uint32_t warnings = GameDataStateStorage.fields_.game_warnings_;
            int popup_flags;
            if ((!queued && !(BaseCurrent->governor & GOV_MANAGES_PRODUCTION
                              && BaseCurrent->governor & GOV_ACTIVE))
                || (new_proto && warnings & 4) || warnings & 0x2000) {
                if (new_proto && warnings & 4) {
                    parse_num(0, cost_factor(BaseCurrent->faction_id_current,
                                             RSC_MINERALS, -1)
                                 * veh_cost(proto_id, BaseIDCurrentSelected, 0));
                    clear();
                    says("PRODUCEPROTO");
                    popup_flags = 4;
                } else {
                    clear();
                    if (VehPrototypes[proto_id].plan < PLAN_COLONIZATION) {
                        says("PRODUCE");
                        popup_flags = 0x400;
                    } else {
                        says("PRODUCE");
                        popup_flags = 2;
                    }
                }
            } else {
                clear();
                says("PRODUCE");
                popup_flags = 0;
            }
            if (queued) {
                parse_num(1, BaseCurrent->queue_size + 1);
                says("Q");
            } else if (BaseCurrent->governor & GOV_MANAGES_PRODUCTION
                       && BaseCurrent->governor & GOV_ACTIVE) {
                says("G");
            }
            // Each picture arm reloads the chassis triad as a byte and
            // compares it in place (cmp al,2 / cmp al,1).
            const char *picture;
            if (Players[faction_id].rule_flags & RFLAG_ALIEN) {
                if (proto_id < MaxVehProtoFactionNum
                    && (Weapon[VehPrototypes[proto_id].weapon_id].offense_rating < 0
                        || proto_id == 15)) {
                    if (Chassis[VehPrototypes[proto_id].chassis_id].triad == TRIAD_AIR) {
                        picture = "chiron_sm.pcx";
                    } else if (Chassis[VehPrototypes[proto_id].chassis_id].triad == TRIAD_SEA) {
                        picture = proto_id == 14 ? "sealrk_sm.pcx" : "isle_sm.pcx";
                    } else {
                        picture = "native_sm.pcx";
                    }
                } else if (Chassis[VehPrototypes[proto_id].chassis_id].triad == TRIAD_AIR) {
                    picture = "alair_sm.pcx";
                } else {
                    picture = Chassis[VehPrototypes[proto_id].chassis_id].triad == TRIAD_SEA
                        ? "navun_sm.pcx" : "alunit_sm.pcx";
                }
                popb(StringTemp, popup_flags, -1, picture, 0);
            } else {
                if (proto_id < MaxVehProtoFactionNum
                    && (Weapon[VehPrototypes[proto_id].weapon_id].offense_rating < 0
                        || proto_id == 15)) {
                    if (Chassis[VehPrototypes[proto_id].chassis_id].triad == TRIAD_AIR) {
                        picture = "chiron_sm.pcx";
                    } else if (Chassis[VehPrototypes[proto_id].chassis_id].triad == TRIAD_SEA) {
                        picture = proto_id == 14 ? "sealrk_sm.pcx" : "isle_sm.pcx";
                    } else {
                        picture = "native_sm.pcx";
                    }
                } else if (Chassis[VehPrototypes[proto_id].chassis_id].triad == TRIAD_AIR) {
                    picture = "air_sm.pcx";
                } else {
                    picture = Chassis[VehPrototypes[proto_id].chassis_id].triad == TRIAD_SEA
                        ? "navun_sm.pcx" : "unit_sm.pcx";
                }
                popb(StringTemp, popup_flags, -1, picture, 0);
            }
            if (!queued && BaseCurrent->governor & GOV_MANAGES_PRODUCTION
                && BaseCurrent->governor & GOV_ACTIVE) {
                base_reset(BaseIDCurrentSelected, 0);
            }
        }
        if (new_proto) {
            mon_prototype_built(faction_id);
            if (IsMultiplayerNet && is_human(faction_id)) {
                VehPrototypes[proto_id].flags |= 0x100;
            } else {
                UpgradePromptAnswered = 0;
                if (!is_human(faction_id)
                    || GameDataStateStorage.fields_.game_more_preferences_ & 0x100000) {
                    for (int i = 0; i < 2 * MaxVehProtoFactionNum; i++) {
                        int upgrade_id = i < MaxVehProtoFactionNum ? i
                            : i % MaxVehProtoFactionNum
                                + faction_id * MaxVehProtoFactionNum;
                        if (VehPrototypes[upgrade_id].flags & 1
                            && VehPrototypes[upgrade_id].flags & 4
                            && VehPrototypes[upgrade_id].flags & 0x100) {
                            upgrade_prototypes(faction_id, upgrade_id);
                        }
                    }
                }
            }
        }
        if (proto_id < MaxVehProtoFactionNum
            && (Weapon[VehPrototypes[proto_id].weapon_id].offense_rating < 0
                || proto_id == 15)) {
            mon_native_life_bred(faction_id);
        } else {
            if (Chassis[VehPrototypes[proto_id].chassis_id].triad == TRIAD_SEA
                && Weapon[VehPrototypes[proto_id].weapon_id].offense_rating) {
                mon_naval_unit_built(faction_id);
            }
            if (Chassis[VehPrototypes[proto_id].chassis_id].triad == TRIAD_AIR) {
                mon_air_unit_built(faction_id);
            }
        }
        }
        return;
    }

    // ---- Facility / secret project production ----
    int facility_id = -BaseCurrent->queue_production_id[0];
    int project_id;
    if (facility_id >= FacilitySPStart) {
        project_id = facility_id - FacilitySPStart;
    } else {
        project_id = -1;
    }
    BOOL is_orbital = facility_id >= FacilityRepStart
                   && facility_id < FAC_STOCKPILE_ENERGY;
    int cost = cost_factor(BaseCurrent->faction_id_current, RSC_MINERALS, -1)
        * Facility[facility_id].cost;
    if (BaseCurrent->minerals_accumulated >= cost
        && project_id == SP_ASCENT_TO_TRANSCENDENCE && !ascending(faction_id)) {
        popb("ASCENTNOTYET", 0x1000, -1, "asctran_sm.pcx", 0);
        return;
    }
    if (project_id >= 0 && base_project(project_id) != -1) {
        if (is_human(faction_id)
            && !(BaseCurrent->governor & GOV_MANAGES_PRODUCTION
                 && BaseCurrent->governor & GOV_ACTIVE)) {
            parse_say(1, Facility[facility_id].name, -1, -1);
            clear();
            says("ALREADYPROJECT");
            int project_base_id = base_project(project_id);
            if (project_base_id >= 0) {
                parse_says(2, Bases[project_base_id].name_string, -1, -1);
                PluralityDefault =
                    Players[Bases[project_base_id].faction_id_current].is_noun_plural;
                GenderDefault =
                    Players[Bases[project_base_id].faction_id_current].noun_gender;
                parse_says(3,
                    Players[Bases[project_base_id].faction_id_current].noun_faction,
                    -1, -1);
                if (Bases[project_base_id].faction_id_current == faction_id) {
                    says("1");
                } else {
                    says("2");
                }
            } else {
                says("0");
            }
            popb(StringTemp, 0x200, -1, "secproj_sm.pcx", 0);
        } else {
            base_reset(BaseIDCurrentSelected, 0);
        }
        return;
    }
    if (project_id < 0 && has_fac(facility_id, BaseIDCurrentSelected, 0)) {
        if (base_queue(BaseIDCurrentSelected)) {
            goto production_again;
        }
        if (is_human(faction_id)
            && !(BaseCurrent->governor & GOV_MANAGES_PRODUCTION
                 && BaseCurrent->governor & GOV_ACTIVE)) {
            parse_say(1, Facility[facility_id].name, -1, -1);
            if (Players[faction_id].rule_flags & RFLAG_ALIEN) {
                popb("ALREADYFACILITY", 0x200, -1, "alfacblt_sm.pcx", 0);
            } else {
                popb("ALREADYFACILITY", 0x200, -1, "facblt_sm.pcx", 0);
            }
        } else {
            base_reset(BaseIDCurrentSelected, 0);
        }
        return;
    }
    if (BaseCurrent->minerals_accumulated < cost) {
        return;
    }
    // A computer-controlled project waits while a human base building the
    // same project can already pay for it.
    if (project_id >= 0 && !is_human(faction_id)) {
        for (int i = 0; i < GameDataStateStorage.fields_.base_current_count_; i++) {
            if (is_human(Bases[i].faction_id_current)
                && Bases[i].queue_production_id[0]
                    == BaseCurrent->queue_production_id[0]) {
                if (Bases[i].queue_production_id[0]
                        != -(FacilitySPStart + SP_ASCENT_TO_TRANSCENDENCE)
                    || ascending(Bases[i].faction_id_current)) {
                    if (Bases[i].mineral_surplus + Bases[i].minerals_accumulated
                        >= cost_factor(Bases[i].faction_id_current, RSC_MINERALS, -1)
                            * Facility[facility_id].cost) {
                        return;
                    }
                }
            }
        }
    }
    if (facility_id <= 0) {
        return;
    }

    // ---- Completion ----
    BaseCurrent->minerals_accumulated -= cost;
    if (BaseCurrent->minerals_accumulated
            > min(Rules.retool_exemption, BaseCurrent->mineral_surplus)
        && is_human(faction_id)) {
        BaseCurrent->minerals_accumulated =
            max(0, min(Rules.retool_exemption, BaseCurrent->mineral_surplus));
    }
    BaseCurrent->minerals_accumulated_2 = BaseCurrent->minerals_accumulated;
    BaseCurrent->state |= BSTATE_PRODUCTION_DONE;
    BaseCurrent->state &= ~0x80000000u;
    if (facility_id == FAC_HEADQUARTERS) {
        for (int i = 0; i < GameDataStateStorage.fields_.base_current_count_; i++) {
            if (Bases[i].faction_id_current == faction_id) {
                set_fac(FAC_HEADQUARTERS, i, FALSE);
            }
        }
    }
    if (project_id >= 0) {
        GameDataStateStorage.fields_.secret_project_[project_id] = BaseIDCurrentSelected;
    } else if (is_orbital) {
        PlayersData[faction_id].satellites_[facility_id - FacilityRepStart]++;
        if (!(PlayersData[faction_id].flags & PFLAG_MAP_REVEALED)) {
            int bit = 1 << faction_id;
            Map *tile = MapTiles;
            for (int i = 0; i < MapArea; tile++, i++) {
                tile->visibility |= bit;
                if (faction_id) {
                    tile->bit_visible[faction_id - 1] = tile->bit;
                }
            }
            for (i = 0; i < GameDataStateStorage.fields_.base_current_count_; i++) {
                Bases[i].visibility |= bit;
                Bases[i].faction_pop_size_intel[faction_id] = Bases[i].population_size;
            }
            PlayersData[faction_id].flags |= PFLAG_MAP_REVEALED;
            if (faction_id == ConsoleGlobal.faction_id_
                && !(GameDataStateStorage.fields_.game_state_ & 1)) {
                popp(ScriptPopupFile, "SEEMAP", 0, "space_sm.pcx", 0);
                set_dirty();
                draw_map(1);
                WorldClimateWorldWin.GraphicWin::redraw();
            }
        }
    } else {
        set_fac(facility_id, BaseIDCurrentSelected, TRUE);
    }
    if (facility_id == FAC_NETWORK_NODE) {
        BaseCurrent->state &= ~(BSTATE_ARTIFACT_LINKED | BSTATE_ARTIFACT_ALREADY_LINKED);
        // The step re-tests veh_id < 0 before reading the link (0x4F221A):
        // veh_next_stack_id expanded in place.
        for (int veh_id = stack_fix(veh_at(BaseCurrent->x, BaseCurrent->y));
             veh_id >= 0; veh_id = veh_next_stack_id(veh_id)) {
            if (VehPrototypes[Vehs[veh_id].proto_id].plan == PLAN_ALIEN_ARTIFACT) {
                Vehs[veh_id].order = ORDER_NONE;
            }
        }
    }
    draw_tile(BaseCurrent->x, BaseCurrent->y, 2);
    int queued = base_queue(BaseIDCurrentSelected);

    // ---- Completion report ----
    if (faction_id == ConsoleGlobal.faction_id_ || facility_id >= FAC_SUBSPACE_GENERATOR) {
        parse_say(1, Facility[facility_id].name, -1, -1);
        parse_says(2, Players[faction_id].adj_name_faction, -1, -1);
        if (faction_id == ConsoleGlobal.faction_id_) {
            if (facility_id == FAC_SUBSPACE_GENERATOR) {
                interlude(38, 0, 1, 0);
                parse_num(0, generators(faction_id, &undersized));
                parse_num(1, undersized);
                parse_num(2, Rules.subspace_generators_needed);
                parse_num(3, Rules.size_base_subspace_gen);
                parse_says(0, BaseCurrent->name_string, -1, -1);
                popp(ScriptPopupFile, "GENERATORME", 0, "subgen_sm.pcx", 0);
            } else if (facility_id < FacilitySPStart
                       && !has_fac_announced(faction_id, facility_id)
                       && !(GameDataStateStorage.fields_.game_state_ & 1)
                       && GameDataStateStorage.fields_.game_warnings_ & 1) {
                parse_says(0, BaseCurrent->name_string, -1, -1);
                popup.init(0, 0);
                popup.set_width(500);
                if (GameDataStateStorage.fields_.game_warnings_ & 1) {
                    clear();
                    if (project_id >= 0) {
                        says("PROJECT");
                        say_num(project_id);
                    } else {
                        says("FAC");
                        say_num(facility_id);
                        set_fac_announced(faction_id, facility_id, TRUE);
                    }
                    popup.start("BLURBS", StringTemp, -1, 0, 0x80, 0);
                    popup.string("^");
                    popup.string("^------");
                    popup.string("^");
                }
                clear();
                says("voices\\fac");
                if (facility_id >= FacilityRepStart && facility_id <= FAC_STOCKPILE_ENERGY) {
                    say_num(facility_id - 31);
                } else {
                    say_num(facility_id);
                }
                if (facility_id >= FAC_COVERT_OPS_CENTER && facility_id <= 41) {
                    say_num(0);
                }
                says(".mp3");
                if (GameDataStateStorage.fields_.game_more_preferences_ & 0x4000
                    && faction_id == ConsoleGlobal.faction_id_
                    && is_human(faction_id)) {
                    g_WAVE_GENERAL.unload();
                    g_WAVE_GENERAL.load(StringTemp);
                    g_WAVE_GENERAL.play();
                }
                clear();
                says("PRODUCEX");
                if (queued) {
                    parse_num(1, BaseCurrent->queue_size + 1);
                    says("Q");
                } else if (BaseCurrent->governor & GOV_MANAGES_PRODUCTION
                           && BaseCurrent->governor & GOV_ACTIVE) {
                    says("G");
                }
                // Two calls, tail-merged: each arm pushes its own picture
                // after the hoisted trailing 0 (0x4F25FB).
                if (Players[faction_id].rule_flags & RFLAG_ALIEN) {
                    popb(StringTemp, 1, -1, "alfacblt_sm.pcx", 0);
                } else {
                    popb(StringTemp, 1, -1, "facblt_sm.pcx", 0);
                }
                if (GameDataStateStorage.fields_.game_more_preferences_ & 0x4000
                    && GameDataStateStorage.fields_.game_more_preferences_ & 0x20000) {
                    g_WAVE_GENERAL.Sound::fade(2000);
                }
            } else {
                clear();
                says("PRODUCE");
                if (queued) {
                    parse_num(1, BaseCurrent->queue_size + 1);
                    says("Q");
                } else if (BaseCurrent->governor & GOV_MANAGES_PRODUCTION
                           && BaseCurrent->governor & GOV_ACTIVE) {
                    says("G");
                }
                // Two calls, tail-merged after the flag test: each arm
                // selects its picture and repeats the queued/governor test
                // (0x4F26E6, 0x4F2712).
                if (Players[faction_id].rule_flags & RFLAG_ALIEN) {
                    popb(StringTemp,
                         ((queued || (BaseCurrent->governor & GOV_MANAGES_PRODUCTION
                                      && BaseCurrent->governor & GOV_ACTIVE))
                          && !(GameDataStateStorage.fields_.game_warnings_ & 0x2000)) ? 0 : 1,
                         -1,
                         is_orbital ? "space_sm.pcx"
                             : facility_id >= FacilitySPStart ? "secproj_sm.pcx"
                             : "alfacblt_sm.pcx",
                         0);
                } else {
                    popb(StringTemp,
                         ((queued || (BaseCurrent->governor & GOV_MANAGES_PRODUCTION
                                      && BaseCurrent->governor & GOV_ACTIVE))
                          && !(GameDataStateStorage.fields_.game_warnings_ & 0x2000)) ? 0 : 1,
                         -1,
                         is_orbital ? "space_sm.pcx"
                             : facility_id >= FacilitySPStart ? "secproj_sm.pcx"
                             : "facblt_sm.pcx",
                         0);
                }
            }
        } else if (project_id >= 0) {
            parse_says(0, BaseCurrent->name_string, -1, -1);
            popup.init(0, 0);
            popup.set_width(600);
            clear();
            says("PROJECT");
            say_num(project_id);
            popup.start("BLURBS", StringTemp, -1, 0, 0x80, 0);
            if (WinScreenWidth != 800) {
                popup.string("^");
            }
            popup.string("^------");
            popup.string("^");
            popup.start(PopupStartCaption, "PRODUCE2", -1, 0, 0x80, 0);
            if (!(GameDataStateStorage.fields_.game_state_ & 1)
                || GameDataStateStorage.fields_.game_state_ & 8) {
                if (!Buffer::get_pcx_dimensions("secproj_sm.pcx", &width, &height)
                    && !popup.owned_picture().init("secproj_sm.pcx",
                                                   width, height)) {
                    popup.select_owned_picture();
                }
                popup.exec(0, 0);
            }
        } else if (facility_id == FAC_SUBSPACE_GENERATOR) {
            if (!(GameDataStateStorage.fields_.game_state_ & 1)
                || GameDataStateStorage.fields_.game_state_ & 8) {
                if (!(Players[ConsoleGlobal.faction_id_].rule_flags & RFLAG_ALIEN)) {
                    interlude(26, BaseCurrent->name_string, 1, 0);
                }
                parse_num(0, generators(faction_id, &undersized));
                parse_num(1, undersized);
                parse_num(2, Rules.subspace_generators_needed);
                parse_num(3, Rules.size_base_subspace_gen);
                parse_says(0, BaseCurrent->name_string, -1, -1);
                parse_says(1, get_title(faction_id), -1, -1);
                parse_says(2, get_name(faction_id), -1, -1);
                parse_says(3, get_noun(faction_id), -1, -1);
                popp(ScriptPopupFile, "GENERATOR", 0, "subgen_sm.pcx", 0);
            }
        } else {
            parse_says(0, BaseCurrent->name_string, -1, -1);
            if (!(GameDataStateStorage.fields_.game_state_ & 1)
                || GameDataStateStorage.fields_.game_state_ & 8) {
                popp(ScriptPopupFile, is_orbital ? "PRODUCE3" : "PRODUCE2", 0,
                     is_orbital ? "space_sm.pcx" : "secproj_sm.pcx", 0);
            }
        }
    }
    if (is_orbital) {
        mon_first_in_space(faction_id);
    } else if (facility_id < FacilitySPStart) {
        mon_facility_built(faction_id, BaseCurrent->name_string);
    }
    if (facility_id == FAC_CENTAURI_PRESERVE) {
        mon_built_preserve(faction_id);
        if (faction_id == ConsoleGlobal.faction_id_) {
            if (!(GameDataStateStorage.fields_.game_interlude_flags_ & 0x20)) {
                interlude(5, 0, 1, 0);
            } else {
                interlude(9, 0, 1, 0);
            }
        }
    }
    if (facility_id == FAC_CENTAURI_PRESERVE || facility_id == FAC_TEMPLE_OF_PLANET
        || facility_id == FAC_TREE_FARM || facility_id == FAC_HYBRID_FOREST) {
        if (PlayersData[faction_id].planet_ecology) {
            PlayersData[faction_id].planet_ecology++;
        }
        PlayersData[faction_id].flags &= ~0x4000;
    }

    // ---- Secret project events ----
    if (project_id >= 0) {
        if (project_id == SP_VOICE_OF_PLANET) {
            interlude(12, 0, 1, 0);
        }
        if (GameDataStateStorage.fields_.game_preferences_ & PREF_AV_SECRET_PROJECT_MOVIES) {
            if (!IsMultiplayerNet && faction_id == ConsoleGlobal.faction_id_) {
                amovie_project(project_id);
            }
            if (GameDataStateStorage.fields_.game_preferences_ & PREF_AV_SECRET_PROJECT_MOVIES
                && !IsMultiplayerNet && faction_id != ConsoleGlobal.faction_id_
                && project_id == SP_ASCENT_TO_TRANSCENDENCE) {
                amovie_project("theyTranscend");
            }
        }
        if (faction_id == ConsoleGlobal.faction_id_) {
            help_project(project_id);
        }
        if (project_id == SP_VOICE_OF_PLANET) {
            // Planet's voice spreads fungus around one inland site per
            // living faction, then at random across the map.
            int handled[MaxPlayerNum];
            // BUG(original): never initialised. The image keeps it in the
            // [ebp-0x10] slot, so it carries whatever an earlier bitmask
            // offset or compiler temporary left there, and it is not reset
            // between bases either.
            int tries;
            for (int i = 0; i < MaxPlayerNum; i++) {
                handled[i] = !is_alive(i);
            }
            for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_;
                 base_id++) {
                int owner_id = Bases[base_id].faction_id_current;
                if (handled[owner_id]) {
                    continue;
                }
                if (is_ocean(Bases[base_id].x, Bases[base_id].y)) {
                    continue;
                }
                handled[owner_id] = 1;
                int x;
                int y;
                do {
                    int offset = rnd(288, "BIGCROD");
                    x = wrap_map_x(Bases[base_id].x + RadiusOffsetX[offset + 1]);
                    y = Bases[base_id].y + RadiusOffsetY[offset + 1];
                    if (++tries >= 1000) {
                        break;
                    }
                } while (!on_map(x, y) || base_who(x, y) >= 0
                         || whose_territory(owner_id, x, y, 0, 0) != owner_id);
                if (tries >= 1000) {
                    continue;
                }
                if (owner_id == ConsoleGlobal.faction_id_) {
                    ConsoleGlobal.focus(x, y, ConsoleGlobal.faction_id_);
                }
                for (int i = 0; i < 9; i++) {
                    int x_radius = wrap_map_x(x + RadiusOffsetX[i]);
                    int y_radius = y + RadiusOffsetY[i];
                    if (on_map(x_radius, y_radius) && base_who(x_radius, y_radius) < 0) {
                        bit_set(x_radius, y_radius, BIT_FUNGUS, TRUE);
                        bit_set(x_radius, y_radius, MapTileFlagClearMask, FALSE);
                        synch_bit(x_radius, y_radius, ConsoleGlobal.faction_id_);
                        draw_tile(x_radius, y_radius, 2);
                    }
                }
            }
            int count = MapArea / 10;
            for (i = 0; i < count; i++) {
                int x = bounded_crt_random(MapLongitudeBounds);
                int y = bounded_crt_random(MapLatitudeBounds);
                if (x & 1) {
                    x--;
                }
                if (y & 1) {
                    x++;
                }
                if (on_map(x, y) && base_who(x, y) < 0) {
                    bit_set(x, y, BIT_FUNGUS, TRUE);
                    bit_set(x, y, MapTileFlagClearMask, FALSE);
                    synch_bit(x, y, ConsoleGlobal.faction_id_);
                    draw_tile(x, y, 2);
                }
            }
            interlude(13, 0, 1, 0);
            parse_says(5, get_name(faction_id), -1, -1);
            CharUpperA(ParseStrBuffer[5].str);
            interlude(14, 0, 0, 0);
        }
        mon_secret_project(faction_id, project_id);
        if (project_id == SP_ASCENT_TO_TRANSCENDENCE) {
            mon_winning_trans(faction_id, -1);
            if (GameDataStateStorage.fields_.game_rules_ & RULES_VICTORY_COOPERATIVE) {
                for (int i = 1; i < MaxPlayerNum; i++) {
                    if (i != faction_id
                        && PlayersData[faction_id].diplo_treaties[i] & DTREATY_PACT
                        && (!(PlayersData[i].diplo_treaties[faction_id]
                              & DTREATY_HAVE_SURRENDERED)
                            || is_human(i))) {
                        mon_winning_trans(i, faction_id);
                    }
                }
            }
            if (faction_id == ConsoleGlobal.faction_id_) {
                interlude(17, 0, 2, 0);
                EndgameExitReason = 1;
            } else {
                EndgameExitReason = 3;
                parse_says(5, get_name(faction_id), -1, -1);
                parse_num(2, game_year(GameDataStateStorage.fields_.turn_current_num_ + 100));
                parse_num(3, (GameDataStateStorage.fields_.turn_current_num_ + 100) / 100);
                if (Players[faction_id].rule_flags & RFLAG_ALIEN) {
                    interlude(27, 0, 4, 0);
                } else {
                    interlude(18, 0, 3, 0);
                }
            }
            GameDataStateStorage.fields_.game_state_ |= 1;
            return;
        }
        if (project_id == SP_UNIVERSAL_TRANSLATOR) {
            tech_advance(faction_id);
            tech_advance(faction_id);
        } else if (project_id == SP_PLANETARY_TRANS_SYS) {
            for (int i = 0; i < GameDataStateStorage.fields_.base_current_count_; i++) {
                if (Bases[i].faction_id_current == faction_id
                    && Bases[i].population_size < 3) {
                    Bases[i].population_size = 3;
                    if (faction_id == ConsoleGlobal.faction_id_) {
                        draw_tile(Bases[i].x, Bases[i].y, 2);
                    }
                }
            }
        }
    }

    // ---- Queue follow-up ----
    if ((BaseCurrent->governor & GOV_MANAGES_PRODUCTION
         && BaseCurrent->governor & GOV_ACTIVE && !queued)
        || !is_human(faction_id)) {
        base_reset(BaseIDCurrentSelected, 0);
    }
    if (is_human(faction_id)
        && (BaseCurrent->governor & (GOV_MANAGES_PRODUCTION | GOV_ACTIVE))
            != (GOV_MANAGES_PRODUCTION | GOV_ACTIVE)
        && BaseCurrent->queue_production_id[0] == -facility_id
        && !BaseCurrent->queue_size) {
        BaseCurrent->queue_production_id[0] = -FAC_STOCKPILE_ENERGY;
        draw_radius(BaseCurrent->x, BaseCurrent->y, 2, 2);
    }
    if (project_id < 0) {
        return;
    }
    // Other bases still building the finished project switch with the
    // cheapest same-faction base building some other project, or reset.
    int base_id_current = BaseIDCurrentSelected;
    for (int i = 0; i < GameDataStateStorage.fields_.base_current_count_; i++) {
        if (i == base_id_current) {
            continue;
        }
        if (is_human(Bases[i].faction_id_current)
            && !(Bases[i].governor & GOV_MANAGES_PRODUCTION
                 && Bases[i].governor & GOV_ACTIVE)) {
            continue;
        }
        if (Bases[i].queue_production_id[0] != -facility_id) {
            continue;
        }
        int best_base_id = -1;
        int best_score = Bases[i].minerals_accumulated + Bases[i].mineral_surplus * 10;
        for (int j = 0; j < GameDataStateStorage.fields_.base_current_count_; j++) {
            if (j != i && j != base_id_current
                && Bases[j].faction_id_current == Bases[i].faction_id_current
                && Bases[j].queue_production_id[0] != -facility_id
                && Bases[j].queue_production_id[0] <= -FacilitySPStart) {
                int score = Bases[j].minerals_accumulated + Bases[j].mineral_surplus * 10;
                if (score < best_score) {
                    best_score = score;
                    best_base_id = j;
                }
            }
        }
        if (best_base_id >= 0) {
            Bases[i].queue_production_id[0] = Bases[best_base_id].queue_production_id[0];
            Bases[best_base_id].queue_production_id[0] = -facility_id;
        } else {
            base_reset(i, 0);
        }
    }
    BaseIDCurrentSelected = base_id_current;
    BaseCurrent = &Bases[base_id_current];
    base_compute(0);
    return;
}

// ORIGINAL: 0x004F3FE0 ?base_hurry@@YAXXZ 0x004F3FE0-0x004F4826
// RECOVERY_BINDING: v1 {"evidence": ["git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/004f3fe0.cpp; recovery/tasks/base-colony-economy-coverage-wave.json; recovery/tests/test_base_colony_economy.py", "Pinned01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf full4F3FE0..4F4826 instruction review and measured VC6 PlayerData/SocialEffect/Base/prototype owners.", "Native4F4365 reads pending.police(+260), not morale;4F4390 biases the wrapped reserve product before SAR2; facilities4/5/33/projects use /50, not /100.", "NativeVoice45F8..464E halves hurry budget;46C4 reads current queue;46D7 preserves explicit zero-factor branch;4702/474C reread lock mask and479D/47CF reread popup faction."], "reason": "Recover complete colony hurry budgets, mineral purchases and project notification using real Base/faction/prototype/facility owners and explicit native 32-bit wrapping. Preserve seed/history and original behavior; no new byte, linked-preservation or runtime acceptance claim."}
// symbol    ?base_hurry@@YAXXZ
// TRIED: full register-transliteration from raw disasm (not Ghidra, whose DAT_0090ea30 offsets are all mis-scaled x2 - it typed that global short*, and its 4th arg to at_goal() is wrong too, using DAT_0090ea30[1] where raw disasm shows edi/cost). Division-by-constant idioms (magic-multiply+shift) written as plain C `/` and trusted to re-lower; verified against Ghidra's own reconstructed divisors (10, 25,50,100,250) rather than reverse-engineering each magic constant by hand. Compiles; frameless prologue vs original's framed - not chased given size.
void __cdecl base_hurry() {
    int faction = BaseCurrent->faction_id_current;
    if (is_human(faction)
        && (!(BaseCurrent->governor & GOV_MAY_HURRY_PRODUCTION)
            | !(BaseCurrent->governor & GOV_ACTIVE))) {
        return;
    }
    if (!BaseCurrent->minerals_accumulated) {
        return;
    }
    int production_id = BaseCurrent->queue_production_id[0];
    int facility_index = production_id < 0 ? -production_id : 0;
    int hurry = 0;
    int cost = facility_index
        ? Facility[facility_index].cost
        : veh_cost(production_id, BaseIDCurrentSelected, nullptr);
    int scaled_cost = cost_factor(faction, 1, -1) * cost;
    int y = BaseCurrent->y;
    int x = BaseCurrent->x;
    int region = map_loc(x, y)->region;

    if (production_id >= 0) {
        if (VehPrototypes[production_id].plan == PLAN_COLONIZATION
            && BaseCurrent->population_size < 2) {
            return;
        }
        if (veh_who(x, y) >= 0
            && !(BaseCurrent->state & BSTATE_COMBAT_LOSS_LAST_TURN)
            && !at_goal(faction, 0x10, x, y)) {
            if (production_id >= 0x40
                && !(VehPrototypes[production_id].flags & PROTO_TYPED_COMPLETE)) {
                hurry = PlayersData[faction].energy_reserves
                    / ((PlayersData[faction].ai_wealth + 2) * 10);
            } else {
                uint8_t regional_plan = PlayersData[faction].region_base_plan[region];
                if (regional_plan == VehPrototypes[production_id].plan
                    || (VehPrototypes[production_id].plan == PLAN_TERRAFORMING
                        && regional_plan == PLAN_COLONIZATION)
                    || (VehPrototypes[production_id].plan == PLAN_COMBAT
                        && regional_plan <= 2)) {
                    hurry = PlayersData[faction].energy_reserves
                        / ((PlayersData[faction].ai_wealth + 2) * 25
                           - PlayersData[faction].ai_power * 10);
                    if (regional_plan == PLAN_COLONIZATION) {
                        hurry = (PlayersData[faction].ai_growth + 5) * hurry / 4;
                        if (PlayersData[faction].energy_reserves
                                > GameDataStateStorage.fields_.turn_current_num_) {
                            hurry *= 2;
                        }
                    }
                }
            }
        } else {
            hurry = PlayersData[faction].energy_reserves / 10;
        }
        if (planet_buster2(production_id) && !PlayersData[faction].planet_busters) {
            for (int i = 1; i < MaxPlayerNum; i++) {
                if (i != faction
                    && (PlayersData[faction].diplo_treaties[i] & DTREATY_VENDETTA)
                    && PlayersData[i].major_atrocities
                    && PlayersData[faction].major_atrocities
                        <= PlayersData[i].major_atrocities) {
                    hurry = PlayersData[faction].energy_reserves / 10;
                }
            }
        }
    } else if ((BaseCurrent->state & BSTATE_DRONE_RIOTS_ACTIVE) && facility_index) {
        hurry = PlayersData[faction].energy_reserves
            / ((PlayersData[faction].ai_power + PlayersData[faction].ai_wealth + 1) * 5);
        if (BaseCurrent->drone_total > BaseCurrent->talent_total
            && PlayersData[faction].soc_effect_pending.values_[SocialEffect::POLICE]
                   + 2 * has_fac(FAC_BROOD_PIT, BaseIDCurrentSelected, 0) <= -2
            && facility_index < FacilitySPStart) {
            // BUG(original): the remaining cost is multiplied by 2 for a human
            // faction and by 0 for an AI one, so AI bases never hurry here.
            hurry = range((scaled_cost - BaseCurrent->minerals_accumulated)
                              * (is_human(faction) ? 2 : 0),
                          0, PlayersData[faction].energy_reserves * 3 / 4);
        }
    } else if (facility_index == FAC_HEADQUARTERS) {
        hurry = PlayersData[faction].energy_reserves / 10;
    } else if (facility_index == FAC_PERIMETER_DEFENSE
               || facility_index == FAC_TACHYON_FIELD) {
        hurry = PlayersData[faction].energy_reserves / 50;
        for (int i = 1; i < MaxPlayerNum; i++) {
            if (i != faction
                && (PlayersData[faction].diplo_treaties[i]
                    & (DTREATY_VENDETTA | DTREATY_WANT_REVENGE | DTREATY_ATROCITY_VICTIM))
                // BUG(original): indexes faction i's region_total_bases by the
                // faction number i, not by a region.
                && (PlayersData[i].region_total_bases[i] || is_human(i))) {
                hurry = PlayersData[faction].energy_reserves / 10;
            }
        }
    } else if (facility_index >= FacilitySPStart) {
        hurry = PlayersData[faction].energy_reserves / 50;
        if (PlayersData[faction].energy_reserves >= 2000) {
            hurry *= 4;
        }
        if (facility_index == FAC_ASCENT_TO_TRANSCENDENCE) {
            hurry *= 4;
        }
    } else if (facility_index == FAC_PSI_GATE) {
        hurry = PlayersData[faction].energy_reserves / 50;
        if (at_climax(faction)) {
            hurry *= 2;
        }
    }

    if (PlayersData[faction].energy_reserves >= 4800) {
        hurry += PlayersData[faction].energy_reserves / 25;
    } else if (PlayersData[faction].energy_reserves > 3200) {
        hurry += PlayersData[faction].energy_reserves / 100;
    } else if (PlayersData[faction].energy_reserves > 2000) {
        hurry += PlayersData[faction].energy_reserves / 250;
    }
    hurry = range(hurry, 0, PlayersData[faction].energy_reserves);
    hurry = range(hurry, 0, scaled_cost - BaseCurrent->minerals_accumulated);
    if (hurry) {
        if (GameDataStateStorage.fields_.secret_project_.voice_of_planet() >= 0) {
            hurry /= 2;
        } else if (!strcmp(Players[faction].filename, "MORGAN")) {
            hurry = hurry * 2 / 3;
        } else if (!strcmp(Players[faction].filename, "PEACE")) {
            hurry = hurry * 3 / 2;
        }
        int rule_hurry = Players[faction].rule_hurry;
        if (rule_hurry != 100 && hurry < PlayersData[faction].energy_reserves / 10) {
            hurry = hurry * 100 / rule_hurry;
        }
        int minerals = BaseCurrent->minerals_accumulated;
        int remaining = scaled_cost - minerals;
        int price = remaining * remaining / 20 + remaining * 2;
        if (BaseCurrent->queue_production_id[0] <= -FacilitySPStart) {
            price *= 2;
        }
        if (GameDataStateStorage.fields_.secret_project_.voice_of_planet() >= 0) {
            price *= 2;
        }
        int bought = price ? remaining * hurry / price : 0;
        BaseCurrent->minerals_accumulated = bought * 100 / rule_hurry + minerals;
        int refund = is_human(faction)
            ? 0
            : GameDataStateStorage.fields_.diff_level_current_ * hurry
                / ((8 - PlayersData[GameDataStateStorage.fields_.ranking_faction_id_unk1_]
                        .ranking) * 10);
        PlayersData[faction].energy_reserves += refund - hurry;
    }

    if (is_human(faction)
        || facility_index < FacilitySPStart
        || base_project(facility_index - FacilitySPStart) != SP_Unbuilt
        || GameDataStateStorage.fields_.sunspot_duration_ > 0
        || (GameDataStateStorage.fields_.game_state_ & STATE_UNK_1)) {
        return;
    }
    if (BaseCurrent->mineral_surplus + BaseCurrent->minerals_accumulated
            < cost_factor(BaseCurrent->faction_id_current, 1, -1) * Facility[facility_index].cost) {
        return;
    }
    parse_says(0, Players[BaseCurrent->faction_id_current].adj_name_faction, -1, -1);
    parse_say(1, Facility[facility_index].name, -1, -1);
    popp(ScriptPopupFile, "DONEPROJECT", 0, "secproj_sm.pcx", nullptr);
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004F4830 ?base_check_support@@YAXXZ 0x004F4830-0x004F4DBB
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/004f4830.cpp", "recovery/tasks/base-colony-economy-coverage-wave.json", "recovery/tests/test_base_colony_economy.py", "Pinned original 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf; complete original extent reviewed against native instructions and measured VC6 owner layouts.", "Native facility35 uses selected-base record; riot score reads prototype.cost(+29), not carry_capacity(+28); first distance scan reapplies abs after wrap, second does not."], "reason": "Recover the complete support deficit and pacifism selection through typed Base/Vehicle/faction owners, preserving original AI double-decrement and selected-base facility semantics. Seed retained; no new byte or runtime claim."}
// symbol    ?base_check_support@@YAXXZ
void __cdecl base_check_support() {
    int faction = BaseCurrent->faction_id_current;
    int best_score;
    int veh_id;
    int best_veh;
    while (BaseCurrentForcesMaintCost > BaseCurrent->intake_2[RSC_MINERALS]) {
        best_score = -1;
        best_veh = -1;
        for (veh_id = 0; veh_id < GameDataStateStorage.fields_.vehicle_current_count_; veh_id++) {
            if (Vehs[veh_id].faction_id != BaseCurrent->faction_id_current
                || Vehs[veh_id].home_base_id != BaseIDCurrentSelected) {
                continue;
            }
            int score;
            uint8_t plan = VehPrototypes[Vehs[veh_id].proto_id].plan;
            if (plan == PLAN_SUPPLY_CONVOY) {
                if (Vehs[veh_id].order != ORDER_CONVOY
                    || Vehs[veh_id].order_auto_type != ORDERA_TERRA_AUTO_ROAD
                    || base_who(Vehs[veh_id].x, Vehs[veh_id].y) < 0) {
                    continue;
                }
                score = 99999;
            } else {
                if (!(Vehs[veh_id].state & VSTATE_REQUIRES_SUPPORT) || plan > 9) {
                    continue;
                }
                score = vector_dist(Vehs[veh_id].x, Vehs[veh_id].y,
                                    BaseCurrent->x, BaseCurrent->y);
            }
            if (score > best_score) {
                best_score = score;
                best_veh = veh_id;
            }
        }
        if (best_score < 0) {
            break;
        }
        uint8_t plan = VehPrototypes[Vehs[best_veh].proto_id].plan;
        if (plan == PLAN_SUPPLY_CONVOY) {
            Vehs[best_veh].order = ORDER_NONE;
            BaseCurrentForcesMaintCost--;
            continue;
        }
        if (GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & (1 << faction)) {
            parse_says(1, VehPrototypes[Vehs[best_veh].proto_id].veh_name, -1, -1);
            popb("NOSUPPORT", 0x8000, 0xD, "genwarning_sm.pcx", 0);
        } else if (plan == PLAN_COLONIZATION) {
            Vehs[best_veh].state &= ~VSTATE_REQUIRES_SUPPORT;
            int support = PlayersData[faction].soc_effect_pending.values_[SocialEffect::SUPPORT];
            BaseCurrentForcesMaintCost -= has_abil(Vehs[best_veh].proto_id, ABL_CLEAN_REACTOR)
                ? 0 : (support <= -4 ? 2 : 1);
        }
        int support = PlayersData[faction].soc_effect_pending.values_[SocialEffect::SUPPORT];
        if (has_abil(Vehs[best_veh].proto_id, ABL_CLEAN_REACTOR)) {
            break;
        }
        BaseCurrentForcesMaintCost -= support <= -4 ? 2 : 1;
        kill(best_veh);
    }
    int faction_bit = 1 << faction;
    if (GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & faction_bit) {
        if (!(BaseCurrent->governor & GOV_MANAGES_CITIZENS_SPECS)) {
            return;
        }
        if (!(BaseCurrent->governor & GOV_ACTIVE)) {
            return;
        }
    }
    if (!(BaseCurrent->state & BSTATE_DRONE_RIOTS_ACTIVE)) {
        return;
    }
    // The image reads POLICE (+0x260) here, as base_doctors does.
    if (PlayersData[faction].soc_effect_pending.values_[SocialEffect::POLICE]
            + 2 * has_fac(0x23, BaseIDCurrentSelected, 0) > -3) {
        return;
    }
    if (BaseCurrent->drone_total <= BaseCurrent->talent_total
        && BaseCurrent->nutrient_surplus >= 0) {
        return;
    }
    best_score = -1;
    best_veh = -1;
    for (veh_id = 0; veh_id < GameDataStateStorage.fields_.vehicle_current_count_; veh_id++) {
        if (Vehs[veh_id].faction_id != faction
            || Vehs[veh_id].home_base_id != BaseIDCurrentSelected) {
            continue;
        }
        if (!(Vehs[veh_id].state & VSTATE_PACIFISM_DRONE)) {
            continue;
        }
        if (faction_bit & GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_) {
            if (!(Vehs[veh_id].state & VSTATE_UNK_200)
                || (Vehs[veh_id].state & 0x1000200) == 0x1000200
                || ((Vehs[veh_id].state & 0x2000200) == 0x2000200
                    && Vehs[veh_id].patrol_current_point < Vehs[veh_id].waypoint_count)
                || Vehs[veh_id].order_auto_type == ORDERA_ON_ALERT
                || Vehs[veh_id].order_auto_type == ORDERA_BOMBING_RUN
                || Vehs[veh_id].order_auto_type) {
                continue;
            }
        }
        int divisor = PlayersData[faction].ai_fight + 2;
        if (rnd(divisor, "BASECHECKSUPPORT")) {
            continue;
        }
        int score = map_range(Vehs[veh_id].x, Vehs[veh_id].y, BaseCurrent->x, BaseCurrent->y) * 100
            / range(VehPrototypes[Vehs[veh_id].proto_id].cost, 1, 99);
        if (VehPrototypes[Vehs[veh_id].proto_id].plan == PLAN_DEFENSIVE) {
            score += score;
        }
        if (VehPrototypes[Vehs[veh_id].proto_id].plan == PLAN_COMBAT) {
            score /= 2;
        }
        if (score > best_score) {
            best_score = score;
            best_veh = veh_id;
        }
    }
    if (best_veh >= 0) {
        kill(best_veh);
        BaseCurrent->state &= ~BSTATE_DRONE_RIOTS_ACTIVE;
    }
}

/*
Purpose: Check if current base has had an energy shortfall. If so, reset all existing energy convoy
         orders for the faction. TODO: Revisit and find a way to only reset specific base convoys.
// ORIGINAL: 0x004F4DC0 ?base_energy_costs@@YAXXZ 0x004F4DC0-0x004F4E73 BYTE_EXACT
// TRIED: already 0.852 similar (best flag set, 3/60 raw). The remaining
//            gap is inside base_who()/map_loc()'s inlined `x >> 1`: the
//            image widens Vehs[i].x to int (movsx) BEFORE the shift, this
//            tree's inline shifts the 16-bit value first and widens after -
//            an explicit `(int)Vehs[i].x` cast at the call site does not
//            change it (measured), so the divergence is inside the shared
//            inline body, not this call site's source shape.
// size      179 bytes
// prototype
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
// The original captures faction before the deficit read and snapshots the scan count.
void __cdecl base_energy_costs() {
    const uint32_t faction_id = BaseCurrent->faction_id_current;
    if (BaseCurrent->energy_surplus >= 0) return;
    const int total = GameDataStateStorage.fields_.vehicle_current_count_;
    for (int i = 0; i < total; i++) {
        if (Vehs[i].faction_id == faction_id
            && VehPrototypes[Vehs[i].proto_id].plan == PLAN_SUPPLY_CONVOY
            && Vehs[i].order == ORDER_CONVOY && Vehs[i].order_auto_type == RSC_ENERGY) {
            Map *const tile = MapTiles + Vehs[i].y * MapLongitude + (Vehs[i].x >> 1);
            int owner = -1;
            if ((tile->bit & BIT_BASE_IN_TILE) != 0) {
                const int candidate = tile->val2 & 0xF;
                if (candidate < MaxPlayerNum) owner = candidate;
            }
            if (owner >= 0) Vehs[i].order = ORDER_NONE;
        }
    }
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004F4E80 ?base_research@@YAXXZ 0x004F4E80-0x004F4FAB BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/004f4e80.cpp", "recovery/tasks/base-colony-economy-coverage-wave.json", "recovery/tests/test_base_colony_economy.py", "Pinned original 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf; complete original extent reviewed against native instructions and measured VC6 owner layouts.", "Superseded interpretation, retained verbatim: Original4F4E80..4F4FAB reads Base.labs_total(+108), PlayerData.soc_effect_pending.research and net_random_event; faction-indexed Base bitset and signed remainder are intentional retained behavior.", "Superseded reason, retained verbatim: Recover the complete base laboratory contribution, scenario/early-turn gates and signed fractional research dispatch with native 32-bit wrap. Preserve original Bases[faction_id] facility lookup rather than silently correcting its behavior. Seed retained; no matching or game claim.", "Pinned 4F4E9F loads the faction into EBX; 4F4EA2 independently loads selected base index 689370 into ESI, used by 4F4EBE..4F4ED7 for Bases facilities. set_base 4E39D0..4E39F3 stores its base_id at 689370 and publishes Bases[base_id] as BaseCurrent. The prior faction-index interpretation is disproved.", "Pinned 4F4ED9..4F4EDF materializes the has_fac_built boolean before halving labs; 4F4F3D..4F4F55 uses the existing signed range operation with bounds -5 and 5."], "reason": "Recover the complete base laboratory contribution, scenario/early-turn gates and signed fractional research dispatch with native 32-bit wrap. The facility test uses BaseIDCurrentSelected, independently of the faction used for PlayerData; retain the existing facility boolean operation and signed range clamp. No game acceptance claim."}
// symbol    ?base_research@@YAXXZ
void __cdecl base_research() {
    if ((GameDataStateStorage.fields_.game_rules_ & RULES_SCN_NO_TECH_ADVANCES) != 0) {
        return;
    }

    const int faction_id = BaseCurrent->faction_id_current;
    int points = BaseCurrent->labs_total;
    if (has_fac(FAC_PUNISHMENT_SPHERE, BaseIDCurrentSelected, 0)) {
        points /= 2;
    }
    if (GameDataStateStorage.fields_.turn_current_num_
        < -5 * PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::RESEARCH]) {
        points = 0;
    }
    if (GameDataStateStorage.fields_.turn_current_num_ < 5 && PlayersData[faction_id].diff_level < 2) {
        points = 0;
    }
    PlayersData[faction_id].labs_total += points;

    int scaled = (range(PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::RESEARCH],
                        -5, 5) + 10) * points * 10;
    PlayersData[faction_id].net_random_event += scaled % 100;
    if (PlayersData[faction_id].net_random_event >= 100) {
        PlayersData[faction_id].net_random_event -= 100;
        scaled += 100;
    }
    tech_research(faction_id, scaled / 100);
}

// Facility indexing follows Base.facilities_built byte and bit indices.

// ORIGINAL: 0x004F4FB0 ?drone_riot@@YAXXZ 0x004F4FB0-0x004F5CD6
// RECOVERY_BINDING: v1 {"evidence":["original:0x004F4FB0-0x004F5CD6:pinned drone-riot span and direct callers","original:0x0090EA30:BaseCurrent receiver; pinned offsets correlate to src/base.h Base fields","original:0x004F4FB0-0x004F5CD6:the receiver member displacements correlate with the typed Base fields","source:src/base.h:Base and BaseCurrent own callback-sensitive base state","source:src/veh.h:Veh and VehPrototype own vehicle transfer records and prototype data","source:src/map.h:Map, bit_set and visibility synchronization own tile state"],"reason":"Recover drone riot through current Base, Faction, Vehicle, Facility, and Map owners while preserving native callback reloads, candidate/radius ordering, and literal call-site distinctions."}
// TRIED: the first ~180 bytes (unrest flag check, draw_tile, the four popb dialog variants) are transcribed and argument-order checked directly against the raw disassembly. Past that, Ghidra typed `DAT_0090ea30` (BaseCurrent) as `short*`, so every offset it prints is HALF the real byte offset - doubled here and cross-checked against src/base.h's `Base` (0x38 governor, 0x4C queue_size, 0x50 queue_production_id, 0x7C specialist_total match exactly). Eight callees (owner_set/kill/best-target-transfer helpers at 0x46B1F0, 0x46B190, 0x580860, 0x591B10, 0x591C10, 0x5ADE80, 0x5B57D0, 0x5C0B00) have no catalogued name or signature in this scaffold and are called with no arguments; the base_energy/minerals/nutrient/support/yield call order is a guess. The nearest-rival-faction base-transfer arithmetic (distance metric, unit disband loop) follows Ghidra's shape but is not verified instruction-for-instruction.
void __cdecl drone_riot() {
    int faction_id = BaseCurrent->faction_id_current;
    int riot_start = !(BaseCurrent->state & BSTATE_DRONE_RIOTS_ACTIVE);
    BaseCurrent->state |= BSTATE_DRONE_RIOTS_ACTIVE;
    if (riot_start) {
        draw_tile(BaseCurrent->x, BaseCurrent->y, 2);
    }
    if (riot_start && BaseCurrent->governor & GOV_MANAGES_CITIZENS_SPECS
        && BaseCurrent->governor & GOV_ACTIVE) {
        if (Players[faction_id].rule_flags & RFLAG_ALIEN) {
            popb("DRONERIOTS2", 0, 0x11, "ALdrone_sm.pcx", 0);
        } else {
            popb("DRONERIOTS2", 0, 0x11, "drone_sm.pcx", 0);
        }
    } else if (Players[faction_id].rule_flags & RFLAG_ALIEN) {
        // The flag repeats the governor test even though this branch is only
        // reached when it failed (always 8); the image evaluates it anyway.
        popb("DRONERIOTS", (riot_start && BaseCurrent->governor & GOV_MANAGES_CITIZENS_SPECS
            && BaseCurrent->governor & GOV_ACTIVE) ? 0 : 8, 0x11, "ALdrone_sm.pcx", 0);
    } else {
        popb("DRONERIOTS", (riot_start && BaseCurrent->governor & GOV_MANAGES_CITIZENS_SPECS
            && BaseCurrent->governor & GOV_ACTIVE) ? 0 : 8, 0x11, "drone_sm.pcx", 0);
    }
    if (!(BaseCurrent->state & BSTATE_DRONE_RIOTS_ACTIVE)) {
        return;
    }
    if (riot_start) {
        if (!is_human(faction_id)) {
            base_reset(BaseIDCurrentSelected, 0);
        }
        return;
    }
    // Continued riots: chance to destroy a facility.
    int odds = PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::POLICE]
        + has_fac(FAC_BROOD_PIT, BaseIDCurrentSelected, 0) * 2 + 7;
    if (rnd(odds, "RIOT")) {
        return;
    }
    if (has_fac(FAC_HEADQUARTERS, BaseIDCurrentSelected, 0)) {
        return;
    }
    if (PlayersData[faction_id].current_num_bases <= 1) {
        return;
    }
    if (BaseCurrent->assimilation_turns_left >= 45) {
        return;
    }
    if (BaseCurrent->population_size < 4 && rnd(3, "RIOT0")) {
        return;
    }
    int tries = BaseCurrent->population_size - PlayersData[faction_id].diff_level
        - PlayersData[faction_id].ranking + 20;
    int attempt = 0;
    int facility_id;
    // do/while, not for: the image runs the body once even when tries < 0.
    do {
        facility_id = rnd(70, "RIOT1");
        BOOL removable = true;
        for (int k = 0; k < Players[faction_id].faction_bonus_count; k++) {
            if ((Players[faction_id].faction_bonus_id[k] == 2
                || (Players[faction_id].faction_bonus_id[k] == 12
                    && has_tech(Facility[Players[faction_id].faction_bonus_val1[k]].preq_tech,
                        faction_id)))
                && facility_id == Players[faction_id].faction_bonus_val1[k]) {
                removable = false;
                break;
            }
        }
        if (facility_id > 1 && has_fac(facility_id, BaseIDCurrentSelected, 0) && removable) {
            break;
        }
    } while (++attempt <= tries);
    if (attempt <= tries) {
        set_fac(facility_id, BaseIDCurrentSelected, false);
        parse_say(1, Facility[facility_id].name, -1, -1);
        if (Players[faction_id].rule_flags & RFLAG_ALIEN) {
            popb("DRONERIOT", 8, -1, "ALdrone_sm.pcx", 0);
        } else {
            popb("DRONERIOT", 8, -1, "drone_sm.pcx", 0);
        }
        return;
    }
    // Severe police state: a human base may defect to another faction.
    if (PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::POLICE]
        + has_fac(FAC_BROOD_PIT, BaseIDCurrentSelected, 0) * 2 > -3) {
        return;
    }
    if (Players[faction_id].rule_flags & RFLAG_ALIEN) {
        return;
    }
    int bit = 1 << faction_id;
    if (!(GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & bit)) {
        return;
    }
    int target;
    if (BaseCurrent->faction_id_former != faction_id) {
        target = BaseCurrent->faction_id_former;
    } else {
        target = -1;
        int best_dist = 9999;
        for (int i = 0; i < GameDataStateStorage.fields_.base_current_count_; i++) {
            if (Bases[i].faction_id_current != faction_id) {
                int dist = vector_dist(Bases[i].x, Bases[i].y, BaseCurrent->x, BaseCurrent->y);
                // BUG(original): weights by the treaty with the best faction
                // found so far (target, -1 at first: reads last_turn_new_base
                // just before diplo_treaties[0]), not with this base's owner.
                uint32_t treaty = PlayersData[faction_id].diplo_treaties[target];
                if (treaty & DTREATY_PACT) {
                    dist *= 2;
                }
                if (treaty & DTREATY_TREATY) {
                    dist = dist * 3 / 2;
                }
                if (treaty & DTREATY_TRUCE) {
                    dist /= 2;
                }
                if (dist < best_dist) {
                    best_dist = dist;
                    target = Bases[i].faction_id_current;
                }
            }
        }
    }
    // Factions with the revolt bonus (13) attract the defection.
    double chance = 1.0;
    int total = 0;
    int shares[8];
    for (int p = 1; p < MaxPlayerNum; p++) {
        // BUG(original): the -1 marker is written one past the array
        // (shares[MaxPlayerNum], [ebp-0x3c]) rather than to shares[p], so
        // factions without the bonus keep uninitialised stack values that the
        // loops below read.
        shares[MaxPlayerNum] = -1;
        for (int k = 0; k < Players[p].faction_bonus_count; k++) {
            if (Players[p].faction_bonus_id[k] == 13) {
                shares[p] = range(Players[p].faction_bonus_val1[k], 0, 100);
                total += shares[p];
                k = Players[p].faction_bonus_count;
            }
        }
    }
    for (int j = 1; j < MaxPlayerNum; j++) {
        if (shares[j] + 1 != 0) {
            chance *= 1.0 - shares[j] * 0.01;
        }
    }
    int tick = timeGetTime();
    if ((int)(chance * 100.0) >= tick % 100) {
        int pick = tick / 100 % total;
        for (int s = 1; s < MaxPlayerNum; s++) {
            if (shares[s] + 1 != 0) {
                target = s;
                if (pick < shares[s]) {
                    s = MaxPlayerNum;
                } else {
                    pick -= shares[s];
                }
            }
        }
    }
    if (target <= 0) {
        return;
    }
    // Restored original defection gate: either side being human suffices
    // (the rioting faction was already required to be human above).
    int human_mask = GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 0xFF;
    if (!(bit & human_mask) && !(human_mask & (1 << target))) {
        return;
    }
    if (Players[target].rule_flags & RFLAG_ALIEN) {
        return;
    }
    // The base defects.
    int x = BaseCurrent->x;
    int y = BaseCurrent->y;
    BaseCurrent->faction_id_current = (uint8_t)target;
    for (int atk = 1; atk < MaxPlayerNum; atk++) {
        if (PlayersData[atk].base_id_atk_target == BaseIDCurrentSelected) {
            PlayersData[atk].base_id_atk_target = -1;
        }
    }
    BaseCurrent->assimilation_turns_left = (BaseCurrent->faction_id_former != target) ? 50 : 0;
    PlayersData[target].last_turn_new_base = GameDataStateStorage.fields_.turn_current_num_;
    BaseCurrent->minerals_accumulated_2 = 0;
    BaseCurrent->minerals_accumulated = 0;
    for (int q = 0; q <= BaseCurrent->queue_size; q++) {
        if (BaseCurrent->queue_production_id[q] >= 0) {
            PlayersData[faction_id].proto_id_queue[BaseCurrent->queue_production_id[q]]--;
        }
    }
    BaseCurrent->queue_size = 0;
    BaseCurrent->production_id_last = -FAC_STOCKPILE_ENERGY;
    BaseCurrent->queue_production_id[0] = -FAC_STOCKPILE_ENERGY;
    PlayersData[faction_id].current_num_bases--;
    PlayersData[target].current_num_bases++;
    replay_base(1, x, y, target);
    // Copy the Governor from the old owner's largest other base (HQ doubles).
    int best_size = 0;
    int best_base = -1;
    for (int b = 0; b < GameDataStateStorage.fields_.base_current_count_; b++) {
        if (b != BaseIDCurrentSelected && Bases[b].faction_id_current == faction_id) {
            int size = Bases[b].population_size;
            if (has_fac(FAC_HEADQUARTERS, b, 0)) {
                size *= 2;
            }
            if (size > best_size) {
                best_size = size;
                best_base = b;
            }
        }
    }
    if (best_base >= 0) {
        BaseCurrent->governor = Bases[best_base].governor;
    }
    BaseCurrent->state &= 0xFFDEF7B1;
    for (int r = 0; r < 21; r++) {
        int x2 = wrap_map_x(x + RadiusOffsetX[r]);
        int y2 = y + RadiusOffsetY[r];
        if (on_map(x2, y2)) {
            using_set(x2, y2, target);
            map_loc(x2, y2)->visibility |= 1 << target;
            for (int v = GameDataStateStorage.fields_.vehicle_current_count_ - 1; v >= 0; v--) {
                if (Vehs[v].x == x2 && Vehs[v].y == y2) {
                    if (Vehs[v].faction_id == target) {
                        Vehs[v].state &= ~VSTATE_UNK_40000;
                    } else if (Vehs[v].faction_id == faction_id && r < 9) {
                        // Units in the base square and inner ring defect too.
                        Vehs[v].faction_id = (uint8_t)target;
                        PlayersData[faction_id].proto_id_active[Vehs[v].proto_id]--;
                        if (Vehs[v].proto_id > 64) {
                            int proto_id = propose_proto(target,
                                VehPrototypes[Vehs[v].proto_id].chassis_id,
                                VehPrototypes[Vehs[v].proto_id].weapon_id,
                                VehPrototypes[Vehs[v].proto_id].armor_id,
                                VehPrototypes[Vehs[v].proto_id].ability_flags,
                                VehPrototypes[Vehs[v].proto_id].reactor_id, -2,
                                VehPrototypes[Vehs[v].proto_id].veh_name);
                            if (proto_id >= 0) {
                                Vehs[v].proto_id = (int16_t)proto_id;
                            }
                        }
                        Vehs[v].home_base_id = (int16_t)BaseIDCurrentSelected;
                        PlayersData[target].proto_id_active[Vehs[v].proto_id]++;
                        owner_set(x2, y2, target);
                        if (Chassis[VehPrototypes[Vehs[v].proto_id].chassis_id].range) {
                            Vehs[v].terraforming_turns = 0;
                        }
                        if (Vehs[v].order != ORDER_HOLD) {
                            Vehs[v].order = ORDER_NONE;
                        }
                        Vehs[v].state &= 0xFCFFBDFF;
                    }
                }
            }
        }
    }
    spot_base(BaseIDCurrentSelected, faction_id);
    reset_territory();
    for (int veh_id = GameDataStateStorage.fields_.vehicle_current_count_ - 1; veh_id >= 0;
        veh_id--) {
        if (veh_id < GameDataStateStorage.fields_.vehicle_current_count_
            && Vehs[veh_id].faction_id == faction_id
            && Vehs[veh_id].home_base_id == BaseIDCurrentSelected) {
            kill(veh_id);
        }
    }
    // Free facilities of both factions.
    for (int k1 = 0; k1 < Players[faction_id].faction_bonus_count; k1++) {
        if (Players[faction_id].faction_bonus_id[k1] == 2
            || (Players[faction_id].faction_bonus_id[k1] == 12
                && has_tech(Facility[Players[faction_id].faction_bonus_val1[k1]].preq_tech,
                    faction_id))) {
            set_fac(range(Players[faction_id].faction_bonus_val1[k1], 1, 69),
                BaseIDCurrentSelected, true);
        }
    }
    for (int k2 = 0; k2 < Players[target].faction_bonus_count; k2++) {
        // BUG(original): tests the tech against the old faction, not target.
        if (Players[target].faction_bonus_id[k2] == 2
            || (Players[target].faction_bonus_id[k2] == 12
                && has_tech(Facility[Players[target].faction_bonus_val1[k2]].preq_tech,
                    faction_id))) {
            set_fac(range(Players[target].faction_bonus_val1[k2], 1, 69),
                BaseIDCurrentSelected, true);
        }
    }
    owner_set(x, y, target);
    base_compute(1);
    int console_faction = ConsoleGlobal.faction_id_;
    if (faction_id == console_faction || target == console_faction
        || Bases[BaseIDCurrentSelected].faction_id_current == console_faction
        || Bases[BaseIDCurrentSelected].visibility & (1 << console_faction)) {
        MapWinTable[0]->set_center(x, y, 1);
        draw_map(1);
        parse_says(0, BaseCurrent->name_string, -1, -1);
        parse_says(2, Players[target].adj_name_faction, -1, -1);
        if (!(GameDataStateStorage.fields_.game_state_ & 1)
            || GameDataStateStorage.fields_.game_state_ & 8) {
            popp(ScriptPopupFile, "DRONEREVOLT", 0, "baseobl_sm.pcx", 0);
        }
    }
}

// Native 0x009A658C is SecretProject.telepathic_matrix (SP_TELEPATHIC_MATRIX).
// The native call enters the mapped drone_riot body below.

// ORIGINAL: 0x004F5CE0 ?base_drones@@YAXXZ 0x004F5CE0-0x004F5F25 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence":["original:0x004F5CE0-0x004F5F25:pinned base drone/riot lifecycle body","original:0x009A658C:SecretProject index 30 owns the telepathic-matrix faction id","source:src/alpha.h:SecretProject and SP_TELEPATHIC_MATRIX provide the typed project owner","source:src/base.h:BaseCurrent and Base-owned drone, talent, governor and state members","source:src/faction.h:Players owns faction rule_flags; LockEnableMask owns human-faction bits"],"reason":"Recover base drone state transitions using the existing Base, SecretProject, Player, and lock owners; retain native callback reloads and the separate drone-riot call."}
void __cdecl base_drones() {
    // Native popup literals remain ordinary C++ literals: "Alopdir.pcx" occurs at 0x689C78..0x689C84, 0x689CAC..0x689CB8, 0x689CE8..0x689CF4 and 0x689D04..0x689D10; the "GOLDENAGE", "GOLDENAGEOVER", "DRONERIOTSOVER" and "DRONERIOTSOVER2" keys and "talent_sm.pcx" have separate pinned call-site extents.
    if (!BaseCurrent->drone_total && BaseCurrent->population_size > 2
        && BaseCurrent->talent_total >= (BaseCurrent->population_size + 1) / 2) {
        if (!(BaseCurrent->state & BSTATE_GOLDEN_AGE_ACTIVE)) {
            if (Players[BaseCurrent->faction_id_current].rule_flags & RFLAG_ALIEN) {
                popb("GOLDENAGE", 0x20, -1, "Alopdir.pcx", 0);
            } else {
                popb("GOLDENAGE", 0x20, -1, "talent_sm.pcx", 0);
            }
        }
        BaseCurrent->state |= BSTATE_GOLDEN_AGE_ACTIVE;
    } else if (BaseCurrent->state & BSTATE_GOLDEN_AGE_ACTIVE) {
        BaseCurrent->state &= ~BSTATE_GOLDEN_AGE_ACTIVE;
        if (BaseCurrent->drone_total <= BaseCurrent->talent_total) {
            if (Players[BaseCurrent->faction_id_current].rule_flags & RFLAG_ALIEN) {
                popb("GOLDENAGEOVER", 0x40, -1, "Alopdir.pcx", 0);
            } else {
                popb("GOLDENAGEOVER", 0x40, -1, "talent_sm.pcx", 0);
            }
        }
    }
    if (BaseCurrent->drone_total > BaseCurrent->talent_total
        && !has_project(SP_TELEPATHIC_MATRIX, BaseCurrent->faction_id_current)
        && !BaseCurrent->nerve_staple_turns_left) {
        drone_riot();
        return;
    }
    if (BaseCurrent->state & BSTATE_DRONE_RIOTS_ACTIVE) {
        BaseCurrent->state &= ~BSTATE_DRONE_RIOTS_ACTIVE;
        Base *base = BaseCurrent;
        if (!(base->state & BSTATE_GOLDEN_AGE_ACTIVE)) {
            int faction_id = base->faction_id_current;
            if (Players[faction_id].rule_flags & RFLAG_ALIEN) {
                if (base->governor & GOV_MANAGES_CITIZENS_SPECS && base->governor & GOV_ACTIVE) {
                    popb("DRONERIOTSOVER2", 0, 0x33, "Alopdir.pcx", 0);
                } else {
                    popb("DRONERIOTSOVER",
                        (base->governor & GOV_MANAGES_CITIZENS_SPECS && base->governor & GOV_ACTIVE) ? 0 : 0x10, 0x33,
                        "Alopdir.pcx", 0);
                }
            } else {
                if (base->governor & GOV_MANAGES_CITIZENS_SPECS && base->governor & GOV_ACTIVE) {
                    popb("DRONERIOTSOVER2", 0, 0x33, "talent_sm.pcx", 0);
                } else {
                    popb("DRONERIOTSOVER",
                        (base->governor & GOV_MANAGES_CITIZENS_SPECS && base->governor & GOV_ACTIVE) ? 0 : 0x10, 0x33,
                        "talent_sm.pcx", 0);
                }
            }
            base = BaseCurrent;
        }
        if (!is_human(base->faction_id_current)) {
            base_reset(BaseIDCurrentSelected, 0);
            base = BaseCurrent;
        }
        draw_tile(base->x, base->y, 2);
    }
}

// ORIGINAL: 0x004F5F30 ?base_doctors@@YAXXZ 0x004F5F30-0x004F650C BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:b2de7f29042d247ee9b448c814029bbbbc35f230:src/unrecovered/004f5f30.cpp", "source:src/base.h"], "reason": "Recover the selected Base's complete specialist/worker adjustment, riot eligibility, Matrix-gated removal and console warning paths through typed Base, citizen, Rules and project state. Preserve selected-base versus entry-faction distinctions and live BaseCurrent reloads across callbacks. Original unsafe accesses, full identity, seed history and matching/restoration obligations remain; no byte, ABI or runtime acceptance is implied."}
// kind      game
// TRIED: near-verbatim transcription of the (clean, non-SEH) Ghidra output with DAT_/FUN_ substituted for the catalogued globals and callees, and `*(char**)g_0090ea30` for the base-pointer dereference; the two duplicated "pick best doctor" loops are written out twice (matching the original's own duplication) rather than factored into a helper, to avoid the one-external-symbol risk seen on 0x00405140. Reaches MISMATCH #6 ('xor' vs 'push'); did not chase the exact register/stack layout further given the function's size.
// Defined after best_specialist and pop_goal_fac: the image inlines both.
// WORKAROUND(decl): the image calls has_fac (0x421670) out of line in
// base_doctors. It is an inline owner, not a pointer: as a plain call inside
// this helper, has_fac becomes a pass-2 (nested) site that base_doctors' /Ob2
// budget refuses, as in the image (measured 2026-11-19: the former local
// function pointer is redundant; writing has_fac at the call site instead
// expands it, 0 -> 28 rows). The helper's original name is unknown.
// The two helpers also shrink base_doctors, so its /Ob2 budget leaves the
// has_fac calls in its pop_goal_fac copy out of line as in the image.
static inline BOOL selected_base_has_fac_call(int facility_id) {
    return has_fac(facility_id, BaseIDCurrentSelected, 0);
}

void __cdecl base_doctors() {
    int faction = BaseCurrent->faction_id_current;
    int riot_adjust;
    if ((BaseCurrent->state & BSTATE_DRONE_RIOTS_ACTIVE) != 0
        && PlayersData[faction].soc_effect_pending.values_[SocialEffect::POLICE]
               + selected_base_has_fac_call(FAC_BROOD_PIT) * 2 < -1
        && !pop_goal_fac(BaseIDCurrentSelected)
        && (((has_project(SP_CLONING_VATS, faction) || BaseCurrentGrowthRate >= 6)
             && Rules.nutrient_req_citizen != 0
             && BaseCurrent->nutrient_surplus >= static_cast<int>(Rules.nutrient_req_citizen))
            || BaseCurrent->nutrient_surplus + BaseCurrent->nutrients_accumulated
                   >= cost_factor(faction, 0, BaseIDCurrentSelected)
                          * (BaseCurrent->population_size + 1))) {
        riot_adjust = 1;
    } else {
        riot_adjust = 0;
    }
    for (int loop = 1; loop < 10; loop++) {
        if (GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & (1 << faction)) {
            if (!(BaseCurrent->governor & GOV_MANAGES_CITIZENS_SPECS)) return;
            if (!(BaseCurrent->governor & GOV_ACTIVE)) return;
        }
        if (BaseNutrientAdequate
            && BaseCurrent->drone_total + riot_adjust > BaseCurrent->talent_total
            && !has_project(SP_TELEPATHIC_MATRIX, faction)) {
            {
                int best = best_specialist();
                int slot = BaseCurrent->specialist_total++;
                if (slot < 16) {
                    const int word = slot >> 3;
                    const int shift = (slot & 7) * 4;
                    BaseCurrent->spec[word] &= ~(15 << shift);
                    BaseCurrent->spec[word] |= best << shift;
                }
            }
            base_compute(1);
            if (BaseCurrent->drone_total + riot_adjust <= BaseCurrent->talent_total) {
                if (UnkGlobal0093A934 == 0) return;
                if (faction != ConsoleGlobal.faction_id_) return;
                if (PlayersData[faction].se_alloc_psych >= 2) return;
                if (BaseCurrent->specialist_total - BaseCurrent->specialist_unk_1 <= 1) return;
                const int production = BaseCurrent->queue_production_id[0];
                if (production == -6 || production == -11 || production == -12
                    || production == -17 || production == -18) return;
                if (production == 8 && has_project(SP_VIRTUAL_WORLD, faction)) return;
                if ((Players[faction].rule_flags & RFLAG_ALIEN) != 0) {
                    popb("PSYCHREQUEST", 0, -1, "Alopdir.pcx", 0);
                } else {
                    popb("PSYCHREQUEST", 0, -1, "talent_sm.pcx", 0);
                }
                return;
            }
        } else {
            int old_total = BaseCurrent->specialist_total;
            if (old_total - BaseCurrent->specialist_unk_1 <= 0) return;
            if (BaseCurrent->state & BSTATE_DRONE_RIOTS_ACTIVE) return;
            if (BaseCurrent->drone_total
                && BaseCurrent->drone_total + riot_adjust >= BaseCurrent->talent_total
                && !has_project(SP_TELEPATHIC_MATRIX, faction)) {
                return;
            }
            BaseCurrent->specialist_total = old_total - 1;
            base_compute(1);
            if (BaseCurrent->drone_total + riot_adjust > BaseCurrent->talent_total
                && !has_project(SP_TELEPATHIC_MATRIX, faction)) {
                {
                    int best = best_specialist();
                    int slot = BaseCurrent->specialist_total++;
                    if (slot < 16) {
                        const int word = slot >> 3;
                        const int shift = (slot & 7) * 4;
                        BaseCurrent->spec[word] &= ~(15 << shift);
                        BaseCurrent->spec[word] |= best << shift;
                    }
                }
                return;
            }
            if (old_total <= BaseCurrent->specialist_total) return;
        }
    }
}

/*
Purpose: Calculate facility maintenance cost for specified faction.
// ORIGINAL: 0x004F6510 ?fac_maint@@YAHHH@Z 0x004F6510-0x004F65E7 BYTE_EXACT
// size      215 bytes
// prototype int (__cdecl ?fac_maint@@YAHHH@Z)(int facilityID, int factionID)
// callers   7   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0057EFA0 0x005B9F20
// LEVER: three changes. (1) walking pointers `bonus_id`/`bonus_val` over
//        Players[faction_id].faction_bonus_id/faction_bonus_val1, stepped
//        alongside `i`, instead of re-indexing the arrays each access
//        (lever 7). (2) dropped the `bonus_count` local so the loop bound
//        re-reads Players[faction_id].faction_bonus_count from the global
//        every iteration, matching the image. (3) compute diff_factor
//        before calling best_reactor(), matching the image's instruction
//        order. Best similarity 0.820 -> 0.924 (agreeing count drops 8->4,
//        but similarity - the tool's real tiebreak - is higher).
Return Value: Facility maintenance cost
Status: Complete
*/
int __cdecl fac_maint(int facility_id, int faction_id) {
    if (facility_id == FAC_COMMAND_CENTER) {
        int diff_factor = base_yield_add(PlayersData[faction_id].diff_level, 1) / 2;
        int reactor = best_reactor(faction_id);
        if (reactor < 0 || diff_factor < 0) {
            return 0;
        }
        return (reactor > diff_factor) ? diff_factor : reactor;
    }
    for (int i = 0; i < Players[faction_id].faction_bonus_count; i++) {
        if ((Players[faction_id].faction_bonus_id[i] == RULE_FACILITY
            || (Players[faction_id].faction_bonus_id[i] == RULE_FREEFAC
                && has_tech(Facility[Players[faction_id].faction_bonus_val1[i]].preq_tech, faction_id)))
            && Players[faction_id].faction_bonus_val1[i] == (int)facility_id) {
            return 0;
        }
    }
    return base_yield_signed_bits(Facility[facility_id].maint);
}

/*
Purpose: Calculate overall maintenance cost for the currently selected base.
// ORIGINAL: 0x004F65F0 ?base_maint@@YAXXZ 0x004F65F0-0x004F67E3 BYTE_EXACT
// TRIED: the image's own call list has ONE call to bitmask() (0x50BA00);
//            this tree's compile makes TWO, so `set_fac(fac,
//            BaseIDCurrentSelected, false)` - a BYTE_EXACT standalone
//            function elsewhere, called 10 times - looked like it was
//            auto-inlining here and contributing its own internal bitmask()
//            call on top of the loop guard's. Hand-inlining set_fac's body
//            (set==false is a literal at this call site, so only the
//            `&= ~mask` arm survives) does NOT improve it: best similarity
//            goes 0.702 -> 0.696, a wash rather than a win, so the
//            standalone `set_fac()` call is kept. Best flag set
//            /O2 /Gy /GR- /Oy- /GX either way.
// size      499 bytes
// prototype
// callers   1   call targets   5
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0048C650 0x004E4430 0x004F6510 0x0050BA00 0x00625E50
Return Value: Base maintenance cost
Status: Complete
*/
void __cdecl base_maint() {
    uint32_t faction_id = BaseCurrent->faction_id_current;
    for (int fac = 1; fac < FacilitySPStart; fac++) {
        if (has_fac(fac, BaseIDCurrentSelected, 0)) {
            int maint = fac_maint(fac, faction_id);
            if (has_project(SP_SELF_AWARE_COLONY, faction_id)) {
                maint += (PlayersData[faction_id].flags & PFLAG_SELF_AWARE_COLONY_LOST_MAINT) != 0;
                if (maint & 1) {
                    PlayersData[faction_id].flags |= PFLAG_SELF_AWARE_COLONY_LOST_MAINT;
                } else {
                    PlayersData[faction_id].flags &= ~PFLAG_SELF_AWARE_COLONY_LOST_MAINT;
                }
                maint /= 2;
            }
            PlayersData[faction_id].energy_reserves -= maint;
            PlayersData[faction_id].maint_cost_total += maint;
            if (PlayersData[faction_id].energy_reserves < 0) {
                if (PlayersData[faction_id].diff_level <= DLVL_SPECIALIST) {
                    PlayersData[faction_id].energy_reserves = 0;
                } else if (BaseCurrent->queue_production_id[0] == -fac) {
                    PlayersData[faction_id].energy_reserves = 0;
                } else {
                    set_fac(fac, BaseIDCurrentSelected, false);
                    PlayersData[faction_id].energy_reserves
                        = cost_factor(BaseCurrent->faction_id_current, RSC_MINERALS, -1)
                          * Facility[fac].cost;
                    parse_say(1, Facility[fac].name, -1, -1);
                    popb("POWERSHORT", 0x10000, 14, "genwarning_sm.pcx", NULL);
                }
            }
        }
    }
}


// POP2 is implemented by the owning Popup API in popup.cpp and declared in
// popup.h.
int __cdecl POP2(const char *caption, const char *image, int base_id);

// The superseded partial scaffold was deleted after full owner recovery.
// Its seed path below is historical Git provenance, not a source dependency.
// ORIGINAL: 0x004F67F0 ?base_ecology@@YAXXZ 0x004F67F0-0x004F79B3
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/004f67f0.cpp; recovery/tasks/base-colony-economy-coverage-wave.json; recovery/tests/test_base_colony_production.py", "Pinned image 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf, full original span 4F67F0..4F79B3.", "Native 4F6C73 reads faction difficulty, not ranking; 4F739B reads population total and 4F73B9 independently reloads ecology.", "Native callbacks are followed by live ecology reloads at 4F6E94/4F6EC2 (including the current viewer), 4F7024, 4F70C7, 4F715B, 4F73B9, 4F74A2/4F7512, 4F75B8 and 4F76EE; signed 32-bit climate arithmetic retains native wrapping.", "Native reuses the spread cursor for the 19-row terraforming scan; preserve that original cursor behavior rather than silently changing the event."], "reason": "Recover the complete ecology event owner using real Base, faction, map, terraforming, Vehicle and climate storage. Retain original seed and obligations; this is no new byte or game acceptance claim."}
// symbol    ?base_ecology@@YAXXZ
// TRIED: heavily time-boxed given batch budget - only the leading probability gate (faction-table accumulator update, a rand()%100 roll against a base-stat threshold, and the two disabling-flag checks) is transcribed; the ecology event itself, most of the function, is a deliberate stub. MISMATCH #0, prologue only.
void __cdecl base_ecology() {
    int faction = BaseCurrent->faction_id_current;
    int divisor = range(4 - PlayersData[faction].soc_effect_pending.values_[SocialEffect::PLANET], 1, 99);
    PlayersData[faction].unk_49 += BaseCurrent->eco_damage * 4 / divisor;
    if ((int)rnd(100, "BASEECOLOGY") >= BaseCurrent->eco_damage
            / ((GameDataStateStorage.fields_.game_state_ & 0x100000) ? 2 : 1)) {
        return;
    }
    if (GameDataStateStorage.fields_.game_rules_ & 0x200000) {
        return;
    }
    if ((GameDataStateStorage.fields_.game_state_ & 1)
            && !(GameDataStateStorage.fields_.game_state_ & 8)) {
        return;
    }
    // Pick the tile that grows fungus: next to land fungus, around this base.
    int best_roll = 0;
    int best_index = -1;
    int i;
    for (i = 1; i < 49; i++) {
        int x = wrap_map_x(BaseCurrent->x + RadiusOffsetX[i]);
        int y = BaseCurrent->y + RadiusOffsetY[i];
        if (!on_map(x, y)) {
            continue;
        }
        Map *tile = map_loc(x, y);
        if ((tile->bit & BIT_FUNGUS) && (tile->climate & ~0x1F) >= ALT_BIT_OCEAN_SHELF) {
            continue;
        }
        if (base_who(x, y) >= 0) {
            continue;
        }
        int prob = 0;
        if (i >= 25) {
            // Outer ring: only while nothing was found yet, and only tiles
            // whose nearest base is this one.
            if (best_roll) {
                continue;
            }
            if (base_find(x, y) != BaseIDCurrentSelected) {
                continue;
            }
            prob = 1;
        }
        for (int j = 0; j < 4; j++) {
            int x2 = wrap_map_x(x + RiverStepX[j]);
            int y2 = y + RiverStepY[j];
            if (on_map(x2, y2)) {
                Map *neighbor = map_loc(x2, y2);
                if ((neighbor->bit & BIT_FUNGUS)
                        && (neighbor->climate & ~0x1F) >= ALT_BIT_OCEAN_SHELF) {
                    prob += 2;
                }
            }
        }
        if (prob) {
            if (prob > 1) {
                prob += bit_count(bit_at(x, y) & MapTileFlagClearMask);
                if (i < 9) {
                    prob++;
                }
            }
            if (is_ocean(x, y)) {
                prob = (prob + 1) / 2;
            }
            prob *= 10;
            int roll = rnd(prob, "BASEECOLOGY2");
            if (roll > best_roll) {
                best_roll = roll;
                best_index = i;
            }
        }
    }
    if (best_index <= 0) {
        return;
    }
    BOOL shown = false;
    int report_terraform = -1;
    int ecology = ++PlayersData[faction].planet_ecology;
    int event_x = wrap_map_x(BaseCurrent->x + RadiusOffsetX[best_index]);
    int event_y = BaseCurrent->y + RadiusOffsetY[best_index];
    int spread = 1;
    if (is_ocean(event_x, event_y) && ecology >= 4) {
        int chance = 10 - ecology / 8 - PlayersData[faction].diff_level;
        if (!rnd(chance, "SEASPREAD")) {
            spread = 9;
        }
    }
    for (i = 0; i < spread; i++) {
        int x = wrap_map_x(event_x + RadiusOffsetX[i]);
        int y = event_y + RadiusOffsetY[i];
        if (!on_map(x, y)) {
            continue;
        }
        if (i && !is_ocean(x, y)) {
            continue;
        }
        Map *tile = map_loc(x, y);
        uint32_t bit = tile->bit;
        if ((bit & BIT_FUNGUS) && (tile->climate & ~0x1F) >= ALT_BIT_OCEAN_SHELF) {
            continue;
        }
        if (base_who(x, y) >= 0) {
            continue;
        }
        if ((tile->bit2 & 0x80000002) == 2) {
            continue;
        }
        if (report_terraform < 0) {
            // BUG(original): the improvement scan reuses the spread loop's
            // counter, so after it runs (i == 19) the spread loop ends.
            for (i = 0; i < 19; i++) {
                if (Terraforming[i].bit & MapTileFlagClearMask & bit) {
                    report_terraform = i;
                }
            }
        }
        bit_set(x, y, BIT_FUNGUS, true);
        bit_set(x, y, MapTileFlagClearMask, false);
        if ((map_loc(x, y)->climate >> 5) < ALT_OCEAN_SHELF) {
            world_alt_set(x, y, ALT_OCEAN_SHELF, false);
        }
        if (is_known(x, y, ConsoleGlobal.faction_id_)
                || (GameDataStateStorage.fields_.game_state_ & 0x80)) {
            shown = true;
            if (faction == ConsoleGlobal.faction_id_) {
                ConsoleGlobal.focus(x, y, faction);
                if (PlayersData[faction].planet_ecology < 2) {
                    interlude(3, nullptr, 1, 0);
                }
                boom(x, y, 0x80);
            }
            synch_bit(x, y, ConsoleGlobal.faction_id_);
            draw_tile(x, y, 2);
        }
    }
    if (shown) {
        if (!HIBYTE(GetAsyncKeyState(VK_SHIFT)) && !IsMultiplayerNet) {
            clock_wait(20);
        }
        if (faction == ConsoleGlobal.faction_id_
                || veh_who(event_x, event_y) == ConsoleGlobal.faction_id_) {
            MapWinTable[0]->set_center(event_x, event_y, 1);
            ConsoleGlobal.focus(event_x, event_y, faction);
            parse_says(0, BaseCurrent->name_string, -1, -1);
            if (report_terraform >= 0 && faction == ConsoleGlobal.faction_id_) {
                // name and name_sea are read as a two-entry run.
                parse_say(1, Terraforming[report_terraform].name[is_ocean(event_x, event_y)],
                          -1, -1);
                POP2("FUNGUSGROWS2", "fung_sm.pcx", BaseIDCurrentSelected);
            } else {
                POP2("FUNGUSGROWS", "fung_sm.pcx", -1);
            }
        }
        if (faction == ConsoleGlobal.faction_id_ && PlayersData[faction].planet_ecology >= 2) {
            interlude(5, BaseCurrent->name_string, 1, 0);
        }
    }
    if (!is_human(faction) && !rnd(2, "LEADERPLANET1")) {
        int odds = GameDataStateStorage.fields_.diff_level_current_ + 1;
        if (rnd(odds, "LEADERPLANET2")) {
            PlayersData[faction].planet_ecology--;
            return;
        }
    }
    if (PlayersData[faction].flags & 0x4000) {
        PlayersData[faction].flags &= ~0x4000u;
        return;
    }
    if (PlayersData[faction].planet_ecology < 7
            || base_project(SP_VOICE_OF_PLANET) != SP_Unbuilt) {
        return;
    }
    // Planet strikes back: native life rises from the new fungus.
    if (faction == ConsoleGlobal.faction_id_) {
        interlude(8, nullptr, 1, 0);
    }
    if (!(rand() % 4) && ExpansionEnabled && !is_ocean(event_x, event_y)) {
        veh_init(BSC_FUNGAL_TOWER, 0, event_x, event_y);
    }
    int gated_spawns = 0;
    int created = 0;
    for (i = 0; i < (PlayersData[faction].planet_ecology < 9
                     ? PlayersData[faction].planet_ecology : 9); i++) {
        shown = wrap_map_x(event_x + RadiusOffsetX[i]);
        report_terraform = event_y + RadiusOffsetY[i];
        if (!on_map(shown, report_terraform)) {
            continue;
        }
        Map *tile = map_loc(shown, report_terraform);
        if (!(tile->bit & BIT_FUNGUS) || (tile->climate & ~0x1F) < ALT_BIT_OCEAN_SHELF) {
            continue;
        }
        int base_id = base_find(shown, report_terraform);
        if (base_id < 0 || Bases[base_id].faction_id_current != faction) {
            continue;
        }
        if (veh_who(shown, report_terraform) > 0 || base_who(shown, report_terraform) > 0) {
            continue;
        }
        int region = region_at(shown, report_terraform);
        int unit_id;
        if (region >= 0x40) {
            unit_id = (gated_spawns % 3) ? BSC_ISLE_OF_THE_DEEP : BSC_LOCUSTS_OF_CHIRON;
        } else {
            unit_id = PlayersData[faction].region_total_bases[region]
                ? BSC_MIND_WORMS : BSC_LOCUSTS_OF_CHIRON;
        }
        if (unit_id != BSC_MIND_WORMS) {
            // The image reads current_num_bases (+0x105C) here.
            gated_spawns++;
            if (gated_spawns >= PlayersData[faction].current_num_bases * 2 / 3
                    || gated_spawns >= PlayersData[faction].planet_ecology / 3) {
                continue;
            }
        }
        created++;
        if (unit_id == BSC_MIND_WORMS && !(rand() % 5) && ExpansionEnabled) {
            unit_id = BSC_SPORE_LAUNCHER;
        }
        int veh_id = veh_init(unit_id, 0, shown, report_terraform);
        if (veh_id >= 0) {
            Vehs[veh_id].order_auto_type = static_cast<uint8_t>(faction);
        }
        draw_tile(shown, report_terraform, 2);
    }
    if (anything_at(event_x, event_y) <= 0) {
        int region = region_at(event_x, event_y);
        int unit_id = PlayersData[faction].region_total_bases[region]
            ? (region >= 0x40 ? BSC_ISLE_OF_THE_DEEP : BSC_MIND_WORMS)
            : BSC_LOCUSTS_OF_CHIRON;
        while (created < PlayersData[faction].planet_ecology / 3
                && created < BaseCurrent->population_size) {
            if (unit_id == BSC_MIND_WORMS && !(rand() % 5) && ExpansionEnabled) {
                unit_id = BSC_SPORE_LAUNCHER;
            }
            veh_init(unit_id, 0, event_x, event_y);
            created++;
            if (unit_id == BSC_SPORE_LAUNCHER) {
                unit_id = BSC_MIND_WORMS;
            }
        }
    }
    if (is_human(faction) || GameDataStateStorage.fields_.diff_level_current_ >= 4) {
        // Global warming.
        GameDataStateStorage.fields_.ecology_event_accumulator_++;
        int sea_level = range(MapSeaLevel, 0, 100);
        int rises = WorldBuilder.sea_level_rises;
        int rise_divisor = range(rises, 1, 99);
        if (Rules.freq_global_warming_num * GameDataStateStorage.fields_.ecology_event_accumulator_
                / Rules.freq_global_warming_denom >= sea_level * 3 / rise_divisor + 6) {
            GameDataStateStorage.fields_.ecology_event_accumulator_ = 0;
            best_index = range(PlayersData[faction].planet_ecology / 6 - 1, 1, 3);
            MapSeaLevelCouncil += best_index;
            GameDataStateStorage.fields_.climate_sea_level_delta_ += rises * best_index / 3;
            GameDataStateStorage.fields_.climate_rise_step_ =
                GameDataStateStorage.fields_.climate_sea_level_delta_ < 0
                    ? -GameDataStateStorage.fields_.climate_sea_level_delta_
                    : GameDataStateStorage.fields_.climate_sea_level_delta_;
            GameDataStateStorage.fields_.climate_rise_limit_ = 20;
            GameDataStateStorage.fields_.climate_rise_accum_ = 0;
            parse_num(0, 20);
            int change = GameDataStateStorage.fields_.climate_sea_level_delta_ < 0
                ? -GameDataStateStorage.fields_.climate_sea_level_delta_
                : GameDataStateStorage.fields_.climate_sea_level_delta_;
            parse_num(1, change * 1000 / 15);
            if (faction == ConsoleGlobal.faction_id_) {
                popp(ScriptPopupFile, "TRIGGERWARMING", 0, "searis_sm.pcx", nullptr);
                return;
            }
            parse_set(Players[faction].noun_gender, Players[faction].is_noun_plural);
            parse_says(0, Players[faction].noun_faction, -1, -1);
            popp(ScriptPopupFile, "TRIGGERWARMING2", 0, "searis_sm.pcx", nullptr);
            return;
        }
    }
    if (PlayersData[faction].planet_ecology < 10
            || (GameDataStateStorage.fields_.game_state_ & 0x20000)) {
        return;
    }
    // Volcano at a random ocean tile with nobody around.
    int rand_x = rnd(MapLongitudeBounds, "VOLCa");
    int volcano_y = rnd(MapLatitudeBounds, "VOLCb");
    int volcano_x = rand_x;
    if (rand_x & 1) {
        volcano_x = rand_x - 1;
    }
    if (volcano_y & 1) {
        volcano_x++;
    }
    if (!on_map(volcano_x, volcano_y)) {
        return;
    }
    Map *volcano_tile = map_loc(volcano_x, volcano_y);
    if ((volcano_tile->climate & ~0x1F) >= ALT_BIT_SHORE_LINE) {
        return;
    }
    for (i = 0; i < 49; i++) {
        int x = wrap_map_x(volcano_x + RadiusOffsetX[i]);
        int y = volcano_y + RadiusOffsetY[i];
        if (on_map(x, y) && anything_at(x, y) >= 0) {
            break;
        }
    }
    if (i != 49) {
        return;
    }
    if ((PlayersData[ConsoleGlobal.faction_id_].flags & PFLAG_MAP_REVEALED)
            || (volcano_tile->visibility & (1 << ConsoleGlobal.faction_id_))) {
        ConsoleGlobal.focus(volcano_x, volcano_y, ConsoleGlobal.faction_id_);
    }
    GameDataStateStorage.fields_.game_state_ |= 0x20000;
    world_volcano(volcano_x, volcano_y, 1);
    world_climate();
    draw_map(1);
    clock_wait(1000);
    GenderDefault = Players[ConsoleGlobal.faction_id_].is_leader_female;
    PluralityDefault = 0;
    parse_says(0, Players[ConsoleGlobal.faction_id_].title_leader, -1, -1);
    PluralityDefault = 0;
    GenderDefault = Players[ConsoleGlobal.faction_id_].is_leader_female;
    parse_says(1, Players[ConsoleGlobal.faction_id_].name_leader, -1, -1);
    if (is_known(volcano_x, volcano_y, ConsoleGlobal.faction_id_)) {
        popp(ScriptPopupFile, "VOLCANO", 0, "volc_sm.pcx", nullptr);
    }
}

// Production sets this request for the subsequent upkeep zoom. Four-byte
// loader-zero initial storage; the original identity is not a runtime address.
// RECOVERY_OBJECT: v2 0x0090E934 0x0090E938 ?BaseProductionZoomRequested@@3HA 0x004F07E0
int BaseProductionZoomRequested = 0;
// Nonzero skips unit creation before veh_init. The indirect producer remains
// unverified; do not infer a permanently false flag from loader-zero storage.
// RECOVERY_OBJECT: v2 0x0090EA40 0x0090EA44 ?BaseProductionUnitCreationDisabled@@3HA 0x004F07E0
int BaseProductionUnitCreationDisabled = 0;

// ORIGINAL: 0x004F79C0 ?base_upkeep@@YAHH@Z 0x004F79C0-0x004F7FDA BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence":["original:0x004F79C0-0x004F7FDA:pinned Base upkeep span","original:0x004F7D9E,0x004F7DDC,0x004F7E1D:BaseCurrent is reloaded at the native parser/climate callback boundaries","original:0x004F7BF6-0x004F7C01:actor and partner rule-flags use distinct faction indexes","source:src/base.h:Base, Bases, BaseCurrent, and the canonical Base-upkeep call surface","source:src/faction.h:Player and PlayerData arrays distinguish actor, partner, loans, energy and sanctions"],"reason":"Recover Base upkeep with native callback reloads and independently indexed actor/partner commerce state through the existing Base, Player and PlayerData owners."}
// LEVER: set_base-like prologue with a local &Bases[base_id] (reused when
//        BaseCurrent is restored), plain int accumulation, signed /8 %8 bit
//        test, project walk through a local project_base pointer, and the
//        focus/rename visibility tests written inline. Remaining gap: the
//        commerce loop's three pointer inductions take different registers.
int __cdecl base_upkeep(int base_id) {
    // Native event/log text remains inline: "al_col_sm.pcx" 0x689EA4..0x689EB2, "RENAMEBASE" 0x689EB4..0x689EBF and 0x689ED0..0x689EDB, "colfoun_sm.pcx" 0x689EC0..0x689ECF, "UPKEEP zoom-to-base" 0x689EDC..0x689EF0, and "FINISHED zoom-to-base" 0x689EF0..0x689F06.

    BaseIDCurrentSelected = base_id;
    Base *base = &Bases[base_id];
    BaseCurrent = base;
    const int faction_id = BaseCurrent->faction_id_current;
    BaseProductionZoomRequested = 0;
    Bases[base_id].state &= 0xDFFFFDFFu;
    base_compute(1);
    base_production();
    base_hurry();

    BaseCurrent->minerals_accumulated_2 = BaseCurrent->minerals_accumulated;
    BaseCurrent->production_id_last = BaseCurrent->queue_production_id[0];
    if (base_growth() != 0) return 1;
    if (GameDataStateStorage.fields_.turn_current_num_ > 1) base_check_support();

    BaseUpkeepStage = 1;
    base_compute(1);

    int tile_weight;
    int region = region_at(BaseCurrent->x, BaseCurrent->y);
    if (PlayersData[faction_id].region_base_plan[region] == 7
        && PlayersData[faction_id].ai_fight <= 0) {
        tile_weight = 4;
    } else {
        tile_weight = (PlayersData[faction_id].region_base_plan[region] != 2) + 1;
    }
    PlayersData[faction_id].unk_47 += bit_count(BaseCurrent->resource_sqr_radius) * tile_weight;

    BaseUpkeepStage = 2;
    if (GameDataStateStorage.fields_.turn_current_num_ > 1) {
        base_drones();
        base_doctors();
        if (BaseCurrent->faction_id_current != faction_id) return 1;
        base_energy_costs();
        if (GameDataStateStorage.fields_.turn_current_num_ > 5 || IsMultiplayerNet != 0 ||
            (GameDataStateStorage.fields_.game_preferences_ & 0x20u) == 0) {
            base_research();
        }
    }

    BaseIDCurrentSelected = base_id;
    BaseCurrent = base;
    base_compute(0);
    base_ecology();
    PlayersData[faction_id].energy_reserves += BaseCurrent->economy_total;

    // Actor and partner rule flags use distinct faction indexes.
    // A walked receipts pointer orders the loop's three pointer inductions.
    int *receipts = BaseCommerceReceipts;
    for (int other_faction = 1; other_faction < MaxPlayerNum; ++other_faction, ++receipts) {
        if (other_faction != faction_id && !(Players[faction_id].rule_flags & 0x80)
            && !(Players[other_faction].rule_flags & 0x80)) {
            PlayersData[faction_id].turn_commerce_income +=
                (PlayersData[faction_id].sanction_turns == 0
                 && PlayersData[other_faction].sanction_turns == 0)
                ? *receipts : 0;
        }
    }

    if ((Players[faction_id].rule_flags & 0x80) != 0) {
        int count = 0;
        for (int facility_id = 0; facility_id < 0x46; ++facility_id) {
            if (BaseCurrent->facilities_built[facility_id / 8] & (1 << (facility_id % 8))) {
                ++count;
            }
        }
        const int *projects = GameDataStateStorage.fields_.secret_project_.project_base;
        for (int project = 0; project < MaxSecretProjectNum; ++project) {
            if (projects[project] == base_id) count += 5;
        }
        int alien_commerce = count / 2;
        if (PlayersData[faction_id].sanction_turns != 0) alien_commerce = 0;
        PlayersData[faction_id].turn_commerce_income += alien_commerce;
    }

    if (GameDataStateStorage.fields_.turn_current_num_ > 1) base_maint();
    base_terraform(0);
    if ((BaseCurrent->state & 0x10000u) != 0 && rnd(3, "BASE1") == 0 &&
        (GameDataStateStorage.fields_.game_state_ & STATE_UNK_1) == 0) {
        if (Bases[BaseIDCurrentSelected].faction_id_current == ConsoleGlobal.faction_id_
            || ((1 << ConsoleGlobal.faction_id_) & Bases[BaseIDCurrentSelected].visibility)) {
            if (!ConsoleGlobal.focus(BaseCurrent->x, BaseCurrent->y, ConsoleGlobal.faction_id_)) {
                draw_tiles(BaseCurrent->x, BaseCurrent->y, 2);
            }
        }
        // 0x4F7D9E reloads after focus/draw callbacks before this phrase.
        parse_says(1, BaseCurrent->name_string, -1, -1);
        GenderDefault = Players[faction_id].noun_gender;
        PluralityDefault = Players[faction_id].is_noun_plural;
        parse_says(0, Players[faction_id].noun_faction, -1, -1);
        // 0x4F7DDC reloads after parse_says(0); use that same live Base for
        // the climate tile and name_base's name pointer.
        Base &climate_base = *BaseCurrent;
        const int event_tile = static_cast<int>(climate_base.y) *
            static_cast<int>(MapLongitude) + (static_cast<int>(climate_base.x) >> 1);
        const int final_name = (MapTiles[event_tile].climate & ~0x1F) < 0x60;
        name_base(faction_id, climate_base.name_string, 1, final_name);
        // 0x4F7E1D reloads after name_base before parsing the renamed phrase.
        parse_says(2, BaseCurrent->name_string, -1, -1);
        if (Bases[BaseIDCurrentSelected].faction_id_current == ConsoleGlobal.faction_id_
            || ((1 << ConsoleGlobal.faction_id_) & Bases[BaseIDCurrentSelected].visibility)) {
            draw_map(1);
            if (Players[faction_id].rule_flags & 0x80) {
                popp(ScriptPopupFile, "RENAMEBASE", 0, "al_col_sm.pcx", 0);
            } else {
                popp(ScriptPopupFile, "RENAMEBASE", 0, "colfoun_sm.pcx", 0);
            }
        }
        BaseCurrent->state &= ~0x10000u;
        BaseCurrent->state |= 0x80u;
    }

    BaseComputedID = -1;
    if (BaseIDCurrentSelected == TutWinBaseWindow.focused_base_id()) {
        TutWinBaseWindow.on_redraw();
    }
    if (BaseProductionZoomRequested != 0) {
        // stand-in: the low-byte read VC6 narrows to without the volatile below.
        const uint8_t state_for_zoom = static_cast<uint8_t>(GameDataStateStorage.fields_.game_state_);
        if ((state_for_zoom & 1u) == 0 || (state_for_zoom & 8u) != 0) {
            log_say("UPKEEP zoom-to-base", base_id, 0, 0);
            TutWinBaseWindow.zoom(base_id, 1);
            log_say("FINISHED zoom-to-base", base_id, 0, 0);
        }
    }

    BaseUpkeepStage = 0;
    if (BaseCurrent->assimilation_turns_left != 0) {
        --BaseCurrent->assimilation_turns_left;
        if (BaseCurrent->faction_id_former == BaseCurrent->faction_id_current &&
            BaseCurrent->assimilation_turns_left != 0) {
            --BaseCurrent->assimilation_turns_left;
        }
        if (BaseCurrent->assimilation_turns_left == 0) {
            BaseCurrent->faction_id_former = BaseCurrent->faction_id_current;
        }
    }
    if (BaseCurrent->nerve_staple_turns_left != 0) --BaseCurrent->nerve_staple_turns_left;
    BaseCurrent->state &= 0xFFDFFFF7u;
    // stand-in: a volatile read forces the image's turn-then-base load order;
    // every plain spelling of the sum loads BaseIDCurrentSelected first.
    const int turn = *(volatile int *)&GameDataStateStorage.fields_.turn_current_num_;
    const int selected = BaseIDCurrentSelected;
    if (((selected + turn) & 7) == 0) {
        BaseCurrent->state &= 0xFFEFFFFFu;
    }
    return 0;
}

/*
Purpose: Count matching base names. If any exist, truncate the target
         to its original 21-character prefix and append a suffix based on the
         full collision count.
// ORIGINAL: 0x004F7FE0 ?make_base_unique@@YAXH@Z 0x004F7FE0-0x004F8087 BYTE_EXACT
// size      167 bytes
// prototype void (__cdecl ?make_base_unique@@YAXH@Z)(int baseID)
// callers   3   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00645470 0x00645660 0x0064FC88
// LEVER: dropped std::string entirely for the image's strcat/_itoa idiom
//        operating directly on Bases[base_id].name_string (no separate core
//        copy) - the image has no C++ library here. Took the body from
//        0/61 to 3/61 agreeing and removed the whole SEH prologue
//        (push -1/push 0/mov eax,fs:[0]) the std::string version needed.
Return Value: n/a
Status: Complete
*/
void __cdecl make_base_unique(int base_id) {
    int collisions = 0;
    for (int i = 0; i < GameDataStateStorage.fields_.base_current_count_; i++) {
        if (base_id != i &&
            !strcmp(Bases[i].name_string, Bases[base_id].name_string)) {
            ++collisions;
        }
    }
    if (collisions != 0) {
        Bases[base_id].name_string[21] = 0;
        // make_base_unique passes this one-byte delimiter to strcat; collision digits use a separate buffer.
// RECOVERY_LITERAL: v1 0x00689F08 0x00689F0A $SG65695 0x004F7FE0
        strcat(Bases[base_id].name_string, "-");
        char digits[0x50];
        _itoa(collisions + 1, digits, 10);
        strcat(Bases[base_id].name_string, digits);
    }
}

/*
Purpose: Check if a faction has the specified secret project built in a base they control.
// ORIGINAL: 0x004F80D0 ?has_project@@YAHHH@Z 0x004F80D0-0x004F8103 BYTE_EXACT
// body      src/base.h
// size      51 bytes
// prototype int (__cdecl ?has_project@@YAHHH@Z)(int projectID, int factionID)
// callers   8   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Does faction have Secret Project? true/false
Status: Complete
*/
