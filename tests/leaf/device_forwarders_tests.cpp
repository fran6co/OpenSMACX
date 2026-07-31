// Recovery leaf tests: device_forwarders.
//
// Carved verbatim out of tests/recovery_leaf_tests.cpp.  Add a case by
// appending a `void test_*()` here plus `LEAF_CASE(LEAF_APPEND, test_*);` at
// the bottom.  This file is the only file that edit touches: the per-family
// case counts are derived from these `LEAF_CASE(` lines at build time, and
// LEAF_APPEND needs no offset because ties there break by name.
#include "leaf_shared.h"

#define LEAF_FAMILY "device_forwarders"

namespace {

void *g_dev_call_self;

int g_dev_enable_calls;

int g_dev_disable_calls;

void __thiscall observe_dev_enable(void *self) {
    g_dev_call_self = self; ++g_dev_enable_calls;
}

void __thiscall observe_dev_disable(void *self) {
    g_dev_call_self = self; ++g_dev_disable_calls;
}

}  // namespace

void test_wave_device_enable_disable() {
    // enable and disable drive the device wrapped at offset 0x14 through two
    // adjacent vtable slots - 0x60 for enable, 0x64 for disable, called with the
    // device as this - and do nothing when there is no wrapped device.
    std::vector<uint8_t> wd(sizeof(Wave_Device) + 16, 0);
    auto *dev = reinterpret_cast<Wave_Device *>(wd.data());

    void *vtable[32] = {};
    vtable[0x60 / 4] = reinterpret_cast<void *>(&observe_dev_enable);
    vtable[0x64 / 4] = reinterpret_cast<void *>(&observe_dev_disable);
    struct FakeDevice { void *vtbl; } fake_device;
    fake_device.vtbl = vtable;

    auto set_device = [&](void *d) { std::memcpy(wd.data() + 0x14, &d, sizeof(d)); };

    // No device: both are no-ops.
    set_device(nullptr);
    g_dev_enable_calls = g_dev_disable_calls = 0;
    dev->enable();
    dev->disable();
    expect(g_dev_enable_calls == 0 && g_dev_disable_calls == 0);

    // With a device: enable hits slot 0x60 with the device as this.
    set_device(&fake_device);
    g_dev_enable_calls = g_dev_disable_calls = 0;
    g_dev_call_self = nullptr;
    dev->enable();
    expect(g_dev_enable_calls == 1 && g_dev_disable_calls == 0);
    expect(g_dev_call_self == &fake_device);

    // disable hits the adjacent slot 0x64.
    g_dev_call_self = nullptr;
    dev->disable();
    expect(g_dev_disable_calls == 1 && g_dev_enable_calls == 1);
    expect(g_dev_call_self == &fake_device);

    // Redirect entries drive the same two slots.
    g_dev_enable_calls = g_dev_disable_calls = 0;
    wave_device_enable_redirect(dev, nullptr);
    wave_device_disable_redirect(dev, nullptr);
    expect(g_dev_enable_calls == 1 && g_dev_disable_calls == 1);
}

namespace {

void *g_sound_slot0_self;

int g_sound_slot0_arg;

int g_sound_slot0_calls;

int g_sound_slot0_ret;

void *g_sound_fallback_self;

int g_sound_fallback_calls;

// The fallback dispatch re-reads the object's vtable pointer, so slot 0 is
// given a chance to swap it; when it does, the fallback must come from the
// replacement table.
void **g_sound_swap_to;

void *g_sound_slot0_obj;

int __thiscall observe_sound_slot0(Sound *self, int a1) {
    g_sound_slot0_self = self;
    g_sound_slot0_arg = a1;
    ++g_sound_slot0_calls;
    if (g_sound_swap_to) {
        std::memcpy(g_sound_slot0_obj, &g_sound_swap_to, sizeof(g_sound_swap_to));
    }
    return g_sound_slot0_ret;
}

void __thiscall observe_sound_fallback(Sound *self) {
    g_sound_fallback_self = self;
    ++g_sound_fallback_calls;
}

int g_sound_fallback2_calls;

void __thiscall observe_sound_fallback2(Sound *) { ++g_sound_fallback2_calls; }

}  // namespace

void test_sound_fade() {
    // fade asks the object's own vtable slot 0 to fade with the argument, and
    // runs slot 0x28 as a fallback only when slot 0 declines by returning zero.
    // Both dispatches pass the Sound as this, and both read the object's live
    // vtable pointer - the original loads it twice, so a slot-0 that swaps the
    // table must redirect the fallback.
    std::vector<uint8_t> storage(sizeof(Sound) + 32, 0);
    auto *sound = reinterpret_cast<Sound *>(storage.data());

    void *vtable[16] = {};
    vtable[0] = reinterpret_cast<void *>(&observe_sound_slot0);
    vtable[0x28 / 4] = reinterpret_cast<void *>(&observe_sound_fallback);
    void *vtable_ptr = vtable;
    std::memcpy(storage.data(), &vtable_ptr, sizeof(vtable_ptr));
    g_sound_slot0_obj = storage.data();

    // Slot 0 accepts (nonzero): no fallback, and it got the argument and this.
    g_sound_swap_to = nullptr;
    g_sound_slot0_calls = g_sound_fallback_calls = 0;
    g_sound_slot0_ret = 1;
    sound->fade(77);
    expect(g_sound_slot0_calls == 1 && g_sound_fallback_calls == 0);
    expect(g_sound_slot0_self == sound && g_sound_slot0_arg == 77);

    // Slot 0 declines (zero): the fallback at 0x28 runs, with the same this.
    g_sound_slot0_calls = g_sound_fallback_calls = 0;
    g_sound_slot0_ret = 0;
    sound->fade(5);
    expect(g_sound_slot0_calls == 1 && g_sound_fallback_calls == 1);
    expect(g_sound_fallback_self == sound && g_sound_slot0_arg == 5);

    // A negative return is still nonzero, so no fallback.
    g_sound_slot0_calls = g_sound_fallback_calls = 0;
    g_sound_slot0_ret = -1;
    sound->fade(0);
    expect(g_sound_fallback_calls == 0);

    // The fallback comes from the vtable as it stands after slot 0 ran: slot 0
    // swaps the object's table, and the second table's 0x28 is what runs.
    void *vtable2[16] = {};
    vtable2[0x28 / 4] = reinterpret_cast<void *>(&observe_sound_fallback2);
    g_sound_swap_to = vtable2;
    g_sound_slot0_ret = 0;
    g_sound_fallback_calls = g_sound_fallback2_calls = 0;
    sound->fade(1);
    expect(g_sound_fallback2_calls == 1 && g_sound_fallback_calls == 0);
    g_sound_swap_to = nullptr;
    std::memcpy(storage.data(), &vtable_ptr, sizeof(vtable_ptr));

    // Redirect entry forwards the argument and drives the same path.
    g_sound_slot0_calls = g_sound_fallback_calls = 0;
    g_sound_slot0_ret = 0;
    sound_fade_redirect(sound, nullptr, 42);
    expect(g_sound_slot0_calls == 1 && g_sound_slot0_arg == 42);
    expect(g_sound_fallback_calls == 1);
}

