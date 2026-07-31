// Recovery leaf tests: wave_and_sound.
//
// Carved verbatim out of tests/recovery_leaf_tests.cpp.  Add a case by
// appending a `void test_*()` here plus one LEAF_CASE line at the bottom, and
// bumping this family's count in tests/leaf/leaf_case_manifest.h.
#include "leaf_shared.h"

#define LEAF_FAMILY "wave_and_sound"

namespace {

BasePop *g_inited_base_pop = nullptr;

int g_init_a = -1;

long g_init_b = -1;

int g_base_pop_init_result = 0;

int __thiscall observe_base_pop_init(BasePop *self, int a, long b) {
    g_inited_base_pop = self;
    g_init_a = a;
    g_init_b = b;
    return g_base_pop_init_result;
}

InfoWin *g_timer_self = nullptr;

int g_timer_arg = 0;

int g_timer_calls = 0;

int g_timer_flag_during_call = -1;

void __thiscall observe_timer_proc(InfoWin *self, int arg) {
    g_timer_self = self;
    g_timer_arg = arg;
    std::memcpy(&g_timer_flag_during_call,
                reinterpret_cast<const uint8_t *>(self) + 0xA8, sizeof(int));
    ++g_timer_calls;
}

int g_wave_play_dev_calls;

void *g_wave_play_dev_self;

int g_wave_play_dev_ret;

int __thiscall observe_wave_device_is_playing(void *self) {
    g_wave_play_dev_self = self;
    ++g_wave_play_dev_calls;
    return g_wave_play_dev_ret;
}

int g_wave_time_calls;

DWORD g_wave_time_value;

DWORD __stdcall observe_wave_time_get_time(void) {
    ++g_wave_time_calls;
    return g_wave_time_value;
}

// Wave destructor doubles. The callbacks reach back into the fixture through
// g_wave_dtor_obj so they can witness which vtable was installed at call time
// and, for the release hook, re-link the wave to make the inlined base
// destructor's normally-dead unlink reachable.
uint8_t *g_wave_dtor_obj;

void *g_wave_dtor_pull_dev;

Wave *g_wave_dtor_pull_wave;

int g_wave_dtor_pull_calls;

uint32_t g_wave_dtor_pull_seen_vtable;

int __thiscall observe_wave_pull_from_group(void *device, Wave *wave) {
    g_wave_dtor_pull_dev = device;
    g_wave_dtor_pull_wave = wave;
    ++g_wave_dtor_pull_calls;
    std::memcpy(&g_wave_dtor_pull_seen_vtable, g_wave_dtor_obj, 4);
    return 3;
}

std::vector<void *> g_wave_dtor_deletes;

std::vector<void *> g_wave_dtor_delete_seen_slot;

uint32_t *g_wave_delete_watch;

uint32_t g_wave_delete_watch_seen;

bool g_wave_delete_poisons;

void __cdecl observe_wave_operator_delete(void *block) {
    g_wave_dtor_deletes.push_back(block);
    if (g_wave_delete_poisons) {
        // Model the block dying at the free: reads after it see zeros.
        std::memset(block, 0, 12);
    }
    // The original clears the buffer slot only AFTER the free, so at call
    // time the slot still names the block being freed - witness it.
    void *slot;
    std::memcpy(&slot, g_wave_dtor_obj + 0x4C, 4);
    g_wave_dtor_delete_seen_slot.push_back(slot);
    if (g_wave_delete_watch) {
        g_wave_delete_watch_seen = *g_wave_delete_watch;
    }
}

void *g_wave_playm_dev_self;

int g_wave_playm_calls;

int g_wave_playm_arg;

int g_wave_playm_ret;

int __thiscall observe_wave_device_play(void *self, int arg) {
    g_wave_playm_dev_self = self;
    g_wave_playm_arg = arg;
    ++g_wave_playm_calls;
    return g_wave_playm_ret;
}

// Shared observers for the wrapped-device forwarder family: one per arity,
// armed one vtable slot at a time so an off-slot dispatch faults on null.
void *g_wave_fam_self;

uint32_t g_wave_fam_args[3];

int g_wave_fam_calls;

int g_wave_fam_ret;

int __thiscall observe_wave_dev0(void *self) {
    g_wave_fam_self = self;
    ++g_wave_fam_calls;
    return g_wave_fam_ret;
}

std::vector<uint32_t> g_wave_fam_arg_log;

int __thiscall observe_wave_dev1(void *self, uint32_t a1) {
    g_wave_fam_self = self;
    g_wave_fam_args[0] = a1;
    g_wave_fam_arg_log.push_back(a1);
    ++g_wave_fam_calls;
    return g_wave_fam_ret;
}

int __thiscall observe_wave_dev2(void *self, uint32_t a1, uint32_t a2) {
    g_wave_fam_self = self;
    g_wave_fam_args[0] = a1;
    g_wave_fam_args[1] = a2;
    ++g_wave_fam_calls;
    return g_wave_fam_ret;
}

int __thiscall observe_wave_dev3(void *self, uint32_t a1, uint32_t a2,
                                 uint32_t a3) {
    g_wave_fam_self = self;
    g_wave_fam_args[0] = a1;
    g_wave_fam_args[1] = a2;
    g_wave_fam_args[2] = a3;
    ++g_wave_fam_calls;
    return g_wave_fam_ret;
}

// Doubles for set_volume/set_fname/play: the game-heap allocator, the
// device singleton's group-disabled query, the original no-argument load
// (which may wrap a device as its observable effect), and the wave's own
// vtable slots 0x40/0x80.
unsigned g_wave_new_size;

int g_wave_new_calls;

char g_wave_new_arena[64];

bool g_wave_new_fails;

void *__cdecl observe_wave_operator_new(unsigned int size) {
    g_wave_new_size = size;
    ++g_wave_new_calls;
    return g_wave_new_fails ? nullptr : g_wave_new_arena;
}

void *g_wave_gd_dev;

uint32_t g_wave_gd_slot;

int g_wave_gd_calls;

int g_wave_gd_ret;

int __thiscall observe_wave_group_disabled(void *device, uint32_t slot) {
    g_wave_gd_dev = device;
    g_wave_gd_slot = slot;
    ++g_wave_gd_calls;
    return g_wave_gd_ret;
}

int g_wave_oload_calls;

void *g_wave_oload_installs;  // written into the wave's device slot when set

int __thiscall observe_wave_original_load(Wave *wave) {
    ++g_wave_oload_calls;
    std::memcpy(reinterpret_cast<uint8_t *>(wave) + 0x3C,
                &g_wave_oload_installs, 4);
    return 7;
}

Wave *g_wave_own40_self;

uint32_t g_wave_own40_arg;

int g_wave_own40_calls;

void __thiscall observe_wave_own_slot40(Wave *self, uint32_t arg) {
    g_wave_own40_self = self;
    g_wave_own40_arg = arg;
    ++g_wave_own40_calls;
}

Wave *g_wave_own80_self;

int g_wave_own80_calls;

void *g_wave_own80_seen_device;

uint32_t g_wave_own80_seen_stamp;

void __thiscall observe_wave_own_slot80(Wave *self) {
    g_wave_own80_self = self;
    ++g_wave_own80_calls;
    // The original stamps the start time and still holds the device when its
    // own slot 0x80 runs; witness both so the ordering is pinned.
    std::memcpy(&g_wave_own80_seen_device,
                reinterpret_cast<uint8_t *>(self) + 0x3C, 4);
    std::memcpy(&g_wave_own80_seen_stamp,
                reinterpret_cast<uint8_t *>(self) + 0x64, 4);
}

// load() doubles: the device-creation hook, the base Sound::load, and the
// wave's own vtable slot 0x58.
void **g_wave_create_slot_arg;

const char *g_wave_create_name;

int g_wave_create_mode;

int g_wave_create_calls;

int g_wave_create_ret;

void *g_wave_create_installs;

int __cdecl observe_wave_device_create(void **slot, const char *name,
                                       int mode) {
    g_wave_create_slot_arg = slot;
    g_wave_create_name = name;
    g_wave_create_mode = mode;
    ++g_wave_create_calls;
    if (g_wave_create_installs) {
        *slot = g_wave_create_installs;
    }
    return g_wave_create_ret;
}

Wave *g_wave_sload_wave;

const char *g_wave_sload_name;

int g_wave_sload_calls;

int g_wave_sload_ret;

int __thiscall observe_wave_sound_load(Wave *wave, const char *name) {
    g_wave_sload_wave = wave;
    g_wave_sload_name = name;
    ++g_wave_sload_calls;
    return g_wave_sload_ret;
}

int g_wave_own58_calls;

int g_wave_own58_ret;

int __thiscall observe_wave_own_slot58(Wave *) {
    ++g_wave_own58_calls;
    return g_wave_own58_ret;
}

int g_wave_own70_calls;

int g_wave_own70_ret;

void *g_wave_own70_swaps_vtbl;

int __thiscall observe_wave_own_slot70(Wave *self) {
    ++g_wave_own70_calls;
    if (g_wave_own70_swaps_vtbl) {
        // dyna_load captures the device vtable BEFORE this query runs; a
        // swapped table here proves the capture happened first.
        void *device;
        std::memcpy(&device, reinterpret_cast<uint8_t *>(self) + 0x3C, 4);
        std::memcpy(device, &g_wave_own70_swaps_vtbl, 4);
    }
    return g_wave_own70_ret;
}

Wave *g_wave_own48_self;

int g_wave_own48_arg;

int g_wave_own48_calls;

uint32_t g_wave_own48_seen_flags;

void __thiscall observe_wave_own_slot48(Wave *self, int arg) {
    g_wave_own48_self = self;
    g_wave_own48_arg = arg;
    ++g_wave_own48_calls;
    std::memcpy(&g_wave_own48_seen_flags,
                reinterpret_cast<uint8_t *>(self) + 0x54, 4);
}

void *g_wave_ginsert_head;

Wave *g_wave_ginsert_wave;

int g_wave_ginsert_calls;

uint32_t g_wave_ginsert_seen_slot;

void __thiscall observe_wave_group_insert(void *group_head, Wave *wave) {
    g_wave_ginsert_head = group_head;
    g_wave_ginsert_wave = wave;
    ++g_wave_ginsert_calls;
    // add_to_group stores the wave's slot AFTER the insert; witness it.
    std::memcpy(&g_wave_ginsert_seen_slot,
                reinterpret_cast<uint8_t *>(wave) + 0x68, 4);
}

Sound *g_sound_own54_self;

uint32_t g_sound_own54_arg;

int g_sound_own54_calls;

int g_sound_own54_ret;

int __thiscall observe_sound_own_slot54(Sound *self, uint32_t arg) {
    g_sound_own54_self = self;
    g_sound_own54_arg = arg;
    ++g_sound_own54_calls;
    return g_sound_own54_ret;
}

// Group-walk observers: per-slot logs, an enabled-byte witness, and an
// optional cursor rewrite on the first visit (simulating a handler that
// pulls waves from the group mid-walk).
std::vector<Wave *> g_wave_volume_log;

std::vector<uint32_t> g_wave_volume_args;

std::vector<Wave *> g_wave_resume_log;

std::vector<Wave *> g_wave_halt_log;

std::vector<uint8_t> g_wave_visit_seen_enabled;

uint8_t *g_wave_visit_group_base;

void *g_wave_visit_cursor_write;

int g_wave_visit_total;

void wave_visit_common() {
    ++g_wave_visit_total;
    if (g_wave_visit_group_base) {
        g_wave_visit_seen_enabled.push_back(g_wave_visit_group_base[0]);
        if (g_wave_visit_cursor_write && g_wave_visit_total == 1) {
            std::memcpy(g_wave_visit_group_base + 0x10,
                        &g_wave_visit_cursor_write, 4);
        }
    }
}

std::vector<uint32_t> g_wave_visit_seen_volume;

void __thiscall observe_wave_group_volume(Wave *self, uint32_t arg) {
    g_wave_volume_log.push_back(self);
    g_wave_volume_args.push_back(arg);
    // The group's volume dword is stored BEFORE the walk; witness it.
    if (g_wave_visit_group_base) {
        uint32_t gv;
        std::memcpy(&gv, g_wave_visit_group_base + 4, 4);
        g_wave_visit_seen_volume.push_back(gv);
    }
    wave_visit_common();
}

void __thiscall observe_wave_group_resume(Wave *self) {
    g_wave_resume_log.push_back(self);
    wave_visit_common();
}

void __thiscall observe_wave_group_halt(Wave *self) {
    g_wave_halt_log.push_back(self);
    wave_visit_common();
}

int g_wdev_factory_calls;

void **g_wdev_factory_slot_arg;

unsigned long g_wdev_factory_kind;

int g_wdev_factory_ret;

void *g_wdev_factory_installs;

int __cdecl observe_wdev_factory(void **slot, unsigned long kind) {
    ++g_wdev_factory_calls;
    g_wdev_factory_slot_arg = slot;
    g_wdev_factory_kind = kind;
    if (g_wdev_factory_installs) {
        *slot = g_wdev_factory_installs;
    }
    return g_wdev_factory_ret;
}

uint8_t *g_wdev_down_obj;

uint32_t g_wdev_down_seen_vtable;

void *g_vec_iter_array;

unsigned int g_vec_iter_size;

int g_vec_iter_count;

void *g_vec_iter_ctor;

void *g_vec_iter_dtor;

int g_vec_iter_calls;

uint32_t g_vec_iter_seen_vtable;

void __stdcall observe_vector_ctor_iter(void *array, unsigned int size,
                                        int count,
                                        func_thiscall_teardown *ctor,
                                        func_thiscall_teardown *dtor) {
    g_vec_iter_array = array;
    g_vec_iter_size = size;
    g_vec_iter_count = count;
    g_vec_iter_ctor = reinterpret_cast<void *>(ctor);
    g_vec_iter_dtor = reinterpret_cast<void *>(dtor);
    ++g_vec_iter_calls;
    std::memcpy(&g_vec_iter_seen_vtable, g_wdev_down_obj, 4);
}

void __stdcall observe_vector_dtor_iter(void *array, unsigned int size,
                                        int count,
                                        func_thiscall_teardown *teardown) {
    g_vec_iter_array = array;
    g_vec_iter_size = size;
    g_vec_iter_count = count;
    g_vec_iter_dtor = reinterpret_cast<void *>(teardown);
    ++g_vec_iter_calls;
    std::memcpy(&g_vec_iter_seen_vtable, g_wdev_down_obj, 4);
}

int g_wdev_destroy_calls;

void *g_wdev_destroy_seen_device;

void __cdecl observe_wdev_destroy(void) {
    ++g_wdev_destroy_calls;
    // The hook runs BEFORE the field clears; witness what it still sees.
    std::memcpy(&g_wdev_destroy_seen_device, g_wdev_down_obj + 0x14, 4);
}

int g_wdev_own0_calls;

unsigned long g_wdev_own0_mode;

int g_wdev_own0_ret;

int __thiscall observe_wdev_own_slot0(Wave_Device *, unsigned long mode) {
    ++g_wdev_own0_calls;
    g_wdev_own0_mode = mode;
    return g_wdev_own0_ret;
}

int g_wdev_own4_calls;

void __thiscall observe_wdev_own_slot4(Wave_Device *) {
    ++g_wdev_own4_calls;
}

int g_wdev_down_calls;

bool g_wdev_down_clears;

void __thiscall observe_wdev_device_down(void *) {
    ++g_wdev_down_calls;
    std::memcpy(&g_wdev_down_seen_vtable, g_wdev_down_obj, 4);
    if (g_wdev_down_clears) {
        // release re-reads the device field after this callback; clearing it
        // here must suppress the destroy hook.
        const uint32_t zero = 0;
        std::memcpy(g_wdev_down_obj + 0x14, &zero, 4);
    }
}

int g_wave_stype_calls;

Wave *g_wave_stype_wave;

uint32_t g_wave_stype_type;

uint32_t g_wave_stype_seen_vtable;

uint32_t g_wave_stype_seen_f40;

uint8_t g_wave_stype_seen_f54;

uint32_t g_wave_stype_seen_pitch;

void __thiscall observe_wave_set_type(Wave *wave, uint32_t type) {
    g_wave_stype_wave = wave;
    g_wave_stype_type = type;
    ++g_wave_stype_calls;
    const uint8_t *raw = reinterpret_cast<const uint8_t *>(wave);
    std::memcpy(&g_wave_stype_seen_vtable, raw, 4);
    std::memcpy(&g_wave_stype_seen_f40, raw + 0x40, 4);
    g_wave_stype_seen_f54 = raw[0x54];
    std::memcpy(&g_wave_stype_seen_pitch, raw + 0x58, 4);
}

int g_wave_own7C_calls;

uint32_t g_wave_own7C_seen_flags;

void *g_wave_own7C_restores_vtbl;

void __thiscall observe_wave_own_slot7C(Wave *self) {
    ++g_wave_own7C_calls;
    // reload sets the loaded bit BEFORE this callback; witness the word.
    std::memcpy(&g_wave_own7C_seen_flags,
                reinterpret_cast<uint8_t *>(self) + 0x40, 4);
    if (g_wave_own7C_restores_vtbl) {
        // Undo a poison swap installed by the 0x70 observer: init dispatches
        // through the LIVE device vtable after this callback, so the table
        // must be healthy again by then.
        void *device;
        std::memcpy(&device, reinterpret_cast<uint8_t *>(self) + 0x3C, 4);
        std::memcpy(device, &g_wave_own7C_restores_vtbl, 4);
    }
}

void *g_wave_dtor_release_dev;

int g_wave_dtor_release_calls;

uint32_t g_wave_dtor_release_seen_vtable;

bool g_wave_dtor_release_relinks;

void *g_wave_dtor_release_relink_prev;

void *g_wave_dtor_release_relink_next;

void __cdecl observe_wave_release(void *device) {
    g_wave_dtor_release_dev = device;
    ++g_wave_dtor_release_calls;
    std::memcpy(&g_wave_dtor_release_seen_vtable, g_wave_dtor_obj, 4);
    if (g_wave_dtor_release_relinks) {
        uint32_t flags;
        std::memcpy(&flags, g_wave_dtor_obj + 0x40, 4);
        flags |= 2;
        std::memcpy(g_wave_dtor_obj + 0x40, &flags, 4);
        std::memcpy(g_wave_dtor_obj + 0x44, &g_wave_dtor_release_relink_prev,
                    4);
        std::memcpy(g_wave_dtor_obj + 0x48, &g_wave_dtor_release_relink_next,
                    4);
    }
}

}  // namespace

void test_wave_is_playing() {
    // is_playing lets a wrapped device at 0x3C answer for itself through the
    // device's own vtable slot 0x5C. With no device it times the wave against
    // the clock: bit 4 of the flag byte at 0x54 arms it, a zero start stamp at
    // 0x64 means it never began, and the stored length at 0x60 is compared
    // UNSIGNED against the elapsed time.
    std::vector<uint8_t> storage(sizeof(Wave) + 32, 0);
    std::vector<uint8_t> expected(storage.size());
    auto *wave = reinterpret_cast<Wave *>(storage.data() + 16);
    uint8_t *const obj = storage.data() + 16;

    // Poison the neighbouring slots: an off-by-one dispatch faults rather than
    // silently landing on another observer.
    void *dev_vtable[32] = {};
    dev_vtable[0x5C / 4] = reinterpret_cast<void *>(&observe_wave_device_is_playing);
    dev_vtable[0x58 / 4] = nullptr;
    dev_vtable[0x60 / 4] = nullptr;
    struct FakeDev { void *vtbl; } fake_dev;
    fake_dev.vtbl = dev_vtable;

    func_time_get_time *time_fn = &observe_wave_time_get_time;
    func_time_get_time **const saved_slot = WaveTimeGetTimeSlot;
    WaveTimeGetTimeSlot = &time_fn;

    auto set_device = [&](void *d) { std::memcpy(obj + 0x3C, &d, sizeof(d)); };
    auto set32 = [&](size_t off, uint32_t v) { std::memcpy(obj + off, &v, 4); };

    // A wrapped device answers, with itself as the receiver, and its result is
    // returned verbatim rather than normalised to 0/1.
    set_device(&fake_dev);
    obj[0x54] = 0;
    set32(0x60, 0);
    set32(0x64, 0);
    g_wave_play_dev_calls = g_wave_time_calls = 0;
    g_wave_play_dev_ret = 0x2A;
    expect(wave->is_playing() == 0x2A);
    expect(g_wave_play_dev_calls == 1 && g_wave_play_dev_self == &fake_dev);
    expect(g_wave_time_calls == 0);          // the clock is not consulted

    // The device wins over every clock input: armed flag, live stamp, long
    // length, and it still returns the device's own answer.
    obj[0x54] = 0x10;
    set32(0x60, 1000);
    set32(0x64, 1);
    g_wave_play_dev_calls = g_wave_time_calls = 0;
    g_wave_play_dev_ret = 0;
    expect(wave->is_playing() == 0);
    expect(g_wave_play_dev_calls == 1);
    expect(g_wave_time_calls == 0);

    // No device and the arming bit clear: 0, and the clock is never read.
    set_device(nullptr);
    obj[0x54] = 0xEF;                        // every bit but 4
    set32(0x60, 1000);
    set32(0x64, 1);
    g_wave_time_calls = 0;
    expect(wave->is_playing() == 0);
    expect(g_wave_time_calls == 0);

    // Bit 4 alone arms it; the other bits are irrelevant either way.
    obj[0x54] = 0x10;
    set32(0x64, 0);                          // never started
    g_wave_time_calls = 0;
    expect(wave->is_playing() == 0);
    expect(g_wave_time_calls == 0);          // the zero stamp short-circuits

    // Started and still inside the length: playing.
    obj[0x54] = 0xFF;
    set32(0x60, 1000);
    set32(0x64, 5000);
    g_wave_time_value = 5999;                // elapsed 999
    g_wave_time_calls = 0;
    expect(wave->is_playing() == 1);
    expect(g_wave_time_calls == 1);

    // The boundary is `jbe`: elapsed == length is finished, one less is not.
    g_wave_time_value = 6000;                // elapsed 1000 == length
    expect(wave->is_playing() == 0);
    g_wave_time_value = 5000;                // elapsed 0
    expect(wave->is_playing() == 1);

    // The comparison is unsigned, so a negative stored length reads as an
    // enormous one rather than as already finished.
    set32(0x60, 0xFFFFFFFFu);
    g_wave_time_value = 5000u + 0x7FFFFFFFu;
    expect(wave->is_playing() == 1);

    // The subtraction is unsigned too: a clock that has wrapped below the
    // stamp yields a huge elapsed, not a negative one.
    set32(0x60, 1000);
    set32(0x64, 0xFFFFFF00u);
    g_wave_time_value = 4;                   // wrapped; elapsed 0x104
    expect(wave->is_playing() == 1);
    g_wave_time_value = 0xFFFFFEFFu;         // one tick before the stamp
    expect(wave->is_playing() == 0);         // elapsed 0xFFFFFFFF

    // Nothing above writes to the object. Seeding comes first: it fills every
    // byte, including the device pointer at 0x3C, so the fields the clock path
    // needs have to be written after it and snapshotted after that.
    seed_storage(storage.data(), expected.data(), storage.size());
    set_device(nullptr);
    obj[0x54] = 0x10;
    set32(0x60, 1000);
    set32(0x64, 5000);
    std::memcpy(expected.data(), storage.data(), storage.size());
    g_wave_time_value = 5500;
    wave->is_playing();
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // Redirect entry, on both branches.
    set32(0x60, 1000);
    set32(0x64, 5000);
    obj[0x54] = 0x10;
    g_wave_time_value = 5500;
    expect(wave_is_playing_redirect(wave, nullptr) == 1);
    set_device(&fake_dev);
    g_wave_play_dev_ret = 0x77;
    expect(wave_is_playing_redirect(wave, nullptr) == 0x77);

    WaveTimeGetTimeSlot = saved_slot;
}

void test_wave_play() {
    // play is the wrapped-device forwarder shape: the device at 0x3C answers
    // through its own vtable slot 0x94 as the receiver, its result returned
    // verbatim; with no device the answer is a fixed 0x14.
    std::vector<uint8_t> storage(sizeof(Wave) + 32, 0);
    std::vector<uint8_t> expected(storage.size());
    uint8_t *const obj = storage.data() + 16;
    auto *wave = reinterpret_cast<Wave *>(obj);

    // Poisoned neighbours: an off-by-one dispatch faults rather than landing
    // on another observer.
    void *dev_vtable[64] = {};
    dev_vtable[0x94 / 4] = reinterpret_cast<void *>(&observe_wave_device_play);
    dev_vtable[0x90 / 4] = nullptr;
    dev_vtable[0x98 / 4] = nullptr;
    struct FakeDev { void *vtbl; } fake_dev;
    fake_dev.vtbl = dev_vtable;

    seed_storage(storage.data(), expected.data(), storage.size());
    void *dev = &fake_dev;
    std::memcpy(obj + 0x3C, &dev, 4);
    std::memcpy(expected.data(), storage.data(), storage.size());

    g_wave_playm_calls = 0;
    g_wave_playm_ret = 0x5A17;
    expect(wave->play(0x1234) == 0x5A17);
    expect(g_wave_playm_calls == 1);
    expect(g_wave_playm_dev_self == &fake_dev);
    expect(g_wave_playm_arg == 0x1234);

    // The result is verbatim, not normalised.
    g_wave_playm_ret = 0;
    expect(wave->play(-7) == 0);
    expect(g_wave_playm_arg == -7);

    // Nothing writes the object on the device path.
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // No device: the fixed answer, no dispatch, still no writes.
    dev = nullptr;
    std::memcpy(obj + 0x3C, &dev, 4);
    std::memcpy(expected.data(), storage.data(), storage.size());
    g_wave_playm_calls = 0;
    expect(wave->play(77) == 0x14);
    expect(g_wave_playm_calls == 0);
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // Redirect entry, both paths.
    expect(wave_play_redirect(wave, nullptr, 5) == 0x14);
    dev = &fake_dev;
    std::memcpy(obj + 0x3C, &dev, 4);
    g_wave_playm_ret = 9;
    expect(wave_play_redirect(wave, nullptr, 6) == 9);
    expect(g_wave_playm_arg == 6);
}

