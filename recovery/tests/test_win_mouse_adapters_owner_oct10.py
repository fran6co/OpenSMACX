"""Actual receiver-free mouse adapters with callback-visible state controls.

Pinned four-entry stack/slot contracts; host results are not native ABI,
linked preservation, GUI or full-game acceptance. Shared mouse-hook type
ambiguity is retained as an explicit conversion, not a recovered registration.
"""
from pathlib import Path
import re

import pytest

from test_texture_sub61f0d0_actual_source import build_and_run, extract_method

ROOT=next(p for p in Path(__file__).resolve().parents if (p/'src/win.cpp').is_file())
SOURCE=ROOT/'src/win.cpp'
SIGNATURES=(
    'void __cdecl Win::OnRButtonDown(void * hwnd, long flags, int x, int y, unsigned int keys)',
    'void __cdecl Win::OnRButtonUp(void * hwnd, int x, int y, unsigned int keys)',
    'void Win::OnLButtonUp(void *hwnd, int x, int y, unsigned int keys)',
    'void __cdecl Win::OnMouseMove(void * hwnd, int x, int y, unsigned int keys)',
)
HOST=r"""
#include <cassert>
#include <stdint.h>
#include <vector>
#define __cdecl
#define nullptr 0
typedef void *HWND;
typedef intptr_t LPARAM;
class Win;
static Win *WinTrackingWindow,*WinHoverWindow,*WinPointerOwner3,*hit;
static int WinCursorMoved,WinMouseDirect;
static void (*WinMessageHook)();
static void (*WinMouseHook)(HWND,LPARAM);
static bool expect_clear,mutate_hooks,mutate_hover;
static std::vector<int> events;
static Win *called_owner,*leave_owner,*tracking_owner;
static int args[5],hook_x,hook_y;
static void mouse_hook(int x,int y) { events.push_back(30); hook_x=x; hook_y=y; }
static void changed_mouse_hook(int x,int y) { events.push_back(31); hook_x=x; hook_y=y; }
static void message_hook() { events.push_back(40); }
static void changed_message_hook() { events.push_back(41); }
class Win {
public:
    // DECLARATIONS_INSERTION
    static Win *get_mouse_window(int *,int *);
    static void update_cursor(Win *,int);
    virtual void on_r_button_down(long dbl,int x,int y,unsigned int keys,int direct) {
        events.push_back(10); called_owner=this;
        args[0]=dbl; args[1]=x; args[2]=y; args[3]=keys; args[4]=direct;
        if(mutate_hooks) {
            WinMouseHook=reinterpret_cast<void(*)(HWND,LPARAM)>(changed_mouse_hook);
            WinMessageHook=changed_message_hook;
        }
    }
    virtual void on_r_button_up(int x,int y,unsigned int keys,int direct) {
        events.push_back(11); called_owner=this;
        args[0]=x; args[1]=y; args[2]=keys; args[3]=direct;
    }
    virtual void on_l_button_up(int x,int y,unsigned int keys,int direct) {
        events.push_back(12); called_owner=this;
        args[0]=x; args[1]=y; args[2]=keys; args[3]=direct;
    }
    void do_tracking(int x,int y) {
        assert(WinCursorMoved==0); events.push_back(13); tracking_owner=this;
        args[0]=x; args[1]=y;
    }
    virtual void on_mouse_leave(int x,int y) {
        events.push_back(14); leave_owner=this; assert(x==101 && y==202);
    }
    virtual void on_mouse_move(int x,int y,unsigned int keys,int direct) {
        assert(WinHoverWindow==this); events.push_back(15); called_owner=this;
        args[0]=x; args[1]=y; args[2]=keys; args[3]=direct;
    }
};
static Win target,prior,tracker;
Win *Win::get_mouse_window(int *x,int *y) {
    if(expect_clear) assert(WinTrackingWindow==0);
    assert(*x==11 && *y==22); *x=101; *y=202; WinMouseDirect=17;
    events.push_back(1); return hit;
}
void Win::update_cursor(Win *window,int mode) {
    assert(window==hit && mode==1 && WinCursorMoved==0); events.push_back(2);
    if(mutate_hover) { WinHoverWindow=&tracker; WinPointerOwner3=&tracker; WinCursorMoved=1; }
}
// BODIES_INSERTION
static void reset() {
    events.clear(); hit=&target; WinTrackingWindow=0; WinHoverWindow=0;
    WinPointerOwner3=0; WinCursorMoved=7; WinMouseDirect=-1;
    WinMouseHook=reinterpret_cast<void(*)(HWND,LPARAM)>(mouse_hook);
    WinMessageHook=message_hook; expect_clear=false; mutate_hooks=false; mutate_hover=false;
    called_owner=0; leave_owner=0; tracking_owner=0; hook_x=0; hook_y=0;
}
static void check_up(int event) {
    assert(events.size()==2 && events[0]==1 && events[1]==event);
    assert(called_owner==&target && WinTrackingWindow==0);
    assert(args[0]==101 && args[1]==202 && args[2]==0x3344 && args[3]==17);
}
int main() {
    void *window=&prior;
    reset(); Win::OnRButtonDown(window,9,11,22,0x3344);
    assert(events.size()==4 && events[0]==1 && events[1]==10 && events[2]==30 && events[3]==40);
    assert(called_owner==&target && args[0]==9 && args[1]==101 && args[2]==202 && args[3]==0x3344 && args[4]==17);
    assert(hook_x==101 && hook_y==202);
    reset(); mutate_hooks=true; Win::OnRButtonDown(window,9,11,22,0x3344);
    assert(events[2]==31 && events[3]==41); // reload hooks after virtual callback
    reset(); hit=0; Win::OnRButtonDown(window,9,11,22,0x3344);
    assert(events.size()==3 && events[0]==1 && events[1]==30 && events[2]==40 && called_owner==0);
    reset(); WinMouseHook=0; WinMessageHook=0; Win::OnRButtonDown(window,9,11,22,0x3344);
    assert(events.size()==2); // integer WinCursorMoved is not a callback

    reset(); expect_clear=true; WinTrackingWindow=&tracker;
    Win::OnLButtonUp(window,11,22,0x3344); check_up(12);
    reset(); expect_clear=true; WinTrackingWindow=&tracker;
    Win::OnRButtonUp(window,11,22,0x3344); check_up(11);
    reset(); expect_clear=true; WinTrackingWindow=&tracker; hit=0;
    Win::OnLButtonUp(window,11,22,0x3344);
    assert(events.size()==1 && called_owner==0 && WinTrackingWindow==0);
    reset(); expect_clear=true; WinTrackingWindow=&tracker; hit=0;
    Win::OnRButtonUp(window,11,22,0x3344);
    assert(events.size()==1 && called_owner==0 && WinTrackingWindow==0);

    reset(); WinTrackingWindow=&tracker; Win::OnMouseMove(window,11,22,0x3344);
    assert(events.size()==1 && events[0]==13 && tracking_owner==&tracker && args[0]==11 && args[1]==22);
    reset(); WinHoverWindow=&prior; Win::OnMouseMove(window,11,22,0x3344);
    assert(events.size()==4 && events[0]==1 && events[1]==2 && events[2]==14 && events[3]==15);
    assert(leave_owner==&prior && called_owner==&target && args[0]==101 && args[1]==202 && args[2]==0x3344 && args[3]==17);
    reset(); WinHoverWindow=&target; Win::OnMouseMove(window,11,22,0x3344);
    assert(events.size()==3 && leave_owner==0);
    reset(); WinHoverWindow=&prior; WinPointerOwner3=&target; Win::OnMouseMove(window,11,22,0x3344);
    assert(events.size()==3 && leave_owner==0);
    reset(); WinHoverWindow=&prior; mutate_hover=true; Win::OnMouseMove(window,11,22,0x3344);
    assert(events.size()==4 && leave_owner==&tracker && called_owner==&target);
    reset(); hit=0; WinHoverWindow=&prior; Win::OnMouseMove(window,11,22,0x3344);
    assert(events.size()==2 && WinHoverWindow==&prior && leave_owner==0 && called_owner==0);
}
"""


