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
#include "net_class.h"
#include "lock.h"
#include "playerlock.h"

#include "stringstruct.h"  // StringAllocationHeap - the 0x009B3374 hand-off
#include "netmsg.h"        // NetMsgPopup, which the lock/wait bodies pop through
#include "basepop.h"       // NetMsg's base, which process_message reaches
#include "netwin.h"        // NetWinDialog - the setup dialog process_message dispatches into
#include "veh.h"           // Vehs / VehCurrentCount, which game_checksum walks
#include "map.h"           // the map bounds, PlayersData and Bases it sums
#include "technology.h"    // GameTechAchieved
#include "game.h"          // IsMultiplayerNet: the application's shared flag

int NetDaemonIsMultiplayerNet;  // 0x0093F660
int NetDaemonLocalFaction;  // 0x00939284

// The next dword after NetDaemonIsMultiplayerNet. Every sender in this file
// increments it around one send_message call, every wait loop increments it
// for the length of its poll and decrements it on the way out, and
// MultiWin::draw_time (0x00478BE0) declines to draw the turn clock unless it
// is zero - so it counts the network exchanges currently in flight.
int NetBusyCount;  // 0x0093F664

// The replay recorder's event count: ReplayWin::timer_callback
// (0x005AD9E0) walks its recorded events while its cursor is below it and
// setup_game (0x005B3920) clears it with the rest of the per-game state.
// send_game synchs the replay only when it is nonzero.
int ReplayEventCount;  // 0x009469F8

// Four bytes below NetDaemonIsMultiplayerNet. on_player_deleted
// (0x0052E370) sets it when a client leaves; a lock whose wait saw it comes
// back with the "NOLOCK" popup instead of a lock.
int NetPlayerDropped;  // 0x0093F65C

// Set as a pair by commlink_attempt (0x00558C60): the flag and the faction
// that asked. process_message tests both when a commlink reply arrives.
int CommlinkPending;   // 0x007492CC
int CommlinkFaction;   // 0x0073B790

// The spock pair: synch (0x00532E00) raises 0x93F670 when it has something to
// notify, check_spock (below) sees it, arms 0x93F674 and calls help_spock
// (0x0051F0C0), which clears both after re-synching the session.
int SpockNotifyPending;  // 0x0093F670
int SpockHelpActive;     // 0x0093F674

extern int BaseAtDebugPopupFlag;  // 0x0093A94C, defined in base.cpp

// The same source storage exported by console.h, without including its
// window hierarchy just for this scalar declaration.
extern int ConsoleControlTurnActive;

// ORIGINAL: 0x00530D50 ?lock_base@NetDaemon@@QAEHHHHH@Z 0x00530D50-0x00530E48
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00530D50-0x00530E48"], "reason": "Recover the base-menu network guard through real Base records, shared turn state and NetDaemon lock-state members; original calls establish argument forwarding and log strings."}
int NetDaemon::lock_base(int base_id, int target, int payload_1C, int payload_20) {
    log_say("LockBase", UnkGlobal0093A934, ConsoleControlTurnActive,
            BaseIDCurrentSelected);
    if (!IsMultiplayerNet) {
        return 0;
    }
    if (UnkGlobal0093A934) {
        Bases[base_id].state |= BSTATE_NET_LOCKED;
        message_data(0x4304, 0, 0, 0, 0, 0);
        return 0;
    }
    if (ConsoleControlTurnActive) {
        return 1;
    }
    if (lock_state_[0].dword_[3]) {
        return 1;
    }
    Base &base = Bases[base_id];
    log_say("LockBase Needs Lock", base_id, base.x, base.y);
    return lock(target | 4, base.x, base.y, 0, payload_1C, payload_20);
}

// The map row stride in dwords, read as a value. mapwin.cpp keeps the same
// fixed-address binding file-local and no header exports it; game_checksum's
// per-row walk multiplies it by the row index to reach a tile record.
static int *const MapRowStride = (int *)0x0068FAF0;

// Two nameless __thiscall helpers the catalogue has no mangled name for -
// the call sites set up a `this` pointer first. Shims only, the same
// reasoning as the FileWin/BasePop recoveries' casts: the symbol they
// resolve to is a relocation on both sides and is masked out of the
// comparison the same way an address literal is.
class RemoveAllShim {
 public:
  void remove_all();  // 0x00402970
};
void RemoveAllShim::remove_all() {
    typedef void(__fastcall *pending)(RemoveAllShim *, void *);
    reinterpret_cast<pending>(static_cast<unsigned long>(0x00402970))(this, nullptr);
}
class Sub401be0Shim {
 public:
  void run();  // 0x00401BE0
};
void Sub401be0Shim::run() {
    typedef void(__fastcall *pending)(Sub401be0Shim *, void *);
    reinterpret_cast<pending>(static_cast<unsigned long>(0x00401BE0))(this, nullptr);
}
// A Heap, reached only by raw offset here - see the note on the destructor.
class HeapShim {
 public:
  void shutdown();  // 0x005D45B0
};
void HeapShim::shutdown() {
    typedef void(__fastcall *pending)(HeapShim *, void *);
    reinterpret_cast<pending>(static_cast<unsigned long>(0x005D45B0))(this, nullptr);
}
// Receiver cleanup uses the shared source-owned helper, not an image-address shim.
class NetFifoShim {
 public:
  ~NetFifoShim();
};
NetFifoShim::~NetFifoShim() {
    typedef void(__fastcall *pending)(NetFifoShim *, void *);
    reinterpret_cast<pending>(static_cast<unsigned long>(0x006339E0))(this, nullptr);
}


/*
Purpose: Build a network daemon - its AlphaNet base, then its own eight
         PlayerLocks and 0x18-entry lock table.
// ORIGINAL: 0x005389F0 ??0NetDaemon@@QAE@XZ 0x005389F0-0x00538B58;0x0065DF50-0x0065DF62
// size      360 bytes
// prototype void (__thiscall ??0NetDaemon@@QAE@XZ)(NetDaemon* this)
// callers   1   call targets   4
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004E2490 0x0058FF70 0x005900D0 0x004E25B0
Return Value: this

NetDaemon has no virtual base anywhere in this chain, so a genuine
constructor is safe (unlike MapWin/PlanWin/Console - see the note in
`mapwin.h`), and the image's own SEH frame here is what a real constructor
with a real destructible base earns. NetDaemon's own fields past AlphaNet
are still opaque storage (see the class declaration), so the eight
PlayerLocks and the 0x18-entry lock table reach their elements by raw
offset rather than through invented array members.
*/
NetDaemon::NetDaemon() : AlphaNet() {
    // The eight per-player locks (0x14A0, 0x1C stride), then the embedded
    // Lock of the first one cleared directly - the image calls the Lock
    // version explicitly, twice (here and again after the table below).
    for (int32_t n = 0; n < 8; ++n) {
        locks_.records_[n].clear();
    }
    locks_.clear();

    for (int32_t m = 0; m < 0x18; ++m) {
        lock_table_[m].flag_ = 0xFF;
        lock_table_[m].word_8_ = 0;
        // the per-entry scratch dword lives past the entry, at +0x5A0
        lock_state_[m].dword_[0] = 0;
    }

    lock_state_[0].dword_[1] = 0;
    lock_state_[0].dword_[2] = 0;
    lock_state_[0].dword_[3] = 0;
    lock_state_[0].dword_[4] = 0;
    lock_state_[0].dword_[13] = 0;
    lock_state_[0].dword_[12] = 0;
    lock_state_[1].dword_[1] = 0;
    lock_state_[1].dword_[0] = 0;
    lock_state_[1].dword_[4] = 0;
    lock_state_[2].dword_[1] = 0;
    lock_state_[1].dword_[14] = 0;
    lock_state_[2].dword_[0] = 0;
    lock_state_[2].dword_[9] = 0;
    lock_state_[2].dword_[8] = 0;
    lock_state_[0].dword_[9] = 0;
    lock_state_[0].dword_[10] = 0;
    lock_state_[0].dword_[11] = 0;
    lock_state_[2].dword_[6] = 0;
    lock_state_[2].dword_[5] = 0;
    lock_state_[2].dword_[3] = 0;
    lock_state_[2].dword_[2] = 0;
    lock_state_[2].dword_[4] = 0;
    lock_state_[0].dword_[5] = 0;
    lock_state_[0].dword_[6] = 0;
    lock_state_[2].dword_[12] = 0;
    lock_state_[2].dword_[11] = 0;
    lock_state_[0].dword_[7] = 0;
    lock_state_[0].dword_[8] = 0;
    lock_state_[1].dword_[3] = 0;

    // 0x1BD0 is lock_state_[2]'s eleventh dword - the ctor sets it to 0xFF.
    lock_state_[2].dword_[11] = 0xFF;
    for (int32_t i = 0; i < 8; i++) {
        lock_state_[1].dword_[5 + i] = 0;
    }

    locks_.clear();
    AlphaNet::close();
}

/*
Purpose: Tear down a network daemon - its own lock table, then AlphaNet's
         and Net's fields, directly (no separate calls into their own
         destructors).
// ORIGINAL: 0x00538D10 ??1NetDaemon@@QAE@XZ 0x00538D10-0x00538EBE;0x004E3710-0x004E372B;0x0065E010-0x0065E0A5
// TRIED: 15/97 with 62 compiled instructions, and the gap is the class, not
//   the spelling. TWO things measured 2026-08-22. (1) call_diff says MORE: this
//   tree makes 13 direct calls where the image makes 12, and the extra one is
//   `??1Net@@QAE@XZ` - NOT written in this body. `NetDaemon : public AlphaNet :
//   public Net`, so a REAL `~NetDaemon()` emits the implicit base-destructor
//   chain; VC6 inlines `~AlphaNet` and calls `~Net` out of line, and the image
//   calls neither, having open-coded both bases' teardown as the field clears
//   and vtable resets this body writes by hand. Under /O1 the count is 14,
//   the fourteenth being `__EH_prolog`. (2) The image's body is a genuine C++
//   unwind state machine: `mov byte ptr [ebp - 4], N` steps through states
//   2,1,3,0,9,0xa,8,7,6,5,4,-1 between the calls, and reserves `sub esp, 8`
//   where this tree reserves `push ecx`. Those state writes come from member
//   and base subobjects the compiler is destroying itself, one protected
//   region per subobject - explicit `->~Shim()` calls on reinterpret_cast
//   pointers, which is what this body has, create no unwind states at all, so
//   no spelling of the current shim scaffolding can produce them. Reaching
//   this needs Net/AlphaNet/NetDaemon modelled with the real members at 0x58,
//   0xb0, 0xe8, 0x10c, 0x130, 0x72c, 0x748, 0x144c, 0x148c - the same layout
//   job the class comment above says is still open - not another pass over the
//   statements.
// size      430 bytes
// prototype void (__thiscall ??1NetDaemon@@QAE@XZ)(NetDaemon* this)
// callers   0   call targets   9
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00401BE0 0x00402970 0x00402DD0 0x004C8DB0 0x004E25B0 0x00538EC0 0x005D45B0 0x0062E010 0x006339E0
Return Value: n/a

MEASURED SHARED_TAIL in the artifact this was transcribed from: one span is
COMDAT-folded with another function, so no per-instruction verdict is well
defined there regardless of spelling. Kept as the faithful coverage
translation anyway. Before several of these calls the image also resets one
or two hand-maintained "vtable" pointers through the same vbtable-relative
pattern `GraphicWin::construct` uses (see graphicwin.cpp) - not reproduced
here field-by-field; the shims above carry the call sequence instead.
*/
NetDaemon::~NetDaemon() {

    char *const self = reinterpret_cast<char *>(this);

    AlphaNet::close();
    reinterpret_cast<HeapShim *>(self + 0x148c)->shutdown();

    reinterpret_cast<RemoveAllShim *>(self + 0x144c)->remove_all();
    *reinterpret_cast<int32_t *>(self + 0x1460) = 0;
    reinterpret_cast<RemoveAllShim *>(self + 0x144c)->remove_all();
    *reinterpret_cast<int32_t *>(self + 0x1460) = 0;
    StringAllocationHeap = *reinterpret_cast<Heap **>(self + 0x1488);

    Net::close();

    reinterpret_cast<RemoveAllShim *>(self + 0x72c)->remove_all();
    *reinterpret_cast<int32_t *>(self + 0x740) = 0;
    reinterpret_cast<Sub401be0Shim *>(self + 0x748)->run();
    StringAllocationHeap = *reinterpret_cast<Heap **>(self + 0x754);
    StringAllocationHeap = *reinterpret_cast<Heap **>(self + 0x75c);

    reinterpret_cast<NetFifoShim *>(self + 0x130)->~NetFifoShim();
    reinterpret_cast<NetFifoShim *>(self + 0x10c)->~NetFifoShim();
    reinterpret_cast<NetFifoShim *>(self + 0xe8)->~NetFifoShim();

    jackal_voice_rx_.clear();
    *reinterpret_cast<int32_t *>(self + 0xc4) = 0;
    StringAllocationHeap = *reinterpret_cast<Heap **>(self + 0xd0);

    reinterpret_cast<VoiceTx *>(self + 0x58)->~VoiceTx();
}

