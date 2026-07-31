// Recovery leaf tests: destructors.
//
// Carved verbatim out of tests/recovery_leaf_tests.cpp.  Add a case by
// appending a `void test_*()` here plus one LEAF_CASE line at the bottom, and
// bumping this family's count in tests/leaf/leaf_case_manifest.h.
#include "leaf_shared.h"

#define LEAF_FAMILY "destructors"

void scroll_dtor_record(int kind, const void *target) {
    const int index = scroll_dtor_event_count++;
    if (index >= static_cast<int>(ARRAYSIZE(scroll_dtor_events))) {
        return;
    }
    ScrollDtorEvent &event = scroll_dtor_events[index];
    event.kind = kind;
    event.target = target;
    std::memcpy(&event.scroll_vtable0, scroll_dtor_base, 4);
    std::memcpy(&event.scroll_vtable444, scroll_dtor_base + 0x444, 4);
    event.time_count = time_close_calls;
}

uint32_t __fastcall scroll_dtor_left_vclose(void *self, void *) {
    scroll_dtor_record(1, self);
    return 0;
}

uint32_t __fastcall scroll_dtor_right_vclose(void *self, void *) {
    scroll_dtor_record(2, self);
    return 0;
}

#if defined(__GNUC__)

#pragma GCC diagnostic push

#pragma GCC diagnostic ignored "-Wattributes"

#endif

void __thiscall scroll_dtor_win_close(void *self) {
    scroll_dtor_record(3, self);
}

void __thiscall scroll_dtor_buffer_close(void *self) {
    scroll_dtor_record(4, self);
}

void __thiscall scroll_dtor_buffer_destructor(void *self) {
    scroll_dtor_record(5, self);
}

void __thiscall scroll_dtor_win_destructor(void *self) {
    scroll_dtor_record(6, self);
}

#if defined(__GNUC__)

#pragma GCC diagnostic pop

#endif

void *scroll_dtor_free_probe(void *block) {
    scroll_dtor_record(7, block);
    return nullptr;
}

void __cdecl scroll_dtor_operator_delete(void *block) {
    scroll_dtor_record(8, block);
}

// Seams shared by every stage the destructor reaches, installed for the
// reference component chain and the destructor run alike.
struct ScrollDtorFixture {
    uint32_t scroll_fixed[11];
    uint32_t scroll_dynamic[17];
    uint32_t base_fixed[5];
    uint32_t base_dynamic[2];
    uint32_t flat_defaults[27];
    uint32_t graphic_default = 0x7B3D19E5U;
    uintptr_t left_vtable[0x16C / sizeof(uintptr_t)] = {};
    uintptr_t right_vtable[0x16C / sizeof(uintptr_t)] = {};

    uint32_t *saved_scroll_fixed;
    uint32_t *saved_scroll_dynamic;
    uint32_t *saved_base_fixed;
    uint32_t *saved_base_dynamic;
    uint32_t *saved_flat_defaults;
    uint32_t *saved_graphic_default;
    func_subobject_close *saved_win_close;
    func_subobject_close *saved_buffer_close;
    func_subobject_destructor *saved_win_destructor;
    func_subobject_destructor *saved_buffer_destructor;
    func2 *saved_free;
    func_operator_delete *saved_operator_delete;

    void install() {
        for (size_t index = 0; index < ARRAYSIZE(scroll_fixed); ++index) {
            scroll_fixed[index] = 0x51000000U
                + static_cast<uint32_t>(index) * 0x010203U;
        }
        for (size_t index = 0; index < ARRAYSIZE(scroll_dynamic); ++index) {
            scroll_dynamic[index] = 0xA1000000U
                + static_cast<uint32_t>(index) * 0x010101U;
        }
        for (size_t index = 0; index < ARRAYSIZE(base_fixed); ++index) {
            base_fixed[index] = 0x61000000U
                + static_cast<uint32_t>(index) * 0x010203U;
        }
        for (size_t index = 0; index < ARRAYSIZE(base_dynamic); ++index) {
            base_dynamic[index] = 0xB1000000U
                + static_cast<uint32_t>(index) * 0x011011U;
        }
        for (size_t index = 0; index < ARRAYSIZE(flat_defaults); ++index) {
            flat_defaults[index] = 0xD1000000U
                + static_cast<uint32_t>(index) * 0x010101U;
        }
        left_vtable[0x168 / sizeof(uintptr_t)] =
            reinterpret_cast<uintptr_t>(&scroll_dtor_left_vclose);
        right_vtable[0x168 / sizeof(uintptr_t)] =
            reinterpret_cast<uintptr_t>(&scroll_dtor_right_vclose);
        saved_scroll_fixed = ScrollCloseStaticDefaults;
        saved_scroll_dynamic = ScrollCloseDynamicDefaults;
        saved_base_fixed = BaseButtonStaticDefaults;
        saved_base_dynamic = BaseButtonDynamicDefaults;
        saved_flat_defaults = FlatButtonDefaults;
        saved_graphic_default = GraphicWinFieldA0CDefault;
        saved_win_close = WinOriginalClose;
        saved_buffer_close = BufferSubobjectClose;
        saved_win_destructor = WinOriginalDestructor;
        saved_buffer_destructor = BufferSubobjectDestructor;
        saved_free = _free;
        saved_operator_delete = ScrollOperatorDelete;
        ScrollCloseStaticDefaults = scroll_fixed;
        ScrollCloseDynamicDefaults = scroll_dynamic;
        BaseButtonStaticDefaults = base_fixed;
        BaseButtonDynamicDefaults = base_dynamic;
        FlatButtonDefaults = flat_defaults;
        GraphicWinFieldA0CDefault = &graphic_default;
        WinOriginalClose = scroll_dtor_win_close;
        BufferSubobjectClose = scroll_dtor_buffer_close;
        WinOriginalDestructor = scroll_dtor_win_destructor;
        BufferSubobjectDestructor = scroll_dtor_buffer_destructor;
        _free = scroll_dtor_free_probe;
        ScrollOperatorDelete = scroll_dtor_operator_delete;
    }

    // Identical bytes in both storages, the button vtables aimed at the
    // recording probes, and every field a stage follows as a pointer zeroed:
    // the three 0xA08 release targets and both buttons' owned strings.
    void prepare(uint8_t *storage, uint8_t *reference, size_t size) {
        seed_storage(storage, reference, size);
        uintptr_t *left = left_vtable;
        uintptr_t *right = right_vtable;
        std::memcpy(storage + 16 + 0xAAC, &left, sizeof(left));
        std::memcpy(storage + 16 + 0x15F8, &right, sizeof(right));
        const uint32_t zero = 0;
        std::memcpy(storage + 16 + 0xA08, &zero, sizeof(zero));
        for (size_t button = 0; button < 2; ++button) {
            const size_t base = button ? 0x15F8 : 0xAAC;
            std::memcpy(storage + 16 + base + 0xA08, &zero, sizeof(zero));
            std::memcpy(storage + 16 + base + 0xA7C, &zero, sizeof(zero));
            std::memcpy(storage + 16 + base + 0xA80, &zero, sizeof(zero));
        }
        std::memcpy(reference, storage, size);
        scroll_dtor_base = storage + 16;
        scroll_dtor_event_count = 0;
        time_close_calls = 0;
        std::memset(time_close_targets, 0, sizeof(time_close_targets));
    }

    // The documented component chain, run on the reference copy. The event
    // recorder reads the destructor-run object, so swap the base in and out.
    void run_reference(uint8_t *reference) {
        uint8_t *const saved_base = scroll_dtor_base;
        scroll_dtor_base = reference + 16;
        auto *ref = reinterpret_cast<Scroll *>(reference + 16);
        ref->close();
        reinterpret_cast<FlatButton *>(reference + 16 + 0x15F8)->destroy();
        reinterpret_cast<FlatButton *>(reference + 16 + 0xAAC)->destroy();
        graphic_win_destructor_redirect(
            reinterpret_cast<GraphicWin *>(reference + 16), nullptr);
        scroll_dtor_base = saved_base;
        scroll_dtor_event_count = 0;
        time_close_calls = 0;
        std::memset(time_close_targets, 0, sizeof(time_close_targets));
    }

    void restore() {
        ScrollCloseStaticDefaults = saved_scroll_fixed;
        ScrollCloseDynamicDefaults = saved_scroll_dynamic;
        BaseButtonStaticDefaults = saved_base_fixed;
        BaseButtonDynamicDefaults = saved_base_dynamic;
        FlatButtonDefaults = saved_flat_defaults;
        GraphicWinFieldA0CDefault = saved_graphic_default;
        WinOriginalClose = saved_win_close;
        BufferSubobjectClose = saved_buffer_close;
        WinOriginalDestructor = saved_win_destructor;
        BufferSubobjectDestructor = saved_buffer_destructor;
        _free = saved_free;
        ScrollOperatorDelete = saved_operator_delete;
        time_close_calls = 0;
        std::memset(time_close_targets, 0, sizeof(time_close_targets));
    }
};

void expect_scroll_dtor_events(uint8_t *base) {
    const struct {
        int kind;
        size_t offset;
        int time_count;
    } expected[] = {
        // Scroll::close under the staged Scroll vtables: the two button
        // virtual closes left then right, then the GraphicWin close pair.
        {1, 0xAAC, 0}, {2, 0x15F8, 0}, {3, 0x000, 0}, {4, 0x444, 0},
        // Right button FlatButton::destroy: Time1 closes first, GraphicWin
        // close runs twice (FlatButton::close then BaseButton::destroy's
        // close), Time2 and Time1 are destroyed, then Buffer before Win.
        {3, 0x15F8 + 0x000, 1}, {4, 0x15F8 + 0x444, 1},
        {3, 0x15F8 + 0x000, 1}, {4, 0x15F8 + 0x444, 1},
        {5, 0x15F8 + 0x444, 3}, {6, 0x15F8 + 0x000, 3},
        // Left button, same shape.
        {3, 0xAAC + 0x000, 4}, {4, 0xAAC + 0x444, 4},
        {3, 0xAAC + 0x000, 4}, {4, 0xAAC + 0x444, 4},
        {5, 0xAAC + 0x444, 6}, {6, 0xAAC + 0x000, 6},
        // The GraphicWin base teardown of the Scroll itself.
        {5, 0x444, 6}, {6, 0x000, 6},
    };
    expect(scroll_dtor_event_count == static_cast<int>(ARRAYSIZE(expected)));
    for (size_t index = 0; index < ARRAYSIZE(expected); ++index) {
        const ScrollDtorEvent &event = scroll_dtor_events[index];
        expect(event.kind == expected[index].kind);
        expect(event.target == base + expected[index].offset);
        expect(event.time_count == expected[index].time_count);
        // The staged Scroll vtables are visible at every event until the
        // final GraphicWin stage installs its own pair first.
        if (index < ARRAYSIZE(expected) - 2) {
            expect(event.scroll_vtable0 == ScrollPrimaryVtable);
            expect(event.scroll_vtable444 == ScrollBufferVtable);
        } else {
            expect(event.scroll_vtable0 == GraphicWinPrimaryVtable);
            expect(event.scroll_vtable444 == GraphicWinBufferVtable);
        }
    }
    expect(time_close_calls == 6);
    const size_t time_targets[6] = {
        0x15F8 + 0xA1C, 0x15F8 + 0xA4C, 0x15F8 + 0xA1C,
        0xAAC + 0xA1C, 0xAAC + 0xA4C, 0xAAC + 0xA1C,
    };
    for (size_t index = 0; index < 6; ++index) {
        expect(time_close_targets[index]
               == reinterpret_cast<Time *>(base + time_targets[index]));
    }
}

