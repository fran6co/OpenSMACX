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
#include "map.h"
#include "faction.h"

// Shared original punctuation storage, defined in socialwin.cpp.
extern char SocialWinColonSpaceText[];
extern char SocialWinCommaSpaceText[];

// Keep these application formatters separate from Strings::get's definition so
// VC6's same-TU /Ob2 inliner cannot absorb the original get calls. Their shared
// storage and Strings implementation remain owned by strings.cpp.

// The shared space is owned by BufferJoinSpace (buffer.cpp, 0x00682820).
// Canonical RECOVERY_LITERAL anchors now live in datalink_entries.cpp.
// Their bindings remain facts; source regrouping does not prove the
// original translation-unit home. Unaffected formatters independently
// corroborate all three starts: 26 original pushes across eight owners,
// including say_facility at 0x00590910. The moved functions are not
// treated as evidence of their own source home.
// These are logical one-character-plus-NUL spans, not enclosing allocations.
// The original spans are in writable .data; these bindings do not assert
// constness, runtime immutability, lifetime or linked COMDAT retention.
// Identical compiler symbols in other translation units remain independent.

// Append a caller-supplied string to the shared scratch buffer.
// ORIGINAL: 0x00500130 ?says@@YAXPBD@Z 0x00500130-0x00500146 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00500130-0x00500146 calls strcat with 0x009B86A0", "git:49d164a4:src/recovered/units/00500130.cpp;source:src/strings.cpp:StringTemp"], "reason": "The original appends a caller string into the source-owned shared StringTemp buffer."}
void __cdecl says(const char *text) {
    strcat(StringTemp, text);
}
