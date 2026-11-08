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

extern Wave g_CRASH_LANDING_WAVE;
extern int ConsoleNoMapOverrideLatch;
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

// These six literals are separate writable source objects because the image
// uses distinct storage starts for the read/write key, value names, and dialog
// label. The registry class strings are one-byte zero-filled views only; no
// larger capacity or independent writer is inferred here.
// RECOVERY_OBJECT: v1 0x0068AD88 _TamperReadApplicationKey
static char TamperReadApplicationKey[] =
    "SOFTWARE\\Microsoft\\DirectPlay\\Applications\\Sid Meier's Alpha Centauri";
// RECOVERY_OBJECT: v1 0x0068ADD8 _TamperWriteApplicationKey
static char TamperWriteApplicationKey[] =
    "SOFTWARE\\Microsoft\\DirectPlay\\Applications\\Sid Meier's Alpha Centauri";
// RECOVERY_OBJECT: v1 0x0068ADD0 _TamperReadValueMVM
static char TamperReadValueMVM[] = "MVM";
// RECOVERY_OBJECT: v1 0x0068ADD4 _TamperReadValueMVB
static char TamperReadValueMVB[] = "MVB";
// RECOVERY_OBJECT: v1 0x0068AE20 _TamperWriteValueMVM
static char TamperWriteValueMVM[] = "MVM";
// RECOVERY_OBJECT: v1 0x0068AE24 _TamperWriteValueMVB
static char TamperWriteValueMVB[] = "MVB";
static char TamperDialogLabel[] = "HOTNOTAMPER";
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
    LONG status = RegCreateKeyExA(HKEY_LOCAL_MACHINE,
        TamperReadApplicationKey, 0, TamperReadClass, 0, 0xf003f, 0, &key, 0);
    if (status == 0) {
        DWORD value_type;
        DWORD byte_count = 4;
        RegQueryValueExA(key, TamperReadValueMVM, 0, &value_type,
                         reinterpret_cast<LPBYTE>(&TamperRegistryCursor),
                         &byte_count);
        byte_count = 0x4B0;
        RegQueryValueExA(key, TamperReadValueMVB, 0, &value_type,
                         reinterpret_cast<LPBYTE>(TamperRegistryRecords),
                         &byte_count);
        RegCloseKey(key);
    }
}

// ORIGINAL: 0x0051E090 ?set_tamper@@YAXXZ 0x0051E090-0x0051E102 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:bb3b3233842b050fb0840e1cc6c226e8f30f7055:src/recovered/units/0051e090.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "recovery/tasks/game-logic-seed-owner-coverage.json"], "reason": "game.cpp beside its already-owned TamperWrite* strings, cursor and 50-record array; uses direct Windows registry APIs. Complete reviewed source translation; historical claims remain restoration obligations, not proof of current matching, linked preservation or game acceptance."}
void __cdecl set_tamper() {
    HKEY key;
    const LONG status = RegCreateKeyExA(
        HKEY_LOCAL_MACHINE, TamperWriteApplicationKey, 0, TamperWriteClass, 0,
        0xF003F, 0, &key, 0);
    if (status == ERROR_SUCCESS) {
        RegSetValueExA(key, TamperWriteValueMVM, 0, REG_BINARY,
                       reinterpret_cast<const BYTE *>(&TamperRegistryCursor), 4);
        RegSetValueExA(key, TamperWriteValueMVB, 0, REG_BINARY,
                       reinterpret_cast<const BYTE *>(TamperRegistryRecords),
                       0x4B0);
        RegCloseKey(key);
    }
}

// ORIGINAL: 0x0051E110 ?check_tamper@@YAXXZ 0x0051E110-0x0051E38B BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:9f6ee90ae7baa6354fcca006def5561fb956792b:src/recovered/units/0051e110.cpp", "recovery/tasks/game-save-load-owner-coverage.json"], "reason": "Recover the full PBEM tamper check using shared typed registry records and cursor, faction state, direct registry APIs and the original unguarded corrupt-cursor/error behavior."}
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
            X_pop(TamperDialogLabel, 0);
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
    memset(TamperDialogValue, 0, sizeof TamperDialogValue);
    memset(TamperDialogFaction, 0, sizeof TamperDialogFaction);
}

// Existing storage belongs to MessageWin's deferred-message owner.
extern StringList MessageWinDeferredBaseMessages[MaxPlayerNum];

// ORIGINAL: 0x0051E3C0 ?clear_all_player_messages@@YAXXZ 0x0051E3C0-0x0051E479 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/0051e3c0.cpp", "Pinned original01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "recovery/tests/test_player_message_state_behavior.py"], "reason": "Recover the complete player-message clear_all_player_messages operation through existing StringList/MessageWin owners and eight faction combat-summary counters. Preserve deletion hooks, live count reloads, reset/display order and retained original seed; no new measured claim."}
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
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/0051e480.cpp", "Pinned original01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "recovery/tests/test_player_message_state_behavior.py"], "reason": "Recover the complete player-message clear_player_messages operation through existing StringList/MessageWin owners and eight faction combat-summary counters. Preserve deletion hooks, live count reloads, reset/display order and retained original seed; no new measured claim."}
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

// ORIGINAL: 0x0051E530 sub_51e530 0x0051E530-0x0051E6ED
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/0051e530.cpp", "Pinned original01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "recovery/tests/test_player_message_state_behavior.py"], "reason": "Recover the complete player-message display_player_messages operation through existing StringList/MessageWin owners and eight faction combat-summary counters. Preserve deletion hooks, live count reloads, reset/display order and retained original seed; no new measured claim."}
// symbol    ?display_player_messages@@YAHH@Z
// TRIED: char* record-base arithmetic (self+0x30*a1 relative to 0x93a7b0) with adjustor-thunk VCall dispatch (slot000/slot001) for the two destroy-list virtual calls; reaches #13
// The id and text lookups are separate inline phases: head_ is reloaded.
static inline int deferred_message_id(StringList &items) {
    return items.head_ != 0 ? items.current_->id : 0;
}
static inline char *deferred_message_text(StringList &items) {
    return items.head_ != 0 ? items.current_->string_payload()->text : 0;
}

// StringStruct::seek_pos in the image's single-tail form (one
// current_position_ store after both arms), as dialog.cpp's local copy.
static inline int deferred_seek_pos(StringStruct *list, int a1) {
    AbsFnT abs_fn = reinterpret_cast<AbsFnT>(&abs);
    if (a1 > list->entry_count_ - 1) {
        return 0;
    }
    list->current_ = list->head_;
    if (a1 < 0) {
        if (abs_fn(a1) > list->entry_count_) {
            return 0;
        }
        int i = abs_fn(a1);
        while (i > 0) {
            --i;
            list->current_ = list->current_->previous;
        }
        a1 += list->entry_count_;
    } else {
        int i = a1;
        while (i > 0) {
            --i;
            list->current_ = list->current_->next;
        }
    }
    list->current_position_ = a1;
    return 1;
}

