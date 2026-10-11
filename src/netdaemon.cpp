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
// ORIGINAL: 0x0052DB10 _$E21 0x0052DB10-0x0052DB26 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot 0x00682634 selects init 0x0052DB10-0x0052DB26, registering fini 0x0052DB30-0x0052DB3A; complete independent instruction decode establishes both ends.", "Both pinned operations identify storage 0x0093CD90; scalar NetDaemon constructor/destructor 0x005389F0/0x00538D10.", "Current owner physical COFF selects _$E21 via .CRT$XCU and registers _$E19, with relocations naming ?NetDaemonNet@@3VNetDaemon@@A; the genuine global definition owns this lifetime, including possible inline scalar construction.", "Current helper names are compiler-output interpretations, not original debug symbols. These individual bindings do not establish full initialization order, allocation extents, linked retained selection or runtime acceptance."], "reason": "Bind original startup operation to the genuine compiler-emitted lifetime of source-owned NetDaemonNet; no callback body is authored."}
// ORIGINAL: 0x0052DB30 _$E19 0x0052DB30-0x0052DB3A FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot 0x00682634 selects init 0x0052DB10-0x0052DB26, registering fini 0x0052DB30-0x0052DB3A; complete independent instruction decode establishes both ends.", "Both pinned operations identify storage 0x0093CD90; scalar NetDaemon constructor/destructor 0x005389F0/0x00538D10.", "Current owner physical COFF selects _$E21 via .CRT$XCU and registers _$E19, with relocations naming ?NetDaemonNet@@3VNetDaemon@@A; the genuine global definition owns this lifetime, including possible inline scalar construction.", "Current helper names are compiler-output interpretations, not original debug symbols. These individual bindings do not establish full initialization order, allocation extents, linked retained selection or runtime acceptance."], "reason": "Bind original exit operation to the genuine compiler-emitted lifetime of source-owned NetDaemonNet; no callback body is authored."}
#include "stdafx.h"
#include <mmsystem.h>
#include "original_seam.h"
#include "netdaemon.h"
#include "log.h"  // log_say, source-owned at 0x006262F0
#include "net_class.h"
#include "lock.h"
#include "playerlock.h"

#include "stringstruct.h"  // StringAllocationHeap - the 0x009B3374 hand-off
#include "netmsg.h"        // NetMsgPrimary, which the lock/wait bodies pop through
#include "base.h"
#include "basepop.h"       // NetMsg's base, which process_message reaches
#include "netwin.h"        // NetWinDialog - the setup dialog process_message dispatches into
#include "veh.h"           // Vehs, which game_checksum walks
#include "map.h"           // the map bounds, PlayersData and Bases it sums
#include "technology.h"    // GameTechAchieved
#include "game.h"          // IsMultiplayerNet: the application's shared flag
#include "strings.h"      // StringTemp shared chat buffer
#include "multiwin.h"
#include "diplowin.h"
#include "faction.h"
#include "councwin.h"
#include "mapwin.h"
#include "maininterface.h"
#include "reportif.h"      // report_intel: actual report command owner
#include "console.h"
#include "fx.h"
#include "popup.h"
#include "win.h"
#include "alpha.h"
#include "council.h"
#include "terraforming.h"
#include "path.h"
#include "tutwin.h"
#include "xpops.h"
#include "general.h"
#include "filemap.h"
#include "zlib.h"

// Original mapped file-transfer wrappers, now implemented in this owner TU.
int __cdecl netdaemon_deflate_buffer(unsigned char *, unsigned long *,
                                      unsigned char *, unsigned int);
int __cdecl netdaemon_inflate_buffer(unsigned char *, unsigned long *,
                                      unsigned char *, unsigned int);
#include <direct.h>
#include <new>
#include <cstring>
#include "replaywin.h"     // ReplayEventBytes: replay recorder's byte length

// do_checksums(4), original 51E760, copies 0x54 bytes from its calculated
// vector; net_upkeep 5296E6..5296F4 sends the same 21 dwords. The complete
// original allocation extent is unknown; this is the observed storage window.
// Initial image storage is zero. The producer remains unfinished.
// RECOVERY_OBJECT: v1 0x0093A008 ?NetTurnChecksums@@3PAHA
int NetTurnChecksums[NetTurnChecksumWords];

// Original process_message 53675D..53677C copies 21 payload dwords to the
// message's faction row. net_upkeep compares rows 1..7 with stride 0x54.
// Its 93A56C logging cursor is row 1, word 2, not a separate allocation.
// These zero-initialized rows cover the observed faction domain only; the
// complete original extent and incoming faction validation remain unproven.
// The corresponding dispatcher branch remains an unfinished dependency.
// RECOVERY_OBJECT: v1 0x0093A510 ?NetPeerTurnChecksums@@3PAY0BF@HA
int NetPeerTurnChecksums[NetTurnChecksumPeerSlots][NetTurnChecksumWords];

// The next dword after IsMultiplayerNet. Every sender in this file
// increments it around one send_message call, every wait loop increments it
// for the length of its poll and decrements it on the way out, and
// MultiWin::draw_time (0x00478BE0) declines to draw the turn clock unless it
// is zero - so it counts the network exchanges currently in flight.
// RECOVERY_OBJECT: v2 0x0093F664 0x0093F668 ?NetBusyCount@@3HA 0x00592DE0
int NetBusyCount;  // 0x0093F664
// Independently addressed shared dword. process_message writes the interval
// (1000/5000), and MultiWin::draw_time consumes/decrements it. Adjacency to
// NetBusyCount does not establish a larger allocation or exclusive owner.
// Evidence: pinned 0x00534E3F/0x00534EEF write this word;
// 0x00478C48/0x00478CC3 read/decrement it with a signed positivity test.
// Its original PE .data zero-fill tail gives zero image-load state.
// RECOVERY_OBJECT: v2 0x0093F668 0x0093F66C ?NetDaemonProgressInterval@@3IA 0x00534400
unsigned int NetDaemonProgressInterval;
// Independent zero-initialized DWORD at 0x93F66C. The dispatcher clears it,
// samples timeGetTime when zero, and records the sample; this is separate
// storage from the shared interval at 0x93F668.
// Evidence: pinned 0x00534E45 clears this independently; 0x00534E6D and
// 0x00534EC9 read/sample/write this word. No containing allocation is inferred.
// RECOVERY_OBJECT: v1 0x0093F66C ?NetDaemonProgressLastTick@@3IA
unsigned int NetDaemonProgressLastTick;
// Original 0x0093A960 accumulates the wait durations returned by the daemon's
// map-order callbacks. Other writers/extent are not yet assigned to an owner.
// The pinned loader initializes this observed dword to zero; its containing
// allocation and complete indirect writer set remain unknown.
// RECOVERY_OBJECT: v1 0x0093A960 ?NetDaemonWaitTicks@@3HA
int NetDaemonWaitTicks;

// send_game synchs replay records only when the recorder has bytes to send;
// the counter and buffer are owned together by replaywin.cpp.

// Four bytes below IsMultiplayerNet. on_player_deleted
// (0x0052E370) sets it when a client leaves; a lock whose wait saw it comes
// back with the "NOLOCK" popup instead of a lock.
int NetPlayerDropped;  // 0x0093F65C

// Set as a pair by commlink_attempt (0x00558C60): the flag and the faction
// that asked. process_message tests both when a commlink reply arrives.
int CommlinkPending;   // 0x007492CC
int CommlinkFaction;   // 0x0073B790

// The spock pair: synch (0x00532E00) raises 0x93F670 when it has something to
// notify, check_spock (below) sees it, arms 0x93F674 and calls help_spock
// (0x0051F0C0), which clears both after re-synching the session.
int SpockNotifyPending;  // 0x0093F670
int SpockHelpActive;     // 0x0093F674

extern int BaseAtDebugPopupFlag;  // 0x0093A94C, defined in base.cpp

// The same source storage exported by console.h, without including its
// window hierarchy just for this scalar declaration.
extern int ConsoleControlTurnActive;

// ORIGINAL: 0x00530D50 ?lock_base@NetDaemon@@QAEHHHHH@Z 0x00530D50-0x00530E48
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00530D50-0x00530E48"], "reason": "Recover the base-menu network guard through real Base records, shared turn state and NetDaemon lock-state members; original calls establish argument forwarding and log strings."}
int NetDaemon::lock_base(int base_id, int target, int payload_1C, int payload_20) {
    log_say("LockBase", UnkGlobal0093A934, ConsoleControlTurnActive,
            BaseIDCurrentSelected);
    if (!IsMultiplayerNet) {
        return 0;
    }
    if (UnkGlobal0093A934) {
        Bases[base_id].state |= BSTATE_NET_LOCKED;
        message_data(0x4304, 0, 0, 0, 0, 0);
        return 0;
    }
    if (ConsoleControlTurnActive) {
        return 1;
    }
    if (primary_state_.leading_words_[2]) {
        return 1;
    }
    Base &base = Bases[base_id];
    log_say("LockBase Needs Lock", base_id, base.x, base.y);
    return lock(target | 4, base.x, base.y, 0, payload_1C, payload_20);
}

// MapLongitude is the source-owned half-width tile-row stride, already
// declared by map.h with original binding 0x0068FAF0. game_checksum's
// per-row walk uses its signed value to reach a tile record.

// Derived player lifecycle callbacks. Retained seeds supply historical hints;
// the original missing-player tail and its unresolved index domain are kept.
// ORIGINAL: 0x0052E160 ?on_player_added@NetDaemon@@UAEXPADK@Z 0x0052E160-0x0052E364
// RECOVERY_BINDING: v1 {"evidence": ["Pinned52E160..52E364 and52E370..52E8FD; retained recovered seeds are hints, not mandatory C++.", "The not-found delete tail still runs and decrements count. Slot8 identity/status bytes belong to the real ledger; lobby prefix/name writes are byte views wholly inside the existing168-byte session object. Keep both session object lifetimes and their original bindings.", "PlayerLock::active return controls notification; process identity offsets are not doubled. Two label_get calls remain independent, and prefix writes depend on live phase below2.", "Source-owned NextTurnReady, NetUpkeepPhase and NetHostChangeSlot replace private address aliases. Upkeep producer remains unfinished; ledger-derived lock index domain remains unproven.", "recovery/tasks/netdaemon-player-lifecycle-recovery.json; actual-source host damage controls and native compilation required", "git:293937ab6f3be75999e0c0a4c9c110c02930624c:src/recovered/units/0052e160.cpp", "Historical seed spelling ?on_player_added@NetDaemon@@QAEXPADK@Z; current source mapping ?on_player_added@NetDaemon@@UAEXPADK@Z; emitted symbol ?on_player_added@NetDaemon@@UAEXPADK@Z."], "reason": "Recover the complete derived network player lifecycle callback through actual AlphaNet records, PlayerLock behavior, NetWin storage and shared turn state. Preserve its original identity and useful seed findings; no exact, runtime or full acceptance claim is implied. Superseded seed retired after owner review; original spans, existing claims and useful seed findings remain in source. This cleanup grants no new match or acceptance claim."}
void NetDaemon::on_player_added(char *name, unsigned long id) {
    const bool same_host = (field_760_ == field_764_);
    primary_state_.leading_words_[0] = same_host ? 1 : 0;

    if (player_count_ < 7 && primary_state_.trailing_words_[0] < 2) {
        const int slot = ++player_count_;
        AlphaNetProcessSlot &player = process_slots_[slot];
        player.pid_ = static_cast<uint32_t>(id);
        strcpy(player.name_, get_player_name(id));

        NetWinLobbyRow &row = NetWinLobbyRows[slot];
        row.field_2_ = NetWinSessionOptions.difficulty_;
        row.prefix_ = static_cast<uint8_t>(slot);
        player.identity_ = static_cast<int8_t>(slot);

        if (slot > 1) {
            bool duplicate_host = false;
            for (int prior = 1; prior < slot; ++prior) {
                if (process_slots_[prior].pid_ == field_760_) {
                    duplicate_host = true;
                }
            }
            if (duplicate_host) {
                parse_says(0, name, -1, -1);
                NetMsgPrimary.pop("ADDED", 3000, 0, nullptr);
            }
        }

        if (primary_state_.leading_words_[0] != 0) {
            row.owner_ = 0;
            row.prefix_ = 0xFF;
            row.ready_or_occupied_ = 0;
            row.field_2_ = 0;
            row.faction_or_color_ = -1;
            row.field_4_ = 2;

            row.faction_or_color_ = -1;
            row.prefix_ = static_cast<uint8_t>(slot);
            row.field_2_ = NetWinSessionOptions.difficulty_;
            row.ready_or_occupied_ = 0;

            send_all_player_packets(-1);
            send_session_packet(slot, -1);
            send_session2_packet(slot, -1);
            send_faction_data(slot);
        }

        if (NetWinDialog.is_visible()) {
            NetWinDialog.on_redraw();
        }
    } else if (same_host) {
        uint16_t packet[16];
        packet[0] = 0x1F01;
        Net::send(packet, 0x20, id, 1);
    }
}

// ORIGINAL: 0x0052E370 ?on_player_deleted@NetDaemon@@UAEXPADK@Z 0x0052E370-0x0052E8FD
// RECOVERY_BINDING: v1 {"evidence": ["Pinned52E160..52E364 and52E370..52E8FD; retained recovered seeds are hints, not mandatory C++.", "The not-found delete tail still runs and decrements count. Slot8 identity/status bytes belong to the real ledger; lobby prefix/name writes are byte views wholly inside the existing168-byte session object. Keep both session object lifetimes and their original bindings.", "PlayerLock::active return controls notification; process identity offsets are not doubled. Two label_get calls remain independent, and prefix writes depend on live phase below2.", "Source-owned NextTurnReady, NetUpkeepPhase and NetHostChangeSlot replace private address aliases. Upkeep producer remains unfinished; ledger-derived lock index domain remains unproven.", "recovery/tasks/netdaemon-player-lifecycle-recovery.json; actual-source host damage controls and native compilation required", "git:293937ab6f3be75999e0c0a4c9c110c02930624c:src/recovered/units/0052e370.cpp", "Historical seed spelling ?on_player_deleted@NetDaemon@@QAEXPADK@Z; current source mapping ?on_player_deleted@NetDaemon@@UAEXPADK@Z; emitted symbol ?on_player_deleted@NetDaemon@@UAEXPADK@Z."], "reason": "Recover the complete derived network player lifecycle callback through actual AlphaNet records, PlayerLock behavior, NetWin storage and shared turn state. Preserve its original identity and useful seed findings; no exact, runtime or full acceptance claim is implied. Superseded seed retired after owner review; original spans, existing claims and useful seed findings remain in source. This cleanup grants no new match or acceptance claim."}
void NetDaemon::on_player_deleted(char *, unsigned long id) {
    parse_says(0, const_cast<char *>("ED"), -1, -1);

    const bool server_left = (NetHostChangeSlot == static_cast<uint32_t>(id));
    if (server_left) {
        channels_[1].trailing_words_[6] = 1;
    }

    int slot = 1;
    while (slot <= player_count_ && process_slots_[slot].pid_ != id) {
        ++slot;
    }
    const bool found = slot <= player_count_;

    if (found) {
        uint32_t &ready_players = primary_state_.trailing_words_[4];
        if (ready_players != 0) {
            ready_players &= ~(1u << (slot & 31));
            if (ready_players == 0) {
                g_FX.play(0x15);
            }
        }

        AlphaNetProcessSlot &player = process_slots_[slot];
        parse_says(0, player.name_, -1, -1);
        const int player_id = player.identity_;

        if (primary_state_.trailing_words_[0] == 2) {
            uint8_t humans = static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 0xFF);
            humans = static_cast<uint8_t>(humans &
                ~static_cast<uint8_t>(1u << (player_id & 31)));
            GameDataStateStorage.fields_.lock_masks_.enable_mask_ = (GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 0xFFFFFF00u) | humans;
        }
        channels_[1].payload_.flags &= ~(1u << (player_id & 31));

        const bool turn_handoff = (GameDataStateStorage.fields_.net_turn_flags_ & 0x10) != 0;
        if (turn_handoff && primary_state_.leading_words_[0] != 0 &&
            GameDataStateStorage.fields_.net_turn_faction_ == player_id) {
            const uint8_t alive = static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_ >> 8);
            const uint8_t humans = static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_);
            bool informed = false;
            for (int next = player_id + 1; next < 8; ++next) {
                const uint8_t bit = static_cast<uint8_t>(1u << (next & 31));
                if ((alive & bit) != 0 && (humans & bit) != 0) {
                    message_data(0x4309, 0, next, 0, 0, 0);
                    informed = true;
                    break;
                }
            }
            if (!informed) {
                NextTurnReady = 1;
            }
        }
    }

    // The original deliberately shares this tail with the not-found path.
    // With seven players a miss yields slot 8: the next identity byte is the
    // low byte of AlphaNetLedger::session_flags_ at +0x1470, not an array
    // member. This typed branch preserves that exact byte without OOB access.
    const int player_id = slot < 8
        ? static_cast<int>(process_slots_[slot].identity_)
        : static_cast<int>(static_cast<int8_t>(
              static_cast<uint8_t>(ledger_.session_flags_ & 0xFF)));

    // Native uses this signed identity as an index without checking its
    // range. The missing-player ledger byte has no proven valid domain.
    locks_.records_[player_id].clear();
    uint32_t &faction_lock_mask = channels_[0].trailing_words_[0];
    const uint32_t player_lock_mask =
        channels_[0].trailing_words_[1 + player_id];
    faction_lock_mask &= ~player_lock_mask;
    if (static_cast<int>(locks_.field_E0_) == player_id) {
        locks_.field_E0_ = 0;
        locks_.field_E4_ = 0;
    }

    const uint8_t turn_bit = static_cast<uint8_t>(GameDataStateStorage.fields_.net_turn_flags_ & 0x10);
    const bool skip_lock_notify =
        (turn_bit != 0 && ConsoleControlTurnActive == 0 &&
         GameDataStateStorage.fields_.net_turn_faction_ == ConsoleGlobal.faction_id_) ||
        (turn_bit == 0 && primary_state_.leading_words_[0] != 0);

    if (!skip_lock_notify && primary_state_.trailing_words_[0] == 2) {
        const uint8_t alive = static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_ >> 8);
        const uint8_t humans = static_cast<uint8_t>(GameDataStateStorage.fields_.lock_masks_.enable_mask_);
        for (int faction = 1; faction < 8; ++faction) {
            const uint8_t bit = static_cast<uint8_t>(1u << (faction & 31));
            PlayerLock &candidate = locks_.records_[faction];
            if ((alive & bit) == 0 || (humans & bit) == 0 ||
                faction == player_id || candidate.active() == 0 ||
                (candidate.active_ & 1) != 0) {
                continue;
            }

            candidate.active_ = static_cast<uint8_t>(
                candidate.active_ | static_cast<uint8_t>(1u << (slot & 31)));
            if ((candidate.active_ & humans) == humans) {
                log_say("SPECIAL Sending EXEC DONE to", faction, 0, 0);
                message_data(0x1206, faction, 0, 0, 0, 0);
                candidate.active_ = static_cast<uint8_t>(candidate.active_ | 1);
            }
        }
    }

    log_say(ParseStrBuffer[0].str, "Dropped *************************",
            static_cast<int>(id), slot, 0);

    if (slot < 8) {
        AlphaNetProcessSlot &removed_player = process_slots_[slot];
        removed_player.identity_ = -1;
        removed_player.field_25_ = 0;
        removed_player.field_26_ = 0;
        removed_player.field_27_ = -1;
        removed_player.status_bytes_[0] = 2;
    } else {
        // Native row 8 begins at the ledger's session_flags_ byte (+0x1470)
        // and its following byte at max_players_ (+0x1474).
        ledger_.session_flags_ = 0xFF0000FFu;
        ledger_.max_players_ = (ledger_.max_players_ & 0xFFFFFF00u) | 2u;
    }

    if (slot == 8) {
        if (primary_state_.trailing_words_[0] < 2) {
            NetWinSessionData.write_row8_prefix(0xFF);
        }
        strcpy(NetWinSessionData.row8_leader_name(), label_get(0xFE4 / 4));
        // Separate native Strings::get calls reload the owner and index.
        strcpy(NetWinSessionData.row8_noun_faction(), label_get(0xFE4 / 4));
    } else {
        NetWinLobbyRow &removed_row = NetWinLobbyRows[slot];
        if (primary_state_.trailing_words_[0] < 2) {
            removed_row.prefix_ = 0xFF;
        }
        strcpy(removed_row.leader_name_, label_get(0xFE4 / 4));
        strcpy(removed_row.noun_faction_, label_get(0xFE4 / 4));
    }

    const int last_slot = player_count_ - 1;
    for (int moving = slot; moving <= last_slot; ++moving) {
        process_slots_[moving] = process_slots_[moving + 1];
        NetWinLobbyRows[moving] = NetWinLobbyRows[moving + 1];
        NetWinLobbyRows[moving].prefix_ = static_cast<uint8_t>(moving);
    }

    --player_count_;
    primary_state_.leading_words_[0] = (field_760_ == field_764_) ? 1 : 0;

    const uint8_t post_turn_bit = static_cast<uint8_t>(GameDataStateStorage.fields_.net_turn_flags_ & 0x10);
    const bool skip_after_delete =
        (post_turn_bit == 0 || ConsoleControlTurnActive != 0 ||
         GameDataStateStorage.fields_.net_turn_faction_ != ConsoleGlobal.faction_id_) &&
        ((post_turn_bit != 0 && ConsoleControlTurnActive == 0) ||
         primary_state_.leading_words_[0] == 0);

    if (skip_after_delete && primary_state_.trailing_words_[0] == 2) {
        if (post_turn_bit == 0 &&
            (primary_state_.trailing_words_[7] &
             (1u << (ConsoleGlobal.faction_id_ & 31))) != 0) {
            message_data(0x8301, 0, 0, 0, 0, 0);
        }
        // The original reloads this global after the 0x8301 callback.
        if (ConsoleControlTurnActive != 0) {
            message_data(0x2303, 0, NetUpkeepPhase,
                         GameDataStateStorage.fields_.turn_current_num_, 0, 0);
        }
    }

    const char *popup_text;
    if (server_left) {
        popup_text = NetWinDialog.is_visible() != 0
            ? "SERVERLOSTINSETUP" : "SERVERLOST";
    } else {
        popup_text = "DROPPED";
    }
    NetMsgPrimary.pop(popup_text, 3000, 2, nullptr);

    if (server_left) {
        if (NetWinDialog.is_visible() != 0) {
            NetWinDialog.on_button_clicked(-2);
        }
        channels_[1].leading_words_[3] = 0;
        channels_[1].trailing_words_[0] = 0;
        channels_[1].leading_words_[1] = 0;
        channels_[0].leading_words_[2] = 2;
        NetPlayerDropped = 1;
        locks_.field_E8_ = 1;
    }

    NetWinDialog.on_redraw();

    int server_slot = 0;
    while (server_slot < 8 &&
           process_slots_[server_slot].pid_ != field_764_) {
        ++server_slot;
    }
    const char *server_name = server_slot < 8
        ? process_slots_[server_slot].name_
        : reinterpret_cast<const char *>(&ledger_);
    log_say(server_name, "is now the Server.", 0, 0, 0);
    primary_state_.leading_words_[2] = 0;
}

/*
Purpose: Build a network daemon - its AlphaNet base, then its own eight
         PlayerLocks and 0x18-entry lock table.
// ORIGINAL: 0x005389F0 ??0NetDaemon@@QAE@XZ 0x005389F0-0x00538B58;0x0065DF50-0x0065DF62
// size      360 bytes
// prototype void (__thiscall ??0NetDaemon@@QAE@XZ)(NetDaemon* this)
// callers   1   call targets   4
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004E2490 0x0058FF70 0x005900D0 0x004E25B0
Return Value: this

NetDaemon has no virtual base anywhere in this chain, so a genuine
constructor is safe (unlike MapWin/PlanWin/Console - see the note in
`mapwin.h`), and the image's own SEH frame here is what a real constructor
with a real destructible base earns. NetDaemon's own fields past AlphaNet
are still opaque storage (see the class declaration), so the eight
PlayerLocks and the 0x18-entry lock table reach their elements by raw
offset rather than through invented array members.
*/
NetDaemon::NetDaemon() : AlphaNet(), deletion_list_(DeletionList::Uninitialized()) {
    // The eight per-player locks (0x14A0, 0x1C stride), then the embedded
    // Lock of the first one cleared directly - the image calls the Lock
    // version explicitly, twice (here and again after the table below).
    for (int32_t n = 0; n < 8; ++n) {
        locks_.records_[n].clear();
    }
    locks_.clear();

    // Native 0x538A53 clears the shared deletion owner once; the backedge
    // at 0x538A64 targets 0x538A59, after this store.
    deletion_list_.current_owner_ = 0;
    for (int32_t m = 0; m < 0x18; ++m) {
        deletion_list_.records_[m].acknowledgements_ = 0xFF;
        deletion_list_.records_[m].count_ = 0;
    }

    primary_state_.leading_words_[0] = 0;
    primary_state_.leading_words_[1] = 0;
    primary_state_.leading_words_[2] = 0;
    primary_state_.flags_ = 0;
    primary_state_.trailing_words_[8] = 0;
    primary_state_.trailing_words_[7] = 0;
    channels_[0].leading_words_[1] = 0;
    channels_[0].leading_words_[0] = 0;
    channels_[0].payload_.vehicle_selection_follow = 0;
    channels_[1].leading_words_[1] = 0;
    channels_[0].trailing_words_[9] = 0;
    channels_[1].leading_words_[0] = 0;
    channels_[1].trailing_words_[4] = 0;
    channels_[1].trailing_words_[3] = 0;
    primary_state_.trailing_words_[4] = 0;
    primary_state_.trailing_words_[5] = 0;
    primary_state_.trailing_words_[6] = 0;
    channels_[1].trailing_words_[1] = 0;
    channels_[1].trailing_words_[0] = 0;
    channels_[1].leading_words_[3] = 0;
    channels_[1].leading_words_[2] = 0;
    channels_[1].payload_.flags = 0;
    primary_state_.trailing_words_[0] = 0;
    primary_state_.trailing_words_[1] = 0;
    channels_[1].trailing_words_[7] = 0;
    channels_[1].trailing_words_[6] = 0;
    primary_state_.trailing_words_[2] = 0;
    primary_state_.trailing_words_[3] = 0;
    channels_[0].leading_words_[3] = 0;

    // 0x1BD0 is lock_state_[2]'s eleventh dword - the ctor sets it to 0xFF.
    channels_[1].trailing_words_[6] = 0xFF;
    for (int32_t i = 0; i < 8; i++) {
        channels_[0].trailing_words_[i] = 0;
    }

    locks_.clear();
    AlphaNet::close();
}

// ORIGINAL: 0x00592DE0 sub_592de0 0x00592DE0-0x00592E0E BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["task:short-network-popup-owner", "historical seed:src/recovered/00592de0.cpp", "pinned:592DE0 increment/send/decrement;51DCC0 two stack arguments and masked x86 SHL count with set/clear behavior", "source:existing NetBusyCount, NetDaemonNet.send_message and PopupWriteCheckState; no new storage or executable calls"], "reason": "Recover the original helper in its existing typed storage owner, preserving historical identity, complete span and byte obligation; descriptive source name is not original debug evidence."}
// symbol    ?netdaemon_send_message_counted@@YAXPADKH@Z
void __cdecl netdaemon_send_message_counted(char *message,
                                          unsigned long length,
                                          int target) {
    ++NetBusyCount;
    NetDaemonNet.send_message(message, length, target);
    --NetBusyCount;
}

// ORIGINAL: 0x00593370 sub_593370 0x00593370-0x0059338C BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:5167ec56aaa9031de73f42c05fbc3bdd0bad0201:src/recovered/00593370.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x593370 complete original body; independent source-owner review.", "source:src/netdaemon.cpp and corresponding header own real receiver/dependencies; count-zero copy underflow is intentionally unchanged."], "reason": "Complete typed application helper; retain original call ordering, argument contract and historical obligation."}
// symbol    ?sub_593370@@YAXXZ
void __cdecl sub_593370() {
    NetDaemonNet.synch(1, 0, 0, 0, 0, 1, 0x2101);
}

// ORIGINAL: 0x00593480 sub_593480 0x00593480-0x005934A2 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:5167ec56aaa9031de73f42c05fbc3bdd0bad0201:src/recovered/00593480.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x593480 complete original body; independent source-owner review.", "source:src/netdaemon.cpp and corresponding header own real receiver/dependencies; count-zero copy underflow is intentionally unchanged."], "reason": "Complete typed application helper; retain original call ordering, argument contract and historical obligation."}
// symbol    ?sub_593480@@YAXH@Z
void __cdecl sub_593480(int value) {
    NetDaemonNet.synch(0x17, value, 0, 0, 0, 1, 0x2101);
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


/*
Purpose: Tear down a network daemon through its lock members and the
         AlphaNet/Net destructor chain.
// ORIGINAL: 0x00538D10 ??1NetDaemon@@QAE@XZ 0x00538D10-0x00538EBE;0x004E3710-0x004E372B;0x0065E010-0x0065E0A5
// TRIED: 15/97 with 62 compiled instructions, and the gap is the class, not
//   the spelling. TWO things measured 2026-08-22. (1) call_diff says MORE: this
//   tree makes 13 direct calls where the image makes 12, and the extra one is
//   `??1Net@@QAE@XZ` - NOT written in this body. `NetDaemon : public AlphaNet :
//   public Net`, so a REAL `~NetDaemon()` emits the implicit base-destructor
//   chain; VC6 inlines `~AlphaNet` and calls `~Net` out of line, and the image
//   calls neither, having open-coded both bases' teardown as the field clears
//   and vtable resets this body writes by hand. Under /O1 the count is 14,
//   the fourteenth being `__EH_prolog`. (2) The image's body is a genuine C++
//   unwind state machine: `mov byte ptr [ebp - 4], N` steps through states
//   2,1,3,0,9,0xa,8,7,6,5,4,-1 between the calls, and reserves `sub esp, 8`
//   where this tree reserves `push ecx`. Those state writes come from member
//   and base subobjects the compiler is destroying itself, one protected
//   region per subobject - explicit `->~Shim()` calls on reinterpret_cast
//   pointers, which is what this body has, create no unwind states at all, so
//   no spelling of the current shim scaffolding can produce them. Reaching
//   this needs Net/AlphaNet/NetDaemon modelled with the real members at 0x58,
//   0xb0, 0xe8, 0x10c, 0x130, 0x72c, 0x748, 0x144c, 0x148c - the same layout
//   job the class comment above says is still open - not another pass over the
//   statements.
// size      430 bytes
// prototype void (__thiscall ??1NetDaemon@@QAE@XZ)(NetDaemon* this)
// callers   0   call targets   9
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00401BE0 0x00402970 0x00402DD0 0x004C8DB0 0x004E25B0 0x00538EC0 0x005D45B0 0x0062E010 0x006339E0
Return Value: n/a

The retained seed includes shared cleanup spans. Their ownership and exact
matching remain unfinished; shared code is not grounds to exclude this body.
The TRIED note above describes the former hand-staged candidate, not current
measurements. Ordinary bases and members now own destruction; the remaining
list-cleanup bridges live in those owners and still require recovery.
*/
NetDaemon::~NetDaemon() {
    // AlphaNet owns transport close, Heap and ledger destruction; Net owns
    // its remaining list, queue and voice cleanup. Do not repeat base lifetimes.
}

/*
Purpose: Poll the network for one message; dispatch it when one arrives and
         report whether it did. The message and its two parameters come back
         from Net::get through out-parameters.
// ORIGINAL: 0x00530320 ?receive@NetDaemon@@QAEHXZ 0x00530320-0x0053035E BYTE_EXACT
// size      62 bytes
// prototype int (__thiscall ?receive@NetDaemon@@QAEHXZ)(NetDaemon* this)
// callers   9   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00534400 0x00630A00
Return Value: 1 when a message was received and dispatched, 0 when none was
Status: Complete
Verification note: the original leaves both out-params as uninitialized stack
garbage for Net::get to fill and reads them only after a nonzero return, so
leaving them uninitialized here is equivalent by construction - Net::get
always sets both before this reads them on the taken path.
*/
int NetDaemon::receive() {
    unsigned long first;
    unsigned long second;
    void *result = NetDaemonNet.Net::get(&first, &second);
    if (result == 0) {
        return 0;
    }
    process_message(static_cast<char *>(result), second, static_cast<int>(first));
    return 1;
}


/*
Purpose: Announce a vehicle's synched state to the network. All four "no
         text, one repeated flag" parameters are fixed by the caller, so only
         the opcode and the id vary across this family.
// ORIGINAL: 0x00593220 ?synch_veh@@YAXH@Z 0x00593220-0x00593242 BYTE_EXACT
// size      34 bytes
// prototype void (__cdecl ?synch_veh@@YAXH@Z)(int vehID)
// callers   32   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00532E00
Return Value: none
Status: Complete
*/
void __cdecl synch_veh(int id) {
    NetDaemonNet.synch(0x11, id, 0, 0, nullptr, 1, 0x2101);
}

/*
Purpose: Announce a base's synched state to the network.
// ORIGINAL: 0x00593250 ?synch_base@@YAXH@Z 0x00593250-0x00593272 BYTE_EXACT
// size      34 bytes
// prototype 
// callers   6   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00532E00
Return Value: none
Status: Complete
*/
void __cdecl synch_base(int id) {
    NetDaemonNet.synch(0x13, id, 0, 0, nullptr, 1, 0x2101);
}

/*
Purpose: Announce a faction's energy synched state to the network.
// ORIGINAL: 0x00593280 ?synch_energy@@YAXH@Z 0x00593280-0x005932A2 BYTE_EXACT
// size      34 bytes
// prototype 
// callers   3   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00532E00
Return Value: none
Status: Complete
*/
void __cdecl synch_energy(int id) {
    NetDaemonNet.synch(0xB, id, 0, 0, nullptr, 1, 0x2101);
}

/*
Purpose: Announce a faction's current research target to the network.
// ORIGINAL: 0x005932B0 ?synch_researching@@YAXH@Z 0x005932B0-0x005932D2 BYTE_EXACT
// size      34 bytes
// prototype void (__cdecl ?synch_researching@@YAXH@Z)(int factionID)
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00532E00
Return Value: none
Status: Complete
*/
void __cdecl synch_researching(int id) {
    NetDaemonNet.synch(0xA, id, 0, 0, nullptr, 1, 0x2101);
}

/*
Purpose: Announce a faction's leader to the network.
// ORIGINAL: 0x005932E0 ?synch_leader@@YAXH@Z 0x005932E0-0x00593302 BYTE_EXACT
// size      34 bytes
// prototype 
// callers   3   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00532E00
Return Value: none
Status: Complete
*/
void __cdecl synch_leader(int id) {
    NetDaemonNet.synch(0x6, id, 0, 0, nullptr, 1, 0x2101);
}

/*
Purpose: Announce a faction's AI state to the network.
// ORIGINAL: 0x00593310 ?synch_ai@@YAXH@Z 0x00593310-0x00593332 BYTE_EXACT
// size      34 bytes
// prototype 
// callers   3   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00532E00
Return Value: none
Status: Complete
*/
void __cdecl synch_ai(int id) {
    NetDaemonNet.synch(0x8, id, 0, 0, nullptr, 1, 0x2101);
}

/*
Purpose: Announce a faction's completed research to the network.
// ORIGINAL: 0x00593340 ?synch_research@@YAXH@Z 0x00593340-0x00593362 BYTE_EXACT
// size      34 bytes
// prototype 
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00532E00
Return Value: none
Status: Complete
*/
void __cdecl synch_research(int id) {
    NetDaemonNet.synch(0x9, id, 0, 0, nullptr, 1, 0x2101);
}

/*
Purpose: Announce a faction's social/resource allocation state to the
         network.
// ORIGINAL: 0x00593390 ?synch_alloc@@YAXH@Z 0x00593390-0x005933B2 BYTE_EXACT
// size      34 bytes
// prototype 
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00532E00
Return Value: none
Status: Complete
*/
void __cdecl synch_alloc(int id) {
    NetDaemonNet.synch(0xC, id, 0, 0, nullptr, 1, 0x2101);
}

/*
Purpose: Announce a faction's social engineering state to the network.
// ORIGINAL: 0x005933C0 ?synch_soc@@YAXH@Z 0x005933C0-0x005933E2 BYTE_EXACT
// size      34 bytes
// prototype 
// callers   2   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00532E00
Return Value: none
Status: Complete
*/
void __cdecl synch_soc(int id) {
    NetDaemonNet.synch(0xD, id, 0, 0, nullptr, 1, 0x2101);
}

/*
Purpose: Announce a faction's protocol/pact state to the network.
// ORIGINAL: 0x005933F0 ?synch_proto@@YAXH@Z 0x005933F0-0x00593412 BYTE_EXACT
// size      34 bytes
// prototype 
// callers   4   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00532E00
Return Value: none
Status: Complete
*/
void __cdecl synch_proto(int id) {
    NetDaemonNet.synch(0xE, id, 0, 0, nullptr, 1, 0x2101);
}

/*
Purpose: Announce an observer's state to the network.
// ORIGINAL: 0x00593420 ?synch_obs@@YAXH@Z 0x00593420-0x00593442 BYTE_EXACT
// size      34 bytes
// prototype 
// callers   3   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00532E00
Return Value: none
Status: Complete
*/
void __cdecl synch_obs(int id) {
    NetDaemonNet.synch(0x10, id, 0, 0, nullptr, 1, 0x2101);
}

/*
Purpose: Announce a diplomatic state change between two factions to the
         network. Unlike the rest of this family, both leading arguments
         come from the caller instead of the second being a fixed zero.
// ORIGINAL: 0x00593450 ?synch_diplo@@YAXHH@Z 0x00593450-0x00593474 BYTE_EXACT
// size      36 bytes
// prototype void (__cdecl ?synch_diplo@@YAXHH@Z)(int factionID, int)
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00532E00
Return Value: none
Status: Complete
*/
void __cdecl synch_diplo(int a, int b) {
    NetDaemonNet.synch(0x16, a, b, 0, nullptr, 1, 0x2101);
}

/*
Purpose: Announce a faction's template/design state to the network.
// ORIGINAL: 0x005934B0 ?synch_template@@YAXH@Z 0x005934B0-0x005934D2 BYTE_EXACT
// size      34 bytes
// prototype 
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00532E00
Return Value: none
Status: Complete
*/
void __cdecl synch_template(int id) {
    NetDaemonNet.synch(0x18, id, 0, 0, nullptr, 1, 0x2101);
}

/*
Purpose: Announce a faction's sensor/territory radius state to the network.
// ORIGINAL: 0x005934E0 ?synch_radius@@YAXH@Z 0x005934E0-0x00593502 BYTE_EXACT
// size      34 bytes
// prototype 
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00532E00
Return Value: none
Status: Complete
*/
void __cdecl synch_radius(int id) {
    NetDaemonNet.synch(0x23, id, 0, 0, nullptr, 1, 0x2101);
}

/*
Purpose: Release the vehicle lock this client is holding. In a net game, tell
         the log and the other players first - log_say names the faction that
         is releasing and message_data broadcasts event 0x2212 - then clear the
         four announce-side fields; in every game, clear the locked vehicle
         record at 0x1B78 and the flag at 0x1BC4. Only the transport flag at
         0x0093F660 gates the announce; any nonzero value is a net game.
// ORIGINAL: 0x005310F0 ?unlock_veh@NetDaemon@@QAEXXZ 0x005310F0-0x0053114A BYTE_EXACT
// symbol    ?unlock_veh@NetDaemon@@QAEIXZ
// size      90 bytes
// prototype void (__thiscall ?unlock_veh@NetDaemon@@QAEXXZ)(NetDaemon* this)
// callers   33   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00592EE0 0x006262F0
// LEVER: the six raw `object[0x1B.. / 4]` stores came out and the named
//        lock_state_ members went in - 0x1BAC/0x1BB0/0x1BC8/0x1BCC are
//        lock_state_[2].dword_[2/3/9/10], 0x1B78 is lock_state_[1].dword_[4]
//        and 0x1BC4 is lock_state_[2].dword_[8]. BYTE_EXACT held, so the
//        member run is pinned by a body that was already exact, and the
//        `reinterpret_cast<volatile uint32_t *>(this)` the raw spelling
//        needed is gone with it.
Return Value: EAX residue. The original is ?unlock_veh@NetDaemon@@QAEXXZ, but
              EAX is live at its ret: on the non-net path it is the loaded
              transport flag, which is zero exactly because that path was
              taken; on the net path it is message_data's residue. Modelled as
              uint32_t and returned, like GraphicWin::close and ListBox::close.
Status: Complete with temporary message_data original dependency
*/
uint32_t NetDaemon::unlock_veh() {
    // Loaded exactly once, and it is also the EAX the non-net path returns.
    uint32_t residue = static_cast<uint32_t>(IsMultiplayerNet);
    if (residue != 0) {
        // `cmp eax, edi` against a zeroed edi: any nonzero flag announces.
        log_say("Client releasing lock", ConsoleGlobal.faction_id_, 0, 0);
        residue = message_data(0x2212, 0, 0, 0, 0, 0);
        // The four announce-side fields, in the original's deliberately
        // non-monotonic order: 1BB0, 1BAC, 1BCC, 1BC8.
        channels_[1].leading_words_[3] = 0;
        channels_[1].leading_words_[2] = 0;
        channels_[1].trailing_words_[5] = 0;
        channels_[1].trailing_words_[4] = 0;
    }
    // Shared tail: 0x1B78 is the vehicle record lock_veh (0x00531020) stores
    // and 0x1BC4 its held flag.
    channels_[0].payload_.vehicle_selection_follow = 0;
    channels_[1].trailing_words_[3] = 0;
    return residue;
}


// The daemon's Net, 0x0093CD90 in the image - see netdaemon.h. The image's
// dynamic initializer at 0x0052DB10 constructs it at the fixed address
// before WinMain; the recovered NetDaemon constructor here runs through this
// build's own startup instead.
// RECOVERY_OBJECT: v3 0x0093E8EC ?ConsoleFactionsWaitingMask@@3HA member ?NetDaemonNet@@3VNetDaemon@@A primary_state_.trailing_words_[7]
// RECOVERY_OBJECT: v3 0x0093E964 0x0093E968 ?MessageWinNetworkMessageCount@@3HA 0x004713B0 member ?NetDaemonNet@@3VNetDaemon@@A channels_[1].trailing_words_[7]
// RECOVERY_OBJECT: v1 0x0093CD90 ?NetDaemonNet@@3VNetDaemon@@A
NetDaemon NetDaemonNet;  // 0x0093CD90


/*
Purpose: Release the global diplomacy lock this client holds. In a net game,
         say so on the log and broadcast event 0x2212, then clear the four
         announce-side fields; out of one there is nothing to release and the
         body does nothing at all.
// ORIGINAL: 0x00530D00 ?unlock@NetDaemon@@QAEXXZ 0x00530D00-0x00530D4E BYTE_EXACT
// size      54 bytes
// prototype void (__thiscall ?unlock@NetDaemon@@QAEXXZ)(NetDaemon* this)
// callers   10   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00592EE0 0x006262F0
// Homed 2026-09-03 out of the recovered-units archive: faction.cpp's
// diplo_unlock calls it, and a pending_bodies forwarder would have landmined
// this address's archive claim.
// RESIDUAL: the annotation's span is SHORT, and it is the only reason this is
//        not BYTE_EXACT. 0x00530D00-0x00530D4E (54 bytes) stops after the
//        third field store. The shipped bytes run
//          89 be b0 1b 00 00 / 89 be ac 1b 00 00 / 89 be cc 1b 00 00
//          89 be c8 1b 00 00 / 5f 5e c3
//        from 0x00530D35, so the function really ends at 0x00530D3C - 28
//        instructions, which is exactly what this tree compiles, and all 21
//        the annotation carries agree. Demoted for the span, not the body.
Status: Complete
*/
void NetDaemon::unlock() {
    // The four announce-side fields sit inside lock_state_[2] - 0x1BAC, 0x1BB0,
    // 0x1BC8 and 0x1BCC are that entry's dword_[2], dword_[3], dword_[9] and
    // dword_[10]. Named through the declared member rather than a raw offset.
    if (IsMultiplayerNet != 0) {
        log_say("Client releasing lock", ConsoleGlobal.faction_id_, 0, 0);
        message_data(0x2212, 0, 0, 0, 0, 0);
        channels_[1].leading_words_[3] = 0;
        channels_[1].leading_words_[2] = 0;
        channels_[1].trailing_words_[5] = 0;
        channels_[1].trailing_words_[4] = 0;
    }
}

/*
Purpose: Tell the other clients this client is leaving the diplomacy channel.
// ORIGINAL: 0x00530600 ?close_channel@NetDaemon@@QAEXXZ 0x00530600-0x0053062F BYTE_EXACT
// size      47 bytes
// prototype void (__thiscall ?close_channel@NetDaemon@@QAEXXZ)(NetDaemon* this)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00592EE0
// Homed 2026-09-03 out of the recovered archive for the same reason.
Status: Complete
*/
void NetDaemon::close_channel() {
    if (IsMultiplayerNet != 0) {
        message_data(0x8241, 0, 0, 0, 0, 0);
        // 0x1BA0 is lock_state_[1].dword_[14].
        channels_[0].trailing_words_[9] = 0;
    }
}


// ORIGINAL: 0x0052E050 ?hang_up@NetDaemon@@QAEXXZ 0x0052E050-0x0052E151 FILE
// size      257 bytes
// prototype void (__thiscall ?hang_up@NetDaemon@@QAEXXZ)(NetDaemon* this)
// callers   9   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0047A7D0 0x0058FF70 0x00592EE0
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0052E050
// measured tier  MISMATCH
// divergence     16
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0052e050/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?hang_up@NetDaemon@@QAEXXZ  at 0x0052E050  (257 bytes)
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


// 0x1B3C is lock_state_[0].dword_[4], 0x1580/0x1584 are the Lock's tail
// dwords, 0x1B7C is lock_state_[1].dword_[5] and the four dwords at 0x1B80
// are lock_state_[1].dword_[6..9] - one per faction.
void NetDaemon::hang_up() {
    if (primary_state_.flags_ != 0) {
        return;
    }
    if (IsMultiplayerNet == 0) {
        return;
    }
    primary_state_.flags_ = 1;
    if (GameDataStateStorage.fields_.net_turn_flags_ & 0x10) {
        if (ConsoleControlTurnActive == 0) {
            if (GameDataStateStorage.fields_.net_turn_faction_ != ConsoleGlobal.faction_id_) {
                goto L0cc;
            }
        } else if (NetDaemonNet.primary_state_.leading_words_[0] == 0) {
            goto L145;
        }
        message_data(0x8301, 0, 0, 0, 0, 0);
    }
L0cc:
    if (NetDaemonNet.primary_state_.leading_words_[0] != 0) {
        locks_.records_[ConsoleGlobal.faction_id_].clear();
        if (locks_.field_E0_ == ConsoleGlobal.faction_id_) {
            locks_.field_E4_ = 0;
            locks_.field_E0_ = 0;
        }
        channels_[0].trailing_words_[0] &=
            ~channels_[0].trailing_words_[1 + ConsoleGlobal.faction_id_];
        message_data(0x4b00, 0, 0, 0, 0, 0);
    }
L145:
    NetMsgPrimary.close();
}


// ORIGINAL: 0x0052E900 ?send_game@NetDaemon@@QAEXXZ 0x0052E900-0x0052ED33 FILE
// size      1075 bytes
// prototype void (__thiscall ?send_game@NetDaemon@@QAEXXZ)(NetDaemon* this)
// callers   1   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00532E00 0x00534400 0x00630A00
// PRESERVED UNIT - measured NO_COMPILE.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0052E900
// measured tier  NO_COMPILE
// refusal        u0052e900.cpp(73) : error C2664: 'synch' : cannot convert parameter 1 from 'const int' to 'struct int16' Source or target has incomplete type u0052e900.cpp(75) 
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0052e900/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?send_game@NetDaemon@@QAEXXZ  at 0x0052E900  (1075 bytes)
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
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?send_game@NetDaemon@@QAEXXZ  at 0x0052E900  (1075 bytes)
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


void NetDaemon::send_game() {
    unsigned long a, b;
    unsigned long c, d;
    unsigned long e, f;
    void *id;

    synch(0, 0, 0, 0, 0, 1, 0x2101);
    for (;;) {
        id = NetDaemonNet.Net::get(&a, &b);
        if (id == 0) break;
        process_message(static_cast<char *>(id), b, a);
    }

    synch(2, 0, 0, 0, 0, 1, 0x2101);
    for (;;) {
        id = NetDaemonNet.Net::get(&a, &b);
        if (id == 0) break;
        process_message(static_cast<char *>(id), b, a);
    }

    synch(4, 0, 0, 0, 0, 1, 0x2101);
    for (;;) {
        id = NetDaemonNet.Net::get(&a, &b);
        if (id == 0) break;
        process_message(static_cast<char *>(id), b, a);
    }

    synch(5, 0, 0, 0, 0, 1, 0x2101);
    for (;;) {
        id = NetDaemonNet.Net::get(&a, &b);
        if (id == 0) break;
        process_message(static_cast<char *>(id), b, a);
    }

    int i = 0;
    int off = 0;
    do {
        synch(6, i, 0, 0, 0, 1, 0x2101);
        for (;;) {
            id = NetDaemonNet.Net::get(&c, &d);
            if (id == 0) break;
            process_message(static_cast<char *>(id), d, c);
        }

        synch(7, i, 0, 0, 0, 1, 0x2101);
        for (;;) {
            id = NetDaemonNet.Net::get(&e, &f);
            if (id == 0) break;
            process_message(static_cast<char *>(id), f, e);
        }

        synch(0xf, off, 0x40, 0, 0, 1, 0x2101);
        for (;;) {
            id = NetDaemonNet.Net::get(&e, &f);
            if (id == 0) break;
            process_message(static_cast<char *>(id), f, e);
        }

        off += 0x40;
        i++;
    } while (off < 0x200);

    synch(0x29, 0, 0, 0, 0, 1, 0x2101);
    for (;;) {
        id = NetDaemonNet.Net::get(&e, &f);
        if (id == 0) break;
        process_message(static_cast<char *>(id), f, e);
    }

    synch(0x12, 0, 0, 0, 0, 1, 0x2101);
    for (;;) {
        id = NetDaemonNet.Net::get(&e, &f);
        if (id == 0) break;
        process_message(static_cast<char *>(id), f, e);
    }

    synch(0x14, 0, 0, 0, 0, 1, 0x2101);
    for (;;) {
        id = NetDaemonNet.Net::get(&e, &f);
        if (id == 0) break;
        process_message(static_cast<char *>(id), f, e);
    }

    synch(0x19, 0, 0, 0, 0, 1, 0x2101);
    for (;;) {
        id = NetDaemonNet.Net::get(&e, &f);
        if (id == 0) break;
        process_message(static_cast<char *>(id), f, e);
    }

    if (GameDataStateStorage.fields_.turn_current_num_ != 0) {
        synch(0x20, 0, 0, 0, 0, 1, 0x2101);
        for (;;) {
            id = NetDaemonNet.Net::get(&e, &f);
            if (id == 0) break;
            process_message(static_cast<char *>(id), f, e);
        }
    }

    synch(0x21, 0, 0, 0, 0, 1, 0x2101);
    for (;;) {
        id = NetDaemonNet.Net::get(&e, &f);
        if (id == 0) break;
        process_message(static_cast<char *>(id), f, e);
    }

    if (ReplayEventBytes != 0) {
        synch(0x22, 0, 0, 0, 0, 1, 0x2101);
        for (;;) {
            id = NetDaemonNet.Net::get(&e, &f);
            if (id == 0) break;
            process_message(static_cast<char *>(id), f, e);
        }
    }

    synch(0x24, 0, 0, 0, 0, 1, 0x2101);
    for (;;) {
        id = NetDaemonNet.Net::get(&e, &f);
        if (id == 0) break;
        process_message(static_cast<char *>(id), f, e);
    }

    synch(0x15, 0, 0, 0, 0, 1, 0x2101);
    for (;;) {
        id = NetDaemonNet.Net::get(&e, &f);
        if (id == 0) break;
        process_message(static_cast<char *>(id), f, e);
    }
}

// ORIGINAL: 0x0052F520 ?send_rules@NetDaemon@@QAEXXZ 0x0052F520-0x0052F8D6 FILE
// size      950 bytes
// prototype void (__thiscall ?send_rules@NetDaemon@@QAEXXZ)(NetDaemon* this)
// callers   1   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00532E00 0x00534400 0x00630A00
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0052F520
// measured tier  MISMATCH
// divergence     28
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0052f520/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?send_rules@NetDaemon@@QAEXXZ  at 0x0052F520  (950 bytes)
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

void NetDaemon::send_rules() {
    unsigned long v4, v8;

    synch(0x80, 0, 0, 0, 0, 1, 0x2101);
    while (void *result = NetDaemonNet.Net::get(&v4, &v8)) {
        process_message(static_cast<char *>(result), v8, v4);
    }

    synch(0x81, 0, 0, 0, 0, 1, 0x2101);
    while (void *result = NetDaemonNet.Net::get(&v4, &v8)) {
        process_message(static_cast<char *>(result), v8, v4);
    }

    synch(0x82, 0, 0, 0, 0, 1, 0x2101);
    while (void *result = NetDaemonNet.Net::get(&v4, &v8)) {
        process_message(static_cast<char *>(result), v8, v4);
    }

    synch(0x83, 0, 0, 0, 0, 1, 0x2101);
    while (void *result = NetDaemonNet.Net::get(&v4, &v8)) {
        process_message(static_cast<char *>(result), v8, v4);
    }

    synch(0x84, 0, 0, 0, 0, 1, 0x2101);
    while (void *result = NetDaemonNet.Net::get(&v4, &v8)) {
        process_message(static_cast<char *>(result), v8, v4);
    }

    synch(0x85, 0, 0, 0, 0, 1, 0x2101);
    while (void *result = NetDaemonNet.Net::get(&v4, &v8)) {
        process_message(static_cast<char *>(result), v8, v4);
    }

    synch(0x86, 0, 0, 0, 0, 1, 0x2101);
    while (void *result = NetDaemonNet.Net::get(&v4, &v8)) {
        process_message(static_cast<char *>(result), v8, v4);
    }

    synch(0x87, 0, 0, 0, 0, 1, 0x2101);
    while (void *result = NetDaemonNet.Net::get(&v4, &v8)) {
        process_message(static_cast<char *>(result), v8, v4);
    }

    synch(0x88, 0, 0, 0, 0, 1, 0x2101);
    while (void *result = NetDaemonNet.Net::get(&v4, &v8)) {
        process_message(static_cast<char *>(result), v8, v4);
    }

    synch(0x89, 0, 0, 0, 0, 1, 0x2101);
    while (void *result = NetDaemonNet.Net::get(&v4, &v8)) {
        process_message(static_cast<char *>(result), v8, v4);
    }

    synch(0x8a, 0, 0, 0, 0, 1, 0x2101);
    while (void *result = NetDaemonNet.Net::get(&v4, &v8)) {
        process_message(static_cast<char *>(result), v8, v4);
    }

    synch(0x8b, 0, 0, 0, 0, 1, 0x2101);
    while (void *result = NetDaemonNet.Net::get(&v4, &v8)) {
        process_message(static_cast<char *>(result), v8, v4);
    }

    synch(0x8c, 0, 0, 0, 0, 1, 0x2101);
    while (void *result = NetDaemonNet.Net::get(&v4, &v8)) {
        process_message(static_cast<char *>(result), v8, v4);
    }

    synch(0x8d, 0, 0, 0, 0, 1, 0x2101);
    while (void *result = NetDaemonNet.Net::get(&v4, &v8)) {
        process_message(static_cast<char *>(result), v8, v4);
    }
}


// ORIGINAL: 0x0052F8E0 ?game_checksum@NetDaemon@@QAEXPAH@Z 0x0052F8E0-0x0053031E FILE
// TRIED: full instruction-order transcription of all ~150 checksum() calls (verified offsets/strides/seed-chaining against raw disasm, not Ghidra scaling); diverges at insn #53, likely register-alloc (lea vs mov) in the bit_count loop or header prologue, not logic.
// working copy - scaffold materialised by --work
// size      2622 bytes
// prototype void (__thiscall ?game_checksum@NetDaemon@@QAEXPAH@Z)(NetDaemon* this, int*)
// callers   1   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0050BA30 0x00539090

void NetDaemon::game_checksum(int *results) {
    unsigned char cs;
    int *out = results;

    cs = checksum((char *)&GameDataStateStorage.fields_.game_rules_, 4, 0);
    cs = checksum((char *)&GameDataStateStorage.fields_.vehicle_current_count_, 0x10, cs);
    cs = checksum((char *)&GameDataStateStorage.fields_.lock_masks_.enable_mask_, 2, cs);
    cs = checksum((char *)&GameDataStateStorage.fields_.secret_project_, 0x100, cs);

    {
        unsigned char *bitTable = (unsigned char *)GameDataStateStorage.fields_.game_tech_achieved_;
        int bi;
        for (bi = 0; bi < 0x59; ++bi) {
            int bc = bit_count((int)bitTable[bi]);
            unsigned char mult = (unsigned char)(bi + 1);
            unsigned char al = (unsigned char)((unsigned char)bc * mult);
            cs = (unsigned char)(cs ^ al);
        }
    }
    *out++ = cs;
    cs = 0;

    {
        // The original cursor starts at faction 1's research word and stops
        // after faction 7. Bases happens to follow PlayersData in the image;
        // its address is not a bound on the source-owned faction array.
        for (int faction = 1; faction < MaxPlayerNum; ++faction) {
            PlayerData &player = PlayersData[faction];
            unsigned int masked1[8];
            unsigned int masked2[8];
            int idx4;
            cs = checksum((char *)&player.tech_accumulated, 4, cs);
            cs = checksum((char *)&player.tech_id_researching, 4, cs);
            cs = checksum((char *)&player.planet_ecology, 4, cs);
            cs = checksum((char *)&player.energy_reserves, 4, cs);
            cs = checksum((char *)&player.soc_category_pending, 0x10, cs);
            cs = checksum((char *)player.satellites_, 0x10, cs);
            for (idx4 = 0; idx4 < 8; ++idx4) {
                masked1[idx4] = player.diplo_treaties[idx4] & 0xb1d3c7f;
                masked2[idx4] = player.diplo_agenda[idx4] & 0x1e68;
            }
            cs = checksum((char *)masked1, 0x20, cs);
            cs = checksum((char *)masked2, 0x20, cs);
            cs = checksum((char *)&player.integrity_blemishes, 4, cs);
        }
    }
    *out++ = cs;
    cs = 0;

    {
        int countA = MapArea;
        if (countA > 0) {
            Map *tile = MapTiles;
            int idxA = 0;
            do {
                cs = checksum((char *)&tile->climate, 1, cs);
                cs = checksum((char *)&tile->region, 1, cs);
                cs = checksum((char *)&tile->bit, 4, cs);
                cs = checksum((char *)&tile->contour, 1, cs);
                cs = checksum((char *)&tile->territory, 1, cs);
                // Only the low byte of bit2 participates in this report.
                cs = checksum((char *)&tile->bit2, 1, cs);
                countA = MapArea;
                ++tile;
                idxA++;
            } while (idxA < countA);
        }
    }
    *out++ = cs;
    cs = 0;

    {
        int idx2;
        int countB = GameDataStateStorage.fields_.vehicle_current_count_;
        for (idx2 = 0; idx2 < 8; ++idx2) {
            if (countB > 0) {
                Veh *vehicle = Vehs;
                int idxB = 0;
                do {
                    if (vehicle->faction_id == idx2) {
                        cs = checksum((char *)&vehicle->x, 2, cs);
                        cs = checksum((char *)&vehicle->y, 2, cs);
                        cs = checksum((char *)&vehicle->proto_id, 2, cs);
                        cs = checksum((char *)&vehicle->faction_id, 1, cs);
                        cs = checksum((char *)&vehicle->dmg_incurred, 1, cs);
                        cs = checksum((char *)&vehicle->morale, 1, cs);
                        cs = checksum((char *)&vehicle->moves_expended, 1, cs);
                        cs = checksum((char *)&vehicle->home_base_id, 2, cs);
                        countB = GameDataStateStorage.fields_.vehicle_current_count_;
                    }
                    ++vehicle;
                    idxB++;
                } while (idxB < countB);
            }
        }
    }
    *out++ = cs;
    cs = 0;

    {
        int countC = GameDataStateStorage.fields_.base_current_count_;
        if (countC > 0) {
            Base *base = Bases;
            int idxC = 0;
            do {
                // Original prefix excludes assimilation_turns_left; the
                // facilities checksum deliberately selects only nine bytes.
                cs = checksum((char *)base, 7, cs);
                cs = checksum((char *)&base->nutrients_accumulated, 4, cs);
                cs = checksum((char *)&base->minerals_accumulated, 4, cs);
                cs = checksum((char *)&base->queue_size, 4, cs);
                cs = checksum((char *)&base->queue_production_id[0], 4, cs);
                cs = checksum((char *)&base->resource_sqr_radius, 4, cs);
                cs = checksum((char *)base->spec, 8, cs);
                cs = checksum((char *)base->facilities_built, 9, cs);
                countC = GameDataStateStorage.fields_.base_current_count_;
                idxC++;
                ++base;
            } while (idxC < countC);
        }
    }
    *out++ = cs;
    cs = 0;

    {
        int rawVal = MapLatitudeBounds;
        int div = (rawVal + ((rawVal >> 31) & 0xf)) >> 4;
        int remainder = rawVal - (div << 4);
        int k;
        for (k = 0; k < 0x10; ++k) {
            int recIdx = k * div;
            if (k == 0xf && remainder != 0) {
                div = div + remainder;
            }
            cs = 0;
            {
                int innerCount = (MapLongitudeBounds >> 1) * div;
                if (div != 0 && innerCount > 0) {
                    Map *tile = MapTiles + static_cast<int>(MapLongitude) * recIdx;
                    int idxE;
                    for (idxE = 0; idxE < innerCount; ++idxE) {
                        cs = checksum((char *)&tile->climate, 1, cs);
                        cs = checksum((char *)&tile->region, 1, cs);
                        cs = checksum((char *)&tile->bit, 4, cs);
                        cs = checksum((char *)&tile->contour, 1, cs);
                        cs = checksum((char *)&tile->territory, 1, cs);
                        ++tile;
                    }
                }
            }
            *out++ = cs;
        }
    }

    if (BaseAtDebugPopupFlag != 0) {
        cs = checksum((char *)&GameDataStateStorage.fields_.game_rules_, 4, 0);
        cs = checksum((char *)&GameDataStateStorage.fields_.vehicle_current_count_, 0x10, cs);
        *out++ = checksum((char *)&GameDataStateStorage.fields_.lock_masks_.enable_mask_, 2, cs);
        *out++ = checksum((char *)&GameDataStateStorage.fields_.secret_project_, 0x100, 0);
        cs = 0;

        {
            unsigned char *bitTable = (unsigned char *)GameDataStateStorage.fields_.game_tech_achieved_;
            int bi2;
            for (bi2 = 0; bi2 < 0x59; ++bi2) {
                int bc = bit_count((int)bitTable[bi2]);
                unsigned char mult = (unsigned char)(bi2 + 1);
                unsigned char al = (unsigned char)((unsigned char)bc * mult);
                cs = (unsigned char)(cs ^ al);
            }
        }
        *out++ = cs;

        {
            for (int faction = 1; faction < MaxPlayerNum; ++faction) {
                PlayerData &player = PlayersData[faction];
                cs = checksum((char *)&player.tech_accumulated, 4, 0);
                cs = checksum((char *)&player.tech_id_researching, 4, cs);
                cs = checksum((char *)&player.planet_ecology, 4, cs);
                cs = checksum((char *)&player.energy_reserves, 4, cs);
                cs = checksum((char *)&player.soc_category_pending, 0x10, cs);
                cs = checksum((char *)player.satellites_, 0x10, cs);
                cs = checksum((char *)player.diplo_treaties, 0x20, cs);
                cs = checksum((char *)player.diplo_agenda, 0x20, cs);
                cs = checksum((char *)player.diplo_friction, 0x20, cs);
                // The original hashes 32 bytes here, not just the eight
                // patience bytes: retain the following sanction/loan bytes.
                cs = checksum((char *)&player + offsetof(PlayerData, diplo_patience), 0x20, cs);
                cs = checksum((char *)player.diplo_merc, 0x20, cs);
                cs = checksum((char *)player.diplo_spoke, 0x20, cs);
                cs = checksum((char *)player.diplo_wrongs, 0x20, cs);
                cs = checksum((char *)&player.integrity_blemishes, 4, cs);
                *out++ = cs;
            }
        }
        cs = 0;

        {
            int cnt1 = MapArea;
            Map *tile = MapTiles;
            int i1;
            for (i1 = 0; i1 < cnt1; ++i1) {
                cs = checksum((char *)&tile->climate, 1, cs);
                cnt1 = MapArea;
                ++tile;
            }
        }
        *out++ = cs;
        cs = 0;

        {
            int cnt2 = MapArea;
            Map *tile = MapTiles;
            int i2;
            for (i2 = 0; i2 < cnt2; ++i2) {
                cs = checksum((char *)&tile->region, 1, cs);
                cnt2 = MapArea;
                ++tile;
            }
        }
        *out++ = cs;
        cs = 0;

        {
            int cnt3 = MapArea;
            Map *tile = MapTiles;
            int i3;
            for (i3 = 0; i3 < cnt3; ++i3) {
                cs = checksum((char *)&tile->bit, 4, cs);
                cnt3 = MapArea;
                ++tile;
            }
        }
        *out++ = cs;
        cs = 0;

        {
            int cnt4 = MapArea;
            Map *tile = MapTiles;
            int i4;
            for (i4 = 0; i4 < cnt4; ++i4) {
                cs = checksum((char *)&tile->contour, 1, cs);
                cnt4 = MapArea;
                ++tile;
            }
        }
        *out++ = cs;
        cs = 0;

        {
            int cnt5 = MapArea;
            Map *tile = MapTiles;
            int i5;
            for (i5 = 0; i5 < cnt5; ++i5) {
                cs = checksum((char *)&tile->territory, 1, cs);
                cnt5 = MapArea;
                ++tile;
            }
        }
        *out++ = cs;
        cs = 0;

        {
            int cnt6 = MapArea;
            Map *tile = MapTiles;
            int i6;
            for (i6 = 0; i6 < cnt6; ++i6) {
                cs = checksum((char *)&tile->bit2, 1, cs);
                cnt6 = MapArea;
                ++tile;
            }
        }
        *out++ = cs;

        *out++ = checksum((char *)&MapSizePlanet, 0x1c, 0);

        cs = 0;
        {
            int idx2b;
            for (idx2b = 0; idx2b < 8; ++idx2b) {
                int countB2 = GameDataStateStorage.fields_.vehicle_current_count_;
                cs = 0;
                if (countB2 > 0) {
                    Veh *vehicle = Vehs;
                    int idxBb = 0;
                    do {
                        if (vehicle->faction_id == idx2b) {
                            cs = checksum((char *)&vehicle->x, 2, cs);
                            cs = checksum((char *)&vehicle->y, 2, cs);
                            // Original three-byte range includes the low
                            // byte of unk_1, beyond the two-byte proto_id.
                            cs = checksum((char *)vehicle + offsetof(Veh, proto_id), 3, cs);
                            cs = checksum((char *)&vehicle->morale, 1, cs);
                            cs = checksum((char *)&vehicle->moves_expended, 1, cs);
                            cs = checksum((char *)&vehicle->home_base_id, 2, cs);
                        }
                        countB2 = GameDataStateStorage.fields_.vehicle_current_count_;
                        ++vehicle;
                        idxBb++;
                    } while (idxBb < countB2);
                }
                *out++ = cs;
            }
        }

        cs = 0;
        {
            int countC2 = GameDataStateStorage.fields_.base_current_count_;
            if (countC2 > 0) {
                Base *base = Bases;
                int idxC2 = 0;
                do {
                    cs = checksum((char *)base, 7, cs);
                    cs = checksum((char *)&base->nutrients_accumulated, 4, cs);
                    cs = checksum((char *)&base->minerals_accumulated, 4, cs);
                    cs = checksum((char *)&base->queue_size, 4, cs);
                    cs = checksum((char *)&base->queue_production_id[0], 4, cs);
                    cs = checksum((char *)&base->resource_sqr_radius, 4, cs);
                    cs = checksum((char *)base->spec, 8, cs);
                    cs = checksum((char *)base->facilities_built, 9, cs);
                    countC2 = GameDataStateStorage.fields_.base_current_count_;
                    idxC2++;
                    ++base;
                } while (idxC2 < countC2);
            }
        }
        *out++ = cs;
    }
}


// ORIGINAL: 0x005328C0 ?end_of_event@NetDaemon@@QAEHHH@Z 0x005328C0-0x00532934 FILE
// working copy - scaffold materialised by --work
// size      116 bytes
// prototype int (__thiscall ?end_of_event@NetDaemon@@QAEHHH@Z)(NetDaemon* this, int, int)
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00592EE0 0x006262F0

// 0x1B2C/0x1B30 are lock_state_[0].dword_[0]/[1]; 0x1BB0 and 0x1BC0 are
// lock_state_[2].dword_[3] and [7].
int NetDaemon::end_of_event(int faction_id, int turn_value) {
    if (primary_state_.leading_words_[0] == 0) {
        deletion_list_.set_sequence_bits(turn_value);
    }
    if (faction_id == ConsoleGlobal.faction_id_) {
        if (channels_[1].trailing_words_[2] != 0) {
            log_say("Remaining Locked2", 0, 0, 0);
            channels_[1].trailing_words_[2] = 0;
            message_data(0x2214, 0, 0, 0, 0, 0);
            return 0;
        }
        channels_[1].leading_words_[3] = 0;
    }
    return 0;
}


// ---------------------------------------------------------------------------
// The message builders. Every sender in the family packs its record on the
// stack, stamps the three leading fields the transport owns - the opcode
// (short), the local faction and the wall-clock stamp - then hands the record
// to NetDaemon::send_message with NetBusyCount held for the duration. Only
// the payload fields differ.
//
// The turn stamp is 0x0093E8BC, which is the daemon's own
// lock_state_[0].dword_[0]: net_end_of_turn (0x00529F30) ends with
// `++*0x0093E8BC`, once per turn, so the field counts turns for the whole
// session. The stamp comes from the SDK timeGetTime import; its original
// import-slot identity is bound beside the other audio clock consumers.
//
// Payload parameters are named for the record offset they land at, which is
// the one thing about them the bodies prove.
// ---------------------------------------------------------------------------

// ORIGINAL: 0x00592E10 ?message_veh@@YAXHHHH@Z 0x00592E10-0x00592E70 BYTE_EXACT
// size      96 bytes
// prototype void (__cdecl ?message_veh@@YAXHHHH@Z)(int, int, int, int)
// callers   12   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00532940
// LEVER: the two payload stores were written field_14_ first; the struct
//        declares field_10_ before field_14_, and VC6 evaluates assignments in
//        source order, so the loads came out swapped - the image reads
//        [ebp+0x10] then [ebp+0xC]. Declaration order for the stores took it
//        SHAPE_EXACT to BYTE_EXACT.
void __cdecl message_veh(int opcode, int payload_10, int payload_14,
                         int target) {
    struct Message {
        short opcode_;
        int faction_;
        unsigned long time_;
        int turn_;
        int field_10_;
        int field_14_;
    } message;

    message.opcode_ = static_cast<short>(opcode);
    message.faction_ = ConsoleGlobal.faction_id_;
    message.time_ = timeGetTime();
    message.turn_ = NetDaemonNet.deletion_list_.sequence_bits();
    message.field_10_ = payload_10;
    message.field_14_ = payload_14;

    NetBusyCount++;
    NetDaemonNet.send_message(reinterpret_cast<char *>(&message), 0x18,
                              target);
    NetBusyCount--;
}

// ORIGINAL: 0x00592E70 ?message_base@@YAXHHPADH@Z 0x00592E70-0x00592EDA BYTE_EXACT
// size      106 bytes
// prototype void (__cdecl ?message_base@@YAXHHPADH@Z)(int, int, char *, int)
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00532940 0x00645460
void __cdecl message_base(int opcode, int payload_10, char *text, int target) {
    struct Message {
        short opcode_;
        int faction_;
        unsigned long time_;
        int turn_;
        int payload_10_;
        char text_[24];
    } message;

    message.opcode_ = static_cast<short>(opcode);
    message.faction_ = ConsoleGlobal.faction_id_;
    message.time_ = timeGetTime();
    message.turn_ = NetDaemonNet.deletion_list_.sequence_bits();
    message.payload_10_ = payload_10;
    strcpy(message.text_, text);

    NetBusyCount++;
    NetDaemonNet.send_message(reinterpret_cast<char *>(&message), 0x2c,
                              target);
    NetBusyCount--;
}

// The two bytes after opcode are observed unwritten in the native stack record;
// keep them unresolved and do not value-initialize the local object.
struct MessageDataRecord {
    int16_t opcode_;
    unsigned char unresolved_2_[2];
    int faction_;
    uint32_t timestamp_;
    int sequence_;
    int payload_10_;
    int payload_14_;
    int payload_18_;
    int payload_1C_;
};

static_assert(sizeof(MessageDataRecord) == 0x20,
              "message_data stack record width from native stores");
static_assert(offsetof(MessageDataRecord, faction_) == 0x04,
              "message_data faction field offset");
static_assert(offsetof(MessageDataRecord, timestamp_) == 0x08,
              "message_data timestamp field offset");
static_assert(offsetof(MessageDataRecord, sequence_) == 0x0C,
              "message_data sequence field offset");
static_assert(offsetof(MessageDataRecord, payload_10_) == 0x10,
              "message_data first payload field offset");
static_assert(offsetof(MessageDataRecord, payload_14_) == 0x14,
              "message_data second payload field offset");
static_assert(offsetof(MessageDataRecord, payload_18_) == 0x18,
              "message_data third payload field offset");
static_assert(offsetof(MessageDataRecord, payload_1C_) == 0x1C,
              "message_data fourth payload field offset");

// ORIGINAL: 0x00592EE0 ?message_data@@YAXHHHHHH@Z 0x00592EE0-0x00592F4C
// RECOVERY_BINDING: v1 {"evidence": ["Pinned592EE0..592F4C: six int inputs, target is argument2; 32-byte local packet has unwritten bytes2..3. Call532940 then dec93F664 and epilogue preserve EAX.", "Pinned53111B caller and source NetDaemon::unlock_veh consume all32 result bits; other current callers discard them.", "Superseded source dead ends: nonvoid body without return failed VC6 C4716; returning NetBusyCount added a wrong load. Returning the actual lower Net::send result resolves both without register tricks.", "Fresh root private VC6 COFF probe observes current symbol ?message_data@@YAHHHHHHH@Z", "recovery/tasks/message-data-owner-oct5.json", "recovery/tests/test_message_data_actual_source.py"], "reason": "Replace the original-address forwarding wrapper with the complete typed automatic packet builder. Expose the send result already observed by unlock_veh without changing native routing, busy-count ordering or the two unwritten packet bytes. Current netdaemon.cpp home is family ownership, not asserted original TU ownership."}
// symbol    ?message_data@@YAHHHHHHH@Z
int __cdecl message_data(int opcode, int target, int payload_10,
                         int payload_14, int payload_18, int payload_1C) {
    MessageDataRecord message;
    message.opcode_ = static_cast<int16_t>(opcode);
    message.faction_ = ConsoleGlobal.faction_id_;
    message.timestamp_ = timeGetTime();
    message.payload_14_ = payload_14;
    message.sequence_ = NetDaemonNet.deletion_list_.sequence_bits();

    ++NetBusyCount;
    message.payload_18_ = payload_18;
    message.payload_10_ = payload_10;
    message.payload_1C_ = payload_1C;
    const int result = NetDaemonNet.send_message(
        reinterpret_cast<char *>(&message), 0x20, target);
    --NetBusyCount;
    return result;
}

// Native automatic wire records. Bytes 2..3 remain unwritten in all three
// builders; the raw bulk sender also leaves data_[8..31] unwritten. These
// objects must not be value-initialized merely to make the wire deterministic.
struct MessageBigDataRecord {
    int16_t opcode_;
    unsigned char unresolved_2_[2];
    int faction_;
    uint32_t timestamp_;
    int sequence_;
    int data_[32];
};

struct MessageLandmarkRecord {
    int16_t opcode_;
    unsigned char unresolved_2_[2];
    int faction_;
    uint32_t timestamp_;
    int sequence_;
    int payload_10_;
    int payload_14_;
    int payload_18_;
    int payload_1C_;
    char text_[32];
};

static_assert(sizeof(MessageBigDataRecord) == 0x90,
              "bulk-data native transmitted extent");
static_assert(offsetof(MessageBigDataRecord, faction_) == 0x04,
              "bulk-data faction word");
static_assert(offsetof(MessageBigDataRecord, timestamp_) == 0x08,
              "bulk-data clock word");
static_assert(offsetof(MessageBigDataRecord, sequence_) == 0x0C,
              "bulk-data sequence word");
static_assert(offsetof(MessageBigDataRecord, data_) == 0x10,
              "bulk-data thirty-two-word payload");
static_assert(sizeof(MessageLandmarkRecord) == 0x40,
              "landmark native transmitted extent");
static_assert(offsetof(MessageLandmarkRecord, faction_) == 0x04,
              "landmark faction word");
static_assert(offsetof(MessageLandmarkRecord, timestamp_) == 0x08,
              "landmark clock word");
static_assert(offsetof(MessageLandmarkRecord, sequence_) == 0x0C,
              "landmark sequence word");
static_assert(offsetof(MessageLandmarkRecord, payload_10_) == 0x10,
              "landmark first payload word");
static_assert(offsetof(MessageLandmarkRecord, payload_14_) == 0x14,
              "landmark second payload word");
static_assert(offsetof(MessageLandmarkRecord, payload_18_) == 0x18,
              "landmark third payload word");
static_assert(offsetof(MessageLandmarkRecord, payload_1C_) == 0x1C,
              "landmark fourth payload word");
static_assert(offsetof(MessageLandmarkRecord, text_) == 0x20,
              "landmark optional text start");

// ORIGINAL: 0x00592F50 sub_592f50 0x00592F50-0x00592FE9
// symbol    ?send_raw_data@@YAXHHHHHHHHHH@Z
// size      153 bytes
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00532940
// The catalogue has no symbol for this address. It is message_big_data's own
// record spelled with the eight payload words taken as parameters instead of
// copied from an array, and nothing in the image calls it.
void __cdecl send_raw_data(int opcode, int target, int payload_10,
                           int payload_14, int payload_18, int payload_1C,
                           int payload_20, int payload_24, int payload_28,
                           int payload_2C) {
    MessageBigDataRecord message;

    message.opcode_ = static_cast<int16_t>(opcode);
    message.faction_ = ConsoleGlobal.faction_id_;
    message.timestamp_ = timeGetTime();
    message.sequence_ = NetDaemonNet.deletion_list_.sequence_bits();
    message.data_[0] = payload_10;
    message.data_[1] = payload_14;
    message.data_[2] = payload_18;
    message.data_[3] = payload_1C;
    message.data_[4] = payload_20;
    message.data_[5] = payload_24;
    message.data_[6] = payload_28;
    message.data_[7] = payload_2C;

    NetBusyCount++;
    NetDaemonNet.send_message(reinterpret_cast<char *>(&message), 0x90, target);
    NetBusyCount--;
}

// ORIGINAL: 0x00592FF0 ?message_big_data@@YAXHHPAHH@Z 0x00592FF0-0x00593089
// size      153 bytes
// prototype void (__cdecl ?message_big_data@@YAXHHPAHH@Z)(int, int, int *, int)
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00532940
// TRIED: assigning `filled = count` AFTER the copy loop instead of before it -
//        the image emits `mov eax,edx` before `pop esi`, which the assignment
//        at the top of the block reproduces, and moving it back agreed on
//        FEWER instructions (25 of 47 against 27). Leave it where it is.
// TRIED: naming a `fill_count` local for the zero-fill (`0x20 - filled`) and
//        spelling the bound inline - both leave VC6 guarding its own
//        `rep stosd` with `test ecx,ecx / jle 0x593058`, two instructions the
//        image does not have because it never re-tests. That test is the
//        residual: 27 of 47 agree, and no spelling of the loop took it out.
void __cdecl message_big_data(int opcode, int target, int *payload,
                              int count) {
    MessageBigDataRecord message;
    int filled;
    int i;
    int *dst;

    message.opcode_ = static_cast<int16_t>(opcode);
    message.faction_ = ConsoleGlobal.faction_id_;
    message.timestamp_ = timeGetTime();
    message.sequence_ = NetDaemonNet.deletion_list_.sequence_bits();

    filled = 0;
    if (count > 0) {
        filled = count;
        dst = message.data_;
        for (i = 0; i < count; i++) {
            dst[i] = payload[i];
        }
    }
    if (filled < 0x20) {
        dst = message.data_ + filled;
        // No named `fill_count` local: the image computes `0x20 - filled`
        // straight into the rep count (`mov ecx,0x20 / sub ecx,eax`), and a
        // local made VC6 test it separately - two instructions the image does
        // not have.
        for (i = 0; i < 0x20 - filled; i++) {
            dst[i] = 0;
        }
    }

    NetBusyCount++;
    NetDaemonNet.send_message(reinterpret_cast<char *>(&message), 0x90,
                              target);
    NetBusyCount--;
}

// ORIGINAL: 0x00593090 ?message_lock@@YAXHHHHHHHH@Z 0x00593090-0x00593108 BYTE_EXACT
// size      120 bytes
// prototype void (__cdecl ?message_lock@@YAXHHHHHHHH@Z)(int, int, int, int, int, int, int, int)
// callers   2   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00532940
// LEVER: the payload-to-slot mapping, not the store order - the image puts the
//        SECOND parameter into the record at 0x18 and the FIFTH into 0x24, and
//        hands the EIGHTH to send_message as the network destination. The old
//        body sent `target` instead: the image broadcasts a lock request and
//        records the target in the record at 0x18, while this tree was
//        addressing it. Faithful to the shipped code, which is what is
//        reproduced here.
void __cdecl message_lock(int opcode, int target, int payload_10,
                          int payload_14, int payload_18, int payload_1C,
                          int payload_20, int send_target) {
    struct Message {
        short opcode_;
        int faction_;
        unsigned long time_;
        int turn_;
        int field_10_;
        int field_14_;
        int field_18_;
        int field_1C_;
        int field_20_;
        int field_24_;
    } message;

    message.opcode_ = static_cast<short>(opcode);
    message.faction_ = ConsoleGlobal.faction_id_;
    message.time_ = timeGetTime();
    message.turn_ = NetDaemonNet.deletion_list_.sequence_bits();
    message.field_10_ = payload_10;
    message.field_14_ = payload_14;
    message.field_18_ = target;
    message.field_1C_ = payload_1C;
    message.field_20_ = payload_20;
    message.field_24_ = payload_18;

    NetBusyCount++;
    NetDaemonNet.send_message(reinterpret_cast<char *>(&message),
                              sizeof(message), send_target);
    NetBusyCount--;
}

// ORIGINAL: 0x00593110 ?message_landmark@@YAXHHPADHHHH@Z 0x00593110-0x00593190
// size      128 bytes
// prototype void (__cdecl ?message_landmark@@YAXHHPADHHHH@Z)(int, int, char *, int, int, int, int)
// callers   1   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00532940 0x00645460
// Native payload stores are +0x10, +0x1C, sequence, +0x14, +0x18.
// Null text leaves the entire text area unwritten; strcpy remains unchecked.
void __cdecl message_landmark(int opcode, int target, char *text,
                              int payload_10, int payload_14, int payload_18,
                              int payload_1C) {
    MessageLandmarkRecord message;

    message.opcode_ = static_cast<int16_t>(opcode);
    message.faction_ = ConsoleGlobal.faction_id_;
    message.timestamp_ = timeGetTime();
    message.payload_10_ = payload_10;
    message.payload_1C_ = payload_1C;
    message.sequence_ = NetDaemonNet.deletion_list_.sequence_bits();
    message.payload_14_ = payload_14;
    message.payload_18_ = payload_18;
    if (text) {
        strcpy(message.text_, text);
    }

    NetBusyCount++;
    NetDaemonNet.send_message(reinterpret_cast<char *>(&message), 0x40, target);
    NetBusyCount--;
}

// ORIGINAL: 0x00593190 ?message_chat@@YAXHHPAD@Z 0x00593190-0x00593215 BYTE_EXACT
// size      133 bytes
// prototype void (__cdecl ?message_chat@@YAXHHPAD@Z)(int, int, char *)
// callers   3   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00532940 0x00645E90
// The image calls strncpy for an unbounded 0x100 and only then NUL-terminates
// the last byte - a BUG IN THE ORIGINAL, preserved: a text of 0x100 bytes
// with no terminator walks off the buffer.
void __cdecl message_chat(int opcode, int target, char *text) {
    struct Message {
        short opcode_;
        short pad_02_;
        int faction_;
        unsigned long time_;
        int turn_;
        char text_[0x100];
    } message;

    message.opcode_ = static_cast<short>(opcode);
    message.faction_ = ConsoleGlobal.faction_id_;
    message.time_ = timeGetTime();
    message.turn_ = NetDaemonNet.deletion_list_.sequence_bits();
    strncpy(message.text_, text, 0x100);

    NetBusyCount++;
    message.text_[0xff] = 0;
    NetDaemonNet.send_message(reinterpret_cast<char *>(&message), 0x110,
                              target);
    NetBusyCount--;
}

// Fixed-slot bindings carried from 00532940.cpp

// ORIGINAL: 0x00532940 ?send_message@NetDaemon@@QAEXPADKH@Z 0x00532940-0x00532A44 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned532940..532A44: initial IsMultiplayerNet load supplies offline EAX; every enabled Net::send630080 result reaches the epilogue unchanged.", "Keep native mask precedence4000,8000,A000 and server/turn-owner fallback routing; caller arguments and object layout remain unchanged.", "Fresh root private VC6 COFF probe observes current symbol ?send_message@NetDaemon@@QAEHPADKH@Z", "recovery/tasks/message-data-owner-oct5.json", "recovery/tests/test_message_data_actual_source.py"], "reason": "Expose the existing native transport result as a typed int while preserving the original void-decorated identity, full spans and historical match/restoration obligations. Disabled transport returns its captured zero flag; each enabled route returns Net::send EAX. This is not a new measured claim."}
// symbol    ?send_message@NetDaemon@@QAEHPADKH@Z
// size      260 bytes
// prototype void (__thiscall ?send_message@NetDaemon@@QAEXPADKH@Z)(NetDaemon* this, int8*, unsigned int, int)
// callers   12   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004E2660 0x00630080
// indirect  0x0053295F
// PRESERVED UNIT - measured NO_COMPILE.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00532940
// measured tier  NO_COMPILE
// refusal        u00532940.cpp(46) : error C2653: 'NetDaemon' : is not a class or namespace name u00532940.cpp(47) : error C2065: 'IsMultiplayerNet' : undeclared identifier u00532940.
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00532940/unit.cpp
// and score it with tools/agent_brief.py.
// 0x00532940  ?send_message@NetDaemon@@QAEXPADKH@Z  (260 bytes)
//
// A four-way dispatch on bits of `*(short*)message`: 0x4000 (timestamp+send),
// 0x8000 (send with a possibly-substituted value), 0xa000 (an extra
// who_2_pid path, falling back to the same substitution as 0x8000), and a
// default (send with `target` or `who_2_pid(target)`). `Net::send` and
// `AlphaNet::who_2_pid` are both called through `reinterpret_cast<Net*>` /
// `reinterpret_cast<AlphaNet*>(this)`: NetDaemon is opaque and neither base
// is declared as a real base class in this unit.
//
// `timeGetTime` is called through its IAT slot (g_00669368), same pattern
// as UNK2's Win32 calls (0x00609bf0) - a `__stdcall` function-pointer
// typedef dereferenced off the fixed-address global, not a named extern.
//
// THREE occurrences of "value = (global==1) ? target : fallback" (two computing
// `this->field_764` for the same 0x764 offset, one computing
// `who_2_pid(target)`) each needed the SAME two source-form fixes to reproduce
// their original instruction order, none of them obvious from a plain
// if/else:
//
//   1. `if (cond) { return_early_no_assignment; }` (no else) let the
//      compiler duplicate the ENTIRE trailing `send(...)` call and epilogue
//      into both arms instead of merging on one shared call site - a
//      single-branch if with no else, immediately followed by the
//      unconditional `send`, is NOT enough; it needs an explicit
//      `if (..) { value = A; } else { value = B; }` with a dedicated local,
//      merging back to ONE assignment before the shared call.
//   2. Even with that, a REUSED PARAMETER (`if (x!=1) target = fallback;`)
//      compiled the load-and-compare in the opposite order from a fresh
//      local (`int value; if (x==1) value=target; else value=fallback; target=value;`)
//      - original loads the flag/condition FIRST, then the "keep target" arm,
//      matching the fresh-local form, not the reused-parameter form.
//   3. A trivial `target = target;` self-assignment (tried for the third,
//      who_2_pid, occurrence, to force the same shape without a spare
//      local) gets optimized away entirely, losing the explicit
//      reload+jmp the original has - only the real `int value; if (){ }
//      else { }` form survives with the merge point intact.
//
// The 0xa000 branch also reads `NetDaemonNet.lock_state_[0].dword_[5]` ONCE into a local (`counter`)
// BEFORE its four-way `&&` guard, and the branch's own fallback re-tests
// that SAME local rather than re-reading the global - the original never
// reloads `[0x93e8d0]` a second time inside this branch (one `mov`, two
// `cmp`s against the same register).
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?send_message@NetDaemon@@QAEXPADKH@Z  at 0x00532940  (260 bytes)
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


// A four-way dispatch on bits of the record's leading word: 0x4000 restamps
// the time and sends, 0x8000 sends with a possibly-substituted player id,
// 0xA000 adds a who_2_pid path that falls back to the same substitution, and
// the default substitutes who_2_pid for any target but the local one. The
// substitution's fallback is field_764_, the two words check_polling compares
// (see net_class.h); the turn counter it reads once into `counter` is
// lock_state_[0].dword_[5].
//
// Each of the three "value = (flag == 1) ? target : fallback" shapes needed
// a dedicated local and an if/else pair - a single-branch if duplicated the
// trailing send into both arms, a reused parameter compiled the load and
// compare in the opposite order, and `target = target;` was folded away
// entirely. Measured, and the three sites are spelled the same way for it.
int NetDaemon::send_message(char *message, unsigned long length, int target) {
    const int multiplayer = IsMultiplayerNet;
    if (!multiplayer) {
        return multiplayer;
    }

    const unsigned short flags = *reinterpret_cast<unsigned short *>(message);
    if (flags & 0x4000) {
        *reinterpret_cast<uint32_t *>(message + 8) = timeGetTime();
        return Net::send(message, length, 0, 1);
    }
    if (flags & 0x8000) {
        int value;
        if (NetDaemonNet.primary_state_.trailing_words_[0] == 1) {
            value = target;
        } else {
            value = field_764_;
        }
        target = value;
        return Net::send(message, length, target, 1);
    }
    if (flags & 0xA000) {
        const int counter = NetDaemonNet.primary_state_.trailing_words_[0];
        if ((GameDataStateStorage.fields_.net_turn_flags_ & 0x10) && ConsoleControlTurnActive == 0 &&
            GameDataStateStorage.fields_.net_turn_faction_ > 0 && counter > 1) {
            return Net::send(message, length,
                             AlphaNet::who_2_pid(GameDataStateStorage.fields_.net_turn_faction_), 1);
        }
        int value;
        if (counter == 1) {
            value = target;
        } else {
            value = field_764_;
        }
        target = value;
        return Net::send(message, length, target, 1);
    }

    int value;
    if (NetDaemonNet.primary_state_.trailing_words_[0] == 1) {
        value = target;
    } else {
        value = AlphaNet::who_2_pid(target);
    }
    target = value;
    return Net::send(message, length, target, 1);
}


// ---------------------------------------------------------------------------
// THE VEHICLE LOCK. 0x00531020 takes the vehicle by POINTER - the pointer is
// what the daemon stores at lock_state_[1].dword_[4] (0x1B78) to remember
// which unit this client holds - and everything else arrives as ints that go
// straight through to NetDaemon::lock at the record offsets named below.
// ---------------------------------------------------------------------------

// ORIGINAL: 0x00531020 ?lock_veh@NetDaemon@@QAEHPAHHHHH@Z 0x00531020-0x005310EB BYTE_EXACT
// size      203 bytes
// prototype int (__thiscall ?lock_veh@NetDaemon@@QAEHPAHHHHH@Z)(NetDaemon* this, int*, int, int, int, int)
// callers   32   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0047A890 0x00530630
// 0x1B78 is lock_state_[1].dword_[4], 0x1BB0 lock_state_[2].dword_[3] and
// 0x1BC0 lock_state_[2].dword_[7]. The is-alive word at 0x93E8EC is the
// daemon's own lock_state_[0].dword_[12]. The unit's x and y come out of
// Vehs[veh_id] - the image's `[0x952828 + id*0x34]` and `[0x95282a + ...]`.
// LEVER: Vehs[*veh_id].x/.y through the modelled Veh record, not an extern
// char[] base plus two short locals. The archive had this at MISMATCH #34;
// naming the array is what lets VC6 fold the 0x34 stride into the two
// `movsx reg, word ptr [id*0x34 + 0x952828/0x95282a]` the image emits, and
// it took this body from 33 agreeing instructions to BYTE_EXACT 68/68
// (2026-09-04).
int NetDaemon::lock_veh(int *veh_id, int target, int payload_1C,
                        int payload_20, int payload_18) {
    if (IsMultiplayerNet == 0) {
        return 0;
    }
    if ((NetDaemonNet.primary_state_.trailing_words_[7] & (1 << ConsoleGlobal.faction_id_)) != 0 &&
        (GameDataStateStorage.fields_.net_turn_flags_ & 0x10) == 0) {
        NetMsgPrimary.pop("ALREADYDONE", 0, 2, 0);
        return 1;
    }

    channels_[0].payload_.vehicle_selection_follow = veh_id;
    int result = lock(target, Vehs[*veh_id].x, Vehs[*veh_id].y, payload_18,
                      payload_1C, payload_20);
    if (result == 0 && *veh_id >= 0) {
        channels_[1].leading_words_[3] = 1;
        channels_[1].trailing_words_[2] = 0;
        return 0;
    }
    channels_[0].payload_.vehicle_selection_follow = 0;
    return 1;
}

// ORIGINAL: 0x00531150 ?automated_lock@NetDaemon@@QAEHPAHH@Z 0x00531150-0x005312F6
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/00531150.cpp; image:0x00531150-0x005312F6", "Native5311A8 owns this+1BB4 in existing lock_state_[2].dword_[4]; warning5311EF uses initial time while polling531230 uses moving retry time."], "reason": "Recover automated vehicle locking through the genuine NetDaemon owner: native calls identify vehicle-ID pointer forwarding, separate initial-warning and retry timers, message pumping and balanced busy/state lifetimes."}
// TRIED: MISMATCH #26 mov/add - NetDaemon has no known field layout, so the `locked` flag at this+0x1BB4 and the embedded Lock at this+0x14A0 are raw offsets rather than named members; close past instruction 26 but register allocation in the retry loop still differs.
int NetDaemon::automated_lock(int *veh_id, int target) {
    if (IsMultiplayerNet == 0) return 0;

    log_say("Getting auto lock", *veh_id,
            static_cast<int>(static_cast<int8_t>(Vehs[*veh_id].order)), 0);
    ++NetBusyCount;
    DWORD (WINAPI *const ticks)() = timeGetTime;
    const DWORD initial_time = ticks();
    DWORD retry_time = initial_time;
    // This is the daemon's existing state entry at this+0x1BB4, not
    // separate storage or an executable-address flag.
    channels_[1].payload_.flags = 1;
    bool awaiting_orders = false;
    if (ExitTurnLoop == 0) {
        do {
            if (lock_veh(veh_id, target, -1, -1, 0) == 0) break;
            // The warning uses the initial time; retry pacing uses its own
            // moving timestamp. Resetting both prevents the native warning.
            if (!awaiting_orders && ticks() - initial_time > 500U) {
                awaiting_orders = true;
                NetMsgPrimary.pop("AWAITORDERS", 0, 2, 0);
            } else {
                log_say("(Auto lock delay)", *veh_id, 0, 0);
                while (ticks() - retry_time < 250U && ExitTurnLoop == 0) {
                    wait_task();
                    unsigned long first;
                    unsigned long second;
                    void *message = NetDaemonNet.Net::get(&first, &second);
                    if (message != 0) {
                        process_message(static_cast<char *>(message),
                                        second, static_cast<int>(first));
                    }
                    locks_.check_global();
                    check_spock();
                }
                retry_time = ticks();
            }
            check_spock();
        } while (ExitTurnLoop == 0);
        if (awaiting_orders) NetMsgPrimary.close();
    }
    log_say("Got auto lock", *veh_id,
            static_cast<int>(static_cast<int8_t>(Vehs[*veh_id].order)), 0);
    channels_[1].payload_.flags = 0;
    --NetBusyCount;
    return ExitTurnLoop;
}

// ORIGINAL: 0x0052DF30 ?init@NetDaemon@@QAEHH@Z 0x0052DF30-0x0052E041
// TRIED: `if (setup(option)==0) return 0; return 1;` and its `!=`/local-var/ if-else variants all boolean-normalize (neg/sbb or xor/setcc) at #51 where the original keeps two distinct epilogues (fall-through returns the call's own eax, the taken branch explicitly sets eax=1). Landing the closest form.
// size      273 bytes
// prototype int (__thiscall ?init@NetDaemon@@QAEHH@Z)(NetDaemon* this, int)
// callers   1   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004E25B0 0x004E31E0 0x005900D0


int NetDaemon::init(int option) {

    primary_state_.leading_words_[0] = 0;
    primary_state_.leading_words_[1] = 0;
    primary_state_.leading_words_[2] = 0;
    primary_state_.flags_ = 0;
    primary_state_.trailing_words_[8] = 0;
    primary_state_.trailing_words_[7] = 0;
    channels_[0].leading_words_[1] = 0;
    channels_[0].leading_words_[0] = 0;
    channels_[0].trailing_words_[0] = 0;
    channels_[1].leading_words_[1] = 0;
    channels_[0].trailing_words_[9] = 0;
    channels_[1].leading_words_[0] = 0;
    channels_[1].trailing_words_[5] = 0;
    channels_[1].trailing_words_[4] = 0;
    primary_state_.trailing_words_[4] = 0;
    primary_state_.trailing_words_[5] = 0;
    primary_state_.trailing_words_[6] = 0;
    channels_[1].trailing_words_[3] = 0;
    channels_[1].trailing_words_[2] = 0;
    channels_[1].leading_words_[3] = 0;
    channels_[1].leading_words_[2] = 0;
    channels_[1].payload_.flags = 0;
    primary_state_.trailing_words_[0] = 0;
    primary_state_.trailing_words_[1] = 0;
    channels_[1].trailing_words_[8] = 0;
    channels_[1].trailing_words_[7] = 0;
    primary_state_.trailing_words_[2] = 0;
    primary_state_.trailing_words_[3] = 0;
    channels_[1].trailing_words_[6] = 0xff;
    channels_[0].payload_.vehicle_selection_follow = 0;

    uint32_t *p = &channels_[0].trailing_words_[1];
    for (int i = 8; i != 0; i--) {
        *p = 0;
        p++;
    }

    locks_.clear();
    AlphaNet::close();

    primary_state_.flags_ = 0;
    primary_state_.leading_words_[2] = 0;

    if (AlphaNet::setup(option)) {
        return 1;
    }
    else {
        return 0;
    }
}

// Fixed-slot bindings carried from 005303b0.cpp

// ORIGINAL: 0x005303B0 ?wait@NetDaemon@@QAEHPADPAHHH@Z 0x005303B0-0x00530493 FILE
// symbol    ?wait@NetDaemon@@QAEHPADPAIHH@Z
// working copy - scaffold materialised by --work
// size      227 bytes
// prototype int (__thiscall ?wait@NetDaemon@@QAEHPADPAHHH@Z)(NetDaemon* this, int8*, int*, int, int)
// callers   2   call targets   7
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0047A7D0 0x0047A890 0x0051F1B0 0x00534400 0x005901D0 0x005FC700 0x00630A00
// indirect  0x005303CF 0x005303F9

int NetDaemon::wait(char * message, uint32_t * state, int value, int timeout) {
    int saved = NetDaemonNet.channels_[0].leading_words_[3];
    ++NetBusyCount;
    DWORD (WINAPI *const timeGetTimeFn)() = timeGetTime;
    DWORD startTime = timeGetTimeFn();
    bool triggeredTimeout = false;
    while (*state == static_cast<uint32_t>(value)) {
        if (ExitTurnLoop != 0) {
            break;
        }
        if (!triggeredTimeout) {
            DWORD now = timeGetTimeFn();
            // Original530405 SUB /530407 CMP /530409 JBE compares DWORD
            // bit patterns unsigned, including tick wrap and negative timeout.
            if (static_cast<DWORD>(now - startTime) > static_cast<DWORD>(timeout)) {
                triggeredTimeout = true;
                NetMsgPrimary.pop(message, 0, 2, 0);
            }
        }
        wait_task();
        unsigned long outVar;
        void *result = NetDaemonNet.Net::get(reinterpret_cast<unsigned long *>(&message), &outVar);
        if (result != 0) {
            process_message(static_cast<char *>(result), outVar, reinterpret_cast<int>(message));
        }
        locks_.check_global();
        check_spock();
    }
    if (triggeredTimeout) {
        NetMsgPrimary.close();
    }
    --NetBusyCount;
    NetDaemonNet.channels_[0].leading_words_[3] = saved;
    return 0;
}

// Fixed-slot bindings carried from 005304a0.cpp

// ORIGINAL: 0x005304A0 ?open_channel@NetDaemon@@QAEHHH@Z 0x005304A0-0x005305FF FILE
// TRIED: 0x00669368 is a function-pointer variable (timeGetTime's IAT slot), called indirectly rather than declared as a normal extern function - matches the disassembly's `call ebx` after `mov ebx,[0x669368]`. Net/NetMsg objects at 0x93cd90/0x805338 are fixed-address singletons. Landed at 0.85 mnemonic similarity.
// working copy - scaffold materialised by --work
// size      351 bytes
// prototype int (__thiscall ?open_channel@NetDaemon@@QAEHHH@Z)(NetDaemon* this, int, int)
// callers   ?   call targets   9
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0047A7D0 0x0047A890 0x0051F1B0 0x00534400 0x005901D0 0x00592EE0
//           0x005FC700 0x006262F0 0x00630A00


int NetDaemon::open_channel(int payload_10, int payload_14) {

  if (IsMultiplayerNet == 0) {
    return 0;
  }
  if (channels_[0].trailing_words_[9] != 0) {
    return 1;
  }

  log_say(reinterpret_cast<char *>("Client requesting Diplo Channel"), ConsoleGlobal.faction_id_, 0, 0);
  channels_[0].trailing_words_[9] = 1;
  channels_[1].leading_words_[0] = 0;

  message_data(0x8240, 0, payload_10, payload_14, 0, 0);

  DWORD (WINAPI *const timeGetTime_)() = timeGetTime;
  int savedE904 = NetDaemonNet.channels_[0].leading_words_[3];
  (NetBusyCount)++;
  payload_14 = static_cast<int>(timeGetTime_());

  bool sentAwait = false;
  if (channels_[1].leading_words_[0] == 0) {
    for (;;) {
      if (ExitTurnLoop != 0) {
        break;
      }
      if (!sentAwait) {
        int elapsed = static_cast<int>(timeGetTime_()) - payload_14;
        if (elapsed > 0x640) {
          sentAwait = true;
          NetMsgPrimary.pop(reinterpret_cast<const char *>("AWAITCHANNEL"), 0, 2, 0);
        }
      }
      wait_task();

      unsigned long local4;
      void *gotten = NetDaemonNet.Net::get(reinterpret_cast<unsigned long *>(&payload_10), &local4);
      if (gotten != 0) {
        process_message(static_cast<char *>(gotten), local4, payload_10);
      }
      locks_.check_global();
      check_spock();

      if (channels_[1].leading_words_[0] != 0) {
        break;
      }
    }
    if (sentAwait) {
      NetMsgPrimary.close();
    }
  }

  (NetBusyCount)--;
  NetDaemonNet.channels_[0].leading_words_[3] = savedE904;

  if (channels_[1].leading_words_[0] < 2) {
    return 0;
  }
  channels_[0].trailing_words_[9] = 0;
  return 1;
}

// Fixed-slot bindings carried from 00530630.cpp

// ORIGINAL: 0x00530630 ?lock@NetDaemon@@QAEHHHHHHH@Z 0x00530630-0x0053091E FILE
// TRIED: MISMATCH #2 'push' vs 'mov' - prologue register-save order (ebx/esi/edi) differs from a straight `mov esi, ecx` first statement; did not chase further given function size (750B, 4 nested wait-loops). Body reproduces both wait-for-lock loops (do/while + break on the global abort flag), the target/scratch/x/y stack-slot reuse as scratch locals matching the disassembly's post-use reuse, and the merged final NOLOCK/NOLOCKGLOBAL warning-pop path.
// working copy - scaffold materialised by --work
// size      750 bytes
// prototype int (__thiscall ?lock@NetDaemon@@QAEHHHHHHH@Z)(NetDaemon* this, int, int, int, int, int, int)
// callers   4   call targets   9
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0047A7D0 0x0047A890 0x0051F1B0 0x00534400 0x005901D0 0x00593090 0x005FC700 0x006262F0 0x00630A00
// indirect  0x005306EA 0x0053070F 0x005307E0 0x00530802

int NetDaemon::lock(int target, int x, int y, int payload_18, int payload_1C, int scratch) {

    if (IsMultiplayerNet == 0) {
        return 0;
    }
    NetPlayerDropped = 0;

    if (*(int *)(channels_[1].leading_words_[2]) != 0) {
        if (*(int *)(channels_[1].trailing_words_[3]) != 0) {
            return 1;
        }
        NetMsgPrimary.pop((const char *)"MOVEPENDING", 0, 2, (const char *)0);
        return 1;
    }

    *(int *)(channels_[1].leading_words_[2]) = 1;
    *(int *)(channels_[0].leading_words_[2]) = 0;
    *(int *)(locks_.field_E8_) = 0;

    log_say((char *)"Client requesting lock", ConsoleGlobal.faction_id_, x, y);
    message_lock(0x2210, target, x, y, payload_18, payload_1C, scratch, 0);

    NetBusyCount = NetBusyCount + 1;
    scratch = NetDaemonNet.channels_[0].leading_words_[3];
    unsigned long t0 = timeGetTime();
    int waited = 0;
    if (*(int *)(channels_[0].leading_words_[2]) == 0) {
        do {
            if (ExitTurnLoop != 0) {
                break;
            }
            if (!waited) {
                unsigned long now = timeGetTime();
                if (now - t0 > 0x640) {
                    waited = 1;
                    NetMsgPrimary.pop((const char *)"AWAITLOCK", 0, 2, (const char *)0);
                }
            }
            wait_task();
            void *got = NetDaemonNet.Net::get((unsigned long *)&y, (unsigned long *)&x);
            if (got != 0) {
                this->process_message(static_cast<char *>(got), (unsigned long)x, y);
            }
            locks_.check_global();
            check_spock();
        } while (*(int *)(channels_[0].leading_words_[2]) == 0);
        if (waited) {
            NetMsgPrimary.close();
        }
    }

    NetBusyCount = NetBusyCount - 1;
    NetDaemonNet.channels_[0].leading_words_[3] = scratch;

    if (*(int *)(channels_[0].leading_words_[2]) == 1) {
        *(int *)(channels_[1].trailing_words_[4]) = 0;
        if ((unsigned char)target & 2) {
            NetBusyCount = NetBusyCount + 1;
            scratch = NetDaemonNet.channels_[0].leading_words_[3];
            unsigned long t0b = timeGetTime();
            target = (int)t0b;
            int waited2 = 0;
            if (*(int *)(locks_.field_E8_) == 0) {
                do {
                    if (ExitTurnLoop != 0) {
                        break;
                    }
                    if (!waited2) {
                        unsigned long now2 = timeGetTime();
                        if (now2 - (unsigned long)target > 0x320) {
                            waited2 = 1;
                            NetMsgPrimary.pop((const char *)"AWAITGLOBALLOCK", 0, 2, (const char *)0);
                        }
                    }
                    wait_task();
                    void *got2 = NetDaemonNet.Net::get((unsigned long *)&y, (unsigned long *)&x);
                    if (got2 != 0) {
                        this->process_message(static_cast<char *>(got2), (unsigned long)x, y);
                    }
                    locks_.check_global();
                    check_spock();
                } while (*(int *)(locks_.field_E8_) == 0);
                if (waited2) {
                    NetMsgPrimary.close();
                }
            }
            NetDaemonNet.channels_[0].leading_words_[3] = scratch;
            NetBusyCount = NetBusyCount - 1;
        }
        if (NetPlayerDropped == 0) {
            return 0;
        }
        *(int *)(channels_[0].leading_words_[2]) = 2;
    }

    if (*(int *)(channels_[1].trailing_words_[3]) == 0) {
        *(int *)(channels_[1].trailing_words_[4]) = *(int *)(channels_[1].trailing_words_[4]) + 1;
        if (*(int *)(channels_[1].trailing_words_[4]) > 0 || *(int *)(channels_[0].leading_words_[2]) > 2) {
            const char *msg = (*(int *)(channels_[0].leading_words_[2]) == 2) ? (const char *)"NOLOCK" : (const char *)"NOLOCKGLOBAL";
            NetMsgPrimary.pop(msg, 5000, 2, (const char *)0);
            *(int *)(channels_[1].trailing_words_[4]) = 0;
        }
    }

    *(int *)(channels_[1].leading_words_[2]) = 0;
    return 1;
}

// Fixed-slot bindings carried from 00530920.cpp

// ORIGINAL: 0x00530920 ?add_lock@NetDaemon@@QAEHHHH@Z 0x00530920-0x00530A91 FILE
// TRIED: reordering the initial `IsMultiplayerNet==0` / `field_1BAC_==0` guards; the mismatch (#5) is deeper, in how the two early returns and the ebx-lazy-save prologue are scheduled.
// size      369 bytes
// prototype int (__thiscall ?add_lock@NetDaemon@@QAEHHHH@Z)(NetDaemon* this, int, int, int)
// callers   2   call targets   9
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0047A7D0 0x0047A890 0x0051F1B0 0x00534400 0x005901D0 0x00593090 0x005FC700 0x006262F0 0x00630A00
// indirect  0x005309A6 0x005309C6

int NetDaemon::add_lock(int target, int x, int y) {
    if (IsMultiplayerNet == 0) {
        return 0;
    }
    if (channels_[1].leading_words_[2] == 0) {
        return 1;
    }

    log_say(reinterpret_cast<char *>("Client requesting ADD LOCK"), ConsoleGlobal.faction_id_, 0, 0);
    channels_[0].leading_words_[2] = 0;
    message_lock(0x2215, target, x, y, 0, -1, -1, 0);

    int savedVal = NetDaemonNet.channels_[0].leading_words_[3];
    DWORD (WINAPI *const timeGetTimeFn)() = timeGetTime;
    ++NetBusyCount;
    y = timeGetTimeFn();

    bool popped = false;
    NetMsg *netMsg = &NetMsgPrimary;
    Lock &lock = locks_;

    if (channels_[0].leading_words_[2] == 0) {
        do {
            if (ExitTurnLoop != 0) {
                break;
            }
            if (!popped) {
                if (static_cast<unsigned int>(timeGetTimeFn() - y) > 0x640) {
                    popped = true;
                    netMsg->pop(reinterpret_cast<const char *>("AWAITLOCK"), 0, 2, 0);
                }
            }
            wait_task();
            void *msgResult = NetDaemonNet.Net::get(reinterpret_cast<unsigned long *>(&x),
                                                reinterpret_cast<unsigned long *>(&target));
            if (msgResult != 0) {
                process_message(static_cast<char *>(msgResult),
                                 static_cast<unsigned long>(target), x);
            }
            lock.check_global();
            check_spock();
        } while (channels_[0].leading_words_[2] == 0);
        if (popped) {
            netMsg->close();
        }
    }

    --NetBusyCount;
    NetDaemonNet.channels_[0].leading_words_[3] = savedVal;
    if (channels_[0].leading_words_[2] != 1) {
        netMsg->pop(reinterpret_cast<const char *>("NOLOCK"), 5000, 2, 0);
    }
    return channels_[0].leading_words_[2] != 1;
}

// Fixed-slot bindings carried from 00530aa0.cpp

// ORIGINAL: 0x00530AA0 ?add_global@NetDaemon@@QAEHXZ 0x00530AA0-0x00530CFF FILE
// TRIED: translated the Ghidra hypothesis directly (two near-identical wait-for-global-lock phases, each: warn-once via NetMsg::pop after a timeGetTime() timeout, Net::get()/process_message() poll loop, Lock::check_global()+check_spock(), then NetMsg::close() if warned). MISMATCH #5 xor/mov - looks like register allocation for the ebx=0 constant, not a structural difference; body compiles and follows the disasm's two-phase shape and final NOLOCK/NOLOCKGLOBAL message selection exactly.
// working copy - scaffold materialised by --work
// size      607 bytes
// prototype int (__thiscall ?add_global@NetDaemon@@QAEHXZ)(NetDaemon* this)
// callers   12   call targets   8
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0047A7D0 0x0047A890 0x0051F1B0 0x00534400 0x005901D0 0x00592EE0 0x005FC700 0x00630A00
// indirect  0x00530AF8 0x00530B1D 0x00530BC1 0x00530BE6

int NetDaemon::add_global() {
    DWORD (WINAPI *const get_time)() = timeGetTime;

    if (IsMultiplayerNet == 0) {
        return 0;
    }
    NetPlayerDropped = 0;
    channels_[0].leading_words_[2] = 0;
    locks_.field_E8_ = 0;
    message_data(0x2211, 0, 0, 0, 0, 0);
    NetBusyCount = NetBusyCount + 1;
    int saved_e904 = NetDaemonNet.channels_[0].leading_words_[3];
    unsigned long start_time1 = get_time();

    bool warned1 = false;
    if (channels_[0].leading_words_[2] == 0) {
        do {
            if (ExitTurnLoop != 0) break;
            if (!warned1) {
                unsigned long now = get_time();
                if (now - start_time1 > 0x640) {
                    warned1 = true;
                    NetMsgPrimary.pop(reinterpret_cast<const char *>("AWAITGLOBALLOCK"), 0, 2, 0);
                }
            }
            wait_task();
            unsigned long out_id, out_len;
            void *result = NetDaemonNet.Net::get(&out_id, &out_len);
            if (result != 0) {
                process_message(static_cast<char *>(result), out_len, static_cast<int>(out_id));
            }
            locks_.check_global();
            check_spock();
        } while (channels_[0].leading_words_[2] == 0);
        if (warned1) {
            NetMsgPrimary.close();
        }
    }
    NetDaemonNet.channels_[0].leading_words_[3] = saved_e904;
    int newE664 = NetBusyCount - 1;

    if (channels_[0].leading_words_[2] == 1) {
        unsigned long start_time2 = get_time();
        bool warned2 = false;
        if (locks_.field_E8_ == 0) {
            do {
                if (ExitTurnLoop != 0) break;
                if (!warned2) {
                    unsigned long now = get_time();
                    if (now - start_time2 > 800) {
                        warned2 = true;
                        NetMsgPrimary.pop(reinterpret_cast<const char *>("AWAITGLOBALLOCK"), 0, 2, 0);
                    }
                }
                wait_task();
                unsigned long out_id2, out_len2;
                void *result2 = NetDaemonNet.Net::get(&out_id2, &out_len2);
                if (result2 != 0) {
                    process_message(static_cast<char *>(result2), out_len2, static_cast<int>(out_id2));
                }
                locks_.check_global();
                check_spock();
            } while (locks_.field_E8_ == 0);
            if (warned2) {
                NetMsgPrimary.close();
            }
        }
        NetDaemonNet.channels_[0].leading_words_[3] = saved_e904;
        NetBusyCount = NetBusyCount - 1;
        if (NetPlayerDropped == 0) {
            channels_[1].trailing_words_[4] = 0;
            return 0;
        }
        channels_[0].leading_words_[2] = 2;
        newE664 = NetBusyCount;
    }
    NetBusyCount = newE664;

    if (channels_[1].trailing_words_[3] == 0) {
        int newBc8 = channels_[1].trailing_words_[4] + 1;
        channels_[1].trailing_words_[4] = newBc8;
        if (newBc8 > 0 || channels_[0].leading_words_[2] > 2) {
            const char *msg = reinterpret_cast<const char *>("NOLOCK");
            if (channels_[0].leading_words_[2] != 2) {
                msg = reinterpret_cast<const char *>("NOLOCKGLOBAL");
            }
            NetMsgPrimary.pop(msg, 5000, 2, 0);
            channels_[1].trailing_words_[4] = 0;
        }
    }
    channels_[1].leading_words_[2] = 0;
    return 1;
}

// Fixed-slot bindings carried from 00530e50.cpp

// ORIGINAL: 0x00530E50 ?unlock_base@NetDaemon@@QAEXH@Z 0x00530E50-0x0053101F FILE
// size      463 bytes
// prototype void (__thiscall ?unlock_base@NetDaemon@@QAEXH@Z)(NetDaemon* this, int)
// callers   26   call targets   10
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0047A7D0 0x0047A890 0x0051F1B0 0x00534400 0x005901D0 0x00592EE0 0x00593250 0x005FC700 0x006262F0 0x00630A00
// indirect  0x00530EED 0x00530F13
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00530E50
// measured tier  MISMATCH
// divergence     88
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00530e50/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?unlock_base@NetDaemon@@QAEXH@Z  at 0x00530E50  (463 bytes)
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

void NetDaemon::unlock_base(int base_id) {

    log_say(reinterpret_cast<char *>("UnlockBase"), NetDaemonNet.channels_[1].leading_words_[2], base_id, 0);

    if (IsMultiplayerNet != 0 && NetDaemonNet.channels_[1].leading_words_[2] != 0) {
        log_say(reinterpret_cast<char *>("UnlockBase doing synch & unlock"), 0, 0, 0);
        synch_base(base_id);

        if (IsMultiplayerNet != 0) {
            channels_[1].trailing_words_[0] = 1;
            log_say(reinterpret_cast<char *>("Awaiting acknowledgement of SYNCH"), ConsoleGlobal.faction_id_, 0, 0);
            int saved = NetDaemonNet.channels_[0].leading_words_[3];
            ++NetBusyCount;
            unsigned long baseline = timeGetTime();

            int logged_wait = 0;
            if (channels_[1].trailing_words_[0] == 1) {
                do {
                    if (ExitTurnLoop != 0) {
                        break;
                    }
                    if (logged_wait == 0) {
                        if (timeGetTime() - baseline > 0x7d0) {
                            logged_wait = 1;
                            NetMsgPrimary.pop(
                                reinterpret_cast<char *>("AWAITEXEC"), 0, 2, 0);
                        }
                    }
                    wait_task();
                    unsigned long p1, p2;
                    void *r = NetDaemonNet.Net::get(&p1, &p2);
                    if (r != 0) {
                        process_message(static_cast<char *>(r), p2, p1);
                    }
                    locks_.check_global();
                    check_spock();
                } while (channels_[1].trailing_words_[0] == 1);
                if (logged_wait) {
                    NetMsgPrimary.close();
                }
            }

            --NetBusyCount;
            NetDaemonNet.channels_[0].leading_words_[3] = saved;
            if (channels_[1].trailing_words_[0] == 0) {
                log_say(reinterpret_cast<char *>("Received SYNCH acknowledgement"), ConsoleGlobal.faction_id_, 0, 0);
            }
            channels_[1].trailing_words_[0] = 0;

            if (IsMultiplayerNet != 0) {
                log_say(reinterpret_cast<char *>("Client releasing lock"), ConsoleGlobal.faction_id_, 0, 0);
                message_data(0x2212, 0, 0, 0, 0, 0);
                channels_[1].leading_words_[3] = 0;
                channels_[1].leading_words_[2] = 0;
                channels_[1].trailing_words_[5] = 0;
                channels_[1].trailing_words_[4] = 0;
            }
        }
    }
}

// Fixed-slot bindings carried from 00531300.cpp

// ORIGINAL: 0x00531300 ?await_exec@NetDaemon@@QAEXH@Z 0x00531300-0x00531477 FILE
// size      375 bytes
// prototype void (__thiscall ?await_exec@NetDaemon@@QAEXH@Z)(NetDaemon* this, int)
// callers   17   call targets   9
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0047A7D0 0x0047A890 0x0051F1B0 0x00534400 0x005901D0 0x00592EE0 0x005FC700 0x006262F0 0x00630A00
// indirect  0x00531345 0x0053136B
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00531300
// measured tier  MISMATCH
// divergence     18
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00531300/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?await_exec@NetDaemon@@QAEXH@Z  at 0x00531300  (375 bytes)
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


void NetDaemon::await_exec(int opcode) {

    if (IsMultiplayerNet == 0) {
        return;
    }

    log_say(reinterpret_cast<char *>("Client waiting for acknowledgement"), ConsoleGlobal.faction_id_, 0, 0);
    int saved = NetDaemonNet.channels_[0].leading_words_[3];
    ++NetBusyCount;
    unsigned long baseline = timeGetTime();

    int logged_wait = 0;
    if (channels_[1].leading_words_[3] == 1) {
        do {
            if (ExitTurnLoop != 0) {
                break;
            }
            if (logged_wait == 0) {
                if (timeGetTime() - baseline > 0x7d0) {
                    logged_wait = 1;
                    NetMsgPrimary.pop(
                        reinterpret_cast<char *>("AWAITEXEC"), 0, 2, 0);
                }
            }
            wait_task();
            unsigned long p1, p2;
            void *r = NetDaemonNet.Net::get(&p1, &p2);
            if (r != 0) {
                process_message(static_cast<char *>(r), p2, p1);
            }
            locks_.check_global();
            check_spock();
        } while (channels_[1].leading_words_[3] == 1);
        if (logged_wait) {
            NetMsgPrimary.close();
        }
    }

    --NetBusyCount;
    NetDaemonNet.channels_[0].leading_words_[3] = saved;
    log_say(reinterpret_cast<char *>("Client received acknowledgement"), ConsoleGlobal.faction_id_, 0, 0);

    if (opcode != 0) {
        if (IsMultiplayerNet != 0) {
            log_say(reinterpret_cast<char *>("Client releasing lock"), ConsoleGlobal.faction_id_, 0, 0);
            message_data(0x2212, 0, 0, 0, 0, 0);
            channels_[1].leading_words_[3] = 0;
            channels_[1].leading_words_[2] = 0;
            channels_[1].trailing_words_[5] = 0;
            channels_[1].trailing_words_[4] = 0;
        }
        channels_[0].payload_.vehicle_selection_follow = 0;
        channels_[1].trailing_words_[3] = 0;
    }
    channels_[1].leading_words_[3] = 0;
}

// ORIGINAL: 0x00531480 ?await_synch@NetDaemon@@QAEXXZ 0x00531480-0x005315B6 FILE
// size      310 bytes
// prototype void (__thiscall ?await_synch@NetDaemon@@QAEXXZ)(NetDaemon* this)
// callers   22   call targets   8
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0047A7D0 0x0047A890 0x0051F1B0 0x00534400 0x005901D0 0x005FC700 0x006262F0 0x00630A00
// indirect  0x005314D0 0x005314F6
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00531480
// measured tier  MISMATCH
// divergence     19
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00531480/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?await_synch@NetDaemon@@QAEXXZ  at 0x00531480  (310 bytes)
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


void NetDaemon::await_synch() {
    uint32_t *state = &channels_[1].trailing_words_[0];

    if (IsMultiplayerNet != 0) {
        int synching = 1;
        *state = synching;
        log_say(reinterpret_cast<char *>("Awaiting acknowledgement of SYNCH"), ConsoleGlobal.faction_id_, 0, 0);

        int saved = NetDaemonNet.channels_[0].leading_words_[3];
        (NetBusyCount)++;
        unsigned long start_time = timeGetTime();

        int warned = 0;
        if (*state == synching) {
            do {
                if (ExitTurnLoop != 0) {
                    break;
                }
                if (!warned) {
                    unsigned long now =
                        timeGetTime();
                    if (now - start_time > 0x7d0) {
                        warned = synching;
                        NetMsgPrimary.pop(
                            reinterpret_cast<const char *>("AWAITEXEC"), 0, 2, 0);
                    }
                }
                wait_task();
                unsigned long v1, v2;
                void *r = NetDaemonNet.Net::get(&v1, &v2);
                if (r != 0) {
                    process_message(static_cast<char *>(r), v2,
                                    static_cast<int>(v1));
                }
                locks_.check_global();
                check_spock();
            } while (*state == synching);
            if (warned) {
                NetMsgPrimary.close();
            }
        }

        (NetBusyCount)--;
        NetDaemonNet.channels_[0].leading_words_[3] = saved;
        if (*state == 0) {
            log_say(reinterpret_cast<char *>("Received SYNCH acknowledgement"), ConsoleGlobal.faction_id_, 0, 0);
        }
        *state = 0;
    }
}

// Fixed-slot bindings carried from 005315c0.cpp

// ORIGINAL: 0x005315C0 ?await_diplo@NetDaemon@@QAEXH@Z 0x005315C0-0x005316F9 FILE
// size      313 bytes
// prototype void (__thiscall ?await_diplo@NetDaemon@@QAEXH@Z)(NetDaemon* this, int)
// callers   35   call targets   8
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0047A7D0 0x0047A890 0x0051F1B0 0x00534400 0x005901D0 0x005FC700 0x006262F0 0x00630A00
// indirect  0x0053160E 0x00531634
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x005315C0
// measured tier  MISMATCH
// divergence     20
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005315c0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?await_diplo@NetDaemon@@QAEXH@Z  at 0x005315C0  (313 bytes)
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


void NetDaemon::await_diplo(int opcode) {
    if (IsMultiplayerNet != 0) {
        channels_[1].leading_words_[1] = opcode;
        log_say(reinterpret_cast<char *>("Awaiting DIPLO event"), ConsoleGlobal.faction_id_, 0, 0);
        (NetBusyCount)++;
        int32_t saved = NetDaemonNet.channels_[0].leading_words_[3];
        unsigned int start_time = timeGetTime();
        int timed_out = 0;
        if (channels_[1].leading_words_[1] == opcode) {
            do {
                if (ExitTurnLoop != 0) {
                    break;
                }
                if (!timed_out) {
                    unsigned int now = timeGetTime();
                    if (now - start_time > 0x7d0) {
                        timed_out = 1;
                        NetMsgPrimary.pop(
                            reinterpret_cast<const char *>("AWAITDIPLO"), 0, 2, 0);
                    }
                }
                wait_task();
                unsigned long v_at_4;
                unsigned long v_at_8;
                void *result = NetDaemonNet.Net::get(&v_at_4, &v_at_8);
                if (result != 0) {
                    process_message(static_cast<char *>(result), v_at_8, v_at_4);
                }
                locks_.check_global();
                check_spock();
            } while (channels_[1].leading_words_[1] == opcode);
            if (timed_out) {
                NetMsgPrimary.close();
            }
        }
        (NetBusyCount)--;
        NetDaemonNet.channels_[0].leading_words_[3] = saved;
        if (channels_[1].leading_words_[1] == 0) {
            log_say(reinterpret_cast<char *>("Received DIPLO acknowledgement"), ConsoleGlobal.faction_id_, 0, 0);
        }
        channels_[1].leading_words_[1] = 0;
    }
}


// ORIGINAL: 0x00534330 ?fixup_message@NetDaemon@@QAEHPAD@Z 0x00534330-0x005343FC FILE
// TRIED: MISMATCH, closest at 212/204 bytes (mnemonic_similarity 0.87, 3 edit blocks). `goto fail;`/`fail:` in place of five scattered `return 0;` was needed to get VC6 to share ONE epilogue the way the original's disassembly jumps into a single shared block - plain early returns each got their own copy. `int code` (signed) not `unsigned short`/`unsigned int` reproduces the `cmp eax,CONST` 32-bit signed compares (jg/jge) the original uses, and writing the boundaries as `<= 0x2406`/`<= 0x2402` (not `< 0x2407`/`< 0x2403`) matches the exact `jg` vs `jge` original picked for each - the `x<N` vs `x<=N-1` lever. `DeletionList*` for `this+0x158c` inlined at each of the three call sites (not cached in an upfront local) matches the original computing it lazily, right before its first use. Remaining diff: the `code== 0x2403` arm compiles to `jne fail; jmp common` where the original does a single `je common`, and the `0x2408 || 0x2413` pair still compiles as two `cmp/je` where the original uses a `sub eax,0x2408; je; sub eax,0xb; jne` switch-lowering idiom - not chased further.
// working copy - scaffold materialised by --work
// size      204 bytes
// prototype int (__thiscall ?fixup_message@NetDaemon@@QAEHPAD@Z)(NetDaemon* this, int8*)
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0052DDC0

int NetDaemon::fixup_message(char * message) {
    unsigned short *msg = reinterpret_cast<unsigned short *>(message);
    int code = *msg;
    if (code <= 0x2406) {
        if (code < 0x2404) {
            if (code <= 0x2402) {
                if (code < 0x2401) {
                    if (code != 0x2101) {
                        goto fail;
                    }
                    if (msg[8] != 0x11) {
                        goto fail;
                    }
                    int r1 = deletion_list_.convert(*reinterpret_cast<int *>(msg + 10), *reinterpret_cast<int *>(msg + 6));
                    *reinterpret_cast<int *>(msg + 10) = r1;
                    if (*(reinterpret_cast<char *>(msg) + 0x35) != 1) {
                        goto fail;
                    }
                    if (static_cast<short>(msg[0x1c]) < 0) {
                        goto fail;
                    }
                    int r2 = deletion_list_.convert(static_cast<short>(msg[0x1c]), *reinterpret_cast<int *>(msg + 6));
                    unsigned short r2s = static_cast<unsigned short>(r2);
                    msg[0x1c] = r2s;
                    if (r2s != 0xffff) {
                        goto fail;
                    }
                    *(reinterpret_cast<char *>(msg) + 0x35) = 0;
                    goto fail;
                }
            } else if (code != 0x2403) {
                goto fail;
            }
        }
    } else if (code != 0x2408 && code != 0x2413) {
        goto fail;
    }
    {
        int r3 = deletion_list_.convert(*reinterpret_cast<int *>(msg + 8), *reinterpret_cast<int *>(msg + 6));
        *reinterpret_cast<int *>(msg + 8) = r3;
        if (r3 >= 0) {
            goto fail;
        }
        return 1;
    }
fail:
    return 0;
}

// File-transfer protocol state and storage, shared by the sender and receiver.

extern char NetDaemonFileTransferPath[0x104];


struct NetDaemonFileIndexRecord {
    uint16_t opcode;          // +0
    uint16_t reserved_2;      // +2
    int32_t source_player;    // +4
    uint32_t ready_flags;     // +8
    uint32_t chunk_count;     // +0xC
    uint8_t file_index;       // +0x10
    uint8_t checksum;         // +0x11
    uint16_t reserved_12;     // +0x12
    uint32_t file_size;       // +0x14
    char filename[0x104];     // +0x18
};
static_assert(sizeof(NetDaemonFileIndexRecord) == 0x11C,
              "native file-list message stride");
static_assert(offsetof(NetDaemonFileIndexRecord, source_player) == 0x04,
              "native file-list sender offset");
static_assert(offsetof(NetDaemonFileIndexRecord, ready_flags) == 0x08,
              "native file-list ready-mask offset");
static_assert(offsetof(NetDaemonFileIndexRecord, chunk_count) == 0x0C,
              "native file-list chunk-count offset");
static_assert(offsetof(NetDaemonFileIndexRecord, file_index) == 0x10,
              "native file-list file-index offset");
static_assert(offsetof(NetDaemonFileIndexRecord, checksum) == 0x11,
              "native file-list checksum offset");
static_assert(offsetof(NetDaemonFileIndexRecord, file_size) == 0x14,
              "native file-list file-size offset");
static_assert(offsetof(NetDaemonFileIndexRecord, filename) == 0x18,
              "native file-list filename offset");

// Native F05 writes its path at +0x18 (0x52EE83..0x52EEAA).
// Only opcode and path are populated; the intervening bytes stay untouched.
struct NetDaemonFilePathPacket {
    uint16_t opcode;
    uint8_t reserved_2[0x16];
    char path[0x104];
};
static_assert(offsetof(NetDaemonFilePathPacket, path) == 0x18,
              "native transfer path offset");
static_assert(sizeof(NetDaemonFilePathPacket) == 0x11C,
              "native transfer path packet extent");

// F0F writes source_player at +4; F07 writes state at +8, not at +2.
// Each operation leaves all other payload/header bytes untouched.
struct NetDaemonFileControlPacket {
    uint16_t opcode;
    uint16_t reserved_2;
    int32_t source_player;
    int32_t state;
    uint8_t reserved_C[0x14];
};
static_assert(offsetof(NetDaemonFileControlPacket, source_player) == 4,
              "native transfer acknowledgement player offset");
static_assert(offsetof(NetDaemonFileControlPacket, state) == 8,
              "native transfer completion state offset");
static_assert(sizeof(NetDaemonFileControlPacket) == 0x20,
              "native transfer control packet extent");

struct NetDaemonFileChunkPacket {
    uint16_t opcode;          // +0
    uint16_t reserved_2;      // +2
    uint32_t compressed_size; // +4
    int32_t file_index;       // +8 (native sendBuf + 4, in uint16_t units)
    uint32_t chunk_index;     // +0xC
    uint8_t reserved_10[0x10];// +0x10
    uint8_t payload[0x800];   // +0x20
};
static_assert(sizeof(NetDaemonFileChunkPacket) == 0x820,
              "native file transfer packet extent");

static NetDaemonFileIndexRecord *netdaemon_allocate_file_index_records() {
    void *const storage = mem_get(0x11C00);
    if (storage == 0) return 0;
    NetDaemonFileIndexRecord *const records =
        static_cast<NetDaemonFileIndexRecord *>(storage);
    for (size_t index = 0; index < 256; ++index) {
        ::new (records + index) NetDaemonFileIndexRecord;
    }
    return records;
}

static_assert(sizeof(void *) == sizeof(uint32_t),
              "the native transfer pointer word is 32-bit");

static NetDaemonFileIndexRecord *netdaemon_file_index_records() {
    return static_cast<NetDaemonFileIndexRecord *>(NetDaemonNet.file_index_records_);
}

static void netdaemon_set_file_index_records(NetDaemonFileIndexRecord *records) {
    NetDaemonNet.file_index_records_ = records;
}
// RECOVERY_OBJECT: v1 0x0093F79C ?NetDaemonFileTransferMode@@3HA
int NetDaemonFileTransferMode;
// RECOVERY_OBJECT: v1 0x0093F7A0 ?NetDaemonFileTransferAckMask@@3IA
uint32_t NetDaemonFileTransferAckMask;
// RECOVERY_OBJECT: v1 0x0093F7A4 ?NetDaemonFileTransferCurrentFile@@3HA
int NetDaemonFileTransferCurrentFile;
// RECOVERY_OBJECT: v1 0x0093F7A8 ?NetDaemonFileTransferChunkCount@@3HA
int NetDaemonFileTransferChunkCount;
// RECOVERY_OBJECT: v1 0x0093F7AC ?NetDaemonFileTransferProgress@@3HA
int NetDaemonFileTransferProgress;
// RECOVERY_OBJECT: v1 0x0093F7B0 ?NetDaemonFileTransferReceiveBuffer@@3PAEA
unsigned char *NetDaemonFileTransferReceiveBuffer;

// Declare this guard between Popup and Filemap in process_message. On an
// armed return, reverse lexical destruction does Filemap exactly once, then
// this guard closes Popup, then Popup's ordinary destructor runs. This
// reproduces the native explicit cleanup order without ending an automatic
// object's lifetime twice.
class NetDaemonTransferModalCloseGuard {
 public:
    explicit NetDaemonTransferModalCloseGuard(Popup &popup)
        : popup_(&popup), armed_(false) {}
    ~NetDaemonTransferModalCloseGuard() {
        if (armed_) popup_->close();
    }
    void arm() { armed_ = true; }
 private:
    Popup *popup_;
    bool armed_;
};

static uint16_t netdaemon_transfer_read_u16(const unsigned char *bytes) {
    uint16_t value;
    memcpy(&value, bytes, sizeof value);
    return value;
}

static uint32_t netdaemon_transfer_read_u32(const unsigned char *bytes) {
    uint32_t value;
    memcpy(&value, bytes, sizeof value);
    return value;
}

static int32_t netdaemon_transfer_read_i32(const unsigned char *bytes) {
    int32_t value;
    memcpy(&value, bytes, sizeof value);
    return value;
}

static void netdaemon_transfer_write_u16(unsigned char *bytes,
                                         uint16_t value) {
    memcpy(bytes, &value, sizeof value);
}

static void netdaemon_transfer_write_u32(unsigned char *bytes,
                                         uint32_t value) {
    memcpy(bytes, &value, sizeof value);
}

static void netdaemon_transfer_write_i32(unsigned char *bytes,
                                         int32_t value) {
    memcpy(bytes, &value, sizeof value);
}

static bool netdaemon_transfer_modal_cleanup(
    NetDaemonTransferModalCloseGuard &guard) {
    guard.arm();
    return true;
}

bool NetDaemon::process_file_transfer_message(char *message,
                                             unsigned long message_size,
                                             int sender_id,
                                             NetDaemonTransferModalCloseGuard &guard,
                                             Filemap &filemap) {
    unsigned char *const bytes = reinterpret_cast<unsigned char *>(message);
    const uint16_t type = netdaemon_transfer_read_u16(bytes);
    const bool local_is_host = primary_state_.leading_words_[0] != 0;
    NetDaemonFileIndexRecord *records = netdaemon_file_index_records();

    if (type == 0xF09) { // decimal 3849: native shared-default route
        if ((bytes[1] & 0xA0U) != 0) {
            if (fixup_message(message) == 0) {
                const uint16_t forwarded = static_cast<uint16_t>(
                    (type & 0x1FFFU) | 0x4000U);
                netdaemon_transfer_write_u16(bytes, forwarded);
                deletion_list_.advance_sequence();
                netdaemon_transfer_write_u32(bytes + 0x0C,
                                             deletion_list_.sequence_bits());
                send_message(message, static_cast<int>(message_size), 0);
            }
        }
        return true;
    }

    if (type == 0xF0D) { // decimal 3853: faction-data owner
        NetWinDialog.setup_faction_data(
            reinterpret_cast<MessageFactionData *>(message));
        return true;
    }

    if (type == 0xF0E) { // decimal 3854: visible-dialog pass callback
        if (NetWinDialog.is_visible()) NetWinDialog.pick_pass(1);
        return true;
    }

    if (type == 0xF05) { // decimal 3845: transfer path from host to client
        if (local_is_host) return netdaemon_transfer_modal_cleanup(guard);
        NetDaemonFileTransferMode = 1;
        NetDaemonFileTransferCurrentFile = -1;
        NetDaemonFileTransferProgress = 0;
        if (records == 0) {
            records = netdaemon_allocate_file_index_records();
            netdaemon_set_file_index_records(records);
        }
        NetDaemonFileTransferPath[0] = '\0';
        strcat(NetDaemonFileTransferPath,
               reinterpret_cast<const char *>(bytes + 0x18));
        return true;
    }

    if (type == 0xF06) { // decimal 3846: host file-list record
        if (local_is_host) return netdaemon_transfer_modal_cleanup(guard);
        if (checksum_file(reinterpret_cast<char *>(bytes + 0x18)) ==
            bytes[0x11]) {
            return true;
        }

        ++NetDaemonFileTransferProgress;
        const unsigned int file_index = bytes[0x10];
        NetDaemonFileTransferCurrentFile = static_cast<int>(file_index);
        NetDaemonFileTransferChunkCount =
            netdaemon_transfer_read_u32(bytes + 0x0C);

        const int local_index = NetDaemonNet.pid_2_idx(
            NetDaemonNet.field_760_);
        netdaemon_transfer_write_u16(bytes, 0xF08);
        netdaemon_transfer_write_i32(bytes + 4, local_index);
        send_message(message, 0x14, sender_id);

        // The list is created by F05. Native does not guard file_index against
        // 256 records; keep that original domain rather than silently clamp.
        records = netdaemon_file_index_records();
        NetDaemonFileIndexRecord &record = records[file_index];
        record.filename[0] = '\0';
        strcat(record.filename, reinterpret_cast<const char *>(bytes + 0x18));
        record.file_size = netdaemon_transfer_read_u32(bytes + 0x14);
        record.chunk_count = netdaemon_transfer_read_u32(bytes + 0x0C);

        filemap.create(record.filename, record.file_size, 1);
        filemap.close();
        return true;
    }

    if (type == 0xF07) { // decimal 3847: mode completion/advance
        if (local_is_host) return netdaemon_transfer_modal_cleanup(guard);
        if (NetDaemonFileTransferCurrentFile < 0) {
            netdaemon_transfer_write_u16(bytes, 0xF0C);
            records = netdaemon_file_index_records();
            if (records != 0) {
                free(records);
                netdaemon_set_file_index_records(0);
            }
        } else {
            netdaemon_transfer_write_u16(bytes, 0xF0A);
        }
        netdaemon_transfer_write_i32(bytes + 4, NetDaemonNet.pid_2_idx(
            NetDaemonNet.field_760_));
        send_message(message, 0x20, sender_id);
        // Native reloads this dword after send_message; preserve callback
        // mutation rather than caching its value across the outgoing call.
        NetDaemonFileTransferMode = netdaemon_transfer_read_i32(bytes + 8);
        return true;
    }

    if (type == 0xF08) { // decimal 3848: peer ready for one file
        if (!local_is_host) return netdaemon_transfer_modal_cleanup(guard);
        const unsigned int file_index = bytes[0x10];
        const unsigned int slot = netdaemon_transfer_read_u32(bytes + 4);
        records = netdaemon_file_index_records();
        records[file_index].ready_flags |= 1U << (slot & 31U);
        return true;
    }

    if (type == 0xF0A || type == 0xF0C) { // decimal 3850 / 3852
        const uint32_t slot = netdaemon_transfer_read_u32(bytes + 4);
        process_slots_[slot].status_bytes_[0] =
            static_cast<uint8_t>(type == 0xF0A ? 1 : 2);
        return true;
    }

    if (type == 0xF0B) { // decimal 3851: compressed file data
        if (local_is_host) return netdaemon_transfer_modal_cleanup(guard);
        records = netdaemon_file_index_records();
        if (records == 0) return true;

        const uint32_t file_index = netdaemon_transfer_read_u32(bytes + 8);
        const uint32_t chunk_index = netdaemon_transfer_read_u32(bytes + 0x0C);
        if (chunk_index == 0) {
            if (NetDaemonFileTransferReceiveBuffer != 0) {
                free(NetDaemonFileTransferReceiveBuffer);
                NetDaemonFileTransferReceiveBuffer = 0;
            }
            const uint32_t declared_size = records[file_index].file_size;
            NetDaemonFileTransferReceiveBuffer =
                static_cast<unsigned char *>(mem_get(static_cast<int>(declared_size)));
            const uint32_t compressed_size =
                netdaemon_transfer_read_u32(bytes + 4);
            NetDaemonFileTransferChunkCount =
                static_cast<int>((compressed_size + 0x800U) >> 11);
            records[file_index].chunk_count =
                static_cast<uint32_t>(NetDaemonFileTransferChunkCount);

            parse_says(0, records[file_index].filename, -1, -1);
            parse_num(0, --NetDaemonFileTransferProgress);
            PopupModalState = 6;
            pop_close();
            x_pop_caption_flags("SCRIPT", "MPSENDFILE", 0x214200, 0);
            PopupModalState = 0;
        }

        // Preserve native packet size arithmetic (message bytes minus header)
        // and the original unchecked chunk offset/capacity behavior.
        memcpy(NetDaemonFileTransferReceiveBuffer + (chunk_index << 11),
               bytes + 0x20, static_cast<size_t>(message_size - 0x20));

        if (records[file_index].chunk_count == chunk_index + 1) {
            uLongf decompressed_size = records[file_index].file_size;
            const uLong compressed_size = netdaemon_transfer_read_u32(bytes + 4);
            unsigned char *const mapped = static_cast<unsigned char *>(
                filemap.create(records[file_index].filename,
                               decompressed_size, 1));
            netdaemon_inflate_buffer(mapped, &decompressed_size,
                                    NetDaemonFileTransferReceiveBuffer,
                                    static_cast<unsigned int>(compressed_size));
            filemap.close();
            if (NetDaemonFileTransferReceiveBuffer != 0) {
                free(NetDaemonFileTransferReceiveBuffer);
                NetDaemonFileTransferReceiveBuffer = 0;
            }
        }

        if (static_cast<int>(file_index) == NetDaemonFileTransferCurrentFile &&
            records[file_index].chunk_count == chunk_index + 1) {
            netdaemon_transfer_write_u16(bytes, 0xF0C);
            netdaemon_transfer_write_i32(bytes + 4, NetDaemonNet.pid_2_idx(
                NetDaemonNet.field_760_));
            send_message(message, 0x20, sender_id);
            records = netdaemon_file_index_records();
            if (records != 0) {
                free(records);
                netdaemon_set_file_index_records(0);
            }
            pop_close();
        }
        return true;
    }

    if (type == 0xF0F) { // decimal 3855: host handshake acknowledgement
        if (!local_is_host) return netdaemon_transfer_modal_cleanup(guard);
        const uint32_t slot = netdaemon_transfer_read_u32(bytes + 4);
        NetDaemonFileTransferAckMask |= 1U << (slot & 31U);
        return true;
    }
    return false;
}
// ORIGINAL: 0x0052ED40 ?send_files@NetDaemon@@QAEHXZ 0x0052ED40-0x0052F517;0x0065E0B0-0x0065E0C2
// RECOVERY_BINDING: v1 {"evidence": ["Pinned52ED40..52F517 and cold65E0B0..65E0C2; retained git:b0d788d3e0ebc4d5b4c20acb834cd37455e53c67:src/unrecovered/0052ed40.cpp.", "Coupled F05..F0F receiver recovery and Net group methods are reviewed together under netdaemon-player-lifecycle-recovery; native protocol anomalies remain explicit."], "reason": "Recover the complete file-transfer producer on the genuine daemon, player, group and file-map owners; packet intervals retain the native protocol rather than fixed executable aliases."}
// TRIED: automatic `Filemap fm;` local (real ctor/dtor, matching the draw_facilities Font precedent) for the compiler-generated SEH frame; retyped the scaffold's `_mkdir`/`sub_634280` externs (declared nullary) to the params the call sites need; IAT slots identified by matching Ghidra's WinAPI names to arg count/order: 0x669110 GetModuleFileNameA, 0x6690e4 GetCurrentDirectoryA, 0x669114 SetCurrentDirectoryA, 0x6690e0 FindFirstFileA, 0x669118 FindNextFileA, 0x66911c FindClose, 0x669130 GetFileSize, 0x669330 GetAsyncKeyState. Ghidra's local_50/uStack_4c/local_48 naming for the FILD operand looked offset from the real ebp deltas (same SEH-prolog quirk seen on other functions this batch); traced the real bytes instead and the fild reduces to plain (double)GetFileSize()*ratio. Not chased to byte-exact given size (2025 bytes, largest in batch).
int NetDaemon::send_files() {
    Filemap filemap;
    uint32_t send_mask = 0;
    int group_handle = 0;
    const int local_index = NetDaemonNet.pid_2_idx(
        NetDaemonNet.field_760_);

    NetDaemonNet.channels_[1].trailing_words_[9] = 0;
    netdaemon_set_file_index_records(netdaemon_allocate_file_index_records());

    char executable_path[0x104];
    GetModuleFileNameA(0, executable_path, sizeof executable_path);
    char *last_separator = strrchr(executable_path, '\\');
    if (last_separator != 0) last_separator[1] = '\0';

    if (primary_state_.leading_words_[0] != 0 && NetDaemonNet.player_count_ > 0) {
        for (int slot = 1; slot <= NetDaemonNet.player_count_; ++slot) {
            if (local_index != slot) send_mask |= 1U << (slot & 31);
            NetDaemonNet.process_slots_[slot].status_bytes_[0] =
                static_cast<uint8_t>(local_index == slot ? 0 : 2);
        }
    }

    if (primary_state_.leading_words_[0] == 0) {
        while (NetDaemonFileTransferMode != 1) {
            unsigned long sender, channel;
            void *packet = NetDaemonNet.Net::get(&sender, &channel);
            if (packet != 0)
                process_message(static_cast<char *>(packet), channel,
                                static_cast<int>(sender));
        }

        NetDaemonFileControlPacket announce;
        announce.opcode = 0xF0F;
        announce.source_player = local_index;
        NetDaemonNet.Net::send(&announce, 0x20,
            static_cast<unsigned long>(field_764_), 1);

        SaveNameBuffer[0] = '\0';
        strcat(SaveNameBuffer, executable_path);
        strcat(SaveNameBuffer, NetDaemonFileTransferPath);
        filefind_set_alternative(SaveNameBuffer);
    } else {
        char path_copy[0x104];
        path_copy[0] = '\0';
        strcat(path_copy, SaveNameBuffer);
        char *relative_path = path_copy;
        if (path_copy[1] == ':') {
            relative_path = path_copy + strlen(executable_path);
        }

        NetDaemonFileTransferPath[0] = '\0';
        strcat(NetDaemonFileTransferPath, relative_path);
        NetDaemonFilePathPacket path_message;
        path_message.opcode = 0xF05;
        path_message.path[0] = '\0';
        strcat(path_message.path, NetDaemonFileTransferPath);
        send_message(reinterpret_cast<char *>(&path_message), 0x11C, 0);
        NetDaemonFileTransferAckMask = 0;
        if (send_mask != 0) {
            do {
                unsigned long sender, channel;
                void *packet = NetDaemonNet.Net::get(&sender, &channel);
                if (packet != 0)
                    process_message(static_cast<char *>(packet), channel,
                                    static_cast<int>(sender));
            } while (NetDaemonFileTransferAckMask != send_mask);
        }
    }

    char previous_directory[0x104];
    GetCurrentDirectoryA(sizeof previous_directory, previous_directory);
    const BOOL changed_directory = SetCurrentDirectoryA(SaveNameBuffer);
    if (changed_directory == 0 && primary_state_.leading_words_[0] == 0) {
        if (_mkdir(SaveNameBuffer) == 0) SetCurrentDirectoryA(SaveNameBuffer);
    }

    int file_count = 0;
    if (primary_state_.leading_words_[0] == 0) {
        for (;;) {
            if (NetDaemonFileTransferMode == 3 ||
                channels_[1].trailing_words_[9] != 0) goto cleanup;
            if ((GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0) break;
            do_draw();
            unsigned long sender, channel;
            void *packet = NetDaemonNet.Net::get(&sender, &channel);
            if (packet != 0)
                process_message(static_cast<char *>(packet), channel,
                                static_cast<int>(sender));
        }
        channels_[1].trailing_words_[9] = 1;
        pop_close();
    } else {
        WIN32_FIND_DATAA found_file;
        HANDLE find_handle = FindFirstFileA("*.*", &found_file);
        if (find_handle != 0) {
            NetDaemonFileIndexRecord *records = netdaemon_file_index_records();
            do {
                if ((found_file.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
                    NetDaemonFileIndexRecord &record = records[file_count];
                    record.opcode = 0xF06;
                    record.source_player = local_index;
                    record.ready_flags = 0;
                    record.chunk_count = (found_file.nFileSizeLow + 0x800) >> 11;
                    record.file_index = static_cast<uint8_t>(file_count);
                    record.file_size = found_file.nFileSizeLow;
                    record.checksum = checksum_file(found_file.cFileName);
                    record.filename[0] = '\0';
                    strcat(record.filename, found_file.cFileName);
                    send_message(reinterpret_cast<char *>(&record), 0x11C, 0);
                    ++file_count;
                }
            } while (FindNextFileA(find_handle, &found_file) != 0);
        }

        NetDaemonFileControlPacket list_done;
        list_done.opcode = 0xF07;
        list_done.state = 2;
        send_message(reinterpret_cast<char *>(&list_done), 0x20, 0);
        FindClose(find_handle);

        for (;;) {
            do_draw();
            unsigned long sender, channel;
            void *packet = NetDaemonNet.Net::get(&sender, &channel);
            if (packet != 0)
                process_message(static_cast<char *>(packet), channel,
                                static_cast<int>(sender));
            if (NetDaemonNet.player_count_ < 1) break;
            bool all_present = true;
            for (int slot = 1; slot <= NetDaemonNet.player_count_; ++slot) {
                if (NetDaemonNet.process_slots_[slot].status_bytes_[0] == 0) {
                    all_present = false;
                    break;
                }
            }
            if (all_present) break;
        }

        NetDaemonFileIndexRecord *records = netdaemon_file_index_records();
        int ready_count = 0;
        for (int i = 0; i < file_count; ++i)
            if (records[i].ready_flags != 0) ++ready_count;

        NetDaemonFileChunkPacket *send_buffer = ::new (mem_get(0x820))
            NetDaemonFileChunkPacket;
        send_buffer->opcode = 0xF0B;
        for (int file_index = 0; file_index < file_count; ++file_index) {
            if ((GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0) {
                channels_[1].trailing_words_[9] = 1;
                goto cleanup;
            }
            NetDaemonFileIndexRecord &record = records[file_index];
            if (record.ready_flags == 0) continue;

            parse_says(0, record.filename, -1, -1);
            --ready_count;
            parse_num(0, ready_count);
            PopupModalState = 6;
            pop_close();
            x_pop_caption_flags(const_cast<char *>("SCRIPT"), "MPSENDFILE",
                                0x214200, 0);
            PopupModalState = 0;
            send_buffer->file_index = file_index;

            if (group_handle != 0) NetDaemonNet.delete_group(group_handle);
            group_handle = NetDaemonNet.create_group(const_cast<char *>("temp"));
            for (int slot = 1; slot < 8; ++slot) {
                if ((record.ready_flags & (1U << (slot & 31))) != 0)
                    NetDaemonNet.add_to_group(group_handle,
                        NetDaemonNet.process_slots_[slot].pid_);
            }

            if (filemap.open_read(record.filename, TRUE) != 0) {
                const DWORD original_size = GetFileSize(filemap.get_handle(), 0);
                const float compressed_capacity_ratio = 1.01f;
                unsigned long compressed_capacity = static_cast<unsigned long>(
                    static_cast<double>(original_size) *
                    static_cast<double>(compressed_capacity_ratio));
                void *compressed = mem_get(static_cast<int>(compressed_capacity));
                const DWORD input_size = GetFileSize(filemap.get_handle(), 0);
                netdaemon_deflate_buffer(static_cast<unsigned char *>(compressed),
                    reinterpret_cast<unsigned long *>(&compressed_capacity),
                    static_cast<unsigned char *>(filemap.get_view()), input_size);
                send_buffer->compressed_size = compressed_capacity;

                const unsigned char *source =
                    static_cast<const unsigned char *>(compressed);
                int remaining = static_cast<int>(compressed_capacity);
                int chunk_index = 0;
                while (remaining > 0x7FF) {
                    do_draw();
                    memcpy(send_buffer->payload, source, 0x800);
                    remaining -= 0x800;
                    source += 0x800;
                    send_buffer->chunk_index = chunk_index++;
                    NetDaemonNet.send_group(send_buffer, 0x820, group_handle, 1);
                }
                send_buffer->chunk_index = chunk_index;
                if (remaining > 0) {
                    memcpy(send_buffer->payload, source,
                           static_cast<size_t>(remaining));
                    NetDaemonNet.send_group(send_buffer, remaining + 0x20,
                                            group_handle, 1);
                }
                filemap.close();
                if (compressed != 0) free(compressed);
            }
        }
        free(send_buffer);

        for (;;) {
            if ((GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0) break;
            unsigned long sender, channel;
            void *packet = NetDaemonNet.Net::get(&sender, &channel);
            if (packet != 0)
                process_message(static_cast<char *>(packet), channel,
                                static_cast<int>(sender));
            do_draw();
            if (NetDaemonNet.player_count_ < 1) break;
            int slot = 1;
            while (slot <= NetDaemonNet.player_count_ &&
                   NetDaemonNet.process_slots_[slot].status_bytes_[0] > 1)
                ++slot;
            if (slot > NetDaemonNet.player_count_) break;
        }
        NetDaemonFileControlPacket transfer_done;
        transfer_done.opcode = 0xF07;
        transfer_done.state = 3;
        send_message(reinterpret_cast<char *>(&transfer_done), 0x20, 0);
    }

cleanup:
    pop_close();
    do_all_draws();
    SetCurrentDirectoryA(previous_directory);
    void *list_storage = netdaemon_file_index_records();
    if (list_storage != 0) free(list_storage);
    const int result = static_cast<int>(NetDaemonNet.channels_[1].trailing_words_[9]);
    netdaemon_set_file_index_records(0);
    NetDaemonFileTransferMode = 0;
    return result;
}

// Source-projected candidates for selected original
// NetDaemon::process_message arms. These helpers are not separately mapped
// original identities; they keep recovered arm operations inside the real
// NetDaemon owner.
// 0x538266 is shared by flagged routed messages. C++ scope cleanup supplies
// Filemap/Popup lifetimes; retain its separate conditional marker clear before
// handlers return from the real process_message scope.
static void netdaemon_process_message_marker_tail(NetDaemon &daemon,
                                                   unsigned char *message) {
    const uint32_t pending = daemon.channels_[1].leading_words_[1];
    if (pending != 0) {
        const unsigned short type =
            *reinterpret_cast<unsigned short *>(message);
        if ((message[1] & 0x40u) != 0 && (type & 0x4000u) != 0 &&
            static_cast<uint32_t>(type | 0x4000u) == pending &&
            *reinterpret_cast<uint32_t *>(message + 4) ==
                static_cast<uint32_t>(ConsoleGlobal.faction_id_)) {
            daemon.channels_[1].leading_words_[1] = 0;
        }
    }
}


static void netdaemon_process_sync_request(NetDaemon *daemon,
                                            unsigned char *message) {
    const uint32_t sender = *reinterpret_cast<uint32_t *>(message + 4);
    uint32_t &pending = daemon->channels_[1].trailing_words_[1];
    const uint32_t old_pending = pending;
    pending |= static_cast<uint32_t>(1) << (sender & 0x1F);

    daemon->synch(
        *reinterpret_cast<int16_t *>(message + 0x10),
        *reinterpret_cast<int *>(message + 0x14),
        *reinterpret_cast<int *>(message + 0x18),
        *reinterpret_cast<int *>(message + 0x1C),
        reinterpret_cast<char *>(message), 0, 0x2101);

    if (old_pending != 0) return;

    while (pending != 0) {
        unsigned long packet_sender;
        unsigned long packet_size;
        void *packet = NetDaemonNet.Net::get(&packet_sender, &packet_size);
        if (packet != 0) {
            daemon->process_message(static_cast<char *>(packet),
                                    packet_size,
                                    static_cast<int>(packet_sender));
        }
    }

    if (sender == static_cast<uint32_t>(ConsoleGlobal.faction_id_))
        daemon->channels_[1].trailing_words_[0] = 0;
}

// 0x2301/0x8301 share the native DONE TURN body at 0x537FD0. Keep the two
// daemon masks, peer notification order, and the separate assigned-turn and
// local slowpoke/UI paths source-owned in the existing receiver.
static void netdaemon_handle_client_done_turn(NetDaemon *daemon,
                                              int resolved_sender) {
    const int faction = resolved_sender;
    const uint32_t faction_bit = static_cast<uint32_t>(1) << (faction & 0x1F);
    uint32_t &turn_waiters = daemon->primary_state_.trailing_words_[7];
    uint32_t &turn_info = daemon->channels_[0].leading_words_[0];
    turn_waiters |= faction_bit;
    turn_info |= faction_bit;

    if ((GameDataStateStorage.fields_.net_turn_flags_ & 0x10u) == 0) {
        for (int notify_slot = 1; notify_slot <= daemon->player_count_; ++notify_slot) {
            const int target = daemon->process_slots_[notify_slot].identity_;
            if (target != ConsoleGlobal.faction_id_) {
                message_data(0x1301, target, faction, 1,
                             GameDataStateStorage.fields_.turn_current_num_, 0);
            }
        }
    }

    MultiWindow.draw(0);
    log_say("From Client: DONE TURN", faction,
            static_cast<int>(turn_waiters), 0);

    if ((GameDataStateStorage.fields_.net_turn_flags_ & 0x10u) != 0) {
        if (daemon->primary_state_.leading_words_[0] == 0) return;

        int assigned_next_faction =
            GameDataStateStorage.fields_.net_turn_faction_ + 1;
        const uint32_t alive =
            GameDataStateStorage.fields_.lock_masks_.enable_mask_ >> 8;
        const uint32_t humans =
            GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 0xFFu;
        while (assigned_next_faction < 8) {
            const uint32_t assigned_bit =
                static_cast<uint32_t>(1) << (assigned_next_faction & 0x1F);
            if ((alive & assigned_bit) != 0 && (humans & assigned_bit) != 0) break;
            ++assigned_next_faction;
        }
        if (assigned_next_faction >= 8) {
            NextTurnReady = 1;
        } else {
            message_data(0x4309, 0, assigned_next_faction, 0, 0, 0);
        }
        return;
    }

    const uint32_t enabled =
        GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 0xFFu;
    const uint32_t outstanding = enabled & ~turn_info;
    if (bit_count(static_cast<int>(outstanding)) != 1) return;
    if (bit_count(static_cast<int>(enabled)) <= 2) return;

    const uint32_t alive =
        GameDataStateStorage.fields_.lock_masks_.enable_mask_ >> 8;
    int slowpoke_next_faction = 1;
    for (; slowpoke_next_faction < 8; ++slowpoke_next_faction) {
        const uint32_t slowpoke_bit =
            static_cast<uint32_t>(1) << (slowpoke_next_faction & 0x1F);
        if ((alive & slowpoke_bit) != 0 && (enabled & slowpoke_bit) != 0 &&
            (turn_info & slowpoke_bit) == 0) break;
    }
    if (slowpoke_next_faction >= 8) return;

    Player &faction_data = Players[slowpoke_next_faction];
    GenderDefault = faction_data.is_leader_female;
    PluralityDefault = 0;
    parse_says(0, faction_data.title_leader, -1, -1);
    parse_says(1, faction_data.name_leader, -1, -1);
    GenderDefault = faction_data.noun_gender;
    PluralityDefault = faction_data.is_noun_plural;
    parse_says(2, faction_data.noun_faction, -1, -1);

    for (int name_slot = 1; name_slot <= daemon->player_count_; ++name_slot) {
        if (daemon->process_slots_[name_slot].identity_ == slowpoke_next_faction) {
            parse_says(3, daemon->process_slots_[name_slot].name_, -1, -1);
        }
    }
    NetMsgPrimary.pop("SLOWPOKE", 5000, 0, 0);
}

// Pinned direct target 0x53511E handles Server TURN INFO. It first records
// the log, then honors simultaneous/assigned-turn gates and the current-turn
// match before mutating the faction wait mask. Player and process-slot scans
// intentionally retain the original unchecked faction and repeated-name
// behavior; return through process_message so its real local cleanup runs.
static void netdaemon_handle_server_turn_info(NetDaemon *daemon,
                                               unsigned char *message) {
    const int faction = *reinterpret_cast<int *>(message + 0x10);
    const int acknowledged = *reinterpret_cast<int *>(message + 0x14);
    const int turn = *reinterpret_cast<int *>(message + 0x18);
    log_say("From Server: TURN INFO", faction, acknowledged, turn);

    if ((GameDataStateStorage.fields_.net_turn_flags_ & 0x10u) != 0) return;
    if (daemon->primary_state_.leading_words_[0] != 0) return;
    if (turn != GameDataStateStorage.fields_.turn_current_num_) {
        MultiWindow.draw(0);
        return;
    }

    const uint32_t faction_bit =
        static_cast<uint32_t>(1) << (static_cast<uint32_t>(faction) & 31u);
    uint32_t &turn_info = daemon->channels_[0].leading_words_[0];
    if (acknowledged == 0) {
        turn_info &= ~faction_bit;
        MultiWindow.draw(0);
        return;
    }

    turn_info |= faction_bit;
    const uint32_t enabled =
        GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 0xFFu;
    if (bit_count(static_cast<int>(enabled & ~turn_info)) != 1) {
        MultiWindow.draw(0);
        return;
    }
    if (bit_count(static_cast<int>(enabled)) <= 2) {
        MultiWindow.draw(0);
        return;
    }

    const uint32_t alive =
        (GameDataStateStorage.fields_.lock_masks_.enable_mask_ >> 8) & 0xFFu;
    int next_faction = 1;
    for (; next_faction < 8; ++next_faction) {
        const uint32_t bit = static_cast<uint32_t>(1) << next_faction;
        if ((alive & bit) != 0 && (enabled & bit) != 0 &&
            (turn_info & bit) == 0) break;
    }
    if (next_faction >= 8) {
        MultiWindow.draw(0);
        return;
    }

    Player &player = Players[next_faction];
    GenderDefault = player.is_leader_female;
    PluralityDefault = 0;
    parse_says(0, player.title_leader, -1, -1);
    parse_says(1, player.name_leader, -1, -1);
    GenderDefault = player.noun_gender;
    PluralityDefault = player.is_noun_plural;
    parse_says(2, player.noun_faction, -1, -1);

    for (int slot = 1; slot <= daemon->player_count_; ++slot) {
        if (daemon->process_slots_[slot].identity_ == next_faction)
            parse_says(3, daemon->process_slots_[slot].name_, -1, -1);
    }
    MultiWindow.draw(0);
    NetMsgPrimary.pop("SLOWPOKE", 5000, 0, 0);
}

// Original 0x534E23..0x534FD9.  The two timing words are separate global
// dwords: 0x93F66C is the first timeGetTime sample, and 0x93F668 is the
// selected 1000/5000 ms interval.  Do not fold them into NetBusyCount's owner.
static void netdaemon_handle_server_lock_status(NetDaemon *daemon,
                                                 unsigned short type,
                                                 unsigned char *message,
                                                 int sender_identity) {
    switch (type) {
    case 0x4301: {
        my_srand(*reinterpret_cast<int *>(message + 8));
        daemon->channels_[0].leading_words_[0] = 0xFF;
        daemon->primary_state_.trailing_words_[7] = 0xFF;
        MultiWindow.draw(0);
        log_say("From Server: NEXT TURN",
                GameDataStateStorage.fields_.turn_current_num_,
                static_cast<int>(daemon->primary_state_.trailing_words_[7]), 0);
        NetDaemonProgressInterval = 0;
        NetDaemonProgressLastTick = 0;
        if ((GameDataStateStorage.fields_.net_turn_flags_ & 0x10u) != 0) {
            GameDataStateStorage.fields_.game_state_ |= 0x800u;
            NextTurnReady = 1;
        }
        return;
    }
    case 0x4303: {
        const int phase = *reinterpret_cast<int *>(message + 0x10);
        const int turn = *reinterpret_cast<int *>(message + 0x14);
        log_say("From Server: NEXT UPKEEP", phase, turn, 0);
        if (phase != NetUpkeepPhase) {
            log_say("(Upkeep instruction for wrong phase!)",
                    phase, NetUpkeepPhase, 0);
        } else if (turn != GameDataStateStorage.fields_.turn_current_num_) {
            log_say("(Upkeep instruction for wrong turn!)",
                    turn, GameDataStateStorage.fields_.turn_current_num_, 0);
        }
        my_srand(*reinterpret_cast<int *>(message + 8));
        daemon->channels_[0].leading_words_[1] = 0xFF;
        daemon->primary_state_.trailing_words_[9] = 0xFF;
        MultiWindow.draw(0);
        NetDaemonProgressInterval = 0;
        NetDaemonProgressLastTick = 0;
        return;
    }
    case 0x1201:
        log_say("From Server: LOCK OK", 0, 0, 0);
        daemon->channels_[0].leading_words_[2] = 1;
        NetDaemonProgressInterval = 0;
        NetDaemonProgressLastTick = 0;
        return;
    case 0x1202:
        log_say("From Server: LOCK FAILED", 0, 0, 0);
        daemon->channels_[0].leading_words_[2] = 2;
        break;
    case 0x1203:
        log_say("From Server: LOCK FAILED GLOBAL", 0, 0, 0);
        daemon->channels_[0].leading_words_[2] = 3;
        break;
    case 0x1204:
        log_say("From Server: LOCK FAILED UPKEEP", 0, 0, 0);
        daemon->channels_[0].leading_words_[2] = 4;
        break;
    case 0x1205:
        log_say("From Server: GLOBAL LOCK READY", 0, 0, 0);
        daemon->locks_.field_E8_ = 1;
        return;
    case 0x1206: {
        const uint32_t remaining = daemon->channels_[1].trailing_words_[2];
        log_say("From Server: EXEC DONE", 0, 0,
                static_cast<int>(remaining));
        if (remaining != 0) {
            log_say("Remaining locked", 0, 0, 0);
            daemon->channels_[1].trailing_words_[2] = 0;
            message_data(0x2214, 0, 0, 0, 0, 0);
        } else {
            daemon->channels_[1].leading_words_[3] = 0;
        }
        return;
    }
    case 0x1207:
        log_say("From Server: ADD LOCK FAIL", 0, 0, 0);
        daemon->channels_[0].leading_words_[2] = 2;
        return;
    case 0x1208:
        log_say("From Server: ADD LOCK OK", 0, 0, 0);
        daemon->channels_[0].leading_words_[2] = 1;
        return;
    case 0x1240:
        if (*reinterpret_cast<uint32_t *>(message + 0x10)) {
            daemon->channels_[1].leading_words_[0] = 1;
            log_say("Received OPEN CHANNEL SUCCESS", 0, 0, 0);
        } else {
            daemon->channels_[1].leading_words_[0] = 2;
            log_say("Received OPEN CHANNEL FAILED", 0, 0, 0);
        }
        return;
    case 0x2308:
        log_say("From Client: HELP ME SPOCK",
                sender_identity,
                0, 0);
        SpockNotifyPending = 1;
        return;
    default:
        return;
    }

    if (NetDaemonProgressLastTick == 0)
        NetDaemonProgressLastTick = timeGetTime();
    const unsigned int now = timeGetTime();
    const unsigned int elapsed = now - NetDaemonProgressLastTick;
    NetDaemonProgressInterval = elapsed > 5000 ? 5000 : 1000;
}

// 0x536037 is opcode 0x2303, "From Client: DONE UPKEEP". The wait words are
// existing NetDaemon lock-state members and the player identities are the
// actual AlphaNet process slots. The sender argument is a process ID; the
// dispatcher supplies its entry-captured identity, retained in native EDI.
static void netdaemon_handle_client_done_upkeep(NetDaemon *daemon,
                                                 unsigned char *message,
                                                 int sender_identity) {
    const int sender = sender_identity;
    const uint32_t sender_bit = static_cast<uint32_t>(1) << (sender & 0x1F);
    const int phase = *reinterpret_cast<int *>(message + 0x10);
    const int turn = *reinterpret_cast<int *>(message + 0x14);

    if (phase != NetUpkeepPhase) {
        log_say("(Upkeep acknowledgement for wrong phase!)",
                phase, NetUpkeepPhase, 0);
    } else if (turn != GameDataStateStorage.fields_.turn_current_num_) {
        log_say("(Upkeep ack for wrong turn!)", turn,
                GameDataStateStorage.fields_.turn_current_num_, 0);
    }

    daemon->channels_[0].leading_words_[1] |= sender_bit;
    uint32_t &already_notified = daemon->primary_state_.trailing_words_[8];
    if ((already_notified & sender_bit) == 0) {
        already_notified |= sender_bit;
        for (int slot = 1; slot <= daemon->player_count_; ++slot) {
            const int target = daemon->process_slots_[slot].identity_;
            if (target != ConsoleGlobal.faction_id_) {
                message_data(0x1303, target, sender, 1,
                             GameDataStateStorage.fields_.turn_current_num_, 0);
            }
        }
        MultiWindow.draw(0);
    }

    log_say("From Client: DONE UPKEEP", sender, phase,
            static_cast<int>(already_notified));
}

// 0x53611D is opcode 0x2302, "From Client: WITHDRAW END-OF-TURN". It
// acknowledges the matching waiting faction, then sends TURN INFO to the
// other active process identities. Every non-early native path reaches draw.
static void netdaemon_handle_withdraw_end_turn(NetDaemon *daemon,
                                                unsigned char *message,
                                                int sender_identity) {
    const int sender = sender_identity;
    const uint32_t sender_bit = static_cast<uint32_t>(1) << (sender & 0x1F);
    log_say("From Client: WITHDRAW END-OF-TURN", sender, 0, 0);

    // The native handler reloads the shared mask after log_say; the callback
    // may change it, so do not carry a pre-callback snapshot into the test.
    const uint32_t waiting = NetDaemonNet.factions_waiting_mask();

    if (waiting != 0xFF && (waiting & sender_bit) != 0 &&
        *reinterpret_cast<int *>(message + 0x10) ==
            GameDataStateStorage.fields_.turn_current_num_) {
        NetDaemonNet.factions_waiting_mask() = waiting & ~sender_bit;
        message_data(0x1302, sender,
                     GameDataStateStorage.fields_.turn_current_num_, 0, 0, 0);
        for (int slot = 1; slot <= daemon->player_count_; ++slot) {
            const int target = daemon->process_slots_[slot].identity_;
            if (target != sender && target != ConsoleGlobal.faction_id_) {
                message_data(0x1301, target, sender, 0,
                             GameDataStateStorage.fields_.turn_current_num_, 0);
            }
        }
    }
    MultiWindow.draw(0);
}

// 0x1302 WITHDRAW APPROVED has the local matching-turn wait-clear behavior.
// 0x1303 UPKEEP INFO is a separate packet-selected wait-mask update below.
static void netdaemon_complete_local_turn_wait(NetDaemon *daemon,
                                                unsigned char *message) {
    const int faction = ConsoleGlobal.faction_id_;
    const uint32_t faction_bit =
        static_cast<uint32_t>(1) << (faction & 0x1F);

    const uint32_t state = GameDataStateStorage.fields_.game_state_;
    const uint32_t waiting = NetDaemonNet.factions_waiting_mask();
    const uint32_t assigned =
        NetDaemonNet.primary_state_.leading_words_[0];
    if ((state & 2u) != 0 && waiting != 0xFF &&
        (((waiting & faction_bit) != 0) || assigned != 0) &&
        *reinterpret_cast<int *>(message + 0x10) ==
            GameDataStateStorage.fields_.turn_current_num_) {
        NetDaemonNet.factions_waiting_mask() = waiting & ~faction_bit;
        NetDaemonNet.channels_[0].leading_words_[0] &= ~faction_bit;
        GameDataStateStorage.fields_.game_state_ = state & ~2u;
    }
    MultiWindow.draw(0);
}

// 0x535048 is opcode 0x1303, "From Server: UPKEEP INFO".
static void netdaemon_handle_server_upkeep_info(NetDaemon *daemon,
                                                 unsigned char *message) {
    log_say("From Server: UPKEEP INFO",
            *reinterpret_cast<int *>(message + 0x10),
            *reinterpret_cast<int *>(message + 0x14),
            *reinterpret_cast<int *>(message + 0x18));
    if (daemon->primary_state_.leading_words_[0] != 0) return;
    const unsigned int faction =
        *reinterpret_cast<uint32_t *>(message + 0x10);
    daemon->channels_[0].leading_words_[1] |=
        static_cast<uint32_t>(1) << (faction & 0x1Fu);
    MultiWindow.draw(0);
}

// 0x535097 is opcode 0x1302, "From Server: WITHDRAW APPROVED".
static void netdaemon_handle_withdraw_approved(NetDaemon *daemon,
                                               unsigned char *message) {
    log_say("From Server: WITHDRAW APPROVED", 0, 0, 0);
    netdaemon_complete_local_turn_wait(daemon, message);
}

// Diplomacy-channel messages run after the dispatcher's 0x1500 modal
// prelude, before its shared marker/Filemap/Popup tail.
static bool netdaemon_handle_diplo_cd_message(unsigned short type,
                                               unsigned char *message) {
    switch (type) {
    case 0x1505: {
        log_say("Received CLOSE CHAN",
                *reinterpret_cast<int *>(message + 4), 0, 0);
        if (CommlinkPending == 0) return true;

        DiploWindow.release_modal();
        const uint32_t language = Language;
        CommlinkPending = 0;
        if (language != 0) return true;

        const int faction = *reinterpret_cast<int *>(message + 4);
        PluralityDefault = 0;
        GenderDefault = Players[faction].is_leader_female;
        parse_says(0, Players[faction].title_leader, -1, -1);
        PluralityDefault = 0;
        GenderDefault = Players[faction].is_leader_female;
        parse_says(1, Players[faction].name_leader, -1, -1);
        NetMsgPrimary.pop("DIPLOCLOSE", 5000, 0, 0);
        return true;
    }
    case 0x1506:
        log_say("Received OPEN OFFER",
                *reinterpret_cast<int *>(message + 4),
                *reinterpret_cast<int *>(message + 0x10),
                *reinterpret_cast<int *>(message + 0x14));
        DiploWindow.add_offer(1,
                              *reinterpret_cast<int *>(message + 0x10),
                              *reinterpret_cast<int *>(message + 0x14));
        DiploWindow.redraw();
        return true;
    case 0x1507:
        log_say("Received KILL OFFER",
                *reinterpret_cast<int *>(message + 4),
                *reinterpret_cast<int *>(message + 0x10), 0);
        DiploWindow.kill_offer(*reinterpret_cast<int *>(message + 0x10), 1);
        DiploWindow.redraw();
        return true;
    case 0x1508:
        log_say("Received CLEAR OFFER",
                *reinterpret_cast<int *>(message + 4), 0, 0);
        DiploWindow.receive_clear_offer();
        return true;
    case 0x1509:
        log_say("Received CHANNEL STATUS",
                *reinterpret_cast<int *>(message + 4),
                *reinterpret_cast<int *>(message + 0x10), 0);
        DiploWindow.receive_channel_status(
            *reinterpret_cast<int *>(message + 0x10));
        return true;
    case 0x150B:
        log_say("Received YES DEAL",
                *reinterpret_cast<int *>(message + 4), 0, 0);
        DiploWindow.do_it();
        return true;
    case 0x150C:
        log_say("Received NO DEAL",
                *reinterpret_cast<int *>(message + 4), 0, 0);
        DiploWindow.receive_no_deal(
            *reinterpret_cast<int *>(message + 0x14),
            *reinterpret_cast<int *>(message + 0x10));
        return true;
    case 0x150D:
        log_say("Received DONE DEAL",
                *reinterpret_cast<int *>(message + 4), 0, 0);
        DiploWindow.done_deal();
        return true;
    case 0x150E:
        log_say("Received DEMAND WITHDRAWAL",
                *reinterpret_cast<int *>(message + 4), 0, 0);
        return true;
    case 0x150F: {
        const int faction = *reinterpret_cast<int *>(message + 4);
        log_say("Received VENDETTA WARNING", faction, 0, 0);
        PluralityDefault = 0;
        GenderDefault = Players[faction].is_leader_female;
        parse_says(0, Players[faction].title_leader, -1, -1);
        PluralityDefault = 0;
        GenderDefault = Players[faction].is_leader_female;
        parse_says(1, Players[faction].name_leader, -1, -1);
        GenderDefault = Players[faction].noun_gender;
        PluralityDefault = Players[faction].is_noun_plural;
        parse_says(2, Players[faction].noun_faction, -1, -1);
        NetMsgPrimary.pop("VENDETTAWARNING", 5000, 0, 0);
        return true;
    }
    default:
        return false;
    }
}

// Source-only implementation candidate for the five concrete 0x2212..0x2216
// arms in NetDaemon::process_message. The caller retains the real dispatcher
// scope and invokes its shared 0x538266 marker tail after this returns true.
// Several packet indices are intentionally unchecked: the original indexes
// the eight PlayerLock records directly from message words/sender identity.
static bool netdaemon_process_table_e(NetDaemon &daemon,
                                      unsigned char *message,
                                      unsigned short opcode,
                                      int sender_identity) {
    switch (opcode) {
    case 0x2212:
        log_say("From Client: Ready to unlock", sender_identity, 0, 0);
        daemon.locks_.unlock(sender_identity);
        return true;

    case 0x2213: {
        const int player_index =
            *reinterpret_cast<int *>(message + 0x10);
        const int sequence = *reinterpret_cast<int *>(message + 0x0C);
        const int identity = *reinterpret_cast<int *>(message + 0x10);
        log_say("From Client: ACKNOWLEDGE", sender_identity, identity, 0);

        // The original performs this one-based lookup but discards EAX before
        // selecting the PlayerLock from the unmodified packet word at +0x10.
        (void)daemon.AlphaNet::who_2_idx(identity);

        for (int table_index = 0; table_index < 24; ++table_index) {
            DeletionRecord &entry = daemon.deletion_list_.records_[table_index];
            const uint8_t enabled = static_cast<uint8_t>(
                GameDataStateStorage.fields_.lock_masks_.enable_mask_);
            if ((enabled & entry.acknowledgements_) == enabled ||
                static_cast<uint32_t>(entry.owner_) != static_cast<uint32_t>(sequence)) {
                continue;
            }

            const uint8_t sender_bit = static_cast<uint8_t>(
                static_cast<uint32_t>(1u) <<
                (static_cast<uint32_t>(sender_identity) & 31u));
            entry.acknowledgements_ = static_cast<uint8_t>(entry.acknowledgements_ | sender_bit);
            if ((entry.acknowledgements_ & enabled) == enabled) {
                entry.acknowledgements_ = 0xFF;
            }
            break;
        }

        PlayerLock &record = daemon.locks_.records_[player_index];
        if (record.active() == 0) return true;
        if ((record.active_ & 1u) != 0) return true;

        const uint8_t sender_bit = static_cast<uint8_t>(
            1u << (static_cast<uint32_t>(sender_identity) & 31u));
        record.active_ = static_cast<uint8_t>(record.active_ | sender_bit);
        const uint8_t enabled = static_cast<uint8_t>(
            GameDataStateStorage.fields_.lock_masks_.enable_mask_);
        if ((record.active_ & enabled) != enabled) return true;

        log_say("Sending EXEC DONE to", player_index, 0, 0);
        message_data(0x1206, player_index, 0, 0, 0, 0);
        // The native store is after message_data, not before the callback.
        record.active_ = static_cast<uint8_t>(record.active_ | 1u);
        return true;
    }

    case 0x2214:
        log_say("From Client: RELOCK", sender_identity, 0, 0);
        daemon.locks_.records_[sender_identity].active_ = 0;
        return true;

    case 0x2215: {
        if (SpockHelpActive != 0 || TurnControlWord93A954 != 0) {
            log_say("(Sending: Lock failed -- REJECT)", 0, 0, 0);
            message_data(0x1208, sender_identity, 0, 0, 0, 0);
            return true;
        }

        const int first = *reinterpret_cast<int *>(message + 0x10);
        const int second = *reinterpret_cast<int *>(message + 0x14);
        const int third = *reinterpret_cast<int *>(message + 0x18);
        log_say("From Client: ADD LOCK", sender_identity, first, second);

        // Stack push order at 0x535F64 passes slot, +0x18, +0x10, +0x14
        // to Lock::add_lock; preserve that non-obvious wire/ABI permutation.
        if (daemon.locks_.add_lock(sender_identity, third, first, second) != 0) {
            log_say("(Sending: Add failed)", 0, 0, 0);
            message_data(0x1208, sender_identity, 0, 0, 0, 0);
        } else {
            log_say("(Sending: Add OK)", 0, 0, 0);
            message_data(0x1207, sender_identity, 0, 0, 0, 0);
        }
        return true;
    }

    case 0x2216:
        log_say("From Server: LOCK OK GLOBAL", 0, 0, 0);
        daemon.channels_[0].leading_words_[2] = 1;
        NetDaemonProgressInterval = 0;
        NetDaemonProgressLastTick = 0;
        daemon.locks_.field_E8_ = 1;
        return true;

    default:
        return false;
    }
}

// Pinned process_message table F (0x4304..0x4309). This helper is source
// material for the real process_message owner; it is not an original identity.
// The caller retains the common marker-tail and normal C++ scope cleanup.

static bool netdaemon_handle_table_f(NetDaemon *daemon, unsigned short type,
                                     unsigned char *message, int sender_id) {
    switch (type) {
    case 0x4304:
        log_say("From Server: UPKEEP_BASE", 0, 0, 0);
        daemon->primary_state_.trailing_words_[9] = 1;
        return true;

    case 0x4305: {
        const int incoming = *reinterpret_cast<int *>(message + 0x10);
        log_say("From Server: MSG_EVENTS", 0, incoming, 0);
        if (static_cast<int32_t>(NetDaemonNet.network_message_count()) <= incoming)
            NetDaemonNet.network_message_count() = static_cast<uint32_t>(incoming);
        return true;
    }

    case 0x4306: {
        const int faction_id = *reinterpret_cast<int *>(message + 4);
        const int event_value = *reinterpret_cast<int *>(message + 0x10);
        log_say("From Server: MSG_TIME_BONUS", faction_id, event_value, 0);
        if (ConsoleControlTurnActive != 0) return true;

        if (PlayersData[faction_id].time_bonus_count > 0)
            --PlayersData[faction_id].time_bonus_count;

        const int selection =
            GameDataStateStorage.fields_.time_controls_selection_;
        const int duration = TimeControl[selection].event * 1000;
        NetDaemonWaitTicks += duration;
        GameDataStateStorage.fields_.turn_control_duration_milliseconds_ +=
            duration;
        MultiWindow.draw(0);

        parse_says(0, daemon->get_player_name(
                          static_cast<unsigned long>(sender_id)), -1, -1);
        parse_num(0, TimeControl[selection].event);
        NetMsgPrimary.pop(
            faction_id == ConsoleGlobal.faction_id_ ? "WEBONUSED" : "HEBONUSED",
            5000, 0, nullptr);
        return true;
    }

    case 0x4307: {
        // The pinned loop indexes by the packet faction without a range check.
        int *destination = NetPeerTurnChecksums[
            *reinterpret_cast<int *>(message + 4)];
        const int *source = reinterpret_cast<int *>(message + 0x10);
        for (int index = 0; index < NetTurnChecksumWords; ++index)
            destination[index] = source[index];
        return true;
    }

    case 0x4309: {
        const int faction_id = *reinterpret_cast<int *>(message + 0x10);
        log_say("From Server: NEXT PLAYER TURN", faction_id, 0, 0);
        GameDataStateStorage.fields_.net_turn_faction_ = faction_id;

        Player &player = Players[faction_id];
        PluralityDefault = 0;
        GenderDefault = player.is_leader_female;
        parse_says(0, player.title_leader, -1, -1);
        PluralityDefault = 0;
        GenderDefault = player.is_leader_female;
        parse_says(1, player.name_leader, -1, -1);
        GenderDefault = player.noun_gender;
        PluralityDefault = player.is_noun_plural;
        parse_says(2, player.noun_faction, -1, -1);
        const int player_pid = daemon->who_2_pid(faction_id);
        parse_says(3, daemon->get_player_name(
                          static_cast<unsigned long>(player_pid)), -1, -1);

        if (faction_id == ConsoleGlobal.faction_id_) {
            char year_text[0x194];
            year_text[0] = 0;
            say_year(year_text);
            parse_says(4, year_text, -1, -1);
            X_pop("SIMULYOU", pop_wait);
        } else {
            NetMsgPrimary.pop("SIMULWHOSE", 5000, 0, nullptr);
        }
        return true;
    }

    // Table F's fifth slot (0x4308) points to the dispatcher default at
    // 0x537F7A; it has no specialized handler here.
    default:
        return false;
    }
}

// Proposal-only helper for the 0x4405..0x4418 dispatch table.
// Include in netdaemon.cpp after its existing game-owner headers.
// Returns true only for a concrete Table G arm; the six default slots remain
// with process_message's existing fixup/rebroadcast path.
static int netdaemon_table_g_word(unsigned char *msg, int offset) {
    return *reinterpret_cast<int *>(msg + offset);
}

static bool netdaemon_process_table_g(NetDaemon &daemon, unsigned char *msg,
                                      unsigned short type) {
    // These three native tails are intentionally distinct: the original
    // dispatch arms enter them at different addresses and 0x4411 bypasses
    // sequence/local-owner processing altogether.
    const int TailDispatch = 0;
    const int TailSequence = 1;
    const int TailUpgrade = 2;
    int tail;

    switch (type) {
    case 0x4405:
        log_say("From Server: SPECIAL TERRAFORM", netdaemon_table_g_word(msg, 4),
                netdaemon_table_g_word(msg, 0x10), netdaemon_table_g_word(msg, 0x14));
        my_srand(netdaemon_table_g_word(msg, 8));
        action_terraform(netdaemon_table_g_word(msg, 0x10),
                         netdaemon_table_g_word(msg, 0x14) - 4, 1);
        Vehs[netdaemon_table_g_word(msg, 0x10)].order = *reinterpret_cast<int8_t *>(msg + 0x18);
        tail = TailDispatch;
        break;
    case 0x4406:
        log_say("From Server: DESTROY TERRAIN", netdaemon_table_g_word(msg, 4),
                netdaemon_table_g_word(msg, 0x10), netdaemon_table_g_word(msg, 0x14));
        my_srand(netdaemon_table_g_word(msg, 8));
        action_destroy(netdaemon_table_g_word(msg, 0x10),
                       netdaemon_table_g_word(msg, 0x14), -1, -1);
        tail = TailSequence;
        break;
    case 0x4407:
        log_say("From Server: KILL VEH", netdaemon_table_g_word(msg, 4),
                netdaemon_table_g_word(msg, 0x10), 0);
        my_srand(netdaemon_table_g_word(msg, 8));
        if (netdaemon_table_g_word(msg, 0x14) != 0) {
            action_destruct(netdaemon_table_g_word(msg, 0x10));
        } else {
            kill(netdaemon_table_g_word(msg, 0x10));
        }
        Vehs[netdaemon_table_g_word(msg, 0x10)].order = *reinterpret_cast<int8_t *>(msg + 0x18);
        tail = TailDispatch;
        break;
    case 0x4408:
    case 0x4416:
        log_say("From Server: GIVE VEH", netdaemon_table_g_word(msg, 4),
                netdaemon_table_g_word(msg, 0x10), netdaemon_table_g_word(msg, 0x14));
        my_srand(netdaemon_table_g_word(msg, 8));
        action_give(netdaemon_table_g_word(msg, 0x10), netdaemon_table_g_word(msg, 0x14));
        tail = TailSequence;
        break;
    case 0x4409:
        log_say("From Server: BUILD BASE",
                reinterpret_cast<char *>(msg + 0x14), netdaemon_table_g_word(msg, 4),
                netdaemon_table_g_word(msg, 0x10), 0);
        my_srand(netdaemon_table_g_word(msg, 8));
        action_build(netdaemon_table_g_word(msg, 0x10), reinterpret_cast<char *>(msg + 0x14));
        tail = TailSequence;
        break;
    case 0x4410:
        log_say("From Server: GATE", netdaemon_table_g_word(msg, 4),
                netdaemon_table_g_word(msg, 0x10), netdaemon_table_g_word(msg, 0x14));
        action_gate(netdaemon_table_g_word(msg, 0x10), netdaemon_table_g_word(msg, 0x14));
        log_say("Acknowledging GATE", 0, 0, 0);
        tail = TailSequence;
        break;
    case 0x4411:
        log_say("From Server: UPGRADE UNIT", netdaemon_table_g_word(msg, 4),
                netdaemon_table_g_word(msg, 0x10), netdaemon_table_g_word(msg, 0x14));
        my_srand(netdaemon_table_g_word(msg, 8));
        do_upgrade(netdaemon_table_g_word(msg, 0x10), netdaemon_table_g_word(msg, 0x14),
                   netdaemon_table_g_word(msg, 0x18));
        tail = TailUpgrade;
        break;
    case 0x4412:
        log_say("From Server: SATELLITE ATTACK", netdaemon_table_g_word(msg, 0x10),
                netdaemon_table_g_word(msg, 0x14), netdaemon_table_g_word(msg, 0x18));
        log_say("From Server: SMACX SAT ATTACK:  GSP Base Target",
                netdaemon_table_g_word(msg, 0x1c), 0, 0);
        my_srand(netdaemon_table_g_word(msg, 8));
        action_sat_attack(netdaemon_table_g_word(msg, 0x10), netdaemon_table_g_word(msg, 0x14),
                          netdaemon_table_g_word(msg, 0x18), netdaemon_table_g_word(msg, 0x1c));
        log_say("Acknowledging SATELLITE ATTACK", 0, 0, 0);
        tail = TailDispatch;
        break;
    case 0x4413:
        log_say("From Server: DESTROY TERRAIN2", netdaemon_table_g_word(msg, 0x10),
                netdaemon_table_g_word(msg, 0x14), netdaemon_table_g_word(msg, 0x18));
        my_srand(netdaemon_table_g_word(msg, 8));
        action_destroy(netdaemon_table_g_word(msg, 0x10), 0,
                       netdaemon_table_g_word(msg, 0x14), netdaemon_table_g_word(msg, 0x18));
        tail = TailSequence;
        break;
    case 0x4414:
        log_say("From Server: NERVE STAPLE", netdaemon_table_g_word(msg, 0x10), 0, 0);
        my_srand(netdaemon_table_g_word(msg, 8));
        action_staple(netdaemon_table_g_word(msg, 0x10));
        log_say("Acknowledging NERVE STAPLE", 0, 0, 0);
        tail = TailSequence;
        break;
    case 0x4415:
        log_say("From Server: RETIRE PROTO", netdaemon_table_g_word(msg, 4),
                netdaemon_table_g_word(msg, 0x10), netdaemon_table_g_word(msg, 0x14));
        my_srand(netdaemon_table_g_word(msg, 8));
        retire_proto(netdaemon_table_g_word(msg, 0x10), netdaemon_table_g_word(msg, 0x14));
        tail = TailSequence;
        break;
    case 0x4417:
        my_srand(netdaemon_table_g_word(msg, 8));
        action_tectonic(netdaemon_table_g_word(msg, 0x10), netdaemon_table_g_word(msg, 0x14),
                        netdaemon_table_g_word(msg, 0x18));
        log_say("Acknowledging TECTONIC", 0, 0, 0);
        tail = TailDispatch;
        break;
    case 0x4418:
        my_srand(netdaemon_table_g_word(msg, 8));
        action_fungal(netdaemon_table_g_word(msg, 0x10), netdaemon_table_g_word(msg, 0x14),
                      netdaemon_table_g_word(msg, 0x18));
        log_say("Acknowledging FUNGAL", 0, 0, 0);
        tail = TailDispatch;
        break;
    default:
        return false;
    }

    if (tail != TailUpgrade) {
        if (daemon.primary_state_.leading_words_[0] == 0) {
            daemon.deletion_list_.set_sequence_bits(netdaemon_table_g_word(msg, 0x0c));
        }
    }

    if (tail == TailSequence) {
        if (netdaemon_table_g_word(msg, 4) == ConsoleGlobal.faction_id_) {
            // 0x536D5E local-owner tail, including its original clear/callback
            // order. The notification branch observes and clears dword 0x1BC0
            // before broadcasting 0x2214; the other arm only clears 0x1BB0.
            if (daemon.channels_[1].leading_words_[7] != 0) {
                log_say("Remaining Locked2", 0, 0, 0);
                daemon.channels_[1].leading_words_[7] = 0;
                message_data(0x2214, 0, 0, 0, 0, 0);
            } else {
                daemon.channels_[1].leading_words_[3] = 0;
            }
        }
    } else if (tail == TailDispatch &&
               netdaemon_table_g_word(msg, 4) == ConsoleGlobal.faction_id_) {
        if (daemon.channels_[1].leading_words_[7] != 0) {
            log_say("Remaining Locked2", 0, 0, 0);
            daemon.channels_[1].leading_words_[7] = 0;
            message_data(0x2214, 0, 0, 0, 0, 0);
        } else {
            daemon.channels_[1].leading_words_[3] = 0;
        }
    }

    // 0x538266 is the actual shared exit of all these arms, including 4411.
    // It clears the await marker only for a matching marked opcode addressed
    // to this faction; retain the native marker/indexing rather than assigning
    // a new semantic member name.
    const uint32_t pending_opcode = daemon.channels_[1].leading_words_[1];
    const unsigned short final_type =
        *reinterpret_cast<unsigned short *>(msg);
    if (pending_opcode != 0 && (msg[1] & 0x40) != 0 &&
        (final_type & 0x4000) != 0 &&
        final_type == static_cast<unsigned short>(pending_opcode | 0x4000) &&
        netdaemon_table_g_word(msg, 4) == ConsoleGlobal.faction_id_) {
        daemon.channels_[1].leading_words_[1] = 0;
    }
    return true;
}

// Source-only implementation candidate for Table H inside the existing
// NetDaemon::process_message owner. These opcode labels are not independent
// ORIGINAL identities. Return true after handling so the caller continues
// through process_message's actual shared tail (including the conditional
// lock-word clear and ordinary C++ scope cleanup); never return from the
// dispatcher itself here.
static void netdaemon_table_h_add_energy(int &energy, int delta) {
    // The pinned instruction is a 32-bit ADD. Express its low-dword result
    // without invoking C++ signed-overflow undefined behavior.
    uint32_t bits;
    memcpy(&bits, &energy, sizeof(bits));
    bits += static_cast<uint32_t>(delta);
    memcpy(&energy, &bits, sizeof(bits));
}

static bool netdaemon_handle_table_h(char *message) {
    const unsigned short opcode =
        *reinterpret_cast<const unsigned short *>(message);
    switch (opcode) {
    case 0x4441: {
        const int actor = *reinterpret_cast<int *>(message + 0x10);
        const int other = *reinterpret_cast<int *>(message + 0x14);
        const int flags = *reinterpret_cast<int *>(message + 0x18);
        log_say_hex("Received TREATY ON", actor, other, flags);
        treaty_on(actor, other, flags);

        if (actor == ConsoleGlobal.faction_id_ ||
            other == ConsoleGlobal.faction_id_)
            break;
        if (!spying(actor) && !spying(other) &&
            (GameDataStateStorage.fields_.game_state_ & 0x80u) == 0)
            break;

        GenderDefault = Players[actor].noun_gender;
        PluralityDefault = Players[actor].is_noun_plural;
        parse_says(0, Players[actor].noun_faction, -1, -1);
        GenderDefault = Players[other].noun_gender;
        PluralityDefault = Players[other].is_noun_plural;
        parse_says(1, Players[other].noun_faction, -1, -1);
        parse_says(2, get_pact_hood(actor, other), -1, -1);

        if (flags & 1) {
            NetMsgPrimary.pop("ENEMIESTEAMUP2", 0, 0, 0);
        } else if (flags & 2) {
            NetMsgPrimary.pop("ENEMYTREATY", 0, 0, 0);
        } else if ((flags & 4) && (flags & 0x10000) &&
                   has_treaty(actor, other, 0x10)) {
            NetMsgPrimary.pop("ENEMYTRUCE", 0, 0, 0);
        } else if (flags & 0x10) {
            NetMsgPrimary.pop("ENEMYWAR", 0, 0, 0);
        }
        break;
    }
    case 0x4442: {
        const int actor = *reinterpret_cast<int *>(message + 0x10);
        const int other = *reinterpret_cast<int *>(message + 0x14);
        const int flags = *reinterpret_cast<int *>(message + 0x18);
        log_say_hex("Received TREATY OFF", actor, other, flags);
        treaty_off(actor, other, flags);
        break;
    }
    case 0x4443: {
        const int actor = *reinterpret_cast<int *>(message + 0x10);
        const int other = *reinterpret_cast<int *>(message + 0x14);
        const int treaty = *reinterpret_cast<int *>(message + 0x18);
        const int set = *reinterpret_cast<int *>(message + 0x1C);
        log_say_hex("Received SET TREATY", actor, other, treaty);
        set_treaty(actor, other, treaty, set);
        break;
    }
    case 0x4444: {
        const int actor = *reinterpret_cast<int *>(message + 0x10);
        const int other = *reinterpret_cast<int *>(message + 0x14);
        const int agenda = *reinterpret_cast<int *>(message + 0x18);
        log_say_hex("Received AGENDA ON", actor, other, agenda);
        agenda_on(actor, other, agenda);
        break;
    }
    case 0x4445: {
        const int actor = *reinterpret_cast<int *>(message + 0x10);
        const int other = *reinterpret_cast<int *>(message + 0x14);
        const int agenda = *reinterpret_cast<int *>(message + 0x18);
        log_say_hex("Received AGENDA OFF", actor, other, agenda);
        agenda_off(actor, other, agenda);
        break;
    }
    case 0x4446: {
        const int actor = *reinterpret_cast<int *>(message + 0x10);
        const int other = *reinterpret_cast<int *>(message + 0x14);
        const int agenda = *reinterpret_cast<int *>(message + 0x18);
        const int set = *reinterpret_cast<int *>(message + 0x1C);
        log_say_hex("Received SET AGENDA", actor, other, agenda);
        set_agenda(actor, other, agenda, set);
        break;
    }
    case 0x4447: {
        const int actor = *reinterpret_cast<int *>(message + 0x10);
        const int amount = *reinterpret_cast<int *>(message + 0x14);
        const int other = *reinterpret_cast<int *>(message + 0x18);
        const int other_amount = *reinterpret_cast<int *>(message + 0x1C);
        log_say("Received ENERGY", actor, amount, other);
        if (actor != 0)
            netdaemon_table_h_add_energy(PlayersData[actor].energy_reserves,
                                         amount);
        if (other != 0)
            netdaemon_table_h_add_energy(PlayersData[other].energy_reserves,
                                         other_amount);
        break;
    }
    case 0x4448: {
        const int actor = *reinterpret_cast<int *>(message + 0x10);
        const int other = *reinterpret_cast<int *>(message + 0x14);
        log_say("Received TRADE MAPS", actor, other, 0);
        trade_maps(actor, other);
        break;
    }
    case 0x4449: {
        const int actor = *reinterpret_cast<int *>(message + 0x10);
        const int other = *reinterpret_cast<int *>(message + 0x14);
        log_say("Received CEDE BASE", actor, other, 0);
        my_srand(*reinterpret_cast<int *>(message + 8));
        give_a_base(actor, other);
        break;
    }
    case 0x444A: {
        const int actor = *reinterpret_cast<int *>(message + 0x10);
        const int other = *reinterpret_cast<int *>(message + 0x14);
        log_say("Received WITHDRAWAL", actor, other, 0);
        my_srand(*reinterpret_cast<int *>(message + 8));
        do_withdrawal(actor, other);
        break;
    }
    case 0x444B:
    case 0x444C: {
        const int actor = *reinterpret_cast<int *>(message + 0x10);
        const int tech = *reinterpret_cast<int *>(message + 0x14);
        const int recipient = *reinterpret_cast<int *>(message + 0x18);
        log_say("From Server: TECH", actor, tech, recipient);
        my_srand(*reinterpret_cast<int *>(message + 8));
        tech_achieved(actor, tech, recipient, 0);

        // The two opcodes share the original target; only 444C may announce
        // the tech to the local recipient.
        if (opcode == 0x444C && recipient == ConsoleGlobal.faction_id_) {
            GenderDefault = Players[actor].noun_gender;
            PluralityDefault = Players[actor].is_noun_plural;
            parse_says(0, Players[actor].noun_faction, -1, -1);
            StringTemp[0] = 0;
            say_tech(StringTemp, tech, 0);
            parse_says(1, StringTemp, -1, -1);
            NetMsgPrimary.pop("STOLETECH", 0, 0, 0);
        }
        break;
    }
    case 0x444D: {
        const int actor = *reinterpret_cast<int *>(message + 0x10);
        const int other = *reinterpret_cast<int *>(message + 0x14);
        log_say("Received PACT ENDS", actor, other, 0);
        my_srand(*reinterpret_cast<int *>(message + 8));
        pact_ends(actor, other);
        break;
    }
    case 0x444E: {
        const int actor = *reinterpret_cast<int *>(message + 0x10);
        const int other = *reinterpret_cast<int *>(message + 0x14);
        const int balance = *reinterpret_cast<int *>(message + 0x18);
        const int payment = *reinterpret_cast<int *>(message + 0x1C);
        log_say("Received LOAN", actor, other, balance);
        // The pinned body performs both stores unconditionally and addresses
        // both fields in the p10 PlayerData row; preserve the unchecked access.
        PlayersData[actor].loan_balance[other] = balance;
        PlayersData[actor].loan_payment[other] = payment;
        break;
    }
    case 0x444F: {
        const int actor = *reinterpret_cast<int *>(message + 0x10);
        const int year = *reinterpret_cast<int *>(message + 0x14);
        const int other = *reinterpret_cast<int *>(message + 0x18);
        log_say_hex("Received CORNERING", actor, year, other);
        if (actor != ConsoleGlobal.faction_id_) {
            GenderDefault = 0;
            PluralityDefault = 0;
            parse_says(0, Players[actor].title_leader, -1, -1);
            parse_says(1, Players[actor].name_leader, -1, -1);
            GenderDefault = Players[actor].noun_gender;
            PluralityDefault = Players[actor].is_noun_plural;
            parse_says(2, Players[actor].noun_faction, -1, -1);
            parse_says(3, &Players[actor].adj_name_faction[4], -1, -1);
            parse_num(0, static_cast<int>(game_year(year)));
            popp(ScriptPopupFile, "CORNERWARNING", 0, "econwin_sm.pcx", 0);
        }
        break;
    }
    default:
        return false;
    }
    return true;
}

// Proposal fragment only. Include in NetDaemon's source owner after its
// project headers. The request-mask reference deliberately keeps the 0x7FFF74
// owner/layout unresolved rather than inventing an address-backed C++ alias.
static int netdaemon_run_contact_prompt(Popup &popup, int faction_id) {
    popup.start(PopupStartCaption, "COMMDIPLO", -1, 0, 0, 0);
    popup.dialogs().SpriteBox::sprite(
        FactionDiplomacyPortraits[faction_id], 0, 0);
    return popup.exec(0, pop_expire);
}

static bool netdaemon_handle_direct_diplomacy_complete(
    unsigned short type, unsigned char *message, Popup &popup,
    int &contact_response_mask) {
    switch (type) {
    case 0x1501: {
        const int sender = *reinterpret_cast<int *>(message + 4);
        const uint32_t sender_bit =
            1u << (static_cast<uint32_t>(sender) & 31u);
        log_say("Received OPEN CHAN", sender, 0, 0);

        if ((static_cast<uint32_t>(contact_response_mask) & sender_bit) != 0) {
            message_data(0x1503, sender, 0, 0, 0, 0);
            return true;
        }
        if (NetDaemonNet.channels_[1].leading_words_[2] != 0) {
            message_data(0x1504, sender, 0, 0, 0, 0);
            return true;
        }
        if (CommlinkPending != 0) {
            if (sender == CommlinkFaction &&
                DiploWindow.is_visible() == 0) {
                message_data(0x1502, sender, 0, 0, 0, 0);
                CommlinkPending = 1;
                diplo(ConsoleGlobal.faction_id_, sender);
            } else {
                message_data(0x1504, sender, 0, 0, 0, 0);
            }
            return true;
        }
        if (DiplomacyActive != 0) {
            message_data(0x1504, sender, 0, 0, 0, 0);
            return true;
        }
        if (PopupWaitActive != 0) {
            message_data(0x1504, sender, 0, 0, 1, 0);
            return true;
        }

        for (;;) {
            prepare_contact_dialog_arguments(sender);
            const int response = netdaemon_run_contact_prompt(popup, sender);
            if ((GameDataStateStorage.fields_.game_state_ & 0x800u) != 0) {
                message_data(0x1503, sender, response == 2, 0, 0, 0);
                return true;
            }
            if (response == 0) {
                message_data(0x1502, sender, 0, 0, 0, 0);
                CommlinkPending = 1;
                diplo(ConsoleGlobal.faction_id_, sender);
                return true;
            }
            if (response == 4) {
                report_intel(sender);
                continue;
            }
            if (response == 3) {
                contact_response_mask = static_cast<int>(
                    static_cast<uint32_t>(contact_response_mask) | sender_bit);
            }
            message_data(0x1503, sender, response == 2, 0, 0, 0);
            return true;
        }
    }
    case 0x1502: {
        const int sender = *reinterpret_cast<int *>(message + 4);
        log_say("Received OPEN ACKNOWLEDGE", sender, 0, 0);
        if (CommlinkPending != 0 && sender != CommlinkFaction) {
            message_data(0x1505, sender, 0, 0, 0, 0);
            return true;
        }
        if (DiploWindow.is_visible() != 0) return true;
        message_data(0x2441, 0, ConsoleGlobal.faction_id_, sender, 8, 0);
        diplo(ConsoleGlobal.faction_id_, sender);
        return true;
    }
    case 0x1503: {
        const int sender = *reinterpret_cast<int *>(message + 4);
        CommlinkPending = 0;
        log_say("Received OPEN REFUSE", sender, 0, 0);
        GenderDefault = Players[sender].is_leader_female;
        PluralityDefault = 0;
        parse_says(0, Players[sender].title_leader, -1, -1);
        GenderDefault = Players[sender].is_leader_female;
        PluralityDefault = 0;
        parse_says(1, Players[sender].name_leader, -1, -1);
        const int response = *reinterpret_cast<int *>(message + 0x10);
        if (response == 0)
            NetMsgPrimary.pop("DIPLOREFUSE", 5000, 0, 0);
        else if (response == 1)
            NetMsgPrimary.pop("DIPLOREFUSE1", 5000, 0, 0);
        return true;
    }
    case 0x1504: {
        const int sender = *reinterpret_cast<int *>(message + 4);
        CommlinkPending = 0;
        log_say("Received OPEN BUSY", sender, 0, 0);
        GenderDefault = Players[sender].is_leader_female;
        PluralityDefault = 0;
        parse_says(0, Players[sender].title_leader, -1, -1);
        GenderDefault = Players[sender].is_leader_female;
        PluralityDefault = 0;
        parse_says(1, Players[sender].name_leader, -1, -1);
        const int response = *reinterpret_cast<int *>(message + 0x10);
        if (response == 0)
            NetMsgPrimary.pop("DIPLOBUSY", 5000, 0, 0);
        else if (response == 1)
            NetMsgPrimary.pop("DIPLOBUSY1", 5000, 0, 0);
        return true;
    }
    case 0x150A: {
        const int sender = *reinterpret_cast<int *>(message + 4);
        log_say("Received MAYBE DEAL", sender, 0, 0);
        DiploWindow.receive_maybe_deal(sender);
        return true;
    }
    default:
        return false;
    }
}

// Pinned direct targets 0x535BDB (LOCK) and 0x535D07 (LOCK GLOBAL). The
// original Lock methods own their record/global-lock mutation; preserve the
// packet argument order and the separate global-lock release test.
static bool netdaemon_handle_client_lock_message(NetDaemon *daemon,
                                                  unsigned short type,
                                                  unsigned char *message,
                                                  int sender_identity) {
    const int sender = sender_identity;
    if (type == 0x2210) {
        const int field_10 = *reinterpret_cast<int *>(message + 0x10);
        const int field_14 = *reinterpret_cast<int *>(message + 0x14);
        log_say("From Client: LOCK", sender, field_10, field_14);

        if ((SpockHelpActive != 0 || TurnControlWord93A954 != 0) &&
            sender != ConsoleGlobal.faction_id_) {
            log_say("(Sending: Lock failed -- REJECT)", 0, 0, 0);
            message_data(0x1203, sender, 0, 0, 0, 0);
            return true;
        }
        if (ConsoleControlTurnActive != 0) {
            log_say("(Sending: Lock failed -- UPKEEP)", 0, 0, 0);
            message_data(0x1203, sender, 0, 0, 0, 0);
            return true;
        }

        const int result = daemon->locks_.lock(
            sender, *reinterpret_cast<int *>(message + 0x18),
            field_10, field_14,
            *reinterpret_cast<int *>(message + 0x1C),
            *reinterpret_cast<int *>(message + 0x24),
            *reinterpret_cast<int *>(message + 0x20));
        if (result != 0) {
            log_say("(Sending: Lock Failed)", 0, 0, 0);
            message_data(0x1201 +
                         static_cast<int>(daemon->locks_.field_E0_ != 0),
                         sender, 0, 0, 0, 0);
            return true;
        }

        log_say("(Sending: Lock OK)", 0, 0, 0);
        if ((*reinterpret_cast<unsigned char *>(message + 0x18) & 2u) != 0 &&
            daemon->locks_.check_global_2(sender) != 0) {
            message_data(0x2216, sender, 0, 0, 0, 0);
        } else {
            message_data(0x1201, sender, 0, 0, 0, 0);
        }
        return true;
    }

    if (type == 0x2211) {
        log_say("From Client: LOCK GLOBAL", sender, 0, 0);
        if (daemon->locks_.check_global_2(sender) != 0) {
            message_data(0x1201 +
                         static_cast<int>(daemon->locks_.field_E0_ != 0),
                         sender, 0, 0, 0, 0);
        }
        daemon->locks_.check_global();
        return true;
    }
    return false;
}

// The session packets carry opaque fixed-size records already owned by NetWin.
// Keep the original bulk-copy extents and post-copy opcode reread/rebroadcast
// order; these are not a new packet-shaped global or a second storage owner.
static void netdaemon_rebroadcast_session_packet(NetDaemon *daemon,
                                                  char *message,
                                                  unsigned long message_size) {
    unsigned short type = *reinterpret_cast<unsigned short *>(message);
    if ((type & 0x2000u) != 0) {
        type = static_cast<unsigned short>((type & 0xDFFFu) | 0x4000u);
        *reinterpret_cast<unsigned short *>(message) = type;
        daemon->send_message(message, message_size, 0);
    }
}

static void netdaemon_handle_session_options(NetDaemon *daemon,
                                              char *message,
                                              unsigned long message_size) {
    NetWinSessionOptions.receive_wire_block(
        reinterpret_cast<const unsigned char *>(message + 0x10));
    netdaemon_rebroadcast_session_packet(daemon, message, message_size);
}

static void netdaemon_handle_session_block(NetDaemon *daemon,
                                            char *message,
                                            unsigned long message_size) {
    NetWinSessionData.receive_wire_block(
        reinterpret_cast<const unsigned char *>(message + 0x10));
    unsigned short type = *reinterpret_cast<unsigned short *>(message);
    if ((type & 0x2000u) != 0) {
        type = static_cast<unsigned short>((type & 0xDFFFu) | 0x4000u);
        *reinterpret_cast<unsigned short *>(message) = type;
        daemon->send_message(message, message_size, 0);
    } else {
        NetWinDialog.on_redraw();
    }
}

static void netdaemon_handle_session_status(NetDaemon *daemon,
                                             unsigned short type,
                                             char *message,
                                             unsigned long message_size) {
    const int faction = *reinterpret_cast<int *>(message + 4);
    if (type == 0x1F00 || type == 0x4E03 || type == 0x4F00)
        daemon->primary_state_.leading_words_[1] = 1;

    unsigned char *status = daemon->process_slots_[faction].status_bytes_;
    status[0] = static_cast<unsigned char>(message[0x18]);
    status[2] = static_cast<unsigned char>(message[0x14]);
    status[1] = static_cast<unsigned char>(message[0x10]);
    netdaemon_rebroadcast_session_packet(daemon, message, message_size);
}

// The three lobby-state targets 1F04/2F04/4F04 share the body at 0x537D62;
// 1F04 and 4F04 additionally set NetDaemon+0x1B34 before entering it.
// EDX is the dispatcher sentinel -1; EDI is reloaded from packet +4. Do not
// replace that sentinel with a snapshot of the packet sender.
static void netdaemon_handle_session_lobby(NetDaemon &daemon,
                                           unsigned char *message,
                                           unsigned short type,
                                           unsigned long message_size) {
    if (type == 0x1F04 || type == 0x4F04)
        daemon.primary_state_.leading_words_[1] = 1;

    unsigned short &wire_type = *reinterpret_cast<unsigned short *>(message);
    const uint32_t sender = *reinterpret_cast<uint32_t *>(message + 4);
    const uint32_t no_sender_sentinel = 0xFFFFFFFFu;
    const bool has_sender = sender != no_sender_sentinel;

    if ((wire_type & 0x2000u) != 0) {
        wire_type = static_cast<unsigned short>(
            (wire_type & static_cast<unsigned short>(~0x2000u)) | 0x4000u);

        if (!has_sender) {
            daemon.send_message(reinterpret_cast<char *>(message),
                                message_size, 0);
            return;
        }

        // EBP-0x14 begins as the saved NetDaemon `this` from 0x53442D. For
        // any non-sentinel sender, 0x537D80 overwrites it with the old signed
        // faction byte in that sender's row before any selection update. The
        // sentinel paths return above without observing the saved-this value.
        const intptr_t previous_faction =
            NetWinLobbyRows[sender].faction_or_color_;

        const unsigned int mode = NetWinSessionData.mode_;
        if (mode == 2 || mode == 3) {
            const int faction = static_cast<int8_t>(message[0x13]);
            if (faction != -1) {
                const NetWinFactionRecord *const records =
                    static_cast<const NetWinFactionRecord *>(
                        NetWinDialog.faction_records_);
                if (records[faction].owner_ != 0 &&
                    records[faction].owner_ !=
                        *reinterpret_cast<uint32_t *>(message + 0x188)) {
                    return;
                }

                for (int row_index = 1;
                     row_index <= NetDaemonNet.player_count_; ++row_index) {
                    if (static_cast<uint32_t>(row_index) == sender) continue;
                    const int row_faction =
                        NetWinLobbyRows[row_index].faction_or_color_;
                    if (row_faction != -1 && row_faction == faction) return;
                }

            }
            // The native -1 branch skips ownership/duplicate validation but
            // still reaches the shared selected-row stores at 0x537E3C/48.
            NetWinLobbyRows[sender].faction_or_color_ =
                static_cast<int8_t>(faction);
            NetWinLobbyRows[sender].owner_ =
                *reinterpret_cast<uint32_t *>(message + 0x188);
        }

        // This comparison runs for every non-sentinel sender, even when the
        // packet carries faction -1 or the dialog mode skipped selection.
        const int current_faction =
            NetWinLobbyRows[sender].faction_or_color_;
        const int faction = static_cast<int8_t>(message[0x13]);
        if (current_faction != faction ||
            previous_faction != current_faction) {
            NetWinDialog.fill_player(
                reinterpret_cast<NetWinLobbyRow *>(message + 0x10), faction);
        }

        daemon.send_message(reinterpret_cast<char *>(message),
                            message_size, 0);
        // The marked-bit branch jumps directly to the common process_message
        // tail at 0x537E91/0x538266 after send_message; it does not notify or
        // redraw here.
        return;
    }

    if (!has_sender) {
        std::memcpy(&NetWinLobbyRows[1], message + 0x10, 0xA64);
        NetWinDialog.on_redraw();
        return;
    }

    std::memcpy(&NetWinLobbyRows[sender], message + 0x10,
                sizeof(NetWinLobbyRow));
    if (daemon.primary_state_.leading_words_[0] != 0) {
        const unsigned int tail_mode = NetWinSessionData.mode_;
        if (tail_mode == 2 || tail_mode == 3) {
            unsigned char notification[0x20];
            *reinterpret_cast<unsigned short *>(notification) = 0x0F0E;
            NetDaemonNet.Net::send(
                notification,
                sizeof(notification),
                static_cast<unsigned long>(daemon.process_slots_[sender].pid_),
                1);
        }
    }
    NetWinDialog.on_redraw();
}

// Pinned process_message targets 0x536781, 0x536835, 0x536874, 0x5368E0,
// and 0x536D0F, projected as source-owned routes on the existing daemon.
// Preserve the distinct checksum source and local-completion branch.
static void netdaemon_process_local_player_completion(NetDaemon &daemon) {
    const uint32_t remaining = daemon.channels_[1].trailing_words_[2];
    if (remaining != 0) {
        log_say("Remaining Locked2", 0, 0, 0);
        daemon.channels_[1].trailing_words_[2] = 0;
        message_data(0x2214, 0, 0, 0, 0, 0);
    } else {
        daemon.channels_[1].leading_words_[3] = 0;
    }
}

// Borrowed wire view over the layout already owned by message_data's sender.
// Read one field at a time: MOVE packets have only two payload words, while
// AIRDROP/DOUBLE CROSS have three. Do not copy a full 32-byte record, impose
// alignment, cache fields across calls, or initialize original unwritten bytes.
// This is a serialization boundary, not a claim about a new original class.
class MessageCommandView {
public:
    explicit MessageCommandView(const unsigned char *bytes) : bytes_(bytes) {}
    int faction() const {
        int value;
        std::memcpy(&value, bytes_ + offsetof(MessageDataRecord, faction_), sizeof value);
        return value;
    }
    int timestamp_bits() const {
        int value;
        std::memcpy(&value, bytes_ + offsetof(MessageDataRecord, timestamp_), sizeof value);
        return value;
    }
    uint32_t sequence() const {
        uint32_t value;
        std::memcpy(&value, bytes_ + offsetof(MessageDataRecord, sequence_), sizeof value);
        return value;
    }
    int payload_10() const {
        int value;
        std::memcpy(&value, bytes_ + offsetof(MessageDataRecord, payload_10_), sizeof value);
        return value;
    }
    int payload_14() const {
        int value;
        std::memcpy(&value, bytes_ + offsetof(MessageDataRecord, payload_14_), sizeof value);
        return value;
    }
    int payload_18() const {
        int value;
        std::memcpy(&value, bytes_ + offsetof(MessageDataRecord, payload_18_), sizeof value);
        return value;
    }
private:
    const unsigned char *bytes_;
};

static bool netdaemon_handle_five_message_routes(
        NetDaemon &daemon, unsigned short type, unsigned char *message,
        int entry_packet_faction, uint32_t entry_sequence) {
    // Original stack+8 is the faction captured at534466, not a transport
    // identity. Stack-10 is the packet sequence captured at534461, not its
    // byte length. The DOUBLE CROSS arm instead rereads both packet fields.
    const MessageCommandView packet(message);
    switch (type) {
    case 0x4401: {
        const int vehicle_id = packet.payload_10();
        const int direction = packet.payload_14();
        log_say("From Server: MOVE VEH", packet.faction(), vehicle_id, direction);
        my_srand(packet.timestamp_bits());
        bool execute_order = true;
        if (Vehs[vehicle_id].faction_id != packet.faction()) {
            if (daemon.primary_state_.leading_words_[0] != 0) {
                SpockNotifyPending = 1;
                // Pinned5367CC jumps past order_veh to the acknowledgement.
                execute_order = false;
            } else {
                message_data(0x2308, 0, 0, 0, 0, 0);
            }
        }
        if (execute_order)
            ::order_veh(vehicle_id, packet.payload_14(), 2);
        log_say("Acknowledging MOVE VEH", entry_packet_faction, 0, 0);
        if (daemon.primary_state_.leading_words_[0] == 0)
            daemon.deletion_list_.set_sequence_bits(entry_sequence);
        if (entry_packet_faction == ConsoleGlobal.faction_id_)
            netdaemon_process_local_player_completion(daemon);
        return true;
    }
    case 0x4402: {
        const int vehicle_id = packet.payload_10();
        const int direction = packet.payload_14();
        log_say("From Server: ARTY", packet.faction(), vehicle_id, direction);
        my_srand(packet.timestamp_bits());
        daemon.channels_[0].leading_words_[3] = 2;
        battle_fight(vehicle_id, packet.payload_14(), 1, 1, nullptr);
        log_say("Acknowledging ARTY", entry_packet_faction, 0, 0);
        if (daemon.primary_state_.leading_words_[0] == 0)
            daemon.deletion_list_.set_sequence_bits(entry_sequence);
        if (entry_packet_faction == ConsoleGlobal.faction_id_)
            netdaemon_process_local_player_completion(daemon);
        return true;
    }
    case 0x4403: {
        const int vehicle_id = packet.payload_10();
        const int x = packet.payload_14();
        const int y = packet.payload_18();
        my_srand(packet.timestamp_bits());
        action_airdrop(vehicle_id, x, y, 2);
        log_say("Acknowledging AIRDROP", entry_packet_faction, 0, 0);
        if (daemon.primary_state_.leading_words_[0] == 0)
            daemon.deletion_list_.set_sequence_bits(entry_sequence);
        if (entry_packet_faction == ConsoleGlobal.faction_id_)
            netdaemon_process_local_player_completion(daemon);
        return true;
    }
    case 0x4404: {
        const int vehicle_id = packet.payload_10();
        const int order = Vehs[vehicle_id].order;
        log_say("From Server: ACTION", packet.faction(), vehicle_id, order);
        my_srand(packet.timestamp_bits());
        ::action(packet.payload_10());
        if (daemon.primary_state_.leading_words_[0] == 0)
            daemon.deletion_list_.set_sequence_bits(entry_sequence);
        if (entry_packet_faction == ConsoleGlobal.faction_id_)
            netdaemon_process_local_player_completion(daemon);
        return true;
    }
    case 0x4440: {
        const int faction = packet.payload_10();
        const int partner = packet.payload_14();
        const int victim = packet.payload_18();
        log_say("From Server: DOUBLE CROSS", faction, partner, victim);
        my_srand(packet.timestamp_bits());
        double_cross(packet.payload_10(),
                     packet.payload_14(),
                     packet.payload_18());
        if (daemon.primary_state_.leading_words_[0] == 0)
            daemon.deletion_list_.set_sequence_bits(packet.sequence());
        if (packet.faction() == ConsoleGlobal.faction_id_)
            netdaemon_process_local_player_completion(daemon);
        return true;
    }
    default:
        return false;
    }
}

// Candidate helper for the four pinned Council message routes in
// NetDaemon::process_message. Keep the caller's existing early file-transfer
// and 0x1101 returns ahead of this dispatch; false leaves the old default
// forwarding path untouched.
static bool process_council_message(unsigned char *msg,
                                    unsigned short type) {
    // Share sender-owned wire offsets without assuming packet alignment.
    // Original logs and actions read independently across external calls.
    const MessageCommandView packet(msg);
    switch (type) {
    case 0x4600:
        log_say("Received CONVENE COUNCIL", packet.payload_10(), packet.payload_14(), packet.payload_18());
        my_srand(packet.timestamp_bits());
        council(packet.payload_10(), packet.payload_14(), packet.payload_18());
        return true;
    case 0x4601:
        log_say("Received COUNCIL VOTE", packet.payload_10(), packet.payload_14(), 0);
        my_srand(packet.timestamp_bits());
        CouncilWindow.set_vote(packet.payload_10(), packet.payload_14());
        return true;
    case 0x4602:
        log_say("Received COUNCIL ARRIVE", packet.payload_10(), 0, 0);
        CouncilWindow.arrived(packet.payload_10());
        return true;
    case 0x4603:
        log_say("Received COUNCIL RESOLUTION", 0, 0, 0);
        CouncilWindow.make_resolution(1);
        return true;
    default:
        return false;
    }
}

// Pinned process_message targets 0x537CE9 (0x8240) and 0x537F35
// (0x8241). The 0x1B7C word is lock_state_[1].leading_words_[5], and the
// indexed 0x1B80 words are lock_state_[1].leading_words_[6..9]. `identity`
// is the resolved EDI faction value used by the native dispatcher, not the
// transport process ID passed to process_message.
static bool netdaemon_handle_lock_mask_residual(NetDaemon &daemon,
                                                 unsigned short type,
                                                 unsigned char *message,
                                                 int identity) {
    if (type == 0x8240) {
        const int first = *reinterpret_cast<int *>(message + 0x10);
        const int second = *reinterpret_cast<int *>(message + 0x14);
        const uint32_t mask =
            (uint32_t(1) << (static_cast<uint32_t>(first) & 31u)) |
            (uint32_t(1) << (static_cast<uint32_t>(second) & 31u));
        uint32_t &aggregate = daemon.channels_[0].leading_words_[5];
        const bool newly_acquired = (mask & aggregate) == 0;
        if (newly_acquired) {
            daemon.channels_[0].leading_words_[6 + identity] = mask;
            aggregate |= mask;
        }
        log_say("Received OPEN CHANNEL request", identity, first, second);
        message_data(0x1240, identity, newly_acquired, 0, 0, 0);
        return true;
    }

    if (type == 0x8241) {
        uint32_t &peer_mask =
            daemon.channels_[0].leading_words_[6 + identity];
        const uint32_t remaining =
            daemon.channels_[0].leading_words_[5] & ~peer_mask;
        daemon.channels_[0].leading_words_[5] = remaining;
        log_say("Received CLOSE CHANNEL", identity, peer_mask, remaining);
        peer_mask = 0;
        return true;
    }
    return false;
}

// Pinned direct process_message routes 0x4B00, 0x4C00, and 0x4C01. These
// compose existing NetMsg, FX, Net, and NetDaemon owners; the neighboring
// 0x4C02 time-control route has additional branches and is not covered here.
static bool netdaemon_handle_server_change_pause(NetDaemon &daemon,
                                                 unsigned short type,
                                                 int sender_id,
                                                 int identity) {
    if (type == 0x4B00) {
        log_say("Received SERVER CHANGE message", 0, 0, 0);
        NetMsgPrimary.pop("CHANGINGSERVERS", 0, 2, nullptr);
        return true;
    }
    if (type == 0x4C00) {
        uint32_t &paused = daemon.primary_state_.trailing_words_[4];
        if (paused == 0) {
            daemon.primary_state_.trailing_words_[5] =
                static_cast<uint32_t>(timeGetTime()) -
                static_cast<uint32_t>(NetDaemonWaitTicks);
            g_FX.play(21);
        }
        const uint32_t sender_bit =
            uint32_t(1) << (static_cast<uint32_t>(identity) & 31u);
        paused |= sender_bit;
        log_say("Received PAUSE message", identity,
                static_cast<int>(paused), 0);
        parse_says(0, daemon.get_player_name(
                          static_cast<unsigned long>(sender_id)), -1, -1);
        NetMsgPrimary.pop("PAUSED", 0, 2, nullptr);
        return true;
    }
    if (type == 0x4C01) {
        const uint32_t sender_bit =
            uint32_t(1) << (static_cast<uint32_t>(identity) & 31u);
        uint32_t &paused = daemon.primary_state_.trailing_words_[4];
        paused &= ~sender_bit;
        log_say("Received UNPAUSE message", identity,
                static_cast<int>(paused), 0);
        if (paused == 0) g_FX.play(21);
        parse_says(0, daemon.get_player_name(
                          static_cast<unsigned long>(sender_id)), -1, -1);
        NetMsgPrimary.pop("UNPAUSED", 0, 2, nullptr);
        return true;
    }
    return false;
}

// Positive pinned process_message routes using the current AlphaNet, Win,
// Player, map and Console owners. Packet reads deliberately retain the native
// unchecked offsets and ordering.
static bool netdaemon_handle_chat_landmark_message(NetDaemon &daemon,
                                                   unsigned char *msg,
                                                   unsigned short type,
                                                   int sender_id) {
    const int *const words = reinterpret_cast<const int *>(msg);
    const int faction = words[1];
    if (type == 0x1E00 || type == 0x4E00) {
        log_say("Received CHAT", StringTemp, faction, 0, 0);
        if ((uint32_t(1) << (static_cast<uint32_t>(faction) & 31u)) &
            static_cast<uint32_t>(MultiWindowContactStatusMask)) {
            return true;
        }
        if (daemon.primary_state_.trailing_words_[0] == 1) {
            const int slot = daemon.pid_2_idx(
                static_cast<uint32_t>(sender_id));
            if (slot < 0) return true;
            NetWinDialog.new_chat(reinterpret_cast<char *>(msg + 0x10), slot);
            return true;
        }
        if (CommlinkPending != 0) {
            DiploWindow.new_chat(reinterpret_cast<char *>(msg + 0x10),
                                 sender_id);
            return true;
        }

        StringTemp[0] = '\0';
        if (daemon.primary_state_.trailing_words_[0] < 2) {
            strcat(StringTemp, daemon.get_player_name(
                static_cast<unsigned long>(sender_id)));
        } else {
            const Player &player = Players[faction];
            PluralityDefault = 0;
            GenderDefault = player.is_leader_female;
            strcat(StringTemp, player.title_leader);
            strcat(StringTemp, " ");
            PluralityDefault = 0;
            GenderDefault = player.is_leader_female;
            strcat(StringTemp, player.name_leader);
        }
        strcat(StringTemp, ": ");
        strcat(StringTemp, reinterpret_cast<char *>(msg + 0x10));
        MainInterfaceGlobal.chat(StringTemp, faction);
        return true;
    }

    const int x = words[4];
    const int y = words[5];
    const char *const name = reinterpret_cast<const char *>(msg + 0x20);
    if (type == 0x4E01) {
        log_say("Received LANDMARK", name, x, y, 0);
        new_landmark(x, y, name);
        if (faction == ConsoleGlobal.faction_id_) draw_map(1);
        return true;
    }
    if (type == 0x4E02) {
        log_say("Received KILL LANDMARK", x, y, 0);
        kill_landmark(x, y);
        if (faction == ConsoleGlobal.faction_id_) draw_map(1);
        return true;
    }
    if (type == 0x4E03) {
        log_say("Received RENAME LANDMARK", name, x, y, 0);
        const int landmark = find_landmark(x, y, 1);
        if (landmark >= 0) strcpy(MapLandmark[landmark].name, name);
        if (faction == ConsoleGlobal.faction_id_) draw_map(1);
        return true;
    }
    return false;
}

// Pinned NetDaemon::process_message route for opcode 0x4C02. Call this after
// the preceding specialized routes and before the default message forwarding
// path. It returns true only for 0x4C02 and preserves the original unchecked
// packet/state behavior.
static int netdaemon_subtract_i32_bits(int left, int right) {
    const uint32_t bits = static_cast<uint32_t>(left) -
                          static_cast<uint32_t>(right);
    int result;
    memcpy(&result, &bits, sizeof result);
    return result;
}

static bool netdaemon_handle_time_control(NetDaemon &daemon,
                                          unsigned char *msg,
                                          unsigned short type,
                                          int sender_id,
                                          int sender_identity) {
    if (type != 0x4C02) {
        return false;
    }

    log_say("Received TIME CONTROL message", sender_identity, 0, 0);

    const int *const message_words = reinterpret_cast<const int *>(msg);
    const int elapsed_ticks = netdaemon_subtract_i32_bits(
        GameDataStateStorage.fields_.turn_control_duration_milliseconds_,
        NetDaemonWaitTicks);
    GameDataStateStorage.fields_.time_controls_selection_ = message_words[4];
    set_time_controls();
    NetDaemonWaitTicks = netdaemon_subtract_i32_bits(
        GameDataStateStorage.fields_.turn_control_duration_milliseconds_,
        elapsed_ticks);

    if (sender_identity == ConsoleGlobal.faction_id_) {
        NetMsgPrimary.pop("WECHANGEDTIME", 5000, 0, 0);
        return true;
    }

    parse_says(0, daemon.Net::get_player_name(
                      static_cast<uint32_t>(sender_id)), -1, -1);
    const RulesTimeControl &control =
        TimeControl[GameDataStateStorage.fields_.time_controls_selection_];
    parse_num(0, control.turn);
    parse_num(1, control.base);
    parse_num(2, control.unit);
    parse_num(3, control.extra);
    parse_num(4, control.refresh);
    parse_num(5, control.accumulated);

    StringTemp[0] = 0;
    strcat(StringTemp, "HECHANGEDTIME");
    say_num(GameDataStateStorage.fields_.time_controls_selection_ != 0);
    log_say("He Changed:", StringTemp, 0, 0, 0);
    NetMsgPrimary.pop(StringTemp, 5000, 0, 0);
    return true;
}

// Pinned NetDaemon::process_message arm 0x4D00 -> 0x537775. This is a
// proposal fragment for insertion in the existing owner, not a new identity.
// Keep packet-derived project and faction indices unchecked as in the image.
static void netdaemon_process_project_4d00(unsigned char *message) {
    const int project_id = *reinterpret_cast<int *>(message + 0x10);
    const int project_status = *reinterpret_cast<int *>(message + 0x14);
    const int related_project_id = *reinterpret_cast<int *>(message + 0x18);
    const int faction_id = *reinterpret_cast<int *>(message + 0x04);

    const RulesFacility &project = Facility[FacilitySPStart + project_id];
    LPSTR project_name = StringTable.get(project.name);
    log_say("Received PROJECT", project_name, faction_id, project_status,
            related_project_id);

    if (faction_id == ConsoleGlobal.faction_id_) {
        return;
    }
    if (project_status < 0) {
        NetMsgPrimary.pop("HALTPROJECT", 5000, 2, nullptr);
        return;
    }
    if (project_status > 2) {
        return;
    }

    const Player &faction = Players[faction_id];
    if (project_status == 2) {
        // The original leaves both parser defaults untouched for slots 0/1.
        parse_says(0, faction.adj_name_faction, -1, -1);
        LPSTR related_name =
            Facility[FacilitySPStart + related_project_id].name;
        parse_say(1, related_name, -1, -1);

        GenderDefault = faction.noun_gender;
        PluralityDefault = faction.is_noun_plural;
        parse_says(2, faction.noun_faction, -1, -1);
        parse_say(3, project_name, -1, -1);
        NetMsgPrimary.pop("CHANGEPROJECT", 5000, 2, nullptr);
        return;
    }

    PluralityDefault = 0;
    GenderDefault = faction.is_leader_female;
    parse_says(0, faction.title_leader, -1, -1);

    GenderDefault = faction.is_leader_female;
    PluralityDefault = 0;
    parse_says(1, faction.name_leader, -1, -1);

    GenderDefault = faction.noun_gender;
    PluralityDefault = faction.is_noun_plural;
    parse_says(2, faction.noun_faction, -1, -1);
    parse_say(3, project_name, -1, -1);

    if (project_status == 0) {
        NetMsgPrimary.pop("BEGINPROJECT", 5000, 2, nullptr);
        return;
    }

    const int base_id = base_project(project_id);
    if (base_id >= 0 && Bases[base_id].faction_id_current == faction_id) {
        return;
    }
    NetMsgPrimary.pop("HALTPROJECT", 5000, 2, nullptr);
}

// ORIGINAL: 0x00534400 ?process_message@NetDaemon@@QAEXPADKH@Z 0x00534400-0x005388B9;0x0065E0C2-0x0065E442 FILE
// Historical seed status: the following TRIED describes the superseded
// first-band-only body, not the dispatch arms now implemented below.
// The cold span is the same frame's compiler-generated exception cleanup;
// automatic Popup/Filemap and nested modal objects own normal/unwind teardown.
// TRIED: no Ghidra hypothesis was available for this one (raw asm only). The real dispatcher spans ~67 distinct message-type case bodies across several chained range checks up to type 0x1300+; only the first band (0xF05-0xF0F, 11 cases) and the shared default "fixup_message + send_message" forwarding path (the target of the large majority of jump-table slots) were read from the disassembly and modelled. Everything above type 0x1101 is unimplemented.
// working copy - scaffold materialised by --work
// size      18489 bytes
// prototype void (__thiscall ?process_message@NetDaemon@@QAEXPADKH@Z)(NetDaemon* this, MessageFactionData*, unsigned int, int)
// callers   18   call targets   133
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004048A0 0x00404900 0x00406380 0x004064D0 0x00406880 0x00424410 0x00424720 0x004271C0 0x00428350 0x004412F0 0x004413F0 0x00441490 0x004426F0 0x00442740 0x004429C0 0x00444D90 0x00446A00 0x0045C0B0 0x0046B190 0x00479330 0x0047A890 0x0047C970 0x00481AD0 0x00481DA0 0x00481FD0 0x00483420 0x0048C0A0 0x004ACBD0 0x004C96E0 0x004C9B00 0x004CA7F0 0x004CAA50 0x004CC360 0x004CD2F0 0x004CD4E0 0x004CE210 0x004CE790 0x004CEDE0 0x004CF380 0x004CF480 0x004CF740 0x004E25E0 0x004E2610 0x004E2660 0x004E26B0 0x004EFB50 0x00506A60 0x0050BA30 0x0050BCC0 0x00523C60 0x00532940 0x00532E00 0x00534330 0x00534400 0x00538FB0 0x00538FE0 0x0053C650 0x00543A20 0x00548F80 0x0054CF40 0x0055B760 0x0055B820 0x0055B870 0x0055BA80 0x0055BB30 0x0055BBA0 0x0055BC00 0x0057F1D0 0x0058FB00 0x0058FC50 0x0058FFA0 0x00590170 0x005901D0 0x00590240 0x005902C0 0x00590300 0x00590470 0x00592550 0x00592600 0x005926F0 0x00592EE0 0x005947C0 0x005B9C40 0x005BB000 0x005BF310 0x005BF420 0x005C0B00 0x005C89A0 0x005C89B0 0x005D4510 0x005D45B0 0x005D4DD0 0x005D5A70 0x005E3820 0x005EE280 0x005F7E90 0x005FA870 0x00600F00 0x00602600 0x00605370 0x00607040 0x00607DA0 0x00608E10 0x00609EC0 0x0060D1B0 0x0060E740 0x00610120 0x006108E0 0x00611A20 0x00612960 0x006169A0 0x00625810 0x00625E30 0x00625E50 0x00625EC0 0x00626250 0x006262F0 0x006263F0 0x006272F0 0x00628380 0x006283E0 0x00628650 0x006287C0 0x00630080 0x00630A00 0x00631A20 0x00634330 0x00644EF2 0x00645460 0x00645470 0x00645550 0x00645930 0x0064FC88
// indirect  0x00534EC9 0x00534ED0 0x005374DF 0x00537F83 0x00538561 0x00538573 0x00538595 0x00538611 0x00538623 0x00538645 0x005386E0 0x005386F4 0x00538715 0x00538791 0x005387A5 0x005387C6

void NetDaemon::process_message(char *message, unsigned long message_size, int sender_id) {
    Popup popup;
    NetDaemonTransferModalCloseGuard guard(popup);
    Filemap filemap;
    // Pinned53444E/534456 resolves the transport ID once, before packet reads
    // and dispatch callbacks. Keep this identity distinct from packet faction.
    const int entry_sender_identity =
        this->AlphaNet::pid_2_who(static_cast<unsigned long>(sender_id));
    // The acknowledged-sync request uses lock_state_[2].trailing_words_[1]
    // at +0x1BBC; its local completion clears trailing_words_[0] at +0x1BB8.
    // The DONE UPKEEP handshake uses lock_state_[0].trailing_words_[8]
    // and lock_state_[1].leading_words_[1]; peer identities come from
    // AlphaNet::process_slots_. The UPKEEP INFO response updates the
    // existing GameDataStateStorage wait bit and faction masks.

    unsigned char *msg = reinterpret_cast<unsigned char *>(message);
    // Pinned534458..534466 captures these original packet words before
    // dispatch. Transport message_size is never the sequence acknowledgement.
    uint32_t entry_sequence;
    int entry_packet_faction;
    std::memcpy(&entry_sequence, msg + 0x0C, sizeof entry_sequence);
    std::memcpy(&entry_packet_faction, msg + 0x04, sizeof entry_packet_faction);
    unsigned short type = *reinterpret_cast<unsigned short *>(message);

    // Masked file-transfer response band; 0x1501 bypasses interception.
    // Its commlink comparison and reply target use the captured packet faction.
    bool skip500 = ((type & 0xf00) != 0x500) || (type == 0x1501);
    if (!skip500 && CommlinkPending != 0 && CommlinkFaction == entry_packet_faction) {
        skip500 = true;
    }
    if (!skip500) {
        if (type == 0x1505) {
            netdaemon_transfer_modal_cleanup(guard);
            return;
        }
        message_data(0x1505, entry_packet_faction, 0, 0, 0, 0);
    }

    type = *reinterpret_cast<unsigned short *>(message);

    // File-transfer opcodes are handled by the complete source-owned routes below.
    if (type >= 0xF05 && type <= 0xF0F) {
        if (process_file_transfer_message(message, message_size, sender_id,
                                          guard, filemap)) {
            return;
        }
    }

    if (netdaemon_handle_diplo_cd_message(type, msg)) {
        netdaemon_process_message_marker_tail(*this, msg);
        return;
    }

    if ((type >= 0x1201 && type <= 0x1208) ||
        type == 0x1240 || type == 0x4301 ||
        type == 0x4303 || type == 0x2308) {
        netdaemon_handle_server_lock_status(this, type, msg, entry_sender_identity);
        netdaemon_process_message_marker_tail(*this, msg);
        return;
    }

    if (type == 0x1101 || type == 0x4101) {
        netdaemon_process_sync_request(this, msg);
        netdaemon_process_message_marker_tail(*this, msg);
        return;
    }

    if (type == 0x2302) {
        netdaemon_handle_withdraw_end_turn(this, msg, entry_sender_identity);
        netdaemon_process_message_marker_tail(*this, msg);
        return;
    }
    if (type == 0x2303) {
        netdaemon_handle_client_done_upkeep(this, msg, entry_sender_identity);
        netdaemon_process_message_marker_tail(*this, msg);
        return;
    }
    if (type == 0x2301 || type == 0x8301) {
        netdaemon_handle_client_done_turn(this,
            entry_sender_identity);
        netdaemon_process_message_marker_tail(*this, msg);
        return;
    }
    if (type == 0x1303) {
        netdaemon_handle_server_upkeep_info(this, msg);
        netdaemon_process_message_marker_tail(*this, msg);
        return;
    }
    if (type == 0x1302) {
        netdaemon_handle_withdraw_approved(this, msg);
        netdaemon_process_message_marker_tail(*this, msg);
        return;
    }
    if (type == 0x1301) {
        netdaemon_handle_server_turn_info(this, msg);
        netdaemon_process_message_marker_tail(*this, msg);
        return;
    }

    if (netdaemon_handle_direct_diplomacy_complete(
            type, msg, popup, MultiWindowContactStatusMask)) {
        netdaemon_process_message_marker_tail(*this, msg);
        return;
    }

    if (type == 0x1F04 || type == 0x2F04 || type == 0x4F04) {
        netdaemon_handle_session_lobby(*this, msg, type, message_size);
        netdaemon_process_message_marker_tail(*this, msg);
        return;
    }

    if (type == 0x1F02 || type == 0x2F02 || type == 0x4F02) {
        netdaemon_handle_session_options(this, message, message_size);
        netdaemon_process_message_marker_tail(*this, msg);
        return;
    }
    if (type == 0x1F03 || type == 0x2F03 || type == 0x4F03) {
        netdaemon_handle_session_block(this, message, message_size);
        netdaemon_process_message_marker_tail(*this, msg);
        return;
    }
    if (type == 0x1F00 || type == 0x2F00 || type == 0x4F00) {
        netdaemon_handle_session_status(this, type, message, message_size);
        netdaemon_process_message_marker_tail(*this, msg);
        return;
    }

    if (type >= 0x2212 && type <= 0x2216) {
        if (netdaemon_process_table_e(*this, msg, type,
                entry_sender_identity)) {
            netdaemon_process_message_marker_tail(*this, msg);
            return;
        }
    }

    if (type == 0x2210 || type == 0x2211) {
        netdaemon_handle_client_lock_message(this, type, msg,
            entry_sender_identity);
        netdaemon_process_message_marker_tail(*this, msg);
        return;
    }

    if (type >= 0x4304 && type <= 0x4309) {
        if (netdaemon_handle_table_f(this, type, msg, sender_id)) {
            netdaemon_process_message_marker_tail(*this, msg);
            return;
        }
    }

    if (netdaemon_process_table_g(*this, msg, type)) {
        return;
    }

    if (netdaemon_handle_table_h(message)) {
        netdaemon_process_message_marker_tail(*this, msg);
        return;
    }

    if (netdaemon_handle_five_message_routes(
            *this, type, msg, entry_packet_faction, entry_sequence)) {
        netdaemon_process_message_marker_tail(*this, msg);
        return;
    }
    if (process_council_message(msg, type)) {
        netdaemon_process_message_marker_tail(*this, msg);
        return;
    }
    if (netdaemon_handle_lock_mask_residual(
            *this, type, msg,
            entry_sender_identity)) {
        netdaemon_process_message_marker_tail(*this, msg);
        return;
    }
    if (netdaemon_handle_server_change_pause(
            *this, type, sender_id,
            entry_sender_identity)) {
        netdaemon_process_message_marker_tail(*this, msg);
        return;
    }
    if (netdaemon_handle_time_control(
            *this, msg, type, sender_id,
            entry_sender_identity)) {
        netdaemon_process_message_marker_tail(*this, msg);
        return;
    }
    if (netdaemon_handle_chat_landmark_message(
            *this, msg, type, sender_id)) {
        netdaemon_process_message_marker_tail(*this, msg);
        return;
    }
    if (type == 0x4D00) {
        netdaemon_process_project_4d00(msg);
        netdaemon_process_message_marker_tail(*this, msg);
        return;
    }

    // Default path, reached by the overwhelming majority of message types
    // (the vast majority of jump-table slots point here in the original):
    // validate and re-broadcast the message.
    if ((msg[1] & 0xa0) != 0) {
        int fixResult = fixup_message(message);
        if (fixResult == 0) {
            unsigned short masked = (unsigned short)((type & 0x1fff) | 0x4000);
            *reinterpret_cast<unsigned short *>(message) = masked;
            const uint32_t seq = deletion_list_.advance_sequence();
            memcpy(message + 0x0C, &seq, sizeof seq);
            send_message(message, message_size, 0);
        }
    }

    netdaemon_process_message_marker_tail(*this, msg);
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

// ORIGINAL: 0x00530360 ?net_tasks@NetDaemon@@QAEXXZ 0x00530360-0x005303A9 BYTE_EXACT
// symbol    ?net_tasks@NetDaemon@@QAEXXZ
// size      74 bytes
// prototype void (__thiscall ?net_tasks@NetDaemon@@QAEXXZ)(NetDaemon* this)
// callers   2   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// One pump turn: drain a message, run the lock check, run spock.
void NetDaemon::net_tasks() {
  wait_task();
  unsigned long sender;
  unsigned long channel;
  void *result = NetDaemonNet.Net::get(&sender, &channel);
  if (result != 0) {
    process_message(static_cast<char *>(result), channel,
                    static_cast<int>(sender));
  }
  locks_.check_global();
  check_spock();
}

// ORIGINAL: 0x00531700 ?maybe_lock@NetDaemon@@QAEHPAHHHHHH@Z 0x00531700-0x00531780 BYTE_EXACT
// size      128 bytes
// prototype int (__thiscall ?maybe_lock@NetDaemon@@QAEHPAHHHHHH@Z)(NetDaemon* this, int*, int, int, int, int, int)
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// 0x1BAC is lock_state_[2].dword_[2] - the lock-held flag unlock() clears -
// 0x1BC4 lock_state_[2].dword_[8] and 0x1BCC lock_state_[2].dword_[10], which
// the archive never mapped. The repeat count compares SIGNED there (the
// image's `jle`, not `jbe`), and that plus naming the field is what took this
// off the archive's MISMATCH.
int NetDaemon::maybe_lock(int *veh_id, int flag, int payload_1C, int payload_20,
                          int payload_18, int extra) {
  if (flag != 0) {
    return 0;
  }
  if (channels_[1].leading_words_[2] != 0) {
    if (channels_[1].trailing_words_[3] == 0) {
      ++channels_[1].trailing_words_[5];
      // The image compares the count SIGNED here (`jle`), not unsigned.
      if (1 < static_cast<int>(channels_[1].trailing_words_[5])) {
        NetMsgPrimary.pop("MOVEPENDING", 0, 2, 0);
      }
    }
    return 1;
  }
  channels_[1].trailing_words_[5] = 0;
  return lock_veh(veh_id, payload_1C | 4, payload_20, payload_18, extra);
}

// Original0x00532140 performs the network half of a vehicle drop. The
// lock-state words are the existing typed entries documented above; the
// non-network action is a separate real owner in veh.cpp.
// ORIGINAL: 0x00532140 ?veh_drop@NetDaemon@@QAEHHHH@Z 0x00532140-0x00532563 FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/00532140.cpp retained signature/extents and complete seed; no historical claim copied", "Coordinator full pinned original instruction decode 00532140..00532563, typed source owner fields and corrected original call/branch evidence", "recovery/tests/test_airdrop_network.py actual-body directional treaty and network reentrancy controls; not linked/runtime/game acceptance"], "reason": "Recover the true network receiver, lock/ack state, global action-mode reentrancy and local execution result; preserve early return versus release paths and leave ExitTurnLoop read-only."}
// TRIED: flat byte-offset fields on `this` (class deliberately opaque); goto-based control flow matching the two shared cleanup labels (LAB_00532357 reject path, LAB_0053254e release path); fixed two Ghidra-vs-raw-asm discrepancies (speed()/array index use a1 not a2 after the reassignment at 0x53236F; message_data arg order is a1,a2,a3). Diverges at #0 (thiscall prologue).
int NetDaemon::veh_drop(int veh_id, int x, int y) {
    if (IsMultiplayerNet == 0) {
        return action_airdrop(veh_id, x, y, 3);
    }

    if (channels_[1].leading_words_[2] != 0) {
        if (channels_[1].trailing_words_[3] != 0) return 0;
        ++channels_[1].trailing_words_[5];
        if (static_cast<int>(channels_[1].trailing_words_[5]) < 2) return 0;
        NetMsgPrimary.pop("MOVEPENDING", 0, 2, nullptr);
        return 0;
    }

    channels_[1].trailing_words_[5] = 0;
    if (lock_veh(&veh_id, 4, x, y, 4) != 0) return 0;

    const int base_id = base_at(x, y);
    bool rejected = false;
    if (base_id >= 0) {
        const uint8_t vehicle_faction = Vehs[veh_id].faction_id;
        const uint8_t base_faction = Bases[base_id].faction_id_current;
        rejected = vehicle_faction == base_faction ||
            (PlayersData[vehicle_faction].diplo_treaties[base_faction] & 1U) != 0;
        if (!rejected) {
            const Map &tile = *map_loc(x, y);
            rejected = (tile.bit & 2U) != 0 && (tile.val2 & 0x0FU) < 8;
        }
        if (!rejected) rejected = add_global() == 0;

        if (!rejected) {
            if (channels_[1].trailing_words_[3] != 0) {
                Vehs[veh_id].order = 0;
                Vehs[veh_id].state &= 0xFCFFBDFFU;
                synch_veh(veh_id);
                if (IsMultiplayerNet != 0) {
                    uint32_t *acknowledged = &channels_[1].trailing_words_[0];
                    *acknowledged = 1;
                    log_say("Awaiting acknowledgement of SYNCH",
                            ConsoleGlobal.faction_id_, 0, 0);
                    const int waited = wait("AWAITEXEC", acknowledged, 1,
                                            2000);
                    NetDaemonWaitTicks += waited;
                    if (*acknowledged == 0) {
                        log_say("Received SYNCH acknowledgement",
                                ConsoleGlobal.faction_id_, 0, 0);
                    }
                    *acknowledged = 0;
                }
            }
            goto release_vehicle;
        }
    }

    if (action_airdrop(veh_id, x, y, 1) != 0) {
        const int current_speed = speed(veh_id, 0);
        const int previous_speed = Vehs[veh_id].moves_expended;
        const int difference = current_speed - previous_speed;
        if (difference >= 0 &&
            (difference > 999 || current_speed != previous_speed)) {
            message_data(0x2403, 0, veh_id, x, y, 0);
            if (IsMultiplayerNet != 0) {
                log_say("Client waiting for acknowledgement",
                        ConsoleGlobal.faction_id_, 0, 0);
                ++NetBusyCount;
                const int saved_action_mode = NetDaemonNet.channels_[0].leading_words_[3];
                const DWORD start_time = timeGetTime();
                bool showed_wait_popup = false;
                if (channels_[1].leading_words_[3] == 1) {
                    do {
                        if (ExitTurnLoop != 0) break;
                        if (!showed_wait_popup &&
                            timeGetTime() - start_time > 2000) {
                            showed_wait_popup = true;
                            NetMsgPrimary.pop("AWAITEXEC", 0, 2, nullptr);
                        }
                        wait_task();
                        receive();
                        locks_.check_global();
                        check_spock();
                    } while (channels_[1].leading_words_[3] == 1);
                    if (showed_wait_popup) NetMsgPrimary.close();
                }
                --NetBusyCount;
                NetDaemonNet.channels_[0].leading_words_[3] = saved_action_mode;
                log_say("Client received acknowledgement",
                        ConsoleGlobal.faction_id_, 0, 0);
                if (IsMultiplayerNet != 0) {
                    log_say("Client releasing lock",
                            ConsoleGlobal.faction_id_, 0, 0);
                    message_data(0x2212, 0, 0, 0, 0, 0);
                    channels_[1].leading_words_[3] = 0;
                    channels_[1].leading_words_[2] = 0;
                    channels_[1].trailing_words_[5] = 0;
                    channels_[1].trailing_words_[4] = 0;
                }
                channels_[1].leading_words_[2] = 0;
                channels_[1].trailing_words_[3] = 0;
                channels_[1].leading_words_[3] = 0;
                return 1;
            }
            return 1;
        }
    }

release_vehicle:
    if (IsMultiplayerNet != 0) {
        log_say("Client releasing lock", ConsoleGlobal.faction_id_, 0, 0);
        message_data(0x2212, 0, 0, 0, 0, 0);
        channels_[1].leading_words_[3] = 0;
        channels_[1].leading_words_[2] = 0;
        channels_[1].trailing_words_[5] = 0;
        channels_[1].trailing_words_[4] = 0;
    }
    channels_[0].payload_.vehicle_selection_follow = 0;
    channels_[1].trailing_words_[3] = 0;
    return 0;
}


// ORIGINAL: 0x00532570 ?action@NetDaemon@@QAEHHH@Z 0x00532570-0x005328BA
// RECOVERY_BINDING: v1 {"evidence": ["Pinned532570..5328BA; retained git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/00532570.cpp is a conflicting historical hint, not the product ABI.", "Entry signed order classification is distinct from the post-automated_lock vehicle/order reload. Absolute93E904 timing storage belongs to NetDaemonNet.lock_state_[1].dword_[3], while wait flags and Lock remain receiver-relative.", "Pinned NUL log/popup literals68C2DC,68C390,68C3B4,68C3C0,68C4AC,68C4C8,68C4F4,68C510,68C530,68C544,68C554; ExitTurnLoop is game owner9B2068."], "reason": "Recover the full multiplayer vehicle-order action through the real NetDaemon, vehicle and transport owners; retain its original seed identity and complete span without a matching or runtime claim."}
// TRIED: g_00669368 is a cached stdcall function pointer (timeGetTime), called twice through the same loaded pointer - matches the two `// indirect` sites.
// TRIED: nothing else tried; landed at MISMATCH #7 (original compares several fields against a zero register cached once via `xor ebx,ebx` and reused, rebuilt re-derives `== 0` per comparison; similarity 0.886).
int NetDaemon::action(int veh_id, int mode) {
    if (IsMultiplayerNet == 0) return ::action(veh_id);

    const int action_type = Vehs[veh_id].order;
    int category = 0;
    enum Branch { await_exec, go_to, road_to };
    Branch branch = await_exec;

    if (action_type < 4 || action_type > 0x17) {
        if (action_type == ORDER_MOVE_TO || action_type == ORDER_AI_MOVE_TO) {
            category = 4;
            branch = go_to;
        } else if (action_type == ORDER_ROAD_TO || action_type == ORDER_MAGTUBE_TO) {
            category = 4;
            branch = road_to;
        } else {
            if (action_type != 0)
                log_say("Falling out of action loop", veh_id, action_type, 0);
            if (mode != 0) {
                if (IsMultiplayerNet != 0) {
                    log_say("Client releasing lock", ConsoleGlobal.faction_id_, 0, 0);
                    message_data(0x2212, 0, 0, 0, 0, 0);
                    channels_[1].leading_words_[3] = 0;
                    channels_[1].leading_words_[2] = 0;
                    channels_[1].trailing_words_[5] = 0;
                    channels_[1].trailing_words_[4] = 0;
                }
                channels_[0].payload_.vehicle_selection_follow = 0;
                channels_[1].trailing_words_[3] = 0;
            }
            return 0;
        }
    } else {
        category = (action_type == 0x14 || action_type == 0x15) ? 0xC : 0;
    }

    channels_[1].trailing_words_[3] = 1;
    if (mode == 0 && automated_lock(&veh_id, category) != 0) return 0;

    if (branch == await_exec) {
        log_say("Net.Action Sending ACTION message to server", veh_id,
                Vehs[veh_id].order, 0);
        message_veh(0x2404, veh_id, 0, 0);
        log_say("Net.Action Awaiting EXEC", 0, 0, 0);
        if (IsMultiplayerNet != 0) {
            log_say("Client waiting for acknowledgement",
                    ConsoleGlobal.faction_id_, 0, 0);
            const uint32_t saved_action_mode =
                NetDaemonNet.channels_[0].leading_words_[3];
            DWORD (WINAPI *const get_time)() = timeGetTime;
            ++NetBusyCount;
            const DWORD start_time = get_time();
            bool showed_wait_popup = false;
            if (channels_[1].leading_words_[3] == 1) {
                do {
                    if (ExitTurnLoop != 0) break;
                    if (!showed_wait_popup && get_time() - start_time > 2000) {
                        showed_wait_popup = true;
                        NetMsgPrimary.pop("AWAITEXEC", 0, 2, nullptr);
                    }
                    wait_task();
                    unsigned long first, second;
                    void *message = NetDaemonNet.Net::get(&first, &second);
                    if (message != 0)
                        process_message(static_cast<char *>(message),
                                        second, static_cast<int>(first));
                    locks_.check_global();
                    check_spock();
                } while (channels_[1].leading_words_[3] == 1);
                if (showed_wait_popup) NetMsgPrimary.close();
            }
            --NetBusyCount;
            NetDaemonNet.channels_[0].leading_words_[3] = saved_action_mode;
            log_say("Client received acknowledgement", ConsoleGlobal.faction_id_, 0, 0);
            if (IsMultiplayerNet != 0) {
                log_say("Client releasing lock", ConsoleGlobal.faction_id_, 0, 0);
                message_data(0x2212, 0, 0, 0, 0, 0);
                channels_[1].leading_words_[3] = 0;
                channels_[1].leading_words_[2] = 0;
                channels_[1].trailing_words_[5] = 0;
                channels_[1].trailing_words_[4] = 0;
            }
            channels_[0].payload_.vehicle_selection_follow = 0;
            channels_[1].trailing_words_[3] = 0;
            channels_[1].leading_words_[3] = 0;
        }
        log_say("Net.Action Finished awaiting", 0, 0, 0);
    } else if (branch == go_to) {
        log_say("Net.Action Go to", veh_id, 0, 0);
        action_go_to(veh_id);
    } else {
        log_say("Net.Action Road", 0, 0, 0);
        action_road_to(veh_id);
    }
    log_say("Falling out of Net.Action", 0, 0, 0);
    return 1;
}

// ORIGINAL: 0x00531780 ?order_veh@NetDaemon@@QAEHHHH@Z 0x00531780-0x00532135
// RECOVERY_BINDING: v1 {"evidence": ["Pinned531780..532135; retained git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/00531780.cpp preserves the original identity and contradictory seed history.", "Native5318D6..5318E8 computes(otherFaction+2099*ownFaction)*4+96C9F8: PlayersData[ownFaction].diplo_treaties[otherFaction], stride20CC, not the seed20C8 transcription.", "Veh34/Base134/Map2C fields and existing coordinate arrays own the accessed state; signed speed and lock cleanup/callback ordering retain native obligations. Literal68C480 is GLOBALFAILED and68C490 is Sending MOVE VEH message; native synchronization and the full message dispatcher remain unfinished."], "reason": "Recover the complete network move request, feasibility, acknowledgement and release paths on the genuine daemon and game records. Preserve unresolved synch/process_message dependencies rather than forwarding this body to the original executable."}
// TRIED: Ghidra pseudocode used as primary source with call-signature/arg-order fixes confirmed against raw disasm (base_at, veh_at, stack_check, stack_fix, order_veh [both the free function at 0x5947C0 and the recursive-looking unqualified call, which needed `::order_veh` to avoid binding to the member of the same name], message_veh, Net::get/process_message argument mapping). Factored the 3x near-identical "release lock" block into a file-scope #define/#undef macro rather than a helper function. One table access (DAT_0096c9f8 with a stride of 0x20c8 rather than the 0x20cc seen everywhere else in this codebase) is transcribed as Ghidra states it but NOT independently re-derived from raw disasm - flagging this as the weakest point if it mismatches. Compiles; first divergence at instr #34. Not chased to byte-exact given size (2485 bytes).
int NetDaemon::order_veh(int veh_id, int direction, int mode) {
    if (IsMultiplayerNet == 0) return ::order_veh(veh_id, direction, 3);

    if (mode == 0) {
        if (channels_[1].leading_words_[2] != 0) {
            if (channels_[1].trailing_words_[3] != 0) return 0;
            ++channels_[1].trailing_words_[5];
            if (static_cast<int>(channels_[1].trailing_words_[5]) < 2) return 0;
            NetMsgPrimary.pop("MOVEPENDING", 0, 2, nullptr);
            return 0;
        }
        channels_[1].trailing_words_[5] = 0;
        if (lock_veh(&veh_id, 4, -1, -1, 0) != 0) return 0;
    }

    int x = Vehs[veh_id].x + VehMoveDeltaX[direction];
    if ((MapIsFlat & 1) == 0) {
        if (x < 0) x += MapLongitudeBounds;
        else if (x >= MapLongitudeBounds) x -= MapLongitudeBounds;
    }
    const int faction = Vehs[veh_id].faction_id;
    const int y = Vehs[veh_id].y + VehMoveDeltaY[direction];
    const int base_id = base_at(x, y);
    bool good_base = false;
    Vehs[veh_id].probe_action = 0;

    if (base_id >= 0) {
        const int base_faction = Bases[base_id].faction_id_current;
        if (base_faction != faction &&
            (PlayersData[faction].diplo_treaties[base_faction] & 1U) == 0) {
            const Map &tile = MapTiles[MapLongitude * y + (x >> 1)];
            if ((tile.bit & 2U) == 0 || (tile.val2 & 0x0F) > 7 ||
                Bases[base_id].population_size < 2) {
                good_base = true;
            }
        }
    }

    bool do_order;
    if (planet_buster(veh_id) == 0) {
        do_order = good_base;
    } else {
        const int fixed_vehicle = stack_fix(veh_at(x, y));
        if (fixed_vehicle < 0 || Vehs[fixed_vehicle].faction_id == faction ||
            (PlayersData[faction]
                 .diplo_treaties[Vehs[fixed_vehicle].faction_id] & 1U) != 0) {
            do_order = good_base;
        } else {
            do_order = true;
        }
    }

    if (do_order && add_global() != 0) goto release_lock;

    if (::order_veh(veh_id, direction, 1) == 0) goto release_lock_only;

    {
        const int difference = speed(veh_id, 0) - Vehs[veh_id].moves_expended;
        if (difference < 0 ||
            (difference < 1000 && difference == 0))
            goto release_lock_only;
    }

    if (VehPrototypes[Vehs[veh_id].proto_id].plan == PLAN_INFO_WARFARE &&
        base_id >= 0 && faction != Bases[base_id].faction_id_current) {
        const int stack_vehicle = veh_at(x, y);
        if (stack_check(stack_vehicle, 2, PLAN_INFO_WARFARE, -1, -1) == 0 &&
            !good_base) {
            if (add_global() == 0) goto send_move;
            goto global_failed;
        }
    }

    if (base_id < 0 || Bases[base_id].population_size > 1 ||
        (Vehs[veh_id].state & 0x80000U) == 0 || good_base || add_global() == 0) {
send_move:
        log_say("Sending MOVE VEH message", veh_id, direction, 0);
        message_veh(0x2401, veh_id, direction, 0);
        if (mode == 0 && IsMultiplayerNet != 0) {
            log_say("Client waiting for acknowledgement", ConsoleGlobal.faction_id_, 0, 0);
            uint32_t *flag = &channels_[1].leading_words_[3];
            const int waited = wait("AWAITEXEC", flag, 1, 2000);
            NetDaemonWaitTicks += waited;
            log_say("Client received acknowledgement", ConsoleGlobal.faction_id_, 0, 0);
            if (IsMultiplayerNet != 0) {
                log_say("Client releasing lock", ConsoleGlobal.faction_id_, 0, 0);
                message_data(0x2212, 0, 0, 0, 0, 0);
                *flag = 0;
                channels_[1].leading_words_[2] = 0;
                channels_[1].trailing_words_[5] = 0;
                channels_[1].trailing_words_[4] = 0;
            }
            channels_[0].payload_.vehicle_selection_follow = 0;
            channels_[1].trailing_words_[3] = 0;
            *flag = 0;
        }

        bool have_other_vehicle = false;
        int other_faction_found = 0;
        for (int byte_offset = 0; byte_offset <= 0x53; byte_offset += 4) {
            int adjacent_x = RadiusOffsetX[byte_offset / 4] + x;
            if ((MapIsFlat & 1) == 0) {
                if (adjacent_x < 0) adjacent_x += MapLongitudeBounds;
                else if (adjacent_x >= MapLongitudeBounds) adjacent_x -= MapLongitudeBounds;
            }
            const int adjacent_y = RadiusOffsetY[byte_offset / 4] + y;
            if (adjacent_y >= 0 && adjacent_y < MapLatitudeBounds &&
                adjacent_x >= 0 && adjacent_x < MapLongitudeBounds) {
                const Map &tile = MapTiles[MapLongitude * adjacent_y + (adjacent_x >> 1)];
                if ((tile.bit & 3U) != 0) {
                    const int other_faction = tile.val2 & 0x0F;
                    if (other_faction < 8 && other_faction != ConsoleGlobal.faction_id_ &&
                        (PlayersData[other_faction]
                             .diplo_treaties[ConsoleGlobal.faction_id_] & 0x10U) != 0 &&
                        (GameDataStateStorage.fields_.lock_masks_.enable_mask_ & (1U << other_faction)) != 0) {
                        int other_vehicle = veh_at(adjacent_x, adjacent_y);
                        while (other_vehicle >= 0) {
                            if (Vehs[other_vehicle].faction_id == other_faction) {
                                const int remaining = speed(other_vehicle, 0) -
                                    Vehs[other_vehicle].moves_expended;
                                if (remaining >= 0 &&
                                    (remaining > 999 || remaining != 0)) {
                                    have_other_vehicle = true;
                                    other_faction_found = other_faction;
                                }
                            }
                            other_vehicle = Vehs[other_vehicle].prev_veh_id_stack;
                        }
                    }
                }
            }
        }
        (void)other_faction_found;

        if (have_other_vehicle) {
            DWORD (WINAPI *const get_time)() = timeGetTime;
            const DWORD start = get_time();
            do {
                do_all_non_input();
                unsigned long first, second;
                void *message = NetDaemonNet.Net::get(&first, &second);
                if (message != 0)
                    process_message(static_cast<char *>(message), second,
                                    static_cast<int>(first));
                locks_.check_global();
                if (ExitTurnLoop != 0) break;
            } while (get_time() - start < 500);
        }
        return 1;
    }

release_lock:
    if (channels_[1].trailing_words_[3] != 0) {
        Vehs[veh_id].order = 0;
        Vehs[veh_id].state &= 0xFCFFBDFFU;
        synch_veh(veh_id);
        if (IsMultiplayerNet != 0) {
            uint32_t *flag = &channels_[1].trailing_words_[0];
            *flag = 1;
            log_say("Awaiting acknowledgement of SYNCH", ConsoleGlobal.faction_id_, 0, 0);
            const int waited = wait("AWAITEXEC", flag, 1, 2000);
            NetDaemonWaitTicks += waited;
            if (*flag == 0)
                log_say("Received SYNCH acknowledgement", ConsoleGlobal.faction_id_, 0, 0);
            *flag = 0;
        }
    }
release_lock_only:
    // Native 0x005320CF is the short failure tail used by a rejected order
    // or exhausted movement. It does not clear/synchronize the vehicle.
    if (IsMultiplayerNet != 0) {
        log_say("Client releasing lock", ConsoleGlobal.faction_id_, 0, 0);
        message_data(0x2212, 0, 0, 0, 0, 0);
        channels_[1].leading_words_[3] = 0;
        channels_[1].leading_words_[2] = 0;
        channels_[1].trailing_words_[5] = 0;
        channels_[1].trailing_words_[4] = 0;
    }
    channels_[0].payload_.vehicle_selection_follow = 0;
    channels_[1].trailing_words_[3] = 0;
    return 0;

global_failed:
    // Kept separate because the seed's add_global failure path pops before
    // returning; it does not perform the generic release helper above.
    if (channels_[1].trailing_words_[3] != 0) {
        Vehs[veh_id].order = 0;
        Vehs[veh_id].state &= 0xFCFFBDFFU;
        synch_veh(veh_id);
        if (IsMultiplayerNet != 0) {
            uint32_t *flag = &channels_[1].trailing_words_[0];
            *flag = 1;
            log_say("Awaiting acknowledgement of SYNCH", ConsoleGlobal.faction_id_, 0, 0);
            const int waited = wait("AWAITEXEC", flag, 1, 2000);
            NetDaemonWaitTicks += waited;
            if (*flag == 0)
                log_say("Received SYNCH acknowledgement", ConsoleGlobal.faction_id_, 0, 0);
            *flag = 0;
        }
    }
    if (IsMultiplayerNet != 0) {
        log_say("Client releasing lock", ConsoleGlobal.faction_id_, 0, 0);
        message_data(0x2212, 0, 0, 0, 0, 0);
        channels_[1].leading_words_[3] = 0;
        channels_[1].leading_words_[2] = 0;
        channels_[1].trailing_words_[5] = 0;
        channels_[1].trailing_words_[4] = 0;
    }
    channels_[0].payload_.vehicle_selection_follow = 0;
    channels_[1].trailing_words_[3] = 0;
    NetMsgPrimary.pop("GLOBALFAILED", 5000, 0, nullptr);
    return 0;
}

// Observed network path window at 0x0093AC88, shared with send_files.
// Complete allocation extent and producer migration remain unproved.
// RECOVERY_OBJECT: v1 0x0093AC88 ?NetDaemonFileTransferPath@@3PADA
char NetDaemonFileTransferPath[0x104];

namespace NetDaemonSyncWire {

template <class T>
static size_t copy_member_bytes(size_t chunk_offset, uint8_t *chunk,
                                size_t chunk_size, size_t wire_offset,
                                T &member, bool packet_to_owner) {
    const size_t chunk_end = chunk_offset + chunk_size;
    const size_t member_end = wire_offset + sizeof member;
    const size_t first = chunk_offset > wire_offset
        ? chunk_offset : wire_offset;
    const size_t last = chunk_end < member_end ? chunk_end : member_end;
    if (first >= last) return 0;

    unsigned char *member_bytes =
        reinterpret_cast<unsigned char *>(&member);
    const size_t member_delta = first - wire_offset;
    const size_t chunk_delta = first - chunk_offset;
    const size_t bytes = last - first;
    if (packet_to_owner) {
        memcpy(member_bytes + member_delta, chunk + chunk_delta, bytes);
    } else {
        memcpy(chunk + chunk_delta, member_bytes + member_delta, bytes);
    }
    return bytes;
}

static bool wire0_range_is_owned(size_t first, size_t count) {
    struct Span { size_t first; size_t size; };
    const Span spans[] = {
        {0x000, sizeof GameDataStateStorage.fields_.game_preferences_}, {0x004, sizeof GameDataStateStorage.fields_.game_more_preferences_},
        {0x008, sizeof GameDataStateStorage.fields_.game_warnings_}, {0x00C, sizeof GameDataStateStorage.fields_.game_rules_},
        {0x010, sizeof GameDataStateStorage.fields_.game_wire_opaque_9a64a0_},
        {0x014, sizeof GameDataStateStorage.fields_.game_interlude_flags_},
        {0x018, sizeof GameDataStateStorage.fields_.game_wire_opaque_9a64a8_},
        {0x01C, sizeof GameDataStateStorage.fields_.tutorial_flags_}, {0x020, sizeof GameDataStateStorage.fields_.tutorial_flags_2_},
        {0x024, sizeof GameDataStateStorage.fields_.terraform_announced_mask_},
        {0x02C, sizeof GameDataStateStorage.fields_.tutwin_flags_9a64bc_}, {0x030, sizeof GameDataStateStorage.fields_.game_state_},
        {0x034, sizeof GameDataStateStorage.fields_.diff_level_current_}, {0x038, sizeof GameDataStateStorage.fields_.vehicle_current_count_},
        {0x03C, sizeof GameDataStateStorage.fields_.base_current_count_}, {0x040, sizeof GameDataStateStorage.fields_.game_setup_reset_word_},
        {0x044, sizeof GameDataStateStorage.fields_.turn_current_num_}, {0x048, sizeof GameDataStateStorage.fields_.mission_year_current_},
        {0x04C, sizeof GameDataStateStorage.fields_.vehicle_selection_follow_limit_},
        {0x050, sizeof GameDataStateStorage.fields_.time_controls_selection_},
        {0x054, sizeof GameDataStateStorage.fields_.turn_control_duration_milliseconds_},
        {0x058, sizeof GameDataStateStorage.fields_.lock_masks_.enable_mask_}, {0x05C, sizeof GameDataStateStorage.fields_.faction_rankings_},
        {0x07C, sizeof GameDataStateStorage.fields_.ranking_faction_id_unk1_},
        {0x080, sizeof GameDataStateStorage.fields_.ranking_faction_id_unk2_}, {0x084, sizeof GameDataStateStorage.fields_.secret_project_},
        {0x184, sizeof GameDataStateStorage.fields_.governor_faction_},
        {0x188, sizeof GameDataStateStorage.fields_.council_resolution_count_},
        {0x1B4, sizeof GameDataStateStorage.fields_.council_last_resolution_turn_},
        {0x1E0, sizeof GameDataStateStorage.fields_.game_tech_achieved_},
        {0x239, sizeof GameDataStateStorage.fields_.legacy_game_save_name_},
        {0x339, sizeof GameDataStateStorage.fields_.game_wire_opaque_9a67c9_},
        {0x33C, sizeof GameDataStateStorage.fields_.climate_rise_step_}, {0x340, sizeof GameDataStateStorage.fields_.climate_rise_limit_},
        {0x344, sizeof GameDataStateStorage.fields_.climate_rise_accum_},
        {0x348, sizeof GameDataStateStorage.fields_.climate_sea_level_delta_},
        {0x34C, sizeof GameDataStateStorage.fields_.transcendence_score_bonus_},
        {0x350, sizeof GameDataStateStorage.fields_.transcendence_score_weights_},
        {0x370, sizeof GameDataStateStorage.fields_.sunspot_duration_}, {0x374, sizeof GameDataStateStorage.fields_.mount_planet_x_},
        {0x378, sizeof GameDataStateStorage.fields_.mount_planet_y_}, {0x37C, sizeof GameDataStateStorage.fields_.dust_cloud_duration_},
        {0x380, sizeof GameDataStateStorage.fields_.save_daemon_state_word_},
        {0x384, sizeof GameDataStateStorage.fields_.scenario_victory_facility_},
        {0x388, sizeof GameDataStateStorage.fields_.global_energy_event_state_},
        {0x38C, sizeof GameDataStateStorage.fields_.net_turn_flags_}, {0x390, sizeof GameDataStateStorage.fields_.net_turn_faction_},
        {0x394, sizeof GameDataStateStorage.fields_.ecology_event_accumulator_}
    };
    const size_t end = first + count;
    size_t covered = first;
    size_t i;
    for (i = 0; i < sizeof spans / sizeof spans[0] && covered < end; ++i) {
        const size_t span_end = spans[i].first + spans[i].size;
        if (spans[i].first <= covered && span_end > covered)
            covered = span_end < end ? span_end : end;
    }
    return covered == end;
}

// Implement in netdaemon.cpp after the listed owner declarations are exposed.
// chunk_offset/count must be bounded to [0,0x398) by the packet layer.
bool opcode0_copy_chunk(size_t chunk_offset, uint8_t *chunk, size_t count,
                        bool packet_to_owner) {
    if (chunk_offset > 0x398 || count > 0x398 - chunk_offset) return false;
    if (!wire0_range_is_owned(chunk_offset, count)) return false;
    size_t copied = 0;
#define COPY0(offset, object) copied += copy_member_bytes(chunk_offset, chunk, count, offset, object, packet_to_owner)

    // game.cpp/game.h objects. The four-byte and three-byte opaque objects
    // below are retained independent arrays, not padding or a packed block.
    COPY0(0x000, GameDataStateStorage.fields_.game_preferences_);
    COPY0(0x004, GameDataStateStorage.fields_.game_more_preferences_);
    COPY0(0x008, GameDataStateStorage.fields_.game_warnings_);
    COPY0(0x00C, GameDataStateStorage.fields_.game_rules_);
    COPY0(0x010, GameDataStateStorage.fields_.game_wire_opaque_9a64a0_);
    COPY0(0x014, GameDataStateStorage.fields_.game_interlude_flags_);
    COPY0(0x018, GameDataStateStorage.fields_.game_wire_opaque_9a64a8_);
    COPY0(0x01C, GameDataStateStorage.fields_.tutorial_flags_);
    COPY0(0x020, GameDataStateStorage.fields_.tutorial_flags_2_);

    // veh.cpp/terraforming.h owns this two-word announced mask.
    COPY0(0x024, GameDataStateStorage.fields_.terraform_announced_mask_);
    COPY0(0x02C, GameDataStateStorage.fields_.tutwin_flags_9a64bc_);

    COPY0(0x030, GameDataStateStorage.fields_.game_state_);
    COPY0(0x034, GameDataStateStorage.fields_.diff_level_current_);
    COPY0(0x038, GameDataStateStorage.fields_.vehicle_current_count_);
    COPY0(0x03C, GameDataStateStorage.fields_.base_current_count_);
    COPY0(0x040, GameDataStateStorage.fields_.game_setup_reset_word_);
    COPY0(0x044, GameDataStateStorage.fields_.turn_current_num_);
    COPY0(0x048, GameDataStateStorage.fields_.mission_year_current_);
    COPY0(0x04C, GameDataStateStorage.fields_.vehicle_selection_follow_limit_);
    COPY0(0x050, GameDataStateStorage.fields_.time_controls_selection_);
    COPY0(0x054, GameDataStateStorage.fields_.turn_control_duration_milliseconds_);
    COPY0(0x058, GameDataStateStorage.fields_.lock_masks_.enable_mask_);

    // Canonical current owners: eight FactionRankings entries, followed by
    // the two other ranking words, then SecretProject exactly once.
    COPY0(0x05C, GameDataStateStorage.fields_.faction_rankings_);
    COPY0(0x07C, GameDataStateStorage.fields_.ranking_faction_id_unk1_);
    COPY0(0x080, GameDataStateStorage.fields_.ranking_faction_id_unk2_);
    COPY0(0x084, GameDataStateStorage.fields_.secret_project_);

    COPY0(0x184, GameDataStateStorage.fields_.governor_faction_);
    COPY0(0x188, GameDataStateStorage.fields_.council_resolution_count_);
    COPY0(0x1B4, GameDataStateStorage.fields_.council_last_resolution_turn_);
    COPY0(0x1E0, GameDataStateStorage.fields_.game_tech_achieved_);
    COPY0(0x239, GameDataStateStorage.fields_.legacy_game_save_name_);
    COPY0(0x339, GameDataStateStorage.fields_.game_wire_opaque_9a67c9_);

    COPY0(0x33C, GameDataStateStorage.fields_.climate_rise_step_);
    COPY0(0x340, GameDataStateStorage.fields_.climate_rise_limit_);
    COPY0(0x344, GameDataStateStorage.fields_.climate_rise_accum_);
    COPY0(0x348, GameDataStateStorage.fields_.climate_sea_level_delta_);
    COPY0(0x34C, GameDataStateStorage.fields_.transcendence_score_bonus_);
    COPY0(0x350, GameDataStateStorage.fields_.transcendence_score_weights_);
    COPY0(0x370, GameDataStateStorage.fields_.sunspot_duration_);
    COPY0(0x374, GameDataStateStorage.fields_.mount_planet_x_);
    COPY0(0x378, GameDataStateStorage.fields_.mount_planet_y_);
    COPY0(0x37C, GameDataStateStorage.fields_.dust_cloud_duration_);
    COPY0(0x380, GameDataStateStorage.fields_.save_daemon_state_word_);
    COPY0(0x384, GameDataStateStorage.fields_.scenario_victory_facility_);
    COPY0(0x388, GameDataStateStorage.fields_.global_energy_event_state_);
    COPY0(0x38C, GameDataStateStorage.fields_.net_turn_flags_);
    COPY0(0x390, GameDataStateStorage.fields_.net_turn_faction_);
    COPY0(0x394, GameDataStateStorage.fields_.ecology_event_accumulator_);

#undef COPY0
    // A valid native request is fully covered by the independently owned
    // fields below. Refuse any uncovered byte rather than sending allocator
    // residue or claiming a partial incoming scatter succeeded.
    return copied == count;
}

} // namespace NetDaemonSyncWire

namespace NetDaemonSyncWire {

static size_t overlap_copy(size_t request_offset, unsigned char *packet,
                           size_t request_size, size_t owner_offset,
                           unsigned char *owner_bytes, size_t owner_size,
                           bool packet_to_owner) {
    const size_t first = request_offset > owner_offset
        ? request_offset : owner_offset;
    const size_t request_end = request_offset + request_size;
    const size_t owner_end = owner_offset + owner_size;
    const size_t last = request_end < owner_end ? request_end : owner_end;
    if (first >= last) return 0;
    const size_t length = last - first;
    if (packet_to_owner)
        memcpy(owner_bytes + first - owner_offset,
               packet + first - request_offset, length);
    else
        memcpy(packet + first - request_offset,
               owner_bytes + first - owner_offset, length);
    return length;
}

template <class T>
static size_t member_copy(size_t request_offset, unsigned char *packet,
                          size_t request_size, size_t owner_offset,
                          T &owner, bool packet_to_owner) {
    return overlap_copy(request_offset, packet, request_size, owner_offset,
        reinterpret_cast<unsigned char *>(&owner), sizeof owner,
        packet_to_owner);
}

template <class T>
static size_t bounded_array_copy(size_t request_offset,
                                 unsigned char *packet,
                                 size_t request_size, T &owner,
                                 size_t extent, bool packet_to_owner) {
    if (extent > sizeof owner) return 0;
    return overlap_copy(request_offset, packet, request_size, 0,
        reinterpret_cast<unsigned char *>(&owner), extent, packet_to_owner);
}

// The saved diplomacy words are one native 0x48-byte local tuple. Pair IDs
// are captured by NetDaemon::synch before callbacks and retained through the
// epilogue; they must not be reconstructed from a later callback-modified
// packet header.
void gather_saved_diplomacy(int faction_a, int faction_b, uint32_t wire[18]);
void scatter_saved_diplomacy(int faction_a, int faction_b,
                             const uint32_t wire[18]);
bool opcode0_copy_chunk(size_t offset, unsigned char *packet, size_t count,
                        bool packet_to_owner);
template <class T>
static uint32_t native_word(const T &value) {
    typedef char NativeWordMustBeFourBytes[
        sizeof(T) == sizeof(uint32_t) ? 1 : -1];
    (void)sizeof(NativeWordMustBeFourBytes);
    uint32_t bits;
    memcpy(&bits, &value, sizeof bits);
    return bits;
}

static uint32_t sign_extended_char(char value) {
    const uint32_t byte = static_cast<unsigned char>(value);
    return (byte & 0x80u) ? (byte | 0xFFFFFF00u) : byte;
}

void gather_saved_diplomacy(int faction_a, int faction_b, uint32_t wire[18]) {
    const PlayerData &a = PlayersData[faction_a];
    const PlayerData &b = PlayersData[faction_b];
    wire[0] = sign_extended_char(a.diplo_patience[faction_b]);
    wire[1] = sign_extended_char(b.diplo_patience[faction_a]);
    wire[2] = native_word(a.diplo_friction[faction_b]);
    wire[3] = native_word(b.diplo_friction[faction_a]);
    wire[4] = native_word(a.global_reputation);
    wire[5] = native_word(b.global_reputation);
    wire[6] = native_word(a.diplo_unk1[faction_b]);
    wire[7] = native_word(b.diplo_unk1[faction_a]);
    wire[8] = native_word(a.diplo_wrongs[faction_b]);
    wire[9] = native_word(b.diplo_wrongs[faction_a]);
    wire[10] = native_word(a.diplo_spoke[faction_b]);
    wire[11] = native_word(b.diplo_spoke[faction_a]);
    wire[12] = a.diplo_treaties[faction_b];
    wire[13] = b.diplo_treaties[faction_a];
    wire[14] = native_word(a.diplo_agenda[faction_b]);
    wire[15] = native_word(b.diplo_agenda[faction_a]);
    wire[16] = native_word(a.diplo_merc[faction_b]);
    wire[17] = native_word(b.diplo_merc[faction_a]);
}

template <class T>
static void scatter_word(T &field, uint32_t bits) {
    typedef char NativeWordMustBeFourBytes[
        sizeof(T) == sizeof(uint32_t) ? 1 : -1];
    (void)sizeof(NativeWordMustBeFourBytes);
    memcpy(&field, &bits, sizeof bits);
}

void scatter_saved_diplomacy(int faction_a, int faction_b,
                             const uint32_t wire[18]) {
    PlayerData &a = PlayersData[faction_a];
    PlayerData &b = PlayersData[faction_b];
    memcpy(&a.diplo_patience[faction_b], &wire[0], 1);
    memcpy(&b.diplo_patience[faction_a], &wire[1], 1);
    scatter_word(a.diplo_friction[faction_b], wire[2]);
    scatter_word(b.diplo_friction[faction_a], wire[3]);
    scatter_word(a.global_reputation, wire[4]);
    scatter_word(b.global_reputation, wire[5]);
    scatter_word(a.diplo_unk1[faction_b], wire[6]);
    scatter_word(b.diplo_unk1[faction_a], wire[7]);
    scatter_word(a.diplo_wrongs[faction_b], wire[8]);
    scatter_word(b.diplo_wrongs[faction_a], wire[9]);
    scatter_word(a.diplo_spoke[faction_b], wire[10]);
    scatter_word(b.diplo_spoke[faction_a], wire[11]);
    memcpy(&a.diplo_treaties[faction_b], &wire[12], sizeof wire[12]);
    memcpy(&b.diplo_treaties[faction_a], &wire[13], sizeof wire[13]);
    scatter_word(a.diplo_agenda[faction_b], wire[14]);
    scatter_word(b.diplo_agenda[faction_a], wire[15]);
    scatter_word(a.diplo_merc[faction_b], wire[16]);
    scatter_word(b.diplo_merc[faction_a], wire[17]);
}


bool copy_nonrule_chunk(int16_t opcode, int a2, int a3, int a4,
                        size_t payload_size, size_t offset,
                        unsigned char *packet, size_t count,
                        bool packet_to_owner, uint32_t diplomacy_wire[18],
                        Map *captured_map_tiles,
                        size_t captured_map_tile_capacity,
                        size_t captured_map_tile_first) {
    (void)a4;
    if (offset > payload_size || count > payload_size - offset) return false;
    if (count == 0) return true;
    size_t copied = 0;
#define COPY_AT(wire, object) copied += member_copy(offset, packet, count, wire, object, packet_to_owner)
#define COPY_BYTES(wire, pointer, bytes) copied += overlap_copy(offset, packet, count, wire, reinterpret_cast<unsigned char *>(pointer), bytes, packet_to_owner)

    switch (opcode) {
    case 0x00: // decimal 0: one interval serialized from separately owned Game globals
        return opcode0_copy_chunk(offset, packet, count, packet_to_owner);
    case 0x01: { // decimal 1: only the first LockEnableMask byte
        uint32_t &mask = GameDataStateStorage.fields_.lock_masks_.enable_mask_;
        COPY_BYTES(0, &mask, 1);
        break;
    }
    case 0x02: case 0x1002: { // decimal 2 / 4098: Map metadata, pointer cell excluded
        COPY_AT(0x000, MapLongitudeBounds);
        COPY_AT(0x004, MapLatitudeBounds);
        COPY_AT(0x008, MapRandSeed);
        COPY_AT(0x00C, MapSeaLevel);
        COPY_AT(0x010, MapSeaLevelCouncil);
        COPY_AT(0x014, MapArea);
        COPY_AT(0x018, MapAreaSqRoot);
        COPY_AT(0x01C, MapIsFlat);
        COPY_AT(0x020, MapLandmarkCount);
        COPY_AT(0x024, MapLandmark);
        COPY_AT(0xA24, MapAbstractLongBounds);
        COPY_AT(0xA28, MapAbstractLatBounds);
        COPY_AT(0xA2C, MapAbstractArea);
        COPY_AT(0xA30, MapSizePlanet);
        COPY_AT(0xA34, MapOceanCoverage);
        COPY_AT(0xA38, MapLandCoverage);
        COPY_AT(0xA3C, MapErosiveForces);
        COPY_AT(0xA40, MapPlanetaryOrbit);
        COPY_AT(0xA44, MapCloudCover);
        COPY_AT(0xA48, MapNativeLifeForms);
        COPY_AT(0xA4C, MapFilePath);
        break;
    }
    case 0x04: { // decimal 4: MapTiles objects, never the pointer cell
        if (payload_size % sizeof(Map) != 0 || !captured_map_tiles ||
            payload_size / sizeof(Map) > captured_map_tile_capacity)
            return false;
        const size_t tile_count = payload_size / sizeof(Map);
        for (size_t i = 0; i < tile_count; ++i)
            copied += member_copy(offset, packet, count,
                i * sizeof(Map), captured_map_tiles[i], packet_to_owner);
        break;
    }
    case 0x05: // decimal 5
        copied += bounded_array_copy(offset, packet, count, Continents,
            128 * sizeof Continents[0], packet_to_owner);
        break;
    case 0x06: // decimal 6: one complete PlayerData record
        if (a2 < 0 || a2 >= MaxPlayerNum) return false;
        COPY_AT(0, PlayersData[a2]);
        break;
    case 0x07: // decimal 7: one complete Player record
        if (a2 < 0 || a2 >= MaxPlayerNum) return false;
        COPY_AT(0, Players[a2]);
        break;
    case 0x08: { // decimal 8: five named AI preference dwords
        if (a2 < 0 || a2 >= MaxPlayerNum) return false;
        PlayerData &p = PlayersData[a2];
        COPY_AT(0x00, p.ai_fight); COPY_AT(0x04, p.ai_growth);
        COPY_AT(0x08, p.ai_tech); COPY_AT(0x0C, p.ai_wealth);
        COPY_AT(0x10, p.ai_power);
        break;
    }
    case 0x09: { // decimal 9
        if (a2 < 0 || a2 >= MaxPlayerNum) return false;
        PlayerData &p = PlayersData[a2];
        COPY_AT(0, p.tech_accumulated); COPY_AT(4, p.tech_id_researching);
        break;
    }
    case 0x0A: case 0x10: { // decimal 10 is research id; decimal 16 is proto obsolete byte
        if (opcode == 0x0A) {
            if (a2 < 0 || a2 >= MaxPlayerNum) return false;
            COPY_AT(0, PlayersData[a2].tech_id_researching);
        } else {
            if (a2 < 0 || a2 >= MaxVehProtoNum) return false;
            COPY_BYTES(0, &VehPrototypes[a2].obsolete_factions, 1);
        }
        break;
    }
    case 0x0B: { // decimal 11
        if (a2 < 0 || a2 >= MaxPlayerNum) return false;
        COPY_AT(0, PlayersData[a2].energy_reserves);
        break;
    }
    case 0x0C: { // decimal 12
        if (a2 < 0 || a2 >= MaxPlayerNum) return false;
        PlayerData &p = PlayersData[a2];
        COPY_AT(0, p.se_alloc_psych); COPY_AT(4, p.se_alloc_labs);
        break;
    }
    case 0x0D: { // decimal 13: pending category plus first active-category dword
        if (a2 < 0 || a2 >= MaxPlayerNum) return false;
        PlayerData &p = PlayersData[a2];
        COPY_AT(0, p.soc_category_pending);
        COPY_BYTES(0x10, &p.soc_category_active, sizeof(uint32_t));
        break;
    }
    case 0x0E: case 0x0F: { // decimal 14/15: prototype record/range
        if (a2 < 0 || a2 >= MaxVehProtoNum) return false;
        if (opcode == 0x0E) COPY_AT(0, VehPrototypes[a2]);
        else {
            if (a3 < 0 || a3 > MaxVehProtoNum - a2) return false;
            COPY_BYTES(0, &VehPrototypes[a2],
                       static_cast<size_t>(a3) * sizeof VehPrototypes[0]);
        }
        break;
    }
    case 0x11: { // decimal 17: first 0x30 bytes of one live Veh object
        if (a2 < 0 || a2 >= 2049) return false;
        COPY_BYTES(0, &Vehs[a2], 0x30);
        break;
    }
    case 0x12: // decimal 18: captured payload size is bounded by the true static array
        copied += bounded_array_copy(offset, packet, count, Vehs,
            payload_size, packet_to_owner);
        break;
    case 0x13: // decimal 19
        if (a2 < 0 || a2 >= MaxBaseNum) return false;
        COPY_AT(0, Bases[a2]);
        break;
    case 0x14: // decimal 20
        copied += bounded_array_copy(offset, packet, count, Bases,
            payload_size, packet_to_owner);
        break;
    case 0x15: // decimal 21
        COPY_AT(0, ControlTurnPhase);
        break;
    case 0x16: // decimal 22: caller supplies the saved local 18-dword tuple
        if (!diplomacy_wire) return false;
        copied += overlap_copy(offset, packet, count, 0,
            reinterpret_cast<unsigned char *>(diplomacy_wire),
            18 * sizeof diplomacy_wire[0], packet_to_owner);
        break;
    case 0x17: // decimal 23: native selects the first dword of PlayerData
        if (a2 < 0 || a2 >= MaxPlayerNum) return false;
        COPY_BYTES(0, &PlayersData[a2], sizeof(uint32_t));
        break;
    case 0x18: { // decimal 24: four separately bounded PlayerData members/slices
        if (a2 < 0 || a2 >= MaxPlayerNum) return false;
        PlayerData &p = PlayersData[a2];
        COPY_BYTES(0x000,
            reinterpret_cast<unsigned char *>(&p.saved_queue_name) + 20, 172);
        COPY_AT(0x0AC, p.saved_queue_size);
        COPY_AT(0x0CC, p.saved_queue_items);
        COPY_BYTES(0x20C, &p.unk_38[0], 6 * sizeof p.unk_38[0]);
        break;
    }
    case 0x19: // decimal 25
        copied += bounded_array_copy(offset, packet, count, FactionRankingsUnk,
            sizeof FactionRankingsUnk, packet_to_owner);
        break;
    case 0x20: // decimal 32: history window is Turns * eight 16-bit ranks
        if (payload_size > sizeof FactionRankingHistory) return false;
        copied += bounded_array_copy(offset, packet, count,
            FactionRankingHistory, payload_size, packet_to_owner);
        break;
    case 0x21: // decimal 33
        COPY_AT(0, ReplayEventBytes);
        break;
    case 0x22: // decimal 34: live event-byte prefix
        if (ReplayEventBytes < 0 ||
            static_cast<size_t>(ReplayEventBytes) > sizeof ReplayEvents ||
            payload_size != static_cast<size_t>(ReplayEventBytes)) return false;
        copied += bounded_array_copy(offset, packet, count, ReplayEvents,
            payload_size, packet_to_owner);
        break;
    case 0x23: { // decimal 35: the native 0x14-byte Base tail
        if (a2 < 0 || a2 >= MaxBaseNum) return false;
        Base &b = Bases[a2];
        COPY_AT(0x00, b.resource_sqr_radius);
        COPY_AT(0x04, b.specialist_total);
        COPY_AT(0x08, b.specialist_unk_1);
        COPY_AT(0x0C, b.spec);
        break;
    }
    case 0x24: { // decimal 36: six named game words + observed opaque wire tail
        COPY_AT(0x00, ScenarioSave.objective_req_victory);
        COPY_AT(0x04, ScenarioSave.objectives_sudden_death_victory);
        COPY_AT(0x08, ScenarioSave.objective_achieve_points);
        COPY_AT(0x0C, ScenarioSave.victory_achieve_bonus_points);
        COPY_AT(0x10, ScenarioSave.starting_mission_year);
        COPY_AT(0x14, ScenarioSave.ending_mission_year);
        COPY_BYTES(0x18, ScenarioSave.wire_opaque_94b4d8,
                   sizeof ScenarioSave.wire_opaque_94b4d8);
        break;
    }
    case 0x25: // decimal 37
        COPY_AT(0, GameDataStateStorage.fields_.vehicle_current_count_);
        break;
    case 0x26: // decimal 38
        COPY_AT(0, GameDataStateStorage.fields_.base_current_count_);
        break;
    case 0x27: // decimal 39: actual NetDaemon::send_files path owner, shared across methods
        copied += bounded_array_copy(offset, packet, count,
            NetDaemonFileTransferPath, sizeof NetDaemonFileTransferPath,
            packet_to_owner);
        break;
    case 0x28: // decimal 40
        return true;
    case 0x29: // decimal 41: actual Veh ProtoBung owner
        return veh_bung_copy_chunk(offset, packet, count, packet_to_owner);
    case 0x30: { // decimal 48: row rectangle in the typed MapTiles array
        if (payload_size % sizeof(Map) != 0 || !captured_map_tiles ||
            captured_map_tile_first > captured_map_tile_capacity ||
            payload_size / sizeof(Map) >
                captured_map_tile_capacity - captured_map_tile_first)
            return false;
        const size_t tile_count = payload_size / sizeof(Map);
        for (size_t i = 0; i < tile_count; ++i)
            copied += member_copy(offset, packet, count,
                i * sizeof(Map), captured_map_tiles[captured_map_tile_first + i],
                packet_to_owner);
        break;
    }
    default:
        return false;
    }
#undef COPY_AT
#undef COPY_BYTES
    return copied == count;
}

} // namespace NetDaemonSyncWire


namespace NetDaemonSyncWire {
    uint32_t current_faction();
    struct MapRouteCapture {
        Map *tiles;
        size_t capacity;
        size_t first;
    };
    // Gather/scatter exactly [offset, offset+length) in the opcode's native
    // wire view. For rules receive, scatter writes the source-owned shared
    // native staging block; commit_rules is invoked only on the last chunk.
    // For opcode 0 each implementation must refuse still-unowned bytes rather
    // than inventing gap data. All calls resolve live fields at call time.
    bool copy_out(int16_t opcode, int a2, int a3, int a4,
                  size_t payload_size, size_t offset, void *packet_bytes,
                  size_t length,
                  const MapRouteCapture &map_capture,
                  uint32_t diplomacy_wire[18]);
    bool copy_in(int16_t opcode, int a2, int a3, int a4,
                 size_t payload_size, size_t offset,
                 const void *packet_bytes, size_t length,
                 const MapRouteCapture &map_capture,
                 uint32_t diplomacy_wire[18]);
    bool compare_and_notify(int16_t opcode, int a2, int a3, int a4,
                            const void *packet_bytes, size_t length,
                            int log_arg_lo, int log_arg_hi);
    void commit_rules(int16_t opcode);
    void gather_saved_diplomacy(int faction_a, int faction_b,
                                uint32_t wire[18]);
    void scatter_saved_diplomacy(int faction_a, int faction_b,
                                 const uint32_t wire[18]);
}

// Persistent incoming-only staging shared by every native rule transfer.
// The observed required window is 0x1920; the image's complete allocation
// extent remains unknown. This object is not cleared between calls.
// RECOVERY_OBJECT: v1 0x0093AD8C _NetDaemonRulesSyncStaging
static unsigned char NetDaemonRulesSyncStaging[0x1920];

static int native_map_sync_rows(int latitude, int a2) {
    // x86's add-sign-mask/sar sequence is signed division toward zero here.
    const int q = latitude / 16;
    return a2 == 0xF ? latitude - q * 15 : q;
}

static NetDaemonSyncWire::MapRouteCapture capture_map_route(
    int16_t opcode, int a2, int map_rows) {
    NetDaemonSyncWire::MapRouteCapture captured;
    captured.tiles = MapTiles;
    // MapArea is the actual MapTiles element extent; payload_size was
    // calculated once at method entry and is not a substitute after callbacks.
    captured.capacity = static_cast<size_t>(MapArea);
    captured.first = 0;
    if (opcode == 0x30) {
        captured.first = static_cast<size_t>(map_rows * a2) * MapLongitude;
    }
    return captured;
}

// Uses the actual NetDaemon declaration from netdaemon.h: lock_state_ and
// this->send_message remain receiver-relative, unlike the singleton notify
// gate below.
static void synch_notify_or_queue() {
    const bool rules_notify = (GameDataStateStorage.fields_.game_rules_ & 0x10u) != 0;
    const bool scripted_turn = ConsoleControlTurnActive != 0;
    if (rules_notify && !scripted_turn) {
        if (GameDataStateStorage.fields_.net_turn_faction_ != ConsoleGlobal.faction_id_) {
            message_data(0x2308, 0, 0, 0, 0, 0);
        } else {
            SpockNotifyPending = 1;
        }
    } else if (NetDaemonNet.primary_state_.leading_words_[0] != 0) {
        SpockNotifyPending = 1;
    } else {
        message_data(0x2308, 0, 0, 0, 0, 0);
    }
}

// Native USER32 import directory independently identifies slot0x00669330.
// GetAsyncKeyState's shared import identity is declared in src/editbox.cpp.
static void synch_debug_popup() {
    if (BaseAtDebugPopupFlag) {
        X_pop("VEHSYNCH", 0);
        // The popup may change the shared action result; read it afterwards.
        if (!ConsoleGroupAction) GetAsyncKeyState(VK_CONTROL);
    }
}

bool NetDaemonSyncWire::compare_and_notify(int16_t opcode, int a2, int a3,
    int a4, const void *packet_payload, size_t payload_size,
    int log_arg_lo, int log_arg_hi) {
    (void)a3; (void)a4;
    const unsigned char *old_bytes =
        static_cast<const unsigned char *>(packet_payload);
    if (opcode == 0x11 && payload_size == 0x30) { // decimal 17
        int16_t old_x, old_y;
        memcpy(&old_x, old_bytes, sizeof old_x);
        memcpy(&old_y, old_bytes + 2, sizeof old_y);
        const Veh &current = Vehs[a2];
        if (old_x == current.x && old_y == current.y) return false;
        parse_num(0, old_x); parse_num(1, old_y);
        parse_num(2, current.x); parse_num(3, current.y);
        parse_num(4, current.faction_id);
        parse_num(5, current.proto_id);
        parse_num(6, a2);
        parse_says(0, VehPrototypes[current.proto_id].veh_name, -1, -1);
        log_say("Synch changed vehicle", ParseStrBuffer[0].str,
                a2, log_arg_lo, log_arg_hi);
        synch_debug_popup();
        synch_notify_or_queue();
        return true;
    }
    if (opcode == 0x13 && payload_size == 0x134) { // decimal 19
        int16_t old_x, old_y;
        memcpy(&old_x, old_bytes, sizeof old_x);
        memcpy(&old_y, old_bytes + 2, sizeof old_y);
        const Base &current = Bases[a2];
        if (old_x == current.x && old_y == current.y) return false;
        parse_num(0, old_x); parse_num(1, old_y);
        parse_num(2, current.x); parse_num(3, current.y);
        parse_num(4, a2);
        parse_says(0, reinterpret_cast<const char *>(old_bytes + 0x13), -1, -1);
        parse_says(1, current.name_string, -1, -1);
        log_say("Synch changed base", ParseStrBuffer[0].str,
                a2, log_arg_lo, log_arg_hi);
        synch_debug_popup();
        synch_notify_or_queue();
        return true;
    }
    return false;
}

namespace NetDaemonSyncWire {
bool copy_nonrule_chunk(int16_t opcode, int a2, int a3, int a4,
                        size_t payload_size, size_t offset,
                        unsigned char *packet, size_t count,
                        bool packet_to_owner, uint32_t diplomacy_wire[18],
                        Map *captured_map_tiles,
                        size_t captured_map_tile_capacity,
                        size_t captured_map_tile_first);
}
static int netdaemon_sync_payload_size(int16_t opcode, int a2, int a3);

uint32_t NetDaemonSyncWire::current_faction() {
    return static_cast<uint32_t>(ConsoleGlobal.faction_id_);
}

bool NetDaemonSyncWire::copy_out(int16_t opcode, int a2, int a3, int a4,
    size_t payload_size, size_t offset, void *packet_bytes, size_t length,
    const MapRouteCapture &map_capture, uint32_t diplomacy_wire[18]) {
    unsigned char *bytes = static_cast<unsigned char *>(packet_bytes);
    if (opcode >= 0x80 && opcode <= 0x8D)
        return NetDaemonSyncRules::copy_rule_wire_chunk(opcode, offset,
            bytes, length, false, NetDaemonRulesSyncStaging);
    return copy_nonrule_chunk(opcode, a2, a3, a4,
        payload_size,
        offset, bytes, length, false, diplomacy_wire, map_capture.tiles,
        map_capture.capacity, map_capture.first);
}

bool NetDaemonSyncWire::copy_in(int16_t opcode, int a2, int a3, int a4,
    size_t payload_size, size_t offset, const void *packet_bytes,
    size_t length,
    const MapRouteCapture &map_capture, uint32_t diplomacy_wire[18]) {
    unsigned char *bytes = const_cast<unsigned char *>(
        static_cast<const unsigned char *>(packet_bytes));
    if (opcode >= 0x80 && opcode <= 0x8D)
        return NetDaemonSyncRules::copy_rule_wire_chunk(opcode, offset,
            bytes, length, true, NetDaemonRulesSyncStaging);
    return copy_nonrule_chunk(opcode, a2, a3, a4,
        payload_size,
        offset, bytes, length, true, diplomacy_wire, map_capture.tiles,
        map_capture.capacity, map_capture.first);
}

void NetDaemonSyncWire::commit_rules(int16_t opcode) {
    NetDaemonSyncRules::commit_staged_rules(opcode,
                                            NetDaemonRulesSyncStaging);
}

static int netdaemon_sync_payload_size(int16_t opcode, int a2, int a3) {
    switch (opcode) {
    case 0x00: return 0x398;                         // decimal 0
    case 0x01: case 0x10: return 1;                 // decimal 1, 16
    case 0x02: case 0x1002: return 0xA9C;           // decimal 2, 4098
    case 0x04: return MapArea * 44;                  // decimal 4
    case 0x05: return 0xE00;                        // decimal 5
    case 0x06: return 0x20CC;                       // decimal 6
    case 0x07: return 0x59C;                        // decimal 7
    case 0x08: case 0x0D: case 0x23: return 0x14;   // decimal 8, 13, 35
    case 0x09: case 0x0C: return 8;                 // decimal 9, 12
    case 0x0A: case 0x0B: case 0x17:
    case 0x25: case 0x26: return 4;                 // decimal 10,11,23,37,38
    case 0x0E: return 0x34;                         // decimal 14; VehPrototype[a2]
    case 0x0F: return a3 * 52;                      // decimal 15
    case 0x11: return 0x30;                         // decimal 17
    case 0x12: return GameDataStateStorage.fields_.vehicle_current_count_ == 0 ? -1 : GameDataStateStorage.fields_.vehicle_current_count_ * 52; // 18
    case 0x13: return 0x134;                        // decimal 19
    case 0x14: return GameDataStateStorage.fields_.base_current_count_ == 0 ? -1 : GameDataStateStorage.fields_.base_current_count_ * 308; // 20
    case 0x15: return 4;                            // decimal 21
    case 0x16: return 0x48;                         // decimal 22
    case 0x18: return 0x224;                        // decimal 24
    case 0x19: return 0x20;                         // decimal 25
    case 0x20: return GameDataStateStorage.fields_.turn_current_num_ << 4;           // decimal 32
    case 0x21: return 4;                            // decimal 33
    case 0x22: return ReplayEventBytes;              // decimal 34
    case 0x24: return 0x98;                         // decimal 36
    case 0x27: return 0x104;                        // decimal 39
    case 0x28: return 0;                            // decimal 40
    case 0x29: return 0x1000;                       // decimal 41
    case 0x30: {
        const int latitude = MapLatitudeBounds;
        const int rows = native_map_sync_rows(latitude, a2);
        return (MapLongitudeBounds >> 1) * rows * sizeof(Map);
    }
    case 0x80: return 0xF4C;  // decimal 128
    case 0x81: return 0x13C;  // decimal 129
    case 0x82: return 0x7C;   // decimal 130
    case 0x83: return 0x280;  // decimal 131
    case 0x84: return 0x90;   // decimal 132
    case 0x85: return 0xC0;   // decimal 133
    case 0x86: return 0x510;  // decimal 134
    case 0x87: return 0x30;   // decimal 135
    case 0x88: return 0x1A0;  // decimal 136
    case 0x89: return 0xE0;   // decimal 137
    case 0x8A: return 0x32C;  // decimal 138
    case 0x8B: return 0x1920; // decimal 139
    case 0x8C: return 0x118;  // decimal 140
    case 0x8D: return 0x350;  // decimal 141
    default: return a2;
    }
}

static void netdaemon_sync_send_chunk(NetDaemon &self, int16_t opcode,
    int a2, int a3, int a4, int16_t flags, int payload_size,
    int total_chunks, int remainder,
    int chunk_index, const NetDaemonSyncWire::MapRouteCapture &map_capture,
    uint32_t diplomacy_wire[18]) {
    const int payload_length = (chunk_index == total_chunks - 1 && remainder)
        ? remainder : 0x800;
    int source_offset = chunk_index * 0x800;
    if (chunk_index == total_chunks - 1 && remainder) {
        source_offset = total_chunks == 1 ? 0 : (total_chunks - 1) * 0x800;
    }
    char *packet = static_cast<char *>(mem_get(
        static_cast<size_t>(payload_length + 0x24)));
    uint8_t *bytes = reinterpret_cast<uint8_t *>(packet);
    // Native writes only these header fields. +8..+0xB remain allocator
    // contents; do not zero-fill them.
    const uint32_t faction = NetDaemonSyncWire::current_faction();
    const uint32_t sequence = self.deletion_list_.sequence_bits();
    const int32_t wire_a2 = a2, wire_a3 = a3, wire_a4 = a4;
    memcpy(bytes + 0x00, &flags, sizeof flags);
    memcpy(bytes + 0x04, &faction, sizeof faction);
    memcpy(bytes + 0x0C, &sequence, sizeof sequence);
    memcpy(bytes + 0x10, &opcode, sizeof opcode);
    const int16_t wire_index = static_cast<int16_t>(chunk_index);
    memcpy(bytes + 0x12, &wire_index, sizeof wire_index);
    memcpy(bytes + 0x14, &wire_a2, sizeof wire_a2);
    memcpy(bytes + 0x18, &wire_a3, sizeof wire_a3);
    memcpy(bytes + 0x1C, &wire_a4, sizeof wire_a4);
    if (!NetDaemonSyncWire::copy_out(opcode, a2, a3, a4,
            static_cast<size_t>(payload_size),
            static_cast<size_t>(source_offset), bytes + 0x24,
            static_cast<size_t>(payload_length), map_capture,
            diplomacy_wire)) {
        free(packet);
        return;
    }
    self.send_message(packet, static_cast<unsigned long>(payload_length + 0x24), 0);
    free(packet);
}

// ORIGINAL: 0x00532E00 ?synch@NetDaemon@@QAEXFHHHPADHF@Z 0x00532E00-0x00534129
// RECOVERY_BINDING: v1 {"evidence": ["Pinned532E00..534129; retained git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/00532e00.cpp; independent jump-table and chunk/callback review.", "Opcode0 exact398-byte partition uses existing source objects and bounded fields; rule80..8D receive stages persist and compact into Alpha-owned fields only on the final chunk.", "Map destinations and diplomacy pair captured before callbacks; outgoing owners reread per chunk; changed base/vehicle notification suppresses scatter; sender mask is lock_state_[2].dword_[6].", "Path93AC88 send_files producer52ED40 remains pending. Stage93AD8C and game tail94B4D8 have observed window bounds only, not complete allocation extents.", "recovery/tasks/netdaemon-player-lifecycle-recovery.json; retained historical claims and prototype-history owner migration."], "reason": "Recover the complete synchronization protocol through genuine NetDaemon state and source-owned game, vehicle, faction, map and rule data. Wire serializers describe explicit member intervals, not an invented contiguous game object. Retain seed provenance and original identity; no matching or runtime acceptance is implied."}
// TRIED: full transcription of both jump-table dispatches (size switch, address switch, tail array-compaction switch) and the chunk loop; MISMATCH #0 in the prologue (frame-size/local-order only), not in the dispatch logic - coverage is close to 100% of the original's ~4900 bytes, byte-exactness not chased further.
void NetDaemon::synch(int16_t opcode, int a2, int a3, int a4, char *packet,
                      int outgoing, int16_t flags) {
    if (!IsMultiplayerNet) return;

    // Native synchronization progress is owned by lock_state_[0]'s existing
    // dword slots 7/8. The popup and state transition order follows the native
    // first dispatch; these calls are before target capture/chunk callbacks.
    if (!outgoing) {
        uint32_t &game_phase = primary_state_.trailing_words_[2];
        uint32_t &rules_phase = primary_state_.trailing_words_[3];
        if (opcode == 0x00 && static_cast<int32_t>(game_phase) >= 0) {
            NetMsgPrimary.pop("SYNCHINGGAME", outgoing, 1,
                             reinterpret_cast<const char *>(outgoing));
            do_all_draws(); game_phase = 1;
        } else if (opcode == 0x04 && game_phase == 1) {
            NetMsgPrimary.pop("SYNCHINGWORLD", outgoing, 1,
                             reinterpret_cast<const char *>(outgoing));
            do_all_draws(); game_phase = 2;
        } else if (opcode == 0x05 && game_phase == 2) {
            NetMsgPrimary.pop("SYNCHINGREGIONS", outgoing, 1,
                             reinterpret_cast<const char *>(outgoing));
            do_all_draws(); game_phase = 3;
        } else if (opcode == 0x06 && game_phase == 3) {
            NetMsgPrimary.pop("SYNCHINGPLAYERS", outgoing, 1,
                             reinterpret_cast<const char *>(outgoing));
            do_all_draws(); game_phase = 4;
        } else if (opcode == 0x12 && game_phase == 4) {
            NetMsgPrimary.pop("SYNCHINGUNITS", outgoing, 1,
                             reinterpret_cast<const char *>(outgoing));
            do_all_draws(); game_phase = 5;
        } else if (opcode == 0x19 && game_phase == 5) {
            NetMsgPrimary.pop("SYNCHINGPRESTIGE", outgoing, 1,
                             reinterpret_cast<const char *>(outgoing));
            do_all_draws(); game_phase = 6;
        } else if (opcode == 0x21 && game_phase == 6) {
            NetMsgPrimary.pop("SYNCHINGREPLAY", outgoing, 1,
                             reinterpret_cast<const char *>(outgoing));
            do_all_draws(); game_phase = 7;
        } else if (opcode == 0x15) {
            NetMsgPrimary.close(); do_all_draws(); game_phase = 8;
        }
        if (opcode == 0x80 && static_cast<int32_t>(rules_phase) >= 0) {
            NetMsgPrimary.pop("RULESYNCHTECH", outgoing, 1,
                             reinterpret_cast<const char *>(outgoing));
            do_all_draws(); rules_phase = 1;
        } else if (opcode == 0x83 && rules_phase == 1) {
            NetMsgPrimary.pop("RULESYNCHTERR", outgoing, 1,
                             reinterpret_cast<const char *>(outgoing));
            do_all_draws(); rules_phase = 2;
        } else if (opcode == 0x86 && rules_phase == 2) {
            NetMsgPrimary.pop("RULESYNCHVEH", outgoing, 1,
                             reinterpret_cast<const char *>(outgoing));
            do_all_draws(); rules_phase = 3;
        } else if (opcode == 0x8B && rules_phase == 3) {
            NetMsgPrimary.pop("RULESYNCHFAC", outgoing, 1,
                             reinterpret_cast<const char *>(outgoing));
            do_all_draws(); rules_phase = 4;
        } else if (opcode == 0x8D) {
            NetMsgPrimary.close(); do_all_draws(); rules_phase = 5;
        }
    }

    int log_arg_lo = 0, log_arg_hi = 0;
    if (opcode == 0x11) { // decimal 17
        const Veh &veh = Vehs[a2];
        log_arg_lo = veh.x;
        log_arg_hi = veh.y;
        log_say(outgoing ? "Sent SYNCH VEH" : "Received SYNCH VEH",
                a2, log_arg_lo, log_arg_hi);
    } else if (opcode == 0x13) { // decimal 19
        log_arg_lo = Bases[a2].x;
        log_arg_hi = Bases[a2].y;
        log_say(outgoing ? "Sending SYNCH_BASE" : "Received SYNCH_BASE",
                Bases[a2].name_string, a2, 0, 0);
    }

    // These locals preserve the native pair identities and dword tuple across
    // callbacks. On receive the tuple is filled from the one 0x48-byte packet;
    // the PlayerData pair is not changed until the post-loop epilogue.
    const int saved_diplomacy_a = a2;
    const int saved_diplomacy_b = a3;
    uint32_t diplomacy_wire[18];
    if (opcode == 0x16 && outgoing)
        NetDaemonSyncWire::gather_saved_diplomacy(
            saved_diplomacy_a, saved_diplomacy_b, diplomacy_wire);

    const int size = netdaemon_sync_payload_size(opcode, a2, a3);
    // Native early exits for empty live vehicle/base tables.
    if (size < 0) return;

    // Match signed x86 div/mod-by-0x800 rounding. Native ordinary inputs are
    // nonnegative; for negative values the arithmetic-shift path is retained.
    const int quotient = size / 0x800;
    const int remainder = size % 0x800;
    const int total_chunks = quotient + (remainder != 0 ? 1 : 0);
    const int map_rows = opcode == 0x30
        ? native_map_sync_rows(MapLatitudeBounds, a2) : 0;
    if (total_chunks <= 0) {
        // Native jumps directly to the epilogue; no callback or packet.
    } else if (outgoing) {
        for (int i = 0; i < total_chunks; ++i) {
            // Native resolves source pointers inside each loop iteration, so
            // send_message mutations can affect the next chunk's live owner.
            const NetDaemonSyncWire::MapRouteCapture map_capture =
                capture_map_route(opcode, a2, map_rows);
            netdaemon_sync_send_chunk(*this, opcode, a2, a3, a4, flags, size,
                                      total_chunks, remainder, i, map_capture,
                                      diplomacy_wire);
        }
    } else {
        // Native receives one packet/chunk per call. It reads the signed short
        // index, calls the display callback, then reloads packet fields/payload.
        int16_t incoming_index;
        memcpy(&incoming_index, packet + 0x12, sizeof incoming_index);
        const int last_index = total_chunks - 1;
        const bool initial_is_last = incoming_index == last_index;
        const int current_length = (initial_is_last && remainder)
            ? remainder : 0x800;
        const size_t current_offset = initial_is_last && remainder
            ? (total_chunks == 1 ? 0 :
               static_cast<size_t>(total_chunks - 1) * 0x800)
            : static_cast<size_t>(incoming_index) * 0x800;
        // Native computes the destination address before synch_display; keep
        // pointer/capacity selection stable across that callback for this
        // received packet, while reading the selected object's current bytes
        // only when the later compare/scatter instruction does so.
        const NetDaemonSyncWire::MapRouteCapture map_capture =
            capture_map_route(opcode, a2, map_rows);
        uint32_t sender;
        memcpy(&sender, packet + 0x04, sizeof sender);
        if (initial_is_last)
            channels_[1].trailing_words_[1] &=
                ~(static_cast<uint32_t>(1) << (sender & 31));
        synch_display(opcode, static_cast<int>(incoming_index) + 1,
                      total_chunks, a2);

        // Native resolves the destination/length before synch_display, then
        // reloads payload from the original packet argument after the callback.
        // The final-chunk postprocessing below independently reloads +0x12.
        bool suppress_copy = false;
        if (current_length == size &&
            (opcode == 0x11 || opcode == 0x13)) { // decimal 17 or 19
            suppress_copy = NetDaemonSyncWire::compare_and_notify(
                opcode, a2, a3, a4, packet + 0x24,
                static_cast<size_t>(current_length), log_arg_lo,
                log_arg_hi);
        }
        if (!suppress_copy && !NetDaemonSyncWire::copy_in(
                opcode, a2, a3, a4, static_cast<size_t>(size),
                current_offset, packet + 0x24,
                static_cast<size_t>(current_length), map_capture,
                diplomacy_wire)) return;

        memcpy(&incoming_index, packet + 0x12, sizeof incoming_index);
        if (incoming_index == last_index) {
            if (opcode == 0x12) // decimal 18
                rebuild_vehicle_bits();
            else if (opcode == 0x14) // decimal 20
                rebuild_base_bits();
            else if (opcode >= 0x80 && opcode <= 0x8D)
                NetDaemonSyncWire::commit_rules(opcode);
        }
    }

    // Native epilogue is keyed by the original signed opcode and runs after
    // send loop / received chunk processing.
    if (opcode == 0) {
        prefs_use();
    } else if (opcode == 0x02 && primary_state_.leading_words_[0] == 0) { // decimal 2
        map_shutdown();
        map_init();
        Paths.init();
    } else if (opcode == 0x13) { // decimal 19
        TutWinBaseWindow.check_base(a2);
    } else if (opcode == 0x11) { // decimal 17
        TutWinBaseWindow.check_veh(a2);
    } else if (opcode == 0x16) { // decimal 22
        NetDaemonSyncWire::scatter_saved_diplomacy(
            saved_diplomacy_a, saved_diplomacy_b, diplomacy_wire);
    } else if (opcode == 0x0D) { // decimal 13
        social_set(static_cast<uint32_t>(a2));
    }
}

// Clear the real daemon lock entries, then close its AlphaNet base.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0051DD00 sub_51dd00 0x0051DD00-0x0051DDE0 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/0051dd00.cpp"], "reason": "Recover typed lock-entry reset, borrowed selection pointer clearing and ordered lock cleanup before qualified AlphaNet close."}
// symbol    ?reset@NetDaemon@@QAEXXZ
void NetDaemon::reset() {
    primary_state_.leading_words_[0] = 0;
    primary_state_.leading_words_[1] = 0;
    primary_state_.leading_words_[2] = 0;
    primary_state_.flags_ = 0;
    primary_state_.trailing_words_[8] = 0;
    primary_state_.trailing_words_[7] = 0;

    channels_[0].leading_words_[1] = 0;
    channels_[0].leading_words_[0] = 0;
    channels_[0].trailing_words_[0] = 0;
    channels_[1].leading_words_[1] = 0;
    channels_[0].trailing_words_[9] = 0;
    channels_[1].leading_words_[0] = 0;
    channels_[1].trailing_words_[5] = 0;
    channels_[1].trailing_words_[4] = 0;

    primary_state_.trailing_words_[4] = 0;
    primary_state_.trailing_words_[5] = 0;
    primary_state_.trailing_words_[6] = 0;
    channels_[1].trailing_words_[3] = 0;
    channels_[1].trailing_words_[2] = 0;
    channels_[1].leading_words_[3] = 0;
    channels_[1].leading_words_[2] = 0;
    channels_[1].payload_.flags = 0;
    primary_state_.trailing_words_[0] = 0;
    primary_state_.trailing_words_[1] = 0;
    channels_[1].trailing_words_[8] = 0;
    channels_[1].trailing_words_[7] = 0;
    primary_state_.trailing_words_[2] = 0;
    primary_state_.trailing_words_[3] = 0;
    channels_[1].trailing_words_[6] = 0xFF;
    channels_[0].payload_.vehicle_selection_follow = nullptr;

    for (int i = 1; i <= 8; ++i) {
        channels_[0].trailing_words_[i] = 0;
    }

    locks_.clear();
    AlphaNet::close();
}

/*
Purpose: Deflate one buffer in a single shot: build the z_stream over the
         caller's in/out pointers, deflate at the default level to the finish
         mark, and hand the produced size back through the out-size slot. A
         dry deflate answers Z_BUF_ERROR; any other non-stream-end answer
         comes back as itself.

// ORIGINAL: 0x00634280 sub_634280 0x00634280-0x00634326 FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:c96a47e3f89e8792c8514ac2ca79259ab6f42c3e:src/unrecovered/00634280.cpp", "Historical seed spelling sub_634280; current source mapping sub_634280; emitted symbol ?sub_634280@@YAHPAEPAK0I@Z."], "reason": "Retain the original function mapping on its compiled C++ owner. Superseded seed retired after owner review; original spans, existing claims and useful seed findings remain in source. This cleanup grants no new match or acceptance claim."}
// TRIED: `if (dr == 1) { ...; return deflateEnd(); } else {...}` flipped je/jne against the original (0.78 sim); the `if (dr != 1) cleanup ... success falls through` polarity matched (0.95 sim); the remaining diff was the duplicated epilogue the compiler did not share between the two returns.
// TRIED: `if (dr == 1) { ...; return deflateEnd_real(); } else {...}` flipped je/jne against the original (0.78 sim). `if (dr != 1) {cleanup path} ...; success path falls through` matches polarity (0.95 sim); the remaining diff is a duplicated epilogue the compiler didn't share between the two returns.
// symbol    ?netdaemon_deflate_buffer@@YAHPAEPAK0I@Z
// size      166 bytes
// callers   1   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0063CF00 0x0063D1A0 0x0063D4D0
Status: Coverage transcription - unclaimed body
*/

int __cdecl netdaemon_deflate_buffer(unsigned char *next_out,
                                      unsigned long *out_size,
                                      unsigned char *next_in,
                                      unsigned int avail_in) {
    z_stream stream;
    stream.next_in = next_in;
    stream.avail_in = static_cast<uInt>(avail_in);
    stream.next_out = next_out;
    stream.avail_out = static_cast<uInt>(*out_size);
    stream.zalloc = Z_NULL; stream.zfree = Z_NULL;
    const int init_status = deflateInit_(&stream, -1, ZLIB_VERSION,
                                       static_cast<int>(sizeof(z_stream)));
    if (init_status != 0) return init_status;
    const int status = deflate(&stream, Z_FINISH);
    if (status != Z_STREAM_END) {
        deflateEnd(&stream);
        return status != 0 ? status : Z_BUF_ERROR;
    }
    *out_size = stream.total_out;
    return deflateEnd(&stream);
}


/*
Purpose: Inflate compressed input into next_out with *out_size as the
         output capacity, writing total_out back only on stream end.
         On other inflate statuses, call inflateEnd and return the
         original inflate status unchanged.

// ORIGINAL: 0x00634330 sub_634330 0x00634330-0x006343BD FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:c96a47e3f89e8792c8514ac2ca79259ab6f42c3e:src/unrecovered/00634330.cpp", "Historical seed spelling sub_634330; current source mapping sub_634330; emitted symbol ?sub_634330@@YAHPAEHPAKI@Z.", "Pinned caller 0x00534C81 pushes destination, destination-size pointer, compressed input pointer and compressed byte count in cdecl order.", "Pinned callee 0x00634330 loads *arg2 into avail_out, arg3 into next_in, arg4 into avail_in, and stores total_out through arg2 only on Z_STREAM_END; the old seed second-argument-unused transcription is false.", "Pinned stream initialization zeros zalloc and zfree but leaves opaque untouched; preserve this original stack-field quirk."], "reason": "Recover the original four-argument inflate wrapper on the NetDaemon transfer path, preserving its size-write and error/cleanup order. Correct the old seed's disproven argument interpretation; this grants no new byte or ABI match."}
// TRIED: the wrapper's second argument is never read - the in-place work pointer (third argument) carries both the input and the output-size handback, which the seed's dword-indexed reading had assigned to the wrong slot.
// symbol    ?netdaemon_inflate_buffer@@YAHPAEPAK0I@Z
// size      141 bytes
// callers   1   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// indirect  0x0063436D 0x00634380 0x00634394 0x006343AF
// Pinned correction: 0x00534C81 passes dest, &file_size, compressed source, source size; 0x00634330 reads capacity from arg2 and input from arg3.
Status: Coverage transcription - unclaimed body
*/

int __cdecl netdaemon_inflate_buffer(unsigned char *next_out,
                                      unsigned long *out_size,
                                      unsigned char *next_in,
                                      unsigned int avail_in) {
    z_stream stream;
    stream.next_in = next_in;
    stream.avail_in = static_cast<uInt>(avail_in);
    stream.next_out = next_out;
    stream.avail_out = static_cast<uInt>(*out_size);
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    const int init_status = inflateInit_(&stream, ZLIB_VERSION,
                                         static_cast<int>(sizeof(z_stream)));
    if (init_status != 0) {
        return init_status;
    }
    const int inflate_status = inflate(&stream, Z_FINISH);
    if (inflate_status != Z_STREAM_END) {
        inflateEnd(&stream);
        return inflate_status;
    }
    *out_size = stream.total_out;
    return inflateEnd(&stream);
}
