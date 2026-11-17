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

// Inferred Console menu-policy contribution at45FB20..46207E. The two
// adjacent entries share the real Console menu receiver. Original MainMenu
// versus AlphaMenu class identity is unresolved; keep current layout and
// typed calls until independent evidence distinguishes those relationships.
#include "stdafx.h"
#include "popup_fonts.h"
#include "alphamenu.h"
#include "netdaemon.h"
#include "alpha.h"
#include "strings.h"
#include "text.h"
#include "terraforming.h"
#include "technology.h"
#include "veh.h"
#include "console.h"
#include "game.h"
#include "interface_palette.h"
#include "maininterface.h"
#include "palette.h"
#include "pulldown.h"
#include "wave.h"
#include "buffer.h"

// Shared punctuation objects the image pushes instead of unit literals.
extern char SocialWinCloseParenText[];
extern char SocialWinOpenParenText[];
// Original pointer cell 0x00691B1C selects the MENU script name at
// 0x00691B9C; menu_init passes the cell's value to every text_open.
// RECOVERY_OBJECT: v1 0x00691B9C ?MenuScriptFileText@@3PADA
char MenuScriptFileText[5] = "MENU";
// RECOVERY_OBJECT: v1 0x00691B1C ?MenuScriptFile@@3PADA
char *MenuScriptFile = MenuScriptFileText;

