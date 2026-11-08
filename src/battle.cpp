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
#include "popup.h"
#include "basepop.h"
#include "text.h"
#include "basewin.h"
#include "buffer.h"
#include "sprite.h"
#include "random.h"
#include "interlude.h"
#include "time.h"
#include "win.h"
#include "veh.h"
#include "enemy_pb_policy.h"
#include "council.h"
#include "lock.h"  // map_tile_bytes() - the map-record table pointer
#include "alpha.h"
#include "base.h"
#include "faction.h"
#include "game.h"
#include "interface_palette.h"
#include "general.h" // range
#include "flic.h"
#include "log.h"
#include "map.h"
#include "strings.h"
#include "palette.h"  // Reactor-zero naming observes the existing palette slot.
#include "technology.h"
#include "terraforming.h"
#include "deletionlist.h"  // NetDaemonNet owns the embedded deletion queue
#include "tutwin.h"  // TutWinBaseWindow, TutorialWindow
#include "mapwin.h"  // draw_tile - the free single-tile redraw the removals end on
#include "netmsg.h"  // NetMsgPrimary - the promote family reports through it
#include "xpops.h"  // X_pop - transferred-unit single-player notifications.
#include "netdaemon.h"  // NetDaemonNet, synch_veh / order_veh - the action family's net half
#include "path.h"  // Paths - the move cost table the go-to verbs consult
#include "console.h"  // ConsoleGlobal - action_gate re-focuses the Console
#include "statuswin.h"  // ConsoleStatusWin owns its hovered vehicle selection.
#include "caviar.h"
#include "font.h"
#include "font_queue.h"
#include "crt_random_inline.h"
#include "infowin.h"
#include "worldwin.h"
#include "reportif.h"
#include "maininterface.h"

#include "wave.h"
#include "fx.h"
#include <mmsystem.h>
#include "designwin.h" // Crawler animation advances while the design window is visible.
#include "vector.h"  // vector_add operates on actual three-float arrays.


/*
Purpose: Calculate maximum range a faction's units can drop (air drops, Drop Pods).
// ORIGINAL: 0x00500320 ?drop_range@@YAHH@Z 0x00500320-0x0050037C BYTE_EXACT
// LEVER: has_tech_call, not has_tech - the image CALLS 0x5B9F20 here rather
//        than open-coding the preq walk, so the `MEASURED inline` in
//        technology.h has to be reached through the non-inline forwarder or
//        /Ob1 inlines it and the body doubles (0/36 at /O2, 3/36 at /O2 /Oy-
//        before the switch; it had held only under the retired /Ob0 set).
// LEVER: has_project(SP_SPACE_ELEVATOR, faction_id) written out by hand
//        (base_project's `*(&SecretProject->human_genome_project + id)` then
//        the owning base's faction) rather than called - at the `/Ob0` flag
//        set this needs (so has_tech does NOT auto-inline, matching the
//        image's real `call 0x5b9f20`), a call to the shared has_project()
//        inline does not fold back open and compiles a real `push 0x1b; call`
//        instead. Moved 10/36 (0.769) -> 17/36 (0.928): the SecretProject
//        constant (0x9a6580) now lands exactly where the image has it.
// size      92 bytes
// prototype int (__cdecl ?drop_range@@YAHH@Z)(int factionID)
// callers   3   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005B9F20
Return Value: Max range
Status: Complete
*/
int __cdecl drop_range(int faction_id) {
    // has_tech() is a real call here (the image's `call 0x5b9f20`), but
    // has_project() is written out rather than called: at `/Ob0` - needed so
    // has_tech does NOT auto-inline, matching the image - a call to the
    // shared `has_project()` inline does not fold back open. Written out in
    // has_project()'s own shape, ternary and BOOL result included: the
    // negated `base_id < 0 || owner != faction_id` form compiled a bare
    // `je` where the image materialises the comparison with `sete al` first.
    if (!has_tech(Rules.tech_orb_insert_sans_spc_elev, faction_id)) { // default 'Graviton Theory'
        int base_id = GameDataStateStorage.fields_.secret_project_[SP_SPACE_ELEVATOR];
        BOOL has_orb_insert = (base_id < 0) ? false
            : (Bases[base_id].faction_id_current == faction_id);
        if (!has_orb_insert) {
            return Rules.max_airdrop_sans_orb_insert;
        }
    }
    return (MapLongitudeBounds <= MapLatitudeBounds) ? MapLatitudeBounds : MapLongitudeBounds;
}

/*
Purpose: Find whether a faction can airdrop a unit onto a tile.
// ORIGINAL: 0x00500380 sub_500380 0x00500380-0x005004EA BYTE_EXACT
// size      362 bytes
// prototype void (__cdecl sub_500380)(int faction_id, int x, int y)
// callers   0   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005B9F20 0x005BF1F0 0x00644F3A
// symbol    ?sub_500380@@YAXHHH@Z
//           The image's catalogue name is an uninformative `sub_` with no
//           prototype and no callers; the body names it, not the catalogue.
//           This tree spells it as an ordinary C++ free function, which
//           decorates to that symbol - the image's own object carries none.
// HOMED from src/unrecovered/00500380.cpp, 2026-09-04. Its `g_009497c8` is
// Rules.tech_orb_insert_sans_spc_elev (Rules + 0x90) and `g_00949740` is
// Rules.max_airdrop_sans_orb_insert (Rules + 0x08) - the pair that decides
// whether the drop range is the whole map or the airdrop limit. Its `g_00952832`
// short view is Vehs[].proto_id (+0x0A), so `rec - 0xa` / `rec - 8` are the
// Vehs[].x/.y of the record being walked and `rec - 6` its state byte.
// LEVER: has_tech() || has_project(SP_SPACE_ELEVATOR) (inline), x_dist() and
//        base_who() (inline) with int x/y locals: 0.85 -> 1.0 mnemonic. Left:
//        ebx/edi swapped between the unit's y and the x distance.
Return Value: n/a - the image discards the result; the walk is the effect.
Status: MISMATCH.
*/
void __cdecl sub_500380(int faction_id, int x, int y) {
    // Orbital insertion lets the drop reach anywhere on the map.
    int threshold;
    if (has_tech(Rules.tech_orb_insert_sans_spc_elev, faction_id)
        || has_project(SP_SPACE_ELEVATOR, faction_id)) {
        threshold = MapLongitudeBounds > MapLatitudeBounds ? MapLongitudeBounds : MapLatitudeBounds;
    } else {
        threshold = Rules.max_airdrop_sans_orb_insert;
    }
    for (int i = 0; i < GameDataStateStorage.fields_.vehicle_current_count_; i++) {
        if (Vehs[i].faction_id == faction_id && has_abil(Vehs[i].proto_id, 0x10)
            && !(Vehs[i].state & VSTATE_MADE_AIRDROP) && !Vehs[i].moves_expended) {
            int veh_x = Vehs[i].x;
            int veh_y = Vehs[i].y;
            if ((x_dist(x, veh_x) + abs(y - veh_y)) >> 1 <= threshold
                && (base_who(veh_x, veh_y) >= 0 || bit_at(veh_x, veh_y) & BIT_AIRBASE)) {
                return;
            }
        }
    }
}

/*
Purpose: Check whether the specified prototype is a planet buster.
// ORIGINAL: 0x005004F0 ?planet_buster2@@YAHH@Z 0x005004F0-0x00500518 BYTE_EXACT
// body      src/veh.h
// symbol    ?planet_buster2@@YAIH@Z
// size      40 bytes
// prototype int (__cdecl ?planet_buster2@@YAHH@Z)(int protoID)
// callers   2   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Reactor id if planet buster, otherwise 0
Status: Complete
*/
// BODY IN veh.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.

/*
Purpose: Check whether the specified unit is a planet buster.
// ORIGINAL: 0x00500520 ?planet_buster@@YAHH@Z 0x00500520-0x00500556 BYTE_EXACT
// symbol    ?planet_buster@@YAIH@Z
// size      54 bytes
// prototype int (__cdecl ?planet_buster@@YAHH@Z)(int vehID)
// callers   4   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Reactor id if planet buster, otherwise 0
Status: Complete
*/
// Historical body-location rationale retained below; the ordinary definition
// now owns this mapping, while veh.h provides only its declaration.
// BODY IN veh.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.
uint32_t __cdecl planet_buster(int veh_id) {
    return planet_buster2(Vehs[veh_id].proto_id);
}

// ORIGINAL: 0x00500560 ?shoot@@YAHHHHH@Z 0x00500560-0x00500B51
// RECOVERY_BINDING: v1 {"evidence": ["task:planet-strike-owner", "pinned:0x00500560 complete native span; real Veh, PlayerData, Base and Console declarations establish accessed storage.", "pinned:shoot6B7/7C2 live satellite inventory reloads; bonus success bypasses decrement; dword counter/subtraction wrapping retained.", "pinned:planetE16/E29 attacker-row PACT; territory callback follows radius interception; kill/draw precede intercept return; warning coordinates precede popupD0C.", "source:map.cpp bounded shared radius tables and veh.cpp TerraformDrownedBaseId; effect59 is the typed FX child observed by the native wait.", "historical seed:src/unrecovered/00500560.cpp original mapping and useful findings retained."], "reason": "Recover the complete original strike operation through genuine vehicle, faction, base, console, effect and shared map-table owners; preserve original identity/spans and seed findings without executable storage or calls."}
// TRIED: structural transcription (two proximity scan loops with early-exit gotos, extern lever on the three lea-taken-as-pointer table bases 0097d053/00946d34/00946d54) compiles and matches to instr #6 (prologue: original zeroes edi with `xor`, rebuilt uses `mov`).
int __cdecl shoot(int atk_factionID, int def_factionID, int xDest, int yDest) {
    int adjacent_flechette = 0;
    for (int i = 0; i < 25; ++i) {
        if (!adjacent_flechette) {
            const int x = wrap_map_x(RadiusOffsetX[i] + xDest);
            const int y = RadiusOffsetY[i] + yDest;
            if (on_map(x, y)) {
                const int base_id = base_at(x, y);
                if (base_id >= 0 && Bases[base_id].faction_id_current != atk_factionID
                    && has_fac(FAC_FLECHETTE_DEFENSE_SYS, base_id, 0)) {
                    adjacent_flechette = 1;
                }
            }
        }
    }
    if (!PlayersData[def_factionID].satellites_[3] && !adjacent_flechette) {
        return 0;
    }
    int bonus_hit = 0;
    const int bonus_rounds = static_cast<int>(PlayersData[def_factionID].satellites_[3]
        - PlayersData[def_factionID].sat_odp_deployed);
    for (int round = 0; round < bonus_rounds; ++round) {
        ++PlayersData[def_factionID].sat_odp_deployed;
        if (rand() % 100 >= 50) {
            bonus_hit = 1;
            break;
        }
    }
    const int shots_left = range(static_cast<int>(PlayersData[def_factionID].satellites_[3]
        - PlayersData[def_factionID].sat_odp_deployed), 0, 9999);
    if (!bonus_hit) {
        for (int i = 0; i < 25; ++i) {
            const int x = wrap_map_x(RadiusOffsetX[i] + xDest);
            const int y = RadiusOffsetY[i] + yDest;
            if (!on_map(x, y)) {
                continue;
            }
            const int base_id = base_at(x, y);
            if (base_id >= 0 && Bases[base_id].faction_id_current != atk_factionID
                && has_fac(FAC_FLECHETTE_DEFENSE_SYS, base_id, 0)
                && rand() % 100 < 50) {
                parse_says(0, Bases[base_id].name_string, -1, -1);
                if (def_factionID == ConsoleGlobal.faction_id_) {
                    parse_says(1, Players[atk_factionID].adj_name_faction, -1, -1);
                    popp(ScriptPopupFile, "WESHOTIT2", 0, "space_sm.pcx", 0);
                } else if (atk_factionID == ConsoleGlobal.faction_id_) {
                    parse_says(1, Players[Bases[base_id].faction_id_current].adj_name_faction,
                               -1, -1);
                    popp(ScriptPopupFile, "THEYSHOTIT2", 0, "space_sm.pcx", 0);
                }
                return 1;
            }
        }
        // The live inventory is reread after the Flechette scan.
        if (!PlayersData[def_factionID].satellites_[3]) {
            goto failed;
        }
        if (!IsMultiplayerNet
            && (GameDataStateStorage.fields_.lock_masks_.enable_mask_ & (1 << def_factionID))) {
            if (def_factionID == ConsoleGlobal.faction_id_) {
                parse_num(0, PlayersData[def_factionID].satellites_[3]);
                parse_says(0, Bases[base_find(xDest, yDest)].name_string, -1, -1);
                parse_says(1, Players[atk_factionID].adj_name_faction, -1, -1);
                if (popp(ScriptPopupFile, "BUSTERSAC", 0, "space_sm.pcx", 0)) {
                    goto failed;
                }
            } else if (atk_factionID == ConsoleGlobal.faction_id_) {
                GenderDefault = Players[def_factionID].noun_gender;
                PluralityDefault = Players[def_factionID].is_noun_plural;
                parse_says(0, Players[def_factionID].noun_faction, -1, -1);
                popp(ScriptPopupFile, "THEYMUSTSAC", 0, "space_sm.pcx", 0);
            }
        } else if (def_factionID == ConsoleGlobal.faction_id_) {
            popp(ScriptPopupFile, "WEMUSTSAC", 0, "space_sm.pcx", 0);
        } else if (atk_factionID == ConsoleGlobal.faction_id_) {
            GenderDefault = Players[def_factionID].noun_gender;
            PluralityDefault = Players[def_factionID].is_noun_plural;
            parse_says(0, Players[def_factionID].noun_faction, -1, -1);
            popp(ScriptPopupFile, "THEYMUSTSAC", 0, "space_sm.pcx", 0);
        }
        --PlayersData[def_factionID].satellites_[3];
        if (static_cast<int>(PlayersData[def_factionID].satellites_[3]
                - PlayersData[def_factionID].sat_odp_deployed) < 0) {
            PlayersData[def_factionID].sat_odp_deployed = PlayersData[def_factionID].satellites_[3];
        }
    }
    {
        int base_id = base_find(xDest, yDest, def_factionID);
        if (base_id < 0) {
            base_id = base_find(xDest, yDest);
        }
        parse_says(0, Bases[base_id].name_string, -1, -1);
        parse_num(0, PlayersData[def_factionID].satellites_[3]);
        parse_num(1, shots_left);
        if (def_factionID == ConsoleGlobal.faction_id_) {
            parse_says(1, Players[atk_factionID].adj_name_faction, -1, -1);
            popp(ScriptPopupFile, "WESHOTIT", 0, "space_sm.pcx", 0);
        } else if (atk_factionID == ConsoleGlobal.faction_id_) {
            parse_says(1, Players[def_factionID].adj_name_faction, -1, -1);
            popp(ScriptPopupFile, "THEYSHOTIT", 0, "space_sm.pcx", 0);
        }
    }
    return 1;

failed:
    if (adjacent_flechette) {
        if (def_factionID == ConsoleGlobal.faction_id_) {
            parse_says(1, Players[atk_factionID].adj_name_faction, -1, -1);
            popp(ScriptPopupFile, "WEFAILEDFLECHETTE", 0, "space_sm.pcx", 0);
        } else if (atk_factionID == ConsoleGlobal.faction_id_) {
            parse_says(1, Players[def_factionID].adj_name_faction, -1, -1);
            popp(ScriptPopupFile, "THEYFAILEDFLECHETTE", 0, "space_sm.pcx", 0);
        }
    }
    return 0;
}

// Inlined: the doubled vector_dist halved once.
static inline int patrol_dist(int x_a, int y_a, int x_b, int y_b) {
    return (x_dist(x_a, x_b) + abs(y_a - y_b)) >> 1;
}

// ORIGINAL: 0x00500B60 ?planet_busting@@YAHHHH@Z 0x00500B60-0x005010BB
// RECOVERY_BINDING: v1 {"evidence": ["task:planet-strike-owner", "pinned:0x00500B60 complete native span; real Veh, PlayerData, Base and Console declarations establish accessed storage.", "pinned:shoot6B7/7C2 live satellite inventory reloads; bonus success bypasses decrement; dword counter/subtraction wrapping retained.", "pinned:planetE16/E29 attacker-row PACT; territory callback follows radius interception; kill/draw precede intercept return; warning coordinates precede popupD0C.", "source:map.cpp bounded shared radius tables and veh.cpp TerraformDrownedBaseId; effect59 is the typed FX child observed by the native wait.", "historical seed:src/unrecovered/00500b60.cpp original mapping and useful findings retained."], "reason": "Recover the complete original strike operation through genuine vehicle, faction, base, console, effect and shared map-table owners; preserve original identity/spans and seed findings without executable storage or calls."}
int __cdecl planet_busting(int veh_id, int x, int y) {
    int offset;
    int intercepted = 0;
    const int faction = Vehs[veh_id].faction_id;
    ConsoleGlobal.focus(x, y, faction);

    const int proto_id = Vehs[veh_id].proto_id;
    const int bust_level =
        VehPrototypes[proto_id].plan == PLAN_PLANET_BUSTER
            ? VehPrototypes[proto_id].reactor_id : 0;
    // Native indexes the parallel pattern-count table at 0x66F8C4. Its
    // values are the radius cardinalities 1,9,25,... used by RadiusOffsetX/Y.
    const int weapon_count = RadiusRange[bust_level];

    const int nearest_base = base_find(x, y);
    if (faction != ConsoleGlobal.faction_id_ && nearest_base >= 0) {
        GenderDefault = Players[faction].is_leader_female;
        PluralityDefault = 0;
        parse_says(0, Players[faction].title_leader, -1, -1);
        GenderDefault = Players[faction].is_leader_female;
        PluralityDefault = 0;
        parse_says(1, Players[faction].name_leader, -1, -1);
        GenderDefault = Players[faction].noun_gender;
        PluralityDefault = Players[faction].is_noun_plural;
        parse_says(2, Players[faction].noun_faction, -1, -1);
        parse_says(3, Bases[nearest_base].name_string, -1, -1);

        const char *const sound =
            (patrol_dist(Bases[nearest_base].x, Bases[nearest_base].y, x, y) <= bust_level)
            ? "baseobl_sm.pcx" : "astp_sm.pcx";
        popp(ScriptPopupFile, "PLANETBUSTER", 0, sound, 0);
    }

    int notified[MaxPlayerNum];
    memset(notified, 0, sizeof(notified));
    for (offset = 0; offset < weapon_count; ++offset) {
        const int blast_x = wrap_map_x(RadiusOffsetX[offset] + x);
        const int blast_y = RadiusOffsetY[offset] + y;
        if (!on_map(blast_x, blast_y)) continue;

        const int base_id = base_at(blast_x, blast_y);
        if (base_id >= 0) {
            const int defender = Bases[base_id].faction_id_current;
            if (faction != defender && !notified[defender]) {
                intercepted = shoot(faction, defender, blast_x, blast_y);
                if (intercepted != 0) break;
                notified[defender] = 1;
            }
            continue;
        }

        const int stack_id = stack_fix(veh_at(blast_x, blast_y));
        if (stack_id < 0) continue;
        const int defender = Vehs[stack_id].faction_id;
        if (faction != defender &&
            (PlayersData[faction].diplo_treaties[defender] & 1) == 0 &&
            !notified[defender]) {
            intercepted = shoot(faction, defender, x, y);
            if (intercepted != 0) break;
            notified[defender] = 1;
        }
    }

    const int territory_owner = whose_territory(faction, x, y, 0, 0);
    if (territory_owner >= 0 && territory_owner != faction &&
        !notified[territory_owner]) {
        intercepted = shoot(faction, territory_owner, x, y);
    }

    kill(veh_id);
    draw_radius(x, y, 0, 2);
    if (intercepted != 0) return intercepted;

    VehBattleUnkTgl = 1;
    int blame_vehicle = -1;
    int blame_base = -1;
    for (int lower = 0; lower < bust_level; ++lower) {
        world_lower_alt(x, y);
    }
    world_climate();
    TerraformDrownedBaseId = -1;
    g_FX.play(0x3b);
    BoomAnimationLevel = bust_level;
    boom(x, y, 0x20);

    for (offset = 0; offset < weapon_count; ++offset) {
        const int blast_x = wrap_map_x(RadiusOffsetX[offset] + x);
        const int blast_y = RadiusOffsetY[offset] + y;
        if (!on_map(blast_x, blast_y)) continue;

        const int base_id = base_at(blast_x, blast_y);
        if (base_id >= 0) {
            if (blame_base < 0) {
                const int defender = Bases[base_id].faction_id_current;
                if (defender != faction) blame_base = defender;
            }
            base_kill(base_id);
        }
        const int stack_id = stack_fix(veh_at(blast_x, blast_y));
        if (stack_id >= 0) {
            const int defender = Vehs[stack_id].faction_id;
            if (blame_vehicle < 0 && defender != faction) {
                blame_vehicle = defender;
            }
            stack_kill(stack_id);
        }
    }

    VehBattleUnkTgl = 0;
    if (blame_base < 0) blame_base = blame_vehicle;
    draw_map(0);
    boom(x, y, 0x40);
    while (g_FX.effect(59).is_playing() != 0) {
        do_task();
    }
    g_FX.play(0x3c);
    boom(x, y, 0x11);
    boom(x, y, 0x12);
    major_atrocity(faction, blame_base);
    // No final return (C4715), as in the image: EAX is whatever
    // major_atrocity left; its observed caller discards it.
}

/*
Purpose: Calculate how defensive the specified tile is to the defending unit based on the terrain.
// ORIGINAL: 0x005010C0 ?defense_value@@YAHHHHHH@Z 0x005010C0-0x0050134F BYTE_EXACT
// LEVER: MORE (3 calls vs image's 0) - `label_get()` folds its array read
//        into a REAL `StringTable.get()` call, but the image never calls
//        anything here: `VehBattleDisplayTerrain` is read back at
//        battle_compute (0x501350-ish, line ~959) as `get(int(...))`, so
//        this site only needs to stash the raw id, not resolve it. Replaced
//        all three `label_get(n)` with the array read it wraps,
//        `*((LPSTR *)Labels.strings_ptr + n)`. Call count now agrees (0);
//        best flag set's similarity 0.391 -> 0.753 (/O2 /GR- /GX). Still
//        MISMATCH: the image writes an unidentified global (0x90f550, from
//        [0x9b90f8]+0x16c or +0x548 depending on branch) that this tree's
//        source has no equivalent for - not a call-count question, left open.
// LEVER: exact with separate early returns, bit_at/map_loc re-read at each
//   test (no `bit` local), the "Rocky" label stored before the rocky test,
//   and `defense` assigned first with `is_rocky` copied from it.
// size      655 bytes
// prototype int (__cdecl ?defense_value@@YAHHHHHH@Z)(int factionID, int xCoord, int yCoord, int vehIDDef, int vehIDAtk)
// callers   2   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: Defense terrain value
Status: Complete
*/
int __cdecl defense_value(int faction_id, int x, int y, int veh_id_def,
                          int veh_id_atk) {
    if (is_ocean(x, y)) {
        return 2;
    }
    if (base_who(x, y) >= 0) {
        return 2;
    }
    if (bit_at(x, y) & BIT_FUNGUS && (map_loc(x, y)->climate & ~0x1F) >= ALT_BIT_OCEAN_SHELF && veh_id_atk >= 0
        && (!Vehs[veh_id_atk].faction_id
            || has_project(SP_PHOLUS_MUTAGEN, Vehs[veh_id_atk].faction_id)
            || (get_offense_rating(veh_id_atk) < 0
                && Vehs[veh_id_atk].proto_id < MaxVehProtoFactionNum))) {
                return 2;
    }
    VehBattleDisplayTerrain = *(Labels.strings_ptr + 91); // "Rocky"
    int defense = (map_loc(x, y)->val3 & ~0x3F) > TERRAIN_BIT_ROLLING;
    int is_rocky = defense;
    if (bit_at(x, y) & BIT_FUNGUS && (map_loc(x, y)->climate & ~0x1F) >= ALT_BIT_OCEAN_SHELF && !defense
        && get_triad(veh_id_def) != TRIAD_AIR) {
        if (veh_id_atk >= 0 && ((get_offense_rating(veh_id_atk) < 0
                && Vehs[veh_id_atk].proto_id < MaxVehProtoFactionNum)
            || has_project(SP_PHOLUS_MUTAGEN, Vehs[veh_id_atk].faction_id))) {
            defense = is_rocky;
        } else {
            defense = 1;
            if (has_project(SP_PHOLUS_MUTAGEN, faction_id)
                || (Vehs[veh_id_def].proto_id < MaxVehProtoFactionNum
                    && get_offense_rating(veh_id_def) < 0)) {
                defense = 2;
            }
            VehBattleDisplayTerrain = *(Labels.strings_ptr + 338); // "Fungus"
        }
    }
    if (bit_at(x, y) & BIT_FOREST && !defense && (veh_id_atk < 0 || get_triad(veh_id_atk) == TRIAD_LAND)) {
        VehBattleDisplayTerrain = *(Labels.strings_ptr + 291); // "Forest"
        defense = 1;
    }
    return defense + 2;
}

