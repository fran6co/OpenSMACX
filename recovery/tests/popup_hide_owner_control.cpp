// C++98 controls for source-owned Popup hide helpers; not native/game acceptance.
#include <vector>
#include <cassert>
#define __cdecl

struct RECT { int left, top, right, bottom; };
typedef unsigned int DWORD;
static int clock_ticks;
static DWORD timeGetTime() { return static_cast<DWORD>(clock_ticks++); }
static int sound_calls;
static int MapMouseFeedbackBusy;
static int ConsoleControlTurnActive;
static int WindowAnimationFrameDelay = 2;
static int mutate_wait_in_sound;
static int mutate_horizontal_step_on_flip;
static int PopupScrollHorizontalStep;
static int WinFlipClipped;
static int fade_flip_state;
static void do_sound();

enum { PREF_AV_SOUND_EFFECTS = 0x400, PREF_AV_SLIDING_WINDOWS = 0x8000000,
       MPREF_AV_SLIDING_SCROLLBARS = 0x80000 };
struct Buffer;
struct Event { int op, source, target, a, b, c, d, e; };
static std::vector<Event> events;
static int next_id = 1;
static int op_copy = 1, op_draw = 2, op_fill = 3, op_hide = 4, op_clip = 5;
static int op_init = 6, op_update_window = 7, op_update_back = 8, op_flip = 9;
static int op_graphic_fill = 16, op_graphic_fill_rect = 17, op_fade = 18;
static int flip_count;
static void record(int op, int source, int target, int a, int b, int c, int d, int e=0) {
    Event ev; ev.op=op; ev.source=source; ev.target=target;
    ev.a=a; ev.b=b; ev.c=c; ev.d=d; ev.e=e; events.push_back(ev);
}

struct Buffer {
    int id, width, height;
    RECT clip, current_clip;
    Buffer() : id(next_id++), width(120), height(80) {
        clip.left = clip.top = current_clip.left = current_clip.top = 0;
        clip.right = current_clip.right = width;
        clip.bottom = current_clip.bottom = height;
        record(10, id, 0, 0, 0, 0, 0);
    }
    ~Buffer() { record(11, id, 0, 0, 0, 0, 0); }
    int pixel_width() const { return width; }
    int pixel_height() const { return height; }
    const RECT &clip_rect() const { return clip; }
    void set_clip(RECT *r) {
        current_clip = *r;
        record(op_clip, id, 0, r->left, r->top, r->right, r->bottom);
    }
    void init(int w, int h, int, int) {
        width = w; height = h < 0 ? -h : h;
        record(op_init, id, 0, width, height, 0, 0);
    }
    void resize(int w, int h) { width = w; height = h; }
    int fill(int color) {
        record(op_fill, id, 0, color, 0, 0, 0); return 0;
    }
    int fill(int x, int y, int w, int h, int color) {
        // Filled by native Buffer::fill(x,y,w,h,color).
        record(op_fill, id, 0, x, y, w, h, color); return 0;
    }
    int copy(Buffer *dst, int sx, int sy, int dx, int dy, int w, int h) {
        record(op_copy, id, dst->id, sx, sy, dx, dy);
        record(12, id, dst->id, w, h, 0, 0); return 0;
    }
    int draw(Buffer *dst, int, int x, int y, int, int) {
        record(op_draw, id, dst->id, x, y, 0, 0); return 0;
    }
};

