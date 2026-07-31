// Recovery leaf tests: leaf_recoveries.
//
// Carved verbatim out of tests/recovery_leaf_tests.cpp.  Add a case by
// appending a `void test_*()` here plus one LEAF_CASE line at the bottom, and
// bumping this family's count in tests/leaf/leaf_case_manifest.h.
#include "leaf_shared.h"

#define LEAF_FAMILY "leaf_recoveries"

namespace {

struct ScrollInitProbeState {
    int calls;
    Scroll *self;
    uint32_t x;
    uint32_t y;
    uint32_t width;
    uint32_t height;
    Win *parent;
    uint32_t setting;
    uint32_t options;
    int nonclient;
    uint32_t result;
    bool clear_nonclient;
};

ScrollInitProbeState scroll_init_probe_state = {};

}  // namespace

ScrollCloseProbeState scroll_close_probe_state = {};

namespace {

uint32_t scroll_close_read(size_t offset) {
    uint32_t value;
    std::memcpy(&value, scroll_close_probe_state.base + offset, sizeof(value));
    return value;
}

void expect_scroll_close_defaults() {
    const uint32_t *const fixed = scroll_close_probe_state.static_defaults;
    const uint32_t *const dynamic = scroll_close_probe_state.dynamic_defaults;
    expect(scroll_close_read(0xA14) == dynamic[0]);
    expect(scroll_close_read(0xA1C) == fixed[3]);
    expect(scroll_close_read(0xA20) == dynamic[1]);
    expect(scroll_close_read(0xA24) == fixed[4]);
    expect(scroll_close_read(0xA28) == 0U);
    expect(scroll_close_read(0xA2C) == dynamic[1]);
    expect(scroll_close_read(0xA30) == fixed[2]);
    expect(scroll_close_read(0xA34) == fixed[1]);
    expect(scroll_close_read(0xA38) == 0U);
    expect(scroll_close_read(0xA3C) == 0xFFFFFFFFU);
    expect(scroll_close_read(0xA40) == fixed[0]);
    expect(scroll_close_read(0xA44) == 0U);
    expect(scroll_close_read(0xA48) == fixed[5]);
    for (size_t offset = 0xA4C; offset <= 0xA58; offset += 4) {
        expect(scroll_close_read(offset) == 0U);
    }
    expect(scroll_close_read(0xA5C) == fixed[6]);
    expect(scroll_close_read(0xA64) == fixed[7]);
    expect(scroll_close_read(0xA68) == fixed[8]);
    expect(scroll_close_read(0xA6C) == fixed[9]);
    expect(scroll_close_read(0xA70) == fixed[10]);
    for (size_t index = 0; index < 3; ++index) {
        expect(scroll_close_read(0xA7C + index * 4) == dynamic[2 + index]);
        expect(scroll_close_read(0xA88 + index * 4) == dynamic[5 + index]);
        expect(scroll_close_read(0xA94 + index * 4) == dynamic[8 + index]);
        expect(scroll_close_read(0xAA0 + index * 4) == dynamic[11 + index]);
    }
    expect(scroll_close_read(0xA74) == dynamic[15]);
    expect(scroll_close_read(0xA78) == dynamic[16]);
    expect(scroll_close_read(0x2144) == 0U);
    expect(scroll_close_read(0x2148) == 0U);
}

}  // namespace

uint32_t __fastcall scroll_close_left_probe(void *self, void *) {
    expect(scroll_close_probe_state.order == 0);
    expect(self == scroll_close_probe_state.base + 0xAAC);
    expect_scroll_close_defaults();
    expect(scroll_close_read(0xA10) == 0x13579BDFU);
    scroll_close_probe_state.order = 1;
    return 0x11112222U;
}

uint32_t __fastcall scroll_close_right_probe(void *self, void *) {
    expect(scroll_close_probe_state.order == 1);
    expect(self == scroll_close_probe_state.base + 0x15F8);
    expect_scroll_close_defaults();
    expect(scroll_close_read(0xA10) == 0x13579BDFU);
    scroll_close_probe_state.order = 2;
    return 0x33334444U;
}

namespace {

#if defined(__GNUC__)

#pragma GCC diagnostic push

#pragma GCC diagnostic ignored "-Wattributes"

#endif

}  // namespace

void __thiscall scroll_close_win_probe(void *self) {
    expect(scroll_close_probe_state.order == 2);
    expect(self == scroll_close_probe_state.base);
    expect_scroll_close_defaults();
    expect(scroll_close_read(0xA10) == 0x13579BDFU);
    scroll_close_probe_state.order = 3;
}

void __thiscall scroll_close_buffer_probe(void *self) {
    expect(scroll_close_probe_state.order == 3);
    expect(self == scroll_close_probe_state.base + 0x444);
    expect_scroll_close_defaults();
    expect(scroll_close_read(0xA10) == 0x13579BDFU);
    scroll_close_probe_state.order = 4;
}

namespace {

#if defined(__GNUC__)

#pragma GCC diagnostic pop

#endif

void write_scroll_close_expected(uint8_t *expected,
                                 const uint32_t fixed[11],
                                 const uint32_t dynamic[17],
                                 uint32_t base_result) {
    write_at(expected, 16 + 0xA14, dynamic[0]);
    write_at(expected, 16 + 0xA1C, fixed[3]);
    write_at(expected, 16 + 0xA20, dynamic[1]);
    write_at(expected, 16 + 0xA24, fixed[4]);
    write_at(expected, 16 + 0xA28, 0U);
    write_at(expected, 16 + 0xA2C, dynamic[1]);
    write_at(expected, 16 + 0xA30, fixed[2]);
    write_at(expected, 16 + 0xA34, fixed[1]);
    write_at(expected, 16 + 0xA38, 0U);
    write_at(expected, 16 + 0xA3C, 0xFFFFFFFFU);
    write_at(expected, 16 + 0xA40, fixed[0]);
    write_at(expected, 16 + 0xA44, 0U);
    write_at(expected, 16 + 0xA48, fixed[5]);
    for (size_t offset = 0xA4C; offset <= 0xA58; offset += 4) {
        write_at(expected, 16 + offset, 0U);
    }
    write_at(expected, 16 + 0xA5C, fixed[6]);
    write_at(expected, 16 + 0xA64, fixed[7]);
    write_at(expected, 16 + 0xA68, fixed[8]);
    write_at(expected, 16 + 0xA6C, fixed[9]);
    write_at(expected, 16 + 0xA70, fixed[10]);
    for (size_t index = 0; index < 3; ++index) {
        write_at(expected, 16 + 0xA7C + index * 4, dynamic[2 + index]);
        write_at(expected, 16 + 0xA88 + index * 4, dynamic[5 + index]);
        write_at(expected, 16 + 0xA94 + index * 4, dynamic[8 + index]);
        write_at(expected, 16 + 0xAA0 + index * 4, dynamic[11 + index]);
    }
    write_at(expected, 16 + 0xA74, dynamic[15]);
    write_at(expected, 16 + 0xA78, dynamic[16]);
    write_at(expected, 16 + 0x2144, 0U);
    write_at(expected, 16 + 0x2148, 0U);

    write_at(expected, 16 + 0x134, 0U);
    write_at(expected, 16 + 0x138, 0U);
    for (size_t offset = 0x9CC; offset <= 0xA04; offset += 4) {
        write_at(expected, 16 + offset, 0U);
    }
    write_at(expected, 16 + 0xA0C, base_result);
    write_at(expected, 16 + 0xA10, 0U);
}

void test_scroll_close() {
    uint32_t fixed[11];
    uint32_t dynamic[17];
    for (size_t index = 0; index < ARRAYSIZE(fixed); ++index) {
        fixed[index] = 0x51000000U + static_cast<uint32_t>(index) * 0x010203U;
    }
    for (size_t index = 0; index < ARRAYSIZE(dynamic); ++index) {
        dynamic[index] = 0xA1000000U + static_cast<uint32_t>(index) * 0x010101U;
    }
    dynamic[14] = 0xDEADC0DEU;
    uint32_t base_result = 0x7B3D19E5U;

    uint32_t *const saved_fixed = ScrollCloseStaticDefaults;
    uint32_t *const saved_dynamic = ScrollCloseDynamicDefaults;
    func_subobject_close *const saved_win = WinOriginalClose;
    func_subobject_close *const saved_buffer = BufferSubobjectClose;
    uint32_t *const saved_base_default = GraphicWinFieldA0CDefault;
    ScrollCloseStaticDefaults = fixed;
    ScrollCloseDynamicDefaults = dynamic;
    WinOriginalClose = scroll_close_win_probe;
    BufferSubobjectClose = scroll_close_buffer_probe;
    GraphicWinFieldA0CDefault = &base_result;

    for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
        alignas(Scroll) uint8_t storage[sizeof(Scroll) + 32];
        uint8_t expected[sizeof(storage)];
        uintptr_t left_vtable[0x16C / sizeof(uintptr_t)] = {};
        uintptr_t right_vtable[0x16C / sizeof(uintptr_t)] = {};
        left_vtable[0x168 / sizeof(uintptr_t)] =
            reinterpret_cast<uintptr_t>(&scroll_close_left_probe);
        right_vtable[0x168 / sizeof(uintptr_t)] =
            reinterpret_cast<uintptr_t>(&scroll_close_right_probe);
        seed_storage(storage, expected, sizeof(storage));
        uintptr_t *left_vtable_pointer = left_vtable;
        uintptr_t *right_vtable_pointer = right_vtable;
        write_at(storage, 16 + 0xAAC, left_vtable_pointer);
        write_at(storage, 16 + 0x15F8, right_vtable_pointer);
        write_at(storage, 16 + 0xA08, 0U);
        write_at(storage, 16 + 0xA10, 0x13579BDFU);
        std::memcpy(expected, storage, sizeof(storage));
        write_scroll_close_expected(expected, fixed, dynamic, base_result);

        auto *scroll = reinterpret_cast<Scroll *>(storage + 16);
        scroll_close_probe_state = {
            storage + 16, fixed, dynamic, 0,
        };
        const uint32_t result = use_adapter
            ? scroll_close_redirect(scroll, nullptr)
            : scroll->close();
        expect(result == base_result);
        expect(scroll_close_probe_state.order == 4);
        expect_storage_bytes(storage, expected, sizeof(storage));
    }

    ScrollCloseStaticDefaults = saved_fixed;
    ScrollCloseDynamicDefaults = saved_dynamic;
    WinOriginalClose = saved_win;
    BufferSubobjectClose = saved_buffer;
    GraphicWinFieldA0CDefault = saved_base_default;
}

int __cdecl scroll_init_probe(Scroll *self, int x, int y, int width, int height,
                              Win *parent, int setting, int options) {
    ++scroll_init_probe_state.calls;
    scroll_init_probe_state.self = self;
    scroll_init_probe_state.x = static_cast<uint32_t>(x);
    scroll_init_probe_state.y = static_cast<uint32_t>(y);
    scroll_init_probe_state.width = static_cast<uint32_t>(width);
    scroll_init_probe_state.height = static_cast<uint32_t>(height);
    scroll_init_probe_state.parent = parent;
    scroll_init_probe_state.setting = static_cast<uint32_t>(setting);
    scroll_init_probe_state.options = static_cast<uint32_t>(options);
    scroll_init_probe_state.nonclient = ScrollNonClientInit
        ? *ScrollNonClientInit : INT_MIN;
    if (scroll_init_probe_state.clear_nonclient) {
        *ScrollNonClientInit = 0;
    }
    return int_from_bits(scroll_init_probe_state.result);
}

void reset_scroll_init_probe(uint32_t result) {
    std::memset(&scroll_init_probe_state, 0, sizeof(scroll_init_probe_state));
    scroll_init_probe_state.result = result;
}

int call_scroll_axis(Scroll *scroll, int kind, bool use_adapter, int x, int y,
                     int length, Win *parent, int setting) {
    if (kind == 0) {
        return use_adapter
            ? scroll_init_vert_redirect(
                scroll, nullptr, x, y, length, parent, setting)
            : scroll->init_vert(x, y, length, parent, setting);
    }
    if (kind == 1) {
        return use_adapter
            ? scroll_init_horz_redirect(
                scroll, nullptr, x, y, length, parent, setting)
            : scroll->init_horz(x, y, length, parent, setting);
    }
    if (kind == 2) {
        return use_adapter
            ? scroll_init_vert_nc_redirect(
                scroll, nullptr, x, y, length, parent, setting)
            : scroll->init_vert_nc(x, y, length, parent, setting);
    }
    return use_adapter
        ? scroll_init_horz_nc_redirect(
            scroll, nullptr, x, y, length, parent, setting)
        : scroll->init_horz_nc(x, y, length, parent, setting);
}

