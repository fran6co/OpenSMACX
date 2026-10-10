#include <assert.h>
#include <stdint.h>
#include <vector>

#define __cdecl

enum Event {
    EVENT_VISIBLE = 1,
    EVENT_SELECTED_ID,
    EVENT_DIMENSIONS,
    EVENT_SET_SELECTED,
    EVENT_ATTACH,
    EVENT_RELEASE,
    EVENT_SHOW,
    EVENT_VERT_RANGE,
    EVENT_VERT_PAGE,
    EVENT_HORZ_RANGE,
    EVENT_HORZ_PAGE,
    EVENT_CALLBACK,
    EVENT_PARENT_NOTIFY,
    EVENT_REDRAW
};

static std::vector<int> events;
static int HandleMain = 1;
static int release_calls;
static int callback_calls;
static int callback_value;

static void __cdecl selection_callback(int selected_id) {
    ++callback_calls;
    callback_value = selected_id;
    events.push_back(EVENT_CALLBACK);
}

static void ReleaseCapture() {
    ++release_calls;
    events.push_back(EVENT_RELEASE);
}

struct Win {
    uint32_t iFlags_;
    int visible_;
    int vert_min_, vert_max_, vert_page_;
    int horz_min_, horz_max_, horz_page_;
    int vslot_calls_, vslot_id_;
    Win *parent_values_[8];
    int parent_value_count_;
    int parent_reads_;
    char child_padding_[0x68];
    Win *child_;

    Win() : iFlags_(0), visible_(0), vert_min_(-1), vert_max_(-1),
        vert_page_(-1), horz_min_(-1), horz_max_(-1), horz_page_(-1),
        vslot_calls_(0), vslot_id_(-1), parent_value_count_(0),
        parent_reads_(0), child_(0) {}
    bool is_visible() { events.push_back(EVENT_VISIBLE); return visible_ != 0; }
    void show(int value) { assert(value == 0); events.push_back(EVENT_SHOW); }
    void set_vert_range(int a, int b) {
        vert_min_ = a; vert_max_ = b; events.push_back(EVENT_VERT_RANGE);
    }
    void set_vert_paging(int a) { vert_page_ = a; events.push_back(EVENT_VERT_PAGE); }
    void set_horz_range(int a, int b) {
        horz_min_ = a; horz_max_ = b; events.push_back(EVENT_HORZ_RANGE);
    }
    void set_horz_paging(int a) { horz_page_ = a; events.push_back(EVENT_HORZ_PAGE); }
    void on_selected(int selected_id) {
        ++vslot_calls_; vslot_id_ = selected_id; events.push_back(EVENT_PARENT_NOTIFY);
    }
    Win *parent_window() {
        int index = parent_reads_++;
        if (index < parent_value_count_) return parent_values_[index];
        return 0;
    }
    Win *legacy_window_36() { return child_; }
};

struct GraphicWin : Win {
    int redraw_calls_;
    GraphicWin() : redraw_calls_(0) {}
    void set_parent(Win *first, Win *second, Win *third) {
        parent_values_[0] = first; parent_values_[1] = second;
        parent_values_[2] = third; parent_value_count_ = 3;
    }
    void redraw() { ++redraw_calls_; events.push_back(EVENT_REDRAW); }
};

struct StringList { int entry_count_; };
struct Dialog {
    StringList item_list_;
    int selected_position_;
    uint32_t field_24_, field_28_, field_54_, field_58_;
    int selected_id_calls_;
    Dialog() : selected_position_(0), field_24_(12), field_28_(13),
        field_54_(2), field_58_(3), selected_id_calls_(0) {
        item_list_.entry_count_ = 6;
    }
    int get_selected_id() {
        ++selected_id_calls_;
        events.push_back(EVENT_SELECTED_ID);
        return 100 + selected_id_calls_;
    }
};

struct OriginalObject {};
struct VCall { void slot007() {} };
template<class Method>
Method vtable_method(const void *, unsigned long) { return Method(); }
#define ORIGINAL(pointer) (reinterpret_cast<OriginalObject *>(pointer))
typedef void (__cdecl *WinNotificationCallback)(int);

