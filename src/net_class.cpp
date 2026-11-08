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
// RECOVERY_IMPORT: v1 __imp__CreateEventA@16 KERNEL32.dll name:CreateEventA
// The launcher call uses the original loader entry, independently identified by its PE import directory.
// RECOVERY_IMPORT: v1 __imp__WinExec@8 KERNEL32.dll name:WinExec
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00401660 ??0ServiceStruct@@QAE@H@Z 0x00401660-0x004016B8 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00401660.cpp: retained original spans and historical seed interpretation", "Pinned original receiver/table/call sequence reviewed in the Alpha ownership binding decision.", "Current C++ class/lifetime source owns the separately observed emitted symbol; this binding is unclaimed and preserves the independent historical exact seed."], "reason": "Original provider base tables6693D4/6693D0 and five cleared receiver fields; legacy ServiceStruct catalogue name is not payload ownership."}
// symbol    ??0ServiceProviderList@@QAE@XZ
// ORIGINAL: 0x004016C0 ?close@ServiceStruct@@QAEXXZ 0x004016C0-0x00401759 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/recovered/004016c0.cpp: retained original spans and historical seed interpretation", "Pinned original receiver/table/call sequence reviewed in the Alpha ownership binding decision.", "Current C++ class/lifetime source owns the separately observed emitted symbol; this binding is unclaimed and preserves the independent historical exact seed.", "git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/recovered/004016c0.cpp"], "reason": "Receiver virtual-base adjustment -1C, provider-base tables and list walk; no saved-owner publication tail. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
// compile_context src/alphanet.cpp
// kind      game
// symbol    ??1ServiceProviderList@@UAE@XZ
// ORIGINAL: 0x00401760 ?add@ServiceStruct@@QAEHH@Z 0x00401760-0x00401B73 FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:293937ab6f3be75999e0c0a4c9c110c02930624c:src/unrecovered/00401760.cpp: retained original spans and historical seed interpretation", "Pinned original receiver/table/call sequence reviewed in the Alpha ownership binding decision.", "Current C++ class/lifetime source owns the separately observed emitted symbol; this binding is unclaimed and preserves the transferred historical seed findings and complete original spans.", "git:293937ab6f3be75999e0c0a4c9c110c02930624c:src/unrecovered/00401760.cpp", "Historical seed spelling ?add@ServiceStruct@@QAEHH@Z; current source mapping ?add@ServiceStruct@@QAEHH@Z; emitted symbol ?add@ServiceProviderList@@QAEHH@Z."], "reason": "Original allocates entry1C/payload20 and invokes provider hook. Superseded seed retired after owner review; original spans, existing claims and useful seed findings remain in source. This cleanup grants no new match or acceptance claim."}
// TRIED: goto-preserving transcription of the Ghidra decompile (3-tier Heap::get/mem_get fallback, twice per node-type A/B, VCall slot0 insert callback); diverges at insn #7 in the prologue/local register allocation, not in the control-flow shape
// symbol    ?add@ServiceProviderList@@QAEHH@Z
// sub_631050 constructs the AlphaNetLedger whose inline constructor emits this.
// ORIGINAL: 0x00401C80 SessionStruct::SessionStruct 0x00401C80-0x00401CD8 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Retired seed: src/recovered/units/00401c80.cpp: retained original spans and historical seed interpretation", "Pinned original receiver/table/call sequence reviewed in the Alpha ownership binding decision.", "Current C++ class/lifetime source owns the separately observed emitted symbol; this binding is unclaimed and preserves the independent historical exact seed."], "reason": "Session base tables669408/669404 and five cleared circular-list fields."}
// body src/alphanet_inline.h
// symbol    ??0AlphaNetStringStruct@@QAE@XZ
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00402350 sub_402350 0x00402350-0x0040239B FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00402350.cpp: retained original spans and historical seed interpretation", "Pinned original receiver/table/call sequence reviewed in the Alpha ownership binding decision.", "Current C++ class/lifetime source owns the separately observed emitted symbol; this binding is unclaimed and preserves the independent historical exact seed."], "reason": "Receiver-24, calls4023A0, publishes owner, tests deletion bit1 and conditionally frees complete receiver."}
// symbol    ??_GNetUnk72C@@UAEPAXI@Z
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004023A0 sub_4023a0 0x004023A0-0x004024F8;0x00650580-0x00650595 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/004023a0.cpp: retained original spans and historical seed interpretation", "Pinned original receiver/table/call sequence reviewed in the Alpha ownership binding decision.", "Current C++ class/lifetime source owns the separately observed emitted symbol; this binding is unclaimed and preserves the independent historical exact seed."], "reason": "Receiver-24, stages nested-derived66941C/669418, runs removal walk then base6693F4/6693F0; primary plus650580 EH span."}
// symbol    ??1NetUnk72C@@UAE@XZ
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00402A60 ServiceStruct::dt 0x00402A60-0x00402AC6 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00402a60.cpp: retained original spans and historical seed interpretation", "Pinned original receiver/table/call sequence reviewed in the Alpha ownership binding decision.", "Current C++ class/lifetime source owns the separately observed emitted symbol; this binding is unclaimed and preserves the independent historical exact seed."], "reason": "Receiver-1C, stages provider-base tables, clear402970, zero cursor, owner publication then bit1/free."}
// symbol    ??_GServiceProviderList@@UAEPAXI@Z
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00402AD0 sub_402ad0 0x00402AD0-0x00402B36 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00402ad0.cpp: retained original spans and historical seed interpretation", "Pinned original receiver/table/call sequence reviewed in the Alpha ownership binding decision.", "Current C++ class/lifetime source owns the separately observed emitted symbol; this binding is unclaimed and preserves the independent historical exact seed."], "reason": "Receiver-1C, stages nested-base6693F4/6693F0, clear402970, owner publication then bit1/free."}
// symbol    ??_GNetTextListBase@@UAEPAXI@Z
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00402CC0 sub_402cc0 0x00402CC0-0x00402D0B FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00402cc0.cpp: retained original spans and historical seed interpretation", "Pinned original receiver/table/call sequence reviewed in the Alpha ownership binding decision.", "Current C++ class/lifetime source owns the separately observed emitted symbol; this binding is unclaimed and preserves the independent historical exact seed."], "reason": "Receiver-58 then calls402DD0; node owner publication then bit1/free. This anchors402DD0 to outer NetAllocationNode, not nested NetUnk72C."}
// symbol    ??_GNetAllocationNode@@UAEPAXI@Z
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00402D80 sub_402d80 0x00402D80-0x00402DC2 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00402d80.cpp: retained original spans and historical seed interpretation", "Pinned original receiver/table/call sequence reviewed in the Alpha ownership binding decision.", "Current C++ class/lifetime source owns the separately observed emitted symbol; this binding is unclaimed and preserves the independent historical exact seed."], "reason": "Provider add401760 installs payload table6693E4, whose scalar slot points402D80; receiver-18 and saved-owner publication/free. Not unseen NetTextPayload nominal proof."}
// symbol    ??_GServiceStruct@@UAEPAXI@Z
// ORIGINAL: 0x004E3660 sub_4e3660 0x004E3660-0x004E370C FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:293937ab6f3be75999e0c0a4c9c110c02930624c:src/unrecovered/004e3660.cpp: retained original spans and historical seed interpretation", "Pinned original receiver/table/call sequence reviewed in the Alpha ownership binding decision.", "Current C++ class/lifetime source owns the separately observed emitted symbol; this binding is unclaimed and preserves the transferred historical seed findings and complete original spans.", "git:293937ab6f3be75999e0c0a4c9c110c02930624c:src/unrecovered/004e3660.cpp", "Historical seed spelling sub_4e3660; current source mapping sub_4e3660; emitted symbol ??_DJackalVoiceRx@@QAEXXZ."], "reason": "Complete receiver stages66EAFC/66EAF8, walk then allocation-owner publication, no delete flag/free. Already genuine Voice-associated class behavior. Superseded seed retired after owner review; original spans, existing claims and useful seed findings remain in source. This cleanup grants no new match or acceptance claim."}
// TRIED: [ecx+N] reads with no stack args -> a thiscall receiver (Obj4e3660), not the given nullary __cdecl. 0.91 mnemonic sim; remaining diff is the prologue's `push ecx` stack-slot local vs a register-kept loop counter, and one lea/add adjustor-call addressing mode (same shape as 0x0051E480's) - not chased.
// symbol    ??_DJackalVoiceRx@@QAEXXZ
// ORIGINAL: 0x004E38E0 sub_4e38e0 0x004E38E0-0x004E38FB FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:0x004E38E0-0x004E38FB is a complete 27-byte entry ending in RET; it adjusts to the ServiceList virtual base, calls the mapped destructor, and restores the saved allocation owner", "pinned:0x006634FF and 0x0065C630 caller-specific EH paths tail-jump to 0x004E38E0; normal paths call 0x004E3730 and restore the owner's saved virtual-base field", "COFF:src/net_class.cpp.obj and src/alphanet.cpp.obj both emit ??_DServiceList@@QAEXXZ with destructor, StringAllocationBase vftable and StringAllocationHeap relocations", "source:src/net_class.h:ServiceList/ServiceProviderList virtual StringAllocationBase hierarchy; original name remains address-derived sub_4e38e0"], "reason": "Map the complete original ServiceList cleanup entry to the actual generated COFF helper while keeping each caller's distinct EH cleanup in its parent mapping."}
// symbol    ??_DServiceList@@QAEXXZ
// ORIGINAL: 0x004E3730 sub_4e3730 0x004E3730-0x004E3888;0x0065C880-0x0065C895 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:293937ab6f3be75999e0c0a4c9c110c02930624c:src/unrecovered/004e3730.cpp: retained original spans and historical seed interpretation", "Pinned original receiver/table/call sequence reviewed in the Alpha ownership binding decision.", "Current C++ class/lifetime source owns the separately observed emitted symbol; this binding is unclaimed and preserves the transferred historical seed findings and complete original spans.", "git:293937ab6f3be75999e0c0a4c9c110c02930624c:src/unrecovered/004e3730.cpp", "Historical seed spelling sub_4e3730; current source mapping sub_4e3730; emitted symbol ??1ServiceList@@UAE@XZ."], "reason": "Receiver-24 stages provider-derived66EB20/66EB1C, walk then provider base; retained65C880 EH span. Superseded seed retired after owner review; original spans, existing claims and useful seed findings remain in source. This cleanup grants no new match or acceptance claim."}
// compile_context src/alphanet.cpp
// TRIED: same shape as 0x004E3350's `ClassX::sub_4e3350` (own-stage list teardown at vtable 0x66eb20/0x66eb1c, then the same 0x6693d4/0x6693d0 pair the real `?close@ServiceStruct@@QAEXXZ` (0x004016c0, referenced from this function's unwind funclet at 0x0065C880) uses, on the member at `this-0x24`) - no SEH frame modelled, same precedent as 0x004C86D0/0x00538D10 and this function's sibling at 0x004E3350. 0.496 mnemonic similarity, first divergence at #2 (dropped SEH prologue).
// symbol    ??1ServiceList@@UAE@XZ
// ORIGINAL: 0x004E3890 sub_4e3890 0x004E3890-0x004E38DB FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/recovered/units/004e3890.cpp: retained original spans and historical seed interpretation", "Pinned original receiver/table/call sequence reviewed in the Alpha ownership binding decision.", "Current C++ class/lifetime source owns the separately observed emitted symbol; this binding is unclaimed and preserves the independent historical exact seed.", "Genuine declared C++ destructor emits the independently named generated COFF body; original receiver/table/lifetime interpretation and current emitted ABI remain separate from the historical byte obligation."], "reason": "Receiver-24 calls4E3730 then saved-owner publication and bit1/free. Superseded scaffold retired after observing this independently emitted generated body; historical claims and findings transfer without a new matching or linked-preservation claim."}
// compile_context src/alphanet.cpp
// symbol    ??_GServiceList@@UAEPAXI@Z
// ORIGINAL: 0x004E3900 sub_4e3900 0x004E3900-0x004E39CF FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:293937ab6f3be75999e0c0a4c9c110c02930624c:src/unrecovered/004e3900.cpp: retained original spans and historical seed interpretation", "Pinned original receiver/table/call sequence reviewed in the Alpha ownership binding decision.", "Current C++ class/lifetime source owns the separately observed emitted symbol; this binding is unclaimed and preserves the transferred historical seed findings and complete original spans.", "git:293937ab6f3be75999e0c0a4c9c110c02930624c:src/unrecovered/004e3900.cpp", "Historical seed spelling sub_4e3900; current source mapping sub_4e3900; emitted symbol ??_GJackalVoiceRx@@UAEPAXI@Z."], "reason": "Receiver-1C stagesVoiceRx tables, walk, allocation-owner publication and bit1/free. Not a new handwritten body. Superseded seed retired after owner review; original spans, existing claims and useful seed findings remain in source. This cleanup grants no new match or acceptance claim."}
// TRIED: register-save push order (ebx/esi/edi prologue) drifts from a struct-model body; single-level VCall casts used for the three argument-taking slots. 207 vs 197 rebuilt bytes.
// symbol    ??_GJackalVoiceRx@@UAEPAXI@Z
// ORIGINAL: 0x005D4860 sub_5d4860 0x005D4860-0x005D4887 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/recovered/005d4860.cpp: retained original spans and historical seed interpretation", "Pinned original receiver/table/call sequence reviewed in the Alpha ownership binding decision.", "Current C++ class/lifetime source owns the separately observed emitted symbol; this binding is unclaimed and preserves the independent historical exact seed.", "git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/recovered/005d4860.cpp"], "reason": "Provider table slot1 frees/nulls name +4 only when allocator+18 null; retained exact seed. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
// kind      game
// symbol    ?on_remove@ServiceList@@UAEXPAUServiceStruct@@@Z
// ORIGINAL: 0x005D4CC0 sub_5d4cc0 0x005D4CC0-0x005D4CE7 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/recovered/005d4cc0.cpp: retained original spans and historical seed interpretation", "Pinned original receiver/table/call sequence reviewed in the Alpha ownership binding decision.", "Current C++ class/lifetime source owns the separately observed emitted symbol; this binding is unclaimed and preserves the independent historical exact seed.", "git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/recovered/005d4cc0.cpp"], "reason": "Nested list slot1 cleanup at observed payload+4; original factory/complete extent unknown, retain exact seed. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
// kind      game
// symbol    ?on_remove@NetUnk72C@@UAEXPAVNetTextPayload@@@Z
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00630C30 sub_630c30 0x00630C30-0x00630C7C FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00630c30.cpp: retained original spans and historical seed interpretation", "Pinned original receiver/table/call sequence reviewed in the Alpha ownership binding decision.", "Current C++ class/lifetime source owns the separately observed emitted symbol; this binding is unclaimed and preserves the independent historical exact seed."], "reason": "SDK session descriptor fields populate ledger borrowed fields and call401D80; synchronous borrow target."}
// symbol    ?enumerate_session_callback@@YGHPBUDPSESSIONDESC2@@PAKKPAX@Z
#include "stdafx.h"
#include <process.h>
#include <mmsystem.h>
#include <new>
#include <cstring>
#include "net_class.h"
#include "net_session_picker.h"  // the session picker window sub_631050 owns
#include "alphanet.h"
#include "alphanet_inline.h"
#include "popup.h"    // pop_caption / pop_caption_value_title, drop_player's asks
#include "stringstruct.h"
#include "wave.h"
#include "wave_device.h"
#include "sounddevice.h"
#include "general.h"  // mem_get, for the lobby's address and session buffers
#include "strings.h"
#include "game.h"
#include "main.h"  // legacy include; popup.h includes BasePop's hook declaration
#include "font.h"


// The lobby probe's DirectPlay types and import. The image really does import
// DirectPlayLobbyCreateA from DPLAYX.DLL, so the link input belongs beside the
// only translation unit that names it - a `#pragma comment` here rather than a
// `target_link_libraries` entry, which this file cannot reach.
#include <dplobby.h>
#if defined(_MSC_VER) && _MSC_VER <= 1200
#pragma comment(lib, "dplayx.lib")
#endif

// Original Net API zeroing sites call the CRT rather than intrinsic stores.
#if defined(_MSC_VER) && _MSC_VER <= 1200
#pragma function(memset)
#endif

// 0x697548 - the application GUID the lobby session registers under, read out
// four dwords at a time into session.guidInstance. It holds the same
// {02D1B480-D680-11D0-821E-44454F5454} that control_game passes in as `guid`
// and that game.cpp keeps its own copy of, so it is spelled once more here
// rather than shared: game.cpp's is file-local and this one is too.
static const GUID NetLobbyAppGuid = {
    0x02D1B480, 0xD680, 0x11D0, {0x82, 0x1E, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00}};

// Superseded seed deleted; seed paths below are historical Git provenance.


// RECOVERY_OBJECT: v2 0x009BE4E8 0x009BE4EC ?SessionEnumTarget@@3PAVAlphaNetLedger@@A 0x00630C30
// Pinned630C3E reads the DWORD published/cleared at62E4B8/62E4CB.
// This binding covers that observed four-byte pointer cell only.
AlphaNetLedger *SessionEnumTarget;
BOOL __stdcall enumerate_session_callback(LPCDPSESSIONDESC2, LPDWORD, DWORD, LPVOID);
namespace {
// Source-only typed algorithms shared by genuine list owners. They own no
// executable identity. Each instantiated body/reference requires measurement;
// sharing source does not prove the original compiler folded any particular pair.


template<class List> inline int seek_owned_list(List *list, int position) {
    AbsFnT abs_fn = &abs;
    if (position > list->entry_count_ - 1) return 0;
    list->current_ = list->head_;
    if (position < 0) {
        if (abs_fn(position) > list->entry_count_) return 0;
        int remaining = abs_fn(position);
        while (remaining > 0) {
            --remaining;
            list->current_ = list->current_->previous_;
        }
        list->current_position_ = list->entry_count_ + position;
    } else {
        int remaining = position;
        while (remaining > 0) {
            --remaining;
            list->current_ = list->current_->next_;
        }
        list->current_position_ = position;
    }
    return 1;
}

template<class List> inline typename List::Payload *next_owned_list(List *list) {
    if (list->head_ == 0) return 0;
    typename List::Entry *next = list->current_->next_;
    int position = list->current_position_;
    list->current_ = next;
    const int count = list->entry_count_;
    ++position;
    list->current_position_ = position;
    if (position == count) list->current_position_ = 0;
    return list->current_->payload_;
}

}



/*
Purpose: Build a Net - its embedded VoiceTx, three NetFifo message queues,
         and the JackalVoiceRx-shaped container at +0xB0.
// ORIGINAL: 0x0062D6A0 ??0Net@@QAE@XZ 0x0062D6A0-0x0062D8DC;0x006634A0-0x006634DF BYTE_EXACT
// size      635 bytes
// prototype void (__thiscall ??0Net@@QAE@XZ)(Net* this)
// callers   1   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x004C8CC0 0x006339C0
Return Value: this

Net's primary virtual surface is distinct from its members' virtual bases.
The original 0x0062D822 vfptr store belongs to the compiler, as do the member
construction and exception-cleanup stages. Scalar initialization and shared
no-op target bindings remain unfinished; the primary table bound follows the
original derived-table boundary and slot-5 dispatch documented in Net.
*/
// QueryInterface in 0x0062D9E0 writes this slot with IID_IDirectPlay2A
// (original GUID at 0x006717C0); Net initialization/close clear the same
// four-byte slot. The former NetEnabled literal pointed outside this object.
// RECOVERY_OBJECT: v2 0x009BE600 0x009BE604 ?NetDirectPlay@@3PAUIDirectPlay2@@A 0x00630A00
IDirectPlay2 *NetDirectPlay;

// Original Net construction publishes this at 0x0062D828; do_net,
// check_net and Win::on_key load the same four-byte pointer object.
// RECOVERY_OBJECT: v2 0x009BE608 0x009BE60C ?ActiveNet@@3PAVNet@@A 0x0062D5B0
Net *ActiveNet;

// 0x009BE5F0, the dword just below it: poll_services parks the ServiceStruct
// the enumeration callback fills there for the length of the call, and the
// callback reads it back out - a hand-off, not a second list.
// Original 0x0062E279/0x0062E285 publish/clear the list pointer loaded by callback at 0x00630C09; cell only.
// RECOVERY_OBJECT: v1 0x009BE5F0 ?ServiceEnumTarget@@3PAUServiceList@@A
ServiceList *ServiceEnumTarget;  // 0x009BE5F0

// The service-provider GUID the enumeration keeps: DirectPlayEnumerateA hands
// each provider's GUID to sub_630bf0, which compares these 16 bytes against it
// and keeps only the match. Read off the image at 0x00697558.
// RECOVERY_OBJECT: v2 0x00697558 0x00697568 _KeptServiceProviderGuid 0x00630BF0
static const GUID KeptServiceProviderGuid = {
    0xD8D29744, 0x208A, 0x11D0, {0xBC, 0x9D, 0x00, 0xA0, 0x24, 0x29, 0x67, 0xB6}};



// The receiver's no-op insertion/removal hooks have one pointer argument.
// The other embedded list's virtual surface remains unfinished.
// ORIGINAL: 0x00538EC0 ??1JackalVoiceRx@@QAE@XZ 0x00538EC0-0x00538F2E FILE BYTE_EXACT
// kind      game
// symbol    ?clear@JackalVoiceRx@@QAEXXZ
// The catalogue destructor label names this shared list-clearing helper.
// Callers stage receiver/base tables and allocation-owner handoff outside it
// (0x00538E5A..0x00538E9E); the complete receiver destructor is separate
// (0x004E3660). Keep the original identity, not the skeleton's class fiction.
// Insertion 0x00633180 establishes distinct 0x1C circular entries and 0x60
// voice payloads. The three virtual calls below are respectively a receiver
// hook, Sound-family destruction, and entry virtual-base destruction.
// This does not complete Net/AlphaNet/NetDaemon lifetime recovery.
// These list/lifetime definitions retain frame-based original code while the
// protocol and callback definitions in this recovered TU omit the frame. The
// local settings reproduce that distinction without asserting original TU
// boundaries; each empty-option restore returns to command-line optimization.
// `inline` lets /Ob2 expand it in Net::close (0x0062E010) and the receiver's
// generated destructors; MEASURED keeps the out-of-line copy emitted.
// body src/net_class.h


int JackalVoiceRx::on_insert(void *) { return 0; }
void JackalVoiceRx::on_remove(void *) { }

namespace {
void *allocate_voice_record(size_t size, Heap *owner) {
    void *memory;
    if (owner != 0) {
        memory = owner->get(size);
        if (memory == 0) {
            MessageBoxA(NULL, "out of heap space in new function", "Aborting", MB_OK);
            exit(3);
        }
    } else {
        memory = mem_get(size);
        if (memory != 0) StringAllocationHeap = 0;
    }
    if (memory != 0) StringAllocationHeap = owner;
    return memory;
}
}

// Original 633180's two allocations each publish the chosen owner for the
// real StringAllocationBase constructor to capture. Its allocation-failure
// branch keeps the native partial list mutation; do not roll it back.
// ORIGINAL: 0x006336D0 sub_6336d0 0x006336D0-0x00633711 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:01a84efe8fe7feece991db1a99114c60b7342c4f:src/recovered/006336d0.cpp; historical seed/source provenance (seed path is retired from the current working tree)", "pinned:0x670F48 primary table slot +8 at 0x670F50 selects 0x6336D0", "COFF:net_class.cpp.obj defines ??_GVoiceRxAllocation@@UAEPAXI@Z; primary vtable selects its weak-external spelling", "COFF weak-external mode 2 falls back to the class deleting destructor; retained map confirms linked same-address placement", "owner declaration:src/net_class.h:VoiceRxAllocation : VoiceRx, virtual StringAllocationBase", "strict full-span readback reports 64 emitted bytes versus 65 original bytes; this proposal does not claim exactness", "current-owner:src/net_class.cpp:0x006336D0; the live ORIGINAL marker retains name sub_6336d0, complete spans 0x006336D0-0x00633711, historical byte claim retained=True, historical semantic claim retained=False, and the complete parsed LEVER/TRIED/UNRECOVERABLE/DEFERRED lesson set (0/0/0/0); current binding is source navigation/provenance only, not a new claim or match result"], "reason": "The genuine VoiceRxAllocation virtual-base hierarchy emits this primary-table deleting destructor. Preserve the full 65-byte original identity and historical exact claim; current physical emission is 64 bytes, so byte restoration remains outstanding."}
// symbol    ??_GVoiceRxAllocation@@UAEPAXI@Z
// ORIGINAL: 0x00633730 sub_633730 0x00633730-0x00633738 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:01a84efe8fe7feece991db1a99114c60b7342c4f:src/deleting_thunks.cpp; historical source-adapter mapping is superseded by the current owner; the source file may still exist, but this mapped entry is not the current owner", "pinned:0x670F44 entry zero selects 0x633730", "COFF:net_class.cpp.obj vtable ??_7VoiceRxAllocation@@6BStringAllocationBase@@@ selects exact generated symbol ??_EVoiceRxAllocation@@WFI@AEPAXI@Z", "COFF weak tail ??_EVoiceRxAllocation@@UAEPAXI@Z mode 2 falls back to ??_GVoiceRxAllocation@@UAEPAXI@Z; retained map confirms placement", "strict native reference-aware comparison passes complete original span 0x633730-0x633738", "current-owner:src/net_class.cpp:0x00633730; the live ORIGINAL marker retains name sub_633730, complete spans 0x00633730-0x00633738, historical byte claim retained=False, historical semantic claim retained=False, and the complete parsed LEVER/TRIED/UNRECOVERABLE/DEFERRED lesson set (0/0/0/0); current binding is source navigation/provenance only, not a new claim or match result"], "reason": "The compiler emits this StringAllocationBase virtual-base deleting adjustor from the genuine VoiceRxAllocation hierarchy; the old adapter is removed from product source in this proposal. The full eight-byte original extent/reference comparison passes."}
// symbol    ??_EVoiceRxAllocation@@WFI@AEPAXI@Z
// ORIGINAL: 0x00633720 j_VoiceRx__VoiceRx_dt 0x00633720-0x00633725 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned:0x006336D3 in the 0x006336D0 deleting-destructor body is the mapped sole direct call of the five-byte 0x00633720 tail entry", "pinned:0x00633720 transfers to the separately catalogued VoiceRx destruction body at 0x004C8A50", "source:src/net_class.h declares VoiceRxAllocation as a VoiceRx-derived owner with compiler-generated complete destruction", "source:src/net_class.cpp owns the VoiceRxAllocation allocation/deletion lifetime; no free-function thunk is added", "source:src/voice.cpp owns VoiceRx::~VoiceRx and its Sound-base cleanup"], "reason": "The original five-byte entry is a generated complete-destructor tail, not independent cleanup logic. Map its identity to the genuine VoiceRxAllocation complete destructor while retaining the original span and exactness obligation."}
// symbol    ??1VoiceRxAllocation@@UAE@XZ
// LEVER: "return foo();" tail call - matching nullary cdecl signatures let the call fold into the original's bare 5-byte jmp

// ORIGINAL: 0x00633180 sub_633180 0x00633180-0x006336C2;0x006637B0-0x0066386E
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/00633180.cpp"], "reason": "Rehome seed catalogue identity and original extents into compiled owning class; native-emitted symbol is separate and no new byte claim granted."}
// TRIED: __stdcall(int) free function per the literal contract - ecx is read as an object throughout and only one dword is popped (ret 4), so recast as a __thiscall member (VoiceRxOwner::method) taking the real stack int; body transcribes the two heap/mem_get alloc-retry blocks (0x1c node, 0x60 VoiceRx) and list splice, but does not reproduce the original's SEH prologue (no local objects requiring an unwind frame here).
// symbol    ?add_player@JackalVoiceRx@@QAEHH@Z
int JackalVoiceRx::add_player(int player_id) {
    const bool first = count_ == 0;
    Heap *const inherited_owner = allocation_owner_;
    Heap *const node_owner = first && entries_heap_ != 0
                                 ? entries_heap_ : inherited_owner;
    VoiceReceiverEntry *node = static_cast<VoiceReceiverEntry *>(
        allocate_voice_record(sizeof(VoiceReceiverEntry), node_owner));
    if (node == 0) return 4;
    ::new(node) VoiceReceiverEntry;

    if (first) {
        head_ = node;
    } else {
        VoiceReceiverEntry *tail = head_->previous_;
        tail->next_ = node;
        node->previous_ = tail;
        node->next_ = head_;
        head_->previous_ = node;
        cursor_ = node;
    }
    node->id_ = player_id;
    if (first) {
        node->next_ = node;
        node->previous_ = node;
    }

    // The payload independently reloads entries_heap_ on both list branches.
    // In particular, append-node allocation uses inherited_owner above while
    // this VoiceRx still prefers entries_heap_ when it is present.
    Heap *const payload_owner = entries_heap_ != 0
                                    ? entries_heap_ : inherited_owner;
    VoiceRxAllocation *voice = static_cast<VoiceRxAllocation *>(
        allocate_voice_record(sizeof(VoiceRxAllocation), payload_owner));
    if (voice != 0) ::new(voice) VoiceRxAllocation;
    node->payload_ = voice;
    if (node->payload_ == 0) return 4;
    on_insert(voice);
    if (first) cursor_ = head_;
    cursor_index_ = count_;
    ++count_;
    return 0;
}

