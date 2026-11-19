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
// The turn translation unit, 0x0051DF80-0x0052AD2D (image-inferred, see the
// 2026-11-07 TU map). It starts after the MessageWin deferred-message array
// thunks and holds the turn loop, its upkeep phases, the network turn
// rendezvous and control_game, in image order. The boundary is inferred from
// the CRT init table and subject change; council.cpp follows without a marker.
#define DIALOGS_DTOR_INLINE  // stand-in: see ~Dialogs in dialogs.h
#include "stdafx.h"

// Original read_tamper/set_tamper at 0x0051E000/0x0051E090 use these registry exports through the SDK.
// RECOVERY_IMPORT: v1 __imp__RegCloseKey@4 ADVAPI32.dll name:RegCloseKey
// RECOVERY_IMPORT: v1 __imp__RegCreateKeyExA@36 ADVAPI32.dll name:RegCreateKeyExA
// RECOVERY_IMPORT: v1 __imp__RegOpenKeyExA@20 ADVAPI32.dll name:RegOpenKeyExA
// RECOVERY_IMPORT: v1 __imp__RegQueryValueExA@24 ADVAPI32.dll name:RegQueryValueExA
// RECOVERY_IMPORT: v1 __imp__RegSetValueExA@24 ADVAPI32.dll name:RegSetValueExA
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
#include "net_checksum_report.h"
#include "font.h"
#include "popup_fonts.h"
// WORKAROUND(decl): counter alignment. end_of_game is a tie-break on the
// front-end symbol counter modulo 64 (guide 2.5); `recover canary
// src/turn.cpp` computes the pad count (none needed now).
// CANARY: 0x00524870 ?end_of_game@@YAHH@Z residues 1-4,6-19,21-23,27-34,39-49,52,54-60,62-63
// CANARY-PADS: function TurnDeclFnPad

extern int BaseAtDebugPopupFlag;  // base.cpp, original93A94C
extern char *InterludeTextFile;  // interlude.cpp

OfferResetCell OfferResetCells[MaxPlayerNum][MaxPlayerNum];

// Original51E390 clears these independent words and two faction arrays.
// Consumer5275B0 displays the faction and repeated-event value. The three
// scalar meanings remain unknown; they are not fabricated gameplay names.
// RECOVERY_OBJECT: v2 0x0093A9AC 0x0093A9B0 ?TamperState93A9AC@@3HA 0x0051E390
int TamperState93A9AC;
// RECOVERY_OBJECT: v2 0x0093A9B0 0x0093A9B4 ?TamperState93A9B0@@3HA 0x0051E390
int TamperState93A9B0;
// RECOVERY_OBJECT: v2 0x0093A9B4 0x0093A9B8 ?TamperState93A9B4@@3HA 0x0051E390
int TamperState93A9B4;
// RECOVERY_OBJECT: v2 0x0093A9B8 0x0093A9D8 ?TamperDialogFaction@@3PAHA 0x0051E390
int TamperDialogFaction[MaxPlayerNum];
// RECOVERY_OBJECT: v2 0x0093A9D8 0x0093A9F8 ?TamperDialogValue@@3PAHA 0x0051E390
int TamperDialogValue[MaxPlayerNum];
// The registry blob is exactly fifty six-DWORD entries; the adjacent cursor
// is consumed as a signed value by native IDIV before it is advanced.
// RECOVERY_OBJECT: v1 0x0093A060 ?TamperRegistryRecords@@3PAUTamperRecord@@A
TamperRecord TamperRegistryRecords[50];
// RECOVERY_OBJECT: v1 0x0093A9F8 ?TamperRegistryCursor@@3HA
int TamperRegistryCursor;

// ORIGINAL: 0x0051DFD0 sub_51dfd0 0x0051DFD0-0x0051DFF9 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:typed-offer-tamper-reset-owners", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x51dfd0", "historical seed:src/recovered/units/0051dfd0.cpp; identity/spans/claims and useful discoveries retained; seed retirement requires fresh verification."], "reason": "Reset the existing fifty-record registry and cursor with native field order 0,4,8,10,C,14. Native interior cursor begins at record+4 and terminates at93A514; the touched storage ends93A510, not93A514. Neutral fields and unknown reachability remain explicit; no larger allocation or game semantics are inferred."}
// symbol    ?clear_tamper_registry@@YAXXZ
void __cdecl clear_tamper_registry() {
    TamperRegistryCursor = 0;
    for (int index = 0; index < 50; ++index) {
        TamperRegistryRecords[index].field_0_ = 0;
        TamperRegistryRecords[index].field_4_ = 0;
        TamperRegistryRecords[index].field_8_ = 0;
        TamperRegistryRecords[index].count_ = 0;
        TamperRegistryRecords[index].field_C_ = 0;
        TamperRegistryRecords[index].field_14_ = 0;
    }
}

// The registry class strings are one-byte zero-filled views only; no
// larger capacity or independent writer is inferred here.
// RECOVERY_OBJECT: v1 0x0093A9FC _TamperReadClass
static char TamperReadClass[1];
// RECOVERY_OBJECT: v1 0x0093AA00 _TamperWriteClass
static char TamperWriteClass[1];

