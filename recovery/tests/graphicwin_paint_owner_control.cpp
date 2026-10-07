#include <cassert>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

struct RECT { int left, top, right, bottom; };
class GraphicWin;
class Buffer;
typedef int (*GraphicWinCaptionDraw)(GraphicWin *, char *, Buffer *);
const unsigned char WinFillColour = 9;
std::vector<std::string> events;
GraphicWin *mutate_receiver;

static std::string point(const char *op, int id, int x, int y) {
    char text[100];
    std::sprintf(text, "%s%d@%d,%d", op, id, x, y);
    return text;
}

class Sprite {
public:
    explicit Sprite(int id, int w, int h) : id_(id), width_(w), height_(h) {}
    unsigned char transparent_index() const { return 7; }
    int width() const { return width_; }
    int height() const { return height_; }
    int draw(Buffer *, int, int x, int y, int, int) {
        events.push_back(point("D", id_, x, y));
        return 0;
    }
    int id_, width_, height_;
};

class Buffer {
public:
    Buffer(int id, int w, int h) : id_(id), width_(w), height_(h) {}
    int fill(int) {
        char text[30];
        std::sprintf(text, "F%d", id_);
        events.push_back(text);
        return 0;
    }
    int pixel_width() const { return width_; }
    int pixel_height() const { return height_; }
    int tile(Sprite *sprite, int, int, int x, int y, int w, int h) {
        char text[100];
        std::sprintf(text, "T%d@%d,%d:%d,%d", sprite->id_, x, y, w, h);
        events.push_back(text);
        return 0;
    }
    int id_, width_, height_;
};

class Win {
public:
    virtual void on_redraw_nc(RECT *area, int side) {
        (void)area;
        char text[30];
        std::sprintf(text, "BASE%d", side);
        events.push_back(text);
    }
    virtual void on_post_redraw_nc_buffer(Buffer *, int side) {
        char text[30];
        std::sprintf(text, "V%d", side);
        events.push_back(text);
    }
    virtual void on_draw_caption(char *, Buffer *) { events.push_back("CAP"); }

    unsigned int iFlags_;
    unsigned int iSomeFlag_;
    Buffer *buffer1_, *buffer2_, *buffer3_, *buffer4_;
    int caption_height_, border_thickness_, bottom_border_thickness_;
    RECT client_rect_;
    char *field_E0_;
};

class GraphicWin : public Win {
public:
    virtual void on_draw_caption(char *, Buffer *) { events.push_back("CAP"); }
    void on_redraw_nc(RECT *area, int side);

    Sprite *frame_top_left_, *frame_top_right_, *frame_bottom_left_, *frame_bottom_right_;
    Sprite *frame_top_, *frame_left_, *frame_right_, *frame_bottom_;
    Sprite *frame_noncaption_left_, *frame_noncaption_right_, *frame_noncaption_middle_;
    GraphicWinCaptionDraw field_A0C_;
};

static int caption_hook(GraphicWin *, char *text, Buffer *) {
    assert(std::strcmp(text, "title") == 0);
    events.push_back("HOOK");
    if (mutate_receiver != 0) {
        mutate_receiver->buffer4_ = 0;
    }
    return 1;
}

/* ACTUAL OWNER BODY */

static void setup(GraphicWin &win, Buffer &top, Buffer &left,
                  Buffer &right, Buffer &bottom, Sprite &tl, Sprite &tr,
                  Sprite &bl, Sprite &br, Sprite &te, Sprite &le,
                  Sprite &re, Sprite &be, Sprite &nl, Sprite &nr,
                  Sprite &nm) {
    win.iFlags_ = 0x10000010;
    win.iSomeFlag_ = 2;
    win.buffer1_ = &top;
    win.buffer2_ = &left;
    win.buffer3_ = &right;
    win.buffer4_ = &bottom;
    win.caption_height_ = 5;
    win.border_thickness_ = 2;
    win.bottom_border_thickness_ = 7;
    win.client_rect_.left = 0;
    win.client_rect_.right = 40;
    win.field_E0_ = const_cast<char *>("title");
    win.field_A0C_ = caption_hook;
    win.frame_top_left_ = &tl;
    win.frame_top_right_ = &tr;
    win.frame_bottom_left_ = &bl;
    win.frame_bottom_right_ = &br;
    win.frame_top_ = &te;
    win.frame_left_ = &le;
    win.frame_right_ = &re;
    win.frame_bottom_ = &be;
    win.frame_noncaption_left_ = &nl;
    win.frame_noncaption_right_ = &nr;
    win.frame_noncaption_middle_ = &nm;
}