/*
Purpose: Poll the network for one message; dispatch it when one arrives and
         report whether it did. The message and its two parameters come back
         from Net::get through out-parameters.
// ORIGINAL: 0x00530320 ?receive@NetDaemon@@QAEHXZ 0x00530320-0x0053035E BYTE_EXACT
// size      62 bytes
// prototype int (__thiscall ?receive@NetDaemon@@QAEHXZ)(NetDaemon* this)
// callers   9   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00534400 0x00630A00
Return Value: 1 when a message was received and dispatched, 0 when none was
Status: Complete
Verification note: the original leaves both out-params as uninitialized stack
garbage for Net::get to fill and reads them only after a nonzero return, so
leaving them uninitialized here is equivalent by construction - Net::get
always sets both before this reads them on the taken path.
*/
int NetDaemon::receive() {
    unsigned long first;
    unsigned long second;
    const int result = NetDaemonNet.Net::get(&first, &second);
    if (result == 0) {
        return 0;
    }
    process_message(reinterpret_cast<char *>(result), second, static_cast<int>(first));
    return 1;
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

/*
Purpose: Release the vehicle lock this client is holding. In a net game, tell
         the log and the other players first - log_say names the faction that
         is releasing and message_data broadcasts event 0x2212 - then clear the
         four announce-side fields; in every game, clear the locked vehicle
         record at 0x1B78 and the flag at 0x1BC4. Only the transport flag at
         0x0093F660 gates the announce; any nonzero value is a net game.
// ORIGINAL: 0x005310F0 ?unlock_veh@NetDaemon@@QAEXXZ 0x005310F0-0x0053114A BYTE_EXACT
// symbol    ?unlock_veh@NetDaemon@@QAEIXZ
// size      90 bytes
// prototype void (__thiscall ?unlock_veh@NetDaemon@@QAEXXZ)(NetDaemon* this)
// callers   33   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00592EE0 0x006262F0
// LEVER: the six raw `object[0x1B.. / 4]` stores came out and the named
//        lock_state_ members went in - 0x1BAC/0x1BB0/0x1BC8/0x1BCC are
//        lock_state_[2].dword_[2/3/9/10], 0x1B78 is lock_state_[1].dword_[4]
//        and 0x1BC4 is lock_state_[2].dword_[8]. BYTE_EXACT held, so the
//        member run is pinned by a body that was already exact, and the
//        `reinterpret_cast<volatile uint32_t *>(this)` the raw spelling
//        needed is gone with it.
Return Value: EAX residue. The original is ?unlock_veh@NetDaemon@@QAEXXZ, but
              EAX is live at its ret: on the non-net path it is the loaded
              transport flag, which is zero exactly because that path was
              taken; on the net path it is message_data's residue. Modelled as
              uint32_t and returned, like GraphicWin::close and ListBox::close.
Status: Complete with temporary message_data original dependency
*/
uint32_t NetDaemon::unlock_veh() {
    // Loaded exactly once, and it is also the EAX the non-net path returns.
    uint32_t residue = static_cast<uint32_t>(NetDaemonIsMultiplayerNet);
    if (residue != 0) {
        // `cmp eax, edi` against a zeroed edi: any nonzero flag announces.
        log_say("Client releasing lock", NetDaemonLocalFaction, 0, 0);
        residue = message_data(0x2212, 0, 0, 0, 0, 0);
        // The four announce-side fields, in the original's deliberately
        // non-monotonic order: 1BB0, 1BAC, 1BCC, 1BC8.
        lock_state_[2].dword_[3] = 0;
        lock_state_[2].dword_[2] = 0;
        lock_state_[2].dword_[10] = 0;
        lock_state_[2].dword_[9] = 0;
    }
    // Shared tail: 0x1B78 is the vehicle record lock_veh (0x00531020) stores
    // and 0x1BC4 its held flag.
    lock_state_[1].dword_[4] = 0;
    lock_state_[2].dword_[8] = 0;
    return residue;
}


// The daemon's Net, 0x0093CD90 in the image - see netdaemon.h. The image's
// dynamic initializer at 0x0052DB00 constructs it at the fixed address
// before WinMain; the recovered NetDaemon constructor here runs through this
// build's own startup instead.
NetDaemon NetDaemonNet;  // 0x0093CD90


/*
Purpose: Release the global diplomacy lock this client holds. In a net game,
         say so on the log and broadcast event 0x2212, then clear the four
         announce-side fields; out of one there is nothing to release and the
         body does nothing at all.
// ORIGINAL: 0x00530D00 ?unlock@NetDaemon@@QAEXXZ 0x00530D00-0x00530D4E BYTE_EXACT
// size      54 bytes
// prototype void (__thiscall ?unlock@NetDaemon@@QAEXXZ)(NetDaemon* this)
// callers   10   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00592EE0 0x006262F0
// Homed 2026-09-03 out of the recovered-units archive: faction.cpp's
// diplo_unlock calls it, and a pending_bodies forwarder would have landmined
// this address's archive claim.
// RESIDUAL: the annotation's span is SHORT, and it is the only reason this is
//        not BYTE_EXACT. 0x00530D00-0x00530D4E (54 bytes) stops after the
//        third field store. The shipped bytes run
//          89 be b0 1b 00 00 / 89 be ac 1b 00 00 / 89 be cc 1b 00 00
//          89 be c8 1b 00 00 / 5f 5e c3
//        from 0x00530D35, so the function really ends at 0x00530D3C - 28
//        instructions, which is exactly what this tree compiles, and all 21
//        the annotation carries agree. Demoted for the span, not the body.
Status: Complete
*/
void NetDaemon::unlock() {
    // The four announce-side fields sit inside lock_state_[2] - 0x1BAC, 0x1BB0,
    // 0x1BC8 and 0x1BCC are that entry's dword_[2], dword_[3], dword_[9] and
    // dword_[10]. Named through the declared member rather than a raw offset.
    if (NetDaemonIsMultiplayerNet != 0) {
        log_say("Client releasing lock", NetDaemonLocalFaction, 0, 0);
        message_data(0x2212, 0, 0, 0, 0, 0);
        lock_state_[2].dword_[3] = 0;
        lock_state_[2].dword_[2] = 0;
        lock_state_[2].dword_[10] = 0;
        lock_state_[2].dword_[9] = 0;
    }
}

/*
Purpose: Tell the other clients this client is leaving the diplomacy channel.
// ORIGINAL: 0x00530600 ?close_channel@NetDaemon@@QAEXXZ 0x00530600-0x0053062F BYTE_EXACT
// size      47 bytes
// prototype void (__thiscall ?close_channel@NetDaemon@@QAEXXZ)(NetDaemon* this)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00592EE0
// Homed 2026-09-03 out of the recovered archive for the same reason.
Status: Complete
*/
void NetDaemon::close_channel() {
    if (NetDaemonIsMultiplayerNet != 0) {
        message_data(0x8241, 0, 0, 0, 0, 0);
        // 0x1BA0 is lock_state_[1].dword_[14].
        lock_state_[1].dword_[14] = 0;
    }
}


// ORIGINAL: 0x0052E050 ?hang_up@NetDaemon@@QAEXXZ 0x0052E050-0x0052E151 FILE
// size      257 bytes
// prototype void (__thiscall ?hang_up@NetDaemon@@QAEXXZ)(NetDaemon* this)
// callers   9   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0047A7D0 0x0058FF70 0x00592EE0
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0052E050
// measured tier  MISMATCH
// divergence     16
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0052e050/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?hang_up@NetDaemon@@QAEXXZ  at 0x0052E050  (257 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    an indirect virtual call use the generated VCall shim below; for a
//    thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.


// 0x1B3C is lock_state_[0].dword_[4], 0x1580/0x1584 are the Lock's tail
// dwords, 0x1B7C is lock_state_[1].dword_[5] and the four dwords at 0x1B80
// are lock_state_[1].dword_[6..9] - one per faction.
void NetDaemon::hang_up() {
    if (lock_state_[0].dword_[4] != 0) {
        return;
    }
    if (NetDaemonIsMultiplayerNet == 0) {
        return;
    }
    lock_state_[0].dword_[4] = 1;
    if (NetTurnFlags & 0x10) {
        if (ConsoleControlTurnActive == 0) {
            if (NetTurnFaction != NetDaemonLocalFaction) {
                goto L0cc;
            }
        } else if (NetDaemonNet.lock_state_[0].dword_[1] == 0) {
            goto L145;
        }
        message_data(0x8301, 0, 0, 0, 0, 0);
    }
L0cc:
    if (NetDaemonNet.lock_state_[0].dword_[1] != 0) {
        locks_.records_[NetDaemonLocalFaction].clear();
        if (locks_.field_E0_ == NetDaemonLocalFaction) {
            locks_.field_E4_ = 0;
            locks_.field_E0_ = 0;
        }
        lock_state_[1].dword_[5] &=
            ~lock_state_[1].dword_[6 + NetDaemonLocalFaction];
        message_data(0x4b00, 0, 0, 0, 0, 0);
    }
L145:
    NetMsgPopup->close();
}


// ORIGINAL: 0x0052E900 ?send_game@NetDaemon@@QAEXXZ 0x0052E900-0x0052ED33 FILE
// size      1075 bytes
// prototype void (__thiscall ?send_game@NetDaemon@@QAEXXZ)(NetDaemon* this)
// callers   1   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00532E00 0x00534400 0x00630A00
// PRESERVED UNIT - measured NO_COMPILE.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0052E900
// measured tier  NO_COMPILE
// refusal        u0052e900.cpp(73) : error C2664: 'synch' : cannot convert parameter 1 from 'const int' to 'struct int16' Source or target has incomplete type u0052e900.cpp(75) 
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0052e900/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?send_game@NetDaemon@@QAEXXZ  at 0x0052E900  (1075 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    an indirect virtual call use the generated VCall shim below; for a
//    thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?send_game@NetDaemon@@QAEXXZ  at 0x0052E900  (1075 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// The VC6 dialect limits and the source-form rules used to live here.
// They are knowledge, not scaffolding, so they now live in the agent
// system prompt (mizuchi.yaml, plugins.claude-runner.systemPrompt),
// where they can be edited without regenerating anything and are in
// context from the first token rather than behind a file read. This
// emitter computes declarations; it does not carry lessons.


void NetDaemon::send_game() {
    unsigned long a, b;
    unsigned long c, d;
    unsigned long e, f;
    unsigned long id;

    synch(0, 0, 0, 0, 0, 1, 0x2101);
    for (;;) {
        id = NetDaemonNet.Net::get(&a, &b);
        if (id == 0) break;
        process_message((char *)id, b, a);
    }

    synch(2, 0, 0, 0, 0, 1, 0x2101);
    for (;;) {
        id = NetDaemonNet.Net::get(&a, &b);
        if (id == 0) break;
        process_message((char *)id, b, a);
    }

    synch(4, 0, 0, 0, 0, 1, 0x2101);
    for (;;) {
        id = NetDaemonNet.Net::get(&a, &b);
        if (id == 0) break;
        process_message((char *)id, b, a);
    }

    synch(5, 0, 0, 0, 0, 1, 0x2101);
    for (;;) {
        id = NetDaemonNet.Net::get(&a, &b);
        if (id == 0) break;
        process_message((char *)id, b, a);
    }

    int i = 0;
    int off = 0;
    do {
        synch(6, i, 0, 0, 0, 1, 0x2101);
        for (;;) {
            id = NetDaemonNet.Net::get(&c, &d);
            if (id == 0) break;
            process_message((char *)id, d, c);
        }

        synch(7, i, 0, 0, 0, 1, 0x2101);
        for (;;) {
            id = NetDaemonNet.Net::get(&e, &f);
            if (id == 0) break;
            process_message((char *)id, f, e);
        }

        synch(0xf, off, 0x40, 0, 0, 1, 0x2101);
        for (;;) {
            id = NetDaemonNet.Net::get(&e, &f);
            if (id == 0) break;
            process_message((char *)id, f, e);
        }

        off += 0x40;
        i++;
    } while (off < 0x200);

    synch(0x29, 0, 0, 0, 0, 1, 0x2101);
    for (;;) {
        id = NetDaemonNet.Net::get(&e, &f);
        if (id == 0) break;
        process_message((char *)id, f, e);
    }

    synch(0x12, 0, 0, 0, 0, 1, 0x2101);
    for (;;) {
        id = NetDaemonNet.Net::get(&e, &f);
        if (id == 0) break;
        process_message((char *)id, f, e);
    }

    synch(0x14, 0, 0, 0, 0, 1, 0x2101);
    for (;;) {
        id = NetDaemonNet.Net::get(&e, &f);
        if (id == 0) break;
        process_message((char *)id, f, e);
    }

    synch(0x19, 0, 0, 0, 0, 1, 0x2101);
    for (;;) {
        id = NetDaemonNet.Net::get(&e, &f);
        if (id == 0) break;
        process_message((char *)id, f, e);
    }

    if (TurnCurrentNum != 0) {
        synch(0x20, 0, 0, 0, 0, 1, 0x2101);
        for (;;) {
            id = NetDaemonNet.Net::get(&e, &f);
            if (id == 0) break;
            process_message((char *)id, f, e);
        }
    }

    synch(0x21, 0, 0, 0, 0, 1, 0x2101);
    for (;;) {
        id = NetDaemonNet.Net::get(&e, &f);
        if (id == 0) break;
        process_message((char *)id, f, e);
    }

    if (ReplayEventCount != 0) {
        synch(0x22, 0, 0, 0, 0, 1, 0x2101);
        for (;;) {
            id = NetDaemonNet.Net::get(&e, &f);
            if (id == 0) break;
            process_message((char *)id, f, e);
        }
    }

    synch(0x24, 0, 0, 0, 0, 1, 0x2101);
    for (;;) {
        id = NetDaemonNet.Net::get(&e, &f);
        if (id == 0) break;
        process_message((char *)id, f, e);
    }

    synch(0x15, 0, 0, 0, 0, 1, 0x2101);
    for (;;) {
        id = NetDaemonNet.Net::get(&e, &f);
        if (id == 0) break;
        process_message((char *)id, f, e);
    }
}

// ORIGINAL: 0x0052F520 ?send_rules@NetDaemon@@QAEXXZ 0x0052F520-0x0052F8D6 FILE
// size      950 bytes
// prototype void (__thiscall ?send_rules@NetDaemon@@QAEXXZ)(NetDaemon* this)
// callers   1   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00532E00 0x00534400 0x00630A00
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0052F520
// measured tier  MISMATCH
// divergence     28
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0052f520/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?send_rules@NetDaemon@@QAEXXZ  at 0x0052F520  (950 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// The VC6 dialect limits and the source-form rules used to live here.
// They are knowledge, not scaffolding, so they now live in the agent
// system prompt (mizuchi.yaml, plugins.claude-runner.systemPrompt),
// where they can be edited without regenerating anything and are in
// context from the first token rather than behind a file read. This
// emitter computes declarations; it does not carry lessons.

void NetDaemon::send_rules() {
    unsigned long v4, v8;

    synch(0x80, 0, 0, 0, 0, 1, 0x2101);
    while (int result = NetDaemonNet.Net::get(&v4, &v8)) {
        process_message(reinterpret_cast<char *>(result), v8, v4);
    }

    synch(0x81, 0, 0, 0, 0, 1, 0x2101);
    while (int result = NetDaemonNet.Net::get(&v4, &v8)) {
        process_message(reinterpret_cast<char *>(result), v8, v4);
    }

    synch(0x82, 0, 0, 0, 0, 1, 0x2101);
    while (int result = NetDaemonNet.Net::get(&v4, &v8)) {
        process_message(reinterpret_cast<char *>(result), v8, v4);
    }

    synch(0x83, 0, 0, 0, 0, 1, 0x2101);
    while (int result = NetDaemonNet.Net::get(&v4, &v8)) {
        process_message(reinterpret_cast<char *>(result), v8, v4);
    }

    synch(0x84, 0, 0, 0, 0, 1, 0x2101);
    while (int result = NetDaemonNet.Net::get(&v4, &v8)) {
        process_message(reinterpret_cast<char *>(result), v8, v4);
    }

    synch(0x85, 0, 0, 0, 0, 1, 0x2101);
    while (int result = NetDaemonNet.Net::get(&v4, &v8)) {
        process_message(reinterpret_cast<char *>(result), v8, v4);
    }

    synch(0x86, 0, 0, 0, 0, 1, 0x2101);
    while (int result = NetDaemonNet.Net::get(&v4, &v8)) {
        process_message(reinterpret_cast<char *>(result), v8, v4);
    }

    synch(0x87, 0, 0, 0, 0, 1, 0x2101);
    while (int result = NetDaemonNet.Net::get(&v4, &v8)) {
        process_message(reinterpret_cast<char *>(result), v8, v4);
    }

    synch(0x88, 0, 0, 0, 0, 1, 0x2101);
    while (int result = NetDaemonNet.Net::get(&v4, &v8)) {
        process_message(reinterpret_cast<char *>(result), v8, v4);
    }

    synch(0x89, 0, 0, 0, 0, 1, 0x2101);
    while (int result = NetDaemonNet.Net::get(&v4, &v8)) {
        process_message(reinterpret_cast<char *>(result), v8, v4);
    }

    synch(0x8a, 0, 0, 0, 0, 1, 0x2101);
    while (int result = NetDaemonNet.Net::get(&v4, &v8)) {
        process_message(reinterpret_cast<char *>(result), v8, v4);
    }

    synch(0x8b, 0, 0, 0, 0, 1, 0x2101);
    while (int result = NetDaemonNet.Net::get(&v4, &v8)) {
        process_message(reinterpret_cast<char *>(result), v8, v4);
    }

    synch(0x8c, 0, 0, 0, 0, 1, 0x2101);
    while (int result = NetDaemonNet.Net::get(&v4, &v8)) {
        process_message(reinterpret_cast<char *>(result), v8, v4);
    }

    synch(0x8d, 0, 0, 0, 0, 1, 0x2101);
    while (int result = NetDaemonNet.Net::get(&v4, &v8)) {
        process_message(reinterpret_cast<char *>(result), v8, v4);
    }
}


// ORIGINAL: 0x0052F8E0 ?game_checksum@NetDaemon@@QAEXPAH@Z 0x0052F8E0-0x0053031E FILE
// TRIED: full instruction-order transcription of all ~150 checksum() calls (verified offsets/strides/seed-chaining against raw disasm, not Ghidra scaling); diverges at insn #53, likely register-alloc (lea vs mov) in the bit_count loop or header prologue, not logic.
// working copy - scaffold materialised by --work
// size      2622 bytes
// prototype void (__thiscall ?game_checksum@NetDaemon@@QAEXPAH@Z)(NetDaemon* this, int*)
// callers   1   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0050BA30 0x00539090

void NetDaemon::game_checksum(int *results) {
    unsigned char cs;
    int *out = results;

    cs = checksum((char *)&GameRules, 4, 0);
    cs = checksum((char *)&VehCurrentCount, 0x10, cs);
    cs = checksum((char *)&LockEnableMask, 2, cs);
    cs = checksum((char *)&SecretProject, 0x100, cs);

    {
        unsigned char *bitTable = (unsigned char *)GameTechAchieved;
        int bi;
        for (bi = 0; bi < 0x59; ++bi) {
            int bc = bit_count((int)bitTable[bi]);
            unsigned char mult = (unsigned char)(bi + 1);
            unsigned char al = (unsigned char)((unsigned char)bc * mult);
            cs = (unsigned char)(cs ^ al);
        }
    }
    *out++ = cs;
    cs = 0;

    {
        char *p;
        for (p = (char *)((char *)PlayersData + 0x2498); p < (char *)((char *)Bases + 0x3CC); p += 0x20cc) {
            unsigned int masked1[8];
            unsigned int masked2[8];
            int idx4;
            cs = checksum(p - 4, 4, cs);
            cs = checksum(p, 4, cs);
            cs = checksum(p + 0x98, 4, cs);
            cs = checksum(p - 0x1ac, 4, cs);
            cs = checksum(p - 0x1a4, 0x10, cs);
            cs = checksum(p + 0x43c, 0x10, cs);
            for (idx4 = 0; idx4 < 8; ++idx4) {
                masked1[idx4] = *(unsigned int *)(p - 0x3b4 + idx4 * 4) & 0xb1d3c7f;
                masked2[idx4] = *(unsigned int *)(p - 0x394 + idx4 * 4) & 0x1e68;
            }
            cs = checksum((char *)masked1, 0x20, cs);
            cs = checksum((char *)masked2, 0x20, cs);
            cs = checksum(p - 0x2a8, 4, cs);
        }
    }
    *out++ = cs;
    cs = 0;

    {
        int countA = MapArea;
        if (countA > 0) {
            char *recBase = (char *)LockMapTable;
            char *p2 = recBase + 8;
            int idxA = 0;
            do {
                cs = checksum(recBase, 1, cs);
                cs = checksum(p2 - 5, 1, cs);
                cs = checksum(p2, 4, cs);
                cs = checksum(p2 - 7, 1, cs);
                cs = checksum(p2 - 1, 1, cs);
                cs = checksum(p2 + 4, 1, cs);
                countA = MapArea;
                recBase += 0x2c;
                p2 += 0x2c;
                idxA++;
            } while (idxA < countA);
        }
    }
    *out++ = cs;
    cs = 0;

    {
        int idx2;
        for (idx2 = 0; idx2 < 8; ++idx2) {
            int countB = *(char *)&VehCurrentCount;
            if (countB > 0) {
                char *p3 = (char *)((char *)Vehs + 2);
                int idxB = 0;
                do {
                    if ((unsigned char)p3[0xc] == idx2) {
                        cs = checksum(p3 - 2, 2, cs);
                        cs = checksum(p3, 2, cs);
                        cs = checksum(p3 + 8, 2, cs);
                        cs = checksum(p3 + 0xc, 1, cs);
                        cs = checksum(p3 + 0xe, 1, cs);
                        cs = checksum(p3 + 0x22, 1, cs);
                        cs = checksum(p3 + 0x26, 1, cs);
                        cs = checksum(p3 + 0x2c, 2, cs);
                        countB = *(char *)&VehCurrentCount;
                    }
                    p3 += 0x34;
                    idxB++;
                } while (idxB < countB);
            }
        }
    }
    *out++ = cs;
    cs = 0;

    {
        int countC = BaseCurrentCount;
        if (countC > 0) {
            char *p4 = (char *)((char *)Bases + 0x3C);
            int idxC = 0;
            do {
                cs = checksum(p4 - 0x3c, 7, cs);
                cs = checksum(p4, 4, cs);
                cs = checksum(p4 + 4, 4, cs);
                cs = checksum(p4 + 0x10, 4, cs);
                cs = checksum(p4 + 0x14, 4, cs);
                cs = checksum(p4 + 0x3c, 4, cs);
                cs = checksum(p4 + 0x48, 8, cs);
                cs = checksum(p4 + 0x50, 9, cs);
                idxC++;
                p4 += 0x134;
            } while (idxC < countC);
        }
    }
    *out++ = cs;
    cs = 0;

    {
        int rawVal = MapLatitudeBounds;
        int div = (rawVal + ((rawVal >> 31) & 0xf)) >> 4;
        int remainder = rawVal - (div << 4);
        int k;
        for (k = 0; k < 0x10; ++k) {
            int recIdx = k * div;
            if (k == 0xf && remainder != 0) {
                div = div + remainder;
            }
            cs = 0;
            {
                int innerCount = (MapLongitudeBounds >> 1) * div;
                if (div != 0 && innerCount > 0) {
                    char *p5 = (char *)(LockMapTable + (*MapRowStride) * recIdx * 0x2c);
                    char *p6 = p5 + 8;
                    int idxE;
                    for (idxE = 0; idxE < innerCount; ++idxE) {
                        cs = checksum(p5, 1, cs);
                        cs = checksum(p6 - 5, 1, cs);
                        cs = checksum(p6, 4, cs);
                        cs = checksum(p6 - 7, 1, cs);
                        cs = checksum(p6 - 1, 1, cs);
                        p5 += 0x2c;
                        p6 += 0x2c;
                    }
                }
            }
            *out++ = cs;
        }
    }

    if (BaseAtDebugPopupFlag != 0) {
        cs = checksum((char *)&GameRules, 4, 0);
        cs = checksum((char *)&VehCurrentCount, 0x10, cs);
        *out++ = checksum((char *)&LockEnableMask, 2, cs);
        *out++ = checksum((char *)&SecretProject, 0x100, 0);
        cs = 0;

        {
            unsigned char *bitTable = (unsigned char *)GameTechAchieved;
            int bi2;
            for (bi2 = 0; bi2 < 0x59; ++bi2) {
                int bc = bit_count((int)bitTable[bi2]);
                unsigned char mult = (unsigned char)(bi2 + 1);
                unsigned char al = (unsigned char)((unsigned char)bc * mult);
                cs = (unsigned char)(cs ^ al);
            }
        }
        *out++ = cs;

        {
            char *p;
            for (p = (char *)((char *)PlayersData + 0x2498); p < (char *)((char *)Bases + 0x3CC); p += 0x20cc) {
                cs = checksum(p - 4, 4, 0);
                cs = checksum(p, 4, cs);
                cs = checksum(p + 0x98, 4, cs);
                cs = checksum(p - 0x1ac, 4, cs);
                cs = checksum(p - 0x1a4, 0x10, cs);
                cs = checksum(p + 0x43c, 0x10, cs);
                cs = checksum(p - 0x3b4, 0x20, cs);
                cs = checksum(p - 0x394, 0x20, cs);
                cs = checksum(p - 0x374, 0x20, cs);
                cs = checksum(p - 0x314, 0x20, cs);
                cs = checksum(p - 0x334, 0x20, cs);
                cs = checksum(p - 0x354, 0x20, cs);
                cs = checksum(p - 0x280, 0x20, cs);
                cs = checksum(p - 0x2a8, 4, cs);
                *out++ = cs;
            }
        }
        cs = 0;

        {
            int cnt1 = MapArea;
            char *pL = (char *)LockMapTable;
            int i1;
            for (i1 = 0; i1 < cnt1; ++i1) {
                cs = checksum(pL, 1, cs);
                cnt1 = MapArea;
                pL += 0x2c;
            }
        }
        *out++ = cs;
        cs = 0;

        {
            int cnt2 = MapArea;
            char *pL = (char *)LockMapTable + 3;
            int i2;
            for (i2 = 0; i2 < cnt2; ++i2) {
                cs = checksum(pL, 1, cs);
                cnt2 = MapArea;
                pL += 0x2c;
            }
        }
        *out++ = cs;
        cs = 0;

        {
            int cnt3 = MapArea;
            char *pL = (char *)LockMapTable + 8;
            int i3;
            for (i3 = 0; i3 < cnt3; ++i3) {
                cs = checksum(pL, 4, cs);
                cnt3 = MapArea;
                pL += 0x2c;
            }
        }
        *out++ = cs;
        cs = 0;

        {
            int cnt4 = MapArea;
            char *pL = (char *)LockMapTable + 1;
            int i4;
            for (i4 = 0; i4 < cnt4; ++i4) {
                cs = checksum(pL, 1, cs);
                cnt4 = MapArea;
                pL += 0x2c;
            }
        }
        *out++ = cs;
        cs = 0;

        {
            int cnt5 = MapArea;
            char *pL = (char *)LockMapTable + 7;
            int i5;
            for (i5 = 0; i5 < cnt5; ++i5) {
                cs = checksum(pL, 1, cs);
                cnt5 = MapArea;
                pL += 0x2c;
            }
        }
        *out++ = cs;
        cs = 0;

        {
            int cnt6 = MapArea;
            char *pL = (char *)LockMapTable + 0xc;
            int i6;
            for (i6 = 0; i6 < cnt6; ++i6) {
                cs = checksum(pL, 1, cs);
                cnt6 = MapArea;
                pL += 0x2c;
            }
        }
        *out++ = cs;

        *out++ = checksum((char *)&MapSizePlanet, 0x1c, 0);

        cs = 0;
        {
            int idx2b;
            for (idx2b = 0; idx2b < 8; ++idx2b) {
                int countB2 = *(char *)&VehCurrentCount;
                cs = 0;
                if (countB2 > 0) {
                    char *p3b = (char *)Vehs;
                    int idxBb = 0;
                    do {
                        if ((unsigned char)p3b[0xe] == idx2b) {
                            cs = checksum(p3b, 2, cs);
                            cs = checksum(p3b + 2, 2, cs);
                            cs = checksum(p3b + 0xa, 3, cs);
                            cs = checksum(p3b + 0x24, 1, cs);
                            cs = checksum(p3b + 0x28, 1, cs);
                            cs = checksum(p3b + 0x2e, 2, cs);
                            countB2 = *(char *)&VehCurrentCount;
                        }
                        p3b += 0x34;
                        idxBb++;
                    } while (idxBb < countB2);
                }
            }
        }
        *out++ = cs;

        cs = 0;
        {
            int countC2 = BaseCurrentCount;
            if (countC2 > 0) {
                char *p4b = (char *)((char *)Bases + 0x3C);
                int idxC2 = 0;
                do {
                    cs = checksum(p4b - 0x3c, 7, cs);
                    cs = checksum(p4b, 4, cs);
                    cs = checksum(p4b + 4, 4, cs);
                    cs = checksum(p4b + 0x10, 4, cs);
                    cs = checksum(p4b + 0x14, 4, cs);
                    cs = checksum(p4b + 0x3c, 4, cs);
                    cs = checksum(p4b + 0x48, 8, cs);
                    cs = checksum(p4b + 0x50, 9, cs);
                    idxC2++;
                    p4b += 0x134;
                } while (idxC2 < countC2);
            }
        }
        *out++ = cs;
    }
}


// ORIGINAL: 0x005328C0 ?end_of_event@NetDaemon@@QAEHHH@Z 0x005328C0-0x00532934 FILE
// working copy - scaffold materialised by --work
// size      116 bytes
// prototype int (__thiscall ?end_of_event@NetDaemon@@QAEHHH@Z)(NetDaemon* this, int, int)
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00592EE0 0x006262F0

// 0x1B2C/0x1B30 are lock_state_[0].dword_[0]/[1]; 0x1BB0 and 0x1BC0 are
// lock_state_[2].dword_[3] and [7].
int NetDaemon::end_of_event(int faction_id, int turn_value) {
    if (lock_state_[0].dword_[1] == 0) {
        lock_state_[0].dword_[0] = turn_value;
    }
    if (faction_id == NetDaemonLocalFaction) {
        if (lock_state_[2].dword_[7] != 0) {
            log_say("Remaining Locked2", 0, 0, 0);
            lock_state_[2].dword_[7] = 0;
            message_data(0x2214, 0, 0, 0, 0, 0);
            return 0;
        }
        lock_state_[2].dword_[3] = 0;
    }
    return 0;
}


// ---------------------------------------------------------------------------
// The message builders. Every sender in the family packs its record on the
// stack, stamps the three leading fields the transport owns - the opcode
// (short), the local faction and the wall-clock stamp - then hands the record
// to NetDaemon::send_message with NetBusyCount held for the duration. Only
// the payload fields differ.
//
// The turn stamp is 0x0093E8BC, which is the daemon's own
// lock_state_[0].dword_[0]: net_end_of_turn (0x00529F30) ends with
// `++*0x0093E8BC`, once per turn, so the field counts turns for the whole
// session. The stamp comes from net_class.h's NetTimeGetTimeSlot(), the same
// 0x00669368 import slot the image calls through as
// `call dword ptr [0x669368]`.
//
// Payload parameters are named for the record offset they land at, which is
// the one thing about them the bodies prove.
// ---------------------------------------------------------------------------

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
    struct Message {
        short opcode_;
        int faction_;
        unsigned long time_;
        int turn_;
        int field_10_;
        int field_14_;
    } message;

    message.opcode_ = static_cast<short>(opcode);
    message.faction_ = NetDaemonLocalFaction;
    message.time_ = NetTimeGetTimeSlot()();
    message.turn_ = NetDaemonNet.lock_state_[0].dword_[0];
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
    struct Message {
        short opcode_;
        int faction_;
        unsigned long time_;
        int turn_;
        int payload_10_;
        char text_[24];
    } message;

    message.opcode_ = static_cast<short>(opcode);
    message.faction_ = NetDaemonLocalFaction;
    message.time_ = NetTimeGetTimeSlot()();
    message.turn_ = NetDaemonNet.lock_state_[0].dword_[0];
    message.payload_10_ = payload_10;
    strcpy(message.text_, text);

    NetBusyCount++;
    NetDaemonNet.send_message(reinterpret_cast<char *>(&message), 0x2c,
                              target);
    NetBusyCount--;
}

// 0x00592EE0 ?message_data@@YAXHHHHHH@Z is the one member of this family
// that stays a pending_bodies forwarder, and the reason is its return value.
// The catalogue spells it void, but EAX is live at its ret - it is whatever
// Net::send left there, two calls down - and unlock_veh (0x005310F0,
// BYTE_EXACT) returns exactly that residue. A body spelled here would have to
// name a value no source expression reaches: the last EAX-writing event is
// inside send_message, after which the image only decrements NetBusyCount.
// TRIED: a uint32_t body with no return statement - VC6 makes that C4716, a
// hard error. TRIED: returning NetBusyCount - adds a load the image has not
// got. The forwarder is the only spelling that passes the callee's EAX
// through, so it stays.
// ORIGINAL: 0x00592F50 sub_592f50 0x00592F50-0x00592FE9
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
    unsigned char message[0x90];

    *reinterpret_cast<short *>(message + 0x00) = static_cast<short>(opcode);
    *reinterpret_cast<int *>(message + 0x04) = NetDaemonLocalFaction;
    *reinterpret_cast<unsigned int *>(message + 0x08) = NetTimeGetTimeSlot()();
    *reinterpret_cast<int *>(message + 0x0c) =
        NetDaemonNet.lock_state_[0].dword_[0];
    *reinterpret_cast<int *>(message + 0x10) = payload_10;
    *reinterpret_cast<int *>(message + 0x14) = payload_14;
    *reinterpret_cast<int *>(message + 0x18) = payload_18;
    *reinterpret_cast<int *>(message + 0x1c) = payload_1C;
    *reinterpret_cast<int *>(message + 0x20) = payload_20;
    *reinterpret_cast<int *>(message + 0x24) = payload_24;
    *reinterpret_cast<int *>(message + 0x28) = payload_28;
    *reinterpret_cast<int *>(message + 0x2c) = payload_2C;

    NetBusyCount++;
    NetDaemonNet.send_message(reinterpret_cast<char *>(message), 0x90, target);
    NetBusyCount--;
}

// ORIGINAL: 0x00592FF0 ?message_big_data@@YAXHHPAHH@Z 0x00592FF0-0x00593089
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
    struct Message {
        short opcode_;
        int faction_;
        unsigned long time_;
        int turn_;
        int data_[32];
    } message;
    int filled;
    int i;
    int *dst;

    message.opcode_ = static_cast<short>(opcode);
    message.faction_ = NetDaemonLocalFaction;
    message.time_ = NetTimeGetTimeSlot()();
    message.turn_ = NetDaemonNet.lock_state_[0].dword_[0];

    filled = 0;
    if (count > 0) {
        filled = count;
        dst = message.data_;
        for (i = 0; i < count; i++) {
            dst[i] = payload[i];
        }
    }
    if (filled < 0x20) {
        dst = message.data_ + filled;
        // No named `fill_count` local: the image computes `0x20 - filled`
        // straight into the rep count (`mov ecx,0x20 / sub ecx,eax`), and a
        // local made VC6 test it separately - two instructions the image does
        // not have.
        for (i = 0; i < 0x20 - filled; i++) {
            dst[i] = 0;
        }
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
    struct Message {
        short opcode_;
        int faction_;
        unsigned long time_;
        int turn_;
        int field_10_;
        int field_14_;
        int field_18_;
        int field_1C_;
        int field_20_;
        int field_24_;
    } message;

    message.opcode_ = static_cast<short>(opcode);
    message.faction_ = NetDaemonLocalFaction;
    message.time_ = NetTimeGetTimeSlot()();
    message.turn_ = NetDaemonNet.lock_state_[0].dword_[0];
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

// ORIGINAL: 0x00593110 ?message_landmark@@YAXHHPADHHHH@Z 0x00593110-0x00593190
// size      128 bytes
// prototype void (__cdecl ?message_landmark@@YAXHHPADHHHH@Z)(int, int, char *, int, int, int, int)
// callers   1   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00532940 0x00645460
// The image stores the payload dwords out of order - 0x1C first, then the
// stamp block, then 0x14 and 0x18 - and that order is kept.
void __cdecl message_landmark(int opcode, int target, char *text,
                              int payload_10, int payload_14, int payload_18,
                              int payload_1C) {
    char message[0x40];

    *reinterpret_cast<short *>(message) = static_cast<short>(opcode);
    *reinterpret_cast<int *>(message + 0x04) = NetDaemonLocalFaction;
    *reinterpret_cast<unsigned long *>(message + 0x08) = NetTimeGetTimeSlot()();
    *reinterpret_cast<int *>(message + 0x1c) = payload_1C;
    *reinterpret_cast<int *>(message + 0x0c) =
        NetDaemonNet.lock_state_[0].dword_[0];
    *reinterpret_cast<int *>(message + 0x14) = payload_14;
    *reinterpret_cast<int *>(message + 0x18) = payload_18;
    *reinterpret_cast<int *>(message + 0x10) = payload_10;
    if (text) {
        strcpy(message + 0x20, text);
    }

    NetBusyCount++;
    NetDaemonNet.send_message(message, 0x40, target);
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
    message.faction_ = NetDaemonLocalFaction;
    message.time_ = NetTimeGetTimeSlot()();
    message.turn_ = NetDaemonNet.lock_state_[0].dword_[0];
    strncpy(message.text_, text, 0x100);

    NetBusyCount++;
    message.text_[0xff] = 0;
    NetDaemonNet.send_message(reinterpret_cast<char *>(&message), 0x110,
                              target);
    NetBusyCount--;
}

// Fixed-slot bindings carried from 00532940.cpp
typedef unsigned long (__stdcall *TimeGetTimeFn)(void);

// ORIGINAL: 0x00532940 ?send_message@NetDaemon@@QAEXPADKH@Z 0x00532940-0x00532A44 FILE BYTE_EXACT
// size      260 bytes
// prototype void (__thiscall ?send_message@NetDaemon@@QAEXPADKH@Z)(NetDaemon* this, int8*, unsigned int, int)
// callers   12   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004E2660 0x00630080
// indirect  0x0053295F
// PRESERVED UNIT - measured NO_COMPILE.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00532940
// measured tier  NO_COMPILE
// refusal        u00532940.cpp(46) : error C2653: 'NetDaemon' : is not a class or namespace name u00532940.cpp(47) : error C2065: 'NetDaemonIsMultiplayerNet' : undeclared identifier u00532940.
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00532940/unit.cpp
// and score it with tools/agent_brief.py.
// 0x00532940  ?send_message@NetDaemon@@QAEXPADKH@Z  (260 bytes)
//
// A four-way dispatch on bits of `*(short*)message`: 0x4000 (timestamp+send),
// 0x8000 (send with a possibly-substituted value), 0xa000 (an extra
// who_2_pid path, falling back to the same substitution as 0x8000), and a
// default (send with `target` or `who_2_pid(target)`). `Net::send` and
// `AlphaNet::who_2_pid` are both called through `reinterpret_cast<Net*>` /
// `reinterpret_cast<AlphaNet*>(this)`: NetDaemon is opaque and neither base
// is declared as a real base class in this unit.
//
// `timeGetTime` is called through its IAT slot (g_00669368), same pattern
// as UNK2's Win32 calls (0x00609bf0) - a `__stdcall` function-pointer
// typedef dereferenced off the fixed-address global, not a named extern.
//
// THREE occurrences of "value = (global==1) ? target : fallback" (two computing
// `this->field_764` for the same 0x764 offset, one computing
// `who_2_pid(target)`) each needed the SAME two source-form fixes to reproduce
// their original instruction order, none of them obvious from a plain
// if/else:
//
//   1. `if (cond) { return_early_no_assignment; }` (no else) let the
//      compiler duplicate the ENTIRE trailing `send(...)` call and epilogue
//      into both arms instead of merging on one shared call site - a
//      single-branch if with no else, immediately followed by the
//      unconditional `send`, is NOT enough; it needs an explicit
//      `if (..) { value = A; } else { value = B; }` with a dedicated local,
//      merging back to ONE assignment before the shared call.
//   2. Even with that, a REUSED PARAMETER (`if (x!=1) target = fallback;`)
//      compiled the load-and-compare in the opposite order from a fresh
//      local (`int value; if (x==1) value=target; else value=fallback; target=value;`)
//      - original loads the flag/condition FIRST, then the "keep target" arm,
//      matching the fresh-local form, not the reused-parameter form.
//   3. A trivial `target = target;` self-assignment (tried for the third,
//      who_2_pid, occurrence, to force the same shape without a spare
//      local) gets optimized away entirely, losing the explicit
//      reload+jmp the original has - only the real `int value; if (){ }
//      else { }` form survives with the merge point intact.
//
// The 0xa000 branch also reads `NetDaemonNet.lock_state_[0].dword_[5]` ONCE into a local (`counter`)
// BEFORE its four-way `&&` guard, and the branch's own fallback re-tests
// that SAME local rather than re-reading the global - the original never
// reloads `[0x93e8d0]` a second time inside this branch (one `mov`, two
// `cmp`s against the same register).
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?send_message@NetDaemon@@QAEXPADKH@Z  at 0x00532940  (260 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// The VC6 dialect limits and the source-form rules used to live here.
// They are knowledge, not scaffolding, so they now live in the agent
// system prompt (mizuchi.yaml, plugins.claude-runner.systemPrompt),
// where they can be edited without regenerating anything and are in
// context from the first token rather than behind a file read. This
// emitter computes declarations; it does not carry lessons.


// A four-way dispatch on bits of the record's leading word: 0x4000 restamps
// the time and sends, 0x8000 sends with a possibly-substituted player id,
// 0xA000 adds a who_2_pid path that falls back to the same substitution, and
// the default substitutes who_2_pid for any target but the local one. The
// substitution's fallback is field_764_, the two words check_polling compares
// (see net_class.h); the turn counter it reads once into `counter` is
// lock_state_[0].dword_[5].
//
// Each of the three "value = (flag == 1) ? target : fallback" shapes needed
// a dedicated local and an if/else pair - a single-branch if duplicated the
// trailing send into both arms, a reused parameter compiled the load and
// compare in the opposite order, and `target = target;` was folded away
// entirely. Measured, and the three sites are spelled the same way for it.
void NetDaemon::send_message(char *message, unsigned long length, int target) {
    if (NetDaemonIsMultiplayerNet) {
        unsigned short flags = *reinterpret_cast<unsigned short *>(message);
        if (flags & 0x4000) {
            *reinterpret_cast<unsigned long *>(message + 8) =
                NetTimeGetTimeSlot()();
            Net::send(message, length, 0, 1);
            return;
        }
        if (flags & 0x8000) {
            int value;
            if (NetDaemonNet.lock_state_[0].dword_[5] == 1) {
                value = target;
            } else {
                value = field_764_;
            }
            target = value;
            Net::send(message, length, target, 1);
            return;
        }
        if (flags & 0xa000) {
            int counter = NetDaemonNet.lock_state_[0].dword_[5];
            if ((NetTurnFlags & 0x10) &&
                ConsoleControlTurnActive == 0 && NetTurnFaction > 0 && counter > 1) {
                Net::send(message, length,
                          AlphaNet::who_2_pid(NetTurnFaction), 1);
                return;
            }
            int value;
            if (counter == 1) {
                value = target;
            } else {
                value = field_764_;
            }
            target = value;
            Net::send(message, length, target, 1);
            return;
        }
        int value;
        if (NetDaemonNet.lock_state_[0].dword_[5] == 1) {
            value = target;
        } else {
            value = AlphaNet::who_2_pid(target);
        }
        target = value;
        Net::send(message, length, target, 1);
    }
}


// ---------------------------------------------------------------------------
// THE VEHICLE LOCK. 0x00531020 takes the vehicle by POINTER - the pointer is
// what the daemon stores at lock_state_[1].dword_[4] (0x1B78) to remember
// which unit this client holds - and everything else arrives as ints that go
// straight through to NetDaemon::lock at the record offsets named below.
// ---------------------------------------------------------------------------

// ORIGINAL: 0x00531020 ?lock_veh@NetDaemon@@QAEHPAHHHHH@Z 0x00531020-0x005310EB BYTE_EXACT
// size      203 bytes
// prototype int (__thiscall ?lock_veh@NetDaemon@@QAEHPAHHHHH@Z)(NetDaemon* this, int*, int, int, int, int)
// callers   32   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0047A890 0x00530630
// 0x1B78 is lock_state_[1].dword_[4], 0x1BB0 lock_state_[2].dword_[3] and
// 0x1BC0 lock_state_[2].dword_[7]. The is-alive word at 0x93E8EC is the
// daemon's own lock_state_[0].dword_[12]. The unit's x and y come out of
// Vehs[veh_id] - the image's `[0x952828 + id*0x34]` and `[0x95282a + ...]`.
// LEVER: Vehs[*veh_id].x/.y through the modelled Veh record, not an extern
// char[] base plus two short locals. The archive had this at MISMATCH #34;
// naming the array is what lets VC6 fold the 0x34 stride into the two
// `movsx reg, word ptr [id*0x34 + 0x952828/0x95282a]` the image emits, and
// it took this body from 33 agreeing instructions to BYTE_EXACT 68/68
// (2026-09-04).
int NetDaemon::lock_veh(int *veh_id, int target, int payload_1C,
                        int payload_20, int payload_18) {
    if (NetDaemonIsMultiplayerNet == 0) {
        return 0;
    }
    if ((NetDaemonNet.lock_state_[0].dword_[12] & (1 << NetDaemonLocalFaction)) != 0 &&
        (NetTurnFlags & 0x10) == 0) {
        NetMsgPopup->pop("ALREADYDONE", 0, 2, 0);
        return 1;
    }

    lock_state_[1].dword_[4] = reinterpret_cast<int>(veh_id);
    int result = lock(target, Vehs[*veh_id].x, Vehs[*veh_id].y, payload_18,
                      payload_1C, payload_20);
    if (result == 0 && *veh_id >= 0) {
        lock_state_[2].dword_[3] = 1;
        lock_state_[2].dword_[7] = 0;
        return 0;
    }
    lock_state_[1].dword_[4] = 0;
    return 1;
}

// ORIGINAL: 0x0052DF30 ?init@NetDaemon@@QAEHH@Z 0x0052DF30-0x0052E041
// TRIED: `if (setup(option)==0) return 0; return 1;` and its `!=`/local-var/ if-else variants all boolean-normalize (neg/sbb or xor/setcc) at #51 where the original keeps two distinct epilogues (fall-through returns the call's own eax, the taken branch explicitly sets eax=1). Landing the closest form.
// size      273 bytes
// prototype int (__thiscall ?init@NetDaemon@@QAEHH@Z)(NetDaemon* this, int)
// callers   1   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004E25B0 0x004E31E0 0x005900D0


int NetDaemon::init(int option) {

    lock_state_[0].dword_[1] = 0;
    lock_state_[0].dword_[2] = 0;
    lock_state_[0].dword_[3] = 0;
    lock_state_[0].dword_[4] = 0;
    lock_state_[0].dword_[13] = 0;
    lock_state_[0].dword_[12] = 0;
    lock_state_[1].dword_[1] = 0;
    lock_state_[1].dword_[0] = 0;
    lock_state_[1].dword_[5] = 0;
    lock_state_[2].dword_[1] = 0;
    lock_state_[1].dword_[14] = 0;
    lock_state_[2].dword_[0] = 0;
    lock_state_[2].dword_[10] = 0;
    lock_state_[2].dword_[9] = 0;
    lock_state_[0].dword_[9] = 0;
    lock_state_[0].dword_[10] = 0;
    lock_state_[0].dword_[11] = 0;
    lock_state_[2].dword_[8] = 0;
    lock_state_[2].dword_[7] = 0;
    lock_state_[2].dword_[3] = 0;
    lock_state_[2].dword_[2] = 0;
    lock_state_[2].dword_[4] = 0;
    lock_state_[0].dword_[5] = 0;
    lock_state_[0].dword_[6] = 0;
    lock_state_[2].dword_[13] = 0;
    lock_state_[2].dword_[12] = 0;
    lock_state_[0].dword_[7] = 0;
    lock_state_[0].dword_[8] = 0;
    lock_state_[2].dword_[11] = 0xff;
    lock_state_[1].dword_[4] = 0;

    uint32_t *p = &lock_state_[1].dword_[6];
    for (int i = 8; i != 0; i--) {
        *p = 0;
        p++;
    }

    locks_.clear();
    AlphaNet::close();

    lock_state_[0].dword_[4] = 0;
    lock_state_[0].dword_[3] = 0;

    if (AlphaNet::setup(option)) {
        return 1;
    }
    else {
        return 0;
    }
}

// Fixed-slot bindings carried from 005303b0.cpp

// ORIGINAL: 0x005303B0 ?wait@NetDaemon@@QAEHPADPAHHH@Z 0x005303B0-0x00530493 FILE
// working copy - scaffold materialised by --work
// size      227 bytes
// prototype int (__thiscall ?wait@NetDaemon@@QAEHPADPAHHH@Z)(NetDaemon* this, int8*, int*, int, int)
// callers   2   call targets   7
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0047A7D0 0x0047A890 0x0051F1B0 0x00534400 0x005901D0 0x005FC700 0x00630A00
// indirect  0x005303CF 0x005303F9

int NetDaemon::wait(char * message, int * state, int value, int timeout) {
    int saved = NetDaemonNet.lock_state_[1].dword_[3];
    ++NetBusyCount;
    typedef unsigned long (__stdcall *TimeGetTimeFn)();
    TimeGetTimeFn timeGetTimeFn = NetTimeGetTimeSlot();
    unsigned long startTime = timeGetTimeFn();
    bool triggeredTimeout = false;
    while (*state == value) {
        if (ExitTurnLoop != 0) {
            break;
        }
        if (!triggeredTimeout) {
            unsigned long now = timeGetTimeFn();
            if ((int)(now - startTime) > timeout) {
                triggeredTimeout = true;
                NetMsgPopup->pop(message, 0, 2, 0);
            }
        }
        wait_task();
        unsigned long outVar;
        int result = NetDaemonNet.Net::get(reinterpret_cast<unsigned long *>(&message), &outVar);
        if (result != 0) {
            process_message(reinterpret_cast<char *>(result), outVar, reinterpret_cast<int>(message));
        }
        locks_.check_global();
        check_spock();
    }
    if (triggeredTimeout) {
        NetMsgPopup->close();
    }
    --NetBusyCount;
    NetDaemonNet.lock_state_[1].dword_[3] = saved;
    return 0;
}

// Fixed-slot bindings carried from 005304a0.cpp

// ORIGINAL: 0x005304A0 ?open_channel@NetDaemon@@QAEHHH@Z 0x005304A0-0x005305FF FILE
// TRIED: 0x00669368 is a function-pointer variable (timeGetTime's IAT slot), called indirectly rather than declared as a normal extern function - matches the disassembly's `call ebx` after `mov ebx,[0x669368]`. Net/NetMsg objects at 0x93cd90/0x805338 are fixed-address singletons. Landed at 0.85 mnemonic similarity.
// working copy - scaffold materialised by --work
// size      351 bytes
// prototype int (__thiscall ?open_channel@NetDaemon@@QAEHHH@Z)(NetDaemon* this, int, int)
// callers   ?   call targets   9
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0047A7D0 0x0047A890 0x0051F1B0 0x00534400 0x005901D0 0x00592EE0
//           0x005FC700 0x006262F0 0x00630A00


int NetDaemon::open_channel(int payload_10, int payload_14) {

  if (NetDaemonIsMultiplayerNet == 0) {
    return 0;
  }
  if (lock_state_[1].dword_[14] != 0) {
    return 1;
  }

  log_say(reinterpret_cast<char *>("Client requesting Diplo Channel"), NetDaemonLocalFaction, 0, 0);
  lock_state_[1].dword_[14] = 1;
  lock_state_[2].dword_[0] = 0;

  message_data(0x8240, 0, payload_10, payload_14, 0, 0);

  TimeGetTimeFn timeGetTime_ = NetTimeGetTimeSlot();
  int savedE904 = NetDaemonNet.lock_state_[1].dword_[3];
  (NetBusyCount)++;
  payload_14 = static_cast<int>(timeGetTime_());

  bool sentAwait = false;
  if (lock_state_[2].dword_[0] == 0) {
    for (;;) {
      if (ExitTurnLoop != 0) {
        break;
      }
      if (!sentAwait) {
        int elapsed = static_cast<int>(timeGetTime_()) - payload_14;
        if (elapsed > 0x640) {
          sentAwait = true;
          NetMsgPopup->pop(reinterpret_cast<const char *>("AWAITCHANNEL"), 0, 2, 0);
        }
      }
      wait_task();

      unsigned long local4;
      int gotten = NetDaemonNet.Net::get(reinterpret_cast<unsigned long *>(&payload_10), &local4);
      if (gotten != 0) {
        process_message(reinterpret_cast<char *>(gotten), local4, payload_10);
      }
      locks_.check_global();
      check_spock();

      if (lock_state_[2].dword_[0] != 0) {
        break;
      }
    }
    if (sentAwait) {
      NetMsgPopup->close();
    }
  }

  (NetBusyCount)--;
  NetDaemonNet.lock_state_[1].dword_[3] = savedE904;

  if (lock_state_[2].dword_[0] < 2) {
    return 0;
  }
  lock_state_[1].dword_[14] = 0;
  return 1;
}

// Fixed-slot bindings carried from 00530630.cpp

// ORIGINAL: 0x00530630 ?lock@NetDaemon@@QAEHHHHHHH@Z 0x00530630-0x0053091E FILE
// TRIED: MISMATCH #2 'push' vs 'mov' - prologue register-save order (ebx/esi/edi) differs from a straight `mov esi, ecx` first statement; did not chase further given function size (750B, 4 nested wait-loops). Body reproduces both wait-for-lock loops (do/while + break on the global abort flag), the target/scratch/x/y stack-slot reuse as scratch locals matching the disassembly's post-use reuse, and the merged final NOLOCK/NOLOCKGLOBAL warning-pop path.
// working copy - scaffold materialised by --work
// size      750 bytes
// prototype int (__thiscall ?lock@NetDaemon@@QAEHHHHHHH@Z)(NetDaemon* this, int, int, int, int, int, int)
// callers   4   call targets   9
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0047A7D0 0x0047A890 0x0051F1B0 0x00534400 0x005901D0 0x00593090 0x005FC700 0x006262F0 0x00630A00
// indirect  0x005306EA 0x0053070F 0x005307E0 0x00530802

int NetDaemon::lock(int target, int x, int y, int payload_18, int payload_1C, int scratch) {
    typedef unsigned long (__stdcall *PTimeGetTime)(void);

    if (NetDaemonIsMultiplayerNet == 0) {
        return 0;
    }
    NetPlayerDropped = 0;

    if (*(int *)(lock_state_[2].dword_[2]) != 0) {
        if (*(int *)(lock_state_[2].dword_[8]) != 0) {
            return 1;
        }
        ((NetMsg *)NetMsgPopup)->pop((const char *)"MOVEPENDING", 0, 2, (const char *)0);
        return 1;
    }

    *(int *)(lock_state_[2].dword_[2]) = 1;
    *(int *)(lock_state_[1].dword_[2]) = 0;
    *(int *)(locks_.field_E8_) = 0;

    log_say((char *)"Client requesting lock", NetDaemonLocalFaction, x, y);
    message_lock(0x2210, target, x, y, payload_18, payload_1C, scratch, 0);

    NetBusyCount = NetBusyCount + 1;
    scratch = NetDaemonNet.lock_state_[1].dword_[3];
    unsigned long t0 = NetTimeGetTimeSlot()();
    int waited = 0;
    if (*(int *)(lock_state_[1].dword_[2]) == 0) {
        do {
            if (ExitTurnLoop != 0) {
                break;
            }
            if (!waited) {
                unsigned long now = NetTimeGetTimeSlot()();
                if (now - t0 > 0x640) {
                    waited = 1;
                    ((NetMsg *)NetMsgPopup)->pop((const char *)"AWAITLOCK", 0, 2, (const char *)0);
                }
            }
            wait_task();
            int got = NetDaemonNet.Net::get((unsigned long *)&y, (unsigned long *)&x);
            if (got != 0) {
                this->process_message((char *)got, (unsigned long)x, y);
            }
            locks_.check_global();
            check_spock();
        } while (*(int *)(lock_state_[1].dword_[2]) == 0);
        if (waited) {
            ((NetMsg *)NetMsgPopup)->close();
        }
    }

    NetBusyCount = NetBusyCount - 1;
    NetDaemonNet.lock_state_[1].dword_[3] = scratch;

    if (*(int *)(lock_state_[1].dword_[2]) == 1) {
        *(int *)(lock_state_[2].dword_[9]) = 0;
        if ((unsigned char)target & 2) {
            NetBusyCount = NetBusyCount + 1;
            scratch = NetDaemonNet.lock_state_[1].dword_[3];
            unsigned long t0b = NetTimeGetTimeSlot()();
            target = (int)t0b;
            int waited2 = 0;
            if (*(int *)(locks_.field_E8_) == 0) {
                do {
                    if (ExitTurnLoop != 0) {
                        break;
                    }
                    if (!waited2) {
                        unsigned long now2 = NetTimeGetTimeSlot()();
                        if (now2 - (unsigned long)target > 0x320) {
                            waited2 = 1;
                            ((NetMsg *)NetMsgPopup)->pop((const char *)"AWAITGLOBALLOCK", 0, 2, (const char *)0);
                        }
                    }
                    wait_task();
                    int got2 = NetDaemonNet.Net::get((unsigned long *)&y, (unsigned long *)&x);
                    if (got2 != 0) {
                        this->process_message((char *)got2, (unsigned long)x, y);
                    }
                    locks_.check_global();
                    check_spock();
                } while (*(int *)(locks_.field_E8_) == 0);
                if (waited2) {
                    ((NetMsg *)NetMsgPopup)->close();
                }
            }
            NetDaemonNet.lock_state_[1].dword_[3] = scratch;
            NetBusyCount = NetBusyCount - 1;
        }
        if (NetPlayerDropped == 0) {
            return 0;
        }
        *(int *)(lock_state_[1].dword_[2]) = 2;
    }

    if (*(int *)(lock_state_[2].dword_[8]) == 0) {
        *(int *)(lock_state_[2].dword_[9]) = *(int *)(lock_state_[2].dword_[9]) + 1;
        if (*(int *)(lock_state_[2].dword_[9]) > 0 || *(int *)(lock_state_[1].dword_[2]) > 2) {
            const char *msg = (*(int *)(lock_state_[1].dword_[2]) == 2) ? (const char *)"NOLOCK" : (const char *)"NOLOCKGLOBAL";
            ((NetMsg *)NetMsgPopup)->pop(msg, 5000, 2, (const char *)0);
            *(int *)(lock_state_[2].dword_[9]) = 0;
        }
    }

    *(int *)(lock_state_[2].dword_[2]) = 0;
    return 1;
}

// Fixed-slot bindings carried from 00530920.cpp

// ORIGINAL: 0x00530920 ?add_lock@NetDaemon@@QAEHHHH@Z 0x00530920-0x00530A91 FILE
// TRIED: reordering the initial `NetDaemonIsMultiplayerNet==0` / `field_1BAC_==0` guards; the mismatch (#5) is deeper, in how the two early returns and the ebx-lazy-save prologue are scheduled.
// size      369 bytes
// prototype int (__thiscall ?add_lock@NetDaemon@@QAEHHHH@Z)(NetDaemon* this, int, int, int)
// callers   2   call targets   9
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0047A7D0 0x0047A890 0x0051F1B0 0x00534400 0x005901D0 0x00593090 0x005FC700 0x006262F0 0x00630A00
// indirect  0x005309A6 0x005309C6

int NetDaemon::add_lock(int target, int x, int y) {
    if (NetDaemonIsMultiplayerNet == 0) {
        return 0;
    }
    if (lock_state_[2].dword_[2] == 0) {
        return 1;
    }

    log_say(reinterpret_cast<char *>("Client requesting ADD LOCK"), NetDaemonLocalFaction, 0, 0);
    lock_state_[1].dword_[2] = 0;
    message_lock(0x2215, target, x, y, 0, -1, -1, 0);

    int savedVal = NetDaemonNet.lock_state_[1].dword_[3];
    typedef unsigned long (__stdcall *FuncTimeGetTime)();
    FuncTimeGetTime timeGetTimeFn = NetTimeGetTimeSlot();
    ++NetBusyCount;
    y = timeGetTimeFn();

    bool popped = false;
    NetMsg *netMsg = NetMsgPopup;
    Lock &lock = locks_;

    if (lock_state_[1].dword_[2] == 0) {
        do {
            if (ExitTurnLoop != 0) {
                break;
            }
            if (!popped) {
                if (static_cast<unsigned int>(timeGetTimeFn() - y) > 0x640) {
                    popped = true;
                    netMsg->pop(reinterpret_cast<const char *>("AWAITLOCK"), 0, 2, 0);
                }
            }
            wait_task();
            unsigned long msgResult = NetDaemonNet.Net::get(reinterpret_cast<unsigned long *>(&x),
                                                reinterpret_cast<unsigned long *>(&target));
            if (msgResult != 0) {
                process_message(reinterpret_cast<char *>(msgResult),
                                 static_cast<unsigned long>(target), x);
            }
            lock.check_global();
            check_spock();
        } while (lock_state_[1].dword_[2] == 0);
        if (popped) {
            netMsg->close();
        }
    }

    --NetBusyCount;
    NetDaemonNet.lock_state_[1].dword_[3] = savedVal;
    if (lock_state_[1].dword_[2] != 1) {
        netMsg->pop(reinterpret_cast<const char *>("NOLOCK"), 5000, 2, 0);
    }
    return lock_state_[1].dword_[2] != 1;
}

// Fixed-slot bindings carried from 00530aa0.cpp

// ORIGINAL: 0x00530AA0 ?add_global@NetDaemon@@QAEHXZ 0x00530AA0-0x00530CFF FILE
// TRIED: translated the Ghidra hypothesis directly (two near-identical wait-for-global-lock phases, each: warn-once via NetMsg::pop after a timeGetTime() timeout, Net::get()/process_message() poll loop, Lock::check_global()+check_spock(), then NetMsg::close() if warned). MISMATCH #5 xor/mov - looks like register allocation for the ebx=0 constant, not a structural difference; body compiles and follows the disasm's two-phase shape and final NOLOCK/NOLOCKGLOBAL message selection exactly.
// working copy - scaffold materialised by --work
// size      607 bytes
// prototype int (__thiscall ?add_global@NetDaemon@@QAEHXZ)(NetDaemon* this)
// callers   12   call targets   8
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0047A7D0 0x0047A890 0x0051F1B0 0x00534400 0x005901D0 0x00592EE0 0x005FC700 0x00630A00
// indirect  0x00530AF8 0x00530B1D 0x00530BC1 0x00530BE6

int NetDaemon::add_global() {
    typedef unsigned long (__stdcall *TimeGetTimeFn)();
    TimeGetTimeFn get_time = NetTimeGetTimeSlot();

    if (NetDaemonIsMultiplayerNet == 0) {
        return 0;
    }
    NetPlayerDropped = 0;
    lock_state_[1].dword_[2] = 0;
    locks_.field_E8_ = 0;
    message_data(0x2211, 0, 0, 0, 0, 0);
    NetBusyCount = NetBusyCount + 1;
    int saved_e904 = NetDaemonNet.lock_state_[1].dword_[3];
    unsigned long start_time1 = get_time();

    bool warned1 = false;
    if (lock_state_[1].dword_[2] == 0) {
        do {
            if (ExitTurnLoop != 0) break;
            if (!warned1) {
                unsigned long now = get_time();
                if (now - start_time1 > 0x640) {
                    warned1 = true;
                    NetMsgPopup->pop(reinterpret_cast<const char *>("AWAITGLOBALLOCK"), 0, 2, 0);
                }
            }
            wait_task();
            unsigned long out_id, out_len;
            int result = NetDaemonNet.Net::get(&out_id, &out_len);
            if (result != 0) {
                process_message(reinterpret_cast<char *>(result), out_len, static_cast<int>(out_id));
            }
            locks_.check_global();
            check_spock();
        } while (lock_state_[1].dword_[2] == 0);
        if (warned1) {
            NetMsgPopup->close();
        }
    }
    NetDaemonNet.lock_state_[1].dword_[3] = saved_e904;
    int newE664 = NetBusyCount - 1;

    if (lock_state_[1].dword_[2] == 1) {
        unsigned long start_time2 = get_time();
        bool warned2 = false;
        if (locks_.field_E8_ == 0) {
            do {
                if (ExitTurnLoop != 0) break;
                if (!warned2) {
                    unsigned long now = get_time();
                    if (now - start_time2 > 800) {
                        warned2 = true;
                        NetMsgPopup->pop(reinterpret_cast<const char *>("AWAITGLOBALLOCK"), 0, 2, 0);
                    }
                }
                wait_task();
                unsigned long out_id2, out_len2;
                int result2 = NetDaemonNet.Net::get(&out_id2, &out_len2);
                if (result2 != 0) {
                    process_message(reinterpret_cast<char *>(result2), out_len2, static_cast<int>(out_id2));
                }
                locks_.check_global();
                check_spock();
            } while (locks_.field_E8_ == 0);
            if (warned2) {
                NetMsgPopup->close();
            }
        }
        NetDaemonNet.lock_state_[1].dword_[3] = saved_e904;
        NetBusyCount = NetBusyCount - 1;
        if (NetPlayerDropped == 0) {
            lock_state_[2].dword_[9] = 0;
            return 0;
        }
        lock_state_[1].dword_[2] = 2;
        newE664 = NetBusyCount;
    }
    NetBusyCount = newE664;

    if (lock_state_[2].dword_[8] == 0) {
        int newBc8 = lock_state_[2].dword_[9] + 1;
        lock_state_[2].dword_[9] = newBc8;
        if (newBc8 > 0 || lock_state_[1].dword_[2] > 2) {
            const char *msg = reinterpret_cast<const char *>("NOLOCK");
            if (lock_state_[1].dword_[2] != 2) {
                msg = reinterpret_cast<const char *>("NOLOCKGLOBAL");
            }
            NetMsgPopup->pop(msg, 5000, 2, 0);
            lock_state_[2].dword_[9] = 0;
        }
    }
    lock_state_[2].dword_[2] = 0;
    return 1;
}

// Fixed-slot bindings carried from 00530e50.cpp

// ORIGINAL: 0x00530E50 ?unlock_base@NetDaemon@@QAEXH@Z 0x00530E50-0x0053101F FILE
// size      463 bytes
// prototype void (__thiscall ?unlock_base@NetDaemon@@QAEXH@Z)(NetDaemon* this, int)
// callers   26   call targets   10
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0047A7D0 0x0047A890 0x0051F1B0 0x00534400 0x005901D0 0x00592EE0 0x00593250 0x005FC700 0x006262F0 0x00630A00
// indirect  0x00530EED 0x00530F13
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00530E50
// measured tier  MISMATCH
// divergence     88
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00530e50/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?unlock_base@NetDaemon@@QAEXH@Z  at 0x00530E50  (463 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// The VC6 dialect limits and the source-form rules used to live here.
// They are knowledge, not scaffolding, so they now live in the agent
// system prompt (mizuchi.yaml, plugins.claude-runner.systemPrompt),
// where they can be edited without regenerating anything and are in
// context from the first token rather than behind a file read. This
// emitter computes declarations; it does not carry lessons.

void NetDaemon::unlock_base(int base_id) {

    log_say(reinterpret_cast<char *>("UnlockBase"), NetDaemonNet.lock_state_[2].dword_[2], base_id, 0);

    if (NetDaemonIsMultiplayerNet != 0 && NetDaemonNet.lock_state_[2].dword_[2] != 0) {
        log_say(reinterpret_cast<char *>("UnlockBase doing synch & unlock"), 0, 0, 0);
        synch_base(base_id);

        if (NetDaemonIsMultiplayerNet != 0) {
            lock_state_[2].dword_[5] = 1;
            log_say(reinterpret_cast<char *>("Awaiting acknowledgement of SYNCH"), NetDaemonLocalFaction, 0, 0);
            int saved = NetDaemonNet.lock_state_[1].dword_[3];
            ++NetBusyCount;
            unsigned long baseline = NetTimeGetTimeSlot()();

            int logged_wait = 0;
            if (lock_state_[2].dword_[5] == 1) {
                do {
                    if (ExitTurnLoop != 0) {
                        break;
                    }
                    if (logged_wait == 0) {
                        if (NetTimeGetTimeSlot()() - baseline > 0x7d0) {
                            logged_wait = 1;
                            NetMsgPopup->pop(
                                reinterpret_cast<char *>("AWAITEXEC"), 0, 2, 0);
                        }
                    }
                    wait_task();
                    unsigned long p1, p2;
                    int r = NetDaemonNet.Net::get(&p1, &p2);
                    if (r != 0) {
                        process_message(reinterpret_cast<char *>(r), p2, p1);
                    }
                    locks_.check_global();
                    check_spock();
                } while (lock_state_[2].dword_[5] == 1);
                if (logged_wait) {
                    NetMsgPopup->close();
                }
            }

            --NetBusyCount;
            NetDaemonNet.lock_state_[1].dword_[3] = saved;
            if (lock_state_[2].dword_[5] == 0) {
                log_say(reinterpret_cast<char *>("Received SYNCH acknowledgement"), NetDaemonLocalFaction, 0, 0);
            }
            lock_state_[2].dword_[5] = 0;

            if (NetDaemonIsMultiplayerNet != 0) {
                log_say(reinterpret_cast<char *>("Client releasing lock"), NetDaemonLocalFaction, 0, 0);
                message_data(0x2212, 0, 0, 0, 0, 0);
                lock_state_[2].dword_[3] = 0;
                lock_state_[2].dword_[2] = 0;
                lock_state_[2].dword_[10] = 0;
                lock_state_[2].dword_[9] = 0;
            }
        }
    }
}

// Fixed-slot bindings carried from 00531300.cpp

// ORIGINAL: 0x00531300 ?await_exec@NetDaemon@@QAEXH@Z 0x00531300-0x00531477 FILE
// size      375 bytes
// prototype void (__thiscall ?await_exec@NetDaemon@@QAEXH@Z)(NetDaemon* this, int)
// callers   17   call targets   9
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0047A7D0 0x0047A890 0x0051F1B0 0x00534400 0x005901D0 0x00592EE0 0x005FC700 0x006262F0 0x00630A00
// indirect  0x00531345 0x0053136B
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00531300
// measured tier  MISMATCH
// divergence     18
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00531300/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?await_exec@NetDaemon@@QAEXH@Z  at 0x00531300  (375 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// The VC6 dialect limits and the source-form rules used to live here.
// They are knowledge, not scaffolding, so they now live in the agent
// system prompt (mizuchi.yaml, plugins.claude-runner.systemPrompt),
// where they can be edited without regenerating anything and are in
// context from the first token rather than behind a file read. This
// emitter computes declarations; it does not carry lessons.


void NetDaemon::await_exec(int opcode) {

    if (NetDaemonIsMultiplayerNet == 0) {
        return;
    }

    log_say(reinterpret_cast<char *>("Client waiting for acknowledgement"), NetDaemonLocalFaction, 0, 0);
    int saved = NetDaemonNet.lock_state_[1].dword_[3];
    ++NetBusyCount;
    unsigned long baseline = NetTimeGetTimeSlot()();

    int logged_wait = 0;
    if (lock_state_[2].dword_[3] == 1) {
        do {
            if (ExitTurnLoop != 0) {
                break;
            }
            if (logged_wait == 0) {
                if (NetTimeGetTimeSlot()() - baseline > 0x7d0) {
                    logged_wait = 1;
                    NetMsgPopup->pop(
                        reinterpret_cast<char *>("AWAITEXEC"), 0, 2, 0);
                }
            }
            wait_task();
            unsigned long p1, p2;
            int r = NetDaemonNet.Net::get(&p1, &p2);
            if (r != 0) {
                process_message(reinterpret_cast<char *>(r), p2, p1);
            }
            locks_.check_global();
            check_spock();
        } while (lock_state_[2].dword_[3] == 1);
        if (logged_wait) {
            NetMsgPopup->close();
        }
    }

    --NetBusyCount;
    NetDaemonNet.lock_state_[1].dword_[3] = saved;
    log_say(reinterpret_cast<char *>("Client received acknowledgement"), NetDaemonLocalFaction, 0, 0);

    if (opcode != 0) {
        if (NetDaemonIsMultiplayerNet != 0) {
            log_say(reinterpret_cast<char *>("Client releasing lock"), NetDaemonLocalFaction, 0, 0);
            message_data(0x2212, 0, 0, 0, 0, 0);
            lock_state_[2].dword_[3] = 0;
            lock_state_[2].dword_[2] = 0;
            lock_state_[2].dword_[10] = 0;
            lock_state_[2].dword_[9] = 0;
        }
        lock_state_[1].dword_[4] = 0;
        lock_state_[2].dword_[8] = 0;
    }
    lock_state_[2].dword_[3] = 0;
}

// ORIGINAL: 0x00531480 ?await_synch@NetDaemon@@QAEXXZ 0x00531480-0x005315B6 FILE
// size      310 bytes
// prototype void (__thiscall ?await_synch@NetDaemon@@QAEXXZ)(NetDaemon* this)
// callers   22   call targets   8
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0047A7D0 0x0047A890 0x0051F1B0 0x00534400 0x005901D0 0x005FC700 0x006262F0 0x00630A00
// indirect  0x005314D0 0x005314F6
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00531480
// measured tier  MISMATCH
// divergence     19
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00531480/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?await_synch@NetDaemon@@QAEXXZ  at 0x00531480  (310 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// The VC6 dialect limits and the source-form rules used to live here.
// They are knowledge, not scaffolding, so they now live in the agent
// system prompt (mizuchi.yaml, plugins.claude-runner.systemPrompt),
// where they can be edited without regenerating anything and are in
// context from the first token rather than behind a file read. This
// emitter computes declarations; it does not carry lessons.


void NetDaemon::await_synch() {
    uint32_t *state = &lock_state_[2].dword_[5];

    if (NetDaemonIsMultiplayerNet != 0) {
        int synching = 1;
        *state = synching;
        log_say(reinterpret_cast<char *>("Awaiting acknowledgement of SYNCH"), NetDaemonLocalFaction, 0, 0);

        int saved = NetDaemonNet.lock_state_[1].dword_[3];
        (NetBusyCount)++;
        unsigned long start_time = NetTimeGetTimeSlot()();

        int warned = 0;
        if (*state == synching) {
            do {
                if (ExitTurnLoop != 0) {
                    break;
                }
                if (!warned) {
                    unsigned long now =
                        NetTimeGetTimeSlot()();
                    if (now - start_time > 0x7d0) {
                        warned = synching;
                        NetMsgPopup->pop(
                            reinterpret_cast<const char *>("AWAITEXEC"), 0, 2, 0);
                    }
                }
                wait_task();
                unsigned long v1, v2;
                int r = NetDaemonNet.Net::get(&v1, &v2);
                if (r != 0) {
                    process_message(reinterpret_cast<char *>(r), v2,
                                    static_cast<int>(v1));
                }
                locks_.check_global();
                check_spock();
            } while (*state == synching);
            if (warned) {
                NetMsgPopup->close();
            }
        }

        (NetBusyCount)--;
        NetDaemonNet.lock_state_[1].dword_[3] = saved;
        if (*state == 0) {
            log_say(reinterpret_cast<char *>("Received SYNCH acknowledgement"), NetDaemonLocalFaction, 0, 0);
        }
        *state = 0;
    }
}

// Fixed-slot bindings carried from 005315c0.cpp

// ORIGINAL: 0x005315C0 ?await_diplo@NetDaemon@@QAEXH@Z 0x005315C0-0x005316F9 FILE
// size      313 bytes
// prototype void (__thiscall ?await_diplo@NetDaemon@@QAEXH@Z)(NetDaemon* this, int)
// callers   35   call targets   8
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0047A7D0 0x0047A890 0x0051F1B0 0x00534400 0x005901D0 0x005FC700 0x006262F0 0x00630A00
// indirect  0x0053160E 0x00531634
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x005315C0
// measured tier  MISMATCH
// divergence     20
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005315c0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?await_diplo@NetDaemon@@QAEXH@Z  at 0x005315C0  (313 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// The VC6 dialect limits and the source-form rules used to live here.
// They are knowledge, not scaffolding, so they now live in the agent
// system prompt (mizuchi.yaml, plugins.claude-runner.systemPrompt),
// where they can be edited without regenerating anything and are in
// context from the first token rather than behind a file read. This
// emitter computes declarations; it does not carry lessons.


void NetDaemon::await_diplo(int opcode) {
    if (NetDaemonIsMultiplayerNet != 0) {
        lock_state_[2].dword_[1] = opcode;
        log_say(reinterpret_cast<char *>("Awaiting DIPLO event"), NetDaemonLocalFaction, 0, 0);
        (NetBusyCount)++;
        int32_t saved = NetDaemonNet.lock_state_[1].dword_[3];
        unsigned int start_time = NetTimeGetTimeSlot()();
        int timed_out = 0;
        if (lock_state_[2].dword_[1] == opcode) {
            do {
                if (ExitTurnLoop != 0) {
                    break;
                }
                if (!timed_out) {
                    unsigned int now = NetTimeGetTimeSlot()();
                    if (now - start_time > 0x7d0) {
                        timed_out = 1;
                        NetMsgPopup->pop(
                            reinterpret_cast<const char *>("AWAITDIPLO"), 0, 2, 0);
                    }
                }
                wait_task();
                unsigned long v_at_4;
                unsigned long v_at_8;
                int result = NetDaemonNet.Net::get(&v_at_4, &v_at_8);
                if (result != 0) {
                    process_message(reinterpret_cast<char *>(result), v_at_8, v_at_4);
                }
                locks_.check_global();
                check_spock();
            } while (lock_state_[2].dword_[1] == opcode);
            if (timed_out) {
                NetMsgPopup->close();
            }
        }
        (NetBusyCount)--;
        NetDaemonNet.lock_state_[1].dword_[3] = saved;
        if (lock_state_[2].dword_[1] == 0) {
            log_say(reinterpret_cast<char *>("Received DIPLO acknowledgement"), NetDaemonLocalFaction, 0, 0);
        }
        lock_state_[2].dword_[1] = 0;
    }
}

class DeletionList { public:
    uint32_t field_0_;
    uint32_t field_4_;
    uint32_t field_8_;
    uint32_t field_C_;
    uint32_t field_10_;
    uint32_t field_14_;
    uint32_t field_18_;
    uint32_t field_1C_;
    uint32_t field_20_;
    uint32_t field_24_;
    uint32_t field_28_;
    uint32_t field_2C_;
    uint32_t field_30_;
    uint32_t field_34_;
    uint32_t field_38_;
    uint32_t field_3C_;
    uint32_t field_40_;
    uint32_t field_44_;
    uint32_t field_48_;
    uint32_t field_4C_;
    uint32_t field_50_;
    uint32_t field_54_;
    uint32_t field_58_;
    uint32_t field_5C_;
    uint32_t field_60_;
    uint32_t field_64_;
    uint32_t field_68_;
    uint32_t field_6C_;
    uint32_t field_70_;
    uint32_t field_74_;
    uint32_t field_78_;
    uint32_t field_7C_;
    uint32_t field_80_;
    uint32_t field_84_;
    uint32_t field_88_;
    uint32_t field_8C_;
    uint32_t field_90_;
    uint32_t field_94_;
    uint32_t field_98_;
    uint32_t field_9C_;
    uint32_t field_A0_;
    uint32_t field_A4_;
    uint32_t field_A8_;
    uint32_t field_AC_;
    uint32_t field_B0_;
    uint32_t field_B4_;
    uint32_t field_B8_;
    uint32_t field_BC_;
    uint32_t field_C0_;
    uint32_t field_C4_;
    uint32_t field_C8_;
    uint32_t field_CC_;
    uint32_t field_D0_;
    uint32_t field_D4_;
    uint32_t field_D8_;
    uint32_t field_DC_;
    uint32_t field_E0_;
    uint32_t field_E4_;
    uint32_t field_E8_;
    uint32_t field_EC_;
    uint32_t field_F0_;
    uint32_t field_F4_;
    uint32_t field_F8_;
    uint32_t field_FC_;
    uint32_t field_100_;
    uint32_t field_104_;
    uint32_t field_108_;
    uint32_t field_10C_;
    uint32_t field_110_;
    uint32_t field_114_;
    uint32_t field_118_;
    uint32_t field_11C_;
    uint32_t field_120_;
    uint32_t field_124_;
    uint32_t field_128_;
    uint32_t field_12C_;
    uint32_t field_130_;
    uint32_t field_134_;
    uint32_t field_138_;
    uint32_t field_13C_;
    uint32_t field_140_;
    uint32_t field_144_;
    uint32_t field_148_;
    uint32_t field_14C_;
    uint32_t field_150_;
    uint32_t field_154_;
    uint32_t field_158_;
    uint32_t field_15C_;
    uint32_t field_160_;
    uint32_t field_164_;
    uint32_t field_168_;
    uint32_t field_16C_;
    uint32_t field_170_;
    uint32_t field_174_;
    uint32_t field_178_;
    uint32_t field_17C_;
    uint32_t field_180_;
    uint32_t field_184_;
    uint32_t field_188_;
    uint32_t field_18C_;
    uint32_t field_190_;
    uint32_t field_194_;
    uint32_t field_198_;
    uint32_t field_19C_;
    uint32_t field_1A0_;
    uint32_t field_1A4_;
    uint32_t field_1A8_;
    uint32_t field_1AC_;
    uint32_t field_1B0_;
    uint32_t field_1B4_;
    uint32_t field_1B8_;
    uint32_t field_1BC_;
    uint32_t field_1C0_;
    uint32_t field_1C4_;
    uint32_t field_1C8_;
    uint32_t field_1CC_;
    uint32_t field_1D0_;
    uint32_t field_1D4_;
    uint32_t field_1D8_;
    uint32_t field_1DC_;
    uint32_t field_1E0_;
    uint32_t field_1E4_;
    uint32_t field_1E8_;
    uint32_t field_1EC_;
    uint32_t field_1F0_;
    uint32_t field_1F4_;
    uint32_t field_1F8_;
    uint32_t field_1FC_;
    uint32_t field_200_;
    uint32_t field_204_;
    uint32_t field_208_;
    uint32_t field_20C_;
    uint32_t field_210_;
    uint32_t field_214_;
    uint32_t field_218_;
    uint32_t field_21C_;
    uint32_t field_220_;
    uint32_t field_224_;
    uint32_t field_228_;
    uint32_t field_22C_;
    uint32_t field_230_;
    uint32_t field_234_;
    uint32_t field_238_;
    uint32_t field_23C_;
    uint32_t field_240_;
    uint32_t field_244_;
    uint32_t field_248_;
    uint32_t field_24C_;
    uint32_t field_250_;
    uint32_t field_254_;
    uint32_t field_258_;
    uint32_t field_25C_;
    uint32_t field_260_;
    uint32_t field_264_;
    uint32_t field_268_;
    uint32_t field_26C_;
    uint32_t field_270_;
    uint32_t field_274_;
    uint32_t field_278_;
    uint32_t field_27C_;
    uint32_t field_280_;
    uint32_t field_284_;
    uint32_t field_288_;
    uint32_t field_28C_;
    uint32_t field_290_;
    uint32_t field_294_;
    uint32_t field_298_;
    uint32_t field_29C_;
    uint32_t field_2A0_;
    uint32_t field_2A4_;
    uint32_t field_2A8_;
    uint32_t field_2AC_;
    uint32_t field_2B0_;
    uint32_t field_2B4_;
    uint32_t field_2B8_;
    uint32_t field_2BC_;
    uint32_t field_2C0_;
    uint32_t field_2C4_;
    uint32_t field_2C8_;
    uint32_t field_2CC_;
    uint32_t field_2D0_;
    uint32_t field_2D4_;
    uint32_t field_2D8_;
    uint32_t field_2DC_;
    uint32_t field_2E0_;
    uint32_t field_2E4_;
    uint32_t field_2E8_;
    uint32_t field_2EC_;
    uint32_t field_2F0_;
    uint32_t field_2F4_;
    uint32_t field_2F8_;
    uint32_t field_2FC_;
    uint32_t field_300_;
    uint32_t field_304_;
    uint32_t field_308_;
    uint32_t field_30C_;
    uint32_t field_310_;
    uint32_t field_314_;
    uint32_t field_318_;
    uint32_t field_31C_;
    uint32_t field_320_;
    uint32_t field_324_;
    uint32_t field_328_;
    uint32_t field_32C_;
    uint32_t field_330_;
    uint32_t field_334_;
    uint32_t field_338_;
    uint32_t field_33C_;
    uint32_t field_340_;
    uint32_t field_344_;
    uint32_t field_348_;
    uint32_t field_34C_;
    uint32_t field_350_;
    uint32_t field_354_;
    uint32_t field_358_;
    uint32_t field_35C_;
    uint32_t field_360_;
    uint32_t field_364_;
    uint32_t field_368_;
    uint32_t field_36C_;
    uint32_t field_370_;
    uint32_t field_374_;
    uint32_t field_378_;
    uint32_t field_37C_;
    uint32_t field_380_;
    uint32_t field_384_;
    uint32_t field_388_;
    uint32_t field_38C_;
    uint32_t field_390_;
    uint32_t field_394_;
    uint32_t field_398_;
    uint32_t field_39C_;
    uint32_t field_3A0_;
    uint32_t field_3A4_;
    uint32_t field_3A8_;
    uint32_t field_3AC_;
    uint32_t field_3B0_;
    uint32_t field_3B4_;
    uint32_t field_3B8_;
    uint32_t field_3BC_;
    uint32_t field_3C0_;
    uint32_t field_3C4_;
    uint32_t field_3C8_;
    uint32_t field_3CC_;
    uint32_t field_3D0_;
    uint32_t field_3D4_;
    uint32_t field_3D8_;
    uint32_t field_3DC_;
    uint32_t field_3E0_;
    uint32_t field_3E4_;
    uint32_t field_3E8_;
    uint32_t field_3EC_;
    uint32_t field_3F0_;
    uint32_t field_3F4_;
    uint32_t field_3F8_;
    uint32_t field_3FC_;
    uint32_t field_400_;
    uint32_t field_404_;
    uint32_t field_408_;
    uint32_t field_40C_;
    uint32_t field_410_;
    uint32_t field_414_;
    uint32_t field_418_;
    uint32_t field_41C_;
    uint32_t field_420_;
    uint32_t field_424_;
    uint32_t field_428_;
    uint32_t field_42C_;
    uint32_t field_430_;
    uint32_t field_434_;
    uint32_t field_438_;
    uint32_t field_43C_;
    uint32_t field_440_;
    uint32_t field_444_;
    uint32_t field_448_;
    uint32_t field_44C_;
    uint32_t field_450_;
    uint32_t field_454_;
    uint32_t field_458_;
    uint32_t field_45C_;
    uint32_t field_460_;
    uint32_t field_464_;
    uint32_t field_468_;
    uint32_t field_46C_;
    uint32_t field_470_;
    uint32_t field_474_;
    uint32_t field_478_;
    uint32_t field_47C_;
    uint32_t field_480_;
    uint32_t field_484_;
    uint32_t field_488_;
    uint32_t field_48C_;
    uint32_t field_490_;
    uint32_t field_494_;
    uint32_t field_498_;
    uint32_t field_49C_;
    uint32_t field_4A0_;
    uint32_t field_4A4_;
    uint32_t field_4A8_;
    uint32_t field_4AC_;
    uint32_t field_4B0_;
    uint32_t field_4B4_;
    uint32_t field_4B8_;
    uint32_t field_4BC_;
    uint32_t field_4C0_;
    uint32_t field_4C4_;
    uint32_t field_4C8_;
    uint32_t field_4CC_;
    uint32_t field_4D0_;
    uint32_t field_4D4_;
    uint32_t field_4D8_;
    uint32_t field_4DC_;
    uint32_t field_4E0_;
    uint32_t field_4E4_;
    uint32_t field_4E8_;
    uint32_t field_4EC_;
    uint32_t field_4F0_;
    uint32_t field_4F4_;
    uint32_t field_4F8_;
    uint32_t field_4FC_;
    uint32_t field_500_;
    uint32_t field_504_;
    uint32_t field_508_;
    uint32_t field_50C_;
    uint32_t field_510_;
    uint32_t field_514_;
    uint32_t field_518_;
    uint32_t field_51C_;
    uint32_t field_520_;
    uint32_t field_524_;
    uint32_t field_528_;
    uint32_t field_52C_;
    uint32_t field_530_;
    uint32_t field_534_;
    uint32_t field_538_;
    uint32_t field_53C_;
    uint32_t field_540_;
    uint32_t field_544_;
    uint32_t field_548_;
    uint32_t field_54C_;
    uint32_t field_550_;
    uint32_t field_554_;
    uint32_t field_558_;
    uint32_t field_55C_;
    uint32_t field_560_;
    uint32_t field_564_;
    uint32_t field_568_;
    uint32_t field_56C_;
    uint32_t field_570_;
    uint32_t field_574_;
    uint32_t field_578_;
    uint32_t field_57C_;
    uint32_t field_580_;
    uint32_t field_584_;
    uint32_t field_588_;
    uint32_t field_58C_;
    uint32_t field_590_;
    uint32_t field_594_;
    uint32_t field_598_;
    uint32_t field_59C_;
    uint32_t field_5A0_;
    int convert(int, int);
};

// ORIGINAL: 0x00534330 ?fixup_message@NetDaemon@@QAEHPAD@Z 0x00534330-0x005343FC FILE
// TRIED: MISMATCH, closest at 212/204 bytes (mnemonic_similarity 0.87, 3 edit blocks). `goto fail;`/`fail:` in place of five scattered `return 0;` was needed to get VC6 to share ONE epilogue the way the original's disassembly jumps into a single shared block - plain early returns each got their own copy. `int code` (signed) not `unsigned short`/`unsigned int` reproduces the `cmp eax,CONST` 32-bit signed compares (jg/jge) the original uses, and writing the boundaries as `<= 0x2406`/`<= 0x2402` (not `< 0x2407`/`< 0x2403`) matches the exact `jg` vs `jge` original picked for each - the `x<N` vs `x<=N-1` lever. `DeletionList*` for `this+0x158c` inlined at each of the three call sites (not cached in an upfront local) matches the original computing it lazily, right before its first use. Remaining diff: the `code== 0x2403` arm compiles to `jne fail; jmp common` where the original does a single `je common`, and the `0x2408 || 0x2413` pair still compiles as two `cmp/je` where the original uses a `sub eax,0x2408; je; sub eax,0xb; jne` switch-lowering idiom - not chased further.
// working copy - scaffold materialised by --work
// size      204 bytes
// prototype int (__thiscall ?fixup_message@NetDaemon@@QAEHPAD@Z)(NetDaemon* this, int8*)
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0052DDC0

int NetDaemon::fixup_message(char * message) {
    unsigned short *msg = reinterpret_cast<unsigned short *>(message);
    int code = *msg;
    if (code <= 0x2406) {
        if (code < 0x2404) {
            if (code <= 0x2402) {
                if (code < 0x2401) {
                    if (code != 0x2101) {
                        goto fail;
                    }
                    if (msg[8] != 0x11) {
                        goto fail;
                    }
                    int r1 = reinterpret_cast<DeletionList *>(lock_table_)
                                 ->convert(*reinterpret_cast<int *>(msg + 10), *reinterpret_cast<int *>(msg + 6));
                    *reinterpret_cast<int *>(msg + 10) = r1;
                    if (*(reinterpret_cast<char *>(msg) + 0x35) != 1) {
                        goto fail;
                    }
                    if (static_cast<short>(msg[0x1c]) < 0) {
                        goto fail;
                    }
                    int r2 = reinterpret_cast<DeletionList *>(lock_table_)
                                 ->convert(static_cast<short>(msg[0x1c]), *reinterpret_cast<int *>(msg + 6));
                    unsigned short r2s = static_cast<unsigned short>(r2);
                    msg[0x1c] = r2s;
                    if (r2s != 0xffff) {
                        goto fail;
                    }
                    *(reinterpret_cast<char *>(msg) + 0x35) = 0;
                    goto fail;
                }
            } else if (code != 0x2403) {
                goto fail;
            }
        }
    } else if (code != 0x2408 && code != 0x2413) {
        goto fail;
    }
    {
        int r3 = reinterpret_cast<DeletionList *>(lock_table_)
                     ->convert(*reinterpret_cast<int *>(msg + 8), *reinterpret_cast<int *>(msg + 6));
        *reinterpret_cast<int *>(msg + 8) = r3;
        if (r3 >= 0) {
            goto fail;
        }
        return 1;
    }
fail:
    return 0;
}

// Fixed-slot bindings carried from 00534400.cpp

class Filemap { public:
    LPVOID map_view_addr_;
    HANDLE file_;
    HANDLE file_map_;
    uint32_t file_size_;
    Filemap();
    int create(char *, unsigned long, int);
    uint32_t get_size();
    void close();
    ~Filemap();
};

// ORIGINAL: 0x00534400 ?process_message@NetDaemon@@QAEXPADKH@Z 0x00534400-0x005388B9;0x0065E0C2-0x0065E442 FILE
// TRIED: no Ghidra hypothesis was available for this one (raw asm only). The real dispatcher spans ~67 distinct message-type case bodies across several chained range checks up to type 0x1300+; only the first band (0xF05-0xF0F, 11 cases) and the shared default "fixup_message + send_message" forwarding path (the target of the large majority of jump-table slots) were read from the disassembly and modelled. Everything above type 0x1101 is unimplemented.
// working copy - scaffold materialised by --work
// size      18489 bytes
// prototype void (__thiscall ?process_message@NetDaemon@@QAEXPADKH@Z)(NetDaemon* this, MessageFactionData*, unsigned int, int)
// callers   18   call targets   133
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004048A0 0x00404900 0x00406380 0x004064D0 0x00406880 0x00424410 0x00424720 0x004271C0 0x00428350 0x004412F0 0x004413F0 0x00441490 0x004426F0 0x00442740 0x004429C0 0x00444D90 0x00446A00 0x0045C0B0 0x0046B190 0x00479330 0x0047A890 0x0047C970 0x00481AD0 0x00481DA0 0x00481FD0 0x00483420 0x0048C0A0 0x004ACBD0 0x004C96E0 0x004C9B00 0x004CA7F0 0x004CAA50 0x004CC360 0x004CD2F0 0x004CD4E0 0x004CE210 0x004CE790 0x004CEDE0 0x004CF380 0x004CF480 0x004CF740 0x004E25E0 0x004E2610 0x004E2660 0x004E26B0 0x004EFB50 0x00506A60 0x0050BA30 0x0050BCC0 0x00523C60 0x00532940 0x00532E00 0x00534330 0x00534400 0x00538FB0 0x00538FE0 0x0053C650 0x00543A20 0x00548F80 0x0054CF40 0x0055B760 0x0055B820 0x0055B870 0x0055BA80 0x0055BB30 0x0055BBA0 0x0055BC00 0x0057F1D0 0x0058FB00 0x0058FC50 0x0058FFA0 0x00590170 0x005901D0 0x00590240 0x005902C0 0x00590300 0x00590470 0x00592550 0x00592600 0x005926F0 0x00592EE0 0x005947C0 0x005B9C40 0x005BB000 0x005BF310 0x005BF420 0x005C0B00 0x005C89A0 0x005C89B0 0x005D4510 0x005D45B0 0x005D4DD0 0x005D5A70 0x005E3820 0x005EE280 0x005F7E90 0x005FA870 0x00600F00 0x00602600 0x00605370 0x00607040 0x00607DA0 0x00608E10 0x00609EC0 0x0060D1B0 0x0060E740 0x00610120 0x006108E0 0x00611A20 0x00612960 0x006169A0 0x00625810 0x00625E30 0x00625E50 0x00625EC0 0x00626250 0x006262F0 0x006263F0 0x006272F0 0x00628380 0x006283E0 0x00628650 0x006287C0 0x00630080 0x00630A00 0x00631A20 0x00634330 0x00644EF2 0x00645460 0x00645470 0x00645550 0x00645930 0x0064FC88
// indirect  0x00534EC9 0x00534ED0 0x005374DF 0x00537F83 0x00538561 0x00538573 0x00538595 0x00538611 0x00538623 0x00538645 0x005386E0 0x005386F4 0x00538715 0x00538791 0x005387A5 0x005387C6

void NetDaemon::process_message(char * message, unsigned long sender, int channel) {
    Popup popup;
    Filemap filemap;
    // process_message reaches three daemon fields by offset - 0x1B30 is
    // lock_state_[0].dword_[1], named below; 0x1B74 is lock_state_[1].dword_[3]
    // and 0x1BB8 is lock_state_[2].dword_[5], both in parts of the dispatcher
    // this transcription does not reach - and one per-slot byte table at
    // 0x794, AlphaNet's slot_region_[8].

    unsigned char *msg = reinterpret_cast<unsigned char *>(message);
    unsigned short type = *reinterpret_cast<unsigned short *>(message);

    // Special-cased "file transfer" message-type band (0x1500-0x15FF).
    bool skip500 = ((type & 0xf00) != 0x500) || (type == 0x1501);
    if (!skip500 && CommlinkPending != 0 && CommlinkFaction == (int)sender) {
        skip500 = true;
    }
    if (!skip500) {
        if (type == 0x1505) {
            filemap.~Filemap();
            popup.close();
            return;
        }
        message_data(0x1505, (int)sender, 0, 0, 0, 0);
    }

    type = *reinterpret_cast<unsigned short *>(message);

    // First dispatch band: message types 0xF05-0xF0F. Bodies below are a
    // faithful-shape but incomplete reading of what is a much larger
    // (roughly 67-case) dispatcher spanning the rest of the type space;
    // see the TRIED note on the file header.
    if (type <= 0x1101) {
        if (type == 0x1101) {
            // TODO: 0x53624a - not reconstructed (schedule/ready-state update).
            return;
        }
        unsigned int idx = (unsigned int)(type - 0xf05);
        if (idx <= 0xa) {
            switch (idx) {
                case 0:
                    // The raw message IS the faction-data record; the cast is
                    // the image's own, which hands the same pointer along
                    // unchanged.
                    NetWinDialog.setup_faction_data(
                        reinterpret_cast<MessageFactionData *>(message));
                    return;
                case 1:
                    if (NetWinDialog.is_visible()) {
                        NetWinDialog.pick_pass(1);
                    }
                    return;
                case 2:
                case 3:
                case 4:
                case 7:
                case 8:
                case 9:
                case 10: {
                    // Modal-dialog guard: when no popup is currently active
                    // (self+0x1b30 == 0), reject and unwind the locals this
                    // function opened; otherwise the original shows a
                    // reason-specific popup this pass does not reconstruct.
                    if (lock_state_[0].dword_[1] == 0) {
                        filemap.~Filemap();
                        popup.close();
                        return;
                    }
                    return;
                }
                case 5:
                case 6: {
                    int field4 = *reinterpret_cast<int *>(message + 4);
                    int slot = field4 * 103;
                    // 0x794 is AlphaNet's slot_region_[8], one status byte
                    // per process slot.
                    slot_region_[8 + slot * 4] = (idx == 5) ? 1 : 2;
                    return;
                }
            }
        }
    }

    // Default path, reached by the overwhelming majority of message types
    // (the vast majority of jump-table slots point here in the original):
    // validate and re-broadcast the message.
    if ((msg[1] & 0xa0) != 0) {
        int fixResult = fixup_message(message);
        if (fixResult == 0) {
            unsigned short masked = (unsigned short)((type & 0x1fff) | 0x4000);
            *reinterpret_cast<unsigned short *>(message) = masked;
            uint32_t *seq = &lock_state_[0].dword_[0];
            *seq = *seq + 1;
            *reinterpret_cast<int *>(message + 0xc) = *seq;
            send_message(message, sender, 0);
        }
    }

}

// ORIGINAL: 0x0051F1B0 ?check_spock@@YAXXZ 0x0051F1B0-0x0051F1F2 BYTE_EXACT
// size      66 bytes
// prototype void (__cdecl ?check_spock@@YAXXZ)(void)
// callers   9   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0051F0C0
// LEVER: the four gates are named, not bound - 0x93E8C0 is the daemon's own
// lock_state_[0].dword_[1], 0x93E230 is &locks_, and the pair 0x93F670/74
// are the flags synch sets and help_spock clears. Homed 2026-09-04 so the
// wait loops in this file could call it with the image's `E8` instead of a
// forwarder beside a compiled claim.
// 0x0093A934 is base.cpp's UnkGlobal0093A934 (the energy-event flag), and
// 0x0093E230 is &NetDaemonNet.locks_ - the Lock the image dereferences as an
// absolute, and NetDaemonNet.locks_.any_locks() folds to the same load.
void __cdecl help_spock();  // defined below
void __cdecl check_spock() {
    if (NetDaemonNet.lock_state_[0].dword_[1] != 0 &&
        SpockNotifyPending != 0 && UnkGlobal0093A934 == 0 &&
        ConsoleControlTurnActive == 0) {
        SpockHelpActive = 1;
        if (NetDaemonNet.locks_.any_locks() == 0) {
            help_spock();
        }
    }
}

// ORIGINAL: 0x0051F0C0 ?help_spock@@YAXXZ 0x0051F0C0-0x0051F1AA BYTE_EXACT
// size      234 bytes
// prototype void (__cdecl ?help_spock@@YAXXZ)(void)
// callers   1   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0052E00 0x00530320
// Homed 2026-09-04 for the same reason as check_spock below it: its caller
// is in this file, and a forwarder beside the compiled claim is what
// address_index refuses. The four synch/receive sweeps are the session
// refresh the game runs when a turn boundary catches an unnotified client.
void __cdecl help_spock() {
    if (NetDaemonNet.lock_state_[0].dword_[1] != 0) {
        NetDaemonNet.synch(0x25, 0, 0, 0, 0, 1, 0x2101);
        while (NetDaemonNet.receive() != 0) {
        }

        NetDaemonNet.synch(0x12, 0, 0, 0, 0, 1, 0x2101);
        while (NetDaemonNet.receive() != 0) {
        }

        NetDaemonNet.synch(0x26, 0, 0, 0, 0, 1, 0x2101);
        while (NetDaemonNet.receive() != 0) {
        }

        NetDaemonNet.synch(0x14, 0, 0, 0, 0, 1, 0x2101);
        while (NetDaemonNet.receive() != 0) {
        }
    }

    SpockNotifyPending = 0;
    SpockHelpActive = 0;
}

// ORIGINAL: 0x00530360 ?net_tasks@NetDaemon@@QAEXXZ 0x00530360-0x005303A9 BYTE_EXACT
// symbol    ?net_tasks@NetDaemon@@QAEXXZ
// size      74 bytes
// prototype void (__thiscall ?net_tasks@NetDaemon@@QAEXXZ)(NetDaemon* this)
// callers   2   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// One pump turn: drain a message, run the lock check, run spock.
void NetDaemon::net_tasks() {
  wait_task();
  unsigned long sender;
  unsigned long channel;
  int result = NetDaemonNet.Net::get(&sender, &channel);
  if (result != 0) {
    process_message(reinterpret_cast<char *>(result), channel,
                    static_cast<int>(sender));
  }
  locks_.check_global();
  check_spock();
}

// ORIGINAL: 0x00531700 ?maybe_lock@NetDaemon@@QAEHPAHHHHHH@Z 0x00531700-0x00531780 BYTE_EXACT
// size      128 bytes
// prototype int (__thiscall ?maybe_lock@NetDaemon@@QAEHPAHHHHHH@Z)(NetDaemon* this, int*, int, int, int, int, int)
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// 0x1BAC is lock_state_[2].dword_[2] - the lock-held flag unlock() clears -
// 0x1BC4 lock_state_[2].dword_[8] and 0x1BCC lock_state_[2].dword_[10], which
// the archive never mapped. The repeat count compares SIGNED there (the
// image's `jle`, not `jbe`), and that plus naming the field is what took this
// off the archive's MISMATCH.
int NetDaemon::maybe_lock(int *veh_id, int flag, int payload_1C, int payload_20,
                          int payload_18, int extra) {
  if (flag != 0) {
    return 0;
  }
  if (lock_state_[2].dword_[2] != 0) {
    if (lock_state_[2].dword_[8] == 0) {
      ++lock_state_[2].dword_[10];
      // The image compares the count SIGNED here (`jle`), not unsigned.
      if (1 < static_cast<int>(lock_state_[2].dword_[10])) {
        NetMsgPopup->pop("MOVEPENDING", 0, 2, 0);
      }
    }
    return 1;
  }
  lock_state_[2].dword_[10] = 0;
  return lock_veh(veh_id, payload_1C | 4, payload_20, payload_18, extra);
}
