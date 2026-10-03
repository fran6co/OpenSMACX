// Host behavior/control fixture for the v3 private source fragment. It checks
// the real argument hierarchy, the public clip getter, and protected access
// restrictions; it does not claim the native ABI or accept the recovery.
#include <assert.h>
#include <vector>

#define __cdecl
typedef unsigned int DWORD;
struct RECT { int left, top, right, bottom; };
static std::vector<int> events;
class Buffer;
struct CopyCall { Buffer *source; Buffer *destination; int x,y,dx,dy,w,h; };
static CopyCall copy_calls[24];
static int copy_count=0;
static Buffer *last_buffer_draw_destination=0;
static int last_buffer_draw_x=0,last_buffer_draw_y=0;
static Buffer *ctor_ptrs[8],*dtor_ptrs[8],*init_ptrs[8];
static int ctor_count=0,dtor_count=0,init_count=0;
static int init_widths[8],init_heights[8];
struct TileCall { int src_x,src_y,x,y,w,h; };
static TileCall tile_calls[8];
static int tile_count=0;
enum { CTOR=1, DTOR, VISIBLE, CLIP, INIT, FILL, COPY, DRAW, TILE,
       CLIP_SET, TRANSFORM, UPDATE, FLIP, SOUND, RESIZE, MOVE, SHOW };

class Buffer {
public:
    Buffer() : width_(0), height_(0) {
        events.push_back(CTOR);
        if(ctor_count<8) ctor_ptrs[ctor_count++]=this;
        rect2_.left=4; rect2_.top=10; rect2_.right=340; rect2_.bottom=280;
    }
    virtual ~Buffer() { events.push_back(DTOR); if(dtor_count<8)dtor_ptrs[dtor_count++]=this; }
    int pixel_width() const { return width_; }
    int pixel_height() const { return height_; }
    const RECT &clip_rect() const { events.push_back(CLIP); return rect2_; }
    void set_test_clip(const RECT &r) { rect2_=r; }
    int init(int w,int h,int,void*) { width_=w; height_=h; events.push_back(INIT); if(init_count<8){init_widths[init_count]=w;init_heights[init_count]=h;init_ptrs[init_count++]=this;} return 0; }
    int resize(int w,int h) { width_=w; height_=h; events.push_back(INIT); return 0; }
    int fill(int) { events.push_back(FILL); return 0; }
    int copy(Buffer *destination,int x,int y,int dx,int dy,int w,int h) {
        events.push_back(COPY);
        if(copy_count<24) {
            copy_calls[copy_count].source=this;
            copy_calls[copy_count].destination=destination;
            copy_calls[copy_count].x=x; copy_calls[copy_count].y=y;
            copy_calls[copy_count].dx=dx; copy_calls[copy_count].dy=dy;
            copy_calls[copy_count].w=w; copy_calls[copy_count].h=h;
            ++copy_count;
        }
        return 0;
    }
    int draw(Buffer *destination,int,int x,int y,int,int) {
        events.push_back(DRAW); last_buffer_draw_destination=destination;
        last_buffer_draw_x=x; last_buffer_draw_y=y; return 0;
    }
    int tile(class Sprite*,int sx,int sy,int x,int y,int w,int h) {
        events.push_back(TILE);
        if(tile_count<8) {
            tile_calls[tile_count].src_x=sx; tile_calls[tile_count].src_y=sy;
            tile_calls[tile_count].x=x; tile_calls[tile_count].y=y;
            tile_calls[tile_count].w=w; tile_calls[tile_count].h=h;
            ++tile_count;
        }
        return 0;
    }
    int set_clip(RECT*) { events.push_back(CLIP_SET); return 0; }
protected:
    RECT rect2_;
    int width_, height_;
};

class Sprite {
public:
    Sprite(int w=12,int h=8,int key=9) : w_(w),h_(h),key_(key) {}
    int width() const { return w_; }
    int height() const { return h_; }
    int transparent_index() const { return key_; }
    int draw(Buffer*,int,int,int,int,int) { events.push_back(DRAW); return 0; }
private: int w_,h_,key_;
};