void test_wave_device_forwarders() {
    // The rest of the wrapped-device family: each method dispatches through
    // one device vtable slot with the device as receiver, or answers a fixed
    // default with no device. Slots and defaults vary per method; nothing
    // here writes the object except set_reverb_mix (0x5C) and set_attrib
    // (0x30 and the 0x54 flag byte).
    std::vector<uint8_t> storage(sizeof(Wave) + 32, 0);
    std::vector<uint8_t> expected(storage.size());
    uint8_t *const obj = storage.data() + 16;
    auto *wave = reinterpret_cast<Wave *>(obj);

    void *dev_vtable[64];
    struct FakeDev { void *vtbl; } fake_dev;
    fake_dev.vtbl = dev_vtable;
    auto arm = [&](size_t slot, void *fn) {
        std::memset(dev_vtable, 0, sizeof(dev_vtable));
        dev_vtable[slot / 4] = fn;
        g_wave_fam_calls = 0;
    };
    auto set_device = [&](void *d) { std::memcpy(obj + 0x3C, &d, 4); };
    auto bits_of = [](float f) {
        uint32_t b;
        std::memcpy(&b, &f, 4);
        return b;
    };

    seed_storage(storage.data(), expected.data(), storage.size());
    set_device(&fake_dev);
    std::memcpy(expected.data(), storage.data(), storage.size());

    // --- the pure forwarders, device path: result verbatim, receiver the
    // device, argument dwords passed through ---
    arm(0xC8, reinterpret_cast<void *>(&observe_wave_dev0));
    g_wave_fam_ret = 0x1101;
    expect(wave->is_hwbuffer() == 0x1101);
    expect(g_wave_fam_calls == 1 && g_wave_fam_self == &fake_dev);

    arm(0xB8, reinterpret_cast<void *>(&observe_wave_dev0));
    g_wave_fam_ret = 0x1102;
    expect(wave->get_current_marker() == 0x1102);
    expect(g_wave_fam_calls == 1);

    arm(0x3C, reinterpret_cast<void *>(&observe_wave_dev0));
    g_wave_fam_ret = 0x1103;
    expect(wave->get_game_hwnd() == 0x1103);
    expect(g_wave_fam_calls == 1);

    arm(0xBC, reinterpret_cast<void *>(&observe_wave_dev0));
    g_wave_fam_ret = 0x1104;
    expect(wave->get_ndevices() == 0x1104);
    expect(g_wave_fam_calls == 1);

    arm(0xDC, reinterpret_cast<void *>(&observe_wave_dev0));
    g_wave_fam_ret = 0x1105;
    expect(wave->is_3d() == 0x1105);
    expect(g_wave_fam_calls == 1);

    arm(0xB4, reinterpret_cast<void *>(&observe_wave_dev1));
    g_wave_fam_ret = 0x1106;
    expect(wave->get_time(0xCAFE) == 0x1106);
    expect(g_wave_fam_calls == 1 && g_wave_fam_args[0] == 0xCAFE);

    arm(0xD0, reinterpret_cast<void *>(&observe_wave_dev1));
    g_wave_fam_ret = 0x1107;
    expect(wave->set_xpos(2.5f) == 0x1107);
    expect(g_wave_fam_calls == 1 && g_wave_fam_args[0] == bits_of(2.5f));

    arm(0xD4, reinterpret_cast<void *>(&observe_wave_dev1));
    g_wave_fam_ret = 0x1108;
    expect(wave->set_ypos(-3.25f) == 0x1108);
    expect(g_wave_fam_calls == 1 && g_wave_fam_args[0] == bits_of(-3.25f));

    arm(0xD8, reinterpret_cast<void *>(&observe_wave_dev1));
    g_wave_fam_ret = 0x1109;
    expect(wave->set_zpos(0.5f) == 0x1109);
    expect(g_wave_fam_calls == 1 && g_wave_fam_args[0] == bits_of(0.5f));

    arm(0xCC, reinterpret_cast<void *>(&observe_wave_dev3));
    g_wave_fam_ret = 0x110A;
    expect(wave->set_position3d(1.0f, 2.0f, 3.0f) == 0x110A);
    expect(g_wave_fam_calls == 1 && g_wave_fam_args[0] == bits_of(1.0f) &&
           g_wave_fam_args[1] == bits_of(2.0f) &&
           g_wave_fam_args[2] == bits_of(3.0f));

    char descr_buf[4] = {'Z', 'Z', 'Z', 'Z'};
    arm(0xC0, reinterpret_cast<void *>(&observe_wave_dev3));
    g_wave_fam_ret = 0x110B;
    expect(wave->get_device_description(descr_buf, 0x22, 0x33) == 0x110B);
    expect(g_wave_fam_calls == 1 &&
           g_wave_fam_args[0] == reinterpret_cast<uintptr_t>(descr_buf) &&
           g_wave_fam_args[1] == 0x22 && g_wave_fam_args[2] == 0x33);
    expect(descr_buf[0] == 'Z');  // the device path never touches the buffer

    // None of the above wrote the object.
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // --- the pure forwarders, no-device defaults ---
    set_device(nullptr);
    std::memcpy(expected.data(), storage.data(), storage.size());
    g_wave_fam_calls = 0;
    expect(wave->is_hwbuffer() == 0);
    expect(wave->get_current_marker() == -1);
    expect(wave->get_game_hwnd() == 0);
    expect(wave->get_ndevices() == 0);
    expect(wave->is_3d() == 0);
    expect(wave->get_time(1) == 0);
    expect(wave->set_xpos(1.0f) == 0x14);
    expect(wave->set_ypos(1.0f) == 0x14);
    expect(wave->set_zpos(1.0f) == 0x14);
    expect(wave->set_position3d(1.0f, 1.0f, 1.0f) == 0x14);
    expect(g_wave_fam_calls == 0);
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // get_device_description with no device: the buffer is terminated - one
    // byte only - but only when the third argument is nonzero.
    std::memcpy(descr_buf, "ZZZ", 4);
    expect(wave->get_device_description(descr_buf, 5, 0) == 1);
    expect(descr_buf[0] == 'Z');
    expect(wave->get_device_description(descr_buf, 5, 9) == 1);
    expect(descr_buf[0] == '\0' && descr_buf[1] == 'Z');
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // --- UNK1: chases the +0x3C chain from the device until null, answers 1,
    // ignores its argument ---
    expect(wave->UNK1(0x7777) == 1);  // no device: immediate
    uint8_t node_a[0x40] = {};
    uint8_t node_b[0x40] = {};
    void *link = node_b;
    std::memcpy(node_a + 0x3C, &link, 4);  // a -> b -> null
    // Poison offset 0 of both nodes with self-loops: a walk that follows the
    // wrong offset never terminates instead of luckily reading a null.
    void *self_a = node_a;
    void *self_b = node_b;
    std::memcpy(node_a, &self_a, 4);
    std::memcpy(node_b, &self_b, 4);
    set_device(node_a);
    expect(wave->UNK1(-5) == 1);
    set_device(nullptr);

    // --- set_reverb_mix: stores the value at 0x5C on BOTH paths ---
    std::memcpy(expected.data(), storage.data(), storage.size());
    expect(wave->set_reverb_mix(7.75f) == 0x14);  // no device
    {
        const uint32_t bits = bits_of(7.75f);
        std::memcpy(expected.data() + 16 + 0x5C, &bits, 4);
    }
    expect_storage_bytes(storage.data(), expected.data(), storage.size());
    set_device(&fake_dev);
    arm(0xE0, reinterpret_cast<void *>(&observe_wave_dev1));
    g_wave_fam_ret = 0x110C;
    expect(wave->set_reverb_mix(-1.5f) == 0x110C);
    expect(g_wave_fam_calls == 1 && g_wave_fam_args[0] == bits_of(-1.5f));
    {
        const uint32_t bits = bits_of(-1.5f);
        std::memcpy(expected.data() + 16 + 0x5C, &bits, 4);
        void *d = &fake_dev;
        std::memcpy(expected.data() + 16 + 0x3C, &d, 4);
    }
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // --- set_attrib: per-bit mapping onto the 0x54 flag byte ---
    const struct { uint32_t a1; uint8_t fl; } set_map[] = {
        {0x001, 0x01}, {0x004, 0x02}, {0x040, 0x08},
        {0x080, 0x10}, {0x010, 0x04}, {0x100, 0x20},
        // bit 2 suppresses the bit-4 and bit-8 mappings
        {0x014, 0x02}, {0x104, 0x02},
    };
    set_device(nullptr);
    for (const auto &c : set_map) {
        obj[0x54] = 0;
        wave->set_attrib(c.a1);
        expect(obj[0x54] == c.fl);
    }
    // Bits already set are never cleared, and bit 1 arms the dword at 0x30.
    obj[0x54] = 0xC0;
    uint32_t before_30;
    std::memcpy(&before_30, obj + 0x30, 4);
    wave->set_attrib(2);
    expect(obj[0x54] == 0xC0);
    uint32_t after_30;
    std::memcpy(&after_30, obj + 0x30, 4);
    expect(after_30 == 1 && before_30 != 1);
    // The device hears the raw mask through slot 0x6C.
    set_device(&fake_dev);
    arm(0x6C, reinterpret_cast<void *>(&observe_wave_dev1));
    wave->set_attrib(0x181);
    expect(g_wave_fam_calls == 1 && g_wave_fam_args[0] == 0x181);
    expect(obj[0x54] == (0xC0 | 0x01 | 0x10 | 0x20));

    // --- get_attrib: inverse mapping, OR-ed over the device's answer ---
    const struct { uint8_t fl; int out; } get_map[] = {
        {0x01, 0x001}, {0x02, 0x004}, {0x04, 0x010},
        {0x08, 0x040}, {0x10, 0x080}, {0x20, 0x100},
    };
    set_device(nullptr);
    std::memset(obj + 0x30, 0, 4);
    for (const auto &c : get_map) {
        obj[0x54] = c.fl;
        expect(wave->get_attrib() == c.out);
    }
    obj[0x54] = 0;
    std::memcpy(obj + 0x30, "\x05\x00\x00\x00", 4);
    expect(wave->get_attrib() == 2);  // any nonzero 0x30 dword reads as bit 1
    // Device path: its answer is the base the fields OR onto.
    set_device(&fake_dev);
    arm(0x70, reinterpret_cast<void *>(&observe_wave_dev0));
    g_wave_fam_ret = 0x10000;
    obj[0x54] = 0x3F;
    expect(wave->get_attrib() == (0x10000 | 2 | 0x1D5));
    expect(g_wave_fam_calls == 1 && g_wave_fam_self == &fake_dev);

    // --- redirect entries ---
    set_device(nullptr);
    expect(wave_is_hwbuffer_redirect(wave, nullptr) == 0);
    expect(wave_get_time_redirect(wave, nullptr, 2) == 0);
    expect(wave_get_current_marker_redirect(wave, nullptr) == -1);
    expect(wave_get_game_hwnd_redirect(wave, nullptr) == 0);
    expect(wave_get_ndevices_redirect(wave, nullptr) == 0);
    expect(wave_unk1_redirect(wave, nullptr, 3) == 1);
    expect(wave_set_reverb_mix_redirect(wave, nullptr, 1.0f) == 0x14);
    expect(wave_is_3d_redirect(wave, nullptr) == 0);
    char rbuf[2] = {'Q', 'Q'};
    expect(wave_get_device_description_redirect(wave, nullptr, rbuf, 1, 1) ==
           1);
    expect(rbuf[0] == '\0');
    expect(wave_set_position3d_redirect(wave, nullptr, 1, 2, 3) == 0x14);
    expect(wave_set_xpos_redirect(wave, nullptr, 1) == 0x14);
    expect(wave_set_ypos_redirect(wave, nullptr, 1) == 0x14);
    expect(wave_set_zpos_redirect(wave, nullptr, 1) == 0x14);
    obj[0x54] = 0;
    std::memset(obj + 0x30, 0, 4);
    wave_set_attrib_redirect(wave, nullptr, 1);
    expect(obj[0x54] == 1);
    expect(wave_get_attrib_redirect(wave, nullptr) == 1);
}

void test_wave_volume_fname_play() {
    std::vector<uint8_t> storage(sizeof(Wave) + 32, 0);
    std::vector<uint8_t> expected(storage.size());
    uint8_t *const obj = storage.data() + 16;
    auto *wave = reinterpret_cast<Wave *>(obj);
    g_wave_dtor_obj = obj;

    auto *const saved_new = WaveOperatorNew;
    auto *const saved_delete = WaveOperatorDelete;
    auto *const saved_gd = WaveDeviceIsGroupDisabled;
    auto *const saved_load = WaveOriginalLoad;
    uint32_t *const saved_gvol = WaveDeviceGroupVolumes;
    void *const saved_dev_global = WaveDeviceGlobal;
    func_time_get_time **const saved_time = WaveTimeGetTimeSlot;

    int fake_singleton = 0;
    func_time_get_time *time_fn = &observe_wave_time_get_time;
    uint32_t gtable[16 * 6];
    for (auto &v : gtable) v = 0xDDDDDDDDu;
    WaveOperatorNew = &observe_wave_operator_new;
    WaveOperatorDelete = &observe_wave_operator_delete;
    WaveDeviceIsGroupDisabled = &observe_wave_group_disabled;
    WaveOriginalLoad = &observe_wave_original_load;
    WaveDeviceGroupVolumes = gtable;
    WaveDeviceGlobal = &fake_singleton;
    WaveTimeGetTimeSlot = &time_fn;

    void *dev_vtable[64];
    struct FakeDev { void *vtbl; } fake_dev;
    fake_dev.vtbl = dev_vtable;
    auto arm_dev = [&](size_t slot, void *fn) {
        std::memset(dev_vtable, 0, sizeof(dev_vtable));
        dev_vtable[slot / 4] = fn;
        g_wave_fam_calls = 0;
    };
    void *own_vtable[64];
    std::memset(own_vtable, 0, sizeof(own_vtable));
    own_vtable[0x40 / 4] = reinterpret_cast<void *>(&observe_wave_own_slot40);
    own_vtable[0x80 / 4] = reinterpret_cast<void *>(&observe_wave_own_slot80);

    auto set32 = [&](size_t off, uint32_t v) { std::memcpy(obj + off, &v, 4); };
    auto get32 = [&](size_t off) {
        uint32_t v;
        std::memcpy(&v, obj + off, 4);
        return v;
    };
    auto setp = [&](size_t off, const void *p) {
        std::memcpy(obj + off, &p, 4);
    };
    auto getp = [&](size_t off) {
        void *p;
        std::memcpy(&p, obj + off, 4);
        return p;
    };

    // --- set_volume: the group-scaled level ---
    seed_storage(storage.data(), expected.data(), storage.size());
    set32(0x68, 3);
    gtable[3 * 6] = 200;
    setp(0x3C, &fake_dev);
    arm_dev(0x40, reinterpret_cast<void *>(&observe_wave_dev1));
    std::memcpy(expected.data(), storage.data(), storage.size());
    wave->set_volume(0x180 | 0x7F);  // only the low seven bits survive
    expect(get32(0x04) == 127);
    expect(g_wave_fam_calls == 1 && g_wave_fam_self == &fake_dev);
    expect(g_wave_fam_args[0] == 200);  // 127/127 * 200
    {
        const uint32_t vol = 127;
        std::memcpy(expected.data() + 16 + 0x04, &vol, 4);
    }
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    gtable[3 * 6] = 100;
    arm_dev(0x40, reinterpret_cast<void *>(&observe_wave_dev1));
    wave->set_volume(64);
    expect(get32(0x04) == 64);
    expect(g_wave_fam_calls == 1 && g_wave_fam_args[0] == 50);  // 64/127*100

    // Outside the group range the raw masked level reaches the device.
    set32(0x68, 0x10);
    arm_dev(0x40, reinterpret_cast<void *>(&observe_wave_dev1));
    wave->set_volume(90);
    expect(g_wave_fam_calls == 1 && g_wave_fam_args[0] == 90);

    // No device: the volume is still remembered, nothing is dispatched.
    setp(0x3C, nullptr);
    arm_dev(0x40, reinterpret_cast<void *>(&observe_wave_dev1));
    wave->set_volume(5);
    expect(get32(0x04) == 5 && g_wave_fam_calls == 0);

    // --- set_fname: game-heap string ownership ---
    g_wave_dtor_deletes.clear();
    g_wave_dtor_delete_seen_slot.clear();
    g_wave_new_calls = 0;
    setp(0x4C, nullptr);
    std::memcpy(expected.data(), storage.data(), storage.size());
    expect(wave->set_fname(nullptr) == 0xA);
    expect(g_wave_new_calls == 0 && g_wave_dtor_deletes.empty());
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    expect(wave->set_fname("wav/menu.wav") == 0);
    expect(g_wave_new_calls == 1 && g_wave_new_size == 13);
    expect(getp(0x4C) == g_wave_new_arena);
    expect(std::strcmp(g_wave_new_arena, "wav/menu.wav") == 0);
    expect(g_wave_dtor_deletes.empty());  // nothing to free the first time

    // A previous name goes back to the game heap first.
    char old_name[4] = {'x', 0, 0, 0};
    setp(0x4C, old_name);
    g_wave_new_calls = 0;
    expect(wave->set_fname("ok") == 0);
    expect(g_wave_dtor_deletes.size() == 1 &&
           g_wave_dtor_deletes[0] == old_name);
    expect(g_wave_new_calls == 1 && g_wave_new_size == 3);
    expect(getp(0x4C) == g_wave_new_arena &&
           std::strcmp(g_wave_new_arena, "ok") == 0);

    // --- play(): the start protocol ---
    auto reset_play = [&] {
        g_wave_gd_calls = g_wave_oload_calls = 0;
        g_wave_own40_calls = g_wave_own80_calls = 0;
        g_wave_fam_calls = 0;
        g_wave_time_calls = 0;
        g_wave_oload_installs = nullptr;
    };
    seed_storage(storage.data(), expected.data(), storage.size());
    setp(0x00, own_vtable);

    // A: in range, "disabled" answer with a zero low byte does NOT disable;
    // unclocked wave with a device: only the device start runs.
    set32(0x68, 5);
    obj[0x54] = 0xEF;  // every bit but 4
    setp(0x3C, &fake_dev);
    set32(0x04, 77);
    arm_dev(0x1C, reinterpret_cast<void *>(&observe_wave_dev0));
    reset_play();
    g_wave_gd_ret = 0x100;
    g_wave_fam_ret = 0x2211;
    std::memcpy(expected.data(), storage.data(), storage.size());
    expect(wave->play() == 0x2211);
    expect(g_wave_gd_calls == 1 && g_wave_gd_dev == &fake_singleton &&
           g_wave_gd_slot == 5);
    expect(g_wave_own40_calls == 0);
    expect(g_wave_fam_calls == 1 && g_wave_fam_self == &fake_dev);
    expect(g_wave_own80_calls == 0 && g_wave_time_calls == 0);
    expect(getp(0x3C) == &fake_dev);  // unclocked: the device is kept
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // B: a genuinely disabled group answers 0x14 before anything happens.
    reset_play();
    g_wave_gd_ret = 1;
    expect(wave->play() == 0x14);
    expect(g_wave_gd_calls == 1 && g_wave_own40_calls == 0 &&
           g_wave_fam_calls == 0 && g_wave_oload_calls == 0);

    // C: clocked wave with a device: volume replay through the wave's own
    // slot 0x40, device start, then the epilogue - timestamp, own slot 0x80,
    // device forgotten.
    obj[0x54] = 0x10;
    reset_play();
    g_wave_gd_ret = 0;
    g_wave_fam_ret = 0x3322;
    g_wave_time_value = 0xABCD1234u;
    expect(wave->play() == 0x3322);
    expect(g_wave_own40_calls == 1 && g_wave_own40_self == wave &&
           g_wave_own40_arg == 77);
    expect(g_wave_fam_calls == 1);
    expect(g_wave_time_calls == 1 && get32(0x64) == 0xABCD1234u);
    expect(g_wave_own80_calls == 1 && g_wave_own80_self == wave);
    expect(g_wave_own80_seen_stamp == 0xABCD1234u);
    expect(g_wave_own80_seen_device == &fake_dev);
    expect(getp(0x3C) == nullptr);

    // D: out of group range (no disabled query), clocked, no device: the
    // original load runs and the device it wraps is started.
    set32(0x68, 0x10);
    reset_play();
    g_wave_oload_installs = &fake_dev;
    arm_dev(0x1C, reinterpret_cast<void *>(&observe_wave_dev0));
    g_wave_fam_ret = 0x4433;
    expect(wave->play() == 0x4433);
    expect(g_wave_gd_calls == 0 && g_wave_own40_calls == 0);
    expect(g_wave_oload_calls == 1);
    expect(g_wave_fam_calls == 1);
    expect(g_wave_own80_calls == 1 && getp(0x3C) == nullptr);
    expect(g_wave_own80_seen_device == &fake_dev);

    // E: the load wraps nothing: the answer is 0, the epilogue still runs.
    reset_play();
    expect(wave->play() == 0);
    expect(g_wave_oload_calls == 1 && g_wave_fam_calls == 0);
    expect(g_wave_own80_calls == 1);

    // F: unclocked with no device: the fixed 0x14, and nothing runs at all.
    obj[0x54] = 0xEF;
    reset_play();
    expect(wave->play() == 0x14);
    expect(g_wave_gd_calls == 0 && g_wave_oload_calls == 0 &&
           g_wave_own40_calls == 0 && g_wave_own80_calls == 0 &&
           g_wave_time_calls == 0);

    // --- the scalar deleting destructor ---
    auto *const saved_pull = WaveDevicePullFromGroup;
    WaveDevicePullFromGroup = &observe_wave_pull_from_group;
    g_wave_dtor_pull_calls = 0;
    g_wave_dtor_deletes.clear();
    g_wave_dtor_delete_seen_slot.clear();
    set32(0x68, 0x10);  // no group pull
    set32(0x40, 0);     // not chained
    setp(0x4C, nullptr);
    setp(0x3C, nullptr);
    expect(wave_scalar_dtor_redirect(wave, nullptr, 0) == wave);
    expect(get32(0x00) == 0x0066E444u);  // the destructor really ran
    expect(g_wave_dtor_deletes.empty()); // mode 0: storage kept
    set32(0x68, 0x10);
    set32(0x40, 0);
    expect(wave_scalar_dtor_redirect(wave, nullptr, 3) == wave);
    expect(g_wave_dtor_deletes.size() == 1 && g_wave_dtor_deletes[0] == wave);
    expect(g_wave_dtor_pull_calls == 0);
    WaveDevicePullFromGroup = saved_pull;

    // --- redirect entries ---
    set32(0x68, 0x10);
    setp(0x3C, nullptr);
    wave_set_volume_redirect(wave, nullptr, 9);
    expect(get32(0x04) == 9);
    setp(0x4C, nullptr);
    expect(wave_set_fname_redirect(wave, nullptr, nullptr) == 0xA);
    obj[0x54] = 0xEF;
    expect(wave_play_empty_redirect(wave, nullptr) == 0x14);

    WaveOperatorNew = saved_new;
    WaveOperatorDelete = saved_delete;
    WaveDeviceIsGroupDisabled = saved_gd;
    WaveOriginalLoad = saved_load;
    WaveDeviceGroupVolumes = saved_gvol;
    WaveDeviceGlobal = saved_dev_global;
    WaveTimeGetTimeSlot = saved_time;
}