void test_scroll_destructor() {
    ScrollDtorFixture fixture;
    fixture.install();

    for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
        alignas(Scroll) uint8_t storage[sizeof(Scroll) + 32];
        uint8_t reference[sizeof(storage)];
        fixture.prepare(storage, reference, sizeof(storage));
        fixture.run_reference(reference);

        auto *self = reinterpret_cast<Scroll *>(storage + 16);
        Scroll *const result = use_adapter
            ? scroll_destructor_redirect(self, nullptr) : self->destroy();
        expect(result == self);
        expect_scroll_dtor_events(storage + 16);
        expect_storage_bytes(storage, reference, sizeof(storage));
    }

    // The scalar deleting destructor frees only on bit 0, after the complete
    // destructor, and always returns the object.
    const unsigned int modes[] = {0U, 1U, 2U};
    for (size_t mode_index = 0; mode_index < ARRAYSIZE(modes); ++mode_index) {
        const unsigned int mode = modes[mode_index];
        alignas(Scroll) uint8_t storage[sizeof(Scroll) + 32];
        uint8_t reference[sizeof(storage)];
        fixture.prepare(storage, reference, sizeof(storage));
        fixture.run_reference(reference);

        auto *self = reinterpret_cast<Scroll *>(storage + 16);
        expect(scroll_scalar_dtor_redirect(self, nullptr, mode) == self);
        const bool deletes = (mode & 1U) != 0U;
        expect(scroll_dtor_event_count == 18 + (deletes ? 1 : 0));
        if (deletes) {
            expect(scroll_dtor_events[18].kind == 8);
            expect(scroll_dtor_events[18].target == self);
        }
        scroll_dtor_event_count = 18;
        expect_scroll_dtor_events(storage + 16);
        expect_storage_bytes(storage, reference, sizeof(storage));
    }

    fixture.restore();
}

int sprite_close_free_calls = 0;

void *sprite_close_free_targets[4];

void *sprite_close_free_probe(void *block) {
    if (sprite_close_free_calls < 4) {
        sprite_close_free_targets[sprite_close_free_calls] = block;
    }
    ++sprite_close_free_calls;
    return nullptr;
}

void test_sprite_close() {
    struct CloseCase {
        uint32_t field_00;   // primary allocation
        uint32_t field_04;   // pixel buffer
        uint32_t field_10;   // width
        uint32_t field_14;   // height
        uint32_t field_28;   // suppresses the pixel accounting when nonzero
        int expected_frees;
        bool accounts;
    };
    const CloseCase cases[] = {
        // Nothing owned: no frees, no accounting.
        {0, 0, 7, 11, 0, 0, false},
        // Pixel buffer owned and accounted, then freed.
        {0, 0x11110000U, 7, 11, 0, 1, true},
        // field_28 set suppresses both the accounting and the pixel free.
        {0, 0x11110000U, 7, 11, 1, 0, false},
        // Primary allocation only.
        {0x22220000U, 0, 7, 11, 0, 1, false},
        // Both owned: pixel buffer freed before the primary allocation.
        {0x22220000U, 0x11110000U, 3, 5, 0, 2, true},
        // Wrapping accounting arithmetic.
        {0, 0x11110000U, 0xFFFFFFFFU, 2, 0, 1, true},
        {0, 0x11110000U, 0x80000000U, 0x80000000U, 0, 1, true},
    };
    for (const CloseCase &test : cases) {
        for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
            alignas(Sprite) uint8_t storage[sizeof(Sprite) + 32];
            uint8_t expected[sizeof(storage)];
            seed_storage(storage, expected, sizeof(storage));
            write_at(storage, 16 + 0x00, test.field_00);
            write_at(storage, 16 + 0x04, test.field_04);
            write_at(storage, 16 + 0x10, test.field_10);
            write_at(storage, 16 + 0x14, test.field_14);
            write_at(storage, 16 + 0x28, test.field_28);
            std::memcpy(expected, storage, sizeof(storage));
            // field_08 holds the type byte and is never cleared by close.
            if (test.field_28 == 0 && test.field_04 != 0) {
                write_at(expected, 16 + 0x04, 0U);
            }
            if (test.field_00 != 0) {
                write_at(expected, 16 + 0x00, 0U);
            }
            for (size_t offset = 0x0C; offset <= 0x28; offset += 4) {
                write_at(expected, 16 + offset, 0U);
            }

            const int32_t starting_total = 0x1000;
            int32_t total = starting_total;
            int *const saved_total = SpriteMemoryUsed;
            func_sprite_free *const saved_free = SpriteFree;
            SpriteMemoryUsed = &total;
            SpriteFree = &sprite_close_free_probe;
            sprite_close_free_calls = 0;

            auto *sprite = reinterpret_cast<Sprite *>(storage + 16);
            if (use_adapter) {
                sprite_close_redirect(sprite, nullptr);
            } else {
                sprite->close();
            }
            SpriteFree = saved_free;
            SpriteMemoryUsed = saved_total;

            expect_storage_bytes(storage, expected, sizeof(storage));
            expect(sprite_close_free_calls == test.expected_frees);
            const int32_t expected_total = test.accounts
                ? static_cast<int32_t>(
                      static_cast<uint32_t>(starting_total)
                      - test.field_14 * test.field_10)
                : starting_total;
            expect(total == expected_total);
            // The pixel buffer is released before the primary allocation.
            if (test.expected_frees == 2) {
                expect(sprite_close_free_targets[0]
                       == reinterpret_cast<void *>(test.field_04));
                expect(sprite_close_free_targets[1]
                       == reinterpret_cast<void *>(test.field_00));
            }
        }
    }
}

struct FakeSurface {
    void **vtable;
};

int surface_lock_calls = 0;

int surface_unlock_calls = 0;

void *surface_unlock_data = nullptr;

uint32_t surface_lock_pitch = 0;

uint32_t surface_lock_data = 0;

long surface_lock_result = 0;

uint32_t surface_lock_seen_size = 0;

long __stdcall fake_surface_lock(
        void *, void *rect, void *descriptor, uint32_t flags, void *event) {
    ++surface_lock_calls;
    auto *bytes = static_cast<uint8_t *>(descriptor);
    std::memcpy(&surface_lock_seen_size, bytes, sizeof(surface_lock_seen_size));
    expect(rect == nullptr && flags == 1U && event == nullptr);
    if (surface_lock_result == 0) {
        std::memcpy(bytes + 0x10, &surface_lock_pitch, sizeof(surface_lock_pitch));
        std::memcpy(bytes + 0x24, &surface_lock_data, sizeof(surface_lock_data));
    }
    return surface_lock_result;
}

long __stdcall fake_surface_unlock(void *, void *data) {
    ++surface_unlock_calls;
    surface_unlock_data = data;
    return 0;
}

void reset_surface_probes() {
    surface_lock_calls = 0;
    surface_unlock_calls = 0;
    surface_unlock_data = nullptr;
    surface_lock_seen_size = 0;
}

void test_buffer_get_data() {
    void *vtable[(0x80 / sizeof(void *)) + 1] = {};
    vtable[BufferSurfaceLockSlot / sizeof(void *)] =
        reinterpret_cast<void *>(&fake_surface_lock);
    vtable[BufferSurfaceUnlockSlot / sizeof(void *)] =
        reinterpret_cast<void *>(&fake_surface_unlock);
    FakeSurface surface = {vtable};

    struct GetCase {
        bool has_surface;
        uint32_t field_50;      // published data pointer
        uint32_t field_54;      // owned storage used without a surface
        uint32_t references;
        long lock_result;
        uint32_t lock_pitch;
        uint32_t lock_data;
        int expected_result;
        int expected_locks;
        uint32_t expected_references;
    };
    const GetCase cases[] = {
        // No surface, no storage: publishes zero and counts nothing.
        {false, 0x9999U, 0, 4, 0, 0, 0, 0, 0, 4},
        // No surface with storage: publishes it and counts one reference.
        {false, 0x9999U, 0x4444U, 4, 0, 0, 0, 0x4444, 0, 5},
        // Surface already published: counts without locking.
        {true, 0x5555U, 0, 7, 0, 0, 0, 0x5555, 0, 8},
        // Surface locks successfully and publishes pitch plus data.
        {true, 0, 0, 0, 0, 0x1234U, 0x8888U, 0x8888, 1, 1},
        // A failed lock publishes nothing and counts nothing.
        {true, 0, 0, 3, 1, 0x1234U, 0x8888U, 0, 1, 3},
    };
    for (const GetCase &test : cases) {
        for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
            alignas(Buffer) uint8_t storage[sizeof(Buffer) + 32];
            uint8_t expected[sizeof(storage)];
            seed_storage(storage, expected, sizeof(storage));
            write_at(storage, 16 + 0x50, test.field_50);
            write_at(storage, 16 + 0x54, test.field_54);
            write_at(storage, 16 + 0x58,
                     test.has_surface ? reinterpret_cast<uint32_t>(&surface) : 0U);
            write_at(storage, 16 + 0x6C, test.references);
            std::memcpy(expected, storage, sizeof(storage));

            reset_surface_probes();
            surface_lock_result = test.lock_result;
            surface_lock_pitch = test.lock_pitch;
            surface_lock_data = test.lock_data;

            auto *buffer = reinterpret_cast<Buffer *>(storage + 16);
            const int result = use_adapter
                ? buffer_get_data_redirect(buffer, nullptr)
                : buffer->get_data();

            if (!test.has_surface) {
                write_at(expected, 16 + 0x50, test.field_54);
            } else if (test.lock_result == 0 && test.field_50 == 0) {
                write_at(expected, 16 + 0x4A8, test.lock_pitch);
                write_at(expected, 16 + 0x50, test.lock_data);
            }
            write_at(expected, 16 + 0x6C, test.expected_references);

            expect(result == test.expected_result);
            expect(surface_lock_calls == test.expected_locks);
            expect_storage_bytes(storage, expected, sizeof(storage));
            // The descriptor always announces its own size to the surface.
            if (test.expected_locks) {
                expect(surface_lock_seen_size == 0x6CU);
            }
        }
    }
}

