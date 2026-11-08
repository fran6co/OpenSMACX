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
// ORIGINAL: 0x00477E10 _$E26 0x00477E10-0x00477E26 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot0x682550 selects original initializer0x477e10..0x477e26, registering fini0x477e30..0x477e3a; complete independent decode establishes exclusive boundaries.", "Source-owned object?MultiWindow@@3VMultiWin@@A identifies original storage0x7fd648. Startup arguments/calls: call 0x47a590; push 0x477e30. Cleanup receiver/calls: mov ecx, 0x7fd648; jmp 0x47a430. Constructor/destructor targets retain their existing class-owned source mappings.", "Fresh physical src/game.cpp COFF selects _$E26 through .CRT$XCU and registers _$E24, both referencing the genuine source global. Original NetMsg argument triples and implicit PlanWin/Console most-derived flags are preserved by the existing declarations; complete virtual-base teardown is compiler-owned, not authored offsets.", "Current local helper symbols interpret present emission, not original debug names or historical TU boundaries. Object starts do not prove allocation extents. This binding establishes neither cross-TU global order, masked-reference correctness, strict linked preservation, dependency-provider equivalence nor runtime/game acceptance; excluded guarded sprite cleanup remains separate."], "reason": "Bind original startup identity to the genuine compiler-generated lifetime of ?MultiWindow@@3VMultiWin@@A in its actual source owner; no manual lifecycle callback is authored."}
// symbol    _$E20
// ORIGINAL: 0x00477E30 _$E24 0x00477E30-0x00477E3A FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot0x682550 selects original initializer0x477e10..0x477e26, registering fini0x477e30..0x477e3a; complete independent decode establishes exclusive boundaries.", "Source-owned object?MultiWindow@@3VMultiWin@@A identifies original storage0x7fd648. Startup arguments/calls: call 0x47a590; push 0x477e30. Cleanup receiver/calls: mov ecx, 0x7fd648; jmp 0x47a430. Constructor/destructor targets retain their existing class-owned source mappings.", "Fresh physical src/game.cpp COFF selects _$E26 through .CRT$XCU and registers _$E24, both referencing the genuine source global. Original NetMsg argument triples and implicit PlanWin/Console most-derived flags are preserved by the existing declarations; complete virtual-base teardown is compiler-owned, not authored offsets.", "Current local helper symbols interpret present emission, not original debug names or historical TU boundaries. Object starts do not prove allocation extents. This binding establishes neither cross-TU global order, masked-reference correctness, strict linked preservation, dependency-provider equivalence nor runtime/game acceptance; excluded guarded sprite cleanup remains separate."], "reason": "Bind original exit identity to the genuine compiler-generated lifetime of ?MultiWindow@@3VMultiWin@@A in its actual source owner; no manual lifecycle callback is authored."}
// symbol    _$E18
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

extern Wave g_CRASH_LANDING_WAVE;
extern int ConsoleNoMapOverrideLatch;

// Shared opening-window owner. The pinned PE places this four-byte pointer
// in the zero-filled .data virtual tail; init_opening publishes the complete
// GraphicWin pointer. All compiled consumers use this source storage.
// RECOVERY_OBJECT: v2 0x00945824 0x00945828 ?OpeningWindow@@3PAVGraphicWin@@A 0x00589B60
GraphicWin *OpeningWindow = 0;


// The rainfall loader publishes three rows of four colors, one byte after
// each get_pixel. The coast renderer consumes row 1 forward or backward.

// Original 452258 walks exactly 53 palette indices. No writer or pointer
// escape was observed; const is that bounded interpretation, not a claim
// about arbitrary computed addresses. Irregular image data stays in the
// pinned, ignored configure-time include, never a second source ledger.


// Mutable panel styles: load_box_sprites fills three columns by seventeen
// rows. The records borrow the actual Sprite and Buffer objects below.
// Original 0x78D528..0x78DD20 is initially zero; it is not a static address table.
// The original loader advances 0x28 per record and stops after 51 buffers.


// Superseded seed deleted; seed paths below are historical Git provenance.
// Each row maps every palette index to its row number (original 0x007A81A0).

// Original 0x006861F0..0x00686330: mutable palette offsets selected by
// alt_get_ocean_detail's 0..79 result. Configuration replaces only the
// accepted prefix, so these startup values and later untouched entries
// persist across missing, partial and repeated load_cycling calls.

// The temporary model retains the fourth record until scope exit, including
// unwinding. A valid record with a different step count leaves its offset alone.
// ORIGINAL: 0x0058F250 ?system_close@@YAXXZ 0x0058F250-0x0058F2E6 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:bb3b3233842b050fb0840e1cc6c226e8f30f7055:src/unrecovered/0058f250.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "recovery/tasks/game-logic-seed-owner-coverage.json", "Pinned58F27B..58F281 and58F2C4..58F2D9: virtual slot0 with deleting flag1, followed by clearing the respective global slot.", "init_opening and system_init allocate GraphicWin and secondary MapWin owners with new; Win/GraphicWin/MapWin virtual destructors and the separate close_opening path establish typed destruction/free semantics.", "recovery/tasks/game-shutdown-owner-oct26dn.json", "recovery/tests/test_system_close_actual_source.py"], "reason": "Recover system shutdown through real heap-owned GraphicWin and MapWin lifetimes: delete the opening window and secondary MapWin slots1..7, preserving each null guard and shared-slot visibility until destruction/free returns. Keep primary slot0 and the original sound, parent, window, Paths and map cleanup order. This replaces the prior mistaken show(1) translation; source recovery alone does not establish current byte exactness or game acceptance."}
void __cdecl system_close() {
    release_sound();

    // Original vtable slot zero and deletion flag 1 destroy heap-owned
    // windows before clearing their shared slots. MapWin slot zero is the
    // primary window and is excluded from this teardown loop.
    Win *console_window = static_cast<Win *>(&ConsoleGlobal);
    BasePopDefaultField322C = console_window;
    GraphicWin *opening = OpeningWindow;
    if (opening != 0) {
        delete opening;
        OpeningWindow = 0;
    }

    TutWinDesWindow.close();
    WinBubbleWindow.close();
    ConsoleGlobal.close();
    Paths.shutdown();
    map_shutdown();

    for (int slot = 1; slot < 8; ++slot) {
        if (MapWinTable[slot] != 0) {
            delete MapWinTable[slot];
            MapWinTable[slot] = 0;
        }
    }
}


char PronounBuffer[256];  // 0x00945838 - see game.h for the naming evidence


// Bound: scalar; every original access is a dword at the start, never indexed, and the following bytes are never referenced.
// RECOVERY_OBJECT: v2 0x009A6488 0x009A648C ?ExpansionEnabled@@3HA 0x004063FB
BOOL ExpansionEnabled;  // 0x009A6488
// RECOVERY_OBJECT: v3 0x009A64EC 0x009A650C ?FactionRankings@@3PAIA 0x005AC690 member ?GameDataStateStorage@@3UGameDataStateWindow@@A fields_.faction_rankings_
// RECOVERY_OBJECT: v3 0x009A6514 0x009A6614 ?SecretProject@@3UBaseSecretProject@@A 0x005002E0 member ?GameDataStateStorage@@3UGameDataStateWindow@@A fields_.secret_project_
// RECOVERY_OBJECT: v3 0x009A64CC 0x009A64D0 ?BaseCurrentCount@@3HA 0x004E68B0 member ?GameDataStateStorage@@3UGameDataStateWindow@@A fields_.base_current_count_
// RECOVERY_OBJECT: v3 0x009A6614 0x009A6618 ?GovernorFaction@@3HA 0x00424410 member ?GameDataStateStorage@@3UGameDataStateWindow@@A fields_.governor_faction_
// RECOVERY_OBJECT: v3 0x009A67DC 0x009A67E0 ?TranscendenceScoreBonus@@3HA 0x005ACBE0 member ?GameDataStateStorage@@3UGameDataStateWindow@@A fields_.transcendence_score_bonus_
// RECOVERY_OBJECT: v3 0x009A67E0 0x009A6800 ?TranscendenceScoreWeights@@3PAHA 0x005ACBE0 member ?GameDataStateStorage@@3UGameDataStateWindow@@A fields_.transcendence_score_weights_
// RECOVERY_OBJECT: v3 0x009A64E0 ?TimeControlsSelection@@3HA member ?GameDataStateStorage@@3UGameDataStateWindow@@A fields_.time_controls_selection_
// RECOVERY_OBJECT: v3 0x009A6618 0x009A6644 ?CouncilResolutionCount@@3PAHA 0x005B3920 member ?GameDataStateStorage@@3UGameDataStateWindow@@A fields_.council_resolution_count_
// RECOVERY_OBJECT: v3 0x009A6644 0x009A6670 ?CouncilLastResolutionTurn@@3PAHA 0x005B3920 member ?GameDataStateStorage@@3UGameDataStateWindow@@A fields_.council_last_resolution_turn_
// RECOVERY_OBJECT: v3 0x009A64D0 0x009A64D4 ?GameSetupResetWord@@3HA 0x005B3920 member ?GameDataStateStorage@@3UGameDataStateWindow@@A fields_.game_setup_reset_word_
// RECOVERY_OBJECT: v3 0x009A64E4 0x009A64E8 ?TurnControlDurationMilliseconds@@3HA 0x00523C60 member ?GameDataStateStorage@@3UGameDataStateWindow@@A fields_.turn_control_duration_milliseconds_
// RECOVERY_OBJECT: v3 0x009A6810 0x009A6814 ?SaveDaemonStateWord@@3HA 0x005A94F0 member ?GameDataStateStorage@@3UGameDataStateWindow@@A fields_.save_daemon_state_word_
// RECOVERY_OBJECT: v3 0x009A6824 0x009A6828 ?EcologyEventAccumulator@@3HA 0x004F67F0 member ?GameDataStateStorage@@3UGameDataStateWindow@@A fields_.ecology_event_accumulator_
// RECOVERY_OBJECT: v3 0x009A64A4 ?GameInterludeFlags@@3IA member ?GameDataStateStorage@@3UGameDataStateWindow@@A fields_.game_interlude_flags_
// RECOVERY_OBJECT: v3 0x009A64A0 0x009A64A4 ?GameWireOpaque_9A64A0@@3PAEA 0x00532E00 member ?GameDataStateStorage@@3UGameDataStateWindow@@A fields_.game_wire_opaque_9a64a0_
// RECOVERY_OBJECT: v3 0x009A64A8 0x009A64AC ?GameWireOpaque_9A64A8@@3PAEA 0x00532E00 member ?GameDataStateStorage@@3UGameDataStateWindow@@A fields_.game_wire_opaque_9a64a8_
// RECOVERY_OBJECT: v3 0x009A66C9 0x009A67C9 ?LegacyGameSaveName@@3PADA 0x005A6AC0 member ?GameDataStateStorage@@3UGameDataStateWindow@@A fields_.legacy_game_save_name_
// RECOVERY_OBJECT: v3 0x009A67C9 0x009A67CC ?GameWireOpaque_9A67C9@@3PAEA 0x00532E00 member ?GameDataStateStorage@@3UGameDataStateWindow@@A fields_.game_wire_opaque_9a67c9_
// RECOVERY_OBJECT: v3 0x009A67CC 0x009A67D0 ?ClimateRiseStep@@3HA 0x00524210 member ?GameDataStateStorage@@3UGameDataStateWindow@@A fields_.climate_rise_step_
// RECOVERY_OBJECT: v3 0x009A67D0 0x009A67D4 ?ClimateRiseLimit@@3HA 0x00524210 member ?GameDataStateStorage@@3UGameDataStateWindow@@A fields_.climate_rise_limit_
// RECOVERY_OBJECT: v3 0x009A67D4 0x009A67D8 ?ClimateRiseAccum@@3HA 0x00524210 member ?GameDataStateStorage@@3UGameDataStateWindow@@A fields_.climate_rise_accum_
// RECOVERY_OBJECT: v3 0x009A67D8 0x009A67DC ?ClimateSeaLevelDelta@@3HA 0x00524210 member ?GameDataStateStorage@@3UGameDataStateWindow@@A fields_.climate_sea_level_delta_
// RECOVERY_OBJECT: v3 0x009A6670 0x009A66C9 ?GameTechAchieved@@3PAEA 0x004DFAD0 member ?GameDataStateStorage@@3UGameDataStateWindow@@A fields_.game_tech_achieved_
// RECOVERY_OBJECT: v3 0x009A64BC ?TutWinFlags9A64BC@@3HA member ?GameDataStateStorage@@3UGameDataStateWindow@@A fields_.tutwin_flags_9a64bc_
// RECOVERY_OBJECT: v3 0x009A64DC 0x009A64E0 ?VehSelectionFollowLimit@@3HA 0x005C08C0 member ?GameDataStateStorage@@3UGameDataStateWindow@@A fields_.vehicle_selection_follow_limit_
// game_data transfers exactly 0x398 bytes from 0x009A6490 (0x005A6C24..
// 0x005A6C9A), ending where the separately bounded Proposal table begins.
// RECOVERY_OBJECT: v2 0x009A6490 0x009A6828 ?GameDataStateStorage@@3UGameDataStateWindow@@A 0x005A6AC0
GameDataStateWindow GameDataStateStorage;

uint32_t SystemInterfaceFlags;  // 0x00933420


// Original441250 clears four words in each of64 cells, preserving the
// payload and the flat/transposed write order. Cell stride is90 bytes.
// RECOVERY_OBJECT: v2 0x007388D8 0x0073ACD8 ?OfferResetCells@@3PAY07UOfferResetCell@@A 0x00441250


// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00441250 ?clear_all_offers@@YAXXZ 0x00441250-0x00441296 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned441250 writes four words at offsets0/4/8/12 in90-byte cells over8x8 faction pairs. Source-extracted tracked assignments exercise exact interleaved flat/transposed order."], "reason": "Recover complete offer reset using genuine64-cell storage and typed fields while preserving flat/transposed write ordering and untouched payload. No wider offer semantics are invented."}
void __cdecl clear_all_offers() {
    for (int row = 0; row < MaxPlayerNum; ++row) {
        for (int column = 0; column < MaxPlayerNum; ++column) {
            OfferResetCells[row][column].count_[0] = 0;
            OfferResetCells[row][column].count_[1] = 0;
            OfferResetCells[column][row].count_[0] = 0;
            OfferResetCells[column][row].count_[1] = 0;
            OfferResetCells[row][column].state_[0] = 0;
            OfferResetCells[row][column].state_[1] = 0;
            OfferResetCells[column][row].state_[0] = 0;
            OfferResetCells[column][row].state_[1] = 0;
        }
    }
}

// 0x009A6494
// 0x009A6498
// 0x009A649C
// 0x009A64C0
// 0x009A64C4
// 0x009A64D4
// setup_game clears this independent DWORD after BaseCurrentCount. No
// additional semantic use is established; it is not part of the base count.
// Historical object identity moved to GameDataStateStorage; see the transport manifest.

// set_time_controls publishes its selected seconds multiplied by 1000.
// net_control_turn copies the duration to the distinct live wait accumulator.
// Historical object identity moved to GameDataStateStorage; see the transport manifest.

// RECOVERY_OBJECT: v3 0x0094B4D8 ?GameWireOpaque_94B4D8@@3PAEA member ?ScenarioSave@@3UScenarioSaveState@@A wire_opaque_94b4d8
ScenarioSaveState ScenarioSave;
typedef char ScenarioSaveStateHasNativeExtent[
    sizeof(ScenarioSaveState) == 0x98 ? 1 : -1];
typedef char ScenarioSaveReqOffset[offsetof(ScenarioSaveState, objective_req_victory) == 0x00 ? 1 : -1];
typedef char ScenarioSaveSuddenDeathOffset[offsetof(ScenarioSaveState, objectives_sudden_death_victory) == 0x04 ? 1 : -1];
typedef char ScenarioSaveObjectivePtsOffset[offsetof(ScenarioSaveState, objective_achieve_points) == 0x08 ? 1 : -1];
typedef char ScenarioSaveVictoryBonusOffset[offsetof(ScenarioSaveState, victory_achieve_bonus_points) == 0x0C ? 1 : -1];
typedef char ScenarioSaveStartingYearOffset[offsetof(ScenarioSaveState, starting_mission_year) == 0x10 ? 1 : -1];
typedef char ScenarioSaveEndingYearOffset[offsetof(ScenarioSaveState, ending_mission_year) == 0x14 ? 1 : -1];
typedef char ScenarioSaveTailOffset[offsetof(ScenarioSaveState, wire_opaque_94b4d8) == 0x18 ? 1 : -1];
// 0x009A64D8
// 0x009A6800
// config_popups copies the live screen dimensions to these independent
// dwords; both are beyond Console's proven end, not Console members.
// RECOVERY_OBJECT: v1 0x00939E58 ?IfaceScreenHeight@@3HA
int IfaceScreenHeight; // 0x00939E58
// RECOVERY_OBJECT: v1 0x00939E5C ?IfaceScreenWidth@@3HA
int IfaceScreenWidth;  // 0x00939E5C
// Original file-backed palette defaults; MessageWin owns the interleaved
// text/background cells at 0x68A5A4/B0 separately.
// RECOVERY_OBJECT: v1 0x0068A5A0 ?PopupThemeWindowColor@@3HA
int PopupThemeWindowColor = 0x62; // 0x0068A5A0
// RECOVERY_OBJECT: v1 0x0068A5A8 ?PopupThemeButtonColor@@3HA
int PopupThemeButtonColor = 0xE8; // 0x0068A5A8
// RECOVERY_OBJECT: v1 0x0068A5AC ?PopupThemeTextShadow@@3HA
int PopupThemeTextShadow = 0xE9; // 0x0068A5AC
// RECOVERY_OBJECT: v1 0x0068A5B8 ?PopupThemeSurfaceColor@@3HA
int PopupThemeSurfaceColor = 0x24; // 0x0068A5B8
// RECOVERY_OBJECT: v1 0x0068A5BC ?PopupThemeBevelUpper@@3HA
int PopupThemeBevelUpper = 0x1E; // 0x0068A5BC
// RECOVERY_OBJECT: v1 0x0068A5C0 ?PopupThemeBevelLower@@3HA
int PopupThemeBevelLower = 0x56; // 0x0068A5C0

// Original455610..455E41. The explicit mode argument is unused. All service
// results are ignored in the original; failed loads still proceed through
// extraction/publication. The existing asset objects own their allocations.
// Superseded seed deleted; seed paths below are historical Git provenance.
// Historical seed observations (not fresh measurements or acceptance):
// TRIED: full transcription landed (MISMATCH #0, prologue push/sub divergence - likely stack-frame shape from the local NCSprites-array/struct layout differing from the original's). Verified every extract()/change_color() argument order and the `this` receiver for the two loop-driven extract() sequences against raw bytes, since Ghidra silently drops the implicit `this` on every extract() call in its pseudocode.
// 0x009A6804
// 0x009A6808
// 0x009A680C
// save_daemon writes 12 before serialization; setup_game clears this word.
// Its precise save-format meaning is not established by that write alone.
// Historical object identity moved to GameDataStateStorage; see the transport manifest.

// Original4838E3 reads this DWORD network-game flag; initial image storage is0.
// RECOVERY_OBJECT: v2 0x0093F660 0x0093F664 ?IsMultiplayerNet@@3HA 0x004838E0
BOOL IsMultiplayerNet;  // DirectPlay - Serial, Modem, Internet (TCP/IP)
// Original 0x00442721/0x004713B3 test the shared PBEM mode value, not a pointer.
// RECOVERY_OBJECT: v2 0x0093A95C 0x0093A960 ?IsMultiplayerPBEM@@3HA 0x00405276
BOOL IsMultiplayerPBEM;  // 0x0093A95C // HotSeat / PBEM
// 0x009A681C
// 0x009A6820

// Shared turn handoff flag: next_player_turn and player deletion write it;
// net_not_my_turn reads it. The observed four bytes start loader-zeroed.
// RECOVERY_OBJECT: v2 0x0093A950 0x0093A954 ?NextTurnReady@@3HA 0x0052DBF0
int NextTurnReady;
// net_upkeep resets/increments this phase and net_upkeep_phase consumes it.
// Its producer is still unfinished; this storage does not recover that body.
// RECOVERY_OBJECT: v2 0x0093A93C 0x0093A940 ?NetUpkeepPhase@@3HA 0x005290A0
int NetUpkeepPhase;



// ---------------------------------------------------------------------------
// Merged from the retired src/game_recovery.cpp on 2026-08-20, following
// src/text_recovery.cpp, which went into text.cpp on 2026-08-15. A class's
// bodies in two translation units is not a neutral choice: which unit a
// piece compiles in decides what can be inlined into it.
// ---------------------------------------------------------------------------

/*
Purpose: The ceiling on a single energy allocation slider. The legacy
         implementation is a constant return.
// ORIGINAL: 0x00445440 ?energy_limit@@YAHH@Z 0x00445440-0x00445446 BYTE_EXACT
// body      src/game.cpp
// size      6 bytes
// prototype int (__cdecl ?energy_limit@@YAHH@Z)(int factionID)
// callers   3   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 10
Status: Complete
*/
// HISTORICAL: BODY IN game.h, as `MEASURED inline`: the image writes it out at
// HISTORICAL: some call sites and calls it at others, and a .cpp definition is only ever
// HISTORICAL: one of those. The marker stays here because that is where the catalogue
// HISTORICAL: reads it.
MEASURED int __cdecl energy_limit(int) {
    return 10;
}


// ---------------------------------------------------------------------------
// The main loop.
// ---------------------------------------------------------------------------

// Loop state globals; addresses and roles documented in
// game.h next to each declaration.
// RECOVERY_OBJECT: v1 0x0093A948 ?GameRestartQueued@@3HA
int GameRestartQueued;  // 0x0093A948
// Original dword stop-loop flag, set by system-close and read by wait paths.
// RECOVERY_OBJECT: v2 0x009B2068 0x009B206C ?ExitTurnLoop@@3HA 0x004050C7
int ExitTurnLoop;       // 0x009B2068
// Earlier recovery interpreted 0x009A6504/0x009A6508 as independent
// EndgameFactionSelectorA/B scalars. They are FactionRankings[6]/[7],
// owned in alpha.cpp: rankings writes all eight slots, and setup_player
// reads these slots at 0x005B231F/0x005B2300. Keep the historical
// interpretation here; separate globals would sever producer/consumer state.
// RECOVERY_OBJECT: v2 0x009B206C 0x009B2070 ?EndgameExitReason@@3HA 0x004F2FFE
int EndgameExitReason;
// Original interlude reads/writes this complete story-mask word.
// Historical object identity moved to GameDataStateStorage; see the transport manifest.

// Wire0 and game_data transfer these exact independent byte intervals.
// No direct semantic use or original allocation boundary is proved; they are
// retained state, never padding or zero-producing packet filler. Keep the
// filename tail separate from the exactly bounded legacy 256-byte copy.
// Historical object identity moved to GameDataStateStorage; see the transport manifest.

