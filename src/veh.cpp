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

// The automatic-design editor exchanges these fields with consider_designs
// (0x00581260). setup_veh writes them, enables the override, then reloads
// the resulting component choices. Their shared consumer remains unfinished.
// RECOVERY_OBJECT: v2 0x0093FC34 0x0093FC38 ?DesignConsiderAbilities@@3IA 0x0043E490
uint32_t DesignConsiderAbilities;
// RECOVERY_OBJECT: v2 0x0093FC38 0x0093FC3C ?DesignConsiderPrototype@@3HA 0x0043E490
int DesignConsiderPrototype;
// RECOVERY_OBJECT: v2 0x0093FC3C 0x0093FC40 ?DesignConsiderWeapon@@3HA 0x0043E490
int DesignConsiderWeapon;
// RECOVERY_OBJECT: v2 0x0093FC40 0x0093FC44 ?DesignConsiderChassis@@3HA 0x0043E490
int DesignConsiderChassis;
// propose_proto exchanges its sixth (reactor) argument with the editor at
// 0x580F07; both original writes use this independent four-byte scalar.
// RECOVERY_OBJECT: v2 0x0093FC44 0x0093FC48 ?DesignConsiderReactor@@3HA 0x00580860
int DesignConsiderReactor;
// Workshop583FA2 publishes its faction here; preview583DB5/583DC1 reloads
// it around veh_fake. This is distinct from the preceding reactor scalar.
// RECOVERY_OBJECT: v2 0x0093FC48 0x0093FC4C ?WorkshopFaction@@3HA 0x00583E30
int WorkshopFaction;
// RECOVERY_OBJECT: v2 0x0093FC50 0x0093FC54 ?DesignConsiderArmor@@3HA 0x0043E490
int DesignConsiderArmor;
// RECOVERY_OBJECT: v2 0x0093FC54 0x0093FC58 ?DesignConsiderOverride@@3HA 0x0043E490
int DesignConsiderOverride;

// Native580B51/580BF7 test immediate ability masks, not executable globals.
static const uint32_t PrototypeAbilityCompatibilityMask = 0x0044AE1Bu;
static const uint32_t PrototypeAbilitySimilarityMask = 0x0048841Au;

// consider_designs resets this scalar to -1; propose_proto records the
// first selected prototype only while the scalar remains negative.
// RECOVERY_OBJECT: v2 0x0068F0F0 0x0068F0F4 ?FirstProposedPrototype@@3HA 0x00580860
int FirstProposedPrototype = -1;
// Preview583CD7 and workshop583F9A read this signed zoom, initially -6.
// Its four-byte initializer is independently backed by the pinned image;
// the preceding FirstProposedPrototype ends at this address, exclusively.
// RECOVERY_OBJECT: v2 0x0068F0F4 0x0068F0F8 ?WorkshopPreviewZoom@@3HA 0x00583CD0
int WorkshopPreviewZoom = -6;

// Compose a readable prototype name from the already-loaded design rules.
// Rule-string fields retain pointer-bit values validated by the loaded rules.
static const char *prototype_rule_name(LPSTR string_id) {
    return StringTable.get(string_id);
}

// Bump a trailing digit of the last duplicate suffix, or append " <suffix>2".
static inline void name_proto_suffix(char *name, int &exhausted) {
    char *found = 0;
    do {
        found = strstr(found ? found + 1 : name, label_get(0xE5C / 4));
    } while (found && strstr(found + 1, label_get(0xE5C / 4)));
    if (found && strlen(found) <= strlen(label_get(0xE5C / 4)) + 1) {
        char *last = &name[strlen(name) - 1];
        if (*last >= '2' && *last < '9') {
            (*last)++;
        } else {
            exhausted = 1;
        }
    } else {
        char number[80];
        strcat(name, " ");
        strcat(name, label_get(0xE5C / 4));
        _itoa(2, number, 10);
        strcat(name, number);
    }
}

// ORIGINAL: 0x0057E040 ?name_proto@@YAXPADHHHHHHH@Z 0x0057E040-0x0057EF95
// RECOVERY_BINDING: v1 {"evidence": ["git:2f89503ec251ffc8b9ff894998c01944c173a4fb:src/unrecovered/0057e040.cpp;image:0x0057E040-0x0057EF95", "pinned image:0x0057EE10 or esi,-1;0x0057EE13 test edi,edi;0x0057EE1B..0x0057EE35 builds VehPrototypes[-1].veh_name and calls strcmp", "pinned image:0x0057EF67..0x0057EF71 resets ESI after a matched alternate suffix and increments/restarts at zero", "Owned accidental storage:Ability base0x009AB538,stride0x1C,abbreviation+8 =>0x009AB834 = VehPrototypes base0x009AB868-0x34;source:src/veh.h RulesAbility;source:src/alpha.cpp ability loader", "git:2f89503ec251ffc8b9ff894998c01944c173a4fb:src/unrecovered/0057e040.cpp"], "reason": "Complete the original duplicate-name candidate loop, including its unconditional primary-collision restart before the differing-form alternate compare and its alternate-collision restart. The -1 candidate name view points to the source-owned object representation of Ability[27].abbreviation, preserving the native pointer-field-byte comparison instead of resolving the pointed-to rule string. The original unbounded strcmp read extent beyond that member remains unresolved; preserve its unsafe read/overflow behavior as an obligation. Full span and prior source/history remain intact; no byte-exact or runtime acceptance is claimed."}

// Rewritten from the image 2026-11-08 (0.94 mnemonic); the image keeps output in ebx for the
// whole function and VC6 does not here.
void __cdecl name_proto(char *output, int prototype_id, int faction_id,
                        int chassis_id, int weapon_id, int armor_id,
                        int ability_flags, int reactor_id) {
    *output = '\0';
    for (int ability_id = 0; ability_id < MaxAbilityNum; ability_id++) {
        if (ability_flags & (1 << ability_id)
            && strlen(StringTable.get(Ability[ability_id].abbreviation))
            && (ability_id != abil_index(ABL_AIR_SUPERIORITY)
                || Chassis[chassis_id].triad != TRIAD_AIR)
            && ability_id != abil_index(ABL_ARTILLERY)
            && (ability_id != abil_index(ABL_AMPHIBIOUS)
                || Chassis[chassis_id].speed != 1
                || Weapon[weapon_id].offense_rating < Armor[armor_id].defense_rating
                || ability_flags & ABL_ARTILLERY)) {
            strcat(output, StringTable.get(Ability[ability_id].abbreviation));
            strcat(output, " ");
            break;
        }
    }
    char alternate[256];
    strcpy(alternate, output);
    int quality = 0;
    int not_sea;
    if (Weapon[weapon_id].mode < 3) {
        int combat;
        int limit;
        if (Armor[armor_id].defense_rating <= Weapon[weapon_id].offense_rating) {
            combat = 3;
            limit = Armor[armor_id].defense_rating;
        } else {
            combat = 6;
            limit = Weapon[weapon_id].offense_rating;
        }
        if (Chassis[chassis_id].triad == TRIAD_SEA) {
            combat++;
        }
        if (Chassis[chassis_id].triad == TRIAD_AIR) {
            combat += combat / 2;
        }
        if (limit > combat) {
            quality = 1;
        }
        not_sea = Chassis[chassis_id].triad != TRIAD_SEA;
        if (!strlen(StringTable.get(Chassis[chassis_id].offsv_lrg_name))
            || !strlen(StringTable.get(Chassis[chassis_id].defsv_lrg_name))) {
            quality = 0;
        }
    }
    int gendered = (reactor_id - 1 ^ faction_id) & 1;
    int unique = 0;
    if ((VehPrototypes[prototype_id].unk_1 == 10 && !is_human(faction_id)
         && Chassis[chassis_id].speed > 1)
        || (Chassis[chassis_id].speed >= Weapon[weapon_id].offense_rating
            && Chassis[chassis_id].speed >= Armor[armor_id].defense_rating
            && Weapon[weapon_id].offense_rating >= 0
            && Chassis[chassis_id].triad == TRIAD_LAND)) {
        if (Weapon[weapon_id].mode < 3) {
            if (has_abil(prototype_id, ABL_POLICE_2X)) {
                if (strlen(output)) {
                    output[strlen(output) - 1] = '\0';
                }
                if (strlen(alternate)) {
                    alternate[strlen(output) - 1] = '\0';
                }
            } else if (Weapon[weapon_id].offense_rating >= 2) {
                strcat(output, label_get(0x2D8 / 4));
                strcat(alternate, label_get(0x2D8 / 4));
            } else {
                strcat(output, label_get(0x248 / 4));
                strcat(alternate, label_get(0x248 / 4));
            }
            unique = 1;
            gendered = Chassis[chassis_id].speed > 1;
        }
    }
    int lower = 0;
    if (!unique) {
        for (int walk = 0; walk < 2 * MaxVehProtoFactionNum; walk++) {
            const int id = walk % MaxVehProtoFactionNum
                + (walk < MaxVehProtoFactionNum ? 0 : faction_id * MaxVehProtoFactionNum);
            if (VehPrototypes[id].flags & PROTO_ACTIVE
                && VehPrototypes[id].plan == VehPrototypes[prototype_id].plan
                && VehPrototypes[id].chassis_id == VehPrototypes[prototype_id].chassis_id) {
                if (VehPrototypes[id].reactor_id >= VehPrototypes[prototype_id].reactor_id) {
                    lower = 0;
                    break;
                }
                lower = 1;
            }
        }
    }
    if (Armor[armor_id].defense_rating > Weapon[weapon_id].offense_rating
        && Weapon[weapon_id].mode < 3 && Weapon[weapon_id].offense_rating >= 0
        && !(ability_flags & ABL_ARTILLERY)) {
        if (!unique) {
            gendered ^= Armor[armor_id].defense_rating & 1;
            if (lower) {
                strcat(output, StringTable.get(Reactor[reactor_id - 1].name_short));
                strcat(alternate, StringTable.get(Reactor[reactor_id - 1].name_short));
            } else {
                strcat(output, StringTable.get(Armor[armor_id].name_short));
                strcat(alternate, StringTable.get(Armor[armor_id].name_short));
            }
        }
        strcat(output, " ");
        strcat(alternate, " ");
        strcat(output, StringTable.get(Chassis[chassis_id].defsv_1_name));
        if (unique && prototype_id & 1) {
            strcat(alternate, StringTable.get(quality && not_sea
                ? Chassis[chassis_id].defsv_lrg_name : Chassis[chassis_id].defsv_1_name));
        } else {
            strcat(alternate, StringTable.get(quality && !not_sea
                ? Chassis[chassis_id].defsv_lrg_name : Chassis[chassis_id].defsv_2_name));
        }
    } else if (Chassis[chassis_id].triad == TRIAD_AIR && ability_flags & ABL_AIR_SUPERIORITY
               && Weapon[weapon_id].mode < 3 && Weapon[weapon_id].offense_rating >= 0) {
        if (!unique) {
            if (lower) {
                strcat(output, StringTable.get(Reactor[reactor_id - 1].name_short));
                strcat(alternate, StringTable.get(Reactor[reactor_id - 1].name_short));
            } else {
                strcat(output, StringTable.get(Weapon[weapon_id].name_short));
                strcat(alternate, StringTable.get(Weapon[weapon_id].name_short));
            }
        }
        strcat(output, " ");
        strcat(alternate, " ");
        strcat(output, StringTable.get(Chassis[chassis_id].defsv_1_name));
        if (unique && prototype_id & 1) {
            strcat(alternate, StringTable.get(quality && not_sea
                ? Chassis[chassis_id].defsv_lrg_name : Chassis[chassis_id].defsv_1_name));
        } else {
            strcat(alternate, StringTable.get(quality && !not_sea
                ? Chassis[chassis_id].defsv_lrg_name : Chassis[chassis_id].defsv_2_name));
        }
    } else if (Chassis[chassis_id].missile && Weapon[weapon_id].offense_rating >= 99) {
        strcat(output, StringTable.get(Reactor[reactor_id - 1].name_short));
        strcat(alternate, StringTable.get(Reactor[reactor_id - 1].name_short));
        strcat(output, " ");
        strcat(alternate, " ");
        strcat(output, StringTable.get(Weapon[weapon_id].name_short));
        strcat(alternate, StringTable.get(Weapon[weapon_id].name_short));
    } else if (Chassis[chassis_id].missile && (weapon_id == 24 || weapon_id == 25)) {
        strcat(output, StringTable.get(Weapon[weapon_id].name_short));
        strcat(alternate, StringTable.get(Weapon[weapon_id].name_short));
        strcat(output, " ");
        strcat(alternate, " ");
        strcat(output, StringTable.get(Reactor[reactor_id - 1].name_short));
        strcat(alternate, StringTable.get(Reactor[reactor_id - 1].name_short));
        strcat(output, " ");
        strcat(alternate, " ");
        strcat(output, StringTable.get(Chassis[chassis_id].offsv_1_name));
        strcat(alternate, StringTable.get(Chassis[chassis_id].offsv_1_name));
    } else {
        if (Weapon[weapon_id].mode >= 3 && chassis_id) {
            if (reactor_id > 1) {
                strcat(output, StringTable.get(Reactor[reactor_id - 1].name_short));
                strcat(alternate, StringTable.get(Reactor[reactor_id - 1].name_short));
                strcat(output, " ");
                strcat(alternate, " ");
            }
            if ((Weapon[weapon_id].mode != 7 || chassis_id != 3)
                && (Weapon[weapon_id].mode != 9 || Chassis[chassis_id].triad != TRIAD_LAND
                    || !strchr(output, ' '))) {
                if ((Weapon[weapon_id].mode == 8 || Weapon[weapon_id].mode == 9)
                    && chassis_id == 3) {
                    strcat(output, label_get(0x28C / 4));
                    strcat(alternate, label_get(0x28C / 4));
                } else {
                    strcat(output, StringTable.get(Chassis[chassis_id].offsv_1_name));
                    strcat(alternate, StringTable.get(Chassis[chassis_id].offsv_2_name));
                }
                strcat(output, " ");
                strcat(alternate, " ");
            }
        }
        if (!unique) {
            gendered ^= Weapon[weapon_id].offense_rating & 1;
            strcat(output, StringTable.get(Weapon[weapon_id].name_short));
            strcat(alternate, StringTable.get(Weapon[weapon_id].name_short));
        }
        if (Weapon[weapon_id].mode < 3) {
            if (ability_flags & ABL_ARTILLERY) {
                strcat(output, " ");
                strcat(output, label_get(0x87C / 4));
                strcat(alternate, " ");
                strcat(alternate, label_get(0x880 / 4));
            } else if (ability_flags & ABL_AMPHIBIOUS && Chassis[chassis_id].speed == 1
                       && Weapon[weapon_id].offense_rating > Armor[armor_id].defense_rating) {
                strcat(output, " ");
                strcat(output, label_get(0x998 / 4));
                strcat(alternate, " ");
                strcat(alternate, label_get(0x99C / 4));
            } else {
                strcat(output, " ");
                if (lower) {
                    strcat(output, StringTable.get(Reactor[reactor_id - 1].name_short));
                } else {
                    strcat(output, StringTable.get(quality && not_sea
                        ? Chassis[chassis_id].offsv_lrg_name : Chassis[chassis_id].offsv_1_name));
                }
                strcat(alternate, " ");
                if (lower) {
                    strcat(alternate, StringTable.get(Reactor[reactor_id - 1].name_short));
                } else {
                    strcat(alternate, StringTable.get(quality && !not_sea
                        ? Chassis[chassis_id].offsv_lrg_name : Chassis[chassis_id].offsv_2_name));
                }
            }
        }
    }
    const int same = !strcmp(output, alternate);
    int exhausted = 0;
    for (int proto_id = 0; proto_id < MaxVehProtoNum; proto_id++) {
        if (proto_id == prototype_id) continue;
        const int proto_faction = proto_id / MaxVehProtoFactionNum;
        if (proto_faction && proto_faction != faction_id) continue;
        if (VehPrototypes[proto_id].chassis_id == chassis_id
            && VehPrototypes[proto_id].weapon_id == weapon_id
            && VehPrototypes[proto_id].armor_id == armor_id
            && VehPrototypes[proto_id].reactor_id == reactor_id
            && VehPrototypes[proto_id].ability_flags == (uint32_t)ability_flags) {
            continue;
        }
        if (!exhausted && !strcmp(output, VehPrototypes[proto_id].veh_name)) {
            if (!same) {
                gendered = 1;
            }
            name_proto_suffix(output, exhausted);
            proto_id = -1;
        }
        // BUG(original): after a restart proto_id is -1 here, so this reads
        // the name slot just before VehPrototypes[0].
        if (!same && !strcmp(alternate, VehPrototypes[proto_id].veh_name)) {
            gendered = 0;
            name_proto_suffix(alternate, exhausted);
            proto_id = -1;
        }
    }
    if (gendered) {
        strcpy(output, alternate);
    }
}

// 0x009A64C8
// RECOVERY_OBJECT: v1 0x009B2280 ?VehDropLiftVehID@@3HA
int VehDropLiftVehID;  // 0x009B2280
// Extent: compiled sizeof 0x4; next mapped object at 0x009B227C.
// RECOVERY_OBJECT: v2 0x009B2278 0x009B227C ?VehLiftX@@3HA 0x004BF928
int VehLiftX;  // 0x009B2278
// Extent: compiled sizeof 0x4; 0x9B2288 unreferenced; VehBitError is 0x9B228C.
// RECOVERY_OBJECT: v2 0x009B2284 0x009B2288 ?VehLiftY@@3HA 0x004BF932
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
Purpose: Read the specified header from a file. This assumes the header string buffer is at least 
         256 characters. TODO: Replace built-in versions of _fgetc and change return to std::string.
