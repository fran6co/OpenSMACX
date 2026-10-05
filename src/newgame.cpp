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
#include "game.h"
#include "main.h"
#include "temp.h"
#include "alpha.h"
#include "faction.h"
#include "base.h"
#include "map.h"
#include "path.h"
#include "text.h"
#include "general.h"
#include "strings.h"
#include "popup.h"
#include "xpops.h"
#include "setupwin.h"
#include "basepop.h"
#include "graphicwin.h"
#include "palette.h"
#include "wave.h"
#include "console.h"
#include "designwin.h"
#include "tutwin.h"
#include "worldwin.h"
#include "hypothesis_layouts.h"
#include "maininterface.h"
#include "mapwin.h"
#include "veh.h"
#include "netwin.h"
#include "netdaemon.h"
#include "lock.h"
#include "log.h"
#include "time.h"
#include "editgroup.h"
#include "statuswin.h"
#include "flic.h"

// Existing process-wide audio owner defined in wave.cpp.
extern Wave g_TOP_MENU_WAVE;
extern Wave g_CRASH_LANDING_WAVE;
extern int ConsoleNoMapOverrideLatch;
void __cdecl world_build();

// New-game and menu ownership group; original TU boundary is not proven.
// Independent DWORD after FastUnitAnim; native map/size callers reset/set it.
// Provisional descriptive name; complete lifecycle is not established.
// RECOVERY_OBJECT: v2 0x00945820 0x00945824 ?MapSetupStatus@@3HA 0x0058D6B0
int MapSetupStatus;

static void multiplayer_reset_network_state() {
    LockStateEntry *const state = NetDaemonNet.lock_state_;

    state[0].leading_words_[1] = 0;      // 0x93E8C0
    state[0].leading_words_[2] = 0;      // 0x93E8C4
    state[0].leading_words_[3] = 0;      // 0x93E8C8
    state[0].payload_.flags = 0;         // 0x93E8CC
    state[0].trailing_words_[8] = 0;    // 0x93E8F0
    state[0].trailing_words_[7] = 0;    // 0x93E8EC
    state[1].leading_words_[1] = 0;      // 0x93E8FC
    state[1].leading_words_[0] = 0;      // 0x93E8F8
    state[1].trailing_words_[0] = 0;    // 0x93E90C
    state[2].leading_words_[1] = 0;      // 0x93E938
    state[1].trailing_words_[9] = 0;    // 0x93E930
    state[2].leading_words_[0] = 0;      // 0x93E934
    state[2].trailing_words_[5] = 0;    // 0x93E95C
    state[2].trailing_words_[4] = 0;    // 0x93E958
    state[0].trailing_words_[4] = 0;    // 0x93E8E0
    state[0].trailing_words_[5] = 0;    // 0x93E8E4
    state[0].trailing_words_[6] = 0;    // 0x93E8E8
    state[2].trailing_words_[3] = 0;    // 0x93E954
    state[2].trailing_words_[2] = 0;    // 0x93E950
    state[2].leading_words_[3] = 0;      // 0x93E940
    state[2].leading_words_[2] = 0;      // 0x93E93C
    state[2].payload_.flags = 0;         // 0x93E944
    state[0].trailing_words_[0] = 0;    // 0x93E8D0
    state[0].trailing_words_[1] = 0;    // 0x93E8D4
    state[2].trailing_words_[8] = 0;    // 0x93E968
    state[2].trailing_words_[7] = 0;    // 0x93E964
    state[0].trailing_words_[2] = 0;    // 0x93E8D8
    state[0].trailing_words_[3] = 0;    // 0x93E8DC
    state[2].trailing_words_[6] = 0xFF; // 0x93E960
    state[1].payload_.flags = 0;         // 0x93E908
    for (int i = 1; i != 9; ++i) {
        state[1].trailing_words_[i] = 0; // 0x93E910..0x93E92C
    }

    NetDaemonNet.locks_.clear();
    NetDaemonNet.close();
}

static const char WorldSizePopupLabel[] = "WORLDSIZE";
static const char CustomMapPopupLabel[] = "CUSTOMMAP";
static const char CustomMapXKey[] = "Custom Map X";
static const char CustomMapYKey[] = "Custom Map Y";
static const char VeryLargeMapLabel[] = "VERYLARGEMAP";