void test_wave_load_empty() {
    std::vector<uint8_t> storage(sizeof(Wave) + 32, 0);
    std::vector<uint8_t> expected(storage.size());
    uint8_t *const obj = storage.data() + 16;
    auto *wave = reinterpret_cast<Wave *>(obj);

    auto **const saved_create = WaveDeviceCreateSlot;
    int *const saved_guard = WaveDeviceReleaseGuard;
    auto *const saved_sload = SoundOriginalLoad;
    func_wave_device_create *create_fn = &observe_wave_device_create;
    int guard = 0;
    WaveDeviceCreateSlot = &create_fn;
    WaveDeviceReleaseGuard = &guard;
    SoundOriginalLoad = &observe_wave_sound_load;

    void *dev_vtable[64];
    std::memset(dev_vtable, 0, sizeof(dev_vtable));
    dev_vtable[0x6C / 4] = reinterpret_cast<void *>(&observe_wave_dev1);
    dev_vtable[0xC4 / 4] = reinterpret_cast<void *>(&observe_wave_dev0);
    struct FakeDev { void *vtbl; } fake_dev;
    fake_dev.vtbl = dev_vtable;
    void *own_vtable[64];
    std::memset(own_vtable, 0, sizeof(own_vtable));
    own_vtable[0x58 / 4] = reinterpret_cast<void *>(&observe_wave_own_slot58);
    own_vtable[0x70 / 4] = reinterpret_cast<void *>(&observe_wave_own_slot70);
    own_vtable[0x7C / 4] = reinterpret_cast<void *>(&observe_wave_own_slot7C);

    auto set32 = [&](size_t off, uint32_t v) { std::memcpy(obj + off, &v, 4); };
    auto get32 = [&](size_t off) {
        uint32_t v;
        std::memcpy(&v, obj + off, 4);
        return v;
    };
    auto setp = [&](size_t off, const void *p) {
        std::memcpy(obj + off, &p, 4);
    };
    auto getp = [&](size_t off) {
        void *p;
        std::memcpy(&p, obj + off, 4);
        return p;
    };
    auto reset_load = [&] {
        g_wave_create_calls = g_wave_sload_calls = 0;
        g_wave_own58_calls = g_wave_fam_calls = 0;
        g_wave_own70_calls = g_wave_own7C_calls = 0;
        g_wave_fam_arg_log.clear();
        g_wave_create_installs = nullptr;
        g_wave_create_ret = 0;
        g_wave_own58_ret = 0;
        g_wave_own70_ret = 0;
        g_wave_sload_ret = 0;
    };
    char name_buf[] = "x.wav";

    seed_storage(storage.data(), expected.data(), storage.size());
    setp(0x00, own_vtable);

    // No filename: the fixed 8, nothing runs.
    setp(0x4C, nullptr);
    reset_load();
    expect(wave->load() == 8);
    expect(g_wave_create_calls == 0 && g_wave_sload_calls == 0 &&
           g_wave_own58_calls == 0);

    // No device and a dead creation hook: 1, the hook is never entered.
    setp(0x4C, name_buf);
    setp(0x3C, nullptr);
    guard = 0;
    reset_load();
    expect(wave->load() == 1);
    expect(g_wave_create_calls == 0 && g_wave_sload_calls == 0);

    // A failing creation propagates its error before any attribute work.
    guard = 1;
    reset_load();
    g_wave_create_ret = 0x33;
    expect(wave->load() == 0x33);
    expect(g_wave_create_calls == 1);
    expect(g_wave_create_slot_arg == reinterpret_cast<void **>(obj + 0x3C));
    expect(g_wave_create_name == reinterpret_cast<char *>(name_buf) &&
           g_wave_create_mode == 1);
    expect(g_wave_sload_calls == 0 && g_wave_own58_calls == 0);

    // Creation installs the device into 0x3C; a failing Sound::load then
    // propagates ITS error after the device heard an empty attribute mask.
    obj[0x54] = 0;
    reset_load();
    g_wave_create_installs = &fake_dev;
    g_wave_sload_ret = 0x55;
    expect(wave->load() == 0x55);
    expect(getp(0x3C) == &fake_dev);
    expect(g_wave_own58_calls == 1);
    expect(g_wave_fam_calls == 1 && g_wave_fam_args[0] == 0);
    expect(g_wave_sload_calls == 1 && g_wave_sload_wave == wave &&
           g_wave_sload_name == reinterpret_cast<char *>(name_buf));
    expect(get32(0x60) != 0x7654u);  // no length query on failure

    // Success with every flag bit: attributes fold per bit (bit 5 suppresses
    // the bit-3 companion), the wave's own slot 0x58 adds bit 1, and the
    // device's slot 0xC4 answer lands in the length field.
    obj[0x54] = 0x3D;
    reset_load();
    g_wave_own58_ret = 7;
    g_wave_fam_ret = 0x7654;
    std::memcpy(expected.data(), storage.data(), storage.size());
    expect(wave->load() == 0);
    expect(g_wave_create_calls == 0);  // the device already existed
    expect(g_wave_own58_calls == 1);
    expect(g_wave_fam_calls == 2 && g_wave_fam_args[0] == 0x1D3);
    expect(g_wave_sload_calls == 1);
    expect(get32(0x60) == 0x7654u);
    {
        const uint32_t len = 0x7654;
        std::memcpy(expected.data() + 16 + 0x60, &len, 4);
    }
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // Bit 3 alone brings its companion bit 0 along.
    obj[0x54] = 0x08;
    reset_load();
    g_wave_fam_ret = 0x11;
    expect(wave->load() == 0);
    expect(g_wave_fam_args[0] == 0x41);

    // Bit 0 alone maps to attribute bit 0 with nothing to mask it.
    obj[0x54] = 0x01;
    reset_load();
    expect(wave->load() == 0);
    expect(g_wave_fam_args[0] == 1);

    // Bit 5 suppresses bit 3's companion: 0x40 and 0x100, and nothing else.
    obj[0x54] = 0x28;
    reset_load();
    expect(wave->load() == 0);
    expect(g_wave_fam_args[0] == 0x140);

    // --- reload(): the loaded-bit protocol ---
    std::memset(dev_vtable, 0, sizeof(dev_vtable));
    dev_vtable[0x6C / 4] = reinterpret_cast<void *>(&observe_wave_dev1);
    dev_vtable[0x84 / 4] = reinterpret_cast<void *>(&observe_wave_dev0);
    dev_vtable[0x48 / 4] = reinterpret_cast<void *>(&observe_wave_dev1);

    setp(0x4C, nullptr);
    reset_load();
    expect(wave->reload() == 8);
    setp(0x4C, name_buf);
    setp(0x3C, nullptr);
    guard = 0;
    reset_load();
    expect(wave->reload() == 1);
    guard = 1;
    reset_load();
    g_wave_create_ret = 0x22;
    expect(wave->reload() == 0x22);
    expect(g_wave_create_calls == 1 && g_wave_fam_calls == 0);
    expect(g_wave_create_mode == 1);

    // A failing device reload propagates before the loaded-bit protocol.
    setp(0x3C, &fake_dev);
    obj[0x54] = 1;
    set32(0x40, 0xF0);
    set32(0x30, 7);
    reset_load();
    g_wave_own58_ret = 5;
    g_wave_fam_ret = 0x66;
    expect(wave->reload() == 0x66);
    expect(g_wave_fam_arg_log == std::vector<uint32_t>({3}));
    expect(g_wave_own7C_calls == 0 && get32(0x40) == 0xF0);

    // First success: the loaded bit is set before the wave's own 0x7C runs,
    // and the nonzero dword at 0x30 starts the device looping with 1.
    reset_load();
    g_wave_own58_ret = 5;
    g_wave_fam_ret = 0;
    std::memcpy(expected.data(), storage.data(), storage.size());
    expect(wave->reload() == 0);
    expect(get32(0x40) == 0xF1);
    expect(g_wave_own7C_calls == 1 && g_wave_own7C_seen_flags == 0xF1);
    expect(g_wave_fam_arg_log == std::vector<uint32_t>({3, 1}));
    {
        const uint32_t loaded_word = 0xF1;
        std::memcpy(expected.data() + 16 + 0x40, &loaded_word, 4);
    }
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // Already loaded: no bit work, no callback, no looping.
    reset_load();
    g_wave_own58_ret = 5;
    g_wave_fam_ret = 0;
    expect(wave->reload() == 0);
    expect(g_wave_own7C_calls == 0);
    expect(g_wave_fam_arg_log == std::vector<uint32_t>({3}));

    // A zero dword at 0x30 skips the looping call; with the 0x54 flag byte
    // cleared too, the attribute mask is exactly zero.
    set32(0x40, 0);
    set32(0x30, 0);
    obj[0x54] = 0;
    reset_load();
    expect(wave->reload() == 0);
    expect(g_wave_own7C_calls == 1);
    expect(g_wave_fam_arg_log == std::vector<uint32_t>({0}));

    // --- dyna_load(): in-memory creation ---
    char data_buf[] = "DATA";
    reset_load();
    expect(wave->dyna_load(data_buf) == 0xC);  // a device already exists
    expect(g_wave_create_calls == 0);
    setp(0x3C, nullptr);
    guard = 0;
    reset_load();
    expect(wave->dyna_load(data_buf) == 1);
    guard = 1;
    reset_load();
    g_wave_create_ret = 0x44;
    expect(wave->dyna_load(data_buf) == 0x44);
    expect(g_wave_create_name == reinterpret_cast<char *>(data_buf));
    expect(g_wave_create_mode == 1);
    expect(g_wave_own70_calls == 0);
    // Success - and the 0x70 query swaps the device's vtable for a poisoned
    // one, so a capture taken after the query would dispatch into nulls.
    void *poison_vtbl[64];
    std::memset(poison_vtbl, 0, sizeof(poison_vtbl));
    reset_load();
    g_wave_create_installs = &fake_dev;
    g_wave_own70_ret = 0x155;
    g_wave_own70_swaps_vtbl = poison_vtbl;
    expect(wave->dyna_load(data_buf) == 0);
    g_wave_own70_swaps_vtbl = nullptr;
    fake_dev.vtbl = dev_vtable;
    expect(getp(0x3C) == &fake_dev);
    expect(g_wave_own70_calls == 1);
    expect(g_wave_fam_arg_log == std::vector<uint32_t>({0x155}));
    expect(g_wave_own7C_calls == 1);

    // --- load(const char *): the replay tail ---
    std::memset(dev_vtable, 0, sizeof(dev_vtable));
    dev_vtable[0x6C / 4] = reinterpret_cast<void *>(&observe_wave_dev1);
    dev_vtable[0xC4 / 4] = reinterpret_cast<void *>(&observe_wave_dev0);
    dev_vtable[0x40 / 4] = reinterpret_cast<void *>(&observe_wave_dev1);
    dev_vtable[0x98 / 4] = reinterpret_cast<void *>(&observe_wave_dev1);
    dev_vtable[0x44 / 4] = reinterpret_cast<void *>(&observe_wave_dev1);
    char name2_buf[] = "y.wav";

    setp(0x3C, nullptr);
    guard = 0;
    reset_load();
    expect(wave->load(name2_buf) == 1);
    guard = 1;
    reset_load();
    g_wave_create_ret = 0x29;
    expect(wave->load(name2_buf) == 0x29);
    expect(g_wave_create_name == reinterpret_cast<char *>(name2_buf));
    expect(g_wave_create_mode == 1);

    // The caller's name - not the remembered one - reaches Sound::load; a
    // failing load stops before the replay tail.
    setp(0x3C, &fake_dev);
    obj[0x54] = 0x19;
    reset_load();
    g_wave_sload_ret = 0x77;
    expect(wave->load(name2_buf) == 0x77);
    expect(g_wave_fam_arg_log == std::vector<uint32_t>({0xC1}));
    expect(g_wave_sload_name == reinterpret_cast<char *>(name2_buf));

    // Success: length lands, then volume, pitch, and the 0x08 dword replay
    // through the device, in that order.
    obj[0x54] = 0;
    set32(0x04, 33);
    set32(0x58, 0x111);
    set32(0x08, 0x222);
    reset_load();
    g_wave_own58_ret = 9;
    g_wave_fam_ret = 0x555;
    std::memcpy(expected.data(), storage.data(), storage.size());
    expect(wave->load(name2_buf) == 0);
    expect(get32(0x60) == 0x555u);
    expect(g_wave_fam_arg_log ==
           std::vector<uint32_t>({2, 33, 0x111, 0x222}));
    {
        const uint32_t len = 0x555;
        std::memcpy(expected.data() + 16 + 0x60, &len, 4);
    }
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // Redirect entries.
    setp(0x4C, nullptr);
    expect(wave_load_empty_redirect(wave, nullptr) == 8);
    expect(wave_reload_redirect(wave, nullptr) == 8);
    expect(wave_dyna_load_redirect(wave, nullptr, data_buf) == 0xC);
    setp(0x3C, nullptr);
    guard = 0;
    expect(wave_load_fname_redirect(wave, nullptr, name2_buf) == 1);

    WaveDeviceCreateSlot = saved_create;
    WaveDeviceReleaseGuard = saved_guard;
    SoundOriginalLoad = saved_sload;
}

void test_wave_init() {
    std::vector<uint8_t> storage(sizeof(Wave) + 32, 0);
    std::vector<uint8_t> expected(storage.size());
    uint8_t *const obj = storage.data() + 16;
    auto *wave = reinterpret_cast<Wave *>(obj);
    g_wave_dtor_obj = obj;

    auto *const saved_delete = WaveOperatorDelete;
    auto *const saved_new = WaveOperatorNew;
    auto **const saved_create = WaveDeviceCreateSlot;
    int *const saved_guard = WaveDeviceReleaseGuard;
    func_wave_device_create *create_fn = &observe_wave_device_create;
    int guard = 1;
    WaveOperatorDelete = &observe_wave_operator_delete;
    WaveOperatorNew = &observe_wave_operator_new;
    WaveDeviceCreateSlot = &create_fn;
    WaveDeviceReleaseGuard = &guard;

    void *dev_vtable[64];
    std::memset(dev_vtable, 0, sizeof(dev_vtable));
    dev_vtable[0x6C / 4] = reinterpret_cast<void *>(&observe_wave_dev1);
    struct FakeDev { void *vtbl; } fake_dev;
    fake_dev.vtbl = dev_vtable;
    void *own_vtable[64];
    std::memset(own_vtable, 0, sizeof(own_vtable));
    own_vtable[0x48 / 4] = reinterpret_cast<void *>(&observe_wave_own_slot48);
    own_vtable[0x70 / 4] = reinterpret_cast<void *>(&observe_wave_own_slot70);
    own_vtable[0x7C / 4] = reinterpret_cast<void *>(&observe_wave_own_slot7C);

    auto set32 = [&](size_t off, uint32_t v) { std::memcpy(obj + off, &v, 4); };
    auto get32 = [&](size_t off) {
        uint32_t v;
        std::memcpy(&v, obj + off, 4);
        return v;
    };
    auto setp = [&](size_t off, const void *p) {
        std::memcpy(obj + off, &p, 4);
    };
    auto getp = [&](size_t off) {
        void *p;
        std::memcpy(&p, obj + off, 4);
        return p;
    };
    auto reset_init = [&] {
        filefind_get_calls = 0;
        g_wave_new_calls = g_wave_create_calls = 0;
        g_wave_own48_calls = g_wave_own70_calls = g_wave_own7C_calls = 0;
        g_wave_fam_calls = 0;
        g_wave_fam_arg_log.clear();
        g_wave_dtor_deletes.clear();
        g_wave_dtor_delete_seen_slot.clear();
        g_wave_create_installs = nullptr;
        g_wave_create_ret = 0;
        g_wave_own70_ret = 0;
    };
    char name_arg[] = "menu.wav";
    char resolved_buf[] = "snd/menu.wav";

    seed_storage(storage.data(), expected.data(), storage.size());
    setp(0x00, own_vtable);

    // Streaming refuses the bit-4 and bit-7 modes before resolving anything.
    reset_init();
    std::memcpy(expected.data(), storage.data(), storage.size());
    wave->init(name_arg, 4 | 0x10);
    wave->init(name_arg, 4 | 0x80);
    expect(filefind_get_calls == 0);
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // An unresolvable name changes nothing - the old filename is kept.
    reset_init();
    filefind_get_result = nullptr;
    wave->init(name_arg, 1);
    expect(filefind_get_calls == 1 &&
           filefind_get_request == static_cast<LPCSTR>(name_arg));
    expect(g_wave_dtor_deletes.empty() && g_wave_new_calls == 0);
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // Non-streaming, every folding bit at once: the old name goes back to
    // the game heap, the resolved path is copied in, the flag dword is
    // rebuilt from zero, and mode bit 1 runs the wave's own slot 0x48 with
    // the first two flag bits already placed and the high ones not yet.
    char old_name[4] = {'x', 0, 0, 0};
    setp(0x4C, old_name);
    setp(0x3C, nullptr);
    reset_init();
    filefind_get_result = resolved_buf;
    std::memcpy(expected.data(), storage.data(), storage.size());
    wave->init(name_arg, 0x1D3);
    expect(g_wave_dtor_deletes.size() == 1 &&
           g_wave_dtor_deletes[0] == old_name);
    expect(g_wave_new_calls == 1 && g_wave_new_size == 13);
    expect(getp(0x4C) == g_wave_new_arena);
    expect(std::strcmp(g_wave_new_arena, "snd/menu.wav") == 0);
    expect(g_wave_create_calls == 0 && g_wave_fam_calls == 0);
    expect(g_wave_own48_calls == 1 && g_wave_own48_self == wave &&
           g_wave_own48_arg == 1);
    expect(g_wave_own48_seen_flags == 5);  // bits 0 and 2 placed, rest later
    expect(get32(0x54) == 0x3D);
    {
        void *arena = g_wave_new_arena;
        std::memcpy(expected.data() + 16 + 0x4C, &arena, 4);
        const uint32_t flags_word = 0x3D;
        std::memcpy(expected.data() + 16 + 0x54, &flags_word, 4);
    }
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // Streaming with no device: creation from the RESOLVED path, the
    // captured-vtable attribute round (the 0x70 query swaps the table for a
    // poisoned one to prove the capture came first), the wave's own 0x7C,
    // then the raw mode mask to the live device.
    void *poison_vtbl2[64];
    std::memset(poison_vtbl2, 0, sizeof(poison_vtbl2));
    setp(0x3C, nullptr);
    setp(0x4C, nullptr);
    reset_init();
    g_wave_create_installs = &fake_dev;
    g_wave_own70_ret = 0x2AA;
    g_wave_own70_swaps_vtbl = poison_vtbl2;
    g_wave_own7C_restores_vtbl = dev_vtable;
    wave->init(name_arg, 4);
    g_wave_own70_swaps_vtbl = nullptr;
    g_wave_own7C_restores_vtbl = nullptr;
    fake_dev.vtbl = dev_vtable;
    expect(g_wave_create_calls == 1);
    expect(g_wave_create_name == static_cast<LPCSTR>(resolved_buf));
    expect(g_wave_create_mode == 1);
    expect(g_wave_own70_calls == 1 && g_wave_own7C_calls == 1);
    expect(g_wave_fam_arg_log == std::vector<uint32_t>({0x2AA, 4}));
    expect(get32(0x54) == 2);
    expect(g_wave_own48_calls == 0);

    // Streaming with a device already wrapped: no creation, just the raw
    // mode mask; mode bit 0 folds alongside the streaming bit.
    reset_init();
    wave->init(name_arg, 5);
    expect(g_wave_create_calls == 0);
    expect(g_wave_fam_arg_log == std::vector<uint32_t>({5}));
    expect(get32(0x54) == 3);

    // Streaming with a dead hook: no creation, no device call, and a failing
    // creation skips the attribute round but still folds the tail bits.
    setp(0x3C, nullptr);
    guard = 0;
    reset_init();
    wave->init(name_arg, 4);
    expect(g_wave_create_calls == 0 && g_wave_fam_calls == 0);
    expect(get32(0x54) == 2);
    guard = 1;
    reset_init();
    g_wave_create_ret = 0x31;
    wave->init(name_arg, 4 | 1);
    expect(g_wave_create_calls == 1);
    expect(g_wave_own70_calls == 0 && g_wave_fam_calls == 0);
    expect(get32(0x54) == 3);

    // Non-streaming keeps its bit-4 mapping (streaming suppressed it above).
    reset_init();
    wave->init(name_arg, 0x10);
    expect(get32(0x54) == 4);

    // Redirect entry.
    reset_init();
    filefind_get_result = nullptr;
    wave_init_redirect(wave, nullptr, name_arg, 1);
    expect(filefind_get_calls == 1);

    WaveOperatorDelete = saved_delete;
    WaveOperatorNew = saved_new;
    WaveDeviceCreateSlot = saved_create;
    WaveDeviceReleaseGuard = saved_guard;
}

void test_wave_device_forwarder_family() {
    std::vector<uint8_t> storage(sizeof(Wave_Device) + 32, 0);
    std::vector<uint8_t> expected(storage.size());
    uint8_t *const obj = storage.data() + 16;
    auto *device = reinterpret_cast<Wave_Device *>(obj);

    void *dev_vtable[64];
    struct FakeDev { void *vtbl; } fake_dev;
    fake_dev.vtbl = dev_vtable;
    auto arm = [&](size_t slot, void *fn) {
        std::memset(dev_vtable, 0, sizeof(dev_vtable));
        dev_vtable[slot / 4] = fn;
        g_wave_fam_calls = 0;
        g_wave_fam_arg_log.clear();
    };
    auto setp = [&](size_t off, const void *ptr) {
        std::memcpy(obj + off, &ptr, 4);
    };
    auto bits_of = [](float f) {
        uint32_t b;
        std::memcpy(&b, &f, 4);
        return b;
    };
    auto ptr_bits = [](const void *p) {
        return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(p));
    };

    seed_storage(storage.data(), expected.data(), storage.size());
    setp(0x14, &fake_dev);
    std::memcpy(expected.data(), storage.data(), storage.size());

    // Device paths: verbatim answers, per-slot dispatch, argument dwords.
    char raw_name[] = "dump.raw";
    arm(0x50, reinterpret_cast<void *>(&observe_wave_dev1));
    g_wave_fam_ret = 0x2101;
    expect(device->start_raw_dump(raw_name) == 0x2101);
    expect(g_wave_fam_calls == 1 && g_wave_fam_self == &fake_dev &&
           g_wave_fam_args[0] == ptr_bits(raw_name));

    auto *eax_props = reinterpret_cast<EAX_REVERB_PROPERTIES *>(raw_name);
    arm(0x7C, reinterpret_cast<void *>(&observe_wave_dev1));
    g_wave_fam_ret = 0x2102;
    expect(device->set_eax(eax_props) == 0x2102);
    expect(g_wave_fam_args[0] == ptr_bits(raw_name));

    arm(0x78, reinterpret_cast<void *>(&observe_wave_dev1));
    g_wave_fam_ret = 0x2103;
    expect(device->set_eax(static_cast<unsigned long>(0xE1)) == 0x2103);
    expect(g_wave_fam_args[0] == 0xE1);

    arm(0x80, reinterpret_cast<void *>(&observe_wave_dev1));
    g_wave_fam_ret = 0x2104;
    expect(device->set_eax_mix(0.25f) == 0x2104);
    expect(g_wave_fam_args[0] == bits_of(0.25f));

    arm(0x88, reinterpret_cast<void *>(&observe_wave_dev3));
    g_wave_fam_ret = 0x2105;
    expect(device->set_listener_position(1.5f, -2.5f, 3.5f) == 0x2105);
    expect(g_wave_fam_args[0] == bits_of(1.5f) &&
           g_wave_fam_args[1] == bits_of(-2.5f) &&
           g_wave_fam_args[2] == bits_of(3.5f));

    float fx = 0, fy = 0, fz = 0;
    arm(0x8C, reinterpret_cast<void *>(&observe_wave_dev3));
    g_wave_fam_ret = 0x2106;
    expect(device->get_listener_position(&fx, &fy, &fz) == 0x2106);
    expect(g_wave_fam_args[0] == ptr_bits(&fx) &&
           g_wave_fam_args[1] == ptr_bits(&fy) &&
           g_wave_fam_args[2] == ptr_bits(&fz));

    const struct {
        size_t slot;
        bool setter;
    } xyz[] = {
        {0x90, true}, {0x94, false}, {0x98, true},
        {0x9C, false}, {0xA0, true}, {0xA4, false},
    };
    int tag = 0x2110;
    for (const auto &c : xyz) {
        arm(c.slot, reinterpret_cast<void *>(&observe_wave_dev1));
        g_wave_fam_ret = tag;
        int result;
        float value = 4.5f + tag;
        switch (c.slot) {
        case 0x90: result = device->set_listener_xpos(value); break;
        case 0x94: result = device->get_listener_xpos(&fx); break;
        case 0x98: result = device->set_listener_ypos(value); break;
        case 0x9C: result = device->get_listener_ypos(&fy); break;
        case 0xA0: result = device->set_listener_zpos(value); break;
        default: result = device->get_listener_zpos(&fz); break;
        }
        expect(result == tag);
        expect(g_wave_fam_calls == 1);
        if (c.setter) {
            expect(g_wave_fam_args[0] == bits_of(value));
        }
        ++tag;
    }

    // get_description: void, three arguments through slot 0x1C.
    char descr[8] = {};
    arm(0x1C, reinterpret_cast<void *>(&observe_wave_dev3));
    device->get_description(2, descr, 7);
    expect(g_wave_fam_calls == 1);
    expect(g_wave_fam_args[0] == 2 && g_wave_fam_args[1] == ptr_bits(descr) &&
           g_wave_fam_args[2] == 7);

    // Nothing above wrote the object.
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // No-device defaults: 0x14 everywhere except the raw dump's 3 and
    // get_description's silence.
    setp(0x14, nullptr);
    std::memcpy(expected.data(), storage.data(), storage.size());
    g_wave_fam_calls = 0;
    expect(device->start_raw_dump(raw_name) == 3);
    expect(device->set_eax(eax_props) == 0x14);
    expect(device->set_eax(static_cast<unsigned long>(1)) == 0x14);
    expect(device->set_eax_mix(1.0f) == 0x14);
    expect(device->set_listener_position(1, 2, 3) == 0x14);
    expect(device->get_listener_position(&fx, &fy, &fz) == 0x14);
    expect(device->set_listener_xpos(1) == 0x14);
    expect(device->get_listener_xpos(&fx) == 0x14);
    expect(device->set_listener_ypos(1) == 0x14);
    expect(device->get_listener_ypos(&fy) == 0x14);
    expect(device->set_listener_zpos(1) == 0x14);
    expect(device->get_listener_zpos(&fz) == 0x14);
    device->get_description(1, descr, 1);
    expect(g_wave_fam_calls == 0);
    expect(descr[0] == 0);
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // Redirect entries, one per export.
    expect(wave_device_start_raw_dump_redirect(device, nullptr, raw_name) ==
           3);
    expect(wave_device_set_eax_props_redirect(device, nullptr, eax_props) ==
           0x14);
    expect(wave_device_set_eax_redirect(device, nullptr, 1) == 0x14);
    expect(wave_device_set_eax_mix_redirect(device, nullptr, 1) == 0x14);
    expect(wave_device_set_listener_position_redirect(device, nullptr, 1, 2,
                                                      3) == 0x14);
    expect(wave_device_get_listener_position_redirect(device, nullptr, &fx,
                                                      &fy, &fz) == 0x14);
    expect(wave_device_set_listener_xpos_redirect(device, nullptr, 1) ==
           0x14);
    expect(wave_device_get_listener_xpos_redirect(device, nullptr, &fx) ==
           0x14);
    expect(wave_device_set_listener_ypos_redirect(device, nullptr, 1) ==
           0x14);
    expect(wave_device_get_listener_ypos_redirect(device, nullptr, &fy) ==
           0x14);
    expect(wave_device_set_listener_zpos_redirect(device, nullptr, 1) ==
           0x14);
    expect(wave_device_get_listener_zpos_redirect(device, nullptr, &fz) ==
           0x14);
    wave_device_get_description_redirect(device, nullptr, 1, descr, 1);
}