void test_buffer_free_data() {
    void *vtable[(0x80 / sizeof(void *)) + 1] = {};
    vtable[BufferSurfaceLockSlot / sizeof(void *)] =
        reinterpret_cast<void *>(&fake_surface_lock);
    vtable[BufferSurfaceUnlockSlot / sizeof(void *)] =
        reinterpret_cast<void *>(&fake_surface_unlock);
    FakeSurface surface = {vtable};

    struct FreeCase {
        bool has_surface;
        uint32_t field_50;
        uint32_t references;
        int count;
        int expected_unlocks;
        bool clears;
        uint32_t expected_references;
    };
    const FreeCase cases[] = {
        // No surface: the release clears once the count reaches zero.
        {false, 0x5555U, 3, 1, 0, false, 2},
        {false, 0x5555U, 1, 1, 0, true, 0},
        {false, 0x5555U, 1, 4, 0, true, 0},
        {false, 0, 1, 1, 0, true, 0},
        // Surface: unlocking additionally requires published data.
        {true, 0x5555U, 3, 1, 0, false, 2},
        {true, 0x5555U, 1, 1, 1, true, 0},
        {true, 0, 1, 1, 0, false, 0},
        // Release counts are compared as signed, so a wrapping subtraction
        // that lands negative still releases.
        {true, 0x5555U, 0, static_cast<int>(0x80000000U), 1, true, 0},
        {false, 0x5555U, 0, static_cast<int>(0x80000000U), 0, true, 0},
        // A large positive remainder keeps the data published.
        {true, 0x5555U, 0x7FFFFFFFU, 1, 0, false, 0x7FFFFFFEU},
    };
    for (const FreeCase &test : cases) {
        for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
            alignas(Buffer) uint8_t storage[sizeof(Buffer) + 32];
            uint8_t expected[sizeof(storage)];
            seed_storage(storage, expected, sizeof(storage));
            write_at(storage, 16 + 0x50, test.field_50);
            write_at(storage, 16 + 0x58,
                     test.has_surface ? reinterpret_cast<uint32_t>(&surface) : 0U);
            write_at(storage, 16 + 0x6C, test.references);
            std::memcpy(expected, storage, sizeof(storage));

            reset_surface_probes();
            auto *buffer = reinterpret_cast<Buffer *>(storage + 16);
            if (use_adapter) {
                buffer_free_data_redirect(buffer, nullptr, test.count);
            } else {
                buffer->free_data(test.count);
            }

            if (test.clears) {
                write_at(expected, 16 + 0x50, 0U);
                write_at(expected, 16 + 0x6C, 0U);
            } else {
                write_at(expected, 16 + 0x6C, test.expected_references);
            }
            expect(surface_unlock_calls == test.expected_unlocks);
            if (test.expected_unlocks) {
                expect(surface_unlock_data
                       == reinterpret_cast<void *>(test.field_50));
            }
            expect_storage_bytes(storage, expected, sizeof(storage));
        }
    }
}

struct RemoveAllProbe {
    int visits;
    int payload_destroys;
    int entry_destroys;
    void *last_payload;
    int last_flags;
};

RemoveAllProbe remove_all_probe = {};

void __thiscall remove_all_visitor(void *, void *payload) {
    ++remove_all_probe.visits;
    remove_all_probe.last_payload = payload;
}

void __thiscall remove_all_payload_destroy(void *, int flags) {
    ++remove_all_probe.payload_destroys;
    remove_all_probe.last_flags = flags;
}

void __thiscall remove_all_entry_destroy(void *, int flags) {
    ++remove_all_probe.entry_destroys;
    remove_all_probe.last_flags = flags;
}

// A destructible stand-in: the object's vtable slot 1 holds the virtual-base
// displacement to itself, so slot 0 of the same vtable is the destructor.
struct Destructible {
    uint32_t *vptr;
    uint32_t vtable[2];
};

void arm_destructible(Destructible &object, void *destructor) {
    object.vtable[0] = reinterpret_cast<uint32_t>(destructor);
    object.vtable[1] = 0;   // displacement selects the object itself
    object.vptr = object.vtable;
}

void test_string_struct_remove_all() {
    struct RemoveCase {
        int entries;
        int count;
        bool with_payloads;
    };
    const RemoveCase cases[] = {
        {0, 0, false},      // empty list returns immediately
        {3, 0, true},       // non-positive count clears without walking
        {3, -1, true},
        {1, 1, true},
        {3, 3, true},
        {3, 3, false},      // null payloads skip the payload destructor
        {3, 2, true},       // count shorter than the list stops early
    };
    for (const RemoveCase &test : cases) {
        for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
            alignas(StringStruct) uint8_t storage[sizeof(StringStruct) + 32];
            uint8_t expected[sizeof(storage)];
            seed_storage(storage, expected, sizeof(storage));

            StringStructEntry entries[3] = {};
            Destructible entry_objects[3] = {};
            Destructible payloads[3] = {};
            uint32_t owner_vtable[2] = {};
            owner_vtable[1] = reinterpret_cast<uint32_t>(&remove_all_visitor);

            for (int index = 0; index < test.entries; ++index) {
                arm_destructible(entry_objects[index],
                                 reinterpret_cast<void *>(&remove_all_entry_destroy));
                arm_destructible(payloads[index],
                                 reinterpret_cast<void *>(&remove_all_payload_destroy));
                // Each entry is prefixed by its own vptr so the destructor
                // dispatch finds the stand-in vtable.
                entries[index].abi_word =
                    reinterpret_cast<uint32_t>(entry_objects[index].vtable);
                entries[index].payload = test.with_payloads
                    ? static_cast<int>(reinterpret_cast<uintptr_t>(&payloads[index]))
                    : 0;
                entries[index].next = (index + 1 < test.entries)
                    ? &entries[index + 1] : nullptr;
            }

            write_at(storage, 16 + 0x00, reinterpret_cast<uint32_t>(owner_vtable));
            write_at(storage, 16 + 0x08,
                     test.entries ? reinterpret_cast<uint32_t>(&entries[0]) : 0U);
            write_at(storage, 16 + 0x10, static_cast<uint32_t>(test.count));
            std::memcpy(expected, storage, sizeof(storage));

            remove_all_probe = RemoveAllProbe();
            auto *list = reinterpret_cast<StringStruct *>(storage + 16);
            if (use_adapter) {
                string_struct_remove_all_redirect(list, nullptr);
            } else {
                list->remove_all();
            }

            const int walked = (test.entries == 0) ? 0
                : (test.count > 0 ? (test.count < test.entries
                                     ? test.count : test.entries) : 0);
            if (test.entries != 0) {
                write_at(expected, 16 + 0x08, 0U);
                write_at(expected, 16 + 0x10, 0U);
                write_at(expected, 16 + 0x14, 0U);
                if (walked) {
                    // current_ trails the last entry the walk advanced past.
                    write_at(expected, 16 + 0x0C,
                             walked < test.entries
                                 ? reinterpret_cast<uint32_t>(&entries[walked])
                                 : 0U);
                }
            }
            expect(remove_all_probe.visits == walked);
            expect(remove_all_probe.entry_destroys == walked);
            expect(remove_all_probe.payload_destroys
                   == (test.with_payloads ? walked : 0));
            if (walked && test.with_payloads) {
                expect(remove_all_probe.last_flags == 1);
            }
            // Every walked entry has its payload field cleared.
            for (int index = 0; index < walked; ++index) {
                expect(entries[index].payload == 0);
            }
            expect_storage_bytes(storage, expected, sizeof(storage));
        }
    }
}

// ---------------------------------------------------------------------------
// StringList::destroy - the complete (non-deleting) ~StringList at 0x00406820.
//
// Compositional oracle, in the test_scroll_destructor sense: the one stage the
// destructor composes is already-recovered code with exact tests of its own, so
// the reference image is produced by running that component on a byte-identical
// twin and the destructor must match it byte for byte.
//
// EVERY CONSTANT BELOW IS A LITERAL, NOT THE RECOVERY'S NAMED CONSTANT.
// Building the reference out of StringVirtualBaseVtable /
// StringListVirtualBaseOffset would move the reference and the implementation
// together under a poison and make the whole comparison vacuous - the failure
// mode AGENTS.md records for the stringstruct oracle's synthetic displacement.
// The literals here are read straight from the disassembly:
//     0x28       lea esi, [ecx + 0x28]           (0x00406821)
//     0x006693AC mov dword ptr [esi], 0x6693ac   (0x0040682E)
//     0x006693A4 mov dword ptr [esi], 0x6693a4   (0x00406782, the base stage)
//     0x006693A0 mov [edx+ebx-0x24], 0x6693a0    (0x0040678E, the base stage)
//
// The function has exactly ONE dependency - the source-complete derived close
// at 0x004066C0 (string_struct_derived_close_redirect) - and it is called
// directly rather than through a seam, because AGENTS.md prescribes calling the
// recovered C++ for a source_complete callee. There is therefore no recording
// probe to install for it; delegation, entry target and ordering are pinned
// instead by three properties of the resulting image:
//
//   * The close's final base stage writes 0x006693A4 at (entry - 0x28). Seeing
//     it at [obj + 0x00] proves the callee was entered at exactly obj + 0x28.
//   * The close clears head_/entry_count_/current_position_. Seeing them
//     cleared proves the delegation actually ran.
//   * In the most-derived shape the close stages 0x006693A0 into the SAME slot
//     (obj + 0x28) that the destructor's tail then overwrites with 0x006693AC.
//     Seeing 0x006693AC proves the tail ran AFTER the delegation; an
//     implementation that ordered them the other way leaves 0x006693A0 there.
//
// Two genuinely different vbtables are installed, per the AGENTS.md RadioButton
// rule. The second is adversarial on purpose: it names StringStruct's own 0x1C
// displacement, which is exactly the wrong constant a careless recovery would
// reuse. Under it the delegated close's virtual-base write moves to obj + 0x20
// while the destructor's two accesses must STILL land at obj + 0x28, because
// the original bakes `lea esi, [ecx + 0x28]` in rather than reading the vbtable.
// A recovery that "improved" that constant into a vbtable read fails this shape
// and only this shape.
//
// The list shapes are all non-walking. They have to be: close_with_tables
// installs the real 0x006698C4 / 0x006693A4 table ADDRESSES into [this] before
// remove_all dispatches through vtable[1], and those addresses are unmapped in
// this executable - the limit AGENTS.md records for exactly this family. The
// walk belongs to 0x004066C0 and is covered in-process by the stringstruct
// runtime-oracle suite; nothing 0x00406820 itself does lives inside it.
//
// Scope note: a poison applied to close_with_tables moves the reference and the
// storage together and is invisible here by construction. That is correct - it
// is the dependency's own contract, covered by test_string_struct_remove_all
// and the stringstruct runtime-oracle suite.

