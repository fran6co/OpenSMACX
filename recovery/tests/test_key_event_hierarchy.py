"""Council key handler dispatches through the real Win virtual slot 25."""
from pathlib import Path
import os
import re
import shutil
import subprocess

import pytest


def project_root() -> Path:
    configured = os.environ.get("OPENSMACX_ROOT")
    if configured:
        return Path(configured).resolve()
    path = Path(__file__).resolve()
    if len(path.parents) > 2 and (path.parents[2] / "src").is_dir():
        return path.parents[2]
    raise AssertionError("set OPENSMACX_ROOT; no scratch/proposal fallback is allowed")


ROOT = project_root()


def extract_definition(source: str, signature: str) -> str:
    if source.count(signature) != 1:
        raise AssertionError(f"expected one selected definition: {signature}")
    start = source.index(signature)
    opening = source.find("{", start)
    if opening < 0:
        raise AssertionError(f"missing definition body: {signature}")
    depth = 0
    state = "code"
    i = opening
    while i < len(source):
        c = source[i]
        n = source[i + 1] if i + 1 < len(source) else ""
        if state == "code":
            if c == "/" and n == "/":
                state = "line"
                i += 2
                continue
            if c == "/" and n == "*":
                state = "block"
                i += 2
                continue
            if c == '"': state = "string"
            elif c == "'": state = "char"
            elif c == "{": depth += 1
            elif c == "}":
                depth -= 1
                if depth == 0:
                    selected = source[start:i + 1]
                    if "BODY GOES HERE" in selected:
                        raise AssertionError(f"selected definition is a placeholder: {signature}")
                    return selected
        elif state == "line":
            if c == "\n": state = "code"
        elif state == "block":
            if c == "*" and n == "/":
                state = "code"
                i += 2
                continue
        elif state in ("string", "char"):
            if c == "\\":
                i += 2
                continue
            if (state == "string" and c == '"') or (state == "char" and c == "'"):
                state = "code"
        i += 1
    raise AssertionError(f"unterminated body: {signature}")


def test_council_keyboard_override_dispatches_through_base(tmp_path):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("host compiler unavailable")
    win_h = (ROOT / "src/win.h").read_text(encoding="utf-8")
    counc_h = (ROOT / "src/councwin.h").read_text(encoding="utf-8")
    base_decl = re.findall(r"virtual int on_key_click\s*\([^;]+;", win_h)
    derived_decl = re.findall(r"int on_key_click\s*\([^;]+;", counc_h)
    assert len(base_decl) == 1 and re.search(r"=\s*0\s*;", base_decl[0]) is None
    assert len(derived_decl) == 1 and "override" in derived_decl[0]
    source = (ROOT / "src/councwin.cpp").read_text(encoding="utf-8")
    body = extract_definition(source, "int CouncWin::on_key_click(int, int key)")
    fixture = r'''
#include <cassert>
#include <cstdint>
#include <vector>
const int MaxPlayerNum=8;
std::vector<int> Events;
struct { int faction_id_=1; } ConsoleGlobal;
uint32_t LockEnableMask, GameMorePreferences;
int IsMultiplayerPBEM, IsMultiplayerNet, CouncilPbemGate;
int TurnRedrawPending=1, CouncilDialogActive=1, Votes;
int council_get_vote(int faction,int proposal,int target) {
    assert(proposal==3 && target==2); ++Votes;
    if(faction==0)LockEnableMask|=1u<<2;
    return faction+10;
}
void do_all_draws(){Events.push_back(20);}
void council_action(int proposal,int target){assert(proposal==3&&target==2);Events.push_back(21);}
struct Button{void hide(){Events.push_back(2);}};
struct Font{void close(){Events.push_back(3);}};
struct Win {
 virtual ~Win() {}
 virtual int on_key_click(int,int) { return 0; }
 int key_click_event(int first,int second){return on_key_click(first,second);}
};
struct CouncWin:Win {
// DERIVED
 uint32_t field_A24_=0,field_A1C_=3,field_A20_=2,leader_label_kind_[8]={0};
 Button buttons_[6]; Font font1_,font2_,font3_,font4_,font5_;
 void release_modal(){Events.push_back(1);}
};
// BODY
int main(){
 CouncWin window; Win *base=&window;
 assert(base->key_click_event(0,65)==1 && Events.empty());
 LockEnableMask=1u<<1;
 assert(base->key_click_event(0,13)==1 && Events.empty()&&Votes==6);
 assert(window.leader_label_kind_[0]==10&&window.leader_label_kind_[1]==0&&window.leader_label_kind_[2]==0);
 window.field_A24_=1; Votes=0;
 assert(base->key_click_event(0,27)==1&&Votes==0);
 assert(TurnRedrawPending==0&&CouncilDialogActive==0&&Events.size()==14);
 assert(Events.front()==1&&Events[7]==3&&Events[12]==20&&Events[13]==21);
 Events.clear(); IsMultiplayerNet=1;
 base->key_click_event(0,13); assert(Events.size()==13&&Events.back()==20);
}
'''.replace("// DERIVED", derived_decl[0]).replace("// BODY", body)
    source_path = tmp_path / "key_click_hierarchy.cpp"
    binary = tmp_path / "key_click_hierarchy"
    source_path.write_text(fixture, encoding="utf-8")
    subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", str(source_path), "-o", str(binary)], check=True)
    subprocess.run([str(binary)], check=True)


