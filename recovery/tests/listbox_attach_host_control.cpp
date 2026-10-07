#include <cassert>
#include <stdint.h>
#include <vector>

enum Event {
    EVENT_DIMENSIONS = 1,
    EVENT_INIT,
    EVENT_VERT_RANGE,
    EVENT_VERT_PAGE,
    EVENT_HORZ_RANGE,
    EVENT_HORZ_PAGE,
    EVENT_HORZ_POS,
    EVENT_VERT_POS,
    EVENT_COLOR1,
    EVENT_COLOR2,
    EVENT_COLOR3,
    EVENT_FONT
};

static std::vector<int> events;
static int HandleMain = 1;

struct Font {};
struct StringList { int entry_count_; };

struct Buffer {
    void set_text_color(int a, int b, int c, int d) {
        assert(a == 11 && b == 12 && c == 13 && d == 14);
        events.push_back(EVENT_COLOR1);
    }
    void set_text_color2(int a, int b, int c, int d) {
        assert(a == 21 && b == 22 && c == 23 && d == 24);
        events.push_back(EVENT_COLOR2);
    }
    void set_text_color3(int a, int b, int c, int d) {
        assert(a == 31 && b == 32 && c == 33 && d == 34);
        events.push_back(EVENT_COLOR3);
    }
    void set_font(Font *a, Font *b, Font *c, int d) {
        assert(a != 0 && b != 0 && c != 0 && d == 0);
        events.push_back(EVENT_FONT);
    }
};

struct Win {
    uint32_t iFlags_;
    int vert_min, vert_max, vert_page;
    int horz_min, horz_max, horz_page;
    int vert_pos, horz_pos;
    Win() : iFlags_(0), vert_min(-1), vert_max(-1), vert_page(-1),
        horz_min(-1), horz_max(-1), horz_page(-1), vert_pos(-1), horz_pos(-1) {}
    bool has_dialog_focus_flag() const { return (iFlags_ & 0x1000) != 0; }
    void set_vert_range(int a, int b) {
        vert_min = a; vert_max = b; events.push_back(EVENT_VERT_RANGE);
    }
    void set_vert_paging(int a) { vert_page = a; events.push_back(EVENT_VERT_PAGE); }
    void set_horz_range(int a, int b) {
        horz_min = a; horz_max = b; events.push_back(EVENT_HORZ_RANGE);
    }
    void set_horz_paging(int a) { horz_page = a; events.push_back(EVENT_HORZ_PAGE); }
    void set_vert_pos(int a) { vert_pos = a; events.push_back(EVENT_VERT_POS); }
    void set_horz_pos(int a) { horz_pos = a; events.push_back(EVENT_HORZ_POS); }
};

struct GraphicWin : Win, Buffer {
    int init_result;
    int init_x, init_y, init_width, init_height, init_flags;
    Win *init_parent;
    GraphicWin() : init_result(0), init_x(0), init_y(0), init_width(0),
        init_height(0), init_flags(0), init_parent(0) {}
    int init(int x, int y, int width, int height, char *, int flags,
             Win *parent, void *, void *) {
        init_x = x; init_y = y; init_width = width; init_height = height;
        init_flags = flags; init_parent = parent; iFlags_ = flags;
        events.push_back(EVENT_INIT);
        return init_result;
    }
};

struct Dialog {
    uint32_t field_24_, field_28_, field_2C_, field_30_;
    uint32_t field_54_, field_58_, field_5C_, field_60_;
    StringList item_list_;
    int selected_position_;
    Font *font1_, *font2_, *font3_;
    int text_color_a_, text_color_2a_, text_color_3a_;
    int text_color_b_, text_color_2b_, text_color_3b_;
    int text_color_c_, text_color_2c_, text_color_3c_;
    int text_color_d_, text_color_2d_, text_color_3d_;
    Dialog() : field_24_(0), field_28_(0), field_2C_(2), field_30_(8),
        field_54_(1), field_58_(2), field_5C_(4), field_60_(6),
        selected_position_(0), font1_(0), font2_(0), font3_(0),
        text_color_a_(11), text_color_2a_(21), text_color_3a_(31),
        text_color_b_(12), text_color_2b_(22), text_color_3b_(32),
        text_color_c_(13), text_color_2c_(23), text_color_3c_(33),
        text_color_d_(14), text_color_2d_(24), text_color_3d_(34) {
        item_list_.entry_count_ = 5;
    }
    virtual int attach(GraphicWin *, int, int, int) { return 0; }
};