namespace {

void *g_wave_dev_self;

int g_wave_dev_calls;

int g_wave_dev_ret;

void *g_wave_self_self;

int g_wave_self_calls;

int __thiscall observe_wave_device_unload(void *self) {
    g_wave_dev_self = self; ++g_wave_dev_calls; return g_wave_dev_ret;
}

void __thiscall observe_wave_self_slot(Wave *self) {
    g_wave_self_self = self; ++g_wave_self_calls;
}

}  // namespace

void test_wave_unload() {
    // unload asks the wrapped device at 0x3C to unload through the device's own
    // vtable slot 0x14, returns that result, forgets the device, runs the
    // object's own vtable slot 0x80 unless bit 1 of the flag byte at 0x54
    // suppresses it, and clears bit 0 of the flag dword at 0x40.
    std::vector<uint8_t> storage(sizeof(Wave) + 32, 0);
    auto *wave = reinterpret_cast<Wave *>(storage.data());
    uint8_t *const obj = storage.data();

    void *wave_vtable[64] = {};
    wave_vtable[0x80 / 4] = reinterpret_cast<void *>(&observe_wave_self_slot);
    void *wave_vtable_ptr = wave_vtable;
    std::memcpy(obj, &wave_vtable_ptr, sizeof(wave_vtable_ptr));

    void *dev_vtable[16] = {};
    dev_vtable[0x14 / 4] = reinterpret_cast<void *>(&observe_wave_device_unload);
    struct FakeDev { void *vtbl; } fake_dev;
    fake_dev.vtbl = dev_vtable;

    auto set_device = [&](void *d) { std::memcpy(obj + 0x3C, &d, sizeof(d)); };
    auto read32 = [&](size_t off) {
        uint32_t v = 0; std::memcpy(&v, obj + off, 4); return v;
    };
    auto set32 = [&](size_t off, uint32_t v) { std::memcpy(obj + off, &v, 4); };

    // A wrapped device: its slot 0x14 runs with the device as this, its result
    // is returned, the device pointer is cleared, the self slot runs, and only
    // bit 0 of the 0x40 dword is cleared.
    set_device(&fake_dev);
    set32(0x40, 0xFFFFFFFFu);
    obj[0x54] = 0;
    g_wave_dev_calls = g_wave_self_calls = 0;
    g_wave_dev_ret = 0x5A;
    expect(wave->unload() == 0x5A);
    expect(g_wave_dev_calls == 1 && g_wave_dev_self == &fake_dev);
    expect(read32(0x3C) == 0);                 // device forgotten
    expect(g_wave_self_calls == 1 && g_wave_self_self == wave);
    expect(read32(0x40) == 0xFFFFFFFEu);       // only bit 0 cleared

    // No device: nothing is dispatched to a device and the result is 0, but the
    // self slot still runs and the bit is still cleared.
    set_device(nullptr);
    set32(0x40, 1);
    g_wave_dev_calls = g_wave_self_calls = 0;
    expect(wave->unload() == 0);
    expect(g_wave_dev_calls == 0);
    expect(g_wave_self_calls == 1);
    expect(read32(0x40) == 0);

    // Bit 1 of the 0x54 flag byte suppresses the self slot; everything else
    // still happens.
    set_device(&fake_dev);
    set32(0x40, 0xF);
    obj[0x54] = 2;
    g_wave_dev_calls = g_wave_self_calls = 0;
    g_wave_dev_ret = 7;
    expect(wave->unload() == 7);
    expect(g_wave_dev_calls == 1);
    expect(g_wave_self_calls == 0);            // suppressed
    expect(read32(0x40) == 0xE);
    expect(read32(0x3C) == 0);

    // Other bits of the 0x54 byte do not suppress it.
    set_device(nullptr);
    obj[0x54] = 0xFD;                          // everything but bit 1
    g_wave_self_calls = 0;
    wave->unload();
    expect(g_wave_self_calls == 1);

    // Redirect entry returns the device result.
    set_device(&fake_dev);
    obj[0x54] = 0;
    g_wave_dev_ret = 0x33;
    expect(wave_unload_redirect(wave, nullptr) == 0x33);
}

namespace {

int g_wave_pitch_arg;

int g_wave_pitch_calls;

void *g_wave_pitch_self;

void __thiscall observe_wave_set_pitch(void *self, int pitch) {
    g_wave_pitch_self = self; g_wave_pitch_arg = pitch; ++g_wave_pitch_calls;
}

int g_wave_load_a1, g_wave_load_a2, g_wave_load_calls;

void *g_wave_load_self;

void __thiscall observe_wave_load_slot(Wave *self, int a1, int a2) {
    g_wave_load_self = self; g_wave_load_a1 = a1; g_wave_load_a2 = a2;
    ++g_wave_load_calls;
}

int g_wave_follow_calls, g_wave_follow_ret;

void *g_wave_follow_self;

int __thiscall observe_wave_follow_slot(Wave *self) {
    g_wave_follow_self = self; ++g_wave_follow_calls; return g_wave_follow_ret;
}

}  // namespace