// Console initializes its embedded AlphaMenu through this catalogue entry.
// Population uses the existing rules and StringTable owners; x86 pool keys
// remain the existing pointer-to-int Strings API, not portable pointer proof.
// ORIGINAL: 0x0045FB20 ?menu_init@MainMenu@@QAEHXZ 0x0045FB20-0x00460DD0
// RECOVERY_BINDING: v1 {"evidence": ["git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/0045fb20.cpp", "image:0x51D922,0x51D996-0x51D99D:Console alpha_menu_ receiver"], "reason": "Pinned Console init receiver+22A2C identifies existing AlphaMenu; catalogue MainMenu identity retained separately from native AlphaMenu symbol. Complete ordered menu population uses source rules and StringTable owners; no byte claim."}
// TRIED: ~100% of the call structure transcribed 1:1 (all 181 add_menu_item, 41 add_separator, 9 add_menu, 9 hide/disable_menu_item, 9 text_open group opens, the 19-iteration struct-array loop building "id: text" labels via Strings::get+strcat, the trailing conditional hide_menu_item and vtable slot093 dispatch); diverges at prologue (#2 sub vs push) because the local frame layout (the [ebp-8] result slot, the [ebp-4] loop cursor) is not reproduced - returns are done via plain `return N;` instead of a stored local, so stack size/order differs from here on. Not chased further given size.
// symbol    ?menu_init@AlphaMenu@@QAEHXZ
int AlphaMenu::menu_init() {
    int result = 1;
    read_rules(0);
    if (text_open(MenuScriptFile, "GAME") != 0) goto done;
    add_menu(0, text_get(), 0);
    add_menu_item(0, 0x51, text_get());
    add_menu_item(0, 0x52, text_get());
    add_separator(0, 0);
    add_menu_item(0, 0x53, text_get());
    add_menu_item(0, 0x54, text_get());
    add_menu_item(0, 0x55, text_get());
    add_menu_item(0, 0x56, text_get());
    add_menu_item(0, 0x57, text_get());
    add_menu_item(0, 0x58, text_get());
    add_separator(0, 0);
    add_menu_item(0, 0x59, text_get());
    add_menu_item(0, 0x5a, text_get());
    add_separator(0, 0);
    add_menu_item(0, 0x5b, text_get());
    add_menu_item(0, 0x5c, text_get());
    add_menu_item(0, 0x5d, text_get());
    if (text_open(MenuScriptFile, "HQ") != 0) goto done;
    add_menu(1, text_get(), 0);
    add_menu_item(1, 0x105, text_get());
    add_menu_item(1, 0x106, text_get());
    add_menu_item(1, 0x107, text_get());
    add_separator(1, 0);
    add_menu_item(1, 0x103, text_get());
    add_separator(1, 0);
    add_menu_item(1, 0x108, text_get());
    add_menu_item(1, 0x109, text_get());
    add_menu_item(1, 0x10a, text_get());
    add_separator(1, 0);
    add_menu_item(1, 0x10b, text_get());
    add_menu_item(1, 0x10c, text_get());
    add_menu_item(1, 0x10d, text_get());
    add_menu_item(1, 0x10e, text_get());
    add_separator(1, 0);
    add_menu_item(1, 0x10f, text_get());
    add_menu_item(1, 0x110, text_get());
    add_menu_item(1, 0x111, text_get());
    add_menu_item(1, 0x113, text_get());
    add_separator(1, 9);
    add_menu_item(1, 0x114, text_get());
    add_menu_item(1, 0x115, text_get());
    if (text_open(MenuScriptFile, "NETWORK") != 0) goto done;
    add_menu(8, text_get(), 0);
    add_menu_item(8, 0x800, text_get());
    add_menu_item(8, 0x801, text_get());
    add_menu_item(8, 0x802, text_get());
    add_menu_item(8, 0x803, text_get());
    add_menu_item(8, 0x804, text_get());
    add_separator(8, 0);
    add_menu_item(8, 0x805, text_get());
    add_separator(8, 0);
    add_menu_item(8, 0x806, text_get());
    add_menu_item(8, 0x807, text_get());
    add_menu_item(8, 0x808, text_get());
    if (text_open(MenuScriptFile, "MAP") != 0) goto done;
    add_menu(2, text_get(), 0);
    add_menu_item(2, 0x200, text_get());
    add_menu_item(2, 0x201, text_get());
    add_separator(2, 0);
    add_menu_item(2, 0x203, text_get());
    add_menu_item(2, 0x204, text_get());
    add_menu_item(2, 0x205, text_get());
    add_menu_item(2, 0x206, text_get());
    add_menu_item(2, 0x207, text_get());
    add_menu_item(2, 0x208, text_get());
    add_separator(2, 0);
    add_menu_item(2, 0x209, text_get());
    add_menu_item(2, 0x20a, text_get());
    add_menu_item(2, 0x20b, text_get());
    add_menu_item(2, 0x20c, text_get());
    add_menu_item(2, 0x215, text_get());
    add_separator(2, 0);
    add_menu_item(2, 0x20f, text_get());
    add_menu_item(2, 0x210, text_get());
    add_menu_item(2, 0x211, text_get());
    add_menu_item(2, 0x212, text_get());
    add_menu_item(2, 0x213, text_get());
    add_separator(2, 0);
    add_menu_item(2, 0x216, text_get());
    add_menu_item(2, 0x217, text_get());
    add_menu_item(2, 0x214, text_get());
    if (text_open(MenuScriptFile, "ACTION") != 0) goto done;
    add_menu(3, text_get(), 0);
    add_menu_item(3, 0x300, text_get());
    add_menu_item(3, 0x301, text_get());
    add_menu_item(3, 0x302, text_get());
    add_separator(3, 2);
    add_menu_item(3, 0x305, text_get());
    add_menu_item(3, 0x309, text_get());
    add_menu_item(3, 0x30a, text_get());
    add_menu_item(3, 0x310, text_get());
    add_menu_item(3, 0x308, text_get());
    add_menu_item(3, 0x311, text_get());
    add_menu_item(3, 0x312, text_get());
    add_separator(3, 0);
    add_menu_item(3, 0x314, text_get());
    add_menu_item(3, 0x31b, text_get());
    add_menu_item(3, 0x324, text_get());
    add_menu_item(3, 0x306, text_get());
    add_menu_item(3, 0x304, text_get());
    add_separator(3, 0);
    add_menu_item(3, 0x318, text_get());
    add_menu_item(3, 0x319, text_get());
    add_menu_item(3, 0x31a, text_get());
    add_menu_item(3, 0x317, text_get());
    add_menu_item(3, 0x316, text_get());
    add_separator(3, 0);
    add_menu_item(3, 0x313, text_get());
    add_menu_item(3, 0x31c, text_get());
    add_menu_item(3, 0x31d, text_get());
    add_menu_item(3, 0x31e, text_get());
    add_menu_item(3, 0x31f, text_get());
    add_menu_item(3, 0x320, text_get());
    add_separator(3, 0);
    add_menu_item(3, 0x321, text_get());
    add_menu_item(3, 0x307, text_get());
    add_menu_item(3, 0x322, text_get());
    add_menu_item(3, 0x315, text_get());
    add_menu_item(3, 0x323, text_get());
    if (text_open(MenuScriptFile, "TERRAFORM") != 0) goto done;
    add_menu(9, text_get(), 0);
    {
        for (int i = 0; i < 19; ++i) {
            StringTemp[0] = 0;
            strcat(StringTemp, StringTable.get(Order[i + 4].order));
            strcat(StringTemp, "|");
            strcat(StringTemp, StringTable.get(Terraforming[i].shortcuts));
            add_menu_item(9, 0x900 + i, StringTemp);
            if (i == 4 || i == 11 || i == 14) {
                add_separator(9, i == 14);
            }
        }
    }
    add_separator(9, 5);
    add_menu_item(9, 0x9f8, text_get());
    add_menu_item(9, 0x9f9, text_get());
    add_separator(9, 6);
    add_menu_item(9, 0x9f0, text_get());
    add_menu_item(9, 0x9f1, text_get());
    add_separator(9, 7);
    add_menu_item(9, 0x9f3, text_get());
    add_menu_item(9, 0x9f4, text_get());
    add_menu_item(9, 0x9f5, text_get());
    add_menu_item(9, 0x9f6, text_get());
    add_menu_item(9, 0x9f7, text_get());
    add_menu_item(9, 0x9f2, text_get());
    /* pop edi; pop ebx (loop epilogue registers) */
    if (text_open(MenuScriptFile, "SCENARIO") != 0) goto done;
    add_menu(6, text_get(), 0);
    add_menu_item(6, 0x600, text_get());
    add_menu_item(6, 0x601, text_get());
    add_separator(6, 0);
    add_menu_item(6, 0x602, text_get());
    add_menu_item(6, 0x603, text_get());
    add_menu_item(6, 0x604, text_get());
    add_menu_item(6, 0x605, text_get());
    add_separator(6, 0);
    add_menu_item(6, 0x606, text_get());
    add_menu_item(6, 0x607, text_get());
    add_menu_item(6, 0x608, text_get());
    add_menu_item(6, 0x609, text_get());
    add_separator(6, 0);
    add_menu_item(6, 0x60a, text_get());
    add_menu_item(6, 0x60b, text_get());
    add_menu_item(6, 0x60c, text_get());
    add_menu_item(6, 0x60d, text_get());
    add_menu_item(6, 0x60e, text_get());
    add_separator(6, 0);
    add_menu_item(6, 0x60f, text_get());
    add_menu_item(6, 0x610, text_get());
    add_menu_item(6, 0x611, text_get());
    add_menu_item(6, 0x612, text_get());
    add_menu_item(6, 0x613, text_get());
    add_separator(6, 0);
    add_menu_item(6, 0x614, text_get());
    add_menu_item(6, 0x615, text_get());
    add_menu_item(6, 0x616, text_get());
    add_menu_item(6, 0x617, text_get());
    add_separator(6, 0);
    add_menu_item(6, 0x618, text_get());
    add_menu_item(6, 0x619, text_get());
    add_menu_item(6, 0x61a, text_get());
    add_menu_item(6, 0x61b, text_get());
    if (text_open(MenuScriptFile, "EDITOR") != 0) goto done;
    add_menu(0xa, text_get(), 0);
    add_menu_item(0xa, 0xa00, text_get());
    add_menu_item(0xa, 0xa01, text_get());
    add_separator(0xa, 0);
    add_menu_item(0xa, 0xa10, text_get());
    add_menu_item(0xa, 0xa11, text_get());
    add_menu_item(0xa, 0xa12, text_get());
    add_menu_item(0xa, 0xa13, text_get());
    add_menu_item(0xa, 0xa14, text_get());
    add_menu_item(0xa, 0xa15, text_get());
    add_menu_item(0xa, 0xa16, text_get());
    add_separator(0xa, 0);
    add_menu_item(0xa, 0xa17, text_get());
    add_menu_item(0xa, 0xa18, text_get());
    add_menu_item(0xa, 0xa19, text_get());
    add_separator(0xa, 0);
    add_menu_item(0xa, 0xa20, text_get());
    add_menu_item(0xa, 0xa21, text_get());
    add_menu_item(0xa, 0xa22, text_get());
    add_menu_item(0xa, 0xa24, text_get());
    add_separator(0xa, 0);
    add_menu_item(0xa, 0xa48, text_get());
    add_menu_item(0xa, 0xa30, text_get());
    add_menu_item(0xa, 0xa31, text_get());
    add_menu_item(0xa, 0xa32, text_get());
    add_separator(0xa, 0);
    add_menu_item(0xa, 0xa36, text_get());
    add_menu_item(0xa, 0xa37, text_get());
    add_menu_item(0xa, 0xa40, text_get());
    add_menu_item(0xa, 0xa41, text_get());
    add_separator(0xa, 0);
    add_menu_item(0xa, 0xa50, text_get());
    add_menu_item(0xa, 0xa51, text_get());
    add_menu_item(0xa, 0xa52, text_get());
    add_menu_item(0xa, 0xa53, text_get());
    add_separator(0xa, 0);
    add_menu_item(0xa, 0xa54, text_get());
    if (text_open(MenuScriptFile, "HELP") != 0) goto done;
    add_menu(7, text_get(), 1);
    add_menu_item(7, 0x701, text_get());
    add_menu_item(7, 0x702, text_get());
    add_menu_item(7, 0x703, text_get());
    add_separator(7, 0);
    add_menu_item(7, 0x704, text_get());
    add_menu_item(7, 0x70b, text_get());
    add_menu_item(7, 0x70c, text_get());
    add_separator(7, 0);
    add_menu_item(7, 0x70d, text_get());
    add_menu_item(7, 0x70e, text_get());
    add_separator(7, 0);
    add_menu_item(7, 0x70a, text_get());
    add_menu_item(7, 0x705, text_get());
    add_menu_item(7, 0x706, text_get());
    add_menu_item(7, 0x707, text_get());
    add_menu_item(7, 0x708, text_get());
    add_menu_item(7, 0x709, text_get());
    add_separator(7, 0);
    add_menu_item(7, 0x720, text_get());
    add_menu_item(7, 0x721, text_get());
    add_menu_item(7, 0x722, text_get());
    add_menu_item(7, 0x723, text_get());
    add_menu_item(7, 0x724, text_get());
    add_menu_item(7, 0x725, text_get());
    add_separator(7, 0);
    add_menu_item(7, 0x750, text_get());
    add_menu_item(7, 0x751, text_get());
    hide_menu_item(1, 0x106);
    hide_menu_item(1, 0x107);
    hide_menu_item(3, 0x301);
    hide_menu_item(3, 0x309);
    hide_menu_item(3, 0x30a);
    hide_menu_item(3, 0x310);
    hide_menu_item(3, 0x306);
    hide_menu_item(3, 0x304);
    disable_menu_item(3, 0x305);
    if ((GameDataStateStorage.fields_.game_rules_ & RULES_IRONMAN) != 0) {
        hide_menu_item(0, 0x5a);
    }
    check(-1);
    adjust();
    result = 0;
done:
    return result;
}