// Superseded seed deleted; seed paths below are historical Git provenance.
/*
Purpose: The install tamper check's registry half: open the game's
         DirectPlay Applications key, read the "MVM" dword and the "MVB"
         string into the tamper slots, and close the key.

// ORIGINAL: 0x0051E000 ?read_tamper@@YAXXZ 0x0051E000-0x0051E087 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned image 0x0051E000 full span", "game.cpp owns matching registry strings and storage"], "reason": "Home the original unguarded registry-read body beside its real shared tamper records and strings; retain ignored query statuses."}
// size      135 bytes
// prototype
// callers   0   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00669000 0x00669004 0x00669010
// The image reaches ADVAPI32 through its own IAT slots (0x00669000
// RegQueryValueExA, 0x00669004 RegCreateKeyExA, 0x00669010 RegCloseKey -
// the same trio game.cpp's directplay_install_is_complete documents);
// this build calls the same three functions through its import library.
// The subkey is the image's 0x0068AD88 literal, lpClass is the empty
// string at 0x0093A9FC, and the value names are 0x0068ADD0 "MVM" and
// 0x0068ADD4 "MVB".
Return Value: n/a
Status: Coverage transcription - unclaimed body
*/
void __cdecl read_tamper() {
    HKEY key;
// RECOVERY_LITERAL: v1 0x0068AD88 0x0068ADCE $SG65328 0x0051E000
    LONG status = RegCreateKeyExA(HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Microsoft\\DirectPlay\\Applications\\Sid Meier's Alpha Centauri", 0, TamperReadClass, 0, 0xf003f, 0, &key, 0);
    if (status == 0) {
        DWORD value_type;
        DWORD byte_count = 4;
// RECOVERY_LITERAL: v1 0x0068ADD0 0x0068ADD4 $SG65333 0x0051E000
        RegQueryValueExA(key, "MVM", 0, &value_type,
                         reinterpret_cast<LPBYTE>(&TamperRegistryCursor),
                         &byte_count);
        byte_count = 0x4B0;
// RECOVERY_LITERAL: v1 0x0068ADD4 0x0068ADD8 $SG65335 0x0051E000
        RegQueryValueExA(key, "MVB", 0, &value_type,
                         reinterpret_cast<LPBYTE>(TamperRegistryRecords),
                         &byte_count);
        RegCloseKey(key);
    }
}

// ORIGINAL: 0x0051E090 ?set_tamper@@YAXXZ 0x0051E090-0x0051E102 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:edbf268ff962712fadc389b8283e4c795b6d27a2:src/recovered/units/0051e090.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "recovery/tasks/game-logic-seed-owner-coverage.json"], "reason": "game.cpp beside its already-owned TamperWrite* strings, cursor and 50-record array; uses direct Windows registry APIs. Complete reviewed source translation; historical claims remain restoration obligations, not proof of current matching, linked preservation or game acceptance."}
void __cdecl set_tamper() {
    HKEY key;
// RECOVERY_LITERAL: v1 0x0068ADD8 0x0068AE1E $SG65341 0x0051E090
    const LONG status = RegCreateKeyExA(
        HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\DirectPlay\\Applications\\Sid Meier's Alpha Centauri", 0, TamperWriteClass, 0,
        0xF003F, 0, &key, 0);
    if (status == ERROR_SUCCESS) {
// RECOVERY_LITERAL: v1 0x0068AE20 0x0068AE24 $SG65344 0x0051E090
        RegSetValueExA(key, "MVM", 0, REG_BINARY,
                       reinterpret_cast<const BYTE *>(&TamperRegistryCursor), 4);
// RECOVERY_LITERAL: v1 0x0068AE24 0x0068AE28 $SG65346 0x0051E090
        RegSetValueExA(key, "MVB", 0, REG_BINARY,
                       reinterpret_cast<const BYTE *>(TamperRegistryRecords),
                       0x4B0);
        RegCloseKey(key);
    }
}

// ORIGINAL: 0x0051E110 ?check_tamper@@YAXXZ 0x0051E110-0x0051E38B BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:e82971b277b557e89a52143c7c51170d6d3b0bd4:src/recovered/units/0051e110.cpp", "recovery/tasks/game-save-load-owner-coverage.json"], "reason": "Recover the full PBEM tamper check using shared typed registry records and cursor, faction state, direct registry APIs and the original unguarded corrupt-cursor/error behavior."}
// symbol    ?check_tamper@@YAXXZ
void __cdecl check_tamper() {
    if (IsMultiplayerPBEM == 0 || TamperState93A9AC == 0) {
        return;
    }

    read_tamper();

    for (int index = 0; index < 50; ++index) {
        if (TamperState93A9AC == TamperRegistryRecords[index].field_0_ &&
            TamperState93A9B0 == TamperRegistryRecords[index].field_4_ &&
            GameDataStateStorage.fields_.turn_current_num_ == TamperRegistryRecords[index].field_8_ &&
            ConsoleGlobal.faction_id_ == TamperRegistryRecords[index].field_C_ &&
            TamperState93A9B4 == TamperRegistryRecords[index].field_14_) {
            const int current_faction = ConsoleGlobal.faction_id_;
            const int count = ++TamperRegistryRecords[index].count_;
            for (int faction = 1; faction < MaxPlayerNum; ++faction) {
                if (faction != current_faction && is_human(faction) && is_alive(faction)) {
                    TamperDialogFaction[faction] = current_faction;
                    TamperDialogValue[faction] = count;
                }
            }

            set_tamper();
            X_pop("HOTNOTAMPER", 0);
            return;
        }
    }

    // Native code uses the registry cursor directly as an index before
    // reducing it modulo 50. Keep that corrupt/out-of-range behavior intact.
    const int cursor = TamperRegistryCursor;
    TamperRegistryRecords[cursor].field_0_ = TamperState93A9AC;
    TamperRegistryRecords[cursor].field_4_ = TamperState93A9B0;
    TamperRegistryRecords[cursor].field_8_ = GameDataStateStorage.fields_.turn_current_num_;
    TamperRegistryRecords[cursor].field_C_ = ConsoleGlobal.faction_id_;
    TamperRegistryRecords[cursor].count_ = 0;
    TamperRegistryRecords[cursor].field_14_ = TamperState93A9B4;
    TamperRegistryCursor = (cursor + 1) % 50;

    set_tamper();
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0051E390 ?clear_tamper@@YAXXZ 0x0051E390-0x0051E3BC BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned51E390 resets value93A9D8 then faction93A9B8 arrays and scalar93A9AC/B0/B4. Consumer5275B0 uses typed faction/value pairs for tamper dialogs; source-extracted fixture checks full arrays and scalars."], "reason": "Recover complete tamper reset with two independent faction arrays and three independent state words. Keep scalar semantics neutral until their actual consumers are recovered."}
void __cdecl clear_tamper() {
    TamperState93A9AC = 0;
    TamperState93A9B0 = 0;
    TamperState93A9B4 = 0;
    for (int i = 0; i < MaxPlayerNum; i++) {
        TamperDialogFaction[i] = 0;
        TamperDialogValue[i] = 0;
    }
}

// Existing storage belongs to MessageWin's deferred-message owner.
extern StringList MessageWinDeferredBaseMessages[MaxPlayerNum];

// ORIGINAL: 0x0051E3C0 ?clear_all_player_messages@@YAXXZ 0x0051E3C0-0x0051E479 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/0051e3c0.cpp", "Pinned original01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "recovery/tests/test_player_message_state_behavior.py"], "reason": "Recover the complete player-message clear_all_player_messages operation through existing StringList/MessageWin owners and eight faction combat-summary counters. Preserve deletion hooks, live count reloads, reset/display order and retained original seed; no new measured claim."}
// symbol    ?clear_all_player_messages@@YAXXZ
// TRIED: second slot000 dispatch expects 'lea' for the adjustor-thunk address but every tried spelling (add, swapped operands, fully inlined) still emits 'add'; jl/jb fixed via reinterpret_cast<int> on the loop-bound pointer compare
void __cdecl clear_all_player_messages() {
    for (int faction = 0; faction < MaxPlayerNum; ++faction) {
        FactionTurnCombatLosses[faction] = 0;
        FactionTurnCombatWins[faction] = 0;
        StringList &messages = MessageWinDeferredBaseMessages[faction];
        messages.remove_all();
        messages.current_position_ = 0;
        messages.allocator_ = 0;
    }
}

// ORIGINAL: 0x0051E480 sub_51e480 0x0051E480-0x0051E523 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/0051e480.cpp", "Pinned original01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "recovery/tests/test_player_message_state_behavior.py"], "reason": "Recover the complete player-message clear_player_messages operation through existing StringList/MessageWin owners and eight faction combat-summary counters. Preserve deletion hooks, live count reloads, reset/display order and retained original seed; no new measured claim."}
// symbol    ?clear_player_messages@@YAXH@Z
// TRIED: named `adj2` local for the second this-adjusted vtable call (mismatched: original uses `lea` there, not `add`, unlike the first call which does match named). MISMATCH #41 is the sole remaining divergence at 98.5% mnemonic similarity.
void __cdecl clear_player_messages(int faction) {
    StringList &messages = MessageWinDeferredBaseMessages[faction];
    messages.remove_all();
    messages.current_position_ = 0;
    messages.allocator_ = 0;
    FactionTurnCombatLosses[faction] = 0;
    FactionTurnCombatWins[faction] = 0;
}

// ORIGINAL: 0x0051E530 sub_51e530 0x0051E530-0x0051E6ED BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/0051e530.cpp", "Pinned original01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "recovery/tests/test_player_message_state_behavior.py"], "reason": "Recover the complete player-message display_player_messages operation through existing StringList/MessageWin owners and eight faction combat-summary counters. Preserve deletion hooks, live count reloads, reset/display order and retained original seed; no new measured claim."}
// symbol    ?display_player_messages@@YAHH@Z
// TRIED: char* record-base arithmetic (self+0x30*a1 relative to 0x93a7b0) with adjustor-thunk VCall dispatch (slot000/slot001) for the two destroy-list virtual calls; reaches #13
// The id and text lookups are separate inline phases: head_ is reloaded.
static inline int deferred_message_id(StringList &items) {
    return items.head_ != 0 ? items.current_->id : 0;
}
static inline char *deferred_message_text(StringList &items) {
    return items.head_ != 0
               ? (*(StringStructEntry *volatile *)&(items.current_))->string_payload()->text
               : 0;
}

int __cdecl display_player_messages(int faction) {
    InterfaceMessageWin.clear();
    for (int index = 0; index < MessageWinDeferredBaseMessages[faction].entry_count_; ++index) {
        MessageWinDeferredBaseMessages[faction].seek_pos(index);
        ++ConsoleGlobal.message_count_;
        const int id = deferred_message_id(MessageWinDeferredBaseMessages[faction]);
        char *text = deferred_message_text(MessageWinDeferredBaseMessages[faction]);
        InterfaceMessageWin.listBox_.item(text, id | 0x40000000);
        InterfaceMessageWin.listBox_.set_selected_pos(-1, 1);
    }
    const int displayed_count = MessageWinDeferredBaseMessages[faction].entry_count_;
    if (displayed_count != 0) {
        InterfaceMessageWin.listBox_.update_changes();
    }
    clear_player_messages(faction);
    return displayed_count;
}

/*
Purpose: Build one log line for the say-buffer: decimal the number,
         strcat it into a fresh buffer, then the space token and the
         caller's text, and hand the result to log_say_hex's four-argument
         form with two zero tails.

// ORIGINAL: 0x0051E6F0 sub_51e6f0 0x0051E6F0-0x0051E75A BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned image 0x0051E6F0 full span", "original separator slot 0x00682820 is the existing BufferJoinSpace object; log_say_hex is source-owned in log.cpp"], "reason": "Home the original decimal/space/text concatenation and four-argument logger call without changing its call order or unsafe buffers."}
// symbol    ?sub_51e6f0@@YAXPBDH@Z
// size      106 bytes
// prototype
// callers   0   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x006263F0 0x00645470 0x0064FC88
Return Value: n/a
Status: Coverage transcription - claim pending re-measurement
*/
void __cdecl sub_51e6f0(const char *text, int value) {
    char line[0x100];
    char digits[0x50];
    line[0] = 0;
    _itoa(value, digits, 10);
    strcat(line, digits);
    strcat(line, BufferJoinSpace);
    strcat(line, text);
    log_say_hex(line, value, 0, 0);
}

// The pinned body's 27 unpooled literals, in source order.
// RECOVERY_LITERAL: v1 0x0068AE34 0x0068AE36 $SG65308 0x0051E760
// RECOVERY_LITERAL: v1 0x0068AE38 0x0068AE51 $SG65314 0x0051E760
// RECOVERY_LITERAL: v1 0x0068AE54 0x0068AE6C $SG65316 0x0051E760
// RECOVERY_LITERAL: v1 0x0068AE6C 0x0068AE89 $SG65318 0x0051E760
// RECOVERY_LITERAL: v1 0x0068AE8C 0x0068AEA8 $SG65320 0x0051E760
// RECOVERY_LITERAL: v1 0x0068AEA8 0x0068AEC7 $SG65322 0x0051E760
// RECOVERY_LITERAL: v1 0x0068AEC8 0x0068AED9 $SG65328 0x0051E760
// RECOVERY_LITERAL: v1 0x0068AEDC 0x0068AEF0 $SG65329 0x0051E760
// RECOVERY_LITERAL: v1 0x0068AEF0 0x0068AF00 $SG65330 0x0051E760
// RECOVERY_LITERAL: v1 0x0068AF00 0x0068AF15 $SG65331 0x0051E760
// RECOVERY_LITERAL: v1 0x0068AF18 0x0068AF2A $SG65332 0x0051E760
// RECOVERY_LITERAL: v1 0x0068AF2C 0x0068AF2E $SG65333 0x0051E760
// RECOVERY_LITERAL: v1 0x0068AF30 0x0068AF3D $SG65340 0x0051E760
// RECOVERY_LITERAL: v1 0x0068AF40 0x0068AF44 $SG65341 0x0051E760
// RECOVERY_LITERAL: v1 0x0068AF44 0x0068AF56 $SG65342 0x0051E760
// RECOVERY_LITERAL: v1 0x0068AF58 0x0068AF6D $SG65343 0x0051E760
// RECOVERY_LITERAL: v1 0x0068AF70 0x0068AF81 $SG65344 0x0051E760
// RECOVERY_LITERAL: v1 0x0068AF84 0x0068AF91 $SG65348 0x0051E760
// RECOVERY_LITERAL: v1 0x0068AF94 0x0068AFAC $SG65349 0x0051E760
// RECOVERY_LITERAL: v1 0x0068AFAC 0x0068AFC3 $SG65350 0x0051E760
// RECOVERY_LITERAL: v1 0x0068AFC4 0x0068AFD8 $SG65351 0x0051E760
// RECOVERY_LITERAL: v1 0x0068AFD8 0x0068AFF0 $SG65352 0x0051E760
// RECOVERY_LITERAL: v1 0x0068AFF0 0x0068B005 $SG65353 0x0051E760
// RECOVERY_LITERAL: v1 0x0068B008 0x0068B022 $SG65354 0x0051E760
// RECOVERY_LITERAL: v1 0x0068B024 0x0068B037 $SG65355 0x0051E760
// RECOVERY_LITERAL: v1 0x0068B038 0x0068B04E $SG65359 0x0051E760
// RECOVERY_LITERAL: v1 0x0068B050 0x0068B062 $SG65360 0x0051E760
// RECOVERY_ASSOCIATED: v1 switch_targets 0x0051E760 0x0051F0A8 0x0051F0BC 0x0051E7AD
// ORIGINAL: 0x0051E760 ?do_checksums@@YAXH@Z 0x0051E760-0x0051F0A7 BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason":"Recover checksum diagnostics through the real network producer, source-owned 21-word turn checksum storage, typed faction names and original local/shared text buffers. Preserve the original unwritten diagnostic tail; no byte or runtime acceptance claim.","evidence":["original:0x51E760-0x51F0A7; image:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf","git:a5b58877:src/unrecovered/0051e760.cpp","Native51E830..51E840 copies21 words; producer appends26 diagnostic words only when BaseAtDebugPopupFlag is nonzero.","Original section and faction header loops mutate StringTemp; the remaining formatted reports use local buffers and _itoa radix10.","recovery/tasks/base-colony-economy-coverage-wave.json; recovery/tests/test_turn_checksums_behavior.py"]}
// TRIED: literal transcription of the ~25 itoa+strcat+log_say_hex report blocks and 3 walk loops (verified every log_say/log_say_hex call is (msg,value,0,0) with the message text pushed last=1st-param, confirmed against raw disasm push order, not assumed); the switch's dead "cmp esi,4;jne" fallthrough at 0x51E82B is provably unreachable (case4's own short jmp skips over it) and is omitted. Used a file-scope #define REPORT(...)/#undef macro instead of a helper function to stay inside "no helper" while avoiding 25x hand-duplicated blocks. Compiles; first divergence at instr #13 (deeper than most landings this batch), not chased to byte-exact given size (2375 bytes).
// kind      game
void __cdecl do_checksums(int type) {
    if (BaseAtDebugPopupFlag == 0 && type != 4) {
        return;
    }

    // The original passes a 256-dword local buffer (frame 0x400 bytes) to
    // game_checksum(int *); NetChecksumReport names its first 47 words.
    // The tail is never initialized when the producer does not write it.
    int checksums[256];
    NetDaemonNet.game_checksum(checksums);

    // WORKAROUND(inline): dead caller size; the image expands all 19
    // sub_51e6f0 sites, ours refuses the last two without it (one to three
    // dead calls buy one site, four buy both).
    if (type == 99 && type == 98) {
        log_say(StringTemp, 0, 0, 0);
        log_say(StringTemp, 1, 0, 0);
        log_say(StringTemp, 2, 0, 0);
        log_say(StringTemp, 3, 0, 0);
    }
    log_say(" ", 0, 0, 0);
    switch (type) {
    case 0:
        log_say("Checksums at end of turn", GameDataStateStorage.fields_.turn_current_num_, 0, 0);
        break;
    case 1:
        log_say("Checksums at game start", GameDataStateStorage.fields_.turn_current_num_, 0, 0);
        break;
    case 2:
        log_say("Checksums before world regen", GameDataStateStorage.fields_.turn_current_num_, 0, 0);
        break;
    case 3:
        log_say("Checksums after world regen", GameDataStateStorage.fields_.turn_current_num_, 0, 0);
        break;
    case 4:
        log_say("Checksums at beginning of turn", GameDataStateStorage.fields_.turn_current_num_, 0, 0);
        break;
    }
    if (type == 4) {
        for (int i = 0; i < NetTurnChecksumWords; i++) {
            NetTurnChecksums[i] = checksums[i];
        }
    }

    sub_51e6f0("GAME checksum = ", checksums[0]);
    sub_51e6f0("LEADERS checksum = ", checksums[1]);
    sub_51e6f0("MAP checksum = ", checksums[2]);
    sub_51e6f0("VEHICLES checksum = ", checksums[3]);
    sub_51e6f0("BASES checksum = ", checksums[4]);

    log_say(" ", 0, 0, 0);
    int i;
    int *value;
    for (i = 0, value = &checksums[5]; i < 16; ++i) {
        char number[80];
        StringTemp[0] = '\0';
        strcat(StringTemp, "Map SECTION ");
        _itoa(i, number, 10);
        strcat(StringTemp, number);
        strcat(StringTemp, " = ");
        sub_51e6f0(StringTemp, *value++);
    }

    sub_51e6f0("Rules checksum = ", checksums[21]);
    sub_51e6f0("Projects checksum = ", checksums[22]);
    sub_51e6f0("Tech checksum = ", checksums[23]);

    value = &checksums[24];
    for (i = 1; i < MaxPlayerNum; ++i) {
        StringTemp[0] = '\0';
        strcat(StringTemp, Players[i].adj_name_faction);
        strcat(StringTemp, " checksum = ");
        sub_51e6f0(StringTemp, *value++);
    }

    sub_51e6f0("Map climate checksum = ", checksums[31]);
    sub_51e6f0("Map region checksum = ", checksums[32]);
    sub_51e6f0("Map bit checksum = ", checksums[33]);
    sub_51e6f0("Map contour checksum = ", checksums[34]);
    sub_51e6f0("Map terr checksum = ", checksums[35]);
    sub_51e6f0("Map bitwho[0] checksum = ", checksums[36]);
    sub_51e6f0("Params checksum = ", checksums[37]);

    value = &checksums[38];
    for (i = 0; i < MaxPlayerNum; ++i) {
        StringTemp[0] = '\0';
        strcat(StringTemp, Players[i].adj_name_faction);
        strcat(StringTemp, " Vehicles checksum = ");
        sub_51e6f0(StringTemp, *value++);
    }

    sub_51e6f0("Bases checksum = ", checksums[46]);
}

extern int SpockNotifyPending;
extern int SpockHelpActive;
extern int UnkGlobal0093A934;
extern int ConsoleControlTurnActive;

// ORIGINAL: 0x0051F0C0 ?help_spock@@YAXXZ 0x0051F0C0-0x0051F1AA BYTE_EXACT
// size      234 bytes
// prototype void (__cdecl ?help_spock@@YAXXZ)(void)
// callers   1   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0052E00 0x00530320
// Homed 2026-09-04 for the same reason as check_spock below it: its caller
// is in this file, and a forwarder beside the compiled claim is what
// address_index refuses. The four synch/receive sweeps are the session
// refresh the game runs when a turn boundary catches an unnotified client.
void __cdecl help_spock() {
    if (NetDaemonNet.primary_state_.leading_words_[0] != 0) {
        NetDaemonNet.synch(0x25, 0, 0, 0, 0, 1, 0x2101);
        while (NetDaemonNet.receive() != 0) {
        }

        NetDaemonNet.synch(0x12, 0, 0, 0, 0, 1, 0x2101);
        while (NetDaemonNet.receive() != 0) {
        }

        NetDaemonNet.synch(0x26, 0, 0, 0, 0, 1, 0x2101);
        while (NetDaemonNet.receive() != 0) {
        }

        NetDaemonNet.synch(0x14, 0, 0, 0, 0, 1, 0x2101);
        while (NetDaemonNet.receive() != 0) {
        }
    }

    SpockNotifyPending = 0;
    SpockHelpActive = 0;
}

// ORIGINAL: 0x0051F1B0 ?check_spock@@YAXXZ 0x0051F1B0-0x0051F1F2 BYTE_EXACT
// size      66 bytes
// prototype void (__cdecl ?check_spock@@YAXXZ)(void)
// callers   9   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0051F0C0
// LEVER: the four gates are named, not bound - 0x93E8C0 is the daemon's own
// lock_state_[0].dword_[1], 0x93E230 is &locks_, and the pair 0x93F670/74
// are the flags synch sets and help_spock clears. Homed 2026-09-04 so the
// wait loops in this file could call it with the image's `E8` instead of a
// forwarder beside a compiled claim.
// 0x0093A934 is base.cpp's UnkGlobal0093A934 (the energy-event flag), and
// 0x0093E230 is &NetDaemonNet.locks_ - the Lock the image dereferences as an
// absolute, and NetDaemonNet.locks_.any_locks() folds to the same load.
void __cdecl help_spock();  // defined below
void __cdecl check_spock() {
    if (NetDaemonNet.primary_state_.leading_words_[0] != 0 &&
        SpockNotifyPending != 0 && UnkGlobal0093A934 == 0 &&
        ConsoleControlTurnActive == 0) {
        SpockHelpActive = 1;
        if (NetDaemonNet.locks_.any_locks() == 0) {
            help_spock();
        }
    }
}

// ORIGINAL: 0x0051F200 ?wait_loop@@YAXXZ 0x0051F200-0x0051F2BF BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:edbf268ff962712fadc389b8283e4c795b6d27a2:src/unrecovered/0051f200.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "recovery/tasks/game-logic-seed-owner-coverage.json"], "reason": "faction.cpp / existing faction.h declaration: typed diplomacy-state loop; reloads ConsoleGlobal.faction_id_ at each loop head and after log_say/open_channel callbacks, and rereads the human-status byte each iteration, matching pinned 0x51F200 instructions. Uses existing NetDaemonNet, Console state, LockEnableMask human-status byte, FactionDiplomacyFlags, log_say and communicate. Complete reviewed source translation; historical claims remain restoration obligations, not proof of current matching, linked preservation or game acceptance."}
void __cdecl wait_loop() {
    if (IsMultiplayerNet != 0) {
        NetDaemonNet.net_tasks();
        int faction = ConsoleGlobal.faction_id_;
        if (UnkGlobal0093A934 == 0 && ConsoleControlTurnActive == 0 &&
            (NetDaemonNet.factions_waiting_mask() & (1u << faction)) == 0) {
            for (int other = 1; other < MaxPlayerNum; ++other) {
                faction = ConsoleGlobal.faction_id_;
                if (other != faction &&
                    (GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ &
                     (1 << other)) == 0 &&
                    FactionDiplomacyFlags[other] != 0) {
                    log_say("Initiating diplomatic channel", faction, other, 0);
                    if (NetDaemonNet.open_channel(ConsoleGlobal.faction_id_, other) == 0) {
                        communicate(ConsoleGlobal.faction_id_, other, 0);
                    }
                }
            }
        }
    } else {
        wait_task();
    }
}

// ORIGINAL: 0x0051F2C0 ?wait_2@@YAXXZ 0x0051F2C0-0x0051F2D8 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:edbf268ff962712fadc389b8283e4c795b6d27a2:src/recovered/0051f2c0.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "recovery/tasks/game-logic-seed-owner-coverage.json"], "reason": "netdaemon.cpp / netdaemon.h: use existing NetDaemonNet.net_tasks(), IsMultiplayerNet and wait_task(). Complete reviewed source translation; historical claims remain restoration obligations, not proof of current matching, linked preservation or game acceptance."}
void __cdecl wait_2() {
    if (IsMultiplayerNet != 0) {
        NetDaemonNet.net_tasks();
    } else {
        wait_task();
    }
}

// RECOVERY_ASSOCIATED: v1 switch_targets 0x0051F2E0 0x00522120 0x00522178 0x0051F5F4
// ORIGINAL: 0x0051F2E0 ?random_events@@YAXH@Z 0x0051F2E0-0x0052211D
// RECOVERY_BINDING: v1 {"evidence": ["git:6e78872480ef8b4a486516e322f23e9f8652665d:src/unrecovered/0051f2e0.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "recovery/tasks/game-logic-seed-owner-coverage.json; preserve original extents and historical seed lessons. Ordinary owned literals have pinned content provenance, not exact original data-allocation claims.", "pinned native CFG 0x0051F2E0-0x0052211D, switch table 0x0097D044; event kinds 0-21, common selection and early returns retained.", "Native 0x0051F5A2-0x0051F5FC captures selected base coordinates before parser callbacks. Cases 7/8 reread coordinates; case 19 uses live base coordinates; cases 17/20/21 retain the captured coordinates. Contradictory case-20/21 climate tests remain in source rather than being optimized away as assumed dead behavior."], "reason": "Typed random-events dispatcher and all 22 native cases, called by turn_upkeep. Base selection captures faction and coordinates before parser callbacks; later case-specific live reads remain live. Original map/UI, lifecycle and notification services are supplied by their genuine source owners."}
// TRIED: mechanical register/stack-slot goto transliteration (260 labels, 39 independent per-case epilogues auto-detected from the pop/pop/ pop/mov-esp-ebp/pop/ret shape, a 22-case switch off 0x0097D044, two thiscall-via-fixed-ecx targets on Console/MapWin, one indirect GetKeyState call through g_00669330): compiles, MISMATCH sim 0.32 on /O2 /Oy- framed, divergence at instruction #33. Large negative 32-bit immediates (e.g. 0xfffffc18) need an explicit (int) cast before a signed compare, or MSVC types the literal unsigned and turns jge into jae - fixed once, may still be lurking elsewhere unverified.
void __cdecl random_events(int mode) {
    int f;
    int j;
    int v;
    int k;
    int seen;
    if (mode == 0) {
        if (GameDataStateStorage.fields_.global_energy_event_state_ & 2) {
            GameDataStateStorage.fields_.global_energy_event_state_ = 0;
        }
        if (GameDataStateStorage.fields_.global_energy_event_state_ & 1) {
            GameDataStateStorage.fields_.global_energy_event_state_ = 2;
        }
        int spots = GameDataStateStorage.fields_.sunspot_duration_;
        if (spots < -1000) {
            spots = -1000;
        }
        if (GameDataStateStorage.fields_.dust_cloud_duration_ < 0) {
            GameDataStateStorage.fields_.dust_cloud_duration_ = 0;
        }
        GameDataStateStorage.fields_.sunspot_duration_ = --spots;
        if (!spots) {
            POP2("NOMORESPOTS", "space_sm.pcx", -1);
        }
        for (int column = 1; column < 8; column++) {
            if (GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & (1 << column)) {
                for (int row = 1; row < 8; row++) {
                    if (!(GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & (1 << row))
                        && !(PlayersData[row].diplo_treaties[column] & 0x30)) {
                        set_treaty(row, column, 0xC00, 0);
                    }
                }
            }
        }
        if (GameDataStateStorage.fields_.dust_cloud_duration_ > 0) {
            if (!--GameDataStateStorage.fields_.dust_cloud_duration_) {
                POP2("NOMOREDUST", "stars_sm.pcx", -1);
            }
        }
        for (int i = 0; i < GameDataStateStorage.fields_.base_current_count_; i++) {
            if (Bases[i].event & 0x7E00) {
                if (Bases[i].random_event_turns >= 0) {
                    Bases[i].random_event_turns--;
                }
                if (!Bases[i].random_event_turns) {
                    Bases[i].event &= 0xFFFF81FF;
                }
            }
        }
        if (GameDataStateStorage.fields_.game_state_ & 0x100000) {
            if (!(((int)game_year(GameDataStateStorage.fields_.turn_current_num_) - 2210) % 80)) {
                GameDataStateStorage.fields_.game_state_ &= ~0x100000;
                POP2("PERIHELIONENDS", "stars_sm.pcx", -1);
                return;
            }
        }
    }
    if (GameDataStateStorage.fields_.turn_current_num_ > 50 && !mode) {
        if (!(((int)game_year(GameDataStateStorage.fields_.turn_current_num_) - 2190) % 80)) {
            GameDataStateStorage.fields_.game_state_ |= 0x100000;
            POP2("PERIHELION", "heat_sm.pcx", -1);
            return;
        }
    }
    if (GameDataStateStorage.fields_.game_rules_ & 0x8000) {
        return;
    }
    if (GameDataStateStorage.fields_.turn_current_num_ < 75 - GameDataStateStorage.fields_.diff_level_current_ * 10) {
        return;
    }
    if (!mode) {
        random_reseed(MapRandSeed + GameDataStateStorage.fields_.turn_current_num_ * 13);
    }
    int base_id = random(0, max(GameDataStateStorage.fields_.base_current_count_, 100));
    if (base_id >= GameDataStateStorage.fields_.base_current_count_) {
        return;
    }
    Base &base = Bases[base_id];
    if (base.population_size <= 3) {
        return;
    }
    if (PlayersData[base.faction_id_current].current_num_bases <= 1) {
        return;
    }
    if (base.event & 0x7E00) {
        return;
    }
    const int event = random(0, 22);
    const int faction = base.faction_id_current;
    const int x = base.x;
    const int y = base.y;
    parse_num(0, 10);
    parse_says(0, base.name_string, -1, -1);
    StringTemp[0] = '\0';
    switch (event) {
    case 0:
        if (GameDataStateStorage.fields_.faction_rankings_[7] == (uint32_t)faction
            && (!is_human(faction) || PlayersData[faction].diff_level > 1)
            && random(0, 4)) {
            return;
        }
        base.event |= BEVENT_BUMPER;
        strcat(StringTemp, "BUMPER");
        if (!strlen(StringTemp)) {
            return;
        }
        if (base.event & 0x7E00) {
            base.random_event_turns = 10;
        }
        if (faction == ConsoleGlobal.faction_id_ || IsMultiplayerPBEM) {
            POP2(StringTemp, "bump_sm.pcx", base_id);
        }
        return;
    case 1:
        if (PlayersData[faction].ranking < (GameDataStateStorage.fields_.turn_current_num_ >= 150 ? 5 : 4)) {
            return;
        }
        base.event |= BEVENT_FAMINE;
        strcat(StringTemp, "FAMINE");
        if (!strlen(StringTemp)) {
            return;
        }
        if (base.event & 0x7E00) {
            base.random_event_turns = 10;
        }
        if (faction == ConsoleGlobal.faction_id_ || IsMultiplayerPBEM) {
            if (Players[faction].rule_flags & RFLAG_ALIEN) {
                POP2(StringTemp, "al_starv_sm.pcx", base_id);
            } else {
                POP2(StringTemp, "starv_sm.pcx", base_id);
            }
        }
        return;
    case 2:
        if (GameDataStateStorage.fields_.faction_rankings_[7] == (uint32_t)faction
            && (!is_human(faction) || PlayersData[faction].diff_level > 1)
            && random(0, 4)) {
            return;
        }
        base.event |= BEVENT_INDUSTRY;
        strcat(StringTemp, "INDUSTRY");
        if (!strlen(StringTemp)) {
            return;
        }
        if (base.event & 0x7E00) {
            base.random_event_turns = 10;
        }
        if (faction == ConsoleGlobal.faction_id_ || IsMultiplayerPBEM) {
            POP2(StringTemp, "indbm_sm.pcx", base_id);
        }
        return;
    case 3:
        if (PlayersData[faction].ranking < (GameDataStateStorage.fields_.turn_current_num_ >= 150 ? 5 : 4)) {
            return;
        }
        base.event |= BEVENT_BUST;
        strcat(StringTemp, "BUST");
        if (!strlen(StringTemp)) {
            return;
        }
        if (base.event & 0x7E00) {
            base.random_event_turns = 10;
        }
        if (faction == ConsoleGlobal.faction_id_ || IsMultiplayerPBEM) {
            POP2(StringTemp, "genwarning_sm.pcx", base_id);
        }
        return;
    case 4:
        if (GameDataStateStorage.fields_.faction_rankings_[7] == (uint32_t)faction
            && (!is_human(faction) || PlayersData[faction].diff_level > 1)
            && random(0, 4)) {
            return;
        }
        base.event |= BEVENT_HEAT_WAVE;
        strcat(StringTemp, "HEATWAVE");
        if (!strlen(StringTemp)) {
            return;
        }
        if (base.event & 0x7E00) {
            base.random_event_turns = 10;
        }
        if (faction == ConsoleGlobal.faction_id_ || IsMultiplayerPBEM) {
            POP2(StringTemp, "heat_sm.pcx", base_id);
        }
        return;
    case 5:
        if (PlayersData[faction].ranking < (GameDataStateStorage.fields_.turn_current_num_ >= 150 ? 5 : 4)) {
            return;
        }
        base.event |= BEVENT_CLOUD_COVER;
        strcat(StringTemp, "CLOUDCOVER");
        if (!strlen(StringTemp)) {
            return;
        }
        if (base.event & 0x7E00) {
            base.random_event_turns = 10;
        }
        if (faction == ConsoleGlobal.faction_id_ || IsMultiplayerPBEM) {
            POP2(StringTemp, "cloud_sm.pcx", base_id);
        }
        return;
    case 6:
        if (has_fac(FAC_RESEARCH_HOSPITAL, base_id, 0)
            || has_fac(FAC_NANOHOSPITAL, base_id, 0)) {
            if (has_fac(FAC_NANOHOSPITAL, base_id, 0)) {
                parse_say(1, Facility[FAC_NANOHOSPITAL].name, -1, -1);
            } else {
                parse_say(1, Facility[FAC_RESEARCH_HOSPITAL].name, -1, -1);
            }
            if (faction == ConsoleGlobal.faction_id_) {
                ConsoleGlobal.focus(base.x, base.y, ConsoleGlobal.faction_id_);
                POP2("PROMETHEUS1", "biohazd_sm.pcx", base_id);
            } else if (IsMultiplayerPBEM) {
                POP2("PROMETHEUS1", "biohazd_sm.pcx", base_id);
            }
        } else if (has_project(SP_HUMAN_GENOME_PROJ, faction)
                   || has_project(SP_LONGEVITY_VACCINE, faction)
                   || has_project(SP_CLINICAL_IMMORTALITY, faction)) {
            if (has_project(SP_HUMAN_GENOME_PROJ, faction)) {
                parse_say(1, Facility[FAC_HUMAN_GENOME_PROJ].name, -1, -1);
            } else if (has_project(SP_LONGEVITY_VACCINE, faction)) {
                parse_say(1, Facility[FAC_LONGEVITY_VACCINE].name, -1, -1);
            } else {
                parse_say(1, Facility[FAC_CLINICAL_IMMORTALITY].name, -1, -1);
            }
            if (faction == ConsoleGlobal.faction_id_) {
                ConsoleGlobal.focus(base.x, base.y, ConsoleGlobal.faction_id_);
                POP2("PROMETHEUS2", "biohazd_sm.pcx", base_id);
            } else if (IsMultiplayerPBEM) {
                POP2("PROMETHEUS2", "biohazd_sm.pcx", base_id);
            }
        } else {
            if (PlayersData[faction].ranking < (GameDataStateStorage.fields_.turn_current_num_ >= 150 ? 5 : 4)) {
                return;
            }
            int radius = (base.population_size + 1) / 2;
            parse_num(1, radius);
                        if (base.faction_id_current == ConsoleGlobal.faction_id_
                || (base.visibility & (1 << ConsoleGlobal.faction_id_))) {
                seen = 1;
                ConsoleGlobal.focus(base.x, base.y, ConsoleGlobal.faction_id_);
            } else {
                seen = 0;
            }
            for (int i = 0; i < GameDataStateStorage.fields_.base_current_count_; i++) {
                if (map_range(Bases[i].x, Bases[i].y, base.x, base.y) <= radius
                    && !has_fac(FAC_RESEARCH_HOSPITAL, i, 0)
                    && !has_fac(FAC_NANOHOSPITAL, i, 0)
                    && !has_project(SP_HUMAN_GENOME_PROJ, Bases[i].faction_id_current)
                    && !has_project(SP_LONGEVITY_VACCINE, Bases[i].faction_id_current)
                    && !has_project(SP_CLINICAL_IMMORTALITY, Bases[i].faction_id_current)) {
                    Bases[i].population_size = (int8_t)((Bases[i].population_size + 1) / 2);
                    draw_tile(Bases[i].x, Bases[i].y, 2);
                    if (Bases[i].faction_id_current == ConsoleGlobal.faction_id_) {
                        ConsoleGlobal.focus(Bases[i].x, Bases[i].y, ConsoleGlobal.faction_id_);
                        seen = i + 1;
                    } else if (IsMultiplayerPBEM) {
                        seen = i + 1;
                    }
                }
            }
            if (seen) {
                POP2("PROMETHEUS0", "biohazd_sm.pcx", seen - 1);
            }
        }
        return;
    case 7:
    case 8: {
        if (IsMultiplayerPBEM) {
            return;
        }
        int pick = random(0, 20) + 1;
        int found = 0;
        int i;
        int tx, ty;
        for (i = 0; i < 9; i++) {
            tx = wrap_map_x(base.x + RadiusOffsetX[i]);
            ty = base.y + RadiusOffsetY[i];
            if ((ty >= 0 && ty < (int)MapLatitudeBounds && tx >= 0 && tx < (int)MapLongitudeBounds) && bonus_at(tx, ty, 0)
                && ((PlayersData[faction].flags & PFLAG_MAP_REVEALED)
                    || (map_loc(tx, ty)->visibility & (1 << faction)))) {
                int owner;
                if (is_ocean(tx, ty)
                    || (owner = whose_territory(faction, tx, ty, 0, 0)) < 0
                    || owner == faction) {
                    pick = i;
                    found = 1;
                    break;
                }
            }
        }
        tx = wrap_map_x(base.x + RadiusOffsetX[pick]);
        ty = base.y + RadiusOffsetY[pick];
        if (!on_map(tx, ty)) {
            return;
        }
        if (!(PlayersData[faction].flags & PFLAG_MAP_REVEALED)
            && !(map_loc(tx, ty)->visibility & (1 << faction))) {
            return;
        }
        if (!is_ocean(tx, ty)) {
            int owner = whose_territory(faction, tx, ty, 0, 0);
            if (owner >= 0 && owner != faction) {
                return;
            }
        }
        if (found && bonus_at(tx, ty, 0)) {
            if (GameDataStateStorage.fields_.turn_current_num_ < 75) {
                return;
            }
            {
                const int bonus = bonus_at(tx, ty, 0) - 1;
                parse_say(1, Resource[bonus].name_plural, -1, -1);
            }
            bit_set(tx, ty, BIT_RSC_BONUS, 0);
            synch_bit(tx, ty, faction);
            draw_tile(tx, ty, 2);
            if (bonus_at(tx, ty, 0)) {
                return;
            }
            if (faction != ConsoleGlobal.faction_id_) {
                return;
            }
            console_map_win()->set_center(tx, ty, 1);
            if (!HIBYTE(GetAsyncKeyState(VK_SHIFT))) {
                for (i = 0; i < 10; i++) {
                    bit_set(tx, ty, BIT_RSC_BONUS, 1);
                    synch_bit(tx, ty, faction);
                    draw_tile(tx, ty, 2);
                    if (!HIBYTE(GetAsyncKeyState(VK_SHIFT)) && !IsMultiplayerNet) {
                        clock_wait(20);
                    }
                    bit_set(tx, ty, BIT_RSC_BONUS, 0);
                    synch_bit(tx, ty, faction);
                    draw_tile(tx, ty, 2);
                    if (!HIBYTE(GetAsyncKeyState(VK_SHIFT)) && !IsMultiplayerNet) {
                        clock_wait(20);
                    }
                }
            }
            if (!HIBYTE(GetAsyncKeyState(VK_SHIFT)) && !IsMultiplayerNet) {
                clock_wait(100);
            }
            POP2("PETERSOUT", "genwarning_sm.pcx", -1);
            return;
        }
        bit_set(tx, ty, BIT_RSC_BONUS, 1);
        synch_bit(tx, ty, faction);
        draw_tile(tx, ty, 2);
        pick = bonus_at(tx, ty, 0) - 1;
        if (pick < 0) {
            return;
        }
        if (faction != ConsoleGlobal.faction_id_) {
            return;
        }
        console_map_win()->set_center(tx, ty, 1);
        if (!HIBYTE(GetAsyncKeyState(VK_SHIFT))) {
            for (i = 0; i < 10; i++) {
                bit_set(tx, ty, BIT_RSC_BONUS, 0);
                synch_bit(tx, ty, faction);
                draw_tile(tx, ty, 2);
                if (!HIBYTE(GetAsyncKeyState(VK_SHIFT)) && !IsMultiplayerNet) {
                    clock_wait(20);
                }
                bit_set(tx, ty, BIT_RSC_BONUS, 1);
                synch_bit(tx, ty, faction);
                draw_tile(tx, ty, 2);
                if (!HIBYTE(GetAsyncKeyState(VK_SHIFT)) && !IsMultiplayerNet) {
                    clock_wait(20);
                }
            }
        }
        parse_say(1, Resource[pick].name_plural, -1, -1);
        int ocean = is_ocean(tx, ty);
        // The image keeps the unreferenced energy pictures: after the
        // nutrient test the nested `pick ?` arm is folded to the minerals one.
        const char *pic = !pick ? (ocean ? "rdneuw_sm.pcx" : "rdneul_sm.pcx")
                        : pick ? (ocean ? "rdminw_sm.pcx" : "rdminl_sm.pcx")
                        : (ocean ? "rdengw_sm.pcx" : "rdengl_sm.pcx");
        POP2("NEWRESOURCE", pic, -1);
        return;
    }
    case 9: {
        if (GameDataStateStorage.fields_.global_energy_event_state_) {
            return;
        }
        if (random(0, 5)) {
            return;
        }
        int changed = 0;
        for (f = 1; f < MaxPlayerNum; f++) {
            if (PlayersData[f].satellites_[2] || PlayersData[f].satellites_[3]) {
                PlayersData[f].satellites_[2] = 0;
                PlayersData[f].satellites_[3] = 0;
                changed = 1;
            }
        }
        GameDataStateStorage.fields_.global_energy_event_state_ = 1;
        if (changed) {
            POP2("SOLARSTORM", "sun_sm.pcx", -1);
        } else {
            POP2("SOLARFLARE", "heat_sm.pcx", -1);
        }
        return;
    }
    case 10: {
        if (random(0, 5)) {
            return;
        }
        int changed = 0;
        for (f = 1; f < MaxPlayerNum; f++) {
            if (PlayersData[f].satellites_[1]) {
                PlayersData[f].satellites_[1] = 0;
                changed = 1;
            }
        }
        if (!changed) {
            return;
        }
        POP2("ASTEROID", "astm_sm.pcx", -1);
        return;
    }
    case 11:
        if (GameDataStateStorage.fields_.turn_current_num_ < 50) {
            return;
        }
        if (GameDataStateStorage.fields_.sunspot_duration_ > -40) {
            return;
        }
        if (GameDataStateStorage.fields_.sunspot_duration_ > -80 && GameDataStateStorage.fields_.turn_current_num_ >= 80 && random(0, 3)) {
            return;
        }
        GameDataStateStorage.fields_.sunspot_duration_ = random(0, 11) + 10;
        parse_num(0, 20);
        POP2("SUNSPOTS", "solar_sm.pcx", -1);
        return;
    case 12: {
        int reserves = PlayersData[faction].energy_reserves;
        if (reserves <= 1000) {
            if (PlayersData[faction].ranking <= 4 && reserves <= 500) {
                PlayersData[faction].energy_reserves = reserves * 2;
                if (faction == ConsoleGlobal.faction_id_) {
                    parse_num(0, reserves * 2);
                    ConsoleGlobal.update_data(0);
                    POP2("ENERGYBOOM", "markbm_sm.pcx", -1);
                }
            }
        } else if (is_human(faction) || reserves > 2000) {
            PlayersData[faction].energy_reserves = reserves / 4;
            if (faction == ConsoleGlobal.faction_id_) {
                parse_num(0, reserves / 4);
                ConsoleGlobal.update_data(0);
                POP2("INFLATION", "markbm_sm.pcx", -1);
            } else if (IsMultiplayerPBEM) {
                parse_num(0, reserves / 4);
                POP2("INFLATION", "markbm_sm.pcx", -1 - faction);
            }
        }
        return;
    }
    case 13: {
        if (GameDataStateStorage.fields_.turn_current_num_ < 75) {
            return;
        }
        if (PlayersData[faction].ranking < (GameDataStateStorage.fields_.turn_current_num_ >= 150 ? 5 : 4)) {
            return;
        }
        seen = 0;
        for (int i = 0; i < 21; i++) {
            int tx = wrap_map_x(base.x + RadiusOffsetX[i]);
            int ty = base.y + RadiusOffsetY[i];
            if (!on_map(tx, ty)) {
                continue;
            }
            if (!(map_loc(tx, ty)->bit & BIT_SOLAR_TIDAL)) {
                continue;
            }
            if (!(PlayersData[faction].flags & PFLAG_MAP_REVEALED)
                && !(map_loc(tx, ty)->visibility & (1 << faction))) {
                continue;
            }
            int owner = whose_territory(faction, tx, ty, 0, 0);
            if (owner >= 0 && owner != faction) {
                continue;
            }
            if (is_ocean(tx, ty)) {
                continue;
            }
            if (IsMultiplayerPBEM) {
                seen = 1;
            } else if (faction == ConsoleGlobal.faction_id_
                       || base.faction_id_current == ConsoleGlobal.faction_id_
                       || (base.visibility & (1 << ConsoleGlobal.faction_id_))) {
                if (!seen) {
                    ConsoleGlobal.focus(x, y, ConsoleGlobal.faction_id_);
                    if (!HIBYTE(GetAsyncKeyState(VK_SHIFT)) && !IsMultiplayerNet) {
                        clock_wait(200);
                    }
                }
                seen |= (faction == ConsoleGlobal.faction_id_) + 1;
            }
            bit_set(tx, ty, BIT_SOLAR_TIDAL, 0);
            synch_bit(tx, ty, faction);
            for (j = 1; j < 8; j++) {
                map_loc(tx, ty)->bit_visible[j - 1] &= ~BIT_SOLAR_TIDAL;
            }
            if (seen) {
                draw_tile(tx, ty, 2);
                if (!HIBYTE(GetAsyncKeyState(VK_SHIFT)) && !IsMultiplayerNet && !IsMultiplayerPBEM) {
                    clock_wait(50);
                }
            }
        }
        if (seen) {
            POP2("HAIL", "hail_sm.pcx", base_id);
        }
        return;
    }
    case 14: {
        if (IsMultiplayerPBEM) {
            return;
        }
        if (GameDataStateStorage.fields_.turn_current_num_ < 75) {
            return;
        }
        if (PlayersData[faction].current_num_bases < 8) {
            return;
        }
        if (GameDataStateStorage.fields_.game_state_ & 0x10) {
            return;
        }
        if (!on_map(GameDataStateStorage.fields_.mount_planet_x_, GameDataStateStorage.fields_.mount_planet_y_)) {
            return;
        }
        if ((map_loc(GameDataStateStorage.fields_.mount_planet_x_, GameDataStateStorage.fields_.mount_planet_y_)->bit2
             & (BIT2_UNK_80000000 | BIT2_VOLCANO)) != BIT2_VOLCANO) {
            return;
        }
        if (map_range(base.x, base.y, GameDataStateStorage.fields_.mount_planet_x_, GameDataStateStorage.fields_.mount_planet_y_) > 5) {
            return;
        }
        if (!(PlayersData[faction].flags & PFLAG_MAP_REVEALED)
            && !(map_loc(GameDataStateStorage.fields_.mount_planet_x_, GameDataStateStorage.fields_.mount_planet_y_)->visibility & (1 << faction))) {
            return;
        }
        if (PlayersData[faction].ranking < 6) {
            return;
        }
        seen = is_known(GameDataStateStorage.fields_.mount_planet_x_, GameDataStateStorage.fields_.mount_planet_y_, ConsoleGlobal.faction_id_);
        for (int i = 0; i < 81; i++) {
            int t = GameDataStateStorage.fields_.mount_planet_x_ + RadiusOffsetX[i];
            int x2 = wrap_map_x(t);
            int y2 = GameDataStateStorage.fields_.mount_planet_y_ + RadiusOffsetY[i];
            if (!on_map(x2, y2)) {
                continue;
            }
            bit_set(x2, y2, (int)0x81E8887C, 0);
            synch_bit(x2, y2, ConsoleGlobal.faction_id_);
            rocky_set(x2, y2, ROCKINESS_ROCKY);
            if ((map_loc(x2, y2)->bit2 & (BIT2_UNK_80000000 | BIT2_VOLCANO)) == BIT2_VOLCANO) {
                for (v = GameDataStateStorage.fields_.vehicle_current_count_ - 1; v >= 0; v--) {
                    if (Vehs[v].x == x2 && Vehs[v].y == y2) {
                        veh_kill(v);
                    }
                }
            } else {
                for (v = GameDataStateStorage.fields_.vehicle_current_count_ - 1; v >= 0; v--) {
                    if (Vehs[v].x == x2 && Vehs[v].y == y2) {
                        Vehs[v].dmg_incurred += range(proto_power(v) - Vehs[v].dmg_incurred, 0, 9999) / 2;
                    }
                }
            }
            int hit = base_at(x2, y2);
            if (hit < 0) {
                continue;
            }
            if (Bases[hit].faction_id_current == ConsoleGlobal.faction_id_
                && Bases[base_id].faction_id_current != ConsoleGlobal.faction_id_) {
                base_id = hit;
                seen = 1;
                parse_says(0, Bases[hit].name_string, -1, -1);
            }
            if (i < 9 && !is_objective(hit)) {
                base_kill(hit);
            } else if (i < 25) {
                Bases[hit].population_size = (Bases[hit].population_size + 3) / 4;
            } else if (i < 49) {
                Bases[hit].population_size = (Bases[hit].population_size + 1) / 2;
            } else {
                Bases[hit].population_size = (Bases[hit].population_size + 1) * 3 / 4;
            }
        }
        if (!seen || !ConsoleGlobal.focus(GameDataStateStorage.fields_.mount_planet_x_, GameDataStateStorage.fields_.mount_planet_y_, ConsoleGlobal.faction_id_)) {
            draw_map(1);
        }
        GameDataStateStorage.fields_.dust_cloud_duration_ = 10;
        popp(ScriptPopupFile, "BIGFATVOLCANO", 0, "volc_sm.pcx", 0);
        return;
    }
    case 15: {
        if (IsMultiplayerPBEM) {
            return;
        }
        if (GameDataStateStorage.fields_.turn_current_num_ < 75) {
            return;
        }
        if (PlayersData[faction].current_num_bases < 8) {
            return;
        }
        if (GameDataStateStorage.fields_.game_state_ & 0x10) {
            return;
        }
        if (near_landmark(x, y)) {
            return;
        }
        seen = is_known(x, y, ConsoleGlobal.faction_id_);
        int water = 0;
        int i;
        for (i = 0; i < 49; i++) {
            int x2 = wrap_map_x(x + RadiusOffsetX[i]);
            int y2 = y + RadiusOffsetY[i];
            if (on_map(x2, y2) && is_ocean(x2, y2)) {
                if (i < 25) {
                    return;
                }
                if (++water > 5) {
                    return;
                }
            }
        }
        if (PlayersData[faction].ranking < 7) {
            return;
        }
        for (i = 0; i < 49; i++) {
            int x2 = wrap_map_x(x + RadiusOffsetX[i]);
            int y2 = y + RadiusOffsetY[i];
            if (!on_map(x2, y2)) {
                continue;
            }
            bit_set(x2, y2, (int)0x81E8887C, 0);
            synch_bit(x2, y2, ConsoleGlobal.faction_id_);
            int hit = base_at(x2, y2);
            if (hit >= 0) {
                if (Bases[hit].faction_id_current == ConsoleGlobal.faction_id_
                    && Bases[base_id].faction_id_current != ConsoleGlobal.faction_id_) {
                    base_id = hit;
                    parse_says(0, Bases[hit].name_string, -1, -1);
                    seen = 1;
                }
                base_kill(hit);
            }
            for (v = GameDataStateStorage.fields_.vehicle_current_count_ - 1; v >= 0; v--) {
                if (Vehs[v].x == x2 && Vehs[v].y == y2) {
                    if (Vehs[v].faction_id == ConsoleGlobal.faction_id_) {
                        seen = 1;
                    }
                    veh_kill(v);
                }
            }
        }
        world_crater(x, y);
        world_climate();
        if (!seen || !ConsoleGlobal.focus(x, y, ConsoleGlobal.faction_id_)) {
            draw_map(1);
        }
        GameDataStateStorage.fields_.dust_cloud_duration_ = 10;
        popp(ScriptPopupFile, "EATTHIS", 0, "astp_sm.pcx", 0);
        return;
    }
    case 16: {
        if (!has_tech(Facility[FAC_BIOLOGY_LAB].preq_tech, faction)) {
            return;
        }
        seen = 0;
        int has_lab = has_fac(FAC_BIOLOGY_LAB, base_id, 0);
        if (!has_lab && PlayersData[faction].ranking < (GameDataStateStorage.fields_.turn_current_num_ >= 150 ? 5 : 4)) {
            return;
        }
        for (int i = 0; i < 21; i++) {
            int x2 = wrap_map_x(x + RadiusOffsetX[i]);
            int y2 = y + RadiusOffsetY[i];
            if (!on_map(x2, y2)) {
                continue;
            }
            if (is_ocean(x2, y2)) {
                continue;
            }
            if (!(map_loc(x2, y2)->bit & (BIT_FOREST | BIT_FARM))) {
                continue;
            }
            if (!(PlayersData[faction].flags & PFLAG_MAP_REVEALED)
                && !(map_loc(x2, y2)->visibility & (1 << faction))) {
                continue;
            }
            int owner = whose_territory(faction, x2, y2, 0, 0);
            if (owner >= 0 && owner != faction) {
                continue;
            }
            if (!has_lab) {
                if (!seen) {
                    ConsoleGlobal.focus(x, y, ConsoleGlobal.faction_id_);
                    if (!HIBYTE(GetAsyncKeyState(VK_SHIFT)) && !IsMultiplayerNet) {
                        clock_wait(200);
                    }
                }
                bit_set(x2, y2, BIT_FOREST | BIT_SOIL_ENRICHER | BIT_FARM, 0);
                synch_bit(x2, y2, faction);
                // The original clears the console faction's word seven times
                // instead of every faction's word.
                for (k = 1; k < 8; k++) {
                    if (map_loc(x2, y2)->bit_visible[ConsoleGlobal.faction_id_ - 1]
                        & (BIT_FOREST | BIT_SOIL_ENRICHER | BIT_FARM)) {
                        map_loc(x2, y2)->bit_visible[ConsoleGlobal.faction_id_ - 1]
                            &= ~(BIT_FOREST | BIT_SOIL_ENRICHER | BIT_FARM);
                    }
                }
                draw_tile(x2, y2, 2);
                if (!HIBYTE(GetAsyncKeyState(VK_SHIFT)) && !IsMultiplayerNet) {
                    clock_wait(50);
                }
            }
            seen = 1;
        }
        if (!seen) {
            return;
        }
        if (has_lab) {
            Bases[base_id].event |= BEVENT_BUMPER;
            Bases[base_id].random_event_turns = 10;
        }
        if (faction == ConsoleGlobal.faction_id_) {
            if (!ConsoleGlobal.focus(base.x, base.y, faction)) {
                draw_radius(base.x, base.y, 2, 2);
            }
        }
        if (faction != ConsoleGlobal.faction_id_ && !IsMultiplayerPBEM) {
            return;
        }
        POP2(has_lab ? "BLIGHT1" : "BLIGHT", "biohazd_sm.pcx", base_id);
        return;
    }
    case 17: {
        if (!has_tech(Facility[FAC_ENERGY_BANK].preq_tech, faction)) {
            return;
        }
        seen = 0;
        int surge1 = has_fac(FAC_ENERGY_BANK, base_id, 0)
            || has_project(SP_PLANETARY_ENERGY_GRID, faction);
        if (!surge1 && PlayersData[faction].ranking < (GameDataStateStorage.fields_.turn_current_num_ >= 150 ? 5 : 4)) {
            return;
        }
        for (int i = 0; i < 21; i++) {
            int x2 = wrap_map_x(x + RadiusOffsetX[i]);
            int y2 = y + RadiusOffsetY[i];
            if (!on_map(x2, y2)) {
                continue;
            }
            if (is_ocean(x2, y2)) {
                continue;
            }
            if (!(map_loc(x2, y2)->bit & BIT_MINE)) {
                continue;
            }
            if (!(PlayersData[faction].flags & PFLAG_MAP_REVEALED)
                && !(map_loc(x2, y2)->visibility & (1 << faction))) {
                continue;
            }
            int owner = whose_territory(faction, x2, y2, 0, 0);
            if (owner >= 0 && owner != faction) {
                continue;
            }
            if (!surge1) {
                if (!seen) {
                    ConsoleGlobal.focus(x, y, ConsoleGlobal.faction_id_);
                    if (!HIBYTE(GetAsyncKeyState(VK_SHIFT)) && !IsMultiplayerNet) {
                        clock_wait(200);
                    }
                }
                bit_set(x2, y2, BIT_MINE, 0);
                synch_bit(x2, y2, faction);
                for (k = 1; k < 8; k++) {
                    if (map_loc(x2, y2)->bit_visible[k - 1] & BIT_MINE) {
                        map_loc(x2, y2)->bit_visible[k - 1] &= ~BIT_MINE;
                    }
                }
                draw_tile(x2, y2, 2);
                if (!HIBYTE(GetAsyncKeyState(VK_SHIFT)) && !IsMultiplayerNet) {
                    clock_wait(50);
                }
            }
            seen = 1;
        }
        if (!seen) {
            return;
        }
        if (surge1) {
            PlayersData[faction].energy_reserves += 50;
        }
        if (faction == ConsoleGlobal.faction_id_) {
            if (!ConsoleGlobal.focus(base.x, base.y, faction)) {
                draw_radius(base.x, base.y, 2, 2);
            }
        }
        if (faction != ConsoleGlobal.faction_id_ && !IsMultiplayerPBEM) {
            return;
        }
        POP2(surge1 ? "SURGE1" : "SURGE", "markbm_sm.pcx", base_id);
        return;
    }
    case 18:
        if (!has_tech(Facility[FAC_NETWORK_NODE].preq_tech, faction)) {
            return;
        }
        if (PlayersData[faction].tech_accumulated < tech_rate(faction) / 4) {
            return;
        }
        if (has_fac(FAC_NETWORK_NODE, base_id, 0)) {
            PlayersData[faction].tech_accumulated = tech_rate(faction);
            if (faction == ConsoleGlobal.faction_id_) {
                ConsoleGlobal.focus(base.x, base.y, faction);
            }
            if (faction != ConsoleGlobal.faction_id_ && !IsMultiplayerPBEM) {
                return;
            }
            POP2("NETBONUS", "markbm_sm.pcx", base_id);
            return;
        }
        if (PlayersData[faction].ranking < (GameDataStateStorage.fields_.turn_current_num_ >= 150 ? 5 : 4)) {
            return;
        }
        PlayersData[faction].tech_accumulated = 0;
        PlayersData[faction].net_random_event = 0;
        if (faction == ConsoleGlobal.faction_id_) {
            ConsoleGlobal.focus(base.x, base.y, faction);
        }
        if (faction != ConsoleGlobal.faction_id_ && !IsMultiplayerPBEM) {
            return;
        }
        POP2("NETCRASH", "netcr_sm.pcx", base_id);
        return;
    case 19:
        if (!has_tech(Facility[FAC_CHILDREN_CRECHE].preq_tech, faction)) {
            return;
        }
        if (has_fac(FAC_CHILDREN_CRECHE, base_id, 0)) {
            int count = 0;
            while (base.nutrient_surplus > 0) {
                if (++count >= 3) {
                    break;
                }
                ++base.population_size;
                set_base(base_id);
                base_compute(1);
            }
            if (!count) {
                return;
            }
            if (faction == ConsoleGlobal.faction_id_) {
                if (!ConsoleGlobal.focus(base.x, base.y, faction)) {
                    draw_tile(base.x, base.y, 2);
                }
            }
            if (faction != ConsoleGlobal.faction_id_ && !IsMultiplayerPBEM) {
                return;
            }
            if (Players[faction].rule_flags & RFLAG_ALIEN) {
                POP2("CRECHE1", "al_pop_sm.pcx", base_id);
            } else {
                POP2("CRECHE1", "pop_sm.pcx", base_id);
            }
            return;
        }
        if (PlayersData[faction].ranking < (GameDataStateStorage.fields_.turn_current_num_ >= 150 ? 5 : 4)) {
            return;
        }
        if (base.assimilation_turns_left) {
            return;
        }
        base.assimilation_turns_left = 5;
        if (faction == ConsoleGlobal.faction_id_) {
            ConsoleGlobal.focus(base.x, base.y, faction);
        }
        if (faction != ConsoleGlobal.faction_id_ && !IsMultiplayerPBEM) {
            return;
        }
        if (Players[faction].rule_flags & RFLAG_ALIEN) {
            POP2("CRECHE", "al_pop_sm.pcx", base_id);
        } else {
            POP2("CRECHE", "pop_sm.pcx", base_id);
        }
        return;
    case 20: {
        if (IsMultiplayerPBEM) {
            return;
        }
        if (GameDataStateStorage.fields_.turn_current_num_ < 75) {
            return;
        }
        if (PlayersData[faction].ranking < (GameDataStateStorage.fields_.turn_current_num_ >= 150 ? 5 : 4)) {
            return;
        }
        seen = 0;
        for (int i = 0; i < 21; i++) {
            int x2 = wrap_map_x(base.x + RadiusOffsetX[i]);
            int y2 = base.y + RadiusOffsetY[i];
            if (!on_map(x2, y2)) {
                continue;
            }
            if (!is_ocean(x2, y2)) {
                continue;
            }
            if (!(map_loc(x2, y2)->bit & BIT_FARM)) {
                continue;
            }
            if (!(PlayersData[faction].flags & PFLAG_MAP_REVEALED)
                && !(map_loc(x2, y2)->visibility & (1 << faction))) {
                continue;
            }
            int owner = whose_territory(faction, x2, y2, 0, 0);
            if (owner >= 0 && owner != faction) {
                continue;
            }
            if ((map_loc(x2, y2)->climate & ~0x1F) < ALT_BIT_SHORE_LINE) {
                continue;
            }
            if (faction == ConsoleGlobal.faction_id_
                || base.faction_id_current == ConsoleGlobal.faction_id_
                || (base.visibility & (1 << ConsoleGlobal.faction_id_))) {
                if (!seen) {
                    ConsoleGlobal.focus(x, y, ConsoleGlobal.faction_id_);
                    if (!HIBYTE(GetAsyncKeyState(VK_SHIFT)) && !IsMultiplayerNet) {
                        clock_wait(200);
                    }
                }
                seen = 1;
            }
            bit_set(x2, y2, BIT_FARM, 0);
            synch_bit(x2, y2, faction);
            for (j = 1; j < 8; j++) {
                map_loc(x2, y2)->bit_visible[j - 1] &= ~BIT_FARM;
            }
            if (seen) {
                draw_tile(x2, y2, 2);
                if (!HIBYTE(GetAsyncKeyState(VK_SHIFT)) && !IsMultiplayerNet) {
                    clock_wait(50);
                }
            }
        }
        if (!seen) {
            return;
        }
        parse_says(0, base.name_string, -1, -1);
        POP2("KELPWIPE", 0, faction == ConsoleGlobal.faction_id_ ? base_id : -1);
        return;
    }
    case 21: {
        if (IsMultiplayerPBEM) {
            return;
        }
        if (GameDataStateStorage.fields_.turn_current_num_ < 75) {
            return;
        }
        if (PlayersData[faction].ranking < (GameDataStateStorage.fields_.turn_current_num_ >= 150 ? 5 : 4)) {
            return;
        }
        seen = 0;
        for (int i = 0; i < 21; i++) {
            int t = base.x + RadiusOffsetX[i];
            int x2 = wrap_map_x(t);
            int y2 = base.y + RadiusOffsetY[i];
            if (!on_map(x2, y2)) {
                continue;
            }
            if (!is_ocean(x2, y2)) {
                continue;
            }
            if (!(map_loc(x2, y2)->bit & BIT_MINE)) {
                continue;
            }
            if (!(PlayersData[faction].flags & PFLAG_MAP_REVEALED)
                && !(map_loc(x2, y2)->visibility & (1 << faction))) {
                continue;
            }
            int owner = whose_territory(faction, x2, y2, 0, 0);
            if (owner >= 0 && owner != faction) {
                continue;
            }
            if ((map_loc(x2, y2)->climate & ~0x1F) < ALT_BIT_SHORE_LINE) {
                continue;
            }
            if (faction == ConsoleGlobal.faction_id_
                || base.faction_id_current == ConsoleGlobal.faction_id_
                || (base.visibility & (1 << ConsoleGlobal.faction_id_))) {
                if (!seen) {
                    ConsoleGlobal.focus(x, y, ConsoleGlobal.faction_id_);
                    if (!HIBYTE(GetAsyncKeyState(VK_SHIFT)) && !IsMultiplayerNet) {
                        clock_wait(200);
                    }
                }
                seen = 1;
            }
            bit_set(x2, y2, BIT_MINE, 0);
            synch_bit(x2, y2, faction);
            for (j = 1; j < 8; j++) {
                map_loc(x2, y2)->bit_visible[j - 1] &= ~BIT_MINE;
            }
            if (seen) {
                draw_tile(x2, y2, 2);
                if (!HIBYTE(GetAsyncKeyState(VK_SHIFT)) && !IsMultiplayerNet) {
                    clock_wait(50);
                }
            }
        }
        if (!seen) {
            return;
        }
        parse_says(0, base.name_string, -1, -1);
        POP2("PLATFORMWIPE", 0, faction == ConsoleGlobal.faction_id_ ? base_id : -1);
        break;
    }
    }
}

static int alien_local_vehicle_visible(int vehicle_id, int faction) {
    if (Vehs[vehicle_id].faction_id == faction && (Vehs[vehicle_id].flags & 0x440) != 0x440) {
        return true;
    }
    if (Vehs[vehicle_id].visibility & (1 << faction)) {
        return true;
    }
    return false;
}

static bool alien_tile_vehicle_visible(int vehicle_id, int faction) {
    return alien_local_vehicle_visible(vehicle_id, faction);
}


// ORIGINAL: 0x00522180 ?alien_fauna@@YAXXZ 0x00522180-0x00522EFC
// RECOVERY_BINDING: v1 {"evidence": ["git:6e78872480ef8b4a486516e322f23e9f8652665d:src/unrecovered/00522180.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "recovery/tasks/game-logic-seed-owner-coverage.json; preserve original extents and historical seed lessons. Ordinary owned literals have pinned content provenance, not exact original data-allocation claims."], "reason": "Recover native-life spread and spawning through real Map/PlayerData/Base/Veh/UI owners. Preserve native-life versus difficulty operands and the complementary signed JL/JGE pair with one saved wrapped limit; the original reproduction tail is unreachable by that pair. Do not invent guards or entry edges. No matching or game acceptance claim."}
// TRIED: near-literal transcription of Ghidra (reliable here, no unaff_* registers, all callees are free functions except MapWin::focus and NetMsg::pop, whose `this` (g_007d3c3c, g_00805338) came from raw disasm since Ghidra drops it); the CONCAT31(...)&0xffffffdf idiom simplified to (uVar13|uVar14)& 0xffffffdf since uVar14's low byte is always 0. Diverges at insn #3 (xor vs mov), likely register-zeroing choice.
void __cdecl alien_fauna() {
    int i;
    int very_early = 0;          // [ebp-0x20], zeroed in the prologue
    int spore_launcher = 0;      // [ebp-0x34], zeroed in the prologue
    int x;
    int y;
    // Function-scope neighbour coordinates: written by the spread scan and the
    // radius scan; the goody spawn below redraws whatever they last held.
    int x2;
    int y2;
    int region;   // live from the transport search into the population tail
    int base_id;  // shared by the spread notice and the sampler

    // Fungus/kelp spread.
    for (i = 0; i < MapArea / (GameDataStateStorage.fields_.turn_current_num_ / 4 + 32); ++i) {
        x = rnd(MapLongitudeBounds, "TREES0");
        y = rnd(MapLatitudeBounds, "TREES1");
        x = (y & 1) - (x & 1) + x;
        if (!on_map(x, y)) continue;

        const BOOL ocean = is_ocean(x, y);
        if (ocean) {
            if (!(map_loc(x, y)->bit & BIT_FARM)) continue;
        } else if (!(map_loc(x, y)->bit & BIT_FOREST)) {
            continue;
        }

        int best_score = 0;
        int best_x = -1;
        int best_y;  // original leaves this uninitialised
        for (int dir = 0; dir < 8; ++dir) {
            int tx = x + VehMoveDeltaX[dir];
            x2 = wrap_map_x(tx);
            y2 = y + VehMoveDeltaY[dir];
            if (!on_map(x2, y2)) continue;
            Map *cand = map_loc(x2, y2);
            if (((cand->climate & ~0x1F) < ALT_BIT_SHORE_LINE) != ocean) continue;
            const int alt = cand->climate >> 5;
            if (alt < 2) continue;
            if (alt > 4 && !(cand->climate & (RAINFALL_RAINY | RAINFALL_MOIST))) continue;
            if ((cand->bit2 & 0x80000002) == 2) continue;
            if ((cand->bit2 & 0x80000040) == 0x40) continue;
            const int score = (3 - ((cand->climate >> 3) & 3)) * 4 - (cand->val3 >> 6);
            if ((cand->val3 & ~0x3F) > 0x40) continue;
            if (cand->bit & (BIT_FOREST | BIT_FARM | BIT_SOLAR_TIDAL | BIT_MINE |
                             BIT_BASE_IN_TILE)) continue;
            if (score > best_score) {
                best_score = score;
                best_x = x2;
                best_y = y2;
            }
        }
        if (best_x < 0) continue;

        Map *spread = map_loc(best_x, best_y);
        const uint32_t spread_bit = ocean ? BIT_FARM : BIT_FOREST;
        if (spread->bit & spread_bit) continue;
        spread->bit = (spread->bit | spread_bit) & ~BIT_FUNGUS;

        for (int faction = 1; faction < MaxPlayerNum; ++faction) {
            if (!(PlayersData[faction].flags & PFLAG_MAP_REVEALED) &&
                !(map_loc(x, y)->visibility & (1 << faction))) continue;
            if (!(map_loc(x, y)->bit_visible[faction - 1] & spread_bit)) continue;
            spread->bit_visible[faction - 1] =
                (spread->bit_visible[faction - 1] | spread_bit) & ~BIT_FUNGUS;
            if (faction != ConsoleGlobal.faction_id_) continue;

            base_id = base_find(x, y, -1, -1, -1, ConsoleGlobal.faction_id_);
            if (base_id >= 0 &&
                Bases[base_id].faction_id_current == ConsoleGlobal.faction_id_ &&
                (BaseFindDist <= 3 ||
                 whose_territory(ConsoleGlobal.faction_id_, x, y, 0, 0) ==
                     ConsoleGlobal.faction_id_)) {
                if (!MapWinTable[0]->focus(x, y)) draw_tiles(x, y, 2);
                parse_says(0, Bases[base_id].name_string, -1, -1);
                if (ocean) POP2("KELPGROWS", "kelp_sm.pcx", -1);
                else POP2("FORESTGROWS", "forgr_sm.pcx", -1);
            } else {
                draw_tiles(x, y, 2);
            }
        }
    }

    if (GameDataStateStorage.fields_.game_rules_ & RULES_SCN_NO_NATIVE_LIFE) return;
    if (GameDataStateStorage.fields_.turn_current_num_ <
        ((3 - MapNativeLifeForms) * 10 - GameDataStateStorage.fields_.diff_level_current_) * 2) {
        very_early = 1;
    }
    if (GameDataStateStorage.fields_.secret_project_.voice_of_planet() != -1) return;
    if (GameDataStateStorage.fields_.vehicle_current_count_ >= 0x700) return;

    // Sea transport (Isle of the Deep / Sealurk) drop.
    if (GameDataStateStorage.fields_.turn_current_num_ % ((5 - MapNativeLifeForms) * 2) == 0) {
        int tries = 0;
        // The counter advances before the tests, so the 1000th sample is
        // never tested. Accept: fungus land, or an unoccupied ocean tile in a
        // valid region of at least 50 tiles.
        do {
            x = rnd(MapLongitudeBounds, "ALIEN0");
            y = rnd(MapLatitudeBounds, "ALIEN1");
            x = (y & 1) - (x & 1) + x;  // no on_map check in the original
            region = map_loc(x, y)->region;
            if (++tries >= 1000) break;
        } while (!(((map_loc(x, y)->bit & BIT_FUNGUS) &&
                    (map_loc(x, y)->climate & ~0x1F) >= ALT_BIT_OCEAN_SHELF) ||
                   (is_ocean(x, y) && anything_at(x, y) < 0 && !bad_reg(region) &&
                    Continents[region].tile_count >= 50)));

        if (tries < 1000) {
            int cargo = ((GameDataStateStorage.fields_.game_state_ & STATE_PERIHELION_ACTIVE) ? 2 : 1) *
                        GameDataStateStorage.fields_.turn_current_num_ /
                        (((8 - MapNativeLifeForms) * 4 -
                          GameDataStateStorage.fields_.diff_level_current_) * 4) +
                        MapNativeLifeForms + 1;
            if (very_early) cargo = 1;

            // The image duplicates the tech test in both arms (only the
            // veh_init(10) call is tail-merged).
            if (ExpansionEnabled && (timeGetTime() & 1)) {
                const int veh_id = veh_init(14, 0, x, y);
                if (veh_id >= 0) {
                    veh_promote(veh_id);
                    if (GameDataStateStorage.fields_.game_tech_achieved_[TECH_CENTPSI] ||
                        GameDataStateStorage.fields_.game_tech_achieved_[TECH_QUANTUM]) {
                        veh_init(10, 0, x, y);
                    }
                }
            } else {
                const int veh_id = veh_init(9, 0, x, y);
                if (veh_id >= 0) {
                    // min(): veh_cargo is evaluated twice when it is the smaller.
                    cargo = cargo < veh_cargo(veh_id) ? cargo : veh_cargo(veh_id);
                    for (; cargo > 0; --cargo) {
                        int spawn_id;
                        if (rand() % 5 == 0 && ExpansionEnabled) {
                            spawn_id = veh_init(15, 0, x, y);
                        } else {
                            spawn_id = veh_init(8, 0, x, y);
                        }
                        sleep(spawn_id);
                    }
                    veh_promote(veh_id);
                    if (GameDataStateStorage.fields_.game_tech_achieved_[TECH_CENTPSI] ||
                        GameDataStateStorage.fields_.game_tech_achieved_[TECH_QUANTUM]) {
                        veh_init(10, 0, x, y);
                    }
                }
            }
        }
    }

    // Native-life sampler. A do/while: no entry test on the limit.
    int attempt = 0;
    do {
        x = rnd(MapLongitudeBounds, "Alien2x");
        y = rnd(MapLatitudeBounds, "Alien2y");
        x = (y & 1) - (x & 1) + x;  // no on_map check in the original
        Map *tile = map_loc(x, y);
        if ((tile->climate & ~0x1F) < ALT_BIT_SHORE_LINE) continue;
        if (!(tile->bit & BIT_FUNGUS)) continue;
        if ((tile->climate & ~0x1F) < ALT_BIT_OCEAN_SHELF) continue;
        region = tile->region;
        if (bad_reg(region)) continue;
        if (base_who(x, y) >= 0) continue;

        base_id = base_find(x, y, -1, region, -1, -1);
        const int owner = veh_who(x, y);
        if (owner > 0 && (base_id < 0 || BaseFindDist > 2)) {
            // Spawn beside an unaware faction's unit, out of its sight.
            for (i = 0; i < 16; ++i) {
                x2 = wrap_map_x(x + RadiusOffsetX[9 + i]);
                y2 = y + RadiusOffsetY[9 + i];
                if (!on_map(x2, y2)) continue;
                if (is_ocean(x2, y2)) continue;
                if (PlayersData[owner].flags & PFLAG_MAP_REVEALED) continue;
                if (map_loc(x2, y2)->visibility & (1 << owner)) continue;
                if (anything_at(x2, y2) >= 0) continue;
                if (zoc_any(x2, y2, 0)) continue;
                int veh_id;
                if (rand() % 5 == 0 && ExpansionEnabled) {
                    veh_id = veh_init(15, 0, x2, y2);
                } else {
                    veh_id = veh_init(8, 0, x2, y2);
                }
                if (veh_id >= 0 &&
                    alien_tile_vehicle_visible(veh_id, ConsoleGlobal.faction_id_)) {
                    draw_tile(x2, y2, 2);
                }
                break;
            }
        }
        if (owner >= 0 ||
            BaseFindDist <= 1 || BaseFindDist >= 6 ||
            !(map_loc(x, y)->bit & BIT_FUNGUS) ||
            (map_loc(x, y)->climate & ~0x1F) < ALT_BIT_OCEAN_SHELF ||
            goody_at(x, y)) continue;
        int veh_id;
        if (rand() % 5 == 0 && ExpansionEnabled) {
            veh_id = veh_init(15, 0, x, y);
        } else {
            veh_id = veh_init(8, 0, x, y);
        }
        // Original bug: redraws the stale (x2, y2), not the spawn tile.
        if (veh_id >= 0 &&
            alien_tile_vehicle_visible(veh_id, ConsoleGlobal.faction_id_)) {
            draw_tile(x2, y2, 2);
        }
    } while (++attempt < MapNativeLifeForms * 2 + 2);

    // Unreachable: the loop only exits with attempt at the limit.
    if (attempt >= MapNativeLifeForms * 2 + 2) return;

    if (!very_early) {
        if (GameDataStateStorage.fields_.game_tech_achieved_[TECH_CENTPSI] ||
            GameDataStateStorage.fields_.game_tech_achieved_[TECH_QUANTUM]) {
            veh_init(10, 0, x, y);
        }
    }
    // Unguarded Bases[base_id] read.
    int count = very_early ? 1 : (Bases[base_id].population_size *
                 ((GameDataStateStorage.fields_.game_state_ & STATE_PERIHELION_ACTIVE) ? 2 : 1) *
                 (MapNativeLifeForms + 1) + 3) / 4;

    int direction = 21;
    int notice_sent = 0;
    for (attempt = 0; attempt < count; ++attempt) {
        do {
            direction = (direction + 1) % 21;
            int tx = x + RadiusOffsetX[direction];
            x2 = wrap_map_x(tx);
            y2 = y + RadiusOffsetY[direction];
        } while (!on_map(x2, y2) ||
                 !(map_loc(x2, y2)->bit & BIT_FUNGUS) ||
                 (map_loc(x2, y2)->climate & ~0x1F) < ALT_BIT_OCEAN_SHELF ||
                 map_loc(x2, y2)->region != region ||
                 anything_at(x2, y2) > 0);

        int veh_id;
        if (rand() % 5 == 0 && ExpansionEnabled) {
            veh_id = veh_init(15, 0, x2, y2);
            spore_launcher = 1;
        } else {
            veh_id = veh_init(8, 0, x2, y2);
        }
        // No veh_id < 0 check here in the original.
        if (alien_tile_vehicle_visible(veh_id, ConsoleGlobal.faction_id_)) {
            draw_tile(x2, y2, 2);
        }
        if (!notice_sent &&
            Bases[base_id].faction_id_current == ConsoleGlobal.faction_id_ &&
            is_known(x2, y2, ConsoleGlobal.faction_id_) &&
            alien_tile_vehicle_visible(veh_id, ConsoleGlobal.faction_id_)) {
            notice_sent = 1;
            parse_says(0, Bases[base_id].name_string, -1, -1);
            NetMsgPrimary.pop("XMINDWORMS", -5000, 0,
                              spore_launcher ? "sporlnch_sm.pcx" : "mindworm_sm.pcx");
        }
    }
}

// ORIGINAL: 0x00522F00 ?do_fungal_towers@@YAXXZ 0x00522F00-0x005230DD BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence":["Pinned native instructions at 0x00522F00..0x005230DD: descending vehicle traversal, random spread checks, radius offsets, per-faction synchronization and 0x0028805C clear mask.","source:src/veh.h Veh/Vehs and source:src/map.h map, bit, synchronization and radius owners."],"reason":"Recover fungal tower spread and local clearing in the vehicle owner through real vehicle/map state and synchronization calls; preserve original random order and seed identity, without claiming matching or runtime acceptance."}
void __cdecl do_fungal_towers() {
    for (int vehicle_id = GameDataStateStorage.fields_.vehicle_current_count_ - 1;
         vehicle_id >= 0; --vehicle_id) {
        if (Vehs[vehicle_id].proto_id != BSC_FUNGAL_TOWER) {
            continue;
        }

        Vehs[vehicle_id].state |= VSTATE_UNK_4;
        const int x = Vehs[vehicle_id].x;
        const int y = Vehs[vehicle_id].y;

        if (rand() % 5 == 0) {
            const int direction = rand() % 8 + 1;
            const int spread_x = wrap_map_x(x + RadiusOffsetX[direction]);
            const int spread_y = y + RadiusOffsetY[direction];
            if (on_map(spread_x, spread_y)) {
                bit_set(spread_x, spread_y, BIT_FUNGUS, 1);
                // This native mask is 0x0028805C, distinct from the nearby
                // MapTileFlagClearMask data value (0x00288050).
                bit_set(spread_x, spread_y, 0x0028805C, 0);
                synch_bit(spread_x, spread_y, 1);
                synch_bit(spread_x, spread_y, 2);
                synch_bit(spread_x, spread_y, 3);
                synch_bit(spread_x, spread_y, 4);
                synch_bit(spread_x, spread_y, 5);
                synch_bit(spread_x, spread_y, 6);
                synch_bit(spread_x, spread_y, 7);
                draw_tile(spread_x, spread_y, 2);
            }
        }

        bit_set(x, y, BIT_FUNGUS, 1);
        bit_set(x, y, 0x0028805C, 0);
        synch_bit(x, y, 1);
        synch_bit(x, y, 2);
        synch_bit(x, y, 3);
        synch_bit(x, y, 4);
        synch_bit(x, y, 5);
        synch_bit(x, y, 6);
        synch_bit(x, y, 7);
        draw_tile(x, y, 2);

        Vehs[vehicle_id].morale = static_cast<uint8_t>(morale_alien(vehicle_id, 0));
        spot_all(vehicle_id, 1);
    }
}

// WORKAROUND(inline): dead budget probe. Flat inline spend after
// interlude's last early return gives the image's EH state numbering (final
// teardown 0x26..0x29, last early return three states deep instead of four).
// With the four CARETAKE label reads written out (the image pushes the two
// -1 arguments before the label load, hoisted above the branch, which an
// inline label_get argument does not allow) two calls give that numbering,
// one and three do not. Natural spends tried and neutral: clear() for
// the five StringTemp clears, get_title/get_name for the leader parse_says.
static inline int interlude_spend(int x) {
    if (x == 1) return 2;
    return x;
}

// ORIGINAL: 0x005230E0 ?interlude@@YAXHPADHH@Z 0x005230E0-0x00523B4E;0x0065D4F0-0x0065D6C8 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/005230e0.cpp: original identity, complete spans and retained seed history", "source:src/interlude.h: genuine GraphicWin base and embedded member lifetime", "source:recovery/tasks/interlude-lifetime-owner-closure.json: bounded coordinator authorization"], "reason": "Original5230E0 owns exactly one real Interlude local and its inline member destruction, not additional fabricated Buffer/StringStruct/RunResult objects; third/fourth arguments and saved-turn calculations follow original behavior."}
// TRIED: heavily SEH-framed function (local Interlude+Buffer+StringStruct objects, __CxxFrameHandler unwind funclets); Ghidra's own pseudocode has unaff_EBX/ESI/DI and garbled puStackY_* return- address literals from the frame, so this is a raw-disasm transcription instead, using RAII (Interlude popup; Buffer buf; StringStruct strObj;) for the destructor cascade rather than hand-written SEH. A `sub_4066c0` thiscall on an untyped 8-byte local is modeled as a throwaway `InterludeRunResult` class. The two faction-scan loops (best commlink / best base) and the StringStruct remove_all/close cleanup are best-effort approximations, not verified bit-for-bit - the two indirect calls through g_0066931c/g_00669344 (CharUpperA/CharLowerA IAT slots) are called directly instead of through the pointer.
void __cdecl interlude(int id, char *text, int mode, int ending) {
    Interlude popup;
    if (IsMultiplayerNet) return;
    if (DiplomacyActive) return;
    if (id < 32) {
        if ((1 << id) & GameDataStateStorage.fields_.game_interlude_flags_) return;
        if (Players[ConsoleGlobal.faction_id_].rule_flags & RFLAG_ALIEN) return;
    } else {
        if ((1 << (id - 32)) & GameDataStateStorage.fields_.game_interlude_flags_) return;
        if (!(Players[ConsoleGlobal.faction_id_].rule_flags & RFLAG_ALIEN)) return;
    }
    if (GameDataStateStorage.fields_.game_state_ & 0x10) return;
    if (GameDataStateStorage.fields_.game_rules_ & 0x200000) return;
    clear();
    says(Players[ConsoleGlobal.faction_id_].filename);
    CharUpperA(StringTemp);
    if (!strcmp(StringTemp, "FUNGBOY")) return;
    if (id < 32) GameDataStateStorage.fields_.game_interlude_flags_ = GameDataStateStorage.fields_.game_interlude_flags_ | (1 << id);
    if (id == 6) GameDataStateStorage.fields_.game_interlude_flags_ |= 0x12;
    if (GameDataStateStorage.fields_.game_preferences_ & 0x40000) return;

    const int turn = GameDataStateStorage.fields_.turn_current_num_;
    parse_num(0, game_year(turn));
    parse_num(1, game_year(turn));
    parse_num(2, game_year(turn));
    clear();
    if (mode) {
        if (mode == 2) says("EPILOGUE");
        else if (mode == 3) says("EPILOGUE2");
        else if (mode == 4) says("EPILOGUE3");
        else says("INTERLUDE");
        X_pop(InterludeTextFile, StringTemp, nullptr);
    }
    parse_num(1, game_year(turn) - game_year(0));
    if (id == 20) {
        parse_num(1, static_cast<int>(game_year(turn) - game_year(0)) / 100);
    }
    parse_set(Players[ConsoleGlobal.faction_id_].is_leader_female, 0);
    parse_says(0, Players[ConsoleGlobal.faction_id_].title_leader, -1, -1);
    parse_set(Players[ConsoleGlobal.faction_id_].is_leader_female, 0);
    parse_says(1, Players[ConsoleGlobal.faction_id_].name_leader, -1, -1);
    parse_says(8, Players[ConsoleGlobal.faction_id_].assistant_name, -1, -1);
    parse_says(9, Players[ConsoleGlobal.faction_id_].scientist_name, -1, -1);
    clear();
    parse_set(Players[ConsoleGlobal.faction_id_].is_leader_female, 0);
    says(Players[ConsoleGlobal.faction_id_].name_leader);
    CharLowerA(StringTemp);
    parse_says(3, StringTemp, -1, -1);

    int best_value = 0;
    // The original writes this local only when a positive best-base score
    // is found; the later tie-selection loop reads the same local.
    int best_base;
    for (int i = 0; i < GameDataStateStorage.fields_.base_current_count_; ++i) {
        if (Bases[i].faction_id_current == ConsoleGlobal.faction_id_) {
            int value = Bases[i].population_size;
            if (has_fac(FAC_HEADQUARTERS, i, 0)) value += 999;
            if (value > best_value) {
                best_value = value;
                best_base = i;
                parse_says(2, Bases[i].name_string, -1, -1);
            }
        }
    }
    if (!ending) {
        int best = 0;
        for (int i = 0; i < GameDataStateStorage.fields_.base_current_count_; ++i) {
            if (Bases[i].faction_id_current == ConsoleGlobal.faction_id_) {
                int value = Bases[i].population_size + Bases[i].energy_surplus * 2;
                if (i == best_base) value = 1;
                if (value >= best) {
                    best = value;
                    parse_says(4, Bases[i].name_string, -1, -1);
                }
            }
        }
    }
    if (text) parse_says(5, text, -1, -1);
    clear();
    says(Players[ConsoleGlobal.faction_id_].search_key);
    CharUpperA(StringTemp);
    if (strcmp(StringTemp, "CARETAKE")) {
        parse_says(6, StringTable.get(Labels.strings_ptr[1127]), -1, -1);
        parse_says(7, StringTable.get(Labels.strings_ptr[1130]), -1, -1);
    } else {
        parse_says(6, StringTable.get(Labels.strings_ptr[1128]), -1, -1);
        parse_says(7, StringTable.get(Labels.strings_ptr[1129]), -1, -1);
    }
    if (id >= 32) {
        // Restored original behaviour: the image re-tests the faction here
        // and returns for a non-alien player (0x0052391A).
        if (Players[ConsoleGlobal.faction_id_].rule_flags & RFLAG_ALIEN) {
            id -= 32;
            if (id == 3) GameDataStateStorage.fields_.game_interlude_flags_ |= 0x10;
        } else return;
    } else {
        if (Players[ConsoleGlobal.faction_id_].rule_flags & RFLAG_ALIEN) return;
    }
    interlude_spend(0);
    interlude_spend(0);
    GameDataStateStorage.fields_.game_interlude_flags_ |= 1 << id;
    clear();
    says("INTERLUDE");
    say_num(id);
    popup.exec(StringTemp);
}

// ORIGINAL: 0x00523B50 ?ascent@@YAXXZ 0x00523B50-0x00523C58 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:pact-ascent-owner", "pinned:0x00523B50 original complete span; Player59C fields and real NetMsgPrimary/ConsoleGlobal owners.", "pinned:68E62C PACTTREATY, ASCENT and asctran_sm.pcx text inputs; source literals, never original-image calls.", "pinned:525F08..52600E same turn_upkeep ascent path; plural0, leader gender and console-faction reload after parser; bit2000 unnamed, not PFLAG_MAP_REVEALED200.", "historical seed:src/recovered/units/00523b50.cpp complete mapping/findings retained.", "recovery/tests/test_pact_ascent.py actual-source callback/damage controls, not native layout or runtime acceptance."], "reason": "Recover the complete announcement/turn-flow function using real source-owned players, console, parser state and callbacks. Retain original identity/spans and seed findings; live faction reloads and grammar come from full pinned instructions. No byte or game acceptance claim."}
// WORKAROUND(inline): the image expands this body into turn_upkeep
// (0x005258C0, sharing its ASCENT and asctran_sm.pcx literals) and also keeps
// this out-of-line copy with no callers; `inline` expands it and dllexport
// (game.h's MEASURED declaration) keeps the copy.
MEASURED inline void __cdecl ascent() {
    if (GameDataStateStorage.fields_.turn_current_num_ == 0x4b &&
        has_tech(Facility[FAC_BIOLOGY_LAB].preq_tech,
                 ConsoleGlobal.faction_id_)) {
        interlude(1, 0, 1, 0);
    }

    if (!ascending(ConsoleGlobal.faction_id_)) {
        return;
    }

    const int faction_for_flag_and_title = ConsoleGlobal.faction_id_;
    if ((PlayersData[faction_for_flag_and_title].flags & 0x2000) != 0) {
        return;
    }
    PlayersData[faction_for_flag_and_title].flags |= 0x2000;

    PluralityDefault = 0;
    GenderDefault = Players[faction_for_flag_and_title].is_leader_female;
    parse_says(0, Players[faction_for_flag_and_title].title_leader, -1, -1);

    const int faction_for_name = ConsoleGlobal.faction_id_;
    PluralityDefault = 0;
    GenderDefault = Players[faction_for_name].is_leader_female;
    parse_says(1, Players[faction_for_name].name_leader, -1, -1);

    // The separate turn_upkeep path at 525FFA/526000 uses these same
    // picture and section keys, corroborating their shared original storage.
    // RECOVERY_LITERAL: v1 0x0068B58C 0x0068B593 ??_C@_06HMLA@ASCENT?$AA@ 0x005258C0
    // RECOVERY_LITERAL: v1 0x0068B57C 0x0068B58B ??_C@_0P@LDGA@asctran_sm?4pcx?$AA@ 0x005258C0
    popp(ScriptPopupFile, "ASCENT", 0, "asctran_sm.pcx", 0);
}

// Session lifecycle flag read by Console::load_proc and the new-game menu.
// RECOVERY_OBJECT: v2 0x009B2074 0x009B2078 ?GameSessionActive@@3HA 0x0058E360
int GameSessionActive;

// ORIGINAL: 0x00523C60 ?set_time_controls@@YAXXZ 0x00523C60-0x00523DC9
// RECOVERY_BINDING: v1 {"evidence": ["task:time-control-calculation-and-dialog", "historical seed:src/recovered/units/00523c60.cpp", "Pinned original native instructions and existing typed source layout; seed aliases independently reconciled."], "reason": "Recover complete turn-duration calculation through typed rules, faction and vehicle records and actual NetDaemon state; preserve the native bonus-update order and state predicates."}
void __cdecl set_time_controls() {
    int active_factions = 0;
    int eligible_vehicle_support = 0;
    int weighted_base_count = 0;
    int seconds = TimeControl[GameDataStateStorage.fields_.time_controls_selection_].turn;
    const unsigned int active_mask = GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 0xFF;

    for (int faction_id = 1; faction_id < MaxPlayerNum; ++faction_id) {
        if (ConsoleControlTurnActive != 0 && ControlTurnPhase == 0 &&
            TimeControl[GameDataStateStorage.fields_.time_controls_selection_].refresh != 0 &&
            GameDataStateStorage.fields_.turn_current_num_ % TimeControl[GameDataStateStorage.fields_.time_controls_selection_].refresh == 0) {
            int next_bonus = PlayersData[faction_id].time_bonus_count + 1;
            if (next_bonus >= TimeControl[GameDataStateStorage.fields_.time_controls_selection_].accumulated) {
                next_bonus = TimeControl[GameDataStateStorage.fields_.time_controls_selection_].accumulated;
            }
            PlayersData[faction_id].time_bonus_count = next_bonus;
        }

        if ((active_mask & (1u << faction_id)) == 0) {
            continue;
        }

        weighted_base_count += TimeControl[GameDataStateStorage.fields_.time_controls_selection_].base *
            PlayersData[faction_id].current_num_bases;
        ++active_factions;

        for (int vehicle_id = 0; vehicle_id < GameDataStateStorage.fields_.vehicle_current_count_; ++vehicle_id) {
            const Veh &vehicle = Vehs[vehicle_id];
            if (vehicle.faction_id != faction_id ||
                vehicle.order == 1 || vehicle.order == 2) {
                continue;
            }
            if ((vehicle.state & 0x200u) != 0 ||
                (vehicle.state & 0x01000200u) == 0x01000200u ||
                (vehicle.state & 0x4000u) != 0) {
                continue;
            }
            eligible_vehicle_support += TimeControl[GameDataStateStorage.fields_.time_controls_selection_].unit;
        }
    }

    const int event_bonus = TimeControl[GameDataStateStorage.fields_.time_controls_selection_].event *
        static_cast<int>(NetDaemonNet.field_1BD8_);
    if (active_factions != 0) {
        const int candidate =
            (weighted_base_count + eligible_vehicle_support) / active_factions +
            event_bonus;
        if (seconds <= candidate) {
            seconds = candidate;
        }
    }
    if (GameDataStateStorage.fields_.turn_current_num_ <= 5) {
        seconds *= 2;
    }
    GameDataStateStorage.fields_.turn_control_duration_milliseconds_ = seconds * 1000;
}

// base_ecology increments this signed accumulator, scales it in its event
// threshold, then clears it when the threshold is met. setup_game also clears
// it. The adjacent NetTurnFaction is a separate object, not an array element.
// Historical object identity moved to GameDataStateStorage; see the transport manifest.


/*
Purpose: Recompute which faction owns each tile of the map, and rebuild every
         per-faction and per-region tally that ownership feeds.
// ORIGINAL: 0x00523DD0 ?reset_territory@@YAXXZ 0x00523DD0-0x00524202
// LEVER: 9/349 and 0.403 similar -> 158/349 and 0.789 similar, on five measured
//   changes, in this order. (1) `territory_xrange` - a local static helper with no
//   marker of its own - was a real `call`; the image inlines it, and
//   `__forceinline` is what VC6 needs to be told. That alone made call_diff agree
//   (10 calls -> 9) and took 0.403 -> 0.434. (2) DROPPED the `PlayerData &player`
//   binding from the six-array zeroing loop and indexed `PlayersData[faction_id]`
//   at each store: 0.434 -> 0.482 and 21/349 -> 55/349. Binding the row gives VC6
//   a second pointer to strength-reduce, and it then carries the region index as
//   `lea esi, [edx + ecx]` off it, where the image keeps EDX on the two byte
//   arrays and ECX as a plain index (`cmp ecx, 0x80`). (3) The site block is an
//   `if (tile->bit & BIT_BASE_RADIUS) { site = 0; } else { ... }`, NOT
//   `site = 0;` followed by an inverted guard: the image falls THROUGH to the
//   zero and jumps to the val2 read (`test ah, 0x10 / je`). 0.482 -> 0.583.
//   (4) `& ~0x1F` and `& ~0x3F` rather than `& 0xE0` and `& 0xC0` - identical on
//   these `uint8_t` fields, but the complement stops VC6 narrowing the whole test
//   to a byte, which is what puts `xor ecx, ecx / mov cl, [edi] / and ecx,
//   0xFFFFFFE0 / cmp ecx, 0x60 / setl` in place of `and cl, 0xE0 / cmp cl, 0x60 /
//   sbb / neg`. 0.583 -> 0.613, and it brought the compiled instruction count to
//   the image's 349 exactly. (5) `territory_xrange` (since replaced by map.h's
//   wrap_map_x, which measures the same) again, rewritten
//   `if (x < 0) ... else if (x >= bounds) ...`: the image's `jge` falls through to
//   the negative arm, the `if (x >= 0)` spelling inverts it. 0.613 -> 0.630.
//   (6) ONE `Continent *` for the region, reused by the unk_3 store and the pods
//   increment: the image computes `region * 28` once into EDI and addresses both
//   as `[edi + 0x9AA738]` / `[edi + 0x9AA73C]`, while `Continents[region].field`
//   at each site made VC6 recompute the whole lea/sub/shl chain across the
//   intervening `goody_at` call. 0.630 -> 0.789.
// TRIED: measured and rejected on the way. Splitting the rocky/fungus,
//   visibility/is_human and BaseFindDist chains into separate `if`s: all three
//   score exactly what the chained form does. Splitting the owner add_site
//   condition into two arms that each call add_site does raise positional
//   agreement (5 -> 13 at the time) but adds a third add_site call the image does
//   not make, which call_diff then reports as MORE - rejected on that. Hoisting
//   the masked climate/rockiness into `const int` locals: no change, VC6 folds
//   them. A `PlayerData *` walked with `player++` in the zeroing loop: no change.
//   Storing the byte arrays first in that loop: no change. `Continent &` instead
//   of `Continent *`, and a `uint8_t *` for the unk_82 read/modify/write: both
//   identical to what is committed, so they are free spellings, not levers.
// TRIED: what is left is register ASSIGNMENT, 20 diverging runs under
//   /c /O2 /Gy /GR- /Oy- /GX and one instruction short of the image's 349. The
//   faction loop is a straight ESI/EDI swap (image counts in ESI and walks the
//   unk_79 pointer in EDI, this tree the other way round); `region * 2` is CSE'd
//   into `shl eax, 1` by the image and re-encoded as `[eax + eax + disp]` here at
//   all three unk_78/80/81 increments; and `cmp eax, ecx` / `cmp ecx, eax` at the
//   territory-distance test is the same comparison with the operands in the other
//   registers. None of those is reachable from the statement level.
// size      1074 bytes
// prototype
// callers   11   call targets   7
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004E3A50 0x004E3D50 0x00579B70 0x00579D20 0x00591B50 0x00592140 0x005C4FD0
Return Value: n/a
Status: Complete

Ownership is decided by proximity: the nearest base within
Rules.territory_max_dist_base owns the tile, and an OCEAN tile halves that
distance and is then overruled outright by any base in its own eight-tile
ring - which is how coastal water ends up belonging to the base beside it
rather than to a larger one further off. A tile with no base in range is
unclaimed, which the map stores as -1.

Everything else here is tallies. Six per-region arrays are zeroed for all eight
factions before the walk, and the walk refills them:

  - unk_79[region] counts tiles the faction can see in that region - or, for a
    faction whose AI is running the objectives strategy, tiles carrying
    BIT_SUPPLY_REMOVE instead. That substitution is what PFLAG_STRAT_SEARCH_-
    OBJECTIVES selects, and it is a substitution rather than an addition.
  - unk_78[region] counts tiles the faction owns; unclaimed tiles count for
    faction zero.
  - unk_80[region] counts owned tiles that are wet, not rocky and not fungus,
    and unk_81[region] the subset of those already inside a base radius.
  - unk_82[region] is the best world_site score the faction owns there, and
    unk_83[region] the supply pods on its tiles.

Continents[region] gets the same last two facts for the region as a whole, in
unk_3 and pods, which is what those two field comments already say.

The walk is the same shape as num_objectives': one Map entry per tile visited,
never reset between rows, starting each row at `x = y & 1`. An empty row - one
where the start is already past MapLongitudeBounds - does not advance the
pointer at all.

A tile inside a base radius is not scored as a site and is not asked to be: its
score is taken as zero without reading val2. Elsewhere the stored score is used
if there is one and world_site() is called and the answer cached with
site_set() if there is not, so a full map walk costs at most one scoring pass
per tile ever.

The supply-pod arm ends in a site request, and its two branches are not
symmetric. An owned tile asks only its owner; an unclaimed one asks every
faction that has a base in the region, and each of those must additionally not
already hold a site here and must have a base at between one and two times the
territory distance - near enough to want it, far enough not to have it already.
Both branches require the faction either to see the tile or to be an AI above
difficulty 3, which is the standard "the AI is allowed to know" test.
*/
void __cdecl reset_territory() {
    // NO `PlayerData &player` HERE. Binding the faction row to a reference
    // gives VC6 a second live pointer to strength-reduce, and it then carries
    // the region INDEX as a subtraction off it; the image keeps `edx` on the
    // byte pair and `ecx` as the index. Indexing `PlayersData[faction_id]`
    // at each store instead is what reproduces that.
    for (int faction_id = 0; faction_id < MaxPlayerNum; faction_id++) {
        for (int region = 0; region < MaxContinentNum; region++) {
            PlayersData[faction_id].unk_80[region] = 0;
            PlayersData[faction_id].unk_78[region] = 0;
            PlayersData[faction_id].unk_79[region] = 0;
            PlayersData[faction_id].unk_81[region] = 0;
            PlayersData[faction_id].unk_83[region] = 0;
            PlayersData[faction_id].unk_82[region] = 0;
        }
    }
    for (int region = 0; region < MaxContinentNum; region++) {
        Continents[region].pods = 0;
        Continents[region].unk_3 = 0;
    }
    Map *tile = map_tiles();
    for (int y = 0; y < MapLatitudeBounds; y++) {
        for (int x = y & 1; x < MapLongitudeBounds; tile++, x += 2) {
            region = tile->region;
            for (faction_id = 1; faction_id < MaxPlayerNum; faction_id++) {
                uint32_t flags = PlayersData[faction_id].flags;
                if (flags & PFLAG_STRAT_SEARCH_OBJECTIVES) {
                    if (tile->bit & BIT_SUPPLY_REMOVE) {
                        PlayersData[faction_id].unk_79[region]++;
                    }
                } else if (flags & PFLAG_MAP_REVEALED
                    || tile->visibility & (1 << faction_id)) {
                    PlayersData[faction_id].unk_79[region]++;
                }
            }
            int owner = -1;
            int base_id = base_find(x, y, -1, region, -1, -1);
            // `& ~0x1F`, not `& 0xE0`. Identical on a `uint8_t` field - both
            // keep bits 5..7 of a value the promotion has already zero-extended -
            // but the complement is what stops VC6 narrowing the whole test to a
            // byte. The image loads `xor ecx, ecx / mov cl, [edi]`, masks
            // `and ecx, 0xFFFFFFE0` and compares the DWORD `cmp ecx, 0x60 / setl`;
            // `& 0xE0` gives `and cl, 0xE0 / cmp cl, 0x60 / sbb / neg` instead.
            BOOL is_ocean_tile = (tile->climate & ~0x1F) < (ALT_SHORE_LINE << 5);
            if (base_id >= 0) {
                int max_dist = Rules.territory_max_dist_base;
                if (is_ocean_tile) {
                    max_dist >>= 1;
                }
                if (BaseFindDist < max_dist) {
                    owner = Bases[base_id].faction_id_current;
                }
            }
            if (is_ocean_tile) {
                for (int i = 0; i < 9; i++) {
                    int x_adj = wrap_map_x(x + VehMoveDeltaX[i]);
                    int y_adj = y + VehMoveDeltaY[i];
                    if (!on_map(x_adj, y_adj)) {
                        continue;
                    }
                    int adjacent_id = base_at(x_adj, y_adj);
                    if (adjacent_id >= 0) {
                        owner = Bases[adjacent_id].faction_id_current;
                        break;
                    }
                }
            }
            if (owner != tile->territory) {
                tile->bit2 |= 0x400000; // TODO: identify value
                WorldClimateWorldWin.field_239BC_ |= 1;
            }
            tile->territory = (int8_t)owner;
            if (owner < 0) {
                owner = 0;
            }
            PlayersData[owner].unk_78[region]++;
            if (tile->climate & (RAINFALL_MOIST | RAINFALL_RAINY)) {
                // `& ~0x3F` for the same reason as the altitude mask above: the
                // image compares the widened dword (`and ecx, 0xFFFFFFC0 /
                // cmp ecx, 0x80 / jge`), `& 0xC0` compares the byte (`jae`).
                if ((tile->val3 & ~0x3F) < (ROCKINESS_ROCKY << 6)
                    && !(tile->bit & BIT_FUNGUS)) {
                    PlayersData[owner].unk_80[region]++;
                    if (tile->bit & BIT_BASE_RADIUS) {
                        PlayersData[owner].unk_81[region]++;
                    }
                }
            }
            // site_at, the map.h header inline: the expansion gives the
            // image's 0x14 frame (the hand-written if/else had 0x10).
            int site = site_at(x, y, false);
            // `Continents[region]` indexed at each site: the image keeps
            // `region * 28` in EDI as an index (`[edi + 0x9AA738]`). Earlier
            // a `Continent *` beat this; with the loop/BaseFindDist fixes the
            // direct index measures better (0.868 -> 0.974).
            int region_best = Continents[region].unk_3;
            if (site > region_best) {
                region_best = site;
            }
            Continents[region].unk_3 = (uint32_t)region_best;
            if (owner) {
                int owner_best = PlayersData[owner].unk_82[region];
                if (site > owner_best) {
                    owner_best = site;
                }
                PlayersData[owner].unk_82[region] = (uint8_t)owner_best;
            }
            if (!goody_at(x, y)) {
                continue;
            }
            Continents[region].pods++;
            PlayersData[owner].unk_83[region]++;
            if (owner) {
                if (tile->visibility & (1 << owner)
                    || (!is_human(owner) && GameDataStateStorage.fields_.diff_level_current_ > 3)) {
                    add_site(owner, 3, 1, x, y);
                }
                continue;
            }
            for (faction_id = 1; faction_id < MaxPlayerNum; faction_id++) {
                if (!PlayersData[faction_id].region_total_bases[region]) {
                    continue;
                }
                if (!(tile->visibility & (1 << faction_id))
                    && (is_human(faction_id) || GameDataStateStorage.fields_.diff_level_current_ <= 3)) {
                    continue;
                }
                if (at_site(faction_id, 3, x, y)) {
                    continue;
                }
                if (base_find(x, y, faction_id, region, -1, -1) < 0) {
                    continue;
                }
                // BaseFindDist loads first in the image.
                int dist = BaseFindDist;
                int limit = Rules.territory_max_dist_base;
                if (dist < limit || dist > limit + limit) {
                    continue;
                }
                add_site(faction_id, 3, 1, x, y);
            }
        }
    }
}

// ORIGINAL: 0x00524210 ?climactic_changes@@YAXXZ 0x00524210-0x005242C0 FILE BYTE_EXACT
// working copy - scaffold materialised by --work
// size      176 bytes
// prototype
// callers   0   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0046B190 0x0048C0A0 0x005C5A30

// Council shade/melt resolutions write these cells; the turn-phase consumer
// advances the accumulated sea-level change against the same signed counters.
// Historical object identity moved to GameDataStateStorage; see the transport manifest.

// Historical object identity moved to GameDataStateStorage; see the transport manifest.

// Historical object identity moved to GameDataStateStorage; see the transport manifest.

// Historical object identity moved to GameDataStateStorage; see the transport manifest.


// WORKAROUND(inline): the image expands this body into turn_upkeep
// (0x005258C0, sharing its SEARISING/SEAFALLING literals) and also keeps an
// out-of-line copy with no callers. A plain definition is not auto-inlined
// there; `inline` expands it and dllexport keeps the copy (exact both ways).
MEASURED inline void __cdecl climactic_changes() {
    int sign;

    if (GameDataStateStorage.fields_.climate_sea_level_delta_ != 0) {
        GameDataStateStorage.fields_.climate_rise_accum_ += GameDataStateStorage.fields_.climate_rise_step_;
        if (GameDataStateStorage.fields_.climate_rise_accum_ >= GameDataStateStorage.fields_.climate_rise_limit_) {
            GameDataStateStorage.fields_.climate_rise_accum_ -= GameDataStateStorage.fields_.climate_rise_limit_;
            if (GameDataStateStorage.fields_.climate_sea_level_delta_ > 0) {
                sign = 1;
            } else {
                sign = (GameDataStateStorage.fields_.climate_sea_level_delta_ >= 0) - 1;
            }
            MapSeaLevel += sign;
            GameDataStateStorage.fields_.climate_sea_level_delta_ -= sign;
            world_climate();
            draw_map(1);
            if (sign > 0) {
                popp(ScriptPopupFile, "SEARISING", 0, "searis_sm.pcx", 0);
            } else {
                popp(ScriptPopupFile, "SEAFALLING", 0, "seafal_sm.pcx", 0);
            }
        }
    }
}

// The original class spelling, complete object extent and allocation/lifetime
// owner are not established. This private role name groups the receiver
// fields used by the four retained original entries.
// RECOVERY_BASE: v1 Demo GraphicWin 0x0 0x00524460
class Demo : public GraphicWin { public:
    int init();
    virtual void redraw();
    virtual void on_left_down(int x, int y);
    int on_key(int a1, int a2);
    void toggle_and_repaint();
    static void __cdecl timer_callback(int self);

    // RECOVERY_MEMBER: v1 Demo selected_link_ 0xA14 0x005243A0
    int selected_link_;       // observed at +0xA14
    // RECOVERY_MEMBER: v1 Demo timer_phase_ 0xA18 0x00524340
    int timer_phase_;         // observed at +0xA18
    // RECOVERY_MEMBER: v2 Demo timer_ Time 0xA1C 0x00524460
    Time timer_;              // observed at +0xA1C
    // RECOVERY_MEMBER: v4 Demo splash_rects_ tagRECT[3] 0xA44 0x30 0x00524460
    RECT splash_rects_[3];    // observed at +0xA44
    // RECOVERY_MEMBER: v4 Demo splash_sprites_ Sprite[2] 0xA74 0x58 0x005242C0
    Sprite splash_sprites_[2];// observed at +0xA74 and +0xAA0
};

// Four consecutive original dwords at 0x939FF8 are initialized/read as RECT.
// Descriptive source-owned storage; the start binding does not prove the
// complete original allocation or original spelling.
// RECOVERY_OBJECT: v1 0x00939FF8 ?DemoFourthHotspot@@3UtagRECT@@A
RECT DemoFourthHotspot;

static void set_demo_rect(RECT &rect, long left, long top, long right,
                          long bottom) {
    rect.left = left;
    rect.top = top;
    rect.right = right;
    rect.bottom = bottom;
}

// ORIGINAL: 0x005242C0 sub_5242c0 0x005242C0-0x00524317 BYTE_EXACT
// symbol    ?redraw@Demo@@UAEXXZ
// kind      game
// Historical matching used GraphicWinSub::sub_5242c0 with raw receiver offsets;
// the observed Demo fields now own those accesses and the virtual redraw slot.
void Demo::redraw() {
    Buffer *const buffer = static_cast<Buffer *>(this);
    const int frame = timer_phase_ == 0 ? 1 : 0;
    Sprite &sprite = splash_sprites_[frame];
    RECT &area = splash_rects_[0];
    sprite.draw(buffer, sprite.transparent_index(),
                area.left, splash_rects_[0].top, 1, 1);
    soft_update(&area);
}

// ORIGINAL: 0x00524320 sub_524320 0x00524320-0x0052433B BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence":["Pinned 0x00524320-0x0052433B toggles receiver+0xA18 through a zero test and tail-dispatches virtual slot62 at +0xF8.","Pinned 0x00524340-0x00524363 independently toggles the same receiver field and dispatches the same slot; Demo::timer_callback owns this operation through Demo::timer_phase_.","Demo::init at 0x00524460 initializes this phase field and registers the callback with its receiver; Demo::redraw at 0x005242C0 reads that field and draws the two embedded splash sprites.","GraphicWin/Win declare slot62 as void redraw(); no original class spelling, complete allocation extent, historical TU or result-consuming caller is inferred."],"reason":"Express the phase toggle through the existing splash-window Demo source owner and genuine redraw virtual; remove the fabricated Base524320 prefix and raw member-pointer dispatch, retaining the unresolved original result contract."}
// symbol    ?toggle_and_repaint@Demo@@QAEXXZ
// RECOVERY_HISTORY: Previous bounded adapter symbol ?toggle_and_repaint@Base524320@@QAEHXZ returned the residue of a raw slot-62 dispatch; no original result-consuming caller is established.
// size      27 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready
// calls     (none)
// 0x00524320  sub_524320  ->  _sub_524320
//
// Historical emitter note (superseded): this was reported as unowned and
// not compiled. The ordinary Demo member below is now compiled by game.cpp;
// the earlier adapter and its matching history remain in Git.


/*
Purpose: Toggle the flag and repaint through virtual slot 62.
Original Offset: 00524320
Return Value: no established result-consuming caller; redraw is a void virtual
Status: Complete
*/
// Catalogued `extern "C" int __cdecl sub_524320()`, and the disassembly
// disagrees: no stack frame at all, ecx read directly as `this`, and the body
// ends in `jmp [edx+0xf8]` rather than `ret`.
//
// Preserve the zero-test toggle and genuine slot-62 virtual dispatch. The
// tail call's original register residue remains an open ABI obligation.
void Demo::toggle_and_repaint() {
    timer_phase_ = (timer_phase_ == 0);
    redraw();
}

// ORIGINAL: 0x00524340 Demo::timer_callback 0x00524340-0x00524363 BYTE_EXACT
// symbol    ?timer_callback@Demo@@SAXH@Z
// kind      game
// Replaces _fn_00524340's manual slot-62 dispatch. Time passes an integer
// receiver payload and ignores a return value; redraw remains virtual.
void __cdecl Demo::timer_callback(int self) {
    Demo *demo = reinterpret_cast<Demo *>(self);
    demo->timer_phase_ = (demo->timer_phase_ == 0);
    demo->redraw();
}

// RECOVERY_ROLE: SCAFFOLD Fabricated Widget receiver and manual virtual-slot adapter; original class, receiver ownership and lifecycle remain unproved.
// ORIGINAL: 0x00524370 sub_524370 0x00524370-0x00524396 BYTE_EXACT
// symbol    ?on_key@Demo@@QAEHHH@Z
// size      38 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00524387
// 0x00524370  sub_524370  ->  _sub_524370
//
// Historical emitter notes below predate this compiled game.cpp adapter.
// Pinned dispatch is slot58 on the unchanged receiver. Win::release_modal
// has that slot, but no receiver/type relationship establishes a Win or Demo
// owner here. The matching history does not resolve that structural debt.
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.


/*
Purpose: Activate on Enter, Escape or Space, through virtual slot 58.
Original Offset: 00524370
Return Value: one
Status: Complete
*/
// A __thiscall member, not the catalogued `extern "C" int __stdcall`: ecx is
// used directly with no stack load, and `ret 8` pops the two explicit
// arguments a thiscall callee cleans up itself. The first is dead.
int Demo::on_key(int, int a2) {
    // 0xd, 0x1b, 0x20 are VK_RETURN, VK_ESCAPE and VK_SPACE.
    if (a2 == 0xd || a2 == 0x1b || a2 == 0x20) {
        release_modal();
    }
    return 1;
}

// ORIGINAL: 0x005243A0 sub_5243a0 0x005243A0-0x0052445B BYTE_EXACT
// symbol    ?on_left_down@Demo@@UAEXHH@Z
// kind      game
// TRIED: extern "C" __stdcall(int,int) contract (ecx has no matching stack slot, so it is __thiscall); own Obj_5243a0 class (3 embedded RECTs) + VCall slot058 reaches MNEMONIC_ONLY, same 187 bytes, same mnemonics, an operand still differs
void Demo::on_left_down(int x, int y) {
    selected_link_ = 0;
    if (in_box(x, y, &splash_rects_[0]) != 0) {
        selected_link_ = 1;
    } else if (in_box(x, y, &splash_rects_[1]) != 0) {
        selected_link_ = 2;
    } else if (in_box(x, y, &splash_rects_[2]) != 0 ||
               in_box(x, y, &DemoFourthHotspot) != 0) {
        selected_link_ = 3;
    }
    release_modal();
}

// Demo::init's sprite cuts: the size arguments are evaluated right to left,
// height first.
static inline int demo_extract(Sprite *sprite, Buffer *buffer, int x, int y, int width, int height) {
    return sprite->extract(buffer, WinFillColour, x, y, width, height, 0);
}

// ORIGINAL: 0x00524460 Demo__init 0x00524460-0x005247A6;0x0065D6C8-0x0065D6DD BYTE_EXACT
// symbol    ?init@Demo@@QAEHXZ
// kind      game
int Demo::init() {
    Buffer local_buffer;

    const int large_layout = WinScreenWidth != 0x320;
    GraphicWin::init(0, 0, WinScreenWidth, WinScreenHeight, 0, 0);
    load_pcx(const_cast<char *>(
        large_layout ? "demosplash1024.pcx" : "demosplash800.pcx"), 0, 10, 0xEC);

    char *foreground;
    if (large_layout) {
        set_demo_rect(splash_rects_[0], 0x2B, 0x268, 0x18B, 0x2A3);
        set_demo_rect(splash_rects_[1], 0x2DA, 0x28E, 0x36E, 0x2C2);
        set_demo_rect(splash_rects_[2], 0x1A7, 0x1EF, 0x259, 0x2C5);
        // The original global RECT transposes its first two coordinates.
        set_demo_rect(DemoFourthHotspot, 0x243, 0x2DA, 0x30C, 0x2F1);
        foreground = "demosplash1024_2.pcx";
    } else {
        set_demo_rect(splash_rects_[0], 0x21, 0x1E1, 0x135, 0x210);
        set_demo_rect(splash_rects_[1], 0x23A, 0x1FF, 0x2AE, 0x228);
        set_demo_rect(splash_rects_[2], 0x14A, 0x182, 0x1D6, 0x22A);
        set_demo_rect(DemoFourthHotspot, 0x1C5, 0x23A, 0x262, 0x24E);
        foreground = "demosplash800_2.pcx";
    }
    local_buffer.load_pcx(foreground, 0, 10, 0xEC);

    // Both sprites are cut from the first RECT, reloaded after each call.
    demo_extract(&splash_sprites_[0], static_cast<Buffer *>(this),
                 splash_rects_[0].left, splash_rects_[0].top,
                 rect_width(&splash_rects_[0]),
                 rect_height(&splash_rects_[0]));
    demo_extract(&splash_sprites_[1], &local_buffer,
                 splash_rects_[0].left, splash_rects_[0].top,
                 rect_width(&splash_rects_[0]),
                 rect_height(&splash_rects_[0]));
    local_buffer.close();
    timer_.start(&Demo::timer_callback, reinterpret_cast<int>(this), 600, 5);

    selected_link_ = 0;
    timer_phase_ = 0;
    set_modal(0, 0, 0);

    switch (selected_link_) {
    case 1:
        ShowWindow(HandleMain, SW_HIDE);
        ShellExecuteA(0, "open",
            "http:\\\\www.eastore.ea.com\\cgi-bin\\eastore.storefront\\EN\\addtobasket\\10509",
            0, 0, SW_SHOWNORMAL);
        break;
    case 2:
        ShowWindow(HandleMain, SW_HIDE);
        ShellExecuteA(0, "open",
            "http:\\\\www.amazon.com\\exec\\obidos\\ASIN\\B00000DMA7\\ref=ad_cgm1",
            0, 0, SW_SHOWNORMAL);
        break;
    case 3:
        ShowWindow(HandleMain, SW_HIDE);
        ShellExecuteA(0, "open", "http:\\\\www.alphacentauri.com",
                      0, 0, SW_SHOWNORMAL);
        break;
    }
    return selected_link_ != 0;
}

// ORIGINAL: 0x005247B0 ?generators@@YAHHPAH@Z 0x005247B0-0x00524862 BYTE_EXACT
// The generator threshold at 0x00949864 is Rules.size_base_subspace_gen,
// one of RulesBasic's last three fields, not a separate global in a gap.
// The original walk reads Players[].rule_flags and Bases[] ownership,
// population and facility bits. Compare population as signed, as in the image.
int __cdecl generators(int faction_id, int *count_out) {
    if (count_out != 0) {
        *count_out = 0;
    }
    if ((Players[faction_id].rule_flags & RFLAG_ALIEN) == 0) {
        return 0;
    }
    int count = 0;
    for (int index = 0; index < GameDataStateStorage.fields_.base_current_count_; ++index) {
        if (Bases[index].faction_id_current == faction_id && has_fac(0x28, index, 0)) {
            if (Bases[index].population_size
                >= Rules.size_base_subspace_gen) {
                ++count;
            } else if (count_out != 0) {
                ++*count_out;
            }
        }
    }
    return count;
}

// Native game/turn lifecycle owners. Their historical TU boundary is not
// established; these bodies live with the current game/turn source owner.
void __cdecl show_replay();
void __cdecl quayle(int faction_id);



enum EndgameReasonValue {
    EndgameReason1 = 1, EndgameReason2, EndgameReason3, EndgameReason4,
    EndgameReason5, EndgameReason6, EndgameReason7, EndgameReason8,
    EndgameReason9, EndgameReason10, EndgameReason11, EndgameReason12,
    EndgameReason13, EndgameReason14, EndgameReason15, EndgameReason16,
    EndgameReason17, EndgameReason18
};
static const uint32_t EndgameRecorded = STATE_UNK_1;
static const uint32_t EndgamePresented = STATE_UNK_8;
static const uint32_t EndgameNetResults = STATE_IS_SCENARIO;
static const uint32_t EndgameScoreMode = STATE_SCN_VICT_HIGHEST_AC_SCORE_WINS;
static const uint32_t PlayerEndgameFlagA = 0x00020000u;
static const uint32_t PlayerEndgameFlagB = 0x00040000u;
static const uint32_t PlayerGeneratorWin = 0x00100000u;
static const uint32_t PlayerConquestWin = 0x00040000u;
static const uint32_t PlayerConquestLoss = 0x00020000u;

static void parse_player_identity(int faction_id) {
    PluralityDefault = 0;
    GenderDefault = Players[faction_id].is_leader_female;
    parse_says(0, Players[faction_id].title_leader, -1, -1);
    GenderDefault = Players[faction_id].is_leader_female;
    PluralityDefault = 0;
    parse_says(1, Players[faction_id].name_leader, -1, -1);
    GenderDefault = Players[faction_id].noun_gender;
    PluralityDefault = Players[faction_id].is_noun_plural;
    parse_says(2, Players[faction_id].noun_faction, -1, -1);
}


// WORKAROUND(inline): generators(faction_id, NULL) as end_of_game expands it.
// The image inlines generators here but keeps its has_fac call out of line
// (0x00421670); calling generators() lets VC6 expand has_fac as well.
// MEASURED 2026-11-19 (par-1): with generators() called directly, has_fac stays
// expanded under 0-24 dead 4-if probes at the top of end_of_game and an
// inline generator_win predicate owner; 32 probes refuse it only after
// interlude/Lock::clear sites flip first (pass 1 binding). Kept.
static inline int turn_generators(int faction_id, int *count_out) {
    BOOL (__cdecl *const has_facility)(int, int, int) = has_fac;
    if (count_out != 0) {
        *count_out = 0;
    }
    if ((Players[faction_id].rule_flags & RFLAG_ALIEN) == 0) {
        return 0;
    }
    int count = 0;
    for (int index = 0; index < GameDataStateStorage.fields_.base_current_count_; ++index) {
        if (Bases[index].faction_id_current == faction_id && has_facility(0x28, index, 0)) {
            if (Bases[index].population_size
                >= Rules.size_base_subspace_gen) {
                ++count;
            } else if (count_out != 0) {
                ++*count_out;
            }
        }
    }
    return count;
}

// ORIGINAL: 0x00524870 ?end_of_game@@YAHH@Z 0x00524870-0x0052581F BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:58721b2a7cbfa00521e18bec7e70fa3d8437a42d:src/unrecovered/00524870.cpp: retained complete native extent and seed findings", "src/game.h: game state, mission years, result flags and canonical end_of_game declaration", "src/base.h and src/faction.h: objective, faction, ranking and project owners", "src/netdaemon.h: original three-entry lock-state cleanup through typed members", "recovery/tasks/endgame-owner-oct5.json", "recovery/tests/test_end_of_game_actual_source.py"], "reason": "Recover complete endgame decisions, result dialogs, media, scores and multiplayer teardown through genuine source owners. Preserve original defects and branch ordering, including repeated cleanup in the separate net_control_turn caller. Historical spans/findings remain obligations, not byte or runtime acceptance."}
// TRIED: mechanical transcription of the Ghidra decompilation (which already resolved the three provided jump tables into real switch/case), with every FUN_ callee re-derived from the disassembly's own `; ?mangled@Class` comments rather than guessed - all resolved to functions the scaffold already declares (X_pop, popp, interlude, num_objectives, compute_score, mon_winning_unify, report_score, hall_of_fame, quayle, show_credits, show_replay, has_fac, say_year, amovie_project, parse_num, parse_says) plus three fixed- receiver member calls confirmed from `mov ecx, ADDR; call` (Lock::clear at 0x93e230, AlphaNet::close and NetDaemon::hang_up both at 0x93cd90). Compiles and preserves every branch and both nested nested switches; did not chase the prologue-class mismatch (#0, push-vs-sub) given the size.
int __cdecl end_of_game(int check_victory) {
    char year_text[80];
    int score_items[9];
    int score;
    int selected_faction = -1;

    if ((GameDataStateStorage.fields_.game_state_ & EndgameRecorded) == 0 ||
        (GameDataStateStorage.fields_.game_state_ & EndgamePresented) == 0) {
    // (body left at function indentation; the two guards close before the
    // single `return 0` below)

    if ((GameDataStateStorage.fields_.game_state_ & EndgameRecorded) == 0 &&
        check_victory != 0) {
        if (GameDataStateStorage.fields_.mission_year_current_ >=
            ScenarioSave.ending_mission_year) {
            EndgameExitReason = EndgameReason8;
            GameDataStateStorage.fields_.game_state_ |= EndgameRecorded;
        }

        for (int faction_id = 1; faction_id < 8; ++faction_id) {
            const unsigned int faction_bit = 1u << faction_id;
            if ((GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_ &
                 faction_bit) == 0) {
                continue;
            }

            if (turn_generators(faction_id, 0) >= Rules.subspace_generators_needed) {
                PlayersData[faction_id].flags |= PlayerGeneratorWin;
                if (faction_id == ConsoleGlobal.faction_id_) {
                    EndgameExitReason = EndgameReason16;
                } else if ((PlayersData[faction_id].diplo_treaties[ConsoleGlobal.faction_id_] & 1) != 0 &&
                           (GameDataStateStorage.fields_.game_rules_ & RULES_VICTORY_COOPERATIVE) != 0) {
                    if (EndgameExitReason != EndgameReason16) {
                        EndgameExitReason = EndgameReason17;
                    }
                } else if (EndgameExitReason != EndgameReason16 &&
                           EndgameExitReason != EndgameReason17) {
                    EndgameExitReason = EndgameReason18;
                }
                GameDataStateStorage.fields_.game_state_ |= EndgameRecorded;
                if (selected_faction < 0 || faction_id == ConsoleGlobal.faction_id_) {
                    selected_faction = faction_id;
                }
                GameDataStateStorage.fields_.net_turn_flags_ |= 0x80u;
            }

            if (PlayersData[faction_id].corner_market_turn > 0 &&
                PlayersData[faction_id].corner_market_turn == GameDataStateStorage.fields_.turn_current_num_ &&
                PlayersData[faction_id].corner_market_active > 0) {
                PlayersData[faction_id].flags |= PlayerConquestWin;
                if (faction_id == ConsoleGlobal.faction_id_) {
                    EndgameExitReason = EndgameReason12;
                } else if ((PlayersData[faction_id].diplo_treaties[ConsoleGlobal.faction_id_] & 1) != 0 &&
                           (GameDataStateStorage.fields_.game_rules_ & RULES_VICTORY_COOPERATIVE) != 0) {
                    if (EndgameExitReason != EndgameReason12) {
                        EndgameExitReason = EndgameReason13;
                    }
                } else if (EndgameExitReason != EndgameReason12 &&
                           EndgameExitReason != EndgameReason13) {
                    EndgameExitReason = EndgameReason14;
                }
                GameDataStateStorage.fields_.game_state_ |= EndgameRecorded | STATE_VICTORY_ECONOMIC;
                selected_faction = faction_id;
                if (faction_id == ConsoleGlobal.faction_id_) {
                    break;
                }
            } else if (((faction_bit & GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_) != 0 ||
                        (GameDataStateStorage.fields_.game_rules_ & RULES_SCN_VICT_SOLO_MISSION) == 0) &&
                       static_cast<int>(ScenarioSave.objectives_sudden_death_victory) >= 1 &&
                       num_objectives(faction_id,
                           GameDataStateStorage.fields_.game_rules_ & RULES_VICTORY_COOPERATIVE) >=
                           static_cast<int>(ScenarioSave.objectives_sudden_death_victory)) {
                GameDataStateStorage.fields_.game_state_ |= EndgameRecorded;
                EndgameExitReason = EndgameReason9;
                if (selected_faction < 0 || faction_id == ConsoleGlobal.faction_id_) {
                    selected_faction = faction_id;
                }
            }
        }
    }

    if (check_victory != 0) {
        if ((GameDataStateStorage.fields_.game_state_ & STATE_IS_SCENARIO) == 0 &&
            (GameDataStateStorage.fields_.game_state_ & EndgameRecorded) == 0 &&
            GameDataStateStorage.fields_.mission_year_current_ ==
                ScenarioSave.ending_mission_year - 20) {
            parse_num(0, ScenarioSave.ending_mission_year);
            StringTemp[0] = 0;
            _itoa(ScenarioSave.ending_mission_year, year_text, 10);
            strcat(StringTemp, year_text);
            parse_says(0, StringTemp, -1, -1);
            X_pop("RETIREWARNING", 0);
        } else if ((GameDataStateStorage.fields_.game_state_ & STATE_IS_SCENARIO) != 0 &&
                   (GameDataStateStorage.fields_.game_state_ & EndgameRecorded) == 0 &&
                   (GameDataStateStorage.fields_.mission_year_current_ ==
                        ScenarioSave.ending_mission_year - 20 ||
                    GameDataStateStorage.fields_.mission_year_current_ ==
                        ScenarioSave.ending_mission_year - 10)) {
            parse_num(0, ScenarioSave.ending_mission_year);
            StringTemp[0] = 0;
            _itoa(ScenarioSave.ending_mission_year, year_text, 10);
            strcat(StringTemp, year_text);
            parse_says(0, StringTemp, -1, -1);
            X_pop("SCENARIO", "SCENARIOWARNING", 0);
        }
    }

    if ((GameDataStateStorage.fields_.game_state_ & EndgameRecorded) != 0 &&
        (GameDataStateStorage.fields_.game_state_ & EndgamePresented) == 0) {
    int winner = 0;
    for (int faction_id = 1; faction_id < 8; ++faction_id) {
        const uint32_t flags = PlayersData[faction_id].flags;
        if ((flags & PlayerEndgameFlagA) != 0) {
            winner = faction_id;
        }
        if ((flags & PlayerEndgameFlagB) != 0 &&
            (winner == 0 ||
             (PlayersData[winner].diplo_treaties[ConsoleGlobal.faction_id_] & 1) != 0 ||
             winner == ConsoleGlobal.faction_id_)) {
            winner = faction_id;
        }
    }

    int roll_credits = 0;
    switch (EndgameExitReason) {
    case EndgameReason1:
    case EndgameReason2:
    case EndgameReason3:
        roll_credits = 1;
        break;
    case EndgameReason7:
        if ((GameDataStateStorage.fields_.game_preferences_ & PREF_AV_SECRET_PROJECT_MOVIES) != 0) {
            if ((Players[ConsoleGlobal.faction_id_].rule_flags & RFLAG_ALIEN) != 0) {
                amovie_project("losealien");
            } else {
                amovie_project(const_cast<char *>(
                    Players[ConsoleGlobal.faction_id_].is_leader_female ? "losewoman" : "loseman"));
            }
        }
        break;
    case EndgameReason15:
        if (Language != 0) {
            X_pop("YOULOSE2", 0);
        }
        break;
    case EndgameReason8:
        parse_num(0, GameDataStateStorage.fields_.mission_year_current_);
        StringTemp[0] = 0;
        say_year(StringTemp);
        parse_says(0, StringTemp, -1, -1);
        if ((GameDataStateStorage.fields_.game_state_ & EndgameNetResults) != 0) {
            if (ScenarioSave.objective_req_victory == 0 ||
                (GameDataStateStorage.fields_.game_state_ & EndgameScoreMode) != 0) {
                int best_faction = ConsoleGlobal.faction_id_;
                int best_score = -9999;
                for (int faction_id = 1; faction_id < 8; ++faction_id) {
                    if ((GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_ &
                         (1u << faction_id)) != 0) {
                        if ((GameDataStateStorage.fields_.game_state_ & EndgameScoreMode) != 0) {
                            compute_score(faction_id, &score, score_items, 1);
                        } else {
                            score = num_objectives(faction_id,
                                GameDataStateStorage.fields_.game_rules_ & RULES_VICTORY_COOPERATIVE);
                        }
                        if (score > best_score ||
                            (score == best_score && faction_id == ConsoleGlobal.faction_id_)) {
                            best_faction = faction_id;
                            best_score = score;
                        }
                    }
                }
                parse_player_identity(best_faction);
                if (best_faction == ConsoleGlobal.faction_id_) {
                    X_pop("SCENARIO", "SCENTIMEWIN", 0);
                } else {
                    X_pop("SCENARIO", "SCENTIMELOSS", 0);
                }
            } else {
                X_pop("SCENARIO", "SCENTIMELIMIT", 0);
            }
        } else {
            X_pop("TIMELIMIT", 0);
        }
        break;
    case EndgameReason6:
        popp(ScriptPopupFile, "DIPLOMATICVICTORY", 0, "dipvic_sm.pcx", 0);
        break;
    case EndgameReason10:
        parse_player_identity(winner);
        popp(ScriptPopupFile, "DIPLOMATICCOOP", 0, "dipvic_sm.pcx", 0);
        break;
    case EndgameReason11:
        parse_player_identity(winner);
        popp(ScriptPopupFile,
             (PlayersData[winner].diplo_treaties[ConsoleGlobal.faction_id_] & 0x40020u) != 0
                 ? "DIPLOMATICLOSE2" : "DIPLOMATICLOSE",
             0, "dipvic_sm.pcx", 0);
        break;
    case EndgameReason12:
        popp(ScriptPopupFile, "ECONOMICVICTORY", 0, "econwin_sm.pcx", 0);
        break;
    case EndgameReason13:
        parse_player_identity(winner);
        popp(ScriptPopupFile, "ECONOMICCOOP", 0, "econwin_sm.pcx", 0);
        break;
    case EndgameReason14:
        parse_player_identity(winner);
        popp(ScriptPopupFile,
             (PlayersData[winner].diplo_treaties[ConsoleGlobal.faction_id_] & 0x40020u) != 0
                 ? "ECONOMICLOSE2" : "ECONOMICLOSE",
             0, "econwin_sm.pcx", 0);
        break;
    case EndgameReason9:
        // The sudden-death text names the selected faction, not the winner.
        parse_player_identity(selected_faction);
        if (selected_faction == ConsoleGlobal.faction_id_) {
            X_pop("SCENARIO", "SUDDENDEATH0", 0);
        } else {
            X_pop("SCENARIO", "SUDDENDEATH", 0);
        }
        break;
    case EndgameReason16:
        interlude(0x20, 0, 4, 0);
        parse_player_identity(winner);
        popp(ScriptPopupFile, "ENDBEACON", 0, "beacon_sm.pcx", 0);
        if ((GameDataStateStorage.fields_.game_preferences_ & PREF_AV_SECRET_PROJECT_MOVIES) != 0) {
            StringTemp[0] = 0;
            strcat(StringTemp, Players[ConsoleGlobal.faction_id_].search_key);
            CharUpperA(StringTemp);
            if (strcmp(StringTemp, "CARETAKE") != 0) {
                amovie_project("close_us");
            } else {
                amovie_project("close_ct");
            }
        }
        break;
    case EndgameReason17:
        parse_player_identity(winner);
        popp(ScriptPopupFile, "ENDBEACONCOOP", 0, "beacon_sm.pcx", 0);
        break;
    case EndgameReason18:
        if ((Players[ConsoleGlobal.faction_id_].rule_flags & RFLAG_ALIEN) == 0) {
            GenderDefault = Players[winner].noun_gender;
            PluralityDefault = Players[winner].is_noun_plural;
            interlude(0x1D, Players[winner].noun_faction, 1, 0);
        }
        parse_player_identity(winner);
        popp(ScriptPopupFile, "ENDBEACONLOSE", 0, "beaconlose_sm.pcx", 0);
        break;
    }

    switch (EndgameExitReason) {
    case EndgameReason4:
    case EndgameReason5:
    case EndgameReason6:
    case EndgameReason12:
        mon_winning_unify(ConsoleGlobal.faction_id_, -1);
        break;
    case EndgameReason10:
    case EndgameReason13:
        mon_winning_unify(winner, -1);
        mon_winning_unify(ConsoleGlobal.faction_id_, winner);
        break;
    case EndgameReason11:
    case EndgameReason14:
        mon_winning_unify(winner, -1);
        break;
    }

    if ((GameDataStateStorage.fields_.game_preferences_ & PREF_AV_INTERLUDES_DISABLED) == 0) {
        switch (EndgameExitReason) {
        case EndgameReason4:
        case EndgameReason6:
        case EndgameReason12:
            if ((Players[ConsoleGlobal.faction_id_].rule_flags & RFLAG_ALIEN) != 0) {
                interlude(0x21, 0, 4, 0);
            } else {
                interlude(0x13, 0, 4, 0);
            }
            roll_credits = 1;
            break;
        case EndgameReason5:
        case EndgameReason10:
        case EndgameReason13:
            interlude(0x14, 0, 4, 0);
            roll_credits = 1;
            break;
        }
    }

    // The image materializes the 1..3 test and ORs it with the flag (no
    // short circuit), hence the bitwise `|`.
    if ((EndgameExitReason == EndgameReason1 || EndgameExitReason == EndgameReason2 ||
         EndgameExitReason == EndgameReason3) | roll_credits) {
        show_credits();
    }
    report_score(1);
    if ((GameDataStateStorage.fields_.game_state_ & EndgameNetResults) == 0) {
        quayle(ConsoleGlobal.faction_id_);
        hall_of_fame(1);
    }
    show_replay();
    GameDataStateStorage.fields_.game_state_ |= EndgamePresented;
    if (IsMultiplayerNet != 0 || EndgameExitReason == EndgameReason7 ||
        popp(ScriptPopupFile, "GAMEOVERMAN", 0, "stars_sm.pcx", 0) == 0) {
        GameRestartQueued = (IsMultiplayerNet == 0);
        if (IsMultiplayerNet != 0) {
            NetDaemonNet.hang_up();
            NetDaemonNet.reset();
            IsMultiplayerNet = 0;
        }
        ExitTurnLoop = 1;
        return 1;
    }
    }  // recorded and not yet presented
    }  // not both recorded and presented
    // Every `return 0` in the image jumps to one shared epilogue laid out
    // after the return-1 tail (0x00525816): the guards are nested ifs around
    // the body with a single final return, not early `goto`/`return 0` exits.
    return 0;
}

// stand-in: proto_power with its range() called through a pointer, which
// forces the image's out-of-line range call (the original's inline budget).
typedef int (__cdecl *TurnRangeFnT)(int, int, int);
static inline int turn_proto_power(int veh_id) {
    TurnRangeFnT range_fn = &range;
    if (VehPrototypes[Vehs[veh_id].proto_id].plan == PLAN_ALIEN_ARTIFACT) {
        return 1;
    }
    int reactor = VehPrototypes[Vehs[veh_id].proto_id].reactor_id;
    return range_fn(reactor, 1, 100) * 10;
}

// WORKAROUND(decl): unidentified inline helper taking the tile coordinates
// as parameters. The image copies x and y into fresh registers before the
// base_who/airbase tile test (0x00525941..0x00525984); only an inline call
// with (x, y) parameters reproduces that, the open-coded test computes the
// tile address in place. Original name and home are unknown.
static inline BOOL turn_air_unit_unsheltered(int x, int y) {
    return base_who(x, y) < 0 && !(map_loc(x, y)->bit & BIT_AIRBASE);
}

// ORIGINAL: 0x005258C0 ?turn_upkeep@@YAXXZ 0x005258C0-0x0052602D BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:6e78872480ef8b4a486516e322f23e9f8652665d:src/unrecovered/005258c0.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "recovery/tasks/game-logic-seed-owner-coverage.json; preserve original extents and historical seed lessons. Ordinary owned literals have pinned content provenance, not exact original data-allocation claims."], "reason": "Recover end-turn aircraft attrition, climate, native-life/alien arrival, caretaker diplomacy and ordered alien-pair stores through genuine owners. Random-events remains an explicitly unfinished separate dependency; build continuity is not game behavior acceptance."}
// TRIED: transcribed directly from Ghidra's decompile (already correct, self-consistent short*/uint* pointer arithmetic throughout), fixing only DAT_/FUN_ names against the catalogue (all ~25 callees were already declared with matching signatures) and CharUpperA (undeclared IAT slot at 0x66931C, called through a typed function-pointer global). MapWin::main_caption()'s receiver is `*g_007d3c3c`, confirmed against src/console.cpp's own use of the same slot. Framed build compiles and matches through instr #3 (mov vs push), sim 0.75 - not chased further, very large orchestration function, landed for coverage.
void __cdecl turn_upkeep() {
    for (int vehicle_id = GameDataStateStorage.fields_.vehicle_current_count_ - 1; vehicle_id >= 0; vehicle_id--) {
        // Air units with limited range burn fuel away from airbases.
        if (get_triad(vehicle_id) == TRIAD_AIR
            && Chassis[VehPrototypes[Vehs[vehicle_id].proto_id].chassis_id].range
            && veh_moves(vehicle_id)
            && turn_air_unit_unsheltered(Vehs[vehicle_id].x, Vehs[vehicle_id].y)
            && !stack_check(vehicle_id, 6, 0x80, -1, -1)) {
            if (Vehs[vehicle_id].terraforming_turns
                < Chassis[VehPrototypes[Vehs[vehicle_id].proto_id].chassis_id].range - 1) {
                Vehs[vehicle_id].terraforming_turns++;
                return;
            }
            Vehs[vehicle_id].state |= 4;
            if (Chassis[VehPrototypes[Vehs[vehicle_id].proto_id].chassis_id].range == 1
                && VehPrototypes[Vehs[vehicle_id].proto_id].chassis_id == 6) {
                Vehs[vehicle_id].dmg_incurred += VehPrototypes[Vehs[vehicle_id].proto_id].reactor_id * 3;
            } else {
                Vehs[vehicle_id].dmg_incurred += range(turn_proto_power(vehicle_id) - Vehs[vehicle_id].dmg_incurred, 0, 9999);
            }
            if (range(turn_proto_power(vehicle_id) - Vehs[vehicle_id].dmg_incurred, 0, 9999) <= 0) {
                kill(vehicle_id);
            }
        }
    }

    VehTerraformStingLatch = 0;
    rankings(1);
    ++GameDataStateStorage.fields_.turn_current_num_;
    GameDataStateStorage.fields_.mission_year_current_ = game_year(GameDataStateStorage.fields_.turn_current_num_);
    MapWinTable[0]->main_caption();

    climactic_changes();

    alien_fauna();
    do_fungal_towers();

    int announced_native_faction = 0;
    if ((GameDataStateStorage.fields_.turn_current_num_ == 4 && !(rand() % 4))
        || (GameDataStateStorage.fields_.turn_current_num_ == 5 && !(rand() % 3))
        || (GameDataStateStorage.fields_.turn_current_num_ == 6 && !(rand() % 2))
        || GameDataStateStorage.fields_.turn_current_num_ >= 7) {
        for (int faction = 1; faction < MaxPlayerNum; ++faction) {
            StringTemp[0] = 0;
            strcat(StringTemp, Players[faction].filename);
            CharUpperA(StringTemp);
            if (((Players[faction].rule_flags & RFLAG_ALIEN) || !strcmp(StringTemp, "FUNGBOY"))
                && !PlayersData[faction].current_num_bases
                && (PlayersData[faction].flags | 0x200) == 0xffffffffu
                && PlayersData[faction].diff_level == -1
                && !is_human(faction) && !is_alive(faction)) {
                if (Players[faction].rule_flags & RFLAG_ALIEN) {
                    announced_native_faction = 1;
                }
                GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_ |= (uint8_t)(1 << faction);
                setup_player(faction, -0x11a, 0);
            }
        }
        if (announced_native_faction) {
            popp(ScriptPopupFile, "ALIENSARRIVE", 0, "al_land_sm.pcx", 0);
            interlude(0x15, 0, 1, 0);
        }
    }

    int caretaker = 0;
    for (int faction = 1; faction < MaxPlayerNum; ++faction) {
        StringTemp[0] = 0;
        strcat(StringTemp, Players[faction].filename);
        CharUpperA(StringTemp);
        if (!strcmp(StringTemp, "CARETAKE")) caretaker = faction;
    }
    if (caretaker) {
        for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; ++base_id) {
            // Bases building the Ascent to Transcendence.
            if (Bases[base_id].queue_production_id[0] == -0x66) {
                int owner = Bases[base_id].faction_id_current;
                if (!(PlayersData[caretaker].diplo_treaties[owner] & 0x10)) {
                    if (PlayersData[owner].diplo_treaties[caretaker] & 1) {
                        pact_ends(owner, caretaker);
                    }
                    treaty_on(caretaker, owner, 0x18);
                    set_treaty(caretaker, owner, 0x40, 1);
                    PlayersData[owner].diplo_spoke[caretaker] = GameDataStateStorage.fields_.turn_current_num_;
                    if (owner == ConsoleGlobal.faction_id_) {
                        X_pops("NOTRANSCEND", 0x100000, FactionDiplomacyPortraits[caretaker], pop_wait);
                    }
                }
            }
        }
    }

    for (int first = 1; first < MaxPlayerNum; ++first) {
        for (int second = 1; second < MaxPlayerNum; ++second) {
            if (first != second && (Players[first].rule_flags & RFLAG_ALIEN)
                && (Players[second].rule_flags & RFLAG_ALIEN)) {
                set_treaty(second, first, 0xc80c0c38, 1);
                set_agenda(second, first, 0x2c2d, 1);
                PlayersData[first].diplo_spoke[second] = GameDataStateStorage.fields_.turn_current_num_;
                PlayersData[second].diplo_spoke[first] = GameDataStateStorage.fields_.turn_current_num_;
            }
        }
    }

    random_events(0);
    ascent();
    if ((GameDataStateStorage.fields_.turn_current_num_ & 3) == 1) reset_territory();
    do_all_non_input();
}

// veh_top (0x00579920) as the image expands it here, guard included.
static inline int repair_veh_top(int veh_id) {
    int top_veh_id = veh_id;
    if (veh_id < 0) {
        return -1;
    }
    while (Vehs[top_veh_id].prev_veh_id_stack >= 0) {
        top_veh_id = Vehs[top_veh_id].prev_veh_id_stack;
    }
    return top_veh_id;
}

/*
Purpose: Run the repair phase for the specified faction: reset the per-turn unit state, heal every
         damaged unit by its reactor-weighted repair rate, and redraw the tiles the units stand on.
// ORIGINAL: 0x00526030 ?repair_phase@@YAXH@Z 0x00526030-0x005267A9
// HISTORICAL LEVER (pre-removal; the wrapper-based measurements below no longer
// describe current source): 2/572 and 0.125 similar -> 59/572 and 0.299 similar, and the direct-call
//   multiset went from 11 to the image's exact 8. Four measured changes.
//   (1) THREE helpers this tree folded in that the image CALLS: `breed_mod`
//   (0x004E65C0) and `draw_tile` (0x0046AF40) are both `MEASURED inline`, and
//   VC6 6.0 has no `__declspec(noinline)` - `#pragma auto_inline(off)` around a
//   plain forwarder in THIS file does the job, and the forwarders sit just above
//   this marker. breed_mod alone was worth three of the extra calls, because
//   inlining it brought its four `has_fac_built_call` bitmask calls with it where
//   the image makes exactly one. (2) That one belongs to THIS body: the
//   facility/project test uses `has_fac_built_call`, not `has_fac_built`, so the
//   bitmask is a real `E8` at 0x005263D1. (1)+(2) took 0.125 -> 0.241.
//   (3) `veh_top` HAND-INLINED. The note below already said the image writes the
//   climb out at 0x005264A8-0x005264C6 rather than calling 0x00579920, because
//   veh.cpp is a different translation unit - but this body still CALLED it, so
//   the note described the image and not the code beside it. Transcribing the
//   climb (starting after the dead `veh_id < 0` guard) brought the call count to
//   8 against the image's 8 and 0.241 -> 0.256; veh.cpp already does this at two
//   of its own sites. (4) `int damage_before`, not `uint8_t`: the image
//   zero-extends on the way in (`xor eax, eax / mov al, [esi + 0x952838]`) and
//   spills a DWORD, where the byte local spills a byte and drops the extension -
//   0.256 -> 0.297 on its own. (5) The VFLAG_UNK_2 test is written NEGATED with
//   the arms swapped, because the image's `jne` goes to the `&= ~VFLAG_UNK_2` arm
//   and falls through to `&= ~VFLAG_UNK_1`. 0.297 -> 0.299.
// TRIED: `triad` as `uint8_t` or `int` rather than `uint32_t`. The image does
//   keep the chassis triad in CL (`mov cl, [ecx + 0x94A379] / test cl, cl /
//   cmp cl, 2`) where this tree zero-extends it into a dword, so the width lever
//   looks right - but measured on all three declarations it is 0.300/0.299/0.299
//   against 0.299 committed, and both `uint8_t` variants RAISE the diverging-run
//   count (55 -> 61 and 62). It is not the lever it looks like.
// TRIED: the VFLAG arms are still 16-bit here (`and eax, 0xFFFE`) against the
//   image's 8-bit (`and al, 0xFE`), and writing the read-modify-write straight on
//   `Vehs[veh_id].flags` with no local does not change that - 0.256, identical to
//   the local. The other standing gap is `tile->bit`: the image re-reads it as a
//   memory operand at every test (`test byte ptr [eax + 8], 1`, then
//   `test dword ptr [eax + 8], 0x800`), while VC6 loads the dword once here and
//   tests CL and CH out of it. Both reads are already written out separately in
//   the source, so the CSE is not something the statement level can withdraw.
// size      1913 bytes
// prototype void (__cdecl ?repair_phase@@YAXH@Z)(int factionID)
// callers   1   call targets   8
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0046AF40 0x004E3A50 0x004E3EF0 0x004E65C0 0x0050BA00 0x005BF1F0 0x005C1760 0x005FCBB0
Return Value: n/a
Status: Complete

TWO PASSES OVER THE UNIT TABLE, NOT ONE. The first heals; the second redraws. They cannot be
folded together, because the first pass can retire a Fungal Tower's owner (below) and the second
pass filters on the faction byte the first one may have just rewritten.

WHAT "REPAIR RATE" MEANS. The rate is a count of repair points, and the damage actually removed is
`reactor_id * rate` - a Fission unit (reactor 1) removes one point per rate step, a Singularity
unit (reactor 4) removes four. The rate starts at 1 and is added to:

  +1  standing in your own territory
  +1  a land unit in a bunker, or an air unit on an airbase
  +1  standing in any base that is not rioting
  x2  a land transport carrying a unit with the Repair ability, at sea or under Sentry/Board

and it is replaced outright by the current damage - a full heal - when the base has the matching
repair facility for the unit's triad (Command Center / Naval Yard / Aerospace Complex) or the
faction holds the matching secret project (Command Nexus / Maritime Control Center / Cloudbase
Academy); when a native-life unit sits in a base whose faction has any lifecycle bonus at all
(`breed_mod`); or, anywhere on the map, when the faction holds the Nano Factory.

Native life in fungus is the one rate bonus that is not additive: it sets the rate to 2 rather
than incrementing it, and it is skipped entirely for a Fungal Tower.

MINIMUM DAMAGE. A unit in the field - non-native faction, not in a base, no repair unit in its
stack, no Nano Factory - cannot be healed below `reactor_id * 2`. Native life in fungus, and
anyone in fungus whose faction holds the Xenoempathy Dome, are exempt; the Dome additionally adds
one more reactor's worth of healing.

Verification note: three tests in this body can never take their false branch and are transcribed
rather than deleted. NONE of them is a mutation-sweep survivor, and the distinction is worth
keeping straight: a test that is constant-true still yields a mutant that is not, so "unreachable"
here is a statement about faithfulness to the original, not a coverage gap being excused.

  - `owner >= 0` after `owner < MaxPlayerNum`, in both passes. This is `base_who()` inlined
    (0x00526152 and 0x00526755); the nibble is masked to 0xF before it is asked, so the sign test
    is constant. can_terraform carries the identical dead half for the identical reason. Mutating
    it to `> 0` is NOT constant - a base owned by faction 0 separates them - and the fixtures
    cover that in both passes.
  - `if (min_damage)` at 0x005265AF. min_damage is `reactor_id * 2`, so the guard is false exactly
    when reactor_id is zero - and with reactor_id zero every statement it guards is a no-op
    (`min_damage` is already 0 and the Dome adds `reactor_id`). The branch is in the original;
    removing it changes nothing, and the harness derives no mutant from it because it carries
    neither a literal nor a comparison.
  - `veh_top()` returning negative. The original inlines the stack walk here rather than calling
    0x00579920, because veh.cpp is a different translation unit; its `veh_id < 0` guard
    (0x005264A2) and the `top < 0` test that follows (0x005264CC) are both dead, since the
    argument is this loop's own index. The walk at 0x005264A8-0x005264C6 is
    instruction-for-instruction that function, and it is now transcribed here rather than called -
    calling it was semantically exact and structurally wrong, one `E8` the image does not have.
    The `stack_id >= 0` this leaves behind is still killable, and killed: unit 0 is a real unit.

The two mutants that DO survive are equivalences rather than gaps. Swapping the `facility_id` and
`project_id` declarations reorders two uninitialised locals that are both assigned before either
is read. Turning `repaired >= damage` into `repaired > damage` changes only the case where they
are already equal, where the guarded statement assigns a variable to itself.

Bug note: the Fungal Tower's owner is reset to faction 0 (the native-life AI) on every repair
phase of whichever faction currently owns it, at 0x005261AF, BEFORE the second pass filters on
the faction byte. A captured Fungal Tower therefore reverts to Planet and is then skipped by its
captor's own redraw pass in the same call. Transcribed as written.
*/
void __cdecl repair_phase(int faction_id) {
    PlayersData[faction_id].sat_odp_deployed = 0;
    for (int veh_id = 0; veh_id < GameDataStateStorage.fields_.vehicle_current_count_; veh_id++) {
        if (Vehs[veh_id].faction_id != faction_id) {
            continue;
        }
        int x = Vehs[veh_id].x;
        int y = Vehs[veh_id].y;
        int damage_before = Vehs[veh_id].dmg_incurred;
        Vehs[veh_id].unk_6 = 0;
        Vehs[veh_id].moves_expended = 0;
        Vehs[veh_id].state &= ~(VSTATE_UNK_2 | VSTATE_UNK_2000 | VSTATE_CRAWLING);
        Vehs[veh_id].flags &= ~VFLAG_UNK_1000;
        // One unit in four each turn, cycling by unit id.
        if (!((GameDataStateStorage.fields_.turn_current_num_ + veh_id) & 3)) {
            Vehs[veh_id].state &= ~VSTATE_UNK_800;
            if (!(Vehs[veh_id].flags & VFLAG_UNK_2)) {
                Vehs[veh_id].flags &= ~VFLAG_UNK_1;
            } else {
                Vehs[veh_id].flags &= ~VFLAG_UNK_2;
            }
        }
        // "Hold 10" counts down in the first waypoint's y.
        if (Vehs[veh_id].order == ORDER_SENTRY_BOARD || Vehs[veh_id].order == ORDER_HOLD) {
            if (Vehs[veh_id].waypoint_y[0]) {
                if (!--Vehs[veh_id].waypoint_y[0]) {
                    Vehs[veh_id].order = ORDER_NONE;
                }
            }
        }
        if (Vehs[veh_id].state & VSTATE_UNK_8) {
            if (base_who(x, y) >= 0
                || (get_triad(veh_id) == TRIAD_LAND && (map_loc(x, y)->bit & BIT_BUNKER))
                || (get_triad(veh_id) == TRIAD_AIR && (map_loc(x, y)->bit & BIT_AIRBASE))) {
                Vehs[veh_id].state &= ~VSTATE_UNK_8;
            }
        }
        if (Vehs[veh_id].proto_id == BSC_FUNGAL_TOWER) {
            Vehs[veh_id].faction_id = 0;  // see the bug note above
        }
        if (Vehs[veh_id].proto_id == BSC_BATTLE_OGRE_MK1 || Vehs[veh_id].proto_id == BSC_BATTLE_OGRE_MK2
            || Vehs[veh_id].proto_id == BSC_BATTLE_OGRE_MK3) {
            continue;  // the Unity Ogres never repair
        }
        if (Vehs[veh_id].proto_id != BSC_FUNGAL_TOWER && (Vehs[veh_id].state & VSTATE_UNK_4)) {
            continue;
        }
        if (!Vehs[veh_id].dmg_incurred) {
            continue;
        }
        int repair_rate = 1;
        if (Vehs[veh_id].proto_id != BSC_FUNGAL_TOWER) {
            if (get_proto_offense_rating(Vehs[veh_id].proto_id) < 0
                && Vehs[veh_id].proto_id < MaxVehProtoFactionNum) {
                Map *tile = map_loc(x, y);
                if ((tile->bit & BIT_FUNGUS) && (tile->climate & ~0x1F) >= ALT_BIT_OCEAN_SHELF) {
                    repair_rate = 2;  // native life regenerates in fungus
                }
            }
            if (whose_territory(faction_id, x, y, NULL, false) == faction_id) {
                repair_rate++;
            }
            if (get_triad(veh_id) == TRIAD_AIR && (map_loc(x, y)->bit & BIT_AIRBASE)) {
                repair_rate++;
            }
            if (get_triad(veh_id) == TRIAD_LAND && (map_loc(x, y)->bit & BIT_BUNKER)) {
                repair_rate++;
            }
        }
        int base_id = base_at(x, y);
        if (base_id >= 0 && !(Bases[base_id].state & BSTATE_DRONE_RIOTS_ACTIVE)) {
            repair_rate++;
            if (get_proto_offense_rating(Vehs[veh_id].proto_id) < 0
                && Vehs[veh_id].proto_id < MaxVehProtoFactionNum) {
                // The lifecycle bonus is the base owner's rather than the unit owner's.
                if (breed_mod(base_id, Bases[base_id].faction_id_current)) {
                    repair_rate = Vehs[veh_id].dmg_incurred;
                }
            } else {
                int facility_id;
                int project_id;
                switch (Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad) {
                case TRIAD_SEA:
                    facility_id = FAC_NAVAL_YARD;
                    project_id = SP_MARITIME_CONTROL_CENTER;
                    break;
                case TRIAD_AIR:
                    facility_id = FAC_AEROSPACE_COMPLEX;
                    project_id = SP_CLOUDBASE_ACADEMY;
                    break;
                default:
                    facility_id = FAC_COMMAND_CENTER;
                    project_id = SP_COMMAND_NEXUS;
                    break;
                }
                if (has_fac(facility_id, base_id, 0)
                    || (project_id >= 0 && has_project(project_id, faction_id))) {
                    repair_rate = Vehs[veh_id].dmg_incurred;
                }
            }
        }
        BOOL has_repair_bay = false;
        if (Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad == TRIAD_LAND
            && veh_cargo(veh_id)
            && (Vehs[veh_id].order == ORDER_SENTRY_BOARD
                || is_ocean(x, y))) {
            for (int stack_id = repair_veh_top(veh_id); stack_id >= 0;
                stack_id = Vehs[stack_id].next_veh_id_stack) {
                if (stack_id != veh_id && has_abil(Vehs[stack_id].proto_id, ABL_REPAIR)) {
                    has_repair_bay = true;
                }
            }
            if (has_repair_bay) {
                repair_rate *= 2;
            }
        }
        if (has_project(SP_NANO_FACTORY, faction_id)) {
            repair_rate = Vehs[veh_id].dmg_incurred;
        }
        int reactor_id = VehPrototypes[Vehs[veh_id].proto_id].reactor_id;
        int repair_total = reactor_id * repair_rate;
        int min_damage;
        if (faction_id && !has_repair_bay && base_id < 0
            && !has_project(SP_NANO_FACTORY, faction_id)) {
            min_damage = reactor_id * 2;
        } else {
            min_damage = 0;
        }
        if (min_damage) {
            Map *tile = map_loc(x, y);
            if ((tile->bit & BIT_FUNGUS) && (tile->climate & ~0x1F) >= ALT_BIT_OCEAN_SHELF) {
                if (get_proto_offense_rating(Vehs[veh_id].proto_id) < 0
                    && Vehs[veh_id].proto_id < MaxVehProtoFactionNum) {
                    min_damage = 0;
                }
                if (has_project(SP_XENOEMPATYH_DOME, faction_id)) {
                    min_damage = 0;
                    repair_total += reactor_id;
                }
            }
        }
        int damage = Vehs[veh_id].dmg_incurred;
        int repaired = range(damage - repair_total, min_damage, 999);
        if (repaired >= damage) {
            repaired = damage;  // repairing never adds damage
        }
        Vehs[veh_id].dmg_incurred = (uint8_t)repaired;
        // Wake a human player's sentry once it is as healed as it is going to get, but only in
        // the open: a land unit at sea is aboard a transport and stays boarded.
        if (Vehs[veh_id].dmg_incurred <= min_damage && is_human(faction_id)
            && Vehs[veh_id].order == ORDER_SENTRY_BOARD
            && Vehs[veh_id].dmg_incurred != damage_before) {
            if (Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].triad != TRIAD_LAND
                || (map_loc(x, y)->climate & ~0x1F) >= ALT_BIT_SHORE_LINE) {
                Vehs[veh_id].order = ORDER_NONE;
            }
        }
    }
    for (veh_id = 0; veh_id < GameDataStateStorage.fields_.vehicle_current_count_; veh_id++) {
        if (Vehs[veh_id].faction_id != faction_id) {
            continue;
        }
        if (base_who(Vehs[veh_id].x, Vehs[veh_id].y) >= 0) {
            continue;  // the base's own draw covers it
        }
        if (Vehs[veh_id].faction_id != ConsoleGlobal.faction_id_
            && !(Vehs[veh_id].visibility & (1 << ConsoleGlobal.faction_id_))) {
            continue;
        }
        draw_tile(Vehs[veh_id].x, Vehs[veh_id].y, -1);
    }
    do_all_draws();
}

