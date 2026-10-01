#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>

struct RECT { int left, top, right, bottom; };
struct Font {};
typedef char *LPSTR;
static std::vector<std::string> events;
struct WriteTrace { const char *pointer; std::string value; int length; RECT rect; };
struct BoxTrace { int style; RECT rect; };
static std::vector<WriteTrace> writes;
static std::vector<int> style_trace;
static std::vector<BoxTrace> box_trace;
static void mark(const char *event) { events.push_back(event); }
static int event_index(const char *event) {
    for (size_t i = 0; i < events.size(); ++i)
        if (events[i] == event) return static_cast<int>(i);
    return -1;
}
static int event_count(const char *event) {
    int count = 0;
    for (size_t i = 0; i < events.size(); ++i)
        if (events[i] == event) ++count;
    return count;
}

struct Buffer {
    Buffer() : last_color_a(-1), last_color_b(-1), last_color2(-1),
               last_style(-1), last_write(0), last_write_length(-1),
               last_write_x(-1), last_write_y(-1), last_write_rect(),
               color_calls(0), box_calls(0) {}
    int last_color_a, last_color_b, last_color2, last_style;
    const char *last_write;
    int last_write_length, last_write_x, last_write_y;
    RECT last_write_rect;
    const RECT *last_write_rect_ptr;
    int color_calls, box_calls;
    void restore_clip() { mark("restore_clip"); }
    void box_sprite(RECT *rect, const int *style) {
        assert(rect != 0 && style != 0);
        last_style = *style;
        style_trace.push_back(*style);
        BoxTrace trace = {*style, *rect};
        box_trace.push_back(trace);
        ++box_calls;
        mark("box_sprite");
    }
    void set_font(Font *, Font *, Font *, int) { mark("set_font"); }
    void set_text_color(int a, int b, int, int) {
        last_color_a = a; last_color_b = b; ++color_calls;
        mark("set_text_color");
    }
    void set_text_color2(int c, int, int, int) {
        last_color2 = c; mark("set_text_color2");
    }
    void write_cent_l(char *text, RECT *rect, int length) {
        last_write = text; last_write_length = length;
        last_write_rect = *rect; last_write_rect_ptr = rect;
        WriteTrace trace = {text, std::string(text, text + length), length, *rect};
        writes.push_back(trace);
        mark("write_cent_l");
    }
};

struct Child {
    const char *tag;
    void show(int value) { assert(value == 0); mark(tag); }
    void hide() { mark(tag); }
};

struct Win {
    Win() { for (int i = 0; i != 150; ++i) children_[i] = 0; }
    Child *children_[150];
    virtual void show(int) { mark("win_show"); }
    virtual void hide() { mark("win_hide"); }
};

struct GraphicWin : public Win, public Buffer {
    GraphicWin() { mark("base_graphic"); }
    virtual void on_redraw() { mark("graphic_on_redraw"); }
    void redraw() { mark("graphic_redraw"); }
    int fill(RECT *, int) { mark("fill"); return 1; }
};

struct SubInterface {
    SubInterface() { mark("base_interface"); }
    virtual void on_status(int) = 0;
    virtual void on_iface_scrolled(int, int) = 0;
    virtual void on_iface_scrolling(int, int) = 0;
    virtual void on_iface_left_click(int, int) = 0;
    virtual void on_iface_right_click(int, int) = 0;
    virtual void on_iface_left_down(int, int) = 0;
    virtual void on_iface_left_up(int, int) = 0;
    virtual void on_iface_right_down(int, int) = 0;
    virtual void on_iface_mouse_move(int, int) = 0;
    virtual void on_iface_mouse_leave(int, int) = 0;
    virtual void on_iface_selected(int, int) = 0;
    virtual void on_iface_left_double_click(int, int) = 0;
    virtual void on_iface_right_double_click(int, int) = 0;
    virtual void on_iface_button_clicked(int) = 0;
    virtual int on_iface_button_pressed(int) = 0;
    virtual void on_iface_button_toggled(int, int) = 0;
    virtual void on_iface_group_clicked(int, int, int) = 0;
    virtual int on_iface_dialog_back_draw(GraphicWin *) = 0;
    virtual int on_iface_dialog_item_back_draw(GraphicWin *, int, int,
                                               RECT *) = 0;
    uint32_t field_4_;
};

