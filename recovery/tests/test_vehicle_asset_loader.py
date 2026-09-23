"""Complete actual vehicle/flag loaders against original-derived service traces.

454270..4547DB and 453F70..454261, with unwind65513A..655182, establish
the literal coordinates, receiver order, marker stores and local Sprite lives.
The fixtures use the real complete loader bodies, asset declarations, chassis
definition and service signatures. PCX, extraction, copy, recolouring and Sprite
lifetime internals are explicitly doubled. These tests do not establish native
ABI/layout, actual image contents, allocator/graphics correctness or game startup.
Synthetic callback mutations test the loader's live reads, not the existence of
callbacks in every real service. ASan/UBSan remain strict; only LSan is disabled
because the sandbox uses ptrace. Damage must compile and reach assertion103.
"""

from pathlib import Path
import os
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
ASSETS = ("g_XI_BOOM_VEH_SPRITES", "g_XF_BOOM_VEH_SPRITES",
          "g_VEH_SPRITES", "g_FLAGS_VEH_SPRITES")
COLORS = ("InfoFactionTextColors", "InfoFactionShadowColors",
          "ChatItemIdTable", "ChatShadowColors")


def braced(source, signature):
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if not depth:
            return source[start:end + 1]
    raise AssertionError("unterminated actual source")


def actual_source():
    files = {name: (ROOT / "src" / name).read_text() for name in
             ("game.cpp", "game.h", "sprite.cpp", "sprite.h", "buffer.cpp",
              "buffer.h", "veh.h", "veh.cpp", "faction.h", "infowin.cpp",
              "infowin.h", "maininterface.cpp", "maininterface.h")}
    implementations = files["game.cpp"] + "\n" + files["sprite.cpp"]
    objects = "\n".join(files[name] for name in
                        ("sprite.cpp", "infowin.cpp", "maininterface.cpp"))
    headers = "\n".join(value for name, value in files.items() if name.endswith(".h"))
    definitions = []
    for name in ASSETS:
        definition, = re.findall(r"^Sprite " + name + r"\[\d+\];", objects, re.M)
        assert "extern " + definition in headers
        definitions.append(definition)
    for name in COLORS:
        definition, = re.findall(r"^int " + name + r"\[8\]\s*=\s*\{[^}]+\};", objects, re.M)
        assert re.search(r"extern int " + name + r"\[8\];", headers)
        definitions.append(definition)
    buffer, = re.findall(r"^Buffer g_PCX_PARSE_TEMP_BUFFER1;", files["buffer.cpp"], re.M)
    assert "extern " + buffer in headers
    definitions.append(buffer)
    sprite_methods = []
    for pattern in (r"  int extract\([^;]+\);", r"  int copy\([^;]+\);",
                    r"  int change_color\([^;]+\);", r"  Sprite\(\);", r"  void close\(\);"):
        method, = re.findall(pattern, files["sprite.h"])
        sprite_methods.append(method)
    buffer_methods = []
    for pattern in (r"  int load_pcx\(const char \*[^;]+;",
                    r"  int change_color\(int a1, int a2\);",
                    r"  int get_pixel\(int[^;]+;", r"  void close\(\);"):
        method, = re.findall(pattern, files["buffer.h"])
        buffer_methods.append(method)
    rule, = re.findall(r"^\s*int rule_flags;", files["faction.h"], re.M)
    alien, = re.findall(r"RFLAG_ALIEN\s*=\s*0x80", files["faction.h"])
    declarations = []
    for name in ("load_flags", "load_vehicles"):
        declaration, = re.findall(r"^void __cdecl " + name + r"\(\);", files["game.h"], re.M)
        declarations.append(declaration)
    # Keep any genuine named constant definitions verbatim, including their type.
    literal_defs = re.findall(r"^(?:static )?const char (?:VehicleUnitSheet|FlagUnitSheet)\[\]\s*=\s*\"UNITS.PCX\";", implementations, re.M)
    return {
        "INTERFACES": "struct Sprite { int identity; " + "\n".join(sprite_methods)
        + " ~Sprite(); };\nstruct Buffer { " + "\n".join(buffer_methods) + " };",
        "TYPES": braced(files["veh.h"], "struct RulesChassis {") + ";\n"
        + "struct Player { " + rule + " }; enum { " + alien + " };",
        "OBJECTS": "\n".join(definitions),
        "DECLARATIONS": "\n".join(declarations + literal_defs),
        "BODY": "\n\n".join(braced(implementations, "void __cdecl " + name + "()")
                                  for name in ("load_flags", "load_vehicles")),
    }


