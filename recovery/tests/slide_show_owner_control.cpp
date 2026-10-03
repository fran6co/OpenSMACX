// Actual-source-shaped C++98 behavior fixture for private slide fragment v2.
// The mock has the real GraphicWin/Buffer and MainInterface member paths;
// protected Buffer state is accessed only by the public clip_rect getter.
#include <assert.h>
#include <vector>
#define __cdecl
typedef unsigned int DWORD;
struct RECT { int left,top,right,bottom; };
static std::vector<int> events;
static int last_draw_x=-1,last_draw_y=-1,last_draw_key=-1;
class Buffer;
static Buffer *ctor_ptrs[8],*dtor_ptrs[8];
static int ctor_count=0,dtor_count=0;
enum { CTOR=1,DTOR,VISIBLE,INIT,FILL,CLIP,CLIP_SET,DRAW,UPDATE,TRANSFORM,
       FLIP,SOUND,RESTORE,SOFT,WAVE_PLAY,WAVE_FADE };
class Buffer {
public:
    Buffer() : width_(800),height_(600) { events.push_back(CTOR); if(ctor_count<8)ctor_ptrs[ctor_count++]=this; rect2_.left=4;rect2_.top=10;rect2_.right=340;rect2_.bottom=280; }
    virtual ~Buffer() { events.push_back(DTOR); if(dtor_count<8)dtor_ptrs[dtor_count++]=this; }
    int pixel_width() const { return width_; } int pixel_height() const { return height_; }
    const RECT &clip_rect() const { events.push_back(CLIP); return rect2_; }
    int init(int w,int h,int,void*) { width_=w;height_=h;events.push_back(INIT);return 0; }
    int resize(int w,int h) { width_=w;height_=h;events.push_back(INIT);return 0; }
    int fill(int) { events.push_back(FILL);return 0; }
    int draw(Buffer*,int,int,int,int,int) { events.push_back(DRAW);return 0; }
    int set_clip(RECT*) { events.push_back(CLIP_SET);return 0; }
protected: RECT rect2_; int width_,height_;
};
class Sprite {
public:
    Sprite(int w=12,int h=8,int k=9):w_(w),h_(h),key_(k){}
    int width()const{return w_;} int height()const{return h_;}
    int transparent_index()const{return key_;}
    int draw(Buffer*,int key,int x,int y,int,int){events.push_back(DRAW);last_draw_x=x;last_draw_y=y;last_draw_key=key;return 0;}
private:int w_,h_,key_;
};
class Win {
public:
    Win():visible_(0){outer_.left=51;outer_.top=62;outer_.right=540;outer_.bottom=400;}
    virtual ~Win(){}
    int is_visible(){events.push_back(VISIBLE);return visible_;}
    void set_visible(int v){visible_=v;}
    void client_to_screen(RECT*){events.push_back(TRANSFORM);}
    const RECT &outer_rect()const{return outer_;}
    int update_window_to_buffer(Buffer*){events.push_back(UPDATE);return 0;}
    virtual int resize(int,int,int){return 0;}
    static void flip(RECT*){events.push_back(FLIP);}
private:int visible_;RECT outer_;
};
class GraphicWin:public Win,public Buffer{};
class MainInterface:public GraphicWin{
public:
    MainInterface(){bottom_.left=244;bottom_.top=504;bottom_.right=779;bottom_.bottom=600;layout_.left=0;layout_.top=0;layout_.right=800;layout_.bottom=600;}
    Sprite &slide_frame_sprite(int i){return sprites2_[i];}
    Sprite &interface_down_sprite(){return sprites13_[1];}
    const RECT &bottom_bounds()const{return bottom_;}
    const RECT &main_layout_rect()const{return layout_;}
    int update_window_to_buffer(Buffer*){events.push_back(UPDATE);return 0;}
    void restore_back(int){events.push_back(RESTORE);}
    void soft_update(){events.push_back(SOFT);}
private:
    Sprite sprites2_[4]; Sprite sprites13_[2]; RECT bottom_,layout_;
};
class Wave { public:int play(){events.push_back(WAVE_PLAY);return 0;} void fade(unsigned long){events.push_back(WAVE_FADE);} };
MainInterface MainInterfaceGlobal;
Buffer ScreenBuffer;
RECT WinScreenClipRect={0,0,800,600};
int WinScreenWidth=800,WinScreenHeight=600,IsMultiplayerNet=0;
unsigned int GamePreferences=0;
unsigned char WinFillColour=32;
int WinFlipClipped=0,MainInterfaceTileFlag=0;
int WindowAnimationFrameDelay=20;
enum { PREF_AV_SOUND_EFFECTS=0x400, PREF_AV_SLIDING_WINDOWS=0x8000000 };
Wave g_MAININTERFACE_WAVE;
static DWORD ticks=0;
DWORD timeGetTime(){ticks+=20;return ticks;}
void do_sound(){events.push_back(SOUND);}

// ACTUAL_SOURCE_BODY

static int count(int id){int n=0;for(std::size_t i=0;i<events.size();++i)if(events[i]==id)++n;return n;}
static int at(int id){for(std::size_t i=0;i<events.size();++i)if(events[i]==id)return static_cast<int>(i);return -1;}
int main(){
    GraphicWin window;
    events.clear();ctor_count=dtor_count=0;window.set_visible(1);slide_show(&window,2);
    assert(count(CTOR)==2&&count(DTOR)==2);
    assert(ctor_ptrs[0]==dtor_ptrs[1]&&ctor_ptrs[1]==dtor_ptrs[0]);
    assert(count(VISIBLE)==1&&count(DRAW)==0&&count(RESTORE)==0&&count(SOFT)==0);

    events.clear();ctor_count=dtor_count=0;last_draw_x=last_draw_y=last_draw_key=-1;window.set_visible(0);GamePreferences=0;slide_show(0,2);
    assert(count(CTOR)==2&&count(DTOR)==2);
    assert(ctor_ptrs[0]==dtor_ptrs[1]&&ctor_ptrs[1]==dtor_ptrs[0]);
    assert(count(DRAW)==1&&MainInterfaceTileFlag==1);
    assert(last_draw_x==244&&last_draw_y==504&&last_draw_key==9);
    assert(count(RESTORE)==1&&count(SOFT)==1&&at(RESTORE)<at(SOFT));

    events.clear();ctor_count=dtor_count=0;ticks=0;GamePreferences=0x8000400;IsMultiplayerNet=0;
    slide_show(&window,0);
    assert(count(WAVE_PLAY)==1&&count(WAVE_FADE)==1);
    assert(count(FLIP)==5&&count(SOUND)>=5&&WinFlipClipped==0);
    assert(count(UPDATE)==2&&count(DRAW)>=20);

    events.clear();ctor_count=dtor_count=0;GamePreferences=0x8000000;IsMultiplayerNet=1;
    slide_show(&window,0);
    assert(count(FLIP)==0&&WinFlipClipped==0&&count(WAVE_PLAY)==0);
    return 0;
}