struct Win {
    int visible;
    Win() : visible(1) {}
    int is_visible() const { return visible; }
    void client_to_screen(RECT *r) { r->left += 100; r->right += 100; r->top += 50; r->bottom += 50; }
    void screen_to_client(RECT *r) { r->left -= 100; r->right -= 100; r->top -= 50; r->bottom -= 50; }
    void update_window_to_buffer(Buffer *b) { record(op_update_window, b->id, 0, 0, 0, 0, 0); }
    void update_back_to_window(Buffer *b) { record(op_update_back, b->id, 0, 0, 0, 0, 0); }
    virtual void hide() { visible = 0; record(op_hide, 0, 0, 0, 0, 0, 0); }
    static void flip(RECT *) {
        ++flip_count; record(op_flip, 0, 0, 0, 0, 0, 0);
        if (mutate_horizontal_step_on_flip && flip_count==1)
            PopupScrollHorizontalStep=7;
    }
};
struct GraphicWin : Win, Buffer {
    int fill(int x, int y, int w, int h, int color) {
        record(op_graphic_fill_rect, static_cast<Buffer *>(this)->id,
               0, x, y, w, h, color); return 0;
    }
    int fill(RECT *r, int color) {
        record(op_graphic_fill_rect, static_cast<Buffer *>(this)->id,
               0, r->left, r->top, r->right, r->bottom, color); return 0;
    }
    void fill(int color) {
        record(op_graphic_fill, static_cast<Buffer *>(this)->id,
               0, color, 0, 0, 0); 
    }
};

struct Sprite {
    int w, h, tag;
    Sprite(int width = 16, int height = 20) : w(width), h(height), tag(-1) {}
    int width() const { return w; }
    int height() const { return h; }
    int transparent_index() const { return 255; }
    int draw(Buffer *dst, int, int x, int y, int, int) {
        record(op_draw, tag, dst->id, x, y, 0, 0); return 0;
    }
};
struct MainInterface : GraphicWin {
    Sprite slide[4], up, down;
    RECT bounds, layout, outer, info;
    MainInterface() {
        bounds.left=4; bounds.top=6; bounds.right=500; bounds.bottom=700;
        layout.left=0; layout.top=0; layout.right=800; layout.bottom=768;
        outer.left=0; outer.top=0; outer.right=800; outer.bottom=768;
        info.left=0; info.top=0; info.right=700; info.bottom=600;
        for (int i=0; i<4; ++i) slide[i].tag=200+i;
        up.tag=210; down.tag=211;
        width=800; height=768; clip.right=current_clip.right=800;
        clip.bottom=current_clip.bottom=768;
    }
    Sprite &slide_frame_sprite(int i) { return slide[i]; }
    Sprite &interface_up_sprite() { return up; }
    Sprite &interface_down_sprite() { return down; }
    const RECT &bottom_bounds() const { return bounds; }
    const RECT &main_layout_rect() const { return layout; }
    const RECT &info_frame_rect() const { return info; }
    const RECT &outer_rect() const { return outer; }
        void update_window_to_buffer(Buffer *b) { record(op_update_window, b->id, 0, 0, 0, 0, 0); }
    void restore_back(int) {}
    void soft_update() {}
};
struct MapWin : virtual GraphicWin {};
struct Console : MapWin {};
struct Scroll : GraphicWin {
    void resize(int w, int h, int) { width=w; height=h; }
    void move(int, int) {}
};

static MainInterface MainInterfaceGlobal;
static Console ConsoleGlobal;
static Buffer ScreenBuffer;
static RECT WinScreenClipRect = {0,0,800,768};
static int GamePreferences;
static unsigned int GameMorePreferences;
static int IsMultiplayerNet;
static int WinScreenWidth = 800;
static unsigned char WinFillColour = 9;
static int PopupScrollVerticalFrameCount;
static int MainInterfaceTileFlag;
struct Wave {
    void play() {}
    void fade(int) {
        fade_flip_state=WinFlipClipped;
        record(op_fade, 0, 0, 0, 0, 0, 0);
    }
};
static Wave g_MAININTERFACE_WAVE;

static void do_sound() {
    // A damaged wait must fail promptly rather than poll billions of times
    // after reinterpreting the deliberately negative signed delay.
    assert(sound_calls < 1000);
    ++sound_calls;
    if (mutate_wait_in_sound && sound_calls==1) {
        if (mutate_wait_in_sound == 1) MapMouseFeedbackBusy=1;
        if (mutate_wait_in_sound == 2) ConsoleControlTurnActive=1;
        if (mutate_wait_in_sound == 3) WindowAnimationFrameDelay=1;
        if (mutate_wait_in_sound == 4) PopupScrollVerticalFrameCount=0;
    }
}

// ACTUAL_SOURCE_BODIES