// ORIGINAL: 0x0057D1F0 ?header_check@@YAXPADPAUFILE@@@Z 0x0057D1F0-0x0057D235 BYTE_EXACT
// LEVER: `header_chr` as `char` (not `int`) so the compare/store stay byte
//        width; `int i` declared and zeroed BEFORE the first `fgetc` call,
//        not inside the guard block; and the do-while-with-break rewritten
//        as a plain `while (header_chr) { if (++i >= 256) break; ... }` -
//        the do-while shape let VC6 fold the loop counter's first
//        increment into its initial value and rotate the loop, which the
//        image's own top-tested-counter / bottom-tested-char shape never
//        does.
// symbol    ?header_check@@YAXPADPAU_iobuf@@@Z
// size      69 bytes
// prototype void (__cdecl ?header_check@@YAXPADPAUFILE@@@Z)(int8* header, FILE* file)
// callers   2   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00646855
Return Value: n/a
Status: Complete
*/
void __cdecl header_check(LPSTR header, FILE *file) {
    int i = 0;
    char header_chr = (char)fgetc(file);
    *header++ = header_chr;
    while (header_chr) {
        if (++i >= 256) {
            break;
        }
        header_chr = (char)fgetc(file);
        *header++ = header_chr;
    }
    fgetc(file);
}

