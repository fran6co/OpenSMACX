// Recovery leaf tests: buffer_and_font.
//
// Carved verbatim out of tests/recovery_leaf_tests.cpp.  Add a case by
// appending a `void test_*()` here plus `LEAF_CASE(LEAF_APPEND, test_*);` at
// the bottom.  This file is the only file that edit touches: the per-family
// case counts are derived from these `LEAF_CASE(` lines at build time, and
// LEAF_APPEND needs no offset because ties there break by name.
#include "leaf_shared.h"

#define LEAF_FAMILY "buffer_and_font"

// Stand-in DirectDraw surface and Buffer vtables for the device-context
// protocol. The surface's GetDC/ReleaseDC slots record their arguments and
// return a configurable status; the Buffer's slot 4 records that the error
// path fired.
struct HdcProbe {
    int get_calls;
    int release_calls;
    int error_calls;
    void *get_surface;
    void *release_surface;
    HDC released_handle;
    long get_status;
    long release_status;
    HDC produced_handle;
};

HdcProbe hdc_probe = {};

long __stdcall hdc_probe_get_dc(void *surface, void *out) {
    ++hdc_probe.get_calls;
    hdc_probe.get_surface = surface;
    std::memcpy(out, &hdc_probe.produced_handle, sizeof(HDC));
    return hdc_probe.get_status;
}

long __stdcall hdc_probe_release_dc(void *surface, void *handle) {
    ++hdc_probe.release_calls;
    hdc_probe.release_surface = surface;
    hdc_probe.released_handle = reinterpret_cast<HDC>(handle);
    return hdc_probe.release_status;
}

#if defined(__GNUC__)

#pragma GCC diagnostic push

#pragma GCC diagnostic ignored "-Wattributes"

#endif

void __thiscall hdc_probe_buffer_virtual(void *) {
    ++hdc_probe.error_calls;
}

#if defined(__GNUC__)

#pragma GCC diagnostic pop

#endif

void test_buffer_hdc_protocol() {
    alignas(Buffer) uint8_t storage[sizeof(Buffer) + 32];
    uint8_t expected[sizeof(storage)];
    auto *buffer = reinterpret_cast<Buffer *>(storage + 16);

    void *buffer_vtable[4] = {};
    buffer_vtable[1] = reinterpret_cast<void *>(&hdc_probe_buffer_virtual);
    void *surface_vtable[0x6C / sizeof(void *)] = {};
    surface_vtable[0x44 / sizeof(void *)] =
        reinterpret_cast<void *>(&hdc_probe_get_dc);
    surface_vtable[0x68 / sizeof(void *)] =
        reinterpret_cast<void *>(&hdc_probe_release_dc);
    void **const buffer_vtable_ptr = buffer_vtable;
    void *surface_object = surface_vtable;
    void **surface = &surface_object;

    const HDC direct_handle = reinterpret_cast<HDC>(0x11110000U);
    const HDC surface_handle = reinterpret_cast<HDC>(0x22220000U);
    const uint32_t zero = 0;

    // Acquire with no surface: the stored handle is published, counted, and
    // returned without any surface traffic.
    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x00, buffer_vtable_ptr);
    write_at(storage, 16 + 0x50, zero);
    write_at(storage, 16 + 0x58, zero);
    write_at(storage, 16 + 0x60, static_cast<HDC>(nullptr));
    write_at(storage, 16 + 0x64, direct_handle);
    write_at(storage, 16 + 0x68, zero);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0x60, direct_handle);
    write_at(expected, 16 + 0x68, 1U);
    hdc_probe = HdcProbe{};
    expect(buffer->get_hdc() == direct_handle);
    expect_storage_bytes(storage, expected, sizeof(storage));
    expect(hdc_probe.get_calls == 0);
    expect(hdc_probe.error_calls == 0);

    // field_50_ set routes through the buffer's own slot 4 first.
    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x00, buffer_vtable_ptr);
    write_at(storage, 16 + 0x50, 1U);
    write_at(storage, 16 + 0x58, zero);
    write_at(storage, 16 + 0x60, static_cast<HDC>(nullptr));
    write_at(storage, 16 + 0x64, direct_handle);
    write_at(storage, 16 + 0x68, zero);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0x60, direct_handle);
    write_at(expected, 16 + 0x68, 1U);
    hdc_probe = HdcProbe{};
    expect(buffer->get_hdc() == direct_handle);
    expect_storage_bytes(storage, expected, sizeof(storage));
    expect(hdc_probe.error_calls == 1);

    // Surface present and no cached handle: GetDC runs once, its out-parameter
    // becomes the cached handle, and a nonzero status trips the error path.
    for (int failing = 0; failing < 2; ++failing) {
        seed_storage(storage, expected, sizeof(storage));
        write_at(storage, 16 + 0x00, buffer_vtable_ptr);
        write_at(storage, 16 + 0x50, zero);
        write_at(storage, 16 + 0x58, surface);
        write_at(storage, 16 + 0x60, static_cast<HDC>(nullptr));
        write_at(storage, 16 + 0x64, direct_handle);
        write_at(storage, 16 + 0x68, zero);
        std::memcpy(expected, storage, sizeof(storage));
        write_at(expected, 16 + 0x60, surface_handle);
        write_at(expected, 16 + 0x68, 1U);
        hdc_probe = HdcProbe{};
        hdc_probe.produced_handle = surface_handle;
        hdc_probe.get_status = failing ? 1 : 0;
        expect(buffer->get_hdc() == surface_handle);
        expect_storage_bytes(storage, expected, sizeof(storage));
        expect(hdc_probe.get_calls == 1);
        expect(hdc_probe.get_surface == surface);
        expect(hdc_probe.error_calls == (failing ? 1 : 0));
    }

    // A cached handle short-circuits: no GetDC, just another reference.
    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x00, buffer_vtable_ptr);
    write_at(storage, 16 + 0x50, zero);
    write_at(storage, 16 + 0x58, surface);
    write_at(storage, 16 + 0x60, surface_handle);
    write_at(storage, 16 + 0x68, 4U);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0x68, 5U);
    hdc_probe = HdcProbe{};
    expect(buffer->get_hdc() == surface_handle);
    expect_storage_bytes(storage, expected, sizeof(storage));
    expect(hdc_probe.get_calls == 0);

    // Release without a surface clears the handle only at or below zero.
    struct DirectCase { uint32_t start; int count; bool cleared; };
    const DirectCase direct_cases[] = {
        {5, 1, false}, {1, 1, true}, {3, 3, true}, {2, 5, true}, {9, 4, false},
        // remaining == 1 is the boundary: one reference is still out, so
        // the handle must survive. Without it, `< 1` and `<= 1` agree.
        {2, 1, false},
    };
    for (const DirectCase &test : direct_cases) {
        seed_storage(storage, expected, sizeof(storage));
        write_at(storage, 16 + 0x00, buffer_vtable_ptr);
        write_at(storage, 16 + 0x58, zero);
        write_at(storage, 16 + 0x60, direct_handle);
        write_at(storage, 16 + 0x68, test.start);
        std::memcpy(expected, storage, sizeof(storage));
        if (test.cleared) {
            write_at(expected, 16 + 0x60, static_cast<HDC>(nullptr));
            write_at(expected, 16 + 0x68, 0U);
        } else {
            write_at(expected, 16 + 0x68,
                     static_cast<uint32_t>(static_cast<int>(test.start)
                                           - test.count));
        }
        hdc_probe = HdcProbe{};
        buffer->release_hdc(test.count);
        expect_storage_bytes(storage, expected, sizeof(storage));
        expect(hdc_probe.release_calls == 0);
    }

    // With a surface, the last reference calls ReleaseDC with the cached
    // handle; a surviving reference must not.
    struct SurfaceCase { uint32_t start; int count; bool released; };
    const SurfaceCase surface_cases[] = {
        {1, 1, true}, {6, 2, false}, {2, 7, true},
        {2, 1, false},   // same boundary on the surface path
    };
    for (const SurfaceCase &test : surface_cases) {
        for (int failing = 0; failing < 2; ++failing) {
            seed_storage(storage, expected, sizeof(storage));
            write_at(storage, 16 + 0x00, buffer_vtable_ptr);
            write_at(storage, 16 + 0x58, surface);
            write_at(storage, 16 + 0x60, surface_handle);
            write_at(storage, 16 + 0x68, test.start);
            std::memcpy(expected, storage, sizeof(storage));
            if (test.released) {
                write_at(expected, 16 + 0x60, static_cast<HDC>(nullptr));
                write_at(expected, 16 + 0x68, 0U);
            } else {
                write_at(expected, 16 + 0x68,
                         static_cast<uint32_t>(static_cast<int>(test.start)
                                               - test.count));
            }
            hdc_probe = HdcProbe{};
            hdc_probe.release_status = failing ? 1 : 0;
            buffer_release_hdc_redirect(buffer, nullptr, test.count);
            expect_storage_bytes(storage, expected, sizeof(storage));
            expect(hdc_probe.release_calls == (test.released ? 1 : 0));
            if (test.released) {
                expect(hdc_probe.release_surface == surface);
                expect(hdc_probe.released_handle == surface_handle);
                expect(hdc_probe.error_calls == (failing ? 1 : 0));
            }
        }
    }

    // A null cached handle with a surface releases nothing but still counts.
    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x00, buffer_vtable_ptr);
    write_at(storage, 16 + 0x58, surface);
    write_at(storage, 16 + 0x60, static_cast<HDC>(nullptr));
    write_at(storage, 16 + 0x68, 1U);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0x68, 0U);
    hdc_probe = HdcProbe{};
    buffer->release_hdc(1);
    expect_storage_bytes(storage, expected, sizeof(storage));
    expect(hdc_probe.release_calls == 0);
}

