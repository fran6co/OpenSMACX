"""Complete actual radius controllers with original-derived scan observations.

Original spans: tile_in_view4216F0..42180C, vehicles468F90..469183,
gen_radius469190..469718, draw_radius46A2A0..46A496 and nullable adapter
4712D0..471316. In particular469333..469355 converts left/top then right/top;
4693A4..4693BB uses distance for negative rows but parity for positive rows.

Five bodies and their arithmetic/view helpers, real hierarchy clauses, relevant
fields/signatures, Vert/Map types and shared corner definition are extracted.
Clip calculation, coordinate conversion, terrain/vehicle/overlay rendering,
textures and window presentation are recording service boundaries. This is not
native ABI, asset loading, callee closure or game/runtime acceptance. Fixtures
use live objects, small nonnegative radii, representable induction arithmetic,
valid cursor indices and initialized map storage. A separate negative-extent
vehicle-only case exercises its explicit early return without scanning.
Nullable behavior is exercised
only for the genuine free adapter, never by invoking a member on null.
"""

from pathlib import Path
import os
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


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
    h = {name: (ROOT / "src" / (name + ".h")).read_text() for name in
         ("win", "graphicwin", "mapwin", "console", "buffer", "vert", "map", "texture", "game",
          "game_data_state_window")}
    source = (ROOT / "src/mapwin.cpp").read_text()
    parts = {}
    for unit, cls in (("win", "Win"), ("graphicwin", "GraphicWin"), ("mapwin", "MapWin"), ("console", "Console")):
        parts[cls.upper() + "_BASES"], = re.findall(r"^class " + cls + r"\s*:[^{]+\{", h[unit], re.M)
    parts["TYPES"] = braced(h["vert"], "struct Vert {") + ";\n" + braced(h["map"], "struct Map {") + ";"
    fields = []
    for name in ("field_1DD70_", "field_1DD78_", "field_1DD98_", "field_1DDC0_", "field_1DDA4_", "field_1DDA8_",
                 "field_1DDCC_", "field_1DDD0_", "field_1DDD4_", "field_1DDD8_"):
        field, = re.findall(r"^  (?:int|int32_t|uint32_t) " + name + r";", h["mapwin"], re.M)
        fields.append(field)
    parts["MAP_FIELDS"] = "\n".join(fields)
    fields = []
    for name in ("faction_id_", "cursor_active_", "field_23BF8_", "survey_mode_", "field_23BE4_",
                 "cursor_history_index_", "cursor_history_x_", "cursor_history_y_"):
        field, = re.findall(r"^  (?:int32_t|uint32_t) " + name + r"(?:\[32\])?;", h["console"], re.M)
        fields.append(field)
    parts["CONSOLE_FIELDS"] = "\n".join(fields)
    parts["BUFFER_FIELDS"] = "\n".join(re.findall(r"^  RECT rect[12]_;", h["buffer"], re.M))
    assert len(parts["BUFFER_FIELDS"].splitlines()) == 2
    shared = []
    for unit, pattern in (("texture", r"^extern Texture g_RADIUS[12]_TEXTURE;"),
                          ("console", r"^extern Console ConsoleGlobal;")):
        shared.extend(decl.removeprefix("extern ") for decl in re.findall(pattern, h[unit], re.M))
    assert len(shared) == 3
    parts["SHARED_DECLARATIONS"] = "\n".join(shared)
    for key, unit, pattern in (
        ("MAP_METHODS", "mapwin", r"^  (?:int|void) (?:tile_in_view|draw_vehicles|gen_radius|draw_radius|compute_clip|tile_to_pixel|pixel_to_tile|draw_base_names|gen_overlays|draw_legend)\([^;{}]*\);"),
        ("GRAPHIC_METHODS", "graphicwin", r"^  (?:int|void) (?:fill\(RECT \*area, int color|soft_update\(RECT \*area)\);"),
        ("CLIP_METHOD", "buffer", r"^  int set_clip\(RECT \*rect\);"),
        ("TEXTURE_METHOD", "texture", r"^  void draw_trans\(Buffer \*buffer, Vert \*verts, int use_eight_vertices\);"),
    ):
        parts[key] = "\n".join(re.findall(pattern, h[unit], re.M))
        assert parts[key], key
    declarations = []
    for name in ("gen_window_terrain", "gen_terrain_poly"):
        declaration, = re.findall(r"^int __cdecl " + name + r"\([^;{}]*\);", h["mapwin"], re.M)
        declarations.append(declaration)
    for unit, names in (("map", ("MapLongitude", "MapLongitudeBounds", "MapLatitudeBounds", "MapIsFlat")),
                        ("game", ("IsMultiplayerNet",)), ("console", ("ConsoleControlTurnActive",))):
        for name in names:
            decl, = re.findall(r"^extern (?:int|uint32_t|BOOL) " + name + r";", h[unit], re.M)
            declarations.append(decl.removeprefix("extern "))
    corners, = re.findall(r"^Vert MapCornerCopy\[4\];", source, re.M)
    declarations.append(corners)
    parts["DECLARATIONS"] = "\n".join(declarations)
    game_state_field, = re.findall(r"^    uint32_t game_state_;[ \t]*// \+0x030$", h["game_data_state_window"], re.M)
    state_storage, = re.findall(r"^extern GameDataStateWindow GameDataStateStorage;$", h["game_data_state_window"], re.M)
    parts["GAME_STATE_STORAGE"] = (
        "struct GameDataStateWindowFields {\n" + game_state_field +
        "\n};\nstruct GameDataStateWindow { GameDataStateWindowFields fields_; };\n" +
        state_storage.removeprefix("extern ") + "\n")
    parts["MAP_ACCESS"] = braced(h["map"], "inline Map *__cdecl map_loc(") + "\n" + braced(h["map"], "MEASURED inline BOOL __cdecl on_map(")
    parts["HELPERS"] = "\n".join(braced(source, sig) for sig in (
        "int map_dest_signed(", "int mapwin_dest_add(int left, int right) {", "int mapwin_dest_sub(int left, int right) {",
        "int mapwin_dest_mul(int left, int right) {", "bool map_dest_in_view(",
        "int map_radius_wrap_x(", "bool map_radius_valid(", "int map_radius_half_width("))
    for key, signature in (("VIEW", "int MapWin::tile_in_view("), ("ADAPTER", "int __cdecl gen_window_terrain("),
                           ("VEHICLES", "void MapWin::draw_vehicles(int x, int y, int radius,"),
                           ("GENERATE", "void MapWin::gen_radius("), ("DRAW", "void MapWin::draw_radius(int x, int y,")):
        parts[key] = braced(source, signature)
    return parts


