#include "../src/stdafx.h"
#include "../src/alphanet.h"
#include "../src/menu.h"
#include "../src/pulldown.h"
#include "../src/random.h"
#include "../src/scroll.h"
#include "../src/vector.h"
#include "../src/win.h"

#include <climits>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <float.h>

class LegacyAlphaNet {
 public:
    int pid_2_who(uint32_t) asm("_opensmacx_legacy_004E2610");
    int who_2_pid(int) asm("_opensmacx_legacy_004E2660");
    int who_2_idx(int) asm("_opensmacx_legacy_004E26B0");
};

class LegacyRandom {
 public:
    void reseed(uint32_t) asm("_opensmacx_legacy_00625750");
    uint32_t get(uint32_t, uint32_t) asm("_opensmacx_legacy_00625770");
};

class LegacyWin {
 public:
    int move(int, int) asm("_opensmacx_legacy_005ED7D0");
    void set_vert_paging(int) asm("_opensmacx_legacy_005EE0F0");
    void set_horz_paging(int) asm("_opensmacx_legacy_005EE110");
};

class LegacyScroll {
 public:
    void set_border_color(int) asm("_opensmacx_legacy_00605B10");
    Sprite *set_sprite_left(Sprite *, Sprite *, Sprite *)
        asm("_opensmacx_legacy_00605BE0");
    Sprite *set_sprite_right(Sprite *, Sprite *, Sprite *)
        asm("_opensmacx_legacy_00605C30");
    RECT *compute_thumb_rect(RECT *) asm("_opensmacx_legacy_00606C50");
};

RECT *__cdecl legacy_expand_rect(RECT *, int, int)
    asm("_opensmacx_legacy_00606F00");

class LegacyMenu {
 public:
    MenuProc set_menu_proc(MenuProc) asm("_opensmacx_legacy_005FB820");
    int id_to_index(int) asm("_opensmacx_legacy_005FB990");
};

class LegacyPullDown {
 public:
    int hide_item(int) asm("_opensmacx_legacy_005F8CB0");
    int show_item(int) asm("_opensmacx_legacy_005F8D20");
    int disable_item(int) asm("_opensmacx_legacy_005F8D90");
    int enable_item(int) asm("_opensmacx_legacy_005F8DF0");
    int check_item(int) asm("_opensmacx_legacy_005F9040");
    int uncheck_item(int) asm("_opensmacx_legacy_005F90A0");
    int get_selected() asm("_opensmacx_legacy_005F9F40");
};

RECT *__cdecl legacy_make_rect(RECT *, int, int, int, int)
    asm("_opensmacx_legacy_005F86C0");
int __cdecl legacy_in_box(int, int, int, int, int, int)
    asm("_opensmacx_legacy_005FA7A0");

class LegacyTutWin {
 public:
    int rect_center(RECT *, int *, int *) asm("_opensmacx_legacy_004BA830");
};

class LegacyVector {
 public:
    Vector *construct() asm("_opensmacx_legacy_006343C0");
    uintptr_t close() asm("_opensmacx_legacy_006343D0");
    Vector *subtract(LegacyVector &, LegacyVector &)
        asm("_opensmacx_legacy_00634430");
    Vector *add_assign(LegacyVector &) asm("_opensmacx_legacy_00634480");
    Vector *subtract_assign(LegacyVector &) asm("_opensmacx_legacy_006344B0");
    Vector *scale(LegacyVector &, uint32_t) asm("_opensmacx_legacy_00634670");
};

namespace {

constexpr size_t CanarySize = 16;
constexpr size_t FirstPidOffset = 0x928;
constexpr size_t IdentityOffset = 0x92C;
constexpr size_t SlotStride = 0x19C;

struct Fixture {
    alignas(AlphaNet) uint8_t storage[sizeof(AlphaNet) + CanarySize * 2];

    AlphaNet *network() {
        return reinterpret_cast<AlphaNet *>(storage + CanarySize);
    }

    LegacyAlphaNet *legacy_network() {
        return reinterpret_cast<LegacyAlphaNet *>(storage + CanarySize);
    }
};

struct RandomFixture {
    alignas(Random) uint8_t storage[sizeof(Random) + CanarySize * 2];

    Random *source() {
        return reinterpret_cast<Random *>(storage + CanarySize);
    }

    LegacyRandom *legacy() {
        return reinterpret_cast<LegacyRandom *>(storage + CanarySize);
    }
};

struct WinFixture {
    alignas(Win) uint8_t storage[sizeof(Win) + CanarySize * 2];

    Win *source() {
        return reinterpret_cast<Win *>(storage + CanarySize);
    }

    LegacyWin *legacy() {
        return reinterpret_cast<LegacyWin *>(storage + CanarySize);
    }
};

struct ScrollFixture {
    alignas(Scroll) uint8_t storage[sizeof(Scroll) + CanarySize * 2];

    Scroll *source() {
        return reinterpret_cast<Scroll *>(storage + CanarySize);
    }

    LegacyScroll *legacy() {
        return reinterpret_cast<LegacyScroll *>(storage + CanarySize);
    }
};

struct MenuFixture {
    alignas(Menu) uint8_t storage[sizeof(Menu) + CanarySize * 2];

    Menu *source() {
        return reinterpret_cast<Menu *>(storage + CanarySize);
    }

    LegacyMenu *legacy() {
        return reinterpret_cast<LegacyMenu *>(storage + CanarySize);
    }
};

struct PullDownFixture {
    alignas(PullDown) uint8_t storage[sizeof(PullDown) + CanarySize * 2];

    PullDown *source() {
        return reinterpret_cast<PullDown *>(storage + CanarySize);
    }