void test_buffer_sync_to_palette() {
    alignas(Buffer) uint8_t storage[sizeof(Buffer) + 32];
    uint8_t expected[sizeof(storage)];
    auto *buffer = reinterpret_cast<Buffer *>(storage + 16);

    alignas(Palette) uint8_t palette_storage[sizeof(Palette)];
    auto *palette = reinterpret_cast<Palette *>(palette_storage);

    void *buffer_vtable[4] = {};
    buffer_vtable[1] = reinterpret_cast<void *>(&hdc_probe_buffer_virtual);
    void **const buffer_vtable_ptr = buffer_vtable;
    void *surface_vtable[0x6C / sizeof(void *)] = {};
    surface_vtable[0x44 / sizeof(void *)] =
        reinterpret_cast<void *>(&hdc_probe_get_dc);
    surface_vtable[0x68 / sizeof(void *)] =
        reinterpret_cast<void *>(&hdc_probe_release_dc);
    void *surface_object = surface_vtable;
    void **surface = &surface_object;

    LPVOID pixels[1] = {};
    LPVOID *const pixel_storage = pixels;
    const LPVOID *const no_pixel_storage = nullptr;
    const uint32_t zero = 0;
    const uint32_t tag = 0x1234ABCDU;

    int *const saved_initialized = PaletteInitialized;
    int initialized = 1;
    PaletteInitialized = &initialized;

    // No pixel storage: error 7 before anything at all is written.
    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x54, no_pixel_storage);
    std::memcpy(expected, storage, sizeof(storage));
    expect(buffer->sync_to_palette(palette) == 7);
    expect_storage_bytes(storage, expected, sizeof(storage));

    // Null palette: error 3, still before any write, even with storage.
    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x54, pixel_storage);
    std::memcpy(expected, storage, sizeof(storage));
    expect(buffer->sync_to_palette(nullptr) == 3);
    expect_storage_bytes(storage, expected, sizeof(storage));

    // Matching generation tag: the republish is skipped entirely - the colour
    // table is untouched - but the trailing pair is still published.
    std::memset(palette_storage, 0x5A, sizeof(palette_storage));
    write_at(palette_storage, 0x400, tag);
    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x54, pixel_storage);
    write_at(storage, 16 + 0x4A4, tag);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0x57C, 1U);
    write_at(expected, 16 + 0x584, palette);
    hdc_probe = HdcProbe{};
    expect(buffer_sync_to_palette_redirect(buffer, nullptr, palette) == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));
    expect(hdc_probe.get_calls == 0);

    // Differing tag with a surface that yields a null context: the tag is
    // cached, the table is republished from the palette, the context is
    // acquired and found null, so neither SetDIBColorTable nor the release
    // runs - but get_hdc still counted its reference.
    std::memset(palette_storage, 0, sizeof(palette_storage));
    for (uint32_t index = 0; index < 0x100; ++index) {
        // Palette entries are three bytes; get_rgbquad swaps red and blue
        // into the RGBQUAD it publishes.
        palette_storage[index * 4 + 0] = static_cast<uint8_t>(index);
        palette_storage[index * 4 + 1] = static_cast<uint8_t>(0xFF - index);
        palette_storage[index * 4 + 2] = static_cast<uint8_t>(index ^ 0x5AU);
    }
    write_at(palette_storage, 0x400, tag);
    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x00, buffer_vtable_ptr);
    write_at(storage, 16 + 0x50, zero);
    write_at(storage, 16 + 0x54, pixel_storage);
    write_at(storage, 16 + 0x58, surface);
    write_at(storage, 16 + 0x60, static_cast<HDC>(nullptr));
    write_at(storage, 16 + 0x68, zero);
    write_at(storage, 16 + 0x4A4, ~tag);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0x4A4, tag);
    write_at(expected, 16 + 0x68, 1U);
    write_at(expected, 16 + 0x57C, 1U);
    write_at(expected, 16 + 0x584, palette);
    for (uint32_t index = 0; index < 0x100; ++index) {
        const size_t cell = 16 + 0xA4 + index * 4;
        expected[cell + 2] = static_cast<uint8_t>(index);
        expected[cell + 1] = static_cast<uint8_t>(0xFF - index);
        expected[cell + 0] = static_cast<uint8_t>(index ^ 0x5AU);
        expected[cell + 3] = 0;
    }
    hdc_probe = HdcProbe{};
    hdc_probe.produced_handle = nullptr;
    expect(buffer->sync_to_palette(palette) == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));
    expect(hdc_probe.get_calls == 1);
    expect(hdc_probe.release_calls == 0);

    // A non-null context reaches the publish branch. SetDIBColorTable is a
    // real GDI import given a synthetic handle, so it fails harmlessly and its
    // effect is unobservable - but the release that follows is not: the
    // reference get_hdc took is handed back through the surface's ReleaseDC
    // and the count returns to zero.
    write_at(palette_storage, 0x400, tag);
    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x00, buffer_vtable_ptr);
    write_at(storage, 16 + 0x50, zero);
    write_at(storage, 16 + 0x54, pixel_storage);
    write_at(storage, 16 + 0x58, surface);
    write_at(storage, 16 + 0x60, static_cast<HDC>(nullptr));
    write_at(storage, 16 + 0x68, zero);
    write_at(storage, 16 + 0x4A4, ~tag);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0x4A4, tag);
    write_at(expected, 16 + 0x60, static_cast<HDC>(nullptr));
    write_at(expected, 16 + 0x68, 0U);
    write_at(expected, 16 + 0x57C, 1U);
    write_at(expected, 16 + 0x584, palette);
    for (uint32_t index = 0; index < 0x100; ++index) {
        const size_t cell = 16 + 0xA4 + index * 4;
        expected[cell + 2] = static_cast<uint8_t>(index);
        expected[cell + 1] = static_cast<uint8_t>(0xFF - index);
        expected[cell + 0] = static_cast<uint8_t>(index ^ 0x5AU);
        expected[cell + 3] = 0;
    }
    hdc_probe = HdcProbe{};
    hdc_probe.produced_handle = reinterpret_cast<HDC>(0x22220000U);
    expect(buffer->sync_to_palette(palette) == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));
    expect(hdc_probe.get_calls == 1);
    expect(hdc_probe.release_calls == 1);
    expect(hdc_probe.released_handle == reinterpret_cast<HDC>(0x22220000U));

    // An uninitialized palette makes get_rgbquad a no-op, so the table keeps
    // its seed while the tag and trailing pair are still published.
    initialized = 0;
    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x00, buffer_vtable_ptr);
    write_at(storage, 16 + 0x50, zero);
    write_at(storage, 16 + 0x54, pixel_storage);
    write_at(storage, 16 + 0x58, surface);
    write_at(storage, 16 + 0x60, static_cast<HDC>(nullptr));
    write_at(storage, 16 + 0x68, zero);
    write_at(storage, 16 + 0x4A4, ~tag);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0x4A4, tag);
    write_at(expected, 16 + 0x68, 1U);
    write_at(expected, 16 + 0x57C, 1U);
    write_at(expected, 16 + 0x584, palette);
    hdc_probe = HdcProbe{};
    expect(buffer->sync_to_palette(palette) == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));

    PaletteInitialized = saved_initialized;
}

