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
#include "faction.h"
#include "general.h"
#include "base.h"
#include "console.h"
#include "game.h"
#include "map.h"
#include "mapwin.h"
#include "veh.h"
#include "xpops.h"

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0050BA50 ?intervention@@YAXHH@Z 0x0050BA50-0x0050BCB6
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/0050ba50.cpp retains original identity/spans and contradictory scaffold history.", "Pinned full-span review and genuine Player/PlayerData/Base/Veh/Map declarations own the field interpretation; Popup uses the real C++ lifecycle.", "Related intervention50BA50/pact_withdraw53C370 bodies own the coupled helper behavior. Shared proposal cell93FAB0 and map outcome93FAD8 have native producer/consumer evidence; seed-only workflow producers remain explicit obligations."], "reason": "Recover this original diplomacy identity in its genuine faction owner, preserving typed game records, shared-state lifetimes and every seed obligation. This binding is not a byte or runtime claim."}
void __cdecl intervention(int target_faction, int actor_faction) {
    for (int candidate = 1; candidate < MaxPlayerNum; ++candidate) {
        if (candidate == target_faction || candidate == actor_faction) continue;

        // LockEnableMask byte +1 (native 0x009A64E9) is the faction-alive mask.
        if ((GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_ & (1 << candidate)) == 0) continue;
        if ((PlayersData[candidate].diplo_treaties[target_faction] & 1u) == 0) continue;
        if ((PlayersData[candidate].diplo_treaties[actor_faction] & 0x11u) != 0) continue;

        GenderDefault = Players[candidate].noun_gender;
        PluralityDefault = Players[candidate].is_noun_plural;
        parse_says(0, Players[candidate].noun_faction, -1, -1);

        GenderDefault = Players[target_faction].noun_gender;
        PluralityDefault = Players[target_faction].is_noun_plural;
        parse_says(1, Players[target_faction].noun_faction, -1, -1);

        GenderDefault = Players[candidate].is_leader_female;
        PluralityDefault = FALSE;
        parse_says(2, Players[candidate].title_leader, -1, -1);

        GenderDefault = Players[candidate].is_leader_female;
        PluralityDefault = FALSE;
        parse_says(3, Players[candidate].name_leader, -1, -1);

        GenderDefault = Players[actor_faction].is_leader_female;
        PluralityDefault = FALSE;
        parse_says(4, Players[actor_faction].title_leader, -1, -1);

        GenderDefault = Players[actor_faction].is_leader_female;
        PluralityDefault = FALSE;
        parse_says(5, Players[actor_faction].name_leader, -1, -1);

        GenderDefault = Players[actor_faction].noun_gender;
        PluralityDefault = Players[actor_faction].is_noun_plural;
        parse_says(6, Players[actor_faction].noun_faction, -1, -1);

        if (((1 << actor_faction) & static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_)) != 0 ||
            (static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) & (1 << target_faction)) != 0) {
            if (((1 << actor_faction) & static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_)) != 0) {
                PlayersData[actor_faction].diplo_spoke[candidate] = GameDataStateStorage.fields_.turn_current_num_;
            }
            treaty_on(candidate, actor_faction, 0x80000018);
            set_treaty(candidate, actor_faction, 0x40, TRUE);
        }

        if (actor_faction == ConsoleGlobal.faction_id_) {
            X_pop("AIDSTHEM", 0);
        } else if (target_faction == ConsoleGlobal.faction_id_) {
            X_pop("AIDSUS", 0);
        }
    }
}

// ORIGINAL: 0x00548F80 ?do_withdrawal@@YAHHH@Z 0x00548F80-0x00549264
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/00548f80.cpp retains original identity/spans and contradictory scaffold history.", "Pinned full-span review and genuine Player/PlayerData/Base/Veh/Map declarations own the field interpretation; Popup uses the real C++ lifecycle.", "Related intervention50BA50/pact_withdraw53C370 bodies own the coupled helper behavior. Shared proposal cell93FAB0 and map outcome93FAD8 have native producer/consumer evidence; seed-only workflow producers remain explicit obligations."], "reason": "Recover this original diplomacy identity in its genuine faction owner, preserving typed game records, shared-state lifetimes and every seed obligation. This binding is not a byte or runtime claim."}
// TRIED: MISMATCH #6 'push' vs 'mov'. Tried both `if (n>0){...}` and the inverted `if (n<=0) return 0;` early-out for the outer guard; neither reproduces the original's push-esi-before-branch / xor+cmp zero-reuse pattern. veh_lift/veh_drop's second push (used as the leftover eax arg to the paired veh_drop call) is reproduced by calling veh_lift(i); veh_drop(i, x, y) as two independent statements rather than chaining a return value, since veh_lift is declared void in this unit.
// The image tests the flat-map bit first and wraps on the fall-through.
static inline int withdrawal_wrap_x(int x) {
    if (MapIsFlat & 1) return x;
    const int longitude_bounds = MapLongitudeBounds;
    if (x < 0) return x + longitude_bounds;
    if (x >= longitude_bounds) return x - longitude_bounds;
    return x;
}

