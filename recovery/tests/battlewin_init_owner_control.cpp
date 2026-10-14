// Dependency doubles for the actual mapped BattleWin initializer.
// Compile as C++98 on a host with 32-bit RECT semantics; this fixture uses
// explicit 32-bit coordinates so LP64 host `long` cannot alter the oracle.
#include <cassert>

struct RECT {
    int left;
    int top;
    int right;
    int bottom;
};

struct Buffer {
    static int constructed;
    static int destroyed;
    Buffer() { ++constructed; }
    ~Buffer() { ++destroyed; }
};
int Buffer::constructed = 0;
int Buffer::destroyed = 0;

struct MainInterface {
    RECT layout;
    RECT frame;
    const RECT &main_layout_rect() const { return layout; }
    const RECT &main_frame_rect() const { return frame; }
};
MainInterface MainInterfaceGlobal;

struct SubInterface {};
struct BattleWin : SubInterface {
    int field_4_;
    RECT battle_rects_[4];
    RECT field_70_;
    RECT field_90_;
    void init();
};
SubInterface *InterfaceRegistry[12];

/* ACTUAL OWNER BODY */

static bool equal_rect(const RECT &actual, int left, int top,
                       int right, int bottom) {
    return actual.left == left && actual.top == top
        && actual.right == right && actual.bottom == bottom;
}

static void run_case(const RECT &layout, const RECT &frame,
                     const RECT (&expected)[4]) {
    MainInterfaceGlobal.layout = layout;
    MainInterfaceGlobal.frame = frame;
    BattleWin window;
    SubInterface sentinel;
    for (int i = 0; i != 12; ++i) InterfaceRegistry[i] = &sentinel;
    window.field_4_ = -1;
    const int constructed_before = Buffer::constructed;
    const int destroyed_before = Buffer::destroyed;
    window.init();
    assert(window.field_4_ == 10);
    assert(equal_rect(window.field_70_, frame.left, frame.top,
                      frame.right, frame.bottom));
    assert(equal_rect(window.field_90_, layout.left, layout.top,
                      layout.right, layout.bottom));
    for (int i = 0; i != 4; ++i) {
        assert(equal_rect(window.battle_rects_[i], expected[i].left,
                          expected[i].top, expected[i].right,
                          expected[i].bottom));
    }
    assert(InterfaceRegistry[10] == &window);
    for (int i = 0; i != 12; ++i) {
        if (i != 10) assert(InterfaceRegistry[i] == &sentinel);
    }
    assert(Buffer::constructed == constructed_before + 1);
    assert(Buffer::destroyed == destroyed_before + 1);
}

int main() {
    const RECT layout = {11, 19, 1012, 747};
    const RECT frame = {5, 27, 803, 597};
    const RECT expected[4] = {
        {15, 23, 510, 743}, {513, 23, 1008, 743},
        {9, 31, 402, 593}, {405, 31, 799, 593}
    };
    run_case(layout, frame, expected);

    // Odd-width case catches floor/rounding changes and keeps origins nonzero.
    const RECT odd_layout = {-9, 3, 801, 303};
    const RECT odd_frame = {7, -11, 640, 481};
    const RECT odd_expected[4] = {
        {-5, 7, 394, 299}, {397, 7, 796, 299},
        {11, -7, 322, 477}, {325, -7, 636, 477}
    };
    run_case(odd_layout, odd_frame, odd_expected);
    return 0;
}
