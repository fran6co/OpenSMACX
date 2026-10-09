#include <algorithm>
#include <cstddef>
#include <stdint.h>
#include <new>
#include <vector>
#include <string>
#include <cstdlib>
#include <cstdio>

typedef int BOOL; typedef unsigned short WORD; typedef unsigned int DWORD;
typedef unsigned int UINT; typedef void *HANDLE; typedef void *HDC;
typedef void *HBITMAP; typedef void *HCURSOR; typedef void *HGDIOBJ;
struct BITMAPINFOHEADER { DWORD biSize; int32_t biWidth, biHeight; WORD biPlanes, biBitCount; DWORD biCompression, biSizeImage; int32_t biXPelsPerMeter, biYPelsPerMeter; DWORD biClrUsed, biClrImportant; };
struct RGBQUAD { unsigned char rgbBlue, rgbGreen, rgbRed, rgbReserved; };
struct BITMAPINFO { BITMAPINFOHEADER bmiHeader; RGBQUAD bmiColors[1]; };
struct Dib { BITMAPINFOHEADER bmiHeader; RGBQUAD bmiColors[256]; };
struct ICONINFO { BOOL fIcon; DWORD xHotspot, yHotspot; HBITMAP hbmMask, hbmColor; };
#define SRCCOPY 0x00cc0020
const int BI_RGB=0, DIB_RGB_COLORS=0, HORZRES=8, VERTRES=10, BITSPIXEL=12, PLANES=14, FALSE=0;

static std::vector<std::string> events;
static std::string number(int n) { char b[32]; std::sprintf(b, "%d", n); return std::string(b); }
static int buffer_live, buffer_constructs, buffer_destroys;
static int last_mono_x, last_mono_y, last_color_x, last_color_y;
static int fail_step, api_step, copied, destroyed_cursor, sprite_closed;
static int buffer_inits, palette_pointer_matches;
static const RGBQUAD *expected_color_palette;
static unsigned char bits[4];
static int dummy_handles[8];
static HBITMAP observed_mono_bitmap, observed_color_bitmap;
static HBITMAP observed_icon_mask, observed_icon_color;
static int next_handle;
static void *new_handle() { return &dummy_handles[next_handle++ % 8]; }

struct Palette {};
class Buffer;
static Buffer *observed_color;
static int mutate_callbacks, flush_count;
class Buffer {
public:
    Buffer() { ++buffer_constructs; ++buffer_live; events.push_back("buffer+"+number(buffer_live)); }
    ~Buffer() { --buffer_live; ++buffer_destroys; events.push_back("buffer-"+number(buffer_live)); }
    void init(int w,int h,int,int) { events.push_back("init:"+number(w)+","+number(h)); dib_.bmiHeader.biWidth=w; dib_.bmiHeader.biHeight=-h; if (++buffer_inits==2) { expected_color_palette=dib_.bmiColors; observed_color=this; } }
    int sync_to_palette(Palette *) { events.push_back("palette"); return 0; }
    int fill(int c) { events.push_back("fill:"+number(c)); return 0; }
    void *get_data() { return bits; }
    HDC get_hdc() { events.push_back("source-hdc"); if(mutate_callbacks) { dib_.bmiHeader.biWidth=7; dib_.bmiHeader.biHeight=-5; } return reinterpret_cast<HDC>(bits); }
    int pixel_width() const { return dib_.bmiHeader.biWidth; }
    int pixel_height() const { return -dib_.bmiHeader.biHeight; }
    Dib dib_;
};
class Sprite {
public:
    void *pcBits_; unsigned char cTransparentIndex_; int iWidth_, iHeight_;
    Sprite() : pcBits_(0), cTransparentIndex_(0), iWidth_(0), iHeight_(0) {}
    ~Sprite() { close(); }
    int draw_mono(Buffer *,int,int x,int y,int) { last_mono_x=x; last_mono_y=y; events.push_back("draw-mono"); if(mutate_callbacks) { iWidth_=3; iHeight_=3; } return 0; }
    int draw(Buffer *,int,int x,int y) { last_color_x=x; last_color_y=y; events.push_back("draw-color"); if(mutate_callbacks) { iWidth_=1; iHeight_=1; } return 0; }
    void close() { ++sprite_closed; events.push_back("sprite-close"); }
    int copy(Sprite *destination) { ++copied; destination->pcBits_=pcBits_; destination->iWidth_=iWidth_; events.push_back("copy"); return 0; }
};
class Cursor {
public:
    HCURSOR cursor_handle_; Sprite *owned_sprite_; int hotspot_x_, hotspot_y_;
    Cursor();
    ~Cursor();
    int init(Sprite *,int,int);
};
static int CursorWidth=8, CursorHeight=6;
static Palette palette;
static Palette *PaletteCurrent=&palette;
static void *BufferDirectDraw=0;

