#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

struct RECT { int left, top, right, bottom; };
static std::vector<std::string> events;
static RECT initial_clip = {7, 8, 9, 10};
static int MapWinCaptionScale = 2;
static int MapWinCaptionDivisor = 5;
static bool MutateScaleOnColor = false;
static bool MutateStackOnButton = false;
static int VehAtResult = 0;
static int WrittenWidths[16];
static int WrittenWidthCount = 0;
static char WrittenTexts[16][256];
static void mutate_stack_if_requested();

static bool same_rect(const RECT &a, const RECT &b) {
    return a.left == b.left && a.top == b.top
        && a.right == b.right && a.bottom == b.bottom;
}
static void event(const std::string &value) { events.push_back(value); }

struct Font {
    Font() { event("font_ctor"); }
    ~Font() { event("font_dtor"); }
    void init(const char *, int, int) { event("font_init"); }
    int line_advance() const { return 12; }
};
struct Buffer {
    RECT rect1_;
    RECT rect2_;
    const RECT &current_clip() const { return rect1_; }
    void restore_clip() { set_clip(&rect2_); }
    void set_clip(RECT *rect);
    void set_clip(int, int, int, int) { event("column_clip"); }
    void tile(void *, int, int, int, int, int, int) { event("tile"); }
    void box_sprite(RECT *, void *) { event("box"); }
    void set_font(Font *, int *, int *, int *) { event("set_font"); }
    void set_text_color(int, int, int, int);
    int write_cent_l(const char *text, int, int, int width, int) {
        event("write_text");
        if (WrittenWidthCount < 16) {
            WrittenWidths[WrittenWidthCount] = width;
            std::strncpy(WrittenTexts[WrittenWidthCount], text, 255);
            WrittenTexts[WrittenWidthCount][255] = 0;
            ++WrittenWidthCount;
        }
        return 0;
    }
    void update(RECT *, void *) { event("update"); }
    void soft_update(RECT *) { event("soft_update"); }
};
struct FlatButton {
    int index;
    void hide() { event("hide" + std::to_string(index)); mutate_stack_if_requested(); }
    void show(int) { event("show" + std::to_string(index)); mutate_stack_if_requested(); }
};
struct MainInterface : Buffer {
    RECT adjacent_panel_left_rect_;
    RECT adjacent_panel_right_rect_;
    FlatButton flat_button_[11];
};
struct Spot {
    void kill_type(int type) { event("kill" + std::to_string(type)); }
    void add(int vehicle, int, int, int, int, int) {
        event("spot" + std::to_string(vehicle));
    }
};
struct Base { int x, y; };
struct Vehicle { short prev_veh_id_stack, next_veh_id_stack, proto_id; };
struct GameState { unsigned game_state_; };
struct GameData { GameState fields_; };
struct InterfaceStyle { void *mask_; };
struct BaseWin {
    RECT garrison_rect_;
    Spot spot1_;
    void draw_garrison(int update_mode);
};

static MainInterface MainInterfaceGlobal;
static BaseWin TestBaseWin;
static Base *BaseCurrent;
static Base TestBase = {0, 0};
static Vehicle Vehs[16];
static int StackHead = 0;
static char DefaultFontFace[] = "fixture-font";
static char StringTemp[256];
static int TextColourTable[128];
static GameData GameDataStateStorage;
static InterfaceStyle InterfaceBoxStyles[10];

static void mutate_stack_if_requested() {
    if (!MutateStackOnButton) return;
    MutateStackOnButton = false;
    // The button callback changes the previous-link path from the initial
    // vehicle. The pinned draw routine rewalks that path after the callbacks.
    Vehs[1].prev_veh_id_stack = 2;
    Vehs[2].prev_veh_id_stack = -1;
    Vehs[2].next_veh_id_stack = -1;
}
void Buffer::set_text_color(int, int, int, int) {
    event("text_color");
    if (MutateScaleOnColor) {
        MutateScaleOnColor = false;
        MapWinCaptionScale = 0;
        MapWinCaptionDivisor = 0;
    }
}
static int veh_at(int, int) { return VehAtResult; }
static void veh_draw(Buffer *, int vehicle, int, int, int, int, int) {
    event("draw" + std::to_string(vehicle));
}
static void say_stats_3(char *out, int proto) {
    std::sprintf(out, "unit%d", proto);
}
static char *_itoa(int value, char *out, int radix) {
    event("itoa" + std::to_string(value));
    if (radix == 10) std::sprintf(out, "%d", value);
    else std::sprintf(out, "%x", value);
    return out;
}
static char *fixture_strcat(char *destination, const char *source) {
    event(std::string("strcat:") + source);
    return std::strcat(destination, source);
}
#define strcat fixture_strcat

