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
#include "temp.h"
#include "game.h"
#include "alpha.h"
#include "base.h"
#include "faction.h"
#include "general.h"
#include "map.h"

BOOL *ExpansionEnabled = (BOOL *)0x009A6488;
uint32_t *GamePreferences = (uint32_t *)0x009A6490;
uint32_t *GameMorePreferences = (uint32_t *)0x009A6494;
uint32_t *GameWarnings = (uint32_t *)0x009A6498;
uint32_t *GameRules = (uint32_t *)0x009A649C;
uint32_t *GameState = (uint32_t *)0x009A64C0;
int *DiffLevelCurrent = (int *)0x009A64C4;
int *TurnCurrentNum = (int *)0x009A64D4;
uint32_t *ObjectiveReqVictory = (uint32_t *)0x0094B4C0;
uint32_t *ObjectivesSuddenDeathVictory = (uint32_t *)0x0094B4C4;
uint32_t *ObjectiveAchievePts = (uint32_t *)0x0094B4C8;
uint32_t *VictoryAchieveBonusPts = (uint32_t *)0x0094B4CC;
uint32_t *MissionYearCurrent = (uint32_t *)0x009A64D8;
uint32_t *StartingMissionYear = (uint32_t *)0x0094B4D0;
uint32_t *EndingMissionYear = (uint32_t *)0x0094B4D4;
uint32_t *TectonicDetonationCount = (uint32_t *)0x00946138; // [8]; value for each faction
int *SunspotDuration = (int *)0x009A6800;
uint32_t *MountPlanetX = (uint32_t *)0x009A6804;
uint32_t *MountPlanetY = (uint32_t *)0x009A6808;
int *DustCloudDuration = (int *)0x009A680C;
BOOL *IsMultiplayerNet = (BOOL *)0x0093F660; // DirectPlay - Serial, Modem, Internet (TCP/IP)
BOOL *IsMultiplayerPBEM = (BOOL *)0x0093A95C; // HotSeat / PBEM
uint8_t *NetTurnFlags = (uint8_t *)0x009A681C;
int *NetTurnFaction = (int *)0x009A6820;

/*
Purpose: Wrap an x coordinate the way reset_territory() wraps it.
Original Offset: n/a
Return Value: Wrapped x
Status: Complete

xrange() at 0048BEE0 cannot stand in, for one bit. It tests the whole of
MapIsFlat; reset_territory reads the low BYTE and tests bit zero
(`mov cl, byte ptr [94988Ch] / test cl, 1`). The two agree on the 0 and 1 the
game stores there and disagree on everything else, and the difference is in the
original rather than in the transcription. world_site() at 005C4FD0 carries the
same distinction as map.cpp's site_xrange().
*/
static int territory_xrange(int x) {
    if (!(*MapIsFlat & 1)) {
        if (x >= 0) {
            if (x >= *MapLongitudeBounds) {
                x -= *MapLongitudeBounds;
            }
        } else {
            x += *MapLongitudeBounds;
        }
    }
    return x;
}

