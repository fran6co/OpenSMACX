"""Actual modal bodies and declarations; native VC6 slot layout is separate evidence."""

from pathlib import Path
import re
import shutil
import subprocess

import pytest

from test_win_pointer_identity_owner_oct10 import HARNESS
from test_win_zorder_pointer_flow_oct10 import extract

ROOT = Path(__file__).resolve().parents[2]


def control(damage):
    win = (ROOT / "src/win.cpp").read_text()
    default = extract((ROOT / "src/win.h").read_text(), "virtual int on_modal()")
    declarations = {}
    for owner, stem in (("BasePop", "basepop"), ("AlphaMovie", "alphamovie")):
        header = (ROOT / ("src/" + stem + ".h")).read_text()
        matches = re.findall(r"(?m)^\s*(int on_modal\(\) override;)", header)
        assert len(matches) == 1
        declarations[owner] = matches[0]
    assert "class Popup : public BasePop" in (ROOT / "src/popup.h").read_text()
    assert "class TutWin : public Popup" in (ROOT / "src/tutwin.h").read_text()
    bodies = "\n".join((
        extract(win, "int Win::set_modal(int flags, int (__cdecl *callback)(), Win * owner)"),
        extract(win, "void Win::release_modal()"),
        extract(win, "void Win::bring_child_to_top(Win * child)"),
        extract((ROOT / "src/basepop.cpp").read_text(), "int BasePop::on_modal()"),
        extract((ROOT / "src/alphamovie.cpp").read_text(), "int AlphaMovie::on_modal()"),
    ))
    if damage:
        assert bodies.count(damage[0]) == 1
        bodies = bodies.replace(*damage, 1)
    fixture = HARNESS.split("int main(){", 1)[0]
    stub = "virtual int on_modal(){events[event_count++]=6;return polls++<max_polls;}"
    assert fixture.count(stub) == 1
    fixture = fixture.replace(stub, default)
    pump = "static void wait_task(){++waits;WinFocusWindow=callback_focus;events[event_count++]=9;}"
    assert fixture.count(pump) == 1
    fixture = fixture.replace(pump,
        "static void (*wait_hook)(); static void wait_task(){++waits;assert(waits<=5);if(wait_hook)wait_hook();}")
    owners = r'''
static int BasePopFalloutFlag;
struct MCIVideo {
    int playing=0, checks=0;
    int is_playing(){++checks;return playing;}
};
struct BasePop : Win { BASE_DECL };
struct Popup : BasePop {};
struct TutWin : Popup {};
struct AlphaMovie : Win { MCIVideo mciVideo_; MOVIE_DECL };
'''.replace("BASE_DECL", declarations["BasePop"]).replace("MOVIE_DECL", declarations["AlphaMovie"])
    fixture = fixture.replace("LIVE_BODIES", owners + bodies)
    return fixture + r'''
static AlphaMovie *movie;
static void fallout_after_wait(){if(waits==2)BasePopFalloutFlag=7;}
static void movie_after_wait(){if(waits==3)movie->mciVideo_.playing=0;}
int main(){
    Win plain; BasePop base; Popup popup; TutWin tutorial; AlphaMovie video;
    Win *policies[]={&plain,&base,&popup,&tutorial,&video};
    BasePopFalloutFlag=0;video.mciVideo_.playing=-3;
    for(int i=0;i!=5;++i)assert(policies[i]->on_modal()==1);
    for(int value : {1,-1,7}){
        BasePopFalloutFlag=value;
        for(int i=1;i!=4;++i)assert(policies[i]->on_modal()==0);
    }
    video.mciVideo_.playing=0;assert(policies[4]->on_modal()==0);
    video.mciVideo_.playing=2;assert(policies[4]->on_modal()==1);

    // Each inherited popup policy is re-evaluated after message processing.
    for(int i=1;i!=4;++i){
        reset();BasePopFalloutFlag=0;wait_hook=fallout_after_wait;
        assert(policies[i]->set_modal(0,nullptr,nullptr)==0 && waits==2);
        assert(WinFocusWindow==policies[i] && WinModalDepth==1);
        policies[i]->release_modal();assert(WinModalDepth==0);
    }
    reset();movie=&video;video.mciVideo_.playing=-1;video.mciVideo_.checks=0;
    wait_hook=movie_after_wait;
    assert(policies[4]->set_modal(0,nullptr,nullptr)==0);
    assert(waits==3 && video.mciVideo_.checks==4 && WinFocusWindow==&video);
    video.release_modal();assert(WinModalDepth==0);

    // A false policy skips both the caller hook and the message pump.
    reset();wait_hook=nullptr;BasePopFalloutFlag=1;callback_focus=&plain;
    assert(base.set_modal(0,callback,nullptr)==0 && waits==0);
    assert(WinFocusWindow==&base && event_count==4);
    base.release_modal();
    reset();callback_focus=&video;
    assert(plain.set_modal(0,callback,nullptr)==1 && waits==0);
    assert(WinFocusWindow==&video && WinModalDepth==1);
}
'''


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
@pytest.mark.parametrize("damage", (None,
    ("this->on_modal()", "this->Win::on_modal()"),
    ("BasePopFalloutFlag == 0", "BasePopFalloutFlag != 0"),
    ("mciVideo_.is_playing() != 0", "mciVideo_.is_playing() == 0")))
def test_actual_modal_policy_dispatch(tmp_path, compiler, damage):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(compiler + " unavailable")
    source = tmp_path / "modal_policy.cpp"
    source.write_text(control(damage))
    binary = tmp_path / "modal_policy"
    result = subprocess.run([executable, "-std=c++11", str(source), "-o", str(binary)], capture_output=True, text=True)
    assert result.returncode == 0, result.stderr
    result = subprocess.run([str(binary)], capture_output=True, timeout=10)
    assert (result.returncode == 0) == (damage is None)
