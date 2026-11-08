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
#include "strings.h"
#include "buffer.h"
#include "alpha.h"
#include "technology.h"
#include "veh.h"
#include "base.h"
#include "faction.h"

// The 1032-byte buffer ends where PopupStartCaption begins at 0x009B8AA8.
// RECOVERY_OBJECT: v2 0x009B86A0 0x009B8AA8 ?StringTemp@@3PADA 0x00500120
char StringTemp[1032];   // 0x009B86A0

// Clear the first character without disturbing the reusable buffer storage.
// ORIGINAL: 0x00500120 ?clear@@YAXXZ 0x00500120-0x00500128 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00500120-0x00500128 writes 0x009B86A0", "git:0cb1eb5c:src/recovered/00500120.cpp;source:src/strings.cpp:StringTemp"], "reason": "The original zeroes the first byte of the shared scratch buffer, whose real storage is StringTemp in this translation unit."}
// body src/strings.h
// BODY IN strings.h, as `MEASURED inline`.

// ORIGINAL: 0x005A58B0 ?space@@YAXXZ 0x005A58B0-0x005A58C3 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:bb3b3233842b050fb0840e1cc6c226e8f30f7055:src/recovered/units/005a58b0.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "recovery/tasks/game-logic-seed-owner-coverage.json"], "reason": "strings.cpp / strings.h: append one ordinary space to the existing StringTemp via strcat. Complete reviewed source translation; historical claims remain restoration obligations, not proof of current matching, linked preservation or game acceptance."}
void __cdecl space() {
    strcat(StringTemp, BufferJoinSpace);
}