static void __cdecl net_receive_thread(void *argument);



// The base table's first four entries share ret 8 at 0x00404260; its last
// two share ret at 0x00404280. These are intentional base defaults, not
// missing daemon callback implementations. Net's constructor publishes the
// primary table at 0x0066EB04; AlphaNet's table at 0x0066EACC retains Net's
// player-added/deleted entries. Those original table references establish
// sharing independently of the unrelated catalogue labels at these leaves.
// Each emitted definition requires its own strict proof. This does not prove
// how the old linker shared code, or resolve slot 0's provisional semantics.
// RECOVERY_SHARED: v1 0x00404260 0x00404263 ?on_net_event_0@Net@@UAEXHH@Z 0x0062D6A0
void Net::on_net_event_0(int, int) { }
// RECOVERY_SHARED: v1 0x00404260 0x00404263 ?on_player_added@Net@@UAEXPADK@Z 0x0062D6A0
void Net::on_player_added(char *, unsigned long) { }
// RECOVERY_SHARED: v1 0x00404260 0x00404263 ?on_player_deleted@Net@@UAEXPADK@Z 0x0062D6A0
void Net::on_player_deleted(char *, unsigned long) { }
// RECOVERY_SHARED: v1 0x00404260 0x00404263 ?on_player_name_changed@Net@@UAEXPADK@Z 0x0062D6A0
void Net::on_player_name_changed(char *, unsigned long) { }
// RECOVERY_SHARED: v1 0x00404280 0x00404281 ?on_session_lost@Net@@UAEXXZ 0x0062D6A0
void Net::on_session_lost() { }
// RECOVERY_SHARED: v1 0x00404280 0x00404281 ?on_host_changed@Net@@UAEXXZ 0x0062D6A0
void Net::on_host_changed() { }

// 0x009BE4E8, two slots above ActiveNet: the hand-off dword on_player_added
// parks its list argument in for the length of the EnumSessions scan. The
// callback reads it back out; nothing else in the image writes the slot.
// The `T *const` spelling folds to the bare immediate the image stores.


// The StringStruct selection-sort pass, defined in stringstruct.cpp (extern
// "C" __fastcall); no header declares it, so this file restates the link
// shape the same way designwin.cpp restates slide_show.

// stand-in: local copy of AlphaNetStringStruct::seek_pos whose volatile
// head_/entry_count_ reads force the image's load order (0x0062E4F4/F7) and
// keep the count in EDI across abs(); the header body reloads it after.
static inline int seek_ledger_pos(AlphaNetStringStruct *list, int position) {
    AbsFnT abs_fn = &abs;
    if (position > list->entry_count_ - 1) return 0;
    AlphaNetLedgerEntry *head = *(AlphaNetLedgerEntry *volatile *)&list->head_;
    const int count = *(volatile int *)&list->entry_count_;
    list->current_ = head;
    if (position < 0) {
        if (abs_fn(position) > count) return 0;
        int remaining = abs_fn(position);
        while (remaining > 0) {
            --remaining;
            list->current_ = list->current_->previous_;
        }
        position += list->entry_count_;
    } else {
        int remaining = position;
        while (remaining > 0) {
            --remaining;
            list->current_ = list->current_->next_;
        }
    }
    list->current_position_ = position;
    return 1;
}

/*
Purpose: Run one lobby enumeration for the setup dialog's player-added path:
         guard the handed-in list, scan the session list through DirectPlay's
         EnumSessions (whose callback fills the list through the guard), then
         sort the received list and step its cursor to the last entry.

         The catalogue spells this ?on_player_added@Net@@QAEHPADI@Z, the
         NON-virtual overload - the virtual table slot's void(unsigned long)
         default above is a different, shared body. The parameter names follow
         the body, not the demangler's reading of PADI: the first stack slot
         (the `char *`) is the list the callers build, and the second (the
         `unsigned int`) is the name text pointer, cast at use. Callers push
         the buffer first (0x0063132B, 0x006313A5) and the callee's tail walks
         the first argument as the StringStruct whose fields sit at
         +0x08/+0x0C/+0x10/+0x14.
// ORIGINAL: 0x0062E420 ?on_player_added@Net@@QAEHPADI@Z 0x0062E420-0x0062E536 FILE BYTE_EXACT
// symbol    ?enumerate_sessions@Net@@QAEHPAVAlphaNetLedger@@PBD@Z
// size      278 bytes
// prototype int (__thiscall ?on_player_added@Net@@QAEHPADI@Z)(Net* this, char*, unsigned int)
// callers   4   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D49D0 0x00644F3A 0x006465F0
// indirect  0x0062E4C4
// Homed from src/unrecovered/0062e420.cpp: the signature correction of wave
// event 142 - the image's own QAEXH identity, an overload beside the virtual
// slot, not its replacement.
Return Value: 7 with no DirectPlay session, 0x10 with no list, 1 when the
         enumeration reported a failure, else 0
Status: Coverage transcription - unclaimed body
*/
// stand-in: auto_inline(off) keeps sub_631050's two direct calls (0x0062E420)
// now that it shares this TU; VC6 auto-inlines the first one otherwise (C4711).
#pragma auto_inline(off)
int Net::enumerate_sessions(AlphaNetLedger *ledger, const char *password) {
    if (NetDirectPlay == 0) return 7;
    if (ledger == 0) return 0x10;
    DPSESSIONDESC2 descriptor;
    memset(&descriptor, 0, sizeof(descriptor));
    descriptor.dwSize = sizeof(descriptor);
    descriptor.guidApplication = *guid_6E8_;
    descriptor.lpszPasswordA = password != 0 && *password != 0
        ? const_cast<char *>(password) : 0;
    SessionEnumTarget = ledger;
    const HRESULT result = NetDirectPlay->EnumSessions(
        &descriptor, 0, enumerate_session_callback, 0, 1);
    SessionEnumTarget = 0;
    ledger->sort_sessions();
    if (result != 0) return 1;

    seek_ledger_pos(ledger, -1);
    return 0;
}
#pragma auto_inline(on)

Net::Net() {
    // voice_tx_ (0x58), jackal_voice_rx_ (0xB0), three queues (0xE8..0x14B)
    // and allocation_node_ (0x700) construct implicitly. The node's virtual
    // base at 0x758 is constructed before its list member at 0x72C, whose
    // own allocation base is at 0x750. This preserves the original owner
    // capture order; treating all three as sequential Net members did not.
    // The scalar state and sixteen player slots below are the constructor's
    // direct stores. In particular, the player +0x0C word is not cleared here.
    ActiveNet = this;
    poll_time_DC_ = 0;
    poll_timeout_E0_ = 0x4E20;
    NetDirectPlay = 0;
    field_6DC_ = 0;
    local_player_id_ = 0;
    host_player_id_ = 0;
    buffer_6E0_ = 0;
    field_6E4_ = 0;
    field_6FC_ = 0xC8;
    for (int player = 0; player != 16; ++player) {
        players_[player].field_0_ = 0;
        players_[player].field_4_ = 0;
        players_[player].field_8_ = 0;
        players_[player].flags_14_ = 0;
        players_[player].stamp_10_ = 0;
    }
    field_6D4_ = 0x64;
    voice_flags_D8_ = 0;
    field_E4_ = 1;
    // Original 0x0062D8A3..0x0062D8AC initializes the window's voice inputs.
    voice_hotkey_48_ = 0;
    voice_hotkey_4c_ = 0;
    voice_target_50_ = 0;
    field_54_ = 0;
    thread_info_.owner_ = 0;
    thread_info_.buffer_ = 0;
    thread_info_.direct_play_ = 0;
    thread_info_.thread_id_ = 0;
    thread_info_.events_[0] = 0;
    thread_info_.events_[1] = 0;
    thread_info_.events_[2] = 0;
    thread_info_.events_[3] = 0;
    thread_info_.events_[4] = 0;
    thread_info_.lock_initialized_ = 0;
}

/*
Purpose: Tear down a Net - its embedded container, three NetFifo message
         queues, and its VoiceTx.
// symbol    ??1Net@@QAE@XZ
// size      525 bytes
// prototype void (__thiscall ??1Net@@QAE@XZ)(Net* this)
// callers   0   call targets   6
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00401BE0 0x00402970 0x004C8DB0 0x0062E010 0x006339E0
// indirect  0x004E35D5 0x004E35E2 0x004E3602
Return Value: n/a
*/
/*
Purpose: Close the network: stop the voice transmitter, reset the timers
         and slot state, and - when a DirectPlay session object exists -
         release it through its interface and drain the player slots.
// ORIGINAL: 0x0062E010 ?close@Net@@QAEXXZ 0x0062E010-0x0062E24A BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned PE and independent IDA9/Ghidra extents agree on62E010..62E24A; all570bytes are covered by184entry-reachable instructions ending RET62E249, with no escaping branch.", "The former interval through62E2CF contains6NOPs, independently mapped poll_services62E250..62E299,7NOPs and47bytes of independently mapped join_service62E2A0..62E41C. Neighbor identities/spans and all historical claims remain; former erroneous bounds survive in Git37368691c. No C++ behavior changes."], "reason": "Bound Net::close by its own reachable return; the previous interval consumed two separately mapped methods."}
// symbol    ?close@Net@@QAEXXZ
// size      570 bytes
// kind      game
// The typed body covers both native halves: DirectPlay/worker rendezvous and
// lock teardown, then handle/string/buffer release, slot and receiver reset,
// all queue closes, and the remaining session/voice state clears. Thread
// fields are source-owned by the evidence-backed NetThreadInfo view.
Return Value: n/a
*/
void Net::close() {
    voice_tx_.unload();

    poll_timeout_E0_ = 0x4E20;
    local_player_id_ = 0;
    host_player_id_ = 0;
    poll_time_DC_ = 0;
    field_6DC_ = 0;

    if (NetDirectPlay != 0) {
        EnterCriticalSection(&thread_info_.receive_lock_);
        SetEvent(thread_info_.events_[3]);
        NetDirectPlay->Close();
        NetDirectPlay->Release();
        NetDirectPlay = 0;
        thread_info_.direct_play_ = 0;
        LeaveCriticalSection(&thread_info_.receive_lock_);
        Sleep(0);
        EnterCriticalSection(&thread_info_.receive_lock_);
        LeaveCriticalSection(&thread_info_.receive_lock_);
        thread_info_.owner_ = 0;
        thread_info_.buffer_ = 0;
        thread_info_.thread_id_ = 0;
    }

    if (thread_info_.lock_initialized_ != 0) {
        thread_info_.lock_initialized_ = 0;
        DeleteCriticalSection(&thread_info_.receive_lock_);
    }
    // Five separate closes in the image, not a loop.
    if (thread_info_.events_[0] != 0) {
        CloseHandle(thread_info_.events_[0]);
        thread_info_.events_[0] = 0;
    }
    if (thread_info_.events_[1] != 0) {
        CloseHandle(thread_info_.events_[1]);
        thread_info_.events_[1] = 0;
    }
    if (thread_info_.events_[2] != 0) {
        CloseHandle(thread_info_.events_[2]);
        thread_info_.events_[2] = 0;
    }
    if (thread_info_.events_[3] != 0) {
        CloseHandle(thread_info_.events_[3]);
        thread_info_.events_[3] = 0;
    }
    if (thread_info_.events_[4] != 0) {
        CloseHandle(thread_info_.events_[4]);
        thread_info_.events_[4] = 0;
    }
    if (allocation_node_.session_name_ != 0) {
        free(allocation_node_.session_name_);
        allocation_node_.session_name_ = 0;
    }
    if (allocation_node_.password_ != 0) {
        free(allocation_node_.password_);
        allocation_node_.password_ = 0;
    }
    if (buffer_6E0_ != 0) {
        free(buffer_6E0_);
        buffer_6E0_ = 0;
    }
    field_6E4_ = 0;
    field_6FC_ = 0xC8;
    voice_flags_D8_ = 0;
    for (int player = 0; player != 16; ++player) {
        players_[player].field_0_ = 0;
        players_[player].field_4_ = 0;
        players_[player].field_8_ = 0;
        players_[player].flags_14_ = 0;
    }
    jackal_voice_rx_.clear();
    voice_key_D4_ = 0;
    voice_flags_D8_ = 0;
    queue_e8_.close();
    queue_10c_.close();
    queue_130_.close();
    memset(&service_provider_guid_, 0, sizeof(service_provider_guid_));
    field_E4_ = 1;
    voice_hotkey_48_ = 0;
    voice_hotkey_4c_ = 0;
    voice_target_50_ = 0;
    field_54_ = 0;
}

// Retained unresolved entry points, now reached by the owning classes rather
// than NetDaemon manually destroying its bases. No new target is inferred.




// ORIGINAL: 0x004E34D0 ??1Net@@QAE@XZ 0x004E34D0-0x004E365C;0x0065C810-0x0065C876 BYTE_EXACT
// compile_context src/alphanet.cpp
// kind      game
// symbol    ??1Net@@QAE@XZ
// LEVER: call_diff said MORE, 8 calls against the image's 7, and all three causes were real defects rather than encoding ones. (1) `voice_tx_.VoiceTx::~VoiceTx()` was written out in the body of a REAL destructor whose `voice_tx_` is a REAL member, so VC6 emitted the destruction TWICE - once here and once after the body, where the image has it. (2) 0x004E3529 `call 0x402970` is StringStruct::remove_all entered on the list at sub1 - 0x24, and (3) 0x004E353A `call 0x401be0` is leaf_recoveries.cpp's sub_401be0 on sub1 - 8. Both went through no-argument `NetVectorTeardownHelper*()` forwarders, which is an indirect call the image does not make AND a receiver it does pass. call_diff is now clean.
// TRIED: 12 of 110 even so, and the ceiling is not spelling. THE IMAGE'S BODY IS COMPILER-GENERATED MEMBER TEARDOWN, and this tree writes it out by hand. The image carries EIGHT EH state stores into [ebp - 4] - 5 before `Net::close`, 6 before remove_all, 4 before sub_401be0, then 3, 2, 1 for the three NetFifos, 0 for the container at 0xB0 and -1 at the end - and VC6 emits those ONLY for members it is destroying itself. The order is exactly reverse declaration order, which is the evidence: the objects in the 0x72C..0x750 region go first, then the fifos at 0x130, 0x10C, 0xE8, then the 0xB0 container, then `voice_tx_` at 0x58. `field_E8_[0x6C]` is precisely three 0x24-byte NetFifos, so the three explicit `reinterpret_cast<NetFifo *>(self + N)->~NetFifo()` calls in this body are the compiler's job; so is the 0xB0 walk and the 0x72C list. Declaring those members - and dropping the matching explicit construction from Net::Net, which double_construction.py will then flag - is what this body needs, and it is a layout job rather than a spelling one.
// body src/net_class.h

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

         The current NetPlayer layout proves the searched key at +0 and the
         returned name at +0x15 in each 0x58-byte entry. This method uses the
         existing typed players_ array while retaining the original
         post-loop miss test and first-match behavior.

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
    // A `for`, WITH THE MISS DETECTED AFTER THE LOOP. The image's loop
    // tests only the key match inline (`cmp [edx], esi / je found`); the
    // `index == 0x10` check that decides found-vs-not happens once, AFTER
    // the loop, as its own `cmp eax, 0x10 / jne have`. A loop that returns
    // from inside the loop body folds that second compare away.
    int index;
    for (index = 0; index < 0x10; ++index) {
        if (players_[index].field_0_ == key) {
            break;
        }
    }
    if (index == 0x10) {
        return nullptr;
    }
    return players_[index].name_and_tail_;
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
    if (ActiveNet && NetDirectPlay) {
        ActiveNet->process_voice();
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
    Net *net = ActiveNet;
    if (net == nullptr) {
        return;
    }
    if (NetDirectPlay == 0) {
        return;
    }
    net->check_polling();
}

/*
Purpose: Begin streaming voice over this connection, when net play is live
         and the voice channel is not already starting or running.

// ORIGINAL: 0x0062DF20 ?start_voice@Net@@QAEHK@Z 0x0062DF20-0x0062DF6E BYTE_EXACT
// size      78 bytes
// prototype int (__thiscall ?start_voice@Net@@QAEHK@Z)(Net* this, unsigned int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0062DF58
//
// PROMOTED out of src/recovered/0062df20.cpp (measured byte-exact there,
// never compiled into the build). The VoiceTx subobject at +0x58 carries the
// dispatch: its vtable slot 36 is the voice "start" entry, and the two flag
// words at +0xd4/+0xd8 are the handle and its state bits - 0x60000000 means
// a start or stop already in flight, 0x20000000 is the voice-started bit
// this body sets once slot 36 has been told to go.
// LEVER: slot 36 is VoiceTx's OWN `?start@VoiceTx@@QAEHXZ` (0x004C8EB0, at
//        its table 0x0066E8C4). The flat VoiceTx model could only spell this
//        dispatch as a local 37-slot VoiceCall shim; the real virtual does
//        it, and the claim held unchanged through the swap.
*/
int Net::start_voice(unsigned long key) {
    if (NetDirectPlay == 0) {
        return 7;
    }
    if ((voice_flags_D8_ & 0x60000000) == 0) {
        field_54_ = 0;
        voice_key_D4_ = key;
        voice_tx_.start();
        voice_flags_D8_ |= 0x20000000;
    }
    return 0;
}

/*
Purpose: Stop the voice stream this connection is carrying, if any, and
         tell the peer the channel closed.

// ORIGINAL: 0x0062DFC0 ?stop_voice@Net@@QAEXXZ 0x0062DFC0-0x0062E005 BYTE_EXACT
// size      69 bytes
// prototype void (__thiscall ?stop_voice@Net@@QAEXXZ)(Net* this)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0062F8A0
// indirect  0x0062DFD5
//
// PROMOTED out of src/recovered/units/0062dfc0.cpp. VoiceTx vtable slot 8
// is the voice "stop" entry; `send_packet_type` then announces the close on
// packet type 0x200. 0x0062F8A0 is still unrecovered, so the call goes
// through that member's pending_bodies forwarder - an `E8` to the same
// place the image jumps.
// LEVER: slot 8 is VoiceTx's OWN `?stop@VoiceTx@@QAEHXZ` (0x004C8F00), an
//        override of Sound's stop. Like start_voice, the dispatch held
//        BYTE_EXACT when the VoiceCall shim gave way to `voice_tx_.stop()`.
*/
void Net::stop_voice() {
    if ((voice_flags_D8_ & 0x20000000) != 0) {
        voice_tx_.stop();
        int handle = static_cast<int>(voice_key_D4_);
        uint32_t flags = voice_flags_D8_;
        flags &= 0xDFFFFFFF;
        voice_flags_D8_ = flags;
        send_packet_type(0, 0, handle, 0x200, 0);
    }
}

// Original 0x00630BA1..0x00630BD4 passes this storage and Net+0x6E4's
// capacity to IDirectPlay2::Receive; 0x0062D8F5 sets that capacity to 0x2000.
// Queue readers also copy payload bytes here, and return this buffer's address.
// This bounds the advertised storage independently of the candidate's size;
// the separate global at 0x009BE4BC corroborates, but does not supply, the bound.
// RECOVERY_OBJECT: v2 0x009BC4BC 0x009BE4BC ?NetGetScratch@@3PAEA 0x00630A00
unsigned char NetGetScratch[0x2000];

namespace {
// Shared wire views for the sender and receiver. Only the fields explicitly
// written below are initialized; neither envelope padding nor packet tails
// acquire default values merely because their layout is named.
struct NetPacketEnvelope {
    WORD type_flags_;
    WORD field_2_;
    DWORD sequence_4_;
};
struct NetPeerPlayerRecord {
    DPID id_;
    char name_[0x20];
    DWORD flags_;
};
static_assert(sizeof(NetPacketEnvelope) == 8, "DirectPlay packet header extent");
static_assert(sizeof(NetPeerPlayerRecord) == 0x28, "peer snapshot record stride");
static_assert(offsetof(NetPeerPlayerRecord, flags_) == 0x24, "peer snapshot flags");
}

static int __stdcall net_group_player_callback(unsigned long player_id, unsigned long, const DPNAME *, unsigned long, void *);

// ORIGINAL: 0x0062F390 sub_62f390 0x0062F390-0x0062F89D
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original 0x00631A60 callsite passes (buffer, size, voice_key, 0x100, 0).", "Pinned 0x0062F390 body uses the shared NetPlayerTable, Net FIFO, DirectPlay send interface, events and timeout UI.", "No byte claim is transferred or granted; preserve the native indeterminate pending/retry stack reads."], "reason": "Recover the group sender as a real Net member; pinned process_voice passes data, byte count, destination, packet type and flags in that order."}
// symbol    ?send_group_flagged@Net@@QAEHPAXHJGI@Z
#if defined(_MSC_VER) && _MSC_VER == 1200
#pragma function(memcpy)
#endif
int Net::send_group_flagged(void *data, int size, long target,
                            unsigned short type, unsigned int flags) {
    HANDLE wait_handles[2];
    DWORD sent_at[16];
    int pending[16];
    DWORD retry_delay[16];
    int index;
    wait_handles[0] = thread_info_.events_[0];
    wait_handles[1] = thread_info_.events_[1];

    if (NetDirectPlay == 0 || size < 0 || size > static_cast<int>(field_6E4_)) {
        return 0;
    }
    check_polling();

    if ((type & 0xFFF0) == 0 &&
        ((voice_flags_D8_ & 1) != 0 || (flags & 4) != 0)) {
        if (target == static_cast<long>(local_player_id_)) {
            queue_10c_.add(data, static_cast<unsigned long>(size),
                           local_player_id_, 0, 0xFFFFFFFFUL);
            return static_cast<int>(field_E4_);
        }
        if (target == 0) {
            queue_10c_.add(data, static_cast<unsigned long>(size),
                           local_player_id_, target, 0xFFFFFFFFUL);
            if (static_cast<int>(field_6DC_) < 2)
                return static_cast<int>(field_E4_);
        }
    }

    const int allocation_size = size + 0x0C;
    unsigned char *const packet =
        static_cast<unsigned char *>(mem_get(allocation_size));
    if (packet == 0) return 0;

    NetPacketEnvelope *const header =
        reinterpret_cast<NetPacketEnvelope *>(packet);
    const int reliable = flags & 1;
    if (reliable != 0) {
        header->sequence_4_ = ++field_E4_;
    } else {
        header->sequence_4_ = 0;
    }
    if (data != 0) memcpy(packet + 8, data, static_cast<size_t>(size));

    if (reliable != 0 && (voice_flags_D8_ & 0x10000000U) == 0) {
        // Unconfirmed reliable send. The callback marks only group members
        // whose NetPlayer flags include bit 1. NetPlayerTable is shared
        // callback context, published exactly as native.
        header->type_flags_ = static_cast<WORD>(type | 4);
        for (index = 0; index < 16; ++index) {
            players_[index].field_8_ = 0;
        }
        NetPlayerTable = players_;
        if (NetDirectPlay == 0) {
            free(packet);
            return 0;
        }
        NetDirectPlay->EnumGroupPlayers(static_cast<DPID>(target), 0,
            net_group_player_callback, 0, 0);

        DWORD wait_timeout = 0xFFFFFFFFU;
        for (index = 0; index < 16; ++index) {
            if (players_[index].field_0_ != 0) break;
        }
        if (index == 16) goto done;

        // Original 0x62F06E..0x62F080 initializes only the selected row,
        // although the retry loop later reads every pending/retry slot.
        pending[index] = 1;
        retry_delay[index] = players_[index].field_C_;
        if (retry_delay[index] < wait_timeout) {
            wait_timeout = retry_delay[index];
        }

        while (NetDirectPlay != 0) {
            sent_at[0] = timeGetTime();
            for (index = 1; index < 16; ++index) sent_at[index] = sent_at[0];
            if (NetDirectPlay->Send(local_player_id_, static_cast<DPID>(target),
                                    0, packet,
                                    static_cast<DWORD>(allocation_size)) != DP_OK) {
                free(packet);
                return 0;
            }
            const DWORD interval_start = timeGetTime();
            while (timeGetTime() - interval_start < 20000) {
                ResetEvent(thread_info_.events_[1]);
                ResetEvent(thread_info_.events_[0]);
                if (WaitForMultipleObjects(2, wait_handles, FALSE,
                                           wait_timeout) == WAIT_OBJECT_0 + 1)
                    handle_sys_msg();
                const DWORD now = timeGetTime();
                int still_pending = 0;
                for (index = 0; index < 16; ++index) {
                    if (pending[index] == 0) continue;
                    if (players_[index].field_0_ == 0) {
                        pending[index] = 0;
                    } else if (players_[index].field_8_ == 0) {
                        const DWORD ack_tick = timeGetTime();
                        pending[index] = 0;
                        players_[index].field_C_ = (ack_tick +
                            players_[index].field_C_ - sent_at[index] + 0x14) >> 1;
                    } else {
                        if (now - sent_at[index] > retry_delay[index]) {
                            retry_delay[index] = (retry_delay[index] * 3) >> 1;
                            if (NetDirectPlay == 0) {
                                free(packet);
                                return 0;
                            }
                            sent_at[index] = timeGetTime();
                            if (NetDirectPlay->Send(local_player_id_,
                                    players_[index].field_0_, 0, packet,
                                    static_cast<DWORD>(allocation_size)) != DP_OK) {
                                MessageBoxA(0, "Oh no!! Send failed!",
                                            "Message from the Net Underworld", 0);
                                free(packet);
                                return 0;
                            }
                        }
                        still_pending = 1;
                    }
                }
                if (still_pending == 0) goto done;
            }

            // Timeout notification uses the canonical shared StringTemp.
            StringTemp[0] = 0;
            int first_name = 1;
            for (index = 0; index < 16; ++index) {
                if (players_[index].field_8_ == 0) continue;
                int name_index;
                for (name_index = 0; name_index < 16; ++name_index) {
                    if (players_[name_index].field_0_ == players_[index].field_0_)
                        break;
                }
                // The native not-found path passes null to strcat.
                strcat(StringTemp, name_index == 16
                    ? 0 : players_[name_index].name_and_tail_);
                if (!first_name) {
                    strcat(StringTemp, ", ");
                    strcat(StringTemp, " ");
                }
                first_name = 0;
            }
            strcat(StringTemp, ".");
            parse_says(0, StringTemp, -1, -1);
            switch (pop_caption(const_cast<char *>("jackal"),
                                const_cast<char *>("NET_SENDTIMEDOUT"), 0)) {
            case 0:
                continue;
            case 1:
                close();
                break;
            case 2:
                ExitTurnLoop = 1;
                break;
            }
            free(packet);
            return 0;
        }
        free(packet);
        return 0;
    } else {
        header->type_flags_ = type;
        if (NetDirectPlay == 0) {
            free(packet);
            return 0;
        }
        if (NetDirectPlay->Send(local_player_id_, static_cast<DPID>(target),
                                reliable != 0, packet,
                                static_cast<DWORD>(allocation_size)) != DP_OK) {
            free(packet);
            return 0;
        }
    }
done:
    free(packet);
    return static_cast<int>(field_E4_);
}

// 0x009BE5FC, four dwords below ActiveNet: the outgoing host_player_id_ the host
// change publishes before the new host's id overwrites it.
// RECOVERY_OBJECT: v2 0x009BE5FC 0x009BE600 ?NetHostChangeSlot@@3IA 0x00632350
uint32_t NetHostChangeSlot;
// RECOVERY_OBJECT: v2 0x009BE60C 0x009BE610 ?NetReapBusy@@3IA 0x006320E0
uint32_t NetReapBusy;
// RECOVERY_OBJECT: v2 0x009BE610 0x009BE614 ?NetKeepAliveBusy@@3IA 0x006320E0
#if defined(_MSC_VER) && _MSC_VER == 1200
#pragma intrinsic(memcpy)
#endif
uint32_t NetKeepAliveBusy;

// The voice receiver loses one player's entry: walk from the cursor, unlink,
// hand the payload to the removal hook, delete it and the node. Inlined by
// Net::drop_player (0x00632350).
inline void JackalVoiceRx::remove_id(unsigned long id) {
    if (head_ != 0) {
        for (int scanned = 0; scanned < count_; ++scanned) {
            if (cursor_->id_ == id) {
                cursor_->next_->previous_ = cursor_->previous_;
                cursor_->previous_->next_ = cursor_->next_;
                VoiceReceiverEntry *const entry = cursor_;
                if (entry == head_) {
                    head_ = entry->next_;
                }
                cursor_ = entry->next_;
                Sound *payload = entry->payload_;
                on_remove(payload);
                delete payload;
                entry->payload_ = 0;
                delete entry;
                --count_;
                break;
            }
            cursor_ = cursor_->next_;
        }
        if (count_ == 0) {
            head_ = 0;
        }
        cursor_index_ = count_ - 1;
    }
}

