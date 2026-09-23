"""Actual cursor/citizen loader sequencing with explicit graphics API doubles.

Original 454FC0..4553F3 determines the event oracle, including the human
variant receiver stride and vertical hotspot row 32. This is not proof of PCX
decoding, Sprite allocation, native layout, startup completeness or rendering.
"""

from pathlib import Path
import os
import re
import shutil
import subprocess

import pytest


ROOT=Path(__file__).resolve().parents[2]


def braced(source,signature):
    start=source.index(signature);opening=source.index("{",start);depth=0
    for end in range(opening,len(source)):
        depth+=(source[end]=="{")-(source[end]=="}")
        if not depth:return source[start:end+1]
    raise AssertionError("unterminated actual definition")


ARRAYS=("g_CURSOR_SPRITES","g_CITIZEN_LG_CURSOR_SPRITES","g_SPECIALIST_LG_CURSOR_SPRITES",
        "g_CITIZEN_SM_CURSOR_SPRITES","g_SPECIALIST_SM_CURSOR_SPRITES",
        "g_AL_CITIZEN_SM_CURSOR_SPRITES","g_AL_SPECIALIST_SM_CURSOR_SPRITES",
        "g_AL_CITIZEN_LG_CURSOR_SPRITES","g_AL_SPECIALIST_LG_CURSOR_SPRITES")


def actual_source():
    source=(ROOT/"src/game.cpp").read_text()
    sprites=(ROOT/"src/sprite.cpp").read_text()
    console=(ROOT/"src/console.cpp").read_text()
    declarations=[]
    for name in ARRAYS:
        definition,=re.findall(r"^Sprite "+name+r"\[\d+\];",sprites,re.M)
        declarations.append(definition)
    for name in ("CursorHotspotX","CursorHotspotY"):
        definition,=re.findall(r"^int "+name+r"\[12\] = \{0\};",console,re.M)
        declarations.append(definition)
    sprite_header=(ROOT/"src/sprite.h").read_text();buffer_header=(ROOT/"src/buffer.h").read_text()
    extract,=re.findall(r"  int extract\([^;]+\);",sprite_header)
    methods=[]
    for pattern in (r"  int load_pcx\(const char \*[^;]+;",r"  int get_pixel\([^;]+;",
                    r"  int change_color\(int a1, int a2\);",r"  void close\(\);"):
        method,=re.findall(pattern,buffer_header);methods.append(method)
    interfaces="struct Sprite {"+extract+"};\nstruct Buffer {"+"\n".join(methods)+"};"
    return interfaces+"\n"+"\n".join(declarations),braced(source,"void __cdecl load_cursors()")


HARNESS=r'''
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#define __cdecl
void check(bool ok,int line){if(!ok){std::fprintf(stderr,"assertion line %d\n",line);std::exit(91);}}
#define require(x) check((x),__LINE__)
struct Buffer;struct Palette;struct TexHeap;
ACTUAL_DECLARATIONS
Buffer g_PCX_PARSE_TEMP_BUFFER1;
void *BufferDirectDraw;
struct Event {char kind;Sprite *sprite;int a,b,c,d,e;std::string file;};
std::vector<Event> expected;
size_t current;
int api_result,marker_mode,expected_hot_x[12],expected_hot_y[12];
Event &take(char kind){require(current<expected.size());Event &e=expected[current++];require(e.kind==kind);return e;}
void verify_hotspots(){for(int i=0;i<12;++i)require(CursorHotspotX[i]==expected_hot_x[i] && CursorHotspotY[i]==expected_hot_y[i]);}
int Buffer::load_pcx(const char *name,Palette *palette,int first,int count){
    require(this==&g_PCX_PARSE_TEMP_BUFFER1 && !palette && first==10 && count==236);
    Event &e=take('L');require(e.file==name);verify_hotspots();return api_result;
}
int Buffer::get_pixel(int x,int y){
    require(this==&g_PCX_PARSE_TEMP_BUFFER1);Event &e=take('P');require(x==e.a && y==e.b);verify_hotspots();
    // Two markers per scan; the final marker (including vertical y=32) wins.
    if(marker_mode==0)return 9;
    if(y==0){int slot=x/33,local=x-(1+slot*33);return local==2 || local==31-slot?5:9;}
    int slot=x/33;return y==2 || y==32-slot?5:9;
}
int Buffer::change_color(int from,int to){
    require(this==&g_PCX_PARSE_TEMP_BUFFER1);Event &e=take('C');require(from==e.a && to==e.b);verify_hotspots();return api_result;
}
int Sprite::extract(Buffer *buffer,int key,int x,int y,int width,int height,TexHeap *heap){
    Event &e=take('E');require(buffer==&g_PCX_PARSE_TEMP_BUFFER1 && !heap && this==e.sprite);
    require(key==265 && x==e.a && y==e.b && width==e.c && height==e.d);verify_hotspots();
    // The loader must publish this cursor's hotspots only after extraction.
    if(e.e>=0){expected_hot_x[e.e]=marker_mode?31-e.e:0;expected_hot_y[e.e]=marker_mode?31-e.e:0;}
    return api_result;
}
void Buffer::close(){require(this==&g_PCX_PARSE_TEMP_BUFFER1);take('X');verify_hotspots();}
ACTUAL_BODY
void event(char kind,Sprite *sprite=nullptr,int a=0,int b=0,int c=0,int d=0,int slot=-1,const char *file=""){
    Event e={kind,sprite,a,b,c,d,slot,file};expected.push_back(e);
}
void cell(Sprite *sprite,int x,int y,int width,int height,int slot=-1){
    event('C',nullptr,263,265);event('C',nullptr,262,265);event('E',sprite,x,y,width,height,slot);
}
void prepare(){
    expected.clear();current=0;BufferDirectDraw=nullptr;
    for(int i=0;i<12;++i)CursorHotspotX[i]=expected_hot_x[i]=-100-i,CursorHotspotY[i]=expected_hot_y[i]=-200-i;
    event('L',nullptr,0,0,0,0,-1,"CURSOR.PCX");
    for(int slot=0;slot<12;++slot){
        for(int x=0;x<32;++x)event('P',nullptr,1+slot*33+x,0);
        for(int y=1;y<=32;++y)event('P',nullptr,slot*33,y);
        cell(&g_CURSOR_SPRITES[slot],1+slot*33,1,32,32,slot);
    }
    event('L',nullptr,0,0,0,0,-1,"NEWICONS.PCX");
    // Image cells alternate human variants; receiver arrays group four moods.
    for(int mood=0;mood<4;++mood)for(int variant=0;variant<2;++variant)
        cell(&g_CITIZEN_LG_CURSOR_SPRITES[mood+4*variant],1+39*(2*mood+variant),501,38,48);
    for(int type=0;type<7;++type)cell(&g_SPECIALIST_LG_CURSOR_SPRITES[type],313+39*type,501,38,48);
    for(int mood=0;mood<4;++mood)for(int variant=0;variant<2;++variant)
        cell(&g_CITIZEN_SM_CURSOR_SPRITES[mood+4*variant],44+31*(2*mood+variant),458,30,38);
    for(int type=0;type<7;++type)cell(&g_SPECIALIST_SM_CURSOR_SPRITES[type],292+31*type,458,30,38);
    event('L',nullptr,0,0,0,0,-1,"ALIENCIT.PCX");
    for(int mood=0;mood<4;++mood)cell(&g_AL_CITIZEN_SM_CURSOR_SPRITES[mood],1+31*mood,1,30,38);
    for(int type=0;type<7;++type)cell(&g_AL_SPECIALIST_SM_CURSOR_SPRITES[type],125+31*type,1,30,38);
    for(int mood=0;mood<4;++mood)cell(&g_AL_CITIZEN_LG_CURSOR_SPRITES[mood],1+39*mood,41,38,48);
    for(int type=0;type<7;++type)cell(&g_AL_SPECIALIST_LG_CURSOR_SPRITES[type],157+39*type,41,38,48);
    event('X');
}
int main(){
    for(int failures=0;failures<2;++failures)for(marker_mode=0;marker_mode<2;++marker_mode){
        prepare();api_result=failures?17:0;load_cursors();require(current==expected.size());verify_hotspots();
        int extracts=0,pixels=0,colors=0;for(const Event &e:expected){extracts+=e.kind=='E';pixels+=e.kind=='P';colors+=e.kind=='C';}
        require(extracts==64 && pixels==768 && colors==128);
    }
    prepare();BufferDirectDraw=&g_PCX_PARSE_TEMP_BUFFER1;load_cursors();require(current==0);verify_hotspots();
}
'''