void test_wave_device_group_admin() {
    std::vector<uint8_t> storage(sizeof(Wave_Device) + 32, 0);
    std::vector<uint8_t> expected(storage.size());
    uint8_t *const obj = storage.data() + 16;
    auto *device = reinterpret_cast<Wave_Device *>(obj);

    void *wave_vtable[64];
    std::memset(wave_vtable, 0, sizeof(wave_vtable));
    wave_vtable[0x40 / 4] =
        reinterpret_cast<void *>(&observe_wave_group_volume);
    wave_vtable[0x8C / 4] =
        reinterpret_cast<void *>(&observe_wave_group_resume);
    wave_vtable[0x14 / 4] = reinterpret_cast<void *>(&observe_wave_group_halt);
    auto make_wave = [&](uint8_t *w, uint32_t volume) {
        std::memset(w, 0, 0x6C);
        void *vt = wave_vtable;
        std::memcpy(w, &vt, 4);
        std::memcpy(w + 4, &volume, 4);
        return reinterpret_cast<Wave *>(w);
    };
    uint8_t wb1[0x6C], wb2[0x6C], wb3[0x6C], wb4[0x6C];
    Wave *w1 = make_wave(wb1, 7);
    Wave *w2 = make_wave(wb2, 9);
    Wave *w3 = make_wave(wb3, 11);
    Wave *w4 = make_wave(wb4, 13);

    auto group_base = [](unsigned g) { return 0x24 + g * 0x18; };
    auto gset32 = [&](unsigned g, size_t off, uint32_t v) {
        std::memcpy(obj + group_base(g) + off, &v, 4);
    };
    auto gget32 = [&](unsigned g, size_t off) {
        uint32_t v;
        std::memcpy(&v, obj + group_base(g) + off, 4);
        return v;
    };
    auto gsetp = [&](unsigned g, size_t off, const void *ptr) {
        std::memcpy(obj + group_base(g) + off, &ptr, 4);
    };
    auto ggetp = [&](unsigned g, size_t off) {
        void *ptr;
        std::memcpy(&ptr, obj + group_base(g) + off, 4);
        return ptr;
    };
    auto reset_walk = [&] {
        g_wave_volume_log.clear();
        g_wave_volume_args.clear();
        g_wave_visit_seen_volume.clear();
        g_wave_resume_log.clear();
        g_wave_halt_log.clear();
        g_wave_visit_seen_enabled.clear();
        g_wave_visit_cursor_write = nullptr;
        g_wave_visit_total = 0;
    };

    seed_storage(storage.data(), expected.data(), storage.size());
    g_wave_visit_group_base = obj + group_base(3);

    // set_group_volume: guards leave everything alone.
    reset_walk();
    std::memcpy(expected.data(), storage.data(), storage.size());
    expect(device->set_group_volume(0x10, 5) == 0xA);
    expect(device->set_group_volume(0, 0x80) == 0xA);
    expect(g_wave_visit_total == 0);
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // The empty boundary group: volume stored, cursor cleared, no visits.
    g_wave_visit_group_base = obj + group_base(0xF);
    gsetp(0xF, 8, nullptr);
    gsetp(0xF, 0x10, reinterpret_cast<void *>(0x1234));
    reset_walk();
    expect(device->set_group_volume(0xF, 0x7F) == 0);
    expect(gget32(0xF, 4) == 0x7F);
    expect(ggetp(0xF, 0x10) == nullptr);
    expect(g_wave_visit_total == 0);

    // Two members: each replays its OWN stored volume, the cursor walks off
    // the end.
    g_wave_visit_group_base = obj + group_base(3);
    WaveGroupNode n1{}, n2{}, n3{}, n4{};
    n1.next = &n2;
    n1.wave = w1;
    n2.prev = &n1;
    n2.wave = w2;
    gsetp(3, 8, &n1);
    reset_walk();
    expect(device->set_group_volume(3, 0x50) == 0);
    expect(gget32(3, 4) == 0x50);
    expect(g_wave_volume_log == std::vector<Wave *>({w1, w2}));
    expect(g_wave_volume_args == std::vector<uint32_t>({7, 9}));
    expect(g_wave_visit_seen_volume == std::vector<uint32_t>({0x50, 0x50}));
    expect(ggetp(3, 0x10) == nullptr);

    // A null wave stops the walk with the cursor parked on its node.
    n1 = WaveGroupNode{};
    n2 = WaveGroupNode{};
    n1.next = &n2;
    n1.wave = w1;
    n2.prev = &n1;
    n2.wave = nullptr;
    gsetp(3, 8, &n1);
    reset_walk();
    expect(device->set_group_volume(3, 0x20) == 0);
    expect(g_wave_volume_log == std::vector<Wave *>({w1}));
    expect(ggetp(3, 0x10) == &n2);

    // A handler that rewrites the cursor mid-walk: the walk resumes from the
    // rewritten node's FOLLOWER - the original's self-removal skip rule.
    n1 = WaveGroupNode{};
    n2 = WaveGroupNode{};
    n3 = WaveGroupNode{};
    n4 = WaveGroupNode{};
    n1.next = &n2;
    n1.wave = w1;
    n2.prev = &n1;
    n2.next = &n3;
    n2.wave = w2;
    n3.prev = &n2;
    n3.next = &n4;
    n3.wave = w3;
    n4.prev = &n3;
    n4.wave = w4;
    gsetp(3, 8, &n1);
    reset_walk();
    g_wave_visit_cursor_write = &n3;
    expect(device->set_group_volume(3, 0x30) == 0);
    expect(g_wave_volume_log == std::vector<Wave *>({w1, w4}));

    // enable_group: an enabled group is untouched, byte and all; a disabled
    // one resumes every member BEFORE the byte flips to exactly 1.
    reset_walk();
    expect(device->enable_group(0x10) == 0xA);
    obj[group_base(3)] = 5;
    expect(device->enable_group(3) == 0);
    expect(g_wave_resume_log.empty());
    expect(obj[group_base(3)] == 5);
    obj[group_base(3)] = 0;
    reset_walk();
    expect(device->enable_group(3) == 0);
    expect(g_wave_resume_log == std::vector<Wave *>({w1, w2, w3, w4}));
    expect(g_wave_visit_seen_enabled ==
           std::vector<uint8_t>({0, 0, 0, 0}));
    expect(obj[group_base(3)] == 1);

    // Slot 15 is the last accepted group for both togglers.
    obj[group_base(0xF)] = 0;
    gsetp(0xF, 8, nullptr);
    reset_walk();
    expect(device->enable_group(0xF) == 0);
    expect(obj[group_base(0xF)] == 1);
    expect(device->disable_group(0xF) == 0);
    expect(obj[group_base(0xF)] == 0);

    // disable_group mirrors it with the halt slot.
    reset_walk();
    expect(device->disable_group(0x10) == 0xA);
    obj[group_base(3)] = 0;
    expect(device->disable_group(3) == 0);
    expect(g_wave_halt_log.empty());
    obj[group_base(3)] = 7;
    reset_walk();
    expect(device->disable_group(3) == 0);
    expect(g_wave_halt_log == std::vector<Wave *>({w1, w2, w3, w4}));
    expect(g_wave_visit_seen_enabled ==
           std::vector<uint8_t>({7, 7, 7, 7}));
    expect(obj[group_base(3)] == 0);

    // Redirect entries.
    expect(wave_device_set_group_volume_redirect(device, nullptr, 0x10, 1) ==
           0xA);
    expect(wave_device_enable_group_redirect(device, nullptr, 0x10) == 0xA);
    expect(wave_device_disable_group_redirect(device, nullptr, 0x10) == 0xA);
    g_wave_visit_group_base = nullptr;
}

void test_wave_device_lifecycle() {
    std::vector<uint8_t> storage(sizeof(Wave_Device) + 32, 0);
    std::vector<uint8_t> expected(storage.size());
    uint8_t *const obj = storage.data() + 16;
    auto *device = reinterpret_cast<Wave_Device *>(obj);
    g_wdev_down_obj = obj;

    auto **const saved_factory = WaveDeviceFactorySlot;
    auto **const saved_destroy = WaveDeviceDestroySlot;
    func_wave_device_factory *factory_fn = &observe_wdev_factory;
    func_wave_device_destroy *destroy_fn = &observe_wdev_destroy;
    WaveDeviceFactorySlot = &factory_fn;
    WaveDeviceDestroySlot = &destroy_fn;

    void *dev_vtable[64];
    std::memset(dev_vtable, 0, sizeof(dev_vtable));
    dev_vtable[0xC / 4] = reinterpret_cast<void *>(&observe_wave_dev2);
    dev_vtable[0x10 / 4] =
        reinterpret_cast<void *>(&observe_wdev_device_down);
    struct FakeDev { void *vtbl; } fake_dev;
    fake_dev.vtbl = dev_vtable;
    void *own_vtable[64];
    std::memset(own_vtable, 0, sizeof(own_vtable));
    own_vtable[0] = reinterpret_cast<void *>(&observe_wdev_own_slot0);
    own_vtable[1] = reinterpret_cast<void *>(&observe_wdev_own_slot4);

    auto setp = [&](size_t off, const void *ptr) {
        std::memcpy(obj + off, &ptr, 4);
    };
    auto getp = [&](size_t off) {
        void *ptr;
        std::memcpy(&ptr, obj + off, 4);
        return ptr;
    };
    auto reset_life = [&] {
        g_wdev_factory_calls = g_wdev_destroy_calls = 0;
        g_wdev_own0_calls = g_wdev_own4_calls = 0;
        g_wdev_down_calls = 0;
        g_wave_fam_calls = 0;
        g_wdev_factory_installs = nullptr;
        g_wdev_factory_ret = 0;
        g_wdev_own0_ret = 0;
        g_wdev_down_clears = false;
    };

    seed_storage(storage.data(), expected.data(), storage.size());
    setp(0x00, own_vtable);

    // create_device: an existing device refuses before the factory; a dead
    // factory answers 0x14; otherwise the factory's answer stands and the
    // device lands in the field.
    setp(0x14, &fake_dev);
    reset_life();
    expect(device->create_device(3) == 0xC);
    expect(g_wdev_factory_calls == 0);
    setp(0x14, nullptr);
    factory_fn = nullptr;
    expect(device->create_device(3) == 0x14);
    factory_fn = &observe_wdev_factory;
    reset_life();
    g_wdev_factory_installs = &fake_dev;
    g_wdev_factory_ret = 0x2A;
    expect(device->create_device(7) == 0x2A);
    expect(g_wdev_factory_calls == 1 &&
           g_wdev_factory_slot_arg ==
               reinterpret_cast<void **>(obj + 0x14) &&
           g_wdev_factory_kind == 7);
    expect(getp(0x14) == &fake_dev);

    // delete_device: needs both a device and a hook; then the hook runs and
    // the field clears.
    reset_life();
    destroy_fn = nullptr;
    expect(device->delete_device() == 0x14);
    expect(getp(0x14) == &fake_dev);  // kept without a hook
    destroy_fn = &observe_wdev_destroy;
    expect(device->delete_device() == 0);
    expect(g_wdev_destroy_calls == 1 && getp(0x14) == nullptr);
    expect(g_wdev_destroy_seen_device == &fake_dev);  // hook before the clear
    expect(device->delete_device() == 0x14);  // nothing left
    expect(g_wdev_destroy_calls == 1);

    // init: the device stack in order - own slot 0 gates, the device's slot
    // 0xC does the work, own slot 4 only cleans up a failure.
    setp(0x14, &fake_dev);
    reset_life();
    g_wdev_own0_ret = 0x31;
    expect(device->init(reinterpret_cast<void *>(0x1234), 9) == 0x31);
    expect(g_wdev_own0_calls == 1 && g_wdev_own0_mode == 9);
    expect(g_wave_fam_calls == 0 && g_wdev_own4_calls == 0);
    reset_life();
    g_wave_fam_ret = 0;
    expect(device->init(reinterpret_cast<void *>(0x1234), 5) == 0);
    expect(g_wdev_own0_calls == 1 && g_wave_fam_calls == 1);
    expect(g_wave_fam_args[0] == 0x1234 && g_wave_fam_args[1] == 5);
    expect(g_wdev_own4_calls == 0);
    reset_life();
    g_wave_fam_ret = 0x55;
    expect(device->init(nullptr, 2) == 0x55);
    expect(g_wdev_own4_calls == 1);

    // release: slot 0x10 winds the device down; the destroy hook only runs
    // if the callback left the device in place; the field always clears.
    reset_life();
    expect(getp(0x14) == &fake_dev);
    device->release();
    expect(g_wdev_down_calls == 1);
    expect(g_wdev_destroy_calls == 1);
    expect(getp(0x14) == nullptr);
    device->release();  // no device: silence
    expect(g_wdev_down_calls == 1);
    // A callback that clears the device suppresses the hook.
    setp(0x14, &fake_dev);
    reset_life();
    g_wdev_down_clears = true;
    device->release();
    expect(g_wdev_down_calls == 1 && g_wdev_destroy_calls == 0);
    expect(getp(0x14) == nullptr);
    // A dead hook is skipped but the teardown still happens.
    setp(0x14, &fake_dev);
    reset_life();
    destroy_fn = nullptr;
    device->release();
    expect(g_wdev_down_calls == 1 && getp(0x14) == nullptr);
    destroy_fn = &observe_wdev_destroy;

    // Redirect entries.
    setp(0x14, nullptr);
    expect(wave_device_delete_device_redirect(device, nullptr) == 0x14);
    g_wdev_factory_installs = &fake_dev;
    g_wdev_factory_ret = 0;
    expect(wave_device_create_device_redirect(device, nullptr, 1) == 0);
    reset_life();
    g_wdev_own0_ret = 1;
    expect(wave_device_init_redirect(device, nullptr, nullptr, 1) == 1);
    wave_device_release_redirect(device, nullptr);
    expect(getp(0x14) == nullptr);

    WaveDeviceFactorySlot = saved_factory;
    WaveDeviceDestroySlot = saved_destroy;
}

void test_sound_chain_and_dtor() {
    std::vector<uint8_t> storage(0xA0 + 32, 0);
    std::vector<uint8_t> expected(storage.size());
    uint8_t *const obj = storage.data() + 16;
    auto *sound = reinterpret_cast<Sound *>(obj);
    g_wave_dtor_obj = obj;

    auto *const saved_delete = WaveOperatorDelete;
    auto **const saved_release_slot = WaveDeviceReleaseSlot;
    int *const saved_guard = WaveDeviceReleaseGuard;
    Wave **const saved_head = WaveChainHead;
    Wave **const saved_tail = WaveChainTail;
    WaveOperatorDelete = &observe_wave_operator_delete;
    func_wave_device_release *release_fn = &observe_wave_release;
    WaveDeviceReleaseSlot = &release_fn;
    int guard = 1;
    WaveDeviceReleaseGuard = &guard;
    Wave *chain_head = nullptr;
    Wave *chain_tail = nullptr;
    WaveChainHead = &chain_head;
    WaveChainTail = &chain_tail;

    auto set32 = [&](size_t off, uint32_t v) { std::memcpy(obj + off, &v, 4); };
    auto get32 = [&](size_t off) {
        uint32_t v;
        std::memcpy(&v, obj + off, 4);
        return v;
    };
    auto setp = [&](size_t off, const void *ptr) {
        std::memcpy(obj + off, &ptr, 4);
    };
    auto getp = [&](size_t off) {
        void *ptr;
        std::memcpy(&ptr, obj + off, 4);
        return ptr;
    };

    // --- attach ---
    seed_storage(storage.data(), expected.data(), storage.size());
    // Either neighbour set: left entirely alone.
    setp(0x44, nullptr);
    setp(0x48, obj);
    chain_head = reinterpret_cast<Wave *>(0x1111);
    chain_tail = reinterpret_cast<Wave *>(0x2222);
    expect(sound->attach() == 0);
    expect(chain_tail == reinterpret_cast<Wave *>(0x2222));
    setp(0x48, nullptr);
    setp(0x44, obj);
    expect(sound->attach() == 0);
    expect(chain_tail == reinterpret_cast<Wave *>(0x2222));

    // An empty chain: the sound becomes both ends, bit 2 sets.
    setp(0x44, nullptr);
    setp(0x48, nullptr);
    set32(0x40, 0xF1);
    chain_head = nullptr;
    chain_tail = nullptr;
    expect(sound->attach() == 0);
    expect(chain_head == reinterpret_cast<Wave *>(sound));
    expect(chain_tail == reinterpret_cast<Wave *>(sound));
    expect(getp(0x44) == nullptr && getp(0x48) == nullptr);
    expect(get32(0x40) == 0xF3);

    // A second sound appends at the tail.
    uint8_t obj2[0xA0];
    std::memset(obj2, 0, sizeof(obj2));
    auto *sound2 = reinterpret_cast<Sound *>(obj2);
    expect(sound2->attach() == 0);
    expect(chain_head == reinterpret_cast<Wave *>(sound));
    expect(chain_tail == reinterpret_cast<Wave *>(sound2));
    expect(getp(0x48) == sound2);  // the old tail learned its next
    void *p2;
    std::memcpy(&p2, obj2 + 0x44, 4);
    expect(p2 == sound);
    uint32_t f2;
    std::memcpy(&f2, obj2 + 0x40, 4);
    expect(f2 == 2);

    // --- detach ---
    // An unchained sound does nothing at all.
    uint8_t obj3[0xA0];
    std::memset(obj3, 0, sizeof(obj3));
    auto *sound3 = reinterpret_cast<Sound *>(obj3);
    expect(sound3->detach() == 0);
    expect(chain_head == reinterpret_cast<Wave *>(sound));

    // The middle of three: neighbours relink around it.
    expect(sound3->attach() == 0);  // chain: sound, sound2, sound3
    expect(sound2->detach() == 0);
    expect(getp(0x48) == sound3);
    void *p3prev;
    std::memcpy(&p3prev, obj3 + 0x44, 4);
    expect(p3prev == sound);
    std::memcpy(&p2, obj2 + 0x44, 4);
    expect(p2 == nullptr);
    std::memcpy(&f2, obj2 + 0x40, 4);
    expect(f2 == 0);

    // The head: the head slot advances.
    expect(sound->detach() == 0);
    expect(chain_head == reinterpret_cast<Wave *>(sound3));
    expect(getp(0x44) == nullptr && getp(0x48) == nullptr);
    expect(get32(0x40) == 0xF1);

    // The tail (and only): both slots empty out.
    expect(sound3->detach() == 0);
    expect(chain_head == nullptr && chain_tail == nullptr);

    // --- ~Sound ---
    // Full teardown: filename freed and cleared, device through the guarded
    // hook then forgotten, the chain unlinked, the base vtable published.
    seed_storage(storage.data(), expected.data(), storage.size());
    uint8_t name_block[8];
    int fake_dev = 0;
    uint8_t prev_node[0xA0], next_node[0xA0];
    std::memset(prev_node, 0x66, sizeof(prev_node));
    std::memset(next_node, 0x77, sizeof(next_node));
    setp(0x4C, name_block);
    setp(0x3C, &fake_dev);
    set32(0x40, 0xA5A5A5F3u);
    setp(0x44, prev_node);
    setp(0x48, next_node);
    chain_head = reinterpret_cast<Wave *>(0x1111);
    chain_tail = reinterpret_cast<Wave *>(0x2222);
    guard = 1;
    g_wave_dtor_deletes.clear();
    g_wave_dtor_delete_seen_slot.clear();
    g_wave_dtor_release_calls = 0;
    g_wave_dtor_release_relinks = false;
    std::memcpy(expected.data(), storage.data(), storage.size());
    sound_dtor_redirect(sound, nullptr);
    expect(g_wave_dtor_deletes.size() == 1 &&
           g_wave_dtor_deletes[0] == name_block);
    expect(g_wave_dtor_delete_seen_slot[0] == name_block);
    expect(g_wave_dtor_release_calls == 1 &&
           g_wave_dtor_release_dev == &fake_dev);
    expect(g_wave_dtor_release_seen_vtable == 0x0066E3C0u);
    void *stored;
    std::memcpy(&stored, prev_node + 0x48, 4);
    expect(stored == next_node);
    std::memcpy(&stored, next_node + 0x44, 4);
    expect(stored == prev_node);
    expect(chain_head == reinterpret_cast<Wave *>(0x1111));
    expect(chain_tail == reinterpret_cast<Wave *>(0x2222));
    {
        uint8_t *const eobj = expected.data() + 16;
        const uint32_t vt = 0x0066E444u;
        const uint32_t zero = 0;
        const uint32_t flags = 0xA5A5A5F1u;
        std::memcpy(eobj + 0x00, &vt, 4);
        std::memcpy(eobj + 0x3C, &zero, 4);
        std::memcpy(eobj + 0x40, &flags, 4);
        std::memcpy(eobj + 0x44, &zero, 4);
        std::memcpy(eobj + 0x48, &zero, 4);
        std::memcpy(eobj + 0x4C, &zero, 4);
    }
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // The sparse path: no filename means the slot KEEPS its seed (unlike
    // Wave's unconditional clear), a dead guard skips the hook but the
    // device still clears, an unchained sound keeps its links.
    seed_storage(storage.data(), expected.data(), storage.size());
    setp(0x4C, nullptr);
    setp(0x3C, &fake_dev);
    set32(0x40, 0xA5A5A5F1u);
    guard = 0;
    g_wave_dtor_deletes.clear();
    g_wave_dtor_release_calls = 0;
    std::memcpy(expected.data(), storage.data(), storage.size());
    sound_dtor_redirect(sound, nullptr);
    expect(g_wave_dtor_deletes.empty());
    expect(g_wave_dtor_release_calls == 0);
    {
        uint8_t *const eobj = expected.data() + 16;
        const uint32_t vt = 0x0066E444u;
        const uint32_t zero = 0;
        std::memcpy(eobj + 0x00, &vt, 4);
        std::memcpy(eobj + 0x3C, &zero, 4);
    }
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // A chained sound with no neighbours: both end slots empty out - the
    // unlink's head and tail arms.
    seed_storage(storage.data(), expected.data(), storage.size());
    setp(0x4C, nullptr);
    setp(0x3C, nullptr);
    set32(0x40, 2);
    setp(0x44, nullptr);
    setp(0x48, nullptr);
    chain_head = reinterpret_cast<Wave *>(0x1111);
    chain_tail = reinterpret_cast<Wave *>(0x2222);
    sound_dtor_redirect(sound, nullptr);
    expect(chain_head == nullptr && chain_tail == nullptr);
    expect(get32(0x40) == 0);

    // The scalar deleting destructor: mode bit 0 frees the storage.
    set32(0x68, 0);
    set32(0x40, 0);
    setp(0x4C, nullptr);
    setp(0x3C, nullptr);
    g_wave_dtor_deletes.clear();
    set32(0x00, 0xDEADDEADu);  // a stale base vtable must not satisfy the canary
    expect(sound_scalar_dtor_redirect(sound, nullptr, 0) == sound);
    expect(g_wave_dtor_deletes.empty());
    expect(get32(0x00) == 0x0066E444u);  // the destructor really ran
    set32(0x40, 0);
    expect(sound_scalar_dtor_redirect(sound, nullptr, 3) == sound);
    expect(g_wave_dtor_deletes.size() == 1 &&
           g_wave_dtor_deletes[0] == sound);

    // Redirect entries.
    set32(0x40, 0);
    setp(0x44, nullptr);
    setp(0x48, nullptr);
    chain_head = nullptr;
    chain_tail = nullptr;
    expect(sound_attach_redirect(sound, nullptr) == 0);
    expect(chain_tail == reinterpret_cast<Wave *>(sound));
    expect(sound_detach_redirect(sound, nullptr) == 0);
    expect(chain_tail == nullptr);

    WaveOperatorDelete = saved_delete;
    WaveDeviceReleaseSlot = saved_release_slot;
    WaveDeviceReleaseGuard = saved_guard;
    WaveChainHead = saved_head;
    WaveChainTail = saved_tail;
}