/*
Purpose: Calculate the lifecycle (morale) of the specified native life unit.
// ORIGINAL: 0x00501350 ?morale_alien@@YAHHH@Z 0x00501350-0x005014F1 BYTE_EXACT
// TRIED: still MISMATCH (~0.72 similar, up from ~0.46) - most of the remaining divergence is register allocation the compiler picked differently around the atrocities/veh-flags block and the two `range()` call sites, which every flag set tried left in place.
// TRIED: source had the Fungal Tower special case as the OUTER `if`, guarded by `veh_id >= 0`, with the turn-ladder as its `else` - the image runs the turn-ladder/atrocities/veh-flags computation FIRST, UNCONDITIONALLY, then reads `Vehs[veh_id].x/.y/.proto_id` with NO veh_id guard (an out-of-bounds `Vehs[-1]` read - reproduced deliberately, see the BUG comment at the site) and only THEN branches on proto_id, overwriting `morale` if it is the Fungal Tower. Each branch ends in its OWN `return range(morale, 0, 6);` (not a shared tail after the `if`) - `int x`/`int y` locals (not `int16_t`) match the image's `movsx` at the point of assignment.
// LEVER: (0.79 -> 0.863) the tower scan walks the eight adjacent offsets
//   (i = 1..8, constant bounds in the image), tests altitude as `& ~0x1F`
//   on the tile, and sets morale = count - 2 for the single shared range().
//   Left: the image keeps a zero in esi for the sign tests and the count.
// symbol    ?morale_alien@@YAHHH@Z
// size      417 bytes
// prototype int (__cdecl ?morale_alien@@YAHHH@Z)(int vehID, int factionIDvsNative)
// callers   9   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: Lifecycle (morale) value
Status: Complete
*/
int __cdecl morale_alien(int veh_id, int faction_id_vs_native) {
    int morale;
    if (GameDataStateStorage.fields_.turn_current_num_ < 45) {
        morale = 0;
    } else if (GameDataStateStorage.fields_.turn_current_num_ < 90) {
        morale = 1;
    } else if (GameDataStateStorage.fields_.turn_current_num_ < 170) {
        morale = 2;
    } else if (GameDataStateStorage.fields_.turn_current_num_ < 250) {
        morale = 3;
    } else if (GameDataStateStorage.fields_.turn_current_num_ < 330) {
        morale = 4;
    } else { // 330+
        morale = 6;
    }
    // Two separate guards: one merged block does not match.
    if (faction_id_vs_native > 0) {
        morale += (PlayersData[faction_id_vs_native].major_atrocities != 0);
    }
    if (faction_id_vs_native > 0) {
        morale += (TectonicDetonationCount[faction_id_vs_native] != 0);
    }
    if (veh_id >= 0) {
        if (Vehs[veh_id].state & VSTATE_MONOLITH_UPGRADED) {
            morale++;
        }
        if (Vehs[veh_id].proto_id == BSC_LOCUSTS_OF_CHIRON) {
            morale++;
        }
        morale += (Vehs[veh_id].flags >> 3) & 3; // 0x8|0x10 > +1, +2, or +3; TODO: id unk flags
    }
    // BUG IN THE ORIGINAL: Vehs[veh_id].x/.y/.proto_id are read here with no
    // veh_id >= 0 guard, unlike the block just above - Vehs[-1] is read out
    // of bounds whenever the caller passes a negative veh_id. Reproduced
    // deliberately: the image has no such check either.
    int fungus_count = 0;
    int x = Vehs[veh_id].x;
    int y = Vehs[veh_id].y;
    // Fungal Tower specific code: overrides the morale computed above entirely.
    if (Vehs[veh_id].proto_id == BSC_FUNGAL_TOWER) {
        // similar to is_coast() > except with fungus check + Ocean Shelf included
        for (int i = 1; i < 9; i++) {
            int x_radius = wrap_map_x(x + RadiusOffsetX[i]);
            int y_radius = y + RadiusOffsetY[i];
            if (on_map(x_radius, y_radius) && bit_at(x_radius, y_radius) & BIT_FUNGUS
                && (map_loc(x_radius, y_radius)->climate & ~0x1F) >= ALT_BIT_OCEAN_SHELF) {
                fungus_count++;
            }
        }
        morale = range(fungus_count - 2, 0, 6);
    }
    return range(morale, 0, 6);
}

/*
Purpose: Calculate the psi combat factor for an attacking or defending unit.
// ORIGINAL: 0x00501500 ?psi_factor@@YAHHHHH@Z 0x00501500-0x005015AB BYTE_EXACT
// LEVER: each `if (has_project(...)) factor += factor/2;`/`if (is_fungal_twr) ...` stores its condition to a shared `add_half` local BEFORE the `if`, even the one used only once - the image tail-merges the attack and defense arms into one shared "if(add_half) factor+=factor/2; return factor;" block, and only materialising every condition into the same local (not just the ones that need to survive a branch) lets the compiler recognise the shared tail.
// size      171 bytes
// prototype int (__cdecl ?psi_factor@@YAHHHHH@Z)(int combatRatio, int factionID, BOOL isAttack, BOOL isFungalTower)
// callers   7   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Psi factor
Status: Complete
*/
int __cdecl psi_factor(int combat_ratio, int faction_id, BOOL is_attack, BOOL is_fungal_twr) {
    int rule_psi = Players[faction_id].rule_psi;
    int factor = rule_psi ? ((rule_psi + 100) * combat_ratio) / 100 : combat_ratio;
    BOOL add_half;
    if (is_attack) {
        add_half = has_project(SP_DREAM_TWISTER, faction_id); // Psi Attack +50%
    } else {
        add_half = has_project(SP_NEURAL_AMPLIFIER, faction_id);
        if (add_half) {
            factor += factor / 2; // Psi Defense +50%s
        }
        add_half = is_fungal_twr; // SMACX only: likely +50% Fungal Tower defense bonus
    }
    if (add_half) {
        factor += factor / 2;
    }
    return factor;
}

/*
Purpose: Get the basic offense value for an attacking unit with an optional defender unit parameter.
// ORIGINAL: 0x005015B0 ?get_basic_offense@@YAHHHHHH@Z 0x005015B0-0x00501940
// LEVER: crèche/brood-pit was `if/else if` with the SAME body on both arms;
//        the disassembly's 3rd bitmask() call proves the crèche-true arm
//        also runs a nested, BARE `morale++` brood-pit check (no range()/cap
//        loop) - only the `else if` arm has the full adjustment.
// LEVER: `bitmask_call` (general.h forwarder, base.cpp's own lever) on the
//        NESTED brood-pit check and the `else if` brood-pit check - both real
//        calls in the image - while the outer crèche check stays plain
//        `has_fac_built` (still auto-inlines there). All three real was
//        MORE (10 vs 9); all three inline was the old FEWER (7 vs 9); this
//        combination is the one that reads 0 disagree AND is the best-
//        similarity combination of the four tried. Best flag set's exact
//        count 5/307 (0.432 similar) -> 8/307 (0.494 similar).
// size      912 bytes
// prototype int (__cdecl ?get_basic_offense@@YAHHHHHH@Z)(int vehIDAtk, int vehIDDef, uint32_t psiCombatType, BOOL isBombardment, int isUnkTgl)
// callers   2   call targets   6
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004E3A50 0x00501350 0x0050BA00 0x005BF1F0 0x005C0E40 0x005C1150
Return Value: Basic offense
Status: Complete
*/
int __cdecl get_basic_offense(int veh_id_atk, int veh_id_def, int psi_combat_type,
                              BOOL is_bombard, BOOL unk_tgl) { // art/duel related? Is off of def?
    Veh *veh_atk = &Vehs[veh_id_atk];
    int faction_id_atk = veh_atk->faction_id;
    int morale = faction_id_atk ? morale_veh(veh_id_atk, true, 0)
        : morale_alien(veh_id_atk, veh_id_def >= 0 ? Vehs[veh_id_def].faction_id : -1);
    int base_id_atk = base_at(veh_atk->x, veh_atk->y);
    if (base_id_atk >= 0) {
        // The brood pit test after a creche only adds one; the morale
        // adjustment below it belongs to the creche-false arm.
        if (has_fac(FAC_CHILDREN_CRECHE, base_id_atk, 0)) {
            morale++;
            int morale_active = range(PlayersData[faction_id_atk].soc_effect_active.values_[SocialEffect::MORALE], -4, 4);
            if (morale_active <= -2) {
                morale_active++;
            }
            morale -= morale_active;
            if (has_fac(FAC_BROOD_PIT, base_id_atk, 0) && veh_atk->proto_id < MaxVehProtoFactionNum
                && (get_proto_offense_rating(veh_atk->proto_id) < 0
                    || veh_atk->proto_id == BSC_SPORE_LAUNCHER)) {
                morale++;
            }
        } else if (has_fac(FAC_BROOD_PIT, base_id_atk, 0) && veh_atk->proto_id < MaxVehProtoFactionNum
            && (get_proto_offense_rating(veh_atk->proto_id) < 0
                || veh_atk->proto_id == BSC_SPORE_LAUNCHER)) {
            morale++;
            int morale_active = range(PlayersData[faction_id_atk].soc_effect_active.values_[SocialEffect::MORALE], -4, 4);
            if (morale_active <= -2) {
                morale_active++;
            }
            morale -= morale_active;
        }
        if (unk_tgl) {
            int morale_pending = PlayersData[faction_id_atk].soc_effect_pending.values_[SocialEffect::MORALE];
            if (morale_pending >= 2 && morale_pending <= 3) {
                morale++;
            }
            if (veh_id_def >= 0) {
                if (!Vehs[veh_id_def].faction_id) {
                    morale++;
                } else if ((veh_atk->proto_id >= MaxVehProtoFactionNum
                        || (get_proto_offense_rating(veh_atk->proto_id) >= 0
                            && veh_atk->proto_id != BSC_SPORE_LAUNCHER))
                    && !has_abil(veh_atk->proto_id, ABL_DISSOCIATIVE_WAVE)
                    && has_abil(Vehs[veh_id_def].proto_id, ABL_SOPORIFIC_GAS)) {
                    morale -= 2;
                }
            }
        }
    }
    if (unk_tgl) {
        morale = range(morale, 1, 6);
    }
    int morale_factor = morale + 6;
    int offense = offense_proto(veh_atk->proto_id, veh_id_def, is_bombard);
    if (psi_combat_type) {
        offense = psi_factor(offense, faction_id_atk, true, false);
    }
    VehBasicBattleMorale[unk_tgl != 0] = morale;
    return offense * morale_factor * 4;
}

/*
Purpose: Get the basic defense value for a defending unit with an optional attacker unit parameter.
// ORIGINAL: 0x00501940 ?get_basic_defense@@YAHHHHH@Z 0x00501940-0x00501D26
// RECOVERY_BINDING: v1 {"evidence": ["pinned:501B3C-501B5D increments only for pending morale2..3; no Headquarters check before501B5E attacker path", "pinned:501C4C-501C79 defender plan11 path reads attacker prototype/plan without new negative-ID guard; earlier501C03 join does not protect it"], "reason": "Restore original defense morale and information-warfare predicate. The manual Headquarters bonus and added attacker-index short-circuit are not present in the pinned body. Preserve existing earlier attacker guards; invalid-index effects, full matching and runtime remain unverified."}
// LEVER: same asymmetric crèche/brood-pit fix as get_basic_offense above -
//        see its comment for the disassembly evidence.
// LEVER: same `bitmask_call` split as get_basic_offense - nested and
//        `else if` brood-pit checks real, outer crèche check left plain
//        `has_fac_built`. Call count now agrees (was FEWER 7 vs 9); best
//        flag set's exact count 1/338 (0.149 similar) -> 32/338 (0.504
//        similar).
// size      998 bytes
// prototype int (__cdecl ?get_basic_defense@@YAHHHHH@Z)(int vehIDDef, int vehIDAtk, uint32_t psiCombatType, BOOL isBombardment)
// callers   1   call targets   6
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004E3A50 0x00501350 0x0050BA00 0x005BF1F0 0x005C0E40 0x005C1290
Return Value: Basic defense
Status: Complete
*/
int __cdecl get_basic_defense(int veh_id_def, int veh_id_atk, int psi_combat_type,
                              BOOL is_bombard) {
    int faction_id_def = Vehs[veh_id_def].faction_id;
    int morale = faction_id_def ? morale_veh(veh_id_def, true, 0)
        : morale_alien(veh_id_def, veh_id_atk >= 0 ? Vehs[veh_id_atk].faction_id : -1);
    int base_id_def = base_at(Vehs[veh_id_def].x, Vehs[veh_id_def].y);
    if (base_id_def >= 0) {
        // Asymmetric with the `else if` below, as in get_basic_offense: the
        // creche-true fallthrough only adds one for a brood pit.
        if (has_fac(FAC_CHILDREN_CRECHE, base_id_def, 0)) {
            morale++;
            int morale_active = range(PlayersData[faction_id_def].soc_effect_active.values_[SocialEffect::MORALE], -4, 0);
            if (morale_active <= -2) {
                morale_active++;
            }
            morale -= morale_active;
            if (has_fac(FAC_BROOD_PIT, base_id_def, 0) && Vehs[veh_id_def].proto_id < MaxVehProtoFactionNum
                && (get_proto_offense_rating(Vehs[veh_id_def].proto_id) < 0
                    || Vehs[veh_id_def].proto_id == BSC_SPORE_LAUNCHER)) {
                morale++;
            }
        } else if (has_fac(FAC_BROOD_PIT, base_id_def, 0) && Vehs[veh_id_def].proto_id < MaxVehProtoFactionNum
            && (get_proto_offense_rating(Vehs[veh_id_def].proto_id) < 0
                || Vehs[veh_id_def].proto_id == BSC_SPORE_LAUNCHER)) {
            morale++;
            int morale_active = range(PlayersData[faction_id_def].soc_effect_active.values_[SocialEffect::MORALE], -4, 4);
            if (morale_active <= -2) {
                morale_active++;
            }
            morale -= morale_active;
        }
        int morale_pending = PlayersData[faction_id_def].soc_effect_pending.values_[SocialEffect::MORALE];
        if (morale_pending >= 2 && morale_pending <= 3) {
            morale++;
        }
        if (veh_id_atk >= 0 && !Vehs[veh_id_atk].faction_id) {
            morale++;
        }
    }
    if (veh_id_atk >= 0 && Vehs[veh_id_atk].faction_id
        && (Vehs[veh_id_def].proto_id >= MaxVehProtoFactionNum
            || (get_proto_offense_rating(Vehs[veh_id_def].proto_id) >= 0
                && Vehs[veh_id_def].proto_id != BSC_SPORE_LAUNCHER))
        && !has_abil(Vehs[veh_id_def].proto_id, ABL_DISSOCIATIVE_WAVE)
        && has_abil(Vehs[veh_id_atk].proto_id, ABL_SOPORIFIC_GAS)) {
        morale -= 2;
    }
    morale = range(morale, 1, 6);
    VehBasicBattleMorale[1] = morale;
    morale += 6;
    int proto_id_def = Vehs[veh_id_def].proto_id;
    if (VehPrototypes[proto_id_def].plan == PLAN_ALIEN_ARTIFACT) {
        return 1;
    }
    if (VehPrototypes[proto_id_def].plan == PLAN_INFO_WARFARE && get_proto_defense_rating(proto_id_def) == 1
        // The original reads the attacker prototype here without an ID guard.
        && VehPrototypes[Vehs[veh_id_atk].proto_id].plan != PLAN_INFO_WARFARE) {
        return 1;
    }
    int defense = armor_proto(proto_id_def, veh_id_atk, is_bombard);
    if (psi_combat_type) {
        defense = psi_factor(defense, faction_id_def, false, Vehs[veh_id_def].proto_id == BSC_FUNGAL_TOWER);
    }
    return defense * morale;
}

/*
Purpose: Initialize or reset the battle related global variables.
// ORIGINAL: 0x00501D30 ?battle_init@@YAXXZ 0x00501D30-0x00501D47 BYTE_EXACT
// body      src/veh.h
// LEVER: the real-object conversion SURVIVED the claim. 2026-08-29
//        VehBattleModCount/VehBattleUnkTgl became real arrays (veh.h);
//        measured 6/6, 16 bytes discounted as relocations.
// size      23 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
// BODY IN veh.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.

/*
Purpose: Add combat battle modifier for type (offense, defense).
// ORIGINAL: 0x00501D50 ?add_bat@@YAXHHPAD@Z 0x00501D50-0x00501D9A BYTE_EXACT
// LEVER: BYTE_EXACT 25/25, from 2/25, on four measured changes at once. (1) SPLIT THE CHAIN: `if (modifier && offset < 4)` shares one epilogue, and the image tests `modifier` FIRST and falls through to `pop ebp; ret` before it even loads the count - two guard clauses, in that order. Reading the count first instead scores 5/25. (2) `int offset`, not `uint32_t`: the image's `cmp eax, 4` is followed by `jge`, and unsigned gives `jae`. (3) `VehBattleModCount[type] = offset + 1` BEFORE the two stores, where this tree had the increment last - the image's `lea esi, [eax + 1]; mov [ecx*4 + 0x915614], esi` comes first. (4) ONE `int index = type * 4 + offset` used by both the modifier store and the display address: the image keeps that value in eax and addresses `[eax*4 + 0x9155f0]` and `(eax + eax*4) << 4`, which `type * 80 + offset * 20` spelled out twice does not reproduce (9/25 -> 13/25).
// LEVER: the real-object conversion SURVIVED the claim. 2026-08-29 the three
//        battle bindings became real arrays (VehBattleModCount[2],
//        VehBattleModifier[8], VehBattleDisplay[160], veh.h); every folded
//        immediate here is now a discounted relocation - measured 25/25,
//        20 bytes discounted as relocations - and the literal-initialised
//        `display` local is gone: as an ARRAY, the name is an address, so
//        `&VehBattleDisplay[index * 20]` folds the base exactly as the old
//        literal did.
// LEVER: strcpy, not strcpy_s. The image's only callee here is 0x00645460, plain strcpy; the safe-CRT form does not exist in it.
// symbol    ?add_bat@@YAXHHPBD@Z
// size      74 bytes
// prototype
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00645460
Return Value: n/a
Status: Complete
*/
void __cdecl add_bat(int type, int modifier, LPCSTR display_str) {
    // TODO: Revise global offsets once all references are decompiled.
    if (!modifier) {
        return;
    }
    int offset = VehBattleModCount[type];
    if (offset >= 4) {
        return;
    }
    VehBattleModCount[type] = offset + 1;
    int index = type * 4 + offset;
    VehBattleModifier[index] = modifier;
    // Native strcpy writes the row itself; no pointer is stored or loaded.
    strcpy(VehBattleDisplay[index], display_str);
}