void test_buffer_text_height() {
    alignas(Buffer) uint8_t storage[sizeof(Buffer) + 32];
    uint8_t expected[sizeof(storage)];
    auto *buffer = reinterpret_cast<Buffer *>(storage + 16);

    alignas(Font) uint8_t default_font[sizeof(Font)];
    alignas(Font) uint8_t cached_font[sizeof(Font)];
    std::memset(default_font, 0xA5, sizeof(default_font));
    std::memset(cached_font, 0xA5, sizeof(cached_font));
    const int default_height = 0x1234;
    const int cached_height = -99;
    write_at(default_font, 0x10, default_height);
    write_at(cached_font, 0x10, cached_height);
    auto *fallback = reinterpret_cast<Font *>(default_font);
    auto *cached = reinterpret_cast<Font *>(cached_font);

    Font **const saved_default = FontDefaultPtr;
    Font *default_slot = fallback;
    FontDefaultPtr = &default_slot;

    // No cached font: the process default is adopted and its height returned.
    for (int adapter = 0; adapter < 2; ++adapter) {
        seed_storage(storage, expected, sizeof(storage));
        write_at(storage, 16 + 0x52C, static_cast<Font *>(nullptr));
        std::memcpy(expected, storage, sizeof(storage));
        write_at(expected, 16 + 0x52C, fallback);
        const int result = adapter
            ? buffer_text_height_redirect(buffer, nullptr)
            : buffer->text_height();
        expect(result == default_height);
        expect_storage_bytes(storage, expected, sizeof(storage));
    }

    // A cached font is used as-is and must not be replaced, even though the
    // default is loaded first.
    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x52C, cached);
    std::memcpy(expected, storage, sizeof(storage));
    expect(buffer->text_height() == cached_height);
    expect_storage_bytes(storage, expected, sizeof(storage));

    FontDefaultPtr = saved_default;
}

void test_win_is_dialog_focus() {
    alignas(Win) uint8_t storage[sizeof(Win) + 32];
    alignas(Win) uint8_t parent_storage[sizeof(Win) + 32];
    uint8_t expected[sizeof(storage)];
    uint8_t parent_expected[sizeof(parent_storage)];
    auto *self = reinterpret_cast<Win *>(storage + 16);
    auto *parent = reinterpret_cast<Win *>(parent_storage + 16);

    // The focus list's entry at +4 names the focused window.
    uintptr_t focus_list[2] = {};
    // Bind an explicit pointer: passing the array to write_at deduces
    // uintptr_t[2] under `const T&` and copies both elements.
    uintptr_t *const focus_list_ptr = focus_list;
    const uint32_t zero = 0;
    const uint32_t nonzero = 1;

    struct FocusCase {
        uint32_t flags;
        bool has_parent;
        uint32_t list_count;
        int focus_target;   // 0 none, 1 self, 2 parent
        int expected;
    };
    const FocusCase cases[] = {
        // Bit 12 short-circuits before the parent is even consulted.
        {0x00001000U, false, 0, 0, 1},
        {0xFFFFFFFFU, false, 0, 0, 1},
        // Without the bit and without a parent there is no focus.
        {0x00000000U, false, 0, 0, 0},
        {0xFFFFEFFFU, false, 0, 0, 0},
        // An empty focus list reads as no focus and never dereferences it.
        {0x00000000U, true, 0, 1, 0},
        // A populated list naming this window grants focus.
        {0x00000000U, true, 1, 1, 1},
        // Naming a different window does not.
        {0x00000000U, true, 1, 2, 0},
        {0x00000000U, true, 1, 0, 0},
    };
    for (const FocusCase &test : cases) {
        for (int adapter = 0; adapter < 2; ++adapter) {
            seed_storage(storage, expected, sizeof(storage));
            seed_storage(parent_storage, parent_expected,
                         sizeof(parent_storage));
            write_at(storage, 16 + 0x98, test.flags);
            write_at(storage, 16 + 0xC4,
                     test.has_parent ? parent : static_cast<Win *>(nullptr));
            focus_list[0] = 0;
            focus_list[1] = test.focus_target == 1
                ? reinterpret_cast<uintptr_t>(self)
                : (test.focus_target == 2
                       ? reinterpret_cast<uintptr_t>(parent) : 0U);
            write_at(parent_storage, 16 + 0xCC,
                     test.list_count ? nonzero : zero);
            write_at(parent_storage, 16 + 0xD0, focus_list_ptr);
            std::memcpy(expected, storage, sizeof(storage));
            std::memcpy(parent_expected, parent_storage,
                        sizeof(parent_storage));
            const int result = adapter
                ? win_is_dialog_focus_redirect(self, nullptr)
                : self->is_dialog_focus();
            expect(result == test.expected);
            // The query must not modify either window.
            expect_storage_bytes(storage, expected, sizeof(storage));
            expect_storage_bytes(parent_storage, parent_expected,
                                 sizeof(parent_storage));
        }
    }
}

struct ClipProbe {
    int set_list_calls;
    int set_clipper_calls;
    void *set_list_this;
    void *set_clipper_this;
    void *set_clipper_arg;
    unsigned long set_list_flags;
    RGNDATAHEADER header;
    RECT rects[1];
};

ClipProbe clip_probe = {};

long __stdcall clip_probe_set_list(void *self, void *data, unsigned long flags) {
    ++clip_probe.set_list_calls;
    clip_probe.set_list_this = self;
    clip_probe.set_list_flags = flags;
    std::memcpy(&clip_probe.header, data, sizeof(RGNDATAHEADER));
    std::memcpy(clip_probe.rects,
                static_cast<const uint8_t *>(data) + sizeof(RGNDATAHEADER),
                sizeof(RECT));
    return 0;
}

long __stdcall clip_probe_set_clipper(void *self, void *clipper) {
    ++clip_probe.set_clipper_calls;
    clip_probe.set_clipper_this = self;
    clip_probe.set_clipper_arg = clipper;
    return 0;
}

