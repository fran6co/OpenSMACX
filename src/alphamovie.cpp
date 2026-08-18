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
#include "original_seam.h"
#include "alphamovie.h"

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x00404280 ?UNK7@AlphaMovie@@QAEXXZ 0x00404280-0x00404281 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?UNK7@AlphaMovie@@QAEXXZ)(AlphaMovie* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void AlphaMovie::UNK7() {
}

void __fastcall alpha_movie_unk7_redirect(AlphaMovie *self, void *) {
    self->UNK7();
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns 0.
// ORIGINAL: 0x00404220 ?UNK2@AlphaMovie@@QAEHHHH@Z 0x00404220-0x00404225 BYTE_EXACT
// size      5 bytes
// prototype int (__thiscall ?UNK2@AlphaMovie@@QAEHHHH@Z)(AlphaMovie* this, int, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0, always
Status: Complete
*/
int AlphaMovie::UNK2(int, int, int) {
    return 0;
}

int __fastcall alpha_movie_unk2_redirect(AlphaMovie *self, void *, int a1, int a2, int a3) {
    return self->UNK2(a1, a2, a3);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns 0.
// ORIGINAL: 0x00404240 ?UNK4@AlphaMovie@@QAEHHHHH@Z 0x00404240-0x00404245 BYTE_EXACT
// size      5 bytes
// prototype int (__thiscall ?UNK4@AlphaMovie@@QAEHHHHH@Z)(AlphaMovie* this, int, int, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0, always
Status: Complete
*/
int AlphaMovie::UNK4(int, int, int, int) {
    return 0;
}

int __fastcall alpha_movie_unk4_redirect(AlphaMovie *self, void *, int a1, int a2, int a3, int a4) {
    return self->UNK4(a1, a2, a3, a4);
}

/*
Purpose: Unknown; the legacy implementation returns its argument unchanged.
         Unlike the rest of this family it is not a constant return - it builds
         a frame, loads the parameter and returns it:

             push ebp / mov ebp, esp / mov eax, [ebp+8] / pop ebp / ret 4

         so the returned value is the caller's own argument, at full 32-bit
         width and with no truncation or re-extension. It touches no field,
         which is why it can be replaced ahead of the class layout like its
         neighbours.
// ORIGINAL: 0x00404230 ?UNK3@AlphaMovie@@QAEHH@Z 0x00404230-0x0040423A BYTE_EXACT
// size      10 bytes
// prototype int (__thiscall ?UNK3@AlphaMovie@@QAEHH@Z)(AlphaMovie* this, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the argument, unchanged
Status: Complete
*/
int AlphaMovie::UNK3(int a1) {
    return a1;
}

int __fastcall alpha_movie_unk3_redirect(AlphaMovie *self, void *, int a1) {
    return self->UNK3(a1);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns 0.
// ORIGINAL: 0x00404250 ?UNK5@AlphaMovie@@QAEHH@Z 0x00404250-0x00404255 BYTE_EXACT
// size      5 bytes
// prototype int (__thiscall ?UNK5@AlphaMovie@@QAEHH@Z)(AlphaMovie* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0, always
Status: Complete
*/
int AlphaMovie::UNK5(int) {
    return 0;
}

int __fastcall alpha_movie_unk5_redirect(AlphaMovie *self, void *, int a1) {
    return self->UNK5(a1);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00404260 ?UNK6@AlphaMovie@@QAEXHH@Z 0x00404260-0x00404263 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?UNK6@AlphaMovie@@QAEXHH@Z)(AlphaMovie* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void AlphaMovie::UNK6(int, int) {
}

void __fastcall alpha_movie_unk6_00404260_redirect(AlphaMovie *self, void *, int a1, int a2) {
    self->UNK6(a1, a2);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x00404270 ?UNK6@AlphaMovie@@QAEXH@Z 0x00404270-0x00404273 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?UNK6@AlphaMovie@@QAEXH@Z)(AlphaMovie* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void AlphaMovie::UNK6(int) {
}

void __fastcall alpha_movie_unk6_00404270_redirect(AlphaMovie *self, void *, int a1) {
    self->UNK6(a1);
}

/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns 1.
// ORIGINAL: 0x004042A0 ?UNK8@AlphaMovie@@QAEHHH@Z 0x004042A0-0x004042A8 BYTE_EXACT
// size      8 bytes
// prototype int (__thiscall ?UNK8@AlphaMovie@@QAEHHH@Z)(AlphaMovie* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 1, always
Status: Complete
*/
int AlphaMovie::UNK8(int, int) {
    return 1;
}

int __fastcall alpha_movie_unk8_redirect(AlphaMovie *self, void *, int a1, int a2) {
    return self->UNK8(a1, a2);
}

/*
// ORIGINAL: 0x004041E0 ?close@AlphaMovie@@QAEXXZ 0x004041E0-0x004041F7 BYTE_EXACT
// size      23 bytes
// prototype void (__thiscall ?close@AlphaMovie@@QAEXXZ)(AlphaMovie* this)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4E40 0x005FFDB0
Status: Complete
*/
void AlphaMovie::close() {
    char *self = reinterpret_cast<char *>(this);
    reinterpret_cast<MCIVideo *>(self + 0xa14)->close();
    reinterpret_cast<GraphicWin *>(self)->close();
}

/*
// ORIGINAL: 0x00404290 ?update@AlphaMovie@@QAEXXZ 0x00404290-0x00404298 BYTE_EXACT
// size      8 bytes
// prototype void (__thiscall ?update@AlphaMovie@@QAEXXZ)(AlphaMovie* this)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D56B0
Status: Complete
*/
void AlphaMovie::update() {
    reinterpret_cast<GraphicWin *>(this)->update(0);
}

func_mci_video_close MCIVideoOriginalClose =
    original_method<func_mci_video_close>(0x005FFDB0);

/*
Purpose: Shut the MCI video device down.
Forwards To: 005FFDB0
Return Value: n/a
Status: Original dependency - forwards to the original image.

The auto_inline(off) is load-bearing. VC6 at /Ob2 defers codegen to the end of
the translation unit, so being defined after AlphaMovie::close does not stop
it folding this forwarder into that recovered body: measured, the caller's
`lea ecx,[esi+0xa14]; call rel32` became `lea ecx,[esi+0xa14]; call dword ptr
[MCIVideoOriginalClose]`. The pragma keeps the caller's direct call and leaves
this a separate thunk.
*/
#pragma auto_inline(off)
void MCIVideo::close() {
    (ORIGINAL(this)->*MCIVideoOriginalClose)();
}
#pragma auto_inline(on)