// Historical object identity moved to GameDataStateStorage; see the transport manifest.

// Native game_data5A6BA3..5A6BB9 copies exactly256 bytes; setup_game
// clears only byte0. NUL-scanning consumers do not prove loaded termination.
// Historical object identity moved to GameDataStateStorage; see the transport manifest.

// Historical object identity moved to GameDataStateStorage; see the transport manifest.

// Logical faction0..7 indexing is independently observed; original
// containing allocation bounds are not asserted.
// RECOVERY_OBJECT: v1 0x00946138 ?TectonicDetonationCount@@3PAIA
uint32_t TectonicDetonationCount[8];
// Original691B0C contains pointer691B7C, whose file-backed text is SCRIPT.
// This owns the observed text and shared pointer cell.
// Bound: the original bytes are "SCRIPT" and its NUL; one zero pad byte precedes the next string, "EDITOR", at 0x00691B84.
// RECOVERY_OBJECT: v2 0x00691B7C 0x00691B83 ?ScriptPopupFileText@@3PADA 0x00406384
char ScriptPopupFileText[] = "SCRIPT";
// Bound: pointer cell; the next dword 0x00691B10 is a separate pointer (to "EDITOR" at 0x00691B84).
// RECOVERY_OBJECT: v2 0x00691B0C 0x00691B10 ?ScriptPopupFile@@3PADA 0x00406384
char *ScriptPopupFile = ScriptPopupFileText;
// human_turn514263 writes this independent dword; draw_bases4671B5 reads it.
// Loader-zero storage ends before the separately owned GameSessionActive.
// RECOVERY_OBJECT: v2 0x009B2070 0x009B2074 ?ControlTurnPhase@@3HA 0x00514050
int ControlTurnPhase;
// net_upkeep5290D0/5292BD publish phases1/2; tech_advance5BE580 and
// tech_research5BEB3A read this distinct shared dword. Original virtual-tail
// storage is loader-zero; fresh compiler objects establish one int owner.
// Remaining net_upkeep recovery must publish to this owner, not an old address.
// RECOVERY_OBJECT: v2 0x00945F48 0x00945F4C ?TurnUpkeepPhase@@3HA 0x005BE940
int TurnUpkeepPhase = 0;
// Native control_game reads only this pinned NUL-terminated source string;
// one byte is the observed lower bound, not its complete original capacity.
char SaveGameFileName[1];  // 0x0093AA0C
// Original serializer transfers two adjacent, independent256-byte records.
// RECOVERY_OBJECT: v2 0x009B2078 0x009B2178 ?SaveNameBuffer@@3PADA 0x005A6AC0
char SaveNameBuffer[256];
// RECOVERY_OBJECT: v2 0x009B2178 0x009B2278 ?ScenarioSaveNameBuffer@@3PADA 0x005A6AC0
char ScenarioSaveNameBuffer[256];

// The window the loop paints colour 9 over and hides while the game boots,
// showing it again for a PBEM session, is ConsoleGlobal.cover_window_: the
// GraphicWin ??0Console builds at +0x21A68 (0x009156B0 + 0x21A68 =
// 0x00937118). The image names it with `mov ecx, 0x937118` immediates; the
// member path keeps that fold. Its historical CoverWindow identity is
// retained beside ConsoleGlobal in console.cpp.

// The multiplayer window - REAL STORAGE at the image's 0x007FD648, not the
// `GraphicWin *const MultiWindow` binding (see game.h). Its class is
// MultiWin. The source now uses its real C++ constructor and destructor;
// their placement in the original initializer/finalizer sequence is still
// an independent restoration obligation.
// Its 0x28B8-byte owned layout ends at the independently catalogued
// g_MULTIWIN_WAVE pointer at 0x007FFF00. The game object file emits
// ?MultiWindow@@3VMultiWin@@A with real constructor/destructor references.
// RECOVERY_OBJECT: v2 0x007FD648 0x007FFF00 ?MultiWindow@@3VMultiWin@@A 0x0047A590
// This cell was misrepresented as standalone ?MultiWindowOpen; it is the
// existing open-state member, retaining the image's loader-zero initial value.
// RECOVERY_OBJECT: v3 0x007FE06C ?MultiWindowOpen@@3HA member ?MultiWindow@@3VMultiWin@@A panel_open_
MultiWin MultiWindow;  // 0x007FD648

// ---------------------------------------------------------------------------
// The teardown half of control_game's wheel, homed 2026-08-29 from
// src/recovered/units/ (0058eff0, 00589b20, 0058f430). Their forwarders in
// pending_bodies.cpp are gone; these definitions own the symbols now.
// ---------------------------------------------------------------------------


/*
Purpose: Bring the shared system up, once per process: create the four data
         directories, read the language preference and load the string and
         label tables, upper-case the two default button captions out of them,
         load the fixed art, then build the eight map-window slots and the
         desktop's standing windows.
// ORIGINAL: 0x0058F040 ?system_init@@YAHXZ 0x0058F040-0x0058F245;0x00660BD7-0x00660BEC BYTE_EXACT
// size      538 bytes
// prototype int (__cdecl ?system_init@@YAHXZ)(void)
// callers   1   call targets   23
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004219E0 0x00429300 0x00455E80 0x004595C0 0x00459680 0x004626E0 0x00471D50 0x004BA220 0x0051D890 0x00588AA0 0x0059DB40 0x005C9860 0x006018A0 0x00601910 0x006168F0 0x006169A0 0x00616A00 0x00626450 0x00627100 0x00645470 0x0064557F 0x0064558A 0x00646829
// indirect  0x0058F11A 0x0058F159
// LEVER: the SEH frame the archive unit could not reproduce is the
//   `new MapWin` below it. MapWin has a real destructor (mapwin.h), so /GX
//   makes VC6 wrap the whole function - `push -1 / push handler / fs:[0]` at
//   instruction 0, the `[ebp-4]` state toggle around the constructor and the
//   `operator delete` funclet at 0x660BD7 - which is exactly the frame the
//   artifact's plain operator-new-plus-placement-new spelling was missing.
//   `new MapWin` with no argument is also what produces the image's
//   `push 1` before the ctor call: that 1 is the hidden most-derived flag,
//   not the catalogue's `H`.
// LEVER: the two captions are `label_get` (alpha.h's forceinline of
//   Strings::get over Labels.strings_ptr) at offsets 102 and 101, which is the
//   image's `[strings_ptr + 0x198]` and `[+ 0x194]`.
// LEVER: slot 0 of the map-window table is published last, as the Console -
//   the image's single `mov dword ptr [0x7d3c3c], 0x9156b0` - and the seven
//   slots above it are the ones the allocation loop walks (0x7D3C40 to
//   0x7D3C5C exclusive), so the loop is `&MapWinTable[1] .. &MapWinTable[8]`
//   and not the whole table.
// Historical constraint below is superseded: InterfaceMessageWin now owns
// original 0x7F67F8, and WinBubbleWindow has its genuine MultiDebug owner.
// system_init uses those source objects; no fixed receiver is needed here.
// TRIED: `static_cast<MultiDebug *>(&WinBubbleWindow)` and a named extern for
//   the MessageWin storage - the image has no MessageWin object in this tree
//   (messagewin.cpp defines nothing at 0x7F67F8) and WinBubbleWindow is a
//   `Win`, so both stay address spellings, the way game_reload reaches
//   0x008E9F60.
// The shared graphics-language API is separately compiled: the original
// calls set_language at 0x0058F0A1, rather than inlining its global store.
// TRIED: the two caption builds spelled `label_get(102)` / `label_get(101)`
//   instead of the local `LPSTR *labels` plus `StringTable.get(labels[n])` now
//   in the body - that spelling scores 32/135 against this one's 34, and
//   game_init measures the same either way, so the two bodies legitimately
//   spell it differently. What is left is the register the label value lands
//   in and the register holding `Labels.strings_ptr` across the StringTemp
//   store (image eax/eax, ours eax/ecx on the first caption), plus the
//   `SystemInterfaceFlags` read-modify-write, which the image interleaves with
//   maininterface_init's argument setup and runs in esi while ours runs it as
//   a block in edx and re-derives the zero twice. All register assignment;
//   /Ob0 scores worse (53/135) because map_win_table and friends turn into
//   real calls.
Return Value: 1 when the label table fails to load, else 0.
*/
int __cdecl system_init() {
    _mkdir("saves");
    _mkdir("saves\\auto");
    _mkdir("maps");
    _mkdir("scenarios");
    WorldClimateSkipTerrainClear = 1;
    log_set_state(0);
    set_language(prefs_get("Language", 0, 0));
    textcolor_init();
    StringTable.init(0xC320);
    if (labels_init() != 0) {
        return 1;
    }

    // The Cancel caption, label 102: upper-cased unless the language is 2.
    LPSTR *labels = Labels.strings_ptr;
    StringTemp[0] = '\0';
    strcat(StringTemp, StringTable.get(labels[102]));
    LPSTR (WINAPI *string_upper)(LPSTR) = CharUpperA;
    if (Language != 2) {
        string_upper(StringTemp);
    }
    BasePop::set_def_cancel_text(StringTemp);

    // The OK caption, label 101: upper-cased whatever the language.
    StringTemp[0] = '\0';
    strcat(StringTemp, label_get(101));
    string_upper(StringTemp);
    BasePop::set_def_ok_text(StringTemp);

    load_fixed_sprites();
    config_popups();

    // Construct only slots 1..7, publishing each after construction. Keep
    // the genuine new-expression so failed construction releases its storage;
    // slot zero is published separately below as the existing Console.
    for (int slot = 1; slot < static_cast<int>(MapWinTableSlots); ++slot) {
        MapWinTable[slot] = new MapWin;
    }

    MapWinTable[0] = &ConsoleGlobal;
    SystemInterfaceFlags &= ~0x8C0u;

    MainInterfaceGlobal.init(0);
    g_BattleWin.init();
    ConsoleGlobal.init(0);

    // The language performs the Console-to-Win virtual-base conversion.
    Win *console_window = &ConsoleGlobal;
    WinBubbleWindow.init(&NetDaemonNet, console_window, ShadowColorTable);

    InterfaceMessageWin.init();
    DatalinkMain.init();
    bung_momma();
    return 0;
}

// The desktop coordinator calls each real source-owned window and publishes
// its four bounds in the existing Console storage used by blink_timer and
// the Console splitter. Original owner TU remains unknown; game.cpp is the
// current home because this is the startup coordinator.
// ORIGINAL: 0x0058EE60 ?desktop_init@@YAHH@Z 0x0058EE60-0x0058EFEB BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:b84036a86445148d25d4e216ac24c131e108bea8:src/unrecovered/0058ee60.cpp", "Pinned58EE60..58EFEB bounds939430..43C are Console+23D80..8C; DIB width/height come from the Console virtual Buffer base, not detached globals.", "Original startup interfaces, multiplayer/PBEM branches and two non-input pumps; OpeningWindow is deleted through its actual GraphicWin owner.", "recovery/tasks/startup-desktop-owner-oct5.json", "recovery/tests/test_startup_actual_source.py"], "reason": "Complete startup coordination through real owned windows and Console bounds; preserve the original calls, stores, redraw/cover ordering and OpeningWindow lifetime. Current source home is game.cpp, not a claim about original translation-unit ownership."}
// kind      game
// TRIED: none confirmed byte-exact; MISMATCH #5 push-count differs - the original sets up an ebp frame (push ebp/mov ebp,esp) that this body's register pressure does not need, so /O2 drops it.
int __cdecl desktop_init(int fresh) {
    load_music(ConsoleGlobal.faction_id_);
    ConsoleInfoWin.init();
    WorldClimateWorldWin.init(fresh);
    ConsoleStatusWin.init();
    mapwin_system_init(fresh);
    MultiWindow.init();
    TutWinBaseWindow.init();
    TutWinDesWindow.init();
    DiplomacyPopup.init();
    ReportIfGlobal.init();
    ConsolePrefWin.init();

    // Both native stores survive: the virtual-base width read sits between
    // them, so VC6 cannot drop the first one.
    ConsoleGlobal.field_23D80_ = 10;
    ConsoleGlobal.field_23D84_ = ConsoleGlobal.dib_.bmiHeader.biWidth - 10;
    ConsoleGlobal.field_23D80_ = ConsoleGlobal.alpha_menu_.client_rect().right
        - ConsoleGlobal.alpha_menu_.client_rect().left + 10;
    if (IsMultiplayerNet) {
        ConsoleGlobal.field_23D84_ += MultiWindow.client_rect().left
            - MultiWindow.client_rect().right;
    }
    ConsoleGlobal.field_23D88_ = 10;
    ConsoleGlobal.field_23D8C_ = -10 - ConsoleGlobal.dib_.bmiHeader.biHeight;

    if (GameDataStateStorage.fields_.turn_current_num_) {
        draw_map(1);
    }

    MainInterfaceGlobal.Win::show(0);
    ConsoleGlobal.Win::show(0);
    if (IsMultiplayerPBEM) {
        ConsoleGlobal.cover_window_.GraphicWin::fill(0);
        ConsoleGlobal.cover_window_.Win::show(0);
    }
    do_all_non_input();

    // A named conversion keeps the null-checked value in one register and
    // stops VC6 caching zero in ESI for the whole function.
    Win *default_parent = static_cast<Win *>(&ConsoleGlobal);
    BasePopDefaultField322C = default_parent;
    if (OpeningWindow != 0) {
        delete OpeningWindow;
        OpeningWindow = 0;
    }
    do_all_non_input();
    return 0;
}


/*
Purpose: Bring a game up: reseed both random chains off the clock, then either
         just turn the log off (mode 0) or reload the string table and
         re-read the two default button captions before the art and the rules
         load. mode 0 is the first boot, anything else a reload into a live
         system.
// ORIGINAL: 0x0058F2F0 ?game_init@@YAHHH@Z 0x0058F2F0-0x0058F421 BYTE_EXACT
// size      305 bytes
// prototype BOOL (__cdecl ?game_init@@YAHHH@Z)(BOOL tglParseStrings, BOOL tglAllRules)
// callers   4   call targets   12
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00455EB0 0x0050F3D0 0x005873C0 0x006018A0 0x00601910 0x006168F0 0x006169A0 0x00616A00 0x006257E0 0x00626450 0x00645470
// indirect  0x0058F304 0x0058F30F 0x0058F38D 0x0058F3CD
// LEVER: the clock slot is read ONCE into a register and called twice, the
//   same shape control_game's GetAsyncKeyState lever records - holding
//   WaveTimeGetTimeSlot in a local is what keeps the load from happening
//   twice.
// LEVER: the `add esp,4` at 0x58F3D9 is one instruction serving BOTH arms -
//   log_set_state's argument on the mode==0 arm, set_def_ok_text's on the
//   other - so the source converges the two arms into one `if/else` whose
//   join the compiler cleans once.
// TRIED: naming the three caption-icon stores through sprite.cpp's array -
//   sprite.h does not declare it, and game.h cannot add a member to a header
//   it does not own, so the three store their addresses (see WinCloseIcon in
//   game.h for the identification).
// TRIED: the two caption builds spelled `LPSTR *labels` and
//   `StringTable.get(labels[n])` instead of label_get - byte-identical output,
//   so it is a free spelling, not a lever. The six runs that remain are the
//   register the label value lands in (image eax, ours ecx on the first
//   caption), the register holding `Labels.strings_ptr` for the second caption
//   (image ecx, ours edx), and the register `reload` is read into for
//   read_rules (image edx, ours ecx). Register naming only; the instruction
//   count and the call set agree exactly.
Return Value: 1 when the label table or the rules fail to load, else 0.
Status: homed from src/unrecovered/0058f2f0.cpp on this pass; its pending
        forwarder is gone. 74/82 agreeing, best set /O2 /Gy /GR- /Oy- /GX,
        156 bytes discounted as relocations.
*/
int __cdecl game_init(int mode, int reload) {
    DWORD (WINAPI *const time_get_time)() = timeGetTime;
    WorldClimateSkipTerrainClear = 1;
    my_srand(time_get_time());
    random_reseed(time_get_time());

    if (mode == 0) {
        log_set_state(0);
    } else {
        StringTable.init(0xC320);
        if (labels_init() != 0) {
            return 1;
        }
        // The same two captions system_init builds, for the same two buttons.
        StringTemp[0] = '\0';
        strcat(StringTemp, label_get(102));
        LPSTR (WINAPI *string_upper)(LPSTR) = CharUpperA;
        if (Language != 2) {
            string_upper(StringTemp);
        }
        BasePop::set_def_cancel_text(StringTemp);

        StringTemp[0] = '\0';
        strcat(StringTemp, label_get(101));
        string_upper(StringTemp);
        BasePop::set_def_ok_text(StringTemp);
    }

    load_sprites();
    if (read_rules(reload) != 0) {
        return 1;
    }
    start_timers();

    // The three caption-button icons: sprite.cpp's array at 0x0075AE20, a 0x2c
    // stride, so these are its [1], [2] and [0]. WinRestoreIcon, the slot
    // between the last two, is not set here.
    WinCloseIcon = &g_SILVER_MENU_ICON_SPRITES[1];
    WinMaximizeIcon = &g_SILVER_MENU_ICON_SPRITES[2];
    WinMinimizeIcon = &g_SILVER_MENU_ICON_SPRITES[0];
    return 0;
}

// Original 0x00455400: palette cycles and ocean offsets are one ordered
// configuration load. A missing cycling section also skips ocean offsets;
// initialization errors do not suppress the following start_cycle call.
// Original section-name strings include their terminating NULs. The two
// "system" arguments instead use distinct original copies at 685150 and
// 685168; a pooled compiler literal is not one shared original identity.
// Construct and destroy the fixed-art loader's genuine local Buffer.
/*
Purpose: Restart the game state: tear the desktop down (mode-gated, the same
         closer sequence desktop_close runs), stop the timers, drop the label
         and string tables, re-init the game, and re-init the desktop when the
         caller asked for it. Always clears the climate skip flag.
// ORIGINAL: 0x0058F450 ?game_reload@@YAHHH@Z 0x0058F450-0x0058F4E7 BYTE_EXACT
// LEVER (2026-09-03, pair normalisation): the same two forwarders game_close
//   uses - the image CALLS 0x0050F440 and 0x006169D0 in this body too, and
//   with the `MEASURED inline` helpers folded in it was 107 compiled
//   instructions against the image's 44, 12 of 44. 44 of 44 after the switch.
// LEVER: byte-exact on promotion from the 0058f450 archive unit. The
//   closer block is desktop_close's own sequence spelled the same way, so
//   every receiver folds to its `mov ecx, imm32` and every call lands on the
//   image's E8 target; `mode` is read into esi once at entry (the image's
//   `mov esi, [ebp + 8]`) because the source uses it twice - the opening
//   guard and the desktop_init guard - exactly as the image does.
// size      151 bytes
// prototype int (__cdecl ?game_reload@@YAHHH@Z)(int, BOOL tglAllRules)
// callers   2   call targets   11
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00408710 0x0043C1A0 0x004710E0 0x004B9F80 0x0050F440 0x0058EE60 0x0058F2F0 0x005D4E40 0x005EDCD0 0x00616950 0x006169D0
Return Value: 1 when game_init fails, else 0.
Status: Complete
*/
// control_game calls this out of line.
#pragma auto_inline(off)
int __cdecl game_reload(int mode, int reload) {
    if (mode != 0) {
        TutWinDesWindow.close();
        TutWinBaseWindow.close();
        mapwin_system_shutdown();
        ConsoleStatusWin.close();
        // The world-climate window, named through worldwin.h's declaration.
        WorldClimateWorldWin.GraphicWin::close();
        MultiWindow.GraphicWin::close();
        MainInterfaceGlobal.Win::hide();
    }
    stop_timers();
    labels_shutdown();
    StringTable.shutdown();
    if (game_init(1, reload) != 0) {
        return 1;
    }
    if (mode != 0) {
        desktop_init(1);
    }
    WorldClimateSkipTerrainClear = 0;
    return 0;
}
#pragma auto_inline(on)




// ORIGINAL: 0x0058F4F0 sub_58f4f0 0x0058F4F0-0x0058F609 FILE BYTE_EXACT
// symbol    _sub_58f4f0
// LEVER: string-routines-are-calls the body was already right; the scaffold was compiling a string routine the image CALLS into an `/Oi` inline expansion. What was written down here as a tried source form - "VC6 lowers `local_104[0]=0;` as `and byte ptr [buf],0` rather than `mov`, and each branch shares one `lea` instead of recomputing it, both compiler instruction-selection choices at /O1 that plain restatement did not move" - was that expansion changing the register pressure around them. The if/else-if chain matching Ghidra's structure is the right shape and always was.
// working copy - scaffold materialised by --work
// size      281 bytes
// prototype
// callers   0   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00625880 0x00645470

// Historical candidate note: the const-pointer spelling reproduced the old
// fixed-address encoding; it is not the current source-owned token storage.
// The original reads these eight short markup strings from 0x0068FA70..8C.
// Keep source-owned semantic constants, not aliases to executable storage.
// The complete original allocation/pooling extent is unknown.
// RECOVERY_OBJECT: v1 0x0068FA70 _SayTokOpen
static const char SayTokOpen[] = "$<";
// RECOVERY_OBJECT: v1 0x0068FA74 _SayTokMale
static const char SayTokMale[] = "M";
// RECOVERY_OBJECT: v1 0x0068FA78 _SayTokFemale
static const char SayTokFemale[] = "F";
// RECOVERY_OBJECT: v1 0x0068FA7C _SayTokNeutral
static const char SayTokNeutral[] = "N";
// RECOVERY_OBJECT: v1 0x0068FA80 _SayTokOne
static const char SayTokOne[] = "1";
// RECOVERY_OBJECT: v1 0x0068FA84 _SayTokTwo
static const char SayTokTwo[] = "2";
// RECOVERY_OBJECT: v1 0x0068FA88 _SayTokSep
static const char SayTokSep[] = ":";
// RECOVERY_OBJECT: v1 0x0068FA8C _SayTokClose
static const char SayTokClose[] = ">";

extern "C" char * __cdecl sub_58f4f0(char *a1, int a2, int a3, int a4) {
    char local_204[256];
    char local_104[256];

    if (a3 < 0) {
        a3 = PluralityDefault;
    }
    if (a4 < 0) {
        a4 = GenderDefault;
    }
    local_104[0] = 0;
    strcat(local_104, SayTokOpen);
    if (a4 == 0) {
        strcat(local_104, SayTokMale);
    } else if (a4 == 1) {
        strcat(local_104, SayTokFemale);
    } else if (a4 == 2) {
        strcat(local_104, SayTokNeutral);
    }
    if (a3 == 0) {
        strcat(local_104, SayTokOne);
    } else if (a3 == 1) {
        strcat(local_104, SayTokTwo);
    }
    strcat(local_104, SayTokSep);
    strcat(local_104, Players[a2].adj_name_faction);
    strcat(local_104, SayTokClose);
    parse_string(local_104, local_204);
    return strcat(a1, local_204);
}