// A base of the faction still riots after recomputation (inlined).
static inline BOOL allocate_energy_riot_found(int faction_id) {
    for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; base_id++) {
        if (Bases[base_id].faction_id_current == faction_id) {
            set_base(base_id);
            base_compute(1);
            if (BaseCurrent->talent_total < BaseCurrent->drone_total
                && (BaseCurrent->state & BSTATE_DRONE_RIOTS_ACTIVE)) {
                return true;
            }
        }
    }
    return false;
}

// ORIGINAL: 0x005267B0 ?allocate_energy@@YAXH@Z 0x005267B0-0x00526E63 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["original:0x005267B0-0x00526E63", "git:33bae30b:src/unrecovered/005267b0.cpp", "source:src/faction.h", "source:src/base.h", "source:src/veh.h"], "reason": "Recover the complete faction energy allocation operation through real PlayerData, Base, Console and VehPrototype owners. Preserve the original first scan of global BaseCurrent without selecting each base, the later severe-police selection/recompute sequence, signed allocation arithmetic and callback order; historical spelling notes are not fresh measurements."}
// TRIED: direct register-mirroring transcription (eax/ebx/.../var_4/var_c/var_10 locals) compiles and matches control flow/offsets faithfully but the extra scratch locals (vs the original's 6-int 0x18 frame) shift register allocation from instr #4; sim 0.68 framed. Not retried with fewer locals given size (1715 bytes).
void __cdecl allocate_energy(int faction_id) {
    if (is_human(faction_id)) {
        // Human-only infrastructure warnings.
        if (PlayersData[faction_id].diff_level <= 1 || faction_id != ConsoleGlobal.faction_id_
            || PlayersData[faction_id].current_num_bases <= 1
            || PlayersData[faction_id].energy_reserves >= 100)
            return;
        if (PlayersData[faction_id].energy_reserves * 11 - CurrentFactionEnergyReserves * 10 >= 0)
            return;
        if (10 - PlayersData[faction_id].se_alloc_psych - PlayersData[faction_id].se_alloc_labs < 5) {
            if (!popp(ScriptPopupFile, "INFRASTRUCTURE", 0, "genwarning_sm.pcx", 0))
                return;
            social_select(faction_id);
            return;
        }
        if (!has_tech(VehPrototypes[1].preq_tech, faction_id)) return;
        if (GameDataStateStorage.fields_.turn_current_num_ % 5) return;
        popp(ScriptPopupFile, "INFRASTRUCTURE2", 0, "genwarning_sm.pcx", 0);
        return;
    }

    int unassigned = 10 - PlayersData[faction_id].se_alloc_psych - PlayersData[faction_id].se_alloc_labs;
    int police = PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::POLICE];
    int max_psych;
    if (police < -2) {
        max_psych = 4;
    } else if (police == -2) {
        max_psych = 2;
    } else {
        max_psych = police <= 1;
    }
    int idle = 1;
    for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; base_id++) {
        if (Bases[base_id].faction_id_current == faction_id) {
            // The image tests BaseCurrent here, not the scanned base.
            if (BaseCurrent->state & BSTATE_DRONE_RIOTS_ACTIVE) {
                goto increase_psych;
            }
            if (BaseCurrent->specialist_total > BaseCurrent->specialist_unk_1) {
                idle = 0;
            }
        }
    }
    if (idle) {
        if (PlayersData[faction_id].se_alloc_psych > max_psych / 2) {
            PlayersData[faction_id].se_alloc_psych--;
        }
    }
    goto psych_done;