/*
Purpose: Calculate the battle outcome between two units.
// ORIGINAL: 0x00501DA0 ?battle_compute@@YAXHHPAHPAHH@Z 0x00501DA0-0x005044C4
// symbol    ?battle_compute@@YAXHHPAH0H@Z
// LEVER: THE SEH FRAME WAS A std::string. The terrain modifier was built with
//        `std::string terrain_modifier = label_get(331); ... .c_str()`, and
//        basic_string's destructor made VC6 emit an unwind frame the image
//        does not pay for: the divergence started at instruction 2, this tree
//        emitting `push -1 / push <handler> / mov eax, fs:[0] / push eax`
//        where the image has `sub esp, 0x2c / mov eax, 8`. Rebuilt as
//        strcpy/strcat onto StringTemp - the image's own callee list here is
//        strcpy (0x00645460) and strcat (0x00645470), and add_bat copies what
//        it is handed - and the frame is gone: the prologue now agrees and the
//        compiled body drops from 2,290 instructions to 2,160.
// TRIED: BYTE_EXACT, on budget rather than on a wall. 2,811 image
//        instructions against 2,160 compiled - 651 SHORT, meaning whole
//        blocks of the image are inlined work this tree still calls out to or
//        does not do - at 0.037 similarity, the best of all ten flag sets, and
//        the metric did not move when the SEH frame came out (0.039 before).
//        A body this size is not reachable by source-form search in one pass;
//        it wants the callee-by-callee treatment say_stats got. What IS
//        pinned: the prologue divergence is now only `sub esp, 0x30` against
//        the image's `sub esp, 0x2c`, one dword of extra locals, and
//        call_diff AGREES on call count, so nothing is being over- or
//        under-called.
// size      10020 bytes
// prototype void (__cdecl ?battle_compute@@YAXHHPAHPAHH@Z)(int vehIDAtk, int vehIDDef, int* offenseOutput, int* defenseOutput, int combatType)
// callers   2   call targets   15
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004E3A50 0x004E3B80 0x004E3EF0 0x005010C0 0x005015B0 0x00501940 0x0050BA00 0x00581170 0x005B9F20 0x005BF010 0x005BF1F0 0x005C13B0 0x006169A0 0x00645460 0x00645470
Return Value: n/a
Status: Complete - testing
*/
void __cdecl battle_compute(int veh_id_atk, int veh_id_def, int *offense_out, int *defense_out,
                            int combat_type) {
    int offense = 8;
    int defense = 8;
    int proto_id_atk = -1;
    int proto_id_def = -1;
    uint32_t faction_id_atk = 0;
    uint32_t faction_id_def = 0;
    if (veh_id_atk >= 0) {
        proto_id_atk = Vehs[veh_id_atk].proto_id;
        faction_id_atk = Vehs[veh_id_atk].faction_id;
    }
    if (veh_id_def >= 0) {
        proto_id_def = Vehs[veh_id_def].proto_id;
        faction_id_def = Vehs[veh_id_def].faction_id;
    }
    battle_init();
    BOOL is_artillery = combat_type & 1;
    // Native non-artillery selection reads this row even for a negative attacker.
    BOOL is_bombard = (is_artillery
        || is_proto_missile(Vehs[veh_id_atk].proto_id)) ? true : false;
    uint32_t psi_combat_type = 0;
    if (!is_bombard || Vehs[veh_id_atk].proto_id == BSC_SPORE_LAUNCHER
        || Vehs[veh_id_def].proto_id == BSC_SPORE_LAUNCHER) {
        if (veh_id_atk >= 0 && get_proto_offense_rating(proto_id_atk) < 0) {
            psi_combat_type = 1; // PSI attacker
        }
        if (veh_id_def >= 0 && get_proto_defense_rating(proto_id_def) < 0) {
            psi_combat_type |= 2; // PSI defender
        }
        // NOTE: PSI bonuses below are only for display purposes, actual calculation is done in
        //       combat morale functions.
        if (psi_combat_type) {
            // Calculate PSI attack bonus
            faction_id_atk = Vehs[veh_id_atk].faction_id;
            int rule_psi_atk = Players[faction_id_atk].rule_psi;
            if (rule_psi_atk) {
                add_bat(0, rule_psi_atk, label_get(342)); // "Gaian Psi Bonus"
            }
            if (has_project(SP_DREAM_TWISTER, faction_id_atk)) {
                add_bat(0, 50, label_get(343)); // "Dream Twist"
            }
        }
        // Native reads the defender row before testing the PSI display gate,
        // including a negative ID. Do not substitute faction zero here.
        faction_id_def = Vehs[veh_id_def].faction_id;
        if (psi_combat_type) {
            int rule_psi_def = Players[faction_id_def].rule_psi;
            if (rule_psi_def) {
                add_bat(1, rule_psi_def, label_get(342)); // "Gaian Psi Bonus"
            }
            if (has_project(SP_NEURAL_AMPLIFIER, faction_id_def)) {
                add_bat(1, 50, label_get(344)); // "Neural Amp"
            }
            if (veh_id_def >= 0 && proto_id_def == BSC_FUNGAL_TOWER) {
                add_bat(1, 50, VehPrototypes[BSC_FUNGAL_TOWER].veh_name); // "Fungal Tower"
            }
        }
        if (veh_id_atk >= 0) {
            offense = get_basic_offense(veh_id_atk, veh_id_def, psi_combat_type, is_bombard, false);
            if (VehPrototypes[proto_id_atk].plan != PLAN_INFO_WARFARE) {
                if (veh_id_def >= 0 && !combat_type) { // checking if isn't wpn vs wpn ; air toggle
                    int x_def = Vehs[veh_id_def].x;
                    int y_def = Vehs[veh_id_def].y;
                    if (bit_at(x_def, y_def) & BIT_FUNGUS
                        && altitude_at(x_def, y_def) >= ALT_BIT_OCEAN_SHELF
                        && ((proto_id_atk < MaxVehProtoFactionNum
                            && get_proto_offense_rating(proto_id_atk) < 0)
                            || has_project(SP_PHOLUS_MUTAGEN, faction_id_atk))
                        && get_proto_offense_rating(proto_id_def) >= 0) {
                        offense += offense / 2;
                        add_bat(0, 50, label_get(338)); // "Fungus"
                    }
                    if (psi_combat_type & 2
                        && (VehPrototypes[proto_id_atk].weapon_id == WPN_RESONANCE_LASER
                            || VehPrototypes[proto_id_atk].weapon_id == WPN_RESONANCE_BOLT)) {
                        offense = (offense * 125) / 100;
                        add_bat(0, 25, label_get(1110)); // "Resonance Attack"
                    }
                    if (!has_abil(proto_id_def, ABL_DISSOCIATIVE_WAVE) && (psi_combat_type & 2)
                        && has_abil(proto_id_atk, ABL_EMPATHIC)
                        && Rules.combat_pct_emp_song_atk_vs_psi) {
                        offense = (Rules.combat_pct_emp_song_atk_vs_psi + 100) * offense / 100;
                        add_bat(0, Rules.combat_pct_emp_song_atk_vs_psi,
                            Ability[abil_index(ABL_EMPATHIC)].name);
                    }
                }
                if (Vehs[veh_id_atk].state & VSTATE_MADE_AIRDROP
                    && has_abil(proto_id_atk, ABL_DROP_POD) && Rules.combat_pen_pct_atk_airdrop) {
                    offense = (100 - Rules.combat_pen_pct_atk_airdrop) * offense / 100;
                    /*
                    uint32_t dropRange;
                    if (has_tech(Rules.tech_orb_insert_sans_spc_elev, faction_id_atk)
                        || has_project(SP_SPACE_ELEVATOR, faction_id_atk)) {
                        dropRange = (*MapHorizontalBounds <= MapLatitudeBounds)
                            ? MapLatitudeBounds : *MapHorizontalBounds;
                    } else {
                        dropRange = Rules.max_airdrop_sans_orb_insert;
                    }
                    */
                    add_bat(0, -Rules.combat_pen_pct_atk_airdrop,
                        drop_range(faction_id_atk) <= Rules.max_airdrop_sans_orb_insert
                        ? label_get(437) // "Air Drop"
                        : label_get(438)); // "Orbital Insertion"
                }
                if (Players[faction_id_atk].rule_flags & RFLAG_FANATIC
                    && Rules.combat_pct_fanatic_atk_bonus && !combat_type && !psi_combat_type) {
                    offense = (Rules.combat_pct_fanatic_atk_bonus + 100) * offense / 100;
                    add_bat(0, Rules.combat_pct_fanatic_atk_bonus, label_get(528));
                }
                int bonus_count = Players[faction_id_atk].faction_bonus_count;
                for (int i = 0; i < bonus_count; i++) {
                    if (Players[faction_id_atk].faction_bonus_id[i] == RULE_OFFENSE) {
                        int rule_off_bonus = Players[faction_id_atk].faction_bonus_val1[i];
                        offense = rule_off_bonus * offense / 100;
                        add_bat(0, rule_off_bonus - 100, label_get(1108)); // "Alien Offense"
                    }
                }
                if (psi_combat_type && faction_id_atk) {
                    int planet_atk = PlayersData[faction_id_atk].soc_effect_active.values_[SocialEffect::PLANET];
                    if (planet_atk && Rules.combat_pct_psi_atk_bonus_planet) {
                        int planet_modifier = planet_atk * Rules.combat_pct_psi_atk_bonus_planet;
                        add_bat(0, planet_modifier, label_get(625)); // "Planet"
                        offense = (planet_modifier + 100) * offense / 100;
                    }
                }
            }
        }
        if (!(combat_type & 2) && veh_id_def >= 0) {
            defense = get_basic_defense(veh_id_def, veh_id_atk, psi_combat_type, is_bombard);
            int bonus_count = Players[faction_id_def].faction_bonus_count;
            for (int i = 0; i < bonus_count; i++) {
                if (Players[faction_id_def].faction_bonus_id[i] == RULE_DEFENSE) {
                    int rule_def_bonus = Players[faction_id_def].faction_bonus_val1[i];
                    defense = rule_def_bonus * defense / 100;
                    add_bat(1, rule_def_bonus - 100, label_get(1109)); // "Alien Defense"
                }
            }
            if (veh_id_atk >= 0 && VehPrototypes[proto_id_def].plan == PLAN_INFO_WARFARE) {
                defense *= 4;
            } else {
                int x_def = Vehs[veh_id_def].x;
                int y_def = Vehs[veh_id_def].y;
                int base_id_def = base_at(x_def, y_def);
                if (psi_combat_type) {
                    if (base_id_def >= 0 && Rules.combat_pct_base_def) {
                        add_bat(1, Rules.combat_pct_base_def, label_get(332)); // "Base"
                        defense = (Rules.combat_pct_base_def + 100) * defense / 100;
                    }
                    defense *= 4;
                } else {
                    uint32_t triad_def = get_proto_triad(proto_id_def);
                    int terrain_def = (triad_def == TRIAD_AIR)
                        ? 2 : defense_value(faction_id_def, x_def, y_def, veh_id_def, veh_id_atk);
                    int alt_atk;
                    int alt_def;
                    if (veh_id_atk >= 0 && get_triad(veh_id_atk) == TRIAD_LAND) {
                        if (combat_type && Rules.combat_pct_art_bonus_lvl_alt
                            && triad_def == TRIAD_LAND
                            && (alt_atk = alt_at(Vehs[veh_id_atk].x, Vehs[veh_id_atk].y),
                                alt_def = alt_at(x_def, y_def), alt_atk > alt_def)) {
                            offense = (Rules.combat_pct_art_bonus_lvl_alt
                                * (alt_atk - alt_def) + 100) * offense / 100;
                            // "Altitude"
                            add_bat(0, Rules.combat_pct_art_bonus_lvl_alt
                                * (alt_atk - alt_def), label_get(576));
                        }
                        if (Rules.combat_pct_mobile_open_ground && !combat_type && base_id_def < 0
                            && terrain_def == 2
                            && rocky_at(x_def, y_def) < TERRAIN_BIT_ROCKY) {
                            int speed_atk = speed_proto(proto_id_atk);
                            if (speed_atk > Rules.move_rate_roads
                                && speed_proto(proto_id_def) <= speed_atk) {
                                offense = (Rules.combat_pct_mobile_open_ground + 100) * offense / 100;
                                // "Mobile in open"
                                add_bat(0, Rules.combat_pct_mobile_open_ground, label_get(611));
                            }
                        }
                        if (Rules.combat_pct_def_vs_mobile_rough && !combat_type
                            && (terrain_def > 2 || base_id_def >= 0)
                            && speed_proto(proto_id_atk) > Rules.move_rate_roads) {
                            defense = (Rules.combat_pct_def_vs_mobile_rough + 100) * defense / 100;
                            // "Rough vs. Mobile" : "Mobile vs. Base"
                            add_bat(1, Rules.combat_pct_def_vs_mobile_rough, base_id_def < 0
                                ? label_get(548) : label_get(612));
                        }
                        if (Rules.combat_pct_atk_road && !combat_type) {
                            offense = (Rules.combat_pct_atk_road + 100) * offense / 100;
                            add_bat(0, Rules.combat_pct_atk_road, label_get(606)); // "Road Attack"
                        }

                        if (Rules.combat_pen_pct_atk_lwr_elev && !combat_type
                            && altitude_at(x_def, y_def)
                            > altitude_at(Vehs[veh_id_atk].x, Vehs[veh_id_atk].y)
                            && !has_abil(proto_id_atk, ABL_ANTIGRAV_STRUTS)) {
                            // "Uphill"
                            add_bat(1, Rules.combat_pen_pct_atk_lwr_elev, label_get(441));
                            defense = (Rules.combat_pen_pct_atk_lwr_elev + 100) * defense / 100;
                        }
                        if (Rules.combat_pct_atk_higher_elev && !combat_type
                            && altitude_at(Vehs[veh_id_atk].x, Vehs[veh_id_atk].y)
                            > altitude_at(x_def, y_def)
                            && !has_abil(proto_id_def, ABL_ANTIGRAV_STRUTS)) {
                            add_bat(0, Rules.combat_pct_atk_higher_elev, label_get(330)); // "Downhill"
                            offense = (Rules.combat_pct_atk_higher_elev + 100) * offense / 100;
                        }
                    }
                    defense *= terrain_def;
                    if (terrain_def > 2) {
                        // NO C++ LIBRARY. The image's own callee list for this
                        // body is strcpy (0x00645460) and strcat (0x00645470),
                        // and add_bat copies what it is handed, so the shared
                        // StringTemp buffer is safe here.
                        strcpy(StringTemp, label_get(331)); // "Terrain"
                        strcat(StringTemp, " (");
                        strcat(StringTemp, StringTable.get(VehBattleDisplayTerrain));
                        strcat(StringTemp, ")");
                        add_bat(1, 10 * (5 * terrain_def - 10), StringTemp);
                    }
                    LPSTR display_def = 0; // only one is displayed
                    uint32_t def_multi = 2;
                    if (bit_at(x_def, y_def) & BIT_BUNKER && (veh_id_atk < 0
                        || get_triad(veh_id_atk) != TRIAD_AIR)) {
                        def_multi = 3;
                        display_def = label_get(358); // "Bunker"
                    }
                    if (base_id_def >= 0) {
                        display_def = label_get(332); // "Base"
                        def_multi = 2;
                        int fac_modifier = veh_id_atk < 0 ? veh_id_atk : 2;
                        if (veh_id_atk >= 0) {
                            uint32_t triad_atk = get_triad(veh_id_atk);
                            switch (triad_atk) {
                              case TRIAD_LAND:
                                if (has_fac(FAC_PERIMETER_DEFENSE, base_id_def, 0)
                                    || has_project(SP_CITIZENS_DEFENSE_FORCE, faction_id_def)) {
                                    display_def = label_get(354); // "Perimeter"
                                    fac_modifier = 4;
                                }
                                break;
                              case TRIAD_SEA:
                                if (has_fac(FAC_NAVAL_YARD, base_id_def, 0)
                                    || has_project(SP_MARITIME_CONTROL_CENTER, faction_id_def)) {
                                    display_def = Facility[FAC_NAVAL_YARD].name;
                                    fac_modifier = 4;
                                }
                                break;
                              case TRIAD_AIR:
                                if (has_fac(FAC_AEROSPACE_COMPLEX, base_id_def, 0)
                                    || has_project(SP_CLOUDBASE_ACADEMY, faction_id_def)) {
                                    display_def = Facility[FAC_AEROSPACE_COMPLEX].name;
                                    fac_modifier = 4;
                                }
                                break;
                              default:
                                fac_modifier = def_multi;
                                break;
                            }
                        }
                        if (has_fac(FAC_TACHYON_FIELD, base_id_def, 0)) {
                            fac_modifier += 2;
                            display_def = label_get(357); // "Tachyon"
                        }
                        if (fac_modifier >= 2) {
                            def_multi = fac_modifier;
                        }
                        if (fac_modifier <= 2
                            && (veh_id_atk < 0 || !has_abil(proto_id_atk, ABL_BLINK_DISPLACER))
                            && Rules.combat_pct_base_def) {
                            add_bat(1, Rules.combat_pct_base_def, label_get(332)); // "Base"
                            defense = (Rules.combat_pct_base_def + 100) * defense / 100;
                        }
                    }
                    if (veh_id_atk >= 0 && has_abil(proto_id_atk, ABL_BLINK_DISPLACER)) {
                        if (def_multi > 2) {
                            def_multi = 2;
                        }
                        display_def = label_get(428); // "Base vs. Blink"
                    }
                    if (Rules.combat_pct_infantry_vs_base && !combat_type
                        && base_id_def >= 0 && Vehs[veh_id_atk].faction_id) {
                        const int infantry_proto = Vehs[veh_id_atk].proto_id;
                        if ((infantry_proto >= MaxVehProtoFactionNum
                            || (get_proto_offense_rating(infantry_proto) >= 0
                                && infantry_proto != BSC_SPORE_LAUNCHER))
                            && !get_proto_triad(infantry_proto)
                            && Chassis[VehPrototypes[infantry_proto].chassis_id].speed == 1) {
                            offense = (Rules.combat_pct_infantry_vs_base + 100) * offense / 100;
                            // "Infantry vs. Base"
                            add_bat(0, Rules.combat_pct_infantry_vs_base, label_get(547));
                        }
                    }
                    uint32_t bit_def;
                    if (is_artillery && def_multi <= 2 && base_id_def < 0
                        && rocky_at(x_def, y_def) < TERRAIN_BIT_ROCKY
                        && (bit_def = bit_at(x_def, y_def),
                            !(bit_def & BIT_FOREST)) && (!(bit_def & BIT_FUNGUS)
                                || altitude_at(x_def, y_def)
                                < ALT_BIT_OCEAN_SHELF)) {
                        def_multi = 3;
                        display_def = label_get(525); // "Open Ground"
                    }
                    defense *= def_multi;
                    if (def_multi > 2) {
                        add_bat(1, 10 * (5 * def_multi - 10),
                            StringTable.get(display_def));
                    }
                }
                if (faction_id_def && veh_id_atk >= 0 && is_proto_missile(proto_id_atk)) {
                    for (int i = 0; i < 25; i++) {
                        int x_radius = wrap_map_x(x_def + RadiusOffsetX[i]);
                        int y_radius = y_def + RadiusOffsetY[i];
                        if (on_map(x_radius, y_radius)) {
                            int base_id_radius = base_at(x_radius, y_radius);
                            if (base_id_radius >= 0
                                && Bases[base_id_radius].faction_id_current == faction_id_def
                                && has_fac(FAC_FLECHETTE_DEFENSE_SYS, base_id_radius, 0)) {
                                defense = 3 * defense / 2;
                                add_bat(1, 50, label_get(1113)); // "Flechette"
                            }
                        }
                    }
                }
                if (!is_ocean(x_def, y_def)) {
                    uint32_t sensor_def = 0;
                    if (faction_id_def) {
                        for (int i = 0; i < 25; i++) {
                            int x_radius = wrap_map_x(x_def + RadiusOffsetX[i]);
                            int y_radius = y_def + RadiusOffsetY[i];
                            uint32_t sensor_status;
                            if (on_map(x_radius, y_radius)
                                && (sensor_status = is_sensor(x_radius, y_radius), sensor_status)) {
                                BOOL has_sensor = false;
                                if (!is_ocean(x_radius, y_radius)) {
                                    int base_id_find = base_find(x_radius, y_radius);
                                    // Preserve the original unchecked lookup result.
                                    if (Bases[base_id_find].faction_id_current == faction_id_def) {
                                        has_sensor = true;
                                    }
                                } else {
                                    int faction_id_terr = whose_territory(faction_id_def, x_radius,
                                        y_radius, NULL, false);
                                    if (faction_id_terr < 0
                                        || (uint32_t)faction_id_terr == faction_id_def) {
                                        has_sensor = true;
                                    }
                                }
                                if (has_sensor) {
                                    sensor_def |= 1;
                                    if (sensor_status > 1) {
                                        sensor_def |= 2;
                                    }
                                }
                            }
                        }
                    }
                    if (sensor_def & 1) {
                        defense = (Rules.combat_pct_def_range_sensor + 100) * defense / 100;
                        add_bat(1, Rules.combat_pct_def_range_sensor, label_get(613)); // "Sensor"
                    }
                    if (sensor_def & 2) {
                        defense = (Rules.combat_pct_def_range_sensor + 100) * defense / 100;
                        add_bat(1, Rules.combat_pct_def_range_sensor, label_get(1123)); // "GSP"
                    }
                }
                if (!has_abil(Vehs[veh_id_atk].proto_id, ABL_DISSOCIATIVE_WAVE)
                    && Rules.combat_pct_trance_def_vs_psi && veh_id_atk >= 0
                    && (psi_combat_type & 1)
                    && has_abil(proto_id_def, ABL_TRANCE)) {
                    defense = (Rules.combat_pct_trance_def_vs_psi + 100) * defense / 100;
                    add_bat(1, Rules.combat_pct_trance_def_vs_psi, label_get(329)); // "Trance"
                }
                if (veh_id_atk >= 0) {
                    uint32_t armor_id_def = VehPrototypes[proto_id_def].armor_id;
                    if ((psi_combat_type & 1) && (armor_id_def == ARM_RESONANCE_3_ARMOR
                        || armor_id_def == ARM_RESONANCE_8_ARMOR)) {
                        defense = 125 * defense / 100;
                        add_bat(1, 25, label_get(1111)); // "Resonance Def."
                    }
                    if (get_proto_triad(proto_id_atk) == TRIAD_AIR
                        && has_abil(proto_id_atk, ABL_AIR_SUPERIORITY) && !psi_combat_type) {
                        if (get_proto_triad(proto_id_def) != TRIAD_AIR) {
                            int ground_strike_pen = Rules.combat_pen_pct_air_supr_vs_grnd;
                            if (ground_strike_pen) {
                                offense = (100 - ground_strike_pen) * offense / 100;
                                add_bat(0, -ground_strike_pen, label_get(448)); // "Ground Strike"
                            }
                        } else {
                            int air_to_air = Rules.combat_pct_air_supr_vs_air;
                            if (air_to_air && !has_abil(proto_id_def, ABL_AIR_SUPERIORITY)) {
                                offense = (air_to_air + 100) * offense / 100;
                                add_bat(0, air_to_air, label_get(449)); // "Air-to-Air"
                            }
                        }
                    }
                    if (get_proto_triad(proto_id_atk) == TRIAD_AIR
                        && get_proto_triad(proto_id_def) == TRIAD_AIR
                        && has_abil(proto_id_def, ABL_AIR_SUPERIORITY)
                        && !has_abil(proto_id_atk, ABL_AIR_SUPERIORITY) && !psi_combat_type
                        && !is_proto_missile(proto_id_atk) && !is_proto_missile(proto_id_def)
                        && Rules.combat_pct_air_supr_vs_air) {
                        defense = (Rules.combat_pct_air_supr_vs_air + 100) * defense / 100;
                        add_bat(1, Rules.combat_pct_air_supr_vs_air, label_get(449)); // "Air-to-Air"
                    }
                    if (!get_proto_offense_rating(proto_id_def)
                        && get_proto_defense_rating(proto_id_def) == 1 && (faction_id_atk
                            || base_id_def < 0) && Rules.combat_pen_pct_non_cbt_def_vs_cbt) {
                        defense = (100 - Rules.combat_pen_pct_non_cbt_def_vs_cbt) * defense / 100;
                        // "Non Combat"
                        add_bat(1, -Rules.combat_pen_pct_non_cbt_def_vs_cbt, label_get(439));
                    }
                    if (get_proto_triad(proto_id_atk) == TRIAD_SEA
                        && get_proto_triad(proto_id_def) == TRIAD_SEA && base_id_def >= 0
                        && Rules.combat_pct_bonus_vs_ship_port) {
                        offense = (Rules.combat_pct_bonus_vs_ship_port + 100) * offense / 100;
                        add_bat(0, Rules.combat_pct_bonus_vs_ship_port, label_get(335)); // "In Port"
                    }
                    if ((armor_id_def == ARM_PULSE_3_ARMOR || armor_id_def == ARM_PULSE_8_ARMOR)
                        && get_proto_triad(proto_id_atk) == TRIAD_LAND
                        && Chassis[VehPrototypes[proto_id_atk].chassis_id].speed > 1) {
                        defense = 125 * defense / 100;
                        add_bat(1, 25, label_get(1112)); // "Pulse Defense"
                    }
                    if (!has_abil(proto_id_atk, ABL_DISSOCIATIVE_WAVE)
                        && has_abil(proto_id_def, ABL_COMM_JAMMER)
                        && get_proto_triad(proto_id_atk) == TRIAD_LAND
                        && Chassis[VehPrototypes[proto_id_atk].chassis_id].speed > 1
                        && Rules.combat_pct_com_jam_def_vs_mobl) {
                        defense = (Rules.combat_pct_com_jam_def_vs_mobl + 100) * defense / 100;
                        add_bat(1, Rules.combat_pct_com_jam_def_vs_mobl, label_get(336)); // "Jammer"
                    }
                    if (!has_abil(proto_id_atk, ABL_DISSOCIATIVE_WAVE)
                        && has_abil(proto_id_def, ABL_AAA) && get_proto_triad(proto_id_atk) == TRIAD_AIR
                        && Rules.combat_pct_aaa_bonus_vs_air) {
                        defense = (Rules.combat_pct_aaa_bonus_vs_air + 100) * defense / 100;
                        add_bat(1, Rules.combat_pct_aaa_bonus_vs_air, label_get(337)); // "Tracking"
                    }
                }
            }
        } else if (veh_id_def >= 0) {
            defense = get_basic_offense(veh_id_def, veh_id_atk, psi_combat_type, is_bombard, true);
            if (!(combat_type & 0x18)) {
                uint32_t triad_atk = get_proto_triad(proto_id_atk);
                uint32_t triad_def = get_proto_triad(proto_id_def);
                if (triad_def == TRIAD_SEA && triad_atk == TRIAD_LAND) {
                    int gun_vs_ship_art = Rules.combat_pct_land_gun_vs_ship_art;
                    if (gun_vs_ship_art) {
                        offense = (gun_vs_ship_art + 100) * offense / 100;
                        add_bat(0, gun_vs_ship_art, label_get(435)); // "Land Based Guns"
                    }
                } else if (triad_atk == TRIAD_SEA && triad_def == TRIAD_LAND) {
                    int gun_vs_ship_art = Rules.combat_pct_land_gun_vs_ship_art;
                    if (gun_vs_ship_art) {
                        defense = (gun_vs_ship_art + 100) * defense / 100;
                        add_bat(1, gun_vs_ship_art, label_get(435)); // "Land Based Guns"
                    }
                } else if (Rules.combat_pct_art_bonus_lvl_alt) {
                    uint32_t alt_atk = alt_at(Vehs[veh_id_atk].x, Vehs[veh_id_atk].y);
                    uint32_t alt_def = alt_at(Vehs[veh_id_def].x, Vehs[veh_id_def].y);
                    if (alt_def >= alt_atk) {
                        if (alt_def > alt_atk) {
                            int alt_modifier_def = (alt_def - alt_atk)
                                * Rules.combat_pct_art_bonus_lvl_alt;
                            defense = (alt_modifier_def + 100) * defense / 100;
                            add_bat(1, alt_modifier_def, label_get(576)); // "Altitude"
                        }
                    } else {
                        int alt_mod_atk = (alt_atk - alt_def) * Rules.combat_pct_art_bonus_lvl_alt;
                        offense = (alt_mod_atk + 100) * offense / 100;
                        add_bat(0, alt_mod_atk, label_get(576)); // "Altitude"
                    }
                }
            }
        }
    }
    if (offense_out) {
        *offense_out = offense;
    }
    if (defense_out) {
        *defense_out = defense;
    }
}

// proto_power() by prototype: best_defender expands it twice.
static inline int best_defender_power(int proto_id) {
    if (VehPrototypes[proto_id].plan == PLAN_ALIEN_ARTIFACT) {
        return 1;
    }
    return range(VehPrototypes[proto_id].reactor_id, 1, 100) * 10;
}

/*
Purpose: Determine the best defender in a stack.
// ORIGINAL: 0x005044D0 ?best_defender@@YAHHHH@Z 0x005044D0-0x00504A9B
// LEVER: MORE (14 calls vs image's 12) - per-callee counts matched the
//        image exactly (base_at 1, get_basic_offense 1, battle_compute 1,
//        stack_check 3, has_abil 3, can_arty 3 = 12) with two calls this
//        tree made that are not in the image's callee set at all:
//        veh_top() and proto_power(). veh_top() is a plain same-TU call that
//        /Ob2 inlines (a hand copy here scored 0.33, the call 0.43).
//        proto_power() moved to veh.h as `MEASURED inline`
//        (see its own ORIGINAL marker) since it was a plain non-`inline`
//        function and so never an /Ob1 inline candidate at either of its
//        two callers. Call count now agrees (0 disagree); best flag set's
//        similarity 0.259 -> 0.275.
// symbol    ?best_defender@@YAHHHH@Z
// size      1483 bytes
// prototype int (__cdecl ?best_defender@@YAHHHH@Z)(int vehIDDef, int vehIDAtk, BOOL useArtillery)
// callers   1   call targets   6
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004E3A50 0x005015B0 0x00501DA0 0x005B9580 0x005BF1F0 0x005C0DB0
Return Value: Unit id of the best defender
Status: Complete - testing
*/
int __cdecl best_defender(int veh_id_def, int veh_id_atk, BOOL check_artillery) {
    int offense = veh_id_atk >= 0
        ? get_basic_offense(veh_id_atk, veh_id_def, false, false, false) : 8;
    int x_def = Vehs[veh_id_def].x;
    int y_def = Vehs[veh_id_def].y;
    BOOL is_ocean_def = is_ocean(x_def, y_def);
    int base_id_def = base_at(x_def, y_def);
    int defender_search = -999;
    int best_def_veh_id = veh_id_def;
    for (veh_id_def = veh_top(veh_id_def); veh_id_def >= 0;
        veh_id_def = Vehs[veh_id_def].next_veh_id_stack) {
        // A land unit at sea outside a base is aboard a transport.
        if (get_proto_triad(Vehs[veh_id_def].proto_id) == TRIAD_LAND && is_ocean_def && base_id_def < 0) {
            continue;
        }
        if (VehPrototypes[Vehs[veh_id_atk].proto_id].plan == PLAN_INFO_WARFARE
            && VehPrototypes[Vehs[veh_id_def].proto_id].plan != PLAN_INFO_WARFARE) {
            continue;
        }
        uint32_t combat_type = (can_arty(Vehs[veh_id_def].proto_id, true)
            && can_arty(Vehs[veh_id_atk].proto_id, true)) ? 2 : 0;
        if (veh_id_atk >= 0 && get_triad(veh_id_atk) == TRIAD_AIR
            && has_abil(Vehs[veh_id_def].proto_id, ABL_AIR_SUPERIORITY)
            && !is_missile(veh_id_def) && !is_missile(veh_id_atk)
            && get_offense_rating(veh_id_atk) > 0 && get_defense_rating(veh_id_atk) > 0
            && get_offense_rating(veh_id_def) > 0) {
            combat_type |= 0xA; // air combat
            if (get_triad(veh_id_def) == TRIAD_AIR) {
                combat_type |= 0x10;
            }
        }
        int offense_output;
        int defense_output;
        battle_compute(veh_id_atk, veh_id_def, &offense_output, &defense_output, combat_type);
        if (!offense_output) {
            break;
        }
        int proto_id_def = Vehs[veh_id_def].proto_id;
        defense_output = range(best_defender_power(proto_id_def) - Vehs[veh_id_def].dmg_incurred, 0, 9999)
            * defense_output / best_defender_power(proto_id_def);
        int def_modifier = defense_output * offense / offense_output / 8
            - get_proto_offense_rating(proto_id_def);
        uint8_t plan_def = VehPrototypes[proto_id_def].plan;
        if (plan_def < PLAN_COLONIZATION || plan_def == PLAN_TERRAFORMING) {
            def_modifier *= 16;
        }
        if (veh_id_atk >= 0 && get_triad(veh_id_atk) == TRIAD_AIR
            && has_abil(Vehs[veh_id_def].proto_id, ABL_AIR_SUPERIORITY)
            && get_triad(veh_id_def) == TRIAD_AIR
            && !is_missile(veh_id_def) && !is_missile(veh_id_atk)) {
            def_modifier += 0x80000;
        } else if (veh_id_atk >= 0 && get_triad(veh_id_atk) == TRIAD_AIR
            && has_abil(Vehs[veh_id_atk].proto_id, ABL_AIR_SUPERIORITY)
            && get_triad(veh_id_def) == TRIAD_AIR && base_id_def < 0
            && !(bit_at(x_def, y_def) & BIT_AIRBASE)
            && !stack_check(veh_id_def, 6, ABL_CARRIER, -1, -1)
            && !is_missile(veh_id_def) && !is_missile(veh_id_atk)) {
            def_modifier += 0x80000;
        } else if (check_artillery) {
            if (can_arty(Vehs[veh_id_def].proto_id, true)) {
                def_modifier += 0x80000;
            }
        } else if ((!stack_check(veh_id_def, 3, TRIAD_AIR, -1, -1) || base_id_def >= 0
            || bit_at(x_def, y_def) & BIT_AIRBASE
            || stack_check(veh_id_def, 6, ABL_CARRIER, -1, -1))
            && Vehs[veh_id_def].state & VSTATE_DESIGNATE_DEFENDER) {
            def_modifier += 0x80000;
        }
        int defense = veh_id_def + (def_modifier << 11);
        if (defense > defender_search) {
            defender_search = defense;
            best_def_veh_id = veh_id_def;
        }
    }
    return best_def_veh_id;
}

// Structural evidence superseding the unresolved object-binding note below:
// defense_value stores the terrain string pointer here, and battle_compute
// reads the same pointer for the battle display.
// RECOVERY_OBJECT: v1 0x0090F550 ?VehBattleDisplayTerrain@@3PADA
LPSTR VehBattleDisplayTerrain;

// get_basic_offense/get_basic_defense write the two side values; battle_report
// reads the selected value at 0x004227DC.
// Extent: compiled sizeof 0x8; next mapped object at 0x00912428.
// RECOVERY_OBJECT: v2 0x00912420 0x00912428 ?VehBasicBattleMorale@@3PAIA 0x004227DC
uint32_t VehBasicBattleMorale[2];  // 0x00912420
// Original battle_init at 0x00501D32 clears the start; add_bat at 0x00501D5D/0x00501D70 indexes counters; no bounds claim.
// Extent: compiled sizeof 0x8; next mapped object at 0x0091561C.
// RECOVERY_OBJECT: v2 0x00915614 0x0091561C ?VehBattleModCount@@3PAIA 0x0042287B
uint32_t VehBattleModCount[2];  // 0x00915614
// Original battle_init/planet_busting write 0x0091561C; kill and alt_set read it.
// RECOVERY_OBJECT: v1 0x0091561C ?VehBattleUnkTgl@@3HA
BOOL VehBattleUnkTgl;  // 0x0091561C
// add_bat/battle_compute write indexed modifiers; battle_report reads them.
// Extent: compiled sizeof 0x20; next mapped object 0x00915614, gap unreferenced.
// RECOVERY_OBJECT: v2 0x009155F0 0x00915610 ?VehBattleModifier@@3PAHA 0x004228A3
int VehBattleModifier[8];  // 0x009155F0
// add_bat/battle_compute fill rows consumed by battle_report.
// Extent: compiled sizeof 0x280; next mapped object at 0x0090F7D4.
// RECOVERY_OBJECT: v2 0x0090F554 0x0090F7D4 ?VehBattleDisplay@@3PAY0FA@DA 0x004228A9
char VehBattleDisplay[8][80];  // 0x0090F554

