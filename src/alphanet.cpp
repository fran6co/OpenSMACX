/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */
#include "stdafx.h"
#include "original_seam.h"
#include "alphanet.h"
#include "spritebox.h"
#include "net_class.h"

#include "stringstruct.h"  // StringAllocationBaseVtable, StringAllocationHeap
#include "heap.h"
#include <new>      // Heap placement-construction at 0x148C

/*
Purpose: Convert a multiplayer process ID to its one-based player index.
// ORIGINAL: 0x004E25E0 ?pid_2_idx@AlphaNet@@QAEHK@Z 0x004E25E0-0x004E2607 BYTE_EXACT
// symbol    ?pid_2_idx@AlphaNet@@QAEHI@Z
// size      39 bytes
// prototype int (__thiscall ?pid_2_idx@AlphaNet@@QAEHK@Z)(AlphaNet* this, unsigned int)
// callers   20   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: Player index (1-7); process ID not found (0)
Status: Complete
*/
int AlphaNet::pid_2_idx(uint32_t process_id) {
    const uint8_t *bytes = reinterpret_cast<const uint8_t *>(this);
    for (int index = 1; index < 8; index++) {
        uint32_t candidate;
        memcpy(&candidate, bytes + 0x928 + (index - 1) * 0x19C, sizeof(candidate));
        if (candidate == process_id) {
            return index;
        }
    }
    return 0;
}

/*
Purpose: Convert a multiplayer process ID to its signed player identity.
// ORIGINAL: 0x004E2610 ?pid_2_who@AlphaNet@@QAEHK@Z 0x004E2610-0x004E2653 BYTE_EXACT
// symbol    ?pid_2_who@AlphaNet@@QAEHI@Z
// size      67 bytes
// prototype int (__thiscall ?pid_2_who@AlphaNet@@QAEHK@Z)(AlphaNet* this, unsigned int)
// callers   2   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: Player identity; process ID not found (0)
Status: Complete
*/
int AlphaNet::pid_2_who(uint32_t process_id) {
    const uint8_t *bytes = reinterpret_cast<const uint8_t *>(this);
    for (int index = 1; index < 8; index++) {
        uint32_t candidate;
        memcpy(&candidate, bytes + 0x928 + (index - 1) * 0x19C, sizeof(candidate));
        if (candidate == process_id) {
            int8_t identity;
            memcpy(&identity, bytes + 0x92C + (index - 1) * 0x19C, sizeof(identity));
            return identity;
        }
    }
    return 0;
}

/*
Purpose: Convert a signed multiplayer player identity to its process ID.
// ORIGINAL: 0x004E2660 ?who_2_pid@AlphaNet@@QAEHH@Z 0x004E2660-0x004E26A8 BYTE_EXACT
// size      72 bytes
// prototype int (__thiscall ?who_2_pid@AlphaNet@@QAEHH@Z)(AlphaNet* this, int)
// callers   4   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: Process ID; player identity not found (0)
Status: Complete
*/
int AlphaNet::who_2_pid(int identity) {
    const uint8_t *bytes = reinterpret_cast<const uint8_t *>(this);
    for (int index = 1; index < 8; index++) {
        int8_t candidate;
        memcpy(&candidate, bytes + 0x92C + (index - 1) * 0x19C, sizeof(candidate));
        if (candidate == identity) {
            int process_id;
            memcpy(&process_id, bytes + 0x928 + (index - 1) * 0x19C, sizeof(process_id));
            return process_id;
        }
    }
    return 0;
}

/*
Purpose: Convert a signed multiplayer player identity to its one-based index.
// ORIGINAL: 0x004E26B0 ?who_2_idx@AlphaNet@@QAEXH@Z 0x004E26B0-0x004E26DC BYTE_EXACT
// symbol    ?who_2_idx@AlphaNet@@QAEHH@Z
// size      44 bytes
// prototype void (__thiscall ?who_2_idx@AlphaNet@@QAEXH@Z)(AlphaNet* this, int)
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: Player index (1-7); player identity not found (0)
Status: Complete
*/
int AlphaNet::who_2_idx(int identity) {
    const uint8_t *bytes = reinterpret_cast<const uint8_t *>(this);
    for (int index = 1; index < 8; index++) {
        int8_t candidate;
        memcpy(&candidate, bytes + 0x92C + (index - 1) * 0x19C, sizeof(candidate));
        if (candidate == identity) {
            return index;
        }
    }
    return 0;
}