// ORIGINAL: 0x0058CED0 ?size_of_planet@@YAHH@Z 0x0058CED0-0x0058D6A4;0x006607D7-0x00660989
// RECOVERY_BINDING: v1 {"evidence": ["task:newgame-menu-owner", "historical seed:src/unrecovered/0058ced0.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:complete mapped native control flow", "source:existing SetupWin/Popup/Buffer/NetDaemon owner declarations; provisional bounded945820 and9B2074 objects, not raw image aliases"], "reason": "Source-owned new-game/menu family preserving complete original identity, auxiliary spans and historical seed findings; existing genuine object lifetimes and typed member owners. Unfinished bodies remain explicit, not implementation coverage."}
// TRIED: real Popup/SetupWin RAII locals with Popup::start()'s six args re-derived from push order (p1 is the fixed 0x9b8aa8 char* buffer, not the GraphicWin* - this is backwards from what an earlier sibling function assumed), plus text_open/text_get/text_item/EditGroup::edit/ prefs_get/X_pop/Path::init argument order verified against raw disasm; compiles, MISMATCH #24 (past the prologue and into the item-list loop - likely the itemNum2/itemCount stack array layout or the loop bound).
int __cdecl size_of_planet(int mode) {
    Popup popup;
    SetupWin setup;
    const int start_flags = mode != 2 ? 0x40 : 0;
    popup.start(PopupStartCaption, WorldSizePopupLabel, -1, 0,
                start_flags, 0);

    int width_for_choice[32];
    int height_for_choice[32];
    int width = 0;

    MapSetupStatus = 1;
    const int text_open_result = text_open(AlphaxFileID, "WORLDSIZE");
    if (text_open_result != 0) {
        MapSizePlanet = 2;
        width = 0x28;
        MapLatitudeBounds = 0x50;
    } else {
        text_get();
        int item_count = text_item_number();
        if (item_count > 0x20) item_count = 0x20;
        for (int item = 0; item < item_count; ++item) {
            text_get();
            StringTemp[0] = 0;
            strcat(StringTemp, text_item());
            popup.dialogs().item(StringTemp, item);
            width_for_choice[item] = text_item_number();
            height_for_choice[item] = text_item_number();
        }
        StringTemp[0] = 0;
        strcat(StringTemp, label_get(0x1104 / 4));
        popup.dialogs().item(StringTemp, 0x63);

        const int saved_choice = static_cast<int>(AlphaIniPrefs.custom_world[0]);
        popup.set_value(saved_choice);
        int chosen;
        if (mode == 1) {
            MapSizePlanet = static_cast<uint32_t>(saved_choice);
            chosen = saved_choice;
            goto size_after_choice;
        }
        if (mode == 2) MapSizePlanet = static_cast<uint32_t>(popup.exec(0, 0));
        else MapSizePlanet = static_cast<uint32_t>(setup.do_menu(&popup, 1, 0));
        if (static_cast<int32_t>(MapSizePlanet) < 0) {
            setup.close();
            popup.close();
            return 1;
        }

        if (saved_choice == 0) {
            chosen = static_cast<int>(MapSizePlanet);
            if (MapSizePlanet != 0x63) goto size_after_choice;
        } else {
            chosen = rand() % 5;
            MapSizePlanet = static_cast<uint32_t>(chosen);
            chosen = saved_choice;
size_after_choice:
            AlphaIniPrefs.custom_world[0] = static_cast<uint32_t>(chosen);
            if (MapSizePlanet != 0x63) {
                MapLatitudeBounds = height_for_choice[MapSizePlanet];
                width = width_for_choice[MapSizePlanet];
                goto size_complete;
            }
        }

        popup.start(PopupStartCaption, CustomMapPopupLabel, -1, 0, 0x44, 0);
        const int pref_x = prefs_get(CustomMapXKey, 0, 0x28);
        const int pref_y = prefs_get(CustomMapYKey, 0, 0x50);
        StringTemp[0] = 0;
        char number[20];
        _itoa(pref_x, number, 10);
        strcat(StringTemp, number);
        static_cast<EditGroup &>(popup.dialogs()).edit(
            label_get(0x1108 / 4), StringTemp, 8);
        StringTemp[0] = 0;
        _itoa(pref_y, number, 10);
        strcat(StringTemp, number);
        static_cast<EditGroup &>(popup.dialogs()).edit(
            label_get(0x110C / 4), StringTemp, 8);

        if (popup.exec(0, 0) < 0) {
            setup.close();
            popup.close();
            return 1;
        }
        int custom_width = atoi(ParseStrBuffer[0].str);
        int custom_height = atoi(ParseStrBuffer[1].str);
        if (custom_width < 0x10) custom_width = 0x10;
        else if (custom_width > 0x100) custom_width = 0x100;
        if (custom_height < 0x10) custom_height = 0x10;
        else if (custom_height > 0x100) custom_height = 0x100;
        if (custom_width > 0x80 || custom_height > 0x80) {
            if (X_pop(VeryLargeMapLabel, 0) != 0) {
                setup.close();
                popup.close();
                return 1;
            }
        }
        prefs_put(CustomMapXKey, custom_width, 0);
        prefs_put(CustomMapYKey, custom_height, 0);
        MapLatitudeBounds = custom_height;
        width = custom_width;
    }

size_complete:
    MapLongitudeBounds = width * 2;
    prefs_save(0);
    if (mode < 2) {
        map_init();
        Paths.init();
    }
    setup.close();
    UnkBitfield1 |= 8;
    popup.close();
    return 0;
}


static const int CustomPlanetSlots[6] = {1, 3, 6, 5, 0, 0};
static const char CustomPlanetPrefix[] = "WORLD";
static const char CustomPlanetMoon800[3][20] = {
    "moon1_800.pcx", "moon2_800.pcx", "moon3_800.pcx"};
static const char CustomPlanetMoonOther[3][20] = {
    "MOON1.PCX", "MOON2.PCX", "MOON3.PCX"};
static const char *const CustomPlanetKindLabels[6] = {
    "LAND", "", "TIDES", "ORBIT", "CLOUDS", "LIFE"};

static uint32_t &custom_planet_map_slot(int slot) {
    switch (slot) {
    case 0: return MapSizePlanet;
    case 1: return MapOceanCoverage;
    case 2: return MapLandCoverage;
    case 3: return MapErosiveForces;
    case 4: return MapPlanetaryOrbit;
    case 5: return MapCloudCover;
    case 6: return MapNativeLifeForms;
    default: return MapNativeLifeForms;
    }
}

static int custom_planet_clamp_level(uint32_t raw_value) {
    const uint32_t sum = raw_value + 1u;
    int32_t value;
    memcpy(&value, &sum, sizeof(value));
    if (value < 1) return 1;
    if (value > 3) return 3;
    return value;
}