// boom temporarily suppresses one vehicle while redrawing the affected tile,
// then restores -1. This is separate from the Console's selected vehicle.
// RECOVERY_OBJECT: v2 0x00689F30 0x00689F34 ?VehDrawSuppressedVehicle@@3HA 0x005593E0
int VehDrawSuppressedVehicle = -1;
// Independent original selection scratch words; both start at -1. The
// selection painter uses these source-owned words, not original addresses.
// RECOVERY_OBJECT: v1 0x00689F28 ?VehDrawSelection689F28@@3HA
int VehDrawSelection689F28 = -1;
// RECOVERY_OBJECT: v1 0x00689F2C ?VehDrawSelection689F2C@@3HA
int VehDrawSelection689F2C = -1;
// The paired arrays are indexed by the prototype weapon byte (+0x25). The
// original reads both at 0x504DB0..0x504DF7; the first selects a 16-frame
// indexed-sprite bank and the second selects its explosion colour group.
// The two observed 26-DWORD extents meet at 0x0066EBA8 and 0x0066EC10.
// RECOVERY_OBJECT: v2 0x0066EB40 0x0066EBA8 _BoomIndexedSpriteBankByWeapon 0x00504AA0
static const int BoomIndexedSpriteBankByWeapon[MaxWeaponNum] = {
    0, 4, 0, 0, 4, 1, 5, 5, 3, 6, 2, 7, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0
};
// RECOVERY_OBJECT: v2 0x0066EBA8 0x0066EC10 _BoomColorGroupByWeapon 0x00504AA0
static const int BoomColorGroupByWeapon[MaxWeaponNum] = {
    3, 1, 0, 0, 2, 4, 5, 6, 5, 2, 7, 5, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0
};

// Producer 0x00500B60 sets this cell before invoking boom(..., 0x20).
// It is separate from the compiler-owned local-static guard at 0x90F7D8.
// RECOVERY_OBJECT: v2 0x0090F7D4 0x0090F7D8 ?BoomAnimationLevel@@3HA 0x00500B60
int BoomAnimationLevel;

static int boom_zoomed_extent(int zoom, int extent) {
    const int product = zoom * extent;
    return (product + (product >> 31 & 0x0f)) >> 4;
}

// ORIGINAL: 0x00504AA0 ?boom@@YAHHHH@Z 0x00504AA0-0x00505D1B FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/00504aa0.cpp", "src/veh.h", "src/flic.h", "recovery/tasks/mapwin-owner-coverage-wave.json", "Pinned 0x00504AA0 guard 0x0090F7D8 lazily constructs the two Buffer[8] arrays at 0x0090F7E0 and 0x00912428, registers distinct atexit destructors, then constructs/registers Flic at 0x0090EA68 and Buffer at 0x00915068.", "The four adjacent original teardown identities now have source-owned local-static object bindings at their exact original data extents; fresh retained COFF callback relocations and MAP owner rows are preserved in the generated identity records beside each declaration.", "All original callback spans, claims and TRIED lesson text are transferred to the corresponding source declaration mapping. Physical wrapper names are written back from retained COFF facts, not inferred from seed names."], "reason": "Recover per-window explosion and Flic animation with genuine typed static buffers, real MapWin/GraphicWin bases, source-owned weapon tables and original branch/timing/sound behavior; preserve the original seed and its correction evidence."}
// TRIED: mechanical transcription of the Ghidra decompilation (integer fixed-point animation math, no float). Two one-time static-init pools (8 Buffers at 0x912428, 8 more at 0x90f7e0, each array-constructed via the compiler's own `??_L` array- construct helper already declared as fn_006457c2) plus a lone Flic/Buffer pair at 0x90ea68/0x915068 constructed with placement `new` and registered via atexit for teardown - all four receivers confirmed from the raw disassembly's `mov ecx, ADDR` before each ctor call. Every FUN_ callee resolved to a real name from the disassembly's own comments. Not fully chased: several Buffer::copy/init/draw_multi_ table_dest calls inside the 8-slot animation loop use a fixed 0x915068 receiver as a placeholder - the raw asm shows the true receiver is a per-iteration resolved screen-buffer pointer (the same `*(int*)0x7d3c3c`-based idiom used throughout this codebase) that would take another pass to thread through every call site correctly. Landed at MISMATCH #4 without chasing the prologue divergence, given the size of this function and the remaining time in this batch.
// symbol    ?boom@@YAHHHH@Z
int __cdecl boom(int x, int y, int flags) {
    // The two arrays and both working buffers are genuine function-local
    // statics. Their constructors/destructors and first-use guard are thus
    // compiler-owned rather than replayed through fixed image addresses.
// ORIGINAL: 0x00505D60 ??__Eg_BOOM_BUFFERS2@@YAXXZ 0x00505D60-0x00505D77 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned original initializer 0x00504AA0 constructs and registers this object's matching teardown callback", "retained COFF build/recovery/builds/build-sep4rpby/CMakeFiles/OpenSMACX.dir/src/veh.cpp.obj: _$E19 physically relocates to _?original_screen_snapshots@?1??boom@@YAHHHH@Z@4PAVBuffer@@A", "retained MAP names veh.cpp.obj as the generated wrapper's physical contribution", "full original span, historical claim and seed lessons are retained; this mapping does not assert byte equality"], "reason": "Map the original BOOM atexit identity to the compiler-emitted destructor wrapper owned by the corresponding genuine function-local object."}
// symbol    _$E14
// kind      game
// TRIED: same wall as 0x00505D40 - the body calls the MSVC 'eh vector constructor iterator' at 0x006456E4, whose mangled name no forward declaration can reach. Needs a real global array with a non-trivial ctor and dtor.
// TRIED: same wall as 0x00505D40 - the body calls the MSVC 'eh vector constructor iterator' at 0x006456E4, whose mangled name no forward declaration can reach. Needs a real global array with a non-trivial ctor and dtor.
// RECOVERY_OBJECT: v2 0x00912428 0x00915068 _?original_screen_snapshots@?1??boom@@YAHHHH@Z@4PAVBuffer@@A 0x00504AA0
    static Buffer original_screen_snapshots[8];
// ORIGINAL: 0x00505D40 ??__Eg_BOOM_BUFFERS1@@YAXXZ 0x00505D40-0x00505D57 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned original initializer 0x00504AA0 constructs and registers this object's matching teardown callback", "retained COFF build/recovery/builds/build-sep4rpby/CMakeFiles/OpenSMACX.dir/src/veh.cpp.obj: _$E20 physically relocates to _?redrawn_map_snapshots@?1??boom@@YAHHHH@Z@4PAVBuffer@@A", "retained MAP names veh.cpp.obj as the generated wrapper's physical contribution", "full original span, historical claim and seed lessons are retained; this mapping does not assert byte equality"], "reason": "Map the original BOOM atexit identity to the compiler-emitted destructor wrapper owned by the corresponding genuine function-local object."}
// symbol    _$E15
// kind      game
// TRIED: homed into general.cpp 2026-09-03 - does not compile: the body calls 0x006456E4, which is the MSVC 'eh vector constructor iterator' CRT helper (mangled ??_M@YGXPAXIHP6EX0@Z@Z). Its mangled name is compiler-generated, so a normal forward declaration cannot reach it; the faithful source form is a real global array of a class with a non-trivial ctor and dtor, which is a recovery design for the global-object pass, not a homing. Stays here.
// RECOVERY_OBJECT: v2 0x0090F7E0 0x00912420 _?redrawn_map_snapshots@?1??boom@@YAHHHH@Z@4PAVBuffer@@A 0x00504AA0
    static Buffer redrawn_map_snapshots[8];
// ORIGINAL: 0x00505D30 ??__Eg_BOOM_FLIC@@YAXXZ 0x00505D30-0x00505D3A FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned original initializer 0x00504AA0 constructs and registers this object's matching teardown callback", "retained COFF build/recovery/builds/build-sep4rpby/CMakeFiles/OpenSMACX.dir/src/veh.cpp.obj: _$E21 physically relocates to _?animation@?1??boom@@YAHHHH@Z@4VFlic@@A", "retained MAP names veh.cpp.obj as the generated wrapper's physical contribution", "full original span, historical claim and seed lessons are retained; this mapping does not assert byte equality"], "reason": "Map the original BOOM atexit identity to the compiler-emitted destructor wrapper owned by the corresponding genuine function-local object."}
// symbol    _$E16
// kind      game
// RECOVERY_OBJECT: v2 0x0090EA68 0x0090F54C _?animation@?1??boom@@YAHHHH@Z@4VFlic@@A 0x00504AA0
    static Flic animation;
// ORIGINAL: 0x00505D20 ??__Eg_BOOM_BUFFER1@@YAXXZ 0x00505D20-0x00505D2A FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned original initializer 0x00504AA0 constructs and registers this object's matching teardown callback", "retained COFF build/recovery/builds/build-sep4rpby/CMakeFiles/OpenSMACX.dir/src/veh.cpp.obj: _$E22 physically relocates to _?working_buffer@?1??boom@@YAHHHH@Z@4VBuffer@@A", "retained MAP names veh.cpp.obj as the generated wrapper's physical contribution", "full original span, historical claim and seed lessons are retained; this mapping does not assert byte equality"], "reason": "Map the original BOOM atexit identity to the compiler-emitted destructor wrapper owned by the corresponding genuine function-local object."}
// symbol    _$E17
// kind      game
// RECOVERY_OBJECT: v2 0x00915068 0x009155F0 _?working_buffer@?1??boom@@YAHHHH@Z@4VBuffer@@A 0x00504AA0
    static Buffer working_buffer;

    int selected_vehicle = VehDrawSelection689F28;
    int other_selected_vehicle = VehDrawSelection689F2C;
    int saved_selection = selected_vehicle;
    int second_selection = other_selected_vehicle;
    int animation_state = flags & 0x70;
    int sprite_bank = 0;
    int palette_row = 0;
    int frame_span = 0;
    int animation_height = 0;
    bool special_chassis = false;
    bool flic_mode = false;
    int faction_color_group[28];

    if (animation_state == 0) {
        const Veh &initial_selected = Vehs[selected_vehicle];
        const VehPrototype &initial_prototype =
            VehPrototypes[initial_selected.proto_id];
        if (initial_prototype.chassis_id == CHSI_MISSILE && (flags & 1) != 0) {
            g_FX.play(0x52, initial_selected.x, initial_selected.y);
            return 0;
        }
        if ((flags & 1) != 0) {
            if (VehPrototypes[Vehs[selected_vehicle].proto_id].weapon_id == 15 ||
                VehPrototypes[Vehs[other_selected_vehicle].proto_id].weapon_id == 15) {
                special_chassis = true;
            }
            if (Vehs[other_selected_vehicle].proto_id == 1) {
                g_FX.play(1);
            }
        }
        if ((flags & 8) == 0 && !special_chassis) {
            selected_vehicle = VehDrawSelection689F2C;
            other_selected_vehicle = VehDrawSelection689F28;
            saved_selection = selected_vehicle;
            second_selection = other_selected_vehicle;
        }

        if ((flags & 2) != 0 && other_selected_vehicle >= 0) {
            const int selected_proto = Vehs[other_selected_vehicle].proto_id;
            if (selected_proto == 8 || selected_proto == 9 ||
                selected_proto == 10 || selected_proto == 14 ||
                selected_proto == 15 || selected_proto == 19) {
                flags |= 0x100;
            }
        }
        if ((flags & 1) != 0) {
            if (special_chassis) {
                sprite_bank = BoomIndexedSpriteBankByWeapon[12];
                palette_row = BoomColorGroupByWeapon[12];
            } else if (saved_selection != -1) {
                const VehPrototype &prototype =
                    VehPrototypes[Vehs[other_selected_vehicle].proto_id];
                sprite_bank = BoomIndexedSpriteBankByWeapon[prototype.weapon_id];
                palette_row = BoomColorGroupByWeapon[prototype.weapon_id];
            }
        }
    } else {
        flic_mode = true;
        if ((flags & 0x20) != 0 &&
            working_buffer.init(300, 0xf8, 0, nullptr) == 0) {
            char animation_file[] = "buster.flc";
            if (animation.init(&working_buffer, animation_file, 0, 0, 0) == 0) {
                animation_state = 1;
            }
        }
        faction_color_group[0] = 0;
        faction_color_group[1] = 5;
        faction_color_group[2] = 2;
        faction_color_group[3] = 10;
        faction_color_group[4] = 9;
        faction_color_group[5] = 11;
        faction_color_group[6] = 0;
        faction_color_group[7] = 2;
        faction_color_group[12] = 2;
        faction_color_group[13] = 2;
        faction_color_group[14] = 2;
        faction_color_group[15] = 2;
        faction_color_group[16] = 3;
        faction_color_group[17] = 4;
        faction_color_group[18] = 5;
        faction_color_group[19] = 5;
        const int level = BoomAnimationLevel;
        animation_height = faction_color_group[level + 15];
        frame_span = faction_color_group[level + 11];
    }

    // Original X/Y are distinct eight-DWORD windows at EBP-0x74/-0x54;
    // RECTs at EBP-0x144 advance 16 bytes. Draw/copy use X/Y, update a RECT.
    int pixel_x_by_slot[8];
    int pixel_y_by_slot[8];
    RECT saved_rectangles[8];
    // Original 0x00504DF5/0x00504EB8 store DWORD flags; the slot cursor
    // advances four bytes over eight entries (0x005054C2/0x005054CB).
    int active_windows[8];
    bool any_window = false;

    for (int slot = 0; slot < 8; ++slot) {
        MapWin *window = MapWinTable[slot];
        active_windows[slot] = false;
        if (window == nullptr || (slot != 0 && window->view_active_ == 0)) {
            continue;
        }
        if (y < window->field_1DDA8_ ||
            y >= window->field_1DDA8_ + window->field_1DDD8_ +
                     window->field_1DDD0_) {
            continue;
        }
        int adjusted_x = x;
        if ((MapIsFlat & 1) == 0) {
            if (adjusted_x < window->field_1DDA4_) {
                adjusted_x += MapLongitudeBounds;
            }
            if (adjusted_x >= window->field_1DDA4_ + window->field_1DDD4_ +
                                  window->field_1DDCC_) {
                adjusted_x -= MapLongitudeBounds;
            }
        }
        if (adjusted_x < window->field_1DDA4_ ||
            adjusted_x >= window->field_1DDA4_ + window->field_1DDD4_ +
                              window->field_1DDCC_) {
            continue;
        }

        int &pixel_x = pixel_x_by_slot[slot];
        int &pixel_y = pixel_y_by_slot[slot];
        // Original 0x00504EC5 uses this slot; 0x00504ECA..0x00504ECD
        // reloads the slot after the call.
        window->tile_to_pixel(x, y, &pixel_x_by_slot[slot],
                              &pixel_y_by_slot[slot]);
        window = MapWinTable[slot];
        pixel_x += window->tile_pixel_width_ / 2;
        pixel_y += window->tile_pixel_height_;

        const int zoom = window->zoom_offset_ + 0x10;
        if (!flic_mode) {
            const bool use_xf = (flags & 1) == 0;
            Sprite *const sizing_sprite = use_xf
                ? &g_XF_BOOM_VEH_SPRITES[0]
                : &g_XI_BOOM_VEH_SPRITES[0];
            const int source_width = sizing_sprite->width();
            const int source_height = sizing_sprite->height();
            int scaled_width;
            int scaled_height;
            int full_scaled_width;
            if (MapWinCaptionScale < 1) {
                scaled_width = boom_zoomed_extent(zoom, source_width / 2);
                full_scaled_width = boom_zoomed_extent(zoom, source_width);
                scaled_height = boom_zoomed_extent(zoom, source_height);
            } else {
                scaled_width = ((source_width / 2) * MapWinCaptionScale) /
                               MapWinCaptionDivisor;
                full_scaled_width = (source_width * MapWinCaptionScale) /
                                    MapWinCaptionDivisor;
                scaled_height = (source_height * MapWinCaptionScale) /
                                MapWinCaptionDivisor;
            }
            pixel_x -= scaled_width;
            pixel_y -= scaled_height;
            saved_rectangles[slot].left = pixel_x;
            saved_rectangles[slot].top = pixel_y;
            saved_rectangles[slot].right = pixel_x + full_scaled_width;
            saved_rectangles[slot].bottom = pixel_y + scaled_height;
        } else {
            const int unscaled_width = (animation_height * 300) / frame_span;
            const int unscaled_height = (animation_height * 0xf8) / frame_span;
            const int vertical_offset =
                (animation_height * 0x3c) / frame_span;
            int scaled_width;
            int scaled_height;
            if (MapWinCaptionScale < 1) {
                scaled_width = boom_zoomed_extent(zoom, unscaled_width);
                scaled_height = boom_zoomed_extent(zoom, unscaled_height);
                pixel_y += boom_zoomed_extent(zoom, vertical_offset);
            } else {
                scaled_width = unscaled_width * MapWinCaptionScale /
                               MapWinCaptionDivisor;
                scaled_height = unscaled_height * MapWinCaptionScale /
                                MapWinCaptionDivisor;
                pixel_y += vertical_offset * MapWinCaptionScale /
                           MapWinCaptionDivisor;
            }
            pixel_x -= scaled_width / 2;
            pixel_y -= scaled_height;
            saved_rectangles[slot].left = pixel_x;
            saved_rectangles[slot].top = pixel_y;
            saved_rectangles[slot].right = pixel_x + scaled_width;
            saved_rectangles[slot].bottom = pixel_y + scaled_height;
        }

        active_windows[slot] = true;
        any_window = true;

        if (animation_state == 0 && (flags & 1) == 0) {
            if ((flags & 0x100) == 0) {
                if (Vehs[second_selection].proto_id == BSC_SPORE_LAUNCHER) {
                    g_FX.play(0x5a, Vehs[second_selection].x,
                              Vehs[second_selection].y);
                } else if (VehPrototypes[Vehs[saved_selection].proto_id]
                               .chassis_id != CHSI_MISSILE) {
                    g_FX.play(0x3e, x, y);
                }
            } else {
                g_FX.play(0x3d, x, y);
            }
        }

        // The preceding FX dispatch is callback-capable. The image reloads
        // this slot before resolving its per-window Buffer receiver.
        window = MapWinTable[slot];

        Buffer *window_buffer =
            static_cast<Buffer *>(static_cast<GraphicWin *>(window));
        const RECT &area = saved_rectangles[slot];
        const int width = area.right - area.left;
        const int height = area.bottom - area.top;
        if (animation_state != 0 && (flags & 0x60) == 0) {
            continue;
        }
        if ((flags & 0x40) == 0) {
            original_screen_snapshots[slot].init(width, height, 0, nullptr);
            window = MapWinTable[slot];
            window_buffer =
                static_cast<Buffer *>(static_cast<GraphicWin *>(window));
            window_buffer->copy(&original_screen_snapshots[slot], pixel_x_by_slot[slot],
                                pixel_y_by_slot[slot], 0, 0,
                                original_screen_snapshots[slot].pixel_width(),
                                original_screen_snapshots[slot].pixel_height());
        }
        if (((flags & 2) == 0 || (flags & 0x20) != 0) &&
            (flags & 0x40) == 0) {
            continue;
        }
        if ((flags & 2) != 0 && (flags & 0x20) == 0 &&
            (flags & 0x40) == 0) {
            VehDrawSuppressedVehicle = (flags & 8) == 0
                ? VehDrawSelection689F28 : VehDrawSelection689F2C;
            window = MapWinTable[slot];
            window->draw_tile(x, y, 0);
            VehDrawSuppressedVehicle = -1;
            window = MapWinTable[slot];
            window_buffer =
                static_cast<Buffer *>(static_cast<GraphicWin *>(window));
        }
        redrawn_map_snapshots[slot].init(width, height, 0, nullptr);
        window = MapWinTable[slot];
        window_buffer =
            static_cast<Buffer *>(static_cast<GraphicWin *>(window));
        window_buffer->copy(&redrawn_map_snapshots[slot], pixel_x_by_slot[slot], pixel_y_by_slot[slot],
                            0, 0, redrawn_map_snapshots[slot].pixel_width(),
                            redrawn_map_snapshots[slot].pixel_height());
        window = MapWinTable[slot];
        window_buffer =
            static_cast<Buffer *>(static_cast<GraphicWin *>(window));
        original_screen_snapshots[slot].copy(window_buffer, 0, 0,
                                             pixel_x_by_slot[slot], pixel_y_by_slot[slot],
                                             original_screen_snapshots[slot].pixel_width(),
                                             original_screen_snapshots[slot].pixel_height());
    }

    if ((flags & 0x60) != 0 || !any_window) return 0;
    const int play_animation = flags & 1;
    TimerHoldFlag = 1;
    if (play_animation != 0) {
        if (special_chassis) {
            g_FX.play(0);
        } else {
            if (VehPrototypes[Vehs[second_selection].proto_id].weapon_id == 0) {
                g_FX.play(1);
            }
            // Native 0x0050558A..0x00505842 tests independently; each
            // condition reloads after any preceding FX call.
            if (VehPrototypes[Vehs[second_selection].proto_id].weapon_id == 1) {
                g_FX.play(0x4, Vehs[second_selection].x,
                          Vehs[second_selection].y);
            }
            if (VehPrototypes[Vehs[second_selection].proto_id].weapon_id == 2) {
                g_FX.play(0x49, Vehs[second_selection].x,
                          Vehs[second_selection].y);
            }
            if (VehPrototypes[Vehs[second_selection].proto_id].weapon_id == 3) {
                g_FX.play(0x4a, Vehs[second_selection].x,
                          Vehs[second_selection].y);
            }
            if (VehPrototypes[Vehs[second_selection].proto_id].weapon_id == 4) {
                g_FX.play(0x59, Vehs[second_selection].x,
                          Vehs[second_selection].y);
            }
            if (VehPrototypes[Vehs[second_selection].proto_id].weapon_id == 5) {
                g_FX.play(0x4c, Vehs[second_selection].x,
                          Vehs[second_selection].y);
            }
            if (VehPrototypes[Vehs[second_selection].proto_id].weapon_id == 6) {
                g_FX.play(0x4d, Vehs[second_selection].x,
                          Vehs[second_selection].y);
            }
            if (VehPrototypes[Vehs[second_selection].proto_id].weapon_id == 7) {
                g_FX.play(0x4e, Vehs[second_selection].x,
                          Vehs[second_selection].y);
            }
            if (VehPrototypes[Vehs[second_selection].proto_id].weapon_id == 8) {
                g_FX.play(0x4f, Vehs[second_selection].x,
                          Vehs[second_selection].y);
            }
            if (VehPrototypes[Vehs[second_selection].proto_id].weapon_id == 9) {
                g_FX.play(0x50, Vehs[second_selection].x,
                          Vehs[second_selection].y);
            }
            if (VehPrototypes[Vehs[second_selection].proto_id].weapon_id == 10) {
                g_FX.play(0x51, Vehs[second_selection].x,
                          Vehs[second_selection].y);
            }
            if (VehPrototypes[Vehs[second_selection].proto_id].weapon_id == 11) {
                g_FX.play(0x58, Vehs[second_selection].x,
                          Vehs[second_selection].y);
            }
            if (VehPrototypes[Vehs[second_selection].proto_id].weapon_id == 12) {
                g_FX.play(0x55, Vehs[second_selection].x,
                          Vehs[second_selection].y);
            }
            if (VehPrototypes[Vehs[second_selection].proto_id].weapon_id == 13) {
                g_FX.play(0x56, Vehs[second_selection].x,
                          Vehs[second_selection].y);
            }
            if (VehPrototypes[Vehs[second_selection].proto_id].weapon_id == 14) {
                g_FX.play(0x57, Vehs[second_selection].x,
                          Vehs[second_selection].y);
            }
            if (Vehs[saved_selection].proto_id == BSC_SPORE_LAUNCHER) {
                g_FX.play(0x5a, Vehs[second_selection].x,
                          Vehs[second_selection].y);
            }
        }
    }

    unsigned long last_tick = timeGetTime();
    // Native 0x00505533..0x00505546 reloads the unsigned header word at
    // every signed frame latch, after initialization and callback work.
    for (int frame = 0;
         frame < (flic_mode ? (animation.frame_count() >> 1) : 16);
         ++frame) {
        const bool skip_multiplayer_frame = IsMultiplayerNet &&
                                            play_animation != 0 &&
                                            (frame & 1ul) == 0;
        if (!skip_multiplayer_frame) {
            for (int slot = 0; slot < 8; ++slot) {
                if (!active_windows[slot]) continue;
                MapWin *window = MapWinTable[slot];
                Buffer *destination =
                    static_cast<Buffer *>(static_cast<GraphicWin *>(window));
                int draw_type = window->zoom_offset_ + 0x10;
                if (MapWinCaptionScale < 1) {
                    SpriteScaleNumerator = draw_type;
                    SpriteScaleDenominator = 0x10;
                } else {
                    SpriteScaleNumerator = MapWinCaptionScale;
                    SpriteScaleDenominator = MapWinCaptionDivisor;
                }

                if (flic_mode) {
                    animation.decode_stored_frame();
                    window = MapWinTable[slot];
                    destination =
                        static_cast<Buffer *>(static_cast<GraphicWin *>(window));
                    draw_type = window->zoom_offset_ + 0x10;
                    const int faction = Vehs[saved_selection].faction_id;
                    working_buffer.draw_multi_table_dest(
                        destination, 0xff, pixel_x_by_slot[slot],
                        pixel_y_by_slot[slot],
                        draw_type * animation_height, frame_span << 4,
                        ExplosionColorTables[faction_color_group[faction]]);
                } else {
                    const int bank = (flags & 0x100) != 0 ? 3
                        : (flags & 0x80) != 0 ? 2
                        : (VehPrototypes[Vehs[second_selection].proto_id]
                               .reactor_id > 1
                               ? 1 : 0);
                    Sprite &sprite = (flags & 1) != 0
                        ? g_XI_BOOM_VEH_SPRITES[sprite_bank * 16 + frame]
                        : g_XF_BOOM_VEH_SPRITES[bank * 16 + frame];
                    const int table_row = (flags & 1) != 0
                        ? palette_row : ((flags & 0x100) != 0 ? 6 : 0);
                    sprite.draw_multi_table_dest(
                        destination, sprite.transparent_index(),
                        pixel_x_by_slot[slot],
                        pixel_y_by_slot[slot],
                        ExplosionColorTables[table_row]);
                }
                window = MapWinTable[slot];
                window->update(&saved_rectangles[slot], 0);
                if (IsMultiplayerNet) break;
            }
            const SHORT shift = GetAsyncKeyState(VK_SHIFT);
            const bool fast = (shift & 0xff00) != 0 ||
                              (((GameDataStateStorage.fields_.game_preferences_ & 8u) != 0) &&
                               ((flags & 0x12) == 0)) || IsMultiplayerNet;
            const int delay = fast ? 0
                : (flags & 0x12) == 0 ? 0x19 : 0x32;
            unsigned long now;
            do {
                now = timeGetTime();
                do_non_input();
            } while (static_cast<int>(now - last_tick) < delay);
            last_tick = now;

            for (int restore_slot = 0; restore_slot < 8; ++restore_slot) {
                if (!active_windows[restore_slot]) continue;
                MapWin *const window = MapWinTable[restore_slot];
                Buffer *const destination =
                    static_cast<Buffer *>(static_cast<GraphicWin *>(window));
                const bool use_redrawn_snapshot = flic_mode
                    ? (play_animation == 0)
                    : ((flags & 2) != 0 && frame > 7);
                Buffer &snapshot = use_redrawn_snapshot
                    ? redrawn_map_snapshots[restore_slot]
                    : original_screen_snapshots[restore_slot];
                snapshot.copy(destination, 0, 0,
                              pixel_x_by_slot[restore_slot],
                              pixel_y_by_slot[restore_slot],
                              snapshot.pixel_width(), snapshot.pixel_height());
                if (IsMultiplayerNet) break;
            }
        }
    }

    if (flags != 0x11) {
        for (int slot = 0; slot < 8; ++slot) {
            if (active_windows[slot]) {
                MapWinTable[slot]->update(&saved_rectangles[slot], 0);
                if (IsMultiplayerNet) break;
            }
        }
    }
    if (flic_mode && (flags & 2) != 0) {
        working_buffer.close();
        animation.close();
        // Native 0x00505C6B..0x00505C7E restores the Console Buffer clip.
        GraphicWin *const console = static_cast<GraphicWin *>(&ConsoleGlobal);
        console->set_clip(console->buffer_working_rect());
    }
    if (flic_mode && (flags & 1) != 0) {
        // Resolve the Console again after the preceding clip call.
        GraphicWin *const console = static_cast<GraphicWin *>(&ConsoleGlobal);
        console->set_clip(saved_rectangles);
    }
    if (!flic_mode || (flags & 2) != 0) {
        for (int slot = 0; slot < 8; ++slot) {
            original_screen_snapshots[slot].close();
            redrawn_map_snapshots[slot].close();
        }
    }
    if ((GetAsyncKeyState(VK_SHIFT) & 0xff00) == 0 &&
        (GameDataStateStorage.fields_.game_preferences_ & 8u) == 0 && !IsMultiplayerNet) {
        clock_wait(0x32);
    }
    TimerHoldFlag = 0;
    SpriteScaleNumerator = 1;
    SpriteScaleDenominator = 1;
    return 1;
}

