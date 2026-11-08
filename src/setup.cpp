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
// The setup translation unit, 0x005AE0F0-0x005B5688 (image-inferred, see the
// 2026-11-07 TU map). It opens with the g_CRASH_LANDING_WAVE init thunks and
// holds new-game and scenario setup, faction setup/elimination and the social
// engineering functions through social_select, in image order.
#include "stdafx.h"
#include <shellapi.h>
#include <ctype.h> // yearmotize preserves the original signed-byte CRT arguments.
#include <stddef.h>
#include <mmsystem.h>
#include "win.h"
#include "xpops.h"  // X_pop
#include "popup.h"  // Existing POP2 owner interface.
#include "ambience.h"  // load_music, the startup faction ambience owner.
#include "game.h"
#include "interface_palette.h"
#include "sound.h"
#include "netmsg.h"
#include "alpha.h"
#include "base.h"
#include "basepop.h"  // BasePopDefaultField322C
#include "buffer.h"  // g_PCX_PARSE_TEMP_BUFFER1
#include "graphicwin.h"  // opening window's Buffer base and virtual update
#include "sprite.h"  // Sprite::extract
#include "texture.h" // Terrain-sheet texture assets.
#include "basewin.h"  // BaseWin::close
#include "console.h"  // ConsoleGlobal, ConsoleStatusWin
#include "designwin.h"  // DesignWin::close
#include "faction.h"
#include "fx.h"  // g_FX
#include "general.h"
#include "filebox.h"
#include "prefwin.h"  // ConsolePrefWin.init needs the complete owner declaration.
#include "lock.h"
#include "map.h"
#include "main.h"  // CommandLineText
#include "mapwin.h" // draw_tile, mapwin_system_shutdown
#include "multiwin.h"
#include "log.h"  // log_set_state
#include "random.h"  // random_reseed
#include "battlewin.h"  // the standing battle window
#include "datalink.h"  // DatalinkMain
#include "messagewin.h"  // the standing MessageWin
#include "multidebug.h"  // MultiDebug, the standing debug window
#include "netdaemon.h"  // NetDaemonNet
#include "halloffame.h"
#include "interlude.h"
#include "quaylewin.h"
#include "reportif.h"
#include "statuswin.h"  // StatusWin::close
#include "strings.h"  // StringTable
#include "text.h"  // text_open/text_get/text_item, TextBufferGetPtr
#include "time.h"  // stop_timers
#include "tutwin.h"  // TutWinDesWindow, TutWinBaseWindow, TutWinIfaceWindow
#include "veh.h"
#include "crt_random_inline.h"  // bounded_crt_random
#include "wave.h"  // g_WAVE_GENERAL
#include "worldwin.h"  // WorldClimateWorldWin
#include "diplowin.h"  // DiploWin::exec, homed here out of the archive
#include "diplopop.h"  // DiplomacyPopup
#include "filemap.h"
#include "palette.h"
#include "caviar.h"
#include "vector.h"  // free three-float operations
#include "newtechwin.h"  // Existing technology-popup sprite assets.
#include "infowin.h"  // Shared faction text/shadow colors.
#include "maininterface.h"  // Shared chat foreground/shadow colors.
#include "pushbutton.h"
#include "flatbutton.h"
#include "council.h"
#include "councwin.h"  // game_data transfers the live council window fields
#include "replaywin.h"
#include "technology.h"
#include "path.h"
#include "alphasave.h"
#include "netwin.h"
#include "deletionlist.h"
#include "worldwin.h"
#include "general.h"
#include "crt_random_inline.h"
#include "time.h"
#include "win.h"
#include "faction.h"
#include "tutwin.h"
#include "alpha.h"
#include "base.h"
#include "council.h"
#include "path.h"
#include "councwin.h"
#include "popup.h"
#include "dialogs.h"
#include "text.h"
#include "game.h"
#include "map.h"
#include "probe.h"
#include "technology.h"
#include "veh.h"
#include "netmsg.h"  // NetMsgPrimary, which the diplomacy bodies pop through
#include "netdaemon.h"  // NetDaemonNet, message_data - the net_* writers
#include "log.h"  // log_say, which the net_* writers announce each move with
#include "console.h"  // ConsoleGlobal, which net_energy refreshes
#include "tutorial_script.h"
#include "socialwin.h"
#include "mapwin.h"
#include "multiwin.h"
#include "planwin.h"
#include "alphamovie.h"
#include "strings.h"
#include "xpops.h"
#include "diplopop.h"
#include "diplowin.h"
#include "infowin.h"
#include "reportif.h"
#include "random.h"
#include "replaywin.h"
#include "datalink.h"
#include "faction_inline.h"  // has_treaty/has_agenda and the shared diplomacy helpers
#include "socialwin.h"
#include "game_data_state_window.h"

extern Wave g_CRASH_LANDING_WAVE;
extern int ConsoleNoMapOverrideLatch;

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
        map_window.tile_pixel_width_ / 2 + tile_pixel_x - 0xa8);
    const float target_y = static_cast<float>(
        map_window.tile_pixel_height_ / 2 + tile_pixel_y - 0xc0);
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
                ExplosionColorTables[12]);
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
                || has_tech(VehPrototypes[proto_id].preq_tech, faction_id))
            && VehPrototypes[proto_id].plan == plan) {
            return proto_id;
        }
    }
    return -1;
}