/*
Purpose: Cut one player out of the session by id, or everyone when the caller
         hands 0xFFFFFFFF. The interactive half (reason 0) asks first: the
         dropped side is named through parse_says - both hosts when the timer
         words disagree and the target is not the host slot, the target alone
         otherwise - and a "jackal" popup answers; the dismissal answer can
         instead open NET_ADJUSTTOLERANCE and re-arm the staleness threshold
         to five, ten, twenty or thirty seconds. The cut itself closes the
         player's slot (later records shift down, the top slot's data left
         duplicated behind them), republishes the host id and raises
         on_host_changed when it was the polling target, counts 0x6DC down,
         raises on_player_deleted with the kept name, and unlinkes the
         player's voice-receiver entry - payload deleted, node freed through
         its StringAllocationBase. A reason-0 cut then announces itself with
         a mode-4 0x80-byte packet whose body is the frame slot the image
         passes through unwritten.
// ORIGINAL: 0x00632350 ?drop_player@Net@@QAEHKH@Z 0x00632350-0x00632716 FILE
// size      966 bytes
// prototype int (__thiscall ?drop_player@Net@@QAEHKH@Z)(Net* this, unsigned int, int)
// callers   2   call targets   5
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00625EC0 0x00627160 0x00627200 0x0062F8A0 0x00645470
// indirect  0x00632612 0x00632633 0x006326A3 0x006326B1 0x006326CC
// Homed from src/unrecovered/00632350.cpp; this body replaces the in-file
// forwarder that stood here for check_polling.
Return Value: 1 when a player was found (cut, or the abort answer taken), 0
         when the id named nobody
Status: Coverage transcription - unclaimed body
*/
int Net::drop_player(unsigned long key, int reason) {
    int index = 0;
    while (index < 0x10 && key != players_[index].field_0_) {
        ++index;
    }
    if (index == 0x10) {
        return 0;
    }

    if (reason == 0) {
        if (local_player_id_ != host_player_id_ && local_player_id_ != players_[1].field_0_) {
            // The two hosts are named - the grammar slots 0 and 1 - and the
            // plural question asked.
            parse_says(0, get_player_name(players_[0].field_0_), -1, -1);
            parse_says(1, get_player_name(players_[1].field_0_), -1, -1);
            int answer = pop_caption_value_title(
                const_cast<char *>("jackal"),
                const_cast<char *>("NET_OKTODROPHOSTS"), 1, 0, 0);
            switch (answer) {
            case 0:
                ExitTurnLoop = 1;
                return 1;
            case 1:
                return 0;
            case 2:
                switch (pop_caption(const_cast<char *>("jackal"),
                                    const_cast<char *>("NET_ADJUSTTOLERANCE"), 0)) {
                case 0:
                    poll_timeout_E0_ = 5000;
                    return 0;
                case 1:
                    poll_timeout_E0_ = 10000;
                    return 0;
                case 2:
                    poll_timeout_E0_ = 20000;
                    return 0;
                case 3:
                    poll_timeout_E0_ = 30000;
                    break;
                }
                return 0;
            }
            return 0;
        }
        // Dropping the paired host or client: the caption names which side.
        parse_says(0, get_player_name(key), -1, -1);
        char *caption = local_player_id_ == host_player_id_
            ? const_cast<char *>("NET_OKTODROPCLIENT")
            : const_cast<char *>("NET_OKTODROPHOST");
        int answer = pop_caption_value_title(
            const_cast<char *>("jackal"), caption, 1, 0, 0);
        switch (answer) {
        case 1:
            return 0;
        case 2:
            switch (pop_caption(const_cast<char *>("jackal"),
                                const_cast<char *>("NET_ADJUSTTOLERANCE"), 0)) {
            case 0:
                poll_timeout_E0_ = 5000;
                return 0;
            case 1:
                poll_timeout_E0_ = 10000;
                return 0;
            case 2:
                poll_timeout_E0_ = 20000;
                return 0;
            case 3:
                poll_timeout_E0_ = 30000;
                break;
            }
            return 0;
        }
        // Any other answer (the confirming 0) falls through to the cut.
    }

    // The removal. The name rides the frame for the on_player_deleted notice,
    // emptied up front the way the image does when the id has already gone.
    char name[0x100];
    name[0] = 0;
    // 63259C repeats the ID search after the modal callbacks. The original
    // presence check's index is not the row subsequently copied/shifted.
    for (index = 0; index < 0x10; ++index) {
        if (players_[index].field_0_ == key) {
            strcat(name, players_[index].name_and_tail_);
            // Close the slot: every later record shifts down one. The image
            // does not clear the top slot, so its data stays duplicated behind.
            for (int moved = index; moved < 0xF; ++moved) {
                players_[moved] = players_[moved + 1];
            }
            break;
        }
    }

    if (key == host_player_id_) {
        NetHostChangeSlot = host_player_id_;
        host_player_id_ = players_[0].field_0_;
        on_host_changed();
    }

    --field_6DC_;
    on_player_deleted(name, key);

    // The voice receiver loses this player's entry: walk from its cursor,
    // unlink, hand the departing payload to the removal hook, delete it through
    // Sound's own deleting destructor, and free the node through its virtual
    // base - the vbase-adjusted call the image makes.
    jackal_voice_rx_.remove_id(key);

    if (reason == 0) {
        // 6326F4's stack address is the original first argument, not a fresh
        // uninitialized local: send the removed player ID as four bytes.
        send_packet_type(&key, 4, 0, 0x80, 1);
    }
    return 1;
}

/*
Purpose: Keep the session alive and reap the players that have gone quiet:
         every two seconds send a 0x20 keep-alive packet, then - if the voice
         flag says this connection tracks its peers at all - walk the player
         table and cut whoever has not been heard from inside poll_timeout_E0_
         milliseconds.

         Three shapes of staleness, in the image's order:
           1. the two timer words agree            - sweep all sixteen slots;
           2. the first agrees with player 1's ID  - reap player 0;
           3. neither                              - look at players 0 and 1,
             and if both are stale, cut every player at once.

         The image captures the imported clock after the DirectPlay guard,
         keeps that pointer for the keep-alive and first two reap arms, then
         reloads the import in the neither-host arm. Its first timeout check
         compares now - 2000 with the stored tick as unsigned values; preserve
         that wraparound behavior rather than rewriting it as an elapsed-time
         subtraction.
// ORIGINAL: 0x006320E0 ?check_polling@Net@@QAEXXZ 0x006320E0-0x00632290
// RECOVERY_BINDING: v1 {"reason":"The one-player reaping arm compares the local player ID with row 1's ID, not row 1's timestamp. Preserve the original row-0 timestamp/drop logic, distinct timeout comparisons, failure returns and busy-flag effects.","evidence":["pinned:0x0063216E loads Net+0x760 into EAX; 0x006321E2 compares EAX with Net+0x1AC","source:Net players_ begins at +0x154; NetPlayer stride is 0x58, field_0_ is the ID at +0 and stamp_10_ is the timestamp at +0x10, so row 1 ID is +0x1AC and row 1 stamp is +0x1BC","pinned:0x006321EA reads row 0 stamp at +0x164; 0x00632206 loads row 0 ID at +0x154 for drop_player and 0x0063221A refreshes the timestamp after successful drop"]}
// symbol    ?check_polling@Net@@QAEXXZ
// size      432 bytes
// prototype void (__thiscall ?check_polling@Net@@QAEXXZ)(Net* this)
// callers   4   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x0062F8A0 0x00632350
// indirect  0x006320FA 0x00632138 0x006321A1 0x006321BF 0x006321F8 0x0063221A 0x00632244 0x0063226E
// LEVER: preserve the original busy-flag effects: neither-host scan exits
//        at 0x632242/0x632254 jump directly to 0x63228A, bypassing the
//        shared clear at 0x632280. A zero or fresh stamp therefore leaves
//        NetReapBusy set on these two returns. Other cleanup paths remain
//        separate; do not extend their clear to these original early exits.
//        Keep this asymmetry when spelling the reap control flow.
// Historical spelling below crosses NetPlayer subobjects; the row-owner
// correction supersedes it, but its compiler observations remain obligations.
// LEVER: branch 3 walks `stamps[checked * 0x16]` from `&players_->stamp_10_`,
//        NOT `players_[checked].stamp_10_`: the image spills the BASE pointer
//        to [esp+0x10] before the loop and writes `*stamps` through it after
//        the drop, which only a pointer local produces. players_[1] is stored
//        directly beside it - two different base expressions, one value.
// TRIED: calling the slot through the accessor at each site, no local - VC6
//        then never hoists the load into a callee-saved register, gives the
//        function no local slot at all (`push ecx` is missing, so the whole
//        register assignment cascades) and spends ebx on a zero register
//        (`cmp reg,ebx` / `push ebx` where the image has `test` / `push 0`);
//        4 of 135, and compiles 126 instructions against the image's 135.
// TRIED: spelling the four staleness tests against 0 in the inverted form,
//        dropping the pointer local for `players_[checked]`, and hoisting the
//        clock into a `register` local - none of them removes the zero
//        register, and the pointer-local removal brings back a frame pointer
//        (`push ebp / mov ebp,esp`), which the image has none of.
// Historical residual analysis predates the corrected row-1 ID discriminator.
// THE RESIDUAL. MISMATCH 11 of 135, and it is one register-assignment cascade
// plus one dead-store difference, not a transcription error. The image loads
// the clock's import slot into EBP once, AFTER the NetDirectPlay test, and never
// stores it to its home; this tree loads it before the test and VC6 keeps a
// home copy (`mov dword ptr [esp+0x10], ebp`) for a local the image does not
// have. Everything else follows: with EBP spent on the clock the image cannot
// also spend EBX on a zero register, so it tests and pushes immediates where
// this tree compares against and pushes the zero register. Six attempts, and
// the trigger for VC6's hoist-without-home is not in this file's spelling.
Return Value: n/a
*/
void Net::check_polling() {
    if (NetDirectPlay == 0) {
        return;
    }
    if (timeGetTime() - 2000U <= poll_time_DC_) {
        return;
    }
    if (NetKeepAliveBusy != 0) {
        return;
    }
    NetKeepAliveBusy = 1;
    send_packet_type(0, 0, 0, 0x20, 0);
    poll_time_DC_ = timeGetTime();
    NetKeepAliveBusy = 0;
    if (NetReapBusy != 0) {
        return;
    }
    NetReapBusy = 1;
    if ((voice_flags_D8_ & 0x40000) != 0) {
        if (local_player_id_ == host_player_id_) {
            // Every slot: a live player other than this one whose stamp is
            // older than the timeout is cut; a successful cut restamps it.
            NetPlayer *player = players_;
            for (int count = 0x10; count != 0; --count, ++player) {
                if (player->field_0_ != 0 && player->field_0_ != local_player_id_) {
                    const DWORD stamp = player->stamp_10_;
                    if (stamp != 0 && timeGetTime() - stamp > poll_timeout_E0_
                        && drop_player(player->field_0_, 0) == 0) {
                        player->stamp_10_ = timeGetTime();
                    }
                }
            }
            NetReapBusy = 0;
            return;
        }
        if (local_player_id_ == players_[1].field_0_) {
            const DWORD stamp = players_[0].stamp_10_;
            if (stamp != 0 && timeGetTime() - stamp > poll_timeout_E0_
                && drop_player(players_[0].field_0_, 0) == 0) {
                players_[0].stamp_10_ = timeGetTime();
                NetReapBusy = 0;
                return;
            }
        } else {
            // Neither leading row is this player: both stale cuts the lot.
            uint32_t *const stamps = &players_[0].stamp_10_;
            uint32_t *stamp = stamps;
            for (int checked = 0; checked < 2; ++checked, stamp += 0x16) {
                const DWORD value = *stamp;
                if (value == 0) {
                    return;
                }
                if (timeGetTime() - value < poll_timeout_E0_) {
                    return;
                }
            }
            if (drop_player(0xFFFFFFFF, 0) == 0) {
                const DWORD cut = timeGetTime();
                players_[1].stamp_10_ = cut;
                *stamps = cut;
            }
        }
    }
    NetReapBusy = 0;
}

/*
Purpose: Move one slice of voice traffic: ride the level meter to open and
         close the channel, and when it is open, forward whatever the
         transmitter's capture buffer holds to the session.

             test 0x40000000      -> the channel is closing
               level <  50        -> clear the closing bit
             level > 100          -> set the closing bit and stop
             else                 -> ship the capture buffer

         The two thresholds are UNSIGNED compares in the image (`jae` against
         0x32, `jbe` against 0x64) over a getter the catalogue spells `H`
         (int), so the level is held in an unsigned local rather than compared
         as the getter's own type.
// ORIGINAL: 0x00631A60 ?process_voice@Net@@QAEXXZ 0x00631A60-0x00631B2E BYTE_EXACT
// symbol    ?process_voice@Net@@QAEXXZ
// size      206 bytes
// prototype void (__thiscall ?process_voice@Net@@QAEXXZ)(Net* this)
// callers   0   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005FD2B0 0x0062F390 0x0062F8A0
// indirect  0x00631A7F 0x00631AAB 0x00631AC8 0x00631AD3 0x00631B0A 0x00631B24
// HISTORICAL LEVER (2026-09-03): an intermediate `voice_sound_call`
//   forwarder over the then-inline `do_sound` restored the original direct
//   call and yielded 78/78 BYTE_EXACT for this body. The pinned caller scan
//   now supports a real out-of-line hook, so this body calls it directly.
Return Value: n/a
*/
void Net::process_voice() {
    do_sound();
    if ((voice_flags_D8_ & 0x40000000) != 0) {
        unsigned int level = voice_tx_.get_nbuffers();
        if (level < 0x32) {
            voice_flags_D8_ &= ~0x40000000u;
        }
    } else {
        unsigned int level = voice_tx_.get_nbuffers();
        if (level > 100) {
            voice_flags_D8_ |= 0x40000000;
            voice_tx_.stop();
            return;
        }
        _MMIOINFO *packet = voice_tx_.get_next_buffer();
        if (packet != 0) {
            if (field_54_ != 0) {
                send_group_flagged(packet->pchBuffer,
                           static_cast<int>(packet->pchEndRead - packet->pchBuffer),
                           voice_key_D4_, 0x100, 0);
            } else {
                send_packet_type(packet->pchBuffer,
                                 static_cast<int>(packet->pchEndRead - packet->pchBuffer),
                                 voice_key_D4_, 0x100, 0);
            }
            voice_tx_.return_buffer(packet);
        }
    }
}

// ORIGINAL: 0x0062EE20 sub_62ee20 0x0062EE20-0x0062EE71 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned62EE20..62EE71; flags bit2 and field8 on sixteen 58-byte NetPlayer records; shared object9BE4BC.", "git:b0d788d3e0ebc4d5b4c20acb834cd37455e53c67:src/recovered/units/0062ee20.cpp"], "reason": "Recover the SDK group enumeration callback against the same source-owned NetPlayerTable used by player enumeration; mark genuine matching NetPlayer records."}
// symbol    ?net_group_player_callback@@YGHKKPBUDPNAME@@KPAX@Z
static int __stdcall net_group_player_callback(unsigned long player_id,
                                               unsigned long,
                                               const DPNAME *, unsigned long,
                                               void *) {
    if (!NetPlayerTable) return 0;
    for (int index = 0; index < 16; ++index) {
        if (NetPlayerTable[index].field_0_ == player_id) {
            if (NetPlayerTable[index].flags_14_ & 2) NetPlayerTable[index].field_8_ = 1;
            break;
        }
    }
    return 1;
}

// Unreferenced in the pinned image (no call, jump or data reference).
// Original effect: return without modifying EAX; callee stack cleanup 0 bytes.
// ORIGINAL: 0x0062EE80 nullsub_261 0x0062EE80-0x0062EE81 BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason": "Bind the unreferenced original leaf as an empty/constant source function in the translation unit whose address band contains it; its original name, prototype and callers remain unknown.", "evidence": ["pinned:terranx_original.exe:0x0062EE80-0x0062EE81 return without modifying EAX; callee stack cleanup 0 bytes", "pinned scan: no rel32 call/jmp and no dword reference to this address anywhere in the image", "address band: nearest mapped neighbours are 0x0062EE20 sub_62ee20 (src/net_class.cpp) and 0x0062EEA0 ?send_group@Net@@QAEHPAXHJH@Z (src/net_class.cpp)"]}
// symbol    ?nullsub_261@@YAXXZ
// kind      game
void __cdecl nullsub_261() {
}

// Unreferenced in the pinned image (no call, jump or data reference).
// Original effect: return without modifying EAX; callee stack cleanup 0 bytes.
// ORIGINAL: 0x0062EE90 nullsub_262 0x0062EE90-0x0062EE91 BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason": "Bind the unreferenced original leaf as an empty/constant source function in the translation unit whose address band contains it; its original name, prototype and callers remain unknown.", "evidence": ["pinned:terranx_original.exe:0x0062EE90-0x0062EE91 return without modifying EAX; callee stack cleanup 0 bytes", "pinned scan: no rel32 call/jmp and no dword reference to this address anywhere in the image", "address band: nearest mapped neighbours are 0x0062EE20 sub_62ee20 (src/net_class.cpp) and 0x0062EEA0 ?send_group@Net@@QAEHPAXHJH@Z (src/net_class.cpp)"]}
// symbol    ?nullsub_262@@YAXXZ
// kind      game
void __cdecl nullsub_262() {
}

// ORIGINAL: 0x0062ED40 ?create_group@Net@@QAEHPAD@Z 0x0062ED40-0x0062EDA2 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned62ED40..62EDA2; git:b0d788d3e0ebc4d5b4c20acb834cd37455e53c67:src/leaf_recoveries.cpp supplies the former facade mapping and unchanged SDK call contract."], "reason": "Rehome the DirectPlay group constructor from its raw receiver facade to the genuine Net method and SDK IDirectPlay2 interface."}
int Net::create_group(char *group_name) {
    if (NetDirectPlay == 0) return 0;
    DPNAME name;
    DPID group;
    memset(&name, 0, sizeof(name));
    name.dwSize = sizeof(name);
    name.lpszLongNameA = group_name;
    const HRESULT result = NetDirectPlay->CreateGroup(&group, &name, 0, 0, 0);
    return result == 0 ? static_cast<int>(group) : 0;
}

// Group callback evidence: NetPlayer.field_8_ is the acknowledgement flag
// at +8; field_C_ is the per-peer retry tick at +0xC. Existing exact 0x58
// stride/16-slot extent is independently established by Net and player APIs.
// ORIGINAL: 0x0062EEA0 ?send_group@Net@@QAEHPAXHJH@Z 0x0062EEA0-0x0062F38C
// RECOVERY_BINDING: v1 {"evidence": ["Pinned62EEA0..62F38C; retained git:b0d788d3e0ebc4d5b4c20acb834cd37455e53c67:src/unrecovered/0062eea0.cpp; genuine NetPlayer/FIFO/DirectPlay owners.", "Native62F038..62F044 selects the first nonzero player DPID, not the group callback acknowledgement flag. Native62F06E..62F080 writes only that pending/retry slot;62F14A..62F199 later reads all sixteen entries. Native62F0A2..62F0B9 initializes every base-time slot from one tick."], "reason": "Recover the original group send and retry operation through genuine Net state and the SDK, preserving its untouched pending/retry stack entries and separate full base-time initialization. The deterministic all-peer repair remains in Git history for the later improvement milestone."}
#if defined(_MSC_VER) && _MSC_VER == 1200
#pragma function(memcpy)
#endif
int Net::send_group(void *data, int size, long group_id, int flags) {
    HANDLE wait_handles[2];
    DWORD sent_at[16];
    int pending[16];
    DWORD retry_delay[16];
    int index;
    wait_handles[0] = thread_info_.events_[0];
    wait_handles[1] = thread_info_.events_[1];

    if (NetDirectPlay == 0 || size < 0 || size > static_cast<int>(field_6E4_)) {
        return 0;
    }
    check_polling();

    if ((voice_flags_D8_ & 1) != 0 || (flags & 4) != 0) {
        if (group_id == static_cast<long>(local_player_id_)) {
            queue_10c_.add(data, static_cast<unsigned long>(size),
                           local_player_id_, 0, 0xFFFFFFFFUL);
            return static_cast<int>(field_E4_);
        }
        if (group_id == 0) {
            queue_10c_.add(data, static_cast<unsigned long>(size),
                           local_player_id_, group_id, 0xFFFFFFFFUL);
            if (static_cast<int>(field_6DC_) < 2)
                return static_cast<int>(field_E4_);
        }
    }

    const int allocation_size = size + 0x0C;
    unsigned char *const packet =
        static_cast<unsigned char *>(mem_get(allocation_size));
    if (packet == 0) return 0;

    NetPacketEnvelope *const header =
        reinterpret_cast<NetPacketEnvelope *>(packet);
    const int reliable = flags & 1;
    if (reliable != 0) {
        header->sequence_4_ = ++field_E4_;
    } else {
        header->sequence_4_ = 0;
    }
    if (data != 0) memcpy(packet + 8, data, static_cast<size_t>(size));

    if (reliable != 0 && (voice_flags_D8_ & 0x10000000U) == 0) {
        // Unconfirmed reliable send. The callback marks only group members
        // whose NetPlayer flags include bit 1. NetPlayerTable is shared
        // callback context, published exactly as native.
        header->type_flags_ = 4;
        for (index = 0; index < 16; ++index) {
            players_[index].field_8_ = 0;
        }
        NetPlayerTable = players_;
        if (NetDirectPlay == 0) {
            free(packet);
            return 0;
        }
        NetDirectPlay->EnumGroupPlayers(static_cast<DPID>(group_id), 0,
            net_group_player_callback, 0, 0);

        DWORD wait_timeout = 0xFFFFFFFFU;
        for (index = 0; index < 16; ++index) {
            if (players_[index].field_0_ != 0) break;
        }
        if (index == 16) {
            free(packet);
            return static_cast<int>(field_E4_);
        }

        // Original 0x62F06E..0x62F080 initializes only the selected row,
        // although the retry loop later reads every pending/retry slot.
        pending[index] = 1;
        retry_delay[index] = players_[index].field_C_;
        if (retry_delay[index] < wait_timeout) {
            wait_timeout = retry_delay[index];
        }

        while (NetDirectPlay != 0) {
            sent_at[0] = timeGetTime();
            for (index = 1; index < 16; ++index) sent_at[index] = sent_at[0];
            if (NetDirectPlay->Send(local_player_id_, static_cast<DPID>(group_id),
                                    0, packet,
                                    static_cast<DWORD>(allocation_size)) != DP_OK) {
                free(packet);
                return 0;
            }
            const DWORD interval_start = timeGetTime();
            while (timeGetTime() - interval_start < 20000) {
                ResetEvent(thread_info_.events_[1]);
                ResetEvent(thread_info_.events_[0]);
                if (WaitForMultipleObjects(2, wait_handles, FALSE,
                                           wait_timeout) == WAIT_OBJECT_0 + 1)
                    handle_sys_msg();
                const DWORD now = timeGetTime();
                int still_pending = 0;
                for (index = 0; index < 16; ++index) {
                    if (pending[index] == 0) continue;
                    if (players_[index].field_0_ == 0) {
                        pending[index] = 0;
                    } else if (players_[index].field_8_ == 0) {
                        const DWORD ack_tick = timeGetTime();
                        pending[index] = 0;
                        players_[index].field_C_ = (ack_tick +
                            players_[index].field_C_ - sent_at[index] + 0x14) >> 1;
                    } else {
                        if (now - sent_at[index] > retry_delay[index]) {
                            retry_delay[index] = (retry_delay[index] * 3) >> 1;
                            if (NetDirectPlay == 0) {
                                free(packet);
                                return 0;
                            }
                            sent_at[index] = timeGetTime();
                            if (NetDirectPlay->Send(local_player_id_,
                                    players_[index].field_0_, 0, packet,
                                    static_cast<DWORD>(allocation_size)) != DP_OK) {
                                MessageBoxA(0, "Oh no!! Send failed!",
                                            "Message from the Net Underworld", 0);
                                free(packet);
                                return 0;
                            }
                        }
                        still_pending = 1;
                    }
                }
                if (still_pending == 0) goto done;
            }

            // Timeout notification uses the canonical shared StringTemp.
            StringTemp[0] = 0;
            int first_name = 1;
            for (index = 0; index < 16; ++index) {
                if (players_[index].field_8_ == 0) continue;
                int name_index;
                for (name_index = 0; name_index < 16; ++name_index) {
                    if (players_[name_index].field_0_ == players_[index].field_0_)
                        break;
                }
                // The native not-found path passes null to strcat.
                strcat(StringTemp, name_index == 16
                    ? 0 : players_[name_index].name_and_tail_);
                if (!first_name) {
                    strcat(StringTemp, ", ");
                    strcat(StringTemp, " ");
                }
                first_name = 0;
            }
            strcat(StringTemp, ".");
            parse_says(0, StringTemp, -1, -1);
            switch (pop_caption(const_cast<char *>("jackal"),
                                const_cast<char *>("NET_SENDTIMEDOUT"), 0)) {
            case 0:
                continue;
            case 1:
                close();
                break;
            case 2:
                ExitTurnLoop = 1;
                break;
            }
            free(packet);
            return 0;
        }
        free(packet);
        return 0;
    } else {
        header->type_flags_ = 1;
        if (NetDirectPlay == 0) {
            free(packet);
            return 0;
        }
        if (NetDirectPlay->Send(local_player_id_, static_cast<DPID>(group_id),
                                reliable != 0, packet,
                                static_cast<DWORD>(allocation_size)) != DP_OK) {
            free(packet);
            return 0;
        }
    }
done:
    free(packet);
    return static_cast<int>(field_E4_);
}
#if defined(_MSC_VER) && _MSC_VER == 1200
#pragma intrinsic(memcpy)
#endif

/*
Purpose: Poll the embedded NetFifo for a message, when net play is enabled
         and a destination is given.
// ORIGINAL: 0x00630A00 ?get@Net@@QAEHPAKPAK@Z 0x00630A00-0x00630A3A BYTE_EXACT
// symbol    ?get@Net@@QAEPAXPAK0@Z
// size      58 bytes
// prototype int (__thiscall ?get@Net@@QAEHPAKPAK@Z)(Net* this, unsigned int*, unsigned int*)
// callers   18   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00633F70
Return Value: NetGetScratch's address when a message was fetched, else 0
Status: Complete
*/
void *Net::get(unsigned long *a, unsigned long *b) {
    if (NetDirectPlay != 0) {
        if (a != 0) {
            int result = queue_10c_.get(NetGetScratch,
                                   reinterpret_cast<unsigned int *>(a), 0,
                                   reinterpret_cast<unsigned int *>(b));
            return result ? NetGetScratch : 0;
        }
    }
    return 0;
}

// The player-table write seam, 0x009BC4B8 - the dword just below NetGetScratch
// - and the table base it indexes, 0x009BE4BC just above the scratch's end.
// enum_players appends one 0x58-byte NetPlayer record per DirectPlay player
// callback through this pair; init neither reads nor writes either slot in its
// recovered body, so they stay independent bindings, not Net member aliases.
// Source-owned callback selection cells, not pointers into the old image.
// 630C80 loads/reloads both around GetPlayerData; its increment is a DWORD
// at 9BC4B8, immediately before the independently owned NetGetScratch bank.
// RECOVERY_OBJECT: v2 0x009BC4B8 0x009BC4BC ?NetPlayerWriteIndex@@3IA 0x00630C80
unsigned int NetPlayerWriteIndex;
// RECOVERY_OBJECT: v2 0x009BE4BC 0x009BE4C0 ?NetPlayerTable@@3PAVNetPlayer@@A 0x00630C80
NetPlayer *NetPlayerTable;

