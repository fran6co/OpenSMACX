"""Actual DesignWin panel-array producers and their live consumers."""
from pathlib import Path
import shutil
import subprocess
import tempfile

import pytest


ROOT = Path(__file__).resolve().parents[2]


def definition(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
    raise AssertionError("unterminated definition: " + signature)


def actual_producers():
    source = (ROOT / "src/designwin.cpp").read_text(encoding="utf-8")
    init = definition(source, "void DesignWin::init()")
    component_start = init.index("    SetRect(&component_panel_rects_[component_panel_chassis]")
    component_end = init.index("\n\n    // The vehicle preview", component_start)
    lower_start = init.index("    // Five lower-panel rows retain", component_end)
    lower_end = init.index("\n\n    // Two lower navigation controls", lower_start)
    return init[component_start:component_end], init[lower_start:lower_end]


HARNESS = r'''#include <cassert>
#include <cstring>
struct RECT { int left, top, right, bottom; };
struct Win {};
struct VehPrototype { unsigned flags; };
VehPrototype VehPrototypes[16] = {};
char StringTemp[256];
int Language = 0;
char labels[2048][32] = {};
struct Event { int kind, first, second, x, y, width, height; };
Event events[64]; int event_count;
struct Spot {
  void init(int n) { events[event_count++] = Event{1,n,0,0,0,0,0}; }
  void add(int a,int b,int x,int y,int w,int h) {
    events[event_count++] = Event{2,a,b,x,y,w,h};
  }
};
void SetRect(RECT *r,int l,int t,int x,int b) {
  r->left=l; r->top=t; r->right=x; r->bottom=b;
}
char *label_get(int id) { return labels[id]; }
char *CharUpperA(char *s) {
  for(char *p=s;*p;++p) if(*p>='a'&&*p<='z') *p=(char)(*p-'a'+'A');
  return s;
}
struct FlatButton {
  int id,x,y,w,h; char title[64];
  int init(const char *s,int button,int px,int py,int width,int height,Win*,int) {
    id=button;x=px;y=py;w=width;h=height;
    std::strncpy(title,s,sizeof(title)-1);title[sizeof(title)-1]=0;
    return 0;
  }
  int set_bubble_text(char *) { return 0; }
};
struct DesignWin : Win {
  enum ComponentPanelIndex { component_panel_chassis, component_panel_weapon,
    component_panel_armor, component_panel_reactor, component_panel_special_one,
    component_panel_special_two };
  enum LowerPanelActionIndex { lower_panel_done, lower_panel_rename,
    lower_panel_obsolete, lower_panel_upgrade, lower_panel_cancel };
  RECT component_panel_rects_[6], lower_panel_rows_[5];
  RECT field_B5C_rect_, field_B6C_rect_, field_B4C_rect_, field_B7C_rect_;
  Spot spot1_, spot2_; FlatButton flatButtons_[9]; unsigned protoID_;
  RECT working_;
  DesignWin() : protoID_(1), working_{10,20,410,300} {}
  RECT *buffer_working_rect() { return &working_; }
  void run() {
    int i, row;
    COMPONENT
    LOWER
  }
};
int main() {
  std::strcpy(labels[0xE58/4],"APPLY"); std::strcpy(labels[0xE7C/4],"APPLY_HINT");
  std::strcpy(labels[0x1024/4],"RENAME"); std::strcpy(labels[0x1028/4],"RENAME_HINT");
  std::strcpy(labels[0xE74/4],"RETIRE"); std::strcpy(labels[0xE78/4],"RETIRE_HINT");
  std::strcpy(labels[0x1034/4],"UPGRADE"); std::strcpy(labels[0x1038/4],"UPGRADE_HINT");
  std::strcpy(labels[0xBD0/4],"OBSOLETE"); std::strcpy(labels[0xC58/4],"OBSOLETE_HINT");
  std::strcpy(labels[0xBD4/4],"UPGRADE2"); std::strcpy(labels[0xC5C/4],"UPGRADE2_HINT");
  std::strcpy(labels[0x864/4],"CANCEL"); std::strcpy(labels[0x198/4],"DONE");
  std::strcpy(labels[0xC64/4],"FACTION"); std::strcpy(labels[0xC60/4],"NEUTRAL");
  DesignWin w; w.run();
  const int coords[6][4]={{30,7,129,92},{30,101,129,186},{30,195,129,280},
    {462,7,561,92},{462,101,561,186},{462,195,561,280}};
  assert(event_count >= 8 && events[0].kind==1 && events[0].first==0x40 &&
         events[1].kind==1 && events[1].first==6);
  for(int i=0;i<6;++i) {
    const Event &e=events[2+i]; assert(e.kind==2 && e.first==i && e.second==i);
    assert(e.x==coords[i][0] && e.y==coords[i][1]);
    assert(e.width==coords[i][2]-coords[i][0] && e.height==coords[i][3]-coords[i][1]);
    const RECT &r=w.component_panel_rects_[i];
    assert(r.left==coords[i][0] && r.top==coords[i][1] &&
           r.right==coords[i][2] && r.bottom==coords[i][3]);
  }
  const int left[5]={37,100,163,226,289};
  const int right[5]={97,160,223,286,349};
  for(int i=0;i<5;++i) {
    const RECT &r=w.lower_panel_rows_[i];
    assert(r.left==left[i] && r.top==260 && r.right==right[i] && r.bottom==280);
  }
  // The copied Apply/Retire rectangles remain independent from their source rows.
  assert(w.field_B5C_rect_.left==37 && w.field_B5C_rect_.top==234 &&
         w.field_B5C_rect_.right==97 && w.field_B5C_rect_.bottom==254);
  assert(w.field_B4C_rect_.left==289 && w.field_B4C_rect_.top==234 &&
         w.field_B4C_rect_.right==349 && w.field_B4C_rect_.bottom==254);
  assert(w.field_B6C_rect_.left==100 && w.field_B6C_rect_.right==160);
  assert(w.field_B7C_rect_.left==226 && w.field_B7C_rect_.right==286);
  assert(w.flatButtons_[4].id==4 && w.flatButtons_[4].x==37 && w.flatButtons_[4].y==260);
  assert(w.flatButtons_[5].id==5 && w.flatButtons_[5].x==100 && w.flatButtons_[5].y==260);
  assert(w.flatButtons_[6].id==6 && w.flatButtons_[6].x==163 && w.flatButtons_[6].y==260);
  assert(w.flatButtons_[7].id==7 && w.flatButtons_[7].x==226 && w.flatButtons_[7].y==260);
  assert(w.flatButtons_[8].id==8 && w.flatButtons_[8].x==289 && w.flatButtons_[8].y==260);
  assert(w.lower_panel_rows_[0].top==260 && w.field_B5C_rect_.top==234);
  return 0;
}
'''


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_panel_array_producers_consumers_and_mutants(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip("host compiler unavailable")
    component, lower = actual_producers()
    actual = HARNESS.replace("COMPONENT", component).replace("LOWER", lower)
    variants = {
        "actual": actual,
        "reversed_component_spots": actual.replace(
            "component_panel_rects_[i].left,",
            "component_panel_rects_[5-i].left,", 1),
        "short_component_loop": actual.replace("i < 6; ++i) {\n        spot2_.add",
                                              "i < 5; ++i) {\n        spot2_.add", 1),
        "short_action_loop": actual.replace("row < 5; ++row) {",
                                            "row < 4; ++row) {", 1),
        "wrong_done_button_row": actual.replace(
            "lower_panel_rows_[lower_panel_done].left,",
            "lower_panel_rows_[lower_panel_rename].left,", 1),
    }
    with tempfile.TemporaryDirectory(prefix="designwin-panel-arrays-") as temporary:
        directory = Path(temporary)
        for name, candidate in variants.items():
            if name != "actual":
                assert candidate != actual, name
            cpp, binary = directory / (name + ".cpp"), directory / name
            cpp.write_text(candidate, encoding="utf-8")
            built = subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra",
                                    "-pedantic", str(cpp), "-o", str(binary)],
                                   capture_output=True, text=True, timeout=45)
            assert built.returncode == 0, built.stdout + built.stderr
            ran = subprocess.run([str(binary)], capture_output=True, text=True,
                                 timeout=15)
            assert (ran.returncode == 0) == (name == "actual"), name + ran.stdout + ran.stderr
