// ORIGINAL: 0x004C59E0 BYTE_EXACT
// name      ?delete_device@Wave_In_Device@@QAEHXZ
// size      39 bytes
// spans     0x004C59E0-0x004C5A07
// prototype int (__thiscall ?delete_device@Wave_In_Device@@QAEHXZ)(Wave_In_Device* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// 0x004C59E0  ?delete_device@Wave_In_Device@@QAEHXZ  ->  ?delete_device@Wave_In_Device@@QAEHXZ
//
// A byte-exact Mizuchi match that no file in the tree owns yet. NOT in
// OPENSMACX_SOURCES and not compiled: it is the emitter's verification style,
// and rewriting it in the tree's own style is a later phase. See README.md
// beside this file. Re-verified in bulk by byte_match_fanout.py --collect.

/*
Purpose: Release the capture device through the driver's destroy hook.
Original Offset: 004C59E0
Return Value: zero on success, 0x14 when there is nothing to release
Status: Complete
*/
// A MACRO, not a `T **` global. Declaring the slot as an ordinary variable
// bound to the address costs an extra `mov` - it loads the variable's own
// storage and then the value at 0x0090DB48, two indirections where the
// original has one. 41 bytes against 39.
typedef void(__cdecl func_wave_in_device_destroy)(void);
#define WaveInDeviceDestroySlot \
    (*reinterpret_cast<func_wave_in_device_destroy **>(0x0090DB48))

int Wave_In_Device::delete_device() {
    // ONE guarded block with a shared failure path. Two separate early
    // returns emit two `mov eax,0x14` blocks and 48 bytes.
    if (field_14_ && WaveInDeviceDestroySlot) {
        WaveInDeviceDestroySlot();
        field_14_ = 0;
        return 0;
    }
    return 0x14;
}
