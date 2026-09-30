"""Bounded controls for the actual editor deletion body, not game acceptance.

The original constructs Popup before auto_undo, reads global Console selection
even for a different member receiver, and preserves cancellation/confirmation
ordering. Service doubles assume valid loaded vehicle/prototype indices; these
controls do not establish native layout, modal UI behavior or unwind equivalence.
"""

from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def test_editor_deletion_global_selection_and_lifetime(tmp_path):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    source = (ROOT / "src/console.cpp").read_text()
    signature = "void Console::editor_kill_veh(int veh_id)"
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            body = source[start:end + 1]
            break
    else:
        raise AssertionError("unterminated product body")
    assert signature.removeprefix("void Console::") in (
        ROOT / "src/console.h").read_text()
    harness = r'''
#include <cassert>
#include <cstring>
#include <string>
#include <vector>
std::vector<std::string> events;
struct Popup {
    Popup() {events.push_back("construct");}
    ~Popup() {events.push_back("destroy");}
};
struct Console {
    int cursor_active_, selected_vehicle_, cursor_history_index_;
    int cursor_history_x_[32], cursor_history_y_[32];
    void editor_kill_veh(int veh_id);
} ConsoleGlobal;
struct Vehicle {int proto_id;} Vehs[8];
struct Prototype {char veh_name[12];} VehPrototypes[8];
int found, picked, confirmed, expected_vehicle;
void auto_undo() {events.push_back("undo");}
int veh_at(int x,int y) {
    assert(x==17 && y==23); events.push_back("at"); return found;
}
int veh_pick(int vehicle,const char *label,int flags) {
    assert(vehicle==found && !strcmp(label,"PICKKILL") && flags==1);
    events.push_back("pick"); return picked;
}
void parse_says(int slot,char *name,int a,int b) {
    assert(slot==0 && a==-1 && b==-1);
    assert(name==VehPrototypes[Vehs[expected_vehicle].proto_id].veh_name);
    events.push_back("parse");
}
int X_pop(const char *key,void *callback) {
    assert(!strcmp(key,"DELETEME") && callback==nullptr);
    events.push_back("confirm"); return confirmed;
}
void kill(int vehicle) {
    assert(vehicle==expected_vehicle); events.push_back("kill");
}
BODY
int main() {
    Console other{};
    other.cursor_active_=1; other.selected_vehicle_=7;
    ConsoleGlobal.cursor_history_index_=2;
    ConsoleGlobal.cursor_history_x_[2]=17;
    ConsoleGlobal.cursor_history_y_[2]=23;
    for(int i=0;i<8;++i) Vehs[i].proto_id=7-i;
    for(int scenario=0;scenario<6;++scenario) {
        events.clear(); confirmed=1; found=2; picked=4;
        ConsoleGlobal.cursor_active_=0; ConsoleGlobal.selected_vehicle_=3;
        int argument=-1; expected_vehicle=4;
        std::vector<std::string> expected={"construct","undo"};
        if(scenario==0) {argument=5; expected_vehicle=5;}
        else if(scenario==1) {
            ConsoleGlobal.cursor_active_=1; expected_vehicle=3;
        } else {
            expected.push_back("at");
            if(scenario==3) found=-1;
            else {expected.push_back("pick"); if(scenario==4) picked=-1;}
        }
        if(scenario!=3 && scenario!=4) {
            expected.push_back("parse"); expected.push_back("confirm");
            if(scenario==5) confirmed=0;
            else expected.push_back("kill");
        }
        expected.push_back("destroy");
        other.editor_kill_veh(argument);
        assert(events==expected);
    }
}
'''.replace("BODY", body)
    unit = tmp_path / "editor.cpp"
    unit.write_text(harness)
    executable = tmp_path / "editor"
    subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
                    str(unit), "-o", str(executable)], check=True,
                   capture_output=True, text=True)
    subprocess.run([str(executable)], check=True, capture_output=True, text=True)