void test_buffer_set_clip() {
    alignas(Buffer) uint8_t storage[sizeof(Buffer) + 32];
    uint8_t expected[sizeof(storage)];
    auto *buffer = reinterpret_cast<Buffer *>(storage + 16);

    void *buffer_vtable[4] = {};
    buffer_vtable[1] = reinterpret_cast<void *>(&hdc_probe_buffer_virtual);
    void **const buffer_vtable_ptr = buffer_vtable;
    void *surface_vtable[0x74 / sizeof(void *)] = {};
    surface_vtable[0x44 / sizeof(void *)] =
        reinterpret_cast<void *>(&hdc_probe_get_dc);
    surface_vtable[0x68 / sizeof(void *)] =
        reinterpret_cast<void *>(&hdc_probe_release_dc);
    surface_vtable[0x70 / sizeof(void *)] =
        reinterpret_cast<void *>(&clip_probe_set_clipper);
    void *surface_object = surface_vtable;
    void **surface = &surface_object;
    void *clipper_vtable[0x20 / sizeof(void *)] = {};
    clipper_vtable[0x1C / sizeof(void *)] =
        reinterpret_cast<void *>(&clip_probe_set_list);
    void *clipper_object = clipper_vtable;
    void **clipper = &clipper_object;

    LPVOID pixels[1] = {};
    LPVOID *const pixel_storage = pixels;
    const LPVOID *const no_pixels = nullptr;
    const uint32_t zero = 0;
    const RECT extent = {0, 0, 100, 50};

    // Neither pixel storage nor a surface: error 7, nothing written.
    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x54, no_pixels);
    write_at(storage, 16 + 0x58, zero);
    std::memcpy(expected, storage, sizeof(storage));
    RECT request = {10, 10, 20, 20};
    expect(buffer->set_clip(&request) == 7);
    expect_storage_bytes(storage, expected, sizeof(storage));

    // Null rectangle: error 3, still nothing written.
    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x54, pixel_storage);
    std::memcpy(expected, storage, sizeof(storage));
    expect(buffer->set_clip(nullptr) == 3);
    expect_storage_bytes(storage, expected, sizeof(storage));

    // A request disjoint from the extent yields an empty intersection: error 1
    // and, per IntersectRect, a zeroed destination rectangle.
    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x54, pixel_storage);
    write_at(storage, 16 + 0x58, zero);
    write_at(storage, 16 + 0x30, extent);
    write_at(storage, 16 + 0x60, static_cast<HDC>(nullptr));
    std::memcpy(expected, storage, sizeof(storage));
    const RECT empty_rect = {0, 0, 0, 0};
    write_at(expected, 16 + 0x20, empty_rect);
    RECT disjoint = {500, 500, 600, 600};
    expect(buffer->set_clip(&disjoint) == 1);
    expect_storage_bytes(storage, expected, sizeof(storage));

    // A partial overlap clips to the intersection. With no surface the buffer
    // owns its context directly, so get_hdc adopts it and release_hdc gives it
    // back, leaving the count at zero.
    const HDC direct_handle = reinterpret_cast<HDC>(0x33330000U);
    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x00, buffer_vtable_ptr);
    write_at(storage, 16 + 0x50, zero);
    write_at(storage, 16 + 0x54, pixel_storage);
    write_at(storage, 16 + 0x58, zero);
    write_at(storage, 16 + 0x30, extent);
    write_at(storage, 16 + 0x60, static_cast<HDC>(nullptr));
    write_at(storage, 16 + 0x64, direct_handle);
    write_at(storage, 16 + 0x68, zero);
    write_at(storage, 16 + 0x70, static_cast<HRGN>(nullptr));
    std::memcpy(expected, storage, sizeof(storage));
    const RECT clipped = {10, 10, 20, 20};
    write_at(expected, 16 + 0x20, clipped);
    write_at(expected, 16 + 0x60, static_cast<HDC>(nullptr));
    write_at(expected, 16 + 0x68, 0U);
    clip_probe = ClipProbe{};
    hdc_probe = HdcProbe{};
    RECT overlap = {10, 10, 20, 20};
    expect(buffer->set_clip(&overlap) == 0);
    // field_70_ now holds a real region handle, so compare around it.
    HRGN produced_region;
    std::memcpy(&produced_region, storage + 16 + 0x70, sizeof(produced_region));
    expect(produced_region != nullptr);
    write_at(expected, 16 + 0x70, produced_region);
    expect_storage_bytes(storage, expected, sizeof(storage));
    // No surface means no clipper traffic at all.
    expect(clip_probe.set_list_calls == 0);
    expect(clip_probe.set_clipper_calls == 0);
    DeleteObject(produced_region);

    // A clip equal to the full extent needs no region: the previously held
    // region is deleted and the slot cleared, and no replacement is created.
    // Every earlier case starts with an empty slot, so this is the only one
    // that exercises the cleanup branch.
    const RECT prior = {1, 2, 3, 4};
    HRGN existing = CreateRectRgnIndirect(&prior);
    expect(existing != nullptr);
    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x00, buffer_vtable_ptr);
    write_at(storage, 16 + 0x50, zero);
    write_at(storage, 16 + 0x54, pixel_storage);
    write_at(storage, 16 + 0x58, zero);
    write_at(storage, 16 + 0x30, extent);
    write_at(storage, 16 + 0x60, static_cast<HDC>(nullptr));
    write_at(storage, 16 + 0x64, direct_handle);
    write_at(storage, 16 + 0x68, zero);
    write_at(storage, 16 + 0x70, existing);
    std::memcpy(expected, storage, sizeof(storage));
    write_at(expected, 16 + 0x20, extent);
    write_at(expected, 16 + 0x60, static_cast<HDC>(nullptr));
    write_at(expected, 16 + 0x68, 0U);
    write_at(expected, 16 + 0x70, static_cast<HRGN>(nullptr));
    clip_probe = ClipProbe{};
    hdc_probe = HdcProbe{};
    RECT full = {0, 0, 100, 50};
    expect(buffer->set_clip(&full) == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));

    // With a surface, the clipper receives a single-rectangle RGNDATA whose
    // bound and only entry are both the clipped rectangle, and the surface is
    // then handed that clipper.
    seed_storage(storage, expected, sizeof(storage));
    write_at(storage, 16 + 0x00, buffer_vtable_ptr);
    write_at(storage, 16 + 0x50, zero);
    write_at(storage, 16 + 0x54, pixel_storage);
    write_at(storage, 16 + 0x58, surface);
    write_at(storage, 16 + 0x5C, clipper);
    write_at(storage, 16 + 0x30, extent);
    write_at(storage, 16 + 0x60, static_cast<HDC>(nullptr));
    write_at(storage, 16 + 0x68, zero);
    write_at(storage, 16 + 0x70, static_cast<HRGN>(nullptr));
    clip_probe = ClipProbe{};
    hdc_probe = HdcProbe{};
    hdc_probe.produced_handle = nullptr;   // GetDC yields nothing: skip the GDI work
    RECT overlap2 = {5, 5, 40, 30};
    expect(buffer->set_clip(&overlap2) == 0);
    RECT stored;
    std::memcpy(&stored, storage + 16 + 0x20, sizeof(stored));
    expect(stored.left == 5 && stored.top == 5);
    expect(stored.right == 40 && stored.bottom == 30);
    expect(clip_probe.set_list_calls == 1);
    expect(clip_probe.set_list_this == clipper);
    expect(clip_probe.set_list_flags == 0);
    expect(clip_probe.header.dwSize == sizeof(RGNDATAHEADER));
    expect(clip_probe.header.iType == RDH_RECTANGLES);
    expect(clip_probe.header.nCount == 1);
    expect(clip_probe.header.nRgnSize == sizeof(RECT));
    expect(clip_probe.header.rcBound.left == 5);
    expect(clip_probe.header.rcBound.top == 5);
    expect(clip_probe.header.rcBound.right == 40);
    expect(clip_probe.header.rcBound.bottom == 30);
    expect(clip_probe.rects[0].left == 5);
    expect(clip_probe.rects[0].top == 5);
    expect(clip_probe.rects[0].right == 40);
    expect(clip_probe.rects[0].bottom == 30);
    expect(clip_probe.set_clipper_calls == 1);
    expect(clip_probe.set_clipper_this == surface);
    expect(clip_probe.set_clipper_arg == clipper);
}