/*
Purpose: Append one player to the callback-selected table: record the original
         DirectPlay id, read one byte of remote player data through the SDK
         GetPlayerData method (slot +0x50), merge in the connection's voice
         bit, copy the DPNAME long text, and publish the id as the polling
         target when the player is local.
// ORIGINAL: 0x00630C80 ?enum_players@Net@@QAEHKKPBUDPNAME@@KPAX@Z 0x00630C80-0x00630D6B FILE BYTE_EXACT
// size      227 bytes
// prototype int (__thiscall ?enum_players@Net@@QAEHKKPBUDPNAME@@KPAX@Z)(Net* this, unsigned long, unsigned long, const DPNAME*, unsigned long, void*)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645460
// indirect  0x00630CC5
// Homed from src/unrecovered/00630c80.cpp.
Return Value: 1, always
Status: Coverage transcription - unclaimed body
*/
int Net::enum_players(unsigned long player_id, unsigned long, const DPNAME *name,
                      unsigned long, void *) {
    NetPlayerTable[NetPlayerWriteIndex].field_0_ = player_id;

    // Native630C8B..CB6 supplies DPGET_REMOTE (zero) as the fifth stack
    // argument including the interface receiver. The one-byte data lands in
    // an ordinary local that VC6 places in the dead player_id slot; the DWORD
    // initialized to one is buffer capacity, not the returned flags.
    DWORD data;
    DWORD data_size = 1;
    NetDirectPlay->GetPlayerData(player_id, &data, &data_size, 0);

    NetPlayerTable[NetPlayerWriteIndex].flags_14_ = static_cast<uint8_t>(data);

    if (ActiveNet != 0 && (ActiveNet->voice_flags_D8_ & 2) != 0) {
        NetPlayerTable[NetPlayerWriteIndex].flags_14_ = static_cast<uint8_t>(
            NetPlayerTable[NetPlayerWriteIndex].flags_14_ | 2);
    }

    NetPlayerTable[NetPlayerWriteIndex].name_and_tail_[0] = 0;
    // The image's build is ANSI: the byte string the callback hands in sits
    // in the DPNAME's A-side union slot, and the record's name text is
    // appended straight from it.
    strcat(NetPlayerTable[NetPlayerWriteIndex].name_and_tail_,
           name->lpszLongNameA);

    if ((data & 1) != 0 && ActiveNet != 0) {
        ActiveNet->host_player_id_ = player_id;
    }

    ++NetPlayerWriteIndex;
    return 1;
}

/*
Purpose: Initialise the network session: tear whatever is running down, take
         the 0x2000-byte DirectPlay buffer, the session GUID and the option
         word, ask the embedded voice transmitter to open its 0x1000-byte
         device buffer, clear the sixteen player slots, and register this
         object - its thread, its five events and its queue lock - as live.
// ORIGINAL: 0x0062D8E0 ?init@Net@@QAEHPAU_GUID@@HH@Z 0x0062D8E0-0x0062D9D8 BYTE_EXACT
// size      248 bytes
// prototype int (__thiscall ?init@Net@@QAEHPAU_GUID@@HH@Z)(Net* this, _GUID*, int, int)
// callers   2   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4510 0x0062E010
// indirect  0x0062D948 0x0062D983 0x0062D996 0x0062D99F 0x0062D9A8 0x0062D9B1 0x0062D9BA 0x0062D9C3
Return Value: 4 when the buffer cannot be had, else 0
Status: Complete
*/
int Net::init(_GUID *guid, int option_a, int option_b) {
    close();
    field_44_ = option_b;
    field_6E4_ = 0x2000;
    void *buffer = mem_get(0x2000);
    buffer_6E0_ = buffer;
    if (buffer == 0) {
        return 4;
    }
    guid_6E8_ = guid;
    voice_flags_D8_ = option_a;
    field_6D8_ = 0xC8;
    // VoiceTx::init, slot 0x84 of the table at voice_tx_'s front.
    voice_tx_.init(0x1000, 0, 0);
    // Preserve the pinned store sequence for every 0x58-byte record:
    // dwords +0/+4/+8, byte +0x14 (flags), then dword +0x10 (stamp).
    // field_C_ and all name bytes are untouched by this initialization.
    uintptr_t player_cursor = reinterpret_cast<uintptr_t>(
        &players_[0].field_4_);
    int count = 0x10;
    do {
        *reinterpret_cast<uint32_t *>(
            player_cursor - offsetof(NetPlayer, field_4_)
                + offsetof(NetPlayer, field_0_)) = 0;
        *reinterpret_cast<uint32_t *>(player_cursor) = 0;
        *reinterpret_cast<uint32_t *>(
            player_cursor - offsetof(NetPlayer, field_4_)
                + offsetof(NetPlayer, field_8_)) = 0;
        *reinterpret_cast<uint8_t *>(
            player_cursor - offsetof(NetPlayer, field_4_)
                + offsetof(NetPlayer, flags_14_)) = 0;
        *reinterpret_cast<uint32_t *>(
            player_cursor - offsetof(NetPlayer, field_4_)
                + offsetof(NetPlayer, stamp_10_)) = 0;
        player_cursor += sizeof(NetPlayer);
        --count;
    } while (count != 0);
    thread_info_.owner_ = this;
    thread_info_.buffer_ = buffer_6E0_;
    thread_info_.direct_play_ = NetDirectPlay;
    thread_info_.thread_id_ = GetCurrentThreadId();
    thread_info_.events_[0] = CreateEventA(0, 0, 0, 0);
    thread_info_.events_[1] = CreateEventA(0, 0, 0, 0);
    thread_info_.events_[2] = CreateEventA(0, 0, 0, 0);
    thread_info_.events_[3] = CreateEventA(0, 0, 0, 0);
    thread_info_.events_[4] = CreateEventA(0, 0, 0, 0);
    InitializeCriticalSection(&thread_info_.receive_lock_);
    thread_info_.lock_initialized_ = 1;
    return 0;
}

/*
Purpose: Probe the DirectPlay lobby for a connection the command line asked
         for, and hand it to the session layer when there is one.

         `/net h` hosts, `/net j` joins, `/net n <name>` names the player and
         `/net tcp <address>` names the host to dial. Everything below the
         option walk is IDirectPlayLobbyA: DirectPlayLobbyCreateA builds the
         lobby, SetConnectionSettings hands it a DPLCONNECTION (an embedded
         DPSESSIONDESC2 plus a DPNAME), GetConnectionSettings reads it back
         sized to DPERR_BUFFERTOOSMALL, and Connect brings up IDirectPlay2A -
         which lands in the 0x9BE600 word every other Net entry point tests as
         the "net play is live" flag.

// ORIGINAL: 0x0062D9E0 ?check_for_lobby@Net@@QAEHPADPAU_GUID@@HH@Z 0x0062D9E0-0x0062DF16
// symbol    ?check_for_lobby@Net@@QAEHPADPAU_GUID@@HH@Z
// size      1334 bytes
// prototype int (__thiscall ?check_for_lobby@Net@@QAEHPADPAU_GUID@@HH@Z)(Net* this, char*, _GUID*, int, int)
// callers   1   call targets   11
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4510 0x0062D8E0 0x0062E010 0x0062E540 0x00644EF2 0x006453E0 0x00645470 0x00645DD0 0x006468D0 0x006474A0 0x00650208
// indirect  0x0062DCAB 0x0062DD24 0x0062DD67 0x0062DDB9 0x0062DDCA 0x0062DE13 0x0062DE46 0x0062DE5B 0x0062DE73 0x0062DE91 0x0062DEBC
//
// The 0x6717C0/0x6717D0/0x6717E0 the image pushes are dxguid's
// IID_IDirectPlay2A / DPAID_INet / DPSPGUID_TCPIP, and 0x697548 is its own
// copy of the application GUID control_game passes in as `guid`. 0x697568 and
// 0x69756c are the literals "tcp" and "0123456789". DirectPlayLobbyCreateA is
// a real DPLAYX.DLL import, so this file carries the
// `#pragma comment(lib, "dplayx.lib")` that names it - the one link input this
// tree cannot reach from CMakeLists.txt.
//
// QueryInterface fills the shared NetDirectPlay pointer with IDirectPlay2A.
// Other Net entry points test that same pointer, rather than a separate flag
// or a read from the original executable's fixed address.
//
// LEVER: the option walk is a `switch`, not an if/else chain - and that is not
//        a style choice. The switch is what makes VC6 promote the case value
//        through `movsx` and emit the image's own `sub eax,0x68 / sub eax,2 /
//        sub eax,4` chain; the if/else spelling gave `mov al,[esi+1] /
//        cmp al,0x68`. The switch ALSO flips VC6's register assignment so
//        `this` lands in EDI instead of ESI, which is what frees ESI for the
//        address buffer and drops the frame back to the image's 0x298.
// LEVER: `player_name` and `net_address` are BOTH 0x100 - the two buffers sit
//        at 0xA8 and 0x1A8 and the frame total is 0x298, so the first guess of
//        0xF0 for the second was wrong by exactly the padding it seemed to
//        explain.
// LEVER: the lobby-address local and the session buffer are TWO variables, not
//        one reused. As one long-lived `address` VC6 gave it a stack slot and
//        every struct offset shifted by 4; split, each fits the register the
//        image uses.
// LEVER: the strings "tcp"/"0123456789", the DP* structs out of <dplobby.h>
//        and the two `strchr(app_name, '/')` calls - the image really does
//        call it twice with the same arguments, the first result discarded.
// TRIED: an if/else chain instead of the switch - `this` moved to ESI, the
//        address buffer to the stack, frame 0x28C, 12 of 432 agreeing.
// TRIED: declaring `host_mode`/`lobby`/`address` before the `init` call - the
//        prologue then zeroed them before the call, where the image zeroes
//        them after; moved after and the first 16 instructions agree.
// TRIED: `char *address` declared without an initialiser and zeroed at the
//        DirectPlayLobbyCreateA call - 16 of 432, worse.
// TRIED: `session`/`name`/`connection` assignment order in all five
//        permutations (name,session,conn / sess,name,conn / conn,sess,name /
//        sess,conn,name / guidInstance first) - 20 of 432 for every one. The
//        interleaved store order the image shows is VC6's scheduler over the
//        hoisted GUID loads, not source order.
// TRIED: dropping the `goto`/`fail:` label and nesting the five late failure
//        conditions - 20 of 432, unchanged. The merge is not caused by the
//        label.
// TRIED: `dwFlags != DPOPEN_JOIN` for the session's status test - the image
//        compares with 2, so it is `!= DPOPEN_CREATE`.
//
// THE RESIDUAL. MISMATCH 20 of 432, and it is two things, not a transcription
// error:
//  1. VC6 merges this body's three BARE `return 0`s into one block at the tail
//     (each is then a 6-byte `jne`), where the image writes all eight
//     epilogues out inline and branches over them with a 2-byte `je`. Probed
//     against four synthetic bodies - a cdecl free function, a __thiscall
//     member with a 0x200 frame, a 4-callee-saved-register member with a
//     switch and eight returns - and VC6 replicated the epilogues in every
//     one, so the trigger is in this body's shape and not in the flag set.
//     Everything after instruction 16 then aligns at a fixed +8 offset.
//  2. The 25 DPSESSIONDESC2/DPNAME/DPLCONNECTION field stores are emitted in a
//     different permutation (every offset otherwise identical). See the TRIED
//     line above: not reachable from assignment order.
Return Value: 1 joined, 2 hosting, 0 on any failure or no lobby request
Status: Homed from src/unrecovered/0062d9e0.cpp. MISMATCH - see THE RESIDUAL.
*/
int Net::check_for_lobby(char *app_name, _GUID *guid, int option_a,
                         int option_b) {
    if (init(guid, option_a, option_b) != 0) {
        return 0;
    }

    int host_mode = 0;
    IDirectPlayLobby *lobby = 0;
    unsigned long address_size;
    IDirectPlay2 *direct_play;
    DPLCONNECTION connection;
    DPNAME name;
    DPSESSIONDESC2 session;
    char net_address[0x100];
    char player_name[0x100];
    char *cursor;
    char *buffer;
    int status;

    if (app_name != 0) {
        field_44_ = option_b;
        if (strchr(app_name, '/') != 0) {
            cursor = strchr(app_name, '/');
            do {
                switch (*++cursor) {
                case 'h':
                    host_mode = 1;
                    break;
                case 'j':
                    host_mode = 0;
                    break;
                case 'n':
                    ++cursor;
                    char *end = strchr(cursor, ' ');
                    player_name[0] = '\0';
                    if (end != 0) {
                        *end = '\0';
                        strcat(player_name, cursor);
                        *end = ' ';
                    } else {
                        strcat(player_name, cursor);
                    }
                    break;
                }
                cursor = strchr(cursor, '/');
            } while (cursor != 0);

            if (host_mode == 0) {
                char *tcp = strstr(app_name, "tcp");
                if (tcp == 0) {
                    goto have_lobby;
                }
                tcp += strcspn(tcp, "0123456789");
                char *end = strchr(tcp, ' ');
                net_address[0] = '\0';
                if (end != 0) {
                    *end = '\0';
                    strcat(net_address, tcp);
                    *end = ' ';
                } else {
                    strcat(net_address, tcp);
                }
            }

            char *address = 0;
            DirectPlayLobbyCreateA(0, &lobby, 0, 0, 0);

            name.dwSize = sizeof(DPNAME);
            name.dwFlags = 0;
            name.lpszShortNameA = player_name;
            name.lpszLongNameA = player_name;

            session.dwSize = sizeof(DPSESSIONDESC2);
            session.dwFlags = DPSESSION_MIGRATEHOST;
            session.guidInstance = NetLobbyAppGuid;
            session.guidApplication = *guid;
            session.dwMaxPlayers = 7;
            session.dwCurrentPlayers = 0;
            session.lpszSessionNameA = 0;
            session.lpszPasswordA = 0;
            session.dwReserved1 = 0;
            session.dwReserved2 = 0;
            session.dwUser1 = option_b;
            session.dwUser2 = 0;
            session.dwUser3 = 0;
            session.dwUser4 = 0;

            connection.dwSize = sizeof(DPLCONNECTION);
            connection.dwFlags = host_mode != 0 ? DPOPEN_CREATE : DPOPEN_JOIN;
            connection.lpSessionDesc = &session;
            connection.lpPlayerName = &name;
            connection.guidSP = DPSPGUID_TCPIP;

            if (host_mode == 0) {
                if (lobby->CreateAddress(DPSPGUID_TCPIP, DPAID_INet,
                                         net_address, strlen(net_address) + 1,
                                         0, &address_size)
                    != DPERR_BUFFERTOOSMALL) {
                    return 0;
                }
                address = static_cast<char *>(mem_get(address_size));
                if (address == 0) {
                    close();
                    return 0;
                }
                if (lobby->CreateAddress(DPSPGUID_TCPIP, DPAID_INet,
                                         net_address, strlen(net_address) + 1,
                                         address, &address_size)
                    != 0) {
                    close();
                    return 0;
                }
                connection.lpAddress = address;
                connection.dwAddressSize = address_size;
            } else {
                connection.lpAddress = 0;
                connection.dwAddressSize = 0;
            }

            lobby->SetConnectionSettings(0, 0, &connection);
            if (address != 0) {
                free(address);
            }
        }
    }

have_lobby:
    if (lobby == 0) {
        if (DirectPlayLobbyCreateA(0, &lobby, 0, 0, 0) != 0) {
            close();
            return 0;
        }
    }

    if (lobby->GetConnectionSettings(0, 0, &address_size)
        != DPERR_BUFFERTOOSMALL) {
        lobby->Release();
        close();
        return 0;
    }

    buffer = static_cast<char *>(mem_get(address_size));
    if (buffer == 0) {
        return 0;
    }

    if (lobby->GetConnectionSettings(0, buffer, &address_size) == 0) {
        status = reinterpret_cast<DPLCONNECTION *>(buffer)->dwFlags
                     != DPOPEN_CREATE
                     ? DPOPEN_CREATE
                     : DPOPEN_JOIN;
        if (reinterpret_cast<DPLCONNECTION *>(buffer)->lpSessionDesc != 0) {
            reinterpret_cast<DPLCONNECTION *>(buffer)->lpSessionDesc->dwFlags =
                DPSESSION_MIGRATEHOST;
            reinterpret_cast<DPLCONNECTION *>(buffer)->lpSessionDesc
                ->dwMaxPlayers = 7;
            if (lobby->SetConnectionSettings(
                    0, 0, reinterpret_cast<DPLCONNECTION *>(buffer)) == 0) {
                if (lobby->Connect(0, &direct_play, 0) == 0) {
                    if (direct_play->QueryInterface(IID_IDirectPlay2A,
                                                    reinterpret_cast<void **>(&NetDirectPlay))
                        == 0) {
                        thread_info_.direct_play_ = NetDirectPlay;
                        direct_play->Release();
                        direct_play = 0;
                        if (status == 1) {
                            voice_flags_D8_ |= 0x10000;
                        } else {
                            voice_flags_D8_ |= 0x20000;
                        }
                        join_session(
                            0,
                            reinterpret_cast<DPLCONNECTION *>(buffer)
                                ->lpPlayerName->lpszShortNameA,
                            0);
                        free(buffer);
                        return status;
                    }
                }
            }
        }
    }

    if (lobby != 0) {
        lobby->Release();
    }
    free(buffer);
    close();
    return 0;
}

// ---------------------------------------------------------------------------
// NetDirectPlay is populated by QueryInterface(IID_IDirectPlay2A). The SDK
// identifies the original +0x20/+0x0C/+0x1C slots as DestroyGroup,
// AddPlayerToGroup and DeletePlayerFromGroup. Capture the interface once,
// as in the original; retain each wrapper's distinct null/result contract.
// ---------------------------------------------------------------------------

// ORIGINAL: 0x0062EDB0 ?delete_group@Net@@QAEXH@Z 0x0062EDB0-0x0062EDC7 BYTE_EXACT
// size      24 bytes
// prototype void (__thiscall ?delete_group@Net@@QAEXH@Z)(Net* this, int)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
void Net::delete_group(int group) {
  IDirectPlay2 *const direct_play = NetDirectPlay;
  if (direct_play != 0) {
    direct_play->DestroyGroup(static_cast<DPID>(group));
  }
}

// ORIGINAL: 0x0062EDD0 ?add_to_group@Net@@QAEHJK@Z 0x0062EDD0-0x0062EDFA BYTE_EXACT
// size      43 bytes
// prototype int (__thiscall ?add_to_group@Net@@QAEHJK@Z)(Net* this, long, unsigned long)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
int Net::add_to_group(long group, unsigned long member) {
  IDirectPlay2 *const direct_play = NetDirectPlay;
  if (direct_play == 0) {
    return 7;
  }
  const HRESULT result = direct_play->AddPlayerToGroup(
      static_cast<DPID>(group), static_cast<DPID>(member));
  return result != 0;
}

// ORIGINAL: 0x0062EE00 sub_62ee00 0x0062EE00-0x0062EE1C BYTE_EXACT
// symbol    ?sub_62ee00@@YGHHH@Z
// size      29 bytes
// prototype int (__stdcall sub_62ee00)(int, int)
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// The catalogue has no name for this one, so the tree keeps the address name
// and the free __stdcall spelling. The original returns zero for a null
// interface, otherwise the DeletePlayerFromGroup HRESULT unchanged.
int __stdcall sub_62ee00(int first, int second) {
  IDirectPlay2 *const direct_play = NetDirectPlay;
  int result = reinterpret_cast<int>(direct_play);
  if (direct_play != 0) {
    result = static_cast<int>(direct_play->DeletePlayerFromGroup(
        static_cast<DPID>(first), static_cast<DPID>(second)));
  }
  return result;
}

// ORIGINAL: 0x0062DF70 ?start_voice@Net@@QAEHJ@Z 0x0062DF70-0x0062DFBE BYTE_EXACT
// size      79 bytes
// prototype int (__thiscall ?start_voice@Net@@QAEHJ@Z)(Net* this, long)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// Gates on the interface, then on the two voice-flag bits at +0xD8: out of an
// in-flight capture there is nothing to start. The key is remembered at +0xD4
// so stop_voice can match it, and the started bit is lit only after
// VoiceTx::start comes back - the order the dispatch proves.
int Net::start_voice(long key) {
  if (NetDirectPlay == 0) {
    return 7;
  }
  if ((voice_flags_D8_ & 0x60000000) == 0) {
    field_54_ = 1;
    voice_key_D4_ = static_cast<uint32_t>(key);
    voice_tx_.start();
    voice_flags_D8_ |= 0x20000000;
  }
  return 0;
}

// ORIGINAL: 0x00630BF0 sub_630bf0 0x00630BF0-0x00630C29 BYTE_EXACT
// symbol    ?enumerate_provider_callback@@YGHPAU_GUID@@PADKKPAX@Z
// size      58 bytes
// prototype int (__stdcall sub_630bf0)(const void*, int, int, int, int)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// DirectPlayEnumerateA's callback. `ret 0x14` pins five stack dwords and
// stdcall; the catalogue's own head spells them LPGUID/LPSTR/DWORD, which no
// declaration here can carry, so they are the plain pointer/int they are on
// the wire. Only the provider whose GUID matches KeptServiceProviderGuid is
// kept, and its description pointer and id land in the ServiceStruct the
// caller parked in ServiceEnumTarget.
BOOL __stdcall enumerate_provider_callback(LPGUID provider, LPSTR name,
                                            DWORD, DWORD, LPVOID) {
    if (memcmp(provider, &KeptServiceProviderGuid, sizeof(GUID)) != 0) {
        ServiceList *const list = ServiceEnumTarget;
        list->name_ = name;
        list->provider_guid_ = provider;
        list->add(0);
    }
    return TRUE;
}

// ORIGINAL: 0x0062E250 ?poll_services@Net@@QAEHPAUServiceList@@@Z 0x0062E250-0x0062E299 BYTE_EXACT
// symbol    ?poll_services@Net@@QAEHPAUServiceList@@@Z
// size      74 bytes
// prototype int (__thiscall ?poll_services@Net@@QAEHPAUServiceList@@@Z)(Net* this, ServiceList*)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// Park the list in ServiceEnumTarget for the length of the call - that is
// where sub_630bf0 picks it up - then enumerate. The stack is not cleaned
// after the call, which is what pins DirectPlayEnumerateA as the two-argument
// stdcall the import table carries; the nullary extern the scaffold invented
// could not even be called.
int Net::poll_services(ServiceList *list) {
    if (list == 0) return 0x10;
    if (guid_6E8_ == 0) return 7;
    ServiceEnumTarget = list;
    const HRESULT result = DirectPlayEnumerateA(enumerate_provider_callback, 0);
    ServiceEnumTarget = 0;
    if (result != 0) return 1;
    return 0;
}

// 0x009BE4C0 - the DPCAPS the service join reads the provider's latency out
// of. GetCaps refuses a zero dwSize, so join_service sizes it to the full
// 0x28 before the call. Real storage beside NetGetScratch's end and the
// player-table pointer above, not a Net member.
// RECOVERY_OBJECT: v2 0x009BE4C0 0x009BE4E8 _NetServiceCaps 0x0062E2A0
static DPCAPS NetServiceCaps;  // 0x009BE4C0

// Original direct-address writable .data string passed to WinExec on the
// kept-provider branch; the private array belongs to this source owner.
// RECOVERY_OBJECT: v2 0x00697578 0x00697585 _NetZoneLauncher 0x0062E2A0
static char NetZoneLauncher[] = "mplaynow.exe";

// ORIGINAL: 0x00630D70 sub_630d70 0x00630D70-0x00631044;0x006634FF-0x00663528 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned 0x00630D70 reads Net provider storage at +0x6E0/+0x6E8 and enumerates into the source-owned ServiceList callback target.", "Pinned 0x004E38E0 adjusts to ServiceList at receiver+0x24, calls its real destructor, then restores the saved StringAllocationHeap owner.", "Pinned 0x006634FF..0x00663528 is retained as the original EH cleanup span of this identity.", "The complete 0x004E38E0..0x004E38FB entry is separately mapped as address-derived sub_4e38e0 to the current generated ??_DServiceList symbol; this caller's EH tail remains in the parent mapping.", "Popup allocation uses the BasePop-owned PopupAllocHook declaration, reached through popup.h; dialog item insertion uses BasePop::dialogs() public accessor."], "reason": "Recover the provider chooser on the real Net owner using ServiceList, the StringAllocationHeap handoff, BasePop, Dialogs and join_service."}
// symbol    ?choose_service@Net@@QAEHXZ
int Net::choose_service() {
    // The real virtual allocation base captures and clears the incoming heap;
    // ServiceList destruction restores it after provider cleanup.
    ServiceList providers;
    if (buffer_6E0_ == 0) return 7;

    BasePop *popup = PopupAllocHook();
    if (popup == 0) return 4;

    // poll_services' own status (0x10 / 7 / enumeration failure) and
    // start's failure code both become the result.
    int result = poll_services(&providers);
    if (result == 0) {
        result = popup->start(const_cast<char *>("jackal"), "SELECTSERVICE",
                              -1, 0, 0x42, 0);
        if (result == 0) {
            providers.seek_pos(-1);
            for (int row = 0; row < providers.entry_count_; ++row) {
                providers.next_entry();
                char *name;
                if (providers.current_ != 0) {
                    ServiceStruct *payload = providers.head_ != 0
                        ? providers.current_->payload_
                        : static_cast<ServiceStruct *>(0);
                    // Preserve the original null+4 fault if current_ is live
                    // while head_ is null; do not turn it into a safe row.
                    name = payload->name_;
                } else {
                    name = 0;
                }
                popup->dialogs().item(name, row);
            }

            const int selected = popup->exec(0, 0);
            if (selected == -1) {
                result = 1;
            } else {
                providers.seek_pos(selected); // original ignores failure
                ServiceStruct *service = providers.head_ != 0
                    ? providers.current_->payload_
                    : static_cast<ServiceStruct *>(0);
                result = join_service(service);
            }
        }
    }

    // The pinned method calls vtable slot zero with argument 1 on every path
    // after allocation. The existing BasePop virtual deleting destructor is
    // the source-owned C++ spelling of that lifetime operation.
    delete popup;
    return result;
}

namespace {

// 0x631050 brackets its stack-owned session structures with the shared
// StringAllocationHeap handoff. Declare this before those objects so it
// restores the incoming value after their natural destruction.
struct SessionPickerAllocationHandoff {
  Heap *saved_;
  SessionPickerAllocationHandoff() : saved_(StringAllocationHeap) {
    StringAllocationHeap = 0;
  }
  ~SessionPickerAllocationHandoff() { StringAllocationHeap = saved_; }
};

}  // namespace

