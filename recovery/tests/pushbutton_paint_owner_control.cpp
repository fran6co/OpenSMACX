#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
typedef long LONG;
typedef unsigned int uint32_t;
struct RECT { LONG left, top, right, bottom; };
struct Event { std::string name; int a,b,c,d; };
static std::vector<Event> events;
static void log(const char *n,int a=0,int b=0,int c=0,int d=0) {
    Event e; e.name=n; e.a=a; e.b=b; e.c=c; e.d=d; events.push_back(e);
}
class Palette {};
static Palette active_palette;
static Palette *PaletteActive=&active_palette;
class Win {
public:
    int flags;
    Win():flags(0){}
    bool has_dialog_focus_flag() const { return (flags&0x1000)!=0; }
    int is_dialog_focus() { return 1; }
};
static Win parent;
static Win *WinCallbackWindow=0;
static void *HandleMain=(void*)1;
class Buffer {
public:
    RECT rect2;
    int width,height;
    void sync_to_palette(Palette *) { log("palette"); }
    int pixel_width() const { return width; }
    int pixel_height() const { return height; }
    int box(RECT *r,int a,int b) { log("box",a,b,r->left,r->top); return 0; }
    int tile(class Sprite *,int,int,int x,int y,int w,int h) { log("tile",x,y,w,h); return 0; }
    int write_cent_l(char *,RECT *,unsigned int) { log("text"); return 0; }
};
class GraphicWin : public Win, public Buffer {
public:
    int fill(int color) { log("fill",color); return 0; }
    RECT *buffer_working_rect() { return &rect2; }
};
class Sprite {
public:
    int w,h,id;
    Sprite(int i=0):w(8),h(6),id(i){}
    unsigned char transparent_index() const { return 4; }
    int width() const { return w; }
    int height() const { return h; }
    int draw(Buffer *,int,int x,int y,int,int) {
        log("draw",id,x,y); return 0;
    }
};
class PushButton : public GraphicWin {
public:
    char *name_;
    uint32_t color_,bevel_upper_,bevel_lower_,bevel_thickness_,field_A94_,field_A98_,field_A9C_;
    Sprite *field_AB8_,*field_ABC_,*field_AC0_,*field_AC4_,*field_AC8_,*field_ACC_;
    Sprite *field_AD0_,*field_AD4_,*field_AD8_,*field_ADC_,*field_AE0_,*field_AE4_;
    Sprite *field_AE8_,*field_AEC_,*field_AF0_,*field_AF4_,*field_AF8_,*field_AFC_;
    Win *parent_window() { return &parent; }
    int is_dialog_focus() { return Win::is_dialog_focus(); }
    void on_redraw();
};

/* ACTUAL OWNER BODY */

static Sprite sprites[18];
static void setup(PushButton &b,int alternate) {
    b.rect2.left=0;b.rect2.top=0;b.rect2.right=100;b.rect2.bottom=60;
    b.width=100;b.height=60;b.name_=0;
    b.color_=3;b.bevel_upper_=11;b.bevel_lower_=22;b.bevel_thickness_=2;
    b.field_A94_=0xFFFFFFFFU;b.field_A9C_=static_cast<uint32_t>(alternate);
    b.field_AB8_=&sprites[0];b.field_ABC_=&sprites[1];b.field_AC0_=&sprites[2];
    b.field_AC4_=&sprites[3];b.field_AC8_=&sprites[4];b.field_ACC_=&sprites[5];
    b.field_AD0_=&sprites[6];b.field_AD4_=&sprites[7];b.field_AD8_=&sprites[8];
    b.field_ADC_=&sprites[9];b.field_AE0_=&sprites[10];b.field_AE4_=&sprites[11];
    b.field_AE8_=&sprites[12];b.field_AEC_=&sprites[13];b.field_AF0_=&sprites[14];
    b.field_AF4_=&sprites[15];b.field_AF8_=&sprites[16];b.field_AFC_=&sprites[17];
    for(int i=0;i<18;++i){sprites[i].id=i;sprites[i].w=10+i;sprites[i].h=5+i;}
}
static int count(const char *n) { int x=0;for(size_t i=0;i<events.size();++i)if(events[i].name==n)++x;return x; }
int main() {
    PushButton b;
    WinCallbackWindow=0; HandleMain=0; setup(b,0); b.on_redraw();
    if(WinCallbackWindow!=&parent || !events.empty()) return 1;
    HandleMain=(void*)1; events.clear(); setup(b,0); b.field_AC8_=0; b.on_redraw();
    if(count("palette")!=1 || count("fill")!=1 || count("box")!=2 || count("draw")!=0) return 2;
    events.clear(); setup(b,0); b.field_AC8_=&sprites[4]; b.on_redraw();
    if(count("draw")!=4 || count("tile")!=5) return 3;
    if(events.back().name!="tile" || events.back().a!=sprites[1].width() ||
       events.back().b!=sprites[2].height()) return 4;
    events.clear(); setup(b,1); b.field_AEC_=&sprites[13]; b.on_redraw();
    if(count("draw")!=4 || count("tile")!=5) return 5;
    return 0;
}