def unit(damage: tuple[str,str] | None=None) -> str:
    source=SOURCE.read_text(); header=(ROOT/'src/win.h').read_text()
    declarations=[]
    for name in ('OnRButtonDown','OnRButtonUp','OnLButtonUp','OnMouseMove'):
        lines=re.findall(r'^\s*static void[^;\n]*\b'+name+r'\([^;\n]*\);',header,re.M)
        assert len(lines)==1
        declarations.append(lines[0])
    bodies='\n'.join(extract_method(source,s) for s in SIGNATURES)
    if damage:
        old,new=damage; assert old in bodies
        bodies=bodies.replace(old,new,1)
    return HOST.replace('// DECLARATIONS_INSERTION','\n'.join(declarations)).replace('// BODIES_INSERTION',bodies)


@pytest.mark.parametrize('compiler',('g++','clang++'))
def test_actual_mouse_adapters(compiler):
    build_and_run(unit(),compiler,expect_success=True)


@pytest.mark.parametrize('compiler',('g++','clang++'))
@pytest.mark.parametrize('old,new',(
    ('get_mouse_window(&x, &y)','get_mouse_window(&y, &x)'),
    ('on_r_button_down(flags, x, y, keys, WinMouseDirect)','on_r_button_down(flags, x, y, y, WinMouseDirect)'),
    ('WinMouseHook)(x, y)','WinMouseHook)(y, x)'),
    ('WinMessageHook();','(void)0;'),
    ('on_l_button_up(x, y, keys, WinMouseDirect)','on_l_button_up(y, x, keys, WinMouseDirect)'),
    ('WinTrackingWindow = nullptr;','(void)0;'),
    ('tracking->do_tracking(x, y)','hit->do_tracking(x, y)'),
    ('WinCursorMoved = 0;','(void)0;'),
))
def test_rejects_argument_receiver_or_hook_damage(compiler,old,new):
    build_and_run(unit((old,new)),compiler,expect_success=False)
