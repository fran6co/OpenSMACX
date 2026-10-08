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
#include "win.h"
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
#include "pickwin.h"
#include "font.h"
#include "basepop.h"
#include "graphicwin.h"
#include "palette.h"
#include "wave.h"
#include "console.h"
#include "designwin.h"
#include "tutwin.h"
#include "worldwin.h"
#include "multiwin.h"
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
    UnkBitfield1 |= 8;
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
    if (mode == 0 && (GameDataStateStorage.fields_.game_preferences_ & 0x800) != 0) {
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
                GameDataStateStorage.fields_.vehicle_current_count_ = 0;
                setup_game(edi);
                ControlTurnPhase = ConsoleGlobal.faction_id_;
                GameDataStateStorage.fields_.game_state_ |= 0x080000E0u;
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
    GameDataStateStorage.fields_.vehicle_current_count_ = 0;
    setup_game(edi);
    ControlTurnPhase = ConsoleGlobal.faction_id_;
    GameDataStateStorage.fields_.game_state_ |= 0x080000E0u;
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
        GameDataStateStorage.fields_.game_state_ |= 0x080000E0u;
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
        parse_num(0, ScenarioSave.objective_req_victory);
        parse_num(1, static_cast<int>(ScenarioSave.objectives_sudden_death_victory));
        parse_num(2, num_objectives(ConsoleGlobal.faction_id_, GameDataStateStorage.fields_.game_rules_ & 0x1000));
        parse_num(3, static_cast<int>(ScenarioSave.ending_mission_year));
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
    if ((GameDataStateStorage.fields_.game_preferences_ & 0x800) == 0) goto top_menu_audio_check;
    goto top_menu_fade_music;

top_menu_normal_return:
    if (mode == 0 && (GameDataStateStorage.fields_.game_preferences_ & 0x800) != 0)
        goto top_menu_fade_music;
top_menu_audio_check:
    if (g_TOP_MENU_WAVE.is_playing() != 0) goto top_menu_fade_music;
    goto top_menu_cleanup;

top_menu_fade_music:
    g_TOP_MENU_WAVE.fade(0xBB8);
    g_TOP_MENU_WAVE.unload();
top_menu_cleanup:
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
        GameDataStateStorage.fields_.game_rules_ = rules_a;
        const uint32_t old_game_state = GameDataStateStorage.fields_.game_state_;
        GameDataStateStorage.fields_.diff_level_current_ = difficulty;
        const uint8_t rules_c = NetWinSessionOptions.rule_bits_c_;
        const uint32_t merged_game_state = old_game_state | (rules_b & 0x01800000u);
        const bool enable_turn_flag = (rules_c & 0x10u) != 0;
        const uint32_t old_turn_flags = GameDataStateStorage.fields_.net_turn_flags_;
        GameDataStateStorage.fields_.game_state_ = merged_game_state;
        GameDataStateStorage.fields_.net_turn_flags_ = enable_turn_flag ? (old_turn_flags | 0x10u)
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
            GameDataStateStorage.fields_.lock_masks_.enable_mask_ = (GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 0xFFFFFF00u) | mask;

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
                GameDataStateStorage.fields_.vehicle_current_count_ = 0;
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

        GameDataStateStorage.fields_.game_state_ |= NetWinSessionOptions.rule_bits_b_ & 0x01800000u;
        if ((NetWinSessionOptions.rule_bits_c_ & 0x10u) != 0)
            GameDataStateStorage.fields_.net_turn_flags_ |= 0x10u;
        else
            GameDataStateStorage.fields_.net_turn_flags_ &= ~0x10u;
        GameDataStateStorage.fields_.time_controls_selection_ = static_cast<uint8_t>(
            NetWinSessionOptions.time_control_);
        ConsoleGlobal.faction_id_ =
            NetDaemonNet.pid_2_who(static_cast<unsigned long>(NetWinPlayerId));

        if (NetWinSessionData.mode_ == 3) {
            GameDataStateStorage.fields_.game_state_ |= 0x10u;
            if (NetDaemonNet.send_files() != 0) {
                pop_close();
                result = 1;
                goto do_cleanup;
            }
            if (NetDaemonNet.lock_state_[0].leading_words_[1] != 0)
                scenario_setup();
        } else {
            GameDataStateStorage.fields_.game_state_ &= ~0x10u;
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
            GameDataStateStorage.fields_.lock_masks_.enable_mask_ = (GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 0xFFFFFF00u) | mask;
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
            parse_num(0, ScenarioSave.objective_req_victory);
            parse_num(1, static_cast<int>(ScenarioSave.objectives_sudden_death_victory));
            parse_num(2, num_objectives(ConsoleGlobal.faction_id_, GameDataStateStorage.fields_.game_rules_ & 0x1000u));
            parse_num(3, static_cast<int>(ScenarioSave.ending_mission_year));
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


// Newgame-owned shared faction preview and callback state.
// Generated initializer and destructor bindings below identify this global's startup lifetime.
// RECOVERY_OBJECT: v2 0x009403E0 0x0094575C ?ConfigFactionPreview@@3VPopup@@A 0x00589870
// ORIGINAL: 0x00589870 sub_589870 0x00589870-0x00589886 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned terranx_original.exe 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf: original initializer 0x00589870-0x00589886 constructs the Popup stored at 0x009403E0 and registers original teardown 0x00589890.", "Source owner: newgame.cpp declares and uses the genuine Popup ConfigFactionPreview at 0x009403E0; the global, not a fixed-address reinterpretation, owns this lifetime.", "The emitted initializer _$E21 references ConfigFactionPreview and Popup's constructor, then registers _$E19 with atexit; _$E19 references the same global and Popup destructor.", "The archived seed's prior general.cpp placement and compiler-spelling trials are historical; the complete original mapping and claim remain live here for restoration measurement."], "reason": "Bind the original Preview startup constructor to the real global Popup in newgame.cpp and the compiler-emitted constructor/atexit lifetime pair. The prior fixed-address experiment is superseded by this typed owner."}
// symbol    _$E21
// TRIED: homed into general.cpp (its real TU) 2026-09-03. Two source forms were
// measured there and both are archived here verbatim:
//   * as written (placement new + _atexit) it does not compile in the TU -
//     libc.lib has no _atexit, and the placement new buys VC6's null guard;
//   * retyped to a qualified constructor call plus plain atexit it compiles and
//     measures 5 of 6 instructions, the one divergence being VC6 cleaning the
//     atexit push with 'add esp,4' where the image has 'pop ecx'. general.cpp
//     compiles under /Oy- and every cleanup spelling tried kept 'add esp,4';
//     'pop ecx' is the /Oy form. Reverted so the BYTE_EXACT claim stays live
//     here rather than reading REGRESSED there. The qualified-ctor + atexit
//     spelling is the right one; it needs a TU on the /Oy flag set.
Popup ConfigFactionPreview;
// ORIGINAL: 0x00589890 sub_589890 0x00589890-0x0058989A FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned terranx_original.exe 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf: original callback 0x00589890-0x0058989A destroys the Popup at 0x009403E0 registered by 0x00589870.", "Source owner: ConfigFactionPreview is the genuine Popup global declared and consumed by newgame.cpp; its lifetime is not independently owned by general.cpp.", "The emitted helper _$E19 references ConfigFactionPreview and Popup's destructor and is the callback registered by the same translation unit's _$E21 global initializer.", "The original identity, complete span and historical claim are preserved on the source-owned FILE mapping; the retired wrapper is not treated as independent recovered behavior."], "reason": "Bind the original ConfigFactionPreview teardown identity to the compiler-generated destructor callback of newgame.cpp's real Popup global, replacing the fixed-address wrapper in general.cpp."}
// symbol    _$E19
// RECOVERY_OBJECT: v2 0x00945818 0x0094581C ?ConfigFactionDetailMode@@3HA 0x005898A0
int ConfigFactionDetailMode;
// The image uses this zero-filled byte string only as initial prompt text.
// The adjacent PronounBuffer is independently owned; no overlay is introduced.
// RECOVERY_OBJECT: v1 0x00945834 ?ConfigRandomLeaderInitial@@3PADA
char ConfigRandomLeaderInitial[4];

// ORIGINAL: 0x005898A0 ?thumb_routine@@YAXH@Z 0x005898A0-0x00589A24
// RECOVERY_BINDING: v1 {"evidence": ["historical seed:src/recovered/units/005898a0.cpp", "pinned:5898A0..589A24; preview9403E0; detail945818; diplomacy art78EE48 with65C faction stride", "source:Popup537C and PickWin redraw/ListBox cdecl(int) callback contract; allocation/lifetime selection still requires native qualification"], "reason": "Typed shared faction preview callback with real Popup/SpriteBox/FactionArt owners and genuine global storage; no executable-address aliases or manual vtables."}
void __cdecl thumb_routine(int faction) {
    if (ExpansionEnabled) {
        PickWinGlobal.GraphicWin::redraw();
        if (strcmp(Players[faction].filename, "JENN282") == 0) {
            ConfigFactionPreview.close();
            return;
        }
    }
    StringTemp[0] = 0;
    strcat(StringTemp, ConfigFactionDetailMode ? "DATALINKS1" : "BLURB");
    ConfigFactionPreview.start(Players[faction].filename, StringTemp, -1, 0, 0x10500, 0);
    if (ConfigFactionDetailMode) {
        StringTemp[0] = 0;
        strcat(StringTemp, "DATALINKS2");
        ConfigFactionPreview.start(Players[faction].filename, StringTemp, -1, 0, 0x80, 0);
    }
    ConfigFactionPreview.set_caller_picture(ConfigFactionPreview.picture_enabled()
        ? 0 : &FactionsArt[faction].diplomacy);
    ConfigFactionPreview.dialogs().SpriteBox::sprite(&FactionsArt[faction].diplomacy, 0, 0);
    ConfigFactionPreview.Win::set_caption(Players[faction].formal_name_faction);
    ConfigFactionPreview.set_string_font(PopupMenuFont, PopupMenuBoldFont, PopupMenuItalicFont, 0);
    PopupModalActive = 1;
    ConfigFactionPreview.set_loc(5, 0x136);
    ConfigFactionPreview.BasePop::exec(0, 0);
    PopupModalActive = 0;
}

// These masks are derived from the original low-byte and CH tests, retaining
// the asymmetric default/reset path and inverse 0x100 checkbox.
static void config_game_seed_checks() {
    static const unsigned int rules[16] = {
        0x800, 2, 8, 4, 0x1000, 1, 0x10, 0x20,
        0x4000, 0x200, 0x40, 0x100, 0x2000, 0x8000, 0x80, 0x400
    };
    unsigned int checks = 0;
    for (int i = 0; i < 16; ++i) {
        bool checked = (GameDataStateStorage.fields_.game_rules_ & rules[i]) != 0;
        if (i == 11) checked = !checked;
        if (checked) checks |= 1u << i;
    }
    if (GameDataStateStorage.fields_.game_state_ & 0x00800000u) checks |= 0x10000u;
    if (GameDataStateStorage.fields_.game_state_ & 0x01000000u) checks |= 0x20000u;
    PopupWriteCheckState = checks;
}

static void config_game_apply_checks() {
    static const unsigned int rules[16] = {
        0x800, 2, 8, 4, 0x1000, 1, 0x10, 0x20,
        0x4000, 0x200, 0x40, 0x100, 0x2000, 0x8000, 0x80, 0x400
    };
    unsigned int checks = PopupWriteCheckState;
    for (int i = 0; i < 16; ++i) {
        bool checked = (checks & (1u << i)) != 0;
        if (i == 11) checked = !checked;
        if (checked) GameDataStateStorage.fields_.game_rules_ |= rules[i];
    }
    if (checks & 0x10000u) GameDataStateStorage.fields_.game_state_ |= 0x00800000u;
    if (checks & 0x20000u) GameDataStateStorage.fields_.game_state_ |= 0x01000000u;
}

static void config_game_play_voice(int faction, bool flagged) {
    StringTemp[0] = 0;
    strcat(StringTemp, "voices\\");
    strcat(StringTemp, Players[faction].filename);
    strcat(StringTemp, ".mp3");
    g_WAVE_GENERAL.unload();
    if (flagged) g_WAVE_GENERAL.load(StringTemp, 0x100UL);
    else g_WAVE_GENERAL.load(StringTemp);
    g_WAVE_GENERAL.play();
}

static unsigned char config_game_original_rule_byte(int selected) {
    // The pinned expression uses unchecked native Player stride. Its one
    // observed adjacent-storage case is represented through its real owner.
    if (selected == MaxPlayerNum) return static_cast<unsigned char>(MapLandmark[23].name[4]);
    return static_cast<unsigned char>(Players[selected].rule_flags);
}

// ORIGINAL: 0x00589D30 ?config_game@@YAHH@Z 0x00589D30-0x0058C29D;0x0066028A-0x006605A5
// RECOVERY_BINDING: v1 {"evidence": ["historical seed:src/unrecovered/00589d30.cpp; existing original identity and useful lessons preserved", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:589D30..58C29D and66028A..6605A5", "pinned:5898A0..589A24 typed cdecl(int) preview callback; ListBox dispatch retains current declaration", "source:BasePop +A44 value setter,+3100 input poll state; Popup picture/dialog/font and PickWin owned ListBox", "pinned:58A9BA passes zero-filled945834 initial string, allocation extent remains unknown", "source:recovery/tests/test_config_game_actual_source.py actual installed body, bounded API fixture and six compile-success runtime damage controls", "source:589870 initializer and589890 teardown identities now bind to the generated ConfigFactionPreview lifetime in this owner"], "reason": "Complete new-game rules, faction selection, custom naming, PBEM/network and world/setup flow with genuine Popup/SetupWin automatic local lifetimes and a source-owned shared preview. Original undefined and unchecked paths and historical main/cleanup spans remain restoration obligations; no byte or runtime acceptance is granted."}
// TRIED: mismatch #0 - only the entry (a1&1 defaults shortcut) and the first two do_menu() results (0=default_rules toggle, 1=difficulty rules-flag encode) are transcribed from the 2618-instruction original; the disassembly shows several more do_menu() results driving a PickWin faction picker, Wave/Sound playback, EditGroup name entry and a time_controls_dialog that are NOT modelled - this body loops back to do_menu() for any other result rather than fabricating their bit-flag encodings.
// Full config_game candidate. Original identities/claims remain on the owning
// TU; this ignored include is a body proposal, not a measurement or annotation.
int __cdecl config_game(int mode) {
    Popup popup;
    SetupWin setup;
    int selected;                 // native EBP-14, not the parser cursor
    int human_mask;               // intentionally not initialized on shortcut
    int picked;
    int use_expansion_picker;
    int special_a, special_b;
    int custom_count;
    int special_count;
    char first_special[0x18];
    char gender_text[0x50];

    if (mode & 1) {
        GameDataStateStorage.fields_.diff_level_current_ = DefaultPrefs.difficulty;
        GameDataStateStorage.fields_.game_rules_ = AlphaIniPrefs.rules;
        selected = DefaultPrefs.faction_id;
        goto parse_factions;
    }

difficulty_menu:
    setup.Win::hide();
    setup.Win::show(0);
    if ((GameDataStateStorage.fields_.game_rules_ & 0x01000000u) == 0) {
        popup.start(PopupStartCaption, "DIFFICULTY", -1, 0, 0x40, 0);
        for (int i = 0; i < 6; ++i) {
            StringTemp[0] = 0;
            strcat(StringTemp, StringTable.get(static_cast<int>(reinterpret_cast<size_t>(Difficulty[i]))));
            if (i == 0 || i == 5) {
                strcat(StringTemp, " ");
                strcat(StringTemp, "(");
                strcat(StringTemp, label_get((i == 0 ? 0x1C4 : 0x1C8) / 4));
                strcat(StringTemp, ")");
            }
            popup.dialogs().item(StringTemp, i);
        }
        popup.set_value(DefaultPrefs.difficulty);
        picked = setup.do_menu(&popup, 1, 0);
        if (picked < 0) return 1;
        DefaultPrefs.difficulty = picked;
        GameDataStateStorage.fields_.diff_level_current_ = picked;
    }

rules_menu:
    human_mask = 0;
    if (mode == 2) goto select_faction;
    setup.Win::hide();
    setup.Win::show(0);
    popup.start(PopupStartCaption, "USERULES", -1, 0, 0x40, 0);
    popup.set_value(AlphaIniPrefs.customize);
    picked = setup.do_menu(&popup, 1, 0);
    if (picked < 0) {
        if ((GameDataStateStorage.fields_.game_rules_ & 0x01000000u) == 0) goto difficulty_menu;
        return 1;
    }
    AlphaIniPrefs.customize = picked;
    if (picked == 0) {
        GameDataStateStorage.fields_.game_rules_ &= 0xFFFF0000u;
        GameDataStateStorage.fields_.game_rules_ |= default_rules();
        goto select_faction;
    }
    if (picked == 1) {
        GameDataStateStorage.fields_.game_rules_ &= 0xFFFF0000u;
        GameDataStateStorage.fields_.game_rules_ |= GameDataStateStorage.fields_.diff_level_current_ != 0 ? AlphaIniPrefs.rules & 0xFFFFu
            : (AlphaIniPrefs.rules & 0xFDFFu) | 0x200u;
        GameDataStateStorage.fields_.game_state_ = (GameDataStateStorage.fields_.game_state_ & 0xFE7FFFFFu) | AlphaIniPrefs.semaphore;
        goto select_faction;
    }
    human_mask = 0;
custom_rules:
    setup.Win::hide();
    setup.Win::show(0);
    GameDataStateStorage.fields_.game_rules_ = (GameDataStateStorage.fields_.game_rules_ & 0xFFFF0000u) |
        (GameDataStateStorage.fields_.diff_level_current_ != 0 ? AlphaIniPrefs.rules & 0xFFFFu
            : (AlphaIniPrefs.rules & 0xFDFFu) | 0x200u);
    GameDataStateStorage.fields_.game_state_ = (GameDataStateStorage.fields_.game_state_ & 0xFE7FFFFFu) | AlphaIniPrefs.semaphore;
    if (IsMultiplayerNet) GameDataStateStorage.fields_.game_rules_ |= 0x200u;
    config_game_seed_checks();
    if (x_pop_short("RULES", 0x41, 0) < 0) goto rules_menu;
    GameDataStateStorage.fields_.game_rules_ &= 0xFFFF0000u;
    GameDataStateStorage.fields_.game_state_ &= 0xFE7FFFFFu;
    if (ConsoleGroupAction) {
        GameDataStateStorage.fields_.game_rules_ |= default_rules();
        AlphaIniPrefs.rules = GameDataStateStorage.fields_.game_rules_;
        AlphaIniPrefs.semaphore = GameDataStateStorage.fields_.game_state_ & 0x01800000u;
        goto custom_rules;
    }
    config_game_apply_checks();
    AlphaIniPrefs.rules = GameDataStateStorage.fields_.game_rules_;
    AlphaIniPrefs.semaphore = GameDataStateStorage.fields_.game_state_ & 0x01800000u;
    ConfigFactionDetailMode = 0;

select_faction:
    if ((GameDataStateStorage.fields_.game_rules_ & 0x00100000u) && !IsMultiplayerPBEM) {
        selected = ConsoleGlobal.faction_id_;
        goto store_single_faction;
    }
    use_expansion_picker = ExpansionEnabled && !(mode & 2);
    if (use_expansion_picker) {
        PickWinGlobal.init(thumb_routine, mode, ConfigFactionDetailMode);
        for (int i = 1; i < 8; ++i) {
            if (mode == 2 && !(factions_status_byte(1) & (1u << i))) continue;
            StringTemp[0] = 0;
            if (strcmp(Players[i].filename, "JENN282") != 0) {
                strcat(StringTemp, Players[i].formal_name_faction);
                strcat(StringTemp, "|");
                strcat(StringTemp, "(");
                strcat(StringTemp, Players[i].desc_name_faction);
                strcat(StringTemp, ")");
            } else strcat(StringTemp, "RANDOM");
            PickWinGlobal.listBox_.item(StringTemp, i);
        }
        PickWinGlobal.listBox_.attach(&PickWinGlobal,
            PickWinGlobal.listBox_.Dialog::field_24_,
            PickWinGlobal.listBox_.Dialog::field_28_, 0x80020);
        picked = DefaultPrefs.faction_id;
        if (picked < 1) picked = 1;
        else if (picked > 7) picked = 7;
        DefaultPrefs.faction_id = picked;
        PickWinGlobal.listBox_.set_selected_id(picked);
        PickWinGlobal.field_5DB4_ = PickWinGlobal.listBox_.Dialog::get_selected_id();
        PickWinGlobal.listBox_.set_selection_callback(thumb_routine);
        thumb_routine(DefaultPrefs.faction_id);
        picked = PickWinGlobal.go_girl();
        if (picked < 0) {
            if (bit_count(human_mask) > 1 && selected > 0 &&
                (human_mask & (1u << (selected & 31)))) {
                human_mask &= ~(1u << (selected & 31));
                if (human_mask > 1) {
                    int first = 1;
                    for (; first < human_mask; ++first) {
                        if (human_mask & (1u << (first & 31))) { selected = first; break; }
                    }
                }
                PickWinGlobal.GraphicWin::close();
                goto pbem_selection;
            }
            ConfigFactionPreview.close();
            PickWinGlobal.GraphicWin::close();
            do_all_draws();
            goto difficulty_menu;
        }
        DefaultPrefs.faction_id = picked;
        if (IsMultiplayerPBEM && (human_mask & (1u << (picked & 31)))) {
            X_pop("FACTIONTAKEN", 0);
            goto select_faction;
        }
        selected = picked;
        if (PickWinGlobal.field_5DA4_ == 0x105) {
            picked = rand() % 7 + 1;
            selected = picked;
            ConfigFactionPreview.close();
            PickWinGlobal.GraphicWin::close();
            goto expansion_name;
        }
        if (PickWinGlobal.field_5DA4_ == 0x107) {
            ConfigFactionDetailMode = ConfigFactionDetailMode == 0;
            goto select_faction;
        }
        if (GameDataStateStorage.fields_.game_more_preferences_ & 0x4000u) config_game_play_voice(selected, false);
        if (PickWinGlobal.field_5DA4_ != 0) {
            if (strcmp(Players[picked].filename, "JENN282") == 0) goto select_faction;
            if (PickWinGlobal.field_5DA4_ != 0) goto edit_names;
        }

expansion_name:
        if (strcmp(Players[picked].filename, "JENN282") == 0) {
            ConfigFactionPreview.close();
            PickWinGlobal.GraphicWin::close();
            goto parse_factions;
        }
expansion_prompt:
        StringTemp[0] = 0;
        strcat(StringTemp, "NAME");
        if (Players[picked].is_leader_female && PickWinGlobal.field_5DA4_ != 0x105)
            strcat(StringTemp, "2");
        if (PickWinGlobal.field_5DA4_ == 0x105) strcat(StringTemp, "3");
        {
            char *initial = PickWinGlobal.field_5DA4_ == 0x105
                ? ConfigRandomLeaderInitial : Players[picked].name_leader;
            int answer = X_pop_ask(StringTemp, 0x17, initial, 0, 0);
            ConfigFactionPreview.close();
            if (GameDataStateStorage.fields_.game_more_preferences_ & 0x4000u) g_WAVE_GENERAL.Sound::fade(0x7D0);
            if (answer != 0) goto select_faction;
        }
        PickWinGlobal.GraphicWin::close();
        strcpy(Players[picked].name_leader, ParseStrBuffer[0].str);
        if (!ConsoleGroupAction) goto parse_factions;
        if (PickWinGlobal.field_5DA4_ == 0x105) {
            Players[picked].is_leader_female = !Players[picked].is_leader_female;
            if (strcmp(Players[picked].filename, "JENN282") != 0) goto expansion_prompt;
            ConfigFactionPreview.close();
            PickWinGlobal.GraphicWin::close();
            goto parse_factions;
        }
        goto edit_names;
    }

    popup.start(PopupStartCaption, "FACTIONX", -1, 0, 0x40, 0);
    {
        for (int i = 1; i < 8; ++i) {
            if (mode == 2 && !(factions_status_byte(1) & (1u << i))) continue;
            StringTemp[0] = 0;
            strcat(StringTemp, Players[i].formal_name_faction);
            strcat(StringTemp, "|");
            strcat(StringTemp, "(");
            strcat(StringTemp, Players[i].desc_name_faction);
            strcat(StringTemp, ")");
            popup.dialogs().item(StringTemp, i);
        }
    }
    popup.set_value(mode & 2 ? ConsoleGlobal.faction_id_ : DefaultPrefs.faction_id);
    popup.dialogs().ListBox::set_selection_callback(thumb_routine);
    thumb_routine(mode & 2 ? ConsoleGlobal.faction_id_ : DefaultPrefs.faction_id);
    picked = popup.BasePop::exec(0, 0);
    if (picked < 0) {
        if (bit_count(human_mask) > 1 && selected > 0 &&
            (human_mask & (1u << (selected & 31)))) {
            human_mask &= ~(1u << (selected & 31));
            if (human_mask > 1) {
                int first = 1;
                for (; first < human_mask; ++first) {
                    if (human_mask & (1u << (first & 31))) { selected = first; break; }
                }
            }
            goto pbem_selection;
        }
        ConfigFactionPreview.close();
        if (mode == 2) goto difficulty_menu;
        goto rules_menu;
    }
    if (!(mode & 2)) DefaultPrefs.faction_id = picked;
    if (IsMultiplayerPBEM && (human_mask & (1u << (picked & 31)))) {
        X_pop("FACTIONTAKEN", 0);
        goto select_faction;
    }
    selected = picked;
    if (popup.input_poll_state() == 2) {
        ConfigFactionDetailMode = ConfigFactionDetailMode == 0;
        goto select_faction;
    }
    if (popup.input_poll_state() == 3) {
        ConfigFactionPreview.close();
        if (X_pop_ask("RELOADFAC", 0x50, Players[picked].filename, 0, 0)) goto select_faction;
        strcpy(Players[picked].filename, ParseStrBuffer[0].str);
        strcpy(Players[picked].search_key, ParseStrBuffer[0].str);
        if (picked) { read_faction(&Players[picked], 0); load_faction_art(picked); }
        compute_faction_modifiers(picked);
        goto select_faction;
    }
    if (GameDataStateStorage.fields_.game_more_preferences_ & 0x4000u) config_game_play_voice(picked, true);
    if (!popup.input_poll_state()) {
        StringTemp[0] = 0;
        strcat(StringTemp, "NAME");
        if (Players[picked].is_leader_female) strcat(StringTemp, "2");
        int answer = X_pop_ask(StringTemp, 0x17, Players[picked].name_leader, 0, 0);
        ConfigFactionPreview.close();
        if (GameDataStateStorage.fields_.game_more_preferences_ & 0x4000u) g_WAVE_GENERAL.Sound::fade(0x7D0);
        if (answer) goto select_faction;
        strcpy(Players[picked].name_leader, ParseStrBuffer[0].str);
        if (!ConsoleGroupAction) goto parse_factions;
    }

edit_names:
    for (;;) {
        strcpy(gender_text, label_get((Players[picked].is_leader_female ? 0xB8C : 0xB88) / 4));
        popup.start(PopupStartCaption, "CUSTOMNAMES", -1, 0, 0x44, 0);
        popup.dialogs().EditGroup::edit(label_get(0x2B8 / 4), Players[picked].name_leader, 0x17);
        popup.dialogs().EditGroup::edit(label_get(0x2BC / 4), Players[picked].formal_name_faction, 0x27);
        popup.dialogs().EditGroup::edit(label_get(0x2C0 / 4), Players[picked].noun_faction, 0x17);
        popup.dialogs().EditGroup::edit(label_get(0x2C4 / 4), Players[picked].adj_name_faction, 0x17);
        popup.dialogs().EditGroup::edit(label_get(0x78C / 4), Players[picked].title_leader, 0x17);
        popup.dialogs().EditGroup::edit(label_get(0x790 / 4), Players[picked].adj_leader, 0x17);
        popup.dialogs().EditGroup::edit(label_get(0x794 / 4), gender_text, 0x17);
        int answer = popup.BasePop::exec(0, 0);
        if (GameDataStateStorage.fields_.game_more_preferences_ & 0x4000u) g_WAVE_GENERAL.Sound::fade(0x7D0);
        if (!use_expansion_picker) ConfigFactionPreview.close();
        if (answer < 0) {
            if (use_expansion_picker) popup.close();
            goto select_faction;
        }
        if (popup.input_poll_state()) {
            if (picked) { read_faction(&Players[picked], 0); load_faction_art(picked); }
            continue;
        }
        if (use_expansion_picker) {
            ConfigFactionPreview.close();
            PickWinGlobal.GraphicWin::close();
        }
        strcpy(Players[picked].name_leader, ParseStrBuffer[0].str);
        strcpy(Players[picked].formal_name_faction, ParseStrBuffer[1].str);
        strcpy(Players[picked].noun_faction, ParseStrBuffer[2].str);
        strcpy(Players[picked].adj_name_faction, ParseStrBuffer[3].str);
        strcpy(Players[picked].title_leader, ParseStrBuffer[4].str);
        strcpy(Players[picked].adj_leader, ParseStrBuffer[5].str);
        Players[picked].is_leader_female = _strnicmp(gender_text, label_get(0xB8C / 4), 1) == 0;
        if (use_expansion_picker) goto select_faction;
        break;
    }

parse_factions:
    if (mode == 0) prefs_save(1);
    custom_count = 14;
    special_a = 0;
    special_b = 0;
    first_special[0] = 0;
    if (text_open(AlphaxFileID, "CUSTOMFACTIONS") == 0) {
        text_get();
        while (*text_item()) { ++custom_count; text_get(); }
    }
    special_count = 0;
    {
        for (int i = 1; i < 8; ++i) {
            if (strcmp(Players[i].filename, "JENN282") != 0 && (Players[i].rule_flags & 0x80)) {
                ++special_count;
                if (special_count == 1) strcat(first_special, Players[i].filename);
            }
        }
    }
    if (special_count == 1) {
        int last_random = 0;
        for (int i = 1; i < 8; ++i)
            if (strcmp(Players[i].filename, "JENN282") == 0) last_random = i;
        if (last_random) {
            if (text_open(AlphaxFileID, "NEWFACTIONS")) goto select_faction;
            text_get();
            for (int skip = 0; skip < 5; ++skip) text_get();
            strcpy(Players[last_random].filename, text_item());
            if (strcmp(Players[last_random].filename, first_special) == 0) {
                text_get();
                strcpy(Players[last_random].filename, text_item());
            }
            strcpy(Players[last_random].search_key, Players[last_random].filename);
            read_faction(&Players[last_random], 0);
            load_faction_art(last_random);
        }
    } else if (!special_count && rand() % 2 != 0) {
        int start = rand() % 7;
        // Native scan computes modulo seven afresh on each iteration.
        for (int attempt = start; 1 - start + attempt < 8; ++attempt) {
            int i = attempt % 7;
            if (strcmp(Players[i + 1].filename, "JENN282") == 0) {
                if (!special_a) special_a = i + 1;
                else if (!special_b) special_b = i + 1;
            }
        }
        if (!special_a || !special_b) { special_a = 0; special_b = 0; }
    }
    {
        for (int i = 1; i < 8; ++i) {
            if (strcmp(Players[i].filename, "JENN282") != 0) continue;
            if (special_a == i || special_b == i) {
                if (text_open(AlphaxFileID, "NEWFACTIONS")) goto select_faction;
                text_get();
                int skip_count = special_a == i ? 5 : 6;
                for (int skip = 0; skip < skip_count; ++skip) text_get();
                strcpy(Players[i].filename, text_item());
                strcpy(Players[i].search_key, Players[i].filename);
                read_faction(&Players[i], 0);
                load_faction_art(i);
                continue;
            }
            for (;;) {
                int residue;
                do { residue = rand() % custom_count; } while (residue == 12 || residue == 13);
                switch (residue / 7) {
                case 0: if (text_open(AlphaxFileID, "FACTIONS")) goto select_faction; break;
                case 1: if (text_open(AlphaxFileID, "NEWFACTIONS")) goto select_faction; break;
                case 2: if (text_open(AlphaxFileID, "CUSTOMFACTIONS")) goto select_faction; break;
                }
                int row = residue % 7;
                text_get();
                for (int skip = 0; skip < row; ++skip) text_get();
                strcpy(Players[i].filename, text_item());
                strcpy(Players[i].search_key, Players[i].filename);
                bool duplicate = false;
                for (int other = 1; other < 8; ++other) {
                    if (i != other && strcmp(Players[i].filename, Players[other].filename) == 0) {
                        duplicate = true; break;
                    }
                }
                if (duplicate) continue;
                read_faction(&Players[i], 0);
                load_faction_art(i);
                break;
            }
        }
    }
    load_flags();
    if (ExpansionEnabled && !IsMultiplayerNet && !IsMultiplayerPBEM &&
        (config_game_original_rule_byte(selected) & 0x80))
        GameDataStateStorage.fields_.game_rules_ = (GameDataStateStorage.fields_.game_rules_ & ~0x200u) | 0x140u;
    if (mode & 1) goto pbem_commit;
    do_all_draws();

pbem_selection:
    if (!IsMultiplayerPBEM) goto store_single_faction;
    {
        unsigned int selected_bit = 1u << (selected & 31);
        human_mask |= selected_bit;
        popup.start(PopupStartCaption, "HOTSEATADD0", -1, 0, 0x40, 0);
        for (int i = 1; i < 8; ++i) {
            if (!(human_mask & (1u << i))) continue;
            if (X_text_open(const_cast<char *>(ScriptTxtID), "MULTISTRING")) break;
            text_get();
            GenderDefault = Players[i].is_leader_female;
            PluralityDefault = 0;
            parse_says(0, Players[i].title_leader, -1, -1);
            GenderDefault = Players[i].is_leader_female;
            PluralityDefault = 0;
            parse_says(1, Players[i].name_leader, -1, -1);
            GenderDefault = Players[i].noun_gender;
            PluralityDefault = Players[i].is_noun_plural;
            parse_says(2, Players[i].noun_faction, -1, -1);
            StringTemp[0] = 0;
            parse_string(TextBufferGetPtr, StringTemp);
            popup.string(StringTemp);
        }
        popup.start(PopupStartCaption, "HOTSEATADD1", -1, 0, 0x80, 0);
        int answer = popup.BasePop::exec(0, 0);
        if (answer) {
            if (answer < 0) human_mask &= ~selected_bit;
            goto select_faction;
        }
    }
pbem_commit:
    if (!IsMultiplayerPBEM) goto store_single_faction;
    *reinterpret_cast<unsigned char *>(&GameDataStateStorage.fields_.lock_masks_.enable_mask_) = static_cast<unsigned char>(human_mask);
    selected = 1;
    for (; selected < 8; ++selected) if ((human_mask & 0xFF) & (1u << selected)) break;
    if (selected >= 8) selected = 1;
    ConsoleGlobal.faction_id_ = selected;
    goto time_controls;

store_single_faction:
    ConsoleGlobal.faction_id_ = selected;
    *reinterpret_cast<unsigned char *>(&GameDataStateStorage.fields_.lock_masks_.enable_mask_) =
        static_cast<unsigned char>(1u << (selected & 31));
time_controls:
    if (IsMultiplayerNet) {
        time_controls_dialog(&popup);
        popup.set_value(AlphaIniPrefs.time_controls);
        int answer = popup.BasePop::exec(0, 0);
        if (answer < 0) {
            if (!(GameDataStateStorage.fields_.game_rules_ & 0x00100000u)) goto select_faction;
            if (mode != 2) goto rules_menu;
            goto difficulty_menu;
        }
        AlphaIniPrefs.time_controls = answer;
        GameDataStateStorage.fields_.time_controls_selection_ = answer;
    }
    prefs_save(0);
    log_set_state(0);
    if (!(mode & 2)) {
        GameDataStateStorage.fields_.vehicle_current_count_ = 0;
        if (MapSetupStatus) world_build();
        if (!(mode & 2)) setup_game(0);
    }
    if (mode == 2) scenario_setup();
    if (IsMultiplayerNet) log_set_state(1);
    if (ExpansionEnabled) alien_start();
    if (!IsMultiplayerNet) {
        if (!IsMultiplayerPBEM) {
            if (mode & 2) return 0;
            planetfall(ConsoleGlobal.faction_id_);
        }
    }
    if (IsMultiplayerPBEM) {
        clear_tamper();
        unsigned int ticks = GetTickCount();
        TamperState93A9AC = ticks + MapRandSeed;
        SYSTEMTIME date;
        GetSystemTime(&date);
        TamperState93A9B4 = 0;
        TamperState93A9B0 = ((date.wYear << 4) + date.wMonth) * 32 + date.wDay;
    }
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
    const int base_count = GameDataStateStorage.fields_.base_current_count_;
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
        const int vehicle_count = GameDataStateStorage.fields_.vehicle_current_count_;
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
        if (fungus) GameDataStateStorage.fields_.turn_current_num_ = 5;
        if (strcmp(StringTemp, "FUNGBOY") == 0) {
            GameDataStateStorage.fields_.turn_current_num_ = 5;
            ConsoleGlobal.update_data(0);
            goto flush_and_continue;
        }
    }
    ConsoleGlobal.update_data(0);

    if (GameDataStateStorage.fields_.game_preferences_ & 0x400) {
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
    if (GameDataStateStorage.fields_.game_preferences_ & 0x400) g_CRASH_LANDING_WAVE.fade(1000);
}
