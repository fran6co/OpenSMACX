"""Actual ListBox/AlphaSave/FileWin callback bodies; native slots checked separately."""

from pathlib import Path
import re
import shutil
import subprocess

import pytest

from test_listbox_rendering_actual_source import extract_function

ROOT = Path(__file__).resolve().parents[2]


def control(damage):
    header = (ROOT / "src/win.h").read_text()
    declarations = re.findall(r"virtual void on_double_clicked\(int(?:, int)?\) \{\}", header)
    assert declarations == ["virtual void on_double_clicked(int) {}", "virtual void on_double_clicked(int, int) {}"]
    assert "void on_double_clicked(int button_id) override;" in (ROOT / "src/alphasave.h").read_text()
    bodies = []
    for path, signature in (
        ("src/listbox.cpp", "void ListBox::on_left_double_click(int, int)"),
        ("src/alphasave.cpp", "void AlphaSave::on_double_clicked(int button_id)"),
        ("src/jackal/filewin.cpp", "void FileWin::on_double_clicked(int a1)"),
    ):
        bodies.append(extract_function((ROOT / path).read_text(), "\n" + signature))
    bodies = "\n".join(bodies)
    if damage:
        assert damage[0] in bodies
        bodies = bodies.replace(*damage, 1)
    return r'''
#include <cassert>
#include <vector>
static std::vector<int> events;
struct Sound {
    int playing=0,checks=0,plays=0;
    int is_playing(){++checks;events.push_back(4);return playing;}
    void play(){++plays;events.push_back(5);}
};
struct Win {
    Sound *sound=nullptr;
    DEFAULTS
    Sound *legacy_sound_37(){return sound;}
};
struct GraphicWin : Win {
    Win *target=nullptr; int reads=0;
    void *legacy_value_31(){++reads;return target;}
};
struct Dialog {int selected=0;int get_selected_id(){return selected;}};
typedef void (*WinNotificationCallback)(int);
struct ListBox : GraphicWin, Dialog {
    WinNotificationCallback double_click_callback_=nullptr;
    void on_left_double_click(int,int);
};
static Win *WinCallbackWindow;
static int ListBoxClickGuard;
struct FileWin {
    int confirmations=0;
    void (*action)()=nullptr;
    void UNK4(){++confirmations;events.push_back(2);if(action)action();}
    void on_double_clicked(int);
};
struct AlphaSave : Win {
    FileWin file_win_;
    void on_double_clicked(int) override;
};
struct TwoArgumentTarget : Win {
    int id=0,guard=0;
    void (*action)()=nullptr;
    void on_double_clicked(int i,int g) override {
        id=i;guard=g;events.push_back(3);if(action)action();
    }
};
static ListBox *live;
static AlphaSave *one;
static TwoArgumentTarget *two;
static Win *audio;
static void notify(int id) {
    assert(id==11 && WinCallbackWindow!=one);
    events.push_back(1);live->target=one;live->selected=17;ListBoxClickGuard=7;
}
static void confirmed() {
    live->target=two;live->selected=23;ListBoxClickGuard=9;
}
static void double_notified() {live->target=audio;}
BODIES
int main() {
    Win initial, last;
    AlphaSave save;
    TwoArgumentTarget paired;
    Sound sound;
    ListBox list;
    live=&list;one=&save;two=&paired;audio=&last;
    save.file_win_.action=confirmed;paired.action=double_notified;
    for(int state=0;state!=3;++state) {
        events.clear();list.target=&initial;list.selected=11;list.reads=0;
        list.double_click_callback_=notify;ListBoxClickGuard=-1;
        last.sound=state==2?nullptr:&sound;sound.playing=state==1;
        sound.checks=sound.plays=0;save.file_win_.confirmations=0;
        list.on_left_double_click(101,203);
        assert(list.reads==4 && WinCallbackWindow==&initial);
        assert(save.file_win_.confirmations==1 && paired.id==23 && paired.guard==9);
        assert(sound.checks==(state!=2) && sound.plays==(state==0));
        assert(events==(state==0?std::vector<int>{1,2,3,4,5}:state==1?std::vector<int>{1,2,3,4}:std::vector<int>{1,2,3}));
    }
    // No user hook still dispatches the genuine single-argument override.
    events.clear();list.double_click_callback_=nullptr;list.target=&save;
    list.reads=0;list.selected=11;ListBoxClickGuard=1;last.sound=nullptr;
    list.on_left_double_click(0,0);
    assert(list.reads==4 && WinCallbackWindow==&save);
    assert((events==std::vector<int>{2,3}) && paired.id==23 && paired.guard==9);
}
'''.replace("DEFAULTS", "\n".join(declarations)).replace("BODIES", bodies)


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
@pytest.mark.parametrize("damage", (None,
    ("\n    target = reinterpret_cast<Win *>(graphic->legacy_value_31());", "\n    target = WinCallbackWindow;"),
    ("const int guard = ListBoxClickGuard;", "const int guard = 0;"),
    ("file_win_.on_double_clicked(button_id);", "(void)button_id;")))
def test_actual_double_click_owner_dispatch(tmp_path, compiler, damage):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(compiler + " unavailable")
    source = tmp_path / "double_click.cpp"
    source.write_text(control(damage))
    binary = tmp_path / "double_click"
    result = subprocess.run([executable, "-std=c++11", str(source), "-o", str(binary)], capture_output=True, text=True)
    assert result.returncode == 0, result.stderr
    result = subprocess.run([str(binary)], capture_output=True)
    assert (result.returncode == 0) == (damage is None)