void test_base_button_text_colors() {
    // The buffer member sits at +0x444 inside the GraphicWin base. Buffer
    // carries four interleaved colour tiers - the three these setters reach
    // plus the hyper tier - so slot s of tier t is at 0x53C + s * 0x10 + t * 4,
    // a 0x10 stride rather than the 0xC of a three-tier table.
    std::vector<uint8_t> button_bytes(sizeof(BaseButton) + 32);
    std::vector<uint8_t> expected_bytes(sizeof(BaseButton) + 32);
    uint8_t *const storage = button_bytes.data();
    uint8_t *const expected = expected_bytes.data();
    auto *button = reinterpret_cast<BaseButton *>(storage + 16);

    alignas(Palette) uint8_t palette_storage[sizeof(Palette)];
    std::memset(palette_storage, 0, sizeof(palette_storage));
    auto *palette = reinterpret_cast<Palette *>(palette_storage);
    Palette *palette_slot = palette;
    Palette **const saved_palette = BaseButtonActivePalette;
    BaseButtonActivePalette = &palette_slot;
    int *const saved_initialized = PaletteInitialized;
    int initialized = 0;   // get_rgbquad becomes a no-op
    PaletteInitialized = &initialized;

    constexpr size_t kBuffer = 0x444;
    const int colors[4] = {INT_MIN, -1, 0x5A5A5A5A, INT_MAX};
    const uint32_t zero = 0;
    LPVOID pixels[1] = {};
    LPVOID *const pixel_storage = pixels;

    struct TierCase {
        size_t tier;
        void (BaseButton::*member)(int, int, int, int);
        void (__fastcall *redirect)(BaseButton *, void *, int, int, int, int);
    };
    const TierCase tiers[] = {
        {0, &BaseButton::set_text_color, base_button_set_text_color_redirect},
        {1, &BaseButton::set_text_color2, base_button_set_text_color2_redirect},
        {2, &BaseButton::set_text_color3, base_button_set_text_color3_redirect},
    };

    for (const TierCase &test : tiers) {
        // No parent: the whole body is skipped, so not even the palette is
        // published and the object is untouched.
        seed_storage(storage, expected, button_bytes.size());
        write_at(storage, 16 + 0xC4, static_cast<Win *>(nullptr));
        std::memcpy(expected, storage, button_bytes.size());
        (button->*test.member)(colors[0], colors[1], colors[2], colors[3]);
        expect_storage_bytes(storage, expected, button_bytes.size());

        for (int adapter = 0; adapter < 2; ++adapter) {
            seed_storage(storage, expected, button_bytes.size());
            // A parent enables the body; the buffer needs pixel storage so
            // sync_to_palette gets past its own guard.
            write_at(storage, 16 + 0xC4, button);
            write_at(storage, 16 + kBuffer + 0x54, pixel_storage);
            write_at(storage, 16 + kBuffer + 0x58, zero);
            write_at(storage, 16 + kBuffer + 0x4A4, zero);
            std::memcpy(expected, storage, button_bytes.size());
            // sync_to_palette publishes the palette and its generation tag,
            // then the tier's four colours land at stride 0xC.
            write_at(expected, 16 + kBuffer + 0x4A4, zero);
            write_at(expected, 16 + kBuffer + 0x57C, 1U);
            write_at(expected, 16 + kBuffer + 0x584, palette);
            for (size_t slot = 0; slot < 4; ++slot) {
                write_at(expected,
                         16 + kBuffer + 0x53C + slot * 0x10 + test.tier * 4,
                         colors[slot]);
            }
            if (adapter) {
                test.redirect(button, nullptr,
                              colors[0], colors[1], colors[2], colors[3]);
            } else {
                (button->*test.member)(
                    colors[0], colors[1], colors[2], colors[3]);
            }
            expect_storage_bytes(storage, expected, button_bytes.size());
        }
    }

    PaletteInitialized = saved_initialized;
    BaseButtonActivePalette = saved_palette;
}

void test_default_font_setters() {
    // Every default-font setter in the codebase shares one shape: reject a
    // null primary with 3, publish the primary only when it is initialized,
    // store the remaining slots unconditionally, and return 0 either way.
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
    auto *fourth = reinterpret_cast<Font *>(0x44444444U);
    Font *const untouched = reinterpret_cast<Font *>(0x33333333U);

    Font *slots[4];
    auto reset = [&slots]() {
        for (int index = 0; index < 4; ++index) {
            slots[index] = reinterpret_cast<Font *>(0x33333333U);
        }
    };

    // The four-slot string setter.
    Font **const saved_string = BasePopDefaultStringFonts;
    BasePopDefaultStringFonts = slots;
    reset();
    expect(BasePop::set_def_string_font(nullptr, second, third, fourth) == 3);
    for (int index = 0; index < 4; ++index) {
        expect(slots[index] == untouched);
    }
    reset();
    expect(BasePop::set_def_string_font(ready, second, third, fourth) == 0);
    expect(slots[0] == ready);
    expect(slots[1] == second);
    expect(slots[2] == third);
    expect(slots[3] == fourth);
    reset();
    expect(base_pop_set_def_string_font_redirect(
               unready, second, third, fourth) == 0);
    expect(slots[0] == untouched);
    expect(slots[1] == second);
    expect(slots[2] == third);
    expect(slots[3] == fourth);
    BasePopDefaultStringFonts = saved_string;

    // The three-slot button setter must not touch a fourth slot.
    Font **const saved_button = BasePopDefaultButtonFonts;
    BasePopDefaultButtonFonts = slots;
    reset();
    expect(BasePop::set_def_button_font(nullptr, second, third) == 3);
    for (int index = 0; index < 4; ++index) {
        expect(slots[index] == untouched);
    }
    reset();
    expect(BasePop::set_def_button_font(ready, second, third) == 0);
    expect(slots[0] == ready);
    expect(slots[1] == second);
    expect(slots[2] == third);
    expect(slots[3] == untouched);
    reset();
    expect(base_pop_set_def_button_font_redirect(unready, second, third) == 0);
    expect(slots[0] == untouched);
    expect(slots[1] == second);
    expect(slots[2] == third);
    expect(slots[3] == untouched);
    BasePopDefaultButtonFonts = saved_button;

    // The dialog setter, which writes its own separate table.
    Font **const saved_dialog = DialogDefaultFonts;
    DialogDefaultFonts = slots;
    reset();
    expect(Dialog::set_def_dialog_font(nullptr, second, third) == 3);
    for (int index = 0; index < 4; ++index) {
        expect(slots[index] == untouched);
    }
    reset();
    expect(Dialog::set_def_dialog_font(ready, second, third) == 0);
    expect(slots[0] == ready);
    expect(slots[1] == second);
    expect(slots[2] == third);
    expect(slots[3] == untouched);
    reset();
    expect(dialog_set_def_dialog_font_redirect(unready, second, third) == 0);
    expect(slots[0] == untouched);
    expect(slots[1] == second);
    expect(slots[2] == third);
    expect(slots[3] == untouched);
    DialogDefaultFonts = saved_dialog;
}

