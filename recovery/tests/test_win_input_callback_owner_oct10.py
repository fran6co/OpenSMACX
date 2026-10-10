"""Actual notification bodies, including callback mutations and virtual key-up."""
from pathlib import Path
import shutil
import subprocess

import pytest

from test_listbox_rendering_actual_source import extract_function

ROOT = Path(__file__).resolve().parents[2]


def group_control(damage):
    body = extract_function((ROOT / "src/buttongroup.cpp").read_text(),
                            "int ButtonGroup::button_click(int clicked_id)")
    if damage:
        assert damage[0] in body
        body = body.replace(*damage, 1)
    return r'''
#include <cassert>
#include <cstdint>
#include <vector>
struct ButtonGroup;
struct Win {
    virtual void on_group_clicked(int,int) {}
    virtual void on_group_clicked(int,int,int) {}
};
struct BaseButton {
    uint32_t field_A78_=0,field_A74_=0; Win *owner=nullptr;
    int state=-1,redraws=0;
    void set(int value){state=value;}
    void redraw(){++redraws;}
    Win *parent_window(){return owner;}
};
struct ButtonGroup {
    uint32_t count_=2,field_84_=10,field_88_=0,field_8C_=91,field_90_=0;
    BaseButton *buttons_[2];
    int button_click(int clicked_id);
};
BODY
struct Owner : Win {
    ButtonGroup *g; std::vector<int> calls; bool remove=false;
    void on_group_clicked(int a,int b) override {
        assert(a==10 && b==20 && g->field_84_==20);
        assert(g->buttons_[0]->state==0 && g->buttons_[1]->state==1);
        calls.push_back(47);
        g->buttons_[0]->field_A78_=11;g->buttons_[1]->field_A78_=21;
        g->field_8C_=92;
        if(remove)g->count_=0;
    }
    void on_group_clicked(int a,int b,int c) override {
        assert(a==11 && b==21 && c==92);calls.push_back(46);
    }
};
int main(){
    for(bool remove : {false,true}) {
        ButtonGroup g;Owner o;o.g=&g;o.remove=remove;
        BaseButton a,b;a.field_A78_=10;b.field_A78_=20;a.owner=b.owner=&o;
        g.buttons_[0]=&a;g.buttons_[1]=&b;b.field_A74_=1;
        assert(g.button_click(20)==0);
        assert((o.calls==(remove?std::vector<int>{47}:std::vector<int>{47,46})));
        assert(b.redraws==(remove?1:2));
    }
    ButtonGroup g;Owner o;o.g=&g;BaseButton a,b;
    a.field_A78_=10;b.field_A78_=20;a.owner=b.owner=&o;
    g.buttons_[0]=&a;g.buttons_[1]=&b;g.field_90_=1;
    assert(g.button_click(20)==0 && o.calls.empty());
    g.field_88_=1;assert(g.button_click(20)==0 && o.calls.empty());
}
'''.replace("BODY", body)


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
@pytest.mark.parametrize("damage", (None,
    ("buttons_[active_index]->field_A78_) : -1;\n                const int clicked_again", "10) : -1;\n                const int clicked_again"),
    ("if (count_ != 0)", "if (true)")))
def test_actual_group_notifications(tmp_path, compiler, damage):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(compiler + " unavailable")
    source = tmp_path / "group.cpp"
    source.write_text(group_control(damage))
    binary = tmp_path / "group"
    subprocess.run([executable, "-std=c++11", str(source), "-o", str(binary)],
                   check=True, capture_output=True)
    result = subprocess.run([str(binary)], capture_output=True)
    assert (result.returncode == 0) == (damage is None)