    LegacyPullDown *legacy() {
        return reinterpret_cast<LegacyPullDown *>(storage + CanarySize);
    }
};

enum class PullOperation {
    Hide,
    Show,
    Disable,
    Enable,
    Check,
    Uncheck,
};

struct ThumbOracleCase {
    const char *name;
    uint32_t flags;
    uint32_t border_color;
    uint32_t width;
    uint32_t stored_height;
    uint32_t minimum;
    uint32_t maximum;
    uint32_t position;
    uint32_t drag_coordinate;
    uint32_t rect[4];
};

void initialize_bytes(uint8_t *storage, size_t storage_size, size_t object_size) {
    std::memset(storage, 0xA5, storage_size);
    for (size_t offset = 0; offset < object_size; ++offset) {
        storage[CanarySize + offset] = static_cast<uint8_t>(0x35U + offset * 17U);
    }
}

void initialize(WinFixture &fixture) {
    initialize_bytes(fixture.storage, sizeof(fixture.storage), sizeof(Win));
}

void initialize(ScrollFixture &fixture) {
    initialize_bytes(fixture.storage, sizeof(fixture.storage), sizeof(Scroll));
}

void initialize(MenuFixture &fixture) {
    initialize_bytes(fixture.storage, sizeof(fixture.storage), sizeof(Menu));
}

void initialize(PullDownFixture &fixture) {
    initialize_bytes(fixture.storage, sizeof(fixture.storage), sizeof(PullDown));
}

void fill_menu_ids(MenuFixture &fixture) {
    for (int index = 0; index < 15; ++index) {
        const int id = 1000 + index;
        std::memcpy(fixture.storage + CanarySize + 0xA38 + index * 0x14,
                    &id, sizeof(id));
    }
}

void fill_pull_ids(PullDownFixture &fixture) {
    for (int index = 0; index < 64; ++index) {
        const int id = 1000 + index;
        std::memcpy(fixture.storage + CanarySize + 0xA20 + index * 0x14,
                    &id, sizeof(id));
    }
}

uint32_t pull_flags(PullOperation operation) {
    const uint32_t preserved = 0xA5A50004U;
    switch (operation) {
      case PullOperation::Hide:
        return preserved | 1U;
      case PullOperation::Enable:
        return preserved | 2U;
      case PullOperation::Uncheck:
        return preserved | 8U;
      case PullOperation::Show:
      case PullOperation::Disable:
      case PullOperation::Check:
        return preserved;
    }
    return preserved;
}

int call_source(PullDown *pull, PullOperation operation, int id) {
    switch (operation) {
      case PullOperation::Hide:
        return pull->hide_item(id);
      case PullOperation::Show:
        return pull->show_item(id);
      case PullOperation::Disable:
        return pull->disable_item(id);
      case PullOperation::Enable:
        return pull->enable_item(id);
      case PullOperation::Check:
        return pull->check_item(id);
      case PullOperation::Uncheck:
        return pull->uncheck_item(id);
    }
    return 0;
}

int call_legacy(LegacyPullDown *pull, PullOperation operation, int id) {
    switch (operation) {
      case PullOperation::Hide:
        return pull->hide_item(id);
      case PullOperation::Show:
        return pull->show_item(id);
      case PullOperation::Disable:
        return pull->disable_item(id);
      case PullOperation::Enable:
        return pull->enable_item(id);
      case PullOperation::Check:
        return pull->check_item(id);
      case PullOperation::Uncheck:
        return pull->uncheck_item(id);
    }
    return 0;
}

void __cdecl oracle_menu_proc(int) {}

template <typename T>
void write_object(uint8_t *storage, size_t offset, const T &value) {
    std::memcpy(storage + CanarySize + offset, &value, sizeof(value));
}

void initialize(ScrollFixture &fixture, const ThumbOracleCase &test) {
    initialize(fixture);
    write_object(fixture.storage, 0x4C4, test.width);
    write_object(fixture.storage, 0x4C8, test.stored_height);
    write_object(fixture.storage, 0xA14, test.flags);
    write_object(fixture.storage, 0xA1C, test.border_color);
    write_object(fixture.storage, 0xA20, test.minimum);
    write_object(fixture.storage, 0xA24, test.maximum);
    write_object(fixture.storage, 0xA2C, test.position);
    write_object(fixture.storage, 0xA3C, test.drag_coordinate);
    std::memcpy(fixture.storage + CanarySize + 0xA4C,
                test.rect, sizeof(test.rect));
}

int int_from_bits(uint32_t bits) {
    int value;
    static_assert(sizeof(value) == sizeof(bits), "oracle requires 32-bit int");
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}

bool canaries_intact(const uint8_t *storage, size_t object_size) {
    uint8_t canary[CanarySize];
    std::memset(canary, 0xA5, sizeof(canary));
    return std::memcmp(storage, canary, sizeof(canary)) == 0
        && std::memcmp(storage + CanarySize + object_size,
                       canary, sizeof(canary)) == 0;
}

void report_difference(const char *fixture, const char *area,
                       const uint8_t *source, const uint8_t *legacy,
                       size_t size) {
    for (size_t offset = 0; offset < size; ++offset) {
        if (source[offset] != legacy[offset]) {
            std::fprintf(stderr,
                "%s %s mismatch at 0x%zx: 0x%02x != 0x%02x\n",
                fixture, area, offset, source[offset], legacy[offset]);
            return;
        }
    }
}

void initialize(RandomFixture &fixture, uint32_t seed) {
    std::memset(fixture.storage, 0xA5, sizeof(fixture.storage));
    std::memcpy(fixture.storage + CanarySize, &seed, sizeof(seed));
}

bool canaries_intact(const RandomFixture &fixture) {
    uint8_t canary[CanarySize];
    std::memset(canary, 0xA5, sizeof(canary));
    return std::memcmp(fixture.storage, canary, CanarySize) == 0
        && std::memcmp(fixture.storage + CanarySize + sizeof(Random),
                       canary, CanarySize) == 0;
}

void initialize(Fixture &fixture) {
    std::memset(fixture.storage, 0xA5, sizeof(fixture.storage));
    const uint32_t process_ids[7] = {
        0U, 1U, 0x7FFFFFFFU, 0x80000000U, 0xFFFFFFFFU, 123U, 456U,
    };
    const int8_t identities[7] = {
        0, 1, 0x7F, static_cast<int8_t>(0x80),
        static_cast<int8_t>(0xFF), 42, -42,
    };
    const uint32_t lower_decoy_pid = 789U;
    const int8_t lower_decoy_identity = 2;
    const uint32_t upper_decoy_pid = 790U;
    const int8_t upper_decoy_identity = 3;
    auto *bytes = reinterpret_cast<uint8_t *>(fixture.network());
    for (int slot = 0; slot < 7; ++slot) {
        std::memcpy(bytes + FirstPidOffset + slot * SlotStride,
                    &process_ids[slot], sizeof(process_ids[slot]));
        std::memcpy(bytes + IdentityOffset + slot * SlotStride,
                    &identities[slot], sizeof(identities[slot]));
    }
    std::memcpy(bytes + FirstPidOffset - SlotStride,
                &lower_decoy_pid, sizeof(lower_decoy_pid));
    std::memcpy(bytes + IdentityOffset - SlotStride,
                &lower_decoy_identity, sizeof(lower_decoy_identity));
    std::memcpy(bytes + FirstPidOffset + 7 * SlotStride,
                &upper_decoy_pid, sizeof(upper_decoy_pid));
    std::memcpy(bytes + IdentityOffset + 7 * SlotStride,
                &upper_decoy_identity, sizeof(upper_decoy_identity));
}

bool unchanged(const Fixture &before, const Fixture &after) {
    return std::memcmp(before.storage, after.storage, sizeof(before.storage)) == 0;
}

uint16_t x87_control_word() {
    uint16_t value;
    __asm__ __volatile__("fnstcw %0" : "=m"(value));
    return value;
}

uint16_t x87_status_word() {
    uint16_t value;
    __asm__ __volatile__("fnstsw %0" : "=am"(value));
    return value;
}

void reset_x87(uint16_t control) {
    __asm__ __volatile__("fninit\n\tfldcw %0" : : "m"(control) : "memory");
}

enum class VectorOracleOperation {
    Subtract,
    AddAssign,
    SubtractAssign,
    Scale,
};

Vector *call_source_vector(VectorOracleOperation operation, uint8_t *storage,
                           size_t left, size_t output, size_t right,
                           uint32_t scalar) {
    auto *left_vector = reinterpret_cast<Vector *>(storage + left);
    auto *output_vector = reinterpret_cast<Vector *>(storage + output);
    auto *right_vector = reinterpret_cast<Vector *>(storage + right);
    switch (operation) {
      case VectorOracleOperation::Subtract:
        return vector_subtract_redirect(
            left_vector, nullptr, output_vector, right_vector);
      case VectorOracleOperation::AddAssign:
        return vector_add_assign_redirect(left_vector, nullptr, right_vector);
      case VectorOracleOperation::SubtractAssign:
        return vector_subtract_assign_redirect(left_vector, nullptr, right_vector);
      case VectorOracleOperation::Scale:
        return vector_scale_redirect(left_vector, nullptr, output_vector, scalar);
    }
    return nullptr;
}

Vector *call_legacy_vector(VectorOracleOperation operation, uint8_t *storage,
                           size_t left, size_t output, size_t right,
                           uint32_t scalar) {
    auto *left_vector = reinterpret_cast<LegacyVector *>(storage + left);
    auto *output_vector = reinterpret_cast<LegacyVector *>(storage + output);
    auto *right_vector = reinterpret_cast<LegacyVector *>(storage + right);
    switch (operation) {
      case VectorOracleOperation::Subtract:
        return left_vector->subtract(*output_vector, *right_vector);
      case VectorOracleOperation::AddAssign:
        return left_vector->add_assign(*right_vector);
      case VectorOracleOperation::SubtractAssign:
        return left_vector->subtract_assign(*right_vector);
      case VectorOracleOperation::Scale:
        return left_vector->scale(*output_vector, scalar);
    }
    return nullptr;
}

void run_geometry_oracles(int &failures) {
    const uint32_t cases[][4] = {
        {0, 0, 0, 0},
        {10, 20, 30, 40},
        {0x7FFFFFFFU, 0x80000000U, 1, 0xFFFFFFFFU},
        {0x80000000U, 0x7FFFFFFFU, 0x80000000U, 0x7FFFFFFFU},
    };
    for (const auto &test : cases) {
        uint8_t source[48];
        uint8_t legacy[48];
        initialize_bytes(source, sizeof(source), 16);
        std::memcpy(legacy, source, sizeof(source));
        auto *source_rect = reinterpret_cast<RECT *>(source + CanarySize);
        auto *legacy_rect = reinterpret_cast<RECT *>(legacy + CanarySize);
        RECT *source_result = make_rect(
            source_rect, int_from_bits(test[0]), int_from_bits(test[1]),
            int_from_bits(test[2]), int_from_bits(test[3]));
        RECT *legacy_result = legacy_make_rect(
            legacy_rect, int_from_bits(test[0]), int_from_bits(test[1]),
            int_from_bits(test[2]), int_from_bits(test[3]));
        if (source_result != source_rect || legacy_result != legacy_rect
                || std::memcmp(source, legacy, sizeof(source)) != 0) {
            std::fprintf(stderr, "RECT construction oracle mismatch\n");
            ++failures;
        }
    }

    const uint32_t box_cases[][6] = {
        {10, 20, 10, 20, 30, 40},
        {40, 20, 10, 20, 30, 40},
        {10, 19, 10, 20, 30, 40},
        {39, 60, 10, 20, 30, 40},
        {0x80000000U, 0, 0x7FFFFFFFU, 0, 1, 1},
        {0x7FFFFFFFU, 0, 0x80000000U, 0, 0xFFFFFFFFU, 1},
    };
    for (const auto &test : box_cases) {
        const int source_result = in_box(
            int_from_bits(test[0]), int_from_bits(test[1]),
            int_from_bits(test[2]), int_from_bits(test[3]),
            int_from_bits(test[4]), int_from_bits(test[5]));
        const int legacy_result = legacy_in_box(
            int_from_bits(test[0]), int_from_bits(test[1]),
            int_from_bits(test[2]), int_from_bits(test[3]),
            int_from_bits(test[4]), int_from_bits(test[5]));
        if (source_result != legacy_result) {
            std::fprintf(stderr, "six-argument in_box oracle mismatch\n");
            ++failures;
        }
    }

    struct CenterCase { size_t x; size_t y; };
    const CenterCase center_cases[] = {{40, 44}, {40, 40}, {20, 44}, {28, 44}};
    const uint32_t rectangles[][4] = {
        {10, 20, 30, 40},
        {10, 20, 5, 13},
        {10, 20, 7, 17},
        {0x80000000U, 0x7FFFFFFFU, 0x7FFFFFFFU, 0x80000000U},
    };
    for (size_t rectangle_index = 0;
            rectangle_index < sizeof(rectangles) / sizeof(rectangles[0]);
            ++rectangle_index) {
      for (size_t case_index = 0;
              case_index < sizeof(center_cases) / sizeof(center_cases[0]);
              ++case_index) {
        const CenterCase &test = center_cases[case_index];
        alignas(RECT) uint8_t source[64];
        alignas(RECT) uint8_t legacy[64];
        initialize_bytes(source, sizeof(source), 32);
        std::memcpy(source + 16, rectangles[rectangle_index],
                    sizeof(rectangles[rectangle_index]));
        std::memcpy(legacy, source, sizeof(source));
        const int source_result = tutwin_rect_center_redirect(
            nullptr, nullptr, reinterpret_cast<RECT *>(source + 16),
            reinterpret_cast<int *>(source + test.x),
            reinterpret_cast<int *>(source + test.y));
        const int legacy_result = reinterpret_cast<LegacyTutWin *>(1)->rect_center(
            reinterpret_cast<RECT *>(legacy + 16),
            reinterpret_cast<int *>(legacy + test.x),
            reinterpret_cast<int *>(legacy + test.y));
        if (source_result != legacy_result
                || std::memcmp(source, legacy, sizeof(source)) != 0) {
            std::fprintf(stderr,
                "rectangle-center oracle mismatch: rectangle=%zu case=%zu "
                "result=%08x/%08x\n",
                rectangle_index, case_index,
                static_cast<uint32_t>(source_result),
                static_cast<uint32_t>(legacy_result));
            report_difference("rectangle-center", "storage",
                              source, legacy, sizeof(source));
            ++failures;
        }
      }
    }
}

extern "C" void __cdecl legacy_vector_add(Vector *, Vector *, Vector *)
    asm("_opensmacx_legacy_00628150");

void run_vector_add_oracles(int &failures) {
    // Component zero and the remaining components load their operands in
    // opposite order, so signalling payloads and rounding are compared under
    // every layout and rounding mode rather than just the numeric result.
    const uint32_t values[][3] = {
        {0x3F800000U, 0x40000000U, 0x40400000U},
        {0x7F800001U, 0x3F800000U, 0xFF800000U},   // signalling NaN
        {0x7FC00000U, 0x00000001U, 0x80000001U},   // quiet NaN, denormals
        {0x7F800000U, 0xFF800000U, 0x00000000U},   // infinities
        {0x80000000U, 0x80000000U, 0x7F7FFFFFU},
    };
    const size_t layouts[][3] = {
        {16, 32, 48}, {16, 32, 16}, {16, 16, 32}, {16, 16, 16},
    };
    const uint16_t rounding_modes[] = {0x037F, 0x077F, 0x0B7F, 0x0F7F};
    for (const auto &triple : values) {
        for (const auto &layout : layouts) {
            for (uint16_t control : rounding_modes) {
                uint8_t source[80];
                uint8_t legacy[80];
                initialize_bytes(source, sizeof(source), 16);
                for (size_t index = 0; index < 3; ++index) {
                    std::memcpy(source + layout[0] + index * 4,
                                &triple[index], sizeof(uint32_t));
                    std::memcpy(source + layout[1] + index * 4,
                                &triple[(index + 1) % 3], sizeof(uint32_t));
                }
                std::memcpy(legacy, source, sizeof(source));

                uint16_t source_status = 0;
                uint16_t legacy_status = 0;
                reset_x87(control);
                legacy_vector_add(
                    reinterpret_cast<Vector *>(legacy + layout[0]),
                    reinterpret_cast<Vector *>(legacy + layout[1]),
                    reinterpret_cast<Vector *>(legacy + layout[2]));
                legacy_status = x87_status_word();

                reset_x87(control);
                vector_add(
                    reinterpret_cast<Vector *>(source + layout[0]),
                    reinterpret_cast<Vector *>(source + layout[1]),
                    reinterpret_cast<Vector *>(source + layout[2]));
                source_status = x87_status_word();

                if (legacy_status != source_status
                        || std::memcmp(source, legacy, sizeof(source)) != 0) {
                    std::fprintf(stderr, "vector add oracle mismatch\n");
                    report_difference("vector-add", "storage",
                                      source, legacy, sizeof(source));
                    ++failures;
                }
            }
        }
    }
}

void run_vector_oracles(int &failures) {
    alignas(Vector) uint8_t source_lifecycle[sizeof(Vector) + 32];
    alignas(Vector) uint8_t legacy_lifecycle[sizeof(Vector) + 32];
    initialize_bytes(source_lifecycle, sizeof(source_lifecycle), sizeof(Vector));
    std::memcpy(legacy_lifecycle, source_lifecycle, sizeof(source_lifecycle));
    auto *source_vector = reinterpret_cast<Vector *>(source_lifecycle + CanarySize);
    auto *legacy_vector = reinterpret_cast<LegacyVector *>(legacy_lifecycle + CanarySize);
    Vector *source_constructed = vector_construct_redirect(source_vector, nullptr);
    Vector *legacy_constructed = legacy_vector->construct();
    if (source_constructed != source_vector
            || legacy_constructed != reinterpret_cast<Vector *>(legacy_vector)
            || std::memcmp(source_lifecycle, legacy_lifecycle,
                           sizeof(source_lifecycle)) != 0) {
        std::fprintf(stderr, "Vector constructor oracle mismatch\n");
        ++failures;
    }
    initialize_bytes(source_lifecycle, sizeof(source_lifecycle), sizeof(Vector));
    std::memcpy(legacy_lifecycle, source_lifecycle, sizeof(source_lifecycle));
    const uintptr_t source_close = vector_close_redirect(source_vector, nullptr);
    const uintptr_t legacy_close = legacy_vector->close();
    if (source_close != legacy_close
            || std::memcmp(source_lifecycle, legacy_lifecycle,
                           sizeof(source_lifecycle)) != 0) {
        std::fprintf(stderr, "Vector close oracle mismatch\n");
        ++failures;
    }

    const VectorOracleOperation operations[] = {
        VectorOracleOperation::Subtract,
        VectorOracleOperation::AddAssign,
        VectorOracleOperation::SubtractAssign,
        VectorOracleOperation::Scale,
    };
    struct Layout { size_t left; size_t output; size_t right; };
    const Layout layouts[] = {
        {16, 32, 48},
        {20, 32, 16},
        {16, 20, 48},
        {16, 16, 16},
    };
    const uint32_t values[][7] = {
        {0x3FC00000U, 0xC0000000U, 0x40800000U,
         0x3F000000U, 0x40400000U, 0xC1000000U, 0x40000000U},
        {0x3F800000U, 0x33800000U, 0x80000000U,
         0x33800000U, 0x3F800001U, 0x00000001U, 0x3F800001U},
        {0x7F800001U, 0x7FC12345U, 0x7F800000U,
         0x3F800000U, 0xFF800001U, 0xFF800000U, 0x7FC54321U},
    };
    const uint16_t original_control = x87_control_word();
    const uint16_t rounding_modes[] = {0x0000, 0x0400, 0x0800, 0x0C00};
    for (VectorOracleOperation operation : operations) {
        for (const Layout &layout : layouts) {
            for (const auto &test : values) {
                for (uint16_t rounding : rounding_modes) {
                    alignas(Vector) uint8_t source[80];
                    alignas(Vector) uint8_t legacy[80];
                    initialize_bytes(source, sizeof(source), 48);
                    std::memcpy(source + layout.left, test, 12);
                    std::memcpy(source + layout.right, test + 3, 12);
                    std::memcpy(legacy, source, sizeof(source));
                    const uint16_t control = static_cast<uint16_t>(
                        (original_control | 0x003F) & ~0x0C00U) | rounding;
                    reset_x87(control);
                    Vector *source_result = call_source_vector(
                        operation, source, layout.left, layout.output,
                        layout.right, test[6]);
                    const uint16_t source_status = x87_status_word();
                    reset_x87(control);
                    Vector *legacy_result = call_legacy_vector(
                        operation, legacy, layout.left, layout.output,
                        layout.right, test[6]);
                    const uint16_t legacy_status = x87_status_word();
                    const ptrdiff_t source_offset = reinterpret_cast<uint8_t *>(
                        source_result) - source;
                    const ptrdiff_t legacy_offset = reinterpret_cast<uint8_t *>(
                        legacy_result) - legacy;
                    if (source_offset != legacy_offset
                            || source_status != legacy_status
                            || std::memcmp(source, legacy, sizeof(source)) != 0) {
                        std::fprintf(stderr,
                            "Vector arithmetic oracle mismatch: op=%d layout=%zu/%zu/%zu "
                            "rounding=0x%04x\n",
                            static_cast<int>(operation), layout.left, layout.output,
                            layout.right, rounding);
                        ++failures;
                    }
                }
            }
        }
    }
    reset_x87(original_control);
}

}  // namespace