HARNESS = r'''
#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <utility>
#include <algorithm>
#define __cdecl
#define MEASURED
typedef int BOOL;
struct RECT {int left,top,right,bottom;};
TYPES
struct AutoSound {virtual ~AutoSound(){}};
WIN_BASES public:};
struct Buffer {
  BUFFER_FIELDS
  const RECT &current_clip() const { return rect1_; }
  void restore_clip() { set_clip(&rect2_); }
  CLIP_METHOD
};
GRAPHICWIN_BASES public:GRAPHIC_METHODS};
MAPWIN_BASES public:MAP_FIELDS MAP_METHODS};
CONSOLE_BASES public:CONSOLE_FIELDS};
struct Texture {TEXTURE_METHOD};
MapWin window;SHARED_DECLARATIONS
DECLARATIONS
GAME_STATE_STORAGE
Map tiles[16000];int map_reads;
Map *map_tiles(){++map_reads;return tiles;}
MAP_ACCESS
typedef std::pair<int,int> Point;
struct Event {char kind;std::vector<int> a;};std::vector<Event> seen;
int scenario,clip_calls,left_y,right_y,conversion_fail,clip_results[2],adapter_mode;
int center_x,center_y,expected_faction,terrain_calls,vehicle_calls,overlay_calls;
RECT proposed_clip;
void check(bool value,int line){if(!value){std::fprintf(stderr,"assertion line %d scenario %d\n",line,scenario);std::exit(92);}}
#define require(x) check((x),__LINE__)
void note(char kind,std::initializer_list<int> values={}){Event e={kind,values};seen.push_back(e);}
bool same(const RECT&a,const RECT&b){return a.left==b.left&&a.top==b.top&&a.right==b.right&&a.bottom==b.bottom;}
int MapWin::compute_clip(int x,int y,int radius,RECT *clip){require(this==&window&&x==center_x&&y==center_y);note('C',{x,y,radius});
    *clip=proposed_clip;int index=clip_calls++;require(index<2);return clip_results[index];}
int Buffer::set_clip(RECT *clip){require(this==static_cast<Buffer*>(&window));note(clip==&rect2_?'R':'S',{clip->left,clip->top,clip->right,clip->bottom});rect1_=*clip;
    if(scenario==1){window.field_1DD70_|=0x10000000;ConsoleGlobal.faction_id_=7;}return -11;}
int GraphicWin::fill(RECT *clip,int color){require(this==static_cast<GraphicWin*>(&window)&&color==0x41);note('F',{clip->left,clip->top,clip->right,clip->bottom,color});return -13;}
int MapWin::pixel_to_tile(int x,int y,int *tx,int *ty){require(this==&window);note('Q',{x,y});*tx=0;
    if(!conversion_fail)*ty=x==proposed_clip.right?right_y:left_y;return conversion_fail;}
void MapWin::tile_to_pixel(int x,int y,int *px,int *py){require(this==&window);note('P',{x,y});*px=x*10+1;*py=y*10+2;
    if(scenario==2){field_1DD98_=uint32_t(-7);field_1DD70_=0x12345;field_1DDC0_=13;}}
int gen_terrain_poly(Buffer *canvas,MapWin *owner,int zoom,int faction,int base,int flags,int x,int y,int px,int py){
    if(adapter_mode){require((owner==nullptr&&canvas==nullptr)||(owner==&window&&canvas==static_cast<Buffer*>(&window)));
        note('A',{zoom,faction,base,flags,x,y,px,py});return -37;}
    require(owner==&window&&canvas==static_cast<Buffer*>(&window)&&base==-1&&faction==expected_faction);
    require(px==x*10+1&&py==y*10+2);note('T',{x,y,zoom,faction,flags});++terrain_calls;
    if(scenario==2)require(zoom==-7&&flags==0x12345);
    if(scenario==3){window.field_1DDA8_=159;window.field_1DDD0_=1;window.field_1DDD8_=0;}
    return -17;
}
void MapWin::draw_vehicles(int x,int y,int px,int py,int faction){require(this==&window&&faction==expected_faction);
    require(px==x*10+1&&py==y*10+2-(scenario==2?13:7));note('V',{x,y,px,py,faction});++vehicle_calls;
    if(scenario==4){field_1DDA8_=159;field_1DDD0_=1;field_1DDD8_=0;}}
void MapWin::draw_base_names(){require(this==&window);note('N');
    if(scenario==5)field_1DD70_|=0x22000;
    if(scenario==9)ConsoleGlobal.cursor_active_=1;
    if(scenario==10){field_1DD98_=uint32_t(-8);ConsoleGlobal.faction_id_=6;}}
void MapWin::gen_overlays(int x,int y){require(this==&window);note('O',{x,y});++overlay_calls;
    if(scenario==6)field_1DD70_|=0x20000;
    if(scenario==7){field_1DDA8_=159;field_1DDD0_=1;field_1DDD8_=0;}}
void MapWin::draw_legend(){require(this==&window);note('L');if(scenario==8)rect2_={1,2,90,91};}
void Texture::draw_trans(Buffer *canvas,Vert *vertices,int eight){require(canvas==static_cast<Buffer*>(&window)&&vertices==MapCornerCopy&&eight==0);
    require(vertices[0].x==11&&vertices[0].y==12&&vertices[3].x==41&&vertices[3].y==42);
    require(this==&g_RADIUS1_TEXTURE||this==&g_RADIUS2_TEXTURE);note('X',{this==&g_RADIUS1_TEXTURE?1:2});}
void GraphicWin::soft_update(RECT *clip){require(this==static_cast<GraphicWin*>(&window)&&same(*clip,proposed_clip));note('U');}
void do_all_draws(){note('D');}
HELPERS
VIEW
ADAPTER
VEHICLES
GENERATE
DRAW
std::vector<Event> events(char kind){std::vector<Event> out;for(const auto&e:seen)if(e.kind==kind)out.push_back(e);return out;}
std::vector<Point> points(char kind){std::vector<Point> out;for(const auto&e:seen)if(e.kind==kind)out.push_back({e.a[0],e.a[1]});return out;}
void reset(){
    scenario=clip_calls=conversion_fail=adapter_mode=terrain_calls=vehicle_calls=overlay_calls=map_reads=0;
    center_x=center_y=50;expected_faction=3;left_y=right_y=48;clip_results[0]=clip_results[1]=1;
    proposed_clip={4,6,80,90};window.rect1_=proposed_clip;window.rect2_={0,0,200,160};
    window.field_1DD70_=window.field_1DD78_=0;window.field_1DD98_=0;window.field_1DDC0_=7;
    window.field_1DDA4_=window.field_1DDA8_=0;window.field_1DDCC_=100;window.field_1DDD4_=100;window.field_1DDD0_=80;window.field_1DDD8_=80;
    MapLongitudeBounds=200;MapLongitude=100;MapLatitudeBounds=160;MapIsFlat=0;GameDataStateStorage.fields_.game_state_=0;IsMultiplayerNet=ConsoleControlTurnActive=0;
    ConsoleGlobal.faction_id_=3;ConsoleGlobal.cursor_active_=0;ConsoleGlobal.field_23BF8_=1;ConsoleGlobal.survey_mode_=0;ConsoleGlobal.field_23BE4_=1;
    ConsoleGlobal.cursor_history_index_=2;for(int i=0;i<32;++i){ConsoleGlobal.cursor_history_x_[i]=-99;ConsoleGlobal.cursor_history_y_[i]=-99;}
    ConsoleGlobal.cursor_history_x_[2]=50;ConsoleGlobal.cursor_history_y_[2]=50;
    for(int i=0;i<4;++i){MapCornerCopy[i].x=11+10*i;MapCornerCopy[i].y=12+10*i;}
    std::memset(tiles,0,sizeof(tiles));seen.clear();
}
std::vector<Point> profile(int extent,int widening,bool viewport=true){
    // Independent row-profile oracle: a rising northern triangle through
    // the center, then alternating southern strips through extent+1.
    // The original is NOT a symmetric Manhattan-radius diamond.
    std::vector<int> widths;for(int n=0;n<=extent;++n)widths.push_back(n+widening);
    for(int n=1;n<=extent+1;++n)widths.push_back(extent-(n%2)+widening);
    std::vector<Point> out;
    for(size_t line=0;line<widths.size();++line){int y=center_y-int(extent)+int(line),w=widths[line];
        for(int point=0;point<=w;++point){int x=center_x-w+2*point;
            if(!(MapIsFlat&1)){if(x<0)x+=MapLongitudeBounds;else if(x>=MapLongitudeBounds)x-=MapLongitudeBounds;}
            if(x<0||x>=MapLongitudeBounds||y<0||y>=MapLatitudeBounds)continue;
            if(viewport&&(x<window.field_1DDA4_||x>=window.field_1DDA4_+window.field_1DDCC_+window.field_1DDD4_||
                y<window.field_1DDA8_||y>=window.field_1DDA8_+window.field_1DDD0_+window.field_1DDD8_))continue;
            out.push_back({x,y});}}
    return out;
}
void adapter_and_view(){
    reset();adapter_mode=1;require(gen_window_terrain(nullptr,1,2,3,4,5,6,7,8)==-37);
    require(gen_window_terrain(&window,11,12,13,14,15,16,17,18)==-37);
    require(seen.size()==2&&seen[0].a==std::vector<int>({1,2,3,4,5,6,7,8})&&seen[1].a==std::vector<int>({11,12,13,14,15,16,17,18}));
    reset();window.field_1DDA4_=10;window.field_1DDA8_=20;window.field_1DDCC_=8;window.field_1DDD4_=12;window.field_1DDD0_=9;window.field_1DDD8_=11;
    require(window.tile_in_view(10,20,0)==1&&window.tile_in_view(29,39,0)==1);
    require(window.tile_in_view(9,20,0)==0&&window.tile_in_view(30,20,0)==0&&window.tile_in_view(10,19,0)==0&&window.tile_in_view(10,40,0)==0&&map_reads==0);
    MapIsFlat=1;require(window.tile_in_view(30,20,0)==0);MapIsFlat=0;
    require(window.tile_in_view(12,22,2)==1&&window.tile_in_view(27,37,2)==1);
    require(window.tile_in_view(11,22,2)==0&&window.tile_in_view(28,22,2)==0&&window.tile_in_view(12,21,2)==0&&window.tile_in_view(12,38,2)==0);
    tiles[22*100+6].climate=0x80;require(window.tile_in_view(12,22,2)==0);
    tiles[23*100+6].climate=0x80;require(window.tile_in_view(12,23,2)==1);
    tiles[22*100+6].climate=0x7f;require(window.tile_in_view(12,22,2)==1);
    window.field_1DDA4_=190;require(window.tile_in_view(2,22,0)==1);MapIsFlat=1;require(window.tile_in_view(2,22,0)==0);
    MapIsFlat=2;require(window.tile_in_view(2,22,0)==1);
    window.field_1DDA4_=-5;window.field_1DDA8_=-5;MapIsFlat=1;
    int before=map_reads;require(window.tile_in_view(-1,-1,1)==1&&map_reads==before);
}
void complete_scans(){
    for(int radius=0;radius<3;++radius){reset();window.field_1DD70_=0x22000;
        auto ground=profile(2*radius+4,radius==0?2:0);auto overlay=profile(2*radius+4,0);
        auto all_ground=ground,all_overlay=overlay;if(radius==0){all_ground.insert(all_ground.begin(),{{49,47},{51,47}});all_overlay.insert(all_overlay.begin(),{{49,47},{51,47}});}
        window.gen_radius(center_x,center_y,radius,3,0);
        require(points('T')==all_ground&&points('V')==ground&&points('O')==all_overlay);
        require(events('F').size()==size_t(radius==0)&&events('N').size()==1&&events('L').size()==1&&seen.back().kind=='R');
        require(same(window.rect1_,window.rect2_));
        if(radius==0)require(ground.size()==57&&overlay.size()==37);
        if(radius==1)require(ground.size()==73&&overlay.size()==73);
        size_t last_ground=0,first_vehicle=seen.size(),names=0,first_overlay=seen.size(),legend=0;
        for(size_t i=0;i<seen.size();++i){if(seen[i].kind=='T')last_ground=i;if(seen[i].kind=='V')first_vehicle=std::min(first_vehicle,i);
            if(seen[i].kind=='N')names=i;if(seen[i].kind=='O')first_overlay=std::min(first_overlay,i);if(seen[i].kind=='L')legend=i;}
        require(last_ground<first_vehicle&&first_vehicle<names&&names<first_overlay&&first_overlay<legend);
    }
    reset();window.field_1DD70_=0x22000;window.gen_radius(50,50,0,3,1);
    require(terrain_calls==0&&events('F').empty()&&vehicle_calls==57&&overlay_calls==39);
    reset();clip_results[0]=0;window.gen_radius(50,50,0,3,0);require(seen.size()==1&&seen[0].kind=='C');
    reset();conversion_fail=1;center_y=100;window.gen_radius(50,100,0,3,0);require(vehicle_calls==57&&terrain_calls==59);
    reset();left_y=40;right_y=36;window.field_1DD70_=0x2000;
    auto terrain=profile(16,2),vehicles=profile(12,2),overlays=profile(12,0);
    terrain.insert(terrain.begin(),{{49,47},{51,47}});overlays.insert(overlays.begin(),{{49,47},{51,47}});
    window.gen_radius(50,50,0,3,0);require(points('T')==terrain&&points('V')==vehicles&&points('O')==overlays);
    reset();left_y=0;auto limited=profile(12,2);window.draw_vehicles(50,50,0,3);require(points('V')==limited);
    reset();left_y=32000;window.draw_vehicles(50,50,-3,3);require(seen.size()==1&&seen[0].kind=='Q');
}
void scan_boundaries_and_callbacks(){
    for(int flat=0;flat<3;++flat){reset();center_x=0;MapLongitudeBounds=20;MapLongitude=10;MapIsFlat=flat;
        auto expected=profile(4,2);window.draw_vehicles(0,50,0,3);require(points('V')==expected);}
    reset();center_x=-40;MapLongitudeBounds=20;MapLongitude=10;window.draw_vehicles(-40,50,0,3);require(vehicle_calls==0);
    reset();center_x=0;MapIsFlat=2;window.field_1DDA4_=190;window.field_1DDCC_=10;window.field_1DDD4_=10;
    auto wrapped=profile(4,2,false);wrapped.erase(std::remove_if(wrapped.begin(),wrapped.end(),[](Point p){return p.first>=10&&p.first<190;}),wrapped.end());
    window.draw_vehicles(0,50,0,3);require(points('V')==wrapped);
    reset();center_y=1;left_y=right_y=0;auto expected=profile(4,2);window.draw_vehicles(50,1,0,3);require(points('V')==expected);
    reset();window.field_1DDA4_=48;window.field_1DDCC_=3;window.field_1DDD4_=2;window.field_1DDA8_=49;window.field_1DDD0_=2;window.field_1DDD8_=1;
    expected=profile(4,2);window.draw_vehicles(50,50,0,3);require(points('V')==expected);
    reset();scenario=1;window.gen_radius(50,50,0,3,0);require(events('F').empty()&&terrain_calls==59);
    reset();scenario=2;window.gen_radius(50,50,1,3,0);require(terrain_calls==73&&vehicle_calls==73);
    reset();scenario=3;window.gen_radius(50,50,1,3,0);require(terrain_calls==1&&vehicle_calls==0);
    reset();scenario=4;window.draw_vehicles(50,50,1,3);require(vehicle_calls==1);
    reset();scenario=5;window.gen_radius(50,50,1,3,0);require(overlay_calls==73&&events('L').size()==1);
    reset();scenario=6;window.field_1DD70_=0x2000;window.gen_radius(50,50,1,3,0);require(events('L').size()==1);
    reset();scenario=7;window.field_1DD70_=0x2000;window.gen_radius(50,50,1,3,0);require(overlay_calls==1);
    reset();scenario=8;window.field_1DD70_=0x20000;window.gen_radius(50,50,1,3,0);require(same(window.rect1_,RECT{1,2,90,91}));
}
void draw_controls(){
    // Clip refusal does not suppress the post-generation selection marker.
    reset();clip_results[0]=clip_results[1]=0;window.draw_radius(50,50,0,0);require(seen.size()==2&&seen[0].kind=='C'&&seen[1].kind=='X');
    for(int gate=0;gate<8;++gate){reset();clip_results[0]=clip_results[1]=0;
        switch(gate){case 0:ConsoleGlobal.cursor_active_=1;break;case 1:IsMultiplayerNet=ConsoleControlTurnActive=1;break;
        case 2:ConsoleGlobal.field_23BF8_=ConsoleGlobal.survey_mode_=0;break;case 3:ConsoleGlobal.field_23BE4_=0;break;
        case 4:window.field_1DD78_=1;break;case 5:ConsoleGlobal.cursor_history_y_[2]=-1;break;
        case 6:ConsoleGlobal.cursor_history_x_[2]=-250;break;case 7:ConsoleGlobal.cursor_history_y_[2]=160;break;}
        window.draw_radius(50,50,0,0);require(events('X').empty());}
    reset();clip_results[0]=0;ConsoleGlobal.field_23BF8_=0;ConsoleGlobal.survey_mode_=2;window.field_1DD78_=1;window.field_1DD70_=0x80000000;
    ConsoleGlobal.field_23BE4_=0;IsMultiplayerNet=1;GameDataStateStorage.fields_.game_state_=2;window.field_1DD98_=uint32_t(-8);window.draw_radius(50,50,0,0);require(events('X').size()==1&&events('X')[0].a[0]==2);
    for(int draw=-1;draw<4;++draw){reset();clip_results[0]=0;window.draw_radius(50,50,0,draw);
        require(events('U').size()==size_t(draw!=0)&&events('D').size()==size_t(draw>=2)&&events('X').size()==size_t(draw>=0));
        if(draw>=2)require(seen[seen.size()-2].kind=='U'&&seen.back().kind=='D');}
    reset();GameDataStateStorage.fields_.game_state_=0x80;expected_faction=-1;window.draw_radius(50,50,0,-1);require(terrain_calls==0&&vehicle_calls==57&&events('U').size()==1&&events('X').empty());
    reset();scenario=9;window.draw_radius(50,50,1,0);require(events('X').empty());
    reset();scenario=10;window.draw_radius(50,50,1,0);require(events('X').size()==1&&events('X')[0].a[0]==2);
    reset();clip_results[0]=clip_results[1]=0;window.draw_radius(50,50,0,2);require(events('U').empty()&&events('D').empty());
    reset();clip_results[0]=0;window.field_1DD98_=uint32_t(-7);window.draw_radius(50,50,0,0);require(events('X').size()==1&&events('X')[0].a[0]==1);
}
int main(){adapter_and_view();complete_scans();scan_boundaries_and_callbacks();draw_controls();}
'''


