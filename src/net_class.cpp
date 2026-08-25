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
#include <cstring>
#include "net_class.h"
#include "stringstruct.h"
#include "hypothesis_layouts.h"

// 0x00401BE0, recovered in src/leaf_recoveries.cpp. Declared here so
// Net::~Net reaches it with the image's own `call rel32` at 0x004E353A
// instead of an indirect through a forwarder.
extern "C" void __fastcall sub_401be0(void *receiver, void *);

NetFifo::~NetFifo() {  // ??1NetFifo@@QAE@XZ at 0x006339E0 - not this batch's
                        // address.
    typedef void(__fastcall *pending)(NetFifo *, void *);
    reinterpret_cast<pending>(static_cast<unsigned long>(0x006339E0))(this, nullptr);
}

// Vtable shim. VC6 rejects a free `__thiscall` function pointer (C4234), so
// an indirect virtual call through the JackalVoiceRx-shaped container's own
// vtable is spelled by calling the Nth virtual of a class that is never
// defined and never instantiated - only DECLARATION ORDER matters.
class VCallArg {
 public:
  virtual void slot000(int);
  virtual void slot001(void *);
  virtual void slot002(int);
};

static void *const g_006693ac = reinterpret_cast<void *>(0x006693AC);
static void *const g_006693f0 = reinterpret_cast<void *>(0x006693F0);
static void *const g_006693f4 = reinterpret_cast<void *>(0x006693F4);
static void *const g_00669418 = reinterpret_cast<void *>(0x00669418);
static void *const g_0066941c = reinterpret_cast<void *>(0x0066941C);
static void *const g_00669424 = reinterpret_cast<void *>(0x00669424);
static void *const g_0066942c = reinterpret_cast<void *>(0x0066942C);
static void *const g_00669430 = reinterpret_cast<void *>(0x00669430);
static void *const g_0066eaf8 = reinterpret_cast<void *>(0x0066EAF8);
static void *const g_0066eafc = reinterpret_cast<void *>(0x0066EAFC);
static void *const g_0066eb04 = reinterpret_cast<void *>(0x0066EB04);
static void *const g_00670dcc = reinterpret_cast<void *>(0x00670DCC);
static int32_t *const g_009b3374 = reinterpret_cast<int32_t *>(0x009B3374);
static int32_t *const g_009be600 = reinterpret_cast<int32_t *>(0x009BE600);