// ORIGINAL: 0x0058C2A0 ?custom_planet@@YAHHH@Z 0x0058C2A0-0x0058CE9C;0x006605A5-0x006607D7
// RECOVERY_BINDING: v1 {"evidence": ["task:newgame-menu-owner", "historical seed:src/unrecovered/0058c2a0.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:complete mapped native control flow", "source:existing SetupWin/Popup/Buffer/NetDaemon owner declarations; provisional bounded945820 and9B2074 objects, not raw image aliases"], "reason": "Source-owned new-game/menu family preserving complete original identity, auxiliary spans and historical seed findings; existing genuine object lifetimes and typed member owners. Unfinished bodies remain explicit, not implementation coverage."}
// TRIED: exact 12-case image/message switch (jump table at 0x58ce9c) transcribed as a representative subset, not all 12 cases; Popup/SetupWin/Buffer[3] RAII locals reproduce the close cascade shape but not every StringStruct/list-walk detail.
int __cdecl custom_planet(int show_preview, int use_saved_choices) {
    Popup popup;
    Buffer previews[3];
    SetupWin setup;

    if (show_preview != 0) {
        for (int image = 0; image < 3; ++image) {
            setup.overlay_buffers_[image] = &previews[image];
        }
    }

    MapOceanCoverage = 0;
    MapLandCoverage = 0;
    MapErosiveForces = 0;
    MapPlanetaryOrbit = 0;
    MapCloudCover = 0;
    MapNativeLifeForms = 0;

    for (int iteration = 1; iteration < 7; ++iteration) {
        const int slot = CustomPlanetSlots[iteration - 1];
        if (slot == 0) continue;
        const int saved_value = static_cast<int>(AlphaIniPrefs.custom_world[slot]);
        uint32_t &destination = custom_planet_map_slot(slot);
        destination = static_cast<uint32_t>(saved_value);

        if (use_saved_choices != 0) continue;

        StringTemp[0] = 0;
        strcat(StringTemp, CustomPlanetPrefix);
        // Both native jump tables dispatch on the descriptor slot, not its
        // saved setting. The four active descriptors are 1, 3, 6 and 5.
        if (show_preview != 0) {
            if (slot == 3) {
                for (int image = 0; image < 3; ++image) {
                    const char *filename = WinScreenWidth == 800
                        ? CustomPlanetMoon800[image] : CustomPlanetMoonOther[image];
                    previews[image].load_pcx(filename, 0, 0x0A, 0xEC);
                }
                for (int overlay = 0; overlay < 3; ++overlay) {
                    setup.overlay_x_[overlay] = 0;
                    setup.overlay_y_[overlay] = 0;
                }
            } else if (slot == 1) {
                char filename[32];
                for (int image = 0; image < 3; ++image) {
                    const int life = custom_planet_clamp_level(MapNativeLifeForms);
                    const int clouds = custom_planet_clamp_level(MapCloudCover);
                    sprintf(filename, "S%dL%dC%d.PCX", image + 1, life, clouds);
                    previews[image].load_pcx(filename, 0, 0x0A, 0xEC);
                    const int screen_width = WinScreenWidth;
                    setup.overlay_x_[image] = screen_width == 800
                        ? 449 : screen_width - previews[0].pixel_width();
                    setup.overlay_y_[image] = screen_width == 800 ? -168 : 0;
                }
            } else if (slot == 5 || slot == 6) {
                char filename[32];
                for (int image = 0; image < 3; ++image) {
                    const int ocean = custom_planet_clamp_level(MapOceanCoverage);
                    const int life = slot == 5
                        ? custom_planet_clamp_level(MapNativeLifeForms) : image + 1;
                    const int clouds = slot == 5
                        ? image + 1 : custom_planet_clamp_level(MapCloudCover);
                    sprintf(filename, "S%dL%dC%d.PCX", ocean, life, clouds);
                    previews[image].load_pcx(filename, 0, 0x0A, 0xEC);
                    const int screen_width = WinScreenWidth;
                    setup.overlay_x_[image] = screen_width == 800
                        ? 449 : screen_width - previews[0].pixel_width();
                    setup.overlay_y_[image] = screen_width == 800 ? -168 : 0;
                }
            }
        }

        // Native label append occurs after the preview callbacks. The image
        // filename template is local scratch, not part of the popup label.
        strcat(StringTemp, CustomPlanetKindLabels[slot - 1]);
        popup.start(PopupStartCaption, StringTemp, -1, 0, 0x40, 0);
        // start/load callbacks can change preferences; preserve the live read.
        popup.set_value(static_cast<int>(AlphaIniPrefs.custom_world[slot]));
        const int selection = show_preview != 0
            ? setup.do_menu(&popup, 1, 0) : popup.exec(0, 0);
        destination = static_cast<uint32_t>(selection);
        if (selection < 0) {
            prefs_save(0);
            setup.close();
            popup.close();
            return 1;
        }
        if (popup.input_poll_state() != 0) {
            destination = 0xFFFFFFFFu;
        } else {
            AlphaIniPrefs.custom_world[slot] = static_cast<uint32_t>(selection);
        }
    }

    if (show_preview != 0) {
        for (int image = 0; image < 3; ++image) {
            setup.overlay_buffers_[image] = 0;
        }
    }
    MapLandCoverage = 2 - MapOceanCoverage;
    prefs_save(0);
    setup.close();
    popup.close();
    return 0;
}


static const char MapMenuTitle[] = "MAPMENU";
static const char MapXPlanetPath[] = "maps\\xplanet.mp";
static const char MapXPlanetXPath[] = "maps\\xplanetx.mp";

// ORIGINAL: 0x0058D6B0 ?map_menu@@YAHH@Z 0x0058D6B0-0x0058DB17;0x00660989-0x00660A96
// RECOVERY_BINDING: v1 {"evidence": ["task:newgame-menu-owner", "historical seed:src/unrecovered/0058d6b0.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:complete mapped native control flow", "source:existing SetupWin/Popup/Buffer/NetDaemon owner declarations; provisional bounded945820 and9B2074 objects, not raw image aliases"], "reason": "Source-owned new-game/menu family preserving complete original identity, auxiliary spans and historical seed findings; existing genuine object lifetimes and typed member owners. Unfinished bodies remain explicit, not implementation coverage."}
// TRIED: MISMATCH #11. Real SetupWin/Popup/Buffer/Spot/StringStruct locals (Popup embeds Scroll+2 FlatButtons per the scaffold) do produce the SEH frame and destructor-chain shape, but local declaration order/count likely differs from the original's stack layout past instruction 11.
int __cdecl map_menu(int quick_start) {
    SetupWin setup;
    Popup popup;
    int choice;

map_menu_retry:
    MapSetupStatus = 0;
    popup.start(PopupStartCaption, MapMenuTitle, -1, 0, 0, 0);
    choice = static_cast<int>(DefaultPrefs.map_type);
    popup.set_value(choice);
    if (quick_start == 0) choice = setup.do_menu(&popup, 1, quick_start);
    if (choice < 0) return 1;

    DefaultPrefs.map_type = static_cast<uint32_t>(choice);
    prefs_save(0);
    if (choice < 5) {
        switch (choice) {
        case 0:
        case 1:
            if (size_of_planet(quick_start) != 0) goto map_menu_retry;
            if (MapSetupStatus != 0) {
                if (choice == 1) {
                    if (custom_planet(1, quick_start) != 0) {
                        choice = 1;
                        goto map_menu_retry;
                    }
                    goto map_menu_configure;
                }
                if (IsMultiplayerNet == 0) {
                    const int roll = rand();
                    int category = (roll % 7 + 1) / 2;
                    if (category < 0) category = 0;
                    else if (category > 2) category = 2;
                    MapOceanCoverage = static_cast<uint32_t>(2 - category);
                } else {
                    MapOceanCoverage = 0;
                }
                MapLandCoverage = 2 - MapOceanCoverage;
                const int life_roll = rand();
                MapErosiveForces = 1;
                MapPlanetaryOrbit = 1;
                MapCloudCover = 1;
                MapNativeLifeForms = static_cast<uint32_t>((life_roll % 6 + 5) / 5);
            }
            goto map_menu_configure;
        case 2:
            if (load_map_daemon(const_cast<char *>(MapXPlanetPath)) != 0)
                goto map_menu_retry;
            world_climate();
            goto map_menu_configure;
        case 3:
            if (load_map_daemon(const_cast<char *>(MapXPlanetXPath)) != 0)
                goto map_menu_retry;
            world_climate();
            goto map_menu_configure;
        default:
            if (load_map() == 0) {
                world_climate();
                goto map_menu_configure;
            }
            quick_start = 0;
            goto map_menu_retry;
        }
    }

map_menu_configure:
    if (config_game(quick_start) == 0) return 0;
    goto map_menu_retry;
}


