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