void test_wave_device_construction() {
    std::vector<uint8_t> storage(sizeof(Wave_Device) + 32, 0);
    std::vector<uint8_t> expected(storage.size());
    uint8_t *const obj = storage.data() + 16;
    auto *device = reinterpret_cast<Wave_Device *>(obj);
    g_wdev_down_obj = obj;

    auto *const saved_delete = WaveOperatorDelete;
    auto *const saved_vctor = VectorCtorIterator;
    auto *const saved_vdtor = VectorDtorIterator;
    WaveOperatorDelete = &observe_wave_operator_delete;
    VectorCtorIterator = &observe_vector_ctor_iter;
    VectorDtorIterator = &observe_vector_dtor_iter;

    // --- the control group's own pair ---
    uint8_t rec[0x18];
    std::memset(rec, 0xA7, sizeof(rec));
    auto *group = reinterpret_cast<WaveControlGroup *>(rec);
    wave_control_group_ctor_redirect(group, nullptr);
    expect(group->head == nullptr && group->tail == nullptr &&
           group->cursor == nullptr && group->count == 0);
    expect(rec[0] == 0xA7 && rec[4] == 0xA7);  // enabled and volume untouched

    // Draining: full, null-wave stop, and the single-node tail arm.
    uint8_t wavebuf[0x6C];
    std::memset(wavebuf, 0, sizeof(wavebuf));
    auto *w = reinterpret_cast<Wave *>(wavebuf);
    WaveGroupNode n1{}, n2{};
    n1.next = &n2;
    n1.wave = w;
    n2.prev = &n1;
    n2.wave = w;
    group->head = &n1;
    group->tail = &n2;
    group->cursor = reinterpret_cast<WaveGroupNode *>(0x5150);
    group->count = 5;
    g_wave_dtor_deletes.clear();
    g_wave_dtor_delete_seen_slot.clear();
    g_wave_delete_poisons = true;  // the drain must read a node BEFORE freeing it
    g_wave_delete_watch = &group->count;
    wave_control_group_dtor_redirect(group, nullptr);
    g_wave_delete_poisons = false;
    g_wave_delete_watch = nullptr;
    expect(g_wave_delete_watch_seen == 4);  // the count drops after each free
    expect(g_wave_dtor_deletes ==
           std::vector<void *>({&n1, &n2}));
    expect(group->head == nullptr && group->tail == nullptr);
    expect(group->count == 3);
    expect(group->cursor == reinterpret_cast<WaveGroupNode *>(0x5150));

    n1 = WaveGroupNode{};
    n2 = WaveGroupNode{};
    n1.next = &n2;
    n1.wave = nullptr;  // stops the drain after its own free
    n2.prev = &n1;
    n2.wave = w;
    group->head = &n1;
    group->tail = &n2;
    group->count = 5;
    g_wave_dtor_deletes.clear();
    wave_control_group_dtor_redirect(group, nullptr);
    expect(g_wave_dtor_deletes == std::vector<void *>({&n1}));
    expect(group->head == &n2 && n2.prev == nullptr);
    expect(group->count == 4);
    expect(group->tail == &n2);

    group->head = nullptr;
    g_wave_dtor_deletes.clear();
    wave_control_group_dtor_redirect(group, nullptr);
    expect(g_wave_dtor_deletes.empty());

    // A follower aimed at the head field itself: the back-link clear lands
    // on the head first and the head store overwrites it second, so the
    // order shows in which value survives.
    n1 = WaveGroupNode{};
    n1.next = reinterpret_cast<WaveGroupNode *>(rec + 8);
    n1.wave = nullptr;  // stop after the first free
    group->head = &n1;
    g_wave_dtor_deletes.clear();
    wave_control_group_dtor_redirect(group, nullptr);
    expect(group->head == reinterpret_cast<WaveGroupNode *>(rec + 8));
    expect(g_wave_dtor_deletes.size() == 1);

    // --- the device constructor ---
    seed_storage(storage.data(), expected.data(), storage.size());
    g_vec_iter_calls = 0;
    std::memcpy(expected.data(), storage.data(), storage.size());
    expect(wave_device_ctor_redirect(device, nullptr) == device);
    expect(g_vec_iter_calls == 1);
    expect(g_vec_iter_array == obj + 0x24);
    expect(g_vec_iter_size == 0x18 && g_vec_iter_count == 0x10);
    expect(g_vec_iter_ctor ==
           reinterpret_cast<void *>(WaveControlGroupOriginalCtor));
    expect(g_vec_iter_dtor ==
           reinterpret_cast<void *>(WaveControlGroupOriginalDtor));
    expect(g_vec_iter_seen_vtable == 0x0066E098u);  // base up during the walk
    {
        uint8_t *const eobj = expected.data() + 16;
        auto e32 = [&](size_t off, uint32_t v) {
            std::memcpy(eobj + off, &v, 4);
        };
        e32(0x00, 0x0066E0E8u);
        e32(0x04, 0);
        e32(0x08, 0x7F);
        e32(0x0C, 0);
        e32(0x10, 0);
        e32(0x14, 0);
        e32(0x18, 0);
        e32(0x1C, 0);
        e32(0x20, 0);
    }
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // --- the device destructor ---
    void *dev_vtable[64];
    std::memset(dev_vtable, 0, sizeof(dev_vtable));
    dev_vtable[0x10 / 4] =
        reinterpret_cast<void *>(&observe_wdev_device_down);
    struct FakeDev { void *vtbl; } fake_dev;
    fake_dev.vtbl = dev_vtable;
    {
        const uint32_t stale = 0xDEADDEADu;  // the ctor's vtable must not
        std::memcpy(obj + 0x00, &stale, 4);  // satisfy the dtor's witness
    }
    void *devp = &fake_dev;
    std::memcpy(obj + 0x14, &devp, 4);
    std::memcpy(expected.data(), storage.data(), storage.size());
    g_vec_iter_calls = 0;
    g_wdev_down_calls = 0;
    g_wdev_down_clears = false;
    wave_device_dtor_redirect(device, nullptr);
    expect(g_vec_iter_calls == 1);
    expect(g_vec_iter_array == obj + 0x24);
    expect(g_vec_iter_size == 0x18 && g_vec_iter_count == 0x10);
    expect(g_vec_iter_seen_vtable == 0x0066E0E8u);  // own vtable up first
    expect(g_wdev_down_calls == 1);
    expect(g_wdev_down_seen_vtable == 0x0066E098u);  // base before wind-down
    {
        uint8_t *const eobj = expected.data() + 16;
        const uint32_t vt = 0x0066E098u;
        std::memcpy(eobj + 0x00, &vt, 4);
        void *dp = &fake_dev;
        std::memcpy(eobj + 0x14, &dp, 4);  // the field is NOT cleared
    }
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // Without a device the wind-down never happens.
    devp = nullptr;
    std::memcpy(obj + 0x14, &devp, 4);
    g_wdev_down_calls = 0;
    wave_device_dtor_redirect(device, nullptr);
    expect(g_wdev_down_calls == 0);

    WaveOperatorDelete = saved_delete;
    VectorCtorIterator = saved_vctor;
    VectorDtorIterator = saved_vdtor;
}

// --- Wave_Device::select fixtures ---------------------------------------
// Every observer appends to one shared event log, so ordering mutants
// (halting after the device switch, replaying before it) surface as
// sequence changes, not just count changes.
enum {
    kWSelAttrib = 1,
    kWSelNext,
    kWSelHalt,
    kWSelPrev,
    kWSelLoad,
    kWSelDevSelect,
};

struct WSelEvent {
    int tag;
    void *obj;
    void *arg;
    bool operator==(const WSelEvent &o) const {
        return tag == o.tag && obj == o.obj && arg == o.arg;
    }
};

std::vector<WSelEvent> g_wsel_events;

std::vector<std::pair<void *, void *>> g_wsel_load_links;

int g_wsel_canary_hits = 0;

uint32_t wsel_read32(void *self, size_t off) {
    uint32_t v;
    std::memcpy(&v, reinterpret_cast<uint8_t *>(self) + off, 4);
    return v;
}

int __thiscall observe_wsel_attrib(void *self) {
    g_wsel_events.push_back({kWSelAttrib, self, nullptr});
    return static_cast<int>(wsel_read32(self, 8));
}

// The chain accessors are emulated exactly as the four-byte originals are
// written: live reads of the link fields. The chain script lives in the
// objects themselves, so a wrong-offset mutant reads the wrong cell.
void *__thiscall observe_wsel_next(void *self) {
    void *const v = reinterpret_cast<void *>(wsel_read32(self, 0x48));
    g_wsel_events.push_back({kWSelNext, self, v});
    return v;
}

void *__thiscall observe_wsel_prev(void *self) {
    void *const v = reinterpret_cast<void *>(wsel_read32(self, 0x44));
    g_wsel_events.push_back({kWSelPrev, self, v});
    return v;
}

void __thiscall observe_wsel_halt(void *self) {
    g_wsel_events.push_back({kWSelHalt, self, nullptr});
}

int __thiscall observe_wsel_load(void *self, char *fname) {
    g_wsel_events.push_back({kWSelLoad, self, fname});
    g_wsel_load_links.emplace_back(
        reinterpret_cast<void *>(wsel_read32(self, 0x44)),
        reinterpret_cast<void *>(wsel_read32(self, 0x48)));
    return 0;
}

int __thiscall observe_wsel_dev_select(void *self, unsigned long a1) {
    g_wsel_events.push_back({kWSelDevSelect, self,
                             reinterpret_cast<void *>(a1)});
    return 0;
}

int __thiscall observe_wsel_canary(void *) {
    ++g_wsel_canary_hits;
    return 0;
}

void test_wave_device_select() {
    std::vector<uint8_t> storage(sizeof(Wave_Device) + 32, 0);
    std::vector<uint8_t> expected(storage.size());
    uint8_t *const obj = storage.data() + 16;
    auto *device = reinterpret_cast<Wave_Device *>(obj);
    seed_storage(storage.data(), expected.data(), storage.size());

    void *wave_vtable[64];
    for (void *&slot : wave_vtable) {
        slot = reinterpret_cast<void *>(&observe_wsel_canary);
    }
    wave_vtable[0x70 / 4] = reinterpret_cast<void *>(&observe_wsel_attrib);
    wave_vtable[0x64 / 4] = reinterpret_cast<void *>(&observe_wsel_next);
    wave_vtable[0x68 / 4] = reinterpret_cast<void *>(&observe_wsel_prev);
    wave_vtable[0x14 / 4] = reinterpret_cast<void *>(&observe_wsel_halt);
    wave_vtable[0x10 / 4] = reinterpret_cast<void *>(&observe_wsel_load);

    void *dev_vtable[64];
    for (void *&slot : dev_vtable) {
        slot = reinterpret_cast<void *>(&observe_wsel_canary);
    }
    dev_vtable[0x18 / 4] = reinterpret_cast<void *>(&observe_wsel_dev_select);
    uint8_t fake_dev[8];
    {
        void *vt = dev_vtable;
        std::memcpy(fake_dev, &vt, 4);
    }

    Wave **const saved_head = WaveChainHead;
    Wave *chain_head = nullptr;
    WaveChainHead = &chain_head;

    auto reset = [&] {
        g_wsel_events.clear();
        g_wsel_load_links.clear();
        g_wsel_canary_hits = 0;
    };
    auto set_device = [&](void *dev) {
        std::memcpy(obj + 0x14, &dev, 4);
        std::memcpy(expected.data() + 16 + 0x14, &dev, 4);
    };
    auto put32 = [](uint8_t *base, size_t off, uint32_t v) {
        std::memcpy(base + off, &v, 4);
    };
    auto putp = [](uint8_t *base, size_t off, const void *p) {
        std::memcpy(base + off, &p, 4);
    };

    // Wave stand-ins: a seeded buffer and its expected mirror. The attribute
    // observer answers from +8, the chain threads through +0x48.
    const size_t kWaveBytes = 0x6C;
    uint8_t wb[5][kWaveBytes], wb_want[5][kWaveBytes];
    auto make_wave = [&](int i, uint32_t attrib) {
        std::memset(wb[i], 0x50 + i, kWaveBytes);
        void *vt = wave_vtable;
        putp(wb[i], 0, vt);
        put32(wb[i], 8, attrib);
        return reinterpret_cast<Wave *>(wb[i]);
    };
    auto mirror_wave = [&](int i) {
        std::memcpy(wb_want[i], wb[i], kWaveBytes);
    };

    // 1) Without a device the answer is 2 and nothing at all happens.
    set_device(nullptr);
    reset();
    chain_head = make_wave(0, 1);
    mirror_wave(0);
    expect(device->select(0x21) == 2);
    expect(g_wsel_events.empty());
    expect(g_wsel_canary_hits == 0);
    expect(std::memcmp(wb[0], wb_want[0], kWaveBytes) == 0);
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // 2) A device but an empty chain: only the device's own select runs,
    //    and the argument passes straight through.
    set_device(fake_dev);
    reset();
    chain_head = nullptr;
    expect(device->select(0x33) == 0);
    {
        std::vector<WSelEvent> want{
            {kWSelDevSelect, fake_dev, reinterpret_cast<void *>(0x33)},
        };
        expect(g_wsel_events == want);
    }
    expect(g_wsel_canary_hits == 0);
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // 3) A five-sound chain. Attribute words: 1 = halt-and-resume, 2 = bit 1
    //    clear (one attribute fetch, skipped), 5 = bit 4 protects it (two
    //    fetches, skipped), and the last active sound is NOT the chain end,
    //    so its stale next link must be scrubbed before its reload.
    Wave *const s1 = make_wave(0, 1);
    Wave *const s2 = make_wave(1, 2);
    Wave *const s3 = make_wave(2, 5);
    Wave *const s4 = make_wave(3, 1);
    Wave *const s5 = make_wave(4, 2);
    putp(wb[0], 0x48, s2);
    putp(wb[1], 0x48, s3);
    putp(wb[2], 0x48, s4);
    putp(wb[3], 0x48, s5);
    putp(wb[4], 0x48, nullptr);
    char *const fname1 = reinterpret_cast<char *>(0xF001);
    char *const fname4 = reinterpret_cast<char *>(0xF004);
    putp(wb[0], 0x4C, fname1);
    putp(wb[3], 0x4C, fname4);
    for (int i = 0; i < 5; ++i) {
        put32(wb[i], 0x44, 0xABC0 + i);
        mirror_wave(i);
    }
    // The two resumed sounds end with both links cleared; the skipped three
    // keep every byte.
    put32(wb_want[0], 0x44, 0);
    putp(wb_want[0], 0x48, nullptr);
    put32(wb_want[3], 0x44, 0);
    putp(wb_want[3], 0x48, nullptr);
    reset();
    chain_head = s1;
    expect(wave_device_select_redirect(device, nullptr, 0x77) == 0);
    {
        std::vector<WSelEvent> want{
            {kWSelAttrib, s1, nullptr},
            {kWSelAttrib, s1, nullptr},
            {kWSelNext, s1, s2},
            {kWSelHalt, s1, nullptr},
            {kWSelAttrib, s2, nullptr},
            {kWSelNext, s2, s3},
            {kWSelAttrib, s3, nullptr},
            {kWSelAttrib, s3, nullptr},
            {kWSelNext, s3, s4},
            {kWSelAttrib, s4, nullptr},
            {kWSelAttrib, s4, nullptr},
            {kWSelNext, s4, s5},
            {kWSelHalt, s4, nullptr},
            {kWSelAttrib, s5, nullptr},
            {kWSelNext, s5, nullptr},
            {kWSelDevSelect, fake_dev, reinterpret_cast<void *>(0x77)},
            {kWSelPrev, s4, s1},
            {kWSelLoad, s4, fname4},
            {kWSelPrev, s1, nullptr},
            {kWSelLoad, s1, fname1},
        };
        expect(g_wsel_events == want);
    }
    // Both links are already scrubbed when each reload is dispatched.
    {
        std::vector<std::pair<void *, void *>> want_links{
            {nullptr, nullptr},
            {nullptr, nullptr},
        };
        expect(g_wsel_load_links == want_links);
    }
    expect(g_wsel_canary_hits == 0);
    for (int i = 0; i < 5; ++i) {
        expect(std::memcmp(wb[i], wb_want[i], kWaveBytes) == 0);
    }
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // 4) Two overlapping stand-ins, B four bytes into A, so B's prev cell IS
    //    A's next cell: the append's two stores hit the same address and
    //    their order decides which pointer survives - the original leaves
    //    the earlier sound there, and the replay walks [B, A], with A's
    //    remembered name already scrubbed to null by B's next-link clear.
    uint8_t ov[0x100], ov_want[0x100];
    std::memset(ov, 0x77, sizeof(ov));
    uint8_t *const wa = ov + 16;
    uint8_t *const wob = wa + 4;
    {
        void *vt = wave_vtable;
        putp(wa, 0, vt);
        putp(wob, 0, vt);
    }
    put32(wa, 8, 1);
    put32(wob, 8, 1);
    putp(wa, 0x48, wob);      // A's chain next, aka B's prev cell
    putp(wob, 0x48, nullptr); // B ends the chain, aka A's fname cell
    char *const fnameB = reinterpret_cast<char *>(0xB00F);
    putp(wob, 0x4C, fnameB);
    put32(wa, 0x44, 0xABCF);
    std::memcpy(ov_want, ov, sizeof(ov));
    put32(ov_want + 16, 0x44, 0);  // A's prev, cleared on first threading
    put32(ov_want + 16, 0x48, 0);  // the shared cell, scrubbed by B's replay
    put32(ov_want + 16, 0x4C, 0);  // A's fname, aka B's next, scrubbed too
    reset();
    chain_head = reinterpret_cast<Wave *>(wa);
    expect(device->select(0x55) == 0);
    {
        Wave *const A = reinterpret_cast<Wave *>(wa);
        Wave *const B = reinterpret_cast<Wave *>(wob);
        std::vector<WSelEvent> want{
            {kWSelAttrib, A, nullptr},
            {kWSelAttrib, A, nullptr},
            {kWSelNext, A, B},
            {kWSelHalt, A, nullptr},
            {kWSelAttrib, B, nullptr},
            {kWSelAttrib, B, nullptr},
            {kWSelNext, B, nullptr},
            {kWSelHalt, B, nullptr},
            {kWSelDevSelect, fake_dev, reinterpret_cast<void *>(0x55)},
            {kWSelPrev, B, A},
            {kWSelLoad, B, fnameB},
            {kWSelPrev, A, nullptr},
            {kWSelLoad, A, nullptr},
        };
        expect(g_wsel_events == want);
    }
    expect(g_wsel_canary_hits == 0);
    expect(std::memcmp(ov, ov_want, sizeof(ov)) == 0);
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // 5) The mirrored overlap, B four bytes BELOW A, aligns B's remembered
    //    name with A's next cell: the append's forward store lands in it, so
    //    B reloads by the pointer that store wrote - dropping the store
    //    reloads B by the null A's first threading left there.
    uint8_t ov5[0x100], ov5_want[0x100];
    std::memset(ov5, 0x66, sizeof(ov5));
    uint8_t *const wb5 = ov5 + 16;
    uint8_t *const wa5 = wb5 + 4;
    {
        void *vt = wave_vtable;
        putp(wa5, 0, vt);
        putp(wb5, 0, vt);
    }
    put32(wa5, 8, 1);
    put32(wb5, 8, 1);
    putp(wa5, 0x48, wb5);      // A's chain next, aka B's fname cell
    putp(wb5, 0x48, nullptr);  // B ends the chain, aka A's prev cell
    char *const fnameA5 = reinterpret_cast<char *>(0xA005);
    putp(wa5, 0x4C, fnameA5);
    std::memcpy(ov5_want, ov5, sizeof(ov5));
    put32(ov5_want + 16, 0x44, 0);       // B's prev (A's field 0x40 cell)
    put32(ov5_want + 16 + 4, 0x44, 0);   // A's prev, aka B's next
    put32(ov5_want + 16 + 4, 0x48, 0);   // A's next, aka B's fname
    reset();
    chain_head = reinterpret_cast<Wave *>(wa5);
    expect(device->select(0x66) == 0);
    {
        Wave *const A = reinterpret_cast<Wave *>(wa5);
        Wave *const B = reinterpret_cast<Wave *>(wb5);
        std::vector<WSelEvent> want{
            {kWSelAttrib, A, nullptr},
            {kWSelAttrib, A, nullptr},
            {kWSelNext, A, B},
            {kWSelHalt, A, nullptr},
            {kWSelAttrib, B, nullptr},
            {kWSelAttrib, B, nullptr},
            {kWSelNext, B, nullptr},
            {kWSelHalt, B, nullptr},
            {kWSelDevSelect, fake_dev, reinterpret_cast<void *>(0x66)},
            {kWSelPrev, B, A},
            {kWSelLoad, B, B},  // reloaded by the pointer the append stored
            {kWSelPrev, A, nullptr},
            {kWSelLoad, A, fnameA5},
        };
        expect(g_wsel_events == want);
    }
    {
        std::vector<std::pair<void *, void *>> want_links{
            {nullptr, nullptr},
            {nullptr, nullptr},
        };
        expect(g_wsel_load_links == want_links);
    }
    expect(g_wsel_canary_hits == 0);
    expect(std::memcmp(ov5, ov5_want, sizeof(ov5)) == 0);
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // 6) A two-sound cycle that revisits the first threaded sound: the walk
    //    reaches it a second time only through the link its first threading
    //    scrubbed, so the revisit's next fetch answers null and the walk
    //    ends - without that scrub the cycle never terminates. The revisit
    //    threads the sound onto itself, and the replay unwinds the self-loop
    //    in exactly two reloads.
    Wave *const s6a = make_wave(0, 1);
    Wave *const s6d = make_wave(1, 2);
    putp(wb[0], 0x48, s6d);
    putp(wb[1], 0x48, s6a);  // the inactive sound points BACK at the first
    char *const fname6 = reinterpret_cast<char *>(0xA006);
    putp(wb[0], 0x4C, fname6);
    put32(wb[0], 0x44, 0xABC9);
    mirror_wave(0);
    mirror_wave(1);
    put32(wb_want[0], 0x44, 0);
    putp(wb_want[0], 0x48, nullptr);
    reset();
    chain_head = s6a;
    expect(device->select(0x11) == 0);
    {
        std::vector<WSelEvent> want{
            {kWSelAttrib, s6a, nullptr},
            {kWSelAttrib, s6a, nullptr},
            {kWSelNext, s6a, s6d},
            {kWSelHalt, s6a, nullptr},
            {kWSelAttrib, s6d, nullptr},
            {kWSelNext, s6d, s6a},
            {kWSelAttrib, s6a, nullptr},
            {kWSelAttrib, s6a, nullptr},
            {kWSelNext, s6a, nullptr},
            {kWSelHalt, s6a, nullptr},
            {kWSelDevSelect, fake_dev, reinterpret_cast<void *>(0x11)},
            {kWSelPrev, s6a, s6a},
            {kWSelLoad, s6a, fname6},
            {kWSelPrev, s6a, nullptr},
            {kWSelLoad, s6a, fname6},
        };
        expect(g_wsel_events == want);
    }
    expect(g_wsel_canary_hits == 0);
    expect(std::memcmp(wb[0], wb_want[0], kWaveBytes) == 0);
    expect(std::memcmp(wb[1], wb_want[1], kWaveBytes) == 0);
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    WaveChainHead = saved_head;
}

// --- AutoSound lifecycle fixtures ---------------------------------------
std::vector<void *> g_asnd_deletes;

void __cdecl observe_asnd_delete(void *block) {
    g_asnd_deletes.push_back(block);
}

// Replay one field-copy in its exact store order against the expected
// buffer. `defaults` null means the source block IS the object - the
// self-aliasing scenarios, where each read sees every store so far, so
// the expected bytes depend on the precise legacy order.
void asnd_simulate(uint8_t *expected, size_t base, const int (*order)[2],
                   size_t count, const uint32_t *defaults) {
    for (size_t step = 0; step < count; ++step) {
        const int dst = order[step][0];
        const int src = order[step][1];
        uint32_t value;
        if (defaults) {
            value = defaults[src];
        } else {
            std::memcpy(&value, expected + base + src * 4u, 4);
        }
        std::memcpy(expected + base + dst * 4u, &value, 4);
    }
}

void asnd_orders(std::vector<std::array<int, 2>> &close_order,
                 std::vector<std::array<int, 2>> &close2_order) {
    close_order = {{1, 0}, {3, 1}, {4, 2}, {2, 3}};
    close2_order = {{1, 0}, {2, 3}, {3, 1}, {4, 2}};
    for (int index = 4; index < 37; ++index) {
        close_order.push_back({index + 1, index});
        close2_order.push_back({index + 1, index});
    }
}

