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