// Descriptive method name only. The pinned 0x631050 entry proves a Net
// receiver, but not its original source identifier. The original local window
// is constructed/closed here; the separate modal BasePop is obtained from
// PopupAllocHook and owns the menus and EditGroup fields below.
// ORIGINAL: 0x00631050 sub_631050 0x00631050-0x00631A06;0x00663528-0x0066372B FILE
// RECOVERY_BINDING: v1 {"evidence": ["Pinned constructor/destructor calls and direct accesses establish the GraphicWin base, ordered Time/Heap/ListBox/PushButton/list members, and observed tail through +0x37A4. The descriptive owner name and complete original allocation remain unproved.", "The integer timer adapter checks nonzero context; the distinct nullary focus member does not. Pinned table 0x670DDC establishes slot 12 at 0x670E0C -> 0x63C9D0 and slot 44 at 0x670E8C -> 0x63C3A0. Automatic owner destruction replaces manual table staging and executable-address forwarding.", "The Net caller retains one automatic picker, original unchecked copies and invalid-node error paths, and password-pointer reloads after modal execution. Indirect event linkage, precise unwind correspondence, historical matching and full linked/runtime fidelity remain obligations."], "reason": "Typed session-picker ownership and its Net receiver-backed caller, preserving the original entries and compiler-generated lifetime identities."}
// symbol    ?session_picker_dialog@Net@@QAEHXZ
// TRIED: no ESP-relative frame (raw buffer + manual calls, not a real frame pointer omission); the entry receiver (`mov edi,ecx`) and Ghidra's `param_2` are not recoverable under the mandated 0-arg `__cdecl` signature, so both are read from placeholder storage instead of a real caller-supplied value; sub_63bb20/ sub_63bd10/sub_4e34b0/sub_4e3350/sub_4080b0/fn_00401c80/ fn_00401ce0/fn_00402970 called with no receiver/args since their declared prototypes are 0-arg extern "C" and changing them would break linkage to the real symbol.
int Net::session_picker_dialog() {
  SessionPickerAllocationHandoff allocation_handoff;
  AlphaNetLedger available_sessions;
  NetSessionPickerWin picker;
  int result = 0;
  NetAllocationNode *selected_session = 0;

  // Native checks Net+0x6E0 before allocating the modal popup. The one
  // 0x63BD10 call in each teardown is the local picker's destructor body,
  // which the compiler expands before natural member/base destruction; do
  // not add a second explicit close here.
  if (buffer_6E0_ == 0) {
    return 0;
  }

  BasePop *popup = PopupAllocHook();
  if (popup == 0) {
    return 4;
  }

  static char choose_title[] = "Would you like to...?";
  static char create_or_join[] = "Start a new session or...";
  static char find_session[] = "Search for an existing one?";
  static char no_sessions[] =
      "No sessions are currently running.  Would you like to try again?";
  static char yes_text[] = "Yes";
  static char no_text[] = "No";
  static char create_text[] = "Create Session";
  static char session_name_text[] = "Session Name:";
  static char player_limit_text[] = "Maximum Players:";
  static char session_password_text[] = "Session Password (Optional):";
  static char player_name_text[] = "Your Name:";
  static char password_required_text[] =
      "Password required for this session:";

  if (popup->init(0x10, 0) != 0) goto session_picker_cleanup;
  popup->string(choose_title);
  popup->dialogs().Dialogs::item(create_or_join, 0);
  popup->dialogs().Dialogs::item(find_session, 1);

  if (popup->exec(0, 0) == 0) goto session_picker_create;

  for (;;) {
    if (enumerate_sessions(&available_sessions, 0) != 0) {
      result = 0;
      goto session_picker_cleanup;
    }

    if (available_sessions.entry_count_ == 0) {
      if (popup->init(0x10, 0) != 0) goto session_picker_cleanup;
      popup->string(no_sessions);
      popup->dialogs().Dialogs::item(yes_text, 0);
      popup->dialogs().Dialogs::item(no_text, 1);
      popup->button(create_text);
      const int retry_choice = popup->exec(0, 0);
      if (popup->input_poll_state() == 1) goto session_picker_create;
      if (retry_choice != 0) {
        // The original calls exit(2) on this affirmative-looking branch.
        exit(2);
      }
      continue;
    }

    if (popup->init(0x42, 0) != 0) goto session_picker_cleanup;
    available_sessions.seek_pos(-1);
    int row = 0;
    while (row < available_sessions.entry_count_) {
      NetAllocationNode *session = available_sessions.next_entry();
      char *name = session == 0 ? 0 : session->session_name_;
      popup->dialogs().Dialogs::item(name, row);
      ++row;
    }
    popup->button(create_text);

    const int selected_row = popup->exec(0, 0);
    if (selected_row == -1) {
      result = 0;
      goto session_picker_cleanup;
    }
    if (popup->input_poll_state() == 1) goto session_picker_create;

    available_sessions.seek_pos(selected_row);
    selected_session = available_sessions.head_ == 0
        ? 0 : available_sessions.current_->payload_;
    goto session_picker_credentials;
  }

session_picker_create:
  for (;;) {
    if (popup->init(0x44, 0) != 0) {
      result = 0;
      goto session_picker_cleanup;
    }
    popup->set_width(0x190);
    char default_players[16];
    _itoa(0x10, default_players, 10);
    popup->dialogs().EditGroup::edit(session_name_text, 0, 0x32);
    popup->dialogs().EditGroup::edit(player_limit_text, default_players, 2);
    popup->dialogs().EditGroup::edit(session_password_text, 0, 0x32);
    // Native 0x63156D resolves the Dialog virtual-base displacement from
    // Dialogs' vbtable, then writes 1 to Dialog::field_4C_. Qualification is
    // required so C++ applies that genuine shared-virtual-base adjustment.
    popup->dialogs().Dialog::field_4C_ = 1;
    ParseStrBuffer[0].str[0] = 0;
    ParseStrBuffer[1].str[0] = 0;
    ParseStrBuffer[2].str[0] = 0;

    if (popup->exec(0, 0) == -1) {
      result = 0;
      goto session_picker_cleanup;
    }
    if (ParseStrBuffer[1].str[0] != 0 &&
        atoi(ParseStrBuffer[1].str) > 0x10) {
      continue;
    }

    // The native local begins at frame+0x58 and the following integer-to-text
    // buffer begins at frame+0x70, leaving 0x18 bytes for this unchecked
    // strcat destination. Preserve that small local rather than widening it.
    char session_name[0x18];
    session_name[0] = 0;
    strcat(session_name, ParseStrBuffer[0].str);
    char *password = ParseStrBuffer[2].str[0] == 0
        ? 0 : ParseStrBuffer[2].str;
    NetAllocationNode *created = create_session(
        session_name, atoi(ParseStrBuffer[1].str), password);
    if (created == 0) goto session_picker_cleanup;
    selected_session = created;
    goto session_picker_credentials;
  }

session_picker_credentials:
  if (popup->init(4, 0) != 0) goto session_picker_cleanup;
  popup->dialogs().EditGroup::edit(player_name_text, 0, 0x32);
  if (selected_session->password_ != 0 &&
      selected_session->password_[0] != 0) {
    popup->dialogs().EditGroup::edit(password_required_text, 0, 0x32);
  }
  if (popup->exec(0, 0) == -1) goto session_picker_cleanup;
  // The pinned code reloads the node's password pointer after the modal
  // dialog. If a password is now present it uses both parse buffers; if not,
  // it snapshots the player name through the native unchecked strcat local
  // and passes a null password (not the empty second parse buffer).
  if (selected_session->password_ != 0 &&
      selected_session->password_[0] != 0) {
    result = join_session(selected_session, ParseStrBuffer[0].str,
                           ParseStrBuffer[1].str);
  } else {
    char player_name[0x18];
    player_name[0] = 0;
    strcat(player_name, ParseStrBuffer[0].str);
    result = join_session(selected_session, player_name, 0);
  }

session_picker_cleanup:
  delete popup;
  return result;
}

// Unreferenced in the pinned image (no call, jump or data reference).
// Original effect: return without modifying EAX; callee stack cleanup 0 bytes.
// ORIGINAL: 0x00631A10 nullsub_256 0x00631A10-0x00631A11 BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason": "Bind the unreferenced original leaf as an empty/constant source function in the translation unit whose address band contains it; its original name, prototype and callers remain unknown.", "evidence": ["pinned:terranx_original.exe:0x00631A10-0x00631A11 return without modifying EAX; callee stack cleanup 0 bytes", "pinned scan: no rel32 call/jmp and no dword reference to this address anywhere in the image", "address band: nearest mapped neighbours are 0x00631050 sub_631050 (src/net_session_picker.cpp) and 0x00631A20 ?get_player_name@Net@@QAEPADK@Z (src/net_class.cpp); sub_631050 is bound as Net::session_picker_dialog, so the surrounding band is Net (net_class.cpp)"]}
// symbol    ?nullsub_256@@YAXXZ
// kind      game
void __cdecl nullsub_256() {
}

// Recognized signed HRESULT branches in the pinned error interpreter bind
// these parse_says token literals; its common popup binds jackal/DPLAYERROR.
// Each target/selector pair belongs to the indicated guarded dispatch.
// These are source interpretations; code spans and historical claims stay intact.
// RECOVERY_LITERAL: v1 0x00697B34 0x00697B4D ??_C@_0BJ@JDAH@DPERR_ALREADYINITIALIZED?$AA@ 0x00631BA0
// RECOVERY_LITERAL: v1 0x00697B50 0x00697B63 ??_C@_0BD@FECJ@DPERR_ACCESSDENIED?$AA@ 0x00631BA0
// RECOVERY_LITERAL: v1 0x00697B64 0x00697B78 ??_C@_0BE@OOHJ@DPERR_ACTIVEPLAYERS?$AA@ 0x00631BA0
// RECOVERY_LITERAL: v1 0x00697B78 0x00697B8D ??_C@_0BF@DJPD@DPERR_BUFFERTOOSMALL?$AA@ 0x00631BA0
// RECOVERY_LITERAL: v1 0x00697B90 0x00697BA4 ??_C@_0BE@FLAJ@DPERR_CANTADDPLAYER?$AA@ 0x00631BA0
// RECOVERY_LITERAL: v1 0x00697BA4 0x00697BBA ??_C@_0BG@IAEB@DPERR_CANTCREATEGROUP?$AA@ 0x00631BA0
// RECOVERY_LITERAL: v1 0x00697BBC 0x00697BD3 ??_C@_0BH@PAOG@DPERR_CANTCREATEPLAYER?$AA@ 0x00631BA0
// RECOVERY_LITERAL: v1 0x00697BD4 0x00697BEC ??_C@_0BI@NINM@DPERR_CANTCREATESESSION?$AA@ 0x00631BA0
// RECOVERY_LITERAL: v1 0x00697BEC 0x00697C06 ??_C@_0BK@BKDD@DPERR_CAPSNOTAVAILABLEYET?$AA@ 0x00631BA0
// RECOVERY_LITERAL: v1 0x00697C08 0x00697C18 ??_C@_0BA@MELA@DPERR_EXCEPTION?$AA@ 0x00631BA0
// RECOVERY_LITERAL: v1 0x00697C18 0x00697C26 ??_C@_0O@PAAE@DPERR_GENERIC?$AA@ 0x00631BA0
// RECOVERY_LITERAL: v1 0x00697C28 0x00697C3B ??_C@_0BD@HKEK@DPERR_INVALIDFLAGS?$AA@ 0x00631BA0
// RECOVERY_LITERAL: v1 0x00697C3C 0x00697C50 ??_C@_0BE@LGCA@DPERR_INVALIDOBJECT?$AA@ 0x00631BA0
// RECOVERY_LITERAL: v1 0x00697C50 0x00697C63 ??_C@_0BD@IAFO@DPERR_INVALIDPARAM?$AA@ 0x00631BA0
// RECOVERY_LITERAL: v1 0x00697C64 0x00697C78 ??_C@_0BE@FMNI@DPERR_INVALIDPLAYER?$AA@ 0x00631BA0
// RECOVERY_LITERAL: v1 0x00697C78 0x00697C85 ??_C@_0N@FDKF@DPERR_NOCAPS?$AA@ 0x00631BA0
// RECOVERY_LITERAL: v1 0x00697C88 0x00697C9B ??_C@_0BD@JLI@DPERR_NOCONNECTION?$AA@ 0x00631BA0
// RECOVERY_LITERAL: v1 0x00697C9C 0x00697CAB ??_C@_0P@OJCI@DPERR_NOMEMORY?$AA@ 0x00631BA0
// RECOVERY_LITERAL: v1 0x00697CAC 0x00697CBD ??_C@_0BB@FAOE@DPERR_NOMESSAGES?$AA@ 0x00631BA0
// RECOVERY_LITERAL: v1 0x00697CC0 0x00697CD8 ??_C@_0BI@KFMN@DPERR_NONAMESERVERFOUND?$AA@ 0x00631BA0
// RECOVERY_LITERAL: v1 0x00697CD8 0x00697CE8 ??_C@_0BA@MGKH@DPERR_NOPLAYERS?$AA@ 0x00631BA0
// RECOVERY_LITERAL: v1 0x00697CE8 0x00697CF9 ??_C@_0BB@BHBA@DPERR_NOSESSIONS?$AA@ 0x00631BA0
// RECOVERY_LITERAL: v1 0x00697CFC 0x00697D0D ??_C@_0BB@GKEG@DPERR_SENDTOOBIG?$AA@ 0x00631BA0
// RECOVERY_LITERAL: v1 0x00697D10 0x00697D1E ??_C@_0O@KDFP@DPERR_TIMEOUT?$AA@ 0x00631BA0
// RECOVERY_LITERAL: v1 0x00697D20 0x00697D32 ??_C@_0BC@MPPN@DPERR_UNAVAILABLE?$AA@ 0x00631BA0
// RECOVERY_LITERAL: v1 0x00697D34 0x00697D46 ??_C@_0BC@MDM@DPERR_UNSUPPORTED?$AA@ 0x00631BA0
// RECOVERY_LITERAL: v1 0x00697D48 0x00697D53 ??_C@_0L@NBGB@DPERR_BUSY?$AA@ 0x00631BA0
// RECOVERY_LITERAL: v1 0x00697D54 0x00697D65 ??_C@_0BB@DEPF@DPERR_USERCANCEL?$AA@ 0x00631BA0
// RECOVERY_LITERAL: v1 0x00697D68 0x00697D7A ??_C@_0BC@MPMN@DPERR_NOINTERFACE?$AA@ 0x00631BA0
// RECOVERY_LITERAL: v1 0x00697D7C 0x00697D95 ??_C@_0BJ@PKBA@DPERR_CANNOTCREATESERVER?$AA@ 0x00631BA0
// RECOVERY_LITERAL: v1 0x00697D98 0x00697DA9 ??_C@_0BB@DHID@DPERR_PLAYERLOST?$AA@ 0x00631BA0
// RECOVERY_LITERAL: v1 0x00697DAC 0x00697DBE ??_C@_0BC@HCBE@DPERR_SESSIONLOST?$AA@ 0x00631BA0
// RECOVERY_LITERAL: v1 0x00697DC0 0x00697DD5 ??_C@_0BF@ECHL@DPERR_BUFFERTOOLARGE?$AA@ 0x00631BA0
// RECOVERY_LITERAL: v1 0x00697DD8 0x00697DF0 ??_C@_0BI@GPHA@DPERR_CANTCREATEPROCESS?$AA@ 0x00631BA0
// RECOVERY_LITERAL: v1 0x00697DF0 0x00697E04 ??_C@_0BE@PIBM@DPERR_APPNOTSTARTED?$AA@ 0x00631BA0
// RECOVERY_LITERAL: v1 0x00697E04 0x00697E1B ??_C@_0BH@NOFK@DPERR_INVALIDINTERFACE?$AA@ 0x00631BA0
// RECOVERY_LITERAL: v1 0x00697E1C 0x00697E34 ??_C@_0BI@CAPJ@DPERR_NOSERVICEPROVIDER?$AA@ 0x00631BA0
// RECOVERY_LITERAL: v1 0x00697E34 0x00697E4D ??_C@_0BJ@KBJE@DPERR_UNKNOWNAPPLICATION?$AA@ 0x00631BA0
// RECOVERY_LITERAL: v1 0x00697E50 0x00697E61 ??_C@_0BB@GNDN@DPERR_NOTLOBBIED?$AA@ 0x00631BA0
// RECOVERY_LITERAL: v1 0x00697E64 0x00697E6F ??_C@_0L@NHBN@DPLAYERROR?$AA@ 0x00631BA0
// RECOVERY_LITERAL: v1 0x00697E70 0x00697E77 ??_C@_06KHMO@jackal?$AA@ 0x00631BA0
// RECOVERY_ASSOCIATED: v1 switch_targets 0x00631BA0 0x00631EDC 0x00631EF0 0x00631C51
// RECOVERY_ASSOCIATED: v1 switch_selector 0x00631BA0 0x00631EF0 0x00631F0A 0x00631C4B
// RECOVERY_ASSOCIATED: v1 switch_targets 0x00631BA0 0x00631F0C 0x00631F34 0x00631CB4
// RECOVERY_ASSOCIATED: v1 switch_selector 0x00631BA0 0x00631F34 0x00631FA3 0x00631CAE
// RECOVERY_ASSOCIATED: v1 switch_targets 0x00631BA0 0x00631FA4 0x00631FCC 0x00631D6E
// RECOVERY_ASSOCIATED: v1 switch_selector 0x00631BA0 0x00631FCC 0x00632027 0x00631D68
// RECOVERY_ASSOCIATED: v1 switch_targets 0x00631BA0 0x00632028 0x0063203C 0x00631E75
// RECOVERY_ASSOCIATED: v1 switch_selector 0x00631BA0 0x0063203C 0x00632065 0x00631E6F
// ORIGINAL: 0x00631BA0 sub_631ba0 0x00631BA0-0x00631EDB BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["The 39 DPERR identifiers are defined by the already included dplay.h and Win32 HRESULT aliases. Their configured SDK values were checked against every original case value; use those signed HRESULT constants instead of narrowing unsigned literals.", "Pinned recognized arms prepare parse_says arguments and join the common call; unknown values skip it. Current general.h takes LPCSTR, so token literals need no mutable-pointer casts.", "HRESULT is signed LONG. Original signed branches at body offsets A,1B,2B,34,1AD,266,26F support switching directly on error rather than converting it to unsigned.", "Pinned 0x00631E1A compares with 0x88770136, the SDK DPERR_SESSIONLOST value. Its matching branch starts at 0x00631E25 and pushes the token at 0x00631E29 through literal 0x00697DAC. The former source 0x88770132 was a translation error; the other 38 mappings agree with the original.", "The body ends at 0x00631EDB. Four compiler-owned target/selector pairs follow at 631EDC/631EF0,631F0C/631F34,631FA4/631FCC,632028/63203C; they are data outside that function-code span, not neighboring functions to borrow.", "Dismissal returns zero; otherwise cd_check at 0x00628F30 supplies the result. Preserve the seed switch-layout finding and original identity/span; no historical byte claim or original nominal linkage proof is granted."], "reason": "Recover the signed DirectPlay HRESULT switch and case-local parse_says operations using the existing SDK error names, preserving original dispatch and popup/CD-check behavior."}
// symbol    ?report_directplay_error@Net@@AAEHJ@Z
// TRIED: a single flat switch over the 38 DPERR_* case values with a `default: goto tail;` matches through instr #16 (the top-level binary-search split) but the compiler's chosen jump-table boundaries at #17 diverge from the original's nested-if split
// Both original calls (0x0062E31B, 0x0062E361) pass the Net receiver in ECX;
// ESI preserves that receiver from 0x0062E2A8. The callee never reads it.
// report_directplay_error is descriptive; the original spelling is unknown.
int Net::report_directplay_error(HRESULT error) {
    switch (error) {
    case DPERR_ALREADYINITIALIZED: parse_says(0, "DPERR_ALREADYINITIALIZED", -1, -1); break;
    case DPERR_ACCESSDENIED: parse_says(0, "DPERR_ACCESSDENIED", -1, -1); break;
    case DPERR_ACTIVEPLAYERS: parse_says(0, "DPERR_ACTIVEPLAYERS", -1, -1); break;
    case DPERR_BUFFERTOOSMALL: parse_says(0, "DPERR_BUFFERTOOSMALL", -1, -1); break;
    case DPERR_CANTADDPLAYER: parse_says(0, "DPERR_CANTADDPLAYER", -1, -1); break;
    case DPERR_CANTCREATEGROUP: parse_says(0, "DPERR_CANTCREATEGROUP", -1, -1); break;
    case DPERR_CANTCREATEPLAYER: parse_says(0, "DPERR_CANTCREATEPLAYER", -1, -1); break;
    case DPERR_CANTCREATESESSION: parse_says(0, "DPERR_CANTCREATESESSION", -1, -1); break;
    case DPERR_CAPSNOTAVAILABLEYET: parse_says(0, "DPERR_CAPSNOTAVAILABLEYET", -1, -1); break;
    case DPERR_EXCEPTION: parse_says(0, "DPERR_EXCEPTION", -1, -1); break;
    case DPERR_GENERIC: parse_says(0, "DPERR_GENERIC", -1, -1); break;
    case DPERR_INVALIDFLAGS: parse_says(0, "DPERR_INVALIDFLAGS", -1, -1); break;
    case DPERR_INVALIDOBJECT: parse_says(0, "DPERR_INVALIDOBJECT", -1, -1); break;
    case DPERR_INVALIDPARAM: parse_says(0, "DPERR_INVALIDPARAM", -1, -1); break;
    case DPERR_INVALIDPLAYER: parse_says(0, "DPERR_INVALIDPLAYER", -1, -1); break;
    case DPERR_NOCAPS: parse_says(0, "DPERR_NOCAPS", -1, -1); break;
    case DPERR_NOCONNECTION: parse_says(0, "DPERR_NOCONNECTION", -1, -1); break;
    case DPERR_NOMEMORY: parse_says(0, "DPERR_NOMEMORY", -1, -1); break;
    case DPERR_NOMESSAGES: parse_says(0, "DPERR_NOMESSAGES", -1, -1); break;
    case DPERR_NONAMESERVERFOUND: parse_says(0, "DPERR_NONAMESERVERFOUND", -1, -1); break;
    case DPERR_NOPLAYERS: parse_says(0, "DPERR_NOPLAYERS", -1, -1); break;
    case DPERR_NOSESSIONS: parse_says(0, "DPERR_NOSESSIONS", -1, -1); break;
    case DPERR_SENDTOOBIG: parse_says(0, "DPERR_SENDTOOBIG", -1, -1); break;
    case DPERR_TIMEOUT: parse_says(0, "DPERR_TIMEOUT", -1, -1); break;
    case DPERR_UNAVAILABLE: parse_says(0, "DPERR_UNAVAILABLE", -1, -1); break;
    case DPERR_UNSUPPORTED: parse_says(0, "DPERR_UNSUPPORTED", -1, -1); break;
    case DPERR_NOINTERFACE: parse_says(0, "DPERR_NOINTERFACE", -1, -1); break;
    case DPERR_BUSY: parse_says(0, "DPERR_BUSY", -1, -1); break;
    case DPERR_USERCANCEL: parse_says(0, "DPERR_USERCANCEL", -1, -1); break;
    case DPERR_CANNOTCREATESERVER: parse_says(0, "DPERR_CANNOTCREATESERVER", -1, -1); break;
    case DPERR_PLAYERLOST: parse_says(0, "DPERR_PLAYERLOST", -1, -1); break;
    case DPERR_SESSIONLOST: parse_says(0, "DPERR_SESSIONLOST", -1, -1); break;
    case DPERR_BUFFERTOOLARGE: parse_says(0, "DPERR_BUFFERTOOLARGE", -1, -1); break;
    case DPERR_CANTCREATEPROCESS: parse_says(0, "DPERR_CANTCREATEPROCESS", -1, -1); break;
    case DPERR_APPNOTSTARTED: parse_says(0, "DPERR_APPNOTSTARTED", -1, -1); break;
    case DPERR_INVALIDINTERFACE: parse_says(0, "DPERR_INVALIDINTERFACE", -1, -1); break;
    case DPERR_NOSERVICEPROVIDER: parse_says(0, "DPERR_NOSERVICEPROVIDER", -1, -1); break;
    case DPERR_UNKNOWNAPPLICATION: parse_says(0, "DPERR_UNKNOWNAPPLICATION", -1, -1); break;
    case DPERR_NOTLOBBIED: parse_says(0, "DPERR_NOTLOBBIED", -1, -1); break;
    default: break;
    }

    if (pop_caption_title(const_cast<char *>("jackal"),
                          const_cast<char *>("DPLAYERROR"), 0x10040, 0) == -1) {
        return 0;
    }
    return cd_check();
}

/*
Purpose: Join one enumerated service provider: copy the provider GUID into
         the session's 0x6EC slot, and when it is the internet TCP/IP service
         the enumeration kept, just launch the Zone's "mplaynow.exe" - the
         DirectPlay lobby does the rest. Anything else creates the DirectPlay
         object directly, publishes it as the shared interface (the same
         0x9BE600 word every other entry point tests), reads the provider's
         latency, and seeds all sixteen player slots with it - 100 ms when
         the provider does not say.
// ORIGINAL: 0x0062E2A0 ?join_service@Net@@QAEHPAUServiceStruct@@@Z 0x0062E2A0-0x0062E41C FILE BYTE_EXACT
// size      380 bytes
// prototype int (__thiscall ?join_service@Net@@QAEHPAUServiceStruct@@@Z)(Net* this, ServiceStruct*)
// callers   2   call targets   4
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00631BA0 0x00645C70 0x006465F0 0x00650214
// indirect  0x0062E2F2 0x0062E33B 0x0062E353 0x0062E373 0x0062E3A8
// Homed from src/unrecovered/0062e2a0.cpp.
Return Value: 0x10 with no service record, 1 after any reported failure, 1
         when the launcher exited below 21, 0x16 when it did not, else 0
Status: Complete
*/
int Net::join_service(ServiceStruct *service) {
    if (service == 0) {
        return 0x10;
    }

    // The enumeration-owned GUID is copied into this session's inline
    // provider GUID and its original address remains the DirectPlay operand.
    GUID *provider = &service->provider_guid_;
    service_provider_guid_ = *provider;

    if (memcmp(provider, &KeptServiceProviderGuid, 0x10) == 0) {
        // The internet service: hand off to the Zone launcher. SW_SHOW, and
        // anything under a successful-process answer reports "failed".
        unsigned int started = WinExec(NetZoneLauncher, SW_SHOW);
        return started < 0x15 ? 1 : 0x16;
    }

    LPDIRECTPLAY direct_play;
    // The DX1 create signature - the image's one DirectPlayCreate import,
    // filled through the out parameter the caller owns.
    HRESULT result = DirectPlayCreate(provider, &direct_play, 0);
    if (result != 0) {
        report_directplay_error(result);
        return 1;
    }

    // QueryInterface for the Ansi IDirectPlay2 the rest of Net dispatches
    // through, publish it, and release the creator either way.
    result = direct_play->QueryInterface(
        IID_IDirectPlay2A, reinterpret_cast<void **>(&NetDirectPlay));
    thread_info_.direct_play_ = NetDirectPlay;
    if (result != 0) {
        direct_play->Release();
        direct_play = 0;
        report_directplay_error(result);
        return 1;
    }
    direct_play->Release();
    direct_play = 0;

    memset(&NetServiceCaps, 0, 0x28);
    NetServiceCaps.dwSize = 0x28;
    if (NetDirectPlay->GetCaps(&NetServiceCaps, 1) != 0) {
        return 1;
    }

    voice_flags_D8_ &= 0xEFFFFFFF;

    // The provider's latency estimate (0 when it would not say - the image
    // stores the whole dword, and a zero means the 100 default) lands in
    // every player slot's third word and in the 0x6D4 default beside them.
    if (NetServiceCaps.dwLatency != 0) {
        NetPlayer *player = players_;
        int count = 16;
        do {
            player->field_C_ = NetServiceCaps.dwLatency;
            ++player;
            --count;
        } while (count != 0);
        field_6D4_ = NetServiceCaps.dwLatency;
    } else {
        NetPlayer *player = players_;
        int count = 16;
        do {
            player->field_C_ = 100;
            ++player;
            --count;
        } while (count != 0);
        field_6D4_ = 100;
    }
    return 0;
}

// The modem and serial service-provider address GUIDs the DX5 SDK spells
// DPAID_Modem / DPAID_Serial - dpaddr.h is not in this include set, so the
// two create_session compares join NetLobbyAppGuid and
// KeptServiceProviderGuid as literals read off the image (0x006717F0 /
// 0x00671800). Two-player sessions are forced for both.
static const GUID NetModemServiceGuid = {
    0x44EAA760, 0xCB68, 0x11CF, {0x9C, 0x4E, 0x00, 0xA0, 0xC9, 0x05, 0x42, 0x5E}};
static const GUID NetSerialServiceGuid = {
    0x0F1D6860, 0x88D9, 0x11CF, {0x9C, 0x4E, 0x00, 0xA0, 0xC9, 0x05, 0x42, 0x5E}};

/*
Purpose: Host a named session: close any live one, size it to two players
         when the joined provider was the modem or the serial service, open
         the DPSESSIONDESC2 (MIGRATEHOST, the session GUID from init as the
         application GUID, the option word at 0x44 riding dwUser1), then
         re-read the live description through GetSessionDesc and file its
         name, password, instance GUID, flags and player ceiling in the
         allocation node at 0x700 - the same record the enumeration callback
         fills for received sessions. Leaves the 0x10000 "in a lobby session"
         bit lit.
// ORIGINAL: 0x0062EAA0 ?create_session@Net@@QAEHPADHPAD@Z 0x0062EAA0-0x0062ED2F FILE BYTE_EXACT
// symbol    ?create_session@Net@@QAEPAVNetAllocationNode@@PADH0@Z
// size      655 bytes
// prototype int (__thiscall ?create_session@Net@@QAEHPADHPAD@Z)(Net* this, int8*, int, int8*)
// callers   3   call targets   6
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4510 0x00644EF2 0x006453E0 0x00645470 0x00645C70 0x006465F0
// indirect  0x0062EB08 0x0062EBD3 0x0062EBF4 0x0062EC20 0x0062EC60
// Homed from src/unrecovered/0062eaa0.cpp.
Return Value: the allocation node's address, or 0 when any stage failed
Status: Coverage transcription - unclaimed body
*/
NetAllocationNode *Net::create_session(char *name, int max_players, char *password) {
    if (NetDirectPlay == 0) {
        return 0;
    }
    if (name == 0) {
        return 0;
    }
    if (max_players < 2) {
        return 0;
    }

    if (allocation_node_.session_name_ != 0) {
        free(allocation_node_.session_name_);
        allocation_node_.session_name_ = 0;
        // The replaced session is closed through the interface.
        NetDirectPlay->Close();
    }
    if (allocation_node_.password_ != 0) {
        free(allocation_node_.password_);
        allocation_node_.password_ = 0;
    }

    // The modem and serial providers host exactly two players.
    if (memcmp(&service_provider_guid_, &NetSerialServiceGuid, 0x10) == 0
        || memcmp(&service_provider_guid_, &NetModemServiceGuid, 0x10) == 0) {
        max_players = 2;
    }

    DPSESSIONDESC2 desc;
    memset(&desc, 0, 0x50);
    desc.dwSize = 0x50;
    desc.guidApplication = *guid_6E8_;
    desc.dwFlags = DPSESSION_MIGRATEHOST;
    desc.dwMaxPlayers = max_players;
    desc.lpszPasswordA =
        password != 0 && *password != 0 ? password : 0;
    desc.dwUser1 = field_44_;
    desc.lpszSessionNameA = name;

    if (NetDirectPlay->Open(&desc, DPOPEN_CREATE) != 0) {
        return 0;
    }

    unsigned long size;
    NetDirectPlay->GetSessionDesc(0, &size);

    memset(&NetServiceCaps, 0, 0x28);
    NetServiceCaps.dwSize = 0x28;
    if (NetDirectPlay->GetCaps(&NetServiceCaps, 1) != 0) {
        return 0;
    }

    DPSESSIONDESC2 *live = static_cast<DPSESSIONDESC2 *>(mem_get(size));
    if (live == 0) {
        return 0;
    }
    if (NetDirectPlay->GetSessionDesc(live, &size) != 0) {
        return 0;
    }

    allocation_node_.session_name_ =
        static_cast<char *>(mem_get(strlen(name) + 1));
    *allocation_node_.session_name_ = 0;
    strcat(allocation_node_.session_name_, name);
    if (password != 0 && *password != 0) {
        allocation_node_.password_ =
            static_cast<char *>(mem_get(strlen(password) + 1));
        *allocation_node_.password_ = 0;
        strcat(allocation_node_.password_, password);
    }

    allocation_node_.instance_guid_ = live->guidInstance;
    allocation_node_.session_flags_ = live->dwFlags;
    allocation_node_.max_players_ = max_players;
    allocation_node_.current_players_ = 0;
    free(live);
    voice_flags_D8_ |= 0x10000;
    return &allocation_node_;
}