int __cdecl display_player_messages(int faction) {
    InterfaceMessageWin.clear();
    for (int index = 0; index < MessageWinDeferredBaseMessages[faction].entry_count_; ++index) {
        deferred_seek_pos(&MessageWinDeferredBaseMessages[faction], index);
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
// symbol    _sub_51e6f0
// size      106 bytes
// prototype
// callers   0   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x006263F0 0x00645470 0x0064FC88
Return Value: n/a
Status: Coverage transcription - claim pending re-measurement
*/
extern "C" void __cdecl sub_51e6f0(const char *text, int value) {
    char line[0x100];
    char digits[0x50];
    line[0] = 0;
    _itoa(value, digits, 10);
    strcat(line, digits);
    strcat(line, BufferJoinSpace);
    strcat(line, text);
    log_say_hex(line, value, 0, 0);
}

// RECOVERY_ASSOCIATED: v1 switch_targets 0x0051E760 0x0051F0A8 0x0051F0BC 0x0051E7AD
// ORIGINAL: 0x0051E760 ?do_checksums@@YAXH@Z 0x0051E760-0x0051F0A7
// RECOVERY_BINDING: v1 {"reason":"Recover checksum diagnostics through the real network producer, source-owned 21-word turn checksum storage, typed faction names and original local/shared text buffers. Preserve the original unwritten diagnostic tail; no byte or runtime acceptance claim.","evidence":["original:0x51E760-0x51F0A7; image:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf","git:b4e2d110:src/unrecovered/0051e760.cpp","Native51E830..51E840 copies21 words; producer appends26 diagnostic words only when BaseAtDebugPopupFlag is nonzero.","Original section and faction header loops mutate StringTemp; the remaining formatted reports use local buffers and _itoa radix10.","recovery/tasks/base-colony-economy-coverage-wave.json; recovery/tests/test_turn_checksums_behavior.py"]}
// TRIED: literal transcription of the ~25 itoa+strcat+log_say_hex report blocks and 3 walk loops (verified every log_say/log_say_hex call is (msg,value,0,0) with the message text pushed last=1st-param, confirmed against raw disasm push order, not assumed); the switch's dead "cmp esi,4;jne" fallthrough at 0x51E82B is provably unreachable (case4's own short jmp skips over it) and is omitted. Used a file-scope #define REPORT(...)/#undef macro instead of a helper function to stay inside "no helper" while avoiding 25x hand-duplicated blocks. Compiles; first divergence at instr #13 (deeper than most landings this batch), not chased to byte-exact given size (2375 bytes).
// kind      game
void __cdecl do_checksums(int type) {
    if (BaseAtDebugPopupFlag == 0 && type != 4) {
        return;
    }

    // NetDaemon::game_checksum writes 21 common values and, when the debug
    // popup flag is set, appends the diagnostic values through slot 46.
    // Keep the tail uninitialized when the producer does not write it: the
    // original type-4 consumer still reads it; no deterministic value is established.
    NetChecksumReport checksums;
    char text[256];
    char number[50];
    NetDaemonNet.game_checksum(&checksums);

    log_say(" ", 0, 0, 0);
    if (type <= 4) {
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
            memcpy(NetTurnChecksums, &checksums, 0x54);
            break;
        }
    }

    int common_index;
    for (common_index = 0; common_index < 5; ++common_index) {
        const char *label;
        switch (common_index) {
        case 0: label = "GAME checksum = "; break;
        case 1: label = "LEADERS checksum = "; break;
        case 2: label = "MAP checksum = "; break;
        case 3: label = "VEHICLES checksum = "; break;
        default: label = "BASES checksum = "; break;
        }
        text[0] = '\0';
        _itoa(checksums.common[common_index], number, 10);
        strcat(text, number);
        strcat(text, " ");
        strcat(text, label);
        log_say_hex(text, checksums.common[common_index], 0, 0);
    }

    log_say(" ", 0, 0, 0);
    int section_index;
    for (section_index = 0; section_index < 16; ++section_index) {
        StringTemp[0] = '\0';
        strcat(StringTemp, "Map SECTION ");
        _itoa(section_index, number, 10);
        strcat(StringTemp, number);
        strcat(StringTemp, " = ");

        text[0] = '\0';
        _itoa(checksums.map_sections[section_index], number, 10);
        strcat(text, number);
        strcat(text, " ");
        strcat(text, StringTemp);
        log_say_hex(text, checksums.map_sections[section_index], 0, 0);
    }

    int aggregate_index;
    for (aggregate_index = 0; aggregate_index < 3; ++aggregate_index) {
        const char *label;
        switch (aggregate_index) {
        case 0: label = "Rules checksum = "; break;
        case 1: label = "Projects checksum = "; break;
        default: label = "Tech checksum = "; break;
        }
        text[0] = '\0';
        _itoa(checksums.diagnostic_aggregates[aggregate_index], number, 10);
        strcat(text, number);
        strcat(text, " ");
        strcat(text, label);
        log_say_hex(text, checksums.diagnostic_aggregates[aggregate_index], 0, 0);
    }

    int player_index;
    for (player_index = 1; player_index < MaxPlayerNum; ++player_index) {
        StringTemp[0] = '\0';
        strcat(StringTemp, Players[player_index].adj_name_faction);
        strcat(StringTemp, " checksum = ");
        text[0] = '\0';
        _itoa(checksums.faction_values[player_index - 1], number, 10);
        strcat(text, number);
        strcat(text, " ");
        strcat(text, StringTemp);
        log_say_hex(text, checksums.faction_values[player_index - 1], 0, 0);
    }

    int map_report_index;
    for (map_report_index = 0; map_report_index < 7; ++map_report_index) {
        const char *label;
        switch (map_report_index) {
        case 0: label = "Map climate checksum = "; break;
        case 1: label = "Map region checksum = "; break;
        case 2: label = "Map bit checksum = "; break;
        case 3: label = "Map contour checksum = "; break;
        case 4: label = "Map terr checksum = "; break;
        case 5: label = "Map bitwho[0] checksum = "; break;
        default: label = "Params checksum = "; break;
        }
        text[0] = '\0';
        _itoa(checksums.map_values[map_report_index], number, 10);
        strcat(text, number);
        strcat(text, " ");
        strcat(text, label);
        log_say_hex(text, checksums.map_values[map_report_index], 0, 0);
    }

    int vehicle_faction;
    for (vehicle_faction = 0; vehicle_faction < MaxPlayerNum; ++vehicle_faction) {
        StringTemp[0] = '\0';
        strcat(StringTemp, Players[vehicle_faction].adj_name_faction);
        strcat(StringTemp, " Vehicles checksum = ");
        text[0] = '\0';
        _itoa(checksums.vehicles_by_faction[vehicle_faction], number, 10);
        strcat(text, number);
        strcat(text, " ");
        strcat(text, StringTemp);
        log_say_hex(text, checksums.vehicles_by_faction[vehicle_faction], 0, 0);
    }

    text[0] = '\0';
    _itoa(checksums.base_value, number, 10);
    strcat(text, number);
    strcat(text, " Bases checksum = ");
    log_say_hex(text, checksums.base_value, 0, 0);
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
// RECOVERY_BINDING: v1 {"evidence": ["git:bb3b3233842b050fb0840e1cc6c226e8f30f7055:src/unrecovered/0051f200.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "recovery/tasks/game-logic-seed-owner-coverage.json"], "reason": "faction.cpp / existing faction.h declaration: typed diplomacy-state loop; reloads ConsoleGlobal.faction_id_ at each loop head and after log_say/open_channel callbacks, and rereads the human-status byte each iteration, matching pinned 0x51F200 instructions. Uses existing NetDaemonNet, Console state, LockEnableMask human-status byte, FactionDiplomacyFlags, log_say and communicate. Complete reviewed source translation; historical claims remain restoration obligations, not proof of current matching, linked preservation or game acceptance."}
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
// RECOVERY_BINDING: v1 {"evidence": ["git:bb3b3233842b050fb0840e1cc6c226e8f30f7055:src/recovered/0051f2c0.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "recovery/tasks/game-logic-seed-owner-coverage.json"], "reason": "netdaemon.cpp / netdaemon.h: use existing NetDaemonNet.net_tasks(), IsMultiplayerNet and wait_task(). Complete reviewed source translation; historical claims remain restoration obligations, not proof of current matching, linked preservation or game acceptance."}
void __cdecl wait_2() {
    if (IsMultiplayerNet != 0) {
        NetDaemonNet.net_tasks();
    } else {
        wait_task();
    }
}

// RECOVERY_ASSOCIATED: v1 switch_targets 0x0051F2E0 0x00522120 0x00522178 0x0051F5F4
// ORIGINAL: 0x0051F2E0 ?random_events@@YAXH@Z 0x0051F2E0-0x0052211D
// RECOVERY_BINDING: v1 {"evidence": ["git:99d6e72bcd5ae3be3566222736528cb0dc57935c:src/unrecovered/0051f2e0.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "recovery/tasks/game-logic-seed-owner-coverage.json; preserve original extents and historical seed lessons. Ordinary owned literals have pinned content provenance, not exact original data-allocation claims.", "pinned native CFG 0x0051F2E0-0x0052211D, switch table 0x0097D044; event kinds 0-21, common selection and early returns retained.", "Native 0x0051F5A2-0x0051F5FC captures selected base coordinates before parser callbacks. Cases 7/8 reread coordinates; case 19 uses live base coordinates; cases 17/20/21 retain the captured coordinates. Contradictory case-20/21 climate tests remain in source rather than being optimized away as assumed dead behavior."], "reason": "Typed random-events dispatcher and all 22 native cases, called by turn_upkeep. Base selection captures faction and coordinates before parser callbacks; later case-specific live reads remain live. Original map/UI, lifecycle and notification services are supplied by their genuine source owners."}
// TRIED: mechanical register/stack-slot goto transliteration (260 labels, 39 independent per-case epilogues auto-detected from the pop/pop/ pop/mov-esp-ebp/pop/ret shape, a 22-case switch off 0x0097D044, two thiscall-via-fixed-ecx targets on Console/MapWin, one indirect GetKeyState call through g_00669330): compiles, MISMATCH sim 0.32 on /O2 /Oy- framed, divergence at instruction #33. Large negative 32-bit immediates (e.g. 0xfffffc18) need an explicit (int) cast before a signed compare, or MSVC types the literal unsigned and turns jge into jae - fixed once, may still be lurking elsewhere unverified.
void __cdecl random_events(int mode) {
    if (mode == 0) {
        // Preserve the original byte-level state transition: when bit 1 is
        // present the complete word is cleared before bit zero is examined.
        if ((GameDataStateStorage.fields_.global_energy_event_state_ & 2) != 0)
            GameDataStateStorage.fields_.global_energy_event_state_ = 0;
        if ((GameDataStateStorage.fields_.global_energy_event_state_ & 1) != 0)
            GameDataStateStorage.fields_.global_energy_event_state_ = 2;

        int sunspot_duration = GameDataStateStorage.fields_.sunspot_duration_;
        if (sunspot_duration < -1000) sunspot_duration = -1000;
        GameDataStateStorage.fields_.sunspot_duration_ = --sunspot_duration;
        if (sunspot_duration == 0)
            (void)POP2("NOMORESPOTS", "space_sm.pcx", -1);

        unsigned char lock_bits = static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_);
        for (int column = 1; column < 8; ++column) {
            if ((lock_bits & static_cast<unsigned char>(1u << column)) == 0)
                continue;
            for (int row = 1; row < 8; ++row) {
                if ((lock_bits & static_cast<unsigned char>(1u << row)) != 0)
                    continue;
                if ((PlayersData[row].diplo_treaties[column] & 0x30u) != 0)
                    continue;
                set_treaty(row, column, 0xC00, 0);
                lock_bits = static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_);
            }
        }

        int dust_duration = GameDataStateStorage.fields_.dust_cloud_duration_;
        if (dust_duration < 0) {
            GameDataStateStorage.fields_.dust_cloud_duration_ = 0;
        } else if (dust_duration > 0) {
            GameDataStateStorage.fields_.dust_cloud_duration_ = --dust_duration;
            if (dust_duration == 0)
                (void)POP2("NOMOREDUST", "stars_sm.pcx", -1);
        }

        const int base_count_for_timers = GameDataStateStorage.fields_.base_current_count_;
        for (int base_id = 0; base_id < base_count_for_timers; ++base_id) {
            Base &base = Bases[base_id];
            if ((base.event & 0x7E00u) == 0) continue;
            if (base.random_event_turns >= 0)
                --base.random_event_turns;
            if (base.random_event_turns == 0)
                base.event &= 0xFFFF81FFu;
        }

        if ((GameDataStateStorage.fields_.game_state_ & 0x100000u) != 0) {
            const int anniversary = static_cast<int>(game_year(GameDataStateStorage.fields_.turn_current_num_))
                                    - 0x8A2;
            if (anniversary % 0x50 == 0) {
                GameDataStateStorage.fields_.game_state_ &= ~0x100000u;
                (void)POP2("PERIHELIONENDS", "heat_sm.pcx", -1);
                return;
            }
        }
    }

    const int current_turn = GameDataStateStorage.fields_.turn_current_num_;
    if (current_turn > 50 && mode == 0) {
        const int anniversary = static_cast<int>(game_year(current_turn))
                                - 0x88E;
        if (anniversary % 0x50 == 0) {
            GameDataStateStorage.fields_.game_state_ |= 0x100000u;
            (void)POP2("PERIHELION", "heat_sm.pcx", -1);
            return;
        }
    }

    if ((GameDataStateStorage.fields_.game_rules_ & 0x8000u) != 0) return;
    const int turn_threshold = 0x4B - 10 * GameDataStateStorage.fields_.diff_level_current_;
    if (current_turn < turn_threshold) return;

    if (mode == 0) {
        const unsigned int seed = static_cast<unsigned int>(MapRandSeed) +
            13u * static_cast<unsigned int>(current_turn);
        random_reseed(static_cast<int>(seed));
    }

    int random_upper_bound = GameDataStateStorage.fields_.base_current_count_;
    if (random_upper_bound <= 100) random_upper_bound = 100;
    const int base_roll = random(0, random_upper_bound);
    if (base_roll >= GameDataStateStorage.fields_.base_current_count_) return;

    const int selected_base_id = base_roll;
    Base &selected_base = Bases[selected_base_id];
    if (selected_base.population_size <= 3) return;
    const int faction_id = selected_base.faction_id_current;
    if (PlayersData[faction_id].current_num_bases <= 1)
        return;
    if ((selected_base.event & 0x7E00u) != 0) return;

    const int event_kind = random(0, 0x16);
    const int base_x_snapshot = selected_base.x;
    const int base_y_snapshot = selected_base.y;

    (void)parse_num(0, 10);
    (void)parse_says(0, selected_base.name_string, -1, -1);
    StringTemp[0] = '\0';
    if (static_cast<unsigned int>(event_kind) > 0x15u) return;

    if (event_kind <= 5) {
        Base &base = Bases[selected_base_id];
        switch (event_kind) {
        case 0:
            if (GameDataStateStorage.fields_.faction_rankings_[7] == static_cast<unsigned int>(faction_id) &&
                ((static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) &
                  static_cast<uint8_t>(1u << faction_id)) == 0 ||
                 PlayersData[faction_id].diff_level > 1) &&
                random(0, 4) != 0) break;
            base.event |= BEVENT_BUMPER;
            strcat(StringTemp, "BUMPER");
            if (strlen(StringTemp) == 0) break;
            if (base.event & 0x7E00u) base.random_event_turns = 10;
            if (faction_id != ConsoleGlobal.faction_id_ && !IsMultiplayerPBEM)
                break;
            POP2(StringTemp, "bump_sm.pcx", base_roll);
            return;

        case 1: {
            const int required_rank =
                (GameDataStateStorage.fields_.turn_current_num_ >= 150) ? 5 : 4;
            if (PlayersData[faction_id].ranking <
                required_rank) break;
            base.event |= BEVENT_FAMINE;
            strcat(StringTemp, "FAMINE");
            if (strlen(StringTemp) == 0) break;
            if (base.event & 0x7E00u) base.random_event_turns = 10;
            if (faction_id != ConsoleGlobal.faction_id_ && !IsMultiplayerPBEM)
                break;
            if (Players[faction_id].rule_flags & 0x80)
                POP2(StringTemp, "al_starv_sm.pcx", base_roll);
            else
                POP2(StringTemp, "starv_sm.pcx", base_roll);
            return;
        }

        case 2:
            if (GameDataStateStorage.fields_.faction_rankings_[7] == static_cast<unsigned int>(faction_id) &&
                ((static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) &
                  static_cast<uint8_t>(1u << faction_id)) == 0 ||
                 PlayersData[faction_id].diff_level > 1) &&
                random(0, 4) != 0) break;
            base.event |= BEVENT_INDUSTRY;
            strcat(StringTemp, "INDUSTRY");
            if (strlen(StringTemp) == 0) break;
            if (base.event & 0x7E00u) base.random_event_turns = 10;
            if (faction_id != ConsoleGlobal.faction_id_ && !IsMultiplayerPBEM)
                break;
            POP2(StringTemp, "indbm_sm.pcx", base_roll);
            return;

        case 3: {
            const int required_rank =
                (GameDataStateStorage.fields_.turn_current_num_ >= 150) ? 5 : 4;
            if (PlayersData[faction_id].ranking <
                required_rank) break;
            base.event |= BEVENT_BUST;
            strcat(StringTemp, "BUST");
            if (strlen(StringTemp) == 0) break;
            if (base.event & 0x7E00u) base.random_event_turns = 10;
            if (faction_id != ConsoleGlobal.faction_id_ && !IsMultiplayerPBEM)
                break;
            POP2(StringTemp, "genwarning_sm.pcx", base_roll);
            return;
        }

        case 4:
            if (GameDataStateStorage.fields_.faction_rankings_[7] == static_cast<unsigned int>(faction_id) &&
                ((static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) &
                  static_cast<uint8_t>(1u << faction_id)) == 0 ||
                 PlayersData[faction_id].diff_level > 1) &&
                random(0, 4) != 0) break;
            base.event |= BEVENT_HEAT_WAVE;
            strcat(StringTemp, "HEATWAVE");
            if (strlen(StringTemp) == 0) break;
            if (base.event & 0x7E00u) base.random_event_turns = 10;
            if (faction_id != ConsoleGlobal.faction_id_ && !IsMultiplayerPBEM)
                break;
            POP2(StringTemp, "heat_sm.pcx", base_roll);
            return;

        case 5: {
            const int required_rank =
                (GameDataStateStorage.fields_.turn_current_num_ >= 150) ? 5 : 4;
            if (PlayersData[faction_id].ranking <
                required_rank) break;
            base.event |= BEVENT_CLOUD_COVER;
            strcat(StringTemp, "CLOUDCOVER");
            if (strlen(StringTemp) == 0) break;
            if (base.event & 0x7E00u) base.random_event_turns = 10;
            if (faction_id != ConsoleGlobal.faction_id_ && !IsMultiplayerPBEM)
                break;
            POP2(StringTemp, "cloud_sm.pcx", base_roll);
            return;
        }
        }
        return;
    }

    if (event_kind >= 9 && event_kind <= 11) {
        switch (event_kind) {
        case 9: {
            if (GameDataStateStorage.fields_.global_energy_event_state_ != 0)
                return;
            if (random(0, 5) != 0) return;
            bool changed = false;
            for (int faction = 1; faction < MaxPlayerNum; ++faction) {
                if (PlayersData[faction].satellites_[2] != 0 ||
                    PlayersData[faction].satellites_[3] != 0) {
                    PlayersData[faction].satellites_[2] = 0;
                    PlayersData[faction].satellites_[3] = 0;
                    changed = true;
                }
            }
            GameDataStateStorage.fields_.global_energy_event_state_ = 1;
            if (changed)
                (void)POP2("SOLARSTORM", "sun_sm.pcx", -1);
            else
                (void)POP2("SOLARFLARE", "heat_sm.pcx", -1);
            return;
        }

        case 10: {
            if (random(0, 5) != 0) return;
            bool changed = false;
            for (int faction = 1; faction < MaxPlayerNum; ++faction) {
                if (PlayersData[faction].satellites_[1] != 0) {
                    PlayersData[faction].satellites_[1] = 0;
                    changed = true;
                }
            }
            GameDataStateStorage.fields_.global_energy_event_state_ = 1;
            if (changed) (void)POP2("ASTEROID", "astm_sm.pcx", -1);
            return;
        }

        case 11: {
            const int turn = GameDataStateStorage.fields_.turn_current_num_;
            if (turn < 50) return;
            if (GameDataStateStorage.fields_.sunspot_duration_ > -40) return;
            if (GameDataStateStorage.fields_.sunspot_duration_ > -80 &&
                turn >= 80 && random(0, 3) != 0) return;
            GameDataStateStorage.fields_.sunspot_duration_ = random(0, 11) + 10;
            (void)parse_num(0, 20);
            (void)POP2("SUNSPOTS", "solar_sm.pcx", -1);
            return;
        }
        }
        return;
    }

    if (event_kind == 12 || event_kind == 13) {
        switch (event_kind) {
        case 12: {
            const int reserves = PlayersData[faction_id].energy_reserves;
            if (reserves > 1000) {
                const unsigned char human_bits =
                    static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_);
                if ((human_bits & static_cast<unsigned char>(1u << faction_id)) == 0 &&
                    reserves <= 2000) break;
                const int new_reserves =
                    (reserves + ((reserves < 0) ? 3 : 0)) >> 2;
                PlayersData[faction_id].energy_reserves = new_reserves;
                if (faction_id == ConsoleGlobal.faction_id_) {
                    parse_num(0, new_reserves);
                    ConsoleGlobal.update_data(0);
                    POP2("INFLATION", "markbm_sm.pcx", -1);
                    return;
                }
                if (IsMultiplayerPBEM) {
                    parse_num(0, new_reserves);
                    POP2("INFLATION", "markbm_sm.pcx", -1 - faction_id);
                    return;
                }
            } else if (PlayersData[faction_id].ranking <= 4 && reserves <= 500) {
                const int new_reserves = reserves * 2;
                PlayersData[faction_id].energy_reserves = new_reserves;
                if (faction_id == ConsoleGlobal.faction_id_) {
                    parse_num(0, new_reserves);
                    ConsoleGlobal.update_data(0);
                    POP2("ENERGYBOOM", "markbm_sm.pcx", -1);
                    return;
                }
            }
            break;
        }

        case 13: {
            if (GameDataStateStorage.fields_.turn_current_num_ < 75) break;
            const int required_ranking =
                (GameDataStateStorage.fields_.turn_current_num_ >= 150) ? 5 : 4;
            if (PlayersData[faction_id].ranking < required_ranking) break;

            int event_visible = 0;
            for (int radius_index = 0; radius_index < 21; ++radius_index) {
                int x = base_x_snapshot + RadiusOffsetX[radius_index];
                const int y = base_y_snapshot + RadiusOffsetY[radius_index];
                if ((MapIsFlat & 1) == 0) {
                    if (x < 0) x += MapLongitudeBounds;
                    else if (x >= MapLongitudeBounds) x -= MapLongitudeBounds;
                }
                if (y < 0 || y >= MapLatitudeBounds ||
                    x < 0 || x >= MapLongitudeBounds) continue;

                Map &tile = map_tiles()[y * static_cast<int>(MapLongitude) + (x >> 1)];
                if ((tile.bit & BIT_SOLAR_TIDAL) == 0) continue;
                const unsigned char faction_bit =
                    static_cast<unsigned char>(1u << faction_id);
                if ((PlayersData[faction_id].flags & 0x200) == 0 &&
                    (tile.visibility & faction_bit) == 0) continue;
                const int territory = whose_territory(faction_id, x, y, 0, 0);
                if (territory >= 0 && territory != faction_id) continue;
                if ((tile.climate & 0xE0) < 0x60) continue;

                if (IsMultiplayerPBEM) {
                    event_visible = 1;
                } else {
                    const unsigned char local_bit = static_cast<unsigned char>(
                        1u << ConsoleGlobal.faction_id_);
                    if (faction_id == ConsoleGlobal.faction_id_ ||
                        Bases[selected_base_id].visibility & local_bit) {
                        if (!event_visible) {
                            ConsoleGlobal.focus(base_x_snapshot, base_y_snapshot,
                                                ConsoleGlobal.faction_id_);
                            if ((GetAsyncKeyState(VK_SHIFT) & 0xFF00) == 0 &&
                                !IsMultiplayerNet) clock_wait(200);
                        }
                        event_visible |= (faction_id == ConsoleGlobal.faction_id_) + 1;
                    }
                }

                bit_set(x, y, BIT_SOLAR_TIDAL, 0);
                synch_bit(x, y, faction_id);
                for (int visible_faction = 1; visible_faction < 8;
                     ++visible_faction) {
                    tile.bit_visible[visible_faction - 1] &=
                        ~static_cast<uint32_t>(BIT_SOLAR_TIDAL);
                }
                if (event_visible) {
                    draw_tile(x, y, 2);
                    if ((GetAsyncKeyState(VK_SHIFT) & 0xFF00) == 0 &&
                        !IsMultiplayerNet && !IsMultiplayerPBEM)
                        clock_wait(50);
                }
            }
            if (event_visible) {
                POP2("HAIL", "hail_sm.pcx", base_roll);
                return;
            }
            break;
        }
        }
        return;
    }

    if (event_kind == 6) {
        const int facility_gate_a = 17;
        const int facility_gate_b = 18;
        const int project_slots[3] = { 0, 14, 26 };
        const int project_facilities[3] = { 70, 84, 96 };
        if (has_fac(facility_gate_a, selected_base_id, 0) ||
            has_fac(facility_gate_b, selected_base_id, 0)) {
            const int has_gate_b = has_fac(facility_gate_b, selected_base_id, 0);
            const int facility = has_gate_b ? 28 : 27;
            parse_say(1, Facility[facility].name, -1, -1);
            if (faction_id == ConsoleGlobal.faction_id_) {
                ConsoleGlobal.focus(Bases[selected_base_id].x,
                                    Bases[selected_base_id].y,
                                    ConsoleGlobal.faction_id_);
                (void)POP2("PROMETHEUS1", "biohazd_sm.pcx", base_roll);
            } else if (IsMultiplayerPBEM) {
                (void)POP2("PROMETHEUS1", "biohazd_sm.pcx", base_roll);
            }
            return;
        }

        int matched_project_slot = -1;
        for (int slot = 0; slot < 3; ++slot) {
            const int owner_base = base_project(project_slots[slot]);
            if (owner_base >= 0 &&
                Bases[owner_base].faction_id_current == faction_id) {
                matched_project_slot = slot;
                break;
            }
        }
        if (matched_project_slot >= 0) {
            parse_say(1, Facility[project_facilities[matched_project_slot]].name,
                      -1, -1);
            if (faction_id == ConsoleGlobal.faction_id_) {
                ConsoleGlobal.focus(Bases[selected_base_id].x,
                                    Bases[selected_base_id].y,
                                    ConsoleGlobal.faction_id_);
                (void)POP2("PROMETHEUS2", "biohazd_sm.pcx", base_roll);
            } else if (IsMultiplayerPBEM) {
                (void)POP2("PROMETHEUS2", "biohazd_sm.pcx", base_roll);
            }
            return;
        }

        const int required_ranking =
            GameDataStateStorage.fields_.turn_current_num_ >= 150 ? 5 : 4;
        if (PlayersData[faction_id].ranking < required_ranking) return;
        const int population = Bases[selected_base_id].population_size;
        const int upper_bound = (population + 1) / 2;
        (void)parse_num(1, upper_bound);

        bool visible = faction_id == ConsoleGlobal.faction_id_;
        if (!visible) {
            const unsigned int local_mask =
                static_cast<unsigned int>(1u << ConsoleGlobal.faction_id_);
            visible = (Bases[selected_base_id].visibility & local_mask) != 0;
        }
        // Original (0x51FBEC..0x51FE0B): `shown` is 1 for a visible event
        // base, then i+1 for each later viewer/PBEM base; POP2 gets shown-1.
        int shown = 0;
        if (visible) {
            shown = 1;
            ConsoleGlobal.focus(Bases[selected_base_id].x,
                                Bases[selected_base_id].y,
                                ConsoleGlobal.faction_id_);
        }

        const int longitude_bounds = MapLongitudeBounds;
        const int longitude_half = MapLongitude;
        const bool flat = (MapIsFlat & 1) != 0;
        const int base_count = GameDataStateStorage.fields_.base_current_count_;
        for (int candidate_id = 0; candidate_id < base_count; ++candidate_id) {
            const int selected_x = Bases[selected_base_id].x;
            const int selected_y = Bases[selected_base_id].y;
            const int candidate_x = Bases[candidate_id].x;
            const int candidate_y = Bases[candidate_id].y;
            int dx = candidate_x - selected_x;
            if (dx < 0) dx = -dx;
            if (!flat && dx > longitude_half) dx = longitude_bounds - dx;
            int dy = candidate_y - selected_y;
            if (dy < 0) dy = -dy;
            if (((dx + dy) >> 1) > upper_bound) continue;

            if (has_fac(facility_gate_a, candidate_id, 0) ||
                has_fac(facility_gate_b, candidate_id, 0)) continue;
            const int candidate_faction = Bases[candidate_id].faction_id_current;
            bool project_protected = false;
            for (int slot = 0; slot < 3; ++slot) {
                const int owner_base = base_project(project_slots[slot]);
                if (owner_base >= 0 &&
                    Bases[owner_base].faction_id_current == candidate_faction) {
                    project_protected = true;
                    break;
                }
            }
            if (project_protected) continue;

            const int old_population = Bases[candidate_id].population_size;
            Bases[candidate_id].population_size =
                static_cast<int8_t>((old_population + 1) / 2);
            draw_tile(Bases[candidate_id].x, Bases[candidate_id].y, 2);
            if (candidate_faction == ConsoleGlobal.faction_id_) {
                ConsoleGlobal.focus(candidate_x, candidate_y,
                                    ConsoleGlobal.faction_id_);
                shown = candidate_id + 1;
            } else if (IsMultiplayerPBEM) {
                shown = candidate_id + 1;
            }
        }

        if (shown) (void)POP2("PROMETHEUS0", "biohazd_sm.pcx", shown - 1);
        return;
    }

    if (event_kind == 7 || event_kind == 8) {
        // Unlike cases 12/13, 15/16, 17, 20 and 21, this path rereads the
        // selected base coordinates during its scan and again for the result.
        if (IsMultiplayerPBEM != 0) return;
        const int fallback_offset = random(0, 20) + 1;
        int selected_offset = fallback_offset;
        bool found_nearby_bonus = false;
        for (int offset = 0; offset < 9; ++offset) {
            const int longitude_bounds = MapLongitudeBounds;
            int x = Bases[selected_base_id].x + RadiusOffsetX[offset];
            if ((MapIsFlat & 1) == 0) {
                if (x < 0) x += longitude_bounds;
                else if (x >= longitude_bounds) x -= longitude_bounds;
            }
            const int y = Bases[selected_base_id].y + RadiusOffsetY[offset];
            if (y < 0 || y >= MapLatitudeBounds ||
                x < 0 || x >= longitude_bounds) continue;
            if (bonus_at(x, y, 0) == 0) continue;
            if ((PlayersData[faction_id].flags & 0x200u) == 0) {
                const unsigned int faction_mask =
                    static_cast<unsigned int>(1u << faction_id);
                if ((map_loc(x, y)->visibility & faction_mask) == 0) continue;
            }
            if ((map_loc(x, y)->climate & 0xE0u) >= 0x60u) {
                const int owner = whose_territory(faction_id, x, y, 0, 0);
                if (owner >= 0 && owner != faction_id) continue;
            }
            selected_offset = offset;
            found_nearby_bonus = true;
            break;
        }

        const int longitude_bounds = MapLongitudeBounds;
        int x = Bases[selected_base_id].x + RadiusOffsetX[selected_offset];
        if ((MapIsFlat & 1) == 0) {
            if (x < 0) x += longitude_bounds;
            else if (x >= longitude_bounds) x -= longitude_bounds;
        }
        const int y = Bases[selected_base_id].y + RadiusOffsetY[selected_offset];
        if (y < 0 || y >= MapLatitudeBounds ||
            x < 0 || x >= longitude_bounds) return;
        if ((PlayersData[faction_id].flags & 0x200u) == 0) {
            const unsigned int faction_mask =
                static_cast<unsigned int>(1u << faction_id);
            if ((map_loc(x, y)->visibility & faction_mask) == 0) return;
        }
        if ((map_loc(x, y)->climate & 0xE0u) >= 0x60u) {
            const int owner = whose_territory(faction_id, x, y, 0, 0);
            if (owner >= 0 && owner != faction_id) return;
        }

        const unsigned int resource_bit = 0x400u;
        if (found_nearby_bonus) {
            if (bonus_at(x, y, 0) != 0) {
                if (GameDataStateStorage.fields_.turn_current_num_ < 75)
                    return;
                const int bonus_id = bonus_at(x, y, 0) - 1;
                parse_say(1, Resource[bonus_id].name_plural, -1, -1);
                bit_set(x, y, (int)resource_bit, 0);
                synch_bit(x, y, faction_id);
                draw_tile(x, y, 2);
                if (bonus_at(x, y, 0) != 0) return;
                if (faction_id != ConsoleGlobal.faction_id_) return;

                console_map_win()->set_center(x, y, 1);
                const int initial_key = GetAsyncKeyState(0x10);
                if (((initial_key >> 8) & 0xFF) == 0) {
                    for (int i = 0; i < 10; ++i) {
                        bit_set(x, y, (int)resource_bit, 1);
                        synch_bit(x, y, faction_id);
                        draw_tile(x, y, 2);
                        if (((GetAsyncKeyState(0x10) >> 8) & 0xFF) == 0 &&
                            IsMultiplayerNet == 0) clock_wait(0x14);
                        bit_set(x, y, (int)resource_bit, 0);
                        synch_bit(x, y, faction_id);
                        draw_tile(x, y, 2);
                        if (((GetAsyncKeyState(0x10) >> 8) & 0xFF) == 0 &&
                            IsMultiplayerNet == 0) clock_wait(0x14);
                    }
                    if (((GetAsyncKeyState(0x10) >> 8) & 0xFF) == 0 &&
                        IsMultiplayerNet == 0) clock_wait(0x64);
                }
                const bool ocean = (map_loc(x, y)->climate & 0xE0u) < 0x60u;
                const char *image_key = bonus_id == 0
                    ? (ocean ? "rdneuw_sm.pcx" : "rdneul_sm.pcx")
                    : (ocean ? "rdminw_sm.pcx" : "rdminl_sm.pcx");
                (void)POP2("NEWRESOURCE", image_key, -1);
                return;
            }
        }

        bit_set(x, y, (int)resource_bit, 1);
        synch_bit(x, y, faction_id);
        draw_tile(x, y, 2);
        const int bonus_id = bonus_at(x, y, 0) - 1;
        if (bonus_id < 0) return;
        if (faction_id != ConsoleGlobal.faction_id_) return;
        console_map_win()->set_center(x, y, 1);
        const int initial_key = GetAsyncKeyState(0x10);
        if (((initial_key >> 8) & 0xFF) == 0) {
            for (int i = 0; i < 10; ++i) {
                bit_set(x, y, (int)resource_bit, 0);
                synch_bit(x, y, faction_id);
                draw_tile(x, y, 2);
                if (((GetAsyncKeyState(0x10) >> 8) & 0xFF) == 0 &&
                    IsMultiplayerNet == 0) clock_wait(0x14);
                bit_set(x, y, (int)resource_bit, 1);
                synch_bit(x, y, faction_id);
                draw_tile(x, y, 2);
                if (((GetAsyncKeyState(0x10) >> 8) & 0xFF) == 0 &&
                    IsMultiplayerNet == 0) clock_wait(0x14);
            }
            if (((GetAsyncKeyState(0x10) >> 8) & 0xFF) == 0 &&
                IsMultiplayerNet == 0) clock_wait(0x64);
        }
        parse_say(1, Resource[bonus_id].name_plural, -1, -1);
        const bool ocean = (map_loc(x, y)->climate & 0xE0u) < 0x60u;
        const char *image_key = bonus_id == 0
            ? (ocean ? "rdneuw_sm.pcx" : "rdneul_sm.pcx")
            : (ocean ? "rdminw_sm.pcx" : "rdminl_sm.pcx");
        (void)POP2("NEWRESOURCE", image_key, -1);
        return;
    }

    if (event_kind == 14) {
        // Native steps the byte offset through 0x140 inclusive, using radius
        // table entries 0..80; retain the 0x144 loop bound and live table reads.
        if (IsMultiplayerPBEM != 0 ||
            GameDataStateStorage.fields_.turn_current_num_ < 0x4B) return;
        PlayerData &event_player = PlayersData[faction_id];
        if (event_player.current_num_bases < 8) return;
        if ((GameDataStateStorage.fields_.game_state_ & 0x10u) != 0) return;

        const int event_x = static_cast<int>(GameDataStateStorage.fields_.mount_planet_x_);
        const int event_y = static_cast<int>(GameDataStateStorage.fields_.mount_planet_y_);
        if (event_y < 0 || event_y >= MapLatitudeBounds) return;
        if (event_x < 0 || event_x >= MapLongitudeBounds) return;
        Map &center = *map_loc(event_x, event_y);
        if ((center.bit2 & (BIT2_UNK_80000000 | BIT2_VOLCANO)) != BIT2_VOLCANO)
            return;

        const Base &selected = Bases[selected_base_id];
        int dx = static_cast<int>(selected.x) - event_x;
        if (dx < 0) dx = -dx;
        if ((MapIsFlat & 1u) == 0 && dx > MapLongitude)
            dx = MapLongitudeBounds - dx;
        const int dy = static_cast<int>(selected.y) - event_y;
        int near_distance = dx + ((dy < 0) ? -dy : dy);
        near_distance &= ~1;
        if (near_distance > 0xA) return;

        const uint32_t event_faction_bit = 1u << (faction_id & 31);
        if ((event_player.flags & PFLAG_MAP_REVEALED) == 0 &&
            (center.visibility & event_faction_bit) == 0) return;
        if (event_player.ranking < 6) return;

        const int local_faction = ConsoleGlobal.faction_id_;
        const uint32_t local_faction_bit = 1u << (local_faction & 31);
        bool effects_are_visible =
            (PlayersData[local_faction].flags & PFLAG_MAP_REVEALED) != 0 ||
            (center.visibility & local_faction_bit) != 0;
        int base_reference_for_news = selected_base_id;
        for (int offset_bytes = 0; offset_bytes < 0x144; offset_bytes += 4) {
            const int radius_index = offset_bytes >> 2;
            int x = event_x + RadiusOffsetX[radius_index];
            const int y = event_y + RadiusOffsetY[radius_index];
            if ((MapIsFlat & 1u) == 0) {
                if (x < 0) x += MapLongitudeBounds;
                else if (x >= MapLongitudeBounds) x -= MapLongitudeBounds;
            }
            if (y < 0 || y >= MapLatitudeBounds ||
                x < 0 || x >= MapLongitudeBounds) continue;

            bit_set(x, y, (int)0x81E8887Cu, 0);
            synch_bit(x, y, local_faction);
            rocky_set(x, y, ROCKINESS_ROCKY);
            Map &tile = *map_loc(x, y);
            const bool is_live_volcano =
                (tile.bit2 & (BIT2_UNK_80000000 | BIT2_VOLCANO)) == BIT2_VOLCANO;
            if (is_live_volcano) {
                for (int veh_id = GameDataStateStorage.fields_.vehicle_current_count_ - 1;
                     veh_id >= 0; --veh_id) {
                    const Veh &veh = Vehs[veh_id];
                    if (veh.x == x && veh.y == y) veh_kill(veh_id);
                }
            } else {
                for (int veh_id = GameDataStateStorage.fields_.vehicle_current_count_ - 1;
                     veh_id >= 0; --veh_id) {
                    Veh &veh = Vehs[veh_id];
                    if (veh.x != x || veh.y != y) continue;
                    const VehPrototype &prototype = VehPrototypes[veh.proto_id];
                    int incoming_damage;
                    if (prototype.plan == 0xC) {
                        incoming_damage = 1;
                    } else {
                        int reactor = prototype.reactor_id;
                        if (reactor < 1) reactor = 1;
                        else if (reactor > 100) reactor = 100;
                        incoming_damage = reactor * 10;
                    }
                    int remaining_damage = incoming_damage - veh.dmg_incurred;
                    if (remaining_damage < 0) remaining_damage = 0;
                    else if (remaining_damage > 0x270F) remaining_damage = 0x270F;
                    const int half_damage =
                        (remaining_damage + ((remaining_damage < 0) ? 1 : 0)) >> 1;
                    const int new_damage = static_cast<int>(veh.dmg_incurred) +
                                           half_damage;
                    veh.dmg_incurred = static_cast<uint8_t>(new_damage);
                }
            }

            const int affected_base_id = base_at(x, y);
            if (affected_base_id < 0) continue;
            Base &affected_base = Bases[affected_base_id];
            if (affected_base.faction_id_current == local_faction &&
                Bases[base_reference_for_news].faction_id_current != local_faction) {
                parse_says(0, affected_base.name_string, -1, -1);
                base_reference_for_news = affected_base_id;
                effects_are_visible = true;
            }
            if (offset_bytes < 0x24 && !is_objective(affected_base_id)) {
                base_kill(affected_base_id);
                continue;
            }

            const int old_population = affected_base.population_size;
            int new_population;
            if (offset_bytes < 0x64) {
                int value = old_population + 3;
                if (value < 0) value += 3;
                new_population = value >> 2;
            } else if (offset_bytes < 0xC4) {
                int value = old_population + 1;
                if (value < 0) ++value;
                new_population = value >> 1;
            } else {
                int value = (old_population + 1) * 3;
                if (value < 0) value += 3;
                new_population = value >> 2;
            }
            affected_base.population_size = static_cast<int8_t>(
                static_cast<uint8_t>(new_population));
        }

        if (effects_are_visible) {
            if (!ConsoleGlobal.focus(event_x, event_y, local_faction)) draw_map(1);
        }
        GameDataStateStorage.fields_.dust_cloud_duration_ = 10;
        popp(ScriptPopupFile, "BIGFATVOLCANO", 0, "volc_sm.pcx", 0);
        return;
    }

    if (event_kind == 15 || event_kind == 16) {
        const int base_x = base_x_snapshot;
        const int base_y = base_y_snapshot;
        switch (event_kind) {
        case 15: {
            if (IsMultiplayerPBEM ||
                GameDataStateStorage.fields_.turn_current_num_ < 75) break;
            if (PlayersData[faction_id].current_num_bases < 8 ||
                (GameDataStateStorage.fields_.game_state_ & 0x10u) != 0 ||
                near_landmark(base_x, base_y)) break;
            const int initial_local_faction = ConsoleGlobal.faction_id_;
            const unsigned char local_bit = static_cast<unsigned char>(
                1u << initial_local_faction);
            const bool local_map_visible =
                (PlayersData[initial_local_faction].flags & 0x200u) != 0 ||
                (map_tiles()[base_y * static_cast<int>(MapLongitude) +
                             (base_x >> 1)].visibility & local_bit) != 0;

            const int preflight_longitude = MapLongitudeBounds;
            const bool preflight_flat = (MapIsFlat & 1) != 0;
            int outer_water_count = 0;
            bool suitable_crater_site = true;
            for (int radius_index = 0; radius_index < 49; ++radius_index) {
                int x = base_x + RadiusOffsetX[radius_index];
                const int y = base_y + RadiusOffsetY[radius_index];
                if (!preflight_flat) {
                    if (x < 0) x += preflight_longitude;
                    else if (x >= preflight_longitude) x -= preflight_longitude;
                }
                if (y < 0 || y >= MapLatitudeBounds || x < 0 ||
                    x >= preflight_longitude) continue;
                const Map &tile = map_tiles()[y * static_cast<int>(MapLongitude) +
                                              (x >> 1)];
                if ((tile.climate & 0xE0) >= 0x60) continue;
                if (radius_index < 25) {
                    suitable_crater_site = false;
                    break;
                }
                ++outer_water_count;
                if (outer_water_count > 5) {
                    suitable_crater_site = false;
                    break;
                }
            }
            if (!suitable_crater_site ||
                PlayersData[faction_id].current_num_bases < 7)
                break;

            int event_seen = local_map_visible ? 1 : 0;
            for (int scan_index = 0; scan_index < 49; ++scan_index) {
                int x = base_x + RadiusOffsetX[scan_index];
                const int y = base_y + RadiusOffsetY[scan_index];
                if ((MapIsFlat & 1) == 0) {
                    const int longitude = MapLongitudeBounds;
                    if (x < 0) x += longitude;
                    else if (x >= longitude) x -= longitude;
                }
                if (y < 0 || y >= MapLatitudeBounds || x < 0 ||
                    x >= MapLongitudeBounds) continue;
                bit_set(x, y, 0x81E8887C, 0);
                synch_bit(x, y, ConsoleGlobal.faction_id_);
                const int affected_base_id = base_at(x, y);
                if (affected_base_id >= 0) {
                    if (Bases[affected_base_id].faction_id_current ==
                            ConsoleGlobal.faction_id_ &&
                        Bases[selected_base_id].faction_id_current !=
                            ConsoleGlobal.faction_id_) {
                        event_seen = 1;
                        parse_says(0, Bases[affected_base_id].name_string, -1, -1);
                    }
                    base_kill(affected_base_id);
                }
                for (int vehicle_id = GameDataStateStorage.fields_.vehicle_current_count_ - 1;
                     vehicle_id >= 0; --vehicle_id) {
                    if (Vehs[vehicle_id].x != x || Vehs[vehicle_id].y != y)
                        continue;
                    if (Vehs[vehicle_id].faction_id == ConsoleGlobal.faction_id_)
                        event_seen = 1;
                    veh_kill(vehicle_id);
                }
            }
            world_crater(base_x, base_y);
            world_climate();
            if (!event_seen || ConsoleGlobal.focus(base_x, base_y,
                                                   ConsoleGlobal.faction_id_) == 0)
                draw_map(1);
            GameDataStateStorage.fields_.dust_cloud_duration_ = 10;
            popp(ScriptPopupFile, "EATTHIS", 0, "astp_sm.pcx", 0);
            return;
        }

        case 16: {
            if (!has_tech(Facility[FAC_BIOLOGY_LAB].preq_tech, faction_id)) break;
            const bool has_biology_lab = has_fac_built(FAC_BIOLOGY_LAB,
                                                       selected_base_id);
            if (!has_biology_lab) {
                const int required_ranking =
                    (GameDataStateStorage.fields_.turn_current_num_ >= 150) ? 5 : 4;
                if (PlayersData[faction_id].ranking < required_ranking) break;
            }
            bool processed_first_tile = false;
            bool changed_any_tile = false;
            for (int radius_index = 0; radius_index < 21; ++radius_index) {
                int x = base_x + RadiusOffsetX[radius_index];
                const int y = base_y + RadiusOffsetY[radius_index];
                if ((MapIsFlat & 1) == 0) {
                    const int longitude = MapLongitudeBounds;
                    if (x < 0) x += longitude;
                    else if (x >= longitude) x -= longitude;
                }
                if (y < 0 || y >= MapLatitudeBounds || x < 0 ||
                    x >= MapLongitudeBounds) continue;
                Map &tile = map_tiles()[y * static_cast<int>(MapLongitude) +
                                        (x >> 1)];
                if ((tile.climate & 0xE0) < 0x60 ||
                    (tile.bit & 0x208000u) == 0) continue;
                const unsigned char faction_bit = static_cast<unsigned char>(
                    1u << faction_id);
                if ((PlayersData[faction_id].flags & 0x200u) == 0 &&
                    (tile.visibility & faction_bit) == 0) continue;
                const int territory = whose_territory(faction_id, x, y, 0, 0);
                if (territory >= 0 && territory != faction_id) continue;

                if (!has_biology_lab && !processed_first_tile) {
                    ConsoleGlobal.focus(base_x, base_y,
                                        ConsoleGlobal.faction_id_);
                    if ((GetAsyncKeyState(VK_SHIFT) & 0xFF00) == 0 &&
                        !IsMultiplayerNet) clock_wait(200);
                }
                bit_set(x, y, 0x288000, 0);
                synch_bit(x, y, faction_id);
                // Native repeats seven clears at the same selected word;
                // it does not advance through the visibility-word array.
                for (int clear_pass = 0; clear_pass < 7; ++clear_pass) {
                    uint32_t *cache_word;
                    switch (ConsoleGlobal.faction_id_) {
                    case 0: cache_word = &tile.bit2; break;
                    case 1: cache_word = &tile.bit_visible[0]; break;
                    case 2: cache_word = &tile.bit_visible[1]; break;
                    case 3: cache_word = &tile.bit_visible[2]; break;
                    case 4: cache_word = &tile.bit_visible[3]; break;
                    case 5: cache_word = &tile.bit_visible[4]; break;
                    case 6: cache_word = &tile.bit_visible[5]; break;
                    default: cache_word = &tile.bit_visible[6]; break;
                    }
                    if (*cache_word & 0x288000u) *cache_word &= ~0x288000u;
                }
                draw_tile(x, y, 2);
                if ((GetAsyncKeyState(VK_SHIFT) & 0xFF00) == 0 &&
                    !IsMultiplayerNet) clock_wait(50);
                changed_any_tile = true;
                processed_first_tile = true;
            }
            if (!changed_any_tile) break;
            if (faction_id != 0) {
                Bases[selected_base_id].event |= 0x200u;
                Bases[selected_base_id].random_event_turns = 10;
            }
            if (faction_id == ConsoleGlobal.faction_id_) {
                if (ConsoleGlobal.focus(Bases[selected_base_id].x,
                                        Bases[selected_base_id].y, faction_id) == 0)
                    draw_radius(Bases[selected_base_id].x,
                                Bases[selected_base_id].y, 2, 2);
            } else if (!IsMultiplayerPBEM) {
                break;
            }
            if (has_biology_lab)
                POP2("BLIGHT1", "markbm_sm.pcx", base_roll);
            else
                POP2("BLIGHT", "markbm_sm.pcx", base_roll);
            return;
        }
        }
        return;
    }

    if (event_kind == 17 || event_kind == 18) {
        switch (event_kind) {
        case 17: {
            if (!has_tech(Facility[FAC_ENERGY_BANK].preq_tech, faction_id)) break;
            const bool has_energy_bank =
                has_fac(FAC_ENERGY_BANK, selected_base_id, 0);
            if (!has_energy_bank) {
                const int minimum_ranking =
                    (GameDataStateStorage.fields_.turn_current_num_ >= 150) ? 5 : 4;
                if (PlayersData[faction_id].ranking <
                    minimum_ranking) break;
            }
            bool event_found = false;
            for (int radius_index = 0; radius_index < 21; ++radius_index) {
                int x = base_x_snapshot + RadiusOffsetX[radius_index];
                const int y = base_y_snapshot + RadiusOffsetY[radius_index];
                if ((MapIsFlat & 1) == 0) {
                    if (x < 0) x += MapLongitudeBounds;
                    else if (x >= MapLongitudeBounds) x -= MapLongitudeBounds;
                }
                if (y < 0 || y >= MapLatitudeBounds || x < 0 ||
                    x >= MapLongitudeBounds) continue;
                Map &tile = map_tiles()[MapLongitude * y + (x >> 1)];
                if ((tile.climate & 0xE0) < 0x60 || (tile.bit & 0x10u) == 0)
                    continue;
                const unsigned char faction_bit =
                    static_cast<unsigned char>(1u << faction_id);
                if ((PlayersData[faction_id].flags & 0x200u) == 0 &&
                    (tile.visibility & faction_bit) == 0) continue;
                const int territory = whose_territory(faction_id, x, y, 0, 0);
                if (territory >= 0 && territory != faction_id) continue;
                if (has_energy_bank) {
                    event_found = true;
                    continue;
                }
                if (faction_id == ConsoleGlobal.faction_id_ && !event_found) {
                    ConsoleGlobal.focus(base_x_snapshot, base_y_snapshot,
                                        ConsoleGlobal.faction_id_);
                    if ((GetAsyncKeyState(VK_SHIFT) & 0xFF00) == 0 &&
                        !IsMultiplayerNet) clock_wait(200);
                }
                bit_set(x, y, 0x10, 0);
                synch_bit(x, y, faction_id);
                for (int visible_faction = 0; visible_faction < 7;
                     ++visible_faction) {
                    if (tile.bit_visible[visible_faction] & 0x10u)
                        tile.bit_visible[visible_faction] &= ~0x10u;
                }
                draw_tile(x, y, 2);
                if ((GetAsyncKeyState(VK_SHIFT) & 0xFF00) == 0 &&
                    !IsMultiplayerNet) clock_wait(50);
                event_found = true;
            }
            if (!event_found) break;
            if (has_energy_bank) PlayersData[faction_id].energy_reserves += 50;
            if (faction_id == ConsoleGlobal.faction_id_) {
                const int base_x = Bases[selected_base_id].x;
                const int base_y = Bases[selected_base_id].y;
                if (ConsoleGlobal.focus(base_x, base_y, faction_id) == 0)
                    draw_radius(base_x, base_y, 2, 2);
            }
            if (faction_id != ConsoleGlobal.faction_id_ && !IsMultiplayerPBEM)
                break;
            if (has_energy_bank)
                POP2("SURGE1", "markbm_sm.pcx", base_roll);
            else
                POP2("SURGE", "markbm_sm.pcx", base_roll);
            return;
        }

        case 18: {
            if (!has_tech(Facility[FAC_NETWORK_NODE].preq_tech, faction_id)) break;
            const int research_rate = tech_rate(faction_id);
            const int minimum_progress = research_rate >= 0
                ? research_rate / 4
                : -static_cast<int>((0u - static_cast<unsigned int>(research_rate)) / 4u);
            if (PlayersData[faction_id].tech_accumulated < minimum_progress) break;
            if (has_fac(FAC_NETWORK_NODE, selected_base_id, 0)) {
                PlayersData[faction_id].tech_accumulated = research_rate;
                if (faction_id == ConsoleGlobal.faction_id_)
                    ConsoleGlobal.focus(Bases[selected_base_id].x,
                                        Bases[selected_base_id].y, faction_id);
                if (faction_id != ConsoleGlobal.faction_id_ && !IsMultiplayerPBEM)
                    break;
                POP2("NETBONUS", "markbm_sm.pcx", base_roll);
                return;
            }
            const int minimum_ranking =
                (GameDataStateStorage.fields_.turn_current_num_ >= 150) ? 5 : 4;
            if (PlayersData[faction_id].ranking <
                minimum_ranking) break;
            PlayersData[faction_id].tech_accumulated = 0;
            PlayersData[faction_id].net_random_event = 0;
            if (faction_id == ConsoleGlobal.faction_id_)
                ConsoleGlobal.focus(Bases[selected_base_id].x,
                                    Bases[selected_base_id].y, faction_id);
            if (faction_id != ConsoleGlobal.faction_id_ && !IsMultiplayerPBEM)
                break;
            POP2("NETCRASH", "netcr_sm.pcx", base_roll);
            return;
        }
        }
        return;
    }

    if (event_kind >= 19 && event_kind <= 21) {
        switch (event_kind) {
        case 19: {
            if (!has_tech(Facility[FAC_CHILDREN_CRECHE].preq_tech, faction_id))
                break;
            const bool has_creche = has_fac_built(FAC_CHILDREN_CRECHE,
                                                   selected_base_id) != 0;
            if (has_creche) {
                int population_increments = 0;
                if (Bases[selected_base_id].nutrient_surplus <= 0) break;
                for (;;) {
                    ++population_increments;
                    if (population_increments >= 3) break;
                    ++Bases[selected_base_id].population_size;
                    set_base(selected_base_id);
                    base_compute(1);
                    if (Bases[selected_base_id].nutrient_surplus <= 0) break;
                }
                if (population_increments == 0) break;
                if (faction_id == ConsoleGlobal.faction_id_) {
                    const int focus_x = Bases[selected_base_id].x;
                    const int focus_y = Bases[selected_base_id].y;
                    if (ConsoleGlobal.focus(focus_x, focus_y, faction_id) == 0)
                        draw_tile(focus_x, focus_y, 2);
                }
                if (faction_id != ConsoleGlobal.faction_id_ &&
                    IsMultiplayerPBEM == 0) break;
                if ((Players[faction_id].rule_flags & RFLAG_ALIEN) != 0)
                    POP2("CRECHE1", "al_pop_sm.pcx", selected_base_id);
                else
                    POP2("CRECHE1", "pop_sm.pcx", selected_base_id);
                return;
            }
            const int required_ranking = 4 +
                (GameDataStateStorage.fields_.turn_current_num_ >= 150 ? 1 : 0);
            if (PlayersData[faction_id].ranking < required_ranking) break;
            if (Bases[selected_base_id].assimilation_turns_left != 0) break;
            Bases[selected_base_id].assimilation_turns_left = 5;
            if (faction_id == ConsoleGlobal.faction_id_) {
                const int focus_x = Bases[selected_base_id].x;
                const int focus_y = Bases[selected_base_id].y;
                ConsoleGlobal.focus(focus_x, focus_y, faction_id);
            }
            if (faction_id != ConsoleGlobal.faction_id_ &&
                IsMultiplayerPBEM == 0) break;
            if ((Players[faction_id].rule_flags & RFLAG_ALIEN) != 0)
                POP2("CRECHE", "al_pop_sm.pcx", selected_base_id);
            else
                POP2("CRECHE", "pop_sm.pcx", selected_base_id);
            return;
        }

        case 20: {
            if (IsMultiplayerPBEM != 0 ||
                GameDataStateStorage.fields_.turn_current_num_ < 75) break;
            const int required_ranking = 4 +
                (GameDataStateStorage.fields_.turn_current_num_ >= 150 ? 1 : 0);
            if (PlayersData[faction_id].ranking < required_ranking) break;
            bool changed_for_local_view = false;
            for (int radius_index = 0; radius_index < 21; ++radius_index) {
                int x = base_x_snapshot + RadiusOffsetX[radius_index];
                const int y = base_y_snapshot + RadiusOffsetY[radius_index];
                if ((MapIsFlat & 1) == 0) {
                    if (x < 0) x += MapLongitudeBounds;
                    else if (x >= MapLongitudeBounds) x -= MapLongitudeBounds;
                }
                if (y < 0 || y >= MapLatitudeBounds || x < 0 ||
                    x >= MapLongitudeBounds) continue;
                const int tile_index = static_cast<int>(MapLongitude) * y + (x >> 1);
                Map &tile = map_tiles()[tile_index];
                // Keep both pinned climate tests, despite their contradiction
                // on an unchanged tile and their positions around territory.
                if ((tile.climate & 0xE0) >= 0x60) continue;
                if ((tile.bit & BIT_FARM) == 0) continue;
                if ((PlayersData[faction_id].flags & 0x200u) == 0 &&
                    (tile.visibility & static_cast<uint8_t>(1u << (faction_id & 31))) == 0)
                    continue;
                const int territory = whose_territory(faction_id, x, y, 0, 0);
                if (territory >= 0 && territory != faction_id) continue;
                if ((map_tiles()[tile_index].climate & 0xE0) < 0x60) continue;
                const int local_faction = ConsoleGlobal.faction_id_;
                if (faction_id == local_faction ||
                    Bases[selected_base_id].faction_id_current == local_faction ||
                    (Bases[selected_base_id].visibility &
                     static_cast<uint8_t>(1u << (local_faction & 31))) != 0) {
                    if (!changed_for_local_view) {
                        ConsoleGlobal.focus(base_x_snapshot, base_y_snapshot,
                                            local_faction);
                        const unsigned short shift_state = static_cast<unsigned short>(
                            GetAsyncKeyState(VK_SHIFT));
                        if ((shift_state & 0xFF00u) == 0 && IsMultiplayerNet == 0)
                            clock_wait(200);
                        changed_for_local_view = true;
                    }
                }
                bit_set(x, y, BIT_FARM, 0);
                synch_bit(x, y, faction_id);
                for (int clear_pass = 1; clear_pass < 8; ++clear_pass)
                    map_tiles()[tile_index].bit2 &= ~BIT_FARM;
                if (changed_for_local_view) {
                    draw_tile(x, y, 2);
                    const unsigned short shift_state = static_cast<unsigned short>(
                        GetAsyncKeyState(VK_SHIFT));
                    if ((shift_state & 0xFF00u) == 0 && IsMultiplayerNet == 0)
                        clock_wait(50);
                }
            }
            if (!changed_for_local_view) break;
            parse_says(0, Bases[selected_base_id].name_string, -1, -1);
            if (faction_id == ConsoleGlobal.faction_id_)
                POP2("KELPWIPE", 0, selected_base_id);
            else
                POP2("KELPWIPE", 0, -1);
            return;
        }

        case 21: {
            if (IsMultiplayerPBEM != 0 ||
                GameDataStateStorage.fields_.turn_current_num_ < 75) break;
            const int required_ranking = 4 +
                (GameDataStateStorage.fields_.turn_current_num_ >= 150 ? 1 : 0);
            if (PlayersData[faction_id].ranking < required_ranking) break;
            bool changed_for_local_view = false;
            for (int radius_index = 0; radius_index < 21; ++radius_index) {
                int x = base_x_snapshot + RadiusOffsetX[radius_index];
                const int y = base_y_snapshot + RadiusOffsetY[radius_index];
                if ((MapIsFlat & 1) == 0) {
                    if (x < 0) x += MapLongitudeBounds;
                    else if (x >= MapLongitudeBounds) x -= MapLongitudeBounds;
                }
                if (y < 0 || y >= MapLatitudeBounds || x < 0 ||
                    x >= MapLongitudeBounds) continue;
                const int tile_index = static_cast<int>(MapLongitude) * y + (x >> 1);
                Map &tile = map_tiles()[tile_index];
                // Preserve the original opposing climate predicates and
                // repeated same-word clearing in this case as well.
                if ((tile.climate & 0xE0) >= 0x60) continue;
                if ((tile.bit & BIT_MINE) == 0) continue;
                if ((PlayersData[faction_id].flags & 0x200u) == 0 &&
                    (tile.visibility & static_cast<uint8_t>(1u << (faction_id & 31))) == 0)
                    continue;
                const int territory = whose_territory(faction_id, x, y, 0, 0);
                if (territory >= 0 && territory != faction_id) continue;
                if ((map_tiles()[tile_index].climate & 0xE0) < 0x60) continue;
                const int local_faction = ConsoleGlobal.faction_id_;
                if (faction_id == local_faction ||
                    Bases[selected_base_id].faction_id_current == local_faction ||
                    (Bases[selected_base_id].visibility &
                     static_cast<uint8_t>(1u << (local_faction & 31))) != 0) {
                    if (!changed_for_local_view) {
                        ConsoleGlobal.focus(base_x_snapshot, base_y_snapshot,
                                            local_faction);
                        const unsigned short shift_state = static_cast<unsigned short>(
                            GetAsyncKeyState(VK_SHIFT));
                        if ((shift_state & 0xFF00u) == 0 && IsMultiplayerNet == 0)
                            clock_wait(200);
                        changed_for_local_view = true;
                    }
                }
                bit_set(x, y, BIT_MINE, 0);
                synch_bit(x, y, faction_id);
                for (int clear_pass = 1; clear_pass < 8; ++clear_pass)
                    map_tiles()[tile_index].bit2 &= ~BIT_MINE;
                if (changed_for_local_view) {
                    draw_tile(x, y, 2);
                    const unsigned short shift_state = static_cast<unsigned short>(
                        GetAsyncKeyState(VK_SHIFT));
                    if ((shift_state & 0xFF00u) == 0 && IsMultiplayerNet == 0)
                        clock_wait(50);
                }
            }
            if (!changed_for_local_view) break;
            parse_says(0, Bases[selected_base_id].name_string, -1, -1);
            const int popup_base_id = faction_id == ConsoleGlobal.faction_id_
                ? selected_base_id : -1;
            POP2("PLATFORMWIPE", 0, popup_base_id);
            return;
        }
        }
        return;
    }

}