struct ListBox : virtual GraphicWin, virtual Dialog {
    uint32_t field_C_;
    int attach_result_;
    int attach_calls_;
    GraphicWin *attach_parent_;
    int attach_a2_, attach_a3_, attach_a4_;
    int dimensions_updates_;
    int selected_position_after_set_;
    int selected_notify_after_set_;
    ListBox() : field_C_(4), attach_result_(0), attach_calls_(0),
        attach_parent_(0), attach_a2_(0), attach_a3_(0), attach_a4_(0),
        dimensions_updates_(-1), selected_position_after_set_(-99),
        selected_notify_after_set_(-99) {
        selection_callback_ = 0;
    }
    WinNotificationCallback selection_callback_;
    void set_callbacks(void (__cdecl *callback)(int)) { selection_callback_ = callback; }
    int calculate_dimensions() {
        events.push_back(EVENT_DIMENSIONS);
        if (dimensions_updates_ >= 0) field_C_ = dimensions_updates_;
        return 0;
    }
    void set_selected_pos(int position, int notify) {
        selected_position_after_set_ = position;
        selected_notify_after_set_ = notify;
        events.push_back(EVENT_SET_SELECTED);
    }
    virtual int attach(GraphicWin *parent, int a2, int a3, int a4) {
        ++attach_calls_; attach_parent_ = parent;
        attach_a2_ = a2; attach_a3_ = a3; attach_a4_ = a4;
        events.push_back(EVENT_ATTACH);
        return attach_result_;
    }
    int update_changes();
};

// UPDATE_METHOD_INSERTION

struct DerivedListBox : ListBox {
    int derived_attach_calls_;
    DerivedListBox() : derived_attach_calls_(0) {}
    int attach(GraphicWin *, int, int, int) {
        ++derived_attach_calls_;
        return 88;
    }
};

static void reset_observations() {
    events.clear();
    release_calls = 0;
    callback_calls = 0;
    callback_value = -1;
}

static void test_main_guard() {
    ListBox box;
    HandleMain = 0;
    assert(box.update_changes() == 7);
    assert(events.empty());
    assert(box.selected_id_calls_ == 0 && box.attach_calls_ == 0);
    HandleMain = 1;
}

static void test_equal_count_callback_reload_and_vertical_precedence() {
    ListBox box;
    GraphicWin parent;
    box.set_parent(&parent, &parent, &parent);
    box.visible_ = 0;
    box.iFlags_ = 0xC; // Both bits set: pinned order selects vertical only.
    box.field_C_ = 6;
    box.dimensions_updates_ = 6;
    box.selected_position_ = -1;
    box.set_callbacks(selection_callback);

    assert(box.update_changes() == 0);
    assert(box.attach_calls_ == 0);
    assert(box.selected_position_after_set_ == -1);
    assert(box.selected_notify_after_set_ == 1);
    assert(box.selected_id_calls_ == 3); // discard, callback, parent notification
    assert(callback_calls == 1 && callback_value == 102);
    assert(parent.vslot_calls_ == 1 && parent.vslot_id_ == 103);
    assert(box.parent_reads_ == 2); // parent is reloaded before both dispatches
    assert(box.vert_min_ == 0 && box.vert_max_ == 5 && box.vert_page_ == 2);
    assert(box.horz_min_ == -1 && box.redraw_calls_ == 0);
    assert(release_calls == 0);
    assert(events[0] == EVENT_VISIBLE && events[1] == EVENT_SELECTED_ID
           && events[2] == EVENT_DIMENSIONS && events[3] == EVENT_SET_SELECTED);
    assert(events[4] == EVENT_VERT_RANGE && events[5] == EVENT_VERT_PAGE);
}