void Buffer::set_clip(RECT *rect) {
    if (same_rect(*rect, TestBaseWin.garrison_rect_)) event("panel_clip");
    else if (same_rect(*rect, initial_clip)) event("restore_clip");
    else if (same_rect(*rect, MainInterfaceGlobal.rect2_)) event("window_clip");
    else event("other_clip");
    rect1_ = *rect;
}

/*__DRAW_GARRISON_BODY__*/

static void reset_fixture(int update_mode, int stack_count, int initial_width,
                          int scale, int divisor) {
    events.clear();
    MainInterfaceGlobal.rect1_ = initial_clip;
    MainInterfaceGlobal.rect2_.left = 20;
    MainInterfaceGlobal.rect2_.top = 21;
    MainInterfaceGlobal.rect2_.right = 220;
    MainInterfaceGlobal.rect2_.bottom = 221;
    MainInterfaceGlobal.adjacent_panel_left_rect_.right = 10;
    MainInterfaceGlobal.adjacent_panel_right_rect_.left = 100;
    for (int i = 0; i < 11; ++i) MainInterfaceGlobal.flat_button_[i].index = i;
    TestBaseWin.garrison_rect_.left = 0;
    TestBaseWin.garrison_rect_.top = 0;
    TestBaseWin.garrison_rect_.right = initial_width;
    TestBaseWin.garrison_rect_.bottom = 80;
    BaseCurrent = &TestBase;
    StackHead = stack_count > 0 ? 0 : -1;
    VehAtResult = stack_count > 0 ? 0 : -1;
    for (int i = 0; i < 16; ++i) {
        Vehs[i].prev_veh_id_stack = -1;
        Vehs[i].next_veh_id_stack = -1;
        Vehs[i].proto_id = i;
    }
    for (int i = 0; i + 1 < stack_count; ++i) {
        Vehs[i].next_veh_id_stack = (short)(i + 1);
        Vehs[i + 1].prev_veh_id_stack = (short)i;
    }
    MapWinCaptionScale = scale;
    MapWinCaptionDivisor = divisor;
    MutateScaleOnColor = false;
    MutateStackOnButton = false;
    WrittenWidthCount = 0;
    for (int i = 0; i < 16; ++i) WrittenTexts[i][0] = 0;
    GameDataStateStorage.fields_.game_state_ = 0;
    (void)update_mode;
}

static int count_event(const std::string &needle) {
    int result = 0;
    for (std::size_t i = 0; i < events.size(); ++i)
        if (events[i] == needle) ++result;
    return result;
}
static int first_event(const std::string &needle) {
    for (std::size_t i = 0; i < events.size(); ++i)
        if (events[i] == needle) return (int)i;
    return -1;
}
static int last_event(const std::string &needle) {
    for (std::size_t i = events.size(); i > 0; --i)
        if (events[i - 1] == needle) return (int)(i - 1);
    return -1;
}
static bool require(bool condition, const char *message) {
    if (condition) return true;
    std::fprintf(stderr, "FAIL: %s\n", message);
    for (std::size_t i = 0; i < events.size(); ++i)
        std::fprintf(stderr, "  %lu %s\n", (unsigned long)i, events[i].c_str());
    return false;
}

