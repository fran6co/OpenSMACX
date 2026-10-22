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
#include "caviar.h"
#include "caviar_io.h"

// These operations share the configured Caviar callback table, not its
// storage. Error staging stays in the engine owner. This source grouping
// does not assert a recovered historical translation-unit boundary.
extern "C" void __cdecl vox_stage_error(const char *message);

// Historical spelling evidence: the image loads the No-room pointer slot
// before pushing it (`mov edx,[0x6989d8] / push edx`); a literal array changed
// that instruction shape. These retain the existing pointer declarations.
static const char *const vox_msg_reading_file = "Reading file";        // 0x006989D4
static const char *const vox_msg_no_room = "No room";                  // 0x006989D8

#define vox_read_slot (CaviarOutputMessageWindow.io_read_)

/*
Purpose: Allocate through the engine's alloc slot into *out, staging "No room"
         and reporting failure when the slot returns nothing.
// ORIGINAL: 0x00638620 sub_638620 0x00638620-0x0063864E FILE BYTE_EXACT
// symbol    _vox_alloc_block
// size      46 bytes
// callers   8   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// indirect  0x00638625
//
// CLAIMED BYTE_EXACT in its scaffold, homed 2026-09-04 and re-measured here.
// Its two bindings are named from the image: g_009c0b80 is the engine's alloc
// slot - slot 0 of the six vx_* callbacks, formerly vox_alloc_slot - and
// g_006989d8 is the "No room" message slot, vox_msg_no_room here.
//
// HOMED from src/recovered/units/00638620.cpp.
Return Value: 0 with *out filled, or -1 with the message staged.
*/
extern "C" int __cdecl vox_alloc_block(int size, void *out_storage) {
    void *block = CaviarOutputMessageWindow.io_allocate_((unsigned long)size);
    // The callback API publishes one native pointer representation into a
    // caller-owned pointer slot. Byte copying avoids treating T** as void**.
    // Publish before staging failure, exactly like the original entry point.
    memcpy(out_storage, &block, sizeof(block));
    if (block == 0) {
        vox_stage_error(vox_msg_no_room);
        return -1;
    }
    return 0;
}

/*
Purpose: Read EXACTLY count bytes through the engine's read slot: loop until
         the accumulated total reaches the request, and stage "Reading file"
         the moment the slot answers negative.
// ORIGINAL: 0x00638650 sub_638650 0x00638650-0x006386A8 FILE
// symbol    _vox_read_exact
// size      88 bytes
// callers   15   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// indirect  0x00638670
//
// HOMED from src/recovered/units/00638650.cpp. Its two bindings are named
// from the image: g_009c0b88 is the read slot - slot 2 of the six vx_*
// callbacks Caviar::init_class installs, this file's vox_read_slot - and
// g_006989d4 is the "Reading file" message slot.
// TRIED: the image dedicates ALL FOUR callee-saved registers here - ebx/ebp
//        hold the buffer and the fd for the whole loop, esi the running
//        total, edi the count - and reaches for them in the order
//        count/total/buffer/fd. Every spelling of this body allocates two
//        fewer registers and loads the arguments at their use, 39
//        instructions against the image's 45 at /O2 (17 agreeing, first
//        divergence at instruction 7, the loop's register set); the /Oy- set
//        scores a higher SIMILARITY (0.739) with a frame the image does not
//        have, so `measure` reports 1 of 45. No spelling tried made VC6
//        dedicate ebp as a callee-saved register - the receiver-spill wall.
//        /c /O2 /Gy /GR- /GX.
Return Value: 0 when the full count was read, -1 when the slot went negative.
*/
extern "C" int __cdecl vox_read_exact(int fd, void *buffer, int count) {
    int total = 0;
    if (count > 0) {
        do {
            int read = vox_read_slot((unsigned long)fd,
                                     (char *)buffer + total, count - total);
            if (read < 0) {
                vox_stage_error(vox_msg_reading_file);
                return -1;
            }
            total += read;
        } while (total < count);
    }
    return 0;
}