void test_wave_set_pitch_and_load() {
    // set_pitch clamps to [-1200, 1200], stores the clamped value at 0x58, and
    // forwards it to the wrapped device's vtable slot 0x98 when one is wrapped.
    // load runs the object's own slot 0x88 with both arguments, then - unless
    // bit 2 of the second argument skips it - its own slot 0x8C, whose result
    // it returns.
    std::vector<uint8_t> storage(sizeof(Wave) + 32, 0);
    auto *wave = reinterpret_cast<Wave *>(storage.data());
    uint8_t *const obj = storage.data();

    void *wave_vtable[64] = {};
    wave_vtable[0x88 / 4] = reinterpret_cast<void *>(&observe_wave_load_slot);
    wave_vtable[0x8C / 4] = reinterpret_cast<void *>(&observe_wave_follow_slot);
    void *wave_vtable_ptr = wave_vtable;
    std::memcpy(obj, &wave_vtable_ptr, sizeof(wave_vtable_ptr));

    void *dev_vtable[64] = {};
    dev_vtable[0x98 / 4] = reinterpret_cast<void *>(&observe_wave_set_pitch);
    struct FakeDev2 { void *vtbl; } fake_dev;
    fake_dev.vtbl = dev_vtable;

    auto set_device = [&](void *d) { std::memcpy(obj + 0x3C, &d, sizeof(d)); };
    auto pitch_field = [&]() {
        int32_t v = 0; std::memcpy(&v, obj + 0x58, 4); return v;
    };

    // In range: stored and forwarded verbatim.
    set_device(&fake_dev);
    g_wave_pitch_calls = 0;
    wave->set_pitch(500);
    expect(pitch_field() == 500);
    expect(g_wave_pitch_calls == 1 && g_wave_pitch_arg == 500);
    expect(g_wave_pitch_self == &fake_dev);

    // Above the ceiling clamps to 1200; the clamped value is what is forwarded.
    wave->set_pitch(99999);
    expect(pitch_field() == 1200 && g_wave_pitch_arg == 1200);
    wave->set_pitch(1200);          // the boundary itself is kept
    expect(pitch_field() == 1200);
    wave->set_pitch(1201);
    expect(pitch_field() == 1200);

    // Below the floor clamps to -1200.
    wave->set_pitch(-99999);
    expect(pitch_field() == -1200 && g_wave_pitch_arg == -1200);
    wave->set_pitch(-1200);         // the boundary itself is kept
    expect(pitch_field() == -1200);
    wave->set_pitch(-1201);
    expect(pitch_field() == -1200);

    // No device: still stored, nothing dispatched.
    set_device(nullptr);
    g_wave_pitch_calls = 0;
    wave->set_pitch(7);
    expect(pitch_field() == 7 && g_wave_pitch_calls == 0);

    // Redirect entry.
    set_device(&fake_dev);
    g_wave_pitch_calls = 0;
    wave_set_pitch_redirect(wave, nullptr, -30);
    expect(pitch_field() == -30 && g_wave_pitch_arg == -30);

    // load: slot 0x88 gets both arguments and this; the follow-up runs and its
    // result is returned.
    g_wave_load_calls = g_wave_follow_calls = 0;
    g_wave_follow_ret = 0x2B;
    expect(wave->load(11, 0) == 0x2B);
    expect(g_wave_load_calls == 1 && g_wave_load_a1 == 11 && g_wave_load_a2 == 0);
    expect(g_wave_load_self == wave);
    expect(g_wave_follow_calls == 1 && g_wave_follow_self == wave);

    // Bit 2 of the second argument skips the follow-up and returns 0, but the
    // first slot still runs with the argument intact.
    g_wave_load_calls = g_wave_follow_calls = 0;
    expect(wave->load(12, 4) == 0);
    expect(g_wave_load_calls == 1 && g_wave_load_a2 == 4);
    expect(g_wave_follow_calls == 0);

    // Other bits do not skip it.
    g_wave_follow_calls = 0;
    g_wave_follow_ret = 9;
    expect(wave->load(13, 0xFB) == 9);
    expect(g_wave_follow_calls == 1);

    // Redirect entry forwards both arguments.
    g_wave_load_calls = g_wave_follow_calls = 0;
    g_wave_follow_ret = 3;
    expect(wave_load_redirect(wave, nullptr, 21, 0) == 3);
    expect(g_wave_load_a1 == 21 && g_wave_follow_calls == 1);
}

void test_zeroed_constant_return_stubs() {
    // Eleven stubs whose bodies are nothing but `xor eax, eax; ret` - three
    // bytes each. They were hidden from the constant-return scan by a
    // length check that could never match that shape, so they are all
    // recovered together here: each returns zero and writes nothing.
    // The device layouts are bounded rather than established, so as elsewhere
    // the canary is sized to what is modelled; a method that should touch no
    // field writes nothing regardless of where the object really ends.
    alignas(Midi_Device) uint8_t midi_storage[sizeof(Midi_Device) + 32];
    uint8_t midi_expected[sizeof(midi_storage)];
    auto *midi = reinterpret_cast<Midi_Device *>(midi_storage + 16);
    seed_storage(midi_storage, midi_expected, sizeof(midi_storage));
    std::memcpy(midi_expected, midi_storage, sizeof(midi_storage));
    expect(midi->get_ndevices() == 0);
    expect(midi->get_volume() == 0);
    expect(midi->stop() == 0);
    expect(midi->get_rate() == 0);
    expect(midi_device_get_ndevices_redirect(midi, nullptr) == 0);
    expect(midi_device_get_volume_redirect(midi, nullptr) == 0);
    expect(midi_device_stop_redirect(midi, nullptr) == 0);
    expect(midi_device_get_rate_redirect(midi, nullptr) == 0);
    expect_storage_bytes(midi_storage, midi_expected, sizeof(midi_storage));

    alignas(Wave_In_Device) uint8_t win_storage[sizeof(Wave_In_Device) + 32];
    uint8_t win_expected[sizeof(win_storage)];
    auto *wave_in = reinterpret_cast<Wave_In_Device *>(win_storage + 16);
    seed_storage(win_storage, win_expected, sizeof(win_storage));
    std::memcpy(win_expected, win_storage, sizeof(win_storage));
    expect(wave_in->get_ndevices() == 0);
    expect(wave_in->stop() == 0);
    expect(wave_in->get_rate() == 0);
    expect(wave_in_device_get_ndevices_redirect(wave_in, nullptr) == 0);
    expect(wave_in_device_stop_redirect(wave_in, nullptr) == 0);
    expect(wave_in_device_get_rate_redirect(wave_in, nullptr) == 0);
    expect_storage_bytes(win_storage, win_expected, sizeof(win_storage));

    alignas(Wave_Device) uint8_t wd_storage[sizeof(Wave_Device) + 32];
    uint8_t wd_expected[sizeof(wd_storage)];
    auto *wave_dev = reinterpret_cast<Wave_Device *>(wd_storage + 16);
    seed_storage(wd_storage, wd_expected, sizeof(wd_storage));
    std::memcpy(wd_expected, wd_storage, sizeof(wd_storage));
    expect(wave_dev->get_volume() == 0);
    expect(wave_dev->stop() == 0);
    expect(wave_device_get_volume_redirect(wave_dev, nullptr) == 0);
    expect(wave_device_stop_redirect(wave_dev, nullptr) == 0);
    expect_storage_bytes(wd_storage, wd_expected, sizeof(wd_storage));

    // A static with no object at all, and a MapWin method that ignores its own.
    expect(CheckButton::init_class() == 0);
    expect(check_button_init_class_redirect() == 0);

    alignas(MapWin) uint8_t mw_storage[sizeof(MapWin) + 32];
    uint8_t mw_expected[sizeof(mw_storage)];
    auto *map_win = reinterpret_cast<MapWin *>(mw_storage + 16);
    seed_storage(mw_storage, mw_expected, sizeof(mw_storage));
    std::memcpy(mw_expected, mw_storage, sizeof(mw_storage));
    expect(map_win->UNK2() == 0);
    expect(map_win_unk2_redirect(map_win, nullptr) == 0);
    expect_storage_bytes(mw_storage, mw_expected, sizeof(mw_storage));
}

