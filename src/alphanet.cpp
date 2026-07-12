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
#include "alphanet.h"

/*
Purpose: Convert a multiplayer process ID to its one-based player index.
Original Offset: 004E25E0
Return Value: Player index (1-7); process ID not found (0)
Status: Complete
*/
int AlphaNet::pid_2_idx(uint32_t process_id) {
    const auto *bytes = reinterpret_cast<const uint8_t *>(this);
    for (int index = 1; index < 8; index++) {
        uint32_t candidate;
        memcpy(&candidate, bytes + 0x928 + (index - 1) * 0x19C, sizeof(candidate));
        if (candidate == process_id) {
            return index;
        }
    }
    return 0;
}

int __fastcall alpha_net_pid_to_idx_redirect(
    AlphaNet *self, void *, uint32_t process_id) {
    return self->pid_2_idx(process_id);
}