/*
Purpose: Recompute which faction owns each tile of the map, and rebuild every
         per-faction and per-region tally that ownership feeds.
Original Offset: 00523DD0
Return Value: n/a
Status: Complete

Ownership is decided by proximity: the nearest base within
Rules->territory_max_dist_base owns the tile, and an OCEAN tile halves that
distance and is then overruled outright by any base in its own eight-tile
ring - which is how coastal water ends up belonging to the base beside it
rather than to a larger one further off. A tile with no base in range is
unclaimed, which the map stores as -1.

Everything else here is tallies. Six per-region arrays are zeroed for all eight
factions before the walk, and the walk refills them:

  - unk_79[region] counts tiles the faction can see in that region - or, for a
    faction whose AI is running the objectives strategy, tiles carrying
    BIT_SUPPLY_REMOVE instead. That substitution is what PFLAG_STRAT_SEARCH_-
    OBJECTIVES selects, and it is a substitution rather than an addition.
  - unk_78[region] counts tiles the faction owns; unclaimed tiles count for
    faction zero.
  - unk_80[region] counts owned tiles that are wet, not rocky and not fungus,
    and unk_81[region] the subset of those already inside a base radius.
  - unk_82[region] is the best world_site score the faction owns there, and
    unk_83[region] the supply pods on its tiles.

Continents[region] gets the same last two facts for the region as a whole, in
unk_3 and pods, which is what those two field comments already say.

The walk is the same shape as num_objectives': one Map entry per tile visited,
never reset between rows, starting each row at `x = y & 1`. An empty row - one
where the start is already past MapLongitudeBounds - does not advance the
pointer at all.

A tile inside a base radius is not scored as a site and is not asked to be: its
score is taken as zero without reading val2. Elsewhere the stored score is used
if there is one and world_site() is called and the answer cached with
site_set() if there is not, so a full map walk costs at most one scoring pass
per tile ever.

The supply-pod arm ends in a site request, and its two branches are not
symmetric. An owned tile asks only its owner; an unclaimed one asks every
faction that has a base in the region, and each of those must additionally not
already hold a site here and must have a base at between one and two times the
territory distance - near enough to want it, far enough not to have it already.
Both branches require the faction either to see the tile or to be an AI above
difficulty 3, which is the standard "the AI is allowed to know" test.
*/
void __cdecl reset_territory() {
    for (int faction_id = 0; faction_id < MaxPlayerNum; faction_id++) {
        PlayerData &player = PlayersData[faction_id];
        for (int region = 0; region < MaxContinentNum; region++) {
            player.unk_80[region] = 0;
            player.unk_78[region] = 0;
            player.unk_79[region] = 0;
            player.unk_81[region] = 0;
            player.unk_83[region] = 0;
            player.unk_82[region] = 0;
        }
    }
    for (int region = 0; region < MaxContinentNum; region++) {
        Continents[region].pods = 0;
        Continents[region].unk_3 = 0;
    }
    Map *tile = *MapTiles;
    for (int y = 0; y < *MapLatitudeBounds; y++) {
        for (int x = y & 1; x < *MapLongitudeBounds; x += 2, tile++) {
            int region = tile->region;
            for (int faction_id = 1; faction_id < MaxPlayerNum; faction_id++) {
                uint32_t flags = PlayersData[faction_id].flags;
                if (flags & PFLAG_STRAT_SEARCH_OBJECTIVES) {
                    if (tile->bit & BIT_SUPPLY_REMOVE) {
                        PlayersData[faction_id].unk_79[region]++;
                    }
                } else if (flags & PFLAG_MAP_REVEALED
                    || tile->visibility & (1 << faction_id)) {
                    PlayersData[faction_id].unk_79[region]++;
                }
            }
            int owner = -1;
            int base_id = base_find(x, y, -1, region, -1, -1);
            BOOL is_ocean_tile = (tile->climate & 0xE0) < (ALT_SHORE_LINE << 5);
            if (base_id >= 0) {
                int max_dist = (int)Rules->territory_max_dist_base;
                if (is_ocean_tile) {
                    max_dist >>= 1;
                }
                if (*BaseFindDist < max_dist) {
                    owner = Bases[base_id].faction_id_current;
                }
            }
            if (is_ocean_tile) {
                for (int i = 0; i < 9; i++) {
                    int x_adj = territory_xrange(x + RadiusBaseX[i]);
                    int y_adj = y + RadiusBaseY[i];
                    if (!on_map(x_adj, y_adj)) {
                        continue;
                    }
                    int adjacent_id = base_at(x_adj, y_adj);
                    if (adjacent_id >= 0) {
                        owner = Bases[adjacent_id].faction_id_current;
                        break;
                    }
                }
            }
            if (owner != tile->territory) {
                tile->bit2 |= 0x400000; // TODO: identify value
                *UnkBitfield1 |= 1; // TODO: identify global + value
            }
            tile->territory = (int8_t)owner;
            if (owner < 0) {
                owner = 0;
            }
            PlayersData[owner].unk_78[region]++;
            if (tile->climate & (RAINFALL_MOIST | RAINFALL_RAINY)) {
                if ((tile->val3 & 0xC0) < (ROCKINESS_ROCKY << 6)
                    && !(tile->bit & BIT_FUNGUS)) {
                    PlayersData[owner].unk_80[region]++;
                    if (tile->bit & BIT_BASE_RADIUS) {
                        PlayersData[owner].unk_81[region]++;
                    }
                }
            }
            int site = 0;
            if (!(tile->bit & BIT_BASE_RADIUS)) {
                site = tile->val2 >> 4;
                if (!site) {
                    site = world_site(x, y, false);
                    site_set(x, y, site);
                }
            }
            int region_best = (int)Continents[region].unk_3;
            if (site > region_best) {
                region_best = site;
            }
            Continents[region].unk_3 = (uint32_t)region_best;
            if (owner) {
                int owner_best = PlayersData[owner].unk_82[region];
                if (site > owner_best) {
                    owner_best = site;
                }
                PlayersData[owner].unk_82[region] = (uint8_t)owner_best;
            }
            if (!goody_at(x, y)) {
                continue;
            }
            Continents[region].pods++;
            PlayersData[owner].unk_83[region]++;
            if (owner) {
                if (tile->visibility & (1 << owner)
                    || (!is_human(owner) && *DiffLevelCurrent > 3)) {
                    add_site(owner, 3, 1, x, y);
                }
                continue;
            }
            for (int faction_id = 1; faction_id < MaxPlayerNum; faction_id++) {
                if (!PlayersData[faction_id].region_total_bases[region]) {
                    continue;
                }
                if (!(tile->visibility & (1 << faction_id))
                    && (is_human(faction_id) || *DiffLevelCurrent <= 3)) {
                    continue;
                }
                if (at_site(faction_id, 3, x, y)) {
                    continue;
                }
                if (base_find(x, y, faction_id, region, -1, -1) < 0) {
                    continue;
                }
                int limit = (int)Rules->territory_max_dist_base;
                if (*BaseFindDist < limit || *BaseFindDist > limit + limit) {
                    continue;
                }
                add_site(faction_id, 3, 1, x, y);
            }
        }
    }
}