/*
Purpose: Build a Net - its embedded VoiceTx, three NetFifo message queues,
         and the JackalVoiceRx-shaped container at +0xB0.
// ORIGINAL: 0x0062D6A0 ??0Net@@QAE@XZ 0x0062D6A0-0x0062D8DC;0x006634A0-0x006634DF
// size      635 bytes
// prototype void (__thiscall ??0Net@@QAE@XZ)(Net* this)
// callers   1   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x004C8CC0 0x006339C0
Return Value: this

Net has no base and no virtual anything of its own - a genuine constructor
is safe here (unlike MapWin/PlanWin/Console, whose virtual `GraphicWin`
base makes VC6 double the most-derived flag - see the note in `mapwin.h`),
and the image's own SEH frame around this body is what a real constructor
with real destructible members earns. Net's own layout past `voice_tx_` is
still opaque storage (see the class declaration), so the rest reaches its
sub-objects by raw offset, the same way this tree's other unmodelled
`??0`/`??1` pairs do.
*/
Net::Net() {
    char *const self = reinterpret_cast<char *>(this);

    voice_tx_.VoiceTx::VoiceTx();

    *reinterpret_cast<void **>(self + 0xb4) = g_00670dcc;
    *reinterpret_cast<void **>(self + 0xcc) = g_006693ac;
    *reinterpret_cast<int32_t *>(self + 0xd0) = *g_009b3374;
    *g_009b3374 = 0;

    *reinterpret_cast<void **>(self + 0xb0) = g_0066eafc;
    {
        char *const vtbl = *reinterpret_cast<char **>(self + 0xb4);
        int32_t const adj = *reinterpret_cast<int32_t *>(vtbl + 4);
        *reinterpret_cast<void **>(self + 0xb4 + adj) = g_0066eaf8;
    }
    *reinterpret_cast<int32_t *>(self + 0xb8) = 0;
    *reinterpret_cast<int32_t *>(self + 0xbc) = 0;
    *reinterpret_cast<int32_t *>(self + 0xc0) = 0;
    *reinterpret_cast<int32_t *>(self + 0xc4) = 0;
    *reinterpret_cast<int32_t *>(self + 0xc8) = 0;

    reinterpret_cast<NetFifo *>(self + 0xe8)->NetFifo::NetFifo();
    reinterpret_cast<NetFifo *>(self + 0x10c)->NetFifo::NetFifo();
    reinterpret_cast<NetFifo *>(self + 0x130)->NetFifo::NetFifo();

    *reinterpret_cast<void **>(self + 0x700) = g_00669430;
    *reinterpret_cast<void **>(self + 0x758) = g_006693ac;
    *reinterpret_cast<int32_t *>(self + 0x75c) = *g_009b3374;
    *g_009b3374 = 0;

    *reinterpret_cast<void **>(self + 0x730) = g_00669424;
    *reinterpret_cast<void **>(self + 0x750) = g_006693ac;
    *reinterpret_cast<int32_t *>(self + 0x754) = *g_009b3374;
    *g_009b3374 = 0;

    {
        int32_t const vtbl = *reinterpret_cast<int32_t *>(self + 0x730);
        *reinterpret_cast<void **>(self + 0x72c) = g_006693f4;
        int32_t const adj = *reinterpret_cast<int32_t *>(vtbl + 4);
        *reinterpret_cast<void **>(self + 0x730 + adj) = g_006693f0;
    }

    {
        int32_t const vtbl = *reinterpret_cast<int32_t *>(self + 0x730);
        *reinterpret_cast<int32_t *>(self + 0x734) = 0;
        *reinterpret_cast<int32_t *>(self + 0x738) = 0;
        *reinterpret_cast<int32_t *>(self + 0x73c) = 0;
        *reinterpret_cast<int32_t *>(self + 0x740) = 0;
        *reinterpret_cast<int32_t *>(self + 0x744) = 0;
        *reinterpret_cast<void **>(self + 0x72c) = g_0066941c;
        int32_t const adj = *reinterpret_cast<int32_t *>(vtbl + 4);
        *reinterpret_cast<void **>(self + 0x730 + adj) = g_00669418;
    }

    {
        int32_t const vtbl = *reinterpret_cast<int32_t *>(self + 0x700);
        int32_t const adj = *reinterpret_cast<int32_t *>(vtbl + 4);
        *reinterpret_cast<void **>(self + 0x700 + adj) = g_0066942c;
    }

    *reinterpret_cast<void **>(self) = g_0066eb04;
    *WinNetBuffer = this;
    *reinterpret_cast<int32_t *>(self + 0xdc) = 0;
    *reinterpret_cast<int32_t *>(self + 0xe0) = 0x4e20;
    *g_009be600 = 0;
    *reinterpret_cast<int32_t *>(self + 0x6dc) = 0;
    *reinterpret_cast<int32_t *>(self + 0x760) = 0;
    *reinterpret_cast<int32_t *>(self + 0x764) = 0;
    *reinterpret_cast<int32_t *>(self + 0x6e0) = 0;
    *reinterpret_cast<int32_t *>(self + 0x6e4) = 0;
    *reinterpret_cast<int32_t *>(self + 0x6fc) = 0xc8;

    char *slot = self + 0x158;
    for (int32_t n = 0x10; n != 0; --n) {
        *reinterpret_cast<int32_t *>(slot - 4) = 0;
        *reinterpret_cast<int32_t *>(slot) = 0;
        *reinterpret_cast<int32_t *>(slot + 4) = 0;
        *reinterpret_cast<uint8_t *>(slot + 0x10) = 0;
        *reinterpret_cast<int32_t *>(slot + 0xc) = 0;
        slot += 0x58;
    }

    *reinterpret_cast<int32_t *>(self + 0x6d4) = 0x64;
    *reinterpret_cast<int32_t *>(self + 0xd8) = 0;
    *reinterpret_cast<int32_t *>(self + 0xe4) = 1;
    field_48_ = 0;
    field_4C_ = 0;
    field_50_ = 0;
    field_54_ = 0;
    field_20_ = 0;
    field_24_ = 0;
    field_1C_ = 0;
    *reinterpret_cast<int32_t *>(self + 4) = 0;
    *reinterpret_cast<int32_t *>(self + 8) = 0;
    field_C_ = 0;
    field_10_ = 0;
    field_14_ = 0;
    field_18_ = 0;
    field_28_ = 0;
}

