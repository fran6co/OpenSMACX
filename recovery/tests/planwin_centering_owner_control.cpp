#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>

static std::vector<std::string> events;
static std::vector<int> zoom_history;
static std::vector<int> x_sample_indices;
static std::vector<int> y_sample_indices;
static int MapIsFlat = 0;
static int MapLongitudeBounds = 100;
static int MapLatitudeBounds = 100;

struct OffsetTable {
    int values[81];
    bool horizontal;
    int operator[](int index) const {
        if (horizontal) x_sample_indices.push_back(index);
        else y_sample_indices.push_back(index);
        return values[index];
    }
};
static OffsetTable RadiusOffsetX = {{0}, true};
static OffsetTable RadiusOffsetY = {{0}, false};

struct BitmapInfoHeader {
    int biWidth;
    int biHeight;
};
struct Dib {
    BitmapInfoHeader bmiHeader;
};

struct PlanWin;
static PlanWin *active_plan = nullptr;
static void (*dimension_hook)(PlanWin *) = nullptr;
static int xrange_calls = 0;
static int xrange_argument = 0;
static int xrange_result = 0x12345678;
static void (*xrange_hook)() = nullptr;

static int xrange(int value) {
    ++xrange_calls;
    xrange_argument = value;
    events.push_back("xrange");
    if (xrange_hook) xrange_hook();
    return xrange_result;
}

struct Buffer {
    Dib dib_;
    int copy_calls = 0;
    Buffer *copy_source = nullptr;
    int copy_x = 0;
    int copy_y = 0;
    int copy_width = 0;
    int copy_height = 0;

    int copy(Buffer *source, int x, int y, int width, int height) {
        ++copy_calls;
        copy_source = source;
        copy_x = x;
        copy_y = y;
        copy_width = width;
        copy_height = height;
        events.push_back("copy");
        return 1;
    }
};

struct GraphicWin : Buffer {
    int fill_calls = 0;
    int fill_color = 0;
    int fill(int color) {
        ++fill_calls;
        fill_color = color;
        events.push_back("fill");
        return 1;
    }
};

struct MapWin : virtual GraphicWin {
    int32_t field_1DD98_ = -14;
    int field_1DD9C_ = 0;
    int field_1DDA0_ = 0;
    int field_1DDA4_ = 0;
    int field_1DDA8_ = 0;
    int field_1DDCC_ = 100;
    int field_1DDD0_ = 100;
    int field_1DDD4_ = 0;
    int field_1DDD8_ = 0;
    uint32_t field_1DD70_ = 0;
    int calculate_calls = 0;

    void calculate_dimensions();
};

struct PlanWin : MapWin {
    Buffer buffer_;
    int draw_map_calls = 0;
    int draw_map_argument = -1;
    int back_redraw_calls = 0;
    bool change_dib_on_draw = false;
    int draw_dib_width = 0;
    int draw_dib_height = 0;

    void opticenter(int x1, int y1, int x2, int y2, int force_redraw);
    void monocenter(int x, int y, int force_redraw);

    void draw_map(int argument) {
        ++draw_map_calls;
        draw_map_argument = argument;
        events.push_back("draw_map");
        if (change_dib_on_draw) {
            dib_.bmiHeader.biWidth = draw_dib_width;
            dib_.bmiHeader.biHeight = draw_dib_height;
        }
    }
    void back_redraw() {
        ++back_redraw_calls;
        events.push_back("back_redraw");
    }
};

void MapWin::calculate_dimensions() {
    ++calculate_calls;
    zoom_history.push_back(field_1DD98_);
    events.push_back("calculate_dimensions");
    if (dimension_hook) dimension_hook(active_plan);
}

// ACTUAL_OPTICENTER
// ACTUAL_MONOCENTER

static void reset_observations() {
    events.clear();
    zoom_history.clear();
    x_sample_indices.clear();
    y_sample_indices.clear();
    xrange_calls = 0;
    xrange_argument = 0;
    dimension_hook = nullptr;
    xrange_hook = nullptr;
    MapIsFlat = 0;
    MapLongitudeBounds = 100;
    MapLatitudeBounds = 100;
    for (int index = 0; index < 81; ++index) {
        RadiusOffsetX.values[index] = 0;
        RadiusOffsetY.values[index] = 0;
    }
}