HARNESS = r'''
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <string>
#include <vector>
#include <algorithm>
#define __cdecl
typedef char* LPSTR; typedef int BOOL;
struct Buffer; struct Palette; struct TexHeap;
INTERFACES
TYPES
bool active=false,unwinding=false,perturb=false,late_change=false;
int scenario=0,throw_at=-1,next_local=0,alien_mask=0,callback_mask=0;
size_t cursor=0;
std::vector<int> live;
OBJECTS
Player Players[8]; RulesChassis Chassis[19],shadow[19];
int colours[4][8];
DECLARATIONS
void check(bool ok,int line){if(!ok){std::fprintf(stderr,"assertion line %d scenario %d event %zu\n",line,scenario,cursor);std::_Exit(103);}}
#define require(x) check((x),__LINE__)
struct Write {int chassis,field,slot,value;};
struct Event { char kind; Sprite *receiver; int local; std::vector<int> args;
    std::string file; int result; std::vector<Write> writes; int colour; int mask_update; };
std::vector<Event> plan;
unsigned char *field(RulesChassis&r,int n){switch(n){
case 0:return r.sprite_flag_x_off;case 1:return r.sprite_flag_y_off;
case 2:return r.sprite_unk1_x_off;case 3:return r.sprite_unk1_y_off;
case 4:return r.sprite_unk2_x_off;case 5:return r.sprite_unk2_y_off;
case 6:return r.sprite_unk3_x_off;default:return r.sprite_unk3_y_off;}}
int *table(int n){switch(n){case 0:return InfoFactionTextColors;
case 1:return InfoFactionShadowColors;case 2:return ChatItemIdTable;default:return ChatShadowColors;}}
void verify(){require(std::memcmp(Chassis,shadow,sizeof shadow)==0);
    for(int n=0;n<4;++n)require(std::memcmp(table(n),colours[n],sizeof colours[n])==0);}
Event &observe(char kind){verify();require(cursor<plan.size());Event&e=plan[cursor++];require(e.kind==kind);
    if(int(cursor)-1==throw_at){unwinding=true;throw 71;}
    if(e.mask_update>=0)callback_mask=e.mask_update;
    if(perturb){for(int n=0;n<4;++n)for(int i=0;i<8;++i)table(n)[i]=colours[n][i]=1000*n+int(cursor)*11+i;
        // Retain all existing object contents except explicit synthetic fields.
        // On copy/recolour callbacks, change a later faction's classification.
        for(int i=0;i<8;++i)Players[i].rule_flags=(callback_mask&(1<<i))?0x80:0;
    }
    for(const Write&w:e.writes)field(shadow[w.chassis],w.field)[w.slot]=static_cast<unsigned char>(w.value);
    if(e.colour>=0)colours[e.colour][0]=e.result;
    return e;
}
bool receiver(const Event&e,Sprite*s){return e.receiver?e.receiver==s:s->identity==e.local;}
Sprite::Sprite():identity(0){if(active){Event&e=observe('N');identity=++next_local;require(identity==e.local);live.push_back(identity);}}
Sprite::~Sprite(){if(active){if(unwinding){require(!live.empty()&&live.back()==identity);live.pop_back();}
    else{Event&e=observe('D');require(e.local==identity&&!live.empty()&&live.back()==identity);live.pop_back();}}}
void Sprite::close(){require(false);}
int Buffer::load_pcx(const char*name,Palette*p,int first,int count){require(this==&g_PCX_PARSE_TEMP_BUFFER1&&p==nullptr);
    Event&e=observe('L');require(e.file==name&&e.args==std::vector<int>({first,count}));return 7;}
int Buffer::change_color(int from,int to){require(this==&g_PCX_PARSE_TEMP_BUFFER1);Event&e=observe('B');
    require(e.args==std::vector<int>({from,to}));return 9;}
void Buffer::close(){require(this==&g_PCX_PARSE_TEMP_BUFFER1);observe('X');}
int Buffer::get_pixel(int x,int y){require(this==&g_PCX_PARSE_TEMP_BUFFER1);Event&e=observe('P');
    require(e.args==std::vector<int>({x,y}));return e.result;}
int Sprite::extract(Buffer*b,int key,int x,int y,int w,int h,TexHeap*heap){
    require(b==&g_PCX_PARSE_TEMP_BUFFER1&&heap==nullptr);Event&e=observe('E');
    require(receiver(e,this)&&e.args==std::vector<int>({key,x,y,w,h}));return 5;}
int Sprite::copy(Sprite*destination){Event&e=observe('C');require(identity==e.local&&destination==e.receiver);return 6;}
int Sprite::change_color(int from,int to){
    // Values are evaluated before entering this service. Check against the
    // current expected colours BEFORE observe deliberately mutates them.
    require(cursor<plan.size());Event&e=plan[cursor];require(e.kind=='R'&&e.receiver==this);
    require(from==e.args[0]&&to==colours[e.args[1]][e.args[2]]);observe('R');return 8;}
BODY
Event &add(char kind,Sprite*s=nullptr,int local=0,std::initializer_list<int>a={},const char*file="",int result=0){
    plan.push_back(Event{kind,s,local,a,file,result,{},-1,-1});return plan.back();}
void extract(Sprite*s,int local,int key,int x,int y,int w,int h){add('E',s,local,{key,x,y,w,h});}
void flags_plan(int first_local){
    for(int n=0;n<14;++n)add('N',nullptr,first_local+n);
    const int columns[]={1,25,49,73,97,121,145};
    add('L',nullptr,0,{10,236},"UNITS.PCX");
    for(int n=0;n<14;++n){add('B',nullptr,0,{263,265});add('B',nullptr,0,{262,265});
        extract(nullptr,first_local+n,265,columns[n/2],n%2?731:700,23,30);}
    for(int f=0;f<8;++f)for(int n=0;n<14;++n){Sprite*s=&g_FLAGS_VEH_SPRITES[f*14+n];
        add('C',s,first_local+n);add('R',s,0,{260,0,f});add('R',s,0,{259,1,f});}
    add('L',nullptr,0,{10,236},"FLAGS.PCX");
    for(int n=0;n<14;++n){add('B',nullptr,0,{263,265});add('B',nullptr,0,{262,265});
        extract(nullptr,first_local+n,265,columns[n/2],n%2?32:1,23,30);}
    for(int f=0;f<8;++f)if((late_change&&f==7?alien_mask^128:alien_mask)&(1<<f))
        for(int n=0;n<14;++n){Sprite*s=&g_FLAGS_VEH_SPRITES[f*14+n];
            Event&e=add('C',s,first_local+n);
            if(late_change&&f==0&&n==0)e.mask_update=alien_mask^128;
            add('R',s,0,{260,0,f});add('R',s,0,{259,1,f});}
    add('X');for(int n=13;n>=0;--n)add('D',nullptr,first_local+n);
}
void marker(int x,int y,int result,int c=-1,int f=0,int slot=0,int value=0){
    Event&e=add('P',nullptr,0,{x,y},"",result);if(c>=0)e.writes.push_back(Write{c,f,slot,value});}
void vehicles_plan(){
    add('N',nullptr,1);add('N',nullptr,2);add('N',nullptr,3);
    const int xi_x[]={2,47,92,137,182,227,272,317,362,407,452,497,542,587,632,677};
    const int xi_y[]={2,51,100,149,198,247,296,345,394};
    add('L',nullptr,0,{0,18},"XI.PCX");add('B',nullptr,0,{253,255});
    for(int n=0;n<144;++n)extract(&g_XI_BOOM_VEH_SPRITES[n],0,255,xi_x[n%16],xi_y[n/16],43,47);
    const int xf_x[]={2,104,206,308,410,512,614,716};
    const int xf_y[]={2,154,306,458,610,762,914,1066};
    add('L',nullptr,0,{0,18},"XF.PCX");add('B',nullptr,0,{253,255});
    for(int pair=0;pair<4;++pair)for(int x=0;x<8;++x)for(int half=0;half<2;++half)
        extract(&g_XF_BOOM_VEH_SPRITES[pair*16+half*8+x],0,255,xf_x[x],xf_y[pair*2+half],100,150);
    add('L',nullptr,0,{10,236},"PALETTE.PCX");
    const int palette_y[]={23,31,39,47};
    for(int n=0;n<4;++n){Event&e=add('P',nullptr,0,{580,palette_y[n]},"",300+n);e.colour=n;}
    add('L',nullptr,0,{10,236},"UNITS.PCX");
    const int count[]={1,7,1,7,7,7,3,1,1,1};
    const int unit_y[]={2,79,156,233,310,387,464,541,618,233};
    for(int c=9;c<19;++c)for(int variant=0;variant<count[c-9];++variant){
        int sx=c==18?716:xf_x[variant],sy=unit_y[c-9];
        add('B',nullptr,0,{263,265});add('B',nullptr,0,{262,265});
        extract(&g_VEH_SPRITES[c*8+variant],0,265,sx,sy,100,75);
        for(int f=0;f<8;++f)plan.back().writes.push_back(Write{c,f,variant,f<4?0:157});
        for(int x=0;x<100;++x)for(int edge=0;edge<2;++edge){
            int y=sy-2+edge;
            marker(sx+x,y-1,42); // Original ignored probe must not disappear.
            bool hit5=(x%17==3+edge),hit2=(x%19==7+edge);
            marker(sx+x,y,hit5?5:42,hit5?c:-1,0,variant,x);
            marker(sx+x,y,hit2?2:42,hit2?c:-1,2,variant,x);
        }
        for(int y=0;y<75;++y)for(int edge=0;edge<2;++edge){
            bool hit5=(y%13==2+edge),hit2=(y%11==5+edge);
            marker(sx-2+edge,sy+y,hit5?5:42,hit5?c:-1,1,variant,y);
            marker(sx-2+edge,sy+y,hit2?2:42,hit2?c:-1,3,variant,y);
        }
    }
    const int legacy_y[]={2,79,156,233,310,387,464,541,618};
    for(int c=0;c<9;++c){
        for(int x=0;x<100;++x)for(int edge=0;edge<2;++edge){
            int y=legacy_y[c]-1+edge;marker(918+x,y-1,42);
            bool hit=x%23==edge+4;marker(918+x,y,hit?5:42,hit?c:-1,0,0,x);
        }
        for(int y=0;y<75;++y)for(int edge=0;edge<2;++edge){
            bool hit=y%17==edge+3;
            marker(916+edge,legacy_y[c]+1+y,hit?5:42,hit?c:-1,1,1,y);
        }
    }
    int counts[256]={};for(const Event&e:plan)++counts[static_cast<unsigned char>(e.kind)];
    require(counts['E']==244&&counts['B']==74&&counts['L']==4&&counts['P']==37354);
    flags_plan(4);add('X');add('X');add('D',nullptr,3);add('D',nullptr,2);add('D',nullptr,1);
}
void prepare(bool vehicles,int mask,bool mutate,int failure=-1,bool late=false){
    active=false;unwinding=false;perturb=mutate;alien_mask=mask;throw_at=failure;
    late_change=late;callback_mask=mask;
    next_local=0;cursor=0;live.clear();plan.clear();
    std::memset(Chassis,0x39,sizeof Chassis);std::memcpy(shadow,Chassis,sizeof shadow);
    for(int n=0;n<4;++n)for(int f=0;f<8;++f)table(n)[f]=colours[n][f]=n*100+f+10;
    // Callback mode starts with the opposite classification. A callback must
    // update it before the later FLAGS phase reads each faction's live bit.
    for(int f=0;f<8;++f)Players[f].rule_flags=((mutate?mask^255:mask)&(1<<f))?0x80:0;
    if(vehicles)vehicles_plan();else flags_plan(1);
    active=true;
}
void run(bool vehicles){bool caught=false;try{if(vehicles)load_vehicles();else load_flags();}
    catch(int n){require(n==71&&unwinding);caught=true;}
    require(caught==(throw_at>=0)&&live.empty());if(!caught){require(cursor==plan.size());verify();}
    active=false;
}
int main(int argc,char**){
    if(argc==1){for(int mask=0;mask<256;++mask){scenario=mask;prepare(false,mask,false);run(false);}
        for(int n=0;n<4;++n){scenario=300+n;prepare(true,n&1?255:0,n&2);run(true);}
        for(int f=0;f<8;++f){scenario=400+f;prepare(false,1<<f,true);run(false);}
        scenario=500;prepare(false,1,true,-1,true);run(false);
        scenario=501;prepare(false,129,true,-1,true);run(false);
    }else{
        prepare(true,255,false);std::vector<int> faults;
        // All 17 local constructor entries, every distinct service kind and
        // each load/close boundary, plus late extraction/marker/copy/recolour.
        for(size_t n=0;n<plan.size();++n)if(plan[n].kind=='N'||plan[n].kind=='L'||plan[n].kind=='X')faults.push_back(int(n));
        for(char k:std::string("BE PCR"))if(k!=' '){
            for(size_t n=0;n<plan.size();++n)if(plan[n].kind==k){faults.push_back(int(n));break;}
            for(size_t n=plan.size();n-->0;)if(plan[n].kind==k){faults.push_back(int(n));break;}}
        active=false;
        for(int fault:faults){scenario=1000+fault;prepare(true,255,false,fault);run(true);}
    }
}
'''