increase_psych:
    if (PlayersData[faction_id].se_alloc_psych < max_psych) {
        PlayersData[faction_id].se_alloc_psych++;
        while (PlayersData[faction_id].se_alloc_psych < max_psych
            && PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::POLICE] < -2
            && allocate_energy_riot_found(faction_id)) {
            PlayersData[faction_id].se_alloc_psych++;
        }
    }

psych_done:
    PlayersData[faction_id].se_alloc_psych = range(PlayersData[faction_id].se_alloc_psych,
        PlayersData[faction_id].soc_effect_pending.values_[SocialEffect::POLICE] < -1 ? 2 : 0, 4);
    if (has_project(SP_TELEPATHIC_MATRIX, faction_id)) {
        PlayersData[faction_id].se_alloc_psych = 0;
    }
    PlayersData[faction_id].se_alloc_labs = (PlayersData[faction_id].ai_tech - PlayersData[faction_id].ai_power
        - PlayersData[faction_id].se_alloc_psych + 11) / 2 - PlayersData[faction_id].ai_wealth;
    if (GameDataStateStorage.fields_.turn_current_num_ < (PlayersData[faction_id].ai_tech
            - PlayersData[faction_id].ai_power - PlayersData[faction_id].ai_wealth + 3) * 25) {
        PlayersData[faction_id].se_alloc_labs++;
    }
    PlayersData[faction_id].se_alloc_labs += PlayersData[faction_id].energy_reserves
        / ((Players[faction_id].rule_techcost * (PlayersData[faction_id].ai_power
            + PlayersData[faction_id].ai_wealth + 1) + GameDataStateStorage.fields_.turn_current_num_) * 2);
    int region_id;
    for (region_id = 1; region_id < 0x80; region_id++) {
        if (PlayersData[faction_id].region_total_bases[region_id]) {
            if (PlayersData[faction_id].region_base_plan[region_id] == 2) {
                PlayersData[faction_id].se_alloc_labs--;
                break;
            }
            if (!PlayersData[faction_id].region_base_plan[region_id]
                && (PlayersData[faction_id].ai_wealth || PlayersData[faction_id].ai_power)) {
                PlayersData[faction_id].se_alloc_labs--;
            }
        }
    }
    if (PlayersData[faction_id].ai_tech && !PlayersData[faction_id].ai_wealth
        && !PlayersData[faction_id].ai_power && region_id >= 0x80) {
        PlayersData[faction_id].se_alloc_labs++;
    }
    if (unassigned) {
        if (10 - PlayersData[faction_id].se_alloc_psych - PlayersData[faction_id].se_alloc_labs >= unassigned) {
            int value = 0;
            for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; base_id++) {
                if (Bases[base_id].faction_id_current == faction_id) {
                    value += Bases[base_id].unk_10;
                    for (int facility_id = 0; facility_id < 0x46; facility_id++) {
                        if (has_fac(facility_id, base_id, 0)) {
                            value -= fac_maint(facility_id, faction_id);
                        }
                    }
                }
            }
            int expected = (PlayersData[faction_id].ai_wealth * 2 + 4 - PlayersData[faction_id].ai_tech)
                * PlayersData[faction_id].current_num_bases / 4;
            if (value > expected) {
                while (10 - PlayersData[faction_id].se_alloc_psych
                    - PlayersData[faction_id].se_alloc_labs >= unassigned) {
                    PlayersData[faction_id].se_alloc_labs++;
                }
            } else if (value < expected / 2) {
                PlayersData[faction_id].se_alloc_labs--;
            }
        }
    }
    if (PlayersData[faction_id].tech_id_researching == 0x58) {
        PlayersData[faction_id].se_alloc_labs--;
    }
    PlayersData[faction_id].se_alloc_labs = range(PlayersData[faction_id].se_alloc_labs, 0,
        min(10 - PlayersData[faction_id].se_alloc_psych, energy_limit(faction_id)));
    while (10 - PlayersData[faction_id].se_alloc_labs - PlayersData[faction_id].se_alloc_psych
           > energy_limit(faction_id)) {
        PlayersData[faction_id].se_alloc_labs++;
    }
    int threshold = range(PlayersData[faction_id].soc_effect_temp.values_[SocialEffect::EFFICIENCY], 0, 99)
        + range(3 - PlayersData[faction_id].current_num_bases, 0, 3) + 1;
    PlayersData[faction_id].se_alloc_labs = range(PlayersData[faction_id].se_alloc_labs, 0, 10);
    while (PlayersData[faction_id].se_alloc_labs > threshold - PlayersData[faction_id].se_alloc_psych
        - PlayersData[faction_id].se_alloc_labs + 10) {
        PlayersData[faction_id].se_alloc_labs--;
    }
    while (10 - PlayersData[faction_id].se_alloc_psych - PlayersData[faction_id].se_alloc_labs
        > PlayersData[faction_id].se_alloc_labs + threshold) {
        PlayersData[faction_id].se_alloc_labs++;
    }
}