// ORIGINAL: 0x0058F700 ?say_fac_special@@YAXPADPADH@Z 0x0058F700-0x0058F80A FILE BYTE_EXACT
// symbol    ?say_fac_special@@YAXPAD0H@Z
// size      266 bytes
// prototype
// callers   1   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00625880 0x00645470
// PRESERVED UNIT - measured BYTE_EXACT.
//
// The following PRESERVED UNIT / GENERATED SKELETON prose records an earlier
// address-bound verification artifact, not current product-source placement.
// Keep its historical compiler and measurement findings intact.
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0058F700
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0058f700/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?say_fac_special@@YAXPADPADH@Z  at 0x0058F700  (266 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// The VC6 dialect limits and the source-form rules used to live here.
// They are knowledge, not scaffolding, so they now live in the agent
// system prompt (mizuchi.yaml, plugins.claude-runner.systemPrompt),
// where they can be edited without regenerating anything and are in
// context from the first token rather than behind a file read. This
// emitter computes declarations; it does not carry lessons.

// Historical candidate note: the const-pointer spelling reproduced the old
// fixed-address encoding; it is not the current source-owned token storage.
// This is a distinct original token set at 0x0068FAB0..CC. Its bytes match
// SayTok's strings, but no shared original allocation/pooling is established.
// RECOVERY_OBJECT: v1 0x0068FAB0 _SayFacTokOpen
static const char SayFacTokOpen[] = "$<";
// RECOVERY_OBJECT: v1 0x0068FAB4 _SayFacTokMale
static const char SayFacTokMale[] = "M";
// RECOVERY_OBJECT: v1 0x0068FAB8 _SayFacTokFemale
static const char SayFacTokFemale[] = "F";
// RECOVERY_OBJECT: v1 0x0068FABC _SayFacTokNeutral
static const char SayFacTokNeutral[] = "N";
// RECOVERY_OBJECT: v1 0x0068FAC0 _SayFacTokOne
static const char SayFacTokOne[] = "1";
// RECOVERY_OBJECT: v1 0x0068FAC4 _SayFacTokTwo
static const char SayFacTokTwo[] = "2";
// RECOVERY_OBJECT: v1 0x0068FAC8 _SayFacTokSep
static const char SayFacTokSep[] = ":";
// RECOVERY_OBJECT: v1 0x0068FACC _SayFacTokClose
static const char SayFacTokClose[] = ">";

// Historical source-form finding for the former indexed fixed-address table
// bases: see "the lever that keeps working" above; current token constants are
// distinct source-owned strings, not indexed table views.

void __cdecl say_fac_special(char *a1, char *a2, int a3) {
    char buf1[256];
    char buf2[256];

    int table_b = Players[a3].is_noun_plural;
    int table_a = Players[a3].noun_gender;

    buf1[0] = 0;
    strcat(buf1, SayFacTokOpen);

    if (table_a == 0) {
        strcat(buf1, SayFacTokMale);
    } else if (table_a == 1) {
        strcat(buf1, SayFacTokFemale);
    } else if (table_a == 2) {
        strcat(buf1, SayFacTokNeutral);
    }

    if (table_b == 0) {
        strcat(buf1, SayFacTokOne);
    } else if (table_b == 1) {
        strcat(buf1, SayFacTokTwo);
    }

    strcat(buf1, SayFacTokSep);
    strcat(buf1, a2);
    strcat(buf1, SayFacTokClose);

    parse_string(buf1, buf2);
    strcat(a1, buf2);
}

// ORIGINAL: 0x0052DB40 sub_52db40 0x0052DB40-0x0052DB67 BYTE_EXACT
// symbol    ?reset_saved_process_slot_state@@YAXXZ
// size      39 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// 0x0052DB40  sub_52db40  ->  _sub_52db40
//
// The following notes describe the earlier verification artifact, when this
// identity had no product owner; current source uses NetDaemon's save view.
// Preserve the original measurement/history details below as such.
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.


void reset_saved_process_slot_state() {
    uintptr_t cursor = reinterpret_cast<uintptr_t>(NetDaemonNet.saved_process_slots_)
                     + offsetof(NetDaemonSaveProcessSlot, field_26_);
    int remaining = 8;
    do {
        *reinterpret_cast<int8_t *>(cursor -
            (offsetof(NetDaemonSaveProcessSlot, field_26_)
             - offsetof(NetDaemonSaveProcessSlot, identity_))) = -1;
        *reinterpret_cast<uint8_t *>(cursor - 1) = 0;
        *reinterpret_cast<uint8_t *>(cursor) = 0;
        *reinterpret_cast<int8_t *>(cursor + 1) = -1;
        *reinterpret_cast<uint8_t *>(cursor + 2) = 2;
        cursor += sizeof(NetDaemonSaveProcessSlot);
    } while (--remaining);
}

// ===== homed out of the archive 2026-09-04 =====

// ORIGINAL: 0x0058FA20 ?get_pacts@@YAPADH@Z 0x0058FA20-0x0058FA69 BYTE_EXACT
// LEVER: the carried bindings renamed to the tree's own models - Players
//         (faction.h, 0x00946A50), Labels (alpha.h, 0x009B90F8), StringTable
//         (strings.h, 0x009B90D8) and general.h's noun defaults - so no
//         anonymous g_ binding arrived with the body.
// TRIED: as `Players[a1].is_leader_female` for the load; the image reads it
//         as `[players + a1*0x59c]`, so the member form is the same operand.
// MISMATCH #10 'mov' vs 'mov': the image keeps `add edx, 0xcb` as its own
//         instruction before `mov ecx, [eax+edx*4]`, where every spelling
//         tried folds the 0xcb into the addressing displacement
//         (`[ecx+edx*4+0x32c]`) and hoists the `Labels.strings_ptr` load
//         above the store. Tried (as 0x0058F9D0, four spellings): inline
//         expression, a named boolean, a named `off` combining the constant
//         with the flag, and a `char *` base with a manual `*4` multiply -
//         all four byte-identical at this divergence.
char *__cdecl get_pacts(int faction_id) {
    PluralityDefault = 1;
    int noun = Players[faction_id].is_leader_female;
    GenderDefault = noun;
    return StringTable.get(Labels.strings_ptr[noun != 0 ? 0xcc : 0xcb]);
}

// ORIGINAL: 0x0058F9D0 ?get_pact@@YAPADH@Z 0x0058F9D0-0x0058FA19 BYTE_EXACT
// MISMATCH #3 'xor' vs 'mov': the original zeroes edx BEFORE storing
// PluralityDefault = 0, and every source order tried compiles that store
// first. A second divergence follows: the original keeps `add edx, 0xc9` as
// its own instruction before `mov ecx, [eax+edx*4]` where every spelling
// tried folds the 0xc9 into the addressing displacement. Tried: inline
// expression, a named boolean `flag`, a named `off` combining the constant
// with the flag, and a `char *` base with a manual `*4` multiply - all four
// byte-identical at the divergence, so it reads as an optimiser
// canonicalisation this call shape always hits.
// TRIED: homed out of the archive once its five fixed-address bindings were
//         all nameable from models the tree already holds (see get_pacts).
char *__cdecl get_pact(int faction_id) {
    PluralityDefault = 0;
    int noun = Players[faction_id].is_leader_female;
    GenderDefault = noun;
    return StringTable.get(Labels.strings_ptr[noun != 0 ? 0xca : 0xc9]);
}

// RECOVERY_LITERAL: v1 0x0068FAAC 0x0068FAAE ??_C@_01FAFK@?$DO?$AA@ 0x0058F610
// RECOVERY_LITERAL: v1 0x0068FAA0 0x0068FAA2 ??_C@_01FBAH@1?$AA@ 0x0058F610
// RECOVERY_LITERAL: v1 0x0068FA98 0x0068FA9A ??_C@_01FKLC@F?$AA@ 0x0058F610
// RECOVERY_LITERAL: v1 0x0068FA9C 0x0068FA9E ??_C@_01OAK@N?$AA@ 0x0058F610
// RECOVERY_LITERAL: v1 0x0068FA94 0x0068FA96 ??_C@_01PBLD@M?$AA@ 0x0058F610
// RECOVERY_LITERAL: v1 0x0068FAA8 0x0068FAAA ??_C@_01PKAG@?3?$AA@ 0x0058F610
// RECOVERY_LITERAL: v1 0x0068FA90 0x0068FA93 ??_C@_02IKIM@$?$DM?$AA@ 0x0058F610
// ORIGINAL: 0x0058F610 ?say_special@@YAXPADPADH@Z 0x0058F610-0x0058F6F6 BYTE_EXACT
// LEVER: the eight fixed-address bindings are .rdata STRING LITERALS, read
//         out of the image with image_data.py and spelled as the text, the
//         same way netdaemon.cpp's captions went: 0x0068FA90 "$<",
//         0x0068FA94 "M", 0x0068FA98 "F", 0x0068FA9C "N", 0x0068FAA0 "1",
//         0x0068FAA8 ":" and 0x0068FAAC ">". 0x0068FAA4 "2" is not read by
//         this body and is left for whoever homes its caller. The eight
//         together are the markup the parser consumes: a faction noun of
//         M/F/N, a noun form digit, then the substituted text.
// LEVER: the ninth binding is `Players` (faction.h, 0x00946A50); the image
//         indexes it `a3 * 0x167` dwords, which is `Players[a3]` - the
//         stride IS sizeof(Player).
// symbol   ?say_special@@YAXPAD0H@Z
//         - this tree's name for the same (char *, char *, int) signature.
//         VC6 folds the two identical `char *` parameters into a
//         back-reference, where the catalogued name spells the second `PAD`
//         longhand; `measure` matches by address, and this fact is what lets
//         it resolve the symbol.
void __cdecl say_special(char *output, char *text, int faction_id) {
    char buffer[256];
    char parsed[256];

    buffer[0] = 0;
    int noun = Players[faction_id].is_leader_female;
    strcat(buffer, "$<");

    if (noun == 0) {
        strcat(buffer, "M");
    } else if (noun == 1) {
        strcat(buffer, "F");
    } else if (noun == 2) {
        strcat(buffer, "N");
    }

    strcat(buffer, "1");
    strcat(buffer, ":");
    strcat(buffer, text);
    strcat(buffer, ">");

    parse_string(buffer, parsed);
    strcat(output, parsed);
}

// ORIGINAL: 0x0058F810 ?get_phrase@@YAPADHH@Z 0x0058F810-0x0058F8A2 BYTE_EXACT
// LEVER: the faction-table bases at 0x00946A54 and 0x00946A6C are
//         `Players[a1].filename` and `Players[a1].search_key` - the image
//         computes `a1 * 0x59c` and adds those two constants, and 0x00946A50
//         (Players) plus 4 and plus 0x1c are exactly those two fields of
//         faction.h's Player.
// LEVER: 0x0068FAD0 is the .rdata literal "PLAYERFILE" (image_data.py), the
//         popup caption for the section that failed to open.
// LEVER: 0x009B7D00 is text.h's `TextBufferGetPtr`, already modelled; the
//         image returns the POINTER stored there (`mov eax, [0x9b7d00]`), so
//         the binding is a real global, not a buffer.
char *__cdecl get_phrase(int faction_id, int phrase_id) {
    // X_pop's PLAYERFILE label has a distinct original start in this function.
    // RECOVERY_OBJECT: v1 0x0068FAD0 _?PlayerFileSectionKey@?1??get_phrase@@YAPADHH@Z@4QBDB
    static const char PlayerFileSectionKey[] = "PLAYERFILE";
    char *search_key = Players[faction_id].search_key;
    char *filename = Players[faction_id].filename;
    int ok = text_open(filename, search_key);
    if (ok != 0) {
        ok = text_open(filename, filename);
        if (ok != 0) {
            parse_says(0, search_key, -1, -1);
            parse_says(1, filename, -1, -1);
            X_pop(PlayerFileSectionKey, 0);
            return StringTable.get(0);
        }
    }
    int remaining = phrase_id + 9;
    while (remaining > 0) {
        text_get();
        --remaining;
    }
    // TRIED: the counter as one local driving both the guard and the loop
    //         (`int n = phrase_id + 9; if (n > 0) { do { text_get(); n--; }`)
    //         scores 46 of 58 - VC6 folds the add into `lea esi,[eax+9]` and
    //         tests esi, where the image tests eax and then copies it to esi
    //         for the loop. Modifying the parameter instead (`phrase_id =
    //         phrase_id + 9;` with a separate `int i`) moves the whole test
    //         to esi too: 45 of 58. The image's shape needs the guard value
    //         and the counter in DIFFERENT registers, which VC6 merges under
    //         both spellings.
    return TextBufferGetPtr;
}

// ORIGINAL: 0x0058F8B0 ?get_noun_phrase@@YAPADHH@Z 0x0058F8B0-0x0058F9C9
// LEVER: same three bindings as get_phrase - Players' filename/search_key at
//         0x00946A54/0x00946A6C, the "PLAYERFILE" caption at 0x0068FADC, and
//         text.h's TextBufferGetPtr at 0x009B7D00.
// BUG IN THE ARCHIVED TRANSCRIPTION, not in the shipped image: the artifact
//         returned the ADDRESS of the TextBufferGetPtr variable rather than
//         the pointer it holds. The image loads `[0x9b7d00]` into eax twice,
//         once as strcpy's destination and once as the return value, so it
//         returns the buffer. Fixed here; the two loads are kept separate on
//         purpose - caching one local would delete the second
//         `mov eax, [0x9b7d00]`.
char *__cdecl get_noun_phrase(int faction_id, int phrase_id) {
    // Keep this API label in a separate original storage identity.
    // RECOVERY_OBJECT: v1 0x0068FADC _?PlayerFileSectionKey@?1??get_noun_phrase@@YAPADHH@Z@4QBDB
    static const char PlayerFileSectionKey[] = "PLAYERFILE";
    char buffer[256];
    char *search_key = Players[faction_id].search_key;
    char *filename = Players[faction_id].filename;

    if ((text_open(filename, search_key) != 0)
        && (text_open(filename, filename) != 0)) {
        parse_says(0, search_key, -1, -1);
        parse_says(1, filename, -1, -1);
        X_pop(PlayerFileSectionKey, 0);
        return StringTable.get(0);
    }

    phrase_id = phrase_id + 9;
    if (phrase_id > 0) {
        int i = phrase_id;
        do {
            text_get();
            --i;
        } while (i != 0);
    }
    strcpy(buffer, text_item());
    char *item = text_item();
    char c = item[0];
    int number = 0;
    int gender = 0;
    if (c == 'M' || c == 'm') {
        gender = 0;
    }
    if (c == 'F' || c == 'f') {
        gender = 1;
    }
    if (c == 'N' || c == 'n') {
        gender = 2;
    }
    if (item[1] == '1') {
        number = 0;
    }
    if (item[1] == '2') {
        number = 1;
    }
    parse_set(gender, number);
    strcpy(TextBufferGetPtr, buffer);
    return TextBufferGetPtr;
}

// ORIGINAL: 0x0058FA70 ?get_pacts@@YAPADHH@Z 0x0058FA70-0x0058FAFA BYTE_EXACT
// working copy - scaffold materialised by --work
// size      138 bytes
// prototype
// callers   2   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x006169A0

// LEVER: the three fixed-address bindings renamed to the tree's own models -
//         Players (faction.h), Labels (alpha.h) and StringTable (strings.h), so
//         no anonymous g_ binding arrived with the body. 0x009BBFEC is
//         GenderDefault and 0x009BBFF0 is PluralityDefault (general.h).
// LEVER: `int flag; if (a && b) flag = 1; else flag = 0;` - the two else arms
//         SHARE one `xor edx,edx` and the true arm jumps over it
//         (`mov edx,1 / jmp / xor edx,edx`). The ternary and the bare `&&`
//         both compile the other way (`jne` straight to the store).
// TRIED: `code` as a ternary, as a nested ternary and with `labels` declared
//         ahead of it - all three move `mine` off ECX (22/43, 4/43, 12/43)
//         against 27/43 for the plain `int code = 0xcb; if (...) code = 0xcc;`.
//         What is left is register ASSIGNMENT: the image keeps `code` in EAX
//         (reusing the dead `Players[other_id]` load) and `labels` in EDX, and
//         hoists the third `test ecx,ecx` above the two global stores.

char *__cdecl get_pacts(int faction_id, int other_id) {
    int mine = Players[faction_id].is_leader_female;
    int flag;
    if (mine != 0 && Players[other_id].is_leader_female != 0) {
        flag = 1;
    } else {
        flag = 0;
    }
    GenderDefault = flag;
    PluralityDefault = 1;
    return label_get((mine != 0 && Players[other_id].is_leader_female != 0) ? 0xcc : 0xcb);
}

// ORIGINAL: 0x0058FBC0 ?get_his_her@@YAPADHH@Z 0x0058FBC0-0x0058FC4B BYTE_EXACT
// LEVER: the fixed-address bindings renamed to the tree's own models - Players
//         (faction.h), Labels (alpha.h), StringTable (strings.h), the CharUpperA
//         import slot (windows.h) and 0x00945838 as PronounBuffer below. The
//         use-site census names that buffer: get_his_her (this body),
//         get_him_her (0x0058FC50) and get_he_she (0x0058FCE0) are the only
//         three sites in the image that touch it, and each strcpy's its fetched
//         pronoun into it and hands back its address.
// TRIED: `LPSTR *labels` read inline in the call instead of as a named local -
//         20/42 against 21/42, so the local stays. Naming the flag `female`
//         first, swapping the `+` operands, and the `(x != 0) + 0xcf` form all
//         score the same 21/42 as what is committed. What is left is register
//         ASSIGNMENT: the image zeroes the `0xcf +` accumulator into EDX before
//         the index chain and keeps `Labels.strings_ptr` in EAX; this tree puts
//         the accumulator in EAX and the label table in ECX.

char *__cdecl get_his_her(int faction_id, int sentence_form) {
    strcpy(PronounBuffer, StringTable.get(Labels.strings_ptr[Players[faction_id].is_leader_female != 0 ? 0xd0 : 0xcf]));
    if (sentence_form != 0) {
        if (sentence_form == 1) {
            char buf[2];
            buf[0] = PronounBuffer[0];
            buf[1] = 0;
            CharUpperA(buf);
            PronounBuffer[0] = buf[0];
            return PronounBuffer;
        }
        CharUpperA(PronounBuffer);
    }
    return PronounBuffer;
}

/*
Purpose: Fetch the "him/her" pronoun into the shared game pronoun buffer,
         optionally upper-casing it, beside the sibling get_his_her body.
// ORIGINAL: 0x0058FC50 ?get_him_her@@YAPADHH@Z 0x0058FC50-0x0058FCDB BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original58FC50..58FCDB; retired seed git:b18d3ac8:src/unrecovered/0058fc50.cpp records the same identity and spans.", "The fixed-address bindings rename to the tree's own models exactly as game.cpp's sibling get_his_her (0x0058FBC0) already does: Players (faction.h, the 0x167-stride first word is is_leader_female), Labels (alpha.h), StringTable (strings.h), PronounBuffer (game.h, 0x00945838) and the CharUpperA import the header supplies.", "Label code 0xC7/0xC8 and the sentence-form handling are the seed's measured behaviour."], "reason": "Home the sibling pronoun fetcher beside its shared buffer and get_his_her in the game source."}
Return Value: PronounBuffer
Status: Coverage transcription - unclaimed body
*/
char *__cdecl get_him_her(int faction_id, int sentence_form) {
    strcpy(PronounBuffer, StringTable.get(Labels.strings_ptr[Players[faction_id].is_leader_female != 0 ? 0xC8 : 0xC7]));
    if (sentence_form != 0) {
        if (sentence_form == 1) {
            char buf[2];
            buf[0] = PronounBuffer[0];
            buf[1] = 0;
            CharUpperA(buf);
            PronounBuffer[0] = buf[0];
            return PronounBuffer;
        }
        CharUpperA(PronounBuffer);
    }
    return PronounBuffer;
}

// ORIGINAL: 0x0058FB00 ?get_pact_hood@@YAPADHH@Z 0x0058FB00-0x0058FB61 BYTE_EXACT
// LEVER: the five fixed-address bindings renamed to the tree's own models - see
//         get_pacts above for the address map. No anonymous g_ binding arrived.
// TRIED: `int code = 0xcd; if (...) code = 0xce;`, the ternary form, and a
//         named `other_female` local for the second read - all three score the
//         same 22/29 as the shared-else `if/else`. The image has no upfront
//         constant store (both arms materialise into EAX), which the
//         shared-else form is the only one to reproduce. What is left is
//         register ASSIGNMENT: the image reuses the dead `Players[other_id]`
//         register in EAX for `code`, this tree puts it in ECX.

char *__cdecl get_pact_hood(int faction_id, int other_id) {
    GenderDefault = 0;
    PluralityDefault = 0;
    int code;
    if (Players[faction_id].is_leader_female != 0
        && Players[other_id].is_leader_female != 0) {
        code = 0xce;
    } else {
        code = 0xcd;
    }
    return label_get(code);
}


// Opcode0x24 transfers this persistent window at 0x0094B4D8.
// No semantic fields or complete original allocation extent are asserted.
// Historical identity moved to ScenarioSaveState::wire_opaque_94b4d8 at parent offset +0x18; see member provenance.


// Shared load request: NetWin47D501/47D79D and58E79B/58E7AD write it;
// game_data5A7951 reads it under read mode. Broader storage remains unknown.
// RECOVERY_OBJECT: v1 0x00945D78 ?GameDataLoadRequest@@3HA
int GameDataLoadRequest;


// Save/load orchestration shares the transaction words below.

// Native transaction words: map_data writes the operation mode at 0x5A9295
// and toggles encrypted mode in game_data callers; consumers read these words
// at the same fixed image locations. 0x945D6C / 0x945D74 are distinct fields.
// RECOVERY_OBJECT: v1 0x00945D6C ?GameDataWriteMode@@3HA
int GameDataWriteMode;       // native storage start 0x00945D6C
// RECOVERY_OBJECT: v1 0x00945D74 ?GameDataEncryptedMode@@3HA
int GameDataEncryptedMode;   // native storage start 0x00945D74
// RECOVERY_OBJECT: v1 0x00945D7C ?GameDataLoadStatusWord@@3HA
int GameDataLoadStatusWord;  // native storage start 0x00945D7C

extern int __cdecl game_data(FILE *file, int load_request);

int __cdecl encrypt_read(void *buffer, unsigned int size, unsigned int count, FILE *file);

int __cdecl encrypt_write(void *buffer, unsigned int size, unsigned int count, FILE *file);

static const char MapHeaderText[] = "TERRANMAP";

// RECOVERY_OBJECT: v1 0x00690D88 _SaveHeaderText
static const char SaveHeaderText[] = "TERRAN";

static const char OpenFileMode[] = "rb";

static const char GameSaveReadMode[] = "rb";

static const char GameSaveHeaderText[] = "TERRAN";

// RECOVERY_OBJECT: v1 0x00690DAC _MapSaveHeaderText
static const char MapSaveHeaderText[] = "TERRANMAP";

// RECOVERY_OBJECT: v1 0x00690F50 _MapFileFilterPattern
static const char MapFileFilterPattern[] = "*.";

// RECOVERY_OBJECT: v1 0x00690F54 _EmptyMapName
static const char EmptyMapName[] = "?";

// RECOVERY_OBJECT: v1 0x00690F58 _LatestMapPreference
static const char LatestMapPreference[] = "Latest Map";

// Distinct writable string cells observed at the original call sites.
// These role names do not claim original source names or allocation bounds.
// RECOVERY_OBJECT: v1 0x00690F64 _InitialAutoSaveDeletePath
static char InitialAutoSaveDeletePath[] = "saves\\auto\\Alpha Centauri Autosave 1.SAV";

// The original 0x691B34 pointer cell refers to the separate text at
// 0x691BEC; keep the pointer and its four-byte NUL-terminated pointee distinct.
// RECOVERY_OBJECT: v1 0x00691BEC ?SaveGameFileExtensionText@@3PADA
char SaveGameFileExtensionText[4] = "SAV";
// RECOVERY_OBJECT: v1 0x00691B34 ?SaveGameFileExtension@@3PBDB
LPCSTR SaveGameFileExtension = SaveGameFileExtensionText;
// Native scenario save/load uses pointerB38 toSC; map save/load uses
// pointerB3C toMP. These three-byte text views claim no neighboring extent.
// RECOVERY_OBJECT: v1 0x00691BF0 ?ScenarioFileExtensionText@@3PADA
char ScenarioFileExtensionText[3] = "SC";
// RECOVERY_OBJECT: v1 0x00691B38 ?ScenarioFileExtension@@3PBDB
LPCSTR ScenarioFileExtension = ScenarioFileExtensionText;
// RECOVERY_OBJECT: v1 0x00691BF4 ?MapFileExtensionText@@3PADA
char MapFileExtensionText[3] = "MP";
// Historical SC attribution of this cell was wrong; originalB3C points toMP.
// RECOVERY_OBJECT: v1 0x00691B3C ?MapFileExtension@@3PBDB
LPCSTR MapFileExtension = MapFileExtensionText;
// RECOVERY_OBJECT: v1 0x00690D9C ?GameSaveMultiplayerMarker@@3PADA
char GameSaveMultiplayerMarker[] = "multi";  // native string starts at 0x00690D9C

struct MapDataTransferHeader {
    char opaque_before_format_[0x380];
    int format_version_;
    char opaque_after_format_[0x14];
};

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x005A6510 sub_5a6510 0x005A6510-0x005A6594 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/005a6510.cpp", "recovery/tasks/game-save-load-owner-coverage.json"], "reason": "Recover direction-sign lookup through the genuine nine-entry movement tables and historical C linkage. Preserve the exact seed obligation; new matching claims require native recording."}
// symbol    _sub_5a6510
extern "C" int __cdecl sub_5a6510(int x, int y) {
    int sign_x;
    if (x > 0) {
        sign_x = 1;
    } else {
        sign_x = (x >= 0) - 1;
    }

    int sign_y;
    if (y > 0) {
        sign_y = 1;
    } else {
        sign_y = (y >= 0) - 1;
    }

    for (int index = 0; index < 9; ++index) {
        const int delta_x = VehMoveDeltaX[index];
        int sign_delta_x;
        if (delta_x > 0) {
            sign_delta_x = 1;
        } else {
            sign_delta_x = (delta_x >= 0) - 1;
        }
        if (sign_x == sign_delta_x) {
            const int delta_y = VehMoveDeltaY[index];
            int sign_delta_y;
            if (delta_y > 0) {
                sign_delta_y = 1;
            } else {
                sign_delta_y = (delta_y >= 0) - 1;
            }
            if (sign_y == sign_delta_y) {
                return index;
            }
        }
    }
    return -1;
}

/*
Purpose: Determine if the specified offsets are within the range radius.
// ORIGINAL: 0x005A65A0 ?radius_move@@YAHHHH@Z 0x005A65A0-0x005A65D0 BYTE_EXACT
// size      48 bytes
// prototype int (__cdecl ?radius_move@@YAHHHH@Z)(int xCoord, int yCoord, int range)
// callers   1   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: Range radius, otherwise -1 if not within range
Status: Complete
*/
// Historical visibility note: the pinned image emits this body and also
// calls it at different sites. The definition is now colocated with its
// canonical marker; this pilot does not establish original TU provenance.


int __cdecl radius_move(int x_radius_off, int y_radius_off, int range) {
    for (int i = 0; i < range; i++) {
        if (x_radius_off == RadiusOffsetX[i] && y_radius_off == RadiusOffsetY[i]) {
            return i; // radius
        }
    }
    return -1;
}

/*
Purpose: Determine if the specified two tiles are within the range radius of each other.
// ORIGINAL: 0x005A65D0 ?radius_move@@YAHHHHHH@Z 0x005A65D0-0x005A662C BYTE_EXACT
// size      92 bytes
// prototype int (__cdecl ?radius_move@@YAHHHHHH@Z)(int xCoordSrc, int yCoordSrc, int xCoordDst, int yCoordDst, int range)
// callers   4   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// LEVER: two fixes, 0.709 -> 0.902 similar (3/40 raw, best flag set /O2 /Gy
//        /GR- /Oy- /GX). (1) BUG IN THIS RECOVERY (not the original): the
//        wraparound test compared against the precomputed `MapLongitude`
//        global and adjusted by `MapLongitudeBounds` - a mismatched pair. The
//        image reads ONLY `MapLongitudeBounds` (0x949870) here, computing its
//        own `/2` inline via `cdq; sub; sar` for the threshold and using the
//        UNHALVED value for the adjustment - the exact shape compass_move
//        (0x005A6630) already documents, with `MapLongitudeBounds` where
//        compass_move uses `MapLongitude`. Rewriting both threshold checks as
//        `-((int)MapLongitudeBounds / 2)` / `((int)MapLongitudeBounds / 2)`
//        and the adjustment as `MapLongitudeBounds` fixed the division shape.
//        (2) Declaring `int y_radius_off = y_dst - y_src;` BEFORE
//        `x_radius_off` (rather than evaluating `y_dst - y_src` inline at the
//        tail call) matches the image computing the y-difference early,
//        interleaved with the x-difference - moved 0.800 -> 0.902.
// TRIED: remaining gap is an esi/ecx register swap running through the
//            whole body - same register-allocation plateau as the rest of
//            this file's xrange/on_map-loop family. Not chased further.
Return Value: Range radius, otherwise -1 if not within range
Status: Complete
*/
int __cdecl radius_move(int x_src, int y_src, int x_dst, int y_dst, int range) {
    int x_radius_off = x_dst - x_src;
    if (x_radius_off < -((int)MapLongitudeBounds / 2)) {
        x_radius_off += MapLongitudeBounds;
    }
    if (x_radius_off > ((int)MapLongitudeBounds / 2)) {
        x_radius_off -= MapLongitudeBounds;
    }
    int y_radius_off = y_dst - y_src;
    return radius_move(x_radius_off, y_radius_off, range);
}

/*
Purpose: Determine if the specified two tiles are within the radius directionally of each other.
// ORIGINAL: 0x005A6630 ?compass_move@@YAHHHHH@Z 0x005A6630-0x005A66E0 BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason":"Use the original full longitude bound for signed half-width thresholds and wrap adjustments, not the half-width tile-row stride.","evidence":["Pinned5A663A loads949870 MapLongitudeBounds into ESI; subsequent thresholds halve this register and wrapping adds/subtracts its unhalved value.","MapLongitude is the distinct68FAF0 packed-row stride; the historical LEVER below incorrectly interpreted the masked relocation target."]}
// Historical LEVER (superseded wrong-global interpretation): this body had been copy-adapted from radius_move's wraparound test (compare against MapLongitude, adjust by MapLongitudeBounds), but the image never loads MapLongitudeBounds here at all - it computes `MapLongitude / 2` (a signed `cdq; sub; sar` halving, done twice) as the threshold AND adjusts by the un-halved `MapLongitude` itself. Matching that literally (`x_radius_off < -(ML/2)` / `+= ML`, and the mirrored upper bound) is BYTE_EXACT.
// size      176 bytes
// prototype int (__cdecl ?compass_move@@YAHHHHH@Z)(int xCoordSrc, int yCoordSrc, int xCoordDst, int yCoordDst)
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: Range radius, otherwise -1 if not within range
Status: Complete
*/
int __cdecl compass_move(int x_src, int y_src, int x_dst, int y_dst) {
    int x_radius_off = x_dst - x_src;
    if (x_radius_off < -((int)MapLongitudeBounds / 2)) {
        x_radius_off += MapLongitudeBounds;
    }
    if (x_radius_off > ((int)MapLongitudeBounds / 2)) {
        x_radius_off -= MapLongitudeBounds;
    }
    int y_radius_off = y_dst - y_src;
    int direction_x = (x_radius_off > 0) ? 1 : (x_radius_off >= 0) - 1;
    int direction_y = (y_radius_off > 0) ? 1 : (y_radius_off >= 0) - 1;
    for (int i = 0; i < 9; i++) {
        if (direction_x == (VehMoveDeltaX[i] > 0 ? 1 : (VehMoveDeltaX[i] >= 0) - 1)
            && direction_y == (VehMoveDeltaY[i] > 0 ? 1 : (VehMoveDeltaY[i] >= 0) - 1)) {
            return i;
        }
    }
    return -1;
}

// ORIGINAL: 0x005A66E0 ?encrypt_write@@YAHPAXIIPAUFILE@@@Z 0x005A66E0-0x005A67A8 FILE BYTE_EXACT
// symbol    ?encrypt_write@@YAHPAXIIPAU_iobuf@@@Z
// this tree's FILE is struct _iobuf, so the tag differs from the image's
// TRIED: nothing ruled out; the [ebp+0xb]/[ebp+0xf]/[ebp+0x13] scratch bytes are stack-slot reuse of the first three parameters' high byte, not real locals - modelled as plain unsigned char key1/key2/byte1/byte2 instead. MISMATCH #3, push vs mov, whole-function frame order
// working copy - scaffold materialised by --work
// size      200 bytes
// prototype int (__cdecl ?encrypt_write@@YAHPAXIIPAUFILE@@@Z)(void*, unsigned int, unsigned int, FILE* file)
// callers   3   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005D4510 0x00644EF2 0x0064603F
int __cdecl encrypt_write(void *data, unsigned int size, unsigned int count,
                          FILE *file) {
    unsigned int raw_size = size * count;
    unsigned int padded = raw_size;
    if ((raw_size & 1) != 0) {
        padded = raw_size + 1;
    }
    unsigned char *buf = reinterpret_cast<unsigned char *>(mem_get(padded));
    if (buf == 0) {
        return 0;
    }
    unsigned char *src = reinterpret_cast<unsigned char *>(data);
    unsigned char *dst = buf;
    unsigned char key1 = 0x80;
    unsigned char key2 = 0x80;
    for (unsigned int i = 0; i < padded / 2; ++i) {
        key2 = key2 + 1;
        key1 = key1 - 1;
        unsigned char byte1 = *src++;
        unsigned char byte2;
        if (i < raw_size) {
            byte2 = *src++;
        } else {
            byte2 = 0;
        }
        if (byte1 != 0) {
            unsigned char value = byte1 ^ key2;
            if (value != 0) {
                byte1 = value;
            }
        }
        if (byte2 != 0) {
            unsigned char value = byte2 ^ key1;
            if (value != 0) {
                byte2 = value;
            }
        }
        *dst++ = byte2;
        *dst++ = byte1;
    }
    int result = fwrite(buf, padded, 1, file);
    free(buf);
    return result;
}

// ORIGINAL: 0x005A67B0 ?encrypt_read@@YAHPAXIIPAUFILE@@@Z 0x005A67B0-0x005A6886 FILE
// symbol    ?encrypt_read@@YAHPAXIIPAU_iobuf@@@Z
// this tree's FILE is struct _iobuf, so the tag differs from the image's
// working copy - scaffold materialised by --work
// NOTE: mirrors encrypt_write; remaining gap is the src load/increment
//        schedule (image: inc esi; mov cl,[esi-1]; inc esi).
// size      214 bytes
// prototype int (__cdecl ?encrypt_read@@YAHPAXIIPAUFILE@@@Z)(void* buffer, unsigned int block, unsigned int count, FILE* file)
// callers   3   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005D4510 0x00644EF2 0x00646178
int __cdecl encrypt_read(void *data, unsigned int size, unsigned int count,
                         FILE *file) {
    unsigned int padded = size * count;
    unsigned int remaining = padded;
    if (padded & 1) {
        padded++;
    }
    unsigned char *buf = reinterpret_cast<unsigned char *>(mem_get(padded));
    if (buf == 0) {
        return 0;
    }
    unsigned char *src = buf;
    if (!fread(src, padded, 1, file)) {
        free(buf);
        return 0;
    }
    unsigned char *dst = reinterpret_cast<unsigned char *>(data);
    unsigned char key1 = 0x80;
    unsigned char key2 = 0x80;
    for (unsigned int i = padded / 2; i != 0; --i) {
        key2 = key2 + 1;
        key1 = key1 - 1;
        unsigned char byte2 = src[1];
        unsigned char byte1 = src[0];
        src += 2;
        if (byte2 != 0) {
            unsigned char value = byte2 ^ key2;
            if (value != 0) {
                byte2 = value;
            }
        }
        if (byte1 != 0) {
            unsigned char value = byte1 ^ key1;
            if (value != 0) {
                byte1 = value;
            }
        }
        *dst++ = byte2;
        if (remaining > 1) {
            *dst++ = byte1;
        }
        remaining -= 2;
    }
    free(buf);
    return 1;
}


// ORIGINAL: 0x005A6890 ?game_io@@YAHHPAUFILE@@@Z 0x005A6890-0x005A6AC0 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/005a6890.cpp", "recovery/tasks/game-save-load-owner-coverage.json"], "reason": "Recover the six ordered MapWin member transfers through standard buffered or encrypted I/O and shared transaction state, not raw self offsets."}
// symbol    ?game_io@@YAHPAVMapWin@@PAU_iobuf@@@Z
// TRIED: divergence at #4 (push edi/esi vs test) is register allocation for a1 not hoisting into a callee-saved reg; a hoisted `base` local and inline `(char*)a1+off` casts compile identically.
int __cdecl game_io(MapWin *record, FILE *file) {
    int result;
    void *field = &record->map_view_index_;
    if (GameDataEncryptedMode != 0) {
        result = GameDataWriteMode != 0
            ? encrypt_write(field, 4, 1, file)
            : encrypt_read(field, 4, 1, file);
    } else {
        result = GameDataWriteMode != 0
            ? static_cast<int>(fwrite(field, 4, 1, file))
            : static_cast<int>(fread(field, 4, 1, file));
    }
    if (result == 0) return 1;

    field = &record->view_active_;
    if (GameDataEncryptedMode != 0) {
        result = GameDataWriteMode != 0
            ? encrypt_write(field, 4, 1, file)
            : encrypt_read(field, 4, 1, file);
    } else {
        result = GameDataWriteMode != 0
            ? static_cast<int>(fwrite(field, 4, 1, file))
            : static_cast<int>(fread(field, 4, 1, file));
    }
    if (result == 0) return 1;

    field = &record->field_1DD70_;
    if (GameDataEncryptedMode != 0) {
        result = GameDataWriteMode != 0
            ? encrypt_write(field, 4, 1, file)
            : encrypt_read(field, 4, 1, file);
    } else {
        result = GameDataWriteMode != 0
            ? static_cast<int>(fwrite(field, 4, 1, file))
            : static_cast<int>(fread(field, 4, 1, file));
    }
    if (result == 0) return 1;

    field = &record->zoom_offset_;
    if (GameDataEncryptedMode != 0) {
        result = GameDataWriteMode != 0
            ? encrypt_write(field, 4, 1, file)
            : encrypt_read(field, 4, 1, file);
    } else {
        result = GameDataWriteMode != 0
            ? static_cast<int>(fwrite(field, 4, 1, file))
            : static_cast<int>(fread(field, 4, 1, file));
    }
    if (result == 0) return 1;

    field = &record->center_x_;
    if (GameDataEncryptedMode != 0) {
        result = GameDataWriteMode != 0
            ? encrypt_write(field, 4, 1, file)
            : encrypt_read(field, 4, 1, file);
    } else {
        result = GameDataWriteMode != 0
            ? static_cast<int>(fwrite(field, 4, 1, file))
            : static_cast<int>(fread(field, 4, 1, file));
    }
    if (result == 0) return 1;

    field = &record->center_y_;
    if (GameDataEncryptedMode != 0) {
        result = GameDataWriteMode != 0
            ? encrypt_write(field, 4, 1, file)
            : encrypt_read(field, 4, 1, file);
    } else {
        result = GameDataWriteMode != 0
            ? static_cast<int>(fwrite(field, 4, 1, file))
            : static_cast<int>(fread(field, 4, 1, file));
    }
    return result == 0;
}

static int map_data_plain_block(FILE *file, void *buffer, unsigned int size) {
    if (GameDataWriteMode) return fwrite(buffer, size, 1, file);
    return fread(buffer, size, 1, file);
}

static int map_data_block(FILE *file, void *buffer, unsigned int size) {
    if (GameDataEncryptedMode) {
        if (GameDataWriteMode) {
            return encrypt_write(buffer, size, 1, file);
        }
        return encrypt_read(buffer, size, 1, file);
    }
    return map_data_plain_block(file, buffer, size);
}

int __cdecl game_data_transfer_block(FILE *file, void *buffer, unsigned int size) {
    return map_data_block(file, buffer, size);
}

int __cdecl game_data_plain_transfer(FILE *file, void *buffer, unsigned int byte_count, int write_mode) {
    if (write_mode != 0) return static_cast<int>(fwrite(buffer, byte_count, 1, file));
    return static_cast<int>(fread(buffer, byte_count, 1, file));
}

// Both map daemons call map_data out of line.
#pragma auto_inline(off)
// ORIGINAL: 0x005A9280 ?map_data@@YAHPAUFILE@@HH@Z 0x005A9280-0x005A94ED BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/005a9280.cpp", "recovery/tasks/game-save-load-owner-coverage.json"], "reason": "Recover complete map-data transfer including archive-header blocks, legacy version-dependent block length, failure ordering and real map_read/map_write owners."}
// symbol    ?map_data@@YAHPAU_iobuf@@HH@Z
// TRIED: inline duplication of each read/write block - factored into rw_plain/rw_block helpers instead; DAT_00945d74 is written once (=0) and never again, so the encrypt branch is dead at runtime but still emitted (matches source, not a live check removed); diverges at instr #5
int __cdecl map_data(FILE *file, int write_mode, int transfer_map_header) {
    MapDataTransferHeader header;
    char large_block[0x10660];
    char medium_block[0x2CE0];
    char small_block[0x20];

    GameDataWriteMode = write_mode;
    GameDataEncryptedMode = 0;
    if (transfer_map_header != 0) {
        if (map_data_plain_block(file, &header, sizeof(header)) == 0) goto fail;
        if (map_data_block(file, &write_mode, sizeof(write_mode)) == 0) goto fail;
        if (header.format_version_ >= 3) {
            if (map_data_block(file, large_block, 0x10660) == 0) return 1;
        } else {
            if (map_data_block(file, large_block, 0xE5E0) == 0) goto fail;
        }
        if (map_data_block(file, medium_block, sizeof(medium_block)) == 0) goto fail;
        if (map_data_block(file, small_block, sizeof(small_block)) == 0) goto fail;
    }
    if (GameDataWriteMode ? map_write(file) : map_read(file)) goto fail;
    return 0;
fail:
    return 1;
}
#pragma auto_inline(on)

// ORIGINAL: 0x005A9C10 ?load_map_daemon@@YAHPAD@Z 0x005A9C10-0x005A9DAD BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/005a9c10.cpp", "recovery/tasks/game-save-load-owner-coverage.json"], "reason": "Recover extension/header/version validation, map windows and map tiles, contour copy direction and Paths lifecycle through existing genuine owners."}
// symbol    ?load_map_daemon@@YAHPAD@Z
// TRIED: extension check, env_open, header_check against the TERRAN/ TERRANMAP string depending on the extension, then (fread reuses the `a1` parameter's own stack slot as scratch, matching the disassembly) a MapWin close sweep, map_data, and the counter table reset/copy. Landed at 0.80 similarity; diverges at #3 on register vs stack scheduling for the extension check.
int __cdecl load_map_daemon(char *path) {
    int is_saved_game = 0;
    char *dot = strchr(path, '.');
    // Native691B34 points to SAV. Saved games include the additional
    // extraction blocks; ordinary scenario archives use TERRANMAP.
    if (dot != 0 && _stricmp(dot + 1, SaveGameFileExtension) == 0) {
        is_saved_game = 1;
    }

    FILE *file = env_open(path, OpenFileMode);
    if (file == 0) return 1;

    int result = 2;
    char header[256];
    header_check(header, file);
    const char *expected_header = is_saved_game ? SaveHeaderText : MapHeaderText;
    if (strcmp(header, expected_header) == 0) {
        result = 3;
        int version;
        if (fread(&version, sizeof(version), 1, file) != 0) {
            const int required_version = is_saved_game ? 0x56 : 5;
            if (version >= required_version) {
                for (size_t slot = 1; slot < MapWinTableSlots; ++slot) {
                    if (MapWinTable[slot] != nullptr
                        && MapWinTable[slot]->view_active_ != 0) {
                        MapWinTable[slot]->view_active_ = 0;
                        MapWinTable[slot]->close();
                    }
                }
                ConsoleGlobal.field_23BE4_ = 0;
                result = 4;
                if (map_data(file, 0, is_saved_game) == 0) {
                    GameDataStateStorage.fields_.base_current_count_ = 0;
                    GameDataStateStorage.fields_.vehicle_current_count_ = 0;
                    ConsoleGlobal.cursor_active_ = 0;
                    ConsoleGlobal.selected_vehicle_ = -1;
                    // Native5A9D58/5A9D73 use signed JLE/JL tests on the
                    // shared signed map-area word.
                    Map *tile = MapTiles;
                    for (int map_tile_index = 0;
                         map_tile_index < MapArea;
                         ++map_tile_index, ++tile) {
                        tile->visibility = 0;
                        tile->bit &= ~3U;
                    }
                    BaseComputedID = -1;
                    for (int contour_index = 0; contour_index < 11; ++contour_index) {
                        AltNatural[contour_index] = AltContour[contour_index];
                    }
                    result = 0;
                    Paths.init();
                }
            }
        }
    }

    fclose(file);
    return result;
}

// ORIGINAL: 0x005A94F0 ?save_daemon@@YAHPAD@Z 0x005A94F0-0x005A96C3 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:bcd1dd555eaeb189f64bcfcf6980cbe672210029:src/unrecovered/005a94f0.cpp: complete identity/span and historical return-accumulator lesson", "Pinned5A95A1..5A95CD copies eight live process records93D4FC..93E1DC to93E978..93F658", "Pinned5389F0 preserves partial alias writes through the24-entry lock-state constructor loop", "src/game.cpp: reset_saved_process_slot_state separately initializes snapshot identity bytes at93E99C+i*19C", "recovery/tasks/save-daemon-owner-oct5.json", "recovery/tests/test_save_daemon_actual_source.py"], "reason": "Recover complete daemon save operation with original mutable-pointer ABI, error statuses, side-effect order and file cleanup. Persistent snapshot is an overlapping NetDaemon storage view; its observed endpoint is a lower bound, not a proven original class size or union declaration. Existing lock-state constructor initialization and separate startup identity initializer remain unchanged."}
// TRIED: `fclose`/`remove` are catalogued nullary cdecl, so they are reached through explicit function-pointer casts rather than by name (a direct call fails C2660: too many arguments). The return-value accumulator (`edi`) starts at 2 before `header_write`, becomes 3 once the first `fwrite` succeeds, and only reaches 0 once `game_data` and the second `fwrite` both succeed - the fclose/remove tail runs unconditionally, and `remove` only when the status is still nonzero. Landing the closest control-flow-faithful form (divergence starts at instruction #6, on the `inc` vs. explicit reload of the reentrancy counter).
#if defined(_MSC_VER) && _MSC_VER == 1200
#pragma function(memcpy)
#endif
int __cdecl save_daemon(char *save_path) {
// RECOVERY_OBJECT: v1 0x00690D80 _?PathDot@?1??save_daemon@@YAHPAD@Z@4PADA
    static char PathDot[] = ".";
// RECOVERY_OBJECT: v1 0x00690D84 _?SaveFileMode@?1??save_daemon@@YAHPAD@Z@4PADA
    static char SaveFileMode[] = "wb";

    if (IsMultiplayerPBEM != 0) ++TamperState93A9B4;

    DWORD save_tick = timeGetTime();
    if ((GameDataStateStorage.fields_.game_rules_ & RULES_IRONMAN) != 0 && IsMultiplayerNet == 0) {
        my_srand(static_cast<int>(save_tick));
    }

    GameDataStateStorage.fields_.save_daemon_state_word_ = 0xC;
    const uint32_t save_state_flag = 0x10000000u;
    if ((GameDataStateStorage.fields_.game_rules_ & RULES_IRONMAN) != 0) {
        GameDataPlayerCountSnapshot = 0;
        GameDataStateStorage.fields_.game_state_ |= save_state_flag;
    }
    if (IsMultiplayerPBEM != 0) {
        GameDataStateStorage.fields_.net_turn_flags_ |= 0x20u;
        GameDataStateStorage.fields_.game_state_ |= save_state_flag;
        GameDataPlayerCountSnapshot = 0;
    }
    if (IsMultiplayerNet != 0) {
        GameDataStateStorage.fields_.game_state_ |= save_state_flag;
        GameDataPlayerCountSnapshot = NetDaemonNet.player_count_;
        memcpy(NetDaemonNet.saved_process_slots_,
               NetDaemonNet.process_slots_,
               sizeof(NetDaemonNet.saved_process_slots_));
    }

    char filename[0x100];
    filename[0] = 0;
    strcat(filename, save_path);
    if (strchr(filename, '.') == 0) {
        strcat(filename, PathDot);
        strcat(filename, SaveGameFileExtension);
    }

    FILE *file = env_open(filename, SaveFileMode);
    if (file == 0) return 1;

    int result = 2;
    header_write(SaveHeaderText, file);
    int version = 0x56;
    if (fwrite(&version, 4, 1, file) != 0) {
        result = 3;
        if (game_data(file, 1) == 0) {
            if (fwrite(&save_tick, 4, 1, file) != 0) result = 0;
        }
    }

    fclose(file);
    if (result != 0) remove(filename);
    return result;
}
#if defined(_MSC_VER) && _MSC_VER == 1200
#pragma intrinsic(memcpy)
#endif
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x005A9B20 ?save_map_daemon@@YAHPAD@Z 0x005A9B20-0x005A9C05 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/005a9b20.cpp", "recovery/tasks/game-save-load-owner-coverage.json"], "reason": "Recover map-save header/version/data flow and preserve the original unconditional fclose and failed-file removal error behavior."}
// symbol    ?save_map_daemon@@YAHPAD@Z
int __cdecl save_map_daemon(char *path) {
// RECOVERY_OBJECT: v1 0x00690DA4 _?PathDot@?1??save_map_daemon@@YAHPAD@Z@4PADA
    static char PathDot[] = ".";
// RECOVERY_OBJECT: v1 0x00690DA8 _?SaveFileMode@?1??save_map_daemon@@YAHPAD@Z@4PADA
    static char SaveFileMode[] = "wb";

    int result = 1;
    char filename[256];
    filename[0] = 0;
    strcat(filename, path);
    if (strchr(filename, '.') == 0) {
        strcat(filename, PathDot);
        strcat(filename, MapFileExtension);
    }

    FILE *file = env_open(filename, SaveFileMode);
    if (file != 0) {
        result = 2;
        header_write(MapSaveHeaderText, file);
        int version = 5;
        if (fwrite(&version, sizeof(version), 1, file) != 0) {
            result = 3;
            if (map_data(file, 1, 0) == 0) result = 0;
        }
    }

    fclose(file);
    if (result != 0) remove(filename);
    return result;
}

// ORIGINAL: 0x005A9760 ?load_daemon@@YAHPADH@Z 0x005A9760-0x005A9B19 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/005a9760.cpp", "recovery/tasks/game-save-load-owner-coverage.json"], "reason": "Recover the complete load orchestration through actual windows, paths, player/base/vehicle state and PBEM/tamper lifecycle. The called game_data serializer remains explicitly unfinished; this is not working-game acceptance."}
// symbol    ?load_daemon@@YAHPBDH@Z
// TRIED: no Ghidra decompilation was provided for this address; transcribed straight from the disassembly. The two `// indirect` sites are vtable slots 1 (takes an int) and 2 (nullary) on the static object at g_007b0cb8, dispatched through VCall as the brief's shim pattern.
// TRIED: nothing further tried; MISMATCH #0 (push vs sub - a prologue/ stack-layout difference from the local array sizing), similarity 0.73 over 953 bytes. The "copy loop" at 0x5A9A02 is a broadcast of the value at a fixed index into all 0x20 slots of two parallel int arrays (g_009392c0, g_00939340), not a running index as first guessed.
int __cdecl load_daemon(LPCSTR path, int linearize_contours) {
    // Function-scope reads keep their own frame slots, as in the image.
    int version;
    int seed;
    FILE *file = env_open(path, GameSaveReadMode);
    if (file == 0) return 1;
    int result = 2;
    char header[256];
    header_check(header, file);
    if (strcmp(header, GameSaveHeaderText) == 0) {
        result = 3;
        if (fread(&version, sizeof(version), 1, file) != 0 && version >= 0x56) {
            for (size_t close_slot = 2; close_slot < MapWinTableSlots; ++close_slot) {
                if (MapWinTable[close_slot] != 0
                    && MapWinTable[close_slot]->view_active_ != 0) {
                    MapWinTable[close_slot]->view_active_ = 0;
                    MapWinTable[close_slot]->close();
                }
            }
            ConsoleGlobal.field_23BE4_ = 0;
            ConsoleGlobal.field_23BE8_ = 0;
            result = 4;
            if (game_data(file, 0) != 0) {
                if (GameDataLoadStatusWord != 0) {
                    result = 5;
                }
            } else {
                if ((GameDataStateStorage.fields_.game_rules_ & RULES_IRONMAN) != 0) {
                    if (fread(&seed, sizeof(seed), 1, file) != 0) my_srand(seed);
                }
                if (IsMultiplayerNet == 0) {
                    for (size_t offline_slot = 2; offline_slot < MapWinTableSlots; ++offline_slot) {
                        if (MapWinTable[offline_slot] != 0) {
                            MapWinTable[offline_slot]->view_active_ = 0;
                        }
                    }
                }
                BaseComputedID = -1;
                result = 0;
                Paths.init();
                if (linearize_contours != 0) world_linearize_contours();
                for (int contour_slot = 0; contour_slot < 11; ++contour_slot) {
                    AltNatural[contour_slot] = AltContour[contour_slot];
                }

                // Bit 0x200 marks a faction with a satellite or a
                // map-revealing technology.
                for (int tech_faction = 1; tech_faction < MaxPlayerNum; ++tech_faction) {
                    PlayersData[tech_faction].flags &= ~PFLAG_MAP_REVEALED;
                    for (int satellite_index = 0; satellite_index < 4; ++satellite_index) {
                        if (PlayersData[tech_faction].satellites_[satellite_index] != 0) {
                            PlayersData[tech_faction].flags |= PFLAG_MAP_REVEALED;
                        }
                    }
                    for (int technology_id = 0; technology_id < MaxTechnologyNum; ++technology_id) {
                        if ((Technology[technology_id].flags & 8U) != 0
                            && has_tech(technology_id, tech_faction) != 0) {
                            PlayersData[tech_faction].flags |= PFLAG_MAP_REVEALED;
                        }
                    }
                }

                for (int alive_faction = 1; alive_faction < MaxPlayerNum; ++alive_faction) {
                    int has_presence = 0;
                    for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; ++base_id) {
                        if (Bases[base_id].faction_id_current == alive_faction) {
                            has_presence = 1;
                            break;
                        }
                    }
                    if (!has_presence) {
                        for (int vehicle_id = 0; vehicle_id < GameDataStateStorage.fields_.vehicle_current_count_; ++vehicle_id) {
                            if (Vehs[vehicle_id].faction_id == alive_faction
                                && VehPrototypes[Vehs[vehicle_id].proto_id].plan == PLAN_COLONIZATION) {
                                has_presence = 1;
                            }
                        }
                    }
                    if (!has_presence) {
                        GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_ &= ~(1 << alive_faction);
                    } else {
                        GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_ |= 1 << alive_faction;
                    }
                }

                if ((GameDataStateStorage.fields_.game_rules_ & RULES_IRONMAN) != 0) remove(path);

                // The current history index is re-read for every slot.
                for (int history_slot = 0; history_slot < 32; ++history_slot) {
                    ConsoleGlobal.cursor_history_x_[history_slot] =
                        ConsoleGlobal.cursor_history_x_[ConsoleGlobal.cursor_history_index_];
                    ConsoleGlobal.cursor_history_y_[history_slot] =
                        ConsoleGlobal.cursor_history_y_[ConsoleGlobal.cursor_history_index_];
                }

                rebuild_vehicle_bits();
                rebuild_base_bits();

                if (strstr(path, GameSaveMultiplayerMarker) != 0 && IsMultiplayerNet == 0) {
                    ConsoleGlobal.cursor_active_ = 0;
                    GameDataStateStorage.fields_.game_state_ |= 0x10C0;
                    const unsigned char state_low_byte = static_cast<unsigned char>(GameDataStateStorage.fields_.game_state_);
                    if ((state_low_byte & 0x80) != 0 && (state_low_byte & 0x40) != 0) {
                        MainInterfaceGlobal.flat_button_[2].show(0);
                    } else {
                        MainInterfaceGlobal.flat_button_[2].hide();
                    }
                }

                ControlTurnPhase = ConsoleGlobal.faction_id_;
                if (IsMultiplayerNet != 0) {
                    IsMultiplayerPBEM = 0;
                } else if ((GameDataStateStorage.fields_.net_turn_flags_ & 0x20) == 0
                    && bit_count(GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 0xFFu) <= 1) {
                    IsMultiplayerPBEM = 0;
                } else {
                    IsMultiplayerPBEM = 1;
                    check_tamper();
                }
                WorldClimateWorldWin.field_239BC_ |= 4;
            }
        }
    }
    fclose(file);
    return result;
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x005ABB90 ?save_map@@YAXXZ 0x005ABB90-0x005ABD0C;0x0066221F-0x00662234 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/005abb90.cpp", "recovery/tasks/game-save-load-owner-coverage.json"], "reason": "Recover map-save dialog, selection, preference and save orchestration through lexical FileBox lifetime. Preserve both original spans including compiler cleanup; retain the seed."}
// symbol    ?save_map@@YAXXZ
void __cdecl save_map() {
    FileBox box;
    box.set_def_name(MapFilePath);
    box.set_def_ext(MapFileExtension);

    char filter[256];
    filter[0] = 0;
    strcat(filter, MapFileFilterPattern);
    strcat(filter, MapFileExtension);
    box.add_filter(filter, StringTable.get(Labels.strings_ptr[0x678 / 4]));

    char *const selection = box.save();
    do_all_draws();
    if (selection == 0) return;

    char name[256];
    strcpy(name, selection);
    if (strcmp(name, EmptyMapName) == 0) {
        strcpy(name, StringTemp);
    }
    char preference_value[256];
    strcpy(preference_value, name);
    char *dot = strchr(preference_value, '.');
    if (dot != 0) *dot = 0;

    strncpy(MapFilePath, preference_value, 0x4B);
    prefs_put(LatestMapPreference, preference_value);
    save_map_daemon(name);
}

/*
Purpose: Delete the initial auto-save game file.
// ORIGINAL: 0x005ABD10 ?kill_auto_save@@YAXXZ 0x005ABD10-0x005ABD1C BYTE_EXACT
// size      12 bytes
// prototype 
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00646950
Return Value: n/a
Status: Complete
*/
void __cdecl kill_auto_save() {
    remove(InitialAutoSaveDeletePath);
}

/*
Purpose: Handle the creation and management of the auto-save game files.
// ORIGINAL: 0x005ABD20 ?auto_save@@YAXXZ 0x005ABD20-0x005ABE39 BYTE_EXACT
// TRIED (2026-09-03, probed against cl 12.00.8168 in isolation): the
//   image's split cleanup - a batched `add esp, 0x1c` covering the seven
//   `remove` calls and then `pop ecx` for the trailing `save_daemon`, so 0x1c
//   + 4 rather than this tree's single `add esp, 0x20` - is the per-call
//   cleanup merged by peephole with a size-shaped `pop ecx` on the last one.
//   Probed with a two-branch function shaped like this one (7 same-callee
//   calls then a different one-arg callee, then a mixed run): /O2 defers all
//   eight into one `add esp, 0x20` and elides nothing else; /O1, /O2 /Os and
//   /O1 /Oy keep the `pop ecx` but replace the pushes with
//   `mov dword ptr [esp], imm32` argument-slot reuse, which the image does
//   NOT have (it pushes every argument); /O2 /Os and /O2 /G6 likewise. No
//   combination of /O1//O2 /Os//Ot /Ob0 /Oi- /Oy- /G5//G6 in FLAG_SETS or
//   outside them produces fresh pushes, a partial batched `add esp`, AND a
//   trailing `pop ecx` - which reads as this translation unit having been
//   compiled with the stack-pop deferral off, a setting the flag space here
//   does not model. listing_diff agrees the ONLY two differences in the body
//   are those cleanups (image 66 instructions, this tree 64); 28/66 is the
//   byte-shift cost of the two bytes they add, not four divergent
//   instructions.
// TRIED: 28/66, 0.954 similar - as close as this gets. Both branches'
//            call sequences end with the image cleaning the LAST call's
//            single pushed argument via `pop ecx` (1 byte) rather than
//            folding it into the preceding `add esp, N` that cleans the
//            other calls in the run; this body folds all of them into one
//            `add esp`. Pure VC6 stack-cleanup-size heuristic across a run
//            of same-shaped `__cdecl` calls, not a source-form lever.
// size      281 bytes
// prototype 
// callers   9   call targets   3
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005A94F0 0x00646950 0x006469F7
Return Value: n/a
Status: Complete
*/
void __cdecl auto_save() {
    if (!IsMultiplayerPBEM || IsMultiplayerNet) { // auto-saving disabled for PBEM/HotSeat games
        if (GameDataStateStorage.fields_.game_rules_ & RULES_IRONMAN && !(GameDataStateStorage.fields_.game_state_ & STATE_SCENARIO_EDITOR)) {
            remove("saves\\auto\\Alpha Centauri Autosave 30.SAV");
            remove("saves\\auto\\Alpha Centauri Autosave 20.SAV");
            remove("saves\\auto\\Alpha Centauri Autosave 10.SAV");
            remove("saves\\auto\\Alpha Centauri Autosave 5.SAV");
            remove("saves\\auto\\Alpha Centauri Autosave 4.SAV");
            remove("saves\\auto\\Alpha Centauri Autosave 3.SAV");
            remove("saves\\auto\\Alpha Centauri Autosave 2.SAV");
        } else { // standard auto saves
            if (!(GameDataStateStorage.fields_.turn_current_num_ % 10)) {
                remove("saves\\auto\\Alpha Centauri Autosave 30.SAV");
                rename("saves\\auto\\Alpha Centauri Autosave 20.SAV",
                    "saves\\auto\\Alpha Centauri Autosave 30.SAV");
                rename("saves\\auto\\Alpha Centauri Autosave 10.SAV",
                    "saves\\auto\\Alpha Centauri Autosave 20.SAV");
                save_daemon(const_cast<char *>("saves\\auto\\Alpha Centauri Autosave 10"));
            }
            remove("saves\\auto\\Alpha Centauri Autosave 5.SAV");
            rename("saves\\auto\\Alpha Centauri Autosave 4.SAV",
                "saves\\auto\\Alpha Centauri Autosave 5.SAV");
            rename("saves\\auto\\Alpha Centauri Autosave 3.SAV",
                "saves\\auto\\Alpha Centauri Autosave 4.SAV");
            rename("saves\\auto\\Alpha Centauri Autosave 2.SAV",
                "saves\\auto\\Alpha Centauri Autosave 3.SAV");
            rename("saves\\auto\\Alpha Centauri Autosave 1.SAV",
                "saves\\auto\\Alpha Centauri Autosave 2.SAV");
        }
        save_daemon(const_cast<char *>("saves\\auto\\Alpha Centauri Autosave 1"));
    }
}

/*
Purpose: Extended auto-saving regardless of the game type or settings for debug purposes.
Original Offset: n/a
Return Value: n/a
Status: Complete
*/
void __cdecl auto_save_debug() {
    if (GameDataStateStorage.fields_.turn_current_num_ == 1) {
        remove("saves\\auto\\Alpha Centauri Autosave Turn 1.SAV");
        save_daemon(const_cast<char *>("saves\\auto\\Alpha Centauri Autosave Turn 1"));
    }
    if (!(GameDataStateStorage.fields_.turn_current_num_ % 10)) {
        remove("saves\\auto\\Alpha Centauri Autosave 500.SAV");
        char save_path_new[45];
        char save_path_old[45];
        for (int i = 0; i < 490; i += 10) {
            sprintf_s(save_path_old, 45, "saves\\auto\\Alpha Centauri Autosave %d.SAV", 490 - i);
            sprintf_s(save_path_new, 45, "saves\\auto\\Alpha Centauri Autosave %d.SAV", 500 - i);
            rename(save_path_old, save_path_new);
        }
        save_daemon(const_cast<char *>("saves\\auto\\Alpha Centauri Autosave 10"));
    }
    remove("saves\\auto\\Alpha Centauri Autosave 5.SAV");
    rename("saves\\auto\\Alpha Centauri Autosave 4.SAV",
        "saves\\auto\\Alpha Centauri Autosave 5.SAV");
    rename("saves\\auto\\Alpha Centauri Autosave 3.SAV",
        "saves\\auto\\Alpha Centauri Autosave 4.SAV");
    rename("saves\\auto\\Alpha Centauri Autosave 2.SAV",
        "saves\\auto\\Alpha Centauri Autosave 3.SAV");
    rename("saves\\auto\\Alpha Centauri Autosave 1.SAV",
        "saves\\auto\\Alpha Centauri Autosave 2.SAV");
    save_daemon(const_cast<char *>("saves\\auto\\Alpha Centauri Autosave 1"));
}


// HISTORICAL DEFERRED ANALYSIS (retained from prior source; body below supersedes status): confirmed tractable shape, not yet landed - 1983 lines of Ghidra decompilation and 196 conditionals is more than this pass could hand-verify, but the structure is now characterised: Overwhelmingly mechanical: 220 of 256 call sites are __fwrite(60)/__fread(60)/?encrypt_write@@(50)/?encrypt_read@@(50); the other 36 are ?mem_get@@(4)/_memcpy(4)/_free(4)/_abs(4)/ _strcpy(3)/_strlen(2)/?game_io@@(2)/?add@StringStruct@@(2)/ ?map_write@@/?map_read@@/?clear_scenario@@/?clear_monuments@@/ ?clear_bunglist@@/?bit_count@@ (1 each). This is a save/load serializer, field after field. a2 selects the direction: the FIRST call is FUN_0064603f(local_378) when a2!=0 (write: pack current globals into a 0x31C-byte header buffer before writing it) vs FUN_00646178(local_378,0x31C,1,a1) when a2==0 (read: fread that many bytes off `a1` first). Confirmed against the raw disasm's branch on a2 at function entry. The named blockers are ONE gate, not many: `if (local_74 < 0xb)` (local_74 is the version field inside that same 0x31C legacy header) guards roughly 10 raw copy-loops migrating the OLD struct layout into the CURRENT globals (`?9a6490` region) - this is the whole "legacy-struct migration block" and it is READ-ONLY-PATH dead code on any save from version >= 0xb, i.e. every save this binary itself produces. It is real work to transcribe (found 0x2C/0x40/0x16-element copy loops in the first 120 Ghidra lines alone) but it is bounded and separable from the ~220 I/O calls. Frame is 0x374 bytes (from the brief) - count locals against it before landing; a batch of seven diverged only at the prologue over this exact mistake.
// Complete source-owned serializer body; historical byte and seed obligations remain.

// The legacy header is not the current 0x398 state record. Only fields
// consumed by the pinned migration/legacy path are typed here; untouched
// bytes remain opaque. Its final ten words are copied, in order, into the
// current state tail at +0x370. Word four is the signed format version.
// Pinned game_data 0x005A6B37-0x005A6C6C supplies offsets and copy widths;
// names describe the current destinations, not known historical declarators.
struct GameDataHeaderWire {
    unsigned char opaque_000_[0x30];
    uint32_t game_state_;                       // +0x030, copied then read
    unsigned char opaque_034_[4];
    int vehicle_current_count_;                 // +0x038, legacy row bound
    int base_current_count_;                    // +0x03C, legacy row bound
    unsigned char opaque_040_[0x18];
    GameDataLockMaskView lock_masks_;            // +0x058, copied then read
    unsigned char opaque_05C_[0x28];
    int legacy_secret_project_prefix_[33];       // +0x084, copied as 33 dwords
    int governor_faction_;                      // +0x108
    int council_resolution_count_[11];           // +0x10C
    int council_last_resolution_turn_[11];       // +0x138
    uint8_t game_tech_achieved_[89];             // +0x164
    char legacy_game_save_name_[256];            // +0x1BD
    unsigned char opaque_2BD_[3];
    int climate_rise_step_;                     // +0x2C0
    int climate_rise_limit_;                    // +0x2C4
    int climate_rise_accum_;                    // +0x2C8
    int climate_sea_level_delta_;               // +0x2CC
    int transcendence_score_bonus_;             // +0x2D0
    int transcendence_score_weights_[8];         // +0x2D4
    int legacy_terminal_words_[10];              // +0x2F4; version is [4]
};

typedef char GameDataHeaderWireHasNativeExtent[
    sizeof(GameDataHeaderWire) == 0x31C ? 1 : -1];
typedef char GameDataHeaderWireStateOffset[
    offsetof(GameDataHeaderWire, game_state_) == 0x030 ? 1 : -1];
typedef char GameDataHeaderWireVehicleCountOffset[
    offsetof(GameDataHeaderWire, vehicle_current_count_) == 0x038 ? 1 : -1];
typedef char GameDataHeaderWireBaseCountOffset[
    offsetof(GameDataHeaderWire, base_current_count_) == 0x03C ? 1 : -1];
typedef char GameDataHeaderWireLockMasksOffset[
    offsetof(GameDataHeaderWire, lock_masks_) == 0x058 ? 1 : -1];
typedef char GameDataHeaderWireProjectPrefixOffset[
    offsetof(GameDataHeaderWire, legacy_secret_project_prefix_) == 0x084 ? 1 : -1];
typedef char GameDataHeaderWireGovernorOffset[
    offsetof(GameDataHeaderWire, governor_faction_) == 0x108 ? 1 : -1];
typedef char GameDataHeaderWireCouncilCountOffset[
    offsetof(GameDataHeaderWire, council_resolution_count_) == 0x10C ? 1 : -1];
typedef char GameDataHeaderWireCouncilTurnOffset[
    offsetof(GameDataHeaderWire, council_last_resolution_turn_) == 0x138 ? 1 : -1];
typedef char GameDataHeaderWireTechOffset[
    offsetof(GameDataHeaderWire, game_tech_achieved_) == 0x164 ? 1 : -1];
typedef char GameDataHeaderWireSaveNameOffset[
    offsetof(GameDataHeaderWire, legacy_game_save_name_) == 0x1BD ? 1 : -1];
typedef char GameDataHeaderWireClimateOffset[
    offsetof(GameDataHeaderWire, climate_rise_step_) == 0x2C0 ? 1 : -1];
typedef char GameDataHeaderWireWeightsOffset[
    offsetof(GameDataHeaderWire, transcendence_score_weights_) == 0x2D4 ? 1 : -1];
typedef char GameDataHeaderWireTailOffset[
    offsetof(GameDataHeaderWire, legacy_terminal_words_) == 0x2F4 ? 1 : -1];
typedef char GameDataHeaderWireVersionOffset[
    offsetof(GameDataHeaderWire, legacy_terminal_words_) + 4 * sizeof(int)
        == 0x304 ? 1 : -1];

static int game_data_candidate_transfer(FILE *file, void *buffer,
                                        unsigned int byte_count) {
    return map_data_block(file, buffer, byte_count);
}

// BungList remains private to veh.cpp behind its owner-local transfer API.
extern int __cdecl veh_game_data_bunglist(FILE *file, int format_version);
// Console owns its private word; ScenarioSave owns one native 0x98 transfer.
// The native StringList records deliberately use raw fread/fwrite, unlike the
// surrounding encrypted-aware fixed blocks.
extern int __cdecl game_data_plain_transfer(FILE *file, void *buffer,
                                            unsigned int byte_count,
                                            int write_mode);

static void game_data_migrate_legacy_header(
        const GameDataHeaderWire *header, int) {
    // Keep the pin's field-specific order after game_data has copied the full
    // 0x31C input record into the live state window.
    memcpy(GameDataStateStorage.fields_.secret_project_.project_base,
           header->legacy_secret_project_prefix_,
           sizeof(header->legacy_secret_project_prefix_));
    memset(&GameDataStateStorage.fields_.secret_project_.project_base[33],
           0xFF, 31 * sizeof(int));
    GameDataStateStorage.fields_.governor_faction_ = header->governor_faction_;

    // The original loads each pair before storing that pair, then advances
    // by one dword through all eleven records.
    for (int resolution = 0; resolution < 11; ++resolution) {
        const int resolution_count =
            header->council_resolution_count_[resolution];
        const int last_resolution_turn =
            header->council_last_resolution_turn_[resolution];
        GameDataStateStorage.fields_.council_resolution_count_[resolution] =
            resolution_count;
        GameDataStateStorage.fields_.council_last_resolution_turn_[resolution] =
            last_resolution_turn;
    }

    memcpy(GameDataStateStorage.fields_.game_tech_achieved_,
           header->game_tech_achieved_,
           sizeof(GameDataStateStorage.fields_.game_tech_achieved_));
    memcpy(GameDataStateStorage.fields_.legacy_game_save_name_,
           header->legacy_game_save_name_,
           sizeof(GameDataStateStorage.fields_.legacy_game_save_name_));
    GameDataStateStorage.fields_.climate_rise_step_ = header->climate_rise_step_;
    GameDataStateStorage.fields_.climate_rise_limit_ = header->climate_rise_limit_;
    // Preserve the original target write order: +0x34C, +0x344, then +0x348.
    GameDataStateStorage.fields_.transcendence_score_bonus_ =
        header->transcendence_score_bonus_;
    GameDataStateStorage.fields_.climate_rise_accum_ = header->climate_rise_accum_;
    GameDataStateStorage.fields_.climate_sea_level_delta_ =
        header->climate_sea_level_delta_;
    memcpy(GameDataStateStorage.fields_.transcendence_score_weights_,
           header->transcendence_score_weights_,
           sizeof(GameDataStateStorage.fields_.transcendence_score_weights_));
    memcpy(&GameDataStateStorage.fields_.sunspot_duration_,
           header->legacy_terminal_words_, sizeof(header->legacy_terminal_words_));
}

// Version 3..10 PlayerData wire rows are 0x20BC bytes, while the live owner
// stride is 0x20CC. Native first copies the complete 0x20BC prefix to the
// PlayerData object, then applies the member rewrites below. Keep this as a
// wire-record view: the old format is not itself the current C++ class.
struct GameDataLegacyPlayerRowV3 {
    unsigned char bytes_[0x20BC];
};

typedef char GameDataLegacyPlayerRowV3HasNativeExtent[
    sizeof(GameDataLegacyPlayerRowV3) == 0x20BC ? 1 : -1];

struct GameDataLegacyPlayerRowV2 {
    unsigned char bytes_[0x1CBC];
};

typedef char GameDataLegacyPlayerRowV2HasNativeExtent[
    sizeof(GameDataLegacyPlayerRowV2) == 0x1CBC ? 1 : -1];

struct GameDataLegacyBaseRowV1 {
    unsigned char bytes_[0x130];
};
typedef char GameDataLegacyBaseRowV1HasNativeExtent[
    sizeof(GameDataLegacyBaseRowV1) == 0x130 ? 1 : -1];

static void game_data_migrate_legacy_base(Base &base,
                                           const unsigned char *row) {
    // Native 5A7Fxx first copies the complete 0x130-byte legacy prefix into
    // the existing Base, leaving its current-only final dword untouched.
    memcpy(&base, row, 0x130);

    // The native conversion then recopies five facility bytes and clears the
    // following four bytes before its later wider facility writes.
    memcpy(base.facilities_built, row + 0x8C, 5);
    memset(base.facilities_built + 5, 0, 4);
    memcpy(&base.mineral_surplus_final, row + 0x94, 4);
    memcpy(&base.minerals_accumulated_2, row + 0x98, 4);

    // The six parallel native copy lanes start at destination offsets
    // A0/B0/C0/D0/E0/F0 and wire offsets 9C/AC/BC/CC/DC/EC respectively.
    // Express their destinations as the current Base members, retaining the
    // native interleaving order rather than treating the wire row as a Base.
    int *const lane_a[] = {&base.unk_6, &base.unk_7, &base.unk_8, &base.unk_9};
    int *const lane_b = base.intake_1;
    int *const lane_c = base.intake_2;
    int *const lane_d[] = {&base.nutrient_surplus, &base.mineral_surplus,
                           &base.energy_surplus, &base.unused_surplus};
    int *const lane_e[] = {&base.nutrient_inefficiency,
                           &base.mineral_inefficiency,
                           &base.energy_inefficiency,
                           &base.unused_inefficiency};
    int *const lane_f[] = {&base.nutrient_consumption,
                           &base.mineral_consumption,
                           &base.energy_consumption,
                           &base.unused_consumption};
    for (int field = 0; field < 4; ++field) {
        memcpy(lane_a[field], row + 0x9C + field * 4, 4);
        memcpy(&lane_b[field], row + 0xAC + field * 4, 4);
        memcpy(&lane_c[field], row + 0xBC + field * 4, 4);
        memcpy(lane_d[field], row + 0xCC + field * 4, 4);
        memcpy(lane_e[field], row + 0xDC + field * 4, 4);
        memcpy(lane_f[field], row + 0xEC + field * 4, 4);
    }
    memcpy(&base.economy_total, row + 0xFC, 4);
    memcpy(&base.psych_total, row + 0x100, 4);
    memcpy(&base.labs_total, row + 0x104, 4);
    memcpy(&base.unk_10, row + 0x108, 4);
    // Native 0x5A80B3..0x5A8120: three words into +0x110 (the fourth,
    // +0x116, is not copied), then +0x118..+0x130 from row +0x114..+0x12C.
    for (int slot = 0; slot < 3; ++slot)
        memcpy(&base.auto_forward_slots[slot], row + 0x10C + slot * 2, 2);
    memcpy(&base.talent_total, row + 0x114, 4);
    memcpy(&base.drone_total, row + 0x118, 4);
    memcpy(&base.super_drone_total, row + 0x11C, 4);
    int *const lane_g[] = {&base.random_event_turns, &base.nerve_staple_count,
                           &base.unk_11, &base.location};
    for (int tail = 0; tail < 4; ++tail)
        memcpy(lane_g[tail], row + 0x120 + tail * 4, 4);

    for (int item = 0; item < 10; ++item) {
        if (base.queue_production_id[item] <= -0x22)
            base.queue_production_id[item] += -0x1F;
    }
}

static int game_data_expand_legacy_prototype_id(int value) {
    int low = value & 0x1F;
    if ((low & 0x10) != 0) low -= 0x20;
    const int high = (value + (value < 0 ? 0x1F : 0)) >> 5;
    return (high << 6) + low;
}

static void game_data_copy_wire_bytes(unsigned char *destination,
                                      unsigned int destination_offset,
                                      const unsigned char *source,
                                      unsigned int source_offset,
                                      unsigned int byte_count) {
    memcpy(destination + destination_offset, source + source_offset,
           byte_count);
}

// Native <=2 conversion first copies each complete 0x1CBC source row into a
// 0x20BC temporary row, then clears/remaps the old diplomacy/prototype and
// region arrays. Offsets here are within explicitly-sized wire records, not
// live C++ object aliases. The later current-PlayerData conversion consumes
// this intermediate row in the same eight-faction order.
static void game_data_expand_legacy_player_v2(
        GameDataLegacyPlayerRowV3 &destination,
        const GameDataLegacyPlayerRowV2 &source) {
    unsigned char *const out = destination.bytes_;
    const unsigned char *const in = source.bytes_;
    memcpy(out, in, sizeof(source));

    // These clears happen after the record copy and before the interleaving
    // loops in the native converter.
    memset(out + 0x854, 0, 0x200);
    memset(out + 0xA54, 0, 0x200);
    memset(out + 0xC54, 0, 0x400);

    for (int faction = 0; faction < 8; ++faction) {
        for (int item = 0; item < 0x20; ++item) {
            out[0x854 + faction * 0x20 + item] =
                in[0x854 + faction * 0x20 + item];
            out[0xA54 + faction * 0x20 + item] =
                in[0x954 + faction * 0x20 + item];
            memcpy(out + 0xC54 + (faction * 0x20 + item) * 2,
                   in + 0xA54 + (faction * 0x20 + item) * 2, 2);
        }
    }

    // Fixed tail fields and arrays are copied in the source's original order.
    game_data_copy_wire_bytes(out, 0x1054, in, 0xC54, 9 * 4);
    for (int index = 0; index < 0x80; ++index) {
        game_data_copy_wire_bytes(out, 0x1078 + index * 2, in,
                                  0xC78 + index * 2, 2);
        game_data_copy_wire_bytes(out, 0x1178 + index, in, 0xD78 + index, 1);
        game_data_copy_wire_bytes(out, 0x11F8 + index, in, 0xDF8 + index, 1);
        game_data_copy_wire_bytes(out, 0x1278 + index * 2, in,
                                  0xE78 + index * 2, 2);
        game_data_copy_wire_bytes(out, 0x1378 + index * 2, in,
                                  0xF78 + index * 2, 2);
        game_data_copy_wire_bytes(out, 0x1478 + index * 2, in,
                                  0x1078 + index * 2, 2);
        game_data_copy_wire_bytes(out, 0x1578 + index * 2, in,
                                  0x1178 + index * 2, 2);
        game_data_copy_wire_bytes(out, 0x1678 + index * 2, in,
                                  0x1278 + index * 2, 2);
        game_data_copy_wire_bytes(out, 0x1778 + index * 2, in,
                                  0x1378 + index * 2, 2);
        game_data_copy_wire_bytes(out, 0x1878 + index, in, 0x1378 + index, 1);
        game_data_copy_wire_bytes(out, 0x18F8 + index, in, 0x13F8 + index, 1);
        game_data_copy_wire_bytes(out, 0x1978 + index, in, 0x1578 + index, 1);
    }
    game_data_copy_wire_bytes(out, 0x13F8, in, 0x15F8, 0x12C);
    game_data_copy_wire_bytes(out, 0x1EA8, in, 0x1AA8, 0x190);
    game_data_copy_wire_bytes(out, 0x2038, in, 0x1C38, 8 * 4);
    game_data_copy_wire_bytes(out, 0x2058, in, 0x1C58, 8 * 4);
    game_data_copy_wire_bytes(out, 0x206C, in, 0x1C6C, 10 * 4);
    game_data_copy_wire_bytes(out, 0x2094, in, 0x1C94, 4);
    game_data_copy_wire_bytes(out, 0x2098, in, 0x1C98, 9 * 4);
}

// Complete per-row translation of the v3..10 body through 0x5A783A. ESI is
// the current PlayerData row plus facility_announced, and EDI is the legacy
// wire row plus 0x460; source/destination expressions below are expressed
// through the actual PlayerData members rather than a live-object byte view.
static void game_data_migrate_legacy_player_v3(
        PlayerData &player, const GameDataLegacyPlayerRowV3 &wire) {
    const unsigned char *const bytes = wire.bytes_;
    memcpy(&player, bytes, sizeof(wire));
    memcpy(player.facility_announced, bytes + 0x458, 5);
    memset(player.facility_announced + 5, 0, 4);
    memcpy(&player.planet_ecology, bytes + 0x460, sizeof(int));
    memcpy(&player.base_id_atk_target, bytes + 0x464, sizeof(int));
    memcpy(&player.unk_37, bytes + 0x468, sizeof(int));
    memcpy(player.saved_queue_name, bytes + 0x46C,
           sizeof(player.saved_queue_name));
    memcpy(player.saved_queue_size, bytes + 0x52C,
           sizeof(player.saved_queue_size));
    memcpy(player.saved_queue_items, bytes + 0x54C,
           sizeof(player.saved_queue_items));
    memcpy(player.unk_38, bytes + 0x68C, sizeof(player.unk_38));
    for (int row = 0; row < 9; ++row) {
        for (int column = 0; column < 8; ++column) {
            memcpy(&player.unk_39[column][row],
                   bytes + 0x6AC + (row + column * 9) * sizeof(int),
                   sizeof(int));
        }
    }
    memcpy(player.unk_46, bytes + 0x7CC, sizeof(player.unk_46));

    memcpy(&player.unk_47, bytes + 0x7F0, sizeof(player.unk_47) * 3);
    memcpy(&player.nutrient_surplus_total, bytes + 0x85C, sizeof(int));
    memcpy(&player.labs_total, bytes + 0x860, sizeof(int));
    memcpy(player.satellites_, bytes + 0x864, sizeof(player.satellites_));
    memcpy(&player.best_weapon_value, bytes + 0x874,
           sizeof(player.best_weapon_value) * 15);
    memcpy(player.unk_69, bytes + 0x850, sizeof(player.unk_69));

    memcpy(player.proto_id_active, bytes + 0x854,
           sizeof(player.proto_id_active));
    memcpy(player.proto_id_queue, bytes + 0xA54,
           sizeof(player.proto_id_queue));
    memcpy(player.proto_id_lost, bytes + 0xC54,
           sizeof(player.proto_id_lost));
    memcpy(&player.total_mil_veh, bytes + 0xF54,
           sizeof(player.total_mil_veh) * 9);

    memcpy(player.region_total_combat_vehs, bytes + 0x1078,
           sizeof(player.region_total_combat_vehs));
    memcpy(player.region_total_bases, bytes + 0x1178,
           sizeof(player.region_total_bases));
    memcpy(player.region_total_offensive_vehs, bytes + 0x11F8,
           sizeof(player.region_total_offensive_vehs));
    memcpy(player.region_force_rating, bytes + 0x1278,
           sizeof(player.region_force_rating));
    memcpy(player.unk_77, bytes + 0x1378, sizeof(player.unk_77));
    memcpy(player.unk_78, bytes + 0x1478, sizeof(player.unk_78));
    memcpy(player.unk_79, bytes + 0x1578, sizeof(player.unk_79));
    memcpy(player.unk_80, bytes + 0x1678, sizeof(player.unk_80));
    memcpy(player.unk_81, bytes + 0x1778, sizeof(player.unk_81));
    memcpy(player.unk_82, bytes + 0x1878, sizeof(player.unk_82));
    memcpy(player.unk_83, bytes + 0x18F8, sizeof(player.unk_83));
    memcpy(player.region_base_plan, bytes + 0x1978,
           sizeof(player.region_base_plan));

    memcpy(player.goals, bytes + 0x19F8, sizeof(player.goals));
    memcpy(player.sites, bytes + 0x1EA8, sizeof(player.sites));
    memcpy(&player.unk_93, bytes + 0x2038,
           sizeof(player.unk_93) * 8);
    memcpy(player.secret_project_intel, bytes + 0x2058,
           sizeof(unsigned int) * 5);
    memset(player.secret_project_intel + 5, 0,
           sizeof(player.secret_project_intel) - sizeof(unsigned int) * 5);
    memcpy(&player.corner_market_turn, bytes + 0x206C,
           sizeof(player.corner_market_turn) * 10);
    memcpy(&player.unk_109, bytes + 0x2094, sizeof(player.unk_109));
    memcpy(player.score_items_, bytes + 0x2098,
           sizeof(player.score_items_));
}

static int game_data_candidate_transfer_players(FILE *file) {
    // Native allocates the v3 row window even for a current v11+ file. Keep
    // allocation and failed-transfer return order: early errors leak these
    // buffers in the original routine.
    GameDataLegacyPlayerRowV3 *const rows =
        static_cast<GameDataLegacyPlayerRowV3 *>(mem_get(0x105E0));
    if (GameDataStateStorage.fields_.save_daemon_state_word_ < 3) {
        GameDataLegacyPlayerRowV2 *const old_rows =
            static_cast<GameDataLegacyPlayerRowV2 *>(mem_get(0xE5E0));
        if (game_data_candidate_transfer(file, old_rows, 0xE5E0) == 0)
            return 1;
        for (int old_faction = 0; old_faction < 8; ++old_faction)
            game_data_expand_legacy_player_v2(
                rows[old_faction], old_rows[old_faction]);
        free(old_rows);
    }

    // 5A7252 reloads after the optional old-row transfer and expansion.
    // This one value selects both the current and remaining legacy paths.
    const int player_format =
        GameDataStateStorage.fields_.save_daemon_state_word_;
    if (player_format >= 0xB) {
        if (game_data_candidate_transfer(file, PlayersData, 0x10660) == 0)
            return 1;
    } else {
        if (player_format >= 3
            && game_data_candidate_transfer(file, rows, 0x105E0) == 0)
            return 1;
        for (int legacy_faction = 0; legacy_faction < 8; ++legacy_faction)
            game_data_migrate_legacy_player_v3(
                PlayersData[legacy_faction], rows[legacy_faction]);
    }
    free(rows);
    return 0;
}

static int game_data_candidate_string_list(FILE *file, StringList &list,
                                           int write_mode) {
    int count = list.entry_count_;
    if (write_mode != 0) {
        if (game_data_plain_transfer(file, &count, 4, write_mode) == 0)
            return 1;
        for (int index = 0; index < count; ++index) {
            list.seek_pos(index);
            StringStructPayload *const payload = list.current_entry();
            int id = list.current_id();
            strcpy(StringTemp, payload->text);
            int length = static_cast<int>(strlen(StringTemp));
            if (game_data_plain_transfer(file, &id, 4, write_mode) == 0)
                return 1;
            if (game_data_plain_transfer(file, &length, 4, write_mode) == 0)
                return 1;
            if (length != 0
                && game_data_plain_transfer(
                       file, StringTemp, static_cast<unsigned int>(length),
                       write_mode) == 0)
                return 1;
        }
        return 0;
    }

    list.remove_all();
    list.current_position_ = 0;
    list.allocator_ = 0;
    if (game_data_plain_transfer(file, &count, 4, write_mode) == 0)
        return 1;
    for (int index = 0; index < count; ++index) {
        int id;
        int length;
        if (game_data_plain_transfer(file, &id, 4, write_mode) == 0)
            return 1;
        if (game_data_plain_transfer(file, &length, 4, write_mode) == 0)
            return 1;
        if (game_data_plain_transfer(
                file, StringTemp, static_cast<unsigned int>(length),
                write_mode) == 0)
            return 1;
        StringTemp[length] = '\0';
        list.field_20_ = 0;
        list.field_24_ = 0;
        list.pending_text_ = StringTemp;
        list.add(id);
    }
    return 0;
}

// ORIGINAL: 0x005A6AC0 ?game_data@@YAHPAUFILE@@H@Z 0x005A6AC0-0x005A9279
// symbol    ?game_data@@YAHPAU_iobuf@@H@Z
int __cdecl game_data(FILE *file, int write_mode) {
    GameDataHeaderWire header;  // deliberately not initialized before write
    GameDataWriteMode = write_mode;
    GameDataEncryptedMode = 0;
    GameDataLoadStatusWord = 0;

    if (game_data_candidate_transfer(file, &header, sizeof(header)) == 0)
        return 1;

    // The legacy header selects only this first state-record layout. Later
    // format gates use the live word carried in that state record instead.
    const int header_version = header.legacy_terminal_words_[4];
    if (header_version >= 0xB) {
        if (game_data_candidate_transfer(
                file, &GameDataStateStorage, 0x398) == 0)
            return 1;
    } else {
        // Native copies the complete old header into the live state window
        // before the field-by-field legacy conversion; do not stage this in a
        // success-only temporary or normalize its read/write ordering.
        memcpy(&GameDataStateStorage, &header, 0x31C);
        game_data_migrate_legacy_header(&header, header_version);
    }

    if (GameDataStateStorage.fields_.save_daemon_state_word_ >= 4) {
        GameDataEncryptedMode =
            static_cast<int>(GameDataStateStorage.fields_.game_state_
                             & 0x10000000u);
        if (GameDataEncryptedMode != 0) {
            if (game_data_candidate_transfer(
                    file, &GameDataStateStorage, 0x398) == 0)
                return 1;
            if (game_data_candidate_transfer(
                    file, NetDaemonNet.saved_process_slots_, 0xCE0) == 0)
                return 1;
            if (game_data_candidate_transfer(
                    file, &GameDataPlayerCountSnapshot, 4) == 0)
                return 1;
            if (GameDataWriteMode == 0) {
                LPSTR saved_time_control[6];
                for (int save_name_index = 0; save_name_index < 6; ++save_name_index)
                    saved_time_control[save_name_index] = TimeControl[save_name_index].name;
                if (game_data_candidate_transfer(file, TimeControl, 0xC0) == 0)
                    return 1;
                for (int restore_name_index = 0; restore_name_index < 6; ++restore_name_index)
                    TimeControl[restore_name_index].name = saved_time_control[restore_name_index];
            } else if (game_data_candidate_transfer(file, TimeControl, 0xC0) == 0) {
                return 1;
            }
        }
        if (game_data_candidate_transfer(
                file, &ConsoleGlobal.faction_id_,
                sizeof(ConsoleGlobal.faction_id_)) == 0)
            return 1;
    } else {
        GameDataEncryptedMode = 0;
        if (game_data_candidate_transfer(
                file, &ConsoleGlobal.faction_id_,
                sizeof(ConsoleGlobal.faction_id_)) == 0)
            return 1;
    }

    if (game_data_candidate_transfer_players(file) != 0)
        return 1;

    if (game_data_candidate_transfer(file, Players, 0x2CE0) == 0)
        return 1;
    if (game_data_candidate_transfer(file, FactionRankingsUnk, 0x20) == 0)
        return 1;

    if (GameDataWriteMode != 0) {
        if (map_write(file) != 0) return 1;
    } else {
        if (GameDataLoadRequest == 0
            && (GameDataStateStorage.fields_.game_state_ & 1u) == 0) {
            const unsigned int lock_mask =
                GameDataStateStorage.fields_.lock_masks_.enable_mask_;
            const unsigned int alive = (lock_mask >> 8) & 0xffu;
            const unsigned int human = lock_mask & 0xffu;
            unsigned int both = alive & human;
            unsigned int faction_bits = both;
            int matching_factions = 0;
            while (faction_bits != 0) {
                matching_factions += static_cast<int>(faction_bits & 1u);
                faction_bits >>= 1;
            }
            if (matching_factions > 1) {
                for (int faction = 1; faction < 8; ++faction) {
                    const unsigned int bit = 1u << faction;
                    if ((alive & bit) != 0 && (human & bit) != 0
                        && PlayersData[faction].unk_102 != 0) {
                        GameDataLoadStatusWord = 1;
                        return 1;
                    }
                }
            }
        }
        // Native reloads the direction global after the legacy permission
        // probe and can branch to map_write at this second test.
        if (GameDataWriteMode != 0) {
            if (map_write(file) != 0) return 1;
        } else if (map_read(file) != 0) {
            return 1;
        }
    }

    if (game_data_candidate_transfer(
            file, FactionRankingHistory, 16000) == 0)
        return 1;
    if (game_data_candidate_transfer(file, &ReplayEventBytes, 4) == 0)
        return 1;
    if (ReplayEventBytes != 0
        && game_data_candidate_transfer(file, ReplayEvents,
                                        static_cast<unsigned int>(ReplayEventBytes)) == 0)
        return 1;
    if (game_data_candidate_transfer(file, Continents, 0xE00) == 0)
        return 1;

    if (GameDataStateStorage.fields_.save_daemon_state_word_ >= 3) {
        if (game_data_candidate_transfer(file, VehPrototypes, 0x6800) == 0)
            return 1;
    } else {
        for (int prototype_block = 0; prototype_block < 512;
             prototype_block += 64) {
            if (game_data_candidate_transfer(
                    file, &VehPrototypes[prototype_block], 0x680) == 0)
                return 1;
            for (int cleared_prototype = prototype_block + 32;
                 cleared_prototype < prototype_block + 64;
                ++cleared_prototype) {
                VehPrototypes[cleared_prototype].veh_name[0] = '\0';
                VehPrototypes[cleared_prototype].flags = 0;
                VehPrototypes[cleared_prototype].unk_1 = 0;
            }
        }
    }

    if (veh_game_data_bunglist(
            file, GameDataStateStorage.fields_.save_daemon_state_word_) == 0)
        return 1;

    // 5A7CCB retains this format across the prototype-ID and vehicle stages.
    const int vehicle_format =
        GameDataStateStorage.fields_.save_daemon_state_word_;
    if (vehicle_format < 0xB) {
        for (int prototype_id = 0; prototype_id < 512; ++prototype_id) {
            if (VehPrototypes[prototype_id].weapon_id >= 0x0C)
                VehPrototypes[prototype_id].weapon_id += 3;
        }
    }

    if (GameDataStateStorage.fields_.vehicle_current_count_ != 0) {
        if (vehicle_format >= 3) {
            if (game_data_candidate_transfer(
                    file, Vehs,
                    static_cast<unsigned int>(GameDataStateStorage.fields_.vehicle_current_count_) * 0x34u) == 0)
                return 1;
        } else {
            typedef char GameDataLegacyVehRowExtent[0x30];
            GameDataLegacyVehRowExtent *const old_rows =
                static_cast<GameDataLegacyVehRowExtent *>(
                    mem_get(GameDataStateStorage.fields_.vehicle_current_count_ * 0x30));
            if (game_data_candidate_transfer(
                    file, old_rows,
                    static_cast<unsigned int>(GameDataStateStorage.fields_.vehicle_current_count_) * 0x30u) == 0)
                return 1;
            for (int vehicle_id = 0; vehicle_id < GameDataStateStorage.fields_.vehicle_current_count_; ++vehicle_id) {
                const unsigned char *const row =
                    reinterpret_cast<const unsigned char *>(old_rows[vehicle_id]);
                Veh &vehicle = Vehs[vehicle_id];
                memcpy(&vehicle.x, row + 0x00, sizeof(vehicle.x));
                memcpy(&vehicle.y, row + 0x02, sizeof(vehicle.y));
                memcpy(&vehicle.state, row + 0x04, sizeof(vehicle.state));
                memcpy(&vehicle.flags, row + 0x08, sizeof(vehicle.flags));
                vehicle.proto_id = row[0x0A];
                vehicle.unk_1 = 0;
                vehicle.faction_id = row[0x0A];
                vehicle.year_end_lurking = 0;
                vehicle.dmg_incurred = row[0x0C];
                vehicle.order = static_cast<int8_t>(row[0x0D]);
                vehicle.waypoint_count = row[0x0E];
                vehicle.patrol_current_point = row[0x0F];
                for (int point = 0; point < 4; ++point) {
                    memcpy(&vehicle.waypoint_x[point], row + 0x10 + point * 2, 2);
                    memcpy(&vehicle.waypoint_y[point], row + 0x18 + point * 2, 2);
                }
                vehicle.morale = row[0x20];
                vehicle.terraforming_turns = row[0x21];
                vehicle.order_auto_type = row[0x22];
                vehicle.visibility = row[0x23];
                vehicle.moves_expended = row[0x24];
                vehicle.facing = static_cast<int8_t>(row[0x25]);
                vehicle.unk_6 = row[0x26];
                vehicle.move_to_ai_type = row[0x27];
                vehicle.probe_action = row[0x28];
                vehicle.probe_sabotage_id = row[0x29];
                memcpy(&vehicle.home_base_id, row + 0x2A, 2);
                memcpy(&vehicle.next_veh_id_stack, row + 0x2C, 2);
                memcpy(&vehicle.prev_veh_id_stack, row + 0x2E, 2);
            }
            if (old_rows != 0) free(old_rows);
        }
    }

    // 5A7EE7 supplies both the icon correction and base-record layout tests.
    const int base_format =
        GameDataStateStorage.fields_.save_daemon_state_word_;
    if (base_format <= 7) {
        // The two byte stores are inside the real VehPrototype icon_offset
        // fields: 0x9ABA36 is prototype 8 + 0x2E, then 0x9ABA02 is
        // prototype 7 + 0x2E.
        VehPrototypes[8].icon_offset = 3;
        VehPrototypes[7].icon_offset = 2;
    }

    if (base_format >= 0xB) {
        if (GameDataStateStorage.fields_.base_current_count_ != 0
            && game_data_candidate_transfer(
                   file, Bases,
                   static_cast<unsigned int>(GameDataStateStorage.fields_.base_current_count_) * 0x134u) == 0)
            return 1;
    } else if (GameDataStateStorage.fields_.base_current_count_ != 0) {
        GameDataLegacyBaseRowV1 *const old_rows =
            static_cast<GameDataLegacyBaseRowV1 *>(
                mem_get(GameDataStateStorage.fields_.base_current_count_ * 0x130));
        if (game_data_candidate_transfer(
                file, old_rows,
                static_cast<unsigned int>(GameDataStateStorage.fields_.base_current_count_) * 0x130u) == 0)
            return 1;
        for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; ++base_id)
            game_data_migrate_legacy_base(
                Bases[base_id], old_rows[base_id].bytes_);
        if (old_rows != 0) free(old_rows);
    }

    // The fresh 5A8179 gate follows base loading and cleanup; failures there
    // must leave these vehicle and queue IDs unconverted.
    if (GameDataStateStorage.fields_.save_daemon_state_word_ < 3) {
        for (int expanded_vehicle_id = 0;
             expanded_vehicle_id < GameDataStateStorage.fields_.vehicle_current_count_;
             ++expanded_vehicle_id)
            Vehs[expanded_vehicle_id].proto_id = static_cast<int16_t>(
                game_data_expand_legacy_prototype_id(
                    Vehs[expanded_vehicle_id].proto_id));
        for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; ++base_id) {
            Base &base = Bases[base_id];
            const int queue_count = static_cast<int>(base.queue_size);
            if (queue_count >= 0) {
                for (int queue_item = 0; queue_item <= queue_count;
                     ++queue_item) {
                    int &production_id = base.queue_production_id[queue_item];
                    if (production_id >= 0)
                        production_id = game_data_expand_legacy_prototype_id(
                            production_id);
                }
            }
        }
    }

    if ((GameDataStateStorage.fields_.game_state_ & 0x10u) != 0
        && game_data_candidate_transfer(file, ScenarioSaveNameBuffer, 0x100) == 0)
        return 1;
    if (GameDataStateStorage.fields_.save_daemon_state_word_ >= 1) {
        if (game_data_candidate_transfer(file, SaveNameBuffer, 0x100) == 0)
            return 1;
    } else {
        strcpy(SaveNameBuffer,
               SaveNameDefaultStorageGlobal.game_data_migration_default_);
    }
    if (GameDataStateStorage.fields_.save_daemon_state_word_ >= 2) {
        if (game_data_candidate_transfer(file, &ScenarioSave, 0x98) == 0)
            return 1;
    } else {
        clear_scenario();
    }
    if (GameDataStateStorage.fields_.save_daemon_state_word_ >= 7) {
        if (game_data_candidate_transfer(file, FactionMonuments, 0x27A0) == 0)
            return 1;
    } else {
        clear_monuments();
    }
    if (GameDataStateStorage.fields_.save_daemon_state_word_ >= 12) {
        if (game_data_candidate_transfer(file, TectonicDetonationCount, 0x20) == 0)
            return 1;
    } else {
        for (int faction = 0; faction < 8; ++faction)
            TectonicDetonationCount[faction] = 0;
    }

    if (game_data_candidate_transfer(
            file, &ConsoleGlobal.cursor_active_, 4) == 0)
        return 1;
    if (game_data_candidate_transfer(
            file, &ConsoleGlobal.selected_vehicle_, 4) == 0)
        return 1;
    const int history_index = ConsoleGlobal.cursor_history_index_;
    if (game_data_candidate_transfer(
            file, &ConsoleGlobal.cursor_history_x_[history_index], 4) == 0)
        return 1;
    if (game_data_candidate_transfer(
            file, &ConsoleGlobal.cursor_history_y_[history_index], 4) == 0)
        return 1;
    if (ConsoleGlobal.game_data_private_word(file, GameDataWriteMode) == 0)
        return 1;

    for (int map_slot = 0; map_slot < 8; ++map_slot) {
        if (game_io(MapWinTable[map_slot], file) != 0) return 1;
    }
    if (game_io(&WorldClimateWorldWin.mapWin_, file) != 0) return 1;

    if ((GameDataStateStorage.fields_.net_turn_flags_ & 0x20u) != 0
        && game_data_candidate_transfer(file, OfferResetCells, 0x2400) == 0)
        return 1;
    if (game_data_candidate_transfer(file, PendingDiplomacyFlags, 0x100) == 0)
        return 1;
    if (game_data_candidate_transfer(file, FactionTurnCombatWins, 0x20) == 0)
        return 1;
    if (game_data_candidate_transfer(file, FactionTurnCombatLosses, 0x20) == 0)
        return 1;
    for (int faction = 1; faction < 8; ++faction) {
        if (game_data_candidate_string_list(
                file, MessageWinDeferredBaseMessages[faction],
                write_mode) != 0)
            return 1;
    }

    // 5A8A39/4A begins at [1][1] but skips that equal initial pointer;
    // 5A8CC3 advances one row (0x180 bytes), and 5A8CDA/DE advances one
    // column (0x30 bytes) up to [2][0] exclusive. The inner counter is
    // incremented before its strict comparison, so each column visits rows
    // 1 through column-1: the 21 off-diagonal lists in columns 2 through 7.
    for (int column = 2; column < 8; ++column) {
        for (int row = 1; row < column; ++row) {
            if (game_data_candidate_string_list(
                    file, DiplomacyTranscriptLists[row][column],
                    write_mode) != 0)
                return 1;
        }
    }

    if (game_data_candidate_transfer(
            file, &CouncilWindow.field_A1C_, 4) == 0)
        return 1;
    if (game_data_candidate_transfer(
            file, &CouncilWindow.field_A20_, 4) == 0)
        return 1;
    if (game_data_candidate_transfer(
            file, &CouncilWindow.field_A24_, 4) == 0)
        return 1;
    if (game_data_candidate_transfer(
            file, &CouncilWindow.field_A28_, 4) == 0)
        return 1;
    if (game_data_candidate_transfer(
            file, CouncilWindow.leader_label_kind_, 0x20) == 0)
        return 1;
    if (game_data_candidate_transfer(
            file, CouncilWindow.leader_tech_, 0x20) == 0)
        return 1;
    if (game_data_candidate_transfer(file, &CouncilPbemGate, 4) == 0)
        return 1;
    if (game_data_candidate_transfer(file, CouncilNotifyFormSlots, 0x20) == 0)
        return 1;
    if (game_data_candidate_transfer(file, CouncilNotifyPluralSlots, 0x20) == 0)
        return 1;
    if (game_data_candidate_transfer(file, &TamperState93A9AC, 4) == 0)
        return 1;
    if (game_data_candidate_transfer(file, &TamperState93A9B0, 4) == 0)
        return 1;
    if (game_data_candidate_transfer(file, &TamperState93A9B4, 4) == 0)
        return 1;
    if (game_data_candidate_transfer(file, TamperDialogFaction, 0x20) == 0)
        return 1;
    if (game_data_candidate_transfer(file, TamperDialogValue, 0x20) == 0)
        return 1;

    if (write_mode != 0) {
        if (game_data_candidate_transfer(file, &IfaceScreenWidth, 4) == 0)
            return 1;
        if (game_data_candidate_transfer(file, &IfaceScreenHeight, 4) == 0)
            return 1;
        return 0;
    }

    int saved_width;
    int saved_height;
    if (game_data_candidate_transfer(file, &saved_width, 4) == 0)
        return 1;
    if (game_data_candidate_transfer(file, &saved_height, 4) == 0)
        return 1;
    if (saved_width != IfaceScreenWidth || saved_height != IfaceScreenHeight) {
        for (int map_slot = 1; map_slot < 8; ++map_slot) {
            if (MapWinTable[map_slot] != 0)
                MapWinTable[map_slot]->view_active_ = 0;
        }
        return 0;
    }
    return 0;
}