class Win {
public:
    Win() : visible_(0) { outer_.left=51; outer_.top=62; outer_.right=540; outer_.bottom=400; }
    virtual ~Win() {}
    int is_visible() { events.push_back(VISIBLE); return visible_; }
    void client_to_screen(RECT*) { events.push_back(TRANSFORM); }
    void screen_to_client(RECT*) { events.push_back(TRANSFORM); }
    const RECT &outer_rect() const { return outer_; }
    int update_window_to_buffer(Buffer*) { events.push_back(UPDATE); return 0; }
    virtual int resize(int,int,int) { events.push_back(RESIZE); return 0; }
    int move(int,int) { events.push_back(MOVE); return 0; }
    virtual void show(int) { events.push_back(SHOW); }
    static void flip(RECT*) { events.push_back(FLIP); }
    void set_visible(int v) { visible_=v; }
private: int visible_; RECT outer_;
};

class GraphicWin : public Win, public Buffer {};
class MapWin : public virtual GraphicWin {};
class Console : public MapWin {};
class MainInterface : public GraphicWin {
public:
    MainInterface() {
        Buffer::init(800,600,0,0);
        sprites1_[0]=Sprite(10,13,5);
        sprites1_[1]=Sprite(80,200,6);
        sprites1_[2]=Sprite(11,19,7);
        sprites1_[3]=Sprite(30,17,8);
    }
    Sprite &slide_frame_sprite(int i) { return sprites2_[i]; }
    Sprite &scroll_frame_sprite(int i) { return sprites1_[i]; }
    int update_window_to_buffer(Buffer*) { events.push_back(UPDATE); return 0; }
private:
    Sprite sprites1_[4]; Sprite sprites2_[4];
};
class Scroll : public Win {
public:
    Scroll() : resize_w(0),resize_h(0),resize_mode(0),move_x(0),move_y(0),shown(-1) {}
    int resize(int w,int h,int mode) { events.push_back(RESIZE); resize_w=w;resize_h=h;resize_mode=mode;return 0; }
    int move(int x,int y) { events.push_back(MOVE);move_x=x;move_y=y;return 0; }
    void show(int mode) { events.push_back(SHOW);shown=mode; }
    int resize_w,resize_h,resize_mode,move_x,move_y,shown;
};

MainInterface MainInterfaceGlobal;
Console ConsoleGlobal;
Buffer ScreenBuffer;
RECT WinScreenClipRect={0,0,800,600};
int WinScreenWidth=800;
int IsMultiplayerNet=0;
int GameMorePreferences=0;
int GamePreferences=0;
enum { PREF_AV_SLIDING_WINDOWS=0x8000000, MPREF_AV_SLIDING_SCROLLBARS=0x80000 };
int WindowAnimationFrameDelay=20;
int WinFlipClipped=0;
int PopupScrollHorizontalStep=0;
int PopupScrollVerticalFrameCount=0;
unsigned char WinFillColour=32;
static DWORD ticks=0;
DWORD timeGetTime() { ticks+=20; return ticks; }
void do_sound() { events.push_back(SOUND); }

// ACTUAL_SOURCE_BODY

static int count(int code) {
    int n=0; for (std::size_t i=0;i<events.size();++i) if(events[i]==code) ++n; return n;
}
static int at(int code) {
    for (std::size_t i=0;i<events.size();++i) if(events[i]==code) return static_cast<int>(i);
    return -1;
}

