#include <cassert>
#include <cstdint>
#include <vector>

struct Sprite { int id; };
struct Palette { int id; };
struct GraphicWinFrameDefaults {
    Sprite *top_left;
    Sprite *bottom_left;
    Sprite *top_right;
    Sprite *bottom_right;
    Sprite *top;
    Sprite *bottom;
    Sprite *left;
    Sprite *right;
    Sprite *noncaption_right;
    Sprite *noncaption_left;
    Sprite *noncaption_middle;
};

enum Event { CloseEvent, WinStubEvent, ComputeEvent, NonclientEvent,
             BufferInitEvent, PaletteSyncEvent };
static std::vector<Event> events;
static Sprite sprites[11];
static GraphicWinFrameDefaults GraphicWinInitDefaults;
static Palette active_palette = {1};
static Palette *PaletteActive = &active_palette;
static int ScrollDefaultThickness = 7;

struct Win {
    uint32_t iFlags_ = 0;
    int stub_result = 0;
    int stub_args[9] = {};
    int client_width_delta = 10;
    int client_height_delta = 20;

    int UNK1(int a1, int a2, int a3, int a4, int a5, int a6,
             int a7, int a8, int a9) {
        events.push_back(WinStubEvent);
        const int values[9] = {a1, a2, a3, a4, a5, a6, a7, a8, a9};
        for (int i = 0; i < 9; ++i) stub_args[i] = values[i];
        return stub_result;
    }

    void nonclient_to_client(int *width, int *height) {
        events.push_back(NonclientEvent);
        *width -= client_width_delta;
        *height -= client_height_delta;
    }
};

struct Buffer {
    void *poOwner_ = nullptr;
    int init_result = 0;
    int sync_result = 0;
    int init_width = 0;
    int init_height = 0;
    int init_arg3 = -1;
    int init_arg4 = -1;
    Palette *synced_palette = nullptr;

    int init(int width, int height, int arg3, int arg4) {
        events.push_back(BufferInitEvent);
        init_width = width;
        init_height = height;
        init_arg3 = arg3;
        init_arg4 = arg4;
        return init_result;
    }
    int sync_to_palette(Palette *palette) {
        events.push_back(PaletteSyncEvent);
        synced_palette = palette;
        return sync_result;
    }
};

class GraphicWin : public Win, public Buffer {
public:
    Sprite *frame_top_left_ = nullptr;
    Sprite *frame_top_right_ = nullptr;
    Sprite *frame_bottom_left_ = nullptr;
    Sprite *frame_bottom_right_ = nullptr;
    Sprite *frame_top_ = nullptr;
    Sprite *frame_bottom_ = nullptr;
    Sprite *frame_left_ = nullptr;
    Sprite *frame_right_ = nullptr;
    Sprite *frame_noncaption_right_ = nullptr;
    Sprite *frame_noncaption_left_ = nullptr;
    Sprite *frame_noncaption_middle_ = nullptr;
    bool compute_saw_owner = false;

    uint32_t close() {
        events.push_back(CloseEvent);
        return 0;
    }
    void compute_min_size() {
        events.push_back(ComputeEvent);
        compute_saw_owner = poOwner_ == this;
    }
    int reinitialize(int a1, int a2, int width, int height,
                     int a5, int flags, int a7, int a8, int a9);
};

// ACTUAL_GRAPHICWIN_REINITIALIZE

static void reset() { events.clear(); }

static void set_defaults() {
    GraphicWinInitDefaults = {
        &sprites[0], &sprites[1], &sprites[2], &sprites[3], &sprites[4],
        &sprites[5], &sprites[6], &sprites[7], &sprites[8], &sprites[9],
        &sprites[10]
    };
}

int main() {
    set_defaults();
    GraphicWin window;

    // All nine words are forwarded unchanged to the existing no-op contract;
    // they remain opaque except the observed width, height and flag positions.
    reset();
    window.iFlags_ = 0;
    window.client_width_delta = 10;
    window.client_height_delta = 20;
    int result = window.reinitialize(11, 22, 100, 80, 55, 0, 77, 88, 99);
    assert(result == 0);
    assert((events == std::vector<Event>{CloseEvent, WinStubEvent,
        ComputeEvent, NonclientEvent, BufferInitEvent, PaletteSyncEvent}));
    const int expected_args[9] = {11, 22, 100, 80, 55, 0, 77, 88, 99};
    for (int i = 0; i < 9; ++i) assert(window.stub_args[i] == expected_args[i]);
    assert(window.poOwner_ == &window && window.compute_saw_owner);
    assert(window.init_width == 90 && window.init_height == 60);
    assert(window.init_arg3 == 0 && window.init_arg4 == 0);
    assert(window.synced_palette == PaletteActive);

    // Either frame bit copies all eleven exact slots before the Win stub.
    reset();
    window.frame_top_left_ = nullptr;
    window.frame_top_right_ = nullptr;
    window.frame_bottom_left_ = nullptr;
    window.frame_bottom_right_ = nullptr;
    window.frame_top_ = nullptr;
    window.frame_bottom_ = nullptr;
    window.frame_left_ = nullptr;
    window.frame_right_ = nullptr;
    window.frame_noncaption_right_ = nullptr;
    window.frame_noncaption_left_ = nullptr;
    window.frame_noncaption_middle_ = nullptr;
    window.compute_saw_owner = false;
    window.stub_result = 6;
    result = window.reinitialize(1, 2, 3, 4, 5, 0x10000000, 7, 8, 9);
    assert(result == 6);
    assert((events == std::vector<Event>{CloseEvent, WinStubEvent}));
    assert(window.frame_top_left_ == &sprites[0]);
    assert(window.frame_bottom_left_ == &sprites[1]);
    assert(window.frame_top_right_ == &sprites[2]);
    assert(window.frame_bottom_right_ == &sprites[3]);
    assert(window.frame_top_ == &sprites[4]);
    assert(window.frame_bottom_ == &sprites[5]);
    assert(window.frame_left_ == &sprites[6]);
    assert(window.frame_right_ == &sprites[7]);
    assert(window.frame_noncaption_right_ == &sprites[8]);
    assert(window.frame_noncaption_left_ == &sprites[9]);
    assert(window.frame_noncaption_middle_ == &sprites[10]);
    assert(!window.compute_saw_owner);

    // The 0x800 branch uses iFlags_ bits 3/2 and the process thickness;
    // Buffer init failure is returned and prevents palette synchronization.
    reset();
    window.stub_result = 0;
    window.iFlags_ = 12;
    window.init_result = 17;
    window.sync_result = 44;
    window.synced_palette = nullptr;
    result = window.reinitialize(1, 2, 100, 80, 5, 0x30000800, 7, 8, 9);
    assert(result == 17);
    assert((events == std::vector<Event>{CloseEvent, WinStubEvent,
        ComputeEvent, BufferInitEvent}));
    assert(window.init_width == 107 && window.init_height == 87);
    assert(window.synced_palette == nullptr);

    // Successful init ignores sync_to_palette's return and returns zero.
    reset();
    window.init_result = 0;
    window.sync_result = 44;
    window.iFlags_ = 0;
    result = window.reinitialize(1, 2, 100, 80, 5, 0x800, 7, 8, 9);
    assert(result == 0);
    assert((events == std::vector<Event>{CloseEvent, WinStubEvent,
        ComputeEvent, BufferInitEvent, PaletteSyncEvent}));
    assert(window.init_width == 100 && window.init_height == 80);
}
