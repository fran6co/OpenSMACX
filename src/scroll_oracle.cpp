#include "stdafx.h"
#include "scroll_oracle.h"

#include "scroll.h"

namespace {

constexpr char ResultEnvironment[] = "OPENSMACX_SCROLL_ORACLE_RESULT";
constexpr size_t CanarySize = 16;
constexpr size_t VtableEntries = 0xFC / sizeof(uintptr_t);
constexpr size_t MaximumCalls = 2;
constexpr size_t MaximumSnapshots = 8;

struct ScrollFixture {
    alignas(Scroll) uint8_t storage[sizeof(Scroll) + CanarySize * 2];

    Scroll *object() {
        return reinterpret_cast<Scroll *>(storage + CanarySize);
    }
};

struct RedrawTrace {
    uint32_t calls;
    uint32_t self_offsets[MaximumCalls];
    uint32_t snapshots[MaximumCalls][MaximumSnapshots];
    uint32_t current_windows[MaximumCalls];
};

struct RedrawContext {
    uint8_t *base;
    size_t offsets[MaximumSnapshots];
    size_t offset_count;
    RedrawTrace trace;
};

RedrawContext Context = {};

uint32_t __fastcall redraw_probe(void *self, void *);

int int_from_bits(uint32_t bits) {
    int value;
    static_assert(sizeof(value) == sizeof(bits),
                  "Scroll oracle requires 32-bit int");
    memcpy(&value, &bits, sizeof(value));
    return value;
}

template <typename T>
void write_object(ScrollFixture &fixture, size_t offset, const T &value) {
    memcpy(fixture.storage + CanarySize + offset, &value, sizeof(value));
}

void initialize_pair(ScrollFixture &legacy, ScrollFixture &source,
                     uintptr_t vtable[VtableEntries]) {
    memset(legacy.storage, 0xA5, sizeof(legacy.storage));
    for (size_t offset = 0; offset < sizeof(Scroll); ++offset) {
        legacy.storage[CanarySize + offset] =
            static_cast<uint8_t>(0x35U + offset * 17U);
    }
    memset(vtable, 0, sizeof(uintptr_t) * VtableEntries);
    vtable[0xF8 / sizeof(uintptr_t)] =
        reinterpret_cast<uintptr_t>(&redraw_probe);
    uintptr_t *vtable_pointer = vtable;
    write_object(legacy, 0, vtable_pointer);
    write_object(legacy, 0xAAC, vtable_pointer);
    write_object(legacy, 0x15F8, vtable_pointer);
    memcpy(source.storage, legacy.storage, sizeof(source.storage));
}

void begin_trace(Scroll *scroll, const size_t *offsets, size_t offset_count) {
    memset(&Context, 0, sizeof(Context));
    Context.base = reinterpret_cast<uint8_t *>(scroll);
    Context.offset_count = offset_count;
    for (size_t index = 0; index < offset_count; ++index) {
        Context.offsets[index] = offsets[index];
    }
}

uint32_t __fastcall redraw_probe(void *self, void *) {
    const uint32_t call = Context.trace.calls++;
    if (call >= MaximumCalls) {
        return 0xDEAD0000U | call;
    }
    Context.trace.self_offsets[call] = static_cast<uint32_t>(
        reinterpret_cast<uint8_t *>(self) - Context.base);
    for (size_t index = 0; index < Context.offset_count; ++index) {
        memcpy(&Context.trace.snapshots[call][index],
               Context.base + Context.offsets[index], sizeof(uint32_t));
    }
    Context.trace.current_windows[call] =
        reinterpret_cast<uintptr_t>(*ScrollCurrentWin);
    return 0x13579BDFU ^ (call * 0x3174A10DU);
}

bool equivalent(const ScrollFixture &legacy, const ScrollFixture &source,
                uint32_t legacy_result, uint32_t source_result,
                const RedrawTrace &legacy_trace,
                const RedrawTrace &source_trace) {
    return legacy_result == source_result
        && memcmp(legacy.storage, source.storage, sizeof(legacy.storage)) == 0
        && memcmp(&legacy_trace, &source_trace, sizeof(legacy_trace)) == 0;
}

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
#endif
// GCC emits the correct ECX/stack ABI for these raw thiscall pointers but
// warns because they are not C++ member-pointer types.
typedef uint32_t (__thiscall *OriginalNoArg)(Scroll *);
typedef uint32_t (__thiscall *OriginalOneArg)(Scroll *, int);
typedef uint32_t (__thiscall *OriginalTwoArgs)(Scroll *, int, int);
typedef uint32_t (__thiscall *OriginalRectInit)(
    Scroll *, RECT *, Win *, int, int);
typedef uint32_t (__thiscall *OriginalAxisInit)(
    Scroll *, int, int, int, Win *, int);
typedef uint32_t (__thiscall *OriginalSprites)(
    Scroll *, Sprite *, Sprite *, Sprite *);
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

bool verify_init_wrappers() {
    auto original_rect = reinterpret_cast<OriginalRectInit>(0x00605840U);
    const uint32_t sentinel = 0x2468ACE0U;
    const int saved_nonclient = *ScrollNonClientInit;
    bool passed = true;

    for (int null_rect = 0; null_rect < 2 && passed; ++null_rect) {
        ScrollFixture legacy;
        ScrollFixture source;
        uintptr_t vtable[VtableEntries];
        initialize_pair(legacy, source, vtable);
        RECT legacy_rect = {10, 20, 30, 40};
        RECT source_rect = legacy_rect;
        RECT *const legacy_rect_arg = null_rect ? nullptr : &legacy_rect;
        RECT *const source_rect_arg = null_rect ? nullptr : &source_rect;
        Win *const parent = null_rect
            ? reinterpret_cast<Win *>(0x45454545U) : nullptr;

        *ScrollNonClientInit = int_from_bits(sentinel);
        const uint32_t legacy_result = original_rect(
            legacy.object(), legacy_rect_arg, parent,
            int_from_bits(0x13579BDFU), int_from_bits(0xFEDCBA98U));
        const int legacy_nonclient = *ScrollNonClientInit;
        *ScrollNonClientInit = int_from_bits(sentinel);
        const uint32_t source_result = source.object()->init(
            source_rect_arg, parent,
            int_from_bits(0x13579BDFU), int_from_bits(0xFEDCBA98U));
        const int source_nonclient = *ScrollNonClientInit;
        passed = legacy_result == source_result
            && legacy_nonclient == source_nonclient
            && memcmp(legacy.storage, source.storage, sizeof(legacy.storage)) == 0
            && memcmp(&legacy_rect, &source_rect, sizeof(legacy_rect)) == 0;
    }

    const uintptr_t addresses[] = {
        0x00605890U, 0x006058D0U, 0x00605910U, 0x00605960U,
    };
    struct InvalidAxisCase {
        uint32_t length;
        Win *parent;
    };
    const InvalidAxisCase cases[] = {
        {0U, reinterpret_cast<Win *>(0x45454545U)},
        {0xFFFFFFFFU, nullptr},
    };
    for (size_t kind = 0; kind < ARRAYSIZE(addresses) && passed; ++kind) {
        auto original = reinterpret_cast<OriginalAxisInit>(addresses[kind]);
        for (const InvalidAxisCase &test : cases) {
            ScrollFixture legacy;
            ScrollFixture source;
            uintptr_t vtable[VtableEntries];
            initialize_pair(legacy, source, vtable);
            *ScrollNonClientInit = int_from_bits(sentinel);
            const uint32_t legacy_result = original(
                legacy.object(), int_from_bits(0x80000000U),
                int_from_bits(0x7FFFFFFFU), int_from_bits(test.length),
                test.parent, int_from_bits(0x13579BDFU));
            const int legacy_nonclient = *ScrollNonClientInit;

            *ScrollNonClientInit = int_from_bits(sentinel);
            uint32_t source_result;
            switch (kind) {
              case 0:
                source_result = source.object()->init_vert(
                    int_from_bits(0x80000000U), int_from_bits(0x7FFFFFFFU),
                    int_from_bits(test.length),
                    test.parent, int_from_bits(0x13579BDFU));
                break;
              case 1:
                source_result = source.object()->init_horz(
                    int_from_bits(0x80000000U), int_from_bits(0x7FFFFFFFU),
                    int_from_bits(test.length),
                    test.parent, int_from_bits(0x13579BDFU));
                break;
              case 2:
                source_result = source.object()->init_vert_nc(
                    int_from_bits(0x80000000U), int_from_bits(0x7FFFFFFFU),
                    int_from_bits(test.length),
                    test.parent, int_from_bits(0x13579BDFU));
                break;
              default:
                source_result = source.object()->init_horz_nc(
                    int_from_bits(0x80000000U), int_from_bits(0x7FFFFFFFU),
                    int_from_bits(test.length),
                    test.parent, int_from_bits(0x13579BDFU));
                break;
            }
            const int source_nonclient = *ScrollNonClientInit;
            if (legacy_result != source_result
                    || legacy_nonclient != source_nonclient
                    || memcmp(legacy.storage, source.storage,
                              sizeof(legacy.storage)) != 0) {
                passed = false;
                break;
            }
        }
    }
    *ScrollNonClientInit = saved_nonclient;
    return passed;
}

bool verify_range() {
    const uint32_t cases[][2] = {
        {10U, 20U},
        {20U, 10U},
        {5U, 5U},
        {0xFFFFFFFFU, 0U},
        {0x80000000U, 0x7FFFFFFFU},
        {0x7FFFFFFFU, 0x80000000U},
    };
    const size_t snapshots[] = {0xA20, 0xA24, 0xA28, 0xA2C};
    auto original = reinterpret_cast<OriginalTwoArgs>(0x006059B0U);
    for (const auto &test : cases) {
        ScrollFixture legacy;
        ScrollFixture source;
        uintptr_t vtable[VtableEntries];
        initialize_pair(legacy, source, vtable);
        begin_trace(legacy.object(), snapshots, ARRAYSIZE(snapshots));
        const uint32_t legacy_result = original(
            legacy.object(), int_from_bits(test[0]), int_from_bits(test[1]));
        const RedrawTrace legacy_trace = Context.trace;
        begin_trace(source.object(), snapshots, ARRAYSIZE(snapshots));
        const uint32_t source_result = source.object()->set_range(
            int_from_bits(test[0]), int_from_bits(test[1]));
        if (!equivalent(legacy, source, legacy_result, source_result,
                        legacy_trace, Context.trace)) {
            return false;
        }
    }
    return true;
}

bool verify_styles() {
    const uintptr_t addresses[] = {
        0x00605A10U, 0x00605A50U, 0x00605A90U, 0x00605AD0U,
    };
    const size_t snapshots[][3] = {
        {0xA5C, 0x1530, 0x207C},
        {0xA68, 0x153C, 0x2088},
        {0xA6C, 0x1534, 0x2080},
        {0xA70, 0x1538, 0x2084},
    };
    const uint32_t values[] = {0U, 0x80000000U, 0xFFFFFFFFU};
    for (size_t style = 0; style < ARRAYSIZE(addresses); ++style) {
        auto original = reinterpret_cast<OriginalOneArg>(addresses[style]);
        for (uint32_t value : values) {
            ScrollFixture legacy;
            ScrollFixture source;
            uintptr_t vtable[VtableEntries];
            initialize_pair(legacy, source, vtable);
            begin_trace(legacy.object(), snapshots[style], 3);
            const uint32_t legacy_result = original(
                legacy.object(), int_from_bits(value));
            const RedrawTrace legacy_trace = Context.trace;
            begin_trace(source.object(), snapshots[style], 3);
            uint32_t source_result;
            switch (style) {
              case 0:
                source_result = source.object()->set_button_color(
                    int_from_bits(value));
                break;
              case 1:
                source_result = source.object()->set_bevel_thickness(
                    int_from_bits(value));
                break;
              case 2:
                source_result = source.object()->set_bevel_upper(
                    int_from_bits(value));
                break;
              default:
                source_result = source.object()->set_bevel_lower(
                    int_from_bits(value));
                break;
            }
            if (!equivalent(legacy, source, legacy_result, source_result,
                            legacy_trace, Context.trace)) {
                return false;
            }
        }
    }
    return true;
}

bool verify_thumb_resetters() {
    const uint32_t colors[] = {0xFFFFFFFFU, 0U, 0x80000000U};
    const uint32_t thicknesses[] = {0U, 1U, 0x80000000U, 0xFFFFFFFFU};
    auto original_bar = reinterpret_cast<OriginalOneArg>(0x00605B80U);
    auto original_thumb = reinterpret_cast<OriginalNoArg>(0x00606EA0U);
    for (uint32_t color : colors) {
        for (uint32_t thickness : thicknesses) {
            for (int set_bar = 0; set_bar < 2; ++set_bar) {
                ScrollFixture legacy;
                ScrollFixture source;
                uintptr_t vtable[VtableEntries];
                initialize_pair(legacy, source, vtable);
                write_object(legacy, 0xA1C, color);
                write_object(legacy, 0xA60, thickness);
                memcpy(source.storage, legacy.storage, sizeof(source.storage));
                begin_trace(legacy.object(), nullptr, 0);
                const uint32_t legacy_result = set_bar
                    ? original_bar(legacy.object(), int_from_bits(thickness))
                    : original_thumb(legacy.object());
                const RedrawTrace legacy_trace = Context.trace;
                begin_trace(source.object(), nullptr, 0);
                const uint32_t source_result = set_bar
                    ? source.object()->set_bar_thickness(int_from_bits(thickness))
                    : source.object()->set_thumb_rect();
                if (!equivalent(legacy, source, legacy_result, source_result,
                                legacy_trace, Context.trace)) {
                    return false;
                }
            }
        }
    }
    return true;
}

bool verify_vertical_sprites() {
    struct Geometry {
        uint32_t width;
        uint32_t stored_height;
    };
    const Geometry geometries[] = {
        {20U, 0xFFFFFF9CU},
        {100U, 0xFFFFFFECU},
        {20U, 0xFFFFFFECU},
        {0x80000000U, 0x80000000U},
    };
    const uintptr_t addresses[] = {0x00605C80U, 0x00605CD0U};
    Sprite *const sprites[] = {
        reinterpret_cast<Sprite *>(0x10101010U),
        reinterpret_cast<Sprite *>(0x20202020U),
        reinterpret_cast<Sprite *>(0x10101010U),
    };
    for (size_t direction = 0; direction < ARRAYSIZE(addresses); ++direction) {
        auto original = reinterpret_cast<OriginalSprites>(addresses[direction]);
        for (const Geometry &geometry : geometries) {
            ScrollFixture legacy;
            ScrollFixture source;
            uintptr_t vtable[VtableEntries];
            initialize_pair(legacy, source, vtable);
            write_object(legacy, 0x4C4, geometry.width);
            write_object(legacy, 0x4C8, geometry.stored_height);
            memcpy(source.storage, legacy.storage, sizeof(source.storage));
            begin_trace(legacy.object(), nullptr, 0);
            const uint32_t legacy_result = original(
                legacy.object(), sprites[0], sprites[1], sprites[2]);
            const RedrawTrace legacy_trace = Context.trace;
            begin_trace(source.object(), nullptr, 0);
            const uint32_t source_result = direction == 0
                ? reinterpret_cast<uintptr_t>(
                    scroll_set_sprite_up_redirect(
                        source.object(), nullptr, sprites[0], sprites[1], sprites[2]))
                : reinterpret_cast<uintptr_t>(
                    scroll_set_sprite_down_redirect(
                        source.object(), nullptr, sprites[0], sprites[1], sprites[2]));
            if (!equivalent(legacy, source, legacy_result, source_result,
                            legacy_trace, Context.trace)) {
                return false;
            }
        }
    }
    return true;
}

bool verify_position() {
    struct PositionCase {
        uint32_t parent;
        uint32_t minimum;
        uint32_t maximum;
        uint32_t reversed;
        uint32_t input;
    };
    const PositionCase cases[] = {
        {0U, 0U, 100U, 0U, 0x80000000U},
        {0x45454545U, 0U, 100U, 0U, 0xFFFFFFFFU},
        {0x45454545U, 0U, 100U, 0U, 101U},
        {0x45454545U, 0U, 100U, 1U, 25U},
        {0x45454545U, 0x80000000U, 0x7FFFFFFFU, 1U, 0U},
    };
    const size_t snapshots[] = {0xA20, 0xA24, 0xA28, 0xA2C};
    auto original = reinterpret_cast<OriginalOneArg>(0x00605D20U);
    ::Win *const saved_current = *ScrollCurrentWin;
    bool passed = true;
    for (const PositionCase &test : cases) {
        ScrollFixture legacy;
        ScrollFixture source;
        uintptr_t vtable[VtableEntries];
        initialize_pair(legacy, source, vtable);
        write_object(legacy, 0xC4, test.parent);
        write_object(legacy, 0xA20, test.minimum);
        write_object(legacy, 0xA24, test.maximum);
        write_object(legacy, 0xA28, test.reversed);
        memcpy(source.storage, legacy.storage, sizeof(source.storage));

        *ScrollCurrentWin = reinterpret_cast<::Win *>(0x24682468U);
        begin_trace(legacy.object(), snapshots, ARRAYSIZE(snapshots));
        const uint32_t legacy_result = original(
            legacy.object(), int_from_bits(test.input));
        const RedrawTrace legacy_trace = Context.trace;
        ::Win *const legacy_current = *ScrollCurrentWin;

        *ScrollCurrentWin = reinterpret_cast<::Win *>(0x24682468U);
        begin_trace(source.object(), snapshots, ARRAYSIZE(snapshots));
        const uint32_t source_result = source.object()->set_pos(
            int_from_bits(test.input));
        ::Win *const source_current = *ScrollCurrentWin;
        if (legacy_current != source_current
                || !equivalent(legacy, source, legacy_result, source_result,
                               legacy_trace, Context.trace)) {
            passed = false;
            break;
        }
    }
    *ScrollCurrentWin = saved_current;
    return passed;
}

bool write_result(const char *path, const char *result) {
    HANDLE file = CreateFileA(path, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
                              FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE) {
        return false;
    }
    const DWORD size = static_cast<DWORD>(strlen(result));
    DWORD written = 0;
    const BOOL write_ok = WriteFile(file, result, size, &written, nullptr);
    const BOOL flush_ok = FlushFileBuffers(file);
    const BOOL close_ok = CloseHandle(file);
    return write_ok && flush_ok && close_ok && written == size;
}

}  // namespace

bool run_scroll_recovery_oracle() {
    char result_path[MAX_PATH];
    const DWORD length = GetEnvironmentVariableA(
        ResultEnvironment, result_path, ARRAYSIZE(result_path));
    if (length == 0) {
        return true;
    }
    if (length >= ARRAYSIZE(result_path)) {
        return false;
    }
    const bool passed = verify_init_wrappers()
        && verify_range()
        && verify_styles()
        && verify_thumb_resetters()
        && verify_vertical_sprites()
        && verify_position();
    const bool result_written = write_result(
        result_path, passed ? "passed\n" : "failed\n");
    return passed && result_written;
}
