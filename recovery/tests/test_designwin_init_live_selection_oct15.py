"""Actual button-8 init source control for DesignWin's two live selection reads."""
from pathlib import Path
import shutil
import subprocess
import tempfile

import pytest

from test_net_hotseat_owner_actual_source_oct6 import extract_definition


ROOT = Path(__file__).resolve().parents[2]


def button8_block(source: str) -> str:
    init = extract_definition(source, "void DesignWin::init()")
    start = init.index("    const int prototype_id = static_cast<int>(protoID_);")
    end = init.index("\n\n    StringTemp[0] = 0;\n    strcat(StringTemp, label_get(0x438 / 4));", start)
    return init[start:end]


def harness(block: str) -> str:
    return r'''#include <cassert>
#include <cstring>
struct RECT { int left, top, right, bottom; };
struct Win {};
struct VehPrototype { unsigned flags; };
VehPrototype VehPrototypes[16];
char StringTemp[256];
int Language = 0;
char labels[1024][32];
int bubble_label = -1;
int title_label = -1;
int mutate_to = -1;
struct DesignWin;
struct FlatButton {
  DesignWin *owner;
  char title[64];
  char bubble[64];
  int seen_id, seen_x, seen_y, seen_width, seen_height;
  Win *seen_parent;
  int init(const char *, int, int, int, int, int, Win *, int);
  int set_bubble_text(char *text);
};
struct DesignWin : Win {
  enum LowerPanelActionIndex { lower_panel_done, lower_panel_rename,
      lower_panel_obsolete, lower_panel_upgrade, lower_panel_cancel };
  unsigned int protoID_;
  RECT lower_panel_rows_[5];
  FlatButton flatButtons_[9];
  void run();
  explicit DesignWin(int selected) : protoID_(selected) {
    lower_panel_rows_[lower_panel_cancel].left=10;
    lower_panel_rows_[lower_panel_cancel].top=20;
    lower_panel_rows_[lower_panel_cancel].right=90;
    lower_panel_rows_[lower_panel_cancel].bottom=80;
    for (int i=0;i<9;++i) flatButtons_[i].owner=this;
  }
};
char *label_get(int id) {
  if (id == 0x864/4 || id == 0x198/4) title_label=id;
  if (id == 0xC64/4 || id == 0xC60/4) bubble_label=id;
  return labels[id];
}
char *CharUpperA(char *s) {
  for(char *p=s;*p;++p) if(*p>='a'&&*p<='z') *p=(char)(*p-'a'+'A');
  return s;
}
int FlatButton::init(const char *name, int id, int x, int y, int width,
                     int height, Win *parent, int) {
  if(id==8) {
    std::strncpy(title,name,sizeof(title)-1); title[sizeof(title)-1]=0;
    seen_id=id; seen_x=x; seen_y=y; seen_width=width; seen_height=height;
    seen_parent=parent;
    if(mutate_to>=0) owner->protoID_=(unsigned int)mutate_to;
  }
  return 0;
}
int FlatButton::set_bubble_text(char *text) {
  std::strncpy(bubble,text,sizeof(bubble)-1); bubble[sizeof(bubble)-1]=0;
  return 0;
}

void DesignWin::run() {
  BLOCK
}
int main() {
  std::strcpy(labels[0x198/4],"title_none");
  std::strcpy(labels[0x864/4],"title_faction");
  std::strcpy(labels[0xC60/4],"bubble_none");
  std::strcpy(labels[0xC64/4],"bubble_faction");
  // Stable non-faction and faction-owned button labels.
  VehPrototypes[1].flags=0; VehPrototypes[2].flags=0x80;
  {
    DesignWin w(1); mutate_to=-1; title_label=bubble_label=-1;
    w.run();
    assert(title_label==0x198/4 && bubble_label==0xC60/4);
    assert(std::strcmp(w.flatButtons_[8].title,"TITLE_NONE")==0);
    assert(std::strcmp(w.flatButtons_[8].bubble,"bubble_none")==0);
    assert(w.flatButtons_[8].seen_id==8 && w.flatButtons_[8].seen_x==10 &&
           w.flatButtons_[8].seen_y==20 && w.flatButtons_[8].seen_width==80 &&
           w.flatButtons_[8].seen_height==60 &&
           w.flatButtons_[8].seen_parent==static_cast<Win *>(&w));
  }
  {
    DesignWin w(2); mutate_to=-1; title_label=bubble_label=-1;
    w.run();
    assert(title_label==0x864/4 && bubble_label==0xC64/4);
    assert(std::strcmp(w.flatButtons_[8].title,"TITLE_FACTION")==0);
    assert(std::strcmp(w.flatButtons_[8].bubble,"bubble_faction")==0);
  }
  // The first label is based on the old selection, but the bubble decision
  // reloads protoID_ after FlatButton::init's observable call boundary.
  {
    DesignWin w(1); mutate_to=2; title_label=bubble_label=-1;
    w.run();
    assert(title_label==0x198/4 && bubble_label==0xC64/4);
    assert(std::strcmp(w.flatButtons_[8].title,"TITLE_NONE")==0);
    assert(std::strcmp(w.flatButtons_[8].bubble,"bubble_faction")==0);
  }
  {
    DesignWin w(2); mutate_to=1; title_label=bubble_label=-1;
    w.run();
    assert(title_label==0x864/4 && bubble_label==0xC60/4);
    assert(std::strcmp(w.flatButtons_[8].title,"TITLE_FACTION")==0);
    assert(std::strcmp(w.flatButtons_[8].bubble,"bubble_none")==0);
  }
  {
    Language=2; DesignWin w(2); mutate_to=-1;
    w.run();
    assert(std::strcmp(w.flatButtons_[8].title,"title_faction")==0);
    Language=0;
  }
  return 0;
}
'''.replace("BLOCK", block)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_button8_selection_reload_and_cached_id_mutant(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip("host compiler unavailable")
    source = (ROOT / "src/designwin.cpp").read_text()
    block = button8_block(source)
    cached = block.replace(
        "const int current_prototype_id = static_cast<int>(protoID_);",
        "const int current_prototype_id = prototype_id;",
        1,
    )
    assert cached != block
    with tempfile.TemporaryDirectory(prefix="designwin-live-selection-") as temporary:
        directory = Path(temporary)
        for name, candidate in (("actual", block), ("cached", cached)):
            cpp, binary = directory / (name + ".cpp"), directory / name
            cpp.write_text(harness(candidate))
            build = subprocess.run(
                [compiler, "-std=c++98", "-Wall", "-Wextra", "-pedantic",
                 str(cpp), "-o", str(binary)],
                capture_output=True, text=True, timeout=45,
            )
            assert build.returncode == 0, build.stdout + build.stderr
            run = subprocess.run([str(binary)], capture_output=True, text=True,
                                 timeout=15)
            assert (run.returncode == 0) == (name == "actual"), run.stdout + run.stderr