struct Spot { Spot() { mark("spot"); } };
struct FlatButton {
    FlatButton() { mark("flatbutton"); }
    void show(int) { mark("button_show"); }
    void hide() { mark("button_hide"); }
};
struct StringBox {
    StringBox() { mark("stringbox_ctor"); }
    void show(int) { mark("stringbox_show"); }
    void hide() { mark("stringbox_hide"); }
};
struct ListBox { ListBox() { mark("listbox_ctor"); } };
struct EditBox {
    EditBox() { mark("editbox_ctor"); for (int i = 0; i != 150; ++i) children_[i] = 0; }
    Child *children_[150];
    void show(int value) { assert(value == 0); mark("editbox_show"); }
    void hide() { mark("editbox_hide"); }
    struct Timer {
        void start() { mark("timer_start"); }
        void stop() { mark("timer_stop"); }
    } time_;
};

#ifdef DAMAGE_BASE_ORDER
struct DiploWin : public SubInterface, public GraphicWin {
#else
struct DiploWin : public GraphicWin, public SubInterface {
#endif
    DiploWin();
    void on_redraw();
    void on_status(int);
    void on_iface_scrolled(int, int);
    void on_iface_scrolling(int, int);
    int on_iface_button_pressed(int);
    void on_iface_group_clicked(int, int, int);
    int on_iface_dialog_back_draw(GraphicWin *);
    int on_iface_dialog_item_back_draw(GraphicWin *, int, int, RECT *);
    void on_iface_left_click(int, int) {}
    void on_iface_right_click(int, int) {}
    void on_iface_left_down(int, int) {}
    void on_iface_left_up(int, int) {}
    void on_iface_right_down(int, int) {}
    void on_iface_mouse_move(int, int) {}
    void on_iface_mouse_leave(int, int) {}
    void on_iface_selected(int, int) {}
    void on_iface_left_double_click(int, int) {}
    void on_iface_right_double_click(int, int) {}
    void on_iface_button_clicked(int) {}
    void on_iface_button_toggled(int, int) {}
    int on_iface_dialog_back_draw(int) { return 9; }
    void show(int) { mark("diplo_show"); }
    int faction_id_;
    int faction_id_with_;
    Spot spot_;
#ifdef DAMAGE_MEMBER_ORDER
    StringBox stringBox_;
    FlatButton flatButtons_[13];
#else
    FlatButton flatButtons_[13];
    StringBox stringBox_;
#endif
    ListBox listBox_;
    EditBox editBox_;
    RECT portrait_rect_[2], rect_F2C_, rect_F3C_, rect_F4C_, rect_F9C_;
    RECT rect_FAC_, rect_FBC_, rect_FCC_, rect_FFC_, rect_100C_, rect_101C_;
    void draw_who() { mark("draw_who"); }
    void draw_whom() { mark("draw_whom"); }
    void draw_offers() { mark("draw_offers"); }
};

struct MainInterface : public GraphicWin {
    RECT right_frame_rect_value, rect4_value;
    RECT right_frame_rect() { return right_frame_rect_value; }
    RECT main_frame_rect() { return rect4_value; }
    void restore_back(int value) { assert(value == 1); mark("restore_back"); }
    void soft_update() { mark("soft_update"); }
};

struct FactionData { uint32_t diplo_treaties[8]; };
static MainInterface MainInterfaceGlobal;
static FactionData PlayersData[8];
static int InterfaceBoxStyles[64];
int TextColourTable[195];
static int Language;
static int MainInterfaceTileFlag;
static char StringTemp[512];
extern char DiploLocalizedTradeTitle;
extern char DiploLocalizedDiplomacyTitle;

static Font fonts[3];
static Font *find_font(int size, int style) {
    assert(size == 14 && style >= 0 && style <= 2);
    return &fonts[style];
}
static char *label_get(int index) {
    static char labels[16][16];
    static int initialized;
    if (!initialized) {
        for (int i = 0; i != 16; ++i) {
            labels[i][0] = 'L'; labels[i][1] = '0' + (i % 10);
            labels[i][2] = 0;
        }
        initialized = 1;
    }
    return labels[index % 16];
}
static void slide_show(GraphicWin *win, int value) {
    assert(win != 0 && value == 2); mark("slide_show");
}
static void slide_hide(GraphicWin *win, int value) {
    assert(win != 0 && value == 2); mark("slide_hide");
}
static int UnionRect(RECT *out, const RECT *a, const RECT *b) {
    out->left = a->left < b->left ? a->left : b->left;
    out->top = a->top < b->top ? a->top : b->top;
    out->right = a->right > b->right ? a->right : b->right;
    out->bottom = a->bottom > b->bottom ? a->bottom : b->bottom;
    return 1;
}

/* PRODUCT */

static void reset_events() { events.clear(); }
static void reset_traces() { writes.clear(); style_trace.clear(); box_trace.clear(); }
static RECT rect(int left, int top, int right, int bottom) {
    RECT result = {left, top, right, bottom}; return result;
}
static void init_globals() {
    for (int i = 0; i != 64; ++i) InterfaceBoxStyles[i] = i;
    for (int i = 0; i != 195; ++i) TextColourTable[i] = 1000 + i;
    for (int i = 0; i != 8; ++i)
        for (int j = 0; j != 8; ++j) PlayersData[i].diplo_treaties[j] = 0;
    MainInterfaceGlobal.right_frame_rect_value = rect(10, 20, 210, 120);
    MainInterfaceGlobal.rect4_value = rect(30, 40, 180, 100);
}
static void init_geometry(DiploWin &win) {
    win.portrait_rect_[0] = rect(20, 30, 90, 80);
    win.portrait_rect_[1] = rect(100, 30, 170, 80);
    win.rect_F4C_ = rect(30, 50, 160, 72);
    win.rect_FFC_ = rect(10, 20, 50, 40);
    win.rect_100C_ = rect(60, 20, 100, 40);
    win.rect_101C_ = rect(110, 20, 150, 40);
    win.rect_F9C_ = rect(10, 50, 100, 70);
    win.rect_FAC_ = rect(105, 50, 195, 70);
    win.rect_FBC_ = rect(10, 75, 100, 95);
    win.rect_FCC_ = rect(105, 75, 195, 95);
    win.rect_F2C_ = rect(2, 2, 10, 10);
    win.rect_F3C_ = rect(12, 2, 20, 10);
}
static void assert_before(const char *first, const char *second) {
    assert(event_index(first) >= 0 && event_index(second) >= 0);
    assert(event_index(first) < event_index(second));
}

int main(int argc, char **argv) {
    assert(argc == 2);
    init_globals();
    const char *scenario = argv[1];
    if (strcmp(scenario, "ctor") == 0) {
        reset_events(); reset_traces();
        DiploWin win;
        assert(event_index("base_graphic") == 0);
        assert(event_index("base_interface") == 1);
        assert(event_index("spot") < event_index("flatbutton"));
        assert(event_index("flatbutton") < event_index("stringbox_ctor"));
        assert(event_index("stringbox_ctor") < event_index("listbox_ctor"));
        assert(event_index("listbox_ctor") < event_index("editbox_ctor"));
        assert(event_count("flatbutton") == 13);
    } else if (strcmp(scenario, "painter") == 0) {
        reset_events(); reset_traces();
        DiploWin win;
        GraphicWin window;
        reset_events(); reset_traces();
        RECT source = {1, 2, 30, 40};
        assert(win.on_iface_dialog_item_back_draw(&window, 0, 3, &source) == 1);
        assert(window.last_color_a == TextColourTable[74]);
        assert(window.last_color_b == TextColourTable[75]);
        assert(window.last_style == InterfaceBoxStyles[30]);
        assert(box_trace.back().rect.left == source.left &&
               box_trace.back().rect.top == source.top &&
               box_trace.back().rect.right == source.right &&
               box_trace.back().rect.bottom == source.bottom);
        assert(source.left == 1 && source.top == 2 && source.right == 30 &&
               source.bottom == 40);
        assert(event_index("restore_clip") < event_index("set_text_color"));
        assert(event_index("set_text_color") < event_index("box_sprite"));
        int calls = window.color_calls;
        assert(win.on_iface_dialog_item_back_draw(&window, 0, 0, &source) == 1);
        assert(window.color_calls == calls + 1);
        assert(window.last_color_a == TextColourTable[80]);
        assert(window.last_color_b == TextColourTable[81]);
        assert(event_count("fill") == 1);
        calls = window.color_calls;
        int boxes = window.box_calls;
        assert(win.on_iface_dialog_item_back_draw(&window, 0, 1, 0) == 0);
        assert(win.on_iface_dialog_item_back_draw(0, 0, 1, &source) == 0);
        assert(window.color_calls == calls && window.box_calls == boxes);
    } else if (strcmp(scenario, "redraw") == 0) {
        reset_events(); reset_traces();
        DiploWin win;
        reset_events(); reset_traces();
        win.faction_id_ = 1; win.faction_id_with_ = 2;
        init_geometry(win);
        PlayersData[1].diplo_treaties[2] = 2;
        Language = 0; MainInterfaceTileFlag = 1;
        win.on_redraw();
        assert(event_index("restore_back") == 0);
        assert(event_index("draw_who") < event_index("draw_whom"));
        assert(event_index("draw_whom") < event_index("draw_offers"));
        assert(event_count("draw_who") == 1 && event_count("draw_whom") == 1);
        assert(event_count("draw_offers") == 1);
        assert(event_count("box_sprite") >= 10);
        assert(event_count("write_cent_l") >= 3);
        assert(writes.size() >= 3);
        assert(writes[0].value == "TRADE");
        assert(writes[1].value == "DIPLOMACY");
        assert(writes[2].value == "L2  (L2: {L4})");
        assert(event_count("set_text_color2") == 1);
        assert(win.last_color2 == 0xFA);
        const int expected_styles[] = {3, 3, 3, 3, 6, 3, 6, 3, 6, 0, 0};
        assert(style_trace.size() == sizeof(expected_styles) / sizeof(expected_styles[0]));
        for (size_t i = 0; i < style_trace.size(); ++i)
            assert(style_trace[i] == expected_styles[i]);
        assert(box_trace[0].rect.left == 14 && box_trace[0].rect.top == 24 &&
               box_trace[0].rect.right == 206 && box_trace[0].rect.bottom == 116);
        assert(box_trace[1].rect.left == 34 && box_trace[1].rect.top == 44 &&
               box_trace[1].rect.right == 176 && box_trace[1].rect.bottom == 96);
    } else if (strcmp(scenario, "titles") == 0) {
        reset_events(); reset_traces();
        DiploWin win;
        reset_events(); reset_traces();
        win.faction_id_ = 1; win.faction_id_with_ = 2;
        win.portrait_rect_[0] = win.portrait_rect_[1] = rect(0, 0, 1, 1);
        win.rect_F4C_ = win.rect_F9C_ = win.rect_FAC_ = win.rect_FBC_ =
            win.rect_FCC_ = win.rect_FFC_ = win.rect_100C_ = win.rect_101C_ =
            win.rect_F2C_ = win.rect_F3C_ = rect(0, 0, 1, 1);
        Language = 1; MainInterfaceTileFlag = 0;
        DiploLocalizedTradeTitle = 0;
        DiploLocalizedDiplomacyTitle = 0;
        DiploLocalizedTradeTitle = 'T';
        assert(DiploLocalizedDiplomacyTitle == 0);
        DiploLocalizedTradeTitle = 0;
        win.on_redraw();
        // Titles are the first two centered writes in this native body.
        assert(event_count("write_cent_l") >= 3);
        assert(writes.size() >= 3);
        assert(writes[0].pointer == &DiploLocalizedTradeTitle);
        assert(writes[1].pointer == &DiploLocalizedDiplomacyTitle);
        assert(writes[0].length == 0 && writes[1].length == 0);
        assert(DiploLocalizedTradeTitle == 0 && DiploLocalizedDiplomacyTitle == 0);
        assert(&DiploLocalizedTradeTitle != &DiploLocalizedDiplomacyTitle);
        assert(event_index("set_font") < event_index("write_cent_l"));
    } else if (strcmp(scenario, "status_on") == 0) {
        reset_events(); reset_traces();
        DiploWin win;
        reset_events(); reset_traces();
        init_geometry(win);
        Child child; child.tag = "child87";
        Child wrong_child; wrong_child.tag = "child86";
        win.editBox_.children_[87] = &child;
        win.editBox_.children_[86] = &wrong_child;
        win.on_status(1);
        assert_before("restore_back", "button_show");
        assert_before("button_show", "slide_show");
        assert_before("slide_show", "win_show");
        assert_before("win_show", "stringbox_show");
        assert_before("stringbox_show", "editbox_show");
        assert_before("editbox_show", "timer_start");
        assert_before("timer_start", "graphic_redraw");
        assert_before("graphic_redraw", "soft_update");
        assert(event_index("win_show") >= 0 && event_index("diplo_show") < 0);
        assert(event_count("button_show") == 13);
    } else if (strcmp(scenario, "status_off") == 0) {
        reset_events(); reset_traces();
        DiploWin win;
        Child child; child.tag = "child87";
        win.editBox_.children_[87] = &child;
        win.on_status(0);
        assert_before("stringbox_hide", "timer_stop");
        assert_before("timer_stop", "editbox_hide");
        assert_before("editbox_hide", "slide_hide");
        assert_before("slide_hide", "win_hide");
        assert_before("win_hide", "button_hide");
        assert(event_count("button_hide") == 13);
        assert(event_index("graphic_redraw") < 0);
    } else {
        assert(false);
    }
    return 0;
}
