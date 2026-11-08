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

static int shoot_signed_bits(unsigned int bits) {
    return bits <= 0x7fffffffu ? static_cast<int>(bits)
        : -1 - static_cast<int>(0xffffffffu - bits);
}

static int shoot_wrapped_subtract(int left, int right) {
    return shoot_signed_bits(static_cast<unsigned int>(left) -
                             static_cast<unsigned int>(right));
}

// ORIGINAL: 0x00500560 ?shoot@@YAHHHHH@Z 0x00500560-0x00500B51
// RECOVERY_BINDING: v1 {"evidence": ["task:planet-strike-owner", "pinned:0x00500560 complete native span; real Veh, PlayerData, Base and Console declarations establish accessed storage.", "pinned:shoot6B7/7C2 live satellite inventory reloads; bonus success bypasses decrement; dword counter/subtraction wrapping retained.", "pinned:planetE16/E29 attacker-row PACT; territory callback follows radius interception; kill/draw precede intercept return; warning coordinates precede popupD0C.", "source:map.cpp bounded shared radius tables and veh.cpp TerraformDrownedBaseId; effect59 is the typed FX child observed by the native wait.", "historical seed:src/unrecovered/00500560.cpp original mapping and useful findings retained."], "reason": "Recover the complete original strike operation through genuine vehicle, faction, base, console, effect and shared map-table owners; preserve original identity/spans and seed findings without executable storage or calls."}
// TRIED: structural transcription (two proximity scan loops with early-exit gotos, extern lever on the three lea-taken-as-pointer table bases 0097d053/00946d34/00946d54) compiles and matches to instr #6 (prologue: original zeroes edi with `xor`, rebuilt uses `mov`).
int __cdecl shoot(int atk_factionID, int def_factionID, int xDest, int yDest) {
    int adjacent_flechette = 0;
    for (int i = 0; i < 25 && !adjacent_flechette; ++i) {
        int x = xDest + RadiusOffsetX[i];
        if (!(MapIsFlat & 1)) {
            if (x < 0) x += MapLongitudeBounds;
            else if (x >= MapLongitudeBounds) x -= MapLongitudeBounds;
        }
        const int y = yDest + RadiusOffsetY[i];
        if (y < 0 || y >= MapLatitudeBounds || x < 0 || x >= MapLongitudeBounds)
            continue;
        const int base_id = base_at(x, y);
        if (base_id >= 0 && Bases[base_id].faction_id_current != atk_factionID &&
            has_fac(FAC_FLECHETTE_DEFENSE_SYS, base_id, 0))
            adjacent_flechette = 1;
    }

    const int deployed_at_start = shoot_signed_bits(
        PlayersData[def_factionID].satellites_[3]);
    if (deployed_at_start == 0 && !adjacent_flechette)
        return 0;

    int bonus_hit = 0;
    const int bonus_rounds = shoot_wrapped_subtract(
        deployed_at_start,
        shoot_signed_bits(PlayersData[def_factionID].sat_odp_deployed));
    if (bonus_rounds > 0) {
        for (int i = 0; i < bonus_rounds; ++i) {
            ++PlayersData[def_factionID].sat_odp_deployed;
            if (rand() % 100 > 0x31) {
                bonus_hit = 1;
                break;
            }
        }
    }

    int shots_left = shoot_wrapped_subtract(
        shoot_signed_bits(PlayersData[def_factionID].satellites_[3]),
        shoot_signed_bits(PlayersData[def_factionID].sat_odp_deployed));
    if (shots_left < 0) shots_left = 0;
    else if (shots_left > 9999) shots_left = 9999;

    const char *message_icon = 0;
    const char *message_label = 0;
    if (!bonus_hit) {
        for (int i = 0; i < 25; ++i) {
            int x = xDest + RadiusOffsetX[i];
            if (!(MapIsFlat & 1)) {
                if (x < 0) x += MapLongitudeBounds;
                else if (x >= MapLongitudeBounds) x -= MapLongitudeBounds;
            }
            const int y = yDest + RadiusOffsetY[i];
            if (y < 0 || y >= MapLatitudeBounds || x < 0 || x >= MapLongitudeBounds)
                continue;
            const int base_id = base_at(x, y);
            if (base_id < 0 || Bases[base_id].faction_id_current == atk_factionID ||
                !has_fac(FAC_FLECHETTE_DEFENSE_SYS, base_id, 0))
                continue;
            if (rand() % 100 >= 0x32)
                continue;

            parse_says(0, Bases[base_id].name_string, -1, -1);
            if (def_factionID == ConsoleGlobal.faction_id_) {
                parse_says(1, Players[atk_factionID].adj_name_faction, -1, -1);
                message_icon = "space_sm.pcx";
                message_label = "WESHOTIT2";
            } else {
                if (atk_factionID != ConsoleGlobal.faction_id_)
                    return 1;
                parse_says(1, Players[Bases[base_id].faction_id_current].adj_name_faction,
                           -1, -1);
                message_icon = "space_sm.pcx";
                message_label = "THEYSHOTIT2";
            }
            goto show_message;
        }
    }

    if (bonus_hit)
        goto report_hit;

    // Native rereads the live inventory at 0x5007bf after the Flechette scan.
    if (PlayersData[def_factionID].satellites_[3] == 0)
        goto no_ammunition;

    if (!IsMultiplayerNet &&
        (GameDataStateStorage.fields_.lock_masks_.enable_mask_ & (1u << (def_factionID & 0x1f))) != 0) {
        if (def_factionID == ConsoleGlobal.faction_id_) {
            parse_num(0, PlayersData[def_factionID].satellites_[3]);
            const int base_id = base_find(xDest, yDest);
            parse_says(0, Bases[base_id].name_string, -1, -1);
            parse_says(1, Players[atk_factionID].adj_name_faction, -1, -1);
            if (popp(ScriptPopupFile, "BUSTERSAC", 0, "space_sm.pcx", 0) != 0)
                goto shot_declined;
        } else if (atk_factionID == ConsoleGlobal.faction_id_) {
            GenderDefault = Players[def_factionID].noun_gender;
            PluralityDefault = Players[def_factionID].is_noun_plural;
            parse_says(0, Players[def_factionID].noun_faction, -1, -1);
            popp(ScriptPopupFile, "THEYMUSTSAC", 0, "space_sm.pcx", 0);
        }
    } else {
        if (def_factionID != ConsoleGlobal.faction_id_) {
            if (atk_factionID == ConsoleGlobal.faction_id_) {
                GenderDefault = Players[def_factionID].noun_gender;
                PluralityDefault = Players[def_factionID].is_noun_plural;
                parse_says(0, Players[def_factionID].noun_faction, -1, -1);
                popp(ScriptPopupFile, "THEYMUSTSAC", 0, "space_sm.pcx", 0);
            }
        } else {
            popp(ScriptPopupFile, "WEMUSTSAC", 0, "space_sm.pcx", 0);
            goto expend_shot;
        }
    }

expend_shot:
    --PlayersData[def_factionID].satellites_[3];
    if (shoot_wrapped_subtract(
            shoot_signed_bits(PlayersData[def_factionID].satellites_[3]),
            shoot_signed_bits(PlayersData[def_factionID].sat_odp_deployed)) < 0)
        PlayersData[def_factionID].sat_odp_deployed =
            PlayersData[def_factionID].satellites_[3];

report_hit:
    {
        const int faction_base = base_find(xDest, yDest, def_factionID);
        const int base_id = faction_base < 0 ? base_find(xDest, yDest) : faction_base;
        parse_says(0, Bases[base_id].name_string, -1, -1);
        parse_num(0, PlayersData[def_factionID].satellites_[3]);
        parse_num(1, shots_left);
        if (def_factionID == ConsoleGlobal.faction_id_) {
            parse_says(1, Players[atk_factionID].adj_name_faction, -1, -1);
            message_icon = "space_sm.pcx";
            message_label = "WESHOTIT";
        } else {
            if (atk_factionID != ConsoleGlobal.faction_id_)
                return 1;
            parse_says(1, Players[def_factionID].adj_name_faction, -1, -1);
            message_icon = "space_sm.pcx";
            message_label = "THEYSHOTIT";
        }
    }
show_message:
    popp(ScriptPopupFile, message_label, 0, message_icon, 0);
    return 1;

shot_declined:
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

no_ammunition:
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

// ORIGINAL: 0x00500B60 ?planet_busting@@YAHHHH@Z 0x00500B60-0x005010BB
// RECOVERY_BINDING: v1 {"evidence": ["task:planet-strike-owner", "pinned:0x00500B60 complete native span; real Veh, PlayerData, Base and Console declarations establish accessed storage.", "pinned:shoot6B7/7C2 live satellite inventory reloads; bonus success bypasses decrement; dword counter/subtraction wrapping retained.", "pinned:planetE16/E29 attacker-row PACT; territory callback follows radius interception; kill/draw precede intercept return; warning coordinates precede popupD0C.", "source:map.cpp bounded shared radius tables and veh.cpp TerraformDrownedBaseId; effect59 is the typed FX child observed by the native wait.", "historical seed:src/unrecovered/00500b60.cpp original mapping and useful findings retained."], "reason": "Recover the complete original strike operation through genuine vehicle, faction, base, console, effect and shared map-table owners; preserve original identity/spans and seed findings without executable storage or calls."}
int __cdecl planet_busting(int veh_id, int x, int y) {
    const Veh &weapon_vehicle = Vehs[veh_id];
    const int faction = weapon_vehicle.faction_id;
    ConsoleGlobal.focus(x, y, faction);

    const int proto_id = weapon_vehicle.proto_id;
    const VehPrototype &weapon_prototype = VehPrototypes[proto_id];
    const int bust_level =
        weapon_prototype.plan == PLAN_PLANET_BUSTER
            ? weapon_prototype.reactor_id : 0;
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

        const int dx = abs(Bases[nearest_base].x - x);
        const int wrapped_dx = (!(MapIsFlat & 1) && dx > MapLongitude)
            ? MapLongitudeBounds - dx : dx;
        const int dy = abs(Bases[nearest_base].y - y);
        const char *const sound = (((dy + wrapped_dx) >> 1) <= bust_level)
            ? "baseobl_sm.pcx" : "astp_sm.pcx";
        popp(ScriptPopupFile, "PLANETBUSTER", 0, sound, 0);
    }

    int notified[MaxPlayerNum] = {0};
    int intercepted = 0;
    for (int offset = 0; offset < weapon_count; ++offset) {
        int blast_x = x + RadiusOffsetX[offset];
        if (!(MapIsFlat & 1)) {
            if (blast_x < 0) blast_x += MapLongitudeBounds;
            else if (blast_x >= MapLongitudeBounds) blast_x -= MapLongitudeBounds;
        }
        const int blast_y = y + RadiusOffsetY[offset];
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
    int blame_base = -1;
    int blame_vehicle = -1;
    for (int lower = 0; lower < bust_level; ++lower) {
        world_lower_alt(x, y);
    }
    world_climate();
    TerraformDrownedBaseId = -1;
    g_FX.play(0x3b);
    BoomAnimationLevel = bust_level;
    boom(x, y, 0x20);

    for (int offset2 = 0; offset2 < weapon_count; ++offset2) {
        int blast_x = x + RadiusOffsetX[offset2];
        if (!(MapIsFlat & 1)) {
            if (blast_x < 0) blast_x += MapLongitudeBounds;
            else if (blast_x >= MapLongitudeBounds) blast_x -= MapLongitudeBounds;
        }
        const int blast_y = y + RadiusOffsetY[offset2];
        if (!on_map(blast_x, blast_y)) continue;

        const int base_id = base_at(blast_x, blast_y);
        if (base_id >= 0) {
            const int defender = Bases[base_id].faction_id_current;
            if (blame_base < 0 && defender != faction) blame_base = defender;
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
    // Original successful terminal EAX is unspecified; its observed caller
    // discards it. This is a source completion value, not an exact-return claim.
    return 0;
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
    int offense = offense_proto(veh_atk->proto_id, veh_id_def, is_bombard);
    if (psi_combat_type) {
        offense = psi_factor(offense, faction_id_atk, true, false);
    }
    VehBasicBattleMorale[unk_tgl != 0] = morale;
    return offense * (morale + 6) * 4;
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
    for (int i = veh_top(veh_id_def); i >= 0; i = Vehs[i].next_veh_id_stack) {
        uint32_t proto_id_def = Vehs[i].proto_id;
        if ((get_proto_triad(proto_id_def) != TRIAD_LAND || !is_ocean_def && base_id_def >= 0)
            && (VehPrototypes[Vehs[veh_id_atk].proto_id].plan != PLAN_INFO_WARFARE
                || VehPrototypes[proto_id_def].plan == PLAN_INFO_WARFARE)) {
            uint32_t combat_type = (can_arty(proto_id_def, true)
                && can_arty(Vehs[veh_id_atk].proto_id, true)) ? 2 : 0;
            if (veh_id_atk >= 0 && get_triad(veh_id_atk) != TRIAD_AIR
                && has_abil(proto_id_def, ABL_AIR_SUPERIORITY)
                && !is_proto_missile(proto_id_def) && !is_missile(veh_id_atk)
                && get_offense_rating(veh_id_atk) > 0 && get_defense_rating(veh_id_atk) > 0
                && get_proto_offense_rating(proto_id_def) > 0) {
                combat_type |= 0xA; // air combat
                if (get_proto_triad(proto_id_def) == TRIAD_AIR) {
                    combat_type |= 0x10;
                }
            }
            int offense_output;
            int defense_output;
            battle_compute(veh_id_atk, i, &offense_output, &defense_output, combat_type);
            if (!offense_output) {
                break;
            }
            int proto_pwr_def = proto_power(i);
            int def_modifier = (((((range(proto_pwr_def - Vehs[i].dmg_incurred, 0, 9999)
                * defense_output) / proto_pwr_def) * offense) / offense_output) / 8)
                - get_proto_offense_rating(proto_id_def);
            uint32_t plan_def = VehPrototypes[proto_id_def].plan;
            if (plan_def < PLAN_COLONIZATION || plan_def == PLAN_TERRAFORMING) {
                def_modifier *= 16;
            }
            if (veh_id_atk >= 0 && get_triad(veh_id_atk) == TRIAD_AIR
                && has_abil(proto_id_def, ABL_AIR_SUPERIORITY)
                && get_proto_triad(proto_id_def) == TRIAD_AIR
                && !is_proto_missile(proto_id_def) && !is_missile(veh_id_atk)) {
                def_modifier += 0x80000;
            } else if (veh_id_atk >= 0 && get_triad(veh_id_atk) == TRIAD_AIR
                && has_abil(Vehs[veh_id_atk].proto_id, ABL_AIR_SUPERIORITY)
                && get_proto_triad(proto_id_def) == TRIAD_AIR && base_id_def < 0
                && !(bit_at(x_def, y_def) & BIT_AIRBASE)
                && !stack_check(veh_id_def, 6, ABL_CARRIER, -1, -1)
                && !is_proto_missile(proto_id_def) && !is_missile(veh_id_atk)) {
                def_modifier += 0x80000;
            } else if (check_artillery) {
                if (can_arty(proto_id_def, true)) {
                    def_modifier += 0x80000;
                }
            } else if ((!stack_check(veh_id_def, 3, TRIAD_AIR, -1, -1) || base_id_def >= 0
                || bit_at(x_def, y_def) & BIT_AIRBASE
                || stack_check(veh_id_def, 6, ABL_CARRIER, -1, -1))
                && Vehs[i].state & VSTATE_DESIGNATE_DEFENDER) {
                def_modifier += 0x80000;
            }
            int defense = i + (def_modifier << 11);
            if (defense > defender_search) {
                defender_search = defense;
                best_def_veh_id = i;
            }
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
        const int y = Vehs[killer_id].y;
        const int x = Vehs[killer_id].x;
        if (on_map(x, y)
            && VehPrototypes[Vehs[vehicle_id].proto_id].plan == PLAN_ALIEN_ARTIFACT
            && !Chassis[VehPrototypes[Vehs[killer_id].proto_id].chassis_id].missile
            && (!is_ocean(x, y) || stack_check(killer_id, 8, -1, -1, -1) > 0
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

// Inlined: the doubled vector_dist halved once.
static inline int patrol_dist(int x_a, int y_a, int x_b, int y_b) {
    return (x_dist(x_a, x_b) + abs(y_a - y_b)) >> 1;
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
            const int power = VehPrototypes[proto_id].plan == PLAN_ALIEN_ARTIFACT
                ? 1 : range(VehPrototypes[proto_id].reactor_id, 1, 100) * 10;
            const int score = weight * range(power - Vehs[vehicle_id].dmg_incurred, 0, 9999);
            if (score >= best_score) {
                best_score = score;
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

// Complete combat-resolution owner for the original 506AF0..50B893 span.
// Helpers share the original mutable argument/local state and genuine owners.
// The local [-0x138] model uses a provisional 0xA0 collision-free interval;
// the original declared extent and malformed-name overflow effects remain
// unproved. No matching or runtime claim follows from this representation.

#include <stdint.h>
#include <string.h>

#include "battlewin.h"
#include "council.h"

// Retained fragment: battle-fight-nerve-cancel-v1.cpp
// Private typed translation of the cancellation epilogue shared by the two
// native nerve-gas confirmation branches. Call only when their specific
// `popp(...)` return is zero. The branch itself is not represented here.
//
// Callbacks have already run when this function begins. Deliberately reload
// NetDaemon timing storage now rather than carrying a pre-popp snapshot.
static int battle_fight_nerve_gas_cancel(int attacker_id,
                                         int movement_charge) {
    g_BattleWin.pulse_timer();

    if ((NetDaemonNet.channels_[0].leading_words_[3] & 2) != 0) {
        const int8_t signed_charge = static_cast<int8_t>(movement_charge);
        Vehs[attacker_id].moves_expended = static_cast<uint8_t>(
            static_cast<int>(Vehs[attacker_id].moves_expended) -
            static_cast<int>(signed_charge));
    }

    Vehs[attacker_id].order = 0;
    Vehs[attacker_id].state &= 0xFCFFBDFFU;
    ConsoleStatusWin.reset();
    ConsoleStatusWin.redraw();
    return 1;
}
// Retained fragment: battle-rounds-subagent/battle-fight-precombat-gaps-v4.cpp
// Corrected private v4 translation of two pre-combat gaps. This supersedes
// v3 only as a proposed composition source; v3 remains immutable evidence.

// Native 0x507132..0x5073D8. `permission_slot` is mutable native arg [ebp+1c].
static int battle_fight_nerve_gas_precombat_v4(
    int attacker_id, int defender_id, int attacker_faction,
    int defender_faction, int &permission_slot, int &nerve_gas_active) {
    nerve_gas_active = 0;
    const int attacker_proto = Vehs[attacker_id].proto_id;
    const int defender_proto = Vehs[defender_id].proto_id;
    if (!has_abil(attacker_proto, ABL_NERVE_GAS) ||
        Weapon[VehPrototypes[attacker_proto].weapon_id].offense_rating < 0 ||
        Armor[VehPrototypes[defender_proto].armor_id].defense_rating < 0)
        return 0;
    if (IsMultiplayerNet &&
        (NetDaemonNet.channels_[0].leading_words_[3] & 1) == 0)
        return 0;

    const uint32_t attacker_bit =
        1u << (static_cast<unsigned>(attacker_faction) & 0x1Fu);
    const bool attacker_is_human = (GameDataStateStorage.fields_.lock_masks_.enable_mask_ & attacker_bit) != 0;
    Vehs[attacker_id].state &= ~VSTATE_USED_NERVE_GAS;
    permission_slot = static_cast<int>(Vehs[attacker_id].state);

    // Native 0x5071E1/1E7 enters this prompt only for the local human
    // attacker. The defender's humanity is deliberately not part of it.
    if (attacker_faction == ConsoleGlobal.faction_id_ && attacker_is_human) {
        parse_says(0, VehPrototypes[attacker_proto].veh_name, -1, -1);
        parse_say(1, Ability[abil_index(ABL_NERVE_GAS)].name, -1, -1);
        g_BattleWin.stop_timer();
        if (popp(ScriptPopupFile, "USENERVE", 0, "biohazd_sm.pcx", 0) != 0)
            Vehs[attacker_id].state |= VSTATE_USED_NERVE_GAS;
        g_BattleWin.pulse_timer();
    } else if (!attacker_is_human) {
        // The native outer routing is local+human => prompt, otherwise route
        // onward; 0x50728D then explicitly skips policy for remaining humans.
        // Consequently this AI policy subblock executes iff attacker is AI.
        const bool defender_is_human =
            (GameDataStateStorage.fields_.lock_masks_.enable_mask_ & (1u << (static_cast<unsigned>(defender_faction) &
                                     0x1Fu))) != 0;
        const uint32_t relation =
            PlayersData[attacker_faction].diplo_treaties[defender_faction];
        if ((relation & DTREATY_ATROCITY_VICTIM) != 0 ||
            (PlayersData[attacker_faction].flags &
             PFLAG_COMMIT_ATROCIT_WANTONLY) != 0 ||
            (defender_is_human &&
             GameDataStateStorage.fields_.council_resolution_count_[PROP_REPEAL_UN_CHARTER] >
                 GameDataStateStorage.fields_.council_resolution_count_[PROP_REINSTATE_UN_CHARTER])) {
            Vehs[attacker_id].state |= VSTATE_USED_NERVE_GAS;
        } else if (defender_is_human &&
                   (relation & DTREATY_WANT_REVENGE) != 0) {
            int rivalry_adjustment = 0;
            if ((GameDataStateStorage.fields_.game_rules_ & RULES_INTENSE_RIVALRY) != 0 &&
                (PlayersData[attacker_faction].ranking <
                     PlayersData[defender_faction].ranking ||
                 PlayersData[attacker_faction].diplo_unk4[defender_faction] <
                     PlayersData[defender_faction].diplo_unk4[attacker_faction])) {
                rivalry_adjustment = (PlayersData[attacker_faction].ai_fight + 1) * 3;
            }
            if (PlayersData[defender_faction].integrity_blemishes +
                    rivalry_adjustment >= 6) {
                Vehs[attacker_id].state =
                    static_cast<uint32_t>(permission_slot) | VSTATE_USED_NERVE_GAS;
            }
        }
    }

    if (IsMultiplayerNet) synch_veh(attacker_id);
    if ((NetDaemonNet.channels_[0].leading_words_[3] & 2) != 0 &&
        (Vehs[attacker_id].state & VSTATE_USED_NERVE_GAS) != 0)
        nerve_gas_active = 1;
    return 1;
}

// Native 0x50752A..0x5077D6; identical to the disassembly-validated v3
// translation. Caller supplies the enclosing local, initialized to zero.
static bool battle_fight_precombat_diplomacy_v4(
    int attacker_id, int defender_id, int attacker_faction,
    int defender_faction, int &diplomacy_report_mode) {
    Vehs[attacker_id].state |= VSTATE_UNK_40;
    if (VehPrototypes[Vehs[attacker_id].proto_id].plan == PLAN_INFO_WARFARE ||
        VehPrototypes[Vehs[defender_id].proto_id].plan == PLAN_ALIEN_ARTIFACT)
        return false;

    const uint32_t defender_human_bit =
        1u << (static_cast<unsigned>(defender_faction) & 0x1Fu);
    if ((GameDataStateStorage.fields_.lock_masks_.enable_mask_ & defender_human_bit) != 0) {
        const uint32_t attacker_to_defender =
            PlayersData[attacker_faction].diplo_treaties[defender_faction];
        const uint32_t defender_to_attacker =
            PlayersData[defender_faction].diplo_treaties[attacker_faction];
        const bool notify =
            (attacker_to_defender & (DTREATY_TREATY | DTREATY_TRUCE)) != 0 ||
            ((attacker_to_defender & DTREATY_COMMLINK) == 0 &&
             (defender_to_attacker & DTREATY_UNK_800) != 0 &&
             (attacker_to_defender & DTREATY_VENDETTA) == 0);
        if (notify) {
            if (defender_faction == ConsoleGlobal.faction_id_) {
                GenderDefault = Players[attacker_faction].is_leader_female;
                PluralityDefault = 0;
                parse_says(0, Players[attacker_faction].title_leader, -1, -1);
                GenderDefault = Players[attacker_faction].is_leader_female;
                PluralityDefault = 0;
                parse_says(1, Players[attacker_faction].name_leader, -1, -1);
                GenderDefault = Players[attacker_faction].noun_gender;
                PluralityDefault = Players[attacker_faction].is_noun_plural;
                parse_says(2, Players[attacker_faction].noun_faction, -1, -1);
                NetMsgPrimary.pop("PACTATTACKING", -5000, 0, 0);
            } else if (IsMultiplayerPBEM) {
                GenderDefault = Players[attacker_faction].is_leader_female;
                PluralityDefault = 0;
                parse_says(0, Players[attacker_faction].title_leader, -1, -1);
                GenderDefault = Players[attacker_faction].is_leader_female;
                PluralityDefault = 0;
                parse_says(1, Players[attacker_faction].name_leader, -1, -1);
                GenderDefault = Players[attacker_faction].noun_gender;
                PluralityDefault = Players[attacker_faction].is_noun_plural;
                parse_says(2, Players[attacker_faction].noun_faction, -1, -1);
                POP2("PACTATTACKED", 0, -1 - defender_faction);
            }
            PlayersData[attacker_faction].diplo_spoke[defender_faction] =
                GameDataStateStorage.fields_.turn_current_num_;
        }
        return true;
    }

    const bool treaty =
        ((PlayersData[defender_faction].diplo_treaties[attacker_faction] |
          PlayersData[attacker_faction].diplo_treaties[defender_faction]) &
         DTREATY_TREATY) != 0;
    const int local_faction = ConsoleGlobal.faction_id_;
    if (treaty && attacker_faction != local_faction &&
        defender_faction != local_faction) {
        if ((PlayersData[local_faction].diplo_treaties[defender_faction] &
             DTREATY_PACT) != 0) {
            diplomacy_report_mode = 2;
        } else if ((PlayersData[local_faction].diplo_treaties[attacker_faction] &
                    DTREATY_PACT) != 0) {
            diplomacy_report_mode = 3;
        } else if ((GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW) != 0) {
            diplomacy_report_mode = 1;
        }
    }
    return true;
}
// Retained fragment: battle-rounds-subagent/battle-fight-precombat-v2.cpp
// Private, body-ready C++ blocks for battle_fight pre-combat transaction.
// This packet owns only the explicitly bounded native blocks listed below;
// it does not claim to fill every edge from 0x507132 through 0x50835D.

// Native 0x5073D8..0x50752A, ordinary flags/movement charge block.
// Inputs correspond to live enclosing locals: target_base_id [-0x20],
// movement_charge [-0x24], source_mode [+0x18], and attacker combat
// strength [-0x14]. Attacker combat is a reference
// because the native writes attacker combat strength in place.
static int battle_fight_precombat_commit_v2(
    int attacker_id, int defender_id, int attacker_faction,
    int target_base_id, int movement_charge,
    int source_mode, int &attacker_combat) {
    if (target_base_id >= 0) {
        Vehs[attacker_id].state &= ~VSTATE_UNK_40000;
    }

    // 0x5073EF..0x5074DA. 0x93E904 is the existing
    // NetDaemonNet.lock_state_[1].leading_words_[3] member (netdaemon.cpp's
    // address/layout notes), not a new global alias.
    if ((Vehs[attacker_id].state & VSTATE_UNK_1000) == 0 &&
        (NetDaemonNet.channels_[0].leading_words_[3] & 2) != 0) {
        if (attacker_faction != 0 &&
            VehPrototypes[Vehs[attacker_id].proto_id].plan != 15 &&
            VehPrototypes[Vehs[attacker_id].proto_id].plan !=
                PLAN_COLONIZATION) {
            int remaining_moves = range(
                speed(attacker_id, 0) - Vehs[attacker_id].moves_expended,
                0, 999);
            if (Rules.move_rate_roads >= 0) {
                if (remaining_moves > Rules.move_rate_roads)
                    remaining_moves = Rules.move_rate_roads;
            } else {
                remaining_moves = 0;
            }

            // Preserve native two IDIVs, signed intermediate arithmetic and
            // unguarded denominator: no new zero-rate policy is invented.
            attacker_combat =
                (remaining_moves * attacker_combat) / Rules.move_rate_roads;
            const int modifier =
                ((remaining_moves - Rules.move_rate_roads) * 100) /
                Rules.move_rate_roads;
            if (modifier != 0 && VehBattleModCount[0] <= 3)
                add_bat(0, modifier, label_get(0x153));
        }

        if (source_mode != 1) {
            Vehs[attacker_id].moves_expended = static_cast<uint8_t>(
                static_cast<unsigned int>(Vehs[attacker_id].moves_expended) +
                static_cast<unsigned int>(movement_charge));
        }
    }

    // Native reloads both vehicle state words after the intervening calls.
    Vehs[attacker_id].state =
        (Vehs[attacker_id].state & ~VSTATE_UNK_1000) | VSTATE_UNK_4;
    Vehs[defender_id].state |= VSTATE_UNK_4;
    if ((NetDaemonNet.channels_[0].leading_words_[3] & 2) == 0)
        return 0; // exact native early return at 0x507521
    Vehs[attacker_id].state |= VSTATE_UNK_40;
    return 1; // fall-through to 0x50752A
}

// Native 0x5077D6..0x5078DC act-of-aggression call and territory-pact sweep.
// All treaty_on and cause_friction calls are retained in native order, with
// the relation reread after treaty mutation.
static void battle_fight_territory_pact_sweep_v2(
    int attacker_faction, int defender_faction, int target_x, int target_y) {
    act_of_aggression(attacker_faction, defender_faction);
    Map *const target = map_loc(target_x, target_y);
    for (int candidate = 1; candidate < MaxPlayerNum; ++candidate) {
        if (candidate == attacker_faction || candidate == defender_faction)
            continue;
        if (target->territory != candidate) continue;
        if ((PlayersData[attacker_faction].diplo_treaties[candidate] &
             DTREATY_PACT) == 0) continue;

        const uint32_t defender_view =
            PlayersData[defender_faction].diplo_treaties[candidate];
        if ((defender_view & DTREATY_VENDETTA) != 0 ||
            (defender_view & DTREATY_COMMLINK) == 0) continue;
        if (PlayersData[defender_faction].mil_strength_1 <=
            PlayersData[candidate].mil_strength_1) continue;

        treaty_on(candidate, defender_faction, DTREATY_WANT_TO_TALK);
        if ((PlayersData[candidate].diplo_treaties[defender_faction] &
             0x00800000u) != 0)
            cause_friction(defender_faction, candidate, 3);
    }
}
// Retained fragment: battle-rounds-subagent/battle-fight-preprompt-residual-v3.cpp
// Private typed residual fragments for battle_fight. These keep the native
// local boundary explicit; they are not a complete battle_fight body.

static int32_t battle_fight_wrap_mul_residual_v2(int32_t left, int32_t right) {
    const uint32_t bits = static_cast<uint32_t>(left) *
                          static_cast<uint32_t>(right);
    if (bits <= 0x7FFFFFFFu) return static_cast<int32_t>(bits);
    return -1 - static_cast<int32_t>(0xFFFFFFFFu - bits);
}

static int battle_fight_ceil_half_residual_v2(int value) {
    const int quotient = value / 2;
    return quotient + ((value % 2) > 0 ? 1 : 0);
}

// Native 0x5078DC..0x5080FC. Result values describe the exact continuation:
// 0 => leave live pressure products for 0x5080FC reduction;
// 1 => native returned 1 after declining a confirmation;
// 2 => native branched directly to 0x50835D (caller resumes that shared tail).
// `*_remaining_power` and `*_power_factor` are the already-populated locals
// [-0x60], [-0x68], [-0x5c], [-0x70]; their native order is retained below.
static int battle_fight_pre_prompt_residual_v2(
    int attacker_id, int defender_id,
    int attacker_faction, int defender_faction,
    int x_coord, int y_coord, int source_mode,
    int target_base_id, int combat_type, int interceptor_result,
    int movement_charge, int &attacker_combat, int &defender_combat,
    int attacker_remaining_power, int attacker_power_factor,
    int defender_remaining_power, int defender_power_factor,
    int nerve_gas_active, int &local_base_context,
    int &attacker_power_product, int &defender_power_product,
    int *attacker_proto_snapshot_out = 0,
    int *defender_proto_snapshot_out = 0) {
    if (nerve_gas_active != 0) {
        attacker_combat += attacker_combat / 2;
        if (VehBattleModCount[0] <= 3)
            add_bat(0, 50, label_get(0x155));
    }

    if (IsMultiplayerNet)
        log_say("Battle Resolution!", attacker_id, combat_type, 0);

    const int attacker_proto = Vehs[attacker_id].proto_id;
    const int defender_proto = Vehs[defender_id].proto_id;
    if (attacker_proto_snapshot_out != 0)
        *attacker_proto_snapshot_out = attacker_proto;
    if (defender_proto_snapshot_out != 0)
        *defender_proto_snapshot_out = defender_proto;
    const int attacker_chassis = VehPrototypes[attacker_proto].chassis_id;
    const int defender_chassis = VehPrototypes[defender_proto].chassis_id;
    const uint32_t attacker_bit =
        1u << (static_cast<unsigned>(attacker_faction) & 0x1Fu);
    const uint32_t defender_bit =
        1u << (static_cast<unsigned>(defender_faction) & 0x1Fu);
    const bool attacker_is_human = (GameDataStateStorage.fields_.lock_masks_.enable_mask_ & attacker_bit) != 0;
    const bool defender_is_human = (GameDataStateStorage.fields_.lock_masks_.enable_mask_ & defender_bit) != 0;
    const int difficulty_floor = attacker_faction == 0 ? 3 : 2;

    // Native's two asymmetric human-vs-AI difficulty corrections.
    if (PlayersData[defender_faction].diff_level < difficulty_floor &&
        attacker_is_human && !defender_is_human) {
        const int numerator =
            (PlayersData[defender_faction].diff_level + 1) * attacker_combat;
        attacker_combat = (numerator + 3) / 4;
    }
    if (PlayersData[attacker_faction].diff_level < difficulty_floor &&
        attacker_is_human && !defender_is_human) {
        const int numerator =
            (4 - PlayersData[attacker_faction].diff_level) * attacker_combat;
        attacker_combat = battle_fight_ceil_half_residual_v2(numerator);
    }

    // Player-0's terrainformer-defense rule, visibility split, and local-base
    // consequences. Local faction 0 is the native alien faction, not UI local.
    if (attacker_faction == 0) {
        if (!defender_is_human) {
            if (VehPrototypes[defender_proto].plan != PLAN_TERRAFORMING) {
                attacker_combat =
                    ((PlayersData[defender_faction].diff_level + 8) *
                     attacker_combat) / 16;
            } else {
                attacker_combat /= 2;
                const uint32_t local_bit =
                    1u << (static_cast<unsigned>(ConsoleGlobal.faction_id_) & 0x1Fu);
                const uint16_t defender_stack_state = static_cast<uint16_t>(
                    Vehs[defender_id].next_veh_id_stack);
                if ((GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW) == 0 &&
                    !IsMultiplayerNet &&
                    !(defender_faction == ConsoleGlobal.faction_id_ &&
                      (defender_stack_state & 0x440u) != 0x440u) &&
                    (Vehs[defender_id].visibility & local_bit) == 0)
                    attacker_combat /= 2;
            }
        }

        if (target_base_id >= 0) {
            if (defender_faction == ConsoleGlobal.faction_id_)
                local_base_context = 1;
            if (PlayersData[defender_faction].current_num_bases <= 1)
                attacker_combat /= 2;
            if (GameDataStateStorage.fields_.turn_current_num_ <= 0x31) attacker_combat /= 2;
            int bit_offset = 0;
            int bit_mask = 0;
            bitmask(1, &bit_offset, &bit_mask);
            (void)bit_offset;
            if ((Bases[target_base_id].intake_2[RSC_PSI] & bit_mask) != 0 &&
                PlayersData[defender_faction].current_num_bases <= 3)
                attacker_combat /= 2;
        }
    }

    if (GameDataStateStorage.fields_.turn_current_num_ < 0x0F) {
        if (attacker_faction == 0) attacker_combat /= 3;
        if (defender_faction == 0) defender_combat /= 2;
    }
    if (target_base_id >= 0 && Chassis[attacker_chassis].triad == TRIAD_AIR)
        Bases[target_base_id].state |= BSTATE_UNK_100000;

    // This is the native human/local/no-interceptor status presentation gate.
    const bool odds_prompt_gate =
        (NetDaemonNet.channels_[0].leading_words_[3] & 1) != 0 &&
        interceptor_result == 0 && attacker_is_human &&
        attacker_faction == ConsoleGlobal.faction_id_ && source_mode == 0 &&
        Chassis[attacker_chassis].missile == 0 &&
        (Vehs[attacker_id].state & (VSTATE_UNK_200 | VSTATE_EXPLORE)) == 0;

    attacker_power_product = battle_fight_wrap_mul_residual_v2(
        battle_fight_wrap_mul_residual_v2(attacker_remaining_power,
                                          attacker_power_factor),
        attacker_combat);
    defender_power_product = battle_fight_wrap_mul_residual_v2(
        battle_fight_wrap_mul_residual_v2(defender_remaining_power,
                                          defender_power_factor),
        defender_combat);

    // Native jumps directly to 0x507E7E when twice attack pressure reaches
    // defense pressure. The opposing edge first draws the battle view and
    // offers the BADIDEA abort confirmation.
    if (!odds_prompt_gate ||
        battle_fight_wrap_mul_residual_v2(attacker_power_product, 2) <
            defender_power_product) {
        if (odds_prompt_gate) {
            g_BattleWin.set_iface_mode();
            g_BattleWin.clear();
            ConsoleStatusWin.draw(attacker_id, combat_type,
                                  attacker_combat, defender_combat, 0);
            // Native writes ConsoleStatusWin +0x15D4 immediately after draw.
            ConsoleStatusWin.set_field_15D4(1);

            GenderDefault = Players[ConsoleGlobal.faction_id_].is_leader_female;
            PluralityDefault = 0;
            parse_says(0, Players[ConsoleGlobal.faction_id_].title_leader,
                       -1, -1);
            parse_says(1, Players[ConsoleGlobal.faction_id_].name_leader,
                       -1, -1);

            const bool attacker_is_local =
                attacker_faction == ConsoleGlobal.faction_id_;
            const bool defender_is_local =
                defender_faction == ConsoleGlobal.faction_id_;
            if ((attacker_is_local || defender_is_local) && !IsMultiplayerNet &&
                (get_offense_rating(attacker_id) < 0 ||
                 get_defense_rating(defender_id) < 0) &&
                tut_check2(2) != 0) {
                TutorialWindow.tut_map("PSICOMBAT", x_coord, y_coord,
                                       attacker_is_local ? attacker_id : combat_type,
                                       0, 1, -1, -1);
            }

            g_BattleWin.stop_timer();
            const int popup_result =
                popp(ScriptPopupFile, "BADIDEA", 0, "hasty_sm.pcx", 0);
            if (popup_result == 0)
                return battle_fight_nerve_gas_cancel(attacker_id,
                                                      movement_charge);
            g_BattleWin.pulse_timer();
            return 2; // accepted BADIDEA popup, native edge to 0x50835D
        }
        return 2; // no status gate: native's outer caller continues at 0x50835D
    }

    // Strength-report preference branch: build the two display strings and
    // leave their power products at the exact 0x5080FC reducer boundary.
    if ((GameDataStateStorage.fields_.game_more_preferences_ & MPREF_ADV_CONFIRM_ODDS_BF_ATTACKING) == 0)
        return 2; // native branch 0x507E86 -> 0x50835D

    g_BattleWin.set_iface_mode();
    g_BattleWin.clear();
    ConsoleStatusWin.draw(attacker_id, combat_type,
                          attacker_combat, defender_combat, 0);
    // Native writes ConsoleStatusWin +0x15D4 immediately after draw.
    ConsoleStatusWin.set_field_15D4(1);
    GenderDefault = Players[ConsoleGlobal.faction_id_].is_leader_female;
    PluralityDefault = 0;
    parse_says(0, Players[ConsoleGlobal.faction_id_].title_leader, -1, -1);
    parse_says(1, Players[ConsoleGlobal.faction_id_].name_leader, -1, -1);

    StringTemp[0] = '\0';
    say_num(attacker_combat >> 8);
    says(".");
    say_num(((attacker_combat & 0xFF) * 10) >> 8);
    parse_says(2, StringTemp, -1, -1);
    StringTemp[0] = '\0';
    say_num(defender_combat >> 8);
    says(".");
    say_num(((defender_combat & 0xFF) * 10) >> 8);
    parse_says(3, StringTemp, -1, -1);
    return 0; // native falls into the 0x5080FC paired-product reducer
}

// Native 0x5082A2..0x50835D consumes the two reduced products produced by the
// paired reduction boundary. Return 1 means the caller must return 1 now;
// return 0 means the confirmation was accepted and execution joins at 835D.
static int battle_fight_strength_prompt_residual_v2(
    int attacker_id, int defender_id, int movement_charge,
    int attacker_power_product, int defender_power_product) {
    g_BattleWin.stop_timer();
    parse_num(0, attacker_power_product);
    parse_num(1, defender_power_product);

    const char *prompt =
        (Vehs[attacker_id].dmg_incurred != 0 ||
         Vehs[defender_id].dmg_incurred != 0)
            ? "GOODIDEA2" : "GOODIDEA";
    if (popp(ScriptPopupFile, prompt, 0, "hasty_sm.pcx", 0) != 0) {
        g_BattleWin.pulse_timer(); // native 0x508358, then join 0x50835D
    return 0;
}
    // Decline: reload the multiplayer slot after popp's callback, then apply
    // the low-byte signed movement rollback and exact state/order cleanup.
    g_BattleWin.pulse_timer();
    if ((NetDaemonNet.channels_[0].leading_words_[3] & 2) != 0) {
        const int8_t signed_charge = static_cast<int8_t>(movement_charge);
        Vehs[attacker_id].moves_expended = static_cast<uint8_t>(
            static_cast<int>(Vehs[attacker_id].moves_expended) -
            static_cast<int>(signed_charge));
    }
    Vehs[attacker_id].order = 0;
    Vehs[attacker_id].state &= ~(VSTATE_UNK_200 | VSTATE_EXPLORE |
                                  VSTATE_UNK_1000000 | VSTATE_UNK_2000000);
    ConsoleStatusWin.reset();
    ConsoleStatusWin.redraw();
    return 1; // native return 1 at 0x50834C..57
}

// Retained fragment: battle-capture-subagent/battle-fight-strength-pair-reduction-v5.cpp
// Corrected immutable source fragment for native 0x005080FC..0x005082A2.
// The values are the live paired strength products in native ESI/EDI at the
// first common-divisor test. Native repeatedly removes a shared divisor,
// then independently applies paired magnitude reductions. Both references
// are output locals consumed by the continuation after 0x5082A2.
static void battle_fight_reduce_strength_pair_v5(
    int &attacker_strength_product, int &defender_strength_product) {
    int common_divisor = 2;
    for (;;) {
        const int smaller = attacker_strength_product < defender_strength_product
                          ? attacker_strength_product
                          : defender_strength_product;
        if (common_divisor >= smaller) break;
        if (attacker_strength_product == 0 ||
            defender_strength_product == 0 ||
            attacker_strength_product % common_divisor != 0 ||
            defender_strength_product % common_divisor != 0) {
            ++common_divisor;
            continue;
        }

        attacker_strength_product /= common_divisor;
        defender_strength_product /= common_divisor;
        if (attacker_strength_product != 0) continue;
        ++common_divisor;
    }

    // Exact paired thresholds/divisors from pinned Kuna native-source lines
    // 137126..137142. These source-confirmed divisors correct v4's five
    // mistaken entries (the 20000/10000 stage was already correct).
    while (attacker_strength_product >= 20000 &&
           defender_strength_product >= 20000) {
        attacker_strength_product /= 10000;
        defender_strength_product /= 10000;
    }
    while (attacker_strength_product >= 10000 &&
           defender_strength_product >= 10000) {
        attacker_strength_product /= 5000;
        defender_strength_product /= 5000;
    }
    while (attacker_strength_product >= 2000 &&
           defender_strength_product >= 2000) {
        attacker_strength_product /= 1000;
        defender_strength_product /= 1000;
    }
    while (attacker_strength_product >= 1000 &&
           defender_strength_product >= 1000) {
        attacker_strength_product /= 500;
        defender_strength_product /= 500;
    }
    while (attacker_strength_product >= 200 &&
           defender_strength_product >= 200) {
        attacker_strength_product /= 100;
        defender_strength_product /= 100;
    }
    while (attacker_strength_product >= 100 &&
           defender_strength_product >= 100) {
        attacker_strength_product /= 50;
        defender_strength_product /= 50;
    }
}
// Retained fragment: battle-fight-artillery-roll-v2.cpp
// Immutable v2 of the 50856C..50876A typed roll fragment. Unlike v1, this
// returns the native minimum-survivor threshold because the following
// animation decision compares the defender's pre-hit damage against it.
struct BattleFightArtilleryRoll {
    int damage;
    int minimum_remaining;
};

static BattleFightArtilleryRoll battle_fight_artillery_roll_v2(
    int defender_id, int x_coord, int y_coord, int attacker_offense,
    int defender_defense, int target_base_id) {
    int denominator = Rules.artillery_dmg_denom *
                      defender_defense;
    if (denominator < 1) denominator = 1;
    else if (denominator > 9999999) denominator = 9999999;

    int damage = Rules.artillery_dmg_num *
                 attacker_offense / denominator + 1;
    if (damage < 1) damage = 1;
    else if (damage > 999) damage = 999;
    if (damage != 1) damage = std::rand() % damage;
    else damage = 0;

    const VehPrototype &proto = VehPrototypes[Vehs[defender_id].proto_id];
    const int chassis_id = proto.chassis_id;
    const Map &tile = *map_loc(x_coord, y_coord);
    int maximum_damage_percent;
    if (target_base_id >= 0 || (tile.bit & 0x800u) != 0) {
        maximum_damage_percent = static_cast<int>(
            Rules.max_pct_dmg_art_vs_unit_base_bnkr);
    } else if (Chassis[chassis_id].triad != TRIAD_LAND) {
        maximum_damage_percent = static_cast<int>(
            Rules.max_pct_dmg_art_vs_units_sea);
    } else {
        maximum_damage_percent = static_cast<int>(
            Rules.max_pct_dmg_art_vs_units_open);
    }

    int minimum_remaining;
    if (target_base_id >= 0 || (tile.bit & 0x800u) != 0) {
        const int hp = proto.plan == PLAN_ALIEN_ARTIFACT
                     ? 1 : range(static_cast<int>(proto.reactor_id), 1, 100) * 10;
        minimum_remaining =
            ((100 - maximum_damage_percent) * hp + 99) / 100;
    } else if (proto.plan == PLAN_ALIEN_ARTIFACT) {
        minimum_remaining = (199 - maximum_damage_percent) / 100;
    } else if (proto.reactor_id > 100) {
        minimum_remaining =
            ((100 - maximum_damage_percent) * 1000 + 99) / 100;
    } else {
        const int hp = range(static_cast<int>(proto.reactor_id), 1, 100) * 10;
        minimum_remaining =
            ((100 - maximum_damage_percent) * hp + 99) / 100;
    }

    int remaining = static_cast<int>(proto.plan == PLAN_ALIEN_ARTIFACT
        ? 1 : range(static_cast<int>(proto.reactor_id), 1, 100) * 10) -
        Vehs[defender_id].dmg_incurred;
    if (remaining < 0) remaining = 0;
    else if (remaining > 9999) remaining = 9999;
    while (damage > 0 && remaining - damage < minimum_remaining) --damage;

    BattleFightArtilleryRoll result;
    result.damage = damage;
    result.minimum_remaining = minimum_remaining;
    return result;
}
// Retained fragment: battle-fight-artillery-cleanup-v1.cpp
// Private typed translation of the source-mode artillery cleanup at native
// 5087E0..508B4F. This intentionally takes the original faction/base inputs:
// stack-kill callbacks can mutate the vehicles and the target stack is
// reacquired after every kill, as in the executable.
static int battle_fight_artillery_cleanup(int target_x,
                                          int target_y,
                                          int attacker_faction,
                                          int defender_faction,
                                          int target_base_id) {
    int vehicle_id = stack_fix(veh_at(target_x, target_y));
    while (vehicle_id >= 0) {
        const int proto_id = Vehs[vehicle_id].proto_id;
        const VehPrototype &proto = VehPrototypes[proto_id];
        const int hit_points = proto.plan == PLAN_ALIEN_ARTIFACT
                             ? 1
                             : range(static_cast<int>(proto.reactor_id),
                                     1, 100) * 10;
        const int remaining = hit_points - Vehs[vehicle_id].dmg_incurred;
        if (remaining > 0) {
            vehicle_id = Vehs[vehicle_id].next_veh_id_stack;
            continue;
        }

        stack_veh(vehicle_id, 2);
        stack_kill(vehicle_id);
        vehicle_id = stack_fix(veh_at(target_x, target_y));
        ++PlayersData[attacker_faction].diplo_unk3[defender_faction];
        ++PlayersData[attacker_faction].diplo_unk4[defender_faction];
    }

    draw_tile_fixup(target_x, target_y, 1, 2);
    if (target_base_id >= 0 &&
        (GameDataStateStorage.fields_.lock_masks_.enable_mask_ &
         (1u << (static_cast<unsigned>(defender_faction) & 0x1Fu))) == 0) {
        base_reset(target_base_id, 0);
    }
    return 0;
}
// Retained fragment: battle-fight-artillery-mode-v1.cpp
// Private typed translation of the native source-mode (combat_type == 1)
// stack transaction, 0x50835D..0x508B4F. It is still an insertion helper,
// not a complete battle_fight body. The parent body must pass its captured
// source coordinates, best defender, target base and report predicate, and
// the real StatusWin accessor for its +0x15D4 write.
//
// This helper preserves live stack links and the callback ordering. In
// particular it reads the next vehicle after the UI callbacks and reacquires
// the complete target stack after each kill.
static bool battle_fight_should_show_artillery(int attacker_id,
                                                int defender_id,
                                                int attacker_faction,
                                                int defender_faction) {
    const int local_faction = ConsoleGlobal.faction_id_;
    bool show = attacker_faction == local_faction ||
                defender_faction == local_faction ||
                (GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW) != 0;
    const uint32_t local_bit =
        1u << (static_cast<unsigned>(local_faction) & 0x1Fu);
    const bool attacker_is_locally_visible =
        (Vehs[attacker_id].visibility & local_bit) != 0;
    const bool defender_is_locally_visible =
        (Vehs[defender_id].visibility & local_bit) != 0;
    const bool any_relevant_unit_seen =
        (attacker_faction == local_faction &&
        (Vehs[attacker_id].flags & 0x440u) != 0x440u) ||
        attacker_is_locally_visible ||
        (defender_faction == local_faction &&
         (Vehs[defender_id].flags & 0x440u) != 0x440u) ||
        defender_is_locally_visible;
    const uint32_t local_to_attacker =
        PlayersData[local_faction].diplo_treaties[attacker_faction];
    const bool visibility_preference = (local_to_attacker & 1u) != 0
                                     ? (GameDataStateStorage.fields_.game_preferences_ & 0x8000u) != 0
                                     : (GameDataStateStorage.fields_.game_preferences_ & 4u) != 0;
    if (visibility_preference) show = show || any_relevant_unit_seen;
    return show;
}

static int battle_fight_source_mode(int attacker_id,
                                   int selected_defender_id,
                                   int attacker_faction,
                                   int defender_faction,
                                   int target_x,
                                   int target_y,
                                   int source_x,
                                   int source_y,
                                   int target_base_id) {
    const bool show_report = battle_fight_should_show_artillery(
        attacker_id, selected_defender_id, attacker_faction, defender_faction);
    add_goal(defender_faction, 0x10, 4, target_x, target_y, -1);
    add_goal(defender_faction, 0, 4, source_x, source_y, -1);
    if (target_base_id >= 0) {
        Bases[target_base_id].state |= BSTATE_UNK_200000;
        Bases[target_base_id].event |= BEVENT_UNK_100;
    }

    int first_defender = selected_defender_id;
    while (first_defender >= 0 &&
           Vehs[first_defender].prev_veh_id_stack >= 0) {
        first_defender = Vehs[first_defender].prev_veh_id_stack;
    }

    int defender_id = first_defender;
    while (defender_id >= 0) {
        const int defender_proto_id = Vehs[defender_id].proto_id;
        const int attacker_proto_id = Vehs[attacker_id].proto_id;
        const Map &target_tile = *map_loc(target_x, target_y);
        const bool may_be_hit =
            Chassis[VehPrototypes[defender_proto_id].chassis_id].triad !=
                TRIAD_AIR ||
            has_abil(attacker_proto_id, ABL_AIR_SUPERIORITY) ||
            target_base_id >= 0 || (target_tile.bit & 0x40000u) != 0;

        if (may_be_hit) {
            veh_skip(attacker_id);
            int attacker_offense;
            int defender_defense;
            battle_compute(attacker_id, defender_id, &attacker_offense,
                           &defender_defense, 1);
            const BattleFightArtilleryRoll roll =
                battle_fight_artillery_roll_v2(
                    defender_id, target_x, target_y, attacker_offense,
                    defender_defense, target_base_id);

            int boom_after_hit = 0;
            if (show_report) {
                VehDrawSelection689F2C = defender_id;
                Vehs[attacker_id].visibility |= static_cast<uint8_t>(
                    1u << (static_cast<unsigned>(defender_faction) & 0x1Fu));
                VehDrawSelection689F28 = attacker_id;
                if (ConsoleGlobal.focus(target_x, target_y,
                                        attacker_faction) == 0) {
                    draw_tile(source_x, source_y, 2);
                    draw_tile(target_x, target_y, 2);
                }
                g_BattleWin.set_iface_mode();
                g_BattleWin.clear();
                ConsoleStatusWin.draw(attacker_id, defender_id,
                                      attacker_offense, defender_defense, 1);
                // Pinned 0x50888F writes the StatusWin member, not local -0x88.
                ConsoleStatusWin.set_field_15D4(1);
                flush_input();

                const Veh &target_before_damage = Vehs[defender_id];
                if (roll.minimum_remaining == 0 ||
                    target_before_damage.dmg_incurred !=
                        roll.minimum_remaining ||
                    target_before_damage.next_veh_id_stack < 0) {
                    boom(target_x, target_y, 0);
                    boom_after_hit = 1;
                }
            }

            // This is a byte store in the image. Keep the wrap/truncation
            // explicit rather than allowing integer promotion to widen it.
            Vehs[defender_id].dmg_incurred = static_cast<uint8_t>(
                static_cast<unsigned>(Vehs[defender_id].dmg_incurred) +
                static_cast<unsigned>(roll.damage));
            Vehs[defender_id].state |= VSTATE_UNK_4;
            if (VehPrototypes[Vehs[defender_id].proto_id].plan ==
                PLAN_TERRAFORMING) {
                Vehs[defender_id].terraforming_turns = 0;
            }
            if (target_base_id < 0) {
                Vehs[defender_id].state |= VSTATE_UNK_8;
            }

            if (show_report) {
                ConsoleStatusWin.draw(attacker_id, defender_id,
                                      attacker_offense, defender_defense, 1);
                draw_tile(target_x, target_y, 2);
                VehDrawSelection689F28 = -1;
                VehDrawSelection689F2C = -1;
                if ((GameDataStateStorage.fields_.game_more_preferences_ & 0x10u) != 0) {
                    g_BattleWin.stop_timer();
                    popp(ScriptPopupFile, "BATTLEPAUSE", 0,
                         "batwon_sm.pcx", 0);
                    g_BattleWin.pulse_timer();
                    if (ConsoleGroupAction != 0) {
                        GameDataStateStorage.fields_.game_more_preferences_ &= ~0x10u;
                        prefs_save(0);
                    }
                } else {
                    const bool shift_down =
                        (GetAsyncKeyState(0x10) & 0x8000) != 0;
                    if (!shift_down && (GameDataStateStorage.fields_.game_preferences_ & 8u) == 0 &&
                        IsMultiplayerNet == 0 && boom_after_hit != 0) {
                        clock_wait(200);
                    }
                }
            }
        }

        // The native follows the possibly callback-mutated linked-list
        // member, rather than the next ID captured before presentation.
        defender_id = Vehs[defender_id].next_veh_id_stack;
    }

    return battle_fight_artillery_cleanup(
        target_x, target_y, attacker_faction, defender_faction,
        target_base_id);
}
// Retained fragment: battle-fight-post-prompt-v1.cpp
// Private typed insertion for native 0x50835D's shared prelude, executed
// after the nerve-gas prompts and before the artillery/ordinary-combat split.
// `base_name_copy` is the native 80-byte frame buffer retained for the later
// result/report path; the source owner must keep it live through that tail.
static void battle_fight_post_prompt_setup(int attacker_id,
                                           int attacker_faction,
                                           int defender_faction,
                                           int nerve_gas,
                                           int target_base_id,
                                           char *base_name_copy,
                                           char *native_text_buffer_minus_138 = 0) {
    if (nerve_gas != 0) {
        atrocity(attacker_faction, defender_faction,
                 target_base_id >= 0, 0);
    }

    // Native writes the distinct C-string destination at [-0x138] before
    // the mode split. Its provisional source representation is supplied by
    // the outer owner; this does not assert the original declared capacity.
    if (native_text_buffer_minus_138 != 0) {
        native_text_buffer_minus_138[0] = '\0';
        if (target_base_id >= 0) {
            strcpy(native_text_buffer_minus_138,
                   Bases[target_base_id].name_string);
        }
    }

    base_name_copy[0] = '\0';
    if (target_base_id >= 0) {
        strcpy(base_name_copy, Bases[target_base_id].name_string);
    }

    if (Vehs[attacker_id].order == 0x18) {
        Vehs[attacker_id].order = 0;
        Vehs[attacker_id].state &= ~(VSTATE_UNK_200 | VSTATE_EXPLORE |
                                     VSTATE_UNK_1000000 |
                                     VSTATE_UNK_2000000);
    }
}
// Retained fragment: battle-entry-subagent/battle-fight-normal-post-prompt-v1.cpp
// Private typed translation of the ordinary (non-source-mode) continuation
// from native 0x50835D to its fallthrough at 0x508B50. The shared effects at
// 0x50835D..0x5083D6 remain in battle_fight_post_prompt_setup(); include that
// definition before this fragment. This helper does not define battle_fight
// and does not replace the separate source-mode artillery owner.
#include <stdint.h>

enum BattleFightPostPromptRouteV1 {
    BATTLE_FIGHT_POST_PROMPT_SOURCE_MODE_V1 = 0,
    BATTLE_FIGHT_POST_PROMPT_NORMAL_V1 = 1
};

// `attacker_id` and `selected_defender_id` are numeric Veh IDs. `combat_type`
// is the current mutable +0x18 frame value. The three output references model
// the native live +0x1c, [-0x78], and [-0x88] storage, respectively.
static BattleFightPostPromptRouteV1 battle_fight_normal_post_prompt_v1(
    int attacker_id,
    int selected_defender_id,
    int attacker_faction_snapshot,
    int defender_faction_snapshot,
    int nerve_gas_active,
    int target_base_id,
    char *base_name_copy_80,
    char *native_text_buffer_minus_138,
    int &combat_type_plus_18,
    int &report_selector_plus_1c,
    int &visibility_gate_minus_78,
    int &status_animation_flag_minus_88) {
    battle_fight_post_prompt_setup(
        attacker_id, attacker_faction_snapshot, defender_faction_snapshot,
        nerve_gas_active, target_base_id, base_name_copy_80,
        native_text_buffer_minus_138);

    const int local_faction = ConsoleGlobal.faction_id_;

    // Native 0x5083D6..0x5083E0: selector defaults to 0 unless either
    // cached participant faction is local; independently clear the status
    // animation local before the mode split.
    status_animation_flag_minus_88 = 0;

    // Native 0x5083D6..0x508401: cached participant faction ownership turns
    // the selector on; otherwise bit 0x80 of GameState is the fallback.
    report_selector_plus_1c =
        (attacker_faction_snapshot == local_faction ||
         defender_faction_snapshot == local_faction ||
         (static_cast<uint8_t>(GameDataStateStorage.fields_.game_state_) & 0x80u) != 0) ? 1 : 0;

    // Native 0x508401..0x50846E: decide whether at least one participant is
    // locally seen. Owner-controlled units count as seen unless both native
    // status bits (0x40 and 0x400) are set; visibility is then tested as an
    // 8-bit faction mask. X86 masks variable shift counts to five bits.
    const uint8_t local_bit = static_cast<uint8_t>(
        1u << (static_cast<uint32_t>(local_faction) & 0x1Fu));
    const Veh &attacker = Vehs[attacker_id];
    const Veh &defender = Vehs[selected_defender_id];
    const int attacker_seen_by_owner =
        attacker.faction_id == local_faction &&
        (attacker.flags & 0x440u) != 0x440u;
    const int attacker_seen_by_mask = (attacker.visibility & local_bit) != 0;
    const int defender_seen_by_owner =
        defender.faction_id == local_faction &&
        (defender.flags & 0x440u) != 0x440u;
    const int defender_seen_by_mask = (defender.visibility & local_bit) != 0;
    visibility_gate_minus_78 =
        attacker_seen_by_owner || attacker_seen_by_mask ||
        defender_seen_by_owner || defender_seen_by_mask;

    // Native 0x50846E..0x5084A8 selects a preference according to the local
    // faction's treaty bit toward the attacker, then ORs the live visibility
    // gate into +0x1c only when that preference is enabled.
    const uint32_t treaty = PlayersData[local_faction].diplo_treaties[
        attacker_faction_snapshot];
    const uint32_t preference_mask = (treaty & 1u) != 0 ? 0x8000u : 4u;
    if ((GameDataStateStorage.fields_.game_preferences_ & preference_mask) != 0)
        report_selector_plus_1c |= visibility_gate_minus_78;

    // Native 0x5084A8..0x5084AE uses an equality test, not a bit test:
    // exactly mode 1 enters the already-owned artillery transaction; every
    // other current +0x18 value falls through at 0x508B50.
    return combat_type_plus_18 == 1
        ? BATTLE_FIGHT_POST_PROMPT_SOURCE_MODE_V1
        : BATTLE_FIGHT_POST_PROMPT_NORMAL_V1;
}
// Retained fragment: battle-rounds-subagent/battle-fight-round-loop-v4.cpp
// Private v3 in-context reconstruction of native battle_fight
// 0x005090D7..0x00509827. This is a C++98 owner fragment, not a complete
// function and not a paste-ready body: the enclosing body must bind the
// measured locals and the small unresolved/global adapters below.
#include <stdlib.h>

enum BattleFightRoundExitV4 {
    BFR_CONTINUE_ROUND_V4,
    BFR_TO_509983_AFTERMATH_V4,
    BFR_TO_509923_SELECTED_RETREAT_V4,
    BFR_TO_509980_TRANSITION_V4,
    BFR_TO_509827_OWNERSHIP_ENTRY_V4
};

struct BattleFightRoundFrameV4 {
    int attacker_id;                 // Numeric +0x08 vehicle-ID argument; native [-0x04] is a row offset.
    int defender_id;                 // Numeric original selected defender ID from [-0x40]; native [-0x08] itself is a row offset.
    int retreating_faction;          // [ebp-0xc], live defender faction
    int attacker_combat_value;       // [ebp-0x14]
    int &selected_defender_id;       // Live alias of numeric current stack-head ID [-0x18].
    int defender_combat_value;       // [ebp-0x1c]
    int round_or_candidate_index;    // [ebp-0x24]
    int coordinate_y_minus_28;       // [ebp-0x28]
    int coordinate_x_minus_2c;       // [ebp-0x2c]
    int attacker_x;                  // [ebp-0x30]
    int attacker_y;                  // [ebp-0x34]
    int post_hit_handoff_gate;       // [ebp-0x48]
    int &execute_display;            // Alias of live +0x1c report/display selector at this phase.
    int &mutable_second_argument;    // Alias of the live +0x0c function argument.
    int attacker_faction_local_50;   // [ebp-0x50], overwritten at 0x50991A
    int x;                            // [ebp+0x10]
    int y;                            // [ebp+0x14]
    int &source_mode;                // Alias of live +0x18 source/combat-mode argument.
    int defender_hit_step;           // low byte of [ebp-0x68]
    int attacker_hit_step;           // low byte of [ebp-0x70]
    int display_scale;               // [ebp-0x6c], same slot in both branches
    int best_direction;              // [ebp-0x74]
    int marine_detach_budget;        // [ebp-0x80], intentionally neutral name
    int best_direction_score;        // [ebp-0x8c]
    int transition_type;             // [ebp-0x54], set to 2 or 1 before UI call
    int candidate_x_plus_20;         // Live native candidate X scratch [ebp+0x20].
    int candidate_y_minus_34;        // Live native candidate Y scratch [-0x34].

    BattleFightRoundFrameV4(int &second_arg, int &mode_arg, int &display_arg,
                             int &stack_head_arg)
        : selected_defender_id(stack_head_arg), execute_display(display_arg),
          mutable_second_argument(second_arg), source_mode(mode_arg) {}
};

// These adapters are only for still-unbound state/call boundaries. The
// implementation below owns the order and branching. A coordinator binding
// may supply current-faction lookup (native global 0x9A6554), source-mode
// booleans, name storage, and the native status-window member call. Other
// operations below use existing typed project APIs directly at integration.
struct BattleFightRoundBindingsV4 {
    // Native 0x9A6554 global, still lacking a source owner.
    int (*current_faction)();
    void (*status_draw)(int attacker_id, int selected_defender_id,
                        int attack_value, int defense_value, int source_mode);
    void (*boom)(int x, int y, int effect);
    void (*redraw_tile)(int x, int y, int mode);
    void (*redraw_tile_fixup)(int x, int y, int flags, int mode);
};

static int battle_fight_round_remaining_v4(int vehicle_id) {
    return proto_power(vehicle_id) - Vehs[vehicle_id].dmg_incurred;
}

static int battle_fight_round_capacity_valid_v4(int remaining) {
    if (remaining < 0) return 0;
    if (remaining <= 9999 && remaining == 0) return 0;
    return 1;
}

static int battle_fight_round_band_changed_v4(int new_damage,
                                               int damage_step,
                                               int scale) {
    const int divisor = damage_step * scale;
    // Preserve x86 signed IDIV truncation toward zero; expected nonzero
    // divisor is guaranteed by the caller's native scale/hit gates.
    return new_damage / divisor != (new_damage - damage_step) / divisor;
}

static int battle_fight_round_candidate_v4(
    BattleFightRoundFrameV4 &f, BattleFightRoundBindingsV4 &b,
    int direction, int &candidate_x, int &candidate_y, int &score) {
    candidate_x = f.x + VehMoveDeltaX[direction];
    if (!(MapIsFlat & 1)) {
        if (candidate_x < 0) candidate_x += MapLongitudeBounds;
        else if (candidate_x >= MapLongitudeBounds)
            candidate_x -= MapLongitudeBounds;
    }
    candidate_y = f.y + VehMoveDeltaY[direction];
    if (candidate_y < 0 || candidate_y >= MapLatitudeBounds) return 0;
    if (candidate_x < 0 || candidate_x >= MapLongitudeBounds) return 0;
    if (zoc_move(candidate_x, candidate_y, f.retreating_faction) != 0) return 0;

    const int triad = Chassis[VehPrototypes[
        Vehs[f.defender_id].proto_id].chassis_id].triad;
    if (triad != TRIAD_AIR) {
        const int candidate_is_ocean =
            (map_loc(candidate_x, candidate_y)->climate & 0xe0) < 0x60;
        if (candidate_is_ocean != (triad == TRIAD_SEA)) return 0;
    }
    if (goody_at(candidate_x, candidate_y) != 0) return 0;

    Map *tile = map_loc(candidate_x, candidate_y);
    if ((tile->bit & BIT_FUNGUS) &&
        (tile->climate & 0xe0) >= 0x40) {
        const int current_faction = b.current_faction();
        if (current_faction < 0 ||
            Bases[current_faction].faction_id_current != f.retreating_faction) {
            const int defender_proto = Vehs[f.defender_id].proto_id;
            if (defender_proto >= 0x40 ||
                (get_proto_offense_rating(defender_proto) >= 0 &&
                 defender_proto != 0x0f)) return 0;
        }
    }

    if (tile->bit & (BIT_BASE_IN_TILE | BIT_VEH_IN_TILE)) {
        const int owner = tile->val2 & 0x0f;
        if (owner < 0 || owner >= 8 || owner == f.retreating_faction) return 0;
        if (!(PlayersData[owner].diplo_treaties[f.retreating_faction] & DTREATY_PACT))
            return 0;
    }

    int d = direction - (f.mutable_second_argument ^ 4);
    if (d < 0) d = -d;
    d &= 7;
    if (d > 4) d = 8 - d;
    if (d <= 2) return 0;
    score = d;
    return 1;
}

// Native 0x50973A..0x509824. Keep the unguarded Bases[base_id] read: the
// pinned code calls base_find(x,y) and immediately reads both fields even if
// that returns a negative value. This is an observed precondition, not a
// structural recommendation.
static void battle_fight_detachment_stack_v4(BattleFightRoundFrameV4 &f) {
    f.mutable_second_argument = Vehs[f.defender_id].faction_id;
    f.attacker_faction_local_50 = Vehs[f.attacker_id].faction_id;
    int first = f.selected_defender_id;
    if (first < 0) return; // exact jump to 0x509827
    int previous = Vehs[first].prev_veh_id_stack;
    while (previous >= 0) {
        first = previous;
        previous = Vehs[first].prev_veh_id_stack;
    }
    if (first < 0) return; // exact jump to 0x509827

    int unit_id = first;
    while (unit_id >= 0) {
        Vehs[unit_id].faction_id = Vehs[f.attacker_id].faction_id;
        const int proto_id = Vehs[unit_id].proto_id;
        parse_says(0, VehPrototypes[proto_id].veh_name, -1, -1);
        const int base_id = base_find(f.x, f.y);
        if (Bases[base_id].faction_id_current ==
                Vehs[f.attacker_id].faction_id &&
            Bases[base_id].population_size >= 3) {
            Vehs[unit_id].home_base_id = static_cast<int16_t>(base_id);
        } else {
            Vehs[unit_id].home_base_id = -1;
        }
        veh_skip(unit_id);
        // Native reloads the stack link after veh_skip rather than keeping a
        // pre-callback snapshot.
        unit_id = Vehs[unit_id].next_veh_id_stack;
    }
}

// Executes the measured span and returns the exact native destination label.
// The enclosing function must refresh this frame's mapped locals after any
// callback that can mutate them, as native explicitly reloads those locations.
static BattleFightRoundExitV4 battle_fight_round_loop_v4(
    BattleFightRoundFrameV4 &f, BattleFightRoundBindingsV4 &b) {
    for (;;) { // native 0x5090D7 back-edge; not a generic both-alive loop
        if (!battle_fight_round_capacity_valid_v4(
                battle_fight_round_remaining_v4(f.attacker_id)))
            return BFR_TO_509983_AFTERMATH_V4;
        if (!battle_fight_round_capacity_valid_v4(
                battle_fight_round_remaining_v4(f.defender_id)))
            return BFR_TO_509983_AFTERMATH_V4;

        // Native consumes zero draws for values <= 1, otherwise attacker first.
        const int attacker_roll = f.attacker_combat_value > 1
            ? rand() % f.attacker_combat_value : 0;
        const int defender_roll = f.defender_combat_value > 1
            ? rand() % f.defender_combat_value : 0;

        if (attacker_roll > defender_roll) {
            // Defender takes [ebp-0x68], using modulo-256 byte addition.
            const int new_damage =
                (Vehs[f.defender_id].dmg_incurred + f.defender_hit_step) & 0xff;
            Vehs[f.defender_id].dmg_incurred = new_damage;
            if (f.execute_display && f.display_scale &&
                battle_fight_round_band_changed_v4(new_damage,
                    f.defender_hit_step, f.display_scale)) {
                f.transition_type = 2;
                b.status_draw(f.attacker_id, f.selected_defender_id,
                    f.attacker_combat_value, f.defender_combat_value,
                    f.source_mode);
                b.boom(f.x, f.y, 1);
                if (battle_fight_round_capacity_valid_v4(
                        battle_fight_round_remaining_v4(f.defender_id))) {
                    b.redraw_tile(f.coordinate_x_minus_2c,
                                  f.coordinate_y_minus_28, 2);
                    b.redraw_tile(f.x, f.y, 2);
                }
                if (f.post_hit_handoff_gate) return BFR_TO_509980_TRANSITION_V4;
            }

            // [ebp-0x80] is independent carried state. This test and the
            // optional third random draw occur only after the presentation.
            if (f.marine_detach_budget &&
                battle_fight_round_capacity_valid_v4(
                    battle_fight_round_remaining_v4(f.defender_id))) {
                f.marine_detach_budget -= f.defender_hit_step;
                if (f.marine_detach_budget <= 0 && rand() % 100 < 50) {
                    battle_fight_detachment_stack_v4(f);
                    return BFR_TO_509827_OWNERSHIP_ENTRY_V4;
                }
            }
        } else {
            // Tie is defender win: attacker takes [ebp-0x70].
            const int new_damage =
                (Vehs[f.attacker_id].dmg_incurred + f.attacker_hit_step) & 0xff;
            Vehs[f.attacker_id].dmg_incurred = new_damage;
            if (f.execute_display && f.display_scale &&
                battle_fight_round_band_changed_v4(new_damage,
                    f.attacker_hit_step, f.display_scale)) {
                f.transition_type = 1;
                b.status_draw(f.attacker_id, f.selected_defender_id,
                    f.attacker_combat_value, f.defender_combat_value,
                    f.source_mode);
                b.boom(f.coordinate_x_minus_2c,
                       f.coordinate_y_minus_28, 9);
                if (battle_fight_round_capacity_valid_v4(
                        battle_fight_round_remaining_v4(f.attacker_id))) {
                    b.redraw_tile(f.coordinate_x_minus_2c,
                                  f.coordinate_y_minus_28, 2);
                    b.redraw_tile_fixup(f.x, f.y, 2, 2);
                }
                if (f.post_hit_handoff_gate) return BFR_TO_509980_TRANSITION_V4;
            }
            // Native's 0x9735 tail jumps straight to 0x5090D7. The local
            // [ebp-0x24] gate and direction scan belong only to the defender-
            // hit path above.
            continue;
        }

        if (f.round_or_candidate_index == 0) continue; // 50936C -> 5090D7
        const int attacker_remaining = battle_fight_round_remaining_v4(f.attacker_id);
        if (!battle_fight_round_capacity_valid_v4(attacker_remaining)) continue;
        f.round_or_candidate_index -= f.defender_hit_step;
        if (f.round_or_candidate_index > 0) continue;

        f.best_direction = -1;
        f.best_direction_score = 0;
        f.round_or_candidate_index = 0;
        for (int direction = 0; direction < 8; ++direction) {
            int candidate_x = 0, candidate_y = 0, candidate_score = 0;
            if (battle_fight_round_candidate_v4(f, b, direction,
                    candidate_x, candidate_y, candidate_score) &&
                candidate_score > f.best_direction_score) {
                f.best_direction_score = candidate_score;
                f.best_direction = direction;
                f.candidate_x_plus_20 = candidate_x;
                f.candidate_y_minus_34 = candidate_y;
            }
            f.round_or_candidate_index = direction + 1;
        }
        if (f.best_direction < 0) {
            f.round_or_candidate_index = 0;
            continue;
        }
        return BFR_TO_509923_SELECTED_RETREAT_V4;
    }
}
// Retained fragment: battle-rounds-subagent/battle-fight-pre-round-v9.cpp
// Private corrected typed translation v9 of battle_fight 0x508B50..0x5090D7. The caller's
// live [-0x18] slot is explicit and is not assumed equal to the defender ID.
// This remains a fragment, not a full battle_fight implementation.

// Catalog identity is `int __cdecl ?planet_busting@@YAHHHH@Z`; the retained
// seed has no compiled source owner yet. Native call sites ignore EAX, so this
// fragment also discards the result without claiming its success semantics.
extern int __cdecl planet_busting(int veh_id, int x, int y);

struct BattleFightPreRoundStateV7 {
    int round_scan_gate_minus_24;
    int detachment_roll_budget_minus_80;
    int damage_band_scale_minus_6c;
    int post_hit_handoff_gate_minus_48;
    int status_animation_flag_minus_88;
    int transition_type_minus_54;
    int early_return;
    int return_value;
};

static int battle_fight_remaining_hit_points_v7(int veh_id) {
    return range(proto_power(veh_id) - Vehs[veh_id].dmg_incurred, 0, 9999);
}

static BattleFightPreRoundStateV7 battle_fight_prepare_pre_round_v7(
    int attacker_id,
    int defender_id,
    int &native_minus_18_vehicle,
    int target_x,
    int target_y,
    int source_x,
    int source_y,
    int &source_mode,
    int &show_report,
    int attacker_combat,
    int defender_combat,
    int combat_type,
    int target_base_id,
    int &direction,
    int *status_animation_flag) {
    BattleFightPreRoundStateV7 state;
    state.round_scan_gate_minus_24 = 0;
    state.detachment_roll_budget_minus_80 = 0;
    state.damage_band_scale_minus_6c = 0;
    state.post_hit_handoff_gate_minus_48 = 0;
    state.status_animation_flag_minus_88 = 0;
    state.transition_type_minus_54 = 0;
    state.early_return = 0;
    state.return_value = 0;

    const int attacker_proto_id = Vehs[attacker_id].proto_id;
    const int defender_proto_id = Vehs[defender_id].proto_id;
    const int attacker_faction = Vehs[attacker_id].faction_id;
    const int defender_faction = Vehs[defender_id].faction_id;
    const int attacker_chassis_id =
        VehPrototypes[attacker_proto_id].chassis_id;
    const int defender_chassis_id =
        VehPrototypes[defender_proto_id].chassis_id;

    if (source_mode != 0)
        add_goal(defender_faction, 0x10, 2, target_x, target_y, -1);

    if (show_report != 0) {
        if (IsMultiplayerNet != 0 &&
            TutWinBaseWindow.zoom_wait_pending_ != 0) {
            // Native sets live [-0x88] at 0x508B88 before selection, focus,
            // drawing, and callbacks. Caller initialized it at 0x5083E0.
            state.status_animation_flag_minus_88 = 1;
            *status_animation_flag = 1;
        }
        Vehs[attacker_id].visibility |= static_cast<uint8_t>(
            1u << (static_cast<unsigned>(defender_faction) & 0x1Fu));
        VehDrawSelection689F28 = attacker_id;
        VehDrawSelection689F2C = defender_id;
        const int focus_result =
            ConsoleGlobal.focus(target_x, target_y, attacker_faction);
        if (focus_result == 0) {
            draw_tile(source_x, source_y, 2);
            draw_tile(target_x, target_y, 2);
        }

        stack_veh(attacker_id, 1);
        g_BattleWin.set_iface_mode();
        g_BattleWin.clear();
        ConsoleStatusWin.draw(attacker_id, native_minus_18_vehicle,
                              attacker_combat, defender_combat, source_mode);
        // Native private ConsoleStatusWin +0x15D4 = 1 immediately after draw.
        // Owner integration needs an approved member/friend access path.
        ConsoleStatusWin.set_field_15D4(1);
        flush_input();

        // Native tests the high byte returned by this exact query (0x508C43..4D).
        const short shift_state = GetAsyncKeyState(0x10);
        if ((shift_state & 0x8000) == 0 && (source_mode & 1) == 0)
            veh_scoot(attacker_id, source_x, source_y, direction, 1);
        stack_put(attacker_id, source_x, source_y);
        draw_tile(source_x, source_y, 2);
        draw_tile(target_x, target_y, 2);
    }

    const int attacker_weapon_id =
        VehPrototypes[attacker_proto_id].weapon_id;
    const int attacker_tri = Chassis[attacker_chassis_id].triad;
    const int defender_tri = Chassis[defender_chassis_id].triad;

    // Native [-0x24] begins as zero, then becomes ceil(remaining HP/2) only
    // under its complete directional-scan gate. Calls use native [-0x18].
    const int slot_vehicle_speed = speed(native_minus_18_vehicle, 1);
    const int attacker_speed = speed(attacker_id, 1);
    if (attacker_speed < slot_vehicle_speed && source_mode == 0 &&
        Weapon[attacker_weapon_id].offense_rating >= 0 &&
        (Vehs[defender_id].state & VSTATE_UNK_40) == 0 &&
        attacker_tri != TRIAD_AIR && defender_tri != TRIAD_AIR &&
        (!has_abil(attacker_proto_id, 0x200) ||
         has_abil(defender_proto_id, 0x2000000)) &&
        Vehs[defender_id].order != 2 && target_base_id < 0 &&
        (map_loc(target_x, target_y)->bit & 0x40800u) == 0 &&
        stack_check(native_minus_18_vehicle, 1, -1, -1, -1) == 1) {
        state.round_scan_gate_minus_24 =
            (battle_fight_remaining_hit_points_v7(defender_id) + 1) / 2;
    }

    const Map *target_tile = map_loc(target_x, target_y);
    if (has_abil(attacker_proto_id, ABL_MARINE_DETACHMENT) &&
        attacker_tri == TRIAD_SEA && (target_tile->climate & 0xE0u) < 0x60u &&
        defender_tri == TRIAD_SEA &&
        (defender_chassis_id >= 0x40 ||
         (Weapon[attacker_weapon_id].offense_rating >= 0 &&
          defender_proto_id != 0x0F)) && target_base_id < 0) {
        state.detachment_roll_budget_minus_80 =
            (battle_fight_remaining_hit_points_v7(defender_id) * 2) / 3;
    }

    if (VehPrototypes[attacker_proto_id].plan == PLAN_PLANET_BUSTER &&
        VehPrototypes[attacker_proto_id].reactor_id != 0) {
        planet_busting(attacker_id, target_x, target_y);
        WorldClimateWorldWin.field_239BC_ |= 4;
        state.early_return = 1;
        return state;
    }

    state.damage_band_scale_minus_6c =
        attacker_tri == TRIAD_LAND && !IsMultiplayerNet ? 3 : 0;
    state.transition_type_minus_54 = 0;
    if (attacker_tri != TRIAD_LAND || IsMultiplayerNet) {
        const int remaining =
            battle_fight_remaining_hit_points_v7(defender_id);
        boom(target_x, target_y, 2 - (remaining != 0));
    }
    if ((source_mode & 2) != 0 &&
        ((attacker_tri == TRIAD_SEA) != (defender_tri == TRIAD_SEA)))
        state.post_hit_handoff_gate_minus_48 = 1;

    (void)combat_type;
    return state;
}




// Retained fragment: battle-rounds-subagent/battle-fight-followthrough-v2.cpp
// Private typed fragment for battle_fight native range 0x509827..0x509923.
// Not a complete function or an integration-ready replacement. The frame
// local aliases passed here must be rebound from the complete owner body.
// Retained fragment: battle-rounds-subagent/battle-fight-transition-controls-v2.h
// Small pure branch controls shared with the private host test. These helpers
// model only the exact equality gates; callers must supply fresh values at
// each native read site, especially after popp callbacks.
static inline int battle_fight_transition_is_local_v2(int faction_id,
                                                      int current_faction) {
    return faction_id == current_faction;
}

static inline int battle_fight_transition_uses_fixup_v2(int tile_owner,
                                                        int current_faction) {
    return tile_owner == current_faction;
}

// Expected owner headers at integration: veh.h, map.h, mapwin.h, console.h,
// faction.h, game.h, popup.h, general.h, log.h.
// mapwin.h owns draw_tile(); the original 0x46AF90 declaration currently
// lives in console.cpp and should be made available through its owner header.

// Native 0x509827..0x509923 is reached with these frame facts already live:
//   - `native_second_argument` ([ebp+0x0C]) was overwritten at 0x509753 with
//     the defender faction id; it is not the requested direction anymore.
//   - `attacker_faction_local_50` ([ebp-0x50]) is the attacker faction read at
//     0x50974D, then overwritten with 1 at 0x50991A after local notifications.
//     Both it and the second argument are deliberately mutable frame values.
//   - the attacker vehicle pointer is re-read at the ownership/redraw sites;
//     don't replace those loads with either cached faction snapshot.
//
// This transition only translates the typed tile ownership, redraw and local
// notification sequence. It does not own the preceding capture decision or
// the destination at 0x509980.
static void battle_fight_ownership_transition_v2(
    int x, int y, int &native_second_argument,
    int &attacker_faction_local_50, int attacker_veh_id) {
    bit_set(x, y, BIT_SUPPLY_REMOVE, TRUE);
    owner_set(x, y, Vehs[attacker_veh_id].faction_id);

    // The native reloads Vehs[attacker_veh_id].faction_id after owner_set.
    if (battle_fight_transition_uses_fixup_v2(
            Vehs[attacker_veh_id].faction_id, ConsoleGlobal.faction_id_)) {
        draw_tile_fixup(x, y, 1, 2);
    } else {
        draw_tile(x, y, 2);
    }

    // The aliased original second-argument slot is the defender faction here.
    // It remains mutable state owned by battle_fight; this helper never restores
    // the earlier direction value.
    if (battle_fight_transition_is_local_v2(
            attacker_faction_local_50, ConsoleGlobal.faction_id_)) {
        parse_says(1, Players[native_second_argument].adj_name_faction, -1, -1);
        popp(ScriptPopupFile, "WEMARINEDETACH", 0, "navun_sm.pcx", 0);
    }
    if (battle_fight_transition_is_local_v2(
            native_second_argument, ConsoleGlobal.faction_id_)) {
        parse_says(1, Players[attacker_faction_local_50].adj_name_faction, -1, -1);
        popp(ScriptPopupFile, "THEYMARINEDETACH", 0, "navun_sm.pcx", 0);
    }
    log_say("Detachment", 0, 0, 0);
    attacker_faction_local_50 = 1; // exact 0x50991A local-flag overwrite
}
// Retained fragment: battle-capture-subagent/battle-fight-outcome-threshold-v2.cpp
// Source-mode translation of the native control-flow block
// 0x00509923..0x00509F10. The second integer parameter slot is intentionally
// passed by reference: the original writes the computed health limit back to
// [ebp+0x0C] and tests that mutated word after subsequent UI callbacks.
//
// `take_candidate_stack_path` represents the incoming edge to 0x509923; the
// alternate edge from 0x509921 lands directly at the common 0x509980 label,
// with its already-established locals. `display_latch`, `outcome_gate`, and
// `branch_selector` retain neutral names because the image only establishes
// their comparisons here, not a defensible domain meaning.
static int battle_fight_health_limit_509923_v2(int vehicle_id) {
    const Veh &vehicle = Vehs[vehicle_id];
    const VehPrototype &prototype = VehPrototypes[vehicle.proto_id];
    int health = prototype.plan == PLAN_ALIEN_ARTIFACT
               ? 1
               : (prototype.reactor_id < 1 ? 1
                  : (prototype.reactor_id > 100 ? 100
                     : prototype.reactor_id)) * 10;
    health -= vehicle.dmg_incurred;
    if (health < 0) health = 0;
    if (health > 9999) health = 9999;
    return health;
}

static int battle_fight_boom_flags_9_or_10_v2(int remaining_health) {
    return remaining_health != 0 ? 9 : 10;
}

static int battle_fight_boom_flags_8_or_10_v2(int remaining_health) {
    return remaining_health != 0 ? 8 : 10;
}

static int battle_fight_boom_flags_1_or_0_v2(int remaining_health) {
    return remaining_health != 0 ? 1 : 0;
}

static int battle_fight_boom_flags_1_or_2_v2(int remaining_health) {
    return remaining_health != 0 ? 1 : 2;
}

static int battle_fight_boom_flags_0_or_2_v2(int remaining_health) {
    return remaining_health != 0 ? 0 : 2;
}

static int battle_fight_outcome_threshold_block_v2(
    int take_candidate_stack_path, int direction_index,
    int anchor_x, int anchor_y, int target_x, int target_y,
    int candidate_x, int candidate_y,
    int stack_vehicle_id, int attacker_vehicle_id, int defender_vehicle_id,
    int &second_argument_slot, int allow_interceptor, int gate_78,
    int &display_latch, int &outcome_gate, int &branch_selector,
    int status_local_18, int status_local_14, int status_local_1c,
    int source_mode) {

    if (take_candidate_stack_path) {
        candidate_x = target_x + VehMoveDeltaX[direction_index];
        if ((MapIsFlat & 1) == 0) {
            if (candidate_x < 0) {
                candidate_x += MapLongitudeBounds;
            } else if (candidate_x >= MapLongitudeBounds) {
                candidate_x -= MapLongitudeBounds;
            }
        }
        candidate_y = target_y + VehMoveDeltaY[direction_index];
        const int moved_stack_vehicle = stack_veh(stack_vehicle_id, 1);
        stack_put(moved_stack_vehicle, candidate_x, candidate_y);
        display_latch = 1;
    }

    // 0x509980: the original reconstructs the attacker power limit and stores
    // it into its second stack-argument slot, even though that slot began as a
    // distinct source parameter.
    second_argument_slot = battle_fight_health_limit_509923_v2(
        attacker_vehicle_id);

    // `[ebp+0x1C] == 0 && [ebp-0x78] == 0` branches directly to 0x509F1D,
    // outside this block. Return 0 means the caller continues there without
    // selection-global clearing.
    if (allow_interceptor == 0 && gate_78 == 0) {
        return 0;
    }

    draw_tile_fixup(anchor_x, anchor_y, target_x, target_y, 2, 2);
    if (display_latch == 0 &&
        !((anchor_x == candidate_x && anchor_y == candidate_y) ||
          (target_x == candidate_x && target_y == candidate_y))) {
        draw_tile_fixup(candidate_x, candidate_y, 2, 2);
    }

    if (allow_interceptor == 0) {
        return 0;
    }

    ConsoleStatusWin.draw(attacker_vehicle_id, status_local_18,
                          status_local_14, status_local_1c, source_mode);
    if (display_latch != 0) {
        goto clear_draw_selection;
    }

    // All reads below occur after the status callback, matching the native
    // reloads of the mutated parameter slot and selector locals.
    if (second_argument_slot != 0) {
        if (outcome_gate == 0) {
            if (branch_selector == 2) {
                if (IsMultiplayerNet) {
                    goto defender_tile_boom_8_10;
                }
                const int defender_health =
                    battle_fight_health_limit_509923_v2(defender_vehicle_id);
                if (defender_health != 0) {
                    goto clear_draw_selection;
                }
                goto defender_tile_boom_8_10;
            }
            if (IsMultiplayerNet) {
                goto defender_tile_boom_8_10;
            }
            goto defender_limit_test;
        }

        if (outcome_gate != 1 && !IsMultiplayerNet) {
            const int attacker_health =
                battle_fight_health_limit_509923_v2(attacker_vehicle_id);
            boom(anchor_x, anchor_y,
                 battle_fight_boom_flags_9_or_10_v2(attacker_health));
        }
        goto defender_limit_test;
    }

    // The zero-valued mutated argument has a separate selector path.
    if (outcome_gate == 0) {
        goto attacker_limit_test;
    }
    if (branch_selector == 2 || IsMultiplayerNet) {
        goto defender_tile_boom_1_0;
    }
    goto defender_tile_boom_8_10;

defender_limit_test:
    {
        const int defender_health =
            battle_fight_health_limit_509923_v2(defender_vehicle_id);
        boom(target_x, target_y,
             battle_fight_boom_flags_9_or_10_v2(defender_health));
    }
    goto clear_draw_selection;

defender_tile_boom_8_10:
    {
        const int defender_health =
            battle_fight_health_limit_509923_v2(defender_vehicle_id);
        boom(target_x, target_y,
             battle_fight_boom_flags_8_or_10_v2(defender_health));
    }
    goto clear_draw_selection;

defender_tile_boom_1_0:
    {
        const int defender_health =
            battle_fight_health_limit_509923_v2(defender_vehicle_id);
        boom(target_x, target_y,
             battle_fight_boom_flags_1_or_0_v2(defender_health));
    }
    goto attacker_candidate_boom_1_2;

attacker_limit_test:
    if (branch_selector == 1 || IsMultiplayerNet) {
        goto attacker_origin_boom_0_2;
    }
    {
        const int attacker_health =
            battle_fight_health_limit_509923_v2(attacker_vehicle_id);
        boom(anchor_x, anchor_y,
             battle_fight_boom_flags_1_or_2_v2(attacker_health));
    }
    goto clear_draw_selection;

attacker_origin_boom_0_2:
    {
        const int attacker_health =
            battle_fight_health_limit_509923_v2(attacker_vehicle_id);
        if (attacker_health > 0 && !IsMultiplayerNet) {
            goto clear_draw_selection;
        }
        // The native recomputes this health expression after its multiplayer
        // test; do not reuse a value across callbacks or shared-state reads.
        const int live_attacker_health =
            battle_fight_health_limit_509923_v2(attacker_vehicle_id);
        boom(anchor_x, anchor_y,
             battle_fight_boom_flags_0_or_2_v2(live_attacker_health));
    }
    goto clear_draw_selection;

attacker_candidate_boom_1_2:
    {
        const int attacker_health =
            battle_fight_health_limit_509923_v2(attacker_vehicle_id);
        boom(anchor_x, anchor_y,
             battle_fight_boom_flags_1_or_2_v2(attacker_health));
    }
    goto clear_draw_selection;

clear_draw_selection:
    VehDrawSelection689F28 = -1;
    VehDrawSelection689F2C = -1;
    return 1;
}
// Retained fragment: battle-report-subagent/battle-fight-late-composition-v2.cpp
// Private C++98 composition packet for the native continuation
// [0x50A119,0x50B893). It is intended for insertion after the owning project
// headers; this is not a second battle_fight definition or a source mapping.
//
// This packet composes already-frozen typed blocks in native join order. The
// component bodies remain in their original private artifacts so each retains
// its own evidence and host control. No executable address calls are used.
#include <stdint.h>
#include <string.h>

// Retained fragment: battle-report-subagent/battle-fight-report-fragments-v1.cpp
// Private typed pieces for the report/diplomacy interval 0x50A119..0x50A72D.
// Inclusion-context candidates only; this does not define battle_fight.
// Include the owning headers for Map, faction, veh, base, game, general,
// netmsg, popup and temp declarations before using these helpers.

static void battle_fight_note_third_faction_v1(
    int target_x, int target_y, int attacker_faction, int defender_faction) {
    Map *target_tile = map_loc(target_x, target_y);
    const int third_faction = target_tile->territory;
    if ((target_tile->climate & 0xE0u) < 0x60u ||
        third_faction == attacker_faction ||
        third_faction == defender_faction ||
        (PlayersData[third_faction].diplo_treaties[defender_faction] & 0x10u) == 0 ||
        (PlayersData[third_faction].diplo_treaties[attacker_faction] & 0x7u) == 0) {
        return;
    }

    if (PlayersData[attacker_faction].diplo_unk1[third_faction] == 0) {
        PlayersData[attacker_faction].diplo_unk1[third_faction] = 1;
    }
    cause_friction(third_faction, attacker_faction, -1);
}

static void battle_fight_record_directional_outcome_v1(
    int attacker_faction, int defender_faction) {
    ++PlayersData[attacker_faction].diplo_unk3[defender_faction];
    ++PlayersData[attacker_faction].diplo_unk4[defender_faction];
    ++FactionTurnCombatWins[attacker_faction];
    ++FactionTurnCombatLosses[defender_faction];
}

// The native literal IDs stay caller-supplied until the project pins their
// string-table identities. The report mode boolean is also deliberately
// supplied by the enclosing exact branch; it does not assign semantics to
// the unresolved native `dat_93a95c` owner.
static void battle_fight_emit_result_notice_v1(
    int report_variant,
    const char *message_label,
    int attacker_prototype_id,
    int defender_prototype_id,
    int attacker_faction,
    int defender_faction,
    int target_x,
    int target_y) {
    parse_says(0, VehPrototypes[attacker_prototype_id].veh_name, -1, -1);
    StringTemp[0] = '\0';
    say_stats_3(StringTemp, defender_prototype_id);
    parse_says(1, StringTemp, -1, -1);
    parse_says(2, get_adjective(attacker_faction), -1, -1);
    parse_says(3, VehPrototypes[attacker_prototype_id].veh_name, -1, -1);
    StringTemp[0] = '\0';
    say_stats_3(StringTemp, attacker_prototype_id);
    parse_says(4, StringTemp, -1, -1);

    const int local_faction = ConsoleGlobal.faction_id_;
    const int base_faction = report_variant == 0 ? local_faction : defender_faction;
    const int base_id = base_find(target_x, target_y, -1, -1, -1, base_faction);
    if (base_id >= 0) {
        parse_says(5, Bases[base_id].name_string, -1, -1);
    }

    if (report_variant == 0) {
        NetMsgPrimary.pop(message_label, 5000, 0, 0);
    } else {
        POP3(message_label, 0, defender_faction, target_x, target_y);
    }
}
// Retained fragment: battle-report-subagent/battle-fight-strength-promotion-v2.cpp
// Private typed translation of the strength/morale/promotion path at
// 0x50A1B3..0x50A44D. This is a sub-branch helper, not battle_fight.
// Inclusion context needs veh.h, general.h and <stdint.h>/<stdlib.h>.

// x86 IMUL writes the low signed 32 bits. Spell the wrap explicitly so the
// source multiplication order does not rely on signed-overflow behavior.
static int32_t battle_fight_mul_i32_v2(int32_t left, int32_t right) {
    const uint32_t product = static_cast<uint32_t>(left) *
                             static_cast<uint32_t>(right);
    if (product <= 0x7FFFFFFFu) {
        return static_cast<int32_t>(product);
    }
    return -1 - static_cast<int32_t>(~product);
}

static int battle_fight_reactor_hp_v2(int vehicle_id) {
    const int proto_id = Vehs[vehicle_id].proto_id;
    const VehPrototype &prototype = VehPrototypes[proto_id];
    int maximum_hp = prototype.plan == PLAN_ALIEN_ARTIFACT
        ? 1
        : range(static_cast<int>(prototype.reactor_id), 1, 100) * 10;
    int remaining_hp = maximum_hp -
                       static_cast<int>(Vehs[vehicle_id].dmg_incurred);
    if (remaining_hp < 0) {
        remaining_hp = 0;
    } else if (remaining_hp > 9999) {
        remaining_hp = 9999;
    }
    return remaining_hp;
}

// `direction_stack_slot` is a reference on purpose: before this address range
// the native writes to [ebp+0x0c]. The branch must use the live slot value,
// not an immutable copy of the original ABI argument.
//
// `cached_attacker_hp` and `cached_defender_hp` are the already-established
// stack locals at [ebp-0x60] and [ebp-0x5c], respectively. `strength_14` and
// `strength_1c` are the mutable battle_compute outputs at [ebp-0x14] and
// [ebp-0x1c]. Do not add zero-divisor guards: the native uses signed IDIV at
// these exact sites and has no guard here.
static void battle_fight_strength_promotion_path_v2(
    int &direction_stack_slot,
    int attacker_vehicle_id,
    int defender_vehicle_id,
    int attacker_faction,
    int defender_faction,
    int target_base_id,
    int cached_attacker_hp,
    int cached_defender_hp,
    int &strength_14,
    int &strength_1c) {
    const int direction_value = direction_stack_slot;

    if (direction_value != 0) {
        // 0x50A1BE..0x50A234: recompute current attacker HP after earlier
        // callbacks, then apply the three native multiply/divide stages.
        int attacker_proto_id = Vehs[attacker_vehicle_id].proto_id;
        int attacker_reactor = static_cast<int>(
            VehPrototypes[attacker_proto_id].reactor_id);
        const int current_attacker_hp =
            battle_fight_reactor_hp_v2(attacker_vehicle_id);
        int32_t scaled = battle_fight_mul_i32_v2(
            attacker_reactor, current_attacker_hp);
        scaled = battle_fight_mul_i32_v2(scaled, strength_14);
        strength_14 = static_cast<int>(scaled / cached_attacker_hp);

        // 0x50A237..0x50A296: defender artifact capacity is one; otherwise
        // use reactor range*10. The cached defender HP is [ebp-0x5c].
        const int defender_proto_id = Vehs[defender_vehicle_id].proto_id;
        const int defender_plan = VehPrototypes[defender_proto_id].plan;
        const int defender_reactor = static_cast<int>(
            VehPrototypes[defender_proto_id].reactor_id);
        const int defender_maximum_hp = defender_plan == PLAN_ALIEN_ARTIFACT
            ? 1
            : range(defender_reactor, 1, 100) * 10;
        scaled = battle_fight_mul_i32_v2(defender_reactor,
                                         cached_defender_hp);
        scaled = battle_fight_mul_i32_v2(scaled, strength_1c);
        strength_1c = static_cast<int>(scaled / defender_maximum_hp);

        // The native tests the defender's captured plan byte, then asks for
        // attacker morale. `promote` may mutate vehicle/prototype state, so
        // no Veh reference survives it; the home-base word is reloaded after.
        if (defender_plan != PLAN_ALIEN_ARTIFACT) {
            if (morale_veh(attacker_vehicle_id, 1, 0) >= 2) {
                const int total_strength = strength_14 + strength_1c;
                int roll = 0;
                if (total_strength > 1) {
                    roll = rand() % total_strength;
                }
                if (strength_1c >= roll) {
                    promote(attacker_vehicle_id);
                }
            } else {
                promote(attacker_vehicle_id);
            }
        }

        if (attacker_faction == 0) {
            Vehs[attacker_vehicle_id].home_base_id =
                Vehs[defender_vehicle_id].home_base_id;
        }
        return;
    }

    // 0x50A2F7..0x50A3D8: reverse branch. Snapshot the attacker plan at the
    // same point as the native; the later promotion gate uses that byte.
    const int attacker_proto_id = Vehs[attacker_vehicle_id].proto_id;
    const int attacker_plan = VehPrototypes[attacker_proto_id].plan;
    const int attacker_reactor = static_cast<int>(
        VehPrototypes[attacker_proto_id].reactor_id);
    const int attacker_maximum_hp = attacker_plan == PLAN_ALIEN_ARTIFACT
        ? 1
        : range(attacker_reactor, 1, 100) * 10;
    int32_t scaled = battle_fight_mul_i32_v2(attacker_reactor,
                                             cached_attacker_hp);
    scaled = battle_fight_mul_i32_v2(scaled, strength_14);
    strength_14 = static_cast<int>(scaled / attacker_maximum_hp);

    const int defender_proto_id = Vehs[defender_vehicle_id].proto_id;
    const int defender_reactor = static_cast<int>(
        VehPrototypes[defender_proto_id].reactor_id);
    const int current_defender_hp =
        battle_fight_reactor_hp_v2(defender_vehicle_id);
    scaled = battle_fight_mul_i32_v2(defender_reactor,
                                     current_defender_hp);
    scaled = battle_fight_mul_i32_v2(scaled, strength_1c);
    strength_1c = static_cast<int>(scaled / cached_defender_hp);

    if (attacker_plan != PLAN_ALIEN_ARTIFACT) {
        if (morale_veh(defender_vehicle_id, 1, 0) >= 2) {
            const int total_strength = strength_14 + strength_1c;
            int roll = 0;
            if (total_strength > 1) {
                roll = rand() % total_strength;
            }
            if (strength_14 < roll &&
                (target_base_id < 0 ||
                 morale_veh(defender_vehicle_id, 1, 0) >= 4)) {
                // 0x50A432: no promotion on this edge.
            } else {
                promote(defender_vehicle_id);
            }
        } else {
            promote(defender_vehicle_id);
        }
    }

    if (defender_faction == 0) {
        Vehs[defender_vehicle_id].home_base_id =
            Vehs[attacker_vehicle_id].home_base_id;
    }
}
// Retained fragment: battle-report-subagent/battle-fight-report-context-v3.cpp
// Private context bridge for the native A119 -> A1B3 -> A44D path. Unlike
// report-context-v2, it derives whether A119 is entered from the pinned
// incoming predicates instead of accepting a caller-supplied boolean.
// Include report-fragments-v1.cpp and strength-promotion-v2.cpp first.
enum BattleFightA119PreStrengthRouteV3 {
    BATTLE_FIGHT_A119_RETURN_AT_A110_V3 = 0,
    BATTLE_FIGHT_A119_FALL_THROUGH_TO_A1B3_V3 = 1
};

// Native A119's SAR EBP+0x10,1. This spells arithmetic shift for negative
// values without relying on implementation-defined signed right shift.
static int battle_fight_arithmetic_half_v3(int value) {
    int half = value / 2;
    if (value < 0 && value % 2 != 0) {
        --half;
    }
    return half;
}

// Incoming predecessor decision immediately before A119. The current-HP
// helper is the already typed native formula (special plan value 0x0C => 1;
// otherwise clamp reactor 1..100, multiply by 10, subtract damage, clamp
// remaining HP to 0..9999). Call order/short-circuiting mirrors the five
// conditional branches at 9F28, 9F7D, 9F8C, 9FE2 and 9FF2.
static BattleFightA119PreStrengthRouteV3
battle_fight_route_before_a119_v3(
    int frame_minus_48,
    int attacker_vehicle_id,
    int defender_vehicle_id) {
    if (frame_minus_48 == 0) {
        return BATTLE_FIGHT_A119_FALL_THROUGH_TO_A1B3_V3;
    }
    if (battle_fight_reactor_hp_v2(attacker_vehicle_id) == 0) {
        return BATTLE_FIGHT_A119_FALL_THROUGH_TO_A1B3_V3;
    }
    if (battle_fight_reactor_hp_v2(defender_vehicle_id) == 0) {
        return BATTLE_FIGHT_A119_FALL_THROUGH_TO_A1B3_V3;
    }
    // With all three predicates false, native takes its A110 return path. If
    // the 0x93F660/local-defender conditions there match, the COMBATSURVIVE
    // report occurs before return; that report body is outside this bridge.
    return BATTLE_FIGHT_A119_RETURN_AT_A110_V3;
}

// On the A119 route, preserve the native +0x20 write and then execute A119's
// optional third-faction diplomacy work before the A1B3 strength helper.
// Return-at-A110 is reported to the caller; caller must perform the native
// optional survival-report gate/body and return, not run the A1B3 path.
static BattleFightA119PreStrengthRouteV3
battle_fight_a119_to_a44d_context_v3(
    int frame_minus_48,
    int target_x,
    int target_y,
    int attacker_faction,
    int defender_faction,
    int &direction_stack_slot,
    int attacker_vehicle_id,
    int defender_vehicle_id,
    int target_base_id,
    int cached_attacker_hp,
    int cached_defender_hp,
    int &strength_14,
    int &strength_1c,
    int &frame_plus_20) {
    const BattleFightA119PreStrengthRouteV3 route =
        battle_fight_route_before_a119_v3(
            frame_minus_48, attacker_vehicle_id, defender_vehicle_id);
    if (route == BATTLE_FIGHT_A119_RETURN_AT_A110_V3) {
        return route;
    }

    frame_plus_20 = battle_fight_arithmetic_half_v3(target_x);
    battle_fight_note_third_faction_v1(
        target_x, target_y, attacker_faction, defender_faction);
    battle_fight_strength_promotion_path_v2(
        direction_stack_slot, attacker_vehicle_id, defender_vehicle_id,
        attacker_faction, defender_faction, target_base_id,
        cached_attacker_hp, cached_defender_hp, strength_14, strength_1c);
    return BATTLE_FIGHT_A119_FALL_THROUGH_TO_A1B3_V3;
}
// Retained fragment: battle-capture-subagent/battle-fight-report-tail-v3.cpp
// Source-mode translation of the bounded native tail 0x00509F10..0x0050A119.
// All locals are supplied explicitly. `attacker_health_limit_slot` is the
// live [ebp+0x0C] slot written by the preceding native block; this bounded
// continuation does not read or write that slot. The separate health values
// below are freshly reconstructed from the vehicles, matching the native
// reloads in this tail.
static int battle_fight_health_limit_report_tail_v3(int vehicle_id) {
    const Veh &vehicle = Vehs[vehicle_id];
    const VehPrototype &prototype = VehPrototypes[vehicle.proto_id];
    int health = prototype.plan == PLAN_ALIEN_ARTIFACT
               ? 1
               : (prototype.reactor_id < 1 ? 1
                  : (prototype.reactor_id > 100 ? 100
                     : prototype.reactor_id)) * 10;
    health -= vehicle.dmg_incurred;
    return health;
}

static int battle_fight_report_tail_v3(
    int outcome_gate_local_48, int attacker_faction_local_10,
    int attacker_vehicle_id, int defender_vehicle_id,
    int report_faction_local_0c, int stats_proto_local_18,
    int x_argument_10, int y_argument_14, const char *report_label,
    int &attacker_health_limit_slot) {

    // The slot is intentionally retained in the signature to make its
    // continuity from the preceding block explicit. No read/write occurs in
    // 0x509F10..0x50A119.
    (void)attacker_health_limit_slot;

    VehDrawSelection689F28 = -1;
    VehDrawSelection689F2C = -1;
    if (outcome_gate_local_48 == 0) {
        return 0;
    }

    const int attacker_health =
        battle_fight_health_limit_report_tail_v3(attacker_vehicle_id);
    if (attacker_health < 0 ||
        (attacker_health <= 9999 && attacker_health == 0)) {
        return 0;
    }
    const int defender_health =
        battle_fight_health_limit_report_tail_v3(defender_vehicle_id);
    if (defender_health < 0 ||
        (defender_health <= 9999 && defender_health == 0)) {
        return 0;
    }

    if (!IsMultiplayerNet || report_faction_local_0c != ConsoleGlobal.faction_id_) {
        return 0;
    }

    const int defender_proto_id = Vehs[defender_vehicle_id].proto_id;
    parse_says(0, VehPrototypes[defender_proto_id].veh_name, -1, -1);

    StringTemp[0] = '\0';
    say_stats_3(StringTemp, stats_proto_local_18);
    parse_says(1, StringTemp, -1, -1);

    parse_says(2, Players[attacker_faction_local_10].adj_name_faction,
               -1, -1);

    const int attacker_proto_id = Vehs[attacker_vehicle_id].proto_id;
    parse_says(3, VehPrototypes[attacker_proto_id].veh_name, -1, -1);

    // The native reloads the attacker's prototype id after parse_says(3).
    const int stats_attacker_proto_id = Vehs[attacker_vehicle_id].proto_id;
    StringTemp[0] = '\0';
    say_stats_3(StringTemp, stats_attacker_proto_id);
    parse_says(4, StringTemp, -1, -1);

    const int local_faction = ConsoleGlobal.faction_id_;
    const int base_id = base_find(x_argument_10, y_argument_14,
                                  -1, -1, -1, local_faction);
    if (base_id >= 0) {
        parse_says(5, Bases[base_id].name_string, -1, -1);
    }

    NetMsgPrimary.pop(report_label, 5000, 0, 0);
    return 0;
}
// Retained fragment: battle-killoutcome-subagent/battle-fight-killoutcome-typed-v3.cpp
// Exact typed route selection for battle_fight 0x50A730..0x50A80D.
// Return labels retain their native destinations; they intentionally do not
// claim behavioral names. Requires map.h and veh.h.
enum BattleFightNativeRouteV3 {
    BATTLE_FIGHT_NATIVE_ROUTE_A80E_V3 = 0,
    BATTLE_FIGHT_NATIVE_ROUTE_A904_V3 = 1
};

static BattleFightNativeRouteV3 battle_fight_route_capture_gate_v3(
    int frame_minus_20,
    int frame_minus_18,
    int defender_vehicle_id,
    int attacker_vehicle_id,
    int target_x,
    int target_y) {
    if (frame_minus_20 >= 0) {
        return BATTLE_FIGHT_NATIVE_ROUTE_A80E_V3;
    }

    const Map *tile_before_stack_check = map_loc(target_x, target_y);
    if ((tile_before_stack_check->bit & 0x800u) != 0) {
        return BATTLE_FIGHT_NATIVE_ROUTE_A80E_V3;
    }

    const int defender_plan =
        VehPrototypes[Vehs[defender_vehicle_id].proto_id].plan;
    if (defender_plan == 2) {
        return BATTLE_FIGHT_NATIVE_ROUTE_A80E_V3;
    }

    const int attacker_plan =
        VehPrototypes[Vehs[attacker_vehicle_id].proto_id].plan;
    if (attacker_plan == 2) {
        return BATTLE_FIGHT_NATIVE_ROUTE_A80E_V3;
    }
    if (attacker_plan == 11) {
        return BATTLE_FIGHT_NATIVE_ROUTE_A80E_V3;
    }
    if (defender_plan == 6) {
        return BATTLE_FIGHT_NATIVE_ROUTE_A80E_V3;
    }

    if (stack_check(frame_minus_18, 2, 8, -1, -1) == 0) {
        return BATTLE_FIGHT_NATIVE_ROUTE_A904_V3;
    }

    // Native reloads the target tile after stack_check; do not reuse the
    // pre-callback climate byte.
    const Map *tile_after_stack_check = map_loc(target_x, target_y);
    if ((tile_after_stack_check->climate & 0xE0u) < 0x60u) {
        return BATTLE_FIGHT_NATIVE_ROUTE_A904_V3;
    }
    return BATTLE_FIGHT_NATIVE_ROUTE_A80E_V3;
}
// Retained fragment: battle-killoutcome-subagent/battle-fight-killoutcome-typed-v4.cpp
// Exact branch/call translation for battle_fight 0x50A80E..0x50A903.
// Requires veh.h. Native frame/register inputs keep neutral identities; the
// selector calls battle_kill and distinguishes the native A904 / AC8B joins.
enum BattleFightNativeRouteA80E_V4 {
    BATTLE_FIGHT_NATIVE_ROUTE_A904_V4 = 0,
    BATTLE_FIGHT_NATIVE_ROUTE_AC8B_AFTER_KILL_V4 = 1
};

static void battle_fight_call_kill_then_reload_v4(
    int frame_minus_18_vehicle_id,
    int *frame_minus_38,
    int *frame_minus_30,
    int *frame_minus_58,
    int original_vehicle_id,
    int frame_minus_10,
    const int *frame_minus_40,
    int *native_edi_after_kill) {
    battle_kill(frame_minus_18_vehicle_id,
                frame_minus_38,
                frame_minus_30,
                frame_minus_58,
                original_vehicle_id,
                frame_minus_10);
    // Native reload at A8F9 occurs only after battle_kill returns.
    *native_edi_after_kill = *frame_minus_40;
}

static BattleFightNativeRouteA80E_V4 battle_fight_route_or_kill_v4(
    int native_esi_before_test,
    int frame_minus_20,
    int frame_minus_18_vehicle_id,
    int defender_vehicle_id,
    int attacker_vehicle_id,
    int *frame_minus_38,
    int *frame_minus_30,
    int *frame_minus_58,
    int original_vehicle_id,
    int frame_minus_10,
    const int *frame_minus_40,
    int *native_edi_after_kill) {
    if (VehPrototypes[Vehs[defender_vehicle_id].proto_id].plan == 12) {
        return BATTLE_FIGHT_NATIVE_ROUTE_A904_V4;
    }

    if (native_esi_before_test == 0) {
        battle_fight_call_kill_then_reload_v4(
            frame_minus_18_vehicle_id, frame_minus_38, frame_minus_30,
            frame_minus_58, original_vehicle_id, frame_minus_10,
            frame_minus_40, native_edi_after_kill);
        return BATTLE_FIGHT_NATIVE_ROUTE_AC8B_AFTER_KILL_V4;
    }

    if (veh_cargo(frame_minus_18_vehicle_id) != 0 && frame_minus_20 < 0 &&
        VehPrototypes[Vehs[attacker_vehicle_id].proto_id].plan != 11) {
        return BATTLE_FIGHT_NATIVE_ROUTE_A904_V4;
    }

    if (VehPrototypes[Vehs[attacker_vehicle_id].proto_id].plan == 11) {
        battle_fight_call_kill_then_reload_v4(
            frame_minus_18_vehicle_id, frame_minus_38, frame_minus_30,
            frame_minus_58, original_vehicle_id, frame_minus_10,
            frame_minus_40, native_edi_after_kill);
        return BATTLE_FIGHT_NATIVE_ROUTE_AC8B_AFTER_KILL_V4;
    }

    int stack_difference =
        stack_check(frame_minus_18_vehicle_id, 2, 12, -1, -1);
    stack_difference =
        stack_check(frame_minus_18_vehicle_id, 1, -1, -1, -1) - stack_difference;
    stack_difference -= stack_check(frame_minus_18_vehicle_id, 2, 8, -1, -1);
    stack_difference -= stack_check(frame_minus_18_vehicle_id, 2, 11, -1, -1);
    stack_difference -= stack_check(frame_minus_18_vehicle_id, 2, 10, -1, -1);

    if (stack_difference <= 1) {
        return BATTLE_FIGHT_NATIVE_ROUTE_A904_V4;
    }

    battle_fight_call_kill_then_reload_v4(
        frame_minus_18_vehicle_id, frame_minus_38, frame_minus_30,
        frame_minus_58, original_vehicle_id, frame_minus_10,
        frame_minus_40, native_edi_after_kill);
    return BATTLE_FIGHT_NATIVE_ROUTE_AC8B_AFTER_KILL_V4;
}
// Retained fragment: battle-killoutcome-subagent/battle-fight-rearm-loop-v1.cpp
// Typed source fragment for the native battle_fight stack-removal/rearm path
// beginning at label 0x50A904 and joining at 0x50AAE0. This is not a complete
// battle_fight body. It is entered only on native edges to 0x50A904; callers
// must preserve the distinct kill path ending at 0x50A8FF.
//
// `frame_vehicle_id` is the source-level vehicle ID corresponding to the
// record base held in EDI at 0x50A91C; the neutral frame mapping stays an
// explicit caller input. `vehicle_to_test` is native [ebp-0x18] (the selected
// defender). The
// `source_mode_slot` is the mutable fifth integer argument, incremented once
// for every unit that reaches 0x50AAB1.
//
// Native instruction source: pinned image SHA-256
// 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf,
// [0x50A904,0x50AAE0). Kuna export was consulted only as CFG commentary.
static void battle_fight_rearm_loop_v1(
    int frame_vehicle_id,
    int vehicle_to_test,
    int attacker_vehicle_id,
    int target_x,
    int target_y,
    int &source_mode_slot) {
    // 0x50A904..0x50A90F: detach/normalize through the existing stack owner.
    stack_veh(vehicle_to_test, 0);
    if (vehicle_to_test < 0) {
        return;
    }

    // 0x50A91C..0x50A940: EDI supplies the first previous-link load, while
    // ESI ([ebp-0x18]) is the initial vehicle id if that link is already -1.
    int vehicle_id = vehicle_to_test;
    int previous = Vehs[frame_vehicle_id].prev_veh_id_stack;
    while (previous >= 0) {
        vehicle_id = previous;
        previous = Vehs[vehicle_id].prev_veh_id_stack;
    }

    // The native follows the mutable next link after completing each vehicle
    // update. No Veh reference is retained across a game callback.
    while (vehicle_id >= 0) {
        const int current_proto_id = Vehs[vehicle_id].proto_id;
        const int current_chassis_id =
            VehPrototypes[current_proto_id].chassis_id;
        const int current_triad = Chassis[current_chassis_id].triad;
        const Map *tile = map_loc(target_x, target_y);

        // 0x50A96C..0x50A9DE. Preserve the ordered alternative branches:
        // non-land chassis, high-altitude terrain, a supply/open owner tile,
        // or the native order/waypoint relation all skip the HP reset.
        const bool exposed_or_special_tile =
            current_triad != TRIAD_LAND ||
            (tile->climate & 0xE0u) >= 0x60u ||
            ((tile->bit & 1u) != 0 && (tile->val2 & 0x0Fu) <= 7u) ||
            (Vehs[vehicle_id].order == 1 &&
             Vehs[vehicle_id].waypoint_x[0] != vehicle_to_test &&
             Vehs[vehicle_id].waypoint_x[0] >= 0);

        if (!exposed_or_special_tile) {
            // 0x50A9E0..0x50AA45. This path computes max HP minus existing
            // damage, then adds that remainder back to the byte. The apparent
            // over-9999 branch is retained even though the typed HP domain is
            // bounded by the native reactor table.
            const int current_reactor = static_cast<int>(
                VehPrototypes[current_proto_id].reactor_id);
            const int maximum_damage =
                VehPrototypes[current_proto_id].plan == PLAN_ALIEN_ARTIFACT
                    ? 1
                    : range(current_reactor, 1, 100) * 10;
            const unsigned char old_damage = Vehs[vehicle_id].dmg_incurred;
            int remaining = maximum_damage - static_cast<int>(old_damage);
            if (remaining < 0) {
                remaining = 0;
            } else if (remaining > 9999) {
                // Native byte addition: old damage + 15, truncated to 8 bits.
                Vehs[vehicle_id].dmg_incurred = static_cast<uint8_t>(
                    static_cast<unsigned>(old_damage) + 15u);
                goto native_post_damage;
            }
            Vehs[vehicle_id].dmg_incurred = static_cast<uint8_t>(
                static_cast<unsigned>(old_damage) +
                static_cast<unsigned>(remaining));
        } else if (vehicle_id != vehicle_to_test) {
            // 0x50AA4C..0x50AACD: only non-selected units with one of these
            // four plans bypass the shared 3x-reactor/state/source-mode tail.
            const int current_plan = VehPrototypes[current_proto_id].plan;
            if (current_plan == PLAN_ALIEN_ARTIFACT ||
                current_plan == PLAN_COLONIZATION ||
                current_plan == PLAN_INFO_WARFARE ||
                current_plan == PLAN_SUPPLY_CONVOY) {
                vehicle_id = Vehs[vehicle_id].next_veh_id_stack;
                continue;
            }
        }

native_post_damage:
        {
            // 0x50AA62..0x50AACD. RulesWeapon::mode is read as a signed byte
            // by the image despite its current storage type being uint8_t.
            // A nonnegative mode selects the attacker's reactor; otherwise
            // the current stack unit's reactor supplies the multiplier.
            const int attacker_proto_id = Vehs[attacker_vehicle_id].proto_id;
            const int attacker_weapon_id =
                VehPrototypes[attacker_proto_id].weapon_id;
            const int multiplier_reactor =
                static_cast<int8_t>(Weapon[attacker_weapon_id].mode) >= 0
                    ? static_cast<int>(VehPrototypes[attacker_proto_id].reactor_id)
                    : static_cast<int>(VehPrototypes[current_proto_id].reactor_id);
            const int byte_delta = static_cast<int>(
                static_cast<int8_t>(static_cast<uint8_t>(multiplier_reactor * 3)));
            Vehs[vehicle_id].dmg_incurred = static_cast<uint8_t>(
                static_cast<unsigned>(Vehs[vehicle_id].dmg_incurred) +
                static_cast<unsigned>(static_cast<uint8_t>(byte_delta)));
            ++source_mode_slot;
            Vehs[vehicle_id].state |= VSTATE_UNK_4;
        }

        vehicle_id = Vehs[vehicle_id].next_veh_id_stack;
    }

}
// Retained fragment: battle-killoutcome-subagent/battle-fight-killoutcome-typed-v22.cpp
// Private corrected translation of native battle_fight [0x50AAE0,0x50AC8B).
// v6 is preserved unchanged; v22 fixes a proven linked-stack traversal
// liveness error from the pinned 0x50AAF1..AB18 sequence.
enum BattleFightRemovalRouteV22 {
    BATTLE_FIGHT_REMOVAL_ROUTE_AC8B_V22 = 0
};

static BattleFightRemovalRouteV22 battle_fight_remove_damaged_stack_v22(
    int frame_minus_18_vehicle_id,
    int frame_minus_8_vehicle_id,
    int frame_minus_10_value,
    int original_vehicle_id,
    int *frame_plus_20_mutable,
    int *frame_minus_38,
    int *frame_minus_30,
    int *frame_minus_58,
    int frame_plus_10_x,
    int frame_plus_14_y,
    int *frame_plus_18_mutable) {
    *frame_plus_20_mutable = original_vehicle_id;
    if (frame_minus_18_vehicle_id >= 0) {
        // Native EDI is seeded from [-0x18]. The predecessor at [-0x08]
        // replaces it only while nonnegative; when the next link is -1, EDI
        // retains the last valid member rather than the sentinel.
        int vehicle_id = frame_minus_18_vehicle_id;
        int predecessor = Vehs[frame_minus_8_vehicle_id].prev_veh_id_stack;
        while (predecessor >= 0) {
            vehicle_id = predecessor;
            predecessor = Vehs[vehicle_id].prev_veh_id_stack;
        }

        while (vehicle_id >= 0) {
            int next_vehicle_id = Vehs[vehicle_id].next_veh_id_stack;
            const int prototype_id = Vehs[vehicle_id].proto_id;
            const int max_damage =
                VehPrototypes[prototype_id].plan == PLAN_ALIEN_ARTIFACT
                    ? 1
                    : range(static_cast<int>(VehPrototypes[prototype_id].reactor_id),
                            1, 100) * 10;
            const int remaining_damage = range(
                max_damage - static_cast<int>(Vehs[vehicle_id].dmg_incurred),
                0, 9999);

            if (remaining_damage <= 0 || Vehs[vehicle_id].faction_id == 0) {
                const int killed = battle_kill(
                    vehicle_id, frame_minus_38, frame_minus_30,
                    frame_minus_58, *frame_plus_20_mutable,
                    frame_minus_10_value);
                if (killed != 0) {
                    if (*frame_plus_20_mutable > vehicle_id) {
                        --*frame_plus_20_mutable;
                    }
                    if (next_vehicle_id > vehicle_id) {
                        --next_vehicle_id;
                    }
                }
            }

            vehicle_id = next_vehicle_id;
        }
    }

    const int adjusted_frame_plus_18 =
        *frame_plus_18_mutable - (*frame_minus_38 + *frame_minus_30);
    *frame_plus_18_mutable = adjusted_frame_plus_18 < 0
                                 ? 0
                                 : adjusted_frame_plus_18;
    int stack_id = stack_fix(veh_at(frame_plus_10_x, frame_plus_14_y));
    if (stack_id >= 0) {
        const int check_12 = stack_check(stack_id, 2, 12, -1, -1);
        const int check_1 = stack_check(stack_id, 1, -1, -1, -1);
        int difference = check_1 - check_12;
        difference -= stack_check(stack_id, 2, 8, -1, -1);
        difference -= stack_check(stack_id, 2, 11, -1, -1);
        difference -= stack_check(stack_id, 2, 10, -1, -1);

        if (difference == 0) {
            battle_kill_stack(stack_id, frame_minus_38, frame_minus_30,
                              frame_minus_58, frame_minus_18_vehicle_id, -1);
            *frame_plus_18_mutable = 0;
        }
    }
    return BATTLE_FIGHT_REMOVAL_ROUTE_AC8B_V22;
}
// Retained fragment: battle-fight-base-objective-v1.cpp
// Private typed translation of battle_fight 0x0050AC8B..0x0050AF13.
// Include after the enclosing owner has declared the ordinary locals.  The
// three references correspond to live stack values, not immutable arguments:
// base_id is [ebp-0x20], population_half is [ebp-0x3c], and
// frame_result is [ebp-0x44].

enum BattleFightBaseRouteV1 {
    BATTLE_FIGHT_BASE_TO_B1B4_V1,
    BATTLE_FIGHT_BASE_TO_B1BC_V1
};

static void battle_fight_sync_killed_base_v1(int x, int y, int defender_faction,
                                             int attacker_faction,
                                             int &frame_result) {
    for (int faction_id = 0; faction_id < 8; ++faction_id) {
        synch_bit(x, y, faction_id);
    }
    draw_map(1);
    if (eliminate_player(defender_faction, attacker_faction)) {
        frame_result = 0;
    }
}

static BattleFightBaseRouteV1 battle_fight_base_objective_v1(
    int &base_id, int target_x, int target_y, int target_stack,
    int attacker_tile_x, int attacker_tile_y, int attacker_faction,
    int defender_faction, int &population_half, int &frame_result) {
    if (base_id >= 0) {
        Base &base = Bases[base_id];
        base.state |= BSTATE_COMBAT_LOSS_LAST_TURN;

        if (!is_ocean(attacker_tile_x, attacker_tile_y)
            && !(is_objective(base_id) && base.population_size <= 1)
            && !has_fac(FAC_PERIMETER_DEFENSE, base_id, 0)
            && !has_fac(FAC_TACHYON_FIELD, base_id, 0)
            && !has_project(SP_CITIZENS_DEFENSE_FORCE, defender_faction)) {
            const int target_proto = Vehs[target_stack].proto_id;
            const int target_weapon = VehPrototypes[target_proto].weapon_id;
            const bool target_has_offense =
                Weapon[target_weapon].offense_rating != 0;
            if (target_has_offense && attacker_faction != 0
                && veh_who(target_x, target_y) < 0
                && !(PlayersData[defender_faction].current_num_bases == 0
                     && is_human(defender_faction))) {
                --base.population_size;
                if (base.population_size <= 0) {
                    population_half = 0;
                    base_kill(base_id);
                    base_id = -1;
                    battle_fight_sync_killed_base_v1(
                        target_x, target_y, defender_faction,
                        attacker_faction, frame_result);
                    goto battle_fight_base_factions_v1;
                }
                base.faction_pop_size_intel[attacker_faction] =
                    static_cast<uint8_t>(base.population_size);
            }
        }

        if (!is_human(defender_faction)) {
            base_reset(base_id, 0);
        }

        if (population_half != 0) {
            int half = (static_cast<int>(base.population_size) + 1) / 2;
            population_half = half;
            base.population_size = static_cast<int8_t>(
                static_cast<int>(base.population_size) - half);
            if (is_objective(base_id) && base.population_size < 1) {
                base.population_size = 1;
            }
            if (base.population_size <= 0) {
                base_kill(base_id);
                base_id = -1;
                battle_fight_sync_killed_base_v1(
                    target_x, target_y, defender_faction,
                    attacker_faction, frame_result);
                goto battle_fight_base_factions_v1;
            }
            spot_base(base_id, attacker_faction);
        }
    }

    if (veh_who(target_x, target_y) < 0
        && !is_human(defender_faction) && base_id >= 0) {
        invasions(base_id);
    }

battle_fight_base_factions_v1:
    if (attacker_faction != 0 && defender_faction != 0) {
        return BATTLE_FIGHT_BASE_TO_B1B4_V1;
    }
    return BATTLE_FIGHT_BASE_TO_B1BC_V1;
}
// Retained fragment: battle-killoutcome-subagent/battle-fight-killoutcome-typed-v2.cpp
// Private, inclusion-context typed fragments for battle_fight native
// 0x50A8DC..0x50A8FF, 0x50AD8E..0x50ADD4, and 0x50B157..0x50B1BC.
// This is not a standalone battle_fight body. It requires base.h, map.h,
// mapwin.h, and veh.h. Unresolved frame values retain neutral names.

// Native 0x50A8DC pushes the six values in reverse order for this call.
// Its return value is ignored; all three pointed-to counters are live outputs.
static void battle_fight_call_kill_frame_value_v2(
    int frame_minus_18,
    int *frame_minus_38,
    int *frame_minus_30,
    int *frame_minus_58,
    int original_vehicle_id,
    int frame_minus_10) {
    battle_kill(frame_minus_18,
                frame_minus_38,
                frame_minus_30,
                frame_minus_58,
                original_vehicle_id,
                frame_minus_10);
}

// Native 0x50B157 passes the original vehicle as the stack root, reuses the
// same output locals in the exact order, supplies killer_id=-1, and supplies
// the current native ESI value as credit_faction. The caller must pass the
// freshly live ESI value at the call site, not a faction snapshot from earlier
// callbacks.
static void battle_fight_call_stack_kill_v2(
    int original_vehicle_id,
    int *frame_minus_38,
    int *frame_minus_30,
    int *frame_minus_58,
    int frame_esi_at_call) {
    battle_kill_stack(original_vehicle_id,
                      frame_minus_38,
                      frame_minus_30,
                      frame_minus_58,
                      -1,
                      frame_esi_at_call);
}

// Exact leaf transition at native 0x50AD8E..0x50ADD4. Caller reaches this
// only after the preceding opaque base-damage eligibility sequence selected
// this population update. true means native follows the base-destroyed path;
// it does not mean the entire later continuation is complete.
static bool battle_fight_apply_base_population_loss_v2(
    int base_id,
    int attacker_faction,
    int target_x,
    int target_y,
    int *frame_minus_20,
    int *frame_minus_3c,
    int defender_faction,
    int *frame_minus_44) {
    const uint8_t population_bits = static_cast<uint8_t>(Bases[base_id].population_size);
    const int8_t population_after = static_cast<int8_t>(
        static_cast<uint8_t>(population_bits - 1u));
    Bases[base_id].population_size = population_after;

    if (population_after > 0) {
        Bases[base_id].faction_pop_size_intel[attacker_faction] =
            static_cast<uint8_t>(population_after);
        return false;
    }

    // Native sets this output before base_kill, then destroys the base. Do not
    // read Bases[base_id] after the lifecycle callback.
    *frame_minus_3c = 0;
    base_kill(base_id);
    *frame_minus_20 = -1;

    // 0x00591E50 receives the same target x/y and faction ids 0 through 7.
    // Preserve the eight calls and their order after base_kill.
    for (int faction = 0; faction < 8; ++faction) {
        synch_bit(target_x, target_y, faction);
    }
    draw_map(1);
    if (eliminate_player(defender_faction, attacker_faction)) {
        *frame_minus_44 = 0;
    }
    return true;
}
// Retained fragment: battle-killoutcome-subagent/battle-fight-killoutcome-typed-v7.cpp
// Typed redraw branch battle_fight [0x50B2FA,0x50B354).
// Requires mapwin.h and temp.h; the map-window owner header declares the
// typed overload used by this native redraw branch.
enum BattleFightRedrawRouteV7 {
    BATTLE_FIGHT_REDRAW_ROUTE_B639_V7 = 0,
    BATTLE_FIGHT_REDRAW_ROUTE_B354_V7 = 1
};

static BattleFightRedrawRouteV7 battle_fight_redraw_pair_v7(
    int frame_plus_1c,
    int frame_plus_0c,
    int frame_minus_2c_x,
    int frame_minus_28_y,
    int frame_plus_10_x,
    int frame_plus_14_y) {
    if (frame_plus_1c == 0) {
        return BATTLE_FIGHT_REDRAW_ROUTE_B639_V7;
    }

    if (frame_plus_0c != 0) {
        draw_tile(frame_minus_2c_x, frame_minus_28_y, 1);
        draw_tile_fixup(frame_plus_10_x, frame_plus_14_y, 2, 1);
    } else {
        draw_tile_fixup(frame_minus_2c_x, frame_minus_28_y, 2, 1);
        draw_tile(frame_plus_10_x, frame_plus_14_y, 1);
    }
    do_all_draws();
    return BATTLE_FIGHT_REDRAW_ROUTE_B354_V7;
}
// The v4 route adapter uses this two-value native join enum. Define only the
// enum here: do not include or call the disputed v8 AD1A..AF13 body. That
// whole native interval is composed from battle_fight_base_objective_v1.
enum BattleFightBaseContinuationRouteV8 {
    BATTLE_FIGHT_BASE_ROUTE_B1B4_V8 = 0,
    BATTLE_FIGHT_BASE_ROUTE_B1BC_V8 = 1
};
// Retained fragment: battle-report-subagent/battle-fight-post-outcome-v4.cpp
// Private insertion-context translation for battle_fight's post-outcome
// blocks [0x50AF13,0x50B2FA) and [0x50B354,0x50B63F). This is not a complete
// battle_fight definition. Include the owning project headers plus existing
// battle-fight-killoutcome-typed-v2.cpp, typed-v7.cpp and typed-v8.cpp.
// Pinned native message labels in this span are emitted as ordinary text.
// 0x00477210's existing recovered unit identifies mon_enemy_destroyed, but
// that seed unit has no public owner header yet; retain this direct symbol
// dependency until its real declaration is homed.
void __cdecl mon_enemy_destroyed(int faction_id, int faction_id_with);

// The base-destruction continuation added by killoutcome-typed-v8 selects
// these same two native edges. Keep the route explicit: B1B4 invokes the
// monument callback, whereas B1BC begins the shared treaty-friction block.
enum BattleFightReportContinuationRouteV4 {
    BATTLE_FIGHT_REPORT_ROUTE_B1B4_V4 = 0,
    BATTLE_FIGHT_REPORT_ROUTE_B1BC_V4 = 1
};

static void battle_fight_b203_score_and_neighbor_refresh_v4(
    int &direction_stack_slot,
    int &native_plus8_loop_slot,
    int attacker_faction,
    int target_x,
    int target_y,
    int frame_minus_58,
    int &frame_minus_40_last_vehicle);

static void battle_fight_b1bc_treaty_tail_v4(
    int attacker_faction,
    int defender_faction,
    int *frame_minus_30) {
    // Shared native B1BC..B203 block. The pointer is live because prior
    // report/kill callbacks can change the output local.
    if (*frame_minus_30 != 0 &&
        (static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) &
         static_cast<uint8_t>(1u <<
             (static_cast<uint32_t>(defender_faction) & 0x1Fu))) == 0 &&
        has_treaty(attacker_faction, defender_faction, 0x10) == 0) {
        cause_friction(defender_faction, attacker_faction, 5);
        treaty_on(attacker_faction, defender_faction, 0x2000);
    }
}

static void battle_fight_resume_report_route_v4(
    BattleFightReportContinuationRouteV4 route,
    int attacker_faction,
    int defender_faction,
    int *frame_minus_30) {
    if (route == BATTLE_FIGHT_REPORT_ROUTE_B1B4_V4) {
        // Native pushes EDI then ESI; cdecl therefore receives defender,
        // attacker. This is also the v8 direct incoming B1B4 edge.
        mon_enemy_destroyed(defender_faction, attacker_faction);
    }
    battle_fight_b1bc_treaty_tail_v4(
        attacker_faction, defender_faction, frame_minus_30);
}

// Incoming composition for killoutcome-typed-v8: that fragment owns the
// AE..AF route choice and stops at B1B4/B1BC. Both edges rejoin at B203 after
// the shared callback/treaty continuation here.
static void battle_fight_base_route_to_b2fa_v4(
    BattleFightBaseContinuationRouteV8 route,
    int attacker_faction,
    int defender_faction,
    int *frame_minus_30,
    int &direction_stack_slot,
    int &native_plus8_loop_slot,
    int target_x,
    int target_y,
    int frame_minus_58,
    int &frame_minus_40_last_vehicle) {
    battle_fight_resume_report_route_v4(
        route == BATTLE_FIGHT_BASE_ROUTE_B1B4_V8
            ? BATTLE_FIGHT_REPORT_ROUTE_B1B4_V4
            : BATTLE_FIGHT_REPORT_ROUTE_B1BC_V4,
        attacker_faction, defender_faction, frame_minus_30);
    battle_fight_b203_score_and_neighbor_refresh_v4(
        direction_stack_slot, native_plus8_loop_slot, attacker_faction,
        target_x, target_y, frame_minus_58, frame_minus_40_last_vehicle);
}

// Exact typed path from the alternate-direction report entry through the
// B203 join. The two report arms remain distinct; `opaque_report_gate` is the
// unresolved 0x93A95C state, not a renamed game option.
static void battle_fight_after_outcome_af13_to_b203_v4(
    int original_vehicle_id,
    int &native_plus8_loop_slot,
    Veh &attacker_vehicle_record,
    Veh &defender_vehicle_record,
    int attacker_faction,
    int defender_faction,
    int target_x,
    int target_y,
    int opaque_report_gate,
    int *frame_minus_38,
    int *frame_minus_30,
    int *frame_minus_58) {
    const int multiplayer_gate_value = IsMultiplayerNet;

    // Native's alternate-direction result accounting is defender wins / the
    // attacker's losses, distinct from the preceding nonzero-direction arm.
    ++FactionTurnCombatWins[defender_faction];
    ++FactionTurnCombatLosses[attacker_faction];

    if (multiplayer_gate_value != 0 &&
        defender_faction == ConsoleGlobal.faction_id_) {
        int proto_id = defender_vehicle_record.proto_id;
        parse_says(0, VehPrototypes[proto_id].veh_name, -1, -1);
        clear();
        proto_id = defender_vehicle_record.proto_id;
        say_stats_3(proto_id);
        parse_says(1, StringTemp, -1, -1);

        parse_says(2, get_adjective(attacker_faction), -1, -1);

        proto_id = attacker_vehicle_record.proto_id;
        parse_says(3, VehPrototypes[proto_id].veh_name, -1, -1);
        clear();
        proto_id = attacker_vehicle_record.proto_id;
        say_stats_3(proto_id);
        parse_says(4, StringTemp, -1, -1);

        const int base_id = base_find(target_x, target_y, -1, -1, -1,
                                      ConsoleGlobal.faction_id_);
        if (base_id >= 0) {
            parse_says(5, Bases[base_id].name_string, -1, -1);
        }
        NetMsgPrimary.pop("COMBATWIN", 5000, 0, 0);
    } else if (opaque_report_gate != 0 &&
               ((static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) &
                 static_cast<uint8_t>(1u <<
                     (static_cast<uint32_t>(defender_faction) & 0x1Fu))) != 0)) {
        int proto_id = defender_vehicle_record.proto_id;
        parse_says(0, VehPrototypes[proto_id].veh_name, -1, -1);
        clear();
        proto_id = defender_vehicle_record.proto_id;
        say_stats_3(proto_id);
        parse_says(1, StringTemp, -1, -1);

        parse_says(2, get_adjective(attacker_faction), -1, -1);

        proto_id = attacker_vehicle_record.proto_id;
        parse_says(3, VehPrototypes[proto_id].veh_name, -1, -1);
        clear();
        proto_id = attacker_vehicle_record.proto_id;
        say_stats_3(proto_id);
        parse_says(4, StringTemp, -1, -1);

        const int base_id = base_find(target_x, target_y, -1, -1, -1,
                                      defender_faction);
        if (base_id >= 0) {
            parse_says(5, Bases[base_id].name_string, -1, -1);
        }
        POP3("COMBATWIN", 0, defender_faction,
             target_x, target_y);
    }

    // 0x50B157..0x50B1BC: callbacks may mutate vehicle records, so carry the
    // vehicle ID and re-index rather than keeping a Veh& across calls.
    native_plus8_loop_slot = original_vehicle_id;
    stack_veh(native_plus8_loop_slot, 1);
    battle_fight_call_stack_kill_v2(
        native_plus8_loop_slot, frame_minus_38, frame_minus_30,
        frame_minus_58, defender_faction);

    // Native pair-array order at B190 is defender row, attacker column.
    ++PlayersData[defender_faction].diplo_unk3[attacker_faction];
    ++PlayersData[defender_faction].diplo_unk4[attacker_faction];
    // AF13's local entry takes the exact native B1AE tests. The distinct
    // v8 route is passed to battle_fight_resume_report_route_v4 by the outer
    // base-destruction caller and can enter directly at B1B4 or B1BC.
    const BattleFightReportContinuationRouteV4 route =
        attacker_faction != 0 && defender_faction != 0
            ? BATTLE_FIGHT_REPORT_ROUTE_B1B4_V4
            : BATTLE_FIGHT_REPORT_ROUTE_B1BC_V4;
    battle_fight_resume_report_route_v4(
        route, attacker_faction, defender_faction, frame_minus_30);
}

// Shared B203 score/UI loop. This block is reached from multiple native
// predecessors (including the nonzero-direction A730 route), so the mutable
// +0x0C/+0x08 slots and output locals are supplied live by the caller.
static void battle_fight_b203_score_and_neighbor_refresh_v4(
    int &direction_stack_slot,
    int &native_plus8_loop_slot,
    int attacker_faction,
    int target_x,
    int target_y,
    int frame_minus_58,
    int &frame_minus_40_last_vehicle) {
    if (direction_stack_slot == 0 || frame_minus_58 == 0) {
        return;
    }

    PlayersData[attacker_faction].energy_reserves += frame_minus_58;
    ConsoleGlobal.update_data(0);

    native_plus8_loop_slot = 0;
    while (native_plus8_loop_slot < 0x20) {
        const int neighbor_index = native_plus8_loop_slot / 4;
        const int neighbor_x = wrap_map_x(target_x + VehMoveDeltaX[neighbor_index]);
        const int neighbor_y = target_y + VehMoveDeltaY[neighbor_index];
        if (on_map(neighbor_x, neighbor_y)) {
            frame_minus_40_last_vehicle = veh_at(neighbor_x, neighbor_y);
            int vehicle_id = frame_minus_40_last_vehicle;
            while (vehicle_id >= 0) {
                // Visibility and the packed word are reread after each draw
                // callback; do not retain a record reference across it.
                if (Vehs[vehicle_id].visibility == 0) {
                    const uint16_t old_word = Vehs[vehicle_id].unk_1;
                    const int old_direction = (old_word >> 3) & 3;
                    Vehs[vehicle_id].unk_1 = static_cast<uint16_t>(
                        old_word & static_cast<uint16_t>(~(old_direction * 8)));
                    const int next_direction = range(old_direction + 1, 0, 3);
                    Vehs[vehicle_id].unk_1 = static_cast<uint16_t>(
                        Vehs[vehicle_id].unk_1 |
                        static_cast<uint16_t>(next_direction << 3));
                    draw_tile(neighbor_x, neighbor_y, 2);
                }
                vehicle_id = Vehs[vehicle_id].prev_veh_id_stack;
            }
        }
        native_plus8_loop_slot += 4;
    }
}

// Concrete in-context bridge for AF13 -> B203 -> B2FA. On the native AF13
// edge the parameter slot is ordinarily zero, but it remains a reference
// because earlier native callbacks may have mutated that slot.
static void battle_fight_af13_to_b2fa_v4(
    int original_vehicle_id,
    int &direction_stack_slot,
    int &native_plus8_loop_slot,
    Veh &attacker_vehicle_record,
    Veh &defender_vehicle_record,
    int attacker_faction,
    int defender_faction,
    int target_x,
    int target_y,
    int opaque_report_gate,
    int *frame_minus_38,
    int *frame_minus_30,
    int *frame_minus_58,
    int &frame_minus_40_last_vehicle) {
    battle_fight_after_outcome_af13_to_b203_v4(
        original_vehicle_id, native_plus8_loop_slot,
        attacker_vehicle_record, defender_vehicle_record,
        attacker_faction, defender_faction, target_x, target_y,
        opaque_report_gate, frame_minus_38, frame_minus_30,
        frame_minus_58);
    battle_fight_b203_score_and_neighbor_refresh_v4(
        direction_stack_slot, native_plus8_loop_slot, attacker_faction,
        target_x, target_y, *frame_minus_58,
        frame_minus_40_last_vehicle);
}

// Typed post-redraw notification/UI continuation [B354,B63F). It ends at the
// native join; the existing typed-v1 selector helper owns B63F..B6F6.
static void battle_fight_post_redraw_notifications_v4(
    int direction_snapshot_at_b308,
    int attacker_faction,
    int defender_faction,
    int frame_minus_18,
    int frame_minus_30,
    int frame_minus_38,
    int frame_minus_3c,
    int frame_minus_40,
    int frame_minus_58,
    int frame_minus_64,
    int frame_minus_88,
    const char *native_text_buffer) {
    const int local_faction_at_b354 = ConsoleGlobal.faction_id_;

    bool frame_minus_30_notice_done = false;
    if (direction_snapshot_at_b308 != 0 && frame_minus_58 != 0 &&
        attacker_faction == local_faction_at_b354) {
        parse_num(0, frame_minus_58);
        NetMsgPrimary.pop("HUSKS", 5000, 0, 0);
        if (frame_minus_30 != 0) {
            frame_minus_30_notice_done = true;
            parse_num(0, frame_minus_30);
            const int local_faction_after_notice = ConsoleGlobal.faction_id_;
            if (defender_faction == local_faction_after_notice) {
                clear();
                says("THEIRBOOTY");
                if (has_treaty(attacker_faction, defender_faction, 0x10)) {
                    say_num(range(frame_minus_30, 1, 2));
                    NetMsgPrimary.pop(StringTemp, 5000, 0, 0);
                } else {
                    parse_says(0, get_noun(attacker_faction), -1, -1);
                    say_num(3);
                    NetMsgPrimary.pop(StringTemp, 5000, 0, 0);
                }
            } else if (attacker_faction == local_faction_after_notice) {
                clear();
                says("OURBOOTY");
                say_num(range(frame_minus_30, 1, 2));
                NetMsgPrimary.pop(StringTemp, 5000, 0, 0);
            }
        }
    } else {
        if (frame_minus_38 > 1 || frame_minus_18 != 0) {
            const int local_faction = ConsoleGlobal.faction_id_;
            if (attacker_faction == local_faction ||
                defender_faction == local_faction) {
                parse_num(0, frame_minus_38);
                parse_num(1, frame_minus_30);
                clear();
                const bool attacker_is_local = attacker_faction == local_faction;
                const bool input_is_nonzero = direction_snapshot_at_b308 != 0;
                if (attacker_is_local == input_is_nonzero) {
                    says("NUMGOT");
                } else {
                    says("NUMLOST");
                }
                if (frame_minus_38 > 1 && frame_minus_18 != 0) {
                    says("2");
                } else if (frame_minus_38 <= 1 && frame_minus_18 != 0) {
                    says("1");
                }
                NetMsgPrimary.pop(StringTemp, 5000, 0, 0);
            }
        }
    }

    if (!frame_minus_30_notice_done && frame_minus_30 != 0) {
        parse_num(0, frame_minus_30);
        const int local_faction = ConsoleGlobal.faction_id_;
        if (defender_faction != local_faction) {
            if (attacker_faction == local_faction) {
                clear();
                says("OURBOOTY");
                say_num(range(frame_minus_30, 1, 2));
                NetMsgPrimary.pop(StringTemp, 5000, 0, 0);
            }
        } else {
            clear();
            says("THEIRBOOTY");
            if (has_treaty(attacker_faction, defender_faction, 0x10)) {
                say_num(range(frame_minus_30, 1, 2));
                NetMsgPrimary.pop(StringTemp, 5000, 0, 0);
            } else {
                parse_says(0, get_noun(attacker_faction), -1, -1);
                say_num(3);
                NetMsgPrimary.pop(StringTemp, 5000, 0, 0);
            }
        }
    }

    if (frame_minus_3c != 0 && strlen(native_text_buffer) != 0) {
        parse_says(0, native_text_buffer, -1, -1);
        NetMsgPrimary.pop("NERVEGAS", 5000, 0, 0);
    }

    if (frame_minus_64 != 0) {
        const int proto_id = Vehs[frame_minus_40].proto_id;
        parse_says(0, VehPrototypes[proto_id].veh_name, -1, -1);
        parse_says(1, get_adjective(defender_faction), -1, -1);
        const int local_faction = ConsoleGlobal.faction_id_;
        if (defender_faction == local_faction) {
            NetMsgPrimary.pop("WEDISENGAGED", 5000, 0, 0);
        } else if (attacker_faction == local_faction) {
            NetMsgPrimary.pop("THEYDISENGAGED", 5000, 0, 0);
        }
    }

    ConsoleStatusWin.redraw();
    if (frame_minus_88 != 0) {
        TutWinBaseWindow.focus();
        MainInterfaceGlobal.redraw();
        do_all_draws();
    }

    if ((GameDataStateStorage.fields_.game_more_preferences_ & 0x10u) != 0) {
        g_BattleWin.stop_timer();
        popp(ScriptPopupFile, "BATTLEPAUSE", 0, "batwon_sm.pcx", 0);
        g_BattleWin.pulse_timer();
        if (ConsoleGroupAction != 0) {
            GameDataStateStorage.fields_.game_more_preferences_ &= ~0x10u;
            prefs_save(0);
        }
    }
}

// Ordered join from the already-translated native redraw pair to B354, then
// this report/UI tail. B639 is a genuine bypass and belongs to the later
// final/selector route, not the B354 notification body.
static int battle_fight_b2fa_to_b63f_v4(
    int frame_plus_1c,
    int direction_snapshot_at_b308,
    int frame_minus_2c_x,
    int frame_minus_28_y,
    int frame_plus_10_x,
    int frame_plus_14_y,
    int attacker_faction,
    int defender_faction,
    int frame_minus_18,
    int frame_minus_30,
    int frame_minus_38,
    int frame_minus_3c,
    int frame_minus_40,
    int frame_minus_58,
    int frame_minus_64,
    int frame_minus_88,
    const char *native_text_buffer) {
    const BattleFightRedrawRouteV7 redraw_route =
        battle_fight_redraw_pair_v7(
            frame_plus_1c, direction_snapshot_at_b308,
            frame_minus_2c_x, frame_minus_28_y,
            frame_plus_10_x, frame_plus_14_y);
    if (redraw_route == BATTLE_FIGHT_REDRAW_ROUTE_B639_V7) {
        return 0;
    }
    battle_fight_post_redraw_notifications_v4(
        direction_snapshot_at_b308, attacker_faction, defender_faction, frame_minus_18,
        frame_minus_30, frame_minus_38, frame_minus_3c,
        frame_minus_40, frame_minus_58, frame_minus_64,
        frame_minus_88, native_text_buffer);
    return 1;
}
// Retained fragment: battle-killoutcome-subagent/battle-fight-killoutcome-typed-v1.cpp
// Complete native selector switch at battle_fight 0x50B63F..0x50B6F6.
// The selector's semantic meaning remains unresolved. The caller supplies the
// native literal pointers; these are values, not newly assigned message names.
// Requires faction.h (get_adjective/get_noun), general.h (parse_says), and
// netmsg.h (NetMsgPrimary).
static void battle_fight_emit_selector_notice_v1(
    int native_selector,
    int attacker_faction,
    int defender_faction,
    const char *selector_one_label,
    const char *selector_two_label,
    const char *selector_three_label) {
    const char *message = 0;

    switch (native_selector) {
    case 1:
        parse_says(0, get_adjective(attacker_faction), -1, -1);
        parse_says(1, get_noun(defender_faction), -1, -1);
        message = selector_one_label;
        break;
    case 2:
        parse_says(0, get_adjective(attacker_faction), -1, -1);
        parse_says(1, get_noun(defender_faction), -1, -1);
        message = selector_two_label;
        break;
    case 3:
        parse_says(0, get_noun(attacker_faction), -1, -1);
        parse_says(1, get_noun(defender_faction), -1, -1);
        message = selector_three_label;
        break;
    default:
        return;
    }

    NetMsg::pop(message, 0);
}
// Retained fragment: battle-final-tail-subagent/battle-fight-final-block-v5.cpp
// Private typed reconstruction of native battle_fight 0x50B6F7..0x50B893.
// Inclusion-context fragment only; not the complete battle_fight body.
#include <stdint.h>

// The reference aliases the enclosing body's live [ebp+0x0c] parameter slot.
// Native reloads that slot after interlude at 0x50B755; a by-value snapshot is
// not equivalent if the callback mutates the slot. Earlier native stores also
// mean its value need not equal the caller's original direction argument.
static int battle_fight_final_block_v5(
    int &parameter_slot_value,
    int target_x,
    int target_y,
    int attacker_prototype_id,
    int defender_prototype_id,
    int attacker_faction,
    int defender_faction,
    int run_interlude,
    int target_base_id) {
    VehPrototypes[attacker_prototype_id].combat_factions = static_cast<int8_t>(
        static_cast<uint8_t>(VehPrototypes[attacker_prototype_id].combat_factions) |
        static_cast<uint8_t>(1u << (static_cast<uint8_t>(defender_faction) & 0x1f)));
    VehPrototypes[defender_prototype_id].combat_factions = static_cast<int8_t>(
        static_cast<uint8_t>(VehPrototypes[defender_prototype_id].combat_factions) |
        static_cast<uint8_t>(1u << (static_cast<uint8_t>(attacker_faction) & 0x1f)));

    if (run_interlude != 0) {
        interlude(0, 0, 1, 0);
    }

    // 0x50B755 observes the reference's current value after interlude.
    if (parameter_slot_value == 0) {
        return 0;
    }
    int result = 1; // native EBX; kill below clears it.

    // 0x50B765 reloads VehSelectionFollowLimit after interlude. Keep the
    // numeric slot ID, then re-index Vehs after each callback (no stale ref).
    const int turn_vehicle_id = GameDataStateStorage.fields_.vehicle_selection_follow_limit_;
    int turn_chassis_id =
        VehPrototypes[Vehs[turn_vehicle_id].proto_id].chassis_id;

    // Native RulesChassis offsets +0x7A and +0x7D are element 4 and element 7
    // of this existing byte-array owner; their game meaning remains unknown.
    const uint8_t triad = Chassis[turn_chassis_id].triad;
    if (triad == TRIAD_AIR &&
        Chassis[turn_chassis_id].sprite_unk2_y_off[4] == 1) {
        // 0x50B799/0x50B7A2: exact air + byte==1 case does not skip.
    } else if (triad == TRIAD_SEA || triad == TRIAD_AIR) {
        veh_skip(turn_vehicle_id);
        // Native byte addition wraps modulo 256; callback may mutate the slot.
        Vehs[turn_vehicle_id].moves_expended = static_cast<uint8_t>(
            Vehs[turn_vehicle_id].moves_expended + Rules.move_rate_roads);
    }

    // Native reloads prototype/chassis after veh_skip, then kill() and clear
    // EBX if byte +0x7D is nonzero.
    turn_chassis_id = VehPrototypes[Vehs[turn_vehicle_id].proto_id].chassis_id;
    if (Chassis[turn_chassis_id].sprite_unk2_y_off[7] != 0) {
        kill(turn_vehicle_id);
        result = 0;
    }

    // 0x50B81C..0x50B842 gates the final special return. veh_who is called
    // only if the attacker faction bit is absent and target_base_id is valid;
    // its return is tested, not ignored: nonnegative bypasses the special arm.
    const uint8_t attacker_bit = static_cast<uint8_t>(
        1u << (static_cast<uint8_t>(attacker_faction) & 0x1f));
    if ((GameDataStateStorage.fields_.lock_masks_.enable_mask_ & attacker_bit) == 0 && target_base_id >= 0 &&
        veh_who(target_x, target_y) < 0) {
        turn_chassis_id = VehPrototypes[Vehs[turn_vehicle_id].proto_id].chassis_id;
        if (Chassis[turn_chassis_id].triad == TRIAD_AIR &&
            Chassis[turn_chassis_id].sprite_unk2_y_off[4] != 0 &&
            static_cast<int>(Vehs[turn_vehicle_id].terraforming_turns) + 1 >=
                Chassis[turn_chassis_id].sprite_unk2_y_off[4]) {
            return 1;
        }
    }
    return result;
}

// Composition-local spelling of outcome-join-v4 with its verified 0x93A95C
// owner read directly at the alternate-notice predicate. This preserves the
// counters and both distinct notice forms while avoiding a frame snapshot of
// IsMultiplayerPBEM. The source owner is game.h/game.cpp, not an opaque slot.
enum BattleFightOutcomeJoinV4 {
    BATTLE_FIGHT_OUTCOME_JOIN_A730_V4 = 0,
    BATTLE_FIGHT_OUTCOME_JOIN_AF13_V4 = 1,
    BATTLE_FIGHT_OUTCOME_JOIN_B203_V4 = 2
};

static BattleFightOutcomeJoinV4 battle_fight_outcome_join_live_v1(
    int frame_minus_50,
    int &frame_minus_64,
    int &direction_parameter_plus_0c,
    int &source_mode_parameter_plus_18,
    int &frame_minus_38,
    int &frame_minus_30,
    int &frame_minus_58,
    Veh &attacker_record_minus_04,
    Veh &defender_record_minus_08,
    int attacker_faction_minus_10,
    int defender_faction_minus_0c,
    int target_x_plus_10,
    int target_y_plus_14) {
    const int minus_50_was_nonzero = frame_minus_50 != 0;
    frame_minus_38 = 0;
    frame_minus_30 = 0;
    frame_minus_58 = 0;
    source_mode_parameter_plus_18 = 0;

    if (minus_50_was_nonzero) {
        return BATTLE_FIGHT_OUTCOME_JOIN_B203_V4;
    }
    if (frame_minus_64 != 0) {
        return BATTLE_FIGHT_OUTCOME_JOIN_B203_V4;
    }
    if (direction_parameter_plus_0c == 0) {
        return BATTLE_FIGHT_OUTCOME_JOIN_AF13_V4;
    }

    int pair_attack_count = PlayersData[attacker_faction_minus_10].diplo_unk3[
        defender_faction_minus_0c];
    PlayersData[attacker_faction_minus_10].diplo_unk3[
        defender_faction_minus_0c] = pair_attack_count + 1;
    int pair_diplomacy_count = PlayersData[attacker_faction_minus_10].diplo_unk4[
        defender_faction_minus_0c];
    int attacker_wins = FactionTurnCombatWins[attacker_faction_minus_10];
    PlayersData[attacker_faction_minus_10].diplo_unk4[
        defender_faction_minus_0c] = pair_diplomacy_count + 1;
    int defender_losses = FactionTurnCombatLosses[defender_faction_minus_0c];
    const int multiplayer_at_native_load = IsMultiplayerNet;
    FactionTurnCombatWins[attacker_faction_minus_10] = attacker_wins + 1;
    FactionTurnCombatLosses[defender_faction_minus_0c] = defender_losses + 1;

    if (multiplayer_at_native_load != 0 &&
        defender_faction_minus_0c == ConsoleGlobal.faction_id_) {
        int proto_id = defender_record_minus_08.proto_id;
        parse_says(0, VehPrototypes[proto_id].veh_name, -1, -1);
        StringTemp[0] = '\0';
        proto_id = defender_record_minus_08.proto_id;
        say_stats_3(StringTemp, proto_id);
        parse_says(1, StringTemp, -1, -1);
        parse_says(2, get_adjective(attacker_faction_minus_10), -1, -1);
        proto_id = attacker_record_minus_04.proto_id;
        parse_says(3, VehPrototypes[proto_id].veh_name, -1, -1);
        StringTemp[0] = '\0';
        proto_id = attacker_record_minus_04.proto_id;
        say_stats_3(StringTemp, proto_id);
        parse_says(4, StringTemp, -1, -1);
        const int local_faction_at_native_base_find = ConsoleGlobal.faction_id_;
        const int base_id = base_find(
            target_x_plus_10, target_y_plus_14, -1, -1, -1,
            local_faction_at_native_base_find);
        if (base_id >= 0) {
            parse_says(5, Bases[base_id].name_string, -1, -1);
        }
        NetMsgPrimary.pop("COMBATLOSS", 5000, 0, 0);
        return BATTLE_FIGHT_OUTCOME_JOIN_A730_V4;
    }

    const uint8_t defender_bit = static_cast<uint8_t>(
        1u << (static_cast<uint32_t>(defender_faction_minus_0c) & 0x1Fu));
    if (IsMultiplayerPBEM == 0 ||
        (static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) & defender_bit) == 0) {
        return BATTLE_FIGHT_OUTCOME_JOIN_A730_V4;
    }

    int proto_id = defender_record_minus_08.proto_id;
    parse_says(0, VehPrototypes[proto_id].veh_name, -1, -1);
    StringTemp[0] = '\0';
    proto_id = defender_record_minus_08.proto_id;
    say_stats_3(StringTemp, proto_id);
    parse_says(1, StringTemp, -1, -1);
    parse_says(2, get_adjective(attacker_faction_minus_10), -1, -1);
    proto_id = attacker_record_minus_04.proto_id;
    parse_says(3, VehPrototypes[proto_id].veh_name, -1, -1);
    StringTemp[0] = '\0';
    proto_id = attacker_record_minus_04.proto_id;
    say_stats_3(StringTemp, proto_id);
    parse_says(4, StringTemp, -1, -1);
    const int base_id = base_find(
        target_x_plus_10, target_y_plus_14, -1, -1, -1,
        defender_faction_minus_0c);
    if (base_id >= 0) {
        parse_says(5, Bases[base_id].name_string, -1, -1);
    }
    POP3("COMBATLOSS", static_cast<const char *>(0),
         defender_faction_minus_0c, target_x_plus_10, target_y_plus_14);
    return BATTLE_FIGHT_OUTCOME_JOIN_A730_V4;
}

// AF13 route copied from post-outcome-v4 solely to bind the now-known PBEM
// global at its native branch, after the ordered counters and the local
// NetMsg branch. All notice/callback order remains the frozen v4 order.
static void battle_fight_after_outcome_af13_live_v1(
    int original_vehicle_id,
    int &native_plus8_loop_slot,
    Veh &attacker_vehicle_record,
    Veh &defender_vehicle_record,
    int attacker_faction,
    int defender_faction,
    int target_x,
    int target_y,
    int *frame_minus_38,
    int *frame_minus_30,
    int *frame_minus_58) {
    const int multiplayer_gate_value = IsMultiplayerNet;
    ++FactionTurnCombatWins[defender_faction];
    ++FactionTurnCombatLosses[attacker_faction];

    if (multiplayer_gate_value != 0 &&
        defender_faction == ConsoleGlobal.faction_id_) {
        int proto_id = defender_vehicle_record.proto_id;
        parse_says(0, VehPrototypes[proto_id].veh_name, -1, -1);
        clear();
        proto_id = defender_vehicle_record.proto_id;
        say_stats_3(proto_id);
        parse_says(1, StringTemp, -1, -1);
        parse_says(2, get_adjective(attacker_faction), -1, -1);
        proto_id = attacker_vehicle_record.proto_id;
        parse_says(3, VehPrototypes[proto_id].veh_name, -1, -1);
        clear();
        proto_id = attacker_vehicle_record.proto_id;
        say_stats_3(proto_id);
        parse_says(4, StringTemp, -1, -1);
        const int base_id = base_find(target_x, target_y, -1, -1, -1,
                                      ConsoleGlobal.faction_id_);
        if (base_id >= 0) {
            parse_says(5, Bases[base_id].name_string, -1, -1);
        }
        NetMsgPrimary.pop("COMBATWIN", 5000, 0, 0);
    } else if (IsMultiplayerPBEM != 0 &&
               ((static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) &
                 static_cast<uint8_t>(1u <<
                     (static_cast<uint32_t>(defender_faction) & 0x1Fu))) != 0)) {
        int proto_id = defender_vehicle_record.proto_id;
        parse_says(0, VehPrototypes[proto_id].veh_name, -1, -1);
        clear();
        proto_id = defender_vehicle_record.proto_id;
        say_stats_3(proto_id);
        parse_says(1, StringTemp, -1, -1);
        parse_says(2, get_adjective(attacker_faction), -1, -1);
        proto_id = attacker_vehicle_record.proto_id;
        parse_says(3, VehPrototypes[proto_id].veh_name, -1, -1);
        clear();
        proto_id = attacker_vehicle_record.proto_id;
        say_stats_3(proto_id);
        parse_says(4, StringTemp, -1, -1);
        const int base_id = base_find(target_x, target_y, -1, -1, -1,
                                      defender_faction);
        if (base_id >= 0) {
            parse_says(5, Bases[base_id].name_string, -1, -1);
        }
        POP3("COMBATWIN", static_cast<const char *>(0), defender_faction,
             target_x, target_y);
    }

    native_plus8_loop_slot = original_vehicle_id;
    stack_veh(native_plus8_loop_slot, 1);
    battle_fight_call_stack_kill_v2(
        native_plus8_loop_slot, frame_minus_38, frame_minus_30,
        frame_minus_58, defender_faction);
    ++PlayersData[defender_faction].diplo_unk3[attacker_faction];
    ++PlayersData[defender_faction].diplo_unk4[attacker_faction];

    const BattleFightReportContinuationRouteV4 route =
        attacker_faction != 0 && defender_faction != 0
            ? BATTLE_FIGHT_REPORT_ROUTE_B1B4_V4
            : BATTLE_FIGHT_REPORT_ROUTE_B1BC_V4;
    battle_fight_resume_report_route_v4(
        route, attacker_faction, defender_faction, frame_minus_30);
}

static void battle_fight_af13_to_b2fa_live_v1(
    int original_vehicle_id,
    int &direction_stack_slot,
    int &native_plus8_loop_slot,
    Veh &attacker_vehicle_record,
    Veh &defender_vehicle_record,
    int attacker_faction,
    int defender_faction,
    int target_x,
    int target_y,
    int *frame_minus_38,
    int *frame_minus_30,
    int *frame_minus_58,
    int &frame_minus_40_last_vehicle) {
    battle_fight_after_outcome_af13_live_v1(
        original_vehicle_id, native_plus8_loop_slot,
        attacker_vehicle_record, defender_vehicle_record,
        attacker_faction, defender_faction, target_x, target_y,
        frame_minus_38, frame_minus_30, frame_minus_58);
    battle_fight_b203_score_and_neighbor_refresh_v4(
        direction_stack_slot, native_plus8_loop_slot,
        attacker_faction, target_x, target_y, *frame_minus_58,
        frame_minus_40_last_vehicle);
}

// Every field is a source-frame value or a pointer to that live value. The
// packet deliberately avoids a byte-layout struct pretending to be EBP: the
// owning body constructs this view from its real locals and argument slots.
// Offsets remain in member names wherever identity is still uncertain.
struct BattleFightLateFrameV1 {
    int frame_minus_48;
    int frame_minus_50;
    // Aliases the outer status_animation_flag local, whose native producers
    // include the zero at 0x5083E0 and the conditional one at 0x508B88; B354
    // reloads this live value at the mapped [ebp-0x88] use.
    int *status_animation_flag_minus_88;
    int frame_minus_90_proto;
    int frame_minus_94_proto;

    int attacker_vehicle_id;
    int defender_vehicle_id;
    int frame_minus_18_vehicle_id;
    int frame_minus_8_vehicle_id;
    int frame_vehicle_id_at_a904;
    int frame_minus_10;
    int attacker_faction;
    int defender_faction;
    int target_x;
    int target_y;
    int attacker_tile_x_minus_2c;
    int attacker_tile_y_minus_28;
    int target_stack_id; // overwritten by the native AC01 live stack lookup.
    int target_base_id;
    int stats_proto_minus_18;
    int cached_attacker_hp_minus_60;
    int cached_defender_hp_minus_5c;
    int native_esi_before_a80e_test;
    const char *native_text_buffer_minus_138;

    int *direction_slot_plus_0c;
    int *source_mode_slot_plus_18;
    int *frame_plus_08_loop_slot;
    int *frame_plus_1c;
    int *frame_plus_20;
    int *frame_minus_20;
    int *frame_minus_18;
    int *frame_minus_38;
    int *frame_minus_30;
    int *frame_minus_3c;
    int *frame_minus_40;
    int *frame_minus_44;
    int *frame_minus_58;
    int *frame_minus_64_slot;
    // Points at the owning body's existing char base_name_copy[80]; this
    // continuation retains the live buffer and never creates/clears a copy.
    char *base_name_copy_80;

    // These are fixed-table records, but their fields are re-read after every
    // callback by the called blocks. Preserve the selected -0x8 EDI value at
    // A730 as a pointer, never as a cached prototype/faction scalar.
    Veh *attacker_record_minus_04;
    Veh *defender_record_minus_08;
    Veh *continuation_edi_at_a730;
};

enum BattleFightLateEntryRouteV1 {
    BATTLE_FIGHT_LATE_RETURN_A110_V1 = 0,
    BATTLE_FIGHT_LATE_JOIN_A730_V1 = 1,
    BATTLE_FIGHT_LATE_JOIN_AF13_V1 = 2,
    BATTLE_FIGHT_LATE_JOIN_B203_V1 = 3
};

// Compose the first live fork and A44D report block. A110's survival notice
// reads IsMultiplayerNet and the local faction at its native gate. The
// alternate A44D notice rereads IsMultiplayerPBEM at its predicate after the
// ordered counters; neither owner is accepted from a caller snapshot.
static BattleFightLateEntryRouteV1 battle_fight_late_enter_v1(
    BattleFightLateFrameV1 &frame,
    int &strength_14,
    int &strength_1c,
    int &frame_plus_20_value) {
    const BattleFightA119PreStrengthRouteV3 incoming =
        battle_fight_route_before_a119_v3(
            frame.frame_minus_48,
            frame.attacker_vehicle_id,
            frame.defender_vehicle_id);

    if (incoming == BATTLE_FIGHT_A119_RETURN_AT_A110_V3) {
        if (IsMultiplayerNet != 0 &&
            frame.defender_faction == ConsoleGlobal.faction_id_) {
            battle_fight_report_tail_v3(
                1, frame.attacker_faction,
                frame.attacker_vehicle_id, frame.defender_vehicle_id,
                frame.defender_faction, frame.stats_proto_minus_18,
                frame.target_x, frame.target_y, "COMBATSURVIVE",
                *frame.direction_slot_plus_0c);
        }
        return BATTLE_FIGHT_LATE_RETURN_A110_V1;
    }

    // A119's arithmetic SAR store is unconditional on its incoming edge and
    // precedes map/diplomacy callbacks. +0x20 is later reused as scratch at
    // AAE0; the same live slot is retained through both lifetimes.
    frame_plus_20_value = battle_fight_arithmetic_half_v3(frame.target_x);
    *frame.frame_plus_20 = frame_plus_20_value;
    battle_fight_note_third_faction_v1(
        frame.target_x, frame.target_y,
        frame.attacker_faction, frame.defender_faction);
    battle_fight_strength_promotion_path_v2(
        *frame.direction_slot_plus_0c,
        frame.attacker_vehicle_id, frame.defender_vehicle_id,
        frame.attacker_faction, frame.defender_faction,
        frame.target_base_id,
        frame.cached_attacker_hp_minus_60,
        frame.cached_defender_hp_minus_5c,
        strength_14, strength_1c);

    const BattleFightOutcomeJoinV4 join = battle_fight_outcome_join_live_v1(
        frame.frame_minus_50,
        *frame.frame_minus_64_slot,
        *frame.direction_slot_plus_0c,
        *frame.source_mode_slot_plus_18,
        *frame.frame_minus_38,
        *frame.frame_minus_30,
        *frame.frame_minus_58,
        *frame.attacker_record_minus_04,
        *frame.defender_record_minus_08,
        frame.attacker_faction, frame.defender_faction,
        frame.target_x, frame.target_y);

    if (join == BATTLE_FIGHT_OUTCOME_JOIN_A730_V4) {
        // All native A730 predecessors establish EDI=[ebp-8]. Retain the
        // source pointer across the next composed stage; use record fields
        // live rather than snapshotting them.
        frame.continuation_edi_at_a730 = frame.defender_record_minus_08;
        return BATTLE_FIGHT_LATE_JOIN_A730_V1;
    }
    if (join == BATTLE_FIGHT_OUTCOME_JOIN_AF13_V4) {
        return BATTLE_FIGHT_LATE_JOIN_AF13_V1;
    }
    return BATTLE_FIGHT_LATE_JOIN_B203_V1;
}

// Compose the A730 selector, A80E kill branch, A904 rearm loop, AAE0 removal,
// and AC8B..AF13 base/objective branch. The frame locals that cannot yet be
// semantically named remain supplied under their proven offsets.
static void battle_fight_late_capture_to_b2fa_v1(
    BattleFightLateFrameV1 &frame) {
    BattleFightNativeRouteV3 capture_route =
        battle_fight_route_capture_gate_v3(
            *frame.frame_minus_20, *frame.frame_minus_18,
            frame.defender_vehicle_id, frame.attacker_vehicle_id,
            frame.target_x, frame.target_y);

    if (capture_route == BATTLE_FIGHT_NATIVE_ROUTE_A80E_V3) {
        const BattleFightNativeRouteA80E_V4 kill_route =
            battle_fight_route_or_kill_v4(
                frame.native_esi_before_a80e_test,
                *frame.frame_minus_20, *frame.frame_minus_18,
                frame.defender_vehicle_id, frame.attacker_vehicle_id,
                frame.frame_minus_38, frame.frame_minus_30,
                frame.frame_minus_58, frame.attacker_vehicle_id,
                frame.frame_minus_10, frame.frame_minus_40,
                frame.frame_minus_40);
        if (kill_route == BATTLE_FIGHT_NATIVE_ROUTE_A904_V4) {
            capture_route = BATTLE_FIGHT_NATIVE_ROUTE_A904_V3;
        } else {
            capture_route = BATTLE_FIGHT_NATIVE_ROUTE_A80E_V3;
        }
    }

    if (capture_route == BATTLE_FIGHT_NATIVE_ROUTE_A904_V3) {
        battle_fight_rearm_loop_v1(
            frame.frame_vehicle_id_at_a904,
            *frame.frame_minus_18,
            frame.attacker_vehicle_id,
            frame.target_x, frame.target_y,
            *frame.source_mode_slot_plus_18);
        battle_fight_remove_damaged_stack_v22(
            *frame.frame_minus_18,
            frame.frame_minus_8_vehicle_id,
            frame.frame_minus_10,
            frame.attacker_vehicle_id,
            frame.frame_plus_20,
            frame.frame_minus_38,
            frame.frame_minus_30,
            frame.frame_minus_58,
            frame.target_x, frame.target_y,
            frame.source_mode_slot_plus_18);
    }

    // Native 0x50AC01 reacquires the target's live normalized stack after
    // the A730/A80E/A904 mutation callbacks; never reuse the entry stack head.
    frame.target_stack_id = stack_fix(veh_at(frame.target_x, frame.target_y));
    const BattleFightBaseRouteV1 base_route =
        battle_fight_base_objective_v1(
            *frame.frame_minus_20,
            frame.target_x, frame.target_y, frame.target_stack_id,
            frame.attacker_tile_x_minus_2c,
            frame.attacker_tile_y_minus_28,
            frame.attacker_faction, frame.defender_faction,
            *frame.frame_minus_3c, *frame.frame_minus_44);
    const BattleFightBaseContinuationRouteV8 continuation_route =
        base_route == BATTLE_FIGHT_BASE_TO_B1B4_V1
            ? BATTLE_FIGHT_BASE_ROUTE_B1B4_V8
            : BATTLE_FIGHT_BASE_ROUTE_B1BC_V8;
    battle_fight_base_route_to_b2fa_v4(
        continuation_route,
        frame.attacker_faction, frame.defender_faction,
        frame.frame_minus_30,
        *frame.direction_slot_plus_0c,
        *frame.frame_plus_08_loop_slot,
        frame.target_x, frame.target_y,
        *frame.frame_minus_58,
        *frame.frame_minus_40);
}

// Join all three incoming arms at B2FA, then follow the B2FA/B354/B63F and
// B6F7 tail. Both B639 bypass and the B354 notification route continue at
// B63F; selector values outside 1..3 emit no notice but still reach B6F7.
static int battle_fight_late_finish_b2fa_v1(
    BattleFightLateFrameV1 &frame,
    int direction_snapshot_at_b308,
    int frame_minus_78_interlude_gate) {
    (void)battle_fight_b2fa_to_b63f_v4(
        *frame.frame_plus_1c, direction_snapshot_at_b308,
        frame.attacker_tile_x_minus_2c, frame.attacker_tile_y_minus_28,
        frame.target_x, frame.target_y,
        frame.attacker_faction, frame.defender_faction,
        *frame.frame_minus_18,
        *frame.frame_minus_30, *frame.frame_minus_38,
        *frame.frame_minus_3c,
        *frame.frame_minus_40,
        *frame.frame_minus_58,
        *frame.frame_minus_64_slot,
        *frame.status_animation_flag_minus_88,
        frame.native_text_buffer_minus_138);

    battle_fight_emit_selector_notice_v1(
        *frame.frame_minus_44,
        frame.attacker_faction, frame.defender_faction,
        "VENDETTAREPORT", "PACTATTACKED", "PACTATTACKING");

    return battle_fight_final_block_v5(
        *frame.direction_slot_plus_0c,
        frame.target_x, frame.target_y,
        frame.frame_minus_90_proto, frame.frame_minus_94_proto,
        frame.attacker_faction, frame.defender_faction,
        frame_minus_78_interlude_gate,
        *frame.frame_minus_20);
}

// End-to-end dispatcher for the native continuation. The A730 branch must
// pass through `battle_fight_late_capture_to_b2fa_v1`; AF13 enters its own
// ordered accounting path; B203 enters the shared score/neighbor block.
// `battle_fight_af13_to_b2fa_v4` is intentionally invoked here only for the
// AF13 edge so it cannot double-count the A44D A730 arm.
static int battle_fight_late_compose_v1(
    BattleFightLateFrameV1 &frame,
    int &strength_14,
    int &strength_1c,
    int &frame_plus_20_value,
    int direction_snapshot_at_b308,
    int frame_minus_78_interlude_gate) {
    const BattleFightLateEntryRouteV1 route = battle_fight_late_enter_v1(
        frame, strength_14, strength_1c, frame_plus_20_value);
    if (route == BATTLE_FIGHT_LATE_RETURN_A110_V1) {
        return 0;
    }

    if (route == BATTLE_FIGHT_LATE_JOIN_A730_V1) {
        battle_fight_late_capture_to_b2fa_v1(frame);
    } else if (route == BATTLE_FIGHT_LATE_JOIN_AF13_V1) {
        battle_fight_af13_to_b2fa_live_v1(
            frame.attacker_vehicle_id,
            *frame.direction_slot_plus_0c,
            *frame.frame_plus_08_loop_slot,
            *frame.attacker_record_minus_04,
            *frame.defender_record_minus_08,
            frame.attacker_faction, frame.defender_faction,
            frame.target_x, frame.target_y,
            frame.frame_minus_38,
            frame.frame_minus_30,
            frame.frame_minus_58,
            *frame.frame_minus_40);
    } else {
        battle_fight_b203_score_and_neighbor_refresh_v4(
            *frame.direction_slot_plus_0c,
            *frame.frame_plus_08_loop_slot,
            frame.attacker_faction,
            frame.target_x, frame.target_y,
            *frame.frame_minus_58,
            *frame.frame_minus_40);
    }

    return battle_fight_late_finish_b2fa_v1(
        frame, direction_snapshot_at_b308,
        frame_minus_78_interlude_gate);
}

// Retained fragment: battle-report-subagent/battle-fight-a110-a119-orchestrator-v1.cpp
// Private flattened composition of the pre-strength A110/A119 fork through
// the A1B3 strength join. This is a helper for composing the owning body, not
// a complete battle_fight definition or a source mapping.
// Include strength-promotion-v2.cpp, report-fragments-v1.cpp,
// report-context-v3.cpp and report-tail-v3.cpp first.
enum BattleFightPreStrengthJoinV1 {
    BATTLE_FIGHT_PRESTRENGTH_RETURN_A110_V1 = 0,
    BATTLE_FIGHT_PRESTRENGTH_JOIN_A44D_V1 = 1
};

// `frame_minus_48`, `stats_proto_minus_18`, `attacker_faction_minus_10`,
// and `direction_slot_plus_0c` preserve neutral native frame identities.
// The latter is in/out because earlier battle code overwrites the ABI slot.
// `frame_plus_20` is written only on the A119 edge.
static BattleFightPreStrengthJoinV1
battle_fight_a110_a119_orchestrator_v1(
    int frame_minus_48,
    int attacker_vehicle_id,
    int defender_vehicle_id,
    int attacker_faction,
    int attacker_faction_minus_10,
    int defender_faction,
    int target_x,
    int target_y,
    int target_base_id,
    int stats_proto_minus_18,
    int cached_attacker_hp_minus_60,
    int cached_defender_hp_minus_5c,
    int &direction_slot_plus_0c,
    int &strength_14,
    int &strength_1c,
    int &frame_plus_20) {
    const BattleFightA119PreStrengthRouteV3 route =
        battle_fight_route_before_a119_v3(
            frame_minus_48, attacker_vehicle_id, defender_vehicle_id);

    if (route == BATTLE_FIGHT_A119_RETURN_AT_A110_V3) {
        // Native A110's optional network/local-defender COMBATSURVIVE report
        // has already passed the raw health/mode edge tests above.
        battle_fight_report_tail_v3(
            1, attacker_faction_minus_10, attacker_vehicle_id,
            defender_vehicle_id, defender_faction, stats_proto_minus_18,
            target_x, target_y, "COMBATSURVIVE", direction_slot_plus_0c);
        return BATTLE_FIGHT_PRESTRENGTH_RETURN_A110_V1;
    }

    // Every A119 entry executes signed arithmetic SAR(+0x10,1) into +0x20
    // before map/treaty work, including the no-friction case.
    frame_plus_20 = battle_fight_arithmetic_half_v3(target_x);
    battle_fight_note_third_faction_v1(
        target_x, target_y, attacker_faction, defender_faction);
    battle_fight_strength_promotion_path_v2(
        direction_slot_plus_0c, attacker_vehicle_id, defender_vehicle_id,
        attacker_faction, defender_faction, target_base_id,
        cached_attacker_hp_minus_60, cached_defender_hp_minus_5c,
        strength_14, strength_1c);
    return BATTLE_FIGHT_PRESTRENGTH_JOIN_A44D_V1;
}
// Retained fragment: battle-rounds-subagent/battle-fight-mid-composition-v3.cpp
// C++98 in-context composition for the live-alias battle_fight mid-CFG packets.
// Include the declared round, ownership, threshold, A110/A119 and support
// fragments before this file. This is an insertion fragment, not a complete
// battle_fight definition; native-local producers at 0x508C96..0x5090D7 are
// applied by battle_fight_apply_mid_frame_producers_v1 before entry here.

enum BattleFightMidComposedExitV3 {
    BATTLE_FIGHT_MID_RETURN_AT_A110_V3 = 0,
    BATTLE_FIGHT_MID_JOIN_A44D_V3 = 1
};

// These fields are native witnesses, not inferred domain names. In
// particular, `second_argument_slot` is the same mutable [ebp+0x0C] word that
// changes from direction to defender faction and then health limit.
struct BattleFightMidNativeSlotsV3 {
    int target_x;
    int target_y;
    int target_base_id_minus_20;
    int anchor_x_minus_2c;
    int anchor_y_minus_28;
    int *candidate_x_plus_20;
    int *candidate_y_minus_34;
    int *second_argument_slot_plus_0c;
    int *attacker_faction_slot_minus_50;
    int *display_latch_minus_64;
    int *outcome_gate_minus_48;
    int *branch_selector_minus_54;
    int *odds_gate_minus_78;
    int *display_selector_slot_plus_1c;
    int *stack_vehicle_slot_minus_18;
    int status_value_minus_14;
    int status_value_minus_1c;
    int *source_mode_slot_plus_18;
    int attacker_faction_minus_10;
    int defender_faction_minus_0c;
    int stats_proto_minus_18;
    int cached_attacker_hp_minus_60;
    int cached_defender_hp_minus_5c;
    int *strength_14;
    int *strength_1c;
    int *frame_plus_20;
};

// Executes the composed 0x5090D7..0x50A44D route. The covered requested
// subrange ends at 0x50A119; on A119 this function continues only because the
// already-frozen A110/A119 owner helper carries the same live frame into its
// explicit A44D join. Every input is bound by the enclosing native frame.
static BattleFightMidComposedExitV3 battle_fight_compose_mid_cfg_v3(
    BattleFightRoundFrameV4 &round_frame,
    BattleFightRoundBindingsV4 &round_bindings,
    BattleFightMidNativeSlotsV3 &s) {
    const BattleFightRoundExitV4 round_exit =
        battle_fight_round_loop_v4(round_frame, round_bindings);

    // The loop writes these native scratch slots only when a better retreat
    // candidate wins the exact score comparison.
    if (round_exit == BFR_TO_509923_SELECTED_RETREAT_V4) {
        *s.candidate_x_plus_20 = round_frame.candidate_x_plus_20;
        *s.candidate_y_minus_34 = round_frame.candidate_y_minus_34;
    }

    if (round_exit == BFR_TO_509827_OWNERSHIP_ENTRY_V4) {
        battle_fight_ownership_transition_v2(
            s.target_x, s.target_y, *s.second_argument_slot_plus_0c,
            *s.attacker_faction_slot_minus_50, round_frame.attacker_id);
    }

    const int selected_retreat =
        round_exit == BFR_TO_509923_SELECTED_RETREAT_V4;
    const int direction_index = selected_retreat
        ? round_frame.best_direction : 0;
    const int threshold_result = battle_fight_outcome_threshold_block_v2(
        selected_retreat, direction_index,
        s.anchor_x_minus_2c, s.anchor_y_minus_28,
        s.target_x, s.target_y,
        *s.candidate_x_plus_20, *s.candidate_y_minus_34,
        *s.stack_vehicle_slot_minus_18, round_frame.attacker_id,
        round_frame.defender_id,
        *s.second_argument_slot_plus_0c, *s.display_selector_slot_plus_1c,
        *s.odds_gate_minus_78, *s.display_latch_minus_64,
        *s.outcome_gate_minus_48, *s.branch_selector_minus_54,
        *s.stack_vehicle_slot_minus_18, s.status_value_minus_14,
        s.status_value_minus_1c, *s.source_mode_slot_plus_18);

    // Threshold result 1 already clears the two globals at native 0x509F10.
    // Result 0 branches to 0x509F1D, whose first common effect is that clear.
    if (threshold_result == 0) {
        VehDrawSelection689F28 = -1;
        VehDrawSelection689F2C = -1;
    }

    const BattleFightPreStrengthJoinV1 outcome_join =
        battle_fight_a110_a119_orchestrator_v1(
            *s.outcome_gate_minus_48, round_frame.attacker_id,
            round_frame.defender_id, s.attacker_faction_minus_10,
            *s.attacker_faction_slot_minus_50,
            s.defender_faction_minus_0c, s.target_x, s.target_y,
            s.target_base_id_minus_20, s.stats_proto_minus_18,
            s.cached_attacker_hp_minus_60, s.cached_defender_hp_minus_5c,
            *s.second_argument_slot_plus_0c, *s.strength_14,
            *s.strength_1c, *s.frame_plus_20);
    if (outcome_join == BATTLE_FIGHT_PRESTRENGTH_RETURN_A110_V1)
        return BATTLE_FIGHT_MID_RETURN_AT_A110_V3;
    return BATTLE_FIGHT_MID_JOIN_A44D_V3;
}
// Retained fragment: battle-rounds-subagent/battle-fight-mid-owner-v7.cpp
// Private C++98 integration block. Required fragments, in order, are the
// alias-aware round-loop-v3, ownership/follow-through/threshold helpers,
// mid dispatcher-v2, and corrected pre-round-v6. This does not define the
// full mapped battle_fight function.

enum BattleFightMidOwnerExitV7 {
    BATTLE_FIGHT_MID_EARLY_RETURN_ZERO_V7 = 0,
    BATTLE_FIGHT_MID_RETURN_AT_A110_V7 = 1,
    BATTLE_FIGHT_MID_JOIN_A44D_V7 = 2
};

struct BattleFightMidPreRoundInputsV7 {
    int source_x;
    int source_y;
    int combat_type;
    int *status_animation_flag_minus_88;
};

// Native 0x9A6554 is SecretProject.pholus_mutagen: a BASE ID, not a faction
// ID. The round candidate then checks Bases[base_id].faction_id_current.
static int battle_fight_pholus_project_base_v3() {
    return GameDataStateStorage.fields_.secret_project_.pholus_mutagen();
}

// All stack witnesses have already been produced at their native sites.
// `slots.stack_vehicle_minus_18` is kept distinct from `round_frame.defender_id`;
// [+0x0c], [-0x50], [-0x48], [-0x54], and +0x20 remain live storage aliases.
static BattleFightMidOwnerExitV7 battle_fight_mid_owner_v7(
    BattleFightRoundFrameV4 &round_frame,
    BattleFightRoundBindingsV4 &round_bindings,
    BattleFightMidNativeSlotsV3 &slots,
    const BattleFightMidPreRoundInputsV7 &pre_inputs) {
    round_bindings.current_faction = battle_fight_pholus_project_base_v3;

    const BattleFightPreRoundStateV7 pre_round =
        battle_fight_prepare_pre_round_v7(
            round_frame.attacker_id, round_frame.defender_id,
            *slots.stack_vehicle_slot_minus_18,
            slots.target_x, slots.target_y,
            pre_inputs.source_x, pre_inputs.source_y,
            *slots.source_mode_slot_plus_18, *slots.display_selector_slot_plus_1c,
            round_frame.attacker_combat_value,
            round_frame.defender_combat_value,
            pre_inputs.combat_type, slots.target_base_id_minus_20,
            round_frame.mutable_second_argument,
            pre_inputs.status_animation_flag_minus_88);
    if (pre_round.early_return != 0)
        return BATTLE_FIGHT_MID_EARLY_RETURN_ZERO_V7;

    // Producers at 0x508C96..0x5090D7 write their exact live frame slots.
    round_frame.round_or_candidate_index =
        pre_round.round_scan_gate_minus_24;
    round_frame.marine_detach_budget =
        pre_round.detachment_roll_budget_minus_80;
    round_frame.display_scale = pre_round.damage_band_scale_minus_6c;
    round_frame.post_hit_handoff_gate =
        pre_round.post_hit_handoff_gate_minus_48;
    round_frame.transition_type = pre_round.transition_type_minus_54;

    // Bind references, not copies: the native mutates and later reloads these
    // stack words in the ownership, threshold, capture, and A119 branches.
    slots.second_argument_slot_plus_0c =
        &round_frame.mutable_second_argument;
    slots.attacker_faction_slot_minus_50 =
        &round_frame.attacker_faction_local_50;
    slots.outcome_gate_minus_48 = &round_frame.post_hit_handoff_gate;
    slots.branch_selector_minus_54 = &round_frame.transition_type;

    const BattleFightMidComposedExitV3 exit =
        battle_fight_compose_mid_cfg_v3(round_frame, round_bindings, slots);
    return exit == BATTLE_FIGHT_MID_RETURN_AT_A110_V3
        ? BATTLE_FIGHT_MID_RETURN_AT_A110_V7
        : BATTLE_FIGHT_MID_JOIN_A44D_V7;
}

// Retained fragment: battle-report-subagent/battle-fight-after-a44d-composition-v3.cpp
// Private corrected A44D continuation for use after the mid-owner v3 block.
// This does not rerun A110/A119. The native B2FA helper reads the live -0x88
// status flag only after its redraw callbacks, through the pointer correction
// in battle-fight-b2fa-live-flag-v1.cpp.
// Retained fragment: battle-fight-b2fa-live-flag-v1.cpp
// Private correction for the B2FA -> B354 continuation. The native redraw
// callbacks run before B354 reads [ebp-0x88], so that slot must be dereferenced
// after those callbacks instead of copied at B2FA entry.
static int battle_fight_b2fa_to_b63f_live_v1(
    int frame_plus_1c,
    int direction_snapshot_at_b308,
    int frame_minus_2c_x,
    int frame_minus_28_y,
    int frame_plus_10_x,
    int frame_plus_14_y,
    int attacker_faction,
    int defender_faction,
    int frame_minus_18,
    int frame_minus_30,
    int frame_minus_38,
    int frame_minus_3c,
    int frame_minus_40,
    int frame_minus_58,
    int frame_minus_64,
    int *frame_minus_88_live,
    const char *native_text_buffer) {
    const BattleFightRedrawRouteV7 redraw_route =
        battle_fight_redraw_pair_v7(
            frame_plus_1c, direction_snapshot_at_b308,
            frame_minus_2c_x, frame_minus_28_y,
            frame_plus_10_x, frame_plus_14_y);
    if (redraw_route == BATTLE_FIGHT_REDRAW_ROUTE_B639_V7)
        return 0;

    battle_fight_post_redraw_notifications_v4(
        direction_snapshot_at_b308, attacker_faction, defender_faction,
        frame_minus_18, frame_minus_30, frame_minus_38, frame_minus_3c,
        frame_minus_40, frame_minus_58, frame_minus_64,
        *frame_minus_88_live, native_text_buffer);
    return 1;
}

static int battle_fight_late_finish_b2fa_live_v1(
    BattleFightLateFrameV1 &frame,
    int direction_snapshot_at_b308,
    int frame_minus_78_interlude_gate) {
    (void)battle_fight_b2fa_to_b63f_live_v1(
        *frame.frame_plus_1c, direction_snapshot_at_b308,
        frame.attacker_tile_x_minus_2c, frame.attacker_tile_y_minus_28,
        frame.target_x, frame.target_y,
        frame.attacker_faction, frame.defender_faction,
        *frame.frame_minus_18,
        *frame.frame_minus_30, *frame.frame_minus_38,
        *frame.frame_minus_3c,
        *frame.frame_minus_40,
        *frame.frame_minus_58,
        *frame.frame_minus_64_slot,
        frame.status_animation_flag_minus_88,
        frame.native_text_buffer_minus_138);

    battle_fight_emit_selector_notice_v1(
        *frame.frame_minus_44,
        frame.attacker_faction, frame.defender_faction,
        "VENDETTAREPORT", "PACTATTACKED", "PACTATTACKING");

    return battle_fight_final_block_v5(
        *frame.direction_slot_plus_0c,
        frame.target_x, frame.target_y,
        frame.frame_minus_90_proto, frame.frame_minus_94_proto,
        frame.attacker_faction, frame.defender_faction,
        frame_minus_78_interlude_gate,
        *frame.frame_minus_20);
}

static int battle_fight_after_a44d_compose_v3(
    BattleFightLateFrameV1 &frame,
    int frame_minus_78_interlude_gate) {
    const BattleFightOutcomeJoinV4 join = battle_fight_outcome_join_live_v1(
        frame.frame_minus_50,
        *frame.frame_minus_64_slot,
        *frame.direction_slot_plus_0c,
        *frame.source_mode_slot_plus_18,
        *frame.frame_minus_38,
        *frame.frame_minus_30,
        *frame.frame_minus_58,
        *frame.attacker_record_minus_04,
        *frame.defender_record_minus_08,
        frame.attacker_faction,
        frame.defender_faction,
        frame.target_x,
        frame.target_y);

    if (join == BATTLE_FIGHT_OUTCOME_JOIN_A730_V4) {
        frame.continuation_edi_at_a730 = frame.defender_record_minus_08;
        battle_fight_late_capture_to_b2fa_v1(frame);
    } else if (join == BATTLE_FIGHT_OUTCOME_JOIN_AF13_V4) {
        battle_fight_af13_to_b2fa_live_v1(
            frame.attacker_vehicle_id,
            *frame.direction_slot_plus_0c,
            *frame.frame_plus_08_loop_slot,
            *frame.attacker_record_minus_04,
            *frame.defender_record_minus_08,
            frame.attacker_faction,
            frame.defender_faction,
            frame.target_x,
            frame.target_y,
            frame.frame_minus_38,
            frame.frame_minus_30,
            frame.frame_minus_58,
            *frame.frame_minus_40);
    } else {
        battle_fight_b203_score_and_neighbor_refresh_v4(
            *frame.direction_slot_plus_0c,
            *frame.frame_plus_08_loop_slot,
            frame.attacker_faction,
            frame.target_x,
            frame.target_y,
            *frame.frame_minus_58,
            *frame.frame_minus_40);
    }

    // Native loads the live direction at B308 after route callbacks; keep this
    // read adjacent to the redraw continuation rather than snapshotting earlier.
    const int direction_snapshot_at_b308 = *frame.direction_slot_plus_0c;
    return battle_fight_late_finish_b2fa_live_v1(
        frame, direction_snapshot_at_b308,
        frame_minus_78_interlude_gate);
}

// The preceding private fragments supply the translated round, outcome and
// report owners. Their real typed APIs are bound below; no fixed-address
// executable calls are introduced by the enclosing function.
static void battle_fight_round_status_draw_v1(
    int attacker_id, int defender_id, int attack_value, int defense_value,
    int source_mode) {
    ConsoleStatusWin.draw(attacker_id, defender_id, attack_value,
                          defense_value, source_mode);
}

static void battle_fight_round_boom_v1(int x, int y, int effect) {
    (void)boom(x, y, effect);
}

static void battle_fight_round_redraw_tile_v1(int x, int y, int mode) {
    draw_tile(x, y, mode);
}

// Native `draw_tile_fixup` is a typed source function declared by mapwin.h,
// not an executable trampoline.
static void battle_fight_round_redraw_fixup_v1(
    int x, int y, int flags, int mode) {
    draw_tile_fixup(x, y, flags, mode);
}

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
                         int source_mode, int allow_interceptor,
                         int *odds_out) {
    const int incoming_source_mode = source_mode;
    int &combat_type = source_mode; // Native mutable [ebp+0x18].
    const int source_x = Vehs[veh_id].x;
    const int source_y = Vehs[veh_id].y;
    const Veh &initial_attacker = Vehs[veh_id];
    const int attacker_proto = initial_attacker.proto_id;
    const int attacker_chassis = VehPrototypes[attacker_proto].chassis_id;
    const int attacker_faction = initial_attacker.faction_id;

    // Native 506B18..6B6C tests the incoming value, then writes the selected
    // combat mode back to the same +0x18 argument slot.
    if (incoming_source_mode == 0 &&
        (can_arty(attacker_proto, 1) == 0 ||
         Chassis[attacker_chassis].triad != 0)) {
        combat_type = 0;
    } else {
        combat_type = 1;
    }

    const int target_vehicle = veh_at(target_x, target_y);
    if (can_arty(attacker_proto, 1) != 0 &&
        Chassis[attacker_chassis].triad == 1) {
        const int high_terrain = map_loc(target_x, target_y)->climate & 0xE0;
        if (high_terrain >= 0x60) {
            const int eligible = stack_check(target_vehicle, 3, 2, -1, -1);
            const int all = stack_check(target_vehicle, 1, -1, -1, -1);
            const Map *tile_after_checks = map_loc(target_x, target_y);
            if (eligible < all ||
                ((tile_after_checks->bit & 1u) != 0 &&
                 (tile_after_checks->val2 & 0x0Fu) < 8)) {
                combat_type = 1;
            }
        }
    }

    const int target_stack = stack_fix(target_vehicle);
    const int target_stack_proto = Vehs[target_stack].proto_id;
    const int target_stack_chassis =
        VehPrototypes[target_stack_proto].chassis_id;
    const int target_stack_faction = Vehs[target_stack].faction_id;
    int interceptor_result = 0; // Native [-0x34], later candidate-Y scratch.
    if (Chassis[target_stack_chassis].triad == 2 &&
        Chassis[target_stack_chassis].missile == 0 &&
        stack_check(target_stack, 6, ABL_AIR_SUPERIORITY, -1, -1) == 0 &&
        allow_interceptor != 0 &&
        (NetDaemonNet.channels_[0].leading_words_[3] & 2u) != 0) {
        interceptor_result = interceptor(target_stack_faction,
                                         attacker_faction,
                                         target_x, target_y);
    }

    int defender_id = static_cast<int>(
        best_defender(target_stack, veh_id, combat_type));
    int current_stack_head = defender_id; // Native [-0x18], same initial ID.
    int native_minus40 = defender_id;     // Distinct live native [-0x40].
    const int defender_proto = Vehs[defender_id].proto_id;
    const int defender_chassis = VehPrototypes[defender_proto].chassis_id;
    const int defender_faction = Vehs[defender_id].faction_id;
    int target_base_id = base_at(target_x, target_y); // Native [-0x20].
    const int attacker_proto_after_interceptor = Vehs[veh_id].proto_id;
    const int attacker_chassis_after_interceptor =
        VehPrototypes[attacker_proto_after_interceptor].chassis_id;

    if (can_arty(attacker_proto_after_interceptor, 1) != 0 &&
        can_arty(defender_proto, 1) != 0 &&
        (Chassis[attacker_chassis_after_interceptor].triad == 0 ||
         combat_type != 0 ||
         (map_loc(target_x, target_y)->climate & 0xE0) >= 0x60)) {
        combat_type |= 2;
    }
    if (has_abil(defender_proto, ABL_AIR_SUPERIORITY) != 0 &&
        Chassis[attacker_chassis_after_interceptor].triad == 2 &&
        Chassis[attacker_chassis_after_interceptor].missile == 0 &&
        Chassis[defender_chassis].missile == 0 &&
        Chassis[defender_chassis].triad == 2) {
        combat_type |= 0x1A;
    }

    int movement_charge;
    if ((combat_type & 3) != 0 && combat_type < 4) {
        movement_charge = speed(veh_id, 0) - Vehs[veh_id].moves_expended;
        if (movement_charge < 0) movement_charge = 0;
        if (movement_charge > 999) movement_charge = 999;
    } else {
        movement_charge = Rules.move_rate_roads;
    }

    int attacker_combat = 0; // Native mutable local [-0x14].
    int defender_combat = 0; // Native mutable local [-0x1c].
    battle_compute(veh_id, defender_id, &attacker_combat, &defender_combat,
                   combat_type);
    const int attacker_battle_proto = Vehs[veh_id].proto_id;
    const VehPrototype &attacker_definition =
        VehPrototypes[attacker_battle_proto];
    const VehPrototype &defender_definition = VehPrototypes[defender_proto];
    int defender_hit_step;
    int attacker_hit_step;
    if (Weapon[attacker_definition.weapon_id].offense_rating >= 0 &&
        Armor[defender_definition.armor_id].defense_rating >= 0) {
        defender_hit_step = 1;
        attacker_hit_step = 1;
    } else {
        defender_hit_step = defender_definition.reactor_id;
        attacker_hit_step = attacker_definition.reactor_id;
    }
    int attacker_remaining = attacker_definition.plan == 0x0C
        ? 1 : range(attacker_definition.reactor_id, 1, 100) * 10;
    attacker_remaining -= initial_attacker.dmg_incurred;
    if (attacker_remaining < 0) attacker_remaining = 0;
    if (attacker_remaining > 9999) attacker_remaining = 9999;
    int defender_remaining = defender_definition.plan == 0x0C
        ? 1 : range(defender_definition.reactor_id, 1, 100) * 10;
    defender_remaining -= Vehs[defender_id].dmg_incurred;
    if (defender_remaining < 0) defender_remaining = 0;
    if (defender_remaining > 9999) defender_remaining = 9999;

    int defender_psi_factor = 1;
    int attacker_psi_factor = 1;
    if (Weapon[attacker_definition.weapon_id].offense_rating < 0 ||
        Armor[defender_definition.armor_id].defense_rating < 0) {
        defender_psi_factor = defender_definition.reactor_id;
        attacker_psi_factor = attacker_definition.reactor_id;
    }
    if (allow_interceptor == 0) {
        const int attacker_reactor_factor = attacker_definition.plan == 0x0C
            ? 1 : range(attacker_definition.reactor_id, 1, 100) * 10;
        const int defender_reactor_factor = defender_definition.plan == 0x0C
            ? 1 : range(defender_definition.reactor_id, 1, 100) * 10;
        union SignedProduct {
            unsigned int bits;
            int value;
        } attack_first, attack_second, defense_first, defense_second,
          odds_numerator, odds_denominator;
        attack_first.bits = static_cast<unsigned int>(attacker_remaining) *
                            static_cast<unsigned int>(defender_psi_factor);
        attack_second.bits = static_cast<unsigned int>(attack_first.value) *
                             static_cast<unsigned int>(attacker_combat);
        const int attack_ratio = attack_second.value / attacker_reactor_factor;
        defense_first.bits = static_cast<unsigned int>(defender_remaining) *
                             static_cast<unsigned int>(attacker_psi_factor);
        defense_second.bits = static_cast<unsigned int>(defense_first.value) *
                              static_cast<unsigned int>(defender_combat);
        const int defense_ratio = defense_second.value / defender_reactor_factor;
        if (odds_out != 0) *odds_out = defender_id;
        odds_numerator.bits = static_cast<unsigned int>(
            (combat_type & 1) != 0 ? 4 : 8) *
            static_cast<unsigned int>(attack_ratio);
        odds_denominator.bits = static_cast<unsigned int>(defense_ratio) + 1u;
        return odds_numerator.value / odds_denominator.value;
    }

    const unsigned int attacker_lock_bit =
        1u << (static_cast<unsigned int>(attacker_faction) & 0x1Fu);
    if ((static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) &
         static_cast<unsigned char>(attacker_lock_bit)) != 0 &&
        attacker_definition.plan != 0x0B &&
        (defender_definition.plan != 0x0C || target_base_id >= 0) &&
        (NetDaemonNet.channels_[0].leading_words_[3] & 1u) != 0 &&
        (IsMultiplayerNet == 0 || interceptor_result == 0) &&
        break_treaty(attacker_faction, defender_faction, 0x0B) != 0) {
        return 2;
    }

    int nerve_gas_active = 0;
    battle_fight_nerve_gas_precombat_v4(
        veh_id, defender_id, attacker_faction, defender_faction,
        allow_interceptor, nerve_gas_active);
    if (battle_fight_precombat_commit_v2(
            veh_id, defender_id, attacker_faction, target_base_id,
            movement_charge, combat_type, attacker_combat) == 0) {
        return 0;
    }

    int diplomacy_report_mode = 0;
    battle_fight_precombat_diplomacy_v4(
        veh_id, defender_id, attacker_faction, defender_faction,
        diplomacy_report_mode);
    battle_fight_territory_pact_sweep_v2(
        attacker_faction, defender_faction, target_x, target_y);

    int native_minus84 = 0;
    int attacker_strength_product = 0;
    int defender_strength_product = 0;
    int attacker_proto_snapshot = 0;
    int defender_proto_snapshot = 0;
    const int prompt_result = battle_fight_pre_prompt_residual_v2(
        veh_id, defender_id, attacker_faction, defender_faction,
        target_x, target_y, combat_type, target_base_id, combat_type,
        interceptor_result, movement_charge, attacker_combat, defender_combat,
        attacker_remaining, defender_psi_factor, defender_remaining,
        attacker_psi_factor, nerve_gas_active, native_minus84,
        attacker_strength_product, defender_strength_product,
        &attacker_proto_snapshot, &defender_proto_snapshot);
    if (prompt_result == 1) return 1;
    if (prompt_result == 0) {
        battle_fight_reduce_strength_pair_v5(attacker_strength_product,
                                             defender_strength_product);
        if (battle_fight_strength_prompt_residual_v2(
                veh_id, defender_id, movement_charge,
                attacker_strength_product, defender_strength_product) != 0) {
            return 1;
        }
    }

    char base_name_copy[80];
    char native_text_minus138[0xA0]; // Provisional collision-boundary model.
    int status_animation_flag = 0; // Native [-0x88], read after callbacks.
    int &report_selector = allow_interceptor; // Same live native +0x1c slot.
    int native_minus78 = 0;
    const BattleFightPostPromptRouteV1 post_prompt_route =
        battle_fight_normal_post_prompt_v1(
            veh_id, defender_id, attacker_faction, defender_faction,
            nerve_gas_active, target_base_id, base_name_copy,
            native_text_minus138, combat_type, report_selector,
            native_minus78, status_animation_flag);
    if (post_prompt_route == BATTLE_FIGHT_POST_PROMPT_SOURCE_MODE_V1) {
        return battle_fight_source_mode(
            veh_id, defender_id, attacker_faction, defender_faction,
            target_x, target_y, source_x, source_y, target_base_id);
    }

    // Continuation locals are deliberately single storage objects across the
    // round, threshold, kill, and presentation paths. Later stages only get
    // pointers/references to these values; no source-coordinate aliases are
    // made from [-0x30] or [-0x34].
    int native_minus24 = 0;
    int native_minus30 = 0;
    int native_minus34 = interceptor_result;
    int native_minus38 = 0;
    int native_minus3c = nerve_gas_active;
    int native_minus44 = diplomacy_report_mode;
    int native_minus48 = 0;
    int native_minus50 = 0;
    int native_minus54 = 0;
    int native_minus58 = 0;
    int native_minus64 = 0;
    int native_minus68 = defender_hit_step;
    int native_minus70 = attacker_hit_step;
    int native_minus6c = 0;
    int native_minus74 = 0;
    int native_minus80 = 0;
    int native_minus8c = 0;
    int native_plus20_scratch = 0; // Separate int after odds-only return.

    BattleFightRoundFrameV4 round_frame(
        dir, combat_type, report_selector, current_stack_head);
    round_frame.attacker_id = veh_id;
    round_frame.defender_id = defender_id;
    round_frame.retreating_faction = defender_faction;
    round_frame.attacker_combat_value = attacker_combat;
    round_frame.defender_combat_value = defender_combat;
    round_frame.round_or_candidate_index = native_minus24;
    round_frame.coordinate_y_minus_28 = source_y;
    round_frame.coordinate_x_minus_2c = source_x;
    round_frame.attacker_x = 0; // Unused legacy field; not native [-0x30].
    round_frame.attacker_y = 0; // Unused legacy field; not native [-0x34].
    round_frame.post_hit_handoff_gate = native_minus48;
    round_frame.attacker_faction_local_50 = native_minus50;
    round_frame.x = target_x;
    round_frame.y = target_y;
    round_frame.defender_hit_step = native_minus68;
    round_frame.attacker_hit_step = native_minus70;
    round_frame.display_scale = native_minus6c;
    round_frame.best_direction = native_minus74;
    round_frame.marine_detach_budget = native_minus80;
    round_frame.best_direction_score = native_minus8c;
    round_frame.transition_type = native_minus54;
    round_frame.candidate_x_plus_20 = native_plus20_scratch;
    round_frame.candidate_y_minus_34 = native_minus34;

    BattleFightRoundBindingsV4 round_bindings;
    round_bindings.current_faction = battle_fight_pholus_project_base_v3;
    round_bindings.status_draw = battle_fight_round_status_draw_v1;
    round_bindings.boom = battle_fight_round_boom_v1;
    round_bindings.redraw_tile = battle_fight_round_redraw_tile_v1;
    round_bindings.redraw_tile_fixup = battle_fight_round_redraw_fixup_v1;

    BattleFightMidNativeSlotsV3 mid_slots;
    mid_slots.target_x = target_x;
    mid_slots.target_y = target_y;
    mid_slots.target_base_id_minus_20 = target_base_id;
    mid_slots.anchor_x_minus_2c = source_x;
    mid_slots.anchor_y_minus_28 = source_y;
    mid_slots.candidate_x_plus_20 = &native_plus20_scratch;
    mid_slots.candidate_y_minus_34 = &native_minus34;
    mid_slots.second_argument_slot_plus_0c = &dir;
    mid_slots.attacker_faction_slot_minus_50 =
        &round_frame.attacker_faction_local_50;
    mid_slots.display_latch_minus_64 = &native_minus64;
    mid_slots.outcome_gate_minus_48 = &round_frame.post_hit_handoff_gate;
    mid_slots.branch_selector_minus_54 = &round_frame.transition_type;
    mid_slots.odds_gate_minus_78 = &native_minus78;
    mid_slots.display_selector_slot_plus_1c = &report_selector;
    mid_slots.stack_vehicle_slot_minus_18 = &current_stack_head;
    mid_slots.status_value_minus_14 = attacker_combat;
    mid_slots.status_value_minus_1c = defender_combat;
    mid_slots.source_mode_slot_plus_18 = &combat_type;
    mid_slots.attacker_faction_minus_10 = attacker_faction;
    mid_slots.defender_faction_minus_0c = defender_faction;
    mid_slots.stats_proto_minus_18 = current_stack_head;
    mid_slots.cached_attacker_hp_minus_60 = attacker_remaining;
    mid_slots.cached_defender_hp_minus_5c = defender_remaining;
    mid_slots.strength_14 = &attacker_combat;
    mid_slots.strength_1c = &defender_combat;
    mid_slots.frame_plus_20 = &native_plus20_scratch;

    BattleFightMidPreRoundInputsV7 pre_round_inputs;
    pre_round_inputs.source_x = source_x;
    pre_round_inputs.source_y = source_y;
    pre_round_inputs.combat_type = combat_type;
    pre_round_inputs.status_animation_flag_minus_88 =
        &status_animation_flag;

    const BattleFightMidOwnerExitV7 mid_exit = battle_fight_mid_owner_v7(
        round_frame, round_bindings, mid_slots, pre_round_inputs);
    if (mid_exit == BATTLE_FIGHT_MID_EARLY_RETURN_ZERO_V7 ||
        mid_exit == BATTLE_FIGHT_MID_RETURN_AT_A110_V7) {
        return 0;
    }

    BattleFightLateFrameV1 late_frame;
    late_frame.frame_minus_48 = round_frame.post_hit_handoff_gate;
    late_frame.frame_minus_50 = round_frame.attacker_faction_local_50;
    late_frame.status_animation_flag_minus_88 = &status_animation_flag;
    late_frame.frame_minus_90_proto = attacker_proto_snapshot;
    late_frame.frame_minus_94_proto = defender_proto_snapshot;
    late_frame.attacker_vehicle_id = veh_id;
    late_frame.defender_vehicle_id = defender_id;
    late_frame.frame_minus_18_vehicle_id = current_stack_head;
    late_frame.frame_minus_8_vehicle_id = defender_id;
    late_frame.frame_vehicle_id_at_a904 = defender_id;
    late_frame.frame_minus_10 = attacker_faction;
    late_frame.attacker_faction = attacker_faction;
    late_frame.defender_faction = defender_faction;
    late_frame.target_x = target_x;
    late_frame.target_y = target_y;
    late_frame.attacker_tile_x_minus_2c = source_x;
    late_frame.attacker_tile_y_minus_28 = source_y;
    late_frame.target_stack_id = -1; // AC01 reacquires before its use.
    late_frame.target_base_id = target_base_id;
    late_frame.stats_proto_minus_18 = current_stack_head;
    late_frame.cached_attacker_hp_minus_60 = attacker_remaining;
    late_frame.cached_defender_hp_minus_5c = defender_remaining;
    late_frame.native_esi_before_a80e_test = attacker_faction;
    late_frame.native_text_buffer_minus_138 = native_text_minus138;
    late_frame.direction_slot_plus_0c = &dir;
    late_frame.source_mode_slot_plus_18 = &combat_type;
    late_frame.frame_plus_08_loop_slot = &veh_id;
    late_frame.frame_plus_1c = &report_selector;
    late_frame.frame_plus_20 = &native_plus20_scratch;
    late_frame.frame_minus_20 = &target_base_id;
    late_frame.frame_minus_18 = &current_stack_head;
    late_frame.frame_minus_38 = &native_minus38;
    late_frame.frame_minus_30 = &native_minus30;
    late_frame.frame_minus_3c = &native_minus3c;
    late_frame.frame_minus_40 = &native_minus40;
    late_frame.frame_minus_44 = &native_minus44;
    late_frame.frame_minus_58 = &native_minus58;
    late_frame.frame_minus_64_slot = &native_minus64;
    late_frame.base_name_copy_80 = base_name_copy;
    late_frame.attacker_record_minus_04 = &Vehs[veh_id];
    late_frame.defender_record_minus_08 = &Vehs[defender_id];
    late_frame.continuation_edi_at_a730 = &Vehs[defender_id];
    return battle_fight_after_a44d_compose_v3(late_frame, native_minus78);
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

