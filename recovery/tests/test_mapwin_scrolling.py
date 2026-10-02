"""Actual scrolling controller, with original-derived geometry observations.

The complete controller, relevant declarations, region/vertex types and real
base clauses are extracted unchanged. Geometry, blitting, terrain generation,
mouse and repaint boundaries are controlled doubles. This tests bounded caller
behavior, not complete rendering, native layouts, linked preservation or game
acceptance. Dimensions and altitude sums stay within signed arithmetic bounds;
callback replacements retain valid objects. Pixel strips are half-open, whereas
the tile-damage regions intentionally retain their different original bounds.
"""

from pathlib import Path
import os
import re
import shutil
import subprocess

import pytest


ROOT = (Path(os.environ["OPENSMACX_ROOT"]).resolve() if "OPENSMACX_ROOT" in os.environ
       else Path(__file__).resolve().parents[2])


def braced(source, signature):
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if not depth:
            return source[start:end + 1]
    raise AssertionError("unterminated actual definition")


def actual_source():
    headers = {name: (ROOT / "src" / (name + ".h")).read_text()
               for name in ("win", "graphicwin", "mapwin", "console", "buffer", "vert", "map", "game")}
    source = (ROOT / "src/mapwin.cpp").read_text()
    parts = {}
    for unit, cls in (("win", "Win"), ("graphicwin", "GraphicWin"),
                      ("mapwin", "MapWin"), ("console", "Console")):
        parts[cls.upper() + "_BASES"], = re.findall(
            r"^class " + cls + r"\s*:[^{]+\{", headers[unit], re.M)
    parts["REGION_TYPE"] = braced(headers["mapwin"], "struct MapTileDamageRect") + ";"
    parts["VERT_TYPE"] = braced(headers["vert"], "struct Vert") + ";"
    parts["DIB_TYPE"] = braced(headers["buffer"], "struct Dib : BITMAPINFO") + ";"
    fields = []
    for name in ("field_1DD80_", "field_1DDA4_", "field_1DDA8_", "field_1DDAC_",
                 "field_1DDB0_", "field_1DDCC_", "field_1DDD0_", "field_1DDD4_",
                 "field_1DDD8_", "field_21A44_", "horizontal_damage_", "vertical_damage_"):
        declaration, = re.findall(
            r"^  (?:uint32_t|int|int32_t|MapTileDamageRect) " + name + r";", headers["mapwin"], re.M)
        fields.append(declaration)
    parts["MAP_FIELDS"] = "\n".join(fields)
    parts["SURFACE_FIELDS"] = "\n".join(re.findall(r"^  (?:Dib dib_|RECT rect2_);", headers["buffer"], re.M))
    parts["FACTION_FIELD"], = re.findall(r"^  int32_t faction_id_;", headers["console"], re.M)
    for key, method in (("MOUSE_DECL", "on_mouse_move"), ("UPDATE_DECL", "on_update")):
        parts[key] = braced(headers["win"], "virtual void " + method + "(")
    parts["MAP_METHODS"] = "\n".join(re.findall(
        r"^  (?:void|int) (?:draw_map|calculate_dimensions|tile_to_pixel|get_alt|gen_map)\([^;{}]*\);",
        headers["mapwin"], re.M))
    assert len(parts["MAP_METHODS"].splitlines()) == 5
    parts["MOUSE_GETTER"], = re.findall(r"^  void get_mouse_pos\([^;{}]*\);", headers["win"], re.M)
    parts["CLIP_DECL"], = re.findall(r"^  int set_clip\(RECT \*rect\);", headers["buffer"], re.M)
    parts["FILL_DECL"], = re.findall(r"^  int fill\(int x1, int y1, int x2, int y2, int color\);", headers["graphicwin"], re.M)
    parts["COPY_DECL"], = re.findall(
        r"^  MEASURED int copy\(Buffer \*buffer, int xCoord, int yCoord, int wx, int wy,\s+int width, int height\)",
        headers["buffer"], re.M)
    parts["COPY_DECL"] += ";"
    globals_ = []
    for unit, name in (("map", "MapLongitudeBounds"), ("map", "MapIsFlat"), ("game", "GameState")):
        declaration, = re.findall(r"^extern \w+ " + name + r";", headers[unit], re.M)
        globals_.append(declaration.removeprefix("extern "))
    parts["GLOBALS"] = "\n".join(globals_)
    arrays = []
    for name in ("MapCornerCopy", "MapCornerScratch"):
        declaration, = re.findall(r"^(?:static )?Vert " + name + r"\[4\](?:\s*=\s*\{\s*\})?;", source, re.M)
        arrays.append(declaration)
    parts["CORNERS"] = "\n".join(arrays)
    parts["BODY"] = braced(source, "void MapWin::draw_map(int draw_type)")
    return parts