void test_scroll_init_wrappers() {
    ScrollPrimaryInitProc const saved_primary_init = ScrollPrimaryInit;
    int *const saved_default_thickness = ScrollDefaultThickness;
    int *const saved_nonclient_init = ScrollNonClientInit;
    int default_thickness = 0;
    int nonclient_init = 0;
    ScrollPrimaryInit = &scroll_init_probe;
    ScrollDefaultThickness = &default_thickness;
    ScrollNonClientInit = &nonclient_init;

    alignas(Scroll) uint8_t storage[sizeof(Scroll) + 32];
    uint8_t expected[sizeof(storage)];
    alignas(Win) uint8_t parent_storage[sizeof(Win)];
    auto *scroll = reinterpret_cast<Scroll *>(storage + 16);
    auto *parent = reinterpret_cast<Win *>(parent_storage);

    for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
        seed_storage(storage, expected, sizeof(storage));
        reset_scroll_init_probe(0xA55AA55AU);
        const int null_rect_result = use_adapter
            ? scroll_init_rect_redirect(
                scroll, nullptr, nullptr, parent, INT_MIN, INT_MAX)
            : scroll->init(nullptr, parent, INT_MIN, INT_MAX);
        expect(null_rect_result == 3);
        expect(scroll_init_probe_state.calls == 0);
        expect_storage_bytes(storage, expected, sizeof(storage));

        reset_scroll_init_probe(0xA55AA55AU);
        auto *poison_rect = reinterpret_cast<RECT *>(1U);
        const int null_parent_result = use_adapter
            ? scroll_init_rect_redirect(
                scroll, nullptr, poison_rect, nullptr, INT_MAX, INT_MIN)
            : scroll->init(poison_rect, nullptr, INT_MAX, INT_MIN);
        expect(null_parent_result == 3);
        expect(scroll_init_probe_state.calls == 0);
        expect_storage_bytes(storage, expected, sizeof(storage));

        alignas(RECT) uint8_t rect_storage[sizeof(RECT) + 32];
        uint8_t rect_expected[sizeof(rect_storage)];
        seed_storage(rect_storage, rect_expected, sizeof(rect_storage));
        const uint32_t coordinates[] = {
            0x7FFFFFFFU, 0x80000000U, 0x80000000U, 0x7FFFFFFFU,
        };
        std::memcpy(rect_storage + 16, coordinates, sizeof(coordinates));
        std::memcpy(rect_expected, rect_storage, sizeof(rect_storage));
        auto *rect = reinterpret_cast<RECT *>(rect_storage + 16);
        nonclient_init = int_from_bits(0x2468ACE0U);
        reset_scroll_init_probe(0x89ABCDEFU);
        const int result = use_adapter
            ? scroll_init_rect_redirect(
                scroll, nullptr, rect, parent,
                int_from_bits(0x13579BDFU), int_from_bits(0xFEDCBA98U))
            : scroll->init(rect, parent,
                           int_from_bits(0x13579BDFU),
                           int_from_bits(0xFEDCBA98U));
        expect(static_cast<uint32_t>(result) == 0x89ABCDEFU);
        expect(scroll_init_probe_state.calls == 1);
        expect(scroll_init_probe_state.self == scroll);
        expect(scroll_init_probe_state.x == coordinates[0]);
        expect(scroll_init_probe_state.y == coordinates[1]);
        expect(scroll_init_probe_state.width
               == coordinates[2] - coordinates[0]);
        expect(scroll_init_probe_state.height
               == coordinates[3] - coordinates[1]);
        expect(scroll_init_probe_state.parent == parent);
        expect(scroll_init_probe_state.setting == 0x13579BDFU);
        expect(scroll_init_probe_state.options == 0xFEDCBA98U);
        expect(scroll_init_probe_state.nonclient
               == int_from_bits(0x2468ACE0U));
        expect(nonclient_init == int_from_bits(0x2468ACE0U));
        expect_storage_bytes(storage, expected, sizeof(storage));
        expect_storage_bytes(rect_storage, rect_expected, sizeof(rect_storage));
    }

    for (int kind = 0; kind < 4; ++kind) {
        const bool nonclient = kind >= 2;
        for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
            seed_storage(storage, expected, sizeof(storage));
            ScrollDefaultThickness = nullptr;
            nonclient_init = int_from_bits(0x2468ACE0U);
            reset_scroll_init_probe(0xA55AA55AU);
            expect(call_scroll_axis(
                       scroll, kind, use_adapter, 10, 20, 0, parent, 30) == 3);
            expect(scroll_init_probe_state.calls == 0);
            expect(nonclient_init == (nonclient
                ? 1 : int_from_bits(0x2468ACE0U)));
            expect_storage_bytes(storage, expected, sizeof(storage));

            nonclient_init = int_from_bits(0x13579BDFU);
            reset_scroll_init_probe(0x5AA55AA5U);
            expect(call_scroll_axis(
                       scroll, kind, use_adapter, INT_MIN, INT_MAX,
                       -1, nullptr, 0) == 3);
            expect(scroll_init_probe_state.calls == 0);
            expect(nonclient_init == (nonclient
                ? 1 : int_from_bits(0x13579BDFU)));
            expect_storage_bytes(storage, expected, sizeof(storage));
            ScrollDefaultThickness = &default_thickness;
        }
    }

    struct AxisCase {
        int kind;
        uint32_t x;
        uint32_t y;
        uint32_t length;
        uint32_t thickness;
        uint32_t setting;
    };
    const AxisCase axis_cases[] = {
        {0, 0x80000000U, 0x7FFFFFFFU, 0x80000000U, 0U, 0xFFFFFFFFU},
        {1, 0xFFFFFFFFU, 0U, 7U, 0xFFFFFFFFU, 0x80000000U},
        {2, 10U, 20U, 0xFFFFFFFFU, 0x7FFFFFFFU, 0x13579BDFU},
        {3, 30U, 40U, 0x7FFFFFFFU, 0x80000000U, 0xFEDCBA98U},
    };
    for (const AxisCase &test : axis_cases) {
        for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
            seed_storage(storage, expected, sizeof(storage));
            default_thickness = int_from_bits(test.thickness);
            nonclient_init = int_from_bits(0x2468ACE0U);
            const uint32_t expected_result = 0x89ABCDEFU
                ^ (static_cast<uint32_t>(test.kind) * 0x11111111U)
                ^ static_cast<uint32_t>(use_adapter);
            reset_scroll_init_probe(expected_result);
            scroll_init_probe_state.clear_nonclient = test.kind >= 2;
            const int result = call_scroll_axis(
                scroll, test.kind, use_adapter,
                int_from_bits(test.x), int_from_bits(test.y),
                int_from_bits(test.length), parent, int_from_bits(test.setting));
            expect(static_cast<uint32_t>(result) == expected_result);
            expect(scroll_init_probe_state.calls == 1);
            expect(scroll_init_probe_state.self == scroll);
            expect(scroll_init_probe_state.x == test.x);
            expect(scroll_init_probe_state.y == test.y);
            expect(scroll_init_probe_state.width
                   == ((test.kind & 1) ? test.length : test.thickness));
            expect(scroll_init_probe_state.height
                   == ((test.kind & 1) ? test.thickness : test.length));
            expect(scroll_init_probe_state.parent == parent);
            expect(scroll_init_probe_state.setting == test.setting);
            expect(scroll_init_probe_state.options == 0U);
            expect(scroll_init_probe_state.nonclient == (test.kind >= 2
                ? 1 : int_from_bits(0x2468ACE0U)));
            expect(nonclient_init == (test.kind >= 2
                ? 0 : int_from_bits(0x2468ACE0U)));
            expect_storage_bytes(storage, expected, sizeof(storage));
        }
    }

    ScrollPrimaryInit = saved_primary_init;
    ScrollDefaultThickness = saved_default_thickness;
    ScrollNonClientInit = saved_nonclient_init;
}

}  // namespace

int scroll_redraw_calls = 0;

Scroll *scroll_redraw_self = nullptr;

uint32_t scroll_redraw_result = 0;

uint32_t scroll_redraw_minimum = 0;

uint32_t scroll_redraw_maximum = 0;

uint32_t scroll_redraw_reverse = 0;

uint32_t scroll_redraw_position = 0;

Win *scroll_redraw_parent = nullptr;

bool scroll_redraw_check_range = false;

bool scroll_redraw_check_current = false;

namespace {

uint32_t __fastcall scroll_redraw_probe(Scroll *self, void *) {
    ++scroll_redraw_calls;
    scroll_redraw_self = self;
    const auto *bytes = reinterpret_cast<const uint8_t *>(self);
    uint32_t minimum;
    uint32_t maximum;
    uint32_t reverse;
    uint32_t position;
    std::memcpy(&minimum, bytes + 0xA20, sizeof(minimum));
    std::memcpy(&maximum, bytes + 0xA24, sizeof(maximum));
    std::memcpy(&reverse, bytes + 0xA28, sizeof(reverse));
    std::memcpy(&position, bytes + 0xA2C, sizeof(position));
    if (scroll_redraw_check_range) {
        expect(minimum == scroll_redraw_minimum);
        expect(maximum == scroll_redraw_maximum);
        expect(reverse == scroll_redraw_reverse);
        expect(position == scroll_redraw_position);
    }
    if (scroll_redraw_check_current) {
        expect(*ScrollCurrentWin == scroll_redraw_parent);
        expect(position == scroll_redraw_position);
    }
    return scroll_redraw_result;
}

}  // namespace

void reset_scroll_redraw_probe() {
    scroll_redraw_calls = 0;
    scroll_redraw_self = nullptr;
    scroll_redraw_check_range = false;
    scroll_redraw_check_current = false;
}

void install_scroll_redraw_probe(uint8_t *storage, uintptr_t vtable[63]) {
    std::memset(vtable, 0, sizeof(uintptr_t) * 63);
    vtable[0xF8 / sizeof(uintptr_t)] =
        reinterpret_cast<uintptr_t>(&scroll_redraw_probe);
    uintptr_t *vtable_pointer = vtable;
    write_at(storage, 16, vtable_pointer);
}

int scroll_style_redraw_calls = 0;

uint8_t *scroll_style_base = nullptr;

size_t scroll_style_offsets[3] = {};

uint32_t scroll_style_value = 0;

uint32_t scroll_style_results[2] = {};

namespace {

void expect_scroll_style_fields() {
    for (size_t offset : scroll_style_offsets) {
        uint32_t actual;
        std::memcpy(&actual, scroll_style_base + offset, sizeof(actual));
        expect(actual == scroll_style_value);
    }
}

uint32_t __fastcall scroll_style_left_redraw_probe(void *self, void *) {
    expect(scroll_style_redraw_calls == 0);
    expect(self == scroll_style_base + 0xAAC);
    expect_scroll_style_fields();
    ++scroll_style_redraw_calls;
    return scroll_style_results[0];
}

uint32_t __fastcall scroll_style_right_redraw_probe(void *self, void *) {
    expect(scroll_style_redraw_calls == 1);
    expect(self == scroll_style_base + 0x15F8);
    expect_scroll_style_fields();
    ++scroll_style_redraw_calls;
    return scroll_style_results[1];
}

}  // namespace

void install_scroll_style_redraw_probes(
        uint8_t *storage, uintptr_t left_vtable[63],
        uintptr_t right_vtable[63]) {
    std::memset(left_vtable, 0, sizeof(uintptr_t) * 63);
    std::memset(right_vtable, 0, sizeof(uintptr_t) * 63);
    left_vtable[0xF8 / sizeof(uintptr_t)] =
        reinterpret_cast<uintptr_t>(&scroll_style_left_redraw_probe);
    right_vtable[0xF8 / sizeof(uintptr_t)] =
        reinterpret_cast<uintptr_t>(&scroll_style_right_redraw_probe);
    uintptr_t *left_vtable_pointer = left_vtable;
    uintptr_t *right_vtable_pointer = right_vtable;
    write_at(storage, 16 + 0xAAC, left_vtable_pointer);
    write_at(storage, 16 + 0x15F8, right_vtable_pointer);
}

namespace {

void test_win_move() {
    struct MoveCase {
        uint32_t flags;
        uint32_t x;
        uint32_t y;
        uint32_t left;
        uint32_t top;
        uint32_t right;
        uint32_t bottom;
    };
    const MoveCase cases[] = {
        {0U, 100U, 200U, 10U, 20U, 50U, 80U},
        {0xFFFFFFFDU, 0xFFFFFF9CU, 0x0000012CU,
         0x00000032U, 0xFFFFFFCEU, 0x00000096U, 0x0000004BU},
        {2U, 0x80000000U, 0x7FFFFFFFU,
         0x7FFFFFFFU, 0x80000000U, 0x80000005U, 0x7FFFFFF0U},
        {0xA5A5A5A6U, 0xFFFFFFFFU, 0U,
         0x80000000U, 0xFFFFFFFFU, 0x7FFFFFFFU, 0x80000000U},
    };

    for (const MoveCase &fixture : cases) {
        for (int use_adapter = 0; use_adapter < 2; ++use_adapter) {
            alignas(Win) uint8_t storage[sizeof(Win) + 32];
            uint8_t expected[sizeof(storage)];
            seed_storage(storage, expected, sizeof(storage));
            write_at(storage, 16 + 0x9C, fixture.flags);
            const uint32_t decoy_rect[4] = {
                0x11223344U, 0x55667788U, 0x99AABBCCU, 0xDDEEFF00U,
            };
            const size_t target = (fixture.flags & 2U) ? 0x14C : 0x13C;
            const size_t decoy = (fixture.flags & 2U) ? 0x13C : 0x14C;
            std::memcpy(storage + 16 + decoy, decoy_rect, sizeof(decoy_rect));
            write_at(storage, 16 + target, fixture.left);
            write_at(storage, 16 + target + 4, fixture.top);
            write_at(storage, 16 + target + 8, fixture.right);
            write_at(storage, 16 + target + 12, fixture.bottom);
            std::memcpy(expected, storage, sizeof(storage));
            const uint32_t dx = fixture.x - fixture.left;
            const uint32_t dy = fixture.y - fixture.top;
            write_at(expected, 16 + target, fixture.x);
            write_at(expected, 16 + target + 4, fixture.y);
            write_at(expected, 16 + target + 8, fixture.right + dx);
            write_at(expected, 16 + target + 12, fixture.bottom + dy);

            auto *window = reinterpret_cast<Win *>(storage + 16);
            const int result = use_adapter
                ? win_move_redirect(window, nullptr,
                                    int_from_bits(fixture.x), int_from_bits(fixture.y))
                : window->move(int_from_bits(fixture.x), int_from_bits(fixture.y));
            expect(result == 0);
            expect_storage_bytes(storage, expected, sizeof(storage));
        }
    }
}

}  // namespace

