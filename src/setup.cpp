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
#define DIALOGS_DTOR_INLINE  // stand-in: see ~Dialogs in dialogs.h
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

// ORIGINAL: 0x005AE0F0 _$E61 0x005AE0F0-0x005AE106 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT dispatcher 0x00644DEC-0x00644DFB supplies [0x00682000,0x006826CC) to 0x00644ED8, which calls each non-null dword. Slot 0x00682658 selects init 0x005AE0F0, which registers exit callback 0x005AE110.", "Original init span 0x005AE0F0-0x005AE106 and exit span 0x005AE110-0x005AE11A are independently disassembled; both identify receiver 0x00945E08 and Wave constructor/destructor 0x004C66E0/0x004C67C0.", "Historical earlier wave.cpp physical COFF selected local STATIC init _$E61 via .CRT$XCU, relocating to g_CRASH_LANDING_WAVE, Wave construction, paired local fini _$E59 and atexit; fini relocates to the same object and Wave destruction. Native comparator can select these physical local function symbols.", "Independent pin/COFF/source review supports individual lifetime ownership. These mappings do not establish complete global initialization order, linked retained selection, full data layout, original debug names or runtime acceptance."], "reason": "Bind the original startup operation to the genuine compiler-emitted lifetime of source-owned g_CRASH_LANDING_WAVE. The local symbol identifies current emitted code, not an original debug name; no callback body is authored."}
// symbol    _$E20
// ORIGINAL: 0x005AE110 _$E59 0x005AE110-0x005AE11A FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT dispatcher 0x00644DEC-0x00644DFB supplies [0x00682000,0x006826CC) to 0x00644ED8, which calls each non-null dword. Slot 0x00682658 selects init 0x005AE0F0, which registers exit callback 0x005AE110.", "Original init span 0x005AE0F0-0x005AE106 and exit span 0x005AE110-0x005AE11A are independently disassembled; both identify receiver 0x00945E08 and Wave constructor/destructor 0x004C66E0/0x004C67C0.", "Historical earlier wave.cpp physical COFF selected local STATIC init _$E61 via .CRT$XCU, relocating to g_CRASH_LANDING_WAVE, Wave construction, paired local fini _$E59 and atexit; fini relocates to the same object and Wave destruction. Native comparator can select these physical local function symbols.", "Independent pin/COFF/source review supports individual lifetime ownership. These mappings do not establish complete global initialization order, linked retained selection, full data layout, original debug names or runtime acceptance."], "reason": "Bind the original exit operation to the genuine compiler-emitted lifetime of source-owned g_CRASH_LANDING_WAVE. The local symbol identifies current emitted code, not an original debug name; no callback body is authored."}
// symbol    _$E18
// RECOVERY_OBJECT: v1 0x00945E08 ?g_CRASH_LANDING_WAVE@@3VWave@@A
Wave g_CRASH_LANDING_WAVE;  // 0x00945E08

// Shared separator objects the image pushes instead of unit literals.
extern const char BufferJoinSpace[];

extern int ConsoleNoMapOverrideLatch;

// WORKAROUND(inline): dead /Ob2 budget probe (folds to nothing). Without eight
// expansions after the no-vehicle return, that site's teardown expands
// ~FlatButton where the image calls it (0x00406880); placement after the site
// does not matter, the count does. The original's spend is not identified.
static inline int crash_landing_budget_probe(int x) {
    if (x == 1) x = 2;
    return x;
}
// ORIGINAL: 0x005AE120 ?crash_landing@@YAXH@Z 0x005AE120-0x005AED43;0x00662450-0x00662705 FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:8e59b5d75bd6859572a92070e841049900f54fa3:src/unrecovered/005ae120.cpp", "pinned:5AE120..5AED43 and662450..662705;5AE594 fungusx0;5AE58E/5AE5D0 signedcenters;5AE74A..74F fungus smoke gate", "source:src/game.cpp:control_turn;src/popup.h;src/flic.h;src/buffer.cpp", "source:recovery/tests/test_crash_landing_actual_source.py"], "reason": "Recover the complete first-landing operation through real Popup, Buffer, Flic, MapWin, Wave, Player, Base and vehicle owners with automatic normal/exceptional lifetimes. The historical second local widget aggregate is interpreted as Popup-owned embedded components, not duplicated manual vtable storage. Preserve first-owned-base/vehicle selection, early no-match teardown, single-player FUNGBOY shortcut, native fungus start at zero, signed viewport halves, previous-footprint restoration and normal-only smoke independent of pod draw return. The smoke table is existing ExplosionColorTables row12 at6FDB7C; the possible lookup beyond its4096-byte row remains unresolved original asset/read-range evidence, not permission to introduce detached storage or a bounds fix. Both primary and cleanup spans and the seed historical transcription limitations remain obligations; no byte or runtime acceptance is granted."}
// TRIED: byte-exactness is out of reach here without more structural knowledge than the scaffold carries. The SEH frame is real (fs:[0] chain, __CxxFrameHandler, ~40-entry unwind thunk table) and Popup/BasePop already has its own giant field layout (FlatButton x2, Sprite, a 0xC94 dialogs_ blob) that the emitter resolved from other callers, so declaring `Popup popup;` and letting real RAII drive its construction/close() is right and reproduces the two search loops -> short-cleanup vs big-body split faithfully. What is NOT resolvable from this scaffold alone: a SECOND, function-local widget aggregate (GraphicWin+Dialog+ListBox+EditGroup+SpriteBox+CheckBox+Sprite+ 2x FlatButton+2x StringStruct+Heap) built with fully-inlined (vtable-write-only, no visible constructor call) construction and torn down through this-pointer adjustor arithmetic that reads a vtable-relative offset (`aiStackY_4900[1]+4/+8`) - true multiple/virtual-inheritance thunk codegen, not reachable by declaring ordinary separate locals. Modelled those as separate top-level objects in the same relative call order instead, which preserves the call graph but not the stack layout or the thunk bytes. Also approximated: the per-frame x87 pixel-interpolation math in the 30-frame drop loop (kept the draw/decode/timing calls exact, simplified the float position derivation) and the two indirect-IAT-style calls to StringStruct::remove_all/sub_406a90/sub_406820, which were dropped rather than guessing a `this`.
void __cdecl crash_landing(int faction_id) {
    int vehicle_start = 0;
    Popup popup;
    Buffer captured_screen;
    Flic pod_animation;
    Flic smoke_animation;
    Buffer pod_frame;
    Buffer smoke_frame;

    int tile_x;
    int tile_y;
    int base_id;
    for (base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; ++base_id) {
        if (Bases[base_id].faction_id_current == faction_id) break;
    }
    if (base_id >= GameDataStateStorage.fields_.base_current_count_) {
        vehicle_start = 1;
        int vehicle_id;
        for (vehicle_id = 0; vehicle_id < GameDataStateStorage.fields_.vehicle_current_count_; ++vehicle_id) {
            if (Vehs[vehicle_id].faction_id == faction_id) break;
        }
        if (vehicle_id >= GameDataStateStorage.fields_.vehicle_current_count_) return;
        tile_x = Vehs[vehicle_id].x;
        tile_y = Vehs[vehicle_id].y;
    } else {
        tile_x = Bases[base_id].x;
        tile_y = Bases[base_id].y;
    }
    crash_landing_budget_probe(3);
    crash_landing_budget_probe(4);
    crash_landing_budget_probe(5);

    clear();
    const int fungus = (Players[ConsoleGlobal.faction_id_].rule_flags >> 7) & 1;
    says(Players[ConsoleGlobal.faction_id_].filename);
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
        g_CRASH_LANDING_WAVE.init("fx\\crash land.wav", 4);
        g_CRASH_LANDING_WAVE.play();
    }

    pod_frame.init(0x15f, 0x197, 0, 0);
    smoke_frame.init(0x15f, 0x197, 0, 0);
    if (fungus) {
        pod_animation.init(&pod_frame, "alien_pod.flc", 0, 0, 0);
    } else {
        pod_animation.init(&pod_frame, "drop_pod.flc", 0, 0, 0);
        smoke_animation.init(&smoke_frame, "drop_smoke.flc", 0, 0, 0);
    }

    {
    MapWinTable[0]->field_1DD70_ &= 0xfffff73f;
    MapWinTable[0]->set_center(tile_x, tile_y, 1);
    draw_map(1);
    do_all_draws();

    const int screen_width = MapWinTable[0]->pixel_width();
    const int screen_height = MapWinTable[0]->pixel_height();
    captured_screen.init(screen_width, screen_height, 0, 0);
    MapWinTable[0]->copy(&captured_screen, 0, 0, screen_width, screen_height);

    int tile_pixel_x;
    int tile_pixel_y;
    MapWinTable[0]->tile_to_pixel(tile_x, tile_y, &tile_pixel_x, &tile_pixel_y);

    float x = static_cast<float>(MapWinTable[0]->pixel_width() - 0x15f);
    float y = static_cast<float>(MapWinTable[0]->pixel_height() - 0x197);
    float target_x;
    float target_y;
    if (fungus) {
        x = 0.0f;
        target_x = static_cast<float>(
            MapWinTable[0]->tile_pixel_width_ / 2 + tile_pixel_x - 0xa8);
        target_y = static_cast<float>(
            MapWinTable[0]->tile_pixel_height_ / 2 + tile_pixel_y - 0xc0);
    } else {
        target_x = static_cast<float>(
            MapWinTable[0]->tile_pixel_width_ / 2 + tile_pixel_x - 0xa8);
        target_y = static_cast<float>(
            MapWinTable[0]->tile_pixel_height_ / 2 + tile_pixel_y - 0xc0);
    }
    const float x_step = (target_x - x) * (1.0f / 30.0f);
    const float y_step = (target_y - y) * (1.0f / 30.0f);
    y = y + 15.0f;

    RECT previous_rect;
    previous_rect.top = static_cast<int>(y);
    previous_rect.left = static_cast<int>(x);
    previous_rect.right = previous_rect.left + 0x15f;
    previous_rect.bottom = previous_rect.top + 0x197;
    tile_y = timeGetTime();

    for (int frame = 0; frame < 0x1e; ++frame) {
        captured_screen.copy(MapWinTable[0], &previous_rect);

        if (fungus) {
            x = x_step - static_cast<float>(frame / 2) + x + 7.0f;
            y = static_cast<float>(frame) + y_step - 15.0f + y;
        } else {
            x = static_cast<float>(frame / 2) + x_step - 7.0f + x;
            y = static_cast<float>(frame) + y_step - 15.0f + y;
        }

        pod_animation.decode_stored_frame();
        smoke_animation.decode_stored_frame();

        pod_frame.draw(MapWinTable[0], 9, static_cast<int>(x), static_cast<int>(y), 1, 1);
        if (!fungus) {
            smoke_frame.draw_multi_table_dest(
                MapWinTable[0], 0xff, static_cast<int>(x), static_cast<int>(y), 1, 1,
                ExplosionColorTables[12]);
        }

        RECT frame_rect;
        frame_rect.left = static_cast<int>(x);
        frame_rect.top = static_cast<int>(y);
        frame_rect.right = frame_rect.left + 0x15f;
        frame_rect.bottom = frame_rect.top + 0x197;
        UnionRect(&previous_rect, &frame_rect, &previous_rect);
        MapWinTable[0]->update(&previous_rect, 0);
        previous_rect = frame_rect;

        const int shift_held = HIBYTE(GetAsyncKeyState(VK_SHIFT));
        const int frame_delay = shift_held ? 1 : 0x47;
        int tick;
        do {
            tick = timeGetTime();
        } while (tick - tile_y < frame_delay && !ConsoleNoMapOverrideLatch);
        tile_y = tick;
    }
    }