// ORIGINAL: 0x00505D80 ?battle_kill@@YAHHPAHPAHPAHHH@Z 0x00505D80-0x0050612A
// RECOVERY_BINDING: v1 {"evidence": ["git:99d6e72bcd5ae3be3566222736528cb0dc57935c:src/unrecovered/00505d80.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "recovery/tasks/game-logic-seed-owner-coverage.json; preserve original extents and historical seed lessons. Ordinary owned literals have pinned content provenance, not exact original data-allocation claims."], "reason": "Recover complete destruction/artifact-capture lifetime behavior through real vehicles, prototypes, player counters and UI. Preserve native captured-vehicle state clear between active-count updates and live reads after callbacks."}
// symbol    ?battle_kill@@YAHHPAH00HH@Z
// TRIED: unit records are 0x34 bytes (0x952800 area); byte-offset casts on the declared g_* globals reproduce the (bVar1*0x20cc+sVar3)-style table indexing and the terrain-base indirection through g_0094a30c/g_0068faf0.
// TRIED: nothing further tried; MISMATCH #8 (xor vs lea, likely a zeroed- register-reuse difference like 0x00532570), similarity 0.86.
int __cdecl battle_kill(int vehicle_id, int *attacker_count_out,
                        int *defender_count_out, int *odds_out,
                        int killer_id, int credit_faction) {
    const int faction_id = Vehs[vehicle_id].faction_id;
    PlayersData[faction_id].proto_id_lost[Vehs[vehicle_id].proto_id]++;

    if (killer_id >= 0 && killer_id != vehicle_id && Vehs[killer_id].faction_id > 0) {
        if (on_map(Vehs[killer_id].x, Vehs[killer_id].y)
            && VehPrototypes[Vehs[vehicle_id].proto_id].plan == PLAN_ALIEN_ARTIFACT
            && !Chassis[VehPrototypes[Vehs[killer_id].proto_id].chassis_id].missile
            && (!is_ocean(Vehs[killer_id].x, Vehs[killer_id].y)
                || stack_check(killer_id, 8, -1, -1, -1) > 0
                || Vehs[vehicle_id].flags & 0x20)) {
            // An artifact is captured, not destroyed.
            veh_lift(vehicle_id);
            const int captured_faction = Vehs[killer_id].faction_id;
            const int prototype_id = Vehs[vehicle_id].proto_id;
            Vehs[vehicle_id].dmg_incurred = 0;
            Vehs[vehicle_id].faction_id = static_cast<uint8_t>(captured_faction);
            Vehs[vehicle_id].order = 0;
            --PlayersData[faction_id].proto_id_active[prototype_id];
            Vehs[vehicle_id].state &= 0xFCFFBDFFU;
            PlayersData[captured_faction].proto_id_active[prototype_id] =
                static_cast<uint8_t>(PlayersData[captured_faction].proto_id_active[prototype_id] + 1);
            veh_drop(vehicle_id, Vehs[killer_id].x, Vehs[killer_id].y);
            veh_skip(vehicle_id);
            if (defender_count_out) {
                ++*defender_count_out;
            }
            return 0;
        }
    }

    if (Vehs[vehicle_id].flags & 0x20 && credit_faction >= 0) {
        ++PlayersData[credit_faction].unk_101;
    }

    if (Vehs[vehicle_id].state & 0x800000 && killer_id >= 0
        && !(GameDataStateStorage.fields_.game_preferences_ & 0x40000)) {
        int base_id = Vehs[killer_id].home_base_id;
        if (base_id < 0) {
            base_id = base_find(Vehs[killer_id].x, Vehs[killer_id].y,
                                Vehs[killer_id].faction_id);
        }
        if (base_id >= 0) {
            PluralityDefault = 0;
            Bases[base_id].state |= 0x40000;
            PlayersData[Vehs[killer_id].faction_id].flags |= 0x1000;
            GenderDefault = Players[Vehs[killer_id].faction_id].is_leader_female;
            parse_says(6, Players[Vehs[killer_id].faction_id].name_leader, -1, -1);
            parse_set(Players[Vehs[killer_id].faction_id].noun_gender,
                      Players[Vehs[killer_id].faction_id].is_noun_plural);
            parse_says(7, Players[Vehs[killer_id].faction_id].noun_faction, -1, -1);
            interlude(6, Bases[base_id].name_string, 1, 0);
        }
    }

    // The original reads Vehs[killer_id] here without a killer_id guard.
    if (!Vehs[vehicle_id].faction_id && Vehs[killer_id].faction_id > 0 && odds_out) {
        *odds_out += (morale_veh(vehicle_id, 1, 0) + 1) * 10;
    }

    if (attacker_count_out) {
        ++*attacker_count_out;
    }
    veh_kill(vehicle_id);
    return 1;
}

/*
Purpose: Kill every vehicle in a stack after a battle.
// ORIGINAL: 0x00506130 ?battle_kill_stack@@YAXHPAHPAHPAHHH@Z 0x00506130-0x005061AE BYTE_EXACT
// size      126 bytes
// prototype void (__cdecl ?battle_kill_stack@@YAXHPAHPAHPAHHH@Z)(int vehID1, int*, int*, int*, int vehID2, int)
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00505D80
// symbol    ?battle_kill_stack@@YAXHPAH00HH@Z
//           The image spells its three int * parameters out; VC6 12.00.8168
//           back-references the repeats to PAH00 for the identical source.
// `credit_faction` is named from battle_kill's own body: at 0x00505F74 it is
// tested >= 0 and then scaled by the 0x20CC PlayerData stride to increment a
// per-faction counter at 0x0096EA64 - the faction credited with the kill.
// HOMED from src/unrecovered/00506130.cpp, 2026-09-04. Its `g_0095285a` /
// `g_00952858` short views are Vehs[].prev_veh_id_stack (+0x32) and
// Vehs[].next_veh_id_stack (+0x30).
Return Value: n/a
Status: BYTE_EXACT.
*/
void __cdecl battle_kill_stack(int veh_id, int *attacker_out,
                               int *defender_out, int *odds_out, int killer_id,
                               int credit_faction) {
    if (veh_id < 0) {
        return;
    }
    short next = Vehs[veh_id].prev_veh_id_stack;
    while (next >= 0) {
        veh_id = next;
        next = Vehs[veh_id].prev_veh_id_stack;
    }
    while (veh_id >= 0) {
        int next_id = Vehs[veh_id].next_veh_id_stack;
        const int killed = battle_kill(veh_id, attacker_out, defender_out,
                                       odds_out, killer_id, credit_faction);
        if (killed != 0) {
            if (killer_id > veh_id) {
                --killer_id;
            }
            if (next_id > veh_id) {
                --next_id;
            }
        }
        veh_id = next_id;
    }
}

/*
Purpose: Resolve a stack's worth of chaff deaths after a battle.
// ORIGINAL: 0x005061B0 ?battle_kill_chaff@@YAXHPAHPAHPAHHH@Z 0x005061B0-0x005062A2 BYTE_EXACT
// size      242 bytes
// prototype void (__cdecl ?battle_kill_chaff@@YAXHPAHPAHPAHHH@Z)(int vehID, int*, int*, int*, int, int)
// callers   ?   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00505D80
// symbol    ?battle_kill_chaff@@YAXHPAH00HH@Z
//           The image spells its three int * parameters out; VC6 12.00.8168
//           back-references the repeats to PAH00 for the identical source.
// HOMED from src/unrecovered/005061b0.cpp, 2026-09-04. Its `g_9528xx` short
// views are Vehs[] members: +0x32 prev_veh_id_stack, +0x30 next_veh_id_stack,
// +0x0A proto_id, +0x0E faction_id, +0x10 dmg_incurred. Its `g_9ab89x` byte
// views are VehPrototype members at +0x2A (plan) and +0x27 (reactor_id).
// TRIED: the direct transcription, landed at first compiling candidate given
//        batch size - mnemonic similarity 0.93, first divergence a `movsx` the
//        rebuilt body splits into two `mov`s around the record base (#25).
Return Value: n/a
Status: MISMATCH.
*/
void __cdecl battle_kill_chaff(int veh_id, int *attacker_out,
                               int *defender_out, int *odds_out, int killer_id,
                               int credit_faction) {
    if (veh_id < 0) {
        return;
    }
    int idx = veh_id;
    short next = Vehs[idx].prev_veh_id_stack;
    while (next >= 0) {
        idx = next;
        next = Vehs[idx].prev_veh_id_stack;
    }
    if (idx < 0) {
        return;
    }
    int cur = idx;
    for (;;) {
        const int proto_id = Vehs[cur].proto_id;
        int next_id = Vehs[cur].next_veh_id_stack;
        int pct;
        if (VehPrototypes[proto_id].plan == PLAN_ALIEN_ARTIFACT) {
            pct = 1;
        } else {
            pct = range(VehPrototypes[proto_id].reactor_id, 1, 100) * 10;
        }
        const int left = pct - Vehs[cur].dmg_incurred;
        if (left < 0 || (left <= 0x270f && left <= 0) ||
            Vehs[cur].faction_id == 0) {
            if (battle_kill(cur, attacker_out, defender_out, odds_out,
                            killer_id, credit_faction) != 0) {
                if (killer_id > cur) {
                    --killer_id;
                }
                if (next_id > cur) {
                    --next_id;
                }
            }
        }
        cur = next_id;
        if (next_id < 0) {
            break;
        }
    }
}

// The battle-resolution messages. Each is the (label, icon) pair a
// NetMsgPrimary.pop takes: the label is the phrase id key, the icon the small
// portrait drawn beside the line.
// RECOVERY_OBJECT: v2 0x0068A0A8 0x0068A0AF ?VehLabelMORALE@@3PADA 0x005062B0
char VehLabelMORALE[] = "MORALE";
// RECOVERY_OBJECT: v2 0x0068A098 0x0068A0A6 ?VehIconBATWON_SM_PCX@@3PADA 0x005062B0
char VehIconBATWON_SM_PCX[] = "batwon_sm.pcx";
// RECOVERY_OBJECT: v2 0x0068A0C0 0x0068A0C8 ?VehLabelMORALE2@@3PADA 0x005062B0
char VehLabelMORALE2[] = "MORALE2";
// RECOVERY_OBJECT: v2 0x0068A0B0 0x0068A0BE ?VehIconBATWON2_SM_PCX@@3PADA 0x005062B0
char VehIconBATWON2_SM_PCX[] = "batwon_sm.pcx";

/*
Purpose: Bump a vehicle up a morale step, and say so.
// ORIGINAL: 0x005062B0 ?promote@@YAXH@Z 0x005062B0-0x0050648B BYTE_EXACT
// size      475 bytes
// prototype void (__cdecl ?promote@@YAXH@Z)(int vehID)
// callers   1   call targets   5
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0047A890 0x005C0E40 0x00625E50 0x00625EC0 0x0064601D
// HOMED from src/recovered/units/005062b0.cpp, 2026-09-04. Its `g_tbl1` /
// `g_tbl2` scaffolding is Vehs[] / VehPrototypes[] and its `g_9ab88x` byte
// views are VehPrototype members at +0x24 (chassis_id), +0x25 (weapon_id),
// +0x26 (armor_id), +0x2A (plan).
Return Value: n/a
Status: MISMATCH.
*/
void __cdecl promote(int veh_id) {
    if (Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].missile != 0) {
        return;
    }

    if (!(VehPrototypes[Vehs[veh_id].proto_id].plan < 8 ||
          Armor[VehPrototypes[Vehs[veh_id].proto_id].armor_id].defense_rating > 1)) {
        return;
    }
    if (morale_veh(veh_id, 1, 0) >= 6) {
        return;
    }
    if (Vehs[veh_id].morale >= 6) {
        return;
    }

    if (morale_veh(veh_id, 1, 0) > 2) {
        if (rand() % 2 != 0) {
            return;
        }
    }

    const int morale_before = morale_veh(veh_id, 1, 0);
    ++Vehs[veh_id].morale;
    if (Vehs[veh_id].faction_id != ConsoleGlobal.faction_id_) {
        return;
    }
    if (!((1 << Vehs[veh_id].faction_id) & GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_)) {
        return;
    }
    if (morale_before == morale_veh(veh_id, 1, 0)) {
        return;
    }

    parse_says(1, VehPrototypes[Vehs[veh_id].proto_id].veh_name, -1, -1);

    if (Vehs[veh_id].proto_id < 0x40) {
        if (Weapon[VehPrototypes[Vehs[veh_id].proto_id].weapon_id].offense_rating < 0) {
            parse_say(0, Morale[morale_veh(veh_id, 1, 0)].name_lifecycle, -1, -1);
            NetMsgPrimary.pop(VehLabelMORALE2, 0xffffec78, 0,
                             VehIconBATWON2_SM_PCX);
            return;
        }
    }

    parse_say(0, Morale[morale_veh(veh_id, 1, 0)].name, -1, -1);
    NetMsgPrimary.pop(VehLabelMORALE, 0xffffec78, 0, VehIconBATWON_SM_PCX);
}

/*
Purpose: Determine whether any enemy naval transports are carrying land units within range to
         attack the specified base. If so, set the units to move towards the base.
// ORIGINAL: 0x00506490 ?invasions@@YAXH@Z 0x00506490-0x00506646 BYTE_EXACT
// TRIED: call structure already matches exactly - `osmx calls --all`
//            lists veh_cargo, stack_check, four abs() (vector_dist/x_dist
//            inlined) and speed, in the image's own order, so vector_dist's
//            4-arg overload IS inlining here despite being declared plain
//            (not `inline`) in map.h; the divergence is pure register
//            allocation/spill-slot choice across a big function (7 locals:
//            base_x, base_y, faction_id_base, i, veh_x, veh_y, proximity),
//            not a missing inline or a chained-condition shape. Best flag
//            set /O2 /Gy /GR- /Oy- /GX, 0.746 similar, 11/143 raw. map.h is
//            out of scope for this batch regardless.
// size      438 bytes
// prototype void (__cdecl ?invasions@@YAXH@Z)(int baseID)
// callers   1   call targets   4
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005B9580 0x005C1540 0x005C1760 0x00644F3A
Return Value: n/a
Status: Complete
*/
void __cdecl invasions(int base_id) {
    int base_x = Bases[base_id].x;
    int base_y = Bases[base_id].y;
    uint32_t faction_id_base = Bases[base_id].faction_id_current;
    for (int i = 0; i < GameDataStateStorage.fields_.vehicle_current_count_; i++) {
        uint32_t veh_faction_id = Vehs[i].faction_id;
        if (veh_faction_id && !is_human(veh_faction_id) && veh_faction_id != faction_id_base
            && !(PlayersData[veh_faction_id].diplo_treaties[faction_id_base] & DTREATY_TREATY)) {
            if (is_ocean(Vehs[i].x, Vehs[i].y) && veh_cargo(i) && stack_check(i, 3, TRIAD_LAND, -1, -1)) {
                int proximity = vector_dist(base_x, base_y, Vehs[i].x, Vehs[i].y);
                if (proximity <= (int)speed(i, false)) {
                    Vehs[i].order = ORDER_MOVE_TO;
                    Vehs[i].waypoint_x[0] = base_x;
                    Vehs[i].waypoint_y[0] = base_y;
                }
            }
        }
    }
}

// ORIGINAL: 0x00506650 ?interceptor@@YAHHHHH@Z 0x00506650-0x00506A53
// RECOVERY_BINDING: v1 {"evidence": ["git:99d6e72bcd5ae3be3566222736528cb0dc57935c:src/unrecovered/00506650.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "recovery/tasks/game-logic-seed-owner-coverage.json; preserve original extents and historical seed lessons. Ordinary owned literals have pinned content provenance, not exact original data-allocation claims."], "reason": "Recover full interception selection, tie-breaking, callback-bearing presentation and motion through genuine vehicles, rules, base lookup, Console and NetMsg owners."}
// TRIED: local var/register order (early prologue push/xor order diverges at insn #5); structure and calls match Ghidra decompile faithfully
int __cdecl interceptor(int faction_id, int opposing_faction,
                        int target_x, int target_y) {
    int best_vehicle = -1;
    int best_score = 0;
    for (int vehicle_id = 0; vehicle_id < GameDataStateStorage.fields_.vehicle_current_count_; ++vehicle_id) {
        if (Vehs[vehicle_id].faction_id == faction_id
            && get_triad(vehicle_id) == TRIAD_AIR
            && has_abil(Vehs[vehicle_id].proto_id, ABL_AIR_SUPERIORITY)
            && (base_who(Vehs[vehicle_id].x, Vehs[vehicle_id].y) >= 0
                || bit_at(Vehs[vehicle_id].x, Vehs[vehicle_id].y) & BIT_AIRBASE
                || stack_check(vehicle_id, 6, ABL_CARRIER, -1, -1))
            && patrol_dist(Vehs[vehicle_id].x, Vehs[vehicle_id].y, target_x, target_y) <= 2) {
            int weight = 1;
            const int proto_id = Vehs[vehicle_id].proto_id;
            const int8_t offense = get_proto_offense_rating(proto_id);
            if (offense >= 1) {
                weight = offense;
            }
            // stand-in: a function pointer keeps this range() out of line, as the image calls it.
            int (__cdecl *const range_fn)(int, int, int) = range;
            const int power = VehPrototypes[proto_id].plan == PLAN_ALIEN_ARTIFACT
                ? 1 : range_fn(VehPrototypes[proto_id].reactor_id, 1, 100) * 10;
            weight *= range(power - Vehs[vehicle_id].dmg_incurred, 0, 9999);
            if (weight >= best_score) {
                best_score = weight;
                best_vehicle = vehicle_id;
            }
        }
    }
    if (best_vehicle < 0) {
        return 0;
    }
    if (faction_id == ConsoleGlobal.faction_id_ || opposing_faction == ConsoleGlobal.faction_id_) {
        parse_says(0, Players[faction_id].adj_name_faction, -1, -1);
        int x = Vehs[best_vehicle].x;
        int y = Vehs[best_vehicle].y;
        parse_says(1, Bases[base_find(x, y)].name_string, -1, -1);
        ConsoleGlobal.focus(x, y, faction_id);
        NetMsgPrimary.pop("AIRSCRAMBLE", 5000, 0, 0);
        // Animate the interceptor one step at a time toward the target.
        while (patrol_dist(x, y, target_x, target_y)) {
            int best_direction = -1;
            int best_distance = 999;
            int next_x;
            int next_y;
            for (int direction = 0; direction < 8; ++direction) {
                const int x_step = wrap_map_x(VehMoveDeltaX[direction] + x);
                const int y_step = VehMoveDeltaY[direction] + y;
                if (on_map(x_step, y_step)) {
                    const int distance = patrol_dist(x_step, y_step, target_x, target_y);
                    if (distance < best_distance) {
                        best_distance = distance;
                        best_direction = direction;
                        next_x = x_step;
                        next_y = y_step;
                    }
                }
            }
            if (best_direction < 0) {
                break;
            }
            veh_scoot(best_vehicle, x, y, best_direction, 0);
            x = next_x;
            y = next_y;
        }
    }
    veh_drop(veh_lift(best_vehicle), target_x, target_y);
    Vehs[best_vehicle].order = 0;
    Vehs[best_vehicle].state &= 0xFCFFBDFF;
    return 1;
}

/*
Purpose: Add a direction's tile offset to a vehicle's coordinates.
// ORIGINAL: 0x00506A60 ?battle_fight@@YAHHHHHPAH@Z 0x00506A60-0x00506AEB BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason":"Use the complete shared radius-offset owners for nonzero mode rather than truncated eight-element duplicates.","evidence":["Pinned506A70/506AB8 index66EFBC/66F440 directly; these are the existing289-dword RadiusOffsetX/Y owners in map.cpp.","Pinned action_destroy4CABEF..4CACFD supplies mode1 indices1..24; preserve unchecked direct indexing and the original single-wrap coordinate adjustment."]}
// size      139 bytes
// prototype int (__cdecl ?battle_fight@@YAHHHHHPAH@Z)(int vehID, int dir, int mode, int allow_interceptor, int *odds_out)
// callers   6   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00506AF0
// HOMED from src/unrecovered/00506a60.cpp, 2026-09-04. Its `g_00952828`
// short views are Vehs[].x (+0x00) and Vehs[].y (+0x02).
// `mode` is named from what it does here and in the seven-parameter body:
// a zero/nonzero flag selecting ordinary directions or full radius offsets. `allow_interceptor` and
// `odds_out` are named from the seven-parameter body - the first gates its
// call to interceptor (0x00506650), the second is the null-tested pointer it
// stores the odds word through at 0x005070A2.
Return Value: whatever the seven-parameter battle_fight returns.
Status: MISMATCH.
*/

// The genuine seven-parameter combat owner remains in veh.cpp.
// Its full original span and historical seed lessons remain source-owned there.
int __cdecl battle_fight(int veh_id, int dir, int x, int y, int src_mode,
                         int allow_interceptor, int *odds_out);

int __cdecl battle_fight(int veh_id, int dir, int mode, int allow_interceptor,
                         int *odds_out) {
    int x;
    if (mode != 0) {
        x = RadiusOffsetX[dir];
    } else {
        x = VehMoveDeltaX[dir];
    }
    const int val_x = wrap_map_x(Vehs[veh_id].x + x);

    const int y = mode != 0 ? RadiusOffsetY[dir] : VehMoveDeltaY[dir];
    const int val_y = Vehs[veh_id].y + y;

    return battle_fight(veh_id, dir, val_x, val_y, mode, allow_interceptor,
                        odds_out);
}

#include <stdint.h>
#include <string.h>

#include "battlewin.h"
#include "council.h"

// One-level HP helper (chunk 2). Use it only where the image keeps proto_power's
// inner range(reactor, 1, 100) as `call 0x422F00` and inlines the outer
// 0..9999 clamp (0x5087B5, 0x508A81, 0x508DFE, 0x508F21). Everywhere else the
// expression is written out.
static inline int veh_cur_hp(int veh_id) {
    return range(proto_power(veh_id) - Vehs[veh_id].dmg_incurred, 0, 9999);
}

// Defined later in battle.cpp (0x0050B970 and 0x0050B9C0). These declarations
// are needed because battle_fight now calls them directly.
int __cdecl popp(const char *label, const char *body, int(__cdecl *callback)());
void __cdecl veh_battle_msg(char *label, char *icon);

// ---------------------------------------------------------------------------

