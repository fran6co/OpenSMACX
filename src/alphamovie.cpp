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
#include "alphamovie.h"

/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
Original Offset: 00404280
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
Original Offset: 00404220
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
Original Offset: 00404240
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
Original Offset: 00404230
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
Original Offset: 00404250
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
Original Offset: 00404260
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
Original Offset: 00404270
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
Original Offset: 004042A0
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
Original Offset: 004041E0
Status: Complete
*/
void AlphaMovie::close() {
    char *self = reinterpret_cast<char *>(this);
    reinterpret_cast<MCIVideo *>(self + 0xa14)->close();
    reinterpret_cast<GraphicWin *>(self)->close();
}

/*
Original Offset: 00404290
Status: Complete
*/
void AlphaMovie::update() {
    reinterpret_cast<GraphicWin *>(this)->update(0);
}