/*
Purpose: Clear every player's process slot and hand off to the network close.
// ORIGINAL: 0x004E25B0 ?close@AlphaNet@@QAEXXZ 0x004E25B0-0x004E25D8 BYTE_EXACT
// size      40 bytes
// prototype void (__thiscall ?close@AlphaNet@@QAEXXZ)(AlphaNet* this)
// callers   14   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Status: Complete with a temporary dependency on the network close
*/
// The ledger's staged tables - the image's .rdata, real storage here.
const uint32_t AlphaNetLedgerVbtable[2] = {0xFFFFFFFC, 0x00000034};   // 0x0066EAF0
const uint32_t AlphaNetStringStructVtable[1] = {0x00402F10};           // 0x00669408
const uint32_t AlphaNetListVirtualBaseVtable[1] = {0x00402C70};        // 0x00669404
const uint32_t AlphaNetListDerivedVtable[1] = {0x00402E40};            // 0x0066EAE8
const uint32_t AlphaNetListDerivedVirtualBaseVtable[1] = {0x00402C00}; // 0x0066EAE4


/*
Purpose: Build an AlphaNet: the Net base, the eight process slots, the
         StringList-family ledger at 0x144C, and the trailing Heap.
// ORIGINAL: 0x004E2490 ??0AlphaNet@@QAE@XZ 0x004E2490-0x004E25A5;0x0065C610-0x0065C630
// symbol    ??0AlphaNet@@QAE@XZ
// size      277 bytes
// kind      game
// PROMOTED from the archived transcription. The Net base constructs
// implicitly; the ledger's two-stage StringStruct/StringList staging and the
// Heap placement-construction are the live parts.
Return Value: n/a
Status: Complete
*/
AlphaNet::AlphaNet() {
    char *const self = reinterpret_cast<char *>(this);

    // Eight per-player process slots at 0x790, 0x19C stride: {FF, 0, 0, FF, 2}.
    for (int slot_i = 0; slot_i < 8; ++slot_i) {
        uint8_t *const slot = reinterpret_cast<uint8_t *>(self + 0x790 + slot_i * 0x19C);
        slot[0] = 0xFF;
        slot[1] = 0;
        slot[2] = 0;
        slot[3] = 0xFF;
        slot[4] = 2;
    }

    // The StringList-family ledger at 0x144C: StringStruct staging over the
    // virtual StringAllocationBase at 0x1484 (owner captured at 0x1488), then
    // the list stage over it. Staged through the tree's real tables.
    *reinterpret_cast<const uint32_t **>(self + 0x1450) = &AlphaNetLedgerVbtable[0];
    *reinterpret_cast<const uint32_t **>(self + 0x1484) = &StringAllocationBaseVtable;
    *reinterpret_cast<Heap **>(self + 0x1488) = StringAllocationHeap;
    StringAllocationHeap = 0;
    *reinterpret_cast<const uint32_t **>(self + 0x144C) = &AlphaNetStringStructVtable[0];
    // the vbtable walk: [0x1450] + vbtable[1]
    const uint32_t off = AlphaNetLedgerVbtable[1];
    *reinterpret_cast<const uint32_t **>(self + 0x1450 + off) =
        &AlphaNetListVirtualBaseVtable[0];
    *reinterpret_cast<uint32_t *>(self + 0x1454) = 0;
    *reinterpret_cast<uint32_t *>(self + 0x1458) = 0;
    *reinterpret_cast<uint32_t *>(self + 0x145C) = 0;
    *reinterpret_cast<uint32_t *>(self + 0x1460) = 0;
    *reinterpret_cast<uint32_t *>(self + 0x1464) = 0;
    // the list stage's own tables
    *reinterpret_cast<const uint32_t **>(self + 0x144C) = &AlphaNetListDerivedVtable[0];
    *reinterpret_cast<const uint32_t **>(self + 0x1450 + off) =
        &AlphaNetListDerivedVirtualBaseVtable[0];

    new (reinterpret_cast<Heap *>(self + 0x148C)) Heap();

    // the eight slot dwords at 0x78C and the count at 0x768
    for (int dword_i = 0; dword_i < 8; ++dword_i) {
        *reinterpret_cast<uint32_t *>(self + 0x78C + dword_i * 0x19C) = 0;
    }
    *reinterpret_cast<uint32_t *>(self + 0x768) = 0;
}