struct ListBox : virtual GraphicWin, virtual Dialog {
    uint32_t field_18_, field_1C_, field_20_;
    int dimensions_result;
    ListBox() : field_18_(0), field_1C_(3), field_20_(2),
        dimensions_result(0) {}
    int calculate_dimensions() {
        events.push_back(EVENT_DIMENSIONS);
        return dimensions_result;
    }
    int attach(GraphicWin *, int, int, int);
};

// ACTUAL_ATTACH_INSERTION

struct DerivedListBox : ListBox {
    int override_calls;
    DerivedListBox() : override_calls(0) {}
    int attach(GraphicWin *, int, int, int) {
        ++override_calls;
        return 77;
    }
    int qualified_call(GraphicWin *parent, int x, int y, int flags) {
        return ListBox::attach(parent, x, y, flags);
    }
    int virtual_call(GraphicWin *parent, int x, int y, int flags) {
        Dialog &dialog = *this;
        return dialog.attach(parent, x, y, flags);
    }
    int ordinary_call(GraphicWin *parent, int x, int y, int flags) {
        return attach(parent, x, y, flags);
    }
};

static void reset() { events.clear(); }

static void test_guards_and_failures() {
    ListBox box;
    GraphicWin parent;
    HandleMain = 0;
    assert(box.attach(0, 1, 2, 0) == 3);
    assert(events.empty());
    assert(box.attach(&parent, 1, 2, 0) == 7);
    assert(events.empty());

    HandleMain = 1;
    box.dimensions_result = 5;
    assert(box.attach(&parent, 1, 2, 0) == 5);
    assert(events.size() == 1 && events[0] == EVENT_DIMENSIONS);
    assert(box.field_18_ == 0 && box.field_1C_ == 3);

    reset();
    box.dimensions_result = 0;
    box.init_result = 9;
    assert(box.attach(&parent, 7, 8, 0) == 9);
    assert(events.size() == 2 && events[0] == EVENT_DIMENSIONS
           && events[1] == EVENT_INIT);
    assert(box.init_x == 7 && box.init_y == 8);
}

static void test_fallback_and_vertical_scroll() {
    ListBox box;
    GraphicWin parent;
    Font f1, f2, f3;
    box.font1_ = &f1; box.font2_ = &f2; box.font3_ = &f3;
    box.dimensions_result = 7;
    box.field_20_ = 8;
    box.field_1C_ = 3;
    box.selected_position_ = 6;
    box.field_18_ = 0;
    box.field_2C_ = 2; box.field_30_ = 8;
    box.field_54_ = 1; box.field_58_ = 2;
    parent.iFlags_ = 0;

    assert(box.attach(&parent, 17, 19, 0) == 0);
    assert(box.field_5C_ == 2 && box.field_60_ == 8);
    assert(box.field_54_ == 1 && box.field_58_ == 1);
    assert(box.field_24_ == 17 && box.field_28_ == 19);
    assert(box.init_width == 8 && box.init_height == 2);
    assert((box.init_flags & 0x100000) != 0);
    assert((box.init_flags & 8) != 0 && (box.init_flags & 4) == 0);
    assert(box.init_parent == static_cast<Win *>(&parent));
    assert(box.vert_min == 0 && box.vert_max == 4 && box.vert_page == 0);
    assert(box.field_18_ == 4);
    assert(box.vert_pos == 4 && box.horz_pos == 4);
    assert(events.size() == 11);
    assert(events[0] == EVENT_DIMENSIONS && events[1] == EVENT_INIT);
    assert(events[2] == EVENT_VERT_RANGE && events[3] == EVENT_VERT_PAGE);
    assert(events[4] == EVENT_VERT_POS);
    assert(events[5] == EVENT_COLOR1 && events[6] == EVENT_COLOR2
           && events[7] == EVENT_COLOR3 && events[8] == EVENT_FONT);
    assert(events[9] == EVENT_VERT_POS && events[10] == EVENT_HORZ_POS);
}