def run_host(tmp_path, parts):
    compiler = shutil.which("clang++") or shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler required")
    source, binary = tmp_path / "radius.cpp", tmp_path / "radius"
    source.write_text(re.sub(r"\b(" + "|".join(parts) + r")\b", lambda m: parts[m.group()], HARNESS))
    compiled = subprocess.run([compiler, "-std=c++11", "-O1", "-fsanitize=address,undefined", "-fno-sanitize-recover=all",
                               str(source), "-o", str(binary)], capture_output=True, text=True, timeout=30)
    assert compiled.returncode == 0, compiled.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True, timeout=10,
                          env=dict(os.environ, ASAN_OPTIONS="detect_leaks=0"))


def test_actual_mapwin_radius(tmp_path):
    result = run_host(tmp_path, actual_source())
    assert result.returncode == 0, result.stderr


# Every alteration must compile and reach an explicit observation failure.
# Crashes, undefined behavior and compiler rejection are not damage coverage.
DAMAGE = [
    ("adapter_return", "ADAPTER", "return gen_terrain_poly(", "return 1 + gen_terrain_poly("),
    ("adapter_arguments", "ADAPTER", "zoom, faction, base_id, flags,", "faction, zoom, base_id, flags,"),
    ("adapter_base", "ADAPTER", "static_cast<Buffer *>(window)", "static_cast<Buffer *>(nullptr)"),
    ("climate_margin_gate", "VIEW", "margin != 0 &&", "margin == 0 &&"),
    ("climate_threshold", "VIEW", "> 0x60", "> 0x80"),
    ("climate_altitude", "VIEW", "climate >> 5", "climate >> 6"),
    ("view_bottom_exclusive", "VIEW", "y >= bottom", "y > bottom"),
    ("view_right_exclusive", "VIEW", "x < right;", "x <= right;"),
    ("view_margin_left", "VIEW", "mapwin_dest_add(field_1DDA4_, margin)", "field_1DDA4_"),
    ("view_margin_bottom", "VIEW", "mapwin_dest_sub(mapwin_dest_add(field_1DDD8_, field_1DDD0_), margin)", "mapwin_dest_add(field_1DDD8_, field_1DDD0_)"),
    ("flat_bit_not_boolean", "HELPERS", "(MapIsFlat & 1) == 0", "MapIsFlat == 0"),
    ("southern_parity", "HELPERS", "row & 1", "row"),
    ("northern_width", "HELPERS", "mapwin_dest_sub(0, row)", "0"),
    ("vehicle_cap", "VEHICLES", "span < 10", "span < 8"),
    ("vehicle_radius_widening", "VEHICLES", "half_width, 2", "half_width, 0"),
    ("vehicle_last_row", "VEHICLES", "row <= extent + 1", "row <= extent"),
    ("vehicle_step", "VEHICLES", "column += 2", "column += 1"),
    ("vehicle_live_vertical_center", "VEHICLES", "mapwin_dest_sub(pixel_y, field_1DDC0_)", "mapwin_dest_sub(pixel_y, 7)"),
    ("vehicle_view_filter", "VEHICLES", "map_dest_in_view(*this, tile_x, tile_y)", "true"),
    ("clip_refusal", "GENERATE", "compute_clip(x, y, radius, &clip) == 0", "compute_clip(x, y, radius, &clip) != 0"),
    ("fill_flag", "GENERATE", "(field_1DD70_ & 0x10000000U) == 0", "true"),
    ("fill_color", "GENERATE", "fill(&clip, 0x41)", "fill(&clip, 0x40)"),
    ("prelude_y", "GENERATE", "mapwin_dest_sub(y, 3)", "mapwin_dest_sub(y, 2)"),
    ("right_conversion", "GENERATE", "pixel_to_tile(clip.right, clip.top", "pixel_to_tile(clip.left, clip.top"),
    ("conversion_sentinel", "GENERATE", "int upper_left_y = 32000", "int upper_left_y = 48"),
    ("terrain_radius_widening", "GENERATE", "half_width, 2", "half_width, 0"),
    ("terrain_skip", "GENERATE", "if (skip_terrain == 0)", "if (true)"),
    ("terrain_live_zoom", "GENERATE", "gen_window_terrain(this, field_1DD98_", "gen_window_terrain(this, 0"),
    ("terrain_faction_snapshot", "GENERATE", "gen_window_terrain(this, field_1DD98_, faction", "gen_window_terrain(this, field_1DD98_, ConsoleGlobal.faction_id_"),
    ("terrain_live_view", "GENERATE", "map_dest_in_view(*this, tile_x, tile_y)", "true"),
    ("names_missing", "GENERATE", "draw_base_names();", "/* omitted */"),
    ("overlays_flag", "GENERATE", "(field_1DD70_ & 0x2000) != 0", "false"),
    ("overlay_left_only", "GENERATE", "while (upper_left_y < mapwin_dest_sub(y, span))", "while (upper_y < mapwin_dest_sub(y, span))"),
    ("overlay_unwidened", "GENERATE", "map_radius_half_width(overlay_extent, row)", "mapwin_dest_add(map_radius_half_width(overlay_extent, row), radius == 0 ? 2 : 0)"),
    ("overlay_live_view", "GENERATE", "tile_in_view(tile_x, tile_y, 0)", "true"),
    ("legend_flag", "GENERATE", "(field_1DD70_ & 0x20000) != 0", "false"),
    ("restore_current_clip", "GENERATE", "canvas->restore_clip()", "canvas->set_clip(&clip)"),
    ("spectator_faction", "DRAW", "(GameDataStateStorage.fields_.game_state_ & 0x80) != 0", "false"),
    ("negative_draw_skip", "DRAW", "faction, draw_type < 0", "faction, 0"),
    ("cursor_active", "DRAW", "ConsoleGlobal.cursor_active_ == 0", "true"),
    ("network_turn", "DRAW", "network == 0 || ConsoleControlTurnActive == 0", "true"),
    ("negative_no_marker", "DRAW", "draw_type >= 0", "true"),
    ("survey_alternative", "DRAW", "ConsoleGlobal.field_23BF8_ != 0 || ConsoleGlobal.survey_mode_ != 0", "ConsoleGlobal.field_23BF8_ != 0"),
    ("display_mode_gate", "DRAW", "field_1DD78_ == 0 || (field_1DD70_ & 0x80000000U) != 0", "true"),
    ("network_game_override", "DRAW", "network != 0 && (GameDataStateStorage.fields_.game_state_ & 2) != 0", "false"),
    ("history_index", "DRAW", "const int history = ConsoleGlobal.cursor_history_index_", "const int history = 0"),
    ("history_view", "DRAW", "map_dest_in_view(*this, cursor_x, cursor_y)", "true"),
    ("signed_zoom_boundary", "DRAW", "map_dest_signed(field_1DD98_) <= -8", "map_dest_signed(field_1DD98_) < -8"),
    ("texture_shared_count", "DRAW", "MapCornerCopy, 0", "MapCornerCopy, 1"),
    ("update_gate", "DRAW", "if (draw_type != 0)", "if (draw_type > 0)"),
    ("pump_threshold", "DRAW", "draw_type >= 2", "draw_type >= 3"),
    ("update_before_pump", "DRAW", "GraphicWin::soft_update(&clip);", "if (draw_type >= 2) do_all_draws(); GraphicWin::soft_update(&clip);"),
]


@pytest.mark.parametrize("name,part,before,after", DAMAGE, ids=[row[0] for row in DAMAGE])
def test_damage_is_observed(tmp_path, name, part, before, after):
    parts = actual_source()
    assert before in parts[part], (name, part)
    parts[part] = parts[part].replace(before, after, 1)
    result = run_host(tmp_path, parts)
    assert result.returncode == 92 and "assertion line" in result.stderr, (name, result.returncode, result.stderr)