// ORIGINAL: 0x005AB600 ?load_map@@YAHXZ 0x005AB600-0x005ABB8D;0x006620A7-0x0066221F
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/005ab600.cpp", "recovery/tasks/game-save-load-owner-coverage.json", "pinned image sha256:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "recovery/tests/test_load_map_behavior.py"], "reason": "Recover map loading through AlphaSave's real selected-path return and existing extension, map-daemon, multiplayer, preference, and dialog owners."}
// TRIED: FileBox/AlphaSave are opaque (no fields) in this scaffold, so their real stack sizes are unknown; used one raw byte buffer + placement-new/reinterpret_cast at the exact ebp-relative offsets from the disasm (incl. the manual vtable pokes before AlphaSave/Scroll/FlatButton close() calls) instead of real SEH locals. Compiles, right call order/addresses/args, sim 0.34-0.50 - the missing SEH frame and giant flat buffer (vs per-object locals) rule out byte-exact; not retried given size (1797 bytes, ~30 embedded objects).
int __cdecl load_map() {
    FileBox file_box_lifetime;
    AlphaSave dialog;
    dialog.init();

    char scenario_filter[16] = ".";
    strcat(scenario_filter, MapFileExtension);
    dialog.add_default_extension(scenario_filter);
    char save_filter[16] = ".";
    strcat(save_filter, SaveGameFileExtension);
    dialog.add_default_extension(save_filter);

    char initial_pattern[260] = "maps\\*.";
    strcat(initial_pattern, MapFileExtension);
    LPSTR preference_name = prefs_get("Latest Map", initial_pattern, 0xFF);
    char default_name[264];
    strncpy(default_name, preference_name, 0xFF);
    if (strlen(default_name) == 0) strcpy(default_name, initial_pattern);
    dialog.set_default_filename(default_name);

    char *selected = dialog.load();
    do_all_draws();
    if (selected == 0) return 1;

    char selected_path[260];
    strcpy(selected_path, selected);
    char preference_value[260];
    strcpy(preference_value, selected);
    char *dot = strchr(preference_value, '.');
    if (dot != 0) *dot = '\0';
    prefs_put("Latest Map", preference_value);

    const int result = load_map_daemon(selected_path);
    if (IsMultiplayerNet != 0) {
        if (result == 0) {
            NetWinDialog.loaded_file_path_[0] = '\0';
            strcat(NetWinDialog.loaded_file_path_,
                   preference_value);
            NetWinDialog.field_7728_ = 1;
        } else {
            parse_says(0, selected_path, -1, -1);
            parse_num(0, result);
            StringTemp[0] = '\0';
            strcat(StringTemp, "LOAD");
            switch (result) {
            case 1: strcat(StringTemp, "NONE"); break;
            case 2: strcat(StringTemp, "NOT"); break;
            case 3: strcat(StringTemp, "OLD"); break;
            default: strcat(StringTemp, "ERROR"); break;
            }
            X_pop(StringTemp, 0);
            do_all_draws();
        }
    } else if (result != 0) {
        parse_says(0, selected_path, -1, -1);
        parse_num(0, result);
        StringTemp[0] = '\0';
        strcat(StringTemp, "LOAD");
        switch (result) {
        case 1: strcat(StringTemp, "NONE"); break;
        case 2: strcat(StringTemp, "NOT"); break;
        case 3: strcat(StringTemp, "OLD"); break;
        default: strcat(StringTemp, "ERROR"); break;
        }
        X_pop(StringTemp, 0);
        do_all_draws();
    }
    return result;
}