namespace {

// One recorder for the whole wrapped-device forwarder family: every method
// under test reaches exactly one vtable slot, so recording which slot ran and
// what it was passed is enough to tell them apart.
int g_fwd_slot_ran;

void *g_fwd_self;

int g_fwd_calls;

int g_fwd_ret;

template <int Slot>
int __thiscall observe_fwd_slot(void *self) {
    g_fwd_slot_ran = Slot; g_fwd_self = self; ++g_fwd_calls; return g_fwd_ret;
}

}  // namespace

void test_wrapped_device_forwarders() {
    // Fourteen methods of one shape: forward to the device the object wraps -
    // at 0x14 for the two device classes, 0x3C for Sound - through one slot of
    // that device's vtable, answering zero (or doing nothing, when void) if no
    // device is wrapped. Each is checked for the slot it reaches, that the
    // device is passed as this, and that the device's answer comes back.
    void *vt[64] = {};
    vt[0x14 / 4] = reinterpret_cast<void *>(&observe_fwd_slot<0x14>);
    vt[0x34 / 4] = reinterpret_cast<void *>(&observe_fwd_slot<0x34>);
    vt[0x3C / 4] = reinterpret_cast<void *>(&observe_fwd_slot<0x3C>);
    vt[0x48 / 4] = reinterpret_cast<void *>(&observe_fwd_slot<0x48>);
    vt[0x4C / 4] = reinterpret_cast<void *>(&observe_fwd_slot<0x4C>);
    vt[0x54 / 4] = reinterpret_cast<void *>(&observe_fwd_slot<0x54>);
    vt[0x58 / 4] = reinterpret_cast<void *>(&observe_fwd_slot<0x58>);
    vt[0x5C / 4] = reinterpret_cast<void *>(&observe_fwd_slot<0x5C>);
    vt[0x70 / 4] = reinterpret_cast<void *>(&observe_fwd_slot<0x70>);
    vt[0x74 / 4] = reinterpret_cast<void *>(&observe_fwd_slot<0x74>);
    struct FwdDev { void *vtbl; } fwd_dev;
    fwd_dev.vtbl = vt;

    std::vector<uint8_t> wd(sizeof(Wave_Device) + 32, 0);
    auto *wave_dev = reinterpret_cast<Wave_Device *>(wd.data());
    std::vector<uint8_t> md(sizeof(Midi_Device) + 32, 0);
    auto *midi = reinterpret_cast<Midi_Device *>(md.data());
    std::vector<uint8_t> sd(sizeof(Sound) + 32, 0);
    auto *sound = reinterpret_cast<Sound *>(sd.data());

    void *dev = &fwd_dev;
    auto attach = [&](std::vector<uint8_t> &obj, size_t off, void *d) {
        std::memcpy(obj.data() + off, &d, sizeof(d));
    };

    // With a device wrapped: each method reaches its own slot, passing the
    // device as this, and each querying one returns the device's answer.
    attach(wd, 0x14, dev);
    attach(md, 0x14, dev);
    attach(sd, 0x3C, dev);
    g_fwd_ret = 0x4321;

    struct VoidCase { void (Wave_Device::*fn)(); int slot; };
    const VoidCase wd_void[] = {{&Wave_Device::suspend, 0x48},
                                {&Wave_Device::restart, 0x4C},
                                {&Wave_Device::update_sound, 0x34}};
    for (const auto &c : wd_void) {
        g_fwd_calls = 0; g_fwd_slot_ran = -1; g_fwd_self = nullptr;
        (wave_dev->*c.fn)();
        expect(g_fwd_calls == 1 && g_fwd_slot_ran == c.slot);
        expect(g_fwd_self == dev);
    }

    struct QueryCase { int (Wave_Device::*fn)(); int slot; };
    const QueryCase wd_query[] = {{&Wave_Device::get_ndevices, 0x14},
                                  {&Wave_Device::get_hw_mem_size, 0x58},
                                  {&Wave_Device::get_rate, 0x3C},
                                  {&Wave_Device::get_ds, 0x70},
                                  {&Wave_Device::is_eax, 0x74}};
    for (const auto &c : wd_query) {
        g_fwd_calls = 0; g_fwd_slot_ran = -1;
        expect((wave_dev->*c.fn)() == 0x4321);
        expect(g_fwd_calls == 1 && g_fwd_slot_ran == c.slot);
        expect(g_fwd_self == dev);
    }

    g_fwd_calls = 0; g_fwd_slot_ran = -1;
    midi->enable();
    expect(g_fwd_calls == 1 && g_fwd_slot_ran == 0x54 && g_fwd_self == dev);
    g_fwd_calls = 0; g_fwd_slot_ran = -1;
    midi->disable();
    expect(g_fwd_calls == 1 && g_fwd_slot_ran == 0x58 && g_fwd_self == dev);

    const struct { int (Sound::*fn)(); int slot; } sound_query[] = {
        {&Sound::is_playing, 0x5C}, {&Sound::is_looping, 0x58},
        {&Sound::get_time, 0x74}};
    for (const auto &c : sound_query) {
        g_fwd_calls = 0; g_fwd_slot_ran = -1;
        expect((sound->*c.fn)() == 0x4321);
        expect(g_fwd_calls == 1 && g_fwd_slot_ran == c.slot);
        expect(g_fwd_self == dev);
    }

    // With no device wrapped: nothing is dispatched and every query answers 0.
    attach(wd, 0x14, nullptr);
    attach(md, 0x14, nullptr);
    attach(sd, 0x3C, nullptr);
    g_fwd_calls = 0;
    for (const auto &c : wd_void) { (wave_dev->*c.fn)(); }
    for (const auto &c : wd_query) { expect((wave_dev->*c.fn)() == 0); }
    midi->enable();
    midi->disable();
    for (const auto &c : sound_query) { expect((sound->*c.fn)() == 0); }
    expect(g_fwd_calls == 0);

    // The redirects reach the same slots.
    attach(wd, 0x14, dev);
    attach(md, 0x14, dev);
    attach(sd, 0x3C, dev);
    g_fwd_ret = 9;
    g_fwd_slot_ran = -1;
    wave_device_suspend_redirect(wave_dev, nullptr);
    expect(g_fwd_slot_ran == 0x48);
    wave_device_restart_redirect(wave_dev, nullptr);
    expect(g_fwd_slot_ran == 0x4C);
    wave_device_update_sound_redirect(wave_dev, nullptr);
    expect(g_fwd_slot_ran == 0x34);
    expect(wave_device_get_ndevices_redirect(wave_dev, nullptr) == 9);
    expect(g_fwd_slot_ran == 0x14);
    expect(wave_device_get_hw_mem_size_redirect(wave_dev, nullptr) == 9);
    expect(g_fwd_slot_ran == 0x58);
    expect(wave_device_get_rate_redirect(wave_dev, nullptr) == 9);
    expect(g_fwd_slot_ran == 0x3C);
    expect(wave_device_get_ds_redirect(wave_dev, nullptr) == 9);
    expect(g_fwd_slot_ran == 0x70);
    expect(wave_device_is_eax_redirect(wave_dev, nullptr) == 9);
    expect(g_fwd_slot_ran == 0x74);
    midi_device_enable_redirect(midi, nullptr);
    expect(g_fwd_slot_ran == 0x54);
    midi_device_disable_redirect(midi, nullptr);
    expect(g_fwd_slot_ran == 0x58);
    expect(sound_is_playing_redirect(sound, nullptr) == 9);
    expect(g_fwd_slot_ran == 0x5C);
    expect(sound_is_looping_redirect(sound, nullptr) == 9);
    expect(g_fwd_slot_ran == 0x58);
    expect(sound_get_time_redirect(sound, nullptr) == 9);
    expect(g_fwd_slot_ran == 0x74);

    // Wave::get_ms_length is a plain read of the field at 0x60, not a forward.
    std::vector<uint8_t> wv(sizeof(Wave) + 32, 0);
    auto *wave_obj = reinterpret_cast<Wave *>(wv.data());
    int32_t length = 12345;
    std::memcpy(wv.data() + 0x60, &length, sizeof(length));
    int32_t decoy = -1;
    std::memcpy(wv.data() + 0x5C, &decoy, sizeof(decoy));   // neighbour differs
    std::memcpy(wv.data() + 0x64, &decoy, sizeof(decoy));
    expect(wave_obj->get_ms_length() == 12345);
    expect(wave_get_ms_length_redirect(wave_obj, nullptr) == 12345);
}

