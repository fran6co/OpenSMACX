"""Actual BaseWin refresh hooks, not a complete base-screen/resource proof.

Both full bodies, Win visibility, distance helpers, Base, field declarations
and relevant inheritance clauses come from source unchanged. Original
41A7A0..41A8A7 supplies the two-stage oracle: visible/admitted -> compute(1),
virtual receiver redraw, qualified main-interface GraphicWin redraw. The
independent distance formula is max(dx,dy)-floor((abs(dx-dy)+2)/4), with the
original single horizontal wrap and absolute value applied first.

Only window storage is projected; Base uses its complete real declaration.
Recomputation and painting are explicit service doubles. In particular this
does not exercise unfinished base_energy, BaseWin's pending contents painter,
native vtable offsets, constructors, or real GUI/Win32 callbacks. Valid base
IDs and bounded, overflow-free signed coordinates are used while visible.
Hidden invalid IDs prove that the original early visibility gate is retained.
"""
from pathlib import Path
import os
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def braced(text, signature):
    start = text.index(signature)
    opening = text.index('{', start)
    depth = 0
    for end in range(opening, len(text)):
        depth += (text[end] == '{') - (text[end] == '}')
        if not depth:
            return text[start:end + 1]
    raise AssertionError('unterminated actual source')


def one(text, pattern):
    result, = re.findall(pattern, text, re.M)
    return result


def actual():
    win = (ROOT / 'src/win.h').read_text()
    graphic = (ROOT / 'src/graphicwin.h').read_text()
    basewin = (ROOT / 'src/basewin.h').read_text()
    interface = (ROOT / 'src/maininterface.h').read_text()
    base = (ROOT / 'src/base.h').read_text()
    map_h = (ROOT / 'src/map.h').read_text()
    cpp = (ROOT / 'src/basewin.cpp').read_text()
    declarations = '\n'.join((
        braced(base, 'struct Base {') + ';',
        one(base, r'^static const int MaxBaseNum = \d+;'),
        one(base, r'^extern Base Bases\[MaxBaseNum\];'),
        one(base, r'^extern int BaseIDCurrentSelected;'),
        one(base, r'^void __cdecl base_compute\(int force\);'),
        one(map_h, r'^extern int MapLongitudeBounds;'),
        one(map_h, r'^extern uint32_t MapLongitude;'),
        one(map_h, r'^extern BOOL MapIsFlat;'),
        'struct Win { public:',
        one(win, r'\buint32_t iSomeFlag_;'),
        one(win, r'\bWin \*win_parent_;'),
        one(win, r'\bint is_visible\(\);'),
        braced(win, 'virtual void redraw()'), '};',
        # Storage/lifetime of the secondary interface is outside this refresh
        # control; retain its real inheritance clause with a service double.
        'struct Buffer {}; struct IfaceRegistryEntry {}; struct SubInterface {};',
        one(graphic, r'class GraphicWin :[^\n]+\{') + ' public:',
        one(graphic, r'virtual void redraw\(\);'), '};',
        one(basewin, r'class BaseWin :[^\n]+\{') + ' public:',
        one(basewin, r'\buint32_t focusedBaseID_;'),
        one(basewin, r'\bvoid check_base\(int base_id\);'),
        one(basewin, r'\bvoid check_loc\(int x, int y, int radius\);'), '};',
        # An extra fixture override makes accidental dynamic dispatch on the
        # main-interface object observable; the real hook must bypass it.
        one(interface, r'class MainInterface :[^\n]+\{') +
        ' public: void redraw() override; };',
        one(interface, r'^extern MainInterface MainInterfaceGlobal;'),
    ))
    methods = {
        'visible': braced((ROOT / 'src/win.cpp').read_text(), 'int Win::is_visible()'),
        'x_dist': braced(map_h, 'MEASURED inline int __cdecl x_dist('),
        'distance': braced(map_h, 'MEASURED inline int __cdecl vector_dist(int x_distance,'),
        'check_base': braced(cpp, 'void BaseWin::check_base('),
        'check_loc': braced(cpp, 'void BaseWin::check_loc('),
    }
    return declarations, methods