/*
Purpose: Tear down a Net - its embedded container, three NetFifo message
         queues, and its VoiceTx.
// ORIGINAL: 0x004E34D0 ??1Net@@QAE@XZ 0x004E34D0-0x004E365C;0x004E3710-0x004E372B;0x0065C810-0x0065C876
// size      525 bytes
// prototype void (__thiscall ??1Net@@QAE@XZ)(Net* this)
// callers   0   call targets   6
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00401BE0 0x00402970 0x004C8DB0 0x0062E010 0x006339E0
// indirect  0x004E35D5 0x004E35E2 0x004E3602
// LEVER: call_diff said MORE, 8 calls against the image's 7, and all three
//        causes were real defects rather than encoding ones. (1)
//        `voice_tx_.VoiceTx::~VoiceTx()` was written out in the body of a REAL
//        destructor whose `voice_tx_` is a REAL member, so VC6 emitted the
//        destruction TWICE - once here and once after the body, where the
//        image has it. (2) 0x004E3529 `call 0x402970` is
//        StringStruct::remove_all entered on the list at sub1 - 0x24, and (3)
//        0x004E353A `call 0x401be0` is leaf_recoveries.cpp's sub_401be0 on
//        sub1 - 8. Both went through no-argument `NetVectorTeardownHelper*()`
//        forwarders, which is an indirect call the image does not make AND a
//        receiver it does pass. call_diff is now clean.
// TRIED: 12 of 110 even so, and the ceiling is not spelling. THE IMAGE'S
//            BODY IS COMPILER-GENERATED MEMBER TEARDOWN, and this tree writes
//            it out by hand. The image carries EIGHT EH state stores into
//            [ebp - 4] - 5 before `Net::close`, 6 before remove_all, 4 before
//            sub_401be0, then 3, 2, 1 for the three NetFifos, 0 for the
//            container at 0xB0 and -1 at the end - and VC6 emits those ONLY
//            for members it is destroying itself. The order is exactly
//            reverse declaration order, which is the evidence: the objects in
//            the 0x72C..0x750 region go first, then the fifos at 0x130,
//            0x10C, 0xE8, then the 0xB0 container, then `voice_tx_` at 0x58.
//            `field_E8_[0x6C]` is precisely three 0x24-byte NetFifos, so the
//            three explicit `reinterpret_cast<NetFifo *>(self + N)->~NetFifo()`
//            calls in this body are the compiler's job; so is the 0xB0 walk
//            and the 0x72C list. Declaring those members - and dropping the
//            matching explicit construction from Net::Net, which
//            double_construction.py will then flag - is what this body needs,
//            and it is a layout job rather than a spelling one.
Return Value: n/a
*/
Net::~Net() {
    char *const self = reinterpret_cast<char *>(this);

    *reinterpret_cast<void **>(self) = g_0066eb04;
    close();

    // Embedded sub-object ending at self+0x750: its own vtable slot sits at
    // -0x24, and a second, virtual-base vtable is reached through the usual
    // vbtable-delta pattern at -0x20 (read the vbtable pointer, add the
    // delta stored at vbtable[1], write the adjusted vtable there).
    char *const sub1 = self + 0x750;
    *reinterpret_cast<void **>(sub1 - 0x24) = g_0066941c;
    {
        char *const vbase = *reinterpret_cast<char **>(sub1 - 0x20);
        int32_t const delta = *reinterpret_cast<int32_t *>(vbase + 4);
        *reinterpret_cast<void **>(sub1 - 0x20 + delta) = g_00669418;
    }
    // 0x004E3529 `call 0x402970` is StringStruct::remove_all, entered on the
    // list at sub1 - 0x24 - the same object the vtable store above addresses.
    // It used to go through a no-argument `NetVectorTeardownHelper1()`
    // forwarder, which is both an indirect call the image does not make and a
    // receiver the image does pass.
    reinterpret_cast<StringStruct *>(sub1 - 0x24)->remove_all();

    *reinterpret_cast<int32_t *>(sub1 - 0x10) = 0;
    // 0x004E353A `call 0x401be0`, entered on sub1 - 8: the same object again,
    // reached 0x1C in, which is the virtual-base receiver sub_401be0 expects.
    sub_401be0(sub1 - 8, nullptr);

    *reinterpret_cast<void **>(sub1) = g_006693ac;
    *g_009b3374 = *reinterpret_cast<int32_t *>(sub1 + 4);

    *reinterpret_cast<void **>(self + 0x758) = g_006693ac;
    *g_009b3374 = *reinterpret_cast<int32_t *>(self + 0x75c);

    reinterpret_cast<NetFifo *>(self + 0x130)->~NetFifo();
    reinterpret_cast<NetFifo *>(self + 0x10c)->~NetFifo();
    reinterpret_cast<NetFifo *>(self + 0xe8)->~NetFifo();

    // Container at self+0xb0: an intrusive-list collection with its own
    // vtable (element allocator interface at +0) plus a second vtable
    // reached through the same vbtable-delta pattern, this time rooted at
    // self+0xb4.
    char *const vec = self + 0xb0;
    *reinterpret_cast<void **>(vec) = g_0066eafc;
    {
        char *const vbase = *reinterpret_cast<char **>(self + 0xb4);
        int32_t const delta = *reinterpret_cast<int32_t *>(vbase + 4);
        *reinterpret_cast<void **>(self + 0xb4 + delta) = g_0066eaf8;
    }

    if (*reinterpret_cast<char **>(vec + 8) != 0) {
        int32_t const count = *reinterpret_cast<int32_t *>(vec + 0x10);
        if (count > 0) {
            int32_t i = 0;
            do {
                char *const node = *reinterpret_cast<char **>(vec + 8);
                char *const next = *reinterpret_cast<char **>(node + 0xc);
                *reinterpret_cast<char **>(vec + 0xc) = next;
                char *const element = *reinterpret_cast<char **>(node + 8);
                reinterpret_cast<VCallArg *>(vec)->slot001(element);
                if (element != 0) {
                    reinterpret_cast<VCallArg *>(element)->slot002(1);
                }
                *reinterpret_cast<int32_t *>(*reinterpret_cast<char **>(vec + 8) + 8) = 0;
                char *const node2 = *reinterpret_cast<char **>(vec + 8);
                if (node2 != 0) {
                    char *const vbase2 = *reinterpret_cast<char **>(node2);
                    int32_t const delta2 = *reinterpret_cast<int32_t *>(vbase2 + 4);
                    reinterpret_cast<VCallArg *>(node2 + delta2)->slot000(1);
                }
                *reinterpret_cast<char **>(vec + 8) =
                    *reinterpret_cast<char **>(vec + 0xc);
                i++;
            } while (i < count);
        }
        *reinterpret_cast<char **>(vec + 8) = 0;
        *reinterpret_cast<int32_t *>(vec + 0x14) = 0;
        *reinterpret_cast<int32_t *>(vec + 0x10) = 0;
    }
    *reinterpret_cast<int32_t *>(vec + 0x14) = 0;

    *g_009b3374 = *reinterpret_cast<int32_t *>(self + 0xd0);
    *reinterpret_cast<void **>(self + 0xcc) = g_006693ac;

    // NO EXPLICIT `voice_tx_.VoiceTx::~VoiceTx()`. This is a real destructor
    // and `voice_tx_` is a real member, so VC6 emits its destruction after the
    // body on its own - `lea ecx, [esi + 0x58]` / `call 0x4c8db0`, exactly
    // where the image has it. Calling it here as well emitted it TWICE, which
    // is what call_diff reported as MORE (8 calls against the image's 7).
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns 1.
// ORIGINAL: 0x0062ED30 ?poll_players@Net@@QAEHPAUSessionStruct@@@Z 0x0062ED30-0x0062ED38 BYTE_EXACT
// symbol    ?poll_players@Net@@QAEHH@Z
// size      8 bytes
// prototype int (__thiscall ?poll_players@Net@@QAEHPAUSessionStruct@@@Z)(Net* this, SessionStruct*)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 1, always
Status: Complete
*/
int Net::poll_players(int) {
    return 1;
}


/*
Purpose: Find a player's name by key, or nothing.

             lea edx,[ecx+0x154] / cmp [edx],esi / je found / inc eax
             add edx,0x58 / cmp eax,0x10 / jl loop
       found: cmp eax,0x10 / jne have / xor eax,eax / ret 4
        have: lea edx,[eax+eax*4] / lea eax,[eax+edx*2]
              lea eax,[ecx+eax*8+0x169] / ret 4

         Sixteen entries of 0x58 bytes from 0x154, keyed on each entry's first
         dword. The three `lea`s multiply the index by 5, then by 11, then by
         8 - which is 88, the same 0x58 the search strides by - and the name
         sits at 0x169, twenty-one bytes into the entry rather than at its
         head.

         The miss returns NULL, and it is detected by the counter REACHING
         0x10 rather than by the loop simply ending: `cmp eax,0x10` is
         repeated after the loop for exactly that reason.

         Reached by raw offset, as this file's header requires: the layout is
         not established and this function is not evidence enough to establish
         it - it only shows that something 0x58 bytes wide lives at 0x154.

// ORIGINAL: 0x00631A20 ?get_player_name@Net@@QAEPADK@Z 0x00631A20-0x00631A55 BYTE_EXACT
// size      53 bytes
// prototype int8* (__thiscall ?get_player_name@Net@@QAEPADK@Z)(Net* this, unsigned int)
// callers   6   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: a pointer to the name, or nullptr
Status: Complete
*/
char *Net::get_player_name(unsigned long key) {
    uint8_t *const bytes = reinterpret_cast<uint8_t *>(this);
    // A `for`, WITH THE MISS DETECTED AFTER THE LOOP. The image's loop
    // tests only the key match inline (`cmp [edx], esi / je found`); the
    // `index == 0x10` check that decides found-vs-not happens once, AFTER
    // the loop, as its own `cmp eax, 0x10 / jne have`. A loop that returns
    // from inside the loop body folds that second compare away.
    int index;
    for (index = 0; index < 0x10; ++index) {
        uint32_t candidate;
        std::memcpy(&candidate, bytes + 0x154 + index * 0x58,
                    sizeof(candidate));
        if (candidate == key) {
            break;
        }
    }
    if (index == 0x10) {
        return nullptr;
    }
    return reinterpret_cast<char *>(bytes + 0x169 + index * 0x58);
}


/*
Purpose: Give the network layer a slice of time to move voice traffic.
// ORIGINAL: 0x0062D5B0 ?do_net@@YAXXZ 0x0062D5B0-0x0062D5C9 BYTE_EXACT
// size      25 bytes
// callers   12   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
//
// PROMOTED from src/recovered/0062d5b0.cpp, which was byte-exact but never
// compiled - and while it sat there `temp.h` bound this address as a
// `func_msg *const`, so every one of its twelve callers emitted
// `call dword ptr [...]` where the image emits `call rel32`.
Return Value: n/a
Status: Complete
*/
void __cdecl do_net() {
    if (NetCurrent() && *NetEnabled) {
        NetCurrent()->process_voice();
    }
}

/*
Purpose: Let the network layer poll, if there is one and it is enabled.
// ORIGINAL: 0x0062D5D0 ?check_net@@YAXXZ 0x0062D5D0-0x0062D5E9 BYTE_EXACT
// size      25 bytes
// callers   21   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
//
// PROMOTED from src/recovered/0062d5d0.cpp. TWO SEPARATE EARLY RETURNS, not
// one `&&`: that is the shape the artifact carried and the shape the image
// has, and `do_net` above genuinely uses the other one.
Return Value: n/a
Status: Complete
*/
void __cdecl check_net() {
    Net *net = NetCurrent();
    if (net == nullptr) {
        return;
    }
    if (*NetEnabled == 0) {
        return;
    }
    net->check_polling();
}

uint32_t NetGetScratch;  // 0x009BC4BC

/*
Purpose: Poll the embedded NetFifo for a message, when net play is enabled
         and a destination is given.
// ORIGINAL: 0x00630A00 ?get@Net@@QAEHPAKPAK@Z 0x00630A00-0x00630A3A BYTE_EXACT
// symbol    ?get@Net@@QAEHPAK0@Z
// size      58 bytes
// prototype int (__thiscall ?get@Net@@QAEHPAKPAK@Z)(Net* this, unsigned int*, unsigned int*)
// callers   18   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00633F70
Return Value: NetGetScratch's address when a message was fetched, else 0
Status: Complete
*/
int Net::get(unsigned long *a, unsigned long *b) {
    if (*NetEnabled != 0) {
        if (a != 0) {
            NetFifo *const fifo = reinterpret_cast<NetFifo *>(
                reinterpret_cast<char *>(this) + 0x10C);
            int result = fifo->get(&NetGetScratch,
                                   reinterpret_cast<unsigned int *>(a), 0,
                                   reinterpret_cast<unsigned int *>(b));
            return result ? reinterpret_cast<int>(&NetGetScratch) : 0;
        }
    }
    return 0;
}

/*
Purpose: Zero the three head words and initialise the lock the fifo guards.
// ORIGINAL: 0x006339C0 ??0NetFifo@@QAE@XZ 0x006339C0-0x006339DB BYTE_EXACT
// LEVER: PROMOTED out of src/recovered/units/006339c0.cpp, which reached the CRITICAL_SECTION through `(*reinterpret_cast<InitFn *>(g_00669168))(this + 0xc)` - the import slot spelled as a raw address. A plain `InitializeCriticalSection` call compiles to the image's own `call dword ptr [0x669168]`.
// LEVER: layout-corrected the IDB gave NetFifo 0x10 bytes and four uint32_t members. It is 0x24: ??0Net builds three of them at 0xE8, 0x10C and 0x130, spaced 0x24 apart, and this body starts a CRITICAL_SECTION at 0xC, which is 24 bytes. The fourth uint32_t was the head of that lock.
// symbol    ??0NetFifo@@QAE@XZ
// size      27 bytes
// kind      game
Return Value: `this`, which the image leaves in eax
Status: Complete
*/
NetFifo::NetFifo() {
    field_0_ = 0;
    field_4_ = 0;
    field_8_ = 0;
    InitializeCriticalSection(&lock_);
}
