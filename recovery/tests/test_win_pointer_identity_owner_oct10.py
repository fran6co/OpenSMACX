"""Actual modal and z-order bodies; host controls do not establish PE32 ABI."""

from pathlib import Path
import shutil
import subprocess

import pytest

from test_win_zorder_pointer_flow_oct10 import extract

ROOT = Path(__file__).resolve().parents[2]

HARNESS = r'''
#include <cassert>
#include <vector>
#define __cdecl
typedef void *HCURSOR;
#define IDC_ARROW ((const char *)32512)
struct Win;
static Win *WinModalStack[4], *WinFocusStack[4], *WinRootWindows[4];
static Win *WinFocusWindow, *WinModalWindow, *WinPointerOwner1, *WinPointerOwner2, *WinPointerOwner3, *WinInputFocus, *WinZOrderWindow;
static int WinModalDepth, WinRootCount, WinZOrderCount, WinZOrderFlag;
static int events[32],event_count, polls, max_polls, waits, shrink_roots;
static Win *show_receiver[4], *callback_focus, *capture_receiver;
struct Win {
    unsigned int iFlags_=0,iSomeFlag_=0;
    Win *children_[4]={nullptr,nullptr,nullptr,nullptr};
    int child_count_=0,shown=0,captured=0,released=0;
    virtual void on_lose_mouse_capture(){++captured;capture_receiver=this;events[event_count++]=1;}
    void show(int value){assert(value==0);show_receiver[shown++]=this;events[event_count++]=2;}
    virtual int on_modal(){events[event_count++]=6;return polls++<max_polls;}
    virtual void release_modal();
    int set_modal(int,int (*)(),Win *);
    void bring_child_to_top(Win *);
};
struct ReleaseTarget : Win {
    void release_modal() override {++released;events[event_count++]=8;}
};
static void flush_input(){events[event_count++]=3;}
static HCURSOR LoadCursorA(void *instance,const char *name){assert(!instance && name==IDC_ARROW);events[event_count++]=4;return reinterpret_cast<void *>(1);}
static void SetCursor(HCURSOR cursor){assert(cursor==reinterpret_cast<void *>(1));events[event_count++]=5;}
static void wait_task(){++waits;WinFocusWindow=callback_focus;events[event_count++]=9;}
static int callback(){WinFocusWindow=callback_focus;events[event_count++]=7;return 0;}
static std::vector<Win *> visited;
static void recurse_zorder(Win *root){
    visited.push_back(root);
    if(root==WinRootWindows[0]){WinZOrderWindow=WinRootWindows[1];WinZOrderFlag=19;if(shrink_roots)WinRootCount=1;}
}
LIVE_BODIES
static void reset(){
    WinModalDepth=0;WinRootCount=0;WinZOrderCount=0;WinZOrderFlag=0;
    WinFocusWindow=WinModalWindow=WinPointerOwner1=WinPointerOwner2=WinPointerOwner3=WinInputFocus=WinZOrderWindow=nullptr;
    callback_focus=capture_receiver=nullptr;event_count=polls=max_polls=waits=shrink_roots=0;
    visited.clear();for(int i=0;i!=4;++i)WinModalStack[i]=WinFocusStack[i]=WinRootWindows[i]=nullptr;
}
int main(){
    Win receiver,owner,prior,root1,root2,child1,child2;ReleaseTarget replacement;
    reset();WinModalDepth=4;WinFocusWindow=&prior;
    assert(receiver.set_modal(7,callback,&owner)==0 && WinModalDepth==4 && WinFocusWindow==&prior && !event_count);

    reset();WinModalDepth=1;WinModalStack[0]=&prior;WinFocusStack[0]=&owner;
    WinPointerOwner1=&prior;WinPointerOwner2=&owner;WinPointerOwner3=&prior;WinInputFocus=&owner;
    assert(receiver.set_modal(7,nullptr,&owner)==0);
    assert(WinModalDepth==2 && WinModalStack[1]==&receiver && WinFocusStack[1]==&owner);
    assert(WinFocusWindow==&receiver && WinModalWindow==&owner && capture_receiver==&prior);
    assert(!WinPointerOwner1 && !WinPointerOwner2 && !WinPointerOwner3 && !WinInputFocus);
    assert(prior.captured==1 && receiver.shown==1 && owner.shown==1);
    assert((std::vector<int>(events,events+event_count)==std::vector<int>{1,2,2,3,4,5,6}));
    receiver.release_modal();assert(WinModalDepth==1 && WinModalWindow==&owner && WinFocusWindow==&prior);
    assert(!WinModalStack[1] && !WinFocusStack[1]);

    reset();WinPointerOwner3=&receiver;max_polls=1;callback_focus=&replacement;
    assert(receiver.set_modal(0,callback,nullptr)==1);
    assert(WinPointerOwner3==&receiver && replacement.released==1 && !waits);
    assert(WinFocusStack[0]==nullptr && receiver.captured==0);
    assert((std::vector<int>(events,events+event_count)==std::vector<int>{2,3,4,5,6,7,8}));
    reset();max_polls=1;callback_focus=&replacement;
    assert(receiver.set_modal(0,nullptr,nullptr)==0 && waits==1 && WinFocusWindow==&replacement);

    // Non-top release compacts both parallel stacks; absent owner restores depth.
    reset();WinModalDepth=3;WinModalStack[0]=&prior;WinModalStack[1]=&receiver;WinModalStack[2]=&owner;
    WinFocusStack[0]=&owner;WinFocusStack[1]=&prior;WinFocusStack[2]=&root1;
    receiver.release_modal();assert(WinModalDepth==2 && WinModalStack[1]==&owner && WinFocusStack[1]==&root1);
    child1.release_modal();assert(WinModalDepth==2);
    reset();receiver.release_modal();assert(WinModalDepth==0);
    reset();WinModalDepth=1;WinModalStack[0]=&receiver;WinFocusWindow=&receiver;WinModalWindow=&owner;
    receiver.release_modal();assert(!WinModalDepth && !WinFocusWindow && WinModalWindow==&owner);

    for(int shrink=0;shrink!=2;++shrink){
        reset();shrink_roots=shrink;receiver.child_count_=2;receiver.children_[0]=&child1;receiver.children_[1]=&child2;
        WinRootCount=2;WinRootWindows[0]=&root1;WinRootWindows[1]=&root2;
        root1.iSomeFlag_=root2.iSomeFlag_=1;WinZOrderWindow=&root1;WinZOrderFlag=7;WinZOrderCount=31;
        receiver.bring_child_to_top(&child2);
        assert(receiver.children_[0]==&child2 && receiver.children_[1]==&child1);
        assert(WinZOrderCount==0 && WinZOrderFlag==(shrink?19:0));
        assert(visited==(shrink?std::vector<Win *>{&root1}:std::vector<Win *>{&root1,&root2}));
    }
    reset();child2.iFlags_=0x2000000;WinRootCount=1;WinRootWindows[0]=&root1;
    receiver.bring_child_to_top(&child2);receiver.bring_child_to_top(nullptr);assert(visited.empty());
}
'''


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
@pytest.mark.parametrize("damage", (None,
    ("WinFocusStack[idx] = owner;", "WinFocusStack[idx] = this;"),
    ("Win *cur = WinFocusWindow;", "Win *cur = this;"),
    ("\n                cur = WinZOrderWindow;", "\n                cur = 0;")))
def test_actual_modal_and_zorder_pointer_flow(tmp_path, compiler, damage):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(compiler + " unavailable")
    text = (ROOT / "src/win.cpp").read_text()
    bodies = "\n".join(extract(text, signature) for signature in (
        "int Win::set_modal(int flags, int (__cdecl *callback)(), Win * owner)",
        "void Win::release_modal()", "void Win::bring_child_to_top(Win * child)"))
    assert "reinterpret_cast" not in bodies
    if damage:
        assert bodies.count(damage[0]) == 1
        bodies = bodies.replace(*damage, 1)
    source = tmp_path / "pointer_identity.cpp"
    source.write_text(HARNESS.replace("LIVE_BODIES", bodies))
    binary = tmp_path / "pointer_identity"
    result = subprocess.run([executable, "-std=c++11", str(source), "-o", str(binary)], capture_output=True, text=True)
    assert result.returncode == 0, result.stderr
    result = subprocess.run([str(binary)], capture_output=True)
    assert (result.returncode == 0) == (damage is None)
