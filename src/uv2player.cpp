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
#include "uv2player.h"
#include <mmsystem.h>

/*
Purpose: Construct the Buffer subobject, then clear one field.

             mov esi,ecx / lea ecx,[esi+0x8dc] / call ??0Buffer
             mov dword [esi+0x10c],0 / mov eax,esi / ret

         The Buffer lives at 0x8dc and the cleared field at 0x10c - which is
         BELOW the Buffer, so the store is in this object's own space and not
         inside the subobject. It happens after the constructor, which is the
         order that would matter if the constructor ever reached that far back.

// ORIGINAL: 0x004BEA30 ??0UV2Player@@QAE@XZ 0x004BEA30-0x004BEA4C BYTE_EXACT
// size      28 bytes
// prototype void (__thiscall ??0UV2Player@@QAE@XZ)(UV2Player* this)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D7210
Return Value: `this`
Status: Complete
*/
UV2Player::UV2Player() {
    // buffer_ has completed construction before the native ready pointer clears.
    field_10C_ = 0;
}

// The movie input loop and timed writer share these original four-byte slots.
// RECOVERY_OBJECT: v2 0x008CC250 0x008CC254 ?UV2PlayerQuitRequested@@3HA 0x004BF390
int UV2PlayerQuitRequested;

// ORIGINAL: 0x004BF390 ?on_key@UV2Player@@QAAHXZ 0x004BF390-0x004BF3FE
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/004bf390.cpp", "image:terranx_original.exe:0x004BF390", "4BF3AA/BE reject only zero message results; Return/Escape/Space and either mouse-down set quit8CC250. Timed text disarms Buffer+8, writes individual bytes, reloads delayE64 after each timeGetTime and re-arms the cdecl callback even for nonpositive length.", "recovery/tasks/uv2player-complete-owner-recovery.json; bounded complete-owner scope, preserved historical baseline/claims, no matching or runtime acceptance."], "reason": "Recover the movie input/timed text owner using SDK messages and real Buffer callback, active instance and unsigned delay storage, preserving native input and callback order."}
// symbol    ?on_key@UV2Player@@SAHXZ
int __cdecl UV2Player::on_key() {
    MSG message;
    if (!PeekMessageA(&message, 0, 0, 0, 0) ||
        !GetMessageA(&message, 0, 0, 0)) {
        return 0;
    }
    if (message.message == WM_KEYDOWN) {
        if (message.wParam != VK_RETURN && message.wParam != VK_ESCAPE &&
            message.wParam != VK_SPACE) {
            return 0;
        }
    } else if (message.message != WM_LBUTTONDOWN &&
               message.message != WM_RBUTTONDOWN) {
        return 0;
    }
    UV2PlayerQuitRequested = 1;
    return 1;
}

// ORIGINAL: 0x004BEDC0 ?UNK1@UV2Player@@QAEHHHHH@Z 0x004BEDC0-0x004BEE5A BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/004bedc0.cpp", "image:terranx_original.exe:0x004BEDC0", "4BF3AA/BE reject only zero message results; Return/Escape/Space and either mouse-down set quit8CC250. Timed text disarms Buffer+8, writes individual bytes, reloads delayE64 after each timeGetTime and re-arms the cdecl callback even for nonpositive length.", "recovery/tasks/uv2player-complete-owner-recovery.json; bounded complete-owner scope, preserved historical baseline/claims, no matching or runtime acceptance."], "reason": "Recover the movie input/timed text owner using SDK messages and real Buffer callback, active instance and unsigned delay storage, preserving native input and callback order."}
// symbol    ?write_timed@UV2Player@@QAEHPADHHH@Z
int UV2Player::write_timed(LPSTR text, int x, int y, int length) {
    buffer_.set_write_raw_callback(0);
    for (int index = 0; index < length; ++index) {
        const DWORD started = timeGetTime();
        x = buffer_.write_raw_l(text + index, x, y, 1);
        while (timeGetTime() - started < write_delay_) {
        }
    }
    buffer_.set_write_raw_callback(&uv2_player_writer_redirect);
    return x;
}

// The one player instance the movie writer serves: 0x008CC254 holds the
// UV2Player whose embedded Buffer the __cdecl writer below walks when it is
// invoked as the buffer's output hook - the writer member's QAA convention
// passes no receiver, so the image routes the object through this slot.
// Modelled as real storage beside its only recovered reader, the
// WorldClimateWorldWin idiom.
// RECOVERY_OBJECT: v2 0x008CC254 0x008CC258 ?UV2PlayerActive@@3PAVUV2Player@@A 0x004BF400
UV2Player *UV2PlayerActive;  // 0x008CC254

/*
Purpose: The buffered writer's re-entry half, run as the embedded Buffer's
         output hook: take the player instance from the active-player slot,
         disarm the hook (so the writes below land raw), then rewrite the
         caller's rows one character at a time through Buffer::write_raw_l
         with the same busy-wait the timed writer uses, and re-arm the hook
         with this function before returning.

// ORIGINAL: 0x004BEE60 ?UNK2@UV2Player@@QAAHHHHH@Z 0x004BEE60-0x004BEEFD FILE
// symbol   _uv2_player_writer_redirect
// size      157 bytes
// prototype
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005DBD00
// indirect  0x004BEE93 0x004BEEB8 0x004BEEC6
Return Value: the x coordinate the last write left off at
Status: Coverage transcription - unclaimed body
*/
extern "C" int __cdecl uv2_player_writer_redirect(LPSTR text, int x_coord,
                                                  int y_coord, int row_count) {
    UV2Player *const instance = UV2PlayerActive;
    if (instance == 0) {
        return 0;
    }
    Buffer &buffer = instance->buffer_;
    buffer.set_write_raw_callback(0);
    int x = x_coord;
    int index = 0;
    if (row_count > 0) {
        DWORD(WINAPI *const get_time)() = timeGetTime;
        do {
            const unsigned int started = get_time();
            x = buffer.write_raw_l(text + index, x,
                                    y_coord, 1);
            while (get_time() - started < instance->write_delay_) {
            }
            ++index;
        } while (index < row_count);
    }
    buffer.set_write_raw_callback(&uv2_player_writer_redirect);
    return x;
}


/*
Purpose: Unknown; the legacy implementation is a constant return that returns 0.
// ORIGINAL: 0x004BF380 ?UNK4@UV2Player@@QAEHXZ 0x004BF380-0x004BF383 BYTE_EXACT
// size      3 bytes
// prototype int (__thiscall ?UNK4@UV2Player@@QAEHXZ)(UV2Player* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0
Status: Complete
*/
int UV2Player::UNK4() {
    return 0;
}


/*
Purpose: Tear down a UV2Player: close it, then destroy its Buffer member.
         buffer_ is a real member, so the compiler installs its own SEH
         unwind protection around close() automatically - matching the
         image, which protects the same call so the Buffer subobject is
         still destroyed if close() throws.
// ORIGINAL: 0x004043D0 ??1UV2Player@@QAE@XZ 0x004043D0-0x0040441C;0x00650740-0x00650758 BYTE_EXACT
// LEVER: MEASURED 22/22 agreeing. `Buffer buffer_` being a REAL typed member (not raw storage) is what does it - the compiler installs the exact SEH-wrapped-close()-then-destroy-Buffer shape on its own once the member is real, so the body is just `close();` with no explicit try/catch and no explicit `~Buffer()` call at all.
// size      100 bytes
// prototype void (__thiscall ??1UV2Player@@QAE@XZ)(UV2Player* this)
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004BF400 0x005D7410
Return Value: n/a
Status: Complete
*/
UV2Player::~UV2Player() {
    close();
}