/*
Purpose: Write the specified header to a file. TODO: Replace built-in versions of _fputc.
// ORIGINAL: 0x0057D240 ?header_write@@YAXPADPAUFILE@@@Z 0x0057D240-0x0057D270 BYTE_EXACT
// LEVER: `header_chr` as `char`, not `int` - the image keeps the byte in
//        `bl` for the loop's `test bl,bl` and only sign-extends it
//        (`movsx eax, bl`) at the `fputc` push, which is the local matching
//        the width it reads from.
// symbol    ?header_write@@YAXPBDPAU_iobuf@@@Z
// size      48 bytes
// prototype 
// callers   2   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00646886
Return Value: n/a
Status: Complete
*/
void __cdecl header_write(LPCSTR header, FILE *file) {
    char header_chr;
    do {
        header_chr = *header++;
        fputc(header_chr, file);
    } while (header_chr);
    fputc(0x1A, file);
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
        return psi_factor((Rules.psi_combat_ratio_def[TRIAD_LAND]
            * PlayersData[faction_id].enemy_best_weapon_value)
            / Rules.psi_combat_ratio_atk[TRIAD_LAND], faction_id, false, false);
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
        return psi_factor((Rules.psi_combat_ratio_atk[TRIAD_LAND]
            * PlayersData[faction_id].enemy_best_armor_value)
            / Rules.psi_combat_ratio_def[TRIAD_LAND], faction_id, true, false);
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
    return weap_strat(VehPrototypes[proto_id].weapon_id, faction_id);
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
// Superseded: this was one TU with veh.cpp, so the direct call inlines.
Return Value: Armor value
Status: Complete
*/
int __cdecl arm_val(int armor_id, int faction_id) {
    // The separate `*= 2` keeps prune_protos' inlined doubling at the merge.
    int defense;
    if (faction_id >= 0) {
        defense = arm_strat(armor_id, faction_id);
    } else {
        defense = Armor[armor_id].defense_rating;
    }
    defense *= 2;
    return defense;
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
// Superseded: this was one TU with veh.cpp, so the direct call inlines.
Return Value: Armor value
Status: Complete
*/
int __cdecl armor_val(int proto_id, int faction_id) {
    return arm_val(VehPrototypes[proto_id].armor_id, faction_id);
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

// Writable formatter strings at distinct pinned operand addresses.
// These names describe their uses; original allocation bounds remain unknown.
// RECOVERY_OBJECT: v1 0x0068F118 _DefenseAAAOpenText
static char DefenseAAAOpenText[] = "<";
// RECOVERY_OBJECT: v1 0x0068F11C _DefenseCommJammerText
static char DefenseCommJammerText[] = "+";
// RECOVERY_OBJECT: v1 0x0068F120 _DefenseTranceText
static char DefenseTranceText[] = "t";
// RECOVERY_OBJECT: v1 0x0068F124 _DefenseAAACloseText
static char DefenseAAACloseText[] = ">";
// RECOVERY_OBJECT: v1 0x0068F128 _DefensePulseText
static char DefensePulseText[] = "p";
// RECOVERY_OBJECT: v1 0x0068F12C _DefenseResonanceText
static char DefenseResonanceText[] = "r";
// RECOVERY_OBJECT: v1 0x0068F130 _Stats3OffenseUnknownText
static char Stats3OffenseUnknownText[] = "?";
// RECOVERY_OBJECT: v1 0x0068F134 _Stats3OffenseExtremeText
static char Stats3OffenseExtremeText[] = "*";
// RECOVERY_OBJECT: v1 0x0068F138 _Stats3OffenseDefenseSeparatorText
static char Stats3OffenseDefenseSeparatorText[] = "-";
// RECOVERY_OBJECT: v1 0x0068F13C _Stats3DefenseUnknownText
static char Stats3DefenseUnknownText[] = "?";
// RECOVERY_OBJECT: v1 0x0068F140 _Stats3DefenseSpeedSeparatorText
static char Stats3DefenseSpeedSeparatorText[] = "-";
// RECOVERY_OBJECT: v1 0x0068F144 _Stats3ReactorPrefixText
static char Stats3ReactorPrefixText[] = "*";
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
        strcat(stat, DefenseAAAOpenText);
    }
    int def_rating = armor_proto(proto_id, -1, 0);
    _itoa(def_rating, num_buf, 10);
    strcat(stat, num_buf);
    if (has_abil(proto_id, ABL_COMM_JAMMER)) {
        strcat(stat, DefenseCommJammerText);
    }
    if (has_abil(proto_id, ABL_TRANCE)) {
        strcat(stat, DefenseTranceText);
    }
    if (has_abil(proto_id, ABL_AAA)) {
        strcat(stat, DefenseAAACloseText);
    }
    if (VehPrototypes[proto_id].armor_id == ARM_PULSE_3_ARMOR
        || VehPrototypes[proto_id].armor_id == ARM_PULSE_8_ARMOR) {
        strcat(stat, DefensePulseText);
    }
    if (VehPrototypes[proto_id].armor_id == ARM_RESONANCE_3_ARMOR
        || VehPrototypes[proto_id].armor_id == ARM_RESONANCE_8_ARMOR) {
        strcat(stat, DefenseResonanceText);
    }
}
/*
Purpose: Generate stats string for specified prototype. Replaced existing non-safe strcat with
         string. Reworked to integrate with existing C code.
// ORIGINAL: 0x0057D7D0 ?say_stats_3@@YAXPADH@Z 0x0057D7D0-0x0057D8D3 BYTE_EXACT
// TRIED: else-if branch order off_rating<0/off_rating<99/else folded the "?"
//            and "*" strcat tails together (29/92); reordering to
//            off_rating<0/off_rating>=99/else (matching the disasm's own branch
//            order) got to 51/92. def_rating stored to a named int8_t local, or
//            inlined as Armor[...].armor_id].defense_rating, made no further
//            difference; plateaus on the def_rating address computation
//            (shl 4 + disp vs add const + shl 4), 4 spellings tried.
// LEVER 2026-09-03: the speed division is SIGNED - `speed_proto(proto_id) /
//            Rules.move_rate_roads` gives the image's `cdq / idiv`,
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
        strcat(stat, Stats3OffenseUnknownText); // PSI
    } else if (off_rating >= 99) {
        strcat(stat, Stats3OffenseExtremeText);
    } else {
        say_offense(stat, proto_id);
    }
    strcat(stat, Stats3OffenseDefenseSeparatorText);
    if (get_proto_defense_rating(proto_id) < 0) {
        strcat(stat, Stats3DefenseUnknownText);
    } else {
        say_defense(stat, proto_id);
    }
    strcat(stat, Stats3DefenseSpeedSeparatorText);
    int speed = speed_proto(proto_id) / Rules.move_rate_roads;
    _itoa(speed, num_buf, 10);
    strcat(stat, num_buf);
    if (VehPrototypes[proto_id].reactor_id > RECT_FISSION) {
        strcat(stat, Stats3ReactorPrefixText);
        _itoa(VehPrototypes[proto_id].reactor_id, num_buf, 10);
        strcat(stat, num_buf);
    }
}
/*
Purpose: Generate stats string for specified prototype. List whether prototype is psi, sea or air.
         Replaced existing non-safe strcat with string. Reworked to integrate with existing C code.
// ORIGINAL: 0x0057D8E0 ?say_stats_2@@YAXPADH@Z 0x0057D8E0-0x0057DA94 BYTE_EXACT
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
        strcat(stat, label_get(196)); // 'Psi'
    } else if (off_rating >= 99) {
        strcat(stat, "*");
    } else {
        say_offense(stat, proto_id);
    }
    strcat(stat, "-");
    if (get_proto_defense_rating(proto_id) < 0) {
        strcat(stat, label_get(196)); // 'Psi'
    } else {
        say_defense(stat, proto_id);
    }
    strcat(stat, "-");
    int speed = speed_proto(proto_id) / Rules.move_rate_roads;
    _itoa(speed, num_buf, 10);
    strcat(stat, num_buf);
    if (get_proto_triad(proto_id) == TRIAD_SEA) {
        strcat(stat, " ");
        strcat(stat, label_get(163)); // 'Sea'
    }
    if (get_proto_triad(proto_id) == TRIAD_AIR) {
        strcat(stat, " ");
        strcat(stat, label_get(162)); // 'Air'
    }
    if (VehPrototypes[proto_id].reactor_id > 1) {
        strcat(stat, "*");
        _itoa(VehPrototypes[proto_id].reactor_id, num_buf, 10);
        strcat(stat, num_buf);
    }
}
/*
Purpose: Generate verbose stats string for specified prototype. Used by Design Workshop and Military
         Command Nexus. Replaced existing non-safe strcat with string. Reworked to integrate with
         existing C code.
// ORIGINAL: 0x0057DAA0 ?say_stats@@YAXPADHPAD@Z 0x0057DAA0-0x0057DED8
// RECOVERY_BINDING: v1 {"evidence": ["pinned:57DCEA-57DCF0 strcat references682E90 comma-space-NUL", "pinned:57DDF0 triad arms separately append682820 space-NUL before localized Sea/Air labels"], "reason": "Retain the original comma-space separator followed by the separate triad space. The visible doubled space is original formatting, not a new output correction. Complete formatter matching and runtime remain obligations."}
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
    if (VehPrototypes[proto_id].plan == PLAN_RECONNAISANCE && get_proto_triad(proto_id) == TRIAD_LAND && get_proto_offense_rating(proto_id) == 1 && get_proto_defense_rating(proto_id) == 1
        && !VehPrototypes[proto_id].ability_flags) {
        strcat(stat, StringTable.get(PlansFullName[3])); // 'Explore/Defense'
        strcat(stat, ", ");
    } else if (Weapon[VehPrototypes[proto_id].weapon_id].mode < 3) { // Projectile, energy, missile
        strcat(stat, StringTable.get((VehPrototypes[proto_id].plan != PLAN_DEFENSIVE || (get_proto_offense_rating(proto_id) >= 0 && get_proto_offense_rating(proto_id) <= get_proto_defense_rating(proto_id)))
            ? PlansShortName[VehPrototypes[proto_id].plan] : Labels.strings_ptr[312])); // 'Combat'
        strcat(stat, ", ");
    }
    if (get_proto_offense_rating(proto_id) < 0 || Weapon[VehPrototypes[proto_id].weapon_id].mode < 3) {
        if (get_proto_offense_rating(proto_id) < 0) {
            strcat(stat, StringTable.get(Labels.strings_ptr[196])); // 'Psi'
        } else {
            say_offense(stat, proto_id);
        }
        if (custom_spacer) {
            strcat(stat, custom_spacer);
        } else {
            strcat(stat, "/");
        }
        goto append_defense_tail;
    } else if (get_proto_defense_rating(proto_id) != 1 || VehPrototypes[proto_id].ability_flags || (Chassis[VehPrototypes[proto_id].chassis_id].speed != 1
        && (Weapon[VehPrototypes[proto_id].weapon_id].mode != WPN_MODE_TRANSPORT || VehPrototypes[proto_id].chassis_id != CHSI_FOIL))) {
        strcat(stat, StringTable.get(PlansShortName[Weapon[VehPrototypes[proto_id].weapon_id].mode]));
        if (VehPrototypes[proto_id].plan == PLAN_NAVAL_TRANSPORT) {
            strcat(stat, "(");
            _itoa(VehPrototypes[proto_id].carry_capacity, num_buf, 10);
            strcat(stat, num_buf);
            strcat(stat, ")");
        }
        strcat(stat, ", ");
append_defense_tail:
        if (get_proto_defense_rating(proto_id) < 0) {
            strcat(stat, StringTable.get(Labels.strings_ptr[196])); // 'Psi'
        } else {
            say_defense(stat, proto_id);
        }
        strcat(stat, custom_spacer ? custom_spacer : "/");
        int speed = speed_proto(proto_id) / Rules.move_rate_roads;
        _itoa(speed, num_buf, 10);
        strcat(stat, num_buf);
    } else {
        strcat(stat, StringTable.get(PlansFullName[Weapon[VehPrototypes[proto_id].weapon_id].mode]));
        if (VehPrototypes[proto_id].plan == PLAN_NAVAL_TRANSPORT) {
            strcat(stat, "(");
            _itoa(VehPrototypes[proto_id].carry_capacity, num_buf, 10);
            strcat(stat, num_buf);
            strcat(stat, ")");
        }
        if (get_proto_triad(proto_id)) { // sea, air
            strcat(stat, ", "); // Original separator; the later triad space remains.
        }
    }
    if (get_proto_triad(proto_id) == TRIAD_SEA) {
        strcat(stat, " ");
        strcat(stat, StringTable.get(Labels.strings_ptr[163])); // 'Sea'
    }
    if (get_proto_triad(proto_id) == TRIAD_AIR) {
        strcat(stat, " ");
        strcat(stat, StringTable.get(Labels.strings_ptr[162])); // 'Air'
    }
    if (VehPrototypes[proto_id].reactor_id > 1) {
        strcat(stat, " ");
        strcat(stat, "(");
        strcat(stat, "*");
        _itoa(VehPrototypes[proto_id].reactor_id, num_buf, 10);
        strcat(stat, num_buf);
        strcat(stat, ")");
    }
}


/*
Purpose: Fill the eight bytes at a pointer with 0xff.

             push ebp / mov ebp,esp / mov eax,[ebp+8] / mov cl,0xff
             mov dword [eax],0xffffffff / mov byte [eax+4],cl ... [eax+7],cl
             pop ebp / ret

         One argument and a bare `ret`, so __cdecl - the caller cleans, and
         nothing here needs to know whether it passes more. The original
         writes the first four bytes as a dword and the next four one at a
         time; the result is eight 0xff bytes either way.

// HISTORICAL scaffold role: SCAFFOLD Transitional leaf collection, not completed genuine owner recovery; preserve body evidence during owner migration.
// ORIGINAL: 0x0057DEE0 sub_57dee0 0x0057DEE0-0x0057DEFC BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned 0x0057DEE0-0x0057DEFC: cdecl pointer argument; writes exactly the 8-byte range [target,target+8) to 0xFF, plain ret", "Pinned instruction stream is a bounded eight-byte memory write with no receiver, global, or struct-field interpretation.", "Source family: src/byte_utilities.cpp owns a bounded byte-fill utility; no original function name or translation-unit claim is made."], "reason": "Own this complete bounded byte-fill utility. The descriptive API makes no original symbol or translation-unit claim."}
// symbol    ?fill_eight_bytes_ff@@YAXPAX@Z
// historical original marker: 0x0057DEE0 sub_57dee0 0x0057DEE0-0x0057DEFC BYTE_EXACT
// historical scaffold emitted-symbol hint: ?leaf_0057dee0_redirect@@YAXPAX@Z
// size      28 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/

void __cdecl fill_eight_bytes_ff(void *target) {
    const uint32_t fill_word = 0xFFFFFFFFU;
    std::memcpy(target, &fill_word, sizeof(fill_word));
    uint8_t *const bytes = static_cast<uint8_t *>(target);
    const uint8_t fill = 0xFF;
    bytes[4] = fill;
    bytes[5] = fill;
    bytes[6] = fill;
    bytes[7] = fill;
}


// The private prototype-history owner keeps its storage and readers/writers
// together. The original source filename is unknown. The predicate is called
// out of line from propose_proto5808B3, which tests full EAX at5808BB; both
// original predicate exits define full EAX.

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
    if (count == 0) return true;
    unsigned char *owner = reinterpret_cast<unsigned char *>(&BungList);
    const uint32_t native_offset = static_cast<uint32_t>(offset);
    if (packet_to_owner) memmove(owner + native_offset, packet, count);
    else memmove(packet, owner + native_offset, count);
    return true;
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
// The image inlines fill_eight_bytes_ff (0x0057DEE0) here, so the two share a TU.
void __cdecl clear_bunglist(int faction_id) {
    for (int i = 0; i < 64; i++) {
        fill_eight_bytes_ff(&BungList[faction_id][i]);
    }
}

int __cdecl veh_game_data_bunglist(FILE *file, int format_version) {
    if (format_version >= 6) {
        return game_data_transfer_block(file, &BungList[0][0], sizeof(BungList));
    }
    for (int faction = 0; faction < 8; ++faction) clear_bunglist(faction);
    return 1;
}

/*
Purpose: Record a prototype design in the faction's run.
// ORIGINAL: 0x0057DF30 sub_57df30 0x0057DF30-0x0057DFB2 BYTE_EXACT
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
// stand-in: retire_proto and propose_proto call these out of line in the
// image; in one TU /Ob2 would otherwise inline them.
#pragma auto_inline(off)
void __cdecl proto_bung_put(int faction_id, int proto_id) {
    const int slot = PlayersData[faction_id].prototype_history_cursor;
    PlayersData[faction_id].prototype_history_cursor = (slot + 1) % 64;
    BungList[faction_id][slot].ability_flags = VehPrototypes[proto_id].ability_flags;
    BungList[faction_id][slot].chassis_id = VehPrototypes[proto_id].chassis_id;
    BungList[faction_id][slot].weapon_id = VehPrototypes[proto_id].weapon_id;
    BungList[faction_id][slot].armor_id = VehPrototypes[proto_id].armor_id;
    BungList[faction_id][slot].reactor_id = VehPrototypes[proto_id].reactor_id;
}

/*
Purpose: Was this exact prototype design already recorded for this faction?
// ORIGINAL: 0x0057DFC0 ?is_bunged@@YA_NHHHHHH@Z 0x0057DFC0-0x0057E037 BYTE_EXACT
// symbol    ?is_bunged@@YAHHHHHHH@Z
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
int __cdecl is_bunged(int faction_id, int chassis_id, int weapon_id,
                       int armor_id, int ability_flags, int reactor_id) {
    if ((1u << faction_id) & (uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_) {
        int i = 0;
        for (; i < 0x40; ++i) {
            const ProtoBung &entry = BungList[faction_id][i];
            if (entry.chassis_id == chassis_id && entry.weapon_id == weapon_id &&
                entry.armor_id == armor_id &&
                entry.ability_flags == (uint32_t)ability_flags &&
                entry.reactor_id == reactor_id) {
                return true;
            }
        }
    }
    return false;
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
// ORIGINAL: 0x005A5A60 ?proto_cost@@YAHHHHHH@Z 0x005A5A60-0x005A5CE1 BYTE_EXACT
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
// ORIGINAL: 0x005A5D40 ?make_proto@@YAXHHHHHH@Z 0x005A5D40-0x005A6269 BYTE_EXACT
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





// ORIGINAL: 0x0057A660 ?goody_box@@YAHHH@Z 0x0057A660-0x0057CDE6
// RECOVERY_BINDING: v1 {"evidence": ["task:goody-box-owner", "pinned:57A660..57CDE6 whole dispatcher; original two-int ABI, unused second parameter and seed findings preserved.", "pinned:A8D2 live vehicle x/y retry reload; spawned vehicle prototype/chassis triad read after veh_init; separate event9 and cases12..17 radius paths.", "pinned:B4D8..B874 relocation stack_veh/stack_put/render/focus/spot_all order;CC16..CDE6 retry/resource/overflow gates;A9A1 GameRules reload.", "source:existing Veh/PlayerData/Base/Map/technology declarations and bounded shared radius arrays; map-window public draw_tile_fixup APIs own original render calls.", "historical seed:src/unrecovered/0057a660.cpp mapping and parsed dispatch findings retained."], "reason": "Recover the complete original pod outcome dispatcher through source-owned vehicle, faction, base, technology and map state; retain original identity/spans and historical seed findings, without executable calls or raw storage aliases."}
// Rewritten from the image 2026-11-08 (0.86 full-text): the redirect cases continue a real
// dispatch loop, which is what makes VC6 lay their checks out before the case bodies.
int __cdecl goody_box(int veh_id, int type) {
    int retry = 0;
    int x = Vehs[veh_id].x;
    int y = Vehs[veh_id].y;
    const int faction_id = Vehs[veh_id].faction_id;
    const int is_sea = is_ocean(x, y);
    if (do_unity_crash(x, y, faction_id)) {
        return 0;
    }
    const int is_local = faction_id == ConsoleGlobal.faction_id_;
    PlayersData[faction_id].unk_94++;
    if (IsMultiplayerNet) {
        my_srand(MapRandSeed + PlayersData[faction_id].unk_94 * 37);
    }
    if (GameDataStateStorage.fields_.turn_current_num_
            >= PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::RESEARCH] * -5
        && !(GameDataStateStorage.fields_.game_rules_ & 0x4000000)) {
        PlayersData[faction_id].tech_accumulated++;
    }
    int forced = 0;
    int kind;
    const int base_id = base_find(x, y);
    Map *tile = map_loc(x, y);
    const int region = tile->region;
    const uint32_t bits = tile->bit;
    if (bits & 0xC000000
        && (!PlayersData[faction_id].current_num_bases
            || (base_id >= 0 && Bases[base_id].faction_id_current == faction_id))) {
        kind = (bits & 0x4000000) ? 9 : 10;
        forced = 1;
        bit_set(x, y, 0xC000000, 0);
    } else if (BaseFindDist <= 2 && base_id >= 0
               && Bases[base_id].faction_id_current != faction_id) {
        kind = 16;
        retry = 1;
    } else if ((!(bits & 0x20) || (tile->climate & ~0x1F) < 0x40) && !is_sea) {
        int odds = (BaseFindDist > 12 && PlayersData[faction_id].region_total_bases[region])
            ? 20 : 40;
        if ((VehPrototypes[Vehs[veh_id].proto_id].plan == 9
             && (BaseFindDist < 12 || GameDataStateStorage.fields_.turn_current_num_ < 40))
            || VehPrototypes[Vehs[veh_id].proto_id].plan == 8) {
            odds += 10;
        }
        kind = bounded_crt_random(odds);
    } else {
reroll:
        x = Vehs[veh_id].x;
        y = Vehs[veh_id].y;
        kind = rand() % 14;
        if (!retry) {
            retry = 1;
        } else if (retry > 2) {
            return 0;
        } else if (retry == 2) {
            retry = 3;
        }
        if ((!y || y == MapLatitudeBounds - 1) && !(rand() % 3)) {
            kind = 12;
        }
        if (!(rand() % 100)) {
            kind = 18;
        }
    }

    for (;;) {
    if (VehPrototypes[Vehs[veh_id].proto_id].plan == 12 && !(rand() % 3)
        && !(Vehs[veh_id].flags & 0x20)) {
        kind = 9999;
    }
    switch (kind) {
    case 0: {
        const int roll = bounded_crt_random(is_sea + 3);
        const int amount = (GameDataStateStorage.fields_.turn_current_num_ >= (is_sea ? 100 : 50)
            ? 50 : 25) * (roll + 1);
        PlayersData[faction_id].energy_reserves += amount;
        if (is_local) {
            parse_num(0, amount);
            g_FX.play(0x16);
            NetMsgPrimary.pop(is_sea ? "GOODYHYDROGEN" : "GOODYURANIUM", -5000, 0,
                              is_sea ? 0 : "supply_sm.pcx");
            g_FX.fade(0x16);
            ConsoleInfoWin.on_redraw();
        }
        return 0;
    }
    case 1: {
        if (is_sea) goto reroll;
        if (map_loc(x, y)->bit & 0x80) goto reroll;
        for (int i = 0; i < 9; i++) {
            const int tx = wrap_map_x(x + RadiusOffsetX[i]);
            const int ty = y + RadiusOffsetY[i];
            if (on_map(tx, ty) && (map_loc(tx, ty)->bit & 0x80 || is_ocean(tx, ty))) {
                goto reroll;
            }
        }
        if (is_local) {
            g_FX.play(0x26);
            NetMsgPrimary.pop("GOODYRIVER", 5000, 0, "supply_sm.pcx");
        }
        bit_set(x, y, 0x100, 1);
        world_climate();
        draw_map(1);
        return 0;
    }
    case 2:
        if (is_sea || (map_loc(x, y)->climate & ~0x1F) >= 0xA0) {
            for (int i = 0; i < 121; i++) {
                const int tx = wrap_map_x(x + RadiusOffsetX[i]);
                const int ty = y + RadiusOffsetY[i];
                if (on_map(tx, ty)
                    && (!is_sea || (is_ocean(tx, ty)
                                    && map_loc(x, y)->region == map_loc(tx, ty)->region))) {
                    spot_loc(tx, ty, faction_id);
                }
            }
            if (is_local) {
                g_FX.play(0x16);
                NetMsgPrimary.pop(is_sea ? "GOODYSONAR" : "GOODYVIEW", 5000, 0, "supply_sm.pcx");
                g_FX.fade(0x16);
                draw_map(1);
            }
            return 0;
        }
        if (GameDataStateStorage.fields_.turn_current_num_ >= 50) goto reroll;
        kind = 8;
        continue;
    case 3: {
        if (GameDataStateStorage.fields_.game_rules_ & 0x1000000
            && GameDataStateStorage.fields_.diff_level_current_ <= 1) {
            goto reroll;
        }
        if ((map_loc(x, y)->climate & ~0x1F) > 0x80) goto reroll;
        if (PlayersData[faction_id].current_num_bases < 2) goto reroll;
        if (is_sea) goto reroll;
        if (IsMultiplayerNet) goto reroll;
        if (bounded_crt_random(PlayersData[faction_id].unk_96 * 2 + 2)) goto reroll;
        PlayersData[faction_id].unk_96++;
        g_FX.play(8);
        world_raise_alt(x, y);
        world_raise_alt(x, y);
        for (int i = 0; i < 25; i++) {
            const int tx = wrap_map_x(x + RadiusOffsetX[i]);
            const int ty = y + RadiusOffsetY[i];
            if (on_map(tx, ty)) {
                if (map_loc(tx, ty)->bit & 4 && !(rand() % 3)) {
                    bit_set(tx, ty, 4, 0);
                    synch_bit(tx, ty, faction_id);
                }
                if (i < 9) {
                    rocky_set(tx, ty, minerals_at(tx, ty));
                }
            }
        }
        world_climate();
        draw_map(1);
        if (is_local) {
            NetMsgPrimary.pop("GOODYQUAKE", -5000, 0, "quake_sm.pcx");
            if (tut_check(0x800)) {
                TutorialWindow.tut_map("TERRAFORM", x, y, -1, 0, 0, -1, -1);
            }
        }
        return 0;
    }
    case 4: {
        if (!is_sea && GameDataStateStorage.fields_.game_state_ & 0x100000) {
            kind = 7;
            continue;
        }
        if (IsMultiplayerNet) goto reroll;
        if (VehPrototypes[Vehs[veh_id].proto_id].plan == 12) goto reroll;
        if (GameDataStateStorage.fields_.game_rules_ & 0x1000000
            && GameDataStateStorage.fields_.diff_level_current_ <= 1) {
            goto reroll;
        }
        if (GameDataStateStorage.fields_.turn_current_num_ < 25
            || (GameDataStateStorage.fields_.turn_current_num_ < 50
                && GameDataStateStorage.fields_.diff_level_current_ < 2)) {
            kind = 8;
            continue;
        }
        int attempts = 0;
        const int region = map_loc(x, y)->region;
        int radius;
        if (Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad == TRIAD_LAND
            && VehPrototypes[Vehs[veh_id].proto_id].plan != 8) {
            radius = (VehPrototypes[Vehs[veh_id].proto_id].plan != 9 ? 16 : 0) + 8;
        } else {
            if (!(rand() % 2)) goto reroll;
            radius = 8;
        }
        int tx;
        int ty;
        for (;;) {
            const int rx = bounded_crt_random(MapLongitudeBounds);
            tx = rx;
            ty = bounded_crt_random(MapLatitudeBounds);
            if (rx & 1) {
                tx = rx - 1;
            }
            if (ty & 1) {
                tx++;
            }
            if (++attempts >= 1000) break;
            if (is_ocean(tx, ty) == is_sea && map_loc(tx, ty)->region == region
                && anything_at(tx, ty) < 0 && !zoc_any(tx, ty, faction_id)
                && !(PlayersData[faction_id].flags & 0x200)
                && !(map_loc(tx, ty)->visibility & (1 << faction_id))
                && patrol_dist(x, y, tx, ty) > 4 && patrol_dist(x, y, tx, ty) <= radius) {
                break;
            }
        }
        if (attempts >= 1000) goto reroll;
        if (is_local) {
            NetMsgPrimary.pop(is_sea ? "GOODYWAVE" : "GOODYGATE", -5000, 0,
                              is_sea ? 0 : "stars_sm.pcx");
        }
        Vehs[veh_id].moves_expended = 0;
        Vehs[veh_id].order = 0;
        Vehs[veh_id].state &= 0xFCFFFDFF;
        stack_put(stack_veh(veh_id, 1), tx, ty);
        if (Vehs[veh_id].faction_id == ConsoleGlobal.faction_id_) {
            draw_tile_fixup(x, y, has_abil(Vehs[veh_id].proto_id, ABL_DEEP_RADAR), 2);
        } else {
            draw_tile(x, y, 2);
        }
        if (is_local) {
            ConsoleGlobal.focus(tx, ty, faction_id);
            WorldClimateWorldWin.redraw();
        }
        spot_all(veh_id, 1);
        return 0;
    }
    case 5: {
        if (GameDataStateStorage.fields_.game_state_ & 0x100000 && !is_sea) {
            kind = 7;
            continue;
        }
        const int base_id = base_find(x, y, faction_id);
        if (!IsMultiplayerNet && base_id >= 0) {
            const int item = Bases[base_id].queue_production_id[0];
            if (item > -70) {
                int cost;
                if (item >= 0) {
                    if (VehPrototypes[item].plan == 8 && Bases[base_id].population_size == 1) {
                        goto reroll;
                    }
                    cost = veh_cost(item, base_id, 0);
                    parse_says(0, VehPrototypes[item].veh_name, -1, -1);
                } else {
                    const int facility_id = -item;
                    cost = Facility[facility_id].cost;
                    parse_say(0, Facility[facility_id].name, -1, -1);
                }
                const int minerals = cost_factor(faction_id, 1, -1) * cost;
                if (Bases[base_id].minerals_accumulated < minerals * 3 / 4) {
                    Bases[base_id].minerals_accumulated = minerals;
                    if (is_local) {
                        parse_says(1, Bases[base_id].name_string, -1, -1);
                        if (is_sea) {
                            NetMsgPrimary.pop("GOODYBUILD", -5000, 0, "indbm_sm.pcx");
                        } else {
                            NetMsgPrimary.pop("GOODYBUILD", -5000, 0, "indbm_sm.pcx");
                        }
                    }
                    return 0;
                }
            }
        }
        if (is_sea) goto reroll;
        if (GameDataStateStorage.fields_.game_rules_ & 0x40000) goto reroll;
        int artifacts = PlayersData[faction_id].proto_id_active[7];
        for (int b = 0; b < GameDataStateStorage.fields_.base_current_count_; b++) {
            if (Bases[b].faction_id_current == faction_id && Bases[b].state & 0x400) {
                artifacts++;
            }
        }
        if (artifacts >= PlayersData[faction_id].current_num_bases
            && !(GameDataStateStorage.fields_.game_state_ & 0x40000)) {
            goto reroll;
        }
    }
    case 6: {
        if (GameDataStateStorage.fields_.game_rules_ & 0x40000) goto reroll;
        if (retry > 1 && kind == 6) {
            kind = 5;
            continue;
        }
        if (is_sea && stack_check(veh_id, 8, -1, -1, -1) <= 0) goto reroll;
        if (IsMultiplayerNet && !(GameDataStateStorage.fields_.game_state_ & 0x40000)) {
            for (int i = 1; i < MaxPlayerNum; i++) {
                if (i != faction_id && is_human(i)
                    && PlayersData[faction_id].unk_95 > PlayersData[i].unk_95) {
                    goto reroll;
                }
            }
        }
        PlayersData[faction_id].unk_95++;
        if (VehPrototypes[Vehs[veh_id].proto_id].plan == 12) goto reroll;
        const int artifact_id = veh_init(BSC_ALIEN_ARTIFACT, faction_id, x, y);
        if (artifact_id < 0) {
            return 0;
        }
        veh_skip(artifact_id);
        draw_tile(x, y, 2);
        if (is_local) {
            if (is_sea) {
                NetMsgPrimary.pop("GOODYARTIFACT", -5000, 0, "art_dis_sm.pcx");
            } else {
                NetMsgPrimary.pop("GOODYARTIFACT", -5000, 0, "art_dis_sm.pcx");
            }
            g_FX.fade(0x1B);
        }
        return 0;
    }
    case 7: {
        if (GameDataStateStorage.fields_.game_rules_ & 0x200000) goto reroll;
        if (GameDataStateStorage.fields_.game_rules_ & 0x1000000
            && GameDataStateStorage.fields_.diff_level_current_ <= 1) {
            goto reroll;
        }
        if (retry > 1) goto reroll;
        if (is_sea) {
            kind = 8;
            continue;
        }
        base_find(x, y);
        if (BaseFindDist < 3) goto reroll;
        if (IsMultiplayerNet && faction_id == GameDataStateStorage.fields_.ranking_faction_id_unk2_
            && !(rand() % 2)) {
            goto reroll;
        }
        int spawned = 0;
        int spawn_ok = 0;
        if (rand() % 2 && ExpansionEnabled) {
            spawn_ok = 1;
        }
        if (is_local) {
            g_FX.play(9);
            NetMsgPrimary.pop("GOODYFUNGUS", -5000, 0, "fung_sm.pcx");
        }
        site_radius(x, y, 0);
        for (int i = 0; i < 9; i++) {
            const int tx = wrap_map_x(x + RadiusOffsetX[i]);
            const int ty = y + RadiusOffsetY[i];
            if (on_map(tx, ty) && (map_loc(tx, ty)->climate & ~0x1F) >= 0x40) {
                bit_set(tx, ty, 0x20, 1);
                bit_set(tx, ty, 0x28805C, 0);
                synch_bit(tx, ty, faction_id);
                if (!spawned && spawn_ok == 1 && rand() % 2 && anything_at(tx, ty) == -1
                    && ExpansionEnabled && !is_ocean(tx, ty)) {
                    veh_init(BSC_FUNGAL_TOWER, 0, tx, ty);
                    spawn_ok = 0;
                    spawned = 1;
                }
                draw_tile(tx, ty, 2);
            }
        }
        return 0;
    }
    case 8:
        if (GameDataStateStorage.fields_.game_rules_ & 0x20000) goto reroll;
        if (bonus_at(x, y, 0)) goto reroll;
        if (is_sea) goto reroll;
        site_radius(x, y, 0);
        bit_set(x, y, 0x2000, 1);
        bit_set(x, y, 0x20, 0);
        synch_bit(x, y, faction_id);
        draw_tiles(x, y, 2);
        monolith(veh_id);
        return 0;
    case 9: {
        if (GameDataStateStorage.fields_.net_turn_flags_ & 1) goto reroll;
        if (GameDataStateStorage.fields_.game_tech_achieved_[TECH_ORBITAL]) goto reroll;
        if (is_sea && stack_check(veh_id, 8, -1, -1, -1) <= 0) goto reroll;
        int unit_id = BSC_UNITY_ROVER;
        if (GameDataStateStorage.fields_.game_tech_achieved_[TECH_FOSSIL] && !(rand() % 3)
            && !PlayersData[faction_id].proto_id_active[BSC_UNITY_SCOUT_CHOPPER]) {
            base_find(x, y, faction_id);
            if (BaseFindDist <= speed_proto(BSC_UNITY_SCOUT_CHOPPER) * 3) {
                unit_id = BSC_UNITY_SCOUT_CHOPPER;
            }
        }
        int odds = PlayersData[faction_id].proto_id_active[BSC_UNITY_FOIL] + 3;
        if (Continents[map_loc(x, y)->region].tile_count <= 50) {
            odds--;
        }
        if (PlayersData[faction_id].proto_id_active[BSC_UNITY_FOIL] < 2
            && !bounded_crt_random(odds)) {
            if (is_sea) {
                unit_id = BSC_UNITY_FOIL;
            } else {
                for (int i = 0; i < 8; i++) {
                    const int tx = wrap_map_x(x + VehMoveDeltaX[i]);
                    const int ty = y + VehMoveDeltaY[i];
                    if (on_map(tx, ty) && is_ocean(tx, ty)
                        && Continents[map_loc(tx, ty)->region].tile_count >= 80) {
                        x = tx;
                        y = ty;
                        unit_id = BSC_UNITY_FOIL;
                        break;
                    }
                }
            }
        }
        if (PlayersData[faction_id].proto_id_active[unit_id]
            && (PlayersData[faction_id].proto_id_active[BSC_UNITY_ROVER]
                    + PlayersData[faction_id].proto_id_active[BSC_UNITY_SCOUT_CHOPPER]
                    + PlayersData[faction_id].proto_id_active[BSC_UNITY_FOIL]
                    > (PlayersData[faction_id].current_num_bases + 1) / 2
                || PlayersData[faction_id].proto_id_active[unit_id] > 1)) {
            kind = rand() % 2 + 12;
            x = Vehs[veh_id].x;
            y = Vehs[veh_id].y;
            continue;
        }
        int level = 0;
        if (unit_id == BSC_UNITY_ROVER && !(rand() % 5) && ExpansionEnabled) {
            for (int i = 1; i < MaxPlayerNum; i++) {
                if (has_tech(82, i) && level < 1) {
                    level = 1;
                }
                if (has_tech(83, i)) {
                    level = 2;
                }
            }
            unit_id = BSC_BATTLE_OGRE_MK1;
            if (level > 0) {
                unit_id = BSC_BATTLE_OGRE_MK2;
            }
            if (level > 1) {
                unit_id = BSC_BATTLE_OGRE_MK3;
            }
        }
        const int spawn_id = veh_init(unit_id, faction_id, x, y);
        if (spawn_id < 0) goto reroll;
        if (forced || GameDataStateStorage.fields_.turn_current_num_ < 50) {
            Vehs[spawn_id].home_base_id = -1;
        }
        Vehs[spawn_id].morale = 2;
        if (unit_id == BSC_BATTLE_OGRE_MK1) {
            Vehs[spawn_id].morale = 2;
        } else if (unit_id == BSC_BATTLE_OGRE_MK2) {
            Vehs[spawn_id].morale = 4;
        } else if (unit_id == BSC_BATTLE_OGRE_MK3) {
            Vehs[spawn_id].morale = 6;
        }
        if (is_sea && Chassis[VehPrototypes[Vehs[spawn_id].proto_id].chassis_id].triad == TRIAD_LAND) {
            sleep(spawn_id);
        }
        if (is_local) {
            parse_says(0, VehPrototypes[unit_id].veh_name, -1, -1);
            if (unit_id == BSC_BATTLE_OGRE_MK1 || unit_id == BSC_BATTLE_OGRE_MK2 || unit_id == BSC_BATTLE_OGRE_MK3) {
                if (is_sea) {
                    NetMsgPrimary.pop("OGREPOD", 5000, 0, "ogre_sm.pcx");
                } else {
                    NetMsgPrimary.pop("OGREPOD", -5000, 0, "ogre_sm.pcx");
                }
            } else if (is_sea) {
                NetMsgPrimary.pop("GOODYPOD", 5000, 0, "supply_sm.pcx");
            } else {
                NetMsgPrimary.pop("GOODYPOD", -5000, 0, "supply_sm.pcx");
            }
        }
        return 0;
    }
    case 10: {
        if (GameDataStateStorage.fields_.net_turn_flags_ & 2) goto reroll;
        if (!(rand() % 4)) {
            const int other = rand() % 7 + 1;
            if (other != faction_id
                && !(PlayersData[faction_id].diplo_treaties[other] & DTREATY_COMMLINK)
                && is_alive(other) && !is_human(other) && is_human(faction_id)
                && !(Players[other].rule_flags & RFLAG_ALIEN)) {
                treaty_on(faction_id, other, DTREATY_COMMLINK);
                if (is_local) {
                    parse_set(Players[other].is_leader_female, 0);
                    parse_says(0, Players[other].title_leader, -1, -1);
                    parse_set(Players[other].is_leader_female, 0);
                    parse_says(1, Players[other].name_leader, -1, -1);
                    parse_set(Players[other].noun_gender, Players[other].is_noun_plural);
                    parse_says(2, Players[other].noun_faction, -1, -1);
                    if (is_sea) {
                        NetMsgPrimary.pop("GOODYCOMM", 5000, 0, "supply_sm.pcx");
                    } else {
                        NetMsgPrimary.pop("GOODYCOMM", -5000, 0, "supply_sm.pcx");
                    }
                }
                return 0;
            }
        }
        if (GameDataStateStorage.fields_.game_rules_ & 0x4000000) goto reroll;
        if (IsMultiplayerNet && !forced) {
            for (int i = 1; i < MaxPlayerNum; i++) {
                if (i != faction_id && is_human(i)
                    && PlayersData[faction_id].unk_97 > PlayersData[i].unk_97 + 1
                    && (PlayersData[faction_id].ranking > PlayersData[i].ranking
                        || PlayersData[faction_id].tech_ranking > PlayersData[i].tech_ranking)) {
                    goto reroll;
                }
            }
        }
        const int start = rand() % 89;
        int i;
        int tech_id;
        for (i = 0; i < 89; i++) {
            tech_id = (start + i) % 89;
            if (has_tech(tech_id, faction_id)) continue;
            if (!tech_avail(tech_id, faction_id)) continue;
            if (tech_is_preq(tech_id, TECH_BRAIN, 9999)
                && !GameDataStateStorage.fields_.game_tech_achieved_[TECH_BRAIN]) {
                continue;
            }
            int roots = 0;
            for (int j = 0; j < 2; j++) {
                const int preq = (&Technology[tech_id].preq_tech_1)[j];
                if (preq == -1) {
                    roots++;
                }
                if (preq >= 0 && Technology[preq].preq_tech_1 == -1
                    && Technology[preq].preq_tech_2 == -1) {
                    roots++;
                    break;
                }
            }
            if (roots >= 2) break;
        }
        if (i >= 89) goto reroll;
        if (tech_id == 78 || tech_id == 79 || tech_id == 81 || tech_id == 82 || tech_id == 83) {
            goto reroll;
        }
        if (!forced) {
            PlayersData[faction_id].unk_97++;
        }
        if (is_local) {
            StringTemp[0] = '\0';
            strcat(StringTemp, "GOODYDATA");
            if (IsMultiplayerNet) {
                strcat(StringTemp, "1");
                parse_say(0, Technology[tech_id].name, -1, -1);
            }
            if (is_sea) {
                NetMsgPrimary.pop(StringTemp, 5000, 0, "supply_sm.pcx");
            } else {
                NetMsgPrimary.pop(StringTemp, -5000, 0, "supply_sm.pcx");
            }
        }
        tech_achieved(faction_id, tech_id, 0, 0);
        if (forced) {
            PlayersData[faction_id].tech_ranking--;
        }
        return 0;
    }
    case 11: {
        if (bad_reg(map_loc(x, y)->region)) goto reroll;
        if (!is_sea && (map_loc(x, y)->val3 & ~0x3F) > 0x40) goto reroll;
        if ((map_loc(x, y)->climate & ~0x1F) < 0x40) {
            kind = 12;
            continue;
        }
        int which;
        if (!is_sea) {
            base_find(x, y);
            if (BaseFindDist < 3) goto reroll;
            which = rand() % 4;
        } else {
            which = 0;
        }
        site_radius(x, y, 0);
        int changed = 0;
        for (int i = 0; i < 9; i++) {
            if (i && !bounded_crt_random(3 - (which > 1))) continue;
            const int tx = wrap_map_x(x + RadiusOffsetX[i]);
            const int ty = y + RadiusOffsetY[i];
            if (!on_map(tx, ty)) continue;
            if (i && map_loc(tx, ty)->bit & 0x100000) continue;
            if (!is_sea) {
                if (is_ocean(tx, ty)) continue;
                const int rocky = map_loc(tx, ty)->val3 & ~0x3F;
                if (rocky > 0x40 && which < 2) continue;
                if (!rocky && which == 2) continue;
            } else {
                const int altitude = map_loc(tx, ty)->climate & ~0x1F;
                if (altitude >= 0x60) continue;
                if (altitude < 0x40) {
                    if (i) continue;
                    alt_set_both(tx, ty, 2);
                }
            }
            bit_set(tx, ty, 0x288070, 0);
            if (is_sea) {
                bit_set(tx, ty, 0x8000, 1);
            } else {
                switch (which) {
                case 0:
                    bit_set(tx, ty, 0x200000, 1);
                    bit_set(tx, ty, 0x8050, 0);
                    break;
                case 1:
                    bit_set(tx, ty, 0x8000, 1);
                    bit_set(tx, ty, 0x200000, 0);
                    break;
                case 2:
                    bit_set(tx, ty, 0x14, 1);
                    bit_set(tx, ty, 0x200040, 0);
                    break;
                case 3:
                    bit_set(tx, ty, 0x40, 1);
                    bit_set(tx, ty, 0x200010, 0);
                    break;
                }
            }
            synch_bit(tx, ty, faction_id);
            draw_tile(tx, ty, 2);
            changed++;
        }
        if (!changed) goto reroll;
        if (is_local) {
            if (is_sea) {
                NetMsgPrimary.pop("GOODYKELP", -5000, 0, "kelp_sm.pcx");
                return 0;
            }
            switch (which) {
            case 0:
                NetMsgPrimary.pop("GOODYFOREST", -5000, 0, "forgr_sm.pcx");
                return 0;
            case 1:
                NetMsgPrimary.pop("GOODYFARM", -5000, 0, "forgr_sm.pcx");
                return 0;
            case 2:
                NetMsgPrimary.pop("GOODYMINE", -5000, 0, "supply_sm.pcx");
                return 0;
            case 3:
                NetMsgPrimary.pop("GOODYSOLAR", -5000, 0, "supply_sm.pcx");
                return 0;
            }
        }
        return 0;
    }
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
    case 17: {
        int spore = 0;
        if (GameDataStateStorage.fields_.game_rules_ & 0x200000) goto reroll;
        if (GameDataStateStorage.fields_.game_rules_ & 0x1000000
            && GameDataStateStorage.fields_.diff_level_current_ <= 1) {
            goto reroll;
        }
        base_find(x, y, faction_id);
        const int own_dist = BaseFindDist;
        base_find(x, y);
        if (BaseFindDist <= 1) goto reroll;
        if (kind > 13) {
            const int terrain = (map_loc(x, y)->bit & 0x20
                                 && (map_loc(x, y)->climate & ~0x1F) >= 0x40) ? 2 : 4;
            if ((GameDataStateStorage.fields_.game_state_ & 0x100000 ? 2 : 1) * own_dist
                < (kind - 13) * terrain + (4 - MapNativeLifeForms) * 2) {
                goto reroll;
            }
        }
        if (IsMultiplayerNet && faction_id == GameDataStateStorage.fields_.ranking_faction_id_unk2_
            && !(rand() % 2)) {
            goto reroll;
        }
        if (is_sea) {
            if (kind > (GameDataStateStorage.fields_.game_state_ >> 20 & 1) + 12) goto reroll;
            Vehs[veh_id].moves_expended += Rules.move_rate_roads;
        }
        int spawned = 0;
        for (int i = 0; i < 8;) {
            const int dir = (GameDataStateStorage.fields_.turn_current_num_ + i) % 8;
            const int tx = wrap_map_x(x + VehMoveDeltaX[dir]);
            const int ty = y + VehMoveDeltaY[dir];
            if (on_map(tx, ty) && is_ocean(tx, ty) == is_sea && anything_at(tx, ty) < 0) {
                if (is_sea) {
                    const int isle_id = veh_init(BSC_ISLE_OF_THE_DEEP, 0, tx, ty);
                    if (isle_id >= 0) {
                        spawned++;
                        Vehs[isle_id].visibility |= map_loc(tx, ty)->visibility;
                        int count = rand() % 2 + 1;
                        if (count >= veh_cargo(isle_id)) {
                            count = veh_cargo(isle_id);
                        }
                        for (; count > 0; count--) {
                            if (!(rand() % 5) && ExpansionEnabled) {
                                veh_init(BSC_SPORE_LAUNCHER, 0, tx, ty);
                                spore = 1;
                            } else {
                                veh_init(BSC_MIND_WORMS, 0, tx, ty);
                            }
                        }
                        break;
                    }
                } else {
                    base_find(tx, ty);
                    if (BaseFindDist > 1) {
                        int worm_id;
                        if (!(rand() % 5) && ExpansionEnabled) {
                            worm_id = veh_init(BSC_SPORE_LAUNCHER, 0, tx, ty);
                            spore = 1;
                        } else {
                            worm_id = veh_init(BSC_MIND_WORMS, 0, tx, ty);
                        }
                        if (worm_id >= 0) {
                            spawned++;
                            Vehs[worm_id].visibility |= map_loc(tx, ty)->visibility;
                            draw_tile(tx, ty, 2);
                            if (kind > (GameDataStateStorage.fields_.game_state_ & 0x100000 ? 13 : 12)
                                || GameDataStateStorage.fields_.turn_current_num_ < 15
                                || BaseFindDist <= 3) {
                                break;
                            }
                        }
                    }
                }
            }
            i += spawned ? range(5 - PlayersData[faction_id].current_num_bases / 2, 1, 4) : 1;
        }
        if (!spawned) goto reroll;
        if (is_local) {
            g_FX.play(0x11);
            if (spore) {
                NetMsgPrimary.pop(is_sea ? "GOODYISLE" : "GOODYWORMS", -5000, 0,
                                  is_sea ? "isle_sm.pcx" : "sporlnch_sm.pcx");
            } else {
                NetMsgPrimary.pop(is_sea ? "GOODYISLE" : "GOODYWORMS", -5000, 0,
                                  is_sea ? "isle_sm.pcx" : "mindworm_sm.pcx");
            }
            g_FX.fade(0x11);
        }
        return 0;
    }
    case 18: {
        if (is_sea) {
            if (rand() % 4) goto reroll;
        } else if (!Weapon[VehPrototypes[Vehs[veh_id].proto_id].weapon_id].offense_rating) {
            goto reroll;
        }
        if (VehPrototypes[Vehs[veh_id].proto_id].plan == 12) goto reroll;
        if (IsMultiplayerNet) {
            int any = 0;
            const uint32_t alive = GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_;
            for (int i = 1; i < MaxPlayerNum; i++) {
                if (alive & (1 << i) && is_human(i) && i != faction_id) {
                    if (is_sea) {
                        if (PlayersData[i].unk_70 >= PlayersData[faction_id].unk_70) {
                            any = 1;
                        }
                    } else if (PlayersData[i].total_mil_veh >= PlayersData[faction_id].total_mil_veh) {
                        any = 1;
                    }
                    if (PlayersData[i].total_mil_veh < PlayersData[faction_id].total_mil_veh / 2) {
                        goto reroll;
                    }
                }
            }
            if (!any) goto reroll;
        }
        const int clone_id = veh_init(Vehs[veh_id].proto_id, faction_id, x, y);
        if (clone_id < 0) goto reroll;
        Vehs[clone_id].home_base_id = -1;
        Vehs[clone_id].morale = Vehs[veh_id].morale;
        Vehs[clone_id].dmg_incurred = Vehs[veh_id].dmg_incurred;
        parse_says(0, VehPrototypes[Vehs[veh_id].proto_id].veh_name, -1, -1);
        if (is_local) {
            NetMsgPrimary.pop("GOODYCLONE", -5000, 0, "clone_sm.pcx");
        }
        return 0;
    }
    case 9999:
        if (is_local) {
            NetMsgPrimary.pop(is_sea ? "GOODYWHIRLPOOL" : "GOODYVANISH", 5000, 0, 0);
        }
        kill(veh_id);
        return 1;
    default: {
        if (retry > 1) {
            return 0;
        }
        if (is_sea && (map_loc(x, y)->climate & ~0x1F) < 0x40) goto reroll;
        if (map_loc(x, y)->bit & 0x20 && (map_loc(x, y)->climate & ~0x1F) >= 0x40) goto reroll;
        if (y <= 1) goto reroll;
        if (y >= MapLatitudeBounds - 2) goto reroll;
        if (GameDataStateStorage.fields_.game_rules_ & 0x10000) goto reroll;
        bit_set(x, y, 0x400, 1);
        const int resource = bonus_at(x, y, 0);
        site_radius(x, y, 0);
        if (is_local && resource) {
            if (resource == 1) {
                wave_it(0x28);
            } else if (resource == 2) {
                wave_it(0x2A);
            } else if (resource == 3) {
                ambience(0xD2);
                wave_it(0x29);
            }
            parse_say(0, Labels.strings_ptr[91 + resource], -1, -1);
            NetMsgPrimary.pop("GOODYRESOURCE", 5000, 0, "supply_sm.pcx");
        }
        draw_tile(x, y, 2);
        if (rand() % 3 && !retry) {
            retry = 2;
            goto reroll;
        }
        return 0;
    }
    }
    }
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

    offense_factor = weap_strat(weapon_id, faction_id);
    defense_factor = arm_strat(armor_id, faction_id);

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
                int other_offense = weap_strat(other_weapon, faction_id);
                int other_armor = VehPrototypes[slot].armor_id;
                int other_defense = arm_strat(other_armor, faction_id);
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

static inline void parse_prototype_report(int parse_id, int design_id) {
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
// Rewritten from the image 2026-11-08 (0.85 full-text); remaining gaps are register ties.
void __cdecl prune_protos(int faction_id, int prototype_id, int options) {
    if (prototype_id < MaxVehProtoFactionNum
        || VehPrototypes[prototype_id].flags & PROTO_TYPED_COMPLETE) {
        for (int i = 0; i < MaxVehProtoFactionNum; i++) {
            const int proto_id = faction_id * MaxVehProtoFactionNum + i;
            if (!(VehPrototypes[proto_id].flags & PROTO_ACTIVE)
                || VehPrototypes[proto_id].flags & PROTO_TYPED_COMPLETE) {
                continue;
            }
            int chassis_done = 0;
            int armor_done = 0;
            int weapon_done = 0;
            for (int walk = 0; walk < 2 * MaxVehProtoFactionNum; walk++) {
                const int id = walk % MaxVehProtoFactionNum
                    + (walk < MaxVehProtoFactionNum ? 0 : faction_id * MaxVehProtoFactionNum);
                if (!(VehPrototypes[id].flags & PROTO_ACTIVE)) continue;
                if (id < MaxVehProtoFactionNum) {
                    if (!has_tech(VehPrototypes[id].preq_tech, faction_id)) continue;
                } else if (!(VehPrototypes[id].flags & PROTO_TYPED_COMPLETE)) {
                    continue;
                }
                if (VehPrototypes[id].weapon_id == VehPrototypes[proto_id].weapon_id) {
                    weapon_done = 1;
                }
                if (VehPrototypes[id].armor_id == VehPrototypes[proto_id].armor_id) {
                    armor_done = 1;
                }
                if (VehPrototypes[id].chassis_id == VehPrototypes[proto_id].chassis_id) {
                    chassis_done = 1;
                }
                if (Weapon[VehPrototypes[proto_id].weapon_id].offense_rating > 0
                    && Weapon[VehPrototypes[id].weapon_id].offense_rating
                        >= Weapon[VehPrototypes[proto_id].weapon_id].offense_rating) {
                    weapon_done = 1;
                }
                if (Armor[VehPrototypes[proto_id].armor_id].defense_rating > 0
                    && Armor[VehPrototypes[id].armor_id].defense_rating
                        >= Armor[VehPrototypes[proto_id].armor_id].defense_rating) {
                    armor_done = 1;
                }
                if (Chassis[VehPrototypes[proto_id].chassis_id].triad
                        == Chassis[VehPrototypes[id].chassis_id].triad
                    && Chassis[VehPrototypes[id].chassis_id].speed
                        >= Chassis[VehPrototypes[proto_id].chassis_id].speed) {
                    chassis_done = 1;
                }
            }
            if (weapon_done && armor_done && chassis_done) {
                VehPrototypes[proto_id].flags |= PROTO_TYPED_COMPLETE | 0x100;
            }
        }
    }

    if (!is_human(faction_id)) {
        for (int i = 0; i < MaxVehProtoFactionNum; i++) {
            const int target_id = faction_id * MaxVehProtoFactionNum + i;
            if (!(VehPrototypes[target_id].flags & PROTO_ACTIVE)
                || VehPrototypes[target_id].obsolete_factions & (1 << faction_id)) {
                continue;
            }
            if (target_id < MaxVehProtoFactionNum
                && !has_tech(VehPrototypes[target_id].preq_tech, faction_id)) {
                continue;
            }
            int typed;
            if (target_id < MaxVehProtoFactionNum
                || VehPrototypes[target_id].flags & PROTO_TYPED_COMPLETE) {
                typed = 1;
                for (int cand = 0; cand < MaxVehProtoFactionNum; cand++) {
                    if (!(VehPrototypes[cand].flags & PROTO_ACTIVE)) continue;
                    if (cand == target_id) continue;
                    if (cand < MaxVehProtoFactionNum
                        && (Weapon[VehPrototypes[cand].weapon_id].offense_rating < 0
                            || cand == BSC_SPORE_LAUNCHER)) {
                        continue;
                    }
                    if (VehPrototypes[cand].unk_1 != VehPrototypes[target_id].unk_1) continue;
                    if (Chassis[VehPrototypes[cand].chassis_id].triad
                        != Chassis[VehPrototypes[target_id].chassis_id].triad) {
                        continue;
                    }
                    if (VehPrototypes[cand].plan != VehPrototypes[target_id].plan) continue;
                    if (VehPrototypes[cand].reactor_id >= VehPrototypes[target_id].reactor_id
                        || Weapon[VehPrototypes[cand].weapon_id].offense_rating < 0) {
                        if (Chassis[VehPrototypes[cand].chassis_id].range
                            != Chassis[VehPrototypes[target_id].chassis_id].range) {
                            continue;
                        }
                        if (Weapon[VehPrototypes[cand].weapon_id].offense_rating <= 0) {
                            if (VehPrototypes[cand].weapon_id != VehPrototypes[target_id].weapon_id) {
                                continue;
                            }
                        } else if (Weapon[VehPrototypes[cand].weapon_id].offense_rating
                                   > Weapon[VehPrototypes[target_id].weapon_id].offense_rating) {
                            continue;
                        }
                        if (VehPrototypes[cand].carry_capacity
                            > VehPrototypes[target_id].carry_capacity) {
                            continue;
                        }
                        if (Armor[VehPrototypes[cand].armor_id].defense_rating <= 0) {
                    if (VehPrototypes[cand].armor_id != VehPrototypes[target_id].armor_id) {
                        continue;
                    }
                } else if (armor_val(cand, faction_id)
                           > armor_val(target_id, faction_id)) {
                    continue;
                }
                        if (Chassis[VehPrototypes[cand].chassis_id].speed
                            > Chassis[VehPrototypes[target_id].chassis_id].speed) {
                            continue;
                        }
                        if (bit_count(VehPrototypes[cand].ability_flags
                                      & VehPrototypes[target_id].ability_flags)
                            < bit_count(VehPrototypes[cand].ability_flags)) {
                            continue;
                        }
                        if ((VehPrototypes[cand].ability_flags
                             ^ VehPrototypes[target_id].ability_flags) & 0x408418) {
                            continue;
                        }
                    }
                    VehPrototypes[cand].obsolete_factions |= 1 << faction_id;
                    if (options) synch_obs(cand);
                }
            } else {
                typed = 0;
            }
            for (int j = 0; j < MaxVehProtoFactionNum; j++) {
                const int peer = faction_id * MaxVehProtoFactionNum + j;
                if (!(VehPrototypes[peer].flags & PROTO_ACTIVE)) continue;
                if (peer == target_id) continue;
                if (VehPrototypes[peer].unk_1 != VehPrototypes[target_id].unk_1) continue;
                if (Chassis[VehPrototypes[peer].chassis_id].triad
                    != Chassis[VehPrototypes[target_id].chassis_id].triad) {
                    continue;
                }
                if (VehPrototypes[peer].plan != VehPrototypes[target_id].plan) continue;
                if (!typed && VehPrototypes[peer].flags & PROTO_TYPED_COMPLETE) continue;
                if (VehPrototypes[peer].reactor_id >= VehPrototypes[target_id].reactor_id) {
                    if (VehPrototypes[peer].carry_capacity
                        > VehPrototypes[target_id].carry_capacity) {
                        continue;
                    }
                    if (Chassis[VehPrototypes[peer].chassis_id].range
                        != Chassis[VehPrototypes[target_id].chassis_id].range) {
                        continue;
                    }
                    if (Weapon[VehPrototypes[peer].weapon_id].offense_rating <= 0) {
                        if (VehPrototypes[peer].weapon_id != VehPrototypes[target_id].weapon_id) {
                            continue;
                        }
                    } else if (Weapon[VehPrototypes[peer].weapon_id].offense_rating
                               > Weapon[VehPrototypes[target_id].weapon_id].offense_rating) {
                        continue;
                    }
                    if (Armor[VehPrototypes[peer].armor_id].defense_rating <= 0) {
                    if (VehPrototypes[peer].armor_id != VehPrototypes[target_id].armor_id) {
                        continue;
                    }
                } else if (armor_val(peer, faction_id)
                           > armor_val(target_id, faction_id)) {
                    continue;
                }
                    if (Chassis[VehPrototypes[peer].chassis_id].speed
                        > Chassis[VehPrototypes[target_id].chassis_id].speed) {
                        continue;
                    }
                    if (VehPrototypes[peer].flags & PROTO_TYPED_COMPLETE
                        && bit_count(VehPrototypes[target_id].ability_flags
                                     & VehPrototypes[peer].ability_flags)
                            < bit_count(VehPrototypes[peer].ability_flags)) {
                        continue;
                    }
                    if ((VehPrototypes[target_id].ability_flags
                         ^ VehPrototypes[peer].ability_flags) & 0x408418) {
                        continue;
                    }
                }
                VehPrototypes[peer].obsolete_factions |= 1 << faction_id;
                if (options) synch_obs(peer);
            }
        }
        return;
    }

    if (!IsMultiplayerNet
        && !(GameDataStateStorage.fields_.game_more_preferences_ & MPREF_BSC_AUTO_PRUNE_OBS_VEH)) {
        return;
    }
    for (int rwalk = 0; rwalk < 2 * MaxVehProtoFactionNum; rwalk++) {
        int rid = rwalk % MaxVehProtoFactionNum;
        if (rwalk >= MaxVehProtoFactionNum) {
            rid += faction_id * MaxVehProtoFactionNum;
        } else if (!has_tech(VehPrototypes[rid].preq_tech, faction_id)) {
            continue;
        }
        if (VehPrototypes[rid].obsolete_factions & (1 << faction_id)) continue;
        if (!(VehPrototypes[rid].flags & PROTO_ACTIVE)) continue;
        for (int twalk = 0; twalk < 2 * MaxVehProtoFactionNum; twalk++) {
            int tid = twalk % MaxVehProtoFactionNum;
            if (twalk >= MaxVehProtoFactionNum) {
                tid += faction_id * MaxVehProtoFactionNum;
            }
            if (twalk == rwalk) continue;
            if (!(VehPrototypes[tid].flags & PROTO_ACTIVE)) continue;
            if (VehPrototypes[tid].obsolete_factions & (1 << faction_id)) continue;
            if (tid < MaxVehProtoFactionNum
                && (Weapon[VehPrototypes[tid].weapon_id].offense_rating < 0
                    || tid == BSC_SPORE_LAUNCHER)) {
                continue;
            }
            if (VehPrototypes[tid].flags & 0x200) continue;
            int prototyped = (VehPrototypes[tid].flags & 8u) >> 3;
            if (rid >= MaxVehProtoFactionNum
                && !(VehPrototypes[rid].flags & PROTO_TYPED_COMPLETE)
                && (tid < MaxVehProtoFactionNum
                    || VehPrototypes[tid].flags & PROTO_TYPED_COMPLETE)) {
                continue;
            }
            if (prototyped
                && VehPrototypes[tid].reactor_id >= VehPrototypes[rid].reactor_id) {
                continue;
            }
            if (Weapon[VehPrototypes[tid].weapon_id].offense_rating <= 0) {
                        if (VehPrototypes[tid].weapon_id != VehPrototypes[rid].weapon_id) {
                            continue;
                        }
                    } else if (Weapon[VehPrototypes[tid].weapon_id].offense_rating
                               > Weapon[VehPrototypes[rid].weapon_id].offense_rating) {
                        continue;
                    }
            if (Armor[VehPrototypes[tid].armor_id].defense_rating <= 0) {
                    if (VehPrototypes[tid].armor_id != VehPrototypes[rid].armor_id) {
                        continue;
                    }
                } else if (armor_val(tid, faction_id)
                           > armor_val(rid, faction_id)) {
                    continue;
                }
            if (VehPrototypes[tid].plan != 3) {
                if ((Weapon[VehPrototypes[tid].weapon_id].offense_rating > 1)
                    != (Weapon[VehPrototypes[rid].weapon_id].offense_rating > 1)) {
                    continue;
                }
                if ((Armor[VehPrototypes[tid].armor_id].defense_rating > 1)
                    != (Armor[VehPrototypes[rid].armor_id].defense_rating > 1)) {
                    continue;
                }
            }
            if (VehPrototypes[tid].reactor_id > VehPrototypes[rid].reactor_id) continue;
            if (VehPrototypes[tid].carry_capacity > VehPrototypes[rid].carry_capacity) continue;
            if (Chassis[VehPrototypes[tid].chassis_id].range
                != Chassis[VehPrototypes[prototype_id].chassis_id].range) {
                continue;
            }
            if (Chassis[VehPrototypes[tid].chassis_id].triad
                != Chassis[VehPrototypes[rid].chassis_id].triad) {
                continue;
            }
            if (Chassis[VehPrototypes[tid].chassis_id].speed
                > Chassis[VehPrototypes[rid].chassis_id].speed) {
                continue;
            }
            if (VehPrototypes[tid].ability_flags != VehPrototypes[rid].ability_flags) {
                if ((VehPrototypes[rid].ability_flags ^ VehPrototypes[tid].ability_flags)
                    & 0xFFFBD5FC) {
                    continue;
                }
                if (VehPrototypes[tid].reactor_id >= VehPrototypes[rid].reactor_id
                    && (VehPrototypes[rid].ability_flags & VehPrototypes[tid].ability_flags)
                        != VehPrototypes[tid].ability_flags) {
                    if (rid < MaxVehProtoFactionNum) continue;
                    if (PlayersData[faction_id].proto_id_active[tid]) continue;
                    if (PlayersData[faction_id].proto_id_queue[tid]) continue;
                }
            }
            if (VehPrototypes[tid].plan != VehPrototypes[rid].plan) continue;
            if (prototyped) {
                if (VehPrototypes[tid].unk_1 != VehPrototypes[rid].unk_1) continue;
                if ((VehPrototypes[rid].ability_flags & VehPrototypes[tid].ability_flags)
                    != VehPrototypes[tid].ability_flags) {
                    continue;
                }
            }
            if (VehPrototypes[tid].cost < VehPrototypes[rid].cost
                && VehPrototypes[tid].reactor_id >= VehPrototypes[rid].reactor_id) {
                if (tid < MaxVehProtoFactionNum) continue;
                if (VehPrototypes[tid].flags & 8) continue;
            }
            if (VehPrototypes[tid].weapon_id == VehPrototypes[rid].weapon_id
                && VehPrototypes[tid].armor_id == VehPrototypes[rid].armor_id
                && VehPrototypes[tid].chassis_id == VehPrototypes[rid].chassis_id
                && VehPrototypes[tid].reactor_id == VehPrototypes[rid].reactor_id
                && (tid < MaxVehProtoFactionNum || rid < MaxVehProtoFactionNum)) {
                continue;
            }
            if (tid == prototype_id) {
                VehPrototypes[tid].flags |= 8;
                if (options) synch_obs(tid);
                continue;
            }
            const uint8_t bit = static_cast<uint8_t>(1 << faction_id);
            VehPrototypes[tid].obsolete_factions |= bit;
            if (options) synch_obs(tid);
            int processed = 0;
            int latch = 0;
            if (tid < MaxVehProtoFactionNum
                && !has_tech(VehPrototypes[tid].preq_tech, faction_id)) {
                continue;
            }
            int found = 0;
            for (int b = 0; b < GameDataStateStorage.fields_.base_current_count_; b++) {
                if (Bases[b].faction_id_current == faction_id) {
                    for (int q = 0; q <= Bases[b].queue_size; q++) {
                        if (Bases[b].queue_production_id[q] == tid) {
                            found = 1;
                            goto searched;
                        }
                    }
                }
            }
        searched:
            for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_;
                 base_id++) {
                if (Bases[base_id].faction_id_current != faction_id) continue;
                if ((!IsMultiplayerNet && faction_id == ConsoleGlobal.faction_id_
                     && found && !processed)
                    || (found && (!options
                                  || !NetDaemonNet.lock_base(base_id, 0, -1, -1)))) {
                    if (!IsMultiplayerNet && faction_id == ConsoleGlobal.faction_id_
                        && !processed) {
                        parse_prototype_report(0, tid);
                        parse_prototype_report(1, rid);
                        int choice = 1;
                        if (!(VehPrototypes[tid].flags & 0x200) && !latch) {
                            choice = X_pop("ASKPRODUPGRADE", 0);
                            if (choice > 1) latch = 1;
                        }
                        if (!latch && (VehPrototypes[tid].flags & 0x200 || choice == 1)) {
                            VehPrototypes[tid].obsolete_factions &= ~bit;
                            if (VehPrototypes[tid].flags & 8) {
                                VehPrototypes[tid].flags |= 0x200;
                            }
                            VehPrototypes[tid].flags |= 8;
                            goto next_target;
                        }
                    }
                    processed = 1;
                    for (int q = 0; q <= Bases[base_id].queue_size; q++) {
                        if (Bases[base_id].queue_production_id[q] == tid) {
                            Bases[base_id].queue_production_id[q] = rid;
                        }
                    }
                    if (IsMultiplayerNet && options) {
                        NetDaemonNet.unlock_base(base_id);
                    }
                }
            }
            if (processed && IsMultiplayerNet && faction_id == ConsoleGlobal.faction_id_) {
                parse_prototype_report(0, tid);
                parse_prototype_report(1, rid);
                if (Players[faction_id].rule_flags & RFLAG_ALIEN) {
                    popp(ScriptPopupFile, "PRODUPGRADE", 0, "alunit_sm.pcx", 0);
                } else {
                    popp(ScriptPopupFile, "PRODUPGRADE", 0, "unit_sm.pcx", 0);
                }
            }
        next_target:;
        }
    }
}

/*
Purpose: Determine whether a faction is able to jump up the tech tree for the specified tech.
// ORIGINAL: 0x0057CE50 ?valid_tech_leap@@YAHHH@Z 0x0057CE50-0x0057CF9E BYTE_EXACT
// size      334 bytes
// prototype int (__cdecl ?valid_tech_leap@@YAHHH@Z)(int techID, int factionID)
// callers   1   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0057F0B0 0x005B9F20
Return Value: Is a tech leap possible? true/false
Status: Complete
*/
// TRIED: has_tech_call() (technology.h) exists for exactly this - a real
// out-of-line has_tech() the way best_specialist (base.cpp) needs one - but
// that idiom only works CROSS-TU. has_tech's `MEASURED inline` body lives in
// technology.h, included at the top of THIS file, so it is visible to every
// function below it uniformly; there is no position in technology.cpp where
// valid_tech_leap could sit "before" it the way tech_avail's own marker
// describes (that split predates has_tech's move into the header and no
// longer holds - tech_avail measures MISMATCH here too, not BYTE_EXACT).
// `#pragma inline_depth(0)` around this function has NO effect (measured):
// it limits the RECURSION depth of nested inline expansion, not whether a
// direct, first-level call to an explicitly `inline` function expands, and
// neither does `/Ob0` on this flag set. has_tech_call still reads correctly
// here even though it does not yet reproduce the image's real `call
// 0x5b9f20` at these 4 sites - fixing it needs has_tech's definition moved
// out of the header, which has ~108 other call sites across the tree and is
// out of scope for this body.
//   (STALE: has_tech_call DOES reproduce the image's four `call 0x5b9f20`
//   now - call_diff agrees on every call this body makes.)
// LEVER: the nine failure paths share one out-of-line `return false` because
//   they are `||`-combined conditions (pattern: `if (a || b) return 0;`), not
//   separate guards or a goto; the two `return true;` sites then stay inline.
// LEVER: indexed loops, not pointer locals - `Reactor[i].preq_tech`,
//   `Weapon[i].preq_tech`, `Weapon[i].offense_rating`. The image strength-
//   reduces each into a field pointer and keeps the weapon counter alive
//   (`shl esi, 4; movsx ecx, byte ptr [esi + 0x94ae68]`) only because the
//   body indexes the array again; a `RulesWeapon *weapon` local read the
//   field off the pointer and lost the call/shl/add/movsx order.
BOOL __cdecl valid_tech_leap(int tech_id, int faction_id) {
    if (Technology[tech_id].preq_tech_1 >= 0
        && (!has_tech(Technology[Technology[tech_id].preq_tech_1].preq_tech_1, faction_id)
            || !has_tech(Technology[Technology[tech_id].preq_tech_1].preq_tech_2, faction_id))) {
        return false; // missing a prerequisite of the 1st prerequisite
    }
    if (Technology[tech_id].preq_tech_2 >= 0
        && (!has_tech(Technology[Technology[tech_id].preq_tech_2].preq_tech_1, faction_id)
            || !has_tech(Technology[Technology[tech_id].preq_tech_2].preq_tech_2, faction_id))) {
        return false; // missing a prerequisite of the 2nd prerequisite
    }
    if (Technology[tech_id].preq_tech_1 <= TechDisabled
        || Technology[tech_id].preq_tech_2 <= TechDisabled) {
        return false; // disabled
    }
    for (int i = 0; i < MaxReactorNum; i++) {
        if (tech_id == Reactor[i].preq_tech) {
            return false; // leap not possible for reactor tech
        }
    }
    for (int j = 0; j < MaxWeaponNum; j++) {
        if (Weapon[j].preq_tech == tech_id) { // may end early if 2 weapons have the same preq
            // `ranking` is compared signed in the image (`jg`).
            if (PlayersData[faction_id].ranking > 2 // lowest two ranking factions
                || Weapon[j].offense_rating > weapon_budget(faction_id, 99, false) + 2) {
                return false;
            }
            return true;
        }
    }
    return true;
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



// ORIGINAL: 0x00581260 ?consider_designs@@YAXH@Z 0x00581260-0x00583C8F
// RECOVERY_BINDING: v1 {"reason":"Recover the complete AI prototype-selection operation through typed faction, rules, prototype and proposal owners, with the original signed faction argument and shared armor-scale lifetime across the strategy routes.","evidence":["Pinned original 0x00581260-0x00583C8F; native dispatch plans 0/4,1,2,6 and skipped 3/5; native negative-armor sentinel gate.","Independent full-owner and proposal-budget reviews; retained mechanical seed TRIED records its unverified shared-push argument history."]}
// TRIED: mechanical register/stack-slot goto transliteration (304 labels, one 7-case switch off 0x0094A37D, 6375-line body): compiles, MISMATCH sim 0.80 on /O2 /Oy- framed. Cross-branch shared-push args (26 pushes never traced to a specific call by the linear model) are a known soft spot - values landed but not re-verified by hand past the two spots that broke arg *counts* outright.
// kind      game
void __cdecl consider_designs(int faction_id) {
    // Rewritten from the image 2026-11-08 block by block.
    FirstProposedPrototype = -1;
    int war_score = 0;
    for (int i = 1; i < MaxPlayerNum; i++) {
        if (i == faction_id) {
            continue;
        }
        uint32_t treaty = PlayersData[faction_id].diplo_treaties[i];
        if (treaty & DTREATY_ATROCITY_VICTIM) {
            war_score += (PlayersData[i].atrocities + 2) / 3;
        } else if (treaty & DTREATY_WANT_REVENGE) {
            if (is_human(i)
                && (PlayersData[i].ranking > PlayersData[faction_id].ranking
                    || PlayersData[i].diplo_unk4[faction_id]
                       > PlayersData[faction_id].diplo_unk4[i])) {
                war_score += (GameDataStateStorage.fields_.game_rules_ >> 6) & 1;
            }
            war_score += (PlayersData[i].integrity_blemishes - 1) / 4;
        }
    }
    enemy_capabilities(faction_id);
    int reactor = best_reactor(faction_id);
    int reactor_power = 2 << reactor;
    int new_armor = -1;
    int new_weapon = -1;
    // The image keeps one slot for the armor scan index and the plan-1 factor,
    // and reads it again in the offensive block: one variable.
    int scratch;
    int weapon_id;
    for (weapon_id = 0; weapon_id < MaxWeaponNum; weapon_id++) {
        if (!has_tech(Weapon[weapon_id].preq_tech, faction_id)
            || Weapon[weapon_id].offense_rating <= 0
            || Weapon[weapon_id].offense_rating >= 99
            || weapon_id == WPN_CONVENTIONAL_PAYLOAD) {
            continue;
        }
        if (new_weapon >= 0
            && Weapon[new_weapon].offense_rating > Weapon[weapon_id].offense_rating) {
            continue;
        }
        for (int i = 0; i < 2 * MaxVehProtoFactionNum; i++) {
            int proto_id = i % MaxVehProtoFactionNum;
            if (i >= MaxVehProtoFactionNum) {
                proto_id += faction_id * MaxVehProtoFactionNum;
            }
            if ((VehPrototypes[proto_id].flags & PROTO_ACTIVE)
                && !(VehPrototypes[proto_id].obsolete_factions & (1 << faction_id))
                && (proto_id >= MaxVehProtoFactionNum
                    || has_tech(VehPrototypes[proto_id].preq_tech, faction_id))
                && !Chassis[VehPrototypes[proto_id].chassis_id].missile) {
                if (Weapon[VehPrototypes[proto_id].weapon_id].offense_rating
                    >= Weapon[weapon_id].offense_rating) {
                    break;
                }
                new_weapon = weapon_id;
            }
        }
    }
    for (scratch = 0; scratch < MaxArmorNum; scratch++) {
        if (!has_tech(Armor[scratch].preq_tech, faction_id)
            || Armor[scratch].defense_rating <= 1) {
            continue;
        }
        if (new_armor >= 0
            && Armor[new_armor].defense_rating > Armor[scratch].defense_rating) {
            continue;
        }
        for (int i = 0; i < 2 * MaxVehProtoFactionNum; i++) {
            int proto_id = i % MaxVehProtoFactionNum;
            if (i >= MaxVehProtoFactionNum) {
                proto_id += faction_id * MaxVehProtoFactionNum;
            }
            if ((VehPrototypes[proto_id].flags & PROTO_ACTIVE)
                && !(VehPrototypes[proto_id].obsolete_factions & (1 << faction_id))
                && (proto_id >= MaxVehProtoFactionNum
                    || has_tech(VehPrototypes[proto_id].preq_tech, faction_id))) {
                if (Armor[VehPrototypes[proto_id].armor_id].defense_rating
                    >= Armor[scratch].defense_rating) {
                    break;
                }
                new_armor = scratch;
            }
        }
    }
    if (new_weapon >= 0 || new_armor >= 0) {
        for (int i = 0; i < MaxVehProtoFactionNum; i++) {
            int proto_id = faction_id * MaxVehProtoFactionNum + i;
            if (!(VehPrototypes[proto_id].flags & PROTO_ACTIVE)
                || VehPrototypes[proto_id].obsolete_factions & (1 << faction_id)) {
                continue;
            }
            int chassis_id = VehPrototypes[proto_id].chassis_id;
            if (Chassis[chassis_id].missile) {
                continue;
            }
            int weapon;
            int armor;
            switch (VehPrototypes[proto_id].plan) {
            case PLAN_OFFENSIVE:
            case PLAN_AIR_SUPERIORITY:
                if (new_weapon >= 0) {
                    propose_proto(faction_id, chassis_id, new_weapon,
                                  VehPrototypes[proto_id].armor_id,
                                  VehPrototypes[proto_id].ability_flags, reactor,
                                  VehPrototypes[proto_id].unk_1, 0);
                }
                break;
            case PLAN_COMBAT:
                if (new_weapon >= 0) {
                    armor = VehPrototypes[proto_id].armor_id;
                    if (new_armor >= 0) {
                        scratch = (reactor == 1) ? 2 : 1;
                        int offense = Weapon[new_weapon].offense_rating;
                        int limit = scratch * reactor_power * 4 / offense;
                        if (limit < 2 || offense < 2) {
                            limit = 2;
                        } else if (limit > offense) {
                            limit = offense;
                        }
                        if (Armor[new_armor].defense_rating <= limit) {
                            armor = new_armor;
                        }
                    }
                    propose_proto(faction_id, chassis_id, new_weapon, armor,
                                  VehPrototypes[proto_id].ability_flags, reactor,
                                  VehPrototypes[proto_id].unk_1, 0);
                } else {
                    scratch = (reactor == 1) ? 2 : 1;
                    weapon = VehPrototypes[proto_id].weapon_id;
                    int offense = Weapon[weapon].offense_rating;
                    int limit = scratch * reactor_power * 4 / offense;
                    if (limit < 2 || offense < 2) {
                        offense = 2;
                    } else if (limit <= offense) {
                        offense = limit;
                    }
                    if (Armor[new_armor].defense_rating <= offense) {
                        propose_proto(faction_id, chassis_id, weapon, new_armor,
                                      VehPrototypes[proto_id].ability_flags, reactor,
                                      VehPrototypes[proto_id].unk_1, 0);
                    }
                }
                break;
            case PLAN_DEFENSIVE:
                if (new_armor >= 0) {
                    propose_proto(faction_id, chassis_id,
                                  VehPrototypes[proto_id].weapon_id, new_armor,
                                  VehPrototypes[proto_id].ability_flags, reactor,
                                  VehPrototypes[proto_id].unk_1, 0);
                }
                break;
            case PLAN_NAVAL_SUPERIORITY:
                armor = VehPrototypes[proto_id].armor_id;
                weapon = VehPrototypes[proto_id].weapon_id;
                if (new_weapon >= 0) {
                    int margin = 5;
                    if (reactor * reactor >= 5) {
                        margin = reactor * reactor;
                    }
                    if (proto_cost(chassis_id, new_weapon, armor,
                                   VehPrototypes[proto_id].ability_flags, reactor)
                        <= PlayersData[faction_id].ai_growth
                           + PlayersData[faction_id].ai_power + margin) {
                        weapon = new_weapon;
                    }
                }
                {
                    BOOL upgrade = false;
                    if (new_armor >= 0) {
                        int margin = 5;
                        if (reactor * reactor >= 5) {
                            margin = reactor * reactor;
                        }
                        if (proto_cost(chassis_id, weapon, new_armor,
                                       VehPrototypes[proto_id].ability_flags, reactor)
                            <= PlayersData[faction_id].ai_growth
                               + PlayersData[faction_id].ai_power + margin) {
                            upgrade = true;
                        }
                    }
                    if (upgrade) {
                        armor = new_armor;
                    } else if (armor != new_armor && weapon != new_weapon) {
                        break;
                    }
                }
                propose_proto(faction_id, chassis_id, weapon, armor,
                              VehPrototypes[proto_id].ability_flags, reactor,
                              VehPrototypes[proto_id].unk_1, 0);
                break;
            }
        }
    }
    // Threat bits from human rivals: 1 psi/Dream Twister, 2 psi/Neural Amplifier.
    uint32_t threats = 0;
    if (Players[faction_id].rule_psi <= 0 && !is_human(faction_id)) {
        for (int i = 1; i < MaxPlayerNum; i++) {
            if (!is_human(i)) {
                continue;
            }
            if (!great_satan(i, 0)) {
                uint32_t mask = (PlayersData[faction_id].diff_level >= DLVL_THINKER
                    || GameDataStateStorage.fields_.game_rules_ & RULES_INTENSE_RIVALRY)
                    ? DTREATY_VENDETTA : 0;
                if (!(PlayersData[faction_id].diplo_treaties[i]
                      & (mask | DTREATY_WANT_REVENGE))) {
                    continue;
                }
            }
            if (Players[i].rule_psi > 0 || has_project(SP_DREAM_TWISTER, i)) {
                threats |= 1;
            }
            if (Players[i].rule_psi > 0 || has_project(SP_NEURAL_AMPLIFIER, i)) {
                threats |= 2;
            }
        }
    }
    int ability;
    int chassis = pick_chassis(faction_id, TRIAD_LAND, 1);
    int weapon;
    int armor;
    if (chassis >= 0) {
        ability = 0;
        armor = armor_budget(faction_id, 99);
        add_abil(&ability, ABL_COMM_JAMMER, faction_id);
        if (PlayersData[faction_id].ai_fight < 0
            || (PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::MORALE] < 0
                && !is_human(faction_id))
            || Players[faction_id].rule_morale < 0) {
            add_abil(&ability, ABL_TRAINED, faction_id);
        }
        if (!is_human(faction_id)
            && (PlayersData[faction_id].enemy_best_psi_offense
                >= PlayersData[faction_id].enemy_best_weapon_value * 2
                || threats & 1)) {
            add_abil(&ability, ABL_TRANCE, faction_id);
        }
        if (has_tech(Ability[abil_bit(ABL_TRANCE)].preq_tech, faction_id)
            && (!ability || has_tech(Rules.tech_two_spec_abilities, faction_id))
            && !is_human(faction_id)
            && (threats
                || (PlayersData[faction_id].planet_ecology >= 4
                    && PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::MORALE] <= 0))) {
            ability |= ABL_TRANCE;
        }
        weapon = weapon_budget(faction_id, 1, 0);
        int cost = 2;
        while (true) {
            int next = weapon_budget(faction_id, cost, 0);
            if (next == weapon) {
                break;
            }
            int weapon_cost = proto_cost(chassis, weapon, armor, ability, reactor);
            if (proto_cost(chassis, next, armor, ability, reactor) > weapon_cost) {
                break;
            }
            weapon = next;
            cost++;
        }
        if (Armor[armor].defense_rating > 1 || DesignConsiderOverride) {
            propose_proto(faction_id, chassis, weapon, armor, ability, reactor, 2, 0);
        }
        if (has_tech(Ability[abil_bit(ABL_TRANCE)].preq_tech, faction_id)
            && !(ability & ABL_TRANCE) && !is_human(faction_id)
            && ((PlayersData[faction_id].planet_ecology >= 4
                 && PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::MORALE] <= 0)
                | threats)) {
            ability = ABL_TRANCE;
            propose_proto(faction_id, chassis, weapon, armor, ability, reactor, 0x12, 0);
        }
        if (has_tech(Ability[abil_bit(ABL_AAA)].preq_tech, faction_id)
            && !is_human(faction_id)
            && (PlayersData[faction_id].unk_66 || PlayersData[faction_id].unk_68)) {
            ability = ABL_AAA;
            propose_proto(faction_id, chassis, weapon, armor, ability, reactor, 0x14, 0);
        }
    }
    chassis = pick_chassis(faction_id, TRIAD_LAND, 1);
    if (chassis >= 0) {
        int divisor = reactor_power * 2;
        int budget = 1;
        ability = 0;
        for (int k = 2; (k + 1) / 2 * k / divisor <= PlayersData[faction_id].ai_power + 5; k++) {
            budget++;
        }
        weapon = weapon_budget(faction_id, is_human(faction_id) ? 99 : budget, 0);
        int offense = Weapon[weapon].offense_rating;
        int limit = scratch * reactor_power * 4 / offense;
        if (limit < 2 || offense < 2) {
            offense = 2;
        } else if (limit <= offense) {
            offense = limit;
        }
        if (offense <= budget / 2) {
            offense = budget / 2;
        }
        int armor_cost = range(offense, 1, 99);
        scratch = armor_budget(faction_id, armor_cost);
        int weapon_cost = Weapon[weapon].offense_rating;
        for (int x = (weapon_cost + 1) * Armor[scratch].defense_rating;
             x / divisor <= PlayersData[faction_id].ai_power + 5;
             x += Armor[scratch].defense_rating) {
            weapon_cost++;
        }
        weapon = weapon_budget(faction_id, weapon_cost, 0);
        if (!is_human(faction_id)
            && PlayersData[faction_id].unk_66 > PlayersData[faction_id].unk_105 * 2
            && PlayersData[faction_id].unk_66 > 1
            && PlayersData[faction_id].unk_66 > PlayersData[faction_id].unk_106) {
            add_abil(&ability, ABL_AAA, faction_id);
        }
        if (GameDataStateStorage.fields_.council_resolution_count_[8]
            > GameDataStateStorage.fields_.council_resolution_count_[9]
            && PlayersData[faction_id].ai_fight + PlayersData[faction_id].ai_power > 0
            && !is_human(faction_id)) {
            add_abil(&ability, ABL_NERVE_GAS, faction_id);
        }
        if (war_score && war_score > 1 - PlayersData[faction_id].ai_fight
            && PlayersData[faction_id].ai_fight >= 0) {
            if (is_human(faction_id)) {
                goto skip_empath;
            }
            add_abil(&ability, ABL_NERVE_GAS, faction_id);
        }
        if (!is_human(faction_id)
            && ((PlayersData[faction_id].enemy_best_psi_defense
                 >= PlayersData[faction_id].best_armor_value * 3 / 2
                 && PlayersData[faction_id].enemy_best_psi_defense
                    > Weapon[weapon].offense_rating / 2)
                || threats)) {
            add_abil(&ability, ABL_EMPATHIC, faction_id);
        }
skip_empath:
        if (PlayersData[faction_id].current_num_bases >= 8
            && PlayersData[faction_id].ranking >= 6
            && (PlayersData[faction_id].ai_wealth || PlayersData[faction_id].ai_tech)) {
            add_abil(&ability, ABL_CLEAN_REACTOR, faction_id);
        }
        if (Players[faction_id].rule_morale < 0) {
            add_abil(&ability, ABL_TRAINED, faction_id);
        }
        if (Armor[scratch].defense_rating > 1 && !ability
            && PlayersData[faction_id].ai_tech > 0
            && PlayersData[faction_id].enemy_best_land_speed > 1
            && PlayersData[faction_id].enemy_best_weapon_value
               >= PlayersData[faction_id].best_armor_value * 2) {
            add_abil(&ability, ABL_COMM_JAMMER, faction_id);
        }
        if (!ability
            && (PlayersData[faction_id].ai_fight < 0
                || (PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::MORALE] < 0
                    && !is_human(faction_id)))) {
            add_abil(&ability, ABL_TRAINED, faction_id);
        }
        if (has_tech(Ability[abil_bit(ABL_ANTIGRAV_STRUTS)].preq_tech, faction_id)
            && (!ability || has_tech(Rules.tech_two_spec_abilities, faction_id))) {
            ability |= ABL_ANTIGRAV_STRUTS;
        }
        if ((Weapon[weapon].offense_rating > 1 && Armor[scratch].defense_rating > 1)
            || Weapon[weapon].offense_rating > 2 || DesignConsiderOverride) {
            propose_proto(faction_id, chassis, weapon, scratch, ability, reactor, 3, 0);
            ability &= ~ABL_ANTIGRAV_STRUTS;
            if (!has_tech(Rules.tech_two_spec_abilities, faction_id)) {
                ability = 0;
            }
            if (has_tech(Ability[abil_bit(ABL_DROP_POD)].preq_tech, faction_id)) {
                propose_proto(faction_id, chassis, weapon, scratch,
                              ability | ABL_DROP_POD, reactor, 0xB, 0);
            }
            if (has_tech(Ability[abil_bit(ABL_AMPHIBIOUS)].preq_tech, faction_id)) {
                propose_proto(faction_id, chassis, weapon, scratch,
                              ability | ABL_AMPHIBIOUS, reactor, 0xC, 0);
            }
        }
    }
    weapon = weapon_budget(faction_id, 99, 0);
    if (has_tech(Ability[abil_bit(ABL_ARTILLERY)].preq_tech, faction_id)
        && (chassis = pick_chassis(faction_id, TRIAD_LAND, 1)) >= 0) {
        ability = 0;
        armor = armor_budget(faction_id, 1);
        int better_armor;
        for (int round = 0; round < 100; round++) {
            BOOL faster = false;
            int alt_chassis;
            if (Chassis[chassis].speed == 1
                && (alt_chassis = pick_chassis(faction_id, TRIAD_LAND, 2)) >= 0
                && alt_chassis != chassis) {
                if ((proto_cost(alt_chassis, weapon, armor, ability, reactor) <= reactor + 5
                     && proto_cost(alt_chassis, weapon, armor, ability, reactor) <= proto_cost(chassis, weapon, armor, ability, reactor) + (faction_id & 1) + 1)
                    || proto_cost(alt_chassis, weapon, armor, ability, reactor) <= proto_cost(chassis, weapon, armor, ability, reactor)) {
                    chassis = alt_chassis;
                    faster = true;
                }
            }
            better_armor = armor_budget(faction_id, Armor[armor].defense_rating + 1);
            if (Armor[better_armor].defense_rating > Armor[armor].defense_rating
                && proto_cost(chassis, weapon, better_armor, ability, reactor)
                   <= proto_cost(chassis, weapon, armor, ability, reactor)) {
                armor = better_armor;
            } else if (!faster) {
                break;
            }
        }
        if (!has_tech(Ability[abil_bit(ABL_ARTILLERY)].preq_tech, faction_id)
            || has_tech(Rules.tech_two_spec_abilities, faction_id)) {
            add_abil(&ability, ABL_BLINK_DISPLACER, faction_id);
            add_abil(&ability, ABL_ANTIGRAV_STRUTS, faction_id);
            add_abil(&ability, ABL_DEEP_RADAR, faction_id);
            proto_cost(chassis, weapon, better_armor, ability, reactor);
        }
        if (has_tech(Ability[abil_bit(ABL_ARTILLERY)].preq_tech, faction_id)) {
            ability |= ABL_ARTILLERY;
        }
        if (Weapon[weapon].offense_rating >= Armor[armor].defense_rating * 3
            || has_tech(Ability[abil_bit(ABL_ARTILLERY)].preq_tech, faction_id)
            || DesignConsiderOverride) {
            propose_proto(faction_id, chassis, weapon, armor, ability, reactor, 1, 0);
        }
    }
    chassis = pick_chassis(faction_id, TRIAD_LAND, 3);
    if (chassis >= 0 || (chassis = pick_chassis(faction_id, TRIAD_LAND, 2)) >= 0) {
        ability = 0;
        armor = armor_budget(faction_id, 1);
        int round;
        int budget = 0;
        int k = 2;
        do {
            k += 2;
            budget++;
        } while (k / reactor_power <= PlayersData[faction_id].ai_power + reactor * 2
                 + PlayersData[faction_id].ai_fight + 3);
        if (is_human(faction_id)) {
            budget = 99;
        }
        weapon = weapon_budget(faction_id, budget, 0);
        for (round = 0; round < 100; round++) {
            if (Armor[armor].defense_rating >= Weapon[weapon].offense_rating - 1) {
                break;
            }
            int cost = Weapon[weapon].offense_rating / 2;
            if (cost <= Armor[armor].defense_rating + 1) {
                cost = Armor[armor].defense_rating + 1;
            }
            int better_armor = armor_budget(faction_id, cost);
            if (Armor[better_armor].defense_rating <= Armor[armor].defense_rating
                || proto_cost(chassis, weapon, better_armor, ability, reactor)
                   > PlayersData[faction_id].ai_power + PlayersData[faction_id].ai_fight
                     + reactor + 3) {
                break;
            }
            armor = better_armor;
        }
        if (war_score && war_score > 3 - PlayersData[faction_id].ai_fight
            && PlayersData[faction_id].ai_fight >= 0 && !is_human(faction_id)) {
            add_abil(&ability, ABL_NERVE_GAS, faction_id);
        }
        if (GameDataStateStorage.fields_.council_resolution_count_[8]
            > GameDataStateStorage.fields_.council_resolution_count_[9]
            && PlayersData[faction_id].ai_power + PlayersData[faction_id].ai_fight > 0) {
            if (is_human(faction_id)) {
                goto skip_empath4;
            }
            add_abil(&ability, ABL_NERVE_GAS, faction_id);
        }
        if (!is_human(faction_id) && threats & 2) {
            add_abil(&ability, ABL_EMPATHIC, faction_id);
        }
skip_empath4:
        if (Players[faction_id].rule_morale < 0
            || (PlayersData[faction_id].ai_fight > 0
                && PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::INDUSTRY] >= 0)
            || (PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::MORALE] < 0
                && !is_human(faction_id))) {
            add_abil(&ability, ABL_TRAINED, faction_id);
        }
        if (has_tech(Ability[abil_bit(ABL_ANTIGRAV_STRUTS)].preq_tech, faction_id)
            && (!ability || has_tech(Rules.tech_two_spec_abilities, faction_id))) {
            ability |= ABL_ANTIGRAV_STRUTS;
        }
        propose_proto(faction_id, chassis, weapon, armor, ability, reactor, 4, 0);
    }
    ability = 0;
    chassis = pick_chassis(faction_id, TRIAD_LAND, 3);
    if (chassis < 0) {
        chassis = pick_chassis(faction_id, TRIAD_LAND, 2);
        if (chassis < 0) {
            chassis = pick_chassis(faction_id, TRIAD_LAND, 1);
            add_abil(&ability, ABL_ANTIGRAV_STRUTS, faction_id);
        }
    }
    if (chassis >= 0) {
        if (Weapon[weapon_budget(faction_id, 2, 0)].offense_rating >= 2
            || DesignConsiderOverride
            || (has_tech(Ability[abil_bit(ABL_EMPATHIC)].preq_tech, faction_id)
                && !is_human(faction_id))) {
            weapon = weapon_budget(faction_id, 1, 0);
            armor = armor_budget(faction_id, 1);
            if (has_tech(Ability[abil_bit(ABL_EMPATHIC)].preq_tech, faction_id)
                && !is_human(faction_id)) {
                ability |= ABL_EMPATHIC;
                if (has_tech(Rules.tech_two_spec_abilities, faction_id)) {
                    if (has_tech(Ability[abil_bit(ABL_ANTIGRAV_STRUTS)].preq_tech, faction_id)) {
                        ability |= ABL_ANTIGRAV_STRUTS;
                    } else if (has_tech(Ability[abil_bit(ABL_TRAINED)].preq_tech, faction_id)) {
                        ability |= ABL_TRAINED;
                    }
                }
            } else {
                if (!ability) {
                    add_abil(&ability, ABL_DEEP_RADAR, faction_id);
                    if (!ability) {
                        add_abil(&ability, ABL_CLOAKED, faction_id);
                    }
                }
                if (has_tech(Ability[abil_bit(ABL_ANTIGRAV_STRUTS)].preq_tech, faction_id)) {
                    if (has_tech(Rules.tech_two_spec_abilities, faction_id)) {
                        ability |= ABL_ANTIGRAV_STRUTS;
                    } else {
                        ability = ABL_ANTIGRAV_STRUTS;
                    }
                }
            }
            propose_proto(faction_id, chassis, weapon, armor, ability, reactor,
                          (ability & ABL_TRANCE) ? 19 : 10, 0);
        }
    }
    chassis = pick_chassis(faction_id, TRIAD_SEA, 0);
    if (chassis >= 0) {
        ability = 0;
        if (PlayersData[faction_id].unk_66 > PlayersData[faction_id].unk_106
            && PlayersData[faction_id].unk_66 > PlayersData[faction_id].unk_105
            && !is_human(faction_id)) {
            add_abil(&ability, ABL_AAA, faction_id);
        }
        add_abil(&ability, ABL_DEEP_RADAR, faction_id);
        add_abil(&ability, ABL_AAA, faction_id);
        if (threats & 1
            && has_tech(Ability[abil_bit(ABL_TRANCE)].preq_tech, faction_id)
            && (!ability || has_tech(Rules.tech_two_spec_abilities, faction_id))) {
            ability |= ABL_TRANCE;
        }
        if (has_tech(Ability[abil_bit(ABL_AAA)].preq_tech, faction_id)
            && !(ability & ABL_AAA) && bit_count(ability) < 2
            && (!ability || has_tech(Rules.tech_two_spec_abilities, faction_id))) {
            ability |= ABL_AAA;
        }
        if (has_tech(Ability[abil_bit(ABL_DEEP_RADAR)].preq_tech, faction_id)
            && !(ability & ABL_DEEP_RADAR) && bit_count(ability) < 2
            && (!ability || has_tech(Rules.tech_two_spec_abilities, faction_id))) {
            ability |= ABL_DEEP_RADAR;
        }
        weapon = weapon_budget(faction_id, 1, 0);
        armor = armor_budget(faction_id, 1);
        for (int round = 0; round < 100; round++) {
            int next = weapon;
            BOOL upgraded = false;
            for (int k = 1; k < 4 && next == weapon; k++) {
                next = weapon_budget(faction_id, Weapon[weapon].offense_rating + k, 0);
            }
            if (next != weapon) {
                int margin = reactor * 2 - reactor / 2;
                if (margin < 5) {
                    margin = 5;
                }
                if (proto_cost(chassis, next, armor, 0, reactor)
                    <= PlayersData[faction_id].ai_power + PlayersData[faction_id].ai_growth
                       + margin) {
                    weapon = next;
                    upgraded = true;
                }
            }
            int next_armor = armor;
            for (int step = 1; step < 4 && next_armor == armor; step++) {
                next_armor = armor_budget(faction_id, Armor[armor].defense_rating + step);
            }
            if (next_armor != armor
                && Armor[next_armor].defense_rating > Armor[armor].defense_rating) {
                int margin = 5;
                if (reactor * 2 - reactor / 2 >= 5) {
                    margin = reactor * 2 - reactor / 2;
                }
                if (proto_cost(chassis, weapon, next_armor, ability, reactor)
                    <= PlayersData[faction_id].ai_power + PlayersData[faction_id].ai_growth
                       + margin) {
                    armor = next_armor;
                    continue;
                }
            }
            if (!upgraded) {
                break;
            }
        }
        if (is_human(faction_id)) {
            weapon = weapon_budget(faction_id, 99, 0);
        }
        if (Weapon[weapon].offense_rating >= 1) {
            propose_proto(faction_id, chassis, weapon, armor, ability, reactor, 6, 0);
        }
        if (!is_human(faction_id)) {
            weapon = weapon_budget(faction_id, 1, 0);
            armor = armor_budget(faction_id, 99);
            ability = 0;
            add_abil(&ability, ABL_AAA, faction_id);
            if (has_tech(Ability[abil_bit(ABL_DEEP_RADAR)].preq_tech, faction_id)
                && has_tech(Rules.tech_two_spec_abilities, faction_id)) {
                ability |= ABL_DEEP_RADAR;
            }
            if (Armor[armor].defense_rating > 1) {
                propose_proto(faction_id, chassis, weapon, armor, ability, reactor, 0x15, 0);
            }
        }
    }
    chassis = pick_chassis(faction_id, TRIAD_SEA, 0);
    if (chassis >= 0) {
        ability = 0;
        armor = armor_budget(faction_id, 1);
        weapon = weapon_budget(faction_id, -7, 0);
        if (PlayersData[faction_id].unk_66 > PlayersData[faction_id].unk_106
            && PlayersData[faction_id].unk_66 > PlayersData[faction_id].unk_105) {
            armor = armor_budget(faction_id, 99);
        }
        propose_proto(faction_id, chassis, weapon, armor, ability, reactor, 7, 0);
        ability = 0;
        if (chassis != CHSI_FOIL && has_tech(Chassis[CHSI_FOIL].preq_tech, faction_id)) {
            chassis = CHSI_FOIL;
        }
        armor = armor_budget(faction_id, 1);
        weapon = weapon_budget(faction_id, -9, 0);
        add_abil(&ability, ABL_SUPER_TERRAFORMER, faction_id);
        add_abil(&ability, ABL_FUNGICIDAL, faction_id);
        if (ability == ABL_SUPER_TERRAFORMER
            && has_tech(Ability[abil_bit(ABL_FUNGICIDAL)].preq_tech, faction_id)
            && has_tech(Rules.tech_two_spec_abilities, faction_id)) {
            ability |= ABL_FUNGICIDAL;
        }
        if (weapon >= 0) {
            propose_proto(faction_id, chassis, weapon, armor, ability, reactor, 0xF, 0);
        }
        ability = 0;
        weapon = weapon_budget(faction_id, -8, 0);
        if (weapon >= 0) {
            propose_proto(faction_id, chassis, weapon, armor, ability, reactor, 0x10, 0);
        }
    }
    chassis = pick_chassis(faction_id, TRIAD_AIR, 0);
    if (chassis >= 0) {
        weapon = weapon_budget(faction_id, 99, 0);
        armor = armor_budget(faction_id, 1);
        ability = 0;
        if (PlayersData[faction_id].ranking >= 6
            && (PlayersData[faction_id].ai_wealth || PlayersData[faction_id].ai_tech)
            && (!PlayersData[faction_id].ai_power || PlayersData[faction_id].ai_fight < 1)) {
            add_abil(&ability, ABL_CLEAN_REACTOR, faction_id);
        }
        if (threats & 2) {
            add_abil(&ability, ABL_EMPATHIC, faction_id);
        }
        if (war_score && war_score > 5 - PlayersData[faction_id].ai_fight
            && PlayersData[faction_id].ai_fight >= 0 && !is_human(faction_id)) {
            add_abil(&ability, ABL_NERVE_GAS, faction_id);
        }
        if (GameDataStateStorage.fields_.council_resolution_count_[8]
            > GameDataStateStorage.fields_.council_resolution_count_[9]
            && PlayersData[faction_id].ai_fight + PlayersData[faction_id].ai_power > 0
            && !is_human(faction_id)) {
            add_abil(&ability, ABL_NERVE_GAS, faction_id);
        }
        if (has_tech(Ability[abil_bit(ABL_DEEP_RADAR)].preq_tech, faction_id)
            && (!ability || has_tech(Rules.tech_two_spec_abilities, faction_id))) {
            ability |= ABL_DEEP_RADAR;
        }
        propose_proto(faction_id, chassis, weapon, armor, ability, reactor, 8, 0);
        weapon = weapon_budget(faction_id, 99, 0);
        ability = ABL_AIR_SUPERIORITY;
        if (has_tech(Ability[abil_bit(ABL_DEEP_RADAR)].preq_tech, faction_id)
            && has_tech(Rules.tech_two_spec_abilities, faction_id)) {
            ability |= ABL_DEEP_RADAR;
        } else if (threats & 1
                   && has_tech(Ability[abil_bit(ABL_TRANCE)].preq_tech, faction_id)
                   && has_tech(Rules.tech_two_spec_abilities, faction_id)) {
            ability |= ABL_TRANCE;
        }
        propose_proto(faction_id, chassis, weapon, armor, ability, reactor, 9, 0);
    }
    chassis = pick_chassis(faction_id, TRIAD_AIR, -2);
    if (chassis >= 0) {
        weapon = weapon_budget(faction_id, 99, 0);
        armor = armor_budget(faction_id, 1);
        ability = 0;
        if (PlayersData[faction_id].ranking >= 6
            && (PlayersData[faction_id].ai_wealth || PlayersData[faction_id].ai_tech)
            && (!PlayersData[faction_id].ai_power || PlayersData[faction_id].ai_fight < 1)) {
            add_abil(&ability, ABL_CLEAN_REACTOR, faction_id);
        }
        if (threats & 2) {
            add_abil(&ability, ABL_EMPATHIC, faction_id);
        }
        if (war_score && war_score > 4 - PlayersData[faction_id].ai_fight
            && PlayersData[faction_id].ai_fight >= 0 && !is_human(faction_id)) {
            add_abil(&ability, ABL_NERVE_GAS, faction_id);
        }
        if (GameDataStateStorage.fields_.council_resolution_count_[8]
            > GameDataStateStorage.fields_.council_resolution_count_[9]
            && PlayersData[faction_id].ai_fight + PlayersData[faction_id].ai_power > 0
            && !is_human(faction_id)) {
            add_abil(&ability, ABL_NERVE_GAS, faction_id);
        }
        if (has_tech(Ability[abil_bit(ABL_DEEP_RADAR)].preq_tech, faction_id)
            && (!ability || has_tech(Rules.tech_two_spec_abilities, faction_id))) {
            ability |= ABL_DEEP_RADAR;
        }
        propose_proto(faction_id, chassis, weapon, armor, ability, reactor, 0x11, 0);
    }
    chassis = pick_chassis(faction_id, TRIAD_AIR, -1);
    if (chassis >= 0) {
        armor = armor_budget(faction_id, 1);
        ability = 0;
        if (has_tech(Weapon[WPN_CONVENTIONAL_PAYLOAD].preq_tech, faction_id)) {
            propose_proto(faction_id, chassis, WPN_CONVENTIONAL_PAYLOAD, armor, ability,
                          reactor, 0xD, 0);
        }
        if (has_tech(Weapon[WPN_TECTONIC_PAYLOAD].preq_tech, faction_id)) {
            propose_proto(faction_id, chassis, WPN_TECTONIC_PAYLOAD, armor, ability,
                          reactor, 0x16, 0);
        }
        if (has_tech(Weapon[WPN_FUNGAL_PAYLOAD].preq_tech, faction_id)) {
            propose_proto(faction_id, chassis, WPN_FUNGAL_PAYLOAD, armor, ability,
                          reactor, 0x17, 0);
        }
        for (weapon = 0; weapon < MaxWeaponNum; weapon++) {
            if (has_tech(Weapon[weapon].preq_tech, faction_id)
                && Weapon[weapon].offense_rating >= 99) {
                propose_proto(faction_id, chassis, weapon, armor, ability, reactor, 0xE, 0);
                break;
            }
        }
    }
    ability = 0;
    if (reactor <= 1 && !has_tech(Ability[abil_bit(ABL_SUPER_TERRAFORMER)].preq_tech,
                                  faction_id)) {
        if (has_tech(Ability[abil_bit(ABL_FUNGICIDAL)].preq_tech, faction_id)
            && (chassis = pick_chassis(faction_id, TRIAD_LAND, 1)) >= 0) {
            armor = armor_budget(faction_id, 1);
            ability = ABL_FUNGICIDAL;
            propose_proto(faction_id, chassis, WPN_TERRAFORMING_UNIT, armor, ability,
                          reactor, -1, 0);
        }
    } else {
        chassis = pick_chassis(faction_id, TRIAD_LAND, 2);
        if (chassis >= 0) {
            armor = armor_budget(faction_id, 1);
            if (has_tech(Ability[abil_bit(ABL_SUPER_TERRAFORMER)].preq_tech, faction_id)) {
                ability |= ABL_SUPER_TERRAFORMER;
            }
            if (has_tech(Ability[abil_bit(ABL_FUNGICIDAL)].preq_tech, faction_id)
                && (!ability || has_tech(Rules.tech_two_spec_abilities, faction_id))) {
                ability |= ABL_FUNGICIDAL;
            }
            propose_proto(faction_id, chassis, WPN_TERRAFORMING_UNIT, armor, ability,
                          reactor, -1, 0);
        }
    }
}




