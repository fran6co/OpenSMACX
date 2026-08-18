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

const uint32_t AutoSoundVtable = 0x0066FF34;
uint32_t *AutoSoundDefaults = reinterpret_cast<uint32_t *>(0x009BC080);
func_auto_sound_delete *AutoSoundOperatorDelete = (func_auto_sound_delete *)0x0064557F;

/*
Purpose: Construct an AutoSound by installing its virtual table and copying
         the complete process-default block in legacy store order.
// ORIGINAL: 0x0062BA80 ??0AutoSound@@QAE@XZ 0x0062BA80-0x0062BBE8
// size      360 bytes
// prototype void (__thiscall ??0AutoSound@@QAE@XZ)(AutoSound* this)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
void AutoSound::construct() {
    volatile uint32_t *const object =
        reinterpret_cast<volatile uint32_t *>(this);
    volatile const uint32_t *const defaults = AutoSoundDefaults;
    object[0x00 / 4] = AutoSoundVtable;
    object[0x04 / 4] = defaults[0];
    object[0x0C / 4] = defaults[1];
    object[0x10 / 4] = defaults[2];
    object[0x08 / 4] = defaults[3];
    for (size_t index = 4; index < 37; ++index) {
        object[index + 1] = defaults[index];
    }
}

AutoSound *__fastcall auto_sound_construct_redirect(AutoSound *self, void *) {
    self->construct();
    return self;
}

/*
Purpose: Reset every field from the process-default block, in the same
         legacy store order as construction but without touching the
         virtual table.
// ORIGINAL: 0x0062BBF0 ?close@AutoSound@@QAEXXZ 0x0062BBF0-0x0062BD3D
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
    volatile uint32_t *const object =
        reinterpret_cast<volatile uint32_t *>(this);
    volatile const uint32_t *const defaults = AutoSoundDefaults;
    object[0x04 / 4] = defaults[0];
    object[0x0C / 4] = defaults[1];
    object[0x10 / 4] = defaults[2];
    object[0x08 / 4] = defaults[3];
    for (size_t index = 4; index < 37; ++index) {
        object[index + 1] = defaults[index];
    }
}

void __fastcall auto_sound_close_redirect(AutoSound *self, void *) {
    self->close();
}

/*
Purpose: Reset every field from the process-default block, exactly as close
         does but storing in ascending field order - the two differ only in
         which of the shuffled leading fields lands first.
// ORIGINAL: 0x0062BDD0 ?close2@AutoSound@@QAEXXZ 0x0062BDD0-0x0062BF1D
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
    volatile uint32_t *const object =
        reinterpret_cast<volatile uint32_t *>(this);
    volatile const uint32_t *const defaults = AutoSoundDefaults;
    object[0x04 / 4] = defaults[0];
    object[0x08 / 4] = defaults[3];
    object[0x0C / 4] = defaults[1];
    object[0x10 / 4] = defaults[2];
    for (size_t index = 4; index < 37; ++index) {
        object[index + 1] = defaults[index];
    }
}

void __fastcall auto_sound_close2_redirect(AutoSound *self, void *) {
    self->close2();
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
/*
Purpose: Zero every field, in the legacy store order construction uses,
         leaving the virtual table alone.
// ORIGINAL: 0x0062BD40 ?init@AutoSound@@QAEXXZ 0x0062BD40-0x0062BDC4
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
    val_8_ = 0;
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

void __fastcall auto_sound_init_redirect(AutoSound *self, void *) {
    self->init();
}

/*
Purpose: The compiler-generated scalar deleting destructor: re-install the
         virtual table, reset the fields through close, and, when bit 0 of
         the mode asks, free the storage to the game heap.
// ORIGINAL: 0x005F8640 ??_GAutoSound@@UAEPAXI@Z 0x005F8640-0x005F8664
// size      36 bytes
// prototype void* (__thiscall ??_GAutoSound@@UAEPAXI@Z)(AutoSound* this, unsigned int)
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0062BBF0 0x0064557F
Return Value: the object pointer
Status: Complete
*/
void *__fastcall auto_sound_scalar_dtor_redirect(AutoSound *self, void *,
                                                 unsigned int mode) {
    *reinterpret_cast<volatile uint32_t *>(self) = AutoSoundVtable;
    self->close();
    if (mode & 1) {
        AutoSoundOperatorDelete(self);
    }
    return self;
}

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
int __cdecl do_sound() {
    return 0;
}

int __cdecl do_sound_redirect() {
    return do_sound();
}