/*
// ORIGINAL: 0x005A9DB0 ?yearmotize@@YAXPAD@Z 0x005A9DB0-0x005A9EB0 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:f802f71a:src/leaf_recoveries.cpp", "image:0x005A9DB0", "source:src/game.cpp"], "reason": "The complete year-digit rewrite is the helper called by save_game; move its unchanged typed body and original identity out of the generic leaf container into the game owner."}
// TRIED: scaffold's own `isdigit()`/`_itoa()` (empty-parens fallback declarations) - redeclared under new names `my_isdigit`/ `my_itoa` with the real signatures instead (relocation is masked regardless of the symbol name, matching the existing `my_itoa` idiom in git:20b88b523285a816ba99f503d727b761b89b227f:src/recovered/units/0042abc0.cpp).
*/
void __cdecl yearmotize(char *text) {
    char *p = text;
    while (*p != 0) {
        if (strlen(p) < 4) {
            return;
        }
        if (isdigit(p[0]) &&
            isdigit(p[1]) &&
            isdigit(p[2]) &&
            isdigit(p[3]) &&
            !isdigit(p[4]) &&
            (p == text || !isdigit(p[-1]))) {
            char buf[80];
            char year_str[80];
            buf[0] = 0;
            const int year = static_cast<int>(game_year(GameDataStateStorage.fields_.turn_current_num_));
            _itoa(year, year_str, 10);
            strcat(buf, year_str);
            p[0] = buf[0];
            p[1] = buf[1];
            p[2] = buf[2];
            p[3] = buf[3];
        }
        ++p;
    }
}


