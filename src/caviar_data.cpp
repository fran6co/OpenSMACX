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
#include "palette.h"

#include <io.h>
#include <stdlib.h>

/*
Purpose: Initialize an empty Caviar object-data record.
// ORIGINAL: 0x00616BC0 ??0CaviarData@@QAE@XZ 0x00616BC0-0x00616BCD BYTE_EXACT
// size      13 bytes
// prototype void (__thiscall ??0CaviarData@@QAE@XZ)(CaviarData* this)
// callers   82   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: n/a
Status: Complete
*/
CaviarData::CaviarData()
    : field_0_(0), fileDescriptor_(0), record_(nullptr) {
}

/*
Purpose: Release the record this data slot owns. A slot with no record is left
         entirely untouched; otherwise the helper walks the record and frees
         its members, and the slot forgets it. The two leading fields keep
         whatever they held either way.
// ORIGINAL: 0x00616C60 ?close@CaviarData@@QAEXXZ 0x00616C60-0x00616C7C BYTE_EXACT
// size      28 bytes
// prototype void (__thiscall ?close@CaviarData@@QAEXXZ)(CaviarData* this)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00638430
Return Value: n/a
Status: Complete
*/
void CaviarData::close() {
    if (record_) {
        caviar_free_record(record_);
        record_ = nullptr;
    }
}

/*
Purpose: Load a .cvr file into this slot: release whatever record it still
         holds, open the file, hand the descriptor to the loader, and close
         the descriptor again once the record is built - the slot keeps the
         record, not the file.
// ORIGINAL: 0x00616BD0 ?init@CaviarData@@QAEHPAD@Z 0x00616BD0-0x00616C55 FILE BYTE_EXACT
// symbol    ?init@CaviarData@@QAEHPAD@Z
// size      133 bytes
// prototype int (__thiscall ?init@CaviarData@@QAEHPAD@Z)(CaviarData* this, int8*)
// callers   1   call targets   4
// kind      game
// flags     sp_ready;purged_ok
// calls     0x006381F0 0x00638430 0x006493C6 0x0064CF73
//
// CLAIMED BYTE_EXACT, homed from src/unrecovered/00616bd0.cpp 2026-09-04 and
// re-measured here. Its three helper bindings rename to bodies this file now
// owns or forwards: caviar_sub_638430 is caviar_free_record, homed above;
// caviar_sub_6381f0 is vox_load_record (0x006381F0);
// caviar_open is the CRT's _open, spelled plainly -
// 0x8000 is _O_BINARY.
Return Value: 0, 0x10 without a name, 6 when the open fails, 0x17 when the
              loader returns no record.
*/
int CaviarData::init(char *name) {
    if (name == 0) {
        return 0x10;
    }
    if (record_) {
        caviar_free_record(record_);
        record_ = 0;
    }
    int fd = _open(name, 0x8000);
    fileDescriptor_ = fd;
    if (fd == -1) {
        return 6;
    }
    CvrRecord *record = vox_load_record(fd);
    record_ = record;
    if (record == 0) {
        return 0x17;
    }
    if (fileDescriptor_ != 0) {
        _close(fileDescriptor_);
        fileDescriptor_ = 0;
    }
    return 0;
}

// Original616C80. The second stack argument is present but unused.
// ORIGINAL: 0x00616C80 sub_616c80 0x00616C80-0x00616D33
// symbol    ?apply_palette@CaviarData@@QAEHPAVPalette@@H@Z
// kind      game
int CaviarData::apply_palette(Palette *destination, int) {
    CvrRecord *record = record_;
    if (record == 0) return 7;
    if (destination == 0) return 0x10;
    if (record->palette_ == 0) return 0x10;

    const int first = record->palette_[0];
    const int count = record->palette_[1] - first;
    RGBQUAD colours[256];
    if (count > 0) {
        RGBQUAD *output = colours;
        int source_index = first * 3;
        int remaining = count;
        do {
            output->rgbRed = record->palette_[source_index + 2];
            output->rgbGreen = record->palette_[source_index + 3];
            output->rgbBlue = record->palette_[source_index + 4];
            // rgbReserved remains unwritten, as in the pinned original.
            source_index += 3;
            ++output;
        } while (--remaining != 0);
    }
    return destination->set_rgbquad(colours, first, count);
}

// Original616D40. This releases one optional chunk, not the loaded record.
// ORIGINAL: 0x00616D40 sub_616d40 0x00616D40-0x00616D66 BYTE_EXACT
// symbol    ?release_palette_chunk@CaviarData@@QAEXXZ
// kind      game
void CaviarData::release_palette_chunk() {
    if (record_ != 0 && record_->field_18_ != 0) {
        free(record_->field_18_);
        // The original reloads the owner's record after the free call.
        record_->field_18_ = 0;
    }
}