namespace {

// The one-argument members of the family need the argument recorded too.
int g_fwd_arg;

template <int Slot>
int __thiscall observe_fwd_arg_slot(void *self, int a1) {
    g_fwd_slot_ran = Slot; g_fwd_self = self; g_fwd_arg = a1;
    ++g_fwd_calls; return g_fwd_ret;
}

}  // namespace

void test_wrapped_device_forwarders_with_defaults() {
    // The rest of the family: the same wrapped-device dispatch, but with a
    // no-device answer that is not zero, an argument passed through, or a field
    // written on the way. Each method's slot, its argument, and its no-device
    // answer are all checked, because those are the three things that separate
    // these from one another.
    // Two vtables, because the wave-device side and the Sound side disagree
    // about the arity of the methods at slots 0x20 and 0x38 - one takes an
    // argument, the other does not. In the game these are different device
    // types, and calling a one-argument __thiscall with none would unbalance
    // the stack, so the test keeps them apart the same way.
    void *vt[64] = {};
    vt[0x20 / 4] = reinterpret_cast<void *>(&observe_fwd_arg_slot<0x20>);
    vt[0x38 / 4] = reinterpret_cast<void *>(&observe_fwd_arg_slot<0x38>);
    vt[0x54 / 4] = reinterpret_cast<void *>(&observe_fwd_slot<0x54>);
    vt[0x58 / 4] = reinterpret_cast<void *>(&observe_fwd_slot<0x58>);
    vt[0x5C / 4] = reinterpret_cast<void *>(&observe_fwd_slot<0x5C>);
    vt[0x68 / 4] = reinterpret_cast<void *>(&observe_fwd_slot<0x68>);
    vt[0x6C / 4] = reinterpret_cast<void *>(&observe_fwd_arg_slot<0x6C>);
    vt[0x84 / 4] = reinterpret_cast<void *>(&observe_fwd_slot<0x84>);
    struct FwdDev2 { void *vtbl; } dev_obj;
    dev_obj.vtbl = vt;
    void *dev = &dev_obj;

    void *svt[64] = {};
    svt[0x18 / 4] = reinterpret_cast<void *>(&observe_fwd_arg_slot<0x18>);
    svt[0x1C / 4] = reinterpret_cast<void *>(&observe_fwd_slot<0x1C>);
    svt[0x20 / 4] = reinterpret_cast<void *>(&observe_fwd_slot<0x20>);
    svt[0x38 / 4] = reinterpret_cast<void *>(&observe_fwd_slot<0x38>);
    svt[0x48 / 4] = reinterpret_cast<void *>(&observe_fwd_arg_slot<0x48>);
    svt[0x4C / 4] = reinterpret_cast<void *>(&observe_fwd_arg_slot<0x4C>);
    struct SoundDev { void *vtbl; } sound_dev_obj;
    sound_dev_obj.vtbl = svt;
    void *sound_dev = &sound_dev_obj;

    std::vector<uint8_t> wd(sizeof(Wave_Device) + 32, 0);
    auto *wave_dev = reinterpret_cast<Wave_Device *>(wd.data());
    std::vector<uint8_t> md(sizeof(Midi_Device) + 32, 0);
    auto *midi = reinterpret_cast<Midi_Device *>(md.data());
    std::vector<uint8_t> wid(sizeof(Wave_In_Device) + 32, 0);
    auto *wave_in = reinterpret_cast<Wave_In_Device *>(wid.data());
    std::vector<uint8_t> sd(sizeof(Sound) + 32, 0);
    auto *sound = reinterpret_cast<Sound *>(sd.data());
    auto attach = [&](std::vector<uint8_t> &o, size_t off, void *d) {
        std::memcpy(o.data() + off, &d, sizeof(d));
    };
    auto read32 = [&](std::vector<uint8_t> &o, size_t off) {
        int32_t v = 0; std::memcpy(&v, o.data() + off, 4); return v;
    };

    // With a device: every method reaches its own slot and returns the answer.
    attach(wd, 0x14, dev); attach(md, 0x14, dev);
    attach(wid, 0x14, dev); attach(sd, 0x3C, sound_dev);
    g_fwd_ret = 0x777;

    g_fwd_slot_ran = -1; expect(wave_dev->is_disabled() == 0x777);
    expect(g_fwd_slot_ran == 0x68);
    g_fwd_slot_ran = -1; expect(wave_dev->stop_raw_dump() == 0x777);
    expect(g_fwd_slot_ran == 0x54);
    g_fwd_slot_ran = -1; expect(wave_dev->is_3d() == 0x777);
    expect(g_fwd_slot_ran == 0x84);
    g_fwd_slot_ran = -1; expect(midi->is_disabled() == 0x777);
    expect(g_fwd_slot_ran == 0x5C);

    // The two recorders dispatch but always answer 0 - the original throws the
    // device's answer away.
    g_fwd_slot_ran = -1; expect(wave_in->start_record() == 0);
    expect(g_fwd_slot_ran == 0x58);
    g_fwd_slot_ran = -1; expect(wave_in->end_record() == 0);
    expect(g_fwd_slot_ran == 0x5C);

    // One-argument forwards pass the argument through.
    g_fwd_slot_ran = -1; g_fwd_arg = 0;
    wave_dev->set_rate(4410u);
    expect(g_fwd_slot_ran == 0x38 && g_fwd_arg == 4410);
    g_fwd_slot_ran = -1; wave_dev->set_volume(77u);
    expect(g_fwd_slot_ran == 0x20 && g_fwd_arg == 77);
    int hwnd_marker = 0;
    g_fwd_slot_ran = -1; expect(wave_dev->set_hwnd(&hwnd_marker) == 0x777);
    expect(g_fwd_slot_ran == 0x6C);
    expect(g_fwd_arg == static_cast<int>(reinterpret_cast<intptr_t>(&hwnd_marker)));

    g_fwd_slot_ran = -1; expect(sound->play() == 0x777);
    expect(g_fwd_slot_ran == 0x1C);
    g_fwd_slot_ran = -1; expect(sound->play(9u) == 0x777);
    expect(g_fwd_slot_ran == 0x18 && g_fwd_arg == 9);
    g_fwd_slot_ran = -1; expect(sound->stop() == 0x777);
    expect(g_fwd_slot_ran == 0x20);
    g_fwd_slot_ran = -1; expect(sound->release() == 0x777);
    expect(g_fwd_slot_ran == 0x38);

    // The two setters write their field and forward the same value.
    g_fwd_slot_ran = -1; sound->set_loop_state(0x31337);
    expect(g_fwd_slot_ran == 0x48 && g_fwd_arg == 0x31337);
    expect(read32(sd, 0x30) == 0x31337);
    g_fwd_slot_ran = -1; sound->set_delay(0x4242u);
    expect(g_fwd_slot_ran == 0x4C && g_fwd_arg == 0x4242);
    expect(read32(sd, 0x34) == 0x4242);

    // With no device: nothing dispatches, and each answers its own default -
    // 1 for the disabled queries, 3 for the raw-dump stop, 0x13 for set_hwnd,
    // 0x14 for the four Sound calls, 0 for the rest. The setters still write.
    attach(wd, 0x14, nullptr); attach(md, 0x14, nullptr);
    attach(wid, 0x14, nullptr); attach(sd, 0x3C, nullptr);
    g_fwd_calls = 0;
    expect(wave_dev->is_disabled() == 1);
    expect(midi->is_disabled() == 1);
    expect(wave_dev->stop_raw_dump() == 3);
    expect(wave_dev->is_3d() == 0);
    expect(wave_dev->set_hwnd(&hwnd_marker) == 0x13);
    expect(wave_in->start_record() == 0 && wave_in->end_record() == 0);
    expect(sound->play() == 0x14 && sound->play(1u) == 0x14);
    expect(sound->stop() == 0x14 && sound->release() == 0x14);
    wave_dev->set_rate(1u);
    wave_dev->set_volume(1u);
    sound->set_loop_state(55);
    sound->set_delay(66u);
    expect(read32(sd, 0x30) == 55 && read32(sd, 0x34) == 66);
    expect(g_fwd_calls == 0);

    // get_group_volume reads the object's own table - sixteen 24-byte records
    // from 0x28 - and answers zero past the end. No device is involved.
    for (unsigned i = 0; i < 16; ++i) {
        int32_t v = static_cast<int32_t>(0x100 + i);
        std::memcpy(wd.data() + 0x28 + i * 24, &v, sizeof(v));
    }
    for (unsigned i = 0; i < 16; ++i) {
        expect(wave_dev->get_group_volume(i) == static_cast<int>(0x100 + i));
    }
    // A sentinel sits exactly where a bound that was one too generous would
    // read, so an off-by-one cannot pass by finding zeroes there.
    int32_t past_end = 0x5EEDBEEF;
    std::memcpy(wd.data() + 0x28 + 16 * 24, &past_end, sizeof(past_end));
    expect(wave_dev->get_group_volume(16) == 0);
    expect(wave_dev->get_group_volume(0xFFFFFFFFu) == 0);

    // Redirects reach the same slots and carry the same answers.
    attach(wd, 0x14, dev); attach(md, 0x14, dev);
    attach(wid, 0x14, dev); attach(sd, 0x3C, sound_dev);
    g_fwd_ret = 5;
    g_fwd_slot_ran = -1;
    expect(wave_device_is_disabled_redirect(wave_dev, nullptr) == 5);
    expect(g_fwd_slot_ran == 0x68);
    expect(wave_device_stop_raw_dump_redirect(wave_dev, nullptr) == 5);
    expect(wave_device_is_3d_redirect(wave_dev, nullptr) == 5);
    expect(midi_device_is_disabled_redirect(midi, nullptr) == 5);
    expect(wave_in_device_start_record_redirect(wave_in, nullptr) == 0);
    expect(wave_in_device_end_record_redirect(wave_in, nullptr) == 0);
    g_fwd_arg = 0;
    wave_device_set_rate_redirect(wave_dev, nullptr, 22050u);
    expect(g_fwd_arg == 22050);
    wave_device_set_volume_redirect(wave_dev, nullptr, 12u);
    expect(g_fwd_arg == 12);
    expect(wave_device_set_hwnd_redirect(wave_dev, nullptr, &hwnd_marker) == 5);
    expect(wave_device_get_group_volume_redirect(wave_dev, nullptr, 3) == 0x103);
    expect(sound_play_redirect(sound, nullptr) == 5);
    expect(sound_play_arg_redirect(sound, nullptr, 4u) == 5);
    expect(g_fwd_arg == 4);
    expect(sound_stop_redirect(sound, nullptr) == 5);
    expect(sound_release_redirect(sound, nullptr) == 5);
    sound_set_loop_state_redirect(sound, nullptr, 71);
    expect(read32(sd, 0x30) == 71 && g_fwd_arg == 71);
    sound_set_delay_redirect(sound, nullptr, 72u);
    expect(read32(sd, 0x34) == 72 && g_fwd_arg == 72);
}