static int time_warp_find_prototype(int faction_id, int plan) {
    const uint8_t faction_bit = static_cast<uint8_t>(1u << faction_id);
    for (int scan_index = 0; scan_index < 128; ++scan_index) {
        const int prototype_id = scan_index < 64
            ? scan_index : faction_id * 64 + scan_index - 64;
        const VehPrototype &prototype = VehPrototypes[prototype_id];
        if ((prototype.flags & 1) == 0 ||
            (prototype.obsolete_factions & faction_bit) != 0) continue;
        if (prototype_id < 64 && !has_tech(prototype.preq_tech, faction_id)) continue;
        if (prototype.plan == plan) return prototype_id;
    }
    return -1;
}
// ORIGINAL: 0x005AEDE0 ?time_warp@@YAXXZ 0x005AEDE0-0x005B0412
// RECOVERY_BINDING: v1 {"evidence": ["Independent pinned full-span review by report_draw_labs_review; retained game-setup-owner-wave full body and actual-source controls for three profiles, land/aquatic sites, path modes and native relocation."], "reason": "Recover complete accelerated-start setup with typed faction, base, prototype, map and Path owners; preserve callback-sensitive region rereads and native facility/path order. External unfinished callees remain explicit."}
// TRIED: MISMATCH #0 push/sub at prologue; full-body transcription from Ghidra hypothesis, corrected against raw disasm for: has_tech arg counts (Ghidra folded a dangling outer-call push into two call sites), Path::find(8 ints) called through g_00945b00, veh_lift/veh_drop split (veh_lift returns a value the mangled YAX hid; PROPOSAL: veh_lift should be `int veh_lift(int)`), and per-field pointer widths (short/byte) recovered from `movsx word/byte ptr` on every g_0097d0xx/g_009ab8xx/g_0096xxxx access since the scaffold's g_ globals are declared int* uniformly.
void __cdecl time_warp() {
    // One vehicle id local ([ebp-8]) holds most veh_init results and the
    // closing vehicle loop index; the smallest-base morale store reuses it.
    int veh_id;
    const int variation = rand() % 3;
    int scenario_turn;
    int initial_research;
    int research_adjustment;
    int expansion_count;
    int initial_population;
    if (variation == 0) {
        scenario_turn = 30;
        initial_research = 6;
        research_adjustment = 2;
        expansion_count = 3;
        initial_population = 5;
    } else if (variation == 1) {
        scenario_turn = 60;
        initial_research = 12;
        research_adjustment = 3;
        expansion_count = 4;
        initial_population = 6;
    } else {
        scenario_turn = 80;
        initial_research = 16;
        research_adjustment = 4;
        expansion_count = 5;
        initial_population = 6;
    }

    for (int faction_id = 1; faction_id < MaxPlayerNum; ++faction_id) {
        const uint32_t alive_faction = 1u << (faction_id + 8);
        if ((GameDataStateStorage.fields_.lock_masks_.enable_mask_ & alive_faction) == 0) continue;
        const int starting_base_id = base_find(0, 0, faction_id);
        if (starting_base_id < 0) continue;

        Base &starting_base = Bases[starting_base_id];
        Player &player = Players[faction_id];
        PlayerData &player_data = PlayersData[faction_id];
        starting_base.population_size = static_cast<int8_t>(initial_population);
        if (player.rule_talent != 0) ++starting_base.population_size;
        if (player.rule_drone != 0 || player.rule_population != 0)
            --starting_base.population_size;
        starting_base.nutrients_accumulated =
            static_cast<int>(starting_base.population_size) * 10 / 2;

        const int starting_region = map_loc(starting_base.x, starting_base.y)->region;
        const int wealth_sign = player_data.ai_wealth < 1
            ? (player_data.ai_wealth >= 0 ? 0 : -1) : 1;
        const int power_sign = player_data.ai_power < 1
            ? (player_data.ai_power >= 0 ? 0 : -1) : 1;
        const int research_advances =
            (wealth_sign - power_sign) * research_adjustment + initial_research;

        AiResearchBatchActive = 1;
        for (int advance = 0; advance < research_advances; ++advance) {
            player_data.tech_id_researching = tech_ai(faction_id);
            tech_advance(faction_id);
        }
        player_data.tech_accumulated = static_cast<int>(player_data.tech_achieved << 1);
        AiResearchBatchActive = 0;
        player_data.tech_id_researching = -1;
        consider_designs(faction_id);

        set_fac(FAC_RECYCLING_TANKS, starting_base_id, 1);
        set_fac(FAC_RECREATION_COMMONS, starting_base_id, 1);
        if (player_data.ai_wealth != 0)
            set_fac(FAC_ENERGY_BANK, starting_base_id, 1);
        if (player_data.ai_power != 0)
            set_fac(FAC_COMMAND_CENTER, starting_base_id, 1);
        if (player_data.ai_growth != 0)
            set_fac(FAC_NETWORK_NODE, starting_base_id, 1);

        if (player_data.ai_fight != 0) {
            int initial_foil;
            if (ExpansionEnabled == 0 || (player.rule_flags & RFLAG_AQUATIC) == 0) {
                initial_foil = BSC_UNITY_ROVER;
            } else {
                initial_foil = has_tech(VehPrototypes[BSC_UNITY_FOIL].preq_tech,
                                        faction_id)
                    ? BSC_TRANSPORT_FOIL : BSC_UNITY_FOIL;
            }
            veh_init(initial_foil, faction_id, starting_base.x, starting_base.y);
            // Native pushes (facility, 1, starting-base-id) in this branch,
            // unlike the ordinary facility stores immediately above.
            set_fac(FAC_PERIMETER_DEFENSE, 1, starting_base_id);
        }

        // Starting unit selection first searches predefined designs, then
        // the faction's 64-design bank. The scan is ordered, and technology
        // is checked only for the predefined range.
        const int plan_six_prototype = time_warp_find_prototype(faction_id, 6);
        const BOOL has_foil_technology =
            has_tech(VehPrototypes[BSC_UNITY_FOIL].preq_tech, faction_id);
        int opening_prototype = time_warp_find_prototype(faction_id, 2);
        if (opening_prototype < 0)
            opening_prototype = time_warp_find_prototype(faction_id, 1);
        if (opening_prototype < 0) opening_prototype = 2;
        VehPrototypes[opening_prototype].flags |= 4;
        int opening_vehicle = veh_init(opening_prototype, faction_id,
                                       starting_base.x, starting_base.y);
        Vehs[opening_vehicle].order = ORDER_HOLD;
        ++Vehs[opening_vehicle].morale;
        opening_vehicle = veh_init(opening_prototype, faction_id,
                                   starting_base.x, starting_base.y);
        Vehs[opening_vehicle].order = ORDER_HOLD;
        const int third_prototype =
            ExpansionEnabled == 0 || (player.rule_flags & RFLAG_AQUATIC) == 0 ? 1 : 4;
        veh_id = veh_init(third_prototype, faction_id, starting_base.x, starting_base.y);

        const int plan_one_prototype = time_warp_find_prototype(faction_id, 1);
        if (plan_one_prototype >= 0) {
            VehPrototypes[plan_one_prototype].flags |= 4;
            veh_id = veh_init(plan_one_prototype, faction_id,
                              starting_base.x, starting_base.y);
            ++Vehs[veh_id].morale;
            if (plan_one_prototype == BSC_MIND_WORMS) Vehs[veh_id].morale = 2;
        }

        const int plan_zero_prototype = time_warp_find_prototype(faction_id, 0);
        if (plan_zero_prototype >= 0) {
            VehPrototypes[plan_zero_prototype].flags |= 4;
            if (player_data.net_random_event < 1 && player_data.ai_power == 0) {
                // The native branch deliberately skips this plan-0 vehicle,
                // then repeats its plan-1 scan and may add that design instead.
                const int fallback = time_warp_find_prototype(faction_id, 1);
                if (fallback >= 0) {
                    VehPrototypes[fallback].flags |= 4;
                    veh_id = veh_init(fallback, faction_id,
                                      starting_base.x, starting_base.y);
                    if (fallback == BSC_MIND_WORMS) Vehs[veh_id].morale = 2;
                }
            } else {
                veh_id = veh_init(plan_zero_prototype, faction_id,
                                  starting_base.x, starting_base.y);
                ++Vehs[veh_id].morale;
                if (player_data.net_random_event > 0)
                    veh_id = veh_init(plan_zero_prototype, faction_id,
                                      starting_base.x, starting_base.y);
            }
        }

        int starting_project;
        int current_project_base;
        do {
            starting_project = rand() % 8;
            switch (starting_project) {
            case 0: current_project_base = GameDataStateStorage.fields_.secret_project_.human_genome_project(); break;
            case 1: current_project_base = GameDataStateStorage.fields_.secret_project_.command_nexus(); break;
            case 2: current_project_base = GameDataStateStorage.fields_.secret_project_.weather_paradigm(); break;
            case 3: current_project_base = GameDataStateStorage.fields_.secret_project_.merchant_exchange(); break;
            case 5: current_project_base = GameDataStateStorage.fields_.secret_project_.citizens_defense_force(); break;
            case 6: current_project_base = GameDataStateStorage.fields_.secret_project_.virtual_world(); break;
            default: current_project_base = GameDataStateStorage.fields_.secret_project_.planetary_transit_system(); break;
            }
        } while (current_project_base != SP_Unbuilt || starting_project == 4 ||
                 (starting_project == 5 && strcmp(player.filename, "HIVE") == 0));
        switch (starting_project) {
        case 0: GameDataStateStorage.fields_.secret_project_.human_genome_project() = starting_base_id; break;
        case 1: GameDataStateStorage.fields_.secret_project_.command_nexus() = starting_base_id; break;
        case 2: GameDataStateStorage.fields_.secret_project_.weather_paradigm() = starting_base_id; break;
        case 3: GameDataStateStorage.fields_.secret_project_.merchant_exchange() = starting_base_id; break;
        case 5: GameDataStateStorage.fields_.secret_project_.citizens_defense_force() = starting_base_id; break;
        case 6: GameDataStateStorage.fields_.secret_project_.virtual_world() = starting_base_id; break;
        default: GameDataStateStorage.fields_.secret_project_.planetary_transit_system() = starting_base_id; break;
        }

        // The native expansion pass considers expansion_count - 1 rounds.
        // A round chooses the best free site around any faction base, then
        // establishes one new settlement and its opening garrison.
        bool placed_coastal_group = false;
        for (int expansion_round = 0; expansion_round < expansion_count - 1;
             ++expansion_round) {
            int best_score = 0;
            int best_x = 0;
            int best_y = 0;
            int best_base_id = -1;
            int scan_base_id = 0;
            int last_base_x = 0;
            int last_base_y = 0;
            if (GameDataStateStorage.fields_.base_current_count_ > 0) {
                for (; scan_base_id < GameDataStateStorage.fields_.base_current_count_; ++scan_base_id) {
                    Base &search_base = Bases[scan_base_id];
                    if (search_base.faction_id_current != faction_id) continue;
                    const int center_x = search_base.x;
                    const int center_y = search_base.y;
                    last_base_x = center_x;
                    last_base_y = center_y;
                    const int center_region = map_loc(center_x, center_y)->region;
                    for (int offset_index = 25; offset_index < 289; ++offset_index) {
                        const int offset_distance = (offset_index - 25) * 4;
                        const int offset_x = RadiusOffsetX[offset_index];
                        const int offset_y = RadiusOffsetY[offset_index];
                        int candidate_x = center_x + offset_x;
                        if (!(MapIsFlat & 1)) {
                            if (candidate_x < 0) candidate_x += MapLongitudeBounds;
                            else if (candidate_x >= MapLongitudeBounds)
                                candidate_x -= MapLongitudeBounds;
                        }
                        const int candidate_y = center_y + offset_y;
                        if (candidate_x < 0 || candidate_x >= MapLongitudeBounds ||
                            candidate_y < 0 || candidate_y >= MapLatitudeBounds) continue;

                        Map *candidate = map_loc(candidate_x, candidate_y);
                        const int altitude = candidate->climate & 0xE0;
                        const uint8_t candidate_region = candidate->region;
                        const bool aquatic = (player.rule_flags & RFLAG_AQUATIC) != 0;
                        if (((candidate->bit & 0x20u) != 0 && altitude >= 0x40) ||
                            (aquatic ? altitude >= 0x60 : altitude < 0x60)) continue;

                        const uint8_t owner_state = candidate->val3;
                        if ((owner_state & 0xC0) >= 0x41 && !aquatic) continue;
                        int site_bonus = 0;
                        if ((owner_state & 7) == 0 && (candidate->bit & 0x1000u) == 0) {
                            site_bonus = candidate->climate >> 4;
                            if (site_bonus == 0) {
                                site_bonus = world_site(candidate_x, candidate_y,
                                                        aquatic ? TRUE : FALSE);
                                site_set(candidate_x, candidate_y, site_bonus);
                            }
                        }

                        int score = (static_cast<int>(search_base.population_size) + site_bonus) * 32;
                        if (center_region != candidate_region)
                            score /= 2;
                        if (offset_distance > 0xDF && offset_distance < 0x240)
                            score = (score * 3) / 2;
                        if (offset_distance > 799)
                            score -= (score + (score >> 31 & 3)) >> 2;
                        if (offset_distance < 0x60) score /= 2;

                        const int nearest_base_id = base_find(candidate_x, candidate_y);
                        if (nearest_base_id != scan_base_id) {
                            const Base &nearest_base = Bases[nearest_base_id];
                            int dx = abs(candidate_x - nearest_base.x);
                            if (!(MapIsFlat & 1) && dx > static_cast<int>(MapLongitude))
                                dx = MapLongitudeBounds - dx;
                            const int dy = abs(candidate_y - nearest_base.y);
                            if (((dx + dy) & ~1) < 8) continue;
                        }
                        const Base &nearest_base = Bases[nearest_base_id];
                        if (nearest_base.faction_id_current != faction_id) {
                            if (map_loc(candidate_x, candidate_y)->region == candidate_region) continue;
                            score /= 2;
                        }
                        if (score > best_score) {
                            best_score = score;
                            best_x = candidate_x;
                            best_y = candidate_y;
                            best_base_id = scan_base_id;
                        }
                    }
                }
            }

            if (best_score <= 0) {
                if (GameDataStateStorage.fields_.base_current_count_ > 0) {
                    int smallest_base_id = -1;
                    int smallest_population = 999;
                    for (int candidate_base_id = 0;
                         candidate_base_id < GameDataStateStorage.fields_.base_current_count_; ++candidate_base_id) {
                        const Base &candidate_base = Bases[candidate_base_id];
                        if (candidate_base.faction_id_current == faction_id &&
                            candidate_base.population_size < smallest_population) {
                            smallest_population = candidate_base.population_size;
                            smallest_base_id = candidate_base_id;
                        }
                    }
                    if (smallest_base_id >= 0) {
                        Base &smallest_base = Bases[smallest_base_id];
                        if (smallest_base.population_size < 6)
                            smallest_base.population_size += 2;
                        set_fac(FAC_RECREATION_COMMONS, smallest_base_id, 1);
                        set_fac(FAC_RECYCLING_TANKS, smallest_base_id, 1);
                        set_fac(FAC_COMMAND_CENTER, smallest_base_id, 1);
                        int garrison = time_warp_find_prototype(faction_id, 1);
                        if (garrison >= 0) {
                            // Original bug (0x5AFDB2..0x5AFDC5): the result
                            // is discarded and the earlier veh_id gets morale.
                            veh_init(garrison, faction_id,
                                     smallest_base.x, smallest_base.y);
                            if (garrison == BSC_MIND_WORMS) Vehs[veh_id].morale = 2;
                        }
                        // Native 0x5AFD7F tests only the aquatic rule here.
                        if ((player.rule_flags & RFLAG_AQUATIC) == 0)
                            veh_init(0, faction_id, smallest_base.x, smallest_base.y);
                        else
                            veh_id = veh_init(21, faction_id, last_base_x, last_base_y);
                        veh_init(7, faction_id, smallest_base.x, smallest_base.y);
                    }
                }
                continue;
            }

            const int new_base_id = base_init(faction_id, best_x, best_y);
            Base &new_base = Bases[new_base_id];
            new_base.population_size = static_cast<int8_t>(expansion_count - expansion_round);
            if (new_base.population_size > 2) set_fac(FAC_RECREATION_COMMONS, new_base_id, 1);
            if (new_base.population_size > 3) set_fac(FAC_RECYCLING_TANKS, new_base_id, 1);
            new_base.nutrients_accumulated = static_cast<int>(new_base.population_size) * 10 / 2;

            veh_id = veh_init(opening_prototype, faction_id, best_x, best_y);
            if (opening_prototype == BSC_MIND_WORMS) Vehs[veh_id].morale = 2;
            Vehs[veh_id].order = ORDER_HOLD;
            if (new_base.population_size > 2) {
                veh_id = veh_init(opening_prototype, faction_id, best_x, best_y);
                if (opening_prototype == BSC_MIND_WORMS) Vehs[veh_id].morale = 2;
                Vehs[veh_id].order = ORDER_HOLD;
            }
            if (player_data.ai_fight != 0 || new_base.population_size > 3) {
                const int defender_prototype =
                    (player.rule_flags & RFLAG_AQUATIC) != 0 ? 4 : 1;
                veh_id = veh_init(defender_prototype, faction_id, best_x, best_y);
            }

            const Map *new_base_tile = map_loc(best_x, best_y);
            if (new_base_tile->region != starting_region && !placed_coastal_group &&
                is_coast(Bases[best_base_id].x, Bases[best_base_id].y, FALSE)) {
                for (int coast_index = 0; coast_index < 8; ++coast_index) {
                    int coast_x = best_x + VehMoveDeltaX[coast_index];
                    if (!(MapIsFlat & 1)) {
                        if (coast_x < 0) coast_x += MapLongitudeBounds;
                        else if (coast_x >= MapLongitudeBounds) coast_x -= MapLongitudeBounds;
                    }
                    const int coast_y = best_y + VehMoveDeltaY[coast_index];
                    if (coast_x < 0 || coast_x >= MapLongitudeBounds || coast_y < 0 ||
                        coast_y >= MapLatitudeBounds) continue;
                    if ((map_loc(coast_x, coast_y)->climate & 0xE0) >= 0x60) continue;
                    if (plan_six_prototype >= 0) {
                        VehPrototypes[plan_six_prototype].flags |= 4;
                        const int native_vehicle = veh_init(plan_six_prototype, faction_id,
                                                             coast_x, coast_y);
                        ++Vehs[native_vehicle].morale;
                    }
                    veh_init(has_foil_technology ? BSC_TRANSPORT_FOIL : BSC_UNITY_FOIL,
                             faction_id, coast_x, coast_y);
                    veh_id = veh_init(BSC_UNITY_ROVER, faction_id,
                                      coast_x, coast_y);
                    sleep(veh_id);
                    placed_coastal_group = true;
                    break;
                }
            }
        }

        AiResearchBatchActive = 1;
        reset_territory();
        enemy_strategy(faction_id);
        for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; ++base_id) {
            Base &base = Bases[base_id];
            if (base.faction_id_current != faction_id) continue;
            for (int synch_radius_index = 0; synch_radius_index < 81; ++synch_radius_index) {
                int radius_x = base.x + RadiusOffsetX[synch_radius_index];
                if (!(MapIsFlat & 1)) {
                    if (radius_x < 0) radius_x += MapLongitudeBounds;
                    else if (radius_x >= MapLongitudeBounds)
                        radius_x -= MapLongitudeBounds;
                }
                const int radius_y = base.y + RadiusOffsetY[synch_radius_index];
                if (radius_x < 0 || radius_x >= MapLongitudeBounds || radius_y < 0 ||
                    radius_y >= MapLatitudeBounds) continue;
                Map *tile = map_loc(radius_x, radius_y);
                tile->visibility |= static_cast<uint8_t>(1u << faction_id);
                if (synch_radius_index < 21 && goody_at(radius_x, radius_y) != 0)
                    tile->bit |= 0x00100400u;
            }
            set_base(base_id);
            base_compute(1);
            const int terraform_passes =
                (static_cast<int>(base.population_size) * 2) / 3 + player_data.ai_fight;
            for (int pass = 0; pass < terraform_passes; ++pass) {
                base_terraform(1);
                base_compute(1);
            }
            base.resource_sqr_radius = 0;

            for (int radius_index = 0; radius_index < 81; ++radius_index) {
                int radius_x = base.x + RadiusOffsetX[radius_index];
                if (!(MapIsFlat & 1)) {
                    if (radius_x < 0) radius_x += MapLongitudeBounds;
                    else if (radius_x >= MapLongitudeBounds)
                        radius_x -= MapLongitudeBounds;
                }
                const int radius_y = base.y + RadiusOffsetY[radius_index];
                if (radius_x >= 0 && radius_x < MapLongitudeBounds && radius_y >= 0 &&
                    radius_y < MapLatitudeBounds)
                    synch_bit(radius_x, radius_y, faction_id);
            }
            const int base_region = map_loc(base.x, base.y)->region;
            for (int other_base_id = 0; other_base_id < GameDataStateStorage.fields_.base_current_count_; ++other_base_id) {
                if (other_base_id == base_id ||
                    Bases[other_base_id].faction_id_current != faction_id) continue;
                Base &other_base = Bases[other_base_id];
                if (map_loc(other_base.x, other_base.y)->region != base_region) continue;
                const int has_tech_for_path =
                    has_tech(Rules.tech_build_roads_fungus, faction_id);
                const int path_mode = has_tech_for_path ? 0 : 4;
                int path_step = Paths.find(other_base.x, other_base.y, base.x, base.y,
                                           0, -1, path_mode, -1);
                int path_x = other_base.x;
                int path_y = other_base.y;
                int path_length = 0;
                while (path_step >= 0) {
                    path_x += VehMoveDeltaX[path_step];
                    if (!(MapIsFlat & 1)) {
                        if (path_x < 0) path_x += MapLongitudeBounds;
                        else if (path_x >= MapLongitudeBounds)
                            path_x -= MapLongitudeBounds;
                    }
                    path_y += VehMoveDeltaY[path_step];
                    if (path_x == base.x && path_y == base.y) break;
                    Map *path_tile = map_loc(path_x, path_y);
                    if ((path_tile->bit & 1u) == 0 || (path_tile->val2 & 0x0F) > 7) {
                        if ((path_tile->bit & 4u) == 0) {
                            path_tile->bit |= 4u;
                            synch_bit(path_x, path_y, faction_id);
                        }
                    }
                    ++path_length;
                    if (path_length > 50) break;
                    const int can_road_fungus =
                        has_tech(Rules.tech_build_roads_fungus, faction_id);
                    path_step = Paths.find(path_x, path_y, base.x, base.y,
                                           0, -1, can_road_fungus ? 0 : 4, -1);
                }
            }
            base_reset(base_id, 0);
        }
        AiResearchBatchActive = 0;

        for (veh_id = 0; veh_id < GameDataStateStorage.fields_.vehicle_current_count_; ++veh_id) {
            Veh &vehicle = Vehs[veh_id];
            if (vehicle.faction_id != faction_id ||
                VehPrototypes[vehicle.proto_id].plan != 9) continue;
            const int native_triad = Chassis[VehPrototypes[vehicle.proto_id].chassis_id].triad;
            const bool aquatic = (player.rule_flags & RFLAG_AQUATIC) != 0;
            if ((!aquatic && native_triad != TRIAD_LAND) ||
                (aquatic && native_triad != TRIAD_SEA)) continue;
            int best_quality = 0;
            for (int move_index = 0; move_index < 8; ++move_index) {
                int destination_x = vehicle.x + VehMoveDeltaX[move_index];
                if (!(MapIsFlat & 1)) {
                    if (destination_x < 0) destination_x += MapLongitudeBounds;
                    else if (destination_x >= MapLongitudeBounds)
                        destination_x -= MapLongitudeBounds;
                }
                const int destination_y = vehicle.y + VehMoveDeltaY[move_index];
                if (destination_x < 0 || destination_x >= MapLongitudeBounds ||
                    destination_y < 0 || destination_y >= MapLatitudeBounds) continue;
                const Map *destination = map_loc(destination_x, destination_y);
                const int altitude = destination->climate & 0xE0;
                if ((!aquatic && altitude < 0x60) || (aquatic && altitude >= 0x60)) continue;
                int quality = 1;
                const uint8_t owner_state = destination->val3 & 0xC0;
                if (owner_state < 0x80) quality = 2;
                if ((destination->bit & 4u) != 0) quality += 2;
                if (owner_state == 0x40 && (destination->climate & 0x18) != 0 &&
                    (destination->climate >> 5) > 2)
                    quality += 2;
                if (quality > best_quality) {
                    best_quality = quality;
                    veh_drop(veh_lift(veh_id), destination_x, destination_y);
                }
            }
        }
    }
    GameDataStateStorage.fields_.turn_current_num_ = scenario_turn;
}

