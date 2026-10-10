"""Actual selection override bodies; host dispatch does not prove native slots."""
from pathlib import Path
import shutil
import subprocess

import pytest

from test_listbox_rendering_actual_source import extract_function

ROOT = Path(__file__).resolve().parents[2]


def fixture(damage):
    parts = []
    for stem, signature in (
        ("maininterface", "void MainInterface::on_selected(int a1, int a2)"),
        ("datalink", "void Datalink::on_selected(int index)"),
        ("quaylewin", "void QuayleWin::on_selected(int selection, int)"),
        ("dipedit", "void DipEdit::on_selected(int index)"),
        ("net_session_picker", "void NetSessionPickerWin::on_selected(int)"),
        ("councwin", "int CouncWin::on_button_text_draw(::GraphicWin *, char *, int)"),
    ):
        # Evidence comments may name the same signature; select its definition.
        parts.append(extract_function((ROOT / ("src/" + stem + ".cpp")).read_text(), "\n" + signature))
    bodies = "\n".join(parts)
    if damage:
        assert damage[0] in bodies
        bodies = bodies.replace(*damage, 1)
    header = (ROOT / "src/win.h").read_text()
    assert "virtual int on_button_text_draw(GraphicWin *, char *, int)" in header
    assert header.index("virtual void on_selected(int)") < header.index("virtual void on_selected(int, int)")
    assert "vslot_53" not in (ROOT / "src/quaylewin.cpp").read_text()
    return r'''
#include <cassert>
#include <vector>
struct GraphicWin {};
struct Win {
    virtual int on_button_text_draw(GraphicWin *,char *,int){return 0;}
    virtual void on_selected(int) {}
    virtual void on_selected(int,int) {}
};
struct SubInterface {
    int calls=0,a=0,b=0;
    void on_iface_selected(int x,int y){++calls;a=x;b=y;}
};
unsigned WorldClimateSkipTerrainClear=0;
int ActiveIfaceIdStorage=2,IfaceIdToSlotStorage[4]={0,0,1,0};
SubInterface *InterfaceRegistry[2]={nullptr,nullptr};
struct MainInterface : Win { void on_selected(int a1,int a2) override; };
struct Datalink : Win {
    int field_2A38_=0,draws=0;
    void draw_entry(){++draws;}
    void on_selected(int index) override;
};
struct QuayleWin : Win {
    int field_2144_=0,draws=0;
    void redraw(){++draws;}
    void on_selected(int selection,int) override;
};
struct DipEdit : Win {
    int selected_target_=3;std::vector<int> reads;
    void read_check(){reads.push_back(selected_target_);}
    void do_check(){reads.push_back(selected_target_);}
    void on_selected(int index) override;
};
struct NetSessionPickerWin : Win { void on_selected(int) override; };
struct CouncWin : Win { int on_button_text_draw(::GraphicWin *,char *,int) override; };
BODIES
int main(){
    MainInterface m;Win *receiver=&m;SubInterface target;InterfaceRegistry[1]=&target;
    receiver->on_selected(17,29);assert(target.calls==1 && target.a==17 && target.b==29);
    WorldClimateSkipTerrainClear=1;receiver->on_selected(3,5);assert(target.calls==1);
    WorldClimateSkipTerrainClear=0;InterfaceRegistry[1]=nullptr;receiver->on_selected(3,5);
    Datalink d;receiver=&d;receiver->on_selected(9);assert(d.draws==1);
    d.field_2A38_=1;receiver->on_selected(9);assert(d.draws==1);
    QuayleWin q;receiver=&q;receiver->on_selected(19,37);assert(q.field_2144_==19 && q.draws==1);
    DipEdit e;receiver=&e;receiver->on_selected(6);
    assert(e.selected_target_==6 && (e.reads==std::vector<int>{3,6}));
    NetSessionPickerWin p;receiver=&p;receiver->on_selected(7);
    CouncWin c;receiver=&c;GraphicWin w;char text[]="label";
    assert(receiver->on_button_text_draw(&w,text,11)==0);
}
'''.replace("BODIES", bodies)


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
@pytest.mark.parametrize("damage", (None,
    ("target->on_iface_selected(a1, a2);", "target->on_iface_selected(a2, a1);"),
    ("field_2144_ = selection;", "field_2144_ = 0;"),
    ("read_check();\n    selected_target_ = index;", "selected_target_ = index;\n    read_check();")))
def test_actual_selection_override_dispatch(tmp_path, compiler, damage):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(compiler + " unavailable")
    source = tmp_path / "selection.cpp"
    source.write_text(fixture(damage))
    binary = tmp_path / "selection"
    subprocess.run([executable, "-std=c++11", str(source), "-o", str(binary)],
                   check=True, capture_output=True)
    result = subprocess.run([str(binary)], capture_output=True)
    assert (result.returncode == 0) == (damage is None)
