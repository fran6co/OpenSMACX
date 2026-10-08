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
// ORIGINAL: 0x004C5C80 ??__Eg_MIDI_DEVICE@@YAXXZ 0x004C5C80-0x004C5C96 FILE
// RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf: lifecycle helper direct receiver/constructor/destructor/atexit instructions", "docs/recovery/derived-prototypes.csv historical dynamic initializer/destructor identity; fresh native sounddevice.cpp COFF confirms object and lifecycle relocation targets"], "reason": "MIDI construction and atexit registration at original receiver 0x0090D950; emitted by ordinary source-owned global lifetime, not a handwritten wrapper; terminal alignment NOPs excluded"}
// symbol    _$E19
// ORIGINAL: 0x004C5CA0 ??__Fg_MIDI_DEVICE@@YAXXZ 0x004C5CA0-0x004C5CAA FILE
// RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf: lifecycle helper direct receiver/constructor/destructor/atexit instructions", "docs/recovery/derived-prototypes.csv historical dynamic initializer/destructor identity; fresh native sounddevice.cpp COFF confirms object and lifecycle relocation targets"], "reason": "MIDI static destruction tail call at original receiver 0x0090D950; emitted by ordinary source-owned global lifetime, not a handwritten wrapper; terminal alignment NOPs excluded"}
// symbol    _$E17
// ORIGINAL: 0x004C5CB0 ??__Eg_WAVE_IN_DEVICE@@YAXXZ 0x004C5CB0-0x004C5CC6 FILE
// RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf: lifecycle helper direct receiver/constructor/destructor/atexit instructions", "docs/recovery/derived-prototypes.csv historical dynamic initializer/destructor identity; fresh native sounddevice.cpp COFF confirms object and lifecycle relocation targets"], "reason": "Wave-In construction and atexit registration at original receiver 0x0090DB50; emitted by ordinary source-owned global lifetime, not a handwritten wrapper; terminal alignment NOPs excluded"}
// symbol    _$E24
// ORIGINAL: 0x004C5CD0 ??__Fg_WAVE_IN_DEVICE@@YAXXZ 0x004C5CD0-0x004C5CDA FILE
// RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf: lifecycle helper direct receiver/constructor/destructor/atexit instructions", "docs/recovery/derived-prototypes.csv historical dynamic initializer/destructor identity; fresh native sounddevice.cpp COFF confirms object and lifecycle relocation targets"], "reason": "Wave-In static destruction tail call at original receiver 0x0090DB50; emitted by ordinary source-owned global lifetime, not a handwritten wrapper; terminal alignment NOPs excluded"}
// symbol    _$E22
#include "sounddevice.h"
#include "sound.h"
#include "soundx_device_abi.h"
#include "dialog.h"

// Original startup helpers 4C5C80/4C5CB0 construct these receivers and register
// destructor callbacks 4C5CA0/4C5CD0. Use ordinary C++ static lifetime, not an
// executable-address alias or manual initialization. The touched 0x20 prefixes
// do not establish complete original allocation sizes; bindings are start-only.
// RECOVERY_OBJECT: v1 0x0090D950 ?g_MIDI_DEVICE@@3VMidi_Device@@A
Midi_Device g_MIDI_DEVICE;
// RECOVERY_OBJECT: v1 0x0090DB50 ?g_WAVE_IN_DEVICE@@3VWave_In_Device@@A
Wave_In_Device g_WAVE_IN_DEVICE;

namespace {
// The loader owns the eleven export cells at original 0x0090DB24.
// Keep references to its live entries: release callbacks can rebind the table.
inline MidiDeviceOpen *&MidiDeviceOpenSlot() {
    return *reinterpret_cast<MidiDeviceOpen **>(&SoundDllProcs[6]);
}
inline MidiDeviceClose *&MidiDeviceCloseSlot() {
    return *reinterpret_cast<MidiDeviceClose **>(&SoundDllProcs[7]);
}
inline WaveInDeviceOpen *&WaveInDeviceOpenSlot() {
    return *reinterpret_cast<WaveInDeviceOpen **>(&SoundDllProcs[8]);
}
inline WaveInDeviceClose *&WaveInDeviceCloseSlot() {
    return *reinterpret_cast<WaveInDeviceClose **>(&SoundDllProcs[9]);
}
}  // namespace