void test_win_shared_hdc() {
    // The shared context lives in three process globals rather than on an
    // instance, so the fixture rebinds all of them plus the window handle.
    int refcount = 0;
    HDC shared = nullptr;
    void *surface_object = nullptr;
    HWND window = nullptr;

    int *const saved_count = WinHdcRefCount;
    HDC *const saved_shared = WinSharedHdc;
    void **const saved_surface = WinHdcSurface;
    HWND *const saved_window = WinHdcWindow;
    WinHdcRefCount = &refcount;
    WinSharedHdc = &shared;
    WinHdcSurface = &surface_object;
    WinHdcWindow = &window;

    void *surface_vtable[0x6C / sizeof(void *)] = {};
    surface_vtable[0x44 / sizeof(void *)] =
        reinterpret_cast<void *>(&hdc_probe_get_dc);
    surface_vtable[0x68 / sizeof(void *)] =
        reinterpret_cast<void *>(&hdc_probe_release_dc);
    void *surface_vtable_object = surface_vtable;
    void **surface = &surface_vtable_object;
    const HDC surface_handle = reinterpret_cast<HDC>(0x77770000U);

    // First acquire through a surface: GetDC runs, the handle is published,
    // and the count starts at one.
    surface_object = surface;
    refcount = 0;
    shared = nullptr;
    hdc_probe = HdcProbe{};
    hdc_probe.produced_handle = surface_handle;
    expect(Win::get_hdc() == surface_handle);
    expect(refcount == 1);
    expect(shared == surface_handle);
    expect(hdc_probe.get_calls == 1);
    expect(hdc_probe.get_surface == surface);

    // A held context is counted again without touching the surface.
    hdc_probe = HdcProbe{};
    expect(win_get_hdc_redirect() == surface_handle);
    expect(refcount == 2);
    expect(hdc_probe.get_calls == 0);

    // Releasing back to one reference must not release the handle.
    hdc_probe = HdcProbe{};
    Win::release_hdc();
    expect(refcount == 1);
    expect(shared == surface_handle);
    expect(hdc_probe.release_calls == 0);

    // The last reference releases through the surface and clears the handle.
    hdc_probe = HdcProbe{};
    win_release_hdc_redirect();
    expect(refcount == 0);
    expect(shared == nullptr);
    expect(hdc_probe.release_calls == 1);
    expect(hdc_probe.release_surface == surface);
    expect(hdc_probe.released_handle == surface_handle);

    // A surface that yields nothing leaves the count at zero, so the next
    // call retries the acquire rather than handing back a null handle as
    // though it were held.
    refcount = 0;
    shared = nullptr;
    hdc_probe = HdcProbe{};
    hdc_probe.produced_handle = nullptr;
    expect(Win::get_hdc() == nullptr);
    expect(refcount == 0);
    hdc_probe = HdcProbe{};
    hdc_probe.produced_handle = surface_handle;
    expect(Win::get_hdc() == surface_handle);
    expect(refcount == 1);
    expect(hdc_probe.get_calls == 1);
    // Leave the count balanced.
    hdc_probe = HdcProbe{};
    Win::release_hdc();
    expect(refcount == 0);

    // Over-releasing drives the count negative and, because the legacy body
    // tests for exactly zero, performs no release at all.
    refcount = 0;
    shared = surface_handle;
    hdc_probe = HdcProbe{};
    Win::release_hdc();
    expect(refcount == -1);
    expect(shared == surface_handle);
    expect(hdc_probe.release_calls == 0);

    // Without a surface the context comes from the window. GetDC(nullptr)
    // yields a real screen context under Wine, which the release returns.
    surface_object = nullptr;
    refcount = 0;
    shared = nullptr;
    hdc_probe = HdcProbe{};
    const HDC window_context = Win::get_hdc();
    expect(hdc_probe.get_calls == 0);
    // Asserted unconditionally: GetDC(nullptr) yields the screen context, so
    // a conditional check here would simply follow a mutant that dropped the
    // acquire into its own else branch and pass.
    expect(window_context != nullptr);
    expect(refcount == 1);
    expect(shared == window_context);
    Win::release_hdc();
    expect(refcount == 0);
    expect(shared == nullptr);

    WinHdcWindow = saved_window;
    WinHdcSurface = saved_surface;
    WinSharedHdc = saved_shared;
    WinHdcRefCount = saved_count;
}

struct SpriteDrawProbe {
    int calls;
    Sprite *self;
    Buffer *buffer;
    int a, b, c;
    int origin_x_during;
    int origin_y_during;
    int result;
};

SpriteDrawProbe sprite_draw_probe = {};

#if defined(__GNUC__)

#pragma GCC diagnostic push

#pragma GCC diagnostic ignored "-Wattributes"

#endif

int __thiscall sprite_draw_probe_body(
        Sprite *self, Buffer *buffer, int a, int b, int c) {
    ++sprite_draw_probe.calls;
    sprite_draw_probe.self = self;
    sprite_draw_probe.buffer = buffer;
    sprite_draw_probe.a = a;
    sprite_draw_probe.b = b;
    sprite_draw_probe.c = c;
    // The substituted origin must be visible from inside the call.
    sprite_draw_probe.origin_x_during = *SpriteDrawOriginX;
    sprite_draw_probe.origin_y_during = *SpriteDrawOriginY;
    return sprite_draw_probe.result;
}

#if defined(__GNUC__)

#pragma GCC diagnostic pop

#endif

void test_sprite_draw_origin() {
    alignas(Sprite) uint8_t storage[sizeof(Sprite) + 32];
    uint8_t expected[sizeof(storage)];
    auto *sprite = reinterpret_cast<Sprite *>(storage + 16);
    auto *buffer = reinterpret_cast<Buffer *>(0x12340000U);

    int origin_x = 0;
    int origin_y = 0;
    int *const saved_x_ptr = SpriteDrawOriginX;
    int *const saved_y_ptr = SpriteDrawOriginY;
    func_sprite_draw_original *const saved_body = SpriteDrawOriginal;
    SpriteDrawOriginX = &origin_x;
    SpriteDrawOriginY = &origin_y;
    SpriteDrawOriginal = &sprite_draw_probe_body;

    struct DrawCase { int prior_x, prior_y, x, y, result; };
    const DrawCase cases[] = {
        {0, 0, 5, 7, 0},
        {111, 222, -1, -2, 42},
        {INT_MIN, INT_MAX, INT_MAX, INT_MIN, -7},
        {9, 9, 9, 9, 1},
    };
    for (const DrawCase &test : cases) {
        for (int adapter = 0; adapter < 2; ++adapter) {
            seed_storage(storage, expected, sizeof(storage));
            std::memcpy(expected, storage, sizeof(storage));
            origin_x = test.prior_x;
            origin_y = test.prior_y;
            sprite_draw_probe = SpriteDrawProbe{};
            sprite_draw_probe.result = test.result;
            const int returned = adapter
                ? sprite_draw_redirect(sprite, nullptr, buffer,
                                       1, 2, 3, test.x, test.y)
                : sprite->draw(buffer, 1, 2, 3, test.x, test.y);
            // The overload sees the substituted origin and this sprite.
            expect(sprite_draw_probe.calls == 1);
            expect(sprite_draw_probe.self == sprite);
            expect(sprite_draw_probe.buffer == buffer);
            expect(sprite_draw_probe.a == 1);
            expect(sprite_draw_probe.b == 2);
            expect(sprite_draw_probe.c == 3);
            expect(sprite_draw_probe.origin_x_during == test.x);
            expect(sprite_draw_probe.origin_y_during == test.y);
            // The origin is restored exactly, and the result passes through.
            expect(origin_x == test.prior_x);
            expect(origin_y == test.prior_y);
            expect(returned == test.result);
            // The sprite itself is never touched.
            expect_storage_bytes(storage, expected, sizeof(storage));
        }
    }

    SpriteDrawOriginal = saved_body;
    SpriteDrawOriginY = saved_y_ptr;
    SpriteDrawOriginX = saved_x_ptr;
}

struct WrapperProbe {
    int cursor_calls;
    Win *cursor_window;
    int cursor_flag;
    int width_calls;
    Buffer *width_self;
    LPSTR width_text;
    size_t width_length;
    int width_result;
};

WrapperProbe wrapper_probe = {};

int __cdecl wrapper_probe_update_cursor(Win *window, int flag) {
    ++wrapper_probe.cursor_calls;
    wrapper_probe.cursor_window = window;
    wrapper_probe.cursor_flag = flag;
    return 0;
}

#if defined(__GNUC__)

#pragma GCC diagnostic push

#pragma GCC diagnostic ignored "-Wattributes"

#endif

int __thiscall wrapper_probe_text_width(Buffer *self, LPSTR text, size_t length) {
    ++wrapper_probe.width_calls;
    wrapper_probe.width_self = self;
    wrapper_probe.width_text = text;
    wrapper_probe.width_length = length;
    return wrapper_probe.width_result;
}

#if defined(__GNUC__)

