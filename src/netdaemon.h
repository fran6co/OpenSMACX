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
// The lock-table entry (0x158C in NetDaemon) and its parallel state entry.
struct LockTableEntry {
  uint8_t flag_;        // the constructor sets this byte to 0xFF
  uint8_t pad_1_[7];
  uint16_t word_8_;
  uint8_t pad_A_[0x32];
};
static_assert(sizeof(LockTableEntry) == 0x3C, "lock table stride");

// ONE LOCK-STATE ENTRY, 0x3C bytes. Which entry and which dword a NetDaemon
// offset names: lock_state_ starts at 0x1B2C, so
// `offset - 0x1B2C` / 0x3C picks the entry and the remainder / 4 picks the
// dword. THE MAP, as far as the recovered bodies pin it -
//
//   0x1B2C  [0].dword_[0]   the shared checksum sequence, incremented per
//                           forwarded message and stamped into message+0xC
//   0x1B30  [0].dword_[1]   the diplomacy/wait marker hang_up, end_of_event
//                           and process_message's modal guard all test
//   0x1B3C  [0].dword_[4]   add_global's "one global lock is held" flag
//   0x1B60  [0].dword_[13]  cleared by the constructor
//   0x1BA0  [1].dword_[14]  the diplomacy-channel flag close_channel clears
//   0x1B68  [1].dword_[0]   cleared by the constructor
//   0x1B74  [1].dword_[3]   cleared by the constructor; process_message reads
//   0x1B78  [1].dword_[4]   THE VEHICLE RECORD - lock_veh stores the unit
//                           this client holds, unlock_veh clears it
//   0x1B7C  [1].dword_[5]   the per-faction mask hang_up ANDs against
//   0x1B80  [1].dword_[6..9] one mask per faction, indexed by faction id
//   0x1BAC  [2].dword_[2]   \ the four announce-side fields unlock and
//   0x1BB0  [2].dword_[3]   \ unlock_veh clear in the order 3, 2, 10, 9
//   0x1BB8  [2].dword_[5]   / (process_message reads dword_[5])
//   0x1BC0  [2].dword_[7]   end_of_event tests and clears
//   0x1BC4  [2].dword_[8]   unlock_veh clears with the vehicle record
//   0x1BC8  [2].dword_[9]   / see 0x1BAC
//   0x1BCC  [2].dword_[10]  / maybe_lock counts repeat requests here
//   0x1BD0  [2].dword_[11]  the ctor sets this one to 0xFF, the only byte it
//                           does not zero
//
// Entries 3..23 are reached by no recovered body. The console's bodies read
// the same run through `NetDaemonNet.lock_state_` - it is the 0x0093E8BC run
// in the image - which is why the struct and the member are public.
struct LockStateEntry {
  uint32_t dword_[15];
};
static_assert(sizeof(LockStateEntry) == 0x3C, "lock state stride");

class NetDaemon : public AlphaNet {
 public:
  // homed from 00534330.cpp
  int fixup_message(char *message);

 public:
  // homed from 00531480.cpp
  void await_synch();

 public:
  // homed from 00531300.cpp
  void await_exec(int opcode);

 public:
  // homed from 00530e50.cpp
  void unlock_base(int base_id);

 public:
  // homed from 00530aa0.cpp
  int add_global();

 public:
  // homed from 00530920.cpp
  int add_lock(int target, int x, int y);

 public:
  // homed from 005304a0.cpp
  int open_channel(int payload_10, int payload_14);

 public:
  // homed from 005303b0.cpp
  int wait(char *message, int *state, int value, int timeout);

 public:
  // homed from 0052df30.cpp
  int init(int option);

 public:
  // 0x00532940. Every message builder in netdaemon.cpp hands its record here.
  void send_message(char *message, unsigned long length, int target);
  // 0x00530630, a pending_bodies forwarder (archived in 00530630.cpp). It
  // broadcasts a lock request, then polls for every client's ack - which is
  // the wait loop unlock_base and await_synch spell out inline. The three
  // trailing ints pass through to the record's 0x18/0x1C/0x20 slots.
  int lock(int target, int x, int y, int payload_18, int payload_1C,
           int payload_20);
  // 0x00531020. Takes the vehicle BY POINTER: the pointer itself is what the
  // daemon stores to remember which unit this client holds. Declaring it here
  // is what unlocks the eleven Console bodies that lock a unit over the net.
  int lock_veh(int *veh_id, int target, int payload_1C, int payload_20,
               int payload_18);

 public:
  // 0x005328C0, homed from 005328c0.cpp. faction_id names the event's owner;
  // turn_value is stored into the lock state when no turn is already marked.
  int end_of_event(int faction_id, int turn_value);

 public:
  // 0x0052F8E0, homed from 0052f8e0.cpp. Writes the running game's checksum
  // sequence into the caller's int array.
  void game_checksum(int *results);

 public:
  // 0x0052F520, homed from 0052f520.cpp
  void send_rules();

 public:
  // 0x0052E900, homed from 0052e900.cpp
  void send_game();

 public:
  // 0x0052E050, homed from 0052e050.cpp
  void hang_up();
  // 0x00530360, homed from 00530360.cpp - one pump turn of the daemon.
  void net_tasks();
  // 0x00531700, homed from 00531700.cpp - the "am I already holding it"
  // wrapper around lock_veh, counting repeat requests at 0x1BCC.
  int maybe_lock(int *veh_id, int flag, int payload_1C, int payload_20,
                 int payload_18, int extra);