// init_sound() directly calls these concrete init methods. Each body invokes
// wrapper slots 0/1 and delegate slot 3 through their observed interfaces.

/*
Purpose: Select the device (vtable slot 0), then open the wrapped device at
         this+0x14 (vtable slot 0xC) with the given window and backends. On
         failure to open, call delete_device (vtable slot 1) and clear the wrapped
         device pointer before reporting the failure.
// ORIGINAL: 0x004C57A0 ?init@Midi_Device@@QAEHPAXK@Z 0x004C57A0-0x004C57E4 BYTE_EXACT
// symbol    ?init@Midi_Device@@UAEHPAXK@Z
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
    int result = create_device(backends);
    if (result != 0) {
        return result;
    }
    int result2 = SoundXDeviceAbi::init_device(device_14_, window, backends);
    if (result2 != 0) {
        delete_device();
        device_14_ = nullptr;
        return result2;
    }
    return 0;
}

/*
Purpose: Select the device (vtable slot 0), then open the wrapped device at
         this+0x14 (vtable slot 0xC) with the given window and backends. On
         failure to open, call delete_device (vtable slot 1) and report the failure -
         unlike Midi_Device::init above, the wrapped device pointer is left
         alone.
// ORIGINAL: 0x004C5A10 ?init@Wave_In_Device@@QAEHPAXK@Z 0x004C5A10-0x004C5A4D BYTE_EXACT
// symbol    ?init@Wave_In_Device@@UAEHPAXK@Z
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
    int result = create_device(backends);
    if (result != 0) {
        return result;
    }
    int result2 = SoundXDeviceAbi::init_device(device_14_, window, backends);
    if (result2 != 0) {
        delete_device();
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
// symbol    ?select@Midi_Device@@UAEHK@Z
// size      5 bytes
// prototype int (__thiscall ?select@Midi_Device@@QAEHI@Z)(Midi_Device* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0, always
Status: Complete
*/
int Midi_Device::select(unsigned long) {
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
// symbol    ?select@Wave_In_Device@@UAEHK@Z
// size      5 bytes
// prototype int (__thiscall ?select@Wave_In_Device@@QAEHI@Z)(Wave_In_Device* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0, always
Status: Complete
*/
int Wave_In_Device::select(unsigned long) {
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
// symbol    ?get_description@Midi_Device@@UAEHKPADK@Z
// size      5 bytes
// prototype int (__thiscall ?get_description@Midi_Device@@QAEHIPADI@Z)(Midi_Device* this, unsigned int, int8*, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0, always
Status: Complete
*/
int Midi_Device::get_description(unsigned long, char *, unsigned long) {
    return 0;
}


/*
Purpose: Report no device description; the legacy implementation returns 0.
// ORIGINAL: 0x004C5AE0 ?get_description@Wave_In_Device@@QAEHIPADI@Z 0x004C5AE0-0x004C5AE5 BYTE_EXACT
// symbol    ?get_description@Wave_In_Device@@UAEHKPADK@Z
// size      5 bytes
// prototype int (__thiscall ?get_description@Wave_In_Device@@QAEHIPADI@Z)(Wave_In_Device* this, unsigned int, int8*, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0, always
Status: Complete
*/
int Wave_In_Device::get_description(unsigned long, char *, unsigned long) {
    return 0;
}


/*
Purpose: Unknown; the legacy implementation is a constant return that returns 0.
// ORIGINAL: 0x004C5830 ?get_ndevices@Midi_Device@@QAEHXZ 0x004C5830-0x004C5833 BYTE_EXACT
// symbol    ?get_ndevices@Midi_Device@@UAEHXZ
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
// symbol    ?get_ndevices@Wave_In_Device@@UAEHXZ
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
// A PLAIN THISCALL-SHAPED FUNCTION POINTER, not an ORIGINAL()
// seam. sound.cpp proved this form compiles byte-identical, and
// unfactoring the helpers into five bodies through the SEAM would
// have multiplied `ORIGINAL() named-pointer seam` 96 -> 99. A
// recovery does not get to raise a ceiling to pay for itself.
typedef int(__fastcall *device_query_fn)(void *);
typedef void (OriginalObject::*midi_device_vfn)();

}  // namespace

// Disassembly at 0x004C4D70 enters with ECX as the receiver, has no stack
// argument reads, and returns ESI in EAX. The original class name is absent;
// SoundDeviceBase is a provisional source-owner name only. The same common
// initialization is inlined by each of the three concrete constructors.
// Retained seed facts: 54 original bytes at 0x004C4D70..0x004C4DA6, one call
// to the original four-byte memset helper 0x006465F0, no mapped callers.
// ORIGINAL: 0x004C4D70 sub_4c4d70 0x004C4D70-0x004C4DA6 BYTE_EXACT
// symbol    ??0SoundDeviceBase@@QAE@XZ
SoundDeviceBase::SoundDeviceBase() {
    memset(&field_4_, 0, 4);
    field_C_ = 0;
    field_18_ = 0;
    field_1C_ = 0;
    field_10_ = 0;
    device_14_ = nullptr;
    volume_8_ = 0x7F;
}

SoundDeviceBase::~SoundDeviceBase() {
    void *const device = device_14_;
    if (device) {
        SoundXDeviceAbi::release_device(device);
    }
}


/*
Purpose: Enable the wrapped device, if there is one, through vtable slot 0x54.
// ORIGINAL: 0x004C5900 ?enable@Midi_Device@@QAEXXZ 0x004C5900-0x004C590F BYTE_EXACT
// symbol    ?enable@Midi_Device@@QAEHXZ
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
int Midi_Device::enable() {
    void *device = device_14_;
    if (device) {
        vtable_slot<device_query_fn>(device, 0x54)(device);
    }
    // RETURNS 0, WHICH IS WHY THE IMAGE CANNOT TAIL-JUMP: it `call`s the
    // slot and then `xor eax, eax`, discarding the callee's answer. Declared
    // void, this compiles the jump and loses the last two instructions.
    return 0;
}

/*
Purpose: Disable the wrapped device, if there is one, through vtable slot 0x58.
// ORIGINAL: 0x004C5910 ?disable@Midi_Device@@QAEXXZ 0x004C5910-0x004C591F BYTE_EXACT
// symbol    ?disable@Midi_Device@@QAEHXZ
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
int Midi_Device::disable() {
    void *device = device_14_;
    if (device) {
        vtable_slot<device_query_fn>(device, 0x58)(device);
    }
    // RETURNS 0, WHICH IS WHY THE IMAGE CANNOT TAIL-JUMP: it `call`s the
    // slot and then `xor eax, eax`, discarding the callee's answer. Declared
    // void, this compiles the jump and loses the last two instructions.
    return 0;
}



namespace {
typedef int (OriginalObject::*device_query_vfn)();

}  // namespace

/*
Purpose: Ask the wrapped device whether it is disabled, through vtable slot
         0x5C. With no device wrapped the answer is yes.
// ORIGINAL: 0x004C5920 ?is_disabled@Midi_Device@@QAEHXZ 0x004C5920-0x004C5932 BYTE_EXACT
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
    void *device = device_14_;
    // THE DEVICE PATH FIRST AND THE DEFAULT LAST, which is the opposite of
    // what a guard clause writes: the image does `je` FORWARD to the
    // `mov eax, 1` and falls through into the call.
    if (device) {
        return vtable_slot<device_query_fn>(device, 0x5C)(device);
    }
    return 1;
}

// Original behavior: return 0x13 when the DLL-owned delegate is null;
// otherwise call its pinned slot and return that result.
// Original prototype fact: int (__thiscall ?set_codec@Wave_In_Device@@QAEHK@Z)(Wave_In_Device* this, unsigned int).
// Retained seed facts: 32-byte span; zero mapped callers; game;
// frame;hidden;sp_ready;purged_ok; calls none; indirect at
// 0x004C5A90. Historical seed Status: Complete was not
// acceptance. Former source shim: ?wave_in_device_set_codec_redirect@@YIHPAX0I@Z.
// The game wrapper owner and consumed external ABI are established. The
// provider's unused API, complete layout and nested lifetime remain unknown;
// those dependency obligations do not make this complete wrapper a stub.
// ORIGINAL: 0x004C5A80 ?set_codec@Wave_In_Device@@QAEHK@Z 0x004C5A80-0x004C5AA0 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["soundx.dll SHA-256 ac272e244b3f1339d9ce63bc2e7acf76605c6ac2695bba3230e72ea59039a3f5 exports ordinal 9 create_device and ordinal 10 delete_device for Dll_Wave_In_Device.", "Pinned DLL factory installs a 26-entry table; nested entries 24 and 25 match the observed +0x60 result-forwarder and +0x64 zero-return bodies.", "Pinned game span 0x004C5A80-0x004C5AA0 reads the delegate from inherited device_14_ storage at +0x14, null-returns 0x13, passes one unsigned value to slot 24, and returns that slot result.", "The factory stores its returned object in device_14_; the DLL close/teardown path owns destruction. Only these two observed extension operations are represented by the partial external ABI boundary.", "The former 24 void prefix declarations were placeholders, not recovered DLL signatures; they are removed. SDK/PDB contract, complete class extent, other slot signatures, and nested object lifetime remain unknown."], "reason": "Use only the observed external call ABIs on the opaque DLL-owned provider; do not conflate it with the game wrapper base or invent intervening virtual methods. Its complete API and extent remain unverified."}
// symbol    ?set_codec@Wave_In_Device@@QAEHK@Z
// size      32 bytes
// prototype int (__thiscall ?set_codec@Wave_In_Device@@QAEHK@Z)(Wave_In_Device* this, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C5A90
int Wave_In_Device::set_codec(unsigned long codec) {
    void *const delegate = device_14_;
    if (delegate != 0) {
        return SoundXDeviceAbi::set_codec(delegate, codec);
    }
    return 0x13;
}

// Original behavior: return 0x13 when the DLL-owned delegate is null;
// otherwise call its pinned slot and return that result.
// Original prototype fact: int (__thiscall ?set_vxw_key@Wave_In_Device@@QAEHPAC@Z)(Wave_In_Device* this, int8*).
// Retained seed facts: 32-byte span; zero mapped callers; game;
// frame;hidden;sp_ready;purged_ok; calls none; indirect at
// 0x004C5AB0. Historical seed Status: Complete was not
// acceptance. Former source shim: ?wave_in_device_set_vxw_key_redirect@@YIHPAX0H@Z.
// The game wrapper owner and consumed external ABI are established. The
// provider's unused API, complete layout and nested lifetime remain unknown;
// those dependency obligations do not make this complete wrapper a stub.
// ORIGINAL: 0x004C5AA0 ?set_vxw_key@Wave_In_Device@@QAEHPAC@Z 0x004C5AA0-0x004C5AC0 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["soundx.dll SHA-256 ac272e244b3f1339d9ce63bc2e7acf76605c6ac2695bba3230e72ea59039a3f5 exports ordinal 9 create_device and ordinal 10 delete_device for Dll_Wave_In_Device.", "Pinned DLL factory installs a 26-entry table; nested entries 24 and 25 match the observed +0x60 result-forwarder and +0x64 zero-return bodies.", "Pinned game span 0x004C5AA0-0x004C5AC0 reads the delegate from inherited device_14_ storage at +0x14, null-returns 0x13, passes one signed-byte pointer to slot 25, and returns that slot result.", "The factory stores its returned object in device_14_; the DLL close/teardown path owns destruction. Only these two observed extension operations are represented by the partial external ABI boundary.", "The former 24 void prefix declarations were placeholders, not recovered DLL signatures; they are removed. SDK/PDB contract, complete class extent, other slot signatures, and nested object lifetime remain unknown."], "reason": "Use only the observed external call ABIs on the opaque DLL-owned provider; do not conflate it with the game wrapper base or invent intervening virtual methods. Its complete API and extent remain unverified."}
// symbol    ?set_vxw_key@Wave_In_Device@@QAEHPAC@Z
// size      32 bytes
// prototype int (__thiscall ?set_vxw_key@Wave_In_Device@@QAEHPAC@Z)(Wave_In_Device* this, int8*)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004C5AB0
int Wave_In_Device::set_vxw_key(signed char *key) {
    void *const delegate = device_14_;
    if (delegate != 0) {
        return SoundXDeviceAbi::set_vxw_key(delegate, key);
    }
    return 0x13;
}

/*
Purpose: Ask the wrapped device to start recording, through vtable slot 0x58.
         The original discards its answer.
// ORIGINAL: 0x004C5B60 ?start_record@Wave_In_Device@@QAEHXZ 0x004C5B60-0x004C5B6F BYTE_EXACT
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
    void *device = device_14_;
    if (device) {
        vtable_slot<device_query_fn>(device, 0x58)(device);
    }
    return 0;
}

/*
Purpose: Ask the wrapped device to stop recording, through vtable slot 0x5C.
         The original discards its answer.
// ORIGINAL: 0x004C5B70 ?end_record@Wave_In_Device@@QAEHXZ 0x004C5B70-0x004C5B7F BYTE_EXACT
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
    void *device = device_14_;
    if (device) {
        vtable_slot<device_query_fn>(device, 0x5C)(device);
    }
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
// ORIGINAL: 0x004C5740 ??0Midi_Device@@QAE@XZ 0x004C5740-0x004C577C
// body      src/sounddevice.h
// size      60 bytes
// prototype void (__thiscall ??0Midi_Device@@QAE@XZ)(Midi_Device* this)
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x006465F0
*/

Midi_Device::Midi_Device() {
}

/*
// ORIGINAL: 0x004C5940 ??0Wave_In_Device@@QAE@XZ 0x004C5940-0x004C597C
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
}

/*
// ORIGINAL: 0x004C5780 ??1Midi_Device@@QAE@XZ 0x004C5780-0x004C5793 BYTE_EXACT
// body      src/sounddevice.cpp
// size      19 bytes
// prototype void (__thiscall ??1Midi_Device@@QAE@XZ)(Midi_Device* this)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
// indirect  0x004C578F
*/
// The image re-installs the shared device vtable and then TAIL-JUMPS through
// the wrapped device's own slot 4 - the dispatch is the destructor's last
// act. Promoted from src/recovered/004c5780.cpp.
Midi_Device::~Midi_Device() {
}

/*
// ORIGINAL: 0x004C5980 ??1Wave_In_Device@@QAE@XZ 0x004C5980-0x004C5993 BYTE_EXACT
// body      src/sounddevice.cpp
// size      19 bytes
// prototype void (__thiscall ??1Wave_In_Device@@QAE@XZ)(Wave_In_Device* this)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
// indirect  0x004C598F
*/
// Same shape as ~Midi_Device() above. Promoted from src/recovered/004c5980.cpp.
Wave_In_Device::~Wave_In_Device() {
}

// The unrecovered vftable slots (a bare `ret` stub at 0x00404280 in the
// image's Midi_Device and Wave_In_Device tables). Semantic debt until named.
void Midi_Device::unk_slot2() {}
void Wave_In_Device::unk_slot2() {}

// HOMED from src/recovered/004c56d0.cpp, 004c5710.cpp, units/004c57f0.cpp
// and their Wave_In_Device counterparts: these were the "unrecovered vftable
// slots" the stubs above stood in for.
//
// INT, not void, on release(): the image's own mangling says
// ?release@Midi_Device@@QAEXXZ, but every one of these bodies ends
// `xor eax, eax; pop esi; ret` (0x004C581F, 0x004C5A6D) - it returns a zero
// the callers may read. Left as the image has it; noted rather than fixed.
// ORIGINAL: 0x004C56D0 ?create_device@Midi_Device@@QAEHK@Z 0x004C56D0-0x004C5707 BYTE_EXACT
// LEVER: the artifact's `char *self = reinterpret_cast<char *>(this); ... self +
//   0x14` pun became `void **slot = &device_14_;` - the member form reproduces the
//   image's `lea eax, [ecx + 0x14]` at 0x004C56D3, which is the address the open
//   call needs, not a value read.
// symbol    ?create_device@Midi_Device@@UAEHK@Z
int Midi_Device::create_device(unsigned long flags) {
    // The image takes `&device_14_` into eax first (0x004C56D3 `lea eax,
    // [ecx + 0x14]`), then loads the field for the test - the wrapped device
    // is opened INTO this slot, so the address outlives the null check.
    void **slot = reinterpret_cast<void **>(&device_14_);
    if (device_14_ == 0) {
        MidiDeviceOpen *const open = MidiDeviceOpenSlot();
        if (open != 0) {
            return open(slot, flags);
        }
        return 0x14;
    }
    return 0xc;
}

// ORIGINAL: 0x004C5710 ?delete_device@Midi_Device@@QAEHXZ 0x004C5710-0x004C5738 BYTE_EXACT
// symbol    ?delete_device@Midi_Device@@UAEHXZ
int Midi_Device::delete_device() {
    if (device_14_ != 0 && MidiDeviceCloseSlot() != 0) {
        MidiDeviceCloseSlot()();
        device_14_ = 0;
        return 0;
    }
    return 0x14;
}

// ORIGINAL: 0x004C57F0 ?release@Midi_Device@@QAEXXZ 0x004C57F0-0x004C5823 BYTE_EXACT
// symbol    ?release@Midi_Device@@UAEHXZ
int Midi_Device::release() {
    if (device_14_ != 0) {
        SoundXDeviceAbi::release_device(device_14_);
        if (device_14_ != 0) {
            if (MidiDeviceCloseSlot() != 0) {
                MidiDeviceCloseSlot()();
                device_14_ = 0;
            }
        }
        device_14_ = 0;
    }
    return 0;
}

// ORIGINAL: 0x004C59A0 ?create_device@Wave_In_Device@@QAEHK@Z 0x004C59A0-0x004C59C5 BYTE_EXACT
// symbol    ?create_device@Wave_In_Device@@UAEHK@Z
int Wave_In_Device::create_device(unsigned long flags) {
    void **slot = reinterpret_cast<void **>(&device_14_);
    if (device_14_ == 0) {
        WaveInDeviceOpen *const open = WaveInDeviceOpenSlot();
        if (open != 0) {
            return open(slot, flags);
        }
        return 0x14;
    }
    return 0xc;
}

// ORIGINAL: 0x004C59E0 ?delete_device@Wave_In_Device@@QAEHXZ 0x004C59E0-0x004C5A08 BYTE_EXACT
// symbol    ?delete_device@Wave_In_Device@@UAEHXZ
int Wave_In_Device::delete_device() {
    if (device_14_ != 0 && WaveInDeviceCloseSlot() != 0) {
        WaveInDeviceCloseSlot()();
        device_14_ = 0;
        return 0;
    }
    return 0x14;
}

// ORIGINAL: 0x004C5A50 ?release@Wave_In_Device@@QAEXXZ 0x004C5A50-0x004C5A71 BYTE_EXACT
// symbol    ?release@Wave_In_Device@@UAEHXZ
int Wave_In_Device::release() {
    if (device_14_ != 0) {
        SoundXDeviceAbi::release_device(device_14_);
        delete_device();
        device_14_ = 0;
    }
    return 0;
}