void test_auto_sound_lifecycle() {
    uint32_t defaults[37];
    for (size_t index = 0; index < ARRAYSIZE(defaults); ++index) {
        defaults[index] = 0x63000000U + index * 0x30201U;
    }
    uint32_t *const saved_defaults = AutoSoundDefaults;
    auto *const saved_delete = AutoSoundOperatorDelete;
    AutoSoundDefaults = defaults;
    AutoSoundOperatorDelete = &observe_asnd_delete;

    std::vector<std::array<int, 2>> close_order, close2_order;
    asnd_orders(close_order, close2_order);
    auto run = [&](void (*invoke)(AutoSound *),
                   const std::vector<std::array<int, 2>> *order,
                   bool self_alias, bool expect_vtable) {
        alignas(AutoSound) uint8_t storage[sizeof(AutoSound) + 32];
        uint8_t expected[sizeof(storage)];
        seed_storage(storage, expected, sizeof(storage));
        const uint32_t canary = 0xDEADDEADu;
        std::memcpy(storage + 16, &canary, 4);
        std::memcpy(expected + 16, &canary, 4);
        auto *const self = reinterpret_cast<AutoSound *>(storage + 16);
        AutoSoundDefaults =
            self_alias ? reinterpret_cast<uint32_t *>(storage + 16) : defaults;
        if (expect_vtable) {
            write_at(expected, 16, AutoSoundVtable);
        }
        if (order) {
            asnd_simulate(
                expected, 16,
                reinterpret_cast<const int(*)[2]>(order->data()->data()),
                order->size(), self_alias ? nullptr : defaults);
        } else {
            for (size_t index = 1; index < 38; ++index) {
                write_at(expected, 16 + index * 4, uint32_t(0));
            }
        }
        invoke(self);
        expect_storage_bytes(storage, expected, sizeof(storage));
        AutoSoundDefaults = defaults;
    };

    // close and close2 against an independent default block, both directly
    // and through their redirects.
    run([](AutoSound *s) { s->close(); }, &close_order, false, false);
    run([](AutoSound *s) { auto_sound_close_redirect(s, nullptr); },
        &close_order, false, false);
    run([](AutoSound *s) { s->close2(); }, &close2_order, false, false);
    run([](AutoSound *s) { auto_sound_close2_redirect(s, nullptr); },
        &close2_order, false, false);

    // init zeroes the same extent; the vtable cell keeps its canary.
    run([](AutoSound *s) { s->init(); }, nullptr, false, false);
    run([](AutoSound *s) { auto_sound_init_redirect(s, nullptr); }, nullptr,
        false, false);

    // The default block aimed at the object itself: every read sees the
    // stores so far, so the two legacy store orders produce two DIFFERENT
    // final patterns - the byte compare pins each function to its own.
    run([](AutoSound *s) { s->close(); }, &close_order, true, false);
    run([](AutoSound *s) { s->close2(); }, &close2_order, true, false);

    // The scalar deleting destructor: vtable re-installed BEFORE the field
    // reset - the self-aliasing copy reads the fresh vtable through the
    // default block, so a late install leaves the canary behind instead.
    g_asnd_deletes.clear();
    run([](AutoSound *s) {
        expect(auto_sound_scalar_dtor_redirect(s, nullptr, 0) == s);
    }, &close_order, true, true);
    expect(g_asnd_deletes.empty());
    run([](AutoSound *s) {
        expect(auto_sound_scalar_dtor_redirect(s, nullptr, 2) == s);
    }, &close_order, false, true);
    expect(g_asnd_deletes.empty());
    run([](AutoSound *s) {
        expect(auto_sound_scalar_dtor_redirect(s, nullptr, 1) == s);
        expect(g_asnd_deletes == std::vector<void *>({s}));
    }, &close_order, false, true);

    AutoSoundDefaults = saved_defaults;
    AutoSoundOperatorDelete = saved_delete;
}

// --- Ambience destructor fixtures ---------------------------------------
struct AmbEvent {
    int tag;  // 1 = operator delete, 2 = device release
    void *ptr;
    uint32_t vtable;
    void *fname_cell;
    bool operator==(const AmbEvent &o) const {
        return tag == o.tag && ptr == o.ptr && vtable == o.vtable &&
               fname_cell == o.fname_cell;
    }
};

std::vector<AmbEvent> g_amb_events;

uint8_t *g_amb_obj = nullptr;

void *g_amb_rearm_fname = nullptr;

void *g_amb_rearm_device = nullptr;

void *g_amb_delete_rearm_device = nullptr;

bool g_amb_release_clears_guard = false;

int *g_amb_guard_cell = nullptr;

AmbEvent amb_event(int tag, void *ptr) {
    uint32_t vt = 0;
    void *fname = nullptr;
    std::memcpy(&vt, g_amb_obj, 4);
    std::memcpy(&fname, g_amb_obj + 0x4C, 4);
    return AmbEvent{tag, ptr, vt, fname};
}

// The second-stage device release is reachable only through a write that
// lands between the first stage's clear and the second stage's re-read -
// and the only call in that window is the second-stage delete. So THIS
// observer re-arms the device when it runs under the Sound-stage vtable.
void __cdecl observe_amb_delete(void *block) {
    g_amb_events.push_back(amb_event(1, block));
    if (g_amb_delete_rearm_device &&
        g_amb_events.back().vtable == 0x0066E3C0u) {
        std::memcpy(g_amb_obj + 0x3C, &g_amb_delete_rearm_device, 4);
    }
}

// The release hook can re-arm the already-cleared fields (or drop the
// guard): that is what makes the inlined second-stage teardown - dead in
// any straight-line run - observable.
void __cdecl observe_amb_release(void *device) {
    g_amb_events.push_back(amb_event(2, device));
    if (g_amb_events.back().vtable == 0x0066E538u) {
        if (g_amb_rearm_fname) {
            std::memcpy(g_amb_obj + 0x4C, &g_amb_rearm_fname, 4);
        }
        if (g_amb_rearm_device) {
            std::memcpy(g_amb_obj + 0x3C, &g_amb_rearm_device, 4);
        }
        if (g_amb_release_clears_guard) {
            *g_amb_guard_cell = 0;
        }
    }
}

void test_ambience_dtor() {
    auto *const saved_delete = WaveOperatorDelete;
    auto *const saved_release_slot = WaveDeviceReleaseSlot;
    auto *const saved_guard = WaveDeviceReleaseGuard;
    Wave **const saved_head = WaveChainHead;
    Wave **const saved_tail = WaveChainTail;

    func_wave_device_release *release_fn = &observe_amb_release;
    int guard = 1;
    Wave *chain_head = nullptr;
    Wave *chain_tail = nullptr;
    WaveOperatorDelete = &observe_amb_delete;
    WaveDeviceReleaseSlot = &release_fn;
    WaveDeviceReleaseGuard = &guard;
    WaveChainHead = &chain_head;
    WaveChainTail = &chain_tail;

    const size_t kAmb = 0x80;
    std::vector<uint8_t> storage(kAmb + 32);
    std::vector<uint8_t> expected(storage.size());
    uint8_t *const obj = storage.data() + 16;
    auto put = [&](size_t off, const void *p) { std::memcpy(obj + off, &p, 4); };
    auto put32 = [&](size_t off, uint32_t v) { std::memcpy(obj + off, &v, 4); };
    auto eput = [&](size_t off, const void *p) {
        std::memcpy(expected.data() + 16 + off, &p, 4);
    };
    auto eput32 = [&](size_t off, uint32_t v) {
        std::memcpy(expected.data() + 16 + off, &v, 4);
    };
    char fbuf1, fbuf2, dbuf1, dbuf2;
    void *const F = &fbuf1;
    void *const F2 = &fbuf2;
    void *const D = &dbuf1;
    void *const D2 = &dbuf2;
    g_amb_obj = obj;
    g_amb_guard_cell = &guard;
    auto reset = [&](uint32_t flags, void *fname, void *device) {
        seed_storage(storage.data(), expected.data(), storage.size());
        put32(0x00, 0xDEADDEADu);
        put32(0x40, flags);
        put(0x3C, device);
        put(0x4C, fname);
        std::memcpy(expected.data(), storage.data(), storage.size());
        eput32(0x00, 0x0066E444u);
        eput(0x3C, nullptr);
        eput(0x4C, nullptr);
        g_amb_events.clear();
        g_amb_rearm_fname = nullptr;
        g_amb_rearm_device = nullptr;
        g_amb_delete_rearm_device = nullptr;
        g_amb_release_clears_guard = false;
        guard = 1;
    };
    auto run = [&] {
        reinterpret_cast<Ambience *>(obj)->~Ambience();
    };

    // The plain run: one delete (name still in place at call time), one
    // guarded release (name already scrubbed by then), every stage ending
    // on the ultimate base vtable, nothing else touched.
    reset(0x11, F, D);
    run();
    {
        std::vector<AmbEvent> want{
            {1, F, 0x0066E538u, F},
            {2, D, 0x0066E538u, nullptr},
        };
        expect(g_amb_events == want);
    }
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // Guard down: the device is forgotten but never released.
    reset(0x11, F, D);
    guard = 0;
    run();
    {
        std::vector<AmbEvent> want{{1, F, 0x0066E538u, F}};
        expect(g_amb_events == want);
    }
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // Nothing to tear down still stages the vtable and scrubs the fields.
    reset(0x11, nullptr, nullptr);
    run();
    expect(g_amb_events.empty());
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // Mid-chain unlink through the redirect: the neighbours bridge, the
    // links scrub, only the chained bit drops.
    uint8_t prev_node[0x50], next_node[0x50];
    std::memset(prev_node, 0x21, sizeof(prev_node));
    std::memset(next_node, 0x22, sizeof(next_node));
    reset(0x13, F, D);
    put(0x44, prev_node);
    put(0x48, next_node);
    chain_head = reinterpret_cast<Wave *>(0x1111);
    chain_tail = reinterpret_cast<Wave *>(0x2222);
    eput(0x44, nullptr);
    eput(0x48, nullptr);
    eput32(0x40, 0x11);
    ambience_dtor_redirect(reinterpret_cast<Ambience *>(obj), nullptr);
    expect(g_amb_events.size() == 2);
    {
        void *bridged;
        std::memcpy(&bridged, prev_node + 0x48, 4);
        expect(bridged == next_node);
        std::memcpy(&bridged, next_node + 0x44, 4);
        expect(bridged == prev_node);
    }
    expect(chain_head == reinterpret_cast<Wave *>(0x1111));
    expect(chain_tail == reinterpret_cast<Wave *>(0x2222));
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // A lone chained node retargets both chain ends instead.
    reset(0x2, nullptr, nullptr);
    put(0x44, nullptr);
    put(0x48, nullptr);
    chain_head = reinterpret_cast<Wave *>(obj);
    chain_tail = reinterpret_cast<Wave *>(obj);
    eput(0x44, nullptr);
    eput(0x48, nullptr);
    eput32(0x40, 0);
    run();
    expect(chain_head == nullptr);
    expect(chain_tail == nullptr);
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // Re-armed by the release hook: the second-stage name teardown comes
    // alive under the Sound-stage vtable. The re-armed device is wiped
    // again by the first stage's own unconditional clear - no second
    // release can come from here.
    reset(0x11, F, D);
    g_amb_rearm_fname = F2;
    g_amb_rearm_device = D2;
    run();
    {
        std::vector<AmbEvent> want{
            {1, F, 0x0066E538u, F},
            {2, D, 0x0066E538u, nullptr},
            {1, F2, 0x0066E3C0u, F2},
        };
        expect(g_amb_events == want);
    }
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // A device re-armed by the second-stage DELETE lands in the only window
    // the second stage still reads: its guarded release runs under the
    // Sound-stage vtable, with the name already scrubbed.
    reset(0x11, F, D);
    g_amb_rearm_fname = F2;
    g_amb_delete_rearm_device = D2;
    run();
    {
        std::vector<AmbEvent> want{
            {1, F, 0x0066E538u, F},
            {2, D, 0x0066E538u, nullptr},
            {1, F2, 0x0066E3C0u, F2},
            {2, D2, 0x0066E3C0u, nullptr},
        };
        expect(g_amb_events == want);
    }
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // The same re-arm under a guard the first release dropped: the second
    // stage re-checks the guard, skips the release, and still forgets the
    // device.
    reset(0x11, F, D);
    g_amb_rearm_fname = F2;
    g_amb_delete_rearm_device = D2;
    g_amb_release_clears_guard = true;
    run();
    {
        std::vector<AmbEvent> want{
            {1, F, 0x0066E538u, F},
            {2, D, 0x0066E538u, nullptr},
            {1, F2, 0x0066E3C0u, F2},
        };
        expect(g_amb_events == want);
    }
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    WaveOperatorDelete = saved_delete;
    WaveDeviceReleaseSlot = saved_release_slot;
    WaveDeviceReleaseGuard = saved_guard;
    WaveChainHead = saved_head;
    WaveChainTail = saved_tail;
}

// --- popup_wave_callback fixtures ---------------------------------------
enum {
    kPWaveIsPlaying = 1,
    kPWaveLoad,
    kPWavePlay,
    kPWaveTime,
    kPWaveOwner,
    kPWaveFx,
};

struct PWaveEvent {
    int tag;
    void *obj;
    long arg;
    bool operator==(const PWaveEvent &o) const {
        return tag == o.tag && obj == o.obj && arg == o.arg;
    }
};

std::vector<PWaveEvent> g_pwave_events;

int g_pwave_canary_hits = 0;

Wave *g_pwave_playing_wave = nullptr;

unsigned long g_pwave_time_value = 0;

uint32_t *g_pwave_time_clears = nullptr;

int __thiscall observe_pwave_is_playing(Wave *wave) {
    g_pwave_events.push_back({kPWaveIsPlaying, wave, 0});
    return wave == g_pwave_playing_wave ? 1 : 0;
}

// The load observer witnesses the last-played slot at call time: the
// original remembers the index BEFORE loading.
int __thiscall observe_pwave_load(Wave *wave) {
    g_pwave_events.push_back({kPWaveLoad, wave, *PopupWaveLastIndex});
    return 1;
}

int __thiscall observe_pwave_play(Wave *wave) {
    g_pwave_events.push_back({kPWavePlay, wave, 0});
    return 1;
}

unsigned long __stdcall observe_pwave_time() {
    g_pwave_events.push_back(
        {kPWaveTime, nullptr, static_cast<long>(g_pwave_time_value)});
    if (g_pwave_time_clears) {
        *g_pwave_time_clears &= ~0x400u;
    }
    return g_pwave_time_value;
}

void __thiscall observe_pwave_owner(void *self) {
    g_pwave_events.push_back({kPWaveOwner, self, 0});
}

void __thiscall observe_pwave_fx(FX *fx, int effect) {
    g_pwave_events.push_back({kPWaveFx, fx, effect});
}

void __thiscall observe_pwave_canary(void *) {
    ++g_pwave_canary_hits;
}

void test_popup_wave_callback() {
    uint32_t *const saved_flags = PopupWaveFlags;
    void **const saved_context = PopupWaveContext;
    Wave *const saved_voice = PopupWaveVoice;
    Wave *const saved_bank = PopupWaveBank;
    int32_t *const saved_last = PopupWaveLastIndex;
    void **const saved_owner_slot = PopupWaveOwnerSlot;
    FX *const saved_fx = PopupWaveFx;
    auto *const saved_is_playing = PopupWaveIsPlaying;
    auto *const saved_load = PopupWaveLoad;
    auto *const saved_play = PopupWavePlay;
    auto *const saved_fx_play = PopupFxPlay;
    auto *const saved_time_slot = PopupWaveTimeSlot;

    uint32_t flags = 0x400;
    uint8_t context_bytes[0x60];
    std::memset(context_bytes, 0x7F, sizeof(context_bytes));
    void *context = context_bytes;
    std::vector<uint8_t> bank(45 * sizeof(Wave), 0x41);
    uint8_t voice_bytes[sizeof(Wave)];
    int32_t last = 7;
    void *owner_vtable[0x148 / 4];
    for (void *&slot : owner_vtable) {
        slot = reinterpret_cast<void *>(&observe_pwave_canary);
    }
    owner_vtable[0x138 / 4] = reinterpret_cast<void *>(&observe_pwave_owner);
    uint8_t owner_bytes[8];
    {
        void *vt = owner_vtable;
        std::memcpy(owner_bytes, &vt, 4);
    }
    void *owner = nullptr;
    uint8_t fx_bytes[8];
    func_popup_time_source *time_fn = &observe_pwave_time;

    PopupWaveFlags = &flags;
    PopupWaveContext = &context;
    PopupWaveVoice = reinterpret_cast<Wave *>(voice_bytes);
    PopupWaveBank = reinterpret_cast<Wave *>(bank.data());
    PopupWaveLastIndex = &last;
    PopupWaveOwnerSlot = &owner;
    PopupWaveFx = reinterpret_cast<FX *>(fx_bytes);
    PopupWaveIsPlaying = &observe_pwave_is_playing;
    PopupWaveLoad = &observe_pwave_load;
    PopupWavePlay = &observe_pwave_play;
    PopupFxPlay = &observe_pwave_fx;
    PopupWaveTimeSlot = &time_fn;

    Wave *const voice = PopupWaveVoice;
    Wave *const bank0 = PopupWaveBank;
    std::vector<uint8_t> popup_bytes(sizeof(PopupWave), 0x5C);
    auto *const popup = reinterpret_cast<PopupWave *>(popup_bytes.data());
    auto reset = [&](int32_t index) {
        g_pwave_events.clear();
        g_pwave_canary_hits = 0;
        g_pwave_playing_wave = nullptr;
        g_pwave_time_clears = nullptr;
        flags = 0x400;
        popup->wave_index_ = index;
        popup->armed_108_ = popup_bytes.data();
    };

    // Every gate alone silences it: the flag bit, a null popup, a negative
    // or out-of-range index, an unarmed popup.
    reset(5);
    flags = ~0x400u;
    popup_wave_callback(popup, 0);
    expect(g_pwave_events.empty());
    reset(5);
    popup_wave_callback(nullptr, 0);
    expect(g_pwave_events.empty());
    reset(-1);
    popup_wave_callback(popup, 0);
    expect(g_pwave_events.empty());
    reset(5);
    popup->armed_108_ = nullptr;
    popup_wave_callback(popup, 0);
    expect(g_pwave_events.empty());
    reset(0x2D);
    popup_wave_callback(popup, 0);
    expect(g_pwave_events.empty());

    // The plain path: both waves idle, the index is remembered before its
    // bank entry loads, and the boundary index 0x2C is still inside.
    reset(5);
    last = 7;
    popup_wave_callback(popup, 0);
    {
        std::vector<PWaveEvent> want{
            {kPWaveIsPlaying, voice, 0},
            {kPWaveIsPlaying, bank0 + 7, 0},
            {kPWaveLoad, bank0 + 5, 5},
            {kPWavePlay, bank0 + 5, 0},
        };
        expect(g_pwave_events == want);
    }
    expect(last == 5);
    expect(popup->wave_index_ == 5);
    reset(0x2C);
    last = 3;
    popup_wave_callback(popup, 0);
    expect(g_pwave_events.size() == 4);
    expect(last == 0x2C);

    // Index zero is a real wave, not a rejected one.
    reset(0);
    last = 3;
    popup_wave_callback(popup, 0);
    {
        std::vector<PWaveEvent> want{
            {kPWaveIsPlaying, voice, 0},
            {kPWaveIsPlaying, bank0 + 3, 0},
            {kPWaveLoad, bank0 + 0, 0},
            {kPWavePlay, bank0 + 0, 0},
        };
        expect(g_pwave_events == want);
    }
    expect(last == 0);

    // A playing voiceover or a still-playing last wave stops the reload.
    reset(5);
    last = 9;
    g_pwave_playing_wave = voice;
    popup_wave_callback(popup, 0);
    {
        std::vector<PWaveEvent> want{{kPWaveIsPlaying, voice, 0}};
        expect(g_pwave_events == want);
    }
    expect(last == 9);
    reset(5);
    last = 9;
    g_pwave_playing_wave = bank0 + 9;
    popup_wave_callback(popup, 0);
    {
        std::vector<PWaveEvent> want{
            {kPWaveIsPlaying, voice, 0},
            {kPWaveIsPlaying, bank0 + 9, 0},
        };
        expect(g_pwave_events == want);
    }
    expect(last == 9);

    // Wave 0x19 turns into 0x25 only below -0x46, and once rewritten the
    // owner's virtual no longer fires.
    reset(0x19);
    last = 2;
    owner = owner_bytes;
    write_at(context_bytes, 0x50, uint32_t(-0x47));
    popup_wave_callback(popup, 0);
    {
        std::vector<PWaveEvent> want{
            {kPWaveIsPlaying, voice, 0},
            {kPWaveIsPlaying, bank0 + 2, 0},
            {kPWaveLoad, bank0 + 0x25, 0x25},
            {kPWavePlay, bank0 + 0x25, 0},
        };
        expect(g_pwave_events == want);
    }
    expect(popup->wave_index_ == 0x25);
    expect(last == 0x25);

    // At exactly -0x46 it stays 0x19 and the owner fires - unless unset.
    reset(0x19);
    last = 2;
    owner = owner_bytes;
    write_at(context_bytes, 0x50, uint32_t(-0x46));
    popup_wave_callback(popup, 0);
    {
        std::vector<PWaveEvent> want{
            {kPWaveIsPlaying, voice, 0},
            {kPWaveIsPlaying, bank0 + 2, 0},
            {kPWaveLoad, bank0 + 0x19, 0x19},
            {kPWavePlay, bank0 + 0x19, 0},
            {kPWaveOwner, owner_bytes, 0},
        };
        expect(g_pwave_events == want);
    }
    expect(popup->wave_index_ == 0x19);
    reset(0x19);
    last = 2;
    owner = nullptr;
    popup_wave_callback(popup, 0);
    expect(g_pwave_events.size() == 4);
    expect(g_pwave_canary_hits == 0);

    // Wave 0x2B rolls the millisecond clock: only remainder one sounds.
    reset(0x2B);
    last = 1;
    g_pwave_time_value = 5;
    popup_wave_callback(popup, 0);
    {
        std::vector<PWaveEvent> want{{kPWaveTime, nullptr, 5}};
        expect(g_pwave_events == want);
    }
    expect(last == 1);
    reset(0x2B);
    last = 1;
    g_pwave_time_value = 4;
    popup_wave_callback(popup, 0);
    {
        std::vector<PWaveEvent> want{
            {kPWaveTime, nullptr, 4},
            {kPWaveIsPlaying, voice, 0},
            {kPWaveIsPlaying, bank0 + 1, 0},
            {kPWaveLoad, bank0 + 0x2B, 0x2B},
            {kPWavePlay, bank0 + 0x2B, 0},
        };
        expect(g_pwave_events == want);
    }

    // The flag word is consulted AGAIN after the clock: a clock handler
    // that clears the bit silences the reload mid-flight.
    reset(0x2B);
    last = 1;
    g_pwave_time_value = 7;
    g_pwave_time_clears = &flags;
    popup_wave_callback(popup, 0);
    {
        std::vector<PWaveEvent> want{{kPWaveTime, nullptr, 7}};
        expect(g_pwave_events == want);
    }
    expect(last == 1);

    // Wave 0x10 chases the reload with effect 0x38, and the redirect is the
    // same callback.
    reset(0x10);
    last = 0;
    popup_wave_callback(popup, 0);
    {
        std::vector<PWaveEvent> want{
            {kPWaveIsPlaying, voice, 0},
            {kPWaveIsPlaying, bank0 + 0, 0},
            {kPWaveLoad, bank0 + 0x10, 0x10},
            {kPWavePlay, bank0 + 0x10, 0},
            {kPWaveFx, fx_bytes, 0x38},
        };
        expect(g_pwave_events == want);
    }
    reset(6);
    last = 0;
    popup_wave_callback_redirect(popup, 0);
    expect(g_pwave_events.size() == 4);
    expect(last == 6);
    expect(g_pwave_canary_hits == 0);

    PopupWaveFlags = saved_flags;
    PopupWaveContext = saved_context;
    PopupWaveVoice = saved_voice;
    PopupWaveBank = saved_bank;
    PopupWaveLastIndex = saved_last;
    PopupWaveOwnerSlot = saved_owner_slot;
    PopupWaveFx = saved_fx;
    PopupWaveIsPlaying = saved_is_playing;
    PopupWaveLoad = saved_load;
    PopupWavePlay = saved_play;
    PopupFxPlay = saved_fx_play;
    PopupWaveTimeSlot = saved_time_slot;
}