// ORIGINAL: 0x00526E70 ?production_phase@@YAXH@Z 0x00526E70-0x00527285 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence":["original:0x00526E70-0x00527285:the full pinned function span is the production-phase body","original:0x00526F0E:the first indexed rule-flags access uses the actor faction id","original:0x00526F43:loan credit is written to the partner PlayerData energy_reserves field","source:src/faction.h:Players, PlayersData, Player.rule_flags, PlayerData.loan_balance, loan_payment, sanction_turns, energy_reserves","source:src/base.h:Base ownership and the declared base-upkeep interface","source:src/technology.h:technology effects and technology progression owners"],"reason":"Recover the production-phase identity in faction.cpp using the existing Player/PlayerData, Base and technology owners while retaining the pinned actor/partner commerce roles and native callback order."}
void __cdecl production_phase(int faction_id) {
    PlayersData[faction_id].unk_28 = 0;
    PlayersData[faction_id].unk_13 = 0;
    PlayersData[faction_id].maint_cost_total = 0;
    PlayersData[faction_id].turn_commerce_income = 0;
    tech_effects(faction_id);
    for (int i = 1; i < MaxPlayerNum; i++) {
        if (i != faction_id && is_alive(i) && is_alive(faction_id)
            && PlayersData[faction_id].loan_balance[i] && !PlayersData[i].sanction_turns) {
            if (PlayersData[faction_id].diplo_treaties[i] & DTREATY_VENDETTA) {
                PlayersData[faction_id].loan_balance[i] += PlayersData[faction_id].loan_payment[i];
            } else {
                int payment = PlayersData[faction_id].loan_payment[i];
                if (payment >= PlayersData[faction_id].energy_reserves) {
                    payment = PlayersData[faction_id].energy_reserves;
                }
                PlayersData[i].energy_reserves += payment;
                PlayersData[faction_id].loan_balance[i] -= payment;
                PlayersData[faction_id].energy_reserves -= payment;
                if (payment < PlayersData[faction_id].loan_payment[i]) {
                    PlayersData[faction_id].loan_balance[i] += PlayersData[faction_id].loan_payment[i] - payment;
                }
            }
        }
    }
    PlayersData[faction_id].flags &= ~PFLAG_SELF_AWARE_COLONY_LOST_MAINT;
    CurrentFactionEnergyReserves = PlayersData[faction_id].energy_reserves;
    for (int slot = 0; slot < 9; slot++) {
        PlayersData[faction_id].unk_46[slot] = 0;
    }
    for (int row = 0; row < 8; row++) {
        PlayersData[faction_id].unk_38[row] = 0;
        for (int column = 0; column < 9; column++) {
            PlayersData[faction_id].unk_39[row][column] = 0;
        }
    }
    PlayersData[faction_id].unk_47 = 0;
    PlayersData[faction_id].unk_48 = 0;
    PlayersData[faction_id].unk_49 = 0;
    PlayersData[faction_id].nutrient_surplus_total = 0;
    PlayersData[faction_id].labs_total = 0;
    for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; base_id++) {
        if (Bases[base_id].faction_id_current == faction_id) {
            if (base_upkeep(base_id)) {
                base_id--;
            }
            do_all_non_input();
        }
    }
    if (!is_human(faction_id) && GameDataStateStorage.fields_.diff_level_current_ > 3
        && PlayersData[faction_id].maint_cost_total) {
        if (GameDataStateStorage.fields_.diff_level_current_ == 4) {
            PlayersData[faction_id].energy_reserves += (int)PlayersData[faction_id].maint_cost_total / 3;
            PlayersData[faction_id].maint_cost_total -= (int)PlayersData[faction_id].maint_cost_total / 3;
        } else {
            PlayersData[faction_id].energy_reserves += (int)PlayersData[faction_id].maint_cost_total * 2 / 3;
            PlayersData[faction_id].maint_cost_total -= (int)PlayersData[faction_id].maint_cost_total * 2 / 3;
        }
    }
    if (Players[faction_id].rule_flags & RFLAG_INTEREST) {
        if (Players[faction_id].rule_interest) {
            PlayersData[faction_id].energy_reserves = Players[faction_id].rule_interest
                * PlayersData[faction_id].energy_reserves / 100 + PlayersData[faction_id].energy_reserves;
        } else {
            for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; base_id++) {
                if (Bases[base_id].faction_id_current == faction_id) {
                    PlayersData[faction_id].energy_reserves++;
                    PlayersData[faction_id].unk_13++;
                }
            }
        }
    }
    PlayersData[faction_id].unk_18 = PlayersData[faction_id].unk_13;
    PlayersData[faction_id].unk_17 = PlayersData[faction_id].turn_commerce_income
        - PlayersData[faction_id].maint_cost_total + PlayersData[faction_id].unk_13;
    if (GameDataStateStorage.fields_.turn_current_num_ == 1) {
        TechAchievedGate = 1;
        for (int i = 0; i < Players[faction_id].rule_tech_selected; i++) {
            log_say("Freebie Advance", Players[faction_id].name_leader, faction_id, 0, 0);
            tech_advance(faction_id);
        }
        TechAchievedGate = 0;
    }
    if ((!(GameDataStateStorage.fields_.game_state_ & 1) || GameDataStateStorage.fields_.game_state_ & 8)
        && PlayersData[faction_id].sanction_turns) {
        if (!--PlayersData[faction_id].sanction_turns && faction_id == ConsoleGlobal.faction_id_) {
            if (Players[faction_id].rule_flags & RFLAG_ALIEN) {
                popp(ScriptPopupFile, "SANCTIONSENDALIEN", 0, "Alopdir.pcx", 0);
                return;
            }
            popp(ScriptPopupFile, "SANCTIONSEND", 0, "council_sm.pcx", 0);
        }
    }
}

