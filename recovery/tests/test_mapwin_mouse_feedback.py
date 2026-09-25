"""Complete actual mouse controller with independently decoded UI observations.

The controller and arithmetic/storage helpers, real inheritance clauses, selected
window declarations and complete Base/Veh/prototype/chassis definitions are
extracted unchanged. Window painting, selection, pumping, routing, labels and
fonts are controlled service boundaries, not recovered services. These bounded
controls do not prove native layouts, rendering, linkage or game acceptance.
Fixtures use valid IDs, waypoint counts 0..3, initialized prior endpoint Y,
positive road rates and nonzero surface width. Callback mutations probe original
load timing; they do not establish reachability through every real service.
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
         ("win", "graphicwin", "mapwin", "console", "buffer", "veh", "base", "alpha", "map", "strings", "font")}
    source = (ROOT / "src/mapwin.cpp").read_text()
    parts = {}
    for unit, cls in (("win", "Win"), ("graphicwin", "GraphicWin"),
                      ("mapwin", "MapWin"), ("console", "Console")):
        parts[cls.upper() + "_BASES"], = re.findall(r"^class " + cls + r"\s*:[^{]+\{", h[unit], re.M)
    structures = []
    for unit, name in (("base", "Base"), ("veh", "Veh"), ("veh", "VehPrototype"), ("veh", "RulesChassis"), ("alpha", "RulesBasic")):
        structures.append(braced(h[unit], "struct " + name + " {") + ";")
    parts["STRUCTURES"] = "\n".join(structures)
    parts["DIB"] = braced(h["buffer"], "struct Dib : BITMAPINFO") + ";"
    members = []
    for name in ("field_1DD78_", "field_1DD80_", "field_1DD84_", "field_1DD8C_", "field_1DDB8_",
                 "feedback_x_", "feedback_y_", "field_1DE0C_", "feedback_hover_y_", "feedback_proto_",
                 "feedback_triad_", "feedback_repeat_", "feedback_segment_count_", "field_1DE24_",
                 "cursor_sprite_", "cursor_resource_id_"):
        declaration, = re.findall(r"^  (?:uint32_t|int|Sprite)\s+\*?" + name + r"(?:\[5\])?;", h["mapwin"], re.M)
        members.append(declaration)
    parts["MAP_MEMBERS"] = "\n".join(members)
    members = []
    for name in ("selected_vehicle_", "faction_id_", "field_23D10_"):
        declaration, = re.findall(r"^  (?:int32_t|uint32_t) " + name + r";", h["console"], re.M)
        members.append(declaration)
    parts["CONSOLE_MEMBERS"] = "\n".join(members)
    parts["MOUSE_DEFAULT"] = braced(h["win"], "virtual void on_mouse_feedback(")
    for key, unit, pattern in (
        ("MAP_METHODS", "mapwin", r"^  (?:virtual )?(?:void|int) (?:on_mouse_feedback|pixel_to_tile|editor|dest_line)\([^;{}]*\);"),
        ("WIN_METHODS", "win", r"^  (?:static )?int (?:set_cursor\(int name|update_cursor\(Win \*window, int tgl)\);"),
        ("GRAPHIC_METHODS", "graphicwin", r"^  void (?:fill\(int color|soft_update\()\);"),
        ("BUFFER_METHODS", "buffer", r"^  (?:int|void) (?:set_text_color|set_font|text_width|write_l)\([^;{}]*\);"),
        ("SELECTING", "console", r"^  int selecting\([^;{}]*\);"),
    ):
        parts[key] = "\n".join(re.findall(pattern, h[unit], re.M))
        assert parts[key], key
    declarations = []
    for unit, names in (("mapwin", ("MapInputEnabled", "MapMouseFeedbackBusy", "MapDestinationCost")),
                        ("console", ("ConsoleControlTurnActive",)), ("base", ("UnkGlobal0093A934",)),
                        ("map", ("MapLongitudeBounds", "MapLatitudeBounds")), ("strings", ("StringTemp",))):
        for name in names:
            declaration, = re.findall(r"^extern (?:int|uint32_t|char) " + name + r"(?:\[\d+\])?;", h[unit], re.M)
            declarations.append(declaration.removeprefix("extern "))
    for unit, name in (("font", "find_font"), ("veh", "say_loc"), ("alpha", "label_get")):
        declaration, = re.findall(r"^(?:Font \*|void |LPSTR )__cdecl " + name + r"\([^;{}]*\);", h[unit], re.M)
        declarations.append(declaration)
    parts["GLOBALS"] = "\n".join(declarations)
    parts["HELPERS"] = "\n".join(braced(source, signature) for signature in (
        "int mouse_feedback_signed(", "int mouse_feedback_add(", "int mouse_feedback_sub(", "int mouse_feedback_mul(",
        "void mouse_feedback_store_x(", "void mouse_feedback_store_y("))
    parts["BODY"] = braced(source, "void MapWin::on_mouse_feedback(")
    return parts


HARNESS = r'''
#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <string>
#include <array>
#include <limits>
#define __cdecl
typedef int BOOL;
typedef char *LPSTR;
struct RECT {int32_t left,top,right,bottom;};
struct RGBQUAD {uint8_t blue,green,red,reserved;};
struct BITMAPINFOHEADER {uint32_t biSize;int32_t biWidth,biHeight;uint16_t biPlanes,biBitCount;
 uint32_t biCompression,biSizeImage;int32_t biXPelsPerMeter,biYPelsPerMeter;uint32_t biClrUsed,biClrImportant;};
struct BITMAPINFO {BITMAPINFOHEADER bmiHeader;RGBQUAD bmiColors[1];};
struct Font{};struct Sprite{};struct AutoSound{virtual ~AutoSound(){}};
DIB
STRUCTURES
static_assert(sizeof(Veh)==52&&offsetof(Veh,waypoint_count)==18&&offsetof(Veh,waypoint_x)==20&&offsetof(Veh,waypoint_y)==28,"original vehicle words");
static_assert(sizeof(VehPrototype)==52&&offsetof(VehPrototype,chassis_id)==36,"original prototype chassis byte");
WIN_BASES public:MOUSE_DEFAULT WIN_METHODS};
struct Buffer {Dib dib_;BUFFER_METHODS};
GRAPHICWIN_BASES public:GRAPHIC_METHODS};
MAPWIN_BASES public:MAP_MEMBERS MAP_METHODS};
CONSOLE_BASES public:CONSOLE_MEMBERS SELECTING};
MapWin window;Console ConsoleGlobal;GraphicWin CoverWindow;Font font;Sprite sprite;
GLOBALS
Base Bases[4];Veh Vehs[4];VehPrototype VehPrototypes[16];RulesChassis Chassis[4];RulesBasic Rules;
struct Event {char kind;std::vector<int> args;std::string text;int busy;};
std::vector<Event> seen;
std::vector<std::array<int,11>> segments;
int scenario,pixel_result,selected_result,tile_x,tile_y,pixel_calls,pump_calls;
int text_width_result,write_x,write_y,write_length;
std::string written;
void check(bool value,int line){if(!value){std::fprintf(stderr,"assertion line %d scenario %d\n",line,scenario);std::exit(95);}}
#define require(x) check((x),__LINE__)
void note(char kind,std::initializer_list<int> args={},const char *text=""){
    Event event={kind,args,text,MapMouseFeedbackBusy};seen.push_back(event);
}
int MapWin::pixel_to_tile(int x,int y,int *tx,int *ty){
    require(this==&window);note('P',{x,y});++pixel_calls;require(pixel_calls==1);*tx=tile_x;*ty=tile_y;return pixel_result;
}
void MapWin::editor(int x,int y,int erase){require(this==&window);note('E',{x,y,erase});}
int Console::selecting(int x,int y){require(this==&ConsoleGlobal);note('S',{x,y});
    if(scenario==18)window.field_1DD80_=1;return selected_result;}
int Win::set_cursor(int id){require(this==static_cast<Win*>(static_cast<GraphicWin*>(&window)));note('R',{id});
    if(scenario==1){window.cursor_resource_id_=17;window.cursor_sprite_=&sprite;}return -1;}
void do_all_non_input(){
    note('N');require(MapMouseFeedbackBusy==1);
    if(scenario==2){window.on_mouse_feedback(999,999);}
    if(scenario==3){window.field_1DE0C_=tile_x;window.feedback_hover_y_=tile_y;}
    if(scenario==4){ConsoleGlobal.selected_vehicle_=1;ConsoleGlobal.field_23D10_=8;}
}
void GraphicWin::fill(int color){require(this==&CoverWindow);note('F',{color});
    if(scenario==5)ConsoleGlobal.selected_vehicle_=2;}
int Win::update_cursor(Win *w,int flag){
    require(w==nullptr&&flag==1&&MapMouseFeedbackBusy==0);note('U',{flag});++pump_calls;
    if(scenario==6&&pump_calls==1){Vehs[1].waypoint_count=2;Vehs[1].x=-9;Vehs[1].y=-7;Vehs[1].waypoint_x[0]=31;Vehs[1].waypoint_y[0]=41;}
    if(scenario==7&&pump_calls==1){window.field_1DE24_=2;}
    if(scenario==8){window.feedback_proto_=11;window.feedback_triad_=1;ConsoleGlobal.faction_id_=6;}
    if(scenario==17&&pump_calls==2)Vehs[1].waypoint_count=0;
    return -3;
}
void MapWin::dest_line(int x1,int y1,int x2,int y2,int proto,int faction,int triad,int repeat,int c1,int c2,int cover){
    require(this==&window);note('D',{x1,y1,x2,y2,proto,faction,triad,repeat,c1,c2,cover});
    segments.push_back({x1,y1,x2,y2,proto,faction,triad,repeat,c1,c2,cover});
    MapDestinationCost=7+int(segments.size());
    if(scenario==9&&segments.size()==1){Vehs[1].waypoint_count=0;ConsoleGlobal.selected_vehicle_=2;ConsoleGlobal.field_23D10_=8;}
    if(scenario==10&&segments.size()==1){window.feedback_repeat_=0;ConsoleGlobal.field_23D10_=1;ConsoleGlobal.faction_id_=5;}
    if(scenario==11){MapDestinationCost=-8;}
    if(scenario==12){MapDestinationCost=6;}
    if(scenario==15&&segments.size()==1)ConsoleGlobal.selected_vehicle_=2;
}
LPSTR label_get(int n){note('L',{n});static char repeat[]="REPEAT",base[]="BASE",move[]="MOVE",cost[]="COST";
    require(n==410||n==433||n==411||n==412);return n==410?repeat:n==433?base:n==411?move:cost;}
void say_loc(char *text,int x,int y,int name,int trailer,int coords){
    require(text==StringTemp&&x==tile_x&&y==tile_y&&name==0&&trailer==2&&coords==0);note('O',{x,y,name,trailer,coords});
    char location[64];std::snprintf(location,sizeof(location),"(%d,%d)",x,y);std::strcat(text,location);
}
char *_itoa(int value,char *out,int base){require(base==10);std::snprintf(out,64,"%d",value);return out;}
void Buffer::set_text_color(int a,int b,int c,int d){require(this==static_cast<Buffer*>(&CoverWindow));note('C',{a,b,c,d});}
Font *find_font(int size,int style){note('Q',{size,style});return &font;}
int Buffer::set_font(Font *a,Font *b,Font *c,Font *d){require(this==static_cast<Buffer*>(&CoverWindow)&&a==&font&&!b&&!c&&!d);note('B');
    if(scenario==13)ConsoleGlobal.dib_.bmiHeader.biWidth=200;return -5;}
int Buffer::text_width(LPSTR text){require(this==static_cast<Buffer*>(&CoverWindow)&&text==StringTemp);note('T',{},text);
    if(scenario==13){ConsoleGlobal.dib_.bmiHeader.biWidth=400;ConsoleGlobal.field_1DDB8_=70;}
    if(scenario==14)StringTemp[0]=0;
    return text_width_result;}
int Buffer::write_l(LPSTR text,int x,int y,int length){require(this==static_cast<Buffer*>(&CoverWindow)&&text==StringTemp);note('W',{x,y,length},text);
    write_x=x;write_y=y;write_length=length;written=text;return -7;}
void GraphicWin::soft_update(){require(this==&CoverWindow);note('Z');}
HELPERS
BODY
std::vector<Event> events(char kind){std::vector<Event> result;for(const auto&e:seen)if(e.kind==kind)result.push_back(e);return result;}
void kinds(const char *expected){size_t i=0;for(const auto&e:seen){require(expected[i]&&e.kind==expected[i]);++i;}require(!expected[i]);}
void reset(){
    scenario=0;pixel_result=0;selected_result=1;tile_x=12;tile_y=14;pixel_calls=pump_calls=0;
    MapLongitudeBounds=64;MapLatitudeBounds=32;MapMouseFeedbackBusy=0;MapInputEnabled=1;UnkGlobal0093A934=ConsoleControlTurnActive=0;
    window.field_1DD78_=window.field_1DD84_=window.field_1DD8C_=0;window.field_1DD80_=1;
    window.field_1DE0C_=99;window.feedback_hover_y_=98;window.feedback_proto_=97;window.feedback_triad_=96;
    window.feedback_repeat_=95;window.feedback_segment_count_=94;window.field_1DE24_=1;
    window.cursor_sprite_=nullptr;window.cursor_resource_id_=0x7f00;
    for(int i=0;i<5;++i){window.feedback_x_[i]=101+i;window.feedback_y_[i]=201+i;}
    ConsoleGlobal.selected_vehicle_=1;ConsoleGlobal.field_23D10_=0;ConsoleGlobal.faction_id_=3;
    ConsoleGlobal.field_1DDB8_=10;ConsoleGlobal.dib_.bmiHeader.biWidth=100;
    std::memset(Vehs,0,sizeof(Vehs));std::memset(VehPrototypes,0,sizeof(VehPrototypes));
    Vehs[1].x=2;Vehs[1].y=4;Vehs[1].proto_id=3;Vehs[1].waypoint_count=0;
    Vehs[2].x=-2;Vehs[2].y=-4;Vehs[2].proto_id=4;Vehs[2].waypoint_x[0]=55;Vehs[2].waypoint_y[0]=56;
    for(int i=0;i<4;++i){Vehs[1].waypoint_x[i]=20+i*2;Vehs[1].waypoint_y[i]=21+i*2;}
    VehPrototypes[3].chassis_id=2;VehPrototypes[4].chassis_id=1;Chassis[2].triad=2;Chassis[1].triad=1;
    Bases[1].x=6;Bases[1].y=8;Bases[2].x=-6;Bases[2].y=-8;
    Rules.move_rate_roads=3;MapDestinationCost=999;std::strcpy(StringTemp,"stale");
    text_width_result=20;write_x=write_y=write_length=-999;written="unwritten";seen.clear();segments.clear();
}
void admission(){
    for(int guard=0;guard<5;++guard){reset();switch(guard){case 0:MapMouseFeedbackBusy=1;break;case 1:UnkGlobal0093A934=1;break;
        case 2:ConsoleControlTurnActive=1;break;case 3:MapInputEnabled=0;break;case 4:window.field_1DD78_=1;break;}
        window.on_mouse_feedback(25,30);require(seen.empty()&&pixel_calls==0);}
    for(int invalid=0;invalid<5;++invalid){reset();switch(invalid){case 0:pixel_result=1;break;case 1:tile_y=-1;break;case 2:tile_y=32;break;
        case 3:tile_x=-1;break;case 4:tile_x=64;break;}window.on_mouse_feedback(25,30);kinds("P");}
    for(int erase=1;erase<4;++erase){reset();window.field_1DD8C_=erase;window.on_mouse_feedback(25,30);kinds("PE");require(events('E')[0].args==std::vector<int>({12,14,erase==2}));}
    reset();selected_result=0;window.on_mouse_feedback(25,30);kinds("PS");
    for(int state=0;state<2;++state){reset();scenario=1;selected_result=0;if(state)window.cursor_resource_id_=7;else window.cursor_sprite_=&sprite;
        window.on_mouse_feedback(25,30);kinds("PSR");require(!window.cursor_sprite_&&window.cursor_resource_id_==0x7f00&&events('R')[0].args[0]==0x7f00);}
    reset();window.field_1DD80_=0;window.on_mouse_feedback(25,30);kinds("PSZ");require(std::string(StringTemp)=="stale");
    reset();window.field_1DE0C_=tile_x;window.feedback_hover_y_=tile_y;window.on_mouse_feedback(25,30);kinds("PSNZ");require(MapMouseFeedbackBusy==0);
    reset();scenario=3;window.on_mouse_feedback(25,30);kinds("PSNZ");require(MapMouseFeedbackBusy==0);
    reset();scenario=2;window.on_mouse_feedback(25,30);require(pixel_calls==1&&events('N').size()==1&&MapMouseFeedbackBusy==0);
    reset();window.field_1DE0C_=tile_x;window.on_mouse_feedback(25,30);require(segments.size()==1);
    reset();window.feedback_hover_y_=tile_y;window.on_mouse_feedback(25,30);require(segments.size()==1);
}
void route_vectors(){
    // Independently listed original path: start vehicle, count+1 waypoints,
    // and (only repeat with nonzero count) return to the vehicle.
    for(int count=0;count<4;++count)for(int repeat=0;repeat<2;++repeat){reset();Vehs[1].waypoint_count=count;ConsoleGlobal.field_23D10_=repeat?8:0;
        const int points[6][2]={{2,4},{20,21},{22,23},{24,25},{26,27},{2,4}};
        window.on_mouse_feedback(25,30);const int n=count+1+(repeat&&count);
        require(int(segments.size())==n&&pump_calls==n&&window.feedback_segment_count_==n);
        for(int i=0;i<n;++i){int sx=points[i][0],sy=points[i][1],ex=points[i+1][0],ey=points[i+1][1];
            if(repeat&&i==count){ex=12;ey=14;}
            if(repeat&&i==count+1){sx=12;sy=14;ex=2;ey=4;}
            require(segments[i]==(std::array<int,11>{{sx,sy,ex,ey,3,3,2,repeat,3,5,1}}));}
        for(int i=0;i<=n&&i<5;++i){int ex=points[i][0],ey=points[i][1];
            if(repeat&&i==count+1){ex=12;ey=14;}
            if(repeat&&count>0&&i==count+2){ex=2;ey=4;}
            if(repeat&&count==3&&i==0)ey=2; // x[5] physical alias of y[0].
            require(window.feedback_x_[i]==ex&&window.feedback_y_[i]==ey);}
        require(window.field_1DE0C_==uint32_t(repeat&&count==3?4:12)&&window.feedback_hover_y_==14);
        require(window.feedback_proto_==3&&window.feedback_triad_==2);
        for(int i=n+1;i<5;++i)require(window.feedback_x_[i]==101+i&&window.feedback_y_[i]==201+i);
    }
    reset();ConsoleGlobal.field_23D10_=1;Vehs[1].waypoint_count=3;window.on_mouse_feedback(25,30);
    require(Vehs[1].waypoint_count==0&&segments.size()==1&&segments[0][2]==12&&segments[0][3]==14);
    reset();ConsoleGlobal.field_23D10_=1;Vehs[1].state=0x02000000;Vehs[1].waypoint_count=3;window.on_mouse_feedback(25,30);
    require(Vehs[1].waypoint_count==3&&segments.size()==4&&segments[3][2]==12&&segments[3][3]==14);
    reset();ConsoleGlobal.selected_vehicle_=-1;window.on_mouse_feedback(25,30);
    require(segments.size()==1&&segments[0]==(std::array<int,11>{{6,8,14,202,10,3,2,0,3,5,1}}));
    require(window.feedback_y_[1]==202&&window.feedback_x_[1]==14&&window.feedback_segment_count_==1);
    reset();scenario=7;ConsoleGlobal.selected_vehicle_=-1;window.on_mouse_feedback(25,30);require(segments[0][0]==-6&&segments[0][1]==-8);
    reset();ConsoleGlobal.selected_vehicle_=0;Vehs[0].x=-3;Vehs[0].y=-5;Vehs[0].proto_id=3;Vehs[0].waypoint_x[0]=7;Vehs[0].waypoint_y[0]=9;
    window.on_mouse_feedback(25,30);require(segments[0][0]==-3&&segments[0][1]==-5&&segments[0][2]==7&&segments[0][3]==9);
}
void callback_controls(){
    reset();scenario=4;ConsoleGlobal.selected_vehicle_=-1;window.on_mouse_feedback(25,30);require(segments[0][4]==3&&segments[0][7]==1);
    reset();scenario=5;window.on_mouse_feedback(25,30);require(segments[0][0]==-2&&segments[0][4]==4&&segments[0][6]==1);
    reset();scenario=6;window.on_mouse_feedback(25,30);require(segments.size()==3&&segments[0][0]==-9&&segments[0][1]==-7&&segments[0][2]==31&&segments[0][3]==41);
    reset();scenario=8;window.on_mouse_feedback(25,30);require(segments[0][4]==11&&segments[0][5]==6&&segments[0][6]==1);
    reset();scenario=9;Vehs[1].waypoint_count=3;window.on_mouse_feedback(25,30);require(segments.size()==1&&events('L')[0].args[0]==410);
    reset();scenario=10;Vehs[1].waypoint_count=2;ConsoleGlobal.field_23D10_=8;window.on_mouse_feedback(25,30);
    require(segments.size()==3&&segments[0][7]==1&&segments[1][7]==0&&segments[1][5]==5&&events('L')[0].args[0]==411);
    reset();scenario=15;Vehs[1].waypoint_count=1;window.on_mouse_feedback(25,30);
    require(segments.size()==2&&segments[1][0]==20&&segments[1][1]==21&&segments[1][2]==22&&segments[1][3]==23);
    reset();scenario=17;Vehs[1].waypoint_count=1;ConsoleGlobal.field_23D10_=8;window.on_mouse_feedback(25,30);
    require(segments.size()==2&&segments[1][0]==20&&segments[1][1]==21&&segments[1][2]==2&&segments[1][3]==4);
    reset();scenario=18;window.field_1DD80_=0;window.on_mouse_feedback(25,30);require(segments.size()==1);
}
void text_controls(){
    reset();window.on_mouse_feedback(25,30);kinds("PSNFUDLOLCQBTWZ");
    require(written=="MOVE (12,14): COST = 2 2/3"&&write_x==20&&write_y==40&&write_length==int(written.size()));
    require(events('C')[0].args==std::vector<int>({0x9f,0x41,1,1})&&events('Q')[0].args==std::vector<int>({12,1}));
    reset();window.field_1DD84_=1;window.on_mouse_feedback(25,30);require(written=="BASE (12,14): COST = 2 2/3");
    reset();ConsoleGlobal.field_23D10_=8;window.field_1DD84_=1;window.on_mouse_feedback(25,30);require(written=="REPEAT (12,14): COST = 2 2/3");
    reset();scenario=11;window.on_mouse_feedback(25,30);require(written=="MOVE (12,14): COST = -2 -2/3");
    reset();scenario=12;window.on_mouse_feedback(25,30);require(written=="MOVE (12,14): COST = 2");
    const int rates[]={1,2,5,100};const char *fractions[]={"8","4","1 3/5","0 8/100"};
    for(int i=0;i<4;++i){reset();Rules.move_rate_roads=rates[i];window.on_mouse_feedback(25,30);
        require(written==std::string("MOVE (12,14): COST = ")+fractions[i]);}
    reset();scenario=13;window.on_mouse_feedback(25,30);require(write_x==23&&write_y==100);
    reset();scenario=14;window.on_mouse_feedback(25,30);require(written.empty()&&write_length==0&&events('W').size()==1);
    reset();ConsoleGlobal.field_1DDB8_=0x7fffffffU;text_width_result=100000;window.on_mouse_feedback(100000,30);
    // Low32(100000*100000)=1410065408; signed divide by100 truncates.
    require(write_x==-14000654&&write_y==(-2147483647-1)+29);
    reset();text_width_result=100000;window.on_mouse_feedback(30000,30);require(write_x==12979672);
}
int main(){admission();route_vectors();callback_controls();text_controls();}
'''


def run_host(tmp_path, parts):
    compiler = shutil.which("clang++") or shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler required")
    source, binary = tmp_path / "mouse.cpp", tmp_path / "mouse"
    source.write_text(re.sub(r"\b(" + "|".join(parts) + r")\b", lambda m: parts[m.group()], HARNESS))
    compiled = subprocess.run([compiler, "-std=c++11", "-O1", "-fsanitize=address,undefined", "-fno-sanitize-recover=all",
                               str(source), "-o", str(binary)], capture_output=True, text=True, timeout=30)
    assert compiled.returncode == 0, compiled.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True, timeout=10,
                          env=dict(os.environ, ASAN_OPTIONS="detect_leaks=0"))


def test_actual_mouse_feedback(tmp_path):
    result = run_host(tmp_path, actual_source())
    assert result.returncode == 0, result.stderr


MUTATIONS = [
    ("busy_gate", "BODY", "MapMouseFeedbackBusy != 0", "false"),
    ("modal_gate", "BODY", "UnkGlobal0093A934 != 0", "false"),
    ("turn_gate", "BODY", "ConsoleControlTurnActive != 0", "false"),
    ("input_gate", "BODY", "MapInputEnabled == 0", "false"),
    ("window_gate", "BODY", "field_1DD78_ != 0", "false"),
    ("bad_conversion_admitted", "BODY", "pixel_to_tile(pixel_x, pixel_y, &tile_x, &tile_y) != 0", "pixel_to_tile(pixel_x, pixel_y, &tile_x, &tile_y) < 0"),
    ("latitude_endpoint", "BODY", "tile_y >= MapLatitudeBounds", "tile_y > MapLatitudeBounds"),
    ("longitude_endpoint", "BODY", "tile_x >= MapLongitudeBounds", "tile_x > MapLongitudeBounds"),
    ("negative_latitude", "BODY", "tile_y < 0", "false"),
    ("negative_longitude", "BODY", "tile_x < 0", "false"),
    ("editor_erase", "BODY", "field_1DD8C_ == 2", "field_1DD8C_ != 2"),
    ("cursor_sprite_test", "BODY", "cursor_sprite_ != 0 ||", "false ||"),
    ("cursor_resource_test", "BODY", "cursor_resource_id_ != 0x7f00", "false"),
    ("cursor_writeback", "BODY", "cursor_resource_id_ = 0x7f00;", ";"),
    ("inactive_text", "BODY", "if (field_1DD80_ == 0)", "if (false)"),
    ("pump_without_busy", "BODY", "MapMouseFeedbackBusy = 1;", "MapMouseFeedbackBusy = 0;"),
    ("busy_not_cleared", "BODY", "MapMouseFeedbackBusy = 0;", "MapMouseFeedbackBusy = 1;"),
    ("one_hover_axis", "BODY", "static_cast<uint32_t>(tile_x) &&", "static_cast<uint32_t>(tile_x) ||"),
    ("hover_y_publication", "BODY", "feedback_hover_y_ = tile_y;", ";"),
    ("zero_vehicle_missing", "BODY", "vehicle < 0", "vehicle <= 0"),
    ("no_vehicle_endpoint_fix", "BODY", "feedback_x_[1] = tile_y;", "feedback_x_[1] = tile_x;"),
    ("no_vehicle_y_initialization", "BODY", "feedback_x_[1] = tile_y;", "feedback_x_[1] = tile_y; feedback_y_[1] = tile_y;"),
    ("missing_pending_waypoint", "BODY", "0x02000000U", "0x04000000U"),
    ("waypoint_reset_count", "BODY", "Vehs[vehicle].waypoint_count = 0;", ";"),
    ("missing_repeat_leg", "BODY", "(feedback_repeat_ != 0 && Vehs[vehicle].waypoint_count != 0)", "0"),
    ("zero_count_repeat_leg", "BODY", "feedback_repeat_ != 0 && Vehs[vehicle].waypoint_count != 0", "feedback_repeat_ != 0"),
    ("endpoint_modulus", "BODY", "% (Vehs[vehicle].waypoint_count + 2)", "% (Vehs[vehicle].waypoint_count + 1)"),
    ("signed_vehicle_y", "BODY", "segment == 0 ? Vehs[vehicle].y", "segment == 0 ? static_cast<uint16_t>(Vehs[vehicle].y)"),
    ("cached_selected_vehicle_lost", "BODY", "Vehs[vehicle]", "Vehs[ConsoleGlobal.selected_vehicle_]"),
    ("live_proto_lost", "BODY", "feedback_proto_, ConsoleGlobal.faction_id_,", "Vehs[vehicle].proto_id, ConsoleGlobal.faction_id_,"),
    ("x5_wrong_physical_field", "HELPERS", "window.feedback_y_[0] = value;", "window.feedback_y_[1] = value;"),
    ("y5_wrong_physical_field", "HELPERS", "window.field_1DE0C_ = static_cast<uint32_t>(value);", "window.feedback_hover_y_ = value;"),
    ("segment_count_lost", "BODY", "feedback_segment_count_ = segment + 1;", ";"),
    ("draw_args_after_alias_stores", "BODY", "dest_line(start_x, start_y, end_x, end_y,", "dest_line(start_x, segment == 4 ? feedback_y_[0] : start_y, end_x, end_y,"),
    ("second_conversion", "BODY", "StringTemp[0] = '\\0';", "pixel_to_tile(pixel_x, pixel_y, &tile_x, &tile_y); StringTemp[0] = '\\0';"),
    ("stale_text_prefix", "BODY", "StringTemp[0] = '\\0';", ";"),
    ("repeat_caption", "BODY", "label_get(410)", "label_get(411)"),
    ("base_caption", "BODY", "label_get(433)", "label_get(411)"),
    ("integer_cost_only", "BODY", "if (MapDestinationCost % mouse_feedback_signed(Rules.move_rate_roads) != 0)", "if (false)"),
    ("missing_current_cost", "BODY", "MapDestinationCost / mouse_feedback_signed(Rules.move_rate_roads)", "0"),
    ("fraction_numerator", "BODY", "_itoa(MapDestinationCost % mouse_feedback_signed(Rules.move_rate_roads), number, 10)", "_itoa(0, number, 10)"),
    ("fixed_road_rate", "BODY", "mouse_feedback_signed(Rules.move_rate_roads)", "3"),
    ("width_reloaded_after_measure", "BODY", "mouse_feedback_mul(text_width, pixel_x) / width", "mouse_feedback_mul(text_width, pixel_x) / ConsoleGlobal.dib_.bmiHeader.biWidth"),
    ("missing_vertical_offset", "BODY", "mouse_feedback_signed(ConsoleGlobal.field_1DDB8_), pixel_y", "0, pixel_y"),
    ("skip_empty_write", "BODY", "CoverWindow.write_l(StringTemp,", "if (StringTemp[0]) CoverWindow.write_l(StringTemp,"),
    ("missing_soft_update", "BODY", "CoverWindow.soft_update();", ";"),
]


@pytest.mark.parametrize("name,part,old,new", MUTATIONS, ids=[m[0] for m in MUTATIONS])
def test_mouse_damage_controls(tmp_path, name, part, old, new):
    parts = actual_source()
    if name == "live_proto_lost":
        # Change only the vehicle branch: the absent-vehicle branch must not
        # acquire an invalid array access merely to reject a mutation.
        before, after = parts[part].split("} else {\n        feedback_proto_", 1)
        assert old in after
        parts[part] = before + "} else {\n        feedback_proto_" + after.replace(old, new)
    else:
        assert old in parts[part], name
        parts[part] = parts[part].replace(old, new)
    result = run_host(tmp_path, parts)
    assert result.returncode == 95 and "assertion line" in result.stderr, (name, result.stderr)
