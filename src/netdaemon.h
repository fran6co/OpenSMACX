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
#include "deletionlist.h"
#include "lock.h"        // locks_ - the whole network-lock block
#include "playerlock.h"  // its records are PlayerLocks
#include <stddef.h>

// Turn checksum wire records contain 21 signed dwords. The peer slot comes
// from the message header; its range is not established by this declaration.
enum { NetTurnChecksumWords = 21, NetTurnChecksumPeerSlots = 8 };
extern int NetTurnChecksums[NetTurnChecksumWords];
extern int NetPeerTurnChecksums[NetTurnChecksumPeerSlots][NetTurnChecksumWords];

 /*
  * NetDaemon class
  *
  * Derives from AlphaNet on an unadjusted `this`. receive() polls the network
  * for one message and dispatches it, but it reads that message from the Net
  * object at a fixed address rather than through `this` - the original loads
  * 0x0093CD90 into ecx before calling Net::get - so that global is a
  * rebindable seam here, not the daemon itself.
  *
  * Recovered bodies establish the lock and parallel state tables after the
  * AlphaNet base. Their channel-dependent payload distinguishes ordinary
  * flags from a borrowed vehicle-ID pointer; other words retain opaque names
  * where their meaning is not established. This is not a complete semantic
  * recovery of the daemon's remaining storage.
  */
  // PUBLIC: access specifiers change nothing about layout, and this
  // tree's bodies reach base methods the image reaches with a direct
  // `call rel32`. Spelled `class X : Base` - private, since that is
  // what `class` means - those calls do not compile at all, and the
  // seam that stood in for them cost the caller `call [ptr]`.
// Native channel offsets below use the former conceptual channel0..2
// numbering. Channel0's first dword is DeletionList.current_owner_, not a
// second object. Its remaining bytes are PrimaryLockStateTail; channels1/2
// are the two LockStateEntry objects. The map is supported by callers:
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
//   0x1B78  [1].payload_   borrowed int* vehicle-ID slot supplied to lock_veh;
//                           unlock_veh clears it, not the pointed-to ID
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
// Runtime accesses establish only channels0..2. The file-index pointer and
// saved process slots afterward are disjoint fields, not another21 channels.
// 0x538A64 branches to0x538A59, after the one-time owner clear at0x538A53.
struct PrimaryLockStateTail {
  uint32_t leading_words_[3];
  uint32_t flags_;
  uint32_t trailing_words_[10];
};
static_assert(sizeof(PrimaryLockStateTail) == 0x38,
              "primary channel excludes the deletion owner's dword");
static_assert(offsetof(PrimaryLockStateTail, flags_) == 0xC,
              "primary channel flag offset from its tail");
static_assert(offsetof(PrimaryLockStateTail, trailing_words_) == 0x10,
              "primary channel trailing words offset");
struct LockStateEntry {
  uint32_t leading_words_[4];
  // Channel0/2 use flags; channel1 borrows a vehicle-ID slot. These are
  // distinct active union members, not an integer view of a pointer.
  union Payload {
    uint32_t flags;
    int *vehicle_selection_follow;
  } payload_;
  uint32_t trailing_words_[10];
};
static_assert(sizeof(void *) == 4, "native lock-state pointer width");
static_assert(offsetof(LockStateEntry, payload_) == 0x10,
              "lock-state channel payload");
static_assert(offsetof(LockStateEntry, trailing_words_) == 0x14,
              "lock-state trailing words");
static_assert(sizeof(LockStateEntry) == 0x3C, "lock state stride");

// Persistent save/restore storage begins at NetDaemon+0x1BE8.
// The native secondary initializer writes the same five leading bytes as an
// AlphaNet process record at 0x93E99C + n*0x19C; save_daemon copies the eight
// complete live records over this view, and game_data transfers the same
// 0xCE0-byte interval. This POD view has no snapshot constructor/destructor.
struct NetDaemonSaveProcessSlot {
  char name_[0x20];
  uint32_t pid_;
  int8_t identity_;
  uint8_t field_25_;
  uint8_t field_26_;
  int8_t field_27_;
  uint8_t status_bytes_[0x174];
};
static_assert(offsetof(NetDaemonSaveProcessSlot, pid_) ==
                  offsetof(AlphaNetProcessSlot, pid_),
              "saved process-slot PID matches live layout");