static bool step() { return ++api_step == fail_step; }
HCURSOR CreateIconIndirect(ICONINFO *p) { events.push_back("icon:"+number(p->xHotspot)+","+number(p->yHotspot)); observed_icon_mask=p->hbmMask; observed_icon_color=p->hbmColor; return step()?0:new_handle(); }
BOOL DestroyCursor(HCURSOR) { ++destroyed_cursor; events.push_back("destroy-cursor"); return 1; }
HDC CreateCompatibleDC(HDC) { events.push_back("create-dc"); return step()?0:new_handle(); }
HBITMAP CreateDIBSection(HDC,const BITMAPINFO *info,UINT,void **out,HANDLE,DWORD) { events.push_back("create-dib:"+number(info->bmiHeader.biPlanes)+","+number(info->bmiHeader.biBitCount)); *out=bits; HBITMAP h=step()?0:new_handle(); if(h) { if(info->bmiHeader.biBitCount==1) observed_mono_bitmap=h; else observed_color_bitmap=h; } return h; }
HGDIOBJ SelectObject(HDC,HGDIOBJ) { events.push_back("select"); return new_handle(); }
UINT SetDIBColorTable(HDC,UINT,UINT count,RGBQUAD *colors) { events.push_back("colors:"+number(count)); if (count==256) palette_pointer_matches=(colors==expected_color_palette); return count; }
int SetDIBits(HDC,HBITMAP,UINT,UINT,void *,const BITMAPINFO *,UINT) { events.push_back("dibits"); return 1; }
HDC CreateDCA(const char *driver,const char *,const char *,const void *) { events.push_back(std::string("display:")+driver); return new_handle(); }
int GetDeviceCaps(HDC,int cap) { events.push_back("caps:"+number(cap)); return cap==BITSPIXEL?24:1; }
BOOL DeleteDC(HDC) { events.push_back("delete-dc"); return 1; }
BOOL GdiFlush() { events.push_back("flush"); if(mutate_callbacks && ++flush_count==1) { observed_color->dib_.bmiHeader.biWidth=4; observed_color->dib_.bmiHeader.biHeight=-3; } return 1; }
BOOL BitBlt(HDC,int,int,int w,int h,HDC,int,int,DWORD rop) { events.push_back("blit:"+number(w)+","+number(h)+","+number(rop)); return 1; }
BOOL DeleteObject(HGDIOBJ) { events.push_back("delete-object"); return 1; }
static void reset() { mutate_callbacks=flush_count=0; events.clear(); buffer_live=buffer_constructs=buffer_destroys=0; last_mono_x=last_mono_y=last_color_x=last_color_y=0; fail_step=api_step=copied=destroyed_cursor=sprite_closed=next_handle=buffer_inits=palette_pointer_matches=0; expected_color_palette=0; observed_mono_bitmap=observed_color_bitmap=observed_icon_mask=observed_icon_color=0; }

