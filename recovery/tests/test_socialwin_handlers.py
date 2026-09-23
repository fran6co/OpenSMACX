"""Actual SocialWin tooltip handler and actual in_box, with service doubles.

Original 4B2220..4B23FA tests economy/labs/psych/effects in that order and
selects labels 841/842/843/839. The stack-slot swaps preserve logical X/Y;
conversion and bubble publication both use MainInterfaceGlobal, not SocialWin.
MainInterface's distinct button-id overload is present and fails if called;
the original direct Win service must be explicitly qualified.
Original 5FA7E0 uses inclusive left/top and exclusive right/bottom.

Complete method bodies and relevant real declarations/inheritance/member types
are extracted unchanged. Windows are bounded storage projections, not native
layout/lifetime/interface-table proof. Tutorial reset, label lookup, conversion,
and bubble publication are explicit service doubles. In particular the real
label_get's legacy pointer-sized index representation is not rewritten for a
64-bit host. Callback changes occur only at these service boundaries: actual
in_box remains a pure four-comparison function. Overflow-free signed Win32
coordinates are exercised. Leak detection is disabled for the host sandbox;
address/undefined/alignment sanitizers remain enabled.
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
    raise AssertionError('unterminated actual definition')


def one(text, pattern):
    result, = re.findall(pattern, text, re.M)
    return result


def actual():
    headers = {name: (ROOT / f'src/{name}.h').read_text() for name in
               ('win', 'graphicwin', 'maininterface', 'socialwin', 'tutwin', 'alpha')}
    declarations = '\n'.join((
        'struct Win {',
        one(headers['win'], r'void client_to_screen\(int \*x, int \*y\);'),
        one(headers['win'], r'void set_bubble_text\(char \* text, RECT \* rect\);'),
        '}; struct Buffer {}; struct SubInterface {}; struct IfaceRegistryEntry {};',
        one(headers['graphicwin'], r'^class GraphicWin :[^\n]+\{') + '};',
        one(headers['maininterface'], r'^class MainInterface :[^\n]+\{') + ' public:',
        one(headers['maininterface'], r'int set_bubble_text\(uint32_t button_id, LPCSTR text\);'),
        '};',
        one(headers['socialwin'], r'^class SocialWin :[^\n]+\{') + ' public:',
        one(headers['socialwin'], r'void on_iface_right_click\(int x, int y\);'),
        '\n'.join(one(headers['socialwin'], rf'RECT {field};') for field in
                  ('tooltipEconomy_', 'tooltipLabs_', 'tooltipPsych_', 'tooltipEffects_')),
        '}; struct TutWin {', one(headers['tutwin'], r'int reset\(\);'), '};',
        one(headers['maininterface'], r'^extern MainInterface MainInterfaceGlobal;'),
        one(headers['tutwin'], r'^extern TutWin TutorialWindow;'),
        one(headers['alpha'], r'^LPSTR __cdecl label_get\(int label_offset\);'),
        one(headers['win'], r'^BOOL __cdecl in_box\(int x, int y, const RECT \*rect\);'),
    ))
    bodies = {
        'handler': braced((ROOT / 'src/socialwin.cpp').read_text(),
                          'void SocialWin::on_iface_right_click('),
        'in_box': braced((ROOT / 'src/win.cpp').read_text(),
                         'BOOL __cdecl in_box(int x, int y, const RECT *rect)'),
    }
    return declarations, bodies


HARNESS = r'''
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <functional>
typedef int BOOL;
typedef char *LPSTR;
typedef const char *LPCSTR;
struct RECT {int32_t left,top,right,bottom;};
#define __cdecl
#define FALSE 0
void check(bool ok,int line){if(!ok){std::fprintf(stderr,"assertion line %d\n",line);std::_Exit(107);}}
#define require(x) check((x),__LINE__)
DECLARATIONS
MainInterface MainInterfaceGlobal;
TutWin TutorialWindow;
SocialWin panel;
std::string events;
std::function<void(char)> callback;
int label_seen,conversion_x,conversion_y,offset_x,offset_y;
char words[4][16]={"economy","labs","psych","effects"};
char alternate[16]="changed";
char *returned_text,*published_text;
RECT published;
void service(char kind){events+=kind;if(callback)callback(kind);}
int TutWin::reset(){require(this==&TutorialWindow);service('R');return 1;}
LPSTR __cdecl label_get(int label){
    label_seen=label;
    service('L');
    require(label==841||label==842||label==843||label==839);
    return returned_text?returned_text:words[label==839?3:label-841];
}
void Win::client_to_screen(int*x,int*y){
    require(this==static_cast<Win*>(&MainInterfaceGlobal));
    require(x!=y);conversion_x=*x;conversion_y=*y;
    service('C');*x+=offset_x;*y+=offset_y;
}
void Win::set_bubble_text(char*text,RECT*rect){
    require(this==static_cast<Win*>(&MainInterfaceGlobal));
    published_text=text;published=*rect;service('B');
}
int MainInterface::set_bubble_text(uint32_t,LPCSTR){require(false);return 0;}
BODIES
RECT& region(int i){return i==0?panel.tooltipEconomy_:i==1?panel.tooltipLabs_:
                         i==2?panel.tooltipPsych_:panel.tooltipEffects_;}
void seed(){
    callback=nullptr;events.clear();label_seen=-1;
    conversion_x=conversion_y=999;offset_x=103;offset_y=-47;
    returned_text=published_text=nullptr;published={-900,-901,-902,-903};
    for(int i=0;i<4;++i)region(i)={500+i*100,600,510+i*100,610};
}
void outcome(int winner,int x,int y){
    if(winner<0){require(events=="R"&&label_seen==-1&&published_text==nullptr);return;}
    require(events=="RLCB");
    require(label_seen==(winner==3?839:841+winner));
    require(conversion_x==x&&conversion_y==y);
    require(published_text==(returned_text?returned_text:words[winner]));
    require(published.left==x+offset_x&&published.right==x+offset_x);
    require(published.top==y+offset_y&&published.bottom==y+offset_y);
}
int main(){
    // Independent half-open rectangle oracle, each boundary on all four paths.
    const int xs[]={-14,-13,-12,6,7,8},ys[]={20,21,22,27,28,29};
    for(int target=0;target<4;++target)for(int x:xs)for(int y:ys){
        seed();region(target)={-13,21,7,28};
        panel.on_iface_right_click(x,y);
        const bool hit=(x>=-13&&x<7&&y>=21&&y<28);
        outcome(hit?target:-1,x,y);
    }
    // All 15 nonempty combinations overlap; original first-match wins.
    for(int mask=1;mask<16;++mask){
        seed();int first=-1;
        for(int i=0;i<4;++i)if(mask&(1<<i)){region(i)={-20,-9,31,14};if(first<0)first=i;}
        panel.on_iface_right_click(17,-7);outcome(first,17,-7);
    }
    // Empty/inverted rectangles must never accidentally match.
    for(int shape=0;shape<3;++shape){
        seed();for(int i=0;i<4;++i)region(i)=shape==0?RECT{4,2,4,9}:
            shape==1?RECT{4,9,10,2}:RECT{10,2,4,9};
        panel.on_iface_right_click(4,4);outcome(-1,4,4);
    }
    // reset runs BEFORE reading members; publishing a new hit here is valid.
    seed();callback=[](char kind){if(kind=='R')region(2)={-33,80,-20,91};};
    panel.on_iface_right_click(-27,86);outcome(2,-27,86);
    // Conversely, reset may invalidate all initially matching rectangles.
    seed();for(int i=0;i<4;++i)region(i)={0,0,40,40};
    callback=[](char kind){if(kind=='R')for(int i=0;i<4;++i)region(i)={90,90,99,99};};
    panel.on_iface_right_click(3,23);outcome(-1,3,23);
    // Once selected, later rect changes do not select another tooltip.
    // Label lookup publishes a different text pointer; conversion uses it even
    // after subsequent service state changes. No caller-side cached screen offsets.
    seed();region(1)={1,2,12,45};
    callback=[](char kind){
        if(kind=='L'){
            require(events=="RL"&&label_seen==842);
            for(int i=0;i<4;++i)region(i)={-1,-1,0,0};
            offset_x=-51;offset_y=74;returned_text=alternate;
        }
        if(kind=='C'){
            require(events=="RLC"&&conversion_x==8&&conversion_y==31);
            alternate[0]='X';
        }
        if(kind=='B')require(published_text==alternate&&published_text[0]=='X');
    };
    panel.on_iface_right_click(8,31);outcome(1,8,31);
    callback=nullptr;
}
'''


def run_host(tmp_path, declarations, bodies):
    compiler = shutil.which('clang++') or shutil.which('g++')
    if compiler is None:
        pytest.skip('host C++ compiler unavailable')
    source, binary = tmp_path / 'social.cpp', tmp_path / 'social'
    source.write_text(HARNESS.replace('DECLARATIONS', declarations).replace(
        'BODIES', '\n'.join(bodies.values())))
    compiled = subprocess.run([compiler, '-std=c++11', '-O1',
                               '-fsanitize=address,undefined', '-fno-sanitize-recover=all',
                               str(source), '-o', str(binary)],
                              capture_output=True, text=True, timeout=45)
    assert compiled.returncode == 0, compiled.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True, timeout=15,
                          env=dict(os.environ, ASAN_OPTIONS='detect_leaks=0'))


def test_actual_social_tooltip_handler(tmp_path):
    result = run_host(tmp_path, *actual())
    assert result.returncode == 0, result.stderr


DAMAGE = [
    ('handler', 'TutorialWindow.reset();', ''),
    ('handler', 'TutorialWindow.reset();', 'TutWin other; other.reset();'),
    ('handler', '    int label;', '    int label; return;'),
    ('handler', '&tooltipEconomy_', '&tooltipEffects_'),
    ('handler', '&tooltipLabs_', '&tooltipPsych_'),
    ('handler', '&tooltipPsych_', '&tooltipLabs_'),
    ('handler', '&tooltipEffects_', '&tooltipEconomy_'),
    ('handler', 'label = 841;', 'label = 839;'),
    ('handler', 'label = 842;', 'label = 841;'),
    ('handler', 'label = 843;', 'label = 842;'),
    ('handler', 'label = 839;', 'label = 843;'),
    ('handler', 'in_box(x, y,', 'in_box(y, x,'),
    ('handler', 'client_to_screen(&x, &y)', 'client_to_screen(&y, &x)'),
    ('handler', 'MainInterfaceGlobal.client_to_screen', 'this->client_to_screen'),
    ('handler', 'MainInterfaceGlobal.Win::set_bubble_text', 'this->Win::set_bubble_text'),
    ('handler', 'RECT rectangle = {x, y, x, y};', 'RECT rectangle = {y, x, y, x};'),
    ('handler', 'RECT rectangle = {x, y, x, y};', 'RECT rectangle = {x, y, x+1, y+1};'),
    ('handler', 'MainInterfaceGlobal.Win::set_bubble_text(text, &rectangle);',
     'MainInterfaceGlobal.set_bubble_text(0, text);'),
    ('handler', 'MainInterfaceGlobal.Win::set_bubble_text(text, &rectangle);', ''),
    ('handler', 'char *text = label_get(label);', 'char *text = label_get(label); TutorialWindow.reset();'),
    ('handler', 'MainInterfaceGlobal.client_to_screen(&x, &y);', ''),
    ('in_box', 'x < rect->left', 'x <= rect->left'),
    ('in_box', 'x >= rect->right', 'x > rect->right'),
    ('in_box', 'y < rect->top', 'y <= rect->top'),
    ('in_box', 'y < rect->bottom', 'y <= rect->bottom'),
]


@pytest.mark.parametrize('body,before,after', DAMAGE)
def test_compiling_tooltip_damage_rejected(tmp_path, body, before, after):
    declarations, bodies = actual()
    assert before in bodies[body], before
    bodies[body] = bodies[body].replace(before, after)
    result = run_host(tmp_path, declarations, bodies)
    assert result.returncode == 107 and 'assertion line' in result.stderr, result.stderr