// Original: 0x00527290..0x005275AC, ?faction_upkeep@@YAXH@Z.

// ORIGINAL: 0x00527290 ?faction_upkeep@@YAXH@Z 0x00527290-0x005275AC BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence":["original:0x00527290-0x005275AC:the full pinned function span is faction upkeep","original:0x0052756D-0x00527581:the final call_council gate tests the HUMAN mask and calls for nonhuman factions","original:0x00527506-0x0052750B:the two Path destination words are reset independently at function exit","source:src/faction.h:Players and PlayersData own faction state and turn bookkeeping","source:src/path.h:Path owns x_dst_ and y_dst_ and the candidate uses its reset operation","source:src/council.h:can_call_council and call_council are the canonical council interfaces"],"reason":"Recover faction upkeep in its faction owner with the native nonhuman council gate, fresh callback state, and Path-owned destination reset."}
// TRIED: literal asm-mirrored translation compiles and matches through instr #41 (of ~230); diverges at #42 (shl vs and, likely the corner_market/has_tech gated block's field-offset codegen) - not chased further, landed at MISMATCH.
void __cdecl faction_upkeep(int faction_id) {
    UnkGlobal0093A934 = 1;
    social_upkeep(faction_id);
    do_all_non_input();
    repair_phase(faction_id);
    do_all_non_input();
    production_phase(faction_id);
    do_all_non_input();

    if (!(GameDataStateStorage.fields_.game_state_ & STATE_UNK_1) || (GameDataStateStorage.fields_.game_state_ & STATE_UNK_8)) {
        allocate_energy(faction_id);
        do_all_non_input();
        enemy_diplomacy(faction_id);
        do_all_non_input();
        enemy_strategy(faction_id);
        do_all_non_input();
        social_ai(faction_id, -1, -1, -1, -1, 0);
        do_all_non_input();

        if (!is_human(faction_id)
            && PlayersData[faction_id].energy_reserves > corner_market(faction_id)
            && has_tech(Rules.tech_economic_victory, faction_id)
            && (GameDataStateStorage.fields_.game_rules_ & RULES_VICTORY_ECONOMIC)) {
            PlayersData[faction_id].corner_market_turn =
                GameDataStateStorage.fields_.turn_current_num_ + Rules.turns_corner_gbl_energy_mrkt;
            PlayersData[faction_id].corner_market_active = corner_market(faction_id);
            const int cost = corner_market(faction_id);
            PlayersData[faction_id].energy_reserves -= cost;
            parse_set(Players[faction_id].is_leader_female, 0);
            parse_says(0, Players[faction_id].title_leader, -1, -1);
            parse_set(Players[faction_id].is_leader_female, 0);
            parse_says(1, Players[faction_id].name_leader, -1, -1);
            parse_set(Players[faction_id].noun_gender, Players[faction_id].is_noun_plural);
            parse_says(2, Players[faction_id].noun_faction, -1, -1);
            parse_says(3, Players[faction_id].adj_name_faction, -1, -1);
            parse_num(0, game_year(PlayersData[faction_id].corner_market_turn));
            popp(ScriptPopupFile, "CORNERWARNING", 0, "econwin_sm.pcx", 0);
        }
    }

    for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; base_id++) {
        if (Bases[base_id].faction_id_current == faction_id) {
            // Native 0x52748E clears bits 0 and 30; bit 0's role is unknown.
            Bases[base_id].state &= ~(BSTATE_PRODUCTION_HURRIED | BSTATE_UNK_1);
        }
    }

    PlayersData[faction_id].energy_reserves -= PlayersData[faction_id].hurry_cost_total;
    PlayersData[faction_id].hurry_cost_total = 0;
    if (PlayersData[faction_id].energy_reserves < 0) {
        PlayersData[faction_id].energy_reserves = 0;
    }
    if (!PlayersData[faction_id].current_num_bases
        && !PlayersData[faction_id].proto_id_active[0]
        && !PlayersData[faction_id].proto_id_active[21]) {
        eliminate_player(faction_id, 0);
    }

    UnkGlobal0093A934 = 0;
    Paths.clear_destination();

    if (!(GameDataStateStorage.fields_.game_state_ & STATE_UNK_1) || (GameDataStateStorage.fields_.game_state_ & STATE_UNK_8)) {
        if (faction_id == ConsoleGlobal.faction_id_
            && !(GameDataStateStorage.fields_.game_state_ & (STATE_DISPLAYED_COUNCIL_AVAIL_MSG | STATE_COUNCIL_HAS_CONVENED))
            && can_call_council(faction_id, 0)
            && !(GameDataStateStorage.fields_.game_state_ & STATE_UNK_1)) {
            GameDataStateStorage.fields_.game_state_ |= STATE_DISPLAYED_COUNCIL_AVAIL_MSG;
            popp(ScriptPopupFile, "COUNCILOPEN", 0, "council_sm.pcx", 0);
        }
        // The native test is the HUMAN bit, not is_alive().
        if (!is_human(faction_id)) {
            call_council(faction_id);
        }
    }

    if (!IsMultiplayerNet && (GameDataStateStorage.fields_.game_preferences_ & PREF_BSC_AUTOSAVE_EACH_TURN)
        && faction_id == ConsoleGlobal.faction_id_) {
        auto_save();
    }
}

// Source-owned storage observed by the retained control_turn body. These
// initial-text objects own only their pinned NUL byte; X_pop_ask's 0x50 is a
// dialog limit, not a writable span for this input pointer.
char ControlTurnMailTemplate[1];             // 0x0093AA04 lower bound
char ControlTurnPasswordInitialText[1];      // 0x0093AA08 lower bound

extern uint32_t FactionDiplomacyFlags[MaxPlayerNum];


// save_game is declared by the real game owner header.
void __cdecl crash_landing(int faction_id);

// ORIGINAL: 0x005275B0 ?control_turn@@YAXXZ 0x005275B0-0x00528E94;0x0065D6DD-0x0065DB67 FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:6167e270cb6ddcd091b2294560a31b1ec1372cef:src/unrecovered/005275b0.cpp: full original identity/spans and useful historical findings", "ConsoleInfoWin original object7AD2A0 owns suppression memberAC at7AD34C; native527A70/527C9E set1/0; init4592A0 and timer_proc459150 confirm ownership.", "OfferResetCells, PendingDiplomacyFlags and FactionDiplomacyFlags retain original flattened indexing through their actual typed owners.", "Initial text at93AA04/08/0C is proven loader-zero and only read by observed consumers; one-byte source bounds are lower-bound interpretations, not claims of complete original capacity or absence of indirect writers.", "Complete native cleanup-route review supports one real Popup owner; matching and exceptional cleanup restoration remain unfinished.", "recovery/tasks/control-turn-owner-oct5.json", "recovery/tests/test_control_turn_actual_source.py"], "reason": "Recover complete turn orchestration through existing genuine application owners and automatic Popup lifetime. Preserve original branch ordering and error behavior; all primary and exceptional spans remain restoration obligations, not a claim of current native lifetime/byte/runtime equivalence."}
// TRIED: mismatch #11 near prologue (xor vs push) - popup/end_game_pop declared as real C++ locals rather than the original's staggered same-slot vtable-poke construction; the "big dialog" tail (Dialogs/Spot/ListBox/CheckBox/EditGroup/ SpriteBox/StringStruct x2/Sprite) is reached by raw offset casts into BasePop's own dialogs_/spot_/sprite_ members, not by a modelled derived class - faithful call order/args, not byte fidelity. Main loop, faction dispatch, diplomacy/ tech/council logic transcribed directly from the disasm.
// WORKAROUND(inline): dead /Ob2 spend. The image tears its first return site
// (end_of_game(1)) and the shared ExitTurnLoop/end_of_game(0) site down
// shallower than this body otherwise does (calls ~FlatButton 0x00406880 and
// keeps ~Dialogs 0x00406910 out of line there; per-site depths 5,5,6,6,7+13,
// 7+13,7+18). The five expand_rect insets before the password loop supply
// the spend two probes there used to; two expansions after the faction loop
// keep 7+13 at the sixth site (none: 7+18). The second site (save failed,
// ExitTurnLoop set) needs seven one-if expansions in the gap between it and
// the next return (any position in that gap; six or fewer, or fewer branchy
// probes, change nothing): teardown decisions start at the first return after
// the last spend region, so spend there makes the earlier site shallower.
// Eight in that gap and one after the faction loop (instead of seven and two)
// give the image's StringStruct depth at the 0x005289A0..0x00528A26 site
// (remove_all called directly, no ~StringStruct call); the one call that
// still differs is the last site's ~RadioButton (0x00528CB9, out of line in
// the image, expanded to close() here). Any expansion counts as one step,
// whatever its body, so no count lands between these two.
// Their original spending call sites are unknown.
static inline int control_turn_spend(int k) { if (k == 1) k = 2; return k; }
void __cdecl control_turn() {
    int retry_count = 0;
    Popup popup;
    uint32_t password_hash;

    if (GameDataStateStorage.fields_.turn_current_num_ == 0 && ControlTurnPhase == 0
        && is_human(ConsoleGlobal.faction_id_) && IsMultiplayerPBEM == 0) {
        crash_landing(ConsoleGlobal.faction_id_);
    }
    if (IsMultiplayerPBEM == 0) {
        draw_map(1);
    }
    MapBaseDisplayOverride = 1;
    if (ControlTurnPhase != 0 && !is_human(ConsoleGlobal.faction_id_)) {
        ConsoleGlobal.human_turn();
    }

turn_loop:
    if (IsMultiplayerPBEM == 0) {
        InterfaceMessageWin.clear();
    }
    if (ControlTurnPhase == 0) {
        ConsoleControlTurnActive = 1;
        turn_upkeep();
        if (end_of_game(1) != 0) {
            return;
        }
    } else {
        rankings(1);
    }

    {
        int human_turn_shown = 0;
        for (int faction = 0; faction < MaxPlayerNum; ++faction) {
            if (!is_alive(faction) || faction < ControlTurnPhase) {
                continue;
            }
            GameDataStateStorage.fields_.net_turn_faction_ = faction;
            if (is_human(faction)
                && (faction != ConsoleGlobal.faction_id_
                    || (GameDataStateStorage.fields_.net_turn_flags_ & 0x20) != 0
                    || IsMultiplayerPBEM != 0)) {
                parse_says(0, ControlTurnMailTemplate, -1, -1);
                parse_set(Players[faction].is_leader_female, 0);
                parse_says(1, Players[faction].title_leader, -1, -1);
                parse_set(Players[faction].is_leader_female, 0);
                parse_says(2, Players[faction].name_leader, -1, -1);
                parse_set(Players[faction].noun_gender, Players[faction].is_noun_plural);
                parse_says(3, Players[faction].noun_faction, -1, -1);
                parse_num(0, GameDataStateStorage.fields_.mission_year_current_);
                ConsoleGlobal.faction_id_ = faction;

                if (IfaceIdToSlotStorage[ActiveIfaceIdStorage] == 10) {
                    g_BattleWin.stop_timer();
                    static_cast<SubInterface &>(g_BattleWin).release_iface_mode();
                }
                stop_timers();
                TutWinBaseWindow.exit();
                ConsoleGlobal.alpha_menu_.hide();
                MultiWindow.hide();
                InterfaceMessageWin.listBox_.Win::hide();
                MainInterfaceGlobal.string_box_.Win::hide();
                WorldClimateWorldWin.hide_all();

                RECT frame;
                frame = MainInterfaceGlobal.status_left_rect_;
                MainInterfaceGlobal.Buffer::box_sprite(&frame, &InterfaceBoxStyles[21]);
                expand_rect(&frame, -3, -3);
                MainInterfaceGlobal.Buffer::box_sprite(&frame, &InterfaceBoxStyles[18]);
                frame = MainInterfaceGlobal.status_right_rect_;
                MainInterfaceGlobal.Buffer::box_sprite(&frame, &InterfaceBoxStyles[21]);
                expand_rect(&frame, -3, -3);
                MainInterfaceGlobal.Buffer::box_sprite(&frame, &InterfaceBoxStyles[18]);
                frame = MainInterfaceGlobal.status_hit_rect_;
                MainInterfaceGlobal.Buffer::box_sprite(&frame, &InterfaceBoxStyles[21]);
                expand_rect(&frame, -3, -3);
                MainInterfaceGlobal.Buffer::box_sprite(&frame, &InterfaceBoxStyles[18]);
                frame = MainInterfaceGlobal.tutorial_bounds_;
                MainInterfaceGlobal.Buffer::box_sprite(&frame, &InterfaceBoxStyles[21]);
                expand_rect(&frame, -3, -3);
                MainInterfaceGlobal.Buffer::box_sprite(&frame, &InterfaceBoxStyles[18]);
                frame = MainInterfaceGlobal.message_bounds_;
                MainInterfaceGlobal.Buffer::box_sprite(&frame, &InterfaceBoxStyles[21]);
                expand_rect(&frame, -3, -3);
                MainInterfaceGlobal.Buffer::box_sprite(&frame, &InterfaceBoxStyles[18]);
                frame = MainInterfaceGlobal.date_rect_;
                MainInterfaceGlobal.Buffer::box_sprite(&frame, &InterfaceBoxStyles[18]);

                MainInterfaceGlobal.update(static_cast<GraphicWin *>(0));
                ConsoleInfoWin.set_timer_suppression(1);
                ConsoleGlobal.cover_window_.fill(0);
                ConsoleGlobal.cover_window_.Win::show(0);
                do_all_draws();
                MainInterfaceGlobal.string_box_.clear();

                for (;;) {
                    popup.start(PopupStartCaption,
                                PlayersData[faction].unk_102 != 0 ? "SWITCHING2" : "SWITCHING",
                                -1, 0, 0x800004, 0);
                    if (popup.exec(0, 0) >= 0) {
                        if (popup.input_poll_state() != 0) {
                            GameDataStateStorage.fields_.net_turn_flags_ |= 0x40;
                            if (save_game(0) != 0) {
                                GameDataStateStorage.fields_.net_turn_flags_ &= ~0x40;
                                if (ExitTurnLoop != 0) {
                                    return;
                                }
                                // WORKAROUND(inline): see control_turn_spend.
                                control_turn_spend(30); control_turn_spend(31);
                                control_turn_spend(32); control_turn_spend(33);
                                control_turn_spend(34); control_turn_spend(35);
                                control_turn_spend(36); control_turn_spend(37);
                                continue;
                            }
                            ExitTurnLoop = 1;
                            GameRestartQueued = 0;
                            ConsoleGlobal.Win::hide();
                            return;
                        }
                        password_hash = checksum_password(ParseStrBuffer[0].str);
                        if (password_hash == static_cast<uint32_t>(PlayersData[faction].unk_102)) {
                            break;
                        }
                        if (PlayersData[faction].unk_102 == 0) {
                            if (X_pop_ask("VERIFYPASSWORD", 0x50, ControlTurnPasswordInitialText,
                                          static_cast<int (__cdecl *)()>(0), 0x800000) == 0
                                && checksum_password(ParseStrBuffer[0].str) == password_hash) {
                                PlayersData[faction].unk_102 = static_cast<int>(password_hash);
                                break;
                            }
                        } else if (++retry_count >= 3) {
                            GameDataStateStorage.fields_.net_turn_flags_ |= 0x40;
                            save_game(0);
                            ConsoleGlobal.Win::hide();
                            ExitTurnLoop = 1;
                            GameRestartQueued = 0;
                            return;
                        }
                    }
                    if (ExitTurnLoop != 0) {
                        goto control_exit;
                    }
                }
                if (ExitTurnLoop != 0) {
                    goto control_exit;
                }

                int i;
                for (i = 0; i < GameDataStateStorage.fields_.base_current_count_; ++i) {
                    if (Bases[i].faction_id_current == ConsoleGlobal.faction_id_) {
                        ConsoleGlobal.set_center(Bases[i].x, Bases[i].y, 1);
                        ConsoleGlobal.set_cursor(Bases[i].x, Bases[i].y);
                        break;
                    }
                }
                if (i >= GameDataStateStorage.fields_.base_current_count_) {
                    for (i = 0; i < GameDataStateStorage.fields_.vehicle_current_count_; ++i) {
                        if (Vehs[i].faction_id == ConsoleGlobal.faction_id_) {
                            ConsoleGlobal.set_center(Vehs[i].x, Vehs[i].y, 1);
                            ConsoleGlobal.set_cursor(Vehs[i].x, Vehs[i].y);
                            break;
                        }
                    }
                }

                ConsoleInfoWin.set_timer_suppression(0);
                ConsoleGlobal.cover_window_.Win::hide();
                ConsoleStatusWin.on_redraw();
                WorldClimateWorldWin.field_239BC_ |= 4;
                WorldClimateWorldWin.show_all();
                WorldClimateWorldWin.redraw();
                MainInterfaceGlobal.string_box_.Win::show(0);
                InterfaceMessageWin.clear();

                if (FactionTurnCombatWins[ConsoleGlobal.faction_id_] != 0
                    || FactionTurnCombatLosses[ConsoleGlobal.faction_id_] != 0) {
                    parse_num(0, FactionTurnCombatWins[ConsoleGlobal.faction_id_]);
                    parse_num(1, FactionTurnCombatLosses[ConsoleGlobal.faction_id_]);
                    popp(ScriptPopupFile, "WONLOST", 0, "bad_sm.pcx", 0);
                }
                if (display_player_messages(ConsoleGlobal.faction_id_) != 0) {
                    MainInterfaceGlobal.button_group(0).set(0x3E8, 1);
                }
                if (MainInterfaceGlobal.button_group(0).field_84_ == 0x3E8) {
                    InterfaceMessageWin.listBox_.Win::show(0);
                }
                ConsoleGlobal.update_data(0);
                start_timers();
                do_all_draws();

                if (TamperDialogFaction[ConsoleGlobal.faction_id_] != 0) {
                    parse_set(Players[TamperDialogFaction[ConsoleGlobal.faction_id_]].is_leader_female, 0);
                    parse_says(0, Players[TamperDialogFaction[ConsoleGlobal.faction_id_]].title_leader, -1, -1);
                    parse_set(Players[TamperDialogFaction[ConsoleGlobal.faction_id_]].is_leader_female, 0);
                    parse_says(1, Players[TamperDialogFaction[ConsoleGlobal.faction_id_]].name_leader, -1, -1);
                    parse_set(Players[TamperDialogFaction[ConsoleGlobal.faction_id_]].noun_gender,
                              Players[TamperDialogFaction[ConsoleGlobal.faction_id_]].is_noun_plural);
                    parse_says(2, Players[TamperDialogFaction[ConsoleGlobal.faction_id_]].noun_faction, -1, -1);
                    parse_num(0, TamperDialogValue[ConsoleGlobal.faction_id_]);
                    X_pop("HOTTAMPER2", 0);
                    TamperDialogFaction[ConsoleGlobal.faction_id_] = 0;
                    TamperDialogValue[ConsoleGlobal.faction_id_] = 0;
                }

                while (PlayersData[ConsoleGlobal.faction_id_].earned_techs_saved > 0) {
                    PlayersData[ConsoleGlobal.faction_id_].earned_techs_saved--;
                    tech_advance(ConsoleGlobal.faction_id_);
                }

                for (int other = 1; other < MaxPlayerNum; ++other) {
                    FactionDiplomacyFlags[other] =
                        PendingDiplomacyFlags[ConsoleGlobal.faction_id_][other];
                    PendingDiplomacyFlags[ConsoleGlobal.faction_id_][other] = 0;
                    if (other != ConsoleGlobal.faction_id_) {
                        const uint32_t flags = FactionDiplomacyFlags[other];
                        if (is_human(other)) {
                            if (flags != 0) {
                                if (OfferResetCells[ConsoleGlobal.faction_id_][other].state_[0] == 3) {
                                    OfferResetCells[ConsoleGlobal.faction_id_][other].state_[0] = 0;
                                    parse_set(Players[other].is_leader_female, 0);
                                    parse_says(0, Players[other].title_leader, -1, -1);
                                    parse_set(Players[other].is_leader_female, 0);
                                    parse_says(1, Players[other].name_leader, -1, -1);
                                    parse_set(Players[other].noun_gender, Players[other].is_noun_plural);
                                    parse_says(2, Players[other].noun_faction, -1, -1);
                                    X_pop("COMMCOMPLETE", 0);
                                } else {
                                    diplo(ConsoleGlobal.faction_id_, other);
                                }
                            }
                        } else if (flags != 0) {
                            communicate(ConsoleGlobal.faction_id_, other, 0);
                        }
                        FactionDiplomacyFlags[other] = 0;
                    }
                }

                if (CouncilPbemGate != 0) {
                    council(ConsoleGlobal.faction_id_, 0, 0);
                } else if (CouncilNotifyPluralSlots[ConsoleGlobal.faction_id_] != 0) {
                    parse_say(0, Proposal[CouncilNotifyFormSlots[ConsoleGlobal.faction_id_]].name, -1, -1);
                    if (CouncilNotifyFormSlots[ConsoleGlobal.faction_id_] != 0) {
                        if (CouncilNotifyPluralSlots[ConsoleGlobal.faction_id_] == 1) {
                            popp(ScriptPopupFile, "COUNCILHOTPASS", 0, "council_sm.pcx", 0);
                        } else if (CouncilNotifyPluralSlots[ConsoleGlobal.faction_id_] == -2) {
                            popp(ScriptPopupFile, "COUNCILHOTVETO", 0, "council_sm.pcx", 0);
                        } else {
                            popp(ScriptPopupFile, "COUNCILHOTFAIL", 0, "council_sm.pcx", 0);
                        }
                    } else {
                        if (GameDataStateStorage.fields_.governor_faction_ > 0) {
                            parse_set(Players[GameDataStateStorage.fields_.governor_faction_].is_leader_female, 0);
                            parse_says(1, Players[GameDataStateStorage.fields_.governor_faction_].title_leader, -1, -1);
                            parse_set(Players[GameDataStateStorage.fields_.governor_faction_].is_leader_female, 0);
                            parse_says(2, Players[GameDataStateStorage.fields_.governor_faction_].name_leader, -1, -1);
                            parse_set(Players[GameDataStateStorage.fields_.governor_faction_].noun_gender,
                                      Players[GameDataStateStorage.fields_.governor_faction_].is_noun_plural);
                            parse_says(3, Players[GameDataStateStorage.fields_.governor_faction_].noun_faction, -1, -1);
                        }
                        if (CouncilNotifyPluralSlots[ConsoleGlobal.faction_id_] == 1) {
                            popp(ScriptPopupFile, "COUNCILHOTGOVWIN", 0, "council_sm.pcx", 0);
                        } else if (GameDataStateStorage.fields_.governor_faction_ > 0) {
                            popp(ScriptPopupFile, "COUNCILHOTGOVLOSE", 0, "council_sm.pcx", 0);
                        } else {
                            popp(ScriptPopupFile, "COUNCILHOTGOVNONE", 0, "council_sm.pcx", 0);
                        }
                    }
                    clear_council_notify(ConsoleGlobal.faction_id_);
                }
            }

            if (ExitTurnLoop != 0) {
                goto control_exit;
            }
            if (ControlTurnPhase == 0
                || (IsMultiplayerPBEM != 0
                    && (GameDataStateStorage.fields_.net_turn_flags_ & 0x40) != 0)) {
                ConsoleControlTurnActive = 1;
                faction_upkeep(faction);
                if (end_of_game(0) != 0) {
                    return;
                }
            }
            GameDataStateStorage.fields_.net_turn_flags_ &= ~0x40;
            ConsoleControlTurnActive = 0;
            unsigned int human_mask;
            human_mask = static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_);
            if (human_mask & (1u << faction)) {
                if (faction == ConsoleGlobal.faction_id_ || bit_count(human_mask) > 1) {
                    ConsoleGlobal.human_turn();
                    if (ControlTurnPhase != 0) {
                        faction = 0;
                        continue;
                    }
                    human_turn_shown = 1;
                }
            } else {
                enemy_turn(faction);
            }
            ControlTurnPhase = 0;
            if (ExitTurnLoop != 0 || end_of_game(0) != 0) {
            control_exit:
                return;
            }
        }
        control_turn_spend(20); // WORKAROUND(inline): see control_turn_spend
        if (human_turn_shown == 0) {
            ConsoleGlobal.human_turn();
        }
    }
    if (ExitTurnLoop != 0) {
        return;
    }
    goto turn_loop;
}

