"""Actual-source contact/introduction controls, not full settlement acceptance."""

from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def actual_contact_source():
    source = (ROOT / "src/faction.cpp").read_text()
    bodies = []
    for signature in ["static void contact_dialog_arguments(",
                      "int __cdecl wants_to_speak(", "void __cdecl introduce("]:
        start = source.index(signature)
        opening = source.index("{", start)
        depth = 0
        for end in range(opening, len(source)):
            depth += (source[end] == "{") - (source[end] == "}")
            if depth == 0:
                bodies.append(source[start:end + 1])
                break
        else:
            raise AssertionError("unterminated contact body")
    return "\n".join(bodies)


HARNESS = r'''
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#define __cdecl
constexpr int RFLAG_ALIEN=0x80;
struct Player {
    int rule_flags=0, is_leader_female=0, noun_gender=0, is_noun_plural=0;
    char title_leader[32]="title", name_leader[32]="leader", noun_faction[32]="noun";
    char adj_name_faction[32]="named", adj_leader[32]="kind", adj_insult_leader[32]="bad";
    char adj_faction[32]="good", adj_insult_faction[32]="evil", insult_leader[32]="insult";
} Players[8];
struct PlayerData { int planet_busters=0, diplo_treaties[8]={}, ranking=0; } PlayersData[8];
int PopupDialogFactionID, DiplomacyActive, DiplomacyContactResult, DiplomacyEndState;
int GenderDefault, PluralityDefault, DiploScratchFA60, Language;
int DiploFocusX=-1, DiploFocusY=-1, MapLatitudeBounds=20, MapLongitudeBounds=40;
int IsMultiplayerNet, IsMultiplayerPBEM, FactionDiplomacyFlags[8];
char StringTemp[1024], PopupStartCaption[256];
char DiploLabelSendingSETTREATY[]="Sending SET TREATY";
struct Sprite {} portraits[8];
Sprite *FactionDiplomacyPortraits[8];
struct MightRecord { char *adj_start, *adj; } Might[7];
struct Strings { char* get(int index) { return index==1 ? const_cast<char*>("Able") : const_cast<char*>("Strong"); } } StringTable;
std::vector<std::string> events, labels, arguments(10);
int popup_dtors, caption_calls, check_calls, report_calls, start_calls, exec_index;
int focus_calls, draw_calls, network_calls, treaty_calls, await_calls, choice;
int info_end, popup_result_change, focus_result, tech[8][89];
bool enabled;
std::vector<int> selections;
struct Dialogs { int sprite(Sprite*,void*,int) { events.push_back("picture"); return 0; } };
struct Popup {
    Popup() { events.push_back("construct"); }
    ~Popup() { ++popup_dtors; events.push_back("destroy"); DiplomacyContactResult=99; }
    void start(char*,const char *label,int n,void*,int f,void*) {
        assert(std::string(label)=="COMM" && n==-1 && f==0); events.push_back("comm");
    }
    bool picture_enabled() { return enabled; }
    void set_caller_picture(Sprite *p) { assert(p==(enabled?nullptr:FactionDiplomacyPortraits[2])); }
    Dialogs &dialogs() { static Dialogs d; return d; }
    int exec(int zero,int (*)()) { assert(zero==0); return selections.at(exec_index++); }
};
struct DiploPop { void start(int faction) { assert(faction==2); ++start_calls; events.push_back("start"); } } DiplomacyPopup;
struct InfoWin { void set_state(int a,int b) { assert(a==2 && b==1); DiplomacyEndState=info_end; } } ConsoleInfoWin;
struct Console { int focus(int x,int y,int who) {
    assert(who==1 && ((x==3 && y==4) || (x==5 && y==6)));
    ++focus_calls; DiploFocusX=5; DiploFocusY=6; return focus_result;
} } ConsoleGlobal;
struct NetDaemon { void await_diplo(int n) { assert(n==0x443); ++await_calls; } } NetDaemonNet;
void introduce(int,int);
int has_tech(int id,int faction) { events.push_back("tech"+std::to_string(id)+":"+std::to_string(faction)); return tech[faction][id]; }
void diplomacy_check(int a,int b,int n) { assert(a==1 && b==2 && n==0); ++check_calls; }
void diplomacy_caption(int a,int b) { assert(a==1 && b==2); ++caption_calls; }
void parse_says(int index,char *text,int,int) { arguments.at(index)=text; }
void parse_num(int index,int value) { assert(index==0 && value>=1 && value<=10); }
int random(int low,int high) { assert(low==0 && choice<high); return choice; }
char *get_him_her(int faction,int n) { assert(faction==2 && n==0); return const_cast<char*>("her"); }
char *get_phrase(int,int n) { return n==1 ? const_cast<char*>("greeting") : const_cast<char*>("phrase"); }
char *get_noun_phrase(int,int n) { return n==9 ? const_cast<char*>("friend") : const_cast<char*>("nounphrase"); }
char *label_get(int index) { return const_cast<char*>(index==0x454/4 ? "an" : "a"); }
char *_itoa(int value,char *out,int radix) { assert(radix==10); sprintf(out,"%d",value); return out; }
int pop_wait() { return 0; }
int x_pops_minimal(const char *label,Sprite*,int (*)()) { labels.push_back(label); DiplomacyContactResult=popup_result_change; return 0; }
int X_pops(const char *label,int value,Sprite*,int (*)()) { assert(value==0x100000); labels.push_back(label); return 0; }
void report_intel(int who) { assert(who==2); ++report_calls; }
void draw_tiles(int x,int y,int n) { assert(x==5 && y==6 && n==2); ++draw_calls; }
void log_say(char*,int a,int b,int t) { assert(a==1 && b==2 && t==0x4000); }
void message_data(int code,int zero,int a,int b,int t,int set) {
    assert(code==0x2443 && zero==0 && a==1 && b==2 && t==0x4000 && set==1); ++network_calls;
}
void set_treaty(int a,int b,int t,int set) { assert(a==1 && b==2 && t==0x4000 && set==1); ++treaty_calls; }
void reset() {
    for(int i=0;i<8;++i) { Players[i]=Player{}; PlayersData[i]=PlayerData{}; FactionDiplomacyPortraits[i]=&portraits[i]; FactionDiplomacyFlags[i]=0; }
    for(auto &m:Might) { m.adj_start=reinterpret_cast<char*>(2); m.adj=reinterpret_cast<char*>(1); }
    memset(tech,0,sizeof tech); events.clear(); labels.clear(); arguments.assign(10,"");
    selections={0}; exec_index=0; choice=0; enabled=false;
    popup_dtors=caption_calls=check_calls=report_calls=start_calls=0;
    focus_calls=draw_calls=network_calls=treaty_calls=await_calls=0;
    DiplomacyContactResult=DiplomacyEndState=DiplomacyActive=0;
    info_end=popup_result_change=focus_result=0;
    IsMultiplayerNet=IsMultiplayerPBEM=Language=DiploScratchFA60=0;
    DiploFocusX= DiploFocusY=-1;
}
'''