// The pending-allocation global, wrapped in guards so a write of the wrong
// width or into the wrong slot is visible rather than absorbed.
uint32_t string_list_owner_cell[3];

void test_string_list_destructor() {
    static_assert(sizeof(StringList) == 0x30,
                  "StringList destructor test requires the legacy 0x30 layout");

    // Read straight from the disassembly; never the recovery's own names.
    const uint32_t kVirtualBaseOwnVtable = 0x006693ACU;   // 0x0040682E
    const uint32_t kStringStructVtable = 0x006693A4U;     // 0x00406782
    const uint32_t kStringStructVbaseVtable = 0x006693A0U; // 0x0040678E
    const size_t kVirtualBaseOffset = 0x28;               // 0x00406821
    const uint32_t kOwnerSentinel = 0xC0FFEE01U;
    const uint32_t kOwnerPrefill = 0xDEADBEEFU;
    const uint32_t kOwnerLeadGuard = 0x11111111U;
    const uint32_t kOwnerTrailGuard = 0x33333333U;

    // The only global this test rewrites.
    uint32_t *const saved_owner_global = StringVirtualBaseOwner;

    // vbtable[0] is the vbptr's own offset (-4 in the original); only
    // vbtable[1] is read, by close_with_tables.
    //   0x24 - the real StringList vbtable 0x0066B0EC = { -4, 0x24 }: the
    //          close's virtual-base write lands on obj + 0x28, the same slot
    //          the destructor's tail overwrites.
    //   0x1C - StringStruct's own most-derived displacement: the close's write
    //          moves to obj + 0x20 and must NOT drag the destructor with it.
    const int32_t displacements[2] = {0x24, 0x1C};

    struct ListCase {
        bool has_head;
        int32_t count;
    };
    const ListCase lists[3] = {
        // Empty list: remove_all returns immediately, so the close only stages
        // the two table pairs and resets current_position_ - twice.
        {false, 7},
        // Populated head with a non-positive count: the clearing path runs
        // without dispatching through the installed (real, unmapped) vtable.
        {true, 0},
        {true, -1},
    };

    for (int shape = 0; shape < 2; ++shape) {
        const int32_t displacement = displacements[shape];
        int32_t vbtable[2] = {-4, displacement};

        for (const ListCase &list : lists) {
            for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
                // 16 leading + 16 trailing canary bytes around the 0x30 object.
                alignas(StringList) uint8_t storage[sizeof(StringList) + 32];
                alignas(StringList) uint8_t reference[sizeof(storage)];
                alignas(StringList) uint8_t seed_copy[sizeof(storage)];
                seed_storage(storage, reference, sizeof(storage));

                // Never dereferenced: head_ only has to be non-null for the
                // clearing path, and no shape here walks the list.
                StringStructEntry entry = {};

                int32_t *const vbtable_pointer = vbtable;
                write_at(storage, 16 + 0x04, vbtable_pointer);
                const uint32_t head = list.has_head
                    ? static_cast<uint32_t>(
                          reinterpret_cast<uintptr_t>(&entry))
                    : 0U;
                write_at(storage, 16 + 0x08, head);
                write_at(storage, 16 + 0x10, list.count);
                // current_position_ gets a nonzero sentinel: the close resets
                // it unconditionally, and a zero-seeded field can never show
                // that it was zeroed (the stringstruct_oracle lesson).
                write_at(storage, 16 + 0x14, 0x77777777U);
                // The saved allocation owner the destructor republishes.
                write_at(storage, 16 + 0x2C, kOwnerSentinel);
                std::memcpy(reference, storage, sizeof(storage));
                std::memcpy(seed_copy, storage, sizeof(storage));

                // --- reference image -------------------------------------
                // The recovered dependency, entered exactly as the original
                // enters it (this + 0x28, literal), on the byte-identical twin.
                // It touches no global, so the guarded cell below is unaffected.
                string_struct_derived_close_redirect(
                    reference + 16 + kVirtualBaseOffset, nullptr);
                // ...then the one tail store into the object, hand-built. The
                // other two tail effects are the global write and the EAX
                // residue, asserted separately below.
                write_at(reference, 16 + kVirtualBaseOffset,
                         kVirtualBaseOwnVtable);
                // The dependency must actually have changed something, or the
                // whole comparison would be comparing the seed with itself.
                expect(std::memcmp(reference, seed_copy, sizeof(reference)) != 0);

                // --- run under test --------------------------------------
                string_list_owner_cell[0] = kOwnerLeadGuard;
                string_list_owner_cell[1] = kOwnerPrefill;
                string_list_owner_cell[2] = kOwnerTrailGuard;
                StringVirtualBaseOwner = &string_list_owner_cell[1];

                auto *const self = reinterpret_cast<StringList *>(storage + 16);
                const uint32_t result = use_adapter
                    ? string_list_destructor_redirect(self, nullptr)
                    : self->destroy();

                // Byte-exact object plus complete leading and trailing canaries.
                if (std::memcmp(storage, reference, sizeof(storage)) != 0) {
                    report_storage_mismatch("string_list_destructor",
                                            storage, reference,
                                            sizeof(storage));
                }
                expect_storage_bytes(storage, reference, sizeof(storage));

                // EAX residue: the saved owner, not zero and not `this`.
                expect(result == kOwnerSentinel);

                // The global is republished: exactly four bytes, in place.
                expect(string_list_owner_cell[0] == kOwnerLeadGuard);
                expect(string_list_owner_cell[1] == kOwnerSentinel);
                expect(string_list_owner_cell[2] == kOwnerTrailGuard);

                // --- delegation, entry target and ordering ----------------
                uint32_t primary = 0;
                std::memcpy(&primary, storage + 16 + 0x00, sizeof(primary));
                // The close's final base stage writes 0x006693A4 at
                // (entry - 0x28); finding it at obj + 0x00 pins the entry
                // pointer the destructor handed the callee.
                expect(primary == kStringStructVtable);

                uint32_t position = 0xFFFFFFFFU;
                std::memcpy(&position, storage + 16 + 0x14, sizeof(position));
                expect(position == 0U);   // the close's unconditional reset

                if (list.has_head) {
                    uint32_t cleared_head = 0xFFFFFFFFU;
                    uint32_t cleared_count = 0xFFFFFFFFU;
                    std::memcpy(&cleared_head, storage + 16 + 0x08,
                                sizeof(cleared_head));
                    std::memcpy(&cleared_count, storage + 16 + 0x10,
                                sizeof(cleared_count));
                    expect(cleared_head == 0U);
                    expect(cleared_count == 0U);
                    // The entry itself was never touched: no walk happened.
                    expect(entry.id == 0);
                    expect(entry.payload == 0);
                } else {
                    // An empty list short-circuits before the clearing writes,
                    // so entry_count_ keeps the seeded value.
                    uint32_t untouched_count = 0;
                    std::memcpy(&untouched_count, storage + 16 + 0x10,
                                sizeof(untouched_count));
                    expect(untouched_count
                           == static_cast<uint32_t>(list.count));
                }

                // The destructor's own vtable store always lands at +0x28.
                uint32_t virtual_base_vtable = 0;
                std::memcpy(&virtual_base_vtable,
                            storage + 16 + kVirtualBaseOffset,
                            sizeof(virtual_base_vtable));
                expect(virtual_base_vtable == kVirtualBaseOwnVtable);

                // The saved-owner slot is read, never written.
                uint32_t retained_owner = 0;
                std::memcpy(&retained_owner, storage + 16 + 0x2C,
                            sizeof(retained_owner));
                expect(retained_owner == kOwnerSentinel);

                if (displacement == 0x24) {
                    // Most-derived shape: the close staged 0x006693A0 into
                    // obj + 0x28 first, so 0x006693AC there proves the tail ran
                    // afterwards. Swapping the two halves of destroy() leaves
                    // 0x006693A0 in the slot and fails here.
                    expect(virtual_base_vtable != kStringStructVbaseVtable);
                } else {
                    // Adversarial vbtable: the close followed vbtable[1] to
                    // obj + 4 + 0x1C, while the destructor stayed on its
                    // baked-in +0x28. Both must hold at once.
                    uint32_t staged = 0;
                    std::memcpy(&staged, storage + 16 + 0x20, sizeof(staged));
                    expect(staged == kStringStructVbaseVtable);
                    expect(virtual_base_vtable == kVirtualBaseOwnVtable);
                }
            }
        }
    }

    StringVirtualBaseOwner = saved_owner_global;
}

// Event log for the Dialog destructor. kind: 1 Dialog::close, 2 owner visitor,
// 3 payload scalar dtor, 4 entry scalar dtor, 5 operator delete.
struct DialogDtorEvent {
    int kind;
    const void *target;
    uint32_t a;   // close: [this+0x00];  visit: [this+0xBC];  dtors: flags
    uint32_t b;   // close: [this+0xC4];  visit: [this+0xE4]
    uint32_t c;   // visit: [this+0x10] (Heap base_size_, still live mid-walk)
};

DialogDtorEvent dialog_dtor_events[16];

int dialog_dtor_event_count = 0;

uint8_t *dialog_dtor_base = nullptr;

void dialog_dtor_record(int kind, const void *target,
                        uint32_t a, uint32_t b, uint32_t c) {
    const int index = dialog_dtor_event_count++;
    if (index >= static_cast<int>(ARRAYSIZE(dialog_dtor_events))) {
        return;
    }
    dialog_dtor_events[index] = {kind, target, a, b, c};
}

void __thiscall dialog_close_probe(Dialog *self) {
    uint32_t vtable0 = 0;
    uint32_t head = 0;
    std::memcpy(&vtable0, dialog_dtor_base + 0x00, sizeof(vtable0));
    std::memcpy(&head, dialog_dtor_base + 0xC4, sizeof(head));
    dialog_dtor_record(1, self, vtable0, head, 0);
}

void __thiscall dialog_visitor_probe(void *, void *payload) {
    uint32_t owner = 0;
    uint32_t vbase = 0;
    uint32_t heap_size = 0;
    std::memcpy(&owner, dialog_dtor_base + 0xBC, sizeof(owner));
    std::memcpy(&vbase, dialog_dtor_base + 0xE4, sizeof(vbase));
    std::memcpy(&heap_size, dialog_dtor_base + 0x10, sizeof(heap_size));
    dialog_dtor_record(2, payload, owner, vbase, heap_size);
}