int main(int argc, char **argv) {
    if (argc != 2) return 2;
    const int mode = std::atoi(argv[1]);
    if (mode == 0 || mode == 1) {
        // Inner width 124 => pitch 45 => capacity 2. The 3-unit stack
        // overflows into an 88-pixel panel, yielding pitch 39.
        reset_fixture(mode, 3, 130, 2, 5);
        TestBaseWin.draw_garrison(mode);
        if (!require(count_event("draw0") == 2 && count_event("draw1") == 2
                     && count_event("draw2") == 0,
                     "draw exactly the first two stack entries")) return 10;
        if (!require(first_event("hide8") < first_event("show9")
                     && first_event("show9") < first_event("show10"),
                     "overflow button ordering")) return 11;
        if (!require(first_event("panel_clip") < first_event("tile")
                     && first_event("tile") < first_event("box")
                     && first_event("box") < first_event("kill6")
                     && first_event("kill6") < first_event("column_clip"),
                     "panel setup precedes per-column painting")) return 12;
        if (!require(first_event("font_ctor") < first_event("panel_clip")
                     && first_event("panel_clip") < first_event("font_init"),
                     "Font construction precedes clip setup; init follows it")) return 13;
        const char *publication = mode == 0 ? "soft_update" : "update";
        if (!require(first_event("column_clip") < first_event("window_clip")
                     && first_event("window_clip") < first_event(publication)
                     && first_event(publication) < first_event("restore_clip"),
                     "window clip, publication, then saved-clip restoration")) return 14;
        if (!require(count_event(publication) == 1
                     && count_event("restore_clip") == 1
                     && last_event("restore_clip") < first_event("font_dtor"),
                     "one update, clip restore, then Font destruction")) return 15;
        if (!require(WrittenWidthCount == 2 && WrittenWidths[0] == 40
                     && WrittenWidths[1] == 40,
                     "text width rereads scale, separate from overflow column pitch")) return 16;
    } else if (mode == 2) {
        // Defined signed division: spacing=-40, pitch=-35, inner width=-100,
        // so capacity is +2. Positive-denominator guards incorrectly draw 0.
        reset_fixture(0, 3, -94, 2, -5);
        TestBaseWin.draw_garrison(0);
        if (!require(count_event("draw0") == 2 && count_event("draw1") == 2
                     && count_event("draw2") == 0,
                     "negative nonzero divisor follows signed arithmetic")) return 20;
    } else if (mode == 3) {
        // Nonpositive scale selects the original pitch 0x38 fallback.
        // Inner width 124 gives capacity 2; a one-unit stack hides overflow.
        reset_fixture(0, 1, 130, 0, 0);
        TestBaseWin.draw_garrison(0);
        if (!require(count_event("draw0") == 2,
                     "fallback pitch 56 gives capacity two")) return 30;
        if (!require(count_event("hide9") == 1 && count_event("hide10") == 1
                     && count_event("show9") == 0,
                     "non-overflow hides both controls")) return 31;
    } else if (mode == 4) {
        // A callback mutates the globals after column layout. The original
        // rereads them for text width after set_text_color.
        reset_fixture(0, 3, 130, 2, 5);
        MutateScaleOnColor = true;
        TestBaseWin.draw_garrison(0);
        if (!require(WrittenWidthCount == 2 && WrittenWidths[0] == 56
                     && WrittenWidths[1] == 56,
                     "text pitch reflects callback-visible global reread")) return 40;
    } else if (mode == 5) {
        // spacing is -5, so capacity division would divide by zero if it
        // were evaluated for an empty stack. Original skips it on this path.
        reset_fixture(0, 0, 130, 1, -20);
        TestBaseWin.draw_garrison(0);
        if (!require(count_event("hide8") == 0
                     && count_event("window_clip") == 1
                     && count_event("restore_clip") == 1,
                     "empty stack skips capacity divide and button callbacks")) return 50;
    } else if (mode == 6) {
        // Initial ID 1 has previous link 0 and counts as a three-item stack.
        // The hide callback changes its previous link to 2; original rewalks
        // from ID 1 after the callbacks, unlike a cached-head implementation.
        reset_fixture(0, 3, 130, 2, 5);
        VehAtResult = 1;
        MutateStackOnButton = true;
        TestBaseWin.draw_garrison(0);
        if (!require(count_event("draw2") == 2
                     && count_event("draw0") == 0 && count_event("draw1") == 0,
                     "stack head is reread after button callbacks")) return 60;
    } else if (mode == 7) {
        reset_fixture(0, 1, 130, 2, 5);
        GameDataStateStorage.fields_.game_state_ = 0x1040;
        TestBaseWin.draw_garrison(0);
        if (!require(WrittenWidthCount == 1
                     && std::strcmp(WrittenTexts[0], "unit0 (0)") == 0,
                     "debug mode appends the vehicle ID label")) return 70;
        const int space = first_event("strcat: ");
        const int opening = first_event("strcat:(");
        const int number = first_event("itoa0");
        const int closing = first_event("strcat:)");
        if (!require(space >= 0 && space < opening && opening < number
                     && number < closing,
                     "debug label preserves separate append and conversion order")) return 71;
    } else {
        return 3;
    }
    return 0;
}
