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
#include "lock.h"        // locks_ - the whole network-lock block
#include "playerlock.h"  // its records are PlayerLocks

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
  // PUBLIC: access specifiers change nothing about layout, and this
  // tree's bodies reach base methods the image reaches with a direct
  // `call rel32`. Spelled `class X : Base` - private, since that is
  // what `class` means - those calls do not compile at all, and the
  // seam that stood in for them cost the caller `call [ptr]`.
class NetDaemon : public AlphaNet {
 public:
  // 0x00532E00, a pending_bodies forwarder.
  void synch(int16_t opcode, int a, int b, int c, char *text, int d, int16_t flags);

 public:
  // 0x00534400, a pending_bodies forwarder.
  void process_message(char *message, unsigned long a, int b);

 public:
  NetDaemon();
  ~NetDaemon();
  int receive();
  uint32_t unlock_veh();

  // Storage the image proves is here: its own methods reach 0x1BD0.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  // THE NETWORK LOCKS - one whole Lock at 0x14A0: records_[8] are the eight
  // per-player locks (0x14A0..0x157F) and field_E0_/E4_/E8 are the dwords at
  // 0x1580/0x1584/0x1588.
  Lock locks_;

  // THE NET LOCK TABLE, 0x158C..0x1B2C: 24 entries of 0x3C. The constructor
  // sets each entry's flag byte to 0xFF and its word at +8 to 0.
  struct LockTableEntry {
    uint8_t flag_;        // 0xFF once constructed
    uint8_t pad_1_[7];
    uint16_t word_8_;
    uint8_t pad_A_[0x32];
  };
  static_assert(sizeof(LockTableEntry) == 0x3C, "lock table stride");
  LockTableEntry lock_table_[24];  // 0x158C

  // THE LOCK-TABLE STATE - parallel to lock_table_ (each entry's state dword
  // sits 0x5A0 past its lock-table twin, so the ctor's merged loop writes
  // [entry + 0x5A0], [entry] and [entry + 8] per iteration). Entries e0..e2
  // carry the named dwords the constructor clears; e3..e23 are unreached by
  // any recovered body.
  struct LockStateEntry {
    uint32_t dword_[15];
  };
  LockStateEntry lock_state_[24];  // 0x1B2C..0x2090
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
};

// Net::get and NetDaemon::process_message are not recovered yet; the Net the
// daemon reads from is THE OBJECT below, at 0x0093CD90 in the image - REAL
// STORAGE, defined in netdaemon.cpp. It was
// `NetDaemon *const NetDaemonNet = (NetDaemon *)0x0093CD90` naming terranx.exe
// data that is unmapped in a standalone build. The image's ??__E dynamic
// initializer at 0x0052DB00 runs the real NetDaemon constructor against it
// and registers the deleting destructor with atexit; here the same recovered
// constructor runs through this build's own startup. Every `NetDaemonNet->`
// receiver keeps its folded `mov ecx, imm32` form, the displacement
// relocated, and `->` becomes `.`.
extern NetDaemon NetDaemonNet;  // 0x0093CD90


// The multiplayer-transport flag at 0x0093F660 and the local faction id at
// 0x00939284, both read by unlock_veh. src/game.cpp binds the same two
// addresses as IsMultiplayerNet / LocalFaction, but that translation unit is
// not linked into recovery-leaf-tests, so NetDaemon owns its own rebindable
// pair the way src/spying.cpp owns SpyingCurrentFaction for the very
// same faction address.
extern int NetDaemonIsMultiplayerNet;
extern int NetDaemonLocalFaction;

// message_data at 0x00592EE0 broadcasts one game event; it is still an
// original dependency (it packs a 0x20-byte record and dispatches
// NetDaemon::send_message at 0x00532940), so unlock_veh reaches it through a
// rebindable seam. Declared returning uint32_t even though the original is
// ?message_data@@YAXHHHHHH@Z: unlock_veh's own EAX residue on the announce
// path IS this call's EAX, so the value has to be passed through rather than
// discarded.
// 0x00592EE0, a pending_bodies forwarder. The catalogued name spells the
// return `X` - void - but the body leaves a value in EAX that unlock_veh
// returns, so the forwarder is typed `uint32_t` and callers that do not want
// it discard it.
uint32_t __cdecl message_data(int a1, int a2, int a3, int a4, int a5, int a6);


// NetDaemon::synch itself is not recovered - 4,905 bytes with its own call
// targets, so it remains an original dependency. The fourteen synch_*
// forwarders below all funnel into it, loading the same daemon at 0x0093CD90
// that NetDaemonNet already binds above, and differ only in the opcode (and,
// for synch_diplo, which of the two leading arguments carries which value).

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