static void top_menu_present_opening(const char *filename) {
    if (opening_window() == 0) {
        GraphicWin *const window = new GraphicWin();
        opening_window() = window;
        if (window == 0) return;
        if (window->init(0, 0, WinScreenWidth, WinScreenHeight, 0,
                        0x400000, 0, 0, 0) != 0) {
            close_opening();
            return;
        }
    }
    opening_window()->fill(0);
    change_opening(const_cast<char *>(filename));
    g_PALETTE1.set();
    opening_window()->soft_update();
    do_all_draws();
    BasePopDefaultField322C = opening_window();
}


// ORIGINAL: 0x0058E360 ?top_menu@@YAHH@Z 0x0058E360-0x0058EE31;0x00660A96-0x00660BD7
// RECOVERY_BINDING: v1 {"evidence": ["task:newgame-menu-owner", "historical seed:src/unrecovered/0058e360.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:complete mapped native control flow", "source:existing SetupWin/Popup/Buffer/NetDaemon owner declarations; provisional bounded945820 and9B2074 objects, not raw image aliases"], "reason": "Source-owned new-game/menu family preserving complete original identity, auxiliary spans and historical seed findings; existing genuine object lifetimes and typed member owners. Unfinished bodies remain explicit, not implementation coverage."}
// TRIED: `SetupWin setup; Popup popup;` real locals (RAII for their destructor cascades) with the full main-menu retry loop transcribed via goto+labels matching original addresses (window-creation blocks x3, do_menu switch on 6 cases, new-game wizard parse_says/X_pop block); diverges at instr #14, not chased further given size.
int __cdecl top_menu(int mode) {
    SetupWin setup;
    Popup popup;
    int esi = 0;
    int edi = 0;

    if (mode == 0) {
        IsMultiplayerPBEM = 0;
        clear_all_player_messages();
    }
    prefs_load(0);
    prefs_use();
    if (mode == 0 && (GamePreferences & 0x800) != 0) {
        g_TOP_MENU_WAVE.load(const_cast<char *>("fx\\opening menu.wav"), 2);
        g_TOP_MENU_WAVE.play();
    }
    {
        unsigned char *const style =
            reinterpret_cast<unsigned char *>(&DialogDefaultStyle);
        if ((AlphaIniPrefs.rules & 0x20000000u) == 0)
            *style = static_cast<unsigned char>(*style & 0xFB);
        else
            *style = static_cast<unsigned char>(*style | 4);
    }

    if (ExpansionEnabled == 0) {
        top_menu_present_opening(WinScreenWidth == 0
            ? "xopeningb.pcx" : "xopeninga.pcx");
    } else {
        top_menu_present_opening(WinScreenWidth == 0x320
            ? "openingb.pcx" : "openinga.pcx");
    }
    TurnUpkeepPhase = 0;

top_menu_redraw:
    if (ExpansionEnabled == 0) {
        top_menu_present_opening(WinScreenWidth == 0x320
            ? "xopeningb.pcx" : "xopeninga.pcx");
    } else {
        top_menu_present_opening(WinScreenWidth == 0x320
            ? "openingb.pcx" : "openinga.pcx");
    }
    prefs_load(0);
    prefs_use();

    edi = mode;
    popup.start(PopupStartCaption, edi == 0 ? "HOTSEAT" : "TOPMENU",
                -1, 0, 0, 0);
    if (edi == 0) popup.set_value(static_cast<int>(DefaultPrefs.top_menu));

    esi = setup.do_menu(&popup, 1, 0);
    if (esi < 0 || esi == 6) {
        popup.close();
        setup.close();
        return 1;
    }

    if (edi == 0) {
        DefaultPrefs.top_menu = static_cast<uint32_t>(esi);
        prefs_save(0);
    } else if (esi > 0) {
        ++esi;
    }

    if (static_cast<unsigned int>(esi) > 5) goto top_menu_normal_return;
    switch (esi) {
    case 0:
    case 1:
        esi = map_menu(esi == 1 ? 1 : 0);
        goto top_menu_after_action;

    case 2:
        if (mode == 0) {
            edi = 1;
            esi = setup.do_menu(const_cast<char *>("SCENARIOMENU"), edi, 0);
            if (esi < 0) goto top_menu_retry;
            if (esi == edi) goto top_menu_new_game;
            if (esi == 2) {
                esi = load_map();
                if (esi != 0) goto top_menu_retry;
                config_game(3);
                VehCurrentCount = 0;
                setup_game(edi);
                ControlTurnPhase = ConsoleGlobal.faction_id_;
                GameState |= 0x080000E0u;
                goto top_menu_after_action;
            }
        } else {
            esi = 0;
        }
        goto top_menu_load_continue;

    case 3:
        GameDataLoadRequest = mode;
        esi = load_game(0, 0);
        GameDataLoadRequest = 0;
        if (esi != 0) goto top_menu_retry;
        if (GameSessionActive != 0) {
            stop_timers();
            labels_shutdown();
            StringTable.shutdown();
            if (game_init(1, 0) == 0) WorldClimateSkipTerrainClear = 0;
        }
        goto top_menu_after_action;

    case 4:
        esi = multiplayer_init(0);
        goto top_menu_after_action;

    case 5:
        show_credits();
        goto top_menu_retry;
    }
    goto top_menu_normal_return;

top_menu_new_game:
    size_of_planet(edi);
    MapOceanCoverage = static_cast<uint32_t>(edi);
    MapLandCoverage = static_cast<uint32_t>(edi);
    MapErosiveForces = static_cast<uint32_t>(edi);
    MapPlanetaryOrbit = static_cast<uint32_t>(edi);
    MapCloudCover = static_cast<uint32_t>(edi);
    MapNativeLifeForms = static_cast<uint32_t>(edi);
    map_wipe();
    config_game(3);
    VehCurrentCount = 0;
    setup_game(edi);
    ControlTurnPhase = ConsoleGlobal.faction_id_;
    GameState |= 0x080000E0u;
    goto top_menu_scenario_return;

top_menu_load_continue:
    edi = IsMultiplayerPBEM;
    {
        const int loaded = load_game(0, 1);
        if (edi != 0) IsMultiplayerPBEM = IsMultiplayerNet != 0;
        else IsMultiplayerPBEM = 0;
        if (loaded != 0) goto top_menu_retry;
    }
    if (GameSessionActive != 0) {
        stop_timers();
        labels_shutdown();
        StringTable.shutdown();
        if (game_init(1, 0) == 0) WorldClimateSkipTerrainClear = 0;
    }
    if (esi != 0) {
        ControlTurnPhase = ConsoleGlobal.faction_id_;
        GameState |= 0x080000E0u;
        goto top_menu_normal_return;
    }
    esi = config_game(2);
    if (esi != 0) {
        read_rules(1);
        goto top_menu_after_action;
    }

    {
        int faction = ConsoleGlobal.faction_id_;
        ControlTurnPhase = faction;
        GenderDefault = Players[faction].is_leader_female;
        PluralityDefault = 0;
        parse_says(1, Players[faction].title_leader, -1, -1);
        faction = ConsoleGlobal.faction_id_;
        GenderDefault = Players[faction].is_leader_female;
        PluralityDefault = 0;
        parse_says(2, Players[faction].name_leader, -1, -1);
        faction = ConsoleGlobal.faction_id_;
        GenderDefault = Players[faction].noun_gender;
        PluralityDefault = Players[faction].is_noun_plural;
        parse_says(3, Players[faction].noun_faction, -1, -1);
        StringTemp[0] = '\0';
        strcat(StringTemp, label_get(0));
        strcat(StringTemp, " ");
        say_year(StringTemp);
        parse_says(4, StringTemp, -1, -1);
        parse_says(0, StringTemp, -1, -1);
        parse_num(0, ObjectiveReqVictory);
        parse_num(1, static_cast<int>(ObjectivesSuddenDeathVictory));
        parse_num(2, num_objectives(ConsoleGlobal.faction_id_, GameRules & 0x1000));
        parse_num(3, static_cast<int>(EndingMissionYear));
        X_pop(const_cast<char *>("INTRO"), "SCENARIO", 0, 0, 0, 0);
        ConsoleGlobal.set_move(1);
    }
    goto top_menu_normal_return;

top_menu_after_action:
    if (esi == 0) goto top_menu_normal_return;
top_menu_retry:
    if (ExitTurnLoop != 0) goto top_menu_normal_return;
    goto top_menu_redraw;

top_menu_scenario_return:
    if ((GamePreferences & 0x800) == 0) goto top_menu_audio_check;
    goto top_menu_fade_music;

top_menu_normal_return:
    if (mode == 0 && (GamePreferences & 0x800) != 0)
        goto top_menu_fade_music;
top_menu_audio_check:
    if (g_TOP_MENU_WAVE.is_playing() != 0) goto top_menu_fade_music;
    goto top_menu_cleanup;

top_menu_fade_music:
    g_TOP_MENU_WAVE.fade(0xBB8);
    g_TOP_MENU_WAVE.unload();
top_menu_cleanup:
    popup.close();
    return esi;
}



