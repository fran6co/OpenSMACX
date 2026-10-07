// Actual paint body with deterministic window, buffer and OS doubles.
#include <assert.h>
#include <string.h>
struct RECT { int left, top, right, bottom; };
enum { DDBLT_WAIT = 0x1000000 };
struct Buffer;
Buffer *WinBackBuffer, *replacement;
bool replace_back, intersection;
int WinViewOriginX = 3, WinViewOriginY = 4;
const unsigned char WinFillColour = 201;
int clips, route, wrong_paint, intersections;
int draw_x, draw_y, draw_width, draw_height;
unsigned char *table;
Buffer *draw_source, *draw_destination, *clip_targets[4];
RECT clip_areas[4], blit_area;
void finish_draw() { if (replace_back) WinBackBuffer = replacement; }
struct Surface {
    Surface *source;
    int Blt(RECT *area, Surface *from, RECT *source_area, int flags, void *effects) {
        assert(!source_area && !effects && flags == DDBLT_WAIT);
        source = from; blit_area = *area; route = 5; finish_draw(); return -1;
    }
};
typedef Surface IDirectDrawSurface2;
Surface *DirectDrawBackBuffer;
struct Buffer {
    Surface *surface_;
    RECT rect2_;
    void set_clip(RECT *area) {
        assert(clips < 4); clip_targets[clips] = this; clip_areas[clips++] = *area;
    }
    void restore_clip() { set_clip(&rect2_); }
    void observe(Buffer *destination, int x, int y, int kind) {
        draw_source = this; draw_destination = destination;
        draw_x = x; draw_y = y; route = kind; finish_draw();
    }
    int copy(Buffer *destination, int sx, int sy, int x, int y, int width, int height) {
        assert(!sx && !sy); draw_width = width; draw_height = height;
        observe(destination, x, y, 1); return 0;
    }
    int draw(Buffer *destination, int index, int x, int y, int n, int d) {
        assert(index == 201 && n == 1 && d == 1);
        observe(destination, x, y, 2); return 0;
    }
    int draw_dest(Buffer *destination, int index, int x, int y, int n, int d, unsigned char *value) {
        table = value; assert(index == 201 && n == 1 && d == 1);
        observe(destination, x, y, 3); return 0;
    }
    int draw_multi_table_dest(Buffer *destination, int index, int x, int y, int n, int d, unsigned char *value) {
        table = value; assert(index == 201 && n == 1 && d == 1);
        observe(destination, x, y, 4); return 0;
    }
};
Buffer ScreenBuffer;
struct Win {
    Win *win_parent_;
    unsigned int iFlags_, field_A0_, field_A4_;
    RECT client_rect_, outer_rect_;
    virtual void on_paint(RECT *) { ++wrong_paint; }
    void client_to_screen(int *x, int *y) { *x += 30; *y += 40; }
    void client_to_screen(RECT *area) {
        area->left += 10; area->right += 10; area->top += 20; area->bottom += 20;
    }
    void nonclient_to_screen(RECT *area) {
        area->left += 100; area->right += 100; area->top += 200; area->bottom += 200;
    }
};
struct GraphicWin : Win, Buffer { virtual void on_paint(RECT *); };
struct EditBox : GraphicWin {};
bool IntersectRect(RECT *out, const RECT *first, const RECT *second) {
    ++intersections;
    RECT a = *first;
    out->left = a.left > second->left ? a.left : second->left;
    out->top = a.top > second->top ? a.top : second->top;
    out->right = a.right < second->right ? a.right : second->right;
    out->bottom = a.bottom < second->bottom ? a.bottom : second->bottom;
    return intersection && out->left < out->right && out->top < out->bottom;
}
// LIVE_PAINT

void equal(RECT a, int left, int top, int right, int bottom) {
    assert(a.left == left && a.top == top && a.right == right && a.bottom == bottom);
}
void reset(EditBox &window) {
    clips = route = wrong_paint = intersections = 0;
    draw_source = draw_destination = 0; table = 0;
    replace_back = false; intersection = true;
    window.win_parent_ = 0; window.iFlags_ = 0;
    window.field_A0_ = window.field_A4_ = 0;
    window.surface_ = 0;
    RECT outer = { 2, 3, 19, 14 }; window.outer_rect_ = outer;
    WinBackBuffer = 0;
}