void test_sound_small_setters() {
    std::vector<uint8_t> storage(0xA0 + 32, 0);
    std::vector<uint8_t> expected(storage.size());
    uint8_t *const obj = storage.data() + 16;
    auto *sound = reinterpret_cast<Sound *>(obj);

    void *dev_vtable[64];
    struct FakeDev { void *vtbl; } fake_dev;
    fake_dev.vtbl = dev_vtable;
    auto arm_dev = [&](size_t slot, void *fn) {
        std::memset(dev_vtable, 0, sizeof(dev_vtable));
        dev_vtable[slot / 4] = fn;
        g_wave_fam_calls = 0;
        g_wave_fam_arg_log.clear();
    };
    void *own_vtable[64];
    std::memset(own_vtable, 0, sizeof(own_vtable));
    auto set32 = [&](size_t off, uint32_t v) { std::memcpy(obj + off, &v, 4); };
    auto get32 = [&](size_t off) {
        uint32_t v;
        std::memcpy(&v, obj + off, 4);
        return v;
    };
    auto setp = [&](size_t off, const void *ptr) {
        std::memcpy(obj + off, &ptr, 4);
    };
    auto getp = [&](size_t off) {
        void *ptr;
        std::memcpy(&ptr, obj + off, 4);
        return ptr;
    };

    seed_storage(storage.data(), expected.data(), storage.size());
    setp(0x00, own_vtable);
    setp(0x3C, &fake_dev);
    std::memcpy(expected.data(), storage.data(), storage.size());

    // set_volume: seven bits kept, no group rescaling, device slot 0x40.
    arm_dev(0x40, reinterpret_cast<void *>(&observe_wave_dev1));
    sound->set_volume(0x1FF);
    expect(get32(0x04) == 0x7F);
    expect(g_wave_fam_calls == 1 && g_wave_fam_self == &fake_dev &&
           g_wave_fam_args[0] == 0x7F);
    {
        const uint32_t vol = 0x7F;
        std::memcpy(expected.data() + 16 + 0x04, &vol, 4);
    }
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // set_fade: zero refused untouched; otherwise field 0x38 + device slot 0.
    arm_dev(0, reinterpret_cast<void *>(&observe_wave_dev1));
    expect(sound->set_fade(0) == 0xA);
    expect(g_wave_fam_calls == 0);
    expect_storage_bytes(storage.data(), expected.data(), storage.size());
    expect(sound->set_fade(0x777) == 0);
    expect(get32(0x38) == 0x777);
    expect(g_wave_fam_calls == 1 && g_wave_fam_args[0] == 0x777);

    // set_fade_in: the same field, device slot 0x54.
    arm_dev(0x54, reinterpret_cast<void *>(&observe_wave_dev1));
    expect(sound->set_fade_in(0) == 0xA);
    expect(g_wave_fam_calls == 0);
    expect(sound->set_fade_in(0x888) == 0);
    expect(get32(0x38) == 0x888);
    expect(g_wave_fam_calls == 1 && g_wave_fam_args[0] == 0x888);

    // Both fade setters skip the device quietly when none is wrapped.
    setp(0x3C, nullptr);
    arm_dev(0, reinterpret_cast<void *>(&observe_wave_dev1));
    expect(sound->set_fade(5) == 0);
    expect(get32(0x38) == 5 && g_wave_fam_calls == 0);
    expect(sound->set_fade_in(6) == 0);
    expect(get32(0x38) == 6 && g_wave_fam_calls == 0);
    sound->set_volume(3);
    expect(get32(0x04) == 3);

    // fade_in: the own slot 0x54 answers; only zero lets slot 0x28 follow.
    own_vtable[0x54 / 4] =
        reinterpret_cast<void *>(&observe_sound_own_slot54);
    own_vtable[0x28 / 4] = reinterpret_cast<void *>(&observe_wave_own_slot80);
    g_sound_own54_calls = 0;
    g_wave_own80_calls = 0;
    g_sound_own54_ret = 5;
    sound->fade_in(0xABC);
    expect(g_sound_own54_calls == 1 && g_sound_own54_self == sound &&
           g_sound_own54_arg == 0xABC);
    expect(g_wave_own80_calls == 0);
    g_sound_own54_ret = 0;
    sound->fade_in(0xDEF);
    expect(g_sound_own54_arg == 0xDEF);
    expect(g_wave_own80_calls == 1);

    // set_pan: the clamp, the field, the device slot 0x44.
    setp(0x3C, &fake_dev);
    arm_dev(0x44, reinterpret_cast<void *>(&observe_wave_dev1));
    sound->set_pan(-0x41);
    expect(get32(0x08) == 0xFFFFFFC0u);
    expect(g_wave_fam_args[0] == 0xFFFFFFC0u);
    sound->set_pan(-0x40);
    expect(get32(0x08) == 0xFFFFFFC0u);
    sound->set_pan(0x40);
    expect(get32(0x08) == 0x3F);
    sound->set_pan(0x3F);
    expect(get32(0x08) == 0x3F);
    sound->set_pan(-1);
    expect(get32(0x08) == 0xFFFFFFFFu);  // mid-range negatives pass through
    sound->set_pan(1);
    expect(get32(0x08) == 1);
    expect(g_wave_fam_calls == 6);

    // unload: the device's answer, the UNCONDITIONAL own slot 0x80 while the
    // device is still remembered, then the device and loaded bit clear.
    own_vtable[0x80 / 4] = reinterpret_cast<void *>(&observe_wave_own_slot80);
    arm_dev(0x14, reinterpret_cast<void *>(&observe_wave_dev0));
    set32(0x40, 0xF1);
    g_wave_own80_calls = 0;
    g_wave_fam_ret = 0x77;
    expect(sound->unload() == 0x77);
    expect(g_wave_fam_calls == 1 && g_wave_fam_self == &fake_dev);
    expect(g_wave_own80_calls == 1 && g_wave_own80_seen_device == &fake_dev);
    expect(getp(0x3C) == nullptr);
    expect(get32(0x40) == 0xF0);
    // No device: answer 0, the callback still runs.
    set32(0x40, 1);
    g_wave_own80_calls = 0;
    g_wave_fam_calls = 0;
    expect(sound->unload() == 0);
    expect(g_wave_fam_calls == 0 && g_wave_own80_calls == 1);
    expect(get32(0x40) == 0);

    // Redirect entries.
    setp(0x3C, nullptr);
    sound_set_volume_redirect(sound, nullptr, 9);
    expect(get32(0x04) == 9);
    expect(sound_set_fade_redirect(sound, nullptr, 0) == 0xA);
    expect(sound_set_fade_in_redirect(sound, nullptr, 0) == 0xA);
    g_sound_own54_ret = 5;
    g_sound_own54_calls = 0;
    sound_fade_in_arg_redirect(sound, nullptr, 2);
    expect(g_sound_own54_calls == 1);
    sound_set_pan_redirect(sound, nullptr, 1);
    expect(get32(0x08) == 1);
    g_wave_own80_calls = 0;
    expect(sound_unload_redirect(sound, nullptr) == 0);
    expect(g_wave_own80_calls == 1);
}

void test_wave_device_groups() {
    static_assert(sizeof(Wave_Device) == 0x1A4,
                  "the group table must reach slot 15's count");
    std::vector<uint8_t> storage(sizeof(Wave_Device) + 32, 0);
    std::vector<uint8_t> expected(storage.size());
    uint8_t *const obj = storage.data() + 16;
    auto *device = reinterpret_cast<Wave_Device *>(obj);

    auto *const saved_insert = WaveDeviceGroupInsert;
    auto *const saved_delete = WaveOperatorDelete;
    WaveDeviceGroupInsert = &observe_wave_group_insert;
    WaveOperatorDelete = &observe_wave_operator_delete;

    auto group_base = [](unsigned g) { return 0x24 + g * 0x18; };
    auto gset32 = [&](unsigned g, size_t off, uint32_t v) {
        std::memcpy(obj + group_base(g) + off, &v, 4);
    };
    auto gsetp = [&](unsigned g, size_t off, const void *ptr) {
        std::memcpy(obj + group_base(g) + off, &ptr, 4);
    };
    auto gget32 = [&](unsigned g, size_t off) {
        uint32_t v;
        std::memcpy(&v, obj + group_base(g) + off, 4);
        return v;
    };
    auto ggetp = [&](unsigned g, size_t off) {
        void *ptr;
        std::memcpy(&ptr, obj + group_base(g) + off, 4);
        return ptr;
    };
    uint8_t wave_a[0x6C];
    uint8_t wave_b[0x6C];
    auto *wa = reinterpret_cast<Wave *>(wave_a);
    auto *wb = reinterpret_cast<Wave *>(wave_b);
    auto wave_slot = [&](uint8_t *w) {
        uint32_t v;
        std::memcpy(&v, w + 0x68, 4);
        return v;
    };
    auto set_wave_slot = [&](uint8_t *w, uint32_t v) {
        std::memcpy(w + 0x68, &v, 4);
    };

    seed_storage(storage.data(), expected.data(), storage.size());
    std::memset(wave_a, 0, sizeof(wave_a));
    std::memset(wave_b, 0, sizeof(wave_b));

    // --- add_to_group ---
    g_wave_ginsert_calls = 0;
    expect(device->add_to_group(0x10, wa) == 0xA);
    expect(device->add_to_group(3, nullptr) == 0xA);
    expect(g_wave_ginsert_calls == 0);
    set_wave_slot(wave_a, 0xEE);
    std::memcpy(expected.data(), storage.data(), storage.size());
    expect(device->add_to_group(5, wa) == 0);
    expect(g_wave_ginsert_calls == 1);
    expect(g_wave_ginsert_head == obj + group_base(5) + 8);
    expect(g_wave_ginsert_wave == wa);
    expect(g_wave_ginsert_seen_slot == 0xEE);  // stored after the insert
    expect(wave_slot(wave_a) == 5);
    expect_storage_bytes(storage.data(), expected.data(), storage.size());
    // Slot 15 is the last accepted one.
    set_wave_slot(wave_a, 0xEE);
    expect(device->add_to_group(0xF, wa) == 0);
    expect(g_wave_ginsert_calls == 2 && wave_slot(wave_a) == 0xF);

    // --- is_group_disabled ---
    expect(device->is_group_disabled(0x10) == 1);
    obj[group_base(2)] = 0;
    expect(device->is_group_disabled(2) == 1);
    obj[group_base(2)] = 9;
    expect(device->is_group_disabled(2) == 0);
    obj[group_base(0xF)] = 1;
    expect(device->is_group_disabled(0xF) == 0);  // 15 is still in range
    obj[group_base(0xF)] = expected[16 + group_base(0xF)];
    obj[group_base(2)] = expected[16 + group_base(2)];
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // --- pull_from_group ---
    g_wave_dtor_deletes.clear();
    expect(device->pull_from_group(nullptr) == 0xA);
    set_wave_slot(wave_a, 0x10);
    expect(device->pull_from_group(wa) == 0xA);
    set_wave_slot(wave_a, 0x33);
    expect(device->pull_from_group(wa) == 0xA);

    // An empty group just forgets the slot.
    set_wave_slot(wave_a, 3);
    gsetp(3, 8, nullptr);
    expect(device->pull_from_group(wa) == 0);
    expect(wave_slot(wave_a) == 0x10);
    expect(g_wave_dtor_deletes.empty());

    // A wave whose node is not on the list walks it and forgets the slot.
    WaveGroupNode n1{};
    WaveGroupNode n2{};
    WaveGroupNode n3{};
    n1.next = &n2;
    n2.prev = &n1;
    n2.wave = wb;
    set_wave_slot(wave_a, 4);
    gsetp(4, 8, &n1);
    expect(device->pull_from_group(wa) == 0);
    expect(wave_slot(wave_a) == 0x10);
    expect(g_wave_dtor_deletes.empty());
    expect(n1.next == &n2 && n2.prev == &n1);

    // Found in the middle: neighbours re-linked, cursor on the follower,
    // head and tail untouched, node freed, count down one.
    n1 = WaveGroupNode{};
    n2 = WaveGroupNode{};
    n3 = WaveGroupNode{};
    n1.next = &n2;
    n1.wave = wb;
    n2.prev = &n1;
    n2.next = &n3;
    n2.wave = wa;
    n3.prev = &n2;
    n3.wave = wb;
    set_wave_slot(wave_a, 4);
    gsetp(4, 8, &n1);
    gsetp(4, 0xC, &n3);
    gsetp(4, 0x10, reinterpret_cast<void *>(0x7777));
    gset32(4, 0x14, 7);
    g_wave_delete_watch =
        reinterpret_cast<uint32_t *>(obj + group_base(4) + 0x14);
    std::memcpy(expected.data(), storage.data(), storage.size());
    expect(device->pull_from_group(wa) == 0);
    // The node is freed BEFORE the count drops; the free observer saw 7.
    expect(g_wave_delete_watch_seen == 7);
    g_wave_delete_watch = nullptr;
    expect(n1.next == &n3 && n3.prev == &n1);
    expect(ggetp(4, 8) == &n1 && ggetp(4, 0xC) == &n3);
    expect(ggetp(4, 0x10) == &n3);
    expect(gget32(4, 0x14) == 6);
    expect(g_wave_dtor_deletes.size() == 1 &&
           g_wave_dtor_deletes[0] == &n2);
    expect(wave_slot(wave_a) == 0x10);
    {
        uint8_t *const eobj = expected.data() + 16;
        void *cur = &n3;
        const uint32_t cnt = 6;
        std::memcpy(eobj + group_base(4) + 0x10, &cur, 4);
        std::memcpy(eobj + group_base(4) + 0x14, &cnt, 4);
    }
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // Found at the head: the head advances, the follower loses its prev.
    n1 = WaveGroupNode{};
    n2 = WaveGroupNode{};
    n1.next = &n2;
    n1.wave = wa;
    n2.prev = &n1;
    n2.wave = wb;
    set_wave_slot(wave_a, 4);
    gsetp(4, 8, &n1);
    g_wave_dtor_deletes.clear();
    expect(device->pull_from_group(wa) == 0);
    expect(ggetp(4, 8) == &n2 && n2.prev == nullptr);
    expect(ggetp(4, 0x10) == &n2);
    expect(g_wave_dtor_deletes.size() == 1 &&
           g_wave_dtor_deletes[0] == &n1);

    // Found at the tail: the cursor clears and the tail steps back.
    n1 = WaveGroupNode{};
    n2 = WaveGroupNode{};
    n1.next = &n2;
    n1.wave = wb;
    n2.prev = &n1;
    n2.wave = wa;
    set_wave_slot(wave_a, 4);
    gsetp(4, 8, &n1);
    gsetp(4, 0xC, &n2);
    gsetp(4, 0x10, reinterpret_cast<void *>(0x8888));
    g_wave_dtor_deletes.clear();
    expect(device->pull_from_group(wa) == 0);
    expect(n1.next == nullptr);
    expect(ggetp(4, 0x10) == nullptr && ggetp(4, 0xC) == &n1);
    expect(g_wave_dtor_deletes.size() == 1 &&
           g_wave_dtor_deletes[0] == &n2);

    // A follower that IS the cursor field: the neighbour store lands on the
    // cursor first and the cursor store overwrites it second, so their order
    // is visible in which value survives.
    n1 = WaveGroupNode{};
    n2 = WaveGroupNode{};
    n1.next = &n2;
    n1.wave = wb;
    n2.prev = &n1;
    n2.next = reinterpret_cast<WaveGroupNode *>(obj + group_base(4) + 0x10);
    n2.wave = wa;
    set_wave_slot(wave_a, 4);
    gsetp(4, 8, &n1);
    g_wave_dtor_deletes.clear();
    expect(device->pull_from_group(wa) == 0);
    expect(n1.next == n2.next);
    // next->prev wrote &n1 into the cursor field, then the cursor store
    // replaced it with the follower's address.
    expect(ggetp(4, 0x10) == obj + group_base(4) + 0x10);
    expect(g_wave_dtor_deletes.size() == 1 &&
           g_wave_dtor_deletes[0] == &n2);

    // The only node: head, tail, and cursor all empty out.
    n1 = WaveGroupNode{};
    n1.wave = wa;
    set_wave_slot(wave_a, 4);
    gsetp(4, 8, &n1);
    gsetp(4, 0xC, &n1);
    g_wave_dtor_deletes.clear();
    expect(device->pull_from_group(wa) == 0);
    expect(ggetp(4, 8) == nullptr && ggetp(4, 0xC) == nullptr &&
           ggetp(4, 0x10) == nullptr);
    expect(g_wave_dtor_deletes.size() == 1);

    // --- the recovered insert helper itself ---
    auto *const saved_new2 = WaveOperatorNew;
    WaveOperatorNew = &observe_wave_operator_new;
    auto *arena_node = reinterpret_cast<WaveGroupNode *>(g_wave_new_arena);

    // Empty list: the node becomes both ends.
    WaveGroupList list{};
    list.count = 5;
    list.cursor = reinterpret_cast<WaveGroupNode *>(0x9999);
    g_wave_new_calls = 0;
    g_wave_new_fails = false;
    wave_group_insert_redirect(&list, nullptr, wa);
    expect(g_wave_new_calls == 1 && g_wave_new_size == 0xC);
    expect(list.head == arena_node && list.tail == arena_node);
    expect(list.count == 6);
    expect(list.cursor == reinterpret_cast<WaveGroupNode *>(0x9999));
    expect(arena_node->prev == nullptr && arena_node->next == nullptr &&
           arena_node->wave == wa);

    // Live tail: append after it; the new node's prev is the re-read tail.
    WaveGroupNode t1{};
    list.head = &t1;
    list.tail = &t1;
    list.count = 1;
    wave_group_insert_redirect(&list, nullptr, wb);
    expect(t1.next == arena_node);
    expect(arena_node->prev == &t1 && arena_node->next == nullptr &&
           arena_node->wave == wb);
    expect(list.head == &t1 && list.tail == arena_node && list.count == 2);

    // A tail aimed at the list itself: the old tail's next write lands ON
    // the tail field, so the re-read prev is the new node - the write
    // order is visible in which value the prev holds.
    list = WaveGroupList{};
    list.tail = reinterpret_cast<WaveGroupNode *>(&list);
    wave_group_insert_redirect(&list, nullptr, wa);
    expect(arena_node->prev == arena_node);
    expect(list.tail == arena_node);

    // A failed allocation on an empty list still counts the phantom node.
    list = WaveGroupList{};
    list.count = 9;
    g_wave_new_fails = true;
    wave_group_insert_redirect(&list, nullptr, wa);
    g_wave_new_fails = false;
    expect(list.head == nullptr && list.tail == nullptr);
    expect(list.count == 10);
    WaveOperatorNew = saved_new2;

    // Redirect entries.
    expect(wave_device_add_to_group_redirect(device, nullptr, 0x10, wa) ==
           0xA);
    expect(wave_device_is_group_disabled_redirect(device, nullptr, 0x10) ==
           1);
    expect(wave_device_pull_from_group_redirect(device, nullptr, nullptr) ==
           0xA);

    WaveDeviceGroupInsert = saved_insert;
    WaveOperatorDelete = saved_delete;
}

void test_sound_set_type_and_load() {
    std::vector<uint8_t> storage(0xA0 + 32, 0);
    std::vector<uint8_t> expected(storage.size());
    uint8_t *const obj = storage.data() + 16;
    auto *sound = reinterpret_cast<Sound *>(obj);
    g_wave_dtor_obj = obj;

    auto set32 = [&](size_t off, uint32_t v) { std::memcpy(obj + off, &v, 4); };
    auto get32 = [&](size_t off) {
        uint32_t v;
        std::memcpy(&v, obj + off, 4);
        return v;
    };
    auto setp = [&](size_t off, const void *ptr) {
        std::memcpy(obj + off, &ptr, 4);
    };
    auto getp = [&](size_t off) {
        void *ptr;
        std::memcpy(&ptr, obj + off, 4);
        return ptr;
    };

    // --- set_type: the whole jump table, plus the invalid arms (3 routes
    // to the invalid arm despite being inside the table's range) ---
    seed_storage(storage.data(), expected.data(), storage.size());
    const struct { uint32_t type; uint32_t bit; } type_map[] = {
        {1, 4}, {2, 8}, {4, 0x10}, {5, 0x28}, {6, 0x100}, {7, 0x80},
    };
    const uint32_t base = 0x60000201u;
    for (const auto &c : type_map) {
        set32(0x40, base);
        set32(0x50, 0xEEEEEEEEu);
        sound->set_type(c.type);
        expect(get32(0x50) == c.type);
        expect(get32(0x40) == (base | c.bit));
    }
    const uint32_t invalid_types[] = {0, 3, 8, 0x7FFFFFFFu};
    for (const uint32_t t : invalid_types) {
        set32(0x40, base);
        set32(0x50, 0xEEEEEEEEu);
        sound->set_type(t);
        expect(get32(0x50) == 0);
        expect(get32(0x40) == base);
    }
    set32(0x40, base);
    set32(0x50, 0);
    std::memcpy(expected.data(), storage.data(), storage.size());
    sound->set_type(6);
    {
        uint8_t *const eobj = expected.data() + 16;
        const uint32_t t6 = 6;
        const uint32_t f6 = base | 0x100;
        std::memcpy(eobj + 0x50, &t6, 4);
        std::memcpy(eobj + 0x40, &f6, 4);
    }
    expect_storage_bytes(storage.data(), expected.data(), storage.size());
    sound_set_type_redirect(sound, nullptr, 2);
    expect(get32(0x50) == 2);

    // --- load ---
    auto *const saved_delete = WaveOperatorDelete;
    auto *const saved_new = WaveOperatorNew;
    auto **const saved_create = WaveDeviceCreateSlot;
    int *const saved_guard = WaveDeviceReleaseGuard;
    func_wave_device_create *create_fn = &observe_wave_device_create;
    int guard = 1;
    WaveOperatorDelete = &observe_wave_operator_delete;
    WaveOperatorNew = &observe_wave_operator_new;
    WaveDeviceCreateSlot = &create_fn;
    WaveDeviceReleaseGuard = &guard;

    void *dev_vtable[64];
    std::memset(dev_vtable, 0, sizeof(dev_vtable));
    dev_vtable[0x60 / 4] = reinterpret_cast<void *>(&observe_wave_dev0);
    dev_vtable[0x10 / 4] = reinterpret_cast<void *>(&observe_wave_dev1);
    dev_vtable[0x48 / 4] = reinterpret_cast<void *>(&observe_wave_dev1);
    struct FakeDev { void *vtbl; } fake_dev;
    fake_dev.vtbl = dev_vtable;
    void *own_vtable[64];
    std::memset(own_vtable, 0, sizeof(own_vtable));
    own_vtable[0x7C / 4] = reinterpret_cast<void *>(&observe_wave_own_slot7C);
    setp(0x00, own_vtable);

    auto reset_sload = [&] {
        filefind_get_calls = 0;
        g_wave_new_calls = g_wave_create_calls = 0;
        g_wave_own7C_calls = g_wave_fam_calls = 0;
        g_wave_fam_arg_log.clear();
        g_wave_dtor_deletes.clear();
        g_wave_dtor_delete_seen_slot.clear();
        g_wave_create_installs = nullptr;
        g_wave_create_ret = 0;
    };
    char name_arg[] = "menu.wav";
    char resolved_buf[] = "snd/menu.wav";

    // Unresolvable: 0xA, and nothing else was even consulted.
    reset_sload();
    filefind_get_result = nullptr;
    expect(sound->load(name_arg) == 0xA);
    expect(filefind_get_calls == 1 &&
           filefind_get_request == static_cast<LPCSTR>(name_arg));
    expect(g_wave_create_calls == 0 && g_wave_new_calls == 0);

    // Resolvable but the hook guard is dead: 1, after the resolution.
    reset_sload();
    filefind_get_result = resolved_buf;
    guard = 0;
    expect(sound->load(name_arg) == 1);
    expect(filefind_get_calls == 1 && g_wave_create_calls == 0);
    guard = 1;

    // A failing creation propagates.
    setp(0x3C, nullptr);
    reset_sload();
    g_wave_create_ret = 0x2E;
    expect(sound->load(name_arg) == 0x2E);
    expect(g_wave_create_calls == 1 && g_wave_create_mode == 1 &&
           g_wave_create_name == static_cast<LPCSTR>(resolved_buf));

    // First successful load through a freshly created device: loaded bit
    // set BEFORE the sound's own 0x7C, loop started, name remembered.
    setp(0x4C, nullptr);
    set32(0x40, 0xF0);
    set32(0x30, 3);
    reset_sload();
    g_wave_create_installs = &fake_dev;
    g_wave_fam_ret = 0;
    // Scrub the shared arena: earlier tests left the same resolved string in
    // it, which would mask a dropped copy.
    std::memset(g_wave_new_arena, 0x5A, sizeof(g_wave_new_arena));
    std::memcpy(expected.data(), storage.data(), storage.size());
    expect(sound->load(name_arg) == 0);
    expect(getp(0x3C) == &fake_dev);
    expect(get32(0x40) == 0xF1);
    expect(g_wave_own7C_calls == 1 && g_wave_own7C_seen_flags == 0xF1);
    expect(g_wave_fam_arg_log ==
           std::vector<uint32_t>(
               {static_cast<uint32_t>(
                    reinterpret_cast<uintptr_t>(resolved_buf)),
                1}));
    expect(g_wave_new_calls == 1 && g_wave_new_size == 13);
    expect(getp(0x4C) == g_wave_new_arena);
    expect(std::strcmp(g_wave_new_arena, "snd/menu.wav") == 0);
    expect(g_wave_dtor_deletes.empty());
    {
        uint8_t *const eobj = expected.data() + 16;
        void *dev = &fake_dev;
        const uint32_t f = 0xF1;
        void *arena = g_wave_new_arena;
        std::memcpy(eobj + 0x3C, &dev, 4);
        std::memcpy(eobj + 0x40, &f, 4);
        std::memcpy(eobj + 0x4C, &arena, 4);
    }
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // A busy existing device answers 0xF before any load.
    reset_sload();
    g_wave_fam_ret = 7;
    expect(sound->load(name_arg) == 0xF);
    expect(g_wave_fam_calls == 1 && g_wave_fam_arg_log.empty());

    // A failing load on an already-loaded sound clears the loaded bit and
    // still replaces the remembered name - new copy first, old freed after.
    reset_sload();
    g_wave_fam_ret = 0;  // not busy...
    // ...but the load answer comes from the same shared return; use the
    // arg-log to tell the calls apart and drive the answer per call through
    // the busy observer being armed to return 0 and the load to fail: the
    // shared return cannot differ per slot, so run the failing load against
    // a sound with NO device instead (fresh creation, failing load).
    setp(0x3C, nullptr);
    set32(0x40, 0xF1);
    reset_sload();
    g_wave_create_installs = &fake_dev;
    g_wave_fam_ret = 0x33;
    expect(sound->load(name_arg) == 0x33);
    expect(get32(0x40) == 0xF0);
    expect(g_wave_own7C_calls == 0);
    expect(g_wave_dtor_deletes.size() == 1 &&
           g_wave_dtor_deletes[0] == g_wave_new_arena);
    expect(getp(0x4C) == g_wave_new_arena);

    // Success with the loaded bit already set: no protocol, no loop.
    set32(0x40, 1);
    reset_sload();
    g_wave_fam_ret = 0;
    expect(sound->load(name_arg) == 0);
    expect(g_wave_own7C_calls == 0);
    expect(get32(0x40) == 1);

    // Success, bit clear, but a zero loop dword: the 0x7C protocol without
    // the loop start.
    set32(0x40, 0);
    set32(0x30, 0);
    reset_sload();
    expect(sound->load(name_arg) == 0);
    expect(g_wave_own7C_calls == 1);
    // busy + load dispatched, and only the load logs an argument - the loop
    // call would have logged a second.
    expect(g_wave_fam_calls == 2 && g_wave_fam_arg_log.size() == 1);
    expect(get32(0x40) == 1);

    // Redirect entry.
    reset_sload();
    filefind_get_result = nullptr;
    expect(sound_load_redirect(sound, nullptr, name_arg) == 0xA);

    WaveOperatorDelete = saved_delete;
    WaveOperatorNew = saved_new;
    WaveDeviceCreateSlot = saved_create;
    WaveDeviceReleaseGuard = saved_guard;
}