CASES = r'''
int main() {
    reset(); Players[1].rule_flags=RFLAG_ALIEN;
    assert(wants_to_speak(1,2)==0 && popup_dtors==1 && !check_calls && !start_calls);
    assert(events[1]=="tech78:2" && events[2]=="tech4:1");
    reset(); Players[2].rule_flags=RFLAG_ALIEN;
    assert(wants_to_speak(1,2)==0 && events[1]=="tech78:1" && events[2]=="tech4:2");
    reset(); Players[1].rule_flags=RFLAG_ALIEN; tech[2][78]=1; selections={1};
    assert(wants_to_speak(1,2)==0 && check_calls==1);
    reset(); DiplomacyEndState=1; DiplomacyContactResult=7;
    assert(wants_to_speak(1,2)==7 && !start_calls && !check_calls);
    reset(); selections={2,0}; DiploFocusX=3; DiploFocusY=4;
    // introduce's inner Popup teardown changes the shared result first;
    // wants_to_speak must return the live value before its own teardown.
    assert(wants_to_speak(1,2)==99 && report_calls==1 && start_calls==1 && popup_dtors==2);
    assert(focus_calls==2 && draw_calls==2 && check_calls==2);
    assert(labels==std::vector<std::string>({"INTRONEW0"}));
    reset(); selections={-1}; enabled=true;
    assert(wants_to_speak(1,2)==0 && !start_calls);
    reset(); DiplomacyContactResult=8; popup_result_change=0;
    assert(wants_to_speak(1,2)==0 && labels[0]=="URGENTCOMM" && !start_calls);
    reset(); PlayersData[2].planet_busters=1;
    assert(wants_to_speak(1,2)==0 && exec_index==0 && labels[0]=="URGENTCOMM");
    reset(); info_end=1; assert(wants_to_speak(1,2)==0 && !check_calls);
    reset(); choice=3; introduce(1,2);
    assert(labels[0]=="INTRONEW3" && arguments[5]=="an Able");
    reset(); choice=3; Language=1; introduce(1,2);
    assert(arguments[5]=="Able");
    reset(); PlayersData[1].diplo_treaties[2]=0x08000000; choice=0;
    introduce(1,2); assert(labels[0]=="INTRO0" && arguments[5]=="friend");
    reset(); PlayersData[1].diplo_treaties[2]=0x08000020; choice=4;
    introduce(1,2); assert(labels[0]=="INTROMAD4" && arguments[6]=="insult" && arguments[8]=="phrase");
    reset(); PlayersData[1].planet_busters=1; PlayersData[2].planet_busters=1;
    introduce(1,2); assert(labels==std::vector<std::string>({"INTRONEW0","PLANETBUSTER0","PLANETBUSTER2"}));
    assert(treaty_calls==1 && network_calls==0);
    reset(); IsMultiplayerNet=1; PlayersData[1].planet_busters=1;
    introduce(1,2); assert(network_calls==1 && await_calls==1 && treaty_calls==0);
}
'''