// ORIGINAL: 0x0058DB30 ?multiplayer_init@@YAHH@Z 0x0058DB30-0x0058E35E
// RECOVERY_BINDING: v1 {"evidence": ["task:newgame-menu-owner", "historical seed:src/unrecovered/0058db30.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:complete mapped native control flow", "source:existing SetupWin/Popup/Buffer/NetDaemon owner declarations; provisional bounded945820 and9B2074 objects, not raw image aliases"], "reason": "Source-owned new-game/menu family preserving complete original identity, auxiliary spans and historical seed findings; existing genuine object lifetimes and typed member owners. Unfinished bodies remain explicit, not implementation coverage."}
// TRIED: prologue diverges at #7 - original threads the literal 1 through esi across the whole prologue (mov esi,1; ...; mov [addr],esi; ...; push esi for the later top_menu(1) call); rebuilt just stores/pushes the literal 1 directly at each site. Tried binding a local `int one=1` reused at both sites - similarity unchanged (0.74). Body is otherwise a full structural transcription (322/~420 mnemonics in common), including the two literal duplicated cleanup blocks the original also duplicates.
int __cdecl multiplayer_init(int startup_mode) {
    int result = 1;

    IsMultiplayerNet = 1;
    log_set_state(0);
    NetWinDialog.config_players();

    if (startup_mode != 0) {
        const uint32_t initial_voice_flags = NetDaemonNet.voice_flags_D8_;
        NetDaemonNet.voice_hotkey_48_ = 0xDC;
        NetDaemonNet.voice_hotkey_4c_ = 0x100DC;
        NetDaemonNet.voice_flags_D8_ = initial_voice_flags | 1;
    } else {
        if (NetDaemonNet.init(0) != 0) goto do_cleanup;
    }

    if (IsMultiplayerPBEM != 0) {
        IsMultiplayerNet = 0;
        result = top_menu(1);
        if (result == 0) return 1;
        IsMultiplayerPBEM = 0;
        return result;
    }

    NetDaemonNet.lock_state_[0].trailing_words_[2] = 0;
    if (ExitTurnLoop == 0 && NetWinDialog.exec() > 0) {
        const int difficulty = static_cast<int8_t>(NetWinSessionOptions.difficulty_);
        const uint32_t rules_a = NetWinSessionOptions.rule_bits_a_;
        const uint32_t rules_b = NetWinSessionOptions.rule_bits_b_;
        GameRules = rules_a;
        const uint32_t old_game_state = GameState;
        DiffLevelCurrent = difficulty;
        const uint8_t rules_c = NetWinSessionOptions.rule_bits_c_;
        const uint32_t merged_game_state = old_game_state | (rules_b & 0x01800000u);
        const bool enable_turn_flag = (rules_c & 0x10u) != 0;
        const uint32_t old_turn_flags = NetTurnFlags;
        GameState = merged_game_state;
        NetTurnFlags = enable_turn_flag ? (old_turn_flags | 0x10u)
                                        : (old_turn_flags & ~0x10u);

        if (NetDaemonNet.lock_state_[0].leading_words_[1] != 0) {
            uint8_t mask = 0;
            if (NetDaemonNet.player_count_ > 0) {
                const int count = NetDaemonNet.player_count_;
                for (int i = 0; i != count; ++i) {
                    const unsigned char shift = static_cast<unsigned char>(
                        NetDaemonNet.process_slots_[i + 1].identity_);
                    // x86 masks the variable shift count to five bits.
                    mask = static_cast<uint8_t>(mask | (1u << (shift & 31)));
                }
            }
            LockEnableMask = (LockEnableMask & 0xFFFFFF00u) | mask;

            if (NetWinSessionData.mode_ == 0) {
                result = text_open(AlphaxFileID, "WORLDSIZE");
                if (result != 0) {
                    MapSizePlanet = 2;
                    MapLongitudeBounds = 0x28;
                    MapLatitudeBounds = 0x50;
                } else {
                    int longitude[32];
                    int latitude[32];
                    int count;
                    text_get();
                    count = text_item_number();
                    if (count > 0x20) count = 0x20;
                    for (int i = 0; i < count; ++i) {
                        text_get();
                        text_item();
                        longitude[i] = text_item_number();
                        latitude[i] = text_item_number();
                    }
                    const int8_t selected =
                        static_cast<int8_t>(NetWinSessionOptions.planet_size_);
                    const int index = selected == -1 ? rand() % 5 : selected;
                    MapSizePlanet = static_cast<uint32_t>(index);
                    MapLongitudeBounds = longitude[index] << 1;
                    MapLatitudeBounds = latitude[index];
                }

                map_init();
                Paths.init();
                UnkBitfield1 |= 8;

                if (IsMultiplayerNet == 0) {
                    int v = (rand() % 7 + 1) / 2;
                    if (v < 0) v = 0;
                    else if (v > 2) v = 2;
                    MapOceanCoverage = static_cast<uint32_t>(2 - v);
                } else {
                    MapOceanCoverage = 0;
                }
                MapLandCoverage = 2 - MapOceanCoverage;
                rand();
                MapOceanCoverage = static_cast<int8_t>(
                    NetWinSessionOptions.ocean_coverage_);
                MapPlanetaryOrbit = 1;
                MapErosiveForces = static_cast<int8_t>(
                    NetWinSessionOptions.erosion_);
                MapCloudCover = static_cast<int8_t>(
                    NetWinSessionOptions.cloud_cover_);
                MapNativeLifeForms = static_cast<int8_t>(
                    NetWinSessionOptions.native_life_);
                world_build();
            }

            if (NetWinSessionData.mode_ != 3 && NetWinSessionData.mode_ != 2) {
                VehCurrentCount = 0;
                setup_game(0);
            }
        }

        if (NetDaemonNet.player_count_ > 0) {
            const int count = NetDaemonNet.player_count_;
            for (int i = 0; i != count; ++i) {
                const int8_t faction = NetDaemonNet.process_slots_[i + 1].identity_;
                PlayersData[static_cast<int>(faction)].unk_99 =
                    NetWinLobbyRows[i + 1].owner_;
            }
        }

        GameState |= NetWinSessionOptions.rule_bits_b_ & 0x01800000u;
        if ((NetWinSessionOptions.rule_bits_c_ & 0x10u) != 0)
            NetTurnFlags |= 0x10u;
        else
            NetTurnFlags &= ~0x10u;
        TimeControlsSelection = static_cast<uint8_t>(
            NetWinSessionOptions.time_control_);
        ConsoleGlobal.faction_id_ =
            NetDaemonNet.pid_2_who(static_cast<unsigned long>(NetWinPlayerId));

        if (NetWinSessionData.mode_ == 3) {
            GameState |= 0x10u;
            if (NetDaemonNet.send_files() != 0) {
                pop_close();
                result = 1;
                goto do_cleanup;
            }
            if (NetDaemonNet.lock_state_[0].leading_words_[1] != 0)
                scenario_setup();
        } else {
            GameState &= ~0x10u;
        }

        log_reset();
        if (NetDaemonNet.lock_state_[0].leading_words_[1] == 0) {
            for (int i = 0; i != 11; ++i)
                AltNatural[i] = AltContour[i];
            while (static_cast<int32_t>(
                       NetDaemonNet.lock_state_[0].trailing_words_[2]) < 8 &&
                   ExitTurnLoop == 0) {
                NetDaemonNet.receive();
                do_all_tasks();
            }
        } else {
            uint8_t mask = 0;
            if (NetDaemonNet.player_count_ > 0) {
                for (int i = 0; i != NetDaemonNet.player_count_; ++i) {
                    const unsigned char shift = static_cast<unsigned char>(
                        NetDaemonNet.process_slots_[i + 1].identity_);
                    mask = static_cast<uint8_t>(mask | (1u << (shift & 31)));
                }
            }
            LockEnableMask = (LockEnableMask & 0xFFFFFF00u) | mask;
            NetDaemonNet.send_rules();
            NetDaemonNet.send_game();
        }

        if (NetWinSessionData.mode_ == 3) {
            TutWinDesWindow.close();
            TutWinBaseWindow.close();
            mapwin_system_shutdown();
            ConsoleStatusWin.close();
            WorldClimateWorldWin.close();
            MultiWindow.close();
            MainInterfaceGlobal.hide();
            stop_timers();
            labels_shutdown();
            StringTable.shutdown();
            if (game_init(1, 0) == 0) {
                desktop_init(1);
                WorldClimateSkipTerrainClear = 0;
            }
        }

        NetDaemonNet.locks_.clear();
        if (ControlTurnPhase == 0) planetfall(ConsoleGlobal.faction_id_);

        if (NetWinSessionData.mode_ == 3) {
            int faction = ConsoleGlobal.faction_id_;
            GenderDefault = Players[faction].is_leader_female;
            PluralityDefault = 0;
            parse_says(1, Players[faction].title_leader, -1, -1);
            faction = ConsoleGlobal.faction_id_;
            GenderDefault = Players[faction].is_leader_female;
            PluralityDefault = 0;
            parse_says(2, Players[faction].name_leader, -1, -1);
            faction = ConsoleGlobal.faction_id_;
            GenderDefault = Players[faction].noun_gender;
            PluralityDefault = Players[faction].is_noun_plural;
            parse_says(3, Players[faction].noun_faction, -1, -1);
            StringTemp[0] = '\0';
            strcat(StringTemp, label_get(0));
            strcat(StringTemp, " ");
            say_year(StringTemp);
            parse_says(4, StringTemp, -1, -1);
            parse_num(0, ObjectiveReqVictory);
            parse_num(1, static_cast<int>(ObjectivesSuddenDeathVictory));
            parse_num(2, num_objectives(ConsoleGlobal.faction_id_, GameRules & 0x1000u));
            parse_num(3, static_cast<int>(EndingMissionYear));
            x_pop_caption_label(const_cast<char *>("INTRO"), "SCENARIO", 0);
        }

        if (ExitTurnLoop == 0) return 0;
        result = ExitTurnLoop;
    } else {
        result = 1;
        multiplayer_reset_network_state();
    }

do_cleanup:
    log_set_state(0);
    multiplayer_reset_network_state();
    IsMultiplayerNet = 0;
    return result;
}


