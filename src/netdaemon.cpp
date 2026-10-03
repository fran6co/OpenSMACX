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
#include "veh.h"           // Vehs / VehCurrentCount, which game_checksum walks
#include "map.h"           // the map bounds, PlayersData and Bases it sums
#include "technology.h"    // GameTechAchieved
#include "game.h"          // IsMultiplayerNet: the application's shared flag
#include "console.h"
#include "fx.h"
#include "temp.h"
#include "alpha.h"
#include "council.h"
#include "terraforming.h"
#include "path.h"
#include "tutwin.h"
#include "xpops.h"
#include "general.h"
#include "filemap.h"
#include "zlib.h"
#include <direct.h>
#include <new>
#include "replaywin.h"     // ReplayEventBytes: replay recorder's byte length

// The next dword after IsMultiplayerNet. Every sender in this file
// increments it around one send_message call, every wait loop increments it
// for the length of its poll and decrements it on the way out, and
// MultiWin::draw_time (0x00478BE0) declines to draw the turn clock unless it
// is zero - so it counts the network exchanges currently in flight.
int NetBusyCount;  // 0x0093F664
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
    if (lock_state_[0].leading_words_[3]) {
        return 1;
    }
    Base &base = Bases[base_id];
    log_say("LockBase Needs Lock", base_id, base.x, base.y);
    return lock(target | 4, base.x, base.y, 0, payload_1C, payload_20);
}

// The map row stride in dwords, read as a value. mapwin.cpp keeps the same
// fixed-address binding file-local and no header exports it; game_checksum's
// per-row walk multiplies it by the row index to reach a tile record.
static int *const MapRowStride = (int *)0x0068FAF0;