/*
Purpose: Determine whether the turn currently belongs to another faction in a
         networked game, which is what gates local input.
Original Offset: 0052DC70
Return Value: TRUE only in a net game that is handing the turn around and whose
              active faction is not the local one
Status: Complete
*/
BOOL __cdecl not_my_turn() {
    // Both guards return before the comparison, so a non-net game and a net
    // game that is not currently passing the turn are both "my turn".
    if (!*IsMultiplayerNet) {
        return false;
    }
    if (!(*NetTurnFlags & 0x10)) {
        return false;
    }
    // `cmp ecx, edx` / `setne al`: the result is the inequality itself, not a
    // normalised flag, and the faction identity is what decides it.
    return *NetTurnFaction != *LocalFaction;
}

/*
Purpose: Handle creation of pop-up message on Planetfall.
Original Offset: 00589180
Return Value: n/a
Status: Complete
*/
void __cdecl planetfall(uint32_t faction_id) {
    parse_set(Players[faction_id].is_leader_female, false);
    parse_says(0, Players[faction_id].adj_name_faction, -1, -1);
    parse_says(2, Players[faction_id].title_leader, -1, -1);
    parse_says(3, Players[faction_id].name_leader, -1, -1);
    parse_set(Players[faction_id].noun_gender, Players[faction_id].is_noun_plural);
    parse_says(1, Players[faction_id].noun_faction, -1, -1); // unused in script, leaving in for now
    char script_id[13];
    if (*TurnCurrentNum) { // shifted logic to top to fix nonexistent accelerated script ids
        parse_num(0, *TurnCurrentNum);
        for (uint32_t i = 0; i < MaxSecretProjectNum; i++) {
            if (has_project(i, faction_id)) { // script assumes at least one SP is built per faction
                parse_says(4, Facility[FAC_HUMAN_GENOME_PROJ + i].name, -1, -1);
            }
        }
        strcpy_s(script_id, 13, "PLANETFALL2"); // bug fix: changed to fixed id rather than concat 2
    } else if (is_alien_faction(faction_id) && !_stricmp(Players[faction_id].filename, "USURPER")) {
        strcpy_s(script_id, 13, "PLANETFALLX");
    } else if (!_stricmp(Players[faction_id].filename, "FUNGBOY")) {
        strcpy_s(script_id, 13, "PLANETFALLF");
    } else {
        strcpy_s(script_id, 13, "PLANETFALL");
    }
    X_pop(script_id, NULL);
}

/*
Purpose: Clear and reset scenario game variables.
Original Offset: 005B38D0
Return Value: n/a
Status: Complete
*/
void __cdecl clear_scenario() {
    *ObjectiveReqVictory = 9999;
    *ObjectivesSuddenDeathVictory = 9999;
    *ObjectiveAchievePts = 0;
    *VictoryAchieveBonusPts = 0;
    *StartingMissionYear = Rules->normal_starting_year;
    *EndingMissionYear = *DiffLevelCurrent < DLVL_LIBRARIAN
        ? Rules->normal_end_year_low_three_diff : Rules->normal_end_year_high_three_diff;
}

/*
Purpose: Calculate game year from start date and turn number.
Original Offset: 005C89A0
Return Value: Game year
Status: Complete
*/
uint32_t __cdecl game_year(int turn) {
    return *StartingMissionYear + turn;
}

/*
Purpose: Craft game year string. Assume buffer of at least 80. Revisit and convert to std::string.
Original Offset: 005C89B0
Return Value: n/a
Status: Complete
*/
void __cdecl say_year(LPSTR output) {
    char year[80];
    _itoa_s(game_year(*TurnCurrentNum), year, 80, 10);
    strcat_s(output, 80, year);
}