// ORIGINAL: 0x005B0420 ?balance@@YAXXZ 0x005B0420-0x005B0A2C
// RECOVERY_BINDING: v1 {"evidence": ["Retained game-setup-owner-wave full body and seed005B0420; independent pinned-span review by report_draw_labs_review; actual-source test_setup_balance_behavior controls. Native faction rule bit0x100 is aquatic; no historical match claim copied."], "reason": "Recover the complete starting-position balance operation with genuine vehicle, faction, continent and map owners, retaining native neighbor scoring, defender gating and the mutable resource-placement bound."}
// TRIED: array-index form for vehDir/dirScore instead of the original's pointer-walk locals (lea + running ptr in [ebp-0x14]/[ebp-8]); diverges by instruction #3, before the veh_find loop even starts, so this is a stack-layout/register-allocation gap, not a logic one - full instruction-level transcription of both passes (7-direction scoring, deficit-driven placement) landed and is structurally faithful, just not byte-identical.
void __cdecl balance() {
    int vehicles_by_faction[8];
    int faction_scores[8];
    for (int faction_id = 1; faction_id < MaxPlayerNum; ++faction_id) {
        int vehicle_id = veh_find(0, 0, faction_id, -1);
        vehicles_by_faction[faction_id] = vehicle_id;
        if (vehicle_id >= 0) {
            const int y = Vehs[vehicle_id].y;
            const int x = Vehs[vehicle_id].x;
            if (map_loc(x, y)->region >= 0x40) {
                vehicles_by_faction[faction_id] = veh_find(x, y, faction_id, vehicle_id);
            }
        }
    }

    int best_score = 0;
    for (faction_id = 1; faction_id < MaxPlayerNum; ++faction_id) {
        const int vehicle_id = vehicles_by_faction[faction_id];
        if (vehicle_id < 0) {
            continue;
        }
        const int x = Vehs[vehicle_id].x;
        const int y = Vehs[vehicle_id].y;
        const int region = map_loc(x, y)->region;
        int nearest = 9999;
        for (int other_id = 1; other_id < MaxPlayerNum; ++other_id) {
            if (faction_id == other_id) {
                continue;
            }
            const int other_vehicle = vehicles_by_faction[other_id];
            if (other_vehicle < 0) {
                continue;
            }
            const int other_x = Vehs[other_vehicle].x;
            const int other_y = Vehs[other_vehicle].y;
            if (region != map_loc(other_x, other_y)->region) {
                continue;
            }
            const int dist = vector_dist(x, y, other_x, other_y) * 25 * 128 / MapArea;
            if (dist < nearest) {
                nearest = dist;
            }
        }
        if (nearest >= 1000) {
            int size_score = (Continents[region].tile_count + 31) / 32;
            if (size_score >= 1) {
                if (size_score > 8) {
                    size_score = 8;
                }
            } else {
                size_score = 1;
            }
            nearest = size_score;
        } else {
            nearest = nearest / 5 - 3;
        }

        int land_count = 0;
        for (int i = 0; i < 21; ++i) {
            const int tile_x = wrap_map_x(RadiusOffsetX[i] + x);
            const int tile_y = RadiusOffsetY[i] + y;
            if (!on_map(tile_x, tile_y)) {
                continue;
            }
            const Map *tile = map_loc(tile_x, tile_y);
            const int rainfall = (tile->climate >> 3) & 3;
            const int altitude = (tile->climate >> 5) - 2;
            const int rockiness = tile->val3 >> 6;
            if (rainfall < 1) {
                continue;
            }
            if (((altitude >= 0 && altitude > 0) || (tile->bit & BIT_RIVER)) && rockiness < 2 &&
                (rockiness == 1 || (altitude >= 0 && altitude > 1) || rainfall > 1)) {
                ++land_count;
            }
            if (rockiness == 1 && altitude >= 0 && altitude > rockiness) {
                ++land_count;
            }
        }

        // Image 0x005B0748..0x005B0790: half the land count clamped to 0..8,
        // plus tile_count / 64 (no rounding) clamped to 0..3.
        const int total = range(land_count / 2, 0, 8) +
                          range(Continents[region].tile_count / 64, 0, 3) + nearest;
        if (total > best_score) {
            best_score = total;
        }
        faction_scores[faction_id] = total;
    }

    for (faction_id = 1; faction_id < MaxPlayerNum; ++faction_id) {
        int vehicle_id = vehicles_by_faction[faction_id];
        if (vehicle_id < 0) {
            continue;
        }
        const int x = Vehs[vehicle_id].x;
        const int y = Vehs[vehicle_id].y;
        const int seed = rand() % 16;
        int placed = 0;
        int deficit = best_score - faction_scores[faction_id];
        for (int bonus = 0; bonus < Players[faction_id].faction_bonus_count; ++bonus) {
            if (Players[faction_id].faction_bonus_id[bonus] == 1) {
                deficit -= VehPrototypes[Players[faction_id].faction_bonus_val1[bonus]].plan == 8 ? 3 : 1;
            }
        }
        if (GameDataStateStorage.fields_.diff_level_current_ > 1 &&
            (deficit >= 3 ||
             GameDataStateStorage.fields_.diff_level_current_ >=
                 ((1 << faction_id) & GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ ? 4 : 5))) {
            deficit -= 3;
            if (!(Players[faction_id].rule_flags & RFLAG_AQUATIC)) {
                vehicle_id = veh_init(0, faction_id, x, y);
            }
            // Native 0x5B08C6 clears home_base_id (+0x2E); the aquatic
            // branch applies it to the earlier unit.
            Vehs[vehicle_id].home_base_id = -1;
        }
        if (GameDataStateStorage.fields_.diff_level_current_ < 2) {
            deficit = (deficit + 1) / 2;
        }
        int angle = seed * 7;
        for (int count = 0; count < deficit; ++count) {
            angle += 7;
            const int index = angle % 16 + 9;
            const int tile_x = wrap_map_x(RadiusOffsetX[index] + x);
            const int tile_y = RadiusOffsetY[index] + y;
            if (!on_map(tile_x, tile_y)) {
                continue;
            }
            if (goody_at(tile_x, tile_y) && !(map_loc(tile_x, tile_y)->bit & 0x0C000000u)) {
                --faction_scores[faction_id];
            }
            if (placed < 2 && deficit - count >= 3 && !((count ^ placed) & 1)) {
                bit_set(tile_x, tile_y, 0x04000000, 1);
                --deficit;
                ++placed;
            } else {
                bit_set(tile_x, tile_y, 0x08000000, 1);
            }
        }
    }
}

// The image expands this four-argument distance once per candidate unit.
static inline int setup_dist(int x_point_a, int y_point_a, int x_point_b, int y_point_b) {
    return (x_dist(x_point_a, x_point_b) + abs(y_point_a - y_point_b)) >> 1;
}

// ORIGINAL: 0x005B0A30 ?scenario_setup@@YAXXZ 0x005B0A30-0x005B0D69
// RECOVERY_BINDING: v1 {"reason":"Recover complete descending vehicle-placement retries with real prototypes/chassis/Map records, native odd-y x adjustment and failed-placement deletion. Original68FAF0 is MapLongitude for tile-row stride and wrap threshold, not full MapLongitudeBounds; air-only placement does not form a terrain pointer. No byte/reference or runtime claim is granted.","evidence":["git:0de05de6:src/unrecovered/005b0a30.cpp","image:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf","recovery/tasks/scenario-artifact-unity-owner-coverage.json","Original full span and native physical COFF emitted symbol ?scenario_setup@@YAXXZ; existing source owns all referenced storage and services."]}
// TRIED: literal asm-mirrored translation (nested short-circuit accept flags for zone/tile/veh-stack checks, best-distance search loop) compiles and matches through instr #4; diverges at #5 (test vs mov, prologue-adjacent) - landed at MISMATCH.
void __cdecl scenario_setup() {
    for (int vehicle_id = GameDataStateStorage.fields_.vehicle_current_count_ - 1; vehicle_id >= 0; --vehicle_id) {
        if (!(Vehs[vehicle_id].flags & (VFLAG_START_RAND_LOCATION | VFLAG_START_RAND_MONOLITH
                | VFLAG_START_RAND_FUNGUS))) {
            continue;
        }
        veh_lift(vehicle_id);
        int counter;
        int x;
        int y;
        for (counter = 0; counter < 10000; counter++) {
            x = bounded_crt_random(MapLongitudeBounds - (MapIsFlat & 1) * 4) + (MapIsFlat & 1) * 2;
            y = bounded_crt_random(MapLatitudeBounds - 4) + 2;
            if (x & 1) {
                x--;
            }
            // Preserve the original store: odd y increments x, not y.
            if (y & 1) {
                x++;
            }
            if (get_proto_triad(Vehs[vehicle_id].proto_id) != TRIAD_AIR
                && ((map_loc(x, y)->climate & ~0x1F) < ALT_BIT_SHORE_LINE)
                    != (get_proto_triad(Vehs[vehicle_id].proto_id) == TRIAD_SEA)
                && counter < 0x26DE) {
                continue;
            }
            if ((Vehs[vehicle_id].flags & VFLAG_START_RAND_MONOLITH)
                && !(map_loc(x, y)->bit & BIT_MONOLITH) && counter < 0x26AC) {
                continue;
            }
            if ((Vehs[vehicle_id].flags & VFLAG_START_RAND_FUNGUS)
                && !((map_loc(x, y)->bit & BIT_FUNGUS) && (map_loc(x, y)->climate & ~0x1F) >= 0x40)
                && counter < 0x26AC) {
                continue;
            }
            int occupant = veh_at(x, y);
            if (occupant >= 0) {
                if (Vehs[occupant].faction_id != Vehs[vehicle_id].faction_id) {
                    continue;
                }
                if (counter < (stack_check(occupant, 1, -1, -1, -1) + 10) * 500) {
                    continue;
                }
            }
            int best_distance = 9999;
            for (int other_id = 0; other_id < GameDataStateStorage.fields_.vehicle_current_count_; ++other_id) {
                if (vehicle_id != other_id) {
                    int distance = setup_dist(x, y, Vehs[other_id].x, Vehs[other_id].y);
                    if (distance < best_distance) {
                        best_distance = distance;
                    }
                }
            }
            base_find(x, y);
            if (best_distance >= BaseFindDist) {
                best_distance = BaseFindDist;
            }
            if (counter >= (0x20 - best_distance) * 100) {
                break;
            }
        }
        veh_drop(vehicle_id, x, y);
        if (counter >= 10000) {
            veh_kill(vehicle_id);
        }
    }
}