// Derived player lifecycle callbacks. Retained seeds supply historical hints;
// the original missing-player tail and its unresolved index domain are kept.
// ORIGINAL: 0x0052E160 ?on_player_added@NetDaemon@@UAEXPADK@Z 0x0052E160-0x0052E364
// RECOVERY_BINDING: v1 {"evidence": ["Pinned52E160..52E364 and52E370..52E8FD; retained recovered seeds are hints, not mandatory C++.", "The not-found delete tail still runs and decrements count. Slot8 identity/status bytes belong to the real ledger; lobby prefix/name writes are byte views wholly inside the existing168-byte session object. Keep both session object lifetimes and their original bindings.", "PlayerLock::active return controls notification; process identity offsets are not doubled. Two label_get calls remain independent, and prefix writes depend on live phase below2.", "Source-owned NextTurnReady, NetUpkeepPhase and NetHostChangeSlot replace private address aliases. Upkeep producer remains unfinished; ledger-derived lock index domain remains unproven.", "recovery/tasks/netdaemon-player-lifecycle-recovery.json; actual-source host damage controls and native compilation required"], "reason": "Recover the complete derived network player lifecycle callback through actual AlphaNet records, PlayerLock behavior, NetWin storage and shared turn state. Preserve its original identity and seed; no exact, runtime or full acceptance claim is implied."}
void NetDaemon::on_player_added(char *name, unsigned long id) {
    const bool same_host = (field_760_ == field_764_);
    lock_state_[0].leading_words_[1] = same_host ? 1 : 0;

    if (player_count_ < 7 && lock_state_[0].trailing_words_[0] < 2) {
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

        if (lock_state_[0].leading_words_[1] != 0) {
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
// RECOVERY_BINDING: v1 {"evidence": ["Pinned52E160..52E364 and52E370..52E8FD; retained recovered seeds are hints, not mandatory C++.", "The not-found delete tail still runs and decrements count. Slot8 identity/status bytes belong to the real ledger; lobby prefix/name writes are byte views wholly inside the existing168-byte session object. Keep both session object lifetimes and their original bindings.", "PlayerLock::active return controls notification; process identity offsets are not doubled. Two label_get calls remain independent, and prefix writes depend on live phase below2.", "Source-owned NextTurnReady, NetUpkeepPhase and NetHostChangeSlot replace private address aliases. Upkeep producer remains unfinished; ledger-derived lock index domain remains unproven.", "recovery/tasks/netdaemon-player-lifecycle-recovery.json; actual-source host damage controls and native compilation required"], "reason": "Recover the complete derived network player lifecycle callback through actual AlphaNet records, PlayerLock behavior, NetWin storage and shared turn state. Preserve its original identity and seed; no exact, runtime or full acceptance claim is implied."}
void NetDaemon::on_player_deleted(char *, unsigned long id) {
    parse_says(0, const_cast<char *>("ED"), -1, -1);

    const bool server_left = (NetHostChangeSlot == static_cast<uint32_t>(id));
    if (server_left) {
        lock_state_[2].trailing_words_[6] = 1;
    }

    int slot = 1;
    while (slot <= player_count_ && process_slots_[slot].pid_ != id) {
        ++slot;
    }
    const bool found = slot <= player_count_;

    if (found) {
        uint32_t &ready_players = lock_state_[0].trailing_words_[4];
        if (ready_players != 0) {
            ready_players &= ~(1u << (slot & 31));
            if (ready_players == 0) {
                g_FX.play(0x15);
            }
        }

        AlphaNetProcessSlot &player = process_slots_[slot];
        parse_says(0, player.name_, -1, -1);
        const int player_id = player.identity_;

        if (lock_state_[0].trailing_words_[0] == 2) {
            uint8_t humans = static_cast<uint8_t>(LockEnableMask & 0xFF);
            humans = static_cast<uint8_t>(humans &
                ~static_cast<uint8_t>(1u << (player_id & 31)));
            LockEnableMask = (LockEnableMask & 0xFFFFFF00u) | humans;
        }
        lock_state_[2].payload_.flags &= ~(1u << (player_id & 31));

        const bool turn_handoff = (NetTurnFlags & 0x10) != 0;
        if (turn_handoff && lock_state_[0].leading_words_[1] != 0 &&
            NetTurnFaction == player_id) {
            const uint8_t alive = static_cast<uint8_t>(LockEnableMask >> 8);
            const uint8_t humans = static_cast<uint8_t>(LockEnableMask);
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
    uint32_t &faction_lock_mask = lock_state_[1].trailing_words_[0];
    const uint32_t player_lock_mask =
        lock_state_[1].trailing_words_[1 + player_id];
    faction_lock_mask &= ~player_lock_mask;
    if (static_cast<int>(locks_.field_E0_) == player_id) {
        locks_.field_E0_ = 0;
        locks_.field_E4_ = 0;
    }

    const uint8_t turn_bit = static_cast<uint8_t>(NetTurnFlags & 0x10);
    const bool skip_lock_notify =
        (turn_bit != 0 && ConsoleControlTurnActive == 0 &&
         NetTurnFaction == ConsoleGlobal.faction_id_) ||
        (turn_bit == 0 && lock_state_[0].leading_words_[1] != 0);

    if (!skip_lock_notify && lock_state_[0].trailing_words_[0] == 2) {
        const uint8_t alive = static_cast<uint8_t>(LockEnableMask >> 8);
        const uint8_t humans = static_cast<uint8_t>(LockEnableMask);
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
        if (lock_state_[0].trailing_words_[0] < 2) {
            NetWinSessionData.write_row8_prefix(0xFF);
        }
        strcpy(NetWinSessionData.row8_leader_name(), label_get(0xFE4 / 4));
        // Separate native Strings::get calls reload the owner and index.
        strcpy(NetWinSessionData.row8_noun_faction(), label_get(0xFE4 / 4));
    } else {
        NetWinLobbyRow &removed_row = NetWinLobbyRows[slot];
        if (lock_state_[0].trailing_words_[0] < 2) {
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
    lock_state_[0].leading_words_[1] = (field_760_ == field_764_) ? 1 : 0;

    const uint8_t post_turn_bit = static_cast<uint8_t>(NetTurnFlags & 0x10);
    const bool skip_after_delete =
        (post_turn_bit == 0 || ConsoleControlTurnActive != 0 ||
         NetTurnFaction != ConsoleGlobal.faction_id_) &&
        ((post_turn_bit != 0 && ConsoleControlTurnActive == 0) ||
         lock_state_[0].leading_words_[1] == 0);

    if (skip_after_delete && lock_state_[0].trailing_words_[0] == 2) {
        if (post_turn_bit == 0 &&
            (lock_state_[0].trailing_words_[7] &
             (1u << (ConsoleGlobal.faction_id_ & 31))) != 0) {
            message_data(0x8301, 0, 0, 0, 0, 0);
        }
        // The original reloads this global after the 0x8301 callback.
        if (ConsoleControlTurnActive != 0) {
            message_data(0x2303, 0, NetUpkeepPhase,
                         TurnCurrentNum, 0, 0);
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
        lock_state_[2].leading_words_[3] = 0;
        lock_state_[2].trailing_words_[0] = 0;
        lock_state_[2].leading_words_[1] = 0;
        lock_state_[1].leading_words_[2] = 2;
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
    lock_state_[0].leading_words_[3] = 0;
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
NetDaemon::NetDaemon() : AlphaNet() {
    // The eight per-player locks (0x14A0, 0x1C stride), then the embedded
    // Lock of the first one cleared directly - the image calls the Lock
    // version explicitly, twice (here and again after the table below).
    for (int32_t n = 0; n < 8; ++n) {
        locks_.records_[n].clear();
    }
    locks_.clear();

    for (int32_t m = 0; m < 0x18; ++m) {
        lock_table_[m].flag_ = 0xFF;
        lock_table_[m].word_8_ = 0;
        // the per-entry scratch dword lives past the entry, at +0x5A0
        lock_state_[m].leading_words_[0] = 0;
    }

    lock_state_[0].leading_words_[1] = 0;
    lock_state_[0].leading_words_[2] = 0;
    lock_state_[0].leading_words_[3] = 0;
    lock_state_[0].payload_.flags = 0;
    lock_state_[0].trailing_words_[8] = 0;
    lock_state_[0].trailing_words_[7] = 0;
    lock_state_[1].leading_words_[1] = 0;
    lock_state_[1].leading_words_[0] = 0;
    lock_state_[1].payload_.vehicle_selection_follow = 0;
    lock_state_[2].leading_words_[1] = 0;
    lock_state_[1].trailing_words_[9] = 0;
    lock_state_[2].leading_words_[0] = 0;
    lock_state_[2].trailing_words_[4] = 0;
    lock_state_[2].trailing_words_[3] = 0;
    lock_state_[0].trailing_words_[4] = 0;
    lock_state_[0].trailing_words_[5] = 0;
    lock_state_[0].trailing_words_[6] = 0;
    lock_state_[2].trailing_words_[1] = 0;
    lock_state_[2].trailing_words_[0] = 0;
    lock_state_[2].leading_words_[3] = 0;
    lock_state_[2].leading_words_[2] = 0;
    lock_state_[2].payload_.flags = 0;
    lock_state_[0].trailing_words_[0] = 0;
    lock_state_[0].trailing_words_[1] = 0;
    lock_state_[2].trailing_words_[7] = 0;
    lock_state_[2].trailing_words_[6] = 0;
    lock_state_[0].trailing_words_[2] = 0;
    lock_state_[0].trailing_words_[3] = 0;
    lock_state_[1].leading_words_[3] = 0;

    // 0x1BD0 is lock_state_[2]'s eleventh dword - the ctor sets it to 0xFF.
    lock_state_[2].trailing_words_[6] = 0xFF;
    for (int32_t i = 0; i < 8; i++) {
        lock_state_[1].trailing_words_[i] = 0;
    }

    locks_.clear();
    AlphaNet::close();
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
    const int result = NetDaemonNet.Net::get(&first, &second);
    if (result == 0) {
        return 0;
    }
    process_message(reinterpret_cast<char *>(result), second, static_cast<int>(first));
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
        lock_state_[2].leading_words_[3] = 0;
        lock_state_[2].leading_words_[2] = 0;
        lock_state_[2].trailing_words_[5] = 0;
        lock_state_[2].trailing_words_[4] = 0;
    }
    // Shared tail: 0x1B78 is the vehicle record lock_veh (0x00531020) stores
    // and 0x1BC4 its held flag.
    lock_state_[1].payload_.vehicle_selection_follow = 0;
    lock_state_[2].trailing_words_[3] = 0;
    return residue;
}


// The daemon's Net, 0x0093CD90 in the image - see netdaemon.h. The image's
// dynamic initializer at 0x0052DB00 constructs it at the fixed address
// before WinMain; the recovered NetDaemon constructor here runs through this
// build's own startup instead.
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
        lock_state_[2].leading_words_[3] = 0;
        lock_state_[2].leading_words_[2] = 0;
        lock_state_[2].trailing_words_[5] = 0;
        lock_state_[2].trailing_words_[4] = 0;
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
        lock_state_[1].trailing_words_[9] = 0;
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
    if (lock_state_[0].payload_.flags != 0) {
        return;
    }
    if (IsMultiplayerNet == 0) {
        return;
    }
    lock_state_[0].payload_.flags = 1;
    if (NetTurnFlags & 0x10) {
        if (ConsoleControlTurnActive == 0) {
            if (NetTurnFaction != ConsoleGlobal.faction_id_) {
                goto L0cc;
            }
        } else if (NetDaemonNet.lock_state_[0].leading_words_[1] == 0) {
            goto L145;
        }
        message_data(0x8301, 0, 0, 0, 0, 0);
    }
L0cc:
    if (NetDaemonNet.lock_state_[0].leading_words_[1] != 0) {
        locks_.records_[ConsoleGlobal.faction_id_].clear();
        if (locks_.field_E0_ == ConsoleGlobal.faction_id_) {
            locks_.field_E4_ = 0;
            locks_.field_E0_ = 0;
        }
        lock_state_[1].trailing_words_[0] &=
            ~lock_state_[1].trailing_words_[1 + ConsoleGlobal.faction_id_];
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
    unsigned long id;

    synch(0, 0, 0, 0, 0, 1, 0x2101);
    for (;;) {
        id = NetDaemonNet.Net::get(&a, &b);
        if (id == 0) break;
        process_message((char *)id, b, a);
    }

    synch(2, 0, 0, 0, 0, 1, 0x2101);
    for (;;) {
        id = NetDaemonNet.Net::get(&a, &b);
        if (id == 0) break;
        process_message((char *)id, b, a);
    }

    synch(4, 0, 0, 0, 0, 1, 0x2101);
    for (;;) {
        id = NetDaemonNet.Net::get(&a, &b);
        if (id == 0) break;
        process_message((char *)id, b, a);
    }

    synch(5, 0, 0, 0, 0, 1, 0x2101);
    for (;;) {
        id = NetDaemonNet.Net::get(&a, &b);
        if (id == 0) break;
        process_message((char *)id, b, a);
    }

    int i = 0;
    int off = 0;
    do {
        synch(6, i, 0, 0, 0, 1, 0x2101);
        for (;;) {
            id = NetDaemonNet.Net::get(&c, &d);
            if (id == 0) break;
            process_message((char *)id, d, c);
        }

        synch(7, i, 0, 0, 0, 1, 0x2101);
        for (;;) {
            id = NetDaemonNet.Net::get(&e, &f);
            if (id == 0) break;
            process_message((char *)id, f, e);
        }

        synch(0xf, off, 0x40, 0, 0, 1, 0x2101);
        for (;;) {
            id = NetDaemonNet.Net::get(&e, &f);
            if (id == 0) break;
            process_message((char *)id, f, e);
        }

        off += 0x40;
        i++;
    } while (off < 0x200);

    synch(0x29, 0, 0, 0, 0, 1, 0x2101);
    for (;;) {
        id = NetDaemonNet.Net::get(&e, &f);
        if (id == 0) break;
        process_message((char *)id, f, e);
    }

    synch(0x12, 0, 0, 0, 0, 1, 0x2101);
    for (;;) {
        id = NetDaemonNet.Net::get(&e, &f);
        if (id == 0) break;
        process_message((char *)id, f, e);
    }

    synch(0x14, 0, 0, 0, 0, 1, 0x2101);
    for (;;) {
        id = NetDaemonNet.Net::get(&e, &f);
        if (id == 0) break;
        process_message((char *)id, f, e);
    }

    synch(0x19, 0, 0, 0, 0, 1, 0x2101);
    for (;;) {
        id = NetDaemonNet.Net::get(&e, &f);
        if (id == 0) break;
        process_message((char *)id, f, e);
    }

    if (TurnCurrentNum != 0) {
        synch(0x20, 0, 0, 0, 0, 1, 0x2101);
        for (;;) {
            id = NetDaemonNet.Net::get(&e, &f);
            if (id == 0) break;
            process_message((char *)id, f, e);
        }
    }

    synch(0x21, 0, 0, 0, 0, 1, 0x2101);
    for (;;) {
        id = NetDaemonNet.Net::get(&e, &f);
        if (id == 0) break;
        process_message((char *)id, f, e);
    }

    if (ReplayEventBytes != 0) {
        synch(0x22, 0, 0, 0, 0, 1, 0x2101);
        for (;;) {
            id = NetDaemonNet.Net::get(&e, &f);
            if (id == 0) break;
            process_message((char *)id, f, e);
        }
    }

    synch(0x24, 0, 0, 0, 0, 1, 0x2101);
    for (;;) {
        id = NetDaemonNet.Net::get(&e, &f);
        if (id == 0) break;
        process_message((char *)id, f, e);
    }

    synch(0x15, 0, 0, 0, 0, 1, 0x2101);
    for (;;) {
        id = NetDaemonNet.Net::get(&e, &f);
        if (id == 0) break;
        process_message((char *)id, f, e);
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
    while (int result = NetDaemonNet.Net::get(&v4, &v8)) {
        process_message(reinterpret_cast<char *>(result), v8, v4);
    }

    synch(0x81, 0, 0, 0, 0, 1, 0x2101);
    while (int result = NetDaemonNet.Net::get(&v4, &v8)) {
        process_message(reinterpret_cast<char *>(result), v8, v4);
    }

    synch(0x82, 0, 0, 0, 0, 1, 0x2101);
    while (int result = NetDaemonNet.Net::get(&v4, &v8)) {
        process_message(reinterpret_cast<char *>(result), v8, v4);
    }

    synch(0x83, 0, 0, 0, 0, 1, 0x2101);
    while (int result = NetDaemonNet.Net::get(&v4, &v8)) {
        process_message(reinterpret_cast<char *>(result), v8, v4);
    }

    synch(0x84, 0, 0, 0, 0, 1, 0x2101);
    while (int result = NetDaemonNet.Net::get(&v4, &v8)) {
        process_message(reinterpret_cast<char *>(result), v8, v4);
    }

    synch(0x85, 0, 0, 0, 0, 1, 0x2101);
    while (int result = NetDaemonNet.Net::get(&v4, &v8)) {
        process_message(reinterpret_cast<char *>(result), v8, v4);
    }

    synch(0x86, 0, 0, 0, 0, 1, 0x2101);
    while (int result = NetDaemonNet.Net::get(&v4, &v8)) {
        process_message(reinterpret_cast<char *>(result), v8, v4);
    }

    synch(0x87, 0, 0, 0, 0, 1, 0x2101);
    while (int result = NetDaemonNet.Net::get(&v4, &v8)) {
        process_message(reinterpret_cast<char *>(result), v8, v4);
    }

    synch(0x88, 0, 0, 0, 0, 1, 0x2101);
    while (int result = NetDaemonNet.Net::get(&v4, &v8)) {
        process_message(reinterpret_cast<char *>(result), v8, v4);
    }

    synch(0x89, 0, 0, 0, 0, 1, 0x2101);
    while (int result = NetDaemonNet.Net::get(&v4, &v8)) {
        process_message(reinterpret_cast<char *>(result), v8, v4);
    }

    synch(0x8a, 0, 0, 0, 0, 1, 0x2101);
    while (int result = NetDaemonNet.Net::get(&v4, &v8)) {
        process_message(reinterpret_cast<char *>(result), v8, v4);
    }

    synch(0x8b, 0, 0, 0, 0, 1, 0x2101);
    while (int result = NetDaemonNet.Net::get(&v4, &v8)) {
        process_message(reinterpret_cast<char *>(result), v8, v4);
    }

    synch(0x8c, 0, 0, 0, 0, 1, 0x2101);
    while (int result = NetDaemonNet.Net::get(&v4, &v8)) {
        process_message(reinterpret_cast<char *>(result), v8, v4);
    }

    synch(0x8d, 0, 0, 0, 0, 1, 0x2101);
    while (int result = NetDaemonNet.Net::get(&v4, &v8)) {
        process_message(reinterpret_cast<char *>(result), v8, v4);
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

    cs = checksum((char *)&GameRules, 4, 0);
    cs = checksum((char *)&VehCurrentCount, 0x10, cs);
    cs = checksum((char *)&LockEnableMask, 2, cs);
    cs = checksum((char *)&SecretProject, 0x100, cs);

    {
        unsigned char *bitTable = (unsigned char *)GameTechAchieved;
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
        char *p;
        for (p = (char *)((char *)PlayersData + 0x2498); p < (char *)((char *)Bases + 0x3CC); p += 0x20cc) {
            unsigned int masked1[8];
            unsigned int masked2[8];
            int idx4;
            cs = checksum(p - 4, 4, cs);
            cs = checksum(p, 4, cs);
            cs = checksum(p + 0x98, 4, cs);
            cs = checksum(p - 0x1ac, 4, cs);
            cs = checksum(p - 0x1a4, 0x10, cs);
            cs = checksum(p + 0x43c, 0x10, cs);
            for (idx4 = 0; idx4 < 8; ++idx4) {
                masked1[idx4] = *(unsigned int *)(p - 0x3b4 + idx4 * 4) & 0xb1d3c7f;
                masked2[idx4] = *(unsigned int *)(p - 0x394 + idx4 * 4) & 0x1e68;
            }
            cs = checksum((char *)masked1, 0x20, cs);
            cs = checksum((char *)masked2, 0x20, cs);
            cs = checksum(p - 0x2a8, 4, cs);
        }
    }
    *out++ = cs;
    cs = 0;

    {
        int countA = MapArea;
        if (countA > 0) {
            char *recBase = (char *)map_tile_bytes();
            char *p2 = recBase + 8;
            int idxA = 0;
            do {
                cs = checksum(recBase, 1, cs);
                cs = checksum(p2 - 5, 1, cs);
                cs = checksum(p2, 4, cs);
                cs = checksum(p2 - 7, 1, cs);
                cs = checksum(p2 - 1, 1, cs);
                cs = checksum(p2 + 4, 1, cs);
                countA = MapArea;
                recBase += 0x2c;
                p2 += 0x2c;
                idxA++;
            } while (idxA < countA);
        }
    }
    *out++ = cs;
    cs = 0;

    {
        int idx2;
        for (idx2 = 0; idx2 < 8; ++idx2) {
            int countB = *(char *)&VehCurrentCount;
            if (countB > 0) {
                char *p3 = (char *)((char *)Vehs + 2);
                int idxB = 0;
                do {
                    if ((unsigned char)p3[0xc] == idx2) {
                        cs = checksum(p3 - 2, 2, cs);
                        cs = checksum(p3, 2, cs);
                        cs = checksum(p3 + 8, 2, cs);
                        cs = checksum(p3 + 0xc, 1, cs);
                        cs = checksum(p3 + 0xe, 1, cs);
                        cs = checksum(p3 + 0x22, 1, cs);
                        cs = checksum(p3 + 0x26, 1, cs);
                        cs = checksum(p3 + 0x2c, 2, cs);
                        countB = *(char *)&VehCurrentCount;
                    }
                    p3 += 0x34;
                    idxB++;
                } while (idxB < countB);
            }
        }
    }
    *out++ = cs;
    cs = 0;

    {
        int countC = BaseCurrentCount;
        if (countC > 0) {
            char *p4 = (char *)((char *)Bases + 0x3C);
            int idxC = 0;
            do {
                cs = checksum(p4 - 0x3c, 7, cs);
                cs = checksum(p4, 4, cs);
                cs = checksum(p4 + 4, 4, cs);
                cs = checksum(p4 + 0x10, 4, cs);
                cs = checksum(p4 + 0x14, 4, cs);
                cs = checksum(p4 + 0x3c, 4, cs);
                cs = checksum(p4 + 0x48, 8, cs);
                cs = checksum(p4 + 0x50, 9, cs);
                idxC++;
                p4 += 0x134;
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
                    char *p5 = (char *)(map_tile_bytes() + (*MapRowStride) * recIdx * 0x2c);
                    char *p6 = p5 + 8;
                    int idxE;
                    for (idxE = 0; idxE < innerCount; ++idxE) {
                        cs = checksum(p5, 1, cs);
                        cs = checksum(p6 - 5, 1, cs);
                        cs = checksum(p6, 4, cs);
                        cs = checksum(p6 - 7, 1, cs);
                        cs = checksum(p6 - 1, 1, cs);
                        p5 += 0x2c;
                        p6 += 0x2c;
                    }
                }
            }
            *out++ = cs;
        }
    }

    if (BaseAtDebugPopupFlag != 0) {
        cs = checksum((char *)&GameRules, 4, 0);
        cs = checksum((char *)&VehCurrentCount, 0x10, cs);
        *out++ = checksum((char *)&LockEnableMask, 2, cs);
        *out++ = checksum((char *)&SecretProject, 0x100, 0);
        cs = 0;

        {
            unsigned char *bitTable = (unsigned char *)GameTechAchieved;
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
            char *p;
            for (p = (char *)((char *)PlayersData + 0x2498); p < (char *)((char *)Bases + 0x3CC); p += 0x20cc) {
                cs = checksum(p - 4, 4, 0);
                cs = checksum(p, 4, cs);
                cs = checksum(p + 0x98, 4, cs);
                cs = checksum(p - 0x1ac, 4, cs);
                cs = checksum(p - 0x1a4, 0x10, cs);
                cs = checksum(p + 0x43c, 0x10, cs);
                cs = checksum(p - 0x3b4, 0x20, cs);
                cs = checksum(p - 0x394, 0x20, cs);
                cs = checksum(p - 0x374, 0x20, cs);
                cs = checksum(p - 0x314, 0x20, cs);
                cs = checksum(p - 0x334, 0x20, cs);
                cs = checksum(p - 0x354, 0x20, cs);
                cs = checksum(p - 0x280, 0x20, cs);
                cs = checksum(p - 0x2a8, 4, cs);
                *out++ = cs;
            }
        }
        cs = 0;

        {
            int cnt1 = MapArea;
            char *pL = (char *)map_tile_bytes();
            int i1;
            for (i1 = 0; i1 < cnt1; ++i1) {
                cs = checksum(pL, 1, cs);
                cnt1 = MapArea;
                pL += 0x2c;
            }
        }
        *out++ = cs;
        cs = 0;

        {
            int cnt2 = MapArea;
            char *pL = (char *)map_tile_bytes() + 3;
            int i2;
            for (i2 = 0; i2 < cnt2; ++i2) {
                cs = checksum(pL, 1, cs);
                cnt2 = MapArea;
                pL += 0x2c;
            }
        }
        *out++ = cs;
        cs = 0;

        {
            int cnt3 = MapArea;
            char *pL = (char *)map_tile_bytes() + 8;
            int i3;
            for (i3 = 0; i3 < cnt3; ++i3) {
                cs = checksum(pL, 4, cs);
                cnt3 = MapArea;
                pL += 0x2c;
            }
        }
        *out++ = cs;
        cs = 0;

        {
            int cnt4 = MapArea;
            char *pL = (char *)map_tile_bytes() + 1;
            int i4;
            for (i4 = 0; i4 < cnt4; ++i4) {
                cs = checksum(pL, 1, cs);
                cnt4 = MapArea;
                pL += 0x2c;
            }
        }
        *out++ = cs;
        cs = 0;

        {
            int cnt5 = MapArea;
            char *pL = (char *)map_tile_bytes() + 7;
            int i5;
            for (i5 = 0; i5 < cnt5; ++i5) {
                cs = checksum(pL, 1, cs);
                cnt5 = MapArea;
                pL += 0x2c;
            }
        }
        *out++ = cs;
        cs = 0;

        {
            int cnt6 = MapArea;
            char *pL = (char *)map_tile_bytes() + 0xc;
            int i6;
            for (i6 = 0; i6 < cnt6; ++i6) {
                cs = checksum(pL, 1, cs);
                cnt6 = MapArea;
                pL += 0x2c;
            }
        }
        *out++ = cs;

        *out++ = checksum((char *)&MapSizePlanet, 0x1c, 0);

        cs = 0;
        {
            int idx2b;
            for (idx2b = 0; idx2b < 8; ++idx2b) {
                int countB2 = *(char *)&VehCurrentCount;
                cs = 0;
                if (countB2 > 0) {
                    char *p3b = (char *)Vehs;
                    int idxBb = 0;
                    do {
                        if ((unsigned char)p3b[0xe] == idx2b) {
                            cs = checksum(p3b, 2, cs);
                            cs = checksum(p3b + 2, 2, cs);
                            cs = checksum(p3b + 0xa, 3, cs);
                            cs = checksum(p3b + 0x24, 1, cs);
                            cs = checksum(p3b + 0x28, 1, cs);
                            cs = checksum(p3b + 0x2e, 2, cs);
                            countB2 = *(char *)&VehCurrentCount;
                        }
                        p3b += 0x34;
                        idxBb++;
                    } while (idxBb < countB2);
                }
            }
        }
        *out++ = cs;

        cs = 0;
        {
            int countC2 = BaseCurrentCount;
            if (countC2 > 0) {
                char *p4b = (char *)((char *)Bases + 0x3C);
                int idxC2 = 0;
                do {
                    cs = checksum(p4b - 0x3c, 7, cs);
                    cs = checksum(p4b, 4, cs);
                    cs = checksum(p4b + 4, 4, cs);
                    cs = checksum(p4b + 0x10, 4, cs);
                    cs = checksum(p4b + 0x14, 4, cs);
                    cs = checksum(p4b + 0x3c, 4, cs);
                    cs = checksum(p4b + 0x48, 8, cs);
                    cs = checksum(p4b + 0x50, 9, cs);
                    idxC2++;
                    p4b += 0x134;
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
    if (lock_state_[0].leading_words_[1] == 0) {
        lock_state_[0].leading_words_[0] = turn_value;
    }
    if (faction_id == ConsoleGlobal.faction_id_) {
        if (lock_state_[2].trailing_words_[2] != 0) {
            log_say("Remaining Locked2", 0, 0, 0);
            lock_state_[2].trailing_words_[2] = 0;
            message_data(0x2214, 0, 0, 0, 0, 0);
            return 0;
        }
        lock_state_[2].leading_words_[3] = 0;
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
    message.turn_ = NetDaemonNet.lock_state_[0].leading_words_[0];
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
    message.turn_ = NetDaemonNet.lock_state_[0].leading_words_[0];
    message.payload_10_ = payload_10;
    strcpy(message.text_, text);

    NetBusyCount++;
    NetDaemonNet.send_message(reinterpret_cast<char *>(&message), 0x2c,
                              target);
    NetBusyCount--;
}

// 0x00592EE0 ?message_data@@YAXHHHHHH@Z is the one member of this family
// that stays a pending_bodies forwarder, and the reason is its return value.
// The catalogue spells it void, but EAX is live at its ret - it is whatever
// Net::send left there, two calls down - and unlock_veh (0x005310F0,
// BYTE_EXACT) returns exactly that residue. A body spelled here would have to
// name a value no source expression reaches: the last EAX-writing event is
// inside send_message, after which the image only decrements NetBusyCount.
// TRIED: a uint32_t body with no return statement - VC6 makes that C4716, a
// hard error. TRIED: returning NetBusyCount - adds a load the image has not
// got. The forwarder is the only spelling that passes the callee's EAX
// through, so it stays.
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
    unsigned char message[0x90];

    *reinterpret_cast<short *>(message + 0x00) = static_cast<short>(opcode);
    *reinterpret_cast<int *>(message + 0x04) = ConsoleGlobal.faction_id_;
    *reinterpret_cast<unsigned int *>(message + 0x08) = timeGetTime();
    *reinterpret_cast<int *>(message + 0x0c) =
        NetDaemonNet.lock_state_[0].leading_words_[0];
    *reinterpret_cast<int *>(message + 0x10) = payload_10;
    *reinterpret_cast<int *>(message + 0x14) = payload_14;
    *reinterpret_cast<int *>(message + 0x18) = payload_18;
    *reinterpret_cast<int *>(message + 0x1c) = payload_1C;
    *reinterpret_cast<int *>(message + 0x20) = payload_20;
    *reinterpret_cast<int *>(message + 0x24) = payload_24;
    *reinterpret_cast<int *>(message + 0x28) = payload_28;
    *reinterpret_cast<int *>(message + 0x2c) = payload_2C;

    NetBusyCount++;
    NetDaemonNet.send_message(reinterpret_cast<char *>(message), 0x90, target);
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
    struct Message {
        short opcode_;
        int faction_;
        unsigned long time_;
        int turn_;
        int data_[32];
    } message;
    int filled;
    int i;
    int *dst;

    message.opcode_ = static_cast<short>(opcode);
    message.faction_ = ConsoleGlobal.faction_id_;
    message.time_ = timeGetTime();
    message.turn_ = NetDaemonNet.lock_state_[0].leading_words_[0];

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
    message.turn_ = NetDaemonNet.lock_state_[0].leading_words_[0];
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
// The image stores the payload dwords out of order - 0x1C first, then the
// stamp block, then 0x14 and 0x18 - and that order is kept.
void __cdecl message_landmark(int opcode, int target, char *text,
                              int payload_10, int payload_14, int payload_18,
                              int payload_1C) {
    char message[0x40];

    *reinterpret_cast<short *>(message) = static_cast<short>(opcode);
    *reinterpret_cast<int *>(message + 0x04) = ConsoleGlobal.faction_id_;
    *reinterpret_cast<unsigned long *>(message + 0x08) = timeGetTime();
    *reinterpret_cast<int *>(message + 0x1c) = payload_1C;
    *reinterpret_cast<int *>(message + 0x0c) =
        NetDaemonNet.lock_state_[0].leading_words_[0];
    *reinterpret_cast<int *>(message + 0x14) = payload_14;
    *reinterpret_cast<int *>(message + 0x18) = payload_18;
    *reinterpret_cast<int *>(message + 0x10) = payload_10;
    if (text) {
        strcpy(message + 0x20, text);
    }

    NetBusyCount++;
    NetDaemonNet.send_message(message, 0x40, target);
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
    message.turn_ = NetDaemonNet.lock_state_[0].leading_words_[0];
    strncpy(message.text_, text, 0x100);

    NetBusyCount++;
    message.text_[0xff] = 0;
    NetDaemonNet.send_message(reinterpret_cast<char *>(&message), 0x110,
                              target);
    NetBusyCount--;
}

// Fixed-slot bindings carried from 00532940.cpp

// ORIGINAL: 0x00532940 ?send_message@NetDaemon@@QAEXPADKH@Z 0x00532940-0x00532A44 FILE BYTE_EXACT
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
void NetDaemon::send_message(char *message, unsigned long length, int target) {
    if (IsMultiplayerNet) {
        unsigned short flags = *reinterpret_cast<unsigned short *>(message);
        if (flags & 0x4000) {
            *reinterpret_cast<unsigned long *>(message + 8) =
                timeGetTime();
            Net::send(message, length, 0, 1);
            return;
        }
        if (flags & 0x8000) {
            int value;
            if (NetDaemonNet.lock_state_[0].trailing_words_[0] == 1) {
                value = target;
            } else {
                value = field_764_;
            }
            target = value;
            Net::send(message, length, target, 1);
            return;
        }
        if (flags & 0xa000) {
            int counter = NetDaemonNet.lock_state_[0].trailing_words_[0];
            if ((NetTurnFlags & 0x10) &&
                ConsoleControlTurnActive == 0 && NetTurnFaction > 0 && counter > 1) {
                Net::send(message, length,
                          AlphaNet::who_2_pid(NetTurnFaction), 1);
                return;
            }
            int value;
            if (counter == 1) {
                value = target;
            } else {
                value = field_764_;
            }
            target = value;
            Net::send(message, length, target, 1);
            return;
        }
        int value;
        if (NetDaemonNet.lock_state_[0].trailing_words_[0] == 1) {
            value = target;
        } else {
            value = AlphaNet::who_2_pid(target);
        }
        target = value;
        Net::send(message, length, target, 1);
    }
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
    if ((NetDaemonNet.lock_state_[0].trailing_words_[7] & (1 << ConsoleGlobal.faction_id_)) != 0 &&
        (NetTurnFlags & 0x10) == 0) {
        NetMsgPrimary.pop("ALREADYDONE", 0, 2, 0);
        return 1;
    }

    lock_state_[1].payload_.vehicle_selection_follow = veh_id;
    int result = lock(target, Vehs[*veh_id].x, Vehs[*veh_id].y, payload_18,
                      payload_1C, payload_20);
    if (result == 0 && *veh_id >= 0) {
        lock_state_[2].leading_words_[3] = 1;
        lock_state_[2].trailing_words_[2] = 0;
        return 0;
    }
    lock_state_[1].payload_.vehicle_selection_follow = 0;
    return 1;
}

// ORIGINAL: 0x00531150 ?automated_lock@NetDaemon@@QAEHPAHH@Z 0x00531150-0x005312F6
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/00531150.cpp; image:0x00531150-0x005312F6", "Native5311A8 owns this+1BB4 in existing lock_state_[2].dword_[4]; warning5311EF uses initial time while polling531230 uses moving retry time."], "reason": "Recover automated vehicle locking through the genuine NetDaemon owner: native calls identify vehicle-ID pointer forwarding, separate initial-warning and retry timers, message pumping and balanced busy/state lifetimes."}
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
    lock_state_[2].payload_.flags = 1;
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
                    const int message = NetDaemonNet.Net::get(&first, &second);
                    if (message != 0) {
                        process_message(reinterpret_cast<char *>(message),
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
    lock_state_[2].payload_.flags = 0;
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

    lock_state_[0].leading_words_[1] = 0;
    lock_state_[0].leading_words_[2] = 0;
    lock_state_[0].leading_words_[3] = 0;
    lock_state_[0].payload_.flags = 0;
    lock_state_[0].trailing_words_[8] = 0;
    lock_state_[0].trailing_words_[7] = 0;
    lock_state_[1].leading_words_[1] = 0;
    lock_state_[1].leading_words_[0] = 0;
    lock_state_[1].trailing_words_[0] = 0;
    lock_state_[2].leading_words_[1] = 0;
    lock_state_[1].trailing_words_[9] = 0;
    lock_state_[2].leading_words_[0] = 0;
    lock_state_[2].trailing_words_[5] = 0;
    lock_state_[2].trailing_words_[4] = 0;
    lock_state_[0].trailing_words_[4] = 0;
    lock_state_[0].trailing_words_[5] = 0;
    lock_state_[0].trailing_words_[6] = 0;
    lock_state_[2].trailing_words_[3] = 0;
    lock_state_[2].trailing_words_[2] = 0;
    lock_state_[2].leading_words_[3] = 0;
    lock_state_[2].leading_words_[2] = 0;
    lock_state_[2].payload_.flags = 0;
    lock_state_[0].trailing_words_[0] = 0;
    lock_state_[0].trailing_words_[1] = 0;
    lock_state_[2].trailing_words_[8] = 0;
    lock_state_[2].trailing_words_[7] = 0;
    lock_state_[0].trailing_words_[2] = 0;
    lock_state_[0].trailing_words_[3] = 0;
    lock_state_[2].trailing_words_[6] = 0xff;
    lock_state_[1].payload_.vehicle_selection_follow = 0;

    uint32_t *p = &lock_state_[1].trailing_words_[1];
    for (int i = 8; i != 0; i--) {
        *p = 0;
        p++;
    }

    locks_.clear();
    AlphaNet::close();

    lock_state_[0].payload_.flags = 0;
    lock_state_[0].leading_words_[3] = 0;

    if (AlphaNet::setup(option)) {
        return 1;
    }
    else {
        return 0;
    }
}

// Fixed-slot bindings carried from 005303b0.cpp

// ORIGINAL: 0x005303B0 ?wait@NetDaemon@@QAEHPADPAHHH@Z 0x005303B0-0x00530493 FILE
// working copy - scaffold materialised by --work
// size      227 bytes
// prototype int (__thiscall ?wait@NetDaemon@@QAEHPADPAHHH@Z)(NetDaemon* this, int8*, int*, int, int)
// callers   2   call targets   7
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0047A7D0 0x0047A890 0x0051F1B0 0x00534400 0x005901D0 0x005FC700 0x00630A00
// indirect  0x005303CF 0x005303F9

int NetDaemon::wait(char * message, int * state, int value, int timeout) {
    int saved = NetDaemonNet.lock_state_[1].leading_words_[3];
    ++NetBusyCount;
    DWORD (WINAPI *const timeGetTimeFn)() = timeGetTime;
    unsigned long startTime = timeGetTimeFn();
    bool triggeredTimeout = false;
    while (*state == value) {
        if (ExitTurnLoop != 0) {
            break;
        }
        if (!triggeredTimeout) {
            unsigned long now = timeGetTimeFn();
            if ((int)(now - startTime) > timeout) {
                triggeredTimeout = true;
                NetMsgPrimary.pop(message, 0, 2, 0);
            }
        }
        wait_task();
        unsigned long outVar;
        int result = NetDaemonNet.Net::get(reinterpret_cast<unsigned long *>(&message), &outVar);
        if (result != 0) {
            process_message(reinterpret_cast<char *>(result), outVar, reinterpret_cast<int>(message));
        }
        locks_.check_global();
        check_spock();
    }
    if (triggeredTimeout) {
        NetMsgPrimary.close();
    }
    --NetBusyCount;
    NetDaemonNet.lock_state_[1].leading_words_[3] = saved;
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
  if (lock_state_[1].trailing_words_[9] != 0) {
    return 1;
  }

  log_say(reinterpret_cast<char *>("Client requesting Diplo Channel"), ConsoleGlobal.faction_id_, 0, 0);
  lock_state_[1].trailing_words_[9] = 1;
  lock_state_[2].leading_words_[0] = 0;

  message_data(0x8240, 0, payload_10, payload_14, 0, 0);

  DWORD (WINAPI *const timeGetTime_)() = timeGetTime;
  int savedE904 = NetDaemonNet.lock_state_[1].leading_words_[3];
  (NetBusyCount)++;
  payload_14 = static_cast<int>(timeGetTime_());

  bool sentAwait = false;
  if (lock_state_[2].leading_words_[0] == 0) {
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
      int gotten = NetDaemonNet.Net::get(reinterpret_cast<unsigned long *>(&payload_10), &local4);
      if (gotten != 0) {
        process_message(reinterpret_cast<char *>(gotten), local4, payload_10);
      }
      locks_.check_global();
      check_spock();

      if (lock_state_[2].leading_words_[0] != 0) {
        break;
      }
    }
    if (sentAwait) {
      NetMsgPrimary.close();
    }
  }

  (NetBusyCount)--;
  NetDaemonNet.lock_state_[1].leading_words_[3] = savedE904;

  if (lock_state_[2].leading_words_[0] < 2) {
    return 0;
  }
  lock_state_[1].trailing_words_[9] = 0;
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

    if (*(int *)(lock_state_[2].leading_words_[2]) != 0) {
        if (*(int *)(lock_state_[2].trailing_words_[3]) != 0) {
            return 1;
        }
        NetMsgPrimary.pop((const char *)"MOVEPENDING", 0, 2, (const char *)0);
        return 1;
    }

    *(int *)(lock_state_[2].leading_words_[2]) = 1;
    *(int *)(lock_state_[1].leading_words_[2]) = 0;
    *(int *)(locks_.field_E8_) = 0;

    log_say((char *)"Client requesting lock", ConsoleGlobal.faction_id_, x, y);
    message_lock(0x2210, target, x, y, payload_18, payload_1C, scratch, 0);

    NetBusyCount = NetBusyCount + 1;
    scratch = NetDaemonNet.lock_state_[1].leading_words_[3];
    unsigned long t0 = timeGetTime();
    int waited = 0;
    if (*(int *)(lock_state_[1].leading_words_[2]) == 0) {
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
            int got = NetDaemonNet.Net::get((unsigned long *)&y, (unsigned long *)&x);
            if (got != 0) {
                this->process_message((char *)got, (unsigned long)x, y);
            }
            locks_.check_global();
            check_spock();
        } while (*(int *)(lock_state_[1].leading_words_[2]) == 0);
        if (waited) {
            NetMsgPrimary.close();
        }
    }

    NetBusyCount = NetBusyCount - 1;
    NetDaemonNet.lock_state_[1].leading_words_[3] = scratch;

    if (*(int *)(lock_state_[1].leading_words_[2]) == 1) {
        *(int *)(lock_state_[2].trailing_words_[4]) = 0;
        if ((unsigned char)target & 2) {
            NetBusyCount = NetBusyCount + 1;
            scratch = NetDaemonNet.lock_state_[1].leading_words_[3];
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
                    int got2 = NetDaemonNet.Net::get((unsigned long *)&y, (unsigned long *)&x);
                    if (got2 != 0) {
                        this->process_message((char *)got2, (unsigned long)x, y);
                    }
                    locks_.check_global();
                    check_spock();
                } while (*(int *)(locks_.field_E8_) == 0);
                if (waited2) {
                    NetMsgPrimary.close();
                }
            }
            NetDaemonNet.lock_state_[1].leading_words_[3] = scratch;
            NetBusyCount = NetBusyCount - 1;
        }
        if (NetPlayerDropped == 0) {
            return 0;
        }
        *(int *)(lock_state_[1].leading_words_[2]) = 2;
    }

    if (*(int *)(lock_state_[2].trailing_words_[3]) == 0) {
        *(int *)(lock_state_[2].trailing_words_[4]) = *(int *)(lock_state_[2].trailing_words_[4]) + 1;
        if (*(int *)(lock_state_[2].trailing_words_[4]) > 0 || *(int *)(lock_state_[1].leading_words_[2]) > 2) {
            const char *msg = (*(int *)(lock_state_[1].leading_words_[2]) == 2) ? (const char *)"NOLOCK" : (const char *)"NOLOCKGLOBAL";
            NetMsgPrimary.pop(msg, 5000, 2, (const char *)0);
            *(int *)(lock_state_[2].trailing_words_[4]) = 0;
        }
    }

    *(int *)(lock_state_[2].leading_words_[2]) = 0;
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
    if (lock_state_[2].leading_words_[2] == 0) {
        return 1;
    }

    log_say(reinterpret_cast<char *>("Client requesting ADD LOCK"), ConsoleGlobal.faction_id_, 0, 0);
    lock_state_[1].leading_words_[2] = 0;
    message_lock(0x2215, target, x, y, 0, -1, -1, 0);

    int savedVal = NetDaemonNet.lock_state_[1].leading_words_[3];
    DWORD (WINAPI *const timeGetTimeFn)() = timeGetTime;
    ++NetBusyCount;
    y = timeGetTimeFn();

    bool popped = false;
    NetMsg *netMsg = &NetMsgPrimary;
    Lock &lock = locks_;

    if (lock_state_[1].leading_words_[2] == 0) {
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
            unsigned long msgResult = NetDaemonNet.Net::get(reinterpret_cast<unsigned long *>(&x),
                                                reinterpret_cast<unsigned long *>(&target));
            if (msgResult != 0) {
                process_message(reinterpret_cast<char *>(msgResult),
                                 static_cast<unsigned long>(target), x);
            }
            lock.check_global();
            check_spock();
        } while (lock_state_[1].leading_words_[2] == 0);
        if (popped) {
            netMsg->close();
        }
    }

    --NetBusyCount;
    NetDaemonNet.lock_state_[1].leading_words_[3] = savedVal;
    if (lock_state_[1].leading_words_[2] != 1) {
        netMsg->pop(reinterpret_cast<const char *>("NOLOCK"), 5000, 2, 0);
    }
    return lock_state_[1].leading_words_[2] != 1;
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
    lock_state_[1].leading_words_[2] = 0;
    locks_.field_E8_ = 0;
    message_data(0x2211, 0, 0, 0, 0, 0);
    NetBusyCount = NetBusyCount + 1;
    int saved_e904 = NetDaemonNet.lock_state_[1].leading_words_[3];
    unsigned long start_time1 = get_time();

    bool warned1 = false;
    if (lock_state_[1].leading_words_[2] == 0) {
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
            int result = NetDaemonNet.Net::get(&out_id, &out_len);
            if (result != 0) {
                process_message(reinterpret_cast<char *>(result), out_len, static_cast<int>(out_id));
            }
            locks_.check_global();
            check_spock();
        } while (lock_state_[1].leading_words_[2] == 0);
        if (warned1) {
            NetMsgPrimary.close();
        }
    }
    NetDaemonNet.lock_state_[1].leading_words_[3] = saved_e904;
    int newE664 = NetBusyCount - 1;

    if (lock_state_[1].leading_words_[2] == 1) {
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
                int result2 = NetDaemonNet.Net::get(&out_id2, &out_len2);
                if (result2 != 0) {
                    process_message(reinterpret_cast<char *>(result2), out_len2, static_cast<int>(out_id2));
                }
                locks_.check_global();
                check_spock();
            } while (locks_.field_E8_ == 0);
            if (warned2) {
                NetMsgPrimary.close();
            }
        }
        NetDaemonNet.lock_state_[1].leading_words_[3] = saved_e904;
        NetBusyCount = NetBusyCount - 1;
        if (NetPlayerDropped == 0) {
            lock_state_[2].trailing_words_[4] = 0;
            return 0;
        }
        lock_state_[1].leading_words_[2] = 2;
        newE664 = NetBusyCount;
    }
    NetBusyCount = newE664;

    if (lock_state_[2].trailing_words_[3] == 0) {
        int newBc8 = lock_state_[2].trailing_words_[4] + 1;
        lock_state_[2].trailing_words_[4] = newBc8;
        if (newBc8 > 0 || lock_state_[1].leading_words_[2] > 2) {
            const char *msg = reinterpret_cast<const char *>("NOLOCK");
            if (lock_state_[1].leading_words_[2] != 2) {
                msg = reinterpret_cast<const char *>("NOLOCKGLOBAL");
            }
            NetMsgPrimary.pop(msg, 5000, 2, 0);
            lock_state_[2].trailing_words_[4] = 0;
        }
    }
    lock_state_[2].leading_words_[2] = 0;
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

    log_say(reinterpret_cast<char *>("UnlockBase"), NetDaemonNet.lock_state_[2].leading_words_[2], base_id, 0);

    if (IsMultiplayerNet != 0 && NetDaemonNet.lock_state_[2].leading_words_[2] != 0) {
        log_say(reinterpret_cast<char *>("UnlockBase doing synch & unlock"), 0, 0, 0);
        synch_base(base_id);

        if (IsMultiplayerNet != 0) {
            lock_state_[2].trailing_words_[0] = 1;
            log_say(reinterpret_cast<char *>("Awaiting acknowledgement of SYNCH"), ConsoleGlobal.faction_id_, 0, 0);
            int saved = NetDaemonNet.lock_state_[1].leading_words_[3];
            ++NetBusyCount;
            unsigned long baseline = timeGetTime();

            int logged_wait = 0;
            if (lock_state_[2].trailing_words_[0] == 1) {
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
                    int r = NetDaemonNet.Net::get(&p1, &p2);
                    if (r != 0) {
                        process_message(reinterpret_cast<char *>(r), p2, p1);
                    }
                    locks_.check_global();
                    check_spock();
                } while (lock_state_[2].trailing_words_[0] == 1);
                if (logged_wait) {
                    NetMsgPrimary.close();
                }
            }

            --NetBusyCount;
            NetDaemonNet.lock_state_[1].leading_words_[3] = saved;
            if (lock_state_[2].trailing_words_[0] == 0) {
                log_say(reinterpret_cast<char *>("Received SYNCH acknowledgement"), ConsoleGlobal.faction_id_, 0, 0);
            }
            lock_state_[2].trailing_words_[0] = 0;

            if (IsMultiplayerNet != 0) {
                log_say(reinterpret_cast<char *>("Client releasing lock"), ConsoleGlobal.faction_id_, 0, 0);
                message_data(0x2212, 0, 0, 0, 0, 0);
                lock_state_[2].leading_words_[3] = 0;
                lock_state_[2].leading_words_[2] = 0;
                lock_state_[2].trailing_words_[5] = 0;
                lock_state_[2].trailing_words_[4] = 0;
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
    int saved = NetDaemonNet.lock_state_[1].leading_words_[3];
    ++NetBusyCount;
    unsigned long baseline = timeGetTime();

    int logged_wait = 0;
    if (lock_state_[2].leading_words_[3] == 1) {
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
            int r = NetDaemonNet.Net::get(&p1, &p2);
            if (r != 0) {
                process_message(reinterpret_cast<char *>(r), p2, p1);
            }
            locks_.check_global();
            check_spock();
        } while (lock_state_[2].leading_words_[3] == 1);
        if (logged_wait) {
            NetMsgPrimary.close();
        }
    }

    --NetBusyCount;
    NetDaemonNet.lock_state_[1].leading_words_[3] = saved;
    log_say(reinterpret_cast<char *>("Client received acknowledgement"), ConsoleGlobal.faction_id_, 0, 0);

    if (opcode != 0) {
        if (IsMultiplayerNet != 0) {
            log_say(reinterpret_cast<char *>("Client releasing lock"), ConsoleGlobal.faction_id_, 0, 0);
            message_data(0x2212, 0, 0, 0, 0, 0);
            lock_state_[2].leading_words_[3] = 0;
            lock_state_[2].leading_words_[2] = 0;
            lock_state_[2].trailing_words_[5] = 0;
            lock_state_[2].trailing_words_[4] = 0;
        }
        lock_state_[1].payload_.vehicle_selection_follow = 0;
        lock_state_[2].trailing_words_[3] = 0;
    }
    lock_state_[2].leading_words_[3] = 0;
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
    uint32_t *state = &lock_state_[2].trailing_words_[0];

    if (IsMultiplayerNet != 0) {
        int synching = 1;
        *state = synching;
        log_say(reinterpret_cast<char *>("Awaiting acknowledgement of SYNCH"), ConsoleGlobal.faction_id_, 0, 0);

        int saved = NetDaemonNet.lock_state_[1].leading_words_[3];
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
                int r = NetDaemonNet.Net::get(&v1, &v2);
                if (r != 0) {
                    process_message(reinterpret_cast<char *>(r), v2,
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
        NetDaemonNet.lock_state_[1].leading_words_[3] = saved;
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
        lock_state_[2].leading_words_[1] = opcode;
        log_say(reinterpret_cast<char *>("Awaiting DIPLO event"), ConsoleGlobal.faction_id_, 0, 0);
        (NetBusyCount)++;
        int32_t saved = NetDaemonNet.lock_state_[1].leading_words_[3];
        unsigned int start_time = timeGetTime();
        int timed_out = 0;
        if (lock_state_[2].leading_words_[1] == opcode) {
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
                int result = NetDaemonNet.Net::get(&v_at_4, &v_at_8);
                if (result != 0) {
                    process_message(reinterpret_cast<char *>(result), v_at_8, v_at_4);
                }
                locks_.check_global();
                check_spock();
            } while (lock_state_[2].leading_words_[1] == opcode);
            if (timed_out) {
                NetMsgPrimary.close();
            }
        }
        (NetBusyCount)--;
        NetDaemonNet.lock_state_[1].leading_words_[3] = saved;
        if (lock_state_[2].leading_words_[1] == 0) {
            log_say(reinterpret_cast<char *>("Received DIPLO acknowledgement"), ConsoleGlobal.faction_id_, 0, 0);
        }
        lock_state_[2].leading_words_[1] = 0;
    }
}

class DeletionList { public:
    uint32_t field_0_;
    uint32_t field_4_;
    uint32_t field_8_;
    uint32_t field_C_;
    uint32_t field_10_;
    uint32_t field_14_;
    uint32_t field_18_;
    uint32_t field_1C_;
    uint32_t field_20_;
    uint32_t field_24_;
    uint32_t field_28_;
    uint32_t field_2C_;
    uint32_t field_30_;
    uint32_t field_34_;
    uint32_t field_38_;
    uint32_t field_3C_;
    uint32_t field_40_;
    uint32_t field_44_;
    uint32_t field_48_;
    uint32_t field_4C_;
    uint32_t field_50_;
    uint32_t field_54_;
    uint32_t field_58_;
    uint32_t field_5C_;
    uint32_t field_60_;
    uint32_t field_64_;
    uint32_t field_68_;
    uint32_t field_6C_;
    uint32_t field_70_;
    uint32_t field_74_;
    uint32_t field_78_;
    uint32_t field_7C_;
    uint32_t field_80_;
    uint32_t field_84_;
    uint32_t field_88_;
    uint32_t field_8C_;
    uint32_t field_90_;
    uint32_t field_94_;
    uint32_t field_98_;
    uint32_t field_9C_;
    uint32_t field_A0_;
    uint32_t field_A4_;
    uint32_t field_A8_;
    uint32_t field_AC_;
    uint32_t field_B0_;
    uint32_t field_B4_;
    uint32_t field_B8_;
    uint32_t field_BC_;
    uint32_t field_C0_;
    uint32_t field_C4_;
    uint32_t field_C8_;
    uint32_t field_CC_;
    uint32_t field_D0_;
    uint32_t field_D4_;
    uint32_t field_D8_;
    uint32_t field_DC_;
    uint32_t field_E0_;
    uint32_t field_E4_;
    uint32_t field_E8_;
    uint32_t field_EC_;
    uint32_t field_F0_;
    uint32_t field_F4_;
    uint32_t field_F8_;
    uint32_t field_FC_;
    uint32_t field_100_;
    uint32_t field_104_;
    uint32_t field_108_;
    uint32_t field_10C_;
    uint32_t field_110_;
    uint32_t field_114_;
    uint32_t field_118_;
    uint32_t field_11C_;
    uint32_t field_120_;
    uint32_t field_124_;
    uint32_t field_128_;
    uint32_t field_12C_;
    uint32_t field_130_;
    uint32_t field_134_;
    uint32_t field_138_;
    uint32_t field_13C_;
    uint32_t field_140_;
    uint32_t field_144_;
    uint32_t field_148_;
    uint32_t field_14C_;
    uint32_t field_150_;
    uint32_t field_154_;
    uint32_t field_158_;
    uint32_t field_15C_;
    uint32_t field_160_;
    uint32_t field_164_;
    uint32_t field_168_;
    uint32_t field_16C_;
    uint32_t field_170_;
    uint32_t field_174_;
    uint32_t field_178_;
    uint32_t field_17C_;
    uint32_t field_180_;
    uint32_t field_184_;
    uint32_t field_188_;
    uint32_t field_18C_;
    uint32_t field_190_;
    uint32_t field_194_;
    uint32_t field_198_;
    uint32_t field_19C_;
    uint32_t field_1A0_;
    uint32_t field_1A4_;
    uint32_t field_1A8_;
    uint32_t field_1AC_;
    uint32_t field_1B0_;
    uint32_t field_1B4_;
    uint32_t field_1B8_;
    uint32_t field_1BC_;
    uint32_t field_1C0_;
    uint32_t field_1C4_;
    uint32_t field_1C8_;
    uint32_t field_1CC_;
    uint32_t field_1D0_;
    uint32_t field_1D4_;
    uint32_t field_1D8_;
    uint32_t field_1DC_;
    uint32_t field_1E0_;
    uint32_t field_1E4_;
    uint32_t field_1E8_;
    uint32_t field_1EC_;
    uint32_t field_1F0_;
    uint32_t field_1F4_;
    uint32_t field_1F8_;
    uint32_t field_1FC_;
    uint32_t field_200_;
    uint32_t field_204_;
    uint32_t field_208_;
    uint32_t field_20C_;
    uint32_t field_210_;
    uint32_t field_214_;
    uint32_t field_218_;
    uint32_t field_21C_;
    uint32_t field_220_;
    uint32_t field_224_;
    uint32_t field_228_;
    uint32_t field_22C_;
    uint32_t field_230_;
    uint32_t field_234_;
    uint32_t field_238_;
    uint32_t field_23C_;
    uint32_t field_240_;
    uint32_t field_244_;
    uint32_t field_248_;
    uint32_t field_24C_;
    uint32_t field_250_;
    uint32_t field_254_;
    uint32_t field_258_;
    uint32_t field_25C_;
    uint32_t field_260_;
    uint32_t field_264_;
    uint32_t field_268_;
    uint32_t field_26C_;
    uint32_t field_270_;
    uint32_t field_274_;
    uint32_t field_278_;
    uint32_t field_27C_;
    uint32_t field_280_;
    uint32_t field_284_;
    uint32_t field_288_;
    uint32_t field_28C_;
    uint32_t field_290_;
    uint32_t field_294_;
    uint32_t field_298_;
    uint32_t field_29C_;
    uint32_t field_2A0_;
    uint32_t field_2A4_;
    uint32_t field_2A8_;
    uint32_t field_2AC_;
    uint32_t field_2B0_;
    uint32_t field_2B4_;
    uint32_t field_2B8_;
    uint32_t field_2BC_;
    uint32_t field_2C0_;
    uint32_t field_2C4_;
    uint32_t field_2C8_;
    uint32_t field_2CC_;
    uint32_t field_2D0_;
    uint32_t field_2D4_;
    uint32_t field_2D8_;
    uint32_t field_2DC_;
    uint32_t field_2E0_;
    uint32_t field_2E4_;
    uint32_t field_2E8_;
    uint32_t field_2EC_;
    uint32_t field_2F0_;
    uint32_t field_2F4_;
    uint32_t field_2F8_;
    uint32_t field_2FC_;
    uint32_t field_300_;
    uint32_t field_304_;
    uint32_t field_308_;
    uint32_t field_30C_;
    uint32_t field_310_;
    uint32_t field_314_;
    uint32_t field_318_;
    uint32_t field_31C_;
    uint32_t field_320_;
    uint32_t field_324_;
    uint32_t field_328_;
    uint32_t field_32C_;
    uint32_t field_330_;
    uint32_t field_334_;
    uint32_t field_338_;
    uint32_t field_33C_;
    uint32_t field_340_;
    uint32_t field_344_;
    uint32_t field_348_;
    uint32_t field_34C_;
    uint32_t field_350_;
    uint32_t field_354_;
    uint32_t field_358_;
    uint32_t field_35C_;
    uint32_t field_360_;
    uint32_t field_364_;
    uint32_t field_368_;
    uint32_t field_36C_;
    uint32_t field_370_;
    uint32_t field_374_;
    uint32_t field_378_;
    uint32_t field_37C_;
    uint32_t field_380_;
    uint32_t field_384_;
    uint32_t field_388_;
    uint32_t field_38C_;
    uint32_t field_390_;
    uint32_t field_394_;
    uint32_t field_398_;
    uint32_t field_39C_;
    uint32_t field_3A0_;
    uint32_t field_3A4_;
    uint32_t field_3A8_;
    uint32_t field_3AC_;
    uint32_t field_3B0_;
    uint32_t field_3B4_;
    uint32_t field_3B8_;
    uint32_t field_3BC_;
    uint32_t field_3C0_;
    uint32_t field_3C4_;
    uint32_t field_3C8_;
    uint32_t field_3CC_;
    uint32_t field_3D0_;
    uint32_t field_3D4_;
    uint32_t field_3D8_;
    uint32_t field_3DC_;
    uint32_t field_3E0_;
    uint32_t field_3E4_;
    uint32_t field_3E8_;
    uint32_t field_3EC_;
    uint32_t field_3F0_;
    uint32_t field_3F4_;
    uint32_t field_3F8_;
    uint32_t field_3FC_;
    uint32_t field_400_;
    uint32_t field_404_;
    uint32_t field_408_;
    uint32_t field_40C_;
    uint32_t field_410_;
    uint32_t field_414_;
    uint32_t field_418_;
    uint32_t field_41C_;
    uint32_t field_420_;
    uint32_t field_424_;
    uint32_t field_428_;
    uint32_t field_42C_;
    uint32_t field_430_;
    uint32_t field_434_;
    uint32_t field_438_;
    uint32_t field_43C_;
    uint32_t field_440_;
    uint32_t field_444_;
    uint32_t field_448_;
    uint32_t field_44C_;
    uint32_t field_450_;
    uint32_t field_454_;
    uint32_t field_458_;
    uint32_t field_45C_;
    uint32_t field_460_;
    uint32_t field_464_;
    uint32_t field_468_;
    uint32_t field_46C_;
    uint32_t field_470_;
    uint32_t field_474_;
    uint32_t field_478_;
    uint32_t field_47C_;
    uint32_t field_480_;
    uint32_t field_484_;
    uint32_t field_488_;
    uint32_t field_48C_;
    uint32_t field_490_;
    uint32_t field_494_;
    uint32_t field_498_;
    uint32_t field_49C_;
    uint32_t field_4A0_;
    uint32_t field_4A4_;
    uint32_t field_4A8_;
    uint32_t field_4AC_;
    uint32_t field_4B0_;
    uint32_t field_4B4_;
    uint32_t field_4B8_;
    uint32_t field_4BC_;
    uint32_t field_4C0_;
    uint32_t field_4C4_;
    uint32_t field_4C8_;
    uint32_t field_4CC_;
    uint32_t field_4D0_;
    uint32_t field_4D4_;
    uint32_t field_4D8_;
    uint32_t field_4DC_;
    uint32_t field_4E0_;
    uint32_t field_4E4_;
    uint32_t field_4E8_;
    uint32_t field_4EC_;
    uint32_t field_4F0_;
    uint32_t field_4F4_;
    uint32_t field_4F8_;
    uint32_t field_4FC_;
    uint32_t field_500_;
    uint32_t field_504_;
    uint32_t field_508_;
    uint32_t field_50C_;
    uint32_t field_510_;
    uint32_t field_514_;
    uint32_t field_518_;
    uint32_t field_51C_;
    uint32_t field_520_;
    uint32_t field_524_;
    uint32_t field_528_;
    uint32_t field_52C_;
    uint32_t field_530_;
    uint32_t field_534_;
    uint32_t field_538_;
    uint32_t field_53C_;
    uint32_t field_540_;
    uint32_t field_544_;
    uint32_t field_548_;
    uint32_t field_54C_;
    uint32_t field_550_;
    uint32_t field_554_;
    uint32_t field_558_;
    uint32_t field_55C_;
    uint32_t field_560_;
    uint32_t field_564_;
    uint32_t field_568_;
    uint32_t field_56C_;
    uint32_t field_570_;
    uint32_t field_574_;
    uint32_t field_578_;
    uint32_t field_57C_;
    uint32_t field_580_;
    uint32_t field_584_;
    uint32_t field_588_;
    uint32_t field_58C_;
    uint32_t field_590_;
    uint32_t field_594_;
    uint32_t field_598_;
    uint32_t field_59C_;
    uint32_t field_5A0_;
    int convert(int, int);
};

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
                    int r1 = reinterpret_cast<DeletionList *>(lock_table_)
                                 ->convert(*reinterpret_cast<int *>(msg + 10), *reinterpret_cast<int *>(msg + 6));
                    *reinterpret_cast<int *>(msg + 10) = r1;
                    if (*(reinterpret_cast<char *>(msg) + 0x35) != 1) {
                        goto fail;
                    }
                    if (static_cast<short>(msg[0x1c]) < 0) {
                        goto fail;
                    }
                    int r2 = reinterpret_cast<DeletionList *>(lock_table_)
                                 ->convert(static_cast<short>(msg[0x1c]), *reinterpret_cast<int *>(msg + 6));
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
        int r3 = reinterpret_cast<DeletionList *>(lock_table_)
                     ->convert(*reinterpret_cast<int *>(msg + 8), *reinterpret_cast<int *>(msg + 6));
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

extern int __cdecl sub_634280(unsigned char *, unsigned long *, unsigned char *, unsigned int);

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
    void *storage = 0;
    memcpy(&storage, &NetDaemonNet.lock_state_[3].leading_words_[0], sizeof storage);
    return static_cast<NetDaemonFileIndexRecord *>(storage);
}

static void netdaemon_set_file_index_records(
    NetDaemonFileIndexRecord *records) {
    void *storage = records;
    memcpy(&NetDaemonNet.lock_state_[3].leading_words_[0], &storage, sizeof storage);
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
    const bool local_is_host = lock_state_[0].leading_words_[1] != 0;
    NetDaemonFileIndexRecord *records = netdaemon_file_index_records();

    if (type == 0xF09) { // decimal 3849: native shared-default route
        if ((bytes[1] & 0xA0U) != 0) {
            if (fixup_message(message) == 0) {
                const uint16_t forwarded = static_cast<uint16_t>(
                    (type & 0x1FFFU) | 0x4000U);
                netdaemon_transfer_write_u16(bytes, forwarded);
                ++lock_state_[0].leading_words_[0];
                netdaemon_transfer_write_u32(bytes + 0x0C,
                                             lock_state_[0].leading_words_[0]);
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
            // Native 0x00634330 is the zlib 1.0.2 one-shot uncompress API:
            // dest, in/out destLen, source, sourceLen. The project already
            // links that genuine vendor component; do not recover a duplicate.
            uncompress(mapped, &decompressed_size,
                       NetDaemonFileTransferReceiveBuffer,
                       compressed_size);
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

    NetDaemonNet.lock_state_[2].trailing_words_[9] = 0;
    netdaemon_set_file_index_records(netdaemon_allocate_file_index_records());

    char executable_path[0x104];
    GetModuleFileNameA(0, executable_path, sizeof executable_path);
    char *last_separator = strrchr(executable_path, '\\');
    if (last_separator != 0) last_separator[1] = '\0';

    if (lock_state_[0].leading_words_[1] != 0 && NetDaemonNet.player_count_ > 0) {
        for (int slot = 1; slot <= NetDaemonNet.player_count_; ++slot) {
            if (local_index != slot) send_mask |= 1U << (slot & 31);
            NetDaemonNet.process_slots_[slot].status_bytes_[0] =
                static_cast<uint8_t>(local_index == slot ? 0 : 2);
        }
    }

    if (lock_state_[0].leading_words_[1] == 0) {
        while (NetDaemonFileTransferMode != 1) {
            unsigned long sender, channel;
            const int packet = NetDaemonNet.Net::get(&sender, &channel);
            if (packet != 0)
                process_message(reinterpret_cast<char *>(packet), channel,
                                static_cast<int>(sender));
        }

        unsigned char announce[0x20]; // native sends 0x20 bytes
        const uint16_t announce_type = 0xF0F;
        memcpy(announce, &announce_type, sizeof announce_type);
        memcpy(announce + 2, &local_index, sizeof local_index);
        NetDaemonNet.Net::send(announce, 0x20,
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
        uint8_t path_message[0x11C];
        const uint16_t path_type = 0xF05;
        memcpy(path_message, &path_type, sizeof path_type);
        path_message[2] = '\0';
        strcat(reinterpret_cast<char *>(path_message + 2),
               NetDaemonFileTransferPath);
        send_message(reinterpret_cast<char *>(path_message), 0x11C, 0);
        NetDaemonFileTransferAckMask = 0;
        if (send_mask != 0) {
            do {
                unsigned long sender, channel;
                const int packet = NetDaemonNet.Net::get(&sender, &channel);
                if (packet != 0)
                    process_message(reinterpret_cast<char *>(packet), channel,
                                    static_cast<int>(sender));
            } while (NetDaemonFileTransferAckMask != send_mask);
        }
    }

    char previous_directory[0x104];
    GetCurrentDirectoryA(sizeof previous_directory, previous_directory);
    const BOOL changed_directory = SetCurrentDirectoryA(SaveNameBuffer);
    if (changed_directory == 0 && lock_state_[0].leading_words_[1] == 0) {
        if (_mkdir(SaveNameBuffer) == 0) SetCurrentDirectoryA(SaveNameBuffer);
    }

    int file_count = 0;
    if (lock_state_[0].leading_words_[1] == 0) {
        for (;;) {
            if (NetDaemonFileTransferMode == 3 ||
                lock_state_[2].trailing_words_[9] != 0) goto cleanup;
            if ((GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0) break;
            do_draw();
            unsigned long sender, channel;
            const int packet = NetDaemonNet.Net::get(&sender, &channel);
            if (packet != 0)
                process_message(reinterpret_cast<char *>(packet), channel,
                                static_cast<int>(sender));
        }
        lock_state_[2].trailing_words_[9] = 1;
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

        uint16_t list_done[0x10];
        list_done[0] = 0xF07;
        const int list_done_state = 2;
        memcpy(reinterpret_cast<unsigned char *>(list_done) + 2,
               &list_done_state, sizeof list_done_state);
        send_message(reinterpret_cast<char *>(list_done), 0x20, 0);
        FindClose(find_handle);

        for (;;) {
            do_draw();
            unsigned long sender, channel;
            const int packet = NetDaemonNet.Net::get(&sender, &channel);
            if (packet != 0)
                process_message(reinterpret_cast<char *>(packet), channel,
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
                lock_state_[2].trailing_words_[9] = 1;
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
                sub_634280(static_cast<unsigned char *>(compressed),
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
            const int packet = NetDaemonNet.Net::get(&sender, &channel);
            if (packet != 0)
                process_message(reinterpret_cast<char *>(packet), channel,
                                static_cast<int>(sender));
            do_draw();
            if (NetDaemonNet.player_count_ < 1) break;
            int slot = 1;
            while (slot <= NetDaemonNet.player_count_ &&
                   NetDaemonNet.process_slots_[slot].status_bytes_[0] > 1)
                ++slot;
            if (slot > NetDaemonNet.player_count_) break;
        }
        uint16_t transfer_done[0x10];
        transfer_done[0] = 0xF07;
        const int transfer_done_state = 3;
        memcpy(reinterpret_cast<unsigned char *>(transfer_done) + 2,
               &transfer_done_state, sizeof transfer_done_state);
        send_message(reinterpret_cast<char *>(transfer_done), 0x20, 0);
    }

cleanup:
    pop_close();
    do_all_draws();
    SetCurrentDirectoryA(previous_directory);
    void *list_storage = netdaemon_file_index_records();
    if (list_storage != 0) free(list_storage);
    const int result = static_cast<int>(NetDaemonNet.lock_state_[2].trailing_words_[9]);
    netdaemon_set_file_index_records(0);
    NetDaemonFileTransferMode = 0;
    return result;
}

// ORIGINAL: 0x00534400 ?process_message@NetDaemon@@QAEXPADKH@Z 0x00534400-0x005388B9;0x0065E0C2-0x0065E442 FILE
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
    // process_message reaches three daemon fields by offset - 0x1B30 is
    // lock_state_[0].dword_[1], named below; 0x1B74 is lock_state_[1].dword_[3]
    // and 0x1BB8 is lock_state_[2].dword_[5], both in parts of the dispatcher
    // this transcription does not reach - and one per-slot byte table at
    // 0x794, the first player's status byte at record offset +0x28.

    unsigned char *msg = reinterpret_cast<unsigned char *>(message);
    unsigned short type = *reinterpret_cast<unsigned short *>(message);

    // Special-cased "file transfer" message-type band (0x1500-0x15FF).
    bool skip500 = ((type & 0xf00) != 0x500) || (type == 0x1501);
    if (!skip500 && CommlinkPending != 0 && CommlinkFaction == sender_id) {
        skip500 = true;
    }
    if (!skip500) {
        if (type == 0x1505) {
            netdaemon_transfer_modal_cleanup(guard);
            return;
        }
        message_data(0x1505, sender_id, 0, 0, 0, 0);
    }

    type = *reinterpret_cast<unsigned short *>(message);

    // File-transfer opcodes are handled by the complete source-owned routes below.
    if (type >= 0xF05 && type <= 0xF0F) {
        if (process_file_transfer_message(message, message_size, sender_id,
                                          guard, filemap)) {
            return;
        }
    }

    if (type == 0x1101) {
        // TODO: 0x53624a - not reconstructed (schedule/ready-state update).
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
            uint32_t *seq = &lock_state_[0].leading_words_[0];
            *seq = *seq + 1;
            *reinterpret_cast<int *>(message + 0xc) = *seq;
            send_message(message, message_size, 0);
        }
    }

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
    if (NetDaemonNet.lock_state_[0].leading_words_[1] != 0 &&
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
    if (NetDaemonNet.lock_state_[0].leading_words_[1] != 0) {
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
  int result = NetDaemonNet.Net::get(&sender, &channel);
  if (result != 0) {
    process_message(reinterpret_cast<char *>(result), channel,
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
  if (lock_state_[2].leading_words_[2] != 0) {
    if (lock_state_[2].trailing_words_[3] == 0) {
      ++lock_state_[2].trailing_words_[5];
      // The image compares the count SIGNED here (`jle`), not unsigned.
      if (1 < static_cast<int>(lock_state_[2].trailing_words_[5])) {
        NetMsgPrimary.pop("MOVEPENDING", 0, 2, 0);
      }
    }
    return 1;
  }
  lock_state_[2].trailing_words_[5] = 0;
  return lock_veh(veh_id, payload_1C | 4, payload_20, payload_18, extra);
}

// Original0x00532140 performs the network half of a vehicle drop. The
// lock-state words are the existing typed entries documented above; the
// non-network action is a separate real owner in veh.cpp.
// ORIGINAL: 0x00532140 ?veh_drop@NetDaemon@@QAEHHHH@Z 0x00532140-0x00532563 FILE
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/00532140.cpp retained signature/extents and complete seed; no historical claim copied", "Coordinator full pinned original instruction decode 00532140..00532563, typed source owner fields and corrected original call/branch evidence", "recovery/tests/test_airdrop_network.py actual-body directional treaty and network reentrancy controls; not linked/runtime/game acceptance"], "reason": "Recover the true network receiver, lock/ack state, global action-mode reentrancy and local execution result; preserve early return versus release paths and leave ExitTurnLoop read-only."}
int NetDaemon::veh_drop(int veh_id, int x, int y) {
    if (IsMultiplayerNet == 0) {
        return action_airdrop(veh_id, x, y, 3);
    }

    if (lock_state_[2].leading_words_[2] != 0) {
        if (lock_state_[2].trailing_words_[3] != 0) return 0;
        ++lock_state_[2].trailing_words_[5];
        if (static_cast<int>(lock_state_[2].trailing_words_[5]) < 2) return 0;
        NetMsgPrimary.pop("MOVEPENDING", 0, 2, nullptr);
        return 0;
    }

    lock_state_[2].trailing_words_[5] = 0;
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
            if (lock_state_[2].trailing_words_[3] != 0) {
                Vehs[veh_id].order = 0;
                Vehs[veh_id].state &= 0xFCFFBDFFU;
                synch_veh(veh_id);
                if (IsMultiplayerNet != 0) {
                    int *acknowledged = reinterpret_cast<int *>(&lock_state_[2].trailing_words_[0]);
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
                const int saved_action_mode = NetDaemonNet.lock_state_[1].leading_words_[3];
                const DWORD start_time = timeGetTime();
                bool showed_wait_popup = false;
                if (lock_state_[2].leading_words_[3] == 1) {
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
                    } while (lock_state_[2].leading_words_[3] == 1);
                    if (showed_wait_popup) NetMsgPrimary.close();
                }
                --NetBusyCount;
                NetDaemonNet.lock_state_[1].leading_words_[3] = saved_action_mode;
                log_say("Client received acknowledgement",
                        ConsoleGlobal.faction_id_, 0, 0);
                if (IsMultiplayerNet != 0) {
                    log_say("Client releasing lock",
                            ConsoleGlobal.faction_id_, 0, 0);
                    message_data(0x2212, 0, 0, 0, 0, 0);
                    lock_state_[2].leading_words_[3] = 0;
                    lock_state_[2].leading_words_[2] = 0;
                    lock_state_[2].trailing_words_[5] = 0;
                    lock_state_[2].trailing_words_[4] = 0;
                }
                lock_state_[2].leading_words_[2] = 0;
                lock_state_[2].trailing_words_[3] = 0;
                lock_state_[2].leading_words_[3] = 0;
                return 1;
            }
            return 1;
        }
    }

release_vehicle:
    if (IsMultiplayerNet != 0) {
        log_say("Client releasing lock", ConsoleGlobal.faction_id_, 0, 0);
        message_data(0x2212, 0, 0, 0, 0, 0);
        lock_state_[2].leading_words_[3] = 0;
        lock_state_[2].leading_words_[2] = 0;
        lock_state_[2].trailing_words_[5] = 0;
        lock_state_[2].trailing_words_[4] = 0;
    }
    lock_state_[1].payload_.vehicle_selection_follow = 0;
    lock_state_[2].trailing_words_[3] = 0;
    return 0;
}


// ORIGINAL: 0x00532570 ?action@NetDaemon@@QAEHHH@Z 0x00532570-0x005328BA
// RECOVERY_BINDING: v1 {"evidence": ["Pinned532570..5328BA; retained src/unrecovered/00532570.cpp is a conflicting historical hint, not the product ABI.", "Entry signed order classification is distinct from the post-automated_lock vehicle/order reload. Absolute93E904 timing storage belongs to NetDaemonNet.lock_state_[1].dword_[3], while wait flags and Lock remain receiver-relative.", "Pinned NUL log/popup literals68C2DC,68C390,68C3B4,68C3C0,68C4AC,68C4C8,68C4F4,68C510,68C530,68C544,68C554; ExitTurnLoop is game owner9B2068."], "reason": "Recover the full multiplayer vehicle-order action through the real NetDaemon, vehicle and transport owners; retain its original seed identity and complete span without a matching or runtime claim."}
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
                    lock_state_[2].leading_words_[3] = 0;
                    lock_state_[2].leading_words_[2] = 0;
                    lock_state_[2].trailing_words_[5] = 0;
                    lock_state_[2].trailing_words_[4] = 0;
                }
                lock_state_[1].payload_.vehicle_selection_follow = 0;
                lock_state_[2].trailing_words_[3] = 0;
            }
            return 0;
        }
    } else {
        category = (action_type == 0x14 || action_type == 0x15) ? 0xC : 0;
    }

    lock_state_[2].trailing_words_[3] = 1;
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
                NetDaemonNet.lock_state_[1].leading_words_[3];
            DWORD (WINAPI *const get_time)() = timeGetTime;
            ++NetBusyCount;
            const DWORD start_time = get_time();
            bool showed_wait_popup = false;
            if (lock_state_[2].leading_words_[3] == 1) {
                do {
                    if (ExitTurnLoop != 0) break;
                    if (!showed_wait_popup && get_time() - start_time > 2000) {
                        showed_wait_popup = true;
                        NetMsgPrimary.pop("AWAITEXEC", 0, 2, nullptr);
                    }
                    wait_task();
                    unsigned long first, second;
                    const int message = NetDaemonNet.Net::get(&first, &second);
                    if (message != 0)
                        process_message(reinterpret_cast<char *>(message),
                                        second, static_cast<int>(first));
                    locks_.check_global();
                    check_spock();
                } while (lock_state_[2].leading_words_[3] == 1);
                if (showed_wait_popup) NetMsgPrimary.close();
            }
            --NetBusyCount;
            NetDaemonNet.lock_state_[1].leading_words_[3] = saved_action_mode;
            log_say("Client received acknowledgement", ConsoleGlobal.faction_id_, 0, 0);
            if (IsMultiplayerNet != 0) {
                log_say("Client releasing lock", ConsoleGlobal.faction_id_, 0, 0);
                message_data(0x2212, 0, 0, 0, 0, 0);
                lock_state_[2].leading_words_[3] = 0;
                lock_state_[2].leading_words_[2] = 0;
                lock_state_[2].trailing_words_[5] = 0;
                lock_state_[2].trailing_words_[4] = 0;
            }
            lock_state_[1].payload_.vehicle_selection_follow = 0;
            lock_state_[2].trailing_words_[3] = 0;
            lock_state_[2].leading_words_[3] = 0;
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
// RECOVERY_BINDING: v1 {"evidence": ["Pinned531780..532135; retained src/unrecovered/00531780.cpp preserves the original identity and contradictory seed history.", "Native5318D6..5318E8 computes(otherFaction+2099*ownFaction)*4+96C9F8: PlayersData[ownFaction].diplo_treaties[otherFaction], stride20CC, not the seed20C8 transcription.", "Veh34/Base134/Map2C fields and existing coordinate arrays own the accessed state; signed speed and lock cleanup/callback ordering retain native obligations. Literal68C480 is GLOBALFAILED and68C490 is Sending MOVE VEH message; native synchronization and the full message dispatcher remain unfinished."], "reason": "Recover the complete network move request, feasibility, acknowledgement and release paths on the genuine daemon and game records. Preserve unresolved synch/process_message dependencies rather than forwarding this body to the original executable."}
int NetDaemon::order_veh(int veh_id, int direction, int mode) {
    if (IsMultiplayerNet == 0) return ::order_veh(veh_id, direction, 3);

    if (mode == 0) {
        if (lock_state_[2].leading_words_[2] != 0) {
            if (lock_state_[2].trailing_words_[3] != 0) return 0;
            ++lock_state_[2].trailing_words_[5];
            if (static_cast<int>(lock_state_[2].trailing_words_[5]) < 2) return 0;
            NetMsgPrimary.pop("MOVEPENDING", 0, 2, nullptr);
            return 0;
        }
        lock_state_[2].trailing_words_[5] = 0;
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
            int *flag = reinterpret_cast<int *>(&lock_state_[2].leading_words_[3]);
            const int waited = wait("AWAITEXEC", flag, 1, 2000);
            NetDaemonWaitTicks += waited;
            log_say("Client received acknowledgement", ConsoleGlobal.faction_id_, 0, 0);
            if (IsMultiplayerNet != 0) {
                log_say("Client releasing lock", ConsoleGlobal.faction_id_, 0, 0);
                message_data(0x2212, 0, 0, 0, 0, 0);
                *flag = 0;
                lock_state_[2].leading_words_[2] = 0;
                lock_state_[2].trailing_words_[5] = 0;
                lock_state_[2].trailing_words_[4] = 0;
            }
            lock_state_[1].payload_.vehicle_selection_follow = 0;
            lock_state_[2].trailing_words_[3] = 0;
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
                        (LockEnableMask & (1U << other_faction)) != 0) {
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
                const int message = NetDaemonNet.Net::get(&first, &second);
                if (message != 0)
                    process_message(reinterpret_cast<char *>(message), second,
                                    static_cast<int>(first));
                locks_.check_global();
                if (ExitTurnLoop != 0) break;
            } while (get_time() - start < 500);
        }
        return 1;
    }

release_lock:
    if (lock_state_[2].trailing_words_[3] != 0) {
        Vehs[veh_id].order = 0;
        Vehs[veh_id].state &= 0xFCFFBDFFU;
        synch_veh(veh_id);
        if (IsMultiplayerNet != 0) {
            int *flag = reinterpret_cast<int *>(&lock_state_[2].trailing_words_[0]);
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
        lock_state_[2].leading_words_[3] = 0;
        lock_state_[2].leading_words_[2] = 0;
        lock_state_[2].trailing_words_[5] = 0;
        lock_state_[2].trailing_words_[4] = 0;
    }
    lock_state_[1].payload_.vehicle_selection_follow = 0;
    lock_state_[2].trailing_words_[3] = 0;
    return 0;

global_failed:
    // Kept separate because the seed's add_global failure path pops before
    // returning; it does not perform the generic release helper above.
    if (lock_state_[2].trailing_words_[3] != 0) {
        Vehs[veh_id].order = 0;
        Vehs[veh_id].state &= 0xFCFFBDFFU;
        synch_veh(veh_id);
        if (IsMultiplayerNet != 0) {
            int *flag = reinterpret_cast<int *>(&lock_state_[2].trailing_words_[0]);
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
        lock_state_[2].leading_words_[3] = 0;
        lock_state_[2].leading_words_[2] = 0;
        lock_state_[2].trailing_words_[5] = 0;
        lock_state_[2].trailing_words_[4] = 0;
    }
    lock_state_[1].payload_.vehicle_selection_follow = 0;
    lock_state_[2].trailing_words_[3] = 0;
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
        {0x000, sizeof GamePreferences}, {0x004, sizeof GameMorePreferences},
        {0x008, sizeof GameWarnings}, {0x00C, sizeof GameRules},
        {0x010, sizeof GameWireOpaque_9A64A0},
        {0x014, sizeof GameInterludeFlags},
        {0x018, sizeof GameWireOpaque_9A64A8},
        {0x01C, sizeof TutorialFlags}, {0x020, sizeof TutorialFlags2},
        {0x024, sizeof TerraformAnnouncedMask},
        {0x02C, sizeof TutWinFlags9A64BC}, {0x030, sizeof GameState},
        {0x034, sizeof DiffLevelCurrent}, {0x038, sizeof VehCurrentCount},
        {0x03C, sizeof BaseCurrentCount}, {0x040, sizeof GameSetupResetWord},
        {0x044, sizeof TurnCurrentNum}, {0x048, sizeof MissionYearCurrent},
        {0x04C, sizeof VehSelectionFollowLimit},
        {0x050, sizeof TimeControlsSelection},
        {0x054, sizeof TurnControlDurationMilliseconds},
        {0x058, sizeof LockEnableMask}, {0x05C, sizeof FactionRankings},
        {0x07C, sizeof RankingFactionIDUnk1},
        {0x080, sizeof RankingFactionIDUnk2}, {0x084, sizeof SecretProject},
        {0x184, sizeof GovernorFaction},
        {0x188, sizeof CouncilResolutionCount},
        {0x1B4, sizeof CouncilLastResolutionTurn},
        {0x1E0, sizeof GameTechAchieved},
        {0x239, sizeof LegacyGameSaveName},
        {0x339, sizeof GameWireOpaque_9A67C9},
        {0x33C, sizeof ClimateRiseStep}, {0x340, sizeof ClimateRiseLimit},
        {0x344, sizeof ClimateRiseAccum},
        {0x348, sizeof ClimateSeaLevelDelta},
        {0x34C, sizeof TranscendenceScoreBonus},
        {0x350, sizeof TranscendenceScoreWeights},
        {0x370, sizeof SunspotDuration}, {0x374, sizeof MountPlanetX},
        {0x378, sizeof MountPlanetY}, {0x37C, sizeof DustCloudDuration},
        {0x380, sizeof SaveDaemonStateWord},
        {0x384, sizeof ScnVictFacilityObj},
        {0x388, sizeof GlobalEnergyEventState},
        {0x38C, sizeof NetTurnFlags}, {0x390, sizeof NetTurnFaction},
        {0x394, sizeof EcologyEventAccumulator}
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
#define COPY0(offset, object) \
    copied += copy_member_bytes(chunk_offset, chunk, count, offset, object, \
                                packet_to_owner)

    // game.cpp/game.h objects. The four-byte and three-byte opaque objects
    // below are retained independent arrays, not padding or a packed block.
    COPY0(0x000, GamePreferences);
    COPY0(0x004, GameMorePreferences);
    COPY0(0x008, GameWarnings);
    COPY0(0x00C, GameRules);
    COPY0(0x010, GameWireOpaque_9A64A0);
    COPY0(0x014, GameInterludeFlags);
    COPY0(0x018, GameWireOpaque_9A64A8);
    COPY0(0x01C, TutorialFlags);
    COPY0(0x020, TutorialFlags2);

    // veh.cpp/terraforming.h owns this two-word announced mask.
    COPY0(0x024, TerraformAnnouncedMask);
    COPY0(0x02C, TutWinFlags9A64BC);

    COPY0(0x030, GameState);
    COPY0(0x034, DiffLevelCurrent);
    COPY0(0x038, VehCurrentCount);
    COPY0(0x03C, BaseCurrentCount);
    COPY0(0x040, GameSetupResetWord);
    COPY0(0x044, TurnCurrentNum);
    COPY0(0x048, MissionYearCurrent);
    COPY0(0x04C, VehSelectionFollowLimit);
    COPY0(0x050, TimeControlsSelection);
    COPY0(0x054, TurnControlDurationMilliseconds);
    COPY0(0x058, LockEnableMask);

    // Canonical current owners: eight FactionRankings entries, followed by
    // the two other ranking words, then SecretProject exactly once.
    COPY0(0x05C, FactionRankings);
    COPY0(0x07C, RankingFactionIDUnk1);
    COPY0(0x080, RankingFactionIDUnk2);
    COPY0(0x084, SecretProject);

    COPY0(0x184, GovernorFaction);
    COPY0(0x188, CouncilResolutionCount);
    COPY0(0x1B4, CouncilLastResolutionTurn);
    COPY0(0x1E0, GameTechAchieved);
    COPY0(0x239, LegacyGameSaveName);
    COPY0(0x339, GameWireOpaque_9A67C9);

    COPY0(0x33C, ClimateRiseStep);
    COPY0(0x340, ClimateRiseLimit);
    COPY0(0x344, ClimateRiseAccum);
    COPY0(0x348, ClimateSeaLevelDelta);
    COPY0(0x34C, TranscendenceScoreBonus);
    COPY0(0x350, TranscendenceScoreWeights);
    COPY0(0x370, SunspotDuration);
    COPY0(0x374, MountPlanetX);
    COPY0(0x378, MountPlanetY);
    COPY0(0x37C, DustCloudDuration);
    COPY0(0x380, SaveDaemonStateWord);
    COPY0(0x384, ScnVictFacilityObj);
    COPY0(0x388, GlobalEnergyEventState);
    COPY0(0x38C, NetTurnFlags);
    COPY0(0x390, NetTurnFaction);
    COPY0(0x394, EcologyEventAccumulator);

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
#define COPY_AT(wire, object) \
    copied += member_copy(offset, packet, count, wire, object, packet_to_owner)
#define COPY_BYTES(wire, pointer, bytes) \
    copied += overlap_copy(offset, packet, count, wire, \
        reinterpret_cast<unsigned char *>(pointer), bytes, packet_to_owner)

    switch (opcode) {
    case 0x00: // decimal 0: one interval serialized from separately owned Game globals
        return opcode0_copy_chunk(offset, packet, count, packet_to_owner);
    case 0x01: { // decimal 1: only the first LockEnableMask byte
        uint32_t &mask = LockEnableMask;
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
        COPY_AT(0x00, ObjectiveReqVictory);
        COPY_AT(0x04, ObjectivesSuddenDeathVictory);
        COPY_AT(0x08, ObjectiveAchievePts);
        COPY_AT(0x0C, VictoryAchieveBonusPts);
        COPY_AT(0x10, StartingMissionYear);
        COPY_AT(0x14, EndingMissionYear);
        COPY_BYTES(0x18, GameWireOpaque_94B4D8,
                   sizeof GameWireOpaque_94B4D8);
        break;
    }
    case 0x25: // decimal 37
        COPY_AT(0, VehCurrentCount);
        break;
    case 0x26: // decimal 38
        COPY_AT(0, BaseCurrentCount);
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
    const bool rules_notify = (GameRules & 0x10u) != 0;
    const bool scripted_turn = ConsoleControlTurnActive != 0;
    if (rules_notify && !scripted_turn) {
        if (NetTurnFaction != ConsoleGlobal.faction_id_) {
            message_data(0x2308, 0, 0, 0, 0, 0);
        } else {
            SpockNotifyPending = 1;
        }
    } else if (NetDaemonNet.lock_state_[0].leading_words_[1] != 0) {
        SpockNotifyPending = 1;
    } else {
        message_data(0x2308, 0, 0, 0, 0, 0);
    }
}

// Native USER32 import directory independently identifies slot0x00669330.
// RECOVERY_IMPORT: v1 __imp__GetAsyncKeyState@4 USER32.dll name:GetAsyncKeyState
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
    case 0x12: return VehCurrentCount == 0 ? -1 : VehCurrentCount * 52; // 18
    case 0x13: return 0x134;                        // decimal 19
    case 0x14: return BaseCurrentCount == 0 ? -1 : BaseCurrentCount * 308; // 20
    case 0x15: return 4;                            // decimal 21
    case 0x16: return 0x48;                         // decimal 22
    case 0x18: return 0x224;                        // decimal 24
    case 0x19: return 0x20;                         // decimal 25
    case 0x20: return TurnCurrentNum << 4;           // decimal 32
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
    const uint32_t sequence = self.lock_state_[0].leading_words_[0];
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
// RECOVERY_BINDING: v1 {"evidence": ["Pinned532E00..534129; retained src/unrecovered/00532e00.cpp; independent jump-table and chunk/callback review.", "Opcode0 exact398-byte partition uses existing source objects and bounded fields; rule80..8D receive stages persist and compact into Alpha-owned fields only on the final chunk.", "Map destinations and diplomacy pair captured before callbacks; outgoing owners reread per chunk; changed base/vehicle notification suppresses scatter; sender mask is lock_state_[2].dword_[6].", "Path93AC88 send_files producer52ED40 remains pending. Stage93AD8C and game tail94B4D8 have observed window bounds only, not complete allocation extents.", "recovery/tasks/netdaemon-player-lifecycle-recovery.json; retained historical claims and prototype-history owner migration."], "reason": "Recover the complete synchronization protocol through genuine NetDaemon state and source-owned game, vehicle, faction, map and rule data. Wire serializers describe explicit member intervals, not an invented contiguous game object. Retain seed provenance and original identity; no matching or runtime acceptance is implied."}
// TRIED: full transcription of both jump-table dispatches (size switch, address switch, tail array-compaction switch) and the chunk loop; MISMATCH #0 in the prologue (frame-size/local-order only), not in the dispatch logic - coverage is close to 100% of the original's ~4900 bytes, byte-exactness not chased further.
void NetDaemon::synch(int16_t opcode, int a2, int a3, int a4, char *packet,
                      int outgoing, int16_t flags) {
    if (!IsMultiplayerNet) return;

    // Native synchronization progress is owned by lock_state_[0]'s existing
    // dword slots 7/8. The popup and state transition order follows the native
    // first dispatch; these calls are before target capture/chunk callbacks.
    if (!outgoing) {
        uint32_t &game_phase = lock_state_[0].trailing_words_[2];
        uint32_t &rules_phase = lock_state_[0].trailing_words_[3];
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
            lock_state_[2].trailing_words_[1] &=
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
    } else if (opcode == 0x02 && lock_state_[0].leading_words_[1] == 0) { // decimal 2
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
    lock_state_[0].leading_words_[1] = 0;
    lock_state_[0].leading_words_[2] = 0;
    lock_state_[0].leading_words_[3] = 0;
    lock_state_[0].payload_.flags = 0;
    lock_state_[0].trailing_words_[8] = 0;
    lock_state_[0].trailing_words_[7] = 0;

    lock_state_[1].leading_words_[1] = 0;
    lock_state_[1].leading_words_[0] = 0;
    lock_state_[1].trailing_words_[0] = 0;
    lock_state_[2].leading_words_[1] = 0;
    lock_state_[1].trailing_words_[9] = 0;
    lock_state_[2].leading_words_[0] = 0;
    lock_state_[2].trailing_words_[5] = 0;
    lock_state_[2].trailing_words_[4] = 0;

    lock_state_[0].trailing_words_[4] = 0;
    lock_state_[0].trailing_words_[5] = 0;
    lock_state_[0].trailing_words_[6] = 0;
    lock_state_[2].trailing_words_[3] = 0;
    lock_state_[2].trailing_words_[2] = 0;
    lock_state_[2].leading_words_[3] = 0;
    lock_state_[2].leading_words_[2] = 0;
    lock_state_[2].payload_.flags = 0;
    lock_state_[0].trailing_words_[0] = 0;
    lock_state_[0].trailing_words_[1] = 0;
    lock_state_[2].trailing_words_[8] = 0;
    lock_state_[2].trailing_words_[7] = 0;
    lock_state_[0].trailing_words_[2] = 0;
    lock_state_[0].trailing_words_[3] = 0;
    lock_state_[2].trailing_words_[6] = 0xFF;
    lock_state_[1].payload_.vehicle_selection_follow = nullptr;

    for (int i = 1; i <= 8; ++i) {
        lock_state_[1].trailing_words_[i] = 0;
    }

    locks_.clear();
    AlphaNet::close();
}