// ORIGINAL: 0x00589D30 ?config_game@@YAHH@Z 0x00589D30-0x0058C29D;0x0066028A-0x006605A5
// RECOVERY_BINDING: v1 {"evidence": ["task:newgame-menu-owner", "historical seed:src/unrecovered/00589d30.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:complete mapped native control flow", "source:existing SetupWin/Popup/Buffer/NetDaemon owner declarations; provisional bounded945820 and9B2074 objects, not raw image aliases"], "reason": "Source-owned new-game/menu family preserving complete original identity, auxiliary spans and historical seed findings; existing genuine object lifetimes and typed member owners. Unfinished bodies remain explicit, not implementation coverage."}
// TRIED: mismatch #0 - only the entry (a1&1 defaults shortcut) and the first two do_menu() results (0=default_rules toggle, 1=difficulty rules-flag encode) are transcribed from the 2618-instruction original; the disassembly shows several more do_menu() results driving a PickWin faction picker, Wave/Sound playback, EditGroup name entry and a time_controls_dialog that are NOT modelled - this body loops back to do_menu() for any other result rather than fabricating their bit-flag encodings.
int __cdecl config_game(int) {
    // BODY GOES HERE.
    return 0;
}

// ORIGINAL: 0x005AE120 ?crash_landing@@YAXH@Z 0x005AE120-0x005AED43;0x00662450-0x00662705 FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:cd1b6385ec054fee65debf65d8bcda2c36f7176c:src/unrecovered/005ae120.cpp", "pinned:5AE120..5AED43 and662450..662705;5AE594 fungusx0;5AE58E/5AE5D0 signedcenters;5AE74A..74F fungus smoke gate", "source:src/game.cpp:control_turn;src/popup.h;src/flic.h;src/buffer.cpp", "source:recovery/tests/test_crash_landing_actual_source.py"], "reason": "Recover the complete first-landing operation through real Popup, Buffer, Flic, MapWin, Wave, Player, Base and vehicle owners with automatic normal/exceptional lifetimes. The historical second local widget aggregate is interpreted as Popup-owned embedded components, not duplicated manual vtable storage. Preserve first-owned-base/vehicle selection, early no-match teardown, single-player FUNGBOY shortcut, native fungus start at zero, signed viewport halves, previous-footprint restoration and normal-only smoke independent of pod draw return. The smoke table is existing ExplosionColorTables row12 at6FDB7C; the possible lookup beyond its4096-byte row remains unresolved original asset/read-range evidence, not permission to introduce detached storage or a bounds fix. Both primary and cleanup spans and the seed historical transcription limitations remain obligations; no byte or runtime acceptance is granted."}
// TRIED: byte-exactness is out of reach here without more structural knowledge than the scaffold carries. The SEH frame is real (fs:[0] chain, __CxxFrameHandler, ~40-entry unwind thunk table) and Popup/BasePop already has its own giant field layout (FlatButton x2, Sprite, a 0xC94 dialogs_ blob) that the emitter resolved from other callers, so declaring `Popup popup;` and letting real RAII drive its construction/close() is right and reproduces the two search loops -> short-cleanup vs big-body split faithfully. What is NOT resolvable from this scaffold alone: a SECOND, function-local widget aggregate (GraphicWin+Dialog+ListBox+EditGroup+SpriteBox+CheckBox+Sprite+ 2x FlatButton+2x StringStruct+Heap) built with fully-inlined (vtable-write-only, no visible constructor call) construction and torn down through this-pointer adjustor arithmetic that reads a vtable-relative offset (`aiStackY_4900[1]+4/+8`) - true multiple/virtual-inheritance thunk codegen, not reachable by declaring ordinary separate locals. Modelled those as separate top-level objects in the same relative call order instead, which preserves the call graph but not the stack layout or the thunk bytes. Also approximated: the per-frame x87 pixel-interpolation math in the 30-frame drop loop (kept the draw/decode/timing calls exact, simplified the float position derivation) and the two indirect-IAT-style calls to StringStruct::remove_all/sub_406a90/sub_406820, which were dropped rather than guessing a `this`.
void __cdecl crash_landing(int faction_id) {
    Popup popup;
    Buffer captured_screen;
    Flic pod_animation;
    Flic smoke_animation;
    Buffer pod_frame;
    Buffer smoke_frame;

    int base_id = 0;
    int base_found = 0;
    const int base_count = BaseCurrentCount;
    for (; base_id < base_count; ++base_id) {
        if (Bases[base_id].faction_id_current == faction_id) {
            base_found = 1;
            break;
        }
    }

    int tile_x = 0;
    int tile_y = 0;
    int found_base = 0;
    if (base_found && base_id < base_count) {
        tile_x = Bases[base_id].x;
        tile_y = Bases[base_id].y;
        found_base = 1;
    } else {
        int vehicle_id = 0;
        const int vehicle_count = VehCurrentCount;
        for (; vehicle_id < vehicle_count; ++vehicle_id) {
            if (Vehs[vehicle_id].faction_id == faction_id) break;
        }
        if (vehicle_id < vehicle_count) {
            tile_x = Vehs[vehicle_id].x;
            tile_y = Vehs[vehicle_id].y;
        } else {
            return;
        }
    }

    const int local_faction = ConsoleGlobal.faction_id_;
    StringTemp[0] = '\0';
    const int fungus = (Players[local_faction].rule_flags >> 7) & 1;
    strcat(StringTemp, Players[local_faction].filename);
    CharUpperA(StringTemp);

    if (!IsMultiplayerNet) {
        if (fungus) TurnCurrentNum = 5;
        if (strcmp(StringTemp, "FUNGBOY") == 0) {
            TurnCurrentNum = 5;
            ConsoleGlobal.update_data(0);
            goto flush_and_continue;
        }
    }
    ConsoleGlobal.update_data(0);

    if (GamePreferences & 0x400) {
        g_CRASH_LANDING_WAVE.init(const_cast<char *>("fx\\crash land.wav"), 4);
        g_CRASH_LANDING_WAVE.play();
    }

    pod_frame.init(0x15f, 0x197, 0, 0);
    smoke_frame.init(0x15f, 0x197, 0, 0);
    if (!fungus) {
        pod_animation.init(&pod_frame, const_cast<char *>("drop_pod.flc"), 0, 0, 0);
        smoke_animation.init(&smoke_frame, const_cast<char *>("drop_smoke.flc"), 0, 0, 0);
    } else {
        pod_animation.init(&pod_frame, const_cast<char *>("alien_pod.flc"), 0, 0, 0);
    }

    {
    MapWin &map_window = *MapWinTable[0];
    map_window.field_1DD70_ &= 0xfffff73f;
    map_window.set_center(tile_x, tile_y, 1);
    map_window.draw_map(1);
    do_all_draws();

    GraphicWin &graphic_window = map_window;
    Buffer &screen = graphic_window;
    const int screen_width = screen.pixel_width();
    const int screen_height = screen.pixel_height();
    captured_screen.init(screen_width, screen_height, 0, 0);
    screen.copy(&captured_screen, 0, 0, screen_width, screen_height);

    int tile_pixel_x;
    int tile_pixel_y;
    map_window.tile_to_pixel(tile_x, tile_y, &tile_pixel_x, &tile_pixel_y);

    const float initial_x = static_cast<float>(screen_width - 0x15f);
    const float initial_y = static_cast<float>(screen_height - 0x197);
    const float target_x = static_cast<float>(
        static_cast<int>(map_window.field_1DDB4_) / 2 + tile_pixel_x - 0xa8);
    const float target_y = static_cast<float>(
        static_cast<int>(map_window.field_1DDB8_) / 2 + tile_pixel_y - 0xc0);
    const float x_start = fungus ? 0.0f : initial_x;
    const float x_step = (target_x - x_start) * (1.0f / 30.0f);
    const float y_step = (target_y - initial_y) * (1.0f / 30.0f);
    float animation_x = x_start;
    float animation_y = initial_y + 15.0f;

    RECT previous_rect;
    previous_rect.left = static_cast<int>(animation_x);
    previous_rect.top = static_cast<int>(animation_y);
    previous_rect.right = previous_rect.left + 0x15f;
    previous_rect.bottom = previous_rect.top + 0x197;
    DWORD last_tick = timeGetTime();

    for (int frame = 0; frame < 0x1e; ++frame) {
        captured_screen.copy(&screen, &previous_rect);

        const int half_frame = frame / 2;
        if (fungus) {
            animation_x = animation_x + x_step - static_cast<float>(half_frame) + 7.0f;
        } else {
            animation_x = animation_x + static_cast<float>(half_frame) + x_step - 7.0f;
        }
        animation_y = animation_y + static_cast<float>(frame) + y_step - 15.0f;

        pod_animation.decode_frame(0, 0);
        smoke_animation.decode_frame(0, 0);

        RECT frame_rect;
        frame_rect.left = static_cast<int>(animation_x);
        frame_rect.top = static_cast<int>(animation_y);
        frame_rect.right = frame_rect.left + 0x15f;
        frame_rect.bottom = frame_rect.top + 0x197;

        pod_frame.draw(&screen, 9, frame_rect.left, frame_rect.top, 1, 1);
        if (!fungus) {
            smoke_frame.draw_multi_table_dest(
                &screen, 0xff, frame_rect.left, frame_rect.top, 1, 1,
                &ExplosionColorTables[12][0][0]);
        }

        RECT damage_rect;
        UnionRect(&damage_rect, &frame_rect, &previous_rect);
        graphic_window.update(&damage_rect, 0);
        previous_rect = frame_rect;

        const short shift_held = GetAsyncKeyState(VK_SHIFT);
        DWORD tick;
        for (;;) {
            tick = timeGetTime();
            const DWORD frame_delay = (shift_held & 0xff00) ? 1 : 0x47;
            if (tick - last_tick >= frame_delay) break;
            if (ConsoleNoMapOverrideLatch != 0) break;
        }
        last_tick = tick;
    }
    }

flush_and_continue:
    flush_input();
    SpriteScaleNumerator = 1;
    SpriteScaleDenominator = 1;
    if (found_base) {
        char base_name[256];
        strcpy(base_name, Bases[base_id].name_string);
        popup.start(PopupStartCaption, "FIRSTBASE", 0x17, base_name, 4, 0);
        popup.exec(0, 0);
        strcpy(Bases[base_id].name_string, ParseStrBuffer[0].str);
        make_base_unique(base_id);
    }

    MapWin &map_window_after_animation = *MapWinTable[0];
    map_window_after_animation.field_1DD70_ |= 0x8c0;
    map_window_after_animation.draw_map(1);
    flush_input();
    captured_screen.close();
    if (GamePreferences & 0x400) g_CRASH_LANDING_WAVE.fade(1000);
}