// ORIGINAL: 0x00583CB0 ?design_new_veh@@YAXHH@Z 0x00583CB0-0x00583CC7 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Retired exact seed git:5da70027:src/recovered/00583cb0.cpp preserves the23byte original wrapper and direct43E9A0 exec call; designwin.cpp owns the actual71F2B0 singleton and corresponding exec identity."], "reason": "Dispatch the two original design arguments through the genuine source-owned TutWinDesWindow singleton and its DesignWin::exec method, not an original-address receiver."}
void __cdecl design_new_veh(int faction_id, int prototype_id) {
    TutWinDesWindow.exec(faction_id, prototype_id);
}

static int workshop_signed_bits(unsigned int bits) {
    return bits <= 0x7fffffffu ? static_cast<int>(bits)
        : -1 - static_cast<int>(0xffffffffu - bits);
}

static int workshop_scaled_dimension(int dimension, int numerator,
                                     int denominator) {
    const int product = workshop_signed_bits(
        static_cast<unsigned int>(dimension) *
        static_cast<unsigned int>(numerator));
    return product / denominator;
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00583CD0 ?hq_list_draw@@YAXPAUGraphicWin@@HHHHH@Z 0x00583CD0-0x00583E2A BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned native583CD0..583E2A: signed scaling and centering, active bit0, shared zoom68F0F4 initial-6 and mutable faction93FC48, fake vehicle5C1D50, obsolete order byte11, Buffer secondary base444, veh_draw5594F0 and unity scale epilogue. Retained src/recovered/units/00583cd0.cpp; recovery/tasks/designwin-owner-coverage.json coupled callback authorization; no original-address aliases."], "reason": "Recover the workshop prototype preview through actual GraphicWin Buffer inheritance, VehPrototype and Veh owners, source-owned zoom/faction storage and sprite scaling. Preserve the seed identity and original span; catalogue struct spelling differs from the actual GraphicWin class symbol. No matching claim is granted."}
// symbol    ?hq_list_draw@@YAXPAVGraphicWin@@HHHHH@Z
void __cdecl hq_list_draw(GraphicWin *window, int prototype, int flags,
                          int x, int y, int height) {
    const int zoom = WorkshopPreviewZoom;
    const int scale = MapWinCaptionScale;
    x += (flags & 1) ? (scale > 0 ? 132 * scale / MapWinCaptionDivisor
                                  : 132 * (WorkshopPreviewZoom + 16) / 16)
                     : 0;
    if ((VehPrototypes[prototype].flags & 1) == 0) return;

    const int draw_height = scale > 0 ? 75 * scale / MapWinCaptionDivisor
                                      : 75 * (WorkshopPreviewZoom + 16) / 16;
    y += (height - draw_height) / 2;
    if (scale > 0) {
        SpriteScaleNumerator = scale;
        SpriteScaleDenominator = MapWinCaptionDivisor;
    } else {
        SpriteScaleDenominator = 16;
        SpriteScaleNumerator = WorkshopPreviewZoom + 16;
    }
    const int vehicle = veh_fake(prototype, WorkshopFaction);
    if (VehPrototypes[prototype].obsolete_factions & (1 << WorkshopFaction))
        Vehs[vehicle].order = 1;
    veh_draw(window ? static_cast<Buffer *>(window) : 0,
             vehicle, x, y, zoom, 0x2044, 0);
    // The original publishes unity, rather than restoring the previous scale.
    SpriteScaleNumerator = 1;
    SpriteScaleDenominator = 1;
}

extern uint32_t UpgradePromptAnswered;

// ORIGINAL: 0x00583E30 ?design_workshop@@YAHHH@Z 0x00583E30-0x00584B4B;0x0065FFD0-0x00660239
// RECOVERY_BINDING: v1 {"evidence": ["Pinned583E30..584B4B and65FFD0..660239; preserved git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/00583e30.cpp; mode-zero list bypass, nonzero64-prototype scan,128 sorted list rows, ordered0/2/3/4/default action chain and native Popup cleanup funclets.", "Native5842DB tests priorflags8 before setting200; upgrade scan skips selected/inactive rows, passes faction/replacement/selected/zero, reports only scan exhaustion and rebuilds the entire list. Rename5845AE copies shared prompt result into the prototype name. Callback60BCDF return is discarded; real hq_list_draw owns preview behavior.", "Existing DesignWin/Popup/Dialog owners, WorkshopFaction/PreviewZoom and source-owned prototype/player storage; recovery/tasks/designwin-owner-coverage.json. Existing NetMsgPrimary fixed-address facade and unfinished callee behavior remain separate dependencies; no byte, runtime or game acceptance claim."], "reason": "Recover the complete production workshop dependency through real lexical Popup lifetime, typed dialog and prototype ownership, native list/action flow and source-owned preview callback. Preserve its original identity, seed and historical obligations instead of a link-only placeholder."}
// TRIED: the two "layout" division computations (0x583F94-0x584054) simplified the signed-rounding cdq/and/sar idiom to a plain shift for the likely-positive case; the upgrade-candidate scan (0x5843D5-0x5843EB) approximated the per-slot style comparison rather than tracing every register precisely.
int __cdecl design_workshop(int faction_id, int mode) {
    Popup popup;
    char number[16];

    // Native mode zero skips the automatic-design search and enters the list.
    if (mode != 0) {
        int best_score = 0;
        int best_prototype = -1;
        const int faction_first = faction_id * MaxVehProtoFactionNum;
        for (int slot = 0; slot < MaxVehProtoFactionNum; ++slot) {
            const int prototype_id = faction_first + slot;
            const VehPrototype &prototype = VehPrototypes[prototype_id];
            int score;
            if ((prototype.flags & PROTO_ACTIVE) != 0) {
                if ((prototype.obsolete_factions & (1 << faction_id)) == 0)
                    continue;
                if (PlayersData[faction_id].proto_id_active[prototype_id] != 0)
                    continue;
                score = 0x384 - PlayersData[faction_id].proto_id_queue[prototype_id];
            } else {
                score = 0x3e7;
            }
            if (score > best_score) {
                best_score = score;
                best_prototype = prototype_id;
            }
        }
        if (best_prototype >= 0)
            return TutWinDesWindow.exec(faction_id, best_prototype);
    }

    for (;;) {
        proto_sort(faction_id);
        popups_medium();
        popup.start(PopupStartCaption, "DESIGNSHOP", -1, 0, 2, 0);
        WorkshopFaction = faction_id;
        PopupModalState = 5;
        popup.dialogs().set_callbacks(&hq_list_draw, 0, 0);

        const int caption_scale = MapWinCaptionScale;
        const int caption_divisor = MapWinCaptionDivisor;
        const int metric = WorkshopPreviewZoom;
        int width = caption_scale > 0
            ? workshop_scaled_dimension(132, caption_scale, caption_divisor)
            : workshop_scaled_dimension(132, workshop_signed_bits(static_cast<unsigned int>(metric) + 16u), 16);
        int height = caption_scale > 0
            ? workshop_scaled_dimension(75, caption_scale, caption_divisor)
            : workshop_scaled_dimension(75, workshop_signed_bits(static_cast<unsigned int>(metric) + 16u), 16);
        Dialog &dialog = static_cast<Dialog &>(popup.dialogs());
        dialog.field_34_ = workshop_signed_bits(static_cast<unsigned int>(width) * 2u + 3u);
        dialog.field_48_ = height;
        dialog.field_50_ = 8;

        for (int row = 0; row < 128; ++row) {
            const int prototype_id = ProtoSortIds[row];
            const int prototype_faction = prototype_id / MaxVehProtoFactionNum;
            if (prototype_faction != 0 && prototype_faction != faction_id)
                continue;
            const VehPrototype &prototype = VehPrototypes[prototype_id];
            const bool active = (prototype.flags & PROTO_ACTIVE) != 0;
            if (active && prototype_faction != 0
                && !has_tech(prototype.preq_tech, faction_id))
                continue;

            StringTemp[0] = 0;
            if (active) {
                const bool obsolete =
                    (prototype.obsolete_factions & (1 << faction_id)) != 0;
                if (obsolete) strcat(StringTemp, "-");
                strcat(StringTemp, prototype.veh_name);
                strcat(StringTemp, "|");
                if (obsolete) {
                    strcat(StringTemp, label_get(0x368 / 4));
                    strcat(StringTemp, ", ");
                }
                if ((prototype.flags & 4) == 0 && prototype_id >= 64) {
                    strcat(StringTemp, label_get(0x2e4 / 4));
                    strcat(StringTemp, ", ");
                }
                strcat(StringTemp, label_get(0x160 / 4));
                strcat(StringTemp, ": ");
                _itoa(workshop_signed_bits(static_cast<unsigned int>(cost_factor(faction_id, 1, -1)) * prototype.cost),
                      number, 10);
                strcat(StringTemp, number);
                strcat(StringTemp, ", ");
                say_stats(StringTemp, prototype_id, 0);
            } else if (prototype_faction == 0) {
                strcat(StringTemp, "- ");
                strcat(StringTemp, label_get(0x1b8 / 4));
                strcat(StringTemp, " -");
            } else {
                continue;
            }
            popup.dialogs().item(StringTemp, prototype_id);
        }

        const int selected = popup.BasePop::exec(0, 0);
        popups_normal();
        g_FX.play(0xf);
        const uint32_t action = popup.input_poll_state();
        if (action == 0)
            return -1;
        if (action == 2) {
            char *prototype_name = VehPrototypes[selected].veh_name;
            if (X_pop_ask("DESIGNNAME", 31, prototype_name, 0, 0) != 0)
                return -1;
            strcpy(prototype_name, ParseStrBuffer[0].str);
            synch_proto(selected);
            return -1;
        }
        if (action == 3) {
            VehPrototype &prototype = VehPrototypes[selected];
            if ((prototype.obsolete_factions & (1 << faction_id)) != 0) {
                if ((prototype.flags & 0x8) != 0)
                    prototype.flags |= 0x200;
                prototype.flags |= 8;
            } else {
                prototype.flags &= static_cast<uint16_t>(~(0x200 | 8));
            }
            prototype.obsolete_factions ^= static_cast<uint8_t>(1 << faction_id);
            synch_obs(selected);
            continue;
        }
        if (action == 4) {
            if (NetDaemonNet.lock(2, -1, -1, 0, -1, -1) != 0)
                continue;
            UpgradePromptAnswered = 0;
            int replacement = -1;
            const VehPrototype &old = VehPrototypes[selected];
            int scanned = 0;
            for (int slot = 0; slot < MaxVehProtoFactionNum; ++slot) {
                scanned = slot;
                const int candidate_id = faction_id * MaxVehProtoFactionNum + slot;
                if (candidate_id == selected) continue;
                const VehPrototype &candidate = VehPrototypes[candidate_id];
                if ((candidate.flags & PROTO_ACTIVE) == 0) continue;
                if ((candidate.obsolete_factions & (1 << faction_id)) != 0)
                    continue;
                if ((candidate.flags & 4) == 0) continue;
                if (candidate.plan != old.plan || candidate.unk_1 != old.unk_1)
                    continue;
                if (Chassis[candidate.chassis_id].triad
                    == Chassis[old.chassis_id].triad) {
                    replacement = candidate_id;
                    break;
                }
            }
            if (replacement < 0) scanned = MaxVehProtoFactionNum;
            if (replacement >= 0)
                upgrade_prototype(faction_id, replacement, selected, 0);
            NetDaemonNet.unlock();
            if (scanned >= MaxVehProtoFactionNum) {
                const char *icon = (Players[faction_id].rule_flags & 0x80)
                    ? "alunit_sm.pcx" : "unit_sm.pcx";
                popp(ScriptPopupFile, "NOUPGRADE", 0, icon, 0);
            }
            continue;
        }
        if (selected < MaxVehProtoFactionNum) {
            NetMsgPrimary.pop("DESIGNNO", 0x1388, 0, 0);
            return -1;
        }
        VehPrototypes[selected].flags &= static_cast<uint16_t>(~0x80);
        return TutWinDesWindow.exec(faction_id, selected);
    }
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
