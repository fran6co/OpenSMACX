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
#include "alphamenu.h"
#include "alpha.h"
#include "strings.h"
#include "text.h"
#include "terraforming.h"
#include "technology.h"
#include "veh.h"

#include "console.h"
#include "game.h"
#include "maininterface.h"
#include "palette.h"
#include "pulldown.h"
#include "wave.h"

// AlphaMenu's four foreground/shadow pairs are the words at indices 30-37
// in the shared text-colour run built by textcolor_init (palette.cpp).
extern int TextColourTable[195];
extern int WindowAnimationFrameDelay;
extern Wave g_ALPHAMENU_WAVE;

namespace {

int animation_frame_x() {
  return WinScreenWidth == 0x320 ? 0 : (WinScreenWidth - 0x400) / 2;
}

void wait_for_animation_frame(DWORD started) {
  while (timeGetTime() - started < static_cast<DWORD>(WindowAnimationFrameDelay)) {
    do_sound();
  }
}

}  // namespace

// The compiler destroys sprites_[2..0], sprite1_, buffer_, then the Menu
// base in the order observed at 0x0051DC10. Keep this as C++ ownership,
// not a manual walk over the object bytes.
// ORIGINAL: 0x0051DC10 ??1AlphaMenu@@QAE@XZ 0x0051DC10-0x0051DC86;0x0065D220-0x0065D24E
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/0051dc10.cpp"], "reason": "C++ AlphaMenu owns Menu, Buffer and Sprite members whose reverse lifetime order follows the original destructor and exception-unwind trace."}
// TRIED: computed that AlphaMenu : public Menu (Menu ends at exactly 0xB64, where Buffer buffer_ starts, which ends at exactly 0x10EC, where Sprite sprite_ starts, ending at 0x1118 where Sprite sprites_[3] starts) - a real member layout that would let the SEH prologue emerge automatically, but AlphaMenu is the fixed scaffold class and cannot carry real members here. Explicit dtor calls at the right offsets/order compile and are structurally faithful; the SEH frame (push -1/handler, unwind funclets) is not reproduced - same open item as 0x00629D60 (Flic::Flic) and 0047acf0.cpp.
// symbol    ??1AlphaMenu@@UAE@XZ
AlphaMenu::~AlphaMenu() {}

