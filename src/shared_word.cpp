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
#include "shared_word.h"

// The original word's semantic domain is unknown; retain only its
// independently evidenced storage identity and operations.
// RECOVERY_OBJECT: v1 0x007591C4 ?SharedWordValue@@3HA
int32_t SharedWordValue;

/*
// ORIGINAL: 0x0044C860 sub_44c860 0x0044C860-0x0044C86D BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf:0x0044C860-0x0044C86D reads one int argument and stores its full dword to 0x007591C4", "pinned:terranx_original.exe:0x0044C870-0x0044C878 directly XORs bit zero of the same dword and returns", "pinned PE virtual .data section contains 0x007591C4; read_initial(0x007591C4..0x007591C8) is four zero bytes", "pinned base-relocation table identifies absolute operand sites in both original instructions; no relocation is claimed inside the target word", "src/shared_word.cpp owns the named SharedWordValue storage plus both original operations; no class, domain-level field meaning, or caller/lifetime relation is asserted"], "reason": "Own the original setter in the same C++ translation unit as the real shared word it writes and the paired toggle operation. The original class/domain meaning and caller set remain unknown."}
// symbol    ?set_shared_word@@YAXH@Z
// size      13 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/

void set_shared_word(int value) {
    SharedWordValue = value;
}

/*
Purpose: sub_44c870 - arithmetic over 1 fixed global(s), one
         statement per original instruction.
// ORIGINAL: 0x0044C870 sub_44c870 0x0044C870-0x0044C878 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:0x0044C870-0x0044C878 is the direct memory-form XOR `xor dword ptr [0x7591c4], 1; ret`", "pinned:terranx_original.exe:0x0044C860-0x0044C86D independently stores the caller-supplied dword at the same address", "pinned PE virtual .data section contains 0x007591C4; read_initial(0x007591C4..0x007591C8) is four zero bytes", "src/shared_word.cpp defines SharedWordValue and contains both paired source operations; no class or domain label is inferred"], "reason": "Own the original toggle operation beside the shared C++ word it mutates; use the same real storage as the original setter while leaving the state domain and callers unnamed."}
// Historical note: these spelling tests predate the paired setter and
// the parsed PE virtual zero-fill proof; retain their experiments, but
// the conclusion that 0x7591C4 has no named source object is superseded.
// TRIED 2026-09-03: five spellings for the single statement, scored with
// tools/try_spellings.py - `*p ^= 1`, `*p = *p ^ 1` (as committed), a
// `uint32_t *const p` local, `volatile int32_t *const p` read-modify-write
// and `volatile ... ^= 1`, plus `|= 1`. Every one still compiles the
// load/xor/store triple (3 or 4 instructions against the image's 2); none
// reaches the memory-form `xor dword ptr [0x7591c4], 1`. VC6 will not emit
// the in-memory read-modify-write through a const-pointer lvalue, only
// through a named global object, and these globals are bound rather than
// placed so no named object can live at 0x7591c4.
// TRIED 2026-09-03, second pass: the reference lvalue is the one new shape -
// `int32_t &r = *GlobalSlot007591C4; r = r ^ 1;` compiles `mov eax,<ptr>;
// xor [eax],1` (9 bytes against the image's 8), still not the memory form.
// Compiled the same statement against a REAL object in the same run as the
// control: `extern int x; x ^= 1` and `x = x ^ 1` BOTH compile to
// `xor dword ptr [<object>],1` - the image's exact 7 bytes, displacement
// relocated. So the memory form IS reachable from C++, and the one thing this
// body lacks is an object at 0x7591c4. That is an EVIDENCE lever, not a
// spelling lever: leaf_recoveries.cpp documents 0x7591C4 as evidence-free
// ("a named global here would be invention"), so it stays bound.
// symbol    ?toggle_shared_word@@YAXXZ
// size      8 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void toggle_shared_word() {
    SharedWordValue = SharedWordValue ^ 1;
}