// ORIGINAL: 0x00630A40 sub_630a40 0x00630A40-0x00630A7D BYTE_EXACT
// symbol    ?sub_630a40@Net@@QAEPAXHPAH@Z
// size      62 bytes
// Historical seed prototype: int (__thiscall sub_630a40)(Net* this, int, int)
// Historical catalogue: callers 1, call targets 1; not reachability proof.
// kind      game
// flags     hidden;sp_ready;purged_ok
// Reads a matching message off the 0x10C queue: NetGetScratch is the buffer,
// key supplies the routing-key filter and length_out receives the signed length.
// The answer is the
// BUFFER's address when the read landed and zero when it did not - the
// image's `neg/sbb/and eax, 0x9bc4bc` tail, which is why the guard has to be
// an `extern` object and not a folded literal.
void *Net::sub_630a40(int key, int *length_out) {
  if (NetDirectPlay == 0) {
    return 0;
  }
  if (key == 0) {
    return 0;
  }
  int got = queue_10c_.get(NetGetScratch, key, 0, length_out);
  return got != 0 ? NetGetScratch : 0;
}

// ORIGINAL: 0x00630980 sub_630980 0x00630980-0x006309BA BYTE_EXACT
// symbol    ?sub_630980@Net@@QAEPAXPAIPAH@Z
// size      59 bytes
// Historical seed prototype: int (__thiscall sub_630980)(Net* this, int, int)
// Historical catalogue: callers 1, call targets 1; not reachability proof.
// kind      game
// flags     hidden;sp_ready;purged_ok
// The sender's twin of sub_630a40 above: hand the message to the 0x10C queue's
// own reader and answer the buffer's address on success.
// LEVER: the two guards SHARE one exit here (`je 0x6309b5` twice, one
//   xor/ret tail), which is the chained `||` shape - unlike sub_630a40 above,
//   whose image gives each guard its own inline `ret 8` and so wants two
//   separate `if`s. Measured both ways: chained 20/20 BYTE_EXACT, separate
//   guards 2/19 with the branch polarity flipped. 2026-09-04.
void *Net::sub_630980(unsigned int *key_out, int *length_out) {
  if (NetDirectPlay == 0 || key_out == 0) {
    return 0;
  }
  int got = queue_10c_.peek_head(NetGetScratch, key_out, 0, length_out);
  return got != 0 ? NetGetScratch : 0;
}

// ORIGINAL: 0x006309C0 sub_6309c0 0x006309C0-0x006309FD BYTE_EXACT
// symbol    ?sub_6309c0@Net@@QAEPAXHPAH@Z
// size      61 bytes
// Historical seed prototype: int (__thiscall sub_6309c0)(Net* this, int, int)
// Historical catalogue: callers 1, call targets 1; not reachability proof.
// kind      game
// flags     hidden;sp_ready;purged_ok
// LEVER: the mirror image of sub_630980 above, and the guards are the one
//   thing that differs - HERE the image gives each guard its own inline
//   `xor/ret 8`, so the two tests stay SEPARATE; chaining them shared one
//   tail and flipped the polarity (2/19). Same body, opposite branch shape,
//   on two functions 64 bytes apart. Measured both ways, 2026-09-04.
void *Net::sub_6309c0(int key, int *length_out) {
  if (NetDirectPlay == 0) {
    return 0;
  }
  if (key == 0) {
    return 0;
  }
  int got = queue_10c_.peek_key(NetGetScratch, static_cast<unsigned int>(key),
                             0, length_out);
  return got != 0 ? NetGetScratch : 0;
}



// Recovered Net receive/player operations; original stack receivers are Net.
/*
Purpose: Read one packet off Net's middle queue, retrying until something
         arrives or the receive timeout at Net+0x6FC expires.

         The queue is the NetFifo at Net+0x10C; each attempt reads into the
         shared NetGetScratch buffer and answers that buffer's address on
         success, zero on timeout.

// ORIGINAL: 0x00630A80 sub_630a80 0x00630A80-0x00630AFC BYTE_EXACT
// symbol    ?read_queue_head_until_timeout@Net@@QAEPAXPAI0@Z
// TRIED: original spills its cached timeGetTime() start value into the (now-unused) first stack argument slot instead of a fresh local; not reproduced.
// Historical emitted-symbol hint: @sub_630a80@16
// size      124 bytes
// prototype
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00633F70
// indirect  0x00630A9D 0x00630AA5 0x00630AD1
Return Value: NetGetScratch when a packet was read, zero otherwise
Status: Complete
*/
void *Net::read_queue_head_until_timeout(unsigned int *key_out, unsigned int *length_out) {
    if (NetDirectPlay == 0 || key_out == 0) {
        return 0;
    }
    const DWORD start = timeGetTime();
    while (timeGetTime() - start < field_6FC_) {
        if (queue_10c_.get(NetGetScratch, key_out, 0, length_out) != 0) {
            return NetGetScratch;
        }
    }
    return 0;
}

/*
Purpose: The sender's twin of sub_630a80: wait on the same queue for the
         NEXT reader, using the signed-length get overload, until the
         timeout at Net+0x6FC expires.

// ORIGINAL: 0x00630B00 sub_630b00 0x00630B00-0x00630B7C BYTE_EXACT
// symbol    ?read_queue_key_until_timeout@Net@@QAEPAXHPAH@Z
// TRIED: kept the contract's __stdcall(int,int) head question open - the disasm sets ebx = ecx on entry, so it is really this receiver.
// Historical emitted-symbol hint: @sub_630b00@16
// size      124 bytes
// prototype
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00633D90
// indirect  0x00630B1D 0x00630B25 0x00630B51
Return Value: NetGetScratch when a packet was read, zero otherwise
Status: Complete
*/
void *Net::read_queue_key_until_timeout(int key, int *length_out) {
    if (NetDirectPlay != 0 && key != 0) {
        const DWORD start = timeGetTime();
        while (timeGetTime() - start < field_6FC_) {
            if (queue_10c_.get(NetGetScratch, key, 0, length_out) != 0) {
                return NetGetScratch;
            }
        }
    }
    return 0;
}

/*
Purpose: Pump DirectPlay receives until one fails: the reader loop the
         receive thread lives in, dispatching through
         IDirectPlay2::Receive on the shared interface.

         The flag argument selects 6 when the player id is non-zero and 3
         when it is not; the size slot starts from Net+0x6E4's capacity
         and is handed by address so the callee can shrink it.

// ORIGINAL: 0x00630B80 sub_630b80 0x00630B80-0x00630BE3 BYTE_EXACT
// symbol    ?receive_directplay_messages@Net@@QAEXH@Z
// Historical emitted-symbol hint: @sub_630b80@12
// size      99 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00630BD4
Return Value: n/a
Status: Complete
*/
void Net::receive_directplay_messages(int player_id) {
    if (NetDirectPlay != 0) {
        DPID working_id;
        HRESULT result;
        do {
            // The native loop restores the input player id before each call,
            // even though Receive writes through this same local.
            DWORD size = field_6E4_;
            DPID counterpart = local_player_id_;
            working_id = static_cast<DPID>(player_id);
            if (player_id == 0) {
                result = NetDirectPlay->Receive(&working_id, &counterpart, 3,
                                                NetGetScratch, &size);
            } else {
                result = NetDirectPlay->Receive(&working_id, &counterpart, 6,
                                                NetGetScratch, &size);
            }
        } while (result == 0);
    }
}

/*
Purpose: Flush Net's whole player table as one packet: when the table's two
         bounds coincide, pack every player record (id, name, flags) into a
         0x28-stride array prefixed with the count and hand it to
         Net::send_packet_type as a 0x10 broadcast.

         The allocation and transmitted length are count*0x2C+4, not the
         packed records' count*0x28+4. Only the low count byte is written;
         preserve the other count bytes and oversized tail untouched.

         The sixteen 0x58-byte NetPlayer records begin at Net+0x154; the
         walk reads each record's first word, its name at +0x15 and its
         flags byte at +0x14, exactly as the player record is laid out.

// ORIGINAL: 0x0062D5F0 sub_62d5f0 0x0062D5F0-0x0062D69D BYTE_EXACT
// symbol    ?broadcast_player_table@Net@@QAEXXZ
// TRIED: calling strcat by name inlines it (repne scasb), which the original does not; the pointer-cast call site was a byte-convergence lever, dropped for this coverage landing.
// Historical emitted-symbol hint: @sub_62d5f0@8
// size      173 bytes
// prototype
// callers   1   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4510 0x0062F8A0 0x00644EF2 0x00645470
Return Value: n/a
Status: Complete
*/
void Net::broadcast_player_table() {
    if (local_player_id_ != host_player_id_) {
        return;
    }
    const int count = static_cast<int>(field_6DC_);
    if (count == 0) {
        return;
    }
    const int size = count * 0x2C + 4;
    unsigned char *const packet = static_cast<unsigned char *>(mem_get(size));
    *packet = static_cast<unsigned char>(count);
    for (int index = 0; index < count; index++) {
        reinterpret_cast<NetPeerPlayerRecord *>(packet + 4)[index].id_ = players_[index].field_0_;
        reinterpret_cast<NetPeerPlayerRecord *>(packet + 4)[index].name_[0] = 0;
        players_[index].name_and_tail_[0x1F] = 0;
        strcat(reinterpret_cast<NetPeerPlayerRecord *>(packet + 4)[index].name_, players_[index].name_and_tail_);
        reinterpret_cast<NetPeerPlayerRecord *>(packet + 4)[index].flags_ = players_[index].flags_14_;
    }
    send_packet_type(packet, size, 0, 0x10, 1);
    if (packet != 0) {
        free(packet);
    }
}

/*
Purpose: Set or clear a player's "host" flag through DirectPlay: read the
         current per-player data word with IDirectPlay2::GetPlayerData,
         flip bit 1 according to the argument, and write it back.

         The original uses slot 0x74 after a successful GetPlayerData, passing
         the player id, one-byte value, size 1 and flags 2. This owner
         expresses the observed call through the typed DirectPlay API;
         its HRESULT becomes the method result; a failed get is
         returned unchanged and suppresses the set call.

// ORIGINAL: 0x00632290 sub_632290 0x00632290-0x006322FC BYTE_EXACT
// symbol    ?set_player_host_flag@Net@@QAEHH@Z
// Historical emitted-symbol hint: @sub_632290@12
// size      108 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
// indirect  0x006322BD 0x006322F2
Return Value: the DirectPlay GetPlayerData failure or SetPlayerData result; 0 when DirectPlay is absent
Status: Complete
*/
int Net::set_player_host_flag(int is_host) {
    int result = reinterpret_cast<int>(NetDirectPlay);
    if (NetDirectPlay != 0) {
        unsigned char data;
        DWORD size = 1;
        result = NetDirectPlay->GetPlayerData(local_player_id_, &data, &size, 0);
        if (result == 0) {
            if (is_host != 0) {
                data = static_cast<unsigned char>(data | 2);
            } else {
                data = static_cast<unsigned char>(data & 0xFD);
            }
            result = NetDirectPlay->SetPlayerData(local_player_id_, &data, 1, 2);
        }
    }
    return result;
}

/*
Purpose: Net::check_message - remember a player's message stamp, answering
         whether the stamp moved. The dead null-receiver branch keeps the
         MessageBox the original would show if the player table could ever
         be null.

// ORIGINAL: 0x00632FA0 ?check_message@Net@@QAEHKK@Z 0x00632FA0-0x00633007
// symbol    ?check_message@Net@@QAEHKK@Z
// TRIED: do-while loop form (same #21 jne/je divergence); reordering the i==0x10 branch did not change it.
// Historical emitted-symbol hint: ?net_check_message_redirect@@YIHPAX0KK@Z
// size      103 bytes
// prototype int (__thiscall ?check_message@Net@@QAEHKK@Z)(Net* this, unsigned int, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
// indirect  0x00632FB6
Return Value: 1 when the stamp already covers this message (or the table is
              null), 0 when the stamp advanced
Status: Complete
*/
int Net::check_message(unsigned long player, unsigned long stamp) {
    NetPlayer *record = &players_[0];
    if (record == 0) {
        MessageBoxA(nullptr, "Help in net!!", "NO message_stamp!", 0);
        return 1;
    }
    int slot;
    for (slot = 0; slot < 0x10; ++slot, ++record) {
        if (record->field_0_ == player) {
            break;
        }
    }
    if (slot == 0x10) {
        return 0;
    }
    uint32_t *last = &players_[slot].field_4_;
    if (stamp > *last) {
        *last = stamp;
        return 0;
    }
    return 1;
}

// ORIGINAL: 0x00632300 ?set_polling@Net@@QAEXH@Z 0x00632300-0x0063234F BYTE_EXACT
// size      80 bytes
// prototype void (__thiscall ?set_polling@Net@@QAEHH@Z)(Net* this, int)
// callers   2   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// Sixteen per-player polling stamps at 0x164 - players_[i].stamp_10_, 0x58
// apart - drop to zero; the polling bit in the 0xD8 flag word follows the
// argument.
void Net::set_polling(int flag) {
  for (int i = 0; i < 0x10; ++i) {
    players_[i].stamp_10_ = 0;
  }
  if (flag != 0) {
    voice_flags_D8_ |= 0x40000;
  } else {
    voice_flags_D8_ &= 0xFFFBFFFF;
  }
}

// ---------------------------------------------------------------------------
// THE SESSION LOCK TOGGLES. close_game and enable_join are one body twice:
// only the DPSESSION_JOINDISABLED bit they store differs. Both gate on the
// 0x10000 bit of the 0xD8 flag word - the "joined a lobby session" state
// check_for_lobby lights - because the round-trip is a lobby-session call.
// ---------------------------------------------------------------------------

/*
Purpose: Close the joined session to further joins: read the session
         description back through the interface, light DPSESSION_JOINDISABLED
         in its flags dword, and store it again.
// ORIGINAL: 0x00631B30 ?close_game@Net@@QAEXXZ 0x00631B30-0x00631B9A FILE BYTE_EXACT
// size      106 bytes
// prototype void (__thiscall ?close_game@Net@@QAEXXZ)(Net* this)
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4510 0x00644EF2
// indirect  0x00631B51 0x00631B71 0x00631B87
// Homed from src/recovered/units/00631b30.cpp.
Return Value: n/a
Status: Coverage transcription - unclaimed body
*/
void Net::close_game() {
    if (NetDirectPlay == 0 || (voice_flags_D8_ & 0x10000) == 0) {
        return;
    }
    // Size pass, then the real read: GetSessionDesc(NULL, &size) answers the
    // needed byte count without touching a buffer.
    unsigned long size;
    NetDirectPlay->GetSessionDesc(0, &size);
    DPSESSIONDESC2 *desc = static_cast<DPSESSIONDESC2 *>(mem_get(size));
    NetDirectPlay->GetSessionDesc(desc, &size);
    // The flags dword at desc+4: the whole dword is read back, bit 5 set, and
    // the dword stored - a byte `or` inside a dword read-modify-write.
    desc->dwFlags |= DPSESSION_JOINDISABLED;
    NetDirectPlay->SetSessionDesc(desc, 0);
    if (desc != 0) {
        free(desc);
    }
}

/*
Purpose: Reopen the joined session to joins - close_game's mirror, clearing
         DPSESSION_JOINDISABLED by the same read-modify-store round trip.
// ORIGINAL: 0x00632070 ?enable_join@Net@@QAEXXZ 0x00632070-0x006320DA FILE BYTE_EXACT
// size      106 bytes
// prototype void (__thiscall ?enable_join@Net@@QAEXXZ)(Net* this)
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4510 0x00644EF2
// Homed from src/recovered/units/00632070.cpp, retired 2026-09-27 by
// record-then-retire: this body was fresh-measured 42/42 BYTE_EXACT in this
// TU and the claim now banks here.
Return Value: n/a
Status: Complete
*/
void Net::enable_join() {
    if (NetDirectPlay == 0 || (voice_flags_D8_ & 0x10000) == 0) {
        return;
    }
    unsigned long size;
    NetDirectPlay->GetSessionDesc(0, &size);
    DPSESSIONDESC2 *desc = static_cast<DPSESSIONDESC2 *>(mem_get(size));
    NetDirectPlay->GetSessionDesc(desc, &size);
    // Bit 5 of the low byte, exactly like close_game sets it - `and al, 0xdf`.
    desc->dwFlags &= ~DPSESSION_JOINDISABLED;
    NetDirectPlay->SetSessionDesc(desc, 0);
    if (desc != 0) {
        free(desc);
    }
}

// GetPlayerName at 632EDB receives original 9BE4EC with capacity 0x100;
// 632EF1 reads the resulting DPNAME::lpszLongNameA at +0xC. The response
// includes provider-written trailing text, not just the sixteen-byte header.
// This is the observed API capacity, not a claim of the complete original
// allocation extent. Failed calls leave the previous response visible.
union NetPlayerNameBuffer {
    DPNAME name_;
    unsigned char bytes_[0x100];
};
// RECOVERY_OBJECT: v1 0x009BE4EC _NetPlayerNameRecord
static NetPlayerNameBuffer NetPlayerNameRecord;

// Original player insertion is bound by the coordinator's native writer.
// ORIGINAL: 0x00632CC0 sub_632cc0 0x00632CC0-0x00632DAA BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:c96a47e3f89e8792c8514ac2ca79259ab6f42c3e:src/unrecovered/00632cc0.cpp", "Historical seed spelling sub_632cc0; current source mapping sub_632cc0; emitted symbol ?sub_632cc0@Net@@QAEHPADHE@Z.", "Pinned632CE8/632CED skips slot initialization at index16 but continues632D2B count/broadcast and632D4F callback. The fixed 0x58-byte row formula computes ID at receiver+6D4 and name at receiver+6E9; these alias existing field_6D4_ and one byte into guid_6E8_. No seventeenth row is allocated.", "Pinned632D6E reloads the callback-visible row ID before JackalVoiceRx::add_player632D7D."], "reason": "Recover player insertion through the real sixteen-row owner and preserve the native full-table fallthrough and fresh callback-visible ID read. The C++ index-16 member access remains outside the declared array while retaining the original computed addresses; historical safety rejection remains in Git for the later improvement milestone. Original spans, claims and seed findings remain; no exactness or acceptance claim."}
// TRIED: bare `strcat` intrinsic-expands to strlen(scasb)+memcpy; `#pragma function(strcat)` forces the real call. ecx is `this` (not the stdcall the IDA guess implied) - rewritten as a thiscall member of a throwaway class taking (char*, int, unsigned char), matching the "ret 0xc" 3-stack-arg cleanup. do-while / for over the 16-slot search loop UNROLLS the first iteration (duplicated body) under /O2, which the original does not show; `while(true){...; if(cond) break;}` keeps a single instance matching the original's layout, but still emits `jge` where the original has `jl` at the loop's back edge - flipping the break condition's polarity didn't change it. 84% mnemonic match, single divergence region.
// symbol    ?sub_632cc0@Net@@QAEHPADHH@Z
int Net::sub_632cc0(char *text, int id, int flag) {
    int index = 0;
    while (index < 16) {
        const uint32_t row_id = players_[index].field_0_;
        if (row_id == 0) break;
        if (row_id == static_cast<uint32_t>(id)) {
            return static_cast<int>(row_id);
        }
        ++index;
    }

    if (index < 16) {
        players_[index].field_0_ = static_cast<uint32_t>(id);
        players_[index].flags_14_ = flag;
        players_[index].name_and_tail_[0] = 0;
        strcat(players_[index].name_and_tail_, text);
        if ((flag & 1) != 0) host_player_id_ = static_cast<uint32_t>(id);
    }

    ++field_6DC_;
    if (local_player_id_ == host_player_id_) broadcast_player_table();
    // The native callback indexes the same 0x58-byte row formula even when
    // index is 16. That preserves the original full-table read from +6D4 and
    // +6E9; it remains beyond the declared players_ array at that index.
    on_player_added(players_[index].name_and_tail_, players_[index].field_0_);
    // The virtual notification may have changed the ID (0x632D6E).
    JackalVoiceRx &receivers = jackal_voice_rx_;
    receivers.add_player(static_cast<int>(players_[index].field_0_));

    VoiceRxAllocation *voice;
    if (receivers.head_ != 0) {
        voice = receivers.cursor_->payload_;
    } else {
        voice = 0;
    }
    return voice->init(0x1000, 0, 0);
}