void __thiscall dialog_payload_dtor_probe(void *self, int flags) {
    dialog_dtor_record(3, self, static_cast<uint32_t>(flags), 0, 0);
}

void __thiscall dialog_entry_dtor_probe(void *self, int flags) {
    dialog_dtor_record(4, self, static_cast<uint32_t>(flags), 0, 0);
}

void __cdecl dialog_operator_delete_probe(void *block) {
    dialog_dtor_record(5, block, 0, 0, 0);
}

void test_dialog_destructor() {
    static_assert(sizeof(Dialog) == 0xF4,
                  "Dialog destructor test requires the legacy layout");

    func_dialog_close *const saved_close = DialogOriginalClose;
    func_operator_delete *const saved_delete = DialogOperatorDelete;
    uint32_t *const saved_published = DialogPublishedGlobal;
    const uint32_t saved_derived = DialogListDerivedVtable;
    const uint32_t saved_derived_vb = DialogListDerivedVirtualBaseVtable;
    const uint32_t saved_list = DialogListVtable;
    const uint32_t saved_list_vb = DialogListVirtualBaseVtable;

    // Stand-in stage-1 owner table: slot 1 is the recording visitor the
    // embedded walk dispatches through once the destructor installs this
    // table's address at this+0xBC. The other three staged tables are written
    // but never dispatched, so distinct sentinels pin each write.
    uint32_t owner_vtable[2] = {
        0, reinterpret_cast<uint32_t>(&dialog_visitor_probe)};
    uint32_t published_slot = 0;

    DialogOriginalClose = &dialog_close_probe;
    DialogOperatorDelete = &dialog_operator_delete_probe;
    DialogPublishedGlobal = &published_slot;
    DialogListDerivedVtable = reinterpret_cast<uint32_t>(owner_vtable);
    DialogListDerivedVirtualBaseVtable = 0xB1B1B1B1U;   // stage-1 vbase table
    DialogListVtable = 0xC2C2C2C2U;                     // stage-2 owner table
    DialogListVirtualBaseVtable = 0xB2B2B2B2U;          // stage-2 vbase table

    struct DtorCase { int entries; };
    const DtorCase cases[] = { {0}, {3} };

    for (const DtorCase &test : cases) {
        for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
            alignas(Dialog) uint8_t storage[sizeof(Dialog) + 32];
            uint8_t expected[sizeof(storage)];
            seed_storage(storage, expected, sizeof(storage));

            uint32_t vbtable[2] = {0xAAAAAAAAU, 0x24U};
            StringStructEntry entries[3] = {};
            Destructible entry_objects[3] = {};
            Destructible payloads[3] = {};
            for (int i = 0; i < test.entries; ++i) {
                arm_destructible(entry_objects[i],
                                 reinterpret_cast<void *>(&dialog_entry_dtor_probe));
                arm_destructible(payloads[i],
                                 reinterpret_cast<void *>(&dialog_payload_dtor_probe));
                entries[i].abi_word =
                    reinterpret_cast<uint32_t>(entry_objects[i].vtable);
                entries[i].payload =
                    static_cast<int>(reinterpret_cast<uintptr_t>(&payloads[i]));
                entries[i].next =
                    (i + 1 < test.entries) ? &entries[i + 1] : nullptr;
            }

            write_at(storage, 16 + 0x08, 0U);   // Heap base_: no real free
            write_at(storage, 16 + 0x10, 0xFEEDF00DU);   // Heap base_size_ sentinel
            write_at(storage, 16 + 0xC0, reinterpret_cast<uint32_t>(vbtable));
            write_at(storage, 16 + 0xC4,
                     test.entries ? reinterpret_cast<uint32_t>(&entries[0]) : 0U);
            // Empty case: 0x55 proves the count survives when the walk block
            // is skipped (the original's je past the clears).
            write_at(storage, 16 + 0xCC,
                     test.entries ? static_cast<uint32_t>(test.entries) : 0x55U);
            write_at(storage, 16 + 0xE8, 0x1234ABCDU);   // vbase context word
            std::memcpy(expected, storage, sizeof(storage));

            write_at(expected, 16 + 0x00, DialogPrimaryVtable);
            expect_heap_clear(expected, 16 + 0x04);
            write_at(expected, 16 + 0xBC, DialogListVtable);   // stage-2 table last
            write_at(expected, 16 + 0xD0, 0U);                 // position, both cases
            write_at(expected, 16 + 0xE4, DialogVirtualBaseFinalVtable);
            if (test.entries) {
                write_at(expected, 16 + 0xC4, 0U);   // head_
                write_at(expected, 16 + 0xC8, 0U);   // current_ past the tail
                write_at(expected, 16 + 0xCC, 0U);   // entry_count_
            }

            auto *self = reinterpret_cast<Dialog *>(storage + 16);
            dialog_dtor_base = storage + 16;
            dialog_dtor_event_count = 0;
            published_slot = 0;

            if (use_adapter) {
                dialog_destructor_redirect(self, nullptr);
            } else {
                self->destroy();
            }

            expect_storage_bytes(storage, expected, sizeof(storage));
            expect(published_slot == 0x1234ABCDU);

            const int walked = test.entries;
            expect(dialog_dtor_event_count == 1 + walked * 3);

            // Dialog::close runs first: the Dialog table is already staged and
            // the list head is still intact.
            expect(dialog_dtor_events[0].kind == 1);
            expect(dialog_dtor_events[0].target == self);
            expect(dialog_dtor_events[0].a == DialogPrimaryVtable);
            expect(dialog_dtor_events[0].b ==
                   (test.entries ? reinterpret_cast<uint32_t>(&entries[0]) : 0U));

            for (int i = 0; i < walked; ++i) {
                const DialogDtorEvent &visit = dialog_dtor_events[1 + i * 3];
                const DialogDtorEvent &pdtor = dialog_dtor_events[2 + i * 3];
                const DialogDtorEvent &edtor = dialog_dtor_events[3 + i * 3];
                expect(visit.kind == 2);
                expect(visit.target == &payloads[i]);
                // Stage-1 tables staged before the walk: the owner table at
                // this+0xBC and, through the vbtable displacement 0x24, the
                // vbase sentinel landed at this+0xE4.
                expect(visit.a == reinterpret_cast<uint32_t>(owner_vtable));
                expect(visit.b == 0xB1B1B1B1U);
                // The Heap is torn down after the walk, not before.
                expect(visit.c == 0xFEEDF00DU);
                expect(pdtor.kind == 3);
                expect(pdtor.target == &payloads[i]);
                expect(pdtor.a == 1U);   // deleting flag
                expect(edtor.kind == 4);
                expect(edtor.target == &entries[i]);
                expect(edtor.a == 1U);
                expect(entries[i].payload == 0);
            }
        }
    }

    // The scalar deleting destructor across modes: full destructor always,
    // operator delete on bit 0 only, always returns the object.
    const unsigned int modes[] = {0U, 1U, 2U};
    for (unsigned int mode : modes) {
        alignas(Dialog) uint8_t storage[sizeof(Dialog) + 32];
        uint8_t expected[sizeof(storage)];
        seed_storage(storage, expected, sizeof(storage));

        uint32_t vbtable[2] = {0xAAAAAAAAU, 0x24U};
        write_at(storage, 16 + 0x08, 0U);
        write_at(storage, 16 + 0xC0, reinterpret_cast<uint32_t>(vbtable));
        write_at(storage, 16 + 0xC4, 0U);
        write_at(storage, 16 + 0xCC, 0U);
        write_at(storage, 16 + 0xE8, 0x1234ABCDU);
        std::memcpy(expected, storage, sizeof(storage));

        write_at(expected, 16 + 0x00, DialogPrimaryVtable);
        expect_heap_clear(expected, 16 + 0x04);
        write_at(expected, 16 + 0xBC, DialogListVtable);
        write_at(expected, 16 + 0xD0, 0U);
        write_at(expected, 16 + 0xE4, DialogVirtualBaseFinalVtable);

        auto *self = reinterpret_cast<Dialog *>(storage + 16);
        dialog_dtor_base = storage + 16;
        dialog_dtor_event_count = 0;
        published_slot = 0;

        void *const result = dialog_scalar_dtor_redirect(self, nullptr, mode);
        expect(result == self);
        expect_storage_bytes(storage, expected, sizeof(storage));
        expect(published_slot == 0x1234ABCDU);

        const bool deletes = (mode & 1U) != 0U;
        expect(dialog_dtor_event_count == (deletes ? 2 : 1));
        expect(dialog_dtor_events[0].kind == 1);
        if (deletes) {
            expect(dialog_dtor_events[1].kind == 5);
            expect(dialog_dtor_events[1].target == self);
        }
    }

    DialogOriginalClose = saved_close;
    DialogOperatorDelete = saved_delete;
    DialogPublishedGlobal = saved_published;
    DialogListDerivedVtable = saved_derived;
    DialogListDerivedVirtualBaseVtable = saved_derived_vb;
    DialogListVtable = saved_list;
    DialogListVirtualBaseVtable = saved_list_vb;
}

// Event log for the Dialogs destructor chain. kind: 1 EditGroup dtor,
// 2 SpriteBox dtor, 3 CheckBox dtor, 4 RadioButton's Dialog close, 5 Win
// close, 6 Buffer close, 7 ListBox's Dialog close, 8 Dialog close (scalar
// path), 9 Win dtor, 10 Buffer dtor, 11 operator delete.
struct DialogsDtorEvent {
    int kind;
    const void *target;
    // Snapshots of the SHARED base slots at call time: the staging blocks
    // overwrite one another, so the intermediate tables are observable only
    // from inside the calls that run between them.
    uint32_t primary;        // [obj + g]
    uint32_t buffer;         // [obj + g + 0x444]
    uint32_t win;            // [obj + d]
    int32_t primary_adjust;  // [obj + g - 4]
    int32_t win_adjust;      // [obj + d - 4]
};

DialogsDtorEvent dialogs_dtor_events[24];

int dialogs_dtor_event_count = 0;

uint8_t *dialogs_dtor_base = nullptr;

int32_t dialogs_dtor_g = 0;

int32_t dialogs_dtor_d = 0;

void dialogs_dtor_record(int kind, const void *target) {
    const int index = dialogs_dtor_event_count++;
    if (index >= static_cast<int>(ARRAYSIZE(dialogs_dtor_events))) {
        return;
    }
    DialogsDtorEvent &event = dialogs_dtor_events[index];
    event.kind = kind;
    event.target = target;
    std::memcpy(&event.primary, dialogs_dtor_base + dialogs_dtor_g, 4);
    std::memcpy(&event.buffer, dialogs_dtor_base + dialogs_dtor_g + 0x444, 4);
    std::memcpy(&event.win, dialogs_dtor_base + dialogs_dtor_d, 4);
    std::memcpy(&event.primary_adjust,
                dialogs_dtor_base + dialogs_dtor_g - 4, 4);
    std::memcpy(&event.win_adjust, dialogs_dtor_base + dialogs_dtor_d - 4, 4);
}