static void test_horizontal_branch_and_dispatch() {
    ListBox box;
    GraphicWin parent;
    Font f1, f2, f3;
    box.font1_ = &f1; box.font2_ = &f2; box.font3_ = &f3;
    box.field_1C_ = 3; box.field_20_ = 4;
    box.field_18_ = 6;
    box.field_54_ = 2; box.field_58_ = 3;
    box.item_list_.entry_count_ = 6;
    assert(box.attach(&parent, 3, 4, 4) == 0);
    assert((box.init_flags & 4) != 0 && (box.init_flags & 8) == 0);
    assert(box.horz_min == 0 && box.horz_max == 1 && box.horz_page == 2);
    assert(box.vert_pos == 6 && box.horz_pos == 3);

    DerivedListBox derived;
    Font df1, df2, df3;
    derived.font1_ = &df1; derived.font2_ = &df2; derived.font3_ = &df3;
    GraphicWin *derived_parent = &derived;
    assert(derived.qualified_call(derived_parent, 0, 0, 0) == 0);
    assert(derived.override_calls == 0);
    assert(derived.virtual_call(derived_parent, 0, 0, 0) == 77);
    assert(derived.override_calls == 1);
    assert(derived.ordinary_call(derived_parent, 0, 0, 0) == 77);
    assert(derived.override_calls == 2);
}

static void test_signed_capacity_comparison() {
    ListBox box;
    GraphicWin parent;
    Font f1, f2, f3;
    box.font1_ = &f1; box.font2_ = &f2; box.font3_ = &f3;
    box.field_20_ = 0xffffffffu;
    box.field_1C_ = 3;
    box.field_18_ = 7;
    box.selected_position_ = -1;
    box.field_54_ = 1;
    box.field_58_ = 1;
    box.item_list_.entry_count_ = 4;
    assert(box.attach(&parent, 0, 0, 0) == 0);
    assert((box.init_flags & 4) == 0 && (box.init_flags & 8) == 0);
    assert(box.field_18_ == 7);
    assert(box.vert_pos == 7 && box.horz_pos == 7);
}

static void test_empty_list_and_parent_flags() {
    ListBox box;
    GraphicWin parent;
    Font f1, f2, f3;
    box.font1_ = &f1; box.font2_ = &f2; box.font3_ = &f3;
    box.item_list_.entry_count_ = 0;
    box.field_18_ = 23;
    box.field_54_ = 2;
    parent.iFlags_ = 0x1000;
    assert(box.attach(&parent, -7, -9, 0x4c) == 0);
    assert(box.init_flags == 0x860);
    assert(box.field_24_ == static_cast<uint32_t>(-7));
    assert(box.field_28_ == static_cast<uint32_t>(-9));
    assert(box.field_18_ == 0 && box.vert_pos == 0 && box.horz_pos == 0);
    assert(events.size() == 8);
    assert(events[0] == EVENT_DIMENSIONS && events[1] == EVENT_INIT);
    assert(events[6] == EVENT_VERT_POS && events[7] == EVENT_HORZ_POS);
}

int main() {
    test_guards_and_failures();
    reset();
    test_fallback_and_vertical_scroll();
    reset();
    test_horizontal_branch_and_dispatch();
    reset();
    test_signed_capacity_comparison();
    reset();
    test_empty_list_and_parent_flags();
    return 0;
}
