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
// make_proto calls it out of line in the image.
#pragma auto_inline(off)
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
#pragma auto_inline(on)




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
// Rewritten from the image: one live weapon-cost variable that becomes combat_mod,
//   no zero-armor guard in the ratio case, separate land increment, final >> 2.
Return Value: Cost of prototype
Status: Complete
*/
int __cdecl proto_cost(int chassis_id, int weapon_id, int armor_id,
                            int ability, int reactor_id) {
    int combat_mod = Weapon[weapon_id].cost;
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
                if (factor >= 0) { // 1+ = Straight Cost; 25% increase per unit of cost
                    abil_modifier += factor;
                } else {
                    switch (factor) {
                      // Increases w/ ratio of weapon to armor: 0, 1, or 2. Rounded DOWN.
                      // Never higher than 2. The original has no zero-armor guard here.
                      case -1:
                        abil_modifier += range(combat_mod / armor_cost, 0, 2);
                        break;
                      case -2: // Increases w/ weapon value
                        abil_modifier += combat_mod - 1;
                        break;
                      case -3: // Increases w/ armor value
                        abil_modifier += armor_cost - 1;
                        break;
                      case -4: // Increases w/ speed value
                        abil_modifier += speed_cost - 1;
                        break;
                      case -5: // Increases w/ weapon+armor value
                        abil_modifier += combat_mod + armor_cost - 2;
                        break;
                      case -6: // Increases w/ weapon+speed value
                        abil_modifier += combat_mod + speed_cost - 2;
                        break;
                      case -7: // Increases w/ armor+speed value
                        abil_modifier += armor_cost + speed_cost - 2;
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
        return Weapon[weapon_id].cost;
    }
    if (armor_cost / 2 + 1 > combat_mod) { // which ever is greater
        combat_mod = armor_cost / 2 + 1;
    }
    // (2 << n) == 2^(n + 1) ; (2 << n) / 2 == 2 ^ n;
    int proto_mod = ((speed_cost + armor_cost) * combat_mod + ((2 << reactor_id) / 2))
        / (2 << reactor_id);
    if (speed_cost == 1) {
        proto_mod = (proto_mod / 2) + 1;
    }
    if (Weapon[weapon_id].cost > 1 && Armor[armor_id].cost > 1) {
        proto_mod++;
    }
    if (Chassis[chassis_id].triad == TRIAD_LAND && Weapon[weapon_id].cost > 1 && Armor[armor_id].cost > 1
        && speed_cost > 1) {
        proto_mod++;
    }
    // excludes sea probes
    if (triad == TRIAD_SEA) {
        if (Weapon[weapon_id].mode != WPN_MODE_INFOWAR) {
            proto_mod = (proto_mod + 1) / 2;
        }
    } else if (triad == TRIAD_AIR) {
        proto_mod /= (Weapon[weapon_id].mode > WPN_MODE_MISSILE) ? 2 : 4; // Non-combat : Combat
    }
    int reactor_mod = reactor_id * 2 - reactor_id / 2;
    if (proto_mod <= reactor_mod) { // which ever is greater
        proto_mod = reactor_mod;
    }
    if (combat_mod == 1 && armor_cost == 1 && speed_cost == 1 && reactor_id == RECT_FISSION) {
        proto_mod = 1;
    }
    return (proto_mod * (abil_modifier + 4) + 2) >> 2;
}

/*
Purpose: Create a new prototype. Sets initial values for everything except veh_name and preq_tech.
// ORIGINAL: 0x005A5D40 ?make_proto@@YAXHHHHHH@Z 0x005A5D40-0x005A6269
// size      1321 bytes
// prototype void (__cdecl ?make_proto@@YAXHHHHHH@Z)(int protoID, int chassisType, int weapType, int armorType, int ability, int reactorType)
// callers   4   call targets   4
// kind      game
// flags     frame;sp_ready;purged_ok
// Rewritten from the image: the land plan reads the stored prototype ids, the
//   speed >= 2 / >= 3 unk_1 tests are independent (no else), transport_val stays a call.
// calls     0x0057D510 0x005A5A60 0x005B9F20 0x005BF1F0
Return Value: n/a
Status: Complete
*/
void __cdecl make_proto(int proto_id, int chassis_id, int weapon_id, int armor_id,
                        int ability, int reactor_id) {
    int unk_local_1 = 0; // TODO: Identify
    BOOL cond3 = false;
    BOOL cond2 = false;
    BOOL cond1 = false;
    if (proto_id >= MaxVehProtoFactionNum) {
        int proto_id_loop;
        for (int i = 0; i < 128; i++) {
            proto_id_loop = i % MaxVehProtoFactionNum;
            if (proto_id_loop != i) {
                proto_id_loop += proto_id - proto_id % MaxVehProtoFactionNum;
            }
            int16_t flags_check = VehPrototypes[proto_id_loop].flags;
            if (!(flags_check & PROTO_ACTIVE)) {
                continue;
            }
            if (proto_id_loop < MaxVehProtoFactionNum) {
                if (!has_tech(VehPrototypes[proto_id_loop].preq_tech,
                        proto_id / MaxVehProtoFactionNum)) {
                    continue;
                }
            } else if (!(flags_check & PROTO_TYPED_COMPLETE)) {
                continue;
            }
            uint32_t loop_weapon_id = VehPrototypes[proto_id_loop].weapon_id;
            if (weapon_id == loop_weapon_id) {
                cond1 = true;
            }
            uint32_t loop_armor_id = VehPrototypes[proto_id_loop].armor_id;
            if (armor_id == loop_armor_id) {
                cond2 = true;
            }
            uint32_t loop_chassis_id = VehPrototypes[proto_id_loop].chassis_id;
            if (chassis_id == loop_chassis_id) {
                cond3 = true;
            }
            int8_t off_rating = Weapon[weapon_id].offense_rating;
            if (off_rating > 0 && Weapon[loop_weapon_id].offense_rating >= off_rating) {
                cond1 = true;
            }
            int8_t def_rating = Armor[armor_id].defense_rating;
            if (def_rating > 0 && Armor[loop_armor_id].defense_rating >= def_rating) {
                cond2 = true;
            }
            if (Chassis[chassis_id].triad == Chassis[loop_chassis_id].triad
                && Chassis[loop_chassis_id].speed >= Chassis[chassis_id].speed) {
                cond3 = true;
            }
        }
        if (cond1 && cond2 && cond3) {
            unk_local_1 = 1;
            if (proto_id_loop >= MaxVehProtoFactionNum
                && VehPrototypes[proto_id_loop].flags & 0x10) {
                unk_local_1 = 3;
            }
        }
    }
    VehPrototypes[proto_id].reactor_id = (uint8_t)reactor_id;
    VehPrototypes[proto_id].chassis_id = (uint8_t)chassis_id;
    VehPrototypes[proto_id].weapon_id = (uint8_t)weapon_id;
    VehPrototypes[proto_id].armor_id = (uint8_t)armor_id;
    VehPrototypes[proto_id].ability_flags = ability;
    VehPrototypes[proto_id].cost = (uint8_t)proto_cost(chassis_id, weapon_id, armor_id, ability,
        reactor_id);
    VehPrototypes[proto_id].carry_capacity = (Weapon[weapon_id].mode == WPN_MODE_TRANSPORT)
        ? transport_val(chassis_id, ability, reactor_id) : 0;
    // set plan & unk1
    if (Chassis[chassis_id].missile) {
        if (Weapon[weapon_id].offense_rating >= 99) {
            VehPrototypes[proto_id].plan = PLAN_PLANET_BUSTER;
            VehPrototypes[proto_id].unk_1 = 14;
        } else if (weapon_id == WPN_TECTONIC_PAYLOAD) {
            VehPrototypes[proto_id].plan = PLAN_TECTONIC_MISSILE;
            VehPrototypes[proto_id].unk_1 = 22;
        } else if (weapon_id == WPN_FUNGAL_PAYLOAD) {
            VehPrototypes[proto_id].plan = PLAN_FUNGAL_MISSILE;
            VehPrototypes[proto_id].unk_1 = 23;
        } else { // Conventional Payload
            VehPrototypes[proto_id].plan = PLAN_OFFENSIVE;
            VehPrototypes[proto_id].unk_1 = 13;
        }
    } else if (Weapon[weapon_id].mode >= WPN_MODE_TRANSPORT) { // Non-combat
        VehPrototypes[proto_id].plan = Weapon[weapon_id].mode;
        VehPrototypes[proto_id].unk_1 = Weapon[weapon_id].mode + 0x20;
    } else if (Chassis[VehPrototypes[proto_id].chassis_id].triad == TRIAD_SEA) { // combat sea
        VehPrototypes[proto_id].plan = PLAN_NAVAL_SUPERIORITY;
        VehPrototypes[proto_id].unk_1 = 6; // same value as plan
    } else if (Chassis[VehPrototypes[proto_id].chassis_id].triad == TRIAD_AIR) { // combat air
        if (has_abil(proto_id, ABL_AIR_SUPERIORITY)) {
            VehPrototypes[proto_id].plan = PLAN_AIR_SUPERIORITY;
            VehPrototypes[proto_id].unk_1 = 9;
        } else {
            VehPrototypes[proto_id].plan = PLAN_OFFENSIVE;
            VehPrototypes[proto_id].unk_1 = 8;
        }
    } else { // TRIAD_LAND combat unit; reads go through the stored prototype ids
        VehPrototypes[proto_id].plan = PLAN_OFFENSIVE;
        if (Armor[VehPrototypes[proto_id].armor_id].defense_rating > 1) {
            VehPrototypes[proto_id].plan = PLAN_DEFENSIVE;
        }
        if (Weapon[VehPrototypes[proto_id].weapon_id].offense_rating >= Armor[VehPrototypes[proto_id].armor_id].defense_rating
            && VehPrototypes[proto_id].plan == PLAN_DEFENSIVE) {
            VehPrototypes[proto_id].plan = PLAN_COMBAT;
        }
        if (Chassis[VehPrototypes[proto_id].chassis_id].speed > 1
            && Weapon[VehPrototypes[proto_id].weapon_id].offense_rating > Armor[VehPrototypes[proto_id].armor_id].defense_rating) {
            VehPrototypes[proto_id].plan = PLAN_OFFENSIVE;
        }
        if (ability & (ABL_ARTILLERY | ABL_DROP_POD | ABL_AMPHIBIOUS)) {
            VehPrototypes[proto_id].plan = PLAN_OFFENSIVE;
        }
        VehPrototypes[proto_id].unk_1 = 3;
        if (Armor[VehPrototypes[proto_id].armor_id].defense_rating > Weapon[VehPrototypes[proto_id].weapon_id].offense_rating) {
            VehPrototypes[proto_id].unk_1 = 2;
        }
        if (Weapon[VehPrototypes[proto_id].weapon_id].offense_rating > Armor[VehPrototypes[proto_id].armor_id].defense_rating * 2) {
            VehPrototypes[proto_id].unk_1 = 1;
        }
        if (Chassis[VehPrototypes[proto_id].chassis_id].speed >= 2) {
            VehPrototypes[proto_id].unk_1 = 4;
        }
        if (Chassis[VehPrototypes[proto_id].chassis_id].speed >= 3) {
            VehPrototypes[proto_id].unk_1 = 5;
        }
        if (Weapon[VehPrototypes[proto_id].weapon_id].offense_rating == 1
            && Armor[VehPrototypes[proto_id].armor_id].defense_rating == 1) {
            VehPrototypes[proto_id].unk_1 = 10;
            if (Chassis[VehPrototypes[proto_id].chassis_id].speed > 1) {
                VehPrototypes[proto_id].plan = PLAN_RECONNAISANCE;
            }
            if (has_abil(proto_id, ABL_POLICE_2X)) {
                VehPrototypes[proto_id].plan = PLAN_DEFENSIVE;
            }
        }
    }
    // set final values
    VehPrototypes[proto_id].flags = PROTO_ACTIVE;
    if (unk_local_1) {
        VehPrototypes[proto_id].flags = 0x105;
        if (unk_local_1 & 2) {
            VehPrototypes[proto_id].flags = 0x115;
        }
    }
    VehPrototypes[proto_id].obsolete_factions = 0;
    VehPrototypes[proto_id].combat_factions = (proto_id < MaxVehProtoFactionNum) ?
        0xFF : 1 << (proto_id / MaxVehProtoFactionNum);
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
// Rewritten from the image 2026-11-08 to 0.98 (one register tie in the compatibility
//   test leaves a reload of ability_flags in the found block).
// TRIED: Ghidra pseudocode used as primary source (call arg orders for is_bunged, psi_factor x4, has_tech, retire_proto, make_proto, name_proto, prune_protos all independently confirmed against raw disasm push order - none needed fixing), with the one known-recurring bug fixed: DAT_009ab898 is a byte array with real record stride 0x34, not Ghidra's 2-byte-scaled 0x1a (same defect class as 0x004AB600's 0x9ab898 use). Compiles; not chased to byte-exact given size (2312 bytes, dense per-record scoring loop).
int __cdecl propose_proto(int faction_id, int chassis_id, int weapon_id,
                          int armor_id, int ability_flags, int reactor_id,
                          int design_category, char *requested_name) {
    int offense_factor;
    int defense_factor;
    int best_prototype = -1;
    int best_score = 0;

    if (DesignConsiderOverride) {
        if (DesignConsiderPrototype >= 0 && DesignConsiderPrototype != design_category) {
            return -1;
        }
    } else if (is_bunged(faction_id, chassis_id, weapon_id, armor_id,
                         ability_flags, reactor_id)) {
        return -1;
    }

    if (Chassis[chassis_id].speed == 1
        && Armor[armor_id].defense_rating == 1
        && Weapon[weapon_id].offense_rating == 1
        && !DesignConsiderOverride) {
        return 2;
    }

    if (!ExpansionEnabled && weapon_id == 12) {
        offense_factor = 1;
    } else if (!ExpansionEnabled && weapon_id == 13) {
        offense_factor = 1;
    } else if (!ExpansionEnabled && weapon_id == 14) {
        offense_factor = 1;
    } else if (Weapon[weapon_id].offense_rating < 0) {
        offense_factor = psi_factor(PlayersData[faction_id].enemy_best_armor_value
            * (int)Rules.psi_combat_ratio_atk[TRIAD_LAND]
            / (int)Rules.psi_combat_ratio_def[TRIAD_LAND], faction_id, TRUE, FALSE);
    } else {
        offense_factor = Weapon[weapon_id].offense_rating;
    }
    if (!ExpansionEnabled && armor_id > 9) {
        defense_factor = 1;
    } else if (Armor[armor_id].defense_rating < 0) {
        defense_factor = psi_factor(PlayersData[faction_id].enemy_best_weapon_value
            * (int)Rules.psi_combat_ratio_def[TRIAD_LAND]
            / (int)Rules.psi_combat_ratio_atk[TRIAD_LAND], faction_id, FALSE, FALSE);
    } else {
        defense_factor = Armor[armor_id].defense_rating;
    }

    for (int walk = 0; walk < 2 * MaxVehProtoFactionNum; walk++) {
        int slot = walk % MaxVehProtoFactionNum
            + ((walk >= MaxVehProtoFactionNum) ? faction_id * MaxVehProtoFactionNum : 0);
        int contribution;
        if (slot >= MaxVehProtoFactionNum
            && VehPrototypes[slot].flags & PROTO_ACTIVE
            && VehPrototypes[slot].obsolete_factions & (1 << faction_id)
            && !PlayersData[faction_id].proto_id_active[slot]
            && !PlayersData[faction_id].proto_id_queue[slot]) {
            contribution = (design_category == (uint8_t)VehPrototypes[slot].unk_1)
                ? 0x2000 : 0x1000;
            goto score;
        }
        if (!(VehPrototypes[slot].flags & PROTO_ACTIVE)) {
            if (slot < MaxVehProtoFactionNum) {
                continue;
            }
            contribution = 0x800;
            goto score;
        }
        if (slot < MaxVehProtoFactionNum
            && !has_tech(VehPrototypes[slot].preq_tech, faction_id)) {
            continue;
        }
        if (VehPrototypes[slot].chassis_id == chassis_id
            && VehPrototypes[slot].weapon_id == weapon_id
            && VehPrototypes[slot].armor_id == armor_id
            && (uint32_t)ability_flags == VehPrototypes[slot].ability_flags
            && VehPrototypes[slot].reactor_id == reactor_id
            && !DesignConsiderOverride
            && GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & (1 << faction_id)) {
            return -1;
        }
        if (VehPrototypes[slot].chassis_id == chassis_id
            && VehPrototypes[slot].weapon_id == weapon_id
            && VehPrototypes[slot].armor_id == armor_id
            && !((VehPrototypes[slot].ability_flags ^ ability_flags)
                & PrototypeAbilityCompatibilityMask)
            && ((VehPrototypes[slot].ability_flags & ability_flags)
                    == VehPrototypes[slot].ability_flags
                || !is_human(faction_id))
            && VehPrototypes[slot].reactor_id == reactor_id) {
            if (design_category == -2) {
                VehPrototypes[slot].flags |= PROTO_TYPED_COMPLETE;
            }
            if (DesignConsiderOverride && DesignConsiderOverride <= 2) {
                DesignConsiderAbilities = ability_flags;
                DesignConsiderWeapon = weapon_id;
                DesignConsiderArmor = armor_id;
                DesignConsiderChassis = chassis_id;
                DesignConsiderReactor = reactor_id;
                DesignConsiderOverride = 2;
            }
            VehPrototypes[slot].obsolete_factions &= ~(1 << faction_id);
            return slot;
        }
        if (slot < MaxVehProtoFactionNum) {
            continue;
        }
        {
            int faction_bit = 1 << faction_id;
            if (VehPrototypes[slot].obsolete_factions & faction_bit
                && !PlayersData[faction_id].proto_id_active[slot]) {
                contribution = 0x400;
                goto score;
            }
            if (VehPrototypes[slot].chassis_id == chassis_id
                && VehPrototypes[slot].reactor_id == reactor_id
                && !((VehPrototypes[slot].ability_flags ^ ability_flags)
                    & PrototypeAbilitySimilarityMask)) {
                int other_weapon = VehPrototypes[slot].weapon_id;
                int other_offense;
                if (!ExpansionEnabled && other_weapon == 12) {
                    other_offense = 1;
                } else if (!ExpansionEnabled && other_weapon == 13) {
                    other_offense = 1;
                } else if (!ExpansionEnabled && other_weapon == 14) {
                    other_offense = 1;
                } else if (Weapon[other_weapon].offense_rating < 0) {
                    other_offense = psi_factor(PlayersData[faction_id].enemy_best_armor_value
                        * (int)Rules.psi_combat_ratio_atk[TRIAD_LAND]
                        / (int)Rules.psi_combat_ratio_def[TRIAD_LAND], faction_id, TRUE, FALSE);
                } else {
                    other_offense = Weapon[other_weapon].offense_rating;
                }
                int other_armor = VehPrototypes[slot].armor_id;
                int other_defense;
                if (!ExpansionEnabled && other_armor > 9) {
                    other_defense = 1;
                } else if (Armor[other_armor].defense_rating < 0) {
                    other_defense = psi_factor(PlayersData[faction_id].enemy_best_weapon_value
                        * (int)Rules.psi_combat_ratio_def[TRIAD_LAND]
                        / (int)Rules.psi_combat_ratio_atk[TRIAD_LAND], faction_id, FALSE, FALSE);
                } else {
                    other_defense = Armor[other_armor].defense_rating;
                }
                int highest_plus = (other_offense > other_defense)
                    ? other_offense : other_defense;
                int highest = highest_plus;
                if (other_offense == other_defense) {
                    highest_plus++;
                }
                if (offense_factor < highest
                    && ((offense_factor - other_offense >= 0)
                        ? offense_factor - other_offense : other_offense - offense_factor) < 2
                    && defense_factor < highest_plus
                    && ((defense_factor - other_defense >= 0)
                        ? defense_factor - other_defense : other_defense - defense_factor) < 2
                    && !(faction_bit & GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_)
                    && strcmp(Players[faction_id].filename, "UNIV")) {
                    return slot;
                }
            }
            if ((uint8_t)VehPrototypes[slot].unk_1 != design_category
                && (PlayersData[faction_id].proto_id_active[slot]
                    || PlayersData[faction_id].proto_id_queue[slot])) {
                continue;
            }
            contribution = 0x40;
            if (!PlayersData[faction_id].proto_id_active[slot]) {
                contribution = 0x240;
            }
            if (VehPrototypes[slot].obsolete_factions & faction_bit) {
                contribution |= 0x100;
            }
            if (!PlayersData[faction_id].proto_id_queue[slot]) {
                contribution |= 0x80;
            }
            contribution -= range(VehPrototypes[slot].cost
                * PlayersData[faction_id].proto_id_active[slot]
                + PlayersData[faction_id].proto_id_queue[slot], 0, 0x3F);
        }
    score:
        if (contribution > best_score) {
            best_score = contribution;
            best_prototype = slot;
        }
    }

    if (best_prototype < 0) {
        return -1;
    }
    if (DesignConsiderOverride) {
        if (DesignConsiderOverride <= 2) {
            DesignConsiderWeapon = weapon_id;
            DesignConsiderArmor = armor_id;
            DesignConsiderChassis = chassis_id;
            DesignConsiderAbilities = ability_flags;
            DesignConsiderReactor = reactor_id;
            DesignConsiderPrototype = design_category;
            DesignConsiderOverride = 3;
        }
        return -1;
    }

    int was_inactive = !(VehPrototypes[best_prototype].flags & PROTO_ACTIVE);
    if (VehPrototypes[best_prototype].flags & PROTO_ACTIVE && !IsMultiplayerNet) {
        retire_proto(best_prototype, faction_id);
    }
    make_proto(best_prototype, chassis_id, weapon_id, armor_id, ability_flags, reactor_id);
    if (design_category == -2) {
        VehPrototypes[best_prototype].flags |= PROTO_TYPED_COMPLETE;
    }
    if (GameDataStateStorage.fields_.diff_level_current_ <= 1) {
        VehPrototypes[best_prototype].flags |= PROTO_TYPED_COMPLETE;
    }
    if (was_inactive) {
        VehPrototypes[best_prototype].flags |= 0x40;
    }
    VehPrototypes[best_prototype].flags |= 0x80;
    if (design_category >= 0) {
        VehPrototypes[best_prototype].unk_1 = (int8_t)design_category;
        if (design_category == 10) {
            VehPrototypes[best_prototype].plan = PLAN_RECONNAISANCE;
        }
    }

    if (requested_name) {
        strcpy(VehPrototypes[best_prototype].veh_name, requested_name);
        for (int other = 0; other < MaxVehProtoNum; other++) {
            if (other != best_prototype) {
                int other_faction = other / MaxVehProtoFactionNum;
                if ((!other_faction || other_faction == faction_id)
                    && !strcmp(VehPrototypes[best_prototype].veh_name,
                        VehPrototypes[other].veh_name)) {
                    strcat(VehPrototypes[best_prototype].veh_name, "*");
                    other = -1;
                }
            }
        }
    } else {
        char generated_name[256];
        name_proto(generated_name, best_prototype, faction_id, chassis_id,
                   weapon_id, armor_id, ability_flags, reactor_id);
        strncpy(VehPrototypes[best_prototype].veh_name, generated_name, 32);
        VehPrototypes[best_prototype].veh_name[31] = '\0';
        VehPrototypes[best_prototype].flags &= ~PROTO_CUSTOM_NAME_SET;
    }

    prune_protos(faction_id, best_prototype, 0);
    for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; base_id++) {
        if (Bases[base_id].faction_id_current == faction_id) {
            // The first scan's result is unused; the second rewrites every match.
            int probe;
            for (probe = 0; probe <= Bases[base_id].queue_size; probe++) {
                int queued = Bases[base_id].queue_production_id[probe];
                if (queued >= 0 && VehPrototypes[queued].obsolete_factions & (1 << faction_id)
                    && VehPrototypes[queued].unk_1 == VehPrototypes[best_prototype].unk_1) {
                    break;
                }
            }
            for (int queue_slot = 0; queue_slot <= Bases[base_id].queue_size; queue_slot++) {
                int queued = Bases[base_id].queue_production_id[queue_slot];
                if (queued >= 0 && VehPrototypes[queued].obsolete_factions & (1 << faction_id)
                    && VehPrototypes[queued].unk_1 == VehPrototypes[best_prototype].unk_1) {
                    Bases[base_id].queue_production_id[queue_slot] = best_prototype;
                }
            }
        }
    }
    if (FirstProposedPrototype < 0) {
        FirstProposedPrototype = best_prototype;
    }
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
