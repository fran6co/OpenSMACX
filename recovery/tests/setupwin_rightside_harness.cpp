// Test template only. The source-extraction test injects the current
// src/setupwin.cpp definition at PRODUCT_BODY_INSERTION_POINT.
#include <cassert>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

typedef uint32_t DWORD;
static std::vector<std::string> events;
static void note(const char *event) { events.push_back(event); }
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
    void write_cent_l(const char *, RECT *, int) { note("title"); }
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
    void init(int) { note("spot-init"); }
    void add(int, int, int, int, int, int) { note("spot-add"); }
};
struct Dialog {
    StringList item_list_;
    int pos_to_id(int row) { note("pos-to-id"); return 40 + row; }
};
struct Popup {
    Dialog dialog_;
    int selected_;
    Popup() : selected_(1) { dialog_.item_list_.entry_count_ = 3; }
    Dialog &dialogs() { return dialog_; }
    int value() const { return selected_; }
    const char *caption_text() const { return 0; }
};
struct Win {
    virtual ~Win() {}
    void restore_clip() { note("restore-clip"); }
};
struct GraphicWin : Win, Buffer {
    void init(int, int, int, int, void *, int, GraphicWin *, void *, void *) {
        note("graphic-init");
    }
};
struct SetupWin : GraphicWin {
    int do_menu_rightside(char *, int);
    uint32_t field_A0_;
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
    void fill(int, int, int, int, int) { note("fill"); }
    void on_update() { note("update"); }
    void soft_update() { note("soft-update"); }
    void set_modal(int, void *, void *) { note("modal"); }
    void draw_item(int, int) { note("draw-item"); }
};

static const int WinScreenWidth = 800, WinScreenHeight = 600;
static const char DefaultFontFace[] = "font";
static const int ExpansionEnabled = 0;
static char RedTintColorTable[1], BlueTintColorTable[1];
static int GamePreferences = 0;
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
        events.clear();
        assert(win.do_menu_rightside(0, 0) == -1);
        assert(index_of("flic-close") < events.size());
        assert(index_of("flic-close") < index_of("flic-dtor"));
        assert(index_of("font-dtor") < events.size());
    }
    {
        SetupWin win;
        GraphicWin parent;
        Popup popup;
        win.opening_window_snapshot_ = &parent;
        win.menu_popup_ = &popup;
        events.clear();
        assert(win.do_menu_rightside(0, 2) == 41);
        assert(win.selected_row_ == 1 && win.row_count_ == 3);
        assert(index_of("graphic-init") < index_of("modal"));
        assert(index_of("modal") < index_of("restore-clip"));
        assert(index_of("restore-clip") < index_of("flic-close"));
        assert(index_of("flic-close") < index_of("palette-dtor"));
        assert(index_of("palette-dtor") < index_of("flic-dtor"));
        assert(index_of("pos-to-id") < index_of("palette-dtor"));
    }
    {
        SetupWin win;
        GraphicWin parent;
        char filename[] = "menu";
        win.opening_window_snapshot_ = &parent;
        events.clear();
        assert(win.do_menu_rightside(filename, 0) == 0);
        assert(index_of("text-open") < index_of("text-close"));
        assert(index_of("text-close") < index_of("graphic-init"));
    }
}
