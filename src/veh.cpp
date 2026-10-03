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
#include "veh.h"
#include "lock.h"  // map_tile_bytes() - the map-record table pointer
#include "alpha.h"
#include "base.h"
#include "faction.h"
#include "game.h"
#include "general.h" // range
#include "flic.h"
#include "log.h"
#include "map.h"
#include "strings.h"
#include "palette.h"  // Reactor-zero naming observes the existing palette slot.
#include "technology.h"
#include "terraforming.h"
#include "deletionlist.h"  // VehDeletionList - the multiplayer deletion queue
#include "tutwin.h"  // TutWinBaseWindow - every removal's check_base/check_loc
#include "mapwin.h"  // draw_tile - the free single-tile redraw the removals end on
#include "netmsg.h"  // NetMsgPrimary - the promote family reports through it
#include "xpops.h"  // X_pop - transferred-unit single-player notifications.
#include "netdaemon.h"  // NetDaemonNet, synch_veh / order_veh - the action family's net half
#include "path.h"  // Paths - the move cost table the go-to verbs consult
#include "console.h"  // ConsoleGlobal - action_gate re-focuses the Console
#include "caviar.h"
#include "font.h"
#include "infowin.h"
#include "worldwin.h"
#include "reportif.h"
#include "maininterface.h"

#include "wave.h"
#include "fx.h"
#include <mmsystem.h>
#include "designwin.h" // Crawler animation advances while the design window is visible.
#include "vector.h"  // vector_add operates on actual three-float arrays.
#include "leaf_recoveries.h"  // The source-owned vector-length operation.
#include "original_seam.h"  // ORIGINAL / original_method - the two BaseWin repaint seams

int __cdecl popp(const char *title, const char *label, int channel,
                 const char *picture, int(__cdecl *callback)());

extern uint32_t LockEnableMask;  // 0x009A64E8, defined in lock.cpp

extern int DesignConsiderPrototype;
extern uint32_t DesignConsiderAbilities;
extern int DesignConsiderWeapon;
extern int DesignConsiderChassis;
extern int DesignConsiderReactor;
extern int DesignConsiderArmor;
extern int DesignConsiderOverride;

// Native580B51/580BF7 test immediate ability masks, not executable globals.
static const uint32_t PrototypeAbilityCompatibilityMask = 0x0044AE1Bu;
static const uint32_t PrototypeAbilitySimilarityMask = 0x0048841Au;

// consider_designs resets this scalar to -1; propose_proto records the
// first selected prototype only while the scalar remains negative.
// RECOVERY_OBJECT: v2 0x0068F0F0 0x0068F0F4 ?FirstProposedPrototype@@3HA 0x00580860
int FirstProposedPrototype = -1;

// Compose a readable prototype name from the already-loaded design rules.
// Rule-string fields retain pointer-bit values validated by the loaded rules.
static const char *prototype_rule_name(LPSTR string_id) {
    return StringTable.get(static_cast<int>(
        reinterpret_cast<intptr_t>(string_id)));
}

static const char *prototype_reactor_name(int reactor_id) {
    if (reactor_id == 0) {
        return StringTable.get(static_cast<int>(reinterpret_cast<intptr_t>(
            g_PALETTE2.animation_colours(4))));
    }
    return prototype_rule_name(Reactor[reactor_id - 1].name_short);
}

static void add_prototype_name_suffix(char *name, const char *suffix,
                                     bool &suffix_exhausted) {
    char *last_suffix = 0;
    for (char *found = strstr(name, suffix);
         found != 0;
         found = strstr(found + 1, suffix)) {
        last_suffix = found;
    }
    if (last_suffix == 0
        || strlen(last_suffix) > strlen(suffix) + 1) {
        ::strcat(name, " ");
        ::strcat(name, suffix);
        ::strcat(name, "2");
        return;
    }
    const size_t name_length = strlen(name);
    const char final_character = name[name_length - 1];
    if (final_character < '2' || final_character > '8') {
        suffix_exhausted = true;
    } else {
        name[name_length - 1] = final_character + 1;
    }
}

// ORIGINAL: 0x0057E040 ?name_proto@@YAXPADHHHHHHH@Z 0x0057E040-0x0057EF95
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/0057e040.cpp;image:0x0057E040-0x0057EF95", "EDI normalizes equality:0x0057EC53-0x0057EC61;primary restart and branch:0x0057EE10-0x0057EE35", "Owned accidental storage:Ability base0x009AB538,stride0x1C,abbreviation+8 =>0x009AB834 = VehPrototypes base0x009AB868-0x34;source:src/veh.h RulesAbility;source:src/alpha.cpp ability loader"], "reason": "Prototype naming composes loaded ability, chassis, weapon, armor and reactor rules, with faction-local duplicate-name selection. The reactor-zero path borrows Palette animation storage. Equal primary/alternate forms restart the candidate scan safely after primary collisions. For differing forms, the native primary-collision restart compares alternate text with bytes beginning at Ability[27].abbreviation storage, not its resolved string; its read boundary remains unresolved. Preserve the complete seed, original spans and unfinished sentinel rather than silently replacing that behavior."}
void __cdecl name_proto(char *output, int prototype_id, int faction_id,
                        int chassis_id, int weapon_id, int armor_id,
                        int ability_flags, int reactor_id) {
    // Duplicate-name restart at 0x0057EE10 reads before VehPrototypes on
    // the differing-name route. Its portable behavior remains unresolved.
    output[0] = '\0';
    for (int ability_id = 0; ability_id < MaxAbilityNum; ++ability_id) {
        if ((static_cast<uint32_t>(ability_flags) & (1u << ability_id)) == 0) {
            continue;
        }
        const char *abbreviation = StringTable.get(
            static_cast<int>(reinterpret_cast<intptr_t>(
                Ability[ability_id].abbreviation)));
        if (::strlen(abbreviation) == 0) continue;
        const RulesChassis &chassis = Chassis[chassis_id];
        if (ability_id == 5 && chassis.triad == TRIAD_AIR) continue;
        if (ability_id == 15) continue;
        if (ability_id == 3 && chassis.speed == 1
            && Weapon[weapon_id].offense_rating
                >= Armor[armor_id].defense_rating
            && (ability_flags & 0x8000) == 0) {
            continue;
        }
        abbreviation = StringTable.get(
            static_cast<int>(reinterpret_cast<intptr_t>(
                Ability[ability_id].abbreviation)));
        ::strcat(output, abbreviation);
        ::strcat(output, " ");
        break;
    }

    char alternate_name[257];
    alternate_name[0] = '\0';
    ::strcpy(alternate_name + 1, output);
    const RulesChassis &chassis = Chassis[chassis_id];
    const RulesWeapon &weapon = Weapon[weapon_id];
    const RulesArmor &armor = Armor[armor_id];
    int name_form_quality = 0;
    int chassis_is_not_basic = 0;
    if (weapon.mode < 3) {
        int name_combat_rating;
        int name_rating_limit;
        if (weapon.offense_rating < armor.defense_rating) {
            name_combat_rating = 6;
        } else {
            name_combat_rating = 3;
            name_rating_limit = armor.defense_rating;
        }
        if (weapon.offense_rating < armor.defense_rating) {
            name_rating_limit = weapon.offense_rating;
        }
        if (chassis.triad == TRIAD_SEA) {
            ++name_combat_rating;
        }
        if (chassis.triad == TRIAD_AIR) {
            name_combat_rating += name_combat_rating / 2;
        }
        name_form_quality = name_combat_rating < name_rating_limit;
        chassis_is_not_basic = chassis.triad != TRIAD_SEA;
        const char *large_offense_form = StringTable.get(
            static_cast<int>(reinterpret_cast<intptr_t>(chassis.offsv_lrg_name)));
        bool has_paired_large_forms = false;
        if (strlen(large_offense_form) != 0) {
            const char *large_defense_form = StringTable.get(
                static_cast<int>(reinterpret_cast<intptr_t>(chassis.defsv_lrg_name)));
            has_paired_large_forms = strlen(large_defense_form) != 0;
        }
        if (!has_paired_large_forms) name_form_quality = 0;
    }

    const int weapon_rating = weapon.offense_rating;
    bool has_gendered_name_branch = false;
    int gendered_form = (reactor_id - 1 ^ faction_id) & 1;
    const VehPrototype &named_prototype = VehPrototypes[prototype_id];
    if (named_prototype.unk_1 == 10
        && (LockEnableMask & (1u << (faction_id & 0x1f))) == 0
        && chassis.speed > 1) {
        has_gendered_name_branch = true;
    } else if (weapon_rating <= chassis.speed
               && armor.defense_rating <= chassis.speed
               && weapon_rating >= 0
               && chassis.triad == TRIAD_LAND) {
        has_gendered_name_branch = true;
    }

    bool has_unique_chassis_form = false;
    if (has_gendered_name_branch && weapon.mode < 3) {
        if (!has_abil(prototype_id, ABL_POLICE_2X)) {
            int chassis_form_label = weapon_rating < 2 ? 0x248 : 0x2d8;
            const char *chassis_form = label_get(chassis_form_label / 4);
            ::strcat(output, chassis_form);
            ::strcat(alternate_name + 1, chassis_form);
        } else {
            size_t primary_length = strlen(output);
            if (primary_length != 0) output[primary_length - 1] = '\0';
            size_t alternate_length = strlen(alternate_name + 1);
            if (alternate_length != 0) {
                const size_t shortened_length = strlen(output);
                alternate_name[shortened_length] = '\0';
            }
        }
        has_unique_chassis_form = true;
        gendered_form = chassis.speed > 1;
    }

    int lower_reactor_design = 0;
    if (!has_unique_chassis_form) {
        for (int walk = 0; walk < 2 * MaxVehProtoFactionNum; ++walk) {
            const int candidate_id = walk < MaxVehProtoFactionNum
                ? walk : faction_id * MaxVehProtoFactionNum + (walk & 0x3f);
            const VehPrototype &candidate = VehPrototypes[candidate_id];
            if ((candidate.flags & PROTO_ACTIVE) != 0
                && candidate.plan == named_prototype.plan
                && candidate.chassis_id == chassis_id) {
                if (candidate.reactor_id >= named_prototype.reactor_id) {
                    lower_reactor_design = 0;
                    break;
                }
                lower_reactor_design = 1;
            }
        }
    }

    const char *weapon_short_name = prototype_rule_name(weapon.name_short);
    const char *alternate_form = "";
    bool handled_name_case = false;
    if (weapon_rating < armor.defense_rating
        && weapon.mode < 3
        && weapon_rating >= 0
        && (static_cast<uint32_t>(ability_flags) & ABL_ARTILLERY) == 0) {
        if (!has_unique_chassis_form) {
            gendered_form ^= static_cast<uint8_t>(armor.defense_rating) & 1;
            const char *component_name = lower_reactor_design == 0
                ? prototype_rule_name(armor.name_short)
                : prototype_reactor_name(reactor_id);
            ::strcat(output, component_name);
            ::strcat(alternate_name + 1, component_name);
        }
        ::strcat(output, " ");
        ::strcat(alternate_name + 1, " ");
        ::strcat(output, prototype_rule_name(chassis.defsv_1_name));

        if (!has_unique_chassis_form || (prototype_id & 1) == 0) {
            alternate_form = prototype_rule_name(
                name_form_quality == 0 || chassis_is_not_basic != 0
                    ? chassis.defsv_2_name : chassis.defsv_lrg_name);
        } else {
            alternate_form = prototype_rule_name(
                name_form_quality == 0 || chassis_is_not_basic == 0
                    ? chassis.defsv_1_name : chassis.defsv_lrg_name);
        }
        handled_name_case = true;
    } else {
        const bool air_superiority_name_case =
            chassis.triad == TRIAD_AIR
            && (static_cast<uint32_t>(ability_flags) & ABL_AIR_SUPERIORITY) != 0
            && weapon.mode < 3 && weapon_rating >= 0;
        if (air_superiority_name_case) {
            if (!has_unique_chassis_form) {
                const char *component_name = lower_reactor_design != 0
                    ? prototype_reactor_name(reactor_id) : weapon_short_name;
                ::strcat(output, component_name);
                ::strcat(alternate_name + 1, component_name);
            }
            ::strcat(output, " ");
            ::strcat(alternate_name + 1, " ");
            ::strcat(output, prototype_rule_name(chassis.defsv_1_name));
            if (has_unique_chassis_form && (prototype_id & 1) != 0) {
                alternate_form = prototype_rule_name(
                    name_form_quality != 0 && chassis_is_not_basic != 0
                        ? chassis.defsv_lrg_name : chassis.defsv_1_name);
            } else {
                alternate_form = prototype_rule_name(
                    name_form_quality != 0 && chassis_is_not_basic == 0
                        ? chassis.defsv_lrg_name : chassis.defsv_2_name);
            }
            handled_name_case = true;
        } else {
            if (chassis.missile != 0) {
                if (weapon_rating > 'b') {
                    const char *reactor_name = prototype_reactor_name(reactor_id);
                    ::strcat(output, reactor_name);
                    ::strcat(alternate_name + 1, reactor_name);
                    ::strcat(output, " ");
                    ::strcat(alternate_name + 1, " ");
                    ::strcat(output, weapon_short_name);
                    ::strcat(alternate_name + 1, weapon_short_name);
                    alternate_form = "";
                    handled_name_case = true;
                } else if (weapon_id == 0x18 || weapon_id == 0x19) {
                    const char *reactor_name = prototype_reactor_name(reactor_id);
                    ::strcat(output, weapon_short_name);
                    ::strcat(alternate_name + 1, weapon_short_name);
                    ::strcat(output, " ");
                    ::strcat(alternate_name + 1, " ");
                    ::strcat(output, reactor_name);
                    ::strcat(alternate_name + 1, reactor_name);
                    ::strcat(output, " ");
                    ::strcat(alternate_name + 1, " ");
                    alternate_form = prototype_rule_name(chassis.offsv_1_name);
                    ::strcat(output, alternate_form);
                    ::strcat(alternate_name + 1, alternate_form);
                    alternate_form = "";
                    handled_name_case = true;
                }
            }
        }

        if (!handled_name_case) {
            if (weapon.mode > 2 && chassis_id != 0) {
                if (reactor_id > 1) {
                    const char *reactor_name = prototype_reactor_name(reactor_id);
                    ::strcat(output, reactor_name);
                    ::strcat(alternate_name + 1, reactor_name);
                    ::strcat(output, " ");
                    ::strcat(alternate_name + 1, " ");
                }
                const bool special_vehicle_mode =
                    weapon.mode == 7 && chassis_id == 3;
                const bool special_air_mode =
                    weapon.mode == 9 && chassis.triad == TRIAD_LAND
                    && strchr(output, ' ') != 0;
                if (!special_vehicle_mode && !special_air_mode) {
                    if ((weapon.mode == 8 || weapon.mode == 9)
                        && chassis_id == 3) {
                        alternate_form = label_get(0x28c / 4);
                        ::strcat(output, alternate_form);
                        ::strcat(alternate_name + 1, alternate_form);
                    } else {
                        ::strcat(output,
                                 prototype_rule_name(chassis.offsv_lrg_name));
                        alternate_form =
                            prototype_rule_name(chassis.offsv_2_name);
                        ::strcat(alternate_name + 1, alternate_form);
                    }
                    ::strcat(output, " ");
                    ::strcat(alternate_name + 1, " ");
                }
            }

            if (!has_unique_chassis_form) {
                gendered_form ^= static_cast<uint8_t>(weapon_rating) & 1;
                ::strcat(output, weapon_short_name);
                ::strcat(alternate_name + 1, weapon_short_name);
            }

            if (weapon.mode > 2) {
                handled_name_case = true;
                alternate_form = "";
            } else if ((static_cast<uint32_t>(ability_flags) & ABL_ARTILLERY) == 0) {
                if ((static_cast<uint32_t>(ability_flags) & ABL_AMPHIBIOUS) == 0
                    || chassis.speed != 1
                    || weapon_rating <= armor.defense_rating) {
                    ::strcat(output, " ");
                    if (lower_reactor_design == 0) {
                        alternate_form = prototype_rule_name(
                            name_form_quality == 0 || chassis_is_not_basic == 0
                                ? chassis.offsv_1_name : chassis.offsv_lrg_name);
                        ::strcat(output, alternate_form);
                    } else {
                        ::strcat(output, prototype_reactor_name(reactor_id));
                    }
                    ::strcat(alternate_name + 1, " ");
                    alternate_form = lower_reactor_design != 0
                        ? prototype_reactor_name(reactor_id)
                        : prototype_rule_name(
                            name_form_quality == 0 || chassis_is_not_basic != 0
                                ? chassis.offsv_2_name : chassis.offsv_lrg_name);
                    handled_name_case = true;
                } else {
                    ::strcat(output, " ");
                    alternate_form = label_get(0x998 / 4);
                    ::strcat(output, alternate_form);
                    ::strcat(alternate_name + 1, " ");
                    alternate_form = label_get(0x99c / 4);
                    handled_name_case = true;
                }
            } else {
                ::strcat(output, " ");
                alternate_form = label_get(0x87c / 4);
                ::strcat(output, alternate_form);
                ::strcat(alternate_name + 1, " ");
                alternate_form = label_get(0x880 / 4);
                handled_name_case = true;
            }
        }

        if (!handled_name_case) {
            if (!has_unique_chassis_form) {
                const char *component_name = lower_reactor_design == 0
                    ? weapon_short_name : prototype_reactor_name(reactor_id);
                ::strcat(output, component_name);
                ::strcat(alternate_name + 1, component_name);
            }
            ::strcat(output, " ");
            ::strcat(alternate_name + 1, " ");
            ::strcat(output, prototype_rule_name(chassis.defsv_1_name));
            if (!has_unique_chassis_form || (prototype_id & 1) == 0) {
                alternate_form = prototype_rule_name(
                    name_form_quality == 0 || chassis_is_not_basic != 0
                        ? chassis.defsv_2_name : chassis.defsv_lrg_name);
            } else {
                alternate_form = prototype_rule_name(
                    name_form_quality == 0 || chassis_is_not_basic == 0
                        ? chassis.defsv_1_name : chassis.defsv_lrg_name);
            }
        }
    }

    ::strcat(alternate_name + 1, alternate_form);

    const int primary_alternate_difference =
        std::strcmp(output, alternate_name + 1);
    bool suffix_exhausted = false;
    const char *duplicate_suffix = label_get(0xe5c / 4);
    for (int candidate_id = 0; candidate_id < MaxVehProtoNum;
         ++candidate_id) {
        const VehPrototype &candidate = VehPrototypes[candidate_id];
        const int candidate_faction = candidate_id / MaxVehProtoFactionNum;
        const bool candidate_is_other_design =
            candidate.chassis_id != chassis_id
            || candidate.weapon_id != weapon_id
            || candidate.armor_id != armor_id
            || candidate.reactor_id != reactor_id
            || candidate.ability_flags != static_cast<uint32_t>(ability_flags);
        if (candidate_id == prototype_id
            || (candidate_faction != 0 && candidate_faction != faction_id)
            || !candidate_is_other_design) {
            continue;
        }

        if (!suffix_exhausted
            && std::strcmp(output, candidate.veh_name) == 0) {
            if (primary_alternate_difference != 0) gendered_form = 1;
            add_prototype_name_suffix(output, duplicate_suffix,
                                      suffix_exhausted);
            // Native EDI is one for equal forms; that path skips the
            // alternate comparison and restarts at candidate zero.
            if (primary_alternate_difference == 0) candidate_id = -1;
        }
        if (primary_alternate_difference != 0
            && std::strcmp(alternate_name + 1, candidate.veh_name) == 0) {
            gendered_form = 0;
            add_prototype_name_suffix(alternate_name + 1, duplicate_suffix,
                                      suffix_exhausted);
            candidate_id = -1;
        }
    }

    if (gendered_form != 0) {
        ::strcpy(output, alternate_name + 1);
    }
    // BODY GOES HERE.
}

int VehCurrentCount;  // 0x009A64C8
int VehDropLiftVehID;  // 0x009B2280
int VehLiftX;  // 0x009B2278
int VehLiftY;  // 0x009B2284
BOOL VehBitError;  // 0x009B228C
int VehMoraleModifierCount; // only used by say_morale(), optimize to local var?
// Battle related globals
LPSTR VehBattleDisplayTerrain;

// ===== MANAGED GLOBALS - the veh.h tables, homed here =====
// Zero initialisation IS the image's initialisation: every base below reads
// ZERO at run time (`image_data.py`, all past the file-backed end of .data),
// and the alpha.txt / x.txt loaders fill them before use.
// RECOVERY_OBJECT: v1 0x009AB868 ?VehPrototypes@@3PAUVehPrototype@@A
VehPrototype VehPrototypes[MaxVehProtoNum];  // 0x009AB868
// RECOVERY_OBJECT: v1 0x00952828 ?Vehs@@3PAUVeh@@A
Veh Vehs[2049];  // 0x00952828
// The rules loader fills entries0..8; original load_vehicles45446C..4546B4
// additionally populates sprite metrics in entries9..18 at the same0x90 stride.
// The full table ends at Natural's independent start0x0094ADE0.
// RECOVERY_OBJECT: v2 0x0094A330 0x0094ADE0 ?Chassis@@3PAURulesChassis@@A 0x00454270
RulesChassis Chassis[ChassisStorageCount];  // 0x0094A330
// RECOVERY_OBJECT: v1 0x0094AE60 ?Weapon@@3PAURulesWeapon@@A
RulesWeapon Weapon[MaxWeaponNum];  // 0x0094AE60
// RECOVERY_OBJECT: v1 0x0094F278 ?Armor@@3PAURulesArmor@@A
RulesArmor Armor[MaxArmorNum];  // 0x0094F278
RulesReactor Reactor[MaxReactorNum];  // 0x009527F8
// Ability-model assembly reads the signed prerequisite at record +0x18;
// the original indexes these existing rules with a 0x1C-byte stride.
// RECOVERY_OBJECT: v1 0x009AB538 ?Ability@@3PAURulesAbility@@A
RulesAbility Ability[MaxAbilityNum];  // 0x009AB538
RulesMorale Morale[MaxMoraleNum];  // 0x00952328
RulesCombatMode DefenseModes[MaxDefenseModeNum];  // 0x00946A00
RulesCombatMode OffenseModes[MaxOffenseModeNum];  // 0x00946178
RulesOrder Order[MaxOrderNum];  // 0x0096C878
LPSTR PlansShortName[MaxPlanNum];  // 0x00945FE0
LPSTR PlansFullName[MaxPlanNum];  // 0x00952360
LPSTR Triad[MaxTriadNum];  // 0x0094F1A8
uint32_t VehBasicBattleMorale[2];  // 0x00912420
uint32_t VehBattleModCount[2];  // 0x00915614
BOOL VehBattleUnkTgl[2];  // 0x0091561C
int VehBattleModifier[8];  // 0x009155F0
char VehBattleDisplay[8][80];  // 0x0090F554

// boom temporarily suppresses one vehicle while redrawing the affected tile,
// then restores -1. This is separate from the Console's selected vehicle.
// RECOVERY_OBJECT: v2 0x00689F30 0x00689F34 ?VehDrawSuppressedVehicle@@3HA 0x005593E0
int VehDrawSuppressedVehicle = -1;

// These header-defined lifetime callbacks are emitted for the real
// Vector arrays in veh_scoot below, not forced by a measurement anchor.
// veh_scoot pairs this destructor with Vector construction for arrays of
// eight 12-byte elements, in both normal and exceptional cleanup paths.
// ORIGINAL: 0x0055B750 ??1Vector@@QAE@XZ 0x0055B750-0x0055B755 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original 0x0055B750 tail-jumps to Vector::close at 0x006343D0; span agrees with src/unrecovered/0055b750.cpp.", "veh_scoot constructs two eight-element arrays of stride 12 using constructor 0x006343C0 and destructor 0x0055B750 at 0x0055A197-0x0055A1D9; normal and exceptional array cleanup reuse that destructor."], "reason": "Real Vector destructor calling close, replacing the seed-only synthetic receiver shim without removing its historical obligation."}
// body      src/vector.h

/*
Purpose: Initialize a zero vector.
// ORIGINAL: 0x006343C0 ??0Vector@@QAE@XZ 0x006343C0-0x006343CD BYTE_EXACT
// body      src/vector.h
// size      13 bytes
// prototype void (__thiscall ??0Vector@@QAE@XZ)(Vector* this)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
// Movement rendering publishes this font for the current order label.
// Original 0x00559DF6 stores the FontQueue result, also passed to set_font.
// RECOVERY_OBJECT: v2 0x0093FAE0 0x0093FAE4 ?VehDrawFont@@3PAVFont@@A 0x005594F0
Font *VehDrawFont;
// Signed preference: negative/default/positive select slow/normal/fast.
// The independent INI loader 0x00588AA0 names it FastUnitAnim.
// RECOVERY_OBJECT: v2 0x0094581C 0x00945820 ?FastUnitAnim@@3HA 0x0055A150
int FastUnitAnim;

// ORIGINAL: 0x005594F0 ?veh_draw@@YAHPAUBuffer@@HHHHHH@Z 0x005594F0-0x0055A14E
// RECOVERY_BINDING: v1 {"evidence": ["Pinned5594F0..55A14E retains raw screen coordinates for vehicle bodies, applies metric offsets only to overlays, and re-reads shared caption scale after callbacks. Health foreground reuses the rectangle passed to black fill. Order labels are truncated to one character then brackets duplicated; copy only that character to avoid original malformed-label overflow. Independent original review corrects seed receiver offsets and preserves cached faction versus fresh Console reads.", "Preserve existing seed identity and complete original spans. Source binding is not a measured matching or runtime claim."], "reason": "Recover vehicle bodies and overlays using actual sprites, chassis metrics, faction palettes and font storage."}
// symbol    ?veh_draw@@YAHPAVBuffer@@HHHHHH@Z
int __cdecl veh_draw(Buffer * buffer, int veh_id, int screen_x, int screen_y, int zoom, int flags, int frame) {
    int x = screen_x, y = screen_y;

    if ((flags & 1) != 0) {
        veh_id = pick_top_veh(veh_id);
        if (VehDrawSuppressedVehicle != -1 && VehDrawSuppressedVehicle == veh_id) {
            return 0;
        }
    }

    unsigned int faction = Vehs[veh_id].faction_id;
    unsigned int factionSaved = faction;

    if ((zoom < -8 || (flags & 0x100) != 0) && (flags & 8) == 0 && (flags & 0x800) == 0) {
        int prototype = Vehs[veh_id].proto_id;
        if (faction != (unsigned int)ConsoleGlobal.faction_id_ &&
            VehPrototypes[prototype].plan == '\v' && (GameState & 0x80) == 0) {
            return 0;
        }
        if ((flags & 0x400) != 0) {
            return 0;
        }
        int rw, rh;
        if (MapWinCaptionScale < 1) {
            rh = (zoom * 5 + 0x50) * 10 / 16;
            rw = (zoom * 5 + 0x50) * 0x14 / 16;
        } else {
            rh = (MapWinCaptionScale * 0x32) / MapWinCaptionDivisor;
            rw = (MapWinCaptionScale * 100) / MapWinCaptionDivisor;
        }
        rw += x;
        rh += y;
        if ((flags & 0x200) == 0) {
            int dxy;
            if (MapWinCaptionScale < 1) {
                dxy = (zoom * 8 + 0x80) / 16;
            } else {
                dxy = (MapWinCaptionScale * 8) / MapWinCaptionDivisor;
            }
            x += dxy;
            rw -= dxy;
            y += dxy;
            rh -= dxy;
        } else {
            x -= 2;
            rw += 2;
            y -= 2;
            rh += 2;
        }
        int qy = (rh - y) / 4;
        int botY = y + qy;
        int botYr = rh + qy;
        int mid = ((rw - x) - botYr + botY) / 2;
        int leftX = x + mid;
        int rightX = rw - mid;
        RECT r1;
        r1.left = leftX; r1.top = botY; r1.right = rightX; r1.bottom = botYr;
        int col = (strcmp(Players[faction].search_key, "SPARTANS") == 0) ? 44 : 0;
        buffer->box(&r1, col, col);
        r1.left++; r1.bottom--; r1.top++; r1.right--;
        buffer->fill(&r1, InfoFactionTextColors[faction]);
        return 0;
    }

    if (MapWinCaptionScale < 1) {
        SpriteScaleNumerator = zoom + 0x10;
        SpriteScaleDenominator = 0x10;
    } else {
        SpriteScaleNumerator = MapWinCaptionScale;
        SpriteScaleDenominator = MapWinCaptionDivisor;
    }

    int prototype = Vehs[veh_id].proto_id;
    unsigned int category = VehPrototypes[prototype].chassis_id;
    if ((char)Weapon[VehPrototypes[prototype].weapon_id].icon > -1 &&
        category == 0) {
        category = (char)Weapon[VehPrototypes[prototype].weapon_id].icon + 9;
    }
    if ((char)VehPrototypes[prototype].icon_offset > -1) {
        category = (char)VehPrototypes[prototype].icon_offset + 9;
    }
    unsigned int sprite_category = category;

    unsigned int morale;
    if (faction == 0) {
        morale = morale_alien(veh_id, ConsoleGlobal.faction_id_);
    } else {
        morale = morale_veh(veh_id, 0, 0);
    }

    unsigned int variation = 0;
    if (sprite_category == 12 || sprite_category == 10 || sprite_category == 13 || sprite_category == 14)
        variation = morale;
    else if (sprite_category == 15) {
        const int prototype = Vehs[veh_id].proto_id;
        if (prototype == 17) variation = 1;
        if (prototype == 18) variation = 2;
    }

    if ((flags & 0x1000) == 0) {
        int dx, dy;
        if (MapWinCaptionScale < 1) {
            dx = (zoom + 0x10) * Chassis[sprite_category].sprite_flag_x_off[variation] / 16;
        } else {
            dx = (int)(Chassis[sprite_category].sprite_flag_x_off[variation] * MapWinCaptionScale) / MapWinCaptionDivisor;
        }
        x += dx;
        if (MapWinCaptionScale < 1) {
            dy = (zoom + 0x10) * Chassis[sprite_category].sprite_flag_y_off[variation] / 16;
        } else {
            dy = (int)(Chassis[sprite_category].sprite_flag_y_off[variation] * MapWinCaptionScale) / MapWinCaptionDivisor;
        }
        y += dy;
    }

    unsigned int flagBit800 = flags & 0x800;
    if (flagBit800 == 0) {
        if ((int)sprite_category > 8) {
            Sprite *sp = &g_VEH_SPRITES[variation + sprite_category * 8];
            // EBX/ESI retain the unshifted origin; the adjusted stack
            // coordinates are only for the flag/health/order overlays.
            sp->draw_dest(buffer, sp->cTransparentIndex_, screen_x, screen_y, ShadowColorTable);
        }
        SpriteScaleNumerator = 1;
        SpriteScaleDenominator = 1;
        if ((flags & 0x80) == 0 && (int)sprite_category < 9) {
            if ((flags & 0x10) == 0) {
                if ((flags & 0x20) == 0) {
                    vehdraw_draw(buffer, veh_id, screen_x, screen_y, flags, zoom, frame);
                } else {
                    vehdraw_scoot(buffer, screen_x, screen_y, frame, zoom);
                }
            } else {
                vehdraw_turn(buffer, screen_x, screen_y, frame, zoom);
            }
        }
    }

    if (MapWinCaptionScale < 1) {
        SpriteScaleDenominator = 0x10;
        SpriteScaleNumerator = zoom + 0x10;
    } else {
        SpriteScaleNumerator = MapWinCaptionScale;
        SpriteScaleDenominator = MapWinCaptionDivisor;
    }

    if ((((flags & 0x40) != 0 || (flags & 0x400) != 0) && flagBit800 == 0) ||
        (factionSaved != (unsigned int)ConsoleGlobal.faction_id_ &&
         VehPrototypes[Vehs[veh_id].proto_id].plan == '\v' &&
         (GameState & 0x80) == 0)) {
        goto tail;
    }

    {
        int exhausted = 0;
        if (IsMultiplayerNet == 0) {
            if (faction == (unsigned int)ConsoleGlobal.faction_id_) {
                unsigned int spd = speed(veh_id, 0);
                int diff = (int)spd - Vehs[veh_id].moves_expended;
                if (diff < 0 || (diff < 1000 && spd == Vehs[veh_id].moves_expended)) {
                    exhausted = 1;
                }
            }
        } else {
            unsigned int spd = speed(veh_id, 0);
            int diff = (int)spd - Vehs[veh_id].moves_expended;
            if (diff < 0 || (diff < 1000 && spd == Vehs[veh_id].moves_expended)) {
                exhausted = 1;
            }
        }

        unsigned int stackFaction = faction;
        if ((flags & 2) != 0) {
            int stackN = stack_check(veh_id, 1, -1, -1, -1);
            stackFaction = faction;
            if (stackN > 1) {
                if (Chassis[sprite_category].sprite_flag_x_off[variation] > 0x32) {
                    int dxs;
                    if (MapWinCaptionScale < 1) {
                        dxs = (zoom * 3 + 0x30) * 2 / 16;
                    } else {
                        dxs = (MapWinCaptionScale * 6) / MapWinCaptionDivisor;
                    }
                    x -= dxs;
                }
                Sprite *sp = &g_FLAGS_VEH_SPRITES[exhausted + (faction * 7 + morale) * 2];
                sp->draw_mono(buffer, sp->cTransparentIndex_, x, y, InfoFactionShadowColors[faction]);
                if (MapWinCaptionScale < 1) {
                    x += (zoom * 3 + 0x30) * 2 / 16;
                } else {
                    x += (MapWinCaptionScale * 6) / MapWinCaptionDivisor;
                }
            }
        }

        Sprite *sp2 = &g_FLAGS_VEH_SPRITES[exhausted + (stackFaction * 7 + morale) * 2];
        sp2->draw(buffer, sp2->cTransparentIndex_, x, y);

        RECT r2;
        int rr24, rr28, rr2c, rr30;
        if (MapWinCaptionScale < 1) {
            rr24 = (zoom + 0x10) * 0x15 / 16;
            rr28 = (zoom * 2 + 0x20) / 16;
            rr2c = (zoom * 3 + 0x30) * 2 / 16;
            rr30 = (zoom * 3 + 0x30) * 2 / 16;
        } else {
            rr24 = (MapWinCaptionScale * 0x15) / MapWinCaptionDivisor;
            rr28 = (MapWinCaptionScale * 2) / MapWinCaptionDivisor;
            rr2c = (MapWinCaptionScale * 6) / MapWinCaptionDivisor;
            rr30 = (MapWinCaptionScale * 6) / MapWinCaptionDivisor;
        }
        rr2c += y;
        rr30 += x;
        rr28 = rr30 + rr28;
        rr24 = rr2c + rr24;
        r2.left = rr30; r2.top = rr2c; r2.right = rr28; r2.bottom = rr24;
        buffer->fill(&r2, 0);

        // No callback intervenes between these state reads and the foreground
        // fill. Reactor and damage are bytes; health is bounded to 1..1000.
        const int health_prototype = Vehs[veh_id].proto_id;
        int reactor = VehPrototypes[health_prototype].reactor_id;
        if (reactor < 1) reactor = 1;
        else if (reactor > 100) reactor = 100;
        const int maximum_health = VehPrototypes[health_prototype].plan == 12
            ? 1 : reactor * 10;
        const int damage = Vehs[veh_id].dmg_incurred;
        const int remaining_health = maximum_health - damage;
        if (remaining_health > 0) {
            // Reuse the rectangle passed to black fill, including any changes
            // made by that call, rather than rebuilding it from stale locals.
            r2.bottom = (r2.bottom - r2.top) * remaining_health / maximum_health + r2.top;
            int health_color = 249;
            if (damage < maximum_health * 2 / 3) health_color = 251;
            if (damage < maximum_health / 3 || damage == 0) health_color = 250;
            buffer->fill(&r2, health_color);
        }

        VehDrawFont = g_FONTQUEUE_VAL2.get_zoom(zoom, 1, 4, 1);
        buffer->set_font(VehDrawFont, 0, 0, 0);
        int col2 = (strcmp(Players[faction].search_key, "SPARTANS") == 0) ? 44 : 0;
        buffer->set_text_color(col2, -1, 1, 1);

        int triadIdx = (signed char)Vehs[veh_id].order;
        if (triadIdx < 0) triadIdx = 0;
        else if (triadIdx > 0x1d) triadIdx = 0x1d;

        if (MapWinCaptionScale < 1) {
            rr24 = (zoom + 0x10) * 0xe / 16;
            rr28 = (zoom + 0x10) * 0xb / 16;
            rr2c = (zoom * 5 + 0x50) / 16;
            rr30 = (zoom * 5 + 0x50) * 2 / 16;
        } else {
            rr24 = (MapWinCaptionScale * 0xe) / MapWinCaptionDivisor;
            rr28 = (MapWinCaptionScale * 0xb) / MapWinCaptionDivisor;
            rr2c = (MapWinCaptionScale * 5) / MapWinCaptionDivisor;
            rr30 = (MapWinCaptionScale * 10) / MapWinCaptionDivisor;
        }
        rr2c += y;
        rr30 += x;
        rr28 = rr30 + rr28;
        rr24 = rr2c + rr24;
        r2.left = rr30; r2.top = rr2c; r2.right = rr28; r2.bottom = rr24;

        char *word = StringTable.get(reinterpret_cast<int>(Order[triadIdx].letter));
        // The original copies an unbounded label then discards all but its
        // first character. Copy only the displayed character: malformed long
        // rule labels must not overflow the original four-byte scratch.
        char lineBuf[4] = {word[0], 0, 0, 0};

        if ((Vehs[veh_id].state & 0x4000) != 0) {
            strcpy(lineBuf, "/");
        }
        if ((Vehs[veh_id].state & 0x1000200) == 0x1000200) {
            strcpy(lineBuf, "P");
        }
        if ((Vehs[veh_id].state & 0x200) != 0) {
            if (Vehs[veh_id].order_auto_type == '\v') {
                strcpy(lineBuf, "*");
            }
            if ((Vehs[veh_id].state & 0x200) != 0 &&
                Vehs[veh_id].order_auto_type == '\n') {
                strcpy(lineBuf, "B");
            }
        }

        // 559FEC/559FF0 terminate the one-character label. 55A082
        // duplicates brackets into byte1 ("[[" / "]]"), not a dead local.
        lineBuf[2] = 0;
        lineBuf[1] = 0;
        if (faction == (unsigned int)ConsoleGlobal.faction_id_ &&
            (static_cast<unsigned char>(LockEnableMask) & (unsigned char)(1 << (faction & 0x1f))) != 0) {
            if (lineBuf[0] == '[' || lineBuf[0] == ']') {
                lineBuf[1] = lineBuf[0];
            }
        } else {
            if ((GameState & 0x80) != 0 ||
                (static_cast<unsigned char>(LockEnableMask) & (unsigned char)(1 << (ConsoleGlobal.faction_id_ & 0x1f))) == 0) {
                if ((GameState & 0x40) != 0 && (GameState & 0x1000) != 0) {
                    lineBuf[0] = Vehs[veh_id].move_to_ai_type;
                }
                if (lineBuf[0] != '-' && (Vehs[veh_id].state & 0x40000) != 0) {
                    int col3 = ((Vehs[veh_id].state & 0x20000) == 0) ? 0xfb : 0xf8;
                    buffer->set_text_color(col3, -1, 1, 1);
                }
                if (lineBuf[0] == '[' || lineBuf[0] == ']') {
                    lineBuf[1] = lineBuf[0];
                }
            } else {
                lineBuf[0] = '-';
            }
        }

        if (faction == (unsigned int)ConsoleGlobal.faction_id_) {
            if ((Vehs[veh_id].state & 0x200) != 0) {
                buffer->set_text_color(InfoFactionShadowColors[faction], -1, 1, 1);
                if (lineBuf[0] == '-') lineBuf[0] = 'a';
                if ((GameState & 0x40) != 0 && (GameState & 0x1000) != 0) {
                    lineBuf[0] = Vehs[veh_id].move_to_ai_type;
                }
            }
            if (faction == (unsigned int)ConsoleGlobal.faction_id_ &&
                (Vehs[veh_id].state & 0x8000000) != 0) {
                int col4 = ConsoleGlobal.faction_id_ == 3 ? 250 : 255;
                buffer->set_text_color(col4, -1, 1, 1);
            }
        }
        buffer->write_cent_l(lineBuf, &r2, (int)strlen(lineBuf));
    }

tail:
    SpriteScaleNumerator = 1;
    SpriteScaleDenominator = 1;
    if (flagBit800 != 0) {
        SpriteScaleDenominator = 1;
        SpriteScaleNumerator = 1;
        return 0;
    }
    return (int)sprite_category < 9;
}

// ORIGINAL: 0x0055A150 ?veh_scoot@@YAXHHHHH@Z 0x0055A150-0x0055AF11;0x0065FF70-0x0065FFC7
// RECOVERY_BINDING: v1 {"evidence": ["Original55A175..55A1D9 constructs Buffer[8],Vector[8],singleton Vector,Vector[8], with reverse normal teardown55AEA7..55AEFB and EH65FF70..65FFC7. Use genuine value subtraction, mutating scale and compound operations. Preserve flags1 turn-only cleanup, flags4000 immediate redraw, fresh MapWinTable receivers and preference reads, source/destination copy direction, wrap and timing. Zero-initialized active flags explicitly make null slots inactive rather than preserving original uninitialized reads.", "Preserve existing seed identity and complete original spans. Source binding is not a measured matching or runtime claim."], "reason": "Recover real movement animation with automatic Buffer and Vector lifetime rather than raw stack/return-slot scaffolding."}
void __cdecl veh_scoot(int veh_id, int tile_x, int tile_y, int direction, int flags) {
    Buffer saved[8];
    Vector position[8];
    Vector destination;
    Vector increment[8];
    // Original skips the flag store for null slots. Startup normally fills
    // all eight slots; explicitly treating null slots as inactive is a
    // coordinator-authorized safety correction to that undefined case.
    int active[8] = {0};
    int start_x[8], start_y[8], end_x[8], end_y[8];
    int margin_x[8], margin_y[8];
    RECT area[8];

    VehDrawPositionY = tile_y;
    VehDrawPositionX = tile_x;
    VehDrawPositionVehicle = veh_id;
    bool retain_origin = false;
    if (direction >= 8) {
        direction %= 8;
        retain_origin = true;
    }
    g_SCOOT_WAVE.play();
    int next_x = tile_x + VehMoveDeltaX[direction];
    if (!(MapIsFlat & 1)) {
        if (next_x < 0) next_x += MapLongitudeBounds;
        else if (next_x >= MapLongitudeBounds) next_x -= MapLongitudeBounds;
    }
    const int next_y = tile_y + VehMoveDeltaY[direction];
    const bool immediate = (flags & 0x4000) != 0;
    int delay = 0, move_frames = 0;
    if (!immediate && IsMultiplayerNet &&
        Vehs[veh_id].faction_id != ConsoleGlobal.faction_id_) {
        WorldClimateWorldWin.set_detail_map();
        WorldClimateWorldWin.viewport_outline_enabled_ = 1;
    }
    draw_tile(tile_x, tile_y, 0);
    draw_tile(next_x, next_y, 0);
    if (!immediate) {
        delay = IsMultiplayerNet ? 10 : 16;
        const int triad = Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad;
        if (triad == 1) delay = delay * 2 / 3;
        if (triad == 2) delay /= 3;
        move_frames = FastUnitAnim < 0 ? 9 : (FastUnitAnim == 0 ? 6 : 3);
        if (IsMultiplayerNet) {
            if (Vehs[veh_id].faction_id == ConsoleGlobal.faction_id_) delay = 0;
            move_frames = 4;
            Vehs[veh_id].facing = static_cast<int8_t>(direction);
        }
    }
    for (int i = 0; i < 8; ++i) {
        MapWin *window = MapWinTable[i];
        if (!window) continue;
        active[i] = 0;
        if (i != 0 && !window->field_1DD74_) continue;
        bool visible = false;
        for (int endpoint = 0; endpoint < 2; ++endpoint) {
            int x = endpoint ? next_x : tile_x;
            const int y = endpoint ? next_y : tile_y;
            if (y < static_cast<int>(window->field_1DDA8_) ||
                y >= static_cast<int>(window->field_1DDD8_ + window->field_1DDD0_ + window->field_1DDA8_))
                continue;
            if (!(MapIsFlat & 1)) {
                if (x < static_cast<int>(window->field_1DDA4_)) x += MapLongitudeBounds;
                if (x >= static_cast<int>(window->field_1DDD4_ + window->field_1DDCC_ + window->field_1DDA4_))
                    x -= MapLongitudeBounds;
            }
            if (x >= static_cast<int>(window->field_1DDA4_) &&
                x < static_cast<int>(window->field_1DDD4_ + window->field_1DDCC_ + window->field_1DDA4_)) {
                visible = true;
                break;
            }
        }
        if (!visible) continue;
        active[i] = 1;
        window->tile_to_pixel(tile_x, tile_y, &start_x[i], &start_y[i]);
        MapWinTable[i]->tile_to_pixel(next_x, next_y, &end_x[i], &end_y[i]);
        window = MapWinTable[i];
        start_y[i] -= window->field_1DDC0_;
        end_y[i] -= window->field_1DDC0_;
        if (!immediate) {
            position[i].x_ = static_cast<float>(start_x[i]);
            position[i].y_ = static_cast<float>(start_y[i]);
            destination.x_ = static_cast<float>(end_x[i]);
            destination.y_ = static_cast<float>(end_y[i]);
            increment[i] = destination - position[i];
            increment[i].scale(1.0f / static_cast<float>(move_frames));
            position[i] += increment[i];
        }
        window = MapWinTable[i];
        area[i].left = start_x[i] < end_x[i] ? start_x[i] : end_x[i];
        area[i].top = start_y[i] < end_y[i] ? start_y[i] : end_y[i];
        area[i].right = (start_x[i] > end_x[i] ? start_x[i] : end_x[i]) + window->field_1DDB4_;
        area[i].bottom = (start_y[i] > end_y[i] ? start_y[i] : end_y[i]) +
                         window->field_1DDB8_ + window->field_1DDC0_;
        if (immediate) {
            RECT origin;
            origin.left = start_x[i]; origin.top = start_y[i];
            origin.right = start_x[i] + window->field_1DDB4_;
            origin.bottom = start_y[i] + window->field_1DDB8_ + window->field_1DDC0_;
            UnionRect(&area[i], &origin, &area[i]);
        }
        RECT end_clip, start_clip;
        MapWinTable[i]->compute_clip(next_x, next_y, 0, &end_clip);
        MapWinTable[i]->compute_clip(tile_x, tile_y, 0, &start_clip);
        UnionRect(&area[i], &end_clip, &start_clip);
        if (!immediate) {
            margin_x[i] = (area[i].right - area[i].left) / 4;
            margin_y[i] = (area[i].bottom - area[i].top) / 4;
            area[i].left -= margin_x[i]; area[i].right += margin_x[i];
            area[i].top -= margin_y[i]; area[i].bottom += margin_y[i];
            saved[i].init(area[i].right - area[i].left, area[i].bottom - area[i].top, 0, 0);
            static_cast<Buffer *>(MapWinTable[i])->copy(&saved[i], area[i].left, area[i].top,
                0, 0, area[i].right - area[i].left, area[i].bottom - area[i].top);
        }
    }
    if (immediate) {
        Vehs[veh_id].facing = static_cast<int8_t>(direction);
    } else {
        // Preference is re-read after all preparation callbacks.
        const int turn_speed = FastUnitAnim < 0 ? 25 : (FastUnitAnim == 0 ? 16 : 8);
        const int turn_frames = vehdraw_turn_start(veh_id, direction, turn_speed, 2);
        DWORD previous_time = timeGetTime();
        for (int turn_frame = 0; turn_frame < turn_frames; ++turn_frame) {
            for (int i = 0; i < 8; ++i) if (active[i]) {
                const int width = area[i].right - area[i].left;
                const int height = area[i].bottom - area[i].top;
                RECT moving = {start_x[i], start_y[i], start_x[i] + width, start_y[i] + height};
                RECT intersection;
                if (IntersectRect(&intersection, &moving, &area[i])) {
                    veh_draw(static_cast<Buffer *>(MapWinTable[i]), veh_id, start_x[i], start_y[i],
                        static_cast<int>(MapWinTable[i]->field_1DD98_), 0x12, turn_frame);
                    MapWinTable[i]->update(&area[i], 0);
                    saved[i].copy(static_cast<Buffer *>(MapWinTable[i]),
                        start_x[i] - margin_x[i] - area[i].left,
                        start_y[i] - margin_y[i] - area[i].top,
                        start_x[i] - margin_x[i], start_y[i] - margin_y[i], width, height);
                }
            }
            DWORD now;
            do { do_non_input(); now = timeGetTime(); }
            while (static_cast<int>(now - previous_time) < delay / 2);
            previous_time = now;
        }
        // Original jumps straight to global-coordinate cleanup for this flag.
        if (flags & 1) {
            VehDrawPositionX = -1;
            VehDrawPositionY = -1;
            flush_input();
            return;
        }
        vehdraw_scoot_start(veh_id, direction, move_frames, 2);
        for (int move_frame = 0; move_frame < move_frames; ++move_frame) {
            for (int i = 0; i < 8; ++i) if (active[i]) {
                const int width = area[i].right - area[i].left;
                const int height = area[i].bottom - area[i].top;
                RECT moving;
                moving.top = static_cast<int>(position[i].y_);
                moving.left = static_cast<int>(position[i].x_);
                moving.right = moving.left + width; moving.bottom = moving.top + height;
                RECT intersection;
                if (IntersectRect(&intersection, &moving, &area[i])) {
                    // Convert again after IntersectRect, as the original does.
                    const int y = static_cast<int>(position[i].y_);
                    const int x = static_cast<int>(position[i].x_);
                    veh_draw(static_cast<Buffer *>(MapWinTable[i]), veh_id, x, y,
                        static_cast<int>(MapWinTable[i]->field_1DD98_), 0x22, move_frame);
                    MapWinTable[i]->update(&area[i], 0);
                    const int restore_y = static_cast<int>(position[i].y_);
                    const int restore_x = static_cast<int>(position[i].x_);
                    saved[i].copy(static_cast<Buffer *>(MapWinTable[i]),
                        restore_x - margin_x[i] - area[i].left,
                        restore_y - margin_y[i] - area[i].top,
                        restore_x - margin_x[i], restore_y - margin_y[i], width, height);
                }
                position[i] += increment[i];
            }
            DWORD now;
            do { do_non_input(); now = timeGetTime(); }
            while (static_cast<int>(now - previous_time) < delay);
            previous_time = now;
        }
        for (int i = 0; i < 8; ++i) position[i] -= increment[i];
    }
    if (!retain_origin) {
        VehDrawPositionX = next_x;
        VehDrawPositionY = next_y;
    }
    for (int final_slot = 0; final_slot < 8; ++final_slot) if (active[final_slot]) {
        veh_draw(static_cast<Buffer *>(MapWinTable[final_slot]), veh_id,
            end_x[final_slot], end_y[final_slot],
            static_cast<int>(MapWinTable[final_slot]->field_1DD98_), 2, 1);
        MapWinTable[final_slot]->soft_update(&area[final_slot]);
        do_all_draws();
    }
    VehDrawPositionX = -1;
    VehDrawPositionY = -1;
    flush_input();
}


// ORIGINAL: 0x005593E0 ?pick_top_veh@@YAHH@Z 0x005593E0-0x005594EB
// RECOVERY_BINDING: v1 {"evidence": ["Original5593E0..5594EB climbs signed previous links then visits next links, snapshots the Console faction, excludes temporary suppression, prefers eligible selected vehicle and retains the first highest-score entry.", "Original559466 test dh,42 establishes state mask4200, correcting the seed420000. Original ctor/reset member accesses and ConsoleGlobal9156B0 establish faction/cursor/selected/enable ownership, not independent globals. Observed VC6 symbol and original complete seed extent."], "reason": "Recover stack display selection against real vehicle/prototype/chassis records and Console state."}
int __cdecl pick_top_veh(int veh_id) {
    int best = veh_id;
    int best_score = -99;
    if (veh_id < 0) {
        return best;
    }
    int index = veh_id;
    int previous = Vehs[index].prev_veh_id_stack;
    while (previous >= 0) {
        index = previous;
        previous = Vehs[index].prev_veh_id_stack;
    }
    if (index < 0) {
        return best;
    }
    const int faction = ConsoleGlobal.faction_id_;
    do {
        if (VehDrawSuppressedVehicle == -1 || index != VehDrawSuppressedVehicle) {
            if (index == ConsoleGlobal.selected_vehicle_
                && Vehs[index].faction_id == faction
                && (Vehs[index].state & 0x4200) == 0
                && ConsoleGlobal.cursor_active_ != 0
                && ConsoleGlobal.field_23BE4_ != 0) {
                return index;
            }
            const int prototype = Vehs[index].proto_id;
            const int chassis = VehPrototypes[prototype].chassis_id;
            const int score = Chassis[chassis].triad
                + (Vehs[index].faction_id == faction ? 4 : 0);
            if (score > best_score) {
                best_score = score;
                best = index;
            }
        }
        index = Vehs[index].next_veh_id_stack;
    } while (index >= 0);
    return best;
}

// Four normal tables, each indexed by the four terrain-height levels in
// vehdraw_compute_facet_normals. The original initializer writes 7^4 vectors
// per table, not one flat array accessed through pointers to its neighbours.
// RECOVERY_OBJECT: v2 0x008CDD10 0x008D4D9C ?VehDrawFacetNormals0@@3PAUVOX_Vect@@A 0x004C1B10
VOX_Vect VehDrawFacetNormals0[2401];
// RECOVERY_OBJECT: v2 0x008D4D9C 0x008DBE28 ?VehDrawFacetNormals1@@3PAUVOX_Vect@@A 0x004C1B10
VOX_Vect VehDrawFacetNormals1[2401];
// RECOVERY_OBJECT: v2 0x008DBE28 0x008E2EB4 ?VehDrawFacetNormals2@@3PAUVOX_Vect@@A 0x004C1B10
VOX_Vect VehDrawFacetNormals2[2401];
// RECOVERY_OBJECT: v2 0x008E2EB4 0x008E9F40 ?VehDrawFacetNormals3@@3PAUVOX_Vect@@A 0x004C1B10
VOX_Vect VehDrawFacetNormals3[2401];

// compute_camera publishes the first rotation here; vehicle rendering reads
// its first component when converting facing angles into camera coordinates.
// RECOVERY_OBJECT: v2 0x008E9F40 0x008E9F4C ?VehDrawCameraAngles@@3UVOX_Vect@@A 0x004BF740
VOX_Vect VehDrawCameraAngles;

// load_voxels reads VEH_LIGHT_DIR_MAP, converts its three integer settings
// to floats and normalizes this vector. These are the image's defaults.
// RECOVERY_OBJECT: v2 0x00687AD8 0x00687AE4 ?VehLightDirMap@@3UVOX_Vect@@A 0x004BF8D0
VOX_Vect VehLightDirMap = {{-0.57735f, 0.57735f, 0.57735f}};
// load_voxels derives these offsets from the first part's two step vectors:
// AVGMTGUN, adroper, AVHT-GunReposition, and AVHT-LightsReposition.
// RECOVERY_OBJECT: v2 0x007AC1B0 0x007AC1BC ?VehGunMountOffset@@3UVOX_Vect@@A 0x00451030
VOX_Vect VehGunMountOffset;    // 0x007AC1B0
// RECOVERY_OBJECT: v2 0x0079A618 0x0079A624 ?VehDropOffset@@3UVOX_Vect@@A 0x00451030
VOX_Vect VehDropOffset;        // 0x0079A618
// RECOVERY_OBJECT: v2 0x00798C30 0x00798C3C ?VehHoverGunOffset@@3UVOX_Vect@@A 0x00451030
VOX_Vect VehHoverGunOffset;    // 0x00798C30
// RECOVERY_OBJECT: v2 0x00789B48 0x00789B54 ?VehHoverLightsOffset@@3UVOX_Vect@@A 0x00451030
VOX_Vect VehHoverLightsOffset; // 0x00789B48

// ORIGINAL: 0x004C1D10 ?vehdraw_setup@@YAXXZ 0x004C1D10-0x004C1D15 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original4C1D10 is a five-byte tail jump to4C1B10; load_voxels451066 calls this entry. No extra renderer initialization or copied thunk bytes."], "reason": "Recover the voxel-loader setup entry as an ordinary call to the existing vehicle facet-normal owner."}

void __cdecl vehdraw_setup() {
    vehdraw_compute_facet_normals();
}

// Animation frame read by model assembly. Original 8E9F50 starts at zero;
// scoot_start sets it to one, and visible crawler assembly advances it.
// RECOVERY_OBJECT: v2 0x008E9F50 0x008E9F54 ?VehDrawAnimationStep@@3HA 0x004C2470
int VehDrawAnimationStep;
// Independent original selection scratch words; both start at -1. The
// selection painter uses these source-owned words, not original addresses.
// RECOVERY_OBJECT: v1 0x00689F28 ?VehDrawSelection689F28@@3HA
int VehDrawSelection689F28 = -1;
// RECOVERY_OBJECT: v1 0x00689F2C ?VehDrawSelection689F2C@@3HA
int VehDrawSelection689F2C = -1;
// Artillery command: source-owned vehicle/rules/treaty state and actual
// network lock protocol. The empty-target destruction dependency below
// remains explicitly unfinished; build continuity does not prove that path.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004CD090 ?action_arty@@YAXHHH@Z 0x004CD090-0x004CD2ED
// RECOVERY_BINDING: v1 {"evidence": ["Original4CD090..4CD2ED: readiness and wrapped-distance gates; stack/treaty protection; Rules.artillery_max_rng indexes existing RadiusRange; mode3 offline versus mode1 network locking and message dispatch.", "Retained src/recovered/units/004cd090.cpp is evidence, not the source owner. Fresh original disassembly confirms imports and call order; no new byte or game-acceptance claim."], "reason": "Recover the artillery command through genuine vehicle, faction, rules and network owners; destruction of an empty target remains an unfinished dependency."}
void __cdecl action_arty(int veh_id, int x, int y) {
    if (Vehs[veh_id].faction_id == ConsoleGlobal.faction_id_ &&
        veh_ready(veh_id) == 0) {
        NetMsgPrimary.pop("UNITMOVED", 5000, 0, nullptr);
        return;
    }
    const int vehicle_y = Vehs[veh_id].y;
    int dx = abs(Vehs[veh_id].x - x);
    if ((MapIsFlat & 1) == 0 && dx > static_cast<int>(MapLongitude)) {
        dx = static_cast<int>(MapLongitudeBounds) - dx;
    }
    if (((abs(vehicle_y - y) + dx) & ~1) > 4) {
        NetMsgPrimary.pop("OUTOFRANGE", 5000, 0, nullptr);
        return;
    }

    const int target = stack_fix(veh_at(x, y));
    if (target < 0) {
        if (IsMultiplayerNet == 0) {
            action_destroy(veh_id, 0, x, y);
        } else if (NetDaemonNet.lock_veh(&veh_id, 4, x, y, 0) == 0) {
            message_data(0x2413, 0, veh_id, x, y, 0);
            NetDaemonNet.await_exec(1);
        }
        return;
    }
    const int faction = Vehs[veh_id].faction_id;
    const int target_faction = Vehs[target].faction_id;
    if (target_faction == faction ||
        (PlayersData[faction].diplo_treaties[target_faction] & 1U) != 0) {
        return;
    }
    const int direction = radius_move(Vehs[veh_id].x, Vehs[veh_id].y,
                                     x, y, RadiusRange[Rules.artillery_max_rng]);
    if (direction < 0) return;

    if (IsMultiplayerNet == 0) {
        NetDaemonNet.lock_state_[1].leading_words_[3] = 3;
        battle_fight(veh_id, direction, 1, 1, nullptr);
        return;
    }
    if (NetDaemonNet.maybe_lock(&veh_id, 0, 0, x, y, 4) != 0) return;
    NetDaemonNet.lock_state_[1].leading_words_[3] = 1;
    if (battle_fight(veh_id, direction, 1, 1, nullptr) != 0) {
        NetDaemonNet.unlock_veh();
    } else {
        message_veh(0x2402, veh_id, direction, 0);
        NetDaemonNet.await_exec(1);
    }
}
// ORIGINAL: 0x004CAA50 ?action_destroy@@YAXHHHH@Z 0x004CAA50-0x004CB302
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/004caa50.cpp", "recovery/tasks/mapwin-owner-coverage-wave.json", "Map byte+2 is val2; diplomatic treaty direction and callback rereads independently decoded from the pinned original."], "reason": "Recover the artillery and bombing destruction paths in their real vehicle owner, replacing the explicitly unfinished dependency while preserving its complete original span and all seed evidence."}
void __cdecl action_destroy(int veh_id, int bits, int x, int y) {
    Veh &vehicle = Vehs[veh_id];
    const int faction = vehicle.faction_id;
    veh_skip(veh_id);
    vehicle.state |= 0x40;

    bool assault = false;
    bool forest_weapon = false;
    int effect_y;
    const bool valid = y >= 0 && y < MapLatitudeBounds &&
                       x >= 0 && x < MapLongitudeBounds;
    if (!valid) {
        x = vehicle.x;
        effect_y = vehicle.y;
    } else {
        effect_y = y;
        Map *const tile = map_loc(x, y);
        // Original reads the low nibble of the tile byte at +0x02 (Map::val2).
        if ((tile->bit & BIT_BASE_IN_TILE) != 0 &&
            (tile->val2 & 0x0f) < 8) {
            if (faction != ConsoleGlobal.faction_id_) return;
            boom(x, y, 0x80);
            return;
        }

        const int scatter = rand() & static_cast<int>(0x80000001u);
        if (scatter != 0) {
            if (faction != ConsoleGlobal.faction_id_) return;
            NetMsgPrimary.pop("ARTYBOMBFAILED", 0x1388, 0, nullptr);
            return;
        }

        assault = true;
        const int initial_local_faction = ConsoleGlobal.faction_id_;
        bool in_local_territory = faction == initial_local_faction;
        if (!in_local_territory) {
            const int owner = whose_territory(initial_local_faction, x, y,
                                              nullptr, 0);
            // whose_territory may run callbacks; the image rereads the live
            // Console faction for this comparison, but retains the earlier
            // faction for its following territory lookup.
            in_local_territory = owner == ConsoleGlobal.faction_id_;
        }
        if (in_local_territory) {
            const int current_local_faction = ConsoleGlobal.faction_id_;
            if (faction != 0 && faction != current_local_faction) {
                boom(x, y, 0x80);
                boom(vehicle.x, vehicle.y, 0x80);
            }
            boom(x, y, 0x80);
        }

        const int target_faction = whose_territory(faction, x, y, nullptr, 0);
        if (target_faction != faction) {
            for (int neighbor = 1; neighbor < 25; ++neighbor) {
                int near_x = vehicle.x + RadiusOffsetX[neighbor];
                if (!(MapIsFlat & 1)) {
                    if (near_x < 0) near_x += MapLongitudeBounds;
                    else if (near_x >= MapLongitudeBounds) near_x -= MapLongitudeBounds;
                }
                const int near_y = vehicle.y + RadiusOffsetY[neighbor];
                if (near_y <= -1 || near_y >= MapLatitudeBounds ||
                    near_x <= -1 || near_x >= MapLongitudeBounds) continue;

                int dx = abs(near_x - x);
                if (!(MapIsFlat & 1) && MapLongitude < static_cast<uint32_t>(dx))
                    dx = MapLongitudeBounds - dx;
                const int dy = abs(near_y - effect_y);
                if (((dy + dx) & ~1) >= 5) continue;

                const int other_vehicle = veh_at(near_x, near_y);
                const int stack = stack_fix(other_vehicle);
                if (stack >= 0 && stack_check(stack, 0x0f, target_faction, -1, -1)) {
                    battle_fight(veh_id, neighbor, 1, 1, nullptr);
                    return;
                }
            }
        }
    }

    const int prototype_id = vehicle.proto_id;
    const int chassis_id = VehPrototypes[prototype_id].chassis_id;
    const RulesChassis &chassis = Chassis[chassis_id];
    const bool artillery = chassis.triad == TRIAD_AIR && chassis.missile == 0;
    if (artillery) {
        const int artillery_scatter = rand() & static_cast<int>(0x80000003u);
        if (artillery_scatter == 0) {
            if (faction != ConsoleGlobal.faction_id_) goto finish_destroy;
            NetMsgPrimary.pop("BOMBFAILED", 0x1388, 0, nullptr);
            goto finish_destroy;
        }
    }

    {
        Map *const tile = map_loc(x, effect_y);
        uint32_t target_bits;
        if (bits <= 0) {
            target_bits = tile->bit;
        } else {
            bits &= static_cast<int>(tile->bit);
            if (bits == 0) goto finish_destroy;
            bit_set(x, effect_y, bits, 0);
            target_bits = static_cast<uint32_t>(bits);
        }

        if (!has_abil(prototype_id, 0x2000)) target_bits &= ~0x20u;
        int probe = -1;
        int weapon;
        for (;;) {
            weapon = probe < 0 ? 9 : probe;
            if (Terraforming[weapon].bit & target_bits) break;
            ++probe;
            if (probe >= 0x13) {
                log_say("Arty Failed The Hard Way", bits,
                        static_cast<int>(target_bits), 0);
                if (assault) {
                    if (faction != ConsoleGlobal.faction_id_) goto finish_destroy;
                    NetMsgPrimary.pop("ARTYBOMBFAILED", 0x1388, 0, nullptr);
                } else {
                    if (chassis.triad != TRIAD_AIR ||
                        faction != ConsoleGlobal.faction_id_)
                        goto finish_destroy;
                    NetMsgPrimary.pop("BOMBFAILED", 0x1388, 0, nullptr);
                }
                goto finish_destroy;
            }
        }

        const uint32_t weapon_bit = Terraforming[weapon].bit;
        bits = static_cast<int>(static_cast<uint32_t>(bits) | weapon_bit);
        bit_set(x, effect_y, static_cast<int>(weapon_bit), 0);

        const uint8_t climate = map_loc(x, effect_y)->climate;
        const int terrain_name = ((climate & 0xe0) < 0x60) ? 1 : 0;
        const int terrain_name_id = reinterpret_cast<int>(
            terrain_name ? Terraforming[weapon].name_sea
                         : Terraforming[weapon].name);
        parse_says(2, StringTable.get(terrain_name_id),
                  -1, -1);

        forest_weapon = weapon == TERRA_FOREST;
        if (chassis.triad == TRIAD_AIR && faction == ConsoleGlobal.faction_id_)
            NetMsgPrimary.pop("BOMBSUCCEEDED", 0x1388, 0, nullptr);

        synch_bit(x, effect_y, faction);
        int enemy = faction == 0 ? ConsoleGlobal.faction_id_ : faction;
        enemy = whose_territory(enemy, x, effect_y, nullptr, 0);
        synch_bit(x, effect_y, enemy);

        if ((static_cast<uint32_t>(bits) & 0x01c00000u) != 0) {
            world_climate();
            draw_map(1);
        } else if ((static_cast<uint32_t>(bits) & 0x80000000u) == 0 ||
                   enemy != ConsoleGlobal.faction_id_) {
            draw_tiles(x, effect_y, 2);
        } else {
            draw_radius(x, effect_y, 2, 2);
        }

        if (faction == 0) {
            const bool local_knows =
                (PlayersData[ConsoleGlobal.faction_id_].flags & PFLAG_MAP_REVEALED) != 0 ||
                (map_loc(x, effect_y)->visibility &
                 (1u << (ConsoleGlobal.faction_id_ & 0x1f))) != 0;
            if (local_knows && enemy == ConsoleGlobal.faction_id_) goto finish_destroy;
            const int base_id = base_find(x, effect_y, -1, -1, -1,
                                          ConsoleGlobal.faction_id_);
            if (base_id < 0 ||
                Bases[base_id].faction_id_current != ConsoleGlobal.faction_id_)
                goto finish_destroy;
            parse_says(0, Bases[base_id].name_string, -1, -1);
            parse_says(1, VehPrototypes[prototype_id].veh_name, -1, -1);
            ConsoleGlobal.focus(x, effect_y, 0);
            if (forest_weapon) {
                if (vehicle.proto_id == BSC_SPORE_LAUNCHER) {
                    popp(*ClimatePopupTitle, "SPOREFOREST", 0,
                         "sporlnch_sm.pcx", nullptr);
                    goto finish_destroy;
                }
                NetMsgPrimary.pop("ATEFOREST", 0x1388, 0, nullptr);
            } else if (vehicle.proto_id == BSC_SPORE_LAUNCHER) {
                popp(*ClimatePopupTitle, "SPORESLAUNCHED", 0,
                     "sporlnch_sm.pcx", nullptr);
                goto finish_destroy;
            } else {
                NetMsgPrimary.pop("ATESTUFF", 0x1388, 0, nullptr);
            }
        } else {
            if (assault && enemy == ConsoleGlobal.faction_id_)
                NetMsgPrimary.pop("BOMBARD SUCCEEDED", 0x1388, 0, nullptr);
            if (enemy < 0 || enemy == faction ||
                (PlayersData[faction].diplo_treaties[enemy] & 8u) == 0)
                goto finish_destroy;
            if ((PlayersData[enemy].flags & PFLAG_MAP_REVEALED) == 0 &&
                (map_loc(x, effect_y)->visibility & (1u << (enemy & 0x1f))) == 0)
                goto finish_destroy;
            synch_bit(x, effect_y, enemy);
            add_goal(enemy, 0, 5, vehicle.x, vehicle.y, -1);
            if (enemy != ConsoleGlobal.faction_id_) goto finish_destroy;
            const int base_id = base_find(x, effect_y, -1, -1, -1,
                                          ConsoleGlobal.faction_id_);
            GenderDefault = Players[faction].noun_gender;
            PluralityDefault = Players[faction].is_noun_plural;
            parse_says(0, Players[faction].noun_faction, -1, -1);
            parse_says(1, Bases[base_id].name_string, -1, -1);
            parse_says(3, Players[faction].adj_name_faction, -1, -1);
            NetMsgPrimary.pop(assault ? "HAVEDESTROYED1" : "HAVEDESTROYED",
                             0x1388, 0, nullptr);
        }
    }

finish_destroy:
    order_veh(veh_id, -1, 3);
}

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
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/00504aa0.cpp", "src/veh.h", "src/flic.h", "recovery/tasks/mapwin-owner-coverage-wave.json"], "reason": "Recover per-window explosion and Flic animation with genuine typed static buffers, real MapWin/GraphicWin bases, source-owned weapon tables and original branch/timing/sound behavior; preserve the original seed and its correction evidence."}
// symbol    ?boom@@YAHHHH@Z
int __cdecl boom(int x, int y, int flags) {
    // The two arrays and both working buffers are genuine function-local
    // statics. Their constructors/destructors and first-use guard are thus
    // compiler-owned rather than replayed through fixed image addresses.
    static Buffer original_screen_snapshots[8];
    static Buffer redrawn_map_snapshots[8];
    static Flic animation;
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
    int flic_frame_count = 0;
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
        flic_frame_count = animation.frame_count() / 2;
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

    RECT saved_rectangles[8];
    bool active_windows[8];
    bool any_window = false;

    for (int slot = 0; slot < 8; ++slot) {
        MapWin *window = MapWinTable[slot];
        active_windows[slot] = false;
        if (window == nullptr || (slot != 0 && window->field_1DD74_ == 0)) {
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

        int pixel_x;
        int pixel_y;
        ConsoleGlobal.tile_to_pixel(x, y, &pixel_x, &pixel_y);
        pixel_x += static_cast<int>(window->field_1DDB4_) / 2;
        pixel_y += static_cast<int>(window->field_1DDB8_);

        const int zoom = window->field_1DD98_ + 0x10;
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
            window_buffer->copy(&original_screen_snapshots[slot], area.left,
                                area.top, 0, 0, width, -height);
        }
        if (((flags & 2) == 0 || (flags & 0x20) != 0) &&
            (flags & 0x40) == 0) {
            continue;
        }
        if ((flags & 2) != 0 && (flags & 0x20) == 0 &&
            (flags & 0x40) == 0) {
            VehDrawSuppressedVehicle = (flags & 8) == 0
                ? VehDrawSelection689F28 : VehDrawSelection689F2C;
            window->draw_tile(x, y, 0);
            VehDrawSuppressedVehicle = -1;
            window = MapWinTable[slot];
            window_buffer =
                static_cast<Buffer *>(static_cast<GraphicWin *>(window));
        }
        redrawn_map_snapshots[slot].init(width, height, 0, nullptr);
        window_buffer->copy(&redrawn_map_snapshots[slot], area.left, area.top,
                            0, 0, width, -height);
        original_screen_snapshots[slot].copy(window_buffer, 0, 0,
                                             area.left, area.top, width, -height);
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
            int sound = -1;
            const int sound_weapon =
                VehPrototypes[Vehs[second_selection].proto_id].weapon_id;
            switch (sound_weapon) {
            case 1: sound = 4; break;
            case 2: sound = 0x49; break;
            case 3: sound = 0x4a; break;
            case 4: sound = 0x59; break;
            case 5: sound = 0x4c; break;
            case 6: sound = 0x4d; break;
            case 7: sound = 0x4e; break;
            case 8: sound = 0x4f; break;
            case 9: sound = 0x50; break;
            case 10: sound = 0x51; break;
            case 11: sound = 0x58; break;
            case 12: sound = 0x55; break;
            case 13: sound = 0x56; break;
            case 14: sound = 0x57; break;
            }
            if (sound >= 0) {
                g_FX.play(sound, Vehs[second_selection].x,
                          Vehs[second_selection].y);
            }
            if (Vehs[saved_selection].proto_id == BSC_SPORE_LAUNCHER) {
                g_FX.play(0x5a, Vehs[second_selection].x,
                          Vehs[second_selection].y);
            }
        }
    }

    const unsigned long total_frames = flic_mode
        ? static_cast<unsigned long>(flic_frame_count)
        : 16ul;
    unsigned long last_tick = timeGetTime();
    for (unsigned long frame = 0; frame < total_frames; ++frame) {
        const bool skip_multiplayer_frame = IsMultiplayerNet &&
                                            play_animation != 0 &&
                                            (frame & 1ul) == 0;
        if (!skip_multiplayer_frame) {
            for (int slot = 0; slot < 8; ++slot) {
                if (!active_windows[slot]) continue;
                MapWin *window = MapWinTable[slot];
                Buffer *destination =
                    static_cast<Buffer *>(static_cast<GraphicWin *>(window));
                int draw_type = window->field_1DD98_ + 0x10;
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
                    draw_type = window->field_1DD98_ + 0x10;
                    const int faction = Vehs[saved_selection].faction_id;
                    working_buffer.draw_multi_table_dest(
                        destination, 0xff, saved_rectangles[slot].left,
                        saved_rectangles[slot].top,
                        draw_type * animation_height, frame_span << 4,
                        ExplosionColorTables[faction_color_group[faction]][0]);
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
                        saved_rectangles[slot].left,
                        saved_rectangles[slot].top,
                        ExplosionColorTables[table_row][0]);
                }
                window->update(&saved_rectangles[slot], 0);
                if (IsMultiplayerNet) break;
            }
            const SHORT shift = GetAsyncKeyState(VK_SHIFT);
            const bool fast = (shift & 0xff00) != 0 ||
                              (((GamePreferences & 8u) != 0) &&
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
                const int width = saved_rectangles[restore_slot].right -
                                  saved_rectangles[restore_slot].left;
                const int height = saved_rectangles[restore_slot].bottom -
                                   saved_rectangles[restore_slot].top;
                const bool use_redrawn_snapshot = flic_mode
                    ? (play_animation == 0)
                    : ((flags & 2) != 0 && frame > 7);
                Buffer &snapshot = use_redrawn_snapshot
                    ? redrawn_map_snapshots[restore_slot]
                    : original_screen_snapshots[restore_slot];
                snapshot.copy(destination, 0, 0,
                              saved_rectangles[restore_slot].left,
                              saved_rectangles[restore_slot].top, width, -height);
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
        working_buffer.set_clip(
            static_cast<GraphicWin *>(&ConsoleGlobal)->buffer_working_rect());
    }
    if (flic_mode && (flags & 1) != 0) {
        working_buffer.set_clip(saved_rectangles);
    }
    if (!flic_mode || (flags & 2) != 0) {
        for (int slot = 0; slot < 8; ++slot) {
            original_screen_snapshots[slot].close();
            redrawn_map_snapshots[slot].close();
        }
    }
    if ((GetAsyncKeyState(VK_SHIFT) & 0xff00) == 0 &&
        (GamePreferences & 8u) == 0 && !IsMultiplayerNet) {
        clock_wait(0x32);
    }
    TimerHoldFlag = 0;
    SpriteScaleNumerator = 1;
    SpriteScaleDenominator = 1;
    return 1;
}

// Shared infantry/native model placement, original 687AE8..687AF4.
// This is distinct from the four offsets derived from loaded voxel parts.
// RECOVERY_OBJECT: v2 0x00687AE8 0x00687AF4 ?VehInfantryOffset@@3UVOX_Vect@@A 0x004C3800
VOX_Vect VehInfantryOffset = {{-400.0f, 200.0f, 0.0f}};

// ORIGINAL: 0x004C2030 ?vehdraw_construct_abil@@YAHHPAUCaviar@@H@Z 0x004C2030-0x004C2360
// RECOVERY_BINDING: v1 {"evidence": ["Original type15 bonus loop calls the technology predicate with signed short prerequisite, reloads bonus value after that call, masks the shift count to five bits, and reloads loop count on continuation. Bonus additions and the subsequent ordinary ability additions are not deduplicated.", "Original faction alien flag selects human or alien colony/terraforming attachments; remaining masks select independently loaded model owners. Mapped callers discard the catalogued integer result."], "reason": "Recover ordered ability model additions from actual faction bonuses and rule records, retaining repeated selections."}
// symbol    ?vehdraw_construct_abil@@YAXIPAVCaviar@@H@Z
void __cdecl vehdraw_construct_abil(uint32_t abilities, Caviar *renderer, int faction_id) {
    for (int i = 0; i < Players[faction_id].faction_bonus_count; ++i) {
        if (Players[faction_id].faction_bonus_id[i] != RULE_FREEABIL) continue;
        if (!has_tech(Ability[Players[faction_id].faction_bonus_val1[i]].preq_tech, faction_id))
            continue;
        // has_tech precedes the second read of the bonus index in the image.
        const uint32_t bonus = abilities &
            (1u << (Players[faction_id].faction_bonus_val1[i] & 31));
        if (bonus & 0x1000000u) renderer->add_object(&g_SP_SOPORIFIC_CAVIARDATA, 0);
        if (bonus & 0x2000000u) renderer->add_object(&g_SP_DISSWAVE_CAVIARDATA, 0);
        if (bonus & 0x8000000u) renderer->add_object(&g_SP_NANOO_CAVIARDATA, 0);
        if (bonus & 0x4000000u) renderer->add_object(&g_SP_MARINED_CAVIARDATA, 0);
        if (Players[faction_id].rule_flags & RFLAG_ALIEN) {
            if (bonus & 1u) renderer->add_object(&g_SSFA_CAVIARDATA, 0);
            if (bonus & 0x2000u) renderer->add_object(&g_SFTA_CAVIARDATA, 0);
        } else {
            if (bonus & 1u) renderer->add_object(&g_SSF_CAVIARDATA, 0);
            if (bonus & 0x2000u) renderer->add_object(&g_SFT_CAVIARDATA, 0);
        }
        if (bonus & 0x20u) renderer->add_object(&g_SAS_CAVIARDATA, 0);
        if (bonus & 0x80u) renderer->add_object(&g_SCD_CAVIARDATA, 0);
        if (bonus & 0x400u) renderer->add_object(&g_SAGS_CAVIARDATA, 0);
        if (bonus & 0x8000u) renderer->add_object(&g_VHR_CAVIARDATA1, 0);
        if (bonus & 0x200000u) renderer->add_object(&g_SRB_CAVIARDATA, 0);
        if (bonus & 0x10u) renderer->add_object(&g_SDP_CAVIARDATA, 0);
        if (bonus & 0x200u) renderer->add_object(&g_SCJ_CAVIARDATA, 0);
        if (bonus & 0x40000u) renderer->add_object(&g_SHT_CAVIARDATA, 0);
    }
    // Bonus parts and explicitly selected parts are not deduplicated.
    if (abilities & 0x1000000u) renderer->add_object(&g_SP_SOPORIFIC_CAVIARDATA, 0);
    if (abilities & 0x2000000u) renderer->add_object(&g_SP_DISSWAVE_CAVIARDATA, 0);
    if (abilities & 0x8000000u) renderer->add_object(&g_SP_NANOO_CAVIARDATA, 0);
    if (abilities & 0x4000000u) renderer->add_object(&g_SP_MARINED_CAVIARDATA, 0);
    if (Players[faction_id].rule_flags & RFLAG_ALIEN) {
        if (abilities & 1u) renderer->add_object(&g_SSFA_CAVIARDATA, 0);
        if (abilities & 0x2000u) renderer->add_object(&g_SFTA_CAVIARDATA, 0);
    } else {
        if (abilities & 1u) renderer->add_object(&g_SSF_CAVIARDATA, 0);
        if (abilities & 0x2000u) renderer->add_object(&g_SFT_CAVIARDATA, 0);
    }
    if (abilities & 0x20u) renderer->add_object(&g_SAS_CAVIARDATA, 0);
    if (abilities & 0x80u) renderer->add_object(&g_SCD_CAVIARDATA, 0);
    if (abilities & 0x400u) renderer->add_object(&g_SAGS_CAVIARDATA, 0);
    if (abilities & 0x8000u) renderer->add_object(&g_VHR_CAVIARDATA1, 0);
    if (abilities & 0x200000u) renderer->add_object(&g_SRB_CAVIARDATA, 0);
    if (abilities & 0x10u) renderer->add_object(&g_SDP_CAVIARDATA, 0);
    if (abilities & 0x200u) renderer->add_object(&g_SCJ_CAVIARDATA, 0);
    if (abilities & 0x40000u) renderer->add_object(&g_SHT_CAVIARDATA, 0);
}

// ORIGINAL: 0x004C3800 ?vehdraw_construct_reactor@@YAHHHHHHPAUCaviar@@@Z 0x004C3800-0x004C3896 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original returns before array access when reactor is zero. VIPTR, VPBR0 and VR operands are biased by one12-byte model and therefore mean reactor-minus-one into existing arrays.", "Infantry and chassis8 return after their selected model; only the general reactor case tests ability0x8000 and adds VHR2. Original4C387D reloads flags before exit, so seed returned add_object values were invented."], "reason": "Recover reactor attachments through source-owned model arrays and infantry placement."}
// symbol    ?vehdraw_construct_reactor@@YAXHHHHIPAVCaviar@@@Z
void __cdecl vehdraw_construct_reactor(int faction_id, int chassis, int weapon,
                                      int reactor, uint32_t abilities, Caviar *renderer) {
    UNUSED(faction_id);
    if (reactor == 0) return;
    if (chassis == 0 && (Weapon[weapon].mode < 3 || Weapon[weapon].mode == 11)) {
        renderer->add_object(&g_VIPTR_CAVIARDATA[reactor - 1], &VehInfantryOffset, 0);
        return;
    }
    if (chassis == 8) {
        renderer->add_object(&g_VPBR0_CAVIARDATA[reactor - 1], 0);
        return;
    }
    renderer->add_object(&g_VR_CAVIARDATA[reactor - 1], 0);
    if (abilities & 0x8000u) renderer->add_object(&g_VHR_CAVIARDATA2, 0);
}

// ORIGINAL: 0x004C2360 ?vehdraw_construct_vehicle@@YAHHHHHHHPAUCaviar@@@Z 0x004C2360-0x004C246D
// RECOVERY_BINDING: v1 {"evidence": ["Original4C23BC loads argument6 as ability flags and pushes faction as the ability helper final argument. The seed swapped these inputs. Original signed armor-defense comparison may substitute infantry for a supply crawler.", "Reactor selection follows all component builders and therefore observes current weapon mode. Original void exits and all mapped callers support no returned value; the legacy catalogue name remains provenance."], "reason": "Recover explicit vehicle component assembly through real chassis, weapon, ability, armor and reactor operations."}
// symbol    ?vehdraw_construct_vehicle@@YAXHHHHHIPAVCaviar@@@Z
void __cdecl vehdraw_construct_vehicle(int faction_id, int chassis, int weapon,
                                      int armor, int reactor, uint32_t abilities,
                                      Caviar *renderer) {
    renderer->reset_objects();
    if (weapon == 21 && chassis == 1 && Armor[armor].defense_rating <= 1) chassis = 0;
    vehdraw_construct_chassis(faction_id, chassis, weapon, armor, reactor, renderer);
    vehdraw_construct_weapon(faction_id, chassis, weapon, renderer);
    vehdraw_construct_abil(abilities, renderer, faction_id);
    vehdraw_construct_armor(faction_id, chassis, weapon, armor, 0, renderer);
    // This operation reads Weapon.mode after the component builders.
    vehdraw_construct_reactor(faction_id, chassis, weapon, reactor, abilities, renderer);
}

// ORIGINAL: 0x004C1D20 ?vehdraw_construct_vehicle@@YAHHPAUCaviar@@@Z 0x004C1D20-0x004C202D
// RECOVERY_BINDING: v1 {"evidence": ["Original resets renderer count before scaling and reloads prototype identity after morale calculation. Ordinary fields are byte-zero-extended, armor defense uses signed comparison, and faction comes from the current vehicle. Native special prototypes 8/9/10 clamp morale before scaling and choose their actual animated model.", "The repeated ordinary final reactor selection is expressed through the genuine shared reactor operation. Catalogued integer return is not consumed by mapped callers; original exits expose incidental register contents, not a supported result."], "reason": "Recover vehicle model assembly from genuine vehicle/prototype state and loaded CaviarData owners."}
// symbol    ?vehdraw_construct_vehicle@@YAXHPAVCaviar@@@Z
void __cdecl vehdraw_construct_vehicle(int veh_id, Caviar *renderer) {
    renderer->reset_objects();
    int proto_id = Vehs[veh_id].proto_id;
    const int initial_chassis = VehPrototypes[proto_id].chassis_id;
    if (initial_chassis == 0) {
        const int weapon = VehPrototypes[proto_id].weapon_id;
        renderer->set_scaling(renderer->get_scaling() *
            ((weapon < 3 || weapon == 21) ? 1.25f : 0.8f));
    } else if (initial_chassis == 1 && VehPrototypes[proto_id].weapon_id == 21) {
        renderer->set_scaling(renderer->get_scaling() * 1.25f);
    }
    int morale;
    if (Vehs[veh_id].faction_id != 0) morale = morale_veh(veh_id, 0, 0);
    else morale = static_cast<int>(morale_alien(veh_id, ConsoleGlobal.faction_id_));
    // Morale calculation may change the vehicle/prototype state.
    proto_id = Vehs[veh_id].proto_id;
    if (proto_id == 8 || proto_id == 9 || proto_id == 10) {
        const int maximum = proto_id == 10 ? 7 : 6;
        if (morale < 0) morale = 0;
        else if (morale > maximum) morale = maximum;
        const int base = proto_id == 8 ? 12 : (proto_id == 9 ? 6 : 5);
        renderer->set_scaling(static_cast<float>(
            static_cast<double>(renderer->get_scaling()) * (morale + base) * 0.125f));
        if (proto_id == 8)
            renderer->add_object(&g_NW_CAVIARDATA, VehDrawAnimationStep % 5 + 30);
        else if (proto_id == 9)
            renderer->add_object(&g_NI_CAVIARDATA, VehDrawAnimationStep % 5 + 30);
        else
            renderer->add_object(&g_NLC_CAVIARDATA, VehDrawAnimationStep);
        return;
    }
    const uint32_t abilities = VehPrototypes[proto_id].ability_flags;
    const int reactor = VehPrototypes[proto_id].reactor_id;
    const int weapon = VehPrototypes[proto_id].weapon_id;
    int chassis = VehPrototypes[proto_id].chassis_id;
    const int armor = VehPrototypes[proto_id].armor_id;
    const int faction_id = Vehs[veh_id].faction_id;
    renderer->reset_objects();
    if (weapon == 21 && chassis == 1 && Armor[armor].defense_rating <= 1) chassis = 0;
    vehdraw_construct_chassis(faction_id, chassis, weapon, armor, reactor, renderer);
    vehdraw_construct_weapon(faction_id, chassis, weapon, renderer);
    vehdraw_construct_abil(abilities, renderer, faction_id);
    vehdraw_construct_armor(faction_id, chassis, weapon, armor, 0, renderer);
    vehdraw_construct_reactor(faction_id, chassis, weapon, reactor, abilities, renderer);
}

// ORIGINAL: 0x004C2470 ?vehdraw_construct_chassis@@YAHHHHHHPAUCaviar@@@Z 0x004C2470-0x004C2C87
// RECOVERY_BINDING: v1 {"evidence": ["Original bounds faction only for chassis tint selection, uses signed armor defense, and selects real reactor-minus-one model arrays. Utility infantry falls through the shared add at4C27AA and also adds VLIGHTS at4C27B8, omitted by the seed.", "Original rover steps and aircraft faction flags are reloaded at call boundaries. Crawler assembly uses signed step remainder then queries the genuine DesignWin Win base at4C2BE9 and increments the current animation storage."], "reason": "Recover chassis model ownership, tint selection and animation through genuine renderer operations."}
// symbol    ?vehdraw_construct_chassis@@YAXHHHHHPAVCaviar@@@Z
void __cdecl vehdraw_construct_chassis(int faction_id, int chassis, int weapon,
                                      int armor, int reactor, Caviar *renderer) {
    if (faction_id < 0 || faction_id > 7) faction_id = 0;
    unsigned char *body_colors = &BodyTintColorTables[faction_id][0][0];
    unsigned char *armor_colors = 0;
    int armor_grade = 0;
    switch (armor) {
    case 1: case 5: armor_grade = 1; break;
    case 2: case 10: case 11: armor_grade = 1; armor_colors = &ArmorTintColorTables[0][0][0]; break;
    case 3: armor_grade = 1; armor_colors = &ArmorTintColorTables[1][0][0]; break;
    case 6: case 12: case 13: armor_grade = 1; armor_colors = &ArmorTintColorTables[2][0][0]; break;
    case 7: armor_grade = 1; armor_colors = &ArmorTintColorTables[3][0][0]; break;
    case 4: case 8: armor_grade = 2; break;
    }
    if (armor == 10 || armor == 12) renderer->add_object(&g_SCJ_CAVIARDATA, 0);
    if (armor == 11 || armor == 13) renderer->add_object(&g_SHT_CAVIARDATA, 0);
    if (weapon == 21 && chassis == 1 && Armor[armor].defense_rating <= 1) chassis = 0;
    if (chassis < 0 || chassis > 8) return;

    if ((chassis == 1 || chassis == 3 || chassis == 5 || chassis == 6 || chassis == 7)
        && reactor != 0) {
        if (Players[faction_id].rule_flags & RFLAG_ALIEN) {
            if (armor_colors)
                renderer->add_object(&g_ACP_CAVIARDATA[reactor - 1], armor_colors, 0);
            else renderer->add_object(&g_ACP_CAVIARDATA[reactor - 1], 0);
        } else renderer->add_object(&g_VRC_CAVIARDATA[reactor - 1], 0);
    }
    switch (chassis) {
    case 0:
        if (Weapon[weapon].mode < 3) {
            if (weapon == 17) return;
            CaviarData *model = (Players[faction_id].rule_flags & RFLAG_ALIEN)
                ? &g_AX_CAVIARDATA : &g_VI_CAVIARDATA;
            renderer->add_object(model, &VehInfantryOffset, VehDrawAnimationStep % 15);
        } else if (weapon == 21) {
            if (Players[faction_id].rule_flags & RFLAG_ALIEN)
                renderer->add_object(&g_AX_CAVIARDATA, &ArmorTintColorTables[3][0][0],
                                     &VehInfantryOffset, VehDrawAnimationStep % 15);
            else renderer->add_object(&g_VPT_CAVIARDATA, &VehInfantryOffset,
                                      VehDrawAnimationStep % 15);
        } else if (weapon != 22 &&
                   (!(Players[faction_id].rule_flags & RFLAG_ALIEN) || weapon != 17)) {
            renderer->add_object(&g_VB_CAVIARDATA, 0);
            renderer->add_object(&g_VBP_CAVIARDATA, body_colors, 0);
            renderer->add_object(&g_VCL_CAVIARDATA, 0);
            renderer->add_object(&g_VCLT00_CAVIARDATA, 0);
            renderer->add_object(&g_VLIGHTS_CAVIARDATA, 0);
        }
        return;
    case 1: case 3:
        renderer->add_object(&g_VGMC_CAVIARDATA, 0);
        renderer->add_object(&g_VGMCP_CAVIARDATA, body_colors, 0);
        renderer->add_object(&g_VB_CAVIARDATA, 0);
        renderer->add_object(&g_VBP_CAVIARDATA, body_colors, 0);
        if (chassis == 3) { renderer->add_object(&g_VFL_CAVIARDATA, 0); return; }
        renderer->add_object(&g_VSP_CAVIARDATA, 0);
        renderer->add_object(&g_VSPTB_CAVIARDATA, VehDrawAnimationStep);
        renderer->add_object(&g_VSPTF_CAVIARDATA, VehDrawAnimationStep);
        renderer->add_object(&g_VLIGHTS_CAVIARDATA, 0);
        return;
    case 2:
        renderer->add_object(&g_VB_CAVIARDATA, 0);
        renderer->add_object(&g_VHT_VBP_CAVIARDATA, body_colors, 0);
        renderer->add_object(&g_VHTP_CAVIARDATA, body_colors, 0);
        renderer->add_object(&g_VHTTP_CAVIARDATA, body_colors, 0);
        if (armor_grade == 0) {
            renderer->add_object(&g_VHTA0_CAVIARDATA[0], 0);
            renderer->add_object(&g_VHTTPA0_CAVIARDATA[0], 0);
        }
        renderer->add_object(&g_VLIGHTS_CAVIARDATA, &VehHoverLightsOffset, 0);
        return;
    case 4: case 5: case 6: case 7:
        renderer->add_object(&g_VB_CAVIARDATA, 0);
        renderer->add_object(&g_VBP_CAVIARDATA, body_colors, 0);
        renderer->add_object(&g_VGMC_CAVIARDATA, 0);
        renderer->add_object(&g_VGMCP_CAVIARDATA, body_colors, 0);
        if (chassis == 4) {
            renderer->add_object(&g_VCU_CAVIARDATA, 0);
            renderer->add_object(&g_VCUP_CAVIARDATA, body_colors, 0);
            renderer->add_object(&g_VCUW_CAVIARDATA, 0);
            if (armor_grade == 0) renderer->add_object(&g_VCUA0_CAVIARDATA[0], 0);
        } else if (chassis == 5) {
            renderer->add_object(&g_VJTP_CAVIARDATA, body_colors, 0);
            if (armor_grade == 0)
                renderer->add_object((Players[faction_id].rule_flags & RFLAG_ALIEN)
                    ? &g_AA_WING_CAVIARDATA[0] : &g_VJT0_CAVIARDATA[0], 0);
        } else if (chassis == 6) {
            renderer->add_object(&g_VCT_CAVIARDATA, 0);
            renderer->add_object(&g_VCTP_CAVIARDATA, body_colors, 0);
            renderer->add_object(&g_VCTB_CAVIARDATA, VehDrawAnimationStep % 15);
            if (TutWinDesWindow.is_visible())
                VehDrawAnimationStep = static_cast<int>(static_cast<uint32_t>(VehDrawAnimationStep) + 1u);
            if (armor_grade == 0) renderer->add_object(&g_VCT0_CAVIARDATA[0], body_colors, 0);
        } else {
            renderer->add_object(&g_VGS_CAVIARDATA, 0);
            renderer->add_object(&g_VGSP_CAVIARDATA, body_colors, 0);
        }
        return;
    case 8:
        if (weapon == 16) renderer->add_object(&g_VM13_CAVIARDATA, 0);
        else {
            renderer->add_object(&g_VM_CAVIARDATA, 0);
            if (weapon == 24) renderer->add_object(&g_TECLOAD_CAVIARDATA, 0);
            else if (weapon == 25) renderer->add_object(&g_FUNLOAD_CAVIARDATA, 0);
        }
        renderer->add_object(&g_VB_CAVIARDATA, 0);
        renderer->add_object(&g_VBP_CAVIARDATA, body_colors, 0);
    }
}

// ORIGINAL: 0x004C2CF0 ?vehdraw_construct_weapon@@YAHHHHPAUCaviar@@@Z 0x004C2CF0-0x004C306A
// RECOVERY_BINDING: v1 {"evidence": ["Original allocates and initializes a26-entry automatic model lookup, not static storage. Offense minus1 and99 have distinct models; bounded other values include a no-model sentinel. Special weapons/chassis return before ordinary faction/rule work.", "Original terraform handling rereads faction flags after the first model addition. Gun, hover-gun and drop offsets are the actual vectors populated by load_voxels, not old executable addresses."], "reason": "Recover weapon model selection and attachments through real model arrays, tint tables and vector offsets."}
// symbol    ?vehdraw_construct_weapon@@YAXHHHPAVCaviar@@@Z
void __cdecl vehdraw_construct_weapon(int faction_id, int chassis, int weapon, Caviar *renderer) {
    const int model_indices[26] = {-1,-1,0,0,1,3,2,2,4,4,5,5,6,7,7,7,8,8,8,8,9,9,9,9,10,13};
    if (chassis == 8) return;
    if (weapon == 16) { renderer->add_object(&g_VM13_CAVIARDATA, 0); return; }
    if (weapon == 23 || weapon == 24 || weapon == 25) {
        renderer->add_object(&g_VM_CAVIARDATA, 0);
        if (weapon == 24) renderer->add_object(&g_TECLOAD_CAVIARDATA, 0);
        else if (weapon == 25) renderer->add_object(&g_FUNLOAD_CAVIARDATA, 0);
        return;
    }
    int offense = Weapon[weapon].offense_rating;
    if (faction_id < 0 || faction_id > 7) faction_id = 0;
    unsigned char *body_colors = &BodyTintColorTables[faction_id][0][0];
    const int mode = Weapon[weapon].mode;
    if (mode < 3) {
        int model;
        if (offense == -1) model = 11;
        else if (offense == 99) model = 12;
        else {
            if (offense < 0) offense = 0;
            else if (offense > 25) offense = 25;
            model = model_indices[offense];
        }
        if (model != -1) {
            CaviarData *definition = weapon == 12 ? &g_RESLASER_CAVIARDATA : &g_VW_CAVIARDATA[model];
            if (chassis == 0) {
                renderer->add_object(definition, &VehGunMountOffset, 0);
                renderer->add_object(&g_VGMT_CAVIARDATA, 0);
                renderer->add_object(&g_VGMTP_CAVIARDATA, body_colors, 0);
            } else if (chassis == 2) renderer->add_object(definition, &VehHoverGunOffset, 0);
            else renderer->add_object(definition, 0);
        }
    } else {
        switch (mode) {
        case 7: renderer->add_object(&g_VWNTT_CAVIARDATA, 0); break;
        case 8:
            if (chassis != 0) renderer->add_object(&g_DROPLET_CAVIARDATA, &VehDropOffset, 0);
            else if (Players[faction_id].rule_flags & RFLAG_ALIEN)
                renderer->add_object(&g_ACOLPOD_CAVIARDATA, VehDrawAnimationStep % 15);
            else {
                renderer->add_object(&g_DROP_CAVIARDATA, 0);
                renderer->add_object(&g_DROPLET_CAVIARDATA, 0);
            }
            break;
        case 9:
            if (chassis == 0)
                renderer->add_object((Players[faction_id].rule_flags & RFLAG_ALIEN)
                    ? &g_VTA_CAVIARDATA : &g_VT_CAVIARDATA, 0);
            // The first add can change faction state; this is a fresh test.
            if (Players[faction_id].rule_flags & RFLAG_ALIEN) {
                renderer->add_object(&g_AT_CAVIARDATA, 0);
                renderer->add_object(&g_ATP_CAVIARDATA, 0);
            } else renderer->add_object(&g_VWNTU_CAVIARDATA, 0);
            break;
        case 10: renderer->add_object(&g_VWNST_CAVIARDATA, 0); break;
        case 11: if (chassis != 0) renderer->add_object(&g_PTMOD_CAVIARDATA, 0); break;
        case 12: renderer->add_object(&g_VWNAA_CAVIARDATA, 0); break;
        }
    }
    if (weapon == 13) {
        if (chassis == 0) renderer->add_object(&g_RESBOLT_CAVIARDATA, &VehGunMountOffset, 0);
        else if (chassis == 2) renderer->add_object(&g_RESBOLT_CAVIARDATA, &VehHoverGunOffset, 0);
        else renderer->add_object(&g_RESBOLT_CAVIARDATA, 0);
    }
}

// ORIGINAL: 0x004C3090 ?vehdraw_construct_armor@@YAHHHHHHPAUCaviar@@@Z 0x004C3090-0x004C3795
// RECOVERY_BINDING: v1 {"evidence": ["Original special and ordinary armor cases are separate; transparent armor5 uses the first constant-color ramp. Reactor-independent model arrays are indexed from their actual starts rather than biased original address expressions.", "Original crawler armor adds its crawler model followed by the human/alien body armor, with the same selected tint on both. Do not clamp faction here: unlike chassis and weapon, the original does not."], "reason": "Recover armor overlays, tint and ordered body-specific pairs using actual model owners."}
// symbol    ?vehdraw_construct_armor@@YAXHHHHHPAVCaviar@@@Z
void __cdecl vehdraw_construct_armor(int faction_id, int chassis, int weapon,
                                    int armor, int transparent, Caviar *renderer) {
    bool special = false;
    bool ordinary = false;
    unsigned char *colors = 0;
    switch (armor) {
    case 1: ordinary = true; break;
    case 2: case 10: case 11: ordinary = true; colors = &ArmorTintColorTables[0][0][0]; break;
    case 3: ordinary = true; colors = &ArmorTintColorTables[1][0][0]; break;
    case 6: case 12: case 13: ordinary = true; colors = &ArmorTintColorTables[2][0][0]; break;
    case 7: ordinary = true; colors = &ArmorTintColorTables[3][0][0]; break;
    case 5:
        if (transparent) { ordinary = true; colors = &ConstantColorTables[0][0]; }
        else special = true;
        break;
    case 4: case 8: case 9:
        if (chassis == 0 && (Weapon[weapon].mode < 3 || Weapon[weapon].mode == 11)) {
            CaviarData *model = armor == 4 ? &g_VIPTAWL_CAVIARDATA :
                (armor == 8 ? &g_VIPTASGN_CAVIARDATA : &g_VIPTAPSI_CAVIARDATA);
            renderer->add_object(model, &VehInfantryOffset, VehDrawAnimationStep);
        } else {
            renderer->add_object(&g_A_CAVIARDATA, 0);
            renderer->add_object(armor == 4 ? &g_APWALL_CAVIARDATA :
                (armor == 8 ? &g_ASGEN_CAVIARDATA : &g_APSID_CAVIARDATA), 0);
        }
        special = true;
        break;
    }
    CaviarData *first = 0;
    CaviarData *second = 0;
    switch (chassis) {
    case 0:
        if (special || !ordinary) return;
        if (weapon != -1 && (Weapon[weapon].mode < 3 || Weapon[weapon].mode == 11)) {
            first = (Players[faction_id].rule_flags & RFLAG_ALIEN)
                ? &g_AA_CAVIARDATA : &g_VIPTA_CAVIARDATA[0];
            if (colors) renderer->add_object(first, colors, &VehInfantryOffset, VehDrawAnimationStep % 15);
            else renderer->add_object(first, &VehInfantryOffset, VehDrawAnimationStep % 15);
            return;
        }
        first = (Players[faction_id].rule_flags & RFLAG_ALIEN) ? &g_AA01_CAVIARDATA : &g_VA01_CAVIARDATA;
        break;
    case 1:
        if (special || !ordinary) return;
        if (Players[faction_id].rule_flags & RFLAG_ALIEN) {
            first = &g_AA01_CAVIARDATA; second = &g_AA_ROVER_CAVIARDATA;
        } else { first = &g_VA01_CAVIARDATA; second = &g_VSPA0_CAVIARDATA[0]; }
        break;
    case 2:
        if (special) { first = &g_VHTA0_CAVIARDATA[0]; second = &g_VHTTPA0_CAVIARDATA[0]; }
        else if (ordinary) { first = &g_VHTA0_CAVIARDATA[1]; second = &g_VHTTPA0_CAVIARDATA[1]; }
        break;
    case 3:
        if (special || !ordinary) return;
        first = (Players[faction_id].rule_flags & RFLAG_ALIEN) ? &g_AA01_CAVIARDATA : &g_VA01_CAVIARDATA;
        break;
    case 4:
        if (special) {
            if (armor != 5) first = &g_VCUA0_CAVIARDATA[0];
        } else if (ordinary) first = &g_VCUA0_CAVIARDATA[1];
        break;
    case 5:
        if (special) {
            if (armor != 5) first = (Players[faction_id].rule_flags & RFLAG_ALIEN)
                ? &g_AA_WING_CAVIARDATA[0] : &g_VJT0_CAVIARDATA[0];
        } else if (ordinary) {
            if (Players[faction_id].rule_flags & RFLAG_ALIEN) {
                first = &g_AA01_CAVIARDATA; second = &g_AA_WING_CAVIARDATA[1];
            } else { first = &g_VA01_CAVIARDATA; second = &g_VJT0_CAVIARDATA[1]; }
        }
        break;
    case 6:
        if (special) {
            if (armor == 5) return;
            first = &g_VCT0_CAVIARDATA[0];
        } else {
            if (!ordinary) return;
            first = &g_VCT0_CAVIARDATA[1];
        }
        second = (Players[faction_id].rule_flags & RFLAG_ALIEN) ? &g_AA01_CAVIARDATA : &g_VA01_CAVIARDATA;
        break;
    case 7:
        if (!ordinary) return;
        first = (Players[faction_id].rule_flags & RFLAG_ALIEN) ? &g_AA01_CAVIARDATA : &g_VA01_CAVIARDATA;
        break;
    case -1:
        if (!ordinary || special) return;
        first = (Players[faction_id].rule_flags & RFLAG_ALIEN) ? &g_AA01_CAVIARDATA : &g_VA01_CAVIARDATA;
        break;
    }
    if (first) {
        if (colors) renderer->add_object(first, colors, 0);
        else renderer->add_object(first, 0);
    }
    if (second) {
        if (colors) renderer->add_object(second, colors, 0);
        else renderer->add_object(second, 0);
    }
}

// Turn setup publishes these for the subsequent animation frames.
// RECOVERY_OBJECT: v2 0x008CC820 0x008CC824 ?VehDrawTurnVehicle@@3HA 0x004BF8D0
int VehDrawTurnVehicle;
// RECOVERY_OBJECT: v2 0x008CC260 0x008CC264 ?VehDrawTurnAngle@@3MA 0x004BF8D0
float VehDrawTurnAngle;
// RECOVERY_OBJECT: v2 0x008CC258 0x008CC25C ?VehDrawTurnIncrement@@3MA 0x004BF8D0
float VehDrawTurnIncrement;

// These are independent animation globals, not fields beyond the renderer's
// complete-object boundary. Scoot setup publishes two normal increments and
// their respective origins; each frame interpolates only its selected half.
// RECOVERY_OBJECT: v2 0x008CC25C 0x008CC260 ?VehDrawScootFacing@@3HA 0x004BFE00
int VehDrawScootFacing;
// RECOVERY_OBJECT: v2 0x008CDCF8 0x008CDCFC ?VehDrawScootSteps@@3HA 0x004BFE00
int VehDrawScootSteps;
// RECOVERY_OBJECT: v2 0x008CC268 0x008CC274 ?VehDrawScootFirstIncrement@@3UVOX_Vect@@A 0x004BFE00
VOX_Vect VehDrawScootFirstIncrement;
// RECOVERY_OBJECT: v2 0x008CC278 0x008CC284 ?VehDrawScootSecondIncrement@@3UVOX_Vect@@A 0x004BFE00
VOX_Vect VehDrawScootSecondIncrement;
// RECOVERY_OBJECT: v2 0x008CC288 0x008CC294 ?VehDrawScootNormalMid@@3UVOX_Vect@@A 0x004BFE00
VOX_Vect VehDrawScootNormalMid;
// RECOVERY_OBJECT: v2 0x008CDD00 0x008CDD0C ?VehDrawScootNormalStart@@3UVOX_Vect@@A 0x004BFE00
VOX_Vect VehDrawScootNormalStart;
// Temporary draw coordinates belong to the vehicle rendering state; -1 means
// no override. veh_draw uses them only when the selected vehicle also agrees.
// RECOVERY_OBJECT: v2 0x0068E564 0x0068E568 ?VehDrawPositionX@@3HA 0x004C12A0
int VehDrawPositionX = -1;
// RECOVERY_OBJECT: v2 0x0068E568 0x0068E56C ?VehDrawPositionY@@3HA 0x004C12A0
int VehDrawPositionY = -1;
// RECOVERY_OBJECT: v2 0x0068E56C 0x0068E570 ?VehDrawPositionVehicle@@3HA 0x004C12A0
int VehDrawPositionVehicle = -1;

static float vehdraw_scale(int zoom) {
    if (MapWinCaptionScale > 0) {
        return MapWinCaptionScale * 0.07f / MapWinCaptionDivisor;
    }
    return (zoom + 16) * 0.07f * 0.0625f;
}

static int vehdraw_centre_offset(int numerator, int zoom) {
    if (numerator > 0) {
        return (numerator * 50) / MapWinCaptionDivisor;
    }
    return ((zoom * 5 + 80) * 10) / 16;
}

static bool vehdraw_uses_terrain(int veh_id, int x, int y) {
    const Map *tiles = map_tiles();
    if ((tiles[y * MapLongitude + (x >> 1)].climate & 0xE0) < 0x60) {
        return false;
    }
    const VehPrototype &prototype = VehPrototypes[Vehs[veh_id].proto_id];
    return prototype.chassis_id != 0 || Weapon[prototype.weapon_id].mode >= 3;
}

static int vehdraw_facet_index(const int *h1, const int *h2,
                               const int *h3, const int *h4, int cell) {
    return ((h1[cell] * 7 + h2[cell]) * 7 + h3[cell]) * 7 + h4[cell] - 400;
}

static void vehdraw_average_facets(int index, VOX_Vect *normal) {
    // Copy every input before the first call, as the original does.
    const VOX_Vect n0 = VehDrawFacetNormals0[index];
    const VOX_Vect n1 = VehDrawFacetNormals1[index];
    const VOX_Vect n2 = VehDrawFacetNormals2[index];
    const VOX_Vect n3 = VehDrawFacetNormals3[index];
    vector_add(n0.values, n1.values, normal->values);
    vector_add(normal->values, n2.values, normal->values);
    vector_add(normal->values, n3.values, normal->values);
    vox_normalise_vector(normal->values, normal->values);
}

static void vehdraw_finish_frame(Buffer *buffer, int veh_id, int x, int y, int flags) {
    // Rendering can change the selected prototype; fetch it after render.
    const VehPrototype &prototype = VehPrototypes[Vehs[veh_id].proto_id];
    if (prototype.armor_id == 5) {
        g_VEHDRAW_CAVIAR.reset_objects();
        vehdraw_construct_armor(Vehs[veh_id].faction_id, prototype.chassis_id,
                               prototype.weapon_id, 5, 1, &g_VEHDRAW_CAVIAR);
        g_VEHDRAW_CAVIAR.render_multi_table_dest(buffer, x, y, 24,
            reinterpret_cast<unsigned char *>(&ArmorTransparentColorTables), flags);
    } else {
        g_VEHDRAW_CAVIAR.clear_buffers(0);
    }
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004BFC70 ?vehdraw_turn@@YAXPAUBuffer@@HHHH@Z 0x004BFC70-0x004BFDFA
// RECOVERY_BINDING: v1 {"evidence": ["Original selects scale, rotates scene from frame/turn increment, computes caption-centred coordinates, resets and assembles the actual vehicle, renders, then reloads its prototype for transparent-armor overlay or buffer clearing.", "Original mapped extent is preserved from the existing unclaimed seed and checked against the pinned executable; catalogue return/type names remain provenance until observed compiler-symbol facts are published."], "reason": "Recover a vehicle turn frame through the real renderer and shared turn state."}
// symbol    ?vehdraw_turn@@YAXPAVBuffer@@HHHH@Z
void __cdecl vehdraw_turn(Buffer *buffer, int x, int y, int frame, int zoom) {
    const int veh_id = VehDrawTurnVehicle;
    g_VEHDRAW_CAVIAR.set_scaling(vehdraw_scale(zoom));
    g_VEHDRAW_CAVIAR.set_scene_rotation(
        frame * VehDrawTurnIncrement + VehDrawTurnAngle - VehDrawCameraAngles.values[0],
        0.0f, 0.0f);
    const int numerator = MapWinCaptionScale;
    x += vehdraw_centre_offset(numerator, zoom);
    y += vehdraw_centre_offset(numerator, zoom);
    g_VEHDRAW_CAVIAR.reset_objects();
    vehdraw_construct_vehicle(veh_id, &g_VEHDRAW_CAVIAR);
    g_VEHDRAW_CAVIAR.render(buffer, x, y, 1);
    vehdraw_finish_frame(buffer, veh_id, x, y, 0);
}

// ORIGINAL: 0x004BFE00 ?vehdraw_scoot_start@@YAXHHHH@Z 0x004BFE00-0x004C0F24
// RECOVERY_BINDING: v1 {"evidence": ["Original samples four nine-element altitude arrays, including the zero-delta centre; it snapshots original facet vectors before additions. Even directions combine destination facet3 and centre facet1; odd directions use eight ordered facets. Both interpolation increments divide by their respective half-frame counts.", "Original mapped extent is preserved from the existing unclaimed seed and checked against the pinned executable; catalogue return/type names remain provenance until observed compiler-symbol facts are published."], "reason": "Recover terrain-normal interpolation setup using nine actual terrain samples and independent animation globals."}
// symbol    ?vehdraw_scoot_start@@YAXHHHH@Z
void __cdecl vehdraw_scoot_start(int veh_id, int direction, int steps, int flags) {
    if (flags & 1) veh_id = pick_top_veh(veh_id);
    VehDrawScootSteps = steps;
    VehDrawTurnVehicle = veh_id;
    VehDrawScootFacing = (direction - 3) & 7;

    // The ninth sample is the centre. The four original stack runs each
    // occupy nine integers, including the values used for the starting tile.
    int h1[9], h2[9], h3[9], h4[9];
    int x, y;
    for (int cell = 0; cell < 9; ++cell) {
        x = xrange(VehMoveDeltaX[cell] + VehLiftX);
        y = VehMoveDeltaY[cell] + VehLiftY;
        h1[cell] = ConsoleGlobal.get_relative_alt(x, y, 1);
        h2[cell] = ConsoleGlobal.get_relative_alt(x, y, 2);
        h3[cell] = ConsoleGlobal.get_relative_alt(x, y, 3);
        h4[cell] = ConsoleGlobal.get_relative_alt(x, y, 4);
    }
    VOX_Vect start, end, midpoint;
    if (vehdraw_uses_terrain(veh_id, x, y)) {
        vehdraw_average_facets(vehdraw_facet_index(h1, h2, h3, h4, 8), &start);
    } else {
        start.values[0] = start.values[1] = 0.0f;
        start.values[2] = 1.0f;
        midpoint = start;
    }
    x = VehMoveDeltaX[direction] + VehLiftX;
    VehDrawAnimationStep = 1;
    x = xrange(x);
    y = VehMoveDeltaY[direction] + VehLiftY;
    if (vehdraw_uses_terrain(veh_id, x, y)) {
        vehdraw_average_facets(vehdraw_facet_index(h1, h2, h3, h4, direction), &end);
    } else {
        end.values[0] = end.values[1] = 0.0f;
        end.values[2] = 1.0f;
    }
    // Re-read terrain/prototype after the normalizer calls. The midpoint is
    // the shared edge (even directions) or the eight meeting facets (odd).
    if (!vehdraw_uses_terrain(veh_id, x, y)) {
        midpoint.values[0] = midpoint.values[1] = 0.0f;
        midpoint.values[2] = 1.0f;
    } else {
        const int centre = vehdraw_facet_index(h1, h2, h3, h4, 8);
        VOX_Vect n[8];
        int count = 0;
        switch (direction) {
        case 0: case 2: case 4: case 6:
            n[0] = VehDrawFacetNormals3[vehdraw_facet_index(h1, h2, h3, h4, direction)];
            n[1] = VehDrawFacetNormals1[centre];
            count = 2;
            break;
        case 1: {
            const int a = vehdraw_facet_index(h1, h2, h3, h4, 0);
            const int b = vehdraw_facet_index(h1, h2, h3, h4, 1);
            const int c = vehdraw_facet_index(h1, h2, h3, h4, 2);
            n[0] = VehDrawFacetNormals1[centre]; n[1] = VehDrawFacetNormals2[centre];
            n[2] = VehDrawFacetNormals2[a]; n[3] = VehDrawFacetNormals3[a];
            n[4] = VehDrawFacetNormals3[b]; n[5] = VehDrawFacetNormals0[b];
            n[6] = VehDrawFacetNormals0[c]; n[7] = VehDrawFacetNormals1[c];
            count = 8;
            break;
        }
        case 3: {
            const int a = vehdraw_facet_index(h1, h2, h3, h4, 2);
            const int b = vehdraw_facet_index(h1, h2, h3, h4, 3);
            const int c = vehdraw_facet_index(h1, h2, h3, h4, 4);
            n[0] = VehDrawFacetNormals2[centre]; n[1] = VehDrawFacetNormals3[centre];
            n[2] = VehDrawFacetNormals3[a]; n[3] = VehDrawFacetNormals0[a];
            n[4] = VehDrawFacetNormals0[b]; n[5] = VehDrawFacetNormals1[b];
            n[6] = VehDrawFacetNormals1[c]; n[7] = VehDrawFacetNormals2[c];
            count = 8;
            break;
        }
        case 5: {
            const int a = vehdraw_facet_index(h1, h2, h3, h4, 4);
            const int b = vehdraw_facet_index(h1, h2, h3, h4, 5);
            const int c = vehdraw_facet_index(h1, h2, h3, h4, 6);
            n[0] = VehDrawFacetNormals3[centre]; n[1] = VehDrawFacetNormals0[centre];
            n[2] = VehDrawFacetNormals0[a]; n[3] = VehDrawFacetNormals1[a];
            n[4] = VehDrawFacetNormals1[b]; n[5] = VehDrawFacetNormals2[b];
            n[6] = VehDrawFacetNormals2[c]; n[7] = VehDrawFacetNormals3[c];
            count = 8;
            break;
        }
        case 7: {
            const int a = vehdraw_facet_index(h1, h2, h3, h4, 6);
            const int b = vehdraw_facet_index(h1, h2, h3, h4, 7);
            const int c = vehdraw_facet_index(h1, h2, h3, h4, 0);
            n[0] = VehDrawFacetNormals0[centre]; n[1] = VehDrawFacetNormals1[centre];
            n[2] = VehDrawFacetNormals1[a]; n[3] = VehDrawFacetNormals2[a];
            n[4] = VehDrawFacetNormals2[b]; n[5] = VehDrawFacetNormals3[b];
            n[6] = VehDrawFacetNormals3[c]; n[7] = VehDrawFacetNormals0[c];
            count = 8;
            break;
        }
        }
        if (count) {
            vector_add(n[0].values, n[1].values, midpoint.values);
            for (int i = 2; i < count; ++i) {
                vector_add(midpoint.values, n[i].values, midpoint.values);
            }
        }
    }
    vox_normalise_vector(midpoint.values, midpoint.values);
    leaf_00628180_redirect(midpoint.values, start.values, VehDrawScootFirstIncrement.values);
    leaf_00628180_redirect(end.values, midpoint.values, VehDrawScootSecondIncrement.values);
    const int half = steps >> 1;
    leaf_006281b0_redirect(VehDrawScootFirstIncrement.values, 1.0f / half,
                          VehDrawScootFirstIncrement.values);
    leaf_006281b0_redirect(VehDrawScootSecondIncrement.values, 1.0f / (steps - half),
                          VehDrawScootSecondIncrement.values);
    VehDrawScootNormalStart = start;
    VehDrawScootNormalMid = midpoint;
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004C0F50 ?vehdraw_scoot@@YAXPAUBuffer@@HHHH@Z 0x004C0F50-0x004C1296
// RECOVERY_BINDING: v1 {"evidence": ["Original chooses first or second normal increment at a strict greater-than half-frame boundary, snapshots light, computes camera and shadow, sets scaling both before and after camera publication, assembles vehicle, increments animation, renders, and reloads armor state.", "Original mapped extent is preserved from the existing unclaimed seed and checked against the pinned executable; catalogue return/type names remain provenance until observed compiler-symbol facts are published."], "reason": "Recover an interpolated vehicle movement frame using real camera, normal, shadow and renderer ownership."}
// symbol    ?vehdraw_scoot@@YAXPAVBuffer@@HHHH@Z
void __cdecl vehdraw_scoot(Buffer *buffer, int x, int y, int frame, int zoom) {
    const int half = VehDrawScootSteps >> 1;
    const VOX_Vect light = VehLightDirMap;
    const VOX_Vect shadow_direction = {{-0.57735f, 0.57735f, 0.57735f}};
    VOX_Vect base, increment, normal;
    if (frame > half) {
        frame -= half;
        base = VehDrawScootNormalMid;
        increment = VehDrawScootSecondIncrement;
    } else {
        base = VehDrawScootNormalStart;
        increment = VehDrawScootFirstIncrement;
    }
    leaf_006281b0_redirect(increment.values, (float)frame, increment.values);
    vector_add(increment.values, base.values, normal.values);
    vox_normalise_vector(normal.values, normal.values);
    VOX_Matrix camera, inverse;
    compute_camera(&normal, &camera);
    g_VEHDRAW_CAVIAR.light_vector_ = light;
    vox_matrix_inverse(camera.values, inverse.values);
    VOX_Vect shadow;
    vox_matrix_vector_mul(inverse.values, shadow_direction.values, shadow.values);
    g_VEHDRAW_CAVIAR.shadow_vector_ = shadow;
    g_VEHDRAW_CAVIAR.set_shadow_table(ShadowColorTable);
    VOX_Vect position;
    position.values[0] = camera.values[2][0] * -10000.0f;
    position.values[1] = camera.values[2][1] * -10000.0f;
    position.values[2] = camera.values[2][2] * -10000.0f;
    g_VEHDRAW_CAVIAR.set_scaling(vehdraw_scale(zoom));
    g_VEHDRAW_CAVIAR.set_camera_direct(&position, &camera);
    // This second scale operation and fresh global reads occur in the image.
    g_VEHDRAW_CAVIAR.set_scaling(vehdraw_scale(zoom));
    g_VEHDRAW_CAVIAR.set_scene_rotation(
        VehDrawScootFacing * 0.785375f - VehDrawCameraAngles.values[0], 0.0f, 0.0f);
    const int veh_id = VehDrawTurnVehicle;
    vehdraw_construct_vehicle(veh_id, &g_VEHDRAW_CAVIAR);
    ++VehDrawAnimationStep;
    const int numerator = MapWinCaptionScale;
    x += vehdraw_centre_offset(numerator, zoom);
    y += vehdraw_centre_offset(numerator, zoom);
    g_VEHDRAW_CAVIAR.render(buffer, x, y, 1);
    vehdraw_finish_frame(buffer, veh_id, x, y, 0);
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004C12A0 ?vehdraw_draw@@YAXPAUBuffer@@HHHHHH@Z 0x004C12A0-0x004C1732
// RECOVERY_BINDING: v1 {"evidence": ["Original null-buffer return precedes work; optional stack selection and temporary coordinate override choose the vehicle/location. Terrain uses the actual map pointer slot and base7 VOX_Vect facet arrays. Vehicle assembly precedes scene rotation; render flags derive bit13 and transparent armor uses the corresponding overlay flags.", "Original mapped extent is preserved from the existing unclaimed seed and checked against the pinned executable; catalogue return/type names remain provenance until observed compiler-symbol facts are published."], "reason": "Recover ordinary vehicle voxel drawing through real terrain and model owners."}
// symbol    ?vehdraw_draw@@YAXPAVBuffer@@HHHHHH@Z
void __cdecl vehdraw_draw(Buffer *buffer, int veh_id, int x, int y, int flags,
                          int zoom, int terrain) {
    if (!buffer) return;
    const VOX_Vect light = VehLightDirMap;
    const VOX_Vect shadow_direction = {{-0.57735f, 0.57735f, 0.57735f}};
    if (flags & 1) veh_id = pick_top_veh(veh_id);
    int tile_x = VehDrawPositionX;
    int tile_y;
    if (tile_x == -1 || VehDrawPositionVehicle != veh_id) {
        tile_x = Vehs[veh_id].x;
        tile_y = Vehs[veh_id].y;
    } else {
        tile_y = VehDrawPositionY;
    }
    int facing = Vehs[veh_id].facing;
    if (facing == -1) facing = 0;
    VOX_Vect normal;
    if (terrain && vehdraw_uses_terrain(veh_id, tile_x, tile_y)) {
        const int h1 = ConsoleGlobal.get_relative_alt(tile_x, tile_y, 1);
        const int h2 = ConsoleGlobal.get_relative_alt(tile_x, tile_y, 2);
        const int h3 = ConsoleGlobal.get_relative_alt(tile_x, tile_y, 3);
        const int h4 = ConsoleGlobal.get_relative_alt(tile_x, tile_y, 4);
        vehdraw_average_facets(((h1 * 7 + h2) * 7 + h3) * 7 + h4 - 400, &normal);
    } else {
        normal.values[0] = normal.values[1] = 0.0f;
        normal.values[2] = 1.0f;
    }
    VOX_Matrix camera, inverse;
    compute_camera(&normal, &camera);
    g_VEHDRAW_CAVIAR.light_vector_ = light;
    vox_matrix_inverse(camera.values, inverse.values);
    VOX_Vect shadow;
    vox_matrix_vector_mul(inverse.values, shadow_direction.values, shadow.values);
    g_VEHDRAW_CAVIAR.shadow_vector_ = shadow;
    g_VEHDRAW_CAVIAR.set_shadow_table(ShadowColorTable);
    VOX_Vect position;
    position.values[0] = camera.values[2][0] * -10000.0f;
    position.values[1] = camera.values[2][1] * -10000.0f;
    position.values[2] = camera.values[2][2] * -10000.0f;
    g_VEHDRAW_CAVIAR.set_scaling(vehdraw_scale(zoom));
    g_VEHDRAW_CAVIAR.set_camera_direct(&position, &camera);
    vehdraw_construct_vehicle(veh_id, &g_VEHDRAW_CAVIAR);
    g_VEHDRAW_CAVIAR.set_scene_rotation(
        ((facing - 3) & 7) * 0.785375f - VehDrawCameraAngles.values[0], 0.0f, 0.0f);
    const int numerator = MapWinCaptionScale;
    x += vehdraw_centre_offset(numerator, zoom);
    y += vehdraw_centre_offset(numerator, zoom);
    g_VEHDRAW_CAVIAR.render(buffer, x, y, ((flags >> 12) & 2) | 1);
    vehdraw_finish_frame(buffer, veh_id, x, y, (flags >> 12) & 2);
}

// ORIGINAL: 0x004BF8D0 ?vehdraw_turn_start@@YAHHHHH@Z 0x004BF8D0-0x004BFC67
// RECOVERY_BINDING: v1 {"evidence": ["Original4BF8D0..4BFC67 returns the integer animation step count, snapshots light before selection, publishes selection before facing access, loads the map pointer slot, and uses unsigned Weapon.mode at+0A.", "All four facet vectors are copied before calls; camera/light/shadow publication follows original call order. Original stores only negative arc to float before comparison; preserving positive precision determines half-turn direction. Zero angle preserves zero steps and NaN increment. Observed VC6 emitted symbol and layouts."], "reason": "Recover vehicle turn setup using actual map, vehicle, renderer and vector ownership instead of fixed executable storage."}
int __cdecl vehdraw_turn_start(int veh_id, int direction, int speed, int flags) {
    const VOX_Vect light = VehLightDirMap;
    const VOX_Vect shadow_direction = {{-0.57735f, 0.57735f, 0.57735f}};
    if (flags & 1) {
        veh_id = pick_top_veh(veh_id);
    }
    VehDrawTurnVehicle = veh_id;
    const int x = VehLiftX;
    const int y = VehLiftY;
    int old_facing = Vehs[veh_id].facing;
    if (old_facing == -1) {
        old_facing = 0;
    }
    Vehs[veh_id].facing = (int8_t)direction;

    const Map *tiles = map_tiles();
    VOX_Vect normal;
    bool sloped = (tiles[y * MapLongitude + (x >> 1)].climate & 0xE0) >= 0x60;
    if (sloped) {
        const VehPrototype &prototype = VehPrototypes[Vehs[veh_id].proto_id];
        if (prototype.chassis_id == 0 && Weapon[prototype.weapon_id].mode < 3) {
            sloped = false;
        }
    }
    if (sloped) {
        const int h1 = ConsoleGlobal.get_relative_alt(x, y, 1);
        const int h2 = ConsoleGlobal.get_relative_alt(x, y, 2);
        const int h3 = ConsoleGlobal.get_relative_alt(x, y, 3);
        const int h4 = ConsoleGlobal.get_relative_alt(x, y, 4);
        const int index = ((h1 * 7 + h2) * 7 + h3) * 7 + h4 - 400;
        const VOX_Vect n0 = VehDrawFacetNormals0[index];
        const VOX_Vect n1 = VehDrawFacetNormals1[index];
        const VOX_Vect n2 = VehDrawFacetNormals2[index];
        const VOX_Vect n3 = VehDrawFacetNormals3[index];
        vector_add(n0.values, n1.values, normal.values);
        vector_add(normal.values, n2.values, normal.values);
        vector_add(normal.values, n3.values, normal.values);
        vox_normalise_vector(normal.values, normal.values);
    } else {
        normal.values[0] = 0.0f;
        normal.values[1] = 0.0f;
        normal.values[2] = 1.0f;
    }

    VOX_Matrix camera;
    compute_camera(&normal, &camera);
    g_VEHDRAW_CAVIAR.light_vector_ = light;
    VOX_Matrix inverse;
    vox_matrix_inverse(camera.values, inverse.values);
    VOX_Vect shadow;
    vox_matrix_vector_mul(inverse.values, shadow_direction.values, shadow.values);
    g_VEHDRAW_CAVIAR.shadow_vector_ = shadow;
    g_VEHDRAW_CAVIAR.set_shadow_table(ShadowColorTable);
    VOX_Vect position;
    position.values[0] = camera.values[2][0] * -10000.0f;
    position.values[1] = camera.values[2][1] * -10000.0f;
    position.values[2] = camera.values[2][2] * -10000.0f;
    g_VEHDRAW_CAVIAR.set_camera_direct(&position, &camera);

    VehDrawTurnAngle = ((old_facing - 3) & 7) * 0.785375f;
    const float target = ((direction - 3) & 7) * 0.785375f;
    float negative_distance;
    // The negative arc is rounded to float; the positive arc remains in the
    // x87 register through comparison. Rounding both can reverse half-turns.
    double positive_distance;
    if (VehDrawTurnAngle > target) {
        negative_distance = VehDrawTurnAngle - target;
        positive_distance = ((double)target + 6.283f) - VehDrawTurnAngle;
    } else {
        positive_distance = (double)target - VehDrawTurnAngle;
        negative_distance = (float)(((double)VehDrawTurnAngle + 6.283f) - target);
    }
    if (negative_distance < positive_distance) {
        const int steps = (int)(speed * negative_distance * 0.15915963f);
        VehDrawTurnIncrement = -(negative_distance / steps);
        return steps;
    }
    const int steps = (int)(speed * positive_distance * 0.15915963f);
    VehDrawTurnIncrement = (float)(positive_distance / steps);
    return steps;
}

// The original calls the CRT atan entry after storing a double argument;
// the compiler's default fpatan intrinsic bypasses that rounding boundary.
#pragma function(atan)
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004BF740 ?compute_camera@@YAXPAUVOX_Vect@@PAUVOX_Matrix@@@Z 0x004BF740-0x004BF8C1
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original control flow and original seed extent; observed VC6 external symbol. Tests exercise actual bodies, call/publication order, unordered/zero camera branches and complete facet tables at both established baselines.", "Camera publishes its first Euler triple before the second rotation; original CRT atan calls round their quotient to double. Facet initialization snapshots ConsoleGlobal inherited MapWin field at1DD6C, established by receiver-relative MapWin initialization; four independent 2401-element tables replace cross-object pointer arithmetic."], "reason": "Vehicle rendering setup with real VOX types, source-owned tables and actual Console terrain-height state."}
void __cdecl compute_camera(VOX_Vect *normal, VOX_Matrix *matrix) {
    VOX_Vect angles = {{0.0f, 0.0f, -1.0466667f}};
    VOX_Matrix first_rotation;
    VOX_Matrix second_rotation;
    VOX_Vect rotated;
    caviar_apply_rotation(angles.values, &first_rotation);
    vox_matrix_vector_mul(first_rotation.values, normal->values, rotated.values);

    // The original x87 tests take the zero arm for unordered values too.
    if (rotated.values[0] == 0.0f || rotated.values[0] != rotated.values[0]) {
        angles.values[0] = 0.0f;
    } else if (rotated.values[1] == 0.0f || rotated.values[1] != rotated.values[1]) {
        angles.values[0] = 1.57f;
    } else {
        angles.values[0] = (float)atan((double)rotated.values[0] / rotated.values[1]);
    }
    angles.values[1] = 0.0f;
    angles.values[2] = 0.0f;
    VehDrawCameraAngles = angles;
    caviar_apply_rotation(angles.values, &first_rotation);

    VOX_Vect projected = {{rotated.values[0], rotated.values[1], 0.0f}};
    float length = leaf_006281e0_redirect(projected.values);
    if (length == 0.0f || length != length) {
        angles.values[2] = 0.0f;
    } else if (rotated.values[2] == 0.0f || rotated.values[2] != rotated.values[2]) {
        angles.values[2] = 1.57f;
    } else {
        angles.values[2] = (float)atan((double)length / rotated.values[2]);
    }
    angles.values[0] = 0.0f;
    angles.values[1] = 0.0f;
    caviar_apply_rotation(angles.values, &second_rotation);
    vox_matrix_matrix_mul(first_rotation.values, second_rotation.values, matrix->values);
    matrix->values[2][0] = -matrix->values[2][0];
    matrix->values[2][1] = -matrix->values[2][1];
    matrix->values[2][2] = -matrix->values[2][2];
}
#pragma intrinsic(atan)

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004C1B10 ?vehdraw_compute_facet_normals@@YAXXZ 0x004C1B10-0x004C1CF7
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original control flow and original seed extent; observed VC6 external symbol. Tests exercise actual bodies, call/publication order, unordered/zero camera branches and complete facet tables at both established baselines.", "Camera publishes its first Euler triple before the second rotation; original CRT atan calls round their quotient to double. Facet initialization snapshots ConsoleGlobal inherited MapWin field at1DD6C, established by receiver-relative MapWin initialization; four independent 2401-element tables replace cross-object pointer arithmetic."], "reason": "Vehicle rendering setup with real VOX types, source-owned tables and actual Console terrain-height state."}
void __cdecl vehdraw_compute_facet_normals() {
    // 0x93341C is ConsoleGlobal + MapWin::field_1DD6C_, initialized to 25
    // or 8 by MapWin::calculate_dimensions. It is not an independent global.
    const int baseline = (int)ConsoleGlobal.field_1DD6C_;
    int index = 0;
    for (int i = 30; i < 240; i += 30) {
        const float height_i = (float)((i >> 2) - baseline);
        for (int j = 30; j < 240; j += 30) {
            const float height_j = (float)((j >> 2) - baseline);
            for (int k = 30; k < 240; k += 30) {
                const float height_k = (float)((k >> 2) - baseline);
                for (int l = 30; l < 240; l += 30) {
                    const float height_l = (float)((l >> 2) - baseline);
                    VOX_Vect a = {{0.0f, 50.0f, height_j}};
                    VOX_Vect b = {{-50.0f, 0.0f, height_i}};
                    VOX_Vect c = {{50.0f, 0.0f, height_k}};
                    VOX_Vect d = {{0.0f, -50.0f, height_l}};
                    VOX_Vect n0, n1, n2, n3;
                    vox_cross_product(a.values, b.values, n0.values);
                    vox_cross_product(c.values, a.values, n1.values);
                    vox_cross_product(d.values, c.values, n2.values);
                    vox_cross_product(b.values, d.values, n3.values);
                    vox_normalise_vector(n0.values, n0.values);
                    vox_normalise_vector(n1.values, n1.values);
                    vox_normalise_vector(n2.values, n2.values);
                    vox_normalise_vector(n3.values, n3.values);
                    VehDrawFacetNormals0[index] = n0;
                    VehDrawFacetNormals1[index] = n1;
                    VehDrawFacetNormals2[index] = n2;
                    VehDrawFacetNormals3[index] = n3;
                    ++index;
                }
            }
        }
    }
}

// has_fac_built_call() (real bitmask() call, general.h bitmask_call) is now
// declared in base.h, which this file already includes - no local
// redeclaration needed.

/*
Purpose: Generate an output string for the specified unit's morale.
// ORIGINAL: 0x004B3FD0 ?say_morale@@YAXPADHH@Z 0x004B3FD0-0x004B43BF
// LEVER: std::string -> strcat, matching the say_stats family (calls went
//        from 16, mostly basic_string internals, to 10; strcat count now
//        exactly 7, matching the image). TRIED: crèche/brood-pit was
//        already the correct asymmetric if/else-if here, unlike
//        get_basic_offense - no change needed.
// LEVER: the ternary feeding the first strcat's `Strings::get()` argument
//        rewritten as an if/else with the strcat duplicated into each arm -
//        the image has 2 static call sites (one per branch) and the ternary
//        let the compiler merge them to 1. call_diff went from 10 (image 14)
//        to 11; best flag set's exact-instruction count nearly doubled,
//        13/313 -> 26/313 at /O2 /GR- /Oy- /GX.
// TRIED: `bitmask_call` forwarder for the crèche/brood-pit
//        `has_fac_built()` sites (general.h, the same lever that helped
//        base.cpp's pop_goal) - tried on the sibling morale_veh (0x5C0E40)
//        first and reverted there, similarity fell at every flag set; not
//        retried here on that evidence. Still open: the same bitmask() x3
//        auto-inline wall (see get_basic_offense, 0x5015B0) accounts for the
//        remaining call-count gap (11 vs 14: 3x bitmask via has_fac_built).
// size      1007 bytes
// prototype void (__cdecl ?say_morale@@YAXPADHH@Z)(int8* moraleOutput, int vehID, int factionIDvsNative)
// callers   2   call targets   5
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004E3A50 0x0050BA00 0x005C0E40 0x006169A0 0x00645470
Return Value: n/a
Status: Complete
*/
void __cdecl say_morale(LPSTR morale_output, int veh_id, int faction_id_vs_native) {
    // TODO: Look into inconsistencies related to morale display
    uint32_t morale = morale_veh(veh_id, false, faction_id_vs_native);
    uint32_t faction_id = Vehs[veh_id].faction_id;
    int proto_id = Vehs[veh_id].proto_id;
    // LEVER: std::string -> raw strcat() straight onto morale_output, per
    // the say_stats family above. The image's own `calls` list is 7 strcat
    // sites, not a string builder: this fill, the drone-riot " (-)", " "
    // and "(" as TWO separate strcat calls (not one " (" literal), the
    // "+" loop body, ")", and the designate-defender "(d)".
    if (proto_id < MaxVehProtoFactionNum &&
        (get_proto_offense_rating(proto_id) < 0 || proto_id == BSC_SPORE_LAUNCHER)) {
        strcat(morale_output, StringTable.get((int)Morale[morale].name_lifecycle));
    } else {
        strcat(morale_output, StringTable.get((int)Morale[morale].name));
    }
    if (VehPrototypes[proto_id].plan < PLAN_COLONIZATION) {
        uint32_t morale_penalty = 0;
        int home_base_id = Vehs[veh_id].home_base_id;
        if (home_base_id >= 0 && Bases[home_base_id].state & BSTATE_DRONE_RIOTS_ACTIVE && morale > 0
            && !(Players[faction_id].rule_flags & RFLAG_MORALE)) {
            strcat(morale_output, " (-)");
            morale_penalty = 1;
        }
        VehMoraleModifierCount = 0;
        int base_id = base_at(Vehs[veh_id].x, Vehs[veh_id].y);
        if (base_id >= 0 && morale < 6) {
            if (has_fac_built(FAC_CHILDREN_CRECHE, base_id)) {
                VehMoraleModifierCount++;
                int morale_active = range(PlayersData[faction_id].soc_effect_active.morale, -4, 4);
                if (morale_active <= -2) {
                    morale_active++;
                }
                if (morale_active < 0) {
                    int morale_cap = morale + VehMoraleModifierCount;
                    do {
                        if (morale_cap >= 6) {
                            break;
                        }
                        VehMoraleModifierCount++;
                        morale_cap++;
                        morale_active++;
                    } while (morale_active < 0);
                }
                if(has_fac_built_call(FAC_BROOD_PIT, base_id) && proto_id < MaxVehProtoFactionNum
                    && (get_proto_offense_rating(proto_id) < 0 || proto_id == BSC_SPORE_LAUNCHER)) {
                    VehMoraleModifierCount++;
                }
            } else if (morale < 6 && has_fac_built_call(FAC_BROOD_PIT, base_id)
                && proto_id < MaxVehProtoFactionNum
                && (get_proto_offense_rating(proto_id) < 0 || proto_id == BSC_SPORE_LAUNCHER)) {
                VehMoraleModifierCount++;
                int morale_active = range(PlayersData[faction_id].soc_effect_active.morale, -4, 4);
                if (morale_active <= -2) {
                    morale_active++;
                }
                if (morale_active < 0) {
                    int morale_cap = morale + VehMoraleModifierCount;
                    do {
                        if (morale_cap >= 6) {
                            break;
                        }
                        VehMoraleModifierCount++;
                        morale_cap++;
                        morale_active++;
                    } while (morale_active < 0);
                }
            }
        }
        int morale_pending = PlayersData[faction_id].soc_effect_pending.morale;
        if (morale_pending == 2 || morale_pending == 3) {
            VehMoraleModifierCount++;
        }
        if (!morale && !VehMoraleModifierCount) {
            VehMoraleModifierCount = 1;
        }
        if(VehMoraleModifierCount) {
            strcat(morale_output, " ");
            strcat(morale_output, "(");
            for (int i = 0; i < VehMoraleModifierCount; i++) {
                strcat(morale_output, "+");
            }
            strcat(morale_output, ")");
        }
        VehMoraleModifierCount -= morale_penalty;
        if (Vehs[veh_id].state & VSTATE_DESIGNATE_DEFENDER) {
            strcat(morale_output, "(d)");
        }
    }
    // TODO: assumes at least 1032 char nulled buffer (stringTemp), eventually remove
}

/*
Purpose: Get the morale string for the specified unit and store it in the stringTemp buffer.
// ORIGINAL: 0x004B43C0 ?say_morale@@YAXHH@Z 0x004B43C0-0x004B43DA BYTE_EXACT
// size      26 bytes
// prototype void (__cdecl ?say_morale@@YAXHH@Z)(int vehID, int factionID)
// callers   4   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004B3FD0
Return Value: n/a
Status: Complete
*/
void __cdecl say_morale(int veh_id, int faction_id_vs_native) {
    say_morale(StringTemp, veh_id, faction_id_vs_native);
}

/*
Purpose: Check whether the specified unit could ever reach the given tile as a patrol waypoint.
// ORIGINAL: 0x004CD6A0 ?valid_patrol@@YAHHHH@Z 0x004CD6A0-0x004CDA23
// size      899 bytes
// prototype int (__cdecl ?valid_patrol@@YAHHHH@Z)(int vehID, int xCoord, int yCoord)
// callers   2   call targets   5
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004E3A50 0x0050DE50 0x0050E160 0x005C1540 0x00644F3A
Return Value: Is the waypoint reachable? true/false
Status: Complete

Three answers, one per triad, and the shared part is only the four bounds checks and the refusal
to patrol to the tile the unit is already standing on.

LAND compares regions, which is the cheap standing-in for "there is a land route".

SEA asks the harbour question instead. A sea unit moves between ports, so what matters is whether
the two ends are connected by water: two bases go through port_to_port, a base and open water
through base_on_sea against the water's region, and open water at both ends falls back to the
same region comparison land uses.

AIR is a fuel calculation. A chassis with no range is unlimited and answers yes at once.
Otherwise both ends are classified as a landing place or not, and the test is `base_at() != 0 ||
airbase`. That is not `base_at() >= 0`: base_at answers -1 on an empty tile, so EMPTY GROUND
counts as a landing place and the one thing that does not is a tile holding base id zero with no
airbase on it. The fixture pins the distinction from both directions rather than assuming it is
a transcription slip. The reachable distance is

  (speed * (chassis range - terraforming_turns - 1) + moves left this turn) / move_rate_roads

with the turn's remaining movement clamped into 0..999 first. terraforming_turns is the field
air units keep their elapsed flight time in. The distance itself is the doubled vector_dist
halved once - (|dy| + wrapped |dx|) >> 1 - and it must be within the reachable distance when both
ends are landing places, or within half of it when either end is not, which is the fuel to get
there and back.

The x term wraps on a round map by testing bit 0 of MapIsFlat, the same reading reset_territory
and territory() use rather than x_dist's whole-int one.

Verification note: the sweep against recovery-gameplay-tests kills 46 of 52 valid mutants, and
the six survivors are dead code or equivalences rather than untested behaviour.

  - Both landing-place flags are set from a comparison and are therefore 0 or 1, so the `< 0`
    pair at 0x004CD900 that returns false when BOTH are negative cannot fire. The original tests
    them all the same and so does this. Three survivors.
  - `moves_left < 0` against `<= 0` or `< 1` differ only at zero, where the clamp writes the zero
    that is already there. Two survivors.
  - `moves_left > 999` against `>= 999` differ only at 999, where the clamp writes the 999 that
    is already there.
*/
int __cdecl valid_patrol(int veh_id, int x, int y) {
    Veh &veh = Vehs[veh_id];
    if (x == veh.x && y == veh.y) {
        return false;
    }
    if (y < 0 || y >= MapLatitudeBounds || x < 0 || x >= MapLongitudeBounds) {
        return false;
    }
    uint32_t triad = Chassis[VehPrototypes[veh.proto_id].chassis_id].triad;
    if (triad == TRIAD_LAND) {
        return map_loc(x, y)->region == map_loc(veh.x, veh.y)->region;
    }
    if (triad == TRIAD_SEA) {
        int base_veh = base_at(veh.x, veh.y);
        int base_dst = base_at(x, y);
        if (base_veh >= 0) {
            if (base_dst >= 0) {
                return port_to_port(base_veh, base_dst) != 0;
            }
            return base_on_sea(base_veh, map_loc(x, y)->region) != 0;
        }
        if (base_dst >= 0) {
            return base_on_sea(base_dst, map_loc(veh.x, veh.y)->region) != 0;
        }
        return map_loc(x, y)->region == map_loc(veh.x, veh.y)->region;
    }
    if (triad != TRIAD_AIR) {
        return true;
    }
    uint32_t chassis_id = VehPrototypes[veh.proto_id].chassis_id;
    if (!Chassis[chassis_id].range) {
        return true;
    }
    int landing_veh;
    if (base_at(veh.x, veh.y)) {
        landing_veh = 1;
    } else {
        landing_veh = 0;
        if (map_loc(veh.x, veh.y)->bit & BIT_AIRBASE) {
            landing_veh = 1;
        }
    }
    int landing_dst;
    if (base_at(x, y)) {
        landing_dst = 1;
    } else {
        landing_dst = 0;
        if (map_loc(x, y)->bit & BIT_AIRBASE) {
            landing_dst = 1;
        }
    }
    if (landing_veh < 0 && landing_dst < 0) {
        return false;
    }
    int moves_left = (int)speed(veh_id, false) - veh.moves_expended;
    if (moves_left < 0) {
        moves_left = 0;
    } else if (moves_left > 999) {
        moves_left = 999;
    }
    int reach = ((int)speed(veh_id, false)
        * (Chassis[VehPrototypes[veh.proto_id].chassis_id].range - veh.terraforming_turns - 1)
        + moves_left) / (int)Rules.move_rate_roads;
    int x_delta = abs(x - veh.x);
    if (!(MapIsFlat & 1) && x_delta > (int)MapLongitude) {
        x_delta = MapLongitudeBounds - x_delta;
    }
    int dist = (abs(y - veh.y) + x_delta) >> 1;
    if (landing_veh && landing_dst) {
        return dist <= reach;
    }
    return dist <= reach / 2;
}

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
    if (!has_tech_call(Rules.tech_orb_insert_sans_spc_elev, faction_id)) { // default 'Graviton Theory'
        int base_id = *(&SecretProject.human_genome_project + SP_SPACE_ELEVATOR);
        BOOL has_orb_insert = (base_id < 0) ? false
            : (Bases[base_id].faction_id_current == faction_id);
        if (!has_orb_insert) {
            return Rules.max_airdrop_sans_orb_insert;
        }
    }
    return (MapLongitudeBounds <= MapLatitudeBounds) ? MapLatitudeBounds : MapLongitudeBounds;
}

// Typed dependencies: capture_base/spot_all/goody_box have explicitly
// unfinished bodies in their owners. Other helpers have product definitions;
// their remaining recovery/reference obligations are not waived by this caller.
extern int __cdecl break_treaty(int faction_id, int other_faction_id, int treaty);
extern int __cdecl act_of_aggression(int faction_id, int target_faction_id);
extern void __cdecl capture_base(int faction_id, int base_owner, int mode);
extern void __cdecl spot_all(int veh_id, int mode);
extern int __cdecl goody_box(int veh_id, int goody_id);
extern void __cdecl clock_wait(int milliseconds);
extern void __cdecl unspot_stack(int veh_id);

// Airdrop a vehicle, including range, interception, landing-site, diplomatic
// and post-landing processing.  The four modes are the caller's network/local
// context flags; mode 1 is the network execution path, mode 3 the local path.
// ORIGINAL: 0x004CC360 ?action_airdrop@@YAHHHHH@Z 0x004CC360-0x004CD051 FILE
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/004cc360.cpp retained signature/extents and complete seed; no historical claim copied", "Coordinator full pinned original instruction decode 004CC360..004CD051, typed source owner fields and corrected original call/branch evidence", "recovery/tests/test_airdrop_network.py actual-body directional treaty and network reentrancy controls; not linked/runtime/game acceptance"], "reason": "Recover complete typed airdrop admission/interception, landing selection, stack movement, capture and post-drop updates. Preserve unsigned damage storage with integer thresholds and directional treaty evidence; referenced unfinished dependencies remain explicit, not game acceptance."}
int __cdecl action_airdrop(int veh_id, int x, int y, int mode) {
    NetDaemonNet.lock_state_[1].leading_words_[3] = mode;

    Veh &vehicle = Vehs[veh_id];
    const int faction = vehicle.faction_id;
    const int old_x = vehicle.x;
    const int old_y = vehicle.y;
    const int distance_x = abs(old_x - x);
    const int wrapped_x = (MapIsFlat & 1) ? distance_x
        : (distance_x > MapLongitude ? MapLongitudeBounds - distance_x : distance_x);
    const int distance = wrapped_x + abs(old_y - y);
    const int radius = drop_range(faction);
    if (radius < (distance >> 1)) {
        if ((mode & 1) != 0 && faction == ConsoleGlobal.faction_id_) {
            parse_num(0, Rules.max_airdrop_sans_orb_insert);
            if (Players[faction].rule_flags & 0x80) {
                NetMsgPrimary.pop("DROPRANGE", 0xFFFFEC78, 0, "al_drop_sm.pcx");
                return 0;
            }
            NetMsgPrimary.pop("DROPRANGE", 0xFFFFEC78, 0, "airdp_sm.pcx");
        }
        return 0;
    }

    Map *target = map_loc(x, y);
    if ((target->climate & 0xE0) < 0x60) {
        if ((mode & 1) != 0 && faction == ConsoleGlobal.faction_id_) {
            if (Players[faction].rule_flags & 0x80) {
                NetMsgPrimary.pop("DROPSEA", 0xFFFFEC78, 0, "al_drop_sm.pcx");
                return 0;
            }
            NetMsgPrimary.pop("DROPSEA", 0xFFFFEC78, 0, "airdp_sm.pcx");
        }
        return 0;
    }

    const int owner = target->val2 & 0x0F;
    if ((target->bit & 2) && owner < 8 && owner != faction &&
        (PlayersData[faction].diplo_treaties[owner] & 1U) == 0) {
        if ((mode & 1) != 0 && faction == ConsoleGlobal.faction_id_) {
            if (Players[faction].rule_flags & 0x80) {
                NetMsgPrimary.pop("DROPUNIT", 0xFFFFEC78, 0, "al_drop_sm.pcx");
                return 0;
            }
            NetMsgPrimary.pop("DROPUNIT", 0xFFFFEC78, 0, "airdp_sm.pcx");
        }
        return 0;
    }

    if (Weapon[VehPrototypes[vehicle.proto_id].weapon_id].offense_rating == 0 &&
        zoc_move(x, y, faction) != 0) {
        if ((mode & 1) != 0 && faction == ConsoleGlobal.faction_id_) {
            if (Players[faction].rule_flags & 0x80) {
                NetMsgPrimary.pop("NONCOMDROP", 0xFFFFEC78, 0, "al_drop_sm.pcx");
                return 0;
            }
            NetMsgPrimary.pop("NONCOMDROP", 0xFFFFEC78, 0, "airdp_sm.pcx");
        }
        return 0;
    }

    // A hostile, non-treaty faction's unit already on the destination prevents
    // a non-combat unit from being dropped into that stack.
    if ((target->bit & 1) != 0 && owner < 8 && owner != faction &&
        (PlayersData[faction].diplo_treaties[owner] & 1U) == 0 &&
        Weapon[VehPrototypes[vehicle.proto_id].weapon_id].offense_rating == 0) {
        if ((mode & 1) != 0 && faction == ConsoleGlobal.faction_id_) {
            if (Players[faction].rule_flags & 0x80) {
                NetMsgPrimary.pop("NONCOMDROP", 0xFFFFEC78, 0, "al_drop_sm.pcx");
                return 0;
            }
            NetMsgPrimary.pop("NONCOMDROP", 0xFFFFEC78, 0, "airdp_sm.pcx");
        }
        return 0;
    }
    if ((target->bit & 1) != 0 && owner < 8 && owner != faction &&
        (PlayersData[faction].diplo_treaties[owner] & 1U) == 0 &&
        (mode & 1) != 0 && faction == ConsoleGlobal.faction_id_ &&
        break_treaty(faction, owner, 7) != 0) {
        return 0;
    }

    // The drop checks the radius tiles for foreign air defenses that would
    // intercept the aircraft.
    for (int offset_index = 0; offset_index < 100; offset_index += 4) {
        const int radius_index = offset_index / 4;
        int near_x = x + RadiusOffsetX[radius_index];
        if (!(MapIsFlat & 1)) {
            if (near_x < 0) near_x += MapLongitudeBounds;
            else if (near_x >= MapLongitudeBounds) near_x -= MapLongitudeBounds;
        }
        const int near_y = y + RadiusOffsetY[radius_index];
        if (near_x < 0 || near_x >= MapLongitudeBounds || near_y < 0 || near_y >= MapLatitudeBounds)
            continue;
        Map *adjacent = map_loc(near_x, near_y);
        const int near_owner = adjacent->val2 & 0x0F;
        if ((adjacent->bit & 1) == 0 || near_owner == 0 || near_owner >= 8 || near_owner == faction ||
            (PlayersData[faction].diplo_treaties[near_owner] & 0x10U) == 0)
            continue;
        const int base_id = base_at(near_x, near_y);
        if (base_id < 0) continue;
        int facility_offset, facility_mask;
        bitmask(0x1D, &facility_offset, &facility_mask);
        const bool airbase = (Bases[base_id].facilities_built[facility_offset] & facility_mask) != 0;
        const int cloudbase = SecretProject.cloudbase_academy;
        const bool has_cloudbase = cloudbase >= 0 && Bases[cloudbase].faction_id_current == near_owner;
        if (!airbase && !has_cloudbase) {
            const int nearby_vehicle = veh_at(near_x, near_y);
            if (stack_check(nearby_vehicle, 2, 4, -1, -1) == 0) continue;
        }
        if ((mode & 1) != 0 && faction == ConsoleGlobal.faction_id_) {
            parse_says(0, Bases[base_id].name_string, -1, -1);
            if (Players[faction].rule_flags & 0x80) {
                NetMsgPrimary.pop("AIRDEFENSES", 0xFFFFEC78, 0, "alair_sm.pcx");
                return 0;
            }
            NetMsgPrimary.pop("AIRDEFENSES", 0xFFFFEC78, 0, "air_sm.pcx");
        }
        return 0;
    }

    if ((mode & 2) == 0) return 1;

    const int goody = goody_at(x, y);
    bool local_seen = faction == ConsoleGlobal.faction_id_;
    if (!local_seen) {
        const bool visible_owner = (target->bit & 3) != 0 && owner < 8;
        local_seen = (visible_owner && owner == ConsoleGlobal.faction_id_) ||
                     ((GameState & 0x80) != 0);
        if (!local_seen) {
            for (int neighbor = 0; neighbor < 8; ++neighbor) {
                int near_x = x + RadiusBaseX[neighbor];
                if (!(MapIsFlat & 1)) {
                    if (near_x < 0) near_x += MapLongitudeBounds;
                    else if (near_x >= MapLongitudeBounds) near_x -= MapLongitudeBounds;
                }
                const int near_y = y + RadiusBaseY[neighbor];
                if (near_x >= 0 && near_x < MapLongitudeBounds && near_y >= 0 &&
                    near_y < MapLatitudeBounds) {
                    const Map *adjacent = map_loc(near_x, near_y);
                    const int near_owner = adjacent->val2 & 0x0F;
                    if ((adjacent->bit & 3) != 0 && near_owner == ConsoleGlobal.faction_id_) {
                        local_seen = true;
                        break;
                    }
                }
            }
        }
    }

    bool allied_local = local_seen && faction == ConsoleGlobal.faction_id_;
    if (local_seen && faction != ConsoleGlobal.faction_id_) {
        const uint32_t treaty = PlayersData[ConsoleGlobal.faction_id_].diplo_treaties[faction];
        allied_local = (treaty & 1U) != 0 ? (GamePreferences & 0x8000U) != 0
                                          : (GamePreferences & 4U) != 0;
    }

    int best_score = 999999;
    int landing_slot = -1;
    for (int neighbor = 0; neighbor < 8; ++neighbor) {
        int near_x = x + RadiusBaseX[neighbor];
        if (!(MapIsFlat & 1)) {
            if (near_x < 0) near_x += MapLongitudeBounds;
            else if (near_x >= MapLongitudeBounds) near_x -= MapLongitudeBounds;
        }
        const int near_y = y + RadiusBaseY[neighbor];
        if (near_x < 0 || near_x >= MapLongitudeBounds || near_y < 0 || near_y >= MapLatitudeBounds)
            continue;
        const int random = rand();
        int score = random % 10 + ((neighbor > 5 || neighbor < 1) ? 0x19 : 0x32);
        if (neighbor == 1 || neighbor == 5) score -= 10;
        const Map *adjacent = map_loc(near_x, near_y);
        if ((adjacent->bit & 3) != 0) {
            const int near_owner = adjacent->val2 & 0x0F;
            if (near_owner < 8) {
                score += 100;
                if (near_owner != faction) {
                    score += 1000;
                    if ((PlayersData[faction].diplo_treaties[near_owner] & 1U) == 0)
                        score += 10000;
                }
            }
        }
        if (score < best_score) { best_score = score; landing_slot = neighbor; }
    }
    if (landing_slot < 0) return 0;

    if (local_seen && faction != ConsoleGlobal.faction_id_) {
        const int base_id = base_find(x, y, -1, -1, -1, 1);
        parse_says(0, Bases[base_id].name_string, -1, -1);
        GenderDefault = Players[faction].noun_gender;
        PluralityDefault = Players[faction].is_noun_plural;
        parse_says(1, Players[faction].noun_faction, -1, -1);
        const bool ai = (Players[faction].rule_flags & 0x80) != 0;
        const char *picture = ai ? "al_drop_sm.pcx" : "airdp_sm.pcx";
        const char *message = drop_range(faction) <= Rules.max_airdrop_sans_orb_insert
            ? "MADEAIRDROP" : "MADEORBITAL";
        popp(*ClimatePopupTitle, message, 0, picture, nullptr);
    }

    vehicle.state |= 0x20;
    if (Weapon[VehPrototypes[vehicle.proto_id].weapon_id].offense_rating == 0) veh_skip(veh_id);
    const int from_x = vehicle.x;
    const int from_y = vehicle.y;
    const int destination_x_unwrapped = x + RadiusBaseX[landing_slot];
    int destination_x = destination_x_unwrapped;
    if (!(MapIsFlat & 1)) {
        if (destination_x < 0) destination_x += MapLongitudeBounds;
        else if (destination_x >= MapLongitudeBounds) destination_x -= MapLongitudeBounds;
    }
    const int destination_y = y + RadiusBaseY[landing_slot];
    stack_veh(veh_id, 1);
    draw_tile(from_x, from_y, 2);
    if (local_seen && allied_local)
        veh_scoot(veh_id, destination_x, destination_y, landing_slot ^ 4, 0);
    unspot_stack(veh_id);

    // Walk from this vehicle to the first member through prev links, then
    // traverse the complete stack in next-link order and skip every other unit.
    int stack_top = veh_id;
    while (Vehs[stack_top].prev_veh_id_stack >= 0)
        stack_top = Vehs[stack_top].prev_veh_id_stack;
    for (int member = stack_top; member >= 0; member = Vehs[member].next_veh_id_stack) {
        if (member != veh_id) veh_skip(member);
    }

    if (veh_id == ConsoleGlobal.selected_vehicle_ && ConsoleGlobal.cursor_active_ != 0)
        ConsoleGlobal.cursor_next(x, y);
    stack_put(veh_id, x, y);

    const int base_here = base_at(x, y);
    if (base_here < 0) {
        vehicle.dmg_incurred = static_cast<uint8_t>(vehicle.dmg_incurred +
            VehPrototypes[vehicle.proto_id].reactor_id * 2);
        const int reactor = VehPrototypes[vehicle.proto_id].reactor_id;
        const int target_damage = VehPrototypes[vehicle.proto_id].plan == 0x0C
            ? 1 : (reactor == 0 ? 1 : (reactor > 100 ? 100 : reactor)) * 10;
        for (;;) {
            const int difference = static_cast<int>(target_damage) - vehicle.dmg_incurred;
            if (difference >= 0 && (difference > 9999 || target_damage != vehicle.dmg_incurred))
                break;
            --vehicle.dmg_incurred;
        }
    } else if (faction != Bases[base_here].faction_id_current &&
               (PlayersData[faction].diplo_treaties[Bases[base_here].faction_id_current] & 1U) == 0) {
        const int previous_owner = Bases[base_here].faction_id_current;
        if ((LockEnableMask & (1U << faction)) == 0) veh_skip(veh_id);
        VehSelectionFollowLimit = veh_id;
        vehicle.dmg_incurred = 0;
        act_of_aggression(faction, previous_owner);
        capture_base(base_here, faction, 0);
        veh_id = VehSelectionFollowLimit;
    }
    spot_all(veh_id, 1);
    ConsoleGlobal.update_data(0);
    if (goody != 0) goody_box(veh_id, goody);
    if (faction != ConsoleGlobal.faction_id_ && (GameState & 0x80) == 0 &&
        local_seen && ConsoleGlobal.field_23BF0_ == 0 &&
        (GetAsyncKeyState(VK_SHIFT) >> 8) == 0 && IsMultiplayerNet == 0) {
        clock_wait(allied_local ? 200 : 50);
    }
    return 1;
}

/*
Purpose: Check whether the specified prototype is a planet buster.
// ORIGINAL: 0x005004F0 ?planet_buster2@@YAHH@Z 0x005004F0-0x00500518 BYTE_EXACT
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
// BODY IN veh.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


/*
Purpose: Calculate how defensive the specified tile is to the defending unit based on the terrain.
// ORIGINAL: 0x005010C0 ?defense_value@@YAHHHHHH@Z 0x005010C0-0x0050134F
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
    if (is_ocean(x, y) || base_who(x, y) >= 0) {
        return 2;
    }
    uint32_t bit = bit_at(x, y);
    if (bit & BIT_FUNGUS && altitude_at(x, y) >= ALT_BIT_OCEAN_SHELF && veh_id_atk >= 0
        && (!Vehs[veh_id_atk].faction_id 
            || has_project(SP_PHOLUS_MUTAGEN, Vehs[veh_id_atk].faction_id) 
            || (get_offense_rating(veh_id_atk) < 0
                && Vehs[veh_id_atk].proto_id < MaxVehProtoFactionNum))) {
                return 2;
    }
    uint32_t is_rocky = rocky_at(x, y) > TERRAIN_BIT_ROLLING;
    VehBattleDisplayTerrain = *((LPSTR *)Labels.strings_ptr + 91); // "Rocky"
    uint32_t defense = is_rocky;
    if (bit & BIT_FUNGUS && altitude_at(x, y) >= ALT_BIT_OCEAN_SHELF && !defense
        && get_triad(veh_id_def) != TRIAD_AIR) {
        if (veh_id_atk >= 0 && ((get_offense_rating(veh_id_atk) < 0
                && Vehs[veh_id_atk].proto_id < MaxVehProtoFactionNum)
            || has_project(SP_PHOLUS_MUTAGEN, Vehs[veh_id_atk].faction_id))) {
            defense = is_rocky;
        } else {
            VehBattleDisplayTerrain = *((LPSTR *)Labels.strings_ptr + 338); // "Fungus"
            defense = (has_project(SP_PHOLUS_MUTAGEN, faction_id) 
                || (Vehs[veh_id_def].proto_id < MaxVehProtoFactionNum 
                    && get_offense_rating(veh_id_def) < 0)) ? 2 : 1;
        }
    }
    if (bit & BIT_FOREST && !defense && (veh_id_atk < 0 || get_triad(veh_id_atk) == TRIAD_LAND)) {
        VehBattleDisplayTerrain = *((LPSTR *)Labels.strings_ptr + 291); // "Forest"
        defense = 1;
    }
    return defense + 2;
}

/*
Purpose: Calculate the lifecycle (morale) of the specified native life unit.
// ORIGINAL: 0x00501350 ?morale_alien@@YAHHH@Z 0x00501350-0x005014F1
// TRIED: still MISMATCH (~0.72 similar, up from ~0.46) - most of the remaining divergence is register allocation the compiler picked differently around the atrocities/veh-flags block and the two `range()` call sites, which every flag set tried left in place.
// TRIED: source had the Fungal Tower special case as the OUTER `if`, guarded by `veh_id >= 0`, with the turn-ladder as its `else` - the image runs the turn-ladder/atrocities/veh-flags computation FIRST, UNCONDITIONALLY, then reads `Vehs[veh_id].x/.y/.proto_id` with NO veh_id guard (an out-of-bounds `Vehs[-1]` read - reproduced deliberately, see the BUG comment at the site) and only THEN branches on proto_id, overwriting `morale` if it is the Fungal Tower. Each branch ends in its OWN `return range(morale, 0, 6);` (not a shared tail after the `if`) - `int x`/`int y` locals (not `int16_t`) match the image's `movsx` at the point of assignment.
// symbol    ?morale_alien@@YAIHH@Z
// size      417 bytes
// prototype int (__cdecl ?morale_alien@@YAHHH@Z)(int vehID, int factionIDvsNative)
// callers   9   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: Lifecycle (morale) value
Status: Complete
*/
uint32_t __cdecl morale_alien(int veh_id, int faction_id_vs_native) {
    int morale;
    if (TurnCurrentNum < 45) {
        morale = 0;
    } else if (TurnCurrentNum < 90) {
        morale = 1;
    } else if (TurnCurrentNum < 170) {
        morale = 2;
    } else if (TurnCurrentNum < 250) {
        morale = 3;
    } else if (TurnCurrentNum < 330) {
        morale = 4;
    } else { // 330+
        morale = 6;
    }
    if (faction_id_vs_native > 0) {
        morale += (PlayersData[faction_id_vs_native].major_atrocities != 0)
            + (TectonicDetonationCount[faction_id_vs_native] != 0);
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
    int x = Vehs[veh_id].x;
    int y = Vehs[veh_id].y;
    // Fungal Tower specific code: overrides the morale computed above entirely.
    if (Vehs[veh_id].proto_id == BSC_FUNGAL_TOWER) {
        morale = 0;
        // similar to is_coast() > except with fungus check + Ocean Shelf included
        for (int i = RadiusRange[0]; i < RadiusRange[1]; i++) {
            int x_radius = xrange(x + RadiusOffsetX[i]);
            int y_radius = y + RadiusOffsetY[i];
            if (on_map(x_radius, y_radius) && bit_at(x_radius, y_radius) & BIT_FUNGUS
                && altitude_at(x_radius, y_radius) >= ALT_BIT_OCEAN_SHELF) {
                morale++;
            }
        }
        morale -= 2;
        return range(morale, 0, 6);
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
    uint32_t faction_id_atk = Vehs[veh_id_atk].faction_id;
    uint32_t proto_id_atk = Vehs[veh_id_atk].proto_id;
    uint32_t morale = faction_id_atk ? morale_veh(veh_id_atk, true, 0) 
        : morale_alien(veh_id_atk, veh_id_def >= 0 ? Vehs[veh_id_def].faction_id : -1);
    int base_id_atk = base_at(Vehs[veh_id_atk].x, Vehs[veh_id_atk].y);
    if (base_id_atk >= 0) {
        if (has_fac_built(FAC_CHILDREN_CRECHE, base_id_atk)) {
            morale++;
            int morale_active = range(PlayersData[faction_id_atk].soc_effect_active.morale, -4, 4);
            if (morale_active <= -2) {
                morale_active++;
            }
            morale -= morale_active;
            // The disassembly calls bitmask() a 3rd time: on the crèche-true
            // fallthrough, a brood pit check ALSO runs, but only as a bare
            // `morale++` - the full range()/cap adjustment below it belongs
            // only to the crèche-false (`else if`) arm.
            if (has_fac_built_call(FAC_BROOD_PIT, base_id_atk) && proto_id_atk < MaxVehProtoFactionNum
                && (get_proto_offense_rating(proto_id_atk) < 0 || proto_id_atk == BSC_SPORE_LAUNCHER)) {
                morale++;
            }
        } else if (has_fac_built_call(FAC_BROOD_PIT, base_id_atk) && proto_id_atk < MaxVehProtoFactionNum
            && (get_proto_offense_rating(proto_id_atk) < 0 || proto_id_atk == BSC_SPORE_LAUNCHER)) {
            morale++;
            int morale_active = range(PlayersData[faction_id_atk].soc_effect_active.morale, -4, 4);
            if (morale_active <= -2) {
                morale_active++;
            }
            morale -= morale_active;
        }
        if (unk_tgl) {
            int morale_pending = PlayersData[faction_id_atk].soc_effect_pending.morale;
            if (morale_pending >= 2 && morale_pending <= 3) {
                morale++;
            }
            if (veh_id_def >= 0) {
                if (Vehs[veh_id_def].faction_id) {
                    if ((proto_id_atk >= MaxVehProtoFactionNum
                        || (get_proto_offense_rating(proto_id_atk) >= 0
                            && proto_id_atk != BSC_SPORE_LAUNCHER))
                        && !has_abil(proto_id_atk, ABL_DISSOCIATIVE_WAVE)
                        && has_abil(Vehs[veh_id_def].proto_id, ABL_SOPORIFIC_GAS)) {
                        morale -= 2;
                    }
                } else {
                    morale++;
                }
            }
        }
    }
    if (unk_tgl) {
        morale = range(morale, 1, 6);
    }
    VehBasicBattleMorale[unk_tgl != 0] = morale; // shifted up from original
    morale += 6;
    uint32_t offense = offense_proto(proto_id_atk, veh_id_def, is_bombard);
    if (psi_combat_type) {
        offense = psi_factor(offense, faction_id_atk, true, false);
    }
    return offense * morale * 4;
}

/*
Purpose: Get the basic defense value for a defending unit with an optional attacker unit parameter.
// ORIGINAL: 0x00501940 ?get_basic_defense@@YAHHHHH@Z 0x00501940-0x00501D26
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
    uint32_t faction_id_def = Vehs[veh_id_def].faction_id;
    uint32_t proto_id_def = Vehs[veh_id_def].proto_id;
    uint32_t morale = faction_id_def ? morale_veh(veh_id_def, true, 0)
        : morale_alien(veh_id_def, veh_id_atk >= 0 ? Vehs[veh_id_atk].faction_id : -1);
    int base_id_def = base_at(Vehs[veh_id_def].x, Vehs[veh_id_def].y);
    if (base_id_def >= 0) {
        if (has_fac_built(FAC_CHILDREN_CRECHE, base_id_def)) {
            morale++;
            int morale_active = range(PlayersData[faction_id_def].soc_effect_active.morale, -4, 0);
            if (morale_active <= -2) {
                morale_active++;
            }
            morale -= morale_active;
            // Asymmetric with the `else if` below - see get_basic_offense
            // above for the disassembly evidence: the crèche-true fallthrough
            // only bumps morale by 1 for a brood pit, the full range()/cap
            // adjustment belongs to the crèche-false arm only.
            if (has_fac_built_call(FAC_BROOD_PIT, base_id_def)
                &&  proto_id_def < MaxVehProtoFactionNum
                && (get_proto_offense_rating(proto_id_def) < 0 || proto_id_def == BSC_SPORE_LAUNCHER)) {
                morale++;
            }
        } else if (has_fac_built_call(FAC_BROOD_PIT, base_id_def)
            &&  proto_id_def < MaxVehProtoFactionNum
            && (get_proto_offense_rating(proto_id_def) < 0 || proto_id_def == BSC_SPORE_LAUNCHER)) {
            morale++;
            int morale_active = range(PlayersData[faction_id_def].soc_effect_active.morale, -4, 4);
            if (morale_active <= -2) {
                morale_active++;
            }
            morale -= morale_active;
        }
        if (has_fac_built(FAC_HEADQUARTERS, base_id_def)) { // bug fix: per manual
            morale++; // "Units in a headquarters base automatically gain +1 Morale when defending."
        }
        int morale_pending = PlayersData[faction_id_def].soc_effect_pending.morale;
        if (morale_pending >= 2 && morale_pending <= 3) {
            morale++;
        }
        if (veh_id_atk >= 0 && !Vehs[veh_id_atk].faction_id) {
            morale++;
        }
    }
    if (veh_id_atk >= 0 && Vehs[veh_id_atk].faction_id && (proto_id_def >= MaxVehProtoFactionNum 
        || (get_proto_offense_rating(proto_id_def) >= 0 && proto_id_def != BSC_SPORE_LAUNCHER)) 
        && !has_abil(proto_id_def, ABL_DISSOCIATIVE_WAVE)
        && has_abil(Vehs[veh_id_atk].proto_id, ABL_SOPORIFIC_GAS)) {
        morale -= 2;
    }
    morale = range(morale, 1, 6);
    VehBasicBattleMorale[1] = morale;
    morale += 6;
    uint32_t plan_def = VehPrototypes[proto_id_def].plan;
    if (plan_def == PLAN_ALIEN_ARTIFACT) {
        return 1;
    }
    if (plan_def == PLAN_INFO_WARFARE && get_proto_defense_rating(proto_id_def) == 1
        // bug fix: added veh_id_atk bounds check to prevent potential arbitrary memory read
        && (veh_id_atk < 0 || VehPrototypes[Vehs[veh_id_atk].proto_id].plan != PLAN_INFO_WARFARE)) {
        return 1;
    }
    uint32_t defense = armor_proto(proto_id_def, veh_id_atk, is_bombard);
    if (psi_combat_type) {
        defense = psi_factor(defense, faction_id_def, false, proto_id_def == BSC_FUNGAL_TOWER);
    }
    return defense * morale;
}

/*
Purpose: Initialize or reset the battle related global variables.
// ORIGINAL: 0x00501D30 ?battle_init@@YAXXZ 0x00501D30-0x00501D47 BYTE_EXACT
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
    BOOL is_bombard = (is_artillery // added veh_id_atk bounds check
        || (veh_id_atk >= 0 && is_proto_missile(proto_id_atk))) ? true : false;
    uint32_t psi_combat_type = 0;
    if (!is_bombard || proto_id_atk == BSC_SPORE_LAUNCHER || proto_id_def == BSC_SPORE_LAUNCHER) {
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
            int rule_psi_atk = Players[faction_id_atk].rule_psi;
            if (rule_psi_atk) {
                add_bat(0, rule_psi_atk, label_get(342)); // "Gaian Psi Bonus"
            }
            if (has_project(SP_DREAM_TWISTER, faction_id_atk)) {
                add_bat(0, 50, label_get(343)); // "Dream Twist"
            }
            // PSI Defense?
            if (psi_combat_type) { // shouldn't this be & 2?
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
                    if (!has_abil(proto_id_def, ABL_DISSOCIATIVE_WAVE) && psi_combat_type 
                        && has_abil(proto_id_atk, ABL_EMPATHIC) 
                        && Rules.combat_pct_emp_song_atk_vs_psi) {
                        offense *= ((Rules.combat_pct_emp_song_atk_vs_psi + 100) / 100);
                        add_bat(0, Rules.combat_pct_emp_song_atk_vs_psi,
                            Ability[abil_index(ABL_EMPATHIC)].name);
                    }
                }
                if (Vehs[veh_id_atk].state & VSTATE_MADE_AIRDROP 
                    && has_abil(proto_id_atk, ABL_DROP_POD) && Rules.combat_pen_pct_atk_airdrop) {
                    offense = (100 - Rules.combat_pct_emp_song_atk_vs_psi) * offense / 100;
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
                    add_bat(0, Rules.combat_pen_pct_atk_airdrop,
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
                        offense *= (rule_off_bonus / 100);
                        add_bat(0, rule_off_bonus, label_get(1108)); // "Alien Offense"
                    }
                }
                if (psi_combat_type && faction_id_atk) {
                    int planet_atk = PlayersData[faction_id_atk].soc_effect_active.planet;
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
                    defense *= (rule_def_bonus / 100);
                    add_bat(1, rule_def_bonus, label_get(1109)); // "Alien Defense"
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
                        defense *= ((Rules.combat_pct_base_def + 100) / 100);
                    }
                    defense *= 4;
                } else {
                    uint32_t triad_def = get_proto_triad(proto_id_def);
                    uint32_t terrain_def = (triad_def == TRIAD_AIR) 
                        ? 2 : defense_value(faction_id_def, x_def, y_def, veh_id_def, veh_id_atk);
                    uint32_t alt_atk;
                    uint32_t alt_def;
                    if (veh_id_atk >= 0 && get_triad(veh_id_atk) == TRIAD_LAND) {
                        if (combat_type && Rules.combat_pct_art_bonus_lvl_alt 
                            && triad_def == TRIAD_LAND 
                            && (alt_atk = alt_at(Vehs[veh_id_atk].x, Vehs[veh_id_atk].y),
                                alt_def = alt_at(x_def, y_def), alt_atk > alt_def)) {
                            offense *= ((Rules.combat_pct_art_bonus_lvl_alt 
                                * (alt_atk - alt_def) + 100) / 100);
                            // "Altitude"
                            add_bat(0, Rules.combat_pct_art_bonus_lvl_alt, label_get(576));
                        }
                        if (Rules.combat_pct_mobile_open_ground && !combat_type && base_id_def < 0
                            && terrain_def == 2 
                            && rocky_at(x_def, y_def) < TERRAIN_BIT_ROCKY) {
                            uint32_t speed_atk = speed_proto(proto_id_atk);
                            if (speed_atk > Rules.move_rate_roads 
                                && speed_proto(proto_id_def) < speed_atk) {
                                offense *= ((Rules.combat_pct_mobile_open_ground + 100) / 100);
                                // "Mobile in open"
                                add_bat(0, Rules.combat_pct_mobile_open_ground, label_get(611));
                            }
                        }
                        if (Rules.combat_pct_def_vs_mobile_rough && !combat_type 
                            && (terrain_def > 2 || base_id_def >= 0) 
                            && rocky_at(x_def, y_def) < TERRAIN_BIT_ROCKY
                            && speed_proto(proto_id_atk) > Rules.move_rate_roads) {
                            defense *= ((Rules.combat_pct_def_vs_mobile_rough + 100) / 100);
                            // "Rough vs. Mobile" : "Mobile vs. Base"
                            add_bat(1, Rules.combat_pct_def_vs_mobile_rough, base_id_def < 0
                                ? label_get(548) : label_get(612));
                        }
                        if (Rules.combat_pct_atk_road && !combat_type) {
                            // TODO: add check road/tube Combat % -> attacking along road
                            offense *= ((Rules.combat_pct_atk_road + 100) / 100);
                            add_bat(0, Rules.combat_pct_atk_road, label_get(606)); // "Road Attack"
                        }

                        if (Rules.combat_pen_pct_atk_lwr_elev && !combat_type 
                            && altitude_at(x_def, y_def) 
                            > altitude_at(Vehs[veh_id_atk].x, Vehs[veh_id_atk].y)
                            && !has_abil(proto_id_atk, ABL_ANTIGRAV_STRUTS)) {
                            defense *= ((Rules.combat_pen_pct_atk_lwr_elev + 100) / 100);
                            // "Uphill"
                            add_bat(1, Rules.combat_pen_pct_atk_lwr_elev, label_get(441));
                        }
                        if (Rules.combat_pct_atk_higher_elev && !combat_type 
                            && altitude_at(Vehs[veh_id_atk].x, Vehs[veh_id_atk].y) 
                            > altitude_at(x_def, y_def) 
                            && !has_abil(proto_id_def, ABL_ANTIGRAV_STRUTS)) {
                            offense *= ((Rules.combat_pct_atk_higher_elev + 100) / 100);
                            add_bat(0, Rules.combat_pct_atk_road, label_get(330)); // "Downhill"
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
                        strcat(StringTemp, StringTable.get(int(VehBattleDisplayTerrain)));
                        strcat(StringTemp, ")");
                        add_bat(1, 10 * (5 * terrain_def - 10), StringTemp);
                    }
                    LPSTR display_def; // only one is displayed
                    uint32_t def_multi = 2;
                    if (bit_at(x_def, y_def) & BIT_BUNKER && (veh_id_atk < 0
                        || get_triad(veh_id_atk) != TRIAD_AIR)) {
                        def_multi = 3;
                        display_def = label_get(358); // "Bunker"
                    }
                    if (base_id_def) {
                        display_def = label_get(332); // "Base"
                        def_multi = 2;
                        if (veh_id_atk >= 0) {
                            uint32_t triad_atk = get_triad(veh_id_atk);
                            uint32_t fac_modifier = 0;
                            switch (triad_atk) {
                              case TRIAD_LAND:
                                if (has_fac_built(FAC_PERIMETER_DEFENSE, base_id_def)
                                    || has_project(SP_CITIZENS_DEFENSE_FORCE, faction_id_def)) {
                                    display_def = label_get(354); // "Perimeter"
                                    fac_modifier = 4;
                                }
                                break;
                              case TRIAD_SEA:
                                if (has_fac_built(FAC_NAVAL_YARD, base_id_def)
                                    || has_project(SP_MARITIME_CONTROL_CENTER, faction_id_def)) {
                                    display_def = Facility[FAC_NAVAL_YARD].name;
                                    fac_modifier = 4;
                                }
                                break;
                              case TRIAD_AIR:
                                if (has_fac_built(FAC_AEROSPACE_COMPLEX, base_id_def)
                                    || has_project(SP_CLOUDBASE_ACADEMY, faction_id_def)) {
                                    display_def = Facility[FAC_AEROSPACE_COMPLEX].name;
                                    fac_modifier = 4;
                                }
                                break;
                              default:
                                fac_modifier = def_multi;
                                break;
                            }
                            if (has_fac_built(FAC_TACHYON_FIELD, base_id_def)) {
                                fac_modifier += 2;
                                display_def = label_get(357); // "Tachyon"
                            }
                            if (fac_modifier < 2 || def_multi == 2
                                && (veh_id_atk < 0 || has_abil(proto_id_atk, ABL_BLINK_DISPLACER)
                                    && Rules.combat_pct_base_def)) {
                                add_bat(1, Rules.combat_pct_base_def, label_get(332)); // "Base"
                                defense *= ((Rules.combat_pct_base_def + 100) / 100);
                            }
                            if (veh_id_atk >= 0 && has_abil(proto_id_atk, ABL_BLINK_DISPLACER)) {
                                if (def_multi > 2) {
                                    def_multi = 2;
                                }
                                display_def = label_get(428); // "Base vs. Blink"
                            }
                            if (Rules.combat_pct_infantry_vs_base && !combat_type 
                                && base_id_def >= 0 && faction_id_atk 
                                && proto_id_atk > MaxVehProtoFactionNum
                                && get_proto_offense_rating(proto_id_atk) >= 0 
                                && proto_id_atk != BSC_SPORE_LAUNCHER 
                                && !get_proto_triad(proto_id_atk)
                                && Chassis[VehPrototypes[proto_id_atk].chassis_id].speed == 1) {
                                offense *= ((Rules.combat_pct_infantry_vs_base + 100) / 100);
                                // "Infantry vs. Base"
                                add_bat(0, Rules.combat_pct_infantry_vs_base, label_get(547));
                            }
                            uint32_t bit_def;
                            if (is_artillery && def_multi <= 2 && base_id_def < 0
                                && rocky_at(x_def, y_def) < TERRAIN_BIT_ROCKY
                                && (bit_def = bit_at(x_def, y_def),
                                    !(bit_def & BIT_FOREST)) && (bit_def & BIT_FUNGUS
                                        || altitude_at(x_def, y_def) 
                                        < ALT_BIT_OCEAN_SHELF)) {
                                def_multi = 3;
                                display_def = label_get(525); // "Open Ground"
                            }
                            defense *= def_multi;
                            if (def_multi > 2) {
                                add_bat(1, 10 * (5 * def_multi - 10), 
                                    StringTable.get(int(display_def)));
                            }
                        }
                    }
                }
                if (faction_id_def && veh_id_atk >= 0 && is_proto_missile(proto_id_atk)) {
                    for (int i = 0; i < 25; i++) {
                        int x_radius = xrange(x_def + RadiusOffsetX[i]);
                        int y_radius = y_def + RadiusOffsetY[i];
                        if (on_map(x_radius, y_radius)) {
                            int base_id_radius = base_at(x_radius, y_radius);
                            if (base_id_radius >= 0
                                && Bases[base_id_radius].faction_id_current == faction_id_def
                                && has_fac_built(FAC_FLECHETTE_DEFENSE_SYS, base_id_radius)) {
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
                            int x_radius = xrange(x_def + RadiusOffsetX[i]);
                            int y_radius = y_def + RadiusOffsetY[i];
                            uint32_t sensor_status;
                            if (on_map(x_radius, y_radius)
                                && (sensor_status = is_sensor(x_radius, y_radius), sensor_status)) {
                                BOOL has_sensor = false;
                                if (!is_ocean(x_radius, y_radius)) {
                                    int faction_id_terr = whose_territory(faction_id_def, x_radius, 
                                        y_radius, NULL, false);
                                    if (faction_id_terr < 0 
                                        || (uint32_t)faction_id_terr == faction_id_def) {
                                        has_sensor = true;
                                    }
                                } else {
                                    int base_id_find = base_find(x_radius, y_radius);
                                    // assumes will find a base?
                                    if (Bases[base_id_find].faction_id_current == faction_id_def) {
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
                        defense *= ((Rules.combat_pct_def_range_sensor + 100) / 100);
                        add_bat(1, Rules.combat_pct_def_range_sensor, label_get(613)); // "Sensor"
                    }
                    if (sensor_def & 2) {
                        defense *= ((Rules.combat_pct_def_range_sensor + 100) / 100);
                        add_bat(1, Rules.combat_pct_def_range_sensor, label_get(1123)); // "GSP"
                    }
                }
                if (!has_abil(proto_id_atk, ABL_DISSOCIATIVE_WAVE) 
                    && Rules.combat_pct_trance_def_vs_psi && veh_id_atk >= 0 && psi_combat_type & 1
                    && has_abil(proto_id_def, ABL_TRANCE)) {
                    defense *= ((Rules.combat_pct_trance_def_vs_psi + 100) / 100);
                    add_bat(1, Rules.combat_pct_trance_def_vs_psi, label_get(329)); // "Trance"
                }
                uint32_t armor_id_def = VehPrototypes[proto_id_def].armor_id;
                if (psi_combat_type & 1 && veh_id_atk >= 0 && (armor_id_def == ARM_RESONANCE_3_ARMOR
                    || armor_id_def == ARM_RESONANCE_8_ARMOR)) {
                    defense = 125 * defense / 100;
                    add_bat(1, 25, label_get(1111)); // "Resonance Def."
                }
                // add check veh_id_atk >= 0 to skip this entire code section? original jumps to end
                if (veh_id_atk >= 0 && get_proto_triad(proto_id_atk) == TRIAD_AIR
                    && has_abil(proto_id_atk, ABL_AIR_SUPERIORITY) && !psi_combat_type) {
                    if (get_proto_triad(proto_id_def) == TRIAD_AIR) {
                        int ground_strike_pen = Rules.combat_pen_pct_air_supr_vs_grnd;
                        if (ground_strike_pen) {
                            offense *= ((100 - ground_strike_pen) / 100);
                            add_bat(0, -ground_strike_pen, label_get(448)); // "Ground Strike"
                        }
                    } else {
                        int air_to_air = Rules.combat_pct_air_supr_vs_air;
                        if (air_to_air && !has_abil(proto_id_def, ABL_AIR_SUPERIORITY)) {
                            offense *= ((air_to_air + 100) / 100);
                            add_bat(0, air_to_air, label_get(449)); // "Air-to-Air"
                        }
                    }
                }
                // added in veh_id_atk bounds check
                if (veh_id_atk >= 0 && get_proto_triad(proto_id_atk) == TRIAD_AIR
                    && get_proto_triad(proto_id_def) == TRIAD_AIR
                    && has_abil(proto_id_def, ABL_AIR_SUPERIORITY)
                    && has_abil(proto_id_atk, ABL_AIR_SUPERIORITY) && !psi_combat_type
                    && !is_proto_missile(proto_id_atk) && !is_proto_missile(proto_id_def)
                    && Rules.combat_pct_air_supr_vs_air) {
                    defense *= ((Rules.combat_pct_air_supr_vs_air + 100) / 100);
                    add_bat(1, Rules.combat_pct_air_supr_vs_air, label_get(449)); // "Air-to-Air"
                }
                if (!get_proto_offense_rating(proto_id_def) 
                    && get_proto_defense_rating(proto_id_def) == 1 && (faction_id_atk 
                        || base_id_def < 0) && Rules.combat_pen_pct_non_cbt_def_vs_cbt) {
                    defense *= ((100 - Rules.combat_pen_pct_non_cbt_def_vs_cbt) / 100);
                    // "Non Combat"
                    add_bat(1, -Rules.combat_pen_pct_non_cbt_def_vs_cbt, label_get(439));
                }
                if (veh_id_atk >= 0 // added veh_id_atk check
                    && get_proto_triad(proto_id_atk) == TRIAD_SEA
                    && get_proto_triad(proto_id_def) == TRIAD_SEA && base_id_def >= 0 
                    && Rules.combat_pct_bonus_vs_ship_port) {
                    offense *= ((Rules.combat_pct_bonus_vs_ship_port + 100) / 100);
                    add_bat(0, Rules.combat_pct_bonus_vs_ship_port, label_get(335)); // "In Port"
                }
                if (armor_id_def == ARM_PULSE_3_ARMOR || armor_id_def == ARM_PULSE_8_ARMOR
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
                    defense *= ((Rules.combat_pct_com_jam_def_vs_mobl + 100) / 100);
                    add_bat(1, Rules.combat_pct_com_jam_def_vs_mobl, label_get(336)); // "Jammer"
                }
                if (!has_abil(proto_id_atk, ABL_DISSOCIATIVE_WAVE) 
                    && has_abil(proto_id_def, ABL_AAA) && get_proto_triad(proto_id_atk) == TRIAD_AIR
                    && Rules.combat_pct_aaa_bonus_vs_air) {
                    defense *= ((Rules.combat_pct_aaa_bonus_vs_air + 100) / 100);
                    add_bat(1, Rules.combat_pct_aaa_bonus_vs_air, label_get(337)); // "Tracking"
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
                        if (alt_def > alt_atk) { // ???
                            int alt_modifier_def = (alt_def - alt_atk) 
                                * Rules.combat_pct_art_bonus_lvl_alt;
                            defense *= ((alt_modifier_def + 100) / 100);
                            add_bat(1, alt_modifier_def, label_get(576)); // "Altitude"
                        }
                    } else {
                        int alt_mod_atk = (alt_atk - alt_def) * Rules.combat_pct_art_bonus_lvl_alt;
                        offense *= ((alt_mod_atk + 100) / 100);
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
//        veh_top() and proto_power(). veh_top() hand-inlined here, matching
//        spot_stack's precedent (its own record shows 0 real callers in the
//        whole image). proto_power() moved to veh.h as `MEASURED inline`
//        (see its own ORIGINAL marker) since it was a plain non-`inline`
//        function and so never an /Ob1 inline candidate at either of its
//        two callers. Call count now agrees (0 disagree); best flag set's
//        similarity 0.259 -> 0.275.
// symbol    ?best_defender@@YAIHHH@Z
// size      1483 bytes
// prototype int (__cdecl ?best_defender@@YAHHHH@Z)(int vehIDDef, int vehIDAtk, BOOL useArtillery)
// callers   1   call targets   6
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004E3A50 0x005015B0 0x00501DA0 0x005B9580 0x005BF1F0 0x005C0DB0
Return Value: Unit id of the best defender
Status: Complete - testing
*/
uint32_t __cdecl best_defender(int veh_id_def, int veh_id_atk, BOOL check_artillery) {
    int offense = veh_id_atk >= 0 
        ? get_basic_offense(veh_id_atk, veh_id_def, false, false, false) : 8;
    int x_def = Vehs[veh_id_def].x;
    int y_def = Vehs[veh_id_def].y;
    BOOL is_ocean_def = is_ocean(x_def, y_def);
    int base_id_def = base_at(x_def, y_def);
    int defender_search = -999;
    uint32_t best_def_veh_id = veh_id_def;
    // veh_top(), hand-inlined - matching spot_stack's precedent: image calls
    // it 0 times total (both its callers inline it).
    int top_veh_id;
    if (veh_id_def < 0) {
        top_veh_id = -1;
    } else {
        top_veh_id = veh_id_def;
        for (int j = Vehs[top_veh_id].prev_veh_id_stack; j >= 0; j = Vehs[j].prev_veh_id_stack) {
            top_veh_id = j;
        }
    }
    for (int i = top_veh_id; i >= 0; i = Vehs[i].next_veh_id_stack) {
        uint32_t proto_id_def = Vehs[i].proto_id;
        if ((get_proto_triad(proto_id_def) != TRIAD_LAND || !is_ocean_def && base_id_def >= 0)
            // added veh_id_atk bounds check
            && (veh_id_atk < 0 || VehPrototypes[Vehs[veh_id_atk].proto_id].plan != PLAN_INFO_WARFARE
                || VehPrototypes[proto_id_def].plan == PLAN_INFO_WARFARE)) {
            uint32_t combat_type = (can_arty(proto_id_def, true) && (veh_id_atk 
                || can_arty(Vehs[veh_id_atk].proto_id, true))) ? 2 : 0; // added bounds?
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
            uint32_t proto_pwr_def = proto_power(i);
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

/*
Purpose: Determine whether any enemy naval transports are carrying land units within range to 
         attack the specified base. If so, set the units to move towards the base.
// ORIGINAL: 0x00506490 ?invasions@@YAXH@Z 0x00506490-0x00506646
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
    uint32_t faction_id_base = Bases[base_id].faction_id_current;
    int16_t base_x = Bases[base_id].x;
    int16_t base_y = Bases[base_id].y;
    for (int i = 0; i < VehCurrentCount; i++) {
        uint32_t veh_faction_id = Vehs[i].faction_id;
        if (veh_faction_id && !is_human(veh_faction_id) && veh_faction_id != faction_id_base
            && !has_treaty(veh_faction_id, faction_id_base, DTREATY_TREATY)) {
            int veh_x = Vehs[i].x;
            int veh_y = Vehs[i].y;
            if (is_ocean(veh_x, veh_y) && veh_cargo(i) && stack_check(i, 3, TRIAD_LAND, -1, -1)) {
                int proximity = vector_dist(base_x, base_y, veh_x, veh_y);
                if (proximity <= (int)speed(i, false)) {
                    Vehs[i].order = ORDER_MOVE_TO;
                    Vehs[i].waypoint_x[0] = base_x;
                    Vehs[i].waypoint_y[0] = base_y;
                }
            }
        }
    }
}

/*
Purpose: Direct the unit to start moving automatically towards the specified tile.
// ORIGINAL: 0x00560AD0 ?go_to@@YAXHDHH@Z 0x00560AD0-0x00560B25 BYTE_EXACT
// size      85 bytes
// prototype void (__cdecl ?go_to@@YAXHDHH@Z)(int vehID, int8 type, int xCoord, int yCoord)
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
// BODY IN veh.h, as `MEASURED inline`: set_course's two calls to go_to are
// both inlined in the image (its own `calls` list has neither), while
// scenario.cpp's single call stays real - `osmx calls` on set_course names
// no go_to among its targets. LEVER: matches bitmask/battle_init above.

/*
Purpose: Wrap an x coordinate on a round map, as set_course does it.
Original Offset: n/a
Return Value: Wrapped x
Status: Complete

xrange() at 0048BEE0 cannot stand in, for one bit: it tests the whole of MapIsFlat while
set_course reads the low BYTE and tests bit zero (`mov cl, byte ptr [94988Ch] / test cl, 1`),
the same reading valid_patrol above and reset_territory use. The two agree on the 0 and 1 the
game stores there and disagree on everything else.
*/
static inline int course_xrange(int x) {
    if (!(MapIsFlat & 1)) {
        if (x >= 0) {
            if (x >= MapLongitudeBounds) {
                x -= MapLongitudeBounds;
            }
        } else {
            x += MapLongitudeBounds;
        }
    }
    return x;
}

/*
Purpose: Send the unit toward the given tile the long way round - by water - choosing the anchorage
         next to it that fronts the most of the destination's own landmass.
// ORIGINAL: 0x00564890 ?set_course@@YAXHDHH@Z 0x00564890-0x00564B82
// TRIED: call structure already matches (`osmx calls --all` lists only
//            base_at and base_on_sea, same as the image's own `calls` list),
//            so the gap is register/control-flow noise across a 754-byte,
//            21-candidate scoring loop, not a missing inline or a wrong
//            callee. Best flag set /O2 /Ob0 /Gy /GR- /Oy- /GX, 0.558
//            similar. Not chased further in this pass - the semantic content
//            is already verified (mutant sweep kills 28/30, two equivalent).
// size      754 bytes
// prototype void (__cdecl ?set_course@@YAXHDHH@Z)(int vehID, int8 type, int xCoord, int yCoord)
// callers   2   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004E3A50 0x0050DE50
Return Value: n/a
Status: Complete

The direct answer comes first and is narrow: only when the destination is in the SAME region as
the unit AND the unit is itself afloat - its own tile below ALT_BIT_SHORE_LINE - does this hand
the coordinates straight to go_to. Anything else is a crossing, and the rest of the body picks
where to cross to.

The candidates are the 21 tiles of the destination's base radius, and each must be water. Which
water is decided by what the unit is sitting in: a unit inside a base asks base_on_sea whether
that base touches the candidate's sea region, and a unit at sea requires the candidate to be in
its own region. That is the reachability test, and it is the only one - nothing here checks
distance or fuel.

Each surviving candidate is then scored by FRONTAGE: how many of its eight neighbours are land,
belong to the DESTINATION's region, and hold no base. Land is the same ALT_BIT_SHORE_LINE line
read the other way round, the region is the destination's rather than the candidate's, and the
base test is base_who() - which answers -1 both for an empty tile and for a base tile whose owner
nibble is 0xF, so only a real faction-owned base takes a neighbour out of the count.

The best frontage wins, and the starting bar depends on the destination itself: zero when the
destination is land, minus one when it is water. A water destination therefore accepts an
anchorage that fronts NOTHING, which is what lets a unit be sent to open sea at all; a land
destination will not move the unit unless some anchorage fronts at least one tile of it.

Ties keep the earlier candidate, and every improvement re-issues the order, so the unit ends up
aimed at the last candidate that strictly beat everything before it.

The x wrap is course_xrange above rather than xrange(), for the one bit that separates them.

Verification note: the sweep against recovery-gameplay-tests kills 28 of 30 valid mutants twice
over, and both survivors are equivalences rather than untested behaviour.

  - Widening the neighbour loop to nine reaches RadiusBase entry 8, which is (0, 0) - the
    anchorage itself. Three lines earlier the anchorage was required to be below
    ALT_BIT_SHORE_LINE, and a neighbour is required to be at or above it, so that iteration can
    never reach the count no matter what the map holds.
  - Swapping `best_frontage = frontage;` with the go_to call behind it. go_to writes Veh fields
    and reads Vehs and VehPrototypes; the assignment reads a local the callee cannot name and
    writes a local the callee cannot see. Neither statement observes the other's effect, in
    either order.
*/
void __cdecl set_course(int veh_id, char type, int x, int y) {
    Veh &veh = Vehs[veh_id];
    int veh_region = map_loc(veh.x, veh.y)->region;
    int base_id = base_at(veh.x, veh.y);
    Map *destination = map_loc(x, y);
    int dst_region = destination->region;
    if (veh_region == dst_region
        && (map_loc(veh.x, veh.y)->climate & 0xE0) < ALT_BIT_SHORE_LINE) {
        go_to(veh_id, type, x, y);
        return;
    }
    int best_frontage = ((destination->climate & 0xE0) >= ALT_BIT_SHORE_LINE) ? 0 : -1;
    for (int i = 0; i < 21; i++) {   // the destination's own base radius
        int x_radius = course_xrange(x + RadiusOffsetX[i]);
        int y_radius = y + RadiusOffsetY[i];
        if (!on_map(x_radius, y_radius)) {
            continue;
        }
        Map *anchorage = map_loc(x_radius, y_radius);
        if ((anchorage->climate & 0xE0) >= ALT_BIT_SHORE_LINE) {
            continue;
        }
        if (base_id >= 0) {
            if (!base_on_sea(base_id, anchorage->region)) {
                continue;
            }
        } else if (anchorage->region != veh_region) {
            continue;
        }
        int frontage = 0;
        for (int j = 0; j < 8; j++) {
            int x_adj = course_xrange(x_radius + RadiusBaseX[j]);
            int y_adj = y_radius + RadiusBaseY[j];
            if (!on_map(x_adj, y_adj)) {
                continue;
            }
            Map *shore = map_loc(x_adj, y_adj);
            if ((shore->climate & 0xE0) < ALT_BIT_SHORE_LINE) {
                continue;
            }
            if (shore->region != dst_region) {
                continue;
            }
            if (base_who(x_adj, y_adj) >= 0) {
                continue;
            }
            frontage++;
        }
        if (frontage > best_frontage) {
            best_frontage = frontage;
            go_to(veh_id, type, x_radius, y_radius);
        }
    }
}

/*
Purpose: Get the unit on the top of the stack.
// ORIGINAL: 0x00579920 ?veh_top@@YAHH@Z 0x00579920-0x0057995A BYTE_EXACT
// LEVER: the climb loop's induction variable is int16_t, not int - the field
//   (prev_veh_id_stack) is int16_t, and declaring the loop variable int forces
//   an immediate movsx the image defers until the value is used as an index.
// size      58 bytes
// prototype int (__cdecl ?veh_top@@YAHH@Z)(int vehID)
// callers   2   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Unit id if found, otherwise -1
Status: Complete
*/
int __cdecl veh_top(int veh_id) {
    if (veh_id < 0) {
        return -1;
    }
    int top_veh_id = veh_id;
    for (int16_t i = Vehs[top_veh_id].prev_veh_id_stack; i >= 0; i = Vehs[i].prev_veh_id_stack) {
        top_veh_id = i;
    }
    return top_veh_id;
}

/*
Purpose: Get the current moves left for the specified unit.
// ORIGINAL: 0x00579960 ?veh_moves@@YAHH@Z 0x00579960-0x00579998 BYTE_EXACT
// size      56 bytes
// prototype int (__cdecl ?veh_moves@@YAHH@Z)(int vehID)
// callers   1   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005C1540
Return Value: Remaining moves
Status: Complete
*/
// BODY IN veh.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


/*
Purpose: Get the specified unit's reactor power value from its prototype.
// ORIGINAL: 0x005799A0 ?proto_power@@YAHH@Z 0x005799A0-0x005799F9 BYTE_EXACT
// LEVER: neither of its two callers (best_defender 0x5044D0, action_home
//        0x4CBAA0) has it in their `calls` list - both inline it - but this
//        tree defined it as a plain (non-`inline`) function, so under /Ob1
//        (the default for every flag set but /Ob0) it was never a candidate
//        and both callers paid a real call the image does not make. `MEASURED
//        inline` keeps this address's own BYTE_EXACT claim measurable
//        (the /Ob0 flag sets still force it real for that purpose).
// size      89 bytes
// prototype int (__cdecl ?proto_power@@YAHH@Z)(int vehID)
// callers   1   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: Power
Status: Complete
*/
// BODY IN veh.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.

/*
Purpose: Determine whether the specified unit is eligible for a monolith morale upgrade.
// ORIGINAL: 0x00579F80 ?want_monolith@@YAHH@Z 0x00579F80-0x00579FE6 BYTE_EXACT
// size      102 bytes
// prototype int (__cdecl ?want_monolith@@YAHH@Z)(int vehID)
// callers   2   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005C0E40
Return Value: Is unit eligible for a monolith morale upgrade? true/false
Status: Complete
*/
BOOL __cdecl want_monolith(int veh_id) {
    if (!(Vehs[veh_id].state & VSTATE_MONOLITH_UPGRADED)
        && morale_veh(veh_id, true, 0) < MORALE_ELITE && Vehs[veh_id].morale < MORALE_ELITE
        && get_offense_rating(veh_id)) {
        return true;
    }
    return false;
}

/*
Purpose: Calculate the armor strategy for the specified armor id.
// ORIGINAL: 0x0057D270 ?arm_strat@@YAHHH@Z 0x0057D270-0x0057D2D5 BYTE_EXACT
// size      101 bytes
// prototype int (__cdecl ?arm_strat@@YAHHH@Z)(int armorID, int factionID)
// callers   7   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00501500
Return Value: Armor strategy
Status: Complete
*/
int __cdecl arm_strat(int armor_id, int faction_id) {
    if (!ExpansionEnabled && armor_id > ARM_PSI_DEFENSE) {
        return 1;
    }
    int8_t defense_rating = Armor[armor_id].defense_rating;
    if (defense_rating < 0) {
        return psi_factor(((int)Rules.psi_combat_ratio_def[TRIAD_LAND]
            * PlayersData[faction_id].enemy_best_weapon_value)
            / (int)Rules.psi_combat_ratio_atk[TRIAD_LAND], faction_id, false, false);
    }
    return defense_rating;
}

/*
Purpose: Calculate the weapon strategy for the specified weapon id.
// ORIGINAL: 0x0057D2E0 ?weap_strat@@YAHHH@Z 0x0057D2E0-0x0057D35D BYTE_EXACT
// LEVER: the `!ExpansionEnabled && (a || b || c)` guard was three SEPARATE early-return `if`s under one outer `if (!ExpansionEnabled)`, not one `||` chain - the image has THREE distinct `mov eax,1; pop ebp; ret` blocks (no shared tail), which VC6 only emits for three genuinely separate `return` statements. Same `int8_t` / `(int)` cast pair as arm_strat for the byte read and the signed division.
// size      125 bytes
// prototype int (__cdecl ?weap_strat@@YAHHH@Z)(int weaponID, int factionID)
// callers   7   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00501500
Return Value: Weapon strategy
Status: Complete
*/
int __cdecl weap_strat(int weapon_id, int faction_id) {
    if (!ExpansionEnabled) {
        if (weapon_id == WPN_RESONANCE_LASER) {
            return 1;
        }
        if (weapon_id == WPN_RESONANCE_BOLT) {
            return 1;
        }
        if (weapon_id == WPN_STRING_DISRUPTOR) {
            return 1;
        }
    }
    int8_t offense_rating = Weapon[weapon_id].offense_rating;
    if (offense_rating < 0) {
        return psi_factor(((int)Rules.psi_combat_ratio_atk[TRIAD_LAND]
            * PlayersData[faction_id].enemy_best_armor_value)
            / (int)Rules.psi_combat_ratio_def[TRIAD_LAND], faction_id, true, false);
    }
    return offense_rating;
}

/*
Purpose: Calculate the weapon value for the specified prototype.
// ORIGINAL: 0x0057D360 ?weap_val@@YAHHH@Z 0x0057D360-0x0057D3ED BYTE_EXACT
// size      141 bytes
// prototype int (__cdecl ?weap_val@@YAHHH@Z)(int protoID, int factionID)
// callers   5   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00501500
Return Value: Weapon value
Status: Complete
*/
int __cdecl weap_val(int proto_id, int faction_id) {
    // weap_strat() (0x0057D2E0, BYTE_EXACT as its own out-of-line function)
    // is hand-inlined here: the image writes its whole body out at this
    // call site (psi_factor() is the only call it keeps), rather than
    // calling 0x0057D2E0.
    int weapon_id = VehPrototypes[proto_id].weapon_id;
    if (!ExpansionEnabled) {
        if (weapon_id == WPN_RESONANCE_LASER) {
            return 1;
        }
        if (weapon_id == WPN_RESONANCE_BOLT) {
            return 1;
        }
        if (weapon_id == WPN_STRING_DISRUPTOR) {
            return 1;
        }
    }
    int8_t offense_rating = Weapon[weapon_id].offense_rating;
    if (offense_rating < 0) {
        return psi_factor(((int)Rules.psi_combat_ratio_atk[TRIAD_LAND]
            * PlayersData[faction_id].enemy_best_armor_value)
            / (int)Rules.psi_combat_ratio_def[TRIAD_LAND], faction_id, true, false);
    }
    return offense_rating;
}

/*
Purpose: Calculate the armor value for the specified armor id.
// ORIGINAL: 0x0057D3F0 ?arm_val@@YAHHH@Z 0x0057D3F0-0x0057D471 BYTE_EXACT
// size      129 bytes
// prototype int (__cdecl ?arm_val@@YAHHH@Z)(int armorID, int factionID)
// callers   3   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00501500
// LEVER: WRONG CALLEE - arm_strat() (0x0057D270, own out-of-line function)
//        is hand-inlined at this site, matching the image's one call
//        (psi_factor), not arm_strat. Best similarity 0.980 (14->44 of 48).
Return Value: Armor value
Status: Complete
*/
int __cdecl arm_val(int armor_id, int faction_id) {
    // arm_strat() (0x0057D270, own out-of-line function) is hand-inlined
    // here: the image writes its whole body out at this call site
    // (psi_factor() is the only call it keeps), rather than calling
    // 0x0057D270.
    int defense;
    if (faction_id >= 0) {
        if (!ExpansionEnabled && armor_id > ARM_PSI_DEFENSE) {
            defense = 1;
        } else {
            int8_t defense_rating = Armor[armor_id].defense_rating;
            if (defense_rating < 0) {
                defense = psi_factor(((int)Rules.psi_combat_ratio_def[TRIAD_LAND]
                    * PlayersData[faction_id].enemy_best_weapon_value)
                    / (int)Rules.psi_combat_ratio_atk[TRIAD_LAND], faction_id, false, false);
            } else {
                defense = defense_rating;
            }
        }
    } else {
        defense = Armor[armor_id].defense_rating;
    }
    return defense * 2;
}

/*
Purpose: Calculate the armor value for the specified prototype.
// ORIGINAL: 0x0057D480 ?armor_val@@YAHHH@Z 0x0057D480-0x0057D50D BYTE_EXACT
// size      141 bytes
// prototype int (__cdecl ?armor_val@@YAHHH@Z)(int protoID, int factionID)
// callers   2   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00501500
// LEVER: WRONG CALLEE - arm_val() (0x0057D3F0) and arm_strat() (0x0057D270)
//        are both hand-inlined at this site, matching the image's one call
//        (psi_factor), not arm_val. Best similarity 0.981.
Return Value: Armor value
Status: Complete
*/
int __cdecl armor_val(int proto_id, int faction_id) {
    // arm_val() (0x0057D3F0) and arm_strat() (0x0057D270) are both
    // hand-inlined here: the image writes the whole chain out at this call
    // site (psi_factor() is the only call it keeps), rather than calling
    // 0x0057D3F0.
    int armor_id = VehPrototypes[proto_id].armor_id;
    int defense;
    if (faction_id >= 0) {
        if (!ExpansionEnabled && armor_id > ARM_PSI_DEFENSE) {
            defense = 1;
        } else {
            int8_t defense_rating = Armor[armor_id].defense_rating;
            if (defense_rating < 0) {
                defense = psi_factor(((int)Rules.psi_combat_ratio_def[TRIAD_LAND]
                    * PlayersData[faction_id].enemy_best_weapon_value)
                    / (int)Rules.psi_combat_ratio_atk[TRIAD_LAND], faction_id, false, false);
            } else {
                defense = defense_rating;
            }
        }
    } else {
        defense = Armor[armor_id].defense_rating;
    }
    return defense * 2;
}

/*
Purpose: Calculate the carrying/transport capacity for the specified chassis, abilities and reactor.
// ORIGINAL: 0x0057D510 ?transport_val@@YAHHHH@Z 0x0057D510-0x0057D556 BYTE_EXACT
// LEVER: uint32_t -> int; image uses signed division (cdq/sub/sar) for `/2`, not shr
// size      70 bytes
// prototype int (__cdecl ?transport_val@@YAHHHH@Z)(int chassis, int ability, int reactor)
// callers   2   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: Transport capacity
Status: Complete
*/
int __cdecl transport_val(int chassis_id, int ability, int reactor_id) {
    int transport = Chassis[chassis_id].cargo;
    if (Chassis[chassis_id].triad == TRIAD_SEA) {
        transport *= reactor_id;
    }
    if (ability & ABL_SLOW) {
        transport /= 2; // -50%, rounded down
    }
    if (ability & ABL_HEAVY_TRANSPORT) {
        transport = (3 * transport + 1) / 2; // +50%, rounded up
    }
    return transport;
}

/*
Purpose: Generate offensive display string for the specified prototype. Replaced existing non-safe
         strcat with string. Original function took a 2nd parameter with char buffer and didn't
         return anything.
// ORIGINAL: 0x0057D560 ?say_offense@@YAXPADH@Z 0x0057D560-0x0057D6C5 BYTE_EXACT
// size      357 bytes
// prototype void (__cdecl ?say_offense@@YAXPADH@Z)(int8*, int protoID)
// callers   3   call targets   5
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005BF1F0 0x005C0DB0 0x005C1150 0x00645470 0x0064FC88
Return Value: Offensive display string
Status: Complete
*/
void __cdecl say_offense(LPSTR stat, int proto_id) {
    char num_buf[80];
    if (has_abil(proto_id, ABL_AIR_SUPERIORITY)) {
        strcat(stat, "<");
    }
    if (can_arty(proto_id, true)) {
        strcat(stat, "(");
    }
    int off_rating = offense_proto(proto_id, -1, 0);
    _itoa(off_rating, num_buf, 10);
    strcat(stat, num_buf);
    if (has_abil(proto_id, ABL_DROP_POD)) {
        strcat(stat, "^");
    }
    if (has_abil(proto_id, ABL_AMPHIBIOUS)) {
        strcat(stat, "~");
    }
    if (has_abil(proto_id, ABL_NERVE_GAS)) {
        strcat(stat, "x");
    }
    if (has_abil(proto_id, ABL_EMPATHIC)) {
        strcat(stat, "e");
    }
    if (has_abil(proto_id, ABL_BLINK_DISPLACER)) {
        strcat(stat, "!");
    }
    if (can_arty(proto_id, true)) {
        strcat(stat, ")");
    }
    if (has_abil(proto_id, ABL_AIR_SUPERIORITY)) {
        strcat(stat, ">");
    }
    uint8_t weapon_id = VehPrototypes[proto_id].weapon_id;
    if (weapon_id == WPN_RESONANCE_LASER || weapon_id == WPN_RESONANCE_BOLT) {
        strcat(stat, "r");
    }
}

/*
Purpose: Generate defense display string for the specified prototype. Replaced existing non-safe 
         strcat with string. Original function took a 2nd parameter with char buffer and didn't 
         return anything.
// ORIGINAL: 0x0057D6D0 ?say_defense@@YAXPADH@Z 0x0057D6D0-0x0057D7C4 BYTE_EXACT
// size      244 bytes
// prototype void (__cdecl ?say_defense@@YAXPADH@Z)(int8*, int protoID)
// callers   3   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005BF1F0 0x005C1290 0x00645470 0x0064FC88
Return Value: Defense display string
Status: Complete
*/
void __cdecl say_defense(LPSTR stat, int proto_id) {
    char num_buf[80];
    if (has_abil(proto_id, ABL_AAA)) {
        strcat(stat, "<");
    }
    int def_rating = armor_proto(proto_id, -1, 0);
    _itoa(def_rating, num_buf, 10);
    strcat(stat, num_buf);
    if (has_abil(proto_id, ABL_COMM_JAMMER)) {
        strcat(stat, "+");
    }
    if (has_abil(proto_id, ABL_TRANCE)) {
        strcat(stat, "t");
    }
    if (has_abil(proto_id, ABL_AAA)) {
        strcat(stat, ">");
    }
    if (VehPrototypes[proto_id].armor_id == ARM_PULSE_3_ARMOR
        || VehPrototypes[proto_id].armor_id == ARM_PULSE_8_ARMOR) {
        strcat(stat, "p");
    }
    if (VehPrototypes[proto_id].armor_id == ARM_RESONANCE_3_ARMOR
        || VehPrototypes[proto_id].armor_id == ARM_RESONANCE_8_ARMOR) {
        strcat(stat, "r");
    }
}

/*
Purpose: Generate stats string for specified prototype. Replaced existing non-safe strcat with 
         string. Reworked to integrate with existing C code.
// ORIGINAL: 0x0057D7D0 ?say_stats_3@@YAXPADH@Z 0x0057D7D0-0x0057D8D3
// TRIED: else-if branch order off_rating<0/off_rating<99/else folded the "?"
//            and "*" strcat tails together (29/92); reordering to
//            off_rating<0/off_rating>=99/else (matching the disasm's own branch
//            order) got to 51/92. def_rating stored to a named int8_t local, or
//            inlined as Armor[...].armor_id].defense_rating, made no further
//            difference; plateaus on the def_rating address computation
//            (shl 4 + disp vs add const + shl 4), 4 spellings tried.
// LEVER 2026-09-03: the speed division is SIGNED - `speed_proto(proto_id) /
//            (int)Rules.move_rate_roads` gives the image's `cdq / idiv`,
//            where the uncast `uint32_t` field gave `xor edx,edx / div`.
//            Same cast now at all three say_stats* sites (0x57D7D0,
//            0x57D8E0, 0x57DAA0).
// size      259 bytes
// prototype void (__cdecl ?say_stats_3@@YAXPADH@Z)(int8*, int protoID)
// callers   13   call targets   5
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0057D560 0x0057D6D0 0x005C13B0 0x00645470 0x0064FC88
Return Value: n/a
Status: Complete
*/
void __cdecl say_stats_3(LPSTR stat, int proto_id) {
    char num_buf[80];
    int8_t off_rating = get_proto_offense_rating(proto_id);
    if (off_rating < 0) {
        strcat(stat, "?"); // PSI
    } else if (off_rating >= 99) {
        strcat(stat, "*");
    } else {
        say_offense(stat, proto_id);
    }
    strcat(stat, "-");
    if (get_proto_defense_rating(proto_id) < 0) {
        strcat(stat, "?");
    } else {
        say_defense(stat, proto_id);
    }
    strcat(stat, "-");
    _itoa(speed_proto(proto_id) / (int)Rules.move_rate_roads, num_buf, 10);
    strcat(stat, num_buf);
    uint8_t reactor = VehPrototypes[proto_id].reactor_id;
    if (reactor > RECT_FISSION) {
        strcat(stat, "*");
        _itoa(reactor, num_buf, 10);
        strcat(stat, num_buf);
    }
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
Purpose: Generate stats string for specified prototype. List whether prototype is psi, sea or air.
         Replaced existing non-safe strcat with string. Reworked to integrate with existing C code.
// ORIGINAL: 0x0057D8E0 ?say_stats_2@@YAXPADH@Z 0x0057D8E0-0x0057DA94
// TRIED: label_get(196/163/162) calls (25/140); open-coding
//            StringTable.get((int)*((LPSTR*)Labels.strings_ptr + N)) plus the
//            off_rating<0/>=99/else branch order got to 58/140. def_rating via
//            get_proto_defense_rating() vs inlined Armor[...] made no further
//            difference; plateaus on the same shl-vs-add-const address split
//            seen at 0x0057D7D0. Call count is 19/21 (call_diff): same
//            get(Labels[196])+strcat 'Psi' CSE across the off_rating<0 and
//            def_rating<0 arms as 0x0057DAA0 - the compiler shares the pair,
//            the image keeps two call sites.
// size      436 bytes
// prototype void (__cdecl ?say_stats_2@@YAXPADH@Z)(int8*, int protoID)
// callers   4   call targets   6
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0057D560 0x0057D6D0 0x005C13B0 0x006169A0 0x00645470 0x0064FC88
Return Value: n/a
Status: Complete
*/
void __cdecl say_stats_2(LPSTR stat, int proto_id) {
    char num_buf[80];
    int8_t off_rating = get_proto_offense_rating(proto_id);
    if (off_rating < 0) {
        // INLINE: the image open-codes label_get(196) here rather than calling it.
        strcat(stat, StringTable.get((int)*((LPSTR *)Labels.strings_ptr + 196))); // 'Psi'
    } else if (off_rating >= 99) {
        strcat(stat, "*");
    } else {
        say_offense(stat, proto_id);
    }
    strcat(stat, "-");
    if (get_proto_defense_rating(proto_id) < 0) {
        strcat(stat, StringTable.get((int)*((LPSTR *)Labels.strings_ptr + 196))); // 'Psi'
    } else {
        say_defense(stat, proto_id);
    }
    strcat(stat, "-");
    _itoa(speed_proto(proto_id) / (int)Rules.move_rate_roads, num_buf, 10);
    strcat(stat, num_buf);
    uint32_t triad = get_proto_triad(proto_id);
    if (triad == TRIAD_SEA) {
        strcat(stat, " ");
        strcat(stat, StringTable.get((int)*((LPSTR *)Labels.strings_ptr + 163))); // 'Sea'
    } else if (triad == TRIAD_AIR) {
        strcat(stat, " ");
        strcat(stat, StringTable.get((int)*((LPSTR *)Labels.strings_ptr + 162))); // 'Air'
    }
    uint32_t reactor = VehPrototypes[proto_id].reactor_id;
    if (reactor > 1) {
        strcat(stat, "*");
        _itoa(reactor, num_buf, 10);
        strcat(stat, num_buf);
    }
}

/*
Purpose: Generate verbose stats string for specified prototype. Used by Design Workshop and Military
         Command Nexus. Replaced existing non-safe strcat with string. Reworked to integrate with
         existing C code.
// ORIGINAL: 0x0057DAA0 ?say_stats@@YAXPADHPAD@Z 0x0057DAA0-0x0057DED8
// symbol    ?say_stats@@YAXPADH0@Z
// TRIED: std::string body called 82 vs the image's 44 (call_diff); rebuilt
//            as char pointer + strcat/sprintf in the image's own branch shape (goto
//            shared "defense+spacer+speed" tail, matching the two physical
//            jumps into 0x57dd70) got the call count to 42/44. label_get(N)
//            calls had to become the open-coded
//            StringTable.get((int)*((LPSTR*)Labels.strings_ptr + N)) - the
//            image never calls a separate label helper here. The
//            `custom_spacer ? custom_spacer : "/"` after the offense append
//            is two strcat call sites in the image (if/else, not a ternary
//            merged into one call); the same spacer after the defense append
//            IS one call site (a shared "push ebx or literal, then push+call"
//            tail) - only the first needed splitting.
//            Still 2 calls short: the compiler CSEs the two identical
//            `StringTable.get(Labels[196])` + strcat 'Psi' pairs (one on the
//            off_rating<0 arm, one on the def_rating<0 arm) into one shared
//            block, where the image keeps them as two separate call sites.
//            No source spelling tried defeated that merge. Also plateaus on
//            prologue shape (push ebp/mov ebp,esp/sub esp,0x50 vs an
//            esp-relative frame) picked by the flag search's own similarity
//            metric; forcing /Oy- narrows but does not close it.
// size      1080 bytes
// prototype void (__cdecl ?say_stats@@YAXPADHPAD@Z)(int8*, int protoID, int8*)
// callers   2   call targets   6
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0057D560 0x0057D6D0 0x005C13B0 0x006169A0 0x00645470 0x0064FC88
Return Value: n/a
Status: Complete
*/
void __cdecl say_stats(LPSTR stat, int proto_id, LPSTR custom_spacer) {
    char num_buf[80];
    uint8_t plan = VehPrototypes[proto_id].plan;
    uint8_t chas = VehPrototypes[proto_id].chassis_id;
    uint8_t triad = Chassis[chas].triad;
    uint8_t mode = Weapon[VehPrototypes[proto_id].weapon_id].mode;
    int8_t off_rating = get_proto_offense_rating(proto_id);
    int8_t def_rating = get_proto_defense_rating(proto_id);
    if (plan == PLAN_RECONNAISANCE && triad == TRIAD_LAND && off_rating == 1 && def_rating == 1
        && !VehPrototypes[proto_id].ability_flags) {
        strcat(stat, StringTable.get((int)PlansFullName[3])); // 'Explore/Defense'
        strcat(stat, ", ");
    } else if (mode < 3) { // Projectile, energy, missile
        strcat(stat, StringTable.get((plan != PLAN_DEFENSIVE || (off_rating >= 0 && off_rating <= def_rating))
            ? (int)PlansShortName[plan] : (int)*((LPSTR *)Labels.strings_ptr + 312))); // 'Combat'
        strcat(stat, ", ");
    }
    if (off_rating < 0 || mode < 3) {
        if (off_rating < 0) {
            strcat(stat, StringTable.get((int)*((LPSTR *)Labels.strings_ptr + 196))); // 'Psi'
        } else {
            say_offense(stat, proto_id);
        }
        if (custom_spacer) {
            strcat(stat, custom_spacer);
        } else {
            strcat(stat, "/");
        }
        goto append_defense_tail;
    } else if (def_rating != 1 || VehPrototypes[proto_id].ability_flags || (Chassis[chas].speed != 1
        && (mode != WPN_MODE_TRANSPORT || chas != CHSI_FOIL))) {
        strcat(stat, StringTable.get(int(PlansShortName[mode])));
        if (plan == PLAN_NAVAL_TRANSPORT) {
            strcat(stat, "(");
            _itoa(VehPrototypes[proto_id].carry_capacity, num_buf, 10);
            strcat(stat, num_buf);
            strcat(stat, ")");
        }
        strcat(stat, ", ");
append_defense_tail:
        if (def_rating < 0) {
            strcat(stat, StringTable.get((int)*((LPSTR *)Labels.strings_ptr + 196))); // 'Psi'
        } else {
            say_defense(stat, proto_id);
        }
        strcat(stat, custom_spacer ? custom_spacer : "/");
        _itoa(speed_proto(proto_id) / (int)Rules.move_rate_roads, num_buf, 10);
        strcat(stat, num_buf);
    } else {
        strcat(stat, StringTable.get((int)PlansFullName[mode]));
        if (plan == PLAN_NAVAL_TRANSPORT) {
            strcat(stat, "(");
            _itoa(VehPrototypes[proto_id].carry_capacity, num_buf, 10);
            strcat(stat, num_buf);
            strcat(stat, ")");
        }
        if (triad) { // sea, air
            strcat(stat, ","); // Bug fix: removed extra space
        }
    }
    if (triad == TRIAD_SEA) {
        strcat(stat, " ");
        strcat(stat, StringTable.get((int)*((LPSTR *)Labels.strings_ptr + 163))); // 'Sea'
    } else if (triad == TRIAD_AIR) {
        strcat(stat, " ");
        strcat(stat, StringTable.get((int)*((LPSTR *)Labels.strings_ptr + 162))); // 'Air'
    }
    uint32_t reactor = VehPrototypes[proto_id].reactor_id;
    if (reactor > 1) {
        strcat(stat, " ");
        strcat(stat, "(");
        strcat(stat, "*");
        _itoa(reactor, num_buf, 10);
        strcat(stat, num_buf);
        strcat(stat, ")");
    }
}

/*
Purpose: Check against faction's available tech for best available reactor.
// ORIGINAL: 0x0057EFA0 ?best_reactor@@YAHH@Z 0x0057EFA0-0x0057EFE2 BYTE_EXACT
// LEVER: has_tech_call, not has_tech - the image CALLS 0x5B9F20 in the loop,
//        so the `MEASURED inline` in technology.h must be reached through the
//        non-inline forwarder; open-coded it cost the body its register
//        allocation (ebx for the argument) and 25 extra instructions (5/31 at
//        /O2 /Oy- before the switch).
// symbol    ?best_reactor@@YAIH@Z
// size      66 bytes
// prototype int (__cdecl ?best_reactor@@YAHH@Z)(int factionID)
// callers   3   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005B9F20
Return Value: Best reactor available (1-4)
Status: Complete
*/
uint32_t __cdecl best_reactor(int faction_id) {
    for (int i = MaxReactorNum - 1; i >= 0; i--) {
        if (has_tech_call(Reactor[i].preq_tech, faction_id)) {
            return i + 1;
        }
    }
    return RECT_FISSION; // default
}

/*
Purpose: Check against faction's available tech for best available chassis meeting requirements.
// ORIGINAL: 0x0057EFF0 ?pick_chassis@@YAHHHH@Z 0x0057EFF0-0x0057F0A1 BYTE_EXACT
// LEVER: has_tech_call, not has_tech - the image CALLS 0x5B9F20 in the loop,
//        so the `MEASURED inline` in technology.h must be reached through the
//        non-inline forwarder or /Ob1 open-codes it (3/70 at /O2 /Oy- before the switch).
// LEVER: declaration/store order (best_speed before chassis_id); comparison operand
//   order (`Chassis[i].triad == triad_chk`, not the reverse - the `cmp` LHS follows
//   source LHS); swapped the `speed_chk > 0`/`speed_chk <= 0` if/else so the `>0`
//   arm is checked first, matching the image's fallthrough; `continue` in place of
//   each branch's own duplicated `if (speed_compare > best_speed)` so the check
//   merges into the ONE shared block the image jumps to from both arms; and the
//   `speed_chk <= 0` arm rewritten as the image's own fallback chain (TRIAD_AIR /
//   -1-needs-missile / -2-needs-range==1 / else-compute) instead of a flat `||` -
//   the image has no explicit `!speed_chk` test, it just falls through once -1 and
//   -2 are ruled out.
// size      177 bytes
// prototype int (__cdecl ?pick_chassis@@YAHHHH@Z)(int factionID, int triadChk, int speedChk)
// callers   1   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005B9F20
Return Value: Best chassis available (0-8)
Status: Complete
*/
int __cdecl pick_chassis(int faction_id, int triad_chk, int speed_chk) {
    int best_speed = 0;
    int chassis_id = -1;
    for (int i = 0; i < MaxChassisNum; i++) {
        if (has_tech_call(Chassis[i].preq_tech, faction_id) && (triad_chk < 0
            || Chassis[i].triad == triad_chk) && (!Chassis[i].missile || speed_chk == -1)) {
            int speed_compare = 1;
            // The image reaches the best_speed check below through ONE shared block, from
            // either branch; where the source used to duplicate the check in each branch
            // (forcing the compiler to regenerate it twice, with different registers each
            // time), `continue` on the branch's own failure reproduces the image's direct
            // jump straight to the loop increment, letting the check merge into one copy.
            if (speed_chk > 0) { // 1, 2, 3
                if (Chassis[i].speed >= speed_chk) {
                    if (Chassis[i].speed == speed_chk) {
                        speed_compare = 2;
                    }
                } else {
                    continue;
                }
            } else { // 0, -1, -2
                // Written as the image's own fallback chain, not a flat `||`: when
                // triad_chk == TRIAD_AIR, speed_chk == -1 needs Chassis[i].missile and
                // speed_chk == -2 needs Chassis[i].range == 1; anything else (including
                // the assumed speed_chk == 0) falls through to compute unconditionally,
                // with no explicit `!speed_chk` test.
                if (triad_chk == TRIAD_AIR) {
                    if (speed_chk == -1) {
                        if (!Chassis[i].missile) {
                            continue;
                        }
                    } else if (speed_chk == -2) {
                        if (Chassis[i].range != 1) {
                            continue;
                        }
                    }
                }
                speed_compare = Chassis[i].speed + 1;
            }
            if (speed_compare > best_speed) {
                best_speed = speed_compare;
                chassis_id = i;
            }
        }
    }
    return chassis_id;
}

/*
Purpose: Check against faction's available tech for the best available weapon meeting requirements.
         The condition variable has a dual purpose of either max cost or search for 1st instance of
         a particular weapon mode.
// ORIGINAL: 0x0057F0B0 ?weapon_budget@@YAHHHH@Z 0x0057F0B0-0x0057F142 BYTE_EXACT
// LEVER: has_tech_call, not has_tech - the image CALLS 0x5B9F20 in the loop,
//        so the `MEASURED inline` in technology.h must be reached through the
//        non-inline forwarder or /Ob1 open-codes it (2/67 at /O2 /Oy- before the switch).
// LEVER: declaration/store order (best_offense before weapon_id, mirroring
//   pick_chassis's best_speed/chassis_id pair); swapped the `condition >= 0`/
//   `condition < 0` if/else so the `< 0` (break) arm is checked first, matching
//   the image's fallthrough; and `Weapon[i].mode < 2` spelled as `<= 1` - both
//   compile to the same `cmp byte, 1`, but only the `<=` spelling picks the `ja`
//   the image emits instead of `jae`.
// size      146 bytes
// prototype int (__cdecl ?weapon_budget@@YAHHHH@Z)(int factionID, int condition, BOOL checkMode)
// callers   2   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005B9F20
Return Value: Best weapon available (0-25)
Status: Complete
*/
int __cdecl weapon_budget(int faction_id, int condition, BOOL check_mode) {
    int best_offense = -1;
    int weapon_id = 0;
    for (int i = 0; i < MaxWeaponNum; i++) {
        if (has_tech_call(Weapon[i].preq_tech, faction_id)) {
            if (condition < 0) {
                if (Weapon[i].mode == -condition) {
                    weapon_id = i;
                    break;
                }
            } else {
                if ((!check_mode || Weapon[i].mode <= 1) && Weapon[i].cost <= condition
                    && Weapon[i].mode < 3 && Weapon[i].offense_rating < 99
                    && i != 23) { // Exclude Conventional Payload
                    int offense_compare = Weapon[i].offense_rating * 2;
                    if (offense_compare >= best_offense) {
                        best_offense = offense_compare;
                        weapon_id = i;
                    }
                }
            }
        }
    }
    return weapon_id;
}

/*
Purpose: Check against the faction's available tech for the best available armor meeting the cost 
         requirement.
// ORIGINAL: 0x0057F150 ?armor_budget@@YAHHH@Z 0x0057F150-0x0057F1CF BYTE_EXACT
// LEVER: has_tech_call, not has_tech - the image CALLS 0x5B9F20 in the loop,
//        so the `MEASURED inline` in technology.h must be reached through the
//        non-inline forwarder or /Ob1 open-codes it (5/53 at the exe's own
//        set). This also retires the "divergence starts at the has_tech() call
//        site (instr 17); not chased further" TRIED below - the call site was
//        the defect, and the fix is the forwarder.
// LEVER: store order - best_defense=-1 before armor_id=0, opposite of
//        declaration order (0.9524 -> 0.971 similar). TRIED: remaining
//        divergence starts at the has_tech() call site (instr 17); not
//        chased further.
// LEVER: the non-expansion override is an if/ELSE with the field read in the
//        else arm, not a load-then-overwrite. `int defense_compare =
//        Armor[i].defense_rating; if (...) defense_compare = 1;` sign-extends
//        the byte unconditionally (`movsx eax, al` before the branch, then
//        overwritten by the true arm); the image extends only in the else arm
//        and jumps over the movsx in the true arm. 26/53 -> 51/53.
// size      127 bytes
// prototype int (__cdecl ?armor_budget@@YAHHH@Z)(int factionID, int maxCost)
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005B9F20
Return Value: Best armor available (0-13)
Status: Complete
*/
int __cdecl armor_budget(int faction_id, int max_cost) {
    int best_defense = -1;
    int armor_id = 0;
    for (int i = 0; i < MaxArmorNum; i++) {
        if (has_tech_call(Armor[i].preq_tech, faction_id) && Armor[i].cost <= max_cost
            && Armor[i].defense_rating >= 0) { // excludes PSI
            int defense_compare;
            if (faction_id >= 0 && !ExpansionEnabled && i > 9) {
                defense_compare = 1; // Pulse + Resonance in non-expansion mode
            } else {
                defense_compare = Armor[i].defense_rating;
            }
            // This really only affects Pulse 8 Armor (Super Tensile Solids)
            defense_compare *= 2;
            if (defense_compare >= best_defense) {
                best_defense = defense_compare;
                armor_id = i;
            }
        }
    }
    return armor_id;
}

/*
Purpose: Get the index value of a particular ability's bitfield.
// ORIGINAL: 0x00581170 ?abil_index@@YAHH@Z 0x00581170-0x00581187 BYTE_EXACT
// size      23 bytes
// prototype int (__cdecl ?abil_index@@YAHH@Z)(int abilityID)
// callers   7   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: Ability index
Status: Complete
*/
// BODY IN veh.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


/*
Purpose: Calculate the movement penalty/cost.
// ORIGINAL: 0x00593510 ?hex_cost@@YAHHHHHHHH@Z 0x00593510-0x0059382F
// TRIED: tree's `proto_id != BSC_SEALURK // Bug fix` term has no matching `cmp` in the image - only ONE proto_id compare (`cmp eax, 9`, BSC_ISLE_OF_THE_DEEP) exists on this path. Removing the extra term is a structural fix (matches the image's condition), not a byte win by itself: 7/308 -> 8/308 agreeing under the best flag set. TRIED: hoisting `bit_src` next to `bit_dst` at the top (image DOES read the src tile's `bit` field before the is_ocean(dst) branch, per raw ebx allocation) - tried both declaration orders, both regressed (18/308 and 5/308) because it forces a second stack-resident local where the image keeps bit_src in a register (ebx) for the whole function and only spills bit_dst. The remaining gap past instruction 8 is VC6's own instruction scheduling across the three nested inlines (bit_at/is_ocean/altitude_at) - a scheduler decision, not reachable by reordering statements. Plateau at this structure.
// size      799 bytes
// prototype int (__cdecl ?hex_cost@@YAHHHHHHHH@Z)(int protoID, int factionID, int xCoordSrc, int yCoordSrc, int xCoordDst, int yCoordDst, int toggle)
// callers   5   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005BF1F0 0x005C13B0 0x00644F3A
Return Value: Movement cost
Status: Complete
*/
int __cdecl hex_cost(int proto_id, int faction_id, int x_src, int y_src, int x_dst,
                     int y_dst, BOOL toggle) {
    uint32_t bit_dst = bit_at(x_dst, y_dst);
    if (is_ocean(x_dst, y_dst)) {
        if (bit_dst & BIT_FUNGUS && altitude_at(x_dst, y_dst) == ALT_BIT_OCEAN_SHELF
            && get_proto_triad(proto_id) == TRIAD_SEA
            && proto_id != BSC_ISLE_OF_THE_DEEP && !has_project(SP_XENOEMPATYH_DOME, faction_id)) {
            return Rules.move_rate_roads * 3;
        }
        return Rules.move_rate_roads;
    }
    if (is_ocean(x_src, y_src)) {
        return Rules.move_rate_roads;
    }
    if (proto_id >= 0 && get_proto_triad(proto_id) != TRIAD_LAND) {
        return Rules.move_rate_roads;
    }
    // Land only conditions
    uint32_t bit_src = bit_at(x_src, y_src);
    if (bit_src & (BIT_MAGTUBE | BIT_BASE_IN_TILE) && bit_dst & (BIT_MAGTUBE | BIT_BASE_IN_TILE)
        && faction_id) {
        return 0;
    }
    if ((bit_src & (BIT_ROAD | BIT_BASE_IN_TILE) || bit_src & BIT_FUNGUS && faction_id > 0
        && has_project(SP_XENOEMPATYH_DOME, faction_id)) && bit_dst & (BIT_ROAD | BIT_BASE_IN_TILE)
        && faction_id) {
        return 1;
    }
    if (faction_id >= 0 && (has_project(SP_XENOEMPATYH_DOME, faction_id) || !faction_id
        || proto_id == BSC_MIND_WORMS || proto_id == BSC_SPORE_LAUNCHER) && bit_dst & BIT_FUNGUS) {
        return 1;
    }
    if (bit_src & BIT_RIVER && bit_dst & BIT_RIVER && x_dist(x_src, x_dst) == 1
        && abs((int)y_src - (int)y_dst) == 1 && faction_id) {
        return 1;
    }
    if (VehPrototypes[proto_id].chassis_id == CHSI_HOVERTANK
        || has_abil(proto_id, ABL_ANTIGRAV_STRUTS)) {
        return Rules.move_rate_roads;
    }
    uint32_t cost = Rules.move_rate_roads;
    if (rocky_at(x_dst, y_dst) > TERRAIN_BIT_ROLLING && !toggle) {
        cost += Rules.move_rate_roads;
    }
    if (bit_dst & BIT_FOREST && !toggle) {
        cost += Rules.move_rate_roads;
    }
    if (faction_id && bit_dst & BIT_FUNGUS && (proto_id >= MaxVehProtoFactionNum
        || get_proto_offense_rating(proto_id) >= 0)) {
        uint8_t plan = VehPrototypes[proto_id].plan;
        if (plan != PLAN_TERRAFORMING && plan != PLAN_ALIEN_ARTIFACT
            && PlayersData[faction_id].soc_effect_active.planet <= 0) {
            return cost + Rules.move_rate_roads * 2;
        }
        uint32_t speed = speed_proto(proto_id);
        if (cost <= speed) {
            return speed;
        }
    }
    return cost;
}

/*
Purpose: Relocate an existing unit to the specified tile.
// ORIGINAL: 0x005A59B0 ?veh_put@@YAXHHH@Z 0x005A59B0-0x005A59D2 BYTE_EXACT
// size      34 bytes
// prototype void (__cdecl ?veh_put@@YAXHHH@Z)(int vehID, int xCoord, int yCoord)
// callers   1   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005BFFA0 0x005C0080
Return Value: n/a
Status: Complete
*/
// BODY IN veh.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


/*
Purpose: Get the current health of the specified unit factoring in damage.
// ORIGINAL: 0x005A59E0 ?veh_health@@YAHH@Z 0x005A59E0-0x005A5A52 BYTE_EXACT
// LEVER: ternary -> if/else. The image branches on `plan != ALIEN_ARTIFACT`
// with `jne` to the range()*10 path and falls through `mov eax,1; jmp` on
// the equal side; a `cond ? range(...)*10 : 1` ternary compiled the
// opposite polarity (`je` past a shared *10). Splitting it into a plain
// if/else in the image's own branch order gives BYTE_EXACT at
// `/c /O2 /Gy /GR- /Oy- /GX` (the FRAMED set - `flags frame` above already
// says so).
// size      114 bytes
// prototype int (__cdecl ?veh_health@@YAHH@Z)(int vehID)
// callers   1   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: Unit health
Status: Complete
*/
int __cdecl veh_health(int veh_id) {
    int proto_id = Vehs[veh_id].proto_id;
    int health;
    if (VehPrototypes[proto_id].plan == PLAN_ALIEN_ARTIFACT) {
        health = 1;
    } else {
        health = range(VehPrototypes[proto_id].reactor_id, 1, 100) * 10;
    }
    return range(health - Vehs[veh_id].dmg_incurred, 0, 9999);
}

/*
Purpose: Calculates cost of the prototype based on various factors. Optimized logic flow from 
         the original without any differences to the final calculation.
// ORIGINAL: 0x005A5A60 ?proto_cost@@YAHHHHHH@Z 0x005A5A60-0x005A5CE1
// symbol    ?proto_cost@@YAIHHHHH@Z
// size      641 bytes
// prototype int (__cdecl ?proto_cost@@YAHHHHHH@Z)(int chassisType, int weapType, int armorType, int ability, int reactorType)
// callers   4   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
// TRIED: reordering the checks to the ORIGINAL's actual sequence (missile check AFTER
//   the triad-based armor/speed adjustments, not before; the AFLAG_COST_INC_LAND_UNIT
//   check as a second standalone loop over Ability[] that only runs for TRIAD_LAND,
//   not folded into the main loop; the all-four-costs==1 override applied at the very
//   END overriding a fully-computed proto_mod, not as an early-out) - confirmed by
//   reading the full disassembly instruction-by-instruction and cross-referencing struct
//   offsets (RulesChassis.missile @0x4D, RulesAbility.cost_factor via a pointer walked by
//   sizeof(RulesAbility)=0x1C). This IS the original's control flow, but agreement is
//   still only ~3/247 instructions: the image reserves locals with `sub esp,0x14` and
//   reuses parameter/local stack slots for later values (armor_id's slot becomes
//   speed_cost; weap_cost's slot becomes combat_mod) in a way this tree's compiler does
//   not reproduce from equivalent-looking C. Left at MISMATCH; the control-flow structure
//   is now faithful even though the byte encoding is not.
Return Value: Cost of prototype
Status: Complete
*/
uint32_t __cdecl proto_cost(int chassis_id, int weapon_id, int armor_id, 
                            int ability, int reactor_id) {
    uint8_t weap_cost = Weapon[weapon_id].cost;
    uint8_t armor_cost = Armor[armor_id].cost;
    int speed_cost = Chassis[chassis_id].cost;
    int abil_modifier = 0;
    if (ability) {
        for (int i = 0; i < MaxAbilityNum; i++) {
            if ((1 << i) & ability) {
                if (abil_modifier) {
                    abil_modifier++; // Increased cost with more than one ability
                }
                int factor = Ability[i].cost_factor;
                if (factor > 0) { // 1+ = Straight Cost; 25% increase per unit of cost
                    abil_modifier += factor;
                } else {
                    switch (factor) {
                      // Increases w/ ratio of weapon to armor: 0, 1, or 2. Rounded DOWN.
                      // Never higher than 2.
                      case -1:
                        if (armor_cost) {
                            abil_modifier += range(weap_cost / armor_cost, 0, 2);
                        }
                        break;
                      case -2: // Increases w/ weapon value
                        abil_modifier += weap_cost - 1;
                        break;
                      case -3: // Increases w/ armor value
                        abil_modifier += armor_cost - 1;
                        break;
                      case -4: // Increases w/ speed value
                        abil_modifier += speed_cost - 1;
                        break;
                      case -5: // Increases w/ weapon+armor value
                        abil_modifier += weap_cost + armor_cost - 2;
                        break;
                      case -6: // Increases w/ weapon+speed value
                        abil_modifier += weap_cost + speed_cost - 2;
                        break;
                      case -7: // Increases w/ armor+speed value
                        abil_modifier += armor_cost + speed_cost - 2;
                        break;
                      case 0: // None
                      default:
                        break;
                    }
                }
            }
        }
    }
    uint8_t triad = Chassis[chassis_id].triad;
    if (triad == TRIAD_SEA) {
        armor_cost /= 2;
        speed_cost += reactor_id;
    } else if (triad == TRIAD_AIR) {
        speed_cost += reactor_id * 2;
        if (armor_cost > 1) {
            armor_cost *= reactor_id * 2;
        }
    } else {
        // 010000000000 - Cost increased for land units; Deep Radar
        for (int i = 0; i < MaxAbilityNum; i++) {
            if ((1 << i) & ability) {
                if (Ability[i].flags & AFLAG_COST_INC_LAND_UNIT) {
                    abil_modifier++;
                }
            }
        }
    }
    if (Chassis[chassis_id].missile && Weapon[weapon_id].offense_rating >= 99) {
        return weap_cost;
    }
    uint32_t combat_mod = armor_cost / 2 + 1;
    if (combat_mod < weap_cost) { // which ever is greater
        combat_mod = weap_cost;
    }
    // (2 << n) == 2^(n + 1) ; (2 << n) / 2 == 2 ^ n;
    int proto_mod = ((speed_cost + armor_cost) * combat_mod + ((2 << reactor_id) / 2))
        / (2 << reactor_id);
    if (speed_cost == 1) {
        proto_mod = (proto_mod / 2) + 1;
    }
    if (Weapon[weapon_id].cost > 1 && Armor[armor_id].cost > 1) {
        proto_mod++;
        if (triad == TRIAD_LAND && speed_cost > 1) {
            proto_mod++;
        }
    }
    // excludes sea probes
    if (triad == TRIAD_SEA && Weapon[weapon_id].mode != WPN_MODE_INFOWAR) {
        proto_mod = (proto_mod + 1) / 2;
    } else if (triad == TRIAD_AIR) {
        proto_mod /= (Weapon[weapon_id].mode > WPN_MODE_MISSILE) ? 2 : 4; // Non-combat : Combat
    }
    int reactor_mod = (reactor_id * 3 + 1) / 2;
    if (proto_mod < reactor_mod) { // which ever is greater
        proto_mod = reactor_mod;
    }
    if (combat_mod == 1 && armor_cost == 1 && speed_cost == 1 && reactor_id == RECT_FISSION) {
        proto_mod = 1;
    }
    return (proto_mod * (abil_modifier + 4) + 2) / 4;
}

/*
Purpose: Calculates the base cost of the specified prototype.
// ORIGINAL: 0x005A5D00 ?base_cost@@YAHH@Z 0x005A5D00-0x005A5D3F BYTE_EXACT
// size      63 bytes
// prototype int (__cdecl ?base_cost@@YAHH@Z)(int protoID)
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005A5A60
Return Value: Base cost of the prototype
Status: Complete
*/
// BODY IN veh.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


/*
Purpose: Create a new prototype. Sets initial values for everything except veh_name and preq_tech.
// ORIGINAL: 0x005A5D40 ?make_proto@@YAXHHHHHH@Z 0x005A5D40-0x005A6269
// size      1321 bytes
// prototype void (__cdecl ?make_proto@@YAXHHHHHH@Z)(int protoID, int chassisType, int weapType, int armorType, int ability, int reactorType)
// callers   4   call targets   4
// kind      game
// flags     frame;sp_ready;purged_ok
// TRIED: nothing tried yet beyond inspection - call_diff agrees on call count/targets,
//   so the divergence (5/361 agreeing, ~0.14 similar) is all in the opening loop. The image
//   computes `i % MaxVehProtoFactionNum` there with the full MSVC signed-modulo-by-constant
//   sign-correction sequence (`and 0x8000003f; jns; dec; or 0xffffffc0; inc`), which this
//   loop's straightforward `i % MaxVehProtoFactionNum` on a provably-nonnegative `i` does not
//   emit - the compiler proves `i>=0` and just masks. Left as MISMATCH; the loop bound/shape
//   likely needs rewriting closer to the disassembly's own register-level computation
//   (proto_id itself, not `i`, appears to be the value being modulo'd in several places) to
//   close this, and that is a bigger rewrite than this pass budgeted for one function.
// calls     0x0057D510 0x005A5A60 0x005B9F20 0x005BF1F0
Return Value: n/a
Status: Complete
*/
void __cdecl make_proto(int proto_id, int chassis_id, int weapon_id, int armor_id, 
                        int ability, int reactor_id) {
    int unk_local_1 = 0; // TODO: Identify
    if (proto_id >= MaxVehProtoFactionNum) {
        BOOL cond1 = false;
        BOOL cond2 = false;
        BOOL cond3 = false;
        int proto_id_loop;
        for (int i = 0; i < 128; i++) {
            proto_id_loop = i % MaxVehProtoFactionNum;
            if ((i % MaxVehProtoFactionNum) != i) {
                proto_id_loop += proto_id - proto_id % MaxVehProtoFactionNum;
            }
            int flags_check = VehPrototypes[proto_id_loop].flags;
            if (flags_check & PROTO_ACTIVE) {
                if ((proto_id_loop <= MaxVehProtoFactionNum &&
                    has_tech(VehPrototypes[proto_id_loop].preq_tech, 
                        proto_id / MaxVehProtoFactionNum))
                    || (proto_id_loop > MaxVehProtoFactionNum
                        && (flags_check & PROTO_TYPED_COMPLETE))) {
                    uint32_t loop_weapon_id = VehPrototypes[proto_id_loop].weapon_id;
                    if (loop_weapon_id == weapon_id) {
                        cond1 = true;
                    }
                    uint32_t loop_armor_id = VehPrototypes[proto_id_loop].armor_id;
                    if (loop_armor_id == armor_id) {
                        cond2 = true;
                    }
                    uint32_t loop_chassis_id = VehPrototypes[proto_id_loop].chassis_id;
                    if (loop_chassis_id == chassis_id) {
                        cond3 = true;
                    }
                    int off_rating = Weapon[weapon_id].offense_rating;
                    if (off_rating > 0 && Weapon[loop_weapon_id].offense_rating >= off_rating) {
                        cond1 = true;
                    }
                    int def_rating = Armor[armor_id].defense_rating;
                    if (def_rating > 0 && Armor[loop_armor_id].defense_rating >= def_rating) {
                        cond2 = true;
                    }
                    if (Chassis[chassis_id].triad == Chassis[loop_chassis_id].triad
                        && Chassis[loop_chassis_id].speed >= Chassis[chassis_id].speed) {
                        cond3 = true;
                    }
                }
            }
        }
        if (cond1 && cond2 && cond3) {
            unk_local_1 = (proto_id_loop >= MaxVehProtoFactionNum
                && VehPrototypes[proto_id_loop].flags & 0x10) ? 3 : 1;
        }
    }
    VehPrototypes[proto_id].chassis_id = (uint8_t)chassis_id;
    VehPrototypes[proto_id].weapon_id = (uint8_t)weapon_id;
    VehPrototypes[proto_id].armor_id = (uint8_t)armor_id;
    VehPrototypes[proto_id].ability_flags = ability;
    VehPrototypes[proto_id].reactor_id = (uint8_t)reactor_id;
    VehPrototypes[proto_id].cost = (uint8_t)proto_cost(chassis_id, weapon_id, armor_id, ability,
        reactor_id);
    VehPrototypes[proto_id].carry_capacity = (Weapon[weapon_id].mode == WPN_MODE_TRANSPORT) 
        ? (uint8_t)transport_val(chassis_id, ability, reactor_id) : 0;
    // set plan & unk1
    if (Chassis[chassis_id].missile) {
        if (Weapon[weapon_id].offense_rating < 99) { // non-PB missiles
            if (weapon_id == WPN_TECTONIC_PAYLOAD) {
                VehPrototypes[proto_id].plan = PLAN_TECTONIC_MISSILE;
                VehPrototypes[proto_id].unk_1 = 22;
            } else if (weapon_id == WPN_FUNGAL_PAYLOAD) {
                VehPrototypes[proto_id].plan = PLAN_FUNGAL_MISSILE;
                VehPrototypes[proto_id].unk_1 = 23;
            } else { // Conventional Payload
                VehPrototypes[proto_id].plan = PLAN_OFFENSIVE;
                VehPrototypes[proto_id].unk_1 = 13;
            }
        } else {
            VehPrototypes[proto_id].plan = PLAN_PLANET_BUSTER;
            VehPrototypes[proto_id].unk_1 = 14;
        }
    } else if (Weapon[weapon_id].mode >= WPN_MODE_TRANSPORT) { // Non-combat
        VehPrototypes[proto_id].plan = Weapon[weapon_id].mode;
        VehPrototypes[proto_id].unk_1 = Weapon[weapon_id].mode + 0x20;
    } else if (Chassis[chassis_id].triad == TRIAD_SEA) { // combat sea
        VehPrototypes[proto_id].plan = PLAN_NAVAL_SUPERIORITY;
        VehPrototypes[proto_id].unk_1 = 6; // same value as plan
    } else if (Chassis[chassis_id].triad == TRIAD_AIR) { // combat air
        if (has_abil(proto_id, ABL_AIR_SUPERIORITY)) {
            VehPrototypes[proto_id].plan = PLAN_AIR_SUPERIORITY;
            VehPrototypes[proto_id].unk_1 = 9;
        } else {
            VehPrototypes[proto_id].plan = PLAN_OFFENSIVE;
            VehPrototypes[proto_id].unk_1 = 8;
        }
    } else { // TRIAD_LAND combat unit
        VehPrototypes[proto_id].plan = PLAN_OFFENSIVE;
        if (Armor[armor_id].defense_rating > 1) {
            VehPrototypes[proto_id].plan = PLAN_DEFENSIVE;
        }
        if (Weapon[weapon_id].offense_rating >= Armor[armor_id].defense_rating
            && VehPrototypes[proto_id].plan == PLAN_DEFENSIVE) {
            VehPrototypes[proto_id].plan = PLAN_COMBAT;
        }
        if (Chassis[chassis_id].speed > 1
            && Weapon[weapon_id].offense_rating > Armor[armor_id].defense_rating) {
            VehPrototypes[proto_id].plan = PLAN_OFFENSIVE;
        }
        if (ability & (ABL_ARTILLERY | ABL_DROP_POD | ABL_AMPHIBIOUS)) {
            VehPrototypes[proto_id].plan = PLAN_OFFENSIVE;
        }
        VehPrototypes[proto_id].unk_1 = 3;
        if (Armor[armor_id].defense_rating > Weapon[weapon_id].offense_rating) {
            VehPrototypes[proto_id].unk_1 = 2;
        }
        if (Weapon[weapon_id].offense_rating > Armor[armor_id].defense_rating * 2) {
            VehPrototypes[proto_id].unk_1 = 1;
        }
        // changed from '>= 2' since '>= 3' will always overwrite it
        if (Chassis[chassis_id].speed == 2) {
            VehPrototypes[proto_id].unk_1 = 4;
        } else if (Chassis[chassis_id].speed >= 3) { // added else, if speed <= 2 would always skip
            VehPrototypes[proto_id].unk_1 = 5;
        }
        if (Weapon[weapon_id].offense_rating == 1 && Armor[armor_id].defense_rating == 1) {
            if (Chassis[chassis_id].speed > 1) {
                VehPrototypes[proto_id].plan = PLAN_RECONNAISANCE;
            }
            if (has_abil(proto_id, ABL_POLICE_2X)) {
                VehPrototypes[proto_id].plan = PLAN_DEFENSIVE;
            }
            VehPrototypes[proto_id].unk_1 = 10;
        }
    }
    // set final values
    VehPrototypes[proto_id].obsolete_factions = 0;
    VehPrototypes[proto_id].combat_factions = (proto_id >= MaxVehProtoFactionNum) ?
        1 << (proto_id / MaxVehProtoFactionNum) : -1;
    VehPrototypes[proto_id].flags = unk_local_1 
        ? ((unk_local_1 & 2) ? 0x115 : 0x105) : PROTO_ACTIVE;
    VehPrototypes[proto_id].icon_offset = -1;
}

/*
Purpose: Search for the 1st prototype (basic + faction specific) that matches the specified plan.
// ORIGINAL: 0x005AED50 ?get_plan@@YAHHH@Z 0x005AED50-0x005AEDE0 BYTE_EXACT
// LEVER: has_tech_call, not has_tech - the image CALLS 0x5B9F20 at 0x5AEDA1,
//        so the `MEASURED inline` in technology.h must be reached through the
//        non-inline forwarder or /Ob1 open-codes it (3/54 at /O2 /Oy- before the switch).
// LEVER: proto_id is a signed `int`, not `uint32_t` - the image's
//        `cmp edi, 0x40 / jge` in front of the has_tech() guard is a SIGNED
//        test, and `uint32_t` gave `jae`. The parenthesisation
//        `faction_id * N + (i - N)` matters too: it keeps the shift on
//        faction_id alone and folds the -N into the following `lea`, where
//        the unparenthesised form hoists -N into the shift's operand
//        ((faction + 0x3ffffff) << 6) and pays an extra `add`.
// size      144 bytes
// prototype int (__cdecl ?get_plan@@YAHHH@Z)(int factionID, int plan)
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005B9F20
Return Value: Prototype id or -1 if not found
Status: Complete
*/
int __cdecl get_plan(int faction_id, int plan) {
    for (int i = 0; i < 128; i++) {
        int proto_id = (i < MaxVehProtoFactionNum) ? i
            : faction_id * MaxVehProtoFactionNum + (i - MaxVehProtoFactionNum);
        if (VehPrototypes[proto_id].flags & PROTO_ACTIVE
            && !(VehPrototypes[proto_id].obsolete_factions & (1 << faction_id))
            && (proto_id >= MaxVehProtoFactionNum
                || has_tech_call(VehPrototypes[proto_id].preq_tech, faction_id))
            && VehPrototypes[proto_id].plan == plan) {
            return proto_id;
        }
    }
    return -1;
}

/*
Purpose: Reveal the specified tile to the specified faction and keep the
         faction's remembered copy of the tile's terrain bits in step with it.
// ORIGINAL: 0x005B5700 sub_5b5700 0x005B5700-0x005B57CA BYTE_EXACT
// LEVER: synch_bit_call, not synch_bit - see the wrapper below. The image
//        CALLS 0x591E50 as this body's one genuine call, and at plain /O2 the
//        optimiser open-coded the `MEASURED inline` from map.h into the tail
//        (3/66 at /O2 /Oy- before the switch), which is also where the extra register
//        pressure came from. on_map() and map_loc() stay written out: the
//        image folds both in here.
// LEVER: FEWER (0 calls vs image's 1: synch_bit) at every flag set that kept a standalone symbol. /Ob0 forces the WHOLE function real, including on_map()/map_loc(), which the image folds in here; without /Ob0 the compiler inlines synch_bit() too, since it is small and `MEASURED inline` in map.h. Writing on_map()/map_loc() as bare expressions (not calls) removes them from the optimiser's inline/no-inline choice entirely, so /Ob0 only has synch_bit() left to force real.
// symbol    ?spot_tile@@YAXHHH@Z
// size      202 bytes
// prototype 
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00591E50
Return Value: n/a
Status: Complete

The image gives this function no symbol - the catalogue calls it sub_5b5700 -
so spot_tile is this recovery's name for it, chosen because it is the tile-level
member of the spot_*() family that surrounds it (005B57D0 spot_base, 005B58E0
spot_stack, 005B5AD0 spot_loc). Every one of those three carries this body
inlined verbatim, which is how it was identified: the one catalogued call site
is not the only place the bytes appear.

The dirty-tile arm is deliberately narrow. It fires only for the LOCAL faction,
only the first time that faction sees the tile, and not at all under omniscient
view or once the faction's map has been revealed outright - four conditions
which between them mean it is the interactive map's repaint hint rather than
game state. `bit2 |= 0x400000` and `UnkBitfield1 |= 1` are the same pair
climate_set() sets at 00591A80 and carry the same unidentified meaning.
*/
// LEVER (0x005B5700 spot_tile / 0x005B57D0 spot_base): synch_bit() has to
//        reach the image's real `call 0x591e50` at plain /O2, where the
//        optimiser otherwise open-codes the whole `MEASURED inline` body from
//        map.h into the caller and costs it the frame's spare registers
//        (spot_base was 5/78, spot_tile 3/66; both BYTE_EXACT through this
//        wrapper). VC6 declines to auto-inline a wrapper whose own body is the
//        expansion of another inline, so it stays a genuine call - the
//        same-TU counterpart of technology.h's has_tech_call, which needed a
//        different TU.
static void __cdecl synch_bit_call(int x, int y, int faction_id) {
    synch_bit(x, y, faction_id);
}

void __cdecl spot_tile(int x, int y, int faction_id) {
    // on_map() and map_loc() are written out here rather than called: the
    // image folds both of them in, and writing the expressions removes the
    // optimiser's inline/no-inline choice instead of hoping for it.
    // synch_bit_call() rather than synch_bit(): see the wrapper above.
    if (!(y >= 0 && y < (int)MapLatitudeBounds
          && x >= 0 && x < (int)MapLongitudeBounds)) {
        return;
    }
    Map *tile = map_tiles() + ((x >> 1) + y * MapLongitude);
    if (faction_id == ConsoleGlobal.faction_id_ && !(tile->visibility & (1 << faction_id))
        && !(GameState & STATE_OMNISCIENT_VIEW)
        && !(PlayersData[faction_id].flags & PFLAG_MAP_REVEALED)) {
        tile->bit2 |= 0x400000; // TODO: identify value
        UnkBitfield1 |= 1; // TODO: identify global + value
    }
    tile->visibility |= (uint8_t)(1 << faction_id);
    synch_bit_call(x, y, faction_id);
}

/*
Purpose: Reveal the specified base, and the tile it stands on, to the specified
         faction.
// ORIGINAL: 0x005B57D0 ?spot_base@@YAXHH@Z 0x005B57D0-0x005B58D2 BYTE_EXACT
// LEVER: synch_bit_call, not synch_bit - see the wrapper above. The
//        image CALLS 0x591E50 as this body's one genuine call, and open-coding
//        the `MEASURED inline` from map.h here cost the frame its spare
//        registers (5/78 at /O2 /Oy- before the switch). The hand-inlined spot_tile
//        body is also spelled in spot_tile's own guard-clause shape, which is
//        what the image's four separate `jl`/`jge` to the shared tail are.
// size      258 bytes
// prototype void (__cdecl ?spot_base@@YAXHH@Z)(int baseID, int factionID)
// callers   8   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00591E50
Return Value: n/a
Status: Complete

The population size is copied into the faction's intel slot at the moment of
sighting rather than read live, so a faction that loses sight of a base keeps
the size it last saw. The copy is unconditional: it happens again on every
sighting whether or not the visibility bit was already set.

There is no bounds check on base_id. All eight call sites pass an id that came
out of base_at() or a bounded loop.
*/
void __cdecl spot_base(int base_id, int faction_id) {
    Bases[base_id].visibility |= (uint8_t)(1 << faction_id);
    Bases[base_id].faction_pop_size_intel[faction_id] = Bases[base_id].population_size;
    int x = Bases[base_id].x;
    int y = Bases[base_id].y;
    // spot_tile(), hand-inlined in spot_tile's OWN shape - guard clause first,
    // the synch_bit call last - so the hand-inline reads exactly like the body
    // it duplicates. synch_bit_call() rather than synch_bit(): see the wrapper
    // above spot_tile.
    if (!(y >= 0 && y < (int)MapLatitudeBounds
          && x >= 0 && x < (int)MapLongitudeBounds)) {
        return;
    }
    Map *tile = map_tiles() + ((x >> 1) + y * MapLongitude);
    if (faction_id == ConsoleGlobal.faction_id_ && !(tile->visibility & (1 << faction_id))
        && !(GameState & STATE_OMNISCIENT_VIEW)
        && !(PlayersData[faction_id].flags & PFLAG_MAP_REVEALED)) {
        tile->bit2 |= 0x400000; // TODO: identify value
        UnkBitfield1 |= 1; // TODO: identify global + value
    }
    tile->visibility |= (uint8_t)(1 << faction_id);
    synch_bit_call(x, y, faction_id);
}

/*
Purpose: Reveal the whole of the specified unit's stack, and the tile it stands
         on, to the specified faction.
// ORIGINAL: 0x005B58E0 ?spot_stack@@YAXHH@Z 0x005B58E0-0x005B5A61
// size      385 bytes
// prototype void (__cdecl ?spot_stack@@YAXHH@Z)(int vehID, int factionID)
// callers   2   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00591E50
Return Value: n/a
Status: Complete

Two separate walks over two separate things. The tile is revealed once, from the
coordinates of the unit that was named; the visibility bit is then set on every
unit in the stack, found by climbing prev_veh_id_stack to the top and descending
next_veh_id_stack from there. The named unit need not be the top one, which is
why the climb exists. A negative id reveals no tile and marks no unit.

The original's `and word [veh+8], 0FBBFh` is written as the two named bits it
clears: 0xFBBF is exactly ~(VFLAG_LURKER | VFLAG_INVISIBLE) in sixteen bits, so
being seen stops a unit lurking and stops it being invisible. It is skipped for
faction zero, and only for faction zero; nothing else here distinguishes
factions.

The climb is veh_top() at 00579920, inlined, and is written as the call it was.
0x005B59F9 to 0x005B5A2A is that function instruction for instruction: the same
`test eax, eax / jl` for a negative id, the same `lea eax+eax*2 / lea eax+ecx*4`
stride, the same `movsx` of prev_veh_id_stack, the same loop. The negative test
that follows the climb is therefore not a dead branch on the climb's output - it
is the caller reading veh_top()'s -1 - and it is the `id >= 0` below.

The original tests on_map() twice before revealing the tile: once for the guard
written out below, and once inside the inlined spot_tile(). It is a pure
predicate over two globals and the same two coordinates, so the second test
cannot disagree with the first.

// LEVER: `calls` disagreed (2 here vs the image's 1: synch_bit only) because
//        spot_tile() and veh_top() were both left as real calls despite the
//        comment above already establishing both are inlined. spot_tile is
//        now plain `inline` (its only callers are spot_base/spot_stack/
//        spot_loc, all in this file); the veh_top climb below is
//        hand-inlined, matching veh_at's precedent.
// LEVER: the second `calls` defect - this tree made ZERO calls, the image
//        one - came from synch_bit() itself being folded in. The outer
//        on_map() guard was the cause: written as a call it kept synch_bit
//        out-of-line... no, the reverse. Writing on_map()'s four compares
//        out at BOTH guard sites (this one and spot_tile's own) is what
//        leaves synch_bit as the single genuine call; both sides now make
//        exactly one. Body still MISMATCH - the widened x/y loads and the
//        `push ecx` frame are allocator choices, not structure.
// TRIED: calling the `inline` spot_tile() directly - the optimiser did
// NOT fold it away here (unlike in spot_base), so the call to spot_tile
// survived and call_diff still reported WRONG CALLEE (spot_tile instead of
// synch_bit). Hand-inlining spot_tile's body, matching spot_base's own
// precedent, is what removes the call.
*/
void __cdecl spot_stack(int veh_id, int faction_id) {
    // The outer on_map() is written out rather than called, as in
    // spot_tile() itself: the image folds the four compares in here, and a
    // call to the `MEASURED inline` helper survived the optimiser at this
    // site. It is still tested twice - once by this guard and once by the
    // inlined spot_tile() body below.
    if (veh_id >= 0
        && Vehs[veh_id].y >= 0 && Vehs[veh_id].y < (int)MapLatitudeBounds
        && Vehs[veh_id].x >= 0 && Vehs[veh_id].x < (int)MapLongitudeBounds) {
        // spot_tile(), hand-inlined - see spot_base's precedent above; a call
        // to `inline` spot_tile survived the optimiser here.
        int x = Vehs[veh_id].x;
        int y = Vehs[veh_id].y;
        if (y >= 0 && y < (int)MapLatitudeBounds
            && x >= 0 && x < (int)MapLongitudeBounds) {
            Map *tile = map_tiles() + ((x >> 1) + y * MapLongitude);
            if (faction_id == ConsoleGlobal.faction_id_ && !(tile->visibility & (1 << faction_id))
                && !(GameState & STATE_OMNISCIENT_VIEW)
                && !(PlayersData[faction_id].flags & PFLAG_MAP_REVEALED)) {
                tile->bit2 |= 0x400000; // TODO: identify value
                UnkBitfield1 |= 1; // TODO: identify global + value
            }
            tile->visibility |= (uint8_t)(1 << faction_id);
            synch_bit(x, y, faction_id);
        }
    }
    int top_veh_id;
    if (veh_id < 0) {
        top_veh_id = -1;
    } else {
        top_veh_id = veh_id;
        for (int j = Vehs[top_veh_id].prev_veh_id_stack; j >= 0; j = Vehs[j].prev_veh_id_stack) {
            top_veh_id = j;
        }
    }
    for (int id = top_veh_id; id >= 0; id = Vehs[id].next_veh_id_stack) {
        Vehs[id].visibility |= (uint8_t)(1 << faction_id);
        if (faction_id) {
            Vehs[id].flags &= (uint16_t)~(VFLAG_LURKER | VFLAG_INVISIBLE);
        }
    }
}

/*
Purpose: Reveal whatever occupies the specified location - the tile itself, and
         then either the base standing on it or the whole unit stack - to the
         specified faction.
// ORIGINAL: 0x005B5AD0 ?spot_loc@@YAXHHH@Z 0x005B5AD0-0x005B5E05
// TRIED: byte-exactness, deliberately - not chased. The image INLINES
// spot_tile, spot_base and spot_stack whole (only base_at and veh_at
// survive as real calls), so a body that calls the five source-owned
// helpers by name - as this one does, on purpose, per the analysis below -
// diverges from instruction 0 (1/247 agreeing, push ebp/mov ebp,esp vs a
// register-argument prologue). Re-inlining all five to chase bytes would
// duplicate spot_tile/spot_base/spot_stack's own already-recovered bodies
// here; the call-graph transcription is judged the better trade and is
// left as MISMATCH.
// size      821 bytes
// prototype void (__cdecl ?spot_loc@@YAXHHH@Z)(int xCoord, int yCoord, int factionID)
// callers   6   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004E3A50 0x00591E50 0x005BFE90
Return Value: n/a
Status: Complete

821 bytes of original for six lines of source, because five of its six calls
were inlined and only base_at and veh_at survive as calls in the image. What
0x005B5AD0 to 0x005B5E04 actually contains, in order, is spot_tile at
0x005B5AD0, base_at, spot_base at 0x005B5BA7, veh_at, and spot_stack at
0x005B5CB4 - each one the whole of the corresponding function, instruction for
instruction, including spot_base's and spot_stack's own inlined copies of
spot_tile and spot_stack's inlined veh_top. Every one is now source-owned, so
this is a transcription of the call sequence rather than a re-inlining of it.

The evidence that these really are those functions and not merely similar code:
the base arm carries spot_base's unconditional intel copy from Base+6 to
Base+0Bh+faction and its SINGLE bounds test, while the unit arm carries
spot_stack's DOUBLED bounds test and its veh_top climb - a difference between
the two arms which has no reason to exist in one hand-written function and
exactly one reason to exist in two inlined ones.

A base wins outright: the unit arm is reached only when base_at answers -1, and
the tile reveal happens first either way, whether or not anything stands on it.
veh_at's own -1 is handed to spot_stack, which returns on a negative id, so
there is no third arm.
*/
void __cdecl spot_loc(int x, int y, int faction_id) {
    spot_tile(x, y, faction_id);
    int base_id = base_at(x, y);
    if (base_id >= 0) {
        spot_base(base_id, faction_id);
        return;
    }
    spot_stack(veh_at(x, y), faction_id);
}

/*
Purpose: Determine if the specified unit wants to wake up based on certain conditions and
         preferences. Optional parameter for spotted veh_id (-1 to skip).
// ORIGINAL: 0x005B5EA0 ?want_to_wake@@YAHHHH@Z 0x005B5EA0-0x005B6060
// TRIED: image never reads Vehs[veh_id].faction_id eagerly - it re-reads it at each conditional use, and never caches it across the early return. Un-caching it into `Vehs[veh_id].faction_id` at every site (instead of a hoisted `veh_faction_id` local) let the compiler pick a plain ebp-frame prologue matching the image's, where the hoisted local forced a param-less-locals, esp-relative frame from instruction 0. Also restored the `veh_id >= 0 &&` half of the trailing guard - the image's `test edi, edi; jl` tests veh_id, not just spotted_veh_id, contradicting this body's own comment that it was safe to drop. Residual divergence past this is register allocation (esi/edi/ebx/ebp swapped throughout) and one 16-bit shift-then-extend vs extend-then-shift on `waypoint_x[0] >> 1`; an explicit `(int)` cast on the base_who() arguments did not change the codegen. Best flag set /O2 /Gy /GR- /Oy- /GX, 0.793 similar.
// TRIED 2026-09-03: `uint8_t triad = get_triad(veh_id);` (the field's own
//        width, against the usual width lever) - the image keeps the byte in
//        `al` and compares `cmp al, 2`, but the narrower local measured WORSE:
//        0.790 against 0.793, because the byte local forces a second
//        zero-extension at the `triad != triad_spotted` compare further down.
//        Reverted.
// size      448 bytes
// prototype int (__cdecl ?want_to_wake@@YAHHHH@Z)(unsigned int factionID, unsigned int vehID, int vehID2)
// callers   1   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: Does unit want to wake? true/false
Status: Complete
*/
BOOL __cdecl want_to_wake(int faction_id, int veh_id, int spotted_veh_id) {
    uint32_t triad = get_triad(veh_id);
    if (triad == TRIAD_AIR && Vehs[veh_id].terraforming_turns
        && Vehs[veh_id].order == ORDER_MOVE_TO) {
        int base_faction_id = base_who(Vehs[veh_id].waypoint_x[0], Vehs[veh_id].waypoint_y[0]);
        if (base_faction_id >= 0
            && (((uint32_t)base_faction_id == Vehs[veh_id].faction_id)
                || has_treaty(Vehs[veh_id].faction_id, base_faction_id, DTREATY_PACT))) {
            return false;
        }
    }
    BOOL wants_to_wake;
    if (IsMultiplayerNet) { // restructured to be more efficient with same logic
        if (has_treaty(Vehs[veh_id].faction_id, faction_id, DTREATY_PACT)) {
            wants_to_wake = false;
        } else if (has_treaty(Vehs[veh_id].faction_id, faction_id, DTREATY_TREATY)) {
            wants_to_wake = true;
        } else if (has_treaty(Vehs[veh_id].faction_id, faction_id, DTREATY_TRUCE)) {
            wants_to_wake = true;
        } else {
            wants_to_wake = true;
        }
    } else {
        if (has_treaty(Vehs[veh_id].faction_id, faction_id, DTREATY_PACT)) {
            wants_to_wake = GamePreferences & PREF_AUTO_END_MOVE_SPOT_VEH_PACT;
        } else if (has_treaty(Vehs[veh_id].faction_id, faction_id, DTREATY_TREATY)) {
            wants_to_wake = GamePreferences & PREF_AUTO_END_MOVE_SPOT_VEH_TREATY;
        } else if (has_treaty(Vehs[veh_id].faction_id, faction_id, DTREATY_TRUCE)) {
            wants_to_wake = GamePreferences & PREF_AUTO_END_MOVE_SPOT_VEH_TRUCE;
        } else {
            wants_to_wake = GamePreferences & PREF_AUTO_END_MOVE_SPOT_VEH_WAR;
        }
        if (!(GamePreferences & PREF_AUTO_DONT_END_MOVE_DIFF_TRIAD)) {
            return wants_to_wake;
        }
    }
    if (veh_id >= 0 && spotted_veh_id >= 0) {
        uint32_t triad_spotted = get_triad(spotted_veh_id);
        if (triad != triad_spotted && ((Vehs[veh_id].state & (VSTATE_UNK_1000000 | VSTATE_UNK_200))
            != (VSTATE_UNK_1000000 | VSTATE_UNK_200) || triad_spotted == TRIAD_LAND)) {
            wants_to_wake = false;
        }
    }
    return wants_to_wake;
}

/*
Purpose: Wake up units meeting specific conditions from the stack where specified unit is located.
// ORIGINAL: 0x005B6060 ?wake_stack@@YAXH@Z 0x005B6060-0x005B619A
// LEVER: veh_top() is called at only 2 real sites in the image (see its own
//        comment on veh_top); here it is inlined, matching veh_at above. The
//        climb's induction variable is int16_t (prev_veh_id_stack's own type),
//        matching veh_top's own lever. is_ocean()/base_who()/bit_at() are hand-
//        inlined sharing ONE tile pointer, and map_loc()'s two globals
//        (map_tiles(), MapLongitude) are hoisted out of the `i` loop into locals
//        BEFORE it - the image reads them once, kept live in registers across
//        every iteration, rather than re-reading them on each tile lookup a
//        plain per-iteration map_loc(x, y) call would. Together: 0.836 -> 0.955.
// TRIED: an `int altitude = tile->climate & 0xE0;` local still narrows to a
//   byte compare (`cmp al`/`jb`) where the image has `cmp eax`/`jl` - VC6 proves
//   the range from the `& 0xE0` mask regardless of the local's declared width;
//   not reachable by re-typing. Left as the last four-instruction gap.
// size      314 bytes
// prototype void (__cdecl ?wake_stack@@YAXH@Z)(int vehID)
// callers   1   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl wake_stack(int veh_id) {
    if (veh_id < 0) {
        return;
    }
    int top_veh_id = veh_id;
    for (int16_t j = Vehs[top_veh_id].prev_veh_id_stack; j >= 0; j = Vehs[j].prev_veh_id_stack) {
        top_veh_id = j;
    }
    if (top_veh_id < 0) {
        return;
    }
    // map_loc()'s two globals (map_tiles(), MapLongitude), hand-hoisted out of the loop
    // below: the image reads them ONCE, before the loop even starts (kept live in
    // registers across every iteration), rather than re-reading them on every tile
    // lookup the way a plain per-iteration map_loc(x, y) call would.
    Map *const map_base = map_tiles();
    const int map_width = MapLongitude;
    for (int i = top_veh_id; i >= 0; i = Vehs[i].next_veh_id_stack) {
        int veh_id_way_point;
        int x;
        int y;
        // is_ocean()/base_who()/bit_at(), hand-inlined and sharing ONE tile pointer -
        // the image computes it once (kept live across the waypoint/get_triad checks
        // in between) and reads climate/val2/bit off it, rather than recomputing
        // map_loc() independently in each helper.
        Map *tile;
        int altitude;
        if (Vehs[i].order == ORDER_SENTRY_BOARD
            && (get_triad(i) || (x = Vehs[i].x, y = Vehs[i].y,
                tile = map_base + ((x >> 1) + y * map_width), altitude = tile->climate & 0xE0,
            altitude >= ALT_BIT_SHORE_LINE
            && ((veh_id_way_point = Vehs[i].waypoint_x[0], veh_id_way_point < 0)
                || get_triad(veh_id_way_point) != TRIAD_AIR
                || ((tile->bit & BIT_BASE_IN_TILE) && (tile->val2 & 0xF) < MaxPlayerNum)
                || (tile->bit & BIT_AIRBASE))))) {
            Vehs[i].order = ORDER_NONE;
            Vehs[i].state &= ~VSTATE_EXPLORE;
        }
    }
}

/*
Purpose: Move a stack of units in the same stack as the specified unit to a destination.
// ORIGINAL: 0x005B8AF0 ?stack_put@@YAXHHH@Z 0x005B8AF0-0x005B8B5D
// size      109 bytes
// prototype void (__cdecl ?stack_put@@YAXHHH@Z)(int vehID, int xCoord, int yCoord)
// callers   5   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005BFFA0 0x005C0080
// TRIED: `veh_top()`/`veh_put()` as plain calls - both are inline-marked
// wrappers, but this call site is not one of veh_top's real sites and
// veh_put did not fold here either, so call_diff reported WRONG CALLEE
// (veh_put/veh_top instead of veh_lift/veh_drop). Hand-inlining both,
// matching veh_promote's precedent, is what removes the mismatch.
Return Value: n/a
Status: Complete
*/
void __cdecl stack_put(int veh_id, int x, int y) {
    // veh_top(), hand-inlined - matching veh_promote's precedent (see its own
    // comment): the image does not call it here.
    int top_veh_id;
    if (veh_id < 0) {
        top_veh_id = -1;
    } else {
        top_veh_id = veh_id;
        for (int j = Vehs[top_veh_id].prev_veh_id_stack; j >= 0; j = Vehs[j].prev_veh_id_stack) {
            top_veh_id = j;
        }
    }
    int veh_id_loop;
    if (top_veh_id >= 0) {
        do {
            veh_id_loop = Vehs[top_veh_id].next_veh_id_stack;
            // veh_put(), hand-inlined: the image calls veh_lift and veh_drop
            // directly here, discarding veh_lift's return (it always returns
            // its own veh_id argument unchanged) rather than threading it
            // through to veh_drop's first parameter.
            veh_lift(top_veh_id);
            veh_drop(top_veh_id, x, y);
            top_veh_id = veh_id_loop;
        } while (veh_id_loop >= 0);
    }
}

/*
Purpose: Sort a stack of units with the transports moved to the top.
// ORIGINAL: 0x005B8B60 ?stack_sort@@YAXH@Z 0x005B8B60-0x005B8C8D
// TRIED: and `stack_put(veh_id_put, x, y)` were plain calls; call_diff's count-based check missed it (6 calls either way, since {veh_top, stack_put} happened to replace the {2 extra veh_lift/veh_drop} the image gets from inlining stack_put), but the image's actual 6 callees are veh_cargo/has_abil/veh_lift x2/ veh_drop x2 - no veh_top or stack_put. Hand-inlining both, including stack_put's OWN internal veh_top climb (its outer `next_veh_id >= 0` check survives - the image keeps it as a second, separate test after veh_top's own), took 0.627 -> 0.758.
// size      301 bytes
// prototype void (__cdecl ?stack_sort@@YAXH@Z)(int vehID)
// callers   2   call targets   4
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005BF1F0 0x005BFFA0 0x005C0080 0x005C1760
Return Value: n/a
Status: Complete
*/
void __cdecl stack_sort(int veh_id) {
    int16_t x = Vehs[veh_id].x;
    int16_t y = Vehs[veh_id].y;
    // veh_top(), hand-inlined - matching stack_fix's precedent (see its own
    // comment): the image does not call it here.
    int next_veh_id;
    if (veh_id < 0) {
        next_veh_id = -1;
    } else {
        next_veh_id = veh_id;
        for (int i = Vehs[next_veh_id].prev_veh_id_stack; i >= 0; i = Vehs[i].prev_veh_id_stack) {
            next_veh_id = i;
        }
    }
    int veh_id_put = -1;
    int veh_id_loop;
    if (next_veh_id >= 0) {
        do {
            veh_id_loop = Vehs[next_veh_id].next_veh_id_stack; // removed redundant < 0 check
            if (veh_cargo(next_veh_id) || has_abil(Vehs[next_veh_id].proto_id, ABL_CARRIER)) {
                veh_id_put = next_veh_id;
                veh_put(next_veh_id, -3, -3);
            }
            next_veh_id = veh_id_loop;
        } while (veh_id_loop >= 0);
        // stack_put(), hand-inlined - the image inlines this call site too,
        // including its own inlined veh_top() climb on veh_id_put.
        int top_veh_id;
        if (veh_id_put < 0) {
            top_veh_id = -1;
        } else {
            top_veh_id = veh_id_put;
            for (int j = Vehs[top_veh_id].prev_veh_id_stack; j >= 0; j = Vehs[j].prev_veh_id_stack) {
                top_veh_id = j;
            }
        }
        if (top_veh_id >= 0) {
            int veh_id_loop2;
            do {
                veh_id_loop2 = Vehs[top_veh_id].next_veh_id_stack;
                veh_put(top_veh_id, x, y);
                top_veh_id = veh_id_loop2;
            } while (veh_id_loop2 >= 0);
        }
    }
}

/*
Purpose: Sort a stack of units with colony pods at the top followed by combat/offensive units.
// ORIGINAL: 0x005B8C90 ?stack_sort_2@@YAXH@Z 0x005B8C90-0x005B8E0A BYTE_EXACT
// LEVER: hand-inlined veh_top()'s climb and stack_put()'s call+its own veh_top()
//   climb, matching stack_sort's precedent (veh_put() itself stays a real call,
//   it MEASURED-inlines to veh_lift/veh_drop on its own). Moved veh_id_put's
//   declaration to the top of the function, matching store order in the image.
//   The image re-derives `next_veh_id_stack` through a `(id < 0) ? id :
//   Vehs[id].next_veh_id_stack` ternary even though the guard is provably true
//   at every use - reproduced verbatim, at all three next_veh_id_stack reads.
//   The two prev_veh_id_stack climb loops' induction variables are int16_t. not
//   int: the field is int16_t, and declaring the induction var int forces an
//   immediate movsx the image defers until the value is used as an array index.
// size      378 bytes
// prototype void (__cdecl ?stack_sort_2@@YAXH@Z)(int vehID)
// callers   2   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005BFE90 0x005BFFA0 0x005C0080
Return Value: n/a
Status: Complete
*/
void __cdecl stack_sort_2(int veh_id) {
    int veh_id_put = -1;
    int x = Vehs[veh_id].x;
    int y = Vehs[veh_id].y;
    // veh_top(), hand-inlined - matching stack_sort's precedent (see its own
    // comment): the image does not call it here.
    int next_veh_id;
    if (veh_id < 0) {
        next_veh_id = -1;
    } else {
        next_veh_id = veh_id;
        for (int16_t i = Vehs[next_veh_id].prev_veh_id_stack; i >= 0; i = Vehs[i].prev_veh_id_stack) {
            next_veh_id = i;
        }
    }
    int veh_id_loop;
    if (next_veh_id >= 0) {
        do {
            // The image re-guards `next_veh_id < 0` here even though it is provably
            // true from the loop's own entry/continuation test; reproduced as the
            // same ternary the compiler folded into this exact branch shape.
            veh_id_loop = (next_veh_id < 0) ? next_veh_id : Vehs[next_veh_id].next_veh_id_stack;
            if (VehPrototypes[Vehs[next_veh_id].proto_id].plan == PLAN_COLONIZATION) { // Colony Pod
                veh_id_put = next_veh_id;
                veh_put(next_veh_id, -3, -3);
            }
            next_veh_id = veh_id_loop;
        } while (veh_id_loop >= 0);
    }
    next_veh_id = veh_at(x, y);
    if (next_veh_id >= 0) {
        do {
            veh_id_loop = (next_veh_id < 0) ? next_veh_id : Vehs[next_veh_id].next_veh_id_stack;
            if (VehPrototypes[Vehs[next_veh_id].proto_id].plan <= PLAN_COMBAT) {
                veh_id_put = next_veh_id;
                veh_put(next_veh_id, -3, -3);
            }
            next_veh_id = veh_id_loop;
        } while (veh_id_loop >= 0);
    }
    // stack_put(), hand-inlined - the image inlines this call site too,
    // including its own inlined veh_top() climb on veh_id_put.
    int top_veh_id;
    if (veh_id_put < 0) {
        top_veh_id = -1;
    } else {
        top_veh_id = veh_id_put;
        for (int16_t j = Vehs[top_veh_id].prev_veh_id_stack; j >= 0; j = Vehs[j].prev_veh_id_stack) {
            top_veh_id = j;
        }
    }
    if (top_veh_id >= 0) {
        int veh_id_loop2;
        do {
            veh_id_loop2 = (top_veh_id < 0) ? top_veh_id : Vehs[top_veh_id].next_veh_id_stack;
            veh_put(top_veh_id, x, y);
            top_veh_id = veh_id_loop2;
        } while (veh_id_loop2 >= 0);
    }
}

/*
Purpose: Refresh and fix the stack. Used by DirectPlay multiplayer only.
// ORIGINAL: 0x005B8E10 ?stack_fix@@YAHH@Z 0x005B8E10-0x005B8ED9
// TRIED: frame pointer - the image keeps ebp (a `push ecx` local-space idiom) and duplicate non-merged epilogues; no flag set in the harness reproduces both at once here - /Oy- restores ebp but the two early returns cross-jump to one shared tail instead of duplicating, unlike the image. Plateau at 0.758 (/O2 /Gy /GR- /GX).
// TRIED: is called at only 2 real sites in the image (see its own comment on veh_top); here it is inlined, matching veh_at above.
// TRIED: combined `if (a || b || c) return veh_id;` compiled to one merged block; the image duplicates the epilogue three times (three separate early returns, each its own `jge`). Splitting into three sequential `if`s took similarity 0.627 -> 0.758 (best flag set has 0 raw agree but is the closest shape - see BUG note below for the loop, which is the bulk of the remaining gap).
// BUG IN THE ORIGINAL: the promote/sort loop compares `Vehs[veh_id]`'s own
//        x/y against themselves, not the loop iterator `Vehs[i]`'s - always
//        true, so `veh_promote(i)`/`stack_sort(veh_id)` fire on EVERY
//        iteration. Reproduced deliberately; do not "fix" to `Vehs[i]`.
// size      201 bytes
// prototype int (__cdecl ?stack_fix@@YAHH@Z)(int vehID)
// callers   26   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005B8B60 0x005C0260
Return Value: Either the parameter unit id or unit id of the stack top; Return is checked if >= 0
Status: Complete
*/
int __cdecl stack_fix(int veh_id) {
    if (veh_id < 0) {
        return veh_id; // invalid veh_id
    }
    if (!IsMultiplayerNet) {
        return veh_id; // not DirectPlay MP
    }
    if (Vehs[veh_id].next_veh_id_stack < 0 && Vehs[veh_id].prev_veh_id_stack < 0) {
        return veh_id; // no stack
    }
    for (int i = 0; i < VehCurrentCount; i++) {
        // BUG IN THE ORIGINAL: compares veh_id's own coordinates against
        // themselves, not the iterator's - always true, so veh_promote(i)/
        // stack_sort(veh_id) run on every iteration of this loop.
        if (Vehs[veh_id].x == Vehs[veh_id].x && Vehs[veh_id].y == Vehs[veh_id].y) {
            veh_promote(i);
            stack_sort(veh_id);
        }
    }
    if (veh_id < 0) {
        return -1;
    }
    int top_veh_id = veh_id;
    for (int j = Vehs[top_veh_id].prev_veh_id_stack; j >= 0; j = Vehs[j].prev_veh_id_stack) {
        top_veh_id = j;
    }
    return top_veh_id;
}

/*
Purpose: Board the eligible units stacked with the specified transport or carrier, optionally
         lifting the whole stack off the map afterwards. Mode 0 marks and counts without moving
         anything; mode 1 sorts the stack first and lifts each boarder plus the subject to the
         off-map holding square; mode 2+ makes a single pass and short-circuits to the lift when
         the subject already sits on a faction-owned base.
// ORIGINAL: 0x005B8EE0 ?stack_veh@@YAHHH@Z 0x005B8EE0-0x005B950F
// TRIED: by 1 (17 vs image's 16) - two `veh_top(veh_id)` calls (the mode-0 refresh loop and the per-pass boarding loop's start index) hand-inlined, matching stack_fix's precedent, and `sleep(i)` forced to a real call via a new `sleep_call` forwarder in veh.h/veh.cpp (its only other reference is the inline body - under /O2 `sleep(i)` folds to 3 field writes, but the image calls it for real at this one site). call_diff now agrees (0 disagreeing, was MORE by 1). This is a 472-instruction function; the remaining gap is per-branch register/scheduling noise not chased further at this budget.
// size      1583 bytes
// prototype int (__cdecl ?stack_veh@@YAHHH@Z)(int vehID, int)
// callers   10   call targets   7
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005B8B60 0x005B8E10 0x005BF1F0 0x005BFFA0 0x005C0080 0x005C01A0 0x005C1760
Return Value: Remaining capacity for mode 0, otherwise the unit id
Status: Complete
*/
int __cdecl stack_veh(int veh_id, int mode) {
    if (veh_id < 0) {
        return mode ? veh_id : 0;
    }
    if (!veh_cargo(veh_id) && !has_abil(Vehs[veh_id].proto_id, ABL_CARRIER)) {
        // Neither a transport nor a carrier: nothing can be stacked onto it.
        return mode ? veh_drop(veh_lift(veh_id), -2, -2) : 0;
    }
    if (IsMultiplayerNet) {
        stack_fix(veh_id); // return value is discarded by the original
    }
    if (mode) {
        if (mode > 1) {
            // Unguarded on purpose: the original reads this tile without an on_map() check and
            // halves x with an arithmetic shift, so map_loc()'s uint32_t parameter would send a
            // negative x roughly a gigabyte away. Same for the two other unguarded reads below.
            const Map *tile = &map_tiles()[(Vehs[veh_id].x >> 1)
                + Vehs[veh_id].y * (int)MapLongitude];
            // The original also compares the masked nibble against 0, which is vacuous.
            if ((tile->bit & BIT_BASE_IN_TILE) && (tile->val2 & 0xF) < MaxPlayerNum) {
                return veh_drop(veh_lift(veh_id), -2, -2);
            }
        }
        stack_sort(veh_id);
    } else {
        // veh_top(veh_id), hand-inlined - matching stack_fix's precedent
        // (see its own comment): the image does not call it here.
        int top_veh_id = veh_id;
        for (int j = Vehs[top_veh_id].prev_veh_id_stack; j >= 0; j = Vehs[j].prev_veh_id_stack) {
            top_veh_id = j;
        }
        for (int i = top_veh_id; i >= 0; i = Vehs[i].next_veh_id_stack) {
            Vehs[i].state &= ~VSTATE_UNK_1; // so this call's boarding flags start clean
        }
    }
    int16_t subject_x = Vehs[veh_id].x;
    int16_t subject_y = Vehs[veh_id].y;
    BOOL is_open_ocean = on_map(subject_x, subject_y) && is_ocean(subject_x, subject_y)
        && !((bit_at(subject_x, subject_y) & BIT_BASE_IN_TILE)
             && owner_at(subject_x, subject_y) < MaxPlayerNum);
    uint32_t subject_faction = Vehs[veh_id].faction_id;
    BOOL subject_is_air = (get_triad(veh_id) == TRIAD_AIR);
    int cargo_left = (int)veh_cargo(veh_id);
    int carrier_left = has_abil(Vehs[veh_id].proto_id, ABL_CARRIER)
        ? ((int)veh_cargo(veh_id) < 1 ? 1 : (int)veh_cargo(veh_id)) : 0;
    if (!subject_faction) {
        cargo_left = 99;
    }
    int pass_count = (mode <= 1) ? 2 : 1;
    for (int pass = 0; pass < pass_count; pass++) {
        int i;
        if (mode) {
            i = veh_id;
        } else {
            // veh_top(veh_id), hand-inlined - matching stack_fix's precedent
            // (see its own comment): the image does not call it here.
            i = veh_id;
            for (int j = Vehs[i].prev_veh_id_stack; j >= 0; j = Vehs[j].prev_veh_id_stack) {
                i = j;
            }
        }
        while (i >= 0) {
            if (!cargo_left && !carrier_left) {
                break;
            }
            // Captured before the body: mode 1 lifts the unit, rewriting this link.
            // The original's redundant negative test on the link is dropped.
            // (This note sits on its own line rather than trailing the statement because
            // tools/mutate_and_verify.py scans a trailing comment as if it were code: the
            // `< 0` it used to carry produced two mutants that only ever rewrote a comment,
            // so they were unkillable by construction. The tool already skips whole-line
            // comments for exactly that reason.)
            int next_veh_id = Vehs[i].next_veh_id_stack;
            if (Vehs[i].faction_id != subject_faction) {
                i = next_veh_id;
                continue;
            }
            BOOL eligible = (cargo_left && get_triad(i) == TRIAD_LAND && !veh_cargo(i));
            if (!eligible && (!carrier_left || get_triad(i) != TRIAD_AIR
                              || has_abil(Vehs[i].proto_id, ABL_CARRIER))) {
                i = next_veh_id;
                continue;
            }
            BOOL accept;
            if (!pass) {
                // First pass re-boards only what is already recorded as riding this transport.
                accept = (Vehs[i].order == ORDER_SENTRY_BOARD
                          && Vehs[i].waypoint_x[0] == veh_id);
            } else {
                accept = true;
                if (subject_is_air) {
                    const Map *tile = &map_tiles()[(Vehs[veh_id].x >> 1)
                        + Vehs[veh_id].y * (int)MapLongitude];
                    uint32_t owner = tile->val2 & 0xF;
                    // Note the > 0 here: unlike the two other owner tests, faction 0 is rejected.
                    if (!((tile->bit & BIT_BASE_IN_TILE) && owner < MaxPlayerNum && owner > 0)
                        && !(tile->bit & BIT_AIRBASE)) {
                        accept = false;
                    }
                }
                uint32_t unit_faction = Vehs[i].faction_id;
                BOOL both_unk_200 = ((Vehs[veh_id].state & VSTATE_UNK_200)
                                     && (Vehs[i].state & VSTATE_UNK_200));
                if (accept && (((1 << unit_faction) & LockEnableMask)) && !both_unk_200) {
                    accept = (Vehs[i].order == ORDER_SENTRY_BOARD)
                        ? (Vehs[i].waypoint_x[0] < 0) : is_open_ocean;
                } else if (accept) {
                    if (Vehs[i].state & VSTATE_UNK_40000) {
                        const Map *tile = &map_tiles()[(Vehs[i].x >> 1)
                            + Vehs[i].y * (int)MapLongitude];
                        accept = ((Vehs[i].state & VSTATE_UNK_20000)
                                  && PlayersData[unit_faction].region_base_plan[tile->region]
                                         == PLAN_NAVAL_TRANSPORT);
                    } else {
                        uint8_t plan = VehPrototypes[Vehs[i].proto_id].plan;
                        if (Vehs[i].order == ORDER_HOLD || plan == PLAN_ALIEN_ARTIFACT
                            || plan == PLAN_TERRAFORMING) {
                            accept = is_open_ocean;
                        } else if (!is_open_ocean) {
                            int16_t unit_x = Vehs[i].x;
                            int16_t unit_y = Vehs[i].y;
                            accept = !(on_map(unit_x, unit_y)
                                       && PlayersData[unit_faction]
                                              .region_base_plan[region_at(unit_x, unit_y)]
                                          == PLAN_DEFENSIVE
                                       && plan < PLAN_COLONIZATION);
                        }
                    }
                }
            }
            if (accept && !(!mode && (Vehs[i].state & VSTATE_UNK_1))) {
                sleep_call(i);
                Vehs[i].waypoint_x[0] = (int16_t)veh_id;
                if (mode) {
                    veh_drop(veh_lift(i), -2, -2);
                    Vehs[i].state &= ~VSTATE_UNK_10000;
                } else {
                    Vehs[i].state |= VSTATE_UNK_1;
                }
                if (carrier_left > 0) {
                    carrier_left--;
                }
                if (cargo_left > 0) {
                    cargo_left--;
                }
            }
            i = next_veh_id;
        }
    }
    if (mode) {
        return veh_drop(veh_lift(veh_id), -2, -2);
    }
    return veh_cargo(veh_id) ? cargo_left : carrier_left;
}

/*
Purpose: Various unit stack related calculations based on type parameter (0-19) and conditions.
// ORIGINAL: 0x005B9580 ?stack_check@@YAHHHHHH@Z 0x005B9580-0x005B9BB8
// TRIED: call_diff still FEWER by one - case 12's `weap_strat(...)` call gets auto-inlined by this tree (weap_strat is small/BYTE_EXACT after this same pass) where the image keeps it a real `call`. `#pragma auto_inline(off)` around weap_strat's OWN definition would fix this call site but also stops weap_val (0x0057D360, not in this batch) from inlining it, which the image DOES do there - out of scope to touch a function outside this batch to fix this one. Still MISMATCH overall (~0.28 similar best) - a 20-case switch this size has per-case register allocation/instruction-scheduling noise that dwarfs the two structural fixes above.
// TRIED: MORE - this tree called `veh_top(veh_id)` and (case 8) `veh_cargo(i)`, neither of which the image calls (both fully inlined here, matching veh_at's precedent on veh_top and veh_cargo's own inline expansion for case 8).
// size      1592 bytes
// prototype int (__cdecl ?stack_check@@YAHHHHHH@Z)(int vehID, int type, int cond1, int cond2, int cond3)
// callers   39   call targets   5
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0057D270 0x0057D2E0 0x005BF1F0 0x005C0DB0 0x005C1760
Return Value: Dependent on type parameter
Status: Complete
*/
int __cdecl stack_check(int veh_id, int type, int cond1, int cond2, int cond3) {
    int retn_val = 0;
    uint32_t plan;
    uint32_t chas;
    int top_veh_id = -1;
    if (veh_id >= 0) {
        top_veh_id = veh_id;
        for (int j = Vehs[top_veh_id].prev_veh_id_stack; j >= 0; j = Vehs[j].prev_veh_id_stack) {
            top_veh_id = j;
        }
    }
    for (int i = top_veh_id; i >= 0; i = Vehs[i].next_veh_id_stack) {
        switch (type) {
          case 0:
            if ((cond2 < 0 || Vehs[i].faction_id == cond2) && Vehs[i].proto_id == cond1) {
                retn_val++;
            }
            break;
          case 1:
            if (cond1 < 0 || Vehs[i].faction_id == cond1) {
                retn_val++;
            }
            break;
          case 2:
            if ((cond2 < 0 || Vehs[i].faction_id == cond2) 
                && VehPrototypes[Vehs[i].proto_id].plan == cond1) {
                retn_val++;
            }
            break;
          case 3:
            if ((cond2 < 0 || Vehs[i].faction_id == cond2) && get_triad(i) == cond1) {
                retn_val++;
            }
            break;
          case 4:
            if (cond1 < 0 || Vehs[i].faction_id == cond1) {
                retn_val += get_offense_rating(i);
            }
            break;
          case 5:
            if (cond1 < 0 || Vehs[i].faction_id == cond1) {
                retn_val += get_defense_rating(i);
            }
            break;
          case 6:
            if ((cond2 < 0 || Vehs[i].faction_id == cond2) && has_abil(Vehs[i].proto_id, cond1)) {
                retn_val++;
            }
            break;
          case 7:
            if (cond1 < 0 || Vehs[i].faction_id == cond1) {
                retn_val += VehPrototypes[Vehs[i].proto_id].cost;
            }
            break;
          case 8:
            if (cond1 < 0 || Vehs[i].faction_id == cond1) {
                uint32_t triad = get_triad(i);
                if (triad == TRIAD_LAND) {
                    retn_val--;
                } else if (triad == TRIAD_SEA) {
                    // inlined veh_cargo(i): the image has no call here, see veh_cargo's own comment
                    int16_t proto_id_cargo = Vehs[i].proto_id;
                    uint32_t cargo = VehPrototypes[proto_id_cargo].carry_capacity;
                    retn_val += (cargo && proto_id_cargo < MaxVehProtoFactionNum
                        && (get_proto_offense_rating(proto_id_cargo) < 0
                            || proto_id_cargo == BSC_SPORE_LAUNCHER))
                        ? Vehs[i].morale + 1 : cargo;
                }
            }
            break;
          case 9:
            if ((cond2 < 0 || Vehs[i].faction_id == cond2) && Vehs[i].order == cond1) {
                retn_val++;
            }
            break;
          case 10:
            if (Vehs[i].faction_id == cond1) {
                retn_val++;
            }
            break;
          case 11:
            if ((cond3 < 0 || Vehs[i].faction_id == cond3)
                && (Vehs[i].state & cond1) == (uint32_t)cond2) {
                retn_val++;
            }
            break;
          case 12:
            if (cond1 < 0 || Vehs[i].faction_id == cond1) {
                uint32_t faction_id = Vehs[i].faction_id;
                int proto_id = Vehs[i].proto_id;
                if (arm_strat(VehPrototypes[proto_id].armor_id, faction_id) 
                    > weap_strat(VehPrototypes[proto_id].weapon_id, faction_id)) {
                    retn_val++;
                }
            }
            break;
          case 13:
            if ((cond1 < 0 || Vehs[i].faction_id == cond1) && is_missile(i)) {
                retn_val++;
            }
            break;
          case 14:
            if ((cond1 < 0 || Vehs[i].faction_id == cond1)
                && (plan = VehPrototypes[Vehs[i].proto_id].plan,
                    plan == PLAN_DEFENSIVE || plan == PLAN_COMBAT)) {
                retn_val++;
            }
            break;
          case 15:
            if ((cond1 < 0 || Vehs[i].faction_id == cond1) && can_arty(Vehs[i].proto_id, TRUE)) {
                retn_val++;
            }
            break;
          case 16:
            if ((cond1 < 0 || Vehs[i].faction_id == cond1)
                && (plan = VehPrototypes[Vehs[i].proto_id].plan, plan == PLAN_DEFENSIVE
                    || plan == PLAN_RECONNAISANCE || plan == PLAN_COMBAT)) {
                retn_val++;
            }
            break;
          case 17:
            if ((cond2 < 0 || Vehs[i].faction_id == cond2)
                && VehPrototypes[Vehs[i].proto_id].unk_1 == cond1) {
                retn_val++;
            }
            break;
          case 18:
            if ((cond1 < 0 || Vehs[i].faction_id == cond1)
                && (chas = VehPrototypes[Vehs[i].proto_id].chassis_id,
                    Chassis[chas].triad == TRIAD_AIR && Chassis[chas].range > 1)) {
                retn_val++;
            }
            break;
          case 19:
            if ((cond1 < 0 || Vehs[i].faction_id == cond1) && !get_offense_rating(i)) {
                retn_val++;
            }
            break;
          default:
            break;
        }
    }
    return retn_val;
}

/*
Purpose: Check to see whether provided faction and base can build a specific prototype. Checks are
         included to prevent SMACX specific units from being built in SMAC mode.
// ORIGINAL: 0x005BA910 ?veh_avail@@YAHHHH@Z 0x005BA910-0x005BAB39 BYTE_EXACT
// LEVER: WRONG CALLEE - is_port(base_id, false) hand-inlined to
//   is_coast(Bases[base_id].x, Bases[base_id].y, false); the image never calls is_port.
//   Split every `||`-combined early return (PROTO_ACTIVE/obsolete_factions, and the
//   whole 18-way !ExpansionEnabled SMAC-mode block) into sequential `if (...) return
//   false;` statements - the image gives each condition its own duplicated epilogue
//   rather than sharing one merged branch. The ability_flags checks are bitwise
//   (`abil_flag & ABL_X`), not equality - the image emits `test`, not `cmp`. Together
//   0.538 -> 0.996 similar; TRIED: the last ~6-instruction gap is register
//   allocation inside has_tech()/is_coast()'s own inlined bodies (shared MEASURED
//   inline helpers) - did not chase further to avoid touching their other callers.
// size      553 bytes
// prototype int (__cdecl ?veh_avail@@YAHHHH@Z)(int protoID, int factionID, int baseID)
// callers   7   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004E49D0
Return Value: Is unit available to faction/base? true/false
Status: Complete
*/
BOOL __cdecl veh_avail(int proto_id, int faction_id, int base_id) {
    // The image gives PROTO_ACTIVE and obsolete_factions separate early returns, each
    // with its own epilogue, rather than one merged `||` - reproduced as sequential ifs.
    if (!(VehPrototypes[proto_id].flags & PROTO_ACTIVE)) {
        return false;
    }
    if (VehPrototypes[proto_id].obsolete_factions & (1 << faction_id)) {
        return false;
    }
    if (proto_id < MaxVehProtoFactionNum) {
        if (!has_tech(VehPrototypes[proto_id].preq_tech, faction_id)) {
            return false;
        }
    }
    if (VehPrototypes[proto_id].plan == PLAN_COLONIZATION
        && GameRules & RULES_SCN_NO_COLONY_PODS) {
        return false;
    }
    // is_port(), hand-inlined - the image calls is_coast() directly here, not is_port().
    if (base_id >= 0 && get_proto_triad(proto_id) == TRIAD_SEA
        && !is_coast(Bases[base_id].x, Bases[base_id].y, false)) {
        return false;
    }
    // The image splits this whole SMAC-mode block into eighteen separate early
    // returns, each with its own epilogue, rather than one merged `||` chain -
    // reproduced as sequential ifs. The ability_flags checks are bitwise (`test`
    // against a single flag bit), not equality - `abil_flag == ABL_X` would only
    // fire for a unit with exactly that one ability and nothing else.
    if (!ExpansionEnabled) {
        if (VehPrototypes[proto_id].armor_id > ARM_PSI_DEFENSE) {
            return false;
        }
        uint8_t weap_id = VehPrototypes[proto_id].weapon_id;
        if (weap_id == WPN_RESONANCE_LASER) {
            return false;
        }
        if (weap_id == WPN_RESONANCE_BOLT) {
            return false;
        }
        if (weap_id == WPN_STRING_DISRUPTOR) {
            return false;
        }
        if (weap_id == WPN_TECTONIC_PAYLOAD) {
            return false;
        }
        if (weap_id == WPN_FUNGAL_PAYLOAD) {
            return false;
        }
        uint32_t abil_flag = VehPrototypes[proto_id].ability_flags;
        if (abil_flag & ABL_SOPORIFIC_GAS) {
            return false;
        }
        if (abil_flag & ABL_DISSOCIATIVE_WAVE) {
            return false;
        }
        if (abil_flag & ABL_MARINE_DETACHMENT) {
            return false;
        }
        if (abil_flag & ABL_FUEL_NANOCELLS) {
            return false;
        }
        if (abil_flag & ABL_ALGO_ENHANCEMENT) {
            return false;
        }
        if (proto_id == BSC_SEALURK) {
            return false;
        }
        if (proto_id == BSC_SPORE_LAUNCHER) {
            return false;
        }
        if (proto_id == BSC_BATTLE_OGRE_MK1) {
            return false;
        }
        if (proto_id == BSC_BATTLE_OGRE_MK2) {
            return false;
        }
        if (proto_id == BSC_BATTLE_OGRE_MK3) {
            return false;
        }
        if (proto_id == BSC_FUNGAL_TOWER) {
            return false;
        }
        if (proto_id == BSC_UNITY_MINING_LASER) {
            return false;
        }
    }
    if (proto_id < MaxVehProtoFactionNum) {
        return true;
    }
    return (proto_id / MaxVehProtoFactionNum) == faction_id;
}

/*
Purpose: Determine whether a faction wants the specified prototype based on the faction's current
         prototype designs.
// ORIGINAL: 0x005BE100 ?wants_prototype@@YAHHH@Z 0x005BE100-0x005BE29D
// TRIED: nested ternary chains (`cond ? a : cond2 ? b : c`) used as a single `if (...)` condition compiled to SETcc-then-test where the image branches directly - splitting each into if/else-if with explicit `continue` (the loop's early-out target is one shared label in the image) recovered the direct jumps. Also split the combined `if (!A || !B) return false;` guard at the top into two separate ifs - the image has two full, separate epilogues, one per check, not a combined OR. And walking a `VehPrototype *proto_cmp` pointer (++proto_cmp each iteration) instead of indexing `VehPrototypes[proto_id_cmp]` per field matches the image's single incrementing esi and its negative-offset field reads (esi-1 for chassis_id, esi+5 for plan) - though the image anchors that pointer at the weapon_id field specifically (esi+0), where this tree's pointer anchors at the struct's own front, a difference not chased further. Moved 0.617 -> 0.865 similar (/O2 /Gy /GR- /Oy- /GX).
// size      413 bytes
// prototype int (__cdecl ?wants_prototype@@YAHHH@Z)(int protoID, int factionID)
// callers   1   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0057D480
Return Value: Does faction want prototype? true/false
Status: Complete
*/
BOOL __cdecl wants_prototype(int proto_id, int faction_id) {
    uint32_t flags = VehPrototypes[proto_id].flags;
    if (!(flags & PROTO_ACTIVE)) {
        return false;
    }
    if (!(flags & PROTO_TYPED_COMPLETE)) {
        return false;
    }
    uint32_t proto_offset = faction_id * MaxVehProtoFactionNum;
    uint32_t proto_id_cmp = proto_offset;
    VehPrototype *proto_cmp = &VehPrototypes[proto_offset];
    for (uint32_t i = 0; i < MaxVehProtoFactionNum; i++, proto_id_cmp++, proto_cmp++) {
        uint32_t flags_cmp = proto_cmp->flags;
        if (!(flags_cmp & PROTO_ACTIVE)) {
            continue;
        }
        if (!(flags_cmp & PROTO_TYPED_COMPLETE)) {
            continue;
        }
        if (VehPrototypes[proto_id].plan != proto_cmp->plan) {
            continue;
        }
        uint8_t chas_cmp = proto_cmp->chassis_id;
        uint8_t chas = VehPrototypes[proto_id].chassis_id;
        if (Chassis[chas].triad != Chassis[chas_cmp].triad) {
            continue;
        }
        uint8_t weap_id_cmp = proto_cmp->weapon_id;
        uint8_t mode_cmp = Weapon[weap_id_cmp].mode;
        if (mode_cmp > WPN_MODE_MISSILE) {
            if (mode_cmp != Weapon[VehPrototypes[proto_id].weapon_id].mode) {
                continue;
            }
        } else {
            int8_t off_rating_cmp = Weapon[weap_id_cmp].offense_rating;
            if (off_rating_cmp < 0) {
                if (off_rating_cmp != get_proto_offense_rating(proto_id)) {
                    continue;
                }
            } else if (off_rating_cmp < get_proto_offense_rating(proto_id)) {
                continue;
            }
        }
        uint8_t arm_id_cmp = proto_cmp->armor_id;
        if (Armor[arm_id_cmp].defense_rating <= 0) {
            if (arm_id_cmp != VehPrototypes[proto_id].armor_id) {
                continue;
            }
        } else if (armor_val(proto_id_cmp, faction_id) < armor_val(proto_id, faction_id)) {
            continue;
        }
        if (Chassis[chas_cmp].speed >= Chassis[chas].speed) {
            return false;
        }
    }
    return true;
}

/*
Purpose: Check the coordinates for units and if at least one is found return the top most id.
// ORIGINAL: 0x005BFE90 ?veh_at@@YAHHH@Z 0x005BFE90-0x005BFF9E
// TRIED: 0.937 similar under every flag set tried - the control flow and
//        field reads already match; what's left is register allocation (the
//        image hoists `MapLatitudeBounds`/`MapLongitudeBounds` into ebx/edi
//        ONCE and reuses them for both `on_map()` call sites, this tree
//        re-reads them at the second site) plus esi/edi swaps throughout.
//        Left as-is per "do not chase register-allocation differences."
// LEVER 2026-09-03: hoisting the two bounds into locals before the FIRST
//        on_map() test is what earns the image's single load - a dllexport
//        inline re-reads its globals at every expansion, so the two sites
//        were two pairs of loads. Both on_map() calls are now written out
//        (the image inlines it at both) against those locals: 0.937 -> 0.938
//        at /Oy-, 0.942 at /GX, agreeing 5/96 -> 44/96.
// size      270 bytes
// prototype int (__cdecl ?veh_at@@YAHHH@Z)(int xCoord, int yCoord)
// callers   61   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00532A90 0x006262F0
Return Value: Unit id or -1 if nothing is found/error
Status: Complete
*/
// LEVER: veh_top() is called at only 2 real sites in the image (see its own comment);
//   here it is inlined - call_diff flagged the `call veh_top` this tree was making that
//   the image does not. Inlined the body by hand instead of `return veh_top(veh_id);`.
int __cdecl veh_at(int x, int y) {
    const int lat_bounds = MapLatitudeBounds;
    const int lon_bounds = MapLongitudeBounds;
    if (y >= 0 && y < lat_bounds && x >= 0 && x < lon_bounds && !(bit_at(x, y) & BIT_VEH_IN_TILE)) {
        return -1; // not found
    }
    for (int veh_id = 0; veh_id < VehCurrentCount; veh_id++) {
        if (Vehs[veh_id].x == x && Vehs[veh_id].y == y) {
            if (veh_id < 0) {
                return -1;
            }
            int top_veh_id = veh_id;
            for (int i = Vehs[top_veh_id].prev_veh_id_stack; i >= 0; i = Vehs[i].prev_veh_id_stack) {
                top_veh_id = i;
            }
            return top_veh_id;
        }
    }
    if (y < 0 || y >= lat_bounds || x < 0 || x >= lon_bounds) {
        return -1;
    }
    if (!VehBitError) {
        log_say("Vehicle Bit Error  (x, y)", x, y, 0);
    }
    if (GameState & STATE_SCENARIO_EDITOR || GameState & STATE_DEBUG_MODE || IsMultiplayerNet) {
        if (VehBitError) {
            return -1;
        }
        VehBitError = true;
    }
    rebuild_vehicle_bits();
    return -1;}

/*
Purpose: Check whether the prototype has a specific ability.
// ORIGINAL: 0x005BF1F0 ?has_abil@@YAHHH@Z 0x005BF1F0-0x005BF310
// TRIED: a negative-proto_id guard clause (with its log_say call) that call_diff flagged as an extra call the image does not make; now 13/109 instructions agree, 0.825 similar (best flags), up from a structurally-unrelated function body before
// size      288 bytes
// prototype int (__cdecl ?has_abil@@YAHHH@Z)(int protoID, int abilityID)
// callers   54   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005B9F20
Return Value: Does prototype have ability? true/false
Status: Complete
*/
BOOL __cdecl has_abil(int proto_id, int ability_id) {
    // BUG IN THE ORIGINAL: no guard against a negative proto_id here - it flows straight
    // into `proto_id / MaxVehProtoFactionNum` and an out-of-bounds VehPrototypes[] read.
    // A prior pass added a guard clause (with a log_say call) that is NOT in the image;
    // call_diff flagged the extra call, and the disassembly at 0x005BF1F0 confirms the
    // first instructions compute proto_id*sizeof(VehPrototype) directly, no branch first.
    if (VehPrototypes[proto_id].ability_flags & ability_id) {
        return true;
    }
    uint32_t faction_id = proto_id / MaxVehProtoFactionNum;
    if (!faction_id) {
        return false; // skip basic prototypes from #UNITS
    }
    if (is_alien_faction(faction_id) && ability_id == ABL_DEEP_RADAR) {
        return true; // Caretakers + Usurpers > "Deep Radar" ability for all units
    }
    for (int i = 0; i < Players[faction_id].faction_bonus_count; i++) {
        if (Players[faction_id].faction_bonus_id[i] == RULE_FREEABIL) {
            int abil_bonus_id = Players[faction_id].faction_bonus_val1[i];
            if (has_tech(Ability[abil_bonus_id].preq_tech, faction_id) &&
                (ability_id & (1 << abil_bonus_id))) {
                // Pirates > "Marine Detachment" ability for combat sea units with Adaptive Doctrine
                return true;
            }
        }
    }
    if (VehPrototypes[proto_id].weapon_id == WPN_PROBE_TEAM && ability_id == ABL_ALGO_ENHANCEMENT
        && has_project(SP_NETHACK_TERMINUS, faction_id)) {
        return true; // All Probe Teams act as though they have the "Algorithmic Enhancement"
    }
    return false;
}

/*
Purpose: Temporarily remove the specified unit from its current square and stack in preparation for 
         another action such as interacting with the stack, moving or killing it.
// ORIGINAL: 0x005BFFA0 ?veh_lift@@YAXH@Z 0x005BFFA0-0x005C007F
// TRIED: swapping the x/y local declaration order - the remaining gap is which register (eax/ecx/edx) the allocator picks for x, y and the -1 constant at the join point; both orders produced the identical register assignment, so it isn't source-order controlled here. Plateau at 49/60.
// TRIED 2026-09-03: NO x/y locals at all - `VehLiftX = Vehs[veh_id].x;` after the four -1 stores. The loads then sit AFTER the stores (VC6 will not hoist a load above a store to the same field), where the image hoists them BEFORE the stores, which needs the locals. 54/60 - better than 49/60, kept; the remaining 6 are the two movsx loads plus which register holds the -1 (ours edx, image eax).
// TRIED 2026-09-03: `VehDropLiftVehID = veh_id;` hoisted ABOVE the x/y reads (so VC6 sees the global store early) - regressed to 49/60, matching the original note's finding that the image assigns it between reading y and zeroing the fields.
// TRIED: were cached into locals BEFORE the prev/next stack-pointer fixup and the on_map/bit_set branch; the image re-reads `Vehs[veh_id].x`/`.y` fresh at each site (inside the branch, and again at the join point for VehLiftX/Y) instead of hoisting one read to the top. Also `VehDropLiftVehID = veh_id;` moved to AFTER the x/y reads (image assigns it between reading y and zeroing the fields, not before). 4/60 -> 49/60 (0.829 -> 0.983 similar).
// symbol    ?veh_lift@@YAHH@Z
// size      223 bytes
// prototype void (__cdecl ?veh_lift@@YAXH@Z)(int vehID)
// callers   18   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00591D60
Return Value: Unit id
Status: Complete
*/
int __cdecl veh_lift(int veh_id) {
    BOOL prev_stack_exists = false;
    int16_t prev_veh_id = Vehs[veh_id].prev_veh_id_stack;
    if (prev_veh_id >= 0) {
        prev_stack_exists = true;
        Vehs[prev_veh_id].next_veh_id_stack = Vehs[veh_id].next_veh_id_stack;
    }
    int16_t next_veh_id = Vehs[veh_id].next_veh_id_stack;
    if (next_veh_id >= 0) {
        Vehs[next_veh_id].prev_veh_id_stack = Vehs[veh_id].prev_veh_id_stack;
    } else if (!prev_stack_exists && on_map(Vehs[veh_id].x, Vehs[veh_id].y)) {
        bit_set(Vehs[veh_id].x, Vehs[veh_id].y, BIT_VEH_IN_TILE, false);
    }
    VehDropLiftVehID = veh_id;
    Vehs[veh_id].x = -1;
    Vehs[veh_id].y = -1;
    Vehs[veh_id].next_veh_id_stack = -1;
    Vehs[veh_id].prev_veh_id_stack = -1;
    VehLiftX = Vehs[veh_id].x;
    VehLiftY = Vehs[veh_id].y;
    return veh_id;
}

/*
Purpose: Move the specified unit to the provided coordinates.
// ORIGINAL: 0x005C0080 ?veh_drop@@YAXHHH@Z 0x005C0080-0x005C019C
// TRIED: the last `mov ecx,[ebp+8]; mov eax,ecx` before the final `ret` - the image loads straight into eax there (ecx is dead, clobbered by the `bit_set` call just before), but this tree's register allocator prefers ecx as veh_id's home register for the whole function and copies to eax at the return. No source reshuffle around the final `return veh_id;` moved it; plateau.
// TRIED: (veh_id_dest < 0) {...} else {...}` had the arms backwards - the image falls through the `>= 0` write first and jumps to the `< 0` block, so the source needs `if (veh_id_dest >= 0) {...} else {...}` to get that polarity. Also the flags ternary `(A && B) ? X|Y : Y` compiled an extra `mov` before the compare the image folds into a direct `cmp byte ptr [...], 2`; splitting it into `flags = 0/BIT_SUPPLY_REMOVE; flags |= BIT_VEH_IN_TILE;` (the image ORs BIT_VEH_IN_TILE in unconditionally at a shared tail) dropped that mov. 39/95 -> 80/95 (0.880 -> 0.973 similar).
// symbol    ?veh_drop@@YAHHHH@Z
// size      284 bytes
// prototype void (__cdecl ?veh_drop@@YAXHHH@Z)(int vehID, int xCoord, int yCoord)
// callers   17   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00591B10 0x00591D60 0x005BFE90
Return Value: Unit id (1st param), doesn't seem to be used
Status: Complete
*/
int __cdecl veh_drop(int veh_id, int x, int y) {
    int veh_id_dest = veh_at(x, y);
    Vehs[veh_id].next_veh_id_stack = (int16_t)veh_id_dest;
    Vehs[veh_id].prev_veh_id_stack = -1;
    Vehs[veh_id].x = (int16_t)x;
    Vehs[veh_id].y = (int16_t)y;
    VehDropLiftVehID = -1;
    if (veh_id_dest >= 0) {
        Vehs[veh_id_dest].prev_veh_id_stack = (int16_t)veh_id;
    } else {
        if (y < 0) {
            return veh_id;
        }
        if (on_map(x, y) && !(bit_at(x, y) & BIT_BASE_IN_TILE)) {
            owner_set(x, y, Vehs[veh_id].faction_id);
        }
    }
    if (on_map(x, y)) {
        uint32_t flags;
        if (!Vehs[veh_id].faction_id || get_triad(veh_id) == TRIAD_AIR) {
            flags = 0;
        } else {
            flags = BIT_SUPPLY_REMOVE;
        }
        flags |= BIT_VEH_IN_TILE;
        bit_set(x, y, flags, true);
    }
    return veh_id;
}

/*
Purpose: Set the unit's status to sentry/board.
// ORIGINAL: 0x005C01A0 ?sleep@@YAXH@Z 0x005C01A0-0x005C01CA BYTE_EXACT
// size      42 bytes
// prototype void (__cdecl ?sleep@@YAXH@Z)(int vehID)
// callers   8   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
// BODY IN veh.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.

// See veh.h: a real, out-of-line forwarder so stack_veh's one call site gets
// an actual `call` where `sleep(i)` directly would inline away under /O2.
void __cdecl sleep_call(int veh_id) {
    sleep(veh_id);
}

// See veh.h: a real, out-of-line forwarder so veh_cost's one call site gets
// an actual `call` where `base_cost(proto_id)` directly would inline away
// under /O2.
int __cdecl base_cost_call(int proto_id) {
    return base_cost(proto_id);
}

/*
Purpose: Move the specified unit to the bottom of the stack.
// ORIGINAL: 0x005C01D0 ?veh_demote@@YAXH@Z 0x005C01D0-0x005C0256
// TRIED: byte-exactness. Best is 0.916 similar (4/40 in position) at
// `/c /O2 /Gy /GR- /Oy- /GX`, and every remaining edit is register
// allocation - `esi`/`edi`/`ebx` permuted against the image's fixed
// `ebx`=veh_id, `esi`=last_veh_id, `edi`=&Vehs[veh_id] - with no control
// flow or field-order difference. Measured two spellings: nested `if` and
// guard-clause early-return (identical 0.916, guard clause kept as closer
// to the image's flat `jl`/`jl`/`je` shape), and a cached `Veh *` for
// `&Vehs[veh_id]`/`&Vehs[last_veh_id]` (WORSE, 0.892 - an extra `lea`
// survives past the call where the image recomputes the address after
// `veh_lift` returns). Do not re-derive the pointer-cache attempt.
// size      134 bytes
// prototype void (__cdecl ?veh_demote@@YAXH@Z)(int vehID)
// callers   4   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005BFFA0
Return Value: n/a
Status: Complete
*/
void __cdecl veh_demote(int veh_id) {
    if (veh_id < 0) {
        return;
    }
    int16_t next_veh_id = Vehs[veh_id].next_veh_id_stack;
    if (next_veh_id < 0) {
        return;
    }
    int16_t last_veh_id;
    do {
        last_veh_id = next_veh_id;
        next_veh_id = Vehs[last_veh_id].next_veh_id_stack;
    } while (next_veh_id >= 0);
    if (last_veh_id == veh_id) {
        return;
    }
    veh_lift(veh_id);
    Vehs[last_veh_id].next_veh_id_stack = (int16_t)veh_id;
    Vehs[veh_id].prev_veh_id_stack = last_veh_id;
    Vehs[veh_id].next_veh_id_stack = -1;
    Vehs[veh_id].x = Vehs[last_veh_id].x;
    Vehs[veh_id].y = Vehs[last_veh_id].y;
}

/*
Purpose: Move the specified unit to the top of the stack.
// ORIGINAL: 0x005C0260 ?veh_promote@@YAXH@Z 0x005C0260-0x005C02CE
// TRIED: a `veh_id_top >= 0` guard after the hand-inlined veh_top - the image has none (see BUG note below). TRIED: seeding `veh_id_top = veh_id` before vs. inside the `if` - same codegen either way. The remaining gap is the compiler proving the `prev_veh_id_stack < 0` (zero-iteration) case makes `veh_id_top != veh_id` tautologically false and skipping straight to the epilogue, where the image still materializes and compares it; not reachable by reordering statements. Plateau at 3/40.
// TRIED: CALLEE - the tree called `veh_top()`/`veh_put()`, both inline-marked wrappers, but this call site is not one of veh_top's two real sites and veh_put did not inline here either. Hand- inlining both (veh_top's loop, and veh_put's `veh_lift(id); veh_drop(id, x, y)` - dropping veh_lift's unused return since it always returns its own argument) took call_diff from WRONG CALLEE to 0 disagreeing and 0.657 -> 0.773 similar.
// size      110 bytes
// prototype 
// callers   6   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005BFFA0 0x005C0080
Return Value: n/a
Status: Complete
*/
void __cdecl veh_promote(int veh_id) {
    // veh_top(), hand-inlined - matching stack_fix's precedent (see its own
    // comment): the image does not call it here.
    int veh_id_top = veh_id;
    if (veh_id >= 0) {
        for (int i = Vehs[veh_id_top].prev_veh_id_stack; i >= 0; i = Vehs[i].prev_veh_id_stack) {
            veh_id_top = i;
        }
    } else {
        veh_id_top = -1;
    }
    // BUG IN THE ORIGINAL: no `veh_id_top >= 0` guard here - if veh_id is
    // itself negative, veh_id_top becomes -1 and this still proceeds to
    // index Vehs[-1] below, as long as veh_id != -1 exactly.
    if (veh_id_top != veh_id) {
        // veh_put(), hand-inlined: the image calls veh_lift and veh_drop
        // directly here, discarding veh_lift's return (it always returns
        // its own veh_id argument unchanged) rather than threading it
        // through to veh_drop's first parameter.
        veh_lift(veh_id);
        veh_drop(veh_id, Vehs[veh_id_top].x, Vehs[veh_id_top].y);
    }
}

/*
Purpose: Clear the specified unit.
// ORIGINAL: 0x005C02D0 ?veh_clear@@YAXHHH@Z 0x005C02D0-0x005C03C3 BYTE_EXACT
// size      243 bytes
// prototype void (__cdecl ?veh_clear@@YAXHHH@Z)(int vehID, int protoID, int factionID)
// callers   2   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl veh_clear(int veh_id, int proto_id, int faction_id) {
    Vehs[veh_id].x = -4;
    Vehs[veh_id].y = -4;
    Vehs[veh_id].year_end_lurking = 0;
    Vehs[veh_id].unk_1 = 0;
    Vehs[veh_id].flags = 0;
    Vehs[veh_id].faction_id = (uint8_t)faction_id;
    Vehs[veh_id].proto_id = (int16_t)proto_id;
    Vehs[veh_id].next_veh_id_stack = -1;
    Vehs[veh_id].prev_veh_id_stack = -1;
    Vehs[veh_id].waypoint_count = 0;
    Vehs[veh_id].patrol_current_point = 0;
    Vehs[veh_id].order = ORDER_NONE;
    for (uint32_t i = 0; i < 4; i++) {
        Vehs[veh_id].waypoint_x[i] = -1;
        Vehs[veh_id].waypoint_y[i] = -1;
    }
    Vehs[veh_id].state = 0;
    Vehs[veh_id].moves_expended = 0;
    Vehs[veh_id].dmg_incurred = 0;
    Vehs[veh_id].order_auto_type = 0;
    uint8_t rule_morale = (uint8_t)Players[faction_id].rule_morale;
    Vehs[veh_id].terraforming_turns = 0;
    Vehs[veh_id].unk_6 = 0;
    Vehs[veh_id].move_to_ai_type = 0;
    Vehs[veh_id].visibility = 0;
    Vehs[veh_id].home_base_id = -1;
    Vehs[veh_id].morale = rule_morale + 1;
    Vehs[veh_id].facing = 2;
    Vehs[veh_id].probe_action = 0;
    Vehs[veh_id].probe_sabotage_id = 0;
}

/*
Purpose: Check if the prototype can perform artillery combat. The 2nd parameter determines how sea 
         units are treated.
// ORIGINAL: 0x005C0DB0 ?can_arty@@YAHHH@Z 0x005C0DB0-0x005C0E35
// TRIED 2026-09-03: the one remaining divergence is NOT the `mov bl/test bl`
// wall the first TRIED line below describes - that is gone (both sides now
// emit `mov bl, [edx+..]; test bl,bl`), and the Armor addressing of the
// second line went with it. What is left is ONE JUMP TARGET: the image's
// `dec eax / je 0x5c0dda` for case TRIAD_AIR against this tree's
// `je 0x5c0df9` - the image shares the FIRST `return false` epilogue, this
// tree the SECOND. Spelling the switch's `return false` as `return 0`,
// putting TRIAD_AIR before TRIAD_SEA in the case list, and adding an
// explicit `default: break;` each measured 53/54 identically. Which of two
// identical epilogues VC6's return-sharing picks is not reachable from the
// case order or from the constant's type; every other byte agrees.
// TRIED: the OR'd (offense<=0 || defense<0) guard into two sequential `if`s, one per term - matches the image's per-term SPORE_LAUNCHER exception check. Also: the TRIAD_SEA/TRIAD_AIR dispatch is a `switch`, not chained `if`s - the image's dec/je chain is switch codegen. 19/54 -> 50/54. One divergence remains: the defense_rating byte load folds into `cmp byte ptr [edx], 0` here where the image keeps `mov bl, ...; test bl, bl` - tried swapping check order, `0 > x`, a BOOL local, and `!(x >= 0)`; none moved it. Plateau.
// TRIED: same `Armor[armor_id].defense_rating` addressing defect as arm_strat (0x0057D270): the image computes `shl edx,4` then loads with a displaced `[edx+0x94F280]`, this tree's `get_proto_defense_rating` inline always compiles `add edx,0x94F28` (pre-adding the displaced/16 constant) then `shl edx,4` then a base-only load. Raw byte-array indexing forms tried at arm_strat produced the identical bytes there too, so not re-tried per-callsite here. Cross-function confirmation that this is a compiler addressing-mode choice, not something source-shape-local to either caller.
// size      133 bytes
// prototype int (__cdecl ?can_arty@@YAHHH@Z)(int protoID, int triad_sea_retn)
// callers   14   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005BF1F0
Return Value: Has artillery ability? true/false
Status: Complete
*/
BOOL __cdecl can_arty(int proto_id, BOOL sea_triad_retn) {
    if (get_proto_offense_rating(proto_id) <= 0 && proto_id != BSC_SPORE_LAUNCHER) {
        return false;
    }
    if (get_proto_defense_rating(proto_id) < 0 && proto_id != BSC_SPORE_LAUNCHER) {
        return false;
    }
    uint8_t triad = get_proto_triad(proto_id);
    switch (triad) {
      case TRIAD_SEA:
        return sea_triad_retn; // cursory check shows this value always being set to true
      case TRIAD_AIR:
        return false;
    }
    return has_abil(proto_id, ABL_ARTILLERY); // TRIAD_LAND
}

/*
Purpose: Calculate a unit's morale. TODO: Determine if 2nd param is a toggle for display vs actual 
         morale.
// ORIGINAL: 0x005C0E40 ?morale_veh@@YAHHHH@Z 0x005C0E40-0x005C1143
// TRIED: call_diff FEWER (2 vs image's 4: morale_alien, has_tech, and
//        `bitmask` TWICE) - the two `has_fac_built(FAC_CHILDREN_CRECHE/
//        FAC_BROOD_PIT, home_base_id)` calls each keep `bitmask()` as a real
//        `call 0x50ba00` in the image, but this tree fully inlines it under
//        every flag set tried, same as the already-documented case on
//        get_basic_offense (0x5015B0). Bypassing `has_fac_built` and calling
//        `bitmask()` directly (with `int offset, mask;` locals) did not stop
//        it inlining either. Otherwise the control-flow order, field offsets
//        and short-circuit structure already match; the rest of the gap
//        looks like register allocation (a cached zero register the image
//        keeps in ebx that this tree re-materializes per comparison).
// TRIED (2nd attempt): base.cpp's `bitmask_call` forwarder (general.h),
//        via a local `has_fac_built_call` matching base.cpp's own helper, at
//        BOTH call sites. Call count agreed (0 disagree, was FEWER 2v4), but
//        the best flag set's similarity fell from 0.484 (4/286 exact, /O1
//        /GR- /Oy- /GX) to 0.340 (2/286 exact, /O2 /Ob0 /GR- /GX). Reverted.
// LEVER (3rd attempt): `bitmask_call` on the CRECHE site ONLY, brood-pit left
//        plain `has_fac_built`, matching the "outer real, nested/nearby
//        inline" split get_basic_offense needed. Best flag set's similarity
//        0.484 -> 0.518 (3/286 exact, /O1 /GR- /Oy- /GX); call count 2v4 ->
//        3v4 (still FEWER by 1 - the brood-pit site stays a gap, same wall
//        as the 2nd attempt's failure mode when forced real).
// size      771 bytes
// prototype int (__cdecl ?morale_veh@@YAHHHH@Z)(int vehID, int checkDroneRiot, int factionIDvsNative)
// callers   16   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00501350 0x0050BA00 0x005B9F20
Return Value: Morale value
Status: Complete
*/
int __cdecl morale_veh(int veh_id, BOOL check_drone_riot, int faction_id_vs_native) {
    uint32_t faction_id = Vehs[veh_id].faction_id;
    if (!faction_id) {
        return morale_alien(veh_id, faction_id_vs_native);
    }
    int16_t proto_id = Vehs[veh_id].proto_id;
    if (VehPrototypes[proto_id].plan == PLAN_INFO_WARFARE) { // probe
        int probe_morale = range(PlayersData[faction_id].soc_effect_active.probe, 0, 3);
        probe_morale += has_project(SP_TELEPATHIC_MATRIX, faction_id) ? 2 : 0;
        for (int i = 0; i < MaxTechnologyNum; i++) {
            if (Technology[i].flags & TFLAG_IMPROVED_PROBES && has_tech(i, faction_id)) {
                probe_morale++;
            }
        }
        probe_morale += Vehs[veh_id].morale;
        return range(probe_morale, 2, 6);
    }
    int8_t offense_rating = get_proto_offense_rating(proto_id);
    if (proto_id < MaxVehProtoFactionNum && offense_rating < 0) {
        return range(Vehs[veh_id].morale, 0, 6); // Basic Psi Veh
    }
    // everything else
    int morale_modifier = range(PlayersData[faction_id].soc_effect_active.morale, -4, 4);
    if (morale_modifier <= -2) {
        morale_modifier++;
    } else if (morale_modifier >= 2) {
        morale_modifier--;
    }
    int rule_morale = Players[faction_id].rule_morale; // different from 'SOCIAL, MORALE'
    if (rule_morale < 0) { // negative effects 1st
        morale_modifier += rule_morale;
    }
    int home_base_id = Vehs[veh_id].home_base_id;
    if (home_base_id >= 0) { // home base countering negative effects
        if (has_fac_built_call(FAC_CHILDREN_CRECHE, home_base_id) && morale_modifier < 0) {
            morale_modifier /= 2;
        }
        if (has_fac_built(FAC_BROOD_PIT, home_base_id) && proto_id < MaxVehProtoFactionNum
            && (offense_rating < 0 || proto_id == BSC_SPORE_LAUNCHER) && morale_modifier < 0) {
            morale_modifier /= 2; // never reached due to above 'Basic Psi Veh' checks
        }
    }
    if (rule_morale > 0) {
        morale_modifier += rule_morale;
    }
    BOOL morale_flag = Players[faction_id].rule_flags & RFLAG_MORALE;
    if (morale_flag && morale_modifier < 0) {
        morale_modifier = 0;
    }
    if (check_drone_riot && home_base_id >= 0 
        && Bases[home_base_id].state & BSTATE_DRONE_RIOTS_ACTIVE && !morale_flag) {
        // bug fix: removed premature range bounding negating negative morale effects
        morale_modifier--;
    }
    return range(Vehs[veh_id].morale + morale_modifier, 0, 6);
}

/*
Purpose: Calculate the offense of the specified prototype. Optional param of the unit defending 
         against (-1 to ignore) as well as whether artillery or missile combat is being utilized.
// ORIGINAL: 0x005C1150 ?offense_proto@@YAHHHH@Z 0x005C1150-0x005C128F
// TRIED: `abs()` for the negate - same similarity as the manual ternary. The remaining gap is the same defense_rating byte-load fold as can_arty's plateau (`cmp byte ptr [x], 0` here vs the image's `mov dl, ...; test dl, dl`) plus register-naming noise past that point; not moved by source reshuffling.
// TRIED: `uint32_t weapon_id = VehPrototypes[proto_id].weapon_id;` local, read at its two use sites, made the compiler spill it back into proto_id's own stack slot (an extra store the image never makes) and renamed registers through the entire rest of the function. Writing `VehPrototypes[proto_id].weapon_id` at both call sites instead of caching it dropped that spill: 4/112 -> 47/112 (0.882 -> 0.918 similar). Also reproduced the missing `veh_id_def < 0` guard on the SECOND `Vehs[veh_id_def].proto_id != SPORE_LAUNCHER` check (see the BUG note below) and split `off_rating` into an `int8_t` read plus a widened ternary negate rather than negating an already-widened `int` - the image tests/negates the BYTE before the `movsx`. 47/112 -> 50/112 (0.932 similar).
// size      319 bytes
// prototype int (__cdecl ?offense_proto@@YAHHHH@Z)(int protoID, int vehIDDef, BOOL isBombardment)
// callers   4   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: Prototype's offense
Status: Complete
*/
int __cdecl offense_proto(int proto_id, int veh_id_def, BOOL is_bombard) {
    if (Weapon[VehPrototypes[proto_id].weapon_id].mode == WPN_MODE_INFOWAR && veh_id_def >= 0
        && VehPrototypes[Vehs[veh_id_def].proto_id].plan == PLAN_INFO_WARFARE) {
        return 16; // probe attacking another probe
    }
    // BUG IN THE ORIGINAL: Vehs[veh_id_def].proto_id is read unguarded here
    // - if veh_id_def is -1 (no defender), this reads Vehs[-1], an
    // arbitrary out-of-bounds (Reactor struct) memory read. The image has
    // no `veh_id_def < 0` guard on this specific comparison (unlike the
    // guarded one a few lines up), so it is left alone deliberately.
    if ((is_bombard || (Weapon[VehPrototypes[proto_id].weapon_id].offense_rating >= 0
        && (veh_id_def < 0 || get_defense_rating(veh_id_def) >= 0)))
        && Vehs[veh_id_def].proto_id != BSC_SPORE_LAUNCHER
        && proto_id != BSC_SPORE_LAUNCHER) {
        int8_t off_rating_byte = get_proto_offense_rating(proto_id);
        int off_rating = (off_rating_byte < 0) ? -off_rating_byte : off_rating_byte;
        if (is_proto_missile(proto_id) && off_rating < 99) {
            off_rating = (off_rating * 3) / 2;
        }
        return (veh_id_def < 0) ? off_rating : off_rating * 8; // conventional
    }
    return (veh_id_def < 0) ? Rules.psi_combat_ratio_atk[TRIAD_LAND] : // PSI
        Rules.psi_combat_ratio_atk[get_triad(veh_id_def)] * 8;
}

/*
Purpose: Calculate the defense of the specified prototype. Optional param if unit is being attacked 
         (-1 to ignore) as well as whether artillery or missile combat is being utilized.
// ORIGINAL: 0x005C1290 ?armor_proto@@YAHHHH@Z 0x005C1290-0x005C13AD
// size      285 bytes
// prototype int (__cdecl ?armor_proto@@YAHHHH@Z)(int protoID, int vehIDAtk, int isBombardment)
// callers   3   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
// LEVER: the first if() had a fabricated `veh_id_atk < 0 ||` guard the image never checks
//   before reading Vehs[veh_id_atk].proto_id (unlike the guarded get_offense_rating() check
//   further down); removing it moved 2/97 -> 35/97 agreeing at /Oy-. Remainder looks like
//   register allocation across the shared early-return tail, not a structural difference.
Return Value: Prototype's defense
Status: Complete
*/
int __cdecl armor_proto(int proto_id, int veh_id_atk, BOOL is_bombard) {
    if (Weapon[VehPrototypes[proto_id].weapon_id].mode == WPN_MODE_INFOWAR && veh_id_atk >= 0
        && VehPrototypes[Vehs[veh_id_atk].proto_id].plan == PLAN_INFO_WARFARE) {
        return 16; // probe defending against another probe
    }
    // BUG IN THE ORIGINAL: this branch reads Vehs[veh_id_atk].proto_id with no veh_id_atk >= 0
    // guard, unlike the get_offense_rating() check a few lines below which does guard it. When
    // is_bombard is set and veh_id_atk is -1 (no attacker), the image reads Vehs[-1], an
    // out-of-bounds array access. Left as shipped rather than adding the missing guard.
    if (is_bombard && Vehs[veh_id_atk].proto_id != BSC_SPORE_LAUNCHER
        && proto_id != BSC_SPORE_LAUNCHER || (get_proto_defense_rating(proto_id) >= 0
            && (veh_id_atk < 0 || get_offense_rating(veh_id_atk) >= 0))) {
        uint32_t def_rating = range(get_proto_defense_rating(proto_id), 1, 9999);
        return (veh_id_atk < 0) ? def_rating : def_rating * 8; // conventional
    }
    return (veh_id_atk < 0) ? Rules.psi_combat_ratio_def[TRIAD_LAND] : // PSI
        Rules.psi_combat_ratio_def[get_proto_triad(proto_id)] * 8;
}

/*
Purpose: Calculate the speed of the specified prototype on roads.
// ORIGINAL: 0x005C13B0 ?speed_proto@@YAHH@Z 0x005C13B0-0x005C1532
// size      386 bytes
// prototype int (__cdecl ?speed_proto@@YAHH@Z)(int protoID)
// callers   12   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005BF1F0
// TRIED: nothing changed - traced the full disassembly instruction-by-instruction
//   against this body (struct offsets, ABL_* constant values, WPN_MODE_TRANSPORT==7,
//   the TRIAD_AIR-only tail) and the control flow order already matches the image
//   exactly. Best flags give 0.81-0.82 similar; remainder looks like register
//   allocation / frame-pointer choice, not a structural difference.
Return Value: Prototype's speed on roads
Status: Complete
*/
int __cdecl speed_proto(int proto_id) {
    if (proto_id == BSC_FUNGAL_TOWER) {
        return 0; // cannot move
    }
    int chas_id = VehPrototypes[proto_id].chassis_id;
    int speed_val = Chassis[chas_id].speed;
    uint8_t triad = Chassis[chas_id].triad;
    uint32_t weap_id = VehPrototypes[proto_id].weapon_id;
    if (triad == TRIAD_AIR) {
        speed_val += VehPrototypes[proto_id].reactor_id * 2;
    }
    if (Weapon[weap_id].mode == WPN_MODE_TRANSPORT) {
        speed_val--;
    }
    if (has_abil(proto_id, ABL_SLOW)) {
        speed_val--;
    }
    if (has_abil(proto_id, ABL_ANTIGRAV_STRUTS)) {
        speed_val += (triad == TRIAD_AIR) ? VehPrototypes[proto_id].reactor_id * 2 : 1;
    }
    if (triad == TRIAD_AIR) {
        if (has_abil(proto_id, ABL_FUEL_NANOCELLS)) {
            speed_val += 2;
        }
        if (has_project(SP_CLOUDBASE_ACADEMY, proto_id / MaxVehProtoFactionNum)) {
            speed_val += 2; // bonus from Aerospace Complex
        }
        if (has_abil(proto_id, ABL_AIR_SUPERIORITY)) {
            // generally -20% to -25%, in some cases higher due to lossy division rounding
            speed_val = (speed_val * 4) / 5;
        }
        if (Weapon[weap_id].mode == WPN_MODE_TRANSPORT) {
            speed_val /= 2; // 2nd penalty for air transports: -50%
        }
    }
    return range(speed_val, 1, 99) * Rules.move_rate_roads;
}

/*
Purpose: Calculate the speed of a unit on roads taking into consideration prototype speed, 
         elite morale, if the unit is damaged and other factors. The skip_morale parameter seems to 
         only be set to true for certain combat calculations in battle_fight().
// ORIGINAL: 0x005C1540 ?speed@@YAHHH@Z 0x005C1540-0x005C1752
// TRIED: is called UNCONDITIONALLY in the image - `!skip_morale` is tested only AFTER the call and its MORALE_ELITE comparison, not before it (short-circuit order swapped: `morale_veh(...) == MORALE_ELITE && !skip_morale && (...)` instead of `!skip_morale && morale_veh(...) == MORALE_ELITE && (...)`). Confirmed against the disassembly: `call 0x5c0e40` at 0x5C15CB has no guard before it, and the skip_morale parameter isn't read until 0x5C15D8, after the MORALE_ELITE compare at 0x5C15D3.
// size      530 bytes
// prototype int (__cdecl ?speed@@YAHHH@Z)(int vehID, int toggle)
// callers   27   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005C0E40 0x005C13B0
Return Value: Speed
Status: Complete
*/
int __cdecl speed(int veh_id, BOOL skip_morale) {
    uint32_t proto_id = Vehs[veh_id].proto_id;
    if (proto_id == BSC_FUNGAL_TOWER) { // moved this check to top vs bottom, same logic
        return 0; // cannot move
    }
    uint32_t speed_val = speed_proto(proto_id);
    uint8_t triad = get_proto_triad(proto_id);
    if (triad == TRIAD_SEA && has_project(SP_MARITIME_CONTROL_CENTER, Vehs[veh_id].faction_id)) {
        speed_val += Rules.move_rate_roads * 2;
    }
    if (morale_veh(veh_id, true, 0) == MORALE_ELITE && !skip_morale
        && (proto_id >= MaxVehProtoFactionNum || get_proto_offense_rating(proto_id) >= 0)) {
        speed_val += Rules.move_rate_roads;
    }
    if (Vehs[veh_id].dmg_incurred && triad != TRIAD_AIR) {
        // optimized code without changes to logic
        uint32_t moves = speed_val / Rules.move_rate_roads;
        uint32_t reactor_fac;
        if (VehPrototypes[proto_id].plan == PLAN_ALIEN_ARTIFACT) {
            speed_val = reactor_fac = 1;
        } else {
            reactor_fac = range(VehPrototypes[proto_id].reactor_id, 1, 100) * 10;
            speed_val = range(reactor_fac, 1, 99);
        }
        speed_val = (moves 
            * range(reactor_fac - Vehs[veh_id].dmg_incurred, 0, 9999) + speed_val - 1) / speed_val;
        speed_val = range(speed_val, (triad == TRIAD_SEA) ? 2 : 1, 999) * Rules.move_rate_roads;
    }
    return speed_val;
}

/*
Purpose: Calculate the cargo capacity of a unit. It seems Spore Launchers were considered to have 
         cargo capacity at one time.
// ORIGINAL: 0x005C1760 ?veh_cargo@@YAHH@Z 0x005C1760-0x005C17C4
// TRIED: still MISMATCH (0.853 similar) - the image computes `proto_id*sizeof(VehPrototype)` ONCE and reuses it for both the `carry_capacity` and `weapon_id` field reads (same array element, two fields); this tree recomputes it for the second read under every flag set tried. A local `VehPrototype *proto = &VehPrototypes[proto_id]` used for both field reads did not change the codegen. Also see the `add`-before-`shl` base-folding note on arm_strat (0x0057D270) - same class of divergence on the `carry_capacity` read itself.
// TRIED: proto_id` (not `uint32_t`/`int`) matches the image, which keeps the 16-bit `proto_id` in `si` for the `< MaxVehProtoFactionNum` and `== BSC_SPORE_LAUNCHER` compares and only does `movsx eax,si` at the array-indexing use sites.
// size      100 bytes
// prototype int (__cdecl ?veh_cargo@@YAHH@Z)(int vehID)
// callers   19   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: Cargo capacity
Status: Complete
*/
int __cdecl veh_cargo(int veh_id) {
    int16_t proto_id = Vehs[veh_id].proto_id;
    uint32_t cargo = VehPrototypes[proto_id].carry_capacity;
    return (cargo && proto_id < MaxVehProtoFactionNum && (get_proto_offense_rating(proto_id) < 0 
        || proto_id == BSC_SPORE_LAUNCHER)) ? Vehs[veh_id].morale + 1 : cargo;
}

/*
Purpose: Determine the extra percent cost for building a prototype. Includes a check if the faction 
         has the free prototype flag set or if the player is using one of the easier difficulties.
// ORIGINAL: 0x005C17D0 ?prototype_factor@@YAHH@Z 0x005C17D0-0x005C184F BYTE_EXACT
// LEVER: the `||` split into two separate guards - the image falls through to an inline `return 0` after the first test where a combined condition jumps to a shared epilogue. 18/48 -> BYTE_EXACT 48/48. Order measured: rule_flags first, `diff_level` first scores 5/48.
// size      127 bytes
// prototype int (__cdecl ?prototype_factor@@YAHH@Z)(int protoID)
// callers   3   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: Percent extra prototype cost
Status: Complete
*/
int __cdecl prototype_factor(int proto_id) {
    uint32_t faction_id = proto_id / MaxVehProtoFactionNum;
    // TWO GUARDS, NOT ONE `||`. Combined, VC6 short-circuits both tests into a
    // shared `return 0` epilogue and jumps to it; the image falls through to
    // its own inline `xor eax, eax; pop ebp; ret` after the first test, which
    // is what two separate statements produce. 18/48 -> 48/48.
    //
    // The ORDER is the image's too, and measured rather than assumed: testing
    // `diff_level` first scores 5 of 48.
    if (Players[faction_id].rule_flags & RFLAG_FREEPROTO) {
        return 0;
    }
    if (PlayersData[faction_id].diff_level <= DLVL_SPECIALIST) {
        return 0;
    }
    uint8_t triad = get_proto_triad(proto_id);
    switch (triad) {
      case TRIAD_SEA:
        return Rules.extra_pct_cost_proto_sea;
      case TRIAD_AIR:
        return Rules.extra_pct_cost_proto_air;
      case TRIAD_LAND:
      default:
        return Rules.extra_pct_cost_proto_land;
    }
}

/*
Purpose: Calculate the specified prototype's overall cost to build. Optional output parameter
         whether there is an associated 1st time prototype cost (true) or just the base (false).
// ORIGINAL: 0x005C1850 ?veh_cost@@YAHHHPAH@Z 0x005C1850-0x005C1A1D
// LEVER: three real structural fixes moved this 0.545 -> 0.854 similar
// (0/167 in position at the winning `/c /O2 /Gy /GR- /GX`):
//  (1) `prototype_factor(proto_id)` INLINED rather than called - call_diff
//      confirmed the image has no call to 0x005C17D0 here, it open-codes
//      the free-proto/difficulty/triad switch directly;
//  (2) a real out-of-line `base_cost_call` forwarder (veh.h/veh.cpp,
//      `sleep_call`'s own idiom) for the ONE call site where the image
//      keeps `base_cost` (0x005A5D00, "callers 1") as a genuine `call`
//      rather than the inline every other site gets - `base_cost(proto_id)`
//      written directly folded away under /O2, exposing the inner
//      `proto_cost` call call_diff flagged as WRONG CALLEE;
//  (3) `cost`/`proto_cost_first` as plain (signed) `int`, not `uint32_t` -
//      the image's `(cost*3)/4` and the `/100` scaling both carry the
//      `cdq`/sign-correction fixup a signed local earns and an unsigned one
//      does not, even though the values are never negative.
// TRIED: not chased further. The residual gap is the boolean
// materialization in `has_fac_built_call`'s `(mask & field) != 0` - the
// image computes an actual 0/1 via `and/neg/sbb/neg` before testing it
// (0x005C18B8), where this tree's `&&`-chain folds the AND straight into a
// `test`. Same result, different instruction selection; not a control-flow
// difference.
// size      461 bytes
// prototype int (__cdecl ?veh_cost@@YAHHHPAH@Z)(int protoID, int baseID, int* hasProtoCost)
// callers   12   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0050BA00 0x005A5D00
Return Value: Mineral cost
Status: Complete
*/
int __cdecl veh_cost(int proto_id, int base_id, BOOL *has_proto_cost) {
    // SIGNED, matching the image's `cdq; and edx,3; add eax,edx; sar eax,2`
    // divide-by-4 fixup below - `VehPrototypes[proto_id].cost` is a uint8_t
    // field, but the local the image computes with is a plain (signed) int.
    int cost = VehPrototypes[proto_id].cost;
    // BUG IN THE ORIGINAL: no `base_id >= 0` guard here, unlike the
    // FAC_SKUNKWORKS check below - `has_fac_built` reads `Bases[base_id]`
    // even when base_id is negative. The image's `test ebx,ebx; jl` guard
    // only appears on the second call site (0x005C19AE); this one
    // (0x005C1890) has none. Reproduced.
    if (proto_id < MaxVehProtoFactionNum
        && (get_proto_offense_rating(proto_id) < 0 || proto_id == BSC_SPORE_LAUNCHER)
        && has_fac_built_call(FAC_BROOD_PIT, base_id)) {
        cost = (cost * 3) / 4; // Decrease the cost of alien units by 25%
    }
    if (VehPrototypes[proto_id].plan == PLAN_COLONIZATION && base_id >= 0) {
        cost = range(cost, 1, 999);
    }
    int proto_cost_first = 0;
    if (proto_id >= MaxVehProtoFactionNum
        && !(VehPrototypes[proto_id].flags & PROTO_TYPED_COMPLETE)) {
        // `prototype_factor` INLINED, not called - the image has no call to
        // 0x005C17D0 here, it open-codes the free-proto/difficulty check and
        // the triad switch directly (0x005C1919-0x005C1986).
        int faction_id = proto_id / MaxVehProtoFactionNum;
        int factor;
        if (Players[faction_id].rule_flags & RFLAG_FREEPROTO
            || PlayersData[faction_id].diff_level <= DLVL_SPECIALIST) {
            factor = 0;
        } else {
            uint8_t triad = get_proto_triad(proto_id);
            switch (triad) {
              case TRIAD_SEA:
                factor = Rules.extra_pct_cost_proto_sea;
                break;
              case TRIAD_AIR:
                factor = Rules.extra_pct_cost_proto_air;
                break;
              case TRIAD_LAND:
              default:
                factor = Rules.extra_pct_cost_proto_land;
                break;
            }
        }
        // `base_cost` IS CALLED UNCONDITIONALLY here in the image, before the
        // FAC_SKUNKWORKS check - not short-circuited behind it the way a
        // `cond ? 0 : (factor * base_cost(...) + 50) / 100` ternary would.
        proto_cost_first = (factor * base_cost_call(proto_id) + 50) / 100;
        if (base_id >= 0 && has_fac_built_call(FAC_SKUNKWORKS, base_id)) {
            proto_cost_first = 0;
        }
        cost += proto_cost_first;
    }
    if (has_proto_cost) {
        *has_proto_cost = proto_cost_first != 0;
    }
    return cost;
}

/*
Purpose: Check if the land unit inside an air transport can disembark. The transport must be in 
         either a base or an airbase to do so.
// ORIGINAL: 0x005C1C40 ?veh_jail@@YAHH@Z 0x005C1C40-0x005C1D16
// size      214 bytes
// prototype int (__cdecl ?veh_jail@@YAHH@Z)(int vehID)
// callers   8   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
// TRIED: splitting the one big && chain into sequential early-return ifs; hoisting
//   Vehs[veh_id].x/y into locals; mixing split-return with the || form - all measured
//   worse or equal. Committed form already gets 6/63 agreeing, 0.800 similar; remaining
//   divergence is register naming only (ecx/edx swapped on otherwise identical opcodes)
Return Value: Can the specified unit disembark? true/false
Status: Complete
*/
BOOL __cdecl veh_jail(int veh_id) {
    if (get_triad(veh_id) == TRIAD_LAND && Vehs[veh_id].order == ORDER_SENTRY_BOARD
        && Vehs[veh_id].waypoint_x[0] >= 0 && get_triad(Vehs[veh_id].waypoint_x[0]) == TRIAD_AIR
        && (!(bit_at(Vehs[veh_id].x, Vehs[veh_id].y) & BIT_BASE_IN_TILE)
            || owner_at(Vehs[veh_id].x, Vehs[veh_id].y) >= 8
            || owner_at(Vehs[veh_id].x, Vehs[veh_id].y) <= 0)
        && !(bit_at(Vehs[veh_id].x, Vehs[veh_id].y) & BIT_AIRBASE)) {
        return true;
    }
    return false;
}

/*
Purpose: Sets all moves for the specified unit as expended.
// ORIGINAL: 0x005C1D20 ?veh_skip@@YAXH@Z 0x005C1D20-0x005C1D42 BYTE_EXACT
// size      34 bytes
// prototype void (__cdecl ?veh_skip@@YAXH@Z)(int vehID)
// callers   19   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005C1540
Return Value: n/a
Status: Complete
*/
// BODY IN veh.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


/*
Purpose: Initialize/reset the fake unit id (2048) used as a placeholder for various UI elements.
// ORIGINAL: 0x005C1D50 ?veh_fake@@YAHHH@Z 0x005C1D50-0x005C1D6F BYTE_EXACT
// size      31 bytes
// prototype int (__cdecl ?veh_fake@@YAHHH@Z)(int protoID, int factionID)
// callers   13   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005C02D0
Return Value: Fake unit id (2048)
Status: Complete
*/
// BODY IN veh.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


/*
Purpose: Activate the specified unit and clear the current action.
// ORIGINAL: 0x005C1D70 ?veh_wake@@YAHH@Z 0x005C1D70-0x005C1E1F BYTE_EXACT
// size      175 bytes
// prototype int (__cdecl ?veh_wake@@YAHH@Z)(int vehID)
// callers   6   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004C9A50 0x005C1540
// LEVER: `order` as int8_t, matching Vehs[].order's OWN (signed) field type -
//        `uint8_t` forces unsigned `jb`/`jae` on the range check below where
//        the image has signed `jl`/`jge`. Moved 0.903 -> 0.964.
// LEVER: no `state` local - the image re-reads Vehs[].state from memory at
//        each use (`test dword ptr [esi+..], 0x4000000`, then
//        `mov eax,[esi+..] / test ah,2 / test al,4`, then again for the
//        trailing `and`), and caching it in a local cost a spill slot that
//        pushed the whole frame onto `push ecx` instead of the image's
//        `push ebp` frame. Byte-exact 55/55 on removing it alone.
Return Value: Unit id (doesn't look to be used on return)
Status: Complete
*/
int __cdecl veh_wake(int veh_id) {
    if (Vehs[veh_id].order >= ORDER_FARM && Vehs[veh_id].order < ORDER_MOVE_TO && !(Vehs[veh_id].state & VSTATE_CRAWLING)) {
        // TODO bug fix: Issue with moves_expended size / speed return, see veh_skip()
        Vehs[veh_id].moves_expended = (uint8_t)(speed(veh_id, false) - Rules.move_rate_roads);
        int terra_turns = Vehs[veh_id].terraforming_turns;
        if (terra_turns) {
            terra_turns -= contribution(veh_id, Vehs[veh_id].order - 4);
            if (terra_turns < 0) {
                terra_turns = 0;
            }
            Vehs[veh_id].terraforming_turns = (uint8_t)terra_turns;
        }
    }
    if (Vehs[veh_id].state & VSTATE_UNK_200 && Vehs[veh_id].order_auto_type == ORDERA_ON_ALERT
        && !(Vehs[veh_id].state & VSTATE_UNK_4)) {
        Vehs[veh_id].moves_expended = 0;
    }
    Vehs[veh_id].order = ORDER_NONE;
    Vehs[veh_id].state &= ~(VSTATE_UNK_200 | VSTATE_EXPLORE | VSTATE_UNK_1000000 
        | VSTATE_UNK_2000000 | VSTATE_UNK_8000000); // TODO: Identify VSTATE flags
    return veh_id;
}









/*
Purpose: Send a unit home. Search the friendly and pacted bases, then - for an air unit - the
         friendly carriers and air supply convoys, then the airbases inside the search radius, for
         the closest place this unit could return to. Clear its orders if it is already standing
         there, otherwise point a move-to waypoint at it.
// ORIGINAL: 0x004CBAA0 ?action_home@@YAHHH@Z 0x004CBAA0-0x004CC35F
// LEVER: MORE (18 calls vs image's 17) - `proto_power()` was the extra one,
//        called once at the `chassis_range == 1` site with no equivalent in
//        the image's `calls` list. Same fix as best_defender (0x5044D0):
//        proto_power() moved to veh.h as `MEASURED inline`, since it was a
//        plain non-`inline` function in this tree. Call count now agrees
//        (0 disagree); best flag set's similarity 0.349 -> 0.503 (/O2 /Oi-
//        /GR- /Oy- /GX).
// size      2239 bytes
// prototype int (__cdecl ?action_home@@YAHHH@Z)(int vehID, int)
// callers   8   call targets   6
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004E3EF0 0x0050DE50 0x005BF1F0 0x005C1540 0x005C1D20 0x00644F3A
Return Value: Was a destination resolved? true/false
Status: Complete
*/
int __cdecl action_home(int veh_id, int flags) {
    Veh &veh = Vehs[veh_id];
    const int faction_id = veh.faction_id;
    const int veh_x = veh.x;
    const int veh_y = veh.y;
    const int veh_region = (int)region_at(veh_x, veh_y);
    const uint32_t proto_id = (uint32_t)veh.proto_id;
    const uint8_t chassis_id = VehPrototypes[proto_id].chassis_id;
    const int chassis_range = Chassis[chassis_id].range;
    const uint8_t triad = Chassis[chassis_id].triad;
    int best_dist = 9999;
    int best_x = -1;
    int best_y = -1;
    // How far the unit could still travel: what is left of this turn, plus - for a chassis with
    // endurance - the sorties it has not spent yet. A range 1 chassis additionally trades its
    // remaining hitpoints for distance. speed() is re-asked at each of the original's call sites
    // rather than hoisted, so the call count is unchanged.
    int moves_left = range((int)speed(veh_id, false) - veh.moves_expended, 0, 999);
    int reach = (int)speed(veh_id, false)
        * (chassis_range - veh.terraforming_turns - 1) + moves_left;
    if (chassis_range == 1) {
        int hp_left = range((int)proto_power(veh_id) - veh.dmg_incurred, 0, 9999);
        reach += (int)speed(veh_id, false)
            * ((hp_left - 1) / VehPrototypes[proto_id].reactor_id);
    }
    const int max_dist = reach / (int)Rules.move_rate_roads;
    // Set by either of the two "the unit is already on alert for this exact waypoint" shortcuts.
    // Both of them skip the remaining searches AND the endurance gate below, so this cannot be
    // collapsed into an ordinary break.
    bool alert_target = false;

    for (int base_id = 0; base_id < BaseCurrentCount; base_id++) {
        Base &base = Bases[base_id];
        const int base_faction = base.faction_id_current;
        if (base_faction != faction_id
            && !(PlayersData[faction_id].diplo_treaties[base_faction] & DTREATY_PACT)) {
            continue; // not ours and not a pact partner's
        }
        if (triad == TRIAD_LAND) {
            if ((int)region_at(base.x, base.y) != veh_region) {
                continue; // no land route
            }
        } else if (triad == TRIAD_SEA) {
            if ((base.x != veh_x || base.y != veh_y) && !base_on_sea(base_id, veh_region)) {
                continue; // not on this body of water
            }
        }
        if (flags == 2 && (base.state & BSTATE_UNK_200000)) {
            continue;
        }
        const int dist = (x_dist(veh_x, base.x) + abs(veh_y - base.y)) >> 1;
        if ((veh.state & VSTATE_UNK_200) && veh.order_auto_type == ORDERA_ON_ALERT
            && base.x == veh.waypoint_x[1] && base.y == veh.waypoint_y[1] && dist <= max_dist) {
            best_dist = dist;
            best_x = base.x;
            best_y = base.y;
            alert_target = true;
            break;
        }
        if (dist < best_dist) {
            best_dist = dist;
            best_y = base.y;
            best_x = base.x;
        }
    }

    if (!alert_target) {
        if (triad == TRIAD_AIR) {
            // An air unit can also come down on a friendly carrier or air supply convoy.
            for (int other_id = 0; other_id < VehCurrentCount; other_id++) {
                Veh &other = Vehs[other_id];
                if (other.faction_id != faction_id) {
                    continue;
                }
                if (other_id == veh_id) {
                    continue;
                }
                if (!has_abil(other.proto_id, ABL_CARRIER)) {
                    const uint8_t other_chassis = VehPrototypes[other.proto_id].chassis_id;
                    if (VehPrototypes[other.proto_id].plan != PLAN_SUPPLY_CONVOY
                        || Chassis[other_chassis].triad != TRIAD_AIR) {
                        continue;
                    }
                }
                int dist = (x_dist(veh_x, other.x) + abs(veh_y - other.y)) >> 1;
                if ((flags & 1) || flags == 4) {
                    if (!is_human(faction_id)) {
                        dist += (int)speed(other_id, false) / (int)Rules.move_rate_roads;
                    }
                    if (VehPrototypes[other.proto_id].plan == PLAN_SUPPLY_CONVOY) {
                        // Recomputed raw, without the bias just added above.
                        const int moves = range((int)speed(veh_id, false)
                                                - veh.moves_expended, 0, 999);
                        const int raw_dist =
                            (x_dist(veh_x, other.x) + abs(veh_y - other.y)) >> 1;
                        if (raw_dist > moves / (int)Rules.move_rate_roads) {
                            continue; // the convoy is not reachable this turn
                        }
                    }
                }
                if (dist < best_dist) {
                    best_dist = dist;
                    best_x = other.x;
                    best_y = other.y;
                }
            }
        }

        // Airbases inside the radius. The radius widens as the best answer so far gets worse, and
        // saturates at the largest ring when nothing has been found at all.
        const int radius_count = RadiusRange[best_dist < 8 ? best_dist : 8];
        for (int i = 0; i < radius_count; i++) {
            const int x_radius = xrange(veh_x + RadiusOffsetX[i]);
            const int y_radius = veh_y + RadiusOffsetY[i];
            if (!on_map(x_radius, y_radius)) {
                continue;
            }
            Map *tile = map_loc(x_radius, y_radius);
            if (!(tile->bit & BIT_AIRBASE)) {
                continue;
            }
            if (tile->bit & BIT_VEH_IN_TILE) {
                const int occupier = tile->val2 & 0xF;
                if (occupier < MaxPlayerNum && occupier != faction_id
                    && !(PlayersData[faction_id].diplo_treaties[occupier] & DTREATY_PACT)) {
                    continue; // someone else is sitting on it
                }
            }
            if (!is_human(faction_id)) {
                // The AI additionally insists the airbase is inside its own territory.
                if (whose_territory(faction_id, x_radius, y_radius, NULL, false) != faction_id) {
                    continue;
                }
                int owner = (int)owner_at(x_radius, y_radius);
                if (owner >= MaxPlayerNum) {
                    owner = -1;
                }
                if (owner != faction_id) {
                    continue;
                }
            }
            const int dist = (x_dist(veh_x, x_radius) + abs(veh_y - y_radius)) >> 1;
            if ((veh.state & VSTATE_UNK_200) && veh.order_auto_type == ORDERA_ON_ALERT
                && !veh.dmg_incurred && x_radius == veh.waypoint_x[1]
                && y_radius == veh.waypoint_y[1] && dist <= max_dist) {
                // best_dist is deliberately left alone here, unlike the base loop's shortcut.
                best_x = x_radius;
                best_y = y_radius;
                alert_target = true;
                break;
            }
            if (dist < best_dist) {
                best_dist = dist;
                best_x = x_radius;
                best_y = y_radius;
            }
        }

        if (!alert_target && ((flags & 1) || flags == 4) && !Chassis[chassis_id].range) {
            return false; // only an endurance chassis answers these queries
        }
    }

    if (!on_map(best_x, best_y)) {
        return false; // nothing found
    }
    if ((flags & 1) && best_dist + 1 <= max_dist - 1) {
        return false; // comfortably within reach, nothing to do
    }
    if (flags == 4) {
        if (best_dist > max_dist) {
            return false;
        }
    } else if (flags < 0) {
        return true; // query only, no state change
    }
    if (best_x == veh_x && best_y == veh_y) {
        veh.order = ORDER_NONE;
        if (!is_human(faction_id)) {
            veh_skip(veh_id);
        }
        return true;
    }
    veh.order = ORDER_MOVE_TO;
    veh.waypoint_x[0] = (int16_t)best_x;
    veh.waypoint_y[0] = (int16_t)best_y;
    if (Chassis[VehPrototypes[veh.proto_id].chassis_id].range && is_human(faction_id)
        && best_dist > max_dist) {
        // Out of reach for a human-controlled endurance unit: drop the order rather than fly it
        // into the ground.
        veh.order = ORDER_NONE;
        // 0xFCFFBDFF clears VSTATE_UNK_200, VSTATE_EXPLORE, VSTATE_UNK_1000000 and
        // VSTATE_UNK_2000000, and nothing else.
        veh.state &= 0xFCFFBDFFU;
    }
    return true;
}

/*
Purpose: Weight the raw odds of an attack by how badly the faction's AI wants that particular
         stack dead. The odds arrive already computed by battle_fight; everything here is a
         strategic multiplier on top of them.
// ORIGINAL: 0x00565F20 ?compute_odds@@YAHHHHHH@Z 0x00565F20-0x005665CC
// TRIED: call structure already matches exactly (`osmx calls --all`'s 15
//            call sites resolve to the same 5 targets as the image's own
//            `calls` list: base_at, at_goal, stack_check x9, has_abil,
//            veh_at x2), so the 123/572 (0.540 best, /O2 /Gy /GR- /Oy- /GX)
//            gap is register/control-flow noise across a 1708-byte function,
//            not a missing inline or wrong callee. Not chased further in
//            this pass - too large for the budget available here.
// size      1708 bytes
// prototype int (__cdecl ?compute_odds@@YAHHHHHH@Z)(int, int factionID, int vehID1, int vehID2, int)
// callers   2   call targets   5
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004E3A50 0x00579CC0 0x005B9580 0x005BF1F0 0x005BFE90
Return Value: Weighted attack odds
Status: Complete

THE BASIS is what the target is worth divided by what the attacker costs. Out in the field the
defender's own prototype cost counts in full and the rest of its stack counts at ONE FIFTH, so a
stack of ten is worth roughly three of them; an Alien Artifact anywhere in that stack adds half
again. On a base tile the stack is ignored and only the defender's own cost is used, because the
garrison gets its own rule below. Either way the result is divided by the attacker's prototype
cost, which is what makes this a cost-effectiveness number rather than a value.

EVERYTHING AFTER THAT IS A DOUBLING, and they compose:

  x2   the defender's plan is PLAN_NAVAL_TRANSPORT or later, i.e. it cannot fight back
  x3/2 an offensive or combat LAND attacker, on a region this faction has no base plan for
  x2   the attacker's domain matches the tile - a sea unit over water or a land unit over land,
       never an air unit - but only for a base tile or a declared AI_GOAL_ATTACK goal
  x2   a base tile whose garrison is one real combat unit or fewer (total minus the ones with no
       offense rating); mutually exclusive with the domain bonus, which returns first
  x4   the unit carries VFLAG_IS_OBJECTIVE

then one of two branches on the attacker's triad.

AN AIR ATTACKER additionally gets x2 for a missile aimed at open water away from a base; and, if
the stack is TERRAFORMING, x2 for not being a missile, x2 over water when the faction carries
player flag 0x80000, and x2 again for each of the two terraform orders. On a base tile it looks
at the eight neighbours for a friendly stack containing LAND units - something already in place
to take the base - and doubles once for the first one it finds. Then, if the garrison is no
larger than a tenth of the faction's active count of this attacker's prototype plus one, it
scans this faction's sixty-four prototype slots for an active, typed, in-service design with
ABL_DROP_POD, and doubles for it - twice if the attacker is a missile.

ANY OTHER ATTACKER on a base tile counts LAND units across all eight neighbouring friendly
stacks and doubles once per whole garrison's worth of them, stopping at a million.

THE TAIL undoes some of it. An air-superiority attacker aimed at something that is neither
airborne nor armed is worth HALF; a plain PLAN_OFFENSIVE air unit that is not a missile is worth
a THIRD, or two thirds where the region's base plan is 2, because it is expected to be doing
something else.

Bug notes, reproduced rather than corrected:

  - veh_at() answers -1 for a tile with no unit, and both neighbour loops read
    Vehs[veh_id].faction_id from that -1 without checking. In the image the
    address is fifty-two bytes below Vehs[0] - four bytes inside Reactor - and
    it is compared against a live faction id. The read is preserved exactly:
    Vehs[-1] is as far out of bounds below the real array as it was above the
    image's Reactor table, and whatever uninitialised byte it compares is the
    original's behaviour, not a defect to fix.
  - The division by the attacker's prototype cost is a bare idiv with no zero guard.
  - The two terraform-order tests are two independent ifs in the original, not an else-if. They
    are mutually exclusive in practice because `order` holds one value, so this is a shape
    difference only, and it is transcribed as written.
  - The `while` below terminates only because `defenders` is known nonzero; the guard is the
    original's, not an addition.

The x wrap is course_xrange() rather than xrange(), for the one bit that separates them: the
original reads the low BYTE of MapIsFlat and tests bit zero (`mov cl, byte ptr [94988Ch] /
test cl, 1`), exactly as set_course and reset_territory do.

Verification note: eighteen mutants survive the sweep and every one is an equivalence that can be
read off another recovered body.

SIXTEEN ARE ARGUMENTS stack_check DOES NOT LOOK AT. Its recovered switch takes cond1, cond2 and
cond3 but each arm reads only some of them, and the -1 in an unread slot is therefore free:
type 1, type 16 and type 19 read cond1 alone, so their cond2 and cond3 are dead - two survivors
each at the stack cost, the two thin-garrison censuses, the drop-pod censuses and the defender
count; type 2 and type 3 read cond1 and cond2, so their cond3 is dead - one survivor each at the
artifact test, the terraforming test and the two neighbour counts. The original passes -1 in all
of them and so does this, because the call is the same call.

THE OTHER TWO ARE THE NINTH RADIUSBASE ENTRY. Both neighbour loops stop at eight, and mutating
`i < 8` to `i <= 8` adds entry eight, which is (0,0) - the tile under attack. The stack there is
the defender's, and a tile in this game holds one faction's units, so
Vehs[veh_at(x_def, y_def)].faction_id is the defender's faction and the loop's
`!= faction_id` skips it for any caller that is attacking rather than reinforcing. Every other
tile of the ring is separately covered, including entry 0, which the first sweep found untested.

A NOTE ON THE SWEEP ITSELF, because it cost a pass to work out: the mutation harness perturbs
digits inside trailing comments as well as in code. A `// RadiusBase entry 8 is (0,0) ...` note
on the `for` line produced four unkillable survivors that read exactly like real coverage holes.
The comment is now on its own line and carries no digits.
*/
int __cdecl compute_odds(int odds, int faction_id, int veh_id_atk, int veh_id_def, int base_id) {
    const int x_def = Vehs[veh_id_def].x;
    const int y_def = Vehs[veh_id_def].y;
    const int triad_atk = Chassis[VehPrototypes[Vehs[veh_id_atk].proto_id].chassis_id].triad;
    const int plan_atk = VehPrototypes[Vehs[veh_id_atk].proto_id].plan;
    if (base_id < -1) {
        // Strictly below -1 asks for the lookup; -1 asserts there is no base here.
        base_id = base_at(x_def, y_def);
    }
    const int region_plan =
        PlayersData[faction_id].region_base_plan[map_loc(x_def, y_def)->region];

    int value;
    if (base_id < 0) {
        const int cost_def = VehPrototypes[Vehs[veh_id_def].proto_id].cost;
        // The rest of the stack counts at one fifth; the defender itself counts in full.
        value = ((stack_check(veh_id_def, 7, -1, -1, -1) - cost_def) / 5 + cost_def) * odds;
        if (stack_check(veh_id_def, 2, PLAN_ALIEN_ARTIFACT, -1, -1)) {
            value = value * 3 / 2;
        }
    } else {
        value = VehPrototypes[Vehs[veh_id_def].proto_id].cost * odds;
    }
    value /= VehPrototypes[Vehs[veh_id_atk].proto_id].cost;

    if (VehPrototypes[Vehs[veh_id_def].proto_id].plan >= PLAN_NAVAL_TRANSPORT) {
        value *= 2;   // it cannot fight back
    }
    if (plan_atk <= PLAN_COMBAT && !region_plan && triad_atk == TRIAD_LAND) {
        value = value * 3 / 2;
    }
    if (base_id >= 0 || at_goal(faction_id, AI_GOAL_ATTACK, x_def, y_def)) {
        if ((triad_atk == TRIAD_SEA) == (is_ocean(x_def, y_def) != 0) && triad_atk != TRIAD_AIR) {
            value *= 2;   // the attacker's own domain, and this returns before the next rule
        } else if (base_id >= 0) {
            const int noncombat = stack_check(veh_id_def, 19, -1, -1, -1);
            const int total = stack_check(veh_id_def, 1, -1, -1, -1);
            if (total - noncombat <= 1) {
                value *= 2;   // one real defender or none
            }
        }
    }
    if (Vehs[veh_id_def].flags & VFLAG_IS_OBJECTIVE) {
        value *= 4;
    }

    if (triad_atk == TRIAD_AIR) {
        if (is_proto_missile(Vehs[veh_id_atk].proto_id)
            && is_ocean(x_def, y_def) && base_id < 0) {
            value *= 2;
        }
        if (stack_check(veh_id_def, 2, PLAN_TERRAFORMING, -1, -1)) {
            if (!is_proto_missile(Vehs[veh_id_atk].proto_id)) {
                value *= 2;
            }
            if (is_ocean(x_def, y_def) && (PlayersData[faction_id].flags & 0x80000)) {
                value *= 2;   // unnamed player flag; PlayerFlagsBitfield skips this bit
            }
            // Two separate tests in the original, not an else-if; `order` cannot be both.
            if (Vehs[veh_id_def].order == ORDER_TERRAFORM_UP) {
                value *= 2;
            }
            if (Vehs[veh_id_def].order == ORDER_TERRAFORM_DOWN) {
                value *= 2;
            }
        }
        if (base_id >= 0) {
            for (int i = 0; i < 8; i++) {
                // The ninth RadiusBase entry is the tile itself and is never reached.
                const int x_radius = course_xrange(x_def + RadiusBaseX[i]);
                const int y_radius = y_def + RadiusBaseY[i];
                if (!on_map(x_radius, y_radius)) {
                    continue;
                }
                const int veh_id = veh_at(x_radius, y_radius);
                // Unchecked -1: see the bug note. Preserved deliberately.
                if (Vehs[veh_id].faction_id != faction_id) {
                    continue;
                }
                if (stack_check(veh_id, 3, TRIAD_LAND, -1, -1)) {
                    value *= 2;   // somebody is already on the ground next to it
                    break;
                }
            }
            const int noncombat = stack_check(veh_id_def, 19, -1, -1, -1);
            const int total = stack_check(veh_id_def, 1, -1, -1, -1);
            const int proto_atk = Vehs[veh_id_atk].proto_id;
            if (total - noncombat
                <= PlayersData[faction_id].proto_id_active[proto_atk] / 10 + 1) {
                for (int i = 0; i < MaxVehProtoFactionNum; i++) {
                    const int proto_id = faction_id * MaxVehProtoFactionNum + i;
                    if (!(VehPrototypes[proto_id].flags & PROTO_ACTIVE)) {
                        continue;
                    }
                    if (!(VehPrototypes[proto_id].flags & PROTO_TYPED_COMPLETE)) {
                        continue;
                    }
                    if (!PlayersData[faction_id].proto_id_active[proto_id]) {
                        continue;
                    }
                    if (!has_abil(proto_id, ABL_DROP_POD)) {
                        continue;
                    }
                    value *= 2;
                    if (is_proto_missile(Vehs[veh_id_atk].proto_id)) {
                        value *= 2;
                    }
                    break;
                }
            }
        }
    } else if (base_id >= 0) {
        int attackers = 0;
        for (int i = 0; i < 8; i++) {
            const int x_radius = course_xrange(x_def + RadiusBaseX[i]);
            const int y_radius = y_def + RadiusBaseY[i];
            if (!on_map(x_radius, y_radius)) {
                continue;
            }
            const int veh_id = veh_at(x_radius, y_radius);
            // Unchecked -1 here too.
            if (Vehs[veh_id].faction_id != faction_id) {
                continue;
            }
            attackers += stack_check(veh_id, 3, TRIAD_LAND, -1, -1);
        }
        const int defenders = stack_check(veh_id_def, 16, -1, -1, -1);
        if (defenders) {   // without this the subtraction below never terminates
            while (attackers > defenders && value < 1000000) {
                value *= 2;
                attackers -= defenders;
            }
        }
    }

    if (plan_atk == PLAN_AIR_SUPERIORITY) {
        const int proto_def = Vehs[veh_id_def].proto_id;
        if (Chassis[VehPrototypes[proto_def].chassis_id].triad == TRIAD_AIR) {
            return value;   // an interceptor doing its job
        }
        if (!Weapon[VehPrototypes[proto_def].weapon_id].offense_rating) {
            return value;   // nothing there to intercept anything
        }
        return value / 2;
    }
    if (triad_atk == TRIAD_AIR && plan_atk == PLAN_OFFENSIVE
        && !is_proto_missile(Vehs[veh_id_atk].proto_id)) {
        if (region_plan == 2) {
            return value * 2 / 3;
        }
        return value / 3;
    }
    return value;
}

/*
Purpose: Choose which base a native life form standing on the specified tile should head for.
// ORIGINAL: 0x005665D0 ?alien_base@@YAHHHH@Z 0x005665D0-0x00566802
// size      562 bytes
// prototype int (__cdecl ?alien_base@@YAHHHH@Z)(int vehID, int xCoord, int yCoord)
// callers   1   call targets   4
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0050DE50 0x005B9580 0x005BFE90 0x00644F3A
Return Value: Base id of the most attractive target, or -1 when there is none
Status: Complete

WHAT THE SCORE MEANS. The comparison variable is a cost, not a merit: the loop keeps the LOWEST
value, seeded at 9999, and the winner is published in BaseFindDist for the caller. The cost is the
radiating distance from the unit to the base, scaled by 32 and then divided by that base's own
mineral plus energy intake plus 32, so a productive base pulls a worm from further away. Three
adjustments follow, each of which the original applies as a whole doubling or halving:

  - an Alien Artifact standing in the base halves the cost. stack_check type 2 counts units in the
    tile's stack whose prototype PLAN is PLAN_ALIEN_ARTIFACT, so this is literally "something is
    sitting there that Planet wants back".
  - the unit's own home base halves it again. Native life gets a home base when a faction breeds
    it, and this is what makes a bred worm drift back toward the base that made it.
  - a base whose owner runs a positive PLANET rating and has no eco damage DOUBLES it. That is the
    in-game promise that a green faction is left alone, expressed as a cost the worm has to pay.

Verification note: the third test compares Veh+0x26 against the base's owner. veh.h names that
byte order_auto_type, and on this path the original is reading it as a faction id rather than as
an order type - base.cpp reads the same byte as a resource id for convoys, so the field is already
known to be multi-purpose. Transcribed as the original reads it rather than corrected.

REACHABILITY. The region test only applies to a native standing on LAND. Water bodies are numbered
from MaxRegionLandNum upward (see the region notes in map.h), so a unit already at sea reaches
every base and the test is skipped - which is the whole reason it is written as a bound and not as
an equality. On land, the base has to share the unit's region, except for a Sealurk - prototype
14, the only native the loop names - which may also take a base that merely touches the region it
is standing in, and base_on_sea answers that.

Verification note: stack_check's fifth argument survives a mutation sweep and always will. Type 2
reads cond1 and cond2 and never looks at cond3, so the -1 the original pushes for it cannot be
observed by any fixture. The fourth argument is observable and is covered - the artifact unit in
the fixture carries a non-zero faction precisely so that turning that -1 into 0 changes the count.

The original inlines region_at (0x00500220), x_dist (0x00579790) and the two-argument vector_dist
(0x004F8090); each inlined copy is instruction-for-instruction the standalone body, so they are
called here. One consequence is visible in the disassembly and worth recording: the inlined
map_loc halves x with SAR, while the exported map_loc that region_at reaches takes uint32_t and
would halve it with SHR. The two agree on every non-negative x, and alien_move at 0x005668A0 -
the only caller - passes a unit's own sign-extended coordinates.

// LEVER: FEWER (6 calls vs image's 7) - the image calls `abs()` FOUR times
//        here and never calls vector_dist(int,int) itself (0x4F8090 is not
//        in its `calls` list, despite being a real, `callers 5`-elsewhere
//        function). Open-coded the `vector_dist(x_dist(...), abs(...))`
//        call as the abs()/largest/smallest expansion vector_dist's own
//        body performs - the same pattern base.cpp's black_market and
//        base_find already use for the 4-arg form. Call count now agrees
//        (0 disagree); best flag set's similarity 0.782 -> 0.883
//        (/O2 /Oi- /GR- /Oy- /GX).
*/
int __cdecl alien_base(int veh_id, int x, int y) {
    int best_value = 9999;
    BaseFindDist = 9999;
    int base_id_best = -1;
    const int region = region_at(x, y);
    for (int base_id = 0; base_id < BaseCurrentCount; base_id++) {
        Base &base = Bases[base_id];
        if (Vehs[veh_id].proto_id == BSC_SEALURK) {
            if (region < MaxRegionLandNum && region != (int)region_at(base.x, base.y)
                && !base_on_sea(base_id, region)) {
                continue;
            }
        } else if (region < MaxRegionLandNum && region != (int)region_at(base.x, base.y)) {
            continue;
        }
        // Open-coded vector_dist(x_dist(...), abs(...)): the image calls
        // abs() four times, never vector_dist itself - the same expansion
        // as black_market (base.cpp, 0x4E4D00-ish) and base_find.
        int dx = x_dist(x, base.x);
        int dy = abs(y - base.y);
        int abs_dx = abs(dx);
        int abs_dy = abs(dy);
        int largest = abs_dx;
        if (abs_dx <= abs_dy) {
            largest = abs_dy;
        }
        int smallest = abs_dx;
        if (abs_dx >= abs_dy) {
            smallest = abs_dy;
        }
        int value = (largest - ((((abs_dy + abs_dx) >> 1) - smallest + 1) >> 1)) * 32
            / (base.mineral_intake_2 + base.energy_intake_2 + 32);
        if (stack_check(veh_at(base.x, base.y), 2, PLAN_ALIEN_ARTIFACT, -1, -1)) {
            value /= 2;
        }
        if (Vehs[veh_id].home_base_id == base_id) {
            value /= 2;
        }
        if (Vehs[veh_id].order_auto_type != base.faction_id_current
            && PlayersData[base.faction_id_current].soc_effect_active.planet > 0
            && !base.eco_damage) {
            value *= 2;
        }
        if (value <= best_value) {
            best_value = value;
            base_id_best = base_id;
        }
    }
    if (base_id_best < 0) {
        return base_id_best;
    }
    BaseFindDist = best_value;
    return base_id_best;
}

/*
Purpose: Kill a vehicle stack by walking the link list to its tail, then
         unwinding it leaf-first (each kill can free the next link, so the
         walk takes the low side whenever the branch points upward).
// ORIGINAL: 0x005B9510 ?stack_kill@@YAXH@Z 0x005B9510-0x005B9577 BYTE_EXACT
// size      103 bytes
// prototype void (__cdecl ?stack_kill@@YAXH@Z)(int vehID)
// callers   5   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005C08C0
// HOMED from src/recovered/units/005b9510.cpp, 2026-09-03: the claim was
//            archive-only while a PENDING_BODY forwarder named it - the
//            address-index landmine. The artifact's two dead g_009528xx
//            bindings were not carried; the body never reads them.
Return Value: n/a
Status: Complete
*/
// The vehLinks array: same 0x34-stride vehicle record the stack walkers
// use, with the two short link fields proven by the disassembly's scaled
// addressing (lea x2 for i*13, then a *4 SIB scale on the load).
struct VehLink {
    uint8_t pad_0_[0x2C];
    short field_2c_;
    short field_2e_;
    uint8_t pad_30_[0x34 - 0x2E - 2];
};

static VehLink *const g_vehLinks = (VehLink *)0x00952858;

// Declared, never defined here (a definition would be inlined); the editor
// command family calls it by name.
void __cdecl veh_kill(int vehID);

void __cdecl stack_kill(int a1) {
    if (a1 >= 0) {
        short s = g_vehLinks[a1].field_2e_;
        while (s >= 0) {
            a1 = s;
            s = g_vehLinks[a1].field_2e_;
        }
    }
    if (a1 >= 0) {
        do {
            int next;
            if (a1 < 0) {
                next = a1;
            } else {
                next = g_vehLinks[a1].field_2c_;
            }
            veh_kill(a1);
            if (next > a1) {
                --next;
            }
            a1 = next;
        } while (a1 >= 0);
    }
}

// ===== veh.cpp, P2 wave 2: the vehicle removal / query verbs =====

// ---- the .data slots veh_kill compacts when a vehicle id disappears ----
// 0x008C6B2C is the vehicle id the map cursor is hovering: MainInterface::
// on_iface_mouse_move (0x0045D000) writes it and clears the base id beside it
// before handing the pair to StatusWin::draw_active. 0x008C6B34 sits in the
// same run; the whole tree reaches it exactly once, here, and re-points it
// exactly as it re-points the hover slot, so the name records the role.
int *const StatusHoverVehId = (int *)0x008C6B2C;
int *const StatusHoverVehIdAlt = (int *)0x008C6B34;

// 0x0093E31C. The queue the multiplayer deletion path parks a dying vehicle on
// until the next turn boundary, guarded by IsMultiplayerNet.
DeletionList VehDeletionList;  // 0x0093E31C

// 0x0093E908. A dword * holding the address of one of the vehicle-id slots
// above: the interface keeps it pointing at whichever selection it wants to
// follow, and veh_kill walks the dereference so a deletion the pointer names is
// re-pointed rather than left dangling. It skips three slots by identity -
// CursorVehIndex, FocusVehId and VehSelectionFollowLimit - which is how the
// image says "the ones already handled above".
// The borrowed selection pointer is NetDaemonNet's channel1 payload, not a
// second global overlapping that daemon-owned word.

// 0x009A64DC. Another vehicle-id slot, and one with its own extra rule: it is
// also decremented when it EQUALS the deleted id, which the three above do not
// do. action_airdrop stores/reloads it across capture_base; veh_kill updates
// the same signed slot, and synch transports its four-byte representation.
// RECOVERY_OBJECT: v2 0x009A64DC 0x009A64E0 ?VehSelectionFollowLimit@@3HA 0x005C08C0
int VehSelectionFollowLimit;

// 0x00691DC0, the log_say title. The image logs every vehicle removal with the
// vehicle id, its owner and its prototype as the three substitutions.
char *const LogKillingVehicle = (char *)0x00691DC0;  // "Killing vehicle (v,who,v2)"

// 0x009B227C, the slot veh_find reports its winning distance into. It sits in
// the VehLiftX (0x009B2278) / VehDropLiftVehID (0x009B2280) run and is named by
// what writes it - nothing else in the tree reads or writes it.
int *const VehFindBestDist = (int *)0x009B227C;

// Two PlayerData fields the image reaches by the array's own 0x20CC stride:
// 0x0096DA38 is +0x1058 into the record and 0x0096D238 is +0x0858. The
// diplomacy views in faction.h are bound the same way (DiploVendettaTable and
// friends) - one address per view, the array spelling cannot share the single
// pair index these compute first. +0x1058 is a dword counter of the faction's
// living non-worm units (decremented only when VehPrototypes[proto].plan <= 7)
// and +0x0858 is the per-prototype byte run beside it.
uint32_t *const FactionAliveUnitCount = (uint32_t *)0x0096DA38;
uint8_t *const FactionProtoSeenCount = (uint8_t *)0x0096D238;

/*
Purpose: Remove a vehicle from the game entirely.
// ORIGINAL: 0x005C08C0 ?veh_kill@@YAXH@Z 0x005C08C0-0x005C0AF4
// size      564 bytes
// prototype void (__cdecl ?veh_kill@@YAXH@Z)(int vehID)
// callers   15   call targets   6
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0041A7A0 0x0041A7E0 0x0052DE70 0x005BFFA0 0x006262F0 0x00645930
// HOMED from src/recovered/units/005c08c0.cpp, 2026-09-04, which carried 14
// anonymous address bindings; every one is named or modelled above, and the
// pending_bodies.cpp forwarder of the same name is gone (this definition
// replaces it). The 0x952858 walk is over Vehs[] itself, not a second array:
// eax starts at Vehs + 0x30 and advances 0x34, so `word [eax]`/`word [eax+2]`
// are next_veh_id_stack/prev_veh_id_stack and `byte [eax - 0x1f]`/`word
// [eax - 0x1c]` are the PREVIOUS record's order and waypoint_x[0]. The
// g_vehLinks spelling stack_kill keeps is the same memory shifted by 0x30.
Return Value: n/a
Status: MISMATCH #15 - the `xor edx,edx` zero-extending faction_id for log_say
        lands one slot away from the image's; every call, table stride and
        indirect target matches (0.79 past it).
*/
void __cdecl veh_kill(int veh_id) {
    if (veh_id < 0) {
        return;
    }

    if (IsMultiplayerNet != 0) {
        VehDeletionList.add_deletion(veh_id);
    }

    log_say(LogKillingVehicle, veh_id, Vehs[veh_id].faction_id,
            Vehs[veh_id].proto_id);

    const int x = Vehs[veh_id].x;
    const int home_base_id = Vehs[veh_id].home_base_id;
    const int y = Vehs[veh_id].y;
    const int proto_id = Vehs[veh_id].proto_id;
    const int faction_id = Vehs[veh_id].faction_id;

    if (VehPrototypes[proto_id].plan <= 7) {
        --FactionAliveUnitCount[faction_id * 2099];
    }
    --FactionProtoSeenCount[faction_id * 2099 + proto_id];

    veh_lift(veh_id);

    if (veh_id < VehCurrentCount - 1) {
        memcpy(&Vehs[veh_id], &Vehs[veh_id + 1],
               (VehCurrentCount - veh_id - 1) * sizeof(Veh));
    }
    if (veh_id < 0x800) {
        --VehCurrentCount;
    }
    if (VehCurrentCount > 0) {
        int count = VehCurrentCount;
        Veh *rec = &Vehs[0];
        do {
            if (veh_id < rec->prev_veh_id_stack) {
                --rec->prev_veh_id_stack;
            }
            if (veh_id < rec->next_veh_id_stack) {
                --rec->next_veh_id_stack;
            }
            if (rec->order == 1) {
                const short transport = rec->waypoint_x[0];
                if (transport == veh_id) {
                    rec->order = 0;
                } else if (veh_id < transport) {
                    rec->waypoint_x[0] = transport - 1;
                }
            }
            ++rec;
        } while (--count != 0);
    }

    if (ConsoleGlobal.selected_vehicle_ == veh_id) {
        ConsoleGlobal.selected_vehicle_ = -1;
    } else if (veh_id < ConsoleGlobal.selected_vehicle_) {
        --ConsoleGlobal.selected_vehicle_;
    }
    if (ConsoleGlobal.focus_vehicle_ == veh_id) {
        ConsoleGlobal.focus_vehicle_ = -1;
    } else if (veh_id < ConsoleGlobal.focus_vehicle_) {
        --ConsoleGlobal.focus_vehicle_;
    }
    if (*StatusHoverVehId == veh_id) {
        *StatusHoverVehId = -1;
    } else if (veh_id < *StatusHoverVehId) {
        --*StatusHoverVehId;
    }
    if (*StatusHoverVehIdAlt == veh_id) {
        *StatusHoverVehIdAlt = -1;
    } else if (veh_id < *StatusHoverVehIdAlt) {
        --*StatusHoverVehIdAlt;
    }
    if (VehSelectionFollowLimit != 0 && veh_id <= VehSelectionFollowLimit) {
        --VehSelectionFollowLimit;
    }
    if (IsMultiplayerNet != 0) {
        int *slot = NetDaemonNet.vehicle_selection_follow();
        if (slot != 0 && slot != &ConsoleGlobal.selected_vehicle_ && slot != &VehSelectionFollowLimit &&
            slot != &ConsoleGlobal.focus_vehicle_) {
            if (veh_id <= *slot) {
                --*slot;
            }
        }
    }

    TutWinBaseWindow.check_loc(x, y, 0);
    TutWinBaseWindow.check_base(home_base_id);
}

/*
Purpose: Kill a vehicle, or strip it down to a transport's cargo first.
// ORIGINAL: 0x005C0B00 ?kill@@YAXH@Z 0x005C0B00-0x005C0CB0
// size      432 bytes
// prototype void (__cdecl ?kill@@YAXH@Z)(int vehID)
// callers   25   call targets   9
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0041A7A0 0x0046AEF0 0x0046AF40 0x005B8EE0 0x005B9510 0x005B9580 0x005BF1F0 0x005BFE90 0x005C08C0
// HOMED from src/recovered/units/005c0b00.cpp, 2026-09-04. Its `g_tbl1` /
// `g_tbl2` / `g_tbl3` scaffolding is Vehs[] / VehPrototypes[] / Weapon[]:
// `base2[0]` is VehPrototypes[p].weapon_id (0x9AB88D = the array front +
// 0x25) and `base2[3]` is VehPrototypes[p].carry_capacity (+0x28).
Return Value: n/a
Status: HOMED, unmeasured tier.
*/
void __cdecl kill(int veh_id) {
    const int x = Vehs[veh_id].x;
    const int y = Vehs[veh_id].y;
    const int proto_id = Vehs[veh_id].proto_id;
    const int faction_id = Vehs[veh_id].faction_id;
    const int home_base_id = Vehs[veh_id].home_base_id;

    const int worm_abil = has_abil(proto_id, 2);

    const int carry_capacity = VehPrototypes[proto_id].carry_capacity;
    int passengers;
    if (carry_capacity == 0 || proto_id > 0x3f ||
        (Weapon[VehPrototypes[proto_id].weapon_id].offense_rating >= 0 &&
         proto_id != 0xf)) {
        passengers = carry_capacity;
    } else {
        passengers = Vehs[veh_id].morale + 1;
    }

    if (passengers == 0) {
        veh_kill(veh_id);
    } else {
        if (y >= 0 && y < MapLatitudeBounds && x >= 0 && x < MapLongitudeBounds) {
            // `[+0]` read as a byte and masked 0xE0 is `climate`'s altitude
            // bits - see map.h's note on the 0x2C stride. 0x60 and above is
            // ocean, and a vehicle that cannot carry passengers drowns there
            // rather than sitting in the stack.
            if ((map_loc(x, y)->climate & 0xe0) >= 0x60) {
                veh_kill(veh_id);
                TutWinBaseWindow.check_base(home_base_id);
                return;
            }
        } else if (x == -2) {
            stack_kill(veh_id);
            return;
        }

        stack_veh(veh_id, 2);
        stack_kill(veh_id);
    }

    TutWinBaseWindow.check_base(home_base_id);

    if (VehBattleUnkTgl[0] == 0 && y >= 0 && y < MapLatitudeBounds && x >= 0 &&
        x < MapLongitudeBounds) {
        if (faction_id == ConsoleGlobal.faction_id_) {
            const int at = veh_at(x, y);
            const int checked = stack_check(at, 1, faction_id, -1, -1);
            if (checked == 0 || worm_abil != 0) {
                draw_radius(x, y, 2, 2);
                return;
            }
        }
        draw_tile(x, y, 2);
    }
}

// ORIGINAL: 0x004CD2F0 ?action_destruct@@YAXH@Z 0x004CD2F0-0x004CD4D1 FILE
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/004cd2f0.cpp", "recovery/tasks/mapwin-owner-coverage-wave.json", "Pinned instructions 4CD365 and 4CD44E identify reactor_id and PLAN_ALIEN_ARTIFACT, correcting seed cost/class labels."], "reason": "Recover vehicle self-destruction through typed reactor and plan fields, nine radius offsets and real stack lifecycle; original rereads the vehicle prototype after weap_val. Preserve seed identity and original extent without granting a byte claim."}
void __cdecl action_destruct(int veh_id) {
    const int x = Vehs[veh_id].x;
    const int y = Vehs[veh_id].y;
    boom(x, y, 0);

    const int proto_id = Vehs[veh_id].proto_id;
    const int faction_id = Vehs[veh_id].faction_id;
    int weapon_value = weap_val(proto_id, faction_id);
    if (weapon_value < 1) weapon_value = 1;
    else if (weapon_value > 20) weapon_value = 20;
    const int damage =
        static_cast<int>(VehPrototypes[Vehs[veh_id].proto_id].reactor_id) * weapon_value / 2;

    kill(veh_id);

    for (int offset = 0; offset < 9; ++offset) {
        int tile_x = x + RadiusOffsetX[offset];
        if ((MapIsFlat & 1) == 0) {
            if (tile_x < 0) tile_x += MapLongitudeBounds;
            else if (tile_x >= MapLongitudeBounds) tile_x -= MapLongitudeBounds;
        }
        const int tile_y = y + RadiusOffsetY[offset];
        if (tile_y < 0 || tile_y >= MapLatitudeBounds ||
            tile_x < 0 || tile_x >= MapLongitudeBounds) {
            continue;
        }
        if (base_at(tile_x, tile_y) >= 0) continue;

        for (int damaged_vehicle_id = veh_at(tile_x, tile_y); damaged_vehicle_id >= 0;
             damaged_vehicle_id = Vehs[damaged_vehicle_id].next_veh_id_stack) {
            Vehs[damaged_vehicle_id].dmg_incurred = static_cast<uint8_t>(
                Vehs[damaged_vehicle_id].dmg_incurred + damage);
        }

        int vehicle_id = stack_fix(veh_at(tile_x, tile_y));
        while (vehicle_id >= 0) {
            const Veh &vehicle = Vehs[vehicle_id];
            const VehPrototype &prototype = VehPrototypes[vehicle.proto_id];
            int threshold;
            if (prototype.plan == PLAN_ALIEN_ARTIFACT) {
                threshold = 1;
            } else {
                int reactor = prototype.reactor_id;
                if (reactor < 1) reactor = 1;
                else if (reactor > 100) reactor = 100;
                threshold = reactor * 10;
            }
            if (vehicle.dmg_incurred >= threshold) {
                stack_veh(vehicle_id, 1);
                stack_kill(vehicle_id);
                vehicle_id = stack_fix(veh_at(tile_x, tile_y));
                continue;
            }
            vehicle_id = vehicle.next_veh_id_stack;
        }
    }
}

/*
Purpose: Report the vehicle the map cursor is nearest to.
// ORIGINAL: 0x005C0CB0 ?veh_find@@YAHHHHH@Z 0x005C0CB0-0x005C0DA9
// size      249 bytes
// prototype int (__cdecl ?veh_find@@YAHHHHH@Z)(int xCoord, int yCoord, int factionID, int skipVehID)
// callers   1   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
// HOMED from src/unrecovered/005c0cb0.cpp, 2026-09-04. Its mangled name
// decodes as return int + FOUR int parameters - the return type is the first H
// - and the body reads exactly four stack slots, so the archive's
// four-argument body was right and its five-argument prototype line was not.
// TRIED: `extern "C" int abs(int);` + a local `#pragma function(abs)` (the
//        archive's device for a real `call _abs`). stdafx.h's pragma line
//        already carries abs, so the plain `abs()` here compiles to the same
//        call; the extra local pragma is dead weight and is not carried.
// TRIED: the archive's own spelling - `best_idx` initialised in a declaration
//        and `or eax,0FFFFFFFFh` vs `mov eax,-1` left as the residual
//        MISMATCH; named `int best_idx = -1;` here and the divergence is the
//        same one instruction.
Return Value: the nearest vehicle's id, -1 when nothing qualified.
Status: MISMATCH.
*/
int __cdecl veh_find(int x, int y, int faction_id, int skip_veh_id) {
    int best_dist = 9999;
    *VehFindBestDist = 9999;
    int best_idx = -1;
    int i = 0;
    if (VehCurrentCount > 0) {
        do {
            if ((faction_id < 0 || Vehs[i].faction_id == (unsigned)faction_id) &&
                i != skip_veh_id) {
                int d1 = abs(x - Vehs[i].x);
                if (!(MapIsFlat & 1) && d1 > (int)MapLongitude) {
                    d1 = MapLongitudeBounds - d1;
                }
                const int d2 = abs(y - Vehs[i].y);
                const int ad1 = abs(d1);
                const int ad2 = abs(d2);
                const int mx = ad1 <= ad2 ? ad2 : ad1;
                const int mn = ad2 <= ad1 ? ad2 : ad1;
                const int dist = mx - ((ad1 + ad2 >> 1) - mn + 1 >> 1);
                if (dist <= best_dist) {
                    best_idx = i;
                    best_dist = dist;
                }
            }
            ++i;
        } while (i < VehCurrentCount);
        if (best_idx >= 0) {
            *VehFindBestDist = best_dist;
        }
    }
    return best_idx;
}

/*
Purpose: Can this vehicle act this turn?
// ORIGINAL: 0x005C1B70 ?veh_ready@@YAHH@Z 0x005C1B70-0x005C1C38
// size      200 bytes
// prototype int (__cdecl ?veh_ready@@YAHH@Z)(int vehID)
// callers   14   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005C1540 0x005C1C40
// HOMED from src/unrecovered/005c1b70.cpp, 2026-09-04, and its
// pending_bodies.cpp forwarder removed - this definition replaces it. The
// archive's `extern char g_952828_arr[]` scaffolding is Vehs[].
// TRIED: separate `if (cond) return 0;` per check - duplicated the epilogue
//        (245 vs 200 bytes); the nested && chain sharing one `return 0` is
//        what the image has.
Return Value: 1 when the vehicle has moves left and is not jailed.
Status: MISMATCH #9 - the veh_id * 0x34 multiply lowers through a different
        lea/shl chain than the image's.
*/
int __cdecl veh_ready(int veh_id) {
    if (veh_id >= 0 && Vehs[veh_id].y >= 0 && Vehs[veh_id].y < MapLatitudeBounds &&
        Vehs[veh_id].x >= 0 && Vehs[veh_id].x < MapLongitudeBounds &&
        Vehs[veh_id].faction_id == (uint8_t)ConsoleGlobal.faction_id_ &&
        ((Vehs[veh_id].flags & 0x40) == 0 || (Vehs[veh_id].flags & 0x400) == 0) &&
        veh_jail(veh_id) == 0 &&
        ((Vehs[veh_id].state & 0x4200) != 0 || Vehs[veh_id].order < 1 ||
         Vehs[veh_id].order > 3)) {
        const int diff = speed(veh_id, 0) - Vehs[veh_id].moves_expended;
        if (diff >= 0 && (diff > 0x3e7 || diff != 0)) {
            return 1;
        }
    }
    return 0;
}

/*
Purpose: May the player select this vehicle?
// ORIGINAL: 0x005C1A20 ?veh_selectable@@YAHH@Z 0x005C1A20-0x005C1AAA
// size      138 bytes
// prototype int (__cdecl ?veh_selectable@@YAHH@Z)(int vehID)
// callers   11   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005C1C40
// HOMED from src/unrecovered/005c1a20.cpp, 2026-09-04.
// TRIED: nesting the veh_id < 0 guard as `if (veh_id >= 0) { ... }` with a
//        single fall-through return - same jl polarity either way, /O2
//        canonicalises both to the same CFG.
Return Value: 1 when the vehicle is on the map, ours, and not jailed.
Status: MISMATCH.
*/
int __cdecl veh_selectable(int veh_id) {
    if (veh_id < 0) {
        return 0;
    }
    if (Vehs[veh_id].y < 0 || Vehs[veh_id].y >= MapLatitudeBounds ||
        Vehs[veh_id].x < 0 || Vehs[veh_id].x >= MapLongitudeBounds) {
        return 0;
    }
    if (Vehs[veh_id].faction_id != (uint8_t)ConsoleGlobal.faction_id_) {
        return 0;
    }
    if ((Vehs[veh_id].flags & 0x40) != 0 && (Vehs[veh_id].flags & 0x400) != 0) {
        return 0;
    }
    return veh_jail(veh_id) == 0;
}

/*
Purpose: Has this vehicle any moves left?
// ORIGINAL: 0x005C1AB0 ?veh_unmoved@@YAHH@Z 0x005C1AB0-0x005C1B6F
// size      191 bytes
// prototype int (__cdecl ?veh_unmoved@@YAHH@Z)(int vehID)
// callers   1   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005C1540 0x005C1C40
// HOMED from src/unrecovered/005c1ab0.cpp, 2026-09-04.
Return Value: 1 when the vehicle cannot move again this turn.
Status: MISMATCH.
*/
int __cdecl veh_unmoved(int veh_id) {
    if (veh_id < 0 || Vehs[veh_id].y < 0 || Vehs[veh_id].y >= MapLatitudeBounds ||
        Vehs[veh_id].x < 0 || Vehs[veh_id].x >= MapLongitudeBounds ||
        Vehs[veh_id].faction_id != (uint8_t)ConsoleGlobal.faction_id_ ||
        ((Vehs[veh_id].flags & 0x40) != 0 && (Vehs[veh_id].flags & 0x400) != 0) ||
        veh_jail(veh_id) != 0) {
        return 0;
    }
    if ((Vehs[veh_id].state & 0x4000000) == 0 && Vehs[veh_id].order >= 4 &&
        Vehs[veh_id].order < 0x18) {
        return 1;
    }
    const int diff = speed(veh_id, 0) - Vehs[veh_id].moves_expended;
    if (diff < 0) {
        return 0;
    }
    if (diff > 0x3e7 || diff != 0) {
        return 1;
    }
    return 0;
}

// ===== veh.cpp, P2 wave 2: the prototype-design and battle families =====

// The faction prototype-design list. 0x0094B558, eight factions x 64 entries x
// 8 bytes - one entry is VehPrototype's ability_flags dword followed by its
// chassis/weapon/armor/reactor bytes, which is exactly the 8 bytes
// 0x0057DF30 copies in. clear_bunglist (0x0057DF00) fills a faction's 64 with
// -1 then 0xFF, which is why the comparison bytes sit at +4..+7 and the dword
// at +0.
struct ProtoBung {
    uint32_t ability_flags;
    uint8_t chassis_id;
    uint8_t weapon_id;
    uint8_t armor_id;
    uint8_t reactor_id;
};
static_assert(sizeof(ProtoBung) == 8, "Prototype history record stride");
static_assert(offsetof(ProtoBung, chassis_id) == 4,
              "Prototype history component offset");
// Eight faction histories, 64 records each. Initial loaded storage is zero.
// Native clear57DF00 uses base94B558+5 then stores at -5..+2; the former
// address alias started five bytes too early and corrupted the preceding data.
// RECOVERY_OBJECT: v2 0x0094B558 0x0094C558 _BungList 0x0057DF00
static ProtoBung BungList[8][64];

bool veh_bung_copy_chunk(size_t offset, unsigned char *packet, size_t count,
                         bool packet_to_owner) {
    const size_t extent = sizeof BungList;
    if (offset > extent || count > extent - offset) return false;
    if (count == 0) return true;
    unsigned char *owner = reinterpret_cast<unsigned char *>(&BungList);
    if (packet_to_owner) memcpy(owner + offset, packet, count);
    else memcpy(packet, owner + offset, count);
    return true;
}

// The battle-resolution messages. Each is the (label, icon) pair a
// NetMsgPrimary.pop takes: the label is the phrase id key, the icon the small
// portrait drawn beside the line.
char *const VehLabelMORALE = (char *)0x0068A0A8;      // "MORALE"
char *const VehIconBATWON_SM_PCX = (char *)0x0068A098;   // "batwon_sm.pcx"
char *const VehLabelMORALE2 = (char *)0x0068A0C0;     // "MORALE2"
char *const VehIconBATWON2_SM_PCX = (char *)0x0068A0B0;  // "batwon_sm.pcx"

int __cdecl battle_kill(int cur, int *attacker_out, int *defender_out,
                        int *odds_out, int killer_id, int credit_faction);

/*
Purpose: Hide a whole stack from the faction that spotted it.
// ORIGINAL: 0x005B5A70 ?unspot_stack@@YAXH@Z 0x005B5A70-0x005B5AC5 BYTE_EXACT
// size      85 bytes
// prototype void (__cdecl ?unspot_stack@@YAXH@Z)(int vehID)
// callers   2   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// HOMED from src/recovered/005b5a70.cpp, 2026-09-04. The archive's
// `g_0095284f[a1 * 0x34] = 0` is Vehs[id].visibility (+0x27) and its two short
// walks are Vehs[].prev_veh_id_stack (+0x32) and Vehs[].next_veh_id_stack
// (+0x30) - the same climb-and-descend spot_stack does.
Return Value: n/a
Status: BYTE_EXACT.
*/
void __cdecl unspot_stack(int veh_id) {
    if (veh_id >= 0) {
        short next = Vehs[veh_id].prev_veh_id_stack;
        while (next >= 0) {
            veh_id = next;
            next = Vehs[veh_id].prev_veh_id_stack;
        }
        for (; veh_id >= 0; veh_id = Vehs[veh_id].next_veh_id_stack) {
            Vehs[veh_id].visibility = 0;
        }
    }
}

/*
Purpose: Add the special sites visible from a tile's eight neighbours.
// ORIGINAL: 0x00579FF0 sub_579ff0 0x00579FF0-0x0057A042 BYTE_EXACT
// size      82 bytes
// prototype void (__cdecl sub_579ff0)(int xCoord, int yCoord)
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00579B70
// symbol    ?veh_site_scan@@YAXHH@Z
//           The image's catalogue name is an uninformative `sub_`; this tree
//           names what the body does. Renames update this fact, not the bytes.
// HOMED from src/recovered/00579ff0.cpp, 2026-09-04. The archive's raw
// `base + 0xc + offset * 4` is the map tile's bit_visible run: +0x0C is
// Map::bit2, so +0x0C + i * 4 for i in 1..7 is bit_visible[0..6], and 0x2000
// is the special-site bit. `11 ints` is the 0x2C tile stride spelled as a
// multiply.
Return Value: n/a
Status: BYTE_EXACT.
*/
void __cdecl veh_site_scan(int x, int y) {
    const int half = x >> 1;
    for (int i = 1; i < 8; i++) {
        const Map *tile = map_loc(x, y);
        if (tile->bit_visible[i - 1] & 0x2000) {
            add_site(i, 0, 2, x, y);
        }
    }
}
int __cdecl morale_veh_call(int veh_id, int check_drone_riot, int faction_id);
/*
Purpose: Bump a vehicle up a morale step, and say so.
// ORIGINAL: 0x005062B0 ?promote@@YAXH@Z 0x005062B0-0x0050648B
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
    const int proto_id = Vehs[veh_id].proto_id;

    if (Chassis[VehPrototypes[proto_id].chassis_id].missile != 0) {
        return;
    }

    if (!(VehPrototypes[proto_id].plan < 8 ||
          Armor[VehPrototypes[proto_id].armor_id].defense_rating > 1)) {
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
    if (Vehs[veh_id].faction_id != (uint8_t)ConsoleGlobal.faction_id_) {
        return;
    }
    if (!((1 << Vehs[veh_id].faction_id) & LockEnableMask)) {
        return;
    }
    if (morale_before == morale_veh(veh_id, 1, 0)) {
        return;
    }

    parse_says(1, VehPrototypes[proto_id].veh_name, -1, -1);

    if (proto_id < 0x40) {
        if (Weapon[VehPrototypes[proto_id].weapon_id].offense_rating < 0) {
            parse_say(0, (int)Morale[morale_veh(veh_id, 1, 0)].name, -1, -1);
            NetMsgPrimary.pop(VehLabelMORALE2, 0xffffec78, 0,
                             VehIconBATWON2_SM_PCX);
            return;
        }
    }

    parse_say(0, (int)Morale[morale_veh(veh_id, 1, 0)].name, -1, -1);
    NetMsgPrimary.pop(VehLabelMORALE, 0xffffec78, 0, VehIconBATWON_SM_PCX);
}

/*
Purpose: Resolve a stack's worth of chaff deaths after a battle.
// ORIGINAL: 0x005061B0 ?battle_kill_chaff@@YAXHPAHPAHPAHHH@Z 0x005061B0-0x005062A2
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
        const short proto_id = Vehs[cur].proto_id;
        short next_id = Vehs[cur].next_veh_id_stack;
        int pct;
        if (VehPrototypes[proto_id].plan == PLAN_ALIEN_ARTIFACT) {
            pct = 1;
        } else {
            int clamped = VehPrototypes[proto_id].reactor_id;
            if (clamped < 1) {
                clamped = 1;
            } else if (clamped > 0x64) {
                clamped = 0x64;
            }
            pct = clamped * 10;
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

/*
Purpose: Was this exact prototype design already recorded for this faction?
// ORIGINAL: 0x0057DFC0 ?is_bunged@@YA_NHHHHHH@Z 0x0057DFC0-0x0057E037
// size      119 bytes
// prototype bool (__cdecl ?is_bunged@@YA_NHHHHHH@Z)(int factionID, int chassisID, int weaponID, int armorID, int abilityFlags, int reactorID)
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// HOMED from src/unrecovered/0057dfc0.cpp, 2026-09-04.
Return Value: 1 when the faction's design run already holds the tuple.
Status: MISMATCH.
*/
bool __cdecl is_bunged(int faction_id, int chassis_id, int weapon_id,
                       int armor_id, int ability_flags, int reactor_id) {
    if ((1u << (faction_id & 0x1f)) & (uint8_t)LockEnableMask) {
        const ProtoBung *entry = &BungList[faction_id][0];
        for (int i = 0; i < 0x40; ++i) {
            if (entry->chassis_id == chassis_id && entry->weapon_id == weapon_id &&
                entry->armor_id == armor_id &&
                entry->ability_flags == (uint32_t)ability_flags &&
                entry->reactor_id == reactor_id) {
                return true;
            }
            ++entry;
        }
    }
    return false;
}

/*
Purpose: Empty one faction's prototype-design run.
// ORIGINAL: 0x0057DF00 ?clear_bunglist@@YAXH@Z 0x0057DF00-0x0057DF2F BYTE_EXACT
// size      47 bytes
// prototype void (__cdecl ?clear_bunglist@@YAXH@Z)(int factionID)
// callers   3   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// HOMED from src/recovered/0057df00.cpp, 2026-09-04 - the first of this
// file's archive-only claims to move into the build. The scaffold's
// `entry[-5]` / `entry[-1]` byte arithmetic expects a pointer to the weapon
// byte (+5), not the record base. Typed members express the actual stores.
Return Value: n/a
Historical byte claim remains a restoration obligation. The typed owner fixes
the former reference-target error; a masked-byte match did not detect it.
*/
void __cdecl clear_bunglist(int faction_id) {
    ProtoBung *entry = &BungList[faction_id][0];
    int count = 0x40;
    do {
        entry->ability_flags = 0xffffffffu;
        entry->chassis_id = 0xff;
        entry->weapon_id = 0xff;
        entry->armor_id = 0xff;
        entry->reactor_id = 0xff;
        ++entry;
    } while (--count);
}

/*
Purpose: Record a prototype design in the faction's run.
// ORIGINAL: 0x0057DF30 sub_57df30 0x0057DF30-0x0057DFB2
// size      130 bytes
// prototype void (__cdecl sub_57df30)(int factionID, int protoID)
// symbol    ?proto_bung_put@@YAXHH@Z
//           The image's catalogue name is an uninformative `sub_`; this tree
//           names what the body does. Renames update this fact, not the bytes.
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// HOMED from src/unrecovered/0057df30.cpp, 2026-09-04.
// TRIED: a separate pointer variable for the RMW counter slot ('mov' then a
//        stray 'lea' to materialise the address) - indexing the array
//        directly drops the divergence from #10 to #22. MISMATCH remains open.
Return Value: n/a
Status: MISMATCH.
*/
void __cdecl proto_bung_put(int faction_id, int proto_id) {
    int &cursor = PlayersData[faction_id].prototype_history_cursor;
    const int slot = cursor;
    cursor = (slot + 1) % 64;
    const VehPrototype &proto = VehPrototypes[proto_id];
    ProtoBung &entry = BungList[faction_id][slot];
    entry.ability_flags = proto.ability_flags;
    entry.chassis_id = proto.chassis_id;
    entry.weapon_id = proto.weapon_id;
    entry.armor_id = proto.armor_id;
    entry.reactor_id = proto.reactor_id;
}

/*
Purpose: Can this vehicle leave the stack on its own?
// ORIGINAL: 0x005B61A0 sub_5b61a0 0x005B61A0-0x005B6259
// size      185 bytes
// prototype int (__cdecl sub_5B61A0)(int vehID)
// symbol    ?veh_can_leave_stack@@YAHH@Z
//           The image's catalogue name is an uninformative `sub_`; this tree
//           names what the body does. Renames update this fact, not the bytes.
// callers   0   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005BF1F0
// HOMED from src/unrecovered/005b61a0.cpp, 2026-09-04.
Return Value: 0 no, 1 yes by ability or order, 2 yes - sea/peach tile.
Status: MISMATCH.
*/
int __cdecl veh_can_leave_stack(int veh_id) {
    if (has_abil(Vehs[veh_id].proto_id, 0x40) != 0) {
        return 1;
    }
    if (has_abil(Vehs[veh_id].proto_id, 4) != 0) {
        return 1;
    }
    if ((Vehs[veh_id].flags & 0x400) != 0) {
        return 1;
    }
    const Map *tile = map_loc(Vehs[veh_id].x, Vehs[veh_id].y);
    if (tile->visibility & 0x20) {
        if ((tile->climate & 0xE0) >= 0x40 && Vehs[veh_id].proto_id != 0x13) {
            return 2;
        }
    }
    return 0;
}

// ===== veh.cpp, P2 wave 2: the order verbs =====

// 0x00687FDC, the NetMsg label action_feature pops: the image's .data holds
// the phrase key the message table looks up.
char *const VehLabelFEATURE = (char *)0x00687FDC;  // "FEATURE"

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
// carries. `ClimatePopupTitle` is the title the image reads at 0x00691B0C and
// lives in game.h for exactly that reason.
// symbol    ?popp@@YAHPBD0P6AHXZ@Z
//           The image spells its two const char * parameters out; VC6
//           12.00.8168 back-references the repeat to PBD0 for the identical
//           source. The archive's `char *` return was also wrong - the first H
//           in YAHPBD... is the return type, so this returns int.
Return Value: whatever the five-argument popp returns.
Status: BYTE_EXACT.
*/
int __cdecl popp(const char *label, const char *body, int(__cdecl *callback)()) {
    return popp((char *)*ClimatePopupTitle, label, 0, body, callback);
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

// The eight per-direction map deltas battle_fight adds to a vehicle's x and y
// before wrapping the result into the map. Which pair applies is chosen by the
// caller's triad/mode flag - the two Alt runs are its alternate - and the x
// result is wrapped by MapLongitudeBounds exactly as an x coordinate is, which
// is what makes them coordinate deltas rather than costs. All four are
// file-backed .data, not .bss: the eight ints of each are the compass deltas
// (X runs +1,+2,+1,0,-1,-2,-1,0; the Alt runs are the same wheel turned one
// step), which is why they are initialised here rather than left to a loader.
// The ordinary tables also contain the stationary centre at index eight.
// Eight directions followed by the centre, sampled by scoot setup's nine
// iterations. Both complete initial runs are present in original .rdata.
// RECOVERY_OBJECT: v2 0x0066EF50 0x0066EF74 ?VehMoveDeltaX@@3PAHA 0x004BFE00
int VehMoveDeltaX[9] = {1, 2, 1, 0, -1, -2, -1, 0, 0};
// RECOVERY_OBJECT: v2 0x0066EF74 0x0066EF98 ?VehMoveDeltaY@@3PAHA 0x004BFE00
int VehMoveDeltaY[9] = {-1, 0, 1, 2, 1, 0, -1, -2, 0};
int VehMoveDeltaXAlt[8] = {0, 1, 2, 1, 0, -1, -2, -1};     // 0x0066EFBC
int VehMoveDeltaYAlt[8] = {0, -1, 0, 1, 2, 1, 0, -1};      // 0x0066F440

/*
Purpose: Report the feature at a site, multiplayer only.
// ORIGINAL: 0x004CD060 ?action_feature@@YAXXZ 0x004CD060-0x004CD082 BYTE_EXACT
// size      34 bytes
// prototype void (__cdecl ?action_feature@@YAXXZ)()
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0047A890
// HOMED from src/recovered/004cd060.cpp, 2026-09-04 - its archive-only
// BYTE_EXACT claim re-earned in the TU that owns the rest of the action
// family. The scaffold's `g_0093f660` is IsMultiplayerNet and
// `g_00805338` is netmsg.h's NetMsgPrimary.
Return Value: n/a
Status: BYTE_EXACT.
*/
void __cdecl action_feature() {
    if (IsMultiplayerNet) {
        NetMsgPrimary.pop(VehLabelFEATURE, 0x1388, 0, 0);
    }
}

/*
Purpose: Clear a vehicle's move state at the end of its stack walk.
// ORIGINAL: 0x005B5E10 sub_5b5e10 0x005B5E10-0x005B5EA0 BYTE_EXACT
// size      144 bytes
// prototype void (__cdecl sub_5b5e10)(int vehID)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// symbol    ?veh_move_end@@YAXH@Z
//           The image's catalogue name is an uninformative `sub_`; this tree
//           names what the body does. Renames update this fact, not the bytes.
// HOMED from src/unrecovered/005b5e10.cpp, 2026-09-04. Its `g_95282c` byte
// view is Vehs[]: +0x04 state, +0x0D nothing in the tree's Veh (see the note
// below), +0x0E faction_id, +0x0F year_end_lurking, +0x11 order.
// BUG IN THE ORIGINAL: the two bytes the body tests against each other,
// 0x95282c + veh_id * 0x34 + 0xD and + 0xE, are `unk_1` and `faction_id` in
// the tree's Veh - the low half of `flags` and the state dword's high byte, so
// `+ 0xD` is one byte *below* the field the comparison reads. The arithmetic
// is reproduced exactly as shipped; a reader who "fixes" the offset changes
// the program.
Return Value: n/a
Status: BYTE_EXACT.
*/
void __cdecl veh_move_end(int veh_id) {
    const int offset = veh_id * 0x34;
    unsigned int *flags = reinterpret_cast<unsigned int *>(
        reinterpret_cast<uint8_t *>(Vehs) + offset + 4);
    uint8_t *byte_d = reinterpret_cast<uint8_t *>(Vehs) + offset + 0xD;
    uint8_t *byte_e = reinterpret_cast<uint8_t *>(Vehs) + offset + 0xE;
    uint8_t *byte_f = reinterpret_cast<uint8_t *>(Vehs) + offset + 0xF;
    if ((*flags & 0x2000200) == 0x2000200 && *byte_f < *byte_e) {
        *flags &= 0xfdfffdff;
    }
    if ((*flags & 0x1000200) == 0x1000200) {
        *flags &= 0xfefffdff;
    }
    *flags &= 0xffffbfff;
    if (*byte_d == 0x18) {
        *byte_d = 0;
    }
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
Purpose: Set a vehicle patrolling a waypoint loop.
// ORIGINAL: 0x004CDA30 ?action_patrol@@YAXHHH@Z 0x004CDA30-0x004CDAEA
// size      186 bytes
// prototype void (__cdecl ?action_patrol@@YAXHHH@Z)(int vehID, int xCoord, int yCoord)
// callers   2   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004CD6A0
// HOMED from src/unrecovered/004cda30.cpp, 2026-09-04. Its eight raw
// `arr_9528xx` casts are Vehs[] members: +0x11 order, +0x12 waypoint_count,
// +0x13 patrol_current_point, +0x14 waypoint_x[0], +0x1C waypoint_y[0],
// +0x00 x, +0x02 y, +0x04 state.
// TRIED: fixed const-pointer globals for the two indexed word arrays (the
//        address itself scales, so extern-style raw casts were used instead);
//        reached MNEMONIC_ONLY, 188 vs 186 bytes.
Return Value: n/a
Status: MISMATCH.
*/
void __cdecl action_patrol(int veh_id, int x, int y) {
    if (valid_patrol(veh_id, x, y) != 0) {
        Vehs[veh_id].order = ORDER_MOVE_TO;
        Vehs[veh_id].waypoint_x[Vehs[veh_id].waypoint_count] = (int16_t)x;
        Vehs[veh_id].waypoint_y[Vehs[veh_id].waypoint_count] = (int16_t)y;
        const uint8_t count = Vehs[veh_id].waypoint_count;
        Vehs[veh_id].waypoint_count = count + 1;
        Vehs[veh_id].waypoint_x[count + 1] = Vehs[veh_id].x;
        Vehs[veh_id].waypoint_y[Vehs[veh_id].waypoint_count] = Vehs[veh_id].y;
        Vehs[veh_id].state |= 0x1000200;
        Vehs[veh_id].patrol_current_point = 0;
    }
}

/*
Purpose: Gate a vehicle into a base, repainting both tiles.
// ORIGINAL: 0x004CF380 ?action_gate@@YAXHH@Z 0x004CF380-0x004CF477
// size      247 bytes
// prototype void (__cdecl ?action_gate@@YAXHH@Z)(int vehID, int baseID)
// callers   3   call targets   5
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0046AF40 0x004E3A50 0x005108A0 0x005BFFA0 0x005C0080
// HOMED from src/unrecovered/004cf380.cpp, 2026-09-04. Its `Bases[n] + off`
// casts are Base members: +0x00 x, +0x02 y, +0x30 state (bit 0x20000000 is
// the "gate/open" event the UI reads).
// TRIED: reproducing veh_lift's post-call EAX reuse - the call site pushes
//        EAX straight into veh_drop's first argument with no reload, so EAX
//        must be veh_lift's return value; a local function-pointer type got
//        86.6% mnemonic similarity and redeclaring veh_lift itself is C2556.
//        veh_lift already returns int here, so the plain spelling is used.
// TRIED: frameless /O2 body where the image has `frame` - a spill-count
//        difference, not a control-flow one.
Return Value: n/a
Status: MISMATCH.
*/
void __cdecl action_gate(int veh_id, int base_id) {
    const short veh_x = Vehs[veh_id].x;
    const short veh_y = Vehs[veh_id].y;
    const int gate_base = base_at(veh_x, veh_y);
    if (gate_base >= 0) {
        Bases[gate_base].state |= 0x20000000;
    }
    Bases[base_id].state |= 0x20000000;

    const int lifted = veh_lift(veh_id);
    veh_drop(lifted, Bases[base_id].x, Bases[base_id].y);

    if (gate_base >= 0) {
        draw_tile(Bases[gate_base].x, Bases[gate_base].y, 2);
    }
    draw_tile(Bases[base_id].x, Bases[base_id].y, 2);

    if (veh_id == ConsoleGlobal.selected_vehicle_) {
        ConsoleGlobal.focus(veh_x, veh_y, Vehs[veh_id].faction_id);
    }
}

/*
Purpose: Add a direction's tile offset to a vehicle's coordinates.
// ORIGINAL: 0x00506A60 ?battle_fight@@YAHHHHHPAH@Z 0x00506A60-0x00506AEB
// size      139 bytes
// prototype int (__cdecl ?battle_fight@@YAHHHHHPAH@Z)(int vehID, int dir, int mode, int allow_interceptor, int *odds_out)
// callers   6   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00506AF0
// HOMED from src/unrecovered/00506a60.cpp, 2026-09-04. Its `g_00952828`
// short views are Vehs[].x (+0x00) and Vehs[].y (+0x02).
// `mode` is named from what it does here and in the seven-parameter body:
// a 0/1 flag that swaps in the Alt delta run. `allow_interceptor` and
// `odds_out` are named from the seven-parameter body - the first gates its
// call to interceptor (0x00506650), the second is the null-tested pointer it
// stores the odds word through at 0x005070A2.
Return Value: whatever the seven-parameter battle_fight returns.
Status: MISMATCH.
*/

// The seven-parameter overload, 0x00506AF0 - 19,875 bytes, the largest body
// in the veh family, still in src/unrecovered/00506af0.cpp. C++ overloading
// gives the pair the two distinct symbols the image carries; the definition
// is its own batch and lands beside this one.
int __cdecl battle_fight(int veh_id, int dir, int x, int y, int src_mode,
                         int allow_interceptor, int *odds_out);

int __cdecl battle_fight(int veh_id, int dir, int mode, int allow_interceptor,
                         int *odds_out) {
    int x;
    if (mode != 0) {
        x = VehMoveDeltaXAlt[dir];
    } else {
        x = VehMoveDeltaX[dir];
    }
    int val_x = Vehs[veh_id].x + x;

    if (!(MapIsFlat & 1)) {
        const int mod = MapLongitudeBounds;
        if (val_x < 0) {
            val_x += mod;
        } else if (val_x >= mod) {
            val_x -= mod;
        }
    }

    const int y = mode != 0 ? VehMoveDeltaYAlt[dir] : VehMoveDeltaY[dir];
    const int val_y = Vehs[veh_id].y + y;

    return battle_fight(veh_id, dir, val_x, val_y, mode, allow_interceptor,
                        odds_out);
}


/*
Purpose: Grant a prototype an ability, if the faction has researched it.
// ORIGINAL: 0x00581190 ?add_abil@@YAXPAHHH@Z 0x00581190-0x00581259
// size      201 bytes
// prototype void (__cdecl ?add_abil@@YAXPAHHH@Z)(int *ability_flags, int ability, int faction_id)
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005B9F20
// HOMED from src/unrecovered/00581190.cpp, 2026-09-04. Its `g_009ab550`
// short table is Ability[].preq_tech - 0x009AB550 is 0x18 into RulesAbility
// (0x009AB538, 0x1C stride) and the index step is 14 shorts = 0x1C. Its
// `g_00946f5c` / `g_00946f80` pair is Players[].faction_bonus_count
// (0x50C) and Players[].faction_bonus_val1 (0x530) - 0x00946A50, 0x59C
// stride - and `rec[-8]` is therefore faction_bonus_id, 0x20 bytes back.
// 0x0F is the "free ability" bonus id: a faction already carrying one gets
// no second.
// LEVER: has_tech_call, not has_tech - the image CALLS 0x005B9F20 here and
//        technology.h's `MEASURED inline` would fold the whole preq walk in.
// TRIED: 2/72, first divergence at instruction 2. The image needs NO local
//        slot: `bit` lands in the dead `ability` parameter slot [ebp+0xc] and
//        `i` in the dead `faction_id` slot [ebp+0x10]. This tree takes ONE
//        `push ecx` for `bit` and puts `i` in the ability slot, and the extra
//        slot costs a matching `mov esp, ebp`. Scored the do-while latch
//        hoisted from the archive and an `ability = bit` aliasing spelling -
//        both 2/72, the same slot assignment. The two dead slots are used and
//        dead identically on both sides; which dead parameter each counter
//        inherits is the allocator's choice and nothing in the source names it.
Return Value: n/a
Status: MISMATCH.
*/
void __cdecl add_abil(int *ability_flags, int ability, int faction_id) {
    if (*ability_flags != 0) {
        return;
    }

    int bit = 0;
    while ((ability & 1) == 0) {
        ability >>= 1;
        ++bit;
    }

    if (has_tech_call(Ability[bit].preq_tech, faction_id) == 0) {
        return;
    }

    // A faction bonus of id 0x0F grants one ability free; refuse a second.
    for (int i = 0; i < Players[faction_id].faction_bonus_count; ++i) {
        if (Players[faction_id].faction_bonus_id[i] == 0x0F
            && has_tech_call(Ability[Players[faction_id].faction_bonus_val1[i]].preq_tech,
                             faction_id) != 0
            && bit == Players[faction_id].faction_bonus_val1[i]) {
            return;
        }
    }

    *ability_flags |= 1 << bit;
}



// ===== veh.cpp, P2 wave 3: the action family =====
//
// The order verbs the unit's `order` byte dispatches to. All of them start the
// same way: under multiplayer, and while this client is not the one playing
// the turn out, they publish the address of their own veh_id parameter into
// VehSelectionFollow so the daemon's lock bookkeeping names the unit they are
// about to move - which is why every one of them reloads veh_id off the stack
// after its first call, and why the source spells the publish as `&veh_id`.

/*
Purpose: Move a unit along its path, one ORDER_MOVE / ORDER_MOVE_TO leg.
// ORIGINAL: 0x004CB310 ?action_go_to@@YAXH@Z 0x004CB310-0x004CB57F
// size      623 bytes
// prototype void (__cdecl ?action_go_to@@YAXH@Z)(int veh_id)
// callers   2   call targets   8
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005310F0 0x00531300 0x00531480 0x00531780 0x00593220 0x005947C0 0x0059BC10 0x005C1D20
// HOMED from src/recovered/units/004cb310.cpp, 2026-09-04. Its `g_00952839`
// byte is Vehs[].order (+0x11), `g_0095283c` the waypoint_x[] run (+0x14),
// `g_00952836` faction_id (+0x0E) and `g_009ab88c` VehPrototypes[].chassis_id
// (+0x24); `g_0094a37b` is Chassis[].cargo (+0x4B, 0x90 stride) - the unit
// has to be a transport for the arrival skip to apply.
// symbol    the free order_veh (0x005947C0) is the host-side move; the
//           member NetDaemon::order_veh (0x00531780) is the client's lock
//           request. Both appear here, spelled by their own names.
// TRIED: 2/175, first divergence at instruction 2. The image opens
//        `push esi / mov esi,[0x93f660] / push edi / mov edi,[0x93a938]` - it
//        loads BOTH net globals into callee-saved registers before testing
//        either, then tests `esi` and `edi`. Spelled as two `const int` locals
//        ahead of the `&&`, this tree still sinks the second load behind the
//        first test (`mov eax,[0x93f660] / test / je / mov eax,[0x93a938]`), so
//        it reads the transport flag at all three guard sites instead of two
//        loads plus the one reload after Paths::move. The globals-read-at-each-
//        site spelling scores the same. Reverting to the plain spelling.
Return Value: n/a
Status: MISMATCH.
*/
void __cdecl action_go_to(int veh_id) {
    const int net = IsMultiplayerNet;
    const int turn_active = ConsoleControlTurnActive;
    if (net != 0 && turn_active == 0) {
        NetDaemonNet.set_vehicle_selection_follow(&veh_id);
    }

    int move_result;
    {
        const uint8_t order = Vehs[veh_id].order;
        if (order == ORDER_MOVE) {
            move_result = Vehs[veh_id].waypoint_x[0];
        } else {
            move_result = Paths.move(
                veh_id, (order & 0x40) != 0 ? -1 : Vehs[veh_id].faction_id);
        }
    }

    if (move_result < 0) {
        Vehs[veh_id].order = ORDER_NONE;
        if (IsMultiplayerNet != 0 && ConsoleControlTurnActive == 0) {
            synch_veh(veh_id);
            NetDaemonNet.await_synch();
            NetDaemonNet.unlock_veh();
        }
        return;
    }

    if (IsMultiplayerNet != 0 && ConsoleControlTurnActive == 0) {
        if (NetDaemonNet.order_veh(veh_id, move_result, 1) != 0) {
            NetDaemonNet.await_exec(1);
        }
        return;
    }

    if (order_veh(veh_id, move_result, 3) == 0) {
        return;
    }

    const uint8_t order = Vehs[veh_id].order;
    if (order == ORDER_MOVE) {
        if (Vehs[veh_id].x == Vehs[veh_id].waypoint_x[1]
            && Vehs[veh_id].y == Vehs[veh_id].waypoint_y[1]) {
            Vehs[veh_id].order = ORDER_NONE;
            return;
        }
    } else if (order == ORDER_MOVE_TO) {
        if (Vehs[veh_id].x == Vehs[veh_id].waypoint_x[0]
            && Vehs[veh_id].y == Vehs[veh_id].waypoint_y[0]) {
            Vehs[veh_id].order = ORDER_NONE;

            if ((Vehs[veh_id].state & 0x2000200) == 0x2000200
                && Vehs[veh_id].patrol_current_point < Vehs[veh_id].waypoint_count) {
                for (int i = 0; i < Vehs[veh_id].waypoint_count; ++i) {
                    Vehs[veh_id].waypoint_x[i] = Vehs[veh_id].waypoint_x[i + 1];
                    Vehs[veh_id].waypoint_y[i] = Vehs[veh_id].waypoint_y[i + 1];
                }
                Vehs[veh_id].order = ORDER_MOVE_TO;
                Vehs[veh_id].waypoint_count -= 1;
                if (Vehs[veh_id].waypoint_count == 0) {
                    Vehs[veh_id].state &= 0xfdfefdff;
                }
            }

            if (!(LockEnableMask & (1 << Vehs[veh_id].faction_id))
                && Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].cargo != 0) {
                Map *tile = map_tiles()
                    + ((Vehs[veh_id].x >> 1) + Vehs[veh_id].y * MapLongitude);
                if (tile->bit & BIT_BASE_IN_TILE) {
                    const int rockiness = tile->val2 & 0xf;
                    if (rockiness >= 0 && rockiness < 8) {
                        veh_skip(veh_id);
                        return;
                    }
                }
            }
        }
    }
}

// 0x0094977C is Rules.tech_build_roads_fungus (Rules + 0x44): the tech that
// lets a unit road a fungus tile instead of clearing it first.
// 0x006919C0 is Terraforming[TERRA_REMOVE_FUNGUS].preq_tech and 0x00691940
// Terraforming[TERRA_MAGTUBE].preq_tech (base 0x00691878, 0x20 stride).
// 0x00687E20 / 0x00687E2C are the two phrase keys the arrival popup is titled
// with, "ROAD" and "TUBE" - which one depends on the order the unit was given.
char *const VehLabelROAD = (char *)0x00687E20;  // "ROAD"
char *const VehLabelTUBE = (char *)0x00687E2C;  // "TUBE"

/*
Purpose: Walk a unit toward its goal, roading or tubing the tiles it crosses.
// ORIGINAL: 0x004CB580 ?action_road_to@@YAXH@Z 0x004CB580-0x004CBA98
// size      1304 bytes
// prototype void (__cdecl ?action_road_to@@YAXH@Z)(int veh_id)
// callers   2   call targets   8
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005310F0 0x00531300 0x00531480 0x00531780 0x00592250 0x005947C0 0x0059A530 0x005B9F20 0x00592EE0 0x00593220 0x004C9B00 0x0047A890 0x00625EC0
// HOMED from src/recovered/units/004cb580.cpp, 2026-09-04. Its `g_00952828`
// short views are Vehs[].x / .y / .waypoint_x[] / .waypoint_y[], its byte
// views Vehs[].faction_id (+0x0E) and Vehs[].order (+0x11) and its dword view
// Vehs[].state (+0x04).
//
// BUG IN THE ORIGINAL (and left as written): the archive this came from put
// the "is there a base on the fungus tile and it is rockiness <= 7" case on
// the SKIP side of the fungus clearing, which would drop the tile walk
// entirely. The image jumps from that test into the tile walk (0x004CB6C0
// `test eax,eax / jge 0x004CB7DF`, and eax is 0..7 after `and eax,0xf`, so
// the branch is always taken). The guard is dead on its <= 7 side - the
// compiler emitted `test eax,eax` after a mask that cannot set the sign flag -
// and the source below spells the fall-through the image actually takes.
Return Value: n/a
Status: MISMATCH.
*/
void __cdecl action_road_to(int veh_id) {
    if (IsMultiplayerNet != 0 && ConsoleControlTurnActive == 0) {
        NetDaemonNet.set_vehicle_selection_follow(&veh_id);
    }

    const unsigned owner = Vehs[veh_id].faction_id;
    const int goal_x = Vehs[veh_id].waypoint_x[0];
    const int y = Vehs[veh_id].y;
    const int goal_y = Vehs[veh_id].waypoint_y[0];
    const int x = Vehs[veh_id].x;

    Map *tile = map_tiles()
        + ((x >> 1) + y * MapLongitude);
    const uint8_t altitude = tile->climate & 0xE0;
    int net_flag;

    if (altitude < ALT_BIT_SHORE_LINE || goal_y < 0
        || goal_y >= MapLatitudeBounds || goal_x < 0
        || goal_x >= MapLongitudeBounds) {
        Vehs[veh_id].order = ORDER_NONE;
        net_flag = IsMultiplayerNet;
    } else {
        if ((tile->bit & BIT_FUNGUS) != 0 && altitude >= ALT_BIT_SHORE_LINE
            && !has_tech_call(Rules.tech_build_roads_fungus, owner)
            && has_tech_call(Terraforming[TERRA_REMOVE_FUNGUS].preq_tech, owner)) {
            if (!(tile->bit & BIT_BASE_IN_TILE) || (tile->val2 & 0xf) > 7) {
                if (IsMultiplayerNet == 0
                    || ConsoleControlTurnActive != 0) {
                    const uint8_t saved_order = Vehs[veh_id].order;
                    action_terraform(veh_id, ORDER_REMOVE_FUNGUS, 1);
                    Vehs[veh_id].order = saved_order;
                    return;
                }
                const int saved_order = Vehs[veh_id].order;
                message_data(0x2405, 0, veh_id, ORDER_REMOVE_FUNGUS,
                             saved_order, 0);
                NetDaemonNet.await_exec(1);
                return;
            }
        }

        if ((tile->bit & BIT_FUNGUS) == 0 || altitude < ALT_BIT_SHORE_LINE
            || has_tech_call(Rules.tech_build_roads_fungus, owner)) {
            if (!(tile->bit & BIT_BASE_IN_TILE) || (tile->val2 & 0xf) > 7) {
                if (!(tile->bit & BIT_ROAD)) {
                    if (IsMultiplayerNet == 0
                        || ConsoleControlTurnActive != 0) {
                        const uint8_t saved_order = Vehs[veh_id].order;
                        action_terraform(veh_id, ORDER_ROAD, 1);
                        Vehs[veh_id].order = saved_order;
                        return;
                    }
                    const int saved_order = Vehs[veh_id].order;
                    message_data(0x2405, 0, veh_id, ORDER_ROAD, saved_order, 0);
                    NetDaemonNet.await_exec(1);
                    return;
                }
                if (Vehs[veh_id].order == ORDER_MAGTUBE_TO
                    && !(tile->bit & BIT_MAGTUBE)
                    && has_tech_call(Terraforming[TERRA_MAGTUBE].preq_tech, owner)) {
                    if (IsMultiplayerNet == 0
                        || ConsoleControlTurnActive != 0) {
                        action_terraform(veh_id, ORDER_ROAD, 1);
                        Vehs[veh_id].order = ORDER_MAGTUBE_TO;
                        return;
                    }
                    message_data(0x2405, 0, veh_id, ORDER_ROAD,
                                 ORDER_MAGTUBE_TO, 0);
                    NetDaemonNet.await_exec(1);
                    return;
                }
            }
        }

        if (x == goal_x && y == goal_y) {
            const uint8_t order = Vehs[veh_id].order;
            Vehs[veh_id].order = ORDER_NONE;
            if (IsMultiplayerNet != 0
                && ConsoleControlTurnActive == 0) {
                synch_veh(veh_id);
                NetDaemonNet.await_synch();
                NetDaemonNet.unlock_veh();
            }
            if (Vehs[veh_id].state & VSTATE_UNK_200) {
                return;
            }
            if (owner != (unsigned)ConsoleGlobal.faction_id_) {
                return;
            }
            StringTemp[0] = 0;
            say_loc(StringTemp, goal_x, goal_y, 0, 2, 0);
            parse_says(0, StringTemp, -1, -1);
            if (order == ORDER_ROAD_TO) {
                NetMsgPrimary.pop(VehLabelROAD, 5000, 0, 0);
            } else {
                NetMsgPrimary.pop(VehLabelTUBE, 5000, 0, 0);
            }
            return;
        }

        const int min_speed = has_tech_call(Rules.tech_build_roads_fungus, owner)
            ? 0 : 0x10;
        const int found = Paths.find(x, y, goal_x, goal_y, 0, -1, min_speed, -1);
        net_flag = IsMultiplayerNet;
        if (found >= 0 && found < 8) {
            if (IsMultiplayerNet != 0
                && ConsoleControlTurnActive == 0) {
                if (NetDaemonNet.order_veh(veh_id, found, 1) == 0) {
                    return;
                }
                NetDaemonNet.await_exec(1);
                return;
            }
            order_veh(veh_id, found, 3);
            return;
        }
        Vehs[veh_id].order = ORDER_NONE;
    }

    if (net_flag != 0 && ConsoleControlTurnActive == 0) {
        synch_veh(veh_id);
        NetDaemonNet.await_synch();
        NetDaemonNet.unlock_veh();
    }
}

// ===== bindings the terraform verb needs =====

// 0x009A64B4, two dwords: the TERRA_* bits the local player has already been
// shown a first-time popup for, [0] land and [1] sea - indexed by the same
// not-ocean test that picks Terraforming[]'s preq_tech from its _sea twin.
uint32_t TerraformAnnouncedMask[2];  // 0x009A64B4

// 0x009B22DC and 0x009B22E0 close the run map.cpp binds as
// MapBaseIdClosestSubmergedVeh[8] (0x009B22BC) and hold two ids the terraform
// epilogue reads: the faction whose territory the tile sits in, and the base
// that lost land to the raise/lower. Both are -1 when nobody is involved.
int TerraformTileFaction;    // 0x009B22DC
int TerraformDrownedBaseId;  // 0x009B22E0

// 0x0090DB84. A latch: the first terraform a unit completes with no order
// queued raises it and plays the unit-improvement sting, and nothing here
// clears it.
uint32_t VehTerraformStingLatch;  // 0x0090DB84

// Two slots of BaseSecretProject (0x009A6514, 64 ints) that this body reads as
// BASE IDS: >= 0 tests "built", and the value indexes Bases[]. All are stored
// as SP_Unbuilt/-1 until the project lands. Bound by address the way
// spying_recovery.h binds interior offsets of PlayersData, because the struct
// spells the 64 ints as project names and these are the three the image reads
// as a base.
int *const SecretProjectBoreholeBase = (int *)0x009A651C;      // +0x08
int *const SecretProjectXenoempathyBase = (int *)0x009A6534;   // +0x20
int *const SecretProjectAirdropBase = (int *)0x009A6580;       // +0x6C

// The phrase keys the terraform popups and the tutorial map call are titled
// with. All are file-backed .data, all read only here.
char *const VehLabelBOREHOLEMINE = (char *)0x00687B80;   // "BOREHOLEMINE"
char *const VehLabelHARVESTFOREST = (char *)0x00687B90;  // "HARVESTFOREST"
char *const VehLabelTERRAMINE = (char *)0x00687BA0;      // "TERRAMINE"
char *const VehLabelTERRAYOURS = (char *)0x00687BAC;     // "TERRAYOURS"
char *const VehLabelDROWNMINE = (char *)0x00687BB8;      // "DROWNMINE"
char *const VehLabelDROWNYOURS = (char *)0x00687BC4;     // "DROWNYOURS"
char *const VehLabelMADEFARM = (char *)0x00687BD0;       // "MADEFARM"
char *const VehLabelMADESOIL = (char *)0x00687BDC;       // "MADESOIL"
char *const VehLabelMADEMINE = (char *)0x00687BE8;       // "MADEMINE"
char *const VehLabelMADESOLAR = (char *)0x00687BF4;      // "MADESOLAR"
char *const VehLabelMADEFOREST = (char *)0x00687C00;     // "MADEFOREST"
char *const VehLabelMADEROAD = (char *)0x00687C0C;       // "MADEROAD"
char *const VehLabelMADETUBE = (char *)0x00687C18;       // "MADETUBE"
char *const VehLabelMADESENSOR = (char *)0x00687C24;     // "MADESENSOR"
char *const VehLabelMADEAQUIFER = (char *)0x00687C30;    // "MADEAQUIFER"
char *const VehLabelMADECONDENSER = (char *)0x00687C3C;  // "MADECONDENSER"
char *const VehLabelMADEBOREHOLE = (char *)0x00687C4C;   // "MADEBOREHOLE"
char *const VehLabelMADESEAFARM = (char *)0x00687C5C;    // "MADESEAFARM"
char *const VehLabelMADESEAMINE = (char *)0x00687C68;    // "MADESEAMINE"
char *const VehLabelMADESEASOLAR = (char *)0x00687C74;   // "MADESEASOLAR"

// The four facility sprites the first-time tutorial popup draws beside its
// text. sprite.cpp owns the objects: the farm and soil enricher runs are five
// 0x2C-stride Sprites and the image hands the tutorial the COMPLETED tile,
// which is element [3] of each run (0x00791FC8 / 0x0079A710 + 3 * 0x2C); the
// mine and solar collector are single Sprites of their own.
extern Sprite g_TER1_FARM_SPRITES[5];             // 0x00791FC8, sprite.cpp
extern Sprite g_TER1_SOIL_ENRICHER_SPRITES[5];    // 0x0079A710, sprite.cpp
extern Sprite g_TER1_MINE_SPRITE;                 // 0x00776A50, sprite.cpp
extern Sprite g_TER1_SOLAR_COLLECTOR_SPRITE;      // 0x007991F8, sprite.cpp
static Sprite *const VehSpriteFARM = &g_TER1_FARM_SPRITES[3];
static Sprite *const VehSpriteSOIL = &g_TER1_SOIL_ENRICHER_SPRITES[3];
static Sprite *const VehSpriteMINE = &g_TER1_MINE_SPRITE;
static Sprite *const VehSpriteSOLAR = &g_TER1_SOLAR_COLLECTOR_SPRITE;

// The remaining unrecovered BaseWin repaint hook:
// recompute the one vehicle's defenders and garrison after its tile changed.
typedef void (OriginalObject::*func_base_win_check_veh)(int veh_id);
func_base_win_check_veh BaseWinOriginalCheckVeh =
    original_method<func_base_win_check_veh>(0x0041A8B0);

// TutWin::tut_map (0x004BE650) is not recovered and tutwin.h declares no such
// member; the terraform epilogue is the tree's first caller. This still uses
// a member-pointer call into the original, with receiver TutWinGlobal.
typedef void (OriginalObject::*func_tut_win_tut_map)(const char *msg, int x,
                                                    int y, int noun,
                                                    void *sprite,
                                                    int tut_win_arg6,
                                                    int arrow_x, int arrow_y);
func_tut_win_tut_map TutWinOriginalTutMap =
    original_method<func_tut_win_tut_map>(0x004BE650);

/*
Purpose: Perform one terraforming action on a unit's tile, over its stack.
// ORIGINAL: 0x004C9B00 ?action_terraform@@YAHHHH@Z 0x004C9B00-0x004CA76B
// size      3179 bytes
// prototype int (__cdecl ?action_terraform@@YAHHHH@Z)(int veh_id, int action, int do_it)
// callers   8   call targets   4
// kind      game
// flags     frame;sp_ready;purged_ok
// HOMED from src/unrecovered/004c9b00.cpp, 2026-09-04. `action` is the TERRA_*
// index (the order byte stores action + 4); `do_it` distinguishes the query
// pass, which only totals the stack's progress, from the pass that spends it.
// Its `g_0095284d` byte is Vehs[].terraforming_turns (+0x25), `g_00952850`
// Vehs[].moves_expended (+0x28), `g_00952856` Vehs[].home_base_id (+0x2E) and
// `g_0095285a`/`g_00952858` prev/next_veh_id_stack (+0x32/+0x30). Its
// `g_0069188x` triple is Terraforming[].bit / .bit_incompatible / .rate
// (0x10/0x14/0x18, 0x20 stride off 0x00691878).
//
// BUG IN THE ORIGINAL: the drowned-base epilogue gates on
// MapBaseIdClosestSubmergedVeh[LocalFaction] (0x004CA466) but then reads
// MapBaseIdClosestSubmergedVeh[faction] (0x004CA47E). For the local player the
// two are the same slot and the slip is invisible; for any other faction the
// guard answers a different slot than the one used. Transcribed as written -
// both sides are the image's.
Return Value: 1 when the action ran, 0 when it only reported progress.
Status: MISMATCH.
*/
int __cdecl action_terraform(int veh_id, int action, int do_it) {
    const int veh_count_before = VehCurrentCount;
    const int y = Vehs[veh_id].y;
    const int x = Vehs[veh_id].x;
    const uint8_t faction = Vehs[veh_id].faction_id;
    Map *tile = map_tiles()
        + ((x >> 1) + y * MapLongitude);
    const int not_ocean = (tile->climate & 0xE0) < ALT_BIT_SHORE_LINE;

    if (action == TERRA_ROAD) {
        if (tile->bit & BIT_ROAD) {
            if (has_tech_call(not_ocean != 0
                                  ? Terraforming[TERRA_MAGTUBE].preq_tech_sea
                                  : Terraforming[TERRA_MAGTUBE].preq_tech,
                              faction)) {
                action = TERRA_MAGTUBE;
            }
        }
    } else if (action == TERRA_FARM && (tile->bit & BIT_FARM) != 0
               && (tile->climate & 0xE0) > 0x5F) {
        if (has_tech_call(not_ocean != 0
                              ? Terraforming[TERRA_FARM].preq_tech_sea
                              : Terraforming[TERRA_FARM].preq_tech,
                          faction)) {
            action = TERRA_SOIL_ENR;
        }
    }

    if (do_it != 0) {
        Vehs[veh_id].order = (uint8_t)(action + 4);
    }

    int base_time = Terraforming[action].rate;
    switch (action) {
    case TERRA_MINE: {
        const uint32_t bonus = ((uint32_t)(tile->val3 >> 6) * base_time) / 2;
        base_time = base_time + bonus;
        break;
    }
    case TERRA_ROAD:
    case TERRA_MAGTUBE: {
        const uint32_t flags = tile->bit;
        base_time = Terraforming[(flags & BIT_ROAD ? 1 : 0) + 5].rate;
        if (flags & BIT_RIVER) {
            base_time++;
        }
        if ((flags & BIT_FUNGUS) && (tile->climate & 0xE0) > 0x3F) {
            base_time += 2;
        }
        if ((flags & BIT_FOREST) == 0) {
            const uint32_t bonus = (uint32_t)(tile->val3 >> 6);
            base_time = base_time + bonus;
        } else {
            base_time += 2;
        }
        break;
    }
    case TERRA_REMOVE_FUNGUS:
        if (has_abil(Vehs[veh_id].proto_id, 0x2000)) {
            base_time = base_time / 2;
        }
        break;
    default:
        break;
    }

    const uint8_t faction_bit = (uint8_t)(1 << (faction & 0x1F));
    if (!(LockEnableMask & faction_bit) && DiffLevelCurrent > 3
        && base_time > 3) {
        base_time--;
    }

    int turns_accum = 0;
    int progress_accum = 0;
    if (veh_id >= 0) {
        int stack_bottom = veh_id;
        int16_t prev = Vehs[stack_bottom].prev_veh_id_stack;
        while (prev >= 0) {
            stack_bottom = prev;
            prev = Vehs[stack_bottom].prev_veh_id_stack;
        }
        for (int v = stack_bottom; v >= 0; v = Vehs[v].next_veh_id_stack) {
            if (v != veh_id && (int8_t)Vehs[v].order == (int8_t)(action + 4)) {
                if (do_it == 0) {
                    const int8_t v_faction = (int8_t)Vehs[v].faction_id;
                    uint32_t base = 2;
                    uint32_t weight;
                    if (has_abil(Vehs[v].proto_id, 1) == 0) {
                        weight = 2;
                    } else {
                        weight = 4;
                        base = 4;
                    }
                    if (action == TERRA_REMOVE_FUNGUS
                        || action == TERRA_PLANT_FUNGUS) {
                        if (*SecretProjectXenoempathyBase >= 0
                            && Bases[*SecretProjectXenoempathyBase]
                                   .faction_id_current
                                == (uint8_t)v_faction) {
                            weight = base * 2;
                        }
                    } else if (*SecretProjectBoreholeBase >= 0
                               && Bases[*SecretProjectBoreholeBase]
                                      .faction_id_current
                                   == (uint8_t)v_faction) {
                        weight = (base * 3) / 2;
                    }
                    turns_accum += weight;
                    progress_accum += Vehs[v].terraforming_turns;
                } else {
                    Vehs[veh_id].terraforming_turns =
                        Vehs[veh_id].terraforming_turns
                        + Vehs[v].terraforming_turns;
                    Vehs[v].terraforming_turns = 0;
                }
            }
        }
    }

    {
        const int8_t my_faction = (int8_t)Vehs[veh_id].faction_id;
        uint32_t base = 2;
        uint32_t weight;
        if (has_abil(Vehs[veh_id].proto_id, 1) == 0) {
            weight = 2;
        } else {
            weight = 4;
            base = 4;
        }
        if (action == TERRA_REMOVE_FUNGUS || action == TERRA_PLANT_FUNGUS) {
            if (*SecretProjectXenoempathyBase >= 0
                && Bases[*SecretProjectXenoempathyBase].faction_id_current
                    == (uint8_t)my_faction) {
                weight = base * 2;
            }
        } else if (*SecretProjectBoreholeBase >= 0
                   && Bases[*SecretProjectBoreholeBase].faction_id_current
                       == (uint8_t)my_faction) {
            weight = (base * 3) / 2;
        }
        turns_accum += weight;
        progress_accum += Vehs[veh_id].terraforming_turns;
    }

    if (do_it == 0) {
        const int cur_speed = speed(veh_id, 0);
        const int since_move = cur_speed - Vehs[veh_id].moves_expended;
        if (since_move >= 0
            && (since_move > 999 || cur_speed != Vehs[veh_id].moves_expended)) {
            progress_accum += turns_accum;
        }
        progress_accum = base_time * 2 - progress_accum;
        if (turns_accum < 1) {
            turns_accum = 1;
        } else if (turns_accum > 0xFF) {
            return (progress_accum + 0xFE) / 0xFF;
        }
        return (progress_accum - 1 + turns_accum) / turns_accum;
    }

    veh_skip(veh_id);
    const uint8_t progress_before = Vehs[veh_id].terraforming_turns;
    Vehs[veh_id].terraforming_turns = progress_before + (uint8_t)turns_accum;
    if ((tile->bit & BIT_BASE_IN_TILE) != 0 && (tile->val2 & 0xF) < 8) {
        Vehs[veh_id].order = ORDER_NONE;
        return 0;
    }
    if ((int)(uint8_t)(progress_before + (uint8_t)turns_accum) < base_time * 2) {
        draw_tile(x, y, 2);
        (ORIGINAL(&TutWinBaseWindow)->*BaseWinOriginalCheckVeh)(veh_id);
        return 0;
    }
    Vehs[veh_id].terraforming_turns = 0;
    if (veh_id >= 0) {
        int stack_bottom = veh_id;
        int16_t prev = Vehs[stack_bottom].prev_veh_id_stack;
        while (prev >= 0) {
            stack_bottom = prev;
            prev = Vehs[stack_bottom].prev_veh_id_stack;
        }
        for (int v = stack_bottom; v >= 0; v = Vehs[v].next_veh_id_stack) {
            if ((int8_t)Vehs[v].order == (int8_t)(action + 4)) {
                Vehs[v].order = ORDER_NONE;
            }
        }
    }

    const uint32_t tile_flags = tile->bit;
    bit_set(x, y, Terraforming[action].bit_incompatible, 0);
    bit_set(x, y, Terraforming[action].bit, 1);
    synch_bit(x, y, faction);
    switch (action) {
    case TERRA_REMOVE_FUNGUS:
    case TERRA_PLANT_FUNGUS:
        site_radius(x, y, 0);
        break;
    case TERRA_CONDENSER:
    case TERRA_THERMAL_BORE:
        site_radius(x, y, 0);
        world_climate();
        draw_map(1);
        break;
    case TERRA_AQUIFER:
        bit_set(x, y, 0x100, 1);
        world_climate();
        draw_map(1);
        break;
    case TERRA_RAISE_LAND:
        world_raise_alt(x, y);
        world_climate();
        draw_map(1);
        break;
    case TERRA_LOWER_LAND:
        world_lower_alt(x, y);
        world_climate();
        draw_map(1);
        break;
    case TERRA_LEVEL_TERRAIN: {
        uint8_t rocky = tile->val3;
        if (rocky & 0xC0) {
            rocky_set(x, y, (rocky >> 6) - 1);
            site_radius(x, y, 0);
        }
        break;
    }
    default:
        break;
    }

    int base_idx = -1;
    if ((Terraforming[action].bit & 0x1208050) != 0) {
        base_idx = base_find(x, y, faction);
        if (base_idx >= 0 && BaseFindDist < 3) {
            Bases[base_idx].state |= 0x100;
        }
        if (action == TERRA_THERMAL_BORE && faction == ConsoleGlobal.faction_id_
            && base_idx >= 0 && Bases[base_idx].faction_id_current == faction) {
            parse_says(0, Bases[base_idx].name_string, -1, -1);
            NetMsgPrimary.pop(VehLabelBOREHOLEMINE, 5000, 0, 0);
        }
    }

    if ((tile_flags & BIT_FOREST) == 0
        || ((map_tiles()
             + ((x >> 1) + y * MapLongitude))
                ->bit
            & BIT_FOREST)
            != 0
        || ((map_tiles()
             + ((x >> 1) + y * MapLongitude))
                ->climate
            & 0xE0)
            < ALT_BIT_SHORE_LINE) {
        if (action == TERRA_RAISE_LAND || action == TERRA_LOWER_LAND
            || action == TERRA_CONDENSER) {
            goto forest_shared;
        }
    } else if (action != TERRA_RAISE_LAND) {
        int near_base = Vehs[veh_id].home_base_id;
        if (near_base < 0) {
            near_base = base_find(x, y, faction);
        }
        if (near_base >= 0) {
            const int forest_val = Bases[near_base].minerals_accumulated
                + Rules.minerals_harvesting_forest;
            Bases[near_base].minerals_accumulated = forest_val;
            Bases[near_base].minerals_accumulated_2 = forest_val;
            Bases[near_base].production_id_last =
                Bases[near_base].queue_production_id[0];
            if (faction == ConsoleGlobal.faction_id_ && (faction_bit & LockEnableMask) != 0
                && Rules.minerals_harvesting_forest != 0) {
                parse_says(0, Bases[near_base].name_string, -1, -1);
                parse_num(0, Rules.minerals_harvesting_forest);
                NetMsgPrimary.pop(VehLabelHARVESTFOREST, 5000, 0, 0);
            }
            TutWinBaseWindow.check_base(near_base);
        }
        goto forest_shared;
    } else {
        if (TerraformTileFaction >= 0
            && !(LockEnableMask & (uint8_t)(1 << (TerraformTileFaction & 0x1F)))
            && (faction_bit & LockEnableMask) != 0
            && (PlayersData[TerraformTileFaction].diplo_treaties[faction]
                & 0x11)
                == 0) {
            if ((PlayersData[faction].diplo_treaties[TerraformTileFaction]
                 & 0x20)
                == 0) {
                cause_friction(TerraformTileFaction, faction, 5);
            } else {
                set_treaty(TerraformTileFaction, faction, 0x400, 1);
            }
        }
        if (TerraformDrownedBaseId >= 0) {
            parse_says(0, Bases[TerraformDrownedBaseId].name_string, -1, -1);
            parse_says(1, Players[faction].adj_name_faction, -1, -1);
            const char *msg = faction == ConsoleGlobal.faction_id_ ? VehLabelTERRAMINE
                                                      : VehLabelTERRAYOURS;
            NetMsgPrimary.pop(msg, 5000, 0, 0);
        }
        if (MapBaseIdClosestSubmergedVeh[ConsoleGlobal.faction_id_] >= 0) {
            const int drown_base = MapBaseIdClosestSubmergedVeh[faction];
            parse_says(0, Bases[drown_base].name_string, -1, -1);
            parse_says(1, Players[faction].adj_name_faction, -1, -1);
            const char *msg = faction == ConsoleGlobal.faction_id_ ? VehLabelDROWNMINE
                                                      : VehLabelDROWNYOURS;
            NetMsgPrimary.pop(msg, 5000, 0, 0);
        }
    }

forest_shared:
    synch_bit(x, y, faction);
    draw_tiles(x, y, 2);
    if (veh_count_before == VehCurrentCount) {
        (ORIGINAL(&TutWinBaseWindow)->*BaseWinOriginalCheckVeh)(veh_id);
        TutWinBaseWindow.check_loc(x, y, 2);
    } else {
        TutWinBaseWindow.GraphicWin::redraw();
    }

    if (faction != ConsoleGlobal.faction_id_) {
        return 1;
    }

    if ((Vehs[veh_id].state & VSTATE_UNK_200) != 0
        && (GamePreferences & 0x20) != 0 && IsMultiplayerNet == 0) {
        const uint32_t bit = 1 << (action & 0x1F);
        uint32_t seen = TerraformAnnouncedMask[not_ocean];
        if ((bit & seen) == 0) {
            const uint32_t new_mask = seen | bit;
            TerraformAnnouncedMask[not_ocean] = new_mask;
            const void *sprite = 0;
            const char *msg = 0;
            int noun = veh_id;
            bool do_popup = true;
            if (not_ocean == 0) {
                switch (action) {
                case TERRA_FARM:
                    sprite = VehSpriteFARM;
                    noun = -1;
                    msg = VehLabelMADEFARM;
                    break;
                case TERRA_SOIL_ENR:
                    sprite = VehSpriteSOIL;
                    noun = -1;
                    msg = VehLabelMADESOIL;
                    break;
                case TERRA_MINE:
                    sprite = VehSpriteMINE;
                    noun = -1;
                    msg = VehLabelMADEMINE;
                    break;
                case TERRA_SOLAR_TIDAL:
                    sprite = VehSpriteSOLAR;
                    noun = -1;
                    msg = VehLabelMADESOLAR;
                    break;
                case TERRA_FOREST:
                    msg = VehLabelMADEFOREST;
                    break;
                case TERRA_ROAD:
                    msg = VehLabelMADEROAD;
                    break;
                case TERRA_MAGTUBE:
                    msg = VehLabelMADETUBE;
                    break;
                case TERRA_SENSOR:
                    msg = VehLabelMADESENSOR;
                    break;
                case TERRA_CONDENSER:
                    msg = VehLabelMADECONDENSER;
                    break;
                case TERRA_THERMAL_BORE:
                    msg = VehLabelMADEBOREHOLE;
                    break;
                case TERRA_AQUIFER:
                    msg = VehLabelMADEAQUIFER;
                    break;
                default:
                    TerraformAnnouncedMask[not_ocean] = ~bit & new_mask;
                    do_popup = false;
                    break;
                }
            } else if (action == TERRA_FARM) {
                sprite = VehSpriteFARM;
                msg = VehLabelMADESEAFARM;
                noun = -1;
            } else if (action == TERRA_MINE) {
                sprite = VehSpriteMINE;
                msg = VehLabelMADESEAMINE;
                noun = -1;
            } else if (action == TERRA_SOLAR_TIDAL) {
                sprite = VehSpriteSOLAR;
                msg = VehLabelMADESEASOLAR;
                noun = -1;
            } else {
                TerraformAnnouncedMask[not_ocean] = ~bit & new_mask;
                do_popup = false;
            }
            if (do_popup) {
                (ORIGINAL(TutWinGlobal)->*TutWinOriginalTutMap)(
                    msg, x, y, noun, const_cast<void *>(sprite), 1, -1, -1);
            }
        }
    }

    if (faction == ConsoleGlobal.faction_id_ && Vehs[veh_id].order == ORDER_NONE
        && !(Vehs[veh_id].state & VSTATE_UNK_200)
        && VehTerraformStingLatch == 0) {
        VehTerraformStingLatch = 1;
        wave_it(0x14);
        ambience(0xCF);
    }
    return 1;
}

/*
Purpose: Dispatch a unit's order byte to the verb that carries it out.
// ORIGINAL: 0x004CF740 ?action@@YAHH@Z 0x004CF740-0x004CF7B4 BYTE_EXACT
// size      116 bytes
// prototype int (__cdecl ?action@@YAHH@Z)(int veh_id)
// callers   3   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004C9B00 0x004CB310 0x004CB580
// HOMED from src/unrecovered/004cf740.cpp, which carried the archive's own
// FILE BYTE_EXACT claim for these 116 bytes; re-earned in the TU 2026-09-04.
// Its `g_00952839` byte view is Vehs[].order (+0x11) - the whole body is a
// dispatch on it. The three arms cover every order the AI and the interface
// can queue: the terraforming range ORDER_FARM..ORDER_MOVE_TO-1 (4..0x17) goes
// to action_terraform with the order's TERRA_* index, the two road verbs go to
// their walkers, and everything in the move family goes to action_go_to.
// LEVER: the order byte is `int8_t`, not `int` and not `uint8_t`. The image
//        keeps it in `al` and compares SIGNED (`cmp al,4 / jl`, `cmp al,0x18 /
//        jge`) - `int` widened it to `movsx eax` and cost the byte form, and
//        `uint8_t` turned the branch unsigned (`jb`/`jae`). Naming it after
//        the field it reads, which veh.h declares `int8_t order`, is what
//        closed the last instruction.
// LEVER: the terraforming arm's upper bound is ORDER_MOVE_TO (0x18), not
//        ORDER_PLACE_MONOLITH - the archive's own `state < 0x18` is the
//        half-open end of the range, and I first transcribed it as the
//        symbolic name the value 0x17 would have had.
Return Value: 1 when an arm ran and the order was cleared, 0 otherwise.
Status: BYTE_EXACT.
*/
int __cdecl action(int veh_id) {
    const int8_t order = Vehs[veh_id].order;
    if (order >= ORDER_FARM && order < ORDER_MOVE_TO) {
        action_terraform(veh_id, order - 4, 1);
    } else if (order == ORDER_ROAD_TO || order == ORDER_MAGTUBE_TO) {
        action_road_to(veh_id);
    } else if (order == ORDER_MOVE_TO || order == ORDER_AI_MOVE_TO
               || order == ORDER_MOVE) {
        action_go_to(veh_id);
    } else {
        return 0;
    }
    if (Vehs[veh_id].order & 0x40) {
        Vehs[veh_id].order = ORDER_NONE;
    }
    return 1;
}

/*
Purpose: Find whether a faction can airdrop a unit onto a tile.
// ORIGINAL: 0x00500380 sub_500380 0x00500380-0x005004EA
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
Return Value: n/a - the image discards the result; the walk is the effect.
Status: MISMATCH.
*/
void __cdecl sub_500380(int faction_id, int x, int y) {
    bool anywhere;
    if (has_tech_call(Rules.tech_orb_insert_sans_spc_elev, faction_id)) {
        anywhere = true;
    } else if (*SecretProjectAirdropBase >= 0) {
        anywhere = Bases[*SecretProjectAirdropBase].faction_id_current
            == (uint8_t)faction_id;
    } else {
        anywhere = false;
    }

    int threshold;
    if (anywhere) {
        threshold = MapLongitudeBounds;
        if (threshold <= MapLatitudeBounds) {
            threshold = MapLatitudeBounds;
        }
    } else {
        threshold = Rules.max_airdrop_sans_orb_insert;
    }

    const int count = VehCurrentCount;
    for (int i = 0; i < count; ++i) {
        if (Vehs[i].faction_id != faction_id) {
            continue;
        }
        if (!has_abil(Vehs[i].proto_id, 0x10)) {
            continue;
        }
        if (Vehs[i].state & VSTATE_MADE_AIRDROP) {
            continue;
        }
        if (Vehs[i].moves_expended != 0) {
            continue;
        }
        int dx = abs(x - Vehs[i].x);
        if (!(MapIsFlat & 1) && dx > (int)MapLongitude) {
            dx = MapLongitudeBounds - dx;
        }
        const int dy = abs(y - Vehs[i].y);
        if ((dy + dx) >> 1 > threshold) {
            continue;
        }
        Map *tile = map_tiles()
            + ((Vehs[i].x >> 1) + Vehs[i].y * MapLongitude);
        if (tile->bit & BIT_BASE_IN_TILE) {
            if ((tile->val2 & 0xf) < 8) {
                return;
            }
        }
        if (tile->bit & BIT_AIRBASE) {
            return;
        }
    }
}

// 0x00691B0C holds the address of the popup title the obliteration report
// shares with the climate popup - game.h binds the same slot as
// ClimatePopupTitle, so this body reads the title through it.
// The other four are the phrase keys and the portrait the two report popups
// take: the obliterator's own faction gets the plain pair, everyone else the
// "2" pair the game uses for a faction hearing about another faction's act.
char *const VehIconBASEOBL_SM_PCX = (char *)0x00687FFC;   // "baseobl_sm.pcx"
char *const VehLabelOBLITTED = (char *)0x0068800C;        // "OBLITTED"
char *const VehIconBASEOBL2_SM_PCX = (char *)0x00688018;  // "baseobl_sm.pcx"
char *const VehLabelOBLITTED2 = (char *)0x00688028;       // "OBLITTED2"

// Still archived, so veh.cpp cannot link to a definition: 0x0055EEE0 carries
// the atrocity the obliteration commits and is forwarded like the other
// unrecovered callees in this file.
void __cdecl atrocity(int faction_id, int faction_id_victim, int kind,
                      int unk);
// base_kill's forwarder lives in pending_bodies.cpp beside the other
// unrecovered callees; no header claims it yet, so this file declares it.
void __cdecl base_kill(int base_id);

/*
Purpose: Obliterate a base and report it to every faction that can hear.
// ORIGINAL: 0x004CD4E0 ?action_oblit@@YAXHH@Z 0x004CD4E0-0x004CD6F9
// size      537 bytes
// prototype void (__cdecl ?action_oblit@@YAXHH@Z)(int veh_id, int base_id)
// callers   1   call targets   4
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0048C0A0 0x0046B190 0x004E5250 0x0055EEE0 0x00625E30 0x00625EC0
// symbol    ?action_oblit@@YAXHH@Z
// HOMED from src/recovered/units/004cd4e0.cpp, 2026-09-04. Its indexed table
// bases are Vehs[].faction_id (0x00952836, 0x34 stride), Bases[]
// .faction_id_current / .faction_id_former / .population_size / .visibility /
// .name_string (0x0097D044/45/46/4A/53, 0x134 stride), and Rules.
// tgl_oblit_base_atrocity (0x00949860 = Rules + 0x128) gating the atrocity.
// The three Player rows it walks are name_leader (+0x34), title_leader
// (+0x4C) and faction_bonus_count (+0x50C, read as GenderDefault's value) -
// 0x00946A50 is Players itself, so a row index of n * 0x167 is the n-th
// record's dword at that offset.
//
// BUG IN THE ORIGINAL: the owner's name row is computed once into a local and
// then reused for BOTH the fourth and the fifth parse_says, so the fifth call
// prints from a row it never recomputed for. Transcribed as written.
//
// TRIED (the units archive, which measured 0.90 similarity / 15 of 138 edits
//        at 445 bytes both sides): the remaining divergence is the prologue's
//        register-save ORDER - the image loads the veh_id argument into eax
//        before saving ebx/esi/edi, this tree saves a callee-saved register
//        first. Register allocation, not control flow.
Return Value: n/a
Status: MISMATCH.
*/
void __cdecl action_oblit(int veh_id, int base_id) {
    const unsigned veh_faction = Vehs[veh_id].faction_id;
    const unsigned base_former = Bases[base_id].faction_id_former;
    parse_num(0, (int)Bases[base_id].population_size * 10);
    veh_id = base_former;
    parse_says(0, Bases[base_id].name_string, -1, -1);

    PluralityDefault = 0;
    GenderDefault = Players[ConsoleGlobal.faction_id_].faction_bonus_count;
    parse_says(1, Players[ConsoleGlobal.faction_id_].name_leader, -1, -1);

    parse_says(2, Players[base_former].adj_name_faction, -1, -1);

    PluralityDefault = 0;
    GenderDefault = Players[veh_faction].faction_bonus_count;
    parse_says(3, Players[veh_faction].title_leader, -1, -1);

    GenderDefault = Players[veh_faction].faction_bonus_count;
    PluralityDefault = 0;
    parse_says(4, Players[veh_faction].name_leader, -1, -1);

    const bool audible = Bases[base_id].faction_id_current == ConsoleGlobal.faction_id_
        || (Bases[base_id].visibility & (1 << (ConsoleGlobal.faction_id_ & 0x1f))) != 0;

    base_kill(base_id);
    draw_map(1);

    if ((int)veh_faction == ConsoleGlobal.faction_id_) {
        popp((char *)*ClimatePopupTitle, VehLabelOBLITTED, 0,
             VehIconBASEOBL_SM_PCX, 0);
    } else if ((int)base_former == ConsoleGlobal.faction_id_ || audible) {
        popp((char *)*ClimatePopupTitle, VehLabelOBLITTED2, 0,
             VehIconBASEOBL2_SM_PCX, 0);
    }

    if (Rules.tgl_oblit_base_atrocity != 0) {
        atrocity(veh_faction, base_former, 2, 0);
    }
}

// Orbital combat uses the four source-owned faction satellite counters.
// ORIGINAL: 0x004CF480 ?action_sat_attack@@YAXHHHH@Z 0x004CF480-0x004CF73E
// RECOVERY_BINDING: v1 {"evidence": ["git:a538e0a3:src/recovered/units/004cf480.cpp", "image:0x004CF480-0x004CF73E complete instructions and outcome literals", "source:src/faction.h:PlayerData satellite array", "source:src/reportif.h", "source:src/maininterface.h"], "reason": "Recover orbital attack effects in the vehicle-action domain using real faction diplomacy, grammar, indexed satellite counters and report state. Original CRT randomness, parsing order, unchecked satellite indexing and popup outcomes are retained; existing popup and diplomacy seams remain separate obligations."}
void __cdecl action_sat_attack(int attacker, int target, int type, int base_id) {
    if (!(PlayersData[attacker].diplo_treaties[target] & 0x10))
        double_cross(attacker, target, -1);

    if (attacker == ConsoleGlobal.faction_id_ || target == ConsoleGlobal.faction_id_) {
        PluralityDefault = 0;
        GenderDefault = Players[attacker].is_leader_female;
        parse_says(0, Players[attacker].title_leader, -1, -1);
        GenderDefault = Players[attacker].is_leader_female;
        PluralityDefault = 0;
        parse_says(1, Players[attacker].name_leader, -1, -1);
        GenderDefault = Players[attacker].noun_gender;
        PluralityDefault = Players[attacker].is_noun_plural;
        parse_says(2, Players[attacker].noun_faction, -1, -1);

        PluralityDefault = 0;
        GenderDefault = Players[target].is_leader_female;
        parse_says(3, Players[target].title_leader, -1, -1);
        GenderDefault = Players[target].is_leader_female;
        PluralityDefault = 0;
        parse_says(4, Players[target].name_leader, -1, -1);
        GenderDefault = Players[target].noun_gender;
        PluralityDefault = Players[target].is_noun_plural;
        parse_says(5, Players[target].noun_faction, -1, -1);
        parse_say(6, reinterpret_cast<int>(Facility[type == 4 ? 41 : 65 + type].name), -1, -1);
        // Native 4CF606 selects Player + 0x304, the faction adjective.
        parse_says(7, Players[target].adj_name_faction, -1, -1);
    }

    // Original CRT rand, signed idiv remainder, exactly once after parsing.
    // No random() substitution, counter clamping or invalid-type filtering.
    if (rand() % 100 < 50) {
        if (type == 4) set_fac(41, base_id, 0);
        else --PlayersData[target].satellites_[type];
        ++PlayersData[attacker].sat_odp_deployed;
        if (attacker == ConsoleGlobal.faction_id_)
            popp(reinterpret_cast<char *>(*ClimatePopupTitle), "WEKILLEDIT", 0, "satbat_sm.pcx", 0);
        else if (target == ConsoleGlobal.faction_id_)
            popp(reinterpret_cast<char *>(*ClimatePopupTitle), "THEYKILLEDIT", 0, "satbat_sm.pcx", 0);
    } else {
        --PlayersData[attacker].satellites_[3];
        if (attacker == ConsoleGlobal.faction_id_)
            popp(reinterpret_cast<char *>(*ClimatePopupTitle), "WEMUFFEDIT", 0, "satbat_sm.pcx", 0);
        else if (target == ConsoleGlobal.faction_id_)
            popp(reinterpret_cast<char *>(*ClimatePopupTitle), "THEYMUFFEDIT", 0, "satbat_sm.pcx", 0);
    }
    if (ReportIfGlobal.field_8_ == 6 && IfaceIdToSlotStorage[ActiveIfaceIdStorage] == 5)
        ReportIfGlobal.on_redraw();
}

// Unfinished dependency of the airdrop recovery. Preserve the complete seed
// in src/unrecovered/005b6260.cpp; no visibility behavior is recovered here.
// ORIGINAL: 0x005B6260 ?spot_all@@YAXHH@Z 0x005B6260-0x005B8AE2
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/005b6260.cpp retained original signature and extent; full seed preserved", "Coordinator user authorizes explicit unfinished empty bodies for new referenced functions, never replacement of recovered behavior"], "reason": "Explicit unfinished owning-TU link-continuity dependency for full action_airdrop authoring; BODY GOES HERE preserves unknown behavior and is not a recovery or matching claim."}
void __cdecl spot_all(int veh_id, int toggle) {
    // BODY GOES HERE.
}

// Unfinished dependency of the airdrop recovery. Preserve the complete seed
// in src/unrecovered/0057a660.cpp. Zero is only an explicit placeholder result,
// not evidence for the original encounter behavior.
// ORIGINAL: 0x0057A660 ?goody_box@@YAHHH@Z 0x0057A660-0x0057CDE6
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/0057a660.cpp retained original signature and extent; full seed preserved", "Coordinator user authorizes explicit unfinished empty bodies for new referenced functions, never replacement of recovered behavior"], "reason": "Explicit unfinished owning-TU link-continuity dependency for full action_airdrop authoring; BODY GOES HERE preserves unknown behavior and is not a recovery or matching claim."}
int __cdecl goody_box(int veh_id, int type) {
    // BODY GOES HERE.
    return 0; // PLACEHOLDER: explicit link continuity, encounter result unrecovered.
}


// Coupled transfer/terrain actions and their still-unfinished prototype dependencies.
// Partial dependencies retain explicit BODY GOES HERE sentinels and seed obligations.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004CEDE0 ?action_give@@YAXHH@Z 0x004CEDE0-0x004CF379
// RECOVERY_BINDING: v1 {"evidence": ["Pinned entry 004CEDE0; retained seed src/recovered/units/004cede0.cpp", "recovery/tasks/vehicle-special-actions-coverage.json; native scope expansion preserves task baseline", "Native4CE326/4CE7F0 kill precedes announcement; fungal EBX retains initial combat faction while later slot reads observe compacted storage;580B51/580BF7 are mask immediates, not globals."], "reason": "Typed transfer or terrain action preserves native parser, kill, interception and post-call vehicle-slot read order; retained seed remains evidence, not executable backing."}
void __cdecl action_give(int vehicle_id, int faction_id) {
    const int x = Vehs[vehicle_id].x;
    const int y = Vehs[vehicle_id].y;
    const int original_owner = Vehs[vehicle_id].faction_id;

    if (vehicle_id >= 0) {
        int stack_head = vehicle_id;
        while (Vehs[stack_head].prev_veh_id_stack >= 0) {
            stack_head = Vehs[stack_head].prev_veh_id_stack;
        }

        for (int current = stack_head; current >= 0;
             current = Vehs[current].next_veh_id_stack) {
            const bool is_root = current == vehicle_id;
            const bool is_cargo = Vehs[current].order == ORDER_SENTRY_BOARD
                && Vehs[current].waypoint_x[0] == vehicle_id;
            if (!is_root && !is_cargo) continue;

            const int source_proto_id = Vehs[current].proto_id;
            int receiving_proto_id = source_proto_id;
            if (source_proto_id >= MaxVehProtoFactionNum) {
                const VehPrototype &source = VehPrototypes[source_proto_id];
                const int faction_proto_base = faction_id * MaxVehProtoFactionNum;
                receiving_proto_id = -1;

                // The recipient's block is a 64-record slice of the global
                // prototype table. The source walk tests the obsolescence
                // bit before comparing the complete component tuple.
                for (int slot = 0; slot < MaxVehProtoFactionNum; ++slot) {
                    const int candidate_id = faction_proto_base + slot;
                    const VehPrototype &candidate = VehPrototypes[candidate_id];
                    if ((candidate.obsolete_factions & 1) != 0
                        && candidate.ability_flags == source.ability_flags
                        && candidate.chassis_id == source.chassis_id
                        && candidate.weapon_id == source.weapon_id
                        && candidate.armor_id == source.armor_id
                        && candidate.reactor_id == source.reactor_id) {
                        receiving_proto_id = candidate_id;
                        break;
                    }
                }

                if (receiving_proto_id < 0) {
                    receiving_proto_id = propose_proto(
                        faction_id, source.chassis_id, source.weapon_id,
                        source.armor_id, static_cast<int>(source.ability_flags),
                        source.reactor_id, source.unk_1, nullptr);
                }

                // Preserve the original ordered fallback probes. The first
                // and last compare bytes at the start of ability_flags (not
                // a reconstructed tuple), while the middle probe checks the
                // low byte of flags together with the plan.
                if (receiving_proto_id < 0) {
                    for (int slot = 0; slot < MaxVehProtoFactionNum; ++slot) {
                        const int candidate_id = faction_proto_base + slot;
                        const VehPrototype &candidate = VehPrototypes[candidate_id];
                        if ((candidate.obsolete_factions & 1) != 0
                            && static_cast<uint8_t>(candidate.ability_flags) == source.chassis_id
                            && ((candidate.ability_flags >> 8) & 0xff) == source.weapon_id) {
                            receiving_proto_id = candidate_id;
                            break;
                        }
                    }
                }
                if (receiving_proto_id < 0) {
                    for (int slot = 0; slot < MaxVehProtoFactionNum; ++slot) {
                        const int candidate_id = faction_proto_base + slot;
                        const VehPrototype &candidate = VehPrototypes[candidate_id];
                        if ((candidate.flags & 1) != 0
                            && candidate.plan == source.plan) {
                            receiving_proto_id = candidate_id;
                            break;
                        }
                    }
                }
                if (receiving_proto_id < 0) {
                    for (int slot = 0; slot < MaxVehProtoFactionNum; ++slot) {
                        const int candidate_id = faction_proto_base + slot;
                        const VehPrototype &candidate = VehPrototypes[candidate_id];
                        if ((candidate.obsolete_factions & 1) != 0
                            && static_cast<uint8_t>(candidate.ability_flags) == source.chassis_id) {
                            receiving_proto_id = candidate_id;
                            break;
                        }
                    }
                }
                if (receiving_proto_id < 0) {
                    receiving_proto_id = Chassis[source.chassis_id].triad == TRIAD_SEA
                        ? BSC_TRANSPORT_FOIL : BSC_SCOUT_PATROL;
                }
            }

            Vehs[current].faction_id = static_cast<uint8_t>(faction_id);
            Vehs[current].proto_id = static_cast<int16_t>(receiving_proto_id);
            Vehs[current].order = ORDER_NONE;
            Vehs[current].state &= 0xfcffbdffu;
            const int home_base = base_find(x, y, faction_id);
            Vehs[current].home_base_id = static_cast<int16_t>(home_base);
        }

        // Map's +8 flag word and +2 owner nibble are Map members, not a
        // second unmodelled tile table. This preserves the original test.
        const Map *tile = map_loc(x, y);
        if ((tile->bit & 1) == 0 || (tile->val2 & 0x0f) > 7) {
            owner_set(x, y, faction_id);
        }
    }

    draw_radius(x, y, 3, 2);
    if (faction_id == ConsoleGlobal.faction_id_) {
        parse_says(0, VehPrototypes[Vehs[vehicle_id].proto_id].veh_name,
                   -1, -1);
        GenderDefault = Players[original_owner].is_leader_female;
        PluralityDefault = 0;
        parse_says(1, Players[original_owner].title_leader, -1, -1);
        GenderDefault = Players[original_owner].is_leader_female;
        PluralityDefault = 0;
        parse_says(2, Players[original_owner].name_leader, -1, -1);
        GenderDefault = Players[original_owner].noun_gender;
        PluralityDefault = Players[original_owner].is_noun_plural;
        parse_says(3, Players[original_owner].noun_faction, -1, -1);
        ConsoleGlobal.focus(x, y, original_owner);
        const int base_id = base_find(x, y);
        if (base_id >= 0) {
            parse_says(4, Bases[base_id].name_string, -1, -1);
            if (Language != 0
                || (LockEnableMask
                    & (1u << (original_owner & 0x1f))) != 0) {
                // The seed tests Language (0x009BC054), not the network
                // mode; this key is the pinned NUL string at 0x006882AC.
                NetMsgPrimary.pop("GOTUNIT", 5000, 0, nullptr);
                return;
            }
            const Map *tile = map_loc(x, y);
            // The original indexes the recipient's +0x197C byte run by the
            // tile's region (+3); PlayerData::unk_83 is the existing owner
            // of that per-region state. The other condition is prototype plan.
            if (PlayersData[faction_id].unk_83[tile->region] != 2
                && VehPrototypes[Vehs[vehicle_id].proto_id].plan != 2) {
                X_pop("NITOFFEND", nullptr);
            } else {
                X_pop("NITDEFEND", nullptr);
            }
            return;
        }
    } else if (original_owner == ConsoleGlobal.faction_id_) {
        GenderDefault = Players[faction_id].noun_gender;
        PluralityDefault = Players[faction_id].is_noun_plural;
        parse_says(3, Players[faction_id].noun_faction, -1, -1);
        NetMsgPrimary.pop(faction_id != 0 ? "GAVEUNIT" : "GAVEWILD",
                         5000, 0, nullptr);
    }
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004CE210 ?action_tectonic@@YAXHHH@Z 0x004CE210-0x004CE6A1
// RECOVERY_BINDING: v1 {"evidence": ["Pinned entry 004CE210; retained seed src/recovered/units/004ce210.cpp", "recovery/tasks/vehicle-special-actions-coverage.json; native scope expansion preserves task baseline", "Native4CE326/4CE7F0 kill precedes announcement; fungal EBX retains initial combat faction while later slot reads observe compacted storage;580B51/580BF7 are mask immediates, not globals."], "reason": "Typed transfer or terrain action preserves native parser, kill, interception and post-call vehicle-slot read order; retained seed remains evidence, not executable backing."}
void __cdecl action_tectonic(int vehicle_id, int x, int y) {
    int attacker = Vehs[vehicle_id].faction_id;
    const int prototype_id = Vehs[vehicle_id].proto_id;
    const int reactor = VehPrototypes[prototype_id].reactor_id;
    const int affected_tile_count = RadiusRange[reactor];
    const int base_id = base_find(x, y);
    bool focused_on_event = false;
    int shot_result = 0;
    int hit_faction[MaxPlayerNum] = {0};

    g_FX.play(0x53, Vehs[vehicle_id].x, Vehs[vehicle_id].y);

    if (attacker != ConsoleGlobal.faction_id_ && base_id >= 0) {
        const Map *tile = map_loc(x, y);
        const uint32_t faction_flags = PlayersData[ConsoleGlobal.faction_id_].flags;
        const bool known_location = (tile->visibility
            & (1u << (ConsoleGlobal.faction_id_ & 0x1f))) != 0;
        if (((faction_flags & PFLAG_MAP_REVEALED) != 0 || known_location)
            && Bases[base_id].faction_id_current == ConsoleGlobal.faction_id_) {
            ConsoleGlobal.focus(x, y, attacker);
            focused_on_event = true;
        }
    }

    // The kill compacts the vehicle table before the announcement parser runs.
    // Keep the original pre-kill attacker for combat, but preserve that order.
    attacker = Vehs[vehicle_id].faction_id;
    kill(vehicle_id);

    if (attacker != ConsoleGlobal.faction_id_ && base_id >= 0) {
        GenderDefault = Players[attacker].is_leader_female;
        PluralityDefault = 0;
        parse_says(0, Players[attacker].title_leader, -1, -1);
        GenderDefault = Players[attacker].is_leader_female;
        PluralityDefault = 0;
        parse_says(1, Players[attacker].name_leader, -1, -1);
        GenderDefault = Players[attacker].noun_gender;
        PluralityDefault = Players[attacker].is_noun_plural;
        parse_says(2, Players[attacker].noun_faction, -1, -1);
        parse_says(3, Bases[base_id].name_string, -1, -1);
        popp(*ClimatePopupTitle, "TECTONICMISSILE", 0,
             "tectonic_sm.pcx", nullptr);
    }

    // The increasing radius index is observable: later defender checks and
    // random calls follow this exact order in the original.
    for (int neighbor_index = 0; neighbor_index < affected_tile_count; ++neighbor_index) {
        const int near_x_unwrapped = x + RadiusOffsetX[neighbor_index];
        const int near_x = xrange(near_x_unwrapped);
        const int near_y = y + RadiusOffsetY[neighbor_index];
        if (!on_map(near_x, near_y)
            || (MapIsFlat && (near_x_unwrapped < 0
                              || near_x_unwrapped >= MapLongitudeBounds))) {
            continue;
        }
        const int nearby_vehicle = stack_fix(veh_at(near_x, near_y));
        if (nearby_vehicle < 0) continue;
        const int defender = Vehs[nearby_vehicle].faction_id;
        if (attacker == defender
            || (PlayersData[attacker].diplo_treaties[defender]
                & DTREATY_PACT) != 0
            || hit_faction[defender]) {
            continue;
        }
        shot_result = shoot_it(attacker, defender, x, y, 0);
        if (shot_result != 0) break;
        hit_faction[defender] = 1;
    }

    const int territory = whose_territory(attacker, x, y, nullptr, 0);
    if (territory >= 0 && territory != attacker
        && !hit_faction[territory]) {
        shot_result = shoot_it(attacker, territory, x, y, 0);
    }

    if (shot_result == 0) {
        for (int raise_index = 0; raise_index < reactor; ++raise_index) world_raise_alt(x, y);
        world_climate();
        draw_map(1);
        if (focused_on_event) {
            parse_says(0, Bases[base_id].name_string, -1, -1);
            GenderDefault = Players[Vehs[vehicle_id].faction_id].is_leader_female;
            PluralityDefault = 0;
            parse_says(1, Players[Vehs[vehicle_id].faction_id].title_leader, -1, -1);
            GenderDefault = Players[Vehs[vehicle_id].faction_id].is_leader_female;
            PluralityDefault = 0;
            parse_says(2, Players[Vehs[vehicle_id].faction_id].name_leader, -1, -1);
            GenderDefault = Players[Vehs[vehicle_id].faction_id].noun_gender;
            PluralityDefault = Players[Vehs[vehicle_id].faction_id].is_noun_plural;
            parse_says(3, Players[Vehs[vehicle_id].faction_id].noun_faction, -1, -1);
            popp(*ClimatePopupTitle, "TECMOFIED", 0,
                 "tectonic_sm.pcx", nullptr);
        }
        ++TectonicDetonationCount[Vehs[vehicle_id].faction_id];
        UnkBitfield1 |= 4;
    }
}

// ORIGINAL: 0x004CE790 ?action_fungal@@YAXHHH@Z 0x004CE790-0x004CEDDB
// RECOVERY_BINDING: v1 {"evidence": ["Pinned entry 004CE790; retained seed src/unrecovered/004ce790.cpp", "recovery/tasks/vehicle-special-actions-coverage.json; native scope expansion preserves task baseline", "Native4CE326/4CE7F0 kill precedes announcement; fungal EBX retains initial combat faction while later slot reads observe compacted storage;580B51/580BF7 are mask immediates, not globals."], "reason": "Typed transfer or terrain action preserves native parser, kill, interception and post-call vehicle-slot read order; retained seed remains evidence, not executable backing."}
void __cdecl action_fungal(int vehicle_id, int x, int y) {
    const int attacker = Vehs[vehicle_id].faction_id;
    const int nearby_tile_count = RadiusRange[1];
    const int local_base = base_find(x, y);
    int shot_by_faction[MaxPlayerNum] = {0};
    int shot_result = 0;
    bool focused_on_spread = false;

    g_FX.play(0x54, Vehs[vehicle_id].x, Vehs[vehicle_id].y);

    kill(vehicle_id);

    if (attacker != ConsoleGlobal.faction_id_ && local_base >= 0) {
        GenderDefault = Players[attacker].is_leader_female;
        PluralityDefault = 0;
        parse_says(0, Players[attacker].title_leader, -1, -1);
        GenderDefault = Players[attacker].is_leader_female;
        PluralityDefault = 0;
        parse_says(1, Players[attacker].name_leader, -1, -1);
        GenderDefault = Players[attacker].noun_gender;
        PluralityDefault = Players[attacker].is_noun_plural;
        parse_says(2, Players[attacker].noun_faction, -1, -1);
        parse_says(3, Bases[local_base].name_string, -1, -1);
        popp(*ClimatePopupTitle, "FUNGALMISSILE", 0,
             "fungpayld_sm.pcx", nullptr);
    }
    for (int neighbor_index = 0; neighbor_index < nearby_tile_count; ++neighbor_index) {
        const int raw_x = x + RadiusOffsetX[neighbor_index];
        const int near_x = xrange(raw_x);
        const int near_y = y + RadiusOffsetY[neighbor_index];
        if (!on_map(near_x, near_y)
            || (MapIsFlat && (raw_x < 0 || raw_x >= MapLongitudeBounds))) {
            continue;
        }
        const int nearby_vehicle = stack_fix(veh_at(near_x, near_y));
        if (nearby_vehicle < 0) continue;
        const int defender = Vehs[nearby_vehicle].faction_id;
        if (attacker == defender
            || (PlayersData[attacker].diplo_treaties[defender]
                & DTREATY_PACT) != 0
            || shot_by_faction[defender]) {
            continue;
        }
        shot_result = shoot_it(attacker, defender, x, y, 1);
        if (shot_result != 0) break;
        shot_by_faction[defender] = 1;
    }

    const int territory = whose_territory(attacker, x, y, nullptr, 0);
    if (territory >= 0 && territory != attacker
        && !shot_by_faction[territory]) {
        shot_result = shoot_it(attacker, territory, x, y, 1);
    }
    if (shot_result != 0) return;

    const int spread_bound = VehPrototypes[Vehs[vehicle_id].proto_id].reactor_id * 2;
    for (int spread_index = 0; spread_index < spread_bound; ++spread_index) {
        const int direction = spread_index == 0 ? 0 : 1 + (rand() & 7);
        const int raw_x = x + RadiusOffsetX[direction];
        const int spread_x = xrange(raw_x);
        const int spread_y = y + RadiusOffsetY[direction];
        if (!on_map(spread_x, spread_y)
            || (MapIsFlat && (raw_x < 0 || raw_x >= MapLongitudeBounds))) {
            continue;
        }

        // The seed initializes a vacant/invalid tile only when the low random
        // bit is set; its signed-mask sequence reduces to this bit test for
        // the CRT's nonnegative rand() result.
        if ((rand() & 1) != 0
            && (((map_loc(spread_x, spread_y)->bit & 3) == 0)
                || ((map_loc(spread_x, spread_y)->val2 & 0x0f) > 7))) {
            veh_init(BSC_MIND_WORMS, 0, spread_x, spread_y);
        }
        Map *tile = map_loc(spread_x, spread_y);
        if ((tile->bit & BIT_FUNGUS) != 0
            && (tile->climate & 0xe0) >= ALT_BIT_OCEAN_SHELF) {
            continue;
        }

        const int focus_faction = Vehs[vehicle_id].faction_id;
        if (focus_faction != ConsoleGlobal.faction_id_
            && ((PlayersData[ConsoleGlobal.faction_id_].flags
                    & PFLAG_MAP_REVEALED) != 0
                || (tile->visibility
                    & (1u << (ConsoleGlobal.faction_id_ & 0x1f))) != 0)
            && local_base >= 0
            && Bases[local_base].faction_id_current == ConsoleGlobal.faction_id_) {
            ConsoleGlobal.focus(spread_x, spread_y, focus_faction);
            focused_on_spread = true;
        }

        bit_set(spread_x, spread_y, BIT_FUNGUS, true);
        bit_set(spread_x, spread_y,
                BIT_ROAD | BIT_MAGTUBE | BIT_MINE | BIT_SOLAR_TIDAL
                | BIT_FARM | BIT_SOIL_ENRICHER | BIT_FOREST, false);
        bit_set(spread_x, spread_y, MapTileFlagClearMask, false);
        if ((tile->climate & 0xe0) < ALT_BIT_OCEAN_SHELF) {
            world_alt_set(spread_x, spread_y, 2, false);
        }
        if ((PlayersData[ConsoleGlobal.faction_id_].flags & PFLAG_MAP_REVEALED) != 0
            || (tile->visibility
                & (1u << (ConsoleGlobal.faction_id_ & 0x1f))) != 0
            || (GameState & 0x80) != 0) {
            synch_bit(spread_x, spread_y, ConsoleGlobal.faction_id_);
            draw_tile(spread_x, spread_y, 2);
        }
    }

    const Map *origin = map_loc(x, y);
    if ((origin->climate & 0xe0) > 0x5f) {
        veh_init(BSC_FUNGAL_TOWER, 0, x, y);
    }
    UnkBitfield1 |= 4;
    if (focused_on_spread && local_base >= 0) {
        parse_says(0, Bases[local_base].name_string, -1, -1);
        GenderDefault = Players[Vehs[vehicle_id].faction_id].is_leader_female;
        PluralityDefault = 0;
        parse_says(1, Players[Vehs[vehicle_id].faction_id].title_leader, -1, -1);
        GenderDefault = Players[Vehs[vehicle_id].faction_id].is_leader_female;
        PluralityDefault = 0;
        parse_says(2, Players[Vehs[vehicle_id].faction_id].name_leader, -1, -1);
        GenderDefault = Players[Vehs[vehicle_id].faction_id].noun_gender;
        PluralityDefault = Players[Vehs[vehicle_id].faction_id].is_noun_plural;
        parse_says(3, Players[Vehs[vehicle_id].faction_id].noun_faction, -1, -1);
        popp(*ClimatePopupTitle, "FUNGMOTIZED", 0,
             "fungpayld_sm.pcx", nullptr);
    }
}

// ORIGINAL: 0x004CDAF0 ?shoot_it@@YAHHHHHH@Z 0x004CDAF0-0x004CE206
// RECOVERY_BINDING: v1 {"evidence": ["Pinned entry 004CDAF0; retained seed src/unrecovered/004cdaf0.cpp", "recovery/tasks/vehicle-special-actions-coverage.json; native scope expansion preserves task baseline", "Native4CDBD4/4CDC08 load inventory after the first base scan;4CDC14 increments live deployed count;4CDC4B/51 snapshots bounded remaining attempts.", "Native4CDFF8..4CE018 unconditionally decrements live inventory after sacrifice callbacks and clamps deployed count;4CE059/063 reads reported inventory after parsing the base.", "Players noun_faction+2E4, noun_gender+2FC, is_noun_plural+300 and adj_name_faction+304 distinguish outcome and sacrifice text."], "reason": "Complete typed orbital-pod and Flechette interception, sacrifice, cancellation and reporting paths. Original binding began as an explicitly unfinished dependency; preserve its original spans and complete retained seed. Source implementation does not establish byte, reference, structure or runtime acceptance."}
int __cdecl shoot_it(int attacker, int defender, int x, int y,
                     int fungal_attack) {
    PlayerData &defending_faction = PlayersData[defender];
    int defense_pods;
    int attempts_spent;
    int result = 0;
    (void)fungal_attack;

    bool has_flechette_defense = false;
    for (int base_scan_index = 0; base_scan_index < RadiusRange[2]; ++base_scan_index) {
        const int neighbor_x = xrange(x + RadiusOffsetX[base_scan_index]);
        const int neighbor_y = y + RadiusOffsetY[base_scan_index];
        if (!on_map(neighbor_x, neighbor_y)) continue;
        const int neighbor_base = base_at(neighbor_x, neighbor_y);
        if (neighbor_base < 0
            || Bases[neighbor_base].faction_id_current == attacker) {
            continue;
        }
        if (has_fac_built(FAC_FLECHETTE_DEFENSE_SYS, neighbor_base)) {
            has_flechette_defense = true;
            break;
        }
    }
    defense_pods = static_cast<int>(defending_faction.satellites_[3]);
    attempts_spent = static_cast<int>(defending_faction.sat_odp_deployed);
    if (defense_pods == 0 && !has_flechette_defense) return 0;

    int unspent_attempts = defense_pods - attempts_spent;
    if (unspent_attempts < 0) unspent_attempts = 0;
    for (int attempt = 0; attempt < unspent_attempts; ++attempt) {
        ++defending_faction.sat_odp_deployed;
        if (rand() % 100 > 49) {
            result = 1;
            break;
        }
    }
    const int post_roll_pods =
        static_cast<int>(defending_faction.satellites_[3]);
    const int post_roll_attempts =
        static_cast<int>(defending_faction.sat_odp_deployed);
    int remaining_attempts = post_roll_pods - post_roll_attempts;
    if (remaining_attempts < 0) {
        remaining_attempts = 0;
    } else if (remaining_attempts > 9999) {
        remaining_attempts = 9999;
    }

    // If the orbital pods did not intercept, each neighboring enemy base
    // with a Flechette system gets its own independent 50-percent roll.
    if (result == 0) {
        for (int flechette_scan_index = 0; flechette_scan_index < RadiusRange[2]; ++flechette_scan_index) {
            const int neighbor_x = xrange(x + RadiusOffsetX[flechette_scan_index]);
            const int neighbor_y = y + RadiusOffsetY[flechette_scan_index];
            if (!on_map(neighbor_x, neighbor_y)) continue;
            const int neighbor_base = base_at(neighbor_x, neighbor_y);
            if (neighbor_base < 0
                || Bases[neighbor_base].faction_id_current == attacker
                || !has_fac_built(FAC_FLECHETTE_DEFENSE_SYS, neighbor_base)) {
                continue;
            }
            if (rand() % 100 < 50) {
                parse_says(0, Bases[neighbor_base].name_string, -1, -1);
                if (fungal_attack == 0) {
                    if (defender == ConsoleGlobal.faction_id_) {
                        parse_says(1, Players[attacker].adj_name_faction, -1, -1);
                        popp(*ClimatePopupTitle, "TWESHOTIT2", 0,
                             "space_sm.pcx", nullptr);
                    } else {
                        if (attacker != ConsoleGlobal.faction_id_) return 1;
                        const int base_faction = Bases[neighbor_base].faction_id_current;
                        parse_says(1, Players[base_faction].adj_name_faction, -1, -1);
                        popp(*ClimatePopupTitle, "TTHEYSHOTIT2", 0,
                             "space_sm.pcx", nullptr);
                    }
                } else if (defender == ConsoleGlobal.faction_id_) {
                    parse_says(1, Players[attacker].adj_name_faction, -1, -1);
                    popp(*ClimatePopupTitle, "FWESHOTIT2", 0,
                         "space_sm.pcx", nullptr);
                } else {
                    if (attacker != ConsoleGlobal.faction_id_) return 1;
                    const int base_faction = Bases[neighbor_base].faction_id_current;
                    parse_says(1, Players[base_faction].adj_name_faction, -1, -1);
                    popp(*ClimatePopupTitle, "FTHEYSHOTIT2", 0,
                         "space_sm.pcx", nullptr);
                }
                return 1;
            }
        }
    }

    // With no orbital pod left, a failed Flechette roll has its own distinct
    // failure report and returns without consuming satellite inventory.
    if (result == 0
        && static_cast<int>(defending_faction.satellites_[3]) == 0) {
        if (has_flechette_defense) {
            if (defender == ConsoleGlobal.faction_id_) {
                parse_says(1, Players[attacker].adj_name_faction, -1, -1);
                popp(*ClimatePopupTitle, "WEFAILEDFLECHETTE", 0,
                     "space_sm.pcx", nullptr);
            } else {
                if (attacker != ConsoleGlobal.faction_id_) return 0;
                parse_says(1, Players[defender].adj_name_faction, -1, -1);
                popp(*ClimatePopupTitle, "THEYFAILEDFLECHETTE", 0,
                     "space_sm.pcx", nullptr);
            }
        }
        return 0;
    }

    // A successful orbital roll reaches the ordinary completion report
    // directly. Failed rolls continue through the native sacrifice prompts.
    bool failed_prompt_dismissed = false;
    if (result == 0) {
        if (IsMultiplayerNet == 0
            && (LockEnableMask & (1u << (defender & 0x1f))) != 0) {
            if (defender == ConsoleGlobal.faction_id_) {
                parse_num(0, static_cast<int>(defending_faction.satellites_[3]));
                const int base_id = base_find(x, y);
                parse_says(0, Bases[base_id].name_string, -1, -1);
                parse_says(1, Players[attacker].adj_name_faction, -1, -1);
                const int dismissed = popp(*ClimatePopupTitle,
                    fungal_attack == 0 ? "TECTONICSAC" : "FUNGALSAC",
                    0, "space_sm.pcx", nullptr);
                failed_prompt_dismissed = dismissed != 0;
            } else if (attacker == ConsoleGlobal.faction_id_) {
                GenderDefault = Players[defender].noun_gender;
                PluralityDefault = Players[defender].is_noun_plural;
                parse_says(0, Players[defender].noun_faction, -1, -1);
                popp(*ClimatePopupTitle, "THEYMUSTSAC", 0,
                     "space_sm.pcx", nullptr);
            }
        } else if (defender == ConsoleGlobal.faction_id_) {
            popp(*ClimatePopupTitle,
                 fungal_attack == 0 ? "TWEMUSTSAC" : "FWEMUSTSAC",
                 0, "space_sm.pcx", nullptr);
        } else if (attacker == ConsoleGlobal.faction_id_) {
            GenderDefault = Players[defender].noun_gender;
            PluralityDefault = Players[defender].is_noun_plural;
            parse_says(0, Players[defender].noun_faction, -1, -1);
            popp(*ClimatePopupTitle, "THEYMUSTSAC", 0,
                 "space_sm.pcx", nullptr);
        }
    }

    if (failed_prompt_dismissed) {
        if (has_flechette_defense) {
            if (defender == ConsoleGlobal.faction_id_) {
                parse_says(1, Players[attacker].adj_name_faction, -1, -1);
                popp(*ClimatePopupTitle, "WEFAILEDFLECHETTE", 0,
                     "space_sm.pcx", nullptr);
            } else {
                if (attacker != ConsoleGlobal.faction_id_) return 0;
                parse_says(1, Players[defender].adj_name_faction, -1, -1);
                popp(*ClimatePopupTitle, "THEYFAILEDFLECHETTE", 0,
                     "space_sm.pcx", nullptr);
            }
        }
        return 0;
    }

    if (result == 0) {
        int current_pods = static_cast<int>(defending_faction.satellites_[3]);
        --current_pods;
        defending_faction.satellites_[3] =
            static_cast<uint32_t>(current_pods);
        const int current_attempts =
            static_cast<int>(defending_faction.sat_odp_deployed);
        if (current_pods - current_attempts < 0) {
            attempts_spent = current_pods;
            defending_faction.sat_odp_deployed =
                static_cast<uint32_t>(current_pods);
        }
    }

    int event_base = base_find(x, y, defender);
    if (event_base < 0) event_base = base_find(x, y);
    parse_says(0, Bases[event_base].name_string, -1, -1);
    const int reported_pods =
        static_cast<int>(defending_faction.satellites_[3]);
    parse_num(0, reported_pods);
    parse_num(1, remaining_attempts);
    if (fungal_attack == 0) {
        if (defender == ConsoleGlobal.faction_id_) {
            parse_says(1, Players[attacker].adj_name_faction, -1, -1);
            popp(*ClimatePopupTitle, "TWESHOTIT", 0,
                 "space_sm.pcx", nullptr);
        } else {
            if (attacker != ConsoleGlobal.faction_id_) return 1;
            parse_says(1, Players[defender].adj_name_faction, -1, -1);
            popp(*ClimatePopupTitle, "TTHEYSHOTIT", 0,
                 "space_sm.pcx", nullptr);
        }
    } else if (defender == ConsoleGlobal.faction_id_) {
        parse_says(1, Players[attacker].adj_name_faction, -1, -1);
        popp(*ClimatePopupTitle, "FWESHOTIT", 0,
             "space_sm.pcx", nullptr);
    } else {
        if (attacker != ConsoleGlobal.faction_id_) return 1;
        parse_says(1, Players[defender].adj_name_faction, -1, -1);
        popp(*ClimatePopupTitle, "FTHEYSHOTIT", 0,
             "space_sm.pcx", nullptr);
    }
    return 1;
}

// ORIGINAL: 0x00580860 ?propose_proto@@YAHHHHHHHHPAD@Z 0x00580860-0x00581168
// RECOVERY_BINDING: v1 {"evidence": ["Pinned580860..581168:580B89 excludes predefined allocation;580B92 obsolete scoring follows compatible return;580CF5 reuses proposed offense stack value, not faction ID.", "Pinned580F98 inactive-slot flag40;5810B5/5810FA skip negative production IDs;58112D reloads inclusive queue bound; existing VehPrototype, PlayerData and Base owners.", "recovery/tasks/vehicle-special-actions-coverage.json immutable baseline and coordinator-reviewed source; retained src/unrecovered/00580860.cpp"], "reason": "Recover complete typed prototype proposal with compatibility-first selection, faction-only allocation, native similarity scoring, override staging, live prototype flags, naming and inclusive signed queue rewrites. Preserve original identity and retained seeds; naming/pruning callees remain separately unfinished and no matching or runtime acceptance is claimed."}
int __cdecl propose_proto(int faction_id, int chassis_id, int weapon_id,
                          int armor_id, int ability_flags, int reactor_id,
                          int design_category, char *requested_name) {
    const uint8_t faction_bit = static_cast<uint8_t>(1u << (faction_id & 0x1f));
    int offense_factor;
    int defense_factor;
    int best_prototype = -1;
    int best_score = 0;

    if (DesignConsiderOverride == 0) {
        if (is_bunged(faction_id, chassis_id, weapon_id, armor_id,
                      ability_flags, reactor_id)) {
            return -1;
        }
    } else if (DesignConsiderPrototype >= 0
               && DesignConsiderPrototype != design_category) {
        return -1;
    }

    if (Chassis[chassis_id].speed == 1
        && Armor[armor_id].defense_rating == 1
        && Weapon[weapon_id].offense_rating == 1
        && DesignConsiderOverride == 0) {
        return 2;
    }

    if (!ExpansionEnabled && weapon_id >= 12 && weapon_id <= 14) {
        offense_factor = 1;
    } else if (Weapon[weapon_id].offense_rating < 0) {
        const int ratio = static_cast<int>(PlayersData[faction_id].enemy_best_armor_value)
            * static_cast<int>(Rules.psi_combat_ratio_atk[TRIAD_LAND])
            / static_cast<int>(Rules.psi_combat_ratio_def[TRIAD_LAND]);
        offense_factor = psi_factor(ratio, faction_id, TRUE, FALSE);
    } else {
        offense_factor = Weapon[weapon_id].offense_rating;
    }

    if (!ExpansionEnabled && armor_id > 9) {
        defense_factor = 1;
    } else {
        if (Armor[armor_id].defense_rating < 0) {
            const int ratio = static_cast<int>(PlayersData[faction_id].enemy_best_weapon_value)
                * static_cast<int>(Rules.psi_combat_ratio_def[TRIAD_LAND])
                / static_cast<int>(Rules.psi_combat_ratio_atk[TRIAD_LAND]);
            defense_factor = psi_factor(ratio, faction_id, FALSE, FALSE);
        } else {
            defense_factor = Armor[armor_id].defense_rating;
        }
    }

    for (int walk = 0; walk < 2 * MaxVehProtoFactionNum; ++walk) {
        const int slot = (walk < MaxVehProtoFactionNum)
            ? walk : faction_id * MaxVehProtoFactionNum + (walk & 0x3f);
        VehPrototype &candidate = VehPrototypes[slot];
        int contribution = 0;
        bool has_contribution = false;
        if (slot >= MaxVehProtoFactionNum) {
            const bool is_active = (candidate.flags & PROTO_ACTIVE) != 0;
            const bool obsolete_to_faction =
                (candidate.obsolete_factions & faction_bit) != 0;
            const unsigned int active =
                PlayersData[faction_id].proto_id_active[slot];
            const unsigned int queued =
                PlayersData[faction_id].proto_id_queue[slot];

            // Native 580A11..580A71 gives obsolete, unused faction designs
            // their category score before the generic scoring path: 0x2000
            // for the same category, 0x1000 otherwise. This requires both
            // active and queued counts to be zero.
            if (is_active && obsolete_to_faction && active == 0 && queued == 0) {
                contribution = (design_category == candidate.unk_1)
                    ? 0x2000 : 0x1000;
                has_contribution = true;
            } else if (!is_active) {
                contribution = 0x800;
                has_contribution = true;
            }
        } else {
            if ((candidate.flags & PROTO_ACTIVE) == 0
                || !has_tech(candidate.preq_tech, faction_id)) {
                continue;
            }
        }

        if (!has_contribution) {
                if (candidate.chassis_id == chassis_id
                    && candidate.weapon_id == weapon_id
                    && candidate.armor_id == armor_id
                    && candidate.ability_flags == static_cast<uint32_t>(ability_flags)
                    && candidate.reactor_id == reactor_id
                    && DesignConsiderOverride == 0
                    && (LockEnableMask & faction_bit) != 0) {
                    return -1;
                }

                if (candidate.chassis_id == chassis_id
                    && candidate.weapon_id == weapon_id
                    && candidate.armor_id == armor_id
                    && candidate.reactor_id == reactor_id
                    && ((candidate.ability_flags ^ static_cast<uint32_t>(ability_flags))
                        & PrototypeAbilityCompatibilityMask) == 0
                    && ((candidate.ability_flags & static_cast<uint32_t>(ability_flags))
                        == candidate.ability_flags
                        || (LockEnableMask & faction_bit) == 0)) {
                    if (design_category == -2) {
                        candidate.flags |= PROTO_TYPED_COMPLETE;
                    }
                    if (DesignConsiderOverride != 0
                        && DesignConsiderOverride < 3) {
                        DesignConsiderAbilities = static_cast<uint32_t>(ability_flags);
                        DesignConsiderWeapon = weapon_id;
                        DesignConsiderArmor = armor_id;
                        DesignConsiderChassis = chassis_id;
                        DesignConsiderReactor = reactor_id;
                        DesignConsiderOverride = 2;
                    }
                    candidate.obsolete_factions &= static_cast<uint8_t>(~faction_bit);
                    return slot;
                }

                // Predefined designs may be reused above, but cannot be
                // chosen as allocation slots (580B89..580B8C).
                if (slot < MaxVehProtoFactionNum) continue;
                uint32_t comparison_bit = faction_bit;
                if (slot > 0x3f) {
                    // Compatibility has precedence over the obsolete score:
                    // queued obsolete designs can still be reused as-is.
                    if ((candidate.obsolete_factions & faction_bit) != 0
                        && PlayersData[faction_id].proto_id_active[slot] == 0) {
                        contribution = 0x400;
                        has_contribution = true;
                    }
                    if (!has_contribution
                        && candidate.chassis_id == chassis_id
                        && candidate.reactor_id == reactor_id
                        && ((candidate.ability_flags
                             ^ static_cast<uint32_t>(ability_flags))
                            & PrototypeAbilitySimilarityMask) == 0) {
                        const int other_weapon = candidate.weapon_id;
                        int other_offense;
                        if (!ExpansionEnabled && other_weapon >= 12
                            && other_weapon <= 14) {
                            other_offense = 1;
                        } else if (Weapon[other_weapon].offense_rating < 0) {
                            const int ratio = static_cast<int>(
                                PlayersData[faction_id].enemy_best_armor_value)
                                * static_cast<int>(Rules.psi_combat_ratio_atk[TRIAD_LAND])
                                / static_cast<int>(Rules.psi_combat_ratio_def[TRIAD_LAND]);
                            other_offense = psi_factor(ratio, faction_id, TRUE, FALSE);
                        } else {
                            other_offense = Weapon[other_weapon].offense_rating;
                        }

                        const int other_armor = candidate.armor_id;
                        int other_defense;
                        if (!ExpansionEnabled && other_armor > 9) {
                            other_defense = 1;
                        } else if (Armor[other_armor].defense_rating < 0) {
                            const int ratio = static_cast<int>(
                                PlayersData[faction_id].enemy_best_weapon_value)
                                * static_cast<int>(Rules.psi_combat_ratio_def[TRIAD_LAND])
                                / static_cast<int>(Rules.psi_combat_ratio_atk[TRIAD_LAND]);
                            other_defense = psi_factor(ratio, faction_id, FALSE, FALSE);
                        } else {
                            other_defense = Armor[other_armor].defense_rating;
                        }

                        const int highest = (other_offense > other_defense)
                            ? other_offense : other_defense;
                        const int highest_plus = (other_offense == other_defense)
                            ? highest + 1 : highest;
                        if (offense_factor < highest) {
                            int offense_distance = offense_factor - other_offense;
                            if (offense_distance < 0) offense_distance = -offense_distance;
                            if (offense_distance < 2 && defense_factor < highest_plus) {
                                int defense_distance = defense_factor - other_defense;
                                if (defense_distance < 0) defense_distance = -defense_distance;
                                if (defense_distance < 2
                                    && (comparison_bit & LockEnableMask) == 0
                                    && std::strcmp(Players[faction_id].filename, "UNIV") != 0) {
                                    return slot;
                                }
                            }
                        }
                    }
                }

                if (!has_contribution
                    && (candidate.unk_1 == design_category
                        || (PlayersData[faction_id].proto_id_active[slot] == 0
                            && PlayersData[faction_id].proto_id_queue[slot] == 0))) {
                    const unsigned int active =
                        PlayersData[faction_id].proto_id_active[slot];
                    unsigned int base_score = active == 0 ? 0x240 : 0x40;
                    if ((candidate.obsolete_factions & comparison_bit) != 0) {
                        base_score |= 0x100;
                    }
                    const unsigned int queued =
                        PlayersData[faction_id].proto_id_queue[slot];
                    if (queued == 0) base_score |= 0x80;
                    unsigned int penalty = candidate.cost * active + queued;
                    if (penalty > 0x3f) penalty = 0x3f;
                    contribution = static_cast<int>(base_score - penalty);
                    has_contribution = true;
                }
        }

        if (has_contribution && best_score < contribution) {
            best_score = contribution;
            best_prototype = slot;
        }
    }

    if (best_prototype < 0) return -1;

    if (DesignConsiderOverride != 0) {
        if (DesignConsiderOverride < 3) {
            DesignConsiderWeapon = weapon_id;
            DesignConsiderArmor = armor_id;
            DesignConsiderChassis = chassis_id;
            DesignConsiderAbilities = static_cast<uint32_t>(ability_flags);
            DesignConsiderReactor = reactor_id;
            DesignConsiderPrototype = design_category;
            DesignConsiderOverride = 3;
        }
        return -1;
    }

    VehPrototype &selected = VehPrototypes[best_prototype];
    const bool was_active = (selected.flags & PROTO_ACTIVE) != 0;
    if (was_active && !IsMultiplayerNet) {
        retire_proto(best_prototype, faction_id);
    }
    make_proto(best_prototype, chassis_id, weapon_id, armor_id,
               ability_flags, reactor_id);
    if (design_category == -2 || DiffLevelCurrent < 2) {
        selected.flags |= PROTO_TYPED_COMPLETE;
    }
    if (!was_active) selected.flags |= 0x40;
    selected.flags |= 0x80;
    if (design_category >= 0) {
        selected.unk_1 = static_cast<int8_t>(design_category);
        if (design_category == 10) selected.plan = 3;
    }

    if (requested_name == nullptr) {
        char generated_name[256];
        name_proto(generated_name, best_prototype, faction_id, chassis_id,
                   weapon_id, armor_id, ability_flags, reactor_id);
        ::strncpy(selected.veh_name, generated_name, sizeof(selected.veh_name));
        selected.veh_name[sizeof(selected.veh_name) - 1] = '\0';
        selected.flags &= static_cast<uint16_t>(~PROTO_CUSTOM_NAME_SET);
    } else {
        std::strcpy(selected.veh_name, requested_name);
        for (int other = 0; other < MaxVehProtoNum; ++other) {
            const int other_faction = other / MaxVehProtoFactionNum;
            if (other != best_prototype
                && (other_faction == 0 || other_faction == faction_id)
                && std::strcmp(selected.veh_name, VehPrototypes[other].veh_name) == 0) {
                ::strcat(selected.veh_name, "*");
                other = -1;
            }
        }
    }

    prune_protos(faction_id, best_prototype, 0);
    for (int base_id = 0; base_id < BaseCurrentCount; ++base_id) {
        Base &base = Bases[base_id];
        if (base.faction_id_current != faction_id) continue;
        const int last_queue_slot = static_cast<int>(base.queue_size);
        if (last_queue_slot < 0) continue;

        // The native loop first walks through the inclusive queue range and
        // stops at the first obsolete design in this category. It then makes
        // a second pass over that same inclusive range and rewrites every
        // matching entry (rather than only the one found by the first scan).
        for (int probe_slot = 0; probe_slot <= last_queue_slot; ++probe_slot) {
            const int queued = base.queue_production_id[probe_slot];
            if (queued < 0) continue;
            const VehPrototype &queued_prototype = VehPrototypes[queued];
            if ((queued_prototype.obsolete_factions & faction_bit) != 0
                && queued_prototype.unk_1 == selected.unk_1) {
                break;
            }
        }
        for (int queue_slot = 0;
             queue_slot <= static_cast<int>(base.queue_size); ++queue_slot) {
            const int queued = base.queue_production_id[queue_slot];
            if (queued < 0) continue;
            const VehPrototype &queued_prototype = VehPrototypes[queued];
            if ((queued_prototype.obsolete_factions & faction_bit) != 0
                && queued_prototype.unk_1 == selected.unk_1) {
                base.queue_production_id[queue_slot] = best_prototype;
            }
        }
    }

    if (FirstProposedPrototype < 0) FirstProposedPrototype = best_prototype;
    return best_prototype;
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0057F1D0 ?retire_proto@@YAXHH@Z 0x0057F1D0-0x0057F4B0
// RECOVERY_BINDING: v1 {"evidence": ["Pinned57F1D0..57F4B0: category snapshot after proto_bung_put;57F28D current-count guard;57F2B3/2C7 live full mask;57F3BA post-stack prototype reload;57F410 predefined branch bypasses active-bit clearing.", "Native57F434..47F inclusive queue walk, first-slot -69 and nonfirst shift/decrement; descending kill path preserves compacted storage. Existing typed owners in veh/base/faction/net headers.", "recovery/tasks/vehicle-special-actions-coverage.json central reviewed recovery; retained src/recovered/units/0057f1d0.cpp and immutable baseline."], "reason": "Recover complete prototype retirement with native descending-compaction traversal, live visibility mask, base/stack fallback, current prototype mineral refund and inclusive queue cleanup. Preserve source identity, full seed and original spans; this is not matching or runtime acceptance."}
void __cdecl retire_proto(int prototype_id, int faction_id) {
    proto_bung_put(faction_id, prototype_id);
    VehPrototype &retired = VehPrototypes[prototype_id];
    const uint8_t faction_bit = static_cast<uint8_t>(1u << (faction_id & 0x1f));
    const uint8_t retired_category = static_cast<uint8_t>(retired.unk_1);
    int replacement = -1;
    int replacement_score = -1;

    for (int slot = 0; slot < MaxVehProtoFactionNum; ++slot) {
        const int candidate_id = faction_id * MaxVehProtoFactionNum + slot;
        const VehPrototype &candidate = VehPrototypes[candidate_id];
        if (candidate_id == prototype_id
            || static_cast<uint8_t>(candidate.unk_1) != retired_category
            || (candidate.flags & PROTO_ACTIVE) == 0) {
            continue;
        }
        const int score = (candidate.obsolete_factions & faction_bit) == 0 ? 2 : 1;
        if (score > replacement_score) {
            replacement = candidate_id;
            replacement_score = score;
        }
    }

    for (int vehicle_id = VehCurrentCount - 1; vehicle_id >= 0; --vehicle_id) {
        if (vehicle_id >= VehCurrentCount) continue;
        Veh &vehicle = Vehs[vehicle_id];
        if (vehicle.proto_id != prototype_id) continue;

        const uint8_t enabled_factions = static_cast<uint8_t>(LockEnableMask);
        // The original locked-faction path kills this prototype's live units
        // before the visibility, map-tile, or base-stack tests.
        if ((enabled_factions & faction_bit) != 0) {
            kill(vehicle_id);
            continue;
        }

        bool may_migrate_to_replacement = replacement >= 0;
        if (may_migrate_to_replacement
            && (vehicle.visibility & enabled_factions) != 0) {
            const Map *tile = map_loc(vehicle.x, vehicle.y);
            const int tile_owner = tile->val2 & 0x0f;
            may_migrate_to_replacement = (tile->bit & BIT_BASE_IN_TILE) != 0
                && tile_owner > 0 && tile_owner < 8;
        }
        if (vehicle.faction_id != faction_id) {
            may_migrate_to_replacement = false;
        }

        if (may_migrate_to_replacement) {
            vehicle.proto_id = static_cast<int16_t>(replacement);
            --PlayersData[faction_id].proto_id_active[prototype_id];
            ++PlayersData[faction_id].proto_id_active[replacement];
            continue;
        }

        const int base_id = base_at(vehicle.x, vehicle.y);
        if (base_id >= 0
            && stack_check(vehicle_id, 1, -1, -1, -1) == 1) {
            vehicle.proto_id = 2;
            --PlayersData[faction_id].proto_id_active[prototype_id];
            ++PlayersData[faction_id].proto_id_active[2];
            continue;
        }

        if (base_id >= 0) {
            Base &base = Bases[base_id];
            const int mineral_refund =
                static_cast<int>(VehPrototypes[Vehs[vehicle_id].proto_id].cost) / 2;
            base.minerals_accumulated += mineral_refund;
            base.minerals_accumulated_2 = base.minerals_accumulated;
            base.production_id_last = base.queue_production_id[0];
        }
        kill(vehicle_id);
    }

    if (prototype_id >= MaxVehProtoFactionNum) {
        for (int base_id = 0; base_id < BaseCurrentCount; ++base_id) {
            Base &base = Bases[base_id];
            for (int queue_index = 0;
                 queue_index <= static_cast<int>(base.queue_size);
                 ++queue_index) {
                if (base.queue_production_id[queue_index] != prototype_id) {
                    continue;
                }
                if (queue_index == 0) {
                    base.queue_production_id[0] = -69;
                    continue;
                }
                for (int shift = queue_index;
                     shift < static_cast<int>(base.queue_size); ++shift) {
                    base.queue_production_id[shift] =
                        base.queue_production_id[shift + 1];
                }
                --base.queue_size;
            }
        }
        retired.flags &= static_cast<uint16_t>(~PROTO_ACTIVE);
    }
}

static int prototype_armor_strength(const VehPrototype &design,
                                    int faction_id) {
    const int armor_id = design.armor_id;
    if (!ExpansionEnabled && armor_id > 9) return 1;
    if (Armor[armor_id].defense_rating < 0) {
        const int ratio = static_cast<int>(
            PlayersData[faction_id].enemy_best_weapon_value)
            * static_cast<int>(Rules.psi_combat_ratio_def[TRIAD_LAND])
            / static_cast<int>(Rules.psi_combat_ratio_atk[TRIAD_LAND]);
        return psi_factor(ratio, faction_id, FALSE, FALSE);
    }
    return static_cast<int>(Armor[armor_id].defense_rating);
}

static void parse_prototype_report(int parse_id, int design_id) {
    StringTemp[0] = '\0';
    ::strcat(StringTemp, VehPrototypes[design_id].veh_name);
    ::strcat(StringTemp, " ");
    ::strcat(StringTemp, "(");
    say_stats_2(StringTemp, design_id);
    if (VehPrototypes[design_id].ability_flags != 0) {
        ::strcat(StringTemp, " ");
        for (int ability_id = 0; ability_id < MaxAbilityNum; ++ability_id) {
            if ((VehPrototypes[design_id].ability_flags
                 & (1u << ability_id)) == 0) continue;
            ::strcat(StringTemp, prototype_rule_name(
                Ability[ability_id].abbreviation));
        }
    }
    ::strcat(StringTemp, ")");
    parse_says(parse_id, StringTemp, -1, -1);
}

// ORIGINAL: 0x0057F4B0 ?prune_protos@@YAXHHH@Z 0x0057F4B0-0x00580859
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/0057f4b0.cpp; image:0x0057F4B0-0x00580859", "Completion propagation:0x0057F4BF-0x0057F693", "LockEnableMask separates retirement modes, unlocked epilogue returns:0x0057F699-0x0057F6AD;0x0057FCF3-0x0057FCFA", "Lower-reactor early marks:0x0057F7C8-0x0057F7DB;0x0057FA95-0x0057FAA0", "Locked replacement/target banks and non-positive armor bypass:0x0057FD13-0x005802A9;0x0057FEC2-0x0057FFC6", "Queue presence and callback-sensitive prompt/rewrite/unlock/report:0x005802AA-0x00580852"], "reason": "Recover typed completion and both mutually exclusive prototype-retirement modes in the genuine vehicle-design owner. Loaded weapon/armor/chassis rules, faction design banks and active/queued counts determine obsolescence; owned Base queues and NetDaemon locking supply production migration, with native local prompt and multiplayer report ordering. Preserve the complete seed and original spans as evidence; this structural interpretation is not a measured matching claim."}
void __cdecl prune_protos(int faction_id, int prototype_id, int options) {
    const int faction_base = faction_id * MaxVehProtoFactionNum;
    const bool can_mark_typed_completions = prototype_id < MaxVehProtoFactionNum
        || (VehPrototypes[prototype_id].flags & PROTO_TYPED_COMPLETE) != 0;
    if (can_mark_typed_completions) {
      for (int local_slot = 0; local_slot < MaxVehProtoFactionNum; ++local_slot) {
        VehPrototype &prototype = VehPrototypes[faction_base + local_slot];
        if ((prototype.flags & PROTO_ACTIVE) == 0
            || (prototype.flags & PROTO_TYPED_COMPLETE) != 0) {
            continue;
        }

        bool has_better_weapon = false;
        bool has_better_armor = false;
        bool has_better_chassis = false;
        for (int walk = 0; walk < 2 * MaxVehProtoFactionNum; ++walk) {
            const int candidate_id = (walk < MaxVehProtoFactionNum)
                ? walk : faction_base + (walk & 0x3f);
            const VehPrototype &candidate = VehPrototypes[candidate_id];
            if ((candidate.flags & PROTO_ACTIVE) == 0) continue;
            if (candidate_id < MaxVehProtoFactionNum) {
                if (!has_tech(candidate.preq_tech, faction_id)) continue;
            } else if ((candidate.flags & PROTO_TYPED_COMPLETE) == 0) {
                continue;
            }

            if (candidate.weapon_id == prototype.weapon_id) {
                has_better_weapon = true;
            }
            if (candidate.armor_id == prototype.armor_id) {
                has_better_armor = true;
            }
            if (candidate.chassis_id == prototype.chassis_id) {
                has_better_chassis = true;
            }

            const int candidate_weapon_rating =
                Weapon[candidate.weapon_id].offense_rating;
            const int proposed_weapon_rating =
                Weapon[prototype.weapon_id].offense_rating;
            if (proposed_weapon_rating > 0
                && candidate_weapon_rating >= proposed_weapon_rating) {
                has_better_weapon = true;
            }
            const int candidate_armor_rating =
                Armor[candidate.armor_id].defense_rating;
            const int proposed_armor_rating =
                Armor[prototype.armor_id].defense_rating;
            if (proposed_armor_rating > 0
                && candidate_armor_rating >= proposed_armor_rating) {
                has_better_armor = true;
            }
            if (Chassis[candidate.chassis_id].triad
                    == Chassis[prototype.chassis_id].triad
                && Chassis[candidate.chassis_id].speed
                    >= Chassis[prototype.chassis_id].speed) {
                has_better_chassis = true;
            }
        }

        if (has_better_weapon && has_better_armor && has_better_chassis) {
            prototype.flags |= static_cast<uint16_t>(PROTO_TYPED_COMPLETE | 0x100);
        }
      }
    }

    // Native 57F69C..57FCF9 processes each faction target in two ordered
    // walks: typed targets compare against the predefined bank first, then
    // every eligible target compares against its faction peers. The first
    // walk has a lower-reactor early mark; the peer walk likewise marks a
    // lower-reactor peer before later chassis, carry, rating and ability
    // checks. The cross-prototype region is skipped while this faction is
    // locked; its automatic-prune tail has a separate native entry path.
    const uint8_t faction_bit = static_cast<uint8_t>(1u << (faction_id & 0x1f));
    if ((LockEnableMask & faction_bit) == 0) {
    // The first native obsolescence walk compares faction designs with the
    // predefined bank too. Preserve its prerequisite and unsupported-weapon
    // exclusions before applying the exact-equivalence subcase.
    for (int target_slot = 0; target_slot < MaxVehProtoFactionNum; ++target_slot) {
        const int target_id = faction_base + target_slot;
        const VehPrototype &target = VehPrototypes[target_id];
        if ((target.flags & PROTO_ACTIVE) == 0
            || (target.obsolete_factions & faction_bit) != 0) {
            continue;
        }
        if (target_id < MaxVehProtoFactionNum
            && !has_tech(target.preq_tech, faction_id)) {
            continue;
        }
        // Native 57F81A sends an untyped faction target directly to the
        // faction-prototype pass. Predefined targets pass directly; faction
        // targets require the typed-complete flag.
        const bool compare_typed_peers = target_id < MaxVehProtoFactionNum
            || (target.flags & PROTO_TYPED_COMPLETE) != 0;
        if (compare_typed_peers) {
        for (int candidate_id = 0; candidate_id < MaxVehProtoFactionNum;
             ++candidate_id) {
            VehPrototype &candidate = VehPrototypes[candidate_id];
            if (candidate_id == target_id || candidate_id == 15
                || (candidate.flags & PROTO_ACTIVE) == 0
                || Weapon[candidate.weapon_id].offense_rating < 0
                || candidate.plan != target.plan
                || candidate.unk_1 != target.unk_1
                || Chassis[candidate.chassis_id].triad
                    != Chassis[target.chassis_id].triad) {
                continue;
            }
            // Native 57F7C8..57F9D2 marks a lower-reactor, non-psi
            // predefined design immediately after the triad check. This
            // dominance branch intentionally precedes range, carry, rating,
            // and ability comparisons.
            if (candidate.reactor_id < target.reactor_id) {
                candidate.obsolete_factions |= faction_bit;
                if (options != 0) synch_obs(candidate_id);
                continue;
            }
            if (Chassis[candidate.chassis_id].range
                    != Chassis[target.chassis_id].range
                || Chassis[candidate.chassis_id].speed
                    > Chassis[target.chassis_id].speed
                || candidate.carry_capacity > target.carry_capacity
                || bit_count(static_cast<int>(candidate.ability_flags
                                               & target.ability_flags))
                    < bit_count(static_cast<int>(candidate.ability_flags))
                || ((candidate.ability_flags ^ target.ability_flags)
                    & 0x00408418u) != 0) {
                continue;
            }
            const int candidate_offense =
                Weapon[candidate.weapon_id].offense_rating;
            const int target_offense = Weapon[target.weapon_id].offense_rating;
            const bool equivalent_weapon = candidate_offense > 0
                ? candidate_offense <= target_offense
                : candidate.weapon_id == target.weapon_id;
            const int candidate_defense =
                Armor[candidate.armor_id].defense_rating;
            const bool equivalent_armor = candidate_defense > 0
                ? 2 * prototype_armor_strength(candidate, faction_id)
                    <= 2 * prototype_armor_strength(target, faction_id)
                : candidate.armor_id == target.armor_id;
            if (!equivalent_weapon || !equivalent_armor) continue;
            candidate.obsolete_factions |= faction_bit;
            if (options != 0) synch_obs(candidate_id);
        }
        }

        // Native peer comparisons follow the predefined scan for this same
        // target, before the outer target index advances.
        for (int peer_slot = 0; peer_slot < MaxVehProtoFactionNum; ++peer_slot) {
            if (peer_slot == target_slot) continue;
            VehPrototype &peer = VehPrototypes[faction_base + peer_slot];
            if ((peer.flags & PROTO_ACTIVE) == 0
                || peer.plan != target.plan
                || peer.unk_1 != target.unk_1
                || Chassis[peer.chassis_id].triad
                    != Chassis[target.chassis_id].triad
                || (!compare_typed_peers
                    && (peer.flags & PROTO_TYPED_COMPLETE) != 0)) {
                continue;
            }
            if (peer.reactor_id < target.reactor_id) {
                peer.obsolete_factions |= faction_bit;
                if (options != 0) synch_obs(faction_base + peer_slot);
                continue;
            }
            if (Chassis[peer.chassis_id].range
                    != Chassis[target.chassis_id].range
                || Chassis[peer.chassis_id].speed
                    > Chassis[target.chassis_id].speed
                || peer.carry_capacity > target.carry_capacity) {
                continue;
            }
            if (((peer.flags & PROTO_TYPED_COMPLETE) != 0
                 && bit_count(static_cast<int>(peer.ability_flags
                                              & target.ability_flags))
                    < bit_count(static_cast<int>(peer.ability_flags)))
                || ((peer.ability_flags ^ target.ability_flags)
                    & 0x00408418u) != 0) {
                continue;
            }
            const int peer_offense = Weapon[peer.weapon_id].offense_rating;
            const int target_offense = Weapon[target.weapon_id].offense_rating;
            const bool equivalent_weapon = peer_offense > 0
                ? peer_offense <= target_offense
                : peer.weapon_id == target.weapon_id;
            const int peer_defense = Armor[peer.armor_id].defense_rating;
            const bool equivalent_armor = peer_defense > 0
                ? 2 * prototype_armor_strength(peer, faction_id)
                    <= 2 * prototype_armor_strength(target, faction_id)
                : peer.armor_id == target.armor_id;
            if (!equivalent_weapon || !equivalent_armor) continue;
            peer.obsolete_factions |= faction_bit;
            if (options != 0) synch_obs(faction_base + peer_slot);
        }
    }
        // This mode returns at 57FCF3 even if a synchronization callback
        // changes LockEnableMask; it cannot enter the automatic mode.
        return;
    }

    // Native 57FD13..5802A9 is reachable from the locked-faction branch and
    // additionally requires multiplayer or the single-player preference.
    // The outer design is the possible replacement; the inner design is the
    // design that receives the faction's obsolete bit.
    if ((LockEnableMask & faction_bit) != 0
        && (IsMultiplayerNet != 0
            || (GameMorePreferences & MPREF_BSC_AUTO_PRUNE_OBS_VEH) != 0)) {
        for (int replacement_walk = 0;
             replacement_walk < 2 * MaxVehProtoFactionNum;
             ++replacement_walk) {
            const int replacement_id = replacement_walk < MaxVehProtoFactionNum
                ? replacement_walk
                : faction_base + (replacement_walk & 0x3f);
            const VehPrototype &replacement = VehPrototypes[replacement_id];
            if (replacement_id < MaxVehProtoFactionNum
                && !has_tech(replacement.preq_tech, faction_id)) {
                continue;
            }
            if ((replacement.flags & PROTO_ACTIVE) == 0
                || (replacement.obsolete_factions & faction_bit) != 0) {
                continue;
            }

            for (int target_walk = 0;
                 target_walk < 2 * MaxVehProtoFactionNum;
                 ++target_walk) {
                const int target_id = target_walk < MaxVehProtoFactionNum
                    ? target_walk : faction_base + (target_walk & 0x3f);
                if (target_walk == replacement_walk) continue;
                VehPrototype &target = VehPrototypes[target_id];
                if ((target.flags & PROTO_ACTIVE) == 0
                    || (target.obsolete_factions & faction_bit) != 0
                    || (target.flags & 0x0200) != 0
                    || (target_id < MaxVehProtoFactionNum
                        && (target_id == 15
                            || Weapon[target.weapon_id].offense_rating < 0))) {
                    continue;
                }
                // Native 57FE2C..FE51 allows predefined replacements and
                // typed-complete faction replacements through directly. An
                // untyped faction replacement is limited to untyped faction
                // targets; it does not compare against predefined targets.
                if (replacement_id >= MaxVehProtoFactionNum
                    && (replacement.flags & PROTO_TYPED_COMPLETE) == 0
                    && (target_id < MaxVehProtoFactionNum
                        || (target.flags & PROTO_TYPED_COMPLETE) != 0)) {
                    continue;
                }

                if (replacement.plan != target.plan
                    || Chassis[target.chassis_id].range
                        != Chassis[VehPrototypes[prototype_id].chassis_id].range
                    || Chassis[replacement.chassis_id].triad
                        != Chassis[target.chassis_id].triad
                    || Chassis[target.chassis_id].speed
                        > Chassis[replacement.chassis_id].speed
                    || target.carry_capacity > replacement.carry_capacity
                    || ((target.flags & 0x0008) != 0
                        && target.reactor_id >= replacement.reactor_id)) {
                    continue;
                }

                if (target.plan != 3) {
                    if ((Weapon[replacement.weapon_id].offense_rating > 1)
                            != (Weapon[target.weapon_id].offense_rating > 1)
                        || (Armor[replacement.armor_id].defense_rating > 1)
                            != (Armor[target.armor_id].defense_rating > 1)) {
                        continue;
                    }
                }
                if ((Weapon[target.weapon_id].offense_rating <= 0
                     && replacement.weapon_id != target.weapon_id)
                    || (Weapon[target.weapon_id].offense_rating > 0
                        && Weapon[target.weapon_id].offense_rating
                            > Weapon[replacement.weapon_id].offense_rating)
                    || (Armor[target.armor_id].defense_rating <= 0
                        && replacement.armor_id != target.armor_id)) {
                    continue;
                }
                if ((Armor[target.armor_id].defense_rating > 0
                     && 2 * prototype_armor_strength(target, faction_id)
                        > 2 * prototype_armor_strength(replacement, faction_id))
                    || target.reactor_id > replacement.reactor_id) {
                    continue;
                }

                const uint32_t ability_difference =
                    replacement.ability_flags ^ target.ability_flags;
                if ((ability_difference & 0xfffbd5fcu) != 0) continue;
                if (ability_difference != 0) {
                    const bool replacement_covers_target =
                        (replacement.ability_flags & target.ability_flags)
                            == target.ability_flags;
                    if (target.reactor_id >= replacement.reactor_id
                        && !replacement_covers_target) {
                        if (replacement_id < MaxVehProtoFactionNum
                            || PlayersData[faction_id]
                                .proto_id_active[target_id] != 0
                            || PlayersData[faction_id]
                                .proto_id_queue[target_id] != 0) {
                            continue;
                        }
                    }
                }
                if ((target.flags & 0x0008) != 0
                    && (replacement.unk_1 != target.unk_1
                        || (replacement.ability_flags & target.ability_flags)
                            != target.ability_flags)) {
                    continue;
                }

                // Without a reactor advantage, a more expensive replacement
                // cannot retire a predefined or flag-8 target.
                if (target.cost < replacement.cost
                    && target.reactor_id >= replacement.reactor_id
                    && (target_id < MaxVehProtoFactionNum
                        || (target.flags & 0x0008) != 0)) {
                    continue;
                }

                if ((target_id < MaxVehProtoFactionNum
                     || replacement_id < MaxVehProtoFactionNum)
                    && target.chassis_id == replacement.chassis_id
                    && target.weapon_id == replacement.weapon_id
                    && target.armor_id == replacement.armor_id
                    && target.reactor_id == replacement.reactor_id) {
                    continue;
                }

                if (target_id == prototype_id) {
                    target.flags |= 0x0008;
                    if (options != 0) synch_obs(target_id);
                    continue;
                }
                target.obsolete_factions |= faction_bit;
                if (options != 0) synch_obs(target_id);
                if (target_id < MaxVehProtoFactionNum
                    && !has_tech(target.preq_tech, faction_id)) {
                    continue;
                }

                // The native affected-base walk rewrites every queued
                // reference to the now-obsolete design to its replacement.
                bool retained_obsolete_target = false;
                bool upgrade_choice_latch = false;
                bool processed_any_owned_base = false;
                bool has_queued_obsolete_prototype = false;
                for (int queued_base_id = 0;
                     queued_base_id < BaseCurrentCount; ++queued_base_id) {
                    const Base &base = Bases[queued_base_id];
                    if (base.faction_id_current != faction_id) continue;
                    const int queue_size =
                        static_cast<int32_t>(base.queue_size);
                    if (queue_size < 0) continue;
                    for (int queue_slot = 0; queue_slot <= queue_size;
                         ++queue_slot) {
                        if (base.queue_production_id[queue_slot] == target_id) {
                            has_queued_obsolete_prototype = true;
                            break;
                        }
                    }
                    if (has_queued_obsolete_prototype) break;
                }
                if (!has_queued_obsolete_prototype) continue;
                for (int base_id = 0; base_id < BaseCurrentCount; ++base_id) {
                    Base &base = Bases[base_id];
                    if (base.faction_id_current != faction_id) continue;

                    int production_upgrade_choice = 0;
                    bool parse_local_reports = IsMultiplayerNet == 0
                        && faction_id == ConsoleGlobal.faction_id_
                        && !processed_any_owned_base;
                    if (!parse_local_reports && options != 0) {
                        if (NetDaemonNet.lock_base(base_id, 0, -1, -1) != 0) {
                            continue;
                        }
                        // Native rechecks live multiplayer/local/processed
                        // state after locking before choosing the parse path.
                        parse_local_reports = IsMultiplayerNet == 0
                            && faction_id == ConsoleGlobal.faction_id_
                            && !processed_any_owned_base;
                    }
                    if (parse_local_reports) {
                        // The first local single-player base publishes both
                        // reports before consulting the live manual flag.
                        parse_prototype_report(0, target_id);
                        parse_prototype_report(1, replacement_id);
                        if ((target.flags & 0x0200) == 0
                            && !upgrade_choice_latch) {
                            production_upgrade_choice =
                                X_pop("ASKPRODUPGRADE", 0);
                            if (production_upgrade_choice > 1) {
                                upgrade_choice_latch = true;
                            }
                        }
                    }
                    // Reload after report/prompt callbacks; manual retention
                    // belongs only to this local parse path.
                    if (parse_local_reports
                        && production_upgrade_choice <= 1
                        && ((target.flags & 0x0200) != 0
                            || production_upgrade_choice == 1)) {
                        target.obsolete_factions &=
                            static_cast<uint8_t>(~faction_bit);
                        if ((target.flags & 0x0008) != 0) {
                            target.flags |= 0x0200;
                        }
                        target.flags |= 0x0008;
                        retained_obsolete_target = true;
                        break;
                    }

                    processed_any_owned_base = true;
                    const int queue_size =
                        static_cast<int32_t>(base.queue_size);
                    if (queue_size >= 0) {
                        for (int queue_slot = 0; queue_slot <= queue_size;
                             ++queue_slot) {
                            if (base.queue_production_id[queue_slot] == target_id) {
                                base.queue_production_id[queue_slot] = replacement_id;
                            }
                        }
                    }
                    if (IsMultiplayerNet != 0 && options != 0) {
                        NetDaemonNet.unlock_base(base_id);
                    }
                }

                if (retained_obsolete_target) continue;
                if (IsMultiplayerNet != 0 && processed_any_owned_base
                    && faction_id == ConsoleGlobal.faction_id_) {
                    parse_prototype_report(0, target_id);
                    parse_prototype_report(1, replacement_id);
                    const char *picture =
                        (Players[faction_id].rule_flags & RFLAG_ALIEN) != 0
                            ? "alunit_sm.pcx" : "unit_sm.pcx";
                    popp(ScriptPopupFile, "PRODUPGRADE", 0, picture, 0);
                }
            }
        }
    }

}


static int veh_init_i32(uint32_t bits) {
    if (bits <= 0x7FFFFFFFu) return static_cast<int>(bits);
    return -1 - static_cast<int>(~bits);
}

static int veh_init_add32(int left, int right) {
    return veh_init_i32(static_cast<uint32_t>(left) +
                        static_cast<uint32_t>(right));
}

static int veh_init_sub32(int left, int right) {
    return veh_init_i32(static_cast<uint32_t>(left) -
                        static_cast<uint32_t>(right));
}

static int veh_init_sar32(int value, unsigned int count) {
    const uint32_t bits = static_cast<uint32_t>(value);
    if (value >= 0) return static_cast<int>(bits >> count);
    return veh_init_i32(~((~bits) >> count));
}

static int veh_init_abs32(int value) {
    return value < 0
        ? veh_init_i32(0u - static_cast<uint32_t>(value))
        : value;
}

// The superseded raw-address scaffold was deleted after full owner recovery.
// Its seed path below is historical Git provenance, not a source dependency.
// ORIGINAL: 0x005C03D0 ?veh_init@@YAHHHHH@Z 0x005C03D0-0x005C08C0
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/005c03d0.cpp; recovery/tasks/base-colony-economy-coverage-wave.json; recovery/tests/test_base_colony_production.py", "Pinned01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf full389instructions5C03D0..5C08C0.", "Native5C04C6/5C04D0 invalidate BaseComputedID, not BaseIDCurrentSelected;5C05D6 scores the current faction region count, not the rival count.", "Native5C06E8 uses signed minimum after wrapped threshold doubling/division;5C07B0 immediately returns when the probe-allocation bit is already set."], "reason": "Recover complete typed Vehicle creation, home-base assignment and target-region allocation, replacing only its executable-address forwarder. Preserve original seed and historical obligations; no new byte, linked-preservation or runtime acceptance claim."}
// symbol    ?veh_init@@YAHHHHH@Z
// TRIED: direct Ghidra transcription with own extern DAT_ table arrays (byte/dword typed per bracket-index width) instead of the given g_ address pointers; diverges at insn #5, plausibly register-allocation shape of the ~200-line body
int __cdecl veh_init(int prototype, int faction, int x, int y) {
    const int vehicle_id = VehCurrentCount;
    if (vehicle_id > 0x7FF) return -1;
    VehCurrentCount = vehicle_id + 1;

    log_say("Adding Vehicle (v,who,v2)", vehicle_id, faction, prototype);

    if (VehPrototypes[prototype].plan < 8) {
        PlayersData[faction].total_mil_veh =
            veh_init_add32(PlayersData[faction].total_mil_veh, 1);
    }
    PlayersData[faction].proto_id_active[prototype] = static_cast<uint8_t>(
        PlayersData[faction].proto_id_active[prototype] + 1);

    veh_clear(vehicle_id, prototype, faction);
    const int base_id = base_find(x, y);
    if (VehPrototypes[prototype].plan == PLAN_ALIEN_ARTIFACT) {
        if (GameState & 0x40000) {
            Vehs[vehicle_id].flags |= VFLAG_IS_OBJECTIVE;
        }
    } else if (base_id >= 0 &&
               Bases[base_id].faction_id_current == faction) {
        Vehs[vehicle_id].home_base_id = static_cast<int16_t>(base_id);
        if (BaseComputedID == base_id) BaseComputedID = -1;
    }

    veh_drop(vehicle_id, x, y);
    spot_all(vehicle_id, 1);

    if (PlayersData[faction].x_target < 0) return vehicle_id;

    const Map *tile = map_tiles() + (x >> 1) + y * MapLongitude;
    if ((tile->climate & 0xE0) <= 0x5F) return vehicle_id;

    const int initialized_prototype = Vehs[vehicle_id].proto_id;
    const int initialized_plan = VehPrototypes[initialized_prototype].plan;
    if (initialized_plan >= 3 && initialized_plan != 11) return vehicle_id;

    const uint8_t region = tile->region;
    if (map_tiles()[PlayersData[faction].y_target * MapLongitude +
                    (PlayersData[faction].x_target >> 1)].region != region) {
        return vehicle_id;
    }

    int other_faction = -1;
    uint8_t largest_region_base_count = 0;
    for (unsigned int candidate_faction = 1; candidate_faction < 8;
         ++candidate_faction) {
        if (candidate_faction != static_cast<unsigned int>(faction) &&
            (PlayersData[faction].diplo_treaties[candidate_faction] &
             DTREATY_VENDETTA) &&
            largest_region_base_count <
                PlayersData[faction].region_total_bases[region]) {
            other_faction = static_cast<int>(candidate_faction);
            largest_region_base_count =
                PlayersData[faction].region_total_bases[region];
        }
    }

    if (other_faction >= 0) {
        base_find(x, y, other_faction, region, -1, -1);
        int dx = veh_init_abs32(veh_init_sub32(
            x, PlayersData[faction].x_target));
        if (!(MapIsFlat & 1) && static_cast<int>(MapLongitude) < dx) {
            dx = veh_init_sub32(static_cast<int>(MapLongitudeBounds), dx);
        }
        const int dy = veh_init_abs32(veh_init_sub32(
            y, PlayersData[faction].y_target));
        const int abs_dx = veh_init_abs32(dx);
        const int abs_dy = veh_init_abs32(dy);
        const int maximum = abs_dx <= abs_dy ? abs_dy : abs_dx;
        const int minimum = abs_dy <= abs_dx ? abs_dy : abs_dx;
        const int half_sum = veh_init_sar32(
            veh_init_add32(abs_dx, abs_dy), 1);
        const int correction = veh_init_sar32(
            veh_init_add32(veh_init_sub32(half_sum, minimum), 1), 1);
        const int hex_distance = veh_init_sub32(maximum, correction);
        if (BaseFindDist < hex_distance) return vehicle_id;
    }

    int spread_limit =
        (static_cast<uint32_t>(PlayersData[faction].region_total_bases[region])
         << 1) / 3;
    if (spread_limit < 3) spread_limit = 3;
    else if (spread_limit > 8) spread_limit = 8;

    if (PlayersData[faction].region_base_plan[region] == 7) {
        const int doubled_threshold = veh_init_add32(
            PlayersData[faction].unk_70, PlayersData[faction].unk_70);
        const uint32_t correction =
            static_cast<uint32_t>(veh_init_sar32(doubled_threshold, 31)) &
            0x3FFFFF;
        const int faction_limit = veh_init_sar32(
            veh_init_add32(doubled_threshold, static_cast<int>(correction)), 22);
        if (faction_limit <= spread_limit) spread_limit = faction_limit;
    }

    const int bit_count_value = bit_count(PlayersData[faction].unk_72);
    if (veh_init_sub32(PlayersData[faction].unk_71, bit_count_value) <
        static_cast<int>(spread_limit)) {
        const int weapon_value = weap_strat(
            VehPrototypes[Vehs[vehicle_id].proto_id].weapon_id, faction);
        const int armor_value = arm_strat(
            VehPrototypes[Vehs[vehicle_id].proto_id].armor_id, faction);
        const int chassis_id =
            VehPrototypes[Vehs[vehicle_id].proto_id].chassis_id;
        if (Chassis[chassis_id].triad == 0 &&
            !has_abil(Vehs[vehicle_id].proto_id, 0x10)) {
            if (VehPrototypes[Vehs[vehicle_id].proto_id].plan ==
                PLAN_INFO_WARFARE) {
                if (PlayersData[faction].unk_72 & 2) return vehicle_id;
                PlayersData[faction].unk_71 =
                    veh_init_add32(PlayersData[faction].unk_71, 1);
                PlayersData[faction].unk_72 |= 2;
                Vehs[vehicle_id].state |= VSTATE_UNK_40000;
                return vehicle_id;
            } else if (weapon_value < armor_value) {
                if (PlayersData[faction].unk_72 & 1) return vehicle_id;
                if (PlayersData[faction].region_base_plan[region] == 7) {
                    return vehicle_id;
                }
            } else if (PlayersData[faction].region_base_plan[region] != 7) {
                uint32_t random_limit =
                    (static_cast<uint32_t>(
                         PlayersData[faction].region_total_bases[region]) << 1) /
                    3;
                if (random_limit < 3) random_limit = 3;
                else if (random_limit > 8) random_limit = 8;
                if (rand() % static_cast<int>(random_limit + 2) == 0) {
                    Vehs[vehicle_id].state |= VSTATE_UNK_20000;
                    return vehicle_id;
                }
            }

            PlayersData[faction].unk_71 =
                veh_init_add32(PlayersData[faction].unk_71, 1);
            Vehs[vehicle_id].state |= VSTATE_UNK_40000;
            if (weapon_value < armor_value) PlayersData[faction].unk_72 |= 1;
        }
    }

    return vehicle_id;
}

// ORIGINAL: 0x0057CFA0 ?study_artifact@@YAHH@Z 0x0057CFA0-0x0057D1E8
// RECOVERY_BINDING: v1 {"reason":"Consume the actual vehicle after snapshotting its location/faction, preserve the translator-base bypass and artifact link state, then choose eligible technology through genuine faction/Technology owners. No byte/reference or runtime claim is granted.","evidence":["git:0de05de6:src/unrecovered/0057cfa0.cpp","image:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf","recovery/tasks/scenario-artifact-unity-owner-coverage.json","Original full span and native physical COFF emitted symbol ?study_artifact@@YAHH@Z; existing source owns all referenced storage and services."]}
// TRIED: direct pointer-cast field reads/multiply-based offsets instead of the lea-chain scaling; diverges at instr #3 (prologue push/save order)
int __cdecl study_artifact(int veh_id) {
    char message[80];
    message[0] = 0;
    strcat(message, "ARTIFACTTECH");

    Veh &vehicle = Vehs[veh_id];
    const int tile_x = vehicle.x;
    const int tile_y = vehicle.y;
    const int faction_id = vehicle.faction_id;
    const int base_id = base_at(tile_x, tile_y);
    if (base_id != SecretProject.universal_translator && base_id >= 0) {
        Base &base = Bases[base_id];
        if ((base.state & BSTATE_ARTIFACT_LINKED) != 0) {
            if ((base.state & BSTATE_ARTIFACT_ALREADY_LINKED) == 0
                && faction_id == ConsoleGlobal.faction_id_) {
                parse_says(0, base.name_string, -1, -1);
                NetMsgPrimary.pop("ALREADYARTIFACT", -5000, 0,
                                  "art_dis_sm.pcx");
                base.state |= BSTATE_ARTIFACT_ALREADY_LINKED;
            }
            return 0;
        }
    }

    veh_kill(veh_id);
    draw_tile(tile_x, tile_y, 2);

    if (faction_id == ConsoleGlobal.faction_id_) {
        g_FX.play(0x23);
        popp(*ClimatePopupTitle, message, 0, "art_dis_sm.pcx", 0);
    }

    PlayerData &faction_data = PlayersData[faction_id];
    if ((faction_data.flags & 0x10) != 0) {
        faction_data.flags |= 0x4;
    } else {
        faction_data.flags |= 0x10;
    }

    if (base_id >= 0) {
        Bases[base_id].state |= BSTATE_ARTIFACT_LINKED;
    }

    int best_score = -1;
    int best_tech = -1;
    for (int tech_id = 0; tech_id < MaxTechnologyNum; ++tech_id) {
        if (!has_tech(tech_id, faction_id)
            && tech_avail(tech_id, faction_id)) {
            const int prereq_1 = Technology[tech_id].preq_tech_1;
            const int prereq_2 = Technology[tech_id].preq_tech_2;
            const BOOL has_prereq_1 = has_tech(prereq_1, faction_id);
            const BOOL has_prereq_2 = has_tech(prereq_2, faction_id);
            if (has_prereq_1 || has_prereq_2) {
                int weight = 8;
                if (!has_prereq_1 || !has_prereq_2) {
                    if (!valid_tech_leap(tech_id, faction_id)) {
                        continue;
                    }
                    weight = (11 - faction_data.ranking) * 2;
                }
                const int roll = (weight > 1) ? rand() % weight : 0;
                if (best_score < roll) {
                    best_tech = tech_id;
                    best_score = roll;
                }
            }
        }
    }

    if (best_tech >= 0) {
        tech_achieved(faction_id, best_tech, 0, 0);
    }
    return 1;
}
