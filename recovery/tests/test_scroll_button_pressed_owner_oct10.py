"""Actual Scroll press body: original return, movement, throttle and live parent."""
from pathlib import Path
import shutil
import subprocess

import pytest

from test_listbox_rendering_actual_source import extract_function

ROOT = Path(__file__).resolve().parents[2]


def control(damage=None):
    text = (ROOT / "src/scroll.cpp").read_text()
    body = extract_function(text, "int Scroll::on_button_pressed(int button_id)")
    assert "int on_button_pressed(int button_id) override;" in (ROOT / "src/scroll.h").read_text()
    if damage:
        old, new = damage
        assert old in body
        body = body.replace(old, new, 1)
    return r'''
#include <cassert>
#include <cstdint>
#include <vector>
typedef unsigned int DWORD;
std::vector<int> events;
unsigned ticks=100, sleeps=0;
DWORD timeGetTime(){return ticks++;}
void Sleep(DWORD duration){assert(duration>0);++sleeps;events.push_back(5);}
struct Sound { int queries=0,plays=0; int is_playing(){++queries;events.push_back(3);return 0;} void play(){++plays;events.push_back(4);} };
struct Scroll;
struct Win {
    Sound *sound=nullptr;Scroll *scroll=nullptr;Win *replacement=nullptr;
    int id=0,pos=0,calls=0;
    virtual int on_button_pressed(int){return 0;}
    void on_scrolling(int,int);
    Sound *legacy_sound_33(){return sound;}
};
Win *WinCallbackWindow=nullptr;int ListBoxGlobalDDC=71;
struct Scroll : Win {
    Win *win_parent_=nullptr;
    int field_A38_=0,field_A30_=10,field_A34_=100;
    int position_=3,range_minimum_=0,range_maximum_=10,field_A18_=17;
    unsigned range_reversed_=0,field_A3C_=0;
    void (*field_2148_)(int,int)=nullptr;
    int redraws=0;void redraw(){++redraws;events.push_back(2);}
    int on_button_pressed(int button_id) override;
};
void Win::on_scrolling(int i,int p){++calls;id=i;pos=p;events.push_back(1);if(replacement)scroll->win_parent_=replacement;}
int callback_id=0,callback_pos=0;
void callback(int id,int pos){callback_id=id;callback_pos=pos;events.push_back(6);}
BODY
int main(){
    Scroll s;Win *receiver=&s;assert(receiver->on_button_pressed(1)==1);
    assert(events.empty() && ticks==100 && ListBoxGlobalDDC==71);
    Win a,b;Sound old_sound,new_sound;a.sound=&old_sound;b.sound=&new_sound;
    s.win_parent_=&a;s.field_A38_=100;ticks=101;
    assert(receiver->on_button_pressed(1)==1);
    assert(s.position_==3 && s.redraws==0 && ListBoxGlobalDDC==71);
    ticks=200;s.field_A38_=0;s.position_=0;
    assert(receiver->on_button_pressed(1)==1);
    assert(s.position_==0 && s.redraws==1 && s.field_A3C_==0xFFFFFFFFu);
    assert(a.calls==0 && old_sound.queries==0 && ListBoxGlobalDDC==0);
    assert((events==std::vector<int>{2,5}));
    events.clear();a.scroll=&s;a.replacement=&b;s.field_A38_=0;s.position_=3;
    s.range_reversed_=1;s.field_2148_=&callback;ticks=300;
    assert(receiver->on_button_pressed(2)==1);
    assert(s.position_==4 && a.id==17 && a.pos==6 && a.calls==1);
    assert(callback_id==17 && callback_pos==6 && s.win_parent_==&b);
    assert(old_sound.queries==0 && new_sound.queries==1 && new_sound.plays==1);
    assert(WinCallbackWindow==&a && (events==std::vector<int>{1,6,3,4,2,5}));
    events.clear();s.win_parent_=&b;s.field_A38_=0;s.position_=10;ticks=400;
    assert(receiver->on_button_pressed(2)==1);
    assert(b.calls==0 && new_sound.queries==1 && s.position_==10);
    assert((events==std::vector<int>{2,5}));
}
'''.replace("BODY", body)


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
@pytest.mark.parametrize("damage", (None,
    ("return 1;", "return 0;"),
    ("if (old_position != position_)", "if (true)"),
    ("win_parent_->legacy_sound_33()", "static_cast<Win *>(WinCallbackWindow)->legacy_sound_33()")))
def test_actual_press_callback(tmp_path, compiler, damage):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(compiler + " unavailable")
    source = tmp_path / "scroll.cpp"
    source.write_text(control(damage))
    binary = tmp_path / "scroll"
    subprocess.run([executable, "-std=c++11", str(source), "-o", str(binary)], check=True, capture_output=True)
    result = subprocess.run([str(binary)], capture_output=True)
    assert (result.returncode == 0) == (damage is None)