HARNESS = r'''
#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <algorithm>
#define __cdecl
#define MEASURED
typedef int BOOL;
void check(bool value,int line){if(!value){std::fprintf(stderr,"assertion line %d\n",line);std::exit(96);}}
#define require(x) check((x),__LINE__)
struct RECT {int32_t left,top,right,bottom;};
struct RGBQUAD {uint8_t blue,green,red,reserved;};
struct BITMAPINFOHEADER {uint32_t biSize;int32_t biWidth,biHeight;uint16_t biPlanes,biBitCount;
 uint32_t biCompression,biSizeImage;int32_t biXPelsPerMeter,biYPelsPerMeter;uint32_t biClrUsed,biClrImportant;};
struct BITMAPINFO {BITMAPINFOHEADER bmiHeader;RGBQUAD bmiColors[1];};
DIB_TYPE
VERT_TYPE
REGION_TYPE
static_assert(sizeof(Vert)==8,"two signed Win32 coordinates");
static_assert(sizeof(MapTileDamageRect)==16,"four signed tile bounds");
struct AutoSound {virtual ~AutoSound(){}};
class GraphicWin;
WIN_BASES public:
    MOUSE_DECL
    UPDATE_DECL
    MOUSE_GETTER
};
struct Buffer {
    SURFACE_FIELDS
    COPY_DECL
    CLIP_DECL
};
GRAPHICWIN_BASES public:FILL_DECL};
MAPWIN_BASES public:
    MAP_FIELDS
    MAP_METHODS
};
CONSOLE_BASES public:FACTION_FIELD};
Console ConsoleGlobal;
struct Observed:MapWin {void on_mouse_move(int,int) override;void on_update() override;} window;
CORNERS
GLOBALS
struct Event {char kind;int a,b,c,d,e,f;RECT clip;MapTileDamageRect h,v;};
std::vector<Event> seen;
int next_x,next_y,old_x,old_y,projection_calls,alt_calls,gen_calls,clip_calls;
int old_px,old_py,new_px,new_py,old_alt[4],new_alt[4];
int scenario;
RECT live_clip;
bool same(const RECT&a,const RECT&b){return a.left==b.left&&a.top==b.top&&a.right==b.right&&a.bottom==b.bottom;}
bool same(const MapTileDamageRect&a,const MapTileDamageRect&b){return a.left==b.left&&a.top==b.top&&a.right==b.right&&a.bottom==b.bottom;}
void note(char kind,int a=0,int b=0,int c=0,int d=0,int e=0,int f=0){
    Event v={kind,a,b,c,d,e,f,live_clip,window.horizontal_damage_,window.vertical_damage_};seen.push_back(v);
}
int IsRectEmpty(const RECT *r){note('E',r->left,r->top,r->right,r->bottom);return r->left>=r->right||r->top>=r->bottom;}
void MapWin::calculate_dimensions(){
    require(this==&window);note('D',field_21A44_);
    field_1DDA4_=next_x;field_1DDA8_=next_y;
    if(scenario==5){GameState=0x80;ConsoleGlobal.faction_id_=17;}
}
void MapWin::tile_to_pixel(int x,int y,int *px,int *py){
    require(this==&window);note('P',x,y);++projection_calls;
    require(projection_calls<=2);
    if(projection_calls==1){require(x==next_x&&y==next_y);*px=new_px;*py=new_py;
        if(scenario==6){field_1DDA4_=42;field_1DDA8_=44;}}
    else {require(x==old_x&&y==old_y);*px=old_px;*py=old_py;}
}
int MapWin::get_alt(int x,int y,int corner){
    require(this==&window&&corner==4-alt_calls%4);note('A',x,y,corner);++alt_calls;
    if(alt_calls<=4){require(x==(scenario==6?42:next_x)&&y==(scenario==6?44:next_y));
        if(scenario==6){field_1DDA4_=62;field_1DDA8_=64;}return new_alt[corner-1];}
    require(alt_calls<=8&&x==old_x&&y==old_y);return old_alt[corner-1];
}
int Buffer::copy(Buffer *source,int x,int y,int sx,int sy,int w,int h){
    require(this==static_cast<Buffer*>(&window)&&source==this);note('B',x,y,sx,sy,w,h);
    if(scenario==4){MapCornerCopy[0]={91,-37};}
    if(scenario==7){window.field_1DDA4_=50;window.field_1DDCC_=1;window.field_1DDD4_=1;
        dib_.bmiHeader.biWidth=120;dib_.bmiHeader.biHeight=-90;}
    if(scenario==11){window.field_1DDA4_=50;window.field_1DDCC_=50;window.field_1DDD4_=50;}
    return -13;
}
int GraphicWin::fill(int x,int y,int w,int h,int colour){
    require(this==static_cast<GraphicWin*>(&window)&&colour==0);note('F',x,y,w,h);
    if(scenario==8){dib_.bmiHeader.biWidth=90;dib_.bmiHeader.biHeight=-96;}
    if(scenario==9)window.field_21A44_=0;
    return -15;
}
int Buffer::set_clip(RECT *r){
    require(this==static_cast<Buffer*>(&window));live_clip=*r;++clip_calls;
    note(r==&rect2_?'Z':'C',r->left,r->top,r->right,r->bottom);
    if(scenario==3 && r!=&rect2_){ConsoleGlobal.faction_id_=5+clip_calls;if(clip_calls==2)GameState=0x80;}
    if(scenario==10 && clip_calls==1){window.vertical_damage_={81,82,83,84};}
    return -17;
}
void MapWin::gen_map(int faction,int draw_only){
    require(this==&window);note('G',faction,draw_only);++gen_calls;
    if(scenario==2){
        if(gen_calls==1){horizontal_damage_={41,42,43,44};vertical_damage_={51,52,53,54};}
        else {horizontal_damage_={61,62,63,64};vertical_damage_={71,72,73,74};}
    }
    if(scenario==3){field_1DD80_=1;}
}
void Win::get_mouse_pos(int *x,int *y){
    require(this==static_cast<Win*>(static_cast<GraphicWin*>(&ConsoleGlobal)));note('M');*x=123;*y=-45;
}
void Observed::on_mouse_move(int x,int y){require(this==&window&&x==123&&y==-45);note('S',x,y);
    if(scenario==3)rect2_={3,4,77,55};}
void Observed::on_update(){require(this==&window);note('U');}
BODY
void reset(){
    seen.clear();old_x=10;old_y=10;next_x=12;next_y=12;
    window.field_1DDA4_=old_x;window.field_1DDA8_=old_y;
    window.field_1DD80_=window.field_1DDAC_=window.field_1DDB0_=0;window.field_21A44_=1;
    window.field_1DDCC_=3;window.field_1DDD4_=5;window.field_1DDD0_=2;window.field_1DDD8_=4;
    window.horizontal_damage_={101,102,103,104};window.vertical_damage_={111,112,113,114};
    window.dib_={};window.dib_.bmiHeader.biWidth=100;window.dib_.bmiHeader.biHeight=-80;
    window.rect2_={0,0,100,80};live_clip=window.rect2_;
    projection_calls=alt_calls=gen_calls=clip_calls=0;
    old_px=old_py=0;new_px=20;new_py=12;scenario=0;
    for(int i=0;i<4;++i){old_alt[i]=new_alt[i]=0;MapCornerCopy[i]={10+i*3,20+i*5};MapCornerScratch[i]={-30-i,50+i};}
    MapLongitudeBounds=100;MapIsFlat=0;GameState=0;ConsoleGlobal.faction_id_=3;
}
std::vector<Event> events(char kind){std::vector<Event> out;for(const auto&e:seen)if(e.kind==kind)out.push_back(e);return out;}
void kinds(const char *expected){size_t i=0;for(const auto&e:seen){require(expected[i]&&e.kind==expected[i]);++i;}require(!expected[i]);}
void origins(int ox,int oy,int nx,int ny){old_x=ox;old_y=oy;next_x=nx;next_y=ny;window.field_1DDA4_=ox;window.field_1DDA8_=oy;}
void no_scroll_controls(){
    reset();next_x=old_x;next_y=old_y;window.draw_map(2);kinds("D");
    require(same(window.horizontal_damage_,MapTileDamageRect{101,102,103,104})&&gen_calls==0);
    for(int axis=0;axis<2;++axis){reset();if(axis)window.field_1DDB0_=1;else window.field_1DDAC_=1;
        window.draw_map(-1);kinds("DGU");require(seen[0].a==0&&gen_calls==1&&events('G')[0].b==1);}
    reset();window.field_21A44_=0;window.draw_map(0);kinds("DG");require(events('G')[0].a==3&&events('G')[0].b==0);
    reset();scenario=5;window.field_21A44_=0;window.draw_map(1);kinds("DGU");require(events('G')[0].a==-1);
}
void both_strips(){
    reset();window.draw_map(1);
    kinds("DPAAAAPAAAABFFEECGCGZU");
    auto b=events('B');require(b.size()==1&&b[0].a==20&&b[0].b==12&&b[0].c==0&&b[0].d==0&&b[0].e==100&&b[0].f==80);
    auto f=events('F');require(f.size()==2&&f[0].a==80&&f[0].b==0&&f[0].c==20&&f[0].d==80);
    require(f[1].a==0&&f[1].b==68&&f[1].c==100&&f[1].d==12);
    auto g=events('G');require(g.size()==2&&same(g[0].clip,RECT{80,0,100,80})&&same(g[1].clip,RECT{0,58,100,80}));
    const MapTileDamageRect h={15,12,21,18},v={12,11,20,21};
    require(same(g[0].h,h)&&same(g[0].v,MapTileDamageRect{21,11,20,21}));
    require(same(g[1].h,MapTileDamageRect{21,12,21,18})&&same(g[1].v,v));
    require(same(window.horizontal_damage_,h)&&same(window.vertical_damage_,v));
    for(int i=0;i<4;++i){require(MapCornerCopy[i].x==30+3*i&&MapCornerCopy[i].y==32+5*i);
        require(MapCornerScratch[i].x==-30-i&&MapCornerScratch[i].y==50+i);}
    reset();scenario=2;window.draw_map(0);g=events('G');
    require(same(g[1].h,MapTileDamageRect{43,42,43,44})&&same(g[1].v,v));
    require(same(window.horizontal_damage_,MapTileDamageRect{41,42,43,44})&&same(window.vertical_damage_,MapTileDamageRect{71,72,73,74}));
    reset();scenario=3;window.draw_map(-1);g=events('G');
    require(g[0].a==6&&g[1].a==-1&&g[0].b==1&&g[1].b==1);
    require(seen[seen.size()-4].kind=='M'&&seen[seen.size()-3].kind=='S'&&seen[seen.size()-2].kind=='Z'&&seen.back().kind=='U');
    require(same(live_clip,RECT{3,4,77,55}));
    reset();scenario=4;window.draw_map(0);require(MapCornerCopy[0].x==111&&MapCornerCopy[0].y==-25);
}
void single_strips_and_negative_average(){
    reset();next_y=old_y;new_py=0;window.draw_map(0);auto g=events('G');
    require(g.size()==1&&same(g[0].clip,RECT{80,0,100,80})&&events('E').size()==1);
    require(same(window.vertical_damage_,MapTileDamageRect{0,0,0,0}));
    require(events('F').size()==2&&events('F')[1].d==0);
    reset();next_x=old_x;new_px=0;window.draw_map(0);g=events('G');
    require(g.size()==1&&same(g[0].clip,RECT{0,58,100,80})&&events('E').size()==2);
    require(same(window.horizontal_damage_,MapTileDamageRect{0,0,0,0}));
    require(events('F').size()==2&&events('F')[0].c==0);
    reset();new_px=new_py=0;window.draw_map(0);g=events('G');
    require(g.size()==1&&same(g[0].clip,RECT{0,0,0,80}));
    reset();next_x=old_x;new_px=new_py=0;new_alt[0]=-1;window.draw_map(0);g=events('G');
    require(g.size()==1&&same(g[0].clip,RECT{0,-10,100,1}));
    require(events('B')[0].d==1&&MapCornerCopy[0].y==19);
    reset();next_x=old_x;new_px=new_py=0;old_alt[0]=-5;window.draw_map(0);g=events('G');
    require(g.size()==1&&same(g[0].clip,RECT{0,68,100,80})&&events('B')[0].b==2);
    reset();new_px=-20;new_py=-12;window.draw_map(0);g=events('G');
    require(same(g[0].clip,RECT{0,0,20,80})&&same(g[1].clip,RECT{0,-10,100,12}));
    require(events('B')[0].a==0&&events('B')[0].b==0&&events('B')[0].c==20&&events('B')[0].d==12);
    require(MapCornerCopy[0].x==-10&&MapCornerCopy[0].y==8);
}
void callback_reload_controls(){
    reset();scenario=6;window.draw_map(0);require(alt_calls==8);
    require(events('F')[0].a==0); // Live origin, not the pre-conversion one.
    reset();scenario=7;window.draw_map(0);auto f=events('F');
    require(f[0].a==0&&f[0].c==20&&f[0].d==90);
    require(f[1].b==78&&f[1].c==120&&f[1].d==12);
    require(same(events('G')[0].clip,RECT{0,0,20,90}));
    reset();scenario=8;window.draw_map(0);f=events('F');
    require(f[0].a==80&&f[0].d==80&&f[1].b==84&&f[1].c==90&&f[1].d==12);
    require(same(events('G')[1].clip,RECT{0,74,90,96}));
    reset();scenario=9;window.field_1DD80_=1;window.draw_map(1);
    kinds("DPAAAAPAAAABFFGU");require(gen_calls==1&&clip_calls==0);
    reset();scenario=10;window.draw_map(0);
    require(same(events('G')[1].v,MapTileDamageRect{12,11,20,21}));
    reset();scenario=11;window.draw_map(0);require(events('F')[0].a==80);
}
struct GeometryCase {int ox,oy,nx,ny,flat;MapTileDamageRect h,v;};
void original_geometry_vectors(){
    // Hand-evaluated from the independent original four branch arms and
    // one add/subtract wrap. No candidate helper is used as the oracle.
    const GeometryCase cases[]={
        {10,10,12,12,0,{15,12,21,18},{12,11,20,21}},
        {10,10,18,12,0,{15,12,27,18},{18,11,26,21}},
        {12,12,10,10,0,{9,10,14,16},{10,6,18,16}},
        {10,10,30,12,0,{28,12,12,18},{30,11,38,21}},
        {10,10,30,12,1,{28,12,112,18},{30,11,38,21}},
        {10,10,30,12,2,{28,12,12,18},{30,11,38,21}},
        {30,12,10,10,0,{36,10,18,16},{10,6,18,16}},
        {98,10,2,12,0,{4,12,10,18},{2,11,10,21}},
        {-10,10,-8,12,0,{95,12,1,18},{92,11,0,21}},
        {-10,10,-8,12,1,{-5,12,1,18},{-8,11,0,21}},
        {-210,10,-208,12,0,{-105,12,-99,18},{-108,11,-100,21}},
    };
    for(const auto&t:cases){reset();origins(t.ox,t.oy,t.nx,t.ny);MapIsFlat=t.flat;
        window.draw_map(0);require(same(window.horizontal_damage_,t.h)&&same(window.vertical_damage_,t.v));}
}
int main(){no_scroll_controls();both_strips();single_strips_and_negative_average();callback_reload_controls();original_geometry_vectors();}
'''