def run_host(tmp_path,declarations,body):
    compiler=shutil.which("clang++") or shutil.which("g++")
    if not compiler:pytest.skip("host C++ compiler required")
    source=tmp_path/"cursor.cpp";binary=tmp_path/"cursor"
    source.write_text(HARNESS.replace("ACTUAL_DECLARATIONS",declarations).replace("ACTUAL_BODY",body))
    compiled=subprocess.run([compiler,"-std=c++11","-O1","-fsanitize=address,undefined","-fno-sanitize-recover=all",str(source),"-o",str(binary)],capture_output=True,text=True,timeout=30)
    assert compiled.returncode==0,compiled.stderr
    return subprocess.run([str(binary)],capture_output=True,text=True,timeout=10,env=dict(os.environ,ASAN_OPTIONS="detect_leaks=0"))


def test_actual_cursor_loader(tmp_path):
    result=run_host(tmp_path,*actual_source());assert result.returncode==0,result.stderr


MUTANTS=[
    ("directdraw-guard","if (BufferDirectDraw)","if (!BufferDirectDraw)"),
    ("vertical-last-row","row < 32","row < 31"),
    ("horizontal-last-column","column < 32","column < 31"),
    ("first-marker","hot_x = column;","hot_x = column; break;"),
    ("vertical-marker-offset","hot_y = row;","hot_y = row + 1;"),
    ("cursor-count","cursor < 12","cursor < 11"),
    ("human-variant-receiver","mood + 4 * variant","mood"),
    ("recolour-order","change_color(0x107, 0x109)","change_color(0x106, 0x109)"),
    ("specialist-count","specialist < 7","specialist < 6"),
    ("large-cell-pitch","x += 39;","x += 38;"),
    ("small-cell-pitch","x += 31;","x += 30;"),
    ("small-row","x, 458, 30, 38","x, 457, 30, 38"),
    ("last-close","g_PCX_PARSE_TEMP_BUFFER1.close();",""),
    ("failure-early-return",'g_PCX_PARSE_TEMP_BUFFER1.load_pcx("CURSOR.PCX", 0, 10, 236);',
        'if (g_PCX_PARSE_TEMP_BUFFER1.load_pcx("CURSOR.PCX", 0, 10, 236)) return;'),
    ("publish-before-extract","g_CURSOR_SPRITES[cursor].extract(",
        "CursorHotspotX[cursor] = hot_x; CursorHotspotY[cursor] = hot_y; g_CURSOR_SPRITES[cursor].extract("),
]


@pytest.mark.parametrize("name,before,after",MUTANTS,ids=[x[0] for x in MUTANTS])
def test_compiling_cursor_damage(tmp_path,name,before,after):
    declarations,body=actual_source();assert before in body
    result=run_host(tmp_path,declarations,body.replace(before,after))
    assert result.returncode!=0,(name,"behavior damage escaped controls")
