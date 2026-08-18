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
#include "original_seam.h"
#include "netdaemon.h"
#include "log.h"  // log_say, source-owned at 0x006262F0

func_net_get NetDaemonNetGet = original_method<func_net_get>(0x00630A00);
func_process_message NetDaemonProcessMessage =
    original_method<func_process_message>(0x00534400);
void *NetDaemonNet = reinterpret_cast<void *>(0x0093CD90);
func_net_daemon_synch NetDaemonSynch =
    original_method<func_net_daemon_synch>(0x00532E00);
int *NetDaemonIsMultiplayerNet = (int *)0x0093F660;
int *NetDaemonLocalFaction = (int *)0x00939284;
func_net_message_data *NetDaemonMessageData =
    (func_net_message_data *)0x00592EE0;

/*
Purpose: Poll the network for one message; dispatch it when one arrives and
         report whether it did. The message and its two parameters come back
         from Net::get through out-parameters.
// ORIGINAL: 0x00530320 ?receive@NetDaemon@@QAEHXZ 0x00530320-0x0053035E
// size      62 bytes
// prototype int (__thiscall ?receive@NetDaemon@@QAEHXZ)(NetDaemon* this)
// callers   9   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00534400 0x00630A00
Return Value: 1 when a message was received and dispatched, 0 when none was
Status: Complete
Verification note: the original leaves both out-params as uninitialized stack
garbage for Net::get to fill and reads them only after a nonzero return; the
zero initializers here pin an unspecified value deterministically, so the
sweep's surviving constant mutants on them are equivalent by construction.
*/
int NetDaemon::receive() {
    unsigned long first = 0;
    unsigned long second = 0;
    const int result = (ORIGINAL(NetDaemonNet)->*NetDaemonNetGet)(&first, &second);
    if (result == 0) {
        return 0;
    }
    (ORIGINAL(this)->*NetDaemonProcessMessage)(reinterpret_cast<char *>(result), second, static_cast<int>(first));
    return 1;
}

int __fastcall net_daemon_receive_redirect(NetDaemon *self, void *) {
    return self->receive();
}