def key_control(damage):
    body = extract_function((ROOT / "src/win.cpp").read_text(),
                            "int Win::key_up_event(int key)")
    pop = extract_function((ROOT / "src/basepop.cpp").read_text(),
                           "int BasePop::on_key_up(int)")
    button = extract_function((ROOT / "src/basebutton.cpp").read_text(),
                              "int BaseButton::on_key_up(int a)")
    if damage:
        assert damage[0] in body
        body = body.replace(*damage, 1)
    return r'''
#include <cassert>
#include <cstdint>
#include <vector>
std::vector<int> events;
struct Win;
Win *WinCallbackWindow=nullptr;
using WinKeyHookFn=int (*)(int);
struct Sound { int is_playing(){events.push_back(3);return 0;} void play(){events.push_back(4);} };
struct Node { Win *focus_owner_=nullptr; };
struct List { int count_=0;Node *head_=nullptr,*current_=nullptr; };
struct Win {
    unsigned iFlags_=0,iSomeFlag_=0;List list_;WinKeyHookFn field_434_=nullptr;
    Sound *val_16_=nullptr;
    virtual int on_key_up(int){return 0;}
    int key_up_event(int key);
};
struct BasePop : Win { uint32_t field_30A8_=0;int on_key_up(int) override; };
struct BaseButton : Win { int on_key_up(int a) override; };
BODY
POP
BUTTON
int hook(int key){assert(key==71);events.push_back(1);return 2;}
struct Probe : Win {
    int on_key_up(int key) override { assert(key==71);assert(WinCallbackWindow==this);events.push_back(2);return 5; }
};
int main(){
    BasePop p;BaseButton b;Win *receiver=&p;
    assert(receiver->on_key_up(71)==1);p.field_30A8_=1u<<14;
    assert(receiver->on_key_up(71)==0);receiver=&b;assert(receiver->on_key_up(71)==0);
    Probe w;Sound sound;w.field_434_=&hook;w.val_16_=&sound;
    assert(w.key_up_event(71)==7 && (events==std::vector<int>{1,2,3,4}));
    events.clear();w.iFlags_=0x200000;assert(w.key_up_event(71)==0 && events.empty());
    w.iFlags_=0;w.iSomeFlag_=8;assert(w.key_up_event(71)==0 && events.empty());
    w.iSomeFlag_=0;Probe child;Node n;n.focus_owner_=&child;
    w.list_.count_=1;w.list_.head_=w.list_.current_=&n;
    assert(w.key_up_event(71)==1 && (events==std::vector<int>{2}));
    assert(WinCallbackWindow==&child);
}
'''.replace("BODY", body).replace("POP", pop).replace("BUTTON", button)


def direct_control(damage):
    body = extract_function((ROOT / "src/datalink.cpp").read_text(),
                            "void Datalink::on_button_passover(int button_id, int state)")
    if damage:
        assert damage[0] in body
        body = body.replace(*damage, 1)
    return r'''
#include <cassert>
int calls=0,seen_id=0,seen_state=0;
struct Win { virtual void on_button_passover(int,int) {} };
struct ReportWin : Win {
    void on_button_passover(int id,int state) override { ++calls;seen_id=id;seen_state=state; }
};
struct Derived : ReportWin { void on_button_passover(int,int) override { calls+=100; } };
Derived report;ReportWin &ReportWinGlobal=report;
struct Datalink : Win { int field_29DC_=0;void on_button_passover(int button_id,int state) override; };
BODY
int main(){ Datalink d;Win *w=&d;w->on_button_passover(7,9);assert(calls==0);
    d.field_29DC_=14;w->on_button_passover(7,9);assert(calls==1 && seen_id==7 && seen_state==9); }
'''.replace("BODY", body)


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
@pytest.mark.parametrize("kind,damage", (
    ("key", None),
    ("key", ("result += this->on_key_up(key);", "result = this->on_key_up(key);")),
    ("direct", None),
    ("direct", ("ReportWinGlobal.ReportWin::on_button_passover", "ReportWinGlobal.on_button_passover"))))
def test_actual_input_call_contracts(tmp_path, compiler, kind, damage):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(compiler + " unavailable")
    source = tmp_path / "input.cpp"
    source.write_text((key_control if kind == "key" else direct_control)(damage))
    binary = tmp_path / "input"
    subprocess.run([executable, "-std=c++11", str(source), "-o", str(binary)],
                   check=True, capture_output=True)
    result = subprocess.run([str(binary)], capture_output=True)
    assert (result.returncode == 0) == (damage is None)
