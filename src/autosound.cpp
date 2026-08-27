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
#include "autosound.h"

// 0x009BC080, in .data's ZERO-FILL tail - the shipped file carries no
// bytes here. The FILLER is measured: FX::init (0x00445CD0) writes the
// entries before any AutoSound is constructed, so this is runtime
// storage, never compile-time data.
uint32_t AutoSoundDefaults[33];


/*
Purpose: Construct an AutoSound by installing its virtual table and copying
         the complete process-default block in legacy store order.
// ORIGINAL: 0x0062BA80 ??0AutoSound@@QAE@XZ 0x0062BA80-0x0062BBE8 BYTE_EXACT
// LEVER: TWO changes, both needed: (1) `construct()` returns `this` rather than void, and (2) the 33-element tail is unrolled into named field assignments. MISMATCH 0/77 -> BYTE_EXACT 77/77. Measured separately: unrolling alone still scores 0 of 77 with 76 compiled instructions, because a void __thiscall keeps the object base in ECX and uses EAX/EDX as the load scratch, where the image opens `mov eax, ecx` and threads every one of the 38 stores through EAX with ECX/EDX as scratch - the register assignment a __thiscall that must leave `this` in EAX produces. The missing 77th instruction is that opening `mov eax, ecx`. Same unroll finding as init() and close() in this file.
// symbol    ??0AutoSound@@QAE@XZ
// size      360 bytes
// prototype void (__thiscall ??0AutoSound@@QAE@XZ)(AutoSound* this)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
AutoSound::AutoSound() {
    val_1_ = static_cast<int>(AutoSoundDefaults[0]);
    val_3_ = static_cast<int>(AutoSoundDefaults[1]);
    val_4_ = static_cast<int>(AutoSoundDefaults[2]);
    val_2_ = static_cast<int>(AutoSoundDefaults[3]);
    val_5_ = static_cast<int>(AutoSoundDefaults[4]);
    val_6_ = static_cast<int>(AutoSoundDefaults[5]);
    val_7_ = reinterpret_cast<Win *>(AutoSoundDefaults[6]);
    flags_ = static_cast<int>(AutoSoundDefaults[7]);
    val_9_ = static_cast<int>(AutoSoundDefaults[8]);
    val_10_ = static_cast<int>(AutoSoundDefaults[9]);
    val_11_ = static_cast<int>(AutoSoundDefaults[10]);
    val_12_ = static_cast<int>(AutoSoundDefaults[11]);
    val_13_ = static_cast<int>(AutoSoundDefaults[12]);
    val_14_ = static_cast<int>(AutoSoundDefaults[13]);
    val_15_ = static_cast<int>(AutoSoundDefaults[14]);
    val_16_ = static_cast<int>(AutoSoundDefaults[15]);
    val_17_ = static_cast<int>(AutoSoundDefaults[16]);
    val_18_ = static_cast<int>(AutoSoundDefaults[17]);
    val_19_ = reinterpret_cast<Win *>(AutoSoundDefaults[18]);
    val_20_ = static_cast<int>(AutoSoundDefaults[19]);
    val_21_ = static_cast<int>(AutoSoundDefaults[20]);
    val_22_ = static_cast<int>(AutoSoundDefaults[21]);
    val_23_ = static_cast<int>(AutoSoundDefaults[22]);
    val_24_ = static_cast<int>(AutoSoundDefaults[23]);
    val_25_ = static_cast<int>(AutoSoundDefaults[24]);
    val_26_ = static_cast<int>(AutoSoundDefaults[25]);
    val_27_ = static_cast<int>(AutoSoundDefaults[26]);
    val_28_ = static_cast<int>(AutoSoundDefaults[27]);
    val_29_ = static_cast<int>(AutoSoundDefaults[28]);
    val_30_ = static_cast<int>(AutoSoundDefaults[29]);
    val_31_ = static_cast<int>(AutoSoundDefaults[30]);
    val_32_ = static_cast<int>(AutoSoundDefaults[31]);
    val_33_ = static_cast<int>(AutoSoundDefaults[32]);
    val_34_ = static_cast<int>(AutoSoundDefaults[33]);
    val_35_ = static_cast<int>(AutoSoundDefaults[34]);
    val_36_ = static_cast<int>(AutoSoundDefaults[35]);
    val_37_ = static_cast<int>(AutoSoundDefaults[36]);
}


/*
Purpose: Reset every field from the process-default block, in the same
         legacy store order as construction but without touching the
         virtual table.
// ORIGINAL: 0x0062BBF0 ?close@AutoSound@@QAEXXZ 0x0062BBF0-0x0062BD3D BYTE_EXACT
// LEVER: unrolled the `for (index = 4; index < 37)` tail into 33 named field assignments: MISMATCH 8/75 -> BYTE_EXACT 75/75. VC6 compiles the loop as 15 instructions stepping an address (`mov eax, 0x9bc090; mov edx, [eax]; ... add eax, 4`), where the image is 37 straight `mov reg, [0x9bc0NN] / mov [ecx + 0xNN], reg` pairs. Same finding init() already carried for its constant fill; measured here for a COPY, so it is not specific to a constant. The `volatile` on the object and defaults aliases is not what mattered - object_plain/object_volatile and AutoSoundDefaults-direct/local-copy all four reach 75/75 once the loop is gone; named fields were chosen because they say what is being written.
// size      333 bytes
// prototype void (__thiscall ?close@AutoSound@@QAEXXZ)(AutoSound* this)
// callers   3   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void AutoSound::close() {
    // Spelled out, NOT a loop - the same finding init() already carries.
    // VC6 keeps the 33-element tail as an address-stepping loop and never
    // unrolls it; the image is 333 bytes of straight-line load/store pairs.
    val_1_ = static_cast<int>(AutoSoundDefaults[0]);
    val_3_ = static_cast<int>(AutoSoundDefaults[1]);
    val_4_ = static_cast<int>(AutoSoundDefaults[2]);
    val_2_ = static_cast<int>(AutoSoundDefaults[3]);
    val_5_ = static_cast<int>(AutoSoundDefaults[4]);
    val_6_ = static_cast<int>(AutoSoundDefaults[5]);
    val_7_ = reinterpret_cast<Win *>(AutoSoundDefaults[6]);
    flags_ = static_cast<int>(AutoSoundDefaults[7]);
    val_9_ = static_cast<int>(AutoSoundDefaults[8]);
    val_10_ = static_cast<int>(AutoSoundDefaults[9]);
    val_11_ = static_cast<int>(AutoSoundDefaults[10]);
    val_12_ = static_cast<int>(AutoSoundDefaults[11]);
    val_13_ = static_cast<int>(AutoSoundDefaults[12]);
    val_14_ = static_cast<int>(AutoSoundDefaults[13]);
    val_15_ = static_cast<int>(AutoSoundDefaults[14]);
    val_16_ = static_cast<int>(AutoSoundDefaults[15]);
    val_17_ = static_cast<int>(AutoSoundDefaults[16]);
    val_18_ = static_cast<int>(AutoSoundDefaults[17]);
    val_19_ = reinterpret_cast<Win *>(AutoSoundDefaults[18]);
    val_20_ = static_cast<int>(AutoSoundDefaults[19]);
    val_21_ = static_cast<int>(AutoSoundDefaults[20]);
    val_22_ = static_cast<int>(AutoSoundDefaults[21]);
    val_23_ = static_cast<int>(AutoSoundDefaults[22]);
    val_24_ = static_cast<int>(AutoSoundDefaults[23]);
    val_25_ = static_cast<int>(AutoSoundDefaults[24]);
    val_26_ = static_cast<int>(AutoSoundDefaults[25]);
    val_27_ = static_cast<int>(AutoSoundDefaults[26]);
    val_28_ = static_cast<int>(AutoSoundDefaults[27]);
    val_29_ = static_cast<int>(AutoSoundDefaults[28]);
    val_30_ = static_cast<int>(AutoSoundDefaults[29]);
    val_31_ = static_cast<int>(AutoSoundDefaults[30]);
    val_32_ = static_cast<int>(AutoSoundDefaults[31]);
    val_33_ = static_cast<int>(AutoSoundDefaults[32]);
    val_34_ = static_cast<int>(AutoSoundDefaults[33]);
    val_35_ = static_cast<int>(AutoSoundDefaults[34]);
    val_36_ = static_cast<int>(AutoSoundDefaults[35]);
    val_37_ = static_cast<int>(AutoSoundDefaults[36]);
}


/*
Purpose: Reset every field from the process-default block, exactly as close
         does but storing in ascending field order - the two differ only in
         which of the shuffled leading fields lands first.
// ORIGINAL: 0x0062BDD0 ?close2@AutoSound@@QAEXXZ 0x0062BDD0-0x0062BF1D BYTE_EXACT
// LEVER: unrolled the `for` tail into 33 named field assignments, exactly as close() above - the loop compiles to 15 address-stepping instructions where the image is 37 straight load/store pairs.
// size      333 bytes
// prototype void (__thiscall ?close2@AutoSound@@QAEXXZ)(AutoSound* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void AutoSound::close2() {
    // Spelled out, NOT a loop - see close() above.
    val_1_ = static_cast<int>(AutoSoundDefaults[0]);
    val_2_ = static_cast<int>(AutoSoundDefaults[3]);
    val_3_ = static_cast<int>(AutoSoundDefaults[1]);
    val_4_ = static_cast<int>(AutoSoundDefaults[2]);
    val_5_ = static_cast<int>(AutoSoundDefaults[4]);
    val_6_ = static_cast<int>(AutoSoundDefaults[5]);
    val_7_ = reinterpret_cast<Win *>(AutoSoundDefaults[6]);
    flags_ = static_cast<int>(AutoSoundDefaults[7]);
    val_9_ = static_cast<int>(AutoSoundDefaults[8]);
    val_10_ = static_cast<int>(AutoSoundDefaults[9]);
    val_11_ = static_cast<int>(AutoSoundDefaults[10]);
    val_12_ = static_cast<int>(AutoSoundDefaults[11]);
    val_13_ = static_cast<int>(AutoSoundDefaults[12]);
    val_14_ = static_cast<int>(AutoSoundDefaults[13]);
    val_15_ = static_cast<int>(AutoSoundDefaults[14]);
    val_16_ = static_cast<int>(AutoSoundDefaults[15]);
    val_17_ = static_cast<int>(AutoSoundDefaults[16]);
    val_18_ = static_cast<int>(AutoSoundDefaults[17]);
    val_19_ = reinterpret_cast<Win *>(AutoSoundDefaults[18]);
    val_20_ = static_cast<int>(AutoSoundDefaults[19]);
    val_21_ = static_cast<int>(AutoSoundDefaults[20]);
    val_22_ = static_cast<int>(AutoSoundDefaults[21]);
    val_23_ = static_cast<int>(AutoSoundDefaults[22]);
    val_24_ = static_cast<int>(AutoSoundDefaults[23]);
    val_25_ = static_cast<int>(AutoSoundDefaults[24]);
    val_26_ = static_cast<int>(AutoSoundDefaults[25]);
    val_27_ = static_cast<int>(AutoSoundDefaults[26]);
    val_28_ = static_cast<int>(AutoSoundDefaults[27]);
    val_29_ = static_cast<int>(AutoSoundDefaults[28]);
    val_30_ = static_cast<int>(AutoSoundDefaults[29]);
    val_31_ = static_cast<int>(AutoSoundDefaults[30]);
    val_32_ = static_cast<int>(AutoSoundDefaults[31]);
    val_33_ = static_cast<int>(AutoSoundDefaults[32]);
    val_34_ = static_cast<int>(AutoSoundDefaults[33]);
    val_35_ = static_cast<int>(AutoSoundDefaults[34]);
    val_36_ = static_cast<int>(AutoSoundDefaults[35]);
    val_37_ = static_cast<int>(AutoSoundDefaults[36]);
}


/*
Purpose: Zero every field, in the same legacy store order as construction,
         leaving the virtual table alone.
// ORIGINAL: 0x0062BD40 ?init@AutoSound@@QAEXXZ 0x0062BD40-0x0062BDC4 BYTE_EXACT
// size      132 bytes
// prototype void (__thiscall ?init@AutoSound@@QAEXXZ)(AutoSound* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void AutoSound::init() {
    // Spelled out, NOT a loop. A constant fill makes no store order
    // observable, so a loop is the natural source - but VC6 keeps a constant
    // fill as an address-stepping loop of 31 bytes and never unrolls it,
    // where the original is 132 bytes of straight-line stores. The odd
    // 1,3,4,2 opening is the order construct() copies in.
    val_1_ = 0;
    val_3_ = 0;
    val_4_ = 0;
    val_2_ = 0;
    val_5_ = 0;
    val_6_ = 0;
    val_7_ = 0;
    flags_ = 0;
    val_9_ = 0;
    val_10_ = 0;
    val_11_ = 0;
    val_12_ = 0;
    val_13_ = 0;
    val_14_ = 0;
    val_15_ = 0;
    val_16_ = 0;
    val_17_ = 0;
    val_18_ = 0;
    val_19_ = 0;
    val_20_ = 0;
    val_21_ = 0;
    val_22_ = 0;
    val_23_ = 0;
    val_24_ = 0;
    val_25_ = 0;
    val_26_ = 0;
    val_27_ = 0;
    val_28_ = 0;
    val_29_ = 0;
    val_30_ = 0;
    val_31_ = 0;
    val_32_ = 0;
    val_33_ = 0;
    val_34_ = 0;
    val_35_ = 0;
    val_36_ = 0;
    val_37_ = 0;
}


/*
Purpose: The compiler-generated scalar deleting destructor: re-install the
         virtual table, reset the fields through close, and, when bit 0 of
         the mode asks, free the storage to the game heap.
// ORIGINAL: 0x005F8640 ??_GAutoSound@@UAEPAXI@Z 0x005F8640-0x005F8664 BYTE_EXACT
// size      36 bytes
// prototype void* (__thiscall ??_GAutoSound@@UAEPAXI@Z)(AutoSound* this, unsigned int)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0062BBF0 0x0064557F
Return Value: the object pointer
Status: Complete
*/

/*
Purpose: Legacy sound processing hook retained as a no-op.
// ORIGINAL: 0x005FD2B0 ?do_sound@@YAHXZ 0x005FD2B0-0x005FD2B3 BYTE_EXACT
// CORRECTED from ?do_sound@@YAXXZ
//   body is `xor eax, eax; ret` - a void function would emit `ret` alone
// size      3 bytes
// prototype 
// callers   21   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Return Value: zero
Status: Complete
*/
// BODY IN autosound.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


