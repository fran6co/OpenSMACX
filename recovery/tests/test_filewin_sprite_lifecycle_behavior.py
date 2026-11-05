"""Host projection of the actual FileWin class-static Sprite lifecycle.

The test injects the actual FileWin::close and prepare_sprites bodies and the
actual class-static Sprite definitions. A host compiler checks ordinary C++
static construction order and reverse teardown only. It cannot observe or
qualify VC6's private generated guard byte, per-object mask, or reentry path;
those require fresh native generated-fini byte/reference evidence.
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


def class_static_sprites(text):
    definitions = re.findall(
        r"(?m)^Sprite FileWin::(shared_sprite[102]_);$", text)
    assert definitions == ["shared_sprite1_", "shared_sprite0_", "shared_sprite2_"]
    assert "FileWinSpriteStorage" not in text
    assert "FileWinSpriteLifetime" not in text
    assert not re.search(r"(?m)^uint8_t FileWinSpritesCleanupGuard;", text)
    assert "storage_.bytes" not in text
    return "\n".join("Sprite FileWin::" + name + ";" for name in definitions)


@pytest.fixture(params=("g++", "clang++"), scope="module")
def control(request, tmp_path_factory):
    compiler = shutil.which(request.param)
    if compiler is None:
        pytest.skip(f"host compiler unavailable: {request.param}")
    cpp = (ROOT / "src/jackal/filewin.cpp").read_text(encoding="utf-8")
    header = (ROOT / "src/jackal/filewin.h").read_text(encoding="utf-8")
    for name in ("shared_sprite0_", "shared_sprite1_", "shared_sprite2_"):
        assert re.search(r"(?m)^\s*static Sprite " + name + r";$", header)

    objects = class_static_sprites(cpp)
    bodies = "\n".join(definition(cpp, signature) for signature in (
        "void FileWin::close()", "int FileWin::prepare_sprites()"))
    harness = r'''
#include <assert.h>
#include <new>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
struct Sprite;
static std::vector<Sprite *> construction;
static std::vector<int> events;
static std::vector<int> destruction_order;
static int load_result, results[3], sheet_destructors;
static int sprite_destructors[3];
struct Buffer {
  Buffer() { events.push_back(20); }
  ~Buffer() { ++sheet_destructors; events.push_back(21); }
  int load_pcx(const char *name, int a, int b, int c) {
    assert(strcmp(name,"jackal.pcx")==0 && a==0 && b==10 && c==0xEC);
    events.push_back(22); return load_result;
  }
};
struct Sprite {
  uint32_t bounded_payload[16];
  Sprite() { memset(bounded_payload, 0, sizeof(bounded_payload)); construction.push_back(this); }
  ~Sprite();
  void close();
  int extract(Buffer *,int,int,int,int,int,int);
  int create_blank(int,int,int);
};
static int sprite_id(Sprite *p);
struct FileWin {
  // Public only in this minimal host projection so main can inspect identity.
  // The installed product keeps these static members private.
  static Sprite shared_sprite1_;
  static Sprite shared_sprite0_;
  static Sprite shared_sprite2_;
  struct Control {
    int id;
    void close() { events.push_back(id); }
  } flat_button2_,flat_button1_,edit_box_,list_box_;
  char selected_path_[260],selected_basename_[260],file_name_[260];
  int extension_count_; char directory_[263];
  void *modal_window_; int directory_list_handle_;
  void close(); int prepare_sprites();
};
struct ShutdownCheck {
  ShutdownCheck() {}
  ~ShutdownCheck() {
    assert(construction.size() == 3);
    assert(construction[0] == &FileWin::shared_sprite1_);
    assert(construction[1] == &FileWin::shared_sprite0_);
    assert(construction[2] == &FileWin::shared_sprite2_);
    const int expected[] = {2, 0, 1};
    assert(destruction_order.size() == 3);
    for (int i=0; i<3; ++i) {
      assert(destruction_order[i] == expected[i]);
      assert(sprite_destructors[expected[i]] == 1);
    }
  }
} shutdown_check;
''' + objects + r'''
static const char FileWinTextTableDefaultNext[]=".";
static const char FileWinSpriteSheet[]="jackal.pcx";
static void DestroyWindow(int handle) {
  assert(handle==7); events.push_back(15);
}
''' + bodies + r'''
static int sprite_id(Sprite *p) {
  if (p == &FileWin::shared_sprite1_) return 1;
  if (p == &FileWin::shared_sprite0_) return 0;
  if (p == &FileWin::shared_sprite2_) return 2;
  assert(false); return -1;
}
Sprite::~Sprite() {
  const int id=sprite_id(this);
  ++sprite_destructors[id]; destruction_order.push_back(id);
  close();
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
static void expect(const int *values,int count) {
  assert(events.size()==static_cast<unsigned>(count));
  for (int i=0;i<count;++i) assert(events[i]==values[i]);
}
int main(int argc,char **argv) {
  assert(argc==2);
  if (strcmp(argv[1],"construction")==0) return 0;
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
    directory = tmp_path_factory.mktemp("filewin-class-static-sprite-control")
    source = directory / "control.cpp"
    source.write_text(harness)
    binary = directory / "control"
    compiled = subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra",
                               str(source), "-o", str(binary)],
                              capture_output=True, text=True, timeout=30)
    assert compiled.returncode == 0, compiled.stdout + compiled.stderr
    return binary


@pytest.mark.parametrize("mode", ("construction", "close", "prepare"))
def test_actual_filewin_paths_and_class_static_startup_shutdown(control, mode):
    run = subprocess.run([str(control), mode], capture_output=True,
                         text=True, timeout=10)
    assert run.returncode == 0, run.stdout + run.stderr