// Combat updates50A4A7/50A4B5 and50AF1B/50AF2D index these eight words.
// Turn display527CEE/527D0F passes them to WONLOST; reset51E3C0 clears all.
// RECOVERY_OBJECT: v2 0x0093A96C 0x0093A98C ?FactionTurnCombatWins@@3PAHA 0x0051E3C0
int FactionTurnCombatWins[MaxPlayerNum];
// RECOVERY_OBJECT: v2 0x0093A98C 0x0093A9AC ?FactionTurnCombatLosses@@3PAHA 0x0051E3C0
int FactionTurnCombatLosses[MaxPlayerNum];

// save_daemon clears this word on its rules/PBEM paths, or copies NetDaemon's
// player count on its network path before game_data transfers its four bytes.
// It begins after PendingDiplomacyFlags, not inside that matrix or NetDaemon.
// RECOVERY_OBJECT: v2 0x0093F798 0x0093F79C ?GameDataPlayerCountSnapshot@@3HA 0x005A94F0
int GameDataPlayerCountSnapshot;


// RECOVERY_ASSOCIATED: v1 switch_targets 0x005BED20 0x005BEFE0 0x005BEFF0 0x005BED70
// RECOVERY_ASSOCIATED: v1 switch_selector 0x005BED20 0x005BEFF0 0x005BF004 0x005BED6A
// ORIGINAL: 0x005BED20 ?do_unity_crash@@YAHHHH@Z 0x005BED20-0x005BEFDF
// RECOVERY_BINDING: v1 {"reason":"Recover Map bit2 event dispatch, spawn/morale and turn-based energy awards, and the live Base table radar scan with real UI/audio objects. The seed note calling this a unit scan is historical; the original stride134 and nativeBase count prove Bases. No byte/reference or runtime claim is granted.","evidence":["git:0de05de6:src/unrecovered/005bed20.cpp","image:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf","recovery/tasks/scenario-artifact-unity-owner-coverage.json","Original full span and native physical COFF emitted symbol ?do_unity_crash@@YAHHHH@Z; existing source owns all referenced storage and services."]}
// NOTE: code bytes match the image with relocations masked; recover measure
//       refuses it while ConsoleInfoWin/WorldClimateWorldWin lack bounded
//       object spans. The image keeps two "unitfound_sm.pcx" literals (unpooled).
int __cdecl do_unity_crash(int x, int y, int faction_id) {
    const uint32_t event = MapTiles[static_cast<int>(MapLongitude) * y + (x >> 1)].bit2;
    if ((event & 0x80004000u) != 0x4000u) {
        return 0;
    }

    const int event_kind = static_cast<int>(event) >> 24;
    switch (event_kind) {
    case 0:
    case 8: {
        int prototype;
        if (event_kind) {
            prototype = 0x0C;
            if (faction_id == ConsoleGlobal.faction_id_) {
                g_FX.play(0x16);
                NetMsgPrimary.pop("UNITYCHOPPER", -5000, 0, "unitfound_sm.pcx");
                g_FX.fade(0x16);
            }
        } else {
            prototype = 0x14;
            if (faction_id == ConsoleGlobal.faction_id_) {
                g_FX.play(0x16);
                NetMsgPrimary.pop("UNITYMINER", -5000, 0, "unitfound_sm.pcx");
                g_FX.fade(0x16);
            }
        }
        Vehs[veh_init(prototype, faction_id, x, y)].morale = 2;
        return 1;
    }
    case 10: {
        // The award expression is written twice; VC6 CSEs it into one value.
        PlayersData[faction_id].energy_reserves += (GameDataStateStorage.fields_.turn_current_num_ < 0x32) ? 150 : 300;
        parse_num(0, (GameDataStateStorage.fields_.turn_current_num_ < 0x32) ? 150 : 300);
        ConsoleInfoWin.on_redraw();
        if (faction_id == ConsoleGlobal.faction_id_) {
            g_FX.play(0x16);
            NetMsgPrimary.pop("UNITYENGINE", -5000, 0, "foundeng_sm.pcx");
            g_FX.fade(0x16);
        }
        return 1;
    }
    case 0x13: {
        for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; base_id++) {
            StringTemp[0] = 0;
            strcat(StringTemp, Players[Bases[base_id].faction_id_current].filename);
            CharUpperA(StringTemp);
            if (has_fac(1, base_id, 0)
                && (Players[Bases[base_id].faction_id_current].rule_flags & RFLAG_ALIEN) == 0
                && strcmp(StringTemp, "FUNGBOY") != 0) {
                spot_loc(Bases[base_id].x, Bases[base_id].y, faction_id);
            }
        }
        if (faction_id == ConsoleGlobal.faction_id_) {
            draw_map(1);
            WorldClimateWorldWin.field_239BC_ |= 4;
            WorldClimateWorldWin.GraphicWin::redraw();
            g_FX.play(0x16);
            NetMsgPrimary.pop("UNITYRADAR", -5000, 0, "foundradar_sm.pcx");
            g_FX.fade(0x16);
        }
        return 1;
    }
    }
    return 0;
}