def run_host(tmp_path, parts):
    compiler = shutil.which("clang++") or shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler required")
    source, binary = tmp_path / "scrolling.cpp", tmp_path / "scrolling"
    text = re.sub(r"\b(" + "|".join(parts) + r")\b", lambda match: parts[match.group()], HARNESS)
    source.write_text(text)
    compiled = subprocess.run([compiler, "-std=c++11", "-O1", "-fsanitize=address,undefined",
                               "-fno-sanitize-recover=all", str(source), "-o", str(binary)],
                              capture_output=True, text=True, timeout=30)
    assert compiled.returncode == 0, compiled.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True, timeout=10,
                          env=dict(os.environ, ASAN_OPTIONS="detect_leaks=0"))


def test_actual_mapwin_scrolling(tmp_path):
    result = run_host(tmp_path, actual_source())
    assert result.returncode == 0, result.stderr


# Every damage control compiles the whole actual controller. Rejection must be
# an observable geometry/state/order assertion, not a compiler error. These
# mutations do not turn mocks or reimplemented controllers into evidence.
MUTATIONS = [
    ("ignore_horizontal_centering", "field_1DDAC_ != 0 || field_1DDB0_ != 0", "field_1DDB0_ != 0"),
    ("ignore_vertical_centering", "field_1DDAC_ != 0 || field_1DDB0_ != 0", "field_1DDAC_ != 0"),
    ("repaint_unchanged", "old_x == new_x && old_y == new_y", "old_x == new_x && old_y == new_y && draw_type == 0"),
    ("incomplete_column_extent", "tile_width = field_1DDCC_ + field_1DDD4_", "tile_width = field_1DDCC_"),
    ("incomplete_row_extent", "tile_height = field_1DDD0_ + field_1DDD8_", "tile_height = field_1DDD0_"),
    ("seam_at_equality", "new_x - old_x > tile_width", "new_x - old_x >= tile_width"),
    ("flat_wrap", "(MapIsFlat & 1) == 0", "(MapIsFlat & 1) != 0"),
    ("repeat_wrap", "left += MapLongitudeBounds;", "do { left += MapLongitudeBounds; } while (left < 0);"),
    ("horizontal_padding", "horizontal_damage_.left = left - 1", "horizontal_damage_.left = left"),
    ("vertical_padding", "tile_height + old_y - 1 - 4", "tile_height + old_y - 1"),
    ("old_projection_origin", "const int altitude_x = field_1DDA4_;", "const int altitude_x = new_x;"),
    ("reload_each_altitude", "get_alt(altitude_x, altitude_y, 3)", "get_alt(field_1DDA4_, field_1DDA8_, 3)"),
    ("negative_average_truncation", "-1 - ((-1 - altitude) / 4)", "altitude / 4"),
    ("old_negative_average_truncation",
     "old_pixel_y += altitude >= 0 ? altitude / 4\n                                    : -1 - ((-1 - altitude) / 4);",
     "old_pixel_y += altitude / 4;"),
    ("three_corners", "corner < 4", "corner < 3"),
    ("wrong_corner_owner", "MapCornerCopy[corner]", "MapCornerScratch[corner]"),
    ("corner_translation_sign", ".x -= dx", ".x += dx"),
    ("blit_source_sign", "dx > 0 ? dx : 0", "dx < 0 ? -dx : 0"),
    ("cached_seam_origin", "const int live_x = field_1DDA4_;", "const int live_x = new_x;"),
    ("cached_seam_extent", "tile_distance > field_1DDD4_ + field_1DDCC_", "tile_distance > tile_width"),
    ("ignore_seam", "crosses_seam ? live_x < old_x : dx < 0", "dx < 0"),
    ("cached_pre_copy_height", "const int pixel_height = -canvas->dib_.bmiHeader.biHeight;", "const int pixel_height = 80;"),
    ("cached_pre_fill_height", "const int live_height = -canvas->dib_.bmiHeader.biHeight;", "const int live_height = pixel_height;"),
    ("cached_pre_fill_width", "vertical_pixels.right = canvas->dib_.bmiHeader.biWidth;", "vertical_pixels.right = pixel_width;"),
    ("skip_zero_width_fill", "GraphicWin::fill(horizontal_pixels.left", "if (exposed_width != 0) GraphicWin::fill(horizontal_pixels.left"),
    ("skip_zero_height_fill", "GraphicWin::fill(0, vertical_pixels.top", "if (exposed_height != 0) GraphicWin::fill(0, vertical_pixels.top"),
    ("ignore_fill_invalidation", "if (field_21A44_ == 0)", "if (false)"),
    ("missing_clip_overlap", "vertical_pixels.top -= 10;", "vertical_pixels.top -= 0;"),
    ("shared_rectangle_snapshot", "saved_horizontal = horizontal_damage_", "saved_horizontal = saved_vertical"),
    ("lost_vertical_restore", "vertical_damage_ = saved_vertical;", ";"),
    ("wrong_horizontal_exclusion", "horizontal_damage_.left = horizontal_damage_.right;", "horizontal_damage_.right = horizontal_damage_.left;"),
    ("wrong_vertical_exclusion", "vertical_damage_.left = horizontal_damage_.right;", "vertical_damage_.right = horizontal_damage_.right;"),
    ("ignore_state_faction", "(GameState & 0x80) != 0", "false"),
    ("cached_second_faction",
     "faction = (GameState & 0x80) != 0\n                          ? -1 : ConsoleGlobal.faction_id_;",
     "faction = faction;"),
    ("wrong_draw_only", "draw_type < 0", "draw_type > 0"),
    ("no_mouse_feedback", "if (field_1DD80_ != 0)", "if (false)"),
    ("wrong_mouse_receiver", "ConsoleGlobal.get_mouse_pos", "this->get_mouse_pos"),
    ("no_clip_restore", "canvas->set_clip(&canvas->rect2_);", ";"),
    ("no_negative_update", "if (draw_type != 0)", "if (draw_type > 0)"),
]


@pytest.mark.parametrize("name,old,new", MUTATIONS, ids=[item[0] for item in MUTATIONS])
def test_scrolling_damage_controls(tmp_path, name, old, new):
    parts = actual_source()
    assert old in parts["BODY"], name
    parts["BODY"] = parts["BODY"].replace(old, new)
    result = run_host(tmp_path, parts)
    assert result.returncode == 96 and "assertion line" in result.stderr, (name, result.stderr)