/*
Purpose: Calculate the basic social engineering modifiers for the specified faction.
// ORIGINAL: 0x005B0D70 ?compute_faction_modifiers@@YAXH@Z 0x005B0D70-0x005B0DF3 BYTE_EXACT
// LEVER: three WALKING POINTERS (bonus_id/bonus_val1/bonus_val2) advanced
//        together in the `for`'s increment, rather than `faction_bonus_id[i]`
//        &c re-subscripted each iteration - the image walks ONE pointer and
//        reaches the other two arrays through fixed +-0x20 offsets from it
//        (all three are 8-int arrays, 32 bytes apart), which three parallel
//        pointers incremented by a constant stride lets VC6 fold the same
//        way. Moved 6/48 (0.757) -> 6/48 (0.875) - same raw count, tighter
//        similarity, and the image's `push ebp`/`ebp`-frame prologue is
//        restored, which the re-subscripted form did not have at any flag
//        set tried.
// size      131 bytes
// prototype void (__cdecl ?compute_faction_modifiers@@YAXH@Z)(int factionID)
// callers   4   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl compute_faction_modifiers(int faction_id) {
    for (int effect = 0; effect < MaxSocialEffectNum; ++effect) {
        PlayersData[faction_id].soc_effect_base.values_[effect] = 0;
    }
    int count = Players[faction_id].faction_bonus_count;
    for (int i = 0; i < count; i++) {
        if (Players[faction_id].faction_bonus_id[i] == RULE_SOCIAL) {
            PlayersData[faction_id].soc_effect_base.values_[Players[faction_id].faction_bonus_val1[i]]
                += Players[faction_id].faction_bonus_val2[i];
        }
    }
}

static int lifecycle_wrap_x(int x) {
    if (!(MapIsFlat & 1)) {
        if (x < 0) x += MapLongitudeBounds;
        else if (x >= MapLongitudeBounds) x -= MapLongitudeBounds;
    }
    return x;
}

static uint8_t lifecycle_faction_mask(int faction) {
    return static_cast<uint8_t>(1u << (faction & 31));
}

static bool lifecycle_is_fresh(int conqueror) {
    return GameDataStateStorage.fields_.turn_current_num_ == 0 || conqueror == -282;
}

static int lifecycle_distance(int x, int y, int other_x, int other_y) {
    int dx = abs(x - other_x);
    if (!(MapIsFlat & 1) && dx > static_cast<int>(MapLongitude)) dx = MapLongitudeBounds - dx;
    dx = abs(dx);
    const int dy = abs(y - other_y);
    const int hi = dx > dy ? dx : dy;
    const int lo = dx < dy ? dx : dy;
    return hi - ((((dx + dy) >> 1) - lo + 1) >> 1);
}

static void lifecycle_clear_social(SocialEffect &effect) {
    SocialEffect zero = {0};
    effect = zero;
}

static void lifecycle_add_social(SocialEffect &effect, int index, int value) {
    // Original unchecked indexed operation on the true eleven-word array.
    effect.values_[index] += value;
}

static void lifecycle_reset_faction(int faction) {
    PlayerData &p = PlayersData[faction];
    p.flags = 0;
    p.base_governor_adv = GameDataStateStorage.fields_.diff_level_current_ ? 0xdfbe41 : 0xc0dfbe41;
    p.base_name_offset = p.base_sea_name_offset = 0;
    p.ai_fight = Players[faction].ai_fight;
    p.ai_growth = Players[faction].ai_growth;
    p.ai_tech = Players[faction].ai_tech;
    p.ai_wealth = Players[faction].ai_wealth;
    p.ai_power = Players[faction].ai_power;
    p.unk_37 = 0;
    for (int queue = 0; queue < 8; ++queue) {
        p.saved_queue_size[queue] = 0;
        StringTemp[0] = 0;
        strcat(StringTemp, label_get(0x984 / 4));
        strcat(StringTemp, " ");
        char number[80];
        _itoa(queue + 1, number, 10);
        strcat(StringTemp, number);
        strcpy(p.saved_queue_name[queue], StringTemp);
    }
    for (int category = 0; category < 4; ++category)
        p.soc_category_pending.models_[category] = p.soc_category_active.models_[category] = 0;
    lifecycle_clear_social(p.soc_effect_pending);
    p.soc_upheaval_cost_paid = 0;
    for (int other = 0; other < 8; ++other) p.secret_project_intel[other] = 0;
    p.diff_level = (GameDataStateStorage.fields_.lock_masks_.enable_mask_ & lifecycle_faction_mask(faction)) ? GameDataStateStorage.fields_.diff_level_current_ : 3;
    p.integrity_blemishes = p.global_reputation = 0;
    p.traded_maps = p.energy_reserves = p.atrocities = p.major_atrocities = 0;
    p.planet_ecology = p.sanction_turns = 0;
    TectonicDetonationCount[faction] = 0;
    p.council_call_turn = -999;
    if (GameDataStateStorage.fields_.game_state_ & 0x800000) {
        p.ai_fight = rand() % 3 - 1;
        p.ai_power = rand() % 2;
        p.ai_tech = rand() % 2;
        p.ai_wealth = rand() % 2;
        p.ai_growth = rand() % 2;
    }
    if (GameDataStateStorage.fields_.game_rules_ & 0x40) p.ai_fight = 1;
    p.total_mil_veh = p.current_num_bases = p.mil_strength_1 = p.mil_strength_2 = p.pop_total = p.unk_70 = 0;
    p.mind_control_total = 0;
    for (int other2 = 0; other2 < 8; ++other2)
        p.diplo_mind_control[other2] = p.stolen_data_count[other2] = 0;
    p.net_random_event = 0;
    for (int region = 0; region < 128; ++region) {
        p.region_total_combat_vehs[region] = p.region_total_bases[region] = 0;
        p.region_total_offensive_vehs[region] = p.region_force_rating[region] = p.unk_77[region] = 0;
    }
    for (int council = 0; council < 11; ++council) p.unk_29[council] = p.unk_30[council] = 0;
    p.corner_market_turn = p.corner_market_active = p.unk_101 = p.unk_102 = p.prototype_history_cursor = 0;
    p.flags_ext = p.unk_105 = p.unk_106 = p.unk_107 = p.unk_108 = p.unk_109 = 0;
    p.unk_93 = p.unk_94 = p.unk_95 = p.unk_96 = p.unk_97 = p.tech_achieved = 0;
    p.time_bonus_count = 1;
}

// ORIGINAL: 0x005B0E00 ?setup_player@@YAHHHH@Z 0x005B0E00-0x005B337C
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/005b0e00.cpp: canonical original identity, full spans and retained seed provenance", "source:src/faction.h: real faction record ownership and typed relationships", "source:recovery/tasks/faction-lifecycle-owner-closure.json: coordinator authorization and retained task baseline"], "reason": "The original faction setup operation belongs to the Faction owner; real Player/PlayerData records, prototype scoring, owned network state, shared game counters and named base/vehicle dependencies express creation and respawn paths without seed executable addresses."}
// TRIED: full-body transcription from Ghidra hypothesis, corrected against raw disasm for: tech_ai/tech_achieved and abil_index/ has_tech dangling-outer-call pushes (same shape as time_warp's has_tech fold), veh_lift/veh_drop split (PROPOSAL: veh_lift should be `int veh_lift(int)`), CharUpperA reached through the IAT thunk at g_0066931c (matches src/recovered's established idiom), and per-field pointer widths across the ~230 referenced globals recovered from `movsx`/`test byte ptr` on the raw disassembly, since the scaffold's g_ globals are declared int* uniformly. All C2xxx NO_COMPILE errors from iterative verify_recovered_function.py runs were fixed; the final byte-tier measurement did not finish inside this session (a single VC6 /O2 pass on this body ran >12 minutes without completing - the same body compiles other optimisation levels' passes fine, so this reads as a slow pass, not a stuck one).
int __cdecl setup_player(int faction, int conqueror, int retry) {
    Player &definition = Players[faction];
    PlayerData &p = PlayersData[faction];
    char number[80];
    char prototype_name[32];
    int duplicate = 0;
    for (int previous = 1; previous < faction; ++previous)
        if (!strcmp(Players[previous].name_leader, definition.name_leader)) ++duplicate;
    if (duplicate) {
        definition.name_leader[21] = 0;
        strcat(definition.name_leader, "-");
        _itoa(duplicate + 1, number, 10);
        strcat(definition.name_leader, number);
    }
    duplicate = 0;
    for (int previous2 = 1; previous2 < faction; ++previous2)
        if (!strcmp(Players[previous2].noun_faction, definition.noun_faction)) ++duplicate;
    if (duplicate) {
        definition.noun_faction[21] = 0;
        strcat(definition.noun_faction, "-");
        _itoa(duplicate + 1, number, 10);
        strcat(definition.noun_faction, number);
    }
    duplicate = 0;
    for (int previous3 = 1; previous3 < faction; ++previous3)
        if (!strcmp(Players[previous3].formal_name_faction, definition.formal_name_faction)) ++duplicate;
    if (duplicate) {
        definition.formal_name_faction[37] = 0;
        strcat(definition.formal_name_faction, "-");
        _itoa(duplicate + 1, number, 10);
        strcat(definition.formal_name_faction, number);
    }
    for (int base = GameDataStateStorage.fields_.base_current_count_ - 1; base >= 0; --base)
        if (Bases[base].faction_id_current == faction) base_kill(base);
    for (int vehicle = GameDataStateStorage.fields_.vehicle_current_count_ - 1; vehicle >= 0; --vehicle)
        if (vehicle < GameDataStateStorage.fields_.vehicle_current_count_ && Vehs[vehicle].faction_id == faction
            && (Vehs[vehicle].proto_id != 0x13 || faction != 0)) kill(vehicle);
    p.x_target = p.y_target = p.base_id_atk_target = -1;
    if (lifecycle_is_fresh(conqueror)) {
        init_goals(faction);
        clear_bunglist(faction);
        lifecycle_reset_faction(faction);
    }
    p.energy_reserves += definition.rule_energy
        + (IsMultiplayerNet ? 100 : Rules.player_start_energy_reserve);
    p.hurry_cost_total = 0;
    p.tech_cost = -1;
    p.last_turn_new_base = GameDataStateStorage.fields_.turn_current_num_;
    for (int other = 0; other < 8; ++other) {
        if (lifecycle_is_fresh(conqueror)) {
            p.diplo_treaties[other] = p.diplo_merc[other] = 0;
            int limit = ((GameDataStateStorage.fields_.game_rules_ & 0x40) ? 5 : GameDataStateStorage.fields_.diff_level_current_) * 3 + 8;
            int friction = limit <= 1 ? 1 : rand() % limit;
            if (friction < 1) friction = 1;
            if (friction > 16) friction = 16;
            p.diplo_friction[other] = friction;
            p.diplo_wrongs[other] = p.diplo_betrayed[other] = p.diplo_unk3[other] = 0;
            p.diplo_patience[other] = 0;
            p.diplo_spoke[other] = -1;
            p.diplo_unk4[other] = p.diplo_unk1[other] = 0;
        } else treaty_off(faction, other, 0x9000ff48);
        p.loan_balance[other] = p.loan_payment[other] = p.unk_12[other] = 0;
    }
    for (int proto = 0; proto < 512; ++proto) {
        p.proto_id_active[proto] = p.proto_id_queue[proto] = 0;
        if (GameDataStateStorage.fields_.turn_current_num_ == 0) p.proto_id_lost[proto] = 0;
    }
    if (lifecycle_is_fresh(conqueror)) {
        for (int goal = 0; goal < 12; ++goal) p.tech_pact_shared_goals[goal] = 0;
        // The original clears22 words and then one byte, leaving the last
        // three bytes of this92-byte field unchanged.
        for (int source = 0; source < 89; ++source) p.tech_trade_source[source] = 0;
        for (int announced = 0; announced < 9; ++announced) p.facility_announced[announced] = 0;
        for (int satellite = 0; satellite < 4; ++satellite) p.satellites_[satellite] = 0;
        p.theory_of_everything = p.tech_ranking = p.sat_odp_deployed = 0;
        p.tech_id_researching = -1;
        p.tech_accumulated = p.earned_techs_saved = p.unk_26 = p.unk_102 = 0;
    }
    p.unk_28 = p.unk_13 = p.maint_cost_total = p.unk_18 = p.unk_17 = 0;
    p.tech_commerce_bonus = p.turn_commerce_income = 0;
    p.tech_fungus_[RSC_NUTRIENTS] = p.tech_fungus_[RSC_MINERALS] = p.tech_fungus_[RSC_ENERGY] = p.tech_fungus_[3] = 0;
    p.best_weapon_value = p.best_armor_value = p.best_land_speed = 0;
    p.enemy_best_weapon_value = p.enemy_best_armor_value = p.enemy_best_land_speed = 0;
    p.best_psi_offense = p.best_psi_defense = p.enemy_best_psi_offense = p.enemy_best_psi_defense = 0;
    if (!faction || !((GameDataStateStorage.fields_.lock_masks_.enable_mask_ >> 8) & lifecycle_faction_mask(faction))) return 0;
    if (conqueror < -1 && conqueror != -282) {
        GameDataStateStorage.fields_.lock_masks_.enable_mask_ &= ~(static_cast<uint32_t>(lifecycle_faction_mask(faction)) << 8);
        return 0;
    }

    int attempts = 0;
    int x = 0, y = 0, region = 0, nearest = -1, occupied = 0, isolation = 32;
    int failed = 0;
    int area_quality = 0;
    int spacing_relaxation = 0;
    Map *tile = 0;
search:
    {
        int margin = (1024 - attempts) * MapLongitudeBounds / 8192;
        if (margin < 0) margin = 0;
        if (margin > 9999) margin = 9999;
        int inset = (MapIsFlat & 1) ? margin * 2 : 0;
        int width = MapLongitudeBounds - inset;
        x = ((MapIsFlat & 1) ? margin : 0) + (width <= 1 ? 0 : rand() % width);
        margin = (1024 - attempts) * MapLatitudeBounds / 8192;
        if (margin < 0) margin = 0;
        if (margin > 9999) margin = 9999;
        int height = MapLatitudeBounds - 2 * margin;
        y = margin + (height <= 1 ? 0 : rand() % height);
        if (x & 1) --x;
        if (y & 1) ++x;
        isolation = 32;
        for (int ring = 0; ring < 8; ++ring) {
            const int begin = ring ? RadiusRange[ring] : 0;
            for (int offset = begin; offset < RadiusRange[ring + 1]; ++offset) {
                int rx = lifecycle_wrap_x(x + RadiusOffsetX[offset]);
                int ry = y + RadiusOffsetY[offset];
                if (rx >= 0 && rx < MapLongitudeBounds && ry >= 0 && ry < MapLatitudeBounds) {
                    uint32_t landmark = map_loc(rx, ry)->bit2;
                    if (landmark && static_cast<int>(landmark & 0xff000000) > 0) {
                        isolation = ring;
                        ring = 99;
                        break;
                    }
                }
            }
        }
        area_quality = 0;
        for (int offset2 = 0; offset2 < 21; ++offset2) {
            int rx = lifecycle_wrap_x(x + RadiusOffsetX[offset2]);
            int ry = y + RadiusOffsetY[offset2];
            if (rx < 0 || rx >= MapLongitudeBounds || ry < 0 || ry >= MapLatitudeBounds) continue;
            Map &square = *map_loc(rx, ry);
            const int altitude = square.climate & 0xe0;
            if (!(definition.rule_flags & 0x100)) {
                if ((!(square.bit & 0x20) || altitude < 0x40)
                    && altitude > 0x5f && (square.val3 & 0xc0) < 0x41
                    && (square.climate & 0x18)) ++area_quality;
            } else if (!(square.bit & 0x20) || altitude < 0x40)
                area_quality += (2 - (square.climate >> 5)) * 2 + 1;
        }
        if (definition.rule_flags & 0x100) area_quality /= 3;
        region = map_loc(x, y)->region;
        occupied = 0;
        for (int player = 1; player < 8; ++player)
            if (PlayersData[player].region_total_bases[region]) occupied |= 1 << player;
        nearest = (!(definition.rule_flags & 0x100) && conqueror != -282)
            ? base_find(x, y, -1, region, -1, -1) : base_find(x, y);
        if (nearest < 0) BaseFindDist = 32;
        if (lifecycle_is_fresh(conqueror)) {
            for (int vehicle2 = 0; vehicle2 < GameDataStateStorage.fields_.vehicle_current_count_; ++vehicle2) {
                if (map_loc(Vehs[vehicle2].x, Vehs[vehicle2].y)->region != region) continue;
                occupied |= 1 << Vehs[vehicle2].faction_id;
                int distance = lifecycle_distance(x, y, Vehs[vehicle2].x, Vehs[vehicle2].y);
                if (distance < BaseFindDist) BaseFindDist = distance;
            }
        }
        ++attempts;
        if (attempts < 1025 || ((GameDataStateStorage.fields_.lock_masks_.enable_mask_ & lifecycle_faction_mask(faction)) && GameDataStateStorage.fields_.turn_current_num_ == 0)) {
            tile = map_loc(x, y);
            int altitude = tile->climate & 0xe0;
            bool bad_altitude = (definition.rule_flags & 0x100) ? altitude > 0x40 : altitude < 0x60;
            if (bad_altitude || ((tile->bit & 0x20) && altitude > 0x3f)
                || (!(definition.rule_flags & 0x100) && (tile->val3 & 0xc0) > 0x40)) goto search;
            int separation = lifecycle_is_fresh(conqueror) ? 6 : 12;
            spacing_relaxation = attempts / 64;
            if (separation * MapAreaSqRoot / 56 > BaseFindDist
                || altitude >= 0xa0 || ((tile->bit & 3) && (tile->val2 & 15) <= 7)
                || y <= 1 || y >= MapLatitudeBounds - 2
                || 12 - attempts / 80 > area_quality
                || (16 - spacing_relaxation) * MapAreaSqRoot / 56 > BaseFindDist
                || (lifecycle_is_fresh(conqueror) && attempts <= 899 && 36 - spacing_relaxation > Continents[region].tile_count)) goto search;
            if (!(definition.rule_flags & 0x100)) {
                int handicap = (GameDataStateStorage.fields_.lock_masks_.enable_mask_ & lifecycle_faction_mask(faction)) ? (6 - GameDataStateStorage.fields_.diff_level_current_) * 8 : 8;
                // Native5B1CEB LEA/5B1CF5 IMUL preserve low-dword arithmetic.
                if (static_cast<int>(Continents[region].open_terrain
                    * (static_cast<uint32_t>(MapOceanCoverage) + 1u)) / 2
                    < 24 + handicap - GameDataStateStorage.fields_.turn_current_num_ / 16 - attempts / 16) goto search;
            }
            int difficulty = (GameDataStateStorage.fields_.lock_masks_.enable_mask_ & lifecycle_faction_mask(faction)) ? GameDataStateStorage.fields_.diff_level_current_ : 3;
            if ((5 - difficulty) * 48 - GameDataStateStorage.fields_.turn_current_num_ / 16 - attempts / 4 > Continents[region].tile_count
                || (occupied && conqueror != -282
                    && 64 - spacing_relaxation > Continents[region].tile_count / (bit_count(occupied) + 1))) goto search;
            int quality = (tile->bit & 0x1000) ? 0 : tile->val2 >> 4;
            if (!(tile->bit & 0x1000) && !quality) {
                quality = world_site(x, y, 0);
                site_set(x, y, quality);
            }
            if ((15 - spacing_relaxation > quality && !GameDataStateStorage.fields_.turn_current_num_ && !(GameDataStateStorage.fields_.game_rules_ & 0x10))
                || (!(definition.rule_flags & 0x100) && ((1024 - attempts) / 128 > isolation || !isolation))
                || (GameDataStateStorage.fields_.turn_current_num_ && conqueror != -282 && tile->territory >= 1)
                || (nearest >= 0 && Bases[nearest].faction_id_current == conqueror
                    && map_loc(Bases[nearest].x, Bases[nearest].y)->region == region)) goto search;
        } else failed = 1;
    }
    if (p.flags & 0x1000) failed = 1;
    if (!failed || retry != 0) {
        if (lifecycle_is_fresh(conqueror)) {
            int best = -1, best_offset = 0;
            for (int offset3 = 0; offset3 < 21; ++offset3) {
                int rx = lifecycle_wrap_x(x + RadiusOffsetX[offset3]);
                int ry = y + RadiusOffsetY[offset3];
                if (rx < 0 || rx >= MapLongitudeBounds || ry < 0 || ry >= MapLatitudeBounds) continue;
                Map &candidate = *map_loc(rx, ry);
                int altitude = candidate.climate & 0xe0;
                if ((altitude < 0x60) != ((definition.rule_flags >> 8) & 1)
                    || candidate.region != region || altitude >= 0xa0
                    || ((candidate.bit & 0x20) && altitude >= 0x40)
                    || ((candidate.bit & 3) && (candidate.val2 & 15) <= 7)) continue;
                base_find(rx, ry);
                // Original5B204E checks distance from the old center, not
                // from this candidate. Keep that quirk when choosing a site.
                for (int vehicle3 = 0; vehicle3 < GameDataStateStorage.fields_.vehicle_current_count_; ++vehicle3) {
                    int distance = lifecycle_distance(x, y, Vehs[vehicle3].x, Vehs[vehicle3].y);
                    if (distance < BaseFindDist) BaseFindDist = distance;
                }
                if (BaseFindDist <= 5) continue;
                int quality = (candidate.bit & 0x1000) ? 0 : candidate.val2 >> 4;
                if (!(candidate.bit & 0x1000) && !quality) {
                    quality = world_site(rx, ry, 0);
                    site_set(rx, ry, quality);
                }
                for (int neighbor = 0; neighbor < 8; ++neighbor) {
                    int nx = lifecycle_wrap_x(rx + VehMoveDeltaX[neighbor]);
                    int ny = ry + VehMoveDeltaY[neighbor];
                    if (nx < 0 || nx >= MapLongitudeBounds || ny < 0 || ny >= MapLatitudeBounds) continue;
                    int altitude2 = map_loc(nx, ny)->climate >> 5;
                    if (!(definition.rule_flags & 0x100)) {
                        if (altitude2 < 3) { ++quality; break; }
                    } else quality += 2 - altitude2;
                }
                if (quality > best) { best = quality; best_offset = offset3; }
            }
            x = lifecycle_wrap_x(x + RadiusOffsetX[best_offset]);
            y += RadiusOffsetY[best_offset];
        }
        if (retry != 0) failed = 0;
        if (!failed && (lifecycle_is_fresh(conqueror) || !(GameDataStateStorage.fields_.game_rules_ & 1)) && GameDataStateStorage.fields_.turn_current_num_ < 101) goto respawn;
    }
    if (!retry) {
        if ((GameDataStateStorage.fields_.lock_masks_.enable_mask_ & lifecycle_faction_mask(faction)) && !IsMultiplayerNet) {
            EndgameExitReason = 7;
            GameDataStateStorage.fields_.lock_masks_.enable_mask_ &= ~(static_cast<uint32_t>(lifecycle_faction_mask(faction)) << 8);
            GameDataStateStorage.fields_.game_state_ |= 1;
            return 0;
        }
        p.unk_102 = 0;
        GameDataStateStorage.fields_.lock_masks_.enable_mask_ &= ~(static_cast<uint32_t>(lifecycle_faction_mask(faction)) << 8);
        int selected = faction == static_cast<int>(GameDataStateStorage.fields_.faction_rankings_[7])
            ? static_cast<int>(GameDataStateStorage.fields_.faction_rankings_[6])
            : static_cast<int>(GameDataStateStorage.fields_.faction_rankings_[7]);
        int survivors = 0, partners = 0;
        for (int survivor = 1; survivor < 8; ++survivor) {
            if (!((GameDataStateStorage.fields_.lock_masks_.enable_mask_ >> 8) & (1 << survivor))) continue;
            ++survivors;
            uint32_t treaty = PlayersData[survivor].diplo_treaties[ConsoleGlobal.faction_id_];
            if (!IsMultiplayerNet && (treaty & 1) && (treaty & 0x2000000)) { --survivors; continue; }
            if (survivor != selected && (PlayersData[survivor].diplo_treaties[selected] & 1)
                && (GameDataStateStorage.fields_.game_rules_ & 0x1000) && partners < 2) ++partners;
        }
        if (faction == ConsoleGlobal.faction_id_) {
            int successor = conqueror < 0 ? 0 : conqueror;
            GenderDefault = Players[successor].noun_gender;
            PluralityDefault = Players[successor].is_noun_plural;
            parse_says(0, Players[successor].noun_faction, -1, -1);
            if (IsMultiplayerNet) {
                NetDaemonNet.hang_up();
                // These are the original individual lock-state words, not
                // a clear of the whole surrounding network allocation.
                NetDaemonNet.primary_state_.leading_words_[0] = 0;
                NetDaemonNet.primary_state_.leading_words_[1] = 0;
                NetDaemonNet.primary_state_.leading_words_[2] = 0;
                NetDaemonNet.primary_state_.flags_ = 0;
                NetDaemonNet.primary_state_.trailing_words_[7] = 0;
                NetDaemonNet.primary_state_.trailing_words_[6] = 0;
                NetDaemonNet.channels_[0].leading_words_[1] = 0;
                NetDaemonNet.channels_[0].leading_words_[0] = 0;
                NetDaemonNet.channels_[0].trailing_words_[0] = 0;
                NetDaemonNet.channels_[1].leading_words_[1] = 0;
                NetDaemonNet.channels_[0].trailing_words_[9] = 0;
                NetDaemonNet.channels_[1].leading_words_[0] = 0;
                NetDaemonNet.channels_[1].trailing_words_[5] = 0;
                NetDaemonNet.channels_[1].trailing_words_[4] = 0;
                NetDaemonNet.primary_state_.trailing_words_[3] = 0;
                NetDaemonNet.primary_state_.trailing_words_[4] = 0;
                NetDaemonNet.primary_state_.trailing_words_[5] = 0;
                NetDaemonNet.channels_[1].trailing_words_[3] = 0;
                NetDaemonNet.channels_[1].trailing_words_[2] = 0;
                NetDaemonNet.channels_[1].leading_words_[3] = 0;
                NetDaemonNet.channels_[1].leading_words_[2] = 0;
                NetDaemonNet.channels_[1].payload_.flags = 0;
                NetDaemonNet.primary_state_.session_phase_ = 0;
                NetDaemonNet.primary_state_.trailing_words_[0] = 0;
                NetDaemonNet.channels_[1].trailing_words_[8] = 0;
                NetDaemonNet.channels_[1].trailing_words_[7] = 0;
                NetDaemonNet.primary_state_.trailing_words_[1] = 0;
                NetDaemonNet.primary_state_.trailing_words_[2] = 0;
                NetDaemonNet.channels_[1].trailing_words_[6] = 255;
                NetDaemonNet.channels_[0].payload_.vehicle_selection_follow = 0;
                for (int lockword = 6; lockword < 14; ++lockword)
                    NetDaemonNet.channels_[0].trailing_words_[lockword - 5] = 0;
                NetDaemonNet.locks_.clear();
                NetDaemonNet.close();
                IsMultiplayerNet = 0;
                ExitTurnLoop = 1;
            }
            X_pop(!conqueror && !Language ? "YOULOSE2" : "YOULOSE", 0);
            if (faction == ConsoleGlobal.faction_id_) {
                GameDataStateStorage.fields_.game_state_ |= 1;
                EndgameExitReason = conqueror ? 7 : 15;
                return 0;
            }
        }
        if (survivors - partners < 2) {
            if (!partners) {
                if (selected == ConsoleGlobal.faction_id_) {
                    popp(ScriptPopupFile, "CONQSING", 0, "conq_sm.pcx", 0);
                    if (!EndgameExitReason) EndgameExitReason = 4;
                }
            } else {
                popp(ScriptPopupFile, "CONQCOOP", 0, "conq_sm.pcx", 0);
                if (!EndgameExitReason) EndgameExitReason = 5;
            }
            GameDataStateStorage.fields_.game_state_ |= 0x2001;
        }
        return 0;
    }
respawn:
    p.se_alloc_labs = 5;
    p.se_alloc_psych = 0;
    if (lifecycle_is_fresh(conqueror) && GameDataStateStorage.fields_.diff_level_current_ > 1) {
        if ((p.ai_wealth || p.ai_tech) && rand() % 4 == 0) p.flags_ext |= 8;
        strcpy(StringTemp, definition.filename);
        CharUpperA(StringTemp);
        if (!strcmp(StringTemp, "ANGELS")) p.flags_ext |= 8;
        if (definition.rule_flags & 0x100) { p.flags_ext |= 4; p.flags |= 0x8000000; }
        if (definition.rule_flags & 0x80) { p.flags_ext |= 0x10; p.flags |= 0x10000000; }
        if (!strcmp(StringTemp, "DRONE")) p.flags_ext = (p.flags_ext & ~8u) | 2;
        if (Continents[region].tile_count < 250 && rand() % 3 == 0) p.flags_ext |= 4;
        if (p.ai_fight >= 0 && rand() % 4 == 0) p.flags_ext |= 0x10;
        if (rand() % 6 == 0) p.flags_ext |= 2;
        if (rand() % 6 == 0) p.flags_ext |= 1;
        if (rand() % 5 == 0) p.flags_ext |= 0x40;
        if (((GameDataStateStorage.fields_.game_rules_ & 0x40) || GameDataStateStorage.fields_.diff_level_current_ > 3)
            && -static_cast<int>((GameDataStateStorage.fields_.game_rules_ & 0x40) != 0) < p.ai_fight
            && rand() % 4 == 0) p.flags_ext |= 0x20;
        if (((GameDataStateStorage.fields_.game_rules_ & 0x40) || GameDataStateStorage.fields_.diff_level_current_ > 3)
            && -static_cast<int>((GameDataStateStorage.fields_.game_rules_ & 0x40) != 0) < p.ai_fight
            && rand() % 4 == 0) {
            p.flags |= 0x1000000;
            if (rand() % 2 == 0) p.flags |= 0x2000000;
        }
        if (p.ai_power || p.ai_fight > 0) {
            if (Continents[region].tile_count > 399 && rand() % 4 == 0) p.flags |= 0x4000000;
            if (rand() % (Continents[region].tile_count > 250 ? 6 : 4) == 0) p.flags |= 0x8000000;
            if (rand() % 4 == 0) p.flags |= 0x10000000;
        }
    }
    for (int offset4 = 0; offset4 < 21; ++offset4) {
        int rx = lifecycle_wrap_x(x + RadiusOffsetX[offset4]);
        int ry = y + RadiusOffsetY[offset4];
        if (rx >= 0 && rx < MapLongitudeBounds && ry >= 0 && ry < MapLatitudeBounds) {
            map_loc(rx, ry)->visibility |= lifecycle_faction_mask(faction);
            synch_bit(rx, ry, faction);
        }
    }
    int scout = 2;
    if (lifecycle_is_fresh(conqueror)) {
        for (int bonus = 0; bonus < definition.faction_bonus_count; ++bonus)
            if (definition.faction_bonus_id[bonus] == 0)
                tech_achieved(faction, definition.faction_bonus_val1[bonus], 0, 0);
        if (ExpansionEnabled && (definition.rule_flags & 0x80)) {
            tech_achieved(faction, 0x4e, 0, 0);
            tech_achieved(faction, 0x4f, 0, 0);
        }
        if (IsMultiplayerNet) {
            int learned = 0;
            for (;;) {
                int research = p.soc_effect_base.values_[SocialEffect::RESEARCH] > 0 ? 1 : p.soc_effect_base.values_[SocialEffect::RESEARCH] < 0 ? -1 : 0;
                int morale = p.soc_effect_base.values_[SocialEffect::MORALE] > 0 ? 1 : p.soc_effect_base.values_[SocialEffect::MORALE] < 0 ? -1 : 0;
                if (learned >= research - morale + 1) break;
                int technology = tech_ai(faction);
                tech_achieved(faction, technology, 0, 0);
                ++learned;
            }
            p.energy_reserves += definition.rule_energy / 2;
        }
        consider_designs(faction);
        for (int local_proto = 0; local_proto < 64; ++local_proto) {
            VehPrototype &prototype = VehPrototypes[faction * 64 + local_proto];
            if (prototype.flags & 1) {
                prototype.flags |= 4;
                if (prototype.plan == 3) scout = faction * 64 + local_proto;
            }
        }
    }
    if (lifecycle_is_fresh(conqueror)) {
        if ((!IsMultiplayerNet && !(GameDataStateStorage.fields_.game_rules_ & 0x10)) || (GameDataStateStorage.fields_.game_rules_ & 0x80)) {
            int base = base_init(faction, x, y);
            strcpy(StringTemp, definition.filename);
            CharUpperA(StringTemp);
            if ((!strcmp(StringTemp, "FUNGBOY") || (definition.rule_flags & 0x80)) && conqueror == -282)
                Bases[base].population_size = 3;
        } else veh_init((definition.rule_flags & 0x100) ? 0x15 : 0, faction, x, y);
        if (!(definition.rule_flags & 0x100)) {
            int vehicle4 = veh_init(scout, faction, x, y);
            Vehs[vehicle4].home_base_id = -1;
            veh_demote(vehicle4);
        }
        if (IsMultiplayerNet && !(definition.rule_flags & 0x100)) {
            int vehicle5 = veh_init(0xb, faction, x, y);
            Vehs[vehicle5].home_base_id = -1;
            veh_demote(vehicle5);
            vehicle5 = veh_init(1, faction, x, y);
            Vehs[vehicle5].home_base_id = -1;
            veh_demote(vehicle5);
        }
        int coast = is_coast(x, y, 0);
        if (coast && (GameDataStateStorage.fields_.diff_level_current_ > 1 || IsMultiplayerNet
            || Continents[region].tile_count < 33)) {
            // Both naval tests reuse the original coast-direction index.
            region = map_loc(x, y)->region;
            if (IsMultiplayerNet
                && Continents[region].tile_count / (bit_count(occupied) + 1) < 76
                && (GameDataStateStorage.fields_.lock_masks_.enable_mask_ & lifecycle_faction_mask(faction))) {
                int vessel = veh_init(0xd, faction, lifecycle_wrap_x(x + RadiusOffsetX[coast]), y + RadiusOffsetY[coast]);
                Vehs[vessel].morale = 2;
                veh_demote(vessel);
                Vehs[vessel].home_base_id = -1;
            }
            if (Continents[region].tile_count / (bit_count(occupied) + 1) < 51) {
                int vessel2 = veh_init(0xd, faction, lifecycle_wrap_x(x + RadiusOffsetX[coast]), y + RadiusOffsetY[coast]);
                Vehs[vessel2].morale = 2;
                veh_demote(vessel2);
                Vehs[vessel2].home_base_id = -1;
            }
        }
    } else {
        veh_init((definition.rule_flags & 0x100) ? 0x15 : 0, faction, x, y);
        int vehicle6 = veh_init((definition.rule_flags & 0x100) ? 4 : 1, faction, x, y);
        veh_demote(vehicle6);
    }
    if (lifecycle_is_fresh(conqueror)) {
        lifecycle_clear_social(p.soc_effect_base);
        for (int bonus2 = 0; bonus2 < definition.faction_bonus_count; ++bonus2) {
            if (definition.faction_bonus_id[bonus2] == 3)
                lifecycle_add_social(p.soc_effect_base, definition.faction_bonus_val1[bonus2], definition.faction_bonus_val2[bonus2]);
        }
        for (int bonus3 = 0; bonus3 < definition.faction_bonus_count; ++bonus3) {
            if (definition.faction_bonus_id[bonus3] != 1) continue;
            int vehicle7 = veh_init(definition.faction_bonus_val1[bonus3], faction, x, y);
            Vehs[vehicle7].home_base_id = -1;
            veh_demote(vehicle7);
            if (!definition.faction_bonus_val1[bonus3]) p.flags |= 0x100;
        }
        if (ExpansionEnabled && (definition.rule_flags & 0x80)) {
            int vehicle8 = veh_init(0x10, faction, x, y);
            Vehs[vehicle8].home_base_id = -1;
            veh_demote(vehicle8);
            vehicle8 = veh_init(0, faction, x, y);
            Vehs[vehicle8].home_base_id = -1;
            veh_demote(vehicle8);
        }
        if (definition.rule_flags & 0x100) {
            if (!(GameDataStateStorage.fields_.lock_masks_.enable_mask_ & lifecycle_faction_mask(faction))) {
                int prototype = conqueror;
                for (int local_proto2 = 0; local_proto2 < 64; ++local_proto2) {
                    int index = faction * 64 + local_proto2;
                    if (!(VehPrototypes[index].flags & 1)) { prototype = index; break; }
                }
                // If no free design exists, the original retains conqueror
                // as the index. No recovery-added guard is inserted.
                make_proto(prototype, 3, 0x11, 0, 0, 1);
                VehPrototypes[prototype].flags = (VehPrototypes[prototype].flags & 0xff3f) | 4;
                name_proto(prototype_name, prototype, faction, 3, 0x11, 0, 0, 1);
                VehPrototypes[prototype].flags &= ~2;
                prototype_name[31] = 0;
                strcpy(VehPrototypes[prototype].veh_name, prototype_name);
            }
            int vehicle9 = veh_init(0x15, faction, x, y);
            Vehs[vehicle9].home_base_id = -1;
            veh_demote(vehicle9);
            vehicle9 = veh_init(0x16, faction, x, y);
            Vehs[vehicle9].home_base_id = -1;
            veh_demote(vehicle9);
        }
    }
    if (GameDataStateStorage.fields_.turn_current_num_ && !failed && conqueror != -282) {
        int defense_value = -1, defense = 2, attack_value = -1, attack = -1;
        int fast_value = -1, fast = -1;
        for (int prototype2 = 0; prototype2 < 512; ++prototype2) {
            VehPrototype &design = VehPrototypes[prototype2];
            if (!veh_avail(prototype2, faction, -1) || !Weapon[design.weapon_id].offense_rating) continue;
            int triad = Chassis[design.chassis_id].triad;
            if ((definition.rule_flags & 0x100) ? triad != 1 : triad != 0) continue;
            int value = armor_val(prototype2, faction) * 8 - weap_val(prototype2, faction);
            if (value > defense_value) { defense_value = value; defense = prototype2; }
            value = Chassis[design.chassis_id].speed + (armor_val(prototype2, faction) + weap_val(prototype2, faction) * 2) * 2;
            if (value > attack_value) { attack_value = value; attack = prototype2; }
            value = Chassis[design.chassis_id].speed * 4 - weap_val(prototype2, faction) - armor_val(prototype2, faction);
            if (value > fast_value && Chassis[design.chassis_id].speed > 1) { fast_value = value; fast = prototype2; }
        }
        int colony = veh_init((definition.rule_flags & 0x100) ? 0x15 : 0, faction, x, y);
        veh_init(defense, faction, x, y);
        if (GameDataStateStorage.fields_.turn_current_num_ > 20 && fast >= 0) veh_init(fast, faction, x, y);
        if (GameDataStateStorage.fields_.turn_current_num_ > 40 && attack >= 0) veh_init(attack, faction, x, y);
        if (GameDataStateStorage.fields_.turn_current_num_ > 60 && defense >= 0) veh_init(defense, faction, x, y);
        if (GameDataStateStorage.fields_.turn_current_num_ > 80 && attack >= 0) veh_init(attack, faction, x, y);
        if (GameDataStateStorage.fields_.turn_current_num_ > 100) veh_init((definition.rule_flags & 0x100) ? 0x15 : 0, faction, x, y);
        veh_promote(colony);
        p.unk_26 = p.tech_ranking * 3 / 4;
    }
    if (faction == ConsoleGlobal.faction_id_) {
        MapWinTable[0]->center_x_ = x;
        MapWinTable[0]->center_y_ = y;
        for (int history = 0; history < 32; ++history) {
            ConsoleGlobal.cursor_history_x_[history] = x;
            ConsoleGlobal.cursor_history_y_[history] = y;
        }
    }
    return 1;
}

// ORIGINAL: 0x005B3380 ?eliminate_player@@YAHHH@Z 0x005B3380-0x005B38C6
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/005b3380.cpp: canonical original identity, full spans and retained seed provenance", "source:src/faction.h: real faction record ownership and typed relationships", "source:recovery/tasks/faction-lifecycle-owner-closure.json: coordinator authorization and retained task baseline"], "reason": "The original faction elimination operation belongs to the Faction owner; typed faction diplomacy, base/vehicle lifecycles and shared endgame/script state preserve actual cross-owner relationships rather than raw executable references."}
// TRIED: direct Ghidra transcription with own extern DAT_ table arrays for every indexed global; diverges at insn #0 (prologue/frame shape of the ~1350-byte body)
int __cdecl eliminate_player(int faction, int conqueror) {
    if (!faction) return 0;
    for (int base = 0; base < GameDataStateStorage.fields_.base_current_count_; ++base) {
        if (Bases[base].faction_id_current == faction) return 0;
    }
    for (int vehicle = GameDataStateStorage.fields_.vehicle_current_count_ - 1; vehicle >= 0; --vehicle) {
        if (vehicle < GameDataStateStorage.fields_.vehicle_current_count_ && Vehs[vehicle].faction_id == faction)
            veh_kill(vehicle);
    }
    PlayersData[faction].current_num_bases = 0;
    int restored = setup_player(faction, conqueror, 0);
    if (!restored) {
        if (conqueror > 0 && !(GameDataStateStorage.fields_.game_state_ & 9))
            ++PlayersData[conqueror].unk_108;
        if (faction == ConsoleGlobal.faction_id_) return 1;
    }
    int successor = conqueror;
    if (successor < 0) {
        successor = 0;
    } else if (successor > 0) {
        set_treaty(faction, successor, 0x2030, 1);
    }
    draw_map(1);
    parse_set(Players[faction].noun_gender, Players[faction].is_noun_plural);
    parse_says(0, Players[faction].noun_faction, -1, -1);
    parse_set(Players[successor].noun_gender, Players[successor].is_noun_plural);
    parse_says(1, Players[successor].noun_faction, -1, -1);
    parse_set(Players[faction].is_leader_female, 0);
    parse_says(2, Players[faction].title_leader, -1, -1);
    parse_set(Players[faction].is_leader_female, 0);
    parse_says(3, Players[faction].name_leader, -1, -1);
    parse_says(4, get_his_her(faction, 0), -1, -1);
    StringTemp[0] = '\0';
    strcat(StringTemp, "WIPEOUT");
    if (!successor) {
        char number[80];
        _itoa(2, number, 10);
        strcat(StringTemp, number);
    } else {
        if (successor == ConsoleGlobal.faction_id_) strcat(StringTemp, "YOU");
        char number[80];
        _itoa(restored != 0, number, 10);
        strcat(StringTemp, number);
    }
    char portrait[80];
    portrait[0] = '\0';
    strcat(portrait, Players[faction].filename);
    strcat(portrait, "2.PCX");
    if (Players[faction].rule_flags & 0x80) {
        popp(ScriptPopupFile, StringTemp, 0, restored ? "al_pod_sm.pcx" : portrait, 0);
    } else {
        popp(ScriptPopupFile, StringTemp, 0, restored ? "escpod_sm.pcx" : portrait, 0);
    }
    if (!restored) mon_killed_faction(successor, faction);
    if (!IsMultiplayerNet && (GameDataStateStorage.fields_.game_preferences_ & 0x10000000)
        && successor == ConsoleGlobal.faction_id_ && !restored) {
        amovie_project(Players[faction].filename);
    } else if (restored && !IsMultiplayerNet && successor == ConsoleGlobal.faction_id_) {
        int chance = (PlayersData[successor].diplo_treaties[faction] & 0x1000) ? 2 : 4;
        if (!bounded_crt_random(chance)) {
            int first = veh_find(0, 0, faction, -1);
            if (first >= 0) {
                int x;
                int y;
                for (int offset = 0; offset < 9; ++offset) {
                    x = wrap_map_x(Vehs[first].x + RadiusOffsetX[offset]);
                    y = Vehs[first].y + RadiusOffsetY[offset];
                    if (on_map(x, y)) {
                        spot_loc(x, y, successor);
                    }
                }
                int vehicle = first;
                int marked = 0;
                while (Vehs[vehicle].prev_veh_id_stack >= 0)
                    vehicle = Vehs[vehicle].prev_veh_id_stack;
                for (; vehicle >= 0; vehicle = Vehs[vehicle].next_veh_id_stack) {
                    Vehs[vehicle].visibility = 0;
                    if (VehPrototypes[Vehs[vehicle].proto_id].plan == 8 && !marked) {
                        marked = 1;
                        Vehs[vehicle].visibility = (uint8_t)(1 << successor);
                    }
                }
                // The original deliberately retains the last radius coordinates.
                draw_tiles(x, y, 2);
                MapWinTable[0]->set_center(x, y, 1);
                parse_says(6, Players[faction].adj_name_faction, -1, -1);
                if (Players[faction].rule_flags & 0x80) {
                    popp(ScriptPopupFile, "TRACKED", 0, "al_pod_sm.pcx", 0);
                } else {
                    popp(ScriptPopupFile, "TRACKED", 0, "escpod_sm.pcx", 0);
                }
            }
        }
    }
    if ((Players[faction].rule_flags & 0x80) && (Players[ConsoleGlobal.faction_id_].rule_flags & 0x80)
        && faction != ConsoleGlobal.faction_id_) interlude(0x2a, 0, 1, 0);
    return 1;
}

/*
Purpose: Clear and reset scenario game variables.
// ORIGINAL: 0x005B38D0 ?clear_scenario@@YAXXZ 0x005B38D0-0x005B3919 BYTE_EXACT
// size      73 bytes
// prototype
// callers   1   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void __cdecl clear_scenario() {
    ScenarioSave.objective_req_victory = 9999;
    ScenarioSave.objectives_sudden_death_victory = 9999;
    ScenarioSave.objective_achieve_points = 0;
    ScenarioSave.victory_achieve_bonus_points = 0;
    ScenarioSave.starting_mission_year = Rules.normal_starting_year;
    ScenarioSave.ending_mission_year = GameDataStateStorage.fields_.diff_level_current_ < DLVL_LIBRARIAN
        ? Rules.normal_end_year_low_three_diff : Rules.normal_end_year_high_three_diff;
}

extern int __cdecl tech_advance(int faction_id);
extern uint32_t TerraformAnnouncedMask[2];
extern int TutorialFlags;
extern int TutorialFlags2;

// ORIGINAL: 0x005B3920 ?setup_game@@YAXH@Z 0x005B3920-0x005B4201
// RECOVERY_BINDING: v1 {"evidence": ["Retained setup owner full native seed and independent full-body review; corrected shared social-effect seen state and pre-callback player labels; actual-source setup controls; no historical byte claim copied."], "reason": "Recover complete game setup using actual source-owned reset storage, player construction and diplomacy initialization; preserve native callback order and final time-warp versus balance branch."}
// Rewritten from the image (2026-11-08, 0.76 -> 0.98): the beginner-prefs bit
// lives in AlphaIniPrefs.preferences, agenda_on takes (outer, inner), each
// faction's ideology is reset to -1/0/-1 before the draw, and the effect scan
// indexes values_ directly. Left: the social loop's i/rejected register choice
// and the image's separate (unpooled) "BELIEVE"/"DRONE"/"HIVE" literals.
void __cdecl setup_game(int reload) {
    GameDataStateStorage.fields_.legacy_game_save_name_[0] = 0;

    // The image passes zero-filled 0x945E78/0x945E7C strings to these keys.
    // Pinned-image initial bytes are NUL, and no other absolute references to
    // either cell exist; the existing prefs_put overload consumes the values.
    prefs_put("Latest Save", "");
    prefs_put("Latest Scenario", "");
    if (GameDataStateStorage.fields_.diff_level_current_ == 0 && reload == 0 &&
        prefs_get("DontResetBeginnerPrefs", 0, 0) == 0) {
        prefs_load(1);
        AlphaIniPrefs.preferences |= 0x20;
        prefs_put("DontResetBeginnerPrefs", 0, 0);
    }
    if (IsMultiplayerNet) AlphaIniPrefs.preferences &= ~0x20u;
    prefs_use();

    ScenarioSave.objective_achieve_points = 0;
    ScenarioSave.objective_req_victory = 9999;
    ScenarioSave.objectives_sudden_death_victory = 9999;
    ScenarioSave.victory_achieve_bonus_points = 0;
    ScenarioSave.starting_mission_year = Rules.normal_starting_year;
    ScenarioSave.ending_mission_year = GameDataStateStorage.fields_.diff_level_current_ < 3
        ? Rules.normal_end_year_low_three_diff
        : Rules.normal_end_year_high_three_diff;

    clear_monuments();
    wipe_undo();
    ReplayEventBytes = 0;
    GameDataStateStorage.fields_.game_state_ &= 0x01800000u;
    GameDataStateStorage.fields_.game_interlude_flags_ = 0;
    GameDataStateStorage.fields_.tutorial_flags_ = 0;
    GameDataStateStorage.fields_.tutorial_flags_2_ = 0;
    GameDataStateStorage.fields_.terraform_announced_mask_[0] = 0;
    GameDataStateStorage.fields_.terraform_announced_mask_[1] = 0;
    GameDataStateStorage.fields_.tutwin_flags_9a64bc_ = 0;
    GameDataStateStorage.fields_.turn_current_num_ = 0;
    GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_ = 0xFF;
    GameDataStateStorage.fields_.base_current_count_ = 0;
    GameDataStateStorage.fields_.game_setup_reset_word_ = 0;
    clear_all_offers();
    clear_hotseat_chat();
    clear_council_notify();
    clear_tamper();
    GameDataStateStorage.fields_.turn_control_duration_milliseconds_ = 0;
    GameDataStateStorage.fields_.governor_faction_ = -1;
    for (int proposal = 0; proposal < MaxProposalNum; ++proposal) {
        GameDataStateStorage.fields_.council_resolution_count_[proposal] = 0;
        GameDataStateStorage.fields_.council_last_resolution_turn_[proposal] = -9999;
    }
    GameDataStateStorage.fields_.climate_rise_step_ = 0;
    GameDataStateStorage.fields_.climate_rise_limit_ = 0;
    GameDataStateStorage.fields_.climate_rise_accum_ = 0;
    GameDataStateStorage.fields_.climate_sea_level_delta_ = 0;

    if (!IsMultiplayerNet && reload != 0 && GameRestartQueued == 0)
        GameDataStateStorage.fields_.game_state_ |= 0xE0;

    memset(GameDataStateStorage.fields_.game_tech_achieved_, 0,
           sizeof GameDataStateStorage.fields_.game_tech_achieved_);
    memset(&GameDataStateStorage.fields_.secret_project_, 0xFF, sizeof GameDataStateStorage.fields_.secret_project_);
    memset(ReplayEvents, 0, sizeof ReplayEvents);
    memset(FactionRankingHistory, 0, sizeof FactionRankingHistory);
    // One block clear of the ten event words sunspot_duration_ through
    // ecology_event_accumulator_ (+0x370..+0x394), as the image's rep stosd.
    memset(&GameDataStateStorage.fields_.sunspot_duration_, 0,
           offsetof(GameDataStateWindowFields, ecology_event_accumulator_) + sizeof(int)
           - offsetof(GameDataStateWindowFields, sunspot_duration_));

    if (GameDataStateStorage.fields_.game_state_ & 0x01000000u) {
        int seen_social_models[12];
        memset(seen_social_models, 0, sizeof seen_social_models);
        for (int social_faction = 1; social_faction < MaxPlayerNum; social_faction++) {
            int rejected = 0;
            int category;
            int model;
            Players[social_faction].soc_ideology_category = -1;
            Players[social_faction].soc_ideology_model = 0;
            Players[social_faction].soc_ideology_effect = -1;
            do {
                category = rand() % 3;
                model = rand() % 3 + 1;
                if (!seen_social_models[category * 3 + model]) break;
            } while (++rejected < social_faction + 1);
            if (rejected <= social_faction + 1) {
                Players[social_faction].soc_ideology_category = category;
                Players[social_faction].soc_ideology_model = model;
                seen_social_models[category * 3 + model] = 1;
                for (int effect = 0; effect < 11; effect++) {
                    if (SocialCategories[category].model_effect[model].values_[effect] > 0) {
                        Players[social_faction].soc_ideology_effect = effect;
                        break;
                    }
                }
            }
        }
    }

    int setup_results[MaxPlayerNum];
    char first_key[24];
    char second_key[24];
    for (int human_faction = 0; human_faction < MaxPlayerNum; human_faction++) {
        if (is_human(human_faction)) {
            setup_player(human_faction, -1 - reload, 0);
            setup_results[human_faction] = 0;
        }
    }
    memset(TectonicDetonationCount, 0, sizeof TectonicDetonationCount);
    for (int computer_faction = 0; computer_faction < MaxPlayerNum; computer_faction++) {
        if (!is_human(computer_faction)) {
            setup_player(computer_faction, -1 - reload, 0);
            setup_results[computer_faction] = 0;
        }
    }

    // This is the row-major 8x8 pending-diplomacy flag matrix. Its native
    // reset follows both setup_player passes and precedes the label copies.
    memset(PendingDiplomacyFlags, 0, sizeof PendingDiplomacyFlags);

    // The original publishes the default human's labels before the optional
    // new-game diplomacy/time-warp branch, whose callbacks observe them.
    strcpy(Players[0].formal_name_faction, StringTable.get(Labels.strings_ptr[245]));
    strcpy(Players[0].noun_faction, StringTable.get(Labels.strings_ptr[245]));
    strcpy(Players[0].adj_name_faction, StringTable.get(Labels.strings_ptr[244]));

    if (reload == 0) {
        // Pair every unpaired social_faction with a distinct random partner.
        for (int pair_faction = 1; pair_faction < MaxPlayerNum; pair_faction++) {
            if (setup_results[pair_faction] == 0) {
                int tries = 0;
                int other;
                do {
                    do {
                        other = rand() % 7 + 1;
                        tries++;
                    } while (other == pair_faction);
                } while ((setup_results[other] != 0 &&
                          (tries < 200 ||
                           (tries < 400 && !(Players[other].rule_flags & 0x1000)))) ||
                         (((uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_ & (1 << pair_faction)) &&
                          ((uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_ & (1 << other)) &&
                          tries < 100));
                setup_results[pair_faction]++;
                setup_results[other]++;
            }
        }

        // Factions with rule flag 0x1000 ask for a second pair.
        for (int extra_faction = 1; extra_faction < MaxPlayerNum; extra_faction++) {
            if (setup_results[extra_faction] <= 1 && (Players[extra_faction].rule_flags & 0x1000)) {
                int tries = 0;
                int other;
                do {
                    do {
                        other = rand() % 7 + 1;
                    } while (other == extra_faction);
                } while (((uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_ & (1 << extra_faction)) &&
                         ((uint8_t)GameDataStateStorage.fields_.lock_masks_.enable_mask_ & (1 << other)) &&
                         ++tries < 200);
                setup_results[extra_faction]++;
                setup_results[other]++;
            }
        }

        if (!IsMultiplayerNet || (GameDataStateStorage.fields_.game_rules_ & 0x40)) {
            if (!(GameDataStateStorage.fields_.game_state_ & 0x00800000u)) {
                for (int first = 1; first < MaxPlayerNum; first++) {
                    for (int second = 1; second < MaxPlayerNum; second++) {
                        if (first != second) {
                            first_key[0] = 0;
                            second_key[0] = 0;
                            strcat(first_key, Players[first].search_key);
                            strcat(second_key, Players[second].search_key);
                            CharUpperA(first_key);
                            CharUpperA(second_key);
                            if ((!strcmp(first_key, "PEACE") && !strcmp(second_key, "SPARTANS"))
                                || (!strcmp(first_key, "GAIANS") && !strcmp(second_key, "MORGAN"))
                                || (!strcmp(first_key, "HIVE") && !strcmp(second_key, "BELIEVE"))
                                || (!strcmp(first_key, "UNIV") && !strcmp(second_key, "BELIEVE"))
                                || (!strcmp(first_key, "FUNGBOY") && !strcmp(second_key, "BELIEVE"))
                                || (!strcmp(first_key, "PEACE") && !strcmp(second_key, "PIRATES"))
                                || (!strcmp(first_key, "DRONE") && !strcmp(second_key, "PIRATES"))
                                || (!strcmp(first_key, "DRONE") && !strcmp(second_key, "MORGAN"))
                                || (!strcmp(first_key, "DRONE") && !strcmp(second_key, "HIVE"))
                                || (!strcmp(first_key, "ANGELS") && !strcmp(second_key, "HIVE"))
                                || (!strcmp(first_key, "ANGELS") && !strcmp(second_key, "CYBORG"))
                                || (Players[first].rule_flags & RFLAG_ALIEN)
                                || (Players[second].rule_flags & RFLAG_ALIEN)) {
                                agenda_on(first, second, 0x200);
                            }
                            if ((Players[first].rule_flags & RFLAG_ALIEN)
                                && (Players[second].rule_flags & RFLAG_ALIEN)) {
                                set_treaty(second, first, 0xC80C0C38, 1);
                                set_agenda(second, first, 0x2C2D, 1);
                            }
                        }
                    }
                }
            } else {
                for (int network_faction = 1; network_faction < MaxPlayerNum; network_faction++) {
                    if (GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & (1 << network_faction)) {
                        for (int assigned = 0; assigned < (int)((GameDataStateStorage.fields_.game_rules_ >> 6) & 1)
                                 + (PlayersData[network_faction].diff_level >= 3); assigned++) {
                            int tries = 0;
                            int other;
                            do {
                                other = rand() % 7 + 1;
                            } while (++tries < 100
                                     && ((PlayersData[other].diplo_agenda[network_faction] & 0x200)
                                         || !is_alive(other) || is_human(other)));
                            if (tries < 100) agenda_on(other, network_faction, 0x200);
                        }
                    }
                }
            }
        }

        if (GameDataStateStorage.fields_.game_rules_ & 0x80) {
            time_warp();
            return;
        }
        balance();
    }
}

/*
Purpose: Calculate the social engineering effect modifiers for the specified faction.
// ORIGINAL: 0x005B4210 ?social_calc@@YAXPAHPAHHHH@Z 0x005B4210-0x005B44C9 BYTE_EXACT
// TRIED: hoisting `base_project(SP_NETWORK_BACKBONE)`/`base_project(
//            SP_CLONING_VATS)` into locals ahead of the loop, keeping the
//            per-branch `Bases[base_id].faction_id_current == faction_id`
//            checks in place (the image loads both project base ids ONCE
//            via `mov ecx,[0x9a6574]`/`mov esi,[0x9a6568]` before the cat/eff
//            loop, then reuses them at both the SE_THOUGHT_CONTROL and
//            SOCIAL_CAT_VALUES/SE_POWER sites, but re-checks Bases[]/
//            faction_id_current locally each time) - agreeing dropped 4->3,
//            no improvement. `store_order` shows this tree spills 24 locals
//            to the stack against the image's 16, so every jump target
//            downstream of instruction ~10 is offset and reads as a
//            mismatch even where the branch shape matches - a stack-layout/
//            register-pressure divergence, not a single fixable bug found so
//            far. Plateaus at 10/244 agreeing (0.155 similar); not chased
//            further given the size of the function.
// symbol    ?social_calc@@YAXPAUSocialCategory@@PAUSocialEffect@@HHH@Z
// size      697 bytes
// prototype void (__cdecl ?social_calc@@YAXPAHPAHHHH@Z)(social_category* category, social_effect* effect, uint32_t factionID, BOOL toggle, BOOL isQuickCalc)
// callers   7   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005BF130
Return Value: n/a
Status: Complete
*/
void __cdecl social_calc(SocialCategory *category, SocialEffect *effect, int faction_id,
    BOOL UNUSED(toggle), BOOL is_quick_calc) {
    ZeroMemory(effect, sizeof(SocialEffect));
    // One counter serves the category walk and the final bonus walk (shared frame slot).
    int cat;
    for (cat = 0; cat < MaxSocialCatNum; cat++) {
        int model = category->models_[cat];
        for (int eff = 0; eff < MaxSocialEffectNum; eff++) {
            if (SocialCategories[cat].model_effect[model].values_[eff] < 0) {
                if (cat == SOCIAL_CAT_FUTURE && model == SE_CYBERNETIC) {
                    if (has_project(SP_NETWORK_BACKBONE, faction_id)) {
                        continue;
                    }
                } else if ((cat == SOCIAL_CAT_VALUES && model == SE_POWER)
                    || (cat == SOCIAL_CAT_FUTURE && model == SE_THOUGHT_CONTROL)) {
                    if (has_project(SP_CLONING_VATS, faction_id)) {
                        continue;
                    }
                }
                for (int i = 0; i < Players[faction_id].faction_bonus_count; i++) {
                    if (Players[faction_id].faction_bonus_val1[i] == cat
                        && Players[faction_id].faction_bonus_val2[i] == model) {
                        if (Players[faction_id].faction_bonus_id[i] == RULE_IMPUNITY) {
                            // negates neg effects
                            effect->values_[eff] -= SocialCategories[cat].model_effect[model].values_[eff];
                        } else if (Players[faction_id].faction_bonus_id[i] == RULE_PENALTY) {
                            // doubles neg effects
                            effect->values_[eff] += SocialCategories[cat].model_effect[model].values_[eff];
                        }
                    }
                }
            }
            effect->values_[eff] += SocialCategories[cat].model_effect[model].values_[eff];
        }
    }
    if (is_quick_calc) {
        return;
    }
    if (has_project(SP_ASCETIC_VIRTUES, faction_id)) {
        effect->values_[SocialEffect::POLICE]++;
    }
    if (has_project(SP_LIVING_REFINERY, faction_id)) {
        effect->values_[SocialEffect::SUPPORT] += 2;
    }
    if (has_temple(faction_id)) {
        effect->values_[SocialEffect::PLANET]++;
        if (Players[faction_id].rule_flags & RFLAG_ALIEN) {
            effect->values_[SocialEffect::RESEARCH]++; // bonus documented in conceptsx.txt but not manual
        }
    }
    for (int eff = 0; eff < MaxSocialEffectNum; eff++) {
        effect->values_[eff] += PlayersData[faction_id].soc_effect_base.values_[eff];
    }
    for (cat = 0; cat < Players[faction_id].faction_bonus_count; cat++) {
        switch (Players[faction_id].faction_bonus_id[cat]) {
        case RULE_IMMUNITY: // cancels neg effects
            effect->values_[Players[faction_id].faction_bonus_val1[cat]]
                = range(effect->values_[Players[faction_id].faction_bonus_val1[cat]], 0, 999);
            break;
        case RULE_ROBUST: // halves neg effects
            if (effect->values_[Players[faction_id].faction_bonus_val1[cat]] < 0) {
                effect->values_[Players[faction_id].faction_bonus_val1[cat]] /= 2;
            }
            break;
        }
    }
}

