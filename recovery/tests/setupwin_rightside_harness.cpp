// Test template only. The source-extraction test injects the current
// src/setupwin.cpp definition at PRODUCT_BODY_INSERTION_POINT.
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

typedef uint32_t DWORD;
static std::vector<std::string> events;
static void note(const char *event) { events.push_back(event); }
static void semantic_failure(const char *message) {
    std::fprintf(stderr, "SEMANTIC97: %s\n", message);
    std::exit(97);
}
static void check(bool condition, const char *message) {
    if (!condition) semantic_failure(message);
}
static int graphic_init_width;
static int graphic_init_height;
static int graphic_init_y;
static int graphic_init_calls;
static std::string last_title;
static int title_calls;
static int caption_fill_calls;
static int draw_item_calls;
static int spot_add_calls;
static int spot_init_rows;
struct RECT { int left, top, right, bottom; };
struct Header { int biHeight, biWidth; };
struct Dib { Header bmiHeader; };

struct Buffer {
    Dib dib_;
    Buffer() { dib_.bmiHeader.biHeight = -10; dib_.bmiHeader.biWidth = 354; }
    ~Buffer() { note("buffer-dtor"); }
    void close() { note("buffer-close"); }
    int init(int, int, int, void *) { note("buffer-init"); return 0; }
    void set_clip(RECT *) { note("clip"); }
    void set_text_color(int, int, int, int) { note("text-color"); }
    void set_font(void *, void *, void *, void *) { note("font"); }
    void copy(Buffer *, int, int, int, int, int, int) { note("copy"); }
    void write_cent_l(const char *text, RECT *, int) {
        ++title_calls;
        last_title = text;
        note("title");
    }
};
struct Font {
    ~Font() { note("font-dtor"); }
    int init(char *, int, int) { note("font-init"); return 0; }
};
struct Wave {
    ~Wave() { note("wave-dtor"); }
    int load(const char *) { note("wave-load"); return 0; }
    void play() { note("wave-play"); }
};
struct Flic {
    ~Flic() { note("flic-dtor"); }
    void close() { note("flic-close"); }
    int init(Buffer *, char *, int, int, int) { note("flic-init"); return 0; }
    void decode_stored_frame() { note("decode"); }
};
struct Palette { ~Palette() { note("palette-dtor"); } };
struct StringList {
    int current_position_, field_20_, field_24_, entry_count_;
    char *pending_text_;
    StringList() : current_position_(0), field_20_(0), field_24_(0),
                   entry_count_(0), pending_text_(0) {}
    void remove_all() { entry_count_ = 0; note("remove-all"); }
    void add(int) { ++entry_count_; note("list-add"); }
};
struct Spot {
    int check(int, int, int *out, int) { *out = 0; return -1; }
    void shutdown() { note("spot-shutdown"); }
    void init(int rows) { spot_init_rows = rows; note("spot-init"); }
    void add(int, int, int, int, int, int) { ++spot_add_calls; note("spot-add"); }
};
struct Dialog {
    StringList item_list_;
    int pos_to_id(int row) { note("pos-to-id"); return 40 + row; }
};
struct Popup {
    Dialog dialog_;
    int selected_;
    const char *caption_;
    Popup() : selected_(1), caption_(nullptr) { dialog_.item_list_.entry_count_ = 3; }
    Dialog &dialogs() { return dialog_; }
    int value() const { return selected_; }
    const char *caption_text() const { return caption_; }
};
struct Win {
    virtual ~Win() {}
    void restore_clip() { note("restore-clip"); }
};
struct GraphicWin : Win, Buffer {
    void init(int, int y, int width, int height, void *, int, GraphicWin *, void *, void *) {
        graphic_init_y = y;
        graphic_init_width = width;
        graphic_init_height = height;
        ++graphic_init_calls;
        note("graphic-init");
    }
};
struct SetupWin : GraphicWin {
    int do_menu_rightside(char *, int);
    void *field_A0_;
    uint32_t field_A14_, field_A18_, row_count_, selected_row_;
    Buffer buffer_, buffer2_;
    GraphicWin *opening_window_snapshot_;
    Spot spot_;
    StringList field_FE0_;
    StringList *item_texts_;
    Popup *menu_popup_;
    SetupWin() : field_A0_(0), field_A14_(0), field_A18_(0), row_count_(0),
                 selected_row_(0), opening_window_snapshot_(0), item_texts_(0),
                 menu_popup_(0) {}
    void show(int) { note("show"); }
    void hide() { note("hide"); }
    void fill(int, int, int, int, int) { ++caption_fill_calls; note("fill"); }
    void on_update() { note("update"); }
    void soft_update() { note("soft-update"); }
    void set_modal(int, void *, void *) { note("modal"); }
    void draw_item(int, int) { ++draw_item_calls; note("draw-item"); }
};

static void reset_observations() {
    events.clear();
    graphic_init_width = graphic_init_height = graphic_init_y = 0;
    graphic_init_calls = title_calls = caption_fill_calls = 0;
    draw_item_calls = spot_add_calls = spot_init_rows = 0;
    last_title.clear();
}