static bool alien_local_vehicle_visible(int vehicle_id, int faction) {
    const Veh &vehicle = Vehs[vehicle_id];
    if (vehicle.faction_id == faction && (vehicle.flags & 0x440) != 0x440) {
        return true;
    }
    return (vehicle.visibility & (1u << faction)) != 0;
}

static bool alien_tile_vehicle_visible(int vehicle_id, int faction) {
    return alien_local_vehicle_visible(vehicle_id, faction);
}


// ORIGINAL: 0x00522180 ?alien_fauna@@YAXXZ 0x00522180-0x00522EFC
// RECOVERY_BINDING: v1 {"evidence": ["git:99d6e72bcd5ae3be3566222736528cb0dc57935c:src/unrecovered/00522180.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "recovery/tasks/game-logic-seed-owner-coverage.json; preserve original extents and historical seed lessons. Ordinary owned literals have pinned content provenance, not exact original data-allocation claims."], "reason": "Recover native-life spread and spawning through real Map/PlayerData/Base/Veh/UI owners. Preserve native-life versus difficulty operands and the complementary signed JL/JGE pair with one saved wrapped limit; the original reproduction tail is unreachable by that pair. Do not invent guards or entry edges. No matching or game acceptance claim."}
// TRIED: near-literal transcription of Ghidra (reliable here, no unaff_* registers, all callees are free functions except MapWin::focus and NetMsg::pop, whose `this` (g_007d3c3c, g_00805338) came from raw disasm since Ghidra drops it); the CONCAT31(...)&0xffffffdf idiom simplified to (uVar13|uVar14)& 0xffffffdf since uVar14's low byte is always 0. Diverges at insn #3 (xor vs mov), likely register-zeroing choice.
void __cdecl alien_fauna() {
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
    for (int i = 0; i < MapArea / (GameDataStateStorage.fields_.turn_current_num_ / 4 + 32); ++i) {
        x = bounded_crt_random(MapLongitudeBounds);
        y = bounded_crt_random(MapLatitudeBounds);
        x = (y & 1) - (x & 1) + x;
        if (!on_map(x, y)) continue;

        const BOOL ocean = is_ocean(x, y);
        const uint32_t bit = map_loc(x, y)->bit;
        if (ocean) {
            if (!(bit & BIT_FARM)) continue;
        } else if (!(bit & BIT_FOREST)) {
            continue;
        }

        int best_score = 0;
        int best_x = -1;
        int best_y;  // original leaves this uninitialised
        for (int dir = 0; dir < 8; ++dir) {
            x2 = wrap_map_x(x + VehMoveDeltaX[dir]);
            y2 = y + VehMoveDeltaY[dir];
            if (!on_map(x2, y2)) continue;
            Map *cand = map_loc(x2, y2);
            const int climate = cand->climate;
            if (((climate & 0xE0) < ALT_BIT_SHORE_LINE) != ocean) continue;
            const int alt = climate >> 5;
            if (alt < 2) continue;
            if (alt > 4 && !(climate & (RAINFALL_RAINY | RAINFALL_MOIST))) continue;
            if ((cand->bit2 & 0x80000002) == 2) continue;
            if ((cand->bit2 & 0x80000040) == 0x40) continue;
            const int score = (3 - ((climate >> 3) & 3)) * 4 - (cand->val3 >> 6);
            if ((cand->val3 & 0xC0) > 0x40) continue;
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
            x = bounded_crt_random(MapLongitudeBounds);
            y = bounded_crt_random(MapLatitudeBounds);
            x = (y & 1) - (x & 1) + x;  // no on_map check in the original
            region = map_loc(x, y)->region;
            if (++tries >= 1000) break;
        } while (!(((map_loc(x, y)->bit & BIT_FUNGUS) &&
                    (map_loc(x, y)->climate & 0xE0) >= ALT_BIT_OCEAN_SHELF) ||
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
                        sleep(veh_init((rand() % 5 == 0 && ExpansionEnabled) ? 15 : 8, 0, x, y));
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
        x = bounded_crt_random(MapLongitudeBounds);
        y = bounded_crt_random(MapLatitudeBounds);
        x = (y & 1) - (x & 1) + x;  // no on_map check in the original
        Map *tile = map_loc(x, y);
        if ((tile->climate & 0xE0) < ALT_BIT_SHORE_LINE) continue;
        if (!(tile->bit & BIT_FUNGUS)) continue;
        if ((tile->climate & 0xE0) < ALT_BIT_OCEAN_SHELF) continue;
        region = tile->region;
        if (bad_reg(region)) continue;
        if (base_who(x, y) >= 0) continue;

        base_id = base_find(x, y, -1, region, -1, -1);
        const int owner = veh_who(x, y);
        if (owner > 0 && (base_id < 0 || BaseFindDist > 2)) {
            // Spawn beside an unaware faction's unit, out of its sight.
            for (int i = 0; i < 16; ++i) {
                x2 = wrap_map_x(x + RadiusOffsetX[9 + i]);
                y2 = y + RadiusOffsetY[9 + i];
                if (!on_map(x2, y2)) continue;
                if (is_ocean(x2, y2)) continue;
                if (PlayersData[owner].flags & PFLAG_MAP_REVEALED) continue;
                if (map_loc(x2, y2)->visibility & (1 << owner)) continue;
                if (anything_at(x2, y2) >= 0) continue;
                if (zoc_any(x2, y2, 0)) continue;
                const int veh_id = veh_init(
                    (rand() % 5 == 0 && ExpansionEnabled) ? 15 : 8, 0, x2, y2);
                if (veh_id >= 0 &&
                    alien_tile_vehicle_visible(veh_id, ConsoleGlobal.faction_id_)) {
                    draw_tile(x2, y2, 2);
                }
                break;
            }
        }
        if (owner >= 0) continue;

        if (BaseFindDist > 1 && BaseFindDist < 6 &&
            (map_loc(x, y)->bit & BIT_FUNGUS) &&
            (map_loc(x, y)->climate & 0xE0) >= ALT_BIT_OCEAN_SHELF &&
            !goody_at(x, y)) {
            const int veh_id = veh_init(
                (rand() % 5 == 0 && ExpansionEnabled) ? 15 : 8, 0, x, y);
            // Original bug: redraws the stale (x2, y2), not the spawn tile.
            if (veh_id >= 0 &&
                alien_tile_vehicle_visible(veh_id, ConsoleGlobal.faction_id_)) {
                draw_tile(x2, y2, 2);
            }
        }
    } while (++attempt < MapNativeLifeForms * 2 + 2);

    // Unreachable: the loop only exits with attempt at the limit.
    if (attempt >= MapNativeLifeForms * 2 + 2) return;

    int count;
    if (!very_early) {
        if (GameDataStateStorage.fields_.game_tech_achieved_[TECH_CENTPSI] ||
            GameDataStateStorage.fields_.game_tech_achieved_[TECH_QUANTUM]) {
            veh_init(10, 0, x, y);
        }
        // Unguarded Bases[base_id] read.
        count = (Bases[base_id].population_size *
                 ((GameDataStateStorage.fields_.game_state_ & STATE_PERIHELION_ACTIVE) ? 2 : 1) *
                 (MapNativeLifeForms + 1) + 3) / 4;
    } else {
        count = 1;
    }

    int direction = 21;
    int notice_sent = 0;
    for (int n = 0; n < count; ++n) {
        do {
            direction = (direction + 1) % 21;
            x2 = wrap_map_x(x + RadiusOffsetX[direction]);
            y2 = y + RadiusOffsetY[direction];
        } while (!on_map(x2, y2) ||
                 !(map_loc(x2, y2)->bit & BIT_FUNGUS) ||
                 (map_loc(x2, y2)->climate & 0xE0) < ALT_BIT_OCEAN_SHELF ||
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
            ((PlayersData[ConsoleGlobal.faction_id_].flags & PFLAG_MAP_REVEALED) ||
             (map_loc(x2, y2)->visibility & (1 << ConsoleGlobal.faction_id_))) &&
            alien_tile_vehicle_visible(veh_id, ConsoleGlobal.faction_id_)) {
            notice_sent = 1;
            parse_says(0, Bases[base_id].name_string, -1, -1);
            NetMsgPrimary.pop("XMINDWORMS", -5000, 0,
                              spore_launcher ? "sporlnch_sm.pcx" : "mindworm_sm.pcx");
        }
    }
}

// ORIGINAL: 0x00522F00 ?do_fungal_towers@@YAXXZ 0x00522F00-0x005230DD
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

// ORIGINAL: 0x005230E0 ?interlude@@YAXHPADHH@Z 0x005230E0-0x00523B4E;0x0065D4F0-0x0065D6C8
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/005230e0.cpp: original identity, complete spans and retained seed history", "source:src/interlude.h: genuine GraphicWin base and embedded member lifetime", "source:recovery/tasks/interlude-lifetime-owner-closure.json: bounded coordinator authorization"], "reason": "Original5230E0 owns exactly one real Interlude local and its inline member destruction, not additional fabricated Buffer/StringStruct/RunResult objects; third/fourth arguments and saved-turn calculations follow original behavior."}
// TRIED: heavily SEH-framed function (local Interlude+Buffer+StringStruct objects, __CxxFrameHandler unwind funclets); Ghidra's own pseudocode has unaff_EBX/ESI/DI and garbled puStackY_* return- address literals from the frame, so this is a raw-disasm transcription instead, using RAII (Interlude popup; Buffer buf; StringStruct strObj;) for the destructor cascade rather than hand-written SEH. A `sub_4066c0` thiscall on an untyped 8-byte local is modeled as a throwaway `InterludeRunResult` class. The two faction-scan loops (best commlink / best base) and the StringStruct remove_all/close cleanup are best-effort approximations, not verified bit-for-bit - the two indirect calls through g_0066931c/g_00669344 (CharUpperA/CharLowerA IAT slots) are called directly instead of through the pointer.
void __cdecl interlude(int id, char *text, int mode, int ending) {
    Interlude popup;
    if (IsMultiplayerNet || DiplomacyActive) return;
    const unsigned int shift = static_cast<unsigned int>(id) & 31u;
    if (GameDataStateStorage.fields_.game_interlude_flags_ & (1u << shift)) return;
    int faction = ConsoleGlobal.faction_id_;
    const bool alien = (Players[faction].rule_flags & RFLAG_ALIEN) != 0;
    if (id < 32 ? alien : !alien) return;
    if ((GameDataStateStorage.fields_.game_state_ & 0x10) || (GameDataStateStorage.fields_.game_rules_ & 0x200000)) return;
    StringTemp[0] = 0;
    strcat(StringTemp, Players[faction].filename);
    CharUpperA(StringTemp);
    if (strcmp(StringTemp, "FUNGBOY") == 0) return;
    if (id < 32) GameDataStateStorage.fields_.game_interlude_flags_ |= 1u << shift;
    if (id == 6) GameDataStateStorage.fields_.game_interlude_flags_ |= 0x12;
    if (GameDataStateStorage.fields_.game_preferences_ & 0x40000) return;

    const int saved_turn = GameDataStateStorage.fields_.turn_current_num_;
    parse_num(0, game_year(saved_turn));
    parse_num(1, game_year(saved_turn));
    parse_num(2, game_year(saved_turn));
    StringTemp[0] = 0;
    if (mode != 0) {
        strcat(StringTemp, mode == 2 ? "EPILOGUE" : mode == 3 ? "EPILOGUE2"
                           : mode == 4 ? "EPILOGUE3" : "INTERLUDE");
        X_pop(InterludeTextFile, StringTemp, nullptr);
    }
    parse_num(1, static_cast<int>(game_year(saved_turn) - game_year(0)));
    if (id == 20) {
        parse_num(1, static_cast<int>(game_year(saved_turn) - game_year(0)) / 100);
    }

    faction = ConsoleGlobal.faction_id_;
    PluralityDefault = 0;
    GenderDefault = Players[faction].is_leader_female;
    parse_says(0, Players[faction].title_leader, -1, -1);
    faction = ConsoleGlobal.faction_id_;
    PluralityDefault = 0;
    GenderDefault = Players[faction].is_leader_female;
    parse_says(1, Players[faction].name_leader, -1, -1);
    parse_says(8, Players[ConsoleGlobal.faction_id_].assistant_name, -1, -1);
    parse_says(9, Players[ConsoleGlobal.faction_id_].scientist_name, -1, -1);
    faction = ConsoleGlobal.faction_id_;
    StringTemp[0] = 0;
    PluralityDefault = 0;
    GenderDefault = Players[faction].is_leader_female;
    strcat(StringTemp, Players[faction].name_leader);
    CharLowerA(StringTemp);
    parse_says(3, StringTemp, -1, -1);

    int best_value = 0;
    // The original writes this local only when a positive best-base score
    // is found; the later tie-selection loop reads the same local.
    int best_base;
    for (int i = 0; i < GameDataStateStorage.fields_.base_current_count_; ++i) {
        if (Bases[i].faction_id_current == ConsoleGlobal.faction_id_) {
            int value = Bases[i].population_size;
            int offset, mask;
            bitmask(1, &offset, &mask);
            if (Bases[i].facilities_built[offset] & mask) value += 999;
            if (value > best_value) {
                parse_says(2, Bases[i].name_string, -1, -1);
                best_value = value;
                best_base = i;
            }
        }
    }
    if (ending == 0) {
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
    StringTemp[0] = 0;
    strcat(StringTemp, Players[ConsoleGlobal.faction_id_].search_key);
    CharUpperA(StringTemp);
    if (strcmp(StringTemp, "CARETAKE") == 0) {
        parse_says(6, label_get(1128), -1, -1);
        parse_says(7, label_get(1129), -1, -1);
    } else {
        parse_says(6, label_get(1127), -1, -1);
        parse_says(7, label_get(1130), -1, -1);
    }
    if (id < 32) {
        if (Players[ConsoleGlobal.faction_id_].rule_flags & RFLAG_ALIEN) return;
    } else {
        id -= 32;
        if (id == 3) GameDataStateStorage.fields_.game_interlude_flags_ |= 0x10;
    }
    GameDataStateStorage.fields_.game_interlude_flags_ |= 1u << (static_cast<unsigned int>(id) & 31u);
    StringTemp[0] = 0;
    strcat(StringTemp, "INTERLUDE");
    char number[20];
    _itoa(id, number, 10);
    strcat(StringTemp, number);
    popup.exec(StringTemp);
}

// ORIGINAL: 0x00523B50 ?ascent@@YAXXZ 0x00523B50-0x00523C58 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:pact-ascent-owner", "pinned:0x00523B50 original complete span; Player59C fields and real NetMsgPrimary/ConsoleGlobal owners.", "pinned:68E62C PACTTREATY, ASCENT and asctran_sm.pcx text inputs; source literals, never original-image calls.", "pinned:525F08..52600E same turn_upkeep ascent path; plural0, leader gender and console-faction reload after parser; bit2000 unnamed, not PFLAG_MAP_REVEALED200.", "historical seed:src/recovered/units/00523b50.cpp complete mapping/findings retained.", "recovery/tests/test_pact_ascent.py actual-source callback/damage controls, not native layout or runtime acceptance."], "reason": "Recover the complete announcement/turn-flow function using real source-owned players, console, parser state and callbacks. Retain original identity/spans and seed findings; live faction reloads and grammar come from full pinned instructions. No byte or game acceptance claim."}
void __cdecl ascent() {
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
        static_cast<int>(NetDaemonNet.channels_[1].trailing_words_[8]);
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
Purpose: Wrap an x coordinate the way reset_territory() wraps it.
Original Offset: n/a
Return Value: Wrapped x
Status: Complete

xrange() at 0048BEE0 cannot stand in, for one bit. It tests the whole of
MapIsFlat; reset_territory reads the low BYTE and tests bit zero
(`mov cl, byte ptr [94988Ch] / test cl, 1`). The two agree on the 0 and 1 the
game stores there and disagree on everything else, and the difference is in the
original rather than in the transcription. world_site() at 005C4FD0 carries the
same distinction as map.cpp's site_xrange().
*/
__forceinline static int territory_xrange(int x) {
    if (!(MapIsFlat & 1)) {
        if (x < 0) {
            x += MapLongitudeBounds;
        } else if (x >= MapLongitudeBounds) {
            x -= MapLongitudeBounds;
        }
    }
    return x;
}

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
//   the image's 349 exactly. (5) `territory_xrange` again, rewritten
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
            int region = tile->region;
            for (int faction_id = 1; faction_id < MaxPlayerNum; faction_id++) {
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
                    int x_adj = territory_xrange(x + VehMoveDeltaX[i]);
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
            // if/else, NOT `site = 0` then an inverted guard. The image
            // falls THROUGH to the zero and jumps to the val2 read
            // (`test ah, 0x10 / je 0x0052401F`), which is what the positive
            // test produces; the guard-clause form inverts every branch in
            // this block.
            int site;
            if (tile->bit & BIT_BASE_RADIUS) {
                site = 0;
            } else {
                site = tile->val2 >> 4;
                if (!site) {
                    site = world_site(x, y, false);
                    site_set(x, y, site);
                }
            }
            // `Continents[region]` indexed at each site: the image keeps
            // `region * 28` in EDI as an index (`[edi + 0x9AA738]`). Earlier
            // a `Continent *` beat this; with the loop/BaseFindDist fixes the
            // direct index measures better (0.868 -> 0.974).
            int region_best = (int)Continents[region].unk_3;
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


void __cdecl climactic_changes() {
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
// symbol    ?on_key@Widget@@QAEHHH@Z
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
class Slot058 { public: int slot058(); };
typedef int (Slot058::*Slot058Fn)();

class Widget {
 public:
    int on_key(int a1, int a2);
};

int Widget::on_key(int, int a2) {
    // 0xd, 0x1b, 0x20 are VK_RETURN, VK_ESCAPE and VK_SPACE.
    if (a2 == 0xd || a2 == 0x1b || a2 == 0x20) {
        (reinterpret_cast<Slot058 *>(this)->*(*reinterpret_cast<Slot058Fn *>(
            reinterpret_cast<char *>(*reinterpret_cast<void **>(this))
            + 0xe8)))();
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

// Sprite::extract's RECT form, expanded at both Demo::init call sites.
static inline int demo_extract(Sprite *sprite, Buffer *buffer, int transparent,
                               RECT *region) {
    return sprite->extract(buffer, transparent, region->left, region->top,
                           region->right - region->left,
                           region->bottom - region->top, 0);
}

// ORIGINAL: 0x00524460 Demo__init 0x00524460-0x005247A6;0x0065D6C8-0x0065D6DD
// symbol    ?init@Demo@@QAEHXZ
// kind      game
int Demo::init() {
    Buffer local_buffer;

    const int large_layout = WinScreenWidth != 0x320;
    GraphicWin::init(0, 0, WinScreenWidth, WinScreenHeight, 0, 0, 0, 0, 0);
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
                 WinFillColour, &splash_rects_[0]);
    demo_extract(&splash_sprites_[1], &local_buffer, WinFillColour,
                 &splash_rects_[0]);
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

namespace {
inline void clear_multiplayer_endgame_state() {
    NetDaemonNet.hang_up();
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
    NetDaemonNet.network_message_count() = 0;
    NetDaemonNet.channels_[1].trailing_words_[8] = 0;
    NetDaemonNet.channels_[1].trailing_words_[7] = 0;
    NetDaemonNet.primary_state_.trailing_words_[1] = 0;
    NetDaemonNet.primary_state_.trailing_words_[2] = 0;
    NetDaemonNet.channels_[1].trailing_words_[6] = 0xFF;
    NetDaemonNet.channels_[0].payload_.flags = 0;
    for (int index = 0; index < 8; ++index) {
        NetDaemonNet.channels_[0].trailing_words_[index + 1] = 0;
    }
    NetDaemonNet.locks_.clear();
    NetDaemonNet.AlphaNet::close();
    IsMultiplayerNet = 0;
}
}  // namespace


namespace {
enum EndgameReasonValue {
    EndgameReason1 = 1, EndgameReason2, EndgameReason3, EndgameReason4,
    EndgameReason5, EndgameReason6, EndgameReason7, EndgameReason8,
    EndgameReason9, EndgameReason10, EndgameReason11, EndgameReason12,
    EndgameReason13, EndgameReason14, EndgameReason15, EndgameReason16,
    EndgameReason17, EndgameReason18
};
const uint32_t EndgameRecorded = STATE_UNK_1;
const uint32_t EndgamePresented = STATE_UNK_8;
const uint32_t EndgameNetResults = STATE_IS_SCENARIO;
const uint32_t EndgameScoreMode = STATE_SCN_VICT_HIGHEST_AC_SCORE_WINS;
const uint32_t PlayerEndgameFlagA = 0x00020000u;
const uint32_t PlayerEndgameFlagB = 0x00040000u;
const uint32_t PlayerGeneratorWin = 0x00100000u;
const uint32_t PlayerConquestWin = 0x00040000u;
const uint32_t PlayerConquestLoss = 0x00020000u;

void parse_player_identity(int faction_id) {
    Player &player = Players[faction_id];
    PluralityDefault = 0;
    GenderDefault = player.is_leader_female;
    parse_says(0, player.title_leader, -1, -1);
    GenderDefault = player.is_leader_female;
    PluralityDefault = 0;
    parse_says(1, player.name_leader, -1, -1);
    GenderDefault = player.noun_gender;
    PluralityDefault = player.is_noun_plural;
    parse_says(2, player.noun_faction, -1, -1);
}

}

// ORIGINAL: 0x00524870 ?end_of_game@@YAHH@Z 0x00524870-0x0052581F
// RECOVERY_BINDING: v1 {"evidence": ["git:990398817a2f8abd81f8ecd21a1c87f88712f379:src/unrecovered/00524870.cpp: retained complete native extent and seed findings", "src/game.h: game state, mission years, result flags and canonical end_of_game declaration", "src/base.h and src/faction.h: objective, faction, ranking and project owners", "src/netdaemon.h: original three-entry lock-state cleanup through typed members", "recovery/tasks/endgame-owner-oct5.json", "recovery/tests/test_end_of_game_actual_source.py"], "reason": "Recover complete endgame decisions, result dialogs, media, scores and multiplayer teardown through genuine source owners. Preserve original defects and branch ordering, including repeated cleanup in the separate net_control_turn caller. Historical spans/findings remain obligations, not byte or runtime acceptance."}
// TRIED: mechanical transcription of the Ghidra decompilation (which already resolved the three provided jump tables into real switch/case), with every FUN_ callee re-derived from the disassembly's own `; ?mangled@Class` comments rather than guessed - all resolved to functions the scaffold already declares (X_pop, popp, interlude, num_objectives, compute_score, mon_winning_unify, report_score, hall_of_fame, quayle, show_credits, show_replay, has_fac, say_year, amovie_project, parse_num, parse_says) plus three fixed- receiver member calls confirmed from `mov ecx, ADDR; call` (Lock::clear at 0x93e230, AlphaNet::close and NetDaemon::hang_up both at 0x93cd90). Compiles and preserves every branch and both nested nested switches; did not chase the prologue-class mismatch (#0, push-vs-sub) given the size.
int __cdecl end_of_game(int check_victory) {
    char year_text[80];
    int score_items[9];
    int score;
    int selected_faction = -1;
    // stand-in: the image calls the out-of-line has_fac COMDAT (0x00421670)
    // here; the header inline would otherwise be expanded.
    BOOL (__cdecl *const has_facility)(int, int, int) = has_fac;

    if ((GameDataStateStorage.fields_.game_state_ & EndgameRecorded) != 0 &&
        (GameDataStateStorage.fields_.game_state_ & EndgamePresented) != 0) {
        return 0;
    }

    if ((GameDataStateStorage.fields_.game_state_ & EndgameRecorded) == 0 &&
        check_victory != 0) {
        if (static_cast<int>(GameDataStateStorage.fields_.mission_year_current_) >=
            static_cast<int>(ScenarioSave.ending_mission_year)) {
            GameDataStateStorage.fields_.game_state_ |= EndgameRecorded;
            EndgameExitReason = EndgameReason8;
        }

        for (int faction_id = 1; faction_id < 8; ++faction_id) {
            const unsigned int faction_bit = 1u << faction_id;
            if ((static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_ >> 8) &
                 faction_bit) == 0) {
                continue;
            }

            int generator_count = 0;
            if ((Players[faction_id].rule_flags & RFLAG_ALIEN) != 0) {
                for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; ++base_id) {
                    if (static_cast<unsigned char>(Bases[base_id].faction_id_current) == faction_id &&
                        has_facility(0x28, base_id, 0) &&
                        static_cast<int>(Bases[base_id].population_size) >=
                            Rules.size_base_subspace_gen) {
                        ++generator_count;
                    }
                }
            }
            if (generator_count >= Rules.subspace_generators_needed) {
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
            } else if (((faction_bit & static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_)) != 0 ||
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
            static_cast<int>(GameDataStateStorage.fields_.mission_year_current_) ==
                static_cast<int>(ScenarioSave.ending_mission_year) - 20) {
            parse_num(0, static_cast<int>(ScenarioSave.ending_mission_year));
            StringTemp[0] = 0;
            _itoa(static_cast<int>(ScenarioSave.ending_mission_year), year_text, 10);
            strcat(StringTemp, year_text);
            parse_says(0, StringTemp, -1, -1);
            X_pop("RETIREWARNING", 0);
        } else if ((GameDataStateStorage.fields_.game_state_ & STATE_IS_SCENARIO) != 0 &&
                   (GameDataStateStorage.fields_.game_state_ & EndgameRecorded) == 0 &&
                   (static_cast<int>(GameDataStateStorage.fields_.mission_year_current_) ==
                        static_cast<int>(ScenarioSave.ending_mission_year) - 20 ||
                    static_cast<int>(GameDataStateStorage.fields_.mission_year_current_) ==
                        static_cast<int>(ScenarioSave.ending_mission_year) - 10)) {
            parse_num(0, static_cast<int>(ScenarioSave.ending_mission_year));
            StringTemp[0] = 0;
            _itoa(static_cast<int>(ScenarioSave.ending_mission_year), year_text, 10);
            strcat(StringTemp, year_text);
            parse_says(0, StringTemp, -1, -1);
            X_pop(const_cast<char *>("SCENARIO"), "SCENARIOWARNING", 0);
        }
    }

    if ((GameDataStateStorage.fields_.game_state_ & EndgameRecorded) == 0) {
        return 0;
    }
    if ((GameDataStateStorage.fields_.game_state_ & EndgamePresented) != 0) {
        return 0;
    }

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
        parse_num(0, static_cast<int>(GameDataStateStorage.fields_.mission_year_current_));
        StringTemp[0] = 0;
        say_year(StringTemp);
        parse_says(0, StringTemp, -1, -1);
        if ((GameDataStateStorage.fields_.game_state_ & EndgameNetResults) != 0) {
            if (ScenarioSave.objective_req_victory != 0 &&
                (GameDataStateStorage.fields_.game_state_ & EndgameScoreMode) == 0) {
                X_pop(const_cast<char *>("SCENARIO"), "SCENTIMELIMIT", 0);
            } else {
                int best_faction = ConsoleGlobal.faction_id_;
                int best_score = -9999;
                for (int faction_id = 1; faction_id < 8; ++faction_id) {
                    if ((static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_ >> 8) &
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
                    X_pop(const_cast<char *>("SCENARIO"), "SCENTIMEWIN", 0);
                } else {
                    X_pop(const_cast<char *>("SCENARIO"), "SCENTIMELOSS", 0);
                }
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
            X_pop(const_cast<char *>("SCENARIO"), "SUDDENDEATH0", 0);
        } else {
            X_pop(const_cast<char *>("SCENARIO"), "SUDDENDEATH", 0);
        }
        break;
    case EndgameReason16:
        interlude(0x20, 0, 4, 0);
        parse_player_identity(winner);
        popp(ScriptPopupFile, "ENDBEACON", 0, "beacon_sm.pcx", 0);
        if ((GameDataStateStorage.fields_.game_preferences_ & PREF_AV_SECRET_PROJECT_MOVIES) != 0) {
            StringTemp[0] = 0;
            strcat(StringTemp, Players[ConsoleGlobal.faction_id_].filename);
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
    if (IsMultiplayerNet == 0 && EndgameExitReason != EndgameReason7) {
        if (popp(ScriptPopupFile, "GAMEOVERMAN", 0, "stars_sm.pcx", 0) != 0) {
            return 0;
        }
    }
    GameRestartQueued = (IsMultiplayerNet == 0);
    if (IsMultiplayerNet != 0) {
        clear_multiplayer_endgame_state();
    }
    ExitTurnLoop = 1;
    return 1;
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

static inline Map *turn_map_loc(int x, int y) {
    return &map_tiles()[(x >> 1) + MapLongitude * y];
}
static inline int turn_base_who(int x, int y) {
    Map *tile = turn_map_loc(x, y);
    if (tile->bit & BIT_BASE_IN_TILE) {
        int owner = tile->val2 & 0xF;
        if (owner < MaxPlayerNum) {
            return owner;
        }
    }
    return -1;
}

// ORIGINAL: 0x005258C0 ?turn_upkeep@@YAXXZ 0x005258C0-0x0052602D
// RECOVERY_BINDING: v1 {"evidence": ["git:99d6e72bcd5ae3be3566222736528cb0dc57935c:src/unrecovered/005258c0.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "recovery/tasks/game-logic-seed-owner-coverage.json; preserve original extents and historical seed lessons. Ordinary owned literals have pinned content provenance, not exact original data-allocation claims."], "reason": "Recover end-turn aircraft attrition, climate, native-life/alien arrival, caretaker diplomacy and ordered alien-pair stores through genuine owners. Random-events remains an explicitly unfinished separate dependency; build continuity is not game behavior acceptance."}
// TRIED: transcribed directly from Ghidra's decompile (already correct, self-consistent short*/uint* pointer arithmetic throughout), fixing only DAT_/FUN_ names against the catalogue (all ~25 callees were already declared with matching signatures) and CharUpperA (undeclared IAT slot at 0x66931C, called through a typed function-pointer global). MapWin::main_caption()'s receiver is `*g_007d3c3c`, confirmed against src/console.cpp's own use of the same slot. Framed build compiles and matches through instr #3 (mov vs push), sim 0.75 - not chased further, very large orchestration function, landed for coverage.
void __cdecl turn_upkeep() {
    for (int vehicle_id = GameDataStateStorage.fields_.vehicle_current_count_ - 1; vehicle_id >= 0; vehicle_id--) {
        // Air units with limited range burn fuel away from airbases.
        if (get_triad(vehicle_id) == TRIAD_AIR
            && Chassis[VehPrototypes[Vehs[vehicle_id].proto_id].chassis_id].range
            && veh_moves(vehicle_id)
            && turn_base_who(Vehs[vehicle_id].x, Vehs[vehicle_id].y) < 0
            && !(turn_map_loc(Vehs[vehicle_id].x, Vehs[vehicle_id].y)->bit & BIT_AIRBASE)
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
            if (!range(turn_proto_power(vehicle_id) - Vehs[vehicle_id].dmg_incurred, 0, 9999)) {
                kill(vehicle_id);
            }
        }
    }

    VehTerraformStingLatch = 0;
    rankings(1);
    ++GameDataStateStorage.fields_.turn_current_num_;
    GameDataStateStorage.fields_.mission_year_current_ = game_year(GameDataStateStorage.fields_.turn_current_num_);
    MapWinTable[0]->main_caption();

    if (GameDataStateStorage.fields_.climate_sea_level_delta_ != 0) {
        GameDataStateStorage.fields_.climate_rise_accum_ += GameDataStateStorage.fields_.climate_rise_step_;
        if (GameDataStateStorage.fields_.climate_rise_accum_ >= GameDataStateStorage.fields_.climate_rise_limit_) {
            GameDataStateStorage.fields_.climate_rise_accum_ -= GameDataStateStorage.fields_.climate_rise_limit_;
            int step = GameDataStateStorage.fields_.climate_sea_level_delta_ > 0 ? 1
                : (GameDataStateStorage.fields_.climate_sea_level_delta_ < 0 ? -1 : 0);
            GameDataStateStorage.fields_.climate_sea_level_delta_ -= step;
            MapSeaLevel += step;
            world_climate();
            draw_map(1);
            if (step > 0) {
                popp(ScriptPopupFile, "SEARISING", 0, "searis_sm.pcx", 0);
            } else {
                popp(ScriptPopupFile, "SEAFALLING", 0, "seafal_sm.pcx", 0);
            }
        }
    }

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
    if (GameDataStateStorage.fields_.turn_current_num_ == 0x4b
        && has_tech(Facility[FAC_BIOLOGY_LAB].preq_tech, ConsoleGlobal.faction_id_)) {
        interlude(1, 0, 1, 0);
    }
    if (ascending(ConsoleGlobal.faction_id_)
        && !(PlayersData[ConsoleGlobal.faction_id_].flags & 0x2000)) {
        PlayersData[ConsoleGlobal.faction_id_].flags |= 0x2000;
        parse_set(Players[ConsoleGlobal.faction_id_].is_leader_female, 0);
        parse_says(0, Players[ConsoleGlobal.faction_id_].title_leader, -1, -1);
        parse_set(Players[ConsoleGlobal.faction_id_].is_leader_female, 0);
        parse_says(1, Players[ConsoleGlobal.faction_id_].name_leader, -1, -1);
        popp(ScriptPopupFile, "ASCENT", 0, "asctran_sm.pcx", 0);
    }
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
        } else {
            min_damage = 0;
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
        int veh_faction_id = Vehs[veh_id].faction_id;
        if (veh_faction_id != faction_id) {
            continue;
        }
        int x = Vehs[veh_id].x;
        int y = Vehs[veh_id].y;
        Map *tile = map_loc(x, y);
        if (tile->bit & BIT_BASE_IN_TILE) {
            int owner = (int)owner_at(x, y);
            if (owner < MaxPlayerNum && owner >= 0) {
                continue;  // the base's own draw covers it
            }
        }
        if (veh_faction_id != ConsoleGlobal.faction_id_
            && !(Vehs[veh_id].visibility & (1 << ConsoleGlobal.faction_id_))) {
            continue;
        }
        draw_tile(x, y, -1);
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
// RECOVERY_BINDING: v1 {"evidence": ["original:0x005267B0-0x00526E63", "git:38853a3f:src/unrecovered/005267b0.cpp", "source:src/faction.h", "source:src/base.h", "source:src/veh.h"], "reason": "Recover the complete faction energy allocation operation through real PlayerData, Base, Console and VehPrototype owners. Preserve the original first scan of global BaseCurrent without selecting each base, the later severe-police selection/recompute sequence, signed allocation arithmetic and callback order; historical spelling notes are not fresh measurements."}
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
        memset(PlayersData[faction_id].unk_39[row], 0, sizeof(PlayersData[faction_id].unk_39[row]));
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
            PlayersData[faction_id].energy_reserves -= corner_market(faction_id);
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

// The original flattens PendingDiplomacyFlags with (other + 2*faction). Keep
// that arithmetic while resolving the touched index into the actual typed
// row-major array. Its broader intended matrix semantics remain unproven.
extern uint32_t FactionDiplomacyFlags[MaxPlayerNum];
static unsigned int &pending_diplomacy_flag(int flat_index) {
    return PendingDiplomacyFlags[flat_index / MaxPlayerNum]
                                [flat_index % MaxPlayerNum];
}

// Native 0x7388E0 walks a 9-DWORD stride through the first word of the
// existing OfferResetCells owner. Its measured start is two DWORDs into
// OfferResetCells[0][0], and each fourth step lands on the next cell's first
// word. These aliases use the actual typed members.
static int &control_turn_relation_first_word(int flat_index) {
    OfferResetCell &cell = OfferResetCells[0][flat_index / 4];
    switch (flat_index % 4) {
    case 0: return cell.state_[0];
    case 1: return cell.type_[0][7];
    case 2: return cell.value_[0][0];
    default: return cell.value_[1][1];
    }
}

// save_game is declared by the real game owner header.
void __cdecl crash_landing(int faction_id);

// ORIGINAL: 0x005275B0 ?control_turn@@YAXXZ 0x005275B0-0x00528E94;0x0065D6DD-0x0065DB67 FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:f22102a247081a5708204bb307a39c7ddafbe018:src/unrecovered/005275b0.cpp: full original identity/spans and useful historical findings", "ConsoleInfoWin original object7AD2A0 owns suppression memberAC at7AD34C; native527A70/527C9E set1/0; init4592A0 and timer_proc459150 confirm ownership.", "OfferResetCells, PendingDiplomacyFlags and FactionDiplomacyFlags retain original flattened indexing through their actual typed owners.", "Initial text at93AA04/08/0C is proven loader-zero and only read by observed consumers; one-byte source bounds are lower-bound interpretations, not claims of complete original capacity or absence of indirect writers.", "Complete native cleanup-route review supports one real Popup owner; matching and exceptional cleanup restoration remain unfinished.", "recovery/tasks/control-turn-owner-oct5.json", "recovery/tests/test_control_turn_actual_source.py"], "reason": "Recover complete turn orchestration through existing genuine application owners and automatic Popup lifetime. Preserve original branch ordering and error behavior; all primary and exceptional spans remain restoration obligations, not a claim of current native lifetime/byte/runtime equivalence."}
// TRIED: mismatch #11 near prologue (xor vs push) - popup/end_game_pop declared as real C++ locals rather than the original's staggered same-slot vtable-poke construction; the "big dialog" tail (Dialogs/Spot/ListBox/CheckBox/EditGroup/ SpriteBox/StringStruct x2/Sprite) is reached by raw offset casts into BasePop's own dialogs_/spot_/sprite_ members, not by a modelled derived class - faithful call order/args, not byte fidelity. Main loop, faction dispatch, diplomacy/ tech/council logic transcribed directly from the disasm.
void __cdecl control_turn() {
    Popup popup;
    int retry_count = 0;
    int save_pending;
    int result;

    if (GameDataStateStorage.fields_.turn_current_num_ == 0 && ControlTurnPhase == 0 &&
        ((static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_ >> 8) >>
          (ConsoleGlobal.faction_id_ & 0x1F)) & 1u) != 0) {
        if (IsMultiplayerPBEM == 0) {
            crash_landing(ConsoleGlobal.faction_id_);
        }
    }
    if (IsMultiplayerPBEM == 0) {
        draw_map(1);
    }

    MapBaseDisplayOverride = 1;
    if (ControlTurnPhase != 0 &&
        ((static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_ >> 8) >>
          (ConsoleGlobal.faction_id_ & 0x1F)) & 1u) == 0) {
        ConsoleGlobal.human_turn();
    }

    do {
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

        int human_turn_shown = 0;
        int faction = 0;
        do {
            const unsigned int alive_mask =
                static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_ >> 8);
            if (((alive_mask >> (faction & 0x1F)) & 1u) != 0 &&
                ControlTurnPhase <= faction) {
                const int active_faction = ConsoleGlobal.faction_id_;
                GameDataStateStorage.fields_.net_turn_faction_ = faction;
                const unsigned int report_mask =
                    static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_);
                const bool has_report =
                    ((report_mask >> (faction & 0x1F)) & 1u) != 0;
                if (has_report &&
                    (faction != active_faction || (GameDataStateStorage.fields_.net_turn_flags_ & 0x20) != 0 ||
                     IsMultiplayerPBEM != 0)) {
                    Player &player = Players[faction];
                    parse_says(0, ControlTurnMailTemplate, -1, -1);
                    GenderDefault = player.is_leader_female;
                    PluralityDefault = 0;
                    parse_says(1, player.name_leader, -1, -1);
                    GenderDefault = player.is_leader_female;
                    PluralityDefault = 0;
                    parse_says(2, player.title_leader, -1, -1);
                    PluralityDefault = player.is_noun_plural;
                    GenderDefault = player.noun_gender;
                    parse_says(3, player.noun_faction, -1, -1);
                    ConsoleGlobal.faction_id_ = faction;

                    if (IfaceIdToSlotStorage[ActiveIfaceIdStorage] == 10) {
                        g_BattleWin.stop_timer();
                        static_cast<SubInterface &>(g_BattleWin)
                            .release_iface_mode();
                    }
                    stop_timers();
                    TutWinBaseWindow.exit();
                    ConsoleGlobal.alpha_menu_.hide();
                    MultiWindow.hide();
                    InterfaceMessageWin.listBox_.Win::hide();
                    MainInterfaceGlobal.string_box_.Win::hide();
                    WorldClimateWorldWin.hide_all();

                    RECT frame = MainInterfaceGlobal.status_left_rect_;
                    static_cast<Buffer &>(MainInterfaceGlobal).box_sprite(
                        &frame, &InterfaceBoxStyles[21]);
                    frame.left += 3; frame.top += 3;
                    frame.right -= 3; frame.bottom -= 3;
                    static_cast<Buffer &>(MainInterfaceGlobal).box_sprite(
                        &frame, &InterfaceBoxStyles[18]);

                    frame = MainInterfaceGlobal.status_right_rect_;
                    static_cast<Buffer &>(MainInterfaceGlobal).box_sprite(
                        &frame, &InterfaceBoxStyles[21]);
                    frame.left += 3; frame.top += 3;
                    frame.right -= 3; frame.bottom -= 3;
                    static_cast<Buffer &>(MainInterfaceGlobal).box_sprite(
                        &frame, &InterfaceBoxStyles[18]);

                    frame = MainInterfaceGlobal.status_hit_rect_;
                    static_cast<Buffer &>(MainInterfaceGlobal).box_sprite(
                        &frame, &InterfaceBoxStyles[21]);
                    frame.left += 3; frame.top += 3;
                    frame.right -= 3; frame.bottom -= 3;
                    static_cast<Buffer &>(MainInterfaceGlobal).box_sprite(
                        &frame, &InterfaceBoxStyles[18]);

                    frame = MainInterfaceGlobal.tutorial_bounds_;
                    MainInterfaceGlobal.Buffer::box_sprite(
                        &frame, &InterfaceBoxStyles[21]);
                    frame.left += 3; frame.top += 3;
                    frame.right -= 3; frame.bottom -= 3;
                    MainInterfaceGlobal.Buffer::box_sprite(
                        &frame, &InterfaceBoxStyles[18]);

                    frame = MainInterfaceGlobal.message_bounds_;
                    MainInterfaceGlobal.Buffer::box_sprite(
                        &frame, &InterfaceBoxStyles[21]);
                    frame.left += 3; frame.top += 3;
                    frame.right -= 3; frame.bottom -= 3;
                    MainInterfaceGlobal.Buffer::box_sprite(
                        &frame, &InterfaceBoxStyles[18]);

                    frame = MainInterfaceGlobal.date_rect_;
                    MainInterfaceGlobal.Buffer::box_sprite(
                        &frame, &InterfaceBoxStyles[18]);

                    MainInterfaceGlobal.update(static_cast<GraphicWin *>(0));
                    ConsoleInfoWin.set_timer_suppression(1);
                    ConsoleGlobal.cover_window_.fill(0);
                    ConsoleGlobal.cover_window_.Win::show(0);
                    do_all_draws();
                    MainInterfaceGlobal.string_box_.clear();

                show_turn_password:
                    popup.start(PopupStartCaption,
                        PlayersData[faction].unk_102 == 0
                            ? "SWITCHING" : "SWITCHING2",
                        -1, 0, 0x800004, 0);
                    result = popup.exec(0, 0);
                    if (result < 0) {
                        if (ExitTurnLoop != 0) goto turn_exit;
                        goto show_turn_password;
                    }

                    save_pending = static_cast<int>(popup.input_poll_state());
                    if (save_pending != 0) {
                        GameDataStateStorage.fields_.net_turn_flags_ |= 0x40;
                        if (save_game(0) == 0) {
                            return;
                        }
                        GameDataStateStorage.fields_.net_turn_flags_ &= ~0x40;
                        if (ExitTurnLoop != 0) {
                            return;
                        }
                        goto show_turn_password;
                    }

                    {
                        const uint32_t password_hash =
                            checksum_password(ParseStrBuffer[0].str);
                        PlayerData &data = PlayersData[faction];
                        if (password_hash != static_cast<uint32_t>(data.unk_102)) {
                            if (data.unk_102 != 0) {
                                if (++retry_count > 2) {
                                    GameDataStateStorage.fields_.net_turn_flags_ |= 0x40;
                                    save_game(0);
                                    ConsoleGlobal.Win::hide();
                                    ExitTurnLoop = 1;
                                    GameRestartQueued = 0;
                                    return;
                                }
                                goto bad_turn_password;
                            }
                            if (X_pop_ask(
                                    "VERIFYPASSWORD", 0x50,
                                    ControlTurnPasswordInitialText,
                                    static_cast<int (__cdecl *)()>(0),
                                    0x800000) != 0) {
                                goto bad_turn_password;
                            }
                            if (checksum_password(ParseStrBuffer[0].str) !=
                                password_hash) {
                                goto bad_turn_password;
                            }
                            data.unk_102 = static_cast<int>(password_hash);
                        }
                    }
                    goto password_accepted;

                bad_turn_password:
                    if (ExitTurnLoop != 0) goto turn_exit;
                    goto show_turn_password;

                password_accepted:
                    if (ExitTurnLoop == 0) {
                        bool found_owned_base = false;
                        int index = 0;
                        if (GameDataStateStorage.fields_.base_current_count_ > 0) {
                            while (index < GameDataStateStorage.fields_.base_current_count_ &&
                                   Bases[index].faction_id_current !=
                                       ConsoleGlobal.faction_id_) {
                                ++index;
                            }
                            if (index < GameDataStateStorage.fields_.base_current_count_) {
                                ConsoleGlobal.set_center(Bases[index].x,
                                                         Bases[index].y, 1);
                                ConsoleGlobal.set_cursor(Bases[index].x,
                                                         Bases[index].y);
                                found_owned_base = true;
                            }
                        }
                        if (!found_owned_base && GameDataStateStorage.fields_.vehicle_current_count_ > 0) {
                            index = 0;
                            while (index < GameDataStateStorage.fields_.vehicle_current_count_ &&
                                   static_cast<unsigned char>(
                                       Vehs[index].faction_id) !=
                                       ConsoleGlobal.faction_id_) {
                                ++index;
                            }
                            if (index < GameDataStateStorage.fields_.vehicle_current_count_) {
                                ConsoleGlobal.set_center(Vehs[index].x,
                                                         Vehs[index].y, 1);
                                ConsoleGlobal.set_cursor(Vehs[index].x,
                                                         Vehs[index].y);
                            }
                        }
                    }

                    ConsoleGlobal.cover_window_.Win::hide();
                    ConsoleInfoWin.set_timer_suppression(0);
                    ConsoleStatusWin.on_redraw();
                    WorldClimateWorldWin.field_239BC_ |= 4;
                    WorldClimateWorldWin.show_all();
                    WorldClimateWorldWin.redraw();
                    MainInterfaceGlobal.string_box_.Win::show(0);
                    InterfaceMessageWin.clear();

                    {
                        const int local_faction = ConsoleGlobal.faction_id_;
                        const int wins = FactionTurnCombatWins[local_faction];
                        const int losses = FactionTurnCombatLosses[local_faction];
                        if (wins != 0 || losses != 0) {
                            parse_num(wins, 0);
                            parse_num(losses, 1);
                            popp(ScriptPopupFile, "WONLOST", 0,
                                 "bad_sm.pcx", 0);
                        }
                    }
                    if (display_player_messages(ConsoleGlobal.faction_id_) != 0) {
                        MainInterfaceGlobal.button_group(0).set(1, 0x3E8);
                    }
                    if (MainInterfaceGlobal.button_group(0).field_84_ == 0x3E8) {
                        InterfaceMessageWin.listBox_.Win::show(0);
                    }
                    ConsoleGlobal.update_data(0);
                    start_timers();
                    do_all_draws();

                    {
                        const int local_faction = ConsoleGlobal.faction_id_;
                        const int other = TamperDialogFaction[local_faction];
                        if (other != 0) {
                            Player &tamper_player = Players[other];
                            GenderDefault = tamper_player.is_leader_female;
                            PluralityDefault = 0;
                            parse_says(0, tamper_player.title_leader, -1, -1);
                            GenderDefault = tamper_player.is_leader_female;
                            PluralityDefault = 0;
                            parse_says(1, tamper_player.name_leader, -1, -1);
                            GenderDefault = tamper_player.noun_gender;
                            PluralityDefault = tamper_player.is_noun_plural;
                            parse_says(2, tamper_player.noun_faction, -1, -1);
                            parse_num(TamperDialogValue[local_faction], 0);
                            X_pop("HOTTAMPER2", 0);
                            TamperDialogFaction[local_faction] = 0;
                            TamperDialogValue[local_faction] = 0;
                        }
                    }

                    {
                        int local_faction = ConsoleGlobal.faction_id_;
                        while (PlayersData[local_faction].earned_techs_saved > 0) {
                            --PlayersData[local_faction].earned_techs_saved;
                            tech_advance(local_faction);
                            local_faction = ConsoleGlobal.faction_id_;
                        }

                        for (int other = 1; other < MaxPlayerNum; ++other) {
                            const int pending_index = other + local_faction * 2;
                            FactionDiplomacyFlags[1] =
                                pending_diplomacy_flag(pending_index);
                            pending_diplomacy_flag(pending_index) = 0;
                            if (other != local_faction) {
                                const unsigned int alive =
                                    static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_ >> 8);
                                if (((alive >> (other & 0x1F)) & 1u) == 0) {
                                    if (FactionDiplomacyFlags[1] != 0) {
                                        communicate(local_faction, other, 0);
                                        local_faction = ConsoleGlobal.faction_id_;
                                    }
                                } else if (FactionDiplomacyFlags[1] != 0) {
                                    int &relation_first_word =
                                        control_turn_relation_first_word(
                                            other + local_faction * 2);
                                    if (relation_first_word == 3) {
                                        relation_first_word = 0;
                                        Player &message_player = Players[other];
                                        GenderDefault = message_player.is_leader_female;
                                        PluralityDefault = 0;
                                        parse_says(0, message_player.title_leader,
                                                   -1, -1);
                                        GenderDefault = message_player.is_leader_female;
                                        PluralityDefault = 0;
                                        parse_says(1, message_player.name_leader,
                                                   -1, -1);
                                        GenderDefault = message_player.noun_gender;
                                        PluralityDefault = message_player.is_noun_plural;
                                        parse_says(2, message_player.noun_faction,
                                                   -1, -1);
                                        X_pop("COMMCOMPLETE", 0);
                                        local_faction = ConsoleGlobal.faction_id_;
                                    } else {
                                        diplo(other, local_faction);
                                        local_faction = ConsoleGlobal.faction_id_;
                                    }
                                }
                                FactionDiplomacyFlags[1] = 0;
                            }
                        }

                        if (CouncilPbemGate == 0) {
                            int council_state =
                                CouncilNotifyPluralSlots[local_faction];
                            if (council_state != 0) {
                                const int proposal_id =
                                    CouncilNotifyFormSlots[local_faction];
                                parse_say(0, Proposal[proposal_id].name,
                                          -1, -1);
                                local_faction = ConsoleGlobal.faction_id_;
                                int message_state =
                                    CouncilNotifyPluralSlots[local_faction];
                                const char *picture;
                                const char *message;
                                if (CouncilNotifyFormSlots[local_faction] == 0) {
                                    if (GameDataStateStorage.fields_.governor_faction_ > 0) {
                                        Player &governor = Players[GameDataStateStorage.fields_.governor_faction_];
                                        GenderDefault = governor.is_leader_female;
                                        PluralityDefault = 0;
                                        parse_says(0, governor.title_leader,
                                                   -1, -1);
                                        GenderDefault = governor.is_leader_female;
                                        PluralityDefault = 0;
                                        parse_says(1, governor.name_leader,
                                                   -1, -1);
                                        GenderDefault = governor.noun_gender;
                                        PluralityDefault = governor.is_noun_plural;
                                        parse_says(2, governor.noun_faction,
                                                   -1, -1);
                                    }
                                    if (message_state == 1) {
                                        picture = "council_sm.pcx";
                                        message = "COUNCILHOTGOVWIN";
                                    } else if (GameDataStateStorage.fields_.governor_faction_ < 1) {
                                        picture = "council_sm.pcx";
                                        message = "COUNCILHOTGOVNONE";
                                    } else {
                                        picture = "council_sm.pcx";
                                        message = "COUNCILHOTGOVLOSE";
                                    }
                                } else if (message_state == 1) {
                                    picture = "council_sm.pcx";
                                    message = "COUNCILHOTPASS";
                                } else if (message_state == -2) {
                                    picture = "council_sm.pcx";
                                    message = "COUNCILHOTVETO";
                                } else {
                                    picture = "council_sm.pcx";
                                    message = "COUNCILHOTFAIL";
                                }
                                popp(ScriptPopupFile, message, 0, picture, 0);
                                local_faction = ConsoleGlobal.faction_id_;
                                clear_council_notify(local_faction);
                            }
                        } else {
                            council(local_faction, 0, 0);
                            local_faction = ConsoleGlobal.faction_id_;
                        }
                    }
                }

                if (ExitTurnLoop == 0) {
                    if (ControlTurnPhase == 0 ||
                        (IsMultiplayerPBEM != 0 && (GameDataStateStorage.fields_.net_turn_flags_ & 0x40) != 0)) {
                        ConsoleControlTurnActive = 1;
                        faction_upkeep(faction);
                        result = end_of_game(0);
                        if (result != 0) goto turn_exit;
                    }
                    GameDataStateStorage.fields_.net_turn_flags_ &= ~0x40;
                    ConsoleControlTurnActive = 0;
                    const unsigned int alive =
                        static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_);
                    const int current_faction = ConsoleGlobal.faction_id_;
                        if ((alive & (1u << (faction & 0x1F))) == 0) {
                            enemy_turn(faction);
                        } else {
                        if (faction != current_faction && bit_count(alive) < 2) {
                            goto post_human_dispatch;
                        }
                        ConsoleGlobal.human_turn();
                        if (ControlTurnPhase != 0) {
                            faction = 0;
                            goto post_human_dispatch;
                        }
                        human_turn_shown = 1;
                    }

                post_human_dispatch:
                    ControlTurnPhase = 0;
                    if (ExitTurnLoop == 0) {
                        result = end_of_game(0);
                        if (result != 0) goto turn_exit;
                    }
                }
            }
            ++faction;
        } while (faction < MaxPlayerNum);

        if (human_turn_shown == 0) {
            ConsoleGlobal.human_turn();
        }
    } while (ExitTurnLoop == 0);

turn_exit:
    return;
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

// RECOVERY_OBJECT: v2 0x0068BBE4 0x0068BBFA _NetUpkeepSkippingUpkeepSynch 0x005290A0
static const char NetUpkeepSkippingUpkeepSynch[] = "Skipping upkeep synch";
// RECOVERY_OBJECT: v2 0x0068BC1C 0x0068BC32 _NetUpkeepSkippingResearchSynch 0x005290A0
static const char NetUpkeepSkippingResearchSynch[] = "Skipping upkeep synch";
// RECOVERY_OBJECT: v2 0x0068BC3C 0x0068BC52 _NetUpkeepSkippingPrototypeSynch 0x005290A0
static const char NetUpkeepSkippingPrototypeSynch[] = "Skipping upkeep synch";

// RECOVERY_OBJECT: v2 0x0068BBFC 0x0068BC0B _NetUpkeepUpkeepAdvance 0x005290A0
static const char NetUpkeepUpkeepAdvance[] = "Upkeep Advance";

// RECOVERY_OBJECT: v2 0x0068BC0C 0x0068BC19 _NetUpkeepSetResearch 0x005290A0
static const char NetUpkeepSetResearch[] = "Set Research";

// RECOVERY_OBJECT: v2 0x0068BC34 0x0068BC3C _NetUpkeepUpkeep4 0x005290A0
static const char NetUpkeepUpkeep4[] = "UPKEEP4";

// RECOVERY_OBJECT: v2 0x0068BC54 0x0068BC5C _NetUpkeepUpkeep5 0x005290A0
static const char NetUpkeepUpkeep5[] = "UPKEEP5";

// RECOVERY_OBJECT: v2 0x0068BC5C 0x0068BC68 _NetUpkeepEnemyTurn 0x005290A0
static const char NetUpkeepEnemyTurn[] = "Enemy turn ";

// RECOVERY_OBJECT: v2 0x0068BC68 0x0068BC70 _NetUpkeepUpkeep6 0x005290A0
static const char NetUpkeepUpkeep6[] = "UPKEEP6";

// RECOVERY_OBJECT: v2 0x0068BC70 0x0068BC89 _NetUpkeepSaveDirectory 0x005290A0
static const char NetUpkeepSaveDirectory[] = "k:\\breynolds\\multisaves\\";

// RECOVERY_OBJECT: v2 0x0068BC8C 0x0068BC8F _NetUpkeepSavePrefix 0x005290A0
static const char NetUpkeepSavePrefix[] = "AC";

// RECOVERY_OBJECT: v2 0x0068BC90 0x0068BC96 _NetUpkeepSaveSuffix 0x005290A0
static const char NetUpkeepSaveSuffix[] = "multi";

// RECOVERY_OBJECT: v2 0x0068BC98 0x0068BCAA _NetUpkeepSaveLog 0x005290A0
static const char NetUpkeepSaveLog[] = "Writing save file";

// RECOVERY_OBJECT: v2 0x0068BCAC 0x0068BCB4 _NetUpkeepUpkeep7 0x005290A0
static const char NetUpkeepUpkeep7[] = "UPKEEP7";

// RECOVERY_OBJECT: v2 0x0068BCB4 0x0068BCD3 _NetUpkeepGameChecksum 0x005290A0
static const char NetUpkeepGameChecksum[] = "Failed GAME checksum - resynch";

// RECOVERY_OBJECT: v2 0x0068BCD4 0x0068BCF6 _NetUpkeepLeadersChecksum 0x005290A0
static const char NetUpkeepLeadersChecksum[] = "Failed LEADERS checksum - resynch";

// RECOVERY_OBJECT: v2 0x0068BCF8 0x0068BD16 _NetUpkeepMapChecksum 0x005290A0
static const char NetUpkeepMapChecksum[] = "Failed MAP checksum - resynch";

// RECOVERY_OBJECT: v2 0x0068BD18 0x0068BD3B _NetUpkeepVehiclesChecksum 0x005290A0
static const char NetUpkeepVehiclesChecksum[] = "Failed VEHICLES checksum - resynch";

// RECOVERY_OBJECT: v2 0x0068BD3C 0x0068BD5C _NetUpkeepBasesChecksum 0x005290A0
static const char NetUpkeepBasesChecksum[] = "Failed BASES checksum - resynch";

// RECOVERY_OBJECT: v2 0x0068BD5C 0x0068BD81 _NetUpkeepMapSectionChecksum 0x005290A0
static const char NetUpkeepMapSectionChecksum[] = "Failed MAP section chcksum - resynch";

// RECOVERY_OBJECT: v2 0x0068BD84 0x0068BD9A _NetUpkeepSkippingUpkeepPhase 0x005290A0
static const char NetUpkeepSkippingUpkeepPhase[] = "Skipping upkeep phase";

// ORIGINAL: 0x00528EA0 ?net_upkeep_phase@@YAHH@Z 0x00528EA0-0x00529097 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf; original:0x00528EA0-0x00529097; saved pinned full-span sha256:6d3147e1a8d83201f57389f6c8ed65a0938667f845bfcec4215f941828ab75ee", "source:src/game.cpp:net_upkeep at 0x005290A0 is the existing caller/turn owner; game.cpp already includes the NetDaemon, Console, lock-mask, MultiWindow, parser/message, log, temp and WinMM interfaces", "source:src/netdaemon.h:LockStateEntry is 0x3C bytes; lock_state_ begins at NetDaemon+0x1B2C; the referenced sent, ack and turn-end words are existing members, not new globals", "source:src/console.h:ConsoleGlobal.faction_id_; source:src/game_data_state_window.h:GameDataStateStorage.fields_.turn_current_num_ and lock_masks_.enable_mask_; source:src/game.h:ExitTurnLoop", "pinned branch review: preserve early returns before close/mask reset, timeout > 1000, phase>=4 popup suppression, live callback mask rereads, server receive loop, common cleanup order and final ExitTurnLoop reread", "historical seed:git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/00528ea0.cpp; existing TRIED note remains attached verbatim"], "reason": "Keep the multiplayer turn rendezvous in the existing game-turn owner, using the already typed NetDaemon lock-state, Console, game-turn, GameDataStateWindow lock-mask and WinMM APIs while retaining this complete original body identity."}
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
        git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/00528ea0.cpp retained.
*/
int __cdecl net_upkeep_phase(int phase) {
    char number_buffer[0x50];
    uint32_t &sent_mask = NetDaemonNet.channels_[0].leading_words_[1];
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
        int faction = (GameDataStateStorage.fields_.turn_current_num_ + i) & 0x80000007;
        if (faction < 0) faction = ((faction - 1) | 0xFFFFFFF8) + 1;
        if (((1u << (faction & 0x1F)) &
             static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_ >> 8)) != 0) {
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
        for (unsigned int faction = 0; faction < 8; ++faction) {
            if (static_cast<int>(faction) == ConsoleGlobal.faction_id_) {
                synch_leader(static_cast<int>(faction));
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
        log_say(NetUpkeepSkippingUpkeepSynch, NetUpkeepPhase, 0, 0);
    }

    TurnUpkeepPhase = 2;
    ++NetUpkeepPhase;
    {
        unsigned int flags = 0;
        for (unsigned int faction = 1; faction < MaxPlayerNum; ++faction) {
            const unsigned char bit =
                static_cast<unsigned char>(1u << (faction & 0x1F));
            PlayerData &data = PlayersData[faction];
            if ((bit & static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_ >> 8)) != 0) {
                while (data.earned_techs_saved != 0) {
                    log_say(NetUpkeepUpkeepAdvance, Players[faction].name_leader,
                            faction, data.earned_techs_saved, 0);
                    --data.earned_techs_saved;
                    tech_advance(static_cast<int>(faction));
                    if ((bit & static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_)) != 0) {
                        flags |= (faction ==
                                  static_cast<unsigned int>(ConsoleGlobal.faction_id_))
                            + 1u;
                    }
                }
                if (data.tech_accumulated != 0 &&
                    data.tech_id_researching < 0) {
                    log_say(NetUpkeepSetResearch, faction, 0, 0);
                    data.tech_id_researching =
                        tech_selection(static_cast<int>(faction));
                    if ((bit & static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_)) != 0) {
                        flags |= (faction ==
                                  static_cast<unsigned int>(ConsoleGlobal.faction_id_))
                            + 1u;
                    }
                }
                if ((data.flags & 2u) != 0) {
                    consider_designs(static_cast<int>(faction));
                }
                data.flags &= ~2u;
            }
        }

        if (flags == 0) {
            log_say(NetUpkeepSkippingResearchSynch, NetUpkeepPhase, 0, 0);
        } else if (net_upkeep_phase(NetUpkeepPhase) != 0) {
            return;
        }
        ++NetUpkeepPhase;
        TurnUpkeepPhase = 0;
        if ((flags & 2u) != 0 && (GameDataStateStorage.fields_.game_rules_ & 0x200u) != 0) {
            ConsoleGlobal.set_ai(ConsoleGlobal.faction_id_);
        }
    }

    NetMsgPrimary.pop(NetUpkeepUpkeep4, 0, 1, 0);
    {
        unsigned int found_prototype = 0;
        UpgradePromptAnswered = 0;
        for (int prototype_id = 0x40; prototype_id < MaxVehProtoNum;
             ++prototype_id) {
            VehPrototype &prototype = VehPrototypes[prototype_id];
            const uint16_t flags = prototype.flags;
            if ((flags & 1u) != 0) {
                const int faction = prototype_id >> 6;
                const unsigned char faction_bit =
                    static_cast<unsigned char>(1u << (faction & 0x1F));
                if ((prototype.obsolete_factions & faction_bit) == 0 &&
                    (flags & 0x100u) != 0) {
                    found_prototype = 1;
                    prototype.flags = static_cast<uint16_t>(flags & 0xFEFFu);
                    if (faction == ConsoleGlobal.faction_id_ &&
                        (GameDataStateStorage.fields_.game_more_preferences_ & 0x100000u) != 0) {
                        upgrade_prototypes(ConsoleGlobal.faction_id_,
                                           prototype_id);
                    }
                }
            }
        }

        if (CouncilPendingMessageArgs[0] < 1) {
            if (found_prototype == 0) {
                log_say(NetUpkeepSkippingPrototypeSynch, NetUpkeepPhase, 0, 0);
                goto upkeep5;
            }
        } else {
            if (NetDaemonNet.primary_state_.leading_words_[0] != 0) {
                message_data(0x2600, 0, CouncilPendingMessageArgs[0],
                             CouncilPendingMessageArgs[1],
                             CouncilPendingMessageArgs[2], 0);
                TurnRedrawPending = 1;
                do {
                    if (ExitTurnLoop != 0) break;
                    wait_task();
                } while (TurnRedrawPending != 0);
            }
            CouncilPendingMessageArgs[0] = -1;
            CouncilPendingMessageArgs[1] = -1;
            CouncilPendingMessageArgs[2] = -1;
        }
        if (net_upkeep_phase(NetUpkeepPhase) != 0) return;
    }

upkeep5:
    ++NetUpkeepPhase;
    NetMsgPrimary.pop(NetUpkeepUpkeep5, 0, 1, 0);
    for (i = 0; i < 8; ++i) {
        const unsigned char bit = static_cast<unsigned char>(1u << i);
        if ((static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_ >> 8) & bit) != 0 &&
            (static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) & bit) == 0) {
            log_say(NetUpkeepEnemyTurn, i, 0, 0);
            enemy_turn(i);
        }
    }
    if (net_upkeep_phase(NetUpkeepPhase) != 0) return;
    ++NetUpkeepPhase;
    NetMsgPrimary.pop(NetUpkeepUpkeep6, 0, 1, 0);
    NetDaemonNet.deletion_list_.set_sequence_bits(0);
    for (i = 0; i < 0x18; ++i) {
        NetDaemonNet.deletion_list_.clear(i);
    }
    do_checksums(4);

    if ((GameDataStateStorage.fields_.game_preferences_ & 2u) != 0) {
        if (BaseAtDebugPopupFlag == 0) {
            auto_save();
        } else {
            StringTemp[0] = 0;
            strcat(StringTemp, NetUpkeepSaveDirectory);
            strcat(StringTemp, NetUpkeepSavePrefix);
            char faction_text[16];
            _itoa(ConsoleGlobal.faction_id_, faction_text, 10);
            strcat(StringTemp, faction_text);
            strcat(StringTemp, NetUpkeepSaveSuffix);
            char turn_text[16];
            _itoa(GameDataStateStorage.fields_.turn_current_num_, turn_text, 10);
            strcat(StringTemp, turn_text);
            log_say(NetUpkeepSaveLog, StringTemp, 0, 0, 0);
            save_daemon(StringTemp);
        }
    }

    message_big_data(0x2307, 0, NetTurnChecksums, 0x15);
    if (net_upkeep_phase(NetUpkeepPhase) != 0) return;
    NetDaemonNet.primary_state_.trailing_words_[1] = 0xFFFFFFFFu;
    ++NetUpkeepPhase;

    {
        unsigned int any_mismatch = 0;
        // Original 0x0052972D clears 21 DWORDs; 0x005297C0/0x005297D0
        // read and set four-byte flags, not packed byte flags.
        int mismatch_seen[NetTurnChecksumWords];
        memset(mismatch_seen, 0, sizeof mismatch_seen);

        for (unsigned int faction = 1; faction < MaxPlayerNum; ++faction) {
            const unsigned char bit =
                static_cast<unsigned char>(1u << (faction & 0x1F));
            if ((static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_ >> 8) & bit) == 0 ||
                (static_cast<unsigned char>(GameDataStateStorage.fields_.lock_masks_.enable_mask_) & bit) == 0 ||
                static_cast<int>(faction) == ConsoleGlobal.faction_id_) {
                continue;
            }

            const int *peer = NetPeerTurnChecksums[faction];
            for (int word = 0; word < NetTurnChecksumWords; ++word) {
                if (NetTurnChecksums[word] == peer[word]) continue;
                if (any_mismatch == 0) {
                    NetMsgPrimary.pop(NetUpkeepUpkeep7, 0, 1, 0);
                }
                any_mismatch = 1;
                if (mismatch_seen[word] != 0) continue;
                mismatch_seen[word] = 1;
                if (NetDaemonNet.primary_state_.leading_words_[0] == 0) {
                    continue;
                }

                switch (word) {
                case 0:
                    log_say(NetUpkeepGameChecksum, faction, NetTurnChecksums[0],
                            peer[0]);
                    NetDaemonNet.synch(0, 0, 0, 0, 0, 1, 0x2101);
                    while (NetDaemonNet.receive() != 0) { }
                    NetDaemonNet.synch(0x24, 0, 0, 0, 0, 1, 0x2101);
                    while (NetDaemonNet.receive() != 0) { }
                    break;
                case 1: {
                    log_say(NetUpkeepLeadersChecksum, faction, NetTurnChecksums[1],
                            peer[1]);
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
                    log_say(NetUpkeepMapChecksum, faction, NetTurnChecksums[2],
                            peer[2]);
                    NetDaemonNet.synch(0x1002, 0, 0, 0, 0, 1, 0x2101);
                    while (NetDaemonNet.receive() != 0) { }
                    NetDaemonNet.synch(5, 0, 0, 0, 0, 1, 0x2101);
                    while (NetDaemonNet.receive() != 0) { }
                    break;
                case 3:
                    log_say(NetUpkeepVehiclesChecksum, faction, NetTurnChecksums[3],
                            peer[3]);
                    NetDaemonNet.synch(0x25, 0, 0, 0, 0, 1, 0x2101);
                    while (NetDaemonNet.receive() != 0) { }
                    NetDaemonNet.synch(0x12, 0, 0, 0, 0, 1, 0x2101);
                    while (NetDaemonNet.receive() != 0) { }
                    break;
                case 4:
                    log_say(NetUpkeepBasesChecksum, faction, NetTurnChecksums[4],
                            peer[4]);
                    NetDaemonNet.synch(0x26, 0, 0, 0, 0, 1, 0x2101);
                    while (NetDaemonNet.receive() != 0) { }
                    NetDaemonNet.synch(0x14, 0, 0, 0, 0, 1, 0x2101);
                    while (NetDaemonNet.receive() != 0) { }
                    break;
                default:
                    // Original 0x00529B16 reloads the local checksum through
                    // the 0x0093A008 cursor, independently of peer row zero.
                    log_say(NetUpkeepMapSectionChecksum, faction,
                            NetTurnChecksums[word], peer[word]);
                    NetDaemonNet.synch(0x30, word - 5, 0, 0, 0, 1,
                                       0x2101);
                    while (NetDaemonNet.receive() != 0) { }
                    break;
                }
            }
        }

        NetMsgPrimary.close();
        SpockHelpActive = 0;
        SpockNotifyPending = 0;
        if (any_mismatch != 0) {
            net_upkeep_phase(NetUpkeepPhase);
            return;
        }
        log_say(NetUpkeepSkippingUpkeepPhase, NetUpkeepPhase, 0, 0);
    }
}

// ORIGINAL: 0x00529C00 ?mash_planes@@YAXXZ 0x00529C00-0x00529E67 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:8b0da75894c71ec098213e7e12cc29fb1dfc01f3:src/recovered/units/00529c00.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "recovery/tasks/game-logic-seed-owner-coverage.json; complete typed owner migration, preserve original spans and seed lessons. Literal content provenance does not assert original allocation or game acceptance."], "reason": "Native aircraft attrition: vehicle movement spent at+0x28 controls eligibility; damage at+0x10 accumulates reactor-derived attrition; terraforming turns at+0x25 clears on survival. Prototype+0x2A is plan, chassis+0x4A is range; preserve post-focus/boom prototype reads."}
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
// RECOVERY_BINDING: v1 {"evidence": ["Original529E70..529F2C; retired seed git:117282ee:src/unrecovered/00529e70.cpp records the same identity and spans with its scheduling TRIED note kept below.", "The turn block is console.h's own ConsoleGlobal (0x009156B0, set_view), game.h's MultiWindow (0x007FD648, the hypothesis layout's draw), netdaemon.h's NetDaemonNet (0x0093CD90, net_tasks) and game.h's ExitTurnLoop (0x009B2068); the waiting latch at 0x0093E8EC takes this faction's bit and 0x00939294/0x00939298 are the two turn-handover flags.", "game.cpp binds 0x0093A950 as NextTurnReady and 0x0093E8C0 as TurnsAssigned; 0x009A6820 is game.cpp's NetTurnFaction and the skip tests are lock.h's LockEnableMask bytes at 0x009A64E8/0x009A64E9.", "next_player_turn (game.cpp's 0x0052DBF0) has no header declaration yet, so the declaration travels with this body.", "Pinned 0x00529EA4 and 0x00529EFF use direct 32-bit SHL by CL. The first shifts the local faction into the waiting set; the guarded active-turn test retains a full-width bit while testing the human/alive bytes. Do not add source count masks or narrow the second shift temporary."], "reason": "Coverage transcription of the network turn wait on the tree's own named objects."}
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

// RECOVERY_OBJECT: v2 0x0068BD9C 0x0068BDB8 _NetEndTurnEnterEndTurnLog 0x00529F30
static const char NetEndTurnEnterEndTurnLog[] = "Entering 'end of turn' loop";

// RECOVERY_OBJECT: v2 0x0068BDB8 0x0068BDC6 _NetEndTurnFinishingTurnLabel 0x00529F30
static const char NetEndTurnFinishingTurnLabel[] = "FINISHINGTURN";

// RECOVERY_OBJECT: v2 0x0068BDC8 0x0068BDD9 _NetEndTurnReadyToProceedLog 0x00529F30
static const char NetEndTurnReadyToProceedLog[] = "Ready to proceed";

// RECOVERY_OBJECT: v2 0x0068BDDC 0x0068BDFC _NetEndTurnProceedingToNextTurnLog 0x00529F30
static const char NetEndTurnProceedingToNextTurnLog[] = "--- Proceeding to next turn ---";

// RECOVERY_OBJECT: v2 0x0068BDFC 0x0068BE15 _NetEndTurnSaveDirectory 0x00529F30
static const char NetEndTurnSaveDirectory[] = "k:\\breynolds\\multisaves\\";

// RECOVERY_OBJECT: v2 0x0068BE18 0x0068BE1B _NetEndTurnSaveGamePrefix 0x00529F30
static const char NetEndTurnSaveGamePrefix[] = "AC";

// RECOVERY_OBJECT: v2 0x0068BE1C 0x0068BE20 _NetEndTurnSaveGameSuffix 0x00529F30
static const char NetEndTurnSaveGameSuffix[] = "end";

// RECOVERY_OBJECT: v2 0x0068BE20 0x0068BE32 _NetEndTurnSaveFileLog 0x00529F30
static const char NetEndTurnSaveFileLog[] = "Writing save file";

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

    log_say(NetEndTurnEnterEndTurnLog,
            static_cast<int>(NetDaemonNet.factions_waiting_mask()),
            NetDaemonNet.channels_[0].leading_words_[0],
            GameDataStateStorage.fields_.game_state_ & 2u);
    g_FX.play(0x22);
    GameDataStateStorage.fields_.game_state_ |= 2u;
    ConsoleGlobal.field_23BE4_ = 0;
    ConsoleGlobal.set_view(0);

    const int local_faction = ConsoleGlobal.faction_id_;
    if (NetDaemonNet.primary_state_.leading_words_[0] == 0) {
        NetDaemonNet.factions_waiting_mask() |= 1u << local_faction;
    }
    NetDaemonNet.channels_[0].leading_words_[0] |= 1u << local_faction;
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
                  NetDaemonNet.channels_[0].trailing_words_[0] == 0))) {
                break;
            }
            if (ExitTurnLoop != 0 || (GameDataStateStorage.fields_.game_state_ & 2u) == 0) {
                break;
            }
            if (sent_finishing == 0 &&
                static_cast<DWORD>(timeGetTime() - start_time) > 0x3E8) {
                NetMsgPrimary.pop(NetEndTurnFinishingTurnLabel, 0, 0, nullptr);
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
            log_say(NetEndTurnReadyToProceedLog,
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
    log_say(NetEndTurnProceedingToNextTurnLog, 0, 0, 0);
    NetDaemonNet.primary_state_.trailing_words_[7] = 0;
    NetDaemonNet.channels_[0].leading_words_[1] = 0;
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
        strcat(StringTemp, NetEndTurnSaveDirectory);
        strcat(StringTemp, NetEndTurnSaveGamePrefix);
        char number[80];
        _itoa(ConsoleGlobal.faction_id_, number, 10);
        strcat(StringTemp, number);
        strcat(StringTemp, NetEndTurnSaveGameSuffix);
        _itoa(GameDataStateStorage.fields_.turn_current_num_, number, 10);
        strcat(StringTemp, number);
        log_say(NetEndTurnSaveFileLog, StringTemp, 0, 0, 0);
        save_daemon(StringTemp);
    }
    NetDaemonNet.deletion_list_.advance_sequence();
}