static void clear_events() {
    events.clear(); clock_ticks=0; sound_calls=0; flip_count=0;
}
static int count_op(int op) {
    int n=0; for (std::vector<Event>::const_iterator i=events.begin(); i!=events.end(); ++i)
        if (i->op==op) ++n;
    return n;
}
static bool saw_copy(int source, int target, int sx, int sy, int dx, int dy,
                     int width, int height) {
    for (std::vector<Event>::const_iterator i=events.begin(); i!=events.end(); ++i) {
        if (i->op==op_copy && i->source==source && i->target==target &&
            i->a==sx && i->b==sy && i->c==dx && i->d==dy) {
            std::vector<Event>::const_iterator n=i; ++n;
            if (n!=events.end() && n->op==12 && n->a==width && n->b==height)
                return true;
        }
    }
    return false;
}
static int event_index(int op, int source) {
    for (unsigned int i=0; i<events.size(); ++i)
        if (events[i].op==op && events[i].source==source)
            return static_cast<int>(i);
    return -1;
}
static bool saw_draw(int source, int target, int x, int y) {
    for (std::vector<Event>::const_iterator i=events.begin(); i!=events.end(); ++i)
        if (i->op==op_draw && i->source==source && i->target==target &&
            i->a==x && i->b==y) return true;
    return false;
}
static bool saw_graphic_rect_fill(int source, int x, int y, int w, int h, int color) {
    for (std::vector<Event>::const_iterator i=events.begin(); i!=events.end(); ++i)
        if (i->op==op_graphic_fill_rect && i->source==source &&
            i->a==x && i->b==y && i->c==w && i->d==h && i->e==color)
            return true;
    return false;
}
static bool saw_op_source(int op, int source) {
    for (std::vector<Event>::const_iterator i=events.begin(); i!=events.end(); ++i)
        if (i->op==op && i->source==source) return true;
    return false;
}
static bool locals_destroy_reverse_order() {
    std::vector<int> created, destroyed;
    for (std::vector<Event>::const_iterator i=events.begin(); i!=events.end(); ++i) {
        if (i->op==10) created.push_back(i->source);
        if (i->op==11) destroyed.push_back(i->source);
    }
    if (created.size()!=3 || destroyed.size()!=3) return false;
    for (unsigned int i=0; i<3; ++i)
        if (destroyed[i] != created[2-i]) return false;
    return true;
}