def test_dialogs_mouse_overrides_dispatch_through_shared_virtual_bases(tmp_path):
    source = (ROOT / "src/dialogs.cpp").read_text(encoding="utf-8")
    move_body = extract_definition(source, "void Dialogs::on_mouse_move(int a1, int a2)")
    up_body = extract_definition(source, "void Dialogs::on_left_up(int a, int b)")
    assert "RadioButton::on_mouse_move" in move_body
    assert "ListBox::on_mouse_move" in move_body
    assert "CheckBox::on_mouse_move" in move_body
    assert "SpriteBox::on_mouse_move" in move_body
    assert "kind_ == 4" in up_body and "kind_ == 8" in up_body
    assert "SpriteBox::on_left_up" in up_body

    fixture = r'''
#include <cassert>
struct GraphicWin {
  virtual ~GraphicWin() {}
  virtual void on_mouse_move(int, int) {}
  virtual void on_left_up(int, int) {}
};
struct Dialog { virtual ~Dialog() {} };
struct Call { int count, x, y; const void *receiver;
  Call() : count(0), x(0), y(0), receiver(0) {}
  void set(int a, int b, const void *r=0) {
    ++count; x=a; y=b; receiver=r;
  }
};
struct ListBox : virtual GraphicWin, virtual Dialog {
  Call moved; void on_mouse_move(int x,int y) { moved.set(x,y); }
};
struct RadioButton : virtual GraphicWin, virtual Dialog {
  Call moved; void on_mouse_move(int x,int y) { moved.set(x,y); }
};
struct CheckBox : virtual GraphicWin, virtual Dialog {
  Call moved; void on_mouse_move(int x,int y) { moved.set(x,y); }
};
struct SpriteBox : virtual GraphicWin, virtual Dialog {
  Call moved, left_up;
  void on_mouse_move(int x,int y) { moved.set(x,y); }
  void on_left_up(int x,int y) { left_up.set(x,y,this); }
};
struct EditGroup : virtual GraphicWin, virtual Dialog {};
class Dialogs : public ListBox, public RadioButton, public CheckBox,
                public SpriteBox, public EditGroup {
 public:
  int kind_;
  void on_mouse_move(int,int);
  void on_left_up(int,int);
};
// MOVE_BODY
// UP_BODY
static void reset(Dialogs &d) {
  d.ListBox::moved.count=0; d.RadioButton::moved.count=0;
  d.CheckBox::moved.count=0; d.SpriteBox::moved.count=0;
  d.SpriteBox::left_up.count=0;
}
int main() {
  Dialogs d;
  GraphicWin *shared = static_cast<GraphicWin *>(&d);
  assert(shared == static_cast<GraphicWin *>(static_cast<ListBox *>(&d)));
  assert(shared == static_cast<GraphicWin *>(static_cast<RadioButton *>(&d)));
  assert(shared == static_cast<GraphicWin *>(static_cast<CheckBox *>(&d)));
  assert(shared == static_cast<GraphicWin *>(static_cast<SpriteBox *>(&d)));
  const int kinds[] = {16,2,1,8};
  const Call *expected[] = {&d.RadioButton::moved, &d.ListBox::moved,
                             &d.CheckBox::moved, &d.SpriteBox::moved};
  for (int i=0;i<4;++i) {
    reset(d); d.kind_=kinds[i]; shared->on_mouse_move(123,-456);
    assert(expected[i]->count==1 && expected[i]->x==123 && expected[i]->y==-456);
    assert(d.ListBox::moved.count+d.RadioButton::moved.count+
           d.CheckBox::moved.count+d.SpriteBox::moved.count==1);
  }
  const int ignored[] = {0,4,3,32,-1};
  for (int i=0;i<5;++i) {
    reset(d); d.kind_=ignored[i]; shared->on_mouse_move(7,9);
    assert(d.ListBox::moved.count+d.RadioButton::moved.count+
           d.CheckBox::moved.count+d.SpriteBox::moved.count==0);
  }
  reset(d); d.kind_=4; shared->on_left_up(41,42);
  assert(d.SpriteBox::left_up.count==0);
  d.kind_=8; shared->on_left_up(-31,212);
  assert(d.SpriteBox::left_up.count==1 && d.SpriteBox::left_up.x==-31 &&
         d.SpriteBox::left_up.y==212 &&
         d.SpriteBox::left_up.receiver==static_cast<SpriteBox *>(&d));
  d.kind_=2; shared->on_left_up(4,5);
  assert(d.SpriteBox::left_up.count==1);
}
'''.replace("// MOVE_BODY", move_body).replace("// UP_BODY", up_body)
    available = [name for name in ("g++", "clang++") if shutil.which(name)]
    if not available:
        pytest.skip("host C++ compiler unavailable")
    for compiler_name in available:
        compiler = shutil.which(compiler_name)
        if compiler is None:
            continue
        source_path = tmp_path / ("dialogs_hierarchy_" + compiler_name.replace("+", "p") + ".cpp")
        binary = source_path.with_suffix("")
        source_path.write_text(fixture, encoding="utf-8")
        subprocess.run([compiler, "-std=c++03", "-Wall", "-Wextra", "-Werror",
                        "-pedantic", "-fsanitize=address,undefined",
                        "-fno-omit-frame-pointer", str(source_path), "-o", str(binary)],
                       check=True)
        subprocess.run([str(binary)], check=True,
                       env=dict(os.environ, ASAN_OPTIONS="detect_leaks=0:abort_on_error=1"))


