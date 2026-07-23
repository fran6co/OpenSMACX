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
#pragma once
#include "alphanet.h"

 /*
  * NetDaemon class
  *
  * Derives from AlphaNet on an unadjusted `this`. receive() polls the network
  * for one message and dispatches it, but it reads that message from the Net
  * object at a fixed address rather than through `this` - the original loads
  * 0x0093CD90 into ecx before calling Net::get - so that global is a
  * rebindable seam here, not the daemon itself.
  *
  * Only receive() is recovered; the class's own layout past the AlphaNet base
  * is not established, so nothing new is modelled and nothing pins its sizeof.
  */
class DLLEXPORT NetDaemon : AlphaNet {
 public:
  NetDaemon() { ; }
  ~NetDaemon() { ; }
  int receive();
};

// Net::get and NetDaemon::process_message are not recovered yet; the Net the
// daemon reads from lives at a fixed address.
typedef int (__thiscall func_net_get)(void *net, unsigned long *a, unsigned long *b);
typedef void (__thiscall func_process_message)(NetDaemon *self, char *message,
                                               unsigned long a, int b);
extern func_net_get *NetDaemonNetGet;
extern func_process_message *NetDaemonProcessMessage;
extern void *NetDaemonNet;

int __fastcall net_daemon_receive_redirect(NetDaemon *self, void *);