namespace {

int g_ramp_a[3], g_ramp_calls;

void __thiscall observe_ramp(void *self, int a1, int a2, int a3) {
    g_fwd_self = self; g_ramp_a[0]=a1; g_ramp_a[1]=a2; g_ramp_a[2]=a3;
    ++g_ramp_calls;
}

}  // namespace

void test_sound_guarded_forwarders() {
    // fade and fade_in carry two guards, not one: the field at 0x38 must be set
    // AND a device wrapped, and either being absent answers 0x13. ramp is a
    // plain three-argument forward. Each is checked for its own slot, both
    // guards independently, and ramp's argument order.
    void *svt[64] = {};
    svt[0x28 / 4] = reinterpret_cast<void *>(&observe_fwd_slot<0x28>);
    svt[0x30 / 4] = reinterpret_cast<void *>(&observe_fwd_slot<0x30>);
    svt[0x34 / 4] = reinterpret_cast<void *>(&observe_ramp);
    struct GDev { void *vtbl; } gdev;
    gdev.vtbl = svt;

    std::vector<uint8_t> sd(sizeof(Sound) + 32, 0);
    auto *sound = reinterpret_cast<Sound *>(sd.data());
    auto set32 = [&](size_t off, int32_t v) { std::memcpy(sd.data()+off,&v,4); };
    void *dev = &gdev;
    std::memcpy(sd.data() + 0x3C, &dev, sizeof(dev));

    // Both guards satisfied: each reaches its own slot and returns the answer.
    set32(0x38, 1);
    g_fwd_ret = 0x2468;
    g_fwd_slot_ran = -1; expect(sound->fade() == 0x2468);
    expect(g_fwd_slot_ran == 0x28 && g_fwd_self == dev);
    g_fwd_slot_ran = -1; expect(sound->fade_in() == 0x2468);
    expect(g_fwd_slot_ran == 0x30 && g_fwd_self == dev);

    // Gate field clear, device present: refused, nothing dispatched.
    set32(0x38, 0);
    g_fwd_calls = 0; g_fwd_slot_ran = -1;
    expect(sound->fade() == 0x13);
    expect(sound->fade_in() == 0x13);
    expect(g_fwd_calls == 0);

    // Gate field set, device absent: also refused. Each guard alone suffices.
    set32(0x38, 1);
    void *none = nullptr;
    std::memcpy(sd.data() + 0x3C, &none, sizeof(none));
    g_fwd_calls = 0;
    expect(sound->fade() == 0x13);
    expect(sound->fade_in() == 0x13);
    expect(g_fwd_calls == 0);

    // ramp forwards all three arguments in order; no gate field involved.
    std::memcpy(sd.data() + 0x3C, &dev, sizeof(dev));
    set32(0x38, 0);                      // deliberately clear - ramp ignores it
    g_ramp_calls = 0;
    sound->ramp(11, 22, 33u);
    expect(g_ramp_calls == 1 && g_fwd_self == dev);
    expect(g_ramp_a[0] == 11 && g_ramp_a[1] == 22 && g_ramp_a[2] == 33);

    // No device: ramp does nothing.
    std::memcpy(sd.data() + 0x3C, &none, sizeof(none));
    g_ramp_calls = 0;
    sound->ramp(1, 2, 3u);
    expect(g_ramp_calls == 0);

    // Redirects.
    std::memcpy(sd.data() + 0x3C, &dev, sizeof(dev));
    set32(0x38, 1);
    g_fwd_ret = 6;
    g_fwd_slot_ran = -1;
    expect(sound_fade_query_redirect(sound, nullptr) == 6);
    expect(g_fwd_slot_ran == 0x28);
    expect(sound_fade_in_redirect(sound, nullptr) == 6);
    expect(g_fwd_slot_ran == 0x30);
    g_ramp_calls = 0;
    sound_ramp_redirect(sound, nullptr, 7, 8, 9u);
    expect(g_ramp_calls == 1 && g_ramp_a[0] == 7 && g_ramp_a[2] == 9);
}