namespace {
// Peer table payload after the eight-byte envelope: count DWORD then a
// variable-length row tail. The local producer sends at most sixteen rows,
// but the original receiver does not impose that bound on incoming packets.
struct NetPeerPlayerTableMessage {
    NetPacketEnvelope envelope_;
    unsigned char count_;
    unsigned char unused_9_[3];
    NetPeerPlayerRecord *records() {
        return reinterpret_cast<NetPeerPlayerRecord *>(this + 1);
    }
};
struct NetPeerDropPlayerMessage {
    NetPacketEnvelope envelope_;
    DPID id_;
};
static_assert(sizeof(NetPeerPlayerTableMessage) == 0xC, "peer snapshot header extent");
static_assert(offsetof(DPMSG_CREATEPLAYERORGROUP, lpData) == 0x10, "create data field");
static_assert(offsetof(DPMSG_SETPLAYERORGROUPDATA, lpData) == 0xC, "player data field");
static_assert(offsetof(DPMSG_SETPLAYERORGROUPNAME, dpnName) == 0xC, "changed name header");
static_assert(offsetof(DPNAME, lpszLongNameA) == 0xC, "ANSI long name field");
static_assert(sizeof(MMIOINFO) == 0x48, "voice packet MMIOINFO extent");
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x006327E0 ?internal_receive@Net@@QAAHPAUNetThreadInfo@@@Z 0x006327E0-0x00632CB3
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/006327e0.cpp"], "reason": "Recover the original explicit NetThreadInfo pointer and cdecl caller cleanup. Native6327F6 reads entry ESP+4;632787/632794 push the record and add ESP,4 after return. The prior intentional thiscall refactor is superseded; no historical class membership is inferred."}
// symbol    ?internal_receive@@YAHPAUNetThreadInfo@@@Z
int __cdecl internal_receive(NetThreadInfo *thread_info) {
    if (NetDirectPlay == 0) return 0;
    Net *const net = thread_info->owner_;

    // 6327FD retains this pointer across Receive, even if the provider changes
    // the record's buffer member while filling the originally supplied block.
    unsigned char *const bytes = static_cast<unsigned char *>(thread_info->buffer_);
    DWORD from = 0;
    DWORD size = net->field_6E4_;
    DWORD to = net->local_player_id_;
    const HRESULT receive_result = thread_info->direct_play_->Receive(
        &from, &to, 3, bytes, &size);
    if (receive_result == DP_OK) {
        if (from == 0 || (bytes[0] & 0x80) != 0) {
            net->queue_130_.add(bytes, size, from, 0,
                                static_cast<unsigned long>(-1));
            PostThreadMessageA(thread_info->thread_id_, 0x402, 0, 0);
            SetEvent(thread_info->events_[1]);
            return receive_result == DP_OK;
        }

        size -= 0x0C;
        NetPacketEnvelope *const packet =
            reinterpret_cast<NetPacketEnvelope *>(bytes);
        JackalVoiceRx &receivers = net->jackal_voice_rx_;
        int index;
        switch (packet->type_flags_ & 0xFFFB) {
        case 0x10:
            net->queue_130_.add(bytes, size + 0x0C, from, 0,
                                static_cast<unsigned long>(-1));
            PostThreadMessageA(thread_info->thread_id_, 0x402, 0, 0);
            SetEvent(thread_info->events_[1]);
            break;
        case 1:
            if (static_cast<int32_t>(net->queue_10c_.count_) < 0xC8) {
                net->queue_10c_.add(bytes + 8, size, from, 0,
                                    static_cast<unsigned long>(-1));
                SetEvent(thread_info->events_[4]);
            }
            break;
        case 0x20:
            for (index = 0; index < 16; ++index) {
                if (net->players_[index].field_0_ == from) break;
            }
            if (index != 16) net->players_[index].stamp_10_ = timeGetTime();
            break;
        case 0x200:
            if (net->check_message(from, packet->sequence_4_) == 0) {
                net->voice_flags_D8_ &= 0xBFFFFFFF;
                if (receivers.head_ != 0) {
                    receivers.cursor_index_ = 0;
                    receivers.cursor_ = receivers.head_;
                    for (index = 0; index < receivers.count_; ++index) {
                        if (receivers.cursor_->id_ == static_cast<int>(from)) {
                            // Incoming 0x200 calls slot +88 (start), not the
                            // Sound stop slot, despite clearing this flag.
                            (receivers.head_ != 0
                                ? receivers.cursor_->payload_ : 0)->start();
                            break;
                        }
                        ++receivers.cursor_index_;
                        receivers.cursor_ = receivers.cursor_->next_;
                    }
                }
            }
            break;
        case 0x100: {
            MMIOINFO voice_packet;
            memset(&voice_packet, 0, sizeof(voice_packet));
            voice_packet.cchBuffer = static_cast<LONG>(size);
            voice_packet.pchBuffer = reinterpret_cast<HPSTR>(bytes + 8);
            voice_packet.pchNext = reinterpret_cast<HPSTR>(bytes + 8);
            voice_packet.pchEndRead =
                reinterpret_cast<HPSTR>(bytes + 8 + size);
            if (receivers.head_ != 0) {
                receivers.cursor_index_ = 0;
                receivers.cursor_ = receivers.head_;
                for (index = 0; index < receivers.count_; ++index) {
                    if (receivers.cursor_->id_ == static_cast<int>(from)) {
                        // Original rechecks head, then still calls through a
                        // null payload if that recheck fails.
                        VoiceRxAllocation *const voice = receivers.head_ != 0
                            ? receivers.cursor_->payload_ : 0;
                        const int level = voice->get_nbuffers();
                        if (static_cast<int>((net->field_6DC_ - 1) * level) < 0xC8) {
                            voice->add_buffer(&voice_packet);
                        }
                        if ((get_sound_version() & 1) != 0) {
                            if (level > 4 && voice->is_playing() == 0) {
                                voice->start();
                            }
                        } else if (level > 15 && voice->is_playing() == 0) {
                            voice->start();
                        }
                        break;
                    }
                    ++receivers.cursor_index_;
                    receivers.cursor_ = receivers.cursor_->next_;
                }
            }
            break;
        }
        }

        switch (packet->type_flags_ & 6) {
        case 2:
            if (packet->sequence_4_ == net->field_E4_) {
                for (index = 0; index < 16; ++index) {
                    if (from == net->players_[index].field_0_) break;
                }
                if (index < 16) net->players_[index].field_8_ = 0;
                for (index = 0; index < 16; ++index) {
                    if (net->players_[index].field_0_ != 0 &&
                        net->players_[index].field_8_ != 0) {
                        return receive_result == DP_OK;
                    }
                }
                SetEvent(thread_info->events_[0]);
            }
            break;
        case 4:
            if ((packet->type_flags_ & 0xFFF0) == 0 &&
                net->check_message(from, packet->sequence_4_) == 0) {
                net->queue_10c_.add(bytes + 8, size, from, 0, timeGetTime());
                SetEvent(thread_info->events_[4]);
            }
            // Queue publication/event callbacks can alter the shared packet WORD.
            packet->type_flags_ = (packet->type_flags_ & 0xFFFB) | 2;
            thread_info->direct_play_->Send(net->local_player_id_, to, 0,
                                            bytes, 0x0C);
            return receive_result == DP_OK;
        }
    }
    return receive_result == DP_OK;
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// Historical seed observations (not fresh measurements or acceptance):
// TRIED: EnterCriticalSection called only once per iteration - calling through `[dword ptr g_0066917c]` directly (not cached in a local like leaveCS/wait, which are each called twice) got the prologue's push/mov schedule to within 1 byte of the original.
// ORIGINAL: 0x00632740 sub_632740 0x00632740-0x006327D2
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/00632740.cpp"], "reason": "Rehome seed catalogue identity and original extents into compiled owning class; native-emitted symbol is separate and no new byte claim granted."}
// symbol    ?net_receive_thread@@YAXPAX@Z
static void __cdecl net_receive_thread(void *argument) {
    NetThreadInfo *const thread_info =
        static_cast<NetThreadInfo *>(argument);
    HANDLE events[2];
    events[0] = thread_info->events_[2];
    events[1] = thread_info->events_[3];
    DWORD wait_result = WaitForMultipleObjects(2, events, FALSE, INFINITE);
    if (wait_result == WAIT_OBJECT_0) {
        do {
            EnterCriticalSection(&thread_info->receive_lock_);
            if (thread_info->direct_play_ == 0) {
                LeaveCriticalSection(&thread_info->receive_lock_);
                goto end_thread;
            }
            while (internal_receive(thread_info) != 0) {
            }
            LeaveCriticalSection(&thread_info->receive_lock_);
            wait_result =
                WaitForMultipleObjects(2, events, FALSE, INFINITE);
        } while (wait_result == WAIT_OBJECT_0);
        _endthread();
        return;
    }
end_thread:
    _endthread();
}

/*
Purpose: Drain the system-message queue: one 0x28-byte record at a time off
         the third fifo, each either a DPSYS_* envelope the image keys on the
         queue's routing key or a peer packet keyed on its first byte.

           key != 0 - a routed peer message:
             byte 0 bit 0x80 set            -> cut the named player;
             byte 0 bit 0x10 set            -> refresh the join list, one
                                               0x28-byte entry per record;
           key == 0 - a system envelope, by the type byte:
             3    both timer words agree and the payload says so
                  -> re-read the player's name and refresh one join-list entry;
             5    -> cut the named player (reason 1);
             0x31 -> the session was lost: raise it and close the connection;
             0x102 -> stamp the named player's slot byte from the message;
             0x103 -> the player's name changed (virtual slot 3).

         The receive scratch doubles as the message body: peer flags are a
         byte, but system messages have the SDK's DWORD type. Peer rows and
         system envelopes are distinct views of the shared receive bytes.
// ORIGINAL: 0x00632DB0 ?handle_sys_msg@Net@@QAEXXZ 0x00632DB0-0x00632F97 FILE BYTE_EXACT
// size      487 bytes
// prototype void (__thiscall ?handle_sys_msg@Net@@QAEXXZ)(Net* this)
// callers   7   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0062E010 0x00632350 0x00632CC0 0x00633F70
// indirect  0x00632EDB 0x00632F0C 0x00632F34
// Homed from src/recovered/units/00632db0.cpp.
Return Value: n/a
Status: Coverage transcription - unclaimed body
*/
void Net::handle_sys_msg() {
    if (NetDirectPlay == 0) {
        return;
    }
    unsigned int key;
    // The pop-the-head read: the routing key comes out with the message, and
    // the scratch receives the body either way.
    while (queue_130_.get(NetGetScratch, &key, 0,
                          static_cast<unsigned int *>(0)) != 0) {
        // These casts are the received-message ABI boundary, not raw object
        // member offsets or executable-address aliases. The source-owned
        // scratch is shared and must be re-read after external callbacks.
        if (key != 0) {
            const unsigned char flags = NetGetScratch[0];
            if ((flags & 0x80) != 0) {
                const NetPeerDropPlayerMessage *const message =
                    reinterpret_cast<const NetPeerDropPlayerMessage *>(NetGetScratch);
                drop_player(message->id_, 1);
            } else if ((flags & 0x10) != 0) {
                NetPeerPlayerTableMessage *const message =
                    reinterpret_cast<NetPeerPlayerTableMessage *>(NetGetScratch);
                // Original632E3D reloads the count byte after each insertion
                // (byte test on entry); the loop index is an integer.
                for (int seen = 0; seen < message->count_; ++seen) {
                    // The image passes the whole flags DWORD (int parameter).
                    sub_632cc0(message->records()[seen].name_,
                               message->records()[seen].id_,
                               message->records()[seen].flags_);
                }
            }
            continue;
        }
        const DPMSG_GENERIC *const generic =
            reinterpret_cast<const DPMSG_GENERIC *>(NetGetScratch);
        switch (generic->dwType) {
        case 3:
            // Refresh one player's name: only while the two timer words name
            // the same target and the payload word agrees to re-read it.
            if (local_player_id_ == host_player_id_) {
                const DPMSG_CREATEPLAYERORGROUP *const message =
                    reinterpret_cast<const DPMSG_CREATEPLAYERORGROUP *>(NetGetScratch);
                if (message->dwPlayerType != 1) break;
                unsigned long size = 0x100;
                NetDirectPlay->GetPlayerName(message->dpId,
                                             NetPlayerNameRecord.bytes_, &size);
                unsigned char *text =
                    static_cast<unsigned char *>(message->lpData);
                int first = text != 0 ? text[0] : 0;
                sub_632cc0(NetPlayerNameRecord.name_.lpszLongNameA, message->dpId,
                    first);
            }
            break;
        case 5: {
            const DPMSG_DESTROYPLAYERORGROUP *const message =
                reinterpret_cast<const DPMSG_DESTROYPLAYERORGROUP *>(NetGetScratch);
            if (message->dwPlayerType == 1) {
                drop_player(message->dpId, 1);
            }
            break;
        }
        case 0x31:
            // The host moved on or the session died: raise it, then close.
            on_session_lost();
            close();
            break;
        case 0x102: {
            const DPMSG_SETPLAYERORGROUPDATA *const message =
                reinterpret_cast<const DPMSG_SETPLAYERORGROUPDATA *>(NetGetScratch);
            // Stamp the named player's slot byte from the message's own text.
            int index = 0;
            while (index < 0x10 && players_[index].field_0_ != message->dpId) {
                ++index;
            }
            if (index == 0x10) {
                break;
            }
            unsigned char *text =
                static_cast<unsigned char *>(message->lpData);
            if (text == 0) {
                break;
            }
            players_[index].flags_14_ = text[0];
            break;
        }
        case 0x103: {
            const DPMSG_SETPLAYERORGROUPNAME *const message =
                reinterpret_cast<const DPMSG_SETPLAYERORGROUPNAME *>(NetGetScratch);
            on_player_name_changed(message->dpnName.lpszLongNameA, message->dpId);
            break;
        }
        default:
            break;
        }
    }
}


/*
Purpose: Construct the node's list block through its real C++ owner.
         The shared allocation virtual base captures the original heap owner;
         this constructor clears the five list words.
// ORIGINAL: 0x00401B80 sub_401b80 0x00401B80-0x00401BD8 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:2f89503ec251ffc8b9ff894998c01944c173a4fb:src/recovered/units/00401b80.cpp"], "reason": "Retain the genuine source constructor, including natural inherited and virtual-base lifetime; superseded assembly scaffold is historical evidence, not another implementation."}
// compile_context src/alphanet_records.cpp
// body src/alphanet_inline.h
// symbol    ??0NetTextListBase@@QAE@XZ
// size      88 bytes
Constructor implemented through the declared C++ base/member lifecycle; matching remains independently measurable.
*/




/*
// ORIGINAL: 0x00401BE0 sub_401be0 0x00401BE0-0x00401C79 BYTE_EXACT
// symbol    ??1NetTextListBase@@UAE@XZ
// size      153 bytes
// prototype
// callers   3   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
// indirect  0x00401C22 0x00401C34 0x00401C50
Status: Complete
*/
// The receiver arrives in ECX pointing 0x1c into the real object - a
// virtual-base subobject - so the body first recovers the object's own
// start (`self`). It then writes two vtable pointers: its own, and a
// second one reached through a displacement read out of a small table at
// self+4 (a vbtable: entry [1] holds the byte offset from self+4 to the
// virtual base's vtable slot, because that offset is only known once the
// most-derived class is). The same "read a vtable, take its [1] entry as
// a THIS-adjustment rather than a function, add it to the pointer, then
// call through the adjusted object's own vtable[0]" shape recurs for
// every node torn down in the loop below - that is the scalar deleting
// destructor called through a pointer typed as a virtual base, which is
// why the adjustment has to be data instead of a compile-time constant.
//
// MISMATCH, 52/64 (0.984 similar), best over every flag set this tree
// tries. The one surviving divergence is the SECOND per-node teardown
// call (deleting the list node itself, `node` below): the image computes
// the adjusted `this` with `lea ecx,[edx+eax]` and then re-derives the
// target vtable via `[edx+eax]` addressing instead of reading `[ecx]`,
// where the FIRST teardown call (on `sub_obj`, same shape) compiles
// byte-exact with `add`+`[ecx]`. TRIED: a named `int *vtbl`/`int
// disp` pair, a fused single expression, and routing the call through
// `vtable_method` all reproduce the identical `add`+`[ecx]` form as the
// first call - the allocator's choice between the two forms tracks
// register pressure at the call site, not source spelling, and no
// spelling tried moves it. Everything else - both vtable-pointer writes,
// the loop's trip count re-read each iteration, the first per-node call,
// and the trailing field clears - is byte-exact.
class SubBase401be0 { public:
    virtual void slot0(int);   // vtable[0]: scalar-deleting-destructor style
    virtual void slot1(int);   // vtable[4]: notifies `self` a node is going
};

#pragma optimize("y", off)
NetTextListBase::~NetTextListBase() {
    close(); // also when empty; current_ is NOT cleared
}
#pragma optimize("", on)







// NetUnk72C::~NetUnk72C: body in net_class.h (inlined by ~Net).



/*
Purpose: Copy one text record's string into fresh storage chosen by the
         record's own allocator (the CRT heap when none is installed) and
         hand the copy plus the four metadata words to the caller's array.
// ORIGINAL: 0x005D47D0 sub_5d47d0 0x005D47D0-0x005D4851 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original 5D47D0-5D4851 and its retained seed/history preserve identity and extent.", "Provider callback 630BF0 borrows name and provider GUID through ServiceList +1C/+20 before add; receiver +18 selects Heap versus mem_get. Payload owns copied name +4 and GUID +8. These are ServiceList/ServiceStruct roles, not the sibling nested-text owner.", "Native v3 emits the genuine virtual ServiceList insertion symbol; source nominal names remain evidence-backed interpretations, not new byte claims."], "reason": "Recover provider-list insertion as ServiceList::on_insert on the real enumerated provider payload."}
// Historical superseded binding interpretation: {"evidence": ["Original5D47D0..5D4851; retired seed git:b18d3ac8:src/unrecovered/005d47d0.cpp records the same identity and spans.", "The receiver is the +0x18 Heap pointer, +0x1C string and +0x20 metadata record the sibling 0x005D4C50 installs into; mem_get is general.cpp's and Heap::get is heap.h's own, both already linked by this TU's list machinery."], "reason": "Coverage transcription of the record copy-out with the thiscall receiver spelled as an explicit first argument; no in-tree class owns the record yet, so its slots stay documented offsets."}
// symbol    ?on_insert@ServiceList@@UAEHPAUServiceStruct@@@Z
Status: Coverage transcription - unclaimed body
*/
int ServiceList::on_insert(ServiceStruct *payload) {
    const char *text = name_;
    if (text != 0) {
        char *copy;
        if (allocator_ != 0) {
            const size_t length = strlen(text) + 1;
            copy = static_cast<char *>(allocator_->get(length));
        } else {
            copy = static_cast<char *>(mem_get(strlen(text) + 1));
        }
        payload->name_ = copy;
        if (copy == 0) return 4;
        *copy = 0;
        // Native re-reads the borrowed input after allocation and the output
        // pointer after publication; neither can be cached across the helper.
        strcat(payload->name_, name_);
    }
    // Four scalar words copied in the native hook; GUID value assignment may
    // lower as a block copy and therefore needs native reference measurement.
    payload->provider_guid_ = *provider_guid_;
    return 0;
}


/*
Purpose: Copy one text record's string into fresh storage chosen by the
         record's own allocator and park the copy in the caller's output
         block; the metadata half is left to the caller.
// ORIGINAL: 0x005D4C50 sub_5d4c50 0x005D4C50-0x005D4CBC FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original 5D4C50-5D4CBC and its retained seed/history preserve identity and extent.", "The nested list receiver uses Heap +18 and borrowed text +1C to copy text into payload +4 through mem_get or Heap::get. Genuine NetUnk72C and NetTextPayload declarations replace the old raw receiver facade.", "The unseen factory, original nominal payload identity and complete allocation extent remain unresolved; observed prefix and delete protocol do not identify them."], "reason": "Recover NetUnk72C::on_insert for its dedicated observed-prefix NetTextPayload role."}
// Historical superseded binding interpretation: {"evidence": ["Original5D4C50..5D4CBC; retired seed git:b18d3ac8:src/unrecovered/005d4c50.cpp records the same identity and spans.", "Same +0x18/+0x1C receiver record as sub_5d47d0 above, with mem_get and Heap::get resolving exactly as there; the seed's function-pointer CRT spellings are ordinary strlen/strcat calls here."], "reason": "Coverage transcription of the record copy-out's string half with the receiver as an explicit first argument."}
// symbol    ?on_insert@NetUnk72C@@UAEHPAVNetTextPayload@@@Z
Status: Coverage transcription - unclaimed body
*/
int NetUnk72C::on_insert(Payload *payload) {
    payload->field_8_ = 0;
    if (source_text_ != 0) {
        if (allocator_ != 0) {
            size_t const length = strlen(source_text_) + 1;
            // Native 5D4C73 reloads allocator_ AFTER the strlen call.
            payload->text = static_cast<char *>(allocator_->get(length));
        } else {
            payload->text = static_cast<char *>(mem_get(strlen(source_text_) + 1));
        }
        if (payload->text == 0) {
            return 4;
        }
        *payload->text = 0;
        strcat(payload->text, source_text_);
    }
    // Null source does NOT clear payload->text; no unseen payload tail is named.
    return 0;
}

// ORIGINAL: 0x0062E540 ?join_session@Net@@QAEHPAUSessionStruct@@PADPAD@Z 0x0062E540-0x0062EA97;0x006634DF-0x006634FF
// kind      game
// symbol    ?join_session@Net@@QAEHPAVNetAllocationNode@@PAD1@Z
// TRIED: real Font locals (SEH-frame lever) + COM-style dispatch through the provided ComSlot004/006/014/022/024 shims, retyped to their real arities, plus an added ComSlot031; diverges at insn #8 of this SEH-wrapped, DirectPlay-COM-heavy ~1400-byte body
// The image keeps the name test inside the outer guard (0x0062E5C3): an
// inline helper boundary.
static inline void free_if_set(void *memory) {
    if (memory != 0) free(memory);
}

int Net::join_session(NetAllocationNode *session, char *player_name,
                      char *password) {
    // Both Font subobjects are constructed in the native prologue and remain
    // alive across the version warning paths (including their cleanup label).
    Font older_version_font;
    Font older_version_shadow;

    {
        if (NetDirectPlay == 0 || player_name == 0) {
            goto fail;
        }

        if (session != 0) {
            if (allocation_node_.session_name_ != 0
                && &allocation_node_ != session) {
                free_if_set(allocation_node_.session_name_);
                allocation_node_.session_name_ = 0;
            }
            free_if_set(allocation_node_.password_);
            allocation_node_.password_ = 0;

            DPSESSIONDESC2 requested;
            memset(&requested, 0, sizeof(requested));
            requested.dwSize = sizeof(requested);
            requested.guidInstance = session->instance_guid_;
            requested.guidApplication = *guid_6E8_;
            requested.lpszPasswordA =
                password != 0 && password[0] != 0 ? password : 0;

            const HRESULT opened = NetDirectPlay->Open(&requested, DPOPEN_JOIN);
            if (opened == DP_OK) {
                memset(&NetServiceCaps, 0, sizeof(NetServiceCaps));
                NetServiceCaps.dwSize = sizeof(NetServiceCaps);
                if (NetDirectPlay->GetCaps(&NetServiceCaps, 1) != DP_OK) {
                    goto fail;
                }
            } else if (opened != DPERR_ALREADYINITIALIZED) {
                goto fail;
            }
        }

        DWORD descriptor_size = 0;
        NetDirectPlay->GetSessionDesc(0, &descriptor_size);
        DPSESSIONDESC2 *descriptor = static_cast<DPSESSIONDESC2 *>(
            mem_get(static_cast<int>(descriptor_size)));
        // Native control reaches the common return without freeing this block on
        // either allocation/read failure or a version mismatch. Keep that
        // lifetime defect visible; successful version comparison frees it below.
        if (descriptor == 0
            || NetDirectPlay->GetSessionDesc(descriptor, &descriptor_size) != DP_OK) {
            goto fail;
        }

        const int local_version = static_cast<int>(field_44_);
        const int remote_version = static_cast<int>(descriptor->dwUser1);
        if (local_version != remote_version) {
            parse_num(0, local_version);
            parse_num(1, remote_version);
            if (local_version < remote_version) {
                older_version_font.init("Times New Roman", 0xE, 0);
                older_version_shadow.init("Times New Roman", 0xE, 2);
                pop_caption("jackal.txt", "CLIENTOLDERVERSION", 0);
            } else {
                older_version_font.init("Times New Roman", 0xE, 0);
                older_version_shadow.init("Times New Roman", 0xE, 2);
                pop_caption("jackal.txt", "CLIENTNEWERVERSION", 0);
            }
            NetDirectPlay->Close();
            goto fail;
        }
        free(descriptor);

        DPNAME player_descriptor;
        memset(&player_descriptor, 0, sizeof(player_descriptor));
        player_descriptor.dwSize = sizeof(player_descriptor);
        player_descriptor.lpszLongNameA = player_name;

        DWORD player_data = (voice_flags_D8_ >> 0x10) & 1;
        if ((voice_flags_D8_ & 2) != 0) {
            player_data |= 2;
        }

        // process.h declares exactly void (__cdecl *)(void *); NetThreadInfo is
        // the evidenced Net+4 record passed to the worker.
        _beginthread(&net_receive_thread, 0, &thread_info_);

        if (NetDirectPlay->CreatePlayer(
                &local_player_id_, &player_descriptor, thread_info_.events_[2],
                &player_data, 1, 0)
            != DP_OK) {
            goto fail;
        }

        const DWORD host_flag = voice_flags_D8_ & 0x10000;
        if (host_flag != 0) {
            host_player_id_ = local_player_id_;
        }
        // The SDK callback can change the process-wide interface. The native
        // host descriptor update tests the live pointer after CreatePlayer.
        if (NetDirectPlay != 0 && host_flag != 0) {
            DWORD live_size = 0;
            NetDirectPlay->GetSessionDesc(0, &live_size);
            DPSESSIONDESC2 *live = static_cast<DPSESSIONDESC2 *>(
                mem_get(static_cast<int>(live_size)));
            NetDirectPlay->GetSessionDesc(live, &live_size);
            live->dwFlags |= DPSESSION_JOINDISABLED;
            NetDirectPlay->SetSessionDesc(live, 0);
            if (live != 0) {
                free(live);
            }
        }

        if (local_player_id_ == host_player_id_) {
            sub_632cc0(player_name, static_cast<int>(local_player_id_),
                       static_cast<unsigned char>(player_data & 0xFF));
        }

        DWORD live_size = 0;
        NetDirectPlay->GetSessionDesc(0, &live_size);
        DPSESSIONDESC2 *live = static_cast<DPSESSIONDESC2 *>(
            mem_get(static_cast<int>(live_size)));
        if (live == 0) {
            goto fail;
        }
        if (NetDirectPlay->GetSessionDesc(live, &live_size) != DP_OK) {
            free(live);
            goto fail;
        }

        if (live->lpszSessionNameA == 0) {
            // The image calls mem_get(1) and discards its result on this branch.
            mem_get(1);
            allocation_node_.session_name_ = 0;
        } else {
            const size_t length = strlen(live->lpszSessionNameA) + 1;
            allocation_node_.session_name_ =
                static_cast<char *>(mem_get(static_cast<int>(length)));
            *allocation_node_.session_name_ = 0;
            strcat(allocation_node_.session_name_, live->lpszSessionNameA);
        }
        if (password != 0 && password[0] != 0) {
            const size_t length = strlen(password) + 1;
            allocation_node_.password_ =
                static_cast<char *>(mem_get(static_cast<int>(length)));
            *allocation_node_.password_ = 0;
            strcat(allocation_node_.password_, password);
        }
        allocation_node_.instance_guid_ = live->guidInstance;
        allocation_node_.session_flags_ = live->dwFlags;
        allocation_node_.max_players_ = live->dwMaxPlayers;
        free(live);

        const DWORD start_time = timeGetTime();
        if (field_6DC_ == 0) {
            while (timeGetTime() - start_time < 10000 && field_6DC_ == 0) {
                do_task();
            }
            if (field_6DC_ == 0) {
                close();
                goto fail;
            }
        }
        return static_cast<int>(local_player_id_);
    }
fail:
    return 0;
}



void NetAllocationNode::operator delete(void *memory, Heap *) {
    cleanup_failed_net_construction(memory);
}
void NetAllocationNode::operator delete(void *memory, AlphaNetStringStruct *) {
    cleanup_failed_net_construction(memory);
}
void NetAllocationNode::operator delete(void *memory) {
    cleanup_failed_net_construction(memory);
}

void *ServiceStruct::operator new(size_t size, Heap *owner) throw() {
    return allocate_net_node(size, owner);
}
inline void *ServiceStruct::operator new(size_t size, ServiceProviderList *list) throw() {
    // Three tiers in the image (0x401780..0x4017F6): the list's explicit
    // allocator, then its inherited allocation owner, then mem_get. exit is
    // not noreturn here, so a failed tier falls into the next one.
    void *memory;
    Heap *allocator = list->allocator_;
    if (allocator != 0) {
        memory = allocator->get(size);
        if (memory != 0) {
            StringAllocationHeap = allocator;
            return memory;
        }
        MessageBoxA(0, "out of heap space in new function", "Aborting", MB_OK);
        exit(3);
    }
    Heap *owner = list->allocation_owner_;
    if (owner != 0) {
        memory = owner->get(size);
        if (memory != 0) {
            StringAllocationHeap = owner;
            return memory;
        }
        MessageBoxA(0, "out of heap space in new function", "Aborting", MB_OK);
        exit(3);
    }
    memory = mem_get(size);
    if (memory == 0) return 0;
    StringAllocationHeap = owner;
    return memory;
}
void ServiceStruct::operator delete(void *memory, Heap *) { cleanup_failed_net_construction(memory); }
void ServiceStruct::operator delete(void *memory, ServiceProviderList *) { cleanup_failed_net_construction(memory); }
void ServiceStruct::operator delete(void *memory) { cleanup_failed_net_construction(memory); }

void *ServiceListEntry::operator new(size_t size, Heap *owner) throw() {
    return allocate_net_node(size, owner);
}
inline void *ServiceListEntry::operator new(size_t size, ServiceProviderList *list) throw() {
    // Three tiers in the image (0x401780..0x4017F6): the list's explicit
    // allocator, then its inherited allocation owner, then mem_get. exit is
    // not noreturn here, so a failed tier falls into the next one.
    void *memory;
    Heap *allocator = list->allocator_;
    if (allocator != 0) {
        memory = allocator->get(size);
        if (memory != 0) {
            StringAllocationHeap = allocator;
            return memory;
        }
        MessageBoxA(0, "out of heap space in new function", "Aborting", MB_OK);
        exit(3);
    }
    Heap *owner = list->allocation_owner_;
    if (owner != 0) {
        memory = owner->get(size);
        if (memory != 0) {
            StringAllocationHeap = owner;
            return memory;
        }
        MessageBoxA(0, "out of heap space in new function", "Aborting", MB_OK);
        exit(3);
    }
    memory = mem_get(size);
    if (memory == 0) return 0;
    StringAllocationHeap = owner;
    return memory;
}
void ServiceListEntry::operator delete(void *memory, Heap *) { cleanup_failed_net_construction(memory); }
void ServiceListEntry::operator delete(void *memory, ServiceProviderList *) { cleanup_failed_net_construction(memory); }
void ServiceListEntry::operator delete(void *memory) { cleanup_failed_net_construction(memory); }

#pragma optimize("y", off)
ServiceProviderList::ServiceProviderList() {
    head_ = 0;
    current_ = 0;
    entry_count_ = 0;
    current_position_ = 0;
    allocator_ = 0;
}
#pragma optimize("", on)


int ServiceProviderList::on_insert(ServiceStruct *) { return 0; }

void ServiceProviderList::on_remove(ServiceStruct *) { }

// ServiceProviderList::seek_pos is inline in net_class.h; next_entry lives in
// alphanet_records.cpp (choose_service and pick_service call it out of line).

// Original401760 installs6693C4 at each entry's allocation base (+0x14).
// Its deleting slot targets402C20, shared with StringStructEntry. The real
// constructions below emit this additional deleting entry; the canonical
// ORIGINAL and historical claim remain in stringstruct.cpp.
// RECOVERY_SHARED: v1 0x00402C20 0x00402C62 ??_GServiceListEntry@@UAEPAXI@Z 0x00401760
#pragma optimize("y", off)
int ServiceProviderList::add(int id) {
    if (entry_count_ == 0) {
        head_ = new(this) ServiceListEntry;
        if (head_ == 0) return 4;
        head_->id_ = id;
        head_->next_ = head_;
        head_->previous_ = head_;
        head_->payload_ = new(this) ServiceStruct;
        if (head_->payload_ == 0) return 4;
        on_insert(head_->payload_);
        current_ = head_;
    } else {
        head_->previous_->next_ = new(this) ServiceListEntry;
        if (head_->previous_->next_ == 0) return 4;
        head_->previous_->next_->previous_ = head_->previous_;
        head_->previous_->next_->next_ = head_;
        head_->previous_ = head_->previous_->next_;
        current_ = head_->previous_;
        current_->id_ = id;
        current_->payload_ = new(this) ServiceStruct;
        if (current_->payload_ == 0) return 4;
        on_insert(current_->payload_);
    }
    current_position_ = entry_count_;
    ++entry_count_;
    return 0;
}
#pragma optimize("", on)


// ServiceProviderList::~ServiceProviderList: body in alphanet_inline.h.


void ServiceList::on_remove(ServiceStruct *payload) {
    if (allocator_ == 0 && payload->name_ != 0) {
        free(payload->name_);
        payload->name_ = 0;
    }
}

// ServiceList::~ServiceList lives in alphanet.cpp: choose_service (0x00630D70)
// calls it out of line, while pick_service (0x004E26E0) expands it.


BOOL __stdcall enumerate_session_callback(LPCDPSESSIONDESC2 descriptor,
                                          LPDWORD, DWORD, LPVOID) {
    if (descriptor == 0) return FALSE;
    // Native captures these descriptor inputs before publishing any into the
    // borrowed ledger, then calls insertion and ignores its status entirely.
    const char *password = descriptor->lpszPasswordA;
    const DWORD current = descriptor->dwCurrentPlayers;
    const char *name = descriptor->lpszSessionNameA;
    const DWORD maximum = descriptor->dwMaxPlayers;
    const DWORD flags = descriptor->dwFlags;
    AlphaNetLedger *ledger = SessionEnumTarget;
    ledger->session_name_ = name;
    ledger->instance_guid_ = &descriptor->guidInstance;
    ledger->session_flags_ = flags;
    ledger->max_players_ = maximum;
    ledger->current_players_ = current;
    ledger->password_ = password;
    ledger->add(0);
    return TRUE;
}

/*
Purpose: Construct the derived node list through its real base lifecycle.
         The inherited list constructor clears its five words; the derived
         source-text and field_20_ words remain uninitialized.
// ORIGINAL: 0x00402D10 sub_402d10 0x00402D10-0x00402D7C FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:2f89503ec251ffc8b9ff894998c01944c173a4fb:src/recovered/units/00402d10.cpp"], "reason": "Retain the genuine source constructor, including natural inherited and virtual-base lifetime; superseded assembly scaffold is historical evidence, not another implementation."}
// compile_context src/alphanet_records.cpp
// body src/alphanet_inline.h
// symbol    ??0NetUnk72C@@QAE@XZ
// size      108 bytes
Constructor implemented through the declared C++ base/member lifecycle; matching remains independently measurable.
*/



// RECOVERY_SHARED: v1 0x00404250 0x00404255 ?on_insert@NetTextListBase@@UAEHPAVNetTextPayload@@@Z 0x00401B80
int NetTextListBase::on_insert(Payload *) { return 0; }

// RECOVERY_SHARED: v1 0x00404270 0x00404273 ?on_remove@NetTextListBase@@UAEXPAVNetTextPayload@@@Z 0x00401B80
void NetTextListBase::on_remove(Payload *) { }

void NetUnk72C::on_remove(Payload *payload) {
    if (allocator_ == 0) {
        if (payload->text != 0) {
            free(payload->text);
            payload->text = 0;
        }
    }
}

int NetTextListBase::seek_pos(int position) {
    typedef int (__cdecl *AbsFunction)(int);
    AbsFunction abs_function = &abs;
    if (position > entry_count_ - 1) return 0;
    current_ = head_;
    if (position < 0) {
        if (abs_function(position) > entry_count_) return 0;
        int steps = abs_function(position);
        if (steps > 0) {
            do {
                --steps;
                current_ = current_->previous;
            } while (steps != 0);
        }
        current_position_ = position + entry_count_;
    } else {
        int steps = position;
        if (steps > 0) {
            do {
                --steps;
                current_ = current_->next;
            } while (steps != 0);
        }
        current_position_ = position;
    }
    return 1;
}

int NetTextListBase::current_id() {
    return head_ != 0 ? current_->id : 0;
}

NetTextListBase::Payload *NetTextListBase::current_entry() {
    return head_ != 0 ? current_->payload : 0;
}

NetTextListBase::Payload *NetTextListBase::next_entry() {
    if (head_ == 0) return 0;
    Entry *const next = current_->next;
    int position = current_position_;
    current_ = next;
    int const count = entry_count_;
    ++position;
    current_position_ = position;
    if (position == count) current_position_ = 0;
    return current_->payload;
}

/*
Purpose: The this-8 teardown one StringStruct-shaped container owner runs:
         restore the outer vtable stages, run the list's remove_all, walk
         the remaining entries releasing each payload through the node's
         own slot-1 notify and slot-0 deleting dispatch, then hand the
         saved heap head back to 0x009B3374.

// ORIGINAL: 0x00402DD0 sub_402dd0 0x00402DD0-0x00402ED2;0x006505A0-0x006505B5 FILE BYTE_EXACT
// TRIED: this is the "StringStruct-shaped member destructor" wall this project already documents (see 0x00421100's ProdPicker dtor and 0x004E3300's ClassX::sub_4e3300, both same idiom): a this-adjusted (`this-8`) inline teardown of an embedded container - vtable-patch, walk entries calling two indirect "scalar deleting destructor" style virtual calls per entry (slot001(payload) on the node's own patched vtable, then slot000(1) through a this-adjustor delta read from each item's own vtable+4) - followed by restoring the outer object's vtable. The BODY-mode scaffold's auto-generated `VCall` shim is nullary and can't carry the arguments these calls need, so this is landed FILE-mode with `VCall` edited to `slot000(int)`/`slot001(void*)`. Not expected exact: the compiler-synthesized SEH try-frame (fs:[0] chain, `push -1; push handler`) that protects partial teardown is not reproduced by a call-by-call translation over raw pointers with no real local object for the compiler to protect (mnemonic_similarity 0.81, 204 vs 258 primary-span bytes).
// size      279 bytes
// prototype
// callers   4   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00402970
// symbol   ??1NetAllocationNode@@UAE@XZ
Status: Compiler-generated. NetAllocationNode declares no destructor; the
        implicit one destroys entries_ without restaging the node's own
        table, as the image does.
*/




/*
// ORIGINAL: 0x00402320 sub_402320 0x00402320-0x00402349 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:360abf0bcf225dda7b384e46e5c26fb7bd2a3bda:src/recovered/00402320.cpp", "pinned:terranx_original:0x402320", "source:src/net_class.cpp"], "reason": "Original cleanup preserves StringAllocationHeap on null input; otherwise free only when owner is null, then clear handoff. Real Net/AlphaNet placement-delete callers discard its result. Indirect return consumers remain unverified. Preserve all original spans, names, claims and seed discoveries; no new byte grant."}
// symbol    ?cleanup_failed_net_construction@@YAXPAX@Z
*/
#pragma optimize("y", off)
void cleanup_failed_net_construction(void *memory) {
    if (memory != 0) {
        if (StringAllocationHeap == 0) free(memory);
        StringAllocationHeap = 0;
    }
    // Native 402320 leaves the handoff untouched when memory is null.
}
#pragma optimize("", on)


// ORIGINAL: 0x0062F8A0 ?send_packet_type@Net@@QAEHPAXHKHH@Z 0x0062F8A0-0x0063007F
// RECOVERY_BINDING: v1 {"evidence": ["task:net-transport-owner", "pinned:terranx_original.exe:0062F8A0 complete direct/reliable targeted/broadcast control flow", "source:src/net_class.h:NetPlayer acknowledgement +8 and RTT +C; existing FIFO and thread event owners", "historical seed:src/unrecovered/0062f8a0.cpp"], "reason": "Recover the complete sender through genuine Net peer state, queues, event handles, SDK DirectPlay and allocator ownership; preserve native timeout error behavior and all original seed obligations without a new byte claim."}
// TRIED: MISMATCH #0 'sub' vs 'push' (prologue diverges; best sim 0.44). Net::send_packet_type - reliable-UDP-style send with per-peer RTT tracking, retry, and a "who's stalling" prompt on timeout. Used the scaffold's own COM-style dispatch hint for slot 26 (the DirectPlay Send call pushes the interface as an explicit arg rather than passing it in ecx) - declared a local function- pointer typedef under a name that does NOT collide with the scaffold's own nullary `ComSlot026`, since redeclaring that symbol with a different signature is a hard conflict, not an override. Hoisted `buf`, `allocSize` and `ackFlag` to function scope: MSVC 6 (C2362) refuses a `goto` that skips a later local's initializer even when the type is POD, and this control flow's early special-case return has to jump past the buffer-allocation block entirely.
#if defined(_MSC_VER) && _MSC_VER == 1200
#pragma function(memcpy)
#endif
int Net::send_packet_type(void *data, int data_size, unsigned long target,
                          int packet_type, int flags) {
    HANDLE wait_handles[2];
    DWORD sent_at[16];
    DWORD retry_delay[16];
    int pending[16];
    int index;
    wait_handles[0] = thread_info_.events_[0];
    wait_handles[1] = thread_info_.events_[1];

    if (NetDirectPlay == 0 || data_size < 0 ||
        data_size > static_cast<int>(field_6E4_)) {
        return 0;
    }
    check_polling();

    if ((packet_type & 0xFFF0) == 0 && (flags & 4) != 0) {
        if (target == local_player_id_) {
            queue_10c_.add(data, static_cast<unsigned long>(data_size),
                           local_player_id_, 0, 0xFFFFFFFFUL);
            return static_cast<int>(field_E4_);
        }
        if (target == 0) {
            queue_10c_.add(data, static_cast<unsigned long>(data_size),
                           local_player_id_, target, 0xFFFFFFFFUL);
            if (static_cast<int>(field_6DC_) < 2)
                return static_cast<int>(field_E4_);
        }
    }

    const int allocation_size = data_size + 0x0C;
    unsigned char *const packet =
        static_cast<unsigned char *>(mem_get(allocation_size));
    if (packet == 0) return 0;

    NetPacketEnvelope *const header =
        reinterpret_cast<NetPacketEnvelope *>(packet);
    if ((flags & 1) != 0) {
        header->sequence_4_ = ++field_E4_;
    } else {
        header->sequence_4_ = 0;
    }
    if (data != 0) memcpy(packet + 8, data, static_cast<size_t>(data_size));

    if ((flags & 1) != 0 && (voice_flags_D8_ & 0x10000000U) == 0) {
        header->type_flags_ = static_cast<WORD>(packet_type | 4);
        DWORD wait_timeout;
        DWORD interval_start;
        if (target != 0) {
            wait_timeout = 0xFFFFFFFFU;
            int found = 0;
            int selected;
            for (index = 0; index < 16; ++index) {
                if (players_[index].field_0_ == target &&
                    ((players_[index].flags_14_ & 2) != 0 ||
                     (packet_type & 0x10) != 0)) {
                    retry_delay[index] = players_[index].field_C_;
                    found = 1;
                    players_[index].field_8_ = 1;
                    selected = index;
                    if (players_[index].field_C_ < wait_timeout)
                        wait_timeout = players_[index].field_C_;
                } else {
                    players_[index].field_8_ = 0;
                }
            }
            if (found == 0) goto done;

            sent_at[selected] = timeGetTime();
            if (NetDirectPlay->Send(local_player_id_, static_cast<DPID>(target), 0,
                                    packet, static_cast<DWORD>(allocation_size)) != DP_OK) {
                MessageBoxA(0, "Oh no!! Send failed!",
                            "Message from the Net Underworld", 0);
                free(packet);
                return 0;
            }
            for (;;) {
                interval_start = timeGetTime();
                while (timeGetTime() - interval_start < 20000) {
                    ResetEvent(thread_info_.events_[1]);
                    ResetEvent(thread_info_.events_[0]);
                    if (WaitForMultipleObjects(2, wait_handles, FALSE,
                                               wait_timeout) == WAIT_OBJECT_0 + 1)
                        handle_sys_msg();
                    if (players_[selected].field_8_ == 0 ||
                        players_[selected].field_0_ == 0) {
                        players_[selected].field_C_ = (timeGetTime() +
                            players_[selected].field_C_ - sent_at[selected] + 0x14) >> 1;
                        goto done;
                    }
                    if (timeGetTime() - sent_at[selected] > retry_delay[selected]) {
                        retry_delay[selected] = (retry_delay[selected] * 3) >> 1;
                        if (NetDirectPlay == 0) { free(packet); return 0; }
                        sent_at[selected] = timeGetTime();
                        if (NetDirectPlay->Send(local_player_id_,
                                players_[selected].field_0_, 0, packet,
                                static_cast<DWORD>(allocation_size)) != DP_OK) {
                            MessageBoxA(0, "Oh no!! Send failed!",
                                        "Message from the Net Underworld", 0);
                            free(packet);
                            return 0;
                        }
                    }
                }

                StringTemp[0] = 0;
                int name_index;
                for (name_index = 0; name_index < 16; ++name_index) {
                    if (players_[name_index].field_0_ == players_[selected].field_0_)
                        break;
                }
                // The native failed lookup passes null to strcat.
                strcat(StringTemp, name_index == 16
                    ? 0 : players_[name_index].name_and_tail_);
                strcat(StringTemp, ".");
                parse_says(0, StringTemp, -1, -1);
                switch (pop_caption(const_cast<char *>("jackal"),
                                    const_cast<char *>("NET_SENDTIMEDOUT"), 0)) {
                case 0:
                    break;
                case 1:
                    close();
                    free(packet);
                    return 0;
                case 2:
                    ExitTurnLoop = 1;
                    free(packet);
                    return 0;
                default:
                    free(packet);
                    return 0;
                }
                sent_at[selected] = timeGetTime();
                if (NetDirectPlay->Send(local_player_id_, static_cast<DPID>(target), 0,
                                        packet, static_cast<DWORD>(allocation_size)) != DP_OK) {
                    MessageBoxA(0, "Oh no!! Send failed!",
                            "Message from the Net Underworld", 0);
                free(packet);
                return 0;
                }
            }
        }

        int any_pending = 0;
        wait_timeout = 0xFFFFFFFFU;
        for (index = 0; index < 16; ++index) {
            if (players_[index].field_0_ != 0 &&
                players_[index].field_0_ != local_player_id_ &&
                ((players_[index].flags_14_ & 2) != 0 || (packet_type & 0x10) != 0)) {
                players_[index].field_8_ = 1;
                pending[index] = 1;
                retry_delay[index] = players_[index].field_C_;
                any_pending = 1;
                if (players_[index].field_C_ < wait_timeout)
                    wait_timeout = players_[index].field_C_;
            } else {
                players_[index].field_8_ = 0;
                pending[index] = 0;
            }
        }
        if (any_pending == 0) goto done;

        while (NetDirectPlay != 0) {
            sent_at[0] = timeGetTime();
            for (index = 1; index < 16; ++index) sent_at[index] = sent_at[0];
            if (NetDirectPlay->Send(local_player_id_, 0, 0, packet,
                                    static_cast<DWORD>(allocation_size)) != DP_OK) {
                MessageBoxA(0, "Oh no!! Send failed!",
                            "Message from the Net Underworld", 0);
                free(packet);
                return 0;
            }
            interval_start = timeGetTime();
            while (timeGetTime() - interval_start < 20000) {
                ResetEvent(thread_info_.events_[1]);
                ResetEvent(thread_info_.events_[0]);
                if (WaitForMultipleObjects(2, wait_handles, FALSE,
                                           wait_timeout) == WAIT_OBJECT_0 + 1)
                    handle_sys_msg();
                const DWORD now = timeGetTime();
                int still_pending = 0;
                for (index = 0; index < 16; ++index) {
                    if (pending[index] == 0) continue;
                    if (players_[index].field_0_ == 0) {
                        pending[index] = 0;
                    } else if (players_[index].field_8_ == 0) {
                        const DWORD ack_tick = timeGetTime();
                        pending[index] = 0;
                        players_[index].field_C_ = (ack_tick +
                            players_[index].field_C_ - sent_at[index] + 0x14) >> 1;
                    } else {
                        if (now - sent_at[index] > retry_delay[index]) {
                            retry_delay[index] = (retry_delay[index] * 3) >> 1;
                            if (NetDirectPlay == 0) { free(packet); return 0; }
                            sent_at[index] = timeGetTime();
                            if (NetDirectPlay->Send(local_player_id_,
                                    players_[index].field_0_, 0, packet,
                                    static_cast<DWORD>(allocation_size)) != DP_OK) {
                                MessageBoxA(0, "Oh no!! Send failed!",
                                            "Message from the Net Underworld", 0);
                                free(packet);
                                return 0;
                            }
                        }
                        still_pending = 1;
                    }
                }
                if (still_pending == 0) goto done;
            }

            StringTemp[0] = 0;
            int first_name = 1;
            for (index = 0; index < 16; ++index) {
                if (players_[index].field_8_ == 0) continue;
                int name_index;
                for (name_index = 0; name_index < 16; ++name_index) {
                    if (players_[name_index].field_0_ == players_[index].field_0_)
                        break;
                }
                strcat(StringTemp, name_index == 16
                    ? 0 : players_[name_index].name_and_tail_);
                if (!first_name) {
                    strcat(StringTemp, ", ");
                    strcat(StringTemp, " ");
                }
                first_name = 0;
            }
            strcat(StringTemp, ".");
            parse_says(0, StringTemp, -1, -1);
            switch (pop_caption(const_cast<char *>("jackal"),
                                const_cast<char *>("NET_SENDTIMEDOUT"), 0)) {
            case 0:
                continue;
            case 1:
                close();
                break;
            case 2:
                ExitTurnLoop = 1;
                break;
            }
            MessageBoxA(0, "Oh no!! Send failed after many tries!",
                        "Message from the Net Underworld", 0);
            break;
        }
        free(packet);
        return 0;
    } else {
        header->type_flags_ = static_cast<WORD>(packet_type);
        if (NetDirectPlay == 0) { free(packet); return 0; }
        if (NetDirectPlay->Send(local_player_id_, static_cast<DPID>(target),
                                (flags & 1) != 0, packet,
                                static_cast<DWORD>(allocation_size)) != DP_OK) {
            free(packet);
            return 0;
        }
    }
    done:
    free(packet);
    return static_cast<int>(field_E4_);
}
#if defined(_MSC_VER) && _MSC_VER == 1200
#pragma intrinsic(memcpy)
#endif


// ORIGINAL: 0x00630080 ?send@Net@@QAEHPAXHKH@Z 0x00630080-0x00630978
// RECOVERY_BINDING: v1 {"evidence": ["task:net-transport-owner", "pinned:terranx_original.exe:00630080 complete direct/reliable targeted/broadcast control flow", "source:src/net_class.h:NetPlayer acknowledgement +8 and RTT +C; existing FIFO and thread event owners", "historical seed:src/unrecovered/00630080.cpp"], "reason": "Recover the complete sender through genuine Net peer state, queues, event handles, SDK DirectPlay and allocator ownership; preserve native timeout error behavior and all original seed obligations without a new byte claim."}
// TRIED: Ghidra-structure translation with the parameter mapping re-derived from raw ESP tracking (Ghidra's param_2..5 are thiscall's stack args a1..a4, one off from its own printed __thiscall param_1), COM-style vtable slot 26 dispatch retyped to 6 args per the scaffold's ComSlot026 note, and Net::close/check_polling/handle_sys_msg and NetFifo::add receivers fixed (Ghidra flattened those away too); compiles, MISMATCH #3 (early - likely the this->ebp store-back-for-stashed-handles at the very top).
#if defined(_MSC_VER) && _MSC_VER == 1200
#pragma function(memcpy)
#endif
int Net::send(void *data, int data_size, unsigned long target, int flags) {
    HANDLE wait_handles[2];
    DWORD sent_at[16];
    DWORD retry_delay[16];
    int pending[16];
    int index;
    // Native snapshots the wait array at function entry.
    wait_handles[0] = thread_info_.events_[0];
    wait_handles[1] = thread_info_.events_[1];
    if (data == 0 || NetDirectPlay == 0) {
        MessageBoxA(0, "Oh no! NULL pointer!!", "Net::send", 0);
        return 0;
    }
    if (data_size <= 0 || data_size > static_cast<int>(field_6E4_)) {
        MessageBoxA(0, "Oh no! data_size is weird!", "Net::send", 0);
        return 0;
    }
    check_polling();

    if ((voice_flags_D8_ & 1) != 0 || (flags & 4) != 0) {
        if (target == local_player_id_) {
            queue_10c_.add(data, static_cast<unsigned long>(data_size),
                           local_player_id_, 0, 0xFFFFFFFFUL);
            return static_cast<int>(field_E4_);
        }
        if (target == 0) {
            queue_10c_.add(data, static_cast<unsigned long>(data_size),
                           local_player_id_, target, 0xFFFFFFFFUL);
            if (static_cast<int>(field_6DC_) < 2)
                return static_cast<int>(field_E4_);
        }
    }

    const int allocation_size = data_size + 0x0C;
    unsigned char *const packet =
        static_cast<unsigned char *>(mem_get(allocation_size));
    if (packet == 0) return 0;
    NetPacketEnvelope *const header =
        reinterpret_cast<NetPacketEnvelope *>(packet);
    if ((flags & 1) != 0) {
        header->sequence_4_ = ++field_E4_;
    } else {
        header->sequence_4_ = 0;
    }
    memcpy(packet + 8, data, static_cast<size_t>(data_size));

    if ((flags & 1) != 0 && (voice_flags_D8_ & 0x10000000U) == 0) {
        header->type_flags_ = 4;
        DWORD wait_timeout;
        DWORD interval_start;
        if (target != 0) {
            int found = 0;
            int selected;
            for (index = 0; index < 16; ++index) {
                if (players_[index].field_0_ == target &&
                    (players_[index].flags_14_ & 2) != 0) {
                    retry_delay[index] = players_[index].field_C_;
                    selected = index;
                    players_[index].field_8_ = 1;
                    found = 1;
                } else {
                    players_[index].field_8_ = 0;
                }
            }
            if (found == 0) {
                goto done;
            }

            sent_at[selected] = timeGetTime();
            if (NetDirectPlay->Send(local_player_id_, static_cast<DPID>(target), 0,
                                    packet, static_cast<DWORD>(allocation_size)) != DP_OK) {
                MessageBoxA(0, "Oh no!! Send failed!",
                            "Message from the Net Underworld", 0);
                free(packet);
                return 0;
            }
            for (;;) {
                wait_timeout = players_[selected].field_C_;
                interval_start = timeGetTime();
                while (timeGetTime() - interval_start < 20000) {
                    ResetEvent(thread_info_.events_[1]);
                    ResetEvent(thread_info_.events_[0]);
                    if (WaitForMultipleObjects(2, wait_handles, FALSE,
                                               wait_timeout) == WAIT_OBJECT_0 + 1)
                        handle_sys_msg();
                    if (players_[selected].field_8_ == 0 ||
                        players_[selected].field_0_ == 0) {
                        players_[selected].field_C_ = (timeGetTime() +
                            players_[selected].field_C_ - sent_at[selected] + 0x14) >> 1;
                        goto done;
                    }
                    if (timeGetTime() - sent_at[selected] > retry_delay[selected]) {
                        retry_delay[selected] = (retry_delay[selected] * 3) >> 1;
                        if (NetDirectPlay == 0) {
                            free(packet);
                            return 0;
                        }
                        sent_at[selected] = timeGetTime();
                        if (NetDirectPlay->Send(local_player_id_,
                                players_[selected].field_0_, 0, packet,
                                static_cast<DWORD>(allocation_size)) != DP_OK) {
                            MessageBoxA(0, "Oh no!! Send failed!",
                                        "Message from the Net Underworld", 0);
                            free(packet);
                            return 0;
                        }
                    }
                }

                StringTemp[0] = 0;
                int name_index;
                for (name_index = 0; name_index < 16; ++name_index) {
                    if (players_[name_index].field_0_ == players_[selected].field_0_)
                        break;
                }
                // The native failed lookup passes null to strcat.
                strcat(StringTemp, name_index == 16
                    ? 0 : players_[name_index].name_and_tail_);
                strcat(StringTemp, ".");
                parse_says(0, StringTemp, -1, -1);
                switch (pop_caption(const_cast<char *>("jackal"),
                                    const_cast<char *>("NET_SENDTIMEDOUT"), 0)) {
                case 0:
                    break;
                case 1:
                    close();
                    free(packet);
                    return 0;
                case 2:
                    ExitTurnLoop = 1;
                    free(packet);
                    return 0;
                default:
                    free(packet);
                    return 0;
                }
                sent_at[selected] = timeGetTime();
                if (NetDirectPlay->Send(local_player_id_, static_cast<DPID>(target), 0,
                                        packet, static_cast<DWORD>(allocation_size)) != DP_OK) {
                    MessageBoxA(0, "Oh no!! Send failed!",
                                "Message from the Net Underworld", 0);
                    free(packet);
                    return 0;
                }
            }
        }

        int any_pending = 0;
        wait_timeout = 0xFFFFFFFFU;
        for (index = 0; index < 16; ++index) {
            if (players_[index].field_0_ != 0 &&
                players_[index].field_0_ != local_player_id_ &&
                (players_[index].flags_14_ & 2) != 0) {
                players_[index].field_8_ = 1;
                pending[index] = 1;
                retry_delay[index] = players_[index].field_C_;
                any_pending = 1;
                if (players_[index].field_C_ < wait_timeout)
                    wait_timeout = players_[index].field_C_;
            } else {
                players_[index].field_8_ = 0;
                pending[index] = 0;
            }
        }
        if (any_pending == 0) {
            goto done;
        }

        while (NetDirectPlay != 0) {
            sent_at[0] = timeGetTime();
            for (index = 1; index < 16; ++index) sent_at[index] = sent_at[0];
            if (NetDirectPlay->Send(local_player_id_, 0, 0, packet,
                                    static_cast<DWORD>(allocation_size)) != DP_OK) {
                MessageBoxA(0, "Oh no!! Send failed!",
                            "Message from the Net Underworld", 0);
                free(packet);
                return 0;
            }
            interval_start = timeGetTime();
            while (timeGetTime() - interval_start < 20000) {
                ResetEvent(thread_info_.events_[1]);
                ResetEvent(thread_info_.events_[0]);
                if (WaitForMultipleObjects(2, wait_handles, FALSE,
                                           wait_timeout) == WAIT_OBJECT_0 + 1)
                    handle_sys_msg();
                const DWORD now = timeGetTime();
                int still_pending = 0;
                for (index = 0; index < 16; ++index) {
                    if (pending[index] == 0) continue;
                    if (players_[index].field_0_ == 0) {
                        pending[index] = 0;
                    } else if (players_[index].field_8_ == 0) {
                        const DWORD ack_tick = timeGetTime();
                        pending[index] = 0;
                        players_[index].field_C_ = (ack_tick +
                            players_[index].field_C_ - sent_at[index] + 0x14) >> 1;
                    } else {
                        if (now - sent_at[index] > retry_delay[index]) {
                            retry_delay[index] = (retry_delay[index] * 3) >> 1;
                            if (NetDirectPlay == 0) {
                                free(packet);
                                return 0;
                            }
                            sent_at[index] = timeGetTime();
                            if (NetDirectPlay->Send(local_player_id_,
                                    players_[index].field_0_, 0, packet,
                                    static_cast<DWORD>(allocation_size)) != DP_OK) {
                                MessageBoxA(0, "Oh no!! Send failed!",
                                            "Message from the Net Underworld", 0);
                                free(packet);
                                return 0;
                            }
                        }
                        still_pending = 1;
                    }
                }
                if (still_pending == 0) {
                    goto done;
                }
            }

            StringTemp[0] = 0;
            int first_name = 1;
            for (index = 0; index < 16; ++index) {
                if (players_[index].field_8_ == 0) continue;
                int name_index;
                for (name_index = 0; name_index < 16; ++name_index) {
                    if (players_[name_index].field_0_ == players_[index].field_0_)
                        break;
                }
                strcat(StringTemp, name_index == 16
                    ? 0 : players_[name_index].name_and_tail_);
                if (!first_name) {
                    strcat(StringTemp, ", ");
                    strcat(StringTemp, " ");
                }
                first_name = 0;
            }
            strcat(StringTemp, ".");
            parse_says(0, StringTemp, -1, -1);
            switch (pop_caption(const_cast<char *>("jackal"),
                                const_cast<char *>("NET_SENDTIMEDOUT"), 0)) {
            case 0:
                break;
            case 1:
                close();
                free(packet);
                return 0;
            case 2:
                ExitTurnLoop = 1;
                free(packet);
                return 0;
            default:
                free(packet);
                return 0;
            }
        }
        free(packet);
        return 0;
    }

    header->type_flags_ = 1;
    if (NetDirectPlay == 0) {
        free(packet);
        return 0;
    }
    if (NetDirectPlay->Send(local_player_id_, static_cast<DPID>(target),
                            (flags & 1) != 0, packet,
                            static_cast<DWORD>(allocation_size)) != DP_OK) {
        MessageBoxA(0, "Oh no!! Send failed!",
                    "Message from the Net Underworld", 0);
        free(packet);
        return 0;
    }
    done:
    free(packet);
    return static_cast<int>(field_E4_);
}
#if defined(_MSC_VER) && _MSC_VER == 1200
#pragma intrinsic(memcpy)
#endif
// ORIGINAL: 0x004E3710 sub_4e3710 0x004E3710-0x004E372B FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x004E3710..0x004E372A adjusts ECX by +0x58, calls 0x00402DD0, restores vptr 0x006693AC, republishes saved owner to 0x009B3374, and returns", "pinned Net, AlphaNet, and NetDaemon cleanup continuations pass the embedded complete NetAllocationNode receiver to 0x004E3710; helper reads no caller-frame state", "src/net_class.h declares NetAllocationNode virtually derives from StringAllocationBase, with vbtable 0x00669430 and base at +0x58; the captured allocator owner is at +4"], "reason": "Bind the pinned generated NetAllocationNode virtual-base cleanup entry to its genuine StringAllocationBase subobject and allocator-owner stage."}
// compile_context src/alphanet.cpp
// kind game
// symbol    ??_DNetAllocationNode@@QAEXXZ
// RECOVERY_GENERATED: v1 vbtable 0x006693DC 0x006693E4 ??_8ServiceProviderList@@7B@ 0x00401660
// RECOVERY_GENERATED: v1 vbtable 0x006693FC 0x00669404 ??_8NetTextListBase@@7B@ 0x00401B80
// RECOVERY_GENERATED: v1 vbtable 0x00669424 0x0066942C ??_8NetUnk72C@@7B@ 0x00402D10
// RECOVERY_GENERATED: v1 vtable 0x006693F0 ? ??_7NetTextListBase@@6BStringAllocationBase@@@ 0x00401B80
// RECOVERY_GENERATED: v1 vtable 0x006693F4 ? ??_7NetTextListBase@@6B0@@ 0x00401B80
// RECOVERY_GENERATED: v1 vtable 0x00669418 ? ??_7NetUnk72C@@6BStringAllocationBase@@@ 0x00402D10
// RECOVERY_GENERATED: v1 vtable 0x0066941C ? ??_7NetUnk72C@@6BNetTextListBase@@@ 0x00402D10
// RECOVERY_GENERATED: v1 vtable 0x006693D4 ? ??_7ServiceProviderList@@6B0@@ 0x00401660
// RECOVERY_GENERATED: v1 vtable 0x006693D0 ? ??_7ServiceProviderList@@6BStringAllocationBase@@@ 0x00401660
// RECOVERY_GENERATED: v1 vtable 0x0066EB20 ? ??_7ServiceList@@6BServiceProviderList@@@ 0x004E3730
// RECOVERY_GENERATED: v1 vtable 0x0066EB1C ? ??_7ServiceList@@6BStringAllocationBase@@@ 0x004E3730
// RECOVERY_GENERATED: v1 vbtable 0x00670DCC ? ??_8JackalVoiceRx@@7B@ 0x0062D6A0
// RECOVERY_GENERATED: v1 vtable 0x0066EAFC ? ??_7JackalVoiceRx@@6B0@@ 0x0062D6A0
// RECOVERY_GENERATED: v1 vtable 0x0066EAF8 ? ??_7JackalVoiceRx@@6BStringAllocationBase@@@ 0x0062D6A0
// RECOVERY_GENERATED: v1 vbtable 0x00669430 ? ??_8NetAllocationNode@@7B@ 0x0062D6A0
// RECOVERY_GENERATED: v1 vtable 0x0066942C ? ??_7NetAllocationNode@@6B@ 0x0062D6A0
