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
#include "vector_teardown.h"

/*
Purpose: Initialize a strided array by calling one typed element callback
         per element. This original helper has no destructor
         parameter and performs no constructed-prefix unwind.
*/
// ORIGINAL: 0x00402EE0 sub_402ee0 0x00402EE0-0x00402F08 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned terranx_original.exe 01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf: 0x00402EE0-0x00402F08 is a four-argument stdcall helper; [ebp+8] is array, +0xC stride, +0x10 count and +0x14 callback code address.", "Original instructions decrement count, skip when the resulting sign bit is set, load the callback once, call it indirectly with ECX=current element, advance by the byte stride, and return with ret 0x10. The typed C++ callback preserves the element receiver; the caller ignores EAX.", "Original has one caller: NetWin::exec pushes 0x00483820, 7, 0x17C and its stack row array, then calls 0x00402EE0. The original callback's field stores match the existing NetWinLobbyRow::initialize_empty_slot owner.", "This is not CRT ??_L: there is no destructor callback argument and no constructed-prefix exception unwind. Callback exceptions propagate; no cleanup loop is added.", "The caller supplies &NetWinLobbyRow::initialize_empty_slot_callback, a real static __fastcall method that forwards the element to the existing member initializer. No raw code address, pointer-to-member pun, copied bytes or hand-authored thunk is used; native emitted identity remains unqualified."], "reason": "Recover the original strided array initializer in the shared vector utility owner with its four-argument stdcall stack shape, typed element callback, decrement/sign-test count behavior and no-unwind semantics. The NetWin stack-row caller supplies a genuine row-owned callback adapter."}
// symbol    _sub_402ee0@16
// size      40 bytes
// prototype
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x00402EF7
// 0x00402EE0  sub_402ee0  ->  _sub_402ee0
extern "C" void __stdcall sub_402ee0(
    void *array, unsigned int element_size, int count,
    func_vector_element_callback callback) {
    // The original decrements count, tests the result sign bit, then performs
    // exactly count callbacks for positive counts. Unsigned arithmetic keeps
    // its x86 wrap at INT_MIN rather than invoking C++ signed-overflow rules.
    unsigned int remaining = static_cast<unsigned int>(count) - 1u;
    if ((remaining & 0x80000000u) != 0) {
        return;
    }
    unsigned int iterations = remaining + 1u;
    char *element = static_cast<char *>(array);
    do {
        callback(element);
        element += element_size;
    } while (--iterations != 0);
}