int main() {
    const uint32_t process_ids[] = {
        0U, 1U, 0x7FFFFFFFU, 0x80000000U, 0xFFFFFFFFU, 123U, 456U, 789U, 790U,
    };
    const int identities[] = {0, 1, 0x7F, -128, -1, 42, -42, 2, 3, 128, -129};
    int failures = 0;

    run_geometry_oracles(failures);
    run_vector_oracles(failures);
    run_vector_add_oracles(failures);

    for (uint32_t process_id : process_ids) {
        Fixture source{};
        Fixture legacy{};
        initialize(source);
        initialize(legacy);
        Fixture source_before = source;
        Fixture legacy_before = legacy;
        const int source_result = source.network()->pid_2_who(process_id);
        const int legacy_result = legacy.legacy_network()->pid_2_who(process_id);
        const bool boundary_decoy = process_id == 789U || process_id == 790U;
        if (source_result != legacy_result
                || (boundary_decoy && source_result != 0)
                || !unchanged(source_before, source)
                || !unchanged(legacy_before, legacy)) {
            std::fprintf(stderr, "pid_2_who mismatch for 0x%08x\n", process_id);
            ++failures;
        }
    }

    for (int identity : identities) {
        Fixture source{};
        Fixture legacy{};
        initialize(source);
        initialize(legacy);
        Fixture source_before = source;
        Fixture legacy_before = legacy;
        const int source_pid = source.network()->who_2_pid(identity);
        const int legacy_pid = legacy.legacy_network()->who_2_pid(identity);
        const int source_index = source.network()->who_2_idx(identity);
        const int legacy_index = legacy.legacy_network()->who_2_idx(identity);
        const bool boundary_decoy = identity == 2 || identity == 3;
        if (source_pid != legacy_pid || source_index != legacy_index
                || (boundary_decoy && (source_pid != 0 || source_index != 0))
                || !unchanged(source_before, source)
                || !unchanged(legacy_before, legacy)) {
            std::fprintf(stderr, "identity lookup mismatch for %d\n", identity);
            ++failures;
        }
    }

    Fixture source{};
    Fixture legacy{};
    initialize(source);
    initialize(legacy);
    const uint32_t duplicate_pid = 1U;
    const int8_t later_pid_identity = 77;
    const int8_t duplicate_identity = 1;
    const uint32_t later_identity_pid = 456U;
    for (Fixture *fixture : {&source, &legacy}) {
        auto *bytes = reinterpret_cast<uint8_t *>(fixture->network());
        std::memcpy(bytes + FirstPidOffset + 5 * SlotStride,
                    &duplicate_pid, sizeof(duplicate_pid));
        std::memcpy(bytes + IdentityOffset + 5 * SlotStride,
                    &later_pid_identity, sizeof(later_pid_identity));
        std::memcpy(bytes + FirstPidOffset + 6 * SlotStride,
                    &later_identity_pid, sizeof(later_identity_pid));
        std::memcpy(bytes + IdentityOffset + 6 * SlotStride,
                    &duplicate_identity, sizeof(duplicate_identity));
    }
    Fixture source_before = source;
    Fixture legacy_before = legacy;
    const int source_identity = source.network()->pid_2_who(duplicate_pid);
    const int legacy_identity = legacy.legacy_network()->pid_2_who(duplicate_pid);
    const int source_pid = source.network()->who_2_pid(duplicate_identity);
    const int legacy_pid = legacy.legacy_network()->who_2_pid(duplicate_identity);
    const int source_index = source.network()->who_2_idx(duplicate_identity);
    const int legacy_index = legacy.legacy_network()->who_2_idx(duplicate_identity);
    if (source_identity != 1 || legacy_identity != 1
            || source_pid != 1 || legacy_pid != 1
            || source_index != 2 || legacy_index != 2
            || !unchanged(source_before, source)
            || !unchanged(legacy_before, legacy)) {
        std::fprintf(stderr, "first-match duplicate behavior mismatch\n");
        ++failures;
    }

    const uint32_t reseed_values[] = {
        0U, 1U, 0x7FFFFFFFU, 0x80000000U, 0xFFFFFFFFU,
    };
    for (uint32_t seed : reseed_values) {
        RandomFixture source_fixture{};
        RandomFixture legacy_fixture{};
        initialize(source_fixture, 0xA55AA55AU);
        initialize(legacy_fixture, 0xA55AA55AU);
        source_fixture.source()->reseed(seed);
        legacy_fixture.legacy()->reseed(seed);
        if (std::memcmp(source_fixture.storage, legacy_fixture.storage,
                        sizeof(source_fixture.storage)) != 0
                || !canaries_intact(source_fixture)
                || !canaries_intact(legacy_fixture)) {
            std::fprintf(stderr, "Random reseed mismatch for 0x%08x\n", seed);
            ++failures;
        }
    }

    struct IntegerFixture {
        uint32_t seed;
        uint32_t min;
        uint32_t max;
    };
    const IntegerFixture integer_fixtures[] = {
        {0U, 0U, 0U},
        {1U, 0U, 1U},
        {0xFFFFFFFFU, 1U, 0xFFFFFFFFU},
        {0x12345678U, 0x80000000U, 0x7FFFFFFFU},
        {0x87654321U, 0xFFFFFFFFU, 0x80000000U},
        {0xA5A5A5A5U, 123U, 456U},
    };
    for (const IntegerFixture &fixture : integer_fixtures) {
        RandomFixture source_fixture{};
        RandomFixture legacy_fixture{};
        initialize(source_fixture, fixture.seed);
        initialize(legacy_fixture, fixture.seed);
        _clearfp();
        const uint32_t source_result = source_fixture.source()->get(
            fixture.min, fixture.max);
        const unsigned int source_status = _statusfp();
        _clearfp();
        const uint32_t legacy_result = legacy_fixture.legacy()->get(
            fixture.min, fixture.max);
        const unsigned int legacy_status = _statusfp();
        _clearfp();
        if (source_result != legacy_result || source_status != legacy_status
                || std::memcmp(source_fixture.storage, legacy_fixture.storage,
                               sizeof(source_fixture.storage)) != 0
                || !canaries_intact(source_fixture)
                || !canaries_intact(legacy_fixture)) {
            std::fprintf(stderr,
                "Random integer mismatch for seed 0x%08x, bounds 0x%08x..0x%08x\n",
                fixture.seed, fixture.min, fixture.max);
            ++failures;
        }
    }

    struct MoveFixture {
        uint32_t flags;
        uint32_t x;
        uint32_t y;
        uint32_t left;
        uint32_t top;
        uint32_t right;
        uint32_t bottom;
    };
    const MoveFixture move_fixtures[] = {
        {0U, 100U, 200U, 10U, 20U, 50U, 80U},
        {0xFFFFFFFDU, 0xFFFFFF9CU, 0x0000012CU,
         0x00000032U, 0xFFFFFFCEU, 0x00000096U, 0x0000004BU},
        {2U, 0x80000000U, 0x7FFFFFFFU,
         0x7FFFFFFFU, 0x80000000U, 0x80000005U, 0x7FFFFFF0U},
        {0xA5A5A5A6U, 0xFFFFFFFFU, 0U,
         0x80000000U, 0xFFFFFFFFU, 0x7FFFFFFFU, 0x80000000U},
    };
    for (const MoveFixture &fixture : move_fixtures) {
        WinFixture source_window{};
        WinFixture legacy_window{};
        initialize(source_window);
        initialize(legacy_window);
        const size_t target = (fixture.flags & 2U) ? 0x14C : 0x13C;
        for (WinFixture *window : {&source_window, &legacy_window}) {
            write_object(window->storage, 0x9C, fixture.flags);
            write_object(window->storage, target, fixture.left);
            write_object(window->storage, target + 4, fixture.top);
            write_object(window->storage, target + 8, fixture.right);
            write_object(window->storage, target + 12, fixture.bottom);
        }
        const int source_result = source_window.source()->move(
            int_from_bits(fixture.x), int_from_bits(fixture.y));
        const int legacy_result = legacy_window.legacy()->move(
            int_from_bits(fixture.x), int_from_bits(fixture.y));
        if (source_result != legacy_result || source_result != 0
                || std::memcmp(source_window.storage, legacy_window.storage,
                               sizeof(source_window.storage)) != 0
                || !canaries_intact(source_window.storage, sizeof(Win))
                || !canaries_intact(legacy_window.storage, sizeof(Win))) {
            std::fprintf(stderr, "Win::move mismatch for flags 0x%08x\n",
                         fixture.flags);
            ++failures;
        }
    }

    const uint32_t paging_values[] = {
        0U, 1U, 0xFFFFFFFFU, 0x80000000U, 0x7FFFFFFFU, 0xA55AA55AU,
    };
    for (int vertical = 0; vertical < 2; ++vertical) {
        for (uint32_t paging_bits : paging_values) {
            WinFixture source_window{};
            WinFixture legacy_window{};
            ScrollFixture source_scroll{};
            ScrollFixture legacy_scroll{};
            initialize(source_window);
            initialize(legacy_window);
            initialize(source_scroll);
            initialize(legacy_scroll);
            const size_t pointer_offset = vertical ? 0x43C : 0x440;
            Scroll *source_pointer = source_scroll.source();
            Scroll *legacy_pointer = reinterpret_cast<Scroll *>(
                legacy_scroll.storage + CanarySize);
            write_object(source_window.storage, pointer_offset, source_pointer);
            write_object(legacy_window.storage, pointer_offset, legacy_pointer);
            WinFixture source_window_before = source_window;
            WinFixture legacy_window_before = legacy_window;
            const int paging = int_from_bits(paging_bits);
            if (vertical) {
                source_window.source()->set_vert_paging(paging);
                legacy_window.legacy()->set_vert_paging(paging);
            } else {
                source_window.source()->set_horz_paging(paging);
                legacy_window.legacy()->set_horz_paging(paging);
            }
            if (std::memcmp(source_scroll.storage, legacy_scroll.storage,
                            sizeof(source_scroll.storage)) != 0
                    || std::memcmp(source_window.storage, source_window_before.storage,
                                   sizeof(source_window.storage)) != 0
                    || std::memcmp(legacy_window.storage, legacy_window_before.storage,
                                   sizeof(legacy_window.storage)) != 0
                    || !canaries_intact(source_scroll.storage, sizeof(Scroll))
                    || !canaries_intact(legacy_scroll.storage, sizeof(Scroll))) {
                std::fprintf(stderr, "Win paging mismatch for %s value 0x%08x\n",
                             vertical ? "vertical" : "horizontal", paging_bits);
                ++failures;
            }
        }
    }

    for (int vertical = 0; vertical < 2; ++vertical) {
        WinFixture null_source{};
        WinFixture null_legacy{};
        initialize(null_source);
        initialize(null_legacy);
        Scroll *null_scroll = nullptr;
        const size_t pointer_offset = vertical ? 0x43C : 0x440;
        write_object(null_source.storage, pointer_offset, null_scroll);
        write_object(null_legacy.storage, pointer_offset, null_scroll);
        WinFixture null_source_before = null_source;
        WinFixture null_legacy_before = null_legacy;
        if (vertical) {
            null_source.source()->set_vert_paging(INT_MIN);
            null_legacy.legacy()->set_vert_paging(INT_MIN);
        } else {
            null_source.source()->set_horz_paging(INT_MAX);
            null_legacy.legacy()->set_horz_paging(INT_MAX);
        }
        if (std::memcmp(null_source.storage, null_source_before.storage,
                        sizeof(null_source.storage)) != 0
                || std::memcmp(null_legacy.storage, null_legacy_before.storage,
                               sizeof(null_legacy.storage)) != 0) {
            std::fprintf(stderr, "Win null %s paging mismatch\n",
                         vertical ? "vertical" : "horizontal");
            ++failures;
        }
    }

    const uint32_t border_colors[] = {
        0xFFFFFFFFU, 0U, 1U, 0x80000000U, 0x7FFFFFFFU,
    };
    const uint32_t thicknesses[] = {
        0xFFFFFFFFU, 0U, 1U, 2U, 0x80000000U, 0x7FFFFFFFU,
    };
    for (uint32_t color : border_colors) {
        for (uint32_t thickness : thicknesses) {
            ScrollFixture source_scroll{};
            ScrollFixture legacy_scroll{};
            initialize(source_scroll);
            initialize(legacy_scroll);
            write_object(source_scroll.storage, 0xA60, thickness);
            write_object(legacy_scroll.storage, 0xA60, thickness);
            source_scroll.source()->set_border_color(int_from_bits(color));
            legacy_scroll.legacy()->set_border_color(int_from_bits(color));
            if (std::memcmp(source_scroll.storage, legacy_scroll.storage,
                            sizeof(source_scroll.storage)) != 0
                    || !canaries_intact(source_scroll.storage, sizeof(Scroll))
                    || !canaries_intact(legacy_scroll.storage, sizeof(Scroll))) {
                std::fprintf(stderr,
                    "Scroll border mismatch for color 0x%08x, thickness 0x%08x\n",
                    color, thickness);
                ++failures;
            }
        }
    }

    struct SpriteSetterCase {
        const char *name;
        uint32_t width;
        uint32_t stored_height;
    };
    const SpriteSetterCase sprite_setter_cases[] = {
        {"horizontal", 100U, 0xFFFFFFECU},
        {"vertical", 20U, 0xFFFFFF9CU},
        {"equal dimensions", 20U, 0xFFFFFFECU},
        {"signed comparison", 1U, 1U},
        {"negated INT_MIN horizontal", 0x80000001U, 0x80000000U},
        {"negated INT_MIN equal", 0x80000000U, 0x80000000U},
    };
    Sprite *sprite_sets[][3] = {
        {nullptr, nullptr, nullptr},
        {reinterpret_cast<Sprite *>(0x10101010U),
         reinterpret_cast<Sprite *>(0x20202020U),
         reinterpret_cast<Sprite *>(0x30303030U)},
        {reinterpret_cast<Sprite *>(0x45454545U),
         reinterpret_cast<Sprite *>(0x56565656U),
         reinterpret_cast<Sprite *>(0x45454545U)},
    };
    for (const SpriteSetterCase &test : sprite_setter_cases) {
        for (Sprite **sprites : sprite_sets) {
            for (int right = 0; right < 2; ++right) {
                ScrollFixture source_scroll{};
                ScrollFixture legacy_scroll{};
                initialize(source_scroll);
                initialize(legacy_scroll);
                write_object(source_scroll.storage, 0x4C4, test.width);
                write_object(legacy_scroll.storage, 0x4C4, test.width);
                write_object(source_scroll.storage, 0x4C8, test.stored_height);
                write_object(legacy_scroll.storage, 0x4C8, test.stored_height);
                Sprite *source_result = right
                    ? scroll_set_sprite_right_redirect(
                        source_scroll.source(), nullptr,
                        sprites[0], sprites[1], sprites[2])
                    : scroll_set_sprite_left_redirect(
                        source_scroll.source(), nullptr,
                        sprites[0], sprites[1], sprites[2]);
                Sprite *legacy_result = right
                    ? legacy_scroll.legacy()->set_sprite_right(
                        sprites[0], sprites[1], sprites[2])
                    : legacy_scroll.legacy()->set_sprite_left(
                        sprites[0], sprites[1], sprites[2]);
                if (source_result != sprites[0]
                        || legacy_result != sprites[0]
                        || std::memcmp(source_scroll.storage,
                                       legacy_scroll.storage,
                                       sizeof(source_scroll.storage)) != 0
                        || !canaries_intact(
                            source_scroll.storage, sizeof(Scroll))
                        || !canaries_intact(
                            legacy_scroll.storage, sizeof(Scroll))) {
                    std::fprintf(stderr,
                        "Scroll %s sprite setter mismatch for %s\n",
                        right ? "right" : "left", test.name);
                    report_difference(test.name, "object",
                                      source_scroll.storage,
                                      legacy_scroll.storage,
                                      sizeof(source_scroll.storage));
                    ++failures;
                }
            }
        }
    }

    struct ExpandRectCase {
        const char *name;
        uint32_t rect[4];
        uint32_t horizontal;
        uint32_t vertical;
    };
    const ExpandRectCase expand_rect_cases[] = {
        {"zero", {0U, 0U, 0U, 0U}, 0U, 0U},
        {"positive", {10U, 20U, 30U, 40U}, 3U, 4U},
        {"negative", {10U, 20U, 30U, 40U},
         0xFFFFFFFFU, 0xFFFFFFFEU},
        {"signed extremes",
         {0x80000000U, 0x7FFFFFFFU, 0xFFFFFFFFU, 0U},
         0x80000000U, 0x7FFFFFFFU},
        {"wrapping", {0U, 0xFFFFFFFFU, 0x7FFFFFFFU, 0x80000000U},
         0x7FFFFFFFU, 0x80000000U},
    };
    for (const ExpandRectCase &test : expand_rect_cases) {
        alignas(RECT) uint8_t source_storage[sizeof(RECT) + CanarySize * 2];
        alignas(RECT) uint8_t legacy_storage[sizeof(RECT) + CanarySize * 2];
        initialize_bytes(source_storage, sizeof(source_storage), sizeof(RECT));
        initialize_bytes(legacy_storage, sizeof(legacy_storage), sizeof(RECT));
        std::memcpy(source_storage + CanarySize, test.rect, sizeof(test.rect));
        std::memcpy(legacy_storage + CanarySize, test.rect, sizeof(test.rect));
        auto *source_rect = reinterpret_cast<RECT *>(
            source_storage + CanarySize);
        auto *legacy_rect = reinterpret_cast<RECT *>(
            legacy_storage + CanarySize);
        RECT *source_result = expand_rect(
            source_rect, int_from_bits(test.horizontal),
            int_from_bits(test.vertical));
        RECT *legacy_result = legacy_expand_rect(
            legacy_rect, int_from_bits(test.horizontal),
            int_from_bits(test.vertical));
        if (source_result != source_rect || legacy_result != legacy_rect
                || std::memcmp(source_storage, legacy_storage,
                               sizeof(source_storage)) != 0
                || !canaries_intact(source_storage, sizeof(RECT))
                || !canaries_intact(legacy_storage, sizeof(RECT))) {
            std::fprintf(stderr, "RECT expansion mismatch for %s\n", test.name);
            report_difference(test.name, "RECT", source_storage,
                              legacy_storage, sizeof(source_storage));
            ++failures;
        }
    }

    const ThumbOracleCase thumb_cases[] = {
        {"horizontal no-buttons minimum", 0xA5A50002U, 0xFFFFFFFFU,
         200U, 0xFFFFFFECU, 0U, 100U, 0U, 0xFFFFFFFFU,
         {3U, 4U, 13U, 14U}},
        {"horizontal buttons midpoint", 0xA5A50000U, 0U,
         300U, 0xFFFFFFECU, 0xFFFFFFF6U, 10U, 0U, 0xFFFFFFFFU,
         {5U, 7U, 25U, 37U}},
        {"vertical truncation", 2U, 0xFFFFFFFFU,
         20U, 0xFFFFFF38U, 0U, 8U, 3U, 0xFFFFFFFFU,
         {9U, 11U, 19U, 41U}},
        {"equal dimensions", 0U, 0U,
         20U, 0xFFFFFFECU, 0U, 10U, 5U, 0xFFFFFFFFU,
         {2U, 3U, 12U, 23U}},
        {"reversed range", 2U, 0U,
         160U, 0xFFFFFFE0U, 10U, 0xFFFFFFF6U, 0U, 0xFFFFFFFFU,
         {6U, 9U, 17U, 30U}},
        {"wrapping static arithmetic", 0U, 0x80000000U,
         0x7FFFFFF0U, 0xFFFFFFE0U, 0x70000000U, 0x90000000U,
         0x80000001U, 0xFFFFFFFFU,
         {0x70000000U, 0x11111111U, 0xF0000010U, 0x33333333U}},
        {"horizontal drag below", 2U, 0xFFFFFFFFU,
         100U, 0xFFFFFFECU, 0U, 10U, 5U, 0xFFFFFF80U,
         {3U, 4U, 13U, 14U}},
        {"horizontal drag above", 2U, 0U,
         100U, 0xFFFFFFECU, 0U, 10U, 5U, 200U,
         {3U, 4U, 13U, 14U}},
        {"horizontal crossed bounds", 0U, 0U,
         10U, 0xFFFFFFFCU, 0U, 10U, 5U, 0U,
         {0U, 0U, 20U, 6U}},
        {"vertical drag", 0U, 0U,
         12U, 0xFFFFFF80U, 0U, 10U, 5U, 90U,
         {1U, 4U, 9U, 40U}},
        {"vertical nonsquare", 0U, 0xFFFFFFFFU,
         10U, 0xFFFFFF00U, 0U, 10U, 5U, 20U,
         {7U, 9U, 12U, 109U}},
        {"horizontal wrapped lower", 0U, 0xFFFFFFFFU,
         100U, 0xFFFFFFECU, 0U, 10U, 5U, 0U,
         {0U, 0U, 0x80000000U, 10U}},
        {"vertical wrapped lower", 0U, 0xFFFFFFFFU,
         10U, 0xFFFFFF9CU, 0U, 10U, 5U, 0U,
         {0U, 0U, 0x80000000U, 10U}},
        // The drag branch re-derives the horizontal/vertical split with its
        // own width > height comparison; the existing "equal dimensions"
        // case only reaches the non-drag copy. Equal axes here make the
        // strictness observable: the correct strict compare goes vertical
        // and shifts top/bottom, while an >= mutant goes horizontal and
        // shifts left/right instead.
        {"equal dimensions drag", 0U, 0xFFFFFFFFU,
         20U, 0xFFFFFFECU, 0U, 10U, 5U, 4U,
         {2U, 3U, 12U, 23U}},
    };
    for (const ThumbOracleCase &test : thumb_cases) {
        ScrollFixture source_scroll{};
        ScrollFixture legacy_scroll{};
        alignas(RECT) uint8_t source_output[sizeof(RECT) + CanarySize * 2];
        alignas(RECT) uint8_t legacy_output[sizeof(RECT) + CanarySize * 2];
        initialize(source_scroll, test);
        initialize(legacy_scroll, test);
        initialize_bytes(source_output, sizeof(source_output), sizeof(RECT));
        initialize_bytes(legacy_output, sizeof(legacy_output), sizeof(RECT));
        auto *source_rect = reinterpret_cast<RECT *>(source_output + CanarySize);
        auto *legacy_rect = reinterpret_cast<RECT *>(legacy_output + CanarySize);
        RECT *source_result = scroll_compute_thumb_rect_redirect(
            source_scroll.source(), nullptr, source_rect);
        RECT *legacy_result = legacy_scroll.legacy()->compute_thumb_rect(legacy_rect);
        if (source_result != reinterpret_cast<RECT *>(
                    source_scroll.storage + CanarySize + 0xA4C)
                || legacy_result != reinterpret_cast<RECT *>(
                    legacy_scroll.storage + CanarySize + 0xA4C)
                || std::memcmp(source_scroll.storage, legacy_scroll.storage,
                               sizeof(source_scroll.storage)) != 0
                || std::memcmp(source_output, legacy_output,
                               sizeof(source_output)) != 0
                || !canaries_intact(source_scroll.storage, sizeof(Scroll))
                || !canaries_intact(legacy_scroll.storage, sizeof(Scroll))
                || !canaries_intact(source_output, sizeof(RECT))
                || !canaries_intact(legacy_output, sizeof(RECT))) {
            std::fprintf(stderr, "Scroll thumb mismatch for %s\n", test.name);
            report_difference(test.name, "object",
                              source_scroll.storage, legacy_scroll.storage,
                              sizeof(source_scroll.storage));
            report_difference(test.name, "output",
                              source_output, legacy_output,
                              sizeof(source_output));
            ++failures;
        }
    }

    ScrollFixture source_alias{};
    ScrollFixture legacy_alias{};
    initialize(source_alias, thumb_cases[1]);
    initialize(legacy_alias, thumb_cases[1]);
    auto *source_alias_rect = reinterpret_cast<RECT *>(
        source_alias.storage + CanarySize + 0xA4C);
    auto *legacy_alias_rect = reinterpret_cast<RECT *>(
        legacy_alias.storage + CanarySize + 0xA4C);
    RECT *source_alias_result = scroll_compute_thumb_rect_redirect(
        source_alias.source(), nullptr, source_alias_rect);
    RECT *legacy_alias_result = legacy_alias.legacy()->compute_thumb_rect(
        legacy_alias_rect);
    if (source_alias_result != source_alias_rect
            || legacy_alias_result != legacy_alias_rect
            || std::memcmp(source_alias.storage, legacy_alias.storage,
                           sizeof(source_alias.storage)) != 0) {
        std::fprintf(stderr, "Scroll aliased thumb mismatch\n");
        ++failures;
    }

    const size_t overlap_offsets[] = {0xA3C, 0xA48, 0xA50};
    for (size_t overlap_offset : overlap_offsets) {
        ScrollFixture source_overlap{};
        ScrollFixture legacy_overlap{};
        const ThumbOracleCase &test = overlap_offset == 0xA3C
            ? thumb_cases[6] : thumb_cases[1];
        initialize(source_overlap, test);
        initialize(legacy_overlap, test);
        auto *source_overlap_rect = reinterpret_cast<RECT *>(
            source_overlap.storage + CanarySize + overlap_offset);
        auto *legacy_overlap_rect = reinterpret_cast<RECT *>(
            legacy_overlap.storage + CanarySize + overlap_offset);
        RECT *source_overlap_result = scroll_compute_thumb_rect_redirect(
            source_overlap.source(), nullptr, source_overlap_rect);
        RECT *legacy_overlap_result = legacy_overlap.legacy()->compute_thumb_rect(
            legacy_overlap_rect);
        if (source_overlap_result != reinterpret_cast<RECT *>(
                    source_overlap.storage + CanarySize + 0xA4C)
                || legacy_overlap_result != reinterpret_cast<RECT *>(
                    legacy_overlap.storage + CanarySize + 0xA4C)
                || std::memcmp(source_overlap.storage, legacy_overlap.storage,
                               sizeof(source_overlap.storage)) != 0
                || !canaries_intact(source_overlap.storage, sizeof(Scroll))
                || !canaries_intact(legacy_overlap.storage, sizeof(Scroll))) {
            std::fprintf(stderr,
                "Scroll overlapping thumb mismatch at 0x%zx\n", overlap_offset);
            report_difference("Scroll overlapping thumb", "object",
                              source_overlap.storage, legacy_overlap.storage,
                              sizeof(source_overlap.storage));
            ++failures;
        }
    }

    const PullOperation pull_operations[] = {
        PullOperation::Hide,
        PullOperation::Show,
        PullOperation::Disable,
        PullOperation::Enable,
        PullOperation::Check,
        PullOperation::Uncheck,
    };
    const int pull_indices[] = {0, 31, 63};
    const int pull_ids[] = {INT_MIN, 17, INT_MAX};
    for (PullOperation operation : pull_operations) {
        for (int target_case = 0; target_case < 3; ++target_case) {
            PullDownFixture source_pull{};
            PullDownFixture legacy_pull{};
            initialize(source_pull);
            initialize(legacy_pull);
            fill_pull_ids(source_pull);
            fill_pull_ids(legacy_pull);
            const int index = pull_indices[target_case];
            const int id = pull_ids[target_case];
            const size_t item_offset = 0xA18 + index * 0x14;
            const uint32_t flags = pull_flags(operation);
            const uint32_t visible = operation == PullOperation::Hide
                ? 0U : operation == PullOperation::Show
                ? 0x7FFFFFFFU : 0x13579BDFU;
            for (PullDownFixture *pull : {&source_pull, &legacy_pull}) {
                write_object(pull->storage, item_offset + 8, id);
                write_object(pull->storage, item_offset + 0xC, flags);
                write_object(pull->storage, 0xF20, INT_MIN);
                write_object(pull->storage, 0xF2C, visible);
            }
            const int source_result = call_source(source_pull.source(), operation, id);
            const int legacy_result = call_legacy(legacy_pull.legacy(), operation, id);
            if (source_result != legacy_result
                    || std::memcmp(source_pull.storage, legacy_pull.storage,
                                   sizeof(source_pull.storage)) != 0
                    || !canaries_intact(source_pull.storage, sizeof(PullDown))
                    || !canaries_intact(legacy_pull.storage, sizeof(PullDown))) {
                std::fprintf(stderr,
                    "PullDown mutation mismatch for operation %d, index %d\n",
                    static_cast<int>(operation), index);
                ++failures;
            }
        }

        PullDownFixture source_miss{};
        PullDownFixture legacy_miss{};
        initialize(source_miss);
        initialize(legacy_miss);
        fill_pull_ids(source_miss);
        fill_pull_ids(legacy_miss);
        const int sentinel = -1;
        const int requested = 77;
        for (PullDownFixture *pull : {&source_miss, &legacy_miss}) {
            write_object(pull->storage, 0xA20 + 0x14, sentinel);
            write_object(pull->storage, 0xA20 + 2 * 0x14, requested);
        }
        const int source_result = call_source(
            source_miss.source(), operation, requested);
        const int legacy_result = call_legacy(
            legacy_miss.legacy(), operation, requested);
        if (source_result != legacy_result
                || std::memcmp(source_miss.storage, legacy_miss.storage,
                               sizeof(source_miss.storage)) != 0) {
            std::fprintf(stderr, "PullDown miss mismatch for operation %d\n",
                         static_cast<int>(operation));
            ++failures;
        }
    }

    for (PullOperation operation : {PullOperation::Hide, PullOperation::Show}) {
        PullDownFixture source_pull{};
        PullDownFixture legacy_pull{};
        initialize(source_pull);
        initialize(legacy_pull);
        fill_pull_ids(source_pull);
        fill_pull_ids(legacy_pull);
        const uint32_t flags = operation == PullOperation::Hide
            ? 0xA5A50004U : 0xA5A50005U;
        const uint32_t visible = 0x89ABCDEFU;
        for (PullDownFixture *pull : {&source_pull, &legacy_pull}) {
            write_object(pull->storage, 0xA24 + 2 * 0x14, flags);
            write_object(pull->storage, 0xF2C, visible);
        }
        const int source_result = call_source(source_pull.source(), operation, 1002);
        const int legacy_result = call_legacy(legacy_pull.legacy(), operation, 1002);
        if (source_result != legacy_result
                || std::memcmp(source_pull.storage, legacy_pull.storage,
                               sizeof(source_pull.storage)) != 0) {
            std::fprintf(stderr, "PullDown stable-state mismatch for operation %d\n",
                         static_cast<int>(operation));
            ++failures;
        }
    }

    const int selected_indices[] = {
        -1, 0, 31, 63, -2, 64, 65, INT_MIN, INT_MAX,
    };
    for (int selected : selected_indices) {
        for (int disabled = 0; disabled < 2; ++disabled) {
            if (selected == -1 && disabled) {
                continue;
            }
            PullDownFixture source_pull{};
            PullDownFixture legacy_pull{};
            initialize(source_pull);
            initialize(legacy_pull);
            for (PullDownFixture *pull : {&source_pull, &legacy_pull}) {
                write_object(pull->storage, 0xF28, selected);
                if (selected != -1) {
                    const uint32_t offset = 0xA24U
                        + static_cast<uint32_t>(selected) * 0x14U;
                    const uint32_t flags = disabled ? 2U : 0xCU;
                    write_object(pull->storage, offset, flags);
                }
            }
            PullDownFixture selected_source_before = source_pull;
            PullDownFixture selected_legacy_before = legacy_pull;
            const int source_result = source_pull.source()->get_selected();
            const int legacy_result = legacy_pull.legacy()->get_selected();
            if (source_result != legacy_result
                    || std::memcmp(source_pull.storage,
                                   selected_source_before.storage,
                                   sizeof(source_pull.storage)) != 0
                    || std::memcmp(legacy_pull.storage,
                                   selected_legacy_before.storage,
                                   sizeof(legacy_pull.storage)) != 0) {
                std::fprintf(stderr,
                    "PullDown selection mismatch for index %d, disabled %d\n",
                    selected, disabled);
                ++failures;
            }
        }
    }

    for (MenuProc proc : {static_cast<MenuProc>(nullptr), &oracle_menu_proc}) {
        MenuFixture source_menu{};
        MenuFixture legacy_menu{};
        initialize(source_menu);
        initialize(legacy_menu);
        const MenuProc source_result = menu_set_menu_proc_redirect(
            source_menu.source(), nullptr, proc);
        const MenuProc legacy_result = legacy_menu.legacy()->set_menu_proc(proc);
        if (source_result != legacy_result || source_result != proc
                || std::memcmp(source_menu.storage, legacy_menu.storage,
                               sizeof(source_menu.storage)) != 0
                || !canaries_intact(source_menu.storage, sizeof(Menu))
                || !canaries_intact(legacy_menu.storage, sizeof(Menu))) {
            std::fprintf(stderr, "Menu callback mismatch\n");
            ++failures;
        }
    }

    struct MenuLookupFixture {
        int index;
        int id;
    };
    const MenuLookupFixture menu_lookups[] = {
        {0, INT_MIN}, {7, 17}, {14, INT_MAX},
    };
    for (const MenuLookupFixture &lookup : menu_lookups) {
        MenuFixture source_menu{};
        MenuFixture legacy_menu{};
        initialize(source_menu);
        initialize(legacy_menu);
        fill_menu_ids(source_menu);
        fill_menu_ids(legacy_menu);
        for (MenuFixture *menu : {&source_menu, &legacy_menu}) {
            write_object(menu->storage, 0xA38 + lookup.index * 0x14, lookup.id);
            write_object(menu->storage, 0xA18, INT_MIN);
        }
        MenuFixture lookup_source_before = source_menu;
        MenuFixture lookup_legacy_before = legacy_menu;
        const int source_result = source_menu.source()->id_to_index(lookup.id);
        const int legacy_result = legacy_menu.legacy()->id_to_index(lookup.id);
        if (source_result != legacy_result
                || std::memcmp(source_menu.storage,
                               lookup_source_before.storage,
                               sizeof(source_menu.storage)) != 0
                || std::memcmp(legacy_menu.storage,
                               lookup_legacy_before.storage,
                               sizeof(legacy_menu.storage)) != 0) {
            std::fprintf(stderr, "Menu lookup mismatch for index %d\n", lookup.index);
            ++failures;
        }
    }

    MenuFixture source_menu_miss{};
    MenuFixture legacy_menu_miss{};
    initialize(source_menu_miss);
    initialize(legacy_menu_miss);
    fill_menu_ids(source_menu_miss);
    fill_menu_ids(legacy_menu_miss);
    const int menu_sentinel = -1;
    const int menu_requested = 77;
    for (MenuFixture *menu : {&source_menu_miss, &legacy_menu_miss}) {
        write_object(menu->storage, 0xA38 + 2 * 0x14, menu_sentinel);
        write_object(menu->storage, 0xA38 + 3 * 0x14, menu_requested);
    }
    const int source_menu_result = source_menu_miss.source()->id_to_index(
        menu_requested);
    const int legacy_menu_result = legacy_menu_miss.legacy()->id_to_index(
        menu_requested);
    if (source_menu_result != legacy_menu_result
            || std::memcmp(source_menu_miss.storage, legacy_menu_miss.storage,
                           sizeof(source_menu_miss.storage)) != 0) {
        std::fprintf(stderr, "Menu sentinel lookup mismatch\n");
        ++failures;
    }

    return failures == 0 ? 0 : 1;
}