flush_and_continue:
    flush_input();
    SpriteScaleNumerator = 1;
    SpriteScaleDenominator = 1;
    if (!vehicle_start) {
        char base_name[256];
        strcpy(base_name, Bases[base_id].name_string);
        popup.start(PopupStartCaption, "FIRSTBASE", 0x17, base_name, 4, 0);
        popup.exec(0, 0);
        strcpy(Bases[base_id].name_string, ParseStrBuffer[0].str);
        make_base_unique(base_id);
    }

    MapWinTable[0]->field_1DD70_ |= 0x8c0;
    draw_map(1);
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

// ORIGINAL: 0x005AEDE0 ?time_warp@@YAXXZ 0x005AEDE0-0x005B0412
// RECOVERY_BINDING: v1 {"evidence": ["Independent pinned full-span review by report_draw_labs_review; retained game-setup-owner-wave full body and actual-source controls for three profiles, land/aquatic sites, path modes and native relocation."], "reason": "Recover complete accelerated-start setup with typed faction, base, prototype, map and Path owners; preserve callback-sensitive region rereads and native facility/path order. External unfinished callees remain explicit."}
// TRIED: MISMATCH #0 push/sub at prologue; full-body transcription from Ghidra hypothesis, corrected against raw disasm for: has_tech arg counts (Ghidra folded a dangling outer-call push into two call sites), Path::find(8 ints) called through g_00945b00, veh_lift/veh_drop split (veh_lift returns a value the mangled YAX hid; PROPOSAL: veh_lift should be `int veh_lift(int)`), and per-field pointer widths (short/byte) recovered from `movsx word/byte ptr` on every g_0097d0xx/g_009ab8xx/g_0096xxxx access since the scaffold's g_ globals are declared int* uniformly.
// Rewritten from the image 2026-11-11 (0.70 -> 0.966): the nine prototype scans
// are get_plan() expanded by /Ob2; research count, facility order, energy bonus
// and ai_* field choices restored from the image.
void __cdecl time_warp() {
    // [ebp-0x58] (imgmerge.py, par-6): the image keeps the coast-placed flag and the path length counter in one home
    int v_58;
    // [ebp-0x1c] (imgmerge.py, par-6): the image stores the best landing score, the
    // smallest-base scan index and the best site quality in one home
    int v_1c;
    int scenario_turn;
    int initial_research;
    int research_adjustment;
    int expansion_count;
    int initial_population;
    int base_id;
    int veh_id;
    int x;
    int y;
    // No default: the image leaves all five uninitialized outside 0..2.
    switch (rnd(3, "SETUPMODE")) {
    case 0:
        scenario_turn = 30;
        initial_research = 6;
        research_adjustment = 2;
        expansion_count = 3;
        initial_population = 5;
        break;
    case 1:
        scenario_turn = 60;
        initial_research = 12;
        research_adjustment = 3;
        expansion_count = 4;
        initial_population = 6;
        break;
    case 2:
        scenario_turn = 80;
        initial_research = 16;
        research_adjustment = 4;
        expansion_count = 5;
        initial_population = 6;
        break;
    }

    for (int faction_id = 1; faction_id < MaxPlayerNum; ++faction_id) {
        if (!is_alive(faction_id)) continue;
        base_id = base_find(0, 0, faction_id);
        if (base_id < 0) continue;
        Bases[base_id].population_size = static_cast<int8_t>(initial_population);
        if (Players[faction_id].rule_talent) ++Bases[base_id].population_size;
        if (Players[faction_id].rule_drone || Players[faction_id].rule_population)
            --Bases[base_id].population_size;
        v_58 = 0;
        Bases[base_id].nutrients_accumulated = Bases[base_id].population_size * 10 / 2;
        int start_region = region_at(Bases[base_id].x, Bases[base_id].y);

        const int research_effect =
            PlayersData[faction_id].soc_effect_base.values_[SocialEffect::RESEARCH];
        const int morale_effect =
            PlayersData[faction_id].soc_effect_base.values_[SocialEffect::MORALE];
        int research_count =
            ((research_effect > 0 ? 1 : (research_effect >= 0 ? 0 : -1))
             - (morale_effect > 0 ? 1 : (morale_effect >= 0 ? 0 : -1)))
            * research_adjustment + initial_research;
        AiResearchBatchActive = 1;
        for (int advance = 0; advance < research_count; ++advance) {
            PlayersData[faction_id].tech_id_researching = tech_ai(faction_id);
            tech_advance(faction_id);
        }
        PlayersData[faction_id].tech_ranking = PlayersData[faction_id].tech_achieved << 1;
        AiResearchBatchActive = 0;
        PlayersData[faction_id].tech_id_researching = -1;
        consider_designs(faction_id);

        set_fac(FAC_RECREATION_COMMONS, base_id, 1);
        set_fac(FAC_RECYCLING_TANKS, base_id, 1);
        if (PlayersData[faction_id].ai_wealth) set_fac(FAC_ENERGY_BANK, base_id, 1);
        if (PlayersData[faction_id].ai_power) set_fac(FAC_COMMAND_CENTER, base_id, 1);
        if (PlayersData[faction_id].ai_tech) set_fac(FAC_NETWORK_NODE, base_id, 1);
        if (PlayersData[faction_id].ai_growth) {
            if (ExpansionEnabled && (Players[faction_id].rule_flags & RFLAG_AQUATIC))
                veh_init(has_tech(VehPrototypes[BSC_TRANSPORT_FOIL].preq_tech, faction_id)
                             ? BSC_TRANSPORT_FOIL : BSC_UNITY_FOIL,
                         faction_id, Bases[base_id].x, Bases[base_id].y);
            else
                veh_init(BSC_UNITY_ROVER, faction_id, Bases[base_id].x, Bases[base_id].y);
            // Original bug (0x5AF0B2): facility and base id arguments swapped.
            set_fac(FAC_PERIMETER_DEFENSE, 1, base_id);
        }
        PlayersData[faction_id].energy_reserves +=
            (PlayersData[faction_id].ai_wealth * 9 + PlayersData[faction_id].ai_growth * 5
             + PlayersData[faction_id].ai_tech + 1) * 25;

        int naval_proto = get_plan(faction_id, PLAN_NAVAL_SUPERIORITY);
        int foil_proto = has_tech(VehPrototypes[BSC_TRANSPORT_FOIL].preq_tech, faction_id)
            ? BSC_TRANSPORT_FOIL : BSC_UNITY_FOIL;
        int opening_proto = get_plan(faction_id, PLAN_DEFENSIVE);
        if (opening_proto < 0) opening_proto = get_plan(faction_id, PLAN_COMBAT);
        if (opening_proto < 0) opening_proto = BSC_SCOUT_PATROL;
        VehPrototypes[opening_proto].flags |= 4;
        int opening_veh = veh_init(opening_proto, faction_id,
                                   Bases[base_id].x, Bases[base_id].y);
        Vehs[opening_veh].order = ORDER_HOLD;
        ++Vehs[opening_veh].morale;
        opening_veh = veh_init(opening_proto, faction_id, Bases[base_id].x, Bases[base_id].y);
        Vehs[opening_veh].order = ORDER_HOLD;
        if (ExpansionEnabled && (Players[faction_id].rule_flags & RFLAG_AQUATIC))
            veh_id = veh_init(BSC_SEA_FORMERS, faction_id, Bases[base_id].x, Bases[base_id].y);
        else
            veh_id = veh_init(BSC_FORMERS, faction_id, Bases[base_id].x, Bases[base_id].y);

        int combat_proto = get_plan(faction_id, PLAN_COMBAT);
        if (combat_proto >= 0) {
            VehPrototypes[combat_proto].flags |= 4;
            veh_id = veh_init(combat_proto, faction_id, Bases[base_id].x, Bases[base_id].y);
            ++Vehs[veh_id].morale;
            if (combat_proto == BSC_MIND_WORMS) Vehs[veh_id].morale = 2;
        }

        int offensive_proto = get_plan(faction_id, PLAN_OFFENSIVE);
        if (offensive_proto >= 0) {
            VehPrototypes[offensive_proto].flags |= 4;
            if (PlayersData[faction_id].ai_fight <= 0 && !PlayersData[faction_id].ai_power) {
                // Native skips the plan-0 unit and adds a plan-1 one, unflagged.
                int fallback_proto = get_plan(faction_id, PLAN_COMBAT);
                if (fallback_proto >= 0) {
                    veh_id = veh_init(fallback_proto, faction_id,
                                      Bases[base_id].x, Bases[base_id].y);
                    if (fallback_proto == BSC_MIND_WORMS) Vehs[veh_id].morale = 2;
                }
            } else {
                veh_id = veh_init(offensive_proto, faction_id,
                                  Bases[base_id].x, Bases[base_id].y);
                ++Vehs[veh_id].morale;
                if (PlayersData[faction_id].ai_fight > 0)
                    veh_id = veh_init(offensive_proto, faction_id,
                                      Bases[base_id].x, Bases[base_id].y);
            }
        }

        int project;
        do {
            project = rnd(8, "SETUPPROJ");
        } while (GameDataStateStorage.fields_.secret_project_.project_base[project] != SP_Unbuilt
                 || project == 4
                 || (project == 5 && !strcmp(Players[faction_id].filename, "HIVE")));
        GameDataStateStorage.fields_.secret_project_.project_base[project] = base_id;

        for (int round = 0; round < expansion_count - 1; ++round) {
            v_1c = 0;
            int best_x;
            int best_y;
            for (base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_;
                 ++base_id) {
                if (Bases[base_id].faction_id_current != faction_id) continue;
                x = Bases[base_id].x;
                y = Bases[base_id].y;
                int center_region = region_at(x, y);
                // Rings 3..8: RadiusOffset entries 25..288.
                for (int offset = 0; offset < 264; ++offset) {
                    int cx = wrap_map_x(RadiusOffsetX[offset + 25] + x);
                    int cy = RadiusOffsetY[offset + 25] + y;
                    if (!(cy >= 0 && cy < (int)MapLatitudeBounds && cx >= 0 && cx < (int)MapLongitudeBounds)) continue;
                    Map *tile = map_loc(cx, cy);
                    if ((tile->bit & BIT_FUNGUS)
                        && (tile->climate & ~0x1F) >= ALT_BIT_OCEAN_SHELF) continue;
                    if (((tile->climate & ~0x1F) < ALT_BIT_SHORE_LINE
                         && !(Players[faction_id].rule_flags & RFLAG_AQUATIC))
                        || ((tile->climate & ~0x1F) >= ALT_BIT_SHORE_LINE
                            && (Players[faction_id].rule_flags & RFLAG_AQUATIC))) continue;
                    if ((tile->val3 & ~0x3F) > 0x40
                        && !(Players[faction_id].rule_flags & RFLAG_AQUATIC)) continue;
                    int region = tile->region;
                    int site;
                    if (Players[faction_id].rule_flags & RFLAG_AQUATIC) {
                        if (tile->val3 & 7) site = 0;
                        else if (tile->bit & BIT_BASE_RADIUS) site = 0;
                        else {
                            site = tile->val2 >> 4;
                            if (!site) {
                                site = world_site(cx, cy, TRUE);
                                site_set(cx, cy, site);
                            }
                        }
                    } else {
                        if (tile->val3 & 7) site = 0;
                        else if (tile->bit & BIT_BASE_RADIUS) site = 0;
                        else {
                            site = tile->val2 >> 4;
                            if (!site) {
                                site = world_site(cx, cy, FALSE);
                                site_set(cx, cy, site);
                            }
                        }
                    }
                    int score = (Bases[base_id].population_size + site) * 32;
                    if (center_region != region) score /= 2;
                    if (offset >= 56 && offset < 144) score = score * 3 / 2;
                    if (offset >= 200) score -= score / 4;
                    if (offset < 24) score /= 2;
                    int nearest = base_find(cx, cy);
                    if (base_id != nearest
                        && map_range(cx, cy, Bases[nearest].x, Bases[nearest].y) < 4)
                        continue;
                    if (Bases[nearest].faction_id_current != faction_id) {
                        // Original quirk: the same tile as `region`, so always equal.
                        if (region_at(cx, cy) == region) continue;
                        score /= 2;
                    }
                    if (score > v_1c) {
                        v_1c = score;
                        best_x = cx;
                        best_y = cy;
                    }
                }
            }

            if (v_1c) {
                int new_base_id = base_init(faction_id, best_x, best_y);
                Bases[new_base_id].population_size =
                    static_cast<int8_t>(expansion_count - round);
                if (Bases[new_base_id].population_size > 2)
                    set_fac(FAC_RECREATION_COMMONS, new_base_id, 1);
                if (Bases[new_base_id].population_size > 3)
                    set_fac(FAC_RECYCLING_TANKS, new_base_id, 1);
                Bases[new_base_id].nutrients_accumulated =
                    Bases[new_base_id].population_size * 10 / 2;
                int garrison_proto = get_plan(faction_id, PLAN_DEFENSIVE);
                if (garrison_proto < 0) garrison_proto = get_plan(faction_id, PLAN_COMBAT);
                if (garrison_proto < 0) garrison_proto = BSC_SCOUT_PATROL;
                veh_id = veh_init(garrison_proto, faction_id, best_x, best_y);
                if (garrison_proto == BSC_MIND_WORMS) Vehs[veh_id].morale = 2;
                Vehs[veh_id].order = ORDER_HOLD;
                if (Bases[new_base_id].population_size > 2) {
                    veh_id = veh_init(garrison_proto, faction_id, best_x, best_y);
                    if (garrison_proto == BSC_MIND_WORMS) Vehs[veh_id].morale = 2;
                    Vehs[veh_id].order = ORDER_HOLD;
                }
                if (PlayersData[faction_id].ai_growth || Bases[new_base_id].population_size > 3) {
                    if (Players[faction_id].rule_flags & RFLAG_AQUATIC)
                        veh_id = veh_init(BSC_SEA_FORMERS, faction_id, best_x, best_y);
                    else
                        veh_id = veh_init(BSC_FORMERS, faction_id, best_x, best_y);
                }
                if (region_at(best_x, best_y) != start_region
                    && is_port(new_base_id, FALSE)
                    && !v_58) {
                    for (int dir = 0; dir < 8; ++dir) {
                        int sx = wrap_map_x(VehMoveDeltaX[dir] + best_x);
                        int sy = VehMoveDeltaY[dir] + best_y;
                        if (on_map(sx, sy) && is_ocean(sx, sy)) {
                            if (naval_proto >= 0) {
                                VehPrototypes[naval_proto].flags |= 4;
                                ++Vehs[veh_init(naval_proto, faction_id, sx, sy)].morale;
                            }
                            veh_init(foil_proto, faction_id, sx, sy);
                            veh_id = veh_init(BSC_UNITY_ROVER, faction_id, sx, sy);
                            sleep(veh_id);
                            v_58 = 1;
                            break;
                        }
                    }
                }
            } else {
                int smallest_id = -1;
                int smallest_pop = 999;
                for (v_1c = 0; v_1c < GameDataStateStorage.fields_.base_current_count_; ++v_1c) {
                    if (Bases[v_1c].faction_id_current == faction_id
                        && Bases[v_1c].population_size < smallest_pop) {
                        smallest_pop = Bases[v_1c].population_size;
                        smallest_id = v_1c;
                    }
                }
                if (smallest_id >= 0) {
                    if (Bases[smallest_id].population_size < 6)
                        Bases[smallest_id].population_size += 2;
                    set_fac(FAC_RECREATION_COMMONS, smallest_id, 1);
                    set_fac(FAC_RECYCLING_TANKS, smallest_id, 1);
                    set_fac(FAC_COMMAND_CENTER, smallest_id, 1);
                    int extra_proto = get_plan(faction_id, PLAN_COMBAT);
                    if (extra_proto >= 0) {
                        // Original bug (0x5AFDB2..0x5AFDC5): result discarded,
                        // the previous veh_id gets the morale.
                        veh_init(extra_proto, faction_id,
                                 Bases[smallest_id].x, Bases[smallest_id].y);
                        if (extra_proto == BSC_MIND_WORMS) Vehs[veh_id].morale = 2;
                    }
                    // Original bug (0x5AFD81): the sea pod uses the last scanned
                    // base centre x/y (possibly stale/uninitialized), not the base.
                    if (Players[faction_id].rule_flags & RFLAG_AQUATIC)
                        veh_id = veh_init(BSC_SEA_ESCAPE_POD, faction_id, x, y);
                    else
                        veh_init(BSC_COLONY_POD, faction_id,
                                 Bases[smallest_id].x, Bases[smallest_id].y);
                    veh_init(7, faction_id, Bases[smallest_id].x, Bases[smallest_id].y);
                }
            }
        }

        AiResearchBatchActive = 1;
        reset_territory();
        enemy_strategy(faction_id);
        for (base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; ++base_id) {
            if (Bases[base_id].faction_id_current != faction_id) continue;
            for (int i = 0; i < 81; ++i) {
                int rx = wrap_map_x(RadiusOffsetX[i] + Bases[base_id].x);
                int ry = RadiusOffsetY[i] + Bases[base_id].y;
                if (!on_map(rx, ry)) continue;
                map_loc(rx, ry)->visibility |= 1 << faction_id;
                if (goody_at(rx, ry) && i < 21)
                    map_loc(rx, ry)->bit |= BIT_SUPPLY_REMOVE | BIT_RSC_BONUS;
            }
            set_base(base_id);
            base_compute(1);
            for (int pass = 0;
                 pass < Bases[base_id].population_size * 2 / 3 + PlayersData[faction_id].ai_growth;
                 ++pass) {
                base_terraform(1);
                base_compute(1);
            }
            Bases[base_id].resource_sqr_radius = 0;
            for (i = 0; i < 81; ++i) {
                x = wrap_map_x(RadiusOffsetX[i] + Bases[base_id].x);
                y = RadiusOffsetY[i] + Bases[base_id].y;
                if (on_map(x, y)) synch_bit(x, y, faction_id);
            }
            for (int other = 0; other < GameDataStateStorage.fields_.base_current_count_; ++other) {
                if (base_id == other) continue;
                if (Bases[other].faction_id_current != faction_id) continue;
                int px = Bases[other].x;
                int py = Bases[other].y;
                if (region_at(px, py) != region_at(Bases[base_id].x, Bases[base_id].y)) continue;
                x = Bases[base_id].x;
                y = Bases[base_id].y;
                v_58 = 0;
                int step = Paths.find(px, py, x, y, 0, -1,
                    has_tech(Rules.tech_build_roads_fungus, faction_id) ? 0 : 4, -1);
                while (step >= 0) {
                    px = wrap_map_x(VehMoveDeltaX[step] + px);
                    py += VehMoveDeltaY[step];
                    if (px == x && py == y) break;
                    if (base_who(px, py) < 0 && !(map_loc(px, py)->bit & BIT_ROAD)) {
                        map_loc(px, py)->bit |= BIT_ROAD;
                        synch_bit(px, py, faction_id);
                    }
                    if (++v_58 > 50) break;
                    step = Paths.find(px, py, x, y, 0, -1,
                        has_tech(Rules.tech_build_roads_fungus, faction_id) ? 0 : 4, -1);
                }
            }
            base_reset(base_id, 0);
        }
        AiResearchBatchActive = 0;

        for (veh_id = 0; veh_id < GameDataStateStorage.fields_.vehicle_current_count_; ++veh_id) {
            if (Vehs[veh_id].faction_id != faction_id) continue;
            if (VehPrototypes[Vehs[veh_id].proto_id].plan != PLAN_TERRAFORMING) continue;
            if (Players[faction_id].rule_flags & RFLAG_AQUATIC) {
                if (Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad != TRIAD_SEA)
                    continue;
            } else if (Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad != TRIAD_LAND)
                continue;
            v_1c = 0;
            for (int dir = 0; dir < 8; ++dir) {
                int tx = wrap_map_x(VehMoveDeltaX[dir] + Vehs[veh_id].x);
                int ty = VehMoveDeltaY[dir] + Vehs[veh_id].y;
                if (!on_map(tx, ty)) continue;
                if (Players[faction_id].rule_flags & RFLAG_AQUATIC) {
                    if (!is_ocean(tx, ty)) continue;
                } else if (is_ocean(tx, ty)) continue;
                int quality = 1;
                int rocky = map_loc(tx, ty)->val3 & ~0x3F;
                if (rocky < 0x80) quality = 2;
                if (map_loc(tx, ty)->bit & BIT_ROAD) quality += 2;
                if (rocky == 0x40 && (map_loc(tx, ty)->climate & 0x18)
                    && max(0, alt_at(tx, ty) - 2))
                    quality += 2;
                if (quality > v_1c) {
                    v_1c = quality;
                    veh_drop(veh_lift(veh_id), tx, ty);
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
            if (map_loc(Vehs[vehicle_id].x, Vehs[vehicle_id].y)->region >= 0x40) {
                vehicles_by_faction[faction_id] =
                    veh_find(Vehs[vehicle_id].x, Vehs[vehicle_id].y, faction_id, vehicle_id);
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
            const int x_raw = RadiusOffsetX[i] + x;
            const int tile_x = wrap_map_x(x_raw);
            const int tile_y = RadiusOffsetY[i] + y;
            if (!on_map(tile_x, tile_y)) {
                continue;
            }
            const Map *tile = map_loc(tile_x, tile_y);
            const int rainfall = (tile->climate >> 3) & 3;
            const int altitude = tile->climate >> 5;
            const int rockiness = tile->val3 >> 6;
            if (rainfall < 1) {
                continue;
            }
            if ((max(0, altitude - 2) || (tile->bit & BIT_RIVER)) && rockiness < 2 &&
                (rockiness == 1 || max(0, altitude - 2) > 1 || rainfall > 1)) {
                ++land_count;
            }
            if (rockiness == 1 && max(0, altitude - 2) > rockiness) {
                ++land_count;
            }
        }

        // Image 0x005B0748..0x005B0790: half the land count clamped to 0..8,
        // plus tile_count / 64 (no rounding) clamped to 0..3.
        nearest += range(land_count / 2, 0, 8) + range(Continents[region].tile_count / 64, 0, 3);
        if (nearest > best_score) {
            best_score = nearest;
        }
        faction_scores[faction_id] = nearest;
    }

    for (faction_id = 1; faction_id < MaxPlayerNum; ++faction_id) {
        int vehicle_id = vehicles_by_faction[faction_id];
        if (vehicle_id < 0) {
            continue;
        }
        const int x = Vehs[vehicle_id].x;
        const int y = Vehs[vehicle_id].y;
        const int seed = rnd(16, "SETUPRAD");
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


// ORIGINAL: 0x005B0A30 ?scenario_setup@@YAXXZ 0x005B0A30-0x005B0D69
// RECOVERY_BINDING: v1 {"reason":"Recover complete descending vehicle-placement retries with real prototypes/chassis/Map records, native odd-y x adjustment and failed-placement deletion. Original68FAF0 is MapLongitude for tile-row stride and wrap threshold, not full MapLongitudeBounds; air-only placement does not form a terrain pointer. No byte/reference or runtime claim is granted.","evidence":["git:3f579310:src/unrecovered/005b0a30.cpp","image:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf","recovery/tasks/scenario-artifact-unity-owner-coverage.json","Original full span and native physical COFF emitted symbol ?scenario_setup@@YAXXZ; existing source owns all referenced storage and services."]}
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
            // WORKAROUND(type): 0x005B0A7D reads the flag as a signed byte
            // (movsx; the image's only byte-sign read of is_flat_) and computes
            // the latitude bound before the x offset; a char-typed term gives
            // both. The original operand's declared type is unknown.
            x = rnd(MapLongitudeBounds - (char)((MapIsFlat & 1) * 4), "SCENSETUP2") + (MapIsFlat & 1) * 2;
            y = rnd(MapLatitudeBounds - 4, "SCENSETUP3") + 2;
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
                    int distance = map_range(x, y, Vehs[other_id].x, Vehs[other_id].y);
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

// ORIGINAL: 0x005B0E00 ?setup_player@@YAHHHH@Z 0x005B0E00-0x005B337C
// RECOVERY_BINDING: v1 {"evidence": ["git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/005b0e00.cpp: canonical original identity, full spans and retained seed provenance", "source:src/faction.h: real faction record ownership and typed relationships", "source:recovery/tasks/faction-lifecycle-owner-closure.json: coordinator authorization and retained task baseline"], "reason": "The original faction setup operation belongs to the Faction owner; real Player/PlayerData records, prototype scoring, owned network state, shared game counters and named base/vehicle dependencies express creation and respawn paths without seed executable addresses."}
// TRIED: full-body transcription from Ghidra hypothesis, corrected against raw disasm for: tech_ai/tech_achieved and abil_index/ has_tech dangling-outer-call pushes (same shape as time_warp's has_tech fold), veh_lift/veh_drop split (PROPOSAL: veh_lift should be `int veh_lift(int)`), CharUpperA reached through the IAT thunk at g_0066931c (matches src/recovered's established idiom), and per-field pointer widths across the ~230 referenced globals recovered from `movsx`/`test byte ptr` on the raw disassembly, since the scaffold's g_ globals are declared int* uniformly. All C2xxx NO_COMPILE errors from iterative verify_recovered_function.py runs were fixed; the final byte-tier measurement did not finish inside this session (a single VC6 /O2 pass on this body ran >12 minutes without completing - the same body compiles other optimisation levels' passes fine, so this reads as a slow pass, not a stuck one).
int __cdecl setup_player(int faction, int conqueror, int retry) {
    // 5B0E0F: failed is cleared before the first duplicate counter.
    int failed = 0;
    int duplicate = 0;
    char number[80];
    char prototype_name[80];
    // No Player/PlayerData references: the image recomputes faction*0x59C
    // and faction*0x20CC per section and never forms a record pointer.
    for (int previous = 1; previous < faction; ++previous)
        if (!strcmp(Players[previous].name_leader, Players[faction].name_leader)) ++duplicate;
    if (duplicate) {
        Players[faction].name_leader[21] = 0;
        strcat(Players[faction].name_leader, "-");
        _itoa(duplicate + 1, number, 10);
        strcat(Players[faction].name_leader, number);
    }
    duplicate = 0;
    for (int previous2 = 1; previous2 < faction; ++previous2)
        if (!strcmp(Players[previous2].noun_faction, Players[faction].noun_faction)) ++duplicate;
    if (duplicate) {
        Players[faction].noun_faction[21] = 0;
        strcat(Players[faction].noun_faction, "-");
        _itoa(duplicate + 1, number, 10);
        strcat(Players[faction].noun_faction, number);
    }
    duplicate = 0;
    for (int previous3 = 1; previous3 < faction; ++previous3)
        if (!strcmp(Players[previous3].formal_name_faction, Players[faction].formal_name_faction)) ++duplicate;
    if (duplicate) {
        Players[faction].formal_name_faction[37] = 0;
        strcat(Players[faction].formal_name_faction, "-");
        _itoa(duplicate + 1, number, 10);
        strcat(Players[faction].formal_name_faction, number);
    }
    for (int base = GameDataStateStorage.fields_.base_current_count_ - 1; base >= 0; --base)
        if (Bases[base].faction_id_current == faction) base_kill(base);
    for (int vehicle = GameDataStateStorage.fields_.vehicle_current_count_ - 1; vehicle >= 0; --vehicle)
        if (vehicle < GameDataStateStorage.fields_.vehicle_current_count_ && Vehs[vehicle].faction_id == faction
            && (Vehs[vehicle].proto_id != 0x13 || faction != 0)) kill(vehicle);
    // 5B1061: y_target, x_target, then base_id_atk_target.
    PlayersData[faction].base_id_atk_target = PlayersData[faction].x_target = PlayersData[faction].y_target = -1;
    if (!GameDataStateStorage.fields_.turn_current_num_ || conqueror == -282) {
        init_goals(faction);
        clear_bunglist(faction);
        // 5B109D..5B13A2: the faction reset is written out here.
        PlayersData[faction].flags = 0;
        // 5B10A7: store, then a conditional overwrite (no setcc arithmetic).
        PlayersData[faction].base_governor_adv = 0xdfbe41;
        if (!GameDataStateStorage.fields_.diff_level_current_) PlayersData[faction].base_governor_adv = 0xc0dfbe41;
        PlayersData[faction].base_name_offset = 0;
        PlayersData[faction].base_sea_name_offset = 0;
        // 5B10E7: fight, power, tech, wealth, growth.
        PlayersData[faction].ai_fight = Players[faction].ai_fight;
        PlayersData[faction].ai_power = Players[faction].ai_power;
        PlayersData[faction].ai_tech = Players[faction].ai_tech;
        PlayersData[faction].ai_wealth = Players[faction].ai_wealth;
        PlayersData[faction].ai_growth = Players[faction].ai_growth;
        PlayersData[faction].unk_37 = 0;
        for (int queue = 0; queue < 8; ++queue) {
            PlayersData[faction].saved_queue_size[queue] = 0;
            StringTemp[0] = 0;
            strcat(StringTemp, label_get(0x984 / 4));
            strcat(StringTemp, BufferJoinSpace);
            _itoa(queue + 1, number, 10);  // 5B116D: the shared number buffer.
            strcat(StringTemp, number);
            strcpy(PlayersData[faction].saved_queue_name[queue], StringTemp);
        }
        for (int category = 0; category < 4; ++category)
            PlayersData[faction].soc_category_pending.models_[category] = PlayersData[faction].soc_category_active.models_[category] = 0;
        // 5B11C2: rep stosd over the 11 pending effects.
        for (int effect = 0; effect < 11; ++effect) PlayersData[faction].soc_effect_pending.values_[effect] = 0;
        PlayersData[faction].soc_upheaval_cost_paid = 0;
        for (int other = 0; other < 8; ++other) PlayersData[faction].secret_project_intel[other] = 0;
        PlayersData[faction].diff_level = is_human(faction) ? GameDataStateStorage.fields_.diff_level_current_ : 3;
        // 5B120E..5B124A: store order of the image.
        PlayersData[faction].integrity_blemishes = 0;
        PlayersData[faction].global_reputation = 0;
        PlayersData[faction].traded_maps = 0;
        PlayersData[faction].energy_reserves = 0;
        PlayersData[faction].atrocities = 0;
        PlayersData[faction].major_atrocities = 0;
        PlayersData[faction].planet_ecology = 0;
        PlayersData[faction].sanction_turns = 0;
        TectonicDetonationCount[faction] = 0;
        PlayersData[faction].council_call_turn = -999;
        if (GameDataStateStorage.fields_.game_state_ & 0x800000) {
            PlayersData[faction].ai_fight = rand() % 3 - 1;
            PlayersData[faction].ai_power = rand() % 2;
            PlayersData[faction].ai_tech = rand() % 2;
            PlayersData[faction].ai_wealth = rand() % 2;
            PlayersData[faction].ai_growth = rand() % 2;
        }
        if (GameDataStateStorage.fields_.game_rules_ & 0x40) PlayersData[faction].ai_fight = 1;
        // 5B12D9: ascending stores.
        PlayersData[faction].total_mil_veh = 0;
        PlayersData[faction].current_num_bases = 0;
        PlayersData[faction].mil_strength_1 = 0;
        PlayersData[faction].mil_strength_2 = 0;
        PlayersData[faction].pop_total = 0;
        PlayersData[faction].unk_70 = 0;
        PlayersData[faction].mind_control_total = 0;
        for (int other2 = 0; other2 < 8; ++other2)
            PlayersData[faction].stolen_data_count[other2] = PlayersData[faction].diplo_mind_control[other2] = 0;
        PlayersData[faction].net_random_event = 0;
        // 5B1327: combat, bases, offensive, force, unk_77.
        for (int region = 0; region < 128; ++region) {
            PlayersData[faction].region_total_combat_vehs[region] = 0;
            PlayersData[faction].region_total_bases[region] = 0;
            PlayersData[faction].region_total_offensive_vehs[region] = 0;
            PlayersData[faction].region_force_rating[region] = 0;
            PlayersData[faction].unk_77[region] = 0;
        }
        for (int council = 0; council < 11; ++council)
            PlayersData[faction].unk_30[council] = PlayersData[faction].unk_29[council] = 0;
        // 5B1369: one rep stosd of ten dwords, corner_market_turn..unk_109.
        for (int clear = 0; clear < 10; clear++) {
            (&PlayersData[faction].corner_market_turn)[clear] = 0;
        }
        PlayersData[faction].unk_93 = 0;
        PlayersData[faction].unk_94 = 0;
        PlayersData[faction].unk_95 = 0;
        PlayersData[faction].unk_96 = 0;
        PlayersData[faction].unk_97 = 0;
        PlayersData[faction].tech_achieved = 0;
        PlayersData[faction].time_bonus_count = 1;
    }
    PlayersData[faction].energy_reserves += Players[faction].rule_energy
        + (IsMultiplayerNet ? 100 : Rules.player_start_energy_reserve);
    PlayersData[faction].hurry_cost_total = 0;
    PlayersData[faction].tech_cost = -1;
    PlayersData[faction].last_turn_new_base = GameDataStateStorage.fields_.turn_current_num_;
    for (int other = 0; other < 8; ++other) {
        // 5B1413: the respawn arm (treaty_off) is the then-block.
        if (GameDataStateStorage.fields_.turn_current_num_ && conqueror != -282)
            treaty_off(faction, other, 0x9000ff48);
        else {
            // 5B143E: treaties, then merc.
            PlayersData[faction].diplo_merc[other] = PlayersData[faction].diplo_treaties[other] = 0;
            int limit = ((GameDataStateStorage.fields_.game_rules_ & 0x40) ? 5 : GameDataStateStorage.fields_.diff_level_current_) * 3 + 8;
            PlayersData[faction].diplo_friction[other] = range(rnd(limit, "SETUP1"), 1, 16);
            // 5B1490: unk1, wrongs, betrayed, patience, spoke, unk4, unk3.
            PlayersData[faction].diplo_betrayed[other] = PlayersData[faction].diplo_wrongs[other] = PlayersData[faction].diplo_unk1[other] = 0;
            PlayersData[faction].diplo_patience[other] = 0;
            PlayersData[faction].diplo_spoke[other] = -1;
            PlayersData[faction].diplo_unk3[other] = PlayersData[faction].diplo_unk4[other] = 0;
        }
        // 5B14BC: unk_12, balance, payment.
        PlayersData[faction].loan_payment[other] = PlayersData[faction].loan_balance[other] = PlayersData[faction].unk_12[other] = 0;
    }
    for (int proto = 0; proto < 512; ++proto) {
        PlayersData[faction].proto_id_active[proto] = PlayersData[faction].proto_id_queue[proto] = 0;
        if (GameDataStateStorage.fields_.turn_current_num_ == 0) PlayersData[faction].proto_id_lost[proto] = 0;
    }
    if (!GameDataStateStorage.fields_.turn_current_num_ || conqueror == -282) {
        for (int goal = 0; goal < 12; ++goal) PlayersData[faction].tech_pact_shared_goals[goal] = 0;
        // The original clears 22 words and then one byte, leaving the last
        // three bytes of this 92-byte field unchanged.
        for (int source = 0; source < 89; ++source) PlayersData[faction].tech_trade_source[source] = 0;
        for (int announced = 0; announced < 9; ++announced) PlayersData[faction].facility_announced[announced] = 0;
        for (int satellite = 0; satellite < 4; ++satellite) PlayersData[faction].satellites_[satellite] = 0;
        // 5B157D: theory, tech_ranking, sat_odp.
        PlayersData[faction].sat_odp_deployed = PlayersData[faction].tech_ranking = PlayersData[faction].theory_of_everything = 0;
        PlayersData[faction].tech_id_researching = -1;
        // 5B1599: accumulated, earned, unk_26, unk_102.
        PlayersData[faction].unk_102 = PlayersData[faction].unk_26 = PlayersData[faction].earned_techs_saved = PlayersData[faction].tech_accumulated = 0;
    }
    // 5B15B1..5B162D: store order of the image.
    PlayersData[faction].unk_17 = PlayersData[faction].unk_18 = PlayersData[faction].maint_cost_total = PlayersData[faction].unk_13 = PlayersData[faction].unk_28 = 0;
    // [S3] 5B15CF..5B15F1: bonus (+0x304), fresh `xor ecx,ecx` + four ascending
    // stores (a 4-trip zeroing loop, as satellites_ at 5B1551), then income (+0x308).
    PlayersData[faction].tech_commerce_bonus = 0;
    for (int fungus = 0; fungus < 4; ++fungus) PlayersData[faction].tech_fungus_[fungus] = 0;
    PlayersData[faction].turn_commerce_income = 0;
    PlayersData[faction].best_weapon_value = 0;
    PlayersData[faction].best_armor_value = 0;
    PlayersData[faction].best_land_speed = 0;
    PlayersData[faction].best_psi_offense = 0;
    PlayersData[faction].best_psi_defense = 0;
    PlayersData[faction].enemy_best_weapon_value = 0;
    PlayersData[faction].enemy_best_armor_value = 0;
    PlayersData[faction].enemy_best_land_speed = 0;
    PlayersData[faction].enemy_best_psi_offense = 0;
    PlayersData[faction].enemy_best_psi_defense = 0;
    if (!faction) return 0;
    if (!is_alive(faction)) return 0;
    if (conqueror < -1 && conqueror != -282) {
        GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_ &= ~(1 << faction);
        return 0;
    }

    int attempts = 0;
    int x, y, region, nearest, occupied, isolation;
    // 5B1C57: area quality and the spacing relaxation share one slot.
    int score;
search:
    {
        int margin = range((1024 - attempts) * MapLongitudeBounds / 8192, 0, 9999);
        x = ((MapIsFlat & 1) ? margin : 0)
            + rnd(MapLongitudeBounds - ((MapIsFlat & 1) ? margin * 2 : 0), "SETUP2");
        margin = range((1024 - attempts) * MapLatitudeBounds / 8192, 0, 9999);
        y = margin + rnd(MapLatitudeBounds - margin * 2, "SETUP3");
        if (x & 1) --x;
        if (y & 1) ++x;  // Original: an odd y adjusts x.
        isolation = 32;
        for (int ring = 0; ring < 8; ++ring) {
            // RadiusRange[ring] ends ring `ring`; ring 0 is the centre only.
            for (int offset = ring ? RadiusRange[ring - 1] : 0; offset < RadiusRange[ring]; ++offset) {
                int rx = wrap_map_x(x + RadiusOffsetX[offset]);
                int ry = y + RadiusOffsetY[offset];
                if (on_map(rx, ry)) {
                    uint32_t landmark = map_loc(rx, ry)->bit2;
                    if (landmark && static_cast<int>(landmark & 0xff000000) > 0) {
                        isolation = ring;
                        ring = 99;
                        break;
                    }
                }
            }
        }
        score = 0;
        for (int offset2 = 0; offset2 < 21; ++offset2) {
            int rx = wrap_map_x(x + RadiusOffsetX[offset2]);
            int ry = y + RadiusOffsetY[offset2];
            if (!on_map(rx, ry)) continue;
            // 5B1888: the sea test precedes map_loc; each arm forms the tile
            // and compares altitude as a dword (`& ~0x1F`).
            if (Players[faction].rule_flags & 0x100) {
                Map *square = map_loc(rx, ry);
                if ((square->bit & 0x20) && (square->climate & ~0x1F) >= 0x40) continue;
                score += (2 - (square->climate >> 5)) * 2;
            } else {
                Map *square = map_loc(rx, ry);
                if ((square->bit & 0x20) && (square->climate & ~0x1F) >= 0x40) continue;
                if ((square->climate & ~0x1F) < 0x60 || (square->val3 & ~0x3F) > 0x40
                    || !(square->climate & 0x18)) continue;
            }
            ++score;  // 5B1914: shared by both arms.
        }
        if (Players[faction].rule_flags & 0x100) score /= 3;
        region = region_at(x, y);
        occupied = 0;
        for (int player = 1; player < 8; ++player)
            if (PlayersData[player].region_total_bases[region]) occupied |= 1 << player;
        nearest = (!(Players[faction].rule_flags & 0x100) && conqueror != -282)
            ? base_find(x, y, -1, region, -1, -1) : base_find(x, y);
        if (nearest < 0) BaseFindDist = 32;
        if (!GameDataStateStorage.fields_.turn_current_num_ || conqueror == -282) {
            for (int vehicle2 = 0; vehicle2 < GameDataStateStorage.fields_.vehicle_current_count_; ++vehicle2) {
                if (region_at(Vehs[vehicle2].x, Vehs[vehicle2].y) != region) continue;
                occupied |= 1 << Vehs[vehicle2].faction_id;
                int distance = vector_dist(x, y, Vehs[vehicle2].x, Vehs[vehicle2].y);
                if (distance < BaseFindDist) BaseFindDist = distance;
            }
        }
        ++attempts;
        if (attempts <= 1024 || (is_human(faction) && !GameDataStateStorage.fields_.turn_current_num_)) {
            // 5B1B23: each arm forms map_loc and materialises an int test.
            int bad_altitude = (Players[faction].rule_flags & 0x100)
                ? (map_loc(x, y)->climate & ~0x1F) > 0x40 : is_ocean(x, y);
            if (bad_altitude || ((map_loc(x, y)->bit & 0x20) && (map_loc(x, y)->climate & ~0x1F) >= 0x40)
                || (!(Players[faction].rule_flags & 0x100) && (map_loc(x, y)->val3 & ~0x3F) > 0x40)) goto search;
            // 5B1B93: 12 on a respawn, 6 when fresh; BaseFindDist on the left.
            // [S4] 5B1BAB: `imul eax,ebx` keeps MapAreaSqRoot in ebx for the
            // second test (5B1C5A `imul ecx,ebx`); a named multiplicand, as the
            // first-pass `separation` local that already produced that CSE.
            int separation = (GameDataStateStorage.fields_.turn_current_num_ && conqueror != -282) ? 12 : 6;
            if (BaseFindDist < separation * MapAreaSqRoot / 56
                || (map_loc(x, y)->climate & ~0x1F) >= 0xa0 || anything_at(x, y) >= 0
                || y < 2 || y >= MapLatitudeBounds - 2
                || score < 12 - attempts / 80) goto search;
            score = attempts / 64;
            if (BaseFindDist < (16 - score) * MapAreaSqRoot / 56
                || ((!GameDataStateStorage.fields_.turn_current_num_ || conqueror == -282) && attempts < 900
                    && Continents[region].tile_count < 36 - score)) goto search;
            if (!(Players[faction].rule_flags & 0x100)) {
                int handicap = is_human(faction) ? (6 - GameDataStateStorage.fields_.diff_level_current_) * 8 : 8;
                if (Continents[region].open_terrain * (MapOceanCoverage + 1) / 2
                    < 24 + handicap - GameDataStateStorage.fields_.turn_current_num_ / 16 - attempts / 16) goto search;
            }
            int difficulty = is_human(faction) ? GameDataStateStorage.fields_.diff_level_current_ : 3;
            // 5B1D7F/5B1DB2: tile counts on the left.
            if (Continents[region].tile_count < (5 - difficulty) * 48 - GameDataStateStorage.fields_.turn_current_num_ / 16 - attempts / 4
                || (occupied && conqueror != -282
                    && Continents[region].tile_count / (bit_count(occupied) + 1) < 64 - score)) goto search;
            // 5B1DBA: map_loc is formed again after the bit_count call.
            int quality = site_at(x, y, 0);
            if ((quality < 15 - score && !GameDataStateStorage.fields_.turn_current_num_ && !(GameDataStateStorage.fields_.game_rules_ & 0x10))
                || (!(Players[faction].rule_flags & 0x100) && (isolation < (1024 - attempts) / 128 || !isolation))
                || (GameDataStateStorage.fields_.turn_current_num_ && conqueror != -282 && map_loc(x, y)->territory > 0)
                || (nearest >= 0 && Bases[nearest].faction_id_current == conqueror
                    && region_at(Bases[nearest].x, Bases[nearest].y) == region)) goto search;
        } else failed = 1;
    }
    if (PlayersData[faction].flags & 0x1000) failed = 1;
    if (!failed || retry) {
        if (!GameDataStateStorage.fields_.turn_current_num_ || conqueror == -282) {
            int best = -1, best_offset = 0;
            for (isolation = 0; isolation < 21; ++isolation) {
                int rx = wrap_map_x(x + RadiusOffsetX[isolation]);
                score = y + RadiusOffsetY[isolation];
                if (!on_map(rx, score)) continue;
                Map &candidate = *map_loc(rx, score);
                int altitude = candidate.climate & 0xe0;
                if ((altitude < 0x60) != ((Players[faction].rule_flags >> 8) & 1)
                    || candidate.region != region || altitude >= 0xa0
                    || ((candidate.bit & 0x20) && altitude >= 0x40)
                    || anything_at(rx, score) >= 0) continue;
                base_find(rx, score);
                // Original 5B204E measures from the old centre, not from
                // this candidate. Keep that quirk when choosing a site.
                for (int vehicle3 = 0; vehicle3 < GameDataStateStorage.fields_.vehicle_current_count_; ++vehicle3) {
                    int distance = vector_dist(x, y, Vehs[vehicle3].x, Vehs[vehicle3].y);
                    if (distance < BaseFindDist) BaseFindDist = distance;
                }
                if (BaseFindDist < 6) continue;
                // 5B20F4: map_loc formed again after the calls.
                int quality;
                Map *site_tile = map_loc(rx, score);
                if (site_tile->bit & 0x1000) quality = 0;
                else {
                    int site = site_tile->val2 >> 4;
                    if (site) quality = site;
                    else {
                        quality = world_site(rx, score, 0);
                        site_set(rx, score, quality);
                    }
                }
                for (int neighbor = 0; neighbor < 8; ++neighbor) {
                    int nx = wrap_map_x(rx + VehMoveDeltaX[neighbor]);
                    int ny = score + VehMoveDeltaY[neighbor];
                    if (!on_map(nx, ny)) continue;
                    if (Players[faction].rule_flags & 0x100) quality += 2 - (map_loc(nx, ny)->climate >> 5);
                    else {
                        // 5B21D9: int compare (jl) of the shifted altitude.
                        if ((map_loc(nx, ny)->climate >> 5) < ALT_SHORE_LINE) { ++quality; break; }
                    }
                }
                if (quality > best) { best = quality; best_offset = isolation; }
            }
            x = wrap_map_x(x + RadiusOffsetX[best_offset]);
            y += RadiusOffsetY[best_offset];
        }
        if (retry) failed = 0;
    }
    // 5B227E..5B22A2: every rejected placement still respawns on a retry.
    if ((failed || (GameDataStateStorage.fields_.turn_current_num_ && conqueror != -282 && (GameDataStateStorage.fields_.game_rules_ & 1))
        || GameDataStateStorage.fields_.turn_current_num_ > 100) && !retry) {
        if (is_human(faction) && !IsMultiplayerNet) {
            EndgameExitReason = 7;
            GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_ &= ~(1 << faction);
            GameDataStateStorage.fields_.game_state_ |= 1;
            return 0;
        }
        PlayersData[faction].unk_102 = 0;
        GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_ &= ~(1 << faction);
        duplicate = faction == static_cast<int>(GameDataStateStorage.fields_.faction_rankings_[7])
            ? static_cast<int>(GameDataStateStorage.fields_.faction_rankings_[6])
            : static_cast<int>(GameDataStateStorage.fields_.faction_rankings_[7]);
        int survivors = 0, partners = 0;
        for (int survivor = 1; survivor < 8; ++survivor) {
            if (!is_alive(survivor)) continue;
            ++survivors;
            uint32_t treaty = PlayersData[survivor].diplo_treaties[ConsoleGlobal.faction_id_];
            if (!IsMultiplayerNet && (treaty & 1) && (treaty & 0x2000000)) { --survivors; continue; }
            if (survivor != duplicate && (PlayersData[survivor].diplo_treaties[duplicate] & 1)
                && (GameDataStateStorage.fields_.game_rules_ & 0x1000) && partners < 2) ++partners;
        }
        if (faction == ConsoleGlobal.faction_id_) {
            // 5B23B6: get_noun's parse_set and field address, expanded.
            parse_says(0, get_noun(conqueror < 0 ? 0 : conqueror), -1, -1);
            if (IsMultiplayerNet) {
                NetDaemonNet.hang_up();
                NetDaemonNet.reset();
                IsMultiplayerNet = 0;
                ExitTurnLoop = 1;
            }
            // 5B2504: two full call setups, tail-merged at 5B2520.
            if (conqueror || Language) X_pop("YOULOSE", 0);
            else X_pop("YOULOSE2", 0);
            if (faction == ConsoleGlobal.faction_id_) {
                GameDataStateStorage.fields_.game_state_ |= 1;
                EndgameExitReason = conqueror ? 7 : 15;
                return 0;
            }
        }
        if (survivors - partners < 2) {
            if (partners) {
                popp(ScriptPopupFile, "CONQCOOP", 0, "conq_sm.pcx", 0);
                if (!EndgameExitReason) EndgameExitReason = 5;
            } else if (duplicate == ConsoleGlobal.faction_id_) {
                popp(ScriptPopupFile, "CONQSING", 0, "conq_sm.pcx", 0);
                if (!EndgameExitReason) EndgameExitReason = 4;
            }
            GameDataStateStorage.fields_.game_state_ |= 0x2001;
        }
        return 0;
    }
    PlayersData[faction].se_alloc_labs = 5;
    PlayersData[faction].se_alloc_psych = 0;
    if ((!GameDataStateStorage.fields_.turn_current_num_ || conqueror == -282) && GameDataStateStorage.fields_.diff_level_current_ >= 2) {
        if ((PlayersData[faction].ai_wealth || PlayersData[faction].ai_tech) && rand() % 4 == 0) PlayersData[faction].flags_ext |= 8;
        // 5B2643: clear-and-append, not strcpy.
        StringTemp[0] = 0;
        strcat(StringTemp, Players[faction].filename);
        CharUpperA(StringTemp);
        if (!strcmp(StringTemp, "ANGELS")) PlayersData[faction].flags_ext |= 8;
        if (Players[faction].rule_flags & 0x100) { PlayersData[faction].flags_ext |= 4; PlayersData[faction].flags |= 0x8000000; }
        if (Players[faction].rule_flags & 0x80) { PlayersData[faction].flags_ext |= 0x10; PlayersData[faction].flags |= 0x10000000; }
        if (!strcmp(StringTemp, "DRONE")) PlayersData[faction].flags_ext = (PlayersData[faction].flags_ext & ~8) | 2;
        if (Continents[region].tile_count < 250 && rand() % 3 == 0) PlayersData[faction].flags_ext |= 4;
        if (PlayersData[faction].ai_fight >= 0 && rand() % 4 == 0) PlayersData[faction].flags_ext |= 0x10;
        if (rand() % 6 == 0) PlayersData[faction].flags_ext |= 2;
        if (rand() % 6 == 0) PlayersData[faction].flags_ext |= 1;
        if (rand() % 5 == 0) PlayersData[faction].flags_ext |= 0x40;
        // 5B27E3: ai_fight on the left (cmp ai_fight, -(rules&0x40 != 0); jle).
        if (((GameDataStateStorage.fields_.game_rules_ & 0x40) || GameDataStateStorage.fields_.diff_level_current_ >= 4)
            && PlayersData[faction].ai_fight > ((GameDataStateStorage.fields_.game_rules_ & 0x40) ? -1 : 0)
            && rand() % 4 == 0) PlayersData[faction].flags_ext |= 0x20;
        if (((GameDataStateStorage.fields_.game_rules_ & 0x40) || GameDataStateStorage.fields_.diff_level_current_ >= 4)
            && PlayersData[faction].ai_fight > ((GameDataStateStorage.fields_.game_rules_ & 0x40) ? -1 : 0)
            && rand() % 4 == 0) {
            PlayersData[faction].flags |= 0x1000000;
            if (rand() % 2 == 0) PlayersData[faction].flags |= 0x2000000;
        }
        if (PlayersData[faction].ai_power || PlayersData[faction].ai_fight > 0) {
            if (Continents[region].tile_count >= 400 && rand() % 4 == 0) PlayersData[faction].flags |= 0x4000000;
            // 5B28BB: guarded CRT draw, so a bound of 1 would always pass.
            if (!bounded_crt_random(Continents[region].tile_count > 250 ? 6 : 4)) PlayersData[faction].flags |= 0x8000000;
            if (rand() % 4 == 0) PlayersData[faction].flags |= 0x10000000;
        }
    }
    for (int offset4 = 0; offset4 < 21; ++offset4) {
        int rx = wrap_map_x(x + RadiusOffsetX[offset4]);
        int ry = y + RadiusOffsetY[offset4];
        if (on_map(rx, ry)) {
            map_loc(rx, ry)->visibility |= 1 << faction;
            synch_bit(rx, ry, faction);
        }
    }
    int scout = 2;
    if (!GameDataStateStorage.fields_.turn_current_num_ || conqueror == -282) {
        for (int bonus = 0; bonus < Players[faction].faction_bonus_count; ++bonus)
            if (Players[faction].faction_bonus_id[bonus] == 0)
                tech_achieved(faction, Players[faction].faction_bonus_val1[bonus], 0, 0);
        if (ExpansionEnabled && (Players[faction].rule_flags & 0x80)) {
            tech_achieved(faction, 0x4e, 0, 0);
            tech_achieved(faction, 0x4f, 0, 0);
        }
        if (IsMultiplayerNet) {
            int learned = 0;
            for (;;) {
                int research = PlayersData[faction].soc_effect_base.values_[SocialEffect::RESEARCH] > 0 ? 1 : PlayersData[faction].soc_effect_base.values_[SocialEffect::RESEARCH] < 0 ? -1 : 0;
                int morale = PlayersData[faction].soc_effect_base.values_[SocialEffect::MORALE] > 0 ? 1 : PlayersData[faction].soc_effect_base.values_[SocialEffect::MORALE] < 0 ? -1 : 0;
                if (learned >= research - morale + 1) break;
                tech_achieved(faction, tech_ai(faction), 0, 0);
                ++learned;
            }
            PlayersData[faction].energy_reserves += Players[faction].rule_energy / 2;
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
    // 5B2AE8: the respawn arm is the then-block; each arm keeps its own
    // pair of veh_init calls (the image tail-merges only the second).
    if (GameDataStateStorage.fields_.turn_current_num_ && conqueror != -282) {
        int vehicle6;
        if (Players[faction].rule_flags & 0x100) {
            veh_init(0x15, faction, x, y);
            vehicle6 = veh_init(4, faction, x, y);
        } else {
            veh_init(0, faction, x, y);
            vehicle6 = veh_init(1, faction, x, y);
        }
        veh_demote(vehicle6);
    } else {
        // 5B2B56: the colony-pod arms are laid out before base_init, each
        // with its own complete call.
        if ((IsMultiplayerNet || (GameDataStateStorage.fields_.game_rules_ & 0x10))
            && !(GameDataStateStorage.fields_.game_rules_ & 0x80)) {
            if (Players[faction].rule_flags & 0x100) veh_init(0x15, faction, x, y);
            else veh_init(0, faction, x, y);
        } else {
            int base = base_init(faction, x, y);
            StringTemp[0] = 0;
            strcat(StringTemp, Players[faction].filename);
            CharUpperA(StringTemp);
            // Two independent tests (5B2BFA, 5B2C18), each with its own store.
            if (!strcmp(StringTemp, "FUNGBOY") && conqueror == -282) Bases[base].population_size = 3;
            if ((Players[faction].rule_flags & 0x80) && conqueror == -282) Bases[base].population_size = 3;
        }
        if (!(Players[faction].rule_flags & 0x100)) {
            int vehicle4 = veh_init(scout, faction, x, y);
            Vehs[vehicle4].home_base_id = -1;
            veh_demote(vehicle4);
        }
        if (IsMultiplayerNet && !(Players[faction].rule_flags & 0x100)) {
            int vehicle5 = veh_init(0xb, faction, x, y);
            Vehs[vehicle5].home_base_id = -1;
            veh_demote(vehicle5);
            vehicle5 = veh_init(1, faction, x, y);
            Vehs[vehicle5].home_base_id = -1;
            veh_demote(vehicle5);
        }
        int coast = is_coast(x, y, 0);
        if (coast && (GameDataStateStorage.fields_.diff_level_current_ > 1 || IsMultiplayerNet
            || Continents[region].tile_count <= 32)) {
            // Both naval tests reuse the original coast-direction index.
            region = region_at(x, y);
            if (IsMultiplayerNet
                && Continents[region].tile_count / (bit_count(occupied) + 1) <= 75
                && is_human(faction)) {
                int vessel = veh_init(0xd, faction, wrap_map_x(x + RadiusOffsetX[coast]), y + RadiusOffsetY[coast]);
                Vehs[vessel].morale = 2;
                veh_demote(vessel);
                Vehs[vessel].home_base_id = -1;
            }
            if (Continents[region].tile_count / (bit_count(occupied) + 1) <= 50) {
                int vessel2 = veh_init(0xd, faction, wrap_map_x(x + RadiusOffsetX[coast]), y + RadiusOffsetY[coast]);
                Vehs[vessel2].morale = 2;
                veh_demote(vessel2);
                Vehs[vessel2].home_base_id = -1;
            }
        }
    }
    if (!GameDataStateStorage.fields_.turn_current_num_ || conqueror == -282) {
        // 5B2E7C..5B2ED2 is this TU's compute_faction_modifiers, inlined.
        compute_faction_modifiers(faction);
        for (int bonus3 = 0; bonus3 < Players[faction].faction_bonus_count; ++bonus3) {
            // 5B2EE3: mov/dec/jne on the id, a switch, not cmp 1.
            switch (Players[faction].faction_bonus_id[bonus3]) {
            case 1: {
                int vehicle7 = veh_init(Players[faction].faction_bonus_val1[bonus3], faction, x, y);
                Vehs[vehicle7].home_base_id = -1;
                veh_demote(vehicle7);
                if (!Players[faction].faction_bonus_val1[bonus3]) PlayersData[faction].flags |= 0x100;
                break;
            }
            }
        }
        if (ExpansionEnabled && (Players[faction].rule_flags & 0x80)) {
            int vehicle8 = veh_init(0x10, faction, x, y);
            Vehs[vehicle8].home_base_id = -1;
            veh_demote(vehicle8);
            vehicle8 = veh_init(0, faction, x, y);
            Vehs[vehicle8].home_base_id = -1;
            veh_demote(vehicle8);
        }
        if (Players[faction].rule_flags & 0x100) {
            if (!is_human(faction)) {
                int prototype = conqueror;
                for (int local_proto2 = 0; local_proto2 < 64; ++local_proto2) {
                    int index = faction * 64 + local_proto2;
                    if (!(VehPrototypes[index].flags & 1)) {
                        prototype = index;
                        local_proto2 = 64;  // 5B2FD7: index forced, no jump out.
                    }
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
            // 5B3110: each arm reads the chassis triad and continues itself.
            if (Players[faction].rule_flags & 0x100) {
                if (Chassis[design.chassis_id].triad != 1) continue;
            } else if (Chassis[design.chassis_id].triad) continue;
            int value = armor_val(prototype2, faction) * 8 - weap_val(prototype2, faction);
            if (value > defense_value) { defense_value = value; defense = prototype2; }
            // 5B316D: weap_val is called before armor_val here.
            value = Chassis[design.chassis_id].speed + (weap_val(prototype2, faction) * 2 + armor_val(prototype2, faction)) * 2;
            if (value > attack_value) { attack_value = value; attack = prototype2; }
            value = Chassis[design.chassis_id].speed * 4 - weap_val(prototype2, faction) - armor_val(prototype2, faction);
            if (value > fast_value && Chassis[design.chassis_id].speed > 1) { fast_value = value; fast = prototype2; }
        }
        // 5B3210/5B32DA: two full call setups per test, tail-merged.
        int colony;
        if (Players[faction].rule_flags & 0x100) colony = veh_init(0x15, faction, x, y);
        else colony = veh_init(0, faction, x, y);
        veh_init(defense, faction, x, y);
        if (GameDataStateStorage.fields_.turn_current_num_ > 20 && fast >= 0) veh_init(fast, faction, x, y);
        if (GameDataStateStorage.fields_.turn_current_num_ > 40 && attack >= 0) veh_init(attack, faction, x, y);
        if (GameDataStateStorage.fields_.turn_current_num_ > 60 && defense >= 0) veh_init(defense, faction, x, y);
        if (GameDataStateStorage.fields_.turn_current_num_ > 80 && attack >= 0) veh_init(attack, faction, x, y);
        if (GameDataStateStorage.fields_.turn_current_num_ > 100) {
            if (Players[faction].rule_flags & 0x100) veh_init(0x15, faction, x, y);
            else veh_init(0, faction, x, y);
        }
        veh_promote(colony);
        PlayersData[faction].unk_26 = PlayersData[faction].tech_ranking * 3 / 4;
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

// ORIGINAL: 0x005B3380 ?eliminate_player@@YAHHH@Z 0x005B3380-0x005B38C6 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/005b3380.cpp: canonical original identity, full spans and retained seed provenance", "source:src/faction.h: real faction record ownership and typed relationships", "source:recovery/tasks/faction-lifecycle-owner-closure.json: coordinator authorization and retained task baseline"], "reason": "The original faction elimination operation belongs to the Faction owner; typed faction diplomacy, base/vehicle lifecycles and shared endgame/script state preserve actual cross-owner relationships rather than raw executable references."}
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
    if (conqueror < 0) {
        conqueror = 0;
    } else if (conqueror > 0) {
        set_treaty(faction, conqueror, 0x2030, 1);
    }
    draw_map(1);
    parse_says(0, get_noun(faction), -1, -1);
    parse_says(1, get_noun(conqueror), -1, -1);
    parse_says(2, get_title(faction), -1, -1);
    parse_says(3, get_name(faction), -1, -1);
    parse_says(4, get_his_her(faction, 0), -1, -1);
    StringTemp[0] = '\0';
    strcat(StringTemp, "WIPEOUT");
    if (!conqueror) {
        char number[80];
        _itoa(2, number, 10);
        strcat(StringTemp, number);
    } else {
        if (conqueror == ConsoleGlobal.faction_id_) strcat(StringTemp, "YOU");
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
    if (!restored) mon_killed_faction(conqueror, faction);
    if (!IsMultiplayerNet && (GameDataStateStorage.fields_.game_preferences_ & 0x10000000)
        && conqueror == ConsoleGlobal.faction_id_ && !restored) {
        amovie_project(Players[faction].filename);
    } else if (restored && !IsMultiplayerNet && conqueror == ConsoleGlobal.faction_id_) {
        int chance = (PlayersData[ConsoleGlobal.faction_id_].diplo_treaties[faction] & 0x1000) ? 2 : 4;
        if (!bounded_crt_random(chance)) {
            int first = veh_find(0, 0, faction, -1);
            if (first >= 0) {
                int x;
                int y;
                for (int offset = 0; offset < 9; ++offset) {
                    x = wrap_map_x(Vehs[first].x + RadiusOffsetX[offset]);
                    y = Vehs[first].y + RadiusOffsetY[offset];
                    if (on_map(x, y)) {
                        spot_loc(x, y, conqueror);
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
                        Vehs[vehicle].visibility = (uint8_t)(1 << conqueror);
                    }
                }
                // The original deliberately retains the last radius coordinates.
                draw_tiles(x, y, 2);
                MapWinTable[0]->set_center(x, y, 1);
                parse_says(6, get_adjective(faction), -1, -1);
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
// The image passes zero-filled 0x945E78/0x945E7C cells, not literals, as
// the cleared "Latest Save"/"Latest Scenario" values; nothing else
// references either cell. Names are descriptive.
// RECOVERY_OBJECT: v1 0x00945E78 ?LatestSaveCleared@@3PADA
char LatestSaveCleared[4];
// RECOVERY_OBJECT: v1 0x00945E7C ?LatestScenarioCleared@@3PADA
char LatestScenarioCleared[4];

void __cdecl setup_game(int reload) {
    GameDataStateStorage.fields_.legacy_game_save_name_[0] = 0;

    prefs_put("Latest Save", LatestSaveCleared);
    prefs_put("Latest Scenario", LatestScenarioCleared);
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

    int clear;
    for (clear = 0; clear < MaxTechnologyNum; clear++) {
        GameDataStateStorage.fields_.game_tech_achieved_[clear] = 0;
    }
    for (clear = 0; clear < int(sizeof GameDataStateStorage.fields_.secret_project_.project_base / sizeof(int)); clear++) {
        GameDataStateStorage.fields_.secret_project_.project_base[clear] = SP_Unbuilt;
    }
    for (clear = 0; clear < 0x2000; clear++) {
        ReplayEvents[clear] = 0;
    }
    for (clear = 0; clear < MaxPlayerNum * MaxRankingHistoryTurns; clear++) {
        FactionRankingHistory[clear] = 0;
    }
    // One block clear of the ten event words sunspot_duration_ through
    // ecology_event_accumulator_ (+0x370..+0x394), as the image's rep stosd.
    for (clear = 0; clear < 10; clear++) {
        (&GameDataStateStorage.fields_.sunspot_duration_)[clear] = 0;
    }

    if (GameDataStateStorage.fields_.game_state_ & 0x01000000u) {
        int seen_social_models[12];
        for (clear = 0; clear < 12; clear++) {
            seen_social_models[clear] = 0;
        }
        for (int social_faction = 1; social_faction < MaxPlayerNum; social_faction++) {
            int rejected = 0;
            int category;
            int model;
            Players[social_faction].soc_ideology_category[0] = -1;
            Players[social_faction].soc_ideology_model[0] = 0;
            Players[social_faction].soc_ideology_effect[0] = -1;
            do {
                category = rand() % 3;
                model = rand() % 3 + 1;
            } while (seen_social_models[category * 3 + model] && ++rejected < social_faction + 1);
            if (rejected <= social_faction + 1) {
                Players[social_faction].soc_ideology_category[0] = category;
                Players[social_faction].soc_ideology_model[0] = model;
                seen_social_models[category * 3 + model] = 1;
                for (int effect = 0; effect < 11; effect++) {
                    if (SocialCategories[category].model_effect[model].values_[effect] > 0) {
                        Players[social_faction].soc_ideology_effect[0] = effect;
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
    for (clear = 0; clear < 8; clear++) {
        TectonicDetonationCount[clear] = 0;
    }
    for (int computer_faction = 0; computer_faction < MaxPlayerNum; computer_faction++) {
        if (!is_human(computer_faction)) {
            setup_player(computer_faction, -1 - reload, 0);
            setup_results[computer_faction] = 0;
        }
    }

    // This is the row-major 8x8 pending-diplomacy flag matrix. Its native
    // reset follows both setup_player passes and precedes the label copies.
    for (clear = 0; clear < MaxPlayerNum * MaxPlayerNum; clear++) {
        PendingDiplomacyFlags[0][clear] = 0;
    }

    // The original publishes the default human's labels before the optional
    // new-game diplomacy/time-warp branch, whose callbacks observe them.
    strcpy(Players[0].formal_name_faction, label_get(245));
    strcpy(Players[0].noun_faction, label_get(245));
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
    int i;
    for (i = 0; i < 11; i++) {
        effect->values_[i] = 0;
    }
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
    if (Players[faction_id].soc_ideology_category[1] == soc_category
        && Players[faction_id].soc_ideology_model[1] == soc_model) {
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
// Rewritten from the image 2026-11-11 (0.866 -> 0.972); remaining: one extra
// frame slot (0x80 vs 0x7c) and the efficiency/planet tail-merge layout.
void __cdecl social_ai(int faction_id, int growth_val, int tech_val, int wealth_val,
                       int power_val, SocialCategory *output) {
    int fight_val;
    int effect_focus = -1; // growth_val > 100 selects one effect to weigh by 1000
    if (output) {
        if (growth_val < 0) {
            return;
        } else if (growth_val > 100) {
            effect_focus = growth_val - 100;
            growth_val = PlayersData[faction_id].ai_growth;
            tech_val = PlayersData[faction_id].ai_tech;
            wealth_val = PlayersData[faction_id].ai_wealth;
            power_val = PlayersData[faction_id].ai_power;
            fight_val = PlayersData[faction_id].ai_fight;
        } else if (power_val) {
            fight_val = 1;
        } else {
            fight_val = wealth_val ? -1 : 0;
        }
    } else if (is_human(faction_id)) {
        return;
    } else {
        growth_val = PlayersData[faction_id].ai_growth;
        tech_val = PlayersData[faction_id].ai_tech;
        wealth_val = PlayersData[faction_id].ai_wealth;
        power_val = PlayersData[faction_id].ai_power;
        fight_val = PlayersData[faction_id].ai_fight;
    }
    // Four models for each of the four categories: 4^4 combinations.
    int combo_count = MaxSocialModelNum;
    for (int i = 1; i < MaxSocialCatNum; i++) {
        combo_count *= MaxSocialModelNum;
    }
    // Population still to grow toward each base's goal.
    int pop_growth = 0;
    for (i = 0; i < GameDataStateStorage.fields_.base_current_count_; i++) {
        if (Bases[i].faction_id_current == faction_id) {
            pop_growth += pop_goal(i) - Bases[i].population_size;
        }
    }
    // Region plan metrics.
    int threat = 0;
    int colony_regions = 0;
    int offense_regions = 0;
    int psi_threat_bases = 0; // accumulated but never read (original)
    for (int region = 1; region < MaxRegionLandNum; region++) {
        if (!bad_reg(region)) {
            if (PlayersData[faction_id].region_total_bases[region]) {
                uint8_t plan = PlayersData[faction_id].region_base_plan[region];
                if (plan == PLAN_DEFENSIVE || PlayersData[faction_id].unk_77[region] & 0x400) {
                    threat += PlayersData[faction_id].region_total_combat_vehs[region] * 2;
                } else if (plan == PLAN_OFFENSIVE) {
                    threat += PlayersData[faction_id].region_total_combat_vehs[region];
                    offense_regions++;
                } else if (plan == PLAN_COLONIZATION) {
                    colony_regions++;
                }
                if (PlayersData[faction_id].region_base_plan[region] <= PLAN_DEFENSIVE) {
                    for (int f = 1; f < MaxPlayerNum; f++) {
                        if (f != faction_id
                            && has_treaty(faction_id, f, DTREATY_VENDETTA | DTREATY_WANT_REVENGE)
                            && PlayersData[f].best_psi_offense >= PlayersData[f].best_weapon_value
                            && PlayersData[f].proto_id_active[BSC_MIND_WORMS] >= 1) {
                            psi_threat_bases += PlayersData[faction_id].region_total_bases[region];
                        }
                    }
                }
            }
        }
    }
    if (output && power_val && threat < 1) {
        threat = 1;
    }
    // Falling behind the tech leader late in the game.
    int rank_factor = 1;
    if (GameDataStateStorage.fields_.game_state_ & 0x200
        && PlayersData[faction_id].ranking < 6) {
        int tech_rank_diff
            = PlayersData[GameDataStateStorage.fields_.ranking_faction_id_unk1_].tech_ranking / 2
            - PlayersData[faction_id].tech_ranking / 2;
        if (tech_rank_diff > 5) {
            rank_factor = 2;
        }
        if (tech_rank_diff > 10) {
            rank_factor++;
        }
    }
    int best_score = -9999;
    int best_combo = -1;
    for (int combo = 0; combo < combo_count; combo++) {
        SocialCategory soc_cat;
        SocialEffect soc_eff;
        int bits = combo;
        BOOL unavailable = false;
        for (int j = 0; j < MaxSocialCatNum; j++) {
            int model = bits & 3;
            soc_cat.models_[j] = model;
            if (model) {
                if ((Players[faction_id].soc_ideology_category[1] == j
                    && Players[faction_id].soc_ideology_model[1] == model)
                    || !has_tech(SocialCategories[j].preq_tech[model], faction_id)) {
                    unavailable = true;
                    break;
                }
            }
            bits >>= 2;
        }
        // Original 5B4A90/5B4AAB skip directly to the outer increment.
        if (unavailable) {
            continue;
        }
        do_all_non_input();
        social_calc(&soc_cat, &soc_eff, faction_id, false, false);
        int score = 0;
        if (effect_focus >= 0) {
            score = soc_eff.values_[effect_focus] * 1000;
        }
        unavailable = false;
        int ideology_cat = Players[faction_id].soc_ideology_category[0];
        if (ideology_cat >= 0) {
            int ideology_model = Players[faction_id].soc_ideology_model[0];
            if (ideology_model) {
                uint32_t model = soc_cat.models_[ideology_cat];
                if (model && model != ideology_model) {
                    unavailable = true;
                }
                if (model == ideology_model) {
                    score += PlayersData[faction_id].current_num_bases;
                }
            }
        }
        int ideology_eff = Players[faction_id].soc_ideology_effect[0];
        if (ideology_eff >= 0) {
            int value = soc_eff.values_[ideology_eff];
            if (value > 0) {
                score += PlayersData[faction_id].current_num_bases;
            }
            if (value < 0) {
                unavailable = true;
            }
        }
        if (unavailable) {
            continue;
        }
        // economy
        int effic_val = PlayersData[faction_id].unk_46[
            range(4 - soc_eff.values_[SocialEffect::EFFICIENCY], 0, 8)];
        int econ_weight = 0;
        if (soc_eff.values_[SocialEffect::ECONOMY] >= 2) {
            int econ_val = soc_eff.values_[SocialEffect::ECONOMY];
            if (econ_val > 4) {
                econ_val = 4;
            }
            econ_weight = (PlayersData[faction_id].unk_47 + econ_val * 2 - 4) / (threat + 1);
        } else if (soc_eff.values_[SocialEffect::ECONOMY] > 0) {
            for (int r = 0; r < MaxContinentNum; r++) {
                econ_weight += PlayersData[faction_id].region_total_bases[r]
                    * ((PlayersData[faction_id].region_base_plan[r] != PLAN_DEFENSIVE) + 1);
            }
            econ_weight /= (threat + 1);
        } else if (soc_eff.values_[SocialEffect::ECONOMY] < -1) {
            for (int r = 0; r < MaxContinentNum; r++) {
                econ_weight -= PlayersData[faction_id].region_total_bases[r]
                    * ((PlayersData[faction_id].region_base_plan[r] != PLAN_DEFENSIVE) + 1)
                    * (soc_eff.values_[SocialEffect::ECONOMY] + 1);
            }
            econ_weight /= (threat + 1);
        } else {
            econ_weight = -1;
        }
        for (; econ_weight > 0; econ_weight--, effic_val--) {
            if (effic_val <= 0) {
                break;
            }
        }
        if (!threat) {
            econ_weight *= 2;
        }
        econ_weight *= ((PlayersData[faction_id].ai_fight < 0 && threat < 2)
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
        score += econ_weight / rank_factor;
        // support
        int support_weight = PlayersData[faction_id].unk_38[
            range(soc_eff.values_[SocialEffect::SUPPORT] + 4, 0, 7)];
        if (soc_eff.values_[SocialEffect::SUPPORT] <= -4) {
            support_weight *= 3;
        }
        if (threat) {
            support_weight *= 2;
            if (soc_eff.values_[SocialEffect::SUPPORT] <= -3) {
                support_weight *= 2;
            }
            if ((soc_eff.values_[SocialEffect::SUPPORT] == -1
                || soc_eff.values_[SocialEffect::SUPPORT] == -2)
                && soc_eff.values_[SocialEffect::ECONOMY] < 2) {
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
        score -= support_weight;
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
        int morale_weight = (rank_factor * morale_val * (fight_val + 2) * (threat + 1)
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
            if (0 > fight_val && !power_val && (wealth_val || tech_val) && !threat) {
                morale_weight /= 2;
            }
        }
        score += morale_weight;
        // efficiency
        int effic_weight = effic_val;
        if (soc_eff.values_[SocialEffect::EFFICIENCY] == -3) {
            effic_weight = 2 * effic_val;
        } else if (soc_eff.values_[SocialEffect::EFFICIENCY] <= -4) {
            effic_weight = 4 * effic_val;
        }
        if (!threat) {
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
            if (wealth_val || tech_val) {
                if (!power_val) {
                    if (!growth_val) {
                        effic_weight *= 2;
                    }
                } else if (!wealth_val) {
                    effic_weight /= 2;
                }
            } else if (power_val) {
                effic_weight /= 2;
            }
        }
        score -= effic_weight;
        // growth
        int growth_weight = (soc_eff.values_[SocialEffect::GROWTH] * (colony_regions + 1)
            * ((growth_val + 1) * 2 - wealth_val)
            * (pop_growth + PlayersData[faction_id].nutrient_surplus_total)) / 5;
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
        } else if (growth_val && power_val) {
            growth_weight *= 2;
        }
        score += growth_weight;
        // police: the original clamps POLICE + 2, not POLICE (5B5010).
        int police_weight = range(soc_eff.values_[SocialEffect::POLICE] + 2, -2, 2)
            * PlayersData[faction_id].current_num_bases * (offense_regions * 2 + 1);
        if (PlayersData[faction_id].tech_ranking
            < PlayersData[GameDataStateStorage.fields_.ranking_faction_id_unk2_].tech_ranking
            && PlayersData[faction_id].ranking
            < PlayersData[GameDataStateStorage.fields_.ranking_faction_id_unk2_].ranking) {
            police_weight *= 2;
        }
        if (PlayersData[faction_id].tech_ranking
            >= PlayersData[GameDataStateStorage.fields_.ranking_faction_id_unk1_].tech_ranking
            && PlayersData[faction_id].ranking
            > PlayersData[GameDataStateStorage.fields_.ranking_faction_id_unk1_].ranking) {
            police_weight /= 2;
        }
        if (output && (power_val || growth_val)) {
            police_weight *= 2;
        }
        score += police_weight / (2 - fight_val);
        // talent
        int talent_weight = PlayersData[faction_id].unk_39
            [range(soc_eff.values_[SocialEffect::TALENT] + 4, 0, 7)]
            [range(soc_eff.values_[SocialEffect::POLICE] + 5, 0, 8)];
        if (threat) {
            talent_weight += soc_eff.values_[SocialEffect::POLICE]
                * PlayersData[faction_id].current_num_bases;
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
        score += talent_weight;
        // planet
        int planet_weight = (4 - soc_eff.values_[SocialEffect::PLANET])
            * PlayersData[faction_id].unk_49 * 4 / 4;
        if (PlayersData[faction_id].soc_effect_base.values_[SocialEffect::PLANET] > 0
            || PlayersData[faction_id].best_psi_offense
            > PlayersData[faction_id].best_weapon_value) {
            planet_weight += PlayersData[faction_id].total_mil_veh;
        } else if (rank_factor > 1) {
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
                if (!growth_val) {
                    planet_weight /= 2;
                }
            } else if (growth_val && !tech_val) {
                planet_weight *= 4;
            }
            if ((tech_val || wealth_val) && !growth_val) {
                planet_weight /= 2;
            }
        }
        score -= planet_weight;
        // research
        int research_weight = range(soc_eff.values_[SocialEffect::RESEARCH], -5, 5)
            * PlayersData[faction_id].labs_total / 10;
        if (!threat) {
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
        score += research_weight;
        // industry
        int industry_weight = soc_eff.values_[SocialEffect::INDUSTRY] * range(rank_factor, 1, 2)
            * PlayersData[faction_id].pop_total * (2 * (wealth_val + threat) + 1) / 10;
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
        score += industry_weight;
        score += social_upheaval(faction_id, &soc_cat) / -3;
        // probe
        if (soc_eff.values_[SocialEffect::PROBE]) {
            for (int f = 1; f < MaxPlayerNum; f++) {
                if (f != faction_id) {
                    int probe_weight = PlayersData[f].mind_control_total / 4 + 1;
                    if (has_treaty(faction_id, f, DTREATY_PACT)) {
                        probe_weight /= 2;
                    }
                    if (!has_treaty(faction_id, f, DTREATY_WANT_REVENGE)) {
                        if (has_treaty(faction_id, f, DTREATY_TREATY)) {
                            probe_weight /= 2;
                        }
                        if (!has_treaty(faction_id, f, DTREATY_COMMLINK)) {
                            probe_weight /= 2;
                        }
                        if (!has_treaty(faction_id, f, DTREATY_VENDETTA)) {
                            probe_weight /= 2;
                        }
                    }
                    probe_weight = ((PlayersData[f].current_num_bases
                        + PlayersData[faction_id].current_num_bases)
                        * range(soc_eff.values_[SocialEffect::PROBE], -2, 3) * probe_weight) / 2;
                    if (tech_val || wealth_val) {
                        probe_weight *= 2;
                    }
                    score += probe_weight;
                }
            }
        }
        if (score >= best_score) {
            best_score = score;
            best_combo = combo;
        }
    }
    if (best_combo < 0) {
        if (output) {
            for (int c = 0; c < MaxSocialCatNum; c++) {
                output->models_[c] = PlayersData[faction_id].soc_category_pending.models_[c];
            }
        }
    } else if (output) {
        for (int c = 0; c < MaxSocialCatNum; c++) {
            output->models_[c] = best_combo & 3;
            best_combo >>= 2;
        }
    } else {
        BOOL unchanged = true;
        for (int c = 0; c < MaxSocialCatNum; c++) {
            uint32_t model = best_combo & 3;
            if (model != PlayersData[faction_id].soc_category_pending.models_[c]) {
                unchanged = false;
                PlayersData[faction_id].soc_category_pending.models_[c] = model;
            }
            best_combo >>= 2;
        }
        if (!unchanged) {
            social_set(faction_id);
            int cost = social_upheaval(faction_id, &PlayersData[faction_id].soc_category_pending);
            PlayersData[faction_id].energy_reserves
                = range(PlayersData[faction_id].energy_reserves - cost, 0, 999999999);
            PlayersData[faction_id].soc_upheaval_cost_paid += cost;
            if (!is_human(faction_id)) {
                consider_designs(faction_id);
            }
        }
    }
}

// ORIGINAL: 0x005B5620 ?social_select@@YAXH@Z 0x005B5620-0x005B5688 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:b2de7f29042d247ee9b448c814029bbbbc35f230:src/recovered/units/005b5620.cpp retained original identity/extent; pinned instructions5B5620..5B5688 establish flags, typed receivers and ordered calls; recovery/tests/test_social_select_behavior.py actual-source controls", "git:b2de7f29042d247ee9b448c814029bbbbc35f230:src/recovered/units/005b5620.cpp"], "reason": "SocialWin owner opens the real shared window, synchronizes choices and applies them offline, preserving native entry-state guards and editor selection. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
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
