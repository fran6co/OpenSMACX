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
#include "flic.h"

/*
Purpose: Construct the embedded Buffer and Palette (the class's only two
         non-POD members, in declaration order - no explicit statement
         reaches either), then zero every scalar/pointer field the image
         zeroes.
// ORIGINAL: 0x00629D60 ??0Flic@@QAE@XZ 0x00629D60-0x00629DE2;0x00663320-0x00663335 BYTE_EXACT
// size      130 bytes
// prototype void (__thiscall ??0Flic@@QAE@XZ)(Flic* this)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok;frame
// calls     0x005D7210 0x005FE2A0
Return Value: n/a
Status: Complete
*/
Flic::Flic() {
    field_5AC_ = 0;
    field_5B0_ = 0;
    field_5B4_ = 0;
    file_ = nullptr;
    field_A10_[0] = 0;
    buffer2_ = nullptr;
    field_0_ = 0;
    field_5A4_ = 0;
    field_59C_ = 0;
    field_5A0_ = 0;
}

/*
Purpose: Release the open file (if any) and zero the same fields close()
         zeroes.
// ORIGINAL: 0x00629DF0 ?close@Flic@@QAEXXZ 0x00629DF0-0x00629E44 BYTE_EXACT
// size      84 bytes
// prototype void (__thiscall ?close@Flic@@QAEXXZ)(Flic* this)
// callers   11   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00645598
Return Value: n/a
Status: Complete
*/
void Flic::close() {
    FILE *file = file_;
    field_5AC_ = 0;
    field_5B0_ = 0;
    field_5B4_ = 0;
    field_A10_[0] = 0;
    buffer2_ = nullptr;
    field_0_ = 0;
    field_5A4_ = 0;
    field_59C_ = 0;
    field_5A0_ = 0;
    if (file) {
        fclose(file);
        file_ = nullptr;
    }
}

/*
Purpose: Close the file, then tear down the embedded Palette and Buffer in
         reverse declaration order - both automatic, no explicit statement
         but the `close()` call.
// ORIGINAL: 0x004327A0 ??1Flic@@QAE@XZ 0x004327A0-0x004327F8;0x00653420-0x00653443
// size      88 bytes
// prototype void (__thiscall ??1Flic@@QAE@XZ)(Flic* this)
// callers   0   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok;frame
// calls     0x00629DF0 0x005D7410 0x005FE2E0
// TRIED: best reached is 13/25 instructions, 0.980 similar (best of 10
//        flag sets, `/c /O2 /Ob0 /Gy /GR- /Oy- /GX`) - one single missing
//        instruction (`tools/listing_diff.py`): the image stores a SECOND
//        EH unwind-state byte (`mov byte ptr [ebp-4], 0`) right after
//        `close()` returns, transitioning from "everything still needs
//        teardown" to "only the two subobjects do". This tree's compiled
//        body never emits that second transition - compiler-generated EH
//        bookkeeping with no corresponding source statement to write, not
//        reachable by changing the body. Left as `close();`, the direct
//        transcription of the one real call in this function.
Return Value: n/a
Status: Complete
*/
Flic::~Flic() {
    close();
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x0062A710 ?UNK4@Flic@@QAEXXZ 0x0062A710-0x0062A711 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?UNK4@Flic@@QAEXXZ)(Flic* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Flic::UNK4() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x0062AAF0 ?UNK5@Flic@@QAEXXZ 0x0062AAF0-0x0062AAF1 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?UNK5@Flic@@QAEXXZ)(Flic* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Flic::UNK5() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x0062AB00 ?UNK6@Flic@@QAEXXZ 0x0062AB00-0x0062AB01 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?UNK6@Flic@@QAEXXZ)(Flic* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Flic::UNK6() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x0062ACC0 ?UNK7@Flic@@QAEXXZ 0x0062ACC0-0x0062ACC1 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?UNK7@Flic@@QAEXXZ)(Flic* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Flic::UNK7() {
}

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x0062B5F0 ?UNK8@Flic@@QAEXXZ 0x0062B5F0-0x0062B5F1 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?UNK8@Flic@@QAEXXZ)(Flic* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Flic::UNK8() {
}