namespace {

void *g_pp_close_self;

int g_pp_close_calls;

void __thiscall observe_pp_win_close(void *self) {
    g_pp_close_self = self; ++g_pp_close_calls;
}

}  // namespace

void test_console_editor_undo_and_prod_picker_close() {
    // editor_undo's whole body is load_undo(1) - the argument is the only thing
    // it can get wrong, and it must not touch the Console.
    alignas(Console) std::vector<uint8_t> cs(sizeof(Console) + 32);
    std::vector<uint8_t> cexp(cs.size());
    auto *console = reinterpret_cast<Console *>(cs.data() + 16);
    seed_storage(cs.data(), cexp.data(), cs.size());
    std::memcpy(cexp.data(), cs.data(), cs.size());
    g_load_undo_calls = 0; g_load_undo_arg = -1;
    console->editor_undo();
    expect(g_load_undo_calls == 1 && g_load_undo_arg == 1);
    console_editor_undo_redirect(console, nullptr);
    expect(g_load_undo_calls == 2 && g_load_undo_arg == 1);
    expect_storage_bytes(cs.data(), cexp.data(), cs.size());

    // ProdPicker::close clears the dword at 0xA14 and then runs the GraphicWin
    // close it inherits - which is source-owned, so it is driven for real and
    // observed at its one seam.
    auto *const saved_win = WinOriginalClose;
    WinOriginalClose = &observe_pp_win_close;
    auto *const saved_bufclose = BufferSubobjectClose;
    BufferSubobjectClose = nullptr;   // GraphicWin::close skips it when unset
    std::vector<uint8_t> pp(sizeof(ProdPicker) + 64, 0);
    auto *picker = reinterpret_cast<ProdPicker *>(pp.data());
    int32_t live = 0x1234;
    std::memcpy(pp.data() + 0xA14, &live, sizeof(live));
    // 0xA10 is not a valid witness - GraphicWin::close clears it by design.
    // 0xA18 sits past ProdPicker's own field and nothing should touch it.
    int32_t neighbour = 0x7777;
    std::memcpy(pp.data() + 0xA18, &neighbour, sizeof(neighbour));
    g_pp_close_calls = 0;
    picker->close();
    int32_t after = -1;
    std::memcpy(&after, pp.data() + 0xA14, sizeof(after));
    expect(after == 0);                       // the field is cleared
    int32_t after_n = 0;
    std::memcpy(&after_n, pp.data() + 0xA18, sizeof(after_n));
    expect(after_n == 0x7777);                // the next dword is untouched
    expect(g_pp_close_calls == 1);            // the inherited close ran
    expect(g_pp_close_self == reinterpret_cast<void *>(picker));

    std::memcpy(pp.data() + 0xA14, &live, sizeof(live));
    g_pp_close_calls = 0;
    prod_picker_close_redirect(picker, nullptr);
    std::memcpy(&after, pp.data() + 0xA14, sizeof(after));
    expect(after == 0 && g_pp_close_calls == 1);

    WinOriginalClose = saved_win;
    BufferSubobjectClose = saved_bufclose;
}