static_assert(offsetof(NetDaemonSaveProcessSlot, identity_) == 0x24,
              "saved process-slot identity offset");
static_assert(offsetof(NetDaemonSaveProcessSlot, identity_) ==
                  offsetof(AlphaNetProcessSlot, identity_),
              "saved process-slot identity matches live layout");
static_assert(offsetof(NetDaemonSaveProcessSlot, status_bytes_) == 0x28,
              "saved process-slot status offset");
static_assert(offsetof(NetDaemonSaveProcessSlot, status_bytes_) ==
                  offsetof(AlphaNetProcessSlot, status_bytes_),
              "saved process-slot status matches live layout");
static_assert(sizeof(NetDaemonSaveProcessSlot) == 0x19C,
              "saved process-slot stride");
static_assert(sizeof(NetDaemonSaveProcessSlot) == sizeof(AlphaNetProcessSlot),
              "saved process-slot matches live stride");
static_assert(sizeof(NetDaemonSaveProcessSlot) * 8 == 0xCE0,
              "saved process-slot table extent");

class NetDaemonTransferModalCloseGuard;
class Filemap;
struct NetDaemonFileIndexRecord;

// Original daemon construction first constructs AlphaNet at this+0.
// RECOVERY_BASE: v1 NetDaemon AlphaNet 0x0 0x005389F0
class NetDaemon : public AlphaNet {
 public:
  // Original 0x00538B21 publishes this table. Only slots 1/2 differ from
  // AlphaNet's six slots; their bodies still need seed recovery. Original
  // Net dispatch requires slot 5, while the word at 0x0066EF24 is neither
  // relocated nor a code address, bounding this primary table independently
  // of the candidate declaration (see Net's inheritance evidence).
  // RECOVERY_GENERATED: v1 vtable 0x0066EF0C 0x0066EF24 ??_7NetDaemon@@6B@ 0x005389F0
  virtual void on_player_added(char *name, unsigned long id);
  virtual void on_player_deleted(char *name, unsigned long id);
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
  // Lock a base through its coordinates; the last two arguments are passed
  // unchanged to lock's payload slots, not interpreted as coordinates here.
  int lock_base(int base_id, int target, int payload_1C, int payload_20);

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
  int wait(char *message, uint32_t *state, int value, int timeout);

 public:
  // homed from 0052df30.cpp
  int init(int option);

 public:
  // 0x00532940. Every message builder in netdaemon.cpp hands its record here.
  // Native void-decorated method leaves Net::send's EAX live (zero offline).
  int send_message(char *message, unsigned long length, int target);
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
  int automated_lock(int *veh_id, int target);

  // Borrowed selection storage already owned by channel1 at this+0x1B78.
  // Callers retain the original callback/clear ordering and lifetime obligations.
  void set_vehicle_selection_follow(int *slot) {
    channels_[0].payload_.vehicle_selection_follow = slot;
  }
  int *vehicle_selection_follow() const {
    return channels_[0].payload_.vehicle_selection_follow;
  }

  // Shared faction wait mask, at NetDaemon+0x1B5C (entry 0 trailing dword 7).
  // The original address is also read by callers outside this class; keeping
  // the accessor on the genuine owner avoids a second scalar storage object.
  uint32_t &factions_waiting_mask() {
    return primary_state_.trailing_words_[7];
  }
  const uint32_t &factions_waiting_mask() const {
    return primary_state_.trailing_words_[7];
  }
  // MessageWin's network message count shares entry 2, trailing dword 7.
  uint32_t &network_message_count() {
    return channels_[1].trailing_words_[7];
  }
  const uint32_t &network_message_count() const {
    return channels_[1].trailing_words_[7];
  }

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
  void reset(); // original sub_51dd00, same NetDaemon receiver
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
  // 0x00534400: partial source dispatcher; higher protocol bands remain
  // unfinished. Source presence is not full protocol-behavior coverage.
  void process_message(char *message, unsigned long message_size, int sender_id);
  int send_files();
  bool process_file_transfer_message(char *message, unsigned long message_size,
                                     int sender_id,
                                     NetDaemonTransferModalCloseGuard &guard,
                                     Filemap &filemap);

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
  // 0x00532140 is the daemon's network-aware vehicle drop, distinct from
  // the free veh_drop helper in veh.cpp.
  int veh_drop(int veh_id, int x_coord, int y_coord);
  // 0x00532570: source-owned multiplayer action and acknowledgement loop.
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

