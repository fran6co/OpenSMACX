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
class NetDaemon : AlphaNet {
 public:
  NetDaemon() { ; }
  ~NetDaemon() { ; }
  int receive();
  uint32_t unlock_veh();

  // Storage the image proves is here: its own methods reach 0x1BD0.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  uint8_t field_14A0_[0x1C];  // 0x14A0
  uint8_t field_14BC_;  // 0x14BC
  uint8_t field_14BD_[0xC3];  // 0x14BD
  uint32_t field_1580_;  // 0x1580
  uint32_t field_1584_;  // 0x1584
  uint8_t field_1588_[0x4];  // 0x1588
  uint8_t field_158C_;  // 0x158C
  uint8_t field_158D_[0x7];  // 0x158D
  uint16_t field_1594_;  // 0x1594
  uint8_t field_1596_[0x596];  // 0x1596
  uint32_t field_1B2C_;  // 0x1B2C
  uint32_t field_1B30_;  // 0x1B30
  uint32_t field_1B34_;  // 0x1B34
  uint32_t field_1B38_;  // 0x1B38
  uint32_t field_1B3C_;  // 0x1B3C
  uint32_t field_1B40_;  // 0x1B40
  uint32_t field_1B44_;  // 0x1B44
  uint32_t field_1B48_;  // 0x1B48
  uint32_t field_1B4C_;  // 0x1B4C
  uint32_t field_1B50_;  // 0x1B50
  uint32_t field_1B54_;  // 0x1B54
  uint32_t field_1B58_;  // 0x1B58
  uint32_t field_1B5C_;  // 0x1B5C
  uint32_t field_1B60_;  // 0x1B60
  uint8_t field_1B64_[0x4];  // 0x1B64
  uint32_t field_1B68_;  // 0x1B68
  uint32_t field_1B6C_;  // 0x1B6C
  uint8_t field_1B70_[0x8];  // 0x1B70
  uint32_t field_1B78_;  // 0x1B78
  uint32_t field_1B7C_;  // 0x1B7C
  uint32_t field_1B80_;  // 0x1B80
  uint8_t field_1B84_[0x1C];  // 0x1B84
  uint32_t field_1BA0_;  // 0x1BA0
  uint32_t field_1BA4_;  // 0x1BA4
  uint32_t field_1BA8_;  // 0x1BA8
  uint32_t field_1BAC_;  // 0x1BAC
  uint32_t field_1BB0_;  // 0x1BB0
  uint32_t field_1BB4_;  // 0x1BB4
  uint32_t field_1BB8_;  // 0x1BB8
  uint32_t field_1BBC_;  // 0x1BBC
  uint32_t field_1BC0_;  // 0x1BC0
  uint32_t field_1BC4_;  // 0x1BC4
  uint32_t field_1BC8_;  // 0x1BC8
  uint32_t field_1BCC_;  // 0x1BCC

  // Storage the image proves is here: its own methods reach 0x1BDC.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  uint32_t field_1BD0_;  // 0x1BD0
  uint32_t field_1BD4_;  // 0x1BD4
  uint32_t field_1BD8_;  // 0x1BD8
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

void __cdecl synch_veh(int id);
void __cdecl synch_base(int id);
void __cdecl synch_energy(int id);
void __cdecl synch_researching(int id);
void __cdecl synch_leader(int id);
void __cdecl synch_ai(int id);
void __cdecl synch_research(int id);
void __cdecl synch_alloc(int id);
void __cdecl synch_soc(int id);
void __cdecl synch_proto(int id);
void __cdecl synch_obs(int id);
void __cdecl synch_diplo(int a, int b);
void __cdecl synch_template(int id);
void __cdecl synch_radius(int id);