void __thiscall dialogs_editgroup_probe(void *s) { dialogs_dtor_record(1, s); }

void __thiscall dialogs_spritebox_probe(void *s) { dialogs_dtor_record(2, s); }

void __thiscall dialogs_checkbox_probe(void *s) { dialogs_dtor_record(3, s); }

void __thiscall dialogs_rb_dialog_close_probe(Dialog *s) {
    dialogs_dtor_record(4, s);
}

void __thiscall dialogs_win_close_probe(void *s) { dialogs_dtor_record(5, s); }

void __thiscall dialogs_buffer_close_probe(void *s) {
    dialogs_dtor_record(6, s);
}

void __thiscall dialogs_lb_dialog_close_probe(Dialog *s) {
    dialogs_dtor_record(7, s);
}

void __thiscall dialogs_dialog_close_probe(Dialog *s) {
    dialogs_dtor_record(8, s);
}

void __thiscall dialogs_win_dtor_probe(void *s) { dialogs_dtor_record(9, s); }

void __thiscall dialogs_buffer_dtor_probe(void *s) {
    dialogs_dtor_record(10, s);
}

void __cdecl dialogs_delete_probe(void *s) { dialogs_dtor_record(11, s); }

void test_dialogs_destructor() {
    // Save every seam and default this recovery's chain reaches: the three
    // widget destructor seams and operator delete (this recovery's own), plus
    // the deep seams inside the recovered RadioButton::close, GraphicWin
    // close/destructor, ListBox teardown and Dialog::destroy bodies.
    func_dialogs_teardown *const saved_eg = DialogsEditGroupDestructor;
    func_dialogs_teardown *const saved_sb = DialogsSpriteBoxDestructor;
    func_dialogs_teardown *const saved_cb = DialogsCheckBoxDestructor;
    func_operator_delete *const saved_delete = DialogsOperatorDelete;
    func_dialog_close *const saved_rb_close = RadioButtonOriginalDialogClose;
    uint32_t *const saved_rb_d1 = RadioButtonDefault1;
    uint32_t *const saved_rb_d2 = RadioButtonDefault2;
    func_subobject_close *const saved_win = WinOriginalClose;
    func_subobject_close *const saved_buffer = BufferSubobjectClose;
    uint32_t *const saved_a0c = GraphicWinFieldA0CDefault;
    func_dialog_close *const saved_lb_close = ListBoxOriginalDialogClose;
    uint32_t *const saved_lb_static = ListBoxCloseStaticDefaults;
    uint32_t *const saved_lb_dynamic = ListBoxCloseDynamicDefault;
    func_dialog_close *const saved_dlg_close = DialogOriginalClose;
    uint32_t *const saved_dlg_published = DialogPublishedGlobal;
    func_subobject_destructor *const saved_win_dtor = WinOriginalDestructor;
    func_subobject_destructor *const saved_buffer_dtor =
        BufferSubobjectDestructor;

    uint32_t rb_default_1 = 0x51D10001U;
    uint32_t rb_default_2 = 0x51D10002U;
    uint32_t a0c_default = 0x7B3D19E5U;
    uint32_t lb_statics[4] = {0x51A70000U, 0x51A70001U, 0x51A70002U,
                              0x51A70003U};
    uint32_t lb_dynamic = 0xB16B00B5U;
    uint32_t dialog_published_slot = 0;

    DialogsEditGroupDestructor = &dialogs_editgroup_probe;
    DialogsSpriteBoxDestructor = &dialogs_spritebox_probe;
    DialogsCheckBoxDestructor = &dialogs_checkbox_probe;
    DialogsOperatorDelete = &dialogs_delete_probe;
    RadioButtonOriginalDialogClose = &dialogs_rb_dialog_close_probe;
    RadioButtonDefault1 = &rb_default_1;
    RadioButtonDefault2 = &rb_default_2;
    WinOriginalClose = &dialogs_win_close_probe;
    BufferSubobjectClose = &dialogs_buffer_close_probe;
    GraphicWinFieldA0CDefault = &a0c_default;
    ListBoxOriginalDialogClose = &dialogs_lb_dialog_close_probe;
    ListBoxCloseStaticDefaults = lb_statics;
    ListBoxCloseDynamicDefault = &lb_dynamic;
    DialogOriginalClose = &dialogs_dialog_close_probe;
    DialogPublishedGlobal = &dialog_published_slot;
    WinOriginalDestructor = &dialogs_win_dtor_probe;
    BufferSubobjectDestructor = &dialogs_buffer_dtor_probe;

    // ---- destructor across two vbtable shapes, direct and adapter ---------
    struct Shape { int32_t g; int32_t d; };
    const Shape shapes[2] = { {0x188, 0xBA0},     // most-derived
                              {0x1A8, 0xBC0} };   // embedded / shifted
    for (int s = 0; s < 2; ++s) {
        const int32_t g = shapes[s].g;
        const int32_t d = shapes[s].d;
        int32_t dlg_vbtable[3] = {0, g, d};
        // The embedded RadioButton's vbtable names the SHARED bases, so its
        // entries are the displacements from base+0x44.
        int32_t rb_vbtable[3] = {0, g - 0x44, d - 0x44};

        for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
            alignas(uint32_t) uint8_t storage[0xD40];
            alignas(uint32_t) uint8_t reference[0xD40];
            seed_storage(storage, reference, sizeof(storage));

            uint8_t *const obj = storage + 16;
            int32_t *const dlg_vbptr = dlg_vbtable;
            int32_t *const rb_vbptr = rb_vbtable;
            std::memcpy(obj + 0x00, &dlg_vbptr, sizeof(dlg_vbptr));
            std::memcpy(obj + 0x44, &rb_vbptr, sizeof(rb_vbptr));
            const uint32_t zero = 0;
            // GraphicWin::close's release target: keep the no-release path.
            std::memcpy(obj + g + 0xA08, &zero, sizeof(zero));
            std::memcpy(reference, storage, sizeof(storage));
            uint8_t *const ref_obj = reference + 16;

            // Compositional reference: hand-apply the two staging blocks this
            // recovery introduces, and run the already-recovered component
            // chain on the twin. Probe events from this build are discarded.
            dialogs_dtor_base = ref_obj;
            dialogs_dtor_g = g;
            dialogs_dtor_d = d;
            auto stage = [](uint8_t *b, int32_t g_disp, int32_t d_disp,
                            uint32_t t1, uint32_t t2, uint32_t t3,
                            int32_t own_g, int32_t own_d) {
                std::memcpy(b + g_disp, &t1, 4);
                std::memcpy(b + g_disp + 0x444, &t2, 4);
                std::memcpy(b + d_disp, &t3, 4);
                const int32_t ag = g_disp - own_g;
                const int32_t ad = d_disp - own_d;
                std::memcpy(b + g_disp - 4, &ag, 4);
                std::memcpy(b + d_disp - 4, &ad, 4);
            };
            stage(ref_obj, g, d, 0x00669BE8U, 0x00669BE0U, 0x00669BD4U,
                  0x188, 0xBA0);
            stage(ref_obj + 0x44, g - 0x44, d - 0x44, 0x00669A6CU,
                  0x00669A64U, 0x00669A58U, 0x18, 0xA30);
            reinterpret_cast<RadioButton *>(ref_obj + 0x44)->close();
            reinterpret_cast<ListBox *>(ref_obj)->destroy();

            dialogs_dtor_base = obj;
            dialogs_dtor_event_count = 0;
            uint32_t result;
            if (use_adapter) {
                result = dialogs_destructor_redirect(
                    obj + DialogsDestructorAdjustment, nullptr);
            } else {
                result = reinterpret_cast<Dialogs *>(obj)->destroy();
            }

            expect_storage_bytes(storage, reference, sizeof(storage));
            expect(result == 0U);

            // Order: the three widget seams at their fixed displacements,
            // then RadioButton::close (its Dialog close, then GraphicWin's
            // Win and Buffer closes through the RB vbtable), then ListBox's
            // teardown (GraphicWin again, then its Dialog close).
            expect(dialogs_dtor_event_count == 9);
            expect(dialogs_dtor_events[0].kind == 1);
            expect(dialogs_dtor_events[0].target == obj + 0xF8 + 0x8C);
            expect(dialogs_dtor_events[1].kind == 2);
            expect(dialogs_dtor_events[1].target == obj + 0x70 + 0x8C);
            expect(dialogs_dtor_events[2].kind == 3);
            expect(dialogs_dtor_events[2].target == obj + 0x58 + 0x1C);
            expect(dialogs_dtor_events[3].kind == 4);
            expect(dialogs_dtor_events[3].target == obj + d);
            expect(dialogs_dtor_events[4].kind == 5);
            expect(dialogs_dtor_events[4].target == obj + g);
            expect(dialogs_dtor_events[5].kind == 6);
            expect(dialogs_dtor_events[5].target == obj + g + 0x444);
            expect(dialogs_dtor_events[6].kind == 5);
            expect(dialogs_dtor_events[6].target == obj + g);
            expect(dialogs_dtor_events[7].kind == 6);
            expect(dialogs_dtor_events[7].target == obj + g + 0x444);
            expect(dialogs_dtor_events[8].kind == 7);
            expect(dialogs_dtor_events[8].target == obj + d);

            // Per-phase snapshots of the shared slots: the widget seams
            // observe the stage-one Dialogs tables, everything inside
            // RadioButton::close observes the RadioButton stage, and
            // everything inside the ListBox teardown observes its own.
            for (int e = 0; e < 3; ++e) {
                expect(dialogs_dtor_events[e].primary == 0x00669BE8U);
                expect(dialogs_dtor_events[e].buffer == 0x00669BE0U);
                expect(dialogs_dtor_events[e].win == 0x00669BD4U);
                expect(dialogs_dtor_events[e].primary_adjust == g - 0x188);
                expect(dialogs_dtor_events[e].win_adjust == d - 0xBA0);
            }
            for (int e = 3; e < 6; ++e) {
                expect(dialogs_dtor_events[e].primary == 0x00669A6CU);
                expect(dialogs_dtor_events[e].buffer == 0x00669A64U);
                expect(dialogs_dtor_events[e].win == 0x00669A58U);
                expect(dialogs_dtor_events[e].primary_adjust ==
                       (g - 0x44) - 0x18);
                expect(dialogs_dtor_events[e].win_adjust ==
                       (d - 0x44) - 0xA30);
            }
            for (int e = 6; e < 9; ++e) {
                expect(dialogs_dtor_events[e].primary == 0x0067041CU);
                expect(dialogs_dtor_events[e].buffer == 0x00670414U);
                expect(dialogs_dtor_events[e].win == 0x00670408U);
                expect(dialogs_dtor_events[e].primary_adjust == g - 0x48);
                expect(dialogs_dtor_events[e].win_adjust == d - 0xA60);
            }

            // The overwrite chain ends with ListBox's tables in the shared
            // slots and its adjust words in both fixup slots.
            uint32_t staged = 0;
            std::memcpy(&staged, obj + g, 4);
            expect(staged == 0x0067041CU);
            std::memcpy(&staged, obj + g + 0x444, 4);
            expect(staged == 0x00670414U);
            std::memcpy(&staged, obj + d, 4);
            expect(staged == 0x00670408U);
            int32_t adjust = 0;
            std::memcpy(&adjust, obj + g - 4, 4);
            expect(adjust == g - 0x48);
            std::memcpy(&adjust, obj + d - 4, 4);
            expect(adjust == d - 0xA60);
        }
    }

    // ---- scalar deleting destructor, most-derived shape, modes 0/1/2 ------
    // ??_GDialogs addresses the trailing Dialog and the GraphicWin virtual
    // base at fixed +0xBA0/+0x188: complete-object facts, so only the
    // most-derived shape applies.
    const unsigned int modes[] = {0U, 1U, 2U};
    for (unsigned int mode : modes) {
        alignas(uint32_t) uint8_t storage[0xD40];
        alignas(uint32_t) uint8_t reference[0xD40];
        seed_storage(storage, reference, sizeof(storage));

        int32_t dlg_vbtable[3] = {0, 0x188, 0xBA0};
        int32_t rb_vbtable[3] = {0, 0x144, 0xB5C};
        uint32_t dialog_list_vbtable[2] = {0xAAAAAAAAU, 0x24U};

        uint8_t *const obj = storage + 16;
        int32_t *const dlg_vbptr = dlg_vbtable;
        int32_t *const rb_vbptr = rb_vbtable;
        uint32_t *const list_vbptr = dialog_list_vbtable;
        std::memcpy(obj + 0x00, &dlg_vbptr, sizeof(dlg_vbptr));
        std::memcpy(obj + 0x44, &rb_vbptr, sizeof(rb_vbptr));
        const uint32_t zero = 0;
        std::memcpy(obj + 0x188 + 0xA08, &zero, sizeof(zero));
        // The trailing Dialog's own state: heap base_ null, its list empty
        // with its own vbtable, and a known virtual-base context word.
        std::memcpy(obj + 0xBA0 + 0x08, &zero, sizeof(zero));
        std::memcpy(obj + 0xBA0 + 0xC0, &list_vbptr, sizeof(list_vbptr));
        std::memcpy(obj + 0xBA0 + 0xC4, &zero, sizeof(zero));
        std::memcpy(obj + 0xBA0 + 0xCC, &zero, sizeof(zero));
        const uint32_t context = 0x1234ABCDU;
        std::memcpy(obj + 0xBA0 + 0xE8, &context, sizeof(context));
        std::memcpy(reference, storage, sizeof(storage));
        uint8_t *const ref_obj = reference + 16;

        // Reference: the ~Dialogs chain, then the recovered Dialog::destroy
        // and GraphicWin destructor on the twin.
        auto stage = [](uint8_t *b, int32_t g_disp, int32_t d_disp,
                        uint32_t t1, uint32_t t2, uint32_t t3, int32_t own_g,
                        int32_t own_d) {
            std::memcpy(b + g_disp, &t1, 4);
            std::memcpy(b + g_disp + 0x444, &t2, 4);
            std::memcpy(b + d_disp, &t3, 4);
            const int32_t ag = g_disp - own_g;
            const int32_t ad = d_disp - own_d;
            std::memcpy(b + g_disp - 4, &ag, 4);
            std::memcpy(b + d_disp - 4, &ad, 4);
        };
        dialogs_dtor_base = ref_obj;
        dialogs_dtor_g = 0x188;
        dialogs_dtor_d = 0xBA0;
        stage(ref_obj, 0x188, 0xBA0, 0x00669BE8U, 0x00669BE0U, 0x00669BD4U,
              0x188, 0xBA0);
        stage(ref_obj + 0x44, 0x144, 0xB5C, 0x00669A6CU, 0x00669A64U,
              0x00669A58U, 0x18, 0xA30);
        reinterpret_cast<RadioButton *>(ref_obj + 0x44)->close();
        reinterpret_cast<ListBox *>(ref_obj)->destroy();
        reinterpret_cast<Dialog *>(ref_obj + 0xBA0)->destroy();
        graphic_win_destructor_probe_reset();
        graphic_win_destructor_redirect(
            reinterpret_cast<GraphicWin *>(ref_obj + 0x188), nullptr);

        dialogs_dtor_base = obj;
        dialogs_dtor_event_count = 0;
        dialog_published_slot = 0;
        graphic_win_destructor_probe_reset();
        void *const result = dialogs_scalar_dtor_redirect(
            obj + DialogsDestructorAdjustment, nullptr, mode);

        expect(result == obj);
        expect_storage_bytes(storage, reference, sizeof(storage));
        expect(dialog_published_slot == 0x1234ABCDU);

        // ~Dialogs' nine events, then the Dialog close from Dialog::destroy,
        // the GraphicWin destructor's Buffer-then-Win subobject seams, and
        // operator delete on the allocation base only when bit 0 asks.
        const bool deletes = (mode & 1U) != 0U;
        expect(dialogs_dtor_event_count == 12 + (deletes ? 1 : 0));
        expect(dialogs_dtor_events[9].kind == 8);
        expect(dialogs_dtor_events[9].target == obj + 0xBA0);
        expect(dialogs_dtor_events[10].kind == 10);
        expect(dialogs_dtor_events[10].target == obj + 0x188 + 0x444);
        expect(dialogs_dtor_events[11].kind == 9);
        expect(dialogs_dtor_events[11].target == obj + 0x188);
        if (deletes) {
            expect(dialogs_dtor_events[12].kind == 11);
            expect(dialogs_dtor_events[12].target == obj);
        }
    }

    DialogsEditGroupDestructor = saved_eg;
    DialogsSpriteBoxDestructor = saved_sb;
    DialogsCheckBoxDestructor = saved_cb;
    DialogsOperatorDelete = saved_delete;
    RadioButtonOriginalDialogClose = saved_rb_close;
    RadioButtonDefault1 = saved_rb_d1;
    RadioButtonDefault2 = saved_rb_d2;
    WinOriginalClose = saved_win;
    BufferSubobjectClose = saved_buffer;
    GraphicWinFieldA0CDefault = saved_a0c;
    ListBoxOriginalDialogClose = saved_lb_close;
    ListBoxCloseStaticDefaults = saved_lb_static;
    ListBoxCloseDynamicDefault = saved_lb_dynamic;
    DialogOriginalClose = saved_dlg_close;
    DialogPublishedGlobal = saved_dlg_published;
    WinOriginalDestructor = saved_win_dtor;
    BufferSubobjectDestructor = saved_buffer_dtor;
}