// Original four-byte accesses529Cxx/52A3xx/52A7xx identify this separate
// turn-control latch. Its complete surrounding allocation/meaning is unknown.
// Initial PE virtual-tail storage is zero, not a runtime-state assertion.
// RECOVERY_OBJECT: v2 0x0093A954 0x0093A958 ?TurnControlWord93A954@@3HA 0x0052A2E0
int TurnControlWord93A954 = 0;
extern uint32_t UpgradePromptAnswered; // base.cpp, original90EA3C
extern int BaseAtDebugPopupFlag; // base.cpp, original93A94C
extern int DiploDefaultFlag93A968; // diplowin.cpp
extern int SpockHelpActive; // netdaemon.cpp, original93F674
extern int SpockNotifyPending; // netdaemon.cpp, original93F670
extern int NetDaemonWaitTicks; // netdaemon.cpp, original93A960




















// ORIGINAL: 0x00528EA0 ?net_upkeep_phase@@YAHH@Z 0x00528EA0-0x00529097 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf; original:0x00528EA0-0x00529097; saved pinned full-span sha256:6d3147e1a8d83201f57389f6c8ed65a0938667f845bfcec4215f941828ab75ee", "source:src/game.cpp:net_upkeep at 0x005290A0 is the existing caller/turn owner; game.cpp already includes the NetDaemon, Console, lock-mask, MultiWindow, parser/message, log, temp and WinMM interfaces", "source:src/netdaemon.h:the sent mask is the flat member field_1B6C_ (NetDaemon+0x1B6C); the ack mask, turn-end marker and cleared word are primary_state_.trailing_words_[7], leading_words_[0] and trailing_words_[8] (NetDaemon+0x1B60, +0x1B30, +0x1B64); the referenced sent, ack and turn-end words are existing members, not new globals", "source:src/console.h:ConsoleGlobal.faction_id_; source:src/game_data_state_window.h:GameDataStateStorage.fields_.turn_current_num_ and lock_masks_.enable_mask_; source:src/game.h:ExitTurnLoop", "pinned branch review: preserve early returns before close/mask reset, timeout > 1000, phase>=4 popup suppression, live callback mask rereads, server receive loop, common cleanup order and final ExitTurnLoop reread", "historical seed:git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/00528ea0.cpp; existing TRIED note remains attached verbatim"], "reason": "Keep the multiplayer turn rendezvous in the existing game-turn owner, using the already typed NetDaemon lock-state, Console, game-turn, GameDataStateWindow lock-mask and WinMM APIs while retaining this complete original body identity."}
// TRIED: `int shiftAmt = *g_00939284; ... 1 << shiftAmt` vs a two-statement `unsigned int bit = 1; bit <<= shiftAmt;` vs computing the bitmask load before/after the shift - all three schedule the same as the original up to instruction #5, where the original still has a bare `mov eax, 1` that the rebuilt folds away before the `shl`. Rest of the function (the do/while upkeep-poll loop, the two log_say/message_data pairs, the NetDaemon/NetMsg calls) is a straight disassembly transcription.
// kind      game
// Native daemon words: 0x0093E8F0 receives peer acknowledgements and
// 0x0093E8FC marks this faction at entry. The literals below replace their
// seed aliases at 0x0068BB88..0x0068BBC0; timeGetTime uses its SDK import.

/*
Purpose: Run the multiplayer turn's upkeep rendezvous: mark the local
         faction in the sent mask, redraw the multiplayer window, log and
         broadcast the upkeep phase, then pump NetDaemon::net_tasks (with
         the "waiting for player N" popup every second) until every enabled
         faction is acknowledged; on the turn-end gate the same pump runs
         NetDaemon::receive instead, and the masks clear on the way out.

// size      503 bytes
// prototype int (__cdecl ?net_upkeep_phase@@YAHH@Z)(int32 phase)
// callers   1   call targets   10
// flags     frame;sp_ready;purged_ok
// calls     0x0047A890 0x00592EE0 0x00593780 0x005D5A70 0x005DAE60 0x005DAEA0 0x00645470 0x0064FC88 0x00669368 0x006454E1
// notes     Seed provenance (TRIED): the `1 << shiftAmt` schedule matches
//           the original only up to instruction #5 - the image keeps a bare
//           `mov eax, 1` the rebuilt folds away - so this body is not
//           measured exact; the rest is the seed's transcription.
Return Value: the exit-turn flag
Status: Coverage transcription - unclaimed body, seed
        git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/00528ea0.cpp retained.
*/
int __cdecl net_upkeep_phase(int phase) {
    char number_buffer[0x50];
    uint32_t &sent_mask = NetDaemonNet.field_1B6C_;
    uint32_t &ack_mask = NetDaemonNet.primary_state_.trailing_words_[7];

    sent_mask |= 1u << ConsoleGlobal.faction_id_;
    MultiWindow.draw(0);

    log_say("Upkeep Phase", phase, GameDataStateStorage.fields_.turn_current_num_,
            static_cast<int>(ack_mask));
    message_data(0x2303, 0, phase,
                 GameDataStateStorage.fields_.turn_current_num_, 0, 0);

    DWORD time_base = timeGetTime();
    int sent_upkeep = phase >= 4 ? 1 : 0;
    while ((static_cast<uint8_t>(
                GameDataStateStorage.fields_.lock_masks_.enable_mask_) &
            ack_mask) != static_cast<uint8_t>(
                GameDataStateStorage.fields_.lock_masks_.enable_mask_)) {
        if (ExitTurnLoop != 0) break;

        const DWORD now = timeGetTime();
        if (now - time_base > 0x3E8) {
            if (sent_upkeep == 0) {
                parse_num(0, static_cast<int>(ack_mask));
                parse_num(1, static_cast<uint8_t>(
                    GameDataStateStorage.fields_.lock_masks_.enable_mask_));
                StringTemp[0] = 0;
                strcat(StringTemp, "UPKEEP");
                _itoa(phase, number_buffer, 10);
                strcat(StringTemp, number_buffer);
                NetMsgPrimary.pop(StringTemp, 0, 1,
                                  static_cast<const char *>(0));
                sent_upkeep = 1;
            }
            time_base = timeGetTime();
        }

        NetDaemonNet.net_tasks();
    }

    if (ExitTurnLoop != 0) return 1;

    if (NetDaemonNet.primary_state_.leading_words_[0] != 0) {
        log_say("Server sending upkeep message", phase, 0, 0);
        message_data(0x4303, 0, phase,
                     GameDataStateStorage.fields_.turn_current_num_, 0, 0);
        ack_mask = 0;
        while ((static_cast<uint8_t>(
                    GameDataStateStorage.fields_.lock_masks_.enable_mask_) &
                ack_mask) != static_cast<uint8_t>(
                    GameDataStateStorage.fields_.lock_masks_.enable_mask_)) {
            NetDaemonNet.receive();
        }
    } else {
        log_say("Client falling out of upkeep loop", phase, 0, 0);
    }

    NetMsgPrimary.close();
    sent_mask = 0;
    ack_mask = 0;
    return ExitTurnLoop;
}
// RECOVERY_ASSOCIATED: v1 switch_targets 0x005290A0 0x00529BE4 0x00529BF8 0x005297EF
// ORIGINAL: 0x005290A0 ?net_upkeep@@YAXXZ 0x005290A0-0x00529BE2
// RECOVERY_BINDING: v1 {"evidence": ["historical seed:src/unrecovered/005290a0.cpp", "task:net-turn-coordinator-owner", "ConsoleGlobal9156B0+23BE4/+23BE8 owns939294/939298; UpgradePromptAnswered90EA3C, TurnRedrawPending703DE0 and CouncilPendingMessageArgs68BF38 already own their storage. Do not create competing aliases.", "Native prototypes64..511 use34-byte records and flags+30/obsolete_factions+2C, not seed1A stride; peer checksums use8x21-word existing storage. Preserve wait/abort ordering and masked PlayerLock active-bit test; extent/lifetime/runtime and retained-link evidence remain bounded."], "reason": "Recover the complete multiplayer turn operation through actual source-owned objects, arrays, members and SDK calls; preserve historical identity, spans and seed discoveries without implying game acceptance."}
// TRIED: direct global-arithmetic translation of the Ghidra listing (fixed globals, DeletionList/NetDaemon/NetMsg/Console members, NET_DRAIN() macro for the receive-loop idiom) matches the first 10 instructions then diverges (xor vs mov) - register allocation for the 5-way local flags, not a structural difference.
void __cdecl net_upkeep() {
    unsigned int flags;
    int i;

    CommlinkPending = 0;
    DiploWindow.close();
    DiploDefaultFlag93A968 = 0;
    NetUpkeepPhase = 0;
    TurnUpkeepPhase = 1;
    NetDaemonNet.primary_state_.trailing_words_[8] = 0;
    SpockHelpActive = 0;
    SpockNotifyPending = 0;

    for (i = 0; i < GameDataStateStorage.fields_.base_current_count_; ++i) {
        Bases[i].state &= 0xEFFFFFEFu;
        if (Bases[i].faction_id_current == ConsoleGlobal.faction_id_) {
            synch_radius(i);
        }
    }
    for (i = 0; i < GameDataStateStorage.fields_.vehicle_current_count_; ++i) {
        veh_promote(i);
    }

    if (net_upkeep_phase(NetUpkeepPhase) != 0) return;
    ++NetUpkeepPhase;

    for (i = 0; i < 8; ++i) {
        const int faction = (GameDataStateStorage.fields_.turn_current_num_ + i) % 8;
        if (((1 << faction) &
             GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_) != 0) {
            faction_upkeep(faction);
        }
    }

    if (net_upkeep_phase(NetUpkeepPhase) != 0) return;
    ++NetUpkeepPhase;
    message_data(0x2305, 0,
                 static_cast<int32_t>(NetDaemonNet.network_message_count()),
                 0, 0, 0);

    if (NetDaemonNet.primary_state_.trailing_words_[8] != 0) {
        for (i = 0; i < GameDataStateStorage.fields_.base_current_count_; ++i) {
            const Base &base = Bases[i];
            if (base.faction_id_current == ConsoleGlobal.faction_id_ &&
                (base.state & 0x10000010u) != 0) {
                synch_base(i);
                while (NetDaemonNet.receive() != 0) { }
                do_all_tasks();
            }
        }
        for (int faction = 0; faction < 8; ++faction) {
            if (faction == ConsoleGlobal.faction_id_) {
                synch_leader(faction);
            }
        }
        if (NetDaemonNet.primary_state_.trailing_words_[8] != 0) {
            while (NetDaemonNet.receive() != 0) { }
            do_all_tasks();
            if (net_upkeep_phase(NetUpkeepPhase) != 0) return;
        } else {
            goto skip_upkeep_synch;
        }
    } else {
skip_upkeep_synch:
// RECOVERY_LITERAL: v1 0x0068BBE4 0x0068BBFA $SG67353 0x005290A0
        log_say("Skipping upkeep synch", NetUpkeepPhase, 0, 0);
    }

    TurnUpkeepPhase = 2;
    ++NetUpkeepPhase;
    {
        flags = 0;
        for (unsigned int faction = 1; faction < MaxPlayerNum; ++faction) {
            const int bit = 1 << faction;
            if ((bit & GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_) != 0) {
                while (PlayersData[faction].earned_techs_saved != 0) {
// RECOVERY_LITERAL: v1 0x0068BBFC 0x0068BC0B $SG67363 0x005290A0
                    log_say("Upkeep Advance", Players[faction].name_leader,
                            faction, PlayersData[faction].earned_techs_saved, 0);
                    --PlayersData[faction].earned_techs_saved;
                    tech_advance(static_cast<int>(faction));
                    if ((bit & GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_) != 0) {
                        flags |= (faction ==
                                  static_cast<unsigned int>(ConsoleGlobal.faction_id_))
                            + 1u;
                    }
                }
                if (PlayersData[faction].tech_accumulated != 0 &&
                    PlayersData[faction].tech_id_researching < 0) {
// RECOVERY_LITERAL: v1 0x0068BC0C 0x0068BC19 $SG67368 0x005290A0
                    log_say("Set Research", faction, 0, 0);
                    PlayersData[faction].tech_id_researching =
                        tech_selection(static_cast<int>(faction));
                    if ((bit & GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_) != 0) {
                        flags |= (faction ==
                                  static_cast<unsigned int>(ConsoleGlobal.faction_id_))
                            + 1u;
                    }
                }
                if ((PlayersData[faction].flags & 2u) != 0) {
                    consider_designs(static_cast<int>(faction));
                }
                PlayersData[faction].flags &= ~2u;
            }
        }

        if (flags != 0) {
            if (net_upkeep_phase(NetUpkeepPhase) != 0) {
                return;
            }
            TurnUpkeepPhase = 0;
        } else {
// RECOVERY_LITERAL: v1 0x0068BC1C 0x0068BC32 $SG67377 0x005290A0
            log_say("Skipping upkeep synch", NetUpkeepPhase, 0, 0);
            TurnUpkeepPhase = 0;
        }
        ++NetUpkeepPhase;
        if ((flags & 2u) != 0 && (GameDataStateStorage.fields_.game_rules_ & 0x200u) != 0) {
            ConsoleGlobal.set_ai(ConsoleGlobal.faction_id_);
        }
    }

// RECOVERY_LITERAL: v1 0x0068BC34 0x0068BC3C $SG67379 0x005290A0
    NetMsgPrimary.pop("UPKEEP4", 0, 1, 0);
    {
        flags = 0;
        UpgradePromptAnswered = 0;
        for (i = 0x40; i < MaxVehProtoNum; ++i) {
            const uint16_t proto_flags = VehPrototypes[i].flags;
            if ((proto_flags & 1u) != 0) {
                const int faction = i / 64;
                const int faction_bit = 1 << faction;
                if ((VehPrototypes[i].obsolete_factions & faction_bit) == 0 &&
                    static_cast<short>(proto_flags & 0x100u) != 0) {
                    flags = 1;
                    VehPrototypes[i].flags = static_cast<uint16_t>(proto_flags & 0xFEFFu);
                    if (faction == ConsoleGlobal.faction_id_ &&
                        (GameDataStateStorage.fields_.game_more_preferences_ & 0x100000u) != 0) {
                        upgrade_prototypes(ConsoleGlobal.faction_id_,
                                           i);
                    }
                }
            }
        }

        if (CouncilPendingMessageArgs[0] > 0) {
            if (NetDaemonNet.primary_state_.leading_words_[0] != 0) {
                message_data(0x2600, 0, CouncilPendingMessageArgs[0],
                             CouncilPendingMessageArgs[1],
                             CouncilPendingMessageArgs[2], 0);
                TurnRedrawPending = 1;
                while (TurnRedrawPending != 0) {
                    if (ExitTurnLoop != 0) break;
                    wait_loop();  // 0x0052951C calls wait_loop (0x0051F200), not wait_task
                }
            }
            CouncilPendingMessageArgs[0] = -1;
            CouncilPendingMessageArgs[1] = -1;
            CouncilPendingMessageArgs[2] = -1;
            goto prototype_synch;
        }
        if (flags != 0) {
prototype_synch:
            if (net_upkeep_phase(NetUpkeepPhase) != 0) return;
        } else {
// RECOVERY_LITERAL: v1 0x0068BC3C 0x0068BC52 $SG67402 0x005290A0
            log_say("Skipping upkeep synch", NetUpkeepPhase, 0, 0);
        }
    }

    ++NetUpkeepPhase;
// RECOVERY_LITERAL: v1 0x0068BC54 0x0068BC5C $SG67403 0x005290A0
    NetMsgPrimary.pop("UPKEEP5", 0, 1, 0);
    for (i = 0; i < 8; ++i) {
        const int bit = 1 << i;
        if ((GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_ & bit) != 0 &&
            (GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & bit) == 0) {
// RECOVERY_LITERAL: v1 0x0068BC5C 0x0068BC68 $SG67409 0x005290A0
            log_say("Enemy turn ", i, 0, 0);
            enemy_turn(i);
        }
    }
    if (net_upkeep_phase(NetUpkeepPhase) != 0) return;
    ++NetUpkeepPhase;
// RECOVERY_LITERAL: v1 0x0068BC68 0x0068BC70 $SG67411 0x005290A0
    NetMsgPrimary.pop("UPKEEP6", 0, 1, 0);
    NetDaemonNet.deletion_list_.set_sequence_bits(0);
    for (i = 0; i < 0x18; ++i) {
        NetDaemonNet.deletion_list_.clear(i);
    }
    do_checksums(4);

    if ((GameDataStateStorage.fields_.game_preferences_ & 2u) != 0) {
        if (BaseAtDebugPopupFlag != 0) {
            StringTemp[0] = 0;
// RECOVERY_LITERAL: v1 0x0068BC70 0x0068BC89 $SG67417 0x005290A0
            strcat(StringTemp, "k:\\breynolds\\multisaves\\");
// RECOVERY_LITERAL: v1 0x0068BC8C 0x0068BC8F $SG67418 0x005290A0
            strcat(StringTemp, "AC");
            char number_text[80];
            _itoa(ConsoleGlobal.faction_id_, number_text, 10);
            strcat(StringTemp, number_text);
// RECOVERY_LITERAL: v1 0x0068BC90 0x0068BC96 $SG67420 0x005290A0
            strcat(StringTemp, "multi");
            _itoa(GameDataStateStorage.fields_.turn_current_num_, number_text, 10);
            strcat(StringTemp, number_text);
// RECOVERY_LITERAL: v1 0x0068BC98 0x0068BCAA $SG67421 0x005290A0
            log_say("Writing save file", StringTemp, 0, 0, 0);
            save_daemon(StringTemp);
        } else {
            auto_save();
        }
    }

    message_big_data(0x2307, 0, NetTurnChecksums, 0x15);
    if (net_upkeep_phase(NetUpkeepPhase) != 0) return;
    NetDaemonNet.primary_state_.trailing_words_[1] = 0xFFFFFFFFu;
    ++NetUpkeepPhase;

    {
        flags = 0;
        // Original 0x0052972D clears 21 DWORDs; 0x005297C0/0x005297D0
        // read and set four-byte flags, not packed byte flags.
        int mismatch_seen[NetTurnChecksumWords];
        for (int clear = 0; clear < NetTurnChecksumWords; clear++) {
            mismatch_seen[clear] = 0;
        }

        for (int faction = 1; faction < MaxPlayerNum; ++faction) {
            if ((GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_ & (1 << faction)) == 0 ||
                (GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ & (1 << faction)) == 0 ||
                faction == ConsoleGlobal.faction_id_) {
                continue;
            }

            for (i = 0; i < NetTurnChecksumWords; ++i) {
                if (NetTurnChecksums[i] == NetPeerTurnChecksums[faction][i]) continue;
                if (flags == 0) {
// RECOVERY_LITERAL: v1 0x0068BCAC 0x0068BCB4 $SG67440 0x005290A0
                    NetMsgPrimary.pop("UPKEEP7", 0, 1, 0);
                }
                flags = 1;
                if (mismatch_seen[i] != 0) continue;
                mismatch_seen[i] = 1;
                if (NetDaemonNet.primary_state_.leading_words_[0] == 0) {
                    continue;
                }

                switch (i) {
                case 0:
// RECOVERY_LITERAL: v1 0x0068BCB4 0x0068BCD3 $SG67448 0x005290A0
                    log_say("Failed GAME checksum - resynch", faction, NetTurnChecksums[0],
                            NetPeerTurnChecksums[faction][0]);
                    NetDaemonNet.synch(0, 0, 0, 0, 0, 1, 0x2101);
                    while (NetDaemonNet.receive() != 0) { }
                    NetDaemonNet.synch(0x24, 0, 0, 0, 0, 1, 0x2101);
                    while (NetDaemonNet.receive() != 0) { }
                    break;
                case 1: {
// RECOVERY_LITERAL: v1 0x0068BCD4 0x0068BCF6 $SG67456 0x005290A0
                    log_say("Failed LEADERS checksum - resynch", faction, NetTurnChecksums[1],
                            NetPeerTurnChecksums[faction][1]);
                    int a = 0;
                    int b = 0;
                    do {
                        NetDaemonNet.synch(6, a, 0, 0, 0, 1, 0x2101);
                        while (NetDaemonNet.receive() != 0) { }
                        NetDaemonNet.synch(7, a, 0, 0, 0, 1, 0x2101);
                        while (NetDaemonNet.receive() != 0) { }
                        NetDaemonNet.synch(0x0F, b, 0x40, 0, 0, 1,
                                           0x2101);
                        while (NetDaemonNet.receive() != 0) { }
                        b += 0x40;
                        ++a;
                    } while (b < 0x200);
                    break;
                }
                case 2:
// RECOVERY_LITERAL: v1 0x0068BCF8 0x0068BD16 $SG67472 0x005290A0
                    log_say("Failed MAP checksum - resynch", faction, NetTurnChecksums[2],
                            NetPeerTurnChecksums[faction][2]);
                    NetDaemonNet.synch(0x1002, 0, 0, 0, 0, 1, 0x2101);
                    while (NetDaemonNet.receive() != 0) { }
                    NetDaemonNet.synch(5, 0, 0, 0, 0, 1, 0x2101);
                    while (NetDaemonNet.receive() != 0) { }
                    break;
                case 3:
// RECOVERY_LITERAL: v1 0x0068BD18 0x0068BD3B $SG67480 0x005290A0
                    log_say("Failed VEHICLES checksum - resynch", faction, NetTurnChecksums[3],
                            NetPeerTurnChecksums[faction][3]);
                    NetDaemonNet.synch(0x25, 0, 0, 0, 0, 1, 0x2101);
                    while (NetDaemonNet.receive() != 0) { }
                    NetDaemonNet.synch(0x12, 0, 0, 0, 0, 1, 0x2101);
                    while (NetDaemonNet.receive() != 0) { }
                    break;
                case 4:
// RECOVERY_LITERAL: v1 0x0068BD3C 0x0068BD5C $SG67488 0x005290A0
                    log_say("Failed BASES checksum - resynch", faction, NetTurnChecksums[4],
                            NetPeerTurnChecksums[faction][4]);
                    NetDaemonNet.synch(0x26, 0, 0, 0, 0, 1, 0x2101);
                    while (NetDaemonNet.receive() != 0) { }
                    NetDaemonNet.synch(0x14, 0, 0, 0, 0, 1, 0x2101);
                    while (NetDaemonNet.receive() != 0) { }
                    break;
                default:
                    // Original 0x00529B16 reloads the local checksum through
                    // the 0x0093A008 cursor, independently of peer row zero.
// RECOVERY_LITERAL: v1 0x0068BD5C 0x0068BD81 $SG67496 0x005290A0
                    log_say("Failed MAP section chcksum - resynch", faction,
                            NetTurnChecksums[i], NetPeerTurnChecksums[faction][i]);
                    NetDaemonNet.synch(0x30, i - 5, 0, 0, 0, 1,
                                       0x2101);
                    while (NetDaemonNet.receive() != 0) { }
                    break;
                }
            }
        }

        NetMsgPrimary.close();
        SpockHelpActive = 0;
        SpockNotifyPending = 0;
        if (flags != 0) {
            net_upkeep_phase(NetUpkeepPhase);
            return;
        }
// RECOVERY_LITERAL: v1 0x0068BD84 0x0068BD9A $SG67501 0x005290A0
        log_say("Skipping upkeep phase", NetUpkeepPhase, 0, 0);
    }
}

