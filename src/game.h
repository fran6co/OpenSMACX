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
#pragma once

 /*
  * Game related objects, variables and functions.
  */
enum GameStateBitfield { // used by GameState
    STATE_UNK_1 = 0x1,
    STATE_UNK_2 = 0x2,
    STATE_UNK_4 = 0x4,
    STATE_UNK_8 = 0x8,
    STATE_IS_SCENARIO = 0x10,
    STATE_SCENARIO_CHEATED_FLAG = 0x20,
    STATE_SCENARIO_EDITOR = 0x40,
    STATE_OMNISCIENT_VIEW = 0x80,
    STATE_UNK_100 = 0x100, // set or unset in Path::continents
    STATE_UNK_200 = 0x200,
    STATE_UNK_400 = 0x400,
    STATE_UNK_800 = 0x800, // time expired? MP related? NOBONUSATEND
    STATE_DEBUG_MODE = 0x1000,
    STATE_VICTORY_CONQUER = 0x2000,
    STATE_DISPLAYED_COUNCIL_AVAIL_MSG = 0x4000, // set when #COUNCILOPEN is displayed to show once
    STATE_SCN_VICT_TERRITORY_COUNT_OBJ = 0x8000,
    STATE_COUNCIL_HAS_CONVENED = 0x10000, // set 1st time Planetary Council is convened
    STATE_VOLCANO_ERUPTED = 0x20000, // #VOLCANO, tied to Planet ecology; can only be triggered once
    STATE_SCN_VICT_ALL_ARTIFACTS_OBJ_UNIT = 0x40000,
    STATE_SCN_VICT_HIGHEST_AC_SCORE_WINS = 0x80000,
    STATE_PERIHELION_ACTIVE = 0x100000,
    STATE_VICTORY_DIPLOMATIC = 0x200000,
    STATE_VICTORY_ECONOMIC = 0x400000,
    STATE_RAND_FAC_LEADER_PERSONALITIES = 0x800000,
    STATE_RAND_FAC_LEADER_SOCIAL_AGENDA = 0x1000000,
    STATE_SCN_VICT_TERRAIN_ENH_COUNT_OBJ = 0x2000000,
    STATE_SCN_VICT_BASE_FACIL_COUNT_OBJ = 0x4000000,
    STATE_EDITOR_ONLY_MODE = 0x8000000,
    STATE_UNK_10000000 = 0x10000000,
    STATE_SCN_VICT_POPULATION_COUNT_OBJ = 0x20000000,
    STATE_SCN_VICT_TECH_COUNT_OBJ = 0x40000000,
    STATE_SCN_VICT_CREDITS_COUNT_OBJ = 0x80000000,
};

enum GameRulesBitfield { // used by GameRules
    RULES_DO_OR_DIE = 0x1,
    RULES_VICTORY_CONQUEST = 0x2,// allow 'Total War'
    RULES_VICTORY_ECONOMIC = 0x4, // allow 'Mine, All Mine'
    RULES_VICTORY_DIPLOMATIC = 0x8, // allow 'Peace in Our Time'
    RULES_LOOK_FIRST = 0x10,
    RULES_TECH_STAGNATION = 0x20,
    RULES_INTENSE_RIVALRY = 0x40,
    RULES_TIME_WARP = 0x80,
    RULES_NO_UNITY_SURVEY = 0x100,
    RULES_BLIND_RESEARCH = 0x200,
    RULES_IRONMAN = 0x400,
    RULES_VICTORY_TRANSCENDENCE = 0x800, // allow 'Higher Goal'
    RULES_VICTORY_COOPERATIVE = 0x1000, // allow 'One For All'
    RULES_NO_UNITY_SCATTERING = 0x2000,
    RULES_SPOILS_OF_WAR = 0x4000,
    RULES_BELL_CURVE = 0x8000,
    RULES_SCN_UNITY_PODS_NO_RESOURCES = 0x10000,
    RULES_SCN_UNITY_PODS_NO_MONOLITHS = 0x20000,
    RULES_SCN_UNITY_PODS_NO_ARTIFACTS = 0x40000,
    RULES_SCN_VICT_SOLO_MISSION = 0x80000,
    RULES_SCN_FORCE_PLAYER_PLAY_CURRENT_FACT = 0x100000,
    RULES_SCN_NO_NATIVE_LIFE = 0x200000,
    RULES_SCN_NO_COLONY_PODS = 0x400000,
    RULES_SCN_NO_TERRAFORMING = 0x800000,
    RULES_SCN_FORCE_CURRENT_DIFF_LEVEL = 0x1000000,
    RULES_SCN_NO_TECH_TRADING = 0x2000000,
    RULES_SCN_NO_TECH_ADVANCES = 0x4000000,
    RULES_SCN_VICT_OBJ_UNITS_REACH_FRIEND_OBJ_BASE = 0x8000000,
    RULES_SCN_VICT_OBJ_UNITS_REACH_FRIEND_HQ_BASE = 0x10000000,
    RULES_SCN_VICT_ALL_BASE_COUNT_OBJ = 0x20000000,
    RULES_SCN_VICT_SP_COUNT_OBJ = 0x40000000,
    RULES_SCN_NO_BUILDING_SP = 0x80000000,
};

