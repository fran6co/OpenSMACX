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

// NetFifo: not a product type elsewhere - AlphaNet's own layout note names
// it (0xB0/0xE8/0x10C/0x130 - see alphanet.h) but nothing carves it out of
// AlphaNet's storage yet. Declared here, private to this file, so
// Net::Net/~Net can reach its real constructor/destructor (0x006339C0 and
// 0x006339E0) with an ordinary call rather than a bound pointer.
class NetFifo {
 public:
  NetFifo();
  ~NetFifo();
};

NetFifo::NetFifo() {  // ??0NetFifo@@QAE@XZ at 0x006339C0 - not this batch's
                       // address.
    typedef void(__fastcall *pending)(NetFifo *, void *);
    reinterpret_cast<pending>(static_cast<unsigned long>(0x006339C0))(this, nullptr);
}

NetFifo::~NetFifo() {  // ??1NetFifo@@QAE@XZ at 0x006339E0 - not this batch's
                        // address.
    typedef void(__fastcall *pending)(NetFifo *, void *);
    reinterpret_cast<pending>(static_cast<unsigned long>(0x006339E0))(this, nullptr);
}

// Two more not-this-batch callees `Net::~Net` reaches directly: teardown
// helpers for the JackalVoiceRx-shaped container at self+0xb0.
extern "C" int __cdecl NetVectorTeardownHelper1() {  // 0x00402970
    typedef int(__cdecl *pending)();
    return reinterpret_cast<pending>(static_cast<unsigned long>(0x00402970))();
}
extern "C" int __cdecl NetVectorTeardownHelper2() {  // 0x00401BE0
    typedef int(__cdecl *pending)();
    return reinterpret_cast<pending>(static_cast<unsigned long>(0x00401BE0))();
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
static Net **const g_009be608 = reinterpret_cast<Net **>(0x009BE608);

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
    *g_009be608 = this;
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
    *reinterpret_cast<int32_t *>(self + 0x48) = 0;
    *reinterpret_cast<int32_t *>(self + 0x4c) = 0;
    *reinterpret_cast<int32_t *>(self + 0x50) = 0;
    *reinterpret_cast<int32_t *>(self + 0x54) = 0;
    *reinterpret_cast<int32_t *>(self + 0x20) = 0;
    *reinterpret_cast<int32_t *>(self + 0x24) = 0;
    *reinterpret_cast<int32_t *>(self + 0x1c) = 0;
    *reinterpret_cast<int32_t *>(self + 4) = 0;
    *reinterpret_cast<int32_t *>(self + 8) = 0;
    *reinterpret_cast<int32_t *>(self + 0xc) = 0;
    *reinterpret_cast<int32_t *>(self + 0x10) = 0;
    *reinterpret_cast<int32_t *>(self + 0x14) = 0;
    *reinterpret_cast<int32_t *>(self + 0x18) = 0;
    *reinterpret_cast<int32_t *>(self + 0x28) = 0;
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
    NetVectorTeardownHelper1();

    *reinterpret_cast<int32_t *>(sub1 - 0x10) = 0;
    NetVectorTeardownHelper2();

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

    voice_tx_.VoiceTx::~VoiceTx();
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

int __fastcall net_poll_players_redirect(Net *self, void *, int a1) {
    return self->poll_players(a1);
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

// ORIGINAL: 0x00631A20 ?get_player_name@Net@@QAEPADK@Z 0x00631A20-0x00631A55
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
    for (uint32_t index = 0; index < 0x10; ++index) {
        uint32_t candidate;
        std::memcpy(&candidate, bytes + 0x154 + index * 0x58,
                    sizeof(candidate));
        if (candidate == key) {
            return reinterpret_cast<char *>(bytes + 0x169 + index * 0x58);
        }
    }
    return nullptr;
}

char *__fastcall net_get_player_name_redirect(
        Net *self, void *, unsigned long key) {
    return self->get_player_name(key);
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
