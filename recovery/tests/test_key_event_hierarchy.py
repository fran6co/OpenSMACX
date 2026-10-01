"""Actual Council key body dispatched through declarations from real headers."""
from pathlib import Path
import re
import shutil
import subprocess
import pytest


def test_council_keyboard_override_dispatches_through_base(tmp_path):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("host compiler unavailable")
    root = Path(__file__).resolve().parents[2]
    base = re.search(r"virtual int on_key_event[^\n]+", (root / "src/win.h").read_text())
    derived = re.search(r"int on_key_event[^\n]+", (root / "src/councwin.h").read_text())
    assert base and derived and "override" in derived.group()
    text = (root / "src/councwin.cpp").read_text()
    start = text.index("int CouncWin::on_key_event(")
    opening = text.index("{", start)
    depth = 0
    for end in range(opening, len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            body = text[start:end + 1]
            break
    else:
        raise AssertionError("unterminated actual body")
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
// BASE
 int key_click_event(int first,int second){return on_key_event(first,second);}
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
'''.replace("// BASE", base.group()).replace("// DERIVED", derived.group()).replace("// BODY", body)
    source = tmp_path / "key_event.cpp"
    executable = tmp_path / "key_event"
    source.write_text(fixture)
    result = subprocess.run([compiler,"-std=c++11","-Wall","-Wextra","-Werror",str(source),"-o",str(executable)],capture_output=True,text=True)
    assert result.returncode == 0, result.stderr
    result = subprocess.run([str(executable)],capture_output=True,text=True)
    assert result.returncode == 0, result.stderr