static void full_paint_order_and_geometry() {
    Buffer top(1, 40, 5), left(2, 2, 20), right(3, 2, 20), bottom(4, 40, 6);
    Sprite tl(1, 4, 3), tr(2, 6, 3), bl(3, 5, 4), br(4, 6, 5);
    Sprite te(5, 2, 2), le(6, 2, 2), re(7, 2, 2), be(8, 2, 2);
    Sprite nl(9, 3, 3), nr(10, 3, 3), nm(11, 3, 3);
    GraphicWin win;
    setup(win, top, left, right, bottom, tl, tr, bl, br, te, le, re, be,
          nl, nr, nm);
    events.clear();
    mutate_receiver = 0;
    RECT area = {0, 0, 10, 10};
    win.on_redraw_nc(&area, 2);
    const char *expected[] = {
        "F1", "D1@0,0", "D2@34,0", "T5@4,0:30,5", "HOOK", "CAP", "V0",
        "F4", "D3@0,-10", "D4@34,-11", "T8@5,-8:29,2", "V2",
        "F3", "D1@0,-5", "D3@0,-17", "T6@0,-2:2,-15", "V3",
        "F2", "D2@-4,-5", "D4@-4,-18", "T7@0,-2:2,-16", "V1"
    };
    assert(events.size() == sizeof(expected) / sizeof(expected[0]));
    for (size_t i = 0; i < events.size(); ++i) {
        if (events[i] != expected[i]) {
            std::fprintf(stderr, "event %lu: got %s expected %s\n",
                         static_cast<unsigned long>(i), events[i].c_str(), expected[i]);
            assert(events[i] == expected[i]);
        }
    }
}

static void fallback_and_single_side_controls() {
    Buffer top(1, 40, 5), left(2, 2, 20), right(3, 2, 20), bottom(4, 40, 6);
    Sprite tl(1, 4, 3), tr(2, 6, 3), bl(3, 5, 4), br(4, 6, 5);
    Sprite te(5, 2, 2), le(6, 2, 2), re(7, 2, 2), be(8, 2, 2);
    Sprite nl(9, 3, 3), nr(10, 3, 3), nm(11, 3, 3);
    GraphicWin win;
    setup(win, top, left, right, bottom, tl, tr, bl, br, te, le, re, be,
          nl, nr, nm);
    RECT area = {0, 0, 10, 10};

    win.iFlags_ &= ~0x10000000u;
    events.clear();
    win.on_redraw_nc(0, 3);
    assert(events.size() == 1 && events[0] == "BASE3");

    win.iFlags_ |= 0x10000000u;
    events.clear();
    win.on_redraw_nc(0, 2);
    assert(events.size() == 5 && events[0] == "F4" && events[4] == "V2");

    events.clear();
    win.on_redraw_nc(&area, 2);
    assert(events.size() > 5 && events[0] == "F1" && events.back() == "V1");

    win.iSomeFlag_ = 0;
    events.clear();
    win.on_redraw_nc(0, -1);
    assert(events.empty());
}

static void callback_mutation_is_observed_by_later_quadrants() {
    Buffer top(1, 40, 5), left(2, 2, 20), right(3, 2, 20), bottom(4, 40, 6);
    Sprite tl(1, 4, 3), tr(2, 6, 3), bl(3, 5, 4), br(4, 6, 5);
    Sprite te(5, 2, 2), le(6, 2, 2), re(7, 2, 2), be(8, 2, 2);
    Sprite nl(9, 3, 3), nr(10, 3, 3), nm(11, 3, 3);
    GraphicWin win;
    setup(win, top, left, right, bottom, tl, tr, bl, br, te, le, re, be,
          nl, nr, nm);
    mutate_receiver = &win;
    events.clear();
    win.on_redraw_nc(0, -1);
    for (size_t i = 0; i < events.size(); ++i) assert(events[i] != "F4");
    assert(events[0] == "F1" && events[6] == "V0");
    mutate_receiver = 0;
}

int main() {
    full_paint_order_and_geometry();
    fallback_and_single_side_controls();
    callback_mutation_is_observed_by_later_quadrants();
}