def test_dialogs_unqualified_widget_dispatch_mutant_is_detected(tmp_path):
    source = (ROOT / "src/dialogs.cpp").read_text(encoding="utf-8")
    move_body = extract_definition(source, "void Dialogs::on_mouse_move(int a1, int a2)")
    up_body = extract_definition(source, "void Dialogs::on_left_up(int a, int b)")
    mutant = move_body.replace("RadioButton::on_mouse_move",
                               "on_mouse_move")
    assert mutant != move_body
    # Keep this mutation control isolated: compilation/run must fail because the
    # unqualified virtual call re-enters Dialogs::on_mouse_move.
    fixture = r'''
#include <cassert>
struct GraphicWin { virtual ~GraphicWin() {} virtual void on_mouse_move(int,int) {} virtual void on_left_up(int,int) {} };
struct Dialog { virtual ~Dialog() {} };
struct ListBox : virtual GraphicWin, virtual Dialog { void on_mouse_move(int,int) {} };
struct RadioButton : virtual GraphicWin, virtual Dialog { void on_mouse_move(int,int) {} };
struct CheckBox : virtual GraphicWin, virtual Dialog { void on_mouse_move(int,int) {} };
struct SpriteBox : virtual GraphicWin, virtual Dialog { void on_mouse_move(int,int) {} void on_left_up(int,int) {} };
struct EditGroup : virtual GraphicWin, virtual Dialog {};
class Dialogs : public ListBox, public RadioButton, public CheckBox, public SpriteBox, public EditGroup {
 public: int kind_;
 void on_mouse_move(int,int); void on_left_up(int,int);
};
// MOVE_BODY
// UP_BODY
int main(){ Dialogs d; d.kind_=16; static_cast<GraphicWin*>(&d)->on_mouse_move(1,2); }
'''.replace("// MOVE_BODY", mutant).replace("// UP_BODY", up_body)
    compiler = shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    source_path = tmp_path / "dialogs_virtual_recursion_mutant.cpp"
    binary = tmp_path / "dialogs_virtual_recursion_mutant"
    source_path.write_text(fixture, encoding="utf-8")
    subprocess.run([compiler, "-std=c++03", "-Wall", "-Wextra", "-Werror",
                    "-pedantic", "-fsanitize=address,undefined",
                    "-fno-omit-frame-pointer", str(source_path), "-o", str(binary)],
                   check=True)
    result = subprocess.run([str(binary)], timeout=10, capture_output=True,
                            env=dict(os.environ, ASAN_OPTIONS="detect_leaks=0:abort_on_error=1"))
    assert result.returncode != 0
    assert "stack-overflow" in result.stderr.decode(errors="replace")
