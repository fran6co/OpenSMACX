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
// RECOVERY_BINDING: v1 {"evidence": ["image:0x00500130-0x00500146 calls strcat with 0x009B86A0", "git:0cb1eb5c:src/recovered/units/00500130.cpp;source:src/strings.cpp:StringTemp"], "reason": "The original appends a caller string into the source-owned shared StringTemp buffer."}
void __cdecl says(const char *text) {
    strcat(StringTemp, text);
}

// Pinned say_loc also references shared punctuation at0x00682E9C/0x00682E98.
// Their canonical original bindings are beside say_vehicles in datalink_entries.cpp;
// qualifying this TU's pooled copies remains a reference-restoration obligation.
// ORIGINAL: 0x00592250 ?say_loc@@YAXPADHHHHH@Z 0x00592250-0x005923FD FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["src/veh.h:say_loc;src/map.h:map_loc", "Historical initial canonical-anchor owner: src/string_formatting.cpp:independently corroborated parenthesis RECOVERY_LITERAL bindings", "src/buffer.cpp:BufferJoinSpace;src/socialwin.cpp:SocialWinCommaSpaceText", "image:0x00592250-0x005923FD uses 0x00682820,0x00682E90,0x00682E9C,0x00682E98", "source:src/datalink_entries.cpp retains the unique canonical parentheses bindings; say_loc original operands at0x00682E9C/0x00682E98 still require qualification for this TUs pooled copies."], "reason": "Group the free location formatter with the other say_* formatters and their independently evidenced punctuation; share the existing source-owned space and comma-space storage. This is a current semantic home, not proof of the original translation unit."}
// size      429 bytes
// prototype void (__cdecl ?say_loc@@YAXPADHHHHH@Z)(int8*, int xCoord, int yCoord, int, int, int)
// callers   4   call targets   5
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004E3A50 0x004E3D50 0x006169A0 0x00645470 0x0064FC88
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00592250
// measured tier  MISMATCH
// divergence     3
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00592250/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?say_loc@@YAXPADHHHHH@Z  at 0x00592250  (429 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// The VC6 dialect limits and the source-form rules used to live here.
// They are knowledge, not scaffolding, so they now live in the agent
// system prompt (mizuchi.yaml, plugins.claude-runner.systemPrompt),
// where they can be edited without regenerating anything and are in
// context from the first token rather than behind a file read. This
// emitter computes declarations; it does not carry lessons.
void __cdecl say_loc(char *text, int x, int y, int want_name, int trailer_mode,
                     int want_coords) {
    char number[80];
    int base_id = base_at(x, y);
    if (want_name != 0) {
        if (base_id < 0) {
            want_coords = 0;
            base_id = base_find(x, y, -1, -1, -1, ConsoleGlobal.faction_id_);
            if (base_id < 0) {
                goto trailer;
            }
            strcat(text, label_get(0xf8 / 4));
            strcat(text, BufferJoinSpace);
        } else {
            goto skip_first_check;
        }
    }
    if (base_id < 0) {
        goto trailer;
    }
skip_first_check:
    if (want_coords != 0) {
        strcat(text, label_get(0x20 / 4));
        strcat(text, BufferJoinSpace);
    }
    strcat(text, Bases[base_id].name_string);
    if (trailer_mode != 0) {
        strcat(text, BufferJoinSpace);
    }

trailer:
    if (trailer_mode == 1 || (trailer_mode == 2 && base_id < 0)) {
        strcat(text, "(");
        _itoa(x, number, 10);
        strcat(text, number);
        strcat(text, SocialWinCommaSpaceText);
        _itoa(y, number, 10);
        strcat(text, number);
        strcat(text, ")");

        int flags = GameDataStateStorage.fields_.game_state_;
        if ((flags & 0x40) != 0 && (flags & 0x1000) != 0) {
            strcat(StringTemp, BufferJoinSpace);
            Map *tile = map_loc(x, y);
            _itoa(tile->region, number, 10);
            strcat(StringTemp, number);
        }
    }
}