PlayerLockProbe g_player_lock_probe = {};

uint8_t *g_player_lock_object = nullptr;

int __thiscall probe_player_lock_lock(void *entry, int a1, int a2, int a3, int a4) {
    ++g_player_lock_probe.lock_calls;
    g_player_lock_probe.lock_entry = entry;
    g_player_lock_probe.lock_faction = a1;
    g_player_lock_probe.lock_flags = a2;
    g_player_lock_probe.lock_x = a3;
    g_player_lock_probe.lock_y = a4;
    return 0x1234ABCD;
}

void __thiscall probe_player_lock_unlock(void *entry, int slot) {
    if (g_player_lock_probe.unlock_calls < 4) {
        g_player_lock_probe.unlock_entries[g_player_lock_probe.unlock_calls] = entry;
    }
    ++g_player_lock_probe.unlock_calls;
    g_player_lock_probe.unlock_faction = slot;
    // The active byte must still be SET while the entries are being released;
    // the original clears it after the loop.
    if (g_player_lock_object) {
        g_player_lock_probe.active_during_last_unlock = *g_player_lock_object;
    }
}

Dialog *g_base_pop_item_dialog = nullptr;

char *g_base_pop_item_text = nullptr;

int g_base_pop_item_index = 0;

int g_base_pop_item_calls = 0;

int __thiscall probe_base_pop_item(Dialog *dialog, char *text, int index) {
    ++g_base_pop_item_calls;
    g_base_pop_item_dialog = dialog;
    g_base_pop_item_text = text;
    g_base_pop_item_index = index;
    return 0x5A5A1234;
}