int __cdecl do_withdrawal(int faction_id, int other_faction_id) {
    int result = 0;
    for (int veh_id = 0; veh_id < GameDataStateStorage.fields_.vehicle_current_count_; ++veh_id) {
        if (Vehs[veh_id].faction_id != other_faction_id) continue;
        const int proto_id = Vehs[veh_id].proto_id;
        if (VehPrototypes[proto_id].armor_id == 0x0B) continue;

        const int x = Vehs[veh_id].x;
        const int y = Vehs[veh_id].y;
        Map &tile = MapTiles[MapLongitude * y + (x >> 1)];
        if ((tile.climate & ~0x1F) < ALT_BIT_SHORE_LINE) continue;
        if (tile.territory != faction_id) continue;

        if (Chassis[VehPrototypes[proto_id].chassis_id].triad == TRIAD_LAND &&
            veh_cargo(veh_id) != 0) {
            for (int neighbor = 0; neighbor < 8; ++neighbor) {
                const int dest_x = withdrawal_wrap_x(VehMoveDeltaX[neighbor] + x);
                const int dest_y = VehMoveDeltaY[neighbor] + y;
                if (!on_map(dest_x, dest_y)) continue;

                Map &dest = MapTiles[MapLongitude * dest_y + (dest_x >> 1)];
                if ((dest.climate & ~0x1F) >= ALT_BIT_SHORE_LINE) {
                    if (dest.territory != other_faction_id) continue;
                } else {
                    const int occupant = stack_fix(veh_at(dest_x, dest_y));
                    if (stack_check(occupant, 8, other_faction_id, -1, -1) <= 0)
                        continue;
                }

                Map &live_dest = MapTiles[MapLongitude * dest_y + (dest_x >> 1)];
                if ((live_dest.bit & 3) != 0) {
                    const int owner = live_dest.val2 & 0x0F;
                    if (owner < 8 && owner >= 0 && owner != other_faction_id) continue;
                }

                veh_drop(veh_lift(veh_id), dest_x, dest_y);
                Vehs[veh_id].visibility = 0;
                Vehs[veh_id].order = ORDER_NONE;
                Vehs[veh_id].state &= 0xFCFFBDFF;
                draw_tiles(x, y, 2);
                ++result;
            }
        }

        const int base_id = base_find(x, y, other_faction_id);
        if (base_id >= 0) {
            veh_drop(veh_lift(veh_id), Bases[base_id].x, Bases[base_id].y);
            Vehs[veh_id].visibility = 0;
            Vehs[veh_id].order = ORDER_NONE;
            Vehs[veh_id].state &= 0xFCFFBDFF;
            draw_tile_fixup(x, y, has_abil(Vehs[veh_id].proto_id, 2), 2);
            ++result;
        }
    }

    if (result != 0) {
        set_agenda(other_faction_id, faction_id, 2, 1);
        set_treaty(other_faction_id, faction_id, 0x100000, 1);
    }
    return result;
}

/*
Purpose: Clear `treaty` in the diplomatic status word between two factions, both
         directions of the pair, and take the side effects the clear implies:
         clearing any of the three low bits also clears 0x1100000, clearing
         VENDETTA also clears 0x8040, and clearing WANT_REVENGE also clears
         SHALL_BETRAY in the +8 view of the same table.
// ORIGINAL: 0x0055B760 ?treaty_off@@YAXHHH@Z 0x0055B760-0x0055B818 BYTE_EXACT
// LEVER: PROMOTED out of src/unrecovered/0055b760.cpp, whose claim proved the
//         ARTIFACT reproduced while the shipped program contained nothing - the
//         same promotion set_state_id made beside CheckBox::get_state_id. The
//         artifact's two raw table bindings are SpyingStatusTable and the same
//         table eight uint32_t elements further in, which is
//         SpyingStatusTable[index + 8] - the view do_check reads for its
//         0x80000000-id boxes, and the fold lands on the same displacement.
// size      184 bytes
// prototype void (__cdecl ?treaty_off@@YAXHHH@Z)(int faction_id, int faction_id_with, int treaty)
// callers   14   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl treaty_off(int faction_id, int faction_id_with, int treaty) {
    if (treaty & 7) {
        treaty |= 0x1100000;
    }
    if (treaty & DTREATY_VENDETTA) {
        treaty |= 0x8040;
    }
    if (treaty & DTREATY_WANT_REVENGE) {
        // 0xfffff3ff == ~(SHALL_BETRAY | 0x800), and in the +8 view: dropping
        // WANT_REVENGE drops that pair as well.
        int index = faction_id_with;
        PlayersData[faction_id].diplo_agenda[index] &= 0xfffff3ff;
        index = faction_id;
        PlayersData[faction_id_with].diplo_agenda[index] &= 0xfffff3ff;
    }
    int index = faction_id_with;
    PlayersData[faction_id].diplo_treaties[index] &= ~treaty;
    index = faction_id;
    PlayersData[faction_id_with].diplo_treaties[index] &= ~treaty;
}