static void test_changed_count_attach_failure_stops_tail() {
    DerivedListBox box;
    GraphicWin parent;
    box.set_parent(&parent, &parent, &parent);
    box.visible_ = 1;
    box.iFlags_ = 8;
    box.field_C_ = 2;
    box.dimensions_updates_ = 3;
    box.selected_position_ = -1;
    box.attach_result_ = 47;
    box.set_callbacks(selection_callback);

    assert(box.update_changes() == 47);
    assert(box.attach_calls_ == 1);
    assert(box.derived_attach_calls_ == 0); // qualified direct ListBox target
    assert(box.attach_parent_ == &parent);
    assert(box.attach_a2_ == 12 && box.attach_a3_ == 13);
    assert(box.attach_a4_ == 8);
    assert(box.selected_position_after_set_ == -99);
    assert(box.selected_id_calls_ == 1);
    assert(box.parent_reads_ == 1);
    assert(callback_calls == 0 && parent.vslot_calls_ == 0);
    assert(release_calls == 0 && box.redraw_calls_ == 0);
    assert(events.size() == 4 && events[0] == EVENT_VISIBLE
           && events[1] == EVENT_SELECTED_ID
           && events[2] == EVENT_DIMENSIONS && events[3] == EVENT_ATTACH);
}

static void test_changed_count_success_saved_minus_one_and_visible_redraw() {
    ListBox box;
    GraphicWin parent;
    box.set_parent(&parent, &parent, &parent);
    box.visible_ = 1;
    box.iFlags_ = 8;
    box.field_C_ = 1;
    box.dimensions_updates_ = 2;
    box.selected_position_ = -1;
    box.set_callbacks(selection_callback);

    assert(box.update_changes() == 0);
    assert(box.attach_calls_ == 1 && box.attach_result_ == 0);
    assert(box.selected_position_after_set_ == -1);
    assert(box.selected_notify_after_set_ == 1);
    assert(release_calls == 1);
    assert(box.redraw_calls_ == 1);
    assert(box.parent_reads_ == 3);
    assert(box.selected_id_calls_ == 3);
    assert(callback_calls == 1 && callback_value == 102);
    assert(parent.vslot_calls_ == 1 && parent.vslot_id_ == 103);
    assert(events[3] == EVENT_ATTACH && events[4] == EVENT_SET_SELECTED);
    assert(events[5] == EVENT_RELEASE && events[6] == EVENT_SHOW);
    assert(events[events.size() - 1] == EVENT_REDRAW);
}

static void test_horizontal_without_callbacks_or_parent() {
    ListBox box;
    box.iFlags_ = 4;
    box.selected_position_ = 4;
    assert(box.update_changes() == 0);
    assert(box.selected_position_after_set_ == 4);
    assert(box.horz_min_ == 0 && box.horz_max_ == 1 && box.horz_page_ == 2);
    assert(box.vert_min_ == -1 && box.vert_page_ == -1);
    assert(box.selected_id_calls_ == 1 && box.parent_reads_ == 2);
    assert(callback_calls == 0 && release_calls == 0 && box.redraw_calls_ == 0);
    assert(events.size() == 6);
    assert(events[4] == EVENT_HORZ_RANGE && events[5] == EVENT_HORZ_PAGE);
}

static void test_changed_count_positive_selection_resets_to_zero() {
    ListBox box;
    GraphicWin parent;
    box.set_parent(&parent, 0, 0);
    box.selected_position_ = 4;
    box.dimensions_updates_ = 7;
    assert(box.update_changes() == 0);
    assert(box.attach_calls_ == 1 && box.attach_parent_ == &parent);
    assert(box.selected_position_after_set_ == 0);
    assert(box.selected_notify_after_set_ == 1 && release_calls == 1);
    assert(box.selected_id_calls_ == 1 && box.parent_reads_ == 3);
    assert(callback_calls == 0 && parent.vslot_calls_ == 0);
    assert(box.redraw_calls_ == 0);
}

int main() {
    test_main_guard();
    reset_observations();
    test_equal_count_callback_reload_and_vertical_precedence();
    reset_observations();
    test_changed_count_attach_failure_stops_tail();
    reset_observations();
    test_changed_count_success_saved_minus_one_and_visible_redraw();
    reset_observations();
    test_horizontal_without_callbacks_or_parent();
    reset_observations();
    test_changed_count_positive_selection_resets_to_zero();
    return 0;
}
