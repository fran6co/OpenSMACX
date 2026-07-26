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
#include "netdaemon.h"

func_net_get *NetDaemonNetGet = (func_net_get *)0x00630A00;
func_process_message *NetDaemonProcessMessage =
    (func_process_message *)0x00534400;
void *NetDaemonNet = reinterpret_cast<void *>(0x0093CD90);
func_net_daemon_synch *NetDaemonSynch =
    (func_net_daemon_synch *)0x00532E00;

/*
Purpose: Poll the network for one message; dispatch it when one arrives and
         report whether it did. The message and its two parameters come back
         from Net::get through out-parameters.
Original Offset: 00530320
Return Value: 1 when a message was received and dispatched, 0 when none was
Status: Complete
*/
int NetDaemon::receive() {
    unsigned long first = 0;
    unsigned long second = 0;
    const int result = NetDaemonNetGet(NetDaemonNet, &first, &second);
    if (result == 0) {
        return 0;
    }
    NetDaemonProcessMessage(this, reinterpret_cast<char *>(result), second,
                            static_cast<int>(first));
    return 1;
}

int __fastcall net_daemon_receive_redirect(NetDaemon *self, void *) {
    return self->receive();
}

/*
Purpose: Announce a vehicle's synched state to the network. All four "no
         text, one repeated flag" parameters are fixed by the caller, so only
         the opcode and the id vary across this family.
Original Offset: 00593220
Return Value: none
Status: Complete
*/
void __cdecl synch_veh(int id) {
    NetDaemonSynch(NetDaemonNet, 0x11, id, 0, 0, nullptr, 1, 0x2101);
}

/*
Purpose: Announce a base's synched state to the network.
Original Offset: 00593250
Return Value: none
Status: Complete
*/
void __cdecl synch_base(int id) {
    NetDaemonSynch(NetDaemonNet, 0x13, id, 0, 0, nullptr, 1, 0x2101);
}

/*
Purpose: Announce a faction's energy synched state to the network.
Original Offset: 00593280
Return Value: none
Status: Complete
*/
void __cdecl synch_energy(int id) {
    NetDaemonSynch(NetDaemonNet, 0xB, id, 0, 0, nullptr, 1, 0x2101);
}

/*
Purpose: Announce a faction's current research target to the network.
Original Offset: 005932B0
Return Value: none
Status: Complete
*/
void __cdecl synch_researching(int id) {
    NetDaemonSynch(NetDaemonNet, 0xA, id, 0, 0, nullptr, 1, 0x2101);
}

/*
Purpose: Announce a faction's leader to the network.
Original Offset: 005932E0
Return Value: none
Status: Complete
*/
void __cdecl synch_leader(int id) {
    NetDaemonSynch(NetDaemonNet, 0x6, id, 0, 0, nullptr, 1, 0x2101);
}

/*
Purpose: Announce a faction's AI state to the network.
Original Offset: 00593310
Return Value: none
Status: Complete
*/
void __cdecl synch_ai(int id) {
    NetDaemonSynch(NetDaemonNet, 0x8, id, 0, 0, nullptr, 1, 0x2101);
}

/*
Purpose: Announce a faction's completed research to the network.
Original Offset: 00593340
Return Value: none
Status: Complete
*/
void __cdecl synch_research(int id) {
    NetDaemonSynch(NetDaemonNet, 0x9, id, 0, 0, nullptr, 1, 0x2101);
}

/*
Purpose: Announce a faction's social/resource allocation state to the
         network.
Original Offset: 00593390
Return Value: none
Status: Complete
*/
void __cdecl synch_alloc(int id) {
    NetDaemonSynch(NetDaemonNet, 0xC, id, 0, 0, nullptr, 1, 0x2101);
}

/*
Purpose: Announce a faction's social engineering state to the network.
Original Offset: 005933C0
Return Value: none
Status: Complete
*/
void __cdecl synch_soc(int id) {
    NetDaemonSynch(NetDaemonNet, 0xD, id, 0, 0, nullptr, 1, 0x2101);
}

/*
Purpose: Announce a faction's protocol/pact state to the network.
Original Offset: 005933F0
Return Value: none
Status: Complete
*/
void __cdecl synch_proto(int id) {
    NetDaemonSynch(NetDaemonNet, 0xE, id, 0, 0, nullptr, 1, 0x2101);
}

/*
Purpose: Announce an observer's state to the network.
Original Offset: 00593420
Return Value: none
Status: Complete
*/
void __cdecl synch_obs(int id) {
    NetDaemonSynch(NetDaemonNet, 0x10, id, 0, 0, nullptr, 1, 0x2101);
}

/*
Purpose: Announce a diplomatic state change between two factions to the
         network. Unlike the rest of this family, both leading arguments
         come from the caller instead of the second being a fixed zero.
Original Offset: 00593450
Return Value: none
Status: Complete
*/
void __cdecl synch_diplo(int a, int b) {
    NetDaemonSynch(NetDaemonNet, 0x16, a, b, 0, nullptr, 1, 0x2101);
}

/*
Purpose: Announce a faction's template/design state to the network.
Original Offset: 005934B0
Return Value: none
Status: Complete
*/
void __cdecl synch_template(int id) {
    NetDaemonSynch(NetDaemonNet, 0x18, id, 0, 0, nullptr, 1, 0x2101);
}

/*
Purpose: Announce a faction's sensor/territory radius state to the network.
Original Offset: 005934E0
Return Value: none
Status: Complete
*/
void __cdecl synch_radius(int id) {
    NetDaemonSynch(NetDaemonNet, 0x23, id, 0, 0, nullptr, 1, 0x2101);
}