// ORIGINAL: 0x00444D90 ?diplo@@YAXHH@Z 0x00444D90-0x00444DA7 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:f178e8db:src/recovered/00444d90.cpp complete original identity/spans and historical claims/findings.", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x444d90 full native body review.", "source:src/game.cpp existing real state owners; no executable-address alias or synthetic receiver."], "reason": "Recover the complete short helper through established source-owned diplomacy, vehicle, text or window state; preserve signed sentinel behavior, original receiver/argument order and conditional callbacks."}
// symbol    ?diplo@@YAXHH@Z
// TRIED: homed into game.cpp (its real TU) 2026-09-03 - does not compile there: the body's only call is `DiploWin::exec` at 0x00443F10, which diplowin.h does not declare and no product TU defines yet. Declaring `void exec(int,int)` on DiploWin without a definition is a link error, so the body stays here until 0x00443F10 is homed into diplowin.cpp and `exec` declared on the class. The `#include "diplowin.h"` game.cpp needs for it is already added.
void __cdecl diplo(int faction_id, int faction_id_with) {
  DiploWindow.exec(faction_id, faction_id_with);
}


// ORIGINAL: 0x004411F0 sub_4411f0 0x004411F0-0x00441241 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:typed-offer-tamper-reset-owners", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x4411f0", "historical seed:src/recovered/units/004411f0.cpp; identity/spans/claims and useful discoveries retained; seed retirement requires fresh verification."], "reason": "Reset the two directed offer cells through the existing typed owner, preserving interleaved count/state write order and payload. Original has two stack ints, no receiver and no meaningful return; caller reachability remains unknown. Seed spelling lesson: full symmetric index expressions preserve native register allocation better than hoisted temporary indices; evidence is not a requirement to use fixed addresses."}
// symbol    ?clear_offer_pair@@YAXHH@Z
void __cdecl clear_offer_pair(int faction_a, int faction_b) {
    OfferResetCells[faction_a][faction_b].count_[0] = 0;
    OfferResetCells[faction_a][faction_b].count_[1] = 0;
    OfferResetCells[faction_b][faction_a].count_[0] = 0;
    OfferResetCells[faction_b][faction_a].count_[1] = 0;
    OfferResetCells[faction_a][faction_b].state_[0] = 0;
    OfferResetCells[faction_a][faction_b].state_[1] = 0;
    OfferResetCells[faction_b][faction_a].state_[0] = 0;
    OfferResetCells[faction_b][faction_a].state_[1] = 0;
}


