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

int NetDaemonIsMultiplayerNet;  // 0x0093F660
int NetDaemonLocalFaction;  // 0x00939284

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
// The JackalVoiceRx-shaped container at self+0xb0 - `Net::~Net` (net_class.cpp)
// models the same container's teardown loop explicitly; this destructor
// reaches a genuinely different, COMDAT-shared helper for it instead
// (0x00538EC0, local to this address's own cold region), so it is left a
// shim rather than duplicated as a second copy of that loop.
class JackalVoiceRxShim {
 public:
  ~JackalVoiceRxShim();
};
JackalVoiceRxShim::~JackalVoiceRxShim() {
    typedef void(__fastcall *pending)(JackalVoiceRxShim *, void *);
    reinterpret_cast<pending>(static_cast<unsigned long>(0x00538EC0))(this, nullptr);
}
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

    reinterpret_cast<JackalVoiceRxShim *>(self + 0xb0)->~JackalVoiceRxShim();
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
    uint32_t residue = static_cast<uint32_t>(NetDaemonIsMultiplayerNet);
    if (residue != 0) {
        // `cmp eax, edi` against a zeroed edi: any nonzero flag announces.
        log_say("Client releasing lock", NetDaemonLocalFaction, 0, 0);
        residue = message_data(0x2212, 0, 0, 0, 0, 0);
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


// The daemon's Net, 0x0093CD90 in the image - see netdaemon.h. The image's
// dynamic initializer at 0x0052DB00 constructs it at the fixed address
// before WinMain; the recovered NetDaemon constructor here runs through this
// build's own startup instead.
NetDaemon NetDaemonNet;  // 0x0093CD90