// ORIGINAL: 0x00529C00 ?mash_planes@@YAXXZ 0x00529C00-0x00529E67 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:b34e6d99635022d29b9b9831a6b0c37bd11d9692:src/recovered/units/00529c00.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "recovery/tasks/game-logic-seed-owner-coverage.json; complete typed owner migration, preserve original spans and seed lessons. Literal content provenance does not assert original allocation or game acceptance."], "reason": "Native aircraft attrition: vehicle movement spent at+0x28 controls eligibility; damage at+0x10 accumulates reactor-derived attrition; terraforming turns at+0x25 clears on survival. Prototype+0x2A is plan, chassis+0x4A is range; preserve post-focus/boom prototype reads."}
void __cdecl mash_planes() {
    for (int veh_id = GameDataStateStorage.fields_.vehicle_current_count_ - 1; veh_id >= 0; --veh_id) {
        const int initial_chassis_id = VehPrototypes[Vehs[veh_id].proto_id].chassis_id;
        if (Chassis[initial_chassis_id].triad != TRIAD_AIR || Chassis[initial_chassis_id].range == 0) {
            continue;
        }
        if (!range(speed(veh_id, false) - Vehs[veh_id].moves_expended, 0, 999)) {
            continue;
        }
        if (base_who(Vehs[veh_id].x, Vehs[veh_id].y) >= 0) {
            continue;
        }
        if ((bit_at(Vehs[veh_id].x, Vehs[veh_id].y) & 0x00040000) != 0 ||
            stack_check(veh_id, 6, 0x80, -1, -1) != 0) {
            continue;
        }

        int focused = 0;
        const int owner = Vehs[veh_id].faction_id;
        if (owner == ConsoleGlobal.faction_id_ ||
            Vehs[veh_id].visibility & (1 << ConsoleGlobal.faction_id_)) {
            ConsoleGlobal.focus(Vehs[veh_id].x, Vehs[veh_id].y, owner);
            focused = 1;
        }
        Vehs[veh_id].state |= 0x4;
        if (focused) {
            boom(Vehs[veh_id].x, Vehs[veh_id].y, 0x80);
        }

        // Native 0x529D43 begins a fresh prototype lookup after both possible
        // callbacks. They may replace the selected vehicle's design.
        if (Chassis[VehPrototypes[Vehs[veh_id].proto_id].chassis_id].range == 1
            && VehPrototypes[Vehs[veh_id].proto_id].chassis_id == 6) {
            Vehs[veh_id].dmg_incurred = static_cast<uint8_t>(Vehs[veh_id].dmg_incurred
                + VehPrototypes[Vehs[veh_id].proto_id].reactor_id * 3);
        } else {
            // proto_power's body, on the prototype already selected above.
            Vehs[veh_id].dmg_incurred = static_cast<uint8_t>(Vehs[veh_id].dmg_incurred + range(
                (VehPrototypes[Vehs[veh_id].proto_id].plan == PLAN_ALIEN_ARTIFACT
                    ? 1 : range(VehPrototypes[Vehs[veh_id].proto_id].reactor_id, 1, 100) * 10)
                - Vehs[veh_id].dmg_incurred, 0, 9999));
        }

        // The terminal check has another native prototype reload at 0x529DD9.
        if (range(proto_power(veh_id) - Vehs[veh_id].dmg_incurred, 0, 9999) <= 0) {
            kill(veh_id);
        } else {
            Vehs[veh_id].terraforming_turns = 0;
            draw_tile(Vehs[veh_id].x, Vehs[veh_id].y, 2);
        }
    }
}

// ---------------------------------------------------------------------------
// net_not_my_turn, 0x00529E70. The end-of-turn wait for the local player in
// a network game: mark the turn as handed over, redraw the multiplayer
// window, latch this faction into the waiting set, then pump the network
// daemon and skip forward through absent factions until the active turn
// comes back around.
/*
Purpose: Wait out the other factions' turns in a network game, skipping
         forward through absent players.
// ORIGINAL: 0x00529E70 ?net_not_my_turn@@YAXXZ 0x00529E70-0x00529F2C FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original529E70..529F2C; retired seed git:ee30a69d:src/unrecovered/00529e70.cpp records the same identity and spans with its scheduling TRIED note kept below.", "The turn block is console.h's own ConsoleGlobal (0x009156B0, set_view), game.h's MultiWindow (0x007FD648, the hypothesis layout's draw), netdaemon.h's NetDaemonNet (0x0093CD90, net_tasks) and game.h's ExitTurnLoop (0x009B2068); the waiting latch at 0x0093E8EC takes this faction's bit and 0x00939294/0x00939298 are the two turn-handover flags.", "game.cpp binds 0x0093A950 as NextTurnReady and 0x0093E8C0 as TurnsAssigned; 0x009A6820 is game.cpp's NetTurnFaction and the skip tests are lock.h's LockEnableMask bytes at 0x009A64E8/0x009A64E9.", "next_player_turn (game.cpp's 0x0052DBF0) has no header declaration yet, so the declaration travels with this body.", "Pinned 0x00529EA4 and 0x00529EFF use direct 32-bit SHL by CL. The first shifts the local faction into the waiting set; the guarded active-turn test retains a full-width bit while testing the human/alive bytes. Do not add source count masks or narrow the second shift temporary."], "reason": "Coverage transcription of the network turn wait on the tree's own named objects."}
// TRIED (seed): byte-exact to 94.5% mnemonic similarity; the remaining
//         divergence is the arg-0 push order for set_view against the two
//         independent global stores.
Status: Coverage transcription - unclaimed body
*/
void __cdecl net_not_my_turn() {
    ConsoleGlobal.field_23BE4_ = 0;
    ConsoleGlobal.field_23BE8_ = 1;
    ConsoleGlobal.set_view(0);
    MultiWindow.draw(0);
    NetDaemonNet.factions_waiting_mask() |=
        (1u << ConsoleGlobal.faction_id_);
    while (ExitTurnLoop == 0 && NextTurnReady == 0
           && GameDataStateStorage.fields_.net_turn_faction_ != ConsoleGlobal.faction_id_) {
        ConsoleGlobal.field_23BE4_ = 1;
        NetDaemonNet.net_tasks();
        if (NetDaemonNet.primary_state_.leading_words_[0] != 0) {
            const int level = GameDataStateStorage.fields_.net_turn_faction_;
            if (level <= 0) {
                next_player_turn();
            } else {
                const uint32_t bit = 1u << level;
                const GameDataLockMaskBytes *const lock_bytes =
                    &GameDataStateStorage.fields_.lock_masks_.bytes_;
                if ((lock_bytes->human_mask_ & bit) == 0) {
                    next_player_turn();
                } else if ((lock_bytes->alive_mask_ & bit) == 0) {
                    next_player_turn();
                }
            }
        }
    }
    ConsoleGlobal.field_23BE4_ = 0;
    ConsoleGlobal.field_23BE8_ = 0;
}









// ORIGINAL: 0x00529F30 ?net_end_of_turn@@YAXXZ 0x00529F30-0x0052A2D3 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical seed:src/recovered/units/00529f30.cpp", "task:net-turn-coordinator-owner", "ConsoleGlobal9156B0+23BE4/+23BE8 owns939294/939298; UpgradePromptAnswered90EA3C, TurnRedrawPending703DE0 and CouncilPendingMessageArgs68BF38 already own their storage. Do not create competing aliases.", "Native prototypes64..511 use34-byte records and flags+30/obsolete_factions+2C, not seed1A stride; peer checksums use8x21-word existing storage. Preserve wait/abort ordering and masked PlayerLock active-bit test; extent/lifetime/runtime and retained-link evidence remain bounded."], "reason": "Recover the complete multiplayer turn operation through actual source-owned objects, arrays, members and SDK calls; preserve historical identity, spans and seed discoveries without implying game acceptance."}
void __cdecl net_end_of_turn() {
    if ((GameDataStateStorage.fields_.net_turn_flags_ & 0x10u) != 0) {
        if (NetDaemonNet.primary_state_.leading_words_[0] != 0) {
            message_data(0x4301, 0, 0, 0, 0, 0);
        } else {
            while (ExitTurnLoop == 0 && NextTurnReady == 0) {
                NetDaemonNet.net_tasks();
            }
        }
    }

// RECOVERY_LITERAL: v1 0x0068BD9C 0x0068BDB8 $SG67549 0x00529F30
    log_say("Entering 'end of turn' loop",
            static_cast<int>(NetDaemonNet.factions_waiting_mask()),
            NetDaemonNet.field_1B68_,
            GameDataStateStorage.fields_.game_state_ & 2u);
    g_FX.play(0x22);
    GameDataStateStorage.fields_.game_state_ |= 2u;
    ConsoleGlobal.field_23BE4_ = 0;
    ConsoleGlobal.set_view(0);

    const int local_faction = ConsoleGlobal.faction_id_;
    if (NetDaemonNet.primary_state_.leading_words_[0] == 0) {
        NetDaemonNet.factions_waiting_mask() |= 1u << local_faction;
    }
    NetDaemonNet.field_1B68_ |= 1u << local_faction;
    MultiWindow.draw(0);

    if ((GameDataStateStorage.fields_.net_turn_flags_ & 0x10u) != 0) {
        goto proceed_next_turn;
    }

    message_data(0x8301, 0, 0, 0, 0, 0);
    {
        const DWORD start_time = timeGetTime();
        int sent_finishing = 0;
        for (;;) {
            if ((static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) &
                 NetDaemonNet.factions_waiting_mask()) ==
                    static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) &&
                (NetDaemonNet.primary_state_.trailing_words_[3] &
                 static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_)) == 0 &&
                (NetDaemonNet.primary_state_.leading_words_[0] == 0 ||
                 (NetDaemonNet.locks_.any_locks() == 0 &&
                  NetDaemonNet.field_1B7C_ == 0))) {
                break;
            }
            if (ExitTurnLoop != 0 || (GameDataStateStorage.fields_.game_state_ & 2u) == 0) {
                break;
            }
            if (sent_finishing == 0 &&
                static_cast<DWORD>(timeGetTime() - start_time) > 0x3E8) {
// RECOVERY_LITERAL: v1 0x0068BDB8 0x0068BDC6 $SG67569 0x00529F30
                NetMsgPrimary.pop("FINISHINGTURN", 0, 0, nullptr);
                sent_finishing = 1;
            }
            NetDaemonNet.net_tasks();
        }
        if (sent_finishing != 0) {
            NetMsgPrimary.close();
        }
    }

    if ((GameDataStateStorage.fields_.game_state_ & 2u) != 0) {
        if (NetDaemonNet.primary_state_.leading_words_[0] == 0) {
// RECOVERY_LITERAL: v1 0x0068BDC8 0x0068BDD9 $SG67574 0x00529F30
            log_say("Ready to proceed",
                    static_cast<int>(NetDaemonNet.factions_waiting_mask()),
                    GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 0xFFu, 0);
        } else {
            goto handle_93e8c0;
        }
    }
    if (NetDaemonNet.primary_state_.leading_words_[0] != 0) {
handle_93e8c0:
        if ((GameDataStateStorage.fields_.game_state_ & 2u) == 0) {
            return;
        }
        if (ExitTurnLoop != 0) {
            goto after_wait;
        }
        for (;;) {
            int locked_count = 0;
            // Native begins at 0x93E24C, which is records_[1], and visits
            // slots 1 .. player_count_-1 with the proven 0x1C PlayerLock stride.
            for (int slot = 1; slot < NetDaemonNet.player_count_; ++slot) {
                PlayerLock &player_lock = NetDaemonNet.locks_.records_[slot];
                if (player_lock.active() != 0 &&
                    (player_lock.active_ & 1u) == 0) {
                    ++locked_count;
                }
            }
            if (locked_count == 0) {
                break;
            }
            wait_task();
            NetDaemonNet.receive();
        }
        message_data(0x4301, 0, 0, 0, 0, 0);
        NetDaemonNet.receive();
    }

after_wait:
    if ((GameDataStateStorage.fields_.game_state_ & 2u) == 0) {
        return;
    }

proceed_next_turn:
// RECOVERY_LITERAL: v1 0x0068BDDC 0x0068BDFC $SG67595 0x00529F30
    log_say("--- Proceeding to next turn ---", 0, 0, 0);
    NetDaemonNet.primary_state_.trailing_words_[7] = 0;
    NetDaemonNet.field_1B6C_ = 0;
    NetUpkeepPhase = 0;
    ConsoleControlTurnActive = 1;
    if (TutWinBaseWindow.is_visible() != 0) {
        TutWinBaseWindow.exit();
    }
    go_reset();
    while (NetDaemonNet.receive() != 0) {
    }
    mash_planes();
    do_checksums(0);

    if ((GameDataStateStorage.fields_.game_preferences_ & 2u) != 0 && BaseAtDebugPopupFlag != 0) {
        StringTemp[0] = 0;
// RECOVERY_LITERAL: v1 0x0068BDFC 0x0068BE15 $SG67601 0x00529F30
        strcat(StringTemp, "k:\\breynolds\\multisaves\\");
// RECOVERY_LITERAL: v1 0x0068BE18 0x0068BE1B $SG67602 0x00529F30
        strcat(StringTemp, "AC");
        char number[80];
        _itoa(ConsoleGlobal.faction_id_, number, 10);
        strcat(StringTemp, number);
// RECOVERY_LITERAL: v1 0x0068BE1C 0x0068BE20 $SG67604 0x00529F30
        strcat(StringTemp, "end");
        _itoa(GameDataStateStorage.fields_.turn_current_num_, number, 10);
        strcat(StringTemp, number);
// RECOVERY_LITERAL: v1 0x0068BE20 0x0068BE32 $SG67605 0x00529F30
        log_say("Writing save file", StringTemp, 0, 0, 0);
        save_daemon(StringTemp);
    }
    NetDaemonNet.deletion_list_.advance_sequence();
}







// ORIGINAL: 0x0052A2E0 ?net_control_turn@@YAXXZ 0x0052A2E0-0x0052A971 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical seed:src/unrecovered/0052a2e0.cpp", "task:net-turn-coordinator-owner", "ConsoleGlobal9156B0+23BE4/+23BE8 owns939294/939298; UpgradePromptAnswered90EA3C, TurnRedrawPending703DE0 and CouncilPendingMessageArgs68BF38 already own their storage. Do not create competing aliases.", "Native prototypes64..511 use34-byte records and flags+30/obsolete_factions+2C, not seed1A stride; peer checksums use8x21-word existing storage. Preserve wait/abort ordering and masked PlayerLock active-bit test; extent/lifetime/runtime and retained-link evidence remain bounded."], "reason": "Recover the complete multiplayer turn operation through actual source-owned objects, arrays, members and SDK calls; preserve historical identity, spans and seed discoveries without implying game acceptance."}
// TRIED: faithful port of the turn-control loop (checksum/log setup, the active-faction bitmask scan requiring both 0x9a64e8 and 0x9a64e9 bits set, the base-then-veh center-view fallback search, the human/AI turn dispatch with the net_tasks polling loops, and the two net_end_of_turn continuation checks incl. the dead defensive branch Ghidra also shows). Compiles; diverges at instr #0 on stack-frame size (local declarations spread across nested blocks change the `sub esp` prologue vs the original's single big frame). Not chased further.
void __cdecl net_control_turn() {
    char log_path[80];

    draw_map(1);
    my_srand(MapRandSeed);
    ConsoleControlTurnActive = 1;
    MapBaseDisplayOverride = 1;

// RECOVERY_LITERAL: v1 0x0068BE34 0x0068BE40 $SG67610 0x0052A2E0
    if (prefs_get("Multi Debug", 0, 0) != 0 &&
// RECOVERY_LITERAL: v1 0x0068BE40 0x0068BE4B $SG67611 0x0052A2E0
        X_pop("MULTIDEBUG", 0) != 0) {
        StringTemp[0] = 0;
// RECOVERY_LITERAL: v1 0x0068BE4C 0x0068BE5D $SG67612 0x0052A2E0
        strcat(StringTemp, "k:\\breynolds\\log");
        _itoa(ConsoleGlobal.faction_id_, log_path, 10);
        strcat(StringTemp, log_path);
// RECOVERY_LITERAL: v1 0x0068BE60 0x0068BE65 $SG67613 0x0052A2E0
        strcat(StringTemp, ".txt");
// RECOVERY_LITERAL: v1 0x0068BE68 0x0068BE6B $SG67615 0x0052A2E0
        FILE *file = fopen(StringTemp, "wt");
        if (file != 0) {
            fclose(file);
        } else {
            StringTemp[0] = 0;
// RECOVERY_LITERAL: v1 0x0068BE6C 0x0068BE70 $SG67618 0x0052A2E0
            strcat(StringTemp, "log");
            _itoa(ConsoleGlobal.faction_id_, log_path, 10);
            strcat(StringTemp, log_path);
// RECOVERY_LITERAL: v1 0x0068BE70 0x0068BE75 $SG67619 0x0052A2E0
            strcat(StringTemp, ".txt");
        }
        Logging.init(StringTemp);
        Logging.reset();
        BaseAtDebugPopupFlag = 1;
        log_set_state(1);
    } else {
        BaseAtDebugPopupFlag = 0;
        log_set_state(0);
    }
    do_checksums(1);
    if ((GameDataStateStorage.fields_.net_turn_flags_ & 0x10u) != 0) {
        TurnControlWord93A954 = 1;
    }

    for (;;) {
// RECOVERY_LITERAL: v1 0x0068BE78 0x0068BE92 $SG67625 0x0052A2E0
        log_say("(Clearing turn semaphore)",
                NetDaemonNet.primary_state_.trailing_words_[6],
                NetDaemonNet.field_1B68_, 0);
        NetDaemonNet.primary_state_.trailing_words_[6] = 0;
        NetDaemonNet.field_1B68_ = 0;
        NetDaemonNet.primary_state_.trailing_words_[0] = 1;
        // The adjacent 0x93E968 store is distinct from the message count and
        // precedes it in the image.
        NetDaemonNet.field_1BD8_ = 0;
        NetDaemonNet.network_message_count() = 0;
        NextTurnReady = 0;
        InterfaceMessageWin.clear();

        if (ControlTurnPhase == 0) {
            for (GameDataStateStorage.fields_.net_turn_faction_ = 1; !((1 << GameDataStateStorage.fields_.net_turn_faction_) & GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_) || !((1 << GameDataStateStorage.fields_.net_turn_faction_) & GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_); ++GameDataStateStorage.fields_.net_turn_faction_) {
            }
            turn_upkeep();
            net_upkeep();
            GameDataStateStorage.fields_.net_turn_faction_ = 0;
            if (end_of_game(1) != 0) {
                NetDaemonNet.hang_up();
                NetDaemonNet.reset();
                IsMultiplayerNet = 0;
                return;
            }
        }

        set_time_controls();
        NetDaemonWaitTicks = static_cast<int>(GameDataStateStorage.fields_.turn_control_duration_milliseconds_);
        ConsoleControlTurnActive = 0;
        NetDaemonNet.primary_state_.trailing_words_[4] = 0;
        GameDataStateStorage.fields_.game_state_ &= 0xFFFFF7FFu;

        if ((GameDataStateStorage.fields_.net_turn_flags_ & 0x10u) != 0) {
            if (GameDataStateStorage.fields_.turn_current_num_ == 1 || ControlTurnPhase != 0) {
                int y = -1;
                int x = -1;
                for (int id = 0; id < GameDataStateStorage.fields_.base_current_count_; ++id) {
                    if (Bases[id].faction_id_current == ConsoleGlobal.faction_id_) {
                        x = Bases[id].x;
                        y = Bases[id].y;
                        break;
                    }
                }
                if (x < 0) {
                    for (int id = 0; id < GameDataStateStorage.fields_.vehicle_current_count_; ++id) {
                        if (Vehs[id].faction_id == ConsoleGlobal.faction_id_) {
                            x = Vehs[id].x;
                            y = Vehs[id].y;
                            break;
                        }
                    }
                }
                if (y >= 0 && y < MapLatitudeBounds &&
                    x >= 0 && x < MapLongitudeBounds) {
                    ConsoleGlobal.set_view(0);
                    ConsoleGlobal.set_center(x, y, 1);
                    ConsoleGlobal.set_cursor(x, y);
                }
            }
            if (NetDaemonNet.primary_state_.leading_words_[0] != 0) {
                if (ControlTurnPhase == 0) {
                    next_player_turn();
                } else {
                    message_data(0x4309, 0, GameDataStateStorage.fields_.net_turn_faction_, 0, 0, 0);
                }
            }
        }
        ControlTurnPhase = 0;

        for (;;) {
            if (ExitTurnLoop == 0) {
                if ((GameDataStateStorage.fields_.net_turn_flags_ & 0x10u) != 0 &&
                    GameDataStateStorage.fields_.net_turn_faction_ != ConsoleGlobal.faction_id_) {
                    ConsoleGlobal.set_view(0);
                    ConsoleGlobal.field_23BE4_ = 0;
                    ConsoleGlobal.field_23BE8_ = 1;
                    ConsoleGlobal.set_view(0);
                    MultiWindow.draw(0);
                    NetDaemonNet.factions_waiting_mask() |= 1u << ConsoleGlobal.faction_id_;
                    while (ExitTurnLoop == 0 && NextTurnReady == 0 &&
                           GameDataStateStorage.fields_.net_turn_faction_ != ConsoleGlobal.faction_id_) {
                        ConsoleGlobal.field_23BE4_ = 1;
                        NetDaemonNet.net_tasks();
                        if (NetDaemonNet.primary_state_.leading_words_[0] != 0 &&
                            (GameDataStateStorage.fields_.net_turn_faction_ <= 0 ||
                             !(GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_ &
                               (1 << GameDataStateStorage.fields_.net_turn_faction_)) ||
                             !(GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_ &
                               (1 << GameDataStateStorage.fields_.net_turn_faction_)))) {
                            next_player_turn();
                        }
                    }
                    ConsoleGlobal.field_23BE4_ = 0;
                    ConsoleGlobal.field_23BE8_ = 0;
                } else {
                    if ((GameDataStateStorage.fields_.net_turn_flags_ & 0x10u) == 0 &&
                        (GameDataStateStorage.fields_.game_state_ & 0x800u) != 0) {
                        goto turn_skipped;
                    }
                    if ((GameDataStateStorage.fields_.net_turn_flags_ & 0x10u) != 0) {
                        NetDaemonWaitTicks = static_cast<int>(GameDataStateStorage.fields_.turn_control_duration_milliseconds_);
                        GameDataStateStorage.fields_.game_state_ &= 0xFFFFF7FFu;
                        TurnControlWord93A954 = 0;
                        NetDaemonNet.factions_waiting_mask() &= ~(1u << ConsoleGlobal.faction_id_);
                    }
                    ConsoleGlobal.human_turn();
                    if ((GameDataStateStorage.fields_.net_turn_flags_ & 0x10u) != 0) {
                        TurnControlWord93A954 = 1;
                        while (ExitTurnLoop == 0 && NetDaemonNet.locks_.any_locks() != 0) {
                            NetDaemonNet.net_tasks();
                        }
                        message_data(0x8301, 0, 0, 0, 0, 0);
                        while (ExitTurnLoop == 0 &&
                               GameDataStateStorage.fields_.net_turn_faction_ == ConsoleGlobal.faction_id_ &&
                               NextTurnReady == 0) {
                            NetDaemonNet.net_tasks();
                        }
                        if (NextTurnReady == 0) {
                            GameDataStateStorage.fields_.game_state_ &= 0xFFFFF7FFu;
                        }
                    }
                }
            } else {
turn_skipped:
                GameDataStateStorage.fields_.game_state_ |= 2u;
            }
            if (ExitTurnLoop != 0) break;
            if ((GameDataStateStorage.fields_.net_turn_flags_ & 0x10u) == 0) {
                net_end_of_turn();
                if (ExitTurnLoop != 0) break;
            }
            if (!((GameDataStateStorage.fields_.net_turn_flags_ & 0x10u) != 0
                      ? NextTurnReady == 0
                      : (~GameDataStateStorage.fields_.game_state_ >> 1) & 1u)) {
                break;
            }
        }
        if ((GameDataStateStorage.fields_.net_turn_flags_ & 0x10u) != 0) {
            net_end_of_turn();
        }
        if (ExitTurnLoop != 0) return;
    }
}

// ORIGINAL: 0x0052A980 sub_52a980 0x0052A980-0x0052A9A5 FILE BYTE_EXACT
// symbol    ?sub_52a980@@YAHXZ
// size      37 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// PRESERVED UNIT - measured MISMATCH.
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
// address        0x0052A980
// measured tier  MISMATCH
// divergence     2
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0052a980/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_52a980  at 0x0052A980  (37 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    an indirect virtual call use the generated VCall shim below; for a
//    thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.

// Historical candidate note: the const-pointer spelling reproduced the old
// fixed-address encoding; it is not the current typed-object implementation.
int __cdecl sub_52a980() {
    // Signed 16-bit flags: VC6 narrows the clear to `and byte ptr [+0x30], 0xfe`.
    for (int prototype_id = 0; prototype_id < MaxVehProtoNum; ++prototype_id) {
        VehPrototype &prototype = VehPrototypes[prototype_id];
        prototype.flags &= ~PROTO_ACTIVE;
        prototype.icon_offset = -1;
    }
    GameDataStateStorage.fields_.vehicle_current_count_ = 0;
    GameDataStateStorage.fields_.base_current_count_ = 0;
    return 0;
}

// The original walks eight factions, two terrain variants, four size tiers
// and three defense tiers, storing pointers to successive Sprite members.
// The real pointer table and its pointees share faction.cpp ownership.
// The preserved LEVER/TRIED notes below describe the superseded raw-storage
// spelling, not a requirement to keep an executable-address table.

// ORIGINAL: 0x0052A9B0 sub_52a9b0 0x0052A9B0-0x0052AA2D FILE BYTE_EXACT
// symbol    ?directplay_install_is_complete@@YAHXZ
// LEVER: the six fixed-address bindings renamed or deleted. Three of them -
//         0x00669000, 0x0066900C and 0x00669010 - are ADVAPI32 import slots
//         (tools/iat_names.py: RegQueryValueExA, RegOpenKeyExA, RegCloseKey),
//         so the dllimport declarations replace them outright. The other three
//         are .rdata literals, read with tools/image_data.py:
//         0x0068BE94 is the DirectPlay Applications key for this game,
//         0x0068BEDC is "Complete" and 0x0068BEE8 is "Yes". String literals
//         fold to their own addresses, so the pushes come out the same.
// LEVER: BYTE_EXACT on declaring `int result = 0` at the TOP of the body.
//         The image zeroes esi before RegOpenKeyExA and then pushes esi twice
//         as the call's two `0` arguments - which it can only do because the
//         result local is already live. Declaring `result` after the call
//         cost VC6 the shared zero, and every `0` argument then materialised
//         as its own `push 0` (24/49, prologue `lea` where the image has
//         `push esi`).

// These three TU-private literals are shared by control_game and this helper
// in the current object. Both pinned bodies refer to these same original
// starts; the bindings do not merge them with copies from other translation
// units or claim linked retention.
// RECOVERY_LITERAL: v1 0x0068BE94 0x0068BEDA $SG67589 0x0052A9B0
// RECOVERY_LITERAL: v1 0x0068BEDC 0x0068BEE5 $SG67595 0x0052A9B0
// RECOVERY_LITERAL: v1 0x0068BEE8 0x0068BEEC $SG67598 0x0052A9B0
int __cdecl directplay_install_is_complete() {
    int result = 0;
    HKEY key;
    LONG status = RegOpenKeyExA(HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Microsoft\\DirectPlay\\Applications\\"
        "Sid Meier's Alpha Centauri", 0, 0, &key);
    if (status != 0) {
        return 0;
    }
    DWORD valueType;
    char valueBuf[12];
    DWORD valueSize = 10;
    status = RegQueryValueExA(key, "Complete", 0, &valueType,
        reinterpret_cast<BYTE *>(valueBuf), &valueSize);
    if (status == 0) {
        if (_strcmpi(valueBuf, "Yes") == 0) {
            result = 1;
        }
    }
    RegCloseKey(key);
    return result;
}

// 0x00689218 - the DirectPlay application GUID the lobby check registers the
// session under, bytes read off the image. The trailing words spell "DEST".
// RECOVERY_OBJECT: v2 0x00689218 0x00689228 ?LobbyAppGuid@@3U_GUID@@B 0x004E31E0
const GUID LobbyAppGuid = {0x02D1B480, 0xD680, 0x11D0,
                                  {0x82, 0x1E, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00}};

// WORKAROUND(decl): stand-in for an unidentified header inline that ors bits
// into MapWin::field_1DD70_. The image materializes 0x8C0 in a callee-saved
// register before the turn-wheel loop (`mov ebx, 0x8c0` at 0x0052AB8D) and ors
// through it (0x0052ACDD); only an inline call's parameter copy reproduces that
// hoist - every literal spelling folds to `or reg, 0x8c0`.
static inline void control_game_set_map_flags(MapWin *window, uint32_t flags) {
    window->field_1DD70_ |= flags;
}

/*
Purpose: Run the game. Brings the system and game up, decides the intro
         movie, then turns the wheel: menu or multiplayer handshake, desktop,
         one turn, teardown - until nothing queues another game.
// ORIGINAL: 0x0052AA30 ?control_game@@YAXXZ 0x0052AA30-0x0052AD2D FILE
// TRIED: MISMATCH #4 push/call - straight g_* global derefs and direct member calls gave the allocator no reason to hold ebx/esi/edi live across the whole body the way the original's cached fn-ptr (ebx) and zero/flag temps (esi/edi) do.
// LEVER: /Oy- flag set, not /O2 alone - the image keeps the ebp frame, and
// under /Oy- this body's prologue, spills ([ebp-8] for lobby_status) and
// tail all line up (2026-08-29: 19/215, best-similar set).
// LEVER (2026-11-07): VC6 itself hoists the GetAsyncKeyState import into ebx
// for the two direct calls; game_reload is out of line (auto_inline off);
// the intro-complete registry probe is the `shift || probe()` argument.
// LEVER: the loop clears lobby_status after the menu/multiplayer branch
// (`mov [ebp-8], esi` at 0x0052AC7D) - only the first pass runs the lobby
// handshake. The first transcription dropped that store; without it the
// image's re-read of [ebp-8] has no writer. NetTurnFlags went uint8_t ->
// uint32_t on the same pass: the image clears it with a whole dword.
// TRIED: `int window_flags = 0x8C0;` before the loop, volatile RMW, and the
// bare literal - all fold to `or reg, 0x8c0` where the image materializes
// 0x8C0 into a dead callee-saved register before the loop (`mov ebx, 0x8c0`
// at 0x0052AB8D) and ors through it at 0x0052ACDD. Not reached from source.
// TRIED: this tree's VC6 caches the literal 1 in a callee-saved register
// from the first `game_init(0, 1)` (`mov edi, 1` ahead of the prologue
// pushes) where the image pushes the immediate every time; spelling the call
// args, dropping the window_flags local, and both flag families leave it
// unchanged.
Status: homed from src/unrecovered/0052aa30.cpp on 2026-08-29. MISMATCH
        against the best-similar flag set, /O2 /Gy /GR- /Oy- /GX; the
        remaining runs are register-role permutation (image ebx/esi/edi =
        fn-ptr+0x8C0/zero/found-to-minus-1; this tree shifts the roles) and
        the two TRIED folds. Callees still original are forwarders in
        pending_bodies.cpp.
*/
void __cdecl control_game() {
    int lobby_status;
    int movie_disabled;

    if (system_init() != 0 || game_init(0, 1) != 0) {
        return;
    }
    {

            GameRestartQueued = 0;
            WorldClimateSkipTerrainClear = 1;
            ExitTurnLoop = 0;
            IsMultiplayerNet = 0;
            lobby_status = NetDaemonNet.Net::check_for_lobby(CommandLineText,
                                                              const_cast<_GUID *>(&LobbyAppGuid), 3, 9);
            
            if (lobby_status == 0) {
                // Holding Shift skips the lobby's "is the intro complete" registry
                // look-up and takes the movie path directly.
                filefind_init("movies\\virtualWorld.wve",
                              HIBYTE(GetAsyncKeyState(VK_SHIFT)) || directplay_install_is_complete());

                prefs_load(0);
                GameDataStateStorage.fields_.game_preferences_ = AlphaIniPrefs.preferences;
                movie_disabled = prefs_get("DisableOpeningMovie", 0, 0);
                prefs_put("DisableOpeningMovie", movie_disabled, 0);
                if (movie_disabled == 0 && (GameDataStateStorage.fields_.game_preferences_ & 0x10000000) != 0) {
                    amovie_project("openingx");
                }
            }

            if ((GetAsyncKeyState(VK_SHIFT) & 0x8000) == 0) {
                g_FX.init();
            }
            for (;;) {
                if (GameRestartQueued != 0) {
                    if (g_WAVE_GENERAL.Wave::is_playing() != 0) {
                        g_WAVE_GENERAL.Sound::fade(2000);
                    }
                    ConsoleGlobal.Win::hide();
                    do_all_draws();
                    strcpy(SaveNameBuffer, SaveGameFileName);
                    filefind_set_alternative(SaveNameBuffer);
                    if (game_reload(0, 1) != 0) {
                        goto close_game;
                    }
                    GameDataStateStorage.fields_.vehicle_current_count_ = 0;
                    setup_game(1);
                    GameDataStateStorage.fields_.game_rules_ = AlphaIniPrefs.rules;
                    GameDataStateStorage.fields_.net_turn_flags_ = 0;
                }

                GameRestartQueued = 0;
                WorldClimateSkipTerrainClear = 1;
                ExitTurnLoop = 0;
                IsMultiplayerNet = 0;
                flush_input();
                if (lobby_status == 0) {
                    if (top_menu(0) != 0) {
                        goto close_opening;
                    }
                } else {
                    prefs_load(0);
                    prefs_use();
                    if (multiplayer_init(1) != 0) {
                        goto close_opening;
                    }
                }

                // Only the first pass runs the lobby handshake; after the menu or the
                // multiplayer branch the loop clears it and every later pass comes
                // through top_menu.
                lobby_status = 0;

                if (IsMultiplayerPBEM != 0) {
                    ConsoleGlobal.cover_window_.fill(0);
                    ConsoleGlobal.cover_window_.Win::show(0);
                } else {
                    ConsoleGlobal.cover_window_.fill(9);
                    ConsoleGlobal.cover_window_.Win::hide();
                }
                if (desktop_init(ControlTurnPhase == 0) != 0) {
                    goto close_opening;
                }
                {
                    MapWin *const primary_window = MapWinTable[0];
                    WorldClimateSkipTerrainClear = 0;
                    control_game_set_map_flags(primary_window, 0x8C0);
                }
                ConsoleGlobal.focus_vehicle_ = -1;
                if (IsMultiplayerNet != 0) {
                    net_control_turn();
                } else {
                    control_turn();
                }
                desktop_close();
                if (GameRestartQueued == 0) {
                    goto close_game;
                }
            }

    close_opening:
            close_opening();
    close_game:
            game_close(0);
            system_close();
    }
}