int main() {
    GraphicWin window; Scroll scroll;
    RECT test_area={4,10,54,510};
    window.Buffer::set_test_clip(test_area);
    events.clear(); ctor_count=dtor_count=init_count=0; scroll.set_visible(1);
    scroll_show(&window,&scroll);
    assert(count(CTOR)==3 && count(DTOR)==3);
    assert(count(VISIBLE)==1 && count(INIT)==0 && count(RESIZE)==0 && count(SHOW)==0);
    assert(ctor_count==3 && dtor_count==3);
    assert(ctor_ptrs[0]==dtor_ptrs[2] && ctor_ptrs[1]==dtor_ptrs[1]
           && ctor_ptrs[2]==dtor_ptrs[0]);

    events.clear(); ctor_count=dtor_count=init_count=0; ticks=0; scroll.set_visible(0);
    window.Buffer::resize(400,600);
    events.clear(); copy_count=0; tile_count=0; last_buffer_draw_destination=0;
    GameMorePreferences=0x80000; GamePreferences=0x8000000; IsMultiplayerNet=0;
    scroll_show(&window,&scroll);
    assert(count(CTOR)==3 && count(DTOR)==3);
    assert(ctor_count==3 && dtor_count==3 && init_count==3);
    assert(ctor_ptrs[0]==init_ptrs[2]); // FA24 panel Buffer constructed first, initialized last.
    assert(ctor_ptrs[1]==init_ptrs[0]); // EF14 interface background.
    assert(ctor_ptrs[2]==init_ptrs[1]); // F49C save Buffer.
    assert(init_widths[1]==50 && init_heights[1]==500);
    assert(init_widths[2]==50 && init_heights[2]==487);
    assert(ctor_ptrs[0]==dtor_ptrs[2] && ctor_ptrs[1]==dtor_ptrs[1]
           && ctor_ptrs[2]==dtor_ptrs[0]);
    assert(PopupScrollVerticalFrameCount>1 && PopupScrollHorizontalStep==4);
    assert(count(FLIP)==14 && count(SOUND)>=14 && WinFlipClipped==0);
    assert(count(TILE)==2 && count(RESIZE)==1 && count(MOVE)==1 && count(SHOW)==1);
    assert(at(RESIZE)<at(MOVE) && at(MOVE)<at(SHOW));
    assert(scroll.resize_w==0x18 && scroll.resize_mode==0 && scroll.shown==0);
    assert(scroll.move_x==window.Buffer::pixel_width()-0x2B && scroll.move_y==0x15);
    assert(copy_count==9);
    Buffer *console_buffer=static_cast<Buffer *>(
        static_cast<GraphicWin *>(&ConsoleGlobal));
    assert(copy_calls[0].source==console_buffer);
    assert(copy_calls[0].destination!=console_buffer);
    assert(copy_calls[0].x==4 && copy_calls[0].y==10);
    assert(copy_calls[0].dx==0 && copy_calls[0].dy==0);
    assert(copy_calls[0].w==50 && copy_calls[0].h==500);
    assert(copy_calls[1].source!=console_buffer);
    assert(copy_calls[1].destination==&ScreenBuffer);
    assert(copy_calls[1].x==0 && copy_calls[1].y==0);
    assert(copy_calls[1].dx==4 && copy_calls[1].dy==23);
    assert(copy_calls[1].w==50 && copy_calls[1].h==500);
    // Native reads scroll_frame's negative Dib.biHeight at +0x84. This case
    // distinguishes -frame_height (-411/200 => -2 by truncation toward zero)
    // from width (126/200 => 0) and positive height (563/200 => 2).
    assert(scroll.resize_h==422);
    assert(tile_count==2);
    assert(tile_calls[0].src_x==0 && tile_calls[0].src_y==0);
    assert(tile_calls[0].x==0 && tile_calls[0].y==3);
    assert(tile_calls[0].w==80 && tile_calls[0].h==432);
    assert(tile_calls[1].src_x==0 && tile_calls[1].src_y==435);
    assert(tile_calls[1].x==0 && tile_calls[1].y==432);
    assert(tile_calls[1].w==30 && tile_calls[1].h==55);
    assert(last_buffer_draw_destination==static_cast<Buffer *>(&window));
    assert(last_buffer_draw_x==350 && last_buffer_draw_y==13);

    // Cross a tile quotient boundary: saved H=480, shortened panel H=467.
    // Reusing saved height would produce panel=432 rather than panel=232.
    test_area.bottom=490;
    window.Buffer::set_test_clip(test_area);
    events.clear(); ctor_count=dtor_count=init_count=0; copy_count=tile_count=0;
    scroll_show(&window,&scroll);
    assert(init_heights[1]==480 && init_heights[2]==467);
    assert(copy_calls[1].h==480);
    assert(scroll.resize_h==222);
    assert(tile_calls[0].h==232 && tile_calls[1].h==235);

    events.clear(); ctor_count=dtor_count=init_count=0; ticks=0; GameMorePreferences=0; GamePreferences=0; IsMultiplayerNet=1;
    scroll_show(&window,&scroll);
    assert(PopupScrollHorizontalStep==8);
    assert(count(FLIP)==0 && WinFlipClipped==0);
    assert(count(RESIZE)==1 && count(SHOW)==1);
    return 0;
}