enum GameDifficultyLevel {
    DLVL_CITIZEN = 0,
    DLVL_SPECIALIST = 1,
    DLVL_TALENT = 2,
    DLVL_LIBRARIAN = 3,
    DLVL_THINKER = 4,
    DLVL_TRANSCEND = 5,
};

// SMACX mode (true); SMAC mode (false). AN OBJECT, not a `BOOL *` to a
// fixed address: 0x009A6488 is above the end of stored `.data`, so its
// initial value is zero either way and the pointer form only bought an
// extra load. WinMain's `mov dword ptr [0x9a6488], 1` is a direct store.
extern BOOL ExpansionEnabled;
extern uint32_t GamePreferences;
extern uint32_t GameMorePreferences;
extern uint32_t GameWarnings; // used for setting pop notifications
extern uint32_t GameRules; // bitfield
extern uint32_t GameState; // bitfield
extern int DiffLevelCurrent;
extern int TurnCurrentNum;
// `int`: great_beelzebub compares it with `<= 1000` and the image's
// branch is `jle`, the signed test.
extern int ObjectiveReqVictory;
extern uint32_t ObjectivesSuddenDeathVictory;
extern uint32_t ObjectiveAchievePts;
extern uint32_t VictoryAchieveBonusPts;
extern uint32_t MissionYearCurrent;
extern uint32_t StartingMissionYear;
extern uint32_t EndingMissionYear;
uint32_t *const TectonicDetonationCount = (uint32_t *)0x00946138;
extern int SunspotDuration;
extern uint32_t MountPlanetX;
extern uint32_t MountPlanetY;
extern int DustCloudDuration;
extern BOOL IsMultiplayerNet;
extern BOOL IsMultiplayerPBEM;

// Net turn control. Bit 4 of the flag byte is set while the net game hands the
// turn around; the faction dword then names whose turn it currently is, and is
// compared against the local faction at 0x00939284. `net_control_turn` and
// `Console::use_time_bonus` read the same pair the same way.
extern uint8_t NetTurnFlags;
extern int NetTurnFaction;
extern int LocalFaction;


void __cdecl reset_territory();
void __cdecl planetfall(int faction_id);
void __cdecl clear_scenario();
void __cdecl say_year(LPSTR output);
void __cdecl repair_phase(int faction_id);

// The game's main loop, called once from WinMain between Jackal bring-up and
// teardown. Everything the player ever sees happens inside this call.
void __cdecl control_game();

MEASURED inline BOOL __cdecl not_my_turn() {
    // Both guards return before the comparison, so a non-net game and a net
    // game that is not currently passing the turn are both "my turn".
    if (!IsMultiplayerNet) {
        return false;
    }
    if (!(NetTurnFlags & 0x10)) {
        return false;
    }
    // `cmp ecx, edx` / `setne al`: the result is the inequality itself, not a
    // normalised flag, and the faction identity is what decides it.
    return NetTurnFaction != LocalFaction;
}

MEASURED inline uint32_t __cdecl game_year(int turn) {
    return StartingMissionYear + turn;
}

MEASURED inline int __cdecl energy_limit(int) {
    return 10;
}
