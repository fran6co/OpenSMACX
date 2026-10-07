#pragma once

#include "original_seam.h"

// Partial external SoundX ABI, not a reconstructed provider class.
// Pinned soundx.dll ac272e244b3f1339d9ce63bc2e7acf76605c6ac2695bba3230e72ea59039a3f5
// and the game callers establish operations at slots 3, 4, 24 and 25.
// Earlier/later slots, provider hierarchy and complete extent remain unknown.
// The DLL factories and paired delete exports own allocation/destruction.
namespace SoundXDeviceAbi {
__forceinline int init_device(void *device, void *window, unsigned long backends) {
    typedef int (OriginalObject::*InitMethod)(void *, unsigned long);
    return (reinterpret_cast<OriginalObject *>(device)->*
            vtable_method<InitMethod>(device, 0x0C))(window, backends);
}

__forceinline void release_device(void *device) {
    // No arguments besides the thiscall receiver; the game ignores EAX.
    typedef void (__fastcall *ReleaseMethod)(void *);
    vtable_slot<ReleaseMethod>(device, 0x10)(device);
}

__forceinline int set_codec(void *device, unsigned long codec) {
    typedef int (OriginalObject::*CodecMethod)(unsigned long);
    return (reinterpret_cast<OriginalObject *>(device)->*
            vtable_method<CodecMethod>(device, 0x60))(codec);
}

__forceinline int set_vxw_key(void *device, signed char *key) {
    typedef int (OriginalObject::*KeyMethod)(signed char *);
    return (reinterpret_cast<OriginalObject *>(device)->*
            vtable_method<KeyMethod>(device, 0x64))(key);
}
}  // namespace SoundXDeviceAbi