HARNESS = r'''
#include <cstdint>
#include <cstddef>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <functional>
#include <type_traits>
#define __cdecl
#define MEASURED
typedef int BOOL;
void check(bool ok,int line){if(!ok){std::fprintf(stderr,"assertion line %d\n",line);std::exit(104);}}
#define require(v) check((v),__LINE__)
DECLARATIONS
static_assert(sizeof(Base)==0x134,"actual Base declaration");
static_assert(std::is_same<decltype(Base::x),int16_t>::value,"signed x");
static_assert(std::is_same<decltype(Base::y),int16_t>::value,"signed y");
static_assert(offsetof(Base,x)==0&&offsetof(Base,y)==2,"actual coordinate fields");
Base Bases[MaxBaseNum];
int BaseIDCurrentSelected=3;
int MapLongitudeBounds=100;
uint32_t MapLongitude=50;
BOOL MapIsFlat=0;
MainInterface MainInterfaceGlobal;
std::vector<char> events;
std::function<void(char)> callback;
BaseWin *active;
struct ObservedBaseWin : BaseWin { void redraw() override; } window;
Win parent,grandparent;
void __cdecl base_compute(int force){
  require(force==1);require(BaseIDCurrentSelected==3);
  events.push_back('C');if(callback)callback('C');
}
void ObservedBaseWin::redraw(){
  require(static_cast<BaseWin*>(this)==active);
  require(events==std::vector<char>({'C'}));
  events.push_back('R');if(callback)callback('R');
}
void GraphicWin::redraw(){
  require(this==static_cast<GraphicWin*>(&MainInterfaceGlobal));
  require(events==std::vector<char>({'C','R'}));
  events.push_back('M');if(callback)callback('M');
}
void MainInterface::redraw(){require(false);}
METHODS
void start(){
  events.clear();callback=nullptr;active=&window;
  window.iSomeFlag_=1;window.win_parent_=nullptr;window.focusedBaseID_=2;
  parent.iSomeFlag_=grandparent.iSomeFlag_=1;
  parent.win_parent_=grandparent.win_parent_=nullptr;
  MainInterfaceGlobal.iSomeFlag_=1;MainInterfaceGlobal.win_parent_=nullptr;
  BaseIDCurrentSelected=3;MapLongitudeBounds=100;MapLongitude=50;MapIsFlat=0;
  for(int i=0;i<MaxBaseNum;++i){Bases[i].x=static_cast<int16_t>(i+13);Bases[i].y=static_cast<int16_t>(i-11);}
}
void outcome(bool repaint){require(events==(repaint?std::vector<char>{'C','R','M'}:std::vector<char>{}));}
int expected_distance(int x,int y,int bx,int by,bool flat){
  int dx=std::abs(x-bx),dy=std::abs(y-by);
  if(!flat&&dx>50)dx=100-dx;
  dx=std::abs(dx);
  return (dx>dy?dx:dy)-(std::abs(dx-dy)+2)/4;
}
int main(){
  // Different selected and focused IDs prove that these hooks do not select
  // the focused base before calling the existing computation pipeline.
  for(int visible : {0,1,2,3,256,257})for(int base_id : {-1,0,1,2,3}){
    start();window.iSomeFlag_=visible;window.check_base(base_id);
    outcome((visible&1)&&base_id==2);
  }
  start();window.focusedBaseID_=0xffffffffu;window.check_base(-1);outcome(true);
  for(int depth : {1,2})for(int hidden : {0,1,2}){
    start();window.win_parent_=&parent;if(depth==2)parent.win_parent_=&grandparent;
    if(hidden==1)parent.iSomeFlag_=0;if(hidden==2)grandparent.iSomeFlag_=0;
    window.check_base(2);outcome(hidden==0||(depth==1&&hidden==2));
    events.clear();window.check_loc(Bases[2].x,Bases[2].y,0);
    outcome(hidden==0||(depth==1&&hidden==2));
  }
  // Hidden invalid focus must never form a Base-array read.
  for(bool ancestor : {false,true}){
    start();window.focusedBaseID_=0xffffffffu;
    if(ancestor){window.win_parent_=&parent;parent.iSomeFlag_=0;}
    else window.iSomeFlag_=0;
    window.check_loc(0,0,99);outcome(false);
  }
  struct Point{int x,y;};
  const Point bases[]={{12,14},{-7,-13},{32760,-32760}};
  const int offsets[]={-101,-99,-69,-51,-50,-35,-18,-7,-4,-3,-2,-1,0,1,2,3,4,7,18,35,50,51,69,99,101};
  for(Point b:bases)for(int flat : {0,1,2,3})for(int dx:offsets)for(int dy : {-19,-8,-3,-1,0,1,3,8,19}){
    const int x=b.x+dx,y=b.y+dy;
    const int distance=expected_distance(x,y,b.x,b.y,(flat&1)!=0);
    for(int radius : {-1,distance-1,distance,distance+1}){
      start();MapIsFlat=flat;Bases[2].x=static_cast<int16_t>(b.x);Bases[2].y=static_cast<int16_t>(b.y);
      window.check_loc(x,y,radius);outcome(distance<=radius);
      require(window.focusedBaseID_==2&&BaseIDCurrentSelected==3);
    }
  }
  // No second visibility, distance or selected-base admission test occurs
  // after computation; virtual redraw and then direct global redraw remain.
  for(bool location : {false,true}){
    start();callback=[](char event){
      if(event=='C'){
        window.iSomeFlag_=0;window.focusedBaseID_=17;
        Bases[2].x=123;Bases[2].y=127;MapLongitudeBounds=40;MapLongitude=20;
      }else if(event=='R'){
        require(window.focusedBaseID_==17&&window.iSomeFlag_==0);
        MainInterfaceGlobal.iSomeFlag_=0;
      }
    };
    if(location)window.check_loc(Bases[2].x,Bases[2].y,0);else window.check_base(2);
    outcome(true);callback=nullptr;
    events.clear();window.check_base(17);outcome(false);
    window.iSomeFlag_=1;events.clear();window.check_base(17);outcome(true);
  }
}
'''


