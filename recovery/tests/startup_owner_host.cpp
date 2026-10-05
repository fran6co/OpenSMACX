#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <string>

#define __cdecl

static std::vector<std::string> events;
static void event(const char *name) { events.push_back(name); }

struct RECT { int left, top, right, bottom; };
struct BitmapHeader { long biWidth, biHeight; };
struct Dib { BitmapHeader bmiHeader; };

class Win {
public:
    const char *name;
    explicit Win(const char *n = "win") : name(n) {}
    virtual ~Win() { event((std::string("delete:") + name).c_str()); }
    void show(int visible) {
        char text[64];
        sprintf(text, "show:%s:%d", name, visible);
        event(text);
    }
};

class Buffer {
public:
    void set_font(void *font, int, int, int) {
        last_font = font;
        event("buffer.set_font");
    }
    void set_text_color(int c, int, int, int) {
        char text[64]; sprintf(text, "buffer.set_text_color:%d", c); event(text);
    }
    void *last_font;
};

class GraphicWin : public Win, public Buffer {
public:
    explicit GraphicWin(const char *n = "graphic") : Win(n) {}
    void fill(int color) {
        char text[64]; sprintf(text, "fill:%s:%d", name, color); event(text);
    }
};

struct Font {
    int unk_1_;
    int line_height_;
    int height_;
    int unk_4_;
    int unk_5_;
    int unk_6_;
    int unk_7_;
    int unk_8_;
    int line_height() const { return line_height_; }
    int pixel_height() const { return height_; }
};

class StringBox : public GraphicWin {
public:
    StringBox() : GraphicWin("message-box"), draw_mode(0), visible_rows(0),
                  x(0), y(0), width(0), height(0), owner(0), list(0), flags(0) {}
    void init(int px, int py, int w, int h, Win *parent, void *items, int f) {
        x = px; y = py; width = w; height = h; owner = parent; list = items; flags = f;
        event("stringbox.init");
    }
    void set_draw_mode(int value) { draw_mode = value; event("stringbox.set_draw_mode"); }
    void set_visible_rows(unsigned int value) { visible_rows = value; event("stringbox.set_visible_rows"); }
    void clear() { event("stringbox.clear"); }
    int draw_mode;
    unsigned int visible_rows;
    int x, y, width, height;
    Win *owner;
    void *list;
    int flags;
};

class Menu : public GraphicWin {
public:
    explicit Menu(const char *n) : GraphicWin(n) {
        rect.left = rect.top = rect.right = rect.bottom = 0;
    }
    const RECT &client_rect() const { return rect; }
    RECT rect;
};

class MainInterface : public GraphicWin {
public:
    MainInterface() : GraphicWin("main-interface") {
        title.left = title.top = title.right = title.bottom = 0;
    }
    Font *secondary_font() { return &font2; }
    Font *tertiary_font() { return &font3; }
    const RECT &title_bounds() const { return title; }
    StringBox &shared_string_box() { return box; }
    RECT title;
    Font font2;
    Font font3;
    StringBox box;
};

struct LoggedBound {
    unsigned int value;
    LoggedBound &operator=(unsigned int next) {
        value = next;
        char text[64]; sprintf(text, "left-bound:%u", next); event(text);
        return *this;
    }
    operator unsigned int() const { return value; }
};

class Console : public Win {
public:
    Console() : Win("console"), alpha_menu_("console-menu"), faction_id_(2) {}
    Menu alpha_menu_;
    Dib dib_;
    int faction_id_;
    LoggedBound field_23D80_;
    unsigned int field_23D84_;
    unsigned int field_23D88_;
    unsigned int field_23D8C_;
};

class TestWindow : public Win {
public:
    explicit TestWindow(const char *n) : Win(n) {}
    RECT rect;
    const RECT &client_rect() const { return rect; }
    void init() { event(name); }
    void init(int) { event(name); }
};

class StatusWindow : public Win { public: StatusWindow():Win("status"){} void init(){event(name);} };
class ReportWindow : public Win { public: ReportWindow():Win("report"){} void init(){event(name);} };
class PrefWindow : public Win { public: PrefWindow():Win("preferences"){} void init(){event(name);} };

MainInterface MainInterfaceGlobal;
Console ConsoleGlobal;
TestWindow ConsoleInfoWin("info");
TestWindow WorldClimateWorldWin("world");
StatusWindow ConsoleStatusWin;
TestWindow MultiWindow("multi");
TestWindow TutWinBaseWindow("base");
TestWindow TutWinDesWindow("design");
TestWindow DiplomacyPopup("diplomacy");
ReportWindow ReportIfGlobal;
PrefWindow ConsolePrefWin;
GraphicWin CoverWindow("cover");
int IsMultiplayerNet = 0;
int IsMultiplayerPBEM = 0;
int TurnCurrentNum = 0;
int WinScreenWidth = 0;
Win *BasePopDefaultField322C = 0;
GraphicWin *OpeningWindow = 0;
TestWindow InterfaceMessageWin("interface-message-window");

