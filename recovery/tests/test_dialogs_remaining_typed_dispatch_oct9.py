from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
METHODS = (
    ("item", "int Dialogs::item("),
    ("get_num_items", "int Dialogs::get_num_items("),
    ("on_mousewheel", "void Dialogs::on_mousewheel("),
    ("on_scroll_delete", "int Dialogs::on_scroll_delete("),
    ("on_left_click", "void Dialogs::on_left_click("),
)


def source_bodies():
    source = (ROOT / "src/dialogs.cpp").read_text()
    bodies = []
    for name, marker in METHODS:
        start = source.index(marker)
        end = source.index("\n}\n", start) + 3
        body = source[start:end]
        assert "kind_" in body
        assert "reinterpret_cast<ListBox" not in body
        assert "reinterpret_cast<SpriteBox" not in body
        assert "self -" not in body
        assert "memcpy" not in body
        bodies.append(body)
    assert "__forceinline Dialog *dialog_of" not in source
    return bodies


def host_source(bodies):
    return r'''#include <cassert>
#include <stdint.h>
#include <vector>
struct Scroll {};
struct Events {
    int item_calls, wheel_calls, left_calls, slot_calls;
    int item_kind, item_index, wheel_arg, left_a, left_b;
    const void *item_receiver, *wheel_receiver, *left_receiver, *slot_receiver;
    void *slot_arg, *slot_result;
    Events() : item_calls(0), wheel_calls(0), left_calls(0), slot_calls(0),
        item_kind(-1), item_index(0), wheel_arg(0), left_a(0), left_b(0),
        item_receiver(0), wheel_receiver(0), left_receiver(0),
        slot_receiver(0), slot_arg(0), slot_result(0) {}
} events;
struct ItemList { int entry_count_; };
struct SpriteList { int entry_count_; };
struct Win {
protected:
    unsigned int iFlags_;
    Win *win_parent_;
public:
    Win() : iFlags_(0), win_parent_(0) {}
    virtual void *vslot_09(void *value) {
        ++events.slot_calls; events.slot_receiver=this; events.slot_arg=value;
        return events.slot_result;
    }
};
struct ParentWin : Win {
    void *vslot_09(void *value) {
        ++events.slot_calls; events.slot_receiver=this; events.slot_arg=value;
        return events.slot_result;
    }
};
struct GraphicWin : virtual Win {};
struct Dialog : virtual Win {
    ItemList item_list_;
    int item(char *, int index) {
        ++events.item_calls; events.item_kind=1; events.item_index=index;
        events.item_receiver=this; return 101;
    }
};
struct ListBox : virtual GraphicWin, virtual Dialog {
    int item(char *, int index) {
        ++events.item_calls; events.item_kind=2; events.item_index=index;
        events.item_receiver=this; return 202;
    }
    void on_mousewheel(int value) {
        ++events.wheel_calls; events.wheel_arg=value; events.wheel_receiver=this;
    }
};
struct RadioButton : virtual GraphicWin, virtual Dialog {};
struct CheckBox : virtual GraphicWin, virtual Dialog {};
struct SpriteBox : virtual GraphicWin, virtual Dialog {
    SpriteList list_;
    void on_left_click(int a, int b) {
        ++events.left_calls; events.left_a=a; events.left_b=b;
        events.left_receiver=this;
    }
};
struct EditGroup : virtual GraphicWin, virtual Dialog {};
struct Dialogs : ListBox, RadioButton, CheckBox, SpriteBox, EditGroup {
    int kind_;
    void setup_scroll(unsigned int flags, Win *parent) {
        iFlags_=flags; win_parent_=parent;
    }
    ListBox &list_box_view() { return static_cast<ListBox &>(*this); }
    SpriteBox &sprite_box_view() { return static_cast<SpriteBox &>(*this); }
    int item(char *, int);
    int get_num_items();
    void on_mousewheel(int);
    int on_scroll_delete(Scroll *);
    void on_left_click(int,int);
};
''' + "\n".join(bodies) + r'''
int main() {
    Dialogs d; d.kind_=2;
    char text[]="x";
    assert(d.item(text, -17)==202);
    assert(events.item_calls==1 && events.item_kind==2);
    assert(events.item_receiver==static_cast<ListBox *>(&d));
    assert(events.item_index==-17);
    d.kind_=1; assert(d.item(text, 29)==101);
    assert(events.item_calls==2 && events.item_kind==1);
    assert(events.item_receiver==static_cast<Dialog *>(&d));
    for (int kind=0; kind<=16; ++kind) {
        d.kind_=kind;
        int expected=(kind==1 || kind==2 || kind==4 || kind==16) ? 13 :
                     (kind==8 ? 27 : 0);
        d.Dialog::item_list_.entry_count_=13;
        d.SpriteBox::list_.entry_count_=27;
        assert(d.get_num_items()==expected);
    }

    d.kind_=2; d.on_mousewheel(-123);
    assert(events.wheel_calls==1 && events.wheel_arg==-123);
    assert(events.wheel_receiver==static_cast<ListBox *>(&d));
    d.kind_=8; d.on_mousewheel(99);
    assert(events.wheel_calls==1);

    for (int kind=0; kind<=16; ++kind) {
        const int previous=events.left_calls;
        d.kind_=kind; d.on_left_click(kind*3, -kind*5);
        if (kind>=1 && kind<=7 || kind>=9 && kind<=16) {
            assert(events.left_calls==previous+1);
            assert(events.left_receiver==static_cast<SpriteBox *>(&d));
            assert(events.left_a==kind*3 && events.left_b==-kind*5);
        } else assert(events.left_calls==previous);
    }

    Scroll scroll;
    d.kind_=2; d.setup_scroll(0, 0);
    assert(d.on_scroll_delete(&scroll)==static_cast<int>(reinterpret_cast<intptr_t>(&scroll)));
    assert(events.slot_calls==0);
    d.setup_scroll(0x40, 0);
    assert(d.on_scroll_delete(&scroll)==static_cast<int>(reinterpret_cast<intptr_t>(&scroll)));
    assert(events.slot_calls==0); // the original parent-null branch passes back
    ParentWin parent; d.setup_scroll(0x40, &parent);
    int token=0; events.slot_result=&token;
    assert(d.on_scroll_delete(&scroll)==static_cast<int>(reinterpret_cast<intptr_t>(&token)));
    assert(events.slot_calls==1 && events.slot_receiver==&parent);
    assert(events.slot_arg==&scroll);
    d.kind_=8;
    assert(d.on_scroll_delete(&scroll)==static_cast<int>(reinterpret_cast<intptr_t>(&scroll)));
    assert(events.slot_calls==1);
}
'''


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_dispatch_bodies_preserve_kinds_receivers_and_arguments(tmp_path,
                                                                        compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip("requested host C++ compiler unavailable")
    source = tmp_path / "dialogs_typed_dispatch.cpp"
    source.write_text(host_source(source_bodies()))
    executable = tmp_path / "dialogs_typed_dispatch"
    compiled = subprocess.run(
        [compiler, "-std=c++98", str(source), "-o", str(executable)],
        capture_output=True, text=True,
    )
    assert compiled.returncode == 0, compiled.stderr
    result = subprocess.run([str(executable)], capture_output=True, text=True)
    assert result.returncode == 0, result.stderr