void test_find_font() {
    // sizes and table each carry one entry past FontSizeTableCount: a decoy
    // the function must never read, sized so its style-slot pointer
    // arithmetic (index * 4) still lands inside table.
    int sizes[FontSizeTableCount + 1] = {
        8, 10, 12, 14, 16, 18, 20, 24, 28, 32, 40, 48, 9000,
    };
    Font table[(FontSizeTableCount + 1) * 4];
    int *const saved_sizes = FontSizeTable;
    Font *const saved_table = FontTable;
    FontSizeTable = sizes;
    FontTable = table;

    struct FontCase { int size; int style; int expected_index; };
    const FontCase cases[] = {
        {8, 0, 0}, {48, 3, 11},
        {13, 0, 2},    // ties resolve to the earlier entry: |12-13| < |14-13| is false
        {11, 1, 1},    // |10-11| == |12-11|, strict less-than keeps the first
        {0, 2, 0}, {-5, 3, 0},
        // The 9999 seed is a threshold: only when every candidate is at
        // least that far away does the lookup fall back to the first entry
        // instead of the nearest. The largest size is 48, so the boundary
        // sits at 48 + 9999.
        {100000, 0, 0}, {-100000, 1, 0},
        {10047, 0, 0},    // farthest delta is exactly 9999, strict < fails
        {10046, 1, 11},   // one closer, so the nearest entry wins
        {22, 0, 6},    // |20-22| == |24-22|, first wins
        {26, 1, 7},
        // Every real entry is 8952+ away from 9000, so the true bound must
        // stop at index 11 (size 48). Reading one past it would find the
        // decoy at index 12 with delta 0 and wrongly report that instead.
        {9000, 1, 11},
    };
    for (const FontCase &test : cases) {
        const Font *const result = find_font(test.size, test.style);
        expect(result == table + (test.style + test.expected_index * 4));
    }
    // The table is never modified by a lookup.
    FontSizeTable = saved_sizes;
    FontTable = saved_table;
}

void test_buffer_text_line_height() {
    struct HeightCase { int override_value; int line_height; int height; int expected; };
    const HeightCase cases[] = {
        {0, 111, 222, 222},          // zero is non-negative: height + 0
        {5, 111, 222, 227},
        {-1, 111, 222, 111},         // negative selects the natural line height
        {INT_MIN, 111, 222, 111},
        {INT_MAX, 111, 1, static_cast<int>(0x80000000U)},   // wraps
    };
    for (const HeightCase &test : cases) {
        for (int preset_font = 0; preset_font < 2; ++preset_font) {
            for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
                alignas(Buffer) uint8_t storage[sizeof(Buffer) + 32];
                uint8_t expected[sizeof(storage)];
                seed_storage(storage, expected, sizeof(storage));

                alignas(Font) uint8_t font_storage[sizeof(Font)] = {};
                int *const fields = reinterpret_cast<int *>(font_storage);
                fields[0x00 / 4] = test.override_value;
                fields[0x0C / 4] = test.line_height;
                fields[0x10 / 4] = test.height;
                auto *font = reinterpret_cast<Font *>(font_storage);

                // The default-font global lives at a fixed address that is
                // only mapped inside the hybrid process, so the binding is
                // pointed at test storage.
                Font **const saved_default_ptr = FontDefaultPtr;
                Font *default_font = font;
                FontDefaultPtr = &default_font;
                if (preset_font) {
                    write_at(storage, 16 + 0x52C, font);
                } else {
                    write_at(storage, 16 + 0x52C, 0U);
                }
                std::memcpy(expected, storage, sizeof(storage));
                // A missing font is filled in from the global default.
                write_at(expected, 16 + 0x52C, font);

                auto *buffer = reinterpret_cast<Buffer *>(storage + 16);
                const int result = use_adapter
                    ? buffer_text_line_height_redirect(buffer, nullptr)
                    : buffer->text_line_height();
                FontDefaultPtr = saved_default_ptr;

                expect(result == test.expected);
                expect_storage_bytes(storage, expected, sizeof(storage));
            }
        }
    }
}

