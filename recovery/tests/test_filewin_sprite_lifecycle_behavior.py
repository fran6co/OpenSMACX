"""Execute actual FileWin sprite paths with bounded host doubles.

The expected receivers/order come from the pinned instructions, not masked
byte equality. This does not prove VC6 ABI, original exit guards or game runtime.
"""
from pathlib import Path
import re
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]


def definition(text, signature):
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for end in range(opening, len(text)):
        if text[end] == "{":
            depth += 1
        elif text[end] == "}":
            depth -= 1
            if depth == 0:
                return text[start:end + 1]
    raise AssertionError("unterminated actual definition")


@pytest.fixture(params=("g++", "clang++"), scope="module")
def control(request, tmp_path_factory):
    compiler = shutil.which(request.param)
    if compiler is None:
        pytest.skip(f"host compiler unavailable: {request.param}")
    text = (ROOT / "src/jackal/filewin.cpp").read_text()
    declarations = re.findall(r"^static Sprite FileWinSprite[012];$", text, re.M)
    assert len(declarations) == 3
    bodies = "\n".join(definition(text, signature) for signature in (
        "void FileWin::close()", "int FileWin::prepare_sprites()"))
    harness = r'''
#include <assert.h>
#include <string.h>
#include <vector>
struct Sprite;
static std::vector<Sprite *> construction;
static std::vector<int> events;
static int load_result, results[3], sheet_destructors;
struct Buffer {
  Buffer() { events.push_back(20); }
  ~Buffer() { ++sheet_destructors; events.push_back(21); }
  int load_pcx(const char *name, int a, int b, int c) {
    assert(strcmp(name,"jackal.pcx")==0 && a==0 && b==10 && c==0xEC);
    events.push_back(22); return load_result;
  }
};
struct Sprite {
  Sprite() { construction.push_back(this); }
  void close();
  int extract(Buffer *,int,int,int,int,int,int);
  int create_blank(int,int,int);
};
''' + "\n".join(declarations) + r'''
static int sprite_id(Sprite *p) {
  if (p==&FileWinSprite1) return 1; // pinned 0x9B9048
  if (p==&FileWinSprite0) return 0; // pinned 0x9B9018
  assert(p==&FileWinSprite2); return 2;
}
void Sprite::close() { events.push_back(sprite_id(this)); }
int Sprite::extract(Buffer *p,int color,int x,int y,int w,int h,int flags) {
  assert(p && color==9 && y==1 && w==16 && h==16 && flags==0);
  const int id=sprite_id(this);
  assert(x==(id==1 ? 1 : id==0 ? 0x12 : 0x23));
  events.push_back(30+id); return results[id];
}
int Sprite::create_blank(int w,int h,int color) {
  assert(w==16 && h==16 && color==9);
  const int id=sprite_id(this);
  events.push_back(40+id); return results[id];
}
struct Control {
  int id;
  void close() { events.push_back(id); }
};
struct FileWin {
  Control flat_button2_,flat_button1_,edit_box_,list_box_;
  char selected_path_[260],selected_basename_[260],file_name_[260];
  int extension_count_; char directory_[263];
  void *modal_window_; int directory_list_handle_;
  void close(); int prepare_sprites();
};
static const char FileWinTextTableDefaultNext[]=".";
static const char FileWinSpriteSheet[]="jackal.pcx";
static void DestroyWindow(int handle) {
  assert(handle==7); events.push_back(15);
}
''' + bodies + r'''
static void expect(const int *values,int count) {
  assert(events.size()==static_cast<unsigned>(count));
  for (int i=0;i<count;++i) assert(events[i]==values[i]);
}
int main(int argc,char **argv) {
  assert(argc==2);
  if (strcmp(argv[1],"construction")==0) {
    assert(construction.size()==3);
    assert(construction[0]==&FileWinSprite1);
    assert(construction[1]==&FileWinSprite0);
    assert(construction[2]==&FileWinSprite2);
    return 0;
  }
  if (strcmp(argv[1],"close")==0) {
    for (int has_handle=0;has_handle<2;++has_handle) {
      FileWin f; memset(&f,0,sizeof(f));
      f.flat_button2_.id=11; f.flat_button1_.id=12;
      f.edit_box_.id=13; f.list_box_.id=14;
      strcpy(f.selected_path_,"path"); strcpy(f.selected_basename_,"name");
      strcpy(f.file_name_,"name"); strcpy(f.directory_,"old");
      f.extension_count_=5; f.modal_window_=&f;
      f.directory_list_handle_=has_handle ? 7 : 0;
      events.clear(); f.close();
      const int want[]={11,12,13,14,1,0,2,15};
      expect(want,has_handle ? 8 : 7);
      assert(!f.selected_path_[0] && !f.selected_basename_[0] && !f.file_name_[0]);
      assert(!f.extension_count_ && strcmp(f.directory_,".")==0);
      assert(!f.modal_window_ && !f.directory_list_handle_);
    }
    return 0;
  }
  assert(strcmp(argv[1],"prepare")==0);
  for (int loaded=0;loaded<2;++loaded) {
    for (int failure=-1;failure<3;++failure) {
      FileWin f; events.clear(); sheet_destructors=0;
      load_result=loaded ? 0 : 3;
      results[0]=results[1]=results[2]=0;
      const int order[]={1,0,2};
      if (failure>=0) results[order[failure]]=failure==1 ? -7 : 17;
      const int expected=failure<0 ? 0 : results[order[failure]];
      assert(f.prepare_sprites()==expected && sheet_destructors==1);
      int want[]={20,22,0,0,0,21};
      const int calls=failure<0 ? 3 : failure+1;
      for (int i=0;i<calls;++i) want[2+i]=(loaded ? 30 : 40)+order[i];
      want[2+calls]=21;
      expect(want,3+calls);
    }
  }
}
'''
    directory = tmp_path_factory.mktemp("filewin-sprite-control")
    source = directory / "control.cpp"
    source.write_text(harness)
    binary = directory / "control"
    compiled = subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra",
                               str(source), "-o", str(binary)],
                              capture_output=True, text=True, timeout=30)
    assert compiled.returncode == 0, compiled.stdout + compiled.stderr
    return binary


@pytest.mark.parametrize("path", ("construction", "close", "prepare"))
def test_actual_sprite_paths(control, path):
    ran = subprocess.run([str(control), path], capture_output=True, text=True,
                         timeout=30)
    assert ran.returncode == 0, ran.stdout + ran.stderr
