from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
RIGHT_EVENT_METHODS = (
    ("on_right_down", 0x612ED0),
    ("on_right_double_click", 0x612EF0),
    ("on_right_up", 0x612F40),
    ("on_right_click", 0x612F60),
)


def source_bodies():
    source = (ROOT / "src/dialogs.cpp").read_text()
    bodies = []
    for name, _ in RIGHT_EVENT_METHODS:
        start = source.index("void Dialogs::" + name + "(")
        end = source.index("\n}\n", start) + 3
        body = source[start:end]
        assert "if (kind_ == 8)" in body
        assert "SpriteBox::" + name in body
        assert "reinterpret_cast" not in body
        assert "memcpy" not in body
        bodies.append(body)
    return bodies


def host_source(bodies):
    return r'''#include <cassert>
#include <vector>
struct Event { int method, a, b; const void *receiver; };
static std::vector<Event> events;
struct Win { virtual ~Win() {} };
struct GraphicWin : virtual Win {};
struct Dialog : virtual Win {};
struct ListBox : virtual GraphicWin, virtual Dialog {};
struct RadioButton : virtual GraphicWin, virtual Dialog {};
struct CheckBox : virtual GraphicWin, virtual Dialog {};
struct SpriteBox : virtual GraphicWin, virtual Dialog {
    void on_right_down(int,int);
    void on_right_double_click(int,int);
    void on_right_up(int,int);
    void on_right_click(int,int);
};
struct EditGroup : virtual GraphicWin, virtual Dialog {};
struct Dialogs : ListBox, RadioButton, CheckBox, SpriteBox, EditGroup {
    int kind_;
    SpriteBox &sprite_box_view() { return static_cast<SpriteBox &>(*this); }
    void on_right_down(int,int);
    void on_right_double_click(int,int);
    void on_right_up(int,int);
    void on_right_click(int,int);
};
void SpriteBox::on_right_down(int a,int b) {
    events.push_back((Event){0,a,b,this});
}
void SpriteBox::on_right_double_click(int a,int b) {
    events.push_back((Event){1,a,b,this});
}
void SpriteBox::on_right_up(int a,int b) {
    events.push_back((Event){2,a,b,this});
}
void SpriteBox::on_right_click(int a,int b) {
    events.push_back((Event){3,a,b,this});
}
''' + "\n".join(bodies) + r'''
int main() {
    Dialogs d; d.kind_=16;
    d.on_right_down(1,2); d.on_right_double_click(3,4);
    d.on_right_up(5,6); d.on_right_click(7,8);
    assert(events.empty());

    // Kind 8 alone dispatches all four direct, qualified SpriteBox calls.
    d.kind_=8;
    d.on_right_down(-11,22);
    d.on_right_double_click(33,-44);
    d.on_right_up(-55,-66);
    d.on_right_click(77,88);
    assert(events.size()==4);
    const int methods[4]={0,1,2,3};
    const int args[4][2]={{-11,22},{33,-44},{-55,-66},{77,88}};
    const void *receiver=static_cast<SpriteBox *>(&d);
    for (int i=0;i<4;++i) {
        assert(events[i].method==methods[i]);
        assert(events[i].a==args[i][0] && events[i].b==args[i][1]);
        assert(events[i].receiver==receiver);
    }

    // Other nonzero kinds remain no-op for these right-event methods.
    const int ignored[]={0,1,2,4,7,16,24,-1};
    for (unsigned i=0;i<sizeof(ignored)/sizeof(ignored[0]);++i) {
        d.kind_=ignored[i];
        d.on_right_down(i,-static_cast<int>(i));
        d.on_right_double_click(i,-static_cast<int>(i));
        d.on_right_up(i,-static_cast<int>(i));
        d.on_right_click(i,-static_cast<int>(i));
    }
    assert(events.size()==4);
}
'''


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_right_event_bodies_use_kind_and_spritebox_receiver(tmp_path,
                                                                    compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip("requested host C++ compiler unavailable")
    source = tmp_path / "dialogs_right_events.cpp"
    source.write_text(host_source(source_bodies()))
    executable = tmp_path / "dialogs_right_events"
    compiled = subprocess.run(
        [compiler, "-std=c++98", str(source), "-o", str(executable)],
        capture_output=True, text=True,
    )
    assert compiled.returncode == 0, compiled.stderr
    result = subprocess.run([str(executable)], capture_output=True, text=True)
    assert result.returncode == 0, result.stderr
