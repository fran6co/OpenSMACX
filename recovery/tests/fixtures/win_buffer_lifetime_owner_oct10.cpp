#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <new>

struct BorderSizing {
    int caption_height;
    int border_thickness;
    int bottom_border_thickness;
};

struct Buffer;
struct Win;
static Win *active_window;
static int allocation_count;
static int fail_allocation_at;
static int constructor_count;
static int throw_constructor_at;
static int delete_count;
static int allocation_sizes[8];
static void *allocated[8];

void *operator new(std::size_t size) {
    const int allocation = ++allocation_count;
    assert(allocation <= 8);
    allocation_sizes[allocation - 1] = static_cast<int>(size);
    if (allocation == fail_allocation_at) return 0;
    void *const memory = std::malloc(size);
    assert(memory != 0);
    allocated[allocation - 1] = memory;
    return memory;
}

void operator delete(void *memory) noexcept {
    if (memory != 0) {
        ++delete_count;
        std::free(memory);
    }
}

void operator delete(void *memory, std::size_t) noexcept {
    ::operator delete(memory);
}

struct Buffer {
    Buffer();
    ~Buffer() {}
};

struct Win {
    int caption_height_;
    int border_thickness_;
    int bottom_border_thickness_;
    int field_1A0_;
    Buffer *buffer1_;
    Buffer *buffer2_;
    Buffer *buffer3_;
    Buffer *buffer4_;

    Win()
        : caption_height_(11), border_thickness_(22),
          bottom_border_thickness_(33), field_1A0_(0), buffer1_(0),
          buffer2_(0), buffer3_(0), buffer4_(0) {}

    void close() {
        buffer1_ = 0;
        buffer2_ = 0;
        buffer3_ = 0;
        buffer4_ = 0;
    }

    int init(BorderSizing *border) {
        this->close();
        // ACTUAL_WIN_INIT_PREFIX
        return 0;
    }
};

Buffer::Buffer() {
    const int index = ++constructor_count;
    assert(index <= 4);
    assert(allocation_count == index);
    assert(active_window != 0);
    Buffer *const fields[4] = {active_window->buffer1_, active_window->buffer2_,
                               active_window->buffer3_, active_window->buffer4_};
    for (int slot = 0; slot < 4; ++slot) {
        if (slot < index - 1) assert(fields[slot] != 0);
        else assert(fields[slot] == 0);
    }
    if (index == throw_constructor_at) throw index;
}

static void reset_counts() {
    allocation_count = 0;
    fail_allocation_at = 0;
    constructor_count = 0;
    throw_constructor_at = 0;
    delete_count = 0;
    for (int i = 0; i < 8; ++i) {
        allocation_sizes[i] = 0;
        allocated[i] = 0;
    }
}

static void dispose_successful_members(Win &window, int count) {
    Buffer **const fields[4] = {&window.buffer1_, &window.buffer2_,
                                &window.buffer3_, &window.buffer4_};
    for (int i = 0; i < count; ++i) {
        delete *fields[i];
        *fields[i] = 0;
    }
}

static void test_null_returns() {
    for (int failed_at = 1; failed_at <= 4; ++failed_at) {
        reset_counts();
        Win window;
        active_window = &window;
        fail_allocation_at = failed_at;
        assert(window.init(0) == 4);
        assert(allocation_count == failed_at);
        assert(constructor_count == failed_at - 1);
        assert(delete_count == 0);
        for (int i = 0; i < allocation_count; ++i)
            assert(allocation_sizes[i] == sizeof(Buffer));
        Buffer *const fields[4] = {window.buffer1_, window.buffer2_,
                                   window.buffer3_, window.buffer4_};
        for (int slot = 0; slot < 4; ++slot) {
            if (slot < failed_at - 1) assert(fields[slot] == allocated[slot]);
            else assert(fields[slot] == 0);
        }
        dispose_successful_members(window, failed_at - 1);
        assert(delete_count == failed_at - 1);
    }
}

static void test_constructor_unwind() {
    for (int throwing_at = 1; throwing_at <= 4; ++throwing_at) {
        reset_counts();
        Win window;
        active_window = &window;
        throw_constructor_at = throwing_at;
        bool caught = false;
        try {
            window.init(0);
        } catch (int value) {
            caught = value == throwing_at;
        }
        assert(caught);
        assert(allocation_count == throwing_at);
        assert(constructor_count == throwing_at);
        assert(delete_count == 1);
        Buffer *const fields[4] = {window.buffer1_, window.buffer2_,
                                   window.buffer3_, window.buffer4_};
        for (int slot = 0; slot < 4; ++slot) {
            if (slot < throwing_at - 1) assert(fields[slot] == allocated[slot]);
            else assert(fields[slot] == 0);
        }
        dispose_successful_members(window, throwing_at - 1);
        assert(delete_count == throwing_at);
    }
}

static BorderSizing captured_sizing;
static void capture_sizing(BorderSizing *sizing) {
    assert(sizing != 0);
    captured_sizing = *sizing;
}

static void test_actual_border_local(int caption_height_, int border_thickness_,
                                    int bottom_border_thickness_) {
    // ACTUAL_BORDER_LOCAL
    capture_sizing(&sizing);
}

static void test_success_and_border_handoff() {
    reset_counts();
    Win window;
    active_window = &window;
    BorderSizing border = {101, 202, 303};
    assert(window.init(&border) == 0);
    assert(window.caption_height_ == 101);
    assert(window.border_thickness_ == 202);
    assert(window.bottom_border_thickness_ == 303);
    assert(allocation_count == 4 && constructor_count == 4 && delete_count == 0);
    assert(window.buffer1_ == allocated[0]);
    assert(window.buffer2_ == allocated[1]);
    assert(window.buffer3_ == allocated[2]);
    assert(window.buffer4_ == allocated[3]);
    for (int i = 0; i < 4; ++i) assert(allocation_sizes[i] == sizeof(Buffer));
    dispose_successful_members(window, 4);
    assert(delete_count == 4);

    reset_counts();
    Win null_border_window;
    null_border_window.caption_height_ = 44;
    null_border_window.border_thickness_ = 55;
    null_border_window.bottom_border_thickness_ = 66;
    active_window = &null_border_window;
    assert(null_border_window.init(0) == 0);
    assert(null_border_window.caption_height_ == 44);
    assert(null_border_window.border_thickness_ == 55);
    assert(null_border_window.bottom_border_thickness_ == 66);
    dispose_successful_members(null_border_window, 4);

    test_actual_border_local(101, 202, 303);
    assert(captured_sizing.caption_height == 101);
    assert(captured_sizing.border_thickness == 202);
    assert(captured_sizing.bottom_border_thickness == 303);
}

int main() {
    test_null_returns();
    test_constructor_unwind();
    test_success_and_border_handoff();
    return 0;
}