#pragma GCC diagnostic pop

#endif

void test_win_set_cursor() {
    alignas(Win) uint8_t storage[sizeof(Win) + 32];
    uint8_t expected[sizeof(storage)];
    auto *win = reinterpret_cast<Win *>(storage + 16);

    func_win_update_cursor *const saved = WinUpdateCursorOriginal;
    WinUpdateCursorOriginal = &wrapper_probe_update_cursor;

    auto *const sentinel_sprite = reinterpret_cast<Sprite *>(0x11110000U);
    auto *const sentinel_handle = reinterpret_cast<HCURSOR *>(0x22220000U);

    // The accepted range is 0x7F00 through 0x7F8A inclusive; the boundaries
    // on both sides are the only values that distinguish the comparisons.
    struct CursorCase { int name; int expected; };
    const CursorCase cases[] = {
        {0x7EFF, 3},   // one below the range
        {0x7F00, 0},   // first accepted
        {0x7F45, 0},
        {0x7F8A, 0},   // last accepted
        {0x7F8B, 3},   // one above the range
        {0, 3}, {-1, 3}, {INT_MAX, 3}, {INT_MIN, 3},
    };
    for (const CursorCase &test : cases) {
        for (int adapter = 0; adapter < 2; ++adapter) {
            seed_storage(storage, expected, sizeof(storage));
            write_at(storage, 16 + 0x188, sentinel_sprite);
            write_at(storage, 16 + 0x194, sentinel_handle);
            std::memcpy(expected, storage, sizeof(storage));
            if (test.expected == 0) {
                write_at(expected, 16 + 0x188, static_cast<Sprite *>(nullptr));
                write_at(expected, 16 + 0x198, test.name);
                write_at(expected, 16 + 0x194, static_cast<HCURSOR *>(nullptr));
            }
            wrapper_probe = WrapperProbe{};
            const int result = adapter
                ? win_set_cursor_redirect(win, nullptr, test.name)
                : win->set_cursor(test.name);
            expect(result == test.expected);
            expect_storage_bytes(storage, expected, sizeof(storage));
            // A rejected name must not trigger the refresh either.
            expect(wrapper_probe.cursor_calls == (test.expected == 0 ? 1 : 0));
            if (test.expected == 0) {
                expect(wrapper_probe.cursor_window == nullptr);
                expect(wrapper_probe.cursor_flag == 1);
            }
        }
    }

    WinUpdateCursorOriginal = saved;
}

void test_buffer_text_width() {
    alignas(Buffer) uint8_t storage[sizeof(Buffer) + 32];
    uint8_t expected[sizeof(storage)];
    auto *buffer = reinterpret_cast<Buffer *>(storage + 16);

    func_buffer_text_width_measured *const saved = BufferTextWidthMeasured;
    BufferTextWidthMeasured = &wrapper_probe_text_width;

    // A null string is rejected without measuring anything.
    seed_storage(storage, expected, sizeof(storage));
    std::memcpy(expected, storage, sizeof(storage));
    wrapper_probe = WrapperProbe{};
    expect(buffer->text_width(nullptr) == 0);
    expect(wrapper_probe.width_calls == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));

    // Otherwise the string and its measured length are forwarded and the
    // result passes through. An empty string still measures.
    char empty[] = "";
    char text[] = "hello world";
    struct WidthCase { LPSTR text; size_t length; int result; };
    const WidthCase cases[] = {
        {empty, 0, 0},
        {text, 11, 4242},
        {text, 11, -1},
    };
    for (const WidthCase &test : cases) {
        for (int adapter = 0; adapter < 2; ++adapter) {
            seed_storage(storage, expected, sizeof(storage));
            std::memcpy(expected, storage, sizeof(storage));
            wrapper_probe = WrapperProbe{};
            wrapper_probe.width_result = test.result;
            const int result = adapter
                ? buffer_text_width_redirect(buffer, nullptr, test.text)
                : buffer->text_width(test.text);
            expect(result == test.result);
            expect(wrapper_probe.width_calls == 1);
            expect(wrapper_probe.width_self == buffer);
            expect(wrapper_probe.width_text == test.text);
            expect(wrapper_probe.width_length == test.length);
            expect_storage_bytes(storage, expected, sizeof(storage));
        }
    }

    BufferTextWidthMeasured = saved;
}

struct TeardownProbe {
    int net_close_calls;
    void *net_close_self;
    int update_calls;
    RECT *update_rect;
    Win *update_window;
    int flip_calls;
    RECT *flip_rect;
    int active_during_update;
    int companion_during_update;
    int sequence;
};

TeardownProbe teardown_probe = {};

#if defined(__GNUC__)

#pragma GCC diagnostic push

#pragma GCC diagnostic ignored "-Wattributes"

#endif

void __thiscall teardown_probe_net_close(void *self) {
    ++teardown_probe.net_close_calls;
    teardown_probe.net_close_self = self;
}

#if defined(__GNUC__)

#pragma GCC diagnostic pop

#endif

int __cdecl teardown_probe_update_screen(RECT *rect, Win *window) {
    ++teardown_probe.update_calls;
    teardown_probe.update_rect = rect;
    teardown_probe.update_window = window;
    // Both flags must already be cleared by the time the refresh runs.
    teardown_probe.active_during_update = *WinBubbleActive;
    teardown_probe.companion_during_update = *WinBubbleCompanion;
    teardown_probe.sequence = (teardown_probe.sequence << 4) | 1;
    return 0;
}

void __cdecl teardown_probe_flip(RECT *rect) {
    ++teardown_probe.flip_calls;
    teardown_probe.flip_rect = rect;
    teardown_probe.sequence = (teardown_probe.sequence << 4) | 2;
}

void test_alpha_net_close() {
    alignas(AlphaNet) uint8_t storage[sizeof(AlphaNet) + 32];
    uint8_t expected[sizeof(storage)];
    auto *network = reinterpret_cast<AlphaNet *>(storage + 16);

    func_net_close *const saved = NetCloseOriginal;
    NetCloseOriginal = &teardown_probe_net_close;

    for (int adapter = 0; adapter < 2; ++adapter) {
        seed_storage(storage, expected, sizeof(storage));
        std::memcpy(expected, storage, sizeof(storage));
        const uint32_t zero = 0;
        // Eight process slots at 0x78C, stride 0x19C, plus the slot at 0x768.
        for (size_t slot = 0; slot < 8; ++slot) {
            write_at(expected, 16 + 0x78C + slot * 0x19C, zero);
        }
        write_at(expected, 16 + 0x768, zero);
        teardown_probe = TeardownProbe{};
        if (adapter) {
            alpha_net_close_redirect(network, nullptr);
        } else {
            network->close();
        }
        expect_storage_bytes(storage, expected, sizeof(storage));
        // The tail jump hands the same object to the network close.
        expect(teardown_probe.net_close_calls == 1);
        expect(teardown_probe.net_close_self == network);
    }

    NetCloseOriginal = saved;
}

// Re-arms the bubble after each refresh, so a SECOND call to
// clear_bubble_text does observable work instead of early-returning. Without
// this the duplicate call in the four handlers below is invisible: the first
// call clears the flag and every later one returns immediately, so one call
// and two calls produce identical probe counts and the fixture would agree
// with a body that had dropped one of them.
int *g_rearm_active = nullptr;

int __cdecl rearming_probe_update_screen(RECT *rect, Win *window) {
    const int result = teardown_probe_update_screen(rect, window);
    if (g_rearm_active) {
        *g_rearm_active = 1;
    }
    return result;
}