 public:
  // 0x00532E00, a pending_bodies forwarder.
  void synch(int16_t opcode, int a, int b, int c, char *text, int d, int16_t flags);

 public:
  // 0x00534400, a pending_bodies forwarder.
  void process_message(char *message, unsigned long a, int b);

 public:
  // 0x005315C0, a pending_bodies forwarder. The net_* treaty/agenda writers
  // in faction.cpp wait on this after broadcasting a move, so the receiving
  // side's ack comes back before play continues.
  void await_diplo(int opcode);
  // diplo_lock (0x00539820, not yet homed) also polls add_global
  // (0x00530AA0) until the diplomacy lock lands; declare it when that body
  // comes across.
  void unlock();
  void close_channel();

 public:
  NetDaemon();
  ~NetDaemon();
  int receive();
  // 0x00531780. The client-side half of a move: lock_veh the unit under the
  // same bookkeeping the free order_veh (0x005947C0) does host-side, and
  // report whether the lock landed. Declared for the veh.cpp action family,
  // which calls it directly at 0x004CB399 / 0x004CB9FD.
  int order_veh(int veh_id, int payload_10, int payload_1C);
  // 0x00532570, still the image's own bytes (forwarded in pending_bodies.cpp).
  // enemy_veh reaches it after its own await_synch, asking the daemon to run
  // the vehicle's order; mode 1 is the only value any caller passes.
  int action(int veh_id, int mode);
  uint32_t unlock_veh();

  // Storage the image proves is here: its own methods reach 0x1BD0.
  // Extent only - this class carries no size assertion, and the bound is a floor.
  // THE NETWORK LOCKS - one whole Lock at 0x14A0: records_[8] are the eight
  // per-player locks (0x14A0..0x157F) and field_E0_/E4_/E8 are the dwords at
  // 0x1580/0x1584/0x1588.
  Lock locks_;

  // THE NET LOCK TABLE, 0x158C..0x1B2C: 24 entries of 0x3C. The constructor
  // sets each entry's flag byte to 0xFF and its word at +8 to 0.
  LockTableEntry lock_table_[24];  // 0x158C

  // THE LOCK-TABLE STATE - the array AFTER lock_table_ (24 * 0x3C = 0x5A0
  // bytes later), which is why the ctor's merged loop writes [entry+0x5A0]
  // alongside [entry] and [entry+8] with one pointer. Entries e0..e2
  // carry the named dwords the constructor clears; e3..e23 are unreached by
  // any recovered body - the per-entry map is on LockStateEntry above.
  // Spans 0x1B2C..0x20CC, which is the image's 0x0093E8BC..0x009C0BFC run.
  LockStateEntry lock_state_[24];  // 0x1B2C

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

// The wait-loop callees the daemon's lock and synch bodies poll through, so
// those bodies can home here without dragging in the whole window stack.
// Both are forwarders in pending_bodies.cpp; wait_task is the image's
// ?wait_loop@@YAXXZ, renamed for what this tree's callers do with it.
void __cdecl wait_task();     // 0x0051F200
void __cdecl check_spock();   // 0x0051F1B0, defined in netdaemon.cpp
void __cdecl do_all_draws();  // also declared in temp.h, which this header
                              // deliberately does not pull in
// 0x004CC360 and 0x005947C0, both forwarders: the veh.cpp action family is
// still archived, and the daemon's own veh_drop / order_veh dispatch into it.
void __cdecl action_airdrop(int faction_id, int x, int y, int mode);
int __cdecl order_veh(int veh_id, int x, int y);


// The multiplayer-transport flag at 0x0093F660 and the local faction id at
// 0x00939284, both read by unlock_veh. src/game.cpp binds the same two
// addresses as IsMultiplayerNet / LocalFaction, but that translation unit is
// not linked into recovery-leaf-tests, so NetDaemon owns its own rebindable
// pair the way src/spying.cpp owns SpyingCurrentFaction for the very
// same faction address.
extern int NetDaemonIsMultiplayerNet;
extern int NetDaemonLocalFaction;

// The message builders at 0x00592E10..0x00593215, defined in netdaemon.cpp.
// They pack a record on the stack, stamp it with the local faction, the
// wall-clock time and the daemon's turn counter, then hand it to
// NetDaemon::send_message with NetBusyCount held for the duration. Payload
// parameters are named for the record offset they land at, which is the one
// thing about them the bodies prove.
//
// message_data is NOT here: see netdaemon.cpp's note by the family. Its
// catalogued return is void, but EAX is live at its ret and unlock_veh
// returns that residue, so it stays a pending_bodies forwarder.
void __cdecl message_veh(int opcode, int payload_10, int payload_14,
                         int target);
void __cdecl message_base(int opcode, int payload_10, char *text, int target);
uint32_t __cdecl message_data(int opcode, int target, int payload_10,
                              int payload_14, int payload_18, int payload_1C);
void __cdecl message_big_data(int opcode, int target, int *payload, int count);
void __cdecl message_lock(int opcode, int target, int payload_10,
                          int payload_14, int payload_18, int payload_1C,
                          int payload_20, int payload_24);
void __cdecl message_landmark(int opcode, int target, char *text,
                              int payload_10, int payload_14, int payload_18,
                              int payload_1C);
void __cdecl message_chat(int opcode, int target, char *text);


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
