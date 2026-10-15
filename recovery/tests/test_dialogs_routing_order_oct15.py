"""Actual-source host controls for Dialogs' five recovered event dispatchers."""

from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/IMAGE_PIN").is_file())
SOURCE = (ROOT / "src/dialogs.cpp").read_text(encoding="utf-8")
SIGNATURES = {
    "down": "void Dialogs::on_left_down(int x, int y)",
    "leave": "void Dialogs::on_mouse_leave(int x, int y)",
    "double": "void Dialogs::on_left_double_click(int x, int y)",
    "key": "int Dialogs::on_key_down(int key)",
    "redraw": "void Dialogs::on_redraw()",
}


def extract_body(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start + len(signature))
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError(f"unterminated function: {signature}")


BODIES = {name: extract_body(SOURCE, signature)
          for name, signature in SIGNATURES.items()}


def assert_case_order(body: str, cases: tuple[int, ...]) -> None:
    positions = [body.index(f"case {kind}:") for kind in cases]
    assert positions == sorted(positions)


def test_source_retains_original_case_order_and_empty_kind_four_routes():
    assert_case_order(BODIES["down"], (16, 2, 1, 4, 8))
    assert_case_order(BODIES["leave"], (16, 2, 1, 4, 8))
    assert_case_order(BODIES["double"], (16, 2, 1, 8))
    assert "case 4:" not in BODIES["double"]
    assert_case_order(BODIES["key"], (16, 2, 1, 4, 8))
    assert_case_order(BODIES["redraw"], (16, 2, 1, 8, 4))
    assert "case 4:\n        return;" in BODIES["down"]
    assert "case 4:\n        return;" in BODIES["leave"]
    assert "case 4:\n        return 0;" in BODIES["key"]
    assert "EditGroup::on_redraw()" in BODIES["redraw"]


HARNESS = r'''#include <cassert>
#include <climits>

struct Call { int count, a, b; const void *receiver;
  Call() : count(0), a(0), b(0), receiver(0) {}
  void set(int x, int y, const void *r) { ++count; a=x; b=y; receiver=r; }
};
Call calls[5];
void record(int slot, int a, int b, const void *receiver) {
  calls[slot].set(a,b,receiver);
}
void clear_calls() { for (int i=0;i<5;++i) calls[i]=Call(); }

#define HANDLERS(Slot, Owner, Result) \
  void on_left_down(int x,int y) { record(Slot,x,y,this); } \
  void on_mouse_leave(int x,int y) { record(Slot,x,y,this); } \
  void on_left_double_click(int x,int y) { record(Slot,x,y,this); } \
  int on_key_down(int key) { record(Slot,key,0,this); return Result; } \
  void on_redraw() { record(Slot,0,0,this); }
struct CheckBox { const char *identity; CheckBox():identity("CheckBox"){} HANDLERS(0,1,101) };
struct ListBox { const char *identity; ListBox():identity("ListBox"){} HANDLERS(1,2,102) };
struct EditGroup { const char *identity; EditGroup():identity("EditGroup"){} HANDLERS(2,3,103) };
struct SpriteBox { const char *identity; SpriteBox():identity("SpriteBox"){} HANDLERS(3,4,104) };
struct RadioButton { const char *identity; RadioButton():identity("RadioButton"){} HANDLERS(4,5,105) };

struct DialogsMock : ListBox, RadioButton, CheckBox, SpriteBox, EditGroup {
  int kind_;
  DialogsMock() : kind_(0) {}
  void on_left_down(int,int);
  void on_mouse_leave(int,int);
  void on_left_double_click(int,int);
  int on_key_down(int);
  void on_redraw();
};
// DOWN
// LEAVE
// DOUBLE
// KEY
// REDRAW

static int owner_for(int kind, int method) {
  if (kind==16) return 5;
  if (kind==2) return 2;
  if (kind==1) return 1;
  if (kind==8) return 4;
  if (method==4 && kind==4) return 3;
  return 0;
}
static const void *receiver_for(DialogsMock &d, int owner) {
  switch(owner) {
  case 1: return static_cast<CheckBox *>(&d);
  case 2: return static_cast<ListBox *>(&d);
  case 3: return static_cast<EditGroup *>(&d);
  case 4: return static_cast<SpriteBox *>(&d);
  case 5: return static_cast<RadioButton *>(&d);
  default: return 0;
  }
}
static void assert_base_identities_and_distinct_receivers(DialogsMock &d) {
  assert(d.CheckBox::identity && d.CheckBox::identity[0]);
  assert(d.ListBox::identity && d.ListBox::identity[0]);
  assert(d.EditGroup::identity && d.EditGroup::identity[0]);
  assert(d.SpriteBox::identity && d.SpriteBox::identity[0]);
  assert(d.RadioButton::identity && d.RadioButton::identity[0]);
  const void *bases[] = {
    static_cast<CheckBox *>(&d), static_cast<ListBox *>(&d),
    static_cast<EditGroup *>(&d), static_cast<SpriteBox *>(&d),
    static_cast<RadioButton *>(&d)
  };
  for (int i=0;i<5;++i)
    for (int j=i+1;j<5;++j) assert(bases[i]!=bases[j]);
}
int main() {
  const int extras[] = {INT_MIN,-1,17,31,INT_MAX};
  DialogsMock d;
  assert_base_identities_and_distinct_receivers(d);
  for (int kind=-1;kind<=32;++kind) {
    d.kind_=kind;
    for (int method=0;method<5;++method) {
      clear_calls();
      int result=-999;
      switch(method) {
      case 0: d.on_left_down(-123,456); break;
      case 1: d.on_mouse_leave(-123,456); break;
      case 2: d.on_left_double_click(-123,456); break;
      case 3: result=d.on_key_down(-789); break;
      case 4: d.on_redraw(); break;
      }
      int owner=owner_for(kind,method);
      int slot=owner==0 ? -1 : owner-1;
      for (int i=0;i<5;++i) assert(calls[i].count==(i==slot ? 1 : 0));
      if (method==3) assert(result==(owner==0 ? 0 : 100+owner));
      if (owner!=0) {
        const int a=method==4 ? 0 : (method==3 ? -789 : -123);
        const int b=(method==4 || method==3) ? 0 : 456;
        assert(calls[slot].a==a && calls[slot].b==b);
        assert(calls[slot].receiver==receiver_for(d,owner));
      }
    }
  }
  for (unsigned i=0;i<sizeof(extras)/sizeof(extras[0]);++i) {
    d.kind_=extras[i];
    clear_calls(); d.on_left_down(1,2); d.on_mouse_leave(1,2);
    d.on_left_double_click(1,2); d.on_redraw();
    assert(d.on_key_down(3)==0);
    for (int slot=0;slot<5;++slot) assert(calls[slot].count==0);
  }
}
'''