void test_bubble_dismiss_handlers() {
    // Four handlers across three classes with byte-identical bodies:
    //     call Win::clear_bubble_text / call Win::clear_bubble_text / ret 8
    // They ignore both arguments and touch no field, so what has to be
    // established is the CALL COUNT and that nothing else moves.
    int active = 0;
    int companion = 0;
    RECT rect = {1, 2, 3, 4};
    int *const saved_active = WinBubbleActive;
    int *const saved_companion = WinBubbleCompanion;
    RECT *const saved_rect = WinBubbleRect;
    func_win_update_screen *const saved_update = WinUpdateScreenOriginal;
    func_win_flip *const saved_flip = WinFlipOriginal;
    WinBubbleActive = &active;
    WinBubbleCompanion = &companion;
    WinBubbleRect = &rect;
    WinUpdateScreenOriginal = &rearming_probe_update_screen;
    WinFlipOriginal = &teardown_probe_flip;
    g_rearm_active = &active;

    alignas(BaseWin) uint8_t base_storage[sizeof(BaseWin) + 32];
    alignas(DesignWin) uint8_t design_storage[sizeof(DesignWin) + 32];
    alignas(SocialWin) uint8_t social_storage[sizeof(SocialWin) + 32];
    uint8_t base_expected[sizeof(base_storage)];
    uint8_t design_expected[sizeof(design_storage)];
    uint8_t social_expected[sizeof(social_storage)];

    for (int which = 0; which < 4; ++which) {
        for (int adapter = 0; adapter < 2; ++adapter) {
            seed_storage(base_storage, base_expected, sizeof(base_storage));
            seed_storage(design_storage, design_expected, sizeof(design_storage));
            seed_storage(social_storage, social_expected, sizeof(social_storage));
            auto *base = reinterpret_cast<BaseWin *>(base_storage + 16);
            auto *design = reinterpret_cast<DesignWin *>(design_storage + 16);
            auto *social = reinterpret_cast<SocialWin *>(social_storage + 16);

            active = 1;
            companion = 0x5A5A5A5A;
            teardown_probe = TeardownProbe{};
            switch (which) {
            case 0:
                if (adapter) base_win_on_mouse_leave_redirect(base, nullptr, -1, 2147483647);
                else base->on_mouse_leave(-1, 2147483647);
                break;
            case 1:
                if (adapter) design_win_on_mouse_leave_redirect(design, nullptr, -1, 2147483647);
                else design->on_mouse_leave(-1, 2147483647);
                break;
            case 2:
                if (adapter) social_win_on_iface_mouse_move_redirect(social, nullptr, -1, 2147483647);
                else social->on_iface_mouse_move(-1, 2147483647);
                break;
            default:
                if (adapter) social_win_on_iface_mouse_leave_redirect(social, nullptr, -1, 2147483647);
                else social->on_iface_mouse_leave(-1, 2147483647);
                break;
            }

            // TWO dismissals, because the probe re-armed the bubble after the
            // first. A body with one call reports one.
            expect(teardown_probe.update_calls == 2);
            expect(teardown_probe.flip_calls == 2);
            expect(teardown_probe.update_rect == &rect);
            expect(teardown_probe.flip_rect == &rect);
            expect(teardown_probe.update_window == nullptr);
            // The flags are always clear when the refresh observes them.
            expect(teardown_probe.active_during_update == 0);
            expect(teardown_probe.companion_during_update == 0);
            // No object is touched - not the one called, not the others.
            expect_storage_bytes(base_storage, base_expected, sizeof(base_storage));
            expect_storage_bytes(design_storage, design_expected, sizeof(design_storage));
            expect_storage_bytes(social_storage, social_expected, sizeof(social_storage));
        }
    }

    // With no bubble pending, none of the four does anything at all.
    for (int which = 0; which < 4; ++which) {
        active = 0;
        companion = 0x5A5A5A5A;
        teardown_probe = TeardownProbe{};
        auto *base = reinterpret_cast<BaseWin *>(base_storage + 16);
        auto *design = reinterpret_cast<DesignWin *>(design_storage + 16);
        auto *social = reinterpret_cast<SocialWin *>(social_storage + 16);
        if (which == 0) base->on_mouse_leave(0, 0);
        else if (which == 1) design->on_mouse_leave(0, 0);
        else if (which == 2) social->on_iface_mouse_move(0, 0);
        else social->on_iface_mouse_leave(0, 0);
        expect(teardown_probe.update_calls == 0);
        expect(teardown_probe.flip_calls == 0);
        expect(companion == 0x5A5A5A5A);
    }

    g_rearm_active = nullptr;
    WinBubbleActive = saved_active;
    WinBubbleCompanion = saved_companion;
    WinBubbleRect = saved_rect;
    WinUpdateScreenOriginal = saved_update;
    WinFlipOriginal = saved_flip;
}

void test_win_clear_bubble_text() {
    int active = 0;
    int companion = 0;
    RECT rect = {1, 2, 3, 4};
    int *const saved_active = WinBubbleActive;
    int *const saved_companion = WinBubbleCompanion;
    RECT *const saved_rect = WinBubbleRect;
    func_win_update_screen *const saved_update = WinUpdateScreenOriginal;
    func_win_flip *const saved_flip = WinFlipOriginal;
    WinBubbleActive = &active;
    WinBubbleCompanion = &companion;
    WinBubbleRect = &rect;
    WinUpdateScreenOriginal = &teardown_probe_update_screen;
    WinFlipOriginal = &teardown_probe_flip;

    // No bubble pending: nothing is cleared and neither refresh runs, even
    // with a stale companion value sitting there.
    active = 0;
    companion = 0x5A5A5A5A;
    teardown_probe = TeardownProbe{};
    Win::clear_bubble_text();
    expect(active == 0);
    expect(companion == 0x5A5A5A5A);
    expect(teardown_probe.update_calls == 0);
    expect(teardown_probe.flip_calls == 0);

    // A pending bubble clears both slots, then refreshes and flips the same
    // rectangle. The flags must already be clear when the refresh observes
    // them, which is why the probe samples them.
    for (int adapter = 0; adapter < 2; ++adapter) {
        active = 1;
        companion = 0x5A5A5A5A;
        teardown_probe = TeardownProbe{};
        if (adapter) {
            win_clear_bubble_text_redirect();
        } else {
            Win::clear_bubble_text();
        }
        expect(active == 0);
        expect(companion == 0);
        expect(teardown_probe.update_calls == 1);
        expect(teardown_probe.update_rect == &rect);
        expect(teardown_probe.update_window == nullptr);
        expect(teardown_probe.active_during_update == 0);
        expect(teardown_probe.companion_during_update == 0);
        expect(teardown_probe.flip_calls == 1);
        expect(teardown_probe.flip_rect == &rect);
        // The refresh must precede the flip, not merely both happen.
        expect(teardown_probe.sequence == 0x12);
    }

    // Any nonzero pending value counts, not just one.
    active = -1;
    teardown_probe = TeardownProbe{};
    Win::clear_bubble_text();
    expect(active == 0);
    expect(teardown_probe.update_calls == 1);

    WinFlipOriginal = saved_flip;
    WinUpdateScreenOriginal = saved_update;
    WinBubbleRect = saved_rect;
    WinBubbleCompanion = saved_companion;
    WinBubbleActive = saved_active;
}

// Self-registration.  Each case carries its baseline position in the original
// main() call list, so the run order does not depend on link or static-init
// order.
namespace {
LEAF_CASE(103, test_buffer_hdc_protocol);
LEAF_CASE(104, test_buffer_sync_to_palette);
LEAF_CASE(105, test_buffer_text_height);
LEAF_CASE(106, test_win_is_dialog_focus);
LEAF_CASE(107, test_buffer_set_clip);
LEAF_CASE(108, test_base_button_text_colors);
LEAF_CASE(109, test_default_font_setters);
LEAF_CASE(110, test_win_shared_hdc);
LEAF_CASE(111, test_sprite_draw_origin);
LEAF_CASE(112, test_win_set_cursor);
LEAF_CASE(113, test_buffer_text_width);
LEAF_CASE(114, test_alpha_net_close);
LEAF_CASE(56, test_bubble_dismiss_handlers);
LEAF_CASE(115, test_win_clear_bubble_text);
}  // namespace