int main() {
    // Null receiver still honors slide mode-2 interface-up tail.
    clear_events(); MainInterfaceTileFlag=9; slide_hide(0,2);
    assert(count_op(op_draw)==1 && MainInterfaceTileFlag==0 && WinFlipClipped==0);
    assert(locals_destroy_reverse_order());

    // Invisible scroll is an early return with no rendering/hide side effect.
    Scroll scroll; scroll.visible=0; GraphicWin window;
    window.width=300; window.clip.right=120; window.clip.bottom=80;
    clear_events();
    const int before = count_op(op_copy); scroll_hide(&window,&scroll);
    assert(count_op(op_copy)==before && count_op(op_hide)==0);
    assert(locals_destroy_reverse_order());

    // No animation preference still performs the final client repaint and
    // dispatches the inherited virtual hide operation.
    clear_events(); scroll.visible=1; window.visible=1;
    GamePreferences=0; GameMorePreferences=0; IsMultiplayerNet=0;
    PopupScrollVerticalFrameCount=1; scroll_hide(&window,&scroll);
    assert(count_op(op_copy)>=2 && count_op(op_graphic_fill)>=1 && count_op(op_hide)==1);
    assert(locals_destroy_reverse_order());
    int no_vertical_clips=0;
    for (std::vector<Event>::const_iterator i=events.begin(); i!=events.end(); ++i) {
        if (i->op==op_clip && i->source==static_cast<Buffer *>(&window)->id) {
            if (no_vertical_clips==0)
                assert(i->a==72 && i->b==0 && i->c==120 && i->d==80);
            if (no_vertical_clips==1)
                assert(i->a==0 && i->b==0 && i->c==120 && i->d==80);
            ++no_vertical_clips;
        }
    }
    assert(no_vertical_clips==2); // <=1 skips the pre-translation bounds reset

    // The screen snapshot is copied from Console's actual Buffer base into
    // the first local Buffer, then the target Buffer is the source of the
    // window-frame capture (not vice versa).
    clear_events(); scroll.visible=1; GamePreferences=PREF_AV_SLIDING_WINDOWS;
    GameMorePreferences=0; PopupScrollVerticalFrameCount=2;
    scroll_hide(&window,&scroll);
    int locals[3] = {-1,-1,-1}; int nlocals=0;
    for (std::vector<Event>::const_iterator i=events.begin(); i!=events.end(); ++i)
        if (i->op==10 && nlocals<3) locals[nlocals++]=i->source;
    assert(nlocals==3);
    const int console_buffer = static_cast<Buffer *>(
        static_cast<GraphicWin *>(&ConsoleGlobal))->id;
    const int window_buffer = static_cast<Buffer *>(&window)->id;
    assert(saw_copy(console_buffer, locals[2], 72, 0, 0, 0, 48, 80));
    assert(saw_copy(window_buffer, locals[0], 72, 0, 0, 0, 48, 80));
    assert(saw_copy(locals[2], ScreenBuffer.id, 0, 0, 172, 50, 48, 80));
    assert(saw_op_source(op_graphic_fill, window_buffer));
    assert(count_op(op_hide)==1 && WinFlipClipped==0);
    assert(locals_destroy_reverse_order());
    int vertical_clips=0;
    for (std::vector<Event>::const_iterator i=events.begin(); i!=events.end(); ++i)
        if (i->op==op_clip && i->source==window_buffer) ++vertical_clips;
    assert(vertical_clips==3); // bounds reset, local clip, final bounds restore
    int frame_clip=-1, first_animation_copy=-1;
    for (unsigned int i=0; i<events.size(); ++i) {
        if (frame_clip<0 && events[i].op==op_clip && events[i].source==ScreenBuffer.id &&
            events[i].a==172 && events[i].b==50 && events[i].c==220 && events[i].d==130)
            frame_clip=static_cast<int>(i);
        if (first_animation_copy<0 && events[i].op==op_copy &&
            events[i].source==locals[2] && events[i].target==ScreenBuffer.id)
            first_animation_copy=static_cast<int>(i);
    }
    assert(frame_clip>=0 && first_animation_copy>=0 && frame_clip<first_animation_copy);

    // Single-player and network mode write different horizontal steps; only
    // the permitted local preference enters timed loops.
    clear_events(); scroll.visible=1; GamePreferences=0; GameMorePreferences=MPREF_AV_SLIDING_SCROLLBARS;
    IsMultiplayerNet=0; PopupScrollVerticalFrameCount=0;
    scroll_hide(&window,&scroll); assert(PopupScrollHorizontalStep==4 && flip_count==8);
    clear_events(); scroll.visible=1; IsMultiplayerNet=1; scroll_hide(&window,&scroll); assert(PopupScrollHorizontalStep==8);

    // The initial frame count is local, but the horizontal displacement is
    // reloaded from the source-owned global after Win::flip callbacks.
    clear_events(); IsMultiplayerNet=0; GamePreferences=0;
    GameMorePreferences=MPREF_AV_SLIDING_SCROLLBARS;
    PopupScrollVerticalFrameCount=0; WindowAnimationFrameDelay=3;
    mutate_horizontal_step_on_flip=0;
    scroll.visible=1; scroll_hide(&window,&scroll);
    assert(flip_count==8 && sound_calls==8); // DWORD delay 3 >> 1 == 1
    clear_events(); WindowAnimationFrameDelay=20;
    PopupScrollVerticalFrameCount=0; mutate_horizontal_step_on_flip=1;
    scroll.visible=1;
    scroll_hide(&window,&scroll);
    int scroll_locals[3] = {-1,-1,-1}; int scroll_nlocals=0;
    for (std::vector<Event>::const_iterator i=events.begin(); i!=events.end(); ++i)
        if (i->op==10 && scroll_nlocals<3) scroll_locals[scroll_nlocals++]=i->source;
    assert(scroll_nlocals==3);
    assert(flip_count==8);
    assert(saw_copy(scroll_locals[2], ScreenBuffer.id, 0, 0, 172, 50, 48, 80));
    const int screen_buffer_id=ScreenBuffer.id;
    const int window_buffer_id=static_cast<Buffer *>(&window)->id;
    assert(saw_draw(scroll_locals[0], screen_buffer_id, 168, 50));
    assert(saw_draw(scroll_locals[0], screen_buffer_id, 161, 50));
    assert(saw_draw(scroll_locals[0], window_buffer_id, 19, 0));
    mutate_horizontal_step_on_flip=0;

    // The vertical entry condition is captured before callbacks; mutating the
    // global during the first do_sound does not bypass its final bounds reset.
    clear_events(); scroll.visible=1; IsMultiplayerNet=0;
    GamePreferences=PREF_AV_SLIDING_WINDOWS; GameMorePreferences=0;
    PopupScrollVerticalFrameCount=2; WindowAnimationFrameDelay=2;
    mutate_wait_in_sound=4; scroll_hide(&window,&scroll);
    assert(flip_count==7 && PopupScrollVerticalFrameCount==0);
    int mutated_vertical_clips=0;
    for (std::vector<Event>::const_iterator i=events.begin(); i!=events.end(); ++i)
        if (i->op==op_clip && i->source==window_buffer_id) ++mutated_vertical_clips;
    assert(mutated_vertical_clips==3);
    mutate_wait_in_sound=0;

    // Native reuses its first-argument stack slot for the divisor; mode only
    // controls the independent mode&2 interface tail. The mocked window has
    // a nonidentity client-to-screen translation (+100,+50).
    clear_events(); scroll.visible=1; IsMultiplayerNet=0;
    GamePreferences=PREF_AV_SLIDING_WINDOWS | PREF_AV_SOUND_EFFECTS;
    window.width=160; window.clip.right=120; window.clip.bottom=80;
    slide_hide(&window,0); const int zero_mode_frames=flip_count;
    assert(zero_mode_frames==5);
    int slide_locals[3] = {-1,-1,-1}; int slide_nlocals=0;
    for (std::vector<Event>::const_iterator i=events.begin(); i!=events.end(); ++i)
        if (i->op==10 && slide_nlocals<3) slide_locals[slide_nlocals++]=i->source;
    assert(slide_nlocals==3);
    assert(saw_copy(slide_locals[2], ScreenBuffer.id, 0, 0, 100, 50, 160, 80));
    assert(locals_destroy_reverse_order());
    const int slide_window_buffer=static_cast<Buffer *>(&window)->id;
    assert(saw_graphic_rect_fill(slide_window_buffer, 0, 0, 23, 80, 9));
    assert(saw_graphic_rect_fill(slide_window_buffer, 137, 0, 23, 80, 9));
    assert(saw_draw(202, ScreenBuffer.id, 196, 66));
    assert(saw_draw(203, ScreenBuffer.id, 212, 94));
    for (std::vector<Event>::const_iterator i=events.begin(); i!=events.end(); ++i)
        assert(!(i->op==op_draw && i->target==slide_window_buffer &&
                 i->source>=200 && i->source<=203)); // no invented final sprite repaint

    const int init_bg=event_index(op_init, slide_locals[1]);
    const int fill_bg=event_index(op_fill, slide_locals[1]);
    const int capture_bg=event_index(op_update_window, slide_locals[1]);
    const int init_restore=event_index(op_init, slide_locals[2]);
    const int init_frame=event_index(op_init, slide_locals[0]);
    const int fill_frame=event_index(op_fill, slide_locals[0]);
    const int capture_frame=event_index(op_update_window, slide_locals[0]);
    const int restore_capture=event_index(op_update_back, slide_locals[2]);
    assert(init_bg < fill_bg && fill_bg < capture_bg && capture_bg < init_restore);
    assert(init_restore < init_frame && init_frame < fill_frame &&
           fill_frame < capture_frame && capture_frame < restore_capture);
    int left_band=-1, right_band=-1;
    for (unsigned int i=0; i<events.size(); ++i) {
        if (events[i].op==op_graphic_fill_rect && events[i].source==slide_window_buffer &&
            events[i].a==0 && events[i].c==23 && events[i].d==80) left_band=static_cast<int>(i);
        if (events[i].op==op_graphic_fill_rect && events[i].source==slide_window_buffer &&
            events[i].a==137 && events[i].c==23 && events[i].d==80) right_band=static_cast<int>(i);
    }
    assert(fill_frame < left_band && left_band < right_band && right_band < capture_frame);
    int first_screen_clip=-1, first_copy=-1;
    for (unsigned int i=0; i<events.size(); ++i) {
        if (first_screen_clip<0 && events[i].op==op_clip &&
            events[i].source==ScreenBuffer.id && events[i].a==100 &&
            events[i].b==50 && events[i].c==220 && events[i].d==130)
            first_screen_clip=static_cast<int>(i);
        if (first_copy<0 && events[i].op==op_copy)
            first_copy=static_cast<int>(i);
    }
    assert(first_screen_clip>=0 && first_copy>=0 && first_screen_clip<first_copy);
    int fade_index=event_index(op_fade,0), reset_clip=-1;
    for (unsigned int i=0; i<events.size(); ++i)
        if (events[i].op==op_clip && events[i].source==ScreenBuffer.id &&
            events[i].a==0 && events[i].b==0 && events[i].c==800 && events[i].d==768)
            reset_clip=static_cast<int>(i);
    assert(fade_index>=0 && reset_clip>=0 && fade_index<reset_clip && fade_flip_state==1);
    clear_events(); WinScreenWidth=1024; slide_hide(&window,0);
    assert(flip_count==5 && saw_draw(203, ScreenBuffer.id, 212, 238));
    WinScreenWidth=800;
    clear_events(); slide_hide(&window,2); assert(flip_count==zero_mode_frames);
    bool saw_info_clip=false, saw_full_clip=false;
    const int main_buffer=static_cast<Buffer *>(&MainInterfaceGlobal)->id;
    for (std::vector<Event>::const_iterator i=events.begin(); i!=events.end(); ++i) {
        if (i->op==op_clip && i->source==main_buffer && i->a==0 && i->b==0 &&
            i->c==700 && i->d==700) saw_info_clip=true;
        if (i->op==op_clip && i->source==main_buffer && i->a==0 && i->b==0 &&
            i->c==800 && i->d==768) saw_full_clip=true;
    }
    assert(saw_info_clip && saw_full_clip);
    clear_events(); MapMouseFeedbackBusy=1; WindowAnimationFrameDelay=-1;
    slide_hide(&window,0);
    assert(flip_count==3 && sound_calls==3); // signed -1/2 truncates to zero
    clear_events(); MapMouseFeedbackBusy=0; ConsoleControlTurnActive=0;
    WindowAnimationFrameDelay=20; mutate_wait_in_sound=1;
    slide_hide(&window,0);
    assert(flip_count==5 && sound_calls==50); // re-read map-busy after callbacks
    clear_events(); MapMouseFeedbackBusy=0; ConsoleControlTurnActive=0;
    WindowAnimationFrameDelay=20; mutate_wait_in_sound=2;
    slide_hide(&window,0);
    assert(flip_count==5 && sound_calls==50); // re-read console-busy after callbacks
    clear_events(); MapMouseFeedbackBusy=0; ConsoleControlTurnActive=0;
    WindowAnimationFrameDelay=20; mutate_wait_in_sound=3;
    slide_hide(&window,0);
    assert(flip_count==5 && sound_calls==5); // re-read changed delay after callbacks
    clear_events(); MapMouseFeedbackBusy=1; ConsoleControlTurnActive=0;
    WindowAnimationFrameDelay=3; mutate_wait_in_sound=0;
    slide_hide(&window,0);
    assert(flip_count==3 && sound_calls==3); // signed positive odd delay truncates to one
    assert(WinFlipClipped==0);
    return 0;
}
