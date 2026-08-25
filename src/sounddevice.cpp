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
#include "original_seam.h"
#include "sounddevice.h"
#include "dialog.h"

// init() for both device classes is reached by init_sound().
//
// A REAL local vtable, not the vtable_method/ORIGINAL seam used elsewhere in
// this file: the seam's extra indirection left window/backends and the
// vtable read on the OPPOSITE registers from the image (mnemonic-identical,
// byte-different). This class is never instantiated, so its own
// VC6-synthesised vtable dispatch is exactly the image's `call [reg+N]`.
namespace {
class DeviceVCall { public:
    virtual int slot000(unsigned long);
    virtual void slot001();
    virtual void slot002();
    virtual int slot003(void *, unsigned long);
};
}  // namespace

/*
Purpose: Select the device (vtable slot 0), then open the wrapped device at
         this+0x14 (vtable slot 0xC) with the given window and backends. On
         failure to open, restart (vtable slot 4) and clear the wrapped
         device pointer before reporting the failure.
// ORIGINAL: 0x004C57A0 ?init@Midi_Device@@QAEHPAXK@Z 0x004C57A0-0x004C57E4 BYTE_EXACT
// size      68 bytes
// prototype int (__thiscall ?init@Midi_Device@@QAEHPAXK@Z)(Midi_Device* this, void*, unsigned long)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C57AD 0x004C57C5 0x004C57D5
Return Value: 0 on success; the failing call's own non-zero result otherwise
Status: Complete
*/
int Midi_Device::init(void *window, unsigned long backends) {
    int result = reinterpret_cast<DeviceVCall *>(this)->slot000(backends);
    if (result != 0) {
        return result;
    }
    DeviceVCall *sub = *reinterpret_cast<DeviceVCall **>(
        reinterpret_cast<uint8_t *>(this) + 0x14);
    int result2 = sub->slot003(window, backends);
    if (result2 != 0) {
        reinterpret_cast<DeviceVCall *>(this)->slot001();
        *reinterpret_cast<DeviceVCall **>(
            reinterpret_cast<uint8_t *>(this) + 0x14) = 0;
        return result2;
    }
    return 0;
}

/*
Purpose: Select the device (vtable slot 0), then open the wrapped device at
         this+0x14 (vtable slot 0xC) with the given window and backends. On
         failure to open, restart (vtable slot 4) and report the failure -
         unlike Midi_Device::init above, the wrapped device pointer is left
         alone.
// ORIGINAL: 0x004C5A10 ?init@Wave_In_Device@@QAEHPAXK@Z 0x004C5A10-0x004C5A4D BYTE_EXACT
// size      61 bytes
// prototype int (__thiscall ?init@Wave_In_Device@@QAEHPAXK@Z)(Wave_In_Device* this, void*, unsigned long)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C5A1D 0x004C5A2D 0x004C5A3A
Return Value: 0 on success; the failing call's own non-zero result otherwise
Status: Complete
*/
int Wave_In_Device::init(void *window, unsigned long backends) {
    int result = reinterpret_cast<DeviceVCall *>(this)->slot000(backends);
    if (result != 0) {
        return result;
    }
    DeviceVCall *sub = *reinterpret_cast<DeviceVCall **>(
        reinterpret_cast<uint8_t *>(this) + 0x14);
    int result2 = sub->slot003(window, backends);
    if (result2 != 0) {
        reinterpret_cast<DeviceVCall *>(this)->slot001();
        return result2;
    }
    return 0;
}