// RECOVERY_OBJECT: v1 0x00945D80 ?SaveNameDefaultStorageGlobal@@3USaveNameDefaultStorage@@A
SaveNameDefaultStorage SaveNameDefaultStorageGlobal;

/*
// ORIGINAL: 0x005A9EB0 ?save_game@@YAHH@Z 0x005A9EB0-0x005AAAA9;0x00661B40-0x00661E84 FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:f802f71ac84cd67eae7b7f525688da0cb17cd211:src/unrecovered/005a9eb0.cpp", "image:0x5a9eb0", "task:game-save-load-complete-oct5"], "reason": "Complete original orchestration uses one FileBox and one AlphaSave; the apparent stack widgets are its real embedded objects, not independent locals. Preserve original error/retry branches, first-dot filename handling, owned selected-buffer results and all body/EH spans. Source recovery is not exact constructor/unwind or linked fidelity proof."}
// TRIED: time-boxed - the filename/faction/year string build, the FileBox/FileWin set_def_name/set_def_ext/add_filter/ add_def_ext setup, prefs_get/put, the is_visible-gated AlphaSave::save vs FileBox::save dispatch and do_all_draws are transcribed against the disassembly; the failure path (daemon-reserved-name checks, save_daemon, auto_save, NetDaemon::hang_up, a large session-struct zero-fill) is a deliberate stub. MISMATCH #0, prologue only.
*/
int __cdecl save_game(int mode) {
    FileBox file_box;
    AlphaSave alpha_save;

    if (ConsoleGlobal.is_visible() != 0 &&
        WorldClimateSkipTerrainClear == 0) {
        alpha_save.init();
    }

    StringTemp[0] = 0;
    strcat(StringTemp, mode != 0 ? "scenarios\\" : "saves\\");

    const int faction_id = ConsoleGlobal.faction_id_;
    PluralityDefault = 0;
    GenderDefault = Players[faction_id].is_leader_female;
    strcat(StringTemp, Players[faction_id].name_leader);
    strcat(StringTemp, " ");
    strcat(StringTemp, label_get(0x2D0 / 4));
    strcat(StringTemp, " ");
    GenderDefault = Players[faction_id].noun_gender;
    PluralityDefault = Players[faction_id].is_noun_plural;
    strcat(StringTemp, Players[faction_id].noun_faction);
    strcat(StringTemp, ",");
    say_year(StringTemp);

    char transient_name[188];
    strcpy(transient_name, mode != 0 ? ScenarioSaveNameBuffer
                                    : GameDataStateStorage.fields_.legacy_game_save_name_);
    yearmotize(transient_name);
    char *const default_name = transient_name[0] != 0
        ? transient_name : StringTemp;

    file_box.set_def_name(default_name);
    alpha_save.set_default_filename(default_name);

    const char *const extension =
        mode != 0 ? ScenarioFileExtension : SaveGameFileExtension;
    file_box.set_def_ext(extension);

    transient_name[0] = 0;
    strcat(transient_name, "*.");
    strcat(transient_name, extension);
    file_box.add_filter(transient_name,
                        label_get(mode != 0 ? 0x9C8 / 4 : 0x674 / 4));

    transient_name[0] = 0;
    strcat(transient_name, ".");
    strcat(transient_name, extension);
    alpha_save.add_default_extension(transient_name);

    const int use_file_box = prefs_get("WindowsFileBox", 0, 0);
    prefs_put("WindowsFileBox", use_file_box, 0);

    char *selected_path;
    if (ConsoleGlobal.is_visible() != 0 &&
        WorldClimateSkipTerrainClear == 0 && use_file_box == 0) {
        selected_path = alpha_save.save();
    } else {
        selected_path = file_box.save();
    }
    do_all_draws();

    if (selected_path == 0) {
        return 1;
    }

    char filename[256];
    strcpy(filename, selected_path);
    if (strcmp(filename, "auto\\") == 0) {
        strcpy(StringTemp, filename);
    }

    char filename_stem[256];
    strcpy(filename_stem, filename);
    char *dot = strchr(filename_stem, '.');
    if (dot != 0) {
        *dot = 0;
    }

    char *selected_basename = strrchr(filename_stem, '\\');
    if (selected_basename == 0) {
        selected_basename = strrchr(filename_stem, ':');
    }
    if (selected_basename != 0) {
        ++selected_basename;
    } else {
        selected_basename = filename_stem;
    }

    char *default_basename = strrchr(StringTemp, '\\');
    if (default_basename == 0) {
        default_basename = strrchr(StringTemp, ':');
    }
    if (default_basename != 0) {
        ++default_basename;
    } else {
        default_basename = StringTemp;
    }

    if (mode != 0) {
        if (_stricmp(selected_basename, default_basename) != 0) {
            strcpy(ScenarioSaveNameBuffer, selected_basename);
            GameDataStateStorage.fields_.game_state_ |= 0x10;
            for (int faction = 0; faction < MaxPlayerNum; ++faction) {
                PlayersData[faction].unk_102 = 0;
            }
        } else {
            strcpy(ScenarioSaveNameBuffer,
                   SaveNameDefaultStorageGlobal.scenario_equal_default_);
        }
    } else {
        const char *save_name = selected_basename;
        if (_stricmp(selected_basename, default_basename) == 0) {
            save_name = SaveNameDefaultStorageGlobal.save_equal_default_;
        }
        strcpy(GameDataStateStorage.fields_.legacy_game_save_name_, save_name);
        prefs_put("Latest Save", selected_basename);
    }

    const int daemon_result = save_daemon(filename);
    if (daemon_result != 0) {
        parse_says(-1, filename, -1, 0);
        X_pop("SAVEFAILED", 0);
        do_all_draws();
        return 1;
    }

    if (IsMultiplayerPBEM != 0) {
        ExitTurnLoop = 1;
        return 0;
    }

    if (mode == 0 && (GameDataStateStorage.fields_.game_rules_ & RULES_IRONMAN) != 0) {
        auto_save();
        ExitTurnLoop = 1;

        if (IsMultiplayerNet != 0) {
            NetDaemonNet.hang_up();

            NetDaemon &network = NetDaemonNet;
            network.primary_state_.leading_words_[0] = 0;
            network.primary_state_.leading_words_[1] = 0;
            network.primary_state_.leading_words_[2] = 0;
            network.primary_state_.flags_ = 0;
            network.primary_state_.session_phase_ = 0;
            network.primary_state_.trailing_words_[0] = 0;
            network.primary_state_.trailing_words_[1] = 0;
            network.primary_state_.trailing_words_[2] = 0;
            network.primary_state_.trailing_words_[3] = 0;
            network.primary_state_.trailing_words_[4] = 0;
            network.primary_state_.trailing_words_[5] = 0;
            network.primary_state_.trailing_words_[6] = 0;
            network.primary_state_.trailing_words_[7] = 0;
            network.channels_[0].leading_words_[0] = 0;
            network.channels_[0].leading_words_[1] = 0;
            network.channels_[0].payload_.flags = 0;
            network.channels_[0].trailing_words_[0] = 0;
            network.channels_[0].trailing_words_[9] = 0;
            for (int word = 1; word <= 8; ++word) {
                network.channels_[0].trailing_words_[word] = 0;
            }
            network.channels_[1].leading_words_[0] = 0;
            network.channels_[1].leading_words_[1] = 0;
            network.channels_[1].leading_words_[2] = 0;
            network.channels_[1].leading_words_[3] = 0;
            network.channels_[1].payload_.flags = 0;
            network.channels_[1].trailing_words_[2] = 0;
            network.channels_[1].trailing_words_[3] = 0;
            network.channels_[1].trailing_words_[4] = 0;
            network.channels_[1].trailing_words_[5] = 0;
            network.channels_[1].trailing_words_[6] = 0xFF;
            network.channels_[1].trailing_words_[7] = 0;
            network.channels_[1].trailing_words_[8] = 0;

            NetDaemonNet.locks_.clear();
            NetDaemonNet.AlphaNet::close();
            IsMultiplayerNet = 0;
        }

        X_pop("IRONMAN", 0);
        do_all_draws();
    }

    return 0;
}

/*
// ORIGINAL: 0x005AAAB0 ?load_game@@YAHHH@Z 0x005AAAB0-0x005AB5E3;0x00661E84-0x006620A7 FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:f802f71ac84cd67eae7b7f525688da0cb17cd211:src/unrecovered/005aaab0.cpp", "image:0x5aaab0", "task:game-save-load-complete-oct5"], "reason": "Complete original orchestration uses one FileBox and one AlphaSave; the apparent stack widgets are its real embedded objects, not independent locals. Preserve original error/retry branches, first-dot filename handling, owned selected-buffer results and all body/EH spans. Source recovery is not exact constructor/unwind or linked fidelity proof."}
// TRIED: RAII locals (FileBox/AlphaSave/FileWin/Scroll/Sprite x7/ FlatButton x4/ListBox/Dialog/EditBox/Time) declared once at function scope in place of the original's per-branch conditionally-constructed objects (evidenced by inline vtable-pointer stores immediately before each dtor/close() in the exception funclets at 0x661E84+); AlphaSave and EditBox were given a GraphicWin base (embedded Buffer at +0x444 matches the paired GraphicWin-dtor calls on the same address) and FileBox::load/AlphaSave::load were retyped to return char* (eax used immediately as a strcpy/strncpy source, but the catalogued mangling is QAEXXZ = void) - see PROPOSALS. Diverges at instr #14 (xor vs lea); not chased further given the 3414-byte, ~15-local-object scope.
*/
int __cdecl load_game(int mode, int load_argument) {
    FileBox file_box;
    AlphaSave alpha_save;
    char default_name[256];
    char selected_path[256];
    char save_stem[336];
    char previous_name[256];
    char filter[256];
    int result = 0;
    int remove_auto_saves = 0;

    for (;;) {
        alpha_save.init();

        const char *const pattern = mode == 0 ? "saves\\*." : "scenarios\\*.";
        const char *const extension = mode == 0 ? SaveGameFileExtension
                                                 : ScenarioFileExtension;
        const char *const latest_key = mode == 0 ? "Latest Save"
                                                   : "Latest Scenario";
        strcpy(default_name, pattern);
        strcat(default_name, extension);
        strcpy(filter, "*.");
        strcat(filter, pattern);
        strcat(filter, extension);

        char *preference = prefs_get(latest_key, default_name, 0);
        strncpy(default_name, preference, sizeof(default_name) - 1);
        if (default_name[0] == '\0') {
            strcpy(default_name, pattern);
            strcat(default_name, extension);
        }

        file_box.set_def_name(default_name);
        file_box.set_def_ext(const_cast<char *>(extension));
        file_box.add_filter(filter,
                            label_get(mode == 0 ? 0x674 / 4 : 0x9C8 / 4));

        char initial_extension[8];
        strcpy(initial_extension, ".");
        strcat(initial_extension, extension);
        alpha_save.set_default_filename(default_name);
        alpha_save.add_default_extension(initial_extension);

        const int use_file_box = prefs_get("WindowsFileBox", 0, 0);
        prefs_put("WindowsFileBox", use_file_box, 0);
        char *loaded_name = use_file_box != 0 ? file_box.load()
                                               : alpha_save.load();
        do_all_draws();
        if (loaded_name == 0) return 1;

        strcpy(selected_path, loaded_name);
        strcpy(save_stem, loaded_name);
        char *dot = strchr(save_stem, '.');
        if (dot != 0) *dot = '\0';
        if (strstr(save_stem, "auto\\") == 0)
            prefs_put(latest_key, save_stem);

        strcpy(previous_name, SaveNameBuffer);
        result = load_daemon(selected_path, load_argument);

        if (IsMultiplayerNet != 0 && result == 0) {
            NetWinDialog.loaded_file_path_[0] = '\0';
            strcat(NetWinDialog.loaded_file_path_, save_stem);
            NetWinDialog.field_7728_ = (mode != 0) + 2;
        }

        if (mode != 0) {
            for (int faction = 1; faction < MaxPlayerNum; ++faction)
                load_faction_art(faction);
            load_flags();
            GameDataStateStorage.fields_.game_state_ &= 0xF79FCF16u;
            if ((GameDataStateStorage.fields_.game_state_ & 0xC0u) == 0xC0u)
                MainInterfaceGlobal.flat_button(2).show(0);
            else
                MainInterfaceGlobal.flat_button(2).hide();

            strcpy(SaveNameBuffer, save_stem);
            char *slash = strrchr(SaveNameBuffer, '\\');
            if (slash != 0) {
                slash[1] = '\0';
            } else if (strlen(SaveNameBuffer) != 0) {
                strcat(SaveNameBuffer, "\\");
            }
            prefs_use();
        }

        GameSessionActive = strcmp(SaveNameBuffer, previous_name);
        filefind_set_alternative(SaveNameBuffer);
        mode |= static_cast<int>(GameDataStateStorage.fields_.game_state_ & 0x10u);

        if (IsMultiplayerNet == 0 && IsMultiplayerPBEM == 0) {
            for (int faction = 1; faction < MaxPlayerNum; ++faction) {
                if (PlayersData[faction].unk_102 != 0) {
                    X_pop(ScriptPopupFile, "PASSWORDEDLOADATTEMPT", 0);
                    if (WorldClimateSkipTerrainClear == 0) ExitTurnLoop = 1;
                    return 5;
                }
            }
        }

        if (result != 0) {
            parse_says(-1, selected_path, -1, -1);
            parse_num(0, result);
            StringTemp[0] = '\0';
            strcat(StringTemp, "LOAD");
            switch (result) {
            case 1: strcat(StringTemp, "NONE"); break;
            case 2: strcat(StringTemp, "NOT"); break;
            case 3: strcat(StringTemp, "OLD"); break;
            case 4: strcat(StringTemp, "ERROR"); break;
            case 5: strcat(StringTemp, "SECURITY"); break;
            default: break;
            }
            X_pop(StringTemp, 0);
            do_all_draws();
            break;
        }

        if (mode != 0) {
            break;
        }

        for (int faction = 1; faction < MaxPlayerNum; ++faction)
            load_faction_art(faction);
        load_flags();
        parse_say(0,
            Difficulty[GameDataStateStorage.fields_.diff_level_current_],
            -1, -1);

        Player &player = Players[ConsoleGlobal.faction_id_];
        GenderDefault = player.is_leader_female;
        PluralityDefault = 0;
        parse_says(1, player.adj_name_faction, -1, -1);
        GenderDefault = player.is_leader_female;
        PluralityDefault = 0;
        parse_says(2, player.name_leader, -1, -1);
        GenderDefault = player.noun_gender;
        PluralityDefault = player.is_noun_plural;
        parse_says(3, player.noun_faction, -1, -1);

        StringTemp[0] = '\0';
        strcat(StringTemp,
               StringTable.get(Labels.strings_ptr[0]));
        strcat(StringTemp, " ");
        say_year(StringTemp);
        parse_says(4, StringTemp, -1, -1);
        const int prompt_result = X_pops(
            "LOADGAME", 0x40, FactionDiplomacyPortraits[ConsoleGlobal.faction_id_], 0);
        do_all_draws();
        if (prompt_result < 0) {
            alpha_save.close_for_load_retry();
            continue;
        }
        remove_auto_saves = 1;
        break;
    }

    if ((GameDataStateStorage.fields_.game_preferences_ & 0x20000000u) == 0)
        dialog_default_style() |= 4u;
    else
        dialog_default_style() &= ~4u;

    if (remove_auto_saves != 0) {
        for (int suffix = 9; suffix >= 1; --suffix) {
            StringTemp[0] = '\0';
            strcat(StringTemp, "saves\\auto\\Scenario Editor Undo ");
            char number[16];
            _itoa(suffix, number, 10);
            strcat(StringTemp, number);
            remove(StringTemp);
        }
    }
    return result;
}