// ORIGINAL: 0x00460DD0 ?check@MainMenu@@QAEXH@Z 0x00460DD0-0x0046207E
// RECOVERY_BINDING: v1 {"evidence": ["historical retired seed src/unrecovered/00460dd0.cpp", "recovery/tasks/alphamenu-check-owner-oct5.json", "Console embedded member +22A2C, actual constructor and by-name callers", "complete pinned 460DD0..46207E CFG and fresh native current owner symbol; actual-source bounded trace/mutant controls"], "reason": "Recover complete menu policy in the actual Console::alpha_menu_ subobject. Preserve historical MainMenu catalogue identity separately from the observed current AlphaMenu symbol; genuine source ownership corrects the old seed hypothesis. No current byte or runtime acceptance claim."}
// symbol    ?check@AlphaMenu@@QAEHH@Z
// TRIED: time-boxed - roughly the first quarter of the function (a long sequence of Menu::show_menu_item/hide_menu_item/ enable_menu_item gates against game-option and tech flags, transcribed with goto-labelled control flow matching the disassembly's branch targets) is faithful; the difficulty-table-driven remainder (several hundred more show/hide/enable calls) is a deliberate stub. MISMATCH #2, well past the prologue.
int AlphaMenu::check(int veh_id) {
    int i;
    int sea;
    int base_id;
    int owner;
    int turns;
    int avail;
    char turn_text[80];

    if (GameDataStateStorage.fields_.game_more_preferences_ & 0x200000) {
        show_menu_item(0, 0x52);
        hide_menu_item(0, 0x51);
    } else {
        show_menu_item(0, 0x51);
        hide_menu_item(0, 0x52);
    }

    // The image lays the hide-0x5C/debug-mode arm out after the next
    // unconditional jump (0x461020) and falls from show 0x5C straight into the
    // shared show 0x5A; an else arm ending in a goto past that join does this.
    if (!IsMultiplayerNet && !IsMultiplayerPBEM &&
        !(GameDataStateStorage.fields_.game_rules_ & RULES_IRONMAN)) {
        show_menu_item(0, 0x5C);
    } else {
        if (IsMultiplayerNet || IsMultiplayerPBEM) {
            hide_menu_item(0, 0x5C);
        }
        if (!(GameDataStateStorage.fields_.game_state_ & STATE_DEBUG_MODE)) {
            hide_menu_item(0, 0x5A);
            goto expansion_gate;
        }
    }
    show_menu_item(0, 0x5A);
expansion_gate:
    if (GameDataStateStorage.fields_.game_more_preferences_ & 0x200000) {
        show_menu_item(2, 0x205);
        show_menu_item(2, 0x206);
        show_menu_item(2, 0x207);
        show_menu_item(2, 0x208);
        show_menu_item(2, 0x20C);
        show_menu_item(2, 0x215);
        show_menu_item(2, 0x211);
        show_menu_item(2, 0x212);
        show_menu_item(3, 0x31B);
        show_menu_item(3, 0x324);
        show_menu_item(3, 0x306);
        show_menu_item(3, 0x304);
        show_menu_item(3, 0x319);
        show_menu_item(3, 0x31A);
        show_menu_item(3, 0x317);
        show_menu_item(3, 0x31F);
        show_menu_item(3, 0x320);
        show_menu_item(3, 0x307);
        show_menu_item(3, 0x322);
        show_menu_item(9, 0x9F8);
        show_menu_item(9, 0x9F9);
        show_menu_item(9, 0x9F6);
        show_menu_item(9, 0x9F5);
        show_menu_item(9, 0x9F4);
        show_menu_item(9, 0x9F3);
        show_menu_item(9, 0x9F1);
        show_menu_item(9, 0x9F0);
        show_menu_item(9, 5);
        show_menu_item(9, 6);
        show_menu_item(9, 7);
    }

    if (!(GameDataStateStorage.fields_.game_state_ & STATE_IS_SCENARIO) &&
        ScenarioSave.objective_req_victory >= 1000 &&
        ScenarioSave.objectives_sudden_death_victory >= 1000) {
        hide_menu_item(1, 0x115);
    } else {
        show_menu_item(1, 0x115);
    }

    if (IsMultiplayerNet && (GameDataStateStorage.fields_.game_state_ & 2) &&
        NetDaemonNet.factions_waiting_mask() &&
        NetDaemonNet.factions_waiting_mask() != 0xFF) {
        hide_menu_item(8, 0x807);
        show_menu_item(8, 0x808);
    } else {
        show_menu_item(8, 0x807);
        hide_menu_item(8, 0x808);
    }

    if (!(GameDataStateStorage.fields_.game_state_ & 0xC0) &&
        (GameDataStateStorage.fields_.net_turn_flags_ & 8)) {
        hide_menu_item(1, 0x105);
    } else {
        show_menu_item(1, 0x105);
    }

    if (GameDataStateStorage.fields_.game_rules_ & RULES_BLIND_RESEARCH) {
        show_menu_item(1, 0x106);
        hide_menu_item(1, 0x107);
    } else {
        show_menu_item(1, 0x107);
        hide_menu_item(1, 0x106);
    }

    if ((has_tech(Rules.tech_economic_victory, ConsoleGlobal.faction_id_) &&
         (GameDataStateStorage.fields_.game_rules_ & RULES_VICTORY_ECONOMIC)) ||
        ((GameDataStateStorage.fields_.game_state_ & STATE_DEBUG_MODE) &&
         (GameDataStateStorage.fields_.game_state_ & STATE_SCENARIO_EDITOR))) {
        show_menu_item(1, 0x114);
        show_menu_item(1, 9);
    } else {
        hide_menu_item(1, 0x114);
        hide_menu_item(1, 9);
    }

    if (ConsoleGlobal.cursor_active_) {
        show_menu_item(2, 0x201);
        hide_menu_item(2, 0x200);
    } else {
        show_menu_item(2, 0x200);
        hide_menu_item(2, 0x201);
    }

    if (ConsoleGlobal.cursor_active_ && veh_id >= 0) {
        for (i = 0x300; i <= 0x324; i++) {
            enable_menu_item(3, i);
        }
        if (!has_abil(Vehs[veh_id].proto_id, ABL_DROP_POD)) {
            hide_menu_item(3, 0x309);
        } else {
            show_menu_item(3, 0x309);
        }

        owner = whose_territory(ConsoleGlobal.faction_id_, Vehs[veh_id].x,
                                Vehs[veh_id].y, NULL, 0);
        if ((owner >= 0 && owner != ConsoleGlobal.faction_id_ &&
             (PlayersData[ConsoleGlobal.faction_id_].diplo_treaties[owner] & DTREATY_COMMLINK) &&
             !(PlayersData[ConsoleGlobal.faction_id_].diplo_treaties[owner] & DTREATY_VENDETTA)) ||
            (Vehs[veh_id].proto_id < MaxVehProtoFactionNum &&
             (get_proto_offense_rating(Vehs[veh_id].proto_id) < 0 ||
              Vehs[veh_id].proto_id == BSC_SPORE_LAUNCHER))) {
            show_menu_item(3, 0x31F);
        } else {
            hide_menu_item(3, 0x31F);
        }

        if (Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].missile &&
            (VehPrototypes[Vehs[veh_id].proto_id].weapon_id == 0x18 ||
             VehPrototypes[Vehs[veh_id].proto_id].weapon_id == 0x19)) {
            show_menu_item(3, 0x30A);
        } else {
            hide_menu_item(3, 0x30A);
        }

        base_id = base_at(Vehs[veh_id].x, Vehs[veh_id].y);
        if (base_id >= 0 && has_fac(FAC_PSI_GATE, base_id, 0)) {
            show_menu_item(3, 0x310);
        } else {
            hide_menu_item(3, 0x310);
        }

        if (!veh_cargo(veh_id) && !has_abil(Vehs[veh_id].proto_id, ABL_CARRIER)) {
            hide_menu_item(3, 0x31D);
        } else {
            show_menu_item(3, 0x31D);
        }

        if (Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad == TRIAD_AIR &&
            get_proto_offense_rating(Vehs[veh_id].proto_id)) {
            show_menu_item(3, 0x306);
            show_menu_item(3, 0x304);
        } else {
            hide_menu_item(3, 0x306);
            hide_menu_item(3, 0x304);
        }

        if (can_arty(Vehs[veh_id].proto_id, true)) {
            show_menu_item(3, 0x305);
        } else {
            hide_menu_item(3, 0x305);
        }

        if (VehPrototypes[Vehs[veh_id].proto_id].plan == PLAN_SUPPLY_CONVOY) {
            show_menu_item(3, 0x308);
            enable_menu_item(3, 0x308);
        } else {
            hide_menu_item(3, 0x308);
            disable_menu_item(3, 0x308);
        }

        if (VehPrototypes[Vehs[veh_id].proto_id].plan == PLAN_COLONIZATION) {
            enable_menu_item(3, 0x300);
            enable_menu_item(3, 0x301);
            show_menu_item(3, 2);
            if (base_who(Vehs[veh_id].x, Vehs[veh_id].y) >= 0) {
                show_menu_item(3, 0x301);
                hide_menu_item(3, 0x300);
            } else {
                hide_menu_item(3, 0x301);
                show_menu_item(3, 0x300);
            }
            hide_menu_item(3, 0x302);
        } else {
            disable_menu_item(3, 0x300);
            disable_menu_item(3, 0x301);
            hide_menu_item(3, 0x300);
            hide_menu_item(3, 0x301);
            if (get_proto_offense_rating(Vehs[veh_id].proto_id) &&
                base_who(Vehs[veh_id].x, Vehs[veh_id].y) == Vehs[veh_id].faction_id &&
                Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad != TRIAD_AIR) {
                show_menu_item(3, 0x302);
                enable_menu_item(3, 0x302);
            } else {
                disable_menu_item(3, 0x302);
                hide_menu_item(3, 0x302);
            }
            hide_menu_item(3, 2);
        }

        if (get_proto_offense_rating(Vehs[veh_id].proto_id)) {
            show_menu_item(3, 0x320);
            show_menu_item(3, 0x311);
        } else {
            hide_menu_item(3, 0x320);
            hide_menu_item(3, 0x311);
        }

        if (VehPrototypes[Vehs[veh_id].proto_id].plan != PLAN_TERRAFORMING) {
            for (i = 0; i < 19; i++) {
                sea = Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad == TRIAD_SEA ||
                      (Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad == TRIAD_AIR &&
                       is_ocean(Vehs[veh_id].x, Vehs[veh_id].y));
                StringTemp[0] = '\0';
                strcat(StringTemp, StringTable.get((&Order[i + 4].order)[sea]));
                strcat(StringTemp, "|");
                strcat(StringTemp, StringTable.get(Terraforming[i].shortcuts));
                rename_menu_item(9, i + 0x900, StringTemp);
                if (!terrain_avail(i, sea, ConsoleGlobal.faction_id_)) {
                    hide_menu_item(9, i + 0x900);
                    if (i == 18) {
                        hide_menu_item(9, 1);
                    }
                } else {
                    show_menu_item(9, i + 0x900);
                    disable_menu_item(9, i + 0x900);
                    if (i == 18) {
                        show_menu_item(9, 1);
                    }
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
        } else {
            for (i = 0; i < 19; i++) {
                sea = Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad == TRIAD_SEA ||
                      (Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad == TRIAD_AIR &&
                       is_ocean(Vehs[veh_id].x, Vehs[veh_id].y));
                // A flag, not one || condition: the image falls from the
                // farm test into the unavailable row (0x46196D) and defers the
                // road and terrain_avail tests; the available row's "|"
                // literal (0x685470) also precedes the unavailable one.
                if (i == 1 && !(map_loc(Vehs[veh_id].x, Vehs[veh_id].y)->bit & BIT_FARM)) {
                    avail = 0;
                } else if (i == 6 && !(map_loc(Vehs[veh_id].x, Vehs[veh_id].y)->bit & BIT_ROAD)) {
                    avail = 0;
                } else {
                    avail = terrain_avail(i, sea, ConsoleGlobal.faction_id_);
                }
                if (avail) {
                    StringTemp[0] = '\0';
                    strcat(StringTemp, StringTable.get((&Order[i + 4].order)[sea]));
                    turns = action_terraform(veh_id, i, 0);
                    strcat(StringTemp, BufferJoinSpace);
                    strcat(StringTemp, SocialWinOpenParenText);
                    _itoa(turns + 1, turn_text, 10);
                    strcat(StringTemp, turn_text);
                    strcat(StringTemp, BufferJoinSpace);
                    if (turns == 1) {
                        strcat(StringTemp, label_get(0x30));
                    } else {
                        strcat(StringTemp, label_get(0x31));
                    }
                    strcat(StringTemp, SocialWinCloseParenText);
                    strcat(StringTemp, "|");
                    strcat(StringTemp, StringTable.get(Terraforming[i].shortcuts));
                    rename_menu_item(9, i + 0x900, StringTemp);
                    show_menu_item(9, i + 0x900);
                    if ((i != 1 || (map_loc(Vehs[veh_id].x, Vehs[veh_id].y)->bit & BIT_FARM)) &&
                        (i != 6 || (map_loc(Vehs[veh_id].x, Vehs[veh_id].y)->bit & BIT_ROAD))) {
                        enable_menu_item(9, i + 0x900);
                    } else {
                        disable_menu_item(9, i + 0x900);
                    }
                    if (i == 18) {
                        show_menu_item(9, 1);
                    }
                } else {
                    StringTemp[0] = '\0';
                    strcat(StringTemp, StringTable.get(Order[i + 4].order));
                    strcat(StringTemp, "|");
                    strcat(StringTemp, StringTable.get(Terraforming[i].shortcuts));
                    rename_menu_item(9, i + 0x900, StringTemp);
                    disable_menu_item(9, i + 0x900);
                    hide_menu_item(9, i + 0x900);
                    if (i == 18) {
                        hide_menu_item(9, 1);
                    }
                }
            }
            if (Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad == TRIAD_SEA) {
                hide_menu_item(9, 0x9F8);
                hide_menu_item(9, 0x9F9);
            } else {
                show_menu_item(9, 0x9F8);
                show_menu_item(9, 0x9F9);
                enable_menu_item(9, 0x9F8);
                enable_menu_item(9, 0x9F9);
            }
            enable_menu_item(9, 0x9F7);
            enable_menu_item(9, 0x9F2);
            if (has_tech(Terraforming[10].prerequisite_tech[sea], ConsoleGlobal.faction_id_)) {
                enable_menu_item(9, 0x9F6);
            } else {
                disable_menu_item(9, 0x9F6);
            }
            if (has_tech(Terraforming[5].prerequisite_tech[sea], ConsoleGlobal.faction_id_)) {
                enable_menu_item(9, 0x9F0);
                enable_menu_item(9, 0x9F3);
            } else {
                disable_menu_item(9, 0x9F0);
                disable_menu_item(9, 0x9F3);
            }
            if (has_tech(Terraforming[6].prerequisite_tech[sea], ConsoleGlobal.faction_id_)) {
                show_menu_item(9, 0x9F1);
                show_menu_item(9, 0x9F4);
                enable_menu_item(9, 0x9F1);
                enable_menu_item(9, 0x9F4);
            } else {
                hide_menu_item(9, 0x9F1);
                hide_menu_item(9, 0x9F4);
            }
            if (has_tech(Terraforming[9].prerequisite_tech[sea], ConsoleGlobal.faction_id_)) {
                show_menu_item(9, 0x9F5);
                enable_menu_item(9, 0x9F5);
            } else {
                hide_menu_item(9, 0x9F5);
            }
        }

        if (base_who(Vehs[veh_id].x, Vehs[veh_id].y) >= 0) {
            hide_menu_item(3, 0x317);
            show_menu_item(3, 0x316);
        } else {
            show_menu_item(3, 0x317);
            hide_menu_item(3, 0x316);
        }
    } else {
        for (i = 0x300; i <= 0x324; i++) {
            if (i != 0x313) {
                disable_menu_item(3, i);
            } else {
                enable_menu_item(3, 0x313);
            }
        }
    }

    for (i = 0x601; i <= 0x61B; i++) {
        if (GameDataStateStorage.fields_.game_state_ & STATE_SCENARIO_EDITOR) {
            enable_menu_item(6, i);
        } else {
            disable_menu_item(6, i);
        }
    }
    for (i = 0xA00; i <= 0xA54; i++) {
        if (GameDataStateStorage.fields_.game_state_ & STATE_SCENARIO_EDITOR) {
            enable_menu_item(10, i);
        } else {
            disable_menu_item(10, i);
        }
    }

    if (!(GameDataStateStorage.fields_.game_more_preferences_ & 0x200000)) {
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
    return 0;
}