/*
Purpose: Handle the social engineering turn upkeep for the specified faction.
// ORIGINAL: 0x005B44D0 ?social_upkeep@@YAXH@Z 0x005B44D0-0x005B4550 BYTE_EXACT
// size      128 bytes
// prototype void (__cdecl ?social_upkeep@@YAXH@Z)(int factionID)
// callers   2   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005B4210
Return Value: n/a
Status: Complete
*/
void __cdecl social_upkeep(int faction_id) {
    for (int i = 0; i < MaxSocialCatNum; i++) {
        PlayersData[faction_id].soc_category_active.models_[i] =
            PlayersData[faction_id].soc_category_pending.models_[i];
    }
    SocialCategory *soc_cat_pen = &PlayersData[faction_id].soc_category_pending;
    social_calc(soc_cat_pen, &PlayersData[faction_id].soc_effect_pending, faction_id, false, false);
    social_calc(soc_cat_pen, &PlayersData[faction_id].soc_effect_active, faction_id, false, false);
    social_calc(soc_cat_pen, &PlayersData[faction_id].soc_effect_temp, faction_id, true, false);
    PlayersData[faction_id].soc_upheaval_cost_paid = 0;
}

/*
Purpose: Calculate the cost of the social upheaval for the specified faction.
// ORIGINAL: 0x005B4550 ?social_upheaval@@YAHHPAH@Z 0x005B4550-0x005B45F8 BYTE_EXACT
// body      src/setup.cpp
// symbol    ?social_upheaval@@YAHHPAUSocialCategory@@@Z
// size      168 bytes
// prototype int (__cdecl ?social_upheaval@@YAHHPAH@Z)(int factionID, social_category* category)
// callers   2   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Social upheaval cost
Status: Complete
*/
// Historical inline rationale: social_ai's own `calls` list (0x004EF090
// 0x00539D40 0x00581260 0x005B4210 0x005B4600 0x005B9F20 0x005FCB20) has no
// 0x005B4550 in it, so its two call sites are inlined in the image even though
// this address is also a real out-of-line body.
MEASURED int __cdecl social_upheaval(int faction_id, SocialCategory *category_new) {
    int change_count = 0;
    for (int i = 0; i < MaxSocialCatNum; i++) {
        change_count += category_new->models_[i] !=
            PlayersData[faction_id].soc_category_active.models_[i];
    }
    if (!change_count) {
        return 0;
    }
    change_count++;
    int diff_lvl = is_human(faction_id) ? PlayersData[faction_id].diff_level : DLVL_LIBRARIAN;
    int cost = change_count * change_count * change_count * diff_lvl;
    if (Players[faction_id].rule_flags & RFLAG_ALIEN) {
        cost += cost / 2;
    }
    return cost;
}