def run_host(tmp_path, parts, unwind=False):
    compiler = shutil.which("clang++") or shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler required")
    source, binary = tmp_path / "vehicles.cpp", tmp_path / "vehicles"
    source.write_text(re.sub(r"\b(" + "|".join(parts) + r")\b", lambda m: parts[m.group()], HARNESS))
    compiled = subprocess.run([compiler, "-std=c++11", "-O1", "-fsanitize=address,undefined",
                               "-fno-sanitize-recover=all", str(source), "-o", str(binary)],
                              capture_output=True, text=True, timeout=40)
    assert compiled.returncode == 0, compiled.stderr
    return subprocess.run([str(binary)] + (["unwind"] if unwind else []),
                          capture_output=True, text=True, timeout=40,
                          env=dict(os.environ, ASAN_OPTIONS="detect_leaks=0"))


@pytest.mark.parametrize("unwind", [False, True])
def test_actual_complete_vehicle_loaders(tmp_path, unwind):
    result = run_host(tmp_path, actual_source(), unwind)
    assert result.returncode == 0, result.stderr


DAMAGE = [
    ("Sprite source[14];", "Sprite source[15];"),
    ("Sprite scratch[2];", "Sprite scratch[1];"),
    ("Sprite temporary;", ""),
    ("700 + frame * 31", "701 + frame * 31"),
    ("1 + frame * 31", "1 + frame * 30"),
    ("1 + kind * 24", "1 + kind * 25"),
    ("source[kind].copy(&destination);", "destination.copy(&source[kind]);"),
    ("destination.change_color(0x104, InfoFactionTextColors[faction]);", "destination.change_color(0x103, InfoFactionTextColors[faction]);"),
    ("InfoFactionShadowColors[faction]", "InfoFactionShadowColors[0]"),
    ("Players[faction].rule_flags & RFLAG_ALIEN", "Players[faction].rule_flags & 0x40"),
    ("faction < 8", "faction < 7"),
    ("2 + column * 45", "2 + column * 44"),
    ("2 + row * 49", "2 + row * 48"),
    ("row * 16 + column + 8", "row * 16 + column + 7"),
    ("154 + row * 304", "153 + row * 304"),
    ("row < 4", "row < 3"),
    ("sheet.get_pixel(580, 23)", "sheet.get_pixel(581, 23)"),
    ("sheet.get_pixel(580, 47)", "sheet.get_pixel(580, 46)"),
    ("chassis < 19", "chassis < 18"),
    ("count = 3", "count = 2"),
    ("source_x = 716", "source_x = 715"),
    ("source_y = 233", "source_y = 232"),
    ("sprite_unk2_y_off[variant] = 157", "sprite_unk2_y_off[variant] = 156"),
    ("sprite_flag_x_off[variant] = 0", "sprite_flag_x_off[variant] = 1"),
    ("sheet.get_pixel(source_x + x, marker_y - 1);", ""),
    ("source_x - 2; marker_x <= source_x - 1", "source_y - 2; marker_x <= source_y - 1"),
    ("sprite_unk1_x_off[variant] = static_cast<unsigned char>(x)", "sprite_unk1_x_off[variant] = static_cast<unsigned char>(x + 1)"),
    ("sprite_flag_y_off[1] = static_cast<unsigned char>(y_offset)", "sprite_flag_y_off[0] = static_cast<unsigned char>(y_offset)"),
    ("sheet.get_pixel(918 + x, y - 1);", ""),
    ("border_y + 1 + y_offset", "border_y + y_offset"),
    ("    load_flags();", ""),
    ("    sheet.close();\n    sheet.close();", "    sheet.close();"),
]


@pytest.mark.parametrize("before,after", DAMAGE)
def test_compiling_loader_damage_rejected(tmp_path, before, after):
    parts = actual_source()
    assert before in parts["BODY"], before
    parts["BODY"] = parts["BODY"].replace(before, after, 1)
    result = run_host(tmp_path, parts)
    assert result.returncode == 103 and "assertion line" in result.stderr, result.stderr
