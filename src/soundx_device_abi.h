#pragma once

// Partial external SoundX ABI, not a reconstructed provider class.
// Pinned soundx.dll SHA-256:
// ac272e244b3f1339d9ce63bc2e7acf76605c6ac2695bba3230e72ea59039a3f5
// The game and pinned DLL establish operations at byte offsets 0x0C, 0x10,
// 0x54, 0x58, 0x5C, 0x60 and 0x64. Unconsumed slots, provider hierarchy
// and complete extent remain unknown. DLL factories and paired delete exports
// own allocation/destruction.
namespace SoundXDeviceAbi {
namespace Detail {

// Borrowed init prefix, never instantiated or used to own a provider.
// Pinned ordinal 7's factory installs table 0x10064234; ordinal 9's
// factory installs 0x10064590. Both prefixes have zero-status RET 4 / RET
// entries, then a no-argument field clear, then init with RET 8 at 0x0C.
// Reserved names retain only those observed call frames, not SDK semantics.
// This view asserts neither a shared provider base nor game-wrapper layout.
class InitProviderCallView {
public:
    virtual int reserved_00(unsigned long) = 0;
    virtual int reserved_04() = 0;
    virtual void reserved_08() = 0;
    virtual int init(void *, unsigned long) = 0;
};

// Used only to encode the observed argument-bearing member-pointer calls.
// This incomplete type conveys no provider hierarchy or virtual declarations.
class __single_inheritance CallReceiver;

template <class Slot>
Slot &slot_reference(void *object, unsigned long offset) {
    const unsigned char *const vtable =
        *reinterpret_cast<const unsigned char *const *>(object);
    return *reinterpret_cast<Slot *>(const_cast<unsigned char *>(vtable) + offset);
}

typedef int (__fastcall *QueryMethod)(void *);

__forceinline int query(void *device, unsigned long offset) {
    return slot_reference<QueryMethod>(device, offset)(device);
}

}  // namespace Detail

__forceinline int init_device(void *device, void *window,
                              unsigned long backends) {
    return static_cast<Detail::InitProviderCallView *>(device)->init(window, backends);
}

__forceinline void release_device(void *device) {
    // Preserve the existing receiver-first fastcall-shaped no-argument call.
    typedef void (__fastcall *ReleaseMethod)(void *);
    Detail::slot_reference<ReleaseMethod>(device, 0x10)(device);
}

__forceinline int set_codec(void *device, unsigned long codec) {
    typedef int (Detail::CallReceiver::*CodecMethod)(unsigned long);
    return (reinterpret_cast<Detail::CallReceiver *>(device)->*
            Detail::slot_reference<CodecMethod>(device, 0x60))(codec);
}

__forceinline int set_vxw_key(void *device, signed char *key) {
    typedef int (Detail::CallReceiver::*KeyMethod)(signed char *);
    return (reinterpret_cast<Detail::CallReceiver *>(device)->*
            Detail::slot_reference<KeyMethod>(device, 0x64))(key);
}

// Names describe only the game wrapper operations. Provider semantics and
// any signature facts beyond these observed call ABIs remain unverified.
__forceinline void enable_device(void *device) {
    (void)Detail::query(device, 0x54);
}

__forceinline void disable_device(void *device) {
    (void)Detail::query(device, 0x58);
}

__forceinline int is_disabled(void *device) {
    return Detail::query(device, 0x5C);
}

__forceinline void start_recording(void *device) {
    (void)Detail::query(device, 0x58);
}

__forceinline void stop_recording(void *device) {
    (void)Detail::query(device, 0x5C);
}

}  // namespace SoundXDeviceAbi