/*
Purpose: Midi_Device devices decline to be polled; the legacy implementation is a bare return.
// ORIGINAL: 0x004C58B0 ?update_sound@Midi_Device@@QAEXXZ 0x004C58B0-0x004C58B1 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?update_sound@Midi_Device@@QAEXXZ)(Midi_Device* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Midi_Device::update_sound() {
}

/*
Purpose: Midi_Device devices decline to be suspended; the legacy implementation is a bare return.
// ORIGINAL: 0x004C58E0 ?suspend@Midi_Device@@QAEXXZ 0x004C58E0-0x004C58E1 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?suspend@Midi_Device@@QAEXXZ)(Midi_Device* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Midi_Device::suspend() {
}

/*
Purpose: Midi_Device devices decline to be restarted; the legacy implementation is a bare return.
// ORIGINAL: 0x004C58F0 ?restart@Midi_Device@@QAEXXZ 0x004C58F0-0x004C58F1 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?restart@Midi_Device@@QAEXXZ)(Midi_Device* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Midi_Device::restart() {
}

/*
Purpose: Wave_In_Device devices decline to be polled; the legacy implementation is a bare return.
// ORIGINAL: 0x004C5B10 ?update_sound@Wave_In_Device@@QAEXXZ 0x004C5B10-0x004C5B11 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?update_sound@Wave_In_Device@@QAEXXZ)(Wave_In_Device* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Wave_In_Device::update_sound() {
}

/*
Purpose: Wave_In_Device devices decline to be suspended; the legacy implementation is a bare return.
// ORIGINAL: 0x004C5B40 ?suspend@Wave_In_Device@@QAEXXZ 0x004C5B40-0x004C5B41 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?suspend@Wave_In_Device@@QAEXXZ)(Wave_In_Device* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Wave_In_Device::suspend() {
}

/*
Purpose: Wave_In_Device devices decline to be restarted; the legacy implementation is a bare return.
// ORIGINAL: 0x004C5B50 ?restart@Wave_In_Device@@QAEXXZ 0x004C5B50-0x004C5B51 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?restart@Wave_In_Device@@QAEXXZ)(Wave_In_Device* this)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Wave_In_Device::restart() {
}







/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns 0.
// ORIGINAL: 0x004C5840 ?select@Midi_Device@@QAEHI@Z 0x004C5840-0x004C5845 BYTE_EXACT
// size      5 bytes
// prototype int (__thiscall ?select@Midi_Device@@QAEHI@Z)(Midi_Device* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0, always
Status: Complete
*/
int Midi_Device::select(unsigned int) {
    return 0;
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x004C5860 ?set_volume@Midi_Device@@QAEXI@Z 0x004C5860-0x004C5863 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?set_volume@Midi_Device@@QAEXI@Z)(Midi_Device* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Midi_Device::set_volume(unsigned int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x004C5880 ?set_pan@Midi_Device@@QAEXH@Z 0x004C5880-0x004C5883 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?set_pan@Midi_Device@@QAEXH@Z)(Midi_Device* this, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Midi_Device::set_pan(int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns 0.
// ORIGINAL: 0x004C5890 ?fade@Midi_Device@@QAEHI@Z 0x004C5890-0x004C5895 BYTE_EXACT
// size      5 bytes
// prototype int (__thiscall ?fade@Midi_Device@@QAEHI@Z)(Midi_Device* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0, always
Status: Complete
*/
int Midi_Device::fade(unsigned int) {
    return 0;
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x004C58C0 ?set_rate@Midi_Device@@QAEXI@Z 0x004C58C0-0x004C58C3 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?set_rate@Midi_Device@@QAEXI@Z)(Midi_Device* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Midi_Device::set_rate(unsigned int) {
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns 0.
// ORIGINAL: 0x004C5AD0 ?select@Wave_In_Device@@QAEHI@Z 0x004C5AD0-0x004C5AD5 BYTE_EXACT
// size      5 bytes
// prototype int (__thiscall ?select@Wave_In_Device@@QAEHI@Z)(Wave_In_Device* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0, always
Status: Complete
*/
int Wave_In_Device::select(unsigned int) {
    return 0;
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns 0.
// ORIGINAL: 0x004C5AF0 ?get_caps@Wave_In_Device@@QAEHI@Z 0x004C5AF0-0x004C5AF5 BYTE_EXACT
// size      5 bytes
// prototype int (__thiscall ?get_caps@Wave_In_Device@@QAEHI@Z)(Wave_In_Device* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0, always
Status: Complete
*/
int Wave_In_Device::get_caps(unsigned int) {
    return 0;
}


/*
Purpose: Unknown; the legacy implementation ignores its arguments and returns.
// ORIGINAL: 0x004C5B20 ?set_rate@Wave_In_Device@@QAEXI@Z 0x004C5B20-0x004C5B23 BYTE_EXACT
// size      3 bytes
// prototype void (__thiscall ?set_rate@Wave_In_Device@@QAEXI@Z)(Wave_In_Device* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void Wave_In_Device::set_rate(unsigned int) {
}


/*
Purpose: Report no device description; the legacy implementation returns 0.
// ORIGINAL: 0x004C5850 ?get_description@Midi_Device@@QAEHIPADI@Z 0x004C5850-0x004C5855 BYTE_EXACT
// size      5 bytes
// prototype int (__thiscall ?get_description@Midi_Device@@QAEHIPADI@Z)(Midi_Device* this, unsigned int, int8*, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0, always
Status: Complete
*/
int Midi_Device::get_description(unsigned int, char *, unsigned int) {
    return 0;
}


/*
Purpose: Report no device description; the legacy implementation returns 0.
// ORIGINAL: 0x004C5AE0 ?get_description@Wave_In_Device@@QAEHIPADI@Z 0x004C5AE0-0x004C5AE5 BYTE_EXACT
// size      5 bytes
// prototype int (__thiscall ?get_description@Wave_In_Device@@QAEHIPADI@Z)(Wave_In_Device* this, unsigned int, int8*, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0, always
Status: Complete
*/
int Wave_In_Device::get_description(unsigned int, char *, unsigned int) {
    return 0;
}


/*
Purpose: Unknown; the legacy implementation is a constant return that returns 0.
// ORIGINAL: 0x004C5830 ?get_ndevices@Midi_Device@@QAEHXZ 0x004C5830-0x004C5833 BYTE_EXACT
// size      3 bytes
// prototype int (__thiscall ?get_ndevices@Midi_Device@@QAEHXZ)(Midi_Device* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0, always
Status: Complete
*/
int Midi_Device::get_ndevices() {
    return 0;
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns 0.
// ORIGINAL: 0x004C5870 ?get_volume@Midi_Device@@QAEHXZ 0x004C5870-0x004C5873 BYTE_EXACT
// size      3 bytes
// prototype int (__thiscall ?get_volume@Midi_Device@@QAEHXZ)(Midi_Device* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0, always
Status: Complete
*/
int Midi_Device::get_volume() {
    return 0;
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns 0.
// ORIGINAL: 0x004C58A0 ?stop@Midi_Device@@QAEHXZ 0x004C58A0-0x004C58A3 BYTE_EXACT
// size      3 bytes
// prototype int (__thiscall ?stop@Midi_Device@@QAEHXZ)(Midi_Device* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0, always
Status: Complete
*/
int Midi_Device::stop() {
    return 0;
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns 0.
// ORIGINAL: 0x004C58D0 ?get_rate@Midi_Device@@QAEHXZ 0x004C58D0-0x004C58D3 BYTE_EXACT
// size      3 bytes
// prototype int (__thiscall ?get_rate@Midi_Device@@QAEHXZ)(Midi_Device* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0, always
Status: Complete
*/
int Midi_Device::get_rate() {
    return 0;
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns 0.
// ORIGINAL: 0x004C5AC0 ?get_ndevices@Wave_In_Device@@QAEHXZ 0x004C5AC0-0x004C5AC3 BYTE_EXACT
// size      3 bytes
// prototype int (__thiscall ?get_ndevices@Wave_In_Device@@QAEHXZ)(Wave_In_Device* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0, always
Status: Complete
*/
int Wave_In_Device::get_ndevices() {
    return 0;
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns 0.
// ORIGINAL: 0x004C5B00 ?stop@Wave_In_Device@@QAEHXZ 0x004C5B00-0x004C5B03 BYTE_EXACT
// size      3 bytes
// prototype int (__thiscall ?stop@Wave_In_Device@@QAEHXZ)(Wave_In_Device* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0, always
Status: Complete
*/
int Wave_In_Device::stop() {
    return 0;
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns 0.
// ORIGINAL: 0x004C5B30 ?get_rate@Wave_In_Device@@QAEHXZ 0x004C5B30-0x004C5B33 BYTE_EXACT
// size      3 bytes
// prototype int (__thiscall ?get_rate@Wave_In_Device@@QAEHXZ)(Wave_In_Device* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0, always
Status: Complete
*/
int Wave_In_Device::get_rate() {
    return 0;
}








namespace {
// Midi_Device wraps its device at 0x14, the same offset Wave_Device uses, and
// drives it through the device's own vtable.
typedef void (OriginalObject::*midi_device_vfn)();

void dispatch_midi_device(Midi_Device *self, int vtable_offset) {
    void *device = *reinterpret_cast<void **>(
        reinterpret_cast<uint8_t *>(self) + 0x14);
    if (device) {
        uint8_t *vtable = *reinterpret_cast<uint8_t **>(device);
        (ORIGINAL(device)->*vtable_method<midi_device_vfn>(device, vtable_offset))();
    }
}
}  // namespace

/*
Purpose: Enable the wrapped device, if there is one, through vtable slot 0x54.
// ORIGINAL: 0x004C5900 ?enable@Midi_Device@@QAEXXZ 0x004C5900-0x004C590F
// size      15 bytes
// prototype void (__thiscall ?enable@Midi_Device@@QAEXXZ)(Midi_Device* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C5909
Return Value: n/a
Status: Complete
*/
void Midi_Device::enable() {
    dispatch_midi_device(this, 0x54);
}

/*
Purpose: Disable the wrapped device, if there is one, through vtable slot 0x58.
// ORIGINAL: 0x004C5910 ?disable@Midi_Device@@QAEXXZ 0x004C5910-0x004C591F
// size      15 bytes
// prototype void (__thiscall ?disable@Midi_Device@@QAEXXZ)(Midi_Device* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C5919
Return Value: n/a
Status: Complete
*/
void Midi_Device::disable() {
    dispatch_midi_device(this, 0x58);
}



namespace {
typedef int (OriginalObject::*device_query_vfn)();

// The querying form of the same dispatch, shared by both device classes: the
// original tail-jumps or calls into the device's method, and answers a fixed
// value when no device is wrapped.
int query_device_at_14(void *self, int vtable_offset, int no_device_result) {
    void *device = *reinterpret_cast<void **>(
        reinterpret_cast<uint8_t *>(self) + 0x14);
    if (!device) {
        return no_device_result;
    }
    uint8_t *vtable = *reinterpret_cast<uint8_t **>(device);
    return (ORIGINAL(device)->*vtable_method<device_query_vfn>(device, vtable_offset))();
}
}  // namespace

/*
Purpose: Ask the wrapped device whether it is disabled, through vtable slot
         0x5C. With no device wrapped the answer is yes.
// ORIGINAL: 0x004C5920 ?is_disabled@Midi_Device@@QAEHXZ 0x004C5920-0x004C5932
// size      18 bytes
// prototype int (__thiscall ?is_disabled@Midi_Device@@QAEHXZ)(Midi_Device* this)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: the device's answer, or 1 when none is wrapped
Status: Complete
*/
int Midi_Device::is_disabled() {
    return query_device_at_14(this, 0x5C, 1);
}

/*
Purpose: Ask the wrapped device to start recording, through vtable slot 0x58.
         The original discards its answer.
// ORIGINAL: 0x004C5B60 ?start_record@Wave_In_Device@@QAEHXZ 0x004C5B60-0x004C5B6F
// size      15 bytes
// prototype int (__thiscall ?start_record@Wave_In_Device@@QAEHXZ)(Wave_In_Device* this)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C5B69
Return Value: 0, always
Status: Complete
*/
int Wave_In_Device::start_record() {
    query_device_at_14(this, 0x58, 0);
    return 0;
}

/*
Purpose: Ask the wrapped device to stop recording, through vtable slot 0x5C.
         The original discards its answer.
// ORIGINAL: 0x004C5B70 ?end_record@Wave_In_Device@@QAEHXZ 0x004C5B70-0x004C5B7F
// size      15 bytes
// prototype int (__thiscall ?end_record@Wave_In_Device@@QAEHXZ)(Wave_In_Device* this)
// callers   1   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
// indirect  0x004C5B79
Return Value: 0, always
Status: Complete
*/
int Wave_In_Device::end_record() {
    query_device_at_14(this, 0x5C, 0);
    return 0;
}





// ---------------------------------------------------------------------------
// DEFINED IN THE HEADER, CLAIMED HERE.
//
// These pieces are written in-class so the image's own inlining reproduces -
// a constructor or destructor the compiler is expected to fold into its
// caller. A marker cannot live beside them: `decomp.reader` globs `*.cpp`
// and `*.c`, and a comparison compiles a TRANSLATION UNIT, so a marker in a
// header could be neither read nor measured. VC6 emits each of them into
// this unit's object as its own COMDAT anyway, which is what the comparison
// pulls out, and the `body` fact says where to go to edit one.
//
// The ratchet still covers the header: this unit includes it, so breaking an
// in-class body here fails the claim below. Measured, not assumed.
// ---------------------------------------------------------------------------

/*
// ORIGINAL: 0x004C5740 ??0Midi_Device@@QAE@XZ 0x004C5740-0x004C577C BYTE_EXACT
// body      src/sounddevice.h
// size      60 bytes
// prototype void (__thiscall ??0Midi_Device@@QAE@XZ)(Midi_Device* this)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006465F0
*/

Midi_Device::Midi_Device() {
    // THE BASE VTABLE FIRST, THE DERIVED ONE LAST, with every scalar settled
    // in between - the same shape `Wave_Device::Wave_Device` next door already
    // reproduces. The image writes 0x0066E098, then the fields OUT of
    // declaration order, then 0x0066E190 over the top.
    vtable_storage_ = 0x0066E098;
    // A REAL `memset` CALL, not a store: the image emits
    // `push 4; push 0; push esi+4; call 0x6465f0` at 0x004C5753. The same
    // shape `Wave::init` needed for its own flag field.
    memset(&field_4_, 0, 4);
    field_C_ = 0;
    field_18_ = 0;
    field_1C_ = 0;
    field_10_ = 0;
    field_14_ = 0;
    field_8_ = 0x7F;
    vtable_storage_ = 0x0066E190;
}

/*
// ORIGINAL: 0x004C5940 ??0Wave_In_Device@@QAE@XZ 0x004C5940-0x004C597C BYTE_EXACT
// body      src/sounddevice.h
// size      60 bytes
// prototype void (__thiscall ??0Wave_In_Device@@QAE@XZ)(Wave_In_Device* this)
// callers   1   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x006465F0
*/

// LEVER: same shape as Midi_Device::Midi_Device() above - the BASE vtable
// goes in FIRST and the DERIVED one is written over it LAST, with the
// scalar fields settled OUT of declaration order in between, and
// `memset(&field_4_, 0, 4)` is a real `memset` CALL in the image
// (`push 4; push 0; push esi+4; call 0x6465f0`), not a store.
Wave_In_Device::Wave_In_Device() {
    vtable_storage_ = 0x0066E098;
    memset(&field_4_, 0, 4);
    field_C_ = 0;
    field_18_ = 0;
    field_1C_ = 0;
    field_10_ = 0;
    field_14_ = 0;
    field_8_ = 0x7F;
    vtable_storage_ = 0x0066E1F0;
}

/*
// ORIGINAL: 0x004C5780 ??1Midi_Device@@QAE@XZ 0x004C5780-0x004C5793
// body      src/sounddevice.h
// size      19 bytes
// prototype void (__thiscall ??1Midi_Device@@QAE@XZ)(Midi_Device* this)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
*/

/*
// ORIGINAL: 0x004C5980 ??1Wave_In_Device@@QAE@XZ 0x004C5980-0x004C5993
// body      src/sounddevice.h
// size      19 bytes
// prototype void (__thiscall ??1Wave_In_Device@@QAE@XZ)(Wave_In_Device* this)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
*/