static void reset_window(PlanWin &view) {
    view.field_1DD98_ = -14;
    view.field_1DD9C_ = 10;
    view.field_1DDA0_ = 10;
    view.field_1DDA4_ = 0;
    view.field_1DDA8_ = 0;
    view.field_1DDCC_ = 100;
    view.field_1DDD0_ = 100;
    view.field_1DDD4_ = 0;
    view.field_1DDD8_ = 0;
    view.field_1DD70_ = 0;
    view.calculate_calls = 0;
    view.back_redraw_calls = 0;
    view.draw_map_calls = 0;
    view.fill_calls = 0;
    view.copy_calls = 0;
    view.buffer_.copy_calls = 0;
    view.change_dib_on_draw = false;
    view.draw_dib_width = 0;
    view.draw_dib_height = 0;
}

static void keep_full_view(PlanWin *) {}

static void fail_at_zoom_minus_ten(PlanWin *view) {
    if (view->field_1DD98_ >= -10) {
        view->field_1DDCC_ = 1;
        view->field_1DDD0_ = 1;
    } else {
        view->field_1DDCC_ = 100;
        view->field_1DDD0_ = 100;
    }
}

static void shifted_view_after_callback(PlanWin *view) {
    view->field_1DDA4_ = 50;
    view->field_1DDA8_ = 50;
    view->field_1DDCC_ = 30;
    view->field_1DDD0_ = 30;
}

static void fail_every_scan(PlanWin *view) {
    view->field_1DDCC_ = 1;
    view->field_1DDD0_ = 1;
}

static void change_longitude_during_xrange() {
    MapLongitudeBounds = 20;
}

static void check_opticenter_preamble_and_live_center() {
    reset_observations();
    PlanWin view;
    reset_window(view);
    active_plan = &view;
    dimension_hook = keep_full_view;
    view.opticenter(10, 12, 15, 16, 0);
    assert(xrange_calls == 1 && xrange_argument == 12);
    assert(events.front() == "xrange");
    assert(events[1] == "calculate_dimensions");
    assert(view.field_1DD9C_ == 10 && view.field_1DDA0_ == 12);
    assert(view.field_1DD98_ == -10);
    assert((zoom_history == std::vector<int>{-13, -12, -11, -10, -10}));
}

static void check_opticenter_rounding_threshold_and_flat_path() {
    reset_observations();
    PlanWin view;
    reset_window(view);
    active_plan = &view;
    dimension_hook = keep_full_view;
    view.opticenter(10, 10, 15, 10, 0);
    assert(xrange_calls == 1 && xrange_argument == 12);

    reset_observations();
    reset_window(view);
    active_plan = &view;
    dimension_hook = keep_full_view;
    MapLongitudeBounds = 11;
    view.opticenter(2, 10, 8, 10, 0);
    assert(xrange_calls == 0); // Delta 6 exceeds the integer half-width, 5.

    reset_observations();
    reset_window(view);
    active_plan = &view;
    dimension_hook = keep_full_view;
    MapLongitudeBounds = 20;
    MapIsFlat = 3;
    view.opticenter(2, 10, 13, 10, 0);
    assert(xrange_calls == 1 && xrange_argument == 7);

    reset_observations();
    reset_window(view);
    active_plan = &view;
    dimension_hook = keep_full_view;
    MapLongitudeBounds = 11;
    view.opticenter(2, 10, 7, 10, 0);
    assert(xrange_calls == 1 && xrange_argument == 4); // Odd delta truncates 5/2.
}

static void check_opticenter_scan_order_and_early_rejection() {
    reset_observations();
    PlanWin view;
    reset_window(view);
    active_plan = &view;
    view.field_1DDCC_ = 60;
    MapLongitudeBounds = 100;
    MapLatitudeBounds = 100;
    RadiusOffsetX.values[24] = 55;
    view.opticenter(0, 10, 10, 10, 0);
    assert(view.field_1DD98_ == -14);
    assert(x_sample_indices.size() == 50);
    assert(y_sample_indices.size() == 50);
    for (int index = 0; index < 25; ++index) {
        assert(x_sample_indices[index] == index);
        assert(x_sample_indices[index + 25] == index);
        assert(y_sample_indices[index] == index);
        assert(y_sample_indices[index + 25] == index);
    }

    reset_observations();
    reset_window(view);
    active_plan = &view;
    view.field_1DDCC_ = 30;
    view.opticenter(40, 10, 10, 10, 0);
    assert(view.field_1DD98_ == -14);
    assert(x_sample_indices.size() == 1 && x_sample_indices[0] == 0);
    assert(y_sample_indices.size() == 1 && y_sample_indices[0] == 0);
}