int pull_down_free_calls = 0;

void *pull_down_free_targets[128];

void *pull_down_free_probe(void *block) {
    if (pull_down_free_calls < 128) {
        pull_down_free_targets[pull_down_free_calls] = block;
    }
    ++pull_down_free_calls;
    return nullptr;
}

void test_pull_down_destructor() {
    alignas(PullDown) uint8_t storage[sizeof(PullDown) + 32];
    uint8_t expected[sizeof(storage)];
    seed_storage(storage, expected, sizeof(storage));
    auto *self = reinterpret_cast<PullDown *>(storage + 16);

    // The seed pattern would make every text slot look allocated; zero all
    // 128 owned-string slots first, then plant sentinels in a shape that
    // distinguishes the two slots per item and proves the walk reaches the
    // final item: text only, right only, both, and both at index 63.
    const uint32_t zero = 0;
    for (size_t index = 0; index < 64; ++index) {
        write_at(storage, 16 + 0xA18 + index * 0x14, zero);
        write_at(storage, 16 + 0xA1C + index * 0x14, zero);
    }
    const uint32_t text_0 = 0x11110001U;
    const uint32_t right_1 = 0x22220002U;
    const uint32_t text_2 = 0x33330003U;
    const uint32_t right_2 = 0x44440004U;
    const uint32_t text_63 = 0x55550005U;
    const uint32_t right_63 = 0x66660006U;
    write_at(storage, 16 + 0xA18 + 0 * 0x14, text_0);
    write_at(storage, 16 + 0xA1C + 1 * 0x14, right_1);
    write_at(storage, 16 + 0xA18 + 2 * 0x14, text_2);
    write_at(storage, 16 + 0xA1C + 2 * 0x14, right_2);
    write_at(storage, 16 + 0xA18 + 63 * 0x14, text_63);
    write_at(storage, 16 + 0xA1C + 63 * 0x14, right_63);
    std::memcpy(expected, storage, sizeof(storage));

    // Final state: the GraphicWin delegation overwrites both vtable slots
    // with its own tables and clears 0xA10; the planted strings are freed
    // and nulled; dirty_ is set and the trailing pair reloads from the
    // rebindable defaults. Mnemonic slots keep their seeds untouched.
    write_at(expected, 16 + 0x000, GraphicWinPrimaryVtable);
    write_at(expected, 16 + 0x444, GraphicWinBufferVtable);
    write_at(expected, 16 + 0xA10, zero);
    write_at(expected, 16 + 0xA18 + 0 * 0x14, zero);
    write_at(expected, 16 + 0xA1C + 1 * 0x14, zero);
    write_at(expected, 16 + 0xA18 + 2 * 0x14, zero);
    write_at(expected, 16 + 0xA1C + 2 * 0x14, zero);
    write_at(expected, 16 + 0xA18 + 63 * 0x14, zero);
    write_at(expected, 16 + 0xA1C + 63 * 0x14, zero);
    const uint8_t dirty = 1;
    write_at(expected, 16 + 0xF34, dirty);
    uint32_t default_f38 = 0xCAFE0001U;
    uint32_t default_f3c = 0xCAFE0002U;
    write_at(expected, 16 + 0xF38, default_f38);
    write_at(expected, 16 + 0xF3C, default_f3c);

    func_sprite_free *const saved_free = PullDownFree;
    uint32_t *const saved_f38 = PullDownFieldF38Default;
    uint32_t *const saved_f3c = PullDownFieldF3CDefault;
    func_subobject_destructor *const saved_buffer = BufferSubobjectDestructor;
    func_subobject_destructor *const saved_win = WinOriginalDestructor;
    PullDownFree = pull_down_free_probe;
    PullDownFieldF38Default = &default_f38;
    PullDownFieldF3CDefault = &default_f3c;
    BufferSubobjectDestructor = graphic_win_stub_buffer_destructor;
    WinOriginalDestructor = graphic_win_stub_win_destructor;
    pull_down_free_calls = 0;
    graphic_win_destructor_probe_reset();
    graphic_win_stub_record = GraphicWinStubRecord{};

    expect(pull_down_destructor_redirect(self, nullptr) == self);

    PullDownFree = saved_free;
    PullDownFieldF38Default = saved_f38;
    PullDownFieldF3CDefault = saved_f3c;
    BufferSubobjectDestructor = saved_buffer;
    WinOriginalDestructor = saved_win;

    expect_storage_bytes(storage, expected, sizeof(storage));
    // Six frees in walk order, text before right-hand text within an item.
    expect(pull_down_free_calls == 6);
    expect(pull_down_free_targets[0] == reinterpret_cast<void *>(text_0));
    expect(pull_down_free_targets[1] == reinterpret_cast<void *>(right_1));
    expect(pull_down_free_targets[2] == reinterpret_cast<void *>(text_2));
    expect(pull_down_free_targets[3] == reinterpret_cast<void *>(right_2));
    expect(pull_down_free_targets[4] == reinterpret_cast<void *>(text_63));
    expect(pull_down_free_targets[5] == reinterpret_cast<void *>(right_63));
    // The GraphicWin delegation actually ran, against this object.
    expect(graphic_win_stub_record.buffer_calls == 1);
    expect(graphic_win_stub_record.win_calls == 1);
    expect(graphic_win_stub_record.buffer_target
           == reinterpret_cast<void *>(storage + 16 + 0x444));
    expect(graphic_win_stub_record.win_target
           == reinterpret_cast<void *>(storage + 16));
    expect(graphic_win_stub_record.sequence == 0x21);
}

void test_base_button_default_setters() {
    // The three colour tiers interleave inside one table: slot s, tier t lives
    // at base + s * 0xC + t * 4. A local table stands in for the game's so the
    // test observes exact placement, and every one of the twelve cells is
    // compared after each call - a setter that strides wrongly, or writes a
    // sibling tier, moves a cell the caller never named.
    uint32_t table[12];
    uint32_t *const saved_table = BaseButtonDefaultTextColors;
    BaseButtonDefaultTextColors = table;

    struct TierCase {
        size_t tier;
        void (*member)(int, int, int, int);
        void (__cdecl *redirect)(int, int, int, int);
    };
    const TierCase tiers[] = {
        {0, &BaseButton::set_def_text_color,
         base_button_set_def_text_color_redirect},
        {1, &BaseButton::set_def_text_color2,
         base_button_set_def_text_color2_redirect},
        {2, &BaseButton::set_def_text_color3,
         base_button_set_def_text_color3_redirect},
    };
    const int colors[4] = {INT_MIN, -1, 0x5A5A5A5A, INT_MAX};
    for (const TierCase &test : tiers) {
        for (int adapter = 0; adapter < 2; ++adapter) {
            uint32_t expected[12];
            for (size_t index = 0; index < 12; ++index) {
                table[index] = 0xA5000000U ^ static_cast<uint32_t>(index);
                expected[index] = table[index];
            }
            for (size_t slot = 0; slot < 4; ++slot) {
                expected[(slot * 0xC + test.tier * 4) / 4] =
                    static_cast<uint32_t>(colors[slot]);
            }
            if (adapter) {
                test.redirect(colors[0], colors[1], colors[2], colors[3]);
            } else {
                test.member(colors[0], colors[1], colors[2], colors[3]);
            }
            for (size_t index = 0; index < 12; ++index) {
                expect(table[index] == expected[index]);
            }
        }
    }
    BaseButtonDefaultTextColors = saved_table;

    // set_def_font reads only Font::is_initialized(), an inline null check of
    // font_obj_ at offset 8, so raw storage stands in for constructed Fonts.
    alignas(Font) uint8_t ready_font[sizeof(Font)];
    alignas(Font) uint8_t unready_font[sizeof(Font)];
    std::memset(ready_font, 0xA5, sizeof(ready_font));
    std::memset(unready_font, 0xA5, sizeof(unready_font));
    const HFONT font_object = reinterpret_cast<HFONT>(0x1234U);
    const HFONT null_font_object = nullptr;
    write_at(ready_font, 0x08, font_object);
    write_at(unready_font, 0x08, null_font_object);
    auto *ready = reinterpret_cast<Font *>(ready_font);
    auto *unready = reinterpret_cast<Font *>(unready_font);
    auto *second = reinterpret_cast<Font *>(0x11111111U);
    auto *third = reinterpret_cast<Font *>(0x22222222U);

    Font *fonts[3];
    Font **const saved_fonts = BaseButtonDefaultFonts;
    BaseButtonDefaultFonts = fonts;

    // A null primary is rejected before anything is written.
    Font *const untouched = reinterpret_cast<Font *>(0x33333333U);
    for (int index = 0; index < 3; ++index) {
        fonts[index] = untouched;
    }
    expect(BaseButton::set_def_font(nullptr, second, third) == 3);
    for (int index = 0; index < 3; ++index) {
        expect(fonts[index] == untouched);
    }

    // An initialized primary publishes all three slots.
    for (int index = 0; index < 3; ++index) {
        fonts[index] = untouched;
    }
    expect(BaseButton::set_def_font(ready, second, third) == 0);
    expect(fonts[0] == ready);
    expect(fonts[1] == second);
    expect(fonts[2] == third);

    // An uninitialized primary leaves slot zero alone, still stores the other
    // two, and still reports success.
    for (int index = 0; index < 3; ++index) {
        fonts[index] = untouched;
    }
    expect(base_button_set_def_font_redirect(unready, second, third) == 0);
    expect(fonts[0] == untouched);
    expect(fonts[1] == second);
    expect(fonts[2] == third);

    BaseButtonDefaultFonts = saved_fonts;
}

// Self-registration.  Each case carries its baseline position in the original
// main() call list, so the run order does not depend on link or static-init
// order.
namespace {
LEAF_CASE(58, test_scroll_destructor);
LEAF_CASE(23, test_sprite_close);
LEAF_CASE(24, test_buffer_get_data);
LEAF_CASE(25, test_buffer_free_data);
LEAF_CASE(26, test_string_struct_remove_all);
LEAF_CASE(27, test_string_list_destructor);
LEAF_CASE(28, test_dialog_destructor);
LEAF_CASE(29, test_dialogs_destructor);
LEAF_CASE(30, test_find_font);
LEAF_CASE(31, test_buffer_text_line_height);
LEAF_CASE(101, test_pull_down_destructor);
LEAF_CASE(102, test_base_button_default_setters);
}  // namespace