def run_host(tmp_path, declarations, methods):
    compiler = shutil.which('clang++') or shutil.which('g++')
    if compiler is None:
        pytest.skip('host C++ compiler unavailable')
    cpp = tmp_path / 'refresh.cpp'
    cpp.write_text(HARNESS.replace('DECLARATIONS', declarations).replace(
        'METHODS', '\n'.join(methods.values())))
    exe = tmp_path / 'refresh'
    compiled = subprocess.run([compiler, '-std=c++11', '-O1', '-g',
                               '-fsanitize=address,undefined', '-fno-sanitize-recover=all',
                               str(cpp), '-o', str(exe)], capture_output=True, text=True, timeout=60)
    assert compiled.returncode == 0, compiled.stderr
    return subprocess.run([str(exe)], capture_output=True, text=True, timeout=20,
                          env=dict(os.environ, ASAN_OPTIONS='detect_leaks=0'))


def test_actual_basewin_refresh(tmp_path):
    result = run_host(tmp_path, *actual())
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize('method', ['check_base', 'check_loc'])
@pytest.mark.parametrize('damage', ['force', 'receiver_dispatch', 'global_dispatch', 'order', 'recheck_visible'])
def test_refresh_dispatch_damage_is_observed(tmp_path, method, damage):
    declarations, methods = actual()
    before, after = {
        'force': ('base_compute(1);','base_compute(0);'),
        'receiver_dispatch': ('\n        redraw();','\n        GraphicWin::redraw();'),
        'global_dispatch': ('MainInterfaceGlobal.GraphicWin::redraw();','MainInterfaceGlobal.redraw();'),
        'order': ('base_compute(1);','redraw(); base_compute(1);'),
        'recheck_visible': ('base_compute(1);','base_compute(1); if (!Win::is_visible()) return;'),
    }[damage]
    # Keep extraction indifferent to indentation depth of the distance branch.
    if damage=='receiver_dispatch':
        before,after='redraw();','GraphicWin::redraw();'
        assert methods[method].count(before)==2
        methods[method]=methods[method].replace(before,after,1)
    else:
        assert before in methods[method]
        methods[method]=methods[method].replace(before,after)
    result=run_host(tmp_path,declarations,methods)
    assert result.returncode==104 and 'assertion line' in result.stderr,result.stderr


@pytest.mark.parametrize('method,before,after', [
    ('check_base','base_id == focusedBaseID_','base_id != focusedBaseID_'),
    ('check_base','Win::is_visible() &&','true &&'),
    ('check_base','base_id == focusedBaseID_','base_id == 3'),
    ('check_loc','Bases[focusedBaseID_].x','Bases[3].x'),
    ('check_loc','Bases[focusedBaseID_].y','Bases[3].y'),
    ('check_loc','Bases[focusedBaseID_].x','static_cast<uint16_t>(Bases[focusedBaseID_].x)'),
    ('check_loc','Bases[focusedBaseID_].y','static_cast<uint16_t>(Bases[focusedBaseID_].y)'),
    ('check_loc','<= radius','< radius'),
    ('check_loc','const int dy = abs(y - base_y);','const int dy = abs(x - base_y);'),
    ('check_loc','vector_dist(dx, dy)','dx + dy'),
    ('x_dist','!(MapIsFlat & 1)','!MapIsFlat'),
    ('x_dist','MapLongitudeBounds - dist','dist'),
    ('x_dist','dist > (int)MapLongitude','dist < (int)MapLongitude'),
    ('distance','smallest + 1','smallest'),
    ('visible','win_parent_ && !win_parent_->is_visible()','false'),
])
def test_refresh_admission_damage_is_observed(tmp_path,method,before,after):
    declarations,methods=actual()
    assert before in methods[method]
    methods[method]=methods[method].replace(before,after)
    result=run_host(tmp_path,declarations,methods)
    assert result.returncode==104 and 'assertion line' in result.stderr,result.stderr