// ORIGINAL: 0x00506AF0 ?battle_fight@@YAHHHHHHHPAH@Z 0x00506AF0-0x0050B893 FILE
// RECOVERY_BINDING: v1 {"evidence": ["image:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x00506AF0-0x0050B893", "git:0b1a6229e8c4bf0dfd2193f776e65a613f7ca0c5:src/unrecovered/00506af0.cpp original identity/span and complete seed history", "source:src/veh.cpp existing five-argument wrapper, seven-argument callers and typed combat ownership", "native:507D83,507EC7,50888F,508C32 write the real ConsoleStatusWin member at +0x15D4", "source:src/statuswin.h existing field_15D4_ and narrow source-owned setter", "source:src/game.h IsMultiplayerPBEM existing 0x0093A95C owner", "native:50AAF1-50AB18 keeps the last valid stack predecessor before the negative sentinel"], "reason": "Recover complete combat resolution in the existing vehicle owner using real vehicle, faction, base, map, network and UI objects. Preserve original branch/callback order, unsafe paths, full span and historical seed lessons. The native -0x138 string scratch capacity and malformed-name frame corruption effects remain unresolved restoration obligations; the provisional 0xA0 local is not an exact source-array claim."}
// symbol    ?battle_fight@@YAHHHHHHHPAH@Z
// size      19875 bytes
// prototype int (__cdecl ?battle_fight@@YAHHHHHHHPAH@Z)(int vehID, int, int xCoord, int yCoord, int, int, int*)
// callers   2   call targets   102
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00405140 0x004055A0 0x00415DD0 0x00421670 0x00421B20 0x00421B40 0x00422C50 0x00422F00 0x0045D310 0x0046AF40 0x0046AF90 0x0046B040 0x0046B190 0x004712A0 0x00477210 0x0047A890 0x0048BEE0 0x0048C0A0 0x004B8830 0x004B8970 0x004B9EA0 0x004BA650 0x004BE650 0x004E3A50 0x004E3B80 0x004E3D50 0x004E5250 0x004E5FE0 0x004F80D0 0x00500120 0x00500130 0x005001E0 0x00500250 0x005002F0 0x00500B60 0x00501DA0 0x005044D0 0x00504AA0 0x00505D80 0x00506130 0x005062B0 0x00506490 0x00506650 0x0050B8A0 0x0050B8D0 0x0050B8F0 0x0050B910 0x0050B930 0x0050B970 0x0050B9A0 0x0050B9C0 0x0050B9E0 0x0050BA00 0x0050C2E0 0x0050DD90 0x005108A0 0x00514880 0x005230E0 0x0053A030 0x0055A150 0x0055B870 0x0055EEE0 0x0055F770 0x00579A30 0x0057D7D0 0x00581170 0x00591B10 0x00591D60 0x00591E50 0x00592140 0x00593220 0x0059E5D0 0x005AC060 0x005B3380 0x005B57D0 0x005B8AF0 0x005B8E10 0x005B8EE0 0x005B9510 0x005B9580 0x005BF1F0 0x005BFE90 0x005C0B00 0x005C0DB0 0x005C0E40 0x005C1540 0x005C1760 0x005C1D20 0x005C8D40 0x005D5A70 0x005FCBB0 0x005FD120 0x006169A0 0x00625E30 0x00625E50 0x00625EC0 0x006262F0 0x006453E0 0x00645460 0x00645470 0x0064601D 0x0064FC88
// indirect  0x005089D9 0x00508C43
// TRIED: mismatch #3 - this is a heavy approximation of a 19875-byte, ~2400-line-of-pseudocode combat resolver. Only the arty/declare-war guard and the offense/defense value clamp (attacker/defender via Vehs[]/VehPrototypes[] at 0x952828/0x9AB868, stride 0x34) are transcribed with real confidence; best_defender()'s argument order is guessed, the a6==0 "odds only" formula is reconstructed from the Ghidra hypothesis rather than the raw disassembly, and the a6!=0 combat-execution path (auto-war declaration, diplomacy-table updates at 0x96C9F8 stride 0x20CC, nerve-gas prompts, combat log) is replaced with a plain battle_compute()+morale_veh() call rather than transcribed.
int __cdecl battle_fight(int veh_id, int dir, int target_x, int target_y,
                         int mode, int allow_interceptor, int *odds_out) {
    // [ebp+0x08] veh_id      attacker. Reused 0x50B236.. as dir_i.
    // [ebp+0x0c] dir         attack direction (withdraw scan 0x5095CC). Reused
    //                        as old_def_faction (0x509753) and atk_left (0x5099DB).
    // [ebp+0x18] mode        incoming source_mode. Rewritten at 0x506B60 with the
    //                        combat mode (bit0 artillery, bit1 duel, |=0x1A air).
    //                        Reused as bombard_boomed (0x50880A) and
    //                        damaged_count (0x50A45D).
    // [ebp+0x1c] allow_interceptor  live until 0x50700F. Reused as nerve_state
    //                        (0x5071DE), the pact-sweep tile (0x50781C) and display
    //                        (0x5083F2..), plus compiler spills (0x507A4B, 0x507BD2).
    // [ebp+0x20] odds_out    odds-only path only. Reused as rx (0x509422) and
    //                        atk_id_adj (0x50AAE8). half_x (0x50A129) is a CSE.
    int pact_report = 0;    // [ebp-0x44] 1 VENDETTAREPORT, 2 PACTATTACKED, 3 PACTATTACKING
    int disengaged = 0;     // [ebp-0x64] defender withdrew (0x509979)
    int local_base = 0;     // [ebp-0x84] alien hit a LOCAL base -> interlude at the end
    int intercepted = 0;    // [ebp-0x34] interceptor() result
    int captured = 0;       // [ebp-0x50] marine detachment captured the stack (0x50991A)
    int def_faction;        // [ebp-0x0c]
    int att_faction;        // [ebp-0x10]
    int att_strength;       // [ebp-0x14] battle_compute offense out
    int def_id;             // [ebp-0x18] best defender; bombard: stack walker
    int def_strength;       // [ebp-0x1c] battle_compute defense out
    int base_id;            // [ebp-0x20] base_at(target) or -1
    int move_charge;        // [ebp-0x24] movement charged for the attack
    int retreat;            // [ebp-0x24] withdraw HP budget (0x508C9F)
    int scan_dir;           // [ebp-0x24] withdraw scan index (0x509400)
    int src_y;              // [ebp-0x28] attacker tile y
    int src_x;              // [ebp-0x2c] attacker tile x
    int booty_count;        // [ebp-0x30] battle_kill 3rd out
    int ry;                 // [ebp-0x34] withdraw candidate y
    int kill_count;         // [ebp-0x38] battle_kill 2nd out
    int nerve_gas;          // [ebp-0x3c] nerve-gas flag; 0x50AE3D reuses it as the
                            //   citizens-killed count (the same nonzero test follows)
    int stack_id;           // [ebp-0x40] copy of best_defender(), then the target
                            //   stack (0x50AC08), then the neighbour walker (0x50B276)
    int single;             // [ebp-0x48] one-exchange duel flag
    int att_bit;            // [ebp-0x4c] 1 << att_faction
    int captor;             // [ebp-0x50] attacker faction during the capture (0x509758)
    int last_hit;           // [ebp-0x54] 0 / 1 attacker hit last / 2 defender hit last
    int husk_energy;        // [ebp-0x58] battle_kill 4th out
    int def_hp;             // [ebp-0x5c] defender HP before combat
    int att_hp;             // [ebp-0x60] attacker HP before combat
    int def_step;           // [ebp-0x68] 1, or the defender's reactor (psi combat):
                            //   HP per hit to the defender and the attacker's odds factor
    int round_div;          // [ebp-0x6c] 3, or 0 for missiles / net game
    int atk_step;           // [ebp-0x70] 1, or the attacker's reactor (psi combat)
    int best_dir;           // [ebp-0x74] withdraw direction (-1 none)
    int any_visible;        // [ebp-0x78] attacker or defender visible to the local faction
    int marine;             // [ebp-0x80] marine-detachment HP budget
    int zoom_refresh;       // [ebp-0x88] net-game zoom refresh latch
    int best_angle;         // [ebp-0x8c] best withdraw angle score
    int att_proto;          // [ebp-0x90] Vehs[veh_id].proto_id snapshot
    int def_proto;          // [ebp-0x94] Vehs[def_id].proto_id snapshot
    char nerve_base_name[0x50];  // [ebp-0x138] up to -0xE9; base name for NERVEGAS
                                 //   ([ebp-0xE8] is the inlined say_num() buffer)
    int target_veh;         // esi: veh_at(target), then stack_fix()
    uint32_t nerve_state;   // [ebp+0x1c] attacker state & ~VSTATE_USED_NERVE_GAS
    int display;            // [ebp+0x1c] show this battle on screen
    int rx;                 // [ebp+0x20] withdraw candidate x
    int atk_id_adj;         // [ebp+0x20] veh_id adjusted for battle_kill compaction
    int old_def_faction;    // [ebp+0x0c] defender faction before the capture
    int atk_left;           // [ebp+0x0c] attacker HP after the rounds (0..9999)
    int bombard_boomed;     // [ebp+0x18] bombard display drew the boom
    int damaged_count;      // [ebp+0x18] collateral-damaged unit count
    int dir_i;              // [ebp+0x08] neighbour loop index

    // 0x506B03: initial mode
    if (mode != 0 ||
        (can_arty(Vehs[veh_id].proto_id, TRUE) &&
         Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad == TRIAD_LAND)) {
        mode = 1;
    } else {
        mode = 0;
    }

    // 0x506B6C
    src_x = Vehs[veh_id].x;
    src_y = Vehs[veh_id].y;
    att_faction = Vehs[veh_id].faction_id;
    target_veh = veh_at(target_x, target_y);
    if (can_arty(Vehs[veh_id].proto_id, TRUE) &&
        Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad == TRIAD_SEA &&
        !is_ocean(target_x, target_y)) {
        // 0x506C12
        if (stack_check(target_veh, 3, TRIAD_AIR, -1, -1) <
                stack_check(target_veh, 1, -1, -1, -1) ||
            base_who(target_x, target_y) >= 0) {
            mode = 1;
        }
    }

    // 0x506C74: the triad and missile tests read the ATTACKER's chassis
    target_veh = stack_fix(target_veh);
    if (Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad == TRIAD_AIR &&
        !Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].missile &&
        !stack_check(target_veh, 6, ABL_AIR_SUPERIORITY, -1, -1) &&
        allow_interceptor &&
        (NetDaemonNet.channels_[0].leading_words_[3] & 2)) {
        intercepted = interceptor(Vehs[target_veh].faction_id, att_faction,
                                  target_x, target_y);
    }

    // 0x506CFE
    def_id = stack_id = best_defender(target_veh, veh_id, mode);
    def_faction = Vehs[def_id].faction_id;
    base_id = base_at(target_x, target_y);

    // 0x506D33
    if (can_arty(Vehs[veh_id].proto_id, TRUE) && can_arty(Vehs[def_id].proto_id, TRUE)) {
        if (Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad == TRIAD_LAND ||
            mode || !is_ocean(target_x, target_y)) {
            mode |= 2;
        }
    }

    // 0x506DCE
    if (has_abil(Vehs[def_id].proto_id, ABL_AIR_SUPERIORITY) &&
        Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad == TRIAD_AIR &&
        !Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].missile &&
        !Chassis[VehPrototypes[Vehs[def_id].proto_id].chassis_id].missile &&
        Chassis[VehPrototypes[Vehs[def_id].proto_id].chassis_id].triad == TRIAD_AIR) {
        mode |= 0x1A;
    }

    // 0x506E50: movement charge
    if ((mode & 3) && mode < 4) {
        move_charge = veh_moves(veh_id);
    } else {
        move_charge = Rules.move_rate_roads;
    }

    // 0x506E95
    battle_compute(veh_id, def_id, &att_strength, &def_strength, mode);

    // 0x506EAB: hit points (the inner range() is the out-of-line call here)
    att_hp = range(proto_power(veh_id) - Vehs[veh_id].dmg_incurred, 0, 9999);
    def_hp = range(proto_power(def_id) - Vehs[def_id].dmg_incurred, 0, 9999);

    // 0x506F6D: psi factors
    if (Weapon[VehPrototypes[Vehs[veh_id].proto_id].weapon_id].offense_rating >= 0 &&
        Armor[VehPrototypes[Vehs[def_id].proto_id].armor_id].defense_rating >= 0) {
        def_step = 1;
    } else {
        def_step = VehPrototypes[Vehs[def_id].proto_id].reactor_id;
    }
    if (Weapon[VehPrototypes[Vehs[veh_id].proto_id].weapon_id].offense_rating >= 0 &&
        Armor[VehPrototypes[Vehs[def_id].proto_id].armor_id].defense_rating >= 0) {
        atk_step = 1;
    } else {
        atk_step = VehPrototypes[Vehs[veh_id].proto_id].reactor_id;
    }

    // 0x507007: odds-only query (proto_power fully inlined here, range included)
    if (!allow_interceptor) {
        int att_odds = att_hp * def_step * att_strength / proto_power(veh_id);
        int def_odds = def_hp * atk_step * def_strength / proto_power(def_id);
        if (odds_out) {
            *odds_out = def_id;
        }
        return ((mode & 1) ? 4 : 8) * att_odds / (def_odds + 1);
    }

    // 0x5070C6: automatic war declaration
    att_bit = 1 << att_faction;
    if ((GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & att_bit) &&
        VehPrototypes[Vehs[veh_id].proto_id].plan != PLAN_INFO_WARFARE &&
        (VehPrototypes[Vehs[def_id].proto_id].plan != PLAN_ALIEN_ARTIFACT || base_id >= 0) &&
        (NetDaemonNet.channels_[0].leading_words_[3] & 1) &&
        (!IsMultiplayerNet || !intercepted)) {
        if (break_treaty(att_faction, def_faction,
                         DTREATY_PACT | DTREATY_TREATY | DTREATY_COMMLINK)) {
            return 2;
        }
    }

    // 0x507132: nerve gas
    nerve_gas = 0;
    if (has_abil(Vehs[veh_id].proto_id, ABL_NERVE_GAS) &&
        Weapon[VehPrototypes[Vehs[veh_id].proto_id].weapon_id].offense_rating >= 0 &&
        Armor[VehPrototypes[Vehs[def_id].proto_id].armor_id].defense_rating >= 0) {
        // 0x5071A7: in a net game without bit 0 this skips the clear and synch_veh
        // but still runs the 0x5073BC test
        if (!IsMultiplayerNet || (NetDaemonNet.channels_[0].leading_words_[3] & 1)) {
            nerve_state = Vehs[veh_id].state & ~VSTATE_USED_NERVE_GAS;
            Vehs[veh_id].state = nerve_state;
            if (att_faction == ConsoleGlobal.faction_id_ &&
                (GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & att_bit)) {
                // 0x5071F0
                parse_says(0, VehPrototypes[Vehs[veh_id].proto_id].veh_name, -1, -1);
                parse_say(1, Ability[abil_index(ABL_NERVE_GAS)].name, -1, -1);
                g_BattleWin.stop_timer();
                if (popp(ScriptPopupFile, "USENERVE", 0, "biohazd_sm.pcx", 0)) {
                    Vehs[veh_id].state |= VSTATE_USED_NERVE_GAS;
                }
                g_BattleWin.pulse_timer();
            } else if (!(att_bit & GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_)) {
                // 0x507295: AI attacker policy
                int def_human = (1 << def_faction) &
                                GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_;
                if ((PlayersData[att_faction].diplo_treaties[def_faction] & DTREATY_ATROCITY_VICTIM) ||
                    (PlayersData[att_faction].flags & PFLAG_COMMIT_ATROCIT_WANTONLY) ||
                    (def_human &&
                     GameDataStateStorage.fields_.council_resolution_count_[PROP_REPEAL_UN_CHARTER] >
                         GameDataStateStorage.fields_.council_resolution_count_[PROP_REINSTATE_UN_CHARTER])) {
                    Vehs[veh_id].state = nerve_state | VSTATE_USED_NERVE_GAS;
                } else if (def_human &&
                           (PlayersData[att_faction].diplo_treaties[def_faction] & DTREATY_WANT_REVENGE)) {
                    int rivalry;
                    if ((GameDataStateStorage.fields_.game_rules_ & RULES_INTENSE_RIVALRY) &&
                        (PlayersData[def_faction].ranking > PlayersData[att_faction].ranking ||
                         PlayersData[def_faction].diplo_unk4[att_faction] >
                             PlayersData[att_faction].diplo_unk4[def_faction])) {
                        rivalry = (PlayersData[att_faction].ai_fight + 1) * 3;
                    } else {
                        rivalry = 0;
                    }
                    if (PlayersData[def_faction].integrity_blemishes + rivalry >= 6) {
                        Vehs[veh_id].state = nerve_state | VSTATE_USED_NERVE_GAS;
                    }
                }
            }
            // 0x5073A7
            if (IsMultiplayerNet) {
                synch_veh(veh_id);
            }
        }
        // 0x5073BC
        if ((NetDaemonNet.channels_[0].leading_words_[3] & 2) &&
            (Vehs[veh_id].state & VSTATE_USED_NERVE_GAS)) {
            nerve_gas = 1;
        }
    }

    // 0x5073D8
    if (base_id >= 0) {
        Vehs[veh_id].state &= ~VSTATE_UNK_40000;
    }
    if (!(Vehs[veh_id].state & VSTATE_UNK_1000) &&
        (NetDaemonNet.channels_[0].leading_words_[3] & 2)) {
        // 0x507405
        if (att_faction != 0 &&
            Vehs[veh_id].proto_id != BSC_SPORE_LAUNCHER &&
            Vehs[veh_id].proto_id != BSC_MIND_WORMS) {
            int moves = range(veh_moves(veh_id), 0, Rules.move_rate_roads);
            att_strength = moves * att_strength / Rules.move_rate_roads;
            // inlined add_bat: label_get() runs before the modifier is computed
            add_bat(0, (moves - Rules.move_rate_roads) * 100 / Rules.move_rate_roads,
                    label_get(0x153));
        }
        // 0x5074DD
        if (mode != 1) {
            Vehs[veh_id].moves_expended += static_cast<uint8_t>(move_charge);
        }
    }

    // 0x5074F4
    Vehs[veh_id].state = (Vehs[veh_id].state & ~VSTATE_UNK_1000) | VSTATE_UNK_4;
    Vehs[def_id].state |= VSTATE_UNK_4;
    if (!(NetDaemonNet.channels_[0].leading_words_[3] & 2)) {
        return 0;
    }
    Vehs[veh_id].state |= VSTATE_UNK_40;

    // 0x507531: pre-combat diplomacy. Probe attackers and artifact defenders skip
    // it, act_of_aggression and the pact sweep.
    if (VehPrototypes[Vehs[veh_id].proto_id].plan != PLAN_INFO_WARFARE &&
        VehPrototypes[Vehs[def_id].proto_id].plan != PLAN_ALIEN_ARTIFACT) {
        bool notify = false;
        if (GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & (1 << def_faction)) {
            // 0x507583: the DEFENDER's view of the attacker
            uint32_t d2a = PlayersData[def_faction].diplo_treaties[att_faction];
            if (d2a & (DTREATY_TREATY | DTREATY_TRUCE)) {
                notify = true;
            } else if (((d2a & DTREATY_COMMLINK) ||
                        (PlayersData[att_faction].diplo_treaties[def_faction] & DTREATY_UNK_800)) &&
                       !(d2a & DTREATY_VENDETTA)) {
                notify = true;
            }
        }
        if (notify) {
            // 0x5075D4
            if (def_faction == ConsoleGlobal.faction_id_) {
                PluralityDefault = 0;
                GenderDefault = Players[att_faction].is_leader_female;
                parse_says(0, Players[att_faction].title_leader, -1, -1);
                GenderDefault = Players[att_faction].is_leader_female;
                PluralityDefault = 0;
                parse_says(1, Players[att_faction].name_leader, -1, -1);
                parse_says(2, get_noun(att_faction), -1, -1);
                NetMsgPrimary.pop("SURPRISE", -5000, 0, 0);
            } else if (IsMultiplayerPBEM) {
                // 0x507696
                PluralityDefault = 0;
                GenderDefault = Players[att_faction].is_leader_female;
                parse_says(0, Players[att_faction].title_leader, -1, -1);
                GenderDefault = Players[att_faction].is_leader_female;
                PluralityDefault = 0;
                parse_says(1, Players[att_faction].name_leader, -1, -1);
                parse_says(2, get_noun(att_faction), -1, -1);
                POP2("SURPRISE", 0, -1 - def_faction);
            }
            // 0x507679 / 0x507731 (each branch has its own copy)
            PlayersData[def_faction].diplo_spoke[att_faction] =
                GameDataStateStorage.fields_.turn_current_num_;
        } else {
            // 0x507744: AI defender, or a human defender that is not notified
            if ((PlayersData[att_faction].diplo_treaties[def_faction] & DTREATY_TREATY) ||
                (PlayersData[def_faction].diplo_treaties[att_faction] & DTREATY_TREATY)) {
                if (att_faction != ConsoleGlobal.faction_id_ &&
                    def_faction != ConsoleGlobal.faction_id_) {
                    if (PlayersData[ConsoleGlobal.faction_id_].diplo_treaties[def_faction] & DTREATY_PACT) {
                        pact_report = 2;
                    } else if (PlayersData[ConsoleGlobal.faction_id_].diplo_treaties[att_faction] & DTREATY_PACT) {
                        pact_report = 3;
                    } else if (GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW) {
                        pact_report = 1;
                    }
                }
            }
        }

        // 0x5077D6
        act_of_aggression(att_faction, def_faction);
        // The sweep reads the ATTACKER's tile, not the target
        Map *src_tile = map_loc(src_x, src_y);   // [ebp+0x1c]
        for (int i = 1; i < MaxPlayerNum; i++) {
            if (i == att_faction || i == def_faction) {
                continue;
            }
            if (src_tile->territory != i) {
                continue;
            }
            if (!(PlayersData[att_faction].diplo_treaties[i] & DTREATY_PACT)) {
                continue;
            }
            uint32_t d2i = PlayersData[def_faction].diplo_treaties[i];
            if ((d2i & DTREATY_VENDETTA) || !(d2i & DTREATY_COMMLINK)) {
                continue;
            }
            if (PlayersData[def_faction].mil_strength_1 <= PlayersData[i].mil_strength_1) {
                continue;
            }
            treaty_on(i, def_faction, DTREATY_WANT_TO_TALK);
            if (PlayersData[i].diplo_treaties[def_faction] & DTREATY_UNK_800000) {
                cause_friction(def_faction, i, 3);
            }
        }
    }

    // 0x5078E2: nerve gas bonus (inlined add_bat; label_get runs unconditionally)
    if (nerve_gas) {
        att_strength += att_strength / 2;
        add_bat(0, 50, label_get(0x155));
    }

    // 0x507944
    if (IsMultiplayerNet) {
        log_say("Battle Resolution!", veh_id, def_id, 0);
    }

    // 0x507964
    att_proto = Vehs[veh_id].proto_id;
    def_proto = Vehs[def_id].proto_id;
    if ((att_faction == ConsoleGlobal.faction_id_ || def_faction == ConsoleGlobal.faction_id_) &&
        !IsMultiplayerNet &&
        (Weapon[VehPrototypes[att_proto].weapon_id].offense_rating < 0 ||
         Armor[VehPrototypes[def_proto].armor_id].defense_rating < 0) &&
        tut_check2(2)) {
        TutorialWindow.tut_map("PSICOMBAT", target_x, target_y,
                               att_faction == ConsoleGlobal.faction_id_ ? veh_id : def_id,
                               0, 1, -1, -1);
    }

    // 0x507A17: difficulty corrections
    {
        int diff_floor = att_faction ? 2 : 3;   // esi
        if (PlayersData[def_faction].diff_level < diff_floor &&
            (GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & (1 << def_faction)) &&
            !(att_bit & GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_)) {
            att_strength = (PlayersData[def_faction].diff_level + 1) * att_strength / 4;
        }
        if (PlayersData[att_faction].diff_level < diff_floor &&
            (att_bit & GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_) &&
            !(GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & (1 << def_faction))) {
            att_strength = (4 - PlayersData[att_faction].diff_level) * att_strength / 2;
        }
    }

    // 0x507AD3: alien attacker
    if (att_faction == 0) {
        if (!(GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & (1 << def_faction))) {
            if (VehPrototypes[Vehs[def_id].proto_id].plan == PLAN_TERRAFORMING) {
                att_strength /= 2;
                if (!(GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW) &&
                    !IsMultiplayerNet &&
                    (Vehs[def_id].faction_id != ConsoleGlobal.faction_id_ ||
                     (Vehs[def_id].flags & (VFLAG_LURKER | VFLAG_INVISIBLE)) ==
                         (VFLAG_LURKER | VFLAG_INVISIBLE)) &&
                    !(Vehs[def_id].visibility & (1 << ConsoleGlobal.faction_id_))) {
                    att_strength /= 2;
                }
            } else {
                // 0x507B6B: PlayerData +0x04 is `ranking`, not diff_level
                att_strength = (PlayersData[def_faction].ranking + 8) * att_strength / 16;
            }
        }
        // 0x507B85
        if (base_id >= 0) {
            if (def_faction == ConsoleGlobal.faction_id_) {
                local_base = 1;
            }
            if (PlayersData[def_faction].current_num_bases <= 1) {
                att_strength /= 2;
            }
            if (GameDataStateStorage.fields_.turn_current_num_ < 50) {
                att_strength /= 2;
            }
            // 0x507BCF: inlined has_fac (its bitmask() out-params use [ebp+0x1c]/[ebp+0x20])
            if (has_fac(FAC_HEADQUARTERS, base_id, 0) &&
                PlayersData[def_faction].current_num_bases < 4) {
                att_strength /= 2;
            }
        }
    }

    // 0x507C22: early-game damping
    if (GameDataStateStorage.fields_.turn_current_num_ < 15) {
        if (att_faction == 0) {
            att_strength /= 3;
        }
        if (def_faction == 0) {
            def_strength /= 2;
        }
    }

    // 0x507C5C
    if (base_id >= 0 &&
        Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad == TRIAD_AIR) {
        Bases[base_id].state |= BSTATE_UNK_100000;
    }

    // 0x507CA4: local human odds prompts
    if ((NetDaemonNet.channels_[0].leading_words_[3] & 1) && !intercepted &&
        (static_cast<uint8_t>(att_bit) & GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_) &&
        att_faction == ConsoleGlobal.faction_id_ && mode == 0 &&
        !Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].missile &&
        !(Vehs[veh_id].state & (VSTATE_UNK_200 | VSTATE_EXPLORE))) {
        if (2 * (att_hp * def_step * att_strength) < def_hp * atk_step * def_strength) {
            // 0x507D48: BADIDEA
            g_BattleWin.set_iface_mode();
            g_BattleWin.clear();
            ConsoleStatusWin.draw(veh_id, def_id, att_strength, def_strength, 0);
            ConsoleStatusWin.set_field_15D4(1);
            PluralityDefault = 0;
            GenderDefault = Players[ConsoleGlobal.faction_id_].is_leader_female;
            parse_says(0, Players[ConsoleGlobal.faction_id_].title_leader, -1, -1);
            PluralityDefault = 0;
            GenderDefault = Players[ConsoleGlobal.faction_id_].is_leader_female;
            parse_says(1, Players[ConsoleGlobal.faction_id_].name_leader, -1, -1);
            g_BattleWin.stop_timer();
            if (popp(ScriptPopupFile, "BADIDEA", 0, "hasty_sm.pcx", 0)) {
                goto L_508358;
            }
            // 0x507E26
            g_BattleWin.pulse_timer();
            if (NetDaemonNet.channels_[0].leading_words_[3] & 2) {
                Vehs[veh_id].moves_expended -= static_cast<uint8_t>(move_charge);
            }
            Vehs[veh_id].order = ORDER_NONE;
            Vehs[veh_id].state &= 0xFCFFBDFF;
            ConsoleStatusWin.reset();
            ConsoleStatusWin.redraw();
            return 1;
        }

        // 0x507E7E: GOODIDEA
        if (!(GameDataStateStorage.fields_.game_more_preferences_ & MPREF_ADV_CONFIRM_ODDS_BF_ATTACKING)) {
            goto L_50835D;
        }
        g_BattleWin.set_iface_mode();
        g_BattleWin.clear();
        ConsoleStatusWin.draw(veh_id, def_id, att_strength, def_strength, 0);
        ConsoleStatusWin.set_field_15D4(1);
        PluralityDefault = 0;
        GenderDefault = Players[ConsoleGlobal.faction_id_].is_leader_female;
        parse_says(0, Players[ConsoleGlobal.faction_id_].title_leader, -1, -1);
        PluralityDefault = 0;
        GenderDefault = Players[ConsoleGlobal.faction_id_].is_leader_female;
        parse_says(1, Players[ConsoleGlobal.faction_id_].name_leader, -1, -1);
        // 0x507F36: inlined say_num(). "." is a direct strcat call, not says()
        clear();
        say_num(att_strength >> 8);
        strcat(StringTemp, ".");
        say_num(((att_strength & 0xFF) * 10) >> 8);
        parse_says(2, StringTemp, -1, -1);
        clear();
        say_num(def_strength >> 8);
        strcat(StringTemp, ".");
        say_num(((def_strength & 0xFF) * 10) >> 8);
        parse_says(3, StringTemp, -1, -1);

        {
            // 0x508034: these products do NOT include the psi factors
            int att_odds = range(proto_power(veh_id) - Vehs[veh_id].dmg_incurred, 0, 9999) * att_strength;   // esi
            int def_odds = range(proto_power(def_id) - Vehs[def_id].dmg_incurred, 0, 9999) * def_strength;   // edi
            // 0x5080FC: the minimum is tested once per divisor, not after each division
            for (int div = 2; div < (att_odds < def_odds ? att_odds : def_odds); div++) {
                while (att_odds && def_odds && !(att_odds % div) && !(def_odds % div)) {
                    att_odds /= div;
                    def_odds /= div;
                }
            }
            while (att_odds >= 20000 && def_odds >= 20000) {
                att_odds /= 10000;
                def_odds /= 10000;
            }
            while (att_odds >= 10000 && def_odds >= 10000) {
                att_odds /= 5000;
                def_odds /= 5000;
            }
            while (att_odds >= 2000 && def_odds >= 2000) {
                att_odds /= 1000;
                def_odds /= 1000;
            }
            while (att_odds >= 1000 && def_odds >= 1000) {
                att_odds /= 500;
                def_odds /= 500;
            }
            while (att_odds >= 200 && def_odds >= 200) {
                att_odds /= 100;
                def_odds /= 100;
            }
            while (att_odds >= 100 && def_odds >= 100) {
                att_odds /= 50;
                def_odds /= 50;
            }
            // 0x5082A2
            g_BattleWin.stop_timer();
            parse_num(0, att_odds);
            parse_num(1, def_odds);
        }
        if (popp(ScriptPopupFile,
                 (Vehs[veh_id].dmg_incurred || Vehs[def_id].dmg_incurred) ? "GOODIDEA2" : "GOODIDEA",
                 0, "hasty_sm.pcx", 0)) {
            goto L_508358;
        }
        // 0x508301
        g_BattleWin.pulse_timer();
        if (NetDaemonNet.channels_[0].leading_words_[3] & 2) {
            Vehs[veh_id].moves_expended -= static_cast<uint8_t>(move_charge);
        }
        Vehs[veh_id].order = ORDER_NONE;
        Vehs[veh_id].state &= 0xFCFFBDFF;
        ConsoleStatusWin.reset();
        ConsoleStatusWin.redraw();
        return 1;
    }
    goto L_50835D;