def run_host(tmp_path, bodies):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("host g++ required")
    source = tmp_path / "contact.cpp"
    source.write_text(HARNESS + bodies + CASES)
    executable = tmp_path / "contact"
    result = subprocess.run([compiler, "-std=c++17", str(source), "-o", str(executable)],
                            capture_output=True, text=True)
    assert result.returncode == 0, result.stderr
    return subprocess.run([str(executable)], capture_output=True, text=True)


def test_actual_contact_and_introduction(tmp_path):
    result = run_host(tmp_path, actual_contact_source())
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("before,after", [
    ("has_tech(0x4E, other_faction_id)", "has_tech(0x4E, faction_id)"),
    ("ConsoleInfoWin.set_state(2, 1)", "ConsoleInfoWin.set_state(1, 2)"),
    ("DiplomacyContactResult = selected == 0", "DiplomacyContactResult = selected != 0"),
    ("strcat(description, word)", "strcat(description, \"\")"),
    ("NetDaemonNet.await_diplo(0x443)", "NetDaemonNet.await_diplo(0x444)"),
])
def test_contact_damage_controls(tmp_path, before, after):
    source = actual_contact_source()
    assert before in source
    result = run_host(tmp_path, source.replace(before, after, 1))
    assert result.returncode != 0, "damaged source escaped control"


def test_tutorial_popup_captures_result_before_restoring_normal_configuration(tmp_path):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("host g++ required")
    source = (ROOT / "src/popup.cpp").read_text()
    start = source.index("int __cdecl popt(")
    end = source.index("\n}", start) + 2
    body = source[start:end]
    harness = r'''
#include <cassert>
#include <vector>
#define __cdecl
std::vector<int> events;
int shared_result=17;
void popups_tutorial() { events.push_back(1); }
int X_pop(char *caption,const char *label,int value,char *text,int flags,int (*callback)()) {
    assert(caption && label && value==-1 && !text && flags==0 && !callback);
    events.push_back(2); return shared_result;
}
void popups_normal() { events.push_back(3); shared_result=99; }
'''
    harness += body + r'''
int main() {
    char caption[]="TUTOR";
    assert(popt(caption,"TERRAFORM",-1,nullptr,0)==17);
    assert(events==std::vector<int>({1,2,3}) && shared_result==99);
}
'''
    unit = tmp_path / "tutorial.cpp"
    unit.write_text(harness)
    executable = tmp_path / "tutorial"
    subprocess.run([compiler, "-std=c++17", str(unit), "-o", str(executable)],
                   check=True, capture_output=True, text=True)
    subprocess.run([str(executable)], check=True, capture_output=True, text=True)
