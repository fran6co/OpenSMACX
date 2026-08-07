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

/*
Purpose: Convert a multiplayer process ID to its one-based player index.
Original Offset: 004E25E0
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
Original Offset: 004E2610
Return Value: Player identity; process ID not found (0)
Status: Complete
*/
int AlphaNet::pid_2_who(uint32_t process_id) {
    const uint8_t *bytes = reinterpret_cast<const uint8_t *>(this);
    for (int slot = 0; slot < 7; ++slot) {
        uint32_t candidate;
        memcpy(&candidate, bytes + 0x928 + slot * 0x19C, sizeof(candidate));
        if (candidate == process_id) {
            int8_t identity;
            memcpy(&identity, bytes + 0x92C + slot * 0x19C, sizeof(identity));
            return identity;
        }
    }
    return 0;
}

/*
Purpose: Convert a signed multiplayer player identity to its process ID.
Original Offset: 004E2660
Return Value: Process ID; player identity not found (0)
Status: Complete
*/
int AlphaNet::who_2_pid(int identity) {
    const uint8_t *bytes = reinterpret_cast<const uint8_t *>(this);
    for (int slot = 0; slot < 7; ++slot) {
        int8_t candidate;
        memcpy(&candidate, bytes + 0x92C + slot * 0x19C, sizeof(candidate));
        if (candidate == identity) {
            int process_id;
            memcpy(&process_id, bytes + 0x928 + slot * 0x19C, sizeof(process_id));
            return process_id;
        }
    }
    return 0;
}

/*
Purpose: Convert a signed multiplayer player identity to its one-based index.
Original Offset: 004E26B0
Return Value: Player index (1-7); player identity not found (0)
Status: Complete
*/
int AlphaNet::who_2_idx(int identity) {
    const uint8_t *bytes = reinterpret_cast<const uint8_t *>(this);
    for (int slot = 0; slot < 7; ++slot) {
        int8_t candidate;
        memcpy(&candidate, bytes + 0x92C + slot * 0x19C, sizeof(candidate));
        if (candidate == identity) {
            return slot + 1;
        }
    }
    return 0;
}

int __fastcall alpha_net_pid_to_idx_redirect(
    AlphaNet *self, void *, uint32_t process_id) {
    return self->pid_2_idx(process_id);
}

int __fastcall alpha_net_pid_to_who_redirect(
    AlphaNet *self, void *, uint32_t process_id) {
    return self->pid_2_who(process_id);
}

int __fastcall alpha_net_who_to_pid_redirect(
    AlphaNet *self, void *, int identity) {
    return self->who_2_pid(identity);
}

int __fastcall alpha_net_who_to_idx_redirect(
    AlphaNet *self, void *, int identity) {
    return self->who_2_idx(identity);
}

func_net_close NetCloseOriginal = original_method<func_net_close>(0x0062E010);

/*
Purpose: Clear every player's process slot and hand off to the network close.
Original Offset: 004E25B0
Status: Complete with a temporary dependency on the network close
*/
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
    (ORIGINAL(this)->*NetCloseOriginal)();
}

void __fastcall alpha_net_close_redirect(AlphaNet *self, void *) {
    self->close();
}