void test_ambience_construct() {
    // Same shape as test_wave_ctor above and deliberately checked against it:
    // Ambience differs from Wave in the final vtable, the flag dword's net
    // value, the WIDTH of the 0x54 clear, and the set_type argument. The
    // storage runs past the class so the "nothing at or above 0x58" claim -
    // which is what pins the shared base's extent - has somewhere to fail.
    std::vector<uint8_t> storage(sizeof(Ambience) + 32, 0);
    std::vector<uint8_t> expected(storage.size());
    uint8_t *const obj = storage.data() + 16;
    auto *ambience = reinterpret_cast<Ambience *>(obj);

    auto *const saved_stype = SoundSetType;
    SoundSetType = &observe_wave_set_type;

    for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
        seed_storage(storage.data(), expected.data(), storage.size());
        uint32_t pre_tail;
        std::memcpy(&pre_tail, obj + 0x54, 4);
        std::memcpy(expected.data(), storage.data(), storage.size());
        g_wave_stype_calls = 0;
        if (use_adapter) {
            expect(ambience_construct_redirect(ambience, nullptr) == ambience);
        } else {
            ambience->construct();
        }
        expect(g_wave_stype_calls == 1);
        expect(g_wave_stype_wave == reinterpret_cast<Wave *>(ambience));
        // 5, where Wave passes 1.
        expect(g_wave_stype_type == 5);
        // At set_type time the final vtable is already up and both flags have
        // been read back, or-ed and stored - the call is the last thing the
        // body does, so nothing follows it to disturb them.
        expect(g_wave_stype_seen_vtable == 0x0066E664u);
        // 8, where Wave nets 4.
        expect(g_wave_stype_seen_f40 == 8);
        expect(g_wave_stype_seen_f54 == 1);
        // The dword at 0x58 still holds its seed: this constructor writes
        // nothing at or above it.
        expect(g_wave_stype_seen_pitch ==
               *reinterpret_cast<const uint32_t *>(
                   expected.data() + 16 + 0x58));

        uint8_t *const eobj = expected.data() + 16;
        auto e32 = [&](size_t off, uint32_t v) { std::memcpy(eobj + off, &v, 4); };
        e32(0x00, 0x0066E664u);
        e32(0x04, 0x7F);
        e32(0x08, 0);
        std::memset(eobj + 0x0C, 0, 0x24);
        e32(0x30, 0);
        // 0x34 is never written and keeps its seed, exactly as in Wave.
        e32(0x38, 0x3E8);
        e32(0x3C, 0);
        e32(0x40, 8);
        e32(0x44, 0);
        e32(0x48, 0);
        e32(0x4C, 0);
        e32(0x50, 0);
        // ONE byte at 0x54. The three bytes above it keep their seed, which a
        // widened clear - Wave's four-byte shape - would destroy.
        eobj[0x54] = 1;
        expect_storage_bytes(storage.data(), expected.data(), storage.size());
        // Restated as its own assertion so the reason is not buried in the
        // byte compare: 0x55..0x57 are untouched.
        expect((pre_tail >> 8) ==
               (*reinterpret_cast<const uint32_t *>(obj + 0x54) >> 8));
    }

    SoundSetType = saved_stype;
}

void test_wave_ctor() {
    std::vector<uint8_t> storage(sizeof(Wave) + 32, 0);
    std::vector<uint8_t> expected(storage.size());
    uint8_t *const obj = storage.data() + 16;
    auto *wave = reinterpret_cast<Wave *>(obj);

    auto *const saved_stype = SoundSetType;
    SoundSetType = &observe_wave_set_type;

    seed_storage(storage.data(), expected.data(), storage.size());
    uint32_t pre_pitch;
    std::memcpy(&pre_pitch, obj + 0x58, 4);
    std::memcpy(expected.data(), storage.data(), storage.size());
    g_wave_stype_calls = 0;
    expect(wave_ctor_redirect(wave, nullptr) == wave);
    expect(g_wave_stype_calls == 1 && g_wave_stype_wave == wave);
    expect(g_wave_stype_type == 1);
    // At set_type time the final vtable is up, the flag dword reads 4, the
    // 0x54 byte is cleared, and the pitch dword still holds its seed - the
    // tail stores run after the call.
    expect(g_wave_stype_seen_vtable == 0x0066E44Cu);
    expect(g_wave_stype_seen_f40 == 4);
    expect(g_wave_stype_seen_f54 == 0);
    expect(g_wave_stype_seen_pitch == pre_pitch);
    // Full end state, byte for byte; only the dword at 0x34 keeps its seed.
    uint8_t *const eobj = expected.data() + 16;
    auto e32 = [&](size_t off, uint32_t v) { std::memcpy(eobj + off, &v, 4); };
    e32(0x00, 0x0066E44Cu);
    e32(0x04, 0x7F);
    e32(0x08, 0);
    std::memset(eobj + 0x0C, 0, 0x24);
    e32(0x30, 0);
    e32(0x38, 0x3E8);
    e32(0x3C, 0);
    e32(0x40, 4);
    e32(0x44, 0);
    e32(0x48, 0);
    e32(0x4C, 0);
    e32(0x50, 0);
    e32(0x54, 0);
    e32(0x58, 0);
    e32(0x5C, 0x3F800000u);
    e32(0x60, 0);
    e32(0x64, 0);
    e32(0x68, 0x10);
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    SoundSetType = saved_stype;
}

void test_wave_destructor() {
    static_assert(sizeof(Wave) == 0x6C, "Wave tests require the legacy layout");
    std::vector<uint8_t> storage(sizeof(Wave) + 32, 0);
    std::vector<uint8_t> expected(storage.size());
    uint8_t *const obj = storage.data() + 16;
    auto *wave = reinterpret_cast<Wave *>(obj);
    g_wave_dtor_obj = obj;

    auto *const saved_pull = WaveDevicePullFromGroup;
    void *const saved_dev_global = WaveDeviceGlobal;
    auto *const saved_delete = WaveOperatorDelete;
    auto **const saved_release_slot = WaveDeviceReleaseSlot;
    int *const saved_guard = WaveDeviceReleaseGuard;
    Wave **const saved_head = WaveChainHead;
    Wave **const saved_tail = WaveChainTail;

    int fake_device_singleton = 0;
    func_wave_device_release *release_fn = &observe_wave_release;
    int release_guard = 1;
    Wave *chain_head = nullptr;
    Wave *chain_tail = nullptr;
    WaveDevicePullFromGroup = &observe_wave_pull_from_group;
    WaveDeviceGlobal = &fake_device_singleton;
    WaveOperatorDelete = &observe_wave_operator_delete;
    WaveDeviceReleaseSlot = &release_fn;
    WaveDeviceReleaseGuard = &release_guard;
    WaveChainHead = &chain_head;
    WaveChainTail = &chain_tail;

    auto set32 = [&](size_t off, uint32_t v) { std::memcpy(obj + off, &v, 4); };
    auto setp = [&](size_t off, const void *p) {
        std::memcpy(obj + off, &p, 4);
    };
    auto get32 = [&](size_t off) {
        uint32_t v;
        std::memcpy(&v, obj + off, 4);
        return v;
    };
    auto getp = [&](size_t off) {
        void *p;
        std::memcpy(&p, obj + off, 4);
        return p;
    };
    auto node_ptr = [](uint8_t *node, size_t off) {
        void *p;
        std::memcpy(&p, node + off, 4);
        return p;
    };
    auto reset_counters = [&] {
        g_wave_dtor_pull_calls = g_wave_dtor_release_calls = 0;
        g_wave_dtor_deletes.clear();
        g_wave_dtor_delete_seen_slot.clear();
        g_wave_dtor_release_relinks = false;
    };
    // The fields the destructor always leaves behind, applied to the expected
    // snapshot: the ultimate base vtable, a null device/prev/next/buffer, and
    // the final flag dword.
    auto apply_end_state = [&](uint32_t final_flags) {
        uint8_t *const eobj = expected.data() + 16;
        const uint32_t vt = 0x0066E444u;
        const uint32_t zero = 0;
        std::memcpy(eobj + 0x00, &vt, 4);
        std::memcpy(eobj + 0x3C, &zero, 4);
        std::memcpy(eobj + 0x40, &final_flags, 4);
        std::memcpy(eobj + 0x44, &zero, 4);
        std::memcpy(eobj + 0x48, &zero, 4);
        std::memcpy(eobj + 0x4C, &zero, 4);
    };

    // Scenario 1: every primary path at once. Group slot 0xF (< 0x10) pulls
    // the wave from the device singleton's group while the wave's OWN vtable
    // is installed; the buffer is freed exactly once (the unconditional clear
    // keeps the inlined base free from seeing it again); both chain
    // neighbours are real nodes, so the chain end slots stay untouched; the
    // release hook runs over the wrapped device with the MIDDLE vtable
    // installed; only bit 1 of the flag dword is cleared.
    uint8_t node_prev[0x6C];
    uint8_t node_next[0x6C];
    std::memset(node_prev, 0x66, sizeof(node_prev));
    std::memset(node_next, 0x77, sizeof(node_next));
    uint8_t buffer_block[8];
    int fake_wrapped_device = 0;
    seed_storage(storage.data(), expected.data(), storage.size());
    set32(0x68, 0x0F);
    setp(0x4C, buffer_block);
    set32(0x40, 0xA5A5A5F3u);
    setp(0x44, node_prev);
    setp(0x48, node_next);
    setp(0x3C, &fake_wrapped_device);
    chain_head = reinterpret_cast<Wave *>(0x1111);
    chain_tail = reinterpret_cast<Wave *>(0x2222);
    release_guard = 1;
    reset_counters();
    std::memcpy(expected.data(), storage.data(), storage.size());
    wave_dtor_redirect(wave, nullptr);
    expect(g_wave_dtor_pull_calls == 1);
    expect(g_wave_dtor_pull_dev == &fake_device_singleton);
    expect(g_wave_dtor_pull_wave == wave);
    expect(g_wave_dtor_pull_seen_vtable == 0x0066E44Cu);
    expect(g_wave_dtor_deletes.size() == 1 &&
           g_wave_dtor_deletes[0] == buffer_block);
    expect(g_wave_dtor_delete_seen_slot.size() == 1 &&
           g_wave_dtor_delete_seen_slot[0] == buffer_block);
    expect(node_ptr(node_prev, 0x48) == node_next);
    expect(node_ptr(node_next, 0x44) == node_prev);
    expect(chain_head == reinterpret_cast<Wave *>(0x1111));
    expect(chain_tail == reinterpret_cast<Wave *>(0x2222));
    expect(g_wave_dtor_release_calls == 1 &&
           g_wave_dtor_release_dev == &fake_wrapped_device);
    expect(g_wave_dtor_release_seen_vtable == 0x0066E3C0u);
    apply_end_state(0xA5A5A5F1u);
    expect_storage_bytes(storage.data(), expected.data(), storage.size());
    expect(get32(0x68) == 0x0F);

    // Scenario 2: the two normally-dead base-destructor arms. The chain prev
    // is aimed 4 bytes into the wave itself, so unlink #1's neighbour write
    // (prev->next at +0x48) lands on the wave's OWN buffer slot at +0x4C and
    // re-populates it with the next pointer - the inlined base free then
    // fires on that block. The release hook re-arms the chain bit with a null
    // prev, so unlink #2 takes the head-slot arm. Group slot exactly 0x10 is
    // the boundary: NOT pulled.
    uint8_t late_block[0x6C];
    uint8_t node_q[0x6C];
    std::memset(late_block, 0x33, sizeof(late_block));
    std::memset(node_q, 0x77, sizeof(node_q));
    seed_storage(storage.data(), expected.data(), storage.size());
    set32(0x68, 0x10);
    setp(0x4C, nullptr);
    set32(0x40, 2);
    setp(0x44, obj + 4);
    setp(0x48, late_block);
    setp(0x3C, &fake_wrapped_device);
    chain_head = reinterpret_cast<Wave *>(0x1111);
    chain_tail = reinterpret_cast<Wave *>(0x2222);
    release_guard = 1;
    reset_counters();
    g_wave_dtor_release_relinks = true;
    g_wave_dtor_release_relink_prev = nullptr;
    g_wave_dtor_release_relink_next = node_q;
    std::memcpy(expected.data(), storage.data(), storage.size());
    wave_dtor_redirect(wave, nullptr);
    expect(g_wave_dtor_pull_calls == 0);
    expect(g_wave_dtor_deletes.size() == 1 &&
           g_wave_dtor_deletes[0] == late_block);
    expect(g_wave_dtor_delete_seen_slot.size() == 1 &&
           g_wave_dtor_delete_seen_slot[0] == late_block);
    expect(node_ptr(late_block, 0x44) == obj + 4);  // next->prev in unlink #1
    expect(g_wave_dtor_release_calls == 1);
    expect(g_wave_dtor_release_seen_vtable == 0x0066E3C0u);
    expect(chain_head == reinterpret_cast<Wave *>(node_q));  // unlink #2 head arm
    expect(chain_tail == reinterpret_cast<Wave *>(0x2222));
    expect(node_ptr(node_q, 0x44) == nullptr);  // next->prev wrote the null prev
    apply_end_state(0);
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // Scenario 3: unlink #1's head arm. A null prev routes the next pointer
    // into the chain head slot; no buffer, no device, no pull - and the
    // buffer slot is still cleared unconditionally.
    uint8_t node_s[0x6C];
    std::memset(node_s, 0x55, sizeof(node_s));
    seed_storage(storage.data(), expected.data(), storage.size());
    set32(0x68, 0x45);
    setp(0x4C, nullptr);
    set32(0x40, 0x13);
    setp(0x44, nullptr);
    setp(0x48, node_s);
    setp(0x3C, nullptr);
    chain_head = reinterpret_cast<Wave *>(0x1111);
    chain_tail = reinterpret_cast<Wave *>(0x2222);
    reset_counters();
    std::memcpy(expected.data(), storage.data(), storage.size());
    wave_dtor_redirect(wave, nullptr);
    expect(g_wave_dtor_pull_calls == 0);
    expect(g_wave_dtor_deletes.empty());
    expect(g_wave_dtor_release_calls == 0);
    expect(chain_head == reinterpret_cast<Wave *>(node_s));
    expect(chain_tail == reinterpret_cast<Wave *>(0x2222));
    expect(node_ptr(node_s, 0x44) == nullptr);
    apply_end_state(0x11);
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // Scenario 4: unlink #1's tail arm, and a dead release guard. A null next
    // routes the prev pointer into the chain tail slot; the guard suppresses
    // the release hook but the device is still forgotten.
    uint8_t node_t[0x6C];
    std::memset(node_t, 0x66, sizeof(node_t));
    uint8_t block_d[8];
    seed_storage(storage.data(), expected.data(), storage.size());
    set32(0x68, 0);
    setp(0x4C, block_d);
    set32(0x40, 2);
    setp(0x44, node_t);
    setp(0x48, nullptr);
    setp(0x3C, &fake_wrapped_device);
    chain_head = reinterpret_cast<Wave *>(0x1111);
    chain_tail = reinterpret_cast<Wave *>(0x2222);
    release_guard = 0;
    reset_counters();
    std::memcpy(expected.data(), storage.data(), storage.size());
    wave_dtor_redirect(wave, nullptr);
    expect(g_wave_dtor_pull_calls == 1);  // slot 0 also counts as grouped
    expect(g_wave_dtor_deletes.size() == 1 &&
           g_wave_dtor_deletes[0] == block_d);
    expect(g_wave_dtor_delete_seen_slot.size() == 1 &&
           g_wave_dtor_delete_seen_slot[0] == block_d);
    expect(node_ptr(node_t, 0x48) == nullptr);
    expect(chain_head == reinterpret_cast<Wave *>(0x1111));
    expect(chain_tail == reinterpret_cast<Wave *>(node_t));
    expect(g_wave_dtor_release_calls == 0);
    expect(getp(0x3C) == nullptr);
    apply_end_state(0);
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    // Scenario 5: unlink #2's tail arm, reached with the chain bit initially
    // CLEAR - unlink #1 must not run (its would-be neighbour stays pristine),
    // and only the release hook's re-link arms the second pass.
    uint8_t poison_node[0x6C];
    uint8_t poison_expected[0x6C];
    uint8_t node_r[0x6C];
    std::memset(poison_node, 0x44, sizeof(poison_node));
    std::memcpy(poison_expected, poison_node, sizeof(poison_node));
    std::memset(node_r, 0x88, sizeof(node_r));
    uint8_t block_e[8];
    seed_storage(storage.data(), expected.data(), storage.size());
    set32(0x68, 0x10);
    setp(0x4C, block_e);
    set32(0x40, 0x11);
    setp(0x44, poison_node);
    setp(0x48, poison_node);
    setp(0x3C, &fake_wrapped_device);
    chain_head = reinterpret_cast<Wave *>(0x1111);
    chain_tail = reinterpret_cast<Wave *>(0x2222);
    release_guard = 1;
    reset_counters();
    g_wave_dtor_release_relinks = true;
    g_wave_dtor_release_relink_prev = node_r;
    g_wave_dtor_release_relink_next = nullptr;
    std::memcpy(expected.data(), storage.data(), storage.size());
    wave_dtor_redirect(wave, nullptr);
    expect(g_wave_dtor_deletes.size() == 1 &&
           g_wave_dtor_deletes[0] == block_e);
    expect(g_wave_dtor_delete_seen_slot.size() == 1 &&
           g_wave_dtor_delete_seen_slot[0] == block_e);
    expect(std::memcmp(poison_node, poison_expected, sizeof(poison_node)) ==
           0);
    expect(g_wave_dtor_release_calls == 1);
    expect(node_ptr(node_r, 0x48) == nullptr);  // prev->next in unlink #2
    expect(chain_head == reinterpret_cast<Wave *>(0x1111));
    expect(chain_tail == reinterpret_cast<Wave *>(node_r));
    apply_end_state(0x11);
    expect_storage_bytes(storage.data(), expected.data(), storage.size());

    WaveDevicePullFromGroup = saved_pull;
    WaveDeviceGlobal = saved_dev_global;
    WaveOperatorDelete = saved_delete;
    WaveDeviceReleaseSlot = saved_release_slot;
    WaveDeviceReleaseGuard = saved_guard;
    WaveChainHead = saved_head;
    WaveChainTail = saved_tail;
}

void test_delegating_closers() {
    // Popup::close has 104 callers, the most of anything left. Its Scroll sits
    // at 0x3230 - exactly sizeof(BasePop) - and Scroll::close is already
    // recovered, so only the base close is reached through a seam.
    // Popup::close is held back from this batch. It runs the real
    // Scroll::close on its member, which needs the whole Scroll fixture -
    // two default tables, three seams and two vtables - and rebuilding that
    // inside this test duplicates the Scroll suite rather than reusing it.
    // The recovery is written and correct by disassembly; it waits on that
    // fixture becoming a shared helper.

    // PopMenu::init passes `this` straight through with two zero arguments,
    // and hands back whatever the base returns.
    auto *const saved_init = BasePopOriginalInit;
    BasePopOriginalInit = &observe_base_pop_init;
    std::vector<uint8_t> pm(sizeof(PopMenu) + 32);
    std::vector<uint8_t> pm_want(pm.size());
    auto *menu = reinterpret_cast<PopMenu *>(pm.data() + 16);
    seed_storage(pm.data(), pm_want.data(), pm.size());
    std::memcpy(pm_want.data(), pm.data(), pm.size());
    g_base_pop_init_result = 0x7FFFFFFF;
    expect(menu->init() == 0x7FFFFFFF);
    expect(reinterpret_cast<void *>(g_inited_base_pop) ==
           reinterpret_cast<void *>(menu));
    expect(g_init_a == 0);
    expect(g_init_b == 0);
    g_base_pop_init_result = -1;
    expect(pop_menu_init_redirect(menu, nullptr) == -1);
    expect_storage_bytes(pm.data(), pm_want.data(), pm.size());
    BasePopOriginalInit = saved_init;

    // InfoWin::reset is guarded: it must clear the flag *before* dispatching,
    // and must not dispatch at all when the flag is already clear.
    auto *const saved_timer = InfoWinOriginalTimerProc;
    InfoWinOriginalTimerProc = &observe_timer_proc;
    std::vector<uint8_t> iw(sizeof(InfoWin) + 32);
    std::vector<uint8_t> iw_want(iw.size());
    auto *info = reinterpret_cast<InfoWin *>(iw.data() + 16);
    auto set_flag = [&](int32_t value) {
        std::memcpy(iw.data() + 16 + 0xA8, &value, sizeof(value));
    };
    auto flag = [&] {
        int32_t value = 0;
        std::memcpy(&value, iw.data() + 16 + 0xA8, sizeof(value));
        return value;
    };

    seed_storage(iw.data(), iw_want.data(), iw.size());
    set_flag(0);
    std::memcpy(iw_want.data(), iw.data(), iw.size());
    g_timer_calls = 0;
    info->reset();
    expect(g_timer_calls == 0);
    expect_storage_bytes(iw.data(), iw_want.data(), iw.size());

    seed_storage(iw.data(), iw_want.data(), iw.size());
    set_flag(0x1234);
    std::memcpy(iw_want.data(), iw.data(), iw.size());
    g_timer_calls = 0;
    info->reset();
    expect(g_timer_calls == 1);
    expect(g_timer_self == info);
    expect(g_timer_arg == 1);
    expect(g_timer_flag_during_call == 0);
    expect(flag() == 0);
    std::memcpy(iw_want.data() + 16 + 0xA8, iw.data() + 16 + 0xA8, 4);
    expect_storage_bytes(iw.data(), iw_want.data(), iw.size());

    set_flag(-1);
    g_timer_calls = 0;
    info_win_reset_redirect(info, nullptr);
    expect(g_timer_calls == 1);
    expect(flag() == 0);
    InfoWinOriginalTimerProc = saved_timer;
}

// Self-registration.  Each case carries its baseline position in the original
// main() call list, so the run order does not depend on link or static-init
// order.
namespace {
LEAF_CASE(187, test_wave_is_playing);
LEAF_CASE(200, test_wave_play);
LEAF_CASE(201, test_wave_device_forwarders);
LEAF_CASE(202, test_wave_volume_fname_play);
LEAF_CASE(203, test_wave_load_empty);
LEAF_CASE(206, test_wave_init);
LEAF_CASE(210, test_wave_device_forwarder_family);
LEAF_CASE(211, test_wave_device_group_admin);
LEAF_CASE(212, test_wave_device_lifecycle);
LEAF_CASE(213, test_sound_chain_and_dtor);
LEAF_CASE(214, test_wave_device_construction);
LEAF_CASE(215, test_wave_device_select);
LEAF_CASE(216, test_auto_sound_lifecycle);
LEAF_CASE(218, test_ambience_dtor);
LEAF_CASE(217, test_popup_wave_callback);
LEAF_CASE(209, test_sound_small_setters);
LEAF_CASE(208, test_wave_device_groups);
LEAF_CASE(207, test_sound_set_type_and_load);
LEAF_CASE(205, test_ambience_construct);
LEAF_CASE(204, test_wave_ctor);
LEAF_CASE(199, test_wave_destructor);
LEAF_CASE(186, test_delegating_closers);
}  // namespace