// RECOVERY_OBJECT: v2 0x0068BE78 0x0068BE92 _NetControlTurnClearTurnSemaphore 0x0052A2E0
static const char NetControlTurnClearTurnSemaphore[] = "(Clearing turn semaphore)";

// RECOVERY_OBJECT: v2 0x0068BE34 0x0068BE40 _NetControlTurnMultiDebugPreference 0x0052A2E0
static const char NetControlTurnMultiDebugPreference[] = "Multi Debug";

// RECOVERY_OBJECT: v2 0x0068BE40 0x0068BE4B _NetControlTurnMultiDebugPopup 0x0052A2E0
static const char NetControlTurnMultiDebugPopup[] = "MULTIDEBUG";

// RECOVERY_OBJECT: v2 0x0068BE4C 0x0068BE5D _NetControlTurnTurnLogDirectory 0x0052A2E0
static const char NetControlTurnTurnLogDirectory[] = "k:\\breynolds\\log";

// RECOVERY_OBJECT: v2 0x0068BE60 0x0068BE65 _NetControlTurnTextFileSuffix 0x0052A2E0
static const char NetControlTurnTextFileSuffix[] = ".txt";
// RECOVERY_OBJECT: v2 0x0068BE70 0x0068BE75 _NetControlTurnFallbackTextFileSuffix 0x0052A2E0
static const char NetControlTurnFallbackTextFileSuffix[] = ".txt";
// RECOVERY_OBJECT: v2 0x0068BE6C 0x0068BE70 _NetControlTurnFallbackLogPrefix 0x0052A2E0
static const char NetControlTurnFallbackLogPrefix[] = "log";