L_508358:
    g_BattleWin.pulse_timer();

L_50835D:
    // 0x50835D: shared post-prompt tail
    if (nerve_gas) {
        atrocity(att_faction, def_faction, base_id >= 0, 0);
        nerve_base_name[0] = '\0';
        if (base_id >= 0) {
            strcpy(nerve_base_name, Bases[base_id].name_string);
        }
    }

    // 0x5083AC
    if (Vehs[veh_id].order == ORDER_MOVE_TO) {
        Vehs[veh_id].order = ORDER_NONE;
        Vehs[veh_id].state &= 0xFCFFBDFF;
    }

    // 0x5083D6
    zoom_refresh = 0;
    if (att_faction == ConsoleGlobal.faction_id_ || def_faction == ConsoleGlobal.faction_id_ ||
        (GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW)) {
        display = 1;
    } else {
        display = 0;
    }

    // 0x508401: Vehs[].flags (+0x08) masked with 0x440, plus Vehs[].visibility (+0x27)
    if ((Vehs[veh_id].faction_id == ConsoleGlobal.faction_id_ &&
         (Vehs[veh_id].flags & (VFLAG_LURKER | VFLAG_INVISIBLE)) != (VFLAG_LURKER | VFLAG_INVISIBLE)) ||
        (Vehs[veh_id].visibility & (1 << ConsoleGlobal.faction_id_)) ||
        (Vehs[def_id].faction_id == ConsoleGlobal.faction_id_ &&
         (Vehs[def_id].flags & (VFLAG_LURKER | VFLAG_INVISIBLE)) != (VFLAG_LURKER | VFLAG_INVISIBLE)) ||
        (Vehs[def_id].visibility & (1 << ConsoleGlobal.faction_id_))) {
        any_visible = 1;
    } else {
        any_visible = 0;
    }

    // 0x50846E
    if ((PlayersData[ConsoleGlobal.faction_id_].diplo_treaties[att_faction] & DTREATY_PACT)
            ? (GameDataStateStorage.fields_.game_preferences_ & PREF_BSC_DONT_QUICK_MOVE_ALLY_VEH)
            : (GameDataStateStorage.fields_.game_preferences_ & PREF_BSC_DONT_QUICK_MOVE_ENEMY_VEH)) {
        display |= any_visible;
    }

    // 0x5084A8
    if (mode == 1) {
        // ---- 0x5084B4: artillery bombardment of the whole stack -------------
        add_goal(def_faction, 0x10, 4, target_x, target_y, -1);
        add_goal(def_faction, 0, 4, src_x, src_y, -1);
        if (base_id >= 0) {
            Bases[base_id].state |= BSTATE_UNK_200000;
            Bases[base_id].event |= BEVENT_UNK_100;
        }

        // 0x508513: inlined veh_top. On a negative def_id the image jumps to the
        // sweep without storing (the stored value would be the same -1).
        // 0x508A0D: the step is inlined veh_next_stack_id ([ebp-0x18] is the walker)
        for (def_id = veh_top(def_id); def_id >= 0; def_id = veh_next_stack_id(def_id)) {
            // 0x508550 ([ebp-0x08] rebased onto def_id)
            if (Chassis[VehPrototypes[Vehs[def_id].proto_id].chassis_id].triad == TRIAD_AIR &&
                !has_abil(Vehs[veh_id].proto_id, ABL_AIR_SUPERIORITY) &&
                base_id < 0 &&
                !(map_loc(target_x, target_y)->bit & BIT_AIRBASE)) {
                continue;
            }
            // 0x5085CE
            veh_skip(veh_id);
            battle_compute(veh_id, def_id, &att_strength, &def_strength, 1);
            int denom = range(Rules.artillery_dmg_denom * def_strength, 1, 9999999);   // ecx
            int dmg_max = range(Rules.artillery_dmg_num * att_strength / denom + 1, 1, 999);   // esi
            int dmg = bounded_crt_random(dmg_max);   // edi, then ebx
            // 0x50864B .. 0x50876A: never take the unit below `keep`
            int keep;   // esi
            if (base_id < 0 && !(map_loc(target_x, target_y)->bit & BIT_BUNKER)) {
                keep = (100 - (Chassis[VehPrototypes[Vehs[def_id].proto_id].chassis_id].triad == TRIAD_LAND
                                   ? Rules.max_pct_dmg_art_vs_units_open
                                   : Rules.max_pct_dmg_art_vs_units_sea))
                       * proto_power(def_id) + 99;
            } else {
                // 0x508712
                keep = (100 - Rules.max_pct_dmg_art_vs_unit_base_bnkr) * proto_power(def_id) + 99;
            }
            keep /= 100;   // 0x50876A
            // 0x50876F
            for (; dmg > 0; dmg--) {
                if (veh_cur_hp(def_id) - dmg >= keep) {   // 0x5087B5 calls range
                    break;
                }
            }

            // 0x5087EC
            if (display) {
                bombard_boomed = 0;
                VehDrawSelection689F2C = def_id;
                Vehs[veh_id].visibility |= static_cast<uint8_t>(1 << def_faction);
                VehDrawSelection689F28 = veh_id;
                if (!ConsoleGlobal.focus(target_x, target_y, att_faction)) {
                    draw_tile(src_x, src_y, 2);
                    draw_tile(target_x, target_y, 2);
                }
                g_BattleWin.set_iface_mode();
                g_BattleWin.clear();
                ConsoleStatusWin.draw(veh_id, def_id, att_strength, def_strength, 1);
                ConsoleStatusWin.set_field_15D4(1);
                flush_input();
                // 0x50889E: the image also tests `def_id < 0`
                if (keep == 0 || Vehs[def_id].dmg_incurred != keep || def_id < 0 ||
                    Vehs[def_id].next_veh_id_stack < 0) {
                    boom(target_x, target_y, 0);
                    bombard_boomed = 1;
                }
            }
            // 0x5088E6
            Vehs[def_id].dmg_incurred += static_cast<uint8_t>(dmg);
            Vehs[def_id].state |= VSTATE_UNK_4;
            if (VehPrototypes[Vehs[def_id].proto_id].plan == PLAN_TERRAFORMING) {
                Vehs[def_id].terraforming_turns = 0;
            }
            if (base_id < 0) {
                Vehs[def_id].state |= VSTATE_UNK_8;
            }
            // 0x508936
            if (display) {
                ConsoleStatusWin.draw(veh_id, def_id, att_strength, def_strength, 1);
                draw_tile(target_x, target_y, 2);
                VehDrawSelection689F28 = -1;
                VehDrawSelection689F2C = -1;
                if (GameDataStateStorage.fields_.game_more_preferences_ & MPREF_ADV_PAUSE_AFTER_BATTLES) {
                    g_BattleWin.stop_timer();
                    popp(ScriptPopupFile, "BATTLEPAUSE", 0, "batwon_sm.pcx", 0);
                    g_BattleWin.pulse_timer();
                    if (ConsoleGroupAction) {
                        GameDataStateStorage.fields_.game_more_preferences_ &= ~MPREF_ADV_PAUSE_AFTER_BATTLES;
                        prefs_save(0);
                    }
                } else if ((GetAsyncKeyState(VK_SHIFT) & 0xFF00) == 0   // mov dl,ah / test dl,dl
                           && !(GameDataStateStorage.fields_.game_preferences_ & PREF_ADV_FAST_BATTLE_RESOLUTION)
                           && !IsMultiplayerNet && bombard_boomed) {
                    clock_wait(200);
                }
            }
        }

        // 0x508A29: kill the units the bombardment left without HP
        for (int k = stack_fix(veh_at(target_x, target_y)); k >= 0; ) {   // esi
            if (veh_cur_hp(k) < 1) {   // 0x508A81 calls range
                stack_veh(k, 2);
                stack_kill(k);
                k = stack_fix(veh_at(target_x, target_y));
                PlayersData[att_faction].diplo_unk3[def_faction]++;
                PlayersData[att_faction].diplo_unk4[def_faction]++;
            } else {
                k = veh_next_stack_id(k);   // inlined
            }
        }
        // 0x508B12
        draw_tile_fixup(target_x, target_y, 1, 2);
        if (base_id >= 0 &&
            !(GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & (1 << def_faction))) {
            base_reset(base_id, 0);
        }
        return 0;   // 0x508B47
    }

    // ---- 0x508B50: ordinary / duel combat (mode != 1) ----------------------
    if (mode != 0) {
        add_goal(def_faction, 0x10, 2, target_x, target_y, -1);
    }
    // 0x508B6B
    if (display) {
        if (IsMultiplayerNet && TutWinBaseWindow.zoom_wait_pending_) {
            zoom_refresh = 1;
        }
        Vehs[veh_id].visibility |= static_cast<uint8_t>(1 << def_faction);
        VehDrawSelection689F28 = veh_id;
        VehDrawSelection689F2C = def_id;
        if (!ConsoleGlobal.focus(target_x, target_y, att_faction)) {
            draw_tile(src_x, src_y, 2);
            draw_tile(target_x, target_y, 2);
        }
        stack_veh(veh_id, 1);
        g_BattleWin.set_iface_mode();
        g_BattleWin.clear();
        ConsoleStatusWin.draw(veh_id, def_id, att_strength, def_strength, mode);
        ConsoleStatusWin.set_field_15D4(1);
        flush_input();
        if ((GetAsyncKeyState(VK_SHIFT) & 0xFF00) == 0 && !(mode & 1)) {   // 0x508C49
            veh_scoot(veh_id, src_x, src_y, dir, 1);
        }
        stack_put(veh_id, src_x, src_y);
        draw_tile(src_x, src_y, 2);
        draw_tile(target_x, target_y, 2);
    }

    // 0x508C99: withdraw threshold. speed(def_id) is called before speed(veh_id).
    retreat = 0;
    if (speed(def_id, 1) > speed(veh_id, 1)
        && mode == 0
        && get_offense_rating(def_id) != 0
        && !(Vehs[def_id].state & VSTATE_UNK_40)
        && get_triad(veh_id) != TRIAD_AIR
        && get_triad(def_id) != TRIAD_AIR
        && (!has_abil(Vehs[veh_id].proto_id, ABL_COMM_JAMMER)
            || has_abil(Vehs[def_id].proto_id, ABL_DISSOCIATIVE_WAVE))
        && Vehs[def_id].order != ORDER_HOLD
        && base_id < 0
        && !(map_loc(target_x, target_y)->bit & (BIT_BUNKER | BIT_AIRBASE))
        && stack_check(def_id, 1, -1, -1, -1) == 1) {
        retreat = (veh_cur_hp(def_id) + 1) / 2;   // 0x508DFE calls range
    }

    // 0x508E30: marine detachment threshold
    marine = 0;
    if (has_abil(Vehs[veh_id].proto_id, ABL_MARINE_DETACHMENT)
        && get_triad(veh_id) == TRIAD_SEA
        && is_ocean(target_x, target_y)
        && get_triad(def_id) == TRIAD_SEA
        // not a native unit: proto < 64 && (psi weapon || spore launcher)
        && !(Vehs[def_id].proto_id < MaxVehProtoFactionNum
             && (get_offense_rating(def_id) < 0
                 || Vehs[def_id].proto_id == BSC_SPORE_LAUNCHER))
        && base_id < 0) {
        marine = veh_cur_hp(def_id) * 2 / 3;   // 0x508F21 calls range
    }

    // 0x508F5E: planet buster (planet_buster2 inlined on the attacker proto)
    if (planet_buster2(Vehs[veh_id].proto_id)) {
        planet_busting(veh_id, target_x, target_y);
        WorldClimateWorldWin.field_239BC_ |= 4;
        return 0;   // 0x508FA5
    }

    // 0x508FB5
    round_div = (!is_missile(veh_id) && !IsMultiplayerNet) ? 3 : 0;
    last_hit = 0;
    if (is_missile(veh_id) || IsMultiplayerNet) {
        boom(target_x, target_y,
             2 - (range(proto_power(def_id) - Vehs[def_id].dmg_incurred, 0, 9999) != 0));
    }
    // 0x50906C
    single = ((mode & 2)
              && ((get_triad(veh_id) == TRIAD_SEA) != (get_triad(def_id) == TRIAD_SEA)))
             ? 1 : 0;

    // ---- 0x5090D7: combat rounds ---------------------------------------------
    // The two loop-top tests jump to 0x509983, i.e. L_509980 without its reload.
    for (;;) {
        if (range(proto_power(veh_id) - Vehs[veh_id].dmg_incurred, 0, 9999) == 0) break;
        if (range(proto_power(def_id) - Vehs[def_id].dmg_incurred, 0, 9999) == 0) break;

        // 0x5091A5: the attacker draws first; the compare is signed
        int roll_atk = bounded_crt_random(att_strength);   // edi
        int roll_def = bounded_crt_random(def_strength);   // edx

        if (roll_atk > roll_def) {
            // 0x5091DB: the defender takes def_step
            int dmg = static_cast<uint8_t>(
                Vehs[def_id].dmg_incurred += static_cast<uint8_t>(def_step));
            if (display && round_div &&
                dmg / (round_div * def_step) != (dmg - def_step) / (round_div * def_step)) {
                // ((dmg - def_step) / div is evaluated first and spilled to [ebp-0x74].)
                last_hit = 2;
                ConsoleStatusWin.draw(veh_id, def_id, att_strength, def_strength, mode);
                boom(target_x, target_y, 1);
                if (range(proto_power(def_id) - Vehs[def_id].dmg_incurred, 0, 9999) != 0) {
                    draw_tile(src_x, src_y, 2);
                    draw_tile(target_x, target_y, 2);
                }
                if (single) break;   // 0x5092E1 -> 0x509980
            }
            // 0x5092EA: marines
            if (marine &&
                range(proto_power(def_id) - Vehs[def_id].dmg_incurred, 0, 9999) != 0) {
                marine -= def_step;
                if (marine <= 0 && rand() % 100 < 50) goto L_50973a;
            }
            // 0x50936C: withdrawal (the defender's HP, [ebp-0x08])
            if (retreat == 0) continue;
            if (range(proto_power(def_id) - Vehs[def_id].dmg_incurred, 0, 9999) == 0) continue;
            retreat -= def_step;
            if (retreat > 0) continue;

            // 0x5093F1: pick the free tile that turns furthest from the attack.
            // rx/ry are written for every examined direction.
            best_dir = -1;
            best_angle = 0;
            for (scan_dir = 0; scan_dir < 8; scan_dir++) {
                rx = wrap_map_x(VehMoveDeltaX[scan_dir] + target_x);
                ry = VehMoveDeltaY[scan_dir] + target_y;
                if (!on_map(rx, ry)) continue;
                if (zoc_move(rx, ry, def_faction)) continue;
                int triad = get_triad(def_id);
                if (triad != TRIAD_AIR &&
                    is_ocean(rx, ry) != (triad == TRIAD_SEA)) continue;
                if (goody_at(rx, ry)) continue;
                Map *tile = map_loc(rx, ry);   // ecx, reused by anything_at below
                if ((tile->bit & BIT_FUNGUS) && (tile->climate & 0xE0) >= 0x40
                    && !has_project(SP_PHOLUS_MUTAGEN, def_faction)) {
                    int dp = Vehs[def_id].proto_id;
                    if (dp >= MaxVehProtoFactionNum) continue;
                    if (get_proto_offense_rating(dp) >= 0 && dp != BSC_SPORE_LAUNCHER) continue;
                }
                // 0x509594
                int who = anything_at(rx, ry);
                if (who >= 0 && who != def_faction &&
                    !(PlayersData[who].diplo_treaties[def_faction] & DTREATY_PACT)) continue;
                // 0x5095CC: angular distance from the reversed attack direction
                int angle = scan_dir - (dir ^ 4);
                if (angle < 0) angle = (dir ^ 4) - scan_dir;
                angle &= 7;
                if (angle > 4) angle = 8 - angle;
                if (angle > 2 && angle > best_angle) {
                    best_angle = angle;
                    best_dir = scan_dir;
                }
            }
            // 0x509612
            if (best_dir >= 0) goto L_509923;
            retreat = 0;
        } else {
            // 0x509629: the attacker takes atk_step (ties go to the defender)
            int dmg = static_cast<uint8_t>(
                Vehs[veh_id].dmg_incurred += static_cast<uint8_t>(atk_step));
            if (!display) continue;
            if (!round_div) continue;
            if (dmg / (round_div * atk_step) == (dmg - atk_step) / (round_div * atk_step)) continue;
            last_hit = 1;
            ConsoleStatusWin.draw(veh_id, def_id, att_strength, def_strength, mode);
            boom(src_x, src_y, 9);
            if (range(proto_power(veh_id) - Vehs[veh_id].dmg_incurred, 0, 9999) != 0) {
                draw_tile(src_x, src_y, 2);
                draw_tile(target_x, target_y, 2);
            }
            if (single) break;   // 0x50972F -> 0x509980
        }
    }
    goto L_509980;

    // ---- 0x50973A: the marine detachment captures the defender's stack -------
L_50973a:
    old_def_faction = Vehs[def_id].faction_id;
    captor = Vehs[veh_id].faction_id;
    if (def_id >= 0) {
        for (int c = veh_top(def_id); c >= 0; c = veh_next_stack_id(c)) {   // edi
            Vehs[c].faction_id = Vehs[veh_id].faction_id;
            parse_says(0, VehPrototypes[Vehs[c].proto_id].veh_name, -1, -1);
            int home = base_find(target_x, target_y);
            // Unguarded: Bases[-1] is read when base_find finds none
            if (Bases[home].faction_id_current == Vehs[veh_id].faction_id
                && Bases[home].population_size >= 3) {
                Vehs[c].home_base_id = static_cast<int16_t>(home);
            } else {
                Vehs[c].home_base_id = -1;
            }
            veh_skip(c);
        }
    }
    // 0x509827
    bit_set(target_x, target_y, BIT_SUPPLY_REMOVE, 1);
    owner_set(target_x, target_y, Vehs[veh_id].faction_id);
    if (Vehs[veh_id].faction_id == ConsoleGlobal.faction_id_) {
        draw_tile_fixup(target_x, target_y, 1, 2);
    } else {
        draw_tile(target_x, target_y, 2);
    }
    if (captor == ConsoleGlobal.faction_id_) {
        parse_says(1, get_adjective(old_def_faction), -1, -1);
        popp(ScriptPopupFile, "WEMARINEDETACH", 0, "navun_sm.pcx", 0);
    }
    if (old_def_faction == ConsoleGlobal.faction_id_) {
        parse_says(1, get_adjective(captor), -1, -1);
        popp(ScriptPopupFile, "THEYMARINEDETACH", 0, "navun_sm.pcx", 0);
    }
    log_say("Detachment", 0, 0, 0);
    captured = 1;   // 0x50991A
    goto L_509980;

    // ---- 0x509923: the defender withdraws to best_dir ------------------------
L_509923:
    rx = wrap_map_x(VehMoveDeltaX[best_dir] + target_x);
    ry = VehMoveDeltaY[best_dir] + target_y;
    stack_put(stack_veh(def_id, 1), rx, ry);
    disengaged = 1;

    // ---- 0x509980: outcome display -------------------------------------------