// ORIGINAL: 0x00460DD0 ?check@MainMenu@@QAEXH@Z 0x00460DD0-0x0046207E
// RECOVERY_BINDING: v1 {"evidence": ["historical retired seed src/unrecovered/00460dd0.cpp", "recovery/tasks/alphamenu-check-owner-oct5.json", "Console embedded member +22A2C, actual constructor and by-name callers", "complete pinned 460DD0..46207E CFG and fresh native current owner symbol; actual-source bounded trace/mutant controls"], "reason": "Recover complete menu policy in the actual Console::alpha_menu_ subobject. Preserve historical MainMenu catalogue identity separately from the observed current AlphaMenu symbol; genuine source ownership corrects the old seed hypothesis. No current byte or runtime acceptance claim."}
// symbol    ?check@AlphaMenu@@QAEXH@Z
// TRIED: time-boxed - roughly the first quarter of the function (a long sequence of Menu::show_menu_item/hide_menu_item/ enable_menu_item gates against game-option and tech flags, transcribed with goto-labelled control flow matching the disassembly's branch targets) is faithful; the difficulty-table-driven remainder (several hundred more show/hide/enable calls) is a deliberate stub. MISMATCH #2, well past the prologue.
void AlphaMenu::check(int veh_id) {
    if ((GameDataStateStorage.fields_.game_more_preferences_ & 0x200000u) != 0) {
        show_menu_item(0, 0x52);
        hide_menu_item(0, 0x51);
    } else {
        show_menu_item(0, 0x51);
        hide_menu_item(0, 0x52);
    }

    if (IsMultiplayerNet || IsMultiplayerPBEM) {
        hide_menu_item(0, 0x5C);
        goto difficulty_gate;
    }
    if ((GameDataStateStorage.fields_.game_rules_ & 0x400u) != 0) {
        goto difficulty_gate;
    }
    show_menu_item(0, 0x5C);
    show_menu_item(0, 0x5A);
    goto expansion_gate;

difficulty_gate:
    if ((GameDataStateStorage.fields_.game_state_ & 0x1000u) != 0) {
        show_menu_item(0, 0x5A);
        goto expansion_gate;
    }
    hide_menu_item(0, 0x5A);

expansion_gate:
    if ((GameDataStateStorage.fields_.game_more_preferences_ & 0x200000u) != 0) {
        const int general_items[] = {
            0x205, 0x206, 0x207, 0x208, 0x20C, 0x215, 0x211, 0x212
        };
        const int terraforming_items[] = {
            0x31B, 0x324, 0x306, 0x304, 0x319, 0x31A, 0x317, 0x31F,
            0x320, 0x307, 0x322
        };
        const int scenario_items[] = {
            0x9F8, 0x9F9, 0x9F6, 0x9F5, 0x9F4, 0x9F3, 0x9F1, 0x9F0,
            5, 6, 7
        };
        for (unsigned int general_index = 0;
             general_index != sizeof(general_items)/sizeof(general_items[0]);
             ++general_index)
            show_menu_item(2, general_items[general_index]);
        for (unsigned int terraforming_index = 0;
             terraforming_index != sizeof(terraforming_items)/sizeof(terraforming_items[0]);
             ++terraforming_index)
            show_menu_item(3, terraforming_items[terraforming_index]);
        for (unsigned int scenario_index = 0;
             scenario_index != sizeof(scenario_items)/sizeof(scenario_items[0]);
             ++scenario_index)
            show_menu_item(9, scenario_items[scenario_index]);
    }

    if ((GameDataStateStorage.fields_.game_state_ & 0x10u) == 0 && ScenarioSave.objective_req_victory >= 0x3E8 &&
        static_cast<int32_t>(ScenarioSave.objectives_sudden_death_victory) >= 0x3E8) {
        hide_menu_item(1, 0x115);
    } else {
        show_menu_item(1, 0x115);
    }

    if (IsMultiplayerNet && (GameDataStateStorage.fields_.game_state_ & 2u) != 0 &&
        ConsoleFactionsWaitingMask != 0 && ConsoleFactionsWaitingMask != 0xFF) {
        hide_menu_item(8, 0x807);
        show_menu_item(8, 0x808);
    } else {
        show_menu_item(8, 0x807);
        hide_menu_item(8, 0x808);
    }

    if ((GameDataStateStorage.fields_.game_state_ & 0xC0u) != 0 && (GameDataStateStorage.fields_.net_turn_flags_ & 8u) != 0) {
        hide_menu_item(1, 0x105);
    } else {
        show_menu_item(1, 0x105);
    }

    if ((GameDataStateStorage.fields_.game_rules_ & 0x200u) != 0) {
        show_menu_item(1, 0x106);
        show_menu_item(1, 0x107);
    } else {
        show_menu_item(1, 0x107);
        show_menu_item(1, 0x106);
    }
    hide_menu_item(1, 0x106);

    if (ConsoleGlobal.cursor_active_ != 0) {
        show_menu_item(2, 0x201);
        hide_menu_item(2, 0x200);
    } else {
        show_menu_item(2, 0x200);
        hide_menu_item(2, 0x201);
    }

    if (ConsoleGlobal.cursor_active_ != 0 && veh_id >= 0) {
        for (int unit_item_id = 0x300; unit_item_id <= 0x324; ++unit_item_id) {
            enable_menu_item(3, unit_item_id);
        }
    // The native branch at 0x4611C2 sends cursor-inactive and negative-ID
    // calls directly to 0x461E48. Keep the following unit-dependent region
    // inside this arm; its final join is 0x461E78.
// Native-derived continuation for AlphaMenu::check after its first known
// prefix. This block corresponds to original 0x004611D5..0x00461E47 and is
// dominated by the native cursor/vehicle guards.
const Veh &selected_veh = Vehs[veh_id];
if (has_abil(selected_veh.proto_id, ABL_DROP_POD)) {
    show_menu_item(3, 0x309);
} else {
    hide_menu_item(3, 0x309);
}

// Native 0x00461224-0x00461273 asks for the owner of this unit's map tile and
// then reads PlayersData[local].diplo_treaties[owner]. The byte at PlayersData
// base+0x18 is the same treaty member declared in faction.h; the native
// arithmetic is local*0x833+owner. It shows 0x31F only when bit 0x08 is set
// and bit 0x10 is clear. The later prototype predicate can overwrite this
// intermediate menu state, so keep this mutation in its original position.
const int selected_tile_owner = whose_territory(
    ConsoleGlobal.faction_id_, selected_veh.x, selected_veh.y, nullptr, 0);
if (selected_tile_owner >= 0 &&
    selected_tile_owner != ConsoleGlobal.faction_id_) {
    const uint32_t treaty = PlayersData[ConsoleGlobal.faction_id_]
                                .diplo_treaties[selected_tile_owner];
    if ((treaty & DTREATY_COMMLINK) != 0 &&
        (treaty & DTREATY_VENDETTA) == 0) {
        show_menu_item(3, 0x31F);
    }
}

const uint32_t selected_proto_id = selected_veh.proto_id;
if (static_cast<int32_t>(selected_proto_id) < 0x40 &&
    (get_proto_offense_rating(selected_proto_id) < 0 ||
     selected_proto_id == BSC_SPORE_LAUNCHER)) {
    show_menu_item(3, 0x31F);
} else {
    hide_menu_item(3, 0x31F);
}

const VehPrototype &selected_proto = VehPrototypes[selected_proto_id];
if (Chassis[selected_proto.chassis_id].missile != 0 &&
    (selected_proto.weapon_id == 0x18 || selected_proto.weapon_id == 0x19)) {
    show_menu_item(3, 0x30A);
} else {
    hide_menu_item(3, 0x30A);
}

const int selected_base = base_at(selected_veh.x, selected_veh.y);
if (selected_base >= 0 && has_fac_built(0x21, selected_base)) {
    show_menu_item(3, 0x310);
} else {
    hide_menu_item(3, 0x310);
}

if (veh_cargo(veh_id) != 0 || has_abil(selected_proto_id, ABL_CARRIER)) {
    show_menu_item(3, 0x31D);
} else {
    hide_menu_item(3, 0x31D);
}

if (Chassis[selected_proto.chassis_id].triad == TRIAD_SEA) {
    show_menu_item(3, 0x306);
    show_menu_item(3, 0x304);
} else {
    hide_menu_item(3, 0x306);
    hide_menu_item(3, 0x304);
}

if (can_arty(selected_proto_id, 1)) {
    show_menu_item(3, 0x305);
} else {
    hide_menu_item(3, 0x305);
}

if (selected_proto.plan == 0x0A) {
    show_menu_item(3, 0x308);
    enable_menu_item(3, 0x308);
} else {
    hide_menu_item(3, 0x308);
    disable_menu_item(3, 0x308);
}

// The image tests plan 8 separately after the plan-10 branch. Preserve the
// native sequential state transitions rather than collapsing them to a final
// enable/show bit for each item.
if (selected_proto.plan == 0x08) {
    enable_menu_item(3, 0x300);
    enable_menu_item(3, 0x301);
    show_menu_item(3, 2);
    const Map *const selected_tile = map_loc(selected_veh.x, selected_veh.y);
    const int tile_owner = (selected_tile->bit & 1u) != 0
        ? static_cast<int>(selected_tile->val2 & 0x0Fu) : -1;
    if (tile_owner >= 0 && tile_owner < 8) {
        show_menu_item(3, 0x301);
        hide_menu_item(3, 0x300);
    } else {
        hide_menu_item(3, 0x301);
        show_menu_item(3, 0x300);
    }
} else {
    hide_menu_item(3, 2);
    disable_menu_item(3, 0x300);
    disable_menu_item(3, 0x301);
    hide_menu_item(3, 0x300);
    hide_menu_item(3, 0x301);
}

const Map *const selected_tile_for_action =
    map_loc(selected_veh.x, selected_veh.y);
const int action_tile_owner = (selected_tile_for_action->bit & 1u) != 0
    ? static_cast<int>(selected_tile_for_action->val2 & 0x0Fu) : -1;
if (get_proto_offense_rating(selected_proto_id) != 0 &&
    action_tile_owner == static_cast<int>(selected_veh.faction_id) &&
    Chassis[selected_proto.chassis_id].triad != TRIAD_AIR) {
    show_menu_item(3, 0x302);
    enable_menu_item(3, 0x302);
} else {
    disable_menu_item(3, 0x302);
    hide_menu_item(3, 0x302);
}

// Native continuation beginning at 0x00461672.
if (Chassis[selected_proto.chassis_id].triad != TRIAD_LAND) {
    show_menu_item(3, 0x320);
    show_menu_item(3, 0x311);
} else {
    hide_menu_item(3, 0x320);
    hide_menu_item(3, 0x311);
}

if (selected_proto.plan != 9) {
    bool sea_order_labels = false;
    const uint8_t selected_triad = Chassis[selected_proto.chassis_id].triad;
    if (selected_triad == TRIAD_SEA) {
        sea_order_labels = true;
    } else if (selected_triad == TRIAD_AIR) {
        const Map *const tile = map_loc(selected_veh.x, selected_veh.y);
        sea_order_labels = (tile->climate & 0xE0u) < 0x60u;
    }

    for (int terraform_id = 0; terraform_id != 19; ++terraform_id) {
        StringTemp[0] = '\0';
        const int order_name = static_cast<int>(reinterpret_cast<intptr_t>(
            sea_order_labels ? Order[terraform_id + 4].order_sea
                             : Order[terraform_id + 4].order));
        strcat(StringTemp, StringTable.get(order_name));
        strcat(StringTemp, "|");
        strcat(StringTemp, StringTable.get(
            static_cast<int>(reinterpret_cast<intptr_t>(
                Terraforming[terraform_id].shortcuts))));
        rename_menu_item(9, 0x900 + terraform_id, StringTemp);

        if (terrain_avail(terraform_id, sea_order_labels,
                          ConsoleGlobal.faction_id_)) {
            enable_menu_item(9, 0x900 + terraform_id);
            if (terraform_id == 18) show_menu_item(9, 1);
        } else {
            disable_menu_item(9, 0x900 + terraform_id);
            if (terraform_id == 18) hide_menu_item(9, 1);
        }
    }

    disable_menu_item(9, 0x9F8);
    disable_menu_item(9, 0x9F9);
    disable_menu_item(9, 0x9F0);
    disable_menu_item(9, 0x9F1);
    disable_menu_item(9, 0x9F6);
    disable_menu_item(9, 0x9F3);
    disable_menu_item(9, 0x9F4);
    disable_menu_item(9, 0x9F5);
    disable_menu_item(9, 0x9F7);
    disable_menu_item(9, 0x9F2);

    const Map *const tile = map_loc(selected_veh.x, selected_veh.y);
    const int owner = (tile->bit & 1u) != 0
        ? static_cast<int>(tile->val2 & 0x0Fu) : -1;
    if (owner >= 0 && owner < 8) {
        hide_menu_item(3, 0x317);
        show_menu_item(3, 0x316);
    } else {
        show_menu_item(3, 0x317);
        hide_menu_item(3, 0x316);
    }
} else {
    // A plan-9 former gets context-sensitive menu rows. The selector is the
    // same typed terrain choice, but rows 1 and 6 are gated by the actual
    // selected-tile flag before their build-cost labels become actionable.
    bool sea_order_labels = false;
    const uint8_t selected_triad = Chassis[selected_proto.chassis_id].triad;
    if (selected_triad == TRIAD_SEA) {
        sea_order_labels = true;
    } else if (selected_triad == TRIAD_AIR) {
        const Map *const tile = map_loc(selected_veh.x, selected_veh.y);
        sea_order_labels = (tile->climate & 0xE0u) < 0x60u;
    }

    const Map *const selected_tile = map_loc(selected_veh.x, selected_veh.y);
    for (int terraform_id = 0; terraform_id != 19; ++terraform_id) {
        const bool row_action_flag =
            (terraform_id != 1 && terraform_id != 6) ||
            (terraform_id == 1 && (selected_tile->bit & 0x80000000u) != 0) ||
            (terraform_id == 6 && (selected_tile->bit & 0x4u) != 0);
        const bool can_build_here = row_action_flag && terrain_avail(
            terraform_id, sea_order_labels, ConsoleGlobal.faction_id_);

        StringTemp[0] = '\0';
        const int order_name = static_cast<int>(reinterpret_cast<intptr_t>(
            sea_order_labels ? Order[terraform_id + 4].order_sea
                             : Order[terraform_id + 4].order));
        strcat(StringTemp, StringTable.get(order_name));
        if (can_build_here) {
            strcat(StringTemp, " (");
            char turn_count[10];
            const int turns = action_terraform(veh_id, terraform_id, 0) + 1;
            _itoa(turns, turn_count, 10);
            strcat(StringTemp, turn_count);
            strcat(StringTemp, " ");
            strcat(StringTemp, label_get(turns == 1 ? 0x30 : 0x31));
            strcat(StringTemp, ")");
        }
        strcat(StringTemp, "|");
        strcat(StringTemp, StringTable.get(static_cast<int>(reinterpret_cast<intptr_t>(
            Terraforming[terraform_id].shortcuts))));
        const int menu_item = 0x900 + terraform_id;
        rename_menu_item(9, menu_item, StringTemp);

        if (can_build_here) {
            show_menu_item(9, menu_item);
            enable_menu_item(9, menu_item);
            if (terraform_id == 18) show_menu_item(9, 1);
        } else {
            disable_menu_item(9, menu_item);
            hide_menu_item(9, menu_item);
            if (terraform_id == 18) hide_menu_item(9, 1);
        }
    }

    // The native state transitions intentionally occur after the row loop.
    show_menu_item(9, 0x9F8);
    show_menu_item(9, 0x9F9);
    enable_menu_item(9, 0x9F8);
    enable_menu_item(9, 0x9F9);
    hide_menu_item(9, 0x9F7);
    hide_menu_item(9, 0x9F2);

    const int local_faction = ConsoleGlobal.faction_id_;
    const int terrain_row_10 = sea_order_labels
        ? Terraforming[10].preq_tech_sea : Terraforming[10].preq_tech;
    if (has_tech(terrain_row_10, local_faction)) {
        enable_menu_item(9, 0x9F6);
    } else {
        disable_menu_item(9, 0x9F6);
    }

    const int terrain_row_5 = sea_order_labels
        ? Terraforming[5].preq_tech_sea : Terraforming[5].preq_tech;
    if (has_tech(terrain_row_5, local_faction)) {
        enable_menu_item(9, 0x9F0);
        show_menu_item(9, 0x9F3);
        enable_menu_item(9, 0x9F3);
    } else {
        disable_menu_item(9, 0x9F0);
        hide_menu_item(9, 0x9F3);
    }

    const int terrain_row_6 = sea_order_labels
        ? Terraforming[6].preq_tech_sea : Terraforming[6].preq_tech;
    if (has_tech(terrain_row_6, local_faction)) {
        show_menu_item(9, 0x9F1);
        show_menu_item(9, 0x9F4);
        enable_menu_item(9, 0x9F1);
        enable_menu_item(9, 0x9F4);
    } else {
        hide_menu_item(9, 0x9F1);
        hide_menu_item(9, 0x9F4);
    }

    const int terrain_row_9 = sea_order_labels
        ? Terraforming[9].preq_tech_sea : Terraforming[9].preq_tech;
    if (has_tech(terrain_row_9, local_faction)) {
        show_menu_item(9, 0x9F5);
        enable_menu_item(9, 0x9F5);
    } else {
        hide_menu_item(9, 0x9F5);
    }

    const int owner = (selected_tile->bit & 1u) != 0
        ? static_cast<int>(selected_tile->val2 & 0x0Fu) : -1;
    if (owner >= 0 && owner < 8) {
        hide_menu_item(3, 0x317);
        show_menu_item(3, 0x316);
    } else {
        show_menu_item(3, 0x317);
        hide_menu_item(3, 0x316);
    }

}
    } else {
        // Native 0x00461E48 fallback for a disabled cursor or negative ID.
        for (int fallback_item_id = 0x300; fallback_item_id <= 0x324;
             ++fallback_item_id) {
            if (fallback_item_id == 0x313) enable_menu_item(3, fallback_item_id);
            else disable_menu_item(3, fallback_item_id);
        }
    }

for (int scenario_menu_id = 0x601; scenario_menu_id <= 0x61B; ++scenario_menu_id) {
    if ((GameDataStateStorage.fields_.game_state_ & 0x40u) != 0) enable_menu_item(6, scenario_menu_id);
    else disable_menu_item(6, scenario_menu_id);
}
for (int scenario_option_id = 0xA00; scenario_option_id <= 0xA54; ++scenario_option_id) {
    if ((GameDataStateStorage.fields_.game_state_ & 0x40u) != 0) enable_menu_item(10, scenario_option_id);
    else disable_menu_item(10, scenario_option_id);
}

if ((GameDataStateStorage.fields_.game_more_preferences_ & 0x200000u) == 0) {
    hide_menu_item(2, 0x205);
    hide_menu_item(2, 0x206);
    hide_menu_item(2, 0x207);
    hide_menu_item(2, 0x208);
    hide_menu_item(2, 0x20C);
    hide_menu_item(2, 0x215);
    hide_menu_item(2, 0x211);
    hide_menu_item(2, 0x212);
    hide_menu_item(3, 0x31B);
    hide_menu_item(3, 0x324);
    hide_menu_item(3, 0x306);
    hide_menu_item(3, 0x304);
    hide_menu_item(3, 0x319);
    hide_menu_item(3, 0x31A);
    hide_menu_item(3, 0x317);
    hide_menu_item(3, 0x320);
    hide_menu_item(3, 0x31F);
    hide_menu_item(3, 0x307);
    hide_menu_item(3, 0x322);
    hide_menu_item(9, 0x9F8);
    hide_menu_item(9, 0x9F9);
    hide_menu_item(9, 0x9F6);
    hide_menu_item(9, 0x9F5);
    hide_menu_item(9, 0x9F4);
    hide_menu_item(9, 0x9F3);
    hide_menu_item(9, 0x9F1);
    hide_menu_item(9, 0x9F0);
    hide_menu_item(9, 5);
    hide_menu_item(9, 6);
    hide_menu_item(9, 7);
}

}
// The initialization loads the screen-size-specific menu sheet into a
// temporary Buffer, extracts the button sprites, then initializes the real
// GraphicWin base. The local's automatic lifetime also covers the early
// null/error returns in the same order as the original C++ object.
// Superseded seed deleted; seed paths below are historical Git provenance.
// Historical seed observations (not fresh measurements or acceptance):
// TRIED: MISMATCH #58 (push vs mov) - a genuine local `Buffer` (RAII) reproduces the SEH try-frame/dtor exactly for 57 instructions (prologue, ctor, null check+early return, globals store, first branch's load_pcx+extract, loop setup); the do/while loop body's byte-read-then-extract() call diverges on push order. Tried inlining the byte read into the call expression (same as the first, unlooped, extract call) - made it worse (#43), so a separate local before the call is closer; kept.
// ORIGINAL: 0x00402F40 ?init@AlphaMenu@@QAEHPAUWin@@@Z 0x00402F40-0x00403199;0x006505C0-0x006505D5 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Retained seeds src/unrecovered/00402f40.cpp,004032a0.cpp,00403af0.cpp and src/recovered/units/00403340.cpp,00403680.cpp,00403830.cpp; pinned original callback style27/29, positive hide height, branch-only clip restoration and actual caller dispatch audited by coordinator."], "reason": "Recover AlphaMenu owner behavior through existing Menu/GraphicWin/Buffer inheritance and owned Sprite lifetimes; historical seed identity and extents remain obligations, not measurement claims."}
// symbol    ?init@AlphaMenu@@UAEHPAVWin@@@Z
int AlphaMenu::init(Win *parent) {
  Buffer sheet;
  if (parent == nullptr) return 3;

  // PullDown copies these shared callback defaults into its trailing fields;
  // AlphaMenu installs the two actual cdecl callbacks before any menus open.
  PullDownFieldF38Default = &AlphaMenu::fill_func;
  PullDownFieldF3CDefault = &AlphaMenu::backfill_func;
  field_A28_ = 5;

  const bool compact = WinScreenWidth == 0x320;
  const int result = sheet.load_pcx(compact ? "menu.pcx" : "menu2.pcx",
                                   nullptr, 10, 0xEC);
  const unsigned char transparent = WinFillColour;
  const int sprite_y = compact ? 0x8A : 0x8B;
  sprite1_.extract(&sheet, transparent, 1, 1, sprite_y, 6, nullptr);
  int frame_y = 8;
  for (int i = 0; i != 3; ++i, frame_y += 0x13) {
    sprites_[i].extract(&sheet, transparent, 1, frame_y, sprite_y, 0x12,
                        nullptr);
  }
  if (result != 0) return result;

  sheet.close();
  const int init_result = GraphicWin::init(
      0, 0, 0x51, 0x8F, nullptr, 0x1000000, &MainInterfaceGlobal,
      nullptr, nullptr);
  if (init_result != 0) return init_result;

  static_cast<Buffer *>(this)->set_markup_enabled(0);
  adjust();
  return 0;
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004032A0 ?adjust@AlphaMenu@@QAEXXZ 0x004032A0-0x00403335 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Retained seeds src/unrecovered/00402f40.cpp,004032a0.cpp,00403af0.cpp and src/recovered/units/00403340.cpp,00403680.cpp,00403830.cpp; pinned original callback style27/29, positive hide height, branch-only clip restoration and actual caller dispatch audited by coordinator."], "reason": "Recover AlphaMenu owner behavior through existing Menu/GraphicWin/Buffer inheritance and owned Sprite lifetimes; historical seed identity and extents remain obligations, not measurement claims."}
// symbol    ?adjust@AlphaMenu@@UAEXXZ
void AlphaMenu::adjust() {
  const int height = sprites_[0].height() * count_ + sprite1_.height();
  resize(sprite1_.width(), height, 0);

  const int x = WinScreenWidth == 0x320 ? 0 : (WinScreenWidth - 0x400) / 2;
  const int y = MainInterfaceGlobal.window_bounds_.top
      - MainInterfaceGlobal.window_bounds_.bottom + WinScreenHeight - height;
  Win::move(x, y);
  redraw();
  compute();
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004031A0 ?compute@AlphaMenu@@QAEXXZ 0x004031A0-0x00403292
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/004031a0.cpp", "git:5190d27f:src/leaf_recoveries.cpp:AlphaMenu_compute", "image:AlphaMenu66ED88 slot92 selects4031A0;4031FC/403208 walk bytes at+A40 with14 stride, matching Menu entries+A38 plus flags+8", "recovery/tasks/popup-font-configuration-owner-closure.json"], "reason": "Recover the real AlphaMenu compute override through owned sprites, inherited Spot and MenuEntry flags. The retired raw facade's byte-zero wording described the flags cursor, not the entry base; preserve its seed's frame-layout lesson without retaining raw offsets or a duplicate source identity."}
// symbol    ?compute@AlphaMenu@@UAEXXZ
// TRIED: SHAPE_EXACT now - every instruction matches and the only difference left is the FRAME LAYOUT, so these are ruled out for the LAST byte rather than for the shape. All measured: any frameless spelling, since the original keeps ebp; a real object local to force an SEH frame - div 2, this prologue registers no handler at all; declaration order of the locals - all six permutations byte-identical; statement order - 13 of 15 legal orders break the shape and the other 2 give the same slots; extra unreferenced ints or arrays to grow sub esp - reclaimed, frame stays 0x18. VC6 lays locals out by size ASCENDING, ties broken by a fixed intrinsic order top, bottom_end, top_end, i, entry, width that no source edit moved. Aggregates are therefore a real size lever and a 16-byte struct member landed 5 of the 6 slots exactly - but the original wants top_end at ebp-0x10 ABOVE top at ebp-0x18 in a 0x2c frame, which needs top_end allocated FIRST, and that forces size top_end < size top with the three summing to 0x20 and size bottom_end >= 0x10. No integers fit.
void AlphaMenu::compute() {
  const int row_height = sprites_[0].height();
  spot_.init(count_);

  int top = sprite1_.height();
  const int width = sprites_[0].width();
  int top_end = top + row_height;
  int bottom = (count_ - 1) * row_height + top;
  int bottom_end = bottom + row_height;

  for (int i = 0; i < count_; ++i) {
    if ((entries_[i].flags & 1) == 0) continue;
    if (entries_[i].flags & 4) {
      spot_.add(i, 0, 0, bottom, width, bottom_end - bottom);
      bottom -= row_height;
      bottom_end -= row_height;
    } else {
      spot_.add(i, 0, 0, top, width, top_end - top);
      top += row_height;
      top_end += row_height;
    }
  }
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns 0.
// ORIGINAL: 0x00403610 ?requested_height@AlphaMenu@@QAEHXZ 0x00403610-0x00403613 BYTE_EXACT
// symbol    ?requested_height@AlphaMenu@@UAEHXZ
// size      3 bytes
// prototype int (__thiscall ?requested_height@AlphaMenu@@QAEHXZ)(AlphaMenu* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0
Status: Complete
*/
int AlphaMenu::requested_height() {
    return 0;
}

// ORIGINAL: 0x00403620 ?on_adjust_pulldown_pos@AlphaMenu@@QAEXPAHPAH@Z 0x00403620-0x00403677 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned image 0x00403620 reads Menu field_A24_ and SpotInternal[index].rect at stride 0x18; Win outer_rect_ is at 0x13c and caption_height_ at 0x118.", "git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/recovered/units/00403620.cpp"], "reason": "Recover AlphaMenu pull-down placement through its inherited Menu, Spot and Win state. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
// kind      game
// symbol    ?on_adjust_pulldown_pos@AlphaMenu@@UAEXPAH0@Z
void AlphaMenu::on_adjust_pulldown_pos(int *x, int *y) {
    if (field_A24_ != -1) {
        RECT spot_rect;
        spot_.get_rect_at(field_A24_, &spot_rect);
        *x = outer_rect_.right;
        *y = spot_rect.top + caption_height_ + outer_rect_.top;
    }
}

// ORIGINAL: 0x00403B70 ?backfill_func@AlphaMenu@@QAAHPAUGraphicWin@@@Z 0x00403B70-0x00403B94 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned image 0x00403B70 calls Buffer::box_sprite at 0x005E2DD3 with the target Buffer::rect2_ at target+0x474 and style record 0x0078D960, which is InterfaceBoxStyles[27]."], "reason": "Recover AlphaMenu static backfill callback using the supplied GraphicWin and its typed Buffer subobject."}
// symbol    ?backfill_func@AlphaMenu@@SAHPAVGraphicWin@@@Z
int AlphaMenu::backfill_func(GraphicWin *target) {
    target->Buffer::box_sprite(target->buffer_working_rect(), &InterfaceBoxStyles[27]);
    return 1;
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00403AF0 ?fill_func@AlphaMenu@@QAAHPAUGraphicWin@@HPAURECT@@@Z 0x00403AF0-0x00403B70 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Retained seeds src/unrecovered/00402f40.cpp,004032a0.cpp,00403af0.cpp and src/recovered/units/00403340.cpp,00403680.cpp,00403830.cpp; pinned original callback style27/29, positive hide height, branch-only clip restoration and actual caller dispatch audited by coordinator."], "reason": "Recover AlphaMenu owner behavior through existing Menu/GraphicWin/Buffer inheritance and owned Sprite lifetimes; historical seed identity and extents remain obligations, not measurement claims."}
// symbol    ?fill_func@AlphaMenu@@SAHPAVGraphicWin@@HPAUtagRECT@@@Z
int __cdecl AlphaMenu::fill_func(GraphicWin *target, int mode, RECT *clip) {
  Buffer *const canvas = static_cast<Buffer *>(target);
  RECT draw_clip = *clip;
  draw_clip.left = 1;
  draw_clip.right = target->buffer_working_rect()->right - 1;
  canvas->set_clip(&draw_clip);
  canvas->box_sprite(target->buffer_working_rect(),
                     &InterfaceBoxStyles[mode != 0 ? 29 : 27]);
  canvas->set_clip(target->buffer_working_rect());
  return 1;
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00403340 ?on_redraw@AlphaMenu@@QAEXXZ 0x00403340-0x00403544 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Retained seeds src/unrecovered/00402f40.cpp,004032a0.cpp,00403af0.cpp and src/recovered/units/00403340.cpp,00403680.cpp,00403830.cpp; pinned original callback style27/29, positive hide height, branch-only clip restoration and actual caller dispatch audited by coordinator."], "reason": "Recover AlphaMenu owner behavior through existing Menu/GraphicWin/Buffer inheritance and owned Sprite lifetimes; historical seed identity and extents remain obligations, not measurement claims."}
// symbol    ?on_redraw@AlphaMenu@@UAEXXZ
void AlphaMenu::on_redraw() {
  Buffer *const canvas = static_cast<Buffer *>(this);
  GraphicWin::fill(9);
  sprite1_.draw(canvas, sprite1_.transparent_index(), 0, 0, 1, 1);
  canvas->set_font(PopupMenuBoldFont, nullptr, nullptr, nullptr);

  int y = sprite1_.height();
  for (int sprite_index = 0; sprite_index < count_; ++sprite_index) {
    Sprite *sprite = &sprites_[0];
    if (sprite_index == field_A24_) {
      sprite = &sprites_[2];
    } else if (sprite_index == MenuSelection) {
      sprite = &sprites_[1];
    }
    sprite->draw(canvas, sprite->transparent_index(), 0, y, 1, 1);
    y += sprites_[0].height();
  }

  for (int label_index = 0; label_index < count_; ++label_index) {
    const uint8_t flags = entries_[label_index].flags;
    if ((flags & 1) == 0) continue;

    const int color_pair = label_index == field_A24_
        ? ((flags & 2) ? 34 : 30)
        : ((flags & 2) ? 36 : 32);
    canvas->set_text_color(TextColourTable[color_pair],
                           TextColourTable[color_pair + 1], 1, 1);

    char text[256];
    text[0] = '\0';
    strcat(text, entries_[label_index].text);
    CharUpperA(text);

    RECT row;
    spot_.get_rect_at(label_index, &row);
    row.left += 8;
    row.right -= 8;
    canvas->write_l(text, &row, strlen(text));
  }
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00403680 ?show@AlphaMenu@@QAEXXZ 0x00403680-0x0040382F FILE
// RECOVERY_BINDING: v1 {"evidence": ["Retained seeds src/unrecovered/00402f40.cpp,004032a0.cpp,00403af0.cpp and src/recovered/units/00403340.cpp,00403680.cpp,00403830.cpp; pinned original callback style27/29, positive hide height, branch-only clip restoration and actual caller dispatch audited by coordinator."], "reason": "Recover AlphaMenu owner behavior through existing Menu/GraphicWin/Buffer inheritance and owned Sprite lifetimes; historical seed identity and extents remain obligations, not measurement claims."}
void AlphaMenu::show() {
  if (is_visible()) return;

  if (GameDataStateStorage.fields_.game_preferences_ & 0x400) g_ALPHAMENU_WAVE.play();
  on_redraw();

  RECT screen_rect = *buffer_working_rect();
  client_to_screen(&screen_rect);
  int y = screen_rect.bottom - sprite1_.height();
  const int step = (screen_rect.bottom - screen_rect.top) / 5;
  ScreenBuffer.set_clip(&screen_rect);

  if ((GameDataStateStorage.fields_.game_preferences_ & 0x8000000) != 0 && IsMultiplayerNet == 0) {
    const int frame_x = animation_frame_x();
    WinFlipClipped = 1;
    for (int frame = 0; frame != 6; ++frame) {
      const DWORD started = timeGetTime();
      static_cast<Buffer *>(this)->draw(&ScreenBuffer, 9,
                                        frame_x, y, 1, 1);
      flip(&screen_rect);
      y -= step;
      do_sound();
      wait_for_animation_frame(started);
    }
    ScreenBuffer.set_clip(&WinScreenClipRect);
    WinFlipClipped = 0;
  }

  ScreenBuffer.set_clip(&WinScreenClipRect);
  WinFlipClipped = 0;
  Win::show(3);
  MainInterfaceGlobal.flat_button_[3].set_pressed_visual(1);
  MainInterfaceGlobal.flat_button_[3].redraw();
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00403830 ?hide@AlphaMenu@@QAEXXZ 0x00403830-0x00403AE5 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Retained seeds src/unrecovered/00402f40.cpp,004032a0.cpp,00403af0.cpp and src/recovered/units/00403340.cpp,00403680.cpp,00403830.cpp; pinned original callback style27/29, positive hide height, branch-only clip restoration and actual caller dispatch audited by coordinator."], "reason": "Recover AlphaMenu owner behavior through existing Menu/GraphicWin/Buffer inheritance and owned Sprite lifetimes; historical seed identity and extents remain obligations, not measurement claims."}
// symbol    ?hide@AlphaMenu@@UAEXXZ
void AlphaMenu::hide() {
  if (!is_visible()) return;

  if (GameDataStateStorage.fields_.game_preferences_ & 0x400) g_ALPHAMENU_WAVE.play();
  MenuSelection = -1;
  MainInterfaceGlobal.field_216E8_ = 0;
  on_redraw();

  RECT screen_rect = *buffer_working_rect();
  client_to_screen(&screen_rect);
  RECT console_rect = screen_rect;
  ConsoleGlobal.screen_to_client(&console_rect);

  const int width = pixel_width();
  const int height = pixel_height();
  buffer_.init(width, height, 0, 0);
  static_cast<Buffer *>(&ConsoleGlobal)->copy(
      &buffer_, console_rect.left, console_rect.top, 0, 0, width, height);

  int y = screen_rect.top;
  const int target_y = screen_rect.bottom - sprite1_.height();
  const int step = (screen_rect.bottom - screen_rect.top) / 5;
  ScreenBuffer.set_clip(&screen_rect);
  if ((GameDataStateStorage.fields_.game_preferences_ & 0x8000000) != 0 && IsMultiplayerNet == 0) {
    const int frame_x = animation_frame_x();
    WinFlipClipped = 1;
    for (int frame = 0; frame != 6; ++frame) {
      const DWORD started = timeGetTime();
      buffer_.copy(&ScreenBuffer, 0, 0, screen_rect.left, screen_rect.top,
                   width, height);
      static_cast<Buffer *>(this)->draw(&ScreenBuffer, 9, frame_x, y, 1, 1);
      flip(&screen_rect);
      y += step;
      if (y >= target_y) y = target_y;
      do_sound();
      wait_for_animation_frame(started);
    }
    if (y < target_y) {
      buffer_.copy(&ScreenBuffer, 0, 0, screen_rect.left, screen_rect.top,
                   width, height);
      static_cast<Buffer *>(this)->draw(&ScreenBuffer, 9,
                                        screen_rect.left, target_y, 1, 1);
      flip(&screen_rect);
    }
    ScreenBuffer.set_clip(&WinScreenClipRect);
    WinFlipClipped = 0;
  }

  Win::hide();
  buffer_.close();
  MainInterfaceGlobal.flat_button_[3].set_pressed_visual(0);
  MainInterfaceGlobal.flat_button_[3].redraw();
}

// The image's on_mouse_move identity is Win slot 17: it tracks the
// highlighted Spot and releases capture when the pointer leaves the menu.
// Unlike the old seed scaffold, this uses Menu's actual Spot and Win's
// source-owned capture globals and virtual callbacks.
// Superseded seed deleted; seed paths below are historical Git provenance.
// Historical seed observations (not fresh measurements or acceptance):
// TRIED: if/else polarity flipped to match layout (jne first, matches); remaining MISMATCH is the compiler omitting the EBP frame that the original allocates (esp-relative &a2 suffices here), not reproduced by reordering statements
// ORIGINAL: 0x00403550 ?on_mouse_move@AlphaMenu@@QAEXHH@Z 0x00403550-0x0040360F
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/00403550.cpp"], "reason": "The original menu mouse body updates Menu spot selection and Win pointer capture through the inherited slot-17 callback, whose product spelling is on_mouse_move."}
// symbol    ?on_mouse_move@AlphaMenu@@UAEXHH@Z
void AlphaMenu::on_mouse_move(int x, int y) {
    if (field_A24_ != -1) {
        Menu::on_mouse_move(x, y);
    } else if (WinPointerOwner3 == 0) {
        WinPointerOwner3 = this;
        WinPointerOwner4 = 0;
    }

    int position = y;
    const int hit = spot_.check(x, y, &position, 0);
    if (hit == -1) {
        MenuSelection = -1;
        redraw();
        on_update();
        if (WinPointerOwner3 == this) {
            WinPointerOwner3 = 0;
            on_lose_mouse_capture();
        }
        if (WinPointerOwner4 == this) {
            WinPointerOwner4 = 0;
        }
    } else {
        MenuSelection = position;
        redraw();
        on_update();
    }
}

// Console initializes its embedded AlphaMenu through this catalogue entry.
// Population uses the existing rules and StringTable owners; x86 pool keys
// remain the existing pointer-to-int Strings API, not portable pointer proof.
// ORIGINAL: 0x0045FB20 ?menu_init@MainMenu@@QAEHXZ 0x0045FB20-0x00460DD0
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/0045fb20.cpp", "image:0x51D922,0x51D996-0x51D99D:Console alpha_menu_ receiver"], "reason": "Pinned Console init receiver+22A2C identifies existing AlphaMenu; catalogue MainMenu identity retained separately from native AlphaMenu symbol. Complete ordered menu population uses source rules and StringTable owners; no byte claim."}
// TRIED: ~100% of the call structure transcribed 1:1 (all 181 add_menu_item, 41 add_separator, 9 add_menu, 9 hide/disable_menu_item, 9 text_open group opens, the 19-iteration struct-array loop building "id: text" labels via Strings::get+strcat, the trailing conditional hide_menu_item and vtable slot093 dispatch); diverges at prologue (#2 sub vs push) because the local frame layout (the [ebp-8] result slot, the [ebp-4] loop cursor) is not reproduced - returns are done via plain `return N;` instead of a stored local, so stack size/order differs from here on. Not chased further given size.
// symbol    ?menu_init@AlphaMenu@@QAEHXZ
int AlphaMenu::menu_init() {
    Menu *const menu = this;
    read_rules(0);
    if (text_open("MENU", "GAME") != 0) return 1;
    menu->add_menu(0, text_get(), 0);
    menu->add_menu_item(0, 0x51, text_get());
    menu->add_menu_item(0, 0x52, text_get());
    menu->add_separator(0, 0);
    menu->add_menu_item(0, 0x53, text_get());
    menu->add_menu_item(0, 0x54, text_get());
    menu->add_menu_item(0, 0x55, text_get());
    menu->add_menu_item(0, 0x56, text_get());
    menu->add_menu_item(0, 0x57, text_get());
    menu->add_menu_item(0, 0x58, text_get());
    menu->add_separator(0, 0);
    menu->add_menu_item(0, 0x59, text_get());
    menu->add_menu_item(0, 0x5a, text_get());
    menu->add_separator(0, 0);
    menu->add_menu_item(0, 0x5b, text_get());
    menu->add_menu_item(0, 0x5c, text_get());
    menu->add_menu_item(0, 0x5d, text_get());
    if (text_open("MENU", "HQ") != 0) return 1;
    menu->add_menu(1, text_get(), 0);
    menu->add_menu_item(1, 0x105, text_get());
    menu->add_menu_item(1, 0x106, text_get());
    menu->add_menu_item(1, 0x107, text_get());
    menu->add_separator(1, 0);
    menu->add_menu_item(1, 0x103, text_get());
    menu->add_separator(1, 0);
    menu->add_menu_item(1, 0x108, text_get());
    menu->add_menu_item(1, 0x109, text_get());
    menu->add_menu_item(1, 0x10a, text_get());
    menu->add_separator(1, 0);
    menu->add_menu_item(1, 0x10b, text_get());
    menu->add_menu_item(1, 0x10c, text_get());
    menu->add_menu_item(1, 0x10d, text_get());
    menu->add_menu_item(1, 0x10e, text_get());
    menu->add_separator(1, 0);
    menu->add_menu_item(1, 0x10f, text_get());
    menu->add_menu_item(1, 0x110, text_get());
    menu->add_menu_item(1, 0x111, text_get());
    menu->add_menu_item(1, 0x113, text_get());
    menu->add_separator(1, 9);
    menu->add_menu_item(1, 0x114, text_get());
    menu->add_menu_item(1, 0x115, text_get());
    if (text_open("MENU", "NETWORK") != 0) return 1;
    menu->add_menu(8, text_get(), 0);
    menu->add_menu_item(8, 0x800, text_get());
    menu->add_menu_item(8, 0x801, text_get());
    menu->add_menu_item(8, 0x802, text_get());
    menu->add_menu_item(8, 0x803, text_get());
    menu->add_menu_item(8, 0x804, text_get());
    menu->add_separator(8, 0);
    menu->add_menu_item(8, 0x805, text_get());
    menu->add_separator(8, 0);
    menu->add_menu_item(8, 0x806, text_get());
    menu->add_menu_item(8, 0x807, text_get());
    menu->add_menu_item(8, 0x808, text_get());
    if (text_open("MENU", "MAP") != 0) return 1;
    menu->add_menu(2, text_get(), 0);
    menu->add_menu_item(2, 0x200, text_get());
    menu->add_menu_item(2, 0x201, text_get());
    menu->add_separator(2, 0);
    menu->add_menu_item(2, 0x203, text_get());
    menu->add_menu_item(2, 0x204, text_get());
    menu->add_menu_item(2, 0x205, text_get());
    menu->add_menu_item(2, 0x206, text_get());
    menu->add_menu_item(2, 0x207, text_get());
    menu->add_menu_item(2, 0x208, text_get());
    menu->add_separator(2, 0);
    menu->add_menu_item(2, 0x209, text_get());
    menu->add_menu_item(2, 0x20a, text_get());
    menu->add_menu_item(2, 0x20b, text_get());
    menu->add_menu_item(2, 0x20c, text_get());
    menu->add_menu_item(2, 0x215, text_get());
    menu->add_separator(2, 0);
    menu->add_menu_item(2, 0x20f, text_get());
    menu->add_menu_item(2, 0x210, text_get());
    menu->add_menu_item(2, 0x211, text_get());
    menu->add_menu_item(2, 0x212, text_get());
    menu->add_menu_item(2, 0x213, text_get());
    menu->add_separator(2, 0);
    menu->add_menu_item(2, 0x216, text_get());
    menu->add_menu_item(2, 0x217, text_get());
    menu->add_menu_item(2, 0x214, text_get());
    if (text_open("MENU", "ACTION") != 0) return 1;
    menu->add_menu(3, text_get(), 0);
    menu->add_menu_item(3, 0x300, text_get());
    menu->add_menu_item(3, 0x301, text_get());
    menu->add_menu_item(3, 0x302, text_get());
    menu->add_separator(3, 2);
    menu->add_menu_item(3, 0x305, text_get());
    menu->add_menu_item(3, 0x309, text_get());
    menu->add_menu_item(3, 0x30a, text_get());
    menu->add_menu_item(3, 0x310, text_get());
    menu->add_menu_item(3, 0x308, text_get());
    menu->add_menu_item(3, 0x311, text_get());
    menu->add_menu_item(3, 0x312, text_get());
    menu->add_separator(3, 0);
    menu->add_menu_item(3, 0x314, text_get());
    menu->add_menu_item(3, 0x31b, text_get());
    menu->add_menu_item(3, 0x324, text_get());
    menu->add_menu_item(3, 0x306, text_get());
    menu->add_menu_item(3, 0x304, text_get());
    menu->add_separator(3, 0);
    menu->add_menu_item(3, 0x318, text_get());
    menu->add_menu_item(3, 0x319, text_get());
    menu->add_menu_item(3, 0x31a, text_get());
    menu->add_menu_item(3, 0x317, text_get());
    menu->add_menu_item(3, 0x316, text_get());
    menu->add_separator(3, 0);
    menu->add_menu_item(3, 0x313, text_get());
    menu->add_menu_item(3, 0x31c, text_get());
    menu->add_menu_item(3, 0x31d, text_get());
    menu->add_menu_item(3, 0x31e, text_get());
    menu->add_menu_item(3, 0x31f, text_get());
    menu->add_menu_item(3, 0x320, text_get());
    menu->add_separator(3, 0);
    menu->add_menu_item(3, 0x321, text_get());
    menu->add_menu_item(3, 0x307, text_get());
    menu->add_menu_item(3, 0x322, text_get());
    menu->add_menu_item(3, 0x315, text_get());
    menu->add_menu_item(3, 0x323, text_get());
    if (text_open("MENU", "TERRAFORM") != 0) return 1;
    menu->add_menu(9, text_get(), 0);
    {
        for (int i = 0; i != 19; ++i) {
            StringTemp[0] = 0;
            strcat(StringTemp,
                   StringTable.get(reinterpret_cast<int>(Order[i + 4].order)));
            strcat(StringTemp, "|");
            strcat(StringTemp, StringTable.get((int)Terraforming[i].shortcuts));
            menu->add_menu_item(9, 0x900 + i, StringTemp);
            if (i == 4 || i == 11 || i == 14) {
                menu->add_separator(9, i == 14);
            }
        }
    }
    menu->add_separator(9, 5);
    menu->add_menu_item(9, 0x9f8, text_get());
    menu->add_menu_item(9, 0x9f9, text_get());
    menu->add_separator(9, 6);
    menu->add_menu_item(9, 0x9f0, text_get());
    menu->add_menu_item(9, 0x9f1, text_get());
    menu->add_separator(9, 7);
    menu->add_menu_item(9, 0x9f3, text_get());
    menu->add_menu_item(9, 0x9f4, text_get());
    menu->add_menu_item(9, 0x9f5, text_get());
    menu->add_menu_item(9, 0x9f6, text_get());
    menu->add_menu_item(9, 0x9f7, text_get());
    menu->add_menu_item(9, 0x9f2, text_get());
    /* pop edi; pop ebx (loop epilogue registers) */
    if (text_open("MENU", "SCENARIO") != 0) return 1;
    menu->add_menu(6, text_get(), 0);
    menu->add_menu_item(6, 0x600, text_get());
    menu->add_menu_item(6, 0x601, text_get());
    menu->add_separator(6, 0);
    menu->add_menu_item(6, 0x602, text_get());
    menu->add_menu_item(6, 0x603, text_get());
    menu->add_menu_item(6, 0x604, text_get());
    menu->add_menu_item(6, 0x605, text_get());
    menu->add_separator(6, 0);
    menu->add_menu_item(6, 0x606, text_get());
    menu->add_menu_item(6, 0x607, text_get());
    menu->add_menu_item(6, 0x608, text_get());
    menu->add_menu_item(6, 0x609, text_get());
    menu->add_separator(6, 0);
    menu->add_menu_item(6, 0x60a, text_get());
    menu->add_menu_item(6, 0x60b, text_get());
    menu->add_menu_item(6, 0x60c, text_get());
    menu->add_menu_item(6, 0x60d, text_get());
    menu->add_menu_item(6, 0x60e, text_get());
    menu->add_separator(6, 0);
    menu->add_menu_item(6, 0x60f, text_get());
    menu->add_menu_item(6, 0x610, text_get());
    menu->add_menu_item(6, 0x611, text_get());
    menu->add_menu_item(6, 0x612, text_get());
    menu->add_menu_item(6, 0x613, text_get());
    menu->add_separator(6, 0);
    menu->add_menu_item(6, 0x614, text_get());
    menu->add_menu_item(6, 0x615, text_get());
    menu->add_menu_item(6, 0x616, text_get());
    menu->add_menu_item(6, 0x617, text_get());
    menu->add_separator(6, 0);
    menu->add_menu_item(6, 0x618, text_get());
    menu->add_menu_item(6, 0x619, text_get());
    menu->add_menu_item(6, 0x61a, text_get());
    menu->add_menu_item(6, 0x61b, text_get());
    if (text_open("MENU", "EDITOR") != 0) return 1;
    menu->add_menu(0xa, text_get(), 0);
    menu->add_menu_item(0xa, 0xa00, text_get());
    menu->add_menu_item(0xa, 0xa01, text_get());
    menu->add_separator(0xa, 0);
    menu->add_menu_item(0xa, 0xa10, text_get());
    menu->add_menu_item(0xa, 0xa11, text_get());
    menu->add_menu_item(0xa, 0xa12, text_get());
    menu->add_menu_item(0xa, 0xa13, text_get());
    menu->add_menu_item(0xa, 0xa14, text_get());
    menu->add_menu_item(0xa, 0xa15, text_get());
    menu->add_menu_item(0xa, 0xa16, text_get());
    menu->add_separator(0xa, 0);
    menu->add_menu_item(0xa, 0xa17, text_get());
    menu->add_menu_item(0xa, 0xa18, text_get());
    menu->add_menu_item(0xa, 0xa19, text_get());
    menu->add_separator(0xa, 0);
    menu->add_menu_item(0xa, 0xa20, text_get());
    menu->add_menu_item(0xa, 0xa21, text_get());
    menu->add_menu_item(0xa, 0xa22, text_get());
    menu->add_menu_item(0xa, 0xa24, text_get());
    menu->add_separator(0xa, 0);
    menu->add_menu_item(0xa, 0xa48, text_get());
    menu->add_menu_item(0xa, 0xa30, text_get());
    menu->add_menu_item(0xa, 0xa31, text_get());
    menu->add_menu_item(0xa, 0xa32, text_get());
    menu->add_separator(0xa, 0);
    menu->add_menu_item(0xa, 0xa36, text_get());
    menu->add_menu_item(0xa, 0xa37, text_get());
    menu->add_menu_item(0xa, 0xa40, text_get());
    menu->add_menu_item(0xa, 0xa41, text_get());
    menu->add_separator(0xa, 0);
    menu->add_menu_item(0xa, 0xa50, text_get());
    menu->add_menu_item(0xa, 0xa51, text_get());
    menu->add_menu_item(0xa, 0xa52, text_get());
    menu->add_menu_item(0xa, 0xa53, text_get());
    menu->add_separator(0xa, 0);
    menu->add_menu_item(0xa, 0xa54, text_get());
    if (text_open("MENU", "HELP") != 0) return 1;
    menu->add_menu(7, text_get(), 1);
    menu->add_menu_item(7, 0x701, text_get());
    menu->add_menu_item(7, 0x702, text_get());
    menu->add_menu_item(7, 0x703, text_get());
    menu->add_separator(7, 0);
    menu->add_menu_item(7, 0x704, text_get());
    menu->add_menu_item(7, 0x70b, text_get());
    menu->add_menu_item(7, 0x70c, text_get());
    menu->add_separator(7, 0);
    menu->add_menu_item(7, 0x70d, text_get());
    menu->add_menu_item(7, 0x70e, text_get());
    menu->add_separator(7, 0);
    menu->add_menu_item(7, 0x70a, text_get());
    menu->add_menu_item(7, 0x705, text_get());
    menu->add_menu_item(7, 0x706, text_get());
    menu->add_menu_item(7, 0x707, text_get());
    menu->add_menu_item(7, 0x708, text_get());
    menu->add_menu_item(7, 0x709, text_get());
    menu->add_separator(7, 0);
    menu->add_menu_item(7, 0x720, text_get());
    menu->add_menu_item(7, 0x721, text_get());
    menu->add_menu_item(7, 0x722, text_get());
    menu->add_menu_item(7, 0x723, text_get());
    menu->add_menu_item(7, 0x724, text_get());
    menu->add_menu_item(7, 0x725, text_get());
    menu->add_separator(7, 0);
    menu->add_menu_item(7, 0x750, text_get());
    menu->add_menu_item(7, 0x751, text_get());
    menu->hide_menu_item(1, 0x106);
    menu->hide_menu_item(1, 0x107);
    menu->hide_menu_item(3, 0x301);
    menu->hide_menu_item(3, 0x309);
    menu->hide_menu_item(3, 0x30a);
    menu->hide_menu_item(3, 0x310);
    menu->hide_menu_item(3, 0x306);
    menu->hide_menu_item(3, 0x304);
    menu->disable_menu_item(3, 0x305);
    if ((GameDataStateStorage.fields_.game_rules_ & RULES_IRONMAN) != 0) {
        menu->hide_menu_item(0, 0x5a);
    }
    check(-1);
    adjust();
    return 0;
}
