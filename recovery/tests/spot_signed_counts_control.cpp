#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <type_traits>

typedef struct tagRECT { int left, top, right, bottom; } RECT;
#define MEASURED
#define private public
#include "spot.h"
#undef private

static int TestMemGetFail;
static void *mem_get(size_t size) {
    return TestMemGetFail ? 0 : malloc(size);
}

static_assert(std::is_same<decltype(Spot::max_count_), int>::value,
              "pinned Spot capacity is a signed int");
static_assert(std::is_same<decltype(Spot::add_count_), int>::value,
              "pinned Spot cursor is a signed int");

// @@ACTUAL_SPOT_METHODS@@

static int kill_specific_start_from_actual_expression(int add_count_) {
    return @@KILL_SPECIFIC_START@@;
}

static int get_rect_start_from_actual_expression(int add_count_) {
    return @@GET_RECT_START@@;
}

static void check_reverse_starts() {
    const int inputs[] = { 0, INT32_MIN, INT32_MIN + 1, -1 };
    const int expected[] = { -1, INT32_MAX, INT32_MIN, -2 };
    for (unsigned int i = 0; i < sizeof(inputs) / sizeof(inputs[0]); ++i) {
        assert(kill_specific_start_from_actual_expression(inputs[i]) == expected[i]);
        assert(get_rect_start_from_actual_expression(inputs[i]) == expected[i]);
    }
}

int main() {
    check_reverse_starts();

    Spot spot;
    spot.init(2);
    assert(spot.max_count_ == 2 && spot.add_count_ == 0);
    assert(spot.add(7, 3, 10, 20, 5, 6) == 0);
    assert(spot.add(8, 4, 30, 40, 7, 8) == 1);
    assert(spot.add(9, 5, 0, 0, 1, 1) == -1);

    int position = -1, type = -1;
    assert(spot.check(12, 25, &position, &type) == 0);
    assert(position == 7 && type == 3);
    spot.replace(7, 3, 11, 21, 5, 6);
    assert(spot.check(10, 25, &position, &type) == -1);
    assert(spot.check(12, 25, &position, &type) == 0);
    assert(spot.check(12, 25, &position, &type, 0) == 0);
    assert(spot.get_rect(7, 3, 0) == 0);

    // The native signed capacity comparison rejects a negative capacity.
    spot.max_count_ = -1;
    spot.add_count_ = 0;
    assert(spot.add(12, 6, 0, 0, 1, 1) == -1);

    // Negative/empty cursor paths stay on their signed early exits and do not
    // access the backing array. Zero reverse starts still return -1.
    spot.add_count_ = -1;
    spot.replace(7, 3, 1, 2, 3, 4);
    assert(spot.check(0, 0, 0, 0) == -1);
    spot.kill_pos(0);
    spot.kill_type(3);
    spot.add_count_ = 0;
    spot.kill_specific(7, 3);
    assert(spot.get_rect(7, 3, 0) == -1);
    assert(spot.check(0, 0, 0, 0) == -1);

    Spot compact;
    compact.init(3);
    assert(compact.add(1, 9, 10, 20, 2, 3) == 0);
    assert(compact.add(2, 9, 30, 40, 4, 5) == 1);
    assert(compact.add(1, 8, 50, 60, 6, 7) == 2);
    compact.kill_specific(1, 9);
    assert(compact.add_count_ == 2);
    assert(compact.spots_[0].position == 2 && compact.spots_[0].type == 9);
    assert(compact.spots_[1].position == 1 && compact.spots_[1].type == 8);
    compact.kill_type(9);
    assert(compact.add_count_ == 1 && compact.spots_[0].position == 1);
    compact.kill_pos(0);
    assert(compact.add_count_ == 0);

    Spot rect_overload;
    rect_overload.init(1);
    RECT rectangle = { 5, 6, 9, 12 };
    assert(rect_overload.add(77, 8, &rectangle) == 0);
    assert(rect_overload.add_count_ == 1);
    assert(rect_overload.spots_[0].rect.left == 5 &&
           rect_overload.spots_[0].rect.top == 6 &&
           rect_overload.spots_[0].rect.right == 9 &&
           rect_overload.spots_[0].rect.bottom == 12);

    Spot failed_init;
    failed_init.init(1);
    assert(failed_init.add(3, 4, 0, 0, 1, 1) == 0);
    TestMemGetFail = 1;
    failed_init.init(4);
    TestMemGetFail = 0;
    assert(failed_init.spots_ == 0 && failed_init.max_count_ == 0 &&
           failed_init.add_count_ == 0);
}