namespace {

void *g_pw_freed;

int g_pw_free_calls;

void *observe_pw_map_win_free(void *p) { g_pw_freed = p; ++g_pw_free_calls; return nullptr; }

void *g_pw_win_close_self;

int g_pw_win_close_calls;

void __thiscall observe_pw_win_close(void *self) {
    g_pw_win_close_self = self; ++g_pw_win_close_calls;
}

}  // namespace

void test_plan_win_close() {
    // PlanWin::close clears one field and then runs MapWin::close on the very
    // same pointer - a real base-class call, since PlanWin's constructor
    // builds a MapWin at offset 0. Both halves are source-owned, so they are
    // driven for real and watched at the two seams underneath.
    auto *const saved_free = MapWinFree;
    auto *const saved_win = WinOriginalClose;
    auto *const saved_bufclose = BufferSubobjectClose;
    MapWinFree = &observe_pw_map_win_free;
    WinOriginalClose = &observe_pw_win_close;
    BufferSubobjectClose = nullptr;   // GraphicWin::close skips it when unset

    std::vector<uint8_t> pw(sizeof(PlanWin) + 64, 0);
    uint8_t *const obj = pw.data();
    auto set32 = [&](size_t off, uint32_t v) { std::memcpy(obj + off, &v, 4); };
    auto get32 = [&](size_t off) {
        uint32_t v = 0; std::memcpy(&v, obj + off, 4); return v;
    };

    // MapWin::close reaches the virtual base through the vbtable, never
    // through its own member, so the object must carry *PlanWin's* table -
    // {0, 0x22050} - and the base it closes must land at 0x22050, not at the
    // 0x21A6C a standalone MapWin would use. That difference is the whole
    // reason this call is safe to make across the two layouts.
    const int32_t vbtable[2] = {0, 0x22050};
    set32(0, reinterpret_cast<uintptr_t>(&vbtable[0]));

    int owned_block = 0;
    const uint32_t owned = reinterpret_cast<uintptr_t>(&owned_block);
    set32(4, owned);                  // MapWin's owned pointer
    set32(0x21A68, 0x1234);           // the field close clears
    set32(0x21A64, 0x11111111);       // last dword of the inherited MapWin data
    set32(0x21A6C, 0x22222222);       // PlanWin's next field - blink's toggle
    set32(sizeof(PlanWin), 0x33333333);   // past the object entirely

    g_pw_free_calls = 0; g_pw_win_close_calls = 0;
    auto *plan = reinterpret_cast<PlanWin *>(obj);
    plan->close();

    expect(get32(0x21A68) == 0);            // the field is cleared
    expect(get32(0x21A64) == 0x11111111);   // neither neighbour moves
    expect(get32(0x21A6C) == 0x22222222);
    expect(get32(sizeof(PlanWin)) == 0x33333333);
    // The inherited MapWin close ran: it released the owned block and dropped
    // the pointer.
    expect(g_pw_free_calls == 1);
    expect(g_pw_freed == reinterpret_cast<void *>(&owned_block));
    expect(get32(4) == 0);
    // ...and reached the GraphicWin base through the vbtable offset, not
    // through MapWin's own.
    expect(g_pw_win_close_calls == 1);
    expect(g_pw_win_close_self == reinterpret_cast<void *>(obj + 0x22050));
    expect(get32(0x22050 + 0xA0C) == *GraphicWinFieldA0CDefault);

    // The redirect drives the identical path.
    set32(4, owned);
    set32(0x21A68, 0x1234);
    g_pw_free_calls = 0; g_pw_win_close_calls = 0;
    plan_win_close_redirect(plan, nullptr);
    expect(get32(0x21A68) == 0);
    expect(get32(4) == 0);
    expect(g_pw_free_calls == 1 && g_pw_win_close_calls == 1);
    expect(g_pw_win_close_self == reinterpret_cast<void *>(obj + 0x22050));

    // A closed-out window frees nothing the second time round.
    g_pw_free_calls = 0;
    plan->close();
    expect(g_pw_free_calls == 0);

    MapWinFree = saved_free;
    WinOriginalClose = saved_win;
    BufferSubobjectClose = saved_bufclose;
}

// Self-registration.  Each case carries its baseline position in the original
// main() call list, so the run order does not depend on link or static-init
// order.
namespace {
LEAF_CASE(166, test_wave_device_enable_disable);
LEAF_CASE(167, test_sound_fade);
LEAF_CASE(168, test_wave_unload);
LEAF_CASE(169, test_wave_set_pitch_and_load);
LEAF_CASE(170, test_zeroed_constant_return_stubs);
LEAF_CASE(171, test_wrapped_device_forwarders);
LEAF_CASE(172, test_wrapped_device_forwarders_with_defaults);
LEAF_CASE(173, test_sound_guarded_forwarders);
LEAF_CASE(174, test_console_editor_undo_and_prod_picker_close);
LEAF_CASE(175, test_plan_win_close);
}  // namespace