// Private, typed proposal for the existing Cursor owner. This file is an
// ignored review artifact; it has not been applied to src/.
// Requires Cursor's existing member context plus `friend class Cursor;` in
// Sprite and Buffer for the private fields read by the original body.

// @@CURSOR_CONSTRUCTOR@@
// @@CURSOR_DESTRUCTOR@@
// @@CURSOR_ARITHMETIC@@

// @@CURSOR_CLIP@@

// @@CURSOR_INIT_BODY@@

#define CHECK(x) do { if (!(x)) { std::fprintf(stderr,"cursor control failed line %d: %s\n",__LINE__,#x); return 1; } } while(0)
int main() {
    Sprite input; input.pcBits_=bits; input.cTransparentIndex_=7; input.iWidth_=11; input.iHeight_=9;
    Cursor c; c.cursor_handle_=reinterpret_cast<HCURSOR>(0x123); c.owned_sprite_=0; c.hotspot_x_=77; c.hotspot_y_=88;
    reset(); c.cursor_handle_=reinterpret_cast<HCURSOR>(0x123); c.hotspot_x_=77; c.hotspot_y_=88;
    CHECK(c.init(0,4,5)==3);
    CHECK(c.cursor_handle_==reinterpret_cast<HCURSOR>(0x123) && c.hotspot_x_==77 && c.hotspot_y_==88);
    CHECK(buffer_constructs==2 && buffer_destroys==2 && buffer_live==0);
    CHECK(events.size()==4 && events[2]=="buffer-1" && events[3]=="buffer-0");

    reset(); c.cursor_handle_=reinterpret_cast<HCURSOR>(0x123); c.owned_sprite_=0; c.hotspot_x_=77; c.hotspot_y_=88; Sprite empty=input; empty.pcBits_=0; CHECK(c.init(&empty,4,5)==7); CHECK(c.cursor_handle_==reinterpret_cast<HCURSOR>(0x123) && c.hotspot_x_==77 && c.hotspot_y_==88 && buffer_destroys==2);
    reset(); c.cursor_handle_=reinterpret_cast<HCURSOR>(0x123); c.owned_sprite_=0; c.hotspot_x_=77; c.hotspot_y_=88; BufferDirectDraw=&palette;
    CHECK(c.init(&input,4,5)==0);
    CHECK(c.cursor_handle_==reinterpret_cast<HCURSOR>(0x123) && c.hotspot_x_==77 && c.hotspot_y_==88 && destroyed_cursor==0);
    CHECK(buffer_destroys==2 && buffer_live==0);

    reset(); BufferDirectDraw=0; c.cursor_handle_=0; c.owned_sprite_=0; c.hotspot_x_=0; c.hotspot_y_=0;
    CursorWidth=8; CursorHeight=6; input.iWidth_=11; input.iHeight_=9;
    CHECK(c.init(&input,3,4)==0);
    CHECK(last_mono_x==-1 && last_mono_y==-1 && last_color_x==-1 && last_color_y==-1);
    CHECK(buffer_constructs==2 && buffer_destroys==2 && buffer_live==0);
    CHECK(c.cursor_handle_!=0 && c.owned_sprite_!=0 && c.hotspot_x_==3 && c.hotspot_y_==4 && copied==1);
    CHECK(reinterpret_cast<Sprite *>(c.owned_sprite_)->iWidth_==input.iWidth_);
    CHECK(input.iWidth_==11);
    CHECK(events[0]=="buffer+1" && events[1]=="buffer+2");
    CHECK(events[events.size()-2]=="buffer-1" && events.back()=="buffer-0");
    CHECK(std::find(events.begin(),events.end(),"display:DISPLAY")!=events.end());
    CHECK(std::find(events.begin(),events.end(),"create-dib:1,1")!=events.end());
    CHECK(std::find(events.begin(),events.end(),"create-dib:1,24")!=events.end());
    {
        std::vector<std::string>::iterator planes=std::find(events.begin(),events.end(),"caps:14");
        std::vector<std::string>::iterator bits_per_pixel=std::find(events.begin(),events.end(),"caps:12");
        CHECK(planes!=events.end() && bits_per_pixel!=events.end() && planes<bits_per_pixel);
    }
    CHECK(std::find(events.begin(),events.end(),"colors:256")!=events.end());
    CHECK(palette_pointer_matches==1);
    CHECK(observed_mono_bitmap!=0 && observed_color_bitmap!=0 && observed_mono_bitmap!=observed_color_bitmap);
    CHECK(observed_icon_mask==observed_mono_bitmap && observed_icon_color==observed_color_bitmap);
    CHECK(std::find(events.begin(),events.end(),"blit:8,6,13369376")!=events.end());
    CHECK(std::find(events.begin(),events.end(),"icon:2,3")!=events.end());
    CHECK(std::find(events.begin(),events.end(),"sprite-close")==events.end());

    for (int failed_call=1; failed_call<=5; ++failed_call) {
        reset(); c.cursor_handle_=0; c.owned_sprite_=0; c.hotspot_x_=91; c.hotspot_y_=92;
        fail_step=failed_call;
        CHECK(c.init(&input,1,2)==(failed_call==5 ? 2 : 1));
        CHECK(c.cursor_handle_==0 && c.owned_sprite_==0 && c.hotspot_x_==0 && c.hotspot_y_==0);
        CHECK(buffer_destroys==2 && buffer_live==0);
        const int dc_deletes=static_cast<int>(std::count(events.begin(),events.end(),"delete-dc"));
        const int object_deletes=static_cast<int>(std::count(events.begin(),events.end(),"delete-object"));
        if (dc_deletes!=(failed_call==1 ? 0 : failed_call==2 ? 1 : failed_call==3 ? 1 : failed_call==4 ? 2 : 3))
            std::fprintf(stderr,"failed API step %d has %d dc deletes\n",failed_call,dc_deletes);
        CHECK(dc_deletes==(failed_call==1 ? 0 : failed_call==2 ? 1 : failed_call==3 ? 1 : failed_call==4 ? 2 : 3));
        CHECK(object_deletes==(failed_call==5 ? 2 : 0));
    }
    delete reinterpret_cast<Sprite *>(c.owned_sprite_);
    c.owned_sprite_=0;

    reset(); mutate_callbacks=1;
    c.cursor_handle_=c.owned_sprite_=0; input.iWidth_=11; input.iHeight_=9;
    CHECK(c.init(&input,3,4)==0);
    CHECK(last_mono_x==-1 && last_mono_y==-1);
    CHECK(last_color_x==2 && last_color_y==1);
    CHECK(std::find(events.begin(),events.end(),"blit:4,3,13369376")!=events.end());
    CHECK(std::find(events.begin(),events.end(),"icon:6,5")!=events.end());
    CHECK(cursor_half(-3)==-1 && cursor_half(-1)==0 && cursor_half(3)==1);
    CHECK(cursor_subtract(-2147483647-1,1)==2147483647);
    CHECK(cursor_add(2147483647,1)==-2147483647-1);
    CHECK(cursor_clip_hotspot(4,0)==0 && cursor_clip_hotspot(-1,8)==0);
    delete reinterpret_cast<Sprite *>(c.owned_sprite_);
    c.owned_sprite_=0;
    {
        const int cursors_before=destroyed_cursor, sprites_before=sprite_closed;
        {
            Cursor scoped;
            CHECK(scoped.cursor_handle_==0 && scoped.owned_sprite_==0);
            CHECK(scoped.hotspot_x_==0 && scoped.hotspot_y_==0);
            scoped.cursor_handle_=new_handle();
            scoped.owned_sprite_=new Sprite;
        }
        CHECK(destroyed_cursor==cursors_before+1 && sprite_closed==sprites_before+1);
    }
    return 0;
}