def fixture(bodies: dict[str, str]) -> str:
    names = {"down": "DOWN", "leave": "LEAVE", "double": "DOUBLE",
             "key": "KEY", "redraw": "REDRAW"}
    result = HARNESS
    for name, marker in names.items():
        body = bodies[name].replace("Dialogs::", "DialogsMock::")
        result = result.replace(f"// {marker}", body)
    return result


def compile_and_run(compiler: str, source: str, stem: str, tmp_path: Path) -> subprocess.CompletedProcess:
    cpp, binary = tmp_path / f"{stem}.cpp", tmp_path / stem
    cpp.write_text(source, encoding="utf-8")
    subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                    str(cpp), "-o", str(binary)], check=True, timeout=30)
    return subprocess.run([str(binary)], capture_output=True, timeout=30)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_handler_routing_arguments_and_receivers(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")
    result = compile_and_run(compiler, fixture(BODIES), "routing", tmp_path)
    assert result.returncode == 0, result.stderr.decode(errors="replace")


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_routing_mutants_compile_but_fail_behavior_controls(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")
    mutants = {}

    wrong_down = dict(BODIES)
    wrong_down["down"] = wrong_down["down"].replace(
        "RadioButton::on_left_down(x, y)", "ListBox::on_left_down(x, y)", 1)
    mutants["wrong_down_receiver"] = wrong_down

    wrong_kind_four = dict(BODIES)
    # This is a source-routing mutant: the real inherited EditGroup handler is
    # empty, but kind 4 must still take the explicit no-op route in Dialogs.
    wrong_kind_four["leave"] = wrong_kind_four["leave"].replace(
        "case 4:\n        return;",
        "case 4:\n        EditGroup::on_mouse_leave(x, y);\n        break;", 1)
    mutants["kind_four_leave"] = wrong_kind_four

    wrong_redraw = dict(BODIES)
    wrong_redraw["redraw"] = wrong_redraw["redraw"].replace(
        "EditGroup::on_redraw()", "SpriteBox::on_redraw()", 1)
    mutants["wrong_redraw_owner"] = wrong_redraw

    for name, bodies in mutants.items():
        result = compile_and_run(compiler, fixture(bodies), name, tmp_path)
        assert result.returncode != 0, f"{name} unexpectedly passed"