static void load_music(int faction) {
    assert(faction == ConsoleGlobal.faction_id_); event("load_music");
}
static void mapwin_system_init(int fresh) { assert(fresh == 7); event("mapwin_system_init"); }
static void draw_map(int mode) { assert(mode == 1); event("draw_map"); }
static void do_all_non_input() { event("do_all_non_input"); }

int __cdecl desktop_init(int fresh);
void __cdecl bung_momma();
void startup_owner_initialize_message_window();

// ACTUAL_STARTUP_BODIES

static void expect(const char *const *expected, size_t count) {
    assert(events.size() == count);
    for (size_t i = 0; i < count; ++i) {
        if (events[i] != expected[i]) {
            fprintf(stderr, "event %lu: got '%s', expected '%s'\n",
                    static_cast<unsigned long>(i), events[i].c_str(), expected[i]);
            assert(false);
        }
    }
}

static void test_desktop_init_all_paths() {
    ConsoleGlobal.alpha_menu_.rect.left = 100;
    ConsoleGlobal.alpha_menu_.rect.right = 500;
    ConsoleGlobal.dib_.bmiHeader.biWidth = 800;
    ConsoleGlobal.dib_.bmiHeader.biHeight = -600;
    MultiWindow.rect.left = 15;
    MultiWindow.rect.right = 615;
    IsMultiplayerNet = 1;
    IsMultiplayerPBEM = 1;
    TurnCurrentNum = 6;
    WinScreenWidth = 1024;
    OpeningWindow = new GraphicWin("opening");

    assert(desktop_init(7) == 0);
    assert(ConsoleGlobal.field_23D80_ == 410);
    assert(ConsoleGlobal.field_23D84_ == 190);
    assert(ConsoleGlobal.field_23D88_ == 10);
    assert(ConsoleGlobal.field_23D8C_ == 590);
    assert(OpeningWindow == 0);
    assert(BasePopDefaultField322C == static_cast<Win *>(&ConsoleGlobal));

    const char *expected[] = {
        "load_music", "info", "world", "status", "mapwin_system_init",
        "multi", "base", "design", "diplomacy", "report", "preferences",
        "left-bound:10", "left-bound:410",
        "draw_map", "show:main-interface:0", "show:console:0", "fill:cover:0",
        "show:cover:0", "do_all_non_input", "delete:opening", "do_all_non_input"
    };
    expect(expected, sizeof(expected) / sizeof(expected[0]));

    events.clear();
    IsMultiplayerNet = 0;
    IsMultiplayerPBEM = 0;
    TurnCurrentNum = 0;
    OpeningWindow = 0;
    assert(desktop_init(7) == 0);
    assert(ConsoleGlobal.field_23D84_ == 790);
    for (size_t i = 0; i < events.size(); ++i) {
        assert(events[i] != "draw_map");
        assert(events[i] != "fill:cover:0");
    }
    assert(events[events.size() - 1] == "do_all_non_input");
}

static void test_bung_momma_font_and_layout() {
    events.clear();
    WinScreenWidth = 1024;
    MainInterfaceGlobal.title.bottom = 300;
    MainInterfaceGlobal.font3.unk_1_ = -1;
    MainInterfaceGlobal.font3.line_height_ = 18;
    MainInterfaceGlobal.font3.height_ = 16;
    MainInterfaceGlobal.font2.unk_1_ = 90;
    MainInterfaceGlobal.font2.line_height_ = 101;
    MainInterfaceGlobal.font2.height_ = 102;
    bung_momma();
    StringBox &box = MainInterfaceGlobal.box;
    assert(box.x == 160 && box.y == 228 && box.width == 704 && box.height == 72);
    assert(box.owner == static_cast<Win *>(&ConsoleGlobal));
    assert(box.list == 0 && box.flags == 0x1000022);
    assert(box.last_font == &MainInterfaceGlobal.font3);
    assert(box.draw_mode == 1 && box.visible_rows == 4);
    const char *expected[] = {"stringbox.init", "stringbox.set_draw_mode",
        "stringbox.set_visible_rows", "buffer.set_font", "buffer.set_text_color:95",
        "stringbox.clear", "show:message-box:0"};
    expect(expected, sizeof(expected) / sizeof(expected[0]));

    events.clear();
    MainInterfaceGlobal.font3.unk_1_ = 4;
    bung_momma();
    assert(box.height == 80 && box.y == 220);
}

static void test_message_window_receiver() {
    events.clear();
    startup_owner_initialize_message_window();
    assert(events.size() == 1);
    assert(events[0] == "interface-message-window");
}

int main() {
    test_desktop_init_all_paths();
    test_bung_momma_font_and_layout();
    test_message_window_receiver();
}