// ORIGINAL: 0x005B4600 ?social_set@@YAXH@Z 0x005B4600-0x005B4721 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence":["Pinned native instructions at 0x005B4600..0x005B4721: pre/post cost factors, pending social calculation, owned-base accumulator scaling and active-window redraw.","source:src/faction.h PlayerData social members; source:src/base.h Base accumulator members."],"reason":"Home social adoption in the faction owner with the original signed-int API and typed pending effect/base accumulators; preserve the FILE seed identity and avoid a forwarder or exactness claim."}
void __cdecl social_set(int faction_id) {
    const int nutrient_cost_before = cost_factor(faction_id, 0, -1);
    const int mineral_cost_before = cost_factor(faction_id, 1, -1);

    social_calc(&PlayersData[faction_id].soc_category_pending,
                &PlayersData[faction_id].soc_effect_pending,
                faction_id, 0, 0);

    const int nutrient_cost_after = cost_factor(faction_id, 0, -1);
    const int mineral_cost_after = cost_factor(faction_id, 1, -1);

    for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; ++base_id) {
        Base &base = Bases[base_id];
        if (base.faction_id_current != faction_id) {
            continue;
        }

        if (nutrient_cost_after != nutrient_cost_before) {
            base.nutrients_accumulated =
                (nutrient_cost_before / 2 +
                 base.nutrients_accumulated * nutrient_cost_after) /
                nutrient_cost_before;
        }
        if (mineral_cost_after != mineral_cost_before) {
            base.minerals_accumulated =
                (mineral_cost_before / 2 +
                 base.minerals_accumulated * mineral_cost_after) /
                mineral_cost_before;
            base.minerals_accumulated_2 = base.minerals_accumulated;
        }

        if (IsMultiplayerNet == 0 &&
            (GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & (1u << faction_id)) != 0) {
            set_base(base_id);
            base_compute(1);
        }
    }

    if (faction_id == ConsoleGlobal.faction_id_) {
        TutWinBaseWindow.redraw();
    }
}