  // fixup_message534330 passes this+158C to DeletionList::convert52DDC0.
  // veh_kill5C08C0 passes93E31C to add_deletion52DE70; that is exactly
  // NetDaemonNet93CD90+158C. There is one embedded object/lifetime, not a
  // separate VehDeletionList global. current_owner_ also owns sequence1B2C.
  // RECOVERY_MEMBER: v1 NetDaemon deletion_list_ 0x158C 0x00534330
  DeletionList deletion_list_;
  // RECOVERY_MEMBER: v1 NetDaemon primary_state_ 0x1B30 0x005389F0
  PrimaryLockStateTail primary_state_;
  // RECOVERY_MEMBER: v1 NetDaemon channels_ 0x1B68 0x005389F0
  LockStateEntry channels_[2];
  // The constructor leaves the file-index pointer and save rows untouched.
  // RECOVERY_MEMBER: v1 NetDaemon file_index_records_ 0x1BE0 0x0052ED40
  NetDaemonFileIndexRecord *file_index_records_;
  uint32_t field_1BE4_;
  // Separate from AlphaNet's live process_slots_ at +0x76C.
  // RECOVERY_MEMBER: v1 NetDaemon saved_process_slots_ 0x1BE8 0x0052DB40
  NetDaemonSaveProcessSlot saved_process_slots_[8];

  // Observed storage reaches+0x28C8 through the persistent save table.
  // Extent only: this is not proof of complete original sizeof.
  // Separate NetPlayerDropped93F65C and network flags93F660 remain globals.
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
extern int NetBusyCount;  // 0x0093F664, network exchanges currently in flight
extern unsigned int NetDaemonProgressInterval;  // distinct 0x0093F668 dword
extern unsigned int NetDaemonProgressLastTick;  // distinct 0x0093F66C dword
extern int CommlinkPending;     // 0x007492CC
extern int CommlinkFaction;     // 0x0073B790

// The wait-loop callees the daemon's lock and synch bodies poll through, so
// those bodies can home here without dragging in the whole window stack.
// Both are forwarders in pending_bodies.cpp; wait_task is the image's
// ?wait_loop@@YAXXZ, renamed for what this tree's callers do with it.
void __cdecl wait_task();     // 0x0051F200
void __cdecl check_spock();   // 0x0051F1B0, defined in netdaemon.cpp
void __cdecl do_all_draws();  // also declared in temp.h, which this header
                              // deliberately does not pull in
// Airdrop is recovered in veh.cpp. order_veh remains an unfinished forwarder;
// the daemon's network methods dispatch through these typed contracts.
int __cdecl action_airdrop(int veh_id, int x, int y, int mode);
int __cdecl order_veh(int veh_id, int x, int y);


// Transport state is game.h's IsMultiplayerNet; faction state is owned by
// ConsoleGlobal. The daemon does not allocate duplicates of either.

// The message builders at 0x00592E10..0x00593215, defined in netdaemon.cpp.
// They pack a record on the stack, stamp it with the local faction, the
// wall-clock time and the daemon's turn counter, then hand it to
// NetDaemon::send_message with NetBusyCount held for the duration. Payload
// parameters are named for the record offset they land at, which is the one
// thing about them the bodies prove.
//
// message_data's source type exposes the live 32-bit EAX result; its original
// decorated name remains void. Only unlock_veh consumes the value in current
// source; the other callers ignore it.
void __cdecl message_veh(int opcode, int payload_10, int payload_14,
                         int target);
void __cdecl message_base(int opcode, int payload_10, char *text, int target);
int __cdecl message_data(int opcode, int target, int payload_10,
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

void __cdecl wait_2();

// Source-owned original helper; descriptive name, not original debug spelling.
void __cdecl netdaemon_send_message_counted(char *message, unsigned long length, int target);