static void check_popup_caption(const char *caption, int rows, bool has_title) {
    SetupWin win;
    GraphicWin parent;
    Popup popup;
    char filename[128] = "original-menu-path";
    const std::string original_filename(filename);
    win.opening_window_snapshot_ = &parent;
    win.menu_popup_ = &popup;
    popup.caption_ = caption;
    popup.dialog_.item_list_.entry_count_ = rows;
    reset_observations();

    check(win.do_menu_rightside(filename, 0) == 41,
          "popup caption path retains the selected item result");
    check(std::strcmp(filename, original_filename.c_str()) == 0,
          "popup caption does not mutate filename argument");
    check(graphic_init_calls == 1 && graphic_init_width == 354,
          "popup caption geometry is initialized once at expected width");
    check(graphic_init_height == rows * 10 + (has_title ? 30 : 0),
          "popup caption height includes exactly one title row");
    check(win.row_count_ == static_cast<uint32_t>(rows + (has_title ? 1 : 0)),
          "popup caption publication increments the row count once");
    check(title_calls == (has_title ? 1 : 0),
          "popup caption controls title rendering");
    check(last_title == (has_title ? caption : ""),
          "popup caption text reaches the title writer");
    check(caption_fill_calls == (has_title ? 12 : 0),
          "popup caption row is filled in intro and outro frames");
    check(draw_item_calls == rows,
          "popup caption row is excluded from item draw count");
    check(spot_init_rows == rows + 2,
          "spot storage is based on original item count");
    check(spot_add_calls == rows,
          "spot rows exclude the title caption");
}

static const int WinScreenWidth = 800, WinScreenHeight = 600;
static const char DefaultFontFace[] = "font";
static const int ExpansionEnabled = 0;
static char RedTintColorTable[1], BlueTintColorTable[1];
struct GameDataStateMockFields { int game_preferences_; };
struct GameDataStateMock { GameDataStateMockFields fields_; };
static GameDataStateMock GameDataStateStorage = {};
static int WinFlipClipped = 0;
static int clock_tick = 0;
static uint32_t timeGetTime() { return static_cast<uint32_t>(clock_tick++); }
static int X_text_open(char *, const char *) { note("text-open"); return 0; }
[[maybe_unused]] static void text_close() { note("text-close"); }
static char *text_get() { static char empty[] = ""; return empty; }
static void purge_spaces(char *) {}
static void purge_leading(char *) {}
static int _strnicmp(const char *, const char *, size_t) { return 1; }

// PRODUCT_BODY_INSERTION_POINT

static size_t index_of(const char *value) {
    for (size_t i = 0; i < events.size(); ++i)
        if (events[i] == value) return i;
    return events.size();
}

int main() {
    {
        SetupWin win;
        reset_observations();
        check(win.do_menu_rightside(0, 0) == -1,
              "null parent snapshot returns native error result");
        check(index_of("flic-close") < events.size(),
              "early return explicitly closes Flic");
        check(index_of("flic-close") < index_of("flic-dtor"),
              "early Flic cleanup precedes destructor");
        check(index_of("font-dtor") < events.size(),
              "early return destroys Font local");
    }
    {
        SetupWin win;
        GraphicWin parent;
        Popup popup;
        win.opening_window_snapshot_ = &parent;
        win.menu_popup_ = &popup;
        reset_observations();
        check(win.do_menu_rightside(0, 2) == 41,
              "popup selection returns mapped dialog id");
        check(win.selected_row_ == 1 && win.row_count_ == 3,
              "popup selection and item count are preserved");
        check(index_of("graphic-init") < index_of("modal"),
              "window initialization precedes modal entry");
        check(index_of("modal") < index_of("restore-clip"),
              "modal close precedes parent clip restoration");
        check(index_of("restore-clip") < index_of("flic-close"),
              "parent clip restoration precedes Flic cleanup");
        check(index_of("flic-close") < index_of("palette-dtor"),
              "Flic explicit cleanup precedes Palette destruction");
        check(index_of("palette-dtor") < index_of("flic-dtor"),
              "Palette scope ends before outer locals");
        check(index_of("pos-to-id") < index_of("palette-dtor"),
              "popup result lookup occurs before Palette destruction");
    }
    {
        SetupWin win;
        GraphicWin parent;
        char filename[] = "menu";
        win.opening_window_snapshot_ = &parent;
        reset_observations();
        check(win.do_menu_rightside(filename, 0) == 0,
              "script menu returns selected row");
        check(index_of("text-open") < index_of("text-close"),
              "script closes text after reading");
        check(index_of("text-close") < index_of("graphic-init"),
              "script text closes before menu window initialization");
    }
    check_popup_caption(nullptr, 0, false);
    check_popup_caption("", 3, false);
    check_popup_caption("A title", 0, true);
    check_popup_caption("A title", 3, true);
}
