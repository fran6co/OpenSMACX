"""Actual GraphicWin fill orchestration with explicit drawing/Win doubles.

The two complete recovered bodies and the real borrowed-pointer initializer
are extracted unchanged. Callbacks deliberately change the parent, rectangle,
dimensions and table, exposing the original re-read boundaries. Buffer drawing
and the virtual query are observers, not proof of their implementations, native
base offsets or a working GUI. Transparent null RECT is an unsafe original
domain, checked separately as sanitizer rejection, never accepted behavior.
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
    raise AssertionError("unterminated source")


def actual_source():
    source = (ROOT / "src/graphicwin.cpp").read_text()
    initializer, = re.findall(r"unsigned char \*GraphicWinColorMapTable\s*=\s*nullptr;", source)
    return initializer, [braced(source, signature) for signature in (
        "void GraphicWin::fill(int color)", "int GraphicWin::fill(RECT *area, int color)")]


HARNESS = r'''
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <initializer_list>
void check(bool ok,int line){if(!ok){std::fprintf(stderr,"assertion line %d\n",line);std::exit(93);}}
#define require(x) check((x),__LINE__)
struct RECT {int32_t left,top,right,bottom;};
struct Dib {struct {int32_t biWidth,biHeight;} bmiHeader;};
struct GraphicWin;
struct Win {virtual GraphicWin *get_graphic_win();};
struct Buffer {
    Dib dib_;
    int fill(int); int fill(RECT *,int);
    int copy(Buffer *,int,int,int,int,int,int);
    int map_colors(int,int,int,int,unsigned char *);
};
struct GraphicWin : Win,Buffer {
    uint32_t iFlags_;Win *win_parent_;RECT client_rect_,outer_rect_;
    void fill(int);int fill(RECT *,int);
};
ACTUAL_INITIALIZER
ACTUAL_BODIES
GraphicWin child,old_parent,new_parent,query_result;
RECT area;
unsigned char old_lut[256],new_lut[256];
std::string events;
bool scalar,query_yes,mutate_query,mutate_copy;
int table_action,copy_result,map_result,fill_result;
RECT *expected_area;
std::vector<int> copy_expected,map_expected;
GraphicWin *copy_parent;
GraphicWin *Win::get_graphic_win(){
    require(this==static_cast<Win *>(&old_parent));events+='Q';
    if(mutate_query){
        child.win_parent_=&new_parent;
        child.client_rect_={20,30,0,0};child.outer_rect_={3,-5,0,0};
        child.dib_.bmiHeader={17,-19};area={4,6,13,18};
    }
    return query_yes ? &query_result : nullptr;
}
int Buffer::fill(int color){
    require(this==static_cast<Buffer *>(&child) && scalar && color==349);events+='F';return fill_result;
}
int Buffer::fill(RECT *rect,int color){
    require(this==static_cast<Buffer *>(&child) && !scalar && rect==expected_area && color==349);
    events+='F';return fill_result;
}
int Buffer::copy(Buffer *target,int sx,int sy,int dx,int dy,int w,int h){
    require(this==static_cast<Buffer *>(copy_parent) && target==static_cast<Buffer *>(&child));
    require((std::vector<int>{sx,sy,dx,dy,w,h}==copy_expected));events+='C';
    if(mutate_copy){area={7,8,16,22};child.dib_.bmiHeader={29,-31};}
    if(table_action==1)GraphicWinColorMapTable=new_lut;
    if(table_action==2)GraphicWinColorMapTable=nullptr;
    return copy_result;
}
int Buffer::map_colors(int l,int t,int r,int b,unsigned char *table){
    require(this==static_cast<Buffer *>(&child));
    require((std::vector<int>{l,t,r,b}==map_expected));
    require(table==(table_action==1 ? new_lut : old_lut));events+='M';return map_result;
}
void start(bool is_scalar){
    scalar=is_scalar;events.clear();query_yes=false;mutate_query=mutate_copy=false;
    table_action=0;copy_result=17;map_result=23;fill_result=71;
    child.iFlags_=0;child.win_parent_=&old_parent;
    child.client_rect_={2,3,0,0};child.outer_rect_={5,7,0,0};
    child.dib_.bmiHeader={11,-13};area={1,2,7,10};expected_area=&area;
    GraphicWinColorMapTable=old_lut;copy_parent=&old_parent;
    copy_expected=scalar ? std::vector<int>{7,10,0,0,11,13} : std::vector<int>{8,12,1,2,6,8};
    map_expected=scalar ? std::vector<int>{0,0,10,12} : std::vector<int>{1,2,6,9};
}
int invoke(){if(scalar){child.fill(349);return 0;}return child.fill(expected_area,349);}
int main(int argc,char **){
    require(GraphicWinColorMapTable==nullptr);
    if(argc>1){start(false);child.iFlags_=0x80000;query_yes=true;expected_area=nullptr;invoke();return 0;}
    for(bool is_scalar : {false,true}){
        start(is_scalar);require(invoke()==(scalar?0:71));require(events=="F");
        start(is_scalar);child.iFlags_=0x40000;require(invoke()==(scalar?0:71));require(events=="F");
        start(is_scalar);child.iFlags_=0x80000;child.win_parent_=nullptr;
        require(invoke()==(scalar?0:71));require(events=="F");
        start(is_scalar);child.iFlags_=0x80000;require(invoke()==(scalar?0:71));require(events=="QF");
        for(int action : {0,1,2})for(bool change_query : {false,true})for(bool change_copy : {false,true}){
            start(is_scalar);child.iFlags_=0x80000;query_yes=true;
            table_action=action;mutate_query=change_query;mutate_copy=change_copy;
            if(change_query){
                copy_parent=&new_parent;
                copy_expected=scalar ? std::vector<int>{23,25,0,0,17,19} : std::vector<int>{27,31,4,6,9,12};
                map_expected=scalar ? std::vector<int>{0,0,16,18} : std::vector<int>{4,6,12,17};
            }
            if(change_copy)map_expected=scalar ? std::vector<int>{0,0,28,30} : std::vector<int>{7,8,15,21};
            if(action==1)GraphicWinColorMapTable=nullptr; // copy publishes previously absent map
            require(invoke()==0);require(events==(action==2?"QC":"QCM"));
        }
    }
    start(false);expected_area=nullptr;require(invoke()==71 && events=="F");
}
'''


def run_host(tmp_path, initializer, bodies, unsafe=False):
    compiler = shutil.which("clang++") or shutil.which("g++")
    if not compiler:
        pytest.skip("host compiler required")
    source = tmp_path / "window.cpp"
    source.write_text(HARNESS.replace("ACTUAL_INITIALIZER", initializer)
                      .replace("ACTUAL_BODIES", "\n".join(bodies)))
    binary = tmp_path / "window"
    result = subprocess.run([compiler,"-std=c++11","-O1","-fsanitize=address,undefined",
                             "-fno-sanitize-recover=all",str(source),"-o",str(binary)],
                            capture_output=True,text=True,timeout=30)
    assert result.returncode == 0, result.stderr
    return subprocess.run([str(binary)] + (["unsafe-null"] if unsafe else []),
                          capture_output=True,text=True,timeout=10,
                          env=dict(os.environ,ASAN_OPTIONS="detect_leaks=0"))


def test_actual_graphicwin_fill(tmp_path):
    result = run_host(tmp_path, *actual_source())
    assert result.returncode == 0, result.stderr


def test_transparent_null_rectangle_is_not_a_supported_domain(tmp_path):
    result = run_host(tmp_path, *actual_source(), unsafe=True)
    assert result.returncode != 0
    assert "runtime error:" in result.stderr or "AddressSanitizer" in result.stderr, result.stderr


@pytest.mark.parametrize("which,before,after", [
    (1,"return Buffer::fill(area, color);","Buffer::fill(area, color); return 0;"),
    (1,"area->right - 1, area->bottom - 1","area->right, area->bottom"),
    (1,"area->right - 1, area->bottom - 1","left + width - 1, top + height - 1"),
    (1,"Buffer::map_colors(area->left, area->top,","Buffer::map_colors(left, top,"),
    (1,"this, left + origin_x, top + origin_y, left, top, width, height",
       "this, origin_x, origin_y, left, top, width, height"),
    (1,"this, left + origin_x, top + origin_y, left, top, width, height",
       "this, left + origin_x, top + origin_y, 0, 0, width, height"),
    (1,"return 0;","return 17;"),
    (0,"width = static_cast<int>(dib_.bmiHeader.biWidth);\n    height = static_cast<int>(dib_.bmiHeader.biHeight);",
       "/* incorrectly reused dimensions from before copy */"),
    (0,"width - 1, -1 - height","width, -height"),
    (0,"surface, origin_x, origin_y, 0, 0, width, -height",
       "surface, origin_x, origin_y, 0, 0, width, height"),
    (0,"(iFlags_ & 0x80000)","(iFlags_ & 0x40000)"),
    (1,"parent->get_graphic_win() != nullptr","parent->get_graphic_win() == nullptr"),
    (1,"unsigned char *const table = GraphicWinColorMapTable;",
       "unsigned char *const table = old_lut;"),
    (0,"unsigned char *const table = GraphicWinColorMapTable;",
       "unsigned char *const table = old_lut;"),
    (1,"static_cast<GraphicWin *>(win_parent_)","&query_result"),
    (0,"static_cast<GraphicWin *>(win_parent_)","&old_parent"),
])
def test_graphicwin_fill_damage_is_observed(tmp_path, which, before, after):
    initializer,bodies = actual_source()
    assert before in bodies[which]
    bodies[which] = bodies[which].replace(before,after)
    # Mutation fixtures use observers declared after actual bodies normally.
    initializer = "extern GraphicWin query_result,old_parent; extern unsigned char old_lut[256];\n" + initializer
    result = run_host(tmp_path,initializer,bodies)
    assert result.returncode == 93, result.stderr