// RECOVERY_OBJECT: v2 0x0068BE68 0x0068BE6B _NetControlTurnTextFileMode 0x0052A2E0
static const char NetControlTurnTextFileMode[] = "wt";

// ORIGINAL: 0x0052A2E0 ?net_control_turn@@YAXXZ 0x0052A2E0-0x0052A971 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical seed:src/unrecovered/0052a2e0.cpp", "task:net-turn-coordinator-owner", "ConsoleGlobal9156B0+23BE4/+23BE8 owns939294/939298; UpgradePromptAnswered90EA3C, TurnRedrawPending703DE0 and CouncilPendingMessageArgs68BF38 already own their storage. Do not create competing aliases.", "Native prototypes64..511 use34-byte records and flags+30/obsolete_factions+2C, not seed1A stride; peer checksums use8x21-word existing storage. Preserve wait/abort ordering and masked PlayerLock active-bit test; extent/lifetime/runtime and retained-link evidence remain bounded."], "reason": "Recover the complete multiplayer turn operation through actual source-owned objects, arrays, members and SDK calls; preserve historical identity, spans and seed discoveries without implying game acceptance."}
// TRIED: faithful port of the turn-control loop (checksum/log setup, the active-faction bitmask scan requiring both 0x9a64e8 and 0x9a64e9 bits set, the base-then-veh center-view fallback search, the human/AI turn dispatch with the net_tasks polling loops, and the two net_end_of_turn continuation checks incl. the dead defensive branch Ghidra also shows). Compiles; diverges at instr #0 on stack-frame size (local declarations spread across nested blocks change the `sub esp` prologue vs the original's single big frame). Not chased further.
void __cdecl net_control_turn() {
    char log_path[80];

    draw_map(1);
    my_srand(MapRandSeed);
    ConsoleControlTurnActive = 1;
    MapBaseDisplayOverride = 1;

    if (prefs_get(NetControlTurnMultiDebugPreference, 0, 0) != 0 &&
        X_pop(NetControlTurnMultiDebugPopup, 0) != 0) {
        StringTemp[0] = 0;
        strcat(StringTemp, NetControlTurnTurnLogDirectory);
        _itoa(ConsoleGlobal.faction_id_, log_path, 10);
        strcat(StringTemp, log_path);
        strcat(StringTemp, NetControlTurnTextFileSuffix);
        FILE *file = fopen(StringTemp, NetControlTurnTextFileMode);
        if (file != 0) {
            fclose(file);
        } else {
            StringTemp[0] = 0;
            strcat(StringTemp, NetControlTurnFallbackLogPrefix);
            _itoa(ConsoleGlobal.faction_id_, log_path, 10);
            strcat(StringTemp, log_path);
            strcat(StringTemp, NetControlTurnFallbackTextFileSuffix);
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
        log_say(NetControlTurnClearTurnSemaphore,
                NetDaemonNet.primary_state_.trailing_words_[6],
                NetDaemonNet.channels_[0].leading_words_[0], 0);
        NetDaemonNet.primary_state_.trailing_words_[6] = 0;
        NetDaemonNet.channels_[0].leading_words_[0] = 0;
        NetDaemonNet.primary_state_.trailing_words_[0] = 1;
        // The adjacent 0x93E968 store is distinct from the message count and
        // precedes it in the image.
        NetDaemonNet.channels_[1].trailing_words_[8] = 0;
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
                clear_multiplayer_endgame_state();
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
// symbol    _directplay_install_is_complete
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
// RECOVERY_LITERAL: v1 0x0068BE94 0x0068BEDA ??_C@_0EG@PBPO@SOFTWARE?2Microsoft?2DirectPlay?2Ap@ 0x0052A9B0
// RECOVERY_LITERAL: v1 0x0068BEDC 0x0068BEE5 ??_C@_08IAPL@Complete?$AA@ 0x0052A9B0
// RECOVERY_LITERAL: v1 0x0068BEE8 0x0068BEEC ??_C@_03PEHN@Yes?$AA@ 0x0052A9B0
extern "C" int __cdecl directplay_install_is_complete() {
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

// Whether the DirectPlay lobby registration records the intro as complete.
static inline int control_game_intro_complete() {
    HKEY hkey;
    unsigned long value_length;
    unsigned long value_type;
    unsigned char value_data[12];
    int is_complete = 0;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                      "SOFTWARE\\Microsoft\\DirectPlay\\Applications\\"
                      "Sid Meier's Alpha Centauri",
                      0, 0, &hkey) == 0) {
        value_length = 10;
        if (RegQueryValueExA(hkey, "Complete", 0, &value_type, value_data,
                             &value_length) == 0
            && _strcmpi((const char *)value_data, "Yes") == 0) {
            is_complete = 1;
        }
        RegCloseKey(hkey);
    }
    return is_complete;
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

    if (system_init() == 0 && game_init(0, 1) == 0) {

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
                              HIBYTE(GetAsyncKeyState(VK_SHIFT)) || control_game_intro_complete());

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
                    // The image loads the primary map-window slot before it stores
                    // the skip flag, then ors the live-flag dword through a register.
                    // Keep the original live volatile dword accesses on the mapped
                    // MapWin field, with the table load before the climate-flag store.
                    MapWin *const primary_window = MapWinTable[0];
                    volatile MapWin *const live_window = primary_window;
                    WorldClimateSkipTerrainClear = 0;
                    uint32_t flags = live_window->field_1DD70_;
                    // The image materializes 0x8C0 into a dead callee-saved
                    // register before the loop and ors through it; every spelling of
                    // that constant this tree has tried folds to the immediate
                    // instead (TRIED: note under the marker).
                    flags |= 0x8C0;
                    live_window->field_1DD70_ = flags;
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