int main() {
    EditBox window;
    Win *event_receiver = &window;
    Buffer back, other;
    Surface source_surface, output_surface;
    DirectDrawBackBuffer = &output_surface;
    RECT screen_clip = { 0, 0, 640, 480 }, back_clip = { 1, 2, 60, 40 };
    ScreenBuffer.rect2_ = screen_clip; back.rect2_ = back_clip; other.rect2_ = back_clip;
    RECT damage = { 4, 5, 100, 120 };
    // Unequal dimensions and nonzero origins catch the seed's swapped copy
    // dimensions and a mistaken copy receiver/destination interpretation.
    for (int mode = 0; mode < 32; ++mode) {
        reset(window);
        bool has_surface = (mode & 1) != 0, transparent = (mode & 2) != 0;
        bool has_back = (mode & 4) != 0;
        window.surface_ = has_surface ? &source_surface : 0;
        if (transparent) window.iFlags_ |= 0x1000000;
        if (mode & 8) window.field_A0_ = 123;
        if (mode & 16) window.field_A4_ = 456;
        WinBackBuffer = has_back ? &back : 0;
        event_receiver->on_paint(&damage);
        assert(!wrong_paint && clips == 2 && clip_targets[0] == &ScreenBuffer);
        equal(clip_areas[0], 4, 5, 100, 120);
        assert(clip_targets[1] == (has_back ? &back : &ScreenBuffer));
        if (!transparent && has_surface) {
            assert(route == 5 && output_surface.source == &source_surface);
            equal(blit_area, 30, 40, 47, 51);
        } else {
            assert(draw_source == static_cast<Buffer *>(&window));
            assert(draw_destination == (has_back ? &back : &ScreenBuffer));
            assert(draw_x == (has_back ? 27 : 30) && draw_y == (has_back ? 36 : 40));
            if (!transparent) assert(route == 1 && draw_width == 17 && draw_height == 11);
            else if (mode & 8) assert(route == 3 && table == reinterpret_cast<unsigned char *>(123));
            else if (mode & 16) assert(route == 4 && table == reinterpret_cast<unsigned char *>(456));
            else assert(route == 2);
        }
    }
    Win parent;
    RECT parent_client = { 7, 9, 87, 69 }, parent_outer = { 2, 4, 102, 84 };
    parent.client_rect_ = parent_client; parent.outer_rect_ = parent_outer;
    RECT broad = { -100, -100, 500, 500 };
    for (int mode2 = 0; mode2 < 8; ++mode2) {
        reset(window); window.win_parent_ = &parent; window.iFlags_ = 0x20;
        if (mode2 & 1) window.iFlags_ |= 0x8000;
        if (mode2 & 2) WinBackBuffer = &back;
        event_receiver->on_paint(mode2 & 4 ? &broad : 0);
        assert(clips == 2 && intersections == ((mode2 & 4) ? 1 : 0));
        assert(clip_targets[0] == ((mode2 & 2) ? &back : &ScreenBuffer));
        int x = mode2 & 1 ? 100 : 10, y = mode2 & 1 ? 200 : 20;
        int width = mode2 & 1 ? 80 : 100, height = mode2 & 1 ? 60 : 80;
        if (mode2 & 2) { x -= 3; y -= 4; }
        equal(clip_areas[0], x, y, x + width, y + height);
    }
    reset(window); window.win_parent_ = &parent; window.iFlags_ = 0x20;
    intersection = false;
    event_receiver->on_paint(&broad);
    assert(intersections == 1 && !clips && !route);
    reset(window); window.win_parent_ = &parent; // Parent alone does not enable clipping.
    event_receiver->on_paint(0);
    assert(clips == 1 && route == 1);
    for (int change = 0; change < 3; ++change) {
        reset(window); WinBackBuffer = change ? &back : 0;
        replace_back = true; replacement = change == 1 ? 0 : &other;
        event_receiver->on_paint(0);
        assert(clips == 1 && clip_targets[0] == (replacement ? replacement : &ScreenBuffer));
    }
    return 0;
}
