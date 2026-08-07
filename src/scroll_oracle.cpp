#include "stdafx.h"
#include "original_seam.h"
#include "scroll_oracle.h"

#include "runtime_oracle.h"
#include "scroll.h"

// What this oracle PROVES: each function below is executed here in its
// ORIGINAL form, at its canonical address inside the hybrid process, and
// compared against the recovered implementation. These markers are the
// input to `unproven_recovered` in docs/recovery/summary.json; see
// tools/export_proven_functions.py for what does and does not count.
//
// PROVEN-AGAINST-ORIGINAL: 0x00605370  ?close@Scroll@@QAEXXZ
// PROVEN-AGAINST-ORIGINAL: 0x006059B0  ?set_range@Scroll@@QAEXHH@Z
// PROVEN-AGAINST-ORIGINAL: 0x00605A50  ?set_bevel_thickness@Scroll@@QAEXH@Z
// PROVEN-AGAINST-ORIGINAL: 0x00605A90  ?set_bevel_upper@Scroll@@QAEXH@Z
// PROVEN-AGAINST-ORIGINAL: 0x00605AD0  ?set_bevel_lower@Scroll@@QAEXH@Z
// PROVEN-AGAINST-ORIGINAL: 0x00606EA0  ?set_thumb_rect@Scroll@@QAEXXZ