void AlphaNet::close() {
    uint8_t *const bytes = reinterpret_cast<uint8_t *>(this);
    // Eight process-ID slots at 0x78C, one per player, stride 0x19C.
    for (size_t slot = 0; slot < 8; ++slot) {
        const uint32_t zero = 0;
        memcpy(bytes + 0x78C + slot * 0x19C, &zero, sizeof(zero));
    }
    const uint32_t zero = 0;
    memcpy(bytes + 0x768, &zero, sizeof(zero));
    // The legacy body tail-jumps here with this unchanged, so the network
    // close runs against the same object.
    Net::close();
}


/*
// ORIGINAL: 0x00401CE0 SessionStruct::close 0x00401CE0-0x00401D79
// size      153 bytes
// prototype 
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00401D22 0x00401D34 0x00401D50
Status: Complete
*/
extern "C" __declspec(naked) int __cdecl fn_00401ce0() {
    __asm {
        push ebp
        mov ebp, esp
        push ecx
        push ebx
        push esi
        lea esi, [ecx - 0x1c]
        xor ebx, ebx
        mov dword ptr [esi], 0x669408
        mov eax, dword ptr [ecx - 0x18]
        mov edx, dword ptr [eax + 4]
        mov dword ptr [edx + ecx - 0x18], 0x669404
        mov eax, dword ptr [esi + 8]
        cmp eax, ebx
        je done
        mov eax, dword ptr [esi + 0x10]
        mov dword ptr [ebp - 4], ebx
        cmp eax, ebx
        jle after_loop
        push edi
    loop_top:
        mov eax, dword ptr [esi + 8]
        mov edx, dword ptr [esi]
        mov ecx, dword ptr [eax + 0xc]
        mov dword ptr [esi + 0xc], ecx
        mov edi, dword ptr [eax + 8]
        push edi
        mov ecx, esi
        call dword ptr [edx + 4]
        cmp edi, ebx
        je skip_release1
        mov eax, dword ptr [edi]
        push 1
        mov ecx, dword ptr [eax + 4]
        add ecx, edi
        mov edx, dword ptr [ecx]
        call dword ptr [edx]
    skip_release1:
        mov eax, dword ptr [esi + 8]
        mov dword ptr [eax + 8], ebx
        mov eax, dword ptr [esi + 8]
        cmp eax, ebx
        je skip_release2
        mov ecx, dword ptr [eax]
        push 1
        mov edx, dword ptr [ecx + 4]
        lea ecx, [edx + eax]
        mov eax, dword ptr [edx + eax]
        call dword ptr [eax]
    skip_release2:
        mov ecx, dword ptr [esi + 0xc]
        mov eax, dword ptr [ebp - 4]
        mov dword ptr [esi + 8], ecx
        mov ecx, dword ptr [esi + 0x10]
        inc eax
        cmp eax, ecx
        mov dword ptr [ebp - 4], eax
        jl loop_top
        pop edi
    after_loop:
        mov dword ptr [esi + 8], ebx
        mov dword ptr [esi + 0x14], ebx
        mov dword ptr [esi + 0x10], ebx
    done:
        mov dword ptr [esi + 0x14], ebx
        pop esi
        pop ebx
        mov esp, ebp
        pop ebp
        ret
    }
}