/*
Purpose: Check to see whether the faction can utilize a specific social category and model.
// ORIGINAL: 0x005B4730 ?society_avail@@YAHHHH@Z 0x005B4730-0x005B4786 BYTE_EXACT
// size      86 bytes
// prototype int (__cdecl ?society_avail@@YAHHHH@Z)(int socCategory, int socModel, int factionID)
// callers   5   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005B9F20
// LEVER (2026-09-03): `has_tech_call` on the `return`. 0 of 36 agreeing,
//   53 instructions against the image's 36, all of it technology.h's
//   MEASURED inline open-coded into a six-line body. With the forwarder the
//   body is the image's own 36 and the winning set flips to `/O2 /Oy- /GX`,
//   which is what produces the image's frame. 36 of 36, BYTE_EXACT. Same
//   shape as see_map_check 0x005A96D0.
Return Value: Is social category/model available to faction? true/false
Status: Complete
*/
BOOL __cdecl society_avail(int soc_category, int soc_model, int faction_id) {
    if (Players[faction_id].soc_anti_ideology_category == soc_category
        && Players[faction_id].soc_anti_ideology_model == soc_model) {
        return false;
    }
    // has_tech_call, not has_tech: the image CALLS 0x005B9F20 once from here
    // (0x005B477B) and technology.h's MEASURED inline was open-coded into
    // this body.
    return has_tech(SocialCategories[soc_category].preq_tech[soc_model], faction_id);
}

