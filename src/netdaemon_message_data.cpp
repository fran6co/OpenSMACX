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

// The message/synch run of the image (0x00592DE0-0x00593502): the packet
// builders and the synch_* announcements interleave in address order, so
// they share one translation unit. The original filename is not known.

// ---------------------------------------------------------------------------
// The message builders. Every sender in the family packs its record on the
// stack, stamps the three leading fields the transport owns - the opcode
// (short), the local faction and the wall-clock stamp - then hands the record
// to NetDaemon::send_message with NetBusyCount held for the duration. Only
// the payload fields differ.
//
// The sequence stamp is 0x0093E8BC, the current_owner field at offset 0x5A0
// of NetDaemonNet.deletion_list_ (NetDaemonNet + 0x158C). The native
// net_end_of_turn (0x00529F30) advances it once per turn. The stamp comes from
// the SDK timeGetTime import; its original import-slot identity is bound
// beside the other audio clock consumers.
//
// Payload parameters are named for the record offset they land at, which is
// the one thing about them the bodies prove.
// ---------------------------------------------------------------------------

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

// ORIGINAL: 0x00592E70 ?message_base@@YAXHHPADH@Z 0x00592E70-0x00592EDA BYTE_EXACT
// size      106 bytes
// prototype void (__cdecl ?message_base@@YAXHHPADH@Z)(int, int, char *, int)
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00532940 0x00645460
void __cdecl message_base(int opcode, int payload_10, char *text, int target) {
    MessageBaseTextRecord message;

    message.opcode_ = static_cast<short>(opcode);
    message.faction_ = ConsoleGlobal.faction_id_;
    message.time_ = timeGetTime();
    message.sequence_ = NetDaemonNet.deletion_list_.sequence_bits();
    message.payload_10_ = payload_10;
    strcpy(message.text_, text);

    NetBusyCount++;
    NetDaemonNet.send_message(reinterpret_cast<char *>(&message), 0x2c,
                              target);
    NetBusyCount--;
}

// ORIGINAL: 0x00592EE0 ?message_data@@YAXHHHHHH@Z 0x00592EE0-0x00592F4C
// RECOVERY_BINDING: v1 {"evidence": ["Pinned592EE0..592F4C: six int inputs, target is argument2; 32-byte local packet has unwritten bytes2..3. Call532940 then dec93F664 and epilogue preserve EAX.", "Pinned53111B caller and source NetDaemon::unlock_veh consume all32 result bits; other current callers discard them.", "Superseded source dead ends: nonvoid body without return failed VC6 C4716; returning NetBusyCount added a wrong load. Returning the actual lower Net::send result resolves both without register tricks.", "Fresh root private VC6 COFF probe observes current symbol ?message_data@@YAHHHHHHH@Z", "recovery/tasks/message-data-owner-oct5.json", "recovery/tests/test_message_data_actual_source.py"], "reason": "Replace the original-address forwarding wrapper with the complete typed automatic packet builder. Expose the send result already observed by unlock_veh without changing native routing, busy-count ordering or the two unwritten packet bytes. Current netdaemon.cpp home is family ownership, not asserted original TU ownership."}
// symbol    ?message_data@@YAHHHHHHH@Z
#pragma optimize("y", off)
int __cdecl message_data(int opcode, int target, int payload_10,
                         int payload_14, int payload_18, int payload_1C) {
    MessageDataRecord message;
    message.opcode_ = static_cast<int16_t>(opcode);
    message.faction_ = ConsoleGlobal.faction_id_;
    message.timestamp_ = timeGetTime();
    message.payload_14_ = payload_14;
    ++NetBusyCount;
    message.sequence_ = NetDaemonNet.deletion_list_.sequence_bits();
    message.payload_18_ = payload_18;
    message.payload_10_ = payload_10;
    message.payload_1C_ = payload_1C;
    const int result = NetDaemonNet.send_message(
        reinterpret_cast<char *>(&message), 0x20, target);
    --NetBusyCount;
    return result;
}
#pragma optimize("", on)

// ORIGINAL: 0x00592F50 sub_592f50 0x00592F50-0x00592FE9 BYTE_EXACT
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

// ORIGINAL: 0x00592FF0 ?message_big_data@@YAXHHPAHH@Z 0x00592FF0-0x00593089 BYTE_EXACT
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
    int i;

    message.opcode_ = static_cast<int16_t>(opcode);
    message.faction_ = ConsoleGlobal.faction_id_;
    message.timestamp_ = timeGetTime();
    message.sequence_ = NetDaemonNet.deletion_list_.sequence_bits();

    for (i = 0; i < count; ++i) {
        message.data_[i] = payload[i];
    }
    for (; i < 0x20; ++i) {
        message.data_[i] = 0;
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

// ORIGINAL: 0x00593110 ?message_landmark@@YAXHHPADHHHH@Z 0x00593110-0x00593190 BYTE_EXACT
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
    message.sequence_ = NetDaemonNet.deletion_list_.sequence_bits();
    message.payload_10_ = payload_10;
    message.payload_14_ = payload_14;
    message.payload_18_ = payload_18;
    message.payload_1C_ = payload_1C;
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

// ORIGINAL: 0x00593370 sub_593370 0x00593370-0x0059338C BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:3f7fc8f78d783f09c46cddc646c31c2f5eb6d013:src/recovered/00593370.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x593370 complete original body; independent source-owner review.", "source:src/netdaemon.cpp and corresponding header own real receiver/dependencies; count-zero copy underflow is intentionally unchanged."], "reason": "Complete typed application helper; retain original call ordering, argument contract and historical obligation."}
// symbol    ?sub_593370@@YAXXZ
void __cdecl sub_593370() {
    NetDaemonNet.synch(1, 0, 0, 0, 0, 1, 0x2101);
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

// ORIGINAL: 0x00593480 sub_593480 0x00593480-0x005934A2 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:3f7fc8f78d783f09c46cddc646c31c2f5eb6d013:src/recovered/00593480.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x593480 complete original body; independent source-owner review.", "source:src/netdaemon.cpp and corresponding header own real receiver/dependencies; count-zero copy underflow is intentionally unchanged."], "reason": "Complete typed application helper; retain original call ordering, argument contract and historical obligation."}
// symbol    ?sub_593480@@YAXH@Z
void __cdecl sub_593480(int value) {
    NetDaemonNet.synch(0x17, value, 0, 0, 0, 1, 0x2101);
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