namespace {

constexpr size_t VtableEntries = 0xFC / sizeof(uintptr_t);

typedef runtime_oracle::Fixture<Scroll> ScrollFixture;
typedef runtime_oracle::Trace RedrawTrace;

const size_t ScrollVtableRefOffsets[] = {0, 0xAAC, 0x15F8};
const runtime_oracle::ClassSpec ScrollSpec = {
    sizeof(Scroll), 0xFC, 0xF8,
    ScrollVtableRefOffsets, ARRAYSIZE(ScrollVtableRefOffsets),
};

const size_t ScrollCloseVtableRefOffsets[] = {0};
const runtime_oracle::ClassSpec ScrollCloseSpec = {
    sizeof(Scroll), 0x0C, 0x08,
    ScrollCloseVtableRefOffsets, ARRAYSIZE(ScrollCloseVtableRefOffsets),
};

const size_t ScrollCloseTraceOffsets[] = {
    0xA10, 0xA14, 0xA20, 0xA28, 0xA74, 0xA7C, 0xAA8, 0x2144,
};

int int_from_bits(uint32_t bits) {
    int value;
    static_assert(sizeof(value) == sizeof(bits),
                  "Scroll oracle requires 32-bit int");
    memcpy(&value, &bits, sizeof(value));
    return value;
}

template <typename T>
void write_object(ScrollFixture &fixture, size_t offset, const T &value) {
    memcpy(fixture.storage + runtime_oracle::CanarySize + offset,
           &value, sizeof(value));
}

uint32_t read_object(const ScrollFixture &fixture, size_t offset) {
    uint32_t value;
    memcpy(&value,
           fixture.storage + runtime_oracle::CanarySize + offset,
           sizeof(value));
    return value;
}

void initialize_pair(ScrollFixture &legacy, ScrollFixture &source,
                     uintptr_t vtable[VtableEntries]) {
    runtime_oracle::set_watched_global(
        reinterpret_cast<void **>(ScrollCurrentWin));
    runtime_oracle::initialize_pair(
        legacy.storage, source.storage, ScrollSpec, vtable);
}

void begin_trace(Scroll *scroll, const size_t *offsets, size_t offset_count) {
    runtime_oracle::begin_trace(scroll, offsets, offset_count);
}

bool equivalent(const ScrollFixture &legacy, const ScrollFixture &source,
                uint32_t legacy_result, uint32_t source_result,
                const RedrawTrace &legacy_trace,
                const RedrawTrace &source_trace) {
    return runtime_oracle::equivalent(
        legacy.storage, source.storage, sizeof(legacy.storage),
        legacy_result, source_result, legacy_trace, source_trace);
}

// GCC emits the correct ECX/stack ABI for these raw thiscall pointers but
// warns because they are not C++ member-pointer types.
typedef uint32_t (OriginalObject::*OriginalNoArg)();
typedef uint32_t (OriginalObject::*OriginalOneArg)(int);
typedef uint32_t (OriginalObject::*OriginalTwoArgs)(int, int);
typedef uint32_t (OriginalObject::*OriginalRectInit)(RECT *, Win *, int, int);
typedef uint32_t (OriginalObject::*OriginalAxisInit)(int, int, int, Win *, int);
typedef uint32_t (OriginalObject::*OriginalSprites)(Sprite *, Sprite *, Sprite *);

struct ScrollCloseButtonTrace {
    uint32_t calls;
    uint32_t self_offsets[2];
    uint32_t field_a10[2];
    uint32_t field_a14[2];
    uint32_t field_2144[2];
};

Scroll *ScrollCloseTraceBase = nullptr;
ScrollCloseButtonTrace ScrollCloseButtons = {0};

uint32_t __fastcall trace_scroll_button_close(void *self) {
    const uint32_t call = ScrollCloseButtons.calls++;
    if (call >= ARRAYSIZE(ScrollCloseButtons.self_offsets)) {
        return 0xBAD0C105U;
    }
    const uintptr_t base = reinterpret_cast<uintptr_t>(ScrollCloseTraceBase);
    const uintptr_t target = reinterpret_cast<uintptr_t>(self);
    ScrollCloseButtons.self_offsets[call] = target - base;
    const uint8_t *const bytes = reinterpret_cast<const uint8_t *>(
        ScrollCloseTraceBase);
    memcpy(&ScrollCloseButtons.field_a10[call], bytes + 0xA10,
           sizeof(uint32_t));
    memcpy(&ScrollCloseButtons.field_a14[call], bytes + 0xA14,
           sizeof(uint32_t));
    memcpy(&ScrollCloseButtons.field_2144[call], bytes + 0x2144,
           sizeof(uint32_t));
    return 0xC1050000U ^ ScrollCloseButtons.self_offsets[call];
}

void prepare_close_fixture(ScrollFixture &fixture, uintptr_t *win_vtable,
                           uintptr_t *left_vtable,
                           uintptr_t *right_vtable) {
    uint8_t *const object =
        fixture.storage + runtime_oracle::CanarySize;
    memset(object, 0, sizeof(Scroll));
    memcpy(object, &win_vtable, sizeof(win_vtable));
    memcpy(object + 0xAAC, &left_vtable, sizeof(left_vtable));
    memcpy(object + 0x15F8, &right_vtable, sizeof(right_vtable));

    // Keep Win and Buffer resource paths empty, while poisoning every field
    // owned by this close and every GraphicWin reset field. The two untouched
    // Scroll holes (A18 and A60) remain poisoned and are covered by the final
    // complete-object comparison.
    for (size_t offset = 0xA14; offset <= 0xAA8; offset += 4) {
        const uint32_t value = 0xA5000000U ^ (offset * 0x1021U);
        write_object(fixture, offset, value);
    }
    write_object(fixture, 0x2144, 0x13579BDFU);
    write_object(fixture, 0x2148, 0x2468ACE0U);
    write_object(fixture, 0x134, 0x31415926U);
    write_object(fixture, 0x138, 0x27182818U);
    for (size_t offset = 0x9CC; offset <= 0xA10; offset += 4) {
        const uint32_t value = 0x5A000000U ^ (offset * 0x2101U);
        write_object(fixture, offset, value);
    }
    write_object(fixture, 0xA08, 0U);
}

bool verify_close() {
    if (ScrollCloseStaticDefaults != reinterpret_cast<uint32_t *>(0x00697020U)
            || ScrollCloseDynamicDefaults
                != reinterpret_cast<uint32_t *>(0x009B8DE0U)) {
        return false;
    }

    uint32_t saved_fixed[11];
    uint32_t saved_dynamic[17];
    volatile uint32_t *const fixed = ScrollCloseStaticDefaults;
    volatile uint32_t *const dynamic = ScrollCloseDynamicDefaults;
    for (size_t index = 0; index < ARRAYSIZE(saved_fixed); ++index) {
        saved_fixed[index] = fixed[index];
        fixed[index] = 0x51000000U
            + static_cast<uint32_t>(index) * 0x010203U;
    }
    for (size_t index = 0; index < ARRAYSIZE(saved_dynamic); ++index) {
        saved_dynamic[index] = dynamic[index];
        dynamic[index] = 0xA1000000U
            + static_cast<uint32_t>(index) * 0x010101U;
    }
    dynamic[14] = 0xDEADC0DEU;

    uintptr_t win_vtable[3] = {0};
    uintptr_t left_vtable[0x16C / sizeof(uintptr_t)] = {0};
    uintptr_t right_vtable[0x16C / sizeof(uintptr_t)] = {0};
    win_vtable[2] = reinterpret_cast<uintptr_t>(&runtime_oracle::probe);
    left_vtable[0x168 / sizeof(uintptr_t)] =
        reinterpret_cast<uintptr_t>(&trace_scroll_button_close);
    right_vtable[0x168 / sizeof(uintptr_t)] =
        reinterpret_cast<uintptr_t>(&trace_scroll_button_close);

    ScrollFixture legacy;
    ScrollFixture source;
    runtime_oracle::initialize_pair(
        legacy.storage, source.storage, ScrollCloseSpec, win_vtable);
    prepare_close_fixture(legacy, win_vtable, left_vtable, right_vtable);
    prepare_close_fixture(source, win_vtable, left_vtable, right_vtable);
    OriginalNoArg original = reinterpret_cast<OriginalNoArg>(0x00605370U);

    ScrollCloseTraceBase = legacy.object();
    ScrollCloseButtons = ScrollCloseButtonTrace{};
    runtime_oracle::begin_trace(
        legacy.object(), ScrollCloseTraceOffsets,
        ARRAYSIZE(ScrollCloseTraceOffsets));
    const uint32_t legacy_result = original(legacy.object());
    const runtime_oracle::Trace legacy_trace = runtime_oracle::current_trace();
    const ScrollCloseButtonTrace legacy_buttons = ScrollCloseButtons;

    ScrollCloseTraceBase = source.object();
    ScrollCloseButtons = ScrollCloseButtonTrace{};
    runtime_oracle::begin_trace(
        source.object(), ScrollCloseTraceOffsets,
        ARRAYSIZE(ScrollCloseTraceOffsets));
    const uint32_t source_result = source.object()->close();
    const runtime_oracle::Trace source_trace = runtime_oracle::current_trace();
    const ScrollCloseButtonTrace source_buttons = ScrollCloseButtons;

    const bool self_pointers_match =
        read_object(legacy, 0xA8) == reinterpret_cast<uintptr_t>(legacy.object())
        && read_object(source, 0xA8)
            == reinterpret_cast<uintptr_t>(source.object());
    write_object(legacy, 0xA8, 0U);
    write_object(source, 0xA8, 0U);
    const bool passed = self_pointers_match
        && legacy_buttons.calls == 2U
        && source_buttons.calls == 2U
        && legacy_buttons.self_offsets[0] == 0xAACU
        && legacy_buttons.self_offsets[1] == 0x15F8U
        && memcmp(&legacy_buttons, &source_buttons,
                  sizeof(legacy_buttons)) == 0
        && runtime_oracle::equivalent(
            legacy.storage, source.storage, sizeof(legacy.storage),
            legacy_result, source_result, legacy_trace, source_trace);

    for (size_t index = 0; index < ARRAYSIZE(saved_fixed); ++index) {
        fixed[index] = saved_fixed[index];
    }
    for (size_t index = 0; index < ARRAYSIZE(saved_dynamic); ++index) {
        dynamic[index] = saved_dynamic[index];
    }
    ScrollCloseTraceBase = nullptr;
    return passed;
}

bool verify_init_wrappers() {
    OriginalRectInit original_rect = reinterpret_cast<OriginalRectInit>(0x00605840U);
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
        OriginalAxisInit original = reinterpret_cast<OriginalAxisInit>(addresses[kind]);
        for (size_t test_index = 0;
             test_index < sizeof(cases) / sizeof(cases[0]); ++test_index) {
            const InvalidAxisCase &test = cases[test_index];
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
    OriginalTwoArgs original = reinterpret_cast<OriginalTwoArgs>(0x006059B0U);
    for (size_t test_index = 0;
         test_index < sizeof(cases) / sizeof(cases[0]); ++test_index) {
        const uint32_t (&test)[2] = cases[test_index];
        ScrollFixture legacy;
        ScrollFixture source;
        uintptr_t vtable[VtableEntries];
        initialize_pair(legacy, source, vtable);
        begin_trace(legacy.object(), snapshots, ARRAYSIZE(snapshots));
        const uint32_t legacy_result = original(
            legacy.object(), int_from_bits(test[0]), int_from_bits(test[1]));
        const RedrawTrace legacy_trace = runtime_oracle::current_trace();
        begin_trace(source.object(), snapshots, ARRAYSIZE(snapshots));
        const uint32_t source_result = source.object()->set_range(
            int_from_bits(test[0]), int_from_bits(test[1]));
        if (!equivalent(legacy, source, legacy_result, source_result,
                        legacy_trace, runtime_oracle::current_trace())) {
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
        OriginalOneArg original = reinterpret_cast<OriginalOneArg>(addresses[style]);
        for (size_t value_index = 0;
             value_index < sizeof(values) / sizeof(values[0]); ++value_index) {
            uint32_t value = values[value_index];
            ScrollFixture legacy;
            ScrollFixture source;
            uintptr_t vtable[VtableEntries];
            initialize_pair(legacy, source, vtable);
            begin_trace(legacy.object(), snapshots[style], 3);
            const uint32_t legacy_result = original(
                legacy.object(), int_from_bits(value));
            const RedrawTrace legacy_trace = runtime_oracle::current_trace();
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
                            legacy_trace, runtime_oracle::current_trace())) {
                return false;
            }
        }
    }
    return true;
}

bool verify_thumb_resetters() {
    const uint32_t colors[] = {0xFFFFFFFFU, 0U, 0x80000000U};
    const uint32_t thicknesses[] = {0U, 1U, 0x80000000U, 0xFFFFFFFFU};
    OriginalOneArg original_bar = reinterpret_cast<OriginalOneArg>(0x00605B80U);
    OriginalNoArg original_thumb = reinterpret_cast<OriginalNoArg>(0x00606EA0U);
    for (size_t color_index = 0;
         color_index < sizeof(colors) / sizeof(colors[0]); ++color_index) {
        uint32_t color = colors[color_index];
        for (size_t thickness_index = 0;
             thickness_index < sizeof(thicknesses) / sizeof(thicknesses[0]);
             ++thickness_index) {
            uint32_t thickness = thicknesses[thickness_index];
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
                const RedrawTrace legacy_trace = runtime_oracle::current_trace();
                begin_trace(source.object(), nullptr, 0);
                const uint32_t source_result = set_bar
                    ? source.object()->set_bar_thickness(int_from_bits(thickness))
                    : source.object()->set_thumb_rect();
                if (!equivalent(legacy, source, legacy_result, source_result,
                                legacy_trace, runtime_oracle::current_trace())) {
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
        OriginalSprites original = reinterpret_cast<OriginalSprites>(addresses[direction]);
        for (size_t geometry_index = 0;
             geometry_index < sizeof(geometries) / sizeof(geometries[0]);
             ++geometry_index) {
            const Geometry &geometry = geometries[geometry_index];
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
            const RedrawTrace legacy_trace = runtime_oracle::current_trace();
            begin_trace(source.object(), nullptr, 0);
            const uint32_t source_result = direction == 0
                ? reinterpret_cast<uintptr_t>(
                    scroll_set_sprite_up_redirect(
                        source.object(), nullptr, sprites[0], sprites[1], sprites[2]))
                : reinterpret_cast<uintptr_t>(
                    scroll_set_sprite_down_redirect(
                        source.object(), nullptr, sprites[0], sprites[1], sprites[2]));
            if (!equivalent(legacy, source, legacy_result, source_result,
                            legacy_trace, runtime_oracle::current_trace())) {
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
        // An inverted range at position == minimum is the only shape that
        // observes the strictness of `position < minimum`: with min <= max
        // both branches store the same value there, but with min > max the
        // else-branch clamps to the maximum while the mutated if-branch
        // stores the minimum.
        {0x45454545U, 100U, 0U, 0U, 100U},
    };
    const size_t snapshots[] = {0xA20, 0xA24, 0xA28, 0xA2C};
    OriginalOneArg original = reinterpret_cast<OriginalOneArg>(0x00605D20U);
    ::Win *const saved_current = *ScrollCurrentWin;
    bool passed = true;
    for (size_t test_index = 0;
         test_index < sizeof(cases) / sizeof(cases[0]); ++test_index) {
        const PositionCase &test = cases[test_index];
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
        const RedrawTrace legacy_trace = runtime_oracle::current_trace();
        ::Win *const legacy_current = *ScrollCurrentWin;

        *ScrollCurrentWin = reinterpret_cast<::Win *>(0x24682468U);
        begin_trace(source.object(), snapshots, ARRAYSIZE(snapshots));
        const uint32_t source_result = source.object()->set_pos(
            int_from_bits(test.input));
        ::Win *const source_current = *ScrollCurrentWin;
        if (legacy_current != source_current
                || !equivalent(legacy, source, legacy_result, source_result,
                               legacy_trace, runtime_oracle::current_trace())) {
            passed = false;
            break;
        }
    }
    *ScrollCurrentWin = saved_current;
    return passed;
}

}  // namespace

bool run_scroll_oracle_suite() {
    return verify_close()
        && verify_init_wrappers()
        && verify_range()
        && verify_styles()
        && verify_thumb_resetters()
        && verify_vertical_sprites()
        && verify_position();
}