/*
Purpose: Calculate an AI faction's social engineering.
// ORIGINAL: 0x005B4790 ?social_ai@@YAXHHHHHPAH@Z 0x005B4790-0x005B5612
// LEVER (2026-09-03): `has_tech_call` at the loop site (0x005B4AA1 in the
//        image's own calls list) - 22/1353 -> 50/1353. Still MISMATCH, and
//        call_diff still says MORE (9 vs 7), for two reasons this file cannot
//        reach: do_all_non_input is MEASURED inline in temp.h, so the image's
//        single `call 0x005FCB20` arrives here as its do_non_input/do_net/
//        check_net expansion - the same ceiling the brief records on
//        Path::continent - and the social_calc call both sides make resolves
//        to DIFFERENT SYMBOLS (the image's `?social_calc@@YAXPAHPAHHHH@Z`
//        spells both pointers `PAH`/char *, this tree's spells the class
//        pointers), so call_diff pairs neither. Measured and rejected:
//        `#pragma auto_inline(off)` around social_calc's definition changes
//        nothing at all (compiled stays 1308 instructions, same call set) -
//        VC6 was never folding it into social_ai.
// LEVER: call_diff showed 9 vs the image's 7 - this body's two calls to
//        social_upheaval() are not in the image's own `calls` list below
//        (no 0x005B4550), so both are inlined there even though 0x005B4550
//        also exists as a real out-of-line body. Moved social_upheaval to
//        faction.h as `MEASURED inline` (see its ORIGINAL marker, which now
//        points there) per the "helper the image inlines" lever.
//        TRIED: `MEASURED __forceinline` on the same definition - call
//        count stayed 9 vs 7 under every flag set `call_diff` tries, so this
//        tree's O2 will not fold social_upheaval's 4-iteration loop in
//        either spelling; reverted to plain `MEASURED inline` since it did
//        not help. This is a 1353-instruction function ("likely has multiple
//        issues due to length + complexity" per its own status line);
//        MISMATCH at 24/1353 agreeing, left for a dedicated pass rather than
//        chased further here.
// symbol    ?social_ai@@YAXHHHHHPAUSocialCategory@@@Z
// size      3714 bytes
// prototype void (__cdecl ?social_ai@@YAXHHHHHPAH@Z)(int factionID, int growthVal, int techVal, int wealthVal, int powerVal, social_category* output)
// callers   2   call targets   7
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004EF090 0x00539D40 0x00581260 0x005B4210 0x005B4600 0x005B9F20 0x005FCB20
Return Value: n/a
Status: Complete - testing (likely has multiple issues due to length + complexity)
*/
void __cdecl social_ai(int faction_id, int growth_val, int tech_val, int wealth_val,
                       int power_val, SocialCategory *output) {
    // setup
    int fight_val;
    int unk_val1 = -1;
    if (!output) {
        if (is_human(faction_id)) {
            return;
        } else {
            growth_val = PlayersData[faction_id].ai_growth;
            tech_val = PlayersData[faction_id].ai_tech;
            wealth_val = PlayersData[faction_id].ai_wealth;
            power_val = PlayersData[faction_id].ai_power;
            fight_val = PlayersData[faction_id].ai_fight;
        }
    } else if (growth_val < 0) {
        return;
    } else if (growth_val > 100) {
        unk_val1 = growth_val - 100;
        growth_val = PlayersData[faction_id].ai_growth;
        tech_val = PlayersData[faction_id].ai_tech;
        wealth_val = PlayersData[faction_id].ai_wealth;
        power_val = PlayersData[faction_id].ai_power;
        fight_val = PlayersData[faction_id].ai_fight;
    } else {
        if (!power_val) {
            fight_val = -(wealth_val != 0);
        } else {
            fight_val = 1;
        }
    }
    // Four choices for each of four categories: original 5B4831..5B483F.
    uint32_t unk_val2 = 4;
    for (int i = 3; i > 0; i--) {
        unk_val2 *= 4;
    }
    // future pop growth
    int pop_goal_growth = 0;
    for (i = 0; i < GameDataStateStorage.fields_.base_current_count_; i++) {
        if (Bases[i].faction_id_current == faction_id) {
            pop_goal_growth += pop_goal(i) - Bases[i].population_size;
        }
    }
    // plan and enemy region metrics
    int unk_sum1 = 0;
    uint32_t unk_count1 = 0;
    uint32_t unk_count2 = 0;
    uint32_t unk_sum2 = 0;
    for (uint32_t region = 1; region < MaxRegionLandNum; region++) {
        if (!bad_reg(region)) {
            uint8_t base_count = PlayersData[faction_id].region_total_bases[region];
            if (base_count) {
                uint8_t plan = PlayersData[faction_id].region_base_plan[region];
                if (plan == PLAN_DEFENSIVE || PlayersData[faction_id].unk_77[region] & 0x400) {
                    unk_sum1 += PlayersData[faction_id].region_total_combat_vehs[region] * 2;
                } else if (plan == PLAN_OFFENSIVE) {
                    unk_sum1 += PlayersData[faction_id].region_total_combat_vehs[region];
                    unk_count2++;
                } else if (plan != PLAN_COLONIZATION) {
                    unk_count1++;
                }
                if (plan <= PLAN_DEFENSIVE) {
                    for (uint32_t f = 1; f < MaxPlayerNum; f++) {
                        if (f != faction_id) {
                            if (has_treaty(faction_id, f, DTREATY_VENDETTA | DTREATY_WANT_REVENGE)
                                && PlayersData[f].enemy_best_psi_offense
                                >= PlayersData[f].enemy_best_weapon_value
                                && PlayersData[f].proto_id_active[BSC_MIND_WORMS] > 1) {
                                unk_sum2 += base_count; // not used
                            }
                        }
                    }
                }
            }
        }
    }
    if (output && power_val && unk_sum1 < 1) {
        unk_sum1 = 1;
    }
    // tech ranking
    int unk_val3 = 1;
    if (GameDataStateStorage.fields_.game_state_ & 0x200 // set in rankings(), related to intense riv + end game
        && PlayersData[faction_id].ranking < 6) {
        int tech_rank_diff = PlayersData[GameDataStateStorage.fields_.ranking_faction_id_unk1_].tech_ranking / 2
            - PlayersData[faction_id].tech_ranking / 2;
        if (tech_rank_diff > 5) {
            unk_val3 = 2;
        }
        if (tech_rank_diff > 10) {
            unk_val3++;
        }
    }
    // search / calculate
    int unk_val4 = -9999; // search val?
    int soc_cat_bits = -1;
    // unk_sum2 = 0 > used as iterator, optimization re-use, var unused
    for (uint32_t player = 0; player < unk_val2; player++) {
        SocialCategory soc_cat;
        SocialEffect soc_eff;
        int k = player;
        bool invalid_combination = false;
        for (int j = 0; j < MaxSocialCatNum; j++) {
            int model = k & 3;
            soc_cat.models_[j] = model;
            if (model) {
                if ((Players[faction_id].soc_anti_ideology_category == j
                    && Players[faction_id].soc_anti_ideology_model == model)
                    // has_tech_call: the image CALLS 0x005B9F20 once here
                    // (0x005B4AA1, see the calls line) instead of open-coding
                    // the preq walk inside this loop.
                    || !has_tech(SocialCategories[j].preq_tech[model], faction_id)) {
                    invalid_combination = true;
                    break;
                }
            }
            k >>= 2;
        }
        // Original 5B4A90/5B4AAB skip directly to the outer increment.
        if (invalid_combination) continue;
        do_all_non_input();
        social_calc(&soc_cat, &soc_eff, faction_id, false, false);
        int unk_val6 = 0;
        if (unk_val1 >= 0) {
            unk_val6 = *(&soc_eff.values_[SocialEffect::ECONOMY] + unk_val1) * 1000;
        }
        BOOL has_ideology = false;
        int ideology_cat = Players[faction_id].soc_ideology_category;
        if (ideology_cat >= 0) {
            int ideology_mod = Players[faction_id].soc_ideology_model;
            if (ideology_mod) {
                int unk_val7 = soc_cat.models_[ideology_cat];
                if (!unk_val7 && unk_val7 == ideology_mod) {
                    unk_val6 += PlayersData[faction_id].current_num_bases;
                } else {
                    has_ideology = true;
                }
            }
        }
        int ideology_eff = Players[faction_id].soc_anti_ideology_effect;
        if (ideology_eff >= 0) {
            int unk_val8 = *(&soc_eff.values_[SocialEffect::ECONOMY] + ideology_eff);
            if (unk_val8 > 0) {
                unk_val6 += PlayersData[faction_id].current_num_bases;
            }
            if (unk_val8 < 0) {
                has_ideology = true;
            }
        }
        if (!has_ideology) {
            // economy
            int effic_val = PlayersData[faction_id].unk_46[range(4 - soc_eff.values_[SocialEffect::EFFICIENCY], 0, 8)];
            int econ_val = soc_eff.values_[SocialEffect::ECONOMY];
            int econ_weight = 0;
            if (econ_val < 2) {
                if (econ_val <= 0) {
                    if (econ_val < -1) {
                        for (int region = 0; region < MaxContinentNum; region++) {
                            econ_weight -= (soc_eff.values_[SocialEffect::ECONOMY] + 1)
                                * PlayersData[faction_id].region_total_bases[region]
                                * ((PlayersData[faction_id].region_base_plan[region]
                                    != PLAN_DEFENSIVE) + 1);
                        }
                        econ_weight /= (unk_sum1 + 1);
                    } else {
                        econ_weight = -1;
                    }
                } else {
                    for (int region = 0; region < MaxContinentNum; region++) {
                        econ_weight += PlayersData[faction_id].region_total_bases[region]
                            * ((PlayersData[faction_id].region_base_plan[region] != PLAN_DEFENSIVE)
                                + 1);
                    }
                    econ_weight /= (unk_sum1 + 1);
                }
            } else {
                if (econ_val > 4) {
                    econ_val = 4;
                }
                econ_weight = (PlayersData[faction_id].unk_47 + econ_val * 2 - 4) / (unk_sum1 + 1);
            }
            for (; econ_weight > 0; econ_weight--, effic_val--) {
                if (effic_val <= 0) {
                    break;
                }
            }
            if (!unk_sum1) {
                econ_weight *= 2;
            }
            econ_weight *= ((PlayersData[faction_id].ai_fight < 0 && unk_sum1 < 2)
                + tech_val * 2 + wealth_val + 1);
            if (output) {
                if (wealth_val || tech_val) {
                    econ_weight *= 2;
                }
                if (growth_val || power_val) {
                    econ_weight /= 2;
                }
            } else if (wealth_val && !power_val && !growth_val) {
                econ_weight *= 2;
            }
            unk_val6 += econ_weight / unk_val3;
            // support
            int support_val = range(soc_eff.values_[SocialEffect::SUPPORT] + 4, 0, 7);
            int support_weight = PlayersData[faction_id].unk_38[support_val];
            if (soc_eff.values_[SocialEffect::SUPPORT] <= -4) {
                support_weight *= 3;
            }
            if (unk_sum1) {
                support_weight *= 2;
                if (soc_eff.values_[SocialEffect::SUPPORT] <= -3) {
                    support_weight *= 2;
                }
                if ((soc_eff.values_[SocialEffect::SUPPORT] == -1 || soc_eff.values_[SocialEffect::SUPPORT] == -2) && soc_eff.values_[SocialEffect::ECONOMY] < 2) {
                    support_weight += support_weight / 2;
                }
            }
            if (output) {
                if (power_val) {
                    support_weight *= 3;
                }
                if (growth_val) {
                    support_weight *= 2;
                }
                if (tech_val) {
                    support_weight /= 2;
                }
            } else {
                if (fight_val > 0) {
                    support_weight *= 2;
                }
                if (growth_val > 0) {
                    support_weight += support_weight / 2;
                }
                if (wealth_val > 0) {
                    support_weight /= 2;
                }
                if (tech_val > 0) {
                    support_weight += support_weight / -4;
                }
                if (power_val > 0) {
                    support_weight += support_weight / 2;
                }
            }
            unk_val6 -= support_weight;
            // morale
            int morale_val = range(soc_eff.values_[SocialEffect::MORALE], -4, 4) * 2;
            if (morale_val < -6) {
                morale_val++;
            }
            if (morale_val < -2) {
                morale_val++;
            }
            if (morale_val > 6) {
                morale_val--;
            }
            if (morale_val > 2) {
                morale_val--;
            }
            int morale_weight = (unk_val3 * morale_val * (fight_val + 2) * (unk_sum1 + 1)
                * (PlayersData[faction_id].current_num_bases + PlayersData[faction_id].unk_48)) / 8;
            if (output) {
                if (power_val) {
                    morale_weight *= 3;
                }
                if (growth_val || wealth_val) {
                    morale_weight /= 2;
                }
            } else {
                if (fight_val > 1) {
                    morale_weight *= 2;
                }
                if (power_val && !growth_val && !wealth_val) {
                    morale_weight *= 2;
                }
                if (fight_val < 0 && !power_val && (wealth_val || tech_val) && !unk_sum1) {
                    morale_weight /= 2;
                }
            }
            unk_val6 += morale_weight;
            // efficiency
            int effic_weight = effic_val;
            if (soc_eff.values_[SocialEffect::EFFICIENCY] == -3) {
                effic_weight = 2 * effic_val;
            } else if (soc_eff.values_[SocialEffect::EFFICIENCY] <= -4) {
                effic_weight = 4 * effic_val;
            }
            if (!unk_sum1) {
                effic_weight *= 2;
            }
            if (output) {
                if (tech_val) {
                    effic_weight *= 3;
                }
                if (growth_val || power_val) {
                    effic_weight /= 2;
                }
            } else {
                if (!wealth_val && !tech_val) {
                    if (power_val) {
                        effic_weight /= 2;
                    }
                } else if (power_val) {
                    if (!wealth_val) {
                        effic_weight /= 2;
                    }
                } else if (!growth_val) {
                    effic_weight *= 2;
                }
            }
            unk_val6 -= effic_weight;
            // growth
            int growth_weight = (soc_eff.values_[SocialEffect::GROWTH] * (unk_count1 + 1)
                * ((growth_val + 1) * 2 - wealth_val)
                * (pop_goal_growth + PlayersData[faction_id].nutrient_surplus_total)) / 5;
            if (aah_ooga(faction_id, faction_id)) {
                growth_weight *= 2;
            }
            if (Players[faction_id].rule_population > 0) {
                growth_weight /= 2;
            }
            if (output) {
                if (growth_val || wealth_val) {
                    growth_weight *= 2;
                }
                if (tech_val) {
                    growth_weight /= 2;
                }
            } else {
                if (growth_val && power_val) {
                    growth_weight *= 2;
                }
            }
            unk_val6 += growth_weight;
            // police
            int police_weight
                = range(soc_eff.values_[SocialEffect::POLICE], -2, 2) * PlayersData[faction_id].current_num_bases
                * (unk_count2 * 2 + 1);
            if (PlayersData[faction_id].tech_ranking
                < PlayersData[GameDataStateStorage.fields_.ranking_faction_id_unk2_].tech_ranking
                && PlayersData[faction_id].ranking < PlayersData[GameDataStateStorage.fields_.ranking_faction_id_unk2_].ranking) {
                police_weight *= 2;
            }
            if (PlayersData[faction_id].tech_ranking
                < PlayersData[GameDataStateStorage.fields_.ranking_faction_id_unk1_].tech_ranking
                && PlayersData[faction_id].ranking < PlayersData[GameDataStateStorage.fields_.ranking_faction_id_unk1_].ranking) {
                police_weight /= 2;
            }
            if (output && (power_val || growth_val)) {
                police_weight *= 2;
            }
            unk_val6 += police_weight / (2 - fight_val);
            // talent
            int talent_weight = PlayersData[faction_id].unk_39[range(soc_eff.values_[SocialEffect::TALENT] + 4, 0, 7)]
                [range(soc_eff.values_[SocialEffect::POLICE] + 5, 0, 8)];
            if (unk_sum1) {
                talent_weight += soc_eff.values_[SocialEffect::POLICE] * PlayersData[faction_id].current_num_bases;
            }
            if (output) {
                if (power_val || growth_val) {
                    talent_weight *= 2;
                }
                if (wealth_val) {
                    talent_weight /= 2;
                }
            } else {
                if (growth_val && tech_val) {
                    talent_weight *= 2;
                }
                if (power_val && (tech_val || wealth_val)) {
                    talent_weight /= 2;
                }
            }
            unk_val6 += talent_weight;
            // planet
            int planet_weight = (4 - soc_eff.values_[SocialEffect::PLANET]) * PlayersData[faction_id].unk_49 * 4 / 4; // ?
            if (PlayersData[faction_id].soc_effect_base.values_[SocialEffect::PLANET] > 0
                || PlayersData[faction_id].best_psi_offense
                > PlayersData[faction_id].best_weapon_value) {
                planet_weight += PlayersData[faction_id].total_mil_veh;
            } else if (unk_val3 > 1) {
                planet_weight /= 2;
            }
            if (output) {
                if (growth_val) {
                    planet_weight *= 2;
                }
                if (wealth_val) {
                    planet_weight /= 2;
                }
            } else {
                if (power_val) {
                    if (growth_val) {
                        planet_weight /= 2;
                    }
                    if ((tech_val || wealth_val) && !growth_val) {
                        planet_weight /= 2;
                    }
                } else {
                    if (growth_val) {
                        if (tech_val) {
                            planet_weight *= 2;
                            if (!wealth_val && growth_val) {
                                planet_weight /= 2;
                            }
                        } else {
                            if (growth_val) {
                                planet_weight /= 2;
                            }
                        }
                    } else {
                        if ((tech_val || wealth_val) && growth_val) {
                            planet_weight /= 2;
                        }
                    }
                }
                unk_val6 -= planet_weight;
                // research
                int research_weight = range(soc_eff.values_[SocialEffect::RESEARCH], -5, 5)
                    * PlayersData[faction_id].labs_total / 10;
                if (!unk_sum1) {
                    research_weight *= 2;
                }
                if (output) {
                    if (tech_val) {
                        research_weight *= 2;
                    }
                    if (power_val) {
                        research_weight /= 2;
                    }
                }
                unk_val6 += research_weight;
                // industry
                int unk_val3_temp = range(unk_val3, 1, 2);
                int industry_weight = soc_eff.values_[SocialEffect::INDUSTRY] * unk_val3_temp
                    * PlayersData[faction_id].pop_total
                    * (2 * (wealth_val + unk_sum1) + 1) / 10;
                if (power_val) {
                    industry_weight *= 2;
                }
                if (wealth_val) {
                    industry_weight += industry_weight / 2;
                }
                if (tech_val) {
                    industry_weight /= 2;
                }
                if (fight_val < 0 && !wealth_val) {
                    industry_weight /= 2;
                }
                unk_val6 += industry_weight;
                // why is this before probe? significance?
                unk_val6 += social_upheaval(faction_id, &soc_cat) / -3;
                // probe
                if (soc_eff.values_[SocialEffect::PROBE]) {
                    int probe_weight;
                    for (uint32_t f = 0; f < MaxPlayerNum; f++) {
                        if (f != faction_id) {
                            probe_weight = PlayersData[player].mind_control_total / 4 + 1;
                            if (has_treaty(faction_id, player, DTREATY_PACT)) {
                                probe_weight /= 2;
                            }
                            if (!has_treaty(faction_id, player, DTREATY_WANT_REVENGE)) {
                                if (has_treaty(faction_id, player, DTREATY_TREATY)) {
                                    probe_weight /= 2;
                                }
                                if (!has_treaty(faction_id, player, DTREATY_COMMLINK)) {
                                    probe_weight /= 2;
                                }
                                if (!has_treaty(faction_id, player, DTREATY_VENDETTA)) {
                                    probe_weight /= 2;
                                }
                            }
                            probe_weight = ((PlayersData[player].current_num_bases
                                + PlayersData[faction_id].current_num_bases)
                                * range(soc_eff.values_[SocialEffect::PROBE], -2, 3) * probe_weight) / 2;
                            if (tech_val || wealth_val) {
                                probe_weight *= 2;
                            }
                            unk_val6 += probe_weight;
                        }
                    }
                }
                if (unk_val6 >= unk_val4) {
                    unk_val4 = unk_val6;
                    soc_cat_bits = unk_sum2;
                }
            }
        }
    }
    if (soc_cat_bits < 0) {
        if (output) {
            for (int i = 0; i < MaxSocialCatNum; i++) {
                output->models_[i]
                    = PlayersData[faction_id].soc_category_pending.models_[i];
            }
        }
    } else {
        if (!output) {
            BOOL no_cat_change = true;
            for (int i = 0; i < MaxSocialCatNum; i++) {
                uint32_t cat_bit = soc_cat_bits & 3;
                if (cat_bit != PlayersData[faction_id].soc_category_pending.models_[i]) {
                    PlayersData[faction_id].soc_category_pending.models_[i] = cat_bit;
                    no_cat_change = false;
                }
                soc_cat_bits >>= 2;
            }
            if (!no_cat_change) {
                social_set(faction_id);
                uint32_t cost
                    = social_upheaval(faction_id, &PlayersData[faction_id].soc_category_pending);
                PlayersData[faction_id].energy_reserves
                    = range(PlayersData[faction_id].energy_reserves - cost, 0, 999999999);
                PlayersData[faction_id].soc_upheaval_cost_paid += cost;
                if (!is_human(faction_id)) {
                    consider_designs(faction_id);
                }
            }
        } else {
            for (int i = 0; i < MaxSocialCatNum; i++) {
                output->models_[i] = soc_cat_bits & 3;
                soc_cat_bits >>= 2;
            }
        }
    }
}

// ORIGINAL: 0x005B5620 ?social_select@@YAXH@Z 0x005B5620-0x005B5688 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/recovered/units/005b5620.cpp retained original identity/extent; pinned instructions5B5620..5B5688 establish flags, typed receivers and ordered calls; recovery/tests/test_social_select_behavior.py actual-source controls", "git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/recovered/units/005b5620.cpp"], "reason": "SocialWin owner opens the real shared window, synchronizes choices and applies them offline, preserving native entry-state guards and editor selection. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
// kind      game
// TRIED: homed into general.cpp 2026-09-03 - does not compile: the body calls
void __cdecl social_select(int faction_id) {
    const uint8_t state = static_cast<uint8_t>(GameDataStateStorage.fields_.game_state_);
    if ((state & (STATE_SCENARIO_EDITOR | STATE_OMNISCIENT_VIEW)) == 0 &&
        (GameDataStateStorage.fields_.net_turn_flags_ & 8u) != 0)
        return;

    int selected_faction;
    if ((state & STATE_OMNISCIENT_VIEW) != 0) {
        selected_faction = ConsoleGlobal.editor_who(faction_id, 1);
        if (selected_faction < 0)
            return;
    } else {
        selected_faction = faction_id;
    }

    TutWinSocWindow.init(selected_faction);
    TutWinSocWindow.close();
    synch_soc(selected_faction);
    if (IsMultiplayerNet == 0)
        social_set(selected_faction);
}
