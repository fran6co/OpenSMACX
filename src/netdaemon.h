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

#include "original_seam.h"
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
  * receive() and unlock_veh() are recovered; the class's own layout past the
  * AlphaNet base is still not established, so nothing new is modelled and
  * nothing pins its sizeof. unlock_veh writes six dwords in the un-modelled
  * tail (0x1B78, 0x1BAC, 0x1BB0, 0x1BC4, 0x1BC8, 0x1BCC) through raw volatile
  * offsets rather than through invented placeholder fields.
  */
class DLLEXPORT NetDaemon : AlphaNet {
 public:
  NetDaemon() { ; }
  ~NetDaemon() { ; }
  int receive();
  uint32_t unlock_veh();

  // Storage the image proves is here: its own methods reach 0x1BD0.
  // Extent only - nothing here is named, and this class carries no size assertion.
  uint8_t field_14A0_[0x730];
};

// Net::get and NetDaemon::process_message are not recovered yet; the Net the
// daemon reads from lives at a fixed address.
typedef int (OriginalObject::*func_net_get)(unsigned long *a, unsigned long *b);
typedef void (OriginalObject::*func_process_message)(char *message, unsigned long a, int b);
extern func_net_get NetDaemonNetGet;
extern func_process_message NetDaemonProcessMessage;
extern void *NetDaemonNet;

int __fastcall net_daemon_receive_redirect(NetDaemon *self, void *);

// The multiplayer-transport flag at 0x0093F660 and the local faction id at
// 0x00939284, both read by unlock_veh. src/game.cpp binds the same two
// addresses as IsMultiplayerNet / LocalFaction, but that translation unit is
// not linked into recovery-leaf-tests, so NetDaemon owns its own rebindable
// pair the way src/spying_recovery.cpp owns SpyingCurrentFaction for the very
// same faction address.
extern int *NetDaemonIsMultiplayerNet;
extern int *NetDaemonLocalFaction;

// message_data at 0x00592EE0 broadcasts one game event; it is still an
// original dependency (it packs a 0x20-byte record and dispatches
// NetDaemon::send_message at 0x00532940), so unlock_veh reaches it through a
// rebindable seam. Declared returning uint32_t even though the original is
// ?message_data@@YAXHHHHHH@Z: unlock_veh's own EAX residue on the announce
// path IS this call's EAX, so the value has to be passed through rather than
// discarded.
typedef uint32_t(__cdecl func_net_message_data)(int a1, int a2, int a3, int a4,
                                                int a5, int a6);
extern func_net_message_data *NetDaemonMessageData;

// unlock_veh is entered on an unadjusted `this` (the original does a plain
// `mov esi, ecx`), so the adapter forwards without displacement.
uint32_t __fastcall net_daemon_unlock_veh_redirect(NetDaemon *self, void *);

// NetDaemon::synch itself is not recovered - 4,905 bytes with its own call
// targets, so it remains an original dependency. The fourteen synch_*
// forwarders below all funnel into it, loading the same daemon at 0x0093CD90
// that NetDaemonNet already binds above, and differ only in the opcode (and,
// for synch_diplo, which of the two leading arguments carries which value).
typedef void (OriginalObject::*func_net_daemon_synch)(int16_t opcode, int a, int b, int c, char *text, int d, int16_t flags);
extern func_net_daemon_synch NetDaemonSynch;

DLLEXPORT void __cdecl synch_veh(int id);
DLLEXPORT void __cdecl synch_base(int id);
DLLEXPORT void __cdecl synch_energy(int id);
DLLEXPORT void __cdecl synch_researching(int id);
DLLEXPORT void __cdecl synch_leader(int id);
DLLEXPORT void __cdecl synch_ai(int id);
DLLEXPORT void __cdecl synch_research(int id);
DLLEXPORT void __cdecl synch_alloc(int id);
DLLEXPORT void __cdecl synch_soc(int id);
DLLEXPORT void __cdecl synch_proto(int id);
DLLEXPORT void __cdecl synch_obs(int id);
DLLEXPORT void __cdecl synch_diplo(int a, int b);
DLLEXPORT void __cdecl synch_template(int id);
DLLEXPORT void __cdecl synch_radius(int id);