static void check_monocenter_81_samples_and_signed_zoom() {
    reset_observations();
    PlanWin view;
    reset_window(view);
    active_plan = &view;
    dimension_hook = fail_at_zoom_minus_ten;
    view.monocenter(10, 10, 0);
    assert(view.field_1DD98_ == -11);
    assert((zoom_history == std::vector<int>{-13, -12, -11, -10, -11}));

    reset_observations();
    reset_window(view);
    active_plan = &view;
    view.field_1DDCC_ = 60;
    RadiusOffsetX.values[80] = 50;
    view.monocenter(10, 10, 0);
    assert(view.field_1DD98_ == -14);
    assert(x_sample_indices.size() == 81 && y_sample_indices.size() == 81);
    for (int index = 0; index < 81; ++index) {
        assert(x_sample_indices[index] == index);
        assert(y_sample_indices[index] == index);
    }
}

static void check_out_of_map_skip_and_flat_wrapping() {
    reset_observations();
    PlanWin view;
    reset_window(view);
    active_plan = &view;
    view.field_1DDCC_ = 60;
    MapLongitudeBounds = 100;
    RadiusOffsetX.values[0] = 200; // One wrap leaves this sample out of map.
    view.monocenter(10, 10, 0);
    assert(view.field_1DD98_ == -10);

    reset_observations();
    reset_window(view);
    active_plan = &view;
    view.field_1DDCC_ = 60;
    MapLongitudeBounds = 100;
    view.monocenter(-1, 10, 0);
    assert(view.field_1DD98_ == -14); // Nonflat path wraps -1 to 99.

    reset_observations();
    reset_window(view);
    active_plan = &view;
    view.field_1DDCC_ = 60;
    MapLongitudeBounds = 100;
    MapIsFlat = 1;
    view.monocenter(-1, 10, 0);
    assert(view.field_1DD98_ == -10); // Flat path leaves -1 out of map.
}

static void check_live_dimensions_force_redraw_and_copy_snapshot() {
    reset_observations();
    PlanWin view;
    reset_window(view);
    active_plan = &view;
    dimension_hook = shifted_view_after_callback;
    view.monocenter(20, 20, 0);
    assert(view.field_1DD98_ == -14);
    assert(view.back_redraw_calls == 0);
    assert(x_sample_indices.size() == 1);

    reset_observations();
    reset_window(view);
    active_plan = &view;
    dimension_hook = keep_full_view;
    static_cast<GraphicWin &>(view).dib_.bmiHeader.biWidth = 321;
    static_cast<GraphicWin &>(view).dib_.bmiHeader.biHeight = -199;
    view.change_dib_on_draw = true;
    view.draw_dib_width = 432;
    view.draw_dib_height = -211;
    view.monocenter(10, 10, 1);
    assert(view.field_1DD70_ == 0x84010002U);
    assert(view.fill_calls == 1 && static_cast<GraphicWin &>(view).fill_color == 0);
    assert(view.draw_map_calls == 1 && view.draw_map_argument == 0);
    assert(view.copy_calls == 1);
    assert(view.copy_source == &view.buffer_);
    assert(view.copy_x == 0 && view.copy_y == 0);
    assert(view.copy_width == 432 && view.copy_height == 211);
    assert(events.size() == 8);
    for (int index = 0; index < 5; ++index) {
        assert(events[index] == "calculate_dimensions");
    }
    assert((std::vector<std::string>(events.begin() + 5, events.end())
            == std::vector<std::string>{"fill", "draw_map", "copy"}));

    reset_observations();
    reset_window(view);
    active_plan = &view;
    dimension_hook = fail_every_scan;
    view.field_1DD98_ = -14;
    view.field_1DD9C_ = 10;
    view.field_1DDA0_ = 10;
    view.monocenter(10, 10, 1);
    assert(view.field_1DD98_ == -14);
    assert(view.back_redraw_calls == 0);
    assert(view.copy_calls == 1 && view.fill_calls == 1 && view.draw_map_calls == 1);

    reset_observations();
    reset_window(view);
    active_plan = &view;
    dimension_hook = keep_full_view;
    view.field_1DD98_ = -10;
    view.field_1DD9C_ = 10;
    view.field_1DDA0_ = 10;
    view.monocenter(10, 10, 0);
    assert(view.buffer_.copy_calls == 0 && view.fill_calls == 0 && view.draw_map_calls == 0);
}

int main() {
    check_opticenter_preamble_and_live_center();
    check_opticenter_rounding_threshold_and_flat_path();
    check_opticenter_scan_order_and_early_rejection();
    check_monocenter_81_samples_and_signed_zoom();
    check_out_of_map_skip_and_flat_wrapping();
    check_live_dimensions_force_redraw_and_copy_snapshot();
    return 0;
}