namespace {

// Ambience::construct reaches Sound::set_type through a seam that still
// points into the original image. A __thiscall no-op has to be a real
// function: a lambda will not convert to that calling convention.
void __thiscall leaf_stub_sound_set_type(Wave *, uint32_t) { ; }

// Recorders for the two destructor chains. The four callees are seams, so the
// chain can be checked for what it IS - three calls, three pointers, one order
// - without standing up three class teardown fixtures.
struct LeafDtorCall { int which; const void *target; };

LeafDtorCall g_leaf_dtor_calls[8];

int g_leaf_dtor_count;

void leaf_dtor_record(int which, const void *target) {
    if (g_leaf_dtor_count < 8) {
        g_leaf_dtor_calls[g_leaf_dtor_count].which = which;
        g_leaf_dtor_calls[g_leaf_dtor_count].target = target;
    }
    ++g_leaf_dtor_count;
}

uint32_t __fastcall leaf_dtor_listbox(void *self, void *) {
    leaf_dtor_record(1, self);
    return 0;
}

uint32_t __fastcall leaf_dtor_dialogs(void *self, void *) {
    leaf_dtor_record(2, self);
    return 0;
}

void __fastcall leaf_dtor_dialog(Dialog *self, void *) {
    leaf_dtor_record(3, self);
}

Buffer *__fastcall leaf_buffer_ctor_probe(Buffer *self, void *) {
    leaf_dtor_record(5, self);
    return self;
}

void __fastcall leaf_buffer_dtor_probe(Buffer *self, void *) {
    leaf_dtor_record(6, self);
}

GraphicWin *__fastcall leaf_dtor_graphic(GraphicWin *self, void *) {
    leaf_dtor_record(4, self);
    return self;
}

// Recorders for the two destructor chains. Every destructor they reach funnels
// through GraphicWin::close's two subobject seams, so logging the pointer each
// one is handed is enough to say WHICH subobject each call got - which is the
// entire content of those two functions.
void *g_chain_targets[16];

int g_chain_count;

void __thiscall leaf_chain_record(void *target) {
    if (g_chain_count < 16) {
        g_chain_targets[g_chain_count] = target;
    }
    ++g_chain_count;
}

bool leaf_chain_saw(const void *target) {
    for (int index = 0; index < g_chain_count && index < 16; ++index) {
        if (g_chain_targets[index] == target) {
            return true;
        }
    }
    return false;
}

// Recorder for sub_59d230's only call. Kept separate from the Dialogs::item
// fixture's own recorders so this test stands on its own.
Dialog *g_leaf_item_dialog;

char *g_leaf_item_text;

int g_leaf_item_index;

int g_leaf_item_calls;

int __thiscall leaf_stub_dialog_item(Dialog *dialog, char *text, int index) {
    g_leaf_item_dialog = dialog;
    g_leaf_item_text = text;
    g_leaf_item_index = index;
    ++g_leaf_item_calls;
    return 0x1234ABCD;
}

void test_leaf_recoveries() {
    // --- float vector arithmetic ---
    //
    // Values chosen so the results are EXACT in float: sums and products of
    // small dyadic rationals, so `==` is the right comparison and says the
    // arithmetic is right rather than approximately right.
    {
        const float left[3] = {1.5f, -2.25f, 8.0f};
        const float right[3] = {0.5f, 0.25f, -4.0f};
        float result[3] = {99.0f, 99.0f, 99.0f};

        leaf_00628180_redirect(left, right, result);
        expect(result[0] == 1.0f);
        expect(result[1] == -2.5f);
        expect(result[2] == 12.0f);

        // The operand ORDER matters: subtraction does not commute, so a body
        // computing right - left would pass any test using symmetric inputs.
        leaf_00628180_redirect(right, left, result);
        expect(result[0] == -1.0f);
        expect(result[1] == 2.5f);
        expect(result[2] == -12.0f);

        // Distinct scale factors per component would go unnoticed if every
        // component held the same value, so they differ.
        leaf_006281b0_redirect(left, 4.0f, result);
        expect(result[0] == 6.0f);
        expect(result[1] == -9.0f);
        expect(result[2] == 32.0f);

        leaf_006281b0_redirect(left, 0.0f, result);
        expect(result[0] == 0.0f);
        expect(result[1] == 0.0f);
        expect(result[2] == 0.0f);

        // Dot product:
        //   0.5*1.5 + 0.25*-2.25 + -4*8 = 0.75 - 0.5625 - 32 = -31.8125
        expect(leaf_00634650_redirect(const_cast<float *>(left), nullptr,
                                      right) == -31.8125f);

        // A vector against itself: 1.5^2 + 2.25^2 + 8^2 = 2.25+5.0625+64
        expect(leaf_00634650_redirect(const_cast<float *>(left), nullptr,
                                      left) == 71.3125f);

        // THE SUMMATION ORDER, which the two cases above do not pin.
        //
        // The original adds component 2 first, then 1, then 0, and floating-
        // point addition is not associative - but x87 keeps intermediates in
        // 64-bit EXTENDED precision, so ordinary magnitudes give the same
        // answer either way. I checked that rather than assumed it: reversing
        // the order in the body left every assertion above passing.
        //
        // Distinguishing it needs terms whose magnitudes differ by more than
        // the extended mantissa. With products 1, 2^100 and -2^100:
        //     (p2 + p1) + p0 = (-2^100 + 2^100) + 1 = 1     <- the original
        //     (p0 + p1) + p2 = (1 + 2^100) - 2^100 = 0      <- reversed,
        //                                                      1 is below the
        //                                                      ulp of 2^100
        const float ones[3] = {1.0f, 1.0f, 1.0f};
        const float spread[3] = {1.0f, 0x1p100f, -0x1p100f};
        expect(leaf_00634650_redirect(const_cast<float *>(ones), nullptr,
                                      spread) == 1.0f);
    }

    // --- two-link chain, and the zero guard on a DIFFERENT field ---
    {
        uint8_t object[0x600 + 32] = {};
        uint8_t middle[16] = {};
        uint8_t leaf[16] = {};
        void *const self = object + 16;
        auto put = [&](uint8_t *where, size_t offset, uint32_t value) {
            std::memcpy(where + offset, &value, sizeof(value));
        };
        auto put_ptr = [&](uint8_t *where, size_t offset, const void *value) {
            std::memcpy(where + offset, &value, sizeof(value));
        };

        put(object + 16, 0x8, 0);                 // guard clear -> 0
        put_ptr(object + 16, 0xc, middle);
        put_ptr(middle, 0x8, leaf);
        put(leaf, 0x4, 0xDEADBEEF);
        expect(leaf_005e3630_redirect(self, nullptr) == 0);

        // The guard is field 8; the chase starts at field 0xc. Setting only
        // the guard must make it follow the chain that was already there.
        put(object + 16, 0x8, 1);
        expect(leaf_005e3630_redirect(self, nullptr) == 0xDEADBEEF);

        // --- field 0 as an index, with a SIGNED test ---
        put(object + 16, 0x0, 5);
        put(object + 16, 0xc, 111);
        put(object + 16, 0x10, 1000);
        expect(leaf_005e3650_redirect(self, nullptr) == 1005);

        put(object + 16, 0x0, 0);
        expect(leaf_005e3650_redirect(self, nullptr) == 1000);

        // 0x80000000 is negative signed and huge unsigned: an unsigned test
        // would take the other branch and return 1000 + 0x80000000.
        put(object + 16, 0x0, 0x80000000U);
        expect(leaf_005e3650_redirect(self, nullptr) == 111);
        put(object + 16, 0x0, 0xFFFFFFFFU);       // -1
        expect(leaf_005e3650_redirect(self, nullptr) == 111);

        // --- clamp to 0..3, inclusive at BOTH ends ---
        auto stored = [&](size_t offset) {
            uint32_t value;
            std::memcpy(&value, object + 16 + offset, sizeof(value));
            return value;
        };
        for (int probe : {0, 1, 3}) {             // kept
            put(object + 16, 0x51c, 0x5A5A5A5AU);
            leaf_005e3660_redirect(self, nullptr, probe);
            expect(stored(0x510) == static_cast<uint32_t>(probe));
            expect(stored(0x514) == static_cast<uint32_t>(probe));
            expect(stored(0x518) == static_cast<uint32_t>(probe));
            expect(stored(0x51c) == 0);
        }
        for (int probe : {-1, 4, 100}) {          // clamped to zero
            leaf_005e3660_redirect(self, nullptr, 7);
            leaf_005e3660_redirect(self, nullptr, probe);
            expect(stored(0x510) == 0);
            expect(stored(0x514) == 0);
            expect(stored(0x518) == 0);
        }

        // --- store, or refuse with 10 and touch nothing ---
        put(object + 16, 0x38, 0x11223344U);
        expect(leaf_004482f0_redirect(self, nullptr, 0) == 0xA);
        expect(stored(0x38) == 0x11223344U);      // the refusal stores NOTHING
        expect(leaf_004482f0_redirect(self, nullptr, 77) == 0);
        expect(stored(0x38) == 77);

        // --- clear one field, set a bit in another, constant in a third ---
        put(object + 16, 0x0, 0xFFFFFFFFU);
        put(object + 16, 0x4, 0xFFFFFFFFU);
        put(object + 16, 0x8, 0xF0F0F0F0U);       // low bit clear
        expect(leaf_004c8070_redirect(self, nullptr) == self);
        expect(stored(0x0) == 0x24);
        expect(stored(0x4) == 0);
        expect(stored(0x8) == 0xF0F0F0F1U);       // ORed, not overwritten
    }

    // --- 3x3 matrix arithmetic: NINE elements, not three ---
    //
    // The nested loops in the originals never reset their pointer, so they
    // walk nine consecutive floats. A body that did three would leave the
    // last six untouched, which is why every element here is distinct and
    // the elements past the ninth are checked to be UNTOUCHED.
    {
        float matrix[12];
        float other[12];
        for (int i = 0; i < 12; ++i) {
            matrix[i] = static_cast<float>(i + 1);          // 1..12
            other[i] = static_cast<float>((i + 1) * 16);    // 16..192
        }
        leaf_006347c0_redirect(matrix, nullptr, other);
        for (int i = 0; i < 9; ++i) {
            expect(matrix[i] == static_cast<float>((i + 1) + (i + 1) * 16));
        }
        for (int i = 9; i < 12; ++i) {
            expect(matrix[i] == static_cast<float>(i + 1));  // past the end
        }

        for (int i = 0; i < 12; ++i) { matrix[i] = static_cast<float>(i + 1); }
        leaf_006348f0_redirect(matrix, nullptr, other);
        for (int i = 0; i < 9; ++i) {
            // this - other, which is negative here: the other order would
            // give the same magnitude and the wrong sign.
            expect(matrix[i] == static_cast<float>((i + 1) - (i + 1) * 16));
        }
        for (int i = 9; i < 12; ++i) {
            expect(matrix[i] == static_cast<float>(i + 1));
        }

        for (int i = 0; i < 12; ++i) { matrix[i] = static_cast<float>(i + 1); }
        leaf_006348c0_redirect(matrix, nullptr, 0.5f);
        for (int i = 0; i < 9; ++i) {
            expect(matrix[i] == static_cast<float>(i + 1) * 0.5f);
        }
        for (int i = 9; i < 12; ++i) {
            expect(matrix[i] == static_cast<float>(i + 1));
        }
    }

    // --- vector equality, including the NaN the x87 flags call equal ---
    {
        float mine[3] = {1.0f, 2.0f, 3.0f};
        float same[3] = {1.0f, 2.0f, 3.0f};
        expect(leaf_006344e0_redirect(mine, nullptr, same));

        // Each component in turn, so a body checking only the first would
        // pass the case above and fail here.
        for (int index = 0; index < 3; ++index) {
            float differs[3] = {1.0f, 2.0f, 3.0f};
            differs[index] = 99.0f;
            expect(!leaf_006344e0_redirect(mine, nullptr, differs));
        }

        // `test ah,0x40` reads C3, which x87 sets for equal OR UNORDERED, so
        // the original answers TRUE here. `a == b` in C++ answers false, and
        // that is the whole reason the body is written as "neither below nor
        // above" instead.
        const float quiet_nan = std::numeric_limits<float>::quiet_NaN();
        float with_nan[3] = {quiet_nan, 2.0f, 3.0f};
        expect(leaf_006344e0_redirect(mine, nullptr, with_nan));
        expect(leaf_006344e0_redirect(with_nan, nullptr, mine));
    }

    // --- vector length ---
    {
        const float unit[3] = {3.0f, 0.0f, 4.0f};       // 9 + 16 = 25
        expect(leaf_006281e0_redirect(unit) == 5.0f);
        const float zero[3] = {0.0f, 0.0f, 0.0f};
        expect(leaf_006281e0_redirect(zero) == 0.0f);
        // Negative components must square away, not cancel.
        const float signs[3] = {-3.0f, 0.0f, -4.0f};
        expect(leaf_006281e0_redirect(signs) == 5.0f);
        // The components are NOT interchangeable in a body that dropped one:
        // each of these has a single non-zero axis.
        for (int axis = 0; axis < 3; ++axis) {
            float single[3] = {0.0f, 0.0f, 0.0f};
            single[axis] = 6.0f;
            expect(leaf_006281e0_redirect(single) == 6.0f);
        }
    }

    // --- magic-number division, in the direction that distinguishes it ---
    //
    // The NEGATIVE cases carry the argument. Magic division without the
    // `shr 31 / add` sign correction floors instead of truncating, so -1/600
    // would be -1 rather than 0 - and every non-negative case would still
    // agree. The exact quotients here come from running the original's own
    // instruction sequence, not from reading the constants.
    {
        uint8_t object[0x3800] = {};
        auto set = [&](size_t offset, int32_t value) {
            std::memcpy(object + offset, &value, sizeof(value));
        };

        set(0x3798, 1);
        expect(leaf_0063beb0_redirect(object, nullptr, 600) == 1);
        expect(leaf_0063beb0_redirect(object, nullptr, 599) == 0);
        expect(leaf_0063beb0_redirect(object, nullptr, 1200) == 2);
        expect(leaf_0063beb0_redirect(object, nullptr, -1) == 0);   // not -1
        expect(leaf_0063beb0_redirect(object, nullptr, -600) == -1);
        expect(leaf_0063beb0_redirect(object, nullptr, -599) == 0); // not -1
        set(0x3798, 7);
        expect(leaf_0063beb0_redirect(object, nullptr, 100) == 1);  // 700/600
        expect(leaf_0063beb0_redirect(object, nullptr, -100) == -1);

        set(0x379c, 1);
        expect(leaf_0063bee0_redirect(object, nullptr, 440) == 1);
        expect(leaf_0063bee0_redirect(object, nullptr, 439) == 0);
        expect(leaf_0063bee0_redirect(object, nullptr, -439) == 0); // not -1
        expect(leaf_0063bee0_redirect(object, nullptr, -880) == -2);
        set(0x379c, 3);
        expect(leaf_0063bee0_redirect(object, nullptr, 300) == 2);  // 900/440

        // The two read DIFFERENT fields; a body using one for both would pass
        // everything above, because the fields were set to the same value.
        set(0x3798, 1);
        set(0x379c, 0);
        expect(leaf_0063beb0_redirect(object, nullptr, 600) == 1);
        expect(leaf_0063bee0_redirect(object, nullptr, 440) == 0);
    }

    // --- fill eight bytes, and only eight ---
    {
        uint8_t buffer[12];
        std::memset(buffer, 0x11, sizeof(buffer));
        leaf_0057dee0_redirect(buffer);
        for (int index = 0; index < 8; ++index) {
            expect(buffer[index] == 0xFF);
        }
        for (int index = 8; index < 12; ++index) {
            expect(buffer[index] == 0x11);      // past the end, untouched
        }
    }

    // --- ten 60-byte slots, wrapping ---
    {
        uint8_t pool[8 + 60 * 10] = {};
        int32_t *const counter = reinterpret_cast<int32_t *>(pool + 4);
        *counter = 0;
        for (int step = 0; step < 10; ++step) {
            // The slot returned is the one current ON ENTRY.
            expect(leaf_006252c0_redirect(pool, nullptr) == pool + 8 + step * 60);
        }
        // The tenth call wrapped the counter, so the eleventh starts over.
        expect(*counter == 0);
        expect(leaf_006252c0_redirect(pool, nullptr) == pool + 8);
        expect(*counter == 1);
    }

    // --- half a span plus an addend, truncating toward zero ---
    {
        uint8_t source[0x20] = {};
        auto set = [&](size_t offset, int32_t value) {
            std::memcpy(source + offset, &value, sizeof(value));
        };
        set(0x10, 100); set(0x14, 110);            // span 10
        expect(leaf_00408470_redirect(source, 0) == 5);
        expect(leaf_00408470_redirect(source, 1) == 5);   // 11/2 truncates
        expect(leaf_00408470_redirect(source, 2) == 6);
        // Negative: -11/2 is -5 truncating, -6 flooring. A bare `sar` floors.
        expect(leaf_00408470_redirect(source, -21) == -5);
        expect(leaf_00408470_redirect(source, -20) == -5);
        expect(leaf_00408470_redirect(source, -22) == -6);
        // The two fields are not interchangeable: swapping them flips the sign.
        set(0x10, 110); set(0x14, 100);
        expect(leaf_00408470_redirect(source, 0) == -5);
    }

    // --- fill 1,536 dwords starting at 0xa20, and nothing either side ---
    {
        static uint8_t region[0xA20 + 0x200 * 3 * 4 + 32];
        std::memset(region, 0x11, sizeof(region));
        leaf_005ad450_redirect(region, nullptr);
        for (size_t offset = 0; offset < 0xA20; ++offset) {
            expect(region[offset] == 0x11);        // before the region
        }
        const uint32_t *const slots =
            reinterpret_cast<const uint32_t *>(region + 0xA20);
        for (int index = 0; index < 0x200 * 3; ++index) {
            expect(slots[index] == 0xFFFFFFFFU);
        }
        for (size_t offset = 0xA20 + 0x200 * 3 * 4; offset < sizeof(region);
             ++offset) {
            expect(region[offset] == 0x11);        // after it
        }
    }

    // --- count bits 0..30, NOT 31 ---
    {
        expect(leaf_005cc430_redirect(0) == 0);
        expect(leaf_005cc430_redirect(1) == 1);
        expect(leaf_005cc430_redirect(3) == 2);
        expect(leaf_005cc430_redirect(0x7FFFFFFFU) == 31);
        // The top bit is never tested, so setting it changes nothing. A real
        // population count would answer 1 and 32 here.
        expect(leaf_005cc430_redirect(0x80000000U) == 0);
        expect(leaf_005cc430_redirect(0xFFFFFFFFU) == 31);
        expect(leaf_005cc430_redirect(0x40000000U) == 1);   // bit 30 counts
    }

    // --- 3, 0 or 7 ---
    {
        uint8_t source[0x20] = {};
        expect(leaf_005d7a10_redirect(nullptr, 0, 0, 0) == 3);
        const uint32_t zero = 0;
        std::memcpy(source + 0xC, &zero, sizeof(zero));
        expect(leaf_005d7a10_redirect(source, 0, 0, 0) == 7);
        for (uint32_t probe : {1U, 0xFFFFFFFFU, 0x80000000U, 0x100U}) {
            std::memcpy(source + 0xC, &probe, sizeof(probe));
            expect(leaf_005d7a10_redirect(source, 0, 0, 0) == 0);
        }
    }

    // --- forwarding all four arguments, unshuffled ---
    {
        uint8_t source[0x20] = {};
        uint8_t object[0x500] = {};
        const uint32_t set = 1;
        std::memcpy(source + 0xC, &set, sizeof(set));
        // The first argument must arrive as the first argument: 005D7A10
        // answers 3 for null, 0 for a set flag and 7 for a clear one, so a
        // forwarder that rotated them would answer 3 here.
        expect(leaf_005d5470_redirect(object, nullptr, source, 0, 0, 0) == 0);
        const uint32_t clear = 0;
        std::memcpy(source + 0xC, &clear, sizeof(clear));
        expect(leaf_005d5470_redirect(object, nullptr, source, 0, 0, 0) == 7);
        expect(leaf_005d5470_redirect(object, nullptr, nullptr, 1, 2, 3) == 3);
    }

    // --- reverse the low N bits, with a DO-WHILE that runs at least once ---
    {
        expect(leaf_00642940_redirect(0x1U, 3) == 0x4U);      // 001 -> 100
        expect(leaf_00642940_redirect(0x4U, 3) == 0x1U);      // 100 -> 001
        expect(leaf_00642940_redirect(0x3U, 3) == 0x6U);      // 011 -> 110
        expect(leaf_00642940_redirect(0x5U, 3) == 0x5U);      // palindrome
        expect(leaf_00642940_redirect(0xFFU, 8) == 0xFFU);
        expect(leaf_00642940_redirect(0x80U, 8) == 0x1U);
        expect(leaf_00642940_redirect(0x1U, 8) == 0x80U);
        // Bits above the count are ignored entirely.
        expect(leaf_00642940_redirect(0xFFFFFF01U, 3) == 0x4U);
        // The loop body runs before the counter is tested, so zero and
        // negative counts return BIT 0, not nothing.
        expect(leaf_00642940_redirect(0x1U, 0) == 0x1U);
        expect(leaf_00642940_redirect(0x2U, 0) == 0x0U);
        expect(leaf_00642940_redirect(0x1U, -5) == 0x1U);
        expect(leaf_00642940_redirect(0x1U, 1) == 0x1U);
    }

    // --- neighbours through two optional out-parameters ---
    {
        uint8_t holder[8] = {};
        uint8_t node[16] = {};
        const uint32_t previous = 0xAAAAAAAAU;
        const uint32_t next = 0xBBBBBBBBU;
        const uint32_t payload = 0xCCCCCCCCU;
        std::memcpy(node + 0, &previous, sizeof(previous));
        std::memcpy(node + 4, &next, sizeof(next));
        std::memcpy(node + 8, &payload, sizeof(payload));

        uint8_t *pointer = nullptr;
        std::memcpy(holder, &pointer, sizeof(pointer));
        // The bytes just past the slot are NOT zero, so the guard's read
        // offset is pinned: reading at 1 instead of 0 would see them, decide
        // the node is present, and walk off it.
        holder[4] = 0x5A; holder[5] = 0x5A; holder[6] = 0x5A; holder[7] = 0x5A;
        expect(leaf_0063e7f0_redirect(holder, nullptr, nullptr, nullptr) == 0);

        pointer = node;
        std::memcpy(holder, &pointer, sizeof(pointer));
        uint32_t got_first = 0;
        uint32_t got_second = 0;
        expect(leaf_0063e7f0_redirect(holder, nullptr, &got_first,
                                      &got_second) == payload);
        // first gets node->[0], second gets node->[4] - not the other way.
        expect(got_first == previous);
        expect(got_second == next);

        // Either out-parameter may be null, independently.
        got_first = 0; got_second = 0;
        expect(leaf_0063e7f0_redirect(holder, nullptr, &got_first,
                                      nullptr) == payload);
        expect(got_first == previous);
        expect(got_second == 0);
        got_first = 0;
        expect(leaf_0063e7f0_redirect(holder, nullptr, nullptr,
                                      &got_second) == payload);
        expect(got_first == 0);
        expect(got_second == next);
    }

    // --- divide, rounding away from zero when there is a remainder ---
    {
        int quotient = 0;
        int remainder = 0;
        expect(leaf_00532a50_redirect(10, &quotient, &remainder, 5) == 2);
        expect(quotient == 2);
        expect(remainder == 0);          // exact: no rounding
        expect(leaf_00532a50_redirect(11, &quotient, &remainder, 5) == 3);
        expect(quotient == 3);           // 2 rounded up
        expect(remainder == 1);
        expect(leaf_00532a50_redirect(0, &quotient, &remainder, 5) == 0);
        expect(quotient == 0);
        expect(remainder == 0);
        // Negative: idiv truncates, so -7/2 is -3 remainder -1, and the +1
        // takes it to -2 - away from zero in magnitude terms, and the
        // remainder keeps the DIVIDEND's sign.
        expect(leaf_00532a50_redirect(-7, &quotient, &remainder, 2) == -2);
        expect(quotient == -2);
        expect(remainder == -1);
        expect(leaf_00532a50_redirect(-8, &quotient, &remainder, 2) == -4);
        expect(remainder == 0);
    }

    // --- focus held by this window AND its parent ---
    {
        alignas(Win) uint8_t self_storage[sizeof(Win) + 32] = {};
        alignas(Win) uint8_t parent_storage[sizeof(Win) + 32] = {};
        uint8_t *const me = self_storage + 16;
        uint8_t *const up = parent_storage + 16;
        auto focus = [&](uint8_t *where, bool held) {
            // Bit 12 of field_98_ short-circuits Win::is_dialog_focus to 1
            // before it ever consults a parent, which keeps this fixture
            // about THIS function rather than about the focus list.
            const uint32_t flags = held ? 0x1000U : 0U;
            std::memcpy(where + 0x98, &flags, sizeof(flags));
        };
        auto set_parent = [&](uint8_t *where, void *parent) {
            std::memcpy(where + 0xC4, &parent, sizeof(parent));
        };

        // No focus here: the parent is never consulted.
        focus(me, false); set_parent(me, nullptr);
        expect(leaf_006161a0_redirect(me, nullptr) == 0);
        focus(up, true); set_parent(up, nullptr);
        set_parent(me, up);
        expect(leaf_006161a0_redirect(me, nullptr) == 0);

        // Focus here and no parent: yes. A null parent must not read as a
        // failure, which is the easiest thing to get backwards.
        focus(me, true); set_parent(me, nullptr);
        expect(leaf_006161a0_redirect(me, nullptr) == 1);

        // Focus here and a parent that also holds it: yes.
        focus(me, true); set_parent(me, up); focus(up, true);
        set_parent(up, nullptr);
        expect(leaf_006161a0_redirect(me, nullptr) == 1);

        // Focus here, parent without it: no.
        focus(up, false); set_parent(up, nullptr);
        expect(leaf_006161a0_redirect(me, nullptr) == 0);
    }

    // --- a Buffer built and destroyed on one piece of storage ---
    //
    // Nothing this function does is observable from outside its own stack
    // frame, so the assertion is the only thing there is to assert: both
    // calls happened, on the SAME pointer, constructor first. The `lea` is
    // issued twice for one address in the original, and two different buffers
    // would read just as naturally.
    {
        func_leaf_buffer_ctor *const saved_ctor = LeafBufferConstruct;
        func_leaf_buffer_dtor *const saved_dtor = LeafBufferDestruct;
        LeafBufferConstruct = &leaf_buffer_ctor_probe;
        LeafBufferDestruct = &leaf_buffer_dtor_probe;

        g_leaf_dtor_count = 0;
        leaf_00455e50_redirect();
        expect(g_leaf_dtor_count == 2);
        expect(g_leaf_dtor_calls[0].which == 5);     // construct first
        expect(g_leaf_dtor_calls[1].which == 6);     // then destroy
        expect(g_leaf_dtor_calls[0].target == g_leaf_dtor_calls[1].target);
        expect(g_leaf_dtor_calls[0].target != nullptr);

        LeafBufferConstruct = saved_ctor;
        LeafBufferDestruct = saved_dtor;
    }

    // --- two destructor chains: three calls, three pointers, one order ---
    {
        func_leaf_adjusted_dtor *const saved_lb = LeafListBoxDestructor;
        func_leaf_adjusted_dtor *const saved_ds = LeafDialogsDestructor;
        func_leaf_dialog_dtor *const saved_dlg = LeafDialogDestructor;
        func_leaf_graphic_dtor *const saved_gw = LeafGraphicWinDestructor;
        LeafListBoxDestructor = &leaf_dtor_listbox;
        LeafDialogsDestructor = &leaf_dtor_dialogs;
        LeafDialogDestructor = &leaf_dtor_dialog;
        LeafGraphicWinDestructor = &leaf_dtor_graphic;

        static uint8_t object[0x2000];
        std::memset(object, 0x11, sizeof(object));

        // 004080B0: ListBox at 0x48, Dialog at 0xa60, GraphicWin at 0x48.
        g_leaf_dtor_count = 0;
        leaf_004080b0_redirect(object, nullptr);
        expect(g_leaf_dtor_count == 3);
        expect(g_leaf_dtor_calls[0].which == 1);            // ListBox first
        expect(g_leaf_dtor_calls[0].target == object + 0x48);
        expect(g_leaf_dtor_calls[1].which == 3);            // then the Dialog
        expect(g_leaf_dtor_calls[1].target == object + 0xA60);
        expect(g_leaf_dtor_calls[2].which == 4);            // then GraphicWin
        // The SAME pointer the ListBox got: EDI is computed once, used twice.
        expect(g_leaf_dtor_calls[2].target == object + 0x48);
        expect(g_leaf_dtor_calls[2].target == g_leaf_dtor_calls[0].target);
        // And never the outer object, which is what a body passing `this`
        // straight to GraphicWin would show.
        expect(g_leaf_dtor_calls[2].target != object);

        // 00406AF0: Dialogs at 0x188, Dialog at 0xba0, GraphicWin at 0x188.
        g_leaf_dtor_count = 0;
        leaf_00406af0_redirect(object, nullptr);
        expect(g_leaf_dtor_count == 3);
        expect(g_leaf_dtor_calls[0].which == 2);            // Dialogs, not ListBox
        expect(g_leaf_dtor_calls[0].target == object + 0x188);
        expect(g_leaf_dtor_calls[1].which == 3);
        expect(g_leaf_dtor_calls[1].target == object + 0xBA0);
        expect(g_leaf_dtor_calls[2].which == 4);
        expect(g_leaf_dtor_calls[2].target == object + 0x188);
        expect(g_leaf_dtor_calls[2].target == g_leaf_dtor_calls[0].target);
        expect(g_leaf_dtor_calls[2].target != object);
        // Neither chain uses the other's offsets.
        expect(g_leaf_dtor_calls[0].target != object + 0x48);
        expect(g_leaf_dtor_calls[1].target != object + 0xA60);

        LeafListBoxDestructor = saved_lb;
        LeafDialogsDestructor = saved_ds;
        LeafDialogDestructor = saved_dlg;
        LeafGraphicWinDestructor = saved_gw;
    }

    // --- one fixed item into the Dialogs at 0x21d0 ---
    {
        func_dialog_item *const saved_item = DialogOriginalItem;
        char *const saved_text = DialogsItemText6900C4;
        char bound[] = "bound text";
        DialogOriginalItem = &leaf_stub_dialog_item;
        DialogsItemText6900C4 = bound;

        static uint8_t outer[0x21D0 + 0x1000];
        std::memset(outer, 0, sizeof(outer));
        uint8_t *const dialogs = outer + 0x21D0;
        const int32_t kind = 1;                 // routes to the Dialog arm
        std::memcpy(dialogs + 0x180, &kind, sizeof(kind));
        // Dialogs::item reaches its Dialog through the object's OWN vbtable -
        // slot 2 holds the displacement - so the fixture has to publish one.
        // A zeroed object faults reading [0+8] instead.
        static const int32_t vbtable[3] = {0, 0, 0xC40};
        const int32_t *const vbtable_ptr = vbtable;
        std::memcpy(dialogs, &vbtable_ptr, sizeof(vbtable_ptr));

        g_leaf_item_calls = 0;
        expect(leaf_0059d230_redirect(outer, nullptr) == 0x1234ABCD);
        expect(g_leaf_item_calls == 1);
        // The Dialogs is at 0x21d0, so the Dialog it forwards to is
        // 0x21d0 + 0xc40 from the outer object - not 0xc40.
        expect(reinterpret_cast<uint8_t *>(g_leaf_item_dialog)
               == outer + 0x21D0 + 0xC40);
        // The text is the bound pointer, and the index is NEGATIVE TWO.
        expect(g_leaf_item_text == bound);
        expect(g_leaf_item_index == -2);

        DialogOriginalItem = saved_item;
        DialogsItemText6900C4 = saved_text;
    }

    // --- four field resets in two pairs, then close as a GraphicWin ---
    {
        func_subobject_close *const saved_win = WinOriginalClose;
        func_subobject_close *const saved_sub = BufferSubobjectClose;
        uint32_t *const saved_default = GraphicWinFieldA0CDefault;
        uint32_t default_a0c = 0x0C0FFEE0U;
        WinOriginalClose = nullptr;
        BufferSubobjectClose = nullptr;
        GraphicWinFieldA0CDefault = &default_a0c;

        static uint8_t object[0xB00];
        auto got = [&](size_t offset) {
            uint32_t value;
            std::memcpy(&value, object + offset, sizeof(value));
            return value;
        };
        std::memset(object, 0x11, sizeof(object));
        write_at(object, 0xA08, 0U);            // keep the release path out

        pick_tech_close_redirect(object, nullptr);

        expect(got(0xA24) == 0U);
        expect(got(0xA28) == 0U);
        expect(got(0xA38) == 0U);
        expect(got(0xA3C) == 0U);
        // The gap between the two pairs is NOT written.
        expect(got(0xA2C) == 0x11111111U);
        expect(got(0xA30) == 0x11111111U);
        expect(got(0xA34) == 0x11111111U);
        // GraphicWin::close ran: only it writes the 0xA0C default.
        expect(got(0xA0C) == default_a0c);

        WinOriginalClose = saved_win;
        BufferSubobjectClose = saved_sub;
        GraphicWinFieldA0CDefault = saved_default;
    }

    // --- big-endian 16-bit append with a two-step cursor ---
    {
        static uint8_t descriptor[0x20];
        static uint8_t buffer[16];
        std::memset(descriptor, 0, sizeof(descriptor));
        std::memset(buffer, 0x11, sizeof(buffer));
        uint8_t *const base = buffer;
        std::memcpy(descriptor + 0x8, &base, sizeof(base));
        const uint32_t start = 0;
        std::memcpy(descriptor + 0x10, &start, sizeof(start));

        leaf_0063d420_redirect(descriptor, 0xABCD);
        // HIGH byte first: a little-endian body would give CD then AB.
        expect(buffer[0] == 0xAB);
        expect(buffer[1] == 0xCD);
        expect(buffer[2] == 0x11);              // and only two bytes
        uint32_t cursor = 0;
        std::memcpy(&cursor, descriptor + 0x10, sizeof(cursor));
        expect(cursor == 2);                    // advanced twice

        // Appending again continues where it left off rather than restarting.
        leaf_0063d420_redirect(descriptor, 0x1234);
        expect(buffer[2] == 0x12);
        expect(buffer[3] == 0x34);
        std::memcpy(&cursor, descriptor + 0x10, sizeof(cursor));
        expect(cursor == 4);

        // Only the low 16 bits are written.
        leaf_0063d420_redirect(descriptor, 0xFFFF5678U);
        expect(buffer[4] == 0x56);
        expect(buffer[5] == 0x78);

        // The cursor is stored BEFORE the second byte is written, and that
        // order is only visible when the byte lands on the cursor itself.
        // Point the stream at its own descriptor with the cursor at 0x0f: the
        // first byte goes to 0x0f, the cursor advances to 0x10, and the
        // second byte then overwrites the cursor's own low byte.
        //
        //   this order  -> cursor field becomes 0xcd, final cursor 0xce
        //   swapped     -> the 0x10 store lands last, final cursor 0x11
        std::memset(descriptor, 0, sizeof(descriptor));
        uint8_t *const self_base = descriptor;
        std::memcpy(descriptor + 0x8, &self_base, sizeof(self_base));
        const uint32_t near_cursor = 0x0F;
        std::memcpy(descriptor + 0x10, &near_cursor, sizeof(near_cursor));

        leaf_0063d420_redirect(descriptor, 0xABCD);
        expect(descriptor[0x0F] == 0xAB);
        std::memcpy(&cursor, descriptor + 0x10, sizeof(cursor));
        expect(cursor == 0xCE);
    }

    // --- find a player's name by key ---
    {
        static uint8_t net_object[0x780];
        std::memset(net_object, 0, sizeof(net_object));
        auto key_at = [&](uint32_t index, uint32_t key) {
            std::memcpy(net_object + 0x154 + index * 0x58, &key, sizeof(key));
        };
        auto *const net = reinterpret_cast<Net *>(net_object);

        // Nothing matches 7 yet.
        expect(net_get_player_name_redirect(net, nullptr, 7) == nullptr);

        // The name is 0x169, twenty-one bytes into the 0x58-byte entry - not
        // at its head, which is where the key lives.
        key_at(0, 7);
        expect(net_get_player_name_redirect(net, nullptr, 7) ==
               reinterpret_cast<char *>(net_object + 0x169));
        key_at(0, 0);
        key_at(3, 7);
        expect(net_get_player_name_redirect(net, nullptr, 7) ==
               reinterpret_cast<char *>(net_object + 0x169 + 3 * 0x58));
        // The last entry is reachable, and the one past it is not.
        key_at(3, 0);
        key_at(0xF, 7);
        expect(net_get_player_name_redirect(net, nullptr, 7) ==
               reinterpret_cast<char *>(net_object + 0x169 + 0xF * 0x58));
        key_at(0xF, 0);
        std::memcpy(net_object + 0x154 + 0x10 * 0x58, "\x07\x00\x00\x00", 4);
        expect(net_get_player_name_redirect(net, nullptr, 7) == nullptr);

        // Key 0 is a real key, not a sentinel: every entry holds it now, and
        // the first must win.
        expect(net_get_player_name_redirect(net, nullptr, 0) ==
               reinterpret_cast<char *>(net_object + 0x169));
    }

    // --- the four Ambience constructors ---
    //
    // They share a shape and differ only in the vtable and in how far the
    // tail runs, so they are checked against each other as much as against
    // themselves: each is seeded identically, and the assertions name the
    // exact last field it writes and the first it does NOT.
    {
        struct Variant {
            void *(*run)(void *);
            uint32_t vtable;
            size_t last_written;   // inclusive, dword offsets past 0x68
        };
        // Ambience::construct reaches Sound::set_type through a rebindable
        // seam that still points at the original image, which is not mapped
        // here - so it is bound to a no-op for the duration. Without this the
        // suite faults executing 0x004c61e0 rather than failing an assertion.
        func_sound_set_type *const saved_set_type = SoundSetType;
        SoundSetType = &leaf_stub_sound_set_type;

        // 0 means "the tail stops at 0x68".
        const Variant variants[] = {
            {[](void *o) -> void * {
                 return faction_ambience_construct_redirect(
                     reinterpret_cast<FactionAmbience *>(o), nullptr); },
             FactionAmbienceVtable, 0},
            {[](void *o) -> void * {
                 return m_ambience_construct_redirect(
                     reinterpret_cast<MAmbience *>(o), nullptr); },
             MAmbienceVtable, 0x70},
            {[](void *o) -> void * {
                 return s_ambience_construct_redirect(
                     reinterpret_cast<SAmbience *>(o), nullptr); },
             SAmbienceVtable, 0x74},
            {[](void *o) -> void * {
                 return g_ambience_construct_redirect(
                     reinterpret_cast<GAmbience *>(o), nullptr); },
             GAmbienceVtable, 0x70},
        };
        for (const Variant &variant : variants) {
            static uint8_t object[0x100];
            // The base region is ZEROED and only the derived fields seeded:
            // Ambience::construct runs real work over [0, 0x58) and is not
            // safe on garbage, while its own documentation establishes it
            // writes nothing at or above 0x58 - which is exactly the boundary
            // these four constructors start at.
            std::memset(object, 0, 0x58);
            std::memset(object + 0x58, 0x11, sizeof(object) - 0x58);
            auto got = [&](size_t offset) {
                uint32_t value;
                std::memcpy(&value, object + offset, sizeof(value));
                return value;
            };

            expect(variant.run(object) == object);

            // The variant's own vtable, published AFTER the base constructor
            // and therefore overwriting whatever it left.
            expect(got(0x0) == variant.vtable);
            // The base constructor ran. Its vtable is gone - overwritten just
            // above - so it is pinned by the 1000ms default it puts at 0x38
            // instead. Without this, dropping the base call entirely passes:
            // everything else asserted here lives at 0x58 and beyond, which
            // the base never touches.
            expect(got(0x38) == 0x3E8U);
            expect(got(0x58) == 0U);
            expect(got(0x5C) == 0U);
            expect(got(0x60) == 0U);
            expect(got(0x64) == 0U);
            expect(got(0x68) == 0U);
            // 0x6c and 0x6d are BYTES; 0x6e and 0x6f keep the seed, which a
            // dword store at 0x6c would have wiped.
            expect(object[0x6C] == 0x00);
            expect(object[0x6D] == 0x00);
            expect(object[0x6E] == 0x11);
            expect(object[0x6F] == 0x11);
            // The tail, and the first dword past it.
            if (variant.last_written >= 0x70) {
                expect(got(0x70) == 0U);
            } else {
                expect(got(0x70) == 0x11111111U);
            }
            if (variant.last_written >= 0x74) {
                expect(got(0x74) == 0U);
            } else {
                expect(got(0x74) == 0x11111111U);
            }
            expect(got(0x78) == 0x11111111U);
        }
        SoundSetType = saved_set_type;

        // All four vtables differ; a copy-paste between them would show here.
        expect(FactionAmbienceVtable != MAmbienceVtable);
        expect(MAmbienceVtable != SAmbienceVtable);
        expect(SAmbienceVtable != GAmbienceVtable);
        expect(FactionAmbienceVtable != GAmbienceVtable);
    }

    // --- six field resets, then GraphicWin::close and Buffer::close ---
    //
    // The callees are made harmless rather than mocked away: GraphicWin's two
    // subobject-close seams are null-checked, so nulling them takes those
    // paths out, and a zeroed release target makes it return before the
    // virtual call. Buffer::close is left to run for real.
    //
    // This does NOT compare the object byte for byte. Buffer::close writes
    // some forty fields unconditionally, and restating them here would just be
    // the recovery written twice. Instead each callee is pinned by one value
    // only it produces, AT THE ADDRESS IT SHOULD HAVE BEEN GIVEN.
    {
        func_subobject_close *const saved_win = WinOriginalClose;
        func_subobject_close *const saved_sub = BufferSubobjectClose;
        uint32_t *const saved_default = GraphicWinFieldA0CDefault;
        uint32_t *const saved_reset = BufferResetValue520;
        Font **const saved_font = FontDefaultPtr;
        uint32_t default_a0c = 0x0BADF00DU;
        uint32_t reset_520 = 0x71234567U;
        Font *default_font = reinterpret_cast<Font *>(0x76543210U);
        WinOriginalClose = nullptr;
        BufferSubobjectClose = nullptr;
        GraphicWinFieldA0CDefault = &default_a0c;
        BufferResetValue520 = &reset_520;
        FontDefaultPtr = &default_font;

        static uint8_t object[0x4700];
        auto got = [&](size_t offset) {
            uint32_t value;
            std::memcpy(&value, object + offset, sizeof(value));
            return value;
        };

        std::memset(object, 0, sizeof(object));
        leaf_00432970_redirect(object, nullptr);

        // The six stores, one of which is -1 and is written fifth.
        expect(got(0x2B60) == 0U);
        expect(got(0x2B6C) == 0U);
        expect(got(0x2B70) == 0U);
        expect(got(0x2B80) == 0U);
        expect(got(0x2B68) == 0xFFFFFFFFU);
        expect(got(0x4648) == 0U);

        // GraphicWin::close ran on `this`: only it writes the 0xA0C default.
        expect(got(0xA0C) == default_a0c);
        // Buffer::close ran AT 0x406c: only it writes -1 at its own 0x50c and
        // the reset value at its 0x520.
        expect(got(0x406C + 0x50C) == 0xFFFFFFFFU);
        expect(got(0x406C + 0x520) == reset_520);
        // ...and NOT on `this`, which is what a body passing the wrong
        // pointer would show. `this + 0x50c` is untouched by everything else.
        expect(got(0x50C) == 0U);
        expect(got(0x520) == 0U);

        // Seeded non-zero, so the five cleared fields must actually be
        // cleared rather than merely left alone.
        std::memset(object, 0x11, sizeof(object));
        std::memset(object + 0x406C, 0, 0x600);   // keep Buffer::close tame
        write_at(object, 0xA08, 0U);              // and its release path out
        // 0x4648 falls INSIDE the region just zeroed, so it has to be dirtied
        // again - otherwise "cleared" and "never written" look identical and
        // dropping that store passes.
        write_at(object, 0x4648, 0x11111111U);
        leaf_00432970_redirect(object, nullptr);
        expect(got(0x2B60) == 0U);
        expect(got(0x2B6C) == 0U);
        expect(got(0x2B70) == 0U);
        expect(got(0x2B80) == 0U);
        expect(got(0x2B68) == 0xFFFFFFFFU);
        expect(got(0x4648) == 0U);

        WinOriginalClose = saved_win;
        BufferSubobjectClose = saved_sub;
        GraphicWinFieldA0CDefault = saved_default;
        BufferResetValue520 = saved_reset;
        FontDefaultPtr = saved_font;
    }

    // --- construct a Buffer subobject at 0x8dc, then clear 0x10c ---
    //
    // Reuses the Buffer-constructor globals and the expected-bytes helper the
    // Buffer test already stands up, so this checks the WHOLE object byte for
    // byte: that the constructor ran at 0x8dc and nowhere else, that 0x10c was
    // cleared, and that nothing between them moved.
    {
        uint32_t reset_520 = 0x71234567U;
        Font *default_font = reinterpret_cast<Font *>(0x76543210U);
        Palette *palette_value = nullptr;
        int palette_state = 0;
        uint32_t *const saved_reset = BufferResetValue520;
        Font **const saved_font = FontDefaultPtr;
        Palette **const saved_palette = BufferPalette;
        int *const saved_initialized = PaletteInitialized;
        BufferResetValue520 = &reset_520;
        FontDefaultPtr = &default_font;
        BufferPalette = &palette_value;
        PaletteInitialized = &palette_state;

        static uint8_t storage[0x8DC + 0x600 + 32];
        static uint8_t expected[sizeof(storage)];
        seed_storage(storage, expected, sizeof(storage));
        write_buffer_construct_expected(
            expected, 16 + 0x8DC, reset_520,
            reinterpret_cast<uintptr_t>(default_font), nullptr, 0);
        write_at(expected, 16 + 0x10C, 0U);

        void *const self = storage + 16;
        expect(leaf_004bea30_redirect(self, nullptr) == self);
        expect_storage_bytes(storage, expected, sizeof(storage));

        BufferResetValue520 = saved_reset;
        FontDefaultPtr = saved_font;
        BufferPalette = saved_palette;
        PaletteInitialized = saved_initialized;
    }

    // --- 24 slots of 60 bytes, in three different widths ---
    {
        static uint8_t object[0x5A0 + 4 + 32];
        std::memset(object, 0x11, sizeof(object));
        expect(leaf_0052dca0_redirect(object, nullptr) == object);

        for (int index = 0; index < 0x18; ++index) {
            uint8_t *const slot = object + index * 0x3C;
            expect(slot[0] == 0xFF);
            // ONE byte at +0: the next three are untouched, which is what
            // separates a byte store from a dword one.
            expect(slot[1] == 0x11);
            expect(slot[2] == 0x11);
            expect(slot[3] == 0x11);
            // TWO bytes at +8, not four.
            expect(slot[8] == 0x00);
            expect(slot[9] == 0x00);
            expect(slot[10] == 0x11);
            expect(slot[11] == 0x11);
        }
        // The dword immediately past the table.
        uint32_t tail = 0xFFFFFFFFU;
        std::memcpy(&tail, object + 0x5A0, sizeof(tail));
        expect(tail == 0);
        // And nothing beyond it.
        for (size_t offset = 0x5A0 + 4; offset < sizeof(object); ++offset) {
            expect(object[offset] == 0x11);
        }
        // 24 slots exactly: the 25th would start at 0x5a0, which is the
        // count, so a miscounted loop would have overwritten it with 0xff.
    }

    // --- movie playing, normalised to 0 or 1 ---
    {
        static uint8_t player[0xA20];
        std::memset(player, 0, sizeof(player));
        auto set = [&](uint32_t value) {
            std::memcpy(player + 0xA14, &value, sizeof(value));
        };
        set(0);
        expect(leaf_004041c0_redirect(player, nullptr) == 0);
        set(1);
        expect(leaf_004041c0_redirect(player, nullptr) == 1);
        // Only bit 0 counts. The original ALSO normalises with
        // `neg / sbb / neg`, and that step is not observable here: the
        // recovered MCIVideo::is_playing already masks with 1, so it can only
        // return 0 or 1 and there is nothing left to normalise. Dropping the
        // normalisation passes this fixture, which is how I know rather than
        // assume.
        set(2);
        expect(leaf_004041c0_redirect(player, nullptr) == 0);
        set(3);
        expect(leaf_004041c0_redirect(player, nullptr) == 1);
        set(0xFFFFFFFFU);
        expect(leaf_004041c0_redirect(player, nullptr) == 1);
        set(0xFFFFFFFEU);
        expect(leaf_004041c0_redirect(player, nullptr) == 0);
    }

    // --- point in an inclusive rectangle, arguments in the odd order ---
    {
        // x, y, left, top, right, bottom.
        expect(leaf_00592db0_redirect(5, 5, 0, 0, 10, 10) == 1);
        // Every edge is INSIDE.
        expect(leaf_00592db0_redirect(0, 5, 0, 0, 10, 10) == 1);
        expect(leaf_00592db0_redirect(10, 5, 0, 0, 10, 10) == 1);
        expect(leaf_00592db0_redirect(5, 0, 0, 0, 10, 10) == 1);
        expect(leaf_00592db0_redirect(5, 10, 0, 0, 10, 10) == 1);
        // One step past each edge, so a swapped bound would show.
        expect(leaf_00592db0_redirect(-1, 5, 0, 0, 10, 10) == 0);
        expect(leaf_00592db0_redirect(11, 5, 0, 0, 10, 10) == 0);
        expect(leaf_00592db0_redirect(5, -1, 0, 0, 10, 10) == 0);
        expect(leaf_00592db0_redirect(5, 11, 0, 0, 10, 10) == 0);
        // A rectangle that is not square, so x and y cannot be interchanged.
        expect(leaf_00592db0_redirect(7, 2, 0, 0, 10, 3) == 1);
        expect(leaf_00592db0_redirect(2, 7, 0, 0, 10, 3) == 0);
        // Signed: negative bounds work, and a big positive is not "negative".
        expect(leaf_00592db0_redirect(-5, -5, -10, -10, -1, -1) == 1);
    }

    // --- the 512-entry table: search, set, and the odd return values ---
    {
        static uint8_t pool[0xA20 + 0x200 * 12 + 32];
        std::memset(pool, 0x11, sizeof(pool));
        leaf_005ad450_redirect(pool, nullptr);           // fill with -1
        int32_t *const table = reinterpret_cast<int32_t *>(pool + 0xA20);

        // A miss returns 512, which looks like an index and is not one.
        expect(leaf_005ad4c0_redirect(pool, nullptr, 7, 9, 42) == 0x200);

        // Plant a key in entry 3 and find it: the answer is 3 * 3, not 3.
        table[3 * 3] = 7;
        table[3 * 3 + 1] = 9;
        expect(leaf_005ad4c0_redirect(pool, nullptr, 7, 9, 42) == 9);
        expect(table[3 * 3 + 2] == 42);                  // word 2 written
        expect(table[3 * 3] == 7);                       // words 0,1 untouched
        expect(table[3 * 3 + 1] == 9);

        // BOTH keys must match, not just the first.
        table[5 * 3] = 7;
        table[5 * 3 + 1] = 99;
        expect(leaf_005ad4c0_redirect(pool, nullptr, 7, 100, 1) == 0x200);

        // The first match wins, and entry 0 returns 0 - not the miss value.
        table[0] = 1; table[1] = 2;
        table[6 * 3] = 1; table[6 * 3 + 1] = 2;
        expect(leaf_005ad4c0_redirect(pool, nullptr, 1, 2, 55) == 0);
        expect(table[2] == 55);
        expect(table[6 * 3 + 2] == -1);                  // the later one is not

        // The last entry is reachable.
        table[0x1FF * 3] = 31; table[0x1FF * 3 + 1] = 41;
        expect(leaf_005ad4c0_redirect(pool, nullptr, 31, 41, 3) == 0x1FF * 3);

        // And the one PAST the last is not. `index < 0x200` written as `<=`
        // searches 513 entries and reads two words beyond the table; planting
        // a key there and requiring a MISS is what separates the two.
        table[0x200 * 3] = 61;
        table[0x200 * 3 + 1] = 71;
        const int32_t beyond = table[0x200 * 3 + 2];
        expect(leaf_005ad4c0_redirect(pool, nullptr, 61, 71, 9) == 0x200);
        expect(table[0x200 * 3 + 2] == beyond);   // nothing written past it
    }

    // --- two independent swaps, with guards on two different objects ---
    {
        uint8_t holder[8] = {};
        uint8_t node[0x60] = {};
        uint8_t *pointer = node;
        std::memcpy(holder, &pointer, sizeof(pointer));
        auto put = [&](uint8_t *where, size_t offset, uint32_t value) {
            std::memcpy(where + offset, &value, sizeof(value));
        };
        auto got = [&](const uint8_t *where, size_t offset) {
            uint32_t value;
            std::memcpy(&value, where + offset, sizeof(value));
            return value;
        };

        // First swap: needs node[0x20] non-zero AND holder[0x4c] == 1. The
        // second condition is on the OUTER object, so a body reading it from
        // the node would take the branch when it should not.
        auto reset = [&]() {
            std::memset(node, 0, sizeof(node));
            std::memset(holder + 4, 0, 4);
            put(node, 0x1C, 0xAAAA); put(node, 0x20, 0xBBBB);
            put(node, 0x4C, 0xCCCC); put(node, 0x50, 0xDDDD);
        };

        reset();                                   // gate clear -> no swap
        leaf_005cbbc0_redirect(holder, nullptr);
        expect(got(node, 0x1C) == 0xAAAA);
        expect(got(node, 0x20) == 0xBBBB);

        reset();
        uint8_t outer[0x60] = {};
        std::memcpy(outer, &pointer, sizeof(pointer));
        put(outer, 0x4C, 1);                       // gate set -> swap
        leaf_005cbbc0_redirect(outer, nullptr);
        expect(got(node, 0x1C) == 0xBBBB);
        expect(got(node, 0x20) == 0xAAAA);

        reset();
        std::memcpy(outer, &pointer, sizeof(pointer));
        put(outer, 0x4C, 1);
        put(node, 0x20, 0);                        // zero -> no swap
        leaf_005cbbc0_redirect(outer, nullptr);
        expect(got(node, 0x1C) == 0xAAAA);

        // Second swap: bit 11 of node[4], which is what `test dh,8` reads.
        reset();
        std::memcpy(outer, &pointer, sizeof(pointer));
        put(node, 0x4, 0x800);
        leaf_005cbbc0_redirect(outer, nullptr);
        expect(got(node, 0x4C) == 0xDDDD);
        expect(got(node, 0x50) == 0xCCCC);

        // The node pointer is RE-READ between the halves of each swap. That
        // only matters when a store lands on the slot itself, so here it is
        // arranged to: the node is placed 0x1c bytes below the holder, which
        // puts node+0x1c exactly on the slot. The first swap then rewrites
        // the slot, and the second half must use the NEW node.
        {
            static uint8_t arena[0x200] = {};
            uint8_t *const outer_alias = arena + 0x40;
            uint8_t *const first_node = outer_alias - 0x1C;   // node+0x1c == slot
            uint8_t *const second_node = arena + 0x140;

            std::memset(arena, 0, sizeof(arena));
            // slot (== node+0x1c) holds the first node.
            std::memcpy(outer_alias, &first_node, sizeof(first_node));
            // node[0x20] names the second node, so the swap moves it in.
            std::memcpy(first_node + 0x20, &second_node, sizeof(second_node));
            const uint32_t one = 1;
            std::memcpy(outer_alias + 0x4C, &one, sizeof(one));

            leaf_005cbbc0_redirect(outer_alias, nullptr);

            // The slot now names the second node...
            uint8_t *after = nullptr;
            std::memcpy(&after, outer_alias, sizeof(after));
            expect(after == second_node);
            // ...and the write-back went to the SECOND node, because the
            // pointer was re-read. Without the re-read it would have landed
            // on first_node+0x20, which is the slot's neighbour.
            uint8_t *written = nullptr;
            std::memcpy(&written, second_node + 0x20, sizeof(written));
            expect(written == first_node);
        }

        // The SECOND swap re-reads the node too, and needs its own aliasing
        // case: this time the store to node+0x50 must land on the slot, so
        // the node sits 0x50 bytes below it.
        {
            static uint8_t arena2[0x300] = {};
            uint8_t *const outer2 = arena2 + 0x100;
            uint8_t *const low_node = outer2 - 0x50;      // node+0x50 == slot
            uint8_t *const other = arena2 + 0x200;

            std::memset(arena2, 0, sizeof(arena2));
            std::memcpy(outer2, &low_node, sizeof(low_node));  // slot -> low_node
            const uint32_t flag = 0x800;
            std::memcpy(low_node + 0x4, &flag, sizeof(flag));  // arm swap two
            // node[0x20] is zero, so the FIRST swap stays out of the way.
            std::memcpy(low_node + 0x4C, &other, sizeof(other));

            leaf_005cbbc0_redirect(outer2, nullptr);

            // The store to node+0x50 rewrote the slot to `other`...
            uint8_t *moved_to = nullptr;
            std::memcpy(&moved_to, outer2, sizeof(moved_to));
            expect(moved_to == other);
            // ...and the write-back landed on `other`, not on low_node,
            // because the pointer was re-read.
            uint8_t *back = nullptr;
            std::memcpy(&back, other + 0x4C, sizeof(back));
            expect(back == low_node);
        }

        // Neighbouring bits must NOT trigger it.
        for (uint32_t bit : {0x400U, 0x1000U, 0x8U, 0x80U}) {
            reset();
            std::memcpy(outer, &pointer, sizeof(pointer));
            put(node, 0x4, bit);
            leaf_005cbbc0_redirect(outer, nullptr);
            expect(got(node, 0x4C) == 0xCCCC);
            expect(got(node, 0x50) == 0xDDDD);
        }
    }

    // --- round toward zero to a multiple ---
    //
    // The NEGATIVE cases are the assertion. `x / y * y` truncates toward zero,
    // so -7 rounded to a multiple of 3 is -6; a body using floor division -
    // which is what the same idea looks like written naively - gives -9.
    expect(leaf_00559210_redirect(7, 3) == 6);
    expect(leaf_00559210_redirect(-7, 3) == -6);
    expect(leaf_00559210_redirect(7, -3) == 6);
    expect(leaf_00559210_redirect(-7, -3) == -6);
    expect(leaf_00559210_redirect(9, 3) == 9);
    expect(leaf_00559210_redirect(0, 5) == 0);
    expect(leaf_00559210_redirect(2, 5) == 0);
}

void test_field_accessors() {
    // The sixteen bodies tools/generate_field_accessors.py emitted. Each is
    // one expression, so the fixture is a table: seed the object, call, and
    // require the exact value AND that the object moved only where the shape
    // says it should. The seed pattern is non-uniform, so a body reading the
    // wrong offset returns a different dword rather than an equal one.
    // 0xB00, not 0x600: sub_589750 writes the dword at 0xa34, which the
    // old size did not even contain - the store would have run past the end
    // of this array rather than being checked by it.
    uint8_t storage[0xB00 + 32];
    uint8_t expected[sizeof(storage)];
    auto seed = [&]() {
        // The `0x35 + i * 17` pattern used elsewhere here REPEATS every 512
        // bytes - 512 * 17 is a multiple of 256 - so the dword at 0 and the
        // dword at 0x200 are identical under it, and the reader at 0x200 could
        // have its offset mutated to 0 without any assertion noticing. The
        // sweep found exactly that. The extra per-page term breaks the period.
        for (size_t i = 0; i < sizeof(storage); ++i) {
            storage[i] = static_cast<uint8_t>(0x35 + i * 17 + (i >> 8) * 0x51);
        }
        std::memcpy(expected, storage, sizeof(storage));
    };
    auto at = [&](size_t offset) {
        uint32_t value;
        std::memcpy(&value, storage + 16 + offset, sizeof(value));
        return value;
    };
    void *const self = storage + 16;

    // --- readers: return the dword at an offset, write nothing ---
    struct Reader { uint32_t (__fastcall *fn)(void *, void *); size_t offset; };
    const Reader readers[] = {
        {&field_accessor_00448310_redirect, 0x48},
        {&field_accessor_00448320_redirect, 0x44},
        {&field_accessor_00448330_redirect, 0x50},
        {&field_accessor_004c75a0_redirect, 0x200},
    };
    for (const Reader &one : readers) {
        seed();
        expect(one.fn(self, nullptr) == at(one.offset));
        expect_storage_bytes(storage, expected, sizeof(storage));
    }

    // --- masked readers ---
    //
    // The mask is only observable when the seeded word HAS the bit, and only
    // the offset is observable when the other candidate word does not. A
    // uniform seed gave neither: bit 15 happened to be clear at 0x4AC, so
    // both `& 0x8000 -> & 0` and `0x4AC -> 0` survived the sweep. These write
    // the two words explicitly and in opposition.
    struct Masked {
        uint32_t (__fastcall *fn)(void *, void *);
        size_t offset; uint32_t mask;
    };
    const Masked masked[] = {
        {&field_accessor_00600320_redirect, 0x000, 1U},
        {&field_accessor_006363f0_redirect, 0x4AC, 0x8000U},
    };
    for (const Masked &one : masked) {
        for (int bit_set = 0; bit_set < 2; ++bit_set) {
            seed();
            // The word under test carries the bit or not; every other word the
            // mutation could reach instead carries the OPPOSITE, so reading
            // the wrong offset gives the wrong answer.
            const uint32_t here = bit_set ? one.mask : 0U;
            const uint32_t elsewhere = bit_set ? 0U : one.mask;
            for (const Masked &other : masked) {
                const uint32_t value = (other.offset == one.offset)
                    ? here : elsewhere;
                std::memcpy(storage + 16 + other.offset, &value, sizeof(value));
                std::memcpy(expected + 16 + other.offset, &value, sizeof(value));
            }
            expect(one.fn(self, nullptr) == here);
            expect_storage_bytes(storage, expected, sizeof(storage));
        }
    }

    // --- constants: same answer whatever the object holds ---
    struct Constant { uint32_t (__fastcall *fn)(void *, void *); uint32_t value; };
    const Constant constants[] = {
        {&field_accessor_00406840_redirect, 1},
        {&field_accessor_0062d390_redirect, 1},
    };
    for (const Constant &one : constants) {
        seed();
        expect(one.fn(self, nullptr) == one.value);
        expect_storage_bytes(storage, expected, sizeof(storage));
    }

    // The five whose originals CLEAN STACK ARGUMENTS are called with that many
    // dwords, and cannot share the table above: their signatures differ, and
    // that is the point. __fastcall leaves the callee to clean everything past
    // the two register parameters, so an adapter declaring only (void *,
    // void *) compiles to a bare `ret` and would leave 4, 12, 16 or 20 bytes
    // on the caller's stack at every call - which is what the first version of
    // these emitted. The compiler refusing to put them in one table is the
    // check that the arities really do differ.
    seed();
    expect(field_accessor_004c93e0_redirect(self, nullptr, 0) == 0xB);
    expect(field_accessor_005da6a0_redirect(self, nullptr, 0, 0, 0) == 8);
    expect(field_accessor_005da6b0_redirect(self, nullptr, 0, 0, 0, 0) == 8);
    expect(field_accessor_005e2460_redirect(self, nullptr, 0, 0, 0, 0, 0) == 8);
    expect(field_accessor_005e2470_redirect(self, nullptr, 0, 0, 0, 0, 0) == 8);
    expect_storage_bytes(storage, expected, sizeof(storage));

    // --- increments: exactly one dword moves, by exactly one ---
    for (auto *fn : {&field_accessor_004476e0_redirect,
                     &field_accessor_00448280_redirect}) {
        seed();
        const uint32_t before = at(0x58);
        const uint32_t after = before + 1;
        std::memcpy(expected + 16 + 0x58, &after, sizeof(after));
        (*fn)(self, nullptr);
        expect_storage_bytes(storage, expected, sizeof(storage));
    }

    // --- store sequences: set N fields to constants ---
    //
    // Transcribed from the emitted bodies, so this catches a body that DRIFTS
    // from what was generated - a wrong offset, a dropped store, a changed
    // constant, a lost residue. It cannot catch the generator and the emitted
    // code sharing a misreading of the original, because both come from the
    // same decode; that is what the mutation sweep and the differential oracle
    // are for, and saying so beats implying otherwise.
    //
    // Split by return type rather than casting one function-pointer type to
    // the other: some of these carry the EAX = this residue and some do not,
    // and calling through the wrong prototype would be undefined even where it
    // happens to work.
    struct Store { size_t offset; uint32_t value; };
    struct ReturnsSelf { void *(__fastcall *fn)(void *, void *);
                         Store stores[16]; int count; };
    struct ReturnsVoid { void (__fastcall *fn)(void *, void *);
                         Store stores[16]; int count; };
    const ReturnsSelf returning_self[] = {
        {&field_accessor_004c8090_redirect, {{0x4, 0x0U}, {0x8, 0x0U}, {0xc, 0x0U}, {0x10, 0x0U}, {0x0, 0x0U}, {0x14, 0x0U}, {0x18, 0x0U}, {0x1c, 0x0U}, {0x20, 0xffffffffU}}, 9},
        {&field_accessor_004c8100_redirect, {{0x4, 0x0U}, {0x8, 0x0U}}, 2},
        {&field_accessor_005e35d0_redirect, {{0x0, 0x0U}, {0x4, 0x0U}, {0x8, 0x0U}, {0xc, 0x0U}, {0x10, 0x0U}, {0x14, 0x0U}, {0x18, 0x0U}, {0x1c, 0x0U}, {0x20, 0x0U}}, 9},
        {&field_accessor_00616b20_redirect, {{0x0, 0x0U}, {0x4, 0x0U}, {0x8, 0x0U}}, 3},
        {&field_accessor_006346c0_redirect, {{0x0, 0x3f800000U}, {0x4, 0x0U}, {0x8, 0x0U}, {0xc, 0x0U}, {0x10, 0x3f800000U}, {0x14, 0x0U}, {0x18, 0x0U}, {0x1c, 0x0U}, {0x20, 0x3f800000U}}, 9},
        {&field_accessor_00634b70_redirect, {{0x0, 0x3f800000U}, {0x4, 0x0U}, {0x8, 0x0U}, {0xc, 0x0U}, {0x10, 0x3f800000U}, {0x14, 0x0U}, {0x18, 0x0U}, {0x1c, 0x0U}, {0x20, 0x3f800000U}}, 9},
        {&field_accessor_00634fb0_redirect, {{0x0, 0x0U}, {0x8, 0x0U}, {0xc, 0x0U}, {0x4, 0x0U}}, 4},
        {&field_accessor_006354d0_redirect, {{0x0, 0x0U}, {0x4, 0x0U}, {0x8, 0x0U}, {0xc, 0x0U}, {0x10, 0x0U}, {0x18, 0x0U}, {0x1c, 0x0U}}, 7},
        {&field_accessor_0063e520_redirect, {{0x0, 0x0U}, {0x4, 0x0U}, {0x8, 0x0U}, {0xc, 0xffffffffU}}, 4},
    };
    const ReturnsVoid returning_void[] = {
        {&field_accessor_00590d60_redirect, {{0x200, 0x0U}}, 1},
        {&field_accessor_005e35f0_redirect, {{0x0, 0x0U}, {0x4, 0x0U}, {0x8, 0x0U}, {0xc, 0x0U}, {0x10, 0x0U}, {0x14, 0x0U}, {0x18, 0x0U}, {0x1c, 0x0U}, {0x20, 0x0U}}, 9},
        {&field_accessor_006346f0_redirect, {{0x0, 0x3f800000U}, {0x4, 0x0U}, {0x8, 0x0U}, {0xc, 0x0U}, {0x10, 0x3f800000U}, {0x14, 0x0U}, {0x18, 0x0U}, {0x1c, 0x0U}, {0x20, 0x3f800000U}}, 9},
    };
    for (const ReturnsSelf &one : returning_self) {
        seed();
        for (int i = 0; i < one.count; ++i) {
            std::memcpy(expected + 16 + one.stores[i].offset,
                        &one.stores[i].value, sizeof(uint32_t));
        }
        expect(one.fn(self, nullptr) == self);
        expect_storage_bytes(storage, expected, sizeof(storage));
    }
    for (const ReturnsVoid &one : returning_void) {
        seed();
        for (int i = 0; i < one.count; ++i) {
            std::memcpy(expected + 16 + one.stores[i].offset,
                        &one.stores[i].value, sizeof(uint32_t));
        }
        one.fn(self, nullptr);
        expect_storage_bytes(storage, expected, sizeof(storage));
    }

    // --- do-nothing bodies, and a constant zero ---
    //
    // Their ONLY content is the stack cleanup, which is invisible from inside
    // the function and only wrong for the caller - so these are called with
    // exactly the argument counts their originals clean, and the compiler
    // checking each call against a declaration derived from `ret N` is the
    // real assertion here.
    seed();
    field_accessor_00406b20_redirect(self, nullptr, 0, 0, 0);          // ret 0xc
    field_accessor_0061f785_redirect(self, nullptr, 0, 0, 0, 0, 0, 0, 0);  // ret 0x1c
    field_accessor_00634c20_redirect(self, nullptr);                   // ret
    field_accessor_0050f640_redirect(self, nullptr, 0);                // ret 0x4
    expect(field_accessor_00616d80_redirect(self, nullptr, 0, 0, 0) == 0);
    expect_storage_bytes(storage, expected, sizeof(storage));

    // --- parameter stores: the argument reaches the right field, in order ---
    //
    // Two DISTINCT values, because the order is the part that can go wrong
    // silently: sub_590cb0 reads [ebp+8] and [ebp+0xc], and a generator that
    // mixed up which slot is which would still write two plausible dwords into
    // two real fields, and every other check in this fixture would agree.
    {
        const uint32_t first = 0x11223344U;
        const uint32_t second = 0x55667788U;

        seed();
        std::memcpy(expected + 16 + 0xa34, &first, sizeof(first));
        field_accessor_00589750_redirect(self, nullptr, static_cast<int>(first));
        expect_storage_bytes(storage, expected, sizeof(storage));

        seed();
        std::memcpy(expected + 16 + 0x128, &first, sizeof(first));
        field_accessor_005f05c0_redirect(self, nullptr, static_cast<int>(first));
        expect_storage_bytes(storage, expected, sizeof(storage));

        seed();
        std::memcpy(expected + 16 + 0x0, &first, sizeof(first));
        std::memcpy(expected + 16 + 0x4, &second, sizeof(second));
        field_accessor_00590cb0_redirect(self, nullptr, static_cast<int>(first),
                                         static_cast<int>(second));
        expect_storage_bytes(storage, expected, sizeof(storage));
    }

    // --- store sequences with byte writes and interior pointers ---
    //
    // ??0FileBox is the reason the tracker follows OFFSETS rather than a set
    // of `this` aliases. It does `lea ecx,[eax+0x30c]` and then
    // `mov byte [ecx],dl`, so that store lands at 0x30c - but ECX is where
    // `this` arrives, and a tracker that just remembered "ECX aliases this"
    // would have recorded offset 0. Both are real fields and both take a zero
    // byte, so nothing but this assertion distinguishes them.
    {
        const size_t zero_bytes[] = {0x30d, 0x0, 0x30c, 0x104, 0x208,
                                     0x414, 0x418};
        uint8_t *const interior = static_cast<uint8_t *>(self) + 0x30c;

        seed();
        for (size_t offset : zero_bytes) {
            expected[16 + offset] = 0;
        }
        std::memcpy(expected + 16 + 0x410, &interior, sizeof(interior));
        expect(field_accessor_00634be0_redirect(self, nullptr) == self);
        expect_storage_bytes(storage, expected, sizeof(storage));

        seed();
        for (size_t offset : zero_bytes) {
            expected[16 + offset] = 0;
        }
        std::memcpy(expected + 16 + 0x410, &interior, sizeof(interior));
        field_accessor_00634f70_redirect(self, nullptr);
        expect_storage_bytes(storage, expected, sizeof(storage));

        // Heap::Heap2: one byte at 0, then four dwords. The byte write is the
        // point - `xor eax,eax / mov byte [ecx],al` sets ONE byte, and
        // treating it as a dword would clear three the original leaves alone.
        seed();
        const uint32_t zero = 0;
        expected[16 + 0x0] = 0;
        for (size_t offset : {0x8, 0x4, 0x10, 0xc}) {
            std::memcpy(expected + 16 + offset, &zero, sizeof(zero));
        }
        field_accessor_005d4540_redirect(self, nullptr);
        expect_storage_bytes(storage, expected, sizeof(storage));
    }

    // --- clamps: BOTH sides of the branch, or the test proves half a body ---
    //
    // The jump SKIPS the guarded store, so the store runs on the negation of
    // the jump condition. Getting that backwards inverts the clamp and still
    // compiles - and a fixture that only ever exercised the taken path would
    // agree with the inverted version. So each of these is called twice, once
    // either side of the field it compares against.
    {
        auto poke = [&](size_t offset, uint32_t value) {
            std::memcpy(storage + 16 + offset, &value, sizeof(value));
            std::memcpy(expected + 16 + offset, &value, sizeof(value));
        };
        auto expect_field = [&](size_t offset, uint32_t value) {
            std::memcpy(expected + 16 + offset, &value, sizeof(value));
        };

        // sub_4c80c0: if (this->[0xc] < arg) this->[0xc] = arg;
        //             this->[0x8] = arg;
        seed(); poke(0xc, 100);
        expect_field(0xc, 200); expect_field(0x8, 200);
        field_accessor_004c80c0_redirect(self, nullptr, 200);
        expect_storage_bytes(storage, expected, sizeof(storage));

        seed(); poke(0xc, 300);
        expect_field(0x8, 200);              // 0xc must NOT move
        field_accessor_004c80c0_redirect(self, nullptr, 200);
        expect_storage_bytes(storage, expected, sizeof(storage));

        // sub_4c80e0: if (this->[0x8] > arg) this->[0x8] = arg;
        //             this->[0xc] = arg;
        seed(); poke(0x8, 300);
        expect_field(0x8, 200); expect_field(0xc, 200);
        field_accessor_004c80e0_redirect(self, nullptr, 200);
        expect_storage_bytes(storage, expected, sizeof(storage));

        seed(); poke(0x8, 100);
        expect_field(0xc, 200);              // 0x8 must NOT move
        field_accessor_004c80e0_redirect(self, nullptr, 200);
        expect_storage_bytes(storage, expected, sizeof(storage));

        // The EQUALITY case. It is exercised for completeness, but it cannot
        // distinguish `<` from `<=` (or `>` from `>=`) and no test can: the
        // guarded store writes the very field being compared, with the very
        // value it is compared against. At equality the store is a no-op, so
        // both readings leave identical state. The mutation sweep reports
        // those two mutants as unobserved, and they are EQUIVALENT rather than
        // uncovered - which is why they are not chased.
        seed(); poke(0xc, 200);
        expect_field(0x8, 200);
        field_accessor_004c80c0_redirect(self, nullptr, 200);
        expect_storage_bytes(storage, expected, sizeof(storage));

        seed(); poke(0x8, 200);
        expect_field(0xc, 200);
        field_accessor_004c80e0_redirect(self, nullptr, 200);
        expect_storage_bytes(storage, expected, sizeof(storage));
    }

    // --- bit 0 smeared across the word: 0 or 0xffffffff ---
    //
    // Both parities, and a value whose OTHER bits are set: `shl 31 / sar 31`
    // discards everything except bit 0, so a body that masked with 0xff or
    // returned the field itself would agree on 0 and 1 and disagree here.
    {
        auto poke = [&](size_t offset, uint32_t value) {
            std::memcpy(storage + 16 + offset, &value, sizeof(value));
            std::memcpy(expected + 16 + offset, &value, sizeof(value));
        };
        seed(); poke(0x40, 0);
        expect(field_accessor_00448380_redirect(self, nullptr) == 0U);
        seed(); poke(0x40, 1);
        expect(field_accessor_00448380_redirect(self, nullptr) == 0xFFFFFFFFU);
        seed(); poke(0x40, 0xFFFFFFFEU);          // every bit but bit 0
        expect(field_accessor_00448380_redirect(self, nullptr) == 0U);
        seed(); poke(0x40, 0x12345679U);          // odd, with high bits set
        expect(field_accessor_00448380_redirect(self, nullptr) == 0xFFFFFFFFU);
        expect_storage_bytes(storage, expected, sizeof(storage));
    }

    // --- byte store: one BYTE moves, which the neighbours prove ---
    seed();
    expected[16 + 0x6D] = 1;
    field_accessor_00447ab0_redirect(self, nullptr);
    expect_storage_bytes(storage, expected, sizeof(storage));
}

}  // namespace

// Self-registration.  Each case carries its baseline position in the original
// main() call list, so the run order does not depend on link or static-init
// order.
namespace {
LEAF_CASE(57, test_scroll_close);
LEAF_CASE(61, test_scroll_init_wrappers);
LEAF_CASE(12, test_win_move);
LEAF_CASE(53, test_leaf_recoveries);
LEAF_CASE(52, test_field_accessors);
}  // namespace
