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
#include "netdaemon.h"
#include "console.h"
#include "netdaemon_message_record.h"

// Packet construction has an observed independent call boundary in the
// NetDaemon callers. This source home does not establish an original
// filename or translation-unit boundary; the sender operation is unchanged.

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
    MessageVehicleRecord message;

    message.opcode_ = static_cast<short>(opcode);
    message.faction_ = ConsoleGlobal.faction_id_;
    message.time_ = timeGetTime();
    message.sequence_ = NetDaemonNet.deletion_list_.sequence_bits();
    message.field_10_ = payload_10;
    message.field_14_ = payload_14;

    NetBusyCount++;
    NetDaemonNet.send_message(reinterpret_cast<char *>(&message), 0x18,
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
    MessageLockRecord message;

    message.opcode_ = static_cast<short>(opcode);
    message.faction_ = ConsoleGlobal.faction_id_;
    message.time_ = timeGetTime();
    message.sequence_ = NetDaemonNet.deletion_list_.sequence_bits();
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
