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
    return StringTable.get(string_id);
}

static const char *prototype_reactor_name(int reactor_id) {
    if (reactor_id == 0) {
        // Preserve the original reactor-0 alias to palette storage preceding
        // Reactor[0]. Strings::get only range-checks this borrowed address;
        // it is not claimed to be text and is not dereferenced here.
        return StringTable.get(const_cast<char *>(static_cast<const char *>(
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

static const char *prototype_collision_name_bytes(int candidate_id) {
    if (candidate_id == -1) {
        // The native negative candidate starts at Ability[27].abbreviation's
        // object-representation bytes and continues through later fields and
        // Ability[28]. Derive that view from the complete array object, not
        // from a pointer to the member; keep strcmp's original unbounded read.
        const unsigned char *const ability_bytes =
            reinterpret_cast<const unsigned char *>(&Ability);
        const size_t name_offset =
            27 * sizeof Ability[0] + offsetof(RulesAbility, abbreviation);
        return reinterpret_cast<const char *>(ability_bytes + name_offset);
    }
    return VehPrototypes[candidate_id].veh_name;
}

static void apply_prototype_name_collisions(
    char *output, char *alternate_name, int prototype_id, int faction_id,
    int chassis_id, int weapon_id, int armor_id, int ability_flags,
    int reactor_id, int primary_alternate_difference, int &gendered_form,
    const char *duplicate_suffix) {
    bool suffix_exhausted = false;
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
            // The original resets ESI before its alternate comparison even
            // when primary and alternate forms differ.
            candidate_id = -1;
        }
        if (primary_alternate_difference != 0
            && std::strcmp(alternate_name + 1,
                           prototype_collision_name_bytes(candidate_id)) == 0) {
            gendered_form = 0;
            add_prototype_name_suffix(alternate_name + 1, duplicate_suffix,
                                      suffix_exhausted);
            candidate_id = -1;
        }
    }
}


// ORIGINAL: 0x0057E040 ?name_proto@@YAXPADHHHHHHH@Z 0x0057E040-0x0057EF95
// RECOVERY_BINDING: v1 {"evidence": ["git:2f89503ec251ffc8b9ff894998c01944c173a4fb:src/unrecovered/0057e040.cpp;image:0x0057E040-0x0057EF95", "pinned image:0x0057EE10 or esi,-1;0x0057EE13 test edi,edi;0x0057EE1B..0x0057EE35 builds VehPrototypes[-1].veh_name and calls strcmp", "pinned image:0x0057EF67..0x0057EF71 resets ESI after a matched alternate suffix and increments/restarts at zero", "Owned accidental storage:Ability base0x009AB538,stride0x1C,abbreviation+8 =>0x009AB834 = VehPrototypes base0x009AB868-0x34;source:src/veh.h RulesAbility;source:src/alpha.cpp ability loader", "git:2f89503ec251ffc8b9ff894998c01944c173a4fb:src/unrecovered/0057e040.cpp"], "reason": "Complete the original duplicate-name candidate loop, including its unconditional primary-collision restart before the differing-form alternate compare and its alternate-collision restart. The -1 candidate name view points to the source-owned object representation of Ability[27].abbreviation, preserving the native pointer-field-byte comparison instead of resolving the pointed-to rule string. The original unbounded strcmp read extent beyond that member remains unresolved; preserve its unsafe read/overflow behavior as an obligation. Full span and prior source/history remain intact; no byte-exact or runtime acceptance is claimed."}

// TRIED: near-direct Ghidra transcription (this decompile had no extraout/jumptable warnings); table strides and Strings::get call sites were cross-checked against the disassembly, but the exact if/else vs goto shape of the two long fallback chains was not re-derived instruction-by-instruction, so byte-exactness is unlikely even though structure matches.
void __cdecl name_proto(char *output, int prototype_id, int faction_id,
                        int chassis_id, int weapon_id, int armor_id,
                        int ability_flags, int reactor_id) {
    output[0] = '\0';
    for (int ability_id = 0; ability_id < MaxAbilityNum; ++ability_id) {
        if ((static_cast<uint32_t>(ability_flags) & (1u << ability_id)) == 0) {
            continue;
        }
        const char *abbreviation = StringTable.get(
            Ability[ability_id].abbreviation);
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
        abbreviation = StringTable.get(Ability[ability_id].abbreviation);
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
        const char *large_offense_form = StringTable.get(chassis.offsv_lrg_name);
        bool has_paired_large_forms = false;
        if (strlen(large_offense_form) != 0) {
            const char *large_defense_form = StringTable.get(chassis.defsv_lrg_name);
            has_paired_large_forms = strlen(large_defense_form) != 0;
        }
        if (!has_paired_large_forms) name_form_quality = 0;
    }

    const int weapon_rating = weapon.offense_rating;
    bool has_gendered_name_branch = false;
    int gendered_form = (reactor_id - 1 ^ faction_id) & 1;
    const VehPrototype &named_prototype = VehPrototypes[prototype_id];
    if (named_prototype.unk_1 == 10
        && (GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & (1u << (faction_id & 0x1f))) == 0
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
    const char *duplicate_suffix = label_get(0xe5c / 4);
    apply_prototype_name_collisions(
        output, alternate_name, prototype_id, faction_id, chassis_id,
        weapon_id, armor_id, ability_flags, reactor_id,
        primary_alternate_difference, gendered_form, duplicate_suffix);

    if (gendered_form != 0) {
        ::strcpy(output, alternate_name + 1);
    }
}

// 0x009A64C8
// RECOVERY_OBJECT: v1 0x009B2280 ?VehDropLiftVehID@@3HA
int VehDropLiftVehID;  // 0x009B2280
// RECOVERY_OBJECT: v1 0x009B2278 ?VehLiftX@@3HA
int VehLiftX;  // 0x009B2278
// RECOVERY_OBJECT: v1 0x009B2284 ?VehLiftY@@3HA
int VehLiftY;  // 0x009B2284
BOOL VehBitError;  // 0x009B228C
int VehMoraleModifierCount; // only used by say_morale(), optimize to local var?

// ===== MANAGED GLOBALS - the veh.h tables, homed here =====
// Zero initialisation IS the image's initialisation: every base below reads
// ZERO at run time (`image_data.py`, all past the file-backed end of .data),
// and the alpha.txt / x.txt loaders fill them before use.
// The table ends where the next mapped object, ExitTurnLoop at 0x009B2068,
// begins: 0x009AB868 + 512 * 0x34 = 0x009B2068.
// RECOVERY_OBJECT: v2 0x009AB868 0x009B2068 ?VehPrototypes@@3PAUVehPrototype@@A 0x00493AF0
VehPrototype VehPrototypes[MaxVehProtoNum];  // 0x009AB868
// The rules loader fills entries0..8; original load_vehicles45446C..4546B4
// additionally populates sprite metrics in entries9..18 at the same0x90 stride.
// The full table ends at Natural's independent start0x0094ADE0.
// RECOVERY_OBJECT: v2 0x0094A330 0x0094ADE0 ?Chassis@@3PAURulesChassis@@A 0x00454270
RulesChassis Chassis[ChassisStorageCount];  // 0x0094A330
// Bound: 26 x 0x10 records; loop ends at `cmp esi, 0x94b000` (0x0044BE13), SocialCategories' start.
// RECOVERY_OBJECT: v2 0x0094AE60 0x0094B000 ?Weapon@@3PAURulesWeapon@@A 0x0044BE13
RulesWeapon Weapon[MaxWeaponNum];  // 0x0094AE60
// Bound: 14 x 0x10 records; loop ends at `cmp esi, 0x94f358` (0x0044BFE0), Technology's start.
// RECOVERY_OBJECT: v2 0x0094F278 0x0094F358 ?Armor@@3PAURulesArmor@@A 0x0044BFE0
RulesArmor Armor[MaxArmorNum];  // 0x0094F278
// Ability-model assembly reads the signed prerequisite at record +0x18;
// the original indexes these existing rules with a 0x1C-byte stride.
// Bound: 29 x 0x1C records; loops end at `cmp edi, 0x9ab864` (0x00485641).
// RECOVERY_OBJECT: v2 0x009AB538 0x009AB864 ?Ability@@3PAURulesAbility@@A 0x00485641
RulesAbility Ability[MaxAbilityNum];  // 0x009AB538
// RECOVERY_OBJECT: v1 0x00952328 ?Morale@@3PAURulesMorale@@A
RulesMorale Morale[MaxMoraleNum];  // 0x00952328
RulesCombatMode DefenseModes[MaxDefenseModeNum];  // 0x00946A00
RulesCombatMode OffenseModes[MaxOffenseModeNum];  // 0x00946178
RulesOrder Order[MaxOrderNum];  // 0x0096C878
LPSTR PlansShortName[MaxPlanNum];  // 0x00945FE0
LPSTR PlansFullName[MaxPlanNum];  // 0x00952360
LPSTR Triad[MaxTriadNum];  // 0x0094F1A8

// Signed preference: negative/default/positive select slow/normal/fast.
// The independent INI loader 0x00588AA0 names it FastUnitAnim.
// RECOVERY_OBJECT: v2 0x0094581C 0x00945820 ?FastUnitAnim@@3HA 0x0055A150
int FastUnitAnim;


// Shared infantry/native model placement, original 687AE8..687AF4.
// This is distinct from the four offsets derived from loaded voxel parts.
// RECOVERY_OBJECT: v2 0x00687AE8 0x00687AF4 ?VehInfantryOffset@@3UVOX_Vect@@A 0x004C3800
VOX_Vect VehInfantryOffset = {{-400.0f, 200.0f, 0.0f}};


// The original calls the CRT atan entry after storing a double argument;
// the compiler's default fpatan intrinsic bypasses that rounding boundary.
#pragma function(atan)
#pragma intrinsic(atan)

// Inlined: the doubled vector_dist halved once.
static inline int patrol_dist(int x_a, int y_a, int x_b, int y_b) {
    return (x_dist(x_a, x_b) + abs(y_a - y_b)) >> 1;
}


// Typed dependencies: capture_base/spot_all/goody_box have explicitly
// unfinished bodies in their owners. Other helpers have product definitions;
// their remaining recovery/reference obligations are not waived by this caller.
extern int __cdecl break_treaty(int faction_id, int other_faction_id, int treaty);
extern int __cdecl act_of_aggression(int faction_id, int target_faction_id);
extern void __cdecl spot_all(int veh_id, int mode);
extern int __cdecl goody_box(int veh_id, int goody_id);
extern void __cdecl clock_wait(int milliseconds);
extern void __cdecl unspot_stack(int veh_id);


// ORIGINAL: 0x0057A050 ?monolith@@YAXH@Z 0x0057A050-0x0057A659
// RECOVERY_BINDING: v1 {"evidence": ["git:8b0da75894c71ec098213e7e12cc29fb1dfc01f3:src/unrecovered/0057a050.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "recovery/tasks/game-logic-seed-owner-coverage.json; complete typed owner migration, preserve original spans and seed lessons. Literal content provenance does not assert original allocation or game acceptance."], "reason": "Native monolith lifecycle: exclude prototype IDs16-18, use Veh.state monolith-upgraded bit0x100 and per-faction Map.bit_visible rows. Preserve live console/prototype reads after callbacks and clamp stored morale to6 independently of morale helper output."}
// TRIED: faithful port of Ghidra's structure using real Veh field offsets (x/y/state/proto_id/faction_id/dmg_incurred/order/waypoint/morale from src/veh.h) as typed globals; matches through instr #10 (address calc). Diverges at #11: original re-reads proto_id with plain 16-bit `mov ax` at the first use and a separate `movsx` at each later use, never caching the sign-extended value; my single `short sVar1` local gets sign-extended once at declaration instead. Not chased further.
void __cdecl monolith(int veh_id) {
    const int faction = Vehs[veh_id].faction_id;
    const int x = Vehs[veh_id].x;
    const int y = Vehs[veh_id].y;
    int value;
    if (Vehs[veh_id].proto_id == 0x10 || Vehs[veh_id].proto_id == 0x11
        || Vehs[veh_id].proto_id == 0x12) {
        return;
    }
    if (faction == ConsoleGlobal.faction_id_ && !IsMultiplayerNet) {
        if (Vehs[veh_id].order == 0x18 && Vehs[veh_id].waypoint_x[0] == x
            && Vehs[veh_id].waypoint_y[0] == y) {
            Vehs[veh_id].order = 0;
        }
        if (Vehs[veh_id].order) {
            return;
        }
    }
    if (!Weapon[VehPrototypes[Vehs[veh_id].proto_id].weapon_id].offense_rating) {
        if (Vehs[veh_id].dmg_incurred) {
            Vehs[veh_id].dmg_incurred = 0;
            veh_skip(veh_id);
            draw_tile(x, y, 2);
            if (faction == ConsoleGlobal.faction_id_) {
                NetMsgPrimary.pop("MONOLITHHEAL", 5000, 0, "monolith_sm.pcx");
                GameDataStateStorage.fields_.game_state_ |= 4u;
            }
        } else if (!(Vehs[veh_id].state & VSTATE_MONOLITH_UPGRADED)) {
            Vehs[veh_id].state |= VSTATE_MONOLITH_UPGRADED;
            if (faction == ConsoleGlobal.faction_id_
                && !(GameDataStateStorage.fields_.game_more_preferences_ & 0x800u)
                && !IsMultiplayerNet) {
                NetMsgPrimary.pop("SEEMONOLITH", -5000, 0, "monolith_sm.pcx");
                GameDataStateStorage.fields_.game_state_ |= 4u;
            }
        }
        return;
    }
    for (value = 1; value < MaxPlayerNum; value++) {
        del_site(value, 0, x, y, 0);
    }
    if (is_human(faction)) {
        if (!IsMultiplayerNet && !(GameDataStateStorage.fields_.game_more_preferences_ & 0x800u)) {
            g_FX.play(6);
            value = popp(ScriptPopupFile, "MONOLITH", 0, "monolith_sm.pcx", 0);
            g_FX.stop(6);
            if (value == 2) {
                GameDataStateStorage.fields_.game_more_preferences_ |= 0x800u;
                prefs_save(0);
            } else if (!value) {
                for (int site_id = 1; site_id < MaxPlayerNum; site_id++) {
                    if (map_loc(x, y)->bit_visible[site_id - 1] & 0x2000u) {
                        add_site(site_id, 0, 2, x, y);
                    }
                }
                return;
            }
        }
    } else if (Vehs[veh_id].state & VSTATE_MONOLITH_UPGRADED || morale_veh(veh_id, 1, 0) >= 6
               || Vehs[veh_id].morale >= 6
               || !Weapon[VehPrototypes[Vehs[veh_id].proto_id].weapon_id].offense_rating) {
        if (Vehs[veh_id].dmg_incurred) {
            Vehs[veh_id].dmg_incurred = 0;
            veh_skip(veh_id);
            draw_tile(x, y, 2);
        }
        for (int site_id = 1; site_id < MaxPlayerNum; site_id++) {
            if (map_loc(x, y)->bit_visible[site_id - 1] & 0x2000u) {
                add_site(site_id, 0, 2, x, y);
            }
        }
        return;
    }
    if (Vehs[veh_id].dmg_incurred) {
        Vehs[veh_id].dmg_incurred = 0;
        veh_skip(veh_id);
        draw_tile(x, y, 2);
        if (Vehs[veh_id].state & VSTATE_MONOLITH_UPGRADED || morale_veh(veh_id, 1, 0) >= 6) {
            if (faction == ConsoleGlobal.faction_id_) {
                if (GameDataStateStorage.fields_.game_more_preferences_ & 0x800u) {
                    NetMsgPrimary.pop("MONOLITHHEAL", 5000, 0, "monolith_sm.pcx");
                } else {
                    NetMsgPrimary.pop("MONOLITHHEAL", -5000, 0, "monolith_sm.pcx");
                }
                GameDataStateStorage.fields_.game_state_ |= 4u;
            }
            return;
        }
    }
    if (!(Vehs[veh_id].state & VSTATE_MONOLITH_UPGRADED) && morale_veh(veh_id, 1, 0) < 6) {
        value = rand() % 32;
        if (!(map_loc(x, y)->bit & 0x02000000u) || GameDataStateStorage.fields_.turn_current_num_ < 100) {
            value = 1;
            bit_set(x, y, 0x02000000, 1);
        }
        Vehs[veh_id].state |= VSTATE_MONOLITH_UPGRADED;
        Vehs[veh_id].morale = static_cast<uint8_t>(range(Vehs[veh_id].morale + 1, 0, 6));
        veh_skip(veh_id);
        StringTemp[0] = '\0';
        say_morale(veh_id, 0);
        parse_says(0, StringTemp, -1, -1);
        CharLowerA(ParseStrBuffer[0].str);
        if (!value) {
            bit_set(x, y, 0x2000, 0);
            synch_bit(x, y, faction);
        } else {
            for (int site_id = 1; site_id < MaxPlayerNum; site_id++) {
                if (map_loc(x, y)->bit_visible[site_id - 1] & 0x2000u) {
                    add_site(site_id, 0, 2, x, y);
                }
            }
        }
        draw_tile(x, y, 2);
        if (faction == ConsoleGlobal.faction_id_) {
            g_FX.play(0x22);
            StringTemp[0] = '\0';
            strcat(StringTemp, "MONOLITH");
            char decimal[0x50];
            _itoa(value != 0, decimal, 10);
            strcat(StringTemp, decimal);
            if (GameDataStateStorage.fields_.game_more_preferences_ & 0x800u) {
                NetMsgPrimary.pop(StringTemp, 5000, 0, "monolith_sm.pcx");
            } else {
                NetMsgPrimary.pop(StringTemp, -5000, 0, "monolith_sm.pcx");
            }
            GameDataStateStorage.fields_.game_state_ |= 4u;
        }
    } else if (faction == ConsoleGlobal.faction_id_) {
        if (GameDataStateStorage.fields_.game_more_preferences_ & 0x800u) {
            NetMsgPrimary.pop("MONOLITHNO", 5000, 0, "monolith_sm.pcx");
        } else {
            NetMsgPrimary.pop("MONOLITHNO", -5000, 0, "monolith_sm.pcx");
        }
    }
}








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
Purpose: Check against faction's available tech for best available reactor.
// ORIGINAL: 0x0057EFA0 ?best_reactor@@YAHH@Z 0x0057EFA0-0x0057EFE2 BYTE_EXACT
// LEVER: has_tech_call, not has_tech - the image CALLS 0x5B9F20 in the loop,
//        so the `MEASURED inline` in technology.h must be reached through the
//        non-inline forwarder; open-coded it cost the body its register
//        allocation (ebx for the argument) and 25 extra instructions (5/31 at
//        /O2 /Oy- before the switch).
// symbol    ?best_reactor@@YAHH@Z
// size      66 bytes
// prototype int (__cdecl ?best_reactor@@YAHH@Z)(int factionID)
// callers   3   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005B9F20
Return Value: Best reactor available (1-4)
Status: Complete
*/
int __cdecl best_reactor(int faction_id) {
    for (int i = MaxReactorNum - 1; i >= 0; i--) {
        if (has_tech(Reactor[i].preq_tech, faction_id)) {
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
        if (has_tech(Chassis[i].preq_tech, faction_id) && (triad_chk < 0
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
        if (has_tech(Weapon[i].preq_tech, faction_id)) {
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
        if (has_tech(Armor[i].preq_tech, faction_id) && Armor[i].cost <= max_cost
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
    uint32_t bit_src = bit_at(x_src, y_src);
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
    // The image open-codes the y distance; only the x distance calls abs().
    if (bit_src & BIT_RIVER && bit_dst & BIT_RIVER && x_dist(x_src, x_dst) == 1
        && (y_src - y_dst < 0 ? y_dst - y_src : y_src - y_dst) == 1 && faction_id) {
        return 1;
    }
    if (VehPrototypes[proto_id].chassis_id == CHSI_HOVERTANK
        || has_abil(proto_id, ABL_ANTIGRAV_STRUTS)) {
        return Rules.move_rate_roads;
    }
    int cost = Rules.move_rate_roads;
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
            && PlayersData[faction_id].soc_effect_active.values_[SocialEffect::PLANET] <= 0) {
            return cost + Rules.move_rate_roads * 2;
        }
        cost = max(cost, speed_proto(proto_id));
    }
    return cost;
}

/*
Purpose: Relocate an existing unit to the specified tile.
// ORIGINAL: 0x005A59B0 ?veh_put@@YAXHHH@Z 0x005A59B0-0x005A59D2 BYTE_EXACT
// body      src/veh.h
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

// RECOVERY_ASSOCIATED: v1 switch_targets 0x005A5A60 0x005A5CE4 0x005A5D00 0x005A5AF4
/*
Purpose: Calculates cost of the prototype based on various factors. Optimized logic flow from
         the original without any differences to the final calculation.
// ORIGINAL: 0x005A5A60 ?proto_cost@@YAHHHHHH@Z 0x005A5A60-0x005A5CE1
// symbol    ?proto_cost@@YAHHHHHH@Z
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
int __cdecl proto_cost(int chassis_id, int weapon_id, int armor_id,
                            int ability, int reactor_id) {
    uint8_t weap_cost = Weapon[weapon_id].cost;
    int armor_cost = Armor[armor_id].cost;
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
    int combat_mod = armor_cost / 2 + 1;
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
                if ((proto_id_loop < MaxVehProtoFactionNum &&
                    has_tech(VehPrototypes[proto_id_loop].preq_tech,
                        proto_id / MaxVehProtoFactionNum))
                    || (proto_id_loop >= MaxVehProtoFactionNum
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
Purpose: Check whether the prototype has a specific ability.
// ORIGINAL: 0x005BF1F0 ?has_abil@@YAHHH@Z 0x005BF1F0-0x005BF310 BYTE_EXACT
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
    int faction_id = proto_id / MaxVehProtoFactionNum;
    if (VehPrototypes[proto_id].ability_flags & ability_id) {
        return true;
    }
    if (!faction_id) {
        return false; // skip basic prototypes from #UNITS
    }
    if ((Players[faction_id].rule_flags & RFLAG_ALIEN) && ability_id == ABL_DEEP_RADAR) {
        return true; // Caretakers + Usurpers > "Deep Radar" ability for all units
    }
    for (int i = 0; i < Players[faction_id].faction_bonus_count; i++) {
        if (Players[faction_id].faction_bonus_id[i] == RULE_FREEABIL) {
            // Original 0x5BF298 rereads the bonus value after has_tech returns.
            if (has_tech(Ability[Players[faction_id].faction_bonus_val1[i]].preq_tech, faction_id) &&
                (ability_id & (1 << Players[faction_id].faction_bonus_val1[i]))) {
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








// ===== veh.cpp, P2 wave 2: the prototype-design and battle families =====


int __cdecl battle_kill(int cur, int *attacker_out, int *defender_out,
                        int *odds_out, int killer_id, int credit_faction);


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






// The ordinary direction rows are shared by map and vehicle consumers.
// battle_fight uses these when mode is zero; nonzero mode selects the full
// RadiusOffsetX/Y owners in map.cpp, not separate eight-element Alt arrays.
// The pinned bases are 66EF50/66EF74 versus 66EFBC/66F440. action_destroy
// passes radius indices 1..24, so the latter must retain their 289 entries.
// These are file-backed initial values rather than independent BSS storage.
// The ordinary tables also contain the stationary centre at index eight.
// Eight directions followed by the centre, sampled by scoot setup's nine
// iterations. Both complete initial runs are present in original .rdata.
// RECOVERY_OBJECT: v2 0x0066EF50 0x0066EF74 ?VehMoveDeltaX@@3PAHA 0x004BFE00
int VehMoveDeltaX[9] = {1, 2, 1, 0, -1, -2, -1, 0, 0};
// RECOVERY_OBJECT: v2 0x0066EF74 0x0066EF98 ?VehMoveDeltaY@@3PAHA 0x004BFE00
int VehMoveDeltaY[9] = {-1, 0, 1, 2, 1, 0, -1, -2, 0};



// Inline bit scan: its by-value copy frees add_abil's dead ability slot for `bit`.
static inline int abil_bit(int ability) {
    int bit = 0;
    while ((ability & 1) == 0) {
        ability >>= 1;
        ++bit;
    }
    return bit;
}

/*
Purpose: Grant a prototype an ability, if the faction has researched it.
// ORIGINAL: 0x00581190 ?add_abil@@YAXPAHHH@Z 0x00581190-0x00581259 BYTE_EXACT
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

    const int bit = abil_bit(ability);

    if (has_tech(Ability[bit].preq_tech, faction_id) == 0) {
        return;
    }

    // A faction bonus of id 0x0F grants one ability free; refuse a second.
    for (int i = 0; i < Players[faction_id].faction_bonus_count; ++i) {
        if (Players[faction_id].faction_bonus_id[i] == 0x0F
            && has_tech(Ability[Players[faction_id].faction_bonus_val1[i]].preq_tech,
                             faction_id) != 0
            && bit == Players[faction_id].faction_bonus_val1[i]) {
            return;
        }
    }

    *ability_flags |= 1 << bit;
}



// base_kill's forwarder lives in pending_bodies.cpp beside the other
// unrecovered callees; no header claims it yet, so this file declares it.
void __cdecl base_kill(int base_id);



namespace goody_box_detail {

static Map *tile_at(int x, int y) {
    return &MapTiles[MapLongitude * y + (x >> 1)];
}

static int wrap_x(int x) {
    if (!(MapIsFlat & 1)) {
        if (x < 0) x += MapLongitudeBounds;
        else if (x >= MapLongitudeBounds) x -= MapLongitudeBounds;
    }
    return x;
}

static int on_map(int x, int y) {
    return x >= 0 && x < MapLongitudeBounds
        && y >= 0 && y < MapLatitudeBounds;
}

// Convert 32-bit two's-complement bits without implementation-defined casts.
static int signed_from_bits(uint32_t bits) {
    if (bits <= 0x7FFFFFFFu) return (int)bits;
    return -1 - (int)(~bits);
}

// Native CDQ/SUB/SAR: negative inputs add one before arithmetic shift, so
// this is truncation toward zero (not floor division).
static int native_half(int value) {
    const uint32_t bits = (uint32_t)value;
    const uint32_t corrected = bits - (value < 0 ? 0xFFFFFFFFu : 0u);
    return signed_from_bits((corrected >> 1)
        | (corrected & 0x80000000u));
}

// Native add wraps to32 bits before signed normalized remainder by8.
// Negative remainder is not repaired into a positive array index: the
// original's invalid negative-turn/table cases remain unverified obligations.
static int rotated_spawn_index(int location, int turn) {
    return signed_from_bits((uint32_t)location + (uint32_t)turn) % 8;
}

static int native_threshold_passes(int event_id, uint32_t game_state,
                                   int owned_distance, int terrain_factor,
                                   int native_lifeforms) {
    const uint32_t game_factor = ((game_state & 0x00100000u) != 0) + 1u;
    const uint32_t lhs = game_factor * (uint32_t)owned_distance;
    const uint32_t population = 4u - (uint32_t)native_lifeforms;
    const uint32_t rhs = (uint32_t)(event_id - 13) * (uint32_t)terrain_factor
                       + population * 2u;
    return signed_from_bits(lhs) >= signed_from_bits(rhs);
}

static void redraw_world() {
    world_climate();
    draw_map(1);
}

static int reveal_nearby_terrain(int x, int y, int faction_id,
                                 int native_terrain, int local) {
    const int source_region = tile_at(x, y)->region;
    for (int radius = 0; radius < 121; ++radius) {
        int tx = x + RadiusOffsetX[radius];
        if (!(MapIsFlat & 1)) {
            if (tx < 0) tx += MapLongitudeBounds;
            else if (tx >= MapLongitudeBounds) tx -= MapLongitudeBounds;
        }
        const int ty = y + RadiusOffsetY[radius];
        if (!on_map(tx, ty)) continue;
        if (native_terrain) {
            Map *nearby_tile = tile_at(tx, ty);
            if ((nearby_tile->climate & 0xE0) >= 0x60) continue;
            if (nearby_tile->region != source_region) continue;
        }
        spot_loc(tx, ty, faction_id);
    }
    if (local) {
        g_FX.play(0x16);
        NetMsgPrimary.pop(native_terrain ? "GOODYSONAR" : "GOODYVIEW",
                          5000, 0,
                          native_terrain ? "supply_sm.pcx" : 0);
        g_FX.fade(0x16);
    }
    draw_map(1);
    return 0;
}

static int relocation_random_below(int bound) {
    if (bound != 1 && bound > 1) return rand() % bound;
    return 0;
}

static uint32_t normalized_crt_mask_draw() {
    uint32_t value = (uint32_t)rand() & 0x80000001u;
    if (value & 0x80000000u)
        value = (value & 1u) ? 0xFFFFFFFFu : 0;
    return value;
}

static uint32_t normalized_crt_mask_3_draw() {
    uint32_t value = (uint32_t)rand() & 0x80000003u;
    if (value & 0x80000000u)
        value = ((value - 1u) | 0xFFFFFFFCu) + 1u;
    return value;
}

static int reroll_event(int y, int &attempt, int &event_id) {
    // Native draws %14 before checking the retry limit, increments attempt
    // zero to one, and promotes attempt two to three before boundary draws.
    event_id = rand() % 14;
    if (attempt) {
        if (attempt >= 3) return 0;
        if (attempt == 2) attempt = 3;
    } else {
        attempt = 1;
    }
    if ((y == 0 || y == MapLatitudeBounds - 1) && rand() % 3 == 0)
        event_id = 12;
    if (rand() % 100 == 0) event_id = 18;
    return 1;
}

static int relocate_vehicle(int veh_id, int x, int y, int faction_id,
                            int native_terrain, int local) {
    const int proto_id = Vehs[veh_id].proto_id;
    const int plan = VehPrototypes[proto_id].plan;
    const int chassis_id = VehPrototypes[proto_id].chassis_id;
    int radius_limit;
    if (Chassis[chassis_id].triad == 0 && plan != 8) {
        radius_limit = (plan != 9 ? 0x10 : 0) + 8;
    } else {
        const uint32_t coin = goody_box_detail::normalized_crt_mask_draw();
        if (coin == 0) return 0;
        radius_limit = 8;
    }

    const int source_region = tile_at(x, y)->region;
    int attempts = 0;
    int target_x = 0;
    int target_y = 0;
    for (;;) {
        int raw_x = relocation_random_below(MapLongitudeBounds);
        const int raw_y = relocation_random_below(MapLatitudeBounds);
        int adjusted_x = raw_x;
        if (raw_x & 1) adjusted_x = raw_x - 1;
        if (raw_y & 1) adjusted_x += 1;
        target_x = adjusted_x;
        target_y = raw_y;
        ++attempts;
        if (attempts >= 1000) return 0;

        if (!on_map(target_x, target_y)) continue;
        Map *target = tile_at(target_x, target_y);
        if (((target->climate & 0xE0) < 0x60) != native_terrain) continue;
        if (target->region != source_region) continue;
        if ((target->bit & 3u) != 0 && (target->val3 & 0x0F) < 8) continue;
        if (zoc_any(target_x, target_y, faction_id) != 0) continue;
        if (PlayersData[faction_id].flags & 0x200u) continue;
        if (target->visibility & (1u << (faction_id & 0x1F))) continue;

        int x_distance = abs(x - target_x);
        if (!(MapIsFlat & 1) && MapLongitude < x_distance)
            // Pinned57B6B8/57B6FE subtract from949870 full map width;
            // the preceding68FAF0 comparison uses only the half-width stride.
            x_distance = MapLongitudeBounds - x_distance;
        const int manhattan_distance = abs(y - target_y) + x_distance;
        // Native first rejects a close site using the even-rounded full
        // Manhattan sum, then independently halves that sum for radius.
        if ((manhattan_distance & ~1) <= 8) continue;
        const int distance = manhattan_distance >> 1;
        if (distance <= radius_limit) break;
    }

    if (local) {
        NetMsgPrimary.pop(native_terrain ? "GOODYWAVE" : "GOODYGATE",
                          -5000, 0, native_terrain ? "stars_sm.pcx" : 0);
    }
    Vehs[veh_id].moves_expended = 0;
    Vehs[veh_id].order = 0;
    Vehs[veh_id].state &= 0xFCFFFDFFu;
    const int moved_stack = stack_veh(veh_id, 1);
    stack_put(moved_stack, target_x, target_y);
    if (Vehs[veh_id].faction_id != ConsoleGlobal.faction_id_) {
        draw_tile(x, y, 2);
    } else {
        draw_tile_fixup(x, y, has_abil(proto_id, 2), 2);
    }
    if (local) {
        ConsoleGlobal.focus(target_x, target_y, faction_id);
        MapWinTable[0]->redraw();
    }
    spot_all(veh_id, 1);
    return 1;
}

static int local_player(int faction_id) {
    return ConsoleGlobal.faction_id_ == faction_id;
}

static void notify_energy(int faction_id, int amount, int native_side, int local) {
    PlayersData[faction_id].energy_reserves += amount;
    if (local) {
        parse_num(0, amount);
        g_FX.play(0x16);
        NetMsgPrimary.pop(native_side ? "GOODYHYDROGEN" : "GOODYURANIUM",
                          -5000, 0,
                          native_side ? "supply_sm.pcx" : 0);
        g_FX.fade(0x16);
        ConsoleInfoWin.on_redraw();
    }
}

static int give_random_technology(int faction_id, int notify, int forced,
                                  int native_terrain) {
    const int start = rand();
    for (int count = 0; count < 89; ++count) {
        const int tech_id = (start % 89 + count) % 89;
        if (has_tech(tech_id, faction_id)) continue;
        if (!tech_avail(tech_id, faction_id)) continue;
        if (tech_is_preq(tech_id, 0x30, 9999)
            && !GameDataStateStorage.fields_.game_tech_achieved_[TECH_BRAIN])
            continue;
        const RulesTechnology &definition = Technology[tech_id];
        int terminal_prerequisites = 0;
        const int prerequisites[2] = { definition.preq_tech_1,
                                        definition.preq_tech_2 };
        for (int slot = 0; slot < 2; ++slot) {
            const int parent = prerequisites[slot];
            if (parent == -1) ++terminal_prerequisites;
            else if (parent >= 0
                     && Technology[parent].preq_tech_1 == -1
                     && Technology[parent].preq_tech_2 == -1) {
                ++terminal_prerequisites;
                break;
            }
        }
        if (terminal_prerequisites < 2) continue;
        if (tech_id == 0x4E || tech_id == 0x4F || tech_id == 0x51
            || tech_id == 0x52 || tech_id == 0x53) continue;
        if (!forced) ++PlayersData[faction_id].tech_achieved;
        if (notify) {
            clear();
            says("GOODYDATA");
            if (IsMultiplayerNet) {
                says("1");
                parse_say(0, Technology[tech_id].name, -1, -1);
            }
            NetMsgPrimary.pop(StringTemp,
                              native_terrain ? 5000 : -5000, 0,
                              "supply_sm.pcx");
        }
        tech_achieved(faction_id, tech_id, -1, 0);
        // tech_achieved owns its normal count increment. Native forced gifts
        // cancel that increment after the callback, while unforced gifts keep
        // it in addition to the pre-callback count recorded above.
        if (forced) --PlayersData[faction_id].tech_achieved;
        return tech_id;
    }
    return -1;
}

} // namespace goody_box_detail


// ORIGINAL: 0x0057A660 ?goody_box@@YAHHH@Z 0x0057A660-0x0057CDE6
// RECOVERY_BINDING: v1 {"evidence": ["task:goody-box-owner", "pinned:57A660..57CDE6 whole dispatcher; original two-int ABI, unused second parameter and seed findings preserved.", "pinned:A8D2 live vehicle x/y retry reload; spawned vehicle prototype/chassis triad read after veh_init; separate event9 and cases12..17 radius paths.", "pinned:B4D8..B874 relocation stack_veh/stack_put/render/focus/spot_all order;CC16..CDE6 retry/resource/overflow gates;A9A1 GameRules reload.", "source:existing Veh/PlayerData/Base/Map/technology declarations and bounded shared radius arrays; map-window public draw_tile_fixup APIs own original render calls.", "historical seed:src/unrecovered/0057a660.cpp mapping and parsed dispatch findings retained."], "reason": "Recover the complete original pod outcome dispatcher through source-owned vehicle, faction, base, technology and map state; retain original identity/spans and historical seed findings, without executable calls or raw storage aliases."}
// DEFERRED: structurally the most promising remaining shape in this batch, not yet landed. Frame is only 0x34 bytes (plain locals, no embedded C++ objects), and the function is a dispatcher: a 26-entry jump table at 0x0057A9BF (targets listed in this file's own brief, already resolved from the image - cases 12-17 share one target 0x0057C5C8, and two more small tables at 0x0057C451/0x0057C531 handle an 8-way and 4-way sub-dispatch off the tail of that). That makes this closer to "26 mostly-independent small case bodies" than one tangled 10KB function - each case is a self-contained goody-box outcome (energy, new tech, new unit, mines, artifact, ...), calling _rand(29x)/?pop@NetMsg@@(25x, the popup message)/ ?bit_set@@(15x, faction tech/facility grant)/?veh_init@@(9x, spawn a unit)/?parse_says@@/?play@FX@@/?draw_tile@@/?base_find@@ - none of which chain into the register-computed multi-field arithmetic that made the two smaller functions in this batch unsafe to rush. Still 3172 instructions / 1243 Ghidra lines total, too much for this pass in the time remaining.
int __cdecl goody_box(int veh_id, int type) {
    (void)type; // Native has no read of [ebp+0x0C].
    int x = Vehs[veh_id].x;
    int y = Vehs[veh_id].y;
    const int faction_id = Vehs[veh_id].faction_id;
    const int native_terrain = (goody_box_detail::tile_at(x, y)->climate & 0xE0) < 0x60;

    // Native order: crash guard returns before counters or seeded RNG mutation.
    if (do_unity_crash(x, y, faction_id)) return 0;
    const int local = goody_box_detail::local_player(faction_id);

    PlayerData &faction = PlayersData[faction_id];
    ++faction.unk_94; // original 0x96EA20; setup_player clears the DWORD.
    if (IsMultiplayerNet) my_srand((int)ScenarioSave.starting_mission_year + faction.unk_94 * 0x25);
    if (faction.soc_effect_pending.values_[SocialEffect::RESEARCH] * -5 <= GameDataStateStorage.fields_.turn_current_num_
        && !(GameDataStateStorage.fields_.game_rules_ & 0x04000000u))
        ++faction.tech_accumulated;

    const int base_id = base_find(x, y);
    Map *tile = goody_box_detail::tile_at(x, y);
    int forced = 0;
    int event_id = 0;
    int attempt = 0;
    if ((tile->bit & 0x0C000000u)
        && (faction.current_num_bases == 0
            || (base_id >= 0 && Bases[base_id].faction_id_current == faction_id))) {
        event_id = 10 - ((tile->bit & 0x04000000u) != 0);
        forced = 1;
        bit_set(x, y, 0x0C000000, FALSE);
    } else if (BaseFindDist <= 2 && base_id >= 0
               && Bases[base_id].faction_id_current != faction_id) {
        event_id = 0x10;
        forced = 1;
    } else if (((tile->bit & 0x20u)
                && (tile->climate & 0xE0u) >= 0x40u)
               || native_terrain) {
        // Native enters its shared reroll block here: %14, optional edge %3,
        // then %100. These draws precede the event loop's chassis test.
        // Native retry target 57A8D2 reloads the vehicle coordinates before
        // its first random draw. The entry terrain classification above stays
        // captured, but subsequent map lookups use the reloaded coordinates.
        x = Vehs[veh_id].x;
        y = Vehs[veh_id].y;
        tile = goody_box_detail::tile_at(x, y);
        if (!goody_box_detail::reroll_event(y, attempt, event_id))
            return 0;
    } else {
        int chance = (BaseFindDist <= 0x0C
                      || !faction.region_total_bases[tile->region])
                   ? 0x28 : 0x14;
        const int plan = VehPrototypes[Vehs[veh_id].proto_id].plan;
        if ((plan == 9 && (BaseFindDist < 0x0C || GameDataStateStorage.fields_.turn_current_num_ < 0x28))
            || plan == 8) chance += 10;
        if (chance > 1) event_id = rand() % chance;
    }

    while (attempt <= 3) {
        if (VehPrototypes[Vehs[veh_id].proto_id].plan == 12
            && rand() % 3 == 0
            && !(Vehs[veh_id].flags & 0x20u))
            event_id = 9999;
dispatch_event:
        switch (event_id) {
        case 0: { // credits
            const int threshold = native_terrain ? 0x64 : 0x32;
            const int base_amount = GameDataStateStorage.fields_.turn_current_num_ >= threshold ? 0x32 : 0x19;
            const int amount = (rand() % (native_terrain ? 4 : 3) + 1)
                             * base_amount;
            goody_box_detail::notify_energy(faction_id, amount,
                                               native_terrain, local);
            return 0;
        }
        case 1: { // remove fungus and update the climate
            if (!native_terrain && !(tile->bit & 0x80u)) {
                int offset = 0;
                for (; offset < 9; ++offset) {
                    int nx = x + RadiusOffsetX[offset];
                    if (!(MapIsFlat & 1)) {
                        if (nx < 0) nx += MapLongitudeBounds;
                        else if (nx >= MapLongitudeBounds) nx -= MapLongitudeBounds;
                    }
                    const int ny = y + RadiusOffsetY[offset];
                    if (goody_box_detail::on_map(nx, ny)) {
                        const Map *nearby_tile = goody_box_detail::tile_at(nx, ny);
                        if ((nearby_tile->bit & 0x80u)
                            || ((nearby_tile->climate & 0xE0u) < 0x60u))
                            break;
                    }
                }
                if (offset == 9) {
                    if (local) {
                        g_FX.play(0x26);
                        NetMsgPrimary.pop("GOODYRIVER", 5000, 0,
                                          "supply_sm.pcx");
                    }
                    bit_set(x, y, 0x100, TRUE);
                    goody_box_detail::redraw_world();
                    return 0;
                }
            }
            break;
        }
        case 2: {
            // Native sonar walks 121 entries from the radius-offset table. Water-origin
            // pods constrain the pass to the same water region; land-origin
            // pods reveal every valid tile in that native table walk.
            if (native_terrain || (tile->climate & 0xE0) >= 0xA0)
                return goody_box_detail::reveal_nearby_terrain(
                    x, y, faction_id, native_terrain, local);
            if (GameDataStateStorage.fields_.turn_current_num_ >= 0x32) break;
            event_id = 8;
            goto dispatch_event; // Native shared dispatch skips the chassis reroll.
        }
        case 3: { // tectonic shift
            const int original_change_count = faction.unk_96;
            const uint32_t doubled_changes =
                (uint32_t)original_change_count * 2u;
            const int32_t tectonic_denominator =
                (int32_t)(doubled_changes + 2u);
            const int32_t denominator_minus_one =
                (int32_t)(doubled_changes + 1u);
            const int eligible_by_preference =
                !(GameDataStateStorage.fields_.game_rules_ & 0x01000000u) || GameDataStateStorage.fields_.diff_level_current_ >= 2;
            if (eligible_by_preference && (tile->climate & 0xE0) <= 0x80
                && faction.current_num_bases >= 2 && !native_terrain
                && !IsMultiplayerNet
                && (tectonic_denominator == 1
                    || denominator_minus_one <= -1
                    || rand() % tectonic_denominator == 0)) {
                ++faction.unk_96;
                g_FX.play(8);
                world_raise_alt(x, y);
                world_raise_alt(x, y);
                for (int radius = 0; radius < 25; ++radius) {
                    const int nx = goody_box_detail::wrap_x(x + RadiusOffsetX[radius]);
                    const int ny = y + RadiusOffsetY[radius];
                    if (!goody_box_detail::on_map(nx, ny)) continue;
                    Map *nearby_tile = goody_box_detail::tile_at(nx, ny);
                    if (nearby_tile->bit & 4u) {
                        if (rand() % 3 == 0) {
                            bit_set(nx, ny, 4, FALSE);
                            synch_bit(nx, ny, faction_id);
                        }
                    }
                    if (radius <= 8) rocky_set(nx, ny, minerals_at(nx, ny));
                }
                goody_box_detail::redraw_world();
                if (local) {
                    NetMsgPrimary.pop("GOODYQUAKE", -5000, 0, "quake_sm.pcx");
                    if (tut_check(0x800))
                        TutorialWindow.tut_map("TERRAFORM", x, y, -1,
                                              0, 0, -1, -1);
                }
                return 0;
            }
            break;
        }
        case 4: {
            const int plan = VehPrototypes[Vehs[veh_id].proto_id].plan;
            if (native_terrain || !(GameDataStateStorage.fields_.game_state_ & 0x00100000u)) {
                if (IsMultiplayerNet || plan == 12
                    || ((GameDataStateStorage.fields_.game_rules_ & 0x01000000u) && GameDataStateStorage.fields_.diff_level_current_ <= 1))
                    break;
                if (GameDataStateStorage.fields_.turn_current_num_ >= 0x19
                    && (GameDataStateStorage.fields_.turn_current_num_ >= 0x32 || GameDataStateStorage.fields_.diff_level_current_ >= 2)) {
                    if (goody_box_detail::relocate_vehicle(
                            veh_id, x, y, faction_id, native_terrain, local))
                        return 0;
                    break;
                }
                event_id = 8;
                goto dispatch_event;
            }
            event_id = 7;
            goto dispatch_event;
        }
        case 5: {
            if ((GameDataStateStorage.fields_.game_state_ & 0x00100000u) && !native_terrain) {
                event_id = 7;
                goto dispatch_event;
            }
            const int owned_base_id = base_find(x, y, faction_id);
            if (!IsMultiplayerNet && owned_base_id >= 0) {
                const int production = (int)Bases[owned_base_id].queue_size;
                if (production >= -0x45) {
                    int production_cost;
                    if (production >= 0) {
                        if (VehPrototypes[production].plan == 8
                            && Bases[owned_base_id].faction_id_current == 1)
                            break;
                        production_cost = veh_cost(production, owned_base_id, 0);
                        parse_says(0, VehPrototypes[production].veh_name,
                                   -1, -1);
                    } else {
                        const int facility_id = -production;
                        production_cost = Facility[facility_id].cost;
                        parse_say(0, Facility[facility_id].name, -1, -1);
                    }
                    const uint32_t adjusted_bits =
                        (uint32_t)cost_factor(faction_id, 1, -1)
                        * (uint32_t)production_cost;
                    const int adjusted_cost = (int32_t)adjusted_bits;
                    const uint32_t three_quarters = adjusted_bits * 3u;
                    const uint32_t sign_adjust =
                        (three_quarters & 0x80000000u) ? 3u : 0u;
                    const int minimum_progress = (int32_t)
                        (three_quarters + sign_adjust) >> 2;
                    if (Bases[owned_base_id].minerals_accumulated
                        < minimum_progress) {
                        Bases[owned_base_id].minerals_accumulated = adjusted_cost;
                        if (local) {
                            parse_says(1, Bases[owned_base_id].name_string,
                                       -1, -1);
                            NetMsgPrimary.pop("GOODYBUILD", -5000, 0,
                                              "indbm_sm.pcx");
                        }
                        return 0;
                    }
                }
            }
            if (native_terrain || (GameDataStateStorage.fields_.game_rules_ & 0x00040000u)) break;

            uint32_t supported = faction.proto_id_active[7];
            for (int base = 0; base < GameDataStateStorage.fields_.base_current_count_; ++base) {
                if (Bases[base].faction_id_current == faction_id
                    && (Bases[base].state & 0x400u))
                    ++supported;
            }
            if (faction.current_num_bases <= (int)supported
                && !(GameDataStateStorage.fields_.game_state_ & 0x00040000u))
                break;
            event_id = 6;
            goto dispatch_event;
        }
        case 6: {
            if (GameDataStateStorage.fields_.game_rules_ & 0x00040000u) break;
            if (attempt <= 1 || event_id != 6) {
                if (native_terrain
                    && stack_check(veh_id, 8, -1, -1, -1) <= 0)
                    break;
                int share_artifact =
                    !IsMultiplayerNet || (GameDataStateStorage.fields_.game_state_ & 0x00040000u);
                if (!share_artifact) {
                    share_artifact = 1;
                    for (int other = 1; other < 8; ++other) {
                        if (other == faction_id) continue;
                        if (!(GameDataStateStorage.fields_.lock_masks_.enable_mask_ & (1u << other))) continue;
                        if (PlayersData[other].unk_95 < faction.unk_95) {
                            share_artifact = 0;
                            break;
                        }
                    }
                }
                if (share_artifact) {
                    ++faction.unk_95;
                    const int plan = VehPrototypes[Vehs[veh_id].proto_id].plan;
                    if (plan != 12) {
                        const int new_vehicle = veh_init(7, faction_id, x, y);
                        if (new_vehicle < 0) return 0;
                        veh_skip(new_vehicle);
                        draw_tile(x, y, 2);
                        if (local) {
                            NetMsgPrimary.pop("GOODYARTIFACT", -5000, 0,
                                              "art_dis_sm.pcx");
                            g_FX.fade(0x1B);
                        }
                        return 0;
                    }
                    break;
                }
                break; // one allied owner is ahead in the native counter test.
            }
            event_id = 5;
            goto dispatch_event;
        }
        case 7: {
            if ((GameDataStateStorage.fields_.game_rules_ & 0x00200000u)
                || ((GameDataStateStorage.fields_.game_rules_ & 0x01000000u) && GameDataStateStorage.fields_.diff_level_current_ <= 1)
                || attempt > 1)
                break;
            if (native_terrain) {
                event_id = 8;
                goto dispatch_event;
            }
            base_find(x, y);
            if (BaseFindDist < 3) break;
            if (IsMultiplayerNet && faction_id == GameDataStateStorage.fields_.ranking_faction_id_unk2_
                && !goody_box_detail::normalized_crt_mask_draw())
                break;
            int tower_roll = goody_box_detail::normalized_crt_mask_draw();
            int may_make_tower = tower_roll != 0 && ExpansionEnabled;
            int made_tower = 0;
            if (local) {
                g_FX.play(9);
                NetMsgPrimary.pop("GOODYFUNGUS", -5000, 0, "fung_sm.pcx");
            }
            site_radius(x, y, 0);
            for (int radius = 0; radius < 9; ++radius) {
                const int nx = goody_box_detail::wrap_x(x + RadiusOffsetX[radius]);
                const int ny = y + RadiusOffsetY[radius];
                if (!goody_box_detail::on_map(nx, ny)) continue;
                Map *nearby_tile = goody_box_detail::tile_at(nx, ny);
                if ((nearby_tile->climate & 0xE0u) >= 0x40u) {
                    bit_set(nx, ny, 0x20, TRUE);
                    bit_set(nx, ny, 0x0028805C, FALSE);
                    synch_bit(nx, ny, faction_id);
                    if (!made_tower && may_make_tower) {
                        if (goody_box_detail::normalized_crt_mask_draw()) {
                            const int occupancy = nearby_tile->bit & 3u;
                            if ((!occupancy || (nearby_tile->val3 & 0x0F) >= 8)
                                && ExpansionEnabled
                                && (nearby_tile->climate & 0xE0u) >= 0x60u) {
                                veh_init(0x13, 0, nx, ny);
                                may_make_tower = 0;
                                made_tower = 1;
                            }
                        }
                    }
                    draw_tile(nx, ny, 2);
                }
            }
            return 0;
        }
        case 8: // monolith
            if (!(GameDataStateStorage.fields_.game_rules_ & 0x00020000u) && !bonus_at(x, y, 0)
                && !native_terrain) {
                site_radius(x, y, 0);
                bit_set(x, y, 0x2000, TRUE);
                bit_set(x, y, 0x20, FALSE);
                synch_bit(x, y, faction_id);
                draw_tiles(x, y, 2);
                monolith(veh_id);
                return 0;
            }
            break;
        case 9: { // native lifeforms
            if ((GameDataStateStorage.fields_.net_turn_flags_ & 1u)
                || GameDataStateStorage.fields_.game_tech_achieved_[TECH_ORBITAL]
                || (native_terrain
                    && stack_check(veh_id, 8, -1, -1, -1) <= 0))
                break;
            int proto_id = 11;
            if (GameDataStateStorage.fields_.game_tech_achieved_[TECH_FOSSIL]
                && rand() % 3 == 0
                && !faction.proto_id_active[12]) {
                base_find(x, y, faction_id);
                if (BaseFindDist <= speed_proto(12) * 3)
                    proto_id = 12;
            }

            int spawn_x = x;
            int spawn_y = y;
            const int active_native = faction.proto_id_active[13];
            int location_chance = active_native + 3;
            if (Chassis[tile->region].speed <= 0x32)
                location_chance = active_native + 2;
            if (active_native <= 1
                && (location_chance == 1 || location_chance <= 0
                    || rand() % location_chance == 0)) {
                if (native_terrain) {
                    proto_id = 13;
                } else {
                    int found = 0;
                    for (int radius = 0; radius < 8; ++radius) {
                        const int index = radius;
                        const int nx = goody_box_detail::wrap_x(
                            x + VehMoveDeltaX[index]);
                        const int ny = y + VehMoveDeltaY[index];
                        if (!goody_box_detail::on_map(nx, ny)) continue;
                        Map *nearby_tile = goody_box_detail::tile_at(nx, ny);
                        if ((nearby_tile->climate & 0xE0u) <= 0x5Fu
                            && Chassis[nearby_tile->region].speed >= 0x50) {
                            spawn_x = nx;
                            spawn_y = ny;
                            proto_id = 13;
                            found = 1;
                            break;
                        }
                    }
                    (void)found;
                }
            }

            const int native_total = faction.proto_id_active[11]
                                   + faction.proto_id_active[12]
                                   + faction.proto_id_active[13];
            if (faction.proto_id_active[proto_id]
                && !(native_total <= (faction.current_num_bases + 1) / 2
                     && faction.proto_id_active[proto_id] < 2)) {
                const uint32_t variant =
                    goody_box_detail::normalized_crt_mask_draw();
                event_id = (int)(variant + 12u);
                goto dispatch_event;
            }
            if (proto_id == 11 && rand() % 5 == 0 && ExpansionEnabled) {
                int alien_tech = 0;
                for (int other = 1; other < 8; ++other) {
                    if (has_tech(0x52, other) && alien_tech < 1)
                        alien_tech = 1;
                    if (has_tech(0x53, other)) alien_tech = 2;
                }
                proto_id = alien_tech >= 2 ? 18 : (alien_tech ? 17 : 16);
            }
            const int new_vehicle = veh_init(proto_id, faction_id,
                                             spawn_x, spawn_y);
            if (new_vehicle < 0) break;
            if (forced || GameDataStateStorage.fields_.turn_current_num_ <= 0x31)
                Vehs[new_vehicle].home_base_id = -1;
            const uint16_t native_order_bits =
                proto_id == 16 ? 2u : proto_id == 17 ? 4u
                : proto_id == 18 ? 6u : 2u;
            Vehs[new_vehicle].morale = (uint8_t)native_order_bits;
            if (native_terrain
                && !Chassis[VehPrototypes[
                    Vehs[new_vehicle].proto_id].chassis_id].triad)
                sleep(new_vehicle);
            if (local) {
                parse_says(0, VehPrototypes[proto_id].veh_name, -1, -1);
                const int alien_proto =
                    proto_id == 16 || proto_id == 17 || proto_id == 18;
                NetMsgPrimary.pop(alien_proto ? "OGREPOD" : "GOODYPOD",
                    native_terrain ? 5000 : -5000, 0,
                    alien_proto ? "ogre_sm.pcx" : "supply_sm.pcx");
            }
            return 0;
        }
        case 10: // treaty gift
            if (GameDataStateStorage.fields_.net_turn_flags_ & 2u) break;
            {
                const uint32_t gift_roll =
                    goody_box_detail::normalized_crt_mask_3_draw();
                if (gift_roll == 0) {
                  const int other = rand() % 7 + 1;
                  const uint32_t other_bit = 1u << other;
                  const uint32_t live_players = GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_;
                  const uint32_t human_players = GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 0xFFu;
                  const int disallowed_human_pair =
                      !(human_players & (1u << faction_id))
                      || (human_players & other_bit);
                  if (other != faction_id
                    && (live_players & other_bit)
                    && !disallowed_human_pair
                    && !(Players[other].rule_flags & 0x80u)
                    && !(PlayersData[faction_id].diplo_treaties[other] & 8u)) {
                      treaty_on(faction_id, other, 8);
                      if (local) {
                          GenderDefault = Players[other].is_leader_female;
                          PluralityDefault = 0;
                          parse_says(0, Players[other].title_leader,
                                     -1, -1);
                          GenderDefault = Players[other].is_leader_female;
                          PluralityDefault = 0;
                          parse_says(1, Players[other].name_leader,
                                     -1, -1);
                          GenderDefault = Players[other].noun_gender;
                          PluralityDefault = Players[other].is_noun_plural;
                          parse_says(2, Players[other].noun_faction,
                                     -1, -1);
                      NetMsgPrimary.pop("GOODYCOMM",
                          native_terrain ? 5000 : -5000, 0,
                              native_terrain ? "supply_sm.pcx" : 0);
                      }
                      return 0;
                  }
                }
            }
            if (GameDataStateStorage.fields_.game_rules_ & 0x04000000u) break;
            if (IsMultiplayerNet && !forced) {
                int behind_peer = 0;
                for (int other = 1; other < 8; ++other) {
                    const uint32_t bit = 1u << other;
                    if (other == faction_id || !(GameDataStateStorage.fields_.lock_masks_.enable_mask_ & bit))
                        continue;
                    const PlayerData &peer = PlayersData[other];
                    if (peer.unk_97 + 1 < faction.unk_97
                        && (peer.ranking < faction.ranking
                            || peer.tech_ranking < faction.tech_ranking)) {
                        behind_peer = 1;
                        break;
                    }
                }
                if (behind_peer) break;
            }
                if (goody_box_detail::give_random_technology(
                        faction_id, local, forced, native_terrain) >= 0)
                return 0;
            break;
        case 11: { // fungus/altitude terrain conversion
            if (tile->region == 0 || tile->region == 0x3F
                || (!native_terrain && (tile->val3 & 0xC0u) > 0x40u))
                break;
            if ((tile->climate & 0xE0u) <= 0x3Fu) {
                event_id = 12;
                goto dispatch_event;
            }
            int32_t conversion = 0;
            if (!native_terrain) {
                base_find(x, y);
                if (BaseFindDist < 3) break;
                conversion = (int32_t)
                    goody_box_detail::normalized_crt_mask_3_draw();
            }
            site_radius(x, y, 0);
            int converted = 0;
            for (int radius = 0; radius < 9; ++radius) {
                if (radius != 0
                    && rand() % (conversion > 1 ? 2 : 3) == 0)
                    continue;
                const int nx = goody_box_detail::wrap_x(
                    x + RadiusOffsetX[radius]);
                const int ny = y + RadiusOffsetY[radius];
                if (!goody_box_detail::on_map(nx, ny)) continue;
                Map *nearby_tile = goody_box_detail::tile_at(nx, ny);
                if ((nearby_tile->bit & 0x00100000u) && radius != 0) continue;
                if (native_terrain) {
                    if ((nearby_tile->climate & 0xE0u) > 0x5Fu) continue;
                    if ((nearby_tile->climate & 0xE0u) <= 0x3Fu) {
                        if (radius != 0) continue;
                        alt_set_both(nx, ny, 2);
                    }
                } else {
                    if ((nearby_tile->climate & 0xE0u) < 0x60u) continue;
                    if ((nearby_tile->val3 & 0xC0u) > 0x40u && conversion < 2)
                        continue;
                    if (!(nearby_tile->val3 & 0xC0u) && conversion == 2) continue;
                }
                bit_set(nx, ny, 0x00288070, FALSE);
                if (native_terrain) bit_set(nx, ny, 0x8000, TRUE);
                else if (conversion == 0) {
                    bit_set(nx, ny, 0x200000, TRUE);
                    bit_set(nx, ny, 0x8050, FALSE);
                } else if (conversion == 1) {
                    bit_set(nx, ny, 0x8000, TRUE);
                    bit_set(nx, ny, 0x200000, FALSE);
                } else if (conversion == 2) {
                    bit_set(nx, ny, 0x14, TRUE);
                    bit_set(nx, ny, 0x200040, FALSE);
                } else if (conversion == 3) {
                    bit_set(nx, ny, 0x40, TRUE);
                    bit_set(nx, ny, 0x200010, FALSE);
                } else continue;
                synch_bit(nx, ny, faction_id);
                draw_tile(nx, ny, 2);
                ++converted;
            }
            if (converted && local) {
                const char *message = native_terrain ? "GOODYKELP"
                    : conversion == 0 ? "GOODYFOREST"
                    : conversion == 1 ? "GOODYFARM"
                    : conversion == 2 ? "GOODYMINE" : "GOODYSOLAR";
                const char *icon = native_terrain ? "kelp_sm.pcx"
                    : conversion <= 1 ? "forgr_sm.pcx" : "supply_sm.pcx";
                NetMsgPrimary.pop(message, -5000, 0, icon);
            }
            if (converted) return 0;
            break;
        }
        case 12: case 13: case 14: case 15: case 16: case 17: {
            if ((GameDataStateStorage.fields_.game_rules_ & 0x00200000u)
                || ((GameDataStateStorage.fields_.game_rules_ & 0x01000000u) && GameDataStateStorage.fields_.diff_level_current_ <= 1))
                break;
            base_find(x, y, faction_id);
            const int owned_base_distance = BaseFindDist;
            base_find(x, y);
            if (BaseFindDist <= 1) break;
            if (event_id >= 14) {
                const int terrain_factor =
                    (tile->bit & 0x20u) && (tile->climate & 0xE0u) >= 0x40u
                    ? 2 : 4;
                if (!goody_box_detail::native_threshold_passes(
                        event_id, GameDataStateStorage.fields_.game_state_, owned_base_distance,
                        terrain_factor, (int)MapNativeLifeForms))
                    break;
            }
            if (IsMultiplayerNet && faction_id == GameDataStateStorage.fields_.ranking_faction_id_unk2_
                && !goody_box_detail::normalized_crt_mask_draw())
                break;
            if (native_terrain && event_id > 12
                + ((GameDataStateStorage.fields_.game_state_ & 0x00100000u) != 0))
                break;

            if (native_terrain)
                Vehs[veh_id].order_auto_type = (uint8_t)
                    (Vehs[veh_id].order_auto_type + Rules.move_rate_roads);

            int created_units = 0;
            int created_mind_worm = 0;
            int location = 0;
            while (location < 8) {
                // Native57C6F5..57C7A1 performs no per-location RNG
                // rejection; event11's separate conversion loop owns that draw.
                const int radius = goody_box_detail::rotated_spawn_index(
                    location, GameDataStateStorage.fields_.turn_current_num_);
                const int nx = goody_box_detail::wrap_x(
                    x + VehMoveDeltaX[radius]);
                const int ny = y + VehMoveDeltaY[radius];
                if (goody_box_detail::on_map(nx, ny)) {
                    Map *nearby_tile = goody_box_detail::tile_at(nx, ny);
                    const int climate_matches =
                        ((nearby_tile->climate & 0xE0u) < 0x60u)
                            == native_terrain;
                    const int unoccupied = !(nearby_tile->bit & 3u)
                                        || (nearby_tile->val2 & 0x0Fu) >= 8;
                    if (climate_matches && unoccupied) {
                        if (native_terrain) {
                            const int worm = veh_init(9, 0, nx, ny);
                            if (worm >= 0) {
                                ++created_units;
                                Vehs[worm].visibility |= nearby_tile->visibility;
                                int cargo_count = (int32_t)(
                                    goody_box_detail::normalized_crt_mask_draw()
                                    + 1u);
                                const int cargo_limit = veh_cargo(worm);
                                if (cargo_count >= cargo_limit)
                                    cargo_count = veh_cargo(worm);
                                else
                                    cargo_count = (int32_t)cargo_count;
                                for (int cargo = 0; cargo < cargo_count; ++cargo) {
                                    if (rand() % 5 || !ExpansionEnabled) {
                                        veh_init(8, 0, nx, ny);
                                    } else {
                                        veh_init(15, 0, nx, ny);
                                        created_mind_worm = 1;
                                    }
                                }
                                // Native success flows directly to its own
                                // cargo/notification tail, not the nonnative stepper.
                                break;
                            }
                        } else {
                            base_find(nx, ny);
                            if (BaseFindDist >= 2) {
                                const int proto =
                                    (rand() % 5 || !ExpansionEnabled) ? 8 : 15;
                                const int created = veh_init(proto, 0, nx, ny);
                                if (proto == 15) created_mind_worm = 1;
                                if (created >= 0) {
                                    ++created_units;
                                    Vehs[created].visibility |= nearby_tile->visibility;
                                    draw_tile(nx, ny, 2);
                                    const int state_limit =
                                        (int)(((GameDataStateStorage.fields_.game_state_ & 0x00100000u)
                                              | 0x00C00000u) >> 20);
                                    if (state_limit < event_id
                                        || GameDataStateStorage.fields_.turn_current_num_ < 0x0F
                                        || BaseFindDist <= 3)
                                        location = 8;
                                }
                            }
                        }
                    }
                }
                if (location >= 8) break;
                if (created_units) {
                    int step = 5 - goody_box_detail::native_half(
                        faction.current_num_bases);
                    if (step < 1) step = 1;
                    if (step >= 5) step = 4;
                    location += step;
                } else {
                    ++location;
                }
                if (created_units && location >= 8) break;
            }
            if (created_units) {
                if (local) {
                    g_FX.play(0x11);
                    const char *message = native_terrain ? "GOODYISLE"
                                                        : "GOODYWORMS";
                    const char *icon = native_terrain ? "isle_sm.pcx"
                        : (created_mind_worm ? "sporlnch_sm.pcx"
                                             : "mindworm_sm.pcx");
                    NetMsgPrimary.pop(message, -5000, 0, icon);
                    g_FX.fade(0x11);
                }
                return 0;
            }
            break;
        }
        case 18: { // duplicate a vehicle after native/peer eligibility checks
            if (native_terrain) {
                uint32_t native_roll = (uint32_t)rand() & 0x80000003u;
                int eligible = native_roll == 0;
                if ((int32_t)native_roll <= -1)
                    eligible = (((native_roll - 1u) | 0xFFFFFFFCu)
                                == 0xFFFFFFFFu);
                if (!eligible) break;
            } else if (!Weapon[VehPrototypes[Vehs[veh_id].proto_id].weapon_id]
                            .offense_rating) {
                break;
            }
            if (VehPrototypes[Vehs[veh_id].proto_id].plan == 12)
                break;

            int peer_can_receive = !IsMultiplayerNet;
            if (IsMultiplayerNet) {
                int peer_ahead = 0;
                peer_can_receive = 1;
                for (int other = 1; other < 8; ++other) {
                    const uint32_t bit = 1u << other;
                    if (other == faction_id
                        || !(GameDataStateStorage.fields_.lock_masks_.enable_mask_ & (bit << 8))
                        || !(GameDataStateStorage.fields_.lock_masks_.enable_mask_ & bit))
                        continue;
                    const int peer_total = PlayersData[other].total_mil_veh;
                    if (native_terrain) {
                        if (faction.unk_70 <= PlayersData[other].unk_70)
                            peer_ahead = 1;
                    } else if (faction.total_mil_veh <= peer_total) {
                        peer_ahead = 1;
                    }
                    if (peer_total < goody_box_detail::native_half(
                            (int)faction.total_mil_veh)) {
                        peer_can_receive = 0;
                        break;
                    }
                }
                if (!peer_ahead) peer_can_receive = 0;
            }
            if (!peer_can_receive) break;

            const int clone = veh_init(Vehs[veh_id].proto_id,
                                       Vehs[veh_id].faction_id, x, y);
            if (clone < 0) break;
            Vehs[clone].home_base_id = -1;
            Vehs[clone].morale = Vehs[veh_id].morale;
            Vehs[clone].dmg_incurred = Vehs[veh_id].dmg_incurred;
            parse_says(0, VehPrototypes[Vehs[veh_id].proto_id].veh_name,
                       -1, -1);
            if (local)
                NetMsgPrimary.pop("GOODYCLONE", -5000, 0, "clone_sm.pcx");
            return 0;
        }
        case 9999: // Unity crash cleanup reached only after the entry guard
            if (local)
                NetMsgPrimary.pop(native_terrain ? "GOODYWHIRLPOOL"
                                                 : "GOODYVANISH",
                                  5000, 0, 0);
            kill(veh_id);
            return 1;
        default:
            break;
        }

        // Unknown/high event ids enter the native overflow arm, whose retry
        // counter guard precedes site-claim side effects.
        if (event_id > 18) {
            if (attempt >= 2) return 0;

            // Only native overflow/default event ids reach this narrow
            // site-claim path. Ordinary failed cases reroll directly.
            const int fallback_allowed =
                ((!native_terrain || (tile->climate & 0xE0u) >= 0x40u)
                 && (!(tile->bit & 0x20u) || (tile->climate & 0xE0u) <= 0x3Fu)
                 && y >= 2 && y < MapLatitudeBounds - 2
                 && !(GameDataStateStorage.fields_.game_rules_ & 0x00010000u));
            if (fallback_allowed) {
                bit_set(x, y, 0x400, TRUE);
                const int existing_bonus = bonus_at(x, y, 0);
                site_radius(x, y, 0);
                if (local && existing_bonus) {
                    if (existing_bonus == 1) wave_it(0x28);
                    else if (existing_bonus == 2) wave_it(0x2A);
                    else if (existing_bonus == 3) {
                        ambience(0xD2);
                        wave_it(0x29);
                    }
                    parse_say(0, Labels.strings_ptr[0x5B + existing_bonus],
                              -1, -1);
                    NetMsgPrimary.pop("GOODYRESOURCE", 5000, 0,
                                      "supply_sm.pcx");
                }
                draw_tile(x, y, 2);
                if (rand() % 3 == 0) return 0;
                if (attempt != 0) return 0;
                attempt = 2;
            }
        }
        // Match the common native retry target, including calls that follow a
        // failed arm which may have changed the source unit's live location.
        x = Vehs[veh_id].x;
        y = Vehs[veh_id].y;
        tile = goody_box_detail::tile_at(x, y);
        if (!goody_box_detail::reroll_event(y, attempt, event_id))
            return 0;
    }
    return 0;
}



// ORIGINAL: 0x00580860 ?propose_proto@@YAHHHHHHHHPAD@Z 0x00580860-0x00581168
// RECOVERY_BINDING: v1 {"evidence": ["Pinned580860..581168:580B89 excludes predefined allocation;580B92 obsolete scoring follows compatible return;580CF5 reuses proposed offense stack value, not faction ID.", "Pinned580F98 inactive-slot flag40;5810B5/5810FA skip negative production IDs;58112D reloads inclusive queue bound; existing VehPrototype, PlayerData and Base owners.", "recovery/tasks/vehicle-special-actions-coverage.json immutable baseline and coordinator-reviewed source; retained git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/00580860.cpp"], "reason": "Recover complete typed prototype proposal with compatibility-first selection, faction-only allocation, native similarity scoring, override staging, live prototype flags, naming and inclusive signed queue rewrites. Preserve original identity and retained seeds; naming/pruning callees remain separately unfinished and no matching or runtime acceptance is claimed."}
// TRIED: Ghidra pseudocode used as primary source (call arg orders for is_bunged, psi_factor x4, has_tech, retire_proto, make_proto, name_proto, prune_protos all independently confirmed against raw disasm push order - none needed fixing), with the one known-recurring bug fixed: DAT_009ab898 is a byte array with real record stride 0x34, not Ghidra's 2-byte-scaled 0x1a (same defect class as 0x004AB600's 0x9ab898 use). Compiles; not chased to byte-exact given size (2312 bytes, dense per-record scoring loop).
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
                    && (GameDataStateStorage.fields_.lock_masks_.enable_mask_ & faction_bit) != 0) {
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
                        || (GameDataStateStorage.fields_.lock_masks_.enable_mask_ & faction_bit) == 0)) {
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
                                    && (comparison_bit & GameDataStateStorage.fields_.lock_masks_.enable_mask_) == 0
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
    if (design_category == -2 || GameDataStateStorage.fields_.diff_level_current_ < 2) {
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
    for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; ++base_id) {
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
// ORIGINAL: 0x0057F1D0 ?retire_proto@@YAXHH@Z 0x0057F1D0-0x0057F4B0 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned57F1D0..57F4B0: category snapshot after proto_bung_put;57F28D current-count guard;57F2B3/2C7 live full mask;57F3BA post-stack prototype reload;57F410 predefined branch bypasses active-bit clearing.", "Native57F434..47F inclusive queue walk, first-slot -69 and nonfirst shift/decrement; descending kill path preserves compacted storage. Existing typed owners in veh/base/faction/net headers.", "recovery/tasks/vehicle-special-actions-coverage.json central reviewed recovery; retained src/recovered/units/0057f1d0.cpp and immutable baseline."], "reason": "Recover complete prototype retirement with native descending-compaction traversal, live visibility mask, base/stack fallback, current prototype mineral refund and inclusive queue cleanup. Preserve source identity, full seed and original spans; this is not matching or runtime acceptance."}
void __cdecl retire_proto(int prototype_id, int faction_id) {
    proto_bung_put(faction_id, prototype_id);
    const int category = static_cast<uint8_t>(VehPrototypes[prototype_id].unk_1);
    int replacement = -1;
    int replacement_score = -1;

    for (int slot = 0; slot < MaxVehProtoFactionNum; ++slot) {
        const int candidate_id = faction_id * MaxVehProtoFactionNum + slot;
        if (candidate_id != prototype_id
            && VehPrototypes[candidate_id].flags & PROTO_ACTIVE
            && static_cast<uint8_t>(VehPrototypes[candidate_id].unk_1) == category) {
            const int score =
                (VehPrototypes[candidate_id].obsolete_factions & (1 << faction_id)) ? 1 : 2;
            if (score > replacement_score) {
                replacement_score = score;
                replacement = candidate_id;
            }
        }
    }

    for (int vehicle_id = GameDataStateStorage.fields_.vehicle_current_count_ - 1;
         vehicle_id >= 0; --vehicle_id) {
        if (vehicle_id >= GameDataStateStorage.fields_.vehicle_current_count_
            || Vehs[vehicle_id].proto_id != prototype_id) {
            continue;
        }
        const uint8_t enabled_factions =
            static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_);
        // A locked faction's units die before the visibility, map-tile or
        // base-stack tests.
        if (!(enabled_factions & (1 << faction_id))) {
            if (replacement >= 0
                && (!(Vehs[vehicle_id].visibility & enabled_factions)
                    || base_who(Vehs[vehicle_id].x, Vehs[vehicle_id].y) > 0)
                && Vehs[vehicle_id].faction_id == faction_id) {
                Vehs[vehicle_id].proto_id = static_cast<int16_t>(replacement);
                PlayersData[faction_id].proto_id_active[prototype_id]--;
                PlayersData[faction_id].proto_id_active[replacement]++;
                continue;
            }
            const int base_id = base_at(Vehs[vehicle_id].x, Vehs[vehicle_id].y);
            if (base_id >= 0) {
                if (stack_check(vehicle_id, 1, -1, -1, -1) == 1) {
                    Vehs[vehicle_id].proto_id = 2;
                    PlayersData[faction_id].proto_id_active[prototype_id]--;
                    PlayersData[faction_id].proto_id_active[2]++;
                    continue;
                }
                Bases[base_id].minerals_accumulated +=
                    VehPrototypes[Vehs[vehicle_id].proto_id].cost / 2;
                Bases[base_id].minerals_accumulated_2 = Bases[base_id].minerals_accumulated;
                Bases[base_id].production_id_last = Bases[base_id].queue_production_id[0];
            }
        }
        kill(vehicle_id);
    }

    if (prototype_id >= MaxVehProtoFactionNum) {
        for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; ++base_id) {
            for (int queue_index = 0; queue_index <= Bases[base_id].queue_size; ++queue_index) {
                if (Bases[base_id].queue_production_id[queue_index] != prototype_id) {
                    continue;
                }
                if (queue_index == 0) {
                    Bases[base_id].queue_production_id[queue_index] = -69;
                    continue;
                }
                for (int shift = queue_index; shift < Bases[base_id].queue_size; ++shift) {
                    Bases[base_id].queue_production_id[shift] =
                        Bases[base_id].queue_production_id[shift + 1];
                }
                Bases[base_id].queue_size--;
            }
        }
        VehPrototypes[prototype_id].flags &= ~PROTO_ACTIVE;
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
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/0057f4b0.cpp; image:0x0057F4B0-0x00580859", "Completion propagation:0x0057F4BF-0x0057F693", "LockEnableMask separates retirement modes, unlocked epilogue returns:0x0057F699-0x0057F6AD;0x0057FCF3-0x0057FCFA", "Lower-reactor early marks:0x0057F7C8-0x0057F7DB;0x0057FA95-0x0057FAA0", "Locked replacement/target banks and non-positive armor bypass:0x0057FD13-0x005802A9;0x0057FEC2-0x0057FFC6", "Queue presence and callback-sensitive prompt/rewrite/unlock/report:0x005802AA-0x00580852"], "reason": "Recover typed completion and both mutually exclusive prototype-retirement modes in the genuine vehicle-design owner. Loaded weapon/armor/chassis rules, faction design banks and active/queued counts determine obsolescence; owned Base queues and NetDaemon locking supply production migration, with native local prompt and multiplayer report ordering. Preserve the complete seed and original spans as evidence; this structural interpretation is not a measured matching claim."}
// TRIED: full goto-per-branch literal transcription of all 1404 instructions (register vars + B/W/D/SB/SW byte-address macros); compiles and matches control flow/field offsets/call order but diverges at instr #0 (prologue/frame layout, MSVC's register allocation for this huge frame is not reproduced by hand).
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
    if ((GameDataStateStorage.fields_.lock_masks_.enable_mask_ & faction_bit) == 0) {
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
    if ((GameDataStateStorage.fields_.lock_masks_.enable_mask_ & faction_bit) != 0
        && (IsMultiplayerNet != 0
            || (GameDataStateStorage.fields_.game_more_preferences_ & MPREF_BSC_AUTO_PRUNE_OBS_VEH) != 0)) {
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
                     queued_base_id < GameDataStateStorage.fields_.base_current_count_; ++queued_base_id) {
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
                for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; ++base_id) {
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

// ORIGINAL: 0x0057CFA0 ?study_artifact@@YAHH@Z 0x0057CFA0-0x0057D1E8 BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason":"Consume the actual vehicle after snapshotting its location/faction, preserve the translator-base bypass and artifact link state, then choose eligible technology through genuine faction/Technology owners. No byte/reference or runtime claim is granted.","evidence":["git:0de05de6:src/unrecovered/0057cfa0.cpp","image:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf","recovery/tasks/scenario-artifact-unity-owner-coverage.json","Original full span and native physical COFF emitted symbol ?study_artifact@@YAHH@Z; existing source owns all referenced storage and services."]}
// TRIED: direct pointer-cast field reads/multiply-based offsets instead of the lea-chain scaling; diverges at instr #3 (prologue push/save order)
// Exact: rewritten from the image (direct Vehs/Bases indexing, flags ternary,
// zero-first random roll).
int __cdecl study_artifact(int veh_id) {
    char message[80];
    message[0] = 0;
    int x = Vehs[veh_id].x;
    int y = Vehs[veh_id].y;
    int faction_id = Vehs[veh_id].faction_id;
    strcat(message, "ARTIFACTTECH");
    int base_id = base_at(x, y);
    if (GameDataStateStorage.fields_.secret_project_.universal_translator() != base_id && base_id >= 0
        && Bases[base_id].state & BSTATE_ARTIFACT_LINKED) {
        if (!(Bases[base_id].state & BSTATE_ARTIFACT_ALREADY_LINKED)
            && faction_id == ConsoleGlobal.faction_id_) {
            parse_says(0, Bases[base_id].name_string, -1, -1);
            NetMsgPrimary.pop("ALREADYARTIFACT", -5000, 0, "art_dis_sm.pcx");
            Bases[base_id].state |= BSTATE_ARTIFACT_ALREADY_LINKED;
        }
        return 0;
    }
    veh_kill(veh_id);
    draw_tile(x, y, 2);
    if (faction_id == ConsoleGlobal.faction_id_) {
        g_FX.play(0x23);
        popp(ScriptPopupFile, message, 0, "art_dis_sm.pcx", 0);
    }
    PlayersData[faction_id].flags = (PlayersData[faction_id].flags & 0x10)
        ? PlayersData[faction_id].flags | 0x4 : PlayersData[faction_id].flags | 0x10;
    if (base_id >= 0) {
        Bases[base_id].state |= BSTATE_ARTIFACT_LINKED;
    }
    int best_score = -1;
    int best_tech = -1;
    for (int tech_id = 0; tech_id < MaxTechnologyNum; tech_id++) {
        if (has_tech(tech_id, faction_id) || !tech_avail(tech_id, faction_id)) {
            continue;
        }
        BOOL has_prereq_1 = has_tech(Technology[tech_id].preq_tech_1, faction_id);
        BOOL has_prereq_2 = has_tech(Technology[tech_id].preq_tech_2, faction_id);
        if (!has_prereq_1 && !has_prereq_2) {
            continue;
        }
        int weight = 8;
        if (!has_prereq_1 || !has_prereq_2) {
            if (!valid_tech_leap(tech_id, faction_id)) {
                continue;
            }
            weight = (11 - PlayersData[faction_id].ranking) * 2;
        }
        int roll = (weight - 1 <= 0) ? 0 : rand() % weight;
        if (roll > best_score) {
            best_score = roll;
            best_tech = tech_id;
        }
    }
    if (best_tech >= 0) {
        tech_achieved(faction_id, best_tech, 0, 0);
    }
    return 1;
}



// consider_designs owner support
struct ConsiderDesignsProposalState {
    VehPrototype &prototype;
    int selected_weapon_id;
    int selected_armor_id;
    int reactor_id;
    int &armor_scale_value;
    ConsiderDesignsProposalState(VehPrototype &prototype_value,
                                 int weapon_id, int armor_id, int reactor,
                                 int &shared_armor_scale_value)
        : prototype(prototype_value), selected_weapon_id(weapon_id),
          selected_armor_id(armor_id), reactor_id(reactor),
          armor_scale_value(shared_armor_scale_value) {}
};
static int consider_designs_weapon_defense_limit(int reactor_id,
                                                 int armor_scale_value,
                                                 int offense_rating) {
    const int numerator = armor_scale_value * (2 << reactor_id) * 4;
    const int quotient = numerator / offense_rating;
    if (quotient < 2 || offense_rating < 2) return 2;
    return quotient > offense_rating ? offense_rating : quotient;
}
static int consider_designs_plan_one_limit(int reactor_id,
                                            int armor_scale_value,
                                            int offense_rating) {
    return consider_designs_weapon_defense_limit(
        reactor_id, armor_scale_value, offense_rating);
}
static bool consider_designs_plan_proposal(int faction_id, int plan,
                                            ConsiderDesignsProposalState &state) {
    VehPrototype &prototype = state.prototype;
    const int chassis_id = prototype.chassis_id;
    const int abilities = static_cast<int>(prototype.ability_flags);
    const int category = prototype.unk_1;
    const int reactor_id = state.reactor_id;
    if (plan == 0 || plan == 4) {
        if (state.selected_weapon_id < 0) return false;
        propose_proto(faction_id, chassis_id, state.selected_weapon_id,
                      prototype.armor_id, abilities, reactor_id,
                      category, 0);
        return true;
    }
    if (plan == 1) {
        int candidate_armor_id = prototype.armor_id;
        if (state.selected_weapon_id >= 0) {
            state.armor_scale_value = state.reactor_id == 1 ? 2 : 1;
            if (state.selected_armor_id >= 0) {
                const int reactor_limit =
                    consider_designs_plan_one_limit(
                        reactor_id,
                        state.armor_scale_value,
                        Weapon[state.selected_weapon_id].offense_rating);
                if (Armor[state.selected_armor_id].defense_rating <=
                    reactor_limit) {
                    candidate_armor_id = state.selected_armor_id;
                }
            }
            propose_proto(faction_id, chassis_id, state.selected_weapon_id,
                          candidate_armor_id, abilities, reactor_id,
                          category, 0);
            return true;
        }
        state.armor_scale_value = state.reactor_id == 1 ? 2 : 1;
        const int offense_rating = Weapon[prototype.weapon_id].offense_rating;
        const int reactor_limit =
            consider_designs_weapon_defense_limit(
                reactor_id, state.armor_scale_value, offense_rating);
        if (Armor[state.selected_armor_id].defense_rating > reactor_limit)
            return false;
        propose_proto(faction_id, chassis_id, prototype.weapon_id,
                      state.selected_armor_id, abilities, reactor_id,
                      category, 0);
        return true;
    }
    if (plan == 2) {
        if (state.selected_armor_id < 0) return false;
        propose_proto(faction_id, chassis_id, prototype.weapon_id,
                      state.selected_armor_id, abilities, reactor_id,
                      category, 0);
        return true;
    }
    if (plan == 6) {
        int candidate_armor_id = prototype.armor_id;
        int candidate_weapon_id = prototype.weapon_id;
        const PlayerData &player_data = PlayersData[faction_id];
        const int design_budget = player_data.ai_power + player_data.ai_growth +
            (reactor_id * reactor_id < 5 ? 5 : reactor_id * reactor_id);
        if (state.selected_weapon_id >= 0) {
            const int weapon_candidate_cost = static_cast<int>(proto_cost(
                chassis_id, state.selected_weapon_id, candidate_armor_id,
                abilities, reactor_id));
            if (weapon_candidate_cost <= design_budget)
                candidate_weapon_id = state.selected_weapon_id;
        }
        if (state.selected_armor_id >= 0) {
            const int armor_candidate_cost = static_cast<int>(proto_cost(
                chassis_id, candidate_weapon_id, state.selected_armor_id,
                abilities, reactor_id));
            if (armor_candidate_cost <= design_budget) {
                candidate_armor_id = state.selected_armor_id;
            } else if (candidate_armor_id != state.selected_armor_id &&
                       candidate_weapon_id != state.selected_weapon_id) {
                return false;
            }
        } else if (candidate_armor_id != state.selected_armor_id &&
                   candidate_weapon_id != state.selected_weapon_id) {
            return false;
        }
        propose_proto(faction_id, chassis_id, candidate_weapon_id,
                      candidate_armor_id, abilities, reactor_id,
                      category, 0);
        return true;
    }
    return false;
}
struct ConsiderDesignsTailInput {
    int prototypes_remaining;
    int weighted_other_faction_score;
    int reactor_id;
    int reactor_capacity;
    int armor_scale_value;
};
struct ConsiderDesignsTailOutput {
    int chassis_id;
    int weapon_id;
    int armor_id;
    int reactor_id;
    uint32_t project_relation_mask;
    uint32_t ability_flags;
    uint32_t plan12_abilities;
    bool plan12_technology_known;
    bool tail_redirects_to_582218;
    bool continue_prototype_loop;
};
static int consider_designs_tail_half(int value) {
    const int adjusted = value - (value < 0 ? -1 : 0);
    return adjusted >> 1;
}
static int consider_designs_tail_defense_limit(int reactor_capacity,
                                                int armor_scale_value,
                                                int armor_cost_cap,
                                                int offense_rating) {
    int limit = ((armor_scale_value * reactor_capacity) * 4) / offense_rating;
    if (limit < 2 || offense_rating < 2) {
        limit = 2;
    } else if (limit > offense_rating) {
        limit = offense_rating;
    }
    const int half_budget = consider_designs_tail_half(armor_cost_cap);
    if (limit <= half_budget) limit = half_budget;
    if (limit < 1) limit = 1;
    if (limit > 0x63) limit = 0x63;
    return limit;
}
static ConsiderDesignsTailOutput consider_designs_strategy_tail(
    int faction_id, const ConsiderDesignsTailInput &input) {
    ConsiderDesignsTailOutput output = {
        -1, -1, -1, input.reactor_id, 0, 0, 0, false, false, false
    };
    if (input.prototypes_remaining != 1) {
        output.continue_prototype_loop = true;
        return output;
    }
    const uint32_t faction_mask = 1u << faction_id;
    const PlayerData &faction_data = PlayersData[faction_id];
    uint32_t other_faction_mask = 0;
    if (Players[faction_id].rule_sharetech <= 0 &&
        (GameDataStateStorage.fields_.lock_masks_.enable_mask_ & faction_mask) == 0) {
        for (int other = 1; other < MaxPlayerNum; ++other) {
            const uint32_t other_mask = 1u << other;
            if ((GameDataStateStorage.fields_.lock_masks_.enable_mask_ & other_mask) == 0) continue;
            int relation_flags = 0;
            const int project_access = great_satan(other, 0);
            if (project_access == 0) {
                if (faction_data.diff_level >= DLVL_THINKER ||
                    (GameDataStateStorage.fields_.game_rules_ & RULES_INTENSE_RIVALRY) != 0) {
                    relation_flags = 0x10;
                }
                relation_flags |= 0x20;
                if ((faction_data.diplo_treaties[other] & relation_flags) == 0)
                    continue;
            }
            const int other_share_tech = Players[other].rule_sharetech;
            if (other_share_tech > 0) {
                other_faction_mask |= 0x3;
                continue;
            }
            const int first_project_base = GameDataStateStorage.fields_.secret_project_.dream_twister();
            if (first_project_base >= 0 &&
                Bases[first_project_base].faction_id_current == other)
                other_faction_mask |= 0x1;
            const int second_project_base = GameDataStateStorage.fields_.secret_project_.neural_amplifier();
            if (second_project_base >= 0 &&
                Bases[second_project_base].faction_id_current == other)
                other_faction_mask |= 0x2;
        }
    }
    output.project_relation_mask = other_faction_mask;
    output.chassis_id = pick_chassis(faction_id, 0, 1);
    int ability_flags = 0;
    int chosen_weapon_id = 0;
    int chosen_armor_id = 0;
    int best_defense_twice = -1;
    int first_armor_id = 0;
    if (output.chassis_id >= 0) {
    for (int armor_id = 0; armor_id < MaxArmorNum; ++armor_id) {
        const RulesArmor &armor = Armor[armor_id];
        if (!has_tech(armor.preq_tech, faction_id)) continue;
        if (armor.cost > 0x63 || armor.defense_rating < 0) continue;
        int defense_value = armor.defense_rating;
        if (faction_id >= 0 && ExpansionEnabled == 0 && armor_id > 9)
            defense_value = 1;
        const int defense_twice = defense_value + defense_value;
        if (defense_twice < best_defense_twice) continue;
        best_defense_twice = defense_twice;
        first_armor_id = armor_id;
    }
    add_abil(&ability_flags, ABL_COMM_JAMMER, faction_id);
    bool trained_gate = faction_data.ai_fight < 0;
    if (!trained_gate && faction_data.soc_effect_pending.values_[SocialEffect::MORALE] < 0 &&
        ((GameDataStateStorage.fields_.lock_masks_.enable_mask_ & faction_mask) == 0 ||
         Players[faction_id].rule_morale < 0)) {
        trained_gate = true;
    }
    if (trained_gate) add_abil(&ability_flags, ABL_TRAINED, faction_id);
    if ((GameDataStateStorage.fields_.lock_masks_.enable_mask_ & faction_mask) == 0 &&
        (faction_data.enemy_best_psi_offense >=
             faction_data.enemy_best_weapon_value * 2 ||
         (other_faction_mask & 0x1) != 0)) {
        add_abil(&ability_flags, ABL_TRANCE, faction_id);
    }
    if (has_tech(Ability[18].preq_tech, faction_id) &&
        (ability_flags == 0 ||
         has_tech(Rules.tech_two_spec_abilities, faction_id))) {
        if ((GameDataStateStorage.fields_.lock_masks_.enable_mask_ & faction_mask) == 0 &&
            (other_faction_mask != 0 ||
             (faction_data.planet_ecology >= 4 &&
              faction_data.soc_effect_pending.values_[SocialEffect::INDUSTRY] <= 0))) {
            ability_flags |= ABL_TRANCE;
        }
    }
    chosen_weapon_id = weapon_budget(faction_id, 1, 0);
    int weapon_budget_cap = 2;
    int current_cost = 0;
    int next_cost = 0;
    for (;;) {
        const int candidate_weapon_id = weapon_budget(faction_id,
                                                      weapon_budget_cap, 0);
        if (candidate_weapon_id == chosen_weapon_id) break;
        current_cost = static_cast<int>(proto_cost(
            output.chassis_id, chosen_weapon_id, first_armor_id,
            ability_flags, input.reactor_id));
        next_cost = static_cast<int>(proto_cost(
            output.chassis_id, candidate_weapon_id, first_armor_id,
            ability_flags, input.reactor_id));
        if (next_cost > current_cost) break;
        chosen_weapon_id = candidate_weapon_id;
        ++weapon_budget_cap;
    }
    chosen_armor_id = first_armor_id;
    if (Armor[chosen_armor_id].defense_rating > 1 ||
        DesignConsiderArmor == 0) {
        propose_proto(faction_id, output.chassis_id, chosen_weapon_id,
                      chosen_armor_id, ability_flags, input.reactor_id,
                      2, 0);
    }
    if (has_tech(Ability[18].preq_tech, faction_id) &&
        (ability_flags & ABL_TRANCE) == 0 &&
        (GameDataStateStorage.fields_.lock_masks_.enable_mask_ & faction_mask) == 0 &&
        ((faction_data.planet_ecology >= 4 &&
          faction_data.soc_effect_pending.values_[SocialEffect::INDUSTRY] <= 0) ||
         (other_faction_mask & 0x3) != 0)) {
        propose_proto(faction_id, output.chassis_id, chosen_weapon_id,
                      chosen_armor_id, ABL_TRANCE, input.reactor_id,
                      0x12, 0);
        ability_flags = ABL_TRANCE;
    }
    if (has_tech(Ability[8].preq_tech, faction_id) &&
        (GameDataStateStorage.fields_.lock_masks_.enable_mask_ & faction_mask) == 0 &&
        (faction_data.unk_66 != 0 || faction_data.unk_68 != 0)) {
        propose_proto(faction_id, output.chassis_id, chosen_weapon_id,
                      chosen_armor_id, ABL_AAA, input.reactor_id,
                      0x14, 0);
        ability_flags = ABL_AAA;
    }
    }
    output.chassis_id = pick_chassis(faction_id, 0, 1);
    if (output.chassis_id < 0) {
        output.weapon_id = chosen_weapon_id;
        output.armor_id = chosen_armor_id;
        output.ability_flags = static_cast<uint32_t>(ability_flags);
        output.plan12_abilities = output.ability_flags;
        output.tail_redirects_to_582218 = true;
        return output;
    }
    ability_flags = 0;
    const int doubled_reactor_capacity = input.reactor_capacity * 2;
    int armor_cost_cap = 1;
    const int initial_division = 2 / doubled_reactor_capacity;
    const int power_target = faction_data.ai_power + 5;
    if (initial_division <= power_target) {
        int candidate_cap = 2;
        for (;;) {
            const int triangular = ((candidate_cap + 1) * candidate_cap) / 2;
            if (triangular / doubled_reactor_capacity > power_target) break;
            ++candidate_cap;
        }
        armor_cost_cap = candidate_cap;
    }
    const int armor_cost_limit =
        (GameDataStateStorage.fields_.lock_masks_.enable_mask_ & faction_mask) != 0
            ? 0x63
            : armor_cost_cap;
    const int budget_weapon_id = weapon_budget(faction_id, armor_cost_limit, 0);
    const int offense_rating = Weapon[budget_weapon_id].offense_rating;
    int armor_defense_limit = consider_designs_tail_defense_limit(
        input.reactor_capacity, input.armor_scale_value, armor_cost_cap,
        offense_rating);
    best_defense_twice = -1;
    chosen_armor_id = 0;
    for (int armor_id = 0; armor_id < MaxArmorNum; ++armor_id) {
        const RulesArmor &armor = Armor[armor_id];
        if (!has_tech(armor.preq_tech, faction_id) ||
            armor.cost > armor_defense_limit || armor.defense_rating < 0)
            continue;
        int defense_value = armor.defense_rating;
        if (faction_id >= 0 && ExpansionEnabled == 0 && armor_id > 9)
            defense_value = 1;
        const int defense_twice = defense_value + defense_value;
        if (defense_twice < best_defense_twice) continue;
        best_defense_twice = defense_twice;
        chosen_armor_id = armor_id;
    }
    const int selected_armor_defense = Armor[chosen_armor_id].defense_rating;
    int weapon_offense_cursor = Weapon[budget_weapon_id].offense_rating;
    int weapon_cost_target = faction_data.ai_power + 5;
    int cost_numerator = (weapon_offense_cursor + 1) * selected_armor_defense;
    int offense_step = selected_armor_defense;
    int denominator = doubled_reactor_capacity;
    while (cost_numerator / denominator <= weapon_cost_target) {
        ++weapon_offense_cursor;
        cost_numerator += offense_step;
    }
    chosen_weapon_id = weapon_budget(faction_id, weapon_offense_cursor, 0);
    if ((GameDataStateStorage.fields_.lock_masks_.enable_mask_ & faction_mask) == 0 &&
        faction_data.unk_66 > faction_data.unk_105 * 2 &&
        faction_data.unk_66 > 1 &&
        faction_data.unk_66 > faction_data.unk_106) {
        add_abil(&ability_flags, ABL_AAA, faction_id);
    }
    if (GameDataStateStorage.fields_.council_resolution_count_[PROP_REPEAL_UN_CHARTER] >
            GameDataStateStorage.fields_.council_resolution_count_[PROP_REINSTATE_UN_CHARTER] &&
        faction_data.ai_fight + faction_data.ai_power > 0 &&
        (GameDataStateStorage.fields_.lock_masks_.enable_mask_ & faction_mask) == 0) {
        add_abil(&ability_flags, ABL_NERVE_GAS, faction_id);
    }
    if (input.weighted_other_faction_score != 0 &&
        input.weighted_other_faction_score >
            1 - faction_data.ai_fight && faction_data.ai_fight >= 0 &&
        (GameDataStateStorage.fields_.lock_masks_.enable_mask_ & faction_mask) == 0) {
        add_abil(&ability_flags, ABL_NERVE_GAS, faction_id);
    }
    if ((GameDataStateStorage.fields_.lock_masks_.enable_mask_ & faction_mask) == 0) {
        const int scaled_armor = faction_data.best_armor_value * 3;
        const int defense_ratio = consider_designs_tail_half(scaled_armor);
        const int enemy_psi_defense = faction_data.enemy_best_psi_defense;
        const int selected_weapon_half = consider_designs_tail_half(
            Weapon[chosen_weapon_id].offense_rating);
        const bool armor_comparison_passed =
            enemy_psi_defense >= defense_ratio;
        const bool add_artillery =
            (armor_comparison_passed &&
             enemy_psi_defense > selected_weapon_half) ||
            other_faction_mask != 0;
        if (add_artillery) add_abil(&ability_flags, ABL_ARTILLERY, faction_id);
    }
    if (faction_data.current_num_bases >= 8 &&
        faction_data.ranking >= 6 &&
        (faction_data.ai_wealth != 0 || faction_data.ai_tech != 0)) {
        add_abil(&ability_flags, ABL_CLEAN_REACTOR, faction_id);
    }
    if (Players[faction_id].rule_morale < 0)
        add_abil(&ability_flags, ABL_TRAINED, faction_id);
    const int budget_weapon_offense = Weapon[budget_weapon_id].offense_rating;
    const int selected_weapon_offense = Weapon[chosen_weapon_id].offense_rating;
    if (budget_weapon_offense > 1 && ability_flags == 0 &&
        faction_data.ai_tech > 0 &&
        faction_data.enemy_best_land_speed > 1 &&
        faction_data.enemy_best_weapon_value >=
            faction_data.best_armor_value * 2) {
        add_abil(&ability_flags, ABL_COMM_JAMMER, faction_id);
    }
    if (ability_flags == 0 &&
        (faction_data.ai_fight < 0 ||
         (faction_data.soc_effect_pending.values_[SocialEffect::MORALE] < 0 &&
          (GameDataStateStorage.fields_.lock_masks_.enable_mask_ & faction_mask) == 0))) {
        add_abil(&ability_flags, ABL_TRAINED, faction_id);
    }
    const bool eligible_for_category_three =
        selected_weapon_offense > 2 ||
        (selected_weapon_offense > 1 && budget_weapon_offense > 1) ||
        DesignConsiderOverride != 0;
    if (!eligible_for_category_three) {
        output.weapon_id = chosen_weapon_id;
        output.armor_id = chosen_armor_id;
        output.reactor_id = input.reactor_id;
        output.ability_flags = static_cast<uint32_t>(ability_flags);
        output.plan12_abilities = output.ability_flags;
        output.tail_redirects_to_582218 = true;
        return output;
    }
    if (has_tech(Ability[10].preq_tech, faction_id) &&
        (ability_flags == 0 ||
         has_tech(Rules.tech_two_spec_abilities, faction_id))) {
        ability_flags |= ABL_ANTIGRAV_STRUTS;
    }
    propose_proto(faction_id, output.chassis_id, chosen_weapon_id,
                  chosen_armor_id, ability_flags, input.reactor_id, 3, 0);
    ability_flags &= ~static_cast<int>(ABL_ANTIGRAV_STRUTS);
    if (!has_tech(Rules.tech_two_spec_abilities, faction_id))
        ability_flags = 0;
    if (has_tech(Ability[4].preq_tech, faction_id) &&
        (ability_flags == 0 ||
         has_tech(Rules.tech_two_spec_abilities, faction_id))) {
        propose_proto(faction_id, output.chassis_id, chosen_weapon_id,
                      chosen_armor_id, ability_flags | ABL_DROP_POD,
                      input.reactor_id, 0xB, 0);
    }
    output.plan12_technology_known =
        has_tech(Ability[3].preq_tech, faction_id) != 0;
    output.tail_redirects_to_582218 = !output.plan12_technology_known;
    output.weapon_id = chosen_weapon_id;
    output.armor_id = chosen_armor_id;
    output.reactor_id = input.reactor_id;
    output.ability_flags = static_cast<uint32_t>(ability_flags);
    output.plan12_abilities = output.plan12_technology_known
        ? (output.ability_flags | ABL_AMPHIBIOUS)
        : output.ability_flags;
    return output;
}
struct ConsiderDesignSelectionState {
    int chassis_id;
    int weapon_id;
    int armor_id;
    int reactor_id;
    int ability_flags;
};
static void consider_designs_plan2_fallback(int faction_id,
                                             ConsiderDesignSelectionState& state) {
    state.ability_flags = 0;
    const int chassis_id = pick_chassis(faction_id, 0, 2);
    if (chassis_id < 0) {
        return;
    }
    int best_armor_score = -1;
    int best_armor_id = 0;
    for (int armor_id = 0; armor_id < MaxArmorNum; ++armor_id) {
        if (!has_tech(Armor[armor_id].preq_tech, faction_id)
            || Armor[armor_id].cost > 1
            || Armor[armor_id].defense_rating < 0) {
            continue;
        }
        int defense_compare;
        if (faction_id >= 0 && !ExpansionEnabled && armor_id > 9) {
            defense_compare = 1;
        } else {
            defense_compare = Armor[armor_id].defense_rating;
        }
        defense_compare *= 2;
        if (defense_compare >= best_armor_score) {
            best_armor_score = defense_compare;
            best_armor_id = armor_id;
        }
    }
    if (has_tech(Ability[0].preq_tech, faction_id)) {
        state.ability_flags |= 0x1;
    }
    if (has_tech(Ability[13].preq_tech, faction_id)
        && (state.ability_flags == 0
            || has_tech(Rules.tech_two_spec_abilities, faction_id))) {
        state.ability_flags |= 0x2000;
    }
    propose_proto(faction_id, chassis_id, 0x12, best_armor_id,
                  static_cast<int>(state.ability_flags), state.reactor_id,
                  -1, 0);
}
static void consider_designs_tech_selection(int faction_id,
                                             ConsiderDesignSelectionState& state,
                                             bool emit_category_c_proposal) {
    if (emit_category_c_proposal
        && has_tech(Ability[3].preq_tech, faction_id)) {
        propose_proto(faction_id, state.chassis_id, state.weapon_id,
                      state.armor_id, state.ability_flags | 0x8,
                      state.reactor_id, 0xC, 0);
    }
    state.weapon_id = weapon_budget(faction_id, 0x63, 0);
    if (!has_tech(Ability[15].preq_tech, faction_id)) {
        return;
    }
    const int picked_chassis = pick_chassis(faction_id, 0, 1);
    state.chassis_id = picked_chassis;
    if (picked_chassis < 0) {
        return;
    }
    int best_armor_score = -1;
    int selected_armor = 0;
    for (int armor_id = 0; armor_id < MaxArmorNum; ++armor_id) {
        if (!has_tech(Armor[armor_id].preq_tech, faction_id)
            || Armor[armor_id].cost > 1
            || Armor[armor_id].defense_rating < 0) {
            continue;
        }
        int defense_compare;
        if (faction_id >= 0 && !ExpansionEnabled && armor_id > 9) {
            defense_compare = 1;
        } else {
            defense_compare = Armor[armor_id].defense_rating;
        }
        defense_compare *= 2;
        if (defense_compare >= best_armor_score) {
            best_armor_score = defense_compare;
            selected_armor = armor_id;
        }
    }
    state.armor_id = selected_armor;
    for (int attempt = 0; attempt < 100; ++attempt) {
        const int current_chassis = state.chassis_id;
        bool chassis_changed = false;
        if (Chassis[current_chassis].speed == 1) {
            const int faster_chassis = pick_chassis(faction_id, 0, 2);
            if (faster_chassis >= 0 && faster_chassis != current_chassis) {
                const int current_chassis_cost = proto_cost(
                    current_chassis, state.weapon_id, state.armor_id,
                    state.ability_flags, state.reactor_id);
                const int faster_chassis_cost = proto_cost(
                    faster_chassis, state.weapon_id, state.armor_id,
                    state.ability_flags, state.reactor_id);
                const int relaxed_limit = state.reactor_id + 5;
                const int parity_limit = current_chassis_cost + (faction_id & 1) + 1;
                if ((faster_chassis_cost <= relaxed_limit
                        && faster_chassis_cost <= parity_limit)
                    || (faster_chassis_cost > relaxed_limit
                        && faster_chassis_cost <= current_chassis_cost)) {
                    state.chassis_id = faster_chassis;
                    chassis_changed = true;
                }
            }
        }
        best_armor_score = -1;
        int replacement_armor = 0;
        for (int armor_id = 0; armor_id < MaxArmorNum; ++armor_id) {
            if (!has_tech(Armor[armor_id].preq_tech, faction_id)
                || Armor[armor_id].cost
                    > Armor[state.armor_id].defense_rating + 1
                || Armor[armor_id].defense_rating < 0) {
                continue;
            }
            int defense_compare;
            if (faction_id >= 0 && !ExpansionEnabled && armor_id > 9) {
                defense_compare = 1;
            } else {
                defense_compare = Armor[armor_id].defense_rating;
            }
            defense_compare *= 2;
            if (defense_compare >= best_armor_score) {
                best_armor_score = defense_compare;
                replacement_armor = armor_id;
            }
        }
        if (Armor[replacement_armor].defense_rating
            > Armor[state.armor_id].defense_rating) {
            const int old_armor_cost = proto_cost(
                state.chassis_id, state.weapon_id, state.armor_id,
                state.ability_flags, state.reactor_id);
            const int new_armor_cost = proto_cost(
                state.chassis_id, state.weapon_id, replacement_armor,
                state.ability_flags, state.reactor_id);
            if (new_armor_cost <= old_armor_cost) {
                state.armor_id = replacement_armor;
            }
        }
        if (!chassis_changed) {
            break;
        }
    }
    if (!has_tech(Ability[15].preq_tech, faction_id)
        || has_tech(Rules.tech_two_spec_abilities, faction_id)) {
        add_abil(&state.ability_flags, 0x20000, faction_id);
        add_abil(&state.ability_flags, 0x400, faction_id);
        add_abil(&state.ability_flags, 0x2, faction_id);
    }
    const int armor_strength = 3 * Armor[state.armor_id].defense_rating;
    if (Weapon[state.weapon_id].offense_rating >= armor_strength
        || has_tech(Ability[15].preq_tech, faction_id)
        || DesignConsiderOverride != 0) {
        propose_proto(faction_id, state.chassis_id, state.weapon_id,
                      state.armor_id, state.ability_flags,
                      state.reactor_id, 1, 0);
    }
}
static void consider_designs_plan3_selection(
    int faction_id, ConsiderDesignSelectionState& state,
    int reactor_cost_scale, int weighted_diplomatic_score,
    int project_relation_mask) {
    int chassis_id = pick_chassis(faction_id, 0, 3);
    if (chassis_id < 0) {
        chassis_id = pick_chassis(faction_id, 0, 2);
    }
    state.chassis_id = chassis_id;
    if (chassis_id < 0) {
        return;
    }
    state.ability_flags = 0;
    const PlayerData& faction_data = PlayersData[faction_id];
    const int pressure_limit = faction_data.ai_power + faction_data.ai_fight
        + 2 * state.reactor_id + 3;
    int max_weapon_cost = 0;
    for (int cost = 4; cost / reactor_cost_scale <= pressure_limit; cost += 2) {
        ++max_weapon_cost;
    }
    const unsigned int faction_lock_bit = 1u << faction_id;
    const bool design_locked =
        ((GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 0xFFu) & faction_lock_bit) != 0;
    if (design_locked) {
        max_weapon_cost = 0x63;
    }
    state.weapon_id = weapon_budget(faction_id, max_weapon_cost, 0);
    int best_armor_score = -1;
    int selected_armor = 0;
    for (int armor_id = 0; armor_id < MaxArmorNum; ++armor_id) {
        if (!has_tech(Armor[armor_id].preq_tech, faction_id)
            || Armor[armor_id].cost > 1
            || Armor[armor_id].defense_rating < 0) {
            continue;
        }
        int defense_compare;
        if (faction_id >= 0 && !ExpansionEnabled && armor_id > 9) {
            defense_compare = 1;
        } else {
            defense_compare = Armor[armor_id].defense_rating;
        }
        defense_compare *= 2;
        if (defense_compare >= best_armor_score) {
            best_armor_score = defense_compare;
            selected_armor = armor_id;
        }
    }
    state.armor_id = selected_armor;
    const int selected_offense = Weapon[state.weapon_id].offense_rating;
    for (int attempt = 0; attempt < 100; ++attempt) {
        if (Armor[state.armor_id].defense_rating >= selected_offense - 1) {
            break;
        }
        int maximum_candidate_cost = selected_offense / 2;
        if (maximum_candidate_cost < Armor[state.armor_id].defense_rating + 1) {
            maximum_candidate_cost = Armor[state.armor_id].defense_rating + 1;
        }
        best_armor_score = -1;
        int replacement_armor = 0;
        for (int armor_id = 0; armor_id < MaxArmorNum; ++armor_id) {
            if (!has_tech(Armor[armor_id].preq_tech, faction_id)
                || Armor[armor_id].cost > maximum_candidate_cost
                || Armor[armor_id].defense_rating < 0) {
                continue;
            }
            int defense_compare;
            if (faction_id >= 0 && !ExpansionEnabled && armor_id > 9) {
                defense_compare = 1;
            } else {
                defense_compare = Armor[armor_id].defense_rating;
            }
            defense_compare *= 2;
            if (defense_compare >= best_armor_score) {
                best_armor_score = defense_compare;
                replacement_armor = armor_id;
            }
        }
        if (Armor[replacement_armor].defense_rating
            <= Armor[state.armor_id].defense_rating) {
            break;
        }
        const int replacement_cost = static_cast<int>(proto_cost(
            state.chassis_id, state.weapon_id, replacement_armor,
            state.ability_flags, state.reactor_id));
        const int cost_limit = faction_data.ai_power + faction_data.ai_fight
            + state.reactor_id + 3;
        if (replacement_cost > cost_limit) {
            break;
        }
        state.armor_id = replacement_armor;
    }
    if (weighted_diplomatic_score != 0
        && weighted_diplomatic_score > 3 - faction_data.ai_fight
        && faction_data.ai_fight >= 0 && !design_locked) {
        add_abil(&state.ability_flags, 0x100000, faction_id);
    }
    if (GameDataStateStorage.fields_.council_resolution_count_[PROP_REPEAL_UN_CHARTER]
            > GameDataStateStorage.fields_.council_resolution_count_[PROP_REINSTATE_UN_CHARTER]
        && faction_data.ai_power + faction_data.ai_fight > 0
        && !design_locked) {
        add_abil(&state.ability_flags, 0x100000, faction_id);
    }
    if (!design_locked && (project_relation_mask & 2) != 0) {
        add_abil(&state.ability_flags, 0x800, faction_id);
    }
    bool add_protective_ability = Players[faction_id].rule_morale < 0;
    if (!add_protective_ability && faction_data.ai_fight > 0
        && faction_data.soc_effect_pending.values_[SocialEffect::INDUSTRY] >= 0) {
        add_protective_ability = true;
    }
    if (!add_protective_ability
        && (faction_data.ai_fight <= 0
            || faction_data.soc_effect_pending.values_[SocialEffect::INDUSTRY] < 0)
        && faction_data.soc_effect_pending.values_[SocialEffect::MORALE] < 0 && !design_locked) {
        add_protective_ability = true;
    }
    if (add_protective_ability) {
        add_abil(&state.ability_flags, 0x4000, faction_id);
    }
    if (has_tech(Ability[10].preq_tech, faction_id)
        && (state.ability_flags == 0
            || has_tech(Rules.tech_two_spec_abilities, faction_id))) {
        state.ability_flags |= 0x400;
    }
    propose_proto(faction_id, state.chassis_id, state.weapon_id,
                  state.armor_id, state.ability_flags, state.reactor_id,
                  4, 0);
}
static bool consider_designs_plan1_ability_selection(
    int faction_id, ConsiderDesignSelectionState& state,
    int project_relation_mask) {
    state.chassis_id = pick_chassis(faction_id, 1, 0);
    if (state.chassis_id < 0) {
        return false;
    }
    state.ability_flags = 0;
    const PlayerData& faction_data = PlayersData[faction_id];
    const unsigned int faction_lock_bit = 1u << faction_id;
    const bool design_locked =
        ((GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 0xFFu) & faction_lock_bit) != 0;
    if (faction_data.unk_66 > faction_data.unk_106
        && faction_data.unk_66 > faction_data.unk_105
        && !design_locked) {
        add_abil(&state.ability_flags, 0x100, faction_id);
    }
    add_abil(&state.ability_flags, 0x2, faction_id);
    add_abil(&state.ability_flags, 0x100, faction_id);
    if ((project_relation_mask & 1) != 0
        && has_tech(Ability[18].preq_tech, faction_id)
        && (state.ability_flags == 0
            || has_tech(Rules.tech_two_spec_abilities, faction_id))) {
        state.ability_flags |= 0x40000;
    }
    if (has_tech(Ability[8].preq_tech, faction_id)
        && (state.ability_flags & 0x100) == 0
        && bit_count(state.ability_flags) < 2
        && (state.ability_flags == 0
            || has_tech(Rules.tech_two_spec_abilities, faction_id))) {
        state.ability_flags |= 0x100;
    }
    if (has_tech(Ability[1].preq_tech, faction_id)
        && (state.ability_flags & 0x2) == 0
        && bit_count(state.ability_flags) < 2
        && (state.ability_flags == 0
            || has_tech(Rules.tech_two_spec_abilities, faction_id))) {
        state.ability_flags |= 0x2;
    }
    return true;
}
static int consider_designs_region3_best_armor(int faction_id, int maximum_cost) {
    int best_score = -1;
    int best_armor = 0;
    for (int armor_id = 0; armor_id < MaxArmorNum; ++armor_id) {
        if (!has_tech(Armor[armor_id].preq_tech, faction_id)
            || Armor[armor_id].cost > maximum_cost
            || Armor[armor_id].defense_rating < 0) {
            continue;
        }
        int defense_compare;
        if (faction_id >= 0 && !ExpansionEnabled && armor_id > 9) {
            defense_compare = 1;
        } else {
            defense_compare = Armor[armor_id].defense_rating;
        }
        defense_compare *= 2;
        if (defense_compare >= best_score) {
            best_score = defense_compare;
            best_armor = armor_id;
        }
    }
    return best_armor;
}
static void consider_designs_weapon_and_armor_choices(
    int faction_id, ConsiderDesignSelectionState& state) {
    const PlayerData& faction_data = PlayersData[faction_id];
    state.weapon_id = weapon_budget(faction_id, 1, 0);
    state.armor_id = consider_designs_region3_best_armor(faction_id, 1);
    for (int attempt = 0; attempt < 100; ++attempt) {
        const int original_weapon = state.weapon_id;
        int candidate_weapon = original_weapon;
        for (int step = 1; step < 4 && candidate_weapon == original_weapon; ++step) {
            const int maximum_offense = Weapon[original_weapon].offense_rating + step;
            candidate_weapon = weapon_budget(faction_id, maximum_offense, 0);
        }
        bool weapon_changed = false;
        if (candidate_weapon != original_weapon) {
            const int reactor_margin = 2 * state.reactor_id - state.reactor_id / 2;
            const int budget_margin = reactor_margin < 5 ? 5 : reactor_margin;
            const int candidate_cost = static_cast<int>(proto_cost(
                state.chassis_id, candidate_weapon, state.armor_id,
                0, state.reactor_id));
            const int cost_limit = faction_data.ai_power + faction_data.ai_growth
                + budget_margin;
            if (candidate_cost <= cost_limit) {
                state.weapon_id = candidate_weapon;
                weapon_changed = true;
            }
        }
        bool armor_changed = false;
        const int current_armor = state.armor_id;
        int candidate_armor = current_armor;
        for (int cost_step = 1; cost_step < 4 && candidate_armor == current_armor;
             ++cost_step) {
            const int maximum_armor_cost =
                Armor[current_armor].defense_rating + cost_step;
            candidate_armor = consider_designs_region3_best_armor(
                faction_id, maximum_armor_cost);
        }
        if (candidate_armor != current_armor
            && Armor[candidate_armor].defense_rating
                > Armor[current_armor].defense_rating) {
            const int reactor_margin = 2 * state.reactor_id - state.reactor_id / 2;
            const int budget_margin = reactor_margin < 5 ? 5 : reactor_margin;
            const int candidate_cost = static_cast<int>(proto_cost(
                state.chassis_id, state.weapon_id, candidate_armor,
                state.ability_flags, state.reactor_id));
            const int cost_limit = faction_data.ai_power + faction_data.ai_growth
                + budget_margin;
            if (candidate_cost <= cost_limit) {
                state.armor_id = candidate_armor;
                armor_changed = true;
            }
        }
        if (!weapon_changed && !armor_changed) {
            break;
        }
    }
    const unsigned int faction_lock_bit = 1u << faction_id;
    const bool design_locked =
        ((GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 0xFFu) & faction_lock_bit) != 0;
    const int category_six_weapon = design_locked
        ? weapon_budget(faction_id, 0x63, 0) : state.weapon_id;
    if (Weapon[category_six_weapon].offense_rating >= 1) {
        propose_proto(faction_id, state.chassis_id, category_six_weapon,
                      state.armor_id, state.ability_flags,
                      state.reactor_id, 6, 0);
    }
    if (design_locked) {
        return;
    }
    state.weapon_id = weapon_budget(faction_id, 1, 0);
    state.armor_id = consider_designs_region3_best_armor(faction_id, 0x63);
    state.ability_flags = 0;
    add_abil(&state.ability_flags, 0x100, faction_id);
    if (has_tech(Ability[1].preq_tech, faction_id)
        && has_tech(Rules.tech_two_spec_abilities, faction_id)) {
        state.ability_flags |= 0x2;
    }
    if (Armor[state.armor_id].defense_rating > 1) {
        propose_proto(faction_id, state.chassis_id, state.weapon_id,
                      state.armor_id, state.ability_flags,
                      state.reactor_id, 0x15, 0);
    }
}
struct ConsiderDesignRegion1State {
    int diplomatic_score;
    int reactor_tier;
    int best_weapon_id;
    int best_armor_id;
    int weapon_cost_limit;
    bool prototype_scan_available;
    uint32_t native_var_18;
};
static int consider_designs_post_58318b_best_armor(int faction_id,
                                                    int maximum_cost) {
    int best_id = 0;
    int best_priority = -1;
    for (int armor_id = 0; armor_id < MaxArmorNum; ++armor_id) {
        const RulesArmor &armor = Armor[armor_id];
        if (!has_tech(armor.preq_tech, faction_id)
            || armor.cost > maximum_cost || armor.defense_rating < 0)
            continue;
        int priority = armor.defense_rating;
        if (faction_id >= 0 && !ExpansionEnabled && armor_id > 9)
            priority = 1;
        priority *= 2;
        if (priority >= best_priority) {
            best_priority = priority;
            best_id = armor_id;
        }
    }
    return best_id;
}
static int consider_designs_middle_best_armor(int faction_id,
                                               int maximum_cost) {
    int best_id = 0;
    int best_priority = -1;
    for (int armor_id = 0; armor_id < MaxArmorNum; ++armor_id) {
        const RulesArmor &armor = Armor[armor_id];
        if (!has_tech(armor.preq_tech, faction_id)
            || armor.cost > maximum_cost || armor.defense_rating < 0)
            continue;
        int priority = armor.defense_rating;
        if (faction_id >= 0 && ExpansionEnabled == 0 && armor_id > 9)
            priority = 1;
        priority *= 2;
        if (priority >= best_priority) {
            best_priority = priority;
            best_id = armor_id;
        }
    }
    return best_id;
}
// RECOVERY_ASSOCIATED: v1 switch_targets 0x00581260 0x00583C90 0x00583CAC 0x00581628
// ORIGINAL: 0x00581260 ?consider_designs@@YAXH@Z 0x00581260-0x00583C8F
// RECOVERY_BINDING: v1 {"reason":"Recover the complete AI prototype-selection operation through typed faction, rules, prototype and proposal owners, with the original signed faction argument and shared armor-scale lifetime across the strategy routes.","evidence":["Pinned original 0x00581260-0x00583C8F; native dispatch plans 0/4,1,2,6 and skipped 3/5; native negative-armor sentinel gate.","Independent full-owner and proposal-budget reviews; retained mechanical seed TRIED records its unverified shared-push argument history."]}
// TRIED: mechanical register/stack-slot goto transliteration (304 labels, one 7-case switch off 0x0094A37D, 6375-line body): compiles, MISMATCH sim 0.80 on /O2 /Oy- framed. Cross-branch shared-push args (26 pushes never traced to a specific call by the linear model) are a known soft spot - values landed but not re-verified by hand past the two spots that broke arg *counts* outright.
// kind      game
void __cdecl consider_designs(int faction_id) {
    ConsiderDesignRegion1State state = {0, 1, -1, -1, 0, false, 0};
FirstProposedPrototype = -1;
state.diplomatic_score = 0;
for (int other_faction = 1; other_faction < MaxPlayerNum; ++other_faction) {
    if (other_faction == faction_id) {
        continue;
    }
    const uint32_t treaty =
        PlayersData[faction_id].diplo_treaties[other_faction];
    if ((treaty & DTREATY_ATROCITY_VICTIM) != 0) {
        state.diplomatic_score +=
            (PlayersData[other_faction].atrocities + 2) / 3;
        continue;
    }
    if ((treaty & DTREATY_WANT_REVENGE) == 0) {
        continue;
    }
    const uint32_t other_bit = 1u << other_faction;
    if ((GameDataStateStorage.fields_.lock_masks_.enable_mask_ & other_bit) != 0) {
        const bool other_ranks_higher =
            PlayersData[other_faction].ranking
                > PlayersData[faction_id].ranking;
        bool add_rivalry_score = other_ranks_higher;
        if (!add_rivalry_score) {
            add_rivalry_score =
                PlayersData[other_faction].diplo_unk4[faction_id]
                    > PlayersData[faction_id].diplo_unk4[other_faction];
        }
        if (add_rivalry_score) {
            state.diplomatic_score += static_cast<int>((GameDataStateStorage.fields_.game_rules_ >> 6) & 1u);
        }
    }
    state.diplomatic_score +=
        (PlayersData[other_faction].integrity_blemishes - 1) / 4;
}
enemy_capabilities(faction_id);
state.reactor_tier = 1;
for (int reactor_index = MaxReactorNum - 1; reactor_index >= 0;
     --reactor_index) {
    if (has_tech(Reactor[reactor_index].preq_tech, faction_id)) {
        state.reactor_tier = reactor_index + 1;
        break;
    }
}
state.best_weapon_id = -1;
state.best_armor_id = -1;
state.weapon_cost_limit = 2 << state.reactor_tier;
for (int weapon_id = 0; weapon_id < MaxWeaponNum; ++weapon_id) {
    const RulesWeapon &weapon = Weapon[weapon_id];
    if (!has_tech(weapon.preq_tech, faction_id)) {
        continue;
    }
    const int offense_rating = weapon.offense_rating;
    if (offense_rating <= 0 || offense_rating >= 0x63 || weapon_id == 23) {
        continue;
    }
    if (state.best_weapon_id >= 0
        && Weapon[state.best_weapon_id].offense_rating > offense_rating) {
        continue;
    }
    for (int candidate_ordinal = 0; candidate_ordinal < 128;
         ++candidate_ordinal) {
        const int prototype_id = candidate_ordinal < MaxVehProtoFactionNum
            ? candidate_ordinal
            : faction_id * MaxVehProtoFactionNum
                + candidate_ordinal - MaxVehProtoFactionNum;
        const VehPrototype &prototype = VehPrototypes[prototype_id];
        if ((prototype.flags & 1u) == 0
            || (prototype.obsolete_factions & (1u << faction_id)) != 0) {
            continue;
        }
        if (prototype_id < MaxVehProtoFactionNum
            && !has_tech(prototype.preq_tech, faction_id)) {
            continue;
        }
        if (Chassis[prototype.chassis_id].missile != 0) {
            continue;
        }
        if (Weapon[prototype.weapon_id].offense_rating >= offense_rating) {
            continue;
        }
        state.best_weapon_id = weapon_id;
    }
}
for (int armor_id = 0; armor_id < MaxArmorNum; ++armor_id) {
    const RulesArmor &armor = Armor[armor_id];
    if (!has_tech(armor.preq_tech, faction_id)) {
        continue;
    }
    const int defense_rating = armor.defense_rating;
    if (defense_rating <= 1) {
        continue;
    }
    if (state.best_armor_id >= 0
        && Armor[state.best_armor_id].defense_rating > defense_rating) {
        continue;
    }
    for (int candidate_ordinal = 0; candidate_ordinal < 128;
         ++candidate_ordinal) {
        const int prototype_id = candidate_ordinal < MaxVehProtoFactionNum
            ? candidate_ordinal
            : faction_id * MaxVehProtoFactionNum
                + candidate_ordinal - MaxVehProtoFactionNum;
        const VehPrototype &prototype = VehPrototypes[prototype_id];
        if ((prototype.flags & 1u) == 0
            || (prototype.obsolete_factions & (1u << faction_id)) != 0) {
            continue;
        }
        if (prototype_id < MaxVehProtoFactionNum
            && !has_tech(prototype.preq_tech, faction_id)) {
            continue;
        }
        if (Armor[prototype.armor_id].defense_rating >= defense_rating) {
            continue;
        }
        state.best_armor_id = armor_id;
    }
}
state.prototype_scan_available = state.best_weapon_id >= 0
    || state.best_armor_id >= 0;
    int shared_armor_scale_value = 0;
    if (state.prototype_scan_available) {
        for (int slot = 0; slot < MaxVehProtoFactionNum; ++slot) {
            const int prototype_id = faction_id * MaxVehProtoFactionNum + slot;
            VehPrototype &prototype = VehPrototypes[prototype_id];
            const uint32_t faction_bit = 1u << faction_id;
            if ((prototype.flags & 1u) == 0
                || (prototype.obsolete_factions & faction_bit) != 0
                || Chassis[prototype.chassis_id].missile != 0) {
                continue;
            }
            if (prototype_id < MaxVehProtoFactionNum
                && !has_tech(prototype.preq_tech, faction_id)) {
                continue;
            }
            ConsiderDesignsProposalState proposal(
                prototype, state.best_weapon_id, state.best_armor_id,
                state.reactor_tier, shared_armor_scale_value);
            consider_designs_plan_proposal(faction_id, prototype.plan,
                                           proposal);
        }
    }
    const ConsiderDesignsTailInput tail_input = {
        1, state.diplomatic_score, state.reactor_tier,
        2 << state.reactor_tier, shared_armor_scale_value
    };
    ConsiderDesignsTailOutput tail =
        consider_designs_strategy_tail(faction_id, tail_input);
    if (tail.continue_prototype_loop) return;
    ConsiderDesignSelectionState selection = {
        tail.chassis_id, tail.weapon_id, tail.armor_id, tail.reactor_id,
        static_cast<int>(tail.ability_flags)
    };
    state.native_var_18 = tail.project_relation_mask;
    consider_designs_tech_selection(
        faction_id, selection, !tail.tail_redirects_to_582218);
    consider_designs_plan3_selection(
        faction_id, selection, 2 << state.reactor_tier,
        state.diplomatic_score, static_cast<int>(state.native_var_18));
int fallback_chassis_id = pick_chassis(faction_id, 0, 3);
if (fallback_chassis_id < 0) {
    fallback_chassis_id = pick_chassis(faction_id, 0, 2);
}
int fallback_ability_flags = 0;
if (fallback_chassis_id < 0) {
    fallback_chassis_id = pick_chassis(faction_id, 0, 1);
    add_abil(&fallback_ability_flags, ABL_ANTIGRAV_STRUTS, faction_id);
}
if (fallback_chassis_id >= 0) {
    const int minimum_weapon_id = weapon_budget(faction_id, 2, 0);
    bool special_weapon_gate_passed = true;
    if (Weapon[minimum_weapon_id].offense_rating < 2
        && DesignConsiderOverride == 0) {
        special_weapon_gate_passed =
            has_tech(Ability[11].preq_tech, faction_id)
            && ((GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 0xFFu) & (1u << faction_id)) == 0;
    }
    if (special_weapon_gate_passed) {
        const int selected_weapon_id = weapon_budget(faction_id, 1, 0);
        int selected_armor_id = 0;
        int best_armor_priority = -1;
        for (int armor_id = 0; armor_id < MaxArmorNum; ++armor_id) {
            const RulesArmor &armor = Armor[armor_id];
            if (!has_tech(armor.preq_tech, faction_id)
                || armor.cost > 1
                || armor.defense_rating < 0) {
                continue;
            }
            int armor_priority = armor.defense_rating;
            if (faction_id >= 0 && !ExpansionEnabled && armor_id > 9) {
                armor_priority = 1;
            }
            armor_priority *= 2;
            if (armor_priority >= best_armor_priority) {
                best_armor_priority = armor_priority;
                selected_armor_id = armor_id;
            }
        }
        const bool has_empathic_tech =
            has_tech(Ability[11].preq_tech, faction_id);
        if (has_empathic_tech
            && ((GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 0xFFu) & (1u << faction_id)) == 0) {
            fallback_ability_flags |= ABL_EMPATHIC;
            if (has_tech(Rules.tech_two_spec_abilities, faction_id)) {
                if (has_tech(Ability[10].preq_tech, faction_id)) {
                    fallback_ability_flags |= ABL_ANTIGRAV_STRUTS;
                } else if (has_tech(Ability[14].preq_tech, faction_id)) {
                    fallback_ability_flags |= ABL_TRAINED;
                }
            }
        } else {
            if (fallback_ability_flags == 0) {
                add_abil(&fallback_ability_flags, ABL_DEEP_RADAR, faction_id);
                if (fallback_ability_flags == 0) {
                    add_abil(&fallback_ability_flags, ABL_CLOAKED, faction_id);
                }
            }
            if (has_tech(Ability[10].preq_tech, faction_id)) {
                if (has_tech(Rules.tech_two_spec_abilities, faction_id)) {
                    fallback_ability_flags |= ABL_ANTIGRAV_STRUTS;
                } else {
                    fallback_ability_flags = ABL_ANTIGRAV_STRUTS;
                }
            }
        }
        const int design_category =
            (fallback_ability_flags & ABL_TRANCE) != 0 ? 19 : 10;
        propose_proto(faction_id, fallback_chassis_id, selected_weapon_id,
                      selected_armor_id, fallback_ability_flags,
                      state.reactor_tier, design_category, 0);
    }
}
    if (consider_designs_plan1_ability_selection(
            faction_id, selection, static_cast<int>(state.native_var_18))) {
        consider_designs_weapon_and_armor_choices(faction_id, selection);
    }
const int sea_chassis_id = pick_chassis(faction_id, TRIAD_SEA, 0);
if (sea_chassis_id >= 0) {
    int selected_armor_id =
        consider_designs_post_58318b_best_armor(faction_id, 1);
    int selected_weapon_id = weapon_budget(faction_id, -7, 0);
    const PlayerData &player_data = PlayersData[faction_id];
    if (player_data.unk_66 > player_data.unk_106
        && player_data.unk_66 > player_data.unk_105) {
        selected_armor_id =
            consider_designs_post_58318b_best_armor(faction_id, 0x63);
    }
    propose_proto(faction_id, sea_chassis_id, selected_weapon_id,
                  selected_armor_id, 0, state.reactor_tier, 7, 0);
    int proposal_chassis_id = sea_chassis_id;
    if (proposal_chassis_id != 3
        && has_tech(Chassis[3].preq_tech, faction_id)) {
        proposal_chassis_id = 3;
    }
    selected_armor_id =
        consider_designs_post_58318b_best_armor(faction_id, 1);
    selected_weapon_id = weapon_budget(faction_id, -9, 0);
    int ability_flags = 0;
    add_abil(&ability_flags, ABL_SUPER_TERRAFORMER, faction_id);
    add_abil(&ability_flags, ABL_FUNGICIDAL, faction_id);
    if (ability_flags == ABL_SUPER_TERRAFORMER
        && has_tech(Ability[13].preq_tech, faction_id)
        && has_tech(Rules.tech_two_spec_abilities, faction_id)) {
        ability_flags |= ABL_FUNGICIDAL;
    }
    if (selected_weapon_id >= 0) {
        propose_proto(faction_id, proposal_chassis_id, selected_weapon_id,
                      selected_armor_id, ability_flags, state.reactor_tier,
                      0xF, 0);
    }
    ability_flags = 0;
    selected_weapon_id = weapon_budget(faction_id, -8, 0);
    if (selected_weapon_id >= 0) {
        propose_proto(faction_id, proposal_chassis_id, selected_weapon_id,
                      selected_armor_id, ability_flags, state.reactor_tier,
                      0x10, 0);
    }
}
int chassis_id = pick_chassis(faction_id, TRIAD_AIR, 0);
if (chassis_id >= 0) {
    int weapon_id = weapon_budget(faction_id, 0x63, 0);
    int armor_id = consider_designs_middle_best_armor(faction_id, 1);
    int ability_flags = 0;
    const PlayerData &faction_data = PlayersData[faction_id];
    const uint32_t faction_bit = 1u << faction_id;
    if (faction_data.diff_level >= 6
        && (faction_data.ai_tech != 0 || faction_data.ai_growth != 0)
        && (faction_data.ai_power != 0 || faction_data.ai_fight < 1)) {
        add_abil(&ability_flags, ABL_CLEAN_REACTOR, faction_id);
    }
    if ((state.native_var_18 & 2) != 0)
        add_abil(&ability_flags, ABL_EMPATHIC, faction_id);
    if (state.diplomatic_score != 0
        && state.diplomatic_score > 5 - faction_data.ai_fight
        && faction_data.ai_fight >= 0
        && ((GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 0xFFu) & (faction_bit & 0xFFu)) == 0) {
        add_abil(&ability_flags, ABL_NERVE_GAS, faction_id);
    }
    if (GameDataStateStorage.fields_.council_resolution_count_[PROP_REPEAL_UN_CHARTER]
            > GameDataStateStorage.fields_.council_resolution_count_[PROP_REINSTATE_UN_CHARTER]
        && faction_data.ai_fight + faction_data.ai_power > 0
        && ((GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 0xFFu) & (faction_bit & 0xFFu)) == 0) {
        add_abil(&ability_flags, ABL_NERVE_GAS, faction_id);
    }
    if (has_tech(Ability[1].preq_tech, faction_id)
        && (ability_flags == 0
            || has_tech(Rules.tech_two_spec_abilities, faction_id))) {
        ability_flags |= ABL_DEEP_RADAR;
    }
    propose_proto(faction_id, chassis_id, weapon_id, armor_id, ability_flags,
                  state.reactor_tier, 8, 0);
    weapon_id = weapon_budget(faction_id, 0x63, 0);
    ability_flags = ABL_AIR_SUPERIORITY;
    if (has_tech(Ability[1].preq_tech, faction_id)
        && has_tech(Rules.tech_two_spec_abilities, faction_id)) {
        ability_flags |= ABL_DEEP_RADAR;
    } else if ((state.native_var_18 & 1) != 0
               && has_tech(Ability[18].preq_tech, faction_id)
               && has_tech(Rules.tech_two_spec_abilities, faction_id)) {
        ability_flags |= ABL_TRANCE;
    }
    propose_proto(faction_id, chassis_id, weapon_id, armor_id, ability_flags,
                  state.reactor_tier, 9, 0);
}
chassis_id = pick_chassis(faction_id, TRIAD_AIR, -2);
if (chassis_id >= 0) {
    const int weapon_id = weapon_budget(faction_id, 0x63, 0);
    const int armor_id = consider_designs_middle_best_armor(faction_id, 1);
    int ability_flags = 0;
    const PlayerData &faction_data = PlayersData[faction_id];
    const uint32_t faction_bit = 1u << faction_id;
    if (faction_data.diff_level >= 6
        && (faction_data.ai_tech != 0 || faction_data.ai_growth != 0)
        && (faction_data.ai_power != 0 || faction_data.ai_fight < 1)) {
        add_abil(&ability_flags, ABL_CLEAN_REACTOR, faction_id);
    }
    if ((state.native_var_18 & 2) != 0)
        add_abil(&ability_flags, ABL_EMPATHIC, faction_id);
    if (state.diplomatic_score != 0
        && state.diplomatic_score > 4 - faction_data.ai_fight
        && faction_data.ai_fight >= 0
        && ((GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 0xFFu) & (faction_bit & 0xFFu)) == 0) {
        add_abil(&ability_flags, ABL_NERVE_GAS, faction_id);
    }
    if (GameDataStateStorage.fields_.council_resolution_count_[PROP_REPEAL_UN_CHARTER]
            > GameDataStateStorage.fields_.council_resolution_count_[PROP_REINSTATE_UN_CHARTER]
        && faction_data.ai_fight + faction_data.ai_power > 0
        && ((GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 0xFFu) & (faction_bit & 0xFFu)) == 0) {
        add_abil(&ability_flags, ABL_NERVE_GAS, faction_id);
    }
    if (has_tech(Ability[1].preq_tech, faction_id)
        && (ability_flags == 0
            || has_tech(Rules.tech_two_spec_abilities, faction_id))) {
        ability_flags |= ABL_DEEP_RADAR;
    }
    propose_proto(faction_id, chassis_id, weapon_id, armor_id, ability_flags,
                  state.reactor_tier, 0x11, 0);
}
chassis_id = pick_chassis(faction_id, TRIAD_AIR, -1);
if (chassis_id >= 0) {
    int armor_id = consider_designs_middle_best_armor(faction_id, 1);
    int ability_flags = 0;
    if (has_tech(Weapon[23].preq_tech, faction_id)) {
        propose_proto(faction_id, chassis_id, 23, armor_id, ability_flags,
                      state.reactor_tier, 0xD, 0);
    }
    if (has_tech(Weapon[24].preq_tech, faction_id)) {
        propose_proto(faction_id, chassis_id, 24, armor_id, ability_flags,
                      state.reactor_tier, 0x16, 0);
    }
    if (has_tech(Weapon[25].preq_tech, faction_id)) {
        propose_proto(faction_id, chassis_id, 25, armor_id, ability_flags,
                      state.reactor_tier, 0x17, 0);
    }
    int regular_weapon_id = -1;
    for (int candidate_weapon_id = 0; candidate_weapon_id < MaxWeaponNum;
         ++candidate_weapon_id) {
        if (!has_tech(Weapon[candidate_weapon_id].preq_tech, faction_id)
            || Weapon[candidate_weapon_id].offense_rating < 0x63) {
            continue;
        }
        regular_weapon_id = candidate_weapon_id;
        break;
    }
    if (regular_weapon_id >= 0) {
        propose_proto(faction_id, chassis_id, regular_weapon_id, armor_id,
                      ability_flags, state.reactor_tier, 0xE, 0);
    }
}
if (state.reactor_tier <= 1
    && !has_tech(Ability[0].preq_tech, faction_id)) {
    if (!has_tech(Ability[13].preq_tech, faction_id)) {
        return;
    }
    chassis_id = pick_chassis(faction_id, TRIAD_LAND, 1);
    if (chassis_id < 0) return;
    const int armor_id = consider_designs_middle_best_armor(faction_id, 1);
    propose_proto(faction_id, chassis_id, 0x12, armor_id, ABL_FUNGICIDAL,
                  state.reactor_tier, -1, 0);
    return;
}
    consider_designs_plan2_fallback(faction_id, selection);
}


// Movement tutorial and terrain-delay counters persist between orders. Native
// accesses at 0x596962 and 0x5979F2 identify these four-byte windows; the
// complete original allocations and original names remain unproved.
// RECOVERY_OBJECT: v1 0x00945AF4 ?OrderVehTutorialNoticeCounter@@3HA
int OrderVehTutorialNoticeCounter = 0;
// RECOVERY_OBJECT: v1 0x00945AF8 ?VehOrderDelayCounter@@3IA
uint32_t VehOrderDelayCounter = 0;

// Vehicle movement owner helpers.
// Native movement corridors are outlined here without changing their shared reaching state.
struct OrderVehDestination {
    int x;
    int y;
    int visible_unit_id;
    int base_id;
    int controlling_faction;
    int goody_type;
    const Map *tile;
    uint8_t region;
};

static bool order_veh_direction8_stack_allows_popup(int veh_id) {
    int stack_veh_id = veh_id;
    if (stack_veh_id >= 0) {
        while (Vehs[stack_veh_id].prev_veh_id_stack >= 0) {
            stack_veh_id = Vehs[stack_veh_id].prev_veh_id_stack;
        }
    }

    while (stack_veh_id >= 0) {
        const Veh &stack_vehicle = Vehs[stack_veh_id];
        if (has_abil(stack_vehicle.proto_id, ABL_CARRIER)) {
            return false;
        }

        const VehPrototype &prototype = VehPrototypes[stack_vehicle.proto_id];
        if (prototype.plan == PLAN_SUPPLY_CONVOY
            && Chassis[prototype.chassis_id].triad == TRIAD_AIR) {
            return false;
        }

        stack_veh_id = stack_vehicle.next_veh_id_stack;
    }
    return true;
}

static bool order_veh_artifact_announcement(int veh_id, int base_id,
                                            int action_faction,
                                            int stored_action_mode) {
    Veh &vehicle = Vehs[veh_id];
    const VehPrototype &prototype = VehPrototypes[vehicle.proto_id];
    if (prototype.plan != PLAN_ALIEN_ARTIFACT) return false;
    if ((vehicle.flags & 0x20U) != 0) return false;
    if ((stored_action_mode & 1) == 0) return false;

    vehicle.terraforming_turns = 0;
    if (base_id < 0) return false;
    if (Bases[base_id].faction_id_current != action_faction) return false;

    parse_says(0, Bases[base_id].name_string, -1, -1);
    return true;
}

static void order_veh_start_artifact_popup(Popup &popup) {
    popup.start(PopupStartCaption, "ARTIFACT", -1, NULL, 0, NULL);
}

static void order_veh_artifact_picture(Popup &popup, Sprite &sprite,
                                       const char *dimensions_path,
                                       const char *sprite_path) {
    int width;
    int height;
    if (Buffer::get_pcx_dimensions(dimensions_path, &width, &height) != 0) {
        return;
    }
    if (sprite.init(const_cast<char *>(sprite_path), width, height) == 0) {
        popup.set_caller_picture(&sprite);
    }
}

static bool order_veh_artifact_dialog_item(Popup &popup, char *file_name,
                                           const char *section, int row) {
    StringTemp[0] = '\0';
    if (X_text_open(file_name, section) != 0) return false;
    LPSTR row_text = text_get();
    parse_string(row_text, StringTemp);
    popup.dialogs().item(StringTemp, row);
    return true;
}

static bool order_veh_direction8_popup_gate(int veh_id, int action_faction,
                                            int destination_x,
                                            int destination_y, int mode) {
    const Veh &vehicle = Vehs[veh_id];
    const VehPrototype &prototype = VehPrototypes[vehicle.proto_id];
    const RulesChassis &chassis = Chassis[prototype.chassis_id];
    if (chassis.triad != TRIAD_AIR || chassis.range == 0) return false;
    if ((mode & 1) == 0 || action_faction != ConsoleGlobal.faction_id_) {
        return false;
    }

    const Map &destination = *map_loc(destination_x, destination_y);
    if ((destination.bit & 1U) != 0) {
        if ((destination.val2 & 0x0FU) < 8U) return false;
    }
    if ((destination.bit & BIT_AIRBASE) != 0) return false;

    if (!order_veh_direction8_stack_allows_popup(veh_id)) return false;

    const int remaining = speed(veh_id, 0) - vehicle.moves_expended;
    if (remaining < 0 || remaining == 0) return false;
    if (chassis.range != 1 && vehicle.terraforming_turns == 0) return false;

    action_home(veh_id, 0); // Native result is ignored; order is reread below.
    return Vehs[veh_id].order == ORDER_MOVE_TO;
}

static void order_veh_artifact_cost_numbers(int base_id, int faction_id) {
    Base &base = Bases[base_id];
    parse_num(0, base.minerals_accumulated);

    int production_cost;
    const int queue_item = base.queue_production_id[0];
    if (queue_item < 0) {
        const int facility_id = -queue_item;
        production_cost = cost_factor(faction_id, 1, -1)
                        * Facility[facility_id].cost;
    } else {
        production_cost = veh_cost(queue_item, base_id, NULL)
                        * cost_factor(faction_id, 1, -1);
    }
    parse_num(1, production_cost);
}

static bool order_veh_waypoint_treaty_clears_order(
    int veh_id, int action_faction, bool preceding_action_mask_selected) {
    if (preceding_action_mask_selected) return false;
    Veh &vehicle = Vehs[veh_id];
    if (vehicle.order != ORDER_MOVE_TO) return false;
    const int owner = whose_territory(action_faction,
                                      vehicle.waypoint_x[0],
                                      vehicle.waypoint_y[0], 0, 0);
    if (owner < 0 || owner == action_faction) return false;
    if ((PlayersData[action_faction].diplo_treaties[owner] & 0xC11U) != 0) {
        return false;
    }
    vehicle.order = ORDER_NONE;
    return true;
}

static void order_veh_artifact_dialog_rows(
    Popup &popup, int veh_id, int base_id, int action_faction) {
    Base &base = Bases[base_id];
    Veh &vehicle = Vehs[veh_id];
    int row_count = 1;

    LPSTR row_text = text_get();
    int byte_offset;
    int bit_mask;
    bitmask(8, &byte_offset, &bit_mask);
    if ((base.facilities_built[byte_offset] & bit_mask) != 0 &&
        GameDataStateStorage.fields_.secret_project_.universal_translator() != base_id &&
        (base.state & 0x800U) == 0) {
        StringTemp[0] = '\0';
        parse_string(row_text, StringTemp);
        popup.dialogs().item(StringTemp, 1);
        row_count = 2;
        if ((GameDataStateStorage.fields_.lock_masks_.enable_mask_ & (1U << (action_faction & 0x1f))) == 0) {
            vehicle.terraforming_turns = 1;
        }
    }

    row_text = text_get();
    if (GameDataStateStorage.fields_.secret_project_.universal_translator() == base_id) {
        StringTemp[0] = '\0';
        parse_string(row_text, StringTemp);
        popup.dialogs().item(StringTemp, 1);
        ++row_count;
        if ((GameDataStateStorage.fields_.lock_masks_.enable_mask_ & (1U << (action_faction & 0x1f))) == 0) {
            vehicle.terraforming_turns = 1;
        }
    }

    row_text = text_get();
    const int queue_item = base.queue_production_id[0];
    if (queue_item <= -0x46) {
        int facility_id = queue_item;
        if (facility_id <= -1) facility_id = -facility_id;
        StringTemp[0] = '\0';
        parse_say(1, Facility[facility_id].name, -1, -1);
        parse_string(row_text, StringTemp);
        popup.dialogs().item(StringTemp, 2);
        ++row_count;
        if ((GameDataStateStorage.fields_.lock_masks_.enable_mask_ & (1U << (action_faction & 0x1f))) == 0) {
            vehicle.terraforming_turns = 2;
        }
    }

    row_text = text_get();
    if (queue_item >= 0x40 &&
        (VehPrototypes[queue_item].flags & 4U) == 0) {
        StringTemp[0] = '\0';
        parse_says(2, VehPrototypes[queue_item].veh_name, -1, -1);
        parse_string(row_text, StringTemp);
        popup.dialogs().item(StringTemp, 2);
        ++row_count;
    }

    if (row_count >= 2 && action_faction == ConsoleGlobal.faction_id_ &&
        (GameDataStateStorage.fields_.lock_masks_.enable_mask_ & (1U << (action_faction & 0x1f))) != 0) {
        vehicle.terraforming_turns = static_cast<uint8_t>(popup.exec(0, NULL));
        synch_veh(veh_id);
        NetDaemonNet.await_synch();
    }
}

enum OrderVehDisembarkDialogResult {
    OrderVehDisembarkDialogCleanup,
    OrderVehDisembarkDialogSelectedAt5962BE
};
static char OrderVehDisembarkTextFile[] = "DISEMBARKERS";

static OrderVehDisembarkDialogResult order_veh_disembark_dialog(
    Popup &popup, int veh_id, int &selected_vehicle_id) {
    Veh &vehicle = Vehs[veh_id];
    vehicle.order = ORDER_NONE;
    parse_says(0,
        VehPrototypes[vehicle.proto_id].veh_name, -1, -1);
    popup.start(PopupStartCaption, "DISEMBARK", -1, NULL, 0, NULL);

    if (X_text_open(ScriptPopupFile, OrderVehDisembarkTextFile) != 0) {
        return OrderVehDisembarkDialogCleanup;
    }

    LPSTR row_template = text_get();
    stack_veh(veh_id, 0);
    int passenger = veh_id;
    int last_moving_passenger = -1;
    int attached_passengers = 0;
    if (passenger >= 0) {
        while (Vehs[passenger].prev_veh_id_stack >= 0) {
            passenger = Vehs[passenger].prev_veh_id_stack;
        }
        do {
            Veh &stacked = Vehs[passenger];
            if (stacked.order == ORDER_SENTRY_BOARD &&
                stacked.waypoint_x[0] == veh_id) {
                ++attached_passengers;
                const int current_speed = speed(passenger, FALSE);
                const int remaining_speed =
                    current_speed - stacked.moves_expended;
                if (remaining_speed >= 0 &&
                    (remaining_speed > 999 ||
                     current_speed != stacked.moves_expended)) {
                    parse_says(1,
                        VehPrototypes[stacked.proto_id].veh_name, -1, -1);
                    StringTemp[0] = '\0';
                    parse_string(row_template, StringTemp);
                    popup.dialogs().item(StringTemp, passenger + 1);
                    last_moving_passenger = passenger;
                }
            }
            passenger = stacked.next_veh_id_stack;
        } while (passenger >= 0);
    }

    if (last_moving_passenger >= 0) {
        selected_vehicle_id = popup.exec(0, NULL) - 1;
        if (selected_vehicle_id >= 0) {
            Vehs[selected_vehicle_id].order = ORDER_NONE;
            return OrderVehDisembarkDialogSelectedAt5962BE;
        }
    } else if (attached_passengers != 0) {
        NetMsgPrimary.pop("DISEMBARKFAIL", 5000, 0, NULL);
    }
    return OrderVehDisembarkDialogCleanup;
}

enum OrderVehNoBaseInfoPath {
    OrderVehNoBaseInfoContinue595083,
    OrderVehNoBaseInfoNetworkReturnOne,
    OrderVehNoBaseInfoCountChangedCleanup
};

extern int __cdecl probe(int veh_id, int target_veh_id,
                         int target_base_id, int action);

static void order_veh_local_stack_animation(int veh_id, int direction,
                                            int original_x, int original_y,
                                            int destination_x,
                                            int destination_y, int mode) {
    ConsoleGlobal.focus(original_x, original_y, mode);
    if (IsMultiplayerNet == 0) clock_wait(0x190);
    stack_veh(veh_id, 1);
    veh_scoot(veh_id, original_x, original_y, direction, 0);
    stack_put(veh_id, original_x, original_y);
    draw_tile(destination_x, destination_y, 2);
}

static bool order_veh_direction8_probe_sync(int veh_id, int base_id,
                                            int order_mode) {
    if (probe(veh_id, base_id, -1, 1) == 0) return false;
    if (IsMultiplayerNet == 0) return false;
    if (ConsoleControlTurnActive != 0) return false;
    if ((order_mode & 1) == 0 || (order_mode & 2) != 0) return false;
    synch_veh(veh_id);
    NetDaemonNet.await_synch();
    return true;
}

static bool order_veh_zoc_branch_gate(
    const VehPrototype &prototype, const RulesChassis &chassis,
    bool source_is_lowland,
    bool destination_is_lowland, int selected_base_id) {
    if (source_is_lowland || destination_is_lowland) return false;
    if (chassis.triad != TRIAD_LAND) return false;
    if ((prototype.ability_flags & 4U) != 0) return false;
    if (prototype.plan == PLAN_INFO_WARFARE) return false;
    if (selected_base_id >= 0) return false;
    return true;
}

static bool order_veh_zoc_tutorial_notice(
    int faction_id, int native_local_gate_value, int action_mode, int veh_id,
    int source_x, int source_y, int destination_x, int destination_y,
    int &tutorial_notice_counter) {
    if (faction_id != ConsoleGlobal.faction_id_) return false;
    if (native_local_gate_value != 0 || (action_mode & 1) == 0) {
        return false;
    }

    g_FX.play(13);
    if (faction_id != ConsoleGlobal.faction_id_) return !IsMultiplayerNet;
    if (IsMultiplayerNet) return false;

    if ((GameDataStateStorage.fields_.game_preferences_ & 0x20) == 0) {
        if (PlayersData[faction_id].diff_level > 1) return true;
        ++tutorial_notice_counter;
        if ((tutorial_notice_counter & 3) != 1) return true;
    }

    const int ability_id = abil_index(4);
    parse_say(0, Ability[ability_id].name, -1, -1);

    int forward_zoc_x;
    int forward_zoc_y;
    int reverse_zoc_x;
    int reverse_zoc_y;
    quick_zoc(source_x, source_y, faction_id, destination_x, destination_y,
              &forward_zoc_x, &forward_zoc_y);
    quick_zoc(destination_x, destination_y, faction_id, source_x, source_y,
              &reverse_zoc_x, &reverse_zoc_y);
    TutorialWindow.tut_map("ZOCMSG", forward_zoc_x, forward_zoc_y, veh_id,
                           NULL, 0, reverse_zoc_x, reverse_zoc_y);
    return !IsMultiplayerNet;
}

static bool order_veh_reveal_unseen_hostiles(
    int action_mode, int origin_x, int origin_y,
    uint8_t native_visibility_mask) {
    bool spotted_any = false;
    for (int direction = 0; direction < 8; ++direction) {
        const int x = wrap_map_x(origin_x + VehMoveDeltaX[direction]);
        const int y = origin_y + VehMoveDeltaY[direction];
        if (y < 0 || y >= MapLatitudeBounds ||
            x < 0 || x >= MapLongitudeBounds) {
            continue;
        }

        const int top_vehicle = stack_fix(veh_at(x, y));
        if (top_vehicle < 0) continue;

        const uint8_t enemy_faction = Vehs[top_vehicle].faction_id;
        if (enemy_faction == action_mode) continue;
        if ((PlayersData[action_mode].diplo_treaties[enemy_faction] & 1U) != 0) {
            continue;
        }
        if ((native_visibility_mask & Vehs[top_vehicle].visibility) != 0) {
            continue;
        }

        spot_loc(x, y, action_mode);
        spotted_any = true;
    }

    if (spotted_any) draw_radius(origin_x, origin_y, 1, 2);
    return spotted_any;
}

enum OrderVehSupplyOptionsPath {
    OrderVehSupplyOptionsCommonCleanupReturnZero,
    OrderVehSupplyOptionsContinue595422,
    OrderVehSupplyOptionsSyncedReturnOne
};

static OrderVehSupplyOptionsPath order_veh_supply_options_join(
    int veh_id, int proto_plan, int selected_base_id, int action_mode) {
    if (proto_plan != PLAN_SUPPLY_CONVOY || selected_base_id < 0 ||
        IsMultiplayerNet) {
        return OrderVehSupplyOptionsContinue595422;
    }

    const int options_shown = supply_options(veh_id, selected_base_id);
    if ((NetDaemonNet.channels_[0].leading_words_[3] & 2) != 0) {
        return options_shown != 0
            ? OrderVehSupplyOptionsCommonCleanupReturnZero
            : OrderVehSupplyOptionsContinue595422;
    }
    if (options_shown == 0) {
        return OrderVehSupplyOptionsCommonCleanupReturnZero;
    }

    if (IsMultiplayerNet && ConsoleControlTurnActive == 0) {
        synch_veh(veh_id);
        NetDaemonNet.await_synch();
        return OrderVehSupplyOptionsSyncedReturnOne;
    }
    return OrderVehSupplyOptionsContinue595422;
}

enum OrderVeh595422Path {
    OrderVeh595422Continue59512c,
    OrderVeh595422Continue595137,
    OrderVeh595422Continue596052,
    OrderVeh595422CommonCleanup,
    OrderVeh595422SpawnedCommonCleanup
};

static OrderVeh595422Path order_veh_595422_supply_spawn(
    int veh_id, int action_mode, int action_faction,
    int destination_x, int destination_y,
    bool source_is_lowland, bool destination_is_lowland,
    int &target_vehicle, int &chance_slot) {
    const Veh &vehicle = Vehs[veh_id];

    if ((action_mode & 2) == 0 ||
        Chassis[VehPrototypes[vehicle.proto_id].chassis_id].triad == TRIAD_AIR) {
        return OrderVeh595422Continue59512c;
    }
    if (target_vehicle >= 0) return OrderVeh595422Continue595137;
    if (action_faction == 0) return OrderVeh595422Continue59512c;

    const Map &destination = MapTiles[
        MapLongitude * destination_y + (destination_x >> 1)];
    if ((destination.bit & 0x20U) == 0 ||
        (destination.climate & 0xE0U) < 0x40U ||
        (destination.bit & 0x08U) != 0 ||
        source_is_lowland != destination_is_lowland) {
        return OrderVeh595422Continue59512c;
    }

    const RulesWeapon &weapon = Weapon[VehPrototypes[vehicle.proto_id].weapon_id];
    if ((weapon.offense_rating == 0 || vehicle.order == ORDER_MOVE_TO) &&
        GameDataStateStorage.fields_.secret_project_.xenoempathy_dome() >= 0 &&
        Bases[GameDataStateStorage.fields_.secret_project_.xenoempathy_dome()].faction_id_current ==
            action_faction) {
        return OrderVeh595422Continue59512c;
    }
    if (goody_at(destination_x, destination_y) != 0 ||
        (GameDataStateStorage.fields_.game_rules_ & 0x00200000U) != 0) {
        return OrderVeh595422Continue59512c;
    }

    (void)base_find(destination_x, destination_y);
    if (BaseFindDist <= 1) return OrderVeh595422Continue59512c;

    int chance = BaseFindDist;
    if (chance < 0) chance = 0;
    if (chance > 32) chance = 32;
    chance += static_cast<int>(MapNativeLifeForms) * 10;
    if ((destination.bit & 0x00100000U) != 0) {
        chance /= 2;
        if (MapNativeLifeForms == 0) chance /= 2;
    }
    if (destination_is_lowland) {
        chance /= 2;
        if (MapNativeLifeForms == 0) chance /= 2;
    }
    if ((destination.bit & 4U) != 0) chance /= 2;
    if (vehicle.proto_id < 64 &&
        (weapon.offense_rating < 0 || vehicle.proto_id == 15)) chance /= 2;
    chance_slot = chance;

    for (int offset_index = 0; offset_index < 100; offset_index += 4) {
        int x = destination_x + RadiusOffsetX[offset_index / 4];
        if ((MapIsFlat & 1) == 0) {
            if (x >= 0) {
                if (x >= MapLongitudeBounds) x -= MapLongitudeBounds;
            } else {
                x += MapLongitudeBounds;
            }
        }
        const int y = destination_y + RadiusOffsetY[offset_index / 4];
        if (y < 0 || y >= MapLatitudeBounds || x < 0 ||
            x >= MapLongitudeBounds) {
            continue;
        }
        const Map &candidate = MapTiles[MapLongitude * y + (x >> 1)];
        if (is_sensor(x, y) != 0 ||
            ((candidate.bit & 1U) != 0 && (candidate.val2 & 0x0FU) <= 7U)) {
            if (offset_index >= 0x24) {
                chance /= 2;
            } else {
                chance = (chance + ((chance >> 31) & 3)) >> 2;
            }
        }
    }

    chance_slot = chance;
    if (chance <= rand() % 100) return OrderVeh595422Continue59512c;

    int spawned_prototype = 9;
    if (!destination_is_lowland) {
        spawned_prototype = (rand() % 5 == 0 && ExpansionEnabled) ? 15 : 8;
    }
    const int spawned_vehicle = target_vehicle =
        veh_init(spawned_prototype, 0, destination_x, destination_y);
    if (spawned_vehicle < 0) {
        return OrderVeh595422Continue596052;
    }

    Vehs[veh_id].order = ORDER_NONE;
    spot_stack(spawned_vehicle, action_faction);
    draw_tile(destination_x, destination_y, 2);
    bit_set(destination_x, destination_y, 0x00100000, 1);
    if (action_faction == ConsoleGlobal.faction_id_) {
        g_FX.play(13);
        parse_says(0, Players[action_faction].adj_name_faction, -1, -1);
        parse_says(1,
            VehPrototypes[Vehs[spawned_vehicle].proto_id].veh_name,
            -1, -1);
        NetMsgPrimary.pop(destination_is_lowland ? "ISLESPOTTED" :
                              "WORMSSPOTTED",
                          5000, 0, NULL);
    }
    return OrderVeh595422SpawnedCommonCleanup;
}

enum OrderVeh595137Path {
    OrderVeh595137CommonCleanup,
    OrderVeh595137Continue596052,
    OrderVeh595137Continue59521F
};

static OrderVeh595137Path order_veh_595137_gate(
    int action_vehicle_id, int visible_vehicle_id, int action_faction,
    int destination_controller, int destination_x, int destination_y,
    int selected_base_id, bool native_probe_attempted, int action_mode) {
    Veh &action_vehicle = Vehs[action_vehicle_id];
    Veh &visible_vehicle = Vehs[visible_vehicle_id];
    const VehPrototype &action_prototype =
        VehPrototypes[action_vehicle.proto_id];

    const bool visible_is_flagged = (visible_vehicle.flags & 0x400U) != 0;
    if (!visible_is_flagged) {
        const bool same_faction = action_faction == destination_controller;
        const bool relation_allows = !same_faction &&
            (PlayersData[action_faction]
                 .diplo_treaties[destination_controller] & 1U) != 0;
        const bool info_warfare_without_flag =
            action_prototype.plan == PLAN_INFO_WARFARE &&
            (action_vehicle.flags & 4U) != 0;
        if (same_faction || (relation_allows && !info_warfare_without_flag)) {
            return OrderVeh595137Continue596052;
        }
    }

    if (native_probe_attempted) return OrderVeh595137CommonCleanup;

    if (visible_is_flagged) {
        action_vehicle.order = ORDER_NONE;
        spot_stack(visible_vehicle_id, action_faction);
        draw_tile(destination_x, destination_y, 2);
        if (action_faction == ConsoleGlobal.faction_id_) {
            g_FX.play(13);
            parse_says(0, Players[destination_controller].adj_name_faction,
                       -1, -1);
            parse_says(1,
                       VehPrototypes[visible_vehicle.proto_id].veh_name,
                       -1, -1);
            NetMsgPrimary.pop(destination_controller == 0 ? "SPOTTED0" : "SPOTTED",
                              5000, 0, NULL);
        }
        return OrderVeh595137CommonCleanup;
    }

    const unsigned int faction_bit =
        1U << (static_cast<unsigned int>(action_faction) & 0x1fU);
    const bool visible_is_other_actionable_stack =
        (visible_vehicle.faction_id != action_faction ||
         (visible_vehicle.flags & 0x440U) == 0x440U) &&
        (visible_vehicle.visibility & faction_bit) == 0 &&
        (GameDataStateStorage.fields_.lock_masks_.enable_mask_ & faction_bit) != 0;
    if (visible_is_other_actionable_stack && selected_base_id < 0 &&
        (action_mode & 1) != 0 &&
        action_faction == ConsoleGlobal.faction_id_) {
        action_vehicle.order = ORDER_NONE;
        spot_stack(visible_vehicle_id, action_faction);
        draw_tile(destination_x, destination_y, 2);
        g_FX.play(13);
        parse_says(0, Players[destination_controller].adj_name_faction, -1, -1);
        parse_says(1, VehPrototypes[visible_vehicle.proto_id].veh_name,
                   -1, -1);
        NetMsgPrimary.pop(destination_controller == 0 ? "SPOTTED0" : "SPOTTED",
                          5000, 0, NULL);
        return OrderVeh595137CommonCleanup;
    }

    return OrderVeh595137Continue59521F;
}

enum OrderVeh59521fPath {
    OrderVeh59521fCommonCleanup,
    OrderVeh59521fContinue595798
};

static OrderVeh59521fPath order_veh_59521f_front_gate(
    int action_vehicle_id, int visible_vehicle_id, int action_faction,
    int selected_base_id, const Map &destination_tile, int action_mode) {
    Veh &vehicle = Vehs[action_vehicle_id];
    if (selected_base_id < 0 &&
        (destination_tile.bit & 0x00040000U) == 0 &&
        stack_check(visible_vehicle_id, 0x12, -1, -1, -1) != 0 &&
        has_abil(vehicle.proto_id, 0x20) == 0 &&
        stack_check(visible_vehicle_id, 6, 0x80, -1, -1) == 0) {
        if (action_faction == ConsoleGlobal.faction_id_ &&
            (action_mode & 1) != 0) {
            const int ability_id = abil_index(0x20);
            parse_say(0, Ability[ability_id].name, -1, -1);
            NetMsgPrimary.pop("FIGHTERRULE", 5000, 0, NULL);
        }
        if ((vehicle.state & 0x00004200U) != 0) veh_skip(action_vehicle_id);
        return OrderVeh59521fCommonCleanup;
    }

    return OrderVeh59521fContinue595798;
}

static bool order_veh_psibomb_notice(int, int);
static bool order_veh_amphibious_tile_notice(int, int, const Map &);
enum OrderVeh595798Path {
    OrderVeh595798CommonCleanup,
    OrderVeh595798Continue5957D7
};

static OrderVeh595798Path order_veh_595798_artillery_amphib(
    int action_vehicle_id, int action_faction,
    bool source_is_lowland, bool destination_is_lowland,
    int saved_triad, const Map &destination_tile, int action_mode) {
    Veh &vehicle = Vehs[action_vehicle_id];
    const VehPrototype &prototype = VehPrototypes[vehicle.proto_id];

    if (vehicle.proto_id != 0x0e && source_is_lowland &&
        !destination_is_lowland && saved_triad == TRIAD_SEA &&
        can_arty(vehicle.proto_id, 1) == 0 &&
        Weapon[VehPrototypes[vehicle.proto_id].weapon_id].offense_rating != 0) {
        order_veh_psibomb_notice(action_faction, action_mode);
        return OrderVeh595798CommonCleanup;
    }

    if (source_is_lowland && saved_triad == TRIAD_LAND &&
        has_abil(vehicle.proto_id, ABL_AMPHIBIOUS) == 0 &&
        VehPrototypes[vehicle.proto_id].plan != PLAN_INFO_WARFARE) {
        order_veh_amphibious_tile_notice(
            action_faction, action_mode, destination_tile);
        return OrderVeh595798CommonCleanup;
    }

    return OrderVeh595798Continue5957D7;
}

static bool order_veh_psibomb_notice(int action_faction, int action_mode) {
    if (action_faction != ConsoleGlobal.faction_id_ || (action_mode & 1) == 0) {
        return false;
    }
    NetMsgPrimary.pop("PSIBOMB", 5000, 0, NULL);
    return true;
}

static bool order_veh_amphibious_tile_notice(int action_faction,
                                             int action_mode,
                                             const Map &tile) {
    const int ability_id = abil_index(ABL_AMPHIBIOUS);
    if (action_faction != ConsoleGlobal.faction_id_ ||
        (NetDaemonNet.channels_[0].leading_words_[3] & 1) == 0) {
        return false;
    }
    parse_say(0, Ability[ability_id].name, -1, -1);

    const int owner = tile.val2 & 0x0F;
    const char *label = (tile.bit & BIT_BASE_IN_TILE) != 0 && owner < 8
        ? "AMPHIBBASE" : "NOTAMPHIB";
    NetMsgPrimary.pop(label, 5000, 0, NULL);
    return true;
}

static bool order_veh_amphibbase_notice(int mode, int selected_faction,
                                       const char *notice_label) {
    if (selected_faction != ConsoleGlobal.faction_id_ || (mode & 1) == 0) {
        return false;
    }

    const int ability = abil_index(ABL_AMPHIBIOUS);
    parse_say(0, Ability[ability].name, -1, -1);
    NetMsgPrimary.pop(notice_label, 5000, 0, NULL);
    return true;
}

enum OrderVeh595083ForkPath {
    OrderVeh595083Continue59533D,
    OrderVeh595083Continue5952C6,
    OrderVeh595083Continue59512C,
    OrderVeh595083CommonCleanup
};

enum OrderVeh595083TransportPath {
    OrderVeh595083TransportContinue59533D,
    OrderVeh595083TransportCommonCleanup
};

static void order_veh_596884_full_transport_notice(
    int vehicle_faction_snapshot, int action_mode, int visible_vehicle_id);
static void order_veh_5967cf_amphibbase2_or_full_transport(int, int, int, int, int);
static void order_veh_5967cf_amphibbase2_or_full_transport(
    int veh_id, int vehicle_faction_snapshot, int action_mode, int selected_base_id,
    int visible_vehicle_id);

static OrderVeh595083TransportPath order_veh_595083_amphibious_transport_fork(
    int veh_id, int vehicle_faction_snapshot, int action_mode,
    int selected_base_id, int visible_vehicle_id) {
    if (stack_check(visible_vehicle_id, 8,
                    vehicle_faction_snapshot, -1, -1) > 0) {
        return OrderVeh595083TransportContinue59533D;
    }

    if (selected_base_id < 0) {
        order_veh_596884_full_transport_notice(
            vehicle_faction_snapshot, action_mode, visible_vehicle_id);
        return OrderVeh595083TransportCommonCleanup;
    }

    if (has_abil(Vehs[veh_id].proto_id, 8) != 0 ||
        stack_check(visible_vehicle_id, 2, PLAN_NAVAL_TRANSPORT,
                    -1, -1) != 0 ||
        stack_check(veh_id, 2, PLAN_NAVAL_TRANSPORT, -1, -1) != 0) {
        return OrderVeh595083TransportContinue59533D;
    }

    order_veh_5967cf_amphibbase2_or_full_transport(
        veh_id, vehicle_faction_snapshot, action_mode, selected_base_id,
        visible_vehicle_id);
    return OrderVeh595083TransportCommonCleanup;
}

static void order_veh_5967cf_amphibbase2_or_full_transport(
    int veh_id, int vehicle_faction_snapshot, int action_mode, int selected_base_id,
    int visible_vehicle_id) {
    if (selected_base_id >= 0 &&
        has_abil(Vehs[veh_id].proto_id, 8) == 0 &&
        stack_check(visible_vehicle_id, 2, PLAN_NAVAL_TRANSPORT,
                    -1, -1) == 0 &&
        stack_check(veh_id, 2, PLAN_NAVAL_TRANSPORT, -1, -1) == 0) {
        if (vehicle_faction_snapshot == ConsoleGlobal.faction_id_ &&
            (NetDaemonNet.channels_[0].leading_words_[3] & 1) != 0) {
            const int ability = abil_index(8);
            parse_say(0, Ability[ability].name, -1, -1);
            NetMsgPrimary.pop("AMPHIBBASE2", 5000, 0, NULL);
        }
        return;
    }

    order_veh_596884_full_transport_notice(
        vehicle_faction_snapshot, action_mode, visible_vehicle_id);
}

static void order_veh_596884_full_transport_notice(
    int vehicle_faction_snapshot, int action_mode, int visible_vehicle_id) {
    if (stack_check(visible_vehicle_id, 2, PLAN_NAVAL_TRANSPORT,
                    -1, -1) == 0 ||
        vehicle_faction_snapshot != ConsoleGlobal.faction_id_ ||
        (NetDaemonNet.channels_[0].leading_words_[3] & 1) == 0) {
        return;
    }

    const int transport_count =
        stack_check(visible_vehicle_id, 2, PLAN_NAVAL_TRANSPORT, -1, -1);
    NetMsgPrimary.pop(transport_count > 1 ? "FULLTRANSPORTS" :
                                           "FULLTRANSPORT",
                      5000, 0, NULL);
}

static bool order_veh_noncombatant_notice(int action_faction, int action_mode) {
    if ((action_mode & 1) == 0) return false;
    if (action_faction != ConsoleGlobal.faction_id_) return false;
    g_FX.play(0x33);
    NetMsgPrimary.pop("NONCOMBATANT", 5000, 0, NULL);
    return true;
}

enum OrderVeh5958B8PromptPath {
    OrderVeh5958B8PromptCleanup,
    OrderVeh5958B8PromptContinue595A92,
    OrderVeh5958B8PromptContinueProbeSuccess
};

static OrderVeh5958B8PromptPath order_veh_5958b8_probe_prompt(
    int action_vehicle_id, int action_faction, int destination_controller,
    int destination_x, int destination_y, int action_mode) {
    const int destination_base_id = base_find(destination_x, destination_y);
    if (destination_base_id >= 0 &&
        Bases[destination_base_id].faction_id_current == action_faction) {
        parse_says(0,
                   Players[destination_controller].adj_name_faction,
                   -1, -1);
        parse_says(1,
                   VehPrototypes[Vehs[action_vehicle_id].proto_id].veh_name,
                   -1, -1);
        PluralityDefault = 0;
        GenderDefault = Players[action_faction].is_leader_female;
        parse_says(2, Players[action_faction].title_leader, -1, -1);
        const bool action_is_locally_enabled =
            ((1U << (action_faction & 0x1f)) & static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_)) != 0;
        if (!action_is_locally_enabled) {
            return OrderVeh5958B8PromptContinueProbeSuccess;
        }

        const bool is_alien =
            (Players[destination_controller].rule_flags & RFLAG_ALIEN) != 0;
        const char *const picture = is_alien
            ? "al_cap_sm.pcx" : "capture_sm.pcx";
        const char *const prompt_label = "ENEMYPROBE";
        const int accepted = popp(ScriptPopupFile, prompt_label, 0,
                                  picture, NULL);
        if (accepted == 0) return OrderVeh5958B8PromptCleanup;
        if (accepted != 1) {
            return OrderVeh5958B8PromptContinue595A92;
        }
        return OrderVeh5958B8PromptContinueProbeSuccess;
    }

    if (action_faction != ConsoleGlobal.faction_id_) {
        return OrderVeh5958B8PromptContinue595A92;
    }

    parse_says(0, Players[destination_controller].adj_name_faction, -1, -1);
    parse_says(1, VehPrototypes[Vehs[action_vehicle_id].proto_id].veh_name,
               -1, -1);
    PluralityDefault = 0;
    GenderDefault = Players[action_faction].is_leader_female;
    parse_says(2, Players[action_faction].title_leader, -1, -1);
    const bool is_alien =
        (Players[destination_controller].rule_flags & RFLAG_ALIEN) != 0;
    const char *const picture = is_alien
        ? "al_cap_sm.pcx" : "capture_sm.pcx";
    const int accepted = popp(ScriptPopupFile, "ENEMYPROBE2", 0,
                              picture, NULL);
    (void)destination_controller;
    (void)action_mode;
    return accepted == 0 ? OrderVeh5958B8PromptCleanup
                         : OrderVeh5958B8PromptContinue595A92;
}

static void order_veh_named_probe_notice(int adjective_faction,
                                         int leader_faction,
                                         const char *notice_label) {
    parse_says(0, Players[adjective_faction].adj_name_faction, -1, -1);
    PluralityDefault = 0;
    GenderDefault = Players[leader_faction].is_leader_female;
    parse_says(1, Players[leader_faction].title_leader, -1, -1);
    NetMsgPrimary.pop(notice_label, 5000, 0, NULL);
}

static bool order_veh_focus_stack_scoot(
    int action_faction_id, int destination_controller, int active_faction,
    int veh_id, int direction, int tile_x, int tile_y) {
    if (action_faction_id != active_faction &&
        destination_controller != active_faction) {
        return false;
    }

    ConsoleGlobal.focus(tile_x, tile_y, action_faction_id);
    (void)stack_veh(veh_id, 1);
    veh_scoot(veh_id, tile_x, tile_y, direction, 0);
    stack_put(veh_id, tile_x, tile_y);
    draw_tile(tile_x, tile_y, 2);
    return true;
}

static void order_veh_probe_base_resolution(
    int selected_vehicle_id, int selected_faction_id, int action_faction_id,
    int active_faction, int origin_x, int origin_y) {
    int selected_base_id = -1;
    int largest_population = -1;
    for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; ++base_id) {
        Base &base = Bases[base_id];
        if (base.faction_id_current == selected_faction_id &&
            base.population_size > largest_population) {
            largest_population = base.population_size;
            selected_base_id = base_id;
        }
    }

    if (selected_base_id >= 0) {
        Base &base = Bases[selected_base_id];
        stack_put(selected_vehicle_id, base.x, base.y);
        Vehs[selected_vehicle_id].visibility = 0;
        parse_says(2, base.name_string, -1, -1);
    }
    else if ((static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) &
              (1U << (selected_faction_id & 0x1f))) == 0) {
        veh_kill(selected_vehicle_id);
    }

    draw_tile(origin_x, origin_y, 2);
    if (selected_faction_id == ConsoleGlobal.faction_id_) {
        order_veh_named_probe_notice(action_faction_id, selected_faction_id,
                                     "GOTMYPROBE");
    } else if (action_faction_id == ConsoleGlobal.faction_id_) {
        order_veh_named_probe_notice(selected_faction_id, action_faction_id,
                                     "GOTYOURPROBE");
    }
}
static void order_veh_probe_success_corridor(
    int action_vehicle_id, int action_faction_id, int destination_controller,
    int active_faction, int direction, int destination_x, int destination_y,
    int selected_vehicle_id, int source_x, int source_y) {
    (void)order_veh_focus_stack_scoot(
        action_faction_id, destination_controller, active_faction,
        action_vehicle_id, direction, source_x, source_y);
    order_veh_probe_base_resolution(
        selected_vehicle_id, destination_controller, action_faction_id,
        active_faction, destination_x, destination_y);
}

enum OrderVeh595A92Path {
    OrderVeh595A92CommonCleanup,
    OrderVeh595A92Continue595BE6
};

static int order_veh_clamp_remaining_moves(int veh_id) {
    int remaining = speed(veh_id, 0) - Vehs[veh_id].moves_expended;
    if (remaining < 0) return 0;
    if (remaining > 999) return 999;
    return remaining;
}

static OrderVeh595A92Path order_veh_595a92_hasty_gate(
    int action_vehicle_id, int action_faction, int action_mode,
    bool native_state_4200_snapshot) {
    const Veh &vehicle = Vehs[action_vehicle_id];
    const int remaining_moves = order_veh_clamp_remaining_moves(
        action_vehicle_id);
    if (remaining_moves >= static_cast<int>(Rules.move_rate_roads)) {
        return OrderVeh595A92Continue595BE6;
    }

    const int prototype_id = vehicle.proto_id;
    if (prototype_id == 15 || prototype_id == 8) {
        return OrderVeh595A92Continue595BE6;
    }

    const bool show_hasty_prompt =
        action_faction == ConsoleGlobal.faction_id_ &&
        !native_state_4200_snapshot &&
        (NetDaemonNet.channels_[0].leading_words_[3] & 1) != 0;
    if (show_hasty_prompt) {
        parse_says(0, VehPrototypes[prototype_id].veh_name, -1, -1);
        const int second_remaining_moves =
            order_veh_clamp_remaining_moves(action_vehicle_id);
        parse_num(0, second_remaining_moves);
        parse_num(1, static_cast<int>(Rules.move_rate_roads));
        if (popp(ScriptPopupFile, "HASTY", 0, "bad_sm.pcx", NULL) != 0) {
            return OrderVeh595A92CommonCleanup;
        }
        return OrderVeh595A92Continue595BE6;
    }

    const uint8_t actor_mask = static_cast<uint8_t>(
        1U << (action_faction & 0x1f));
    if ((actor_mask & GameDataStateStorage.fields_.lock_masks_.enable_mask_) != 0) {
        return OrderVeh595A92Continue595BE6;
    }
    const int second_remaining_moves = order_veh_clamp_remaining_moves(
        action_vehicle_id);
    if (second_remaining_moves <=
        static_cast<int>(Rules.move_rate_roads) / 2) {
        return OrderVeh595A92CommonCleanup;
    }
    return OrderVeh595A92Continue595BE6;
}

enum OrderVehBattleResultPath {
    OrderVehBattleResultCleanupZero,
    OrderVehBattleResultContinue598CB9,
    OrderVehBattleResultCleanupOther
};

enum OrderVeh596052Path {
    OrderVeh596052CommonCleanup,
    OrderVeh596052Continue597345,
    OrderVeh596052Continue597350,
    OrderVeh596052Continue596163
};

enum OrderVeh597350Path {
    OrderVeh597350CommonCleanup,
    OrderVeh597350Continue597564
};

int break_treaty(int faction_id, int target_faction_id, int treaty);

enum OrderVeh597564GoodyPath {
    OrderVeh597564GoodyContinue,
    OrderVeh597564GoodyCommonCleanup
};

enum OrderVeh59763bMovementPath {
    OrderVeh59763bCommonCleanup,
    OrderVeh59763bContinue5976D6,
    OrderVeh59763bContinue597B24
};

extern uint32_t VehOrderDelayCounter;

enum OrderVeh5976d6Path {
    OrderVeh5976d6Continue597B24,
    OrderVeh5976d6CommonCleanup
};

static int order_veh_5976d6_clamp_remaining(int veh_id) {
    const int spent_before_speed = Vehs[veh_id].moves_expended;
    int remaining = speed(veh_id, 0) - spent_before_speed;
    if (remaining < 0) {
        remaining = 0;
    } else if (remaining > 999) {
        remaining = 999;
    }
    return remaining;
}

static void order_veh_5976d6_focus_stack_scoot(
    int veh_id, int source_x, int source_y, int destination_x,
    int destination_y, int direction, int action_faction) {
    ConsoleGlobal.focus(source_x, source_y, ConsoleGlobal.faction_id_);
    stack_veh(veh_id, 1);
    veh_scoot(veh_id, source_x, source_y, direction + 8, 0);
    stack_put(veh_id, source_x, source_y);

    for (int neighbor = 0; neighbor < 8; ++neighbor) {
        int x = destination_x + VehMoveDeltaX[neighbor];
        const int y = destination_y + VehMoveDeltaY[neighbor];
        if ((MapIsFlat & 1) == 0) {
            if (x < 0) {
                x += MapLongitudeBounds;
            } else if (x >= MapLongitudeBounds) {
                x -= MapLongitudeBounds;
            }
        }
        if (y >= 0 && y < MapLatitudeBounds && x >= 0 &&
            x < MapLongitudeBounds) {
            spot_loc(x, y, action_faction);
        }
    }
    draw_tiles(destination_x, destination_y, 2);
}

static OrderVeh5976d6Path order_veh_5976d6_terrain_delay(
    int veh_id, int action_faction, int direction,
    int source_x, int source_y, int destination_x, int destination_y,
    int visible_vehicle_id, int hex_cost, bool native_state_4200_snapshot,
    int &native_return_value) {
    Veh &vehicle = Vehs[veh_id];

    const int current_remaining = order_veh_5976d6_clamp_remaining(veh_id);
    if (current_remaining >= static_cast<int>(Rules.move_rate_roads)) {
        const Map &destination = *map_loc(destination_x, destination_y);
        if ((destination.bit & BIT_FUNGUS) == 0 ||
            (destination.climate & 0xE0U) < 0x40U) {
            return OrderVeh5976d6Continue597B24;
        }
        const VehPrototype &prototype = VehPrototypes[vehicle.proto_id];
        if (vehicle.proto_id < 64 &&
            Weapon[prototype.weapon_id].offense_rating < 0) {
            return OrderVeh5976d6Continue597B24;
        }
        if (prototype.plan == PLAN_TERRAFORMING ||
            prototype.plan == PLAN_ALIEN_ARTIFACT) {
            return OrderVeh5976d6Continue597B24;
        }
        if (visible_vehicle_id >= 0) return OrderVeh5976d6Continue597B24;
    }
    if ((NetDaemonNet.channels_[0].leading_words_[3] & 1U) == 0 &&
        (static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) &
         static_cast<uint8_t>(1U << (action_faction & 0x1f))) != 0) {
        return OrderVeh5976d6Continue597B24;
    }

    int chance = 0;
    if (ConsoleControlTurnActive != 0) {
        if (hex_cost > 1) chance = std::rand() % hex_cost;
    } else {
        chance = static_cast<int>(random(0, hex_cost));
    }
    const int spent_before_speed = vehicle.moves_expended;
    int remaining_after_chance = speed(veh_id, 0) - spent_before_speed;
    if (remaining_after_chance < 0) remaining_after_chance = 0;
    else if (remaining_after_chance > 999) remaining_after_chance = 999;
    const int chance_threshold = remaining_after_chance +
        ((vehicle.state & 0x8000U) != 0
             ? static_cast<int>(Rules.move_rate_roads)
             : 0);
    if (chance < chance_threshold) {
        return OrderVeh5976d6Continue597B24;
    }

    if (action_faction == ConsoleGlobal.faction_id_ && vehicle.order == ORDER_NONE &&
        (::GetAsyncKeyState(VK_SHIFT) & 0xff00) == 0 &&
        (NetDaemonNet.channels_[0].leading_words_[3] & 1U) != 0 &&
        (vehicle.state & 0x0200U) == 0 &&
        (GameDataStateStorage.fields_.game_more_preferences_ & 0x80U) == 0) {
        order_veh_5976d6_focus_stack_scoot(
            veh_id, source_x, source_y, destination_x, destination_y,
            direction, action_faction);
    }

    bool tutorial_map_was_shown = false;
    bool slow_fungus_tutorial_gate = false;
    if (action_faction == ConsoleGlobal.faction_id_) {
        const Map &tutorial_destination = *map_loc(destination_x, destination_y);
        slow_fungus_tutorial_gate =
            (tutorial_destination.bit & BIT_FUNGUS) != 0 &&
            (tutorial_destination.climate & 0xE0U) >= 0x40U &&
            vehicle.order == ORDER_NONE &&
            VehPrototypes[vehicle.proto_id].plan != PLAN_TERRAFORMING &&
            VehPrototypes[vehicle.proto_id].plan != PLAN_ALIEN_ARTIFACT &&
            (NetDaemonNet.channels_[0].leading_words_[3] & 1U) != 0;
    }
    if (slow_fungus_tutorial_gate) {
        if (tut_check(0x1000) != 0) {
            ++VehOrderDelayCounter;
            g_FX.play(9);
            TutorialWindow.tut_map("SLOWFUNGUS", destination_x,
                                   destination_y, -1, NULL, 0, -1, -1);
            tutorial_map_was_shown = true;
        } else if (!native_state_4200_snapshot) {
            ++VehOrderDelayCounter;
        }
    }
    if (!tutorial_map_was_shown &&
        action_faction == ConsoleGlobal.faction_id_ &&
        vehicle.order == ORDER_NONE && !TutorialWindow.tutorial_blocked() &&
        (NetDaemonNet.channels_[0].leading_words_[3] & 1U) != 0 &&
        !((map_loc(destination_x, destination_y)->bit & BIT_FUNGUS) != 0 &&
          (map_loc(destination_x, destination_y)->climate & 0xE0U) >= 0x40U) &&
        tut_check(0x4000) != 0) {
        TutorialWindow.tut_map("FRACTIONALMOVE", destination_x,
                               destination_y, veh_id, NULL, 0, source_x, source_y);
        tutorial_map_was_shown = true;
    }

    if (tutorial_map_was_shown) {
        vehicle.order = ORDER_NONE;
        return OrderVeh5976d6CommonCleanup;
    }
    veh_skip(veh_id);
    vehicle.state |= 0x8000U;
    if ((NetDaemonNet.channels_[0].leading_words_[3] & 1U) != 0 &&
        IsMultiplayerNet != 0 &&
        ConsoleControlTurnActive == 0) {
        synch_veh(veh_id);
        NetDaemonNet.await_synch();
    }
    native_return_value = 1;
    return OrderVeh5976d6CommonCleanup;
}

enum OrderVeh597b24EntryPath {
    OrderVeh597b24EntryCleanupOne,
    OrderVeh597b24EntryContinueCombat
};

static OrderVeh597b24EntryPath order_veh_597b24_entry(
    int veh_id, int vehicle_faction_snapshot, int active_faction,
    int destination_base_id, int destination_controller, int native_local_2c,
    int &native_return_value) {
    const uint32_t action_mode =
        NetDaemonNet.channels_[0].leading_words_[3];
    native_return_value = 1;
    if ((action_mode & 2) == 0) {
        return OrderVeh597b24EntryCleanupOne;
    }
    Veh &vehicle = Vehs[veh_id];
    vehicle.moves_expended = static_cast<uint8_t>(
        vehicle.moves_expended + static_cast<uint8_t>(native_local_2c));
    vehicle.state &= ~0x00008040U;

    const bool foreign_unpacted_base = destination_base_id >= 0 &&
        destination_controller != vehicle_faction_snapshot &&
        (PlayersData[vehicle_faction_snapshot]
             .diplo_treaties[destination_controller] & 1U) == 0;
    if (foreign_unpacted_base &&
        (PlayersData[destination_controller]
             .diplo_treaties[vehicle_faction_snapshot] & 7U) != 0 &&
        destination_controller == active_faction) {
        const Player &actor = Players[vehicle_faction_snapshot];
        GenderDefault = actor.is_leader_female;
        PluralityDefault = 0;
        parse_says(0, actor.title_leader, -1, -1);
        GenderDefault = actor.is_leader_female;
        PluralityDefault = 0;
        parse_says(1, actor.name_leader, -1, -1);
        GenderDefault = actor.noun_gender;
        PluralityDefault = actor.is_noun_plural;
        parse_says(2, actor.noun_faction, -1, -1);
        NetMsgPrimary.pop("SURPRISE", 5000, 0, NULL);
    }

    active_faction = ConsoleGlobal.faction_id_;
    if (foreign_unpacted_base &&
        vehicle_faction_snapshot != active_faction &&
        destination_controller != active_faction &&
        (Bases[destination_base_id].faction_id_current == active_faction ||
        (Bases[destination_base_id].visibility &
          static_cast<uint8_t>(1U << (active_faction & 0x1f))) != 0) &&
        vehicle_faction_snapshot != 0) {
        spot_stack(veh_id, active_faction);
    }
    return OrderVeh597b24EntryContinueCombat;
}

static bool order_veh_597d80_sensor_radius_scan(
    int destination_x, int destination_y, int active_faction) {
    const Map &destination = *map_loc(destination_x, destination_y);
    bool found_local_control = false;

    for (int radius_index = 0; radius_index < 0x54; radius_index += 4) {
        int x = destination_x + RadiusOffsetX[radius_index / 4];
        const int y = destination_y + RadiusOffsetY[radius_index / 4];
        if ((MapIsFlat & 1) == 0) {
            if (x < 0) {
                x += MapLongitudeBounds;
            } else if (x >= MapLongitudeBounds) {
                x -= MapLongitudeBounds;
            }
        }
        if (y < 0 || y >= MapLatitudeBounds || x < 0 ||
            x >= MapLongitudeBounds) {
            continue;
        }

        const Map &neighbor = *map_loc(x, y);
        const bool destination_is_lowland =
            (destination.climate & 0xE0U) < 0x60U;
        const bool neighbor_is_lowland =
            (neighbor.climate & 0xE0U) < 0x60U;
        if (destination_is_lowland != neighbor_is_lowland ||
            destination.region == neighbor.region) {
            if (is_sensor(x, y) != 0) {
                const int owner = whose_territory(ConsoleGlobal.faction_id_, x, y, NULL, 0);
                active_faction = ConsoleGlobal.faction_id_;
                if (owner == active_faction) found_local_control = true;
            }
        } else {
            continue;
        }

        active_faction = ConsoleGlobal.faction_id_;
        bool recognized_owner = false;
        if (radius_index < 0x24) {
            recognized_owner = (neighbor.bit & 3U) != 0 &&
                               (neighbor.val2 & 0x0FU) < 8;
        } else if ((neighbor.bit & 1U) != 0) {
            recognized_owner = (neighbor.val2 & 0x0FU) < 8;
        }
        if (recognized_owner &&
            static_cast<int>(neighbor.val2 & 0x0FU) == active_faction) {
            found_local_control = true;
        }
        if (found_local_control) break;
    }
    return found_local_control;
}

enum OrderVehPrefixResult {
    ORDER_VEH_RUN_RADIUS = 0,
    ORDER_VEH_ENTER_597F31 = 1,
    ORDER_VEH_SKIP_RADIUS_TO_59814A = 2
};

static OrderVehPrefixResult order_veh_prefix_result(
    int veh_id, int vehicle_faction_snapshot, int current_faction,
    int target_x, int target_y) {
    const Veh &vehicle = Vehs[veh_id];
    const uint32_t faction_mask = 1u << current_faction;
    int short_circuit = 0;

    if (vehicle_faction_snapshot == current_faction) {
        short_circuit = 1;
    } else if (vehicle.faction_id == current_faction
               && (vehicle.flags & 0x440u) != 0x440u) {
        short_circuit = 1;
    } else if ((vehicle.visibility & faction_mask) != 0) {
        short_circuit = 1;
    } else if ((GameDataStateStorage.fields_.game_state_ & 0x80u) != 0) {
        short_circuit = 1;
    }

    if ((GameDataStateStorage.fields_.game_state_ & 0x80u) == 0
        && (PlayersData[current_faction].flags & 0x200u) == 0) {
        const int map_index = target_y * MapLongitude + (target_x >> 1);
        if ((MapTiles[map_index].visibility & faction_mask) == 0) {
            return short_circuit != 0
                       ? ORDER_VEH_ENTER_597F31
                       : ORDER_VEH_SKIP_RADIUS_TO_59814A;
        }
    }

    return short_circuit != 0 ? ORDER_VEH_ENTER_597F31
                              : ORDER_VEH_RUN_RADIUS;
}

static bool order_veh_597cc8_to_597f31(
    int veh_id, int vehicle_faction_snapshot, int current_faction,
    int target_x, int target_y) {
    const OrderVehPrefixResult prefix = order_veh_prefix_result(
        veh_id, vehicle_faction_snapshot, current_faction,
        target_x, target_y);
    if (prefix == ORDER_VEH_ENTER_597F31) return true;
    if (prefix == ORDER_VEH_SKIP_RADIUS_TO_59814A) return false;
    return order_veh_597d80_sensor_radius_scan(
        target_x, target_y, current_faction);
}

static void order_veh_combat_and_turn_prelude(
    int veh_id, int vehicle_faction_snapshot,
    int original_action_mode,
    uint8_t vehicle_faction_mask,
    const Map &destination_tile,
    bool native_local_24_nonzero, bool native_local_30_nonzero) {
    Veh &vehicle = Vehs[veh_id];
    (void)original_action_mode;

    if (Weapon[VehPrototypes[vehicle.proto_id].weapon_id].offense_rating == 0) {
        veh_demote(veh_id);
    }

    if (vehicle_faction_snapshot == 0) {
        if ((destination_tile.bit & BIT_FUNGUS) != 0 &&
            (destination_tile.climate & 0xE0u) >= 0x40u
            && vehicle.dmg_incurred != 0) {
            --vehicle.dmg_incurred;
        }
    }

    spot_all(veh_id, 1);

    const bool local_faction =
        vehicle_faction_snapshot == ConsoleGlobal.faction_id_;
    const bool lock_disallows_sleep =
        (vehicle_faction_mask & static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_)) != 0;
    const bool omniscient = (GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW) != 0;
    if (!local_faction && !lock_disallows_sleep && !omniscient
        && native_local_24_nonzero && ConsoleGlobal.vehicle_order_ui_state() == 0) {
        const bool shift_down = (GetAsyncKeyState(VK_SHIFT) & 0xFF00) != 0;
        if (!shift_down && IsMultiplayerNet == 0) {
            int delay = 5;
            if (native_local_30_nonzero && ConsoleGlobal.vehicle_order_ui_state() == 0) {
                delay = 100;
            }
            clock_wait(delay);
        }
    }
}

static bool order_veh_air_departure_stack_needs_transport(
    int veh_id, const Map &tile) {
    const Veh &selected = Vehs[veh_id];
    const VehPrototype &prototype = VehPrototypes[selected.proto_id];
    if (Chassis[prototype.chassis_id].triad != TRIAD_AIR) return false;

    const int base_owner = tile.val2 & 0x0F;
    bool needs_transport = true;
    if ((tile.bit & BIT_BASE_IN_TILE) != 0 && base_owner >= 0 && base_owner < 8) {
        needs_transport = false;
    } else if ((tile.bit & BIT_AIRBASE) != 0) {
        needs_transport = false;
    }

    int stack_veh_id = veh_id;
    if (stack_veh_id >= 0) {
        while (Vehs[stack_veh_id].prev_veh_id_stack >= 0) {
            stack_veh_id = Vehs[stack_veh_id].prev_veh_id_stack;
        }
    }
    if (needs_transport) {
        while (stack_veh_id >= 0) {
            const Veh &member = Vehs[stack_veh_id];
            const bool carrier = has_abil(member.proto_id, ABL_CARRIER);
            if (carrier) {
                needs_transport = false;
            }
            stack_veh_id = member.next_veh_id_stack;
            if (!needs_transport) break;
        }
    }
    return needs_transport;
}

struct OrderVeh5980a8State {
    bool native_local_30;
    bool native_v42;
    bool native_local_24;
};

static OrderVeh5980a8State order_veh_597f31_to_5980a8(
    int veh_id, int vehicle_faction_snapshot, int current_faction,
    int destination_base_id, int destination_x, int destination_y,
    int source_x, int source_y, int chassis_triad, int native_hex_cost,
    bool native_local_24_nonzero) {
    const Veh &vehicle = Vehs[veh_id];
    bool native_v42 = true;

    if (vehicle_faction_snapshot != current_faction &&
        (((GameDataStateStorage.fields_.game_preferences_ & 0x04U) == 0) ||
         (PlayersData[current_faction]
              .diplo_treaties[vehicle_faction_snapshot] & 1U) != 0) &&
        (((GameDataStateStorage.fields_.game_preferences_ & 0x8000U) == 0) ||
         (PlayersData[current_faction]
              .diplo_treaties[vehicle_faction_snapshot] & 1U) == 0) &&
        (destination_base_id < 0 ||
         Bases[destination_base_id].faction_id_current ==
             vehicle_faction_snapshot ||
         (PlayersData[vehicle_faction_snapshot]
              .diplo_treaties[Bases[destination_base_id].faction_id_current]
          & 1U) != 0)) {
        native_v42 = false;
    }
    bool native_local_30 = native_v42;

    const bool shift_down =
        (GetAsyncKeyState(VK_SHIFT) & 0xFF00) != 0;
    current_faction = ConsoleGlobal.faction_id_;
    if (shift_down) {
        native_v42 = false;
        native_local_30 = false;
    }

    bool take_cursor_path = false;
    if (native_hex_cost == 0) {
        if (vehicle_faction_snapshot != current_faction ||
            (vehicle.state & 0x00004200U) != 0 || vehicle.order == 0x18) {
            take_cursor_path = true;
        }
    }

    if (take_cursor_path || shift_down || (GameDataStateStorage.fields_.game_more_preferences_ & 0x80U) != 0) {
        native_v42 = false;
        native_local_30 = false;
        ConsoleGlobal.vehicle_order_ui_state() = 1;
        if (vehicle_faction_snapshot == current_faction) {
            ConsoleGlobal.vehicle_order_ui_state() = 2;
            ConsoleGlobal.cursor_next(destination_x, destination_y);
        }
    } else if ((GameDataStateStorage.fields_.game_more_preferences_ & 0x40U) != 0) {
        if (vehicle_faction_snapshot == current_faction) {
            if ((vehicle.state & 0x00004200U) != 0 ||
                vehicle.order == 0x18) {
                native_v42 = false;
                native_local_30 = false;
                ConsoleGlobal.vehicle_order_ui_state() = 1;
                ConsoleGlobal.vehicle_order_ui_state() = 2;
                ConsoleGlobal.cursor_next(destination_x, destination_y);
            }
        } else if (chassis_triad == TRIAD_AIR) {
            native_v42 = false;
            native_local_30 = false;
            ConsoleGlobal.vehicle_order_ui_state() = 1;
        }
    }

    if (native_local_24_nonzero &&
        (GetAsyncKeyState(VK_CONTROL) & 0xFF00) == 0 &&
        (native_local_30 || (GameDataStateStorage.fields_.game_preferences_ & 0x80000U) == 0)) {
        ConsoleGlobal.focus(source_x, source_y, vehicle_faction_snapshot);
    }

    stack_veh(veh_id, 1);
    { OrderVeh5980a8State result = {
        native_local_30, native_v42, native_local_24_nonzero}; return result; }
}

static void order_veh_5980a8_restack_and_visibility(
    int veh_id, int vehicle_faction_snapshot,
    int direction, int destination_x, int destination_y,
    int source_x, int source_y, const Map &source_tile,
    const Map &destination_tile, bool native_v42, bool native_local_24) {
    (void)destination_x;
    (void)destination_y;
    if (native_v42) {
        if (vehicle_faction_snapshot == ConsoleGlobal.faction_id_) {
            g_FX.engine_pitch(
                ((destination_tile.climate & 0xE0U) <=
                         (source_tile.climate & 0xE0U)
                     ? 0
                     : 100) + 200);
        }
        veh_scoot(veh_id, source_x, source_y, direction, 0);
        if (vehicle_faction_snapshot == ConsoleGlobal.faction_id_) {
            g_FX.engine_pitch(0);
        }
    } else if (native_local_24) {
        veh_scoot(veh_id, source_x, source_y, direction, 0x4000);
    } else {
        Vehs[veh_id].facing = static_cast<uint8_t>(direction);
    }

    if (vehicle_faction_snapshot != 0) {
        unspot_stack(veh_id);
    } else {
        Veh &vehicle = Vehs[veh_id];
        vehicle.visibility &= destination_tile.visibility;
        for (int faction = 1; faction < MaxPlayerNum; ++faction) {
            const uint8_t faction_bit = static_cast<uint8_t>(1U << faction);
            if ((vehicle.visibility & faction_bit) == 0) continue;
            (void)base_find(destination_x, destination_y, faction);
            if (BaseFindDist >= 8 &&
                whose_territory(faction, destination_x, destination_y,
                                NULL, 0) != faction) {
                vehicle.visibility &= static_cast<uint8_t>(~faction_bit);
            }
        }
    }
}

static void order_veh_598240_no_base_bunker_response(
    int veh_id, int action_faction, int destination_x, int destination_y,
    int destination_base_id, const Map &destination_tile) {
    const Veh &vehicle = Vehs[veh_id];
    const VehPrototype &prototype = VehPrototypes[vehicle.proto_id];
    const RulesChassis &chassis = Chassis[prototype.chassis_id];
    if ((destination_tile.bit & BIT_BUNKER) == 0 ||
        destination_base_id >= 0 || chassis.triad != TRIAD_LAND ||
        prototype.plan == PLAN_INFO_WARFARE) {
        return;
    }

    const int territory_faction = whose_territory(
        action_faction, destination_x, destination_y, NULL, 0);
    if (territory_faction < 0 || territory_faction == action_faction ||
        (PlayersData[action_faction]
             .diplo_treaties[territory_faction] & 1U) != 0) {
        return;
    }

    veh_skip(veh_id);
    if (action_faction == ConsoleGlobal.faction_id_) {
        NetMsgPrimary.pop("BUNKERCAPTURED", 0x1388, 0, NULL);
    }
}

static bool order_veh_5986ec_stack_put_and_capture(
    int &veh_id, int vehicle_faction_snapshot, int destination_x,
    int destination_y, int destination_base_id, int destination_base_faction) {
    stack_put(veh_id, destination_x, destination_y);
    if (destination_base_id < 0 ||
        vehicle_faction_snapshot == destination_base_faction ||
        (PlayersData[vehicle_faction_snapshot]
             .diplo_treaties[destination_base_faction] & 1U) != 0) {
        return false;
    }

    Veh &vehicle = Vehs[veh_id];
    if (ExpansionEnabled != 0 && vehicle.proto_id != 0x10 &&
        vehicle.proto_id != 0x11 && vehicle.proto_id != 0x12) {
        vehicle.dmg_incurred = 0;
    }

    GameDataStateStorage.fields_.vehicle_selection_follow_limit_ = veh_id;
    (void)act_of_aggression(vehicle_faction_snapshot,
                            destination_base_faction);
    capture_base(destination_base_id, vehicle_faction_snapshot, 0);
    veh_id = GameDataStateStorage.fields_.vehicle_selection_follow_limit_;
    return true;
}

struct OrderVehEventFacilityResult {
    int facility_id;
    int attempts;
    bool facility_valid;
    bool native_local_68;
    bool native_local_30;
};

static OrderVehEventFacilityResult order_veh_598338_pick_event_facility(
    int base_id, int destination_x, int destination_y) {
    int candidate_facility = 0;
    int attempts = 0;
    bool facility_valid = true;
    bool native_local_68 = false;
    bool native_local_30 = true;
    do {
        candidate_facility = rand() % 0x46;
        facility_valid = true;
        native_local_30 = true;
        for (int bonus = 0; bonus < Players[0].faction_bonus_count; ++bonus) {
            const int bonus_kind = Players[0].faction_bonus_id[bonus];
            if (bonus_kind != 2 &&
                (bonus_kind != 0x0C ||
                 !has_tech(Facility[Players[0].faction_bonus_val1[bonus]].preq_tech, 0))) {
                continue;
            }
            if (candidate_facility ==
                Players[0].faction_bonus_val1[bonus]) {
                facility_valid = false;
                native_local_30 = false;
                break;
            }
        }

        if (candidate_facility == 0x1A) {
            const Map *tile = map_loc(destination_x, destination_y);
            if ((tile->climate & 0xE0U) < 0x60U) {
                facility_valid = false;
                native_local_30 = false;
            }
        } else if (candidate_facility <= 1 || candidate_facility >= 0x41) {
            ++attempts;
            continue;
        }

        int byte_offset;
        int bit_mask;
        bitmask(candidate_facility, &byte_offset, &bit_mask);
        const uint8_t native_byte = Bases[base_id].facilities_built[byte_offset];
        native_local_68 =
            (native_byte & static_cast<uint8_t>(bit_mask)) != 0;
        if (native_local_68 && facility_valid) break;
        ++attempts;
    } while (attempts <= 100);
    { OrderVehEventFacilityResult result = {candidate_facility, attempts, facility_valid, native_local_68,
            native_local_30}; return result; }
}

struct OrderVehNativeLifeEventResult {
    bool player_eliminated;
    bool native_local_68;
    int native_local_30;
};

static OrderVehNativeLifeEventResult order_veh_598317_native_life_base_event(
    int &veh_id, int base_id, int destination_faction,
    int vehicle_faction_snapshot, int target_x, int target_y,
    bool native_message_flag, int native_local_30_before_event) {
    if (base_id < 0 || vehicle_faction_snapshot == destination_faction ||
        vehicle_faction_snapshot != 0) {
        { OrderVehNativeLifeEventResult result = {false, false, native_local_30_before_event}; return result; }
    }

    const OrderVehEventFacilityResult event =
        order_veh_598338_pick_event_facility(base_id, target_x, target_y);
    StringTemp[0] = 0;
    strcpy(StringTemp, "WORMSATTACK");
    parse_says(2, VehPrototypes[Vehs[veh_id].proto_id].veh_name, -1, -1);
    if (event.attempts <= 100) {
        StringTemp[0] = 0;
        strcpy(StringTemp, "1");
        set_fac(event.facility_id, base_id, 0);
        parse_say(1, Facility[event.facility_id].name, -1, -1);
    }
    if (native_message_flag &&
        destination_faction == ConsoleGlobal.faction_id_) {
        parse_says(0, Bases[base_id].name_string, -1, -1);
        NetMsgPrimary.pop(StringTemp, 5000, 0, NULL);
    }

    kill(veh_id);
    int native_life_units_seen = 0;
    for (int radius_index = 0; radius_index < 100; radius_index += 4) {
        int x = target_x + RadiusOffsetX[radius_index / 4];
        if (!MapIsFlat) {
            if (x < 0) {
                x += MapLongitudeBounds;
            } else if (x >= MapLongitudeBounds) {
                x -= MapLongitudeBounds;
            }
        }
        const int y = target_y + RadiusOffsetY[radius_index / 4];
        if (y < 0 || y >= MapLatitudeBounds ||
            x < 0 || x >= MapLongitudeBounds) {
            continue;
        }

        int stack_id = veh_id = stack_fix(veh_at(x, y));
        if (stack_id < 0) continue;
        if (Vehs[stack_id].faction_id != 0) continue;
        while (Vehs[stack_id].prev_veh_id_stack >= 0) {
            stack_id = Vehs[stack_id].prev_veh_id_stack;
        }
        veh_id = stack_id;
        while (stack_id >= 0) {
            const int next_stack_id = Vehs[stack_id].next_veh_id_stack;
            if (PlayersData[destination_faction].planet_ecology <= native_life_units_seen) {
                veh_kill(stack_id);
            } else {
                ++native_life_units_seen;
            }
            stack_id = veh_id = next_stack_id;
        }
    }

    if (GameDataStateStorage.fields_.turn_current_num_ >= 10) {
        Base &base = Bases[base_id];
        base.population_size = static_cast<int8_t>(
            static_cast<uint8_t>(base.population_size) - 1U);
        if (is_objective(base_id) && base.population_size <= 0) {
            base.population_size = 1;
        }
        if (base.population_size == 0) {
            base_kill(base_id);
            draw_map(1);
            (void)eliminate_player(destination_faction, 0);
            draw_map(1);
            { OrderVehNativeLifeEventResult result = {true, event.native_local_68, 100}; return result; }
        }
    }
    if (event.attempts > 100) {
        Bases[base_id].minerals_accumulated = 0;
    }
    draw_tile(Bases[base_id].x, Bases[base_id].y, 2);
    TutWinBaseWindow.check_base(base_id);
    { OrderVehNativeLifeEventResult result = {false, event.native_local_68, 100}; return result; }
}

static int order_veh_598793_no_base_sea_artillery_scan(
    int &attacker_id, int vehicle_faction_snapshot, int source_x, int source_y,
    bool native_sea_scan_enabled) {
    if (!native_sea_scan_enabled) return 0;

    Veh &attacker = Vehs[attacker_id];
    if (Chassis[VehPrototypes[attacker.proto_id].chassis_id].triad !=
        TRIAD_SEA) {
        return 0;
    }
    int spotted_count = 0;
    int last_spotted_faction = -1;
    for (int radius_index = 0; radius_index < 100; radius_index += 4) {
        int x = source_x + RadiusOffsetX[radius_index / 4];
        if (!MapIsFlat) {
            if (x < 0) {
                x += MapLongitudeBounds;
            } else if (x >= MapLongitudeBounds) {
                x -= MapLongitudeBounds;
            }
        }
        const int y = source_y + RadiusOffsetY[radius_index / 4];
        if (y < 0 || y >= MapLatitudeBounds ||
            x < 0 || x >= MapLongitudeBounds) {
            continue;
        }

        int stack_id = stack_fix(veh_at(x, y));
        while (stack_id >= 0) {
            const Veh &candidate = Vehs[stack_id];
            const int next_stack_id = candidate.next_veh_id_stack;
            const int faction = candidate.faction_id;
            const uint8_t faction_bit = static_cast<uint8_t>(1U << faction);
            if (faction != vehicle_faction_snapshot &&
                can_arty(candidate.proto_id, 1) &&
                Chassis[VehPrototypes[candidate.proto_id].chassis_id].triad ==
                    TRIAD_LAND &&
                (PlayersData[vehicle_faction_snapshot]
                     .diplo_treaties[faction] & 0x10U) != 0 &&
                ((attacker.faction_id == faction &&
                  (attacker.flags & 0x0440U) != 0x0440U) ||
                 (attacker.visibility & faction_bit) != 0)) {
                last_spotted_faction = faction;
                ++spotted_count;
                spot_stack(stack_id, vehicle_faction_snapshot);
            }
            stack_id = next_stack_id;
        }
    }

    if (spotted_count != 0) {
        attacker.moves_expended = static_cast<uint8_t>(
            attacker.moves_expended + static_cast<int8_t>(spotted_count) *
                                          static_cast<int>(Rules.move_rate_roads));
        if (vehicle_faction_snapshot == ConsoleGlobal.faction_id_) {
            parse_says(0,
                Players[last_spotted_faction].adj_name_faction, -1, -1);
            NetMsgPrimary.pop("ARTILLERY", 5000, 0, NULL);
        }
    }
    return spotted_count;
}

static void order_veh_598993_clear_carried_sentry_orders(
    int veh_id, int vehicle_faction_snapshot, int destination_base_id,
    int destination_x, int destination_y,
    uint8_t faction_mask) {
    const int native_cargo_result = veh_cargo(veh_id);
    const bool native_controller_gate_allows_clear =
        vehicle_faction_snapshot == ConsoleGlobal.faction_id_ ||
        (IsMultiplayerNet && (faction_mask & static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_)) != 0);
    if (!native_cargo_result || destination_base_id < 0 ||
        !native_controller_gate_allows_clear || veh_id < 0) {
        return;
    }

    const Veh &mover = Vehs[veh_id];
    if (mover.order == 0x18 &&
        (mover.waypoint_x[0] != destination_x ||
         mover.waypoint_y[0] != destination_y)) {
        return;
    }

    int stack_id = veh_id;
    while (Vehs[stack_id].prev_veh_id_stack >= 0) {
        stack_id = Vehs[stack_id].prev_veh_id_stack;
    }
    while (stack_id >= 0) {
        Veh &stack_vehicle = Vehs[stack_id];
        const int next_stack_id = stack_vehicle.next_veh_id_stack;
        if (stack_vehicle.faction_id == vehicle_faction_snapshot &&
            stack_vehicle.order == ORDER_SENTRY_BOARD &&
            (stack_vehicle.waypoint_x[0] < 0 ||
             stack_vehicle.waypoint_x[0] == veh_id)) {
            stack_vehicle.order = ORDER_NONE;
        }
        stack_id = next_stack_id;
    }
}

static bool order_veh_598a84_sea_cargo_recovery(
    int veh_id, int vehicle_faction_snapshot, int target_x, int target_y,
    bool native_high_bit_enabled, bool destination_is_lowland) {
    if (vehicle_faction_snapshot != ConsoleGlobal.faction_id_ ||
        !native_high_bit_enabled) return false;
    Veh &mover = Vehs[veh_id];
    if (Chassis[VehPrototypes[mover.proto_id].chassis_id].triad != TRIAD_SEA ||
        veh_cargo(veh_id) == 0 || !destination_is_lowland) return false;
    const int mover_speed = speed(veh_id, 0);
    const int mover_spent = mover.moves_expended;
    const int mover_remaining = mover_speed - mover_spent;
    if (!(mover_remaining < 0 || (mover_remaining <= 999 && mover_remaining == 0)))
        return false;
    if (mover.order == ORDER_MOVE_TO &&
        (mover.waypoint_x[0] != target_x || mover.waypoint_y[0] != target_y))
        return false;

    const bool flat = (MapIsFlat & 1) != 0;
    bool adjacent_landing_tile = false;
    for (int direction = 0; direction < 8; ++direction) {
        int x = target_x + VehMoveDeltaX[direction];
        const int y = target_y + VehMoveDeltaY[direction];
        if (!flat) {
            if (x < 0) x += MapLongitudeBounds;
            else if (x >= MapLongitudeBounds) x -= MapLongitudeBounds;
        }
        if (y < 0 || y >= MapLatitudeBounds || x < 0 || x >= MapLongitudeBounds)
            continue;
        const Map &tile = *map_loc(x, y);
        if ((tile.climate & 0xe0U) >= 0x60U ||
            ((tile.bit & BIT_BASE_IN_TILE) != 0 && (tile.val2 & 15) < 8)) {
            adjacent_landing_tile = true;
            break;
        }
    }
    if (!adjacent_landing_tile) return false;

    int member = veh_id;
    if (member < 0) return false;
    while (Vehs[member].prev_veh_id_stack >= 0)
        member = Vehs[member].prev_veh_id_stack;
    bool cleared = false;
    while (member >= 0) {
        Veh &passenger = Vehs[member];
        if (Chassis[VehPrototypes[passenger.proto_id].chassis_id].triad == TRIAD_LAND &&
            passenger.order == ORDER_SENTRY_BOARD) {
            const int passenger_speed = speed(member, 0);
            const int passenger_remaining = passenger_speed - passenger.moves_expended;
            if (passenger_remaining >= 0 &&
                (passenger_remaining > 999 || passenger_remaining != 0)) {
                passenger.order = ORDER_NONE;
                ConsoleGlobal.selected_vehicle_ = member;
                cleared = true;
            }
        }
        member = passenger.next_veh_id_stack;
    }
    return cleared;
}

enum OrderVeh598E84Continuation {
    OrderVeh598E84ContinueTo598F9E,
    OrderVeh598E84ContinueTo599151
};

struct OrderVeh598240TailResult {
    bool player_eliminated;
    bool needs_air_transport;
    OrderVeh598E84Continuation after_range_gate;
    bool exhausted_at_59903f;
    bool vehicle_killed;
    bool continue_to_599154;
    bool native_local_68;
};

static OrderVeh598E84Continuation order_veh_598e84_range_speed_gate(
    int veh_id, int vehicle_faction_snapshot, uint8_t vehicle_faction_mask,
    bool needs_transport, bool native_battle_continuation,
    int destination_x, int destination_y);
static bool order_veh_598f9e_action_home_gate(
    int veh_id, int vehicle_faction_snapshot,
    uint8_t vehicle_faction_mask);
static bool order_veh_59903f_speed_exhausted(int veh_id);
static bool order_veh_599061_fatigue_and_damage(
    int veh_id, int vehicle_faction_snapshot,
    uint8_t vehicle_faction_mask, bool native_local_24_nonzero);

static OrderVeh598E84Continuation order_veh_598e84_range_speed_gate(
    int veh_id, int vehicle_faction_snapshot, uint8_t vehicle_faction_mask,
    bool needs_transport, bool native_battle_continuation,
    int destination_x, int destination_y) {
    Veh &vehicle = Vehs[veh_id];
    const RulesChassis &chassis =
        Chassis[VehPrototypes[vehicle.proto_id].chassis_id];

    if ((vehicle_faction_mask & static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_)) == 0) {
        if (needs_transport) return OrderVeh598E84ContinueTo598F9E;
        vehicle.terraforming_turns = 0;
        veh_skip(veh_id);
        veh_demote(veh_id);
        draw_tile(vehicle.x, vehicle.y, 2);
        return OrderVeh598E84ContinueTo599151;
    }
    if (needs_transport) return OrderVeh598E84ContinueTo598F9E;

    if (chassis.range == 1) {
        const int initial_speed = speed(veh_id, 0);
        const int remaining = initial_speed - vehicle.moves_expended;
        if (remaining >= 0 &&
            (remaining > 999 || initial_speed != vehicle.moves_expended) &&
            native_battle_continuation) {
            return OrderVeh598E84ContinueTo598F9E;
        }
    }

    const int checked_speed = speed(veh_id, 0);
    int remaining = checked_speed - vehicle.moves_expended;
    if (remaining < 0 || (remaining <= 999 &&
                          checked_speed == vehicle.moves_expended)) {
        vehicle.terraforming_turns = 0;
        veh_skip(veh_id);
        veh_demote(veh_id);
        draw_tile(vehicle.x, vehicle.y, 2);
        return OrderVeh598E84ContinueTo599151;
    }

    remaining = speed(veh_id, 0) - vehicle.moves_expended;
    if (remaining < 0) {
        remaining = 0;
    } else if (remaining > 999) {
        remaining = 999;
    }
    if (remaining <= static_cast<int>(Rules.move_rate_roads) ||
        Chassis[VehPrototypes[vehicle.proto_id].chassis_id].range <=
            static_cast<int>(vehicle.terraforming_turns) + 1) {
        if (zoc_veh(destination_x, destination_y,
                    vehicle_faction_snapshot) == 0) {
            vehicle.terraforming_turns = 0;
            veh_skip(veh_id);
            veh_demote(veh_id);
            draw_tile(vehicle.x, vehicle.y, 2);
            return OrderVeh598E84ContinueTo599151;
        }
    }
    return OrderVeh598E84ContinueTo598F9E;
}

static bool order_veh_598f9e_action_home_gate(
    int veh_id, int vehicle_faction_snapshot,
    uint8_t vehicle_faction_mask) {
    Veh &vehicle = Vehs[veh_id];
    const VehPrototype &prototype = VehPrototypes[vehicle.proto_id];
    const RulesChassis &chassis = Chassis[prototype.chassis_id];
    if (chassis.range == 0) return false;

    const uint32_t state = vehicle.state;
    const bool action_home_allowed =
        (vehicle_faction_mask & static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_)) == 0 ||
        (vehicle_faction_snapshot == ConsoleGlobal.faction_id_ &&
         (((GameDataStateStorage.fields_.game_preferences_ & 0x00010000U) != 0 ||
           (state & 0x00004200U) != 0) &&
          (state & 0x01000200U) != 0x01000200U &&
          ((state & 0x02000200U) != 0x02000200U ||
           vehicle.waypoint_count <= vehicle.patrol_current_point)));
    if (action_home_allowed) action_home(veh_id, 1);
    return true;
}

static bool order_veh_59903f_speed_exhausted(int veh_id) {
    const int current_speed = speed(veh_id, 0);
    const int spent = Vehs[veh_id].moves_expended;
    const int remaining = current_speed - spent;
    return remaining < 0 ||
           (remaining <= 999 && current_speed == spent);
}

static bool order_veh_599061_fatigue_and_damage(
    int veh_id, int vehicle_faction_snapshot,
    uint8_t vehicle_faction_mask, bool native_local_24_nonzero) {
    Veh &vehicle = Vehs[veh_id];
    const VehPrototype &prototype = VehPrototypes[vehicle.proto_id];
    const RulesChassis &chassis = Chassis[prototype.chassis_id];
    const int next_morale = static_cast<int>(vehicle.terraforming_turns) + 1;

    if (static_cast<int>(chassis.range) <= next_morale) {
        vehicle.state |= 0x00000004U;
        if (native_local_24_nonzero) {
            boom(vehicle.x, vehicle.y, 0x80);
        }
        const VehPrototype &damage_prototype = VehPrototypes[vehicle.proto_id];
        const RulesChassis &damage_chassis = Chassis[damage_prototype.chassis_id];
        int damage_limit;
        if (damage_chassis.range == 1 && damage_prototype.chassis_id == 6) {
            const uint8_t damage_step = static_cast<uint8_t>(
                static_cast<unsigned int>(damage_prototype.reactor_id) * 3U);
            vehicle.dmg_incurred = static_cast<uint8_t>(
                vehicle.dmg_incurred + damage_step);
            damage_limit = damage_prototype.plan == PLAN_ALIEN_ARTIFACT ? 1 :
                (damage_prototype.reactor_id < 1 ? 10 :
                 (damage_prototype.reactor_id > 100 ? 1000 :
                  static_cast<int>(damage_prototype.reactor_id) * 10));
        } else {
            if (damage_prototype.plan == PLAN_ALIEN_ARTIFACT) {
                damage_limit = 1;
            } else {
                int weapon_step = damage_prototype.reactor_id;
                if (weapon_step < 1) weapon_step = 1;
                if (weapon_step > 100) weapon_step = 100;
                damage_limit = weapon_step * 10;
            }
            int damage_delta = damage_limit - vehicle.dmg_incurred;
            if (damage_delta < 0) {
                damage_delta = 0;
            } else if (damage_delta > 9999) {
                damage_delta = 9999;
            }
            vehicle.dmg_incurred = static_cast<uint8_t>(
                vehicle.dmg_incurred + static_cast<uint8_t>(damage_delta));
        }

        int remaining_damage = damage_limit - vehicle.dmg_incurred;
        if (remaining_damage < 0 ||
            (remaining_damage <= 9999 && remaining_damage <= 0)) {
            kill(veh_id);
            if (native_local_24_nonzero &&
                vehicle_faction_snapshot == ConsoleGlobal.faction_id_) {
                NetMsgPrimary.pop("PLANECRASH", 0x1388, 0, NULL);
            }
            return true;
        }

        vehicle.terraforming_turns = 0;
        draw_tile(vehicle.x, vehicle.y, 2);
        if (native_local_24_nonzero &&
            vehicle_faction_snapshot == ConsoleGlobal.faction_id_) {
            NetMsgPrimary.pop("CHOPPERFUEL", 0x1388, 0, NULL);
        }
        return false;
    }

    if (static_cast<int>(chassis.range) / 2 <= next_morale &&
        ((vehicle_faction_mask & static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_)) == 0 ||
        (vehicle_faction_snapshot == ConsoleGlobal.faction_id_ &&
        (((GameDataStateStorage.fields_.game_preferences_ & 0x00010000U) != 0 ||
          (vehicle.state & 0x00004200U) != 0) &&
         (vehicle.state & 0x01000200U) != 0x01000200U &&
         ((vehicle.state & 0x02000200U) != 0x02000200U ||
          vehicle.waypoint_count <= vehicle.patrol_current_point)) &&
        vehicle.order != 0x18))) {
        action_home(veh_id, 0);
    }
    ++vehicle.terraforming_turns;
    return false;
}

static bool order_veh_monolith_goody_corridor(
    int veh_id, int direction, int action_mode, const Map &destination_tile,
    int vehicle_triad, bool native_local_6c_nonzero, int goody_type) {
    Veh &vehicle = Vehs[veh_id];
    if ((destination_tile.bit & BIT_MONOLITH) != 0 && vehicle_triad != TRIAD_AIR) {
        bool monolith_allowed = true;
        if (direction < 0) {
            const int offense =
                Weapon[VehPrototypes[vehicle.proto_id].weapon_id].offense_rating;
            if (offense == 0 || (vehicle.dmg_incurred == 0 &&
                (vehicle.state & 0x00000100u) != 0)) {
                monolith_allowed = false;
            }
        }
        if (monolith_allowed && !native_local_6c_nonzero) {
            monolith(veh_id);
        }
    }

    if (goody_type == 0 || vehicle_triad == TRIAD_AIR || action_mode == 0) {
        return false;
    }
    return goody_box(veh_id, goody_type) != 0;
}

enum OrderVehHighlandStackClaim {
    OrderVehNoHighlandStackClaim,
    OrderVehHighlandStackClaimed
};

static OrderVehHighlandStackClaim order_veh_highland_stack_claim(
    int veh_id, int current_stack, int selected_base_id,
    int original_direction, int destination_x, int destination_y,
    int action_mode, int &selected_stack_id) {
    const Veh &vehicle = Vehs[veh_id];
    if (VehPrototypes[vehicle.proto_id].plan != PLAN_ALIEN_ARTIFACT
        || selected_base_id >= 0
        || stack_check(current_stack, 1, -1, -1, -1) != 1
        || original_direction >= 8) {
        return OrderVehNoHighlandStackClaim;
    }

    int selected_stack = -1;
    int selected_faction = -1;

    for (int direction = 0; direction < 8; ++direction) {
        const int x = wrap_map_x(destination_x + VehMoveDeltaX[direction]);
        const int y = destination_y + VehMoveDeltaY[direction];
        if (!on_map(x, y)) continue;
        if ((map_loc(x, y)->climate & 0xE0) < 0x60) continue;

        const int stack = stack_fix(veh_at(x, y));
        if (stack < 0) continue;
        const int faction = Vehs[stack].faction_id;
        if (faction == action_mode || faction == 0) continue;
        if ((PlayersData[action_mode].diplo_treaties[faction] & 1U) != 0) {
            continue;
        }

        const int artifact_units = stack_check(stack, 2,
                                               PLAN_ALIEN_ARTIFACT,
                                               -1, -1);
        const int total_units = stack_check(stack, 1, -1, -1, -1);
        if (total_units > artifact_units) {
            selected_stack = stack;
            selected_faction = faction;
            break;
        }
    }

    if (selected_stack < 0) return OrderVehNoHighlandStackClaim;
    selected_stack_id = selected_stack;

    Vehs[veh_id].faction_id = static_cast<uint8_t>(selected_faction);
    Vehs[veh_id].order = ORDER_NONE;
    draw_tile(destination_x, destination_y, 2);
    owner_set(destination_x, destination_y, Vehs[veh_id].faction_id);
    return OrderVehHighlandStackClaimed;
}

static void order_veh_highland_artifact_notice(
    int veh_id, int selected_faction, int action_mode) {
    if (action_mode == ConsoleGlobal.faction_id_) {
        ConsoleGlobal.selected_vehicle_ = -1;
        GenderDefault = Players[selected_faction].noun_gender;
        PluralityDefault = Players[selected_faction].is_noun_plural;
        parse_says(0, Players[selected_faction].noun_faction, -1, -1);
        popp(ScriptPopupFile, "TOOKARTIFACT", 0,
             "artstolen_sm.pcx", NULL);
        return;
    }

    if (selected_faction != ConsoleGlobal.faction_id_) return;

    const int prototype_id = Vehs[veh_id].proto_id;
    parse_says(0, VehPrototypes[prototype_id].veh_name, -1, -1);
    GenderDefault = Players[action_mode].noun_gender;
    PluralityDefault = Players[action_mode].is_noun_plural;
    parse_says(1, Players[action_mode].noun_faction, -1, -1);
    popp(ScriptPopupFile, "TAKENARTIFACT", 0,
         "artstolen_sm.pcx", NULL);
}

static void order_veh_599732_mark_region_plan_flag(
    int veh_id, int vehicle_faction_snapshot, uint8_t native_target_region,
    bool source_is_lowland, bool destination_is_lowland) {
    if (!source_is_lowland || destination_is_lowland ||
        VehPrototypes[Vehs[veh_id].proto_id].plan != 8) {
        return;
    }

    PlayersData[vehicle_faction_snapshot].unk_77[native_target_region] |= 2U;
}

static void order_veh_move_to_terrain_goal(
    int veh_id, int action_mode, int destination_x, int destination_y) {
    Veh &vehicle = Vehs[veh_id];
    int goal_x = destination_x;
    int goal_y = destination_y;
    if (vehicle.order == ORDER_MOVE_TO) {
        goal_x = (destination_x + vehicle.waypoint_x[0]) / 2;
        goal_y = (destination_y + vehicle.waypoint_y[0]) / 2;
        if ((goal_x & 1) != 0) --goal_x;
        if ((goal_y & 1) != 0) ++goal_x;
        const Map &candidate = MapTiles[MapLongitude * goal_y + (goal_x >> 1)];
        if ((candidate.climate & 0xe0U) >= 0x60U) {
            goal_x = destination_x;
            goal_y = destination_y;
        }
    }
    const Map &goal = MapTiles[MapLongitude * goal_y + (goal_x >> 1)];
    if ((goal.climate & 0xe0U) < 0x60U) {
        add_goal(action_mode, 6, 2, goal_x, goal_y, -1);
    }
}

static void order_veh_facing_keepmoving_tail(
    int veh_id, int direction, int action_mode) {
    Veh &vehicle = Vehs[veh_id];

    if (action_mode == ConsoleGlobal.faction_id_
        && (vehicle.order == ORDER_MOVE_TO ||
            (vehicle.state & 0x00004200u) != 0)) {
        ++vehicle.unk_6;
        if ((vehicle.state & 0x00010000u) != 0) {
            const int reverse_facing = static_cast<int>(vehicle.facing) ^ 4;
            if (direction == reverse_facing) {
                vehicle.unk_6 = static_cast<uint8_t>(vehicle.unk_6 + 8u);
            }
        }

        if (vehicle.unk_6 >= 0x20) {
            parse_says(0, VehPrototypes[vehicle.proto_id].veh_name, -1, -1);
            if (IsMultiplayerNet != 0 || X_pop("KEEPMOVING", NULL) != 0) {
                vehicle.order = ORDER_NONE;
                vehicle.state &= 0xFCFFBDFFu;
            }
            vehicle.unk_6 = 0;
        }
    }

    if (direction >= 0 && direction < 8) {
        vehicle.facing = static_cast<int8_t>(direction);
        vehicle.state |= 0x00010000u;
    } else {
        vehicle.state &= ~0x00010000u;
    }
}

static void order_veh_59977a_selector_and_action_corridor(
    int &veh_id, int vehicle_faction_snapshot, int native_stack_candidate,
    bool native_local_68, int original_direction, int action_mode,
    int destination_base_id, int destination_x, int destination_y,
    const Map &destination_tile) {
    if (native_local_68) {
        sleep(native_stack_candidate);
        veh_skip(native_stack_candidate);

        if (native_stack_candidate >= 0) {
            int stack_member = veh_id;
            while (stack_member >= 0 &&
                   Vehs[stack_member].prev_veh_id_stack >= 0) {
                stack_member = Vehs[stack_member].prev_veh_id_stack;
            }
            for (int member = stack_member; member >= 0;
                 member = Vehs[member].next_veh_id_stack) {
                if (veh_cargo(member) == 0) {
                    const int available = speed(member, 0);
                    const int spent = Vehs[member].moves_expended;
                    if (spent > available) {
                        Vehs[member].order = ORDER_NONE;
                    } else {
                        stack_veh(member, 0);
                        if (Vehs[veh_id].waypoint_x[0] == member) {
                            ConsoleGlobal.selected_vehicle_ = member;
                            Vehs[member].order = ORDER_NONE;
                            break;
                        }
                    }
                }
            }
        }

        draw_tile(destination_x, destination_y, 2);
    }

    bool reaches_goal_scan = true;
    const uint8_t faction_lock_bit = static_cast<uint8_t>(
        1U << (static_cast<unsigned>(vehicle_faction_snapshot) & 31U));
    if ((static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) & faction_lock_bit) != 0 ||
        VehPrototypes[Vehs[veh_id].proto_id].plan != PLAN_NAVAL_TRANSPORT ||
        (destination_tile.climate & 0xE0U) >= 0x60U ||
        destination_base_id >= 0) {
        reaches_goal_scan = false;
    } else {
        const int remaining_speed = speed(veh_id, 0) -
                                    Vehs[veh_id].moves_expended;
        if (remaining_speed > 0) {
            reaches_goal_scan = false;
        } else {
            const int zoc = zoc_any(Vehs[veh_id].x, Vehs[veh_id].y,
                                    vehicle_faction_snapshot);
            if (zoc == 0 &&
                (GameDataStateStorage.fields_.lock_masks_.enable_mask_ & Vehs[veh_id].visibility) == 0 &&
                Vehs[veh_id].move_to_ai_type != 0x75U) {
                reaches_goal_scan = true;
            } else {
                const int vehicle_id = veh_id;
                int head = vehicle_id;
                while (head >= 0 && Vehs[head].prev_veh_id_stack >= 0)
                    head = Vehs[head].prev_veh_id_stack;
                for (int member = head;
                     member >= 0;
                     member = Vehs[member].next_veh_id_stack) {
                    if (Vehs[member].order == 1 &&
                        Vehs[member].waypoint_x[0] == vehicle_id) {
                        Vehs[vehicle_id].order = ORDER_NONE;
                    }
                }
            }
        }
    }

    if (reaches_goal_scan) {
        order_veh_move_to_terrain_goal(
            veh_id, action_mode, destination_x, destination_y);
    }
    order_veh_facing_keepmoving_tail(veh_id, original_direction,
                                     action_mode);
}

static void order_veh_finalize_unselected_action(
    int veh_id, int action_faction_id, int action_mode) {
    Veh &vehicle = Vehs[veh_id];
    vehicle.order = ORDER_NONE;

    const uint32_t lock_bit = 1u << (action_faction_id & 0x1f);
    const bool faction_locked =
        (static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) & lock_bit) != 0;
    if (!faction_locked || (vehicle.state & 0x00004200u) != 0) {
        vehicle.unk_6 = static_cast<uint8_t>(vehicle.unk_6 + 1u);
        if (vehicle.unk_6 >= 0x18u || faction_locked) {
            vehicle.unk_6 = 0;
            veh_skip(veh_id);
        }
    }

    if ((action_mode & 2) == 0 && IsMultiplayerNet != 0 &&
        ConsoleControlTurnActive == 0) {
        synch_veh(veh_id);
        NetDaemonNet.await_synch();
    }
}

// Vehicle movement owner entry.
// The movement entry retains one automatic Popup and the original shared-state joins.
// The few reused scalar slots deliberately have no default initialization.
// ORIGINAL: 0x005947C0 ?order_veh@@YAHHHH@Z 0x005947C0-0x0059A1AB;0x00660F47-0x00661200
// symbol    ?order_veh@@YAHHHH@Z
// kind      game
// DEFERRED: largest address in this batch (6708 instructions, no jump table) and its frame confirms the SAME object graph as 0x005BB000/tech_achieved in this batch, offset-for-offset: [ebp-0x5410] BasePop,GraphicWin,Popup   [ebp-0x49B4]/[ebp-0x3E68] FlatButton (Popup's own two, gap 0xB4C = sizeof(FlatButton) each)   [ebp-0x32F8] Sprite   [ebp-0x31F8] ListBox [ebp-0x30B8] GraphicWin   [ebp-0x26A0] Dialog   [ebp-0x2378] Spot   [ebp-0x21E0] GraphicWin (a second, separate Scroll) [ebp-0x1734]/[ebp-0xBE8] FlatButton,BaseButton (that Scroll's own two buttons) - plus [eax+0x1C] CheckBox and [eax+0x8C] EditGroup/SpriteBox reached through a register, matching tech_achieved's DialogEntry-linked-list widget teardown exactly. tech_achieved's own DEFERRED note (this batch) traced that ListBox/ GraphicWin/Dialog cluster to a hand-inlined `Dialogs::Dialogs()` doing real vbtable-relative virtual-base placement - i.e. `class Dialogs : public ListBox` sharing a GraphicWin virtual base, which the flattened `Dialogs` class this file's scaffold emits (a direct member, not `: ListBox`) cannot reproduce. Whatever fraction of THIS function's Popup/Dialogs teardown block hits that same inlined constructor is blocked the same way; the rest is dispatch-heavy multiplayer/combat logic (?parse_says@@ 36x, ?pop@NetMsg@@ 28x, ?stack_check@@ 20x, ?speed@@ 18x, ?draw_tile@@ 12x, ?has_abil@@ 10x, ?veh_skip@@ 8x, ?synch_veh@@/?await_synch@NetDaemon@@ 7x, _rand 7x) with the same low-repetition/bespoke-argument risk profile flagged on 0x00561080/enemy_strategy in this batch.
// Historical scaffold dependency note (not a current blocker):
// UNBLOCKER: same as 0x005BB000 for the Dialogs cluster (direct-offset
//     approximation, not true vbtable computation); the rest needs the
//     g_00xxxxxx cross-check technique applied at this function's scale,
//     which is a multi-pass undertaking on its own given 6708
//     instructions.
int __cdecl order_veh(int veh_id, int direction, int faction) {
    int selector = 0;
    int facility_present = 0;
    int battle_continuation = 0;
    int monolith_suppressed = 0;
    bool probe_attempted = false;
    Popup popup;
    int local_24;
    int local_2c;
    int local_30;
    int selected_passenger;
    int triad;
    int visible_vehicle;
    int controller;
    int base_id;
    int goody_type;
    int region;
    int source_lowland;
    int destination_lowland;
    int faction_mask;
    int destination_x;
    int destination_y;
    bool animate;
    NetDaemonNet.channels_[0].leading_words_[3] = faction;
    if ((faction & 1) != 0) ConsoleGlobal.field_23BE4_ = 0;
    else ConsoleGlobal.vehicle_order_ui_state() = 0;
    const int group_count_at_entry = GameDataStateStorage.fields_.vehicle_current_count_;
    const int source_x = Vehs[veh_id].x;
    const int source_y = Vehs[veh_id].y;
    faction = Vehs[veh_id].faction_id;
    if (direction < 0) direction = 8;
    destination_x = wrap_map_x(source_x + VehMoveDeltaX[direction]);
    destination_y = source_y + VehMoveDeltaY[direction];
    if (!on_map(destination_x, destination_y)) goto label_599b1a;
    region = map_loc(destination_x, destination_y)->region;
    visible_vehicle = stack_fix(veh_at(destination_x, destination_y));
    base_id = base_at(destination_x, destination_y);
    if (base_id >= 0) controller = Bases[base_id].faction_id_current;
    else if (visible_vehicle >= 0) controller = Vehs[visible_vehicle].faction_id;
    else controller = -1;
    source_lowland = (map_loc(source_x, source_y)->climate & 0xe0) < 0x60;
    destination_lowland = (map_loc(destination_x, destination_y)->climate & 0xe0) < 0x60;
    goody_type = goody_at(destination_x, destination_y);
    faction_mask = 1 << (faction & 31);

label_5949b3:
    local_30 = (Vehs[veh_id].state & 0x4200) != 0;
    triad = Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad;
    if (order_veh_waypoint_treaty_clears_order(
            veh_id, faction, (faction_mask & static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_)) != 0))
        goto label_599b1a;
    if (order_veh_artifact_announcement(veh_id, base_id, faction,
            NetDaemonNet.channels_[0].leading_words_[3])) {
        order_veh_start_artifact_popup(popup);
        order_veh_artifact_picture(popup, popup.owned_picture(), "art_dis_sm.pcx", "art_dis_sm.pcx");
        order_veh_artifact_cost_numbers(base_id, faction);
        if (order_veh_artifact_dialog_item(popup, ScriptPopupFile, "ARTIFACTMENU", 0))
            order_veh_artifact_dialog_rows(popup, veh_id, base_id, faction);
    }
    if (direction >= 8) {
        if (order_veh_direction8_popup_gate(veh_id, faction, destination_x,
                destination_y, NetDaemonNet.channels_[0].leading_words_[3])) {
            if (IsMultiplayerNet && !ConsoleControlTurnActive) {
                synch_veh(veh_id);
                NetDaemonNet.await_synch();
            }
            return 0;
        }
        selector = 1;
        if ((NetDaemonNet.channels_[0].leading_words_[3] & 2) == 0)
            goto label_599b1a;
        veh_skip(veh_id);
        Vehs[veh_id].state &= ~0x40U;
        goto label_598cb9;
    }
    Vehs[veh_id].state |= 4;
    if (VehPrototypes[Vehs[veh_id].proto_id].plan == PLAN_INFO_WARFARE &&
            controller >= 0 && faction != controller) {
        if ((NetDaemonNet.channels_[0].leading_words_[3] & 1) != 0) {
            Vehs[veh_id].flags &= ~4U;
            if ((PlayersData[faction].diplo_treaties[controller] & 1) != 0 &&
                    !local_30 && faction == ConsoleGlobal.faction_id_) {
                int accepted;
                if (base_id >= 0) {
                    parse_says(0, Bases[base_id].name_string, -1, -1);
                    parse_says(1, Players[controller].adj_name_faction, -1, -1);
                    accepted = X_pop("PROBEPACT", NULL);
                } else {
                    if (!stack_check(visible_vehicle, 2, PLAN_INFO_WARFARE, -1, -1))
                        goto label_594f3c;
                    parse_says(0, Players[controller].adj_name_faction, -1, -1);
                    accepted = X_pop("PROBEPACTTEAM", NULL);
                }
                if (!accepted) goto label_595083;
                Vehs[veh_id].flags |= 4;
            }
        }
label_594f3c:
        if (base_id >= 0) {
            if (visible_vehicle >= 0 && stack_check(visible_vehicle, 2,
                    PLAN_INFO_WARFARE, -1, -1)) goto label_595083;
            if ((faction_mask & static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_)) == 0 &&
                    (PlayersData[faction].diplo_treaties[controller] & 1) != 0 &&
                    (Vehs[veh_id].waypoint_x[0] != Bases[base_id].x ||
                     Vehs[veh_id].waypoint_y[0] != Bases[base_id].y)) goto label_595083;
            if (controller == ConsoleGlobal.faction_id_)
                order_veh_local_stack_animation(veh_id, direction, source_x,
                    source_y, destination_x, destination_y, faction);
            if (order_veh_direction8_probe_sync(veh_id, base_id,
                    NetDaemonNet.channels_[0].leading_words_[3])) return 1;
            goto label_599b1a;
        }
        if ((PlayersData[faction].diplo_treaties[controller] & 1) == 0) {
            if (visible_vehicle >= 0 && stack_check(visible_vehicle, 2,
                    PLAN_INFO_WARFARE, -1, -1)) goto label_595083;
            probe_attempted = true;
            if (probe(veh_id, visible_vehicle, -1, 1) && IsMultiplayerNet &&
                    !ConsoleControlTurnActive &&
                    (NetDaemonNet.channels_[0].leading_words_[3] & 1) != 0 &&
                    (NetDaemonNet.channels_[0].leading_words_[3] & 2) == 0) {
                synch_veh(veh_id);
                NetDaemonNet.await_synch();
                return 1;
            }
            if (GameDataStateStorage.fields_.vehicle_current_count_ != group_count_at_entry) return 0;
            if (visible_vehicle >= 0) controller = Vehs[visible_vehicle].faction_id;
        }
    }

label_595083:
    if (triad == TRIAD_LAND && destination_lowland) {
        if (faction == controller) goto label_5952c6;
        if ((PlayersData[faction].diplo_treaties[controller] & 1) != 0)
            goto label_599b1a;
        if (base_id >= 0) {
            if (!has_abil(Vehs[veh_id].proto_id, ABL_AMPHIBIOUS)) {
                order_veh_amphibbase_notice(NetDaemonNet.channels_[0].leading_words_[3],
                    faction, "AMPHIBBASE");
                goto label_599b1a;
            }
            goto label_59533d;
        }
        if (visible_vehicle < 0 || !has_abil(Vehs[veh_id].proto_id, 0x20) ||
                !stack_check(visible_vehicle, 0x12, -1, -1, -1) ||
                stack_check(visible_vehicle, 6, 0x80, -1, -1)) goto label_599b1a;
        goto label_59512c;
    }
    goto label_59533d;

label_5952c6:
    {
        const OrderVeh595083TransportPath path =
            order_veh_595083_amphibious_transport_fork(veh_id, faction,
                NetDaemonNet.channels_[0].leading_words_[3], base_id, visible_vehicle);
        if (path == OrderVeh595083TransportCommonCleanup) goto label_599b1a;
        if (base_id < 0) facility_present = 1;
    }

label_59533d:
    if (order_veh_zoc_branch_gate(VehPrototypes[Vehs[veh_id].proto_id],
            Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id],
            source_lowland != 0, destination_lowland != 0, visible_vehicle) &&
            zoc_move(source_x, source_y, faction) &&
            zoc_move(destination_x, destination_y, faction)) {
        if (order_veh_zoc_tutorial_notice(faction, local_30,
                NetDaemonNet.channels_[0].leading_words_[3], veh_id,
                source_x, source_y, destination_x, destination_y, OrderVehTutorialNoticeCounter))
            order_veh_reveal_unseen_hostiles(faction, destination_x, destination_y,
                static_cast<uint8_t>(faction_mask));
        goto label_599b1a;
    }
    {
        const OrderVehSupplyOptionsPath supply = order_veh_supply_options_join(
            veh_id, VehPrototypes[Vehs[veh_id].proto_id].plan, base_id,
            NetDaemonNet.channels_[0].leading_words_[3]);
        if (supply == OrderVehSupplyOptionsSyncedReturnOne) return 1;
        if (supply == OrderVehSupplyOptionsCommonCleanupReturnZero) goto label_599b1a;
    }
    {
        const OrderVeh595422Path supply = order_veh_595422_supply_spawn(veh_id,
            NetDaemonNet.channels_[0].leading_words_[3], faction,
            destination_x, destination_y, source_lowland != 0,
            destination_lowland != 0, visible_vehicle, local_24);
        if (supply == OrderVeh595422CommonCleanup ||
                supply == OrderVeh595422SpawnedCommonCleanup) goto label_599b1a;
        if (supply == OrderVeh595422Continue596052) goto label_596052;
        if (supply == OrderVeh595422Continue595137) goto label_595137;
    }

label_59512c:
    if (visible_vehicle < 0) goto label_596052;

label_595137:
    local_24 = visible_vehicle * sizeof(Veh);
    {
        const OrderVeh595137Path path = order_veh_595137_gate(veh_id,
            visible_vehicle, faction, controller, destination_x, destination_y,
            base_id, probe_attempted, NetDaemonNet.channels_[0].leading_words_[3]);
        if (path == OrderVeh595137CommonCleanup) goto label_599b1a;
        if (path == OrderVeh595137Continue596052) goto label_596052;
    }
    if (order_veh_59521f_front_gate(veh_id, visible_vehicle, faction, base_id,
            *map_loc(destination_x, destination_y),
            NetDaemonNet.channels_[0].leading_words_[3]) == OrderVeh59521fCommonCleanup)
        goto label_599b1a;
    if (order_veh_595798_artillery_amphib(veh_id, faction, source_lowland != 0,
            destination_lowland != 0, triad, *map_loc(source_x, source_y),
            NetDaemonNet.channels_[0].leading_words_[3]) == OrderVeh595798CommonCleanup)
        goto label_599b1a;
    {
        const RulesWeapon &weapon = Weapon[VehPrototypes[Vehs[veh_id].proto_id].weapon_id];
        if (weapon.mode == 11 && stack_check(visible_vehicle, 2, PLAN_INFO_WARFARE, -1, -1))
            goto label_595a92;
        if (weapon.offense_rating == 0) {
            order_veh_noncombatant_notice(faction, NetDaemonNet.channels_[0].leading_words_[3]);
            goto label_599b1a;
        }
        if (VehPrototypes[Vehs[visible_vehicle].proto_id].plan != PLAN_INFO_WARFARE ||
                stack_check(visible_vehicle, 1, -1, -1, -1) != 1 || IsMultiplayerNet)
            goto label_595a92;
        if ((PlayersData[faction].diplo_treaties[controller] & 6) == 0 ||
                (PlayersData[faction].diplo_treaties[controller] & 1) != 0)
            goto label_595a92;
        const OrderVeh5958B8PromptPath prompt = order_veh_5958b8_probe_prompt(
            visible_vehicle, faction, controller, destination_x, destination_y,
            NetDaemonNet.channels_[0].leading_words_[3]);
        if (prompt == OrderVeh5958B8PromptCleanup) goto label_599b1a;
        if (prompt == OrderVeh5958B8PromptContinueProbeSuccess) {
            order_veh_probe_success_corridor(veh_id, faction, controller,
                ConsoleGlobal.faction_id_, direction, destination_x, destination_y,
                visible_vehicle, source_x, source_y);
            goto label_599b1a;
        }
    }

label_595a92:
    if (order_veh_595a92_hasty_gate(veh_id, faction,
            NetDaemonNet.channels_[0].leading_words_[3], local_30 != 0) ==
            OrderVeh595A92CommonCleanup) goto label_599b1a;
    goto label_595be6;

// In-function fragment. Inputs and joins are documented in contracts.md.
// The enclosing owner supplies the sole Popup and native automatic lifetime.
label_595be6:
    GameDataStateStorage.fields_.vehicle_selection_follow_limit_ = veh_id;
    if ((NetDaemonNet.channels_[0].leading_words_[3] & 2) == 0)
        goto label_59729d;
    if (controller != 0) goto label_59724d;
    if (Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad == 2)
        goto label_59724d;
    if (Vehs[visible_vehicle].proto_id == 7) goto label_59724d;
    if (Vehs[visible_vehicle].proto_id == 19) goto label_59724d;
    local_2c = faction * static_cast<int>(sizeof(PlayerData));
    if (PlayersData[faction].soc_effect_active.values_[SocialEffect::PLANET] <= 0)
        goto label_59724d;
    {
        int remaining = speed(veh_id, 0) - Vehs[veh_id].moves_expended;
        if (remaining < 0) remaining = 0;
        else if (remaining > 999) remaining = 999;
        if (remaining < static_cast<int>(Rules.move_rate_roads)) {
            if (PlayersData[faction].soc_effect_base.values_[SocialEffect::PLANET] <= 0)
                goto label_59724d;
            if (PlayersData[faction].proto_id_active[Vehs[visible_vehicle].proto_id] != 0)
                goto label_59724d;
        }
    }
    {
        const int nearby_base = base_find(destination_x, destination_y, faction);
        if ((NetDaemonNet.channels_[0].leading_words_[3] & 2) == 0)
            goto label_59724d;
        if (Vehs[visible_vehicle].order_auto_type == faction)
            goto label_597216;
        if (nearby_base >= 0 && Bases[nearby_base].eco_damage != 0)
            goto label_597216;
    }
    log_say("Precapture who, enemy", faction, visible_vehicle, 0);
    log_say("Precapture fac, proto", PlayersData[faction].soc_effect_base.values_[SocialEffect::PLANET],
            PlayersData[faction].proto_id_active[Vehs[visible_vehicle].proto_id], 0);
    log_say("Precapture type, soc2", Vehs[visible_vehicle].proto_id,
            PlayersData[faction].soc_effect_active.values_[SocialEffect::PLANET], 0);
    if ((Vehs[visible_vehicle].flags & 0x2000) != 0)
        goto label_5971ce;
    if (rand() % 4 < PlayersData[faction].soc_effect_active.values_[SocialEffect::PLANET]) {
        const int bound = GameDataStateStorage.fields_.turn_current_num_ / 5 + 100;
        int roll;
        if (bound - 1 > 0) roll = rand() % bound;
        else roll = 0;
        if (roll >= PlayersData[faction].proto_id_active[Vehs[visible_vehicle].proto_id] * 10)
            goto label_595e1b;
        if (rand() % 10 == 0) goto label_595e1b;
    }
    if (PlayersData[faction].soc_effect_base.values_[SocialEffect::PLANET] <= 0)
        goto label_5971ce;
    if (PlayersData[faction].proto_id_active[Vehs[visible_vehicle].proto_id] != 0)
        goto label_5971ce;

label_595e1b:
    {
        int captured_vehicle = visible_vehicle;
        if (captured_vehicle >= 0) {
            int previous = Vehs[visible_vehicle].prev_veh_id_stack;
            while (previous >= 0) {
                captured_vehicle = previous;
                previous = Vehs[captured_vehicle].prev_veh_id_stack;
            }
            if (captured_vehicle >= 0) {
                do {
                    Veh &captured = Vehs[captured_vehicle];
                    captured.faction_id = static_cast<unsigned char>(faction);
                    captured.morale = static_cast<unsigned char>(morale_alien(captured_vehicle, 0));
                    const int nearest = base_find(destination_x, destination_y);
                    // Original indexes Bases even when nearest is negative.
                    if (Bases[nearest].faction_id_current != faction ||
                        Bases[nearest].population_size < 3)
                        captured.home_base_id = -1;
                    else
                        captured.home_base_id = static_cast<short>(nearest);
                    veh_skip(captured_vehicle);
                    if (captured_vehicle < 0) break;
                    captured_vehicle = captured.next_veh_id_stack;
                } while (captured_vehicle >= 0);
            }
        }
    }
    bit_set(destination_x, destination_y, 0x100000, 1);
    owner_set(destination_x, destination_y, faction);
    if (faction == ConsoleGlobal.faction_id_)
        draw_tile_fixup(destination_x, destination_y, 1, 2);
    else
        draw_tile(destination_x, destination_y, 2);
    if (faction == ConsoleGlobal.faction_id_) {
        parse_says(0, VehPrototypes[Vehs[visible_vehicle].proto_id].veh_name, -1, -1);
        const int captured_proto = Vehs[visible_vehicle].proto_id;
        if (captured_proto == 15) {
            if (PlayersData[faction].proto_id_active[15] != 0)
                NetMsgPrimary.pop("WORMCAPTURE", 5000, 0, 0);
            else
                popp(ScriptPopupFile, "WORMCAPTURE", 0, "sporlnch_sm.pcx", 0);
        } else if (captured_proto == 14) {
            if (PlayersData[faction].proto_id_active[14] != 0)
                NetMsgPrimary.pop("WORMCAPTURE", 5000, 0, 0);
            else
                popp(ScriptPopupFile, "WORMCAPTURE", 0, "sealrk_sm.pcx", 0);
        } else {
            if (PlayersData[faction].proto_id_active[captured_proto] != 0)
                NetMsgPrimary.pop("WORMCAPTURE", 5000, 0, 0);
            else {
                const char *picture = "isle_sm.pcx";
                if ((map_loc(destination_x, destination_y)->climate & 0xe0) >= 0x60)
                    picture = "mindworm_sm.pcx";
                popp(ScriptPopupFile, "WORMCAPTURE", 0, picture, 0);
            }
        }
    }
    log_say("Captured", 0, 0, 0);
    goto label_596052;

label_5971ce:
    Vehs[visible_vehicle].flags |= 0x2000;
    if (faction == ConsoleGlobal.faction_id_)
        NetMsgPrimary.pop("WORMNOCAPTURE", 5000, 0, 0);
    log_say("No Capture", 0, 0, 0);
    goto label_59724d;

label_597216:
    log_say("Agitated", 0, 0, 0);
    if (faction == ConsoleGlobal.faction_id_)
        NetMsgPrimary.pop("WORMSAGITATED", 5000, 0, 0);

label_59724d:
    monolith_suppressed = 1;
    {
        const int battle_result = battle_fight(veh_id, direction, 0, 1, 0);
        if (battle_result == 0) goto label_599b1a;
        if (battle_result != 1) {
            selector = 0;
            goto label_599b1a;
        }
        veh_id = GameDataStateStorage.fields_.vehicle_selection_follow_limit_;
        destination_x = source_x;
        destination_y = source_y;
        battle_continuation = 1;
        goody_type = 0;
        goto label_598cb9;
    }

label_59729d:
    selector = battle_fight(veh_id, direction, 0, 1, 0) == 0;
    goto label_599b1a;

label_596052:
    if (destination_lowland) goto label_597345;
    if (source_lowland && triad == TRIAD_LAND && visible_vehicle < 0 &&
            !has_abil(Vehs[veh_id].proto_id, ABL_AMPHIBIOUS) &&
            (map_loc(source_x, source_y)->bit & BIT_BASE_IN_TILE) != 0 &&
            (map_loc(source_x, source_y)->val2 & 15) < 8 &&
            !stack_check(veh_id, 2, PLAN_NAVAL_TRANSPORT, -1, -1)) {
        order_veh_amphibbase_notice(NetDaemonNet.channels_[0].leading_words_[3],
            faction, "AMPHIBBASE2");
        goto label_599b1a;
    }
    if (base_id >= 0) goto label_597350;
    if (triad != TRIAD_SEA) goto label_597345;
    if (IsMultiplayerNet || faction != ConsoleGlobal.faction_id_ ||
            (NetDaemonNet.channels_[0].leading_words_[3] & 1) == 0)
        goto label_599b1a;
    if (order_veh_disembark_dialog(popup, veh_id, selected_passenger) !=
            OrderVehDisembarkDialogSelectedAt5962BE) goto label_599b1a;
    veh_id = selected_passenger;
    goto label_5949b3;

label_597345:
    if (base_id < 0) goto label_597564;
// Additional owner-requested raw-asm audit:597350..5976D6.
label_597350:
    if (controller == faction) goto label_597564;
    if ((PlayersData[faction].diplo_treaties[controller] & 1) != 0)
        goto label_597564;
    if (triad == 2) {
        const RulesChassis &chassis = Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id];
        if (chassis.range != 0) {
            if (faction == ConsoleGlobal.faction_id_ &&
                (NetDaemonNet.channels_[0].leading_words_[3] & 1) != 0) {
                parse_say(0, chassis.offsv_1_name, -1, -1);
                NetMsgPrimary.pop("AIRCONQUER", 5000, 0, 0);
            }
            goto label_599b1a;
        }
    } else if (triad == 1) {
        if (!destination_lowland) {
            if (faction == ConsoleGlobal.faction_id_ &&
                (NetDaemonNet.channels_[0].leading_words_[3] & 1) != 0)
                NetMsgPrimary.pop("SEACONQUER", 5000, 0, 0);
            goto label_599b1a;
        }
    } else if (triad == 0 && destination_lowland) {
        if (has_abil(Vehs[veh_id].proto_id, 8) == 0) {
            if (faction == ConsoleGlobal.faction_id_ &&
                (NetDaemonNet.channels_[0].leading_words_[3] & 1) != 0)
                NetMsgPrimary.pop("LANDCONQUER", 5000, 0, 0);
            goto label_599b1a;
        }
    }
    if (Weapon[VehPrototypes[Vehs[veh_id].proto_id].weapon_id].offense_rating == 0) {
        if (faction == ConsoleGlobal.faction_id_ &&
            (NetDaemonNet.channels_[0].leading_words_[3] & 1) != 0)
            NetMsgPrimary.pop("COMBATCONQUER", 5000, 0, 0);
        goto label_599b1a;
    }
    if ((PlayersData[faction].diplo_treaties[controller] & 7) == 0)
        goto label_597564;
    if ((static_cast<unsigned char>(faction_mask) & static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_)) == 0) {
        Vehs[veh_id].order = ORDER_NONE;
        goto label_599b1a;
    }
    if (faction != ConsoleGlobal.faction_id_) goto label_59763b;
    if ((NetDaemonNet.channels_[0].leading_words_[3] & 1) == 0)
        goto label_597564;
    if (break_treaty(faction, controller, 7) != 0) goto label_599b1a;

label_597564:
    if (faction != ConsoleGlobal.faction_id_) goto label_59763b;
    if ((static_cast<unsigned char>(faction_mask) & static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_)) == 0)
        goto label_59763b;
    if ((NetDaemonNet.channels_[0].leading_words_[3] & 1) == 0)
        goto label_59763b;
    if (Vehs[veh_id].order != ORDER_MOVE_TO) goto label_59763b;
    if (Vehs[veh_id].waypoint_x[0] == destination_x &&
        Vehs[veh_id].waypoint_y[0] == destination_y)
        goto label_59763b;
    if (Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad == 2)
        goto label_59763b;
    if ((Vehs[veh_id].state & 0x4200) != 0) goto label_59763b;
    if (goody_at(destination_x, destination_y) == 0) goto label_59763b;
    if (popp(ScriptPopupFile, "GOTOGOODY", 0, "supply_sm.pcx", 0) != 0) {
        Vehs[veh_id].order = ORDER_NONE;
        goto label_599b1a;
    }

label_59763b:
    {
        const int initial_remaining = speed(veh_id, 0) - Vehs[veh_id].moves_expended;
        if (initial_remaining < 0) goto label_599b1a;
        if (initial_remaining <= 999 && initial_remaining == 0)
            goto label_599b1a;
        const int movement_cost = hex_cost(Vehs[veh_id].proto_id, faction,
                                          source_x, source_y,
                                          destination_x, destination_y, 0);
        // Native loads this byte before its second speed call.
        const int expended_before_speed = Vehs[veh_id].moves_expended;
        local_2c = movement_cost;
        int remaining = speed(veh_id, 0) - expended_before_speed;
        if (remaining < 0) remaining = 0;
        else if (remaining > 999) remaining = 999;
        if (local_2c <= remaining) goto label_597b24;
    }
    goto label_5976d6;
label_5976d6:
    if (triad == TRIAD_LAND &&
            order_veh_5976d6_terrain_delay(veh_id, faction, direction,
                source_x, source_y, destination_x, destination_y, visible_vehicle,
                local_2c, local_30 != 0, selector) == OrderVeh5976d6CommonCleanup)
        goto label_599b1a;
label_597b24:
    if (order_veh_597b24_entry(veh_id, faction, ConsoleGlobal.faction_id_, base_id,
            controller, local_2c, selector) == OrderVeh597b24EntryCleanupOne)
        goto label_599b1a;
    if (order_veh_597cc8_to_597f31(veh_id, faction, ConsoleGlobal.faction_id_,
            destination_x, destination_y)) {
        local_24 = 1;
        const OrderVeh5980a8State state = order_veh_597f31_to_5980a8(veh_id,
            faction, ConsoleGlobal.faction_id_, base_id, destination_x,
            destination_y, source_x, source_y, triad, local_2c, true);
        local_30 = state.native_local_30;
        animate = state.native_v42;
    } else {
        local_24 = 0;
        local_30 = 0;
        animate = false;
        stack_veh(veh_id, 1);
    }
    order_veh_5980a8_restack_and_visibility(veh_id, faction, direction,
        destination_x, destination_y, source_x, source_y,
        *map_loc(source_x, source_y), *map_loc(destination_x, destination_y),
        animate, local_24 != 0);
    order_veh_598240_no_base_bunker_response(veh_id, faction, destination_x,
        destination_y, base_id, *map_loc(destination_x, destination_y));
    if (base_id >= 0 && faction != controller && faction == 0) {
        const OrderVehNativeLifeEventResult event = order_veh_598317_native_life_base_event(
            veh_id, base_id, controller, faction, destination_x, destination_y,
            local_24 != 0, local_30);
        facility_present = event.native_local_68;
        local_30 = event.native_local_30;
        goto label_599b1a;
    }
    order_veh_5986ec_stack_put_and_capture(veh_id, faction, destination_x,
        destination_y, base_id, controller);
    if (base_id < 0)
        order_veh_598793_no_base_sea_artillery_scan(veh_id, faction,
            destination_x, destination_y, destination_lowland != 0);
    order_veh_598993_clear_carried_sentry_orders(veh_id, faction, base_id,
        destination_x, destination_y, static_cast<uint8_t>(faction_mask));
    order_veh_598a84_sea_cargo_recovery(veh_id, faction, destination_x,
        destination_y, (GameDataStateStorage.fields_.game_preferences_ & 0x80000000U) != 0, destination_lowland != 0);
    if (triad == TRIAD_LAND && source_lowland && !destination_lowland)
        ++PlayersData[faction].region_total_combat_vehs[region];

label_598cb9:
    order_veh_combat_and_turn_prelude(veh_id, faction,
        NetDaemonNet.channels_[0].leading_words_[3],
        static_cast<uint8_t>(faction_mask), *map_loc(destination_x, destination_y),
        local_24 != 0, local_30 != 0);
    if (Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad == TRIAD_AIR) {
        const bool needs_transport = order_veh_air_departure_stack_needs_transport(
            veh_id, *map_loc(destination_x, destination_y));
        if (order_veh_598e84_range_speed_gate(veh_id, faction,
                static_cast<uint8_t>(faction_mask), needs_transport,
                battle_continuation != 0, destination_x, destination_y) ==
                OrderVeh598E84ContinueTo598F9E) {
            if (order_veh_598f9e_action_home_gate(veh_id, faction,
                    static_cast<uint8_t>(faction_mask)) &&
                    order_veh_59903f_speed_exhausted(veh_id) &&
                    order_veh_599061_fatigue_and_damage(veh_id, faction,
                        static_cast<uint8_t>(faction_mask), local_24 != 0))
                goto label_599b1a;
        }
    }
    if (order_veh_monolith_goody_corridor(veh_id, direction, faction,
            *map_loc(destination_x, destination_y), triad,
            monolith_suppressed != 0, goody_type)) goto label_599b1a;
    {
        int selected_artifact_stack;
        const OrderVehHighlandStackClaim claim = order_veh_highland_stack_claim(
            veh_id, veh_id, base_id, direction, destination_x, destination_y, faction,
            selected_artifact_stack);
        if (claim != OrderVehNoHighlandStackClaim) {
            order_veh_highland_artifact_notice(selected_artifact_stack,
                Vehs[selected_artifact_stack].faction_id, faction);
            faction = Vehs[veh_id].faction_id;
        }
    }
    if (VehPrototypes[Vehs[veh_id].proto_id].plan == PLAN_ALIEN_ARTIFACT &&
            base_id >= 0 && Vehs[veh_id].terraforming_turns != 0) {
        if (Vehs[veh_id].terraforming_turns == 1) {
            if (study_artifact(veh_id)) goto label_599b1a;
        } else if (Vehs[veh_id].terraforming_turns == 2) {
            Base &base = Bases[base_id];
            base.minerals_accumulated += VehPrototypes[Vehs[veh_id].proto_id].cost * 10 / 2;
            base.minerals_accumulated_2 = base.minerals_accumulated;
            base.production_id_last = base.queue_production_id[0];
            kill(veh_id);
            TutWinBaseWindow.check_base(base_id);
        }
    }
    order_veh_599732_mark_region_plan_flag(veh_id, faction, static_cast<uint8_t>(region),
        source_lowland != 0, destination_lowland != 0);
    order_veh_59977a_selector_and_action_corridor(veh_id, faction, veh_id,
        facility_present != 0, direction, faction, base_id, destination_x,
        destination_y, *map_loc(destination_x, destination_y));

label_599b1a:
    if (GameDataStateStorage.fields_.vehicle_current_count_ != group_count_at_entry) return 0;
    if (selector == 0) {
        order_veh_finalize_unselected_action(veh_id, faction,
            NetDaemonNet.channels_[0].leading_words_[3]);
    } else if ((NetDaemonNet.channels_[0].leading_words_[3] & 2) != 0) {
        Veh &vehicle = Vehs[veh_id];
        if (direction >= 0 && direction < 8 && (vehicle.state & 8) != 0 &&
                vehicle.x == destination_x && vehicle.y == destination_y &&
                zoc_veh(destination_x, destination_y, faction) == 0)
            vehicle.state &= ~8U;
        if (IsMultiplayerNet && !ConsoleControlTurnActive && vehicle.order == ORDER_MOVE_TO &&
                vehicle.x == vehicle.waypoint_x[0] && vehicle.y == vehicle.waypoint_y[0]) {
            vehicle.order = ORDER_NONE;
            if ((vehicle.state & 0x02000200U) == 0x02000200U &&
                    vehicle.patrol_current_point < vehicle.waypoint_count) {
                ++vehicle.patrol_current_point;
                vehicle.order = ORDER_MOVE_TO;
                vehicle.waypoint_x[0] = vehicle.waypoint_x[vehicle.patrol_current_point];
                vehicle.waypoint_y[0] = vehicle.waypoint_y[vehicle.patrol_current_point];
                vehicle.state &= ~0x02010200U;
            }
        }
    }
    return selector;
}


/*
Purpose: Test whether one ability is legal for a chassis/weapon/armor
         combination under the faction rule flags: missile chassis carry no
         abilities, the chassis triad gates the ability's land/sea/air bits,
         the weapon's offense/mode pair picks the offensive/defensive and
         special-mode bits, expansion-only abilities drop without
         ExpansionEnabled, and the remaining flags gate psi ratings and
         single-mode chassis.

// ORIGINAL: 0x00584B50 ?abil_cond@@YAHHHHH@Z 0x00584B50-0x00584CFF FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence":["src/veh.h owns the declared Ability, Chassis, Weapon, and Armor record arrays and their field layouts consumed by this body.","designwin.cpp is the existing caller and includes veh.h; the proposal centralizes its declaration in veh.h instead of retaining a caller-local prototype.","All original ability-bit tests, expansion gates, signed ratings, and mode/speed branches are moved as written; no safety or mask normalization is added."],"reason":"Move the existing cdecl ability eligibility routine beside its actual vehicle-record owners and publish its single header declaration; do not alter its historical masks or branch order."}
// TRIED: extern-array field access for the a1*7 flags table and a2*0x90/a3*0x10 lookups; diverges at #5 (prologue register order), not the field math
// size      431 bytes
// prototype int (__cdecl ?abil_cond@@YAHHHHH@Z)(int32 abilityID, int32 chassisID, int32 weaponID, int32 armorID)
// callers   3   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// notes     Seed provenance (TRIED): the a1*7 dword step is Ability[]'s
//           0x1C stride and the a2*0x90/a3*0x10/a4*0x10 lookups are
//           Chassis[]/Weapon[]/Armor[] member reads (veh.h); the surviving
//           divergence is #5, the prologue register order, not the field
//           math - so this body is not measured exact.
Return Value: 1 allowed, 0 not
Status: Complete (rewritten from the image: triad switch, flags reread per test).
*/
int __cdecl abil_cond(int ability_id, int chassis_id, int weapon_id,
                      int armor_id) {
    if (Chassis[chassis_id].missile) {
        return 0;
    }
    switch (Chassis[chassis_id].triad) {
    case TRIAD_LAND:
        if (!(Ability[ability_id].flags & 1)) {
            return 0;
        }
        break;
    case TRIAD_SEA:
        if (!(Ability[ability_id].flags & 2)) {
            return 0;
        }
        break;
    case TRIAD_AIR:
        if (!(Ability[ability_id].flags & 4)) {
            return 0;
        }
        break;
    }
    int8_t offense = Weapon[weapon_id].offense_rating;
    if (offense) {
        if (!(Ability[ability_id].flags & 8)) {
            return 0;
        }
    } else if (Weapon[weapon_id].mode == 9) {
        if (!(Ability[ability_id].flags & 0x10)) {
            return 0;
        }
    } else if (!(Ability[ability_id].flags & 0x20)) {
        return 0;
    }
    if (Weapon[weapon_id].mode == 0xb && Ability[ability_id].flags & 0x40) {
        return 0;
    }
    if (Weapon[weapon_id].mode != 0xb && Ability[ability_id].flags & 0x800) {
        return 0;
    }
    if (!ExpansionEnabled) {
        int bit = 1 << ability_id;
        if (bit == 0x1000000) {
            return 0;
        }
        if (bit == 0x2000000) {
            return 0;
        }
        if (bit == 0x4000000) {
            return 0;
        }
        if (bit == 0x8000000) {
            return 0;
        }
        if (bit == 0x10000000) {
            return 0;
        }
    }
    if ((offense < 0 || Armor[armor_id].defense_rating < 0) && Ability[ability_id].flags & 0x80) {
        return 0;
    }
    if (Ability[ability_id].flags & 0x100 && Weapon[weapon_id].mode != 7) {
        return 0;
    }
    if (Ability[ability_id].flags & 0x200 && Chassis[chassis_id].speed > 1) {
        return 0;
    }
    return 1;
}


/*
Purpose: The single-click handler for a fleet sprite: in a net game a
         selectable, idle, unbusy unit is simply taken; a jailed unit pops
         INJAIL; otherwise the click runs the net lock round-trip (lock,
         wake, synch, unlock) before returning.

// ORIGINAL: 0x0048CF50 ?veh_sprite_click@@YAHH@Z 0x0048CF50-0x0048D006 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence":["veh.h already declares the exact cdecl callback signature and console.cpp installs this function as the sprite click callback.","The body uses the existing Veh array, NetMsgPrimary, NetDaemonNet, and declared selection/wake/sync helpers; lock, wake, sync, await, and unlock order is preserved.","The literal at pinned 0x00686B5C is exactly `49 4e 4a 41 49 4c 00` (`INJAIL\\0`); replacing the fixed pointer alias with the source literal preserves the label bytes without an executable-address dependency.","The callback remains the existing cdecl function, not a guessed member or generated adapter; native symbol/reference measurement remains separate."],"reason":"Move the existing registered click callback into veh.cpp, replace only its verified INJAIL text pointer with the equivalent source literal, and preserve every return/call path."}
// size      182 bytes
// prototype int (__cdecl ?veh_sprite_click@@YAHH@Z)(int a1)
// callers   ?   call targets   7
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0047A890 0x00531020 0x005310F0 0x00531480 0x00593220 0x005C1A20 0x005C1C40 0x005C1D70
Return Value: 1 when the click only selects the unit, else 0
Status: Coverage transcription - claim pending re-measurement
*/
#pragma inline_depth(0)
int __cdecl veh_sprite_click(int veh_id) {
    if (IsMultiplayerNet != 0 && veh_selectable(veh_id) != 0
        && Vehs[veh_id].order == ORDER_NONE
        && (Vehs[veh_id].state & 0x8004200) == 0) {
        return 1;
    }
    if (veh_jail(veh_id) != 0) {
        NetMsgPrimary.pop("INJAIL", 5000, 0, 0);
        return 0;
    }
    if (NetDaemonNet.lock_veh(&veh_id, 0, -1, -1, 0) == 0) {
        veh_wake(veh_id);
        synch_veh(veh_id);
        NetDaemonNet.await_synch();
        NetDaemonNet.unlock_veh();
    }
    return 0;
}
#pragma inline_depth()

/*
Purpose: The double-click twin: the same net-select shortcut and INJAIL
         pop, but the lock failure path returns early and a completed
         lock round-trip reports the unit taken.

// ORIGINAL: 0x0048D010 ?veh_sprite_double_click@@YAHH@Z 0x0048D010-0x0048D0CD FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence":["veh.h already declares the exact cdecl callback signature and console.cpp installs this function as the sprite double-click callback.","The body uses the existing Veh array, NetMsgPrimary, NetDaemonNet, and declared selection/wake/sync helpers; lock failure still returns before the wake/sync path.","The literal at pinned 0x00686B64 is exactly `49 4e 4a 41 49 4c 00` (`INJAIL\\0`); replacing the fixed pointer alias with the source literal preserves the label bytes without an executable-address dependency.","The callback remains the existing cdecl function, not a guessed member or generated adapter; native symbol/reference measurement remains separate."],"reason":"Move the existing registered double-click callback into veh.cpp, replace only its verified INJAIL text pointer with the equivalent source literal, and preserve its distinct lock-failure return."}
// size      189 bytes
// prototype int (__cdecl ?veh_sprite_double_click@@YAHH@Z)(int a1)
// callers   ?   call targets   7
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0047A890 0x00531020 0x005310F0 0x00531480 0x00593220 0x005C1A20 0x005C1C40 0x005C1D70
Return Value: 1 when the double-click takes the unit, else 0
Status: Coverage transcription - claim pending re-measurement
*/
#pragma inline_depth(0)
int __cdecl veh_sprite_double_click(int veh_id) {
    if (IsMultiplayerNet != 0 && veh_selectable(veh_id) != 0
        && Vehs[veh_id].order == ORDER_NONE
        && (Vehs[veh_id].state & 0x8004200) == 0) {
        return 1;
    }
    if (veh_jail(veh_id) != 0) {
        NetMsgPrimary.pop("INJAIL", 5000, 0, 0);
        return 0;
    }
    if (NetDaemonNet.lock_veh(&veh_id, 0, -1, -1, 0) != 0) {
        return 0;
    }
    veh_wake(veh_id);
    synch_veh(veh_id);
    NetDaemonNet.await_synch();
    NetDaemonNet.unlock_veh();
    return 1;
}
#pragma inline_depth()

/*
Purpose: Get the index value of a particular ability's bitfield.
// ORIGINAL: 0x00581170 ?abil_index@@YAHH@Z 0x00581170-0x00581187 BYTE_EXACT
// Previous body location: src/veh.h.
// size      23 bytes
// prototype int (__cdecl ?abil_index@@YAHH@Z)(int abilityID)
// callers   7   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: Ability index
Status: Complete
*/
// Historical placement note (superseded by pinned direct-call evidence):
// BODY IN veh.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.

// An ordinary owner definition restores calls from other translation units,
// including 0x439D5A/0x439D9A/0x439E26. Same-TU Veh calls still auto-inline;
// this placement does not establish the original translation-unit boundaries.
int __cdecl abil_index(int ability_id) {
    int index = 0;
    for (int check = ability_id; !(check & 1); index++) {
        check >>= 1;
    }
    return index;
}