/*
Purpose: Announce a vehicle's synched state to the network. All four "no
         text, one repeated flag" parameters are fixed by the caller, so only
         the opcode and the id vary across this family.
// ORIGINAL: 0x00593220 ?synch_veh@@YAXH@Z 0x00593220-0x00593242
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
    (ORIGINAL(NetDaemonNet)->*NetDaemonSynch)(0x11, id, 0, 0, nullptr, 1, 0x2101);
}

/*
Purpose: Announce a base's synched state to the network.
// ORIGINAL: 0x00593250 ?synch_base@@YAXH@Z 0x00593250-0x00593272
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
    (ORIGINAL(NetDaemonNet)->*NetDaemonSynch)(0x13, id, 0, 0, nullptr, 1, 0x2101);
}

/*
Purpose: Announce a faction's energy synched state to the network.
// ORIGINAL: 0x00593280 ?synch_energy@@YAXH@Z 0x00593280-0x005932A2
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
    (ORIGINAL(NetDaemonNet)->*NetDaemonSynch)(0xB, id, 0, 0, nullptr, 1, 0x2101);
}

/*
Purpose: Announce a faction's current research target to the network.
// ORIGINAL: 0x005932B0 ?synch_researching@@YAXH@Z 0x005932B0-0x005932D2
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
    (ORIGINAL(NetDaemonNet)->*NetDaemonSynch)(0xA, id, 0, 0, nullptr, 1, 0x2101);
}

/*
Purpose: Announce a faction's leader to the network.
// ORIGINAL: 0x005932E0 ?synch_leader@@YAXH@Z 0x005932E0-0x00593302
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
    (ORIGINAL(NetDaemonNet)->*NetDaemonSynch)(0x6, id, 0, 0, nullptr, 1, 0x2101);
}

/*
Purpose: Announce a faction's AI state to the network.
// ORIGINAL: 0x00593310 ?synch_ai@@YAXH@Z 0x00593310-0x00593332
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
    (ORIGINAL(NetDaemonNet)->*NetDaemonSynch)(0x8, id, 0, 0, nullptr, 1, 0x2101);
}

/*
Purpose: Announce a faction's completed research to the network.
// ORIGINAL: 0x00593340 ?synch_research@@YAXH@Z 0x00593340-0x00593362
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
    (ORIGINAL(NetDaemonNet)->*NetDaemonSynch)(0x9, id, 0, 0, nullptr, 1, 0x2101);
}

/*
Purpose: Announce a faction's social/resource allocation state to the
         network.
// ORIGINAL: 0x00593390 ?synch_alloc@@YAXH@Z 0x00593390-0x005933B2
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
    (ORIGINAL(NetDaemonNet)->*NetDaemonSynch)(0xC, id, 0, 0, nullptr, 1, 0x2101);
}

/*
Purpose: Announce a faction's social engineering state to the network.
// ORIGINAL: 0x005933C0 ?synch_soc@@YAXH@Z 0x005933C0-0x005933E2
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
    (ORIGINAL(NetDaemonNet)->*NetDaemonSynch)(0xD, id, 0, 0, nullptr, 1, 0x2101);
}

/*
Purpose: Announce a faction's protocol/pact state to the network.
// ORIGINAL: 0x005933F0 ?synch_proto@@YAXH@Z 0x005933F0-0x00593412
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
    (ORIGINAL(NetDaemonNet)->*NetDaemonSynch)(0xE, id, 0, 0, nullptr, 1, 0x2101);
}

/*
Purpose: Announce an observer's state to the network.
// ORIGINAL: 0x00593420 ?synch_obs@@YAXH@Z 0x00593420-0x00593442
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
    (ORIGINAL(NetDaemonNet)->*NetDaemonSynch)(0x10, id, 0, 0, nullptr, 1, 0x2101);
}

/*
Purpose: Announce a diplomatic state change between two factions to the
         network. Unlike the rest of this family, both leading arguments
         come from the caller instead of the second being a fixed zero.
// ORIGINAL: 0x00593450 ?synch_diplo@@YAXHH@Z 0x00593450-0x00593474
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
    (ORIGINAL(NetDaemonNet)->*NetDaemonSynch)(0x16, a, b, 0, nullptr, 1, 0x2101);
}

/*
Purpose: Announce a faction's template/design state to the network.
// ORIGINAL: 0x005934B0 ?synch_template@@YAXH@Z 0x005934B0-0x005934D2
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
    (ORIGINAL(NetDaemonNet)->*NetDaemonSynch)(0x18, id, 0, 0, nullptr, 1, 0x2101);
}

/*
Purpose: Announce a faction's sensor/territory radius state to the network.
// ORIGINAL: 0x005934E0 ?synch_radius@@YAXH@Z 0x005934E0-0x00593502
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
    (ORIGINAL(NetDaemonNet)->*NetDaemonSynch)(0x23, id, 0, 0, nullptr, 1, 0x2101);
}

/*
Purpose: Release the vehicle lock this client is holding. In a net game, tell
         the log and the other players first - log_say names the faction that
         is releasing and message_data broadcasts event 0x2212 - then clear the
         four announce-side fields; in every game, clear the locked vehicle
         record at 0x1B78 and the flag at 0x1BC4. Only the transport flag at
         0x0093F660 gates the announce; any nonzero value is a net game.
// ORIGINAL: 0x005310F0 ?unlock_veh@NetDaemon@@QAEXXZ 0x005310F0-0x0053114A
// size      90 bytes
// prototype void (__thiscall ?unlock_veh@NetDaemon@@QAEXXZ)(NetDaemon* this)
// callers   33   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00592EE0 0x006262F0
Return Value: EAX residue. The original is ?unlock_veh@NetDaemon@@QAEXXZ, but
              EAX is live at its ret: on the non-net path it is the loaded
              transport flag, which is zero exactly because that path was
              taken; on the net path it is message_data's residue. Modelled as
              uint32_t and returned, like GraphicWin::close and ListBox::close.
Status: Complete with temporary message_data original dependency
*/
uint32_t NetDaemon::unlock_veh() {
    // The layout past the AlphaNet base is not established, so the six fields
    // are written through raw volatile offsets - the ListBox::close idiom -
    // which also preserves the original's deliberately non-monotonic store
    // order (B0, AC, CC, C8, then the shared 78, C4) under optimization.
    volatile uint32_t *const object =
        reinterpret_cast<volatile uint32_t *>(this);

    // Loaded exactly once, and it is also the EAX the non-net path returns.
    uint32_t residue = static_cast<uint32_t>(*NetDaemonIsMultiplayerNet);
    if (residue != 0) {
        // `cmp eax, edi` against a zeroed edi: any nonzero flag announces.
        log_say("Client releasing lock", *NetDaemonLocalFaction, 0, 0);
        residue = NetDaemonMessageData(0x2212, 0, 0, 0, 0, 0);
        object[0x1BB0 / 4] = 0;
        object[0x1BAC / 4] = 0;
        object[0x1BCC / 4] = 0;
        object[0x1BC8 / 4] = 0;
    }
    // Shared tail; 0x1B78 is the vehicle record lock_veh (0x00531020) stores.
    object[0x1B78 / 4] = 0;
    object[0x1BC4 / 4] = 0;
    return residue;
}

// self == the NetDaemon; the original takes `this` straight out of ecx with no
// adjustment, so there is nothing to subtract here.
uint32_t __fastcall net_daemon_unlock_veh_redirect(NetDaemon *self, void *) {
    return self->unlock_veh();
}