L_509980:
    atk_left = range(proto_power(veh_id) - Vehs[veh_id].dmg_incurred, 0, 9999);
    if (display || any_visible) {
        draw_tile_fixup(src_x, src_y, target_x, target_y, 2, 2);   // 0x46B040
        if (disengaged && (src_x != rx || src_y != ry) && (target_x != rx || target_y != ry)) {
            draw_tile_fixup(rx, ry, 2, 2);
        }
        if (display) {   // 0x509A54
            ConsoleStatusWin.draw(veh_id, def_id, att_strength, def_strength, mode);
            if (!disengaged) {   // 0x509A7F
                if (atk_left != 0) {
                    if (single) {
                        // 0x509A9E
                        if (last_hit != 1 && !IsMultiplayerNet) {
                            boom(src_x, src_y,
                                 (2 - (range(proto_power(veh_id) - Vehs[veh_id].dmg_incurred, 0, 9999) != 0)) | 8);
                        }
                        // 0x509B44
                        boom(target_x, target_y,
                             (2 - (range(proto_power(def_id) - Vehs[def_id].dmg_incurred, 0, 9999) != 0)) | 8);
                    } else if (last_hit == 2) {
                        // 0x509BBF
                        if (IsMultiplayerNet ||
                            range(proto_power(def_id) - Vehs[def_id].dmg_incurred, 0, 9999) == 0) {
                            // 0x509C2F
                            boom(target_x, target_y,
                                 (range(proto_power(def_id) - Vehs[def_id].dmg_incurred, 0, 9999) != 0 ? 0 : 2) | 8);
                        }
                    } else if (IsMultiplayerNet) {
                        // 0x509C2C
                        boom(target_x, target_y,
                             (range(proto_power(def_id) - Vehs[def_id].dmg_incurred, 0, 9999) != 0 ? 0 : 2) | 8);
                    } else {
                        // 0x509B44
                        boom(target_x, target_y,
                             (2 - (range(proto_power(def_id) - Vehs[def_id].dmg_incurred, 0, 9999) != 0)) | 8);
                    }
                } else if (single) {
                    // 0x509CB4: attacker destroyed in a one-exchange duel
                    if (last_hit != 2 && !IsMultiplayerNet) {
                        boom(target_x, target_y,
                             range(proto_power(def_id) - Vehs[def_id].dmg_incurred, 0, 9999) != 0);
                    }
                    // 0x509D42 (clamp/push at 0x509E1A)
                    boom(src_x, src_y,
                         2 - (range(proto_power(veh_id) - Vehs[veh_id].dmg_incurred, 0, 9999) != 0));
                } else {
                    // 0x509DB2: attacker destroyed
                    if (last_hit != 1 && !IsMultiplayerNet) {
                        // 0x509DC8
                        boom(src_x, src_y,
                             2 - (range(proto_power(veh_id) - Vehs[veh_id].dmg_incurred, 0, 9999) != 0));
                    } else if (range(proto_power(veh_id) - Vehs[veh_id].dmg_incurred, 0, 9999) == 0 ||
                               IsMultiplayerNet) {
                        // 0x509E48 / 0x509EB3: the HP expression is evaluated again
                        boom(src_x, src_y,
                             range(proto_power(veh_id) - Vehs[veh_id].dmg_incurred, 0, 9999) != 0 ? 0 : 2);
                    }
                }
            }
            // 0x509F10
            VehDrawSelection689F28 = -1;
            VehDrawSelection689F2C = -1;
        }
    }

    // 0x509F1D: a one-exchange duel in which both units survive
    if (single &&
        range(proto_power(veh_id) - Vehs[veh_id].dmg_incurred, 0, 9999) != 0 &&
        range(proto_power(def_id) - Vehs[def_id].dmg_incurred, 0, 9999) != 0) {
        // 0x509FF8
        if (IsMultiplayerNet && def_faction == ConsoleGlobal.faction_id_) {
            parse_says(0, VehPrototypes[Vehs[def_id].proto_id].veh_name, -1, -1);
            // BUG(original): passes the defender VEHICLE id, not its proto id.
            // (clear() + say_stats_3(int) are inlined by the image here.)
            clear();
            say_stats_3(def_id);
            parse_says(1, StringTemp, -1, -1);
            parse_says(2, get_adjective(att_faction), -1, -1);   // inlined
            parse_says(3, VehPrototypes[Vehs[veh_id].proto_id].veh_name, -1, -1);
            clear();
            say_stats_3(Vehs[veh_id].proto_id);
            parse_says(4, StringTemp, -1, -1);
            int report_base = base_find(target_x, target_y, -1, -1, -1, ConsoleGlobal.faction_id_);
            if (report_base >= 0) {
                parse_says(5, Bases[report_base].name_string, -1, -1);
            }
            veh_battle_msg("COMBATSURVIVE", 0);   // inlined NetMsgPrimary.pop(label, 5000, 0, 0)
        }
        return 0;   // 0x50A110
    }

    // 0x50A119: someone died (or the defender withdrew / was captured)
    {
        Map *tile = map_loc(target_x, target_y);
        if ((tile->climate & 0xE0) >= 0x60) {   // land (alt >= 3)
            int owner = tile->territory;        // movsx: may be -1 (unguarded)
            if (owner != att_faction && owner != def_faction
                && (PlayersData[owner].diplo_treaties[def_faction] & DTREATY_VENDETTA)
                && (PlayersData[owner].diplo_treaties[att_faction]
                    & (DTREATY_PACT | DTREATY_TREATY | DTREATY_TRUCE))) {
                if (!PlayersData[att_faction].diplo_unk1[owner]) {
                    PlayersData[att_faction].diplo_unk1[owner] = 1;
                }
                cause_friction(owner, att_faction, -1);
            }
        }
    }

    // 0x50A1B3: rescale strengths and promotions
    if (atk_left != 0) {
        // the attacker survived
        att_strength = VehPrototypes[Vehs[veh_id].proto_id].reactor_id
                       * range(proto_power(veh_id) - Vehs[veh_id].dmg_incurred, 0, 9999)
                       * att_strength / att_hp;
        // 0x50A237
        def_strength = VehPrototypes[Vehs[def_id].proto_id].reactor_id * def_hp
                       * def_strength / proto_power(def_id);
        if (VehPrototypes[Vehs[def_id].proto_id].plan != PLAN_ALIEN_ARTIFACT) {
            // 0x50A29E (rand at 0x64601D; signed compare)
            if (morale_veh(veh_id, 1, 0) <= 1
                || static_cast<int>(bounded_crt_random(att_strength + def_strength)) <= def_strength) {
                promote(veh_id);
            }
        }
        // 0x50A2DD
        if (att_faction == 0) {
            Vehs[veh_id].home_base_id = Vehs[def_id].home_base_id;
        }
    } else {
        // 0x50A2F7: the defender survived
        att_strength = VehPrototypes[Vehs[veh_id].proto_id].reactor_id * att_hp
                       * att_strength / proto_power(veh_id);
        // 0x50A357
        def_strength = VehPrototypes[Vehs[def_id].proto_id].reactor_id
                       * range(proto_power(def_id) - Vehs[def_id].dmg_incurred, 0, 9999)
                       * def_strength / def_hp;
        if (VehPrototypes[Vehs[veh_id].proto_id].plan != PLAN_ALIEN_ARTIFACT) {
            // 0x50A3DA
            if (morale_veh(def_id, 1, 0) <= 1
                || static_cast<int>(bounded_crt_random(att_strength + def_strength)) <= att_strength
                || (base_id >= 0 && morale_veh(def_id, 1, 0) < 4)) {
                promote(def_id);
            }
        }
        // 0x50A432
        if (def_faction == 0) {
            Vehs[def_id].home_base_id = Vehs[veh_id].home_base_id;
        }
    }

    // 0x50A44D
    kill_count = 0;
    booty_count = 0;
    husk_energy = 0;
    damaged_count = 0;
    if (captured != 0 || disengaged != 0) {
        goto L_50b203;
    }
    if (atk_left == 0) {
        goto L_50af13;
    }

    // 0x50A478: the attacker won
    PlayersData[att_faction].diplo_unk3[def_faction]++;
    PlayersData[att_faction].diplo_unk4[def_faction]++;
    FactionTurnCombatWins[att_faction]++;
    FactionTurnCombatLosses[def_faction]++;
    if (IsMultiplayerNet && def_faction == ConsoleGlobal.faction_id_) {
        // 0x50A4E5 (clear/say_stats_3/get_adjective/veh_battle_msg inlined here)
        parse_says(0, VehPrototypes[Vehs[def_id].proto_id].veh_name, -1, -1);
        clear();
        say_stats_3(Vehs[def_id].proto_id);
        parse_says(1, StringTemp, -1, -1);
        parse_says(2, get_adjective(att_faction), -1, -1);
        parse_says(3, VehPrototypes[Vehs[veh_id].proto_id].veh_name, -1, -1);
        clear();
        say_stats_3(Vehs[veh_id].proto_id);
        parse_says(4, StringTemp, -1, -1);
        int report_base = base_find(target_x, target_y, -1, -1, -1, ConsoleGlobal.faction_id_);
        if (report_base >= 0) {
            parse_says(5, Bases[report_base].name_string, -1, -1);
        }
        veh_battle_msg("COMBATLOSS", 0);
    } else if (IsMultiplayerPBEM &&
               (GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & (1 << def_faction))) {
        // 0x50A61D
        parse_says(0, VehPrototypes[Vehs[def_id].proto_id].veh_name, -1, -1);
        clear();
        say_stats_3(Vehs[def_id].proto_id);
        parse_says(1, StringTemp, -1, -1);
        parse_says(2, get_adjective(att_faction), -1, -1);
        parse_says(3, VehPrototypes[Vehs[veh_id].proto_id].veh_name, -1, -1);
        clear();
        say_stats_3(Vehs[veh_id].proto_id);
        parse_says(4, StringTemp, -1, -1);
        int report_base = base_find(target_x, target_y, -1, -1, -1, def_faction);
        if (report_base >= 0) {
            parse_says(5, Bases[report_base].name_string, -1, -1);
        }
        POP3("COMBATLOSS", def_faction, target_x, target_y);   // inlined to POP3(s, 0, f, x, y)
    }

    // 0x50A730: kill the defender alone, or damage the whole stack
    if (base_id < 0
        && !(map_loc(target_x, target_y)->bit & BIT_BUNKER)
        && get_triad(def_id) != TRIAD_AIR
        && get_triad(veh_id) != TRIAD_AIR
        && VehPrototypes[Vehs[veh_id].proto_id].plan != PLAN_INFO_WARFARE
        && VehPrototypes[Vehs[def_id].proto_id].plan != PLAN_NAVAL_SUPERIORITY) {
        // 0x50A7C8
        if (stack_check(def_id, 2, PLAN_COLONIZATION, -1, -1) == 0
            || (map_loc(target_x, target_y)->climate & 0xE0) < 0x60) {
            goto L_50a904;
        }
    }
    // 0x50A80E
    if (VehPrototypes[Vehs[def_id].proto_id].plan == PLAN_ALIEN_ARTIFACT) {
        goto L_50a904;
    }
    if (att_faction != 0) {
        // 0x50A831
        if (veh_cargo(def_id) != 0 && base_id < 0
            && VehPrototypes[Vehs[veh_id].proto_id].plan != PLAN_INFO_WARFARE) {
            goto L_50a904;
        }
        // 0x50A863
        if (VehPrototypes[Vehs[veh_id].proto_id].plan != PLAN_INFO_WARFARE) {
            // call order: (2,12), (1), (2,8), (2,11), (2,10)
            int c12 = stack_check(def_id, 2, PLAN_ALIEN_ARTIFACT, -1, -1);
            int n = stack_check(def_id, 1, -1, -1, -1) - c12;
            n -= stack_check(def_id, 2, PLAN_COLONIZATION, -1, -1);
            n -= stack_check(def_id, 2, PLAN_INFO_WARFARE, -1, -1);
            n -= stack_check(def_id, 2, PLAN_SUPPLY_CONVOY, -1, -1);
            if (n <= 1) {
                goto L_50a904;
            }
        }
    }
    // 0x50A8DC: kill just the defender
    battle_kill(def_id, &kill_count, &booty_count, &husk_energy, veh_id, att_faction);
    goto L_50ac8b;   // (EDI is reloaded from stack_id at 0x50A8F9)

L_50a904:
    {
    // 0x50A904: damage the whole defending stack
    stack_veh(def_id, 0);
    // veh_top(def_id) inlined; the first prev-link read goes through [ebp-0x08]
    for (int v = veh_top(def_id); v >= 0; v = veh_next_stack_id(v)) {   // next inlined at 0x50AACD
        int v_proto = Vehs[v].proto_id;
        // 0x50A94B
        if (Chassis[VehPrototypes[v_proto].chassis_id].triad == TRIAD_LAND
            && (map_loc(target_x, target_y)->climate & 0xE0) < 0x60   // sea tile
            && base_who(target_x, target_y) < 0                        // inlined
            && !(Vehs[v].order == ORDER_SENTRY_BOARD
                 && Vehs[v].waypoint_x[0] != def_id
                 && Vehs[v].waypoint_x[0] >= 0)) {
            // 0x50A9E0: a land unit carried at sea takes all its HP (byte add)
            Vehs[v].dmg_incurred += range(proto_power(v) - Vehs[v].dmg_incurred, 0, 9999);
        } else if (v != def_id) {
            // 0x50AA4C
            int plan = VehPrototypes[v_proto].plan;
            if (plan == PLAN_ALIEN_ARTIFACT || plan == PLAN_COLONIZATION
                || plan == PLAN_INFO_WARFARE || plan == PLAN_SUPPLY_CONVOY) {
                continue;   // 0x50AACD
            }
        }
        // 0x50AA62: collateral damage. Psi weapons use the victim's reactor
        int reactor = (Weapon[VehPrototypes[Vehs[veh_id].proto_id].weapon_id].offense_rating < 0)
                          ? VehPrototypes[Vehs[v].proto_id].reactor_id
                          : VehPrototypes[Vehs[veh_id].proto_id].reactor_id;
        Vehs[v].dmg_incurred += reactor * 3;   // 8-bit imul/add
        Vehs[v].state |= VSTATE_UNK_4;
        damaged_count++;
    }

    // 0x50AAE0: remove the stack's dead units
    atk_id_adj = veh_id;
    if (def_id >= 0) {
        // veh_top(def_id) inlined again (first link via [ebp-0x08])
        int v = veh_top(def_id);
        while (v >= 0) {
            // 0x50AB1E
            int next = Vehs[v].next_veh_id_stack;
            // the outer range() is CALLED here (0x422F00); proto_power stays inlined
            if (range(proto_power(v) - Vehs[v].dmg_incurred, 0, 9999) <= 0
                || Vehs[v].faction_id == 0) {
                // 0x50AB9B
                if (battle_kill(v, &kill_count, &booty_count, &husk_energy,
                                atk_id_adj, att_faction)) {
                    if (atk_id_adj > v) {
                        atk_id_adj--;
                    }
                    if (next > v) {
                        next--;
                    }
                }
            }
            v = next;   // 0x50ABC9
        }
    }
    // 0x50ABD3 (sets/dec/and idiom)
    {
        int t = damaged_count - (kill_count + booty_count);
        damaged_count = t < 0 ? 0 : t;
    }
    stack_id = stack_fix(veh_at(target_x, target_y));
    if (stack_id >= 0) {
        // 0x50AC0D call order: (2,12), (1), (2,8), (2,11), (2,10)
        int c12 = stack_check(stack_id, 2, PLAN_ALIEN_ARTIFACT, -1, -1);
        int n = stack_check(stack_id, 1, -1, -1, -1) - c12;
        n -= stack_check(stack_id, 2, PLAN_COLONIZATION, -1, -1);
        n -= stack_check(stack_id, 2, PLAN_INFO_WARFARE, -1, -1);
        n -= stack_check(stack_id, 2, PLAN_SUPPLY_CONVOY, -1, -1);
        if (n == 0) {
            // only non-combat units are left
            battle_kill_stack(stack_id, &kill_count, &booty_count, &husk_energy,
                              veh_id, att_faction);
            damaged_count = 0;
        }
    }
    }

L_50ac8b:
    // 0x50AC8B: base consequences (EDI = stack_id on both entries)
    if (base_id >= 0) {
        Bases[base_id].state |= BSTATE_COMBAT_LOSS_LAST_TURN;
        if (!is_ocean(src_x, src_y)
            && (!is_objective(base_id) || Bases[base_id].population_size > 1)
            && !has_fac(FAC_PERIMETER_DEFENSE, base_id, 0)       // called (0x421670)
            && !has_fac(FAC_TACHYON_FIELD, base_id, 0)
            && !has_project(SP_CITIZENS_DEFENSE_FORCE, def_faction)   // called
            // unguarded: stack_id may be -1 here
            && Weapon[VehPrototypes[Vehs[stack_id].proto_id].weapon_id].offense_rating != 0
            && att_faction != 0
            && veh_who(target_x, target_y) < 0                   // called (0x500250)
            && (PlayersData[def_faction].diff_level != 0
                || !(GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & (1 << def_faction)))) {
            // 0x50AD8E
            if (--Bases[base_id].population_size <= 0) {
                nerve_gas = 0;
                base_kill(base_id);
                base_id = -1;
                for (int i = 0; i < 8; i++) {
                    synch_bit(target_x, target_y, i);
                }
                draw_map(1);
                // 0x50ADDC (shared with 0x50AEAD)
                if (eliminate_player(def_faction, att_faction)) {
                    pact_report = 0;
                }
                goto L_50aebf;
            }
            // 0x50ADF8
            Bases[base_id].faction_pop_size_intel[att_faction] = Bases[base_id].population_size;
        }
        // 0x50AE02
        if (!(GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & (1 << def_faction))) {
            base_reset(base_id, 0);
        }
        if (nerve_gas != 0) {
            // 0x50AE2B: nerve_gas becomes the number of citizens killed
            int pop = Bases[base_id].population_size;
            nerve_gas = (pop + 1) / 2;
            Bases[base_id].population_size = pop - nerve_gas;   // byte
            if (is_objective(base_id)) {
                int p = Bases[base_id].population_size;
                Bases[base_id].population_size = p < 1 ? 1 : p;   // unconditional store
            }
            // 0x50AE6C
            if (Bases[base_id].population_size <= 0) {
                base_kill(base_id);
                base_id = -1;
                for (int i = 0; i < 8; i++) {
                    synch_bit(target_x, target_y, i);
                }
                draw_map(1);
                // cross-jumps to 0x50ADDC
                if (eliminate_player(def_faction, att_faction)) {
                    pact_report = 0;
                }
            } else {
                // 0x50AEB2
                spot_base(base_id, att_faction);
            }
        }
L_50aebf:
        // 0x50AEBF
        if (veh_who(target_x, target_y) < 0
            && (GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & (1 << def_faction))
            && base_id >= 0) {
            invasions(base_id);
        }
    }
    // 0x50AEF3
    if (att_faction != 0 && def_faction != 0) {
        mon_enemy_destroyed(att_faction, def_faction);   // shared call at 0x50B1B4
    }
    goto L_50b1bc;

L_50af13:
    // 0x50AF13: the defender won (helpers are called here, not inlined)
    FactionTurnCombatWins[def_faction]++;
    FactionTurnCombatLosses[att_faction]++;
    if (IsMultiplayerNet && def_faction == ConsoleGlobal.faction_id_) {
        // 0x50AF50
        parse_says(0, VehPrototypes[Vehs[def_id].proto_id].veh_name, -1, -1);
        clear();
        say_stats_3(Vehs[def_id].proto_id);
        parse_says(1, StringTemp, -1, -1);
        parse_says(2, get_adjective(att_faction), -1, -1);
        parse_says(3, VehPrototypes[Vehs[veh_id].proto_id].veh_name, -1, -1);
        clear();
        say_stats_3(Vehs[veh_id].proto_id);
        parse_says(4, StringTemp, -1, -1);
        int report_base = base_find(target_x, target_y, -1, -1, -1, ConsoleGlobal.faction_id_);
        if (report_base >= 0) {
            parse_says(5, Bases[report_base].name_string, -1, -1);
        }
        veh_battle_msg("COMBATWIN", 0);
    } else if (IsMultiplayerPBEM &&
               (GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & (1 << def_faction))) {
        // 0x50B068
        parse_says(0, VehPrototypes[Vehs[def_id].proto_id].veh_name, -1, -1);
        clear();
        say_stats_3(Vehs[def_id].proto_id);
        parse_says(1, StringTemp, -1, -1);
        parse_says(2, get_adjective(att_faction), -1, -1);
        parse_says(3, VehPrototypes[Vehs[veh_id].proto_id].veh_name, -1, -1);
        clear();
        say_stats_3(Vehs[veh_id].proto_id);
        parse_says(4, StringTemp, -1, -1);
        int report_base = base_find(target_x, target_y, -1, -1, -1, def_faction);
        if (report_base >= 0) {
            parse_says(5, Bases[report_base].name_string, -1, -1);
        }
        POP3("COMBATWIN", def_faction, target_x, target_y);   // 0x50B8D0
    }
    // 0x50B157
    stack_veh(veh_id, 1);
    battle_kill_stack(veh_id, &kill_count, &booty_count, &husk_energy, -1, def_faction);
    PlayersData[def_faction].diplo_unk3[att_faction]++;
    PlayersData[def_faction].diplo_unk4[att_faction]++;
    if (att_faction != 0 && def_faction != 0) {
        mon_enemy_destroyed(def_faction, att_faction);
    }

L_50b1bc:
    // 0x50B1BC
    if (booty_count != 0
        && !(GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & (1 << def_faction))
        && !has_treaty(att_faction, def_faction, DTREATY_VENDETTA)) {   // called (0x5002F0)
        cause_friction(def_faction, att_faction, 5);
        treaty_on(att_faction, def_faction, DTREATY_WANT_TO_TALK);
    }

L_50b203:
    // 0x50B203: energy from native kills, and angering neighbouring natives
    if (atk_left != 0 && husk_energy != 0) {
        PlayersData[att_faction].energy_reserves += husk_energy;
        ConsoleGlobal.update_data(0);
        for (dir_i = 0; dir_i < 8; dir_i++) {   // [ebp+0x08], stored as dir_i*4
            int nx = xrange(target_x + VehMoveDeltaX[dir_i]);   // called
            int ny = target_y + VehMoveDeltaY[dir_i];
            if (on_map(nx, ny)) {   // called (0x4712A0)
                for (stack_id = veh_at(nx, ny); stack_id >= 0;
                     stack_id = veh_next_stack_id(stack_id)) {   // called (0x50B8F0)
                    if (Vehs[stack_id].faction_id == 0) {
                        // flags bits 3..4: +1, clamped by range(.., 0, 3) (called)
                        int16_t f = Vehs[stack_id].flags;
                        int lvl = (f >> 3) & 3;
                        Vehs[stack_id].flags = f & ~(lvl << 3);
                        Vehs[stack_id].flags |= range(lvl + 1, 0, 3) << 3;
                        draw_tile(nx, ny, 2);
                    }
                }
            }
        }
    }

    // 0x50B2FA: on-screen aftermath
    if (display != 0) {
        if (atk_left != 0) {
            draw_tile(src_x, src_y, 1);
            draw_tile_fixup(target_x, target_y, 2, 1);
        } else {
            draw_tile_fixup(src_x, src_y, 2, 1);
            draw_tile(target_x, target_y, 1);
        }
        do_all_draws();
        // 0x50B354 (the local faction is loaded once into ECX for 0x50B370/0x50B424/0x50B428)
        if (atk_left != 0 && husk_energy != 0 && att_faction == ConsoleGlobal.faction_id_) {
            // 0x50B378
            parse_num(0, husk_energy);
            veh_battle_msg("HUSKS", 0);
        } else if (kill_count > 1 || damaged_count != 0) {
            // 0x50B40E / 0x50B421
            if (att_faction == ConsoleGlobal.faction_id_ || def_faction == ConsoleGlobal.faction_id_) {
                parse_num(0, kill_count);
                parse_num(1, damaged_count);
                clear();
                says((att_faction == ConsoleGlobal.faction_id_) == (atk_left != 0) ? "NUMGOT" : "NUMLOST");
                if (kill_count > 1) {
                    if (damaged_count) {
                        says("2");
                    }
                } else if (damaged_count) {
                    says("1");
                }
                veh_battle_msg(StringTemp, 0);
            }
        }
        // 0x50B392
        if (booty_count != 0) {
            parse_num(0, booty_count);
            if (def_faction == ConsoleGlobal.faction_id_) {
                clear();
                says("THEIRBOOTY");
                if (has_treaty(att_faction, def_faction, DTREATY_VENDETTA)) {
                    // 0x50B4AF
                    say_num(range(booty_count, 1, 2));   // range called
                    veh_battle_msg(StringTemp, 0);
                } else {
                    parse_says(0, get_noun(att_faction), -1, -1);
                    say_num(3);
                    veh_battle_msg(StringTemp, 0);
                }
            } else if (att_faction == ConsoleGlobal.faction_id_) {
                // 0x50B4DA
                clear();
                says("OURBOOTY");
                say_num(range(booty_count, 1, 2));
                veh_battle_msg(StringTemp, 0);
            }
        }
        // 0x50B50B
        if (nerve_gas != 0 && strlen(nerve_base_name) != 0) {
            parse_says(0, nerve_base_name, -1, -1);
            veh_battle_msg("NERVEGAS", 0);
        }
        // 0x50B546
        if (disengaged != 0) {
            parse_says(0, VehPrototypes[Vehs[stack_id].proto_id].veh_name, -1, -1);
            parse_says(1, get_adjective(def_faction), -1, -1);
            if (def_faction == ConsoleGlobal.faction_id_) {
                veh_battle_msg("WEDISENGAGED", 0);
            } else if (att_faction == ConsoleGlobal.faction_id_) {
                veh_battle_msg("THEYDISENGAGED", 0);
            }
        }
        // 0x50B5B7
        ConsoleStatusWin.redraw();
        if (zoom_refresh != 0) {
            TutWinBaseWindow.focus();
            MainInterfaceGlobal.redraw();   // GraphicWin::redraw 0x5D5A70 on 0x7AE820
            do_all_draws();
        }
        if (GameDataStateStorage.fields_.game_more_preferences_ & MPREF_ADV_PAUSE_AFTER_BATTLES) {
            g_BattleWin.stop_timer();
            popp("BATTLEPAUSE", "batwon_sm.pcx", 0);   // 3-arg popp (0x50B970)
            g_BattleWin.pulse_timer();
            if (ConsoleGroupAction) {
                GameDataStateStorage.fields_.game_more_preferences_ &= ~MPREF_ADV_PAUSE_AFTER_BATTLES;
                prefs_save(0);
            }
        }
    }

    // 0x50B63F: treaty reports (EDI = att_faction, ESI = def_faction)
    switch (pact_report) {
    case 1:
        parse_says(0, get_adjective(att_faction), -1, -1);
        parse_says(1, get_noun(def_faction), -1, -1);
        NetMsg::pop("VENDETTAREPORT", 0);
        break;
    case 2:
        parse_says(0, get_adjective(att_faction), -1, -1);
        parse_says(1, get_noun(def_faction), -1, -1);
        NetMsg::pop("PACTATTACKED", 0);
        break;
    case 3:
        parse_says(0, get_noun(att_faction), -1, -1);
        parse_says(1, get_noun(def_faction), -1, -1);
        NetMsg::pop("PACTATTACKING", 0);
        break;
    }

    // 0x50B6F7: mark both prototypes as seen in combat
    VehPrototypes[att_proto].combat_factions |= 1 << def_faction;
    VehPrototypes[def_proto].combat_factions |= 1 << att_faction;
    if (local_base != 0) {
        interlude(0, 0, 1, 0);
    }
    if (atk_left == 0) {
        return 0;   // 0x50B75C
    }

    // 0x50B765: the attacker survived; the vehicle in 0x9A64DC pays moves
    {
        int cur = GameDataStateStorage.fields_.vehicle_selection_follow_limit_;   // 0x9A64DC
        int chas = VehPrototypes[Vehs[cur].proto_id].chassis_id;
        if (Chassis[chas].triad == TRIAD_AIR) {
            if (Chassis[chas].range != 1) {
                veh_skip(cur);
                Vehs[cur].moves_expended += Rules.move_rate_roads;   // byte add, 0x949738
            }
        } else if (Chassis[chas].triad == TRIAD_SEA) {
            veh_skip(cur);
            Vehs[cur].moves_expended += Rules.move_rate_roads;
        }
        // 0x50B7E4
        int result = 1;   // EBX
        if (Chassis[VehPrototypes[Vehs[cur].proto_id].chassis_id].missile) {
            kill(cur);
            result = 0;
        }
        // 0x50B81C
        if (!(static_cast<uint8_t>(att_bit) & GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_) &&
            base_id >= 0 && veh_who(target_x, target_y) < 0) {
            int c = VehPrototypes[Vehs[cur].proto_id].chassis_id;
            if (Chassis[c].triad == TRIAD_AIR && Chassis[c].range != 0
                && Vehs[cur].terraforming_turns + 1 >= Chassis[c].range) {
                return 1;
            }
        }
        return result;
    }
}

// ORIGINAL: 0x0050B8F0 sub_50b8f0 0x0050B8F0-0x0050B90A BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:f178e8db:src/recovered/0050b8f0.cpp complete original identity/spans and historical claims/findings.", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x50b8f0 full native body review.", "source:src/veh.cpp existing real state owners; no executable-address alias or synthetic receiver."], "reason": "Recover the complete short helper through established source-owned diplomacy, vehicle, text or window state; preserve signed sentinel behavior, original receiver/argument order and conditional callbacks."}
// symbol    ?veh_next_stack_id@@YAHH@Z
int __cdecl veh_next_stack_id(int veh_id) {
  if (veh_id < 0) return veh_id;
  return Vehs[veh_id].next_veh_id_stack;
}

// ===== veh.cpp, P2 wave 2: the order verbs =====

/*
Purpose: Open a popup with the climate title and a body, no buttons.
// ORIGINAL: 0x0050B970 ?popp@@YAHPBDPBDP6AHXZ@Z 0x0050B970-0x0050B991 BYTE_EXACT
// size      33 bytes
// prototype int (__cdecl ?popp@@YAHPBDPBDP6AHXZ@Z)(const char *label, const char *body, int (__cdecl *callback)())
// callers   2   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0048C0A0
// HOMED from src/recovered/0050b970.cpp, 2026-09-04 - its archive-only
// BYTE_EXACT claim re-earned in the TU. A third popp overload beside temp.h's
// five-argument one; C++ overloading gives the pair the two symbols the image
// carries. ScriptPopupFile is the mutable title pointer the image reads at
// 0x00691B0C; older source exposed it through ClimatePopupTitle in game.h.
// symbol    ?popp@@YAHPBD0P6AHXZ@Z
//           The image spells its two const char * parameters out; VC6
//           12.00.8168 back-references the repeat to PBD0 for the identical
//           source. The archive's `char *` return was also wrong - the first H
//           in YAHPBD... is the return type, so this returns int.
Return Value: whatever the five-argument popp returns.
Status: BYTE_EXACT.
*/
int __cdecl popp(const char *label, const char *body, int(__cdecl *callback)()) {
    return popp(ScriptPopupFile, label, 0, body, callback);
}

/*
Purpose: Generate prototype stats using stringTemp buffer.
// ORIGINAL: 0x0050B9A0 ?say_stats_3@@YAXH@Z 0x0050B9A0-0x0050B9B6 BYTE_EXACT
// size      22 bytes
// prototype void (__cdecl ?say_stats_3@@YAXH@Z)(int protoID)
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0057D7D0
Return Value: n/a
Status: Complete
*/
void __cdecl say_stats_3(int proto_id) {
    say_stats_3(StringTemp, proto_id);
}

/*
Purpose: Pop a battle message with its portrait.
// ORIGINAL: 0x0050B9C0 sub_50b9c0 0x0050B9C0-0x0050B9DE BYTE_EXACT
// size      30 bytes
// prototype void (__cdecl sub_50b9c0)(char *label, char *icon)
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0047A890
// symbol    ?veh_battle_msg@@YAXPAD0@Z
//           The image's catalogue name is an uninformative `sub_`; this tree
//           names what the body does. Renames update this fact, not the bytes.
// HOMED from src/recovered/0050b9c0.cpp, 2026-09-04. Its `g_00805338` is
// netmsg.h's NetMsgPrimary; 5000 is the message lifetime in ticks.
Return Value: n/a
Status: BYTE_EXACT.
*/
void __cdecl veh_battle_msg(char *label, char *icon) {
    NetMsgPrimary.pop(label, 5000, 0, icon);
}

