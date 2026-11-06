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
#include "general.h"
#include "strings.h"
#include "text.h"
#include "popup.h"
#include "game.h"
#include "faction.h"
#include "console.h"
#include <cstring>

// Read a script section into the shared substitution buffer. The text
// reader owns TextBufferGetPtr; parse_string supplies the substitutions.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0048C060 ?parse_it@@YAXPADPBD@Z 0x0048C060-0x0048C099 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned48C060..48C099 clears StringTemp9B86A0, calls X_text_open5BECA0, then text_get5FD570 and parse_string625880(TextBufferGetPtr9B7D00,StringTemp) on success.", "src/recovered/units/0048c060.cpp retains this same original identity, complete57-byte span and historical BYTE_EXACT claim; Quayle redraw4956D0 passes Holobook filename and BOOKTITLE/BOOKINTRO/BOOKLIST section names."], "reason": "Recover the filename/section wrapper using the real shared text and substitution owners; the existing seed retains its historical matching obligation."}
void __cdecl parse_it(char *file_name, const char *section) {
    StringTemp[0] = 0;
    if (X_text_open(file_name, section) == 0) {
        text_get();
        parse_string(TextBufferGetPtr, StringTemp);
    }
}
// ORIGINAL: 0x00584D30 sub_584d30 0x00584D30-0x00584D52 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:5167ec56aaa9031de73f42c05fbc3bdd0bad0201:src/recovered/00584d30.cpp", "pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x584d30 complete original body; independent source-owner review.", "source:src/general.cpp and corresponding header own real receiver/dependencies; count-zero copy underflow is intentionally unchanged."], "reason": "Complete typed application helper; retain original call ordering, argument contract and historical obligation."}
// symbol    ?sub_584d30@@YAPADPAD0I@Z
char *__cdecl sub_584d30(char *dest, char *src, unsigned int count) {
    char *result = strncpy(dest, src, count);
    dest[count - 1] = 0;
    return result;
}
/*
Purpose: Set the global gender and plurality variables used by various parse functions.
// ORIGINAL: 0x005A58E0 ?parse_set@@YAXHH@Z 0x005A58E0-0x005A58F6 BYTE_EXACT
// body      src/general.h
// size      22 bytes
// prototype 
// callers   1   call targets   0
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
// BODY IN general.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.

// ORIGINAL: 0x005BECA0 ?X_text_open@@YAHPADPBD@Z 0x005BECA0-0x005BED17 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["the current helper calls only text_open and compares the requested file to the process script-file pointer", "ClimatePopupTitle aliases &ScriptPopupFile; 0x00939284 is ConsoleGlobal.faction_id_; 0x00946f58 is Players[0].rule_flags with 0x59c stride", "src/game.h, src/console.h, src/faction.h and src/popup.h declare the corresponding real objects and flags; unrelated leaf bodies still use the two TU-local aliases, which this transfer leaves in place", "the current function checks only the local faction's 0x80 rule bit and ExpansionEnabled, tries PopupStartAltFile1, then falls back to the original requested file"], "reason": "Home the original section-opening policy beside Text::open/text_open, replacing transitional pointer aliases with existing source-owned script, local-faction, player-rule, expansion, and alternate-file objects without changing branch order."}
// size      119 bytes
// prototype int (__cdecl ?X_text_open@@YAHPADPBD@Z)(int8* srcFileID, int8* sectionID)
// callers   19   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005FD550 0x00645660
// notes     Seed provenance: the LEVER note records that the entry frame is
//           downstream of the strcmp intrinsic - inlining a string routine
//           forced esi/edi saves - and that the two globals are PTR_s_
//           slots holding a char*, one dereference more than the string.
// historical Return Value: text_open's result for the section
// historical Status: Measured BYTE_EXACT here in place (wave 7 record-then-retire); the
// LEVER: string-routines-are-calls "Original keeps a `push ebp; mov ebp, esp` frame this source form never reproduces (register-pressure shape, not yet found)" was the note here, and the shape has now been found: the scaffold inlined a string routine the image calls, the expansion needs esi and edi, and VC6 answered by changing what it saved on entry. The prologue was downstream of the intrinsic. The `g_00691b0c`/`g_00691b20` reading stands - they are PTR_s_ globals holding a `char*`, one dereference more than the address of the string itself.
// auto_inline(off): parse_it (0x0048C060) calls X_text_open (0x005BECA0) by
// `call rel32`; the two images are far apart, so the shared reconstruction TU
// is not original. Without the pragma /Ob2 folds this body into parse_it.
#pragma auto_inline(off)
#pragma optimize("y", off)
#pragma inline_depth(0)
int __cdecl X_text_open(char *src_file_id, const char *section_id) {
    if (strcmp(src_file_id, ScriptPopupFile) != 0) { return text_open(src_file_id, section_id); }
    const int faction_id = ConsoleGlobal.faction_id_;
    if ((Players[faction_id].rule_flags & RFLAG_ALIEN) != 0 && ExpansionEnabled != 0) {
        if (text_open(PopupStartAltFile1, section_id) == 0) { return 0; }
    }
    return text_open(src_file_id, section_id);
}
#pragma inline_depth(8)
#pragma optimize("y", on)
#pragma auto_inline(on)
